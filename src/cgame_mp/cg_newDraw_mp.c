/*
 * cg_newDraw_mp.c -- the ownerdraw handlers the menu system dispatches into
 * cgame, plus the small "selected player" helpers that feed them.
 * (original source: cgame/cg_newDraw.c -- capital D)
 *
 * cgame_mp_x86.dll 0x30023AE0 .. 0x30027304, 50 functions in binary order.
 * RTCW's cgame/cg_newDraw.c is the ancestor.  What CoD changed:
 *
 *   - EVERY drawing handler gained a `font` argument.  CoD's displayContextDef
 *     ownerDrawItem is ( x, y, w, h, text_x, text_y, ownerDraw, ownerDrawFlags,
 *     align, special, FONT, scale, color, shader, textStyle ) -- one slot more
 *     than RTCW's -- and CG_OwnerDraw hands the font down to each handler.
 *     CG_OwnerDraw's stack offsets: arg_28 font, arg_2C scale, arg_30 color,
 *     arg_34 shader, arg_38 textStyle.
 *   - the bitmap font is gone, so RTCW's CG_Text_Paint/CG_Text_Width/
 *     CG_Text_Height wrappers do not exist here: every handler issues
 *     trap_R_Text_Paint / trap_R_Text_Width / trap_R_Text_Height itself, and
 *     CG_LoadHudMenu wires the traps straight into cgDC.
 *   - the whole CTF/flag/powerup/head/armour half of RTCW's file is gone.  In
 *     its place: the compass ownerdraws, the stance widget, the cursor hint,
 *     the weapon-name pair and the health bar -- all CoD additions.
 *   - CG_OwnerDraw is gated on cg_drawStatus.integer (0x30026BB0); RTCW has no
 *     such gate.
 *
 * CG_ClientNumFromName (0x300271F0) is unnamed in retail; it sits exactly
 * where RTCW puts it (between CG_KeyEvent and CG_ShowResponseHead) and is
 * RTCW's function with Q_stricmp replaced by Q_stricmpn( .., .., 99999 ).
 *
 * Sizes and offsets in comments are addresses in that DLL.
 *
 * @fidelity: likely
 */

#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <string.h>

#include "cg_local.h"

/*
 * ui_shared.h cannot be included here: it re-declares qhandle_t and defines
 * its own refdef_t / refEntity_t, which collide with cg_public.h's.  The one
 * type this unit needs from it is rectDef_t, so it is repeated under that
 * header's guard.
 */
#ifndef __UI_SHARED_H__
typedef struct {
	float x;
	float y;
	float w;
	float h;
} rectDef_t;
#endif

/* ui/ui_shared.c, statically linked into this DLL.  menuDef_t is opaque here. */
struct menuDef_s;
struct menuDef_s *Menus_FindByName( const char *p );
void        Menus_Open( struct menuDef_s *menu );
void        Controls_GetConfig( void );
qboolean    GetCommandHasBinding( const char *name );
int         GetKeyBindingLocalizedString( const char *name, char **out );

/* cg_draw_mp.c / cg_scoreboard_mp.c.  CG_DrawField keeps RTCW's declaration --
   it is the one cg_newDraw.c already carried at its head. */
int         CG_DrawField( int x, int y, int width, int value, int charWidth, int charHeight,
						  qboolean dodrawpic, qboolean leftAlign );
void        CG_ScrollScoreboardDown( void );
/* CG_DrawTeamBackground and CG_CheckForCursorHints are cg_draw_mp.c's and
   cg_local.h already declares them. */

/* game_mp/bg_weapon.c; bg_public.h does not declare this one. */
int         BG_GetTotalAmmoReserve( const playerState_t *ps, int weapon );

#ifndef PITCH
#define PITCH   0
#define YAW     1
#define ROLL    2
#endif

/* playerState_t.pm_flags.  game_mp/bg_local.h owns these four and cg_local.h
   does not pull it in, so they are repeated here under a guard; the values and
   addresses are bg_local.h's. */
#ifndef PMF_PRONE
#define PMF_PRONE               0x00000001
#define PMF_DUCKED              0x00000002
#define PMF_PRONE_BLOCKED       0x00008000
#endif
#ifndef PMF_INWORLD
#define PMF_INWORLD             0x00040000  /* g_client_mp.c's inferred name */
#endif

/* keycodes.h.  ui_mp/ui_shared.h carries the same six values, recovered from
   the engine's keynames[]; that header cannot be included here (see above). */
#ifndef K_PGDN
#define K_PGDN              163
#define K_PGUP              164
#define K_KP_PGUP           184
#define K_KP_PGDN           190
#define K_MWHEELDOWN        205
#define K_MWHEELUP          206
#endif

#ifndef STAT_HEALTH
#define STAT_HEALTH         0
#endif
#define STAT_MAX_HEALTH     2       /* ps.stats[2] (CG_DrawPlayerBarHealth 0x300248DC) */

#define ANGLE2SHORT( x )    ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )    ( (float)( x ) * ( 360.0f / 65536.0f ) )
#define DEG2RAD( a )        ( ( ( a ) * (float)M_PI ) / 180.0F )

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

/* g_local.h's team_t, which cgame cannot see; the two the colour tables
   distinguish are the two playing teams (CG_GetTeamColor 0x300272B3). */
#ifndef TEAM_AXIS
#define TEAM_FREE       0
#define TEAM_AXIS       1
#define TEAM_ALLIES     2
#endif

/* RTCW's cg_local.h constant; CoD's sortedTeamPlayers is the same 8 entries
   (0x3019E800 .. 0x3019E820). */
#define TEAM_MAXOVERLAY 8

/* SCORE_NOT_PRESENT, unchanged from RTCW (0x3002637A compares -9999). */
#define SCORE_NOT_PRESENT   -9999

/*
 * menudef.h's ownerdraw ids.  Every value below is the one CG_OwnerDraw
 * (0x30026BB0) and CG_GetValue (0x30026590) switch on.  4..79 keep RTCW's
 * numbering except where CoD reused a slot -- 5/6/70 became the ammo trio and
 * 20 became the stance widget (RTCW's CG_PLAYER_SCORE moved to 21).  81..91
 * are CoD's own and their names are inferred from what they draw.
 */
#define CG_PLAYER_HEALTH                4
#define CG_PLAYER_AMMO_VALUE            5
#define CG_PLAYER_AMMO_BACKDROP         6
#define CG_SELECTEDPLAYER_NAME          8
#define CG_SELECTEDPLAYER_LOCATION      9
#define CG_PLAYER_STANCE                20
#define CG_PLAYER_SCORE                 21
#define CG_BLUE_SCORE                   27
#define CG_RED_SCORE                    28
#define CG_PLAYER_LOCATION              33
#define CG_TEAM_COLOR                   34
#define CG_GAME_TYPE                    39
#define CG_SELECTEDPLAYER_HEALTH        41
#define CG_AREA_SYSTEMCHAT              46
#define CG_AREA_TEAMCHAT                47
#define CG_AREA_CHAT                    48
#define CG_KILLER                       50
#define CG_VOICE_NAME                   63
#define CG_1STPLACE                     67
#define CG_2NDPLACE                     68
#define CG_PLAYER_AMMOCLIP_VALUE        70
#define CG_CURSORHINT                   72
#define CG_AREA_WEAPON                  78
#define CG_AREA_HOLDABLE                79
#define CG_PLAYER_WEAPON_NAME           81
#define CG_PLAYER_WEAPON_NAME_BACK      82
#define CG_PLAYER_WEAPON_MODE_ICON      83
#define CG_PLAYER_COMPASS               84
#define CG_PLAYER_COMPASS_BACK          85
#define CG_PLAYER_COMPASS_POINTERS      86
#define CG_PLAYER_COMPASS_FRIENDLIES    88
#define CG_PLAYER_BAR_HEALTH            89
#define CG_PLAYER_BAR_HEALTH_TITLE      90
#define CG_MENU_SHADER                  91

/* menudef.h's ownerdrawflags, unchanged from RTCW (CG_OwnerDrawVisible). */
#define CG_SHOW_HEALTHCRITICAL          0x00000080
#define CG_SHOW_HEALTHOK                0x00004000
#define CG_SHOW_TEAMINFO                0x00008000
#define CG_SHOW_NOTEAMINFO              0x00010000

#define HINT_HEALTH             7       /* 0x30025926 */

/*
 * cg_weapons[] fields this unit reads.  cg_local.h holds +0x0A4.. open as one
 * `unknown_0x0A4[244]` hole; CG_RegisterWeapon (0x30035CF4 / 0x30035D56 /
 * 0x30035CB3) writes these three.
 */
#define CGWEAP_DISPLAYNAME( w ) ( *(const char **)( (byte *)&cg_weapons[w] + 0x0A4 ) )
#define CGWEAP_MODENAME( w )    ( *(const char **)( (byte *)&cg_weapons[w] + 0x0A8 ) )
#define CGWEAP_MODEICON( w )    ( *(qhandle_t *)( (byte *)&cg_weapons[w] + 0x11C ) )


/* Configstring bases.  cgame has no CS_ enum; both are the constant the
   inlined CG_ConfigString call adds (0x30024BCC, 0x30025773, 0x30025EF3). */
/* RTCW's q3print_t.  Only SYSTEM_PRINT is provable from this module --
   CG_SetPrintString compares `type` against zero and nothing else. */
#define SYSTEM_PRINT        0

