/*
 * ui_main_mp.c -- user interface main.
 *
 * ui_mp_x86.dll (CoD 1.1, imagebase 0x40000000), unit ui/ui_main.c:
 * 0x400076A0 .. 0x4000ED76, 110 functions in address order.  RTCW's
 * ui/ui_main.c is the ancestor text; every function that descends from it is
 * that text, adjusted where the binary says so.  What CoD changed, in short:
 *
 *   - Text_Paint/Text_Width and the rest of RTCW's font code are gone from
 *     this unit; the renderer draws text (trap_R_Text_Paint) and every owner
 *     draw carries a `font` beside `scale`, exactly as ui_shared.h's
 *     ownerDrawItem slot does.  UI_GetFontInfo picks the fontInfo_t.
 *   - Every trap_R_RegisterShaderNoMip / trap_R_RegisterFont call carries an
 *     imageTrack; UI_ParseMenu, Asset_Parse, Load_Menu, Load_ScriptMenu and
 *     UI_LoadMenus pass it down, _UI_Init and UI_Load hand in the literal 2.
 *   - Menus load per language: Load_Menu/Load_ScriptMenu try
 *     <dir><language>/<file> first (trap_GetLanguagename), then the plain path.
 *   - Game types come from maps/mp/gametypes/*.gsc (UI_GetGameTypesList),
 *     not gameinfo.txt; gtEnum is the display name read from the matching
 *     .txt, a string.
 *   - Strings are localized through UI_SafeTranslateString / the SE_ traps;
 *     the EXE_* references are the retail ones.
 *   - The bot, skirmish, team-arena, savegame, limbo and PunkBuster code of
 *     RTCW is absent.
 *
 * Global names are RTCW's where the symbol survived; unnamed cells are
 * resolved to uiInfo members through the offsets in ui_local.h.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "ui_local.h"

/*
 * menudef.h in the original tree.  Only the ids this unit dispatches on;
 * the values are RTCW's (UI_OwnerDraw's switch 0x400094E7, UI_FeederCount's
 * compares 0x4000C5B0).
 */
#define UI_HANDICAP                 200
#define UI_GAMETYPE                 205
#define UI_MAPPREVIEW               206
#define UI_NETSOURCE                220
#define UI_NETMAPPREVIEW            221
#define UI_NETFILTER                222
#define UI_MAPCINEMATIC             244
#define UI_NETGAMETYPE              245
#define UI_NETMAPCINEMATIC          246
#define UI_SERVERREFRESHDATE        247
#define UI_GLINFO                   249
#define UI_KEYBINDSTATUS            250
#define UI_JOINGAMETYPE             253
#define UI_PREVIEWCINEMATIC         254
#define UI_STARTMAPCINEMATIC        255

#define UI_SHOW_LEADER              0x00000001
#define UI_SHOW_NOTLEADER           0x00000002
#define UI_SHOW_FAVORITESERVERS     0x00000004
#define UI_SHOW_NEWHIGHSCORE        0x00000020
#define UI_SHOW_DEMOAVAILABLE       0x00000040
#define UI_SHOW_NEWBESTTIME         0x00000080
#define UI_SHOW_FFA                 0x00000100
#define UI_SHOW_NOTFFA              0x00000200
#define UI_SHOW_NOTFAVORITESERVERS  0x00001000

#define FEEDER_HEADS                0x00
#define FEEDER_MAPS                 0x01
#define FEEDER_SERVERS              0x02
#define FEEDER_ALLMAPS              0x04
#define FEEDER_PLAYER_LIST          0x07
#define FEEDER_TEAM_LIST            0x08
#define FEEDER_MODS                 0x09
#define FEEDER_DEMOS                0x0a
#define FEEDER_SERVERSTATUS         0x0d
#define FEEDER_FINDPLAYER           0x0e
#define FEEDER_CINEMATICS           0x0f

#define ITEM_TEXTSTYLE_SHADOWED     3
#define ITEM_TEXTSTYLE_SHADOWEDMORE 6

/* ui_shared.h's asset paths, CoD's "ui/assets" (AssetCache 0x400077F0). */
#define ASSET_GRADIENTBAR           "ui/assets/gradientbar2.tga"
#define ASSET_SCROLLBAR             "ui/assets/scrollbar.tga"
#define ASSET_SCROLLBAR_ARROWDOWN   "ui/assets/scrollbar_arrow_dwn_a.tga"
#define ASSET_SCROLLBAR_ARROWUP     "ui/assets/scrollbar_arrow_up_a.tga"
#define ASSET_SCROLLBAR_ARROWLEFT   "ui/assets/scrollbar_arrow_left.tga"
#define ASSET_SCROLLBAR_ARROWRIGHT  "ui/assets/scrollbar_arrow_right.tga"
#define ASSET_SCROLL_THUMB          "ui/assets/scrollbar_thumb.tga"
#define ASSET_SLIDER_BAR            "ui/assets/slider2.tga"
#define ASSET_SLIDER_THUMB          "ui/assets/sliderbutt_1.tga"

/* q_shared.h's; universal/q_shared.h does not carry them. */
#define SCREEN_WIDTH                640
#define SCREEN_HEIGHT               480
#define MAX_INFO_VALUE              1024
#define KEYCATCH_UI                 0x0002
#define CS_SERVERINFO               0
#define AS_LOCAL                    0
#define AS_GLOBAL                   1
#define AS_FAVORITES                2
#define CDKEY_LEN                   16
#define CIN_loop                    2
#define CIN_silent                  8

/* CoD's Com_Error level for a missing localized string (0x4000EC54);
   above q_shared.h's errorParm_t. */
#define ERR_LOCALIZATION            7

/*
 * ui_shared.h keeps the DC's glconfig as an opaque 160-byte range, but this
 * unit fills it (_UI_Init 0x4000D331) and reads it (UI_DrawGLInfo
 * 0x400091EA); the layout is ui_public.h's glconfig_t.
 */
#define UI_GLCONFIG                 ( *(glconfig_t *)uiInfo.uiDC.glconfig )

typedef struct {
	const char *description;
	const char *basedir;
} serverFilter_t;

uiInfo_t uiInfo;                                    /* 0x401C3CE0 */

/* 0x40036AA8: the twelve EXE_MONTH_ABV_* references UI_StartServerRefresh translates */
static const char *MonthAbbrev[] = {
	"EXE_MONTH_ABV_JANUARY", "EXE_MONTH_ABV_FEBRUARY", "EXE_MONTH_ABV_MARCH",
	"EXE_MONTH_ABV_APRIL", "EXE_MONTH_ABV_MAY", "EXE_MONTH_ABV_JUN",
	"EXE_MONTH_ABV_JULY", "EXE_MONTH_ABV_AUGUST", "EXE_MONTH_ABV_SEPTEMBER",
	"EXE_MONTH_ABV_OCTOBER", "EXE_MONTH_ABV_NOVEMBER", "EXE_MONTH_ABV_DECEMBER"
};

/* 0x40036AD8 */
static const char *netSources[] = {
	"EXE_LOCAL",
	"EXE_INTERNET",
	"EXE_FAVORITES"
};
static const int numNetSources = sizeof( netSources ) / sizeof( const char* );

/* 0x4002D700 */
static const serverFilter_t serverFilters[] = {
	{"EXE_ALL", "" }
};

static const int numServerFilters = sizeof( serverFilters ) / sizeof( serverFilter_t );

static void UI_StartServerRefresh( qboolean full );
static void UI_StopServerRefresh( void );
static void UI_DoServerRefresh( void );
static void UI_FeederSelection( float feederID, int index );
static void UI_BuildServerDisplayList( qboolean force );
static void UI_UpdateDisplayServers( void );
static void UI_BuildServerStatus( qboolean force );
static void UI_BuildFindPlayerList( qboolean force );
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 );
static int UI_MapCountByGameType( void );
static const char *UI_SelectedMap( int index, int *actual );
static void UI_SelectCurrentGameType( void );
static void UI_SelectCurrentMap( void );
static void UI_BuildPlayerList( void );
static void UI_Update( const char *name );

qboolean    UI_CheckExecKey( int key );
void        UI_LoadMenus( const char *menuFile, qboolean reset, int imageTrack );
void        UI_GetGameTypesList( void );
void        UI_VerifyLanguage( void );

extern displayContextDef_t *DC;

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
vmCvar_t ui_new;                                    /* 0x401EF960, never registered */
vmCvar_t ui_serverFilterType;                       /* 0x401C0DA0, never registered: RTCW's table row is gone */

void _UI_Init( void );
void _UI_Shutdown( void );
void _UI_KeyEvent( int key, qboolean down );
void _UI_MouseEvent( int dx, int dy );
void _UI_Refresh( int realtime );
qboolean _UI_IsFullscreen( void );
qboolean _UI_SetActiveMenu( uiMenuCommand_t menu );
uiMenuCommand_t _UI_GetActiveMenu( void );

/* ---- vmMain  0x400076A0 ---- */
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {
	switch ( command ) {
	case UI_GETAPIVERSION:
		return UI_API_VERSION;

	case UI_INIT:
		_UI_Init();
		return 0;

	case UI_SHUTDOWN:
		_UI_Shutdown();
		return 0;

	case UI_KEY_EVENT:
		_UI_KeyEvent( arg0, arg1 );
		return 0;

	case UI_MOUSE_EVENT:
		_UI_MouseEvent( arg0, arg1 );
		return 0;

	case UI_REFRESH:
		_UI_Refresh( arg0 );
		return 0;

	case UI_IS_FULLSCREEN:
		return _UI_IsFullscreen();

	case UI_SET_ACTIVE_MENU:
		/* returns what _UI_SetActiveMenu answers (0x40007713..0x4000771A), not RTCW's 0 */
		return _UI_SetActiveMenu( arg0 );

	case UI_GET_ACTIVE_MENU:
		return _UI_GetActiveMenu();

	case UI_GET_MAP_DISPLAY_NAME:
		return (int)UI_GetMapDisplayName( (const char *)arg0 );

	case UI_GET_GAME_TYPE_DISPLAY_NAME:
		return (int)UI_GetGameTypeDisplayName( (const char *)arg0 );

	case UI_CONSOLE_COMMAND:
		return UI_ConsoleCommand( arg0 );

	case UI_DRAW_CONNECT_SCREEN:
		UI_DrawConnectScreen( arg0 );
		return 0;
	case UI_HASUNIQUECDKEY:             // mod authors need to observe this
		return qtrue;
		// NERVE - SMF
	case UI_CHECKEXECKEY:
		return UI_CheckExecKey( arg0 );

	case UI_LOAD_SCRIPT_MENU:
		return Load_ScriptMenu( (const char *)arg0, arg1 );

	case UI_GET_FONT_INFO:
		/* the engine sends the scale as a percentage (0x40007782 fild/fmul 0.01) */
		return (int)UI_GetFontInfo( arg0, arg1 * 0.01f );
	}

	return -1;
}


/* ---- AssetCache  0x400077F0 -- the nine ui/assets registrations, imageTrack 2 ---- */
void AssetCache( void ) {
	//if (Assets.textFont == NULL) {
	//}
	//Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
	uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR, 2 );
	uiInfo.uiDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR, 2 );
	uiInfo.uiDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN, 2 );
	uiInfo.uiDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP, 2 );
	uiInfo.uiDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT, 2 );
	uiInfo.uiDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT, 2 );
	uiInfo.uiDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB, 2 );
	uiInfo.uiDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR, 2 );
	uiInfo.uiDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB, 2 );
}

/* ---- _UI_DrawSides  0x40007890 ---- */
void _UI_DrawSides( float x, float y, float w, float h, float size ) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiDC.xscale;
	trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

/* ---- _UI_DrawTopBottom  0x40007910 ---- */
void _UI_DrawTopBottom( float x, float y, float w, float h, float size ) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiDC.yscale;
	trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}
/*
================
UI_DrawRect  0x40007990

Coordinates are 640*480 virtual values
=================
*/
void _UI_DrawRect( float x, float y, float width, float height, float size, const float *color ) {
	trap_R_SetColor( color );

	_UI_DrawTopBottom( x, y, width, height, size );
	_UI_DrawSides( x, y, width, height, size );

	trap_R_SetColor( NULL );
}

/*
================
UI_GetFontInfo  0x400079E0

The renderer's font for a menu font index at a text scale.  2, 3 and 5 are
the big, small and console fonts outright; 4 (bold) and the rest pick by the
scaled size against ui_smallFont / ui_bigFont / ui_extraBigFont.
================
*/
fontInfo_t *UI_GetFontInfo( int font, float scale ) {
	float scaledSize;

	switch ( font ) {
	case 2:
		return &uiInfo.uiDC.Assets.bigFont;
	case 3:
		return &uiInfo.uiDC.Assets.smallFont;
	case 5:
		return &uiInfo.uiDC.Assets.consoleFont;
	}

	scaledSize = uiInfo.uiDC.yscale * scale;

	if ( font == 4 ) {
		if ( scaledSize <= ui_smallFont.value ) {
			return &uiInfo.uiDC.Assets.smallFont;
		}
		if ( scaledSize < ui_bigFont.value ) {
			return &uiInfo.uiDC.Assets.textFont;
		}
		return &uiInfo.uiDC.Assets.boldFont;
	}

	if ( scaledSize <= ui_smallFont.value ) {
		return &uiInfo.uiDC.Assets.smallFont;
	}
	if ( scaledSize >= ui_extraBigFont.value ) {
		return &uiInfo.uiDC.Assets.extraBigFont;
	}
	if ( scaledSize >= ui_bigFont.value ) {
		return &uiInfo.uiDC.Assets.bigFont;
	}
	return &uiInfo.uiDC.Assets.textFont;
}

// NERVE - SMF
/* ---- Text_SetActiveFont  0x40007A70 ---- */
void Text_SetActiveFont( int font ) {
	uiInfo.activeFont = font;
}

/* ---- UI_ShowPostGame  0x40007A80 -- RTCW's, less cg_cameraOrbit ---- */
void UI_ShowPostGame( qboolean newHigh ) {
	trap_Cvar_Set( "cg_thirdPerson", "0" );
	trap_Cvar_Set( "sv_killserver", "1" );
	uiInfo.soundHighScore = newHigh;
	_UI_SetActiveMenu( UIMENU_POSTGAME );
}
/*
=================
_UI_Refresh
=================
*/

/* ---- UI_DrawCenteredPic  0x40007AC0 -- no caller survives in the DLL ---- */
void UI_DrawCenteredPic( qhandle_t image, int w, int h ) {
	int x, y;
	x = ( SCREEN_WIDTH - w ) / 2;
	y = ( SCREEN_HEIGHT - h ) / 2;
	UI_DrawHandlePic( x, y, w, h, image );
}

#define UI_FPS_FRAMES   4
/* ---- _UI_Refresh  0x40007B20 ---- */
void _UI_Refresh( int realtime ) {
	static int index;                       /* 0x4005A980 */
	static int previousTimes[UI_FPS_FRAMES]; /* 0x4005B190 */

	//if ( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
	//	return;
	//}

	uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realtime;

	previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiDC.frameTime;
	index++;
	if ( index > UI_FPS_FRAMES ) {
		int i, total;
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < UI_FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		uiInfo.uiDC.FPS = 1000 * UI_FPS_FRAMES / total;
	}



	UI_UpdateCvars();

	if ( Menu_Count() > 0 ) {
		// paint all the menus
		Menu_PaintAll();
		// refresh server browser list
		UI_DoServerRefresh();
		// refresh server status
		UI_BuildServerStatus( qfalse );
		// refresh find player list
		UI_BuildFindPlayerList( qfalse );
	}

	// draw cursor
	UI_SetColor( NULL );
	if ( Menu_Count() > 0 ) {
		UI_DrawHandlePic( uiInfo.uiDC.cursorx - 16, uiInfo.uiDC.cursory - 16, 32, 32, uiInfo.uiDC.Assets.cursor );
	}

#ifndef NDEBUG
	if ( uiInfo.uiDC.debug ) {
		// cursor coordinates
		//FIXME
		//UI_DrawString( 0, 0, va("(%d,%d)",uis.cursorx,uis.cursory), UI_LEFT|UI_SMALLFONT, colorRed );
	}
#endif

}

/*
=================
_UI_Shutdown  0x40007C20
=================
*/
void _UI_Shutdown( void ) {
	Menus_CloseAll();
	trap_LAN_SaveCachedServers();
}

char *defaultMenu = NULL;                           /* 0x401BF420 */

/* ---- getMenuBuffer  0x40007C30 -- RTCW's GetMenuBuffer ---- */
char *getMenuBuffer( const char *filename ) {
	int len;
	fileHandle_t f;
	static char buf[MAX_MENUFILE];                  /* 0x40052960 */

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return defaultMenu;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return defaultMenu;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	//COM_Compress(buf);
	return buf;

}

