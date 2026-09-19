/*
 * cg_draw_mp.c -- draw all of the graphical elements during active (after
 * loading) gameplay.  (original source: cgame/cg_draw.c)
 *
 * cgame_mp_x86.dll 0x30014400 .. 0x30018CB6, 48 functions in binary order.
 * RTCW's cgame/cg_draw.c is the ancestor.  CoD replaced the whole 2D layer:
 *
 *   - SURVIVING, essentially unchanged: CG_DrawFieldWidth, CG_DrawField (both
 *     dead in 1.1 -- nothing calls them), CG_DrawSnapshot, CG_DrawFPS,
 *     CG_DrawUpperRight, CG_AddLagometerFrameInfo, CG_AddLagometerSnapshotInfo,
 *     CG_DrawDisconnect, CG_DrawLagometer, CG_CenterPrint,
 *     CG_PriorityCenterPrint, CG_DrawCenterString, CG_ScanForCrosshairEntity,
 *     CG_DrawFlashFade, CG_DrawFlashDamage, CG_DrawTimedMenus, CG_ScreenFade,
 *     CG_Fade, CG_UpdateCameraShake, CG_StartShakeCamera, CG_ShakeCamera,
 *     CG_DrawSpectator, CG_DrawVote, CG_DrawIntermission,
 *     CG_DrawSpectatorMessage, CG_DrawFollow, CG_Draw2D, CG_DrawActive.
 *   - GONE: the bitmap font (every string here goes out through
 *     trap_R_Text_Paint / trap_R_Text_Width), the whole hardcoded status bar,
 *     CG_DrawTimer, CG_DrawTeamOverlay, CG_DrawTeamInfo, CG_DrawPickupItem,
 *     CG_DrawNotify, CG_DrawDynamiteStatus, CG_ActivateLimboMenu,
 *     CG_DrawLimboMessage, CG_DrawWarmup, CG_ObjectivePrint,
 *     CG_DrawObjectiveInfo/Icons, CG_DrawCompass*, CG_Draw3DModel, CG_DrawHead,
 *     CG_DrawFlagModel, CG_DrawKeyModel, CG_DrawFlashZoomTransition,
 *     CG_DrawFlashFire, CG_DrawFlashLightning, CG_DrawFlashBlend*,
 *     CG_DrawBinocReticle.  The HUD is hudelems (cg_hudelem.c) and the menu
 *     system now.
 *   - NEW in CoD: CG_DrawScriptUsage, CG_DrawChatMessages,
 *     CG_CalcCrosshairPosition, CG_GetWeapReticleZoom, CG_DrawTurretCrossHair,
 *     CG_CheckForCursorHints, CG_DrawDamageDirectionIndicators,
 *     CG_DrawSoundOverlay, CG_DrawGameMessages, CG_DrawBoldGameMessages,
 *     CG_DrawMiniConsole, CG_DrawSubtitles, CG_DrawSay,
 *     CG_DrawPerformanceWarnings, CG_DrawDebugOverlays,
 *     CG_DrawSavedScreenBlend.
 *   - CG_DrawWeapReticle and CG_DrawCrosshair are RTCW functions rewritten from
 *     scratch around the weaponInfo_t reticle fields.
 *
 * Addresses in comments are addresses in that DLL.
 *
 * @fidelity: likely
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "cg_local.h"

/* Declarations no header carries. */

/* universal/q_shared.c, universal/com_math.c. */
extern vec3_t vec3_origin;
extern vec4_t colorWhite;                       /* 0x30060804 */

#ifndef COLOR_RED
#define COLOR_RED       '1'
#define COLOR_GREEN     '2'
#define COLOR_YELLOW    '3'
#define COLOR_BLUE      '4'
#endif
#ifndef ColorIndex
#define ColorIndex( c ) ( ( (c) - '0' ) & 7 )
#endif

/* game_mp/bg_weapon.c. */
float       BG_GetMinSpreadForWeapon( int time, const playerState_t *ps, int weaponIndex );

/* other cgame_mp units. */
qboolean    CG_DrawScoreboard( void );
void        CG_DrawScoreboard_GetTeamColor( vec4_t color, int team );
void        CG_DrawHudElems( void );
void        CG_DrawWeaponSelect( void );
/* Returns the sound length; `name` comes in through eax, the rest on the
   stack. */
int         CG_PlaySoundAliasByName( const char *name, int entnum, const vec3_t origin );

/* ui_mp/ui_shared.c and ui_mp/ui_players.c. */
void       *Menus_FindByName( const char *p );
void        Menus_Close( void *menu );
void        Menu_PaintAll( void );
void        Controls_GetConfig( void );
/* ui_shared.c 0x30046940: ( name, out ). */
int         GetKeyBindingLocalizedString( const char *name, char **out );

/*
 * cg_t fields cg_local.h has no name for; each one is inside a range that
 * header spells as a byte hole.
 */
/* cg+0x2AA40 -- the gun's aim angles, [PITCH,YAW]; CG_CalcCrosshairPosition
   0x30015D73/0x30015D79 runs AngleVectors' forward on them. */
#define CG_GUN_ANGLES       ( (float *)( cg.unknown_0x2AA40 ) )
/* cg+0x2AA9C -- RTCW's cg.cameraMode (CG_Draw2D 0x3001882B). */
#define CG_CAMERA_MODE      ( *(int *)( cg.unknown_0x2AA40 + 0x5C ) )
/* cg+0x27318 -- nonzero picks the weapon's "in" fractions over its "out"
   fractions (CG_GetWeapReticleZoom 0x30015F60, CG_DrawCrosshair 0x300168C9). */
#define CG_ADS_ZOOMING_IN   ( *(int *)( cg.unknown_0x27314 + 4 ) )
/* cg+0x2A87C -- the grenadeTimeLeft the last pulse sound was played for
   (CG_DrawCrosshair 0x30016AB6). */
#define CG_LAST_GRENADE_PULSE   ( *(int *)( cg.unknown_0x2A87C ) )
/* cg+0x2A798 / cg+0x2A79C -- CG_DrawCrosshairNames' "flash this name" latch
   and its 0..100 countdown (0x30017050, 0x30017058). */
#define CG_NAMEFLASH_CLIENT ( cg.unknown_0x2A798 )
#define CG_NAMEFLASH_VALUE  ( cg.unknown_0x2A79C )

/* Neither header carries these. */
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
#ifndef STAT_MINUS
#define STAT_MINUS      10          /* num frame for '-' (0x3001464F) */
#endif
#ifndef CMD_BACKUP
#define CMD_BACKUP      64          /* CG_DrawDisconnect's `sub eax, 3Fh` */
#endif
#ifndef SNAPFLAG_RATE_DELAYED
#define SNAPFLAG_RATE_DELAYED   1
#endif
#ifndef CS_SERVERINFO
#define CS_SERVERINFO   0
#endif
/* CG_DrawTeamBackground's two cases; no header carries the team enum. */
#ifndef TEAM_AXIS
#define TEAM_AXIS       1
#define TEAM_ALLIES     2
#endif
/* vmMain's CG_DRAW_ACTIVE_FRAME stereoView argument; spelled with plain ints
   the way cg_local.h already spells CG_DrawActiveFrame's. */
#ifndef STEREO_CENTER
#define STEREO_CENTER   0
#define STEREO_LEFT     1
#define STEREO_RIGHT    2
#endif

/* The damage-indicator quad's texture coordinates, 0x30060FB4. */
static float cg_damageIconTexCoords[8] = { 1, 1,  0, 1,  0, 0,  1, 0 };

static void CG_DrawTurretCrossHair( void );
static void CG_DrawDisconnect( void );