#define CS_SHADERS          1500
#define CS_HINTSTRINGS      1212
#define CS_LOCATIONS        44

/*
=============================================================================

    FILE-SCOPE STATE

    sortedTeamPlayers / numSortedTeamPlayers live in RTCW's cg_draw.c; in CoD
    nothing outside this file touches them, so they are this unit's.  The three
    chat lines are RTCW's statics at the head of cg_newDraw.c and keep their
    retail order in memory: teamChat1, then numSortedTeamPlayers, then
    systemChat and teamChat2.

=============================================================================
*/

int         sortedTeamPlayers[TEAM_MAXOVERLAY];         /* 0x3019E800 */
static char teamChat1[256];                             /* 0x3019E820 */
int         numSortedTeamPlayers;                       /* 0x3019E920 */
static char systemChat[256];                            /* 0x3019E940 */
static char teamChat2[256];                             /* 0x3019EA40 */

/* CG_DrawPlayerStance's latches (0x30074984, 0x30074988, 0x3007498C). */
static int  stanceHintTime = -1;
static int  stanceLast = -1;
static int  proneBlockedTime = -1;

/* CG_DrawPlayerBarHealth's lagging-damage bar (0x30074990, 0x30074994,
   0x30074998, 0x300EEF1C).  barHoldTime is never written by retail. */
static int      barHoldTime = 1;
static int      barLagHoldTime = 1;
static int      barLastClient = -1;
static float    barLagFrac;

/* CG_GetTranslatedLocationString's return buffer, 0x400 bytes (0x30095B20). */
static char locationString[MAX_STRING_CHARS];

/*
================
CG_SetPrintString             0x30023A80  (unnamed in retail)

RTCW's function verbatim, and the only writer of the three chat buffers.  Both
strcpy calls are expanded inline, and the else branch got its own out-of-line
head at 0x30023A9B (teamChat2 <- teamChat1 at 0x30023AA0, teamChat1 <- p at
0x30023AB8).  Dead in 1.1: nothing references it.

`type` reaches the body in ecx and is only ever tested against zero, so
RTCW's SYSTEM_PRINT is the 0 the comparison tests and the other two
q3print_t values are not distinguishable from here.
================
*/
void CG_SetPrintString( int type, const char *p ) {
	if ( type == SYSTEM_PRINT ) {
		strcpy( systemChat, p );
	} else {
		strcpy( teamChat2, teamChat1 );
		strcpy( teamChat1, p );
	}
}

/*
================
CG_SetSelectedPlayerName      0x30023AE0

RTCW also pushes cg_selectedPlayerName; CoD sets only the index.
================
*/
static void CG_SetSelectedPlayerName( void ) {
	if ( cg_currentSelectedPlayer.integer >= 0 && cg_currentSelectedPlayer.integer < numSortedTeamPlayers ) {
		trap_Cvar_Set( "cg_selectedPlayer", va( "%d", sortedTeamPlayers[cg_currentSelectedPlayer.integer] ) );
	}
}

/*
================
CG_GetSelectedPlayer          0x30023B20
================
*/
int CG_GetSelectedPlayer( void ) {
	if ( cg_currentSelectedPlayer.integer < 0 || cg_currentSelectedPlayer.integer >= numSortedTeamPlayers ) {
		cg_currentSelectedPlayer.integer = 0;
	}
	return cg_currentSelectedPlayer.integer;
}

/*
================
CG_SelectNextPlayer           0x30023B40

RTCW's function without its leading CG_CheckOrderPending (CoD has no order
system).  CG_SetSelectedPlayerName is inlined at the tail of both selectors.
================
*/
void CG_SelectNextPlayer( void ) {
	if ( cg_currentSelectedPlayer.integer >= 0 && cg_currentSelectedPlayer.integer < numSortedTeamPlayers ) {
		cg_currentSelectedPlayer.integer++;
	} else {
		cg_currentSelectedPlayer.integer = 0;
	}
	CG_SetSelectedPlayerName();
}

/*
================
CG_SelectPrevPlayer           0x30023B90
================
*/
void CG_SelectPrevPlayer( void ) {
	if ( cg_currentSelectedPlayer.integer > 0 && cg_currentSelectedPlayer.integer < numSortedTeamPlayers ) {
		cg_currentSelectedPlayer.integer--;
	} else {
		cg_currentSelectedPlayer.integer = numSortedTeamPlayers;
	}
	CG_SetSelectedPlayerName();
}

/*
================
CG_DrawPlayerAmmoBackdrop     0x30023BE0

RETAIL: __usercall( rect@<esi>, color, shader ).
RTCW's CG_DrawPlayerAmmoIcon; the icon became a plain backdrop.
================
*/
static void CG_DrawPlayerAmmoBackdrop( rectDef_t *rect, vec4_t color, qhandle_t shader ) {
	if ( cg.predictedPlayerState.weapon ) {
		trap_R_SetColor( color );
		CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
		trap_R_SetColor( NULL );
	}
}

/*
================
CG_DrawPlayerWeaponName       0x30023C30

RETAIL: __usercall( rect@<edi>, font@<ebx>, color@<eax>, scale, textStyle ).
Right-aligned, 28 units in from the right edge.
================
*/
static void CG_DrawPlayerWeaponName( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	float       *fade;
	vec4_t      drawColor;
	weaponInfo_t *weapDef;
	const char  *text;
	int         width;
	int         weap;

	fade = CG_FadeColor( cg.weaponSelectTime, 1800 );
	if ( !fade ) {
		return;
	}

	drawColor[0] = color[0];
	drawColor[1] = color[1];
	drawColor[2] = color[2];
	drawColor[3] = fade[3];

	weap = cg_weaponSelect.integer;
	if ( weap >= 0 && weap < bg_numWeapons
		 && ( cg.predictedPlayerState.weapons[weap >> 5] & ( 1 << ( weap & 31 ) ) ) ) {
		weapDef = bg_weaponInfo[weap];
	} else {
		weapDef = cg.weaponInfo;
	}
	if ( !weapDef->weapIndex ) {
		return;
	}

	if ( weapDef->modeName[0] ) {
		text = va( "%s / %s", CGWEAP_DISPLAYNAME( weapDef->weapIndex ),
				   CGWEAP_MODENAME( weapDef->weapIndex ) );
	} else {
		text = va( "%s", CGWEAP_DISPLAYNAME( weapDef->weapIndex ) );
	}

	width = trap_R_Text_Width( text, font, scale, 0 );
	trap_R_Text_Paint( rect->x + rect->w - width - 28.0f, rect->y, font, scale,
					   drawColor, text, 0, 0, textStyle );
}

/*
================
CG_DrawPlayerWeaponNameBack   0x30023D50

RETAIL: __usercall( rect@<esi>, color@<eax>, font, scale, shader ).
The plate behind CG_DrawPlayerWeaponName: same right edge, sized to the text
plus 36.
================
*/
static void CG_DrawPlayerWeaponNameBack( rectDef_t *rect, int font, float scale, vec4_t color, qhandle_t shader ) {
	float       *fade;
	vec4_t      drawColor;
	weaponInfo_t *weapDef;
	const char  *text;
	float       width, x;
	int         weap;

	fade = CG_FadeColor( cg.weaponSelectTime, 1800 );
	if ( !fade ) {
		return;
	}

	drawColor[0] = color[0];
	drawColor[1] = color[1];
	drawColor[2] = color[2];
	drawColor[3] = fade[3];

	weap = cg_weaponSelect.integer;
	if ( weap >= 0 && weap < bg_numWeapons
		 && ( cg.predictedPlayerState.weapons[weap >> 5] & ( 1 << ( weap & 31 ) ) ) ) {
		weapDef = bg_weaponInfo[weap];
	} else {
		weapDef = cg.weaponInfo;
	}
	if ( !weapDef->weapIndex ) {
		return;
	}

	if ( weapDef->modeName[0] ) {
		text = va( "%s / %s", CGWEAP_DISPLAYNAME( weapDef->weapIndex ),
				   CGWEAP_MODENAME( weapDef->weapIndex ) );
	} else {
		text = va( "%s", CGWEAP_DISPLAYNAME( weapDef->weapIndex ) );
	}

	width = trap_R_Text_Width( text, font, scale, 0 ) + 36.0f;
	x = rect->x + rect->w - width;

	trap_R_SetColor( drawColor );
	CG_DrawPic( x, rect->y, width, rect->h, shader );
	trap_R_SetColor( NULL );
}

/*
================
CG_DrawPlayerWeaponModeIcon   0x30023E90

RETAIL: __usercall( rect@<esi>, color ).
================
*/
static void CG_DrawPlayerWeaponModeIcon( rectDef_t *rect, vec4_t color ) {
	weaponInfo_t *weapDef;
	qhandle_t   shader;
	int         weap;

	if ( !cg.predictedPlayerState.weapon && !CG_FadeColor( cg.weaponSelectTime, 1800 ) ) {
		return;
	}

	weap = cg_weaponSelect.integer;
	if ( weap >= 0 && weap < bg_numWeapons
		 && ( cg.predictedPlayerState.weapons[weap >> 5] & ( 1 << ( weap & 31 ) ) ) ) {
		weapDef = bg_weaponInfo[weap];
	} else {
		weapDef = cg.weaponInfo;
	}
	if ( !weapDef->weapIndex ) {
		return;
	}

	shader = CGWEAP_MODEICON( weapDef->weapIndex );
	if ( !shader ) {
		return;
	}

	trap_R_SetColor( color );
	CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
	trap_R_SetColor( NULL );
}