/* ---- Asset_Parse  0x40007CC0 -- RTCW's plus extraBigFont/boldFont/consoleFont/fadeInAmount and the imageTrack ---- */
qboolean Asset_Parse( int handle, int imageTrack ) {
	pc_token_t token;
	const char *tempStr;

	if ( !trap_PC_ReadToken( handle, &token ) ) {
		return qfalse;
	}
	if ( Q_stricmp( token.string, "{" ) != 0 ) {
		return qfalse;
	}

	while ( 1 ) {

		memset( &token, 0, sizeof( pc_token_t ) );

		if ( !trap_PC_ReadToken( handle, &token ) ) {
			return qfalse;
		}

		if ( Q_stricmp( token.string, "}" ) == 0 ) {
			return qtrue;
		}

		// font
		if ( Q_stricmp( token.string, "font" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle,&pointSize ) ) {
				return qfalse;
			}
			trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.textFont, imageTrack );
			uiInfo.uiDC.Assets.fontRegistered = qtrue;
			continue;
		}

		if ( Q_stricmp( token.string, "smallFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle,&pointSize ) ) {
				return qfalse;
			}
			trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.smallFont, imageTrack );
			continue;
		}

		if ( Q_stricmp( token.string, "bigFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle,&pointSize ) ) {
				return qfalse;
			}
			trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.bigFont, imageTrack );
			continue;
		}

		if ( Q_stricmp( token.string, "extraBigFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle,&pointSize ) ) {
				return qfalse;
			}
			trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.extraBigFont, imageTrack );
			continue;
		}

		if ( Q_stricmp( token.string, "boldFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle,&pointSize ) ) {
				return qfalse;
			}
			trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.boldFont, imageTrack );
			continue;
		}

		if ( Q_stricmp( token.string, "consoleFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle,&pointSize ) ) {
				return qfalse;
			}
			trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.consoleFont, imageTrack );
			continue;
		}


		// gradientbar
		if ( Q_stricmp( token.string, "gradientbar" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( tempStr, imageTrack );
			continue;
		}

		// enterMenuSound
		if ( Q_stricmp( token.string, "menuEnterSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr );
			continue;
		}

		// exitMenuSound
		if ( Q_stricmp( token.string, "menuExitSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr );
			continue;
		}

		// itemFocusSound
		if ( Q_stricmp( token.string, "itemFocusSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr );
			continue;
		}

		// menuBuzzSound
		if ( Q_stricmp( token.string, "menuBuzzSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr );
			continue;
		}

		if ( Q_stricmp( token.string, "cursor" ) == 0 ) {
			if ( !PC_String_Parse( handle, &uiInfo.uiDC.Assets.cursorStr ) ) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.cursor = trap_R_RegisterShaderNoMip( uiInfo.uiDC.Assets.cursorStr, imageTrack );
			continue;
		}

		if ( Q_stricmp( token.string, "fadeClamp" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.fadeClamp ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeCycle" ) == 0 ) {
			if ( !PC_Int_Parse( handle, &uiInfo.uiDC.Assets.fadeCycle ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeAmount" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.fadeAmount ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeInAmount" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.fadeInAmount ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowX" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.shadowX ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowY" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.shadowY ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowColor" ) == 0 ) {
			if ( !PC_Color_Parse( handle, &uiInfo.uiDC.Assets.shadowColor ) ) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.shadowFadeClamp = uiInfo.uiDC.Assets.shadowColor[3];
			continue;
		}

	}
	return qfalse;
}

/* ---- Font_Report  0x400081E0 ---- */
void Font_Report( void ) {
	int i;
	Com_Printf( "Font Info\n" );
	Com_Printf( "=========\n" );
	for ( i = 32; i < 96; i++ ) {
		Com_Printf( "Glyph handle %i: %i\n", i, uiInfo.uiDC.Assets.textFont.glyphs[i].glyph );
	}
}

/* ---- UI_Report  0x40008230 ---- */
void UI_Report( void ) {
	String_Report();
	//Font_Report();

}

/* ---- UI_ParseMenu  0x40008240 -- RTCW's plus the imageTrack handed to Asset_Parse and Menu_New ---- */
qboolean UI_ParseMenu( const char *menuFile, int imageTrack ) {
	int handle;
	pc_token_t token;

	Com_DPrintf( "Parsing menu file:%s\n", menuFile );

	handle = trap_PC_LoadSource( menuFile );
	if ( !handle ) {
		return qfalse;
	}

	while ( 1 ) {
		memset( &token, 0, sizeof( pc_token_t ) );
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( Q_stricmp( token.string, "assetGlobalDef" ) == 0 ) {
			if ( Asset_Parse( handle, imageTrack ) ) {
				continue;
			} else {
				break;
			}
		}

		if ( Q_stricmp( token.string, "menudef" ) == 0 ) {
			// start a new menu
			Menu_New( handle, imageTrack );
		}
	}
	trap_PC_FreeSource( handle );
	return qtrue;
}

/*
===============
Load_ScriptMenu  0x40008330

vmMain's UI_LOAD_SCRIPT_MENU: ui_mp/scriptmenus/<name>.menu, the language
subdirectory first when cl_language is set.  The engine passes imageTrack 5.
===============
*/
qboolean Load_ScriptMenu( const char *menuname, int imageTrack ) {
	char menuFile[256];
	int cl_language;

	strcpy( menuFile, "ui_mp/scriptmenus/" );
	strcat( menuFile, menuname );
	strcat( menuFile, ".menu" );

	cl_language = atoi( UI_Cvar_VariableString( "cl_language" ) );

	if ( cl_language ) {
		const char *filename;
		char out[256];

		Com_StripFilename( menuFile, out );

		filename = Com_SkipPath( menuFile );

		if ( UI_ParseMenu( va( "%s%s", va( "%s%s/", out, trap_GetLanguagename( cl_language ) ), filename ), imageTrack ) ) {
			return qtrue;
		}
	}

	if ( UI_ParseMenu( menuFile, imageTrack ) ) {
		return qtrue;
	}
	return qfalse;
}

/* ---- Load_Menu  0x40008480 -- RTCW's, the language directory from trap_GetLanguagename ---- */
qboolean Load_Menu( int handle, int imageTrack ) {
	pc_token_t token;
	int cl_language;    // NERVE - SMF

	if ( !trap_PC_ReadToken( handle, &token ) ) {
		return qfalse;
	}
	if ( token.string[0] != '{' ) {
		return qfalse;
	}

	while ( 1 ) {

		if ( !trap_PC_ReadToken( handle, &token ) ) {
			return qfalse;
		}

		if ( token.string[0] == 0 ) {
			return qfalse;
		}

		if ( token.string[0] == '}' ) {
			return qtrue;
		}

		// NERVE - SMF - localization crap
		cl_language = atoi( UI_Cvar_VariableString( "cl_language" ) );

		if ( cl_language ) {
			const char *filename;
			char out[256];

			Com_StripFilename( token.string, out );

			filename = Com_SkipPath( token.string );

			if ( UI_ParseMenu( va( "%s%s", va( "%s%s/", out, trap_GetLanguagename( cl_language ) ), filename ), imageTrack ) ) {
				continue;
			}
		}
		// -NERVE

		UI_ParseMenu( token.string, imageTrack );
	}
	return qfalse;
}

/* ---- UI_LoadMenus  0x400085B0 ---- */
void UI_LoadMenus( const char *menuFile, qboolean reset, int imageTrack ) {
	pc_token_t token;
	int handle;
	int start;

	start = trap_Milliseconds();

	handle = trap_PC_LoadSource( menuFile );
	if ( !handle ) {
		trap_Error( va( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile ) );
		handle = trap_PC_LoadSource( "ui_mp/menus.txt" );
		if ( !handle ) {
			trap_Error( va( S_COLOR_RED "default menu file not found: ui_mp/menus.txt, unable to continue!\n", menuFile ) );
		}
	}

	ui_new.integer = 1;

	if ( reset ) {
		Menu_Reset();
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}
		if ( token.string[0] == 0 || token.string[0] == '}' ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( Q_stricmp( token.string, "loadmenu" ) == 0 ) {
			if ( Load_Menu( handle, imageTrack ) ) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_DPrintf( "UI menu load time = %d milli seconds\n", trap_Milliseconds() - start );

	trap_PC_FreeSource( handle );
}

/* ---- UI_Load  0x400086C0 -- UI_GetGameTypesList for RTCW's gameinfo.txt, Menus_OpenByName for ActivateByName ---- */
void UI_Load( void ) {
	char lastName[1024];
	menuDef_t *menu = Menu_GetFocused();
	char *menuSet = UI_Cvar_VariableString( "ui_menuFiles" );
	if ( menu && menu->window.name ) {
		strcpy( lastName, menu->window.name );
	}
	if ( menuSet == NULL || menuSet[0] == '\0' ) {
		menuSet = "ui_mp/menus.txt";
	}

	String_Init();

	UI_GetGameTypesList();
	UI_LoadArenas();

	UI_LoadMenus( menuSet, qtrue, 2 );
	Menus_CloseAll();
	Menus_OpenByName( lastName );

}

static const char *handicapValues[] = {"None","95","90","85","80","75","70","65","60","55","50","45","40","35","30","25","20","15","10","5",NULL};  /* 0x40036E70 */
//static int numHandicaps = sizeof(handicapValues) / sizeof(const char*); // TTimo: unused

/* ---- UI_DrawHandicap  0x40008760 ---- */
static void UI_DrawHandicap( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	int i, h;

	h = Com_Clamp( 5, 100, trap_Cvar_VariableValue( "handicap" ) );
	i = 20 - h / 5;

	trap_R_Text_Paint( rect->x, rect->y, font, scale, color, handicapValues[i], 0, 0, textStyle );
}

// ui_gameType assumes gametype 0 is -1 ALL and will not show
/* ---- UI_DrawGameType  0x400087D0 -- gtEnum is the display name; "All" when it is empty ---- */
static void UI_DrawGameType( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	const char *s = uiInfo.gameTypes[ui_gameType.integer].gtEnum;

	if ( *s ) {
		trap_R_Text_Paint( rect->x, rect->y, font, scale, color, s, 0, 0, textStyle );
	} else {
		trap_R_Text_Paint( rect->x, rect->y, font, scale, color, "All", 0, 0, textStyle );
	}
}

/* ---- UI_DrawNetGameType  0x40008820 ---- */
static void UI_DrawNetGameType( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	const char *s;

	if ( ui_netGameType.integer < 0 || ui_netGameType.integer > uiInfo.numGameTypes ) {
		ui_netGameType.integer = 0;
		trap_Cvar_Set( "ui_netGameType", "0" );
		trap_Cvar_Set( "ui_netGameTypeName", uiInfo.gameTypes[0].gameType );
	}
	s = uiInfo.gameTypes[ui_netGameType.integer].gtEnum;
	if ( *s ) {
		s = UI_SafeTranslateString( s );
	} else {
		s = UI_SafeTranslateString( "EXE_ALL" );
	}
	trap_R_Text_Paint( rect->x, rect->y, font, scale, color, s, 0, 0, textStyle );
}

/* ---- UI_DrawJoinGameType  0x400088B0 ---- */
static void UI_DrawJoinGameType( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	const char *s;

	if ( ui_joinGameType.integer < 0 || ui_joinGameType.integer > uiInfo.numJoinGameTypes ) {
		ui_joinGameType.integer = 0;
		trap_Cvar_Set( "ui_joinGameType", "0" );
	}
	s = uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum;
	if ( *s ) {
		s = UI_SafeTranslateString( s );
	} else {
		s = UI_SafeTranslateString( "EXE_ALL" );
	}
	trap_R_Text_Paint( rect->x, rect->y, font, scale, color, s, 0, 0, textStyle );
}

/* ---- UI_DrawPreviewCinematic  0x40008930 ---- */
static void UI_DrawPreviewCinematic( rectDef_t *rect, float scale, vec4_t color ) {
	if ( uiInfo.previewMovie > -2 ) {
		uiInfo.previewMovie = trap_CIN_PlayCinematic( va( "%s.roq", uiInfo.movieList[uiInfo.movieIndex] ), 0, 0, 0, 0, ( CIN_loop | CIN_silent ) );
		if ( uiInfo.previewMovie >= 0 ) {
			trap_CIN_RunCinematic( uiInfo.previewMovie );
			trap_CIN_SetExtents( uiInfo.previewMovie, rect->x, rect->y, rect->w, rect->h );
			trap_CIN_DrawCinematic( uiInfo.previewMovie );
		} else {
			uiInfo.previewMovie = -2;
		}
	}

}

/* ---- UI_DrawMapPreview  0x400089E0 ---- */
static void UI_DrawMapPreview( rectDef_t *rect, float scale, vec4_t color, qboolean net ) {
	int map = ( net ) ? ui_currentNetMap.integer : ui_currentMap.integer;
	if ( map < 0 || map > uiInfo.mapCount ) {
		if ( net ) {
			ui_currentNetMap.integer = 0;
			trap_Cvar_Set( "ui_currentNetMap", "0" );
		} else {
			ui_currentMap.integer = 0;
			trap_Cvar_Set( "ui_currentMap", "0" );
		}
		map = 0;
	}

	if ( uiInfo.mapList[map].levelShot == -1 ) {
		uiInfo.mapList[map].levelShot = trap_R_RegisterShaderNoMip( uiInfo.mapList[map].imageName, 2 );
	}

	if ( uiInfo.mapList[map].levelShot > 0 ) {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.mapList[map].levelShot );
	} else {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip( "menu/art/unknownmap", 2 ) );
	}
}

/* ---- UI_DrawMapCinematic  0x40008AA0 -- RTCW's, the cinematic half gone: always the preview ---- */
static void UI_DrawMapCinematic( rectDef_t *rect, float scale, vec4_t color, qboolean net ) {

	int map = ( net ) ? ui_currentNetMap.integer : ui_currentMap.integer;
	if ( map < 0 || map > uiInfo.mapCount ) {
		if ( net ) {
			ui_currentNetMap.integer = 0;
			trap_Cvar_Set( "ui_currentNetMap", "0" );
		} else {
			ui_currentMap.integer = 0;
			trap_Cvar_Set( "ui_currentMap", "0" );
		}
		map = 0;
	}

	UI_DrawMapPreview( rect, scale, color, net );
}

/* ---- UI_DrawNetSource  0x40008B00 ----
   The literal at 0x4002F058 is "EXE_NETSOURCE\x14%s" (0x4002F034 is the
   SERVERFILTER twin): key, then the \x14 that SE_LocalizeMessage substitutes
   the tail into "Source:     %s". */
static void UI_DrawNetSource( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	if ( ui_netSource.integer < 0 || ui_netSource.integer > numNetSources /*uiInfo.numGameTypes*/ ) {        // NERVE - SMF - possible bug
		ui_netSource.integer = 0;
	}
	trap_R_Text_Paint( rect->x, rect->y, font, scale, color, trap_SE_LocalizeMessage( va( "EXE_NETSOURCE\x14%s", netSources[ui_netSource.integer] ), "net source" ), 0, 0, textStyle );
}

/* ---- UI_DrawNetMapPreview  0x40008B60 ---- */
static void UI_DrawNetMapPreview( rectDef_t *rect, float scale, vec4_t color ) {

	if ( uiInfo.serverStatus.currentServerPreview > 0 ) {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.serverStatus.currentServerPreview );
	} else {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip( "menu/art/unknownmap", 2 ) );
	}
}

/* ---- UI_DrawNetMapCinematic  0x40008BB0 ---- */
static void UI_DrawNetMapCinematic( rectDef_t *rect, float scale, vec4_t color ) {
	if ( ui_currentNetMap.integer < 0 || ui_currentNetMap.integer > uiInfo.mapCount ) {
		ui_currentNetMap.integer = 0;
		trap_Cvar_Set( "ui_currentNetMap", "0" );
	}

	if ( uiInfo.serverStatus.currentServerCinematic >= 0 ) {
		trap_CIN_RunCinematic( uiInfo.serverStatus.currentServerCinematic );
		trap_CIN_SetExtents( uiInfo.serverStatus.currentServerCinematic, rect->x, rect->y, rect->w, rect->h );
		trap_CIN_DrawCinematic( uiInfo.serverStatus.currentServerCinematic );
	} else {
		UI_DrawNetMapPreview( rect, scale, color );
	}
}

/* ---- UI_DrawNetFilter  0x40008C50 ---- */
static void UI_DrawNetFilter( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	if ( ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters ) {
		ui_serverFilterType.integer = 0;
	}
	trap_R_Text_Paint( rect->x, rect->y, font, scale, color, trap_SE_LocalizeMessage( va( "EXE_SERVERFILTER\x14%s", serverFilters[ui_serverFilterType.integer].description ), "server filter" ), 0, 0, textStyle );
}

/* ---- UI_GetMapDisplayName  0x40008CB0 -- vmMain's UI_GET_MAP_DISPLAY_NAME ---- */
const char *UI_GetMapDisplayName( const char *mapname ) {
	int i;

	for ( i = 0; i < uiInfo.mapCount; i++ ) {
		if ( !Q_stricmp( uiInfo.mapList[i].mapLoadName, mapname ) ) {
			return uiInfo.mapList[i].mapName;
		}
	}
	return mapname;
}

/* ---- UI_GetGameTypeDisplayName  0x40008D00 -- vmMain's UI_GET_GAME_TYPE_DISPLAY_NAME ---- */
const char *UI_GetGameTypeDisplayName( const char *gametype ) {
	int i;

	for ( i = 0; i < uiInfo.numGameTypes; i++ ) {
		if ( !Q_stricmp( uiInfo.gameTypes[i].gameType, gametype ) ) {
			return uiInfo.gameTypes[i].gtEnum;
		}
	}
	return gametype;
}

/* ---- UI_OwnerDrawWidth  0x40008D40 ---- */
static int UI_OwnerDrawWidth( int ownerDraw, int font, float scale ) {
	int i, h;
	const char *s = NULL;

	switch ( ownerDraw ) {
	case UI_HANDICAP:
		h = Com_Clamp( 5, 100, trap_Cvar_VariableValue( "handicap" ) );
		i = 20 - h / 5;
		s = handicapValues[i];
		break;
	case UI_GAMETYPE:
		s = uiInfo.gameTypes[ui_gameType.integer].gtEnum;
		break;
	case UI_NETSOURCE:
		if ( ui_netSource.integer < 0 || ui_netSource.integer > uiInfo.numJoinGameTypes ) {
			ui_netSource.integer = 0;
		}
		s = trap_SE_LocalizeMessage( va( "EXE_NETSOURCE\x14%s", netSources[ui_netSource.integer] ), "net source" );
		break;
	case UI_NETFILTER:
		if ( ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters ) {
			ui_serverFilterType.integer = 0;
		}
		s = trap_SE_LocalizeMessage( va( "EXE_SERVERFILTER\x14%s", serverFilters[ui_serverFilterType.integer].description ), "server filter" );
		break;
	case UI_KEYBINDSTATUS:
		if ( Display_KeyBindPending() ) {
			s = UI_SafeTranslateString( "EXE_KEYWAIT" );
		} else {
			s = UI_SafeTranslateString( "EXE_KEYCHANGE" );
		}
		break;
	case UI_SERVERREFRESHDATE:
		s = UI_Cvar_VariableString( va( "ui_lastServerRefresh_%i", ui_netSource.integer ) );
		break;
	default:
		break;
	}

	if ( s ) {
		return trap_R_Text_Width( s, font, scale, 0 );
	}
	return 0;
}

/* ---- UI_BuildPlayerList  0x40008ED0 -- names from trap_GetClientName, no team bookkeeping ---- */
static void UI_BuildPlayerList( void ) {
	uiClientState_t cs;
	int n, count;
	char info[MAX_INFO_STRING];
	char name[MAX_NAME_LENGTH];

	trap_GetClientState( &cs );
	uiInfo.playerNumber = cs.clientNum;
	trap_GetConfigString( CS_SERVERINFO, info, sizeof( info ) );
	count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	uiInfo.playerCount = 0;
	for ( n = 0; n < count; n++ ) {
		if ( trap_GetClientName( n, name, sizeof( name ) ) ) {
			Q_strncpyz( uiInfo.playerNames[uiInfo.playerCount], name, MAX_NAME_LENGTH );
			Q_CleanStr( uiInfo.playerNames[uiInfo.playerCount] );
			uiInfo.playerCount++;
		}
	}
}

/* ---- UI_DrawServerRefreshDate  0x40008FB0 ---- */
static void UI_DrawServerRefreshDate( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	int serverCount;            // NERVE - SMF
//#ifdef MISSIONPACK
	if ( uiInfo.serverStatus.refreshActive ) {
		vec4_t lowLight, newColor;
		lowLight[0] = 0.8 * color[0];
		lowLight[1] = 0.8 * color[1];
		lowLight[2] = 0.8 * color[2];
		lowLight[3] = 0.8 * color[3];
		LerpColor( color,lowLight,newColor,0.5 * ( 1.0 + sin( uiInfo.uiDC.realTime / PULSE_DIVISOR ) ) );
		// NERVE - SMF
		serverCount = trap_LAN_GetServerCount( ui_netSource.integer );
		if ( serverCount >= 0 ) {
			trap_R_Text_Paint( rect->x, rect->y, font, scale, newColor, va( UI_SafeTranslateString( "EXE_GETTINGINFOFORSERVERS" ), serverCount ), 0, 0, textStyle );
		} else {
			trap_R_Text_Paint( rect->x, rect->y, font, scale, newColor, UI_SafeTranslateString( "EXE_WAITINGFORMASTERSERVERRESPONSE" ), 0, 0, textStyle );
		}
	} else {
		char buff[64];
		Q_strncpyz( buff, UI_Cvar_VariableString( va( "ui_lastServerRefresh_%i", ui_netSource.integer ) ), 64 );
		trap_R_Text_Paint( rect->x, rect->y, font, scale, color, va( UI_SafeTranslateString( "EXE_REFRESHTIME" ), buff ), 0, 0, textStyle );
	}
//#endif	// #ifdef MISSIONPACK
}

/* ---- UI_DrawKeyBindStatus  0x40009180 ---- */
static void UI_DrawKeyBindStatus( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	//int ofs = 0; // TTimo: unused
	if ( Display_KeyBindPending() ) {
		trap_R_Text_Paint( rect->x, rect->y, font, scale, color, UI_SafeTranslateString( "EXE_KEYWAIT" ), 0, 0, textStyle );
	} else {
		trap_R_Text_Paint( rect->x, rect->y, font, scale, color, UI_SafeTranslateString( "EXE_KEYCHANGE" ), 0, 0, textStyle );
	}
}

/* ---- UI_DrawGLInfo  0x400091C0 -- 256 lines in three columns, limits 80/36, float y ---- */
static void UI_DrawGLInfo( rectDef_t *rect, int font, float scale, vec4_t color, int textStyle ) {
	char * eptr;
	char buff[4096];
	const char *lines[256];
	float y;
	int numLines, i;

	trap_R_Text_Paint( rect->x + 2, rect->y, font, scale, color, va( "VENDOR: %s", UI_GLCONFIG.vendor_string ), 0, 80, textStyle );
	trap_R_Text_Paint( rect->x + 2, rect->y + 15, font, scale, color, va( "VERSION: %s: %s", UI_GLCONFIG.version_string,UI_GLCONFIG.renderer_string ), 0, 80, textStyle );
	trap_R_Text_Paint( rect->x + 2, rect->y + 30, font, scale, color, va( "PIXELFORMAT: color(%d-bits) Z(%d-bits) stencil(%d-bits)", UI_GLCONFIG.colorBits, UI_GLCONFIG.depthBits, UI_GLCONFIG.stencilBits ), 0, 80, textStyle );

	// build null terminated extension strings
	Q_strncpyz( buff, UI_GLCONFIG.extensions_string, 4096 );
	eptr = buff;
	y = rect->y + 45;
	numLines = 0;
	while ( y < rect->y + rect->h && *eptr && numLines < 256 )
	{
		while ( *eptr && *eptr == ' ' )
			*eptr++ = '\0';

		// track start of valid string
		if ( *eptr && *eptr != ' ' ) {
			lines[numLines++] = eptr;
		}

		while ( *eptr && *eptr != ' ' )
			eptr++;
	}

	i = 0;
	while ( i < numLines ) {
		trap_R_Text_Paint( rect->x + 2, y, font, scale, color, lines[i++], 0, 36, textStyle );
		if ( i < numLines ) {
			trap_R_Text_Paint( rect->x + rect->w / 3, y, font, scale, color, lines[i++], 0, 36, textStyle );
			if ( i < numLines ) {
				trap_R_Text_Paint( rect->x + ( rect->w / 3 ) * 2, y, font, scale, color, lines[i++], 0, 36, textStyle );
			}
		}
		y += 10;
		if ( y > rect->y + rect->h - 11 ) {
			break;
		}
	}


}

// FIXME: table drive
//
/* ---- UI_OwnerDraw  0x400094A0 -- the DC's ownerDrawItem; font sits between special and scale ---- */
static void UI_OwnerDraw( float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, int font, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	rectDef_t rect;

	rect.x = x + text_x;
	rect.y = y + text_y;
	rect.w = w;
	rect.h = h;

	switch ( ownerDraw ) {
	case UI_HANDICAP:
		UI_DrawHandicap( &rect, font, scale, color, textStyle );
		break;
	case UI_PREVIEWCINEMATIC:
		UI_DrawPreviewCinematic( &rect, scale, color );
		break;
	case UI_GAMETYPE:
		UI_DrawGameType( &rect, font, scale, color, textStyle );
		break;
	case UI_NETGAMETYPE:
		UI_DrawNetGameType( &rect, font, scale, color, textStyle );
		break;
	case UI_JOINGAMETYPE:
		UI_DrawJoinGameType( &rect, font, scale, color, textStyle );
		break;
	case UI_MAPPREVIEW:
		UI_DrawMapPreview( &rect, scale, color, qtrue );
		break;
	case UI_MAPCINEMATIC:
		UI_DrawMapCinematic( &rect, scale, color, qfalse );
		break;
	case UI_STARTMAPCINEMATIC:
		UI_DrawMapCinematic( &rect, scale, color, qtrue );
		break;
	case UI_NETSOURCE:
		UI_DrawNetSource( &rect, font, scale, color, textStyle );
		break;
	case UI_NETMAPPREVIEW:
		UI_DrawNetMapPreview( &rect, scale, color );
		break;
	case UI_NETMAPCINEMATIC:
		UI_DrawNetMapCinematic( &rect, scale, color );
		break;
	case UI_NETFILTER:
		UI_DrawNetFilter( &rect, font, scale, color, textStyle );
		break;
	case UI_SERVERREFRESHDATE:
		UI_DrawServerRefreshDate( &rect, font, scale, color, textStyle );
		break;
	case UI_GLINFO:
		UI_DrawGLInfo( &rect, font, scale, color, textStyle );
		break;
	case UI_KEYBINDSTATUS:
		UI_DrawKeyBindStatus( &rect, font, scale, color, textStyle );
		break;
	default:
		break;
	}
}

/* ---- UI_OwnerDrawVisible  0x40009780 -- FFA is g_gametype "dm"; the team-game flags are gone ---- */
static qboolean UI_OwnerDrawVisible( int flags ) {
	qboolean vis = qtrue;

	while ( flags ) {

		if ( flags & UI_SHOW_FFA ) {
			if ( Q_stricmp( "dm", UI_Cvar_VariableString( "g_gametype" ) ) != 0 ) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_FFA;
		}

		if ( flags & UI_SHOW_NOTFFA ) {
			if ( Q_stricmp( "dm", UI_Cvar_VariableString( "g_gametype" ) ) == 0 ) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_NOTFFA;
		}

		if ( flags & UI_SHOW_LEADER ) {
			// these need to show when this client can give orders to a player or a group
			if ( !uiInfo.teamLeader ) {
				vis = qfalse;
			} else {
				// if showing yourself
				if ( ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber ) {
					vis = qfalse;
				}
			}
			flags &= ~UI_SHOW_LEADER;
		}
		if ( flags & UI_SHOW_NOTLEADER ) {
			// these need to show when this client is assigning their own status or they are NOT the leader
			if ( uiInfo.teamLeader ) {
				// if not showing yourself
				if ( !( ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber ) ) {
					vis = qfalse;
				}
			}
			flags &= ~UI_SHOW_NOTLEADER;
		}
		if ( flags & UI_SHOW_FAVORITESERVERS ) {
			// this assumes you only put this type of display flag on something showing in the proper context
			if ( ui_netSource.integer != AS_FAVORITES ) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_FAVORITESERVERS;
		}
		if ( flags & UI_SHOW_NOTFAVORITESERVERS ) {
			// this assumes you only put this type of display flag on something showing in the proper context
			if ( ui_netSource.integer == AS_FAVORITES ) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_NOTFAVORITESERVERS;
		}
		if ( flags & UI_SHOW_NEWHIGHSCORE ) {
			if ( uiInfo.newHighScoreTime < uiInfo.uiDC.realTime ) {
				vis = qfalse;
			} else {
				if ( uiInfo.soundHighScore ) {
					if ( trap_Cvar_VariableValue( "sv_killserver" ) == 0 ) {
						// wait on server to go down before playing sound
						/* RTCW's handle, pushed whole to a trap that now takes an alias
						   name (0x400098A4); nothing in this DLL ever assigns it, so the
						   engine sees NULL and MSS_PlayLocalSoundAlias returns early. */
						trap_S_StartLocalSound( (const char *)uiInfo.newHighScoreSound );
						uiInfo.soundHighScore = qfalse;
					}
				}
			}
			flags &= ~UI_SHOW_NEWHIGHSCORE;
		}
		if ( flags & UI_SHOW_NEWBESTTIME ) {
			if ( uiInfo.newBestTime < uiInfo.uiDC.realTime ) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_NEWBESTTIME;
		}
		if ( flags & UI_SHOW_DEMOAVAILABLE ) {
			if ( !uiInfo.demoAvailable ) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_DEMOAVAILABLE;
		} else {
			flags = 0;
		}
	}
	return vis;
}

/* ---- UI_Handicap_HandleKey  0x40009900 ---- */
static qboolean UI_Handicap_HandleKey( int flags, float *special, int key ) {
	if ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER ) {
		int h;
		h = Com_Clamp( 5, 100, trap_Cvar_VariableValue( "handicap" ) );
		if ( key == K_MOUSE2 ) {
			h -= 5;
		} else {
			h += 5;
		}
		if ( h > 100 ) {
			h = 5;
		} else if ( h < 0 ) {
			h = 100;
		}
		trap_Cvar_Set( "handicap", va( "%i", h ) );
		return qtrue;
	}
	return qfalse;
}

/* ---- UI_GameType_HandleKey  0x40009990 ---- */
static qboolean UI_GameType_HandleKey( int flags, float *special, int key, qboolean resetMap ) {
//#ifdef MISSIONPACK
	if ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER ) {
		int oldCount = UI_MapCountByGameType();

		// hard coded mess here
		if ( key == K_MOUSE2 ) {
			ui_gameType.integer--;
			if ( ui_gameType.integer == 2 ) {
				ui_gameType.integer = 1;
			} else if ( ui_gameType.integer < 2 ) {
				ui_gameType.integer = uiInfo.numGameTypes - 1;
			}
		} else {
			ui_gameType.integer++;
			if ( ui_gameType.integer >= uiInfo.numGameTypes ) {
				ui_gameType.integer = 1;
			} else if ( ui_gameType.integer == 2 ) {
				ui_gameType.integer = 3;
			}
		}

		trap_Cvar_Set( "ui_gameType", va( "%d", ui_gameType.integer ) );
		if ( resetMap && oldCount != UI_MapCountByGameType() ) {
			trap_Cvar_Set( "ui_currentMap", "0" );
			Menu_SetFeederSelection( NULL, FEEDER_MAPS, 0, NULL );
		}
		return qtrue;
	}
//#endif	// #ifdef MISSIONPACK
	return qfalse;
}

/* ---- UI_NetGameType_HandleKey  0x40009A60 ---- */
static qboolean UI_NetGameType_HandleKey( int flags, float *special, int key ) {
//#ifdef MISSIONPACK
	if ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER ) {

		if ( key == K_MOUSE2 ) {
			ui_netGameType.integer--;
		} else {
			ui_netGameType.integer++;
		}

		if ( ui_netGameType.integer < 0 ) {
			ui_netGameType.integer = uiInfo.numGameTypes - 1;
		} else if ( ui_netGameType.integer >= uiInfo.numGameTypes ) {
			ui_netGameType.integer = 0;
		}

		trap_Cvar_Set( "ui_netGameType", va( "%d", ui_netGameType.integer ) );
		trap_Cvar_Set( "ui_netGameTypeName", uiInfo.gameTypes[ui_netGameType.integer].gameType );
		trap_Cvar_Set( "ui_currentNetMap", "0" );
		UI_MapCountByGameType();
		Menu_SetFeederSelection( NULL, FEEDER_ALLMAPS, 0, NULL );
		UI_SelectCurrentMap();
		return qtrue;
	}
//#endif	// #ifdef MISSIONPACK
	return qfalse;
}

/* ---- UI_JoinGameType_HandleKey  0x40009B10 ---- */
static qboolean UI_JoinGameType_HandleKey( int flags, float *special, int key ) {
//#ifdef MISSIONPACK
	if ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER ) {

		if ( key == K_MOUSE2 ) {
			ui_joinGameType.integer--;
		} else {
			ui_joinGameType.integer++;
		}

		if ( ui_joinGameType.integer < 0 ) {
			ui_joinGameType.integer = uiInfo.numJoinGameTypes - 1;
		} else if ( ui_joinGameType.integer >= uiInfo.numJoinGameTypes ) {
			ui_joinGameType.integer = 0;
		}

		trap_Cvar_Set( "ui_joinGameType", va( "%d", ui_joinGameType.integer ) );
		UI_BuildServerDisplayList( qtrue );
		return qtrue;
	}
//#endif	// #ifdef MISSIONPACK
	return qfalse;
}