/*
================
CG_DrawFieldWidth     0x30014400

How wide CG_DrawField's output would be.  Straight RTCW; nothing in 1.1 calls
it.  `width` comes in through ecx.
================
*/
int CG_DrawFieldWidth( int x, int y, int width, int value, int charWidth, int charHeight ) {
	char num[16], *ptr;
	int l;
	int frame;
	int totalwidth = 0;

	if ( width < 1 ) {
		return 0;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf( num, sizeof( num ), "%i", value );
	l = strlen( num );
	if ( l > width ) {
		l = width;
	}

	ptr = num;
	while ( *ptr && l )
	{
		if ( *ptr == '-' ) {
			frame = STAT_MINUS;
		} else {
			frame = *ptr - '0';
		}

		totalwidth += charWidth;
		ptr++;
		l--;
	}

	return totalwidth;
}

/*
================
CG_DrawField          0x30014530

Straight RTCW again, and just as dead.  CG_DrawPic is inlined at the one call
site (0x300146F4).
================
*/
int CG_DrawField( int x, int y, int width, int value, int charWidth, int charHeight, qboolean dodrawpic, qboolean leftAlign ) {
	char num[16], *ptr;
	int l;
	int frame;
	int startx;

	if ( width < 1 ) {
		return 0;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf( num, sizeof( num ), "%i", value );
	l = strlen( num );
	if ( l > width ) {
		l = width;
	}

	if ( !leftAlign ) {
		x -= 2 + charWidth * ( l );
	}

	startx = x;

	ptr = num;
	while ( *ptr && l )
	{
		if ( *ptr == '-' ) {
			frame = STAT_MINUS;
		} else {
			frame = *ptr - '0';
		}

		if ( dodrawpic ) {
			CG_DrawPic( (float)x, (float)y, (float)charWidth, (float)charHeight,
						cgs.media.numberShaders[frame] );
		}
		x += charWidth;
		ptr++;
		l--;
	}

	return startx;
}

/*
================
CG_DrawTeamBackground 0x30014740

RTCW's, with cgs.media.teamStatusBar replaced by the hud colour bar.  `team`
comes in through eax.  CG_DrawPic is inlined (0x300147E1).  The rectangle is
FLOATS, not RTCW's ints: 0x30014796 onward are `fmul dword ptr [esp+arg_*]`.
================
*/
void CG_DrawTeamBackground( float x, float y, float w, float h, float alpha, int team ) {
	vec4_t hcolor;

	hcolor[3] = alpha;
	if ( team == TEAM_AXIS ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if ( team == TEAM_ALLIES ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.hudColorBar );
	trap_R_SetColor( NULL );
}

/*
=================
CG_DrawScriptUsage    0x30014800

cg_drawScriptUsage.  The three traps are no-ops in retail (205, 206) and one
global dword (207), so all three lines read zero in a shipped build.
=================
*/
void CG_DrawScriptUsage( void ) {
	trap_R_Text_Paint( 480, 92.8f, 5, 0.33333334f, colorWhite,
					   va( "num vars:    %d", trap_syscall_0xCD() ), 8, 0, 3 );
	trap_R_Text_Paint( 480, 108.8f, 5, 0.33333334f, colorWhite,
					   va( "num threads: %d", trap_syscall_0xCE() ), 8, 0, 3 );
	trap_R_Text_Paint( 480, 124.8f, 5, 0.33333334f, colorWhite,
					   va( "string usage: %d", trap_syscall_0xCF() ), 8, 0, 3 );
}

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

#define UPPERRIGHT_X 620

/*
==================
CG_DrawSnapshot       0x30014990
==================
*/
static float CG_DrawSnapshot( float y ) {
	char        *s;
	float w;        /* retail: fild; fmul f:16; fsubr f:620 (0x300149D4) */

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime,
			cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = (float)Q_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( UPPERRIGHT_X - w, y + 2, s, 1.0F );
	y += BIGCHAR_HEIGHT + 4;

	return y;
}

/*
==================
CG_DrawFPS            0x30014A00

RTCW's four-frame average became a 32-frame one that also prints the min/max
frame rate and the mean absolute deviation, and cg_drawFPS 2 adds the renderer's
own statistics block underneath.
==================
*/
#define FPS_FRAMES  32

/* trap 107 (re slot +0xA0) fills this.  The names are what CG_DrawFPS prints
   each dword as; the three at +0x10..+0x18 are only ever printed as megabytes. */
typedef struct {
	int tris;                       /* +0x00 */
	int totalTris;                  /* +0x04 */
	int verts;                      /* +0x08 */
	int prims;                      /* +0x0C */
	int memA;                       /* +0x10 */
	int memB;                       /* +0x14 */
	int memC;                       /* +0x18 */
	int ents;                       /* +0x1C */
	float dc;                       /* +0x20 */
} rendererStats_t;

static int previousTimes[FPS_FRAMES];   /* 0x30093A60 */
static int fadeLastTime;                /* 0x30093AE0 -- CG_DrawFlashFade's */
static int index;                       /* 0x30093AE4 */
static int previous;                    /* 0x30093AE8 */
static rendererStats_t r_stats;         /* 0x30093AEC */

static float CG_DrawFPS( float y ) {
	char        *s;
	float w;
	int i, total;
	int minTime, maxTime;
	int fps;
	float variance;
	int t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index < FPS_FRAMES ) {
		return y;
	}

	// average multiple frames together to smooth changes out a bit
	total = 0;
	minTime = 0x7FFFFFFF;
	maxTime = 0;
	for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
		total += previousTimes[i];
		if ( minTime > previousTimes[i] ) {
			minTime = previousTimes[i];
		}
		if ( maxTime < previousTimes[i] ) {
			maxTime = previousTimes[i];
		}
	}

	variance = 0;
	for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
		variance += (float)fabs( previousTimes[i] - total * ( 1.0f / FPS_FRAMES ) );
	}
	variance *= ( 1.0f / FPS_FRAMES );

	if ( !total ) {
		total = 1;
	}
	if ( minTime <= 0 ) {
		minTime = 1;
	}
	fps = Q_ftol( 1000 * FPS_FRAMES / (float)total );

	s = va( "%ifps(%i-%i,%i)", fps, Q_ftol( 1000 / (float)maxTime ),
			Q_ftol( 1000 / (float)minTime ), Q_ftol( variance ) );
	w = (float)Q_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( UPPERRIGHT_X - w, y + 2, s, 1.0F );
	y += BIGCHAR_HEIGHT + 4;

	if ( cg_drawFPS.integer <= 1 ) {
		return y;
	}

	trap_syscall_0x6B( (int)&r_stats );

	s = va( "%i/%i tris", r_stats.tris / 3, r_stats.totalTris / 3 );
	w = (float)Q_DrawStrlen( s ) * SMALLCHAR_WIDTH;
	CG_DrawSmallString( UPPERRIGHT_X - w, y + 1, s, 1.0F );
	y += SMALLCHAR_HEIGHT;

	s = va( "%i vert", r_stats.verts );
	w = (float)Q_DrawStrlen( s ) * SMALLCHAR_WIDTH;
	CG_DrawSmallString( UPPERRIGHT_X - w, y + 1, s, 1.0F );
	y += SMALLCHAR_HEIGHT;

	s = va( "%i prim", r_stats.prims );
	w = (float)Q_DrawStrlen( s ) * SMALLCHAR_WIDTH;
	CG_DrawSmallString( UPPERRIGHT_X - w, y + 1, s, 1.0F );
	y += SMALLCHAR_HEIGHT;

	s = va( "%i ents", r_stats.ents );
	w = (float)Q_DrawStrlen( s ) * SMALLCHAR_WIDTH;
	CG_DrawSmallString( UPPERRIGHT_X - w, y + 1, s, 1.0F );
	y += SMALLCHAR_HEIGHT;

	s = va( "%.2f/%.2f/%.2f mb  ", r_stats.memC * ( 1.0f / ( 1024 * 1024 ) ),
			( r_stats.memA - r_stats.memB ) * ( 1.0f / ( 1024 * 1024 ) ),
			r_stats.memA * ( 1.0f / ( 1024 * 1024 ) ) );
	w = (float)Q_DrawStrlen( s ) * SMALLCHAR_WIDTH;
	CG_DrawSmallString( UPPERRIGHT_X - w, y + 1, s, 1.0F );
	y += SMALLCHAR_HEIGHT;

	if ( r_stats.dc == 0 ) {
		return y;
	}

	s = va( "%.2f dc  ", r_stats.dc );
	w = (float)Q_DrawStrlen( s ) * SMALLCHAR_WIDTH;
	CG_DrawSmallString( UPPERRIGHT_X - w, y + 1, s, 1.0F );
	y += SMALLCHAR_HEIGHT;

	return y;
}

/*
=====================
CG_DrawUpperRight     0x30015070
=====================
*/
static void CG_DrawUpperRight( void ) {
	float y;

	y = 0;

	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}
	if ( cg_drawFPS.integer ) {
		y = CG_DrawFPS( y );
	}
}

/*
=================
CG_DrawChatMessages   0x300150B0

CoD's replacement for RTCW's CG_DrawTeamInfo.  Each line gets a coloured bar
behind it, and the "^7" CG_AddToTeamChat puts after the speaker's name splits
the line into a team-coloured name and a white message.
=================
*/
static void CG_DrawChatMessages( void ) {
	int chatHeight;
	int i;
	float alpha;
	float y;
	char *s;
	char *sep;
	int w;
	vec4_t hcolor;

	chatHeight = cg_chatHeight.integer;
	if ( chatHeight >= TEAMCHAT_HEIGHT ) {
		chatHeight = TEAMCHAT_HEIGHT;
	} else if ( chatHeight <= 0 ) {
		return;
	}

	if ( cgs.teamLastChatPos == cgs.teamChatPos ) {
		return;
	}

	if ( cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_chatTime.integer ) {
		cgs.teamLastChatPos++;
	}

	for ( i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i-- ) {
		if ( !bg_clientinfo[cg.snap->ps.clientNum].infoValid ) {
			continue;
		}

		alpha = cg_chatTime.value - ( cg.time - cgs.teamChatMsgTimes[i % chatHeight] );
		if ( alpha > 200.0f ) {
			alpha = 1.0f;
		} else {
			alpha /= 200;
			if ( alpha <= 0 ) {
				continue;
			}
		}

		s = cgs.teamChatMsgs[i % chatHeight];
		y = (float)( ( i - cgs.teamChatPos ) * 10 + 84 );

		CG_DrawScoreboard_GetTeamColor( hcolor, bg_clientinfo[cg.snap->ps.clientNum].team );
		hcolor[0] *= 0.25f;
		hcolor[1] *= 0.25f;
		hcolor[2] *= 0.25f;
		hcolor[3] = alpha * 0.6f;

		w = trap_R_Text_Width( s, 0, 10 / 48.0f, 0 );
		trap_R_SetColor( hcolor );
		CG_DrawPic( 0, y, (float)( w + 24 ), 10, cgs.media.hudColorBar );

		hcolor[3] = alpha;
		y = (float)( ( i - cgs.teamChatPos ) * 10 + 93 );

		sep = strstr( s, "^7" );
		if ( sep && sep != s ) {
			/* the name half, in the speaker's team colour */
			CG_DrawScoreboard_GetTeamColor( hcolor, bg_clientinfo[cg.snap->ps.clientNum].team );
			trap_R_Text_Paint( 8, y, 0, 10 / 48.0f, hcolor, s, 0, sep - s, 3 );
			w = trap_R_Text_Width( s, 0, 10 / 48.0f, sep - s );
			hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
			trap_R_Text_Paint( w + 8.0f, y, 0, 10 / 48.0f, hcolor, sep, 0, 0, 3 );
		} else {
			hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
			trap_R_Text_Paint( 8, y, 0, 10 / 48.0f, hcolor, s, 0, 0, 3 );
		}
	}

	trap_R_SetColor( NULL );
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define LAG_SAMPLES     128

typedef struct {
	int frameSamples[LAG_SAMPLES];          /* 0x3019E1E0 */
	int frameCount;                         /* 0x3019E3E0 */
	int snapshotFlags[LAG_SAMPLES];         /* 0x3019E3E4 */
	int snapshotSamples[LAG_SAMPLES];       /* 0x3019E5E4 */
	int snapshotCount;                      /* 0x3019E7E4 */
} lagometer_t;

lagometer_t lagometer;

/*
==============
CG_AddLagometerFrameInfo      0x300153D0

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1 ) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo   0x30015400

Each time a snapshot is received, log its ping time and the number of snapshots
that were dropped before it.  Pass NULL for a dropped packet.  `snap` comes in
through eax.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1 ) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect             0x30015450

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	int cmdNum;
	usercmd_t cmd;
	const char      *s;
	float w;        /* retail: fild after trap_R_Text_Width, ftol before the subtract (0x300154F8) */
	vec4_t color = { 1, 1, 1, 1 };

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		 || cmd.serverTime > cg.time ) {    // special check for map_restart
		return;
	}

	// also add text in center of screen
	s = CG_SafeTranslateString_Internal( "cgame", "CGAME_CONNECTIONINTERUPTED" );
	w = trap_R_Text_Width( s, 0, 16 / 48.0f, 0 );
	trap_R_Text_Paint( (float)( ( 640 - (int)w ) / 2 ), 100, 0, 16 / 48.0f, color, s, 0, 0, 3 );

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	CG_DrawPic( 296, 416, 48, 48, trap_R_RegisterShader( "gfx/2d/net.tga", 5 ) );
}


