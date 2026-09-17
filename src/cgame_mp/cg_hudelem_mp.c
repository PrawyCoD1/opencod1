/*
 * cg_hudelem_mp.c -- drawing the script-created HUD elements.
 * (original source: cgame/cg_hudelem.c)
 *
 * cgame_mp_x86.dll 0x3001EC10 .. 0x3001F9B7, twenty-one functions.  CoD's own;
 * RTCW has no hudelem system.  This is the client half of
 * game_mp/g_hud_mp.c -- the server owns the 124-byte g_hudelem_t and copies the
 * first 112 bytes of each live one into ps.hud.current[] / ps.hud.archival[],
 * and everything below reads only those 112 bytes.
 *
 * CG_Draw2D calls CG_DrawHudElems (0x300188B3).  That collects the live
 * elements out of both playerState arrays, sorts them by sortKey, and for each
 * one builds a hudElemInfo_t -- the resolved position, size, colour, label and
 * text -- then draws a string, a shader, or a clock face.
 *
 * Four of the twenty-one have no call site in 1.1: CG_HudElemShaderDimension,
 * CG_HudElemWidth and CG_HudElemAlignY were inlined into every user (compare
 * CG_GetHudElemInfo 0x3001F2BF against CG_HudElemWidth, and CG_DrawHudElemString
 * 0x3001F49A against CG_HudElemAlignY), and CG_HudElemWidth's body is
 * byte-for-byte the block CG_GetHudElemInfo carries.
 *
 * @fidelity: likely
 */

#include <stdlib.h>
#include <string.h>

#include "cg_local.h"

/*
 * hudelem_t.type.  The same enum g_hud_mp.c carries; the client only ever
 * switches on it.
 */
typedef enum {
	HE_TYPE_FREE,
	HE_TYPE_TEXT,
	HE_TYPE_VALUE,
	HE_TYPE_SHADER,
	HE_TYPE_TIMER_DOWN,
	HE_TYPE_TIMER_UP,
	HE_TYPE_TENTHS_TIMER_DOWN,
	HE_TYPE_TENTHS_TIMER_UP,
	HE_TYPE_CLOCK_DOWN,
	HE_TYPE_CLOCK_UP
} hudElemType_t;

/* hudelem_t.alignX / .alignY. */
#define HE_ALIGN_LEFT       0
#define HE_ALIGN_CENTER     1
#define HE_ALIGN_RIGHT      2
#define HE_ALIGN_TOP        0
#define HE_ALIGN_MIDDLE     1
#define HE_ALIGN_BOTTOM     2

/* hudelem_t.font, and the engine font handle each one resolves to. */
#define HE_FONT_DEFAULT     0
#define HE_FONT_BIGFIXED    1
#define HE_FONT_SMALLFIXED  2

/* CG_GetSortedHudElems stops at 31 per array (0x3001F942); q_shared.h's
   hudElemState_t is current[31] + archival[31]. */
#define MAX_CLIENT_HUDELEMS 31

/* elem->label / elem->text are indices into the CS_HUDELEM_STRINGS block --
   cg_main_mp.c's CG_SafeTranslateHudElemString carries the same 1244. */
#define CS_HUDELEM_STRINGS  1244

#define ANGLE2SHORT( x )    ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )    ( (x) * ( 360.0f / 65536.0f ) )

/*
 * Everything CG_GetHudElemInfo resolves out of one hudelem_t, and the only
 * thing the drawing functions read.  64 bytes; CG_DrawSingleHudElem's frame
 * puts it at esp+0 with the 8192-byte consolidation buffer right behind it.
 */