/*
================
CG_DrawPlayerStance           0x30023F50

CoD addition, no RTCW counterpart.  Three jobs in one ownerdraw: the
"prone blocked" flash across the middle of the screen, the four stance-change
key hints beside the widget, and the stance icon itself (plus the colour flash
that follows a stance change).

The three command tables are the ones the retail function builds on the stack;
the row index is the hint being offered (jump / stand / crouch / prone) and the
first command in the row with a real binding is the one whose key is shown.
================
*/
static void CG_DrawPlayerStance( rectDef_t *rect, vec4_t color, int font, float scale, int textStyle ) {
	const char *hintNames[4] = {
		"CGAME_STANCEHINT_JUMP",
		"CGAME_STANCEHINT_STAND",
		"CGAME_STANCEHINT_CROUCH",
		"CGAME_STANCEHINT_PRONE"
	};
	const char  *proneCmds[4][6];
	const char  *duckCmds[4][6];
	const char  *standCmds[4][6];
	const char  *( *cmds )[6];
	const char  *bind[4];
	char        *keyString;
	const char  *text;
	vec4_t      drawColor;
	qhandle_t   shader;
	float       xBase, y;
	int         textHeight;
	int         count;
	int         i, j;

	if ( !cg_hudStanceHintPrints.integer ) {
		stanceHintTime = -1;
	} else if ( stanceHintTime > cg.time
				|| stanceLast != ( cg.predictedPlayerState.pm_flags & ( PMF_PRONE | PMF_DUCKED ) ) ) {
		stanceHintTime = cg.time;
	}

	drawColor[0] = color[0];
	drawColor[1] = color[1];
	drawColor[2] = color[2];

	xBase = ( cg_hudCompassSize.value - 1.0f ) * 112.0f + rect->x;
	stanceLast = cg.predictedPlayerState.pm_flags & ( PMF_PRONE | PMF_DUCKED );

	if ( cg.predictedPlayerState.pm_flags & PMF_PRONE_BLOCKED ) {
		if ( proneBlockedTime < cg.time ) {
			proneBlockedTime = cg.time + 1500;
		}
	}
	if ( proneBlockedTime > cg.time ) {
		text = CG_SafeTranslateString_Internal( "cgame", "CGAME_PRONE_BLOCKED" );
		i = trap_R_Text_Width( text, font, scale, 0 );
		drawColor[3] = (float)fabs( sin( DEG2RAD( ( ( proneBlockedTime - cg.time ) / 1500.0f ) * 540.0f ) ) );
		trap_R_Text_Paint( 320.0f - i * 0.5f, 270.0f, font, scale,
						   drawColor, text, 0, 0, textStyle );
	}

	if ( stanceHintTime + 3000 > cg.time ) {
		standCmds[0][0] = "+gostand";
		standCmds[0][1] = "+moveup";
		standCmds[0][2] = NULL;
		standCmds[0][3] = NULL;
		standCmds[0][4] = NULL;
		standCmds[0][5] = NULL;
		standCmds[1][0] = NULL;
		standCmds[1][1] = NULL;
		standCmds[1][2] = NULL;
		standCmds[1][3] = NULL;
		standCmds[1][4] = NULL;
		standCmds[1][5] = NULL;
		standCmds[2][0] = "gocrouch";
		standCmds[2][1] = "togglecrouch";
		standCmds[2][2] = "lowerstance";
		standCmds[2][3] = "+movedown";
		standCmds[2][4] = NULL;
		standCmds[2][5] = NULL;
		standCmds[3][0] = "goprone";
		standCmds[3][1] = "+prone";
		standCmds[3][2] = NULL;
		standCmds[3][3] = NULL;
		standCmds[3][4] = NULL;
		standCmds[3][5] = NULL;

		duckCmds[0][0] = NULL;
		duckCmds[0][1] = NULL;
		duckCmds[0][2] = NULL;
		duckCmds[0][3] = NULL;
		duckCmds[0][4] = NULL;
		duckCmds[0][5] = NULL;
		duckCmds[1][0] = "+gostand";
		duckCmds[1][1] = "raisestance";
		duckCmds[1][2] = "+moveup";
		duckCmds[1][3] = NULL;
		duckCmds[1][4] = NULL;
		duckCmds[1][5] = NULL;
		duckCmds[2][0] = NULL;
		duckCmds[2][1] = NULL;
		duckCmds[2][2] = NULL;
		duckCmds[2][3] = NULL;
		duckCmds[2][4] = NULL;
		duckCmds[2][5] = NULL;
		duckCmds[3][0] = "goprone";
		duckCmds[3][1] = "lowerstance";
		duckCmds[3][2] = "toggleprone";
		duckCmds[3][3] = "+prone";
		duckCmds[3][4] = NULL;
		duckCmds[3][5] = NULL;

		proneCmds[0][0] = NULL;
		proneCmds[0][1] = NULL;
		proneCmds[0][2] = NULL;
		proneCmds[0][3] = NULL;
		proneCmds[0][4] = NULL;
		proneCmds[0][5] = NULL;
		proneCmds[1][0] = "+gostand";
		proneCmds[1][1] = "toggleprone";
		proneCmds[1][2] = NULL;
		proneCmds[1][3] = NULL;
		proneCmds[1][4] = NULL;
		proneCmds[1][5] = NULL;
		proneCmds[2][0] = "gocrouch";
		proneCmds[2][1] = "togglecrouch";
		proneCmds[2][2] = "raisestance";
		proneCmds[2][3] = "+movedown";
		proneCmds[2][4] = "+moveup";
		proneCmds[2][5] = NULL;
		proneCmds[3][0] = NULL;
		proneCmds[3][1] = NULL;
		proneCmds[3][2] = NULL;
		proneCmds[3][3] = NULL;
		proneCmds[3][4] = NULL;
		proneCmds[3][5] = NULL;

		Controls_GetConfig();

		if ( stanceHintTime + 2000 > cg.time ) {
			drawColor[3] = 1.0f;
		} else {
			drawColor[3] = ( stanceHintTime - cg.time + 3000 ) / 1000.0f;
		}

		textHeight = trap_R_Text_Height( font, scale );

		count = 0;
		for ( i = 0; i < 4; i++ ) {
			bind[i] = NULL;
			if ( stanceLast & PMF_PRONE ) {
				cmds = proneCmds;
			} else if ( stanceLast & PMF_DUCKED ) {
				cmds = duckCmds;
			} else {
				cmds = standCmds;
			}
			for ( j = 0; j < 6 && cmds[i][j]; j++ ) {
				if ( GetCommandHasBinding( cmds[i][j] ) ) {
					bind[i] = cmds[i][j];
					count++;
					break;
				}
			}
		}

		y = rect->y + rect->h * 0.5f - 1.5f;
		if ( count == 1 ) {
			y = y + textHeight * 0.5f;
		} else if ( count == 3 ) {
			y = y - ( textHeight * 0.5f + 1.5f );
		}

		for ( i = 0; i < 4; i++ ) {
			if ( !bind[i] ) {
				continue;
			}
			GetKeyBindingLocalizedString( bind[i], &keyString );
			text = va( CG_SafeTranslateString_Internal( "cgame", hintNames[i] ), keyString );
			trap_R_Text_Paint( xBase + rect->w, y, font, scale,
							   drawColor, text, 0, 0, textStyle );
			y = y + textHeight + 1.5f;
		}
	}

	if ( stanceLast & PMF_PRONE ) {
		shader = cgs.media.hudStanceProne;
	} else if ( stanceLast & PMF_DUCKED ) {
		shader = cgs.media.hudStanceCrouch;
	} else {
		shader = cgs.media.hudStanceStand;
	}

	drawColor[3] = color[3];
	trap_R_SetColor( drawColor );
	trap_R_DrawStretchPic( cgs.screenXScale * xBase, cgs.screenYScale * rect->y,
						   cgs.screenXScale * rect->w, cgs.screenYScale * rect->h,
						   0, 0, 1, 1, shader );

	if ( stanceHintTime + 1000 > cg.time ) {
		if ( cg_hudStanceFlash_r.value < 0 ) {
			drawColor[0] = 0;
		} else if ( cg_hudStanceFlash_r.value > 1 ) {
			drawColor[0] = 1.0f;
		} else {
			drawColor[0] = cg_hudStanceFlash_r.value;
		}
		if ( cg_hudStanceFlash_g.value < 0 ) {
			drawColor[1] = 0;
		} else if ( cg_hudStanceFlash_g.value > 1 ) {
			drawColor[1] = 1.0f;
		} else {
			drawColor[1] = cg_hudStanceFlash_g.value;
		}
		if ( cg_hudStanceFlash_b.value < 0 ) {
			drawColor[2] = 0;
		} else if ( cg_hudStanceFlash_b.value > 1 ) {
			drawColor[2] = 1.0f;
		} else {
			drawColor[2] = cg_hudStanceFlash_b.value;
		}
		drawColor[3] = ( ( stanceHintTime - cg.time + 1000 ) / 1000.0f ) * 0.8f;

		trap_R_SetColor( drawColor );
		trap_R_DrawStretchPic( cgs.screenXScale * xBase, cgs.screenYScale * rect->y,
							   cgs.screenXScale * rect->w, cgs.screenYScale * rect->h,
							   0, 0, 1, 1, cgs.media.hudStanceFlash );
	}

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawPlayerCompass          0x30024800

RETAIL: __usercall( rect@<eax>, shader, color ).
The rose scales about ( -25, 345 ) and slides up as cg_hudCompassSize grows.
================
*/
static void CG_DrawPlayerCompass( rectDef_t *rect, qhandle_t shader, vec4_t color ) {
	float x, y, w, h;

	x = ( rect->x - -25.0f ) * cg_hudCompassSize.value - 25.0f;
	y = ( rect->y - 345.0f ) * cg_hudCompassSize.value + 345.0f
		- ( cg_hudCompassSize.value - 1.0f ) * 160.0f;
	w = cg_hudCompassSize.value * rect->w;
	h = cg_hudCompassSize.value * rect->h;

	CG_UpdateCompassOrientation();

	trap_R_SetColor( color );
	CG_DrawRotatedPic( x, y, w, h, cg.compassPlayerAngle, shader );
	trap_R_SetColor( NULL );
}

/*
================
CG_DrawPlayerBarHealth        0x300248C0

RETAIL: __usercall( rect@<edi>, color@<esi>, shader ).  `color` is written
through: the bar tints itself red as it empties, and the trailing damage bar
overwrites it outright.
================
*/
static void CG_DrawPlayerBarHealth( rectDef_t *rect, vec4_t color, qhandle_t shader ) {
	playerState_t *ps;
	float frac;

	ps = &cg.snap->ps;

	if ( ps->stats[STAT_HEALTH] && ps->stats[STAT_MAX_HEALTH] ) {
		frac = (float)ps->stats[STAT_HEALTH] / ps->stats[STAT_MAX_HEALTH];
		if ( frac < 0 ) {
			frac = 0;
		} else if ( frac > 1 ) {
			frac = 1.0f;
		}
	} else {
		frac = 0;
	}

	if ( frac > 0 ) {
		if ( frac > 0.5f ) {
			color[0] = ( 1.0f - frac ) * color[0] * 2;
			color[2] = ( 1.0f - frac ) * color[2] * 2;
		} else {
			color[1] = ( frac + 0.2f ) * color[1] + 0.3f;
		}
		trap_R_SetColor( color );
		trap_R_DrawStretchPic( cgs.screenXScale * rect->x, rect->y * cgs.screenYScale,
							   frac * rect->w * cgs.screenXScale, rect->h * cgs.screenYScale,
							   0, 0, frac, 1.0f, shader );
	}

	// the trailing bar that drains down to the new value after damage
	if ( barLastClient != ps->clientNum ) {
		barLastClient = ps->clientNum;
		barLagFrac = frac;
		barLagHoldTime = barHoldTime;
	} else if ( frac < barLagFrac ) {
		if ( barLagHoldTime ) {
			barLagHoldTime -= cg.frametime;
			if ( barLagHoldTime < 0 ) {
				barLagHoldTime = 0;
			}
		} else {
			barLagFrac = barLagFrac - cg.frametime * 0.0012f;
			if ( barLagFrac <= frac ) {
				barLagFrac = frac;
				barLagHoldTime = barHoldTime;
			}
		}
	} else {
		barLagFrac = frac;
		barLagHoldTime = barHoldTime;
	}

	if ( barLagFrac > frac ) {
		color[0] = 1.0f;
		color[1] = 0;
		color[2] = 0;
		trap_R_SetColor( color );
		trap_R_DrawStretchPic( ( frac * rect->w + rect->x ) * cgs.screenXScale,
							   rect->y * cgs.screenYScale,
							   ( barLagFrac - frac ) * rect->w * cgs.screenXScale,
							   rect->h * cgs.screenYScale,
							   frac, 0, barLagFrac, 1.0f, shader );
	}

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawPlayerBarHealthTitle   0x30024B50

RETAIL: __usercall( rect@<esi>, font, scale, color, textStyle ).
================
*/
static void CG_DrawPlayerBarHealthTitle( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	const char *text;

	text = CG_SafeTranslateString_Internal( "cgame", "CGAME_HEALTH" );
	trap_R_Text_Paint( rect->x, rect->y, font, scale, color, text, 0, 0, textStyle );
}

/*
================
CG_ServerShaderName           0x30024BC0

RETAIL: __usercall( index@<eax>, buffer, bufferSize ).  CG_ConfigString is
inlined at 0x30024BD2.
================
*/
/* NOT static: cg_hudelem_mp.c's clock and shader elements call it. */
qboolean CG_ServerShaderName( int index, char *buffer, int bufferSize ) {
	const char *s;

	if ( index <= 0 || index >= 128 ) {
		return qfalse;
	}

	s = CG_ConfigString( CS_SHADERS + index );
	if ( !s[0] ) {
		return qfalse;
	}
	if ( strlen( s ) >= (unsigned int)bufferSize ) {
		return qfalse;
	}

	strcpy( buffer, s );
	return qtrue;
}

/*
================
CG_ObjectiveIcon              0x30024C40

RETAIL: __usercall( dir@<ecx>, shaderIndex ).  `dir` is 0 for level, 1 for an
objective above the player and 2 for one below; the suffix is appended to the
server-supplied shader name, and the three cgs.media handles are the fallback.
trap_R_RegisterShaderNoMip is inlined (0x30024CDA is its CG_DrawInformation).
================
*/
static qhandle_t CG_ObjectiveIcon( int dir, int shaderIndex ) {
	const char *suffix[3] = { "", "_up", "_down" };
	char name[MAX_QPATH];
	char *s;

	if ( shaderIndex ) {
		if ( CG_ServerShaderName( shaderIndex, name, sizeof( name ) ) ) {
			for ( s = name; *s && *s != '.'; s++ ) {
			}
			*s = '\0';
			strcat( name, suffix[dir] );
			return trap_R_RegisterShaderNoMip( name, 5 );
		}
	}

	return ( &cgs.media.hudObjective )[dir];
}

/*
================
CG_DrawPlayerCompassPointers  0x30024D20

RETAIL: __usercall( rect@<eax>, shader, color ) -- the shader argument is never
read; the icon comes from CG_ObjectiveIcon.
================
*/
static void CG_DrawPlayerCompassPointers( rectDef_t *rect, qhandle_t shader, vec4_t color ) {
	playerState_t *ps;
	objective_t *obj;
	centity_t   *cent;
	vec4_t      drawColor;
	vec3_t      pos, dir;
	qhandle_t   icon;
	float       centerX, centerY;
	float       x, y, size, radius, dist, alpha, angle, t;
	float       s, c;
	int         i;

	CG_UpdateCompPointerOrientation();

	centerX = cg_hudCompassSize.value * rect->w * 0.5f + rect->x;
	centerY = cg_hudCompassSize.value * rect->h * 0.5f + rect->y
			  - ( cg_hudCompassSize.value - 1.0f ) * 160.0f;

	ps = &cg.snap->ps;
	if ( !bg_clientinfo[ps->clientNum].infoValid ) {
		return;
	}

	for ( i = 0; i < 16; i++ ) {
		obj = &ps->objective[i];
		if ( obj->state != 4 ) {
			continue;
		}

		if ( obj->entNum != ENTITYNUM_NONE ) {
			cent = &cg_entities[obj->entNum];
			pos[0] = cent->lerpOrigin[0];
			pos[1] = cent->lerpOrigin[1];
			pos[2] = cent->lerpOrigin[2];
		} else {
			pos[0] = obj->origin[0];
			pos[1] = obj->origin[1];
			pos[2] = obj->origin[2];
		}

		dir[0] = pos[0] - cg.refdef.vieworg[0];
		dir[1] = pos[1] - cg.refdef.vieworg[1];
		dir[2] = pos[2] - cg.refdef.vieworg[2];

		angle = (float)SHORT2ANGLE( ANGLE2SHORT( vectoyaw( dir ) - cg.compassPointerAngle ) );
		dist = (float)sqrt( dir[1] * dir[1] + dir[0] * dir[0] );

		drawColor[0] = color[0];
		drawColor[1] = color[1];
		drawColor[2] = color[2];

		if ( dist <= cg_hudCompassMaxRange.value ) {
			alpha = 1.0f;
		} else if ( dist < cg_hudObjectiveMaxRange.value ) {
			t = 0;
			if ( cg_hudObjectiveMaxRange.value - cg_hudCompassMaxRange.value != 0 ) {
				t = ( dist - cg_hudCompassMaxRange.value )
					/ ( cg_hudObjectiveMaxRange.value - cg_hudCompassMaxRange.value );
			}
			alpha = 1.0f - ( 1.0f - cg_hudObjectiveMinAlpha.value ) * t;
		} else {
			alpha = cg_hudObjectiveMinAlpha.value;
		}
		drawColor[3] = alpha;

		if ( dist <= cg_hudCompassMinRange.value ) {
			radius = cg_hudCompassMinRadius.value;
		} else if ( dist < cg_hudCompassMaxRange.value ) {
			t = 0;
			if ( cg_hudCompassMaxRange.value - cg_hudCompassMinRange.value != 0 ) {
				t = ( cg_hudCompassMaxRange.value - dist )
					/ ( cg_hudCompassMaxRange.value - cg_hudCompassMinRange.value );
			}
			radius = 1.0f - ( 1.0f - cg_hudCompassMinRadius.value ) * t;
		} else {
			radius = 1.0f;
		}
		/* 43.75 is a double in retail (dbl_30069728), not a float literal. */
		radius = (float)( cg_hudCompassSize.value * radius * 43.75 );

		angle = angle * (float)M_PI / 180.0f;
		c = (float)cos( angle );
		s = (float)sin( angle );

		size = cg_hudCompassSize.value * 20.0f;
		x = centerX - size * 0.5f - s * radius;
		y = centerY - size * 0.5f - c * radius;

		if ( dir[2] > cg_hudObjectiveMaxHeight.value ) {
			icon = CG_ObjectiveIcon( 1, obj->icon );
		} else if ( dir[2] < cg_hudObjectiveMinHeight.value ) {
			icon = CG_ObjectiveIcon( 2, obj->icon );
		} else {
			icon = CG_ObjectiveIcon( 0, obj->icon );
		}

		trap_R_SetColor( drawColor );
		trap_R_DrawStretchPic( cgs.screenXScale * x, cgs.screenYScale * y,
							   cgs.screenXScale * size, cgs.screenYScale * size,
							   0, 0, 1.0f, 1.0f, icon );
	}

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawPlayerCompassBack      0x30025120

RETAIL: __usercall( rect@<eax>, shader, color ).  CG_DrawPlayerCompass's
geometry, drawn unrotated and in screen space.
================
*/
static void CG_DrawPlayerCompassBack( rectDef_t *rect, qhandle_t shader, vec4_t color ) {
	float x, y, w, h;

	x = ( ( rect->x - -25.0f ) * cg_hudCompassSize.value - 25.0f ) * cgs.screenXScale;
	y = ( ( rect->y - 345.0f ) * cg_hudCompassSize.value + 345.0f
		  - ( cg_hudCompassSize.value - 1.0f ) * 160.0f ) * cgs.screenYScale;
	w = cg_hudCompassSize.value * rect->w * cgs.screenXScale;
	h = cg_hudCompassSize.value * rect->h * cgs.screenYScale;

	trap_R_SetColor( color );
	trap_R_DrawStretchPic( x, y, w, h, 0, 0, 1.0f, 1.0f, shader );
	trap_R_SetColor( NULL );
}

/*
================
CG_DrawCursorhint             0x300251F0

CoD addition.  cg.cursorHintIcon selects both the icon and the message: 2..9
are the fixed hints, 10..73 a weapon pickup, 74..137 an ammo pickup, and
anything else falls back on cg.cursorHintString (a configstring) or the health
pickup.  The icon pulses or grows with cg_cursorHints.
================
*/
static void CG_DrawCursorhint( rectDef_t *rect, int font, float scale, int textStyle ) {
	float       *fade;
	qhandle_t   shader;
	weaponInfo_t *weapDef;
	const char  *s;
	const char  *text;
	char        *keyString;
	char        buffer[MAX_STRING_CHARS];   /* ebp-0x804, 0x400 bytes */
	char        lower[MAX_STRING_CHARS];    /* ebp-0x404 */
	float       widthScale, xOfs, grow, halfGrow;
	int         icon, weapon, slot;
	int         width, height;
	char        *p;
	int         n;

	widthScale = 1.0f;
	xOfs = 0;

	if ( !cg_cursorHints.integer ) {
		return;
	}

	CG_CheckForCursorHints();

	icon = cg.cursorHintIcon;
	shader = cgs.media.hintShaders[icon];
	if ( icon >= 0 && icon <= 1 ) {
		return;
	}
	if ( !shader ) {
		return;
	}

	fade = CG_FadeColor( cg.cursorHintTime, cg.cursorHintFade );
	if ( !fade ) {
		trap_R_SetColor( NULL );
		cg.cursorHintIcon = 0;
		return;
	}

	if ( cg_cursorHints.integer == 3 ) {
		fade[3] = ( (float)sin( cg.time * 0.0066666668f ) + 1.0f ) * fade[3] * 0.5f;
	}
	if ( cg_cursorHints.integer >= 3 ) {
		halfGrow = 0;
		grow = 0;
	} else {
		if ( cg_cursorHints.integer == 2 ) {
			grow = ( cg.cursorHintTime % 1000 ) * 0.01f;
		} else {
			grow = ( ( (float)sin( cg.time * 0.0066666668f ) + 1.0f ) * 0.5f ) * 10.0f;
		}
		halfGrow = grow * 0.5f;
	}

	if ( icon >= HINT_WEAPON_FIRST && icon <= HINT_WEAPON_LAST ) {
		weapon = icon - ( HINT_WEAPON_FIRST - 1 );
		weapDef = bg_weaponInfo[weapon];

		if ( weapDef->wideListIcon ) {
			widthScale = 2.0f;
			xOfs = rect->w * -0.5f;
		}

		if ( !BG_GetEmptySlotForWeapon( &cg.predictedPlayerState, weapon ) ) {
			Controls_GetConfig();
			GetKeyBindingLocalizedString( "+activate", &keyString );

			slot = weapDef->weaponSlot;
			if ( bg_weaponInfo[cg.predictedPlayerState.weapon]->weaponSlot == slot ) {
				if ( (int)cg.predictedPlayerState.weapon == weapon ) {
					return;
				}
				text = va( "%s %s %s %s",
						   va( CG_SafeTranslateString_Internal( "cgame", "CGAME_SWAPWEAPONS" ), keyString ),
						   CGWEAP_DISPLAYNAME( cg.predictedPlayerState.weapon ),
						   CG_SafeTranslateString_Internal( "cgame", "CGAME_FOR" ),
						   CGWEAP_DISPLAYNAME( weapon ) );
			} else {
				if ( cg.predictedPlayerState.weaponslots[slot] == weapon ) {
					return;
				}
				text = va( "%s %s %s %s",
						   va( CG_SafeTranslateString_Internal( "cgame", "CGAME_SWAPWEAPONS" ), keyString ),
						   CGWEAP_DISPLAYNAME( cg.predictedPlayerState.weaponslots[weapDef->weaponSlot] ),
						   CG_SafeTranslateString_Internal( "cgame", "CGAME_FOR" ),
						   CGWEAP_DISPLAYNAME( weapon ) );
			}
		} else {
			s = CG_SafeTranslateString_Internal( "cgame", "CGAME_PICKUPNEWWEAPON" );
			p = strstr( s, "[%s]" );
			if ( p && *p ) {
				Controls_GetConfig();
				GetKeyBindingLocalizedString( "+activate", &keyString );
				p++;
				strncpy( buffer, s, p - s );
				buffer[p - s] = '\0';
				strcat( buffer, keyString );
				strcat( buffer, p + 2 );
			} else {
				strcpy( buffer, s );
			}
			text = va( buffer, CGWEAP_DISPLAYNAME( weapon ) );
		}

		width = trap_R_Text_Width( text, font, scale, 0 );
		height = trap_R_Text_Height( font, scale );
		trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y - height,
						   font, scale, fade, text, 0, 0, textStyle );

	} else if ( icon >= HINT_AMMO_FIRST && icon <= HINT_AMMO_LAST ) {
		weapon = icon - ( HINT_AMMO_FIRST - 1 );

		s = CG_SafeTranslateString_Internal( "cgame", "CGAME_PICKUPAMMO" );
		p = strstr( s, "[%s]" );
		if ( p && *p ) {
			Controls_GetConfig();
			GetKeyBindingLocalizedString( "+activate", &keyString );
			p++;
			strncpy( buffer, s, p - s );
			buffer[p - s] = '\0';
			strcat( buffer, keyString );
			strcat( buffer, p + 2 );
		} else {
			strcpy( buffer, s );
		}
		text = va( buffer, CGWEAP_DISPLAYNAME( weapon ) );

		width = trap_R_Text_Width( text, font, scale, 0 );
		height = trap_R_Text_Height( font, scale );
		trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y - height,
						   font, scale, fade, text, 0, 0, textStyle );

	} else if ( cg.cursorHintString >= 0 ) {
		s = CG_ConfigString( CS_HINTSTRINGS + cg.cursorHintString );
		if ( s && s[0] ) {
			/* the "[%s]" is found in a lowercased copy but spliced out of the
			   translated original */
			s = CG_TranslateMessage( s, "Hint String" );
			strcpy( lower, s );
			Q_strlwr( lower );

			p = strstr( lower, "[%s]" );
			if ( p ) {
				n = p - lower;
				strncpy( lower, s, n + 1 );
				lower[n + 1] = '\0';
				n += 3;
				Controls_GetConfig();
				if ( GetKeyBindingLocalizedString( "+activate", &keyString ) ) {
					text = keyString;
				} else {
					text = CG_SafeTranslateString_Internal( "cgame", "KEY_USE" );
				}
				strcat( lower, text );
				strcat( lower, s + n );
				text = lower;
			} else {
				text = s;
			}

			width = trap_R_Text_Width( text, font, scale, 0 );
			height = trap_R_Text_Height( font, scale );
			trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y - height,
							   font, scale, fade, text, 0, 0, textStyle );
		}

	} else if ( icon == HINT_HEALTH ) {
		Controls_GetConfig();
		GetKeyBindingLocalizedString( "+activate", &keyString );
		text = va( CG_SafeTranslateString_Internal( "cgame", "CGAME_PICKUPHEALTH" ), keyString );

		width = trap_R_Text_Width( text, font, scale, 0 );
		height = trap_R_Text_Height( font, scale );
		trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y - height,
						   font, scale, fade, text, 0, 0, textStyle );
	}

	trap_R_SetColor( fade );
	CG_DrawPic( rect->x - halfGrow + xOfs, rect->y - halfGrow,
				widthScale * rect->w + grow, rect->h + grow, shader );
	trap_R_SetColor( NULL );

	if ( cg.cursorHintValue ) {
		fade[0] = 0;
		fade[1] = 0;
		fade[2] = 1.0f;
		fade[3] = 0.5f;
		CG_FilledBar( rect->x, rect->y + rect->h + 4.0f, rect->w, 8.0f,
					  fade, NULL, NULL, cg.cursorHintValue * ( 1.0f / 255 ), 0 );
	}
}

