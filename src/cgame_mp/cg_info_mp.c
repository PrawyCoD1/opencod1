/*
 * cg_info_mp.c -- display information while data is being loaded.
 * (original source: cgame/cg_info.c)
 *
 * cgame_mp_x86.dll 0x3001F9C0 .. 0x3001FEB4, two functions.
 *
 * RTCW's cgame/cg_info.c is the ancestor of both, but CG_DrawInformation was
 * rewritten: the sound/model/graphic tallies and the "press ESC" line are all
 * gone, and what is left is the levelshot, a percent bar driven by
 * com_expectedhunkusage, and -- new in CoD -- a second `localized` mode that
 * draws the map and gametype names while the client waits for the server to
 * finish loading.
 *
 * REENTRANCY.  trap_R_RegisterModel, trap_R_RegisterShader and
 * trap_R_RegisterShaderNoMip each call CG_DrawInformation( 0 ) before issuing
 * their syscall, so the function can call itself: the two
 * trap_R_RegisterShaderNoMip calls below appear in the binary as a recursive
 * `call CG_DrawInformation` followed by `syscall( 88, ... )` (0x3001FB62 and
 * 0x3001FB7D).  cg_drawingInformation is what stops the recursion.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdlib.h>

#include "cg_local.h"

/*
======================
CG_LoadingString       0x3001F9C0
======================
*/
void CG_LoadingString( const char *s ) {
	strncpy( cg.infoScreenText, s, sizeof( cg.infoScreenText ) - 1 );
	cg.infoScreenText[sizeof( cg.infoScreenText ) - 1] = 0;

	if ( s && s[0] ) {
		CG_Printf( va( "LOADING... %s\n", s ) );
	}

	trap_UpdateScreen();
}

/* 0x300EEF28 -- the guard the register traps recurse through. */
/* Not static: CG_DrawActive (0x3001895F) and CG_DrawActiveFrame (0x30033A5E)
   read the same dword at 0x300EEF28. */
int cg_drawingInformation;

/*
====================
CG_DrawInformation   0x3001FA10

Draw all the status / pacifier stuff during level loading.

`localized` selects the waiting-for-the-server screen: it reads the map and
gametype out of cl_serverloadmap / cl_serverloadgametype rather than out of
the serverinfo, dims the levelshot and prints three lines of localized text.
====================
*/
void CG_DrawInformation( int localized ) {
	const char *s;
	char buf[MAX_STRING_CHARS];
	char expected[64];
	vec4_t color;
	qhandle_t levelshot;
	const char *dots;
	int hunkUsed, hunkExpected;
	float frac;

	if ( ( cg.snap && !localized ) || cg_drawingInformation ) {
		return;
	}
	cg_drawingInformation = 1;

	if ( !localized ) {
		if ( cl_serverloadmap.string[0] ) {
			trap_Cvar_Set( "cl_serverloadmap", "" );
		}
		if ( cl_serverloadgametype.string[0] ) {
			trap_Cvar_Set( "cl_serverloadgametype", "" );
		}
		if ( cl_serverloadwaiting.integer ) {
			trap_Cvar_Set( "cl_serverloadwaiting", "0" );
		}
	} else {
		if ( !cl_serverloadwaiting.integer ) {
			trap_Cvar_Set( "ui_scriptMenuAllowResponse", "0" );
			CG_CloseScriptMenu();
			CG_CloseScriptMenu();
			trap_Cvar_Set( "ui_scriptMenuAllowResponse", "1" );
			trap_UI_CloseAllMenus();
			trap_Cvar_Set( "cl_serverloadwaiting", "1" );
		}
	}

	//
	// draw the dialog background
	//
	if ( localized ) {
		trap_Cvar_VariableStringBuffer( "cl_serverloadmap", buf, sizeof( buf ) );
		s = buf;
	} else {
		/* CG_ConfigString( CS_SERVERINFO ) inlined (0x3001FB0F). */
		s = Info_ValueForKey( cg_gameState.stringData + cg_gameState.stringOffsets[0],
							  "mapname" );
	}

	levelshot = 0;
	if ( s && s[0] ) {
		levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", s ), 2 );
	}
	if ( !levelshot ) {
		levelshot = trap_R_RegisterShaderNoMip( "menu/art/unknownmap", 2 );
	}

	if ( localized ) {
		color[0] = color[1] = color[2] = 0.75f;
		color[3] = 1.0f;
		trap_R_SetColor( color );
	} else {
		trap_R_SetColor( NULL );
	}

	CG_DrawPic( 0, 0, 640, 480, levelshot );

	if ( localized ) {
		color[0] = color[1] = color[2] = 0.75f;
		color[3] = 1.0f;

		s = CG_SafeTranslateString_Internal( "cgame",
				trap_UI_GetGameTypeDisplayName( cl_serverloadgametype.string ) );
		trap_R_Text_Paint( ( 640 - trap_R_Text_Width( s, 0, 0.5f, 0 ) ) * 0.5f, 55.0f,
						   0, 0.5f, color, s, 0, 0, 3 );

		s = CG_SafeTranslateString_Internal( "cgame",
				trap_UI_GetMapDisplayName( cl_serverloadmap.string ) );
		trap_R_Text_Paint( ( 640 - trap_R_Text_Width( s, 0, 0.5f, 0 ) ) * 0.5f, 85.0f,
						   0, 0.5f, color, s, 0, 0, 3 );

		switch ( ( trap_Milliseconds() / 750 ) & 3 ) {
		case 1:
			dots = ".";
			break;
		case 2:
			dots = "..";
			break;
		case 3:
			dots = "...";
			break;
		default:
			dots = "";
			break;
		}

		s = CG_SafeTranslateString_Internal( "cgame", "CGAME_WAITINGFORSERVERLOAD" );
		trap_R_Text_Paint( ( 640 - trap_R_Text_Width( s, 0, 0.5f, 0 ) ) * 0.5f, 435.0f,
						   0, 0.5f, color, va( "%s%s", s, dots ), 0, 0, 3 );
	} else {
		trap_Cvar_VariableStringBuffer( "com_expectedhunkusage", expected, sizeof( expected ) );
		hunkExpected = atoi( expected );

		/* set and never read in 1.1 (0x3001FE11) */
		color[0] = color[1] = color[2] = color[3] = 0.8f;

		if ( hunkExpected > 0 ) {
			hunkUsed = trap_hunkUsed();
			frac = (float)hunkUsed / (float)hunkExpected;
			if ( frac > 1.0f ) {
				frac = 1.0f;
			}
			CG_HorizontalPercentBar( 200, 468, 240, 10, frac );
		}
	}

	trap_UpdateScreen();

	cg_drawingInformation--;
}