typedef struct hudElemInfo_s {
	float       x;                          /* +0x00 CG_HudElemX */
	float       y;                          /* +0x04 CG_HudElemY */
	float       width;                      /* +0x08 */
	float       height;                     /* +0x0C */
	const char *label;                      /* +0x10 */
	float       labelWidth;                 /* +0x14 */
	const char *text;                       /* +0x18 */
	float       textWidth;                  /* +0x1C */
	int         font;                       /* +0x20 the engine font handle */
	float       fontScale;                  /* +0x24 */
	float       charHeight;                 /* +0x28 */
	float       charWidth;                  /* +0x2C 0 for the proportional font */
	vec4_t      color;                      /* +0x30 */
} hudElemInfo_t;
CG_ASSERT_SIZE( hudElemInfo_t, 64 );

/* cg_newDraw_mp.c. */
qboolean    CG_ServerShaderName( int index, char *buffer, int size );

static float CG_HudElemShaderWidth( const hudelem_t *elem, const hudElemInfo_t *info );
static float CG_HudElemShaderHeight( const hudelem_t *elem, const hudElemInfo_t *info );

/*
===============
CG_HudElemStringWidth

The proportional font measures through the renderer; the two fixed fonts are
just the printable length times the cell width.
===============
*/
static float CG_HudElemStringWidth( const char *string, const hudElemInfo_t *info ) {
	if ( info->charWidth != 0.0f ) {
		return trap_SE_PrintStrlen( string ) * info->charWidth;
	}
	return (float)trap_R_Text_Width( string, info->font, info->fontScale, 0 );
}

/*
===============
CG_GetHudElemTime

Milliseconds the element's timer still has to run, or has run for.  The +999
and +99 round the display up so a one-second timer shows "1" for its whole
last second.
===============
*/
static int CG_GetHudElemTime( const hudelem_t *elem ) {
	int t;

	switch ( elem->type ) {
	case HE_TYPE_TIMER_DOWN:
		t = elem->timerValue - cg.time + 999;
		break;
	case HE_TYPE_TIMER_UP:
		t = cg.time - elem->timerValue;
		break;
	case HE_TYPE_TENTHS_TIMER_DOWN:
		t = elem->timerValue - cg.time + 99;
		break;
	case HE_TYPE_TENTHS_TIMER_UP:
		t = cg.time - elem->timerValue;
		break;
	case HE_TYPE_CLOCK_DOWN:
		t = elem->timerValue - cg.time;
		break;
	case HE_TYPE_CLOCK_UP:
		t = cg.time - elem->timerValue;
		break;
	default:
		return 0;
	}

	if ( t < 0 ) {
		t = 0;
	}
	return t;
}

/*
===============
CG_HudElemTimerString
===============
*/
static const char *CG_HudElemTimerString( const hudelem_t *elem ) {
	int t;
	int hours;
	int rem;

	t = CG_GetHudElemTime( elem );

	hours = t / 1000 / 3600;
	rem = t / 1000 % 3600;

	if ( hours ) {
		return va( "%i:%02i:%02i", hours, rem / 60, rem % 60 );
	}
	return va( "%i:%02i", rem / 60, rem % 60 );
}

/*
===============
CG_HudElemTenthsTimerString
===============
*/
static const char *CG_HudElemTenthsTimerString( const hudelem_t *elem ) {
	int t;
	int hours;
	int mins;
	int rem;

	t = CG_GetHudElemTime( elem );

	hours = t / 100 / 36000;
	rem = t / 100 % 36000;
	mins = rem / 600;
	rem %= 600;

	if ( hours ) {
		return va( "%i:%02i:%02i.%i", hours, mins, rem / 10, rem % 10 );
	}
	return va( "%i:%02i.%i", mins, rem / 10, rem % 10 );
}

/*
===============
CG_HudElemShaderDimension

A zero width or height on a shader element means "one character cell".  No call
site in 1.1 -- inlined into both shader-dimension functions.
===============
*/
static float CG_HudElemShaderDimension( int dimension, const hudElemInfo_t *info ) {
	if ( dimension ) {
		return (float)dimension;
	}
	return info->charHeight;
}