/* ---- UI_NetSource_HandleKey  0x40009B90 ---- */
static qboolean UI_NetSource_HandleKey( int flags, float *special, int key ) {
//#ifdef MISSIONPACK
	if ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER ) {

		if ( key == K_MOUSE2 ) {
			ui_netSource.integer--;
		} else {
			ui_netSource.integer++;
		}

		if ( ui_netSource.integer >= numNetSources ) {
			ui_netSource.integer = 0;
		} else if ( ui_netSource.integer < 0 ) {
			ui_netSource.integer = numNetSources - 1;
		}

		UI_BuildServerDisplayList( qtrue );
		if ( ui_netSource.integer != AS_GLOBAL ) {
			UI_StartServerRefresh( qtrue );
		}
		trap_Cvar_Set( "ui_netSource", va( "%d", ui_netSource.integer ) );
		return qtrue;
	}
//#endif	// #ifdef MISSIONPACK
	return qfalse;
}

/* ---- UI_NetFilter_HandleKey  0x40009C30 ---- */
static qboolean UI_NetFilter_HandleKey( int flags, float *special, int key ) {
//#ifdef MISSIONPACK
	if ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER ) {

		if ( key == K_MOUSE2 ) {
			ui_serverFilterType.integer--;
		} else {
			ui_serverFilterType.integer++;
		}

		if ( ui_serverFilterType.integer >= numServerFilters ) {
			ui_serverFilterType.integer = 0;
		} else if ( ui_serverFilterType.integer < 0 ) {
			ui_serverFilterType.integer = numServerFilters - 1;
		}
		UI_BuildServerDisplayList( qtrue );
		return qtrue;
	}
//#endif	// #ifdef MISSIONPACK
	return qfalse;
}

/* ---- UI_OwnerDrawHandleKey  0x40009C90 ---- */
static qboolean UI_OwnerDrawHandleKey( int ownerDraw, int flags, float *special, int key ) {
	switch ( ownerDraw ) {
	case UI_HANDICAP:
		return UI_Handicap_HandleKey( flags, special, key );
		break;
	case UI_GAMETYPE:
		return UI_GameType_HandleKey( flags, special, key, qtrue );
		break;
	case UI_NETGAMETYPE:
		return UI_NetGameType_HandleKey( flags, special, key );
		break;
	case UI_JOINGAMETYPE:
		return UI_JoinGameType_HandleKey( flags, special, key );
		break;
	case UI_NETSOURCE:
		UI_NetSource_HandleKey( flags, special, key );
		break;
	case UI_NETFILTER:
		UI_NetFilter_HandleKey( flags, special, key );
		break;
	default:
		break;
	}

	return qfalse;
}


/* ---- UI_GetValue  0x40009DA0 ---- */
static float UI_GetValue( int ownerDraw, int type ) {
	return 0;
}

/*
=================
UI_ServersQsortCompare  0x40009DB0
=================
*/
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 ) {
//#ifdef MISSIONPACK
	return trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey, uiInfo.serverStatus.sortDir, *(int*)arg1, *(int*)arg2 );
//#else
//	return qfalse;
//#endif	// #ifdef MISSIONPACK
}


/*
=================
UI_ServersSort  0x40009DE0
=================
*/
void UI_ServersSort( int column, qboolean force ) {

	if ( !force ) {
		if ( uiInfo.serverStatus.sortKey == column ) {
			return;
		}
	}

	uiInfo.serverStatus.sortKey = column;
	qsort( &uiInfo.serverStatus.displayServers[0], uiInfo.serverStatus.numDisplayServers, sizeof( int ), UI_ServersQsortCompare );
}



/*
===============
UI_LoadMods  0x40009E10
===============
*/
static void UI_LoadMods( void ) {
	int numdirs;
	char dirlist[2048];
	char    *dirptr;
	char  *descptr;
	int i;
	int dirlen;

	uiInfo.modCount = 0;
	numdirs = trap_FS_GetFileList( "$modlist", "", dirlist, sizeof( dirlist ) );
	dirptr  = dirlist;
	for ( i = 0; i < numdirs; i++ ) {
		dirlen = strlen( dirptr ) + 1;
		descptr = dirptr + dirlen;
		uiInfo.modList[uiInfo.modCount].modName = String_Alloc( dirptr );
		uiInfo.modList[uiInfo.modCount].modDescr = String_Alloc( descptr );
		dirptr += dirlen + strlen( descptr ) + 1;
		uiInfo.modCount++;
		if ( uiInfo.modCount >= MAX_MODS ) {
			break;
		}
	}

}


/*
===============
UI_LoadMovies  0x40009EF0
===============
*/
static void UI_LoadMovies( void ) {
	char movielist[4096];
	char    *moviename;
	int i, len;

	uiInfo.movieCount = trap_FS_GetFileList( "video", "roq", movielist, 4096 );

	if ( uiInfo.movieCount ) {
		if ( uiInfo.movieCount > MAX_MOVIES ) {
			uiInfo.movieCount = MAX_MOVIES;
		}
		moviename = movielist;
		for ( i = 0; i < uiInfo.movieCount; i++ ) {
			len = strlen( moviename );
			if ( !Q_stricmp( ".roq", moviename +  len - 4 ) ) {
				moviename[len - 4] = '\0';
			}
			Q_strupr( moviename );
			uiInfo.movieList[i] = String_Alloc( moviename );
			moviename += len + 1;
		}
	}

}



/*
===============
UI_LoadDemos  0x40009FC0
===============
*/
static void UI_LoadDemos( void ) {
	char demolist[4096];
	char demoExt[32];
	char    *demoname;
	int i, len;

	Com_sprintf( demoExt, sizeof( demoExt ), "dm_%d", (int)trap_Cvar_VariableValue( "protocol" ) );

	uiInfo.demoCount = trap_FS_GetFileList( "demos", demoExt, demolist, 4096 );

	Com_sprintf( demoExt, sizeof( demoExt ), ".dm_%d", (int)trap_Cvar_VariableValue( "protocol" ) );

	if ( uiInfo.demoCount ) {
		if ( uiInfo.demoCount > MAX_DEMOS ) {
			uiInfo.demoCount = MAX_DEMOS;
		}
		demoname = demolist;
		for ( i = 0; i < uiInfo.demoCount; i++ ) {
			len = strlen( demoname );
			if ( !Q_stricmp( demoname +  len - strlen( demoExt ), demoExt ) ) {
				demoname[len - strlen( demoExt )] = '\0';
			}
			Q_strupr( demoname );
			uiInfo.demoList[i] = String_Alloc( demoname );
			demoname += len + 1;
		}
	}

}


/*
 * 0x4000A100: one `retn`, between UI_LoadDemos and WM_setItemPic -- where
 * RTCW keeps UI_SetNextMap / UI_StartSkirmish / WM_getWeaponIndex /
 * WM_getWeaponAnim.  Unnamed in the Mac symbols.
 */
void nullsub_5( void ) {
}

/* ---- WM_setItemPic  0x4000A110 -- imageTrack 2 ---- */
void WM_setItemPic( char *name, const char *shader ) {
	menuDef_t *menu = Menu_GetFocused();
	itemDef_t *item;

	item = Menu_FindItemByName( menu, name );
	if ( item ) {
		item->window.background = DC->registerShaderNoMip( shader, 2 );
	}
}

/* ---- WM_setVisibility  0x4000A140 ---- */
void WM_setVisibility( char *name, qboolean show ) {
	menuDef_t *menu = Menu_GetFocused();
	itemDef_t *item;

	item = Menu_FindItemByName( menu, name );
	if ( item ) {
		if ( show ) {
			item->window.flags |= WINDOW_VISIBLE;
		} else {
			item->window.flags &= ~WINDOW_VISIBLE;
		}
	}
}

/* ---- UI_CheckExecKey  0x4000A170 ---- */
qboolean UI_CheckExecKey( int key ) {
	menuDef_t *menu = Menu_GetFocused();

	if ( g_editingField ) {
		return qtrue;
	}

	if ( key > 256 ) {
		return qfalse;
	}

	if ( !menu ) {
		if ( !trap_Cvar_VariableValue( "cl_bypassMouseInput" ) ) {
			trap_Cvar_Set( "cl_bypassMouseInput", "0" );
		}
		return qfalse;
	}

	if ( menu->onKey[key] ) {
		return qtrue;
	}

	return qfalse;
}

/*
==============
UI_Update  0x4000A1E0 -- RTCW's less ui_glCustom
==============
*/
static void UI_Update( const char *name ) {
	int val = trap_Cvar_VariableValue( name );

	if ( Q_stricmp( name, "ui_SetName" ) == 0 ) {
		trap_Cvar_Set( "name", UI_Cvar_VariableString( "ui_Name" ) );
	} else if ( Q_stricmp( name, "ui_setRate" ) == 0 ) {
		float rate = trap_Cvar_VariableValue( "rate" );
		if ( rate >= 5000 ) {
			trap_Cvar_Set( "cl_maxpackets", "30" );
			trap_Cvar_Set( "cl_packetdup", "1" );
		} else if ( rate >= 4000 ) {
			trap_Cvar_Set( "cl_maxpackets", "15" );
			trap_Cvar_Set( "cl_packetdup", "2" );       // favor less prediction errors when there's packet loss
		} else {
			trap_Cvar_Set( "cl_maxpackets", "15" );
			trap_Cvar_Set( "cl_packetdup", "1" );       // favor lower bandwidth
		}
	} else if ( Q_stricmp( name, "ui_GetName" ) == 0 ) {
		trap_Cvar_Set( "ui_Name", UI_Cvar_VariableString( "name" ) );
	} else if ( Q_stricmp( name, "r_colorbits" ) == 0 ) {
		switch ( val ) {
		case 0:
			trap_Cvar_SetValue( "r_depthbits", 0 );
			trap_Cvar_SetValue( "r_stencilbits", 0 );
			break;
		case 16:
			trap_Cvar_SetValue( "r_depthbits", 16 );
			trap_Cvar_SetValue( "r_stencilbits", 0 );
			break;
		case 32:
			trap_Cvar_SetValue( "r_depthbits", 24 );
			break;
		}
	} else if ( Q_stricmp( name, "r_lodbias" ) == 0 ) {
		switch ( val ) {
		case 0:
			trap_Cvar_SetValue( "r_subdivisions", 4 );
			break;
		case 1:
			trap_Cvar_SetValue( "r_subdivisions", 12 );
			break;
		case 2:
			trap_Cvar_SetValue( "r_subdivisions", 20 );
			break;
		}
	} else if ( Q_stricmp( name, "ui_mousePitch" ) == 0 ) {
		if ( val == 0 ) {
			trap_Cvar_SetValue( "m_pitch", 0.022f );
		} else {
			trap_Cvar_SetValue( "m_pitch", -0.022f );
		}
	}
}

