/*
 * cg_drawtools_mp.c -- helper functions called by cg_draw, cg_scoreboard,
 * cg_info, etc.  (original source: cgame/cg_drawtools.c)
 *
 * cgame_mp_x86.dll 0x30018CC0 .. 0x3001A4CD, 28 functions in binary order.
 * RTCW's cgame/cg_drawtools.c is the ancestor; the divergences are all CoD's:
 *
 *   - CG_AdjustFrom640 lost RTCW's limbo-menu and widescreen hacks; it is
 *     four multiplies and nothing else, and it is INLINED at every one of its
 *     call sites in this file (the standalone copy at 0x30018CC0 exists for
 *     the other units).
 *   - the bitmap font is gone.  CG_DrawChar / CG_DrawChar2 / CG_DrawStringExt2
 *     / CG_DrawStringExt3 / CG_DrawStrlen / CG_DrawMotd / CG_TeamColor do not
 *     exist here; every string goes out through trap_R_Text_Paint with a font
 *     index and a scale of charHeight/48, and CG_DrawStrlen became
 *     Q_DrawStrlen with CoD's '0'..'7'-only colour escapes.
 *   - CG_DrawRotatedPic builds four rotated verts itself and submits them
 *     through trap_R_DrawQuadPic; CG_DrawRotatedQuadPic (no RTCW counterpart)
 *     does the same for a caller-supplied quad.
 *   - CG_GetColorForHealth / CG_ColorForHealth dropped the armour term.
 *   - the four compass/debug functions at the end are CoD additions.
 *
 * Sizes and offsets in comments are addresses in that DLL.
 *
 * @fidelity: likely
 */

#include <math.h>

#include "cg_local.h"

/* q_shared.h carries only the vec3 macros; both of these are RTCW
 * q_shared.h's. */
#ifndef Vector4Copy
#define Vector4Copy( a,b )      ( (b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3] )
#endif
#ifndef Vector4Average
#define Vector4Average( v,b,s,o )   ( (o)[0]=((v)[0]*(1-(s)))+((b)[0]*(s)), \
                                      (o)[1]=((v)[1]*(1-(s)))+((b)[1]*(s)), \
                                      (o)[2]=((v)[2]*(1-(s)))+((b)[2]*(s)), \
                                      (o)[3]=((v)[3]*(1-(s)))+((b)[3]*(s)) )
#endif

/* RTCW keeps these four in cg_local.h and PITCH/YAW/ROLL in q_shared.h.
 * The sizes are the ones the four string helpers push: 16x16 for big, 8x16
 * for small (0x3001972F, 0x30019817). */
#ifndef BIGCHAR_WIDTH
#define BIGCHAR_WIDTH       16
#define BIGCHAR_HEIGHT      16
#define SMALLCHAR_WIDTH     8
#define SMALLCHAR_HEIGHT    16
#endif
#ifndef YAW
#define PITCH   0
#define YAW     1
#define ROLL    2
#endif
#ifndef STAT_HEALTH
#define STAT_HEALTH     0       /* cg.snap->ps.stats[0] (0x30019B27) */
#endif

/* ANGLE2SHORT/SHORT2ANGLE round trips appear all through the compass code:
 * `fmul 182.04445 / call __ftol2 / and 0FFFFh / fild / fmul 0.0054931641`. */
#define ANGLE2SHORT( x )    ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )    ( (float)( x ) * ( 360.0f / 65536.0f ) )

/*
================
CG_AdjustFrom640      0x30018CC0

Adjusted for resolution and screen aspect ratio.
================
*/
void CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// scale for screen sizes
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

/*
================
G_AdjustCoordinates   0x30018CF0

CG_AdjustFrom640 for a bare point.  The G_ prefix is the recovered spelling.
================
*/
void G_AdjustCoordinates( float *x, float *y ) {
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
}

/*
================
CG_FillRect           0x30018D10

Coordinates are 640*480 virtual values.
=================
*/
void CG_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 1, cgs.media.whiteShader );

	trap_R_SetColor( NULL );
}

/*
==============
CG_FillRectGradient   0x30018D80
==============
*/
void CG_FillRectGradient( float x, float y, float width, float height, const float *color, const float *gradcolor, int gradientType ) {
	trap_R_SetColor( color );

	CG_AdjustFrom640( &x, &y, &width, &height );
	/* trap 74 -- RTCW's trap_R_DrawStretchPicGradient; the two trailing
	   pointers are typed int. */
	trap_syscall_0x4A( x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader,
					   (int)gradcolor, gradientType );

	trap_R_SetColor( NULL );
}