/*
===============
CG_HudElemShaderWidth
===============
*/
static float CG_HudElemShaderWidth( const hudelem_t *elem, const hudElemInfo_t *info ) {
	float   w;
	float   from;
	int     t;

	w = CG_HudElemShaderDimension( elem->width, info );

	if ( elem->scaleTime > 0 ) {
		t = cg.time - elem->scaleStartTime;
		if ( t < elem->scaleTime ) {
			from = CG_HudElemShaderDimension( elem->scaleFromWidth, info );
			return ( w - from ) * ( 1.0f / elem->scaleTime * t ) + from;
		}
	}

	return w;
}

/*
===============
CG_HudElemShaderHeight
===============
*/
static float CG_HudElemShaderHeight( const hudelem_t *elem, const hudElemInfo_t *info ) {
	float   h;
	float   from;
	int     t;

	h = CG_HudElemShaderDimension( elem->height, info );

	if ( elem->scaleTime > 0 ) {
		t = cg.time - elem->scaleStartTime;
		if ( t < elem->scaleTime ) {
			from = CG_HudElemShaderDimension( elem->scaleFromHeight, info );
			return ( h - from ) * ( 1.0f / elem->scaleTime * t ) + from;
		}
	}

	return h;
}

/*
===============
CG_HudElemWidth

No call site in 1.1: CG_GetHudElemInfo carries this body inlined (0x3001F2A8).
===============
*/
static float CG_HudElemWidth( const hudelem_t *elem, const hudElemInfo_t *info ) {
	switch ( elem->type ) {
	case HE_TYPE_TEXT:
	case HE_TYPE_VALUE:
	case HE_TYPE_TIMER_DOWN:
	case HE_TYPE_TIMER_UP:
	case HE_TYPE_TENTHS_TIMER_DOWN:
	case HE_TYPE_TENTHS_TIMER_UP:
		return info->textWidth + info->labelWidth;

	case HE_TYPE_SHADER:
	case HE_TYPE_CLOCK_DOWN:
	case HE_TYPE_CLOCK_UP:
		return CG_HudElemShaderWidth( elem, info ) + info->labelWidth;

	default:
		return 0.0f;
	}
}

/*
===============
CG_HudElemHeight

A labelled element is never shorter than one line of its own font.
===============
*/
static float CG_HudElemHeight( const hudelem_t *elem, const hudElemInfo_t *info ) {
	float h;

	switch ( elem->type ) {
	case HE_TYPE_TEXT:
	case HE_TYPE_VALUE:
	case HE_TYPE_TIMER_DOWN:
	case HE_TYPE_TIMER_UP:
	case HE_TYPE_TENTHS_TIMER_DOWN:
	case HE_TYPE_TENTHS_TIMER_UP:
		h = info->charHeight;
		break;

	case HE_TYPE_SHADER:
	case HE_TYPE_CLOCK_DOWN:
	case HE_TYPE_CLOCK_UP:
		h = CG_HudElemShaderHeight( elem, info );
		break;

	default:
		return 0.0f;
	}

	if ( info->label && h < info->charHeight ) {
		h = info->charHeight;
	}
	return h;
}

/*
===============
CG_HudElemX

moveOverTime lerps x from moveFromX in integer units before the alignment is
applied.
===============
*/
static float CG_HudElemX( const hudelem_t *elem, const hudElemInfo_t *info ) {
	float   x;
	int     t;

	x = (float)elem->x;

	if ( elem->moveTime > 0 ) {
		t = cg.time - elem->moveStartTime;
		if ( t < elem->moveTime ) {
			x = (float)( ( elem->x - elem->moveFromX ) * t ) * ( 1.0f / elem->moveTime )
				+ elem->moveFromX;
		}
	}

	switch ( elem->alignX ) {
	case HE_ALIGN_CENTER:
		return x - info->width * 0.5f;
	case HE_ALIGN_RIGHT:
		return x - info->width;
	default:
		return x;
	}
}