/*
================
CG_DrawPlayerAmmoValue        0x30025AB0

RETAIL: __usercall( rect@<esi>, font@<edi>, scale, color, shader, textStyle,
type ).  `shader` and `type` are both dead here -- CoD prints the clip on the
left, the reserve on the right and a '|' between them, and neither ownerdraw
5 nor 70 changes that.
================
*/
static void CG_DrawPlayerAmmoValue( rectDef_t *rect, int font, float scale, vec4_t color,
									qhandle_t shader, int textStyle, int type ) {
	centity_t   *cent;
	weaponInfo_t *weapDef;
	const char  *clipText;
	const char  *ammoText;
	int         weapon, clip, reserve;
	qboolean    showClip, showAmmo;
	int         width;

	clipText = NULL;
	ammoText = NULL;

	if ( !cg.predictedPlayerState.weapon ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];

	if ( ( cg.snap->ps.pm_flags & PMF_INWORLD )
		 && cg_weaponSelect.integer >= 0 && cg_weaponSelect.integer < bg_numWeapons ) {
		weapon = cg_weaponSelect.integer;
	} else {
		weapon = cent->currentState.weapon;
	}
	if ( !weapon ) {
		return;
	}

	showAmmo = qtrue;
	showClip = qtrue;

	reserve = BG_GetTotalAmmoReserve( &cg.predictedPlayerState, weapon );
	weapDef = bg_weaponInfo[weapon];

	if ( weapDef->clipOnly ) {
		clip = -1;
		showClip = qfalse;
	} else {
		clip = cg.predictedPlayerState.ammoclip[weapDef->clipIndex];
		if ( clip < 0 ) {
			showClip = qfalse;
		}
	}
	if ( clip > 999 ) {
		clip = 999;
	}
	if ( reserve < 0 ) {
		showAmmo = qfalse;
	}
	if ( reserve > 999 ) {
		reserve = 999;
	}

	if ( showClip ) {
		clipText = va( "%2i", clip );
	}
	if ( showAmmo ) {
		ammoText = va( "%i", reserve );
	}

	if ( showClip ) {
		if ( showAmmo ) {
			trap_R_Text_Paint( rect->x, rect->y, font, scale,
							   color, clipText, 0, 0, textStyle );

			width = trap_R_Text_Width( ammoText, font, scale, 0 );
			trap_R_Text_Paint( rect->x + rect->w - width, rect->y, font, scale,
							   color, ammoText, 0, 0, textStyle );

			width = trap_R_Text_Width( "|", font, scale, 0 );
			trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y, font, scale,
							   color, "|", 0, 0, textStyle );
		} else {
			width = trap_R_Text_Width( clipText, font, scale, 0 );
			trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y, font, scale,
							   color, clipText, 0, 0, textStyle );
		}
	} else if ( showAmmo ) {
		width = trap_R_Text_Width( ammoText, font, scale, 0 );
		trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y, font, scale,
						   color, ammoText, 0, 0, textStyle );
	}
}