#define MAX_LAGOMETER_PING  900
#define MAX_LAGOMETER_RANGE 300

/*
==============
CG_DrawLagometer              0x30015580
==============
*/
static void CG_DrawLagometer( void ) {
	int a, i;
	float v;
	float ax, ay, aw, ah, mid, range;
	int color;
	float vscale;

	if ( !cg_lagometer.integer || cgs.localServer ) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
	trap_R_SetColor( NULL );
	CG_DrawPic( 585, 340, 48, 48, cgs.media.lagometerShader );

	ax = 585;
	ay = 340;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - a - 1 ) & ( LAG_SAMPLES - 1 );
		v = (float)lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex( COLOR_YELLOW )] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex( COLOR_BLUE )] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - a - 1 ) & ( LAG_SAMPLES - 1 );
		v = (float)lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;      // YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex( COLOR_YELLOW )] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex( COLOR_GREEN )] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;          // RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex( COLOR_RED )] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	if ( cg_nopredict.integer || g_synchronousClients.integer ) {
		CG_DrawBigString( (float)(int)ax, (float)(int)ay, "snc", 1.0 );
	}

	CG_DrawDisconnect();
}


/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

#define CP_LINEWIDTH 75

/*
==============
CG_PriorityCenterPrint        0x300159E0

Called for important messages that should stay in the center of the screen for a
few moments.  `priority` comes in through eax; `y` and `charWidth` are floats
here and are stored as ints.
==============
*/
void CG_PriorityCenterPrint( const char *str, float y, float charWidth, int priority ) {
	char    *s;
	int len;
	qboolean neednewline;
	int c;

	// don't draw if this print message is less important
	if ( cg.centerPrintTime && priority < cg.centerPrintPriority ) {
		return;
	}

	strncpy( cg.centerPrint, CG_TranslateMessage( str, "Center Print" ),
			 sizeof( cg.centerPrint ) - 1 );
	cg.centerPrintPriority = priority;
	cg.centerPrint[ sizeof( cg.centerPrint ) - 1 ] = 0;

	// turn spaces into newlines, if we've run over the linewidth
	len = 0;
	neednewline = qfalse;
	s = cg.centerPrint;
	while ( *s ) {
		c = trap_SE_ReadCharFromString( (const unsigned char **)&s, NULL );
		if ( c == '\n' ) {
			neednewline = qfalse;
			len = 0;
			continue;
		}
		len++;
		if ( len >= CP_LINEWIDTH ) {
			neednewline = qtrue;
		} else if ( !neednewline ) {
			continue;
		}
		// only break on spaces, and if we've run over the linewidth
		if ( c == ' ' ) {
			s[-1] = '\n';
			neednewline = qfalse;
			len = 0;
		}
	}

	cg.centerPrintTime = cg.time + 2000;
	/* cg_local.h types both float; the binary __ftol2's the argument and
	   stores an int, and CG_DrawCenterString fild's them back. */
	cg.centerPrintY = (float)(int)y;
	cg.centerPrintCharWidth = (float)(int)charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while ( *s ) {
		c = trap_SE_ReadCharFromString( (const unsigned char **)&s, NULL );
		if ( c == '\n' ) {
			cg.centerPrintLines++;
		} else if ( c == '\\' && *s == 'n' ) {
			cg.centerPrintLines++;
			s++;
		}
	}
}

/*
==============
CG_CenterPrint                0x30015B20

`str` comes in through edx.  Unnamed in retail; the name is RTCW's, and it is
the priority-0 wrapper RTCW's is.
==============
*/
void CG_CenterPrint( const char *str, float y, float charWidth ) {
	CG_PriorityCenterPrint( str, y, charWidth, 0 );
}