/*
===============
CG_HudElemY
===============
*/
static float CG_HudElemY( const hudelem_t *elem, const hudElemInfo_t *info ) {
	float   y;
	int     t;

	y = (float)elem->y;

	if ( elem->moveTime > 0 ) {
		t = cg.time - elem->moveStartTime;
		if ( t < elem->moveTime ) {
			y = (float)( ( elem->y - elem->moveFromY ) * t ) * ( 1.0f / elem->moveTime )
				+ elem->moveFromY;
		}
	}

	switch ( elem->alignY ) {
	case HE_ALIGN_MIDDLE:
		return y - info->height * 0.5f;
	case HE_ALIGN_BOTTOM:
		return y - info->height;
	default:
		return y;
	}
}

/*
===============
CG_ConsolidateHudElemText

An element with both a label and a text draws as one string: the label with its
"%s" replaced by the text, or the text appended if the label has no "%s".  The
result becomes info->text and the label is emptied, so the caller draws once.

The first loop indexes the destination with the label position rather than the
output length -- the two are equal until the "%s" is reached, and the loop stops
there, so it makes no difference.
===============
*/
static void CG_ConsolidateHudElemText( hudElemInfo_t *info, int size, char *buffer ) {
	int     len;
	int     labelPos;
	int     textPos;
	char    c;

	len = 0;
	labelPos = 0;

	while ( len < size - 1 ) {
		c = info->label[labelPos];
		if ( !c ) {
			break;
		}
		if ( c == '%' && info->label[labelPos + 1] == 's' ) {
			labelPos += 2;
			break;
		}
		len++;
		buffer[labelPos] = c;
		labelPos++;
	}

	textPos = 0;
	while ( len < size - 1 ) {
		c = info->text[textPos];
		if ( !c ) {
			break;
		}
		buffer[len] = c;
		len++;
		textPos++;
	}

	while ( len < size - 1 ) {
		c = info->label[labelPos];
		if ( !c ) {
			break;
		}
		buffer[len] = c;
		len++;
		labelPos++;
	}

	buffer[len] = 0;

	info->textWidth = info->textWidth + info->labelWidth;
	info->text = buffer;
	info->label = "";
	info->labelWidth = 0;
}

/*
===============
CG_GetHudElemInfo

Resolves one networked hudelem_t into everything the drawing functions need.
===============
*/
static void CG_GetHudElemInfo( hudElemInfo_t *info, const hudelem_t *elem,
							   char *buffer, int size ) {
	int     t;
	float   frac;
	int     i;

	switch ( elem->font ) {
	case HE_FONT_DEFAULT:
		info->font = 0;
		info->fontScale = elem->fontScale * 0.25f;
		info->charHeight = (float)trap_R_Text_Height( 0, info->fontScale );
		info->charWidth = 0.0f;
		break;
	case HE_FONT_BIGFIXED:
		info->font = 4;
		info->fontScale = elem->fontScale * ( 1.0f / 3.0f );
		info->charHeight = 16.0f;
		info->charWidth = 16.0f;
		break;
	case HE_FONT_SMALLFIXED:
		info->font = 5;
		info->fontScale = elem->fontScale * ( 1.0f / 3.0f );
		info->charHeight = 16.0f;
		info->charWidth = 8.0f;
		break;
	default:
		break;
	}

	info->label = CG_SafeTranslateHudElemString( elem->label );

	switch ( elem->type ) {
	case HE_TYPE_TEXT:
		if ( elem->text ) {
			info->text = CG_SafeTranslateHudElemString( elem->text );
		} else {
			info->text = "";
		}
		break;
	case HE_TYPE_TIMER_DOWN:
	case HE_TYPE_TIMER_UP:
		info->text = CG_HudElemTimerString( elem );
		break;
	case HE_TYPE_TENTHS_TIMER_DOWN:
	case HE_TYPE_TENTHS_TIMER_UP:
		info->text = CG_HudElemTenthsTimerString( elem );
		break;
	case HE_TYPE_VALUE:
		info->text = va( "%g", elem->value );
		break;
	default:
		info->text = "";
		break;
	}

	if ( info->label[0] && info->text[0] ) {
		CG_ConsolidateHudElemText( info, size, buffer );
	}

	if ( info->label[0] ) {
		info->labelWidth = CG_HudElemStringWidth( info->label, info );
	} else {
		info->labelWidth = 0.0f;
	}

	if ( info->text[0] ) {
		info->textWidth = CG_HudElemStringWidth( info->text, info );
	} else {
		info->textWidth = 0.0f;
	}

	info->width = CG_HudElemWidth( elem, info );
	info->height = CG_HudElemHeight( elem, info );
	info->x = CG_HudElemX( elem, info );
	info->y = CG_HudElemY( elem, info );

	if ( elem->fadeTime > 0 ) {
		t = cg.time - elem->fadeStartTime;
		if ( t < elem->fadeTime ) {
			frac = (float)t / elem->fadeTime;
			for ( i = 0; i < 4; i++ ) {
				info->color[i] = ( ( elem->color[i] - elem->fromColor[i] ) * frac
								   + elem->fromColor[i] ) * ( 1.0f / 255.0f );
			}
			return;
		}
	}

	for ( i = 0; i < 4; i++ ) {
		info->color[i] = elem->color[i] * ( 1.0f / 255.0f );
	}
}