/*
==============
UI_VerifyLanguage  0x4000A3B0

Lets the engine veto ui_language (trap_VerifyLanguageSelection) and flags
ui_languagechanged when it differs from cl_language.
==============
*/
void UI_VerifyLanguage( void ) {
	char cl_language[8];
	char ui_language[8];
	int verified;

	cl_language[0] = '\0';
	Q_strncpyz( cl_language, UI_Cvar_VariableString( "cl_language" ), sizeof( cl_language ) );
	ui_language[0] = '\0';
	Q_strncpyz( ui_language, UI_Cvar_VariableString( "ui_language" ), sizeof( ui_language ) );

	verified = trap_VerifyLanguageSelection( atoi( ui_language ) );
	if ( verified != atoi( ui_language ) ) {
		Q_strncpyz( ui_language, va( "%i", verified ), sizeof( ui_language ) );
		trap_Cvar_Set( "ui_language", ui_language );
	}

	if ( atoi( ui_language ) == atoi( cl_language ) ) {
		trap_Cvar_Set( "ui_languagechanged", "0" );
	} else {
		trap_Cvar_Set( "ui_languagechanged", "1" );
	}
}

/*
==============
UI_AddServerToFavoritesList  0x4000A4A0

The favorites half of RTCW's addFavorite/createFavorite scripts, shared and
reporting through ui_favorite_message.
==============
*/
static void UI_AddServerToFavoritesList( const char *name, const char *addr ) {
	int res;

	if ( !strlen( name ) ) {
		Com_Printf( "%s\n", UI_SafeTranslateString( "EXE_FAVORITENAMEEMPTY" ) );
		trap_Cvar_Set( "ui_favorite_message", "@EXE_FAVORITENAMEEMPTY" );
		return;
	}
	if ( !strlen( addr ) ) {
		Com_Printf( "%s\n", UI_SafeTranslateString( "EXE_FAVORITEADDRESSEMPTY" ) );
		trap_Cvar_Set( "ui_favorite_message", "@EXE_FAVORITEADDRESSEMPTY" );
		return;
	}

	res = trap_LAN_AddServer( AS_FAVORITES, name, addr );
	if ( res == 0 ) {
		// server already in the list
		Com_Printf( "%s\n", UI_SafeTranslateString( "EXE_FAVORITEINLIST" ) );
		trap_Cvar_Set( "ui_favorite_message", "@EXE_FAVORITEINLIST" );
	} else if ( res == -1 ) {
		// list full
		Com_Printf( "%s\n", UI_SafeTranslateString( "EXE_FAVORITELISTFULL" ) );
		trap_Cvar_Set( "ui_favorite_message", "@EXE_FAVORITELISTFULL" );
	} else if ( res == -2 ) {
		Com_Printf( "%s\n", UI_SafeTranslateString( "EXE_BADSERVERADDRESS" ) );
		trap_Cvar_Set( "ui_favorite_message", "@EXE_BADSERVERADDRESS" );
	} else {
		// successfully added
		Com_Printf( va( "%s\n", UI_SafeTranslateString( "EXE_FAVORITEADDED" ) ), addr );
		trap_Cvar_Set( "ui_favorite_message", "@EXE_FAVORITEADDED" );
	}
}