/*
===================
CG_DrawCenterString           0x30015B40
===================
*/
static void CG_DrawCenterString( void ) {
	char    *start;
	int l;
	int h;
	float x, y, lineHeight;
	float scale;
	float   *color;
	char linebuffer[1024];

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, (int)( cg_centertime.value * 1000 ) );
	if ( !color ) {
		cg.centerPrintTime = 0;
		cg.centerPrintPriority = 0;
		return;
	}

	trap_R_SetColor( color );

	start = cg.centerPrint;

	scale = cg.centerPrintCharWidth / 32.0f;
	h = trap_R_Text_Height( 0, scale );

	y = cg.centerPrintY - h * 0.5f * cg.centerPrintLines - h;
	lineHeight = h * 1.2f;

	while ( 1 ) {
		for ( l = 0; l < CP_LINEWIDTH; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		x = ( 640.0f - trap_R_Text_Width( linebuffer, 0, scale, 0 ) ) * 0.5f;
		trap_R_Text_Paint( x, y, 0, scale, color, linebuffer, 0, 0, 3 );

		y += lineHeight;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	trap_R_SetColor( NULL );
}


/*
=================
CG_CalcCrosshairPosition      0x30015D70

Project the gun's aim direction onto the 640x480 virtual screen, relative to its
centre.  `x` comes in through edi and `y` through esi.  No RTCW counterpart --
RTCW's crosshair is always dead centre.
=================
*/
static void CG_CalcCrosshairPosition( float *x, float *y ) {
	float sp, cp, sy, cy;
	vec3_t forward;
	float dot;

	cy = (float)cos( CG_GUN_ANGLES[YAW] * ( M_PI / 180.0f ) );
	sy = (float)sin( CG_GUN_ANGLES[YAW] * ( M_PI / 180.0f ) );
	cp = (float)cos( CG_GUN_ANGLES[PITCH] * ( M_PI / 180.0f ) );
	sp = (float)sin( CG_GUN_ANGLES[PITCH] * ( M_PI / 180.0f ) );

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;

	dot = cg.refdef.viewaxis[0][2] * forward[2]
		  + cg.refdef.viewaxis[0][1] * forward[1]
		  + cg.refdef.viewaxis[0][0] * forward[0];

	if ( dot <= 0 || cg.refdef.fov_x <= 0 || cg.refdef.fov_y <= 0 ) {
		*x = 0;
		*y = 0;
		return;
	}

	*x = ( cg.refdef.viewaxis[1][2] * forward[2]
		   + cg.refdef.viewaxis[1][1] * forward[1]
		   + cg.refdef.viewaxis[1][0] * forward[0] )
		 / ( (float)tan( cg.refdef.fov_x * ( M_PI / 360.0f ) ) * dot ) * -320.0f;

	*y = ( cg.refdef.viewaxis[2][2] * forward[2]
		   + cg.refdef.viewaxis[2][1] * forward[1]
		   + cg.refdef.viewaxis[2][0] * forward[0] )
		 / ( (float)tan( cg.refdef.fov_y * ( M_PI / 360.0f ) ) * dot ) * -240.0f;
}

/*
=================
CG_GetWeapReticleZoom         0x30015F20

How far into the ADS overlay we are, 0..1.  `zoom` comes in through ecx.
=================
*/
/* NOT static: cg_weapons_mp.c's CG_AddViewWeapon calls it. */
qboolean CG_GetWeapReticleZoom( float *zoom ) {
	float frac;

	frac = cg.predictedPlayerState.fWeaponPosFrac;
	*zoom = 0;

	if ( ( !cg.weaponInfo->adsOverlayShader[0] && !cg.weaponInfo->adsOverlayReticle )
		 || frac == 0 ) {
		return qfalse;
	}

	if ( CG_ADS_ZOOMING_IN ) {
		*zoom = frac - ( 1.0f - cg.weaponInfo->adsZoomInFrac );
		if ( *zoom > 0 ) {
			*zoom = *zoom / cg.weaponInfo->adsZoomInFrac;
		}
	} else {
		*zoom = frac - ( 1.0f - cg.weaponInfo->adsZoomOutFrac );
		if ( *zoom > 0 ) {
			*zoom = *zoom / cg.weaponInfo->adsZoomOutFrac;
		}
	}

	return *zoom > 0.01f;
}

/*
=================
CG_DrawWeapReticle            0x30015FE0

Draw the scope overlay: four mirrored corner quads of the side shader, the black
fill outside them, and then whichever centre reticle the weapon asked for.
Returns how much of the world is still visible, which is what the crosshair
fades itself by.
=================
*/
static float CG_DrawWeapReticle( void ) {
	vec4_t color = { 1, 1, 1, 1 };
	float zoom;
	int weapnum;
	float crossX, crossY;
	float halfW, halfH;
	float cx, cy;
	float x0, y0, x1, y1;
	float size, w, h;
	qhandle_t shader;

	if ( !CG_GetWeapReticleZoom( &zoom ) ) {
		return 1.0f;
	}
	color[3] = zoom;

	weapnum = cg.weaponInfo->weapIndex;

	CG_CalcCrosshairPosition( &crossX, &crossY );

	halfW = cg.weaponInfo->adsOverlayWidth * cgs.screenXScale;
	halfH = cg.weaponInfo->adsOverlayHeight * cgs.screenYScale;
	cx = cgs.screenXScale * crossX + cg.refdef.width * 0.5f + cg.refdef.x;
	cy = cgs.screenYScale * crossY + cg.refdef.height * 0.5f + cg.refdef.y;

	trap_R_SetColor( color );

	if ( cg.weaponInfo->adsOverlayShader[0] ) {
		shader = cg_weapons[ weapnum ].adsOverlayShader;

		y0 = cy - halfH;
		x0 = cx - halfW;

		// the overlay is one quadrant, mirrored into the other three
		trap_R_DrawStretchPic( x0, y0, halfW, halfH, 0, 0, 1, 1, shader );
		trap_R_DrawStretchPic( cx, y0, halfW, halfH, 1, 0, 0, 1, shader );
		trap_R_DrawStretchPic( x0, cy, halfW, halfH, 0, 1, 1, 0, shader );
		trap_R_DrawStretchPic( cx, cy, halfW, halfH, 1, 1, 0, 0, shader );

		// and everything outside it is filled with the overlay's edge texel
		if ( x0 > 0 ) {
			trap_R_DrawStretchPic( 0, 0, x0, (float)cg.refdef.height, 0, 0, 0, 1, shader );
		}
		x1 = halfW + cx;
		if ( x1 < cg.refdef.width ) {
			trap_R_DrawStretchPic( x1, 0, cg.refdef.width - x1, (float)cg.refdef.height,
								   0, 0, 0, 1, shader );
		}
		if ( y0 > 0 ) {
			trap_R_DrawStretchPic( x0, 0, halfW + halfW, y0, 0, 0, 1, 0, shader );
		}
		y1 = halfH + cy;
		if ( y1 < cg.refdef.height ) {
			trap_R_DrawStretchPic( x0, y1, halfW + halfW, cg.refdef.height - y1,
								   0, 0, 1, 0, shader );
		}
	}

	switch ( cg.weaponInfo->adsOverlayReticle ) {
	case 1:     // the weapon's own centre reticle, at its natural size
		size = (float)cg.weaponInfo->reticleCenterSize;
		w = size * cgs.screenXScale;
		h = size * cgs.screenYScale;
		trap_R_DrawStretchPic(
			cg.refdef.x + ( cg.refdef.width - w ) * 0.5f + cgs.screenXScale * crossX,
			cg.refdef.y + ( cg.refdef.height - h ) * 0.5f + cgs.screenYScale * crossY,
			w, h, 0, 0, 1, 1, cg_weapons[ weapnum ].reticleCenter );
		break;

	case 2:     // a short crosshair with a gap
		color[0] = color[1] = color[2] = 0;
		trap_R_SetColor( color );
		trap_R_DrawStretchPic( cx - 1, cy, 3, halfH * 0.9f, 0, 0, 1, 1,
							   cgs.media.hudSoftLineShader );
		trap_R_DrawStretchPic( cx - halfW * 0.9f, cy - 1, halfW * 0.75f, 3, 0, 0, 1, 1,
							   cgs.media.hudSoftLineHShader );
		trap_R_DrawStretchPic( cx + halfW * 0.15f, cy - 1, halfW * 0.75f, 3, 0, 0, 1, 1,
							   cgs.media.hudSoftLineHShader );
		break;

	case 3:     // one long cross through the middle
		color[0] = color[1] = color[2] = 0;
		trap_R_SetColor( color );
		trap_R_DrawStretchPic( cx - 1, cy - halfH * 0.9f, 3, halfH * 1.8f, 0, 0, 1, 1,
							   cgs.media.hudSoftLineShader );
		trap_R_DrawStretchPic( cx - halfW * 0.9f, cy - 1, halfW * 1.8f, 3, 0, 0, 1, 1,
							   cgs.media.hudSoftLineHShader );
		break;

	case 4:     // same as case 2; the compiler emitted both copies
		color[0] = color[1] = color[2] = 0;
		trap_R_SetColor( color );
		trap_R_DrawStretchPic( cx - 1, cy, 3, halfH * 0.9f, 0, 0, 1, 1,
							   cgs.media.hudSoftLineShader );
		trap_R_DrawStretchPic( cx - halfW * 0.9f, cy - 1, halfW * 0.75f, 3, 0, 0, 1, 1,
							   cgs.media.hudSoftLineHShader );
		trap_R_DrawStretchPic( cx + halfW * 0.15f, cy - 1, halfW * 0.75f, 3, 0, 0, 1, 1,
							   cgs.media.hudSoftLineHShader );
		break;

	default:
		break;
	}

	trap_R_SetColor( NULL );

	return 1.0f - zoom;
}

/*
=================
CG_DrawTurretCrossHair        0x30016610

The crosshair of the MG42 / tank the player is locked to.  The two `* 0.0`
multiplies are the compiler keeping a provably-zero crosshair offset.
=================
*/
static void CG_DrawTurretCrossHair( void ) {
	vec4_t color = { 1, 1, 1, 0 };
	int weapnum;
	weaponInfo_t *weapon;
	float size, w, h;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}
	if ( cl_paused.integer ) {   /* retail gates on cl_paused here, not crosshairNoGun */
		return;
	}
	if ( cg.renderingThirdPerson ) {
		return;
	}

	weapnum = cg_entities[ cg.predictedPlayerState.viewlocked_entNum ].currentState.weapon;
	if ( !weapnum ) {
		return;
	}
	weapon = bg_weaponInfo[weapnum];
	if ( !weapon->reticleCenter[0] ) {
		return;
	}

	color[3] = cg_crosshairAlpha.value;
	if ( color[3] < 0.01f ) {
		return;
	}

	trap_R_SetColor( color );

	size = (float)weapon->reticleCenterSize;
	w = size * cgs.screenXScale;
	h = size * cgs.screenYScale;

	trap_R_DrawStretchPic(
		cg.refdef.x + ( cg.refdef.width - w ) * 0.5f + cgs.screenXScale * 0.0f,
		cg.refdef.y + ( cg.refdef.height - h ) * 0.5f + cgs.screenYScale * 0.0f,
		w, h, 0, 0, 1, 1, cg_weapons[ weapnum ].reticleCenter );
}

/*
=================
CG_DrawCrosshair              0x30016760

Nothing is ever written into this in 1.1, so the fallback below always registers
the empty string.  It is a file-static, at 0x3020D5F0.
=================
*/
static char cg_crosshairShaderName[64];