/*
================
CG_DrawSelectedPlayerHealth   0x30025D40

RETAIL: __usercall( rect@<ebx>, shader@<ecx>, font, scale, color, textStyle ).
CG_GetSelectedPlayer is inlined at 0x30025D55.
================
*/
static void CG_DrawSelectedPlayerHealth( rectDef_t *rect, int font, float scale, vec4_t color,
										 qhandle_t shader, int textStyle ) {
	int  client;
	char num[16];
	int  width;

	client = sortedTeamPlayers[CG_GetSelectedPlayer()];
	if ( !bg_clientinfo[client].infoValid ) {
		return;
	}

	if ( shader ) {
		trap_R_SetColor( color );
		CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
		trap_R_SetColor( NULL );
	} else {
		Com_sprintf( num, sizeof( num ), "%i", bg_clientinfo[client].health );
		width = trap_R_Text_Width( num, font, scale, 0 );
		trap_R_Text_Paint( rect->x + ( rect->w - width ) * 0.5f, rect->y + rect->h,
						   font, scale, color, num, 0, 0, textStyle );
	}
}

/*
================
CG_DrawSelectedPlayerName     0x30025E60

RETAIL: __usercall( rect@<ecx>, font, scale, color, voice, textStyle ).
`voice` is what tells ownerdraw 63 from ownerdraw 8, and CoD never reads it.
================
*/
static void CG_DrawSelectedPlayerName( rectDef_t *rect, int font, float scale, vec4_t color,
									   qboolean voice, int textStyle ) {
	int client;

	client = sortedTeamPlayers[CG_GetSelectedPlayer()];
	if ( !bg_clientinfo[client].infoValid ) {
		return;
	}

	trap_R_Text_Paint( rect->x, rect->y + rect->h, font, scale,
					   color, bg_clientinfo[client].name, 0, 0, textStyle );
}