/* ---- UI_RunMenuScript  0x4000A5F0 -- the DC's runScript; the script names are the retail set ---- */
static void UI_RunMenuScript( char **args ) {
	const char *name, *name2;
	char buff[1024];

	if ( String_Parse( args, &name ) ) {

		if ( Q_stricmp( name, "StartServer" ) == 0 ) {
			trap_Cvar_Set( "cg_thirdPerson", "0" );
			trap_Cvar_SetValue( "dedicated", Com_Clamp( 0, 2, ui_dedicated.integer ) );
			trap_Cvar_Set( "g_gametype", uiInfo.gameTypes[ui_netGameType.integer].gameType );

			trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName ) );

		} else if ( Q_stricmp( name, "resetDefaults" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_NOW, "cvar_restart\n" );            // NERVE - SMF - changed order
			trap_Cmd_ExecuteText( EXEC_NOW, "exec default_mp.cfg\n" );
			trap_Cmd_ExecuteText( EXEC_NOW, "exec language.cfg\n" );       // NERVE - SMF
			trap_Cmd_ExecuteText( EXEC_NOW, "setRecommended\n" );     // NERVE - SMF
			Controls_SetDefaults();
			trap_Cvar_Set( "com_introPlayed", "1" );
			trap_Cvar_Set( "com_recommendedSet", "1" );                   // NERVE - SMF
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} else if ( Q_stricmp( name, "getCDKey" ) == 0 ) {
			char out[17];
			char checksum[1024];
			trap_GetCDKey( buff, 17, checksum, 5 );
			trap_Cvar_Set( "cdkey1", "" );
			trap_Cvar_Set( "cdkey2", "" );
			trap_Cvar_Set( "cdkey3", "" );
			trap_Cvar_Set( "cdkey4", "" );
			trap_Cvar_Set( "cdkey5", "" );
			if ( strlen( buff ) == CDKEY_LEN ) {
				Q_strncpyz( out, buff, 5 );
				trap_Cvar_Set( "cdkey1", out );
				Q_strncpyz( out, buff + 4, 5 );
				trap_Cvar_Set( "cdkey2", out );
				Q_strncpyz( out, buff + 8, 5 );
				trap_Cvar_Set( "cdkey3", out );
				Q_strncpyz( out, buff + 12, 5 );
				trap_Cvar_Set( "cdkey4", out );
			}
			if ( strlen( checksum ) == 4 ) {
				Q_strncpyz( out, checksum, 5 );
				trap_Cvar_Set( "cdkey5", out );
			}

		} else if ( Q_stricmp( name, "verifyCDKey" ) == 0 ) {
			char checksum[1024];
			buff[0] = '\0';
			Q_strcat( buff, 1024, UI_Cvar_VariableString( "cdkey1" ) );
			Q_strcat( buff, 1024, UI_Cvar_VariableString( "cdkey2" ) );
			Q_strcat( buff, 1024, UI_Cvar_VariableString( "cdkey3" ) );
			Q_strcat( buff, 1024, UI_Cvar_VariableString( "cdkey4" ) );
			checksum[0] = '\0';
			Q_strcat( checksum, 1024, UI_Cvar_VariableString( "cdkey5" ) );
			if ( trap_VerifyCDKey( buff, checksum ) ) {
				trap_Cvar_Set( "ui_cdkeyvalid", UI_SafeTranslateString( "EXE_CDKEYVALID" ) );
				trap_SetCDKey( buff, checksum );
			} else {
				trap_Cvar_Set( "ui_cdkeyvalid", UI_SafeTranslateString( "EXE_CDKEYINVALID" ) );
			}
		} else if ( Q_stricmp( name, "loadArenas" ) == 0 ) {
			UI_LoadArenas();
			UI_SelectCurrentGameType();
			UI_MapCountByGameType();
			Menu_SetFeederSelection( NULL, FEEDER_ALLMAPS, 0, NULL );
			UI_SelectCurrentMap();
		} else if ( Q_stricmp( name, "saveControls" ) == 0 ) {
			Controls_SetConfig( qtrue );
		} else if ( Q_stricmp( name, "loadControls" ) == 0 ) {
			Controls_GetConfig();
		} else if ( Q_stricmp( name, "clearError" ) == 0 ) {
			trap_Cvar_Set( "com_errorMessage", "" );
		} else if ( Q_stricmp( name, "loadGameInfo" ) == 0 ) {
			UI_GetGameTypesList();
		} else if ( Q_stricmp( name, "resetScores" ) == 0 ) {
		} else if ( Q_stricmp( name, "RefreshServers" ) == 0 ) {
			UI_StartServerRefresh( qtrue );
			UI_BuildServerDisplayList( qtrue );
		} else if ( Q_stricmp( name, "RefreshFilter" ) == 0 ) {
			UI_StartServerRefresh( qfalse );
			UI_BuildServerDisplayList( qtrue );
		} else if ( Q_stricmp( name, "RunSPDemo" ) == 0 ) {
			if ( uiInfo.demoAvailable ) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "demo %s_%s\n", uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gameType ) );
			}
		} else if ( Q_stricmp( name, "LoadDemos" ) == 0 ) {
			UI_LoadDemos();
		} else if ( Q_stricmp( name, "LoadMovies" ) == 0 ) {
			UI_LoadMovies();
		} else if ( Q_stricmp( name, "LoadMods" ) == 0 ) {
			UI_LoadMods();
		} else if ( Q_stricmp( name, "playMovie" ) == 0 ) {
			if ( uiInfo.previewMovie >= 0 ) {
				trap_CIN_StopCinematic( uiInfo.previewMovie );
			}
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "cinematic %s.roq 2\n", uiInfo.movieList[uiInfo.movieIndex] ) );
		} else if ( Q_stricmp( name, "RunMod" ) == 0 ) {
			trap_Cvar_Set( "fs_game", uiInfo.modList[uiInfo.modIndex].modName );
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
		} else if ( Q_stricmp( name, "RunDemo" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "demo %s\n", uiInfo.demoList[uiInfo.demoIndex] ) );
		} else if ( Q_stricmp( name, "Quake3" ) == 0 ) {
			trap_Cvar_Set( "fs_game", "" );
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
		} else if ( Q_stricmp( name, "closeJoin" ) == 0 ) {
			if ( uiInfo.serverStatus.refreshActive ) {
				UI_StopServerRefresh();
				uiInfo.serverStatus.nextDisplayRefresh = 0;
				uiInfo.nextServerStatusRefresh = 0;
				uiInfo.nextFindPlayerRefresh = 0;
				UI_BuildServerDisplayList( qtrue );
			} else {
				Menus_CloseByName( "joinserver" );
				Menus_OpenByName( "main" );
			}
		} else if ( Q_stricmp( name, "StopRefresh" ) == 0 ) {
			UI_StopServerRefresh();
			uiInfo.serverStatus.nextDisplayRefresh = 0;
			uiInfo.nextServerStatusRefresh = 0;
			uiInfo.nextFindPlayerRefresh = 0;
		} else if ( Q_stricmp( name, "UpdateFilter" ) == 0 ) {
			if ( ui_netSource.integer == AS_LOCAL ) {
				UI_StartServerRefresh( qtrue );
			}
			UI_BuildServerDisplayList( qtrue );
			UI_FeederSelection( FEEDER_SERVERS, 0 );
		} else if ( Q_stricmp( name, "ServerStatus" ) == 0 ) {
			UI_UpdateDisplayServers();
			if ( uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers ) {
				trap_LAN_GetServerAddressString( ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], uiInfo.serverStatusAddress, sizeof( uiInfo.serverStatusAddress ) );
				UI_BuildServerStatus( qtrue );
			}
		} else if ( Q_stricmp( name, "FoundPlayerServerStatus" ) == 0 ) {
			Q_strncpyz( uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof( uiInfo.serverStatusAddress ) );
			UI_BuildServerStatus( qtrue );
			Menu_SetFeederSelection( NULL, FEEDER_FINDPLAYER, 0, NULL );
		} else if ( Q_stricmp( name, "FindPlayer" ) == 0 ) {
			UI_BuildFindPlayerList( qtrue );
			// clear the displayed server status info
			uiInfo.serverStatusInfo.numLines = 0;
			Menu_SetFeederSelection( NULL, FEEDER_FINDPLAYER, 0, NULL );
		} else if ( Q_stricmp( name, "JoinServer" ) == 0 ) {
			UI_UpdateDisplayServers();
			trap_Cvar_Set( "cg_thirdPerson", "0" );
			if ( uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers ) {
				trap_LAN_GetServerAddressString( ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, 1024 );
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
			}
		} else if ( Q_stricmp( name, "FoundPlayerJoinServer" ) == 0 ) {
			if ( uiInfo.currentFoundPlayerServer >= 0 && uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers ) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer] ) );
			}
		} else if ( Q_stricmp( name, "Quit" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_NOW, "quit" );
		} else if ( Q_stricmp( name, "Controls" ) == 0 ) {
			trap_Cvar_Set( "cl_paused", "1" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_OpenByName( "setup_menu2" );
		} else if ( Q_stricmp( name, "Leave" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_OpenByName( "main" );
		} else if ( Q_stricmp( name, "ServerSort" ) == 0 ) {
			int sortColumn;
			if ( Int_Parse( args, &sortColumn ) ) {
				// if same column we're already sorting on then flip the direction
				if ( sortColumn == uiInfo.serverStatus.sortKey ) {
					uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
				}
				// make sure we sort again
				UI_ServersSort( sortColumn, qtrue );
			}
		} else if ( Q_stricmp( name, "nextSkirmish" ) == 0 ) {
		} else if ( Q_stricmp( name, "SkirmishStart" ) == 0 ) {
		} else if ( Q_stricmp( name, "closeingame" ) == 0 ) {
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
			trap_Key_ClearStates();
			trap_Cvar_Set( "cl_paused", "0" );
			Menus_CloseAll();
		} else if ( Q_stricmp( name, "voteTypeMap" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote typemap %s %s\n", uiInfo.gameTypes[ui_netGameType.integer].gameType, uiInfo.mapList[ui_currentNetMap.integer].mapLoadName ) );
		} else if ( Q_stricmp( name, "voteMap" ) == 0 ) {
			if ( ui_currentNetMap.integer >= 0 && ui_currentNetMap.integer < uiInfo.mapCount ) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote map %s\n",uiInfo.mapList[ui_currentNetMap.integer].mapLoadName ) );
			}
		} else if ( Q_stricmp( name, "voteKick" ) == 0 ) {
			if ( uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount ) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote kick \"%s\"\n",uiInfo.playerNames[uiInfo.playerIndex] ) );
			}
		} else if ( Q_stricmp( name, "voteGame" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote g_gametype %s\n",uiInfo.gameTypes[ui_netGameType.integer].gameType ) );
		} else if ( Q_stricmp( name, "addFavorite" ) == 0 ) {
			if ( ui_netSource.integer != AS_FAVORITES ) {
				char name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];

				trap_LAN_GetServerInfo( ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS );
				name[0] = addr[0] = '\0';
				Q_strncpyz( name,    Info_ValueForKey( buff, "hostname" ), MAX_NAME_LENGTH );
				Q_strncpyz( addr,    Info_ValueForKey( buff, "addr" ), MAX_NAME_LENGTH );
				UI_AddServerToFavoritesList( name, addr );
			}
		} else if ( Q_stricmp( name, "deleteFavorite" ) == 0 ) {
			if ( ui_netSource.integer == AS_FAVORITES ) {
				char addr[MAX_NAME_LENGTH];
				trap_LAN_GetServerInfo( ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS );
				addr[0] = '\0';
				Q_strncpyz( addr,    Info_ValueForKey( buff, "addr" ), MAX_NAME_LENGTH );
				if ( strlen( addr ) > 0 ) {
					trap_LAN_RemoveServer( AS_FAVORITES, addr );
				}
			}
		} else if ( Q_stricmp( name, "createFavorite" ) == 0 ) {
			if ( ui_netSource.integer == AS_FAVORITES ) {
				char name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];

				name[0] = addr[0] = '\0';
				Q_strncpyz( name,    UI_Cvar_VariableString( "ui_favoriteName" ), MAX_NAME_LENGTH );
				Q_strncpyz( addr,    UI_Cvar_VariableString( "ui_favoriteAddress" ), MAX_NAME_LENGTH );
				UI_AddServerToFavoritesList( name, addr );
			}
		} else if ( Q_stricmp( name, "orders" ) == 0 ) {
			const char *orders;
			if ( String_Parse( args, &orders ) ) {
				int selectedPlayer = trap_Cvar_VariableValue( "cg_selectedPlayer" );
				if ( selectedPlayer < uiInfo.myTeamCount ) {
					strcpy( buff, orders );
					trap_Cmd_ExecuteText( EXEC_APPEND, va( buff, uiInfo.teamClientNums[selectedPlayer] ) );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				} else {
					int i;
					for ( i = 0; i < uiInfo.myTeamCount; i++ ) {
						if ( Q_stricmp( UI_Cvar_VariableString( "name" ), uiInfo.teamNames[i] ) == 0 ) {
							continue;
						}
						strcpy( buff, orders );
						trap_Cmd_ExecuteText( EXEC_APPEND, va( buff, uiInfo.teamNames[i] ) );
						trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
					}
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
			}
		} else if ( Q_stricmp( name, "voiceOrdersTeam" ) == 0 ) {
			const char *orders;
			if ( String_Parse( args, &orders ) ) {
				int selectedPlayer = trap_Cvar_VariableValue( "cg_selectedPlayer" );
				if ( selectedPlayer == uiInfo.myTeamCount ) {
					trap_Cmd_ExecuteText( EXEC_APPEND, orders );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
			}
		} else if ( Q_stricmp( name, "voiceOrders" ) == 0 ) {
			const char *orders;
			if ( String_Parse( args, &orders ) ) {
				int selectedPlayer = trap_Cvar_VariableValue( "cg_selectedPlayer" );
				if ( selectedPlayer < uiInfo.myTeamCount ) {
					strcpy( buff, orders );
					trap_Cmd_ExecuteText( EXEC_APPEND, va( buff, uiInfo.teamClientNums[selectedPlayer] ) );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
			}
		} else if ( Q_stricmp( name, "update" ) == 0 ) {
			if ( String_Parse( args, &name2 ) ) {
				UI_Update( name2 );
			}
			// NERVE - SMF
		} else if ( Q_stricmp( name, "showSpecScores" ) == 0 ) {
			if ( atoi( UI_Cvar_VariableString( "ui_isSpectator" ) ) ) {
				trap_Cmd_ExecuteText( EXEC_APPEND, "+scores\n" );
			}
		} else if ( Q_stricmp( name, "startSingleplayer" ) == 0 ) {
			trap_Cmd_ExecuteText( EXEC_APPEND, "startSingleplayer\n" );
		} else if ( Q_stricmp( name, "getLanguage" ) == 0 ) {
			char language[8];

			language[0] = '\0';
			Q_strncpyz( language, UI_Cvar_VariableString( "cl_language" ), sizeof( language ) );
			trap_Cvar_Set( "ui_language", language );
			UI_VerifyLanguage();
		} else if ( Q_stricmp( name, "verifyLanguage" ) == 0 ) {
			UI_VerifyLanguage();
		} else if ( Q_stricmp( name, "updateLanguage" ) == 0 ) {
			char language[8];

			language[0] = '\0';
			Q_strncpyz( language, UI_Cvar_VariableString( "ui_language" ), sizeof( language ) );
			trap_Cvar_Set( "cl_language", language );
			UI_VerifyLanguage();
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} else {
			Com_Printf( "unknown UI script %s\n", name );
		}
	}
}

/* ---- UI_GetTeamColor  0x4000B500 ---- */
static void UI_GetTeamColor( vec4_t *color ) {
}

/*
==================
UI_SelectCurrentGameType  0x4000B510

Points ui_netGameType at the g_gametype the server is running.
==================
*/
static void UI_SelectCurrentGameType( void ) {
	char gametype[MAX_STRING_CHARS];
	int i;

	trap_Cvar_VariableStringBuffer( "g_gametype", gametype, sizeof( gametype ) );
	for ( i = 0; i < uiInfo.numGameTypes; i++ ) {
		if ( !Q_stricmp( uiInfo.gameTypes[i].gameType, gametype ) ) {
			trap_Cvar_Set( "ui_netGameType", va( "%d", i ) );
			trap_Cvar_Set( "ui_netGameTypeName", uiInfo.gameTypes[i].gameType );
			return;
		}
	}
}

/*
==================
UI_MapCountByGameType  0x4000B5C0 -- RTCW's, always ui_netGameType
==================
*/
static int UI_MapCountByGameType( void ) {
	int i, c, game;
	c = 0;
	game = ui_netGameType.integer;

	for ( i = 0; i < uiInfo.mapCount; i++ ) {
		uiInfo.mapList[i].active = qfalse;
		if ( uiInfo.mapList[i].typeBits & ( 1 << game ) ) {
			c++;
			uiInfo.mapList[i].active = qtrue;
		}
	}
	return c;
}

/*
==================
UI_SelectCurrentMap  0x4000B610

In game, moves the map feeder onto the map the server is running.
==================
*/
static void UI_SelectCurrentMap( void ) {
	uiClientState_t cs;
	char info[MAX_INFO_STRING];
	char mapname[64];
	int i, c;

	trap_GetClientState( &cs );
	if ( cs.connState != CA_ACTIVE ) {
		return;
	}

	info[0] = '\0';
	if ( !trap_GetConfigString( CS_SERVERINFO, info, sizeof( info ) ) ) {
		return;
	}
	strcpy( mapname, Info_ValueForKey( info, "mapname" ) );

	c = 0;
	for ( i = 0; i < uiInfo.mapCount; i++ ) {
		if ( uiInfo.mapList[i].active ) {
			if ( !Q_stricmp( uiInfo.mapList[i].mapName, mapname ) ) {
				Menu_SetFeederSelection( NULL, FEEDER_ALLMAPS, c, NULL );
				return;
			}
			c++;
		}
	}
}

/*
==================
UI_InsertServerIntoDisplayList  0x4000B6F0
==================
*/
static void UI_InsertServerIntoDisplayList( int num, int position ) {
	int i;

	if ( position < 0 || position > uiInfo.serverStatus.numDisplayServers ) {
		return;
	}
	if ( position <= uiInfo.serverStatus.currentServer ) {
		uiInfo.serverStatus.currentServer++;
	}
	uiInfo.serverStatus.numDisplayServers++;
	for ( i = uiInfo.serverStatus.numDisplayServers; i > position; i-- ) {
		uiInfo.serverStatus.displayServers[i] = uiInfo.serverStatus.displayServers[i - 1];
	}
	uiInfo.serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList  0x4000B730
==================
*/
static void UI_RemoveServerFromDisplayList( int num ) {
	int i, j;

	for ( i = 0; i < uiInfo.serverStatus.numDisplayServers; i++ ) {
		if ( uiInfo.serverStatus.displayServers[i] == num ) {
			uiInfo.serverStatus.numDisplayServers--;
			for ( j = i; j < uiInfo.serverStatus.numDisplayServers; j++ ) {
				uiInfo.serverStatus.displayServers[j] = uiInfo.serverStatus.displayServers[j + 1];
			}
			return;
		}
	}
}

/*
==================
UI_BinaryServerInsertion  0x4000B780
==================
*/
static void UI_BinaryServerInsertion( int num ) {
	int mid, offset, res, len;

	// use binary search to insert server
	len = uiInfo.serverStatus.numDisplayServers;
	mid = len;
	offset = 0;
	res = 0;
	while ( mid > 0 ) {
		mid = len >> 1;
		//
		res = trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey,
									   uiInfo.serverStatus.sortDir, num, uiInfo.serverStatus.displayServers[offset + mid] );
		// if equal
		if ( res == 0 ) {
			UI_InsertServerIntoDisplayList( num, offset + mid );
			return;
		}
		// if larger
		else if ( res == 1 ) {
			offset += mid;
			len -= mid;
		}
		// if smaller
		else {
			len -= mid;
		}
	}
	if ( res == 1 ) {
		offset++;
	}
	UI_InsertServerIntoDisplayList( num, offset );
}

/*
==================
UI_BuildServerDisplayList  0x4000B800

RTCW's with CoD's filters: the 000.000.000.000 placeholder, show empty /
full / password / no-password, the join gametype by name, the mod filter.
trap_ClearDisplayedServers is ui_public.h's name for LAN_MarkServerVisible,
trap_LAN_ServerIsDirty its name for LAN_ServerIsVisible.
==================
*/
/* CoD 1.5 UI_ClearDisplayedServers; use the unused numServers field
 * for the remembered count to preserve this module's existing layout. */
static void UI_ClearDisplayedServers( void ) {
	uiInfo.serverStatus.numDisplayServers = 0;
	uiInfo.serverStatus.numPlayersOnServers = 0;
	uiInfo.serverStatus.numServers = trap_LAN_GetServerCount( ui_netSource.integer );
}

/* CoD 1.5 UI_UpdateDisplayServers (ui_mp.dll.c). */
static void UI_UpdateDisplayServers( void ) {
	int count = trap_LAN_GetServerCount( ui_netSource.integer );
	if ( uiInfo.serverStatus.numServers != count ) {
		uiInfo.serverStatus.numServers = count;
		if ( uiInfo.serverStatus.numDisplayServers ) {
			uiInfo.serverStatus.currentServer = -1;
			UI_BuildServerDisplayList( qtrue );
		}
	}
}

static void UI_BuildServerDisplayList( qboolean force ) {
	int i, count, clients, maxClients, ping, len;
	char info[MAX_STRING_CHARS];
	//qboolean startRefresh = qtrue; // TTimo: unused
	static int numinvisible;                        /* 0x4005B18C */

	if ( !( force || uiInfo.uiDC.realTime > uiInfo.serverStatus.nextDisplayRefresh ) ) {
		return;
	}
	// if we shouldn't reset
	if ( force == 2 ) {
		force = 0;
	}

	// do motd updates here too
	trap_Cvar_VariableStringBuffer( "cl_motdString", uiInfo.serverStatus.motd, sizeof( uiInfo.serverStatus.motd ) );
	len = strlen( uiInfo.serverStatus.motd );
	if ( len == 0 ) {
		strcpy( uiInfo.serverStatus.motd, va( "%s - %s", UI_SafeTranslateString( "EXE_COD_MULTIPLAYER" ), "1.1x" ) );
		len = strlen( uiInfo.serverStatus.motd );
	}
	if ( len != uiInfo.serverStatus.motdLen ) {
		uiInfo.serverStatus.motdLen = len;
		uiInfo.serverStatus.motdWidth = -1;
	}

	if ( force ) {
		numinvisible = 0;
		// clear number of displayed servers
		UI_ClearDisplayedServers();
		// set list box index to zero
		if ( uiInfo.serverStatus.currentServer >= 0 ) {
			Menu_SetFeederSelection( NULL, FEEDER_SERVERS, 0, NULL );
		}
		// mark all servers as visible so we store ping updates for them
		trap_ClearDisplayedServers( ui_netSource.integer, -1, qtrue );
	}

	// get the server count (comes from the master)
	count = trap_LAN_GetServerCount( ui_netSource.integer );
	if ( count == -1 || ( ui_netSource.integer == AS_LOCAL && count == 0 ) ) {
		// still waiting on a response from the master
		UI_ClearDisplayedServers();
		uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 500;
		return;
	}

	for ( i = 0; i < count; i++ ) {
		// if we already got info for this server
		if ( !trap_LAN_ServerIsDirty( ui_netSource.integer, i ) ) {
			continue;
		}
		// get the ping for this server
		ping = trap_LAN_GetServerPing( ui_netSource.integer, i );
		if ( ping > 0 || ui_netSource.integer == AS_FAVORITES ) {

			trap_LAN_GetServerInfo( ui_netSource.integer, i, info, MAX_STRING_CHARS );

			clients = atoi( Info_ValueForKey( info, "clients" ) );
			uiInfo.serverStatus.numPlayersOnServers += clients;

			if ( !Q_stricmpn( Info_ValueForKey( info, "addr" ), "000.000.000.000", 15 ) ) {
				trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
				continue;
			}

			if ( ui_browserShowEmpty.integer == 0 ) {
				if ( clients == 0 ) {
					trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
					continue;
				}
			}

			if ( ui_browserShowFull.integer == 0 ) {
				maxClients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
				if ( clients == maxClients ) {
					trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
					continue;
				}
			}

			if ( ui_browserShowPassword.integer == 0 ) {
				if ( atoi( Info_ValueForKey( info, "pswrd" ) ) ) {
					trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
					continue;
				}
			}

			if ( ui_browserShowNoPassword.integer == 0 ) {
				if ( !atoi( Info_ValueForKey( info, "pswrd" ) ) ) {
					trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
					continue;
				}
			}

			if ( uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum[0] ) {
				if ( Q_stricmp( uiInfo.joinGameTypes[ui_joinGameType.integer].gameType, Info_ValueForKey( info, "gametype" ) ) != 0 ) {
					trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
					continue;
				}
			}

			if ( ui_serverFilterType.integer > 0 ) {
				if ( Q_stricmp( Info_ValueForKey( info, "game" ), serverFilters[ui_serverFilterType.integer].basedir ) != 0 ) {
					trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
					continue;
				}
			}
			// make sure we never add a favorite server twice
			if ( ui_netSource.integer == AS_FAVORITES ) {
				UI_RemoveServerFromDisplayList( i );
			}
			// insert the server into the list
			UI_BinaryServerInsertion( i );
			// done with this server
			if ( ping > 0 ) {
				trap_ClearDisplayedServers( ui_netSource.integer, i, qfalse );
				numinvisible++;
			}
		}
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime;
}

/* CoD adds the third member: a 1 turns the value into @EXE_YES / @EXE_NO. */
typedef struct
{
	char *name, *altName;
	int isBoolean;
} serverStatusCvar_t;

/* 0x40036EC8 */
serverStatusCvar_t serverStatusCvars[] = {
	{"sv_hostname", "@EXE_SV_INFO_SERVERNAME", 0},
	{"address", "@EXE_SV_INFO_ADDRESS", 0},
	{"pswrd", "@EXE_SV_INFO_PASSWORD", 1},
	{"gamename", "@EXE_SV_INFO_GAMENAME", 0},
	{"g_gametype", "@EXE_SV_INFO_GAMETYPE", 0},
	{"sv_pure", "@EXE_SV_INFO_PURE", 1},
	{"mapname", "@EXE_SV_INFO_MAP", 0},
	{"shortversion", "@EXE_SV_INFO_VERSION", 0},
	{"protocol", "@EXE_SV_INFO_PROTOCOL", 0},
	{"sv_maxping", "@EXE_SV_INFO_MAXPING", 0},
	{"sv_minping", "@EXE_SV_INFO_MINPING", 0},
	{"sv_maxrate", "@EXE_SV_INFO_MAXRATE", 0},
	{"sv_floodprotect", "@EXE_SV_INFO_FLOODPROTECT", 0},
	{"sv_allowanonymous", "@EXE_SV_INFO_ALLOWANON", 0},
	{"sv_maxclients", "@EXE_SV_INFO_MAXCLIENTS", 0},
	{"sv_privateclients", "@EXE_SV_INFO_PRIVATECLIENTS", 0},
	{NULL, NULL, 0}
};

/*
==================
UI_SortServerStatusInfo  0x4000BBB0
==================
*/
static void UI_SortServerStatusInfo( serverStatusInfo_t *info ) {
	int i, j, index;
	char *tmp1, *tmp2;

	// FIXME: if "gamename" == "baseq3" or "missionpack" then
	// replace the gametype number by FFA, CTF etc.
	//
	index = 0;
	for ( i = 0; serverStatusCvars[i].name; i++ ) {
		for ( j = 0; j < info->numLines; j++ ) {
			if ( !info->lines[j][1] || info->lines[j][1][0] ) {
				continue;
			}
			if ( !Q_stricmp( serverStatusCvars[i].name, info->lines[j][0] ) ) {
				// swap lines
				tmp1 = info->lines[index][0];
				tmp2 = info->lines[index][3];
				info->lines[index][0] = info->lines[j][0];
				info->lines[index][3] = info->lines[j][3];
				info->lines[j][0] = tmp1;
				info->lines[j][3] = tmp2;
				//
				if ( strlen( serverStatusCvars[i].altName ) ) {
					info->lines[index][0] = serverStatusCvars[i].altName;
				}
				if ( serverStatusCvars[i].isBoolean == 1 ) {
					if ( atoi( info->lines[index][3] ) ) {
						info->lines[index][3] = "@EXE_YES";
					} else {
						info->lines[index][3] = "@EXE_NO";
					}
				}
				index++;
			}
		}
	}
}

/*
==================
UI_GetServerStatusInfo  0x4000BCC0 -- RTCW's, less the URL cvars
==================
*/
static int UI_GetServerStatusInfo( const char *serverAddress, serverStatusInfo_t *info ) {
	char *p, *score, *ping, *name;
	int i, len;

	if ( !info ) {
		trap_LAN_ServerStatus( serverAddress, NULL, 0 );
		return qfalse;
	}
	memset( info, 0, sizeof( *info ) );
	if ( trap_LAN_ServerStatus( serverAddress, info->text, sizeof( info->text ) ) ) {

		Q_strncpyz( info->address, serverAddress, sizeof( info->address ) );
		p = info->text;
		info->numLines = 0;
		info->lines[info->numLines][0] = "address";
		info->lines[info->numLines][1] = "";
		info->lines[info->numLines][2] = "";
		info->lines[info->numLines][3] = info->address;
		info->numLines++;
		// get the cvars
		while ( p && *p ) {
			p = strchr( p, '\\' );
			if ( !p ) {
				break;
			}
			*p++ = '\0';
			if ( *p == '\\' ) {
				break;
			}
			info->lines[info->numLines][0] = p;
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			p = strchr( p, '\\' );
			if ( !p ) {
				break;
			}
			*p++ = '\0';
			info->lines[info->numLines][3] = p;

			info->numLines++;
			if ( info->numLines >= MAX_SERVERSTATUS_LINES ) {
				break;
			}
		}
		// get the player list
		if ( info->numLines < MAX_SERVERSTATUS_LINES - 3 ) {
			// empty line
			info->lines[info->numLines][0] = "";
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			info->lines[info->numLines][3] = "";
			info->numLines++;
			// header
			info->lines[info->numLines][0] = "@EXE_SV_INFO_NUM";
			info->lines[info->numLines][1] = "@EXE_SV_INFO_SCORE";
			info->lines[info->numLines][2] = "@EXE_SV_INFO_PING";
			info->lines[info->numLines][3] = "@EXE_SV_INFO_NAME";
			info->numLines++;
			// parse players
			i = 0;
			len = 0;
			while ( p && *p ) {
				if ( *p == '\\' ) {
					*p++ = '\0';
				}
				if ( !p ) {
					break;
				}
				score = p;
				p = strchr( p, ' ' );
				if ( !p ) {
					break;
				}
				*p++ = '\0';
				ping = p;
				p = strchr( p, ' ' );
				if ( !p ) {
					break;
				}
				*p++ = '\0';
				name = p;
				Com_sprintf( &info->pings[len], sizeof( info->pings ) - len, "%d", i );
				info->lines[info->numLines][0] = &info->pings[len];
				len += strlen( &info->pings[len] ) + 1;
				info->lines[info->numLines][1] = score;
				info->lines[info->numLines][2] = ping;
				info->lines[info->numLines][3] = name;
				info->numLines++;
				if ( info->numLines >= MAX_SERVERSTATUS_LINES ) {
					break;
				}
				p = strchr( p, '\\' );
				if ( !p ) {
					break;
				}
				*p++ = '\0';
				//
				i++;
			}
		}
		UI_SortServerStatusInfo( info );
		return qtrue;
	}
	return qfalse;
}

/*
==================
stristr  0x4000BFD0
==================
*/
static char *stristr( char *str, char *charset ) {
	int i;

	while ( *str ) {
		for ( i = 0; charset[i] && str[i]; i++ ) {
			if ( toupper( charset[i] ) != toupper( str[i] ) ) {
				break;
			}
		}
		if ( !charset[i] ) {
			return str;
		}
		str++;
	}
	return NULL;
}

/*
==================
UI_BuildFindPlayerList  0x4000C040
==================
*/
static void UI_BuildFindPlayerList( qboolean force ) {
	static int numFound, numTimeOuts;               /* 0x4005B188, 0x4005B1A0 */
	int i, j, resend;
	serverStatusInfo_t info;
	char name[MAX_NAME_LENGTH + 2];
	char infoString[MAX_STRING_CHARS];

	if ( !force ) {
		if ( !uiInfo.nextFindPlayerRefresh || uiInfo.nextFindPlayerRefresh > uiInfo.uiDC.realTime ) {
			return;
		}
	} else {
		memset( &uiInfo.pendingServerStatus, 0, sizeof( uiInfo.pendingServerStatus ) );
		uiInfo.numFoundPlayerServers = 0;
		uiInfo.currentFoundPlayerServer = 0;
		trap_Cvar_VariableStringBuffer( "ui_findPlayer", uiInfo.findPlayerName, sizeof( uiInfo.findPlayerName ) );
		Q_CleanStr( uiInfo.findPlayerName );
		// should have a string of some length
		if ( !strlen( uiInfo.findPlayerName ) ) {
			uiInfo.nextFindPlayerRefresh = 0;
			return;
		}
		// set resend time
		resend = ui_serverStatusTimeOut.integer / 2 - 10;
		if ( resend < 50 ) {
			resend = 50;
		}
		trap_Cvar_Set( "cl_serverStatusResendTime", va( "%d", resend ) );
		// reset all server status requests
		trap_LAN_ServerStatus( NULL, NULL, 0 );
		//
		uiInfo.numFoundPlayerServers = 1;
		Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
					 sizeof( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1] ),
					 "searching %d...", uiInfo.pendingServerStatus.num );
		numFound = 0;
		numTimeOuts++;
	}
	UI_UpdateDisplayServers();
	for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
		// if this pending server is valid
		if ( uiInfo.pendingServerStatus.server[i].valid ) {
			// try to get the server status for this server
			if ( UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, &info ) ) {
				//
				numFound++;
				// parse through the server status lines
				for ( j = 0; j < info.numLines; j++ ) {
					// should have ping info
					if ( !info.lines[j][2] || !info.lines[j][2][0] ) {
						continue;
					}
					// clean string first
					Q_strncpyz( name, info.lines[j][3], sizeof( name ) );
					Q_CleanStr( name );
					// if the player name is a substring
					if ( stristr( name, uiInfo.findPlayerName ) ) {
						// add to found server list if we have space (always leave space for a line with the number found)
						if ( uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS - 1 ) {
							//
							Q_strncpyz( uiInfo.foundPlayerServerAddresses[uiInfo.numFoundPlayerServers - 1],
										uiInfo.pendingServerStatus.server[i].adrstr,
										sizeof( uiInfo.foundPlayerServerAddresses[0] ) );
							Q_strncpyz( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
										uiInfo.pendingServerStatus.server[i].name,
										sizeof( uiInfo.foundPlayerServerNames[0] ) );
							uiInfo.numFoundPlayerServers++;
						} else {
							// can't add any more so we're done
							uiInfo.pendingServerStatus.num = uiInfo.serverStatus.numDisplayServers;
						}
					}
				}
				Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
							 sizeof( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1] ),
							 "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound );
				// retrieved the server status so reuse this spot
				uiInfo.pendingServerStatus.server[i].valid = qfalse;
			}
		}
		// if empty pending slot or timed out
		if ( !uiInfo.pendingServerStatus.server[i].valid ||
			 uiInfo.pendingServerStatus.server[i].startTime < uiInfo.uiDC.realTime - ui_serverStatusTimeOut.integer ) {
			if ( uiInfo.pendingServerStatus.server[i].valid ) {
				numTimeOuts++;
			}
			// reset server status request for this address
			UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, NULL );
			// reuse pending slot
			uiInfo.pendingServerStatus.server[i].valid = qfalse;
			// if we didn't try to get the status of all servers in the main browser yet
			UI_UpdateDisplayServers();
			if ( uiInfo.pendingServerStatus.num < uiInfo.serverStatus.numDisplayServers ) {
				uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiDC.realTime;
				trap_LAN_GetServerAddressString( ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
												 uiInfo.pendingServerStatus.server[i].adrstr, sizeof( uiInfo.pendingServerStatus.server[i].adrstr ) );
				trap_LAN_GetServerInfo( ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num], infoString, sizeof( infoString ) );
				Q_strncpyz( uiInfo.pendingServerStatus.server[i].name, Info_ValueForKey( infoString, "hostname" ), sizeof( uiInfo.pendingServerStatus.server[0].name ) );
				uiInfo.pendingServerStatus.server[i].valid = qtrue;
				uiInfo.pendingServerStatus.num++;
				Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
							 sizeof( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1] ),
							 "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound );
			}
		}
	}
	for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
		if ( uiInfo.pendingServerStatus.server[i].valid ) {
			break;
		}
	}
	// if still trying to retrieve server status info
	if ( i < MAX_SERVERSTATUSREQUESTS ) {
		uiInfo.nextFindPlayerRefresh = uiInfo.uiDC.realTime + 25;
	} else {
		// add a line that shows the number of servers found
		if ( !uiInfo.numFoundPlayerServers ) {
			Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1], sizeof( uiInfo.foundPlayerServerAddresses[0] ), "no servers found" );
		} else {
			Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1], sizeof( uiInfo.foundPlayerServerAddresses[0] ),
						 "%d server%s found with player %s", uiInfo.numFoundPlayerServers - 1,
						 uiInfo.numFoundPlayerServers == 2 ? "" : "s", uiInfo.findPlayerName );
		}
		uiInfo.nextFindPlayerRefresh = 0;
		// show the server status info for the selected server
		UI_FeederSelection( FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer );
	}
}