static void CG_DrawCrosshair( void ) {
	vec4_t color = { 1, 1, 1, 0 };
	float frac;
	float scale;
	float offsetY;
	float crossX, crossY;
	int weapnum;
	weaponInfo_t *weapon;
	float f;
	float size;
	float hx, hy;
	qhandle_t shader;
	float minSpread, spread;
	float spreadX, spreadY, minOfs;
	int left;
	int i;
	float dir[4][2], ofs[4][2], add[4][2];

	if ( cg.renderingThirdPerson ) {
		return;
	}

	frac = cg.predictedPlayerState.fWeaponPosFrac;
	scale = 1.0f;
	offsetY = 0;
	color[3] = cg_crosshairAlpha.value;

	if ( cg.predictedPlayerState.eFlags & 0xC000 ) {        // riding a turret
		if ( cg.predictedPlayerState.viewlocked_entNum != ENTITYNUM_NONE ) {
			CG_DrawTurretCrossHair();
		}
		return;
	}

	if ( cg.predictedPlayerState.serverCursorHint == 9 ) {
		color[0] = 0.25f;
		color[2] = 0.25f;
	}

	weapnum = cg.weaponInfo->weapIndex;
	if ( !weapnum ) {
		return;
	}

	color[3] = CG_DrawWeapReticle() * cg_crosshairAlpha.value;
	if ( color[3] < 0.01f ) {
		return;
	}
	if ( !cg_drawCrosshair.integer ) {
		return;
	}
	if ( cl_paused.integer ) {   /* retail gates on cl_paused here, not crosshairNoGun */
		return;
	}
	if ( frac == 1.0f && cg_drawGun.integer ) {
		return;
	}
	if ( cg.predictedPlayerState.weaponstate == WEAPON_MELEE_INIT
		 || cg.predictedPlayerState.weaponstate == WEAPON_MELEE_FIRE
		 || cg.predictedPlayerState.weaponstate == WEAPON_RELOADING
		 || cg.predictedPlayerState.weaponstate == WEAPON_RAISING
		 || cg.predictedPlayerState.weaponstate == WEAPON_DROPPING ) {
		return;
	}

	CG_CalcCrosshairPosition( &crossX, &crossY );
	trap_R_SetColor( color );

	weapon = cg.weaponInfo;

	if ( frac != 0 ) {
		if ( CG_ADS_ZOOMING_IN ) {
			f = frac - ( 1.0f - weapon->adsCrosshairInFrac );
			if ( f > 0 ) {
				f = f / weapon->adsCrosshairInFrac;
			}
		} else {
			f = frac - ( 1.0f - weapon->adsCrosshairOutFrac );
			if ( f > 0 ) {
				f = f / weapon->adsCrosshairOutFrac;
			}
		}
		if ( f > 0 ) {
			scale = 1.0f - 0.5f * f;
			offsetY = 480.0f / cg.refdef.fov_y * weapon->adsAimPitch * f;
		}

		if ( !cg_drawGun.integer ) {
			// the shrinking hip crosshair, drawn only when the gun is hidden
			if ( scale < 1.0f ) {
				shader = cg_weapons[ weapnum ].reticleCenter;
				if ( !shader ) {
					shader = trap_R_RegisterShader( cg_crosshairShaderName, 5 );
				}
				if ( shader ) {
					size = (float)weapon->reticleCenterSize * ( 1.5f - scale );
					trap_R_DrawStretchPic(
						cg.refdef.x + ( cg.refdef.width - size ) * 0.5f + cgs.screenXScale * crossX,
						cg.refdef.y + ( cg.refdef.height - size ) * 0.5f + cgs.screenYScale * crossY,
						size, size, 0, 0, 1, 1, shader );
				}
			}
			if ( frac == 1.0f ) {
				trap_R_SetColor( NULL );
				return;
			}
		}
	}

	if ( !cg_crosshairDynamic.integer ) {
		crossX = 0;
		crossY = offsetY;
	}

	if ( weapon->reticleCenter[0] ) {
		hx = cgs.screenXScale * crossX;
		hy = cgs.screenYScale * crossY;
		size = (float)weapon->reticleCenterSize;

		// a cooking grenade grows its reticle and ticks once a second
		if ( weapon->weaponType == 1 && cg.predictedPlayerState.grenadeTimeLeft
			 && weapon->cookOffHold ) {
			left = cg.predictedPlayerState.grenadeTimeLeft;
			if ( CG_LAST_GRENADE_PULSE % 1000 < left % 1000 && (unsigned)( left / 1000 ) < 4 ) {
				CG_PlayClientSoundAliasByName( (const char *)cgs.media.grenadePulse[left / 1000] );
				left = cg.predictedPlayerState.grenadeTimeLeft;
			}
			CG_LAST_GRENADE_PULSE = left;
			size += ( left % 1000 ) * 0.01f;
		}

		size *= scale;
		trap_R_DrawStretchPic(
			cg.refdef.x + ( cg.refdef.width - size ) * 0.5f + hx,
			cg.refdef.y + ( cg.refdef.height - size ) * 0.5f + hy,
			size, size, 0, 0, 1, 1, cg_weapons[ weapnum ].reticleCenter );
		weapon = cg.weaponInfo;
	}

	if ( !weapon->reticleSide[0] ) {
		trap_R_SetColor( NULL );
		return;
	}

	// the four spread bars
	color[3] = CG_DrawWeapReticle()
			   * ( 1.0f - cg.predictedPlayerState.aimSpreadScale / 255.0f )
			   * cg_crosshairAlpha.value;
	if ( color[3] < cg_crosshairAlphaMin.value ) {
		color[3] = cg_crosshairAlphaMin.value;
	}
	trap_R_SetColor( color );

	minSpread = BG_GetMinSpreadForWeapon( cg.snap->serverTime, &cg.predictedPlayerState, weapnum );
	spread = ( weapon->hipSpreadMax - minSpread )
			 * ( cg.predictedPlayerState.aimSpreadScale / 255.0f ) + minSpread;
	spread *= scale;

	spreadX = 640.0f / cg.refdef.fov_x * spread;
	spreadY = 480.0f / cg.refdef.fov_y * spread;
	minOfs = (float)weapon->reticleMinOfs;
	if ( spreadX < minOfs ) {
		spreadX = minOfs;
	}
	if ( spreadY < minOfs ) {
		spreadY = minOfs;
	}

	shader = cg_weapons[ weapnum ].reticleSide;

	dir[0][0] = 0;      dir[0][1] = -1;
	dir[1][0] = 1;      dir[1][1] = 0;
	dir[2][0] = 0;      dir[2][1] = 1;
	dir[3][0] = -1;     dir[3][1] = 0;

	ofs[0][0] = -0.5f;  ofs[0][1] = -1;
	ofs[1][0] = 0;      ofs[1][1] = -0.5f;
	ofs[2][0] = -0.5f;  ofs[2][1] = 0;
	ofs[3][0] = -1;     ofs[3][1] = -0.5f;

	add[0][0] = 0;      add[0][1] = -1;
	add[1][0] = 0;      add[1][1] = 0;
	add[2][0] = 0;      add[2][1] = 0;
	add[3][0] = -1;     add[3][1] = 0;

	for ( i = 0; i < 4; i++ ) {
		size = (float)weapon->reticleSideSize * scale;
		trap_R_DrawStretchPicRotate(
			cg.refdef.x + cg.refdef.width * 0.5f
				+ ( spreadX * dir[i][0] + crossX ) * cgs.screenXScale
				+ size * ofs[i][0] + add[i][0]
				- dir[i][0] * weapon->hipReticleSidePos * size,
			cg.refdef.y + cg.refdef.height * 0.5f
				+ ( spreadY * dir[i][1] + crossY ) * cgs.screenYScale
				+ size * ofs[i][1] + add[i][1]
				- dir[i][1] * weapon->hipReticleSidePos * size,
			size, size,
			0, (float)( ( i >> 1 ) & 1 ), 1, (float)( ( ( i - 2 ) >> 1 ) & 1 ),
			( i & 1 ) * 90.0f, shader );
		weapon = cg.weaponInfo;
	}

	trap_R_SetColor( NULL );
}

