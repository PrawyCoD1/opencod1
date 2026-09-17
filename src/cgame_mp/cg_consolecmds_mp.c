/*
 * @fidelity: likely
 *
 * cg_consolecmds.c -- text commands typed in at the local console, or executed
 * by a key binding.
 *
 * cgame_mp_x86.dll, CoD 1.1, imagebase 0x30000000; functions in address
 * order, 0x30013BC0 .. 0x300142C0.  RTCW-MP's cg_consolecmds.c is the
 * ancestor; CoD dropped the test-model, inventory, zoom, weapon-bank, item and
 * limbo commands and added the fade, shellshock and fx ones.
 *
 * commands[] is NUL-terminated here rather than walked with sizeof, and its
 * last real entry ("mr") has no function: CG_ConsoleCommand still claims the
 * name, and the engine forwards it to the server.
 */

#include "cg_local.h"

#include <stdlib.h>
#include <string.h>

/* Declarations no header carries. */

/* ui/ui_shared.c. */
void        String_Init( void );
extern int  menuCount;                  /* 0x300EEEE8 */
/*
 * 0x300EEF34.  CG_LoadHud_f's `mov dword_300EEF34, 0` is the ONLY access in the
 * whole DLL -- nothing reads it -- so the name is unrecovered.  Not
 * ui_shared's: ui_shared.c's globals stop at 0x300EEF00, 0x300EEF2C/0x300EEF30
 * just below are cg_drawtools.c's CG_UpdateCompassOrientation statics and
 * 0x300EEF38 just above is bg_weapon.c's bg_numWeapons.  Defined here, in the
 * only unit that touches it.
 */
int         cg_unknown_300EEF34;

/* RTCW keeps PITCH/YAW/ROLL in q_shared.h; q_shared.h here is types-only, so
   the cgame units define them locally. */
#ifndef YAW
#define YAW     1
#endif


/* pm_flags bit gating a client that is actually playing.  bg_local.h stops at
   PMF_FOLLOW 0x10000 and this bit has no recovered name (0x300140E5). */
#define PMF_INGAME              0x00040000


/*
=================
CG_TargetCommand_f

CG_CrosshairPlayer (cg_main.c 0x30021940) is inlined; its -1 return makes the
`!targetNum` test dead on the stale-crosshair path.
=================
*/
static void CG_TargetCommand_f( void ) {
	int targetNum;
	char test[4];

	targetNum = CG_CrosshairPlayer();
	if ( !targetNum ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f( void ) {
	trap_Cvar_Set( "cg_viewsize", va( "%i", (int)( cg_viewsize.integer + 10 ) ) );
}

/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f( void ) {
	trap_Cvar_Set( "cg_viewsize", va( "%i", (int)( cg_viewsize.integer - 10 ) ) );
}

/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f( void ) {
	CG_Printf( "(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
			   (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
			   (int)cg.refdefViewAngles[YAW] );
}

/*
=================
CG_ScoresDown_f

Bound to "-scores" here, not "+scores": CoD swapped the two bodies relative to
RTCW and kept the names.
=================
*/
static void CG_ScoresDown_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

/*
=================
CG_ScoresUp_f

Bound to "+scores".
=================
*/
static void CG_ScoresUp_f( void ) {
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of date,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( cg.showScores ) {
			return;
		}
		cg.numScores = 0;
		cg.scoreboardScrollPos = 0;
	}
	cg.showScores = qtrue;
}

/*
=============
CG_LoadHud_f
=============
*/
static void CG_LoadHud_f( void ) {
	String_Init();
	menuCount = 0;
	CG_LoadMenus( "ui_mp/hud.txt", 5 );
	cg_unknown_300EEF34 = 0;
}

/*
=============
CG_Fade_f

"fade <r> <g> <b> <a> <seconds>".  CG_Fade (cg_draw.c 0x300174C0) is inlined
here; it ignores r, g and b, which is why they are parsed and dropped.
=============
*/
static void CG_Fade_f( void ) {
	int r, g, b, a, time;

	if ( trap_Argc() < 6 ) {
		return;
	}

	r = atoi( CG_Argv( 1 ) );
	g = atoi( CG_Argv( 2 ) );
	b = atoi( CG_Argv( 3 ) );
	a = atoi( CG_Argv( 4 ) );
	time = atoi( CG_Argv( 5 ) ) * 1000;

	/* the inlined body stores cg.time into cgs.scriptFadeStartTime (0x30013E5A)
	   and `time` into scriptFadeDuration -- CG_Fade takes both. */
	CG_Fade( r, g, b, a, cg.time, time );
}

/*
=============
CG_ShellShock_f

"cg_shellshock <duration> [filename]"
=============
*/
static void CG_ShellShock_f( void ) {
	char name[256];
	float duration;
	int msec;

	switch ( trap_Argc() ) {
	case 2:
		break;
	case 3:
		trap_Argv( 2, name, sizeof( name ) );
		if ( !CG_LoadShellShockCvars( name ) ) {
			return;
		}
		break;
	default:
		CG_Printf( "USAGE: cg_shellshock <duration> <filename?>\n" );
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );
	duration = atof( name );
	duration *= 1000;
	msec = Q_ftol( duration );

	CG_SetShellShockParmsFromCvars( &cgs.shellshockParms[0] );
	cg.shellshock.forcedStartTime = cg.time;
	cg.shellshock.forcedDuration = msec;
}

/*
=============
CG_ShellShock_Load_f
=============
*/
static void CG_ShellShock_Load_f( void ) {
	char name[MAX_QPATH];

	if ( trap_Argc() != 2 ) {
		CG_Printf( "USAGE: cg_shellshock_load <name>\n" );
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );
	CG_LoadShellShockCvars( name );
}

/*
=============
CG_ShellShock_Save_f
=============
*/
static void CG_ShellShock_Save_f( void ) {
	char name[MAX_QPATH];

	if ( trap_Argc() != 2 ) {
		CG_Printf( "USAGE: cg_shellshock_save <name>\n" );
		return;
	}

	trap_Argv( 1, name, sizeof( name ) );
	CG_SaveShellShockCvars( name );
}

/*
=================
CG_TellTarget_f
=================
*/
static void CG_TellTarget_f( void ) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i \"\x15%s\"", clientNum, message );
	trap_SendClientCommand( command );
}