/*
================
CG_GetTranslatedLocationString    0x30025EF0

RETAIL: __usercall( location@<eax> ).  CG_ConfigString and the two
trap_SE_TranslateReference failure paths are inlined.
================
*/
/* NOT static: cg_servercmds_mp.c's CG_VoiceChatLocal calls it. */
const char *CG_GetTranslatedLocationString( int location ) {
	const char *s;
	const char *t;

	s = CG_ConfigString( CS_LOCATIONS + location );
	if ( !s || !s[0] ) {
		s = "CGAME_UNKNOWN";
	}

	t = trap_SE_TranslateReference( s );
	if ( t ) {
		return t;
	}

	if ( cl_languagewarnings.integer ) {
		if ( cl_languagewarningsaserrors.integer ) {
			Com_Error( 7, "Could not translate map location string \"%s\"", s );
		} else {
			Com_Printf( "^3WARNING: Could not translate map location string \"%s\"\n", s );
		}
		strcpy( locationString, "^1UNLOCALIZED(^7" );
		strcat( locationString, s );
		strcat( locationString, "^1)^7" );
		return locationString;
	}

	strcpy( locationString, s );
	return locationString;
}

/*
================
CG_DrawSelectedPlayerLocation 0x30026030

RETAIL: __usercall( rect@<esi>, font, scale, color, textStyle ).
================
*/
static void CG_DrawSelectedPlayerLocation( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	int         client;
	const char  *s;

	client = sortedTeamPlayers[CG_GetSelectedPlayer()];
	if ( !bg_clientinfo[client].infoValid ) {
		return;
	}

	s = CG_GetTranslatedLocationString( bg_clientinfo[client].location );
	trap_R_Text_Paint( rect->x, rect->y + rect->h, font, scale,
					   color, s, 0, 0, textStyle );
}

/*
================
CG_DrawPlayerLocation         0x300260C0

RETAIL: __usercall( rect@<esi>, font, scale, color, textStyle ).
================
*/
static void CG_DrawPlayerLocation( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	int         client;
	const char  *s;

	client = cg.snap->ps.clientNum;
	if ( !bg_clientinfo[client].infoValid ) {
		return;
	}

	s = CG_GetTranslatedLocationString( bg_clientinfo[client].location );
	trap_R_Text_Paint( rect->x, rect->y + rect->h, font, scale,
					   color, s, 0, 0, textStyle );
}

/*
================
CG_DrawPlayerHealth           0x30026140

RETAIL: __usercall( rect, color@<edx>, shader@<ecx>, scale, textStyle ).
RTCW's function with the follow test replaced (CoD has no PMF_FOLLOW arm here,
it compares the snapshot's client against the local one) and everything after
the CG_DrawField call dropped -- no lives counter, no invulnerability icon.
CG_DrawField (cg_draw.c 0x30014530) is LTCG-inlined at 0x30026229, specialised
for width 3 and leftAlign qtrue.
================
*/
static void CG_DrawPlayerHealth( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	playerState_t *ps;
	int value;

	ps = &cg.snap->ps;

	if ( ps->clientNum != cg.clientNum ) {
		if ( !bg_clientinfo[ps->clientNum].infoValid ) {
			return;
		}
		value = bg_clientinfo[ps->clientNum].health;
	} else {
		value = ps->stats[STAT_HEALTH];
	}

	// Don't show negative health
	if ( value < 0 ) {
		value = 0;
	}

	if ( shader ) {
		trap_R_SetColor( color );
		CG_DrawPic( rect->x, rect->y, rect->w, rect->h, shader );
		trap_R_SetColor( NULL );
	} else {
		trap_R_SetColor( color );
		CG_DrawField( rect->x, rect->y, 3, value, 20 * scale, 32 * scale, qtrue, qtrue );
	}
}

/*
================
CG_DrawRedScore               0x30026360

RETAIL: __usercall( rect@<ebx>, font, scale, color, shader, textStyle ).
================
*/
static void CG_DrawRedScore( rectDef_t *rect, int font, float scale, vec4_t color,
							 qhandle_t shader, int textStyle ) {
	char num[16];
	int  width;

	if ( cgs.scores1 == SCORE_NOT_PRESENT ) {
		Com_sprintf( num, sizeof( num ), "-" );
	} else {
		Com_sprintf( num, sizeof( num ), "%i", cgs.scores1 );
	}
	width = trap_R_Text_Width( num, font, scale, 0 );
	trap_R_Text_Paint( rect->x + rect->w - width, rect->y + rect->h, font, scale,
					   color, num, 0, 0, textStyle );
}