/*
=================
CG_ScanForCrosshairEntity     0x30016E50
=================
*/
static void CG_ScanForCrosshairEntity( void ) {
	trace_t trace;
	vec3_t start, end;

	start[0] = cg.refdef.vieworg[0];
	start[1] = cg.refdef.vieworg[1];
	start[2] = cg.refdef.vieworg[2];

	end[0] = cg.refdef.viewaxis[0][0] * 8192 + cg.refdef.vieworg[0];
	end[1] = cg.refdef.viewaxis[0][1] * 8192 + cg.refdef.vieworg[1];
	end[2] = cg.refdef.viewaxis[0][2] * 8192 + cg.refdef.vieworg[2];

	/* 0x2000001 -- CONTENTS_SOLID | CONTENTS_BODY */
	CG_Trace( &trace, start, vec3_origin, vec3_origin, end,
			  cg.snap->ps.clientNum, 0x2000001 );

	if ( trace.entityNum >= MAX_CLIENTS ) {
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}

/*
==============
CG_CheckForCursorHints        0x30016F10
==============
*/
void CG_CheckForCursorHints( void ) {
	if ( cg.renderingThirdPerson ) {
		return;
	}
	if ( !cg.snap->ps.serverCursorHint ) {
		return;
	}

	cg.cursorHintTime = cg.time;
	cg.cursorHintFade = cg_hintFadeTime.integer;
	cg.cursorHintIcon = cg.snap->ps.serverCursorHint;
	cg.cursorHintValue = cg.snap->ps.serverCursorHintVal;
	cg.cursorHintString = cg.snap->ps.serverCursorHintString;
}

/*
=====================
CG_DrawCrosshairNames         0x30016F70
=====================
*/
static void CG_DrawCrosshairNames( void ) {
	float   *fadeColor;
	char    *name;
	int myTeam;
	float f;
	vec4_t color;

	if ( cg_drawCrosshair.integer < 0 ) {
		return;
	}
	if ( !cg_drawCrosshairNames.integer ) {
		return;
	}
	if ( cg.renderingThirdPerson ) {
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	fadeColor = CG_FadeColor( cg.crosshairClientTime, 150 );
	if ( !fadeColor ) {
		return;
	}
	if ( cg.crosshairClientNum > MAX_CLIENTS ) {
		return;
	}
	if ( !bg_clientinfo[cg.snap->ps.clientNum].infoValid ) {
		return;
	}
	myTeam = bg_clientinfo[cg.snap->ps.clientNum].team;
	if ( !myTeam ) {
		return;
	}
	if ( !bg_clientinfo[cg.crosshairClientNum].infoValid ) {
		return;
	}
	if ( myTeam != 3 && bg_clientinfo[cg.crosshairClientNum].team != myTeam ) {
		return;
	}

	name = va( "%s", bg_clientinfo[cg.crosshairClientNum].name );
	if ( !name || !*name ) {
		return;
	}
	Q_CleanStr( name );

	if ( cg.crosshairClientNum == CG_NAMEFLASH_CLIENT ) {
		f = CG_NAMEFLASH_VALUE * 0.01f;
		if ( f > 1.0f ) {
			f = 1.0f;
		} else if ( f < 0 ) {
			f = 0;
		}
		color[2] = 0;
		if ( f > 0.5 ) {
			color[0] = ( 1.0f - f ) * 2;
			color[1] = 1.0f;
		} else {
			color[0] = 1.0f;
			color[1] = f * 2;
		}
	} else {
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
	}
	color[3] = fadeColor[3] * 0.6f;

	trap_R_Text_Paint( 345, 217, 0, 0.25f, color, name, 0, 0, 3 );
}

/*
==============
CG_DrawFlashFade              0x30017120

The script fade: cgs.scriptFade[0] is the target alpha, [1] the current one;
scriptFadeStartTime / scriptFadeDuration sit beside them.
==============
*/
static void CG_DrawFlashFade( void ) {
	int t, msec;
	float d;
	vec4_t col;

	if ( cgs.scriptFadeStartTime + cgs.scriptFadeDuration < cg.time ) {
		cgs.scriptFade[1] = cgs.scriptFade[0];
	} else if ( cgs.scriptFade[1] != cgs.scriptFade[0] ) {
		t = trap_Milliseconds();
		msec = t - fadeLastTime;
		fadeLastTime = t;
		if ( msec < 500 && msec > 0 ) {
			d = msec / (float)cgs.scriptFadeDuration;
			if ( cgs.scriptFade[1] > cgs.scriptFade[0] ) {
				cgs.scriptFade[1] -= d;
				if ( cgs.scriptFade[1] < cgs.scriptFade[0] ) {
					cgs.scriptFade[1] = cgs.scriptFade[0];
				}
			} else {
				cgs.scriptFade[1] += d;
				if ( cgs.scriptFade[1] > cgs.scriptFade[0] ) {
					cgs.scriptFade[1] = cgs.scriptFade[0];
				}
			}
		}
	}

	if ( cgs.scriptFade[1] > 0 ) {
		col[0] = 0;
		col[1] = 0;
		col[2] = 0;
		col[3] = cgs.scriptFade[1];
		CG_FillRect( 0, 0, 640, 480, col );
	}
}

/*
==============
CG_DrawFlashDamage            0x30017250
==============
*/
#define DAMAGE_TIME 500

static void CG_DrawFlashDamage( void ) {
	float redFlash;
	vec4_t col;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.v_dmg_time <= cg.time ) {
		return;
	}

	redFlash = (float)fabs( cg.damageY * ( (float)( cg.v_dmg_time - cg.time ) / DAMAGE_TIME ) );

	// blend the entire screen red
	if ( redFlash > 5 ) {
		redFlash = 5;
	}

	col[0] = 0.2f;
	col[1] = 0;
	col[2] = 0;
	col[3] = 0.7f * ( redFlash / 5 );
	CG_FillRect( -10, -10, 650, 490, col );
}

/*
==============
CG_DrawDamageDirectionIndicators      0x300172F0

Eight 12-byte records in cg.damageIndicators: { serverTime, lifetime, yaw }.
==============
*/
static void CG_DrawDamageDirectionIndicators( void ) {
	float zoom;
	float x, y;
	float w;
	float verts[8];
	vec4_t color = { 1, 1, 1, 0 };
	int i;
	int t, duration;
	float yaw;
	float alpha;

	if ( !cg.snap ) {
		return;
	}

	if ( CG_GetWeapReticleZoom( &zoom ) ) {
		if ( !cg_hudDamageIconInScope.integer ) {
			return;
		}
		CG_CalcCrosshairPosition( &x, &y );
		x += 320;
		y += 240;
	} else {
		x = 320;
		y = 240;
	}

	w = cg_hudDamageIconWidth.value * 0.5f;
	verts[0] = -w;
	verts[1] = cg_hudDamageIconOffset.value;
	verts[2] = w;
	verts[3] = cg_hudDamageIconOffset.value;
	verts[4] = w;
	verts[5] = cg_hudDamageIconHeight.value + cg_hudDamageIconOffset.value;
	verts[6] = -w;
	verts[7] = cg_hudDamageIconHeight.value + cg_hudDamageIconOffset.value;

	for ( i = 0; i < 8; i++ ) {
		t = cg.time - cg.damageIndicators[i][0];
		duration = cg.damageIndicators[i][1];
		if ( t <= 0 || t >= duration ) {
			continue;
		}

		yaw = vectoyaw( cg.refdef.viewaxis[0] ) - *(float *)&cg.damageIndicators[i][2];

		alpha = 2.0f - ( t + t ) / (float)duration;
		if ( alpha > 1.0f ) {
			color[3] = 1.0f;
		} else {
			color[3] = alpha;
		}

		trap_R_SetColor( color );
		CG_DrawRotatedQuadPic( x, y, verts, cg_damageIconTexCoords, yaw,
							   cgs.media.hudHitDirection );
	}
}

/*
==============
CG_DrawTimedMenus             0x30017470
==============
*/
static void CG_DrawTimedMenus( void ) {
	void *menu;

	if ( cg.voicetime && cg.time - cg.voicetime > 2500 ) {
		menu = Menus_FindByName( "voiceMenu" );
		if ( menu ) {
			Menus_Close( menu );
		}
		trap_Cvar_Set( "cl_conXOffset", "0" );
		cg.voicetime = 0;
	}
}

/*
==============
CG_Fade                       0x300174C0

`startTime` comes in through ecx and `duration` through eax; r, g and b are
accepted and never read -- CoD's script fade is alpha only.  Nothing in the DLL
calls this.
==============
*/
void CG_Fade( int r, int g, int b, int a, int startTime, int duration ) {
	cgs.scriptFade[0] = (float)a / 255.0f;
	cgs.scriptFadeStartTime = startTime;
	cgs.scriptFadeDuration = duration;

	if ( startTime + duration <= cg.time ) {
		cgs.scriptFade[1] = cgs.scriptFade[0];
	}
}

/*
==============
CG_ScreenFade                 0x30017500
==============
*/
static void CG_ScreenFade( void ) {
	int msec;
	int i;
	float t, invt;
	vec4_t color;

	if ( !cg.fadeRate ) {
		return;
	}

	msec = cg.fadeTime - cg.time;
	if ( msec <= 0 ) {
		cg.fadeFrom[0] = cg.fadeTo[0];
		cg.fadeFrom[3] = cg.fadeTo[3];
		cg.fadeFrom[1] = cg.fadeTo[1];
		cg.fadeFrom[2] = cg.fadeTo[2];

		if ( !cg.fadeTo[3] ) {
			cg.fadeRate = 0;
			return;
		}

		CG_FillRect( 0, 0, 640, 480, cg.fadeFrom );
	} else {
		t = (float)msec * cg.fadeRate;
		invt = 1.0f - t;

		for ( i = 0; i < 4; i++ ) {
			color[i] = cg.fadeTo[i] * invt + cg.fadeFrom[i] * t;
		}

		if ( color[3] ) {
			CG_FillRect( 0, 0, 640, 480, color );
		}
	}
}

/*
==============
CG_DrawSoundOverlay           0x30017620

cg_drawSoundOverlay.  The engine fills an array of 20-byte records, one per
mixer channel: { name, vol, rvol, hz, pit }.
==============
*/
typedef struct {
	const char *name;               /* +0x00 */
	float vol;                      /* +0x04 */
	float rvol;                     /* +0x08 */
	int hz;                         /* +0x0C */
	float pit;                      /* +0x10 */
} soundOverlay_t;

void CG_DrawSoundOverlay( void ) {
	soundOverlay_t chans[64];
	int cpu;
	int count;
	int i;
	float y;
	char provider[1024];
	char buf[1024];
	int bits, khz, stereo;
	char *s;

	count = trap_MSS_GetSoundOverlay( cg_drawSoundOverlay.integer, (int)chans, 64, (int)&cpu );
	if ( count <= 0 ) {
		return;
	}

	trap_Cvar_VariableStringBuffer( "mss_3d_provider", provider, sizeof( provider ) );
	trap_Cvar_VariableStringBuffer( "mss_bits", buf, sizeof( buf ) );
	bits = atoi( buf );
	trap_Cvar_VariableStringBuffer( "mss_khz", buf, sizeof( buf ) );
	khz = atoi( buf );
	trap_Cvar_VariableStringBuffer( "mss_stereo", buf, sizeof( buf ) );
	stereo = atoi( buf );

	s = va( "CPU: ^3%%%i ^73D provider: ^3%s ^7bits: ^3%i ^7kHz: ^3%i ^7chan: ^3%i",
			cpu, provider, bits, khz, ( stereo != 0 ) + 1 );
	CG_DrawStringExt( 2, 82, s, colorWhite, qtrue, qfalse, 8, 16, 0, qfalse );

	y = 98;
	for ( i = 0; i < count; i++ ) {
		if ( chans[i].name ) {
			s = va( "%2i %-50s vol:^3%04.2f ^7rvol:^3%04.2f ^7pit:^3%04.2f ^7hz:^3%5i",
					i, chans[i].name, chans[i].vol, chans[i].rvol, chans[i].pit, chans[i].hz );
		} else {
			s = va( "%2i", i );
		}
		CG_DrawStringExt( 2, y, s, NULL, qfalse, qtrue, 8, 16, 0, qfalse );
		y += 16;
	}
}

/*
==============
CG_DrawGameMessages           0x30017880

The console's notify lines, parked under the compass.
==============
*/
static void CG_DrawGameMessages( void ) {
	float x, y;
	float alpha;
	float   *color;

	x = 6;
	y = 345.0f - ( cg_hudCompassSize.value - 1.0f ) * 160.0f + 12.0f;

	if ( cg.showScores ) {
		color = CG_FadeColor( cg.scoreFadeTime, 100 );
		if ( !color ) {
			return;
		}
		alpha = cg_hudAlpha.value * color[3];
	} else {
		alpha = cg_hudAlpha.value;
	}

	trap_DrawNotifyLines( Q_ftol( x ), Q_ftol( y ), alpha, 2 );
}

/*
==============
CG_DrawBoldGameMessages       0x30017940
==============
*/
static void CG_DrawBoldGameMessages( void ) {
	float x, y;
	float alpha;
	float   *color;

	x = 320;
	y = 180;

	if ( cg.showScores ) {
		color = CG_FadeColor( cg.scoreFadeTime, 100 );
		if ( !color ) {
			return;
		}
		alpha = cg_hudAlpha.value * color[3];
	} else {
		alpha = cg_hudAlpha.value;
	}

	trap_DrawBoldMessages( Q_ftol( x ), Q_ftol( y ), alpha, 3 );
}

/*
==============
CG_DrawMiniConsole            0x300179F0
==============
*/
static void CG_DrawMiniConsole( void ) {
	float x, y;

	if ( con_minicon.integer < 0 ) {
		return;
	}
	if ( !cg_developer.integer && !con_minicon.integer ) {
		return;
	}

	x = 2;
	y = 4;
	trap_DrawMiniConsole( Q_ftol( x ), Q_ftol( y ), cg_hudAlpha.value );
}

/*
==============
CG_DrawSubtitles              0x30017A80
==============
*/
static void CG_DrawSubtitles( void ) {
	float x, y;

	if ( !cg_subtitles.integer ) {
		return;
	}

	x = 135;
	y = 425;
	trap_DrawSubtitles( Q_ftol( x ), Q_ftol( y ), cg_hudAlpha.value, 2 );
}

/*
==============
CG_DrawSay                    0x30017B00
==============
*/
static void CG_DrawSay( void ) {
	trap_DrawSay( 100 );
}

/*
==============
CG_DrawPerformanceWarnings    0x30017B10

StatMon's warning icons, laid out two per row in the top-left corner.  The
engine hands back an array of { endTime, shader } pairs.
==============
*/
typedef struct {
	int endTime;
	qhandle_t shader;
} statMonWarning_t;

static void CG_DrawPerformanceWarnings( void ) {
	int now;
	statMonWarning_t *warnings;
	int count;
	int i;
	float x, y;

	now = trap_Milliseconds();
	trap_syscall_0xBB( (int)&warnings, (int)&count );

	x = 2;
	y = 200;

	for ( i = 0; i < count; i++ ) {
		if ( warnings[i].endTime >= now ) {
			trap_R_DrawStretchPic( x * cgs.screenXScale, y * cgs.screenYScale,
								   32 * cgs.screenXScale, 32 * cgs.screenYScale,
								   0, 0, 1, 1, warnings[i].shader );
		}
		x += 34;
		if ( x + 32 > 68 ) {
			x = 2;
			y += 34;
		}
	}
}

/*
==============
CG_DrawShader                 0x30017C50

The renderer's own per-frame statistics, three lines of them.  Trap 108 fills
three buffers and returns whether it wrote anything; only the last two are
0x1000 bytes, the first is 64.

0x30017C50..0x30017D0F, a separate body the compiler tail-inlined into
CG_DrawDebugOverlays.
==============
*/
static void CG_DrawShader( void ) {
	char header[64];
	char line1[1024];
	char line2[1024];

	if ( !trap_R_TrackStatistics( (int)cg.refdef.vieworg, (int)cg.refdef.viewaxis,
								  (int)header, (int)line1, (int)line2,
								  sizeof( line1 ) ) ) {
		return;
	}

	CG_DrawSmallString( 8, 240, header, 1.0f );
	CG_DrawSmallString( 8, 256, line1, 1.0f );
	CG_DrawSmallString( 8, 272, line2, 1.0f );
}

/*
==============
CG_DrawDebugOverlays          0x30017D10

Three mutually exclusive debug pages.  CG_DrawShader (0x30017C50) above is a
separate function that the compiler tail-inlined into this one.
==============
*/
static void CG_DrawDebugOverlays( void ) {
	if ( cg_drawSoundOverlay.integer ) {
		CG_DrawSoundOverlay();
	} else if ( cg_drawScriptUsage.integer ) {
		CG_DrawScriptUsage();
	} else if ( cg_drawShader.integer ) {
		CG_DrawShader();
	}
}

/*
==============
CG_UpdateCameraShake          0x30017D40

Recompute one shake's weight for this frame.  `shake` comes in through esi.
==============
*/
static qboolean CG_UpdateCameraShake( cameraShake_t *shake ) {
	int msec;
	float scale;
	float amplitude;

	msec = cg.time - shake->startTime;
	if ( msec < 0 ) {
		return qfalse;
	}
	if ( (float)msec >= shake->duration ) {
		return qfalse;
	}

	scale = 1.0f - Distance( cg.refdef.vieworg, shake->origin ) / shake->radius;
	amplitude = ( 1.0f - msec / shake->duration ) * shake->amplitude;

	if ( scale < 0 ) {
		shake->weight = scale / amplitude;
	} else {
		shake->weight = scale * amplitude;
	}
	shake->frameAmplitude = amplitude;

	return qtrue;
}

/*
==============
CG_StartShakeCamera           0x30017DD0

`origin` comes in through ecx.  Take the first free or expired slot; if all four
are busy, replace the weakest -- and if this one is weaker than all of them,
drop it.
==============
*/
void CG_StartShakeCamera( float amplitude, int duration, float radius, const vec3_t origin ) {
	cameraShake_t shake;
	int i;
	float weakest;

	if ( amplitude <= 0 ) {
		return;
	}

	shake.startTime = cg.time;
	shake.amplitude = amplitude;
	shake.duration = (float)duration;
	shake.radius = radius;
	shake.origin[0] = origin[0];
	shake.origin[1] = origin[1];
	shake.origin[2] = origin[2];

	CG_UpdateCameraShake( &shake );

	for ( i = 0; i < 4; i++ ) {
		if ( cg.cameraShakes[i].startTime > cg.time ) {
			break;
		}
		if ( cg.time >= (float)cg.cameraShakes[i].startTime
			 + cg.cameraShakes[i].duration ) {
			break;
		}
	}

	if ( i == 4 ) {
		weakest = shake.weight;
		if ( cg.cameraShakes[0].weight < weakest ) {
			i = 0;
			weakest = cg.cameraShakes[0].weight;
		}
		if ( cg.cameraShakes[1].weight < weakest ) {
			i = 1;
			weakest = cg.cameraShakes[1].weight;
		}
		if ( cg.cameraShakes[2].weight < weakest ) {
			i = 2;
			weakest = cg.cameraShakes[2].weight;
		}
		if ( cg.cameraShakes[3].weight < weakest ) {
			i = 3;
		} else if ( i == 4 ) {
			return;
		}
	}

	memcpy( &cg.cameraShakes[i], &shake, sizeof( shake ) );
}

/*
==============
CG_ShakeCamera                0x30017F00

cg.shakePhase is the sine phase and cg.shakeMinAmplitude an always-on floor.
==============
*/
void CG_ShakeCamera( void ) {
	float weight, amplitude;
	float t;
	float r;
	int i;

	amplitude = 0;
	weight = 0;
	t = cg.time / 600.0f;

	for ( i = 0; i < 4; i++ ) {
		if ( CG_UpdateCameraShake( &cg.cameraShakes[i] )
			 && cg.cameraShakes[i].weight > weight ) {
			weight = cg.cameraShakes[i].weight;
			amplitude = cg.cameraShakes[i].frameAmplitude;
		}
	}

	if ( cg.shakeMinAmplitude > weight ) {
		weight = cg.shakeMinAmplitude;
		amplitude = cg.shakeMinAmplitude;
	}

	if ( weight <= 0 ) {
		/* crandom() * M_PI: ONE rand (0x30017F95), then `fadd st,st` doubles it
		   before the -1.0 (0x30017FA8) -- not two draws from the sequence */
		r = rand() / 32768.0f;
		cg.shakePhase = ( r + r - 1.0f ) * (float)M_PI;
		return;
	}

	if ( weight > 1.0f ) {
		weight = 1.0f;
	}

	cg.refdefViewAngles[PITCH] += (float)sin( t * 25.132742f + cg.shakePhase )
								  * weight * amplitude * 18.0f;
	cg.refdefViewAngles[YAW] += (float)sin( t * 47.12389f + cg.shakePhase )
								* weight * amplitude * 16.0f;
	cg.refdefViewAngles[ROLL] += (float)sin( t * 37.699112f + cg.shakePhase )
								 * weight * amplitude * 10.0f;
}

/*
=================
CG_DrawSpectator              0x30018070
=================
*/
static void CG_DrawSpectator( void ) {
	const char *s;
	int w;
	vec4_t color = { 1, 1, 1, 1 };

	s = CG_SafeTranslateString_Internal( "cgame", "CGAME_SPECTATOR" );
	w = trap_R_Text_Width( s, 0, 16 / 48.0f, 0 );
	trap_R_Text_Paint( ( 640.0f - w ) * 0.5f, 458, 4, 16 / 48.0f, color, s, 0, 0, 3 );
}

/*
=================
CG_DrawVote                   0x30018120
=================
*/
static void CG_DrawVote( void ) {
	const char *s;
	int sec;
	vec4_t color = { 1, 1, 0, 1 };
	char *binding;
	char yesKey[256];
	char noKey[256];

	if ( cgs.unknown_0x0A434 > cg.time || cgs.voteTime ) {
		if ( GetKeyBindingLocalizedString( "vote yes", &binding ) ) {
			strncpy( yesKey, binding, sizeof( yesKey ) - 1 );
		} else {
			strncpy( yesKey, "vote yes", sizeof( yesKey ) - 1 );
		}
		yesKey[ sizeof( yesKey ) - 1 ] = 0;

		if ( GetKeyBindingLocalizedString( "vote no", &binding ) ) {
			strncpy( noKey, binding, sizeof( noKey ) - 1 );
		} else {
			strncpy( noKey, "vote no", sizeof( noKey ) - 1 );
		}
		noKey[ sizeof( noKey ) - 1 ] = 0;
	}

	if ( cgs.unknown_0x0A434 > cg.time ) {
		// a teamkill complaint
		switch ( cgs.unknown_0x0A430 ) {
		case -1:
			s = CG_SafeTranslateString_Internal( "cgame", "CGAME_COMPLAINTFILED" );
			break;
		case -2:
			s = CG_SafeTranslateString_Internal( "cgame", "CGAME_COMPLAINTDISMISSED" );
			break;
		case -3:
			s = CG_SafeTranslateString_Internal( "cgame", "CGAME_COMPLAINTSERVERHOST" );
			break;
		case -4:
			s = CG_SafeTranslateString_Internal( "cgame", "CGAME_SERVERHOSTTEAMKILLED" );
			break;
		default:
			s = NULL;
			break;
		}

		if ( s ) {
			trap_R_Text_Paint( 8, 200, 0, 10 / 48.0f, color, s, 0, 0, 3 );
			return;
		}

		if ( !bg_clientinfo[cgs.unknown_0x0A430].infoValid ) {
			return;
		}

		s = CG_SafeTranslateString_Internal( "cgame", "CGAME_COMPLAINTTEAMKILLFILE" );
		trap_R_Text_Paint( 8, 200, 0, 10 / 48.0f, color,
						   va( s, bg_clientinfo[cgs.unknown_0x0A430].name ), 0, 0, 3 );

		s = CG_SafeTranslateString_Internal( "cgame", "CGAME_PRESSYESNO" );
		trap_R_Text_Paint( 8, 210, 0, 10 / 48.0f, color,
						   va( s, yesKey, noKey ), 0, 0, 3 );
		return;
	}

	if ( !cgs.voteTime ) {
		return;
	}

	if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		CG_PlaySoundAliasByName( (const char *)cgs.media.talkSound,
								 cg.snap->ps.clientNum, cg.snap->ps.origin );
	}

	sec = ( cgs.voteTime - cg.time ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}

	if ( cg.snap->ps.eFlags & 0x20000 ) {
		s = CG_SafeTranslateString_Internal( "cgame", "CGAME_VOTE" );
		trap_R_Text_Paint( 8, 200, 0, 10 / 48.0f, color,
						   va( "%s(%i):%s", s, sec, cgs.voteString ), 0, 0, 3 );

		trap_R_Text_Paint( 8, 210, 0, 10 / 48.0f, color,
						   va( "%s:%i, %s:%i",
									CG_SafeTranslateString_Internal( "cgame", "CGAME_YES" ),
									cgs.voteYes,
									CG_SafeTranslateString_Internal( "cgame", "CGAME_NO" ),
									cgs.voteNo ), 0, 0, 3 );
	} else {
		s = CG_SafeTranslateString_Internal( "cgame", "CGAME_VOTE" );
		trap_R_Text_Paint( 8, 200, 0, 10 / 48.0f, color,
						   va( "%s(%i):%s", s, sec, cgs.voteString ), 0, 0, 3 );

		trap_R_Text_Paint( 8, 210, 0, 10 / 48.0f, color,
						   va( "%s(%s):%i, %s(%s):%i",
									CG_SafeTranslateString_Internal( "cgame", "CGAME_YES" ),
									yesKey, cgs.voteYes,
									CG_SafeTranslateString_Internal( "cgame", "CGAME_NO" ),
									noKey, cgs.voteNo ), 0, 0, 3 );
	}
}

