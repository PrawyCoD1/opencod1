/*
 * ui_atoms_mp.c -- user interface building blocks and support functions.
 *
 * ui_mp_x86.dll (CoD 1.1, imagebase 0x40000000), unit ui/ui_atoms.c:
 * 0x40006B30 .. 0x40007162, in address order.  RTCW's ui/ui_atoms.c is the
 * ancestor text; every function below is that text, adjusted where the
 * binary says so:
 *
 *   - Com_DPrintf tests "com_developer", not "developer" (0x40006B46).
 *   - The three printf helpers format with vsprintf (0x40006B78 ..), Q3's
 *     original, not RTCW's Q_vsnprintf.
 *   - UI_Cvar_VariableString has ONE static buffer (0x40050858) and no
 *     flip-flop -- Q3's original body, which RTCW later doubled.
 *   - UI_DrawNamedPic carries an imageTrack for trap_R_RegisterShaderNoMip.
 *   - UI_ConsoleCommand keeps ui_test / ui_report / ui_load / ui_cache /
 *     ui_cdkey and drops RTCW's remapShader, postgame and ui_teamOrders.
 *     LTCG inlined it into vmMain (chunk 0x40006D00); it is written here, in
 *     its source home, and vmMain calls it.
 *   - RTCW's MISSIONPACK-only score helpers (UI_LoadBestScores,
 *     UI_ClearScores, UI_CalcPostGameStats) do not exist in the DLL.
 *
 * There is no `uis` here: nothing in the DLL reads a uiStatic_t (see
 * ui_local.h).  RTCW's `m_entersound` is likewise absent.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "ui_local.h"

/* ---- Com_DPrintf  0x40006B30 ---- */
#define MAXPRINTMSG 4096
void QDECL Com_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG];
	int developer;

	developer = trap_Cvar_VariableValue( "com_developer" );
	if ( !developer ) {
		return;
	}

	va_start( argptr,fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );

	Com_Printf( "%s", msg );
}

/* ---- Com_Error  0x40006BB0 ---- */
void QDECL Com_Error( int level, const char *error, ... ) {
	va_list argptr;
	char text[1024];

	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr );

	trap_Error( va( "%s", text ) );
}

/* ---- Com_Printf  0x40006C10 ---- */
void QDECL Com_Printf( const char *msg, ... ) {
	va_list argptr;
	char text[1024];

	va_start( argptr, msg );
	vsprintf( text, msg, argptr );
	va_end( argptr );

	trap_Print( va( "%s", text ) );
}

/*
=================
UI_ClampCvar  0x40006C70
=================
*/
float UI_ClampCvar( float min, float max, float value ) {
	if ( value < min ) {
		return min;
	}
	if ( value > max ) {
		return max;
	}
	return value;
}

/*
=================
UI_StartDemoLoop  0x40006CA0
=================
*/
void UI_StartDemoLoop( void ) {
	trap_Cmd_ExecuteText( EXEC_APPEND, "d1\n" );
}

/* ---- UI_Argv  0x40006CB0 ---- */
char *UI_Argv( int arg ) {
	static char buffer[MAX_STRING_CHARS];      /* 0x40050458 */

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}

/* ---- UI_Cvar_VariableString  0x40006CD0 ---- */
char *UI_Cvar_VariableString( const char *var_name ) {
	static char buffer[MAX_STRING_CHARS];      /* 0x40050858 */

	trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}

/* ---- UI_Cache_f  0x40006CF0 ---- */
static void UI_Cache_f( void ) {
	Display_CacheAll();
}

/*
=================
UI_ConsoleCommand  0x40006D00 (vmMain chunk)
=================
*/
/* Read simple menu bindings without executing open/close or other UI effects.
 * Conditional/custom actions remain the responsibility of a vsay listener. */
static qboolean UI_VsayAction( const char *script, const char *action, char *value, int size ) {
	char *cursor = (char *)script;
	const char *token, *argument;
	qboolean found = qfalse, match;
	if ( !script ) return qfalse;
	while ( String_Parse( &cursor, &token ) ) {
		if ( !strcmp( token, ";" ) ) continue;
		match = !Q_stricmp( token, action );
		if ( Q_stricmp( token, "open" ) && Q_stricmp( token, "close" )
			 && Q_stricmp( token, "scriptMenuResponse" ) ) return qfalse;
		if ( !String_Parse( &cursor, &argument ) || !strcmp( argument, ";" ) ) return qfalse;
		if ( match ) {
			if ( found || strlen( argument ) >= (size_t)size ) return qfalse;
			Q_strncpyz( value, argument, size );
			found = qtrue;
		}
	}
	return found;
}