/*
==================
UI_BuildServerStatus  0x4000C500 -- RTCW's, less the URL buttons
==================
*/
static void UI_BuildServerStatus( qboolean force ) {

	if ( uiInfo.nextFindPlayerRefresh ) {
		return;
	}
	if ( !force ) {
		if ( !uiInfo.nextServerStatusRefresh || uiInfo.nextServerStatusRefresh > uiInfo.uiDC.realTime ) {
			return;
		}
	} else {
		Menu_SetFeederSelection( NULL, FEEDER_SERVERSTATUS, 0, NULL );
		uiInfo.serverStatusInfo.numLines = 0;
		// reset all server status requests
		trap_LAN_ServerStatus( NULL, NULL, 0 );
	}
	UI_UpdateDisplayServers();
	if ( uiInfo.serverStatus.currentServer < 0 || uiInfo.serverStatus.currentServer > uiInfo.serverStatus.numDisplayServers || uiInfo.serverStatus.numDisplayServers == 0 ) {
		return;
	}
	if ( UI_GetServerStatusInfo( uiInfo.serverStatusAddress, &uiInfo.serverStatusInfo ) ) {
		uiInfo.nextServerStatusRefresh = 0;
		UI_GetServerStatusInfo( uiInfo.serverStatusAddress, NULL );
	} else {
		uiInfo.nextServerStatusRefresh = uiInfo.uiDC.realTime + 500;
	}
}

/*
==================
UI_FeederCount  0x4000C5B0 -- no FEEDER_TEAM_LIST count in CoD
==================
*/
static int UI_FeederCount( float feederID ) {
	if ( feederID == FEEDER_HEADS ) {
		return uiInfo.characterCount;
	} else if ( feederID == FEEDER_CINEMATICS ) {
		return uiInfo.movieCount;
	} else if ( feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS ) {
		return UI_MapCountByGameType();
	} else if ( feederID == FEEDER_SERVERS ) {
		UI_UpdateDisplayServers();
		return uiInfo.serverStatus.numDisplayServers;
	} else if ( feederID == FEEDER_SERVERSTATUS ) {
		return uiInfo.serverStatusInfo.numLines;
	} else if ( feederID == FEEDER_FINDPLAYER ) {
		return uiInfo.numFoundPlayerServers;
	} else if ( feederID == FEEDER_PLAYER_LIST ) {
		if ( uiInfo.uiDC.realTime > uiInfo.playerRefresh ) {
			uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
			UI_BuildPlayerList();
		}
		return uiInfo.playerCount;
	} else if ( feederID == FEEDER_MODS ) {
		return uiInfo.modCount;
	} else if ( feederID == FEEDER_DEMOS ) {
		return uiInfo.demoCount;
	}
	return 0;
}

/* ---- UI_SelectedMap  0x4000C6C0 ---- */
static const char *UI_SelectedMap( int index, int *actual ) {
	int i, c;
	c = 0;
	*actual = 0;
	for ( i = 0; i < uiInfo.mapCount; i++ ) {
		if ( uiInfo.mapList[i].active ) {
			if ( c == index ) {
				*actual = i;
				return uiInfo.mapList[i].mapName;
			} else {
				c++;
			}
		}
	}
	return "";
}

/* ---- UI_UpdatePendingPings  0x4000C710 ---- */
static void UI_UpdatePendingPings( void ) {
	trap_LAN_ResetPings( ui_netSource.integer );
	uiInfo.serverStatus.refreshActive = qtrue;
	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
}

// NERVE - SMF
/* ---- UI_FeederAddItem  0x4000C740 ---- */
static void UI_FeederAddItem( float feederID, const char *name, int index ) {

}
// -NERVE - SMF

//----(SA)	added (whoops, this got nuked in a check-in...)
/* ---- UI_FileText  0x4000C750 ---- */
static const char *UI_FileText( const char *fileName ) {
	int len;
	fileHandle_t f;
	static char buf[MAX_MENUDEFFILE];               /* 0x40051160 */

//	return "flubber";

	len = trap_FS_FOpenFile( fileName, &f, FS_READ );
	if ( !f ) {
		return NULL;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	return &buf[0];
}
//----(SA)	end


/* ---- UI_FeederItemText  0x4000C7A0 -- CoD's server columns (password/host/map/clients/game/ping) and the @-translated status lines ---- */
static const char *UI_FeederItemText( float feederID, int index, int column, qhandle_t *handle ) {
	static char info[MAX_STRING_CHARS];             /* 0x40050D60 */
	static char clientBuff[32];                     /* 0x4005A960 */
	static int lastColumn = -1;                     /* 0x40036F94 */
	static int lastTime = 0;                        /* 0x401BF424 */
	*handle = -1;
	if ( feederID == FEEDER_HEADS ) {
		if ( index >= 0 && index < uiInfo.characterCount ) {
			return uiInfo.characterList[index].name;
		}
	} else if ( feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS ) {
		int actual;
		return UI_SelectedMap( index, &actual );
	} else if ( feederID == FEEDER_SERVERS ) {
		UI_UpdateDisplayServers();
		if ( index >= 0 && index < uiInfo.serverStatus.numDisplayServers ) {
			int ping;
			if ( lastColumn != column || lastTime > uiInfo.uiDC.realTime + 5000 ) {
				trap_LAN_GetServerInfo( ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS );
				lastColumn = column;
				lastTime = uiInfo.uiDC.realTime;
			}
			ping = atoi( Info_ValueForKey( info, "ping" ) );
			switch ( column ) {
			case SORT_PASSWORD:
				if ( atoi( Info_ValueForKey( info, "pswrd" ) ) ) {
					return "X";
				} else {
					return "";
				}
			case SORT_HOST:
				if ( ping <= 0 ) {
					return Info_ValueForKey( info, "addr" );
				} else {
					return Info_ValueForKey( info, "hostname" );
				}
			case SORT_MAP: return Info_ValueForKey( info, "mapname" );
			case SORT_CLIENTS:
				Com_sprintf( clientBuff, sizeof( clientBuff ), "%s (%s)", Info_ValueForKey( info, "clients" ), Info_ValueForKey( info, "sv_maxclients" ) );
				return clientBuff;
			case SORT_GAME:
				if ( Info_ValueForKey( info, "gametype" ) && *Info_ValueForKey( info, "gametype" ) ) {
					return Info_ValueForKey( info, "gametype" );
				} else {
					return "?";
				}
			case SORT_PING:
				if ( ping <= 0 ) {
					return "...";
				} else {
					return Info_ValueForKey( info, "ping" );
				}
			}
		}
	} else if ( feederID == FEEDER_SERVERSTATUS ) {
		if ( index >= 0 && index < uiInfo.serverStatusInfo.numLines ) {
			if ( column >= 0 && column < 4 ) {
				const char *s = uiInfo.serverStatusInfo.lines[index][column];
				if ( s[0] == '@' ) {
					return UI_SafeTranslateString( s + 1 );
				}
				return s;
			}
		}
	} else if ( feederID == FEEDER_FINDPLAYER ) {
		if ( index >= 0 && index < uiInfo.numFoundPlayerServers ) {
			//return uiInfo.foundPlayerServerAddresses[index];
			return uiInfo.foundPlayerServerNames[index];
		}
	} else if ( feederID == FEEDER_PLAYER_LIST ) {
		if ( index >= 0 && index < uiInfo.playerCount ) {
			return uiInfo.playerNames[index];
		}
	} else if ( feederID == FEEDER_TEAM_LIST ) {
		if ( index >= 0 && index < uiInfo.myTeamCount ) {
			return uiInfo.teamNames[index];
		}
	} else if ( feederID == FEEDER_MODS ) {
		if ( index >= 0 && index < uiInfo.modCount ) {
			if ( uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr ) {
				return uiInfo.modList[index].modDescr;
			} else {
				return uiInfo.modList[index].modName;
			}
		}
	} else if ( feederID == FEEDER_CINEMATICS ) {
		if ( index >= 0 && index < uiInfo.movieCount ) {
			return uiInfo.movieList[index];
		}
	} else if ( feederID == FEEDER_DEMOS ) {
		if ( index >= 0 && index < uiInfo.demoCount ) {
			return uiInfo.demoList[index];
		}
	}
	return "";
}


/* ---- UI_FeederItemImage  0x4000CB70 -- imageTrack 2 ---- */
static qhandle_t UI_FeederItemImage( float feederID, int index ) {
	if ( feederID == FEEDER_HEADS ) {
		if ( index >= 0 && index < uiInfo.characterCount ) {
			if ( uiInfo.characterList[index].headImage == -1 ) {
				uiInfo.characterList[index].headImage = trap_R_RegisterShaderNoMip( uiInfo.characterList[index].imageName, 2 );
			}
			return uiInfo.characterList[index].headImage;
		}
	} else if ( feederID == FEEDER_ALLMAPS || feederID == FEEDER_MAPS ) {
		int actual;

		UI_SelectedMap( index, &actual );
		index = actual;
		if ( index >= 0 && index < uiInfo.mapCount ) {
			if ( uiInfo.mapList[index].levelShot == -1 ) {
				uiInfo.mapList[index].levelShot = trap_R_RegisterShaderNoMip( uiInfo.mapList[index].imageName, 2 );
			}
			return uiInfo.mapList[index].levelShot;
		}
	}

	return 0;
}

/* ---- UI_FeederSelection  0x4000CC40 -- RTCW's without the map/server cinematics ---- */
static void UI_FeederSelection( float feederID, int index ) {
	static char info[MAX_STRING_CHARS];             /* 0x40052160 */
	if ( feederID == FEEDER_HEADS ) {
		if ( index >= 0 && index < uiInfo.characterCount ) {
			trap_Cvar_Set( "team_model", uiInfo.characterList[index].female ? "janet" : "james" );
			trap_Cvar_Set( "team_headmodel", va( "*%s", uiInfo.characterList[index].name ) );
		}
	} else if ( feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS ) {
		int actual, map;
		map = ( feederID == FEEDER_ALLMAPS ) ? ui_currentNetMap.integer : ui_currentMap.integer;
		if ( uiInfo.mapList[map].cinematic >= 0 ) {
			trap_CIN_StopCinematic( uiInfo.mapList[map].cinematic );
			uiInfo.mapList[map].cinematic = -1;
		}
		UI_SelectedMap( index, &actual );
		trap_Cvar_Set( "ui_mapIndex", va( "%d", index ) );
		ui_mapIndex.integer = index;

		// NERVE - SMF - setup advanced server vars
		if ( feederID == FEEDER_ALLMAPS ) {
			ui_currentMap.integer = actual;
			trap_Cvar_Set( "ui_currentMap", va( "%d", actual ) );
		}
		// -NERVE - SMF

		if ( feederID == FEEDER_MAPS ) {
			ui_currentMap.integer = actual;
			trap_Cvar_Set( "ui_currentMap", va( "%d", actual ) );
		} else {
			ui_currentNetMap.integer = actual;
			trap_Cvar_Set( "ui_currentNetMap", va( "%d", actual ) );
		}

	} else if ( feederID == FEEDER_SERVERS ) {
		uiInfo.serverStatus.currentServer = index;
		trap_LAN_GetServerInfo( ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS );
		uiInfo.serverStatus.currentServerPreview = trap_R_RegisterShaderNoMip( va( "levelshots/%s", Info_ValueForKey( info, "mapname" ) ), 2 );
		if ( uiInfo.serverStatus.currentServerCinematic >= 0 ) {
			trap_CIN_StopCinematic( uiInfo.serverStatus.currentServerCinematic );
			uiInfo.serverStatus.currentServerCinematic = -1;
		}
	} else if ( feederID == FEEDER_SERVERSTATUS ) {
		//
	} else if ( feederID == FEEDER_FINDPLAYER ) {
		uiInfo.currentFoundPlayerServer = index;
		//
		if ( index < uiInfo.numFoundPlayerServers - 1 ) {
			// build a new server status for this server
			Q_strncpyz( uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof( uiInfo.serverStatusAddress ) );
			Menu_SetFeederSelection( NULL, FEEDER_SERVERSTATUS, 0, NULL );
			UI_BuildServerStatus( qtrue );
		}
	} else if ( feederID == FEEDER_PLAYER_LIST ) {
		uiInfo.playerIndex = index;
	} else if ( feederID == FEEDER_TEAM_LIST ) {
		uiInfo.teamIndex = index;
	} else if ( feederID == FEEDER_MODS ) {
		uiInfo.modIndex = index;
	} else if ( feederID == FEEDER_CINEMATICS ) {
		uiInfo.movieIndex = index;
		if ( uiInfo.previewMovie >= 0 ) {
			trap_CIN_StopCinematic( uiInfo.previewMovie );
		}
		uiInfo.previewMovie = -1;
	} else if ( feederID == FEEDER_DEMOS ) {
		uiInfo.demoIndex = index;
	}
}

/*
===============
UI_GetGameTypesList  0x4000CF80

CoD's replacement for UI_ParseGameInfo: one game type per
maps/mp/gametypes/*.gsc (files starting with '_' skipped), its display name
the first token of the matching .txt.  joinGameTypes gets "All" first.
===============
*/
void UI_GetGameTypesList( void ) {
	char dirlist[4096];
	char *dirptr;
	char *buf;
	int numdirs;
	int i;
	int dirlen;

	uiInfo.numGameTypes = 0;
	uiInfo.numJoinGameTypes = 0;

	uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gameType = String_Alloc( "All" );
	uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gtEnum = "";
	uiInfo.numJoinGameTypes++;

	numdirs = trap_FS_GetFileList( "maps/mp/gametypes", "gsc", dirlist, sizeof( dirlist ) );
	dirptr = dirlist;
	for ( i = 0; i < numdirs; i++, dirptr += dirlen + 1 ) {
		dirlen = strlen( dirptr );
		if ( dirptr[0] == '_' ) {
			continue;
		}
		if ( !Q_stricmp( dirptr + dirlen - 4, ".gsc" ) ) {
			dirptr[dirlen - 4] = '\0';
		}

		if ( uiInfo.numGameTypes == MAX_GAMETYPES || uiInfo.numJoinGameTypes == MAX_GAMETYPES ) {
			Com_Printf( "Too many game type scripts found! Only loading the first %i\n", MAX_GAMETYPES - 1 );
			break;
		}

		uiInfo.gameTypes[uiInfo.numGameTypes].gameType = String_Alloc( dirptr );
		uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gameType = uiInfo.gameTypes[uiInfo.numGameTypes].gameType;

		buf = getMenuBuffer( va( "maps/mp/gametypes/%s.txt", dirptr ) );
		if ( buf ) {
			uiInfo.gameTypes[uiInfo.numGameTypes].gtEnum = String_Alloc( Com_Parse( &buf ) );
		} else {
			uiInfo.gameTypes[uiInfo.numGameTypes].gtEnum = uiInfo.gameTypes[uiInfo.numGameTypes].gameType;
		}
		uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gtEnum = uiInfo.gameTypes[uiInfo.numGameTypes].gtEnum;

		uiInfo.numGameTypes++;
		uiInfo.numJoinGameTypes++;
	}

	if ( !uiInfo.numGameTypes ) {
		Com_Error( ERR_FATAL, "\x15No game type scripts found in maps/mp/gametypes folder" );
	}
}

/* ---- UI_Pause  0x4000D1A0 ---- */
static void UI_Pause( qboolean b ) {
	if ( b ) {
		// pause the game and set the ui keycatcher
		trap_Cvar_Set( "cl_paused", "1" );
		trap_Key_SetCatcher( KEYCATCH_UI );
	} else {
		// unpause the game and clear the ui keycatcher
		trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
		trap_Key_ClearStates();
		trap_Cvar_Set( "cl_paused", "0" );
	}
}

/*
// TTimo: unused
static int UI_OwnerDraw_Width(int ownerDraw) {
  return 0;
}
*/

/* ---- UI_PlayCinematic  0x4000D1F0 ---- */
static int UI_PlayCinematic( const char *name, float x, float y, float w, float h ) {
	return trap_CIN_PlayCinematic( name, x, y, w, h, ( CIN_loop | CIN_silent ) );
}

/* ---- UI_StopCinematic  0x4000D240 ---- */
static void UI_StopCinematic( int handle ) {
	if ( handle >= 0 ) {
		trap_CIN_StopCinematic( handle );
	} else {
		handle = abs( handle );
		if ( handle == UI_MAPCINEMATIC ) {
			if ( uiInfo.mapList[ui_currentMap.integer].cinematic >= 0 ) {
				trap_CIN_StopCinematic( uiInfo.mapList[ui_currentMap.integer].cinematic );
				uiInfo.mapList[ui_currentMap.integer].cinematic = -1;
			}
		} else if ( handle == UI_NETMAPCINEMATIC ) {
			if ( uiInfo.serverStatus.currentServerCinematic >= 0 ) {
				trap_CIN_StopCinematic( uiInfo.serverStatus.currentServerCinematic );
				uiInfo.serverStatus.currentServerCinematic = -1;
			}
		}
	}
}

/* ---- UI_DrawCinematic  0x4000D2B0 ---- */
static void UI_DrawCinematic( int handle, float x, float y, float w, float h ) {
	trap_CIN_SetExtents( handle, x, y, w, h );
	trap_CIN_DrawCinematic( handle );
}

/* ---- UI_RunCinematicFrame  0x4000D300 ---- */
static void UI_RunCinematicFrame( int handle ) {
	trap_CIN_RunCinematic( handle );
}

/*
=================
_UI_Init  0x4000D310

Fills the DC by slot in ui_shared.h's order.  RTCW's inGameLoad parameter
is gone: vmMain's UI_INIT case sets nothing up before the call.
=================
*/
void _UI_Init( void ) {
	int start;
	const char *menuSet;

	//uiInfo.inGameLoad = inGameLoad;

	UI_RegisterCvars();
	UI_InitMemory();

	trap_Cvar_Set( "ui_menuFiles", "ui_mp/menus.txt" ); // NERVE - SMF - we need to hardwire for wolfMP

	// cache redundant calulations
	trap_GetGlconfig( &UI_GLCONFIG );

	// for 640x480 virtualized screen
	uiInfo.uiDC.yscale = UI_GLCONFIG.vidHeight * ( 1.0f / 480.0f );
	uiInfo.uiDC.xscale = UI_GLCONFIG.vidWidth * ( 1.0f / 640.0f );
	if ( UI_GLCONFIG.vidWidth * 480 > UI_GLCONFIG.vidHeight * 640 ) {
		// wide screen
		uiInfo.uiDC.bias = 0.5f * ( UI_GLCONFIG.vidWidth - ( UI_GLCONFIG.vidHeight * ( 640.0f / 480.0f ) ) );
	} else {
		// no wide screen
		uiInfo.uiDC.bias = 0;
	}


	//UI_Load();
	uiInfo.uiDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	uiInfo.uiDC.setColor = &UI_SetColor;
	uiInfo.uiDC.drawHandlePic = &UI_DrawHandlePic;
	uiInfo.uiDC.drawStretchPic = &trap_R_DrawStretchPic;
	uiInfo.uiDC.drawText = &trap_R_Text_Paint;
	uiInfo.uiDC.textWidth = &trap_R_Text_Width;
	uiInfo.uiDC.textHeight = &trap_R_Text_Height;
	uiInfo.uiDC.translateReference = &trap_SE_TranslateReference;
	uiInfo.uiDC.safeTranslateString = &UI_SafeTranslateString;
	uiInfo.uiDC.translatedMessage = &trap_SE_LocalizeMessage;
	uiInfo.uiDC.setFont = &Text_SetActiveFont;
	uiInfo.uiDC.registerModel = &trap_R_RegisterModel;
	uiInfo.uiDC.modelBounds = &trap_R_ModelBounds;
	uiInfo.uiDC.fillRect = &UI_FillRect;
	uiInfo.uiDC.drawRect = &_UI_DrawRect;
	uiInfo.uiDC.drawTopBottom = &_UI_DrawTopBottom;
	uiInfo.uiDC.clearScene = &trap_R_ClearScene;
	uiInfo.uiDC.drawSides = &_UI_DrawSides;
	uiInfo.uiDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	uiInfo.uiDC.renderScene = &trap_R_RenderScene;
	uiInfo.uiDC.registerFont = &trap_R_RegisterFont;
	uiInfo.uiDC.ownerDrawItem = &UI_OwnerDraw;
	uiInfo.uiDC.getValue = &UI_GetValue;
	uiInfo.uiDC.ownerDrawVisible = &UI_OwnerDrawVisible;
	uiInfo.uiDC.runScript = &UI_RunMenuScript;
	uiInfo.uiDC.getTeamColor = &UI_GetTeamColor;
	uiInfo.uiDC.setCVar = trap_Cvar_Set;
	uiInfo.uiDC.getCVarString = trap_Cvar_VariableStringBuffer;
	uiInfo.uiDC.getCVarValue = trap_Cvar_VariableValue;
	uiInfo.uiDC.configString = &UI_ConfigString;
	uiInfo.uiDC.drawTextWithCursor = &trap_R_Text_PaintWithCursor;
	uiInfo.uiDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	uiInfo.uiDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	uiInfo.uiDC.playClientSoundAliasByName = &trap_S_StartLocalSound;
	uiInfo.uiDC.ownerDrawHandleKey = &UI_OwnerDrawHandleKey;
	uiInfo.uiDC.feederCount = &UI_FeederCount;
	uiInfo.uiDC.feederItemImage = &UI_FeederItemImage;
	uiInfo.uiDC.feederItemText = &UI_FeederItemText;
	uiInfo.uiDC.fileText = &UI_FileText;    //----(SA)	re-added
	uiInfo.uiDC.feederSelection = &UI_FeederSelection;
	uiInfo.uiDC.feederAddItem = &UI_FeederAddItem;                  // NERVE - SMF
	uiInfo.uiDC.setBinding = &trap_Key_SetBinding;
	uiInfo.uiDC.getBindingBuf = &trap_Key_GetBindingBuf;
	uiInfo.uiDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	uiInfo.uiDC.executeText = &trap_Cmd_ExecuteText;
	uiInfo.uiDC.Error = &Com_Error;
	uiInfo.uiDC.Print = &Com_Printf;
	uiInfo.uiDC.Pause = &UI_Pause;
	uiInfo.uiDC.ownerDrawWidth = &UI_OwnerDrawWidth;
	uiInfo.uiDC.registerSound = &trap_S_RegisterSound;
	uiInfo.uiDC.playCinematic = &UI_PlayCinematic;
	uiInfo.uiDC.stopCinematic = &UI_StopCinematic;
	uiInfo.uiDC.drawCinematic = &UI_DrawCinematic;
	uiInfo.uiDC.runCinematicFrame = &UI_RunCinematicFrame;
	/* +0xA4 <- trap 0x64, +0xA8 <- trap 0x65 (0x4000D5C0, 0x4000D5CA): ui_shared.h's
	   slot names are the semantic ones, the wrappers keep RTCW's names */
	uiInfo.uiDC.getAutoUpdate = &trap_CheckAutoUpdate;            // DHM - Nerve
	uiInfo.uiDC.runningGame = &trap_GetAutoUpdate;                // DHM - Nerve

	Init_Display( &uiInfo.uiDC );

	String_Init();

	uiInfo.uiDC.whiteShader = trap_R_RegisterShaderNoMip( "white", 2 );

	AssetCache();

	start = trap_Milliseconds();

	uiInfo.teamCount = 0;
	uiInfo.characterCount = 0;
	uiInfo.aliasCount = 0;

	UI_GetGameTypesList();
	UI_LoadArenas();

	menuSet = UI_Cvar_VariableString( "ui_menuFiles" );
	if ( menuSet == NULL || menuSet[0] == '\0' ) {
		menuSet = "ui_mp/menus.txt";
	}
	UI_LoadMenus( menuSet, qtrue, 2 );
	UI_LoadMenus( "ui_mp/ingame.txt", qfalse, 2 );

	Menus_CloseAll();

	trap_LAN_LoadCachedServers();

	UI_ServersSort( SORT_HOST, qfalse );

	// sets defaults for ui temp cvars
	trap_Cvar_Set( "ui_mousePitch", ( trap_Cvar_VariableValue( "m_pitch" ) >= 0 ) ? "0" : "1" );

	uiInfo.serverStatus.currentServerCinematic = -1;
	uiInfo.previewMovie = -1;

	trap_Cvar_Register( NULL, "debug_protocol", "", 0 );

	// NERVE - SMF - hardwire net cvars
	trap_Cvar_Set( "ui_netGameTypeName", uiInfo.gameTypes[ui_netGameType.integer].gameType );
	// -NERVE - SMF

	trap_Cvar_Register( NULL, "ui_multiplayer", "1", CVAR_ROM );
}


/*
=================
UI_KeyEvent  0x4000D6F0
=================
*/
void _UI_KeyEvent( int key, qboolean down ) {
	static qboolean bypassKeyClear = qfalse;        /* 0x401BF428 */

	if ( Menu_Count() > 0 ) {
		menuDef_t *menu = Menu_GetFocused();
		if ( menu ) {
			if ( trap_Cvar_VariableValue( "cl_bypassMouseInput" ) ) {
				bypassKeyClear = qtrue;
			}

			if ( key == K_ESCAPE && down && !UI_IsFullscreen() ) {
				Menus_CloseAll();
			} else {
				Menu_HandleKey( menu, key, down );
			}
		} else {
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );

			// NERVE - SMF - we don't want to clear key states if bypassing input
			if ( !bypassKeyClear ) {
				trap_Key_ClearStates();
			}

			bypassKeyClear = qfalse;

			trap_Cvar_Set( "cl_paused", "0" );
		}
	}

	//if ((s > 0) && (s != menu_null_sound)) {
	//  trap_S_StartLocalSound( s, CHAN_LOCAL_SOUND );
	//}
}