/*
=================
CG_DrawIntermission           0x30018530
=================
*/
static void CG_DrawIntermission( void ) {
	trap_UI_CloseAllMenus();
	cg.scoreFadeTime = cg.time;
	CG_DrawScoreboard();
}

/*
=================
CG_DrawSpectatorMessage       0x30018550
=================
*/
static void CG_DrawSpectatorMessage( void ) {
	const char *str;
	char *binding;
	vec4_t color = { 1, 1, 1, 1 };

	if ( !cg_descriptiveText.integer ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & 0x60000 ) {
		return;
	}

	Controls_GetConfig();

	if ( !GetKeyBindingLocalizedString( "+attack", &binding ) ) {
		binding = (char *)"+attack";
	}
	str = CG_SafeTranslateString_Internal( "cgame", "CGAME_FOLLOWNEXTPLAYER" );
	trap_R_Text_Paint( 240, 426, 0, 10 / 48.0f, color,
					   va( str, binding ), 0, 0, 3 );

	if ( !GetKeyBindingLocalizedString( "+melee", &binding ) ) {
		binding = (char *)"+melee";
	}
	str = CG_SafeTranslateString_Internal( "cgame", "CGAME_FOLLOWPREVIOUSPLAYER" );
	trap_R_Text_Paint( 240, 436, 0, 10 / 48.0f, color,
					   va( str, binding ), 0, 0, 3 );

	if ( !( cg.snap->ps.pm_flags & 0x10000 ) ) {
		return;
	}
	if ( !GetKeyBindingLocalizedString( "toggle cl_run", &binding )
		 && !GetKeyBindingLocalizedString( "+speed", &binding ) ) {
		return;
	}
	str = CG_SafeTranslateString_Internal( "cgame", "CGAME_FOLLOWSTOP" );
	trap_R_Text_Paint( 240, 446, 0, 10 / 48.0f, color,
					   va( str, binding ), 0, 0, 3 );
}