static void UI_VsayResponse_f( void ) {
	char pair[32], name[128], response[128], config[128], serverId[32];
	menuDef_t *root, *menu;
	uiClientState_t state;
	int i;
	if ( trap_syscall_0xE() != 2 ) return;
	trap_Argv( 1, pair, sizeof( pair ) );
	if ( strpbrk( pair, "\"\r\n" ) ) return;
	trap_GetClientState( &state );
	if ( state.connState != CA_ACTIVE || !trap_Cvar_VariableValue( "ui_scriptMenuAllowResponse" ) ) return;
	root = Menus_FindByName( "quickmessage" );
	if ( strlen( pair ) == 3 && pair[0] >= '0' && pair[0] <= '9'
		 && pair[1] == ' ' && pair[2] >= '0' && pair[2] <= '9' && root
		 && UI_VsayAction( root->onKey[(unsigned char)pair[0]], "open", name, sizeof( name ) ) ) {
		menu = Menus_FindByName( name );
		if ( menu && UI_VsayAction( menu->onKey[(unsigned char)pair[2]], "scriptMenuResponse", response, sizeof( response ) ) ) {
			/* Match the same registered menu index used by the normal UI. */
			for ( i = 0; i < 32; ++i ) {
				trap_GetConfigString( 1180 + i, config, sizeof( config ) );
				if ( !Q_stricmp( config, name ) ) {
					trap_Cvar_VariableStringBuffer( "sv_serverId", serverId, sizeof( serverId ) );
					if ( !strpbrk( response, "\"\r\n" ) )
						trap_Cmd_ExecuteText( EXEC_APPEND, va( "cmd mr %i %i \"%s\"\n",
							atoi( serverId ), i, response ) );
					return;
				}
			}
		}
	}
	trap_Cmd_ExecuteText( EXEC_APPEND, va( "cmd voice \"%s\"\n", pair ) );
}

qboolean UI_ConsoleCommand( int realTime ) {
	char    *cmd;

	uiInfo.uiDC.frameTime = realTime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realTime;

	cmd = UI_Argv( 0 );
	if ( !Q_stricmp( cmd, "ui_vsay" ) ) {
		UI_VsayResponse_f();
		return qtrue;
	}

	// ensure minimum menu data is available
	//Menu_Cache();

	if ( Q_stricmp( cmd, "ui_test" ) == 0 ) {
		UI_ShowPostGame( qtrue );
	}

	if ( Q_stricmp( cmd, "ui_report" ) == 0 ) {
		UI_Report();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "ui_load" ) == 0 ) {
		UI_Load();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "ui_cache" ) == 0 ) {
		UI_Cache_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "ui_cdkey" ) == 0 ) {
		//UI_CDKeyMenu_f();
		return qtrue;
	}

	return qfalse;
}

/*
=================
UI_Shutdown  0x40006DA0
=================
*/
void UI_Shutdown( void ) {
}

/*
================
UI_AdjustFrom640  0x40006DB0

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
	*x *= uiInfo.uiDC.xscale;
	*y *= uiInfo.uiDC.yscale;
	*w *= uiInfo.uiDC.xscale;
	*h *= uiInfo.uiDC.yscale;

}

/* ---- UI_DrawNamedPic  0x40006DE0 -- no caller survives in the DLL ---- */
void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname, int imageTrack ) {
	qhandle_t hShader;

	hShader = trap_R_RegisterShaderNoMip( picname, imageTrack );
	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

/* ---- UI_DrawHandlePic  0x40006E30 ---- */
void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float s0;
	float s1;
	float t0;
	float t1;

	if ( w < 0 ) {   // flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	} else {
		s0 = 0;
		s1 = 1;
	}

	if ( h < 0 ) {   // flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	} else {
		t0 = 0;
		t1 = 1;
	}

	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect  0x40006EF0

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );

	trap_R_SetColor( NULL );
}

/* ---- UI_DrawSides  0x40006F50 ---- */
void UI_DrawSides( float x, float y, float w, float h ) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x + w - 1, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

/* ---- UI_DrawTopBottom  0x40006FD0 ---- */
void UI_DrawTopBottom( float x, float y, float w, float h ) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x, y + h - 1, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

/*
================
UI_DrawRect  0x40007050

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_DrawTopBottom( x, y, width, height );
	UI_DrawSides( x, y, width, height );

	trap_R_SetColor( NULL );
}

/* ---- UI_SetColor  0x40007090 ---- */
void UI_SetColor( const float *rgba ) {
	trap_R_SetColor( rgba );
}

/* ---- UI_UpdateScreen  0x400070A0 ---- */
void UI_UpdateScreen( void ) {
	trap_UpdateScreen();
}

/* ---- UI_DrawTextBox  0x400070B0 -- no caller survives in the DLL ---- */
void UI_DrawTextBox( int x, int y, int width, int lines ) {
	UI_FillRect( x + BIGCHAR_WIDTH / 2, y + BIGCHAR_HEIGHT / 2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
	UI_DrawRect( x + BIGCHAR_WIDTH / 2, y + BIGCHAR_HEIGHT / 2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}

/* ---- UI_CursorInRect  0x40007130 -- no caller survives in the DLL ---- */
qboolean UI_CursorInRect( int x, int y, int width, int height ) {
	if ( uiInfo.uiDC.cursorx < x ||
		 uiInfo.uiDC.cursory < y ||
		 uiInfo.uiDC.cursorx > x + width ||
		 uiInfo.uiDC.cursory > y + height ) {
		return qfalse;
	}

	return qtrue;
}