/*
=================
UI_MouseEvent  0x4000D790
=================
*/
void _UI_MouseEvent( int dx, int dy ) {
	// update mouse screen position
	uiInfo.uiDC.cursorx += dx;
	if ( uiInfo.uiDC.cursorx < 0 ) {
		uiInfo.uiDC.cursorx = 0;
	} else if ( uiInfo.uiDC.cursorx > SCREEN_WIDTH ) {
		uiInfo.uiDC.cursorx = SCREEN_WIDTH;
	}

	uiInfo.uiDC.cursory += dy;
	if ( uiInfo.uiDC.cursory < 0 ) {
		uiInfo.uiDC.cursory = 0;
	} else if ( uiInfo.uiDC.cursory > SCREEN_HEIGHT ) {
		uiInfo.uiDC.cursory = SCREEN_HEIGHT;
	}

	if ( Menu_Count() > 0 ) {
		//menuDef_t *menu = Menu_GetFocused();
		//Menu_HandleMouseMove(menu, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
		Display_MouseMove( NULL, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory );
	}

}


//----(SA)	added
static uiMenuCommand_t menutype = UIMENU_NONE;      /* 0x401BF42C */

/* ---- _UI_GetActiveMenu  0x4000D800 ---- */
uiMenuCommand_t _UI_GetActiveMenu( void ) {
	return menutype;
}
//----(SA)	end

/*
=================
_UI_SetActiveMenu  0x4000D810

Answers qtrue for every menu it opened (each case ends `mov eax, 1`),
qfalse otherwise; vmMain hands that back.  The two script popups do not
replace menutype on entry and refuse to open over a focused non-popup menu.
=================
*/
qboolean _UI_SetActiveMenu( uiMenuCommand_t menu ) {
	char buf[256];

	// this should be the ONLY way the menu system is brought up
	// enusure minumum menu data is cached
	if ( Menu_Count() > 0 ) {
		if ( menu != UIMENU_SCRIPT_POPUP && menu != UIMENU_SCRIPT_POPUP_NO_MOUSE ) {
			menutype = menu;    //----(SA)	added
		}

		switch ( menu ) {
		case UIMENU_NONE:
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
			trap_Key_ClearStates();
			trap_Cvar_Set( "cl_paused", "0" );
			Menus_CloseAll();

			return qtrue;
		case UIMENU_MAIN:
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_OpenByName( "main" );
			trap_Cvar_VariableStringBuffer( "com_errorMessage", buf, sizeof( buf ) );
			// JPW NERVE stricmp() is silly but works, take a look at error.menu to see why.  I think this is bustified in q3ta
			if ( ( strlen( buf ) ) && ( Q_stricmp( buf,";" ) ) ) {
				Menus_OpenByName( "error_popmenu" );
			}
			return qtrue;

		case UIMENU_INGAME:
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			trap_Cvar_VariableStringBuffer( "g_scriptMainMenu", buf, sizeof( buf ) );
			Menus_OpenByName( buf );
			return qtrue;

		case UIMENU_NEED_CD:
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_OpenByName( "needcd" );
			return qtrue;

		case UIMENU_BAD_CD_KEY:
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_OpenByName( "badcd" );
			return qtrue;

		case UIMENU_TEAM:
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_OpenByName( "team" );
			return qtrue;

			// NERVE - SMF
		case UIMENU_WM_QUICKMESSAGE:
			DC->cursorx = 639;
			DC->cursory = 479;
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_OpenByName( "quickmessage" );
			return qtrue;

		case UIMENU_WM_AUTOUPDATE:
			Menus_OpenByName( "autoupdate" );
			return qtrue;
			// -NERVE - SMF

		case UIMENU_SCRIPT_POPUP:
		case UIMENU_SCRIPT_POPUP_NO_MOUSE:
		{
			menuDef_t *focused = Menu_GetFocused();

			if ( focused && menutype != UIMENU_SCRIPT_POPUP && menutype != UIMENU_SCRIPT_POPUP_NO_MOUSE ) {
				return qfalse;
			}

			trap_Cvar_VariableStringBuffer( "ui_newScriptMenu", buf, sizeof( buf ) );
			if ( !focused || Q_stricmp( buf, focused->window.name ) ) {
				menutype = UIMENU_SCRIPT_POPUP;
				if ( menu == UIMENU_SCRIPT_POPUP_NO_MOUSE ) {
					DC->cursorx = 639;
					DC->cursory = 479;
				} else {
					DC->cursorx = 320;
					DC->cursory = 240;
				}
				trap_Key_SetCatcher( KEYCATCH_UI );
				Menus_CloseAll();
				trap_Cvar_Set( "ui_scriptMenu", buf );
				trap_Cvar_VariableStringBuffer( "ui_newScriptMenuIndex", buf, sizeof( buf ) );
				trap_Cvar_Set( "ui_scriptMenuIndex", buf );
				trap_Cvar_Set( "ui_newScriptMenu", "" );
				trap_Cvar_Set( "ui_newScriptMenuIndex", "-1" );
				trap_Cvar_VariableStringBuffer( "ui_scriptMenu", buf, sizeof( buf ) );
				Menus_OpenByName( buf );
			}
			return qtrue;
		}

		default:
			return qfalse; // TTimo: a lot of not handled
		}
	}
	return qfalse;
}

/* ---- _UI_IsFullscreen  0x4000DC00 ---- */
qboolean _UI_IsFullscreen( void ) {
	return UI_IsFullscreen();
}

/* ---- UI_ReadableSize  0x4000DC10 -- the units are EXE_* references ---- */
static void UI_ReadableSize( char *buf, int bufsize, int value ) {
	if ( value > 1024 * 1024 * 1024 ) { // gigs
		Com_sprintf( buf, bufsize, "%d", value / ( 1024 * 1024 * 1024 ) );
		Com_sprintf( buf + strlen( buf ), bufsize - strlen( buf ), ".%02d %s",
					 ( value % ( 1024 * 1024 * 1024 ) ) * 100 / ( 1024 * 1024 * 1024 ), UI_SafeTranslateString( "EXE_GIGABYTE" ) );
	} else if ( value > 1024 * 1024 ) { // megs
		Com_sprintf( buf, bufsize, "%d", value / ( 1024 * 1024 ) );
		Com_sprintf( buf + strlen( buf ), bufsize - strlen( buf ), ".%02d %s",
					 ( value % ( 1024 * 1024 ) ) * 100 / ( 1024 * 1024 ), UI_SafeTranslateString( "EXE_MEGABYTE" ) );
	} else if ( value > 1024 ) { // kilos
		Com_sprintf( buf, bufsize, "%d %s", value / 1024, UI_SafeTranslateString( "EXE_KILOBYTE" ) );
	} else { // bytes
		Com_sprintf( buf, bufsize, "%d %s", value, UI_SafeTranslateString( "EXE_BYTES" ) );
	}
}

// Assumes time is in sec
/* ---- UI_PrintTime  0x4000DD90 ---- */
static void UI_PrintTime( char *buf, int bufsize, int time ) {
	//time /= 1000;  // change to seconds

	if ( time > 3600 ) { // in the hours range
		Com_sprintf( buf, bufsize, "%d %s %d %s", time / 3600, UI_SafeTranslateString( "EXE_HOURS" ), ( time % 3600 ) / 60, UI_SafeTranslateString( "EXE_MINUTES" ) );
	} else if ( time > 60 ) { // mins
		Com_sprintf( buf, bufsize, "%d %s %d %s", time / 60, UI_SafeTranslateString( "EXE_MINUTES" ), time % 60, UI_SafeTranslateString( "EXE_SECONDS" ) );
	} else  { // secs
		Com_sprintf( buf, bufsize, "%d %s", time, UI_SafeTranslateString( "EXE_SECONDS" ) );
	}
}

/* ---- Text_PaintCenter  0x4000DE50 ---- */
void Text_PaintCenter( float x, float y, int font, float scale, const vec4_t color, const char *text, float adjust ) {
	int len = trap_R_Text_Width( text, font, scale, 0 );
	trap_R_Text_Paint( x - len / 2, y, font, scale, color, text, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE );
}

#define ESTIMATES 80
/*
=================
UI_DisplayDownloadInfo  0x4000DEA0

RTCW's, with CoD's progress bar (three nested fills, black under red) and
the EXE_* captions in colorLtGrey at x 24/192/264/200.
=================
*/
static void UI_DisplayDownloadInfo( const char *downloadName, float centerPoint, float yStart, int font, float scale ) {
	static char dlText[]    = "EXE_DOWNLOADING";    /* 0x40036F98 */
	static char etaText[]   = "EXE_EST_TIME_LEFT";  /* 0x40036FA8 */
	static char xferText[]  = "EXE_TRANS_RATE";     /* 0x40036FBC */
	static int tleEstimates[ESTIMATES] = { 60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,
										   60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,
										   60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,
										   60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60 };  /* 0x40036FD0 */
	static int tleIndex = 0;                        /* 0x401BF430 */

	int downloadSize, downloadCount, downloadTime;
	char dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
	int xferRate;
	int width;
	const char *s;
	vec4_t color;

	downloadSize = trap_Cvar_VariableValue( "cl_downloadSize" );
	downloadCount = trap_Cvar_VariableValue( "cl_downloadCount" );
	downloadTime = trap_Cvar_VariableValue( "cl_downloadTime" );

	// Background
	VectorCopy( colorBlack, color );
	color[3] = 0.2f;
	UI_FillRect( 0, yStart + 184, 640, 85, color );
	UI_FillRect( 0, yStart + 185, 640, 83, color );
	UI_FillRect( 0, yStart + 186, 640, 81, color );

	if ( downloadSize > 0 ) {
		VectorCopy( colorRed, color );
		color[3] = 0.15f;
		width = (float)downloadCount / downloadSize * 640;
		UI_FillRect( 0, yStart + 184, width + 2, 85, color );
		UI_FillRect( 0, yStart + 185, width + 1, 83, color );
		UI_FillRect( 0, yStart + 186, width, 81, color );
	}

	UI_SetColor( colorLtGrey );
	trap_R_Text_Paint( 24, yStart + 210, font, scale, colorLtGrey, UI_SafeTranslateString( dlText ), 0, 64, ITEM_TEXTSTYLE_SHADOWED );
	trap_R_Text_Paint( 24, yStart + 235, font, scale, colorLtGrey, UI_SafeTranslateString( etaText ), 0, 64, ITEM_TEXTSTYLE_SHADOWED );
	trap_R_Text_Paint( 24, yStart + 260, font, scale, colorLtGrey, UI_SafeTranslateString( xferText ), 0, 64, ITEM_TEXTSTYLE_SHADOWED );

	// CoD 1.5 displays the complete download name without the 1.1 +8 offset.
	if ( downloadSize > 0 ) {
		s = va( "%s (%d%%)", downloadName, downloadCount * 100 / downloadSize );
	} else {
		s = downloadName;
	}

	trap_R_Text_Paint( 192, yStart + 210, font, scale, colorLtGrey, s, 0, 0, ITEM_TEXTSTYLE_SHADOWED );

	UI_ReadableSize( dlSizeBuf,     sizeof dlSizeBuf,       downloadCount );
	UI_ReadableSize( totalSizeBuf,  sizeof totalSizeBuf,    downloadSize );

	if ( downloadCount < 4096 || !downloadTime ) {
		Text_PaintCenter( centerPoint, yStart + 235, font, scale, colorLtGrey, UI_SafeTranslateString( "EXE_ESTIMATING" ), 0 );
		Text_PaintCenter( centerPoint, yStart + 340, font, scale, colorLtGrey, va( "(%s %s %s %s)", dlSizeBuf, UI_SafeTranslateString( "EXE_OF" ), totalSizeBuf, UI_SafeTranslateString( "EXE_COPIED" ) ), 0 );
	} else {
		if ( ( uiInfo.uiDC.realTime - downloadTime ) / 1000 ) {
			xferRate = downloadCount / ( ( uiInfo.uiDC.realTime - downloadTime ) / 1000 );
		} else {
			xferRate = 0;
		}
		UI_ReadableSize( xferRateBuf, sizeof xferRateBuf, xferRate );

		// Extrapolate estimated completion time
		if ( downloadSize && xferRate ) {
			int n = downloadSize / xferRate; // estimated time for entire d/l in secs
			int timeleft = 0, i;

			// We do it in K (/1024) because we'd overflow around 4MB
			tleEstimates[ tleIndex ] = ( n - ( ( ( downloadCount / 1024 ) * n ) / ( downloadSize / 1024 ) ) );
			tleIndex++;
			if ( tleIndex >= ESTIMATES ) {
				tleIndex = 0;
			}

			for ( i = 0; i < ESTIMATES; i++ )
				timeleft += tleEstimates[ i ];

			timeleft /= ESTIMATES;

			UI_PrintTime( dlTimeBuf, sizeof dlTimeBuf, timeleft );

			trap_R_Text_Paint( 264, yStart + 235, font, scale, colorLtGrey, dlTimeBuf, 0, 0, ITEM_TEXTSTYLE_SHADOWED );
			Text_PaintCenter( centerPoint, yStart + 320, font, scale, colorLtGrey, va( "(%s %s %s %s)", dlSizeBuf, UI_SafeTranslateString( "EXE_OF" ), totalSizeBuf, UI_SafeTranslateString( "EXE_COPIED" ) ), 0 );
		} else {
			Text_PaintCenter( centerPoint, yStart + 235, font, scale, colorLtGrey, UI_SafeTranslateString( "EXE_ESTIMATING" ), 0 );
			if ( downloadSize ) {
				Text_PaintCenter( centerPoint, yStart + 320, font, scale, colorLtGrey, va( "(%s %s %s %s)", dlSizeBuf, UI_SafeTranslateString( "EXE_OF" ), totalSizeBuf, UI_SafeTranslateString( "EXE_COPIED" ) ), 0 );
			} else {
				Text_PaintCenter( centerPoint, yStart + 320, font, scale, colorLtGrey, va( "(%s %s)", dlSizeBuf, UI_SafeTranslateString( "EXE_COPIED" ) ), 0 );
			}
		}

		if ( xferRate ) {
			trap_R_Text_Paint( 200, yStart + 260, font, scale, colorLtGrey, va( "%s/%s", xferRateBuf, UI_SafeTranslateString( "EXE_SECONDS" ) ), 0, 0, ITEM_TEXTSTYLE_SHADOWED );
		}
	}
}