/*
===============
CG_HudElemAlignY

The vertical placement of one drawn item of height h inside the element's box.
No call site in 1.1 -- inlined into CG_DrawHudElemString (0x3001F49A),
CG_DrawHudElemClock (0x3001F650) and CG_DrawHudElemShader (0x3001F748).
===============
*/
static float CG_HudElemAlignY( const hudelem_t *elem, const hudElemInfo_t *info, float h ) {
	switch ( elem->alignY ) {
	case HE_ALIGN_TOP:
		return info->y;
	case HE_ALIGN_MIDDLE:
		return ( info->height - h ) * 0.5f + info->y;
	case HE_ALIGN_BOTTOM:
		return info->height + info->y - h;
	default:
		return 0.0f;
	}
}

/*
===============
CG_DrawHudElemString

trap_R_Text_Paint takes the baseline, which is one character cell below the top
of the line.
===============
*/
static void CG_DrawHudElemString( const hudElemInfo_t *info, const hudelem_t *elem,
								  const char *string ) {
	float y;

	y = CG_HudElemAlignY( elem, info, info->charHeight );

	trap_R_Text_Paint( info->x, y + info->charHeight, info->font, info->fontScale,
					   info->color, string, info->charWidth, 0, 3 );
}

/*
===============
CG_DrawHudElemClock

Draws the element's material, then the same name with "Needle" appended rotated
to the fraction of the clock that is left.  A zero rotationPeriodMs means the
needle sweeps once a minute.
===============
*/
static void CG_DrawHudElemClock( const hudElemInfo_t *info, const hudelem_t *elem ) {
	char        name[64];
	qhandle_t   hFace;
	qhandle_t   hNeedle;
	int         t;
	float       angle;
	float       w, h, y;

	if ( !CG_ServerShaderName( elem->materialIndex, name, sizeof( name ) - 6 ) ) {
		return;
	}

	hFace = trap_R_RegisterShaderNoMip( name, 5 );
	strcat( name, "Needle" );
	hNeedle = trap_R_RegisterShaderNoMip( name, 5 );

	t = CG_GetHudElemTime( elem );
	if ( elem->rotationPeriodMs ) {
		angle = SHORT2ANGLE( ANGLE2SHORT( (float)t * 360.0f / elem->rotationPeriodMs ) );
	} else {
		/* the 360/60000 and the ANGLE2SHORT scale fold into one constant here */
		angle = SHORT2ANGLE( ANGLE2SHORT( (float)t * 360.0f / 60000.0f ) );
	}

	w = CG_HudElemShaderWidth( elem, info );
	h = CG_HudElemShaderHeight( elem, info );
	y = CG_HudElemAlignY( elem, info, h );

	trap_R_SetColor( info->color );
	CG_DrawPic( info->x, y, w, h, hFace );
	CG_DrawRotatedPic( info->x, y, w, h, angle, hNeedle );
	trap_R_SetColor( NULL );
}