/*
==============
CG_FilledBar          0x30018E00

Generic routine for pretty much all status indicators that show a fractional
value to the player by virtue of how full a drawn box is.

flags:
	left		- 1
	center		- 2		// direction is 'right' by default and orientation is 'horizontal'
	vert		- 4
	nohudalpha	- 8		// don't adjust bar's alpha value by the cg_hudAlpha value
	bg			- 16	// background contrast box
	spacing		- 32
	lerp color	- 256	// use an average of the start and end colors to set the fill color
==============
*/

#define BAR_LEFT        0x0001
#define BAR_CENTER      0x0002
#define BAR_VERT        0x0004
#define BAR_NOHUDALPHA  0x0008
#define BAR_BG          0x0010
// different spacing modes for use w/ BAR_BG
#define BAR_BGSPACING_X0Y5  0x0020
#define BAR_BGSPACING_X0Y0  0x0040

#define BAR_LERP_COLOR  0x0100

/* 1 here, not RTCW's 2: the default inset is `x += 1, w -= 2`
   (0x30018F42..0x30018F6C) and the X0Y5 inset is 3 and 6. */
#define BAR_BORDERSIZE 1

void CG_FilledBar( float x, float y, float w, float h, const float *startColor, float *endColor, const float *bgColor, float frac, int flags ) {
	vec4_t scolor;                                          // 1.1 works on a copy of startColor
	vec4_t backgroundcolor = {1, 1, 1, 0.25f}, colorAtPos;  // colorAtPos is the lerped color if necessary
	int indent = BAR_BORDERSIZE;

	Vector4Copy( startColor, scolor );

	if ( ( flags & BAR_BG ) && bgColor ) { // BAR_BG set, and color specified, use specified bg color
		Vector4Copy( bgColor, backgroundcolor );
	}

	// hud alpha
	if ( !( flags & BAR_NOHUDALPHA ) ) {
		scolor[3] *= cg_hudAlpha.value;
		if ( endColor ) {
			endColor[3] *= cg_hudAlpha.value;
		}
		backgroundcolor[3] *= cg_hudAlpha.value;
	}

	if ( flags & BAR_LERP_COLOR ) {
		Vector4Average( scolor, endColor, frac, colorAtPos );
	}

	// background
	if ( ( flags & BAR_BG ) ) {
		// draw background at full size and shrink the remaining box to fit inside with a border.
		CG_FillRect(   x,
					   y,
					   w,
					   h,
					   backgroundcolor );

		if ( flags & BAR_BGSPACING_X0Y0 ) {          // fill the whole box (no border)

		} else if ( flags & BAR_BGSPACING_X0Y5 ) {   // spacing created for weapon heat
			indent *= 3;
			y += indent;
			h -= ( 2 * indent );

		} else {                                // default spacing of 1 unit on each side
			x += indent;
			y += indent;
			w -= ( 2 * indent );
			h -= ( 2 * indent );
		}
	}

	// adjust for horiz/vertical and draw the fractional box
	if ( flags & BAR_VERT ) {
		if ( flags & BAR_LEFT ) {
			y += ( h * ( 1 - frac ) );
		} else if ( flags & BAR_CENTER ) {
			y += ( h * ( 1 - frac ) / 2 );
		}

		if ( flags & BAR_LERP_COLOR ) {
			CG_FillRect( x, y, w, h * frac, colorAtPos );
		} else {
			CG_FillRect( x, y, w, h * frac, scolor );
		}

	} else {

		if ( flags & BAR_LEFT ) {
			x += ( w * ( 1 - frac ) );
		} else if ( flags & BAR_CENTER ) {
			x += ( w * ( 1 - frac ) / 2 );
		}

		if ( flags & BAR_LERP_COLOR ) {
			CG_FillRect( x, y, w * frac, h, colorAtPos );
		} else {
			CG_FillRect( x, y, w * frac, h, scolor );
		}
	}

}

/*
=================
CG_HorizontalPercentBar   0x30019090
=================
*/
void CG_HorizontalPercentBar( float x, float y, float width, float height, float percent ) {
	vec4_t bgcolor = {0.5f, 0.5f, 0.5f, 0.3f},
		   color = {1.0f, 1.0f, 1.0f, 0.3f};
	CG_FilledBar( x, y, width, height, color, NULL, bgcolor, percent, BAR_BG | BAR_NOHUDALPHA );
}