/*
=================
CG_QuickMessage_f
=================
*/
static void CG_QuickMessage_f( void ) {
	if ( cg.snap->ps.pm_flags & PMF_INGAME ) {
		trap_UI_Popup( "UIMENU_WM_QUICKMESSAGE" );
	}
}

/*
=================
CG_VoiceChat_f
=================
*/
static void CG_VoiceChat_f( void ) {
	char cmd[MAX_QPATH];

	if ( trap_Argc() != 2 ) {
		return;
	}

	if ( cg.snap && cg.snap->ps.pm_type != PM_INTERMISSION
		 && !( cg.snap->ps.pm_flags & PMF_INGAME ) ) {
		CG_Printf( "%s\n", CG_SafeTranslateString_Internal( "cgame", "CGAME_NOSPECTATORVOICECHAT" ) );
		return;
	}

	trap_Argv( 1, cmd, sizeof( cmd ) );
	trap_SendConsoleCommand( va( "cmd vsay %s\n", cmd ) );
}

/*
=================
CG_TeamVoiceChat_f
=================
*/
static void CG_TeamVoiceChat_f( void ) {
	char cmd[MAX_QPATH];

	if ( trap_Argc() != 2 ) {
		return;
	}

	if ( cg.snap && cg.snap->ps.pm_type != PM_INTERMISSION
		 && !( cg.snap->ps.pm_flags & PMF_INGAME ) ) {
		CG_Printf( "%s\n", CG_SafeTranslateString_Internal( "cgame", "CGAME_NOSPECTATORVOICECHAT" ) );
		return;
	}

	trap_Argv( 1, cmd, sizeof( cmd ) );
	trap_SendConsoleCommand( va( "cmd vsay_team %s\n", cmd ) );
}

typedef struct {
	char    *cmd;
	void ( *function )( void );
} consoleCommand_t;

static consoleCommand_t commands[] = {
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresUp_f },
	{ "-scores", CG_ScoresDown_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapalt", CG_AltWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "weaponslot", CG_WeaponSlot_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "loadhud", CG_LoadHud_f },
	{ "fade", CG_Fade_f },
	{ "fxSetTestPosition", CG_FxSetTestPosition },
	{ "fxTest", CG_FxTest },
	{ "fxRestart", CG_FxRestart },
	{ "cg_shellshock", CG_ShellShock_f },
	{ "cg_shellshock_load", CG_ShellShock_Load_f },
	{ "cg_shellshock_save", CG_ShellShock_Save_f },
	{ "tell_target", CG_TellTarget_f },
	{ "mp_QuickMessage", CG_QuickMessage_f },
	{ "VoiceChat", CG_VoiceChat_f },
	{ "VoiceTeamChat", CG_TeamVoiceChat_f },
	// the menu-response command is claimed locally but handled by the server
	{ "mr", NULL },
	{ NULL, NULL }
};

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char  *cmd;
	int i;

	cmd = CG_Argv( 0 );

	for ( i = 0 ; commands[i].cmd ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			if ( commands[i].function ) {
				commands[i].function();
			}
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands so it can perform tab
completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int i;

	for ( i = 0 ; commands[i].cmd ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand( "kill" );
	trap_AddCommand( "give" );
	trap_AddCommand( "take" );
	trap_AddCommand( "god" );
	trap_AddCommand( "notarget" );
	trap_AddCommand( "noclip" );
	trap_AddCommand( "ufo" );
	trap_AddCommand( "levelshot" );
	trap_AddCommand( "setviewpos" );
	trap_AddCommand( "jumptonode" );
	trap_AddCommand( "stats" );
	trap_AddCommand( "say" );
	trap_AddCommand( "say_team" );
	trap_AddCommand( "tell" );
	trap_AddCommand( "team" );
	trap_AddCommand( "follow" );
	trap_AddCommand( "callvote" );
	trap_AddCommand( "vote" );
	trap_AddCommand( "follownext" );
	trap_AddCommand( "followprev" );
}