/*
===============
CG_DrawHudElemShader
===============
*/
static void CG_DrawHudElemShader( const hudElemInfo_t *info, const hudelem_t *elem ) {
	char        name[64];
	qhandle_t   hShader;
	float       w, h, y;

	if ( !CG_ServerShaderName( elem->materialIndex, name, sizeof( name ) ) ) {
		return;
	}

	hShader = trap_R_RegisterShader( name, 5 );

	w = CG_HudElemShaderWidth( elem, info );
	h = CG_HudElemShaderHeight( elem, info );
	y = CG_HudElemAlignY( elem, info, h );

	trap_R_SetColor( info->color );
	CG_DrawPic( info->x, y, w, h, hShader );
	trap_R_SetColor( NULL );
}

/*
===============
CG_DrawSingleHudElem
===============
*/
static void CG_DrawSingleHudElem( const hudelem_t *elem ) {
	hudElemInfo_t   info;
	char            buffer[8192];

	CG_GetHudElemInfo( &info, elem, buffer, sizeof( buffer ) );

	if ( info.label[0] ) {
		CG_DrawHudElemString( &info, elem, info.label );
		info.x = info.x + info.labelWidth;
	}

	switch ( elem->type ) {
	case HE_TYPE_TEXT:
	case HE_TYPE_VALUE:
	case HE_TYPE_TIMER_DOWN:
	case HE_TYPE_TIMER_UP:
	case HE_TYPE_TENTHS_TIMER_DOWN:
	case HE_TYPE_TENTHS_TIMER_UP:
		if ( info.text[0] ) {
			CG_DrawHudElemString( &info, elem, info.text );
		}
		break;

	case HE_TYPE_CLOCK_DOWN:
	case HE_TYPE_CLOCK_UP:
		CG_DrawHudElemClock( &info, elem );
		break;

	case HE_TYPE_SHADER:
		CG_DrawHudElemShader( &info, elem );
		break;

	default:
		break;
	}
}

/*
===============
compare_hudelems
===============
*/
static int compare_hudelems( const void *a, const void *b ) {
	float d;

	d = ( *(const hudelem_t **)a )->sortKey - ( *(const hudelem_t **)b )->sortKey;

	if ( d < 0.0f ) {
		return -1;
	}
	if ( d > 0.0f ) {
		return 1;
	}
	return 0;
}

/*
===============
CG_GetSortedHudElems

Both playerState arrays, archival first, each stopping at the first free slot,
then sorted by sortKey.
===============
*/
static int CG_GetSortedHudElems( const hudelem_t **list ) {
	const hudelem_t *elem;
	int             count;
	int             i;

	count = 0;

	elem = cg.snap->ps.hud.archival;
	for ( i = 0; i < MAX_CLIENT_HUDELEMS; i++, elem++ ) {
		if ( !elem->type ) {
			break;
		}
		list[count] = elem;
		count++;
	}

	elem = cg.snap->ps.hud.current;
	for ( i = 0; i < MAX_CLIENT_HUDELEMS; i++, elem++ ) {
		if ( !elem->type ) {
			break;
		}
		list[count] = elem;
		count++;
	}

	qsort( list, count, sizeof( list[0] ), compare_hudelems );

	return count;
}

/*
===============
CG_DrawHudElems
===============
*/
void CG_DrawHudElems( void ) {
	const hudelem_t *list[2 * MAX_CLIENT_HUDELEMS];
	int             count;
	int             i;

	count = CG_GetSortedHudElems( list );

	for ( i = 0; i < count; i++ ) {
		CG_DrawSingleHudElem( list[i] );
	}
}