/*
================
CG_DrawBlueScore              0x30026440
================
*/
static void CG_DrawBlueScore( rectDef_t *rect, int font, float scale, vec4_t color,
							  qhandle_t shader, int textStyle ) {
	char num[16];
	int  width;

	if ( cgs.scores2 == SCORE_NOT_PRESENT ) {
		Com_sprintf( num, sizeof( num ), "-" );
	} else {
		Com_sprintf( num, sizeof( num ), "%i", cgs.scores2 );
	}
	width = trap_R_Text_Width( num, font, scale, 0 );
	trap_R_Text_Paint( rect->x + rect->w - width, rect->y + rect->h, font, scale,
					   color, num, 0, 0, textStyle );
}

/*
================
CG_DrawTeamColor              0x30026520

RETAIL: __usercall( rect@<ecx>, color ).
================
*/
static void CG_DrawTeamColor( rectDef_t *rect, vec4_t color ) {
	int client;

	client = cg.snap->ps.clientNum;
	if ( !bg_clientinfo[client].infoValid ) {
		return;
	}

	CG_DrawTeamBackground( rect->x, rect->y, rect->w, rect->h,
						   color[3], bg_clientinfo[client].team );
}

/*
================
CG_DrawAreaHoldable           0x30026570
CG_DrawAreaWeapons            0x30026580

Both survive as empty bodies -- CoD kept the symbols and dropped RTCW's
inventory strips.  CG_OwnerDraw still dispatches CG_AREA_WEAPON (78) and
CG_AREA_HOLDABLE (79) to them; the calls fold into the epilogue (slot 21 of
the 33-entry switch table at 0x300270F0).
================
*/
static void CG_DrawAreaHoldable( rectDef_t *rect, int align, float spacing, float scale, vec4_t color ) {
}

static void CG_DrawAreaWeapons( rectDef_t *rect, int align, float spacing, float scale, vec4_t color ) {
}

/*
================
CG_GetValue                   0x30026590

cgDC.getValue.  BG_GetAmmoTypeMax / BG_GetAmmoClipSize are inlined at
0x30026627 / 0x30026668.
================
*/
float CG_GetValue( int ownerDraw, int type ) {
	centity_t     *cent;
	playerState_t *ps;
	int           client;
	int           weapon;
	weaponInfo_t  *weapDef;

	ps = &cg.snap->ps;
	cent = &cg_entities[ps->clientNum];

	switch ( ownerDraw ) {
	case CG_SELECTEDPLAYER_HEALTH:
		client = sortedTeamPlayers[CG_GetSelectedPlayer()];
		/* retail quirk (0x300265ef): health only when info is INVALID, -1 when
		   valid -- inverted vs CG_PLAYER_HEALTH below. Reproduced faithfully. */
		if ( !bg_clientinfo[client].infoValid ) {
			return (float)bg_clientinfo[client].health;
		}
		break;

	case CG_PLAYER_AMMO_VALUE:
		weapon = cent->currentState.weapon;
		if ( weapon ) {
			weapDef = bg_weaponInfo[weapon];
			if ( type == 1 ) {
				return (float)ps->ammo[weapDef->ammoIndex];
			}
			return (float)ps->ammo[weapDef->ammoIndex] / BG_GetAmmoTypeMax( weapDef->ammoIndex );
		}
		break;

	case CG_PLAYER_AMMOCLIP_VALUE:
		weapon = cent->currentState.weapon;
		if ( weapon ) {
			weapDef = bg_weaponInfo[weapon];
			if ( type == 1 ) {
				return (float)ps->ammoclip[weapDef->clipIndex];
			}
			return (float)ps->ammoclip[weapDef->clipIndex] / BG_GetAmmoClipSize( weapDef->clipIndex );
		}
		break;

	case CG_PLAYER_SCORE:
		client = ps->clientNum;
		if ( !bg_clientinfo[client].infoValid ) {
			return 0;
		}
		return (float)bg_clientinfo[client].score;

	case CG_PLAYER_HEALTH:
		client = ps->clientNum;
		if ( !bg_clientinfo[client].infoValid ) {
			return 0;
		}
		return (float)bg_clientinfo[client].health;

	case CG_RED_SCORE:
		return (float)cgs.scores1;

	case CG_BLUE_SCORE:
		return (float)cgs.scores2;
	}

	return -1;
}