/*
================
CG_DrawSides          0x30019110

Coords are virtual 640x480.
================
*/
void CG_DrawSides( float x, float y, float w, float h, float size ) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenXScale;
	trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
}

/*
================
CG_DrawTopBottom      0x300191B0
================
*/
void CG_DrawTopBottom( float x, float y, float w, float h, float size ) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenYScale;
	trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}

/*
================
CG_DrawRect           0x30019250

Coordinates are 640*480 virtual values.
=================
*/
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color ) {
	vec4_t hudAlphaColor;

	Vector4Copy( color, hudAlphaColor );
	hudAlphaColor[3] *= cg_hudAlpha.value;

	trap_R_SetColor( hudAlphaColor );

	CG_DrawTopBottom( x, y, width, height, size );
	CG_DrawSides( x, y, width, height, size );

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawPic            0x300192D0

Coordinates are 640*480 virtual values.
=================
*/
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader ) {
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

/* The two constant tables CG_DrawRotatedPic works from, at 0x30060F10 and
 * 0x30060F30.  Both are read one element at a time by an unrolled loop. */
static float cg_quadTexCoords[8] = { 0, 0,  1, 0,  1, 1,  0, 1 };
static float cg_quadCorners[8] = { -1, -1,  1, -1,  1, 1,  -1, 1 };

/*
================
CG_DrawRotatedPic     0x30019330

Coordinates are 640*480 virtual values.  Unlike RTCW's, this one does the
rotation itself and hands the renderer a finished quad.
=================
*/
void CG_DrawRotatedPic( float x, float y, float width, float height, float angle, qhandle_t hShader ) {
	float verts[8];
	float angleRadians, sinAngle, cosAngle;
	float CenterX, CenterY, rWidth, rHeight;
	int i;

	CG_AdjustFrom640( &x, &y, &width, &height );

	angleRadians = angle * (float)M_PI * ( 1.0f / 180.0f );
	sinAngle = (float)sin( angleRadians );
	cosAngle = (float)cos( angleRadians );

	rWidth = width * 0.5f;
	rHeight = height * 0.5f;
	CenterX = x + rWidth;
	CenterY = y + rHeight;

	for ( i = 0; i < 4; i++ ) {
		verts[i * 2 + 0] = CenterX + cg_quadCorners[i * 2 + 0] * rWidth * cosAngle
						   - cg_quadCorners[i * 2 + 1] * rWidth * sinAngle;
		verts[i * 2 + 1] = CenterY + cg_quadCorners[i * 2 + 1] * rHeight * cosAngle
						   + cg_quadCorners[i * 2 + 0] * rHeight * sinAngle;
	}

	trap_R_DrawQuadPic( (int)verts, (int)cg_quadTexCoords, hShader );
}

/*
================
CG_DrawRotatedQuadPic 0x300194F0

Rotate a caller-supplied quad about the origin, translate it to x,y and draw
it.  No RTCW counterpart.
=================
*/
void CG_DrawRotatedQuadPic( float x, float y, const float *verts, const float *texCoords, float angle, qhandle_t hShader ) {
	float outVerts[8];
	float angleRadians, sinAngle, cosAngle;
	float sinX, cosX, sinY, cosY;
	int i;

	angleRadians = angle * (float)M_PI * ( 1.0f / 180.0f );
	sinAngle = (float)sin( angleRadians );
	cosAngle = (float)cos( angleRadians );

	cosX = cgs.screenXScale * cosAngle;
	sinX = cgs.screenXScale * sinAngle;
	sinY = cgs.screenYScale * sinAngle;
	cosY = cgs.screenYScale * cosAngle;

	x *= cgs.screenXScale;
	y *= cgs.screenYScale;

	for ( i = 0; i < 4; i++ ) {
		outVerts[i * 2 + 0] = x + cosX * verts[i * 2 + 0] - sinX * verts[i * 2 + 1];
		outVerts[i * 2 + 1] = y + sinY * verts[i * 2 + 0] + cosY * verts[i * 2 + 1];
	}

	trap_R_DrawQuadPic( (int)outVerts, (int)texCoords, hShader );
}

/*
==================
CG_DrawStringExt      0x30019640

Draws a string through the renderer's text painter.  RTCW walked the string a
character at a time against a bitmap font; 1.1 hands the whole thing to
trap_R_Text_Paint, so forceColor is dead (the painter owns the colour escapes)
and shadow only picks the text style.

`virtualScreen` false means the caller's coordinates and sizes are real pixels
and get divided back into the 640x480 space the painter works in.

Exported to the engine as vmMain entry point 15 (0x3002042B).
==================
*/
void CG_DrawStringExt( float x, float y, const char *string, const float *setColor,
					   qboolean forceColor, qboolean shadow, float charWidth, float charHeight,
					   int maxChars, qboolean virtualScreen ) {
	vec4_t color = { 1, 1, 1, 1 };
	float scale;

	y += charHeight * 0.8f;
	scale = charHeight * ( 1.0f / 48.0f );

	if ( !virtualScreen ) {
		x /= cgs.screenXScale;
		y /= cgs.screenYScale;
		charWidth /= cgs.screenXScale;
		scale /= cgs.screenYScale;
	}

	if ( !setColor ) {
		setColor = color;
	}

	trap_R_Text_Paint( x, y, 5, scale, setColor, string, charWidth, maxChars,
					   shadow ? 3 : 0 );
}

/*
==================
CG_DrawBigString      0x30019710
==================
*/
void CG_DrawBigString( float x, float y, const char *s, float alpha ) {
	float color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	trap_R_Text_Paint( x, y + BIGCHAR_HEIGHT - 2, 4, BIGCHAR_HEIGHT / 48.0f,
					   color, s, BIGCHAR_WIDTH, 0, 3 );
}

/*
==================
CG_DrawBigStringColor 0x30019790
==================
*/
void CG_DrawBigStringColor( float x, float y, const char *s, const float *color ) {
	trap_R_Text_Paint( x, y + BIGCHAR_HEIGHT - 2, 4, BIGCHAR_HEIGHT / 48.0f,
					   color, s, BIGCHAR_WIDTH, 0, 3 );
}

/*
==================
CG_DrawSmallString    0x300197F0
==================
*/
void CG_DrawSmallString( float x, float y, const char *s, float alpha ) {
	float color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	trap_R_Text_Paint( x, y + SMALLCHAR_HEIGHT - 2, 5, SMALLCHAR_HEIGHT / 48.0f,
					   color, s, SMALLCHAR_WIDTH, 0, 0 );
}

/*
==================
CG_DrawSmallStringColor   0x30019870
==================
*/
void CG_DrawSmallStringColor( float x, float y, const char *s, const float *color ) {
	trap_R_Text_Paint( x, y + SMALLCHAR_HEIGHT - 2, 5, SMALLCHAR_HEIGHT / 48.0f,
					   color, s, SMALLCHAR_WIDTH, 0, 0 );
}

/*
=================
Q_DrawStrlen          0x300198D0

Returns character count, skipping color escape codes.  RTCW calls this
CG_DrawStrlen and its Q_IsColorString does not range-check the code; 1.1 only
eats '^0'..'^7'.
=================
*/
int Q_DrawStrlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( s[0] == '^' && s[1] && s[1] != '^' && s[1] >= '0' && s[1] <= '7' ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
=============
CG_TileClearBox       0x30019900

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox( int x, int y, int w, int h, qhandle_t hShader ) {
	float s1, t1, s2, t2;
	s1 = x / 64.0;
	t1 = y / 64.0;
	s2 = ( x + w ) / 64.0;
	t2 = ( y + h ) / 64.0;
	trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}

/*
==============
CG_TileClear          0x30019990

Clear around a sized down screen.
==============
*/
void CG_TileClear( void ) {
	int top, bottom, left, right;
	int w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if ( cg.refdef.x == 0 && cg.refdef.y == 0 &&
		 cg.refdef.width == w && cg.refdef.height == h ) {
		return;     // full screen rendering
	}

	top = cg.refdef.y;
	bottom = top + cg.refdef.height - 1;
	left = cg.refdef.x;
	right = left + cg.refdef.width - 1;

	// clear above view screen
	CG_TileClearBox( 0, 0, w, top, cgs.media.backTileShader );

	// clear below view screen
	CG_TileClearBox( 0, bottom, w, h - bottom, cgs.media.backTileShader );

	// clear left of view screen
	CG_TileClearBox( 0, top, left, bottom - top + 1, cgs.media.backTileShader );

	// clear right of view screen
	CG_TileClearBox( right, top, w - right, bottom - top + 1, cgs.media.backTileShader );
}

/*
================
CG_FadeColor          0x30019A30

FADE_TIME is 100 here, not RTCW's 200 (0x30019A46).
================
*/
float *CG_FadeColor( int startMsec, int totalMsec ) {
	static vec4_t color;
	int t;

	if ( startMsec == 0 ) {
		return NULL;
	}

	t = cg.time - startMsec;

	if ( t >= totalMsec ) {
		return NULL;
	}

	// fade out
	if ( totalMsec - t < FADE_TIME ) {
		color[3] = ( totalMsec - t ) * 1.0f / FADE_TIME;
	} else {
		color[3] = 1.0;
	}
	color[0] = color[1] = color[2] = 1;

	color[3] *= cg_hudAlpha.value;

	return color;
}

/*
=================
CG_GetColorForHealth  0x30019AA0

1.1 dropped RTCW's armour term: `armor` is accepted and never read, and the
two divisions come out of the compiler as multiplies by the reciprocal.
Nothing in the DLL calls this or CG_ColorForHealth.
=================
*/
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor ) {
	if ( health <= 0 ) {
		VectorClear( hcolor );  // black
		hcolor[3] = 1;
		return;
	}

	// set the color based on health
	hcolor[0] = 1.0;
	hcolor[3] = 1.0;
	if ( health >= 100 ) {
		hcolor[2] = 1.0;
	} else if ( health < 66 ) {
		hcolor[2] = 0;
	} else {
		hcolor[2] = ( health - 66 ) / 33.0f;
	}

	if ( health > 60 ) {
		hcolor[1] = 1.0;
	} else if ( health < 30 ) {
		hcolor[1] = 0;
	} else {
		hcolor[1] = ( health - 30 ) / 30.0f;
	}
}

/*
=================
CG_ColorForHealth     0x30019B20
=================
*/
void CG_ColorForHealth( vec4_t hcolor ) {
	int health;

	health = cg.snap->ps.stats[STAT_HEALTH];
	if ( health <= 0 ) {
		VectorClear( hcolor );  // black
		hcolor[3] = 1;
		return;
	}

	hcolor[0] = 1.0;
	hcolor[3] = 1.0;
	if ( health >= 100 ) {
		hcolor[2] = 1.0;
	} else if ( health < 66 ) {
		hcolor[2] = 0;
	} else {
		hcolor[2] = ( health - 66 ) / 33.0f;
	}

	if ( health > 60 ) {
		hcolor[1] = 1.0;
	} else if ( health < 30 ) {
		hcolor[1] = 0;
	} else {
		hcolor[1] = ( health - 30 ) / 30.0f;
	}
}

/*
=================
CG_UpdateCompassOrientation   0x30019BD0

Spring the compass rose towards the player's yaw, in 5 msec steps.  Both
statics below start at -1 (0x300EEF2C, 0x300EEF30).
=================
*/
void CG_UpdateCompassOrientation( void ) {
	static int lastCompassTime = -1;
	int msec, step;
	float playerAngle, delta, frametime;

	playerAngle = (float)SHORT2ANGLE( ANGLE2SHORT( cg.refdefViewAngles[YAW] - cg.compassNorthYaw ) );

	if ( lastCompassTime > cg.time ) {
		goto reset;
	}
	msec = cg.time - lastCompassTime;
	if ( (float)msec > 500.0f ) {
		goto reset;
	}
	lastCompassTime = cg.time;

	delta = AngleSubtract( cg.compassPlayerAngle, playerAngle );

	while ( msec > 0 ) {
		if ( msec > 5 ) {
			step = 5;
			msec -= 5;
		} else {
			step = msec;
			msec = 0;
		}
		frametime = step * 0.001f;

		if ( fabs( delta ) < 0.25 && fabs( cg.unknown_0x2AF64 ) < 1.0 ) {
			cg.unknown_0x2AF64 = 0;
			cg.compassPlayerAngle = playerAngle;
			return;
		}

		delta = (float)SHORT2ANGLE( ANGLE2SHORT( cg.unknown_0x2AF64 * frametime + delta ) );
		if ( delta > 180.0f ) {
			delta -= 360.0f;
		}

		if ( delta > 0 ) {
			cg.unknown_0x2AF64 -= frametime * 1000.0f;
		} else if ( delta < 0 ) {
			cg.unknown_0x2AF64 += frametime * 1000.0f;
		}

		cg.unknown_0x2AF64 -= cg.unknown_0x2AF64 * frametime * 2;

		if ( cg.unknown_0x2AF64 > 0 ) {
			if ( delta > 0 ) {
				cg.unknown_0x2AF64 -= cg.unknown_0x2AF64 * frametime * 3.5f;
			}
			cg.unknown_0x2AF64 -= frametime;
			if ( cg.unknown_0x2AF64 < 0 ) {
				cg.unknown_0x2AF64 = 0;
				continue;
			}
		} else {
			if ( delta < 0 ) {
				cg.unknown_0x2AF64 -= cg.unknown_0x2AF64 * frametime * 3.5f;
			}
			cg.unknown_0x2AF64 += frametime;
			if ( cg.unknown_0x2AF64 > 0 ) {
				cg.unknown_0x2AF64 = 0;
				continue;
			}
		}

		if ( cg.unknown_0x2AF64 > 30000.0f ) {
			cg.unknown_0x2AF64 = 30000.0f;
		} else if ( cg.unknown_0x2AF64 < -30000.0f ) {
			cg.unknown_0x2AF64 = -30000.0f;
		}
	}

	cg.compassPlayerAngle = (float)SHORT2ANGLE( ANGLE2SHORT( delta + playerAngle ) );
	return;

reset:
	lastCompassTime = cg.time;
	cg.compassPlayerAngle = playerAngle;
	cg.unknown_0x2AF64 = 0;
}

/*
=================
CG_UpdateCompPointerOrientation   0x30019F00

The same spring for the compass pointers, with its own constants and a hard
+-10 degree cap on the error it will chase.
=================
*/
void CG_UpdateCompPointerOrientation( void ) {
	static int lastPointerTime = -1;
	int msec, step;
	float angle, delta, frametime;

	if ( !cg_hudCompassSpringyPointers.integer ) {
		cg.compassPointerAngle = cg.refdefViewAngles[YAW];
		return;
	}

	if ( lastPointerTime == cg.time ) {
		return;
	}

	angle = cg.refdefViewAngles[YAW];

	if ( lastPointerTime > cg.time ) {
		goto reset;
	}
	msec = cg.time - lastPointerTime;
	if ( (float)msec > 500.0f ) {
		goto reset;
	}
	lastPointerTime = cg.time;

	delta = AngleSubtract( cg.compassPointerAngle, angle );
	if ( fabs( delta ) > 10.0 ) {
		if ( delta >= 0 ) {
			delta = 10.0f;
		} else {
			delta = -10.0f;
		}
	}

	while ( msec > 0 ) {
		if ( msec > 5 ) {
			step = 5;
			msec -= 5;
		} else {
			step = msec;
			msec = 0;
		}
		frametime = step * 0.001f;

		if ( fabs( delta ) < 0.5 && fabs( cg.unknown_0x2AF6C ) < 2.0 ) {
			cg.compassPointerAngle = angle;
			cg.unknown_0x2AF6C = 0;
			return;
		}

		delta = AngleNormalize180( cg.unknown_0x2AF6C * frametime + delta );

		if ( delta > 0 ) {
			cg.unknown_0x2AF6C -= frametime * 1500.0f;
		} else if ( delta < 0 ) {
			cg.unknown_0x2AF6C += frametime * 1500.0f;
		}

		cg.unknown_0x2AF6C -= frametime * cg.unknown_0x2AF6C * 3;

		if ( cg.unknown_0x2AF6C > 0 ) {
			if ( delta > 0 ) {
				cg.unknown_0x2AF6C -= frametime * cg.unknown_0x2AF6C * 5;
			}
			cg.unknown_0x2AF6C -= frametime * 2;
			if ( cg.unknown_0x2AF6C < 0 ) {
				cg.unknown_0x2AF6C = 0;
				continue;
			}
		} else {
			if ( delta < 0 ) {
				cg.unknown_0x2AF6C -= frametime * cg.unknown_0x2AF6C * 5;
			}
			cg.unknown_0x2AF6C += frametime * 2;
			if ( cg.unknown_0x2AF6C > 0 ) {
				cg.unknown_0x2AF6C = 0;
				continue;
			}
		}

		if ( cg.unknown_0x2AF6C > 2000.0f ) {
			cg.unknown_0x2AF6C = 2000.0f;
		} else if ( cg.unknown_0x2AF6C < -2000.0f ) {
			cg.unknown_0x2AF6C = -2000.0f;
		}
	}

	cg.compassPointerAngle = AngleNormalize360( delta + angle );
	return;

reset:
	lastPointerTime = cg.time;
	cg.compassPointerAngle = angle;
	cg.unknown_0x2AF6C = 0;
}

/* The twelve edges of a box, indices into the eight corners CG_DebugBox
 * builds.  0x30060F50. */
static int cg_debugBoxEdges[12][2] = {
	{0, 1}, {0, 2}, {0, 4}, {1, 3}, {1, 5}, {2, 3},
	{2, 6}, {3, 7}, {4, 5}, {4, 6}, {5, 7}, {6, 7}
};

/*
=================
CG_DebugBox           0x3001A1A0

Trap 198 is the engine's CL_AddDebugLine( start, end, color, depthTest,
duration, fromServer ).

`duration` is accepted and then IGNORED -- the twelve trap calls pass a
hardcoded 0 (`push 0` at 0x3001A1EB).  It is a real parameter, not a dropped
one: the only caller, bg_misc.c's BG_CheckProneValid, pushes it on the stack and
cleans it (`push 1` 0x30006321, `call` 0x30006330, `add esp,4` 0x30006335),
which a four-argument __usercall taking eax/edx/ebx/edi would never need.  The
game DLL's G_DebugBox passes the same parameter through; this copy has the typo.
=================
*/
void CG_DebugBox( const vec3_t mins, const vec3_t maxs, const float *color, int depthTest, int duration ) {
	vec3_t points[8];
	int i;

	for ( i = 0; i < 8; i++ ) {
		points[i][0] = ( i & 1 ) ? maxs[0] : mins[0];
		points[i][1] = ( i & 2 ) ? maxs[1] : mins[1];
		points[i][2] = ( i & 4 ) ? maxs[2] : mins[2];
	}

	for ( i = 0; i < 12; i++ ) {
		trap_AddDebugLine( points[cg_debugBoxEdges[i][0]], points[cg_debugBoxEdges[i][1]],
						   color, depthTest, 0 );
	}
}

/*
=================
CG_DebugCircleEx      0x3001A220

A 16-segment circle of `radius` about org, in the plane whose normal is dir.
=================
*/
void CG_DebugCircleEx( const vec3_t dir, const vec3_t org, float radius, const float *color, int depthTest, int duration ) {
	vec3_t axis, vRight, vUp;
	vec3_t points[16];
	float angle, s, c;
	int i;

	VectorNormalize2( dir, axis );
	PerpendicularVector( vRight, axis );
	CrossProduct( axis, vRight, vUp );

	for ( i = 0; i < 16; i++ ) {
		angle = i * 0.39269909f;    // 2*M_PI/16
		s = (float)sin( angle );
		c = (float)cos( angle );
		s *= radius;
		c *= radius;
		points[i][0] = vRight[0] * c + vUp[0] * s + org[0];
		points[i][1] = vRight[1] * c + vUp[1] * s + org[1];
		points[i][2] = vRight[2] * c + vUp[2] * s + org[2];
	}

	for ( i = 0; i < 16; i++ ) {
		trap_AddDebugLine( points[i], points[( i + 1 ) & 15], color, depthTest, duration );
	}
}

/*
=================
CG_DebugArc           0x3001A3B0

15 segments of an arc in the XY plane at org[2], from startAngle to endAngle.
=================
*/
void CG_DebugArc( const vec3_t org, float radius, float startAngle, float endAngle, const float *color, int depthTest, int duration ) {
	vec3_t points[16];
	float step, angle, s, c;
	int i;

	step = ( endAngle - startAngle ) * ( 1.0f / 15 );
	if ( step < 0 ) {
		startAngle -= 360.0f;
		step = ( endAngle - startAngle ) * ( 1.0f / 15 );
	}

	for ( i = 0; i < 16; i++ ) {
		angle = ( i * step + startAngle ) * (float)M_PI * ( 1.0f / 180.0f );
		s = (float)sin( angle );
		c = (float)cos( angle );
		points[i][2] = org[2];
		points[i][0] = c * radius + org[0];
		points[i][1] = s * radius + org[1];
	}

	for ( i = 0; i < 15; i++ ) {
		trap_AddDebugLine( points[i], points[i + 1], color, depthTest, duration );
	}
}