/*
========================
UI_DrawConnectScreen  0x4000E4E0

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
#define CP_LINEWIDTH 50

void UI_DrawConnectScreen( qboolean overlay ) {
	const char      *s;
	uiClientState_t cstate;
	char info[MAX_INFO_VALUE];
	char text[256];

	char downloadName[MAX_INFO_VALUE];

	menuDef_t *menu = Menus_FindByName( "Connect" );


	if ( !overlay && menu ) {
		Menu_Paint( menu, qtrue );
	} else {
		UI_FillRect( 0, 0, 640, 96, colorBlack );	// 0x4000E53A: retail fills 640 x 96, not the full screen
	}

	// see what information we should display
	trap_GetClientState( &cstate );

	if ( cstate.connState >= CA_LOADING ) {
		if ( Q_stricmp( cstate.servername, "Auto-Updater" ) ) {
			info[0] = '\0';
			if ( trap_GetConfigString( CS_SERVERINFO, info, sizeof( info ) ) ) {
				Text_PaintCenter( 320, 55, 0, 0.5, colorWhite, UI_SafeTranslateString( UI_GetGameTypeDisplayName( Info_ValueForKey( info, "g_gametype" ) ) ), 0 );
				Text_PaintCenter( 320, 85, 0, 0.5, colorWhite, va( "%s", Info_ValueForKey( info, "mapname" ) ), 0 );
			}
		}
	} else if ( !Q_stricmp( cstate.servername,"localhost" ) ) {
		Text_PaintCenter( 320, 55, 0, 0.5, colorWhite, va( "%s - %s", UI_SafeTranslateString( "EXE_COD_MULTIPLAYER" ), "1.1x" ), 0 );
	} else {
		if ( !Q_stricmp( cstate.servername, "Auto-Updater" ) ) {
			trap_Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof( downloadName ) );
			if ( *downloadName ) {
				trap_Cvar_VariableStringBuffer( "cl_updateversion", downloadName, sizeof( downloadName ) );
				strcpy( text, va( UI_SafeTranslateString( "EXE_DOWNLOADINGUPDATE" ), downloadName ) );
			} else {
				strcpy( text, va( UI_SafeTranslateString( "EXE_CONNECTINGTO" ), cstate.servername ) );
			}
		} else {
			strcpy( text, va( UI_SafeTranslateString( "EXE_CONNECTINGTO" ), cstate.servername ) );
		}
		Text_PaintCenter( 320, 55, 0, 0.5, colorWhite, text, 0 );
	}

	// display global MOTD at bottom
	Text_PaintCenter( 320, 460, 0, 0.5, colorWhite, Info_ValueForKey( cstate.updateInfoString, "motd" ), 0 );

	// print any server info (server full, bad version, etc)
	// DHM - Nerve :: This now accepts strings up to 256 chars long, and will break them up into multiple lines.
	//					They are also now printed in Yellow for readability.
	if ( cstate.connState < CA_CONNECTED ) {
		const char *s;
		char ps[60];
		int i, len, index = 0, yPrint = 265;
		qboolean neednewline = qfalse;

		s = UI_SafeTranslateString( cstate.messageString );
		len = strlen( s );

		for ( i = 0; i < len; i++, index++ ) {

			// copy to temp buffer
			ps[index] = s[i];

			if ( index > ( CP_LINEWIDTH - 10 ) && i > 0 ) {
				neednewline = qtrue;
			}

			// if out of temp buffer room OR end of string OR it is time to linebreak & we've found a space
			if ( ( index >= 58 ) || ( i == ( len - 1 ) ) || ( neednewline && s[i] == ' ' ) ) {
				ps[index + 1] = '\0';

				Text_PaintCenter( 320, yPrint, 0, 0.5, colorYellow, ps, 0 );

				neednewline = qfalse;
				yPrint += 22;       // next line
				index = -1;         // sigh, for loop will increment to 0
			}
		}

	}

	switch ( cstate.connState ) {
	case CA_CONNECTING:
		s = va( UI_SafeTranslateString( "EXE_AWAITINGCONNECTION" ), cstate.connectPacketCount );
		break;
	case CA_CHALLENGING:
		s = va( UI_SafeTranslateString( "EXE_AWAITINGCHALLENGE" ), cstate.connectPacketCount );
		break;
	case CA_CONNECTED:
		trap_Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof( downloadName ) );
		if ( *downloadName ) {
			UI_DisplayDownloadInfo( downloadName, 320, 55, 0, 0.25 );
			return;
		}
		s = UI_SafeTranslateString( "EXE_AWAITINGGAMESTATE" );
		break;
	default:
		return;
	}


	if ( s && Q_stricmp( cstate.servername,"localhost" ) ) {
		Text_PaintCenter( 320, 85, 0, 0.5, colorWhite, s, 0 );
	}

	// password required / connection rejected information goes here
}


/*
================
cvars
================
*/

vmCvar_t ui_arenasFile;
vmCvar_t ui_allowVote;
vmCvar_t ui_brass;
vmCvar_t ui_marks;

vmCvar_t ui_server1;
vmCvar_t ui_server2;
vmCvar_t ui_server3;
vmCvar_t ui_server4;
vmCvar_t ui_server5;
vmCvar_t ui_server6;
vmCvar_t ui_server7;
vmCvar_t ui_server8;
vmCvar_t ui_server9;
vmCvar_t ui_server10;
vmCvar_t ui_server11;
vmCvar_t ui_server12;
vmCvar_t ui_server13;
vmCvar_t ui_server14;
vmCvar_t ui_server15;
vmCvar_t ui_server16;

vmCvar_t ui_dedicated;
vmCvar_t ui_smallFont;
vmCvar_t ui_bigFont;
vmCvar_t ui_extraBigFont;
vmCvar_t ui_cdkeychecked;

vmCvar_t ui_selectedPlayer;
vmCvar_t ui_netSource;
vmCvar_t ui_menuFiles;
vmCvar_t ui_gameType;
vmCvar_t ui_joinGameType;
vmCvar_t ui_netGameType;
vmCvar_t ui_netGameTypeName;

vmCvar_t ui_newScriptMenu;
vmCvar_t ui_newScriptMenuIndex;
vmCvar_t ui_scriptMenu;
vmCvar_t ui_scriptMenuIndex;
vmCvar_t ui_scriptMenuAllowResponse;
vmCvar_t ui_waitingScriptMenu;
vmCvar_t ui_waitingScriptMenuIndex;
vmCvar_t ui_waitingScriptMenuNoMouse;

// NERVE - SMF - cvars for multiplayer
vmCvar_t ui_mapIndex;
vmCvar_t ui_currentMap;
vmCvar_t ui_currentNetMap;

vmCvar_t ui_browserMaster;
vmCvar_t ui_browserGameType;
vmCvar_t ui_browserSortKey;
vmCvar_t ui_browserShowFull;
vmCvar_t ui_browserShowEmpty;
vmCvar_t ui_browserShowPassword;
vmCvar_t ui_browserShowNoPassword;

vmCvar_t ui_serverStatusTimeOut;

vmCvar_t ui_cmd;

vmCvar_t ui_isSpectator;
// -NERVE - SMF

vmCvar_t ui_hudAlpha;

vmCvar_t cl_languagewarnings;
vmCvar_t cl_languagewarningsaserrors;

/* 0x40036AE8, 56 rows */
cvarTable_t cvarTable[] = {
	{ &ui_arenasFile, "g_arenasFile", "", CVAR_INIT | CVAR_ROM },

	{ &ui_allowVote, "g_allowvote", "1", CVAR_ARCHIVE },

	{ &ui_brass, "cg_brass", "1", CVAR_ARCHIVE },
	{ &ui_marks, "cg_marks", "1", CVAR_ARCHIVE },

	{ &ui_server1, "server1", "", CVAR_ARCHIVE },
	{ &ui_server2, "server2", "", CVAR_ARCHIVE },
	{ &ui_server3, "server3", "", CVAR_ARCHIVE },
	{ &ui_server4, "server4", "", CVAR_ARCHIVE },
	{ &ui_server5, "server5", "", CVAR_ARCHIVE },
	{ &ui_server6, "server6", "", CVAR_ARCHIVE },
	{ &ui_server7, "server7", "", CVAR_ARCHIVE },
	{ &ui_server8, "server8", "", CVAR_ARCHIVE },
	{ &ui_server9, "server9", "", CVAR_ARCHIVE },
	{ &ui_server10, "server10", "", CVAR_ARCHIVE },
	{ &ui_server11, "server11", "", CVAR_ARCHIVE },
	{ &ui_server12, "server12", "", CVAR_ARCHIVE },
	{ &ui_server13, "server13", "", CVAR_ARCHIVE },
	{ &ui_server14, "server14", "", CVAR_ARCHIVE },
	{ &ui_server15, "server15", "", CVAR_ARCHIVE },
	{ &ui_server16, "server16", "", CVAR_ARCHIVE },

	{ &ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE },
	{ &ui_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &ui_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
	{ &ui_extraBigFont, "ui_extraBigFont", "0.55", CVAR_ARCHIVE},
	{ &ui_cdkeychecked, "ui_cdkeychecked", "0", CVAR_ROM },
	{ &ui_selectedPlayer, "cg_selectedPlayer", "0", CVAR_ARCHIVE},
	{ &ui_netSource, "ui_netSource", "0", CVAR_ARCHIVE },
	{ &ui_menuFiles, "ui_menuFiles", "ui_mp/menus.txt", 0 },
	{ &ui_gameType, "ui_gametype", "3", CVAR_ARCHIVE },
	{ &ui_joinGameType, "ui_joinGametype", "0", CVAR_ARCHIVE },
	{ &ui_netGameType, "ui_netGametype", "0", CVAR_ARCHIVE },                 // NERVE - SMF - hardwired for now
	{ &ui_netGameTypeName, "ui_netGametypeName", "", CVAR_ARCHIVE },

	{ &ui_newScriptMenu, "ui_newScriptMenu", "", CVAR_ROM },
	{ &ui_newScriptMenuIndex, "ui_newScriptMenuIndex", "-1", CVAR_ROM },
	{ &ui_scriptMenu, "ui_scriptMenu", "", CVAR_ROM },
	{ &ui_scriptMenuIndex, "ui_scriptMenuIndex", "-1", CVAR_ROM },
	{ &ui_scriptMenuAllowResponse, "ui_scriptMenuAllowResponse", "1", CVAR_ROM },
	{ &ui_waitingScriptMenu, "ui_waitingScriptMenu", "", CVAR_ROM },
	{ &ui_waitingScriptMenuIndex, "ui_waitingScriptMenuIndex", "-1", CVAR_ROM },
	{ &ui_waitingScriptMenuNoMouse, "ui_waitingScriptMenuNoMouse", "0", CVAR_ROM },

	// NERVE - SMF - multiplayer cvars
	{ &ui_mapIndex, "ui_mapIndex", "0", CVAR_ARCHIVE },
	{ &ui_currentMap, "ui_currentMap", "0", CVAR_ARCHIVE },
	{ &ui_currentNetMap, "ui_currentNetMap", "0", CVAR_ARCHIVE },

	{ &ui_browserMaster, "ui_browserMaster", "0", CVAR_ARCHIVE },
	{ &ui_browserGameType, "ui_browserGameType", "0", CVAR_ARCHIVE },
	{ &ui_browserSortKey, "ui_browserSortKey", "4", CVAR_ARCHIVE },
	{ &ui_browserShowFull, "ui_browserShowFull", "1", CVAR_ARCHIVE },
	{ &ui_browserShowEmpty, "ui_browserShowEmpty", "1", CVAR_ARCHIVE },
	{ &ui_browserShowPassword, "ui_browserShowPassword", "1", CVAR_ARCHIVE },
	{ &ui_browserShowNoPassword, "ui_browserShowNoPassword", "1", CVAR_ARCHIVE },

	{ &ui_serverStatusTimeOut, "ui_serverStatusTimeOut", "7000", CVAR_ARCHIVE},

	{ &ui_cmd, "ui_cmd", "", 0 },

	{ &ui_isSpectator, "ui_isSpectator", "1", 0 },
	// -NERVE - SMF

	{ &ui_hudAlpha, "cg_hudAlpha", "1.0", CVAR_ARCHIVE },

	{ &cl_languagewarnings, "cl_languagewarnings", "0", 0 },
	{ &cl_languagewarningsaserrors, "cl_languagewarningsaserrors", "0", 0 }
};

int cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );  /* 0x40036E68 */


/*
=================
UI_RegisterCvars  0x4000E940
=================
*/
void UI_RegisterCvars( void ) {
	int i;
	cvarTable_t *cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
	}
}

/*
=================
UI_UpdateCvars  0x4000E980
=================
*/
void UI_UpdateCvars( void ) {
	int i;
	cvarTable_t *cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Update( cv->vmCvar );
	}
}

// NERVE - SMF
/*
=================
ArenaServers_StopRefresh  0x4000E9B0
=================
*/
static void UI_StopServerRefresh( void ) {
	int count;

	if ( !uiInfo.serverStatus.refreshActive ) {
		// not currently refreshing
		return;
	}
	uiInfo.serverStatus.refreshActive = qfalse;
	Com_Printf( "%d servers listed in browser with %d players.\n",
				uiInfo.serverStatus.numDisplayServers,
				uiInfo.serverStatus.numPlayersOnServers );
	count = trap_LAN_GetServerCount( ui_netSource.integer );
	if ( count - uiInfo.serverStatus.numDisplayServers > 0 ) {
		// TTimo - used to be about cl_maxping filtering, that was Q3 legacy, RTCW browser has much more filtering options
		Com_Printf( "%d servers not listed (filtered out by game browser settings)\n",
					count - uiInfo.serverStatus.numDisplayServers );
	}

}

/*
=================
UI_DoServerRefresh  0x4000EA00
=================
*/
static void UI_DoServerRefresh( void ) {
	qboolean wait = qfalse;

	if ( !uiInfo.serverStatus.refreshActive ) {
		return;
	}
	if ( ui_netSource.integer != AS_FAVORITES ) {
		if ( ui_netSource.integer == AS_LOCAL ) {
			if ( !trap_LAN_GetServerCount( ui_netSource.integer ) ) {
				wait = qtrue;
			}
		} else {
			if ( trap_LAN_GetServerCount( ui_netSource.integer ) < 0 ) {
				wait = qtrue;
			}
		}
	}

	if ( uiInfo.uiDC.realTime < uiInfo.serverStatus.refreshtime ) {
		if ( wait ) {
			return;
		}
	}

	UI_UpdateDisplayServers();

	// if still trying to retrieve pings
	if ( trap_LAN_UpdateVisiblePings( ui_netSource.integer ) ) {
		uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
	} else if ( !wait ) {
		// get the last servers in the list
		UI_BuildServerDisplayList( 2 );
		// stop the refresh
		UI_StopServerRefresh();
	}
	//
	UI_BuildServerDisplayList( qfalse );
}

/*
=================
UI_StartServerRefresh  0x4000EA90
=================
*/
static void UI_StartServerRefresh( qboolean full ) {
	char    *ptr;

	qtime_t q;
	trap_RealTime( &q );
	trap_Cvar_Set( va( "ui_lastServerRefresh_%i", ui_netSource.integer ), va( "%s %i, %i   %i:%02i", UI_SafeTranslateString( MonthAbbrev[q.tm_mon] ),q.tm_mday, 1900 + q.tm_year,q.tm_hour,q.tm_min ) );

	if ( !full ) {
		UI_UpdatePendingPings();
		return;
	}

	uiInfo.serverStatus.refreshActive = qtrue;
	uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 1000;
	// clear number of displayed servers
	UI_ClearDisplayedServers();
	// mark all servers as visible so we store ping updates for them
	trap_ClearDisplayedServers( ui_netSource.integer, -1, qtrue );
	// reset all the pings
	trap_LAN_ResetPings( ui_netSource.integer );
	//
	if ( ui_netSource.integer == AS_LOCAL ) {
		trap_Cmd_ExecuteText( EXEC_NOW, "localservers\n" );
		uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
		return;
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 5000;
	if ( ui_netSource.integer == AS_GLOBAL ) {
		ptr = UI_Cvar_VariableString( "debug_protocol" );
		if ( strlen( ptr ) ) {
			trap_Cmd_ExecuteText( EXEC_NOW, va( "globalservers %d %s full empty\n", 0, ptr ) );
		} else {
			trap_Cmd_ExecuteText( EXEC_NOW, va( "globalservers %d %d full empty\n", 0, (int)trap_Cvar_VariableValue( "protocol" ) ) );
		}
	}
}
// -NERVE - SMF

/* ---- UI_ConfigString  0x4000EC00 -- the DC's configString ---- */
const char *UI_ConfigString( int index ) {
	static char buff[MAX_STRING_CHARS];             /* 0x4005AD88 */

	trap_GetConfigString( index, buff, sizeof( buff ) );
	return buff;
}

/*
=================
UI_SafeTranslateString  0x4000EC20

The DC's safeTranslateString: the reference's translation, or the reference
itself (marked ^1UNLOCALIZED when cl_languagewarnings is on).
=================
*/
const char *UI_SafeTranslateString( const char *reference ) {
	static char buffer[1024];                       /* 0x40052560 */
	const char *s;

	s = trap_SE_TranslateReference( reference );
	if ( s ) {
		return s;
	}

	if ( cl_languagewarnings.integer ) {
		if ( cl_languagewarningsaserrors.integer ) {
			Com_Error( ERR_LOCALIZATION, "Could not translate ui string \"%s\"", reference );
		} else {
			Com_Printf( "^3WARNING: Could not translate ui string \"%s\"\n", reference );
		}
		strcpy( buffer, "^1UNLOCALIZED(^7" );
		strcat( buffer, reference );
		strcat( buffer, "^1)^7" );
		return buffer;
	}

	strcpy( buffer, reference );
	return buffer;
}

/*
 * The unit's tail: four functions the Mac symbols do not name.  0x4000ED20
 * and 0x4000ED30 are one `retn` each; 0x4000ED40 returns its float argument
 * on the FPU stack; 0x4000ED50 truncates its float argument less 0.5 - 2^-30
 * to an int.  Nothing in the DLL calls any of them.
 */
void nullsub_6( void ) {
}

void nullsub_7( void ) {
}

float sub_4000ED40( float a1 ) {
	return a1;
}

int sub_4000ED50( float a1 ) {
	return (int)( a1 - 0.4999999990686774 );
}