/*
================
CG_OwnerDrawVisible           0x30026740

cgDC.ownerDrawVisible.  RTCW's HEALTHOK arm is `>= 25`; retail compares `> 25`
(0x3002679F is a jg).
================
*/
qboolean CG_OwnerDrawVisible( int flags ) {
	if ( flags & CG_SHOW_TEAMINFO ) {
		return ( cg_currentSelectedPlayer.integer == numSortedTeamPlayers );
	}

	if ( flags & CG_SHOW_NOTEAMINFO ) {
		return !( cg_currentSelectedPlayer.integer == numSortedTeamPlayers );
	}

	if ( flags & CG_SHOW_HEALTHCRITICAL ) {
		if ( cg.snap->ps.stats[STAT_HEALTH] < 25 ) {
			return qtrue;
		}
	}

	if ( flags & CG_SHOW_HEALTHOK ) {
		if ( cg.snap->ps.stats[STAT_HEALTH] > 25 ) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
================
CG_DrawAreaSystemChat         0x300267B0

RETAIL: __usercall( rect@<eax>, font, scale, color, shader ).  All three chat
ownerdraws paint with style 0, not the item's textStyle.
================
*/
static void CG_DrawAreaSystemChat( rectDef_t *rect, int font, float scale, vec4_t color, qhandle_t shader ) {
	trap_R_Text_Paint( rect->x, rect->y + rect->h, font, scale,
					   color, systemChat, 0, 0, 0 );
}

/*
================
CG_DrawAreaTeamChat           0x30026810
================
*/
static void CG_DrawAreaTeamChat( rectDef_t *rect, int font, float scale, vec4_t color, qhandle_t shader ) {
	trap_R_Text_Paint( rect->x, rect->y + rect->h, font, scale,
					   color, teamChat1, 0, 0, 0 );
}

/*
================
CG_DrawAreaChat               0x30026870
================
*/
static void CG_DrawAreaChat( rectDef_t *rect, int font, float scale, vec4_t color, qhandle_t shader ) {
	trap_R_Text_Paint( rect->x, rect->y + rect->h, font, scale,
					   color, teamChat2, 0, 0, 0 );
}

/*
================
CG_GetKillerText              0x300268D0
================
*/
const char *CG_GetKillerText( void ) {
	const char *s = "";

	if ( cg.killerName[0] ) {
		s = va( "Fragged by %s", cg.killerName );
	}
	return s;
}

/*
================
CG_DrawKiller                 0x30026900

RETAIL: __usercall( rect@<esi>, font@<ebx>, scale, color, shader, textStyle ).
CG_GetKillerText is inlined twice, exactly as RTCW calls it twice.
================
*/
static void CG_DrawKiller( rectDef_t *rect, int font, float scale, vec4_t color,
						   qhandle_t shader, int textStyle ) {
	float x;

	// fragged by ... line
	if ( cg.killerName[0] ) {
		x = rect->x + rect->w * 0.5f;
		trap_R_Text_Paint( x - trap_R_Text_Width( CG_GetKillerText(), font, scale, 0 ) / 2,
						   rect->y + rect->h, font, scale,
						   color, CG_GetKillerText(), 0, 0, textStyle );
	}
}

/*
================
CG_Draw1stPlace               0x300269D0

RETAIL: __usercall( rect@<esi>, font, scale, color, shader, textStyle ).
================
*/
static void CG_Draw1stPlace( rectDef_t *rect, int font, float scale, vec4_t color,
							 qhandle_t shader, int textStyle ) {
	if ( cgs.scores1 != SCORE_NOT_PRESENT ) {
		trap_R_Text_Paint( rect->x, rect->y, font, scale,
						   color, va( "%2i", cgs.scores1 ), 0, 0, textStyle );
	}
}

/*
================
CG_Draw2ndPlace               0x30026A40
================
*/
static void CG_Draw2ndPlace( rectDef_t *rect, int font, float scale, vec4_t color,
							 qhandle_t shader, int textStyle ) {
	if ( cgs.scores2 != SCORE_NOT_PRESENT ) {
		trap_R_Text_Paint( rect->x, rect->y, font, scale,
						   color, va( "%2i", cgs.scores2 ), 0, 0, textStyle );
	}
}

/*
================
CG_GameTypeString             0x30026AB0

RTCW maps the gametype enum onto a display string; CoD's g_gametype is already
a string, so this is one instruction.
================
*/
const char *CG_GameTypeString( void ) {
	return cgs.gametype;
}

/*
================
CG_DrawGameType               0x30026AC0

RETAIL: __usercall( rect@<eax>, font, scale, color, shader, textStyle ).
CG_GameTypeString is inlined.
================
*/
static void CG_DrawGameType( rectDef_t *rect, int font, float scale, vec4_t color,
							 qhandle_t shader, int textStyle ) {
	trap_R_Text_Paint( rect->x, rect->y + rect->h, font, scale,
					   color, CG_GameTypeString(), 0, 0, textStyle );
}

/*
================
CG_DrawMenuShader             0x30026B20

RETAIL: __usercall( rect@<eax>, shader, color ).  CoD addition: a plain
screen-space blit of the item's shader.
================
*/
static void CG_DrawMenuShader( rectDef_t *rect, qhandle_t shader, vec4_t color ) {
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( cgs.screenXScale * rect->x, rect->y * cgs.screenYScale,
						   rect->w * cgs.screenXScale, rect->h * cgs.screenYScale,
						   0, 0, 1.0f, 1.0f, shader );
	trap_R_SetColor( NULL );
}

/*
================
CG_OwnerDraw                  0x30026BB0

cgDC.ownerDrawItem.  `font` is CoD's extra argument, between `special` and
`scale`; every handler below takes it.  The whole function is gated on
cg_drawStatus.
================
*/
void CG_OwnerDraw( float x, float y, float w, float h, float text_x, float text_y,
				   int ownerDraw, int ownerDrawFlags, int align, float special,
				   int font, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	rectDef_t rect;

	if ( !cg_drawStatus.integer ) {
		return;
	}

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	switch ( ownerDraw ) {
	case CG_PLAYER_HEALTH:
		CG_DrawPlayerHealth( &rect, scale, color, shader, textStyle );
		break;
	case CG_PLAYER_AMMO_VALUE:
		CG_DrawPlayerAmmoValue( &rect, font, scale, color, shader, textStyle, 0 );
		break;
	case CG_PLAYER_AMMO_BACKDROP:
		CG_DrawPlayerAmmoBackdrop( &rect, color, shader );
		break;
	case CG_SELECTEDPLAYER_NAME:
		CG_DrawSelectedPlayerName( &rect, font, scale, color, qfalse, textStyle );
		break;
	case CG_SELECTEDPLAYER_LOCATION:
		CG_DrawSelectedPlayerLocation( &rect, font, scale, color, textStyle );
		break;
	case CG_PLAYER_STANCE:
		CG_DrawPlayerStance( &rect, color, font, scale, textStyle );
		break;
	case CG_BLUE_SCORE:
		CG_DrawBlueScore( &rect, font, scale, color, shader, textStyle );
		break;
	case CG_RED_SCORE:
		CG_DrawRedScore( &rect, font, scale, color, shader, textStyle );
		break;
	case CG_PLAYER_LOCATION:
		CG_DrawPlayerLocation( &rect, font, scale, color, textStyle );
		break;
	case CG_TEAM_COLOR:
		CG_DrawTeamColor( &rect, color );
		break;
	case CG_GAME_TYPE:
		CG_DrawGameType( &rect, font, scale, color, shader, textStyle );
		break;
	case CG_SELECTEDPLAYER_HEALTH:
		CG_DrawSelectedPlayerHealth( &rect, font, scale, color, shader, textStyle );
		break;
	case CG_AREA_SYSTEMCHAT:
		CG_DrawAreaSystemChat( &rect, font, scale, color, shader );
		break;
	case CG_AREA_TEAMCHAT:
		CG_DrawAreaTeamChat( &rect, font, scale, color, shader );
		break;
	case CG_AREA_CHAT:
		CG_DrawAreaChat( &rect, font, scale, color, shader );
		break;
	case CG_KILLER:
		CG_DrawKiller( &rect, font, scale, color, shader, textStyle );
		break;
	case CG_VOICE_NAME:
		CG_DrawSelectedPlayerName( &rect, font, scale, color, qtrue, textStyle );
		break;
	case CG_1STPLACE:
		CG_Draw1stPlace( &rect, font, scale, color, shader, textStyle );
		break;
	case CG_2NDPLACE:
		CG_Draw2ndPlace( &rect, font, scale, color, shader, textStyle );
		break;
	case CG_PLAYER_AMMOCLIP_VALUE:
		CG_DrawPlayerAmmoValue( &rect, font, scale, color, shader, textStyle, 1 );
		break;
	case CG_CURSORHINT:
		CG_DrawCursorhint( &rect, font, scale, textStyle );
		break;
	case CG_AREA_WEAPON:
		CG_DrawAreaWeapons( &rect, align, special, scale, color );
		break;
	case CG_AREA_HOLDABLE:
		CG_DrawAreaHoldable( &rect, align, special, scale, color );
		break;
	case CG_PLAYER_WEAPON_NAME:
		CG_DrawPlayerWeaponName( &rect, font, scale, color, textStyle );
		break;
	case CG_PLAYER_WEAPON_NAME_BACK:
		CG_DrawPlayerWeaponNameBack( &rect, font, scale, color, shader );
		break;
	case CG_PLAYER_WEAPON_MODE_ICON:
		CG_DrawPlayerWeaponModeIcon( &rect, color );
		break;
	case CG_PLAYER_COMPASS:
		CG_DrawPlayerCompass( &rect, shader, color );
		break;
	case CG_PLAYER_COMPASS_BACK:
		CG_DrawPlayerCompassBack( &rect, shader, color );
		break;
	case CG_PLAYER_COMPASS_POINTERS:
		CG_DrawPlayerCompassPointers( &rect, shader, color );
		break;
	case CG_PLAYER_COMPASS_FRIENDLIES:
		CG_DrawCompassFriendlies( (float *)&rect, shader, color );
		break;
	case CG_PLAYER_BAR_HEALTH:
		CG_DrawPlayerBarHealth( &rect, color, shader );
		break;
	case CG_PLAYER_BAR_HEALTH_TITLE:
		CG_DrawPlayerBarHealthTitle( &rect, font, scale, color, textStyle );
		break;
	case CG_MENU_SHADER:
		CG_DrawMenuShader( &rect, shader, color );
		break;
	default:
		break;
	}
}

/*
================
CG_MouseEvent                 0x30027150
CG_EventHandling              0x30027160
CG_KeyEvent                   0x30027170

All three survive as empty bodies: CoD's in-game menus are driven by the engine
side, not by cgame.
================
*/
void CG_MouseEvent( int x, int y ) {
}

void CG_EventHandling( int type ) {
}

void CG_KeyEvent( int key, qboolean down ) {
}

/*
================
CG_KeyInterceptEvent          0x30027180

RETAIL: __usercall( key@<eax>, down@<ecx> ).  CoD addition: the scoreboard's
own scroll wheel, which has to run before the key reaches the binding system.
================
*/
qboolean CG_KeyInterceptEvent( int key, qboolean down ) {
	if ( !down ) {
		return qfalse;
	}
	if ( !cg.showScores ) {
		return qfalse;
	}

	if ( key == K_MWHEELUP || key == K_PGUP || key == K_KP_PGUP ) {
		if ( cg.scoreboardScrollPos > 0 ) {
			cg.scoreboardScrollPos -= cg_scoreboardScrollStep.integer;
			if ( cg.scoreboardScrollPos < 0 ) {
				cg.scoreboardScrollPos = 0;
			}
		}
		return qtrue;
	}

	if ( key == K_MWHEELDOWN || key == K_PGDN || key == K_KP_PGDN ) {
		CG_ScrollScoreboardDown();
		return qtrue;
	}

	return qfalse;
}

/*
================
CG_ClientNumFromName          0x300271F0  (unnamed in retail)

RETAIL: __usercall( p@<ebx> ).  RTCW's function with Q_stricmp replaced by
Q_stricmpn( .., .., 99999 ) and cgs.clientinfo replaced by bg_clientinfo.
================
*/
int CG_ClientNumFromName( const char *p ) {
	int i;

	for ( i = 0; i < cgs.maxclients; i++ ) {
		if ( bg_clientinfo[i].infoValid && bg_clientinfo[i].name && p
			 && Q_stricmpn( bg_clientinfo[i].name, p, 99999 ) == 0 ) {
			return i;
		}
	}
	return -1;
}

/*
================
CG_ShowResponseHead           0x30027240

Menus_OpenByName is inlined (0x30027246).
================
*/
void CG_ShowResponseHead( void ) {
	struct menuDef_s *menu;

	menu = Menus_FindByName( "voiceMenu" );
	if ( menu ) {
		Menus_Open( menu );
	}
	trap_Cvar_Set( "cl_conXOffset", "72" );
	cg.voicetime = cg.time;
}

/*
================
CG_RunMenuScript              0x30027280

cgDC.runScript.  Empty: cgame runs no menu scripts of its own.
================
*/
void CG_RunMenuScript( char **args ) {
}

/*
================
CG_GetTeamColor               0x30027290

cgDC.getTeamColor.  RTCW reads ps.persistant[PERS_TEAM]; CoD reads the shared
clientInfo_t.
================
*/
void CG_GetTeamColor( vec4_t *color ) {
	int client;

	client = cg.snap->ps.clientNum;

	if ( bg_clientinfo[client].infoValid && bg_clientinfo[client].team == TEAM_AXIS ) {
		( *color )[0] = 1.0f;
		( *color )[1] = 0;
		( *color )[2] = 0;
		( *color )[3] = .25f;
	} else if ( bg_clientinfo[client].infoValid && bg_clientinfo[client].team == TEAM_ALLIES ) {
		( *color )[0] = 0;
		( *color )[1] = 0;
		( *color )[2] = 1.0f;
		( *color )[3] = .25f;
	} else {
		( *color )[0] = 0;
		( *color )[1] = .17f;
		( *color )[2] = 0;
		( *color )[3] = .25f;
	}
}