/*
=================
CG_DrawFollow                 0x30018710
=================
*/
static qboolean CG_DrawFollow( void ) {
	const char *s;
	int w;
	vec4_t color = { 1, 1, 1, 1 };

	if ( !( cg.snap->ps.pm_flags & 0x10000 ) ) {
		return qfalse;
	}

	if ( bg_clientinfo[cg.snap->ps.clientNum].infoValid ) {
		s = va( "CGAME_FOLLOWING\x15: %s", bg_clientinfo[cg.snap->ps.clientNum].name );
	} else {
		s = va( "CGAME_FOLLOWING\x15: %s", "?" );
	}
	s = trap_SE_LocalizeMessage( s, "spectator follow string" );

	w = trap_R_Text_Width( s, 0, 16 / 48.0f, 0 );
	trap_R_Text_Paint( ( 640 - w ) * 0.5f, 414, 0, 16 / 48.0f, color, s, 0, 0, 3 );

	return qtrue;
}

/*
=================
CG_Draw2D                     0x30018810
=================
*/
static void CG_Draw2D( void ) {
	if ( *(int *)cg.unknown_0x0000C ) {
		return;
	}
	if ( cg.unknown_0x00010 ) {
		return;
	}

	// if we are taking a levelshot for the menu, don't draw anything
	if ( CG_CAMERA_MODE || !cg_draw2D.integer ) {
		CG_DrawFlashFade();
		return;
	}

	CG_ScreenFade();
	CG_DrawFlashDamage();
	CG_DrawDamageDirectionIndicators();

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		return;
	}

	if ( cg.snap->ps.pm_type == PM_SPECTATOR ) {
		CG_DrawSpectator();
		CG_DrawCrosshairNames();
		CG_DrawChatMessages();
		if ( cg_drawStatus.integer ) {
			CG_DrawHudElems();
		}
	} else {
		if ( cg.snap->ps.pm_type < PM_DEAD ) {
			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();
		}
		CG_DrawChatMessages();
		if ( cg_drawStatus.integer ) {
			Menu_PaintAll();
			CG_DrawTimedMenus();
			CG_DrawHudElems();
		}
	}

	if ( !CG_DrawScoreboard() ) {
		CG_DrawSpectatorMessage();
		CG_DrawFollow();
	}

	CG_DrawVote();
	CG_DrawLagometer();
	CG_DrawPerformanceWarnings();
	CG_DrawDebugOverlays();
	CG_DrawUpperRight();

	if ( !CG_DrawScoreboard() ) {
		CG_DrawCenterString();
		CG_DrawGameMessages();
		CG_DrawBoldGameMessages();
		CG_DrawMiniConsole();
		CG_DrawSubtitles();
		CG_DrawSay();
	}

	CG_DrawFlashFade();
}

/*
=====================
CG_DrawSavedScreenBlend       0x30018920

Uncalled in 1.1; CG_DrawActive inlines the same call.
=====================
*/
void CG_DrawSavedScreenBlend( void ) {
	CG_DrawShellShockSavedScreenBlend( cg.shellshock.startTime, cg.shellshock.duration,
									   cg.shellshock.parms );
}

/*
=====================
CG_DrawActive                 0x30018940

Perform all drawing needed to completely fill the screen.  The no-snapshot half
is CoD's loading screen; RTCW draws that from CG_DrawInformation alone.
=====================
*/
/*
 * 0x300EEF28 is cg_info_mp.c's cg_drawingInformation, NOT a static of this
 * unit: CG_DrawActive's two accesses (0x3001895F, 0x30018973) hit the same
 * dword CG_DrawInformation, CG_DrawActiveFrame, CG_RegisterGraphics,
 * CG_RegisterWeapon, CG_SetConfigValues, CG_DrawObjectiveInfo and the three
 * Register traps use, so the recursion guard is shared.
 */
extern int  cg_drawingInformation;              /* cg_info.c, 0x300EEF28 */

void CG_DrawActive( int stereoView ) {
	float separation;
	vec3_t baseOrg;
	const char *info;
	qhandle_t levelshot;
	char buf[64];
	int expected;
	float frac;

	if ( !cg.snap ) {
		// no snapshot yet: this is the loading screen
		if ( cg_drawingInformation ) {
			return;
		}
		cg_drawingInformation = 1;

		if ( cl_serverloadmap.string[0] ) {
			trap_Cvar_Set( "cl_serverloadmap", "" );
		}
		if ( cl_serverloadgametype.string[0] ) {
			trap_Cvar_Set( "cl_serverloadgametype", "" );
		}
		if ( cl_serverloadwaiting.integer ) {
			trap_Cvar_Set( "cl_serverloadwaiting", "0" );
		}

		levelshot = 0;
		info = Info_ValueForKey( CG_ConfigString( CS_SERVERINFO ), "mapname" );
		if ( info && *info ) {
			levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", info ), 2 );
		}
		if ( !levelshot ) {
			levelshot = trap_R_RegisterShaderNoMip( "menu/art/unknownmap", 2 );
		}

		trap_R_SetColor( NULL );
		trap_R_DrawStretchPic( 0 * cgs.screenXScale, 0 * cgs.screenYScale,
							   640 * cgs.screenXScale, 480 * cgs.screenYScale,
							   0, 0, 1, 1, levelshot );

		trap_Cvar_VariableStringBuffer( "com_expectedhunkusage", buf, sizeof( buf ) );
		expected = atoi( buf );
		if ( expected > 0 ) {
			frac = trap_hunkUsed() / (float)expected;
			if ( frac > 1.0f ) {
				frac = 1.0f;
			}
			CG_HorizontalPercentBar( 200, 468, 240, 10, frac );
		}

		trap_UpdateScreen();
		cg_drawingInformation--;
		return;
	}

	// optionally draw the info screen instead
	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value * 0.5f;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value * 0.5f;
		break;
	default:
		separation = 0;
		CG_Error( "CG_DrawActive: Undefined stereoView" );
		break;
	}

	// clear around the rendered view if sized down
	baseOrg[0] = cg.refdef.vieworg[0];
	baseOrg[1] = cg.refdef.vieworg[1];
	baseOrg[2] = cg.refdef.vieworg[2];
	if ( separation != 0 ) {
		cg.refdef.vieworg[0] = cg.refdef.viewaxis[1][0] * -separation + cg.refdef.vieworg[0];
		cg.refdef.vieworg[1] = cg.refdef.viewaxis[1][1] * -separation + cg.refdef.vieworg[1];
		cg.refdef.vieworg[2] = cg.refdef.viewaxis[1][2] * -separation + cg.refdef.vieworg[2];
	}

	cg.refdef.rdflags |= 0x10;
	if ( !cg_skybox.integer ) {
		cg.refdef.rdflags &= ~0x10;
	}

	// draw 3D view
	trap_R_RenderScene( &cg.refdef );

	// restore original viewpoint if running stereo
	if ( separation != 0 ) {
		cg.refdef.vieworg[0] = baseOrg[0];
		cg.refdef.vieworg[1] = baseOrg[1];
		cg.refdef.vieworg[2] = baseOrg[2];
	}

	CG_DrawShellShockSavedScreenBlend( cg.shellshock.startTime, cg.shellshock.duration,
									   cg.shellshock.parms );
	CG_TileClear();

	// draw status bar and other overlays
	CG_Draw2D();
}
