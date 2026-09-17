/*
 * @fidelity: partial
 *
 * ui_local.h -- local definitions for the multiplayer user interface module
 * (ui_mp_x86.dll, CoD 1.1, imagebase 0x40000000).
 *
 * RTCW's ui/ui_local.h is the structural model; each layout below carries
 * its retail address.
 *
 * The one big object is `uiInfo` at 0x401C3CE0.  Mac-named members:
 * uiInfo.uiDC.yscale at 0x401C3DC0 is DC+0xE0, uiInfo.uiDC.whiteShader at
 * 0x401E20B0 is DC+0x1E3D0, mapCount at 0x401E4610, serverStatusInfo.numLines
 * at 0x401ED810, activeFont at 0x401EECF0.  The other members follow from the
 * strides the code walks it with:
 * characterList 16, teamList 44 (by the 0xB04 between teamCount and
 * numGameTypes), gameTypes 8, playerNames 32, mapList 0xA4 (`imul eax, 0A4h`
 * throughout UI_LoadArenas 0x40007400), modList 8, displayServers 4 and
 * pendingServerStatus 0x8C4 (the memset at 0x4000C040).  Nothing memsets the
 * whole record; its size is activeFont + 4 = 0x2B014, and the next global
 * (vmCvar_t ui_dedicated at 0x401EED00) caps it at 0x2B020, so at most three
 * ints could follow activeFont -- RTCW has none.
 *
 * There is NO uiStatic_t / `uis` in this DLL.  RTCW's ui_atoms.c defines one,
 * but nothing in CoD's ui_atoms.c reads it (UI_AdjustFrom640 0x40006DB0 and
 * UI_CursorInRect 0x40007130 take xscale/yscale/cursorx/cursory from
 * uiInfo.uiDC, as RTCW's do) and no data symbol of its shape survives, so it is
 * not declared here.
 *
 * Ranges named `unknown_0x...` have no known CoD 1.1 access, or a known role
 * but no recovered member name.  A name taken from RTCW's ordering without a
 * CoD 1.1 access is marked as such.
 */

#ifndef __UI_LOCAL_H__
#define __UI_LOCAL_H__

#include "../universal/q_shared.h"
/* ui_public.h owns the engine/ui ABI: the trap numbers and wrappers,
 * uiClientState_t, glconfig_t, uiMenuCommand_t and the vmMain commands. */
#include "ui_public.h"
#include "ui_shared.h"

/*
 * These belong in q_shared.h; universal/q_shared.h does not carry them yet,
 * so the ui module defines them here, as cg_local.h and g_local.h do for theirs.
 */
#define MAX_CLIENTS             64      /* playerNames/teamNames rows, teamClientNums (UI_BuildPlayerList 0x40008ED0) */
#define MAX_NAME_LENGTH         32      /* playerNames stride (0x40008F5D) */
#define MAX_INFO_STRING         1024
#define BIGCHAR_WIDTH           16
#define BIGCHAR_HEIGHT          16

/* Cbuf_ExecuteText's exec_when; UI_StartDemoLoop (0x40006CA0) pushes 2. */
#define EXEC_NOW                0
#define EXEC_INSERT             1
#define EXEC_APPEND             2

#define S_COLOR_RED             "^1"
#define S_COLOR_YELLOW          "^3"

/* universal/q_shared.c; this DLL links its own copy (0x4002D70C, 0x4002D7DC). */
extern const vec4_t colorBlack;
extern const vec4_t colorWhite;

/*
=============================================================================

        UI-PRIVATE TYPES

=============================================================================
*/

/* ui_gameinfo.c: `int[64]` at 0x40050C58, and UI_ParseInfos sizes the arena
   number with va( "%d", 64 ) (0x4000725E); the .arena read buffer is 0x2000
   (UI_LoadArenasFromFile 0x400072F0). */
#define MAX_ARENAS              64
#define MAX_ARENAS_TEXT         8192

#define MAX_HEADS               64      /* characterList: 0x400 bytes of 16 */
#define MAX_ALIASES             64      /* aliasList: 0x300 bytes of 12 */
#define MAX_TEAMS               64      /* teamList: 0xB00 bytes of 44 */
/* 32, not RTCW's 16: gameTypes[] is 0x100 bytes of 8, and UI_GetGameTypesList
   (0x4000CF80) stops at 32 with "Only loading the first 31". */
#define MAX_GAMETYPES           32
#define MAX_MAPS                128     /* UI_LoadArenas 0x4000765D */
#define TEAM_MEMBERS            5
#define MAX_PINGREQUESTS        16      /* RTCW's; the 0x450-byte block it sits in is unreferenced */
#define MAX_ADDRESSLENGTH       64
#define MAX_DISPLAY_SERVERS     2048    /* displayServers: 0x2000 bytes of 4 */
#define MAX_SERVERSTATUS_LINES  128     /* lines: 0x800 bytes of 16 */
#define MAX_SERVERSTATUS_TEXT   1024
#define MAX_SERVERSTATUSREQUESTS 16     /* pendingServerStatus: 4 + 16 x 140 (0x4000C040 memset 0x8C4) */
#define MAX_FOUNDPLAYER_SERVERS 16      /* UI_BuildFindPlayerList caps at 15 (0x4000C1F0) */
#define MAX_MODS                64      /* UI_LoadMods 0x40009EBC */
#define MAX_DEMOS               256     /* UI_LoadDemos 0x4000A030 */
#define MAX_MOVIES              256     /* UI_LoadMovies 0x40009F30 */

/* UI_FeederItemImage (0x4000CB70) registers imageName into headImage when it
   is -1, exactly as RTCW's does: name +0, imageName +4, headImage +8. */
typedef struct {
	const char  *name;          /* +0x00 UI_FeederItemText 0x4000C7DA */
	const char  *imageName;     /* +0x04 UI_FeederItemImage 0x4000CBA8 */
	qhandle_t   headImage;      /* +0x08 UI_FeederItemImage 0x4000CB9F */
	qboolean    female;         /* +0x0C UI_FeederSelection 0x4000CC70 */
} characterInfo;
UI_ASSERT_SIZE( characterInfo, 16 );

/* No CoD 1.1 access into the array; only aliasCount is written (_UI_Init
   0x4000D610).  RTCW's record fills the 0x300 bytes exactly. */
typedef struct {
	const char  *name;
	const char  *ai;
	const char  *action;
} aliasInfo;
UI_ASSERT_SIZE( aliasInfo, 12 );

/* Likewise: only teamCount is written (_UI_Init 0x4000D5FC); 64 of RTCW's
   44-byte record is what lies between it and numGameTypes. */
typedef struct {
	const char  *teamName;
	const char  *imageName;
	const char  *teamMembers[TEAM_MEMBERS];
	qhandle_t   teamIcon;
	qhandle_t   teamIcon_Metal;
	qhandle_t   teamIcon_Name;
	int         cinematic;
} teamInfo;
UI_ASSERT_SIZE( teamInfo, 44 );

/* gameType is the script name UI_LoadArenas matches .arena "gametype" tokens
   against (0x400075F0); gtEnum holds the display name from
   maps/mp/gametypes/<name>.txt (UI_GetGameTypesList 0x4000D0C4) -- a string,
   kept under RTCW's name because the slot is RTCW's. */
typedef struct {
	const char  *gameType;      /* +0x00 */
	const char  *gtEnum;        /* +0x04 */
} gameTypeInfo;
UI_ASSERT_SIZE( gameTypeInfo, 8 );

/*
 * 0xA4 bytes (UI_LoadArenas' imul).  RTCW's record less the three NERVE
 * respawn fields, with timeToBeat sized to MAX_GAMETYPES=32 -- which is what
 * makes levelShot land at +0x9C.
 */
typedef struct {
	const char  *mapName;       /* +0x00 UI_LoadArenas 0x4000751B */
	const char  *mapLoadName;   /* +0x04 0x400074F2 */
	const char  *imageName;     /* +0x08 0x40007561 */
	const char  *opponentName;  /* +0x0C no CoD access; RTCW's name */
	int         teamMembers;    /* +0x10 no CoD access; RTCW's name */
	int         typeBits;       /* +0x14 0x4000759A, -1 when no "gametype" key */
	int         cinematic;      /* +0x18 0x400074D5 */
	int         timeToBeat[MAX_GAMETYPES];  /* +0x1C no CoD access; RTCW's name */
	qhandle_t   levelShot;      /* +0x9C 0x4000752C */
	qboolean    active;         /* +0xA0 UI_MapCountByGameType 0x4000B5DC */
} mapInfo;
UI_ASSERT_SIZE( mapInfo, 0xA4 );

typedef struct {
	char    adrstr[MAX_ADDRESSLENGTH];
	int     start;
} pinglist_t;
UI_ASSERT_SIZE( pinglist_t, 68 );

/*
 * 0x289C bytes at uiInfo+0x26554.  Offsets below are relative to the record;
 * the reads are UI_StartServerRefresh (refreshtime 0x4000EB75,
 * refreshActive 0x4000EB25), UI_ServersSort (sortKey 0x40009DF1),
 * UI_FeederSelection (currentServer 0x4000CCF8), UI_InsertServerIntoDisplayList
 * (displayServers/numDisplayServers 0x4000B6F0) and UI_BuildServerDisplayList
 * (motdLen/motdWidth 0x4000B8AB, motd 0x4000B842).
 */
typedef struct serverStatus_s {
	pinglist_t  pingList[MAX_PINGREQUESTS]; /* +0x0000 no CoD access */
	int         numqueriedservers;          /* +0x0440 no CoD access */
	int         currentping;                /* +0x0444 no CoD access */
	int         nextpingtime;               /* +0x0448 no CoD access */
	int         maxservers;                 /* +0x044C no CoD access */
	int         refreshtime;                /* +0x0450 */
	int         numServers;                 /* +0x0454 no CoD access */
	int         sortKey;                    /* +0x0458 */
	int         sortDir;                    /* +0x045C UI_ServersQsortCompare 0x40009DC2 */
	int         lastCount;                  /* +0x0460 no CoD access */
	qboolean    refreshActive;              /* +0x0464 */
	int         currentServer;              /* +0x0468 */
	int         displayServers[MAX_DISPLAY_SERVERS];    /* +0x046C */
	int         numDisplayServers;          /* +0x246C */
	int         numPlayersOnServers;        /* +0x2470 UI_BuildServerDisplayList 0x4000B8D9 */
	int         nextDisplayRefresh;         /* +0x2474 0x4000BB84 */
	int         nextSortTime;               /* +0x2478 no CoD access */
	qhandle_t   currentServerPreview;       /* +0x247C UI_DrawNetMapPreview 0x40008B60 */
	int         currentServerCinematic;     /* +0x2480 UI_StopCinematic 0x4000D29F */
	int         motdLen;                    /* +0x2484 */
	int         motdWidth;                  /* +0x2488 */
	int         motdPaintX;                 /* +0x248C */
	int         motdPaintX2;                /* +0x2490 */
	int         motdOffset;                 /* +0x2494 */
	int         motdTime;                   /* +0x2498 */
	char        motd[MAX_STRING_CHARS];     /* +0x249C */
} serverStatus_t;
UI_ASSERT_SIZE( serverStatus_t, 0x289C );

/* 140 bytes: UI_BuildFindPlayerList steps 140 (0x4000C040), reads startTime
   at +0x80 and valid at +0x88 and copies the hostname into name at +0x40. */
typedef struct {
	char        adrstr[MAX_ADDRESSLENGTH];  /* +0x00 */
	char        name[MAX_ADDRESSLENGTH];    /* +0x40 */
	int         startTime;                  /* +0x80 */
	int         serverNum;                  /* +0x84 no CoD access; RTCW's name */
	qboolean    valid;                      /* +0x88 */
} pendingServer_t;
UI_ASSERT_SIZE( pendingServer_t, 140 );

typedef struct {
	int             num;                                    /* +0x000 */
	pendingServer_t server[MAX_SERVERSTATUSREQUESTS];       /* +0x004 */
} pendingServerStatus_t;
UI_ASSERT_SIZE( pendingServerStatus_t, 0x8C4 );

/* 0xD04 bytes: lines at +0x40 (UI_FeederItemText 0x4000C9EC), numLines at
   +0xD00 (the Mac-named uiInfo.serverStatusInfo.numLines, 0x401ED810). */
typedef struct {
	char        address[MAX_ADDRESSLENGTH];             /* +0x000 */
	char        *lines[MAX_SERVERSTATUS_LINES][4];      /* +0x040 */
	char        text[MAX_SERVERSTATUS_TEXT];            /* +0x840 */
	char        pings[MAX_CLIENTS * 3];                 /* +0xC40 no CoD access; RTCW's name */
	int         numLines;                               /* +0xD00 */
} serverStatusInfo_t;
UI_ASSERT_SIZE( serverStatusInfo_t, 0xD04 );

/* UI_LoadMods (0x40009E10) fills both from the "$modlist" file list. */
typedef struct {
	const char  *modName;       /* +0x00 */
	const char  *modDescr;      /* +0x04 */
} modInfo_t;
UI_ASSERT_SIZE( modInfo_t, 8 );

/*
 * uiInfo_t -- 0x2B014 bytes at 0x401C3CE0.  Offsets are from the record
 * start.  The address on each member is the one that reads or writes it;
 * where several do, the first in address order.
 */
typedef struct {
	displayContextDef_t uiDC;               /* +0x00000 _UI_Init 0x4000D3A4 fills it slot by slot */

	int         newHighScoreTime;           /* +0x1E3E0 UI_OwnerDrawVisible 0x40009875 */
	int         newBestTime;                /* +0x1E3E4 0x400098CA */
	int         showPostGameTime;           /* +0x1E3E8 no CoD access; RTCW's name */
	qboolean    newHighScore;               /* +0x1E3EC no CoD access; RTCW's name */
	qboolean    demoAvailable;              /* +0x1E3F0 0x400098DF */
	qboolean    soundHighScore;             /* +0x1E3F4 UI_ShowPostGame 0x40007AAB */

	int         characterCount;             /* +0x1E3F8 UI_FeederCount 0x4000C5C1 */
	int         botIndex;                   /* +0x1E3FC no CoD access; RTCW's name */
	characterInfo characterList[MAX_HEADS]; /* +0x1E400 */

	int         aliasCount;                 /* +0x1E800 _UI_Init 0x4000D610 */
	aliasInfo   aliasList[MAX_ALIASES];     /* +0x1E804 */

	int         teamCount;                  /* +0x1EB04 _UI_Init 0x4000D5FC */
	teamInfo    teamList[MAX_TEAMS];        /* +0x1EB08 */

	int         numGameTypes;               /* +0x1F608 UI_LoadArenas 0x400075E5 */
	gameTypeInfo gameTypes[MAX_GAMETYPES];  /* +0x1F60C 0x400075F0 */

	int         numJoinGameTypes;           /* +0x1F70C UI_DrawJoinGameType 0x400088BE */
	gameTypeInfo joinGameTypes[MAX_GAMETYPES];  /* +0x1F710 UI_GetGameTypesList 0x4000CFC3 */

	int         redBlue;                    /* +0x1F810 no CoD access; RTCW's name */
	int         playerCount;                /* +0x1F814 UI_BuildPlayerList 0x40008F34 */
	int         myTeamCount;                /* +0x1F818 UI_OwnerDrawVisible 0x400097F4 */
	int         teamIndex;                  /* +0x1F81C UI_FeederSelection 0x4000CE0F */
	int         playerRefresh;              /* +0x1F820 UI_FeederCount 0x4000C663 */
	int         playerIndex;                /* +0x1F824 UI_RunMenuScript 0x4000AFC7 */
	int         playerNumber;               /* +0x1F828 UI_BuildPlayerList 0x40008F02 */
	qboolean    teamLeader;                 /* +0x1F82C UI_OwnerDrawVisible 0x400097E6 */
	char        playerNames[MAX_CLIENTS][MAX_NAME_LENGTH];  /* +0x1F830 UI_BuildPlayerList 0x40008F5D */
	char        teamNames[MAX_CLIENTS][MAX_NAME_LENGTH];    /* +0x20030 UI_RunMenuScript 0x4000B230 */
	int         teamClientNums[MAX_CLIENTS];                /* +0x20830 UI_OwnerDrawVisible 0x400097FC */

	int         mapCount;                   /* +0x20930 UI_LoadArenas 0x4000743B */
	mapInfo     mapList[MAX_MAPS];          /* +0x20934 */

	/* RTCW has tierCount, tierList[] and skillIndex here; CoD keeps one int
	   between mapList and modList and nothing reads it. */
	int         unknown_0x25B34;            /* +0x25B34 */

	modInfo_t   modList[MAX_MODS];          /* +0x25B38 UI_LoadMods 0x40009E81 */
	int         modCount;                   /* +0x25D38 0x40009E40 */
	int         modIndex;                   /* +0x25D3C UI_FeederSelection 0x4000CE2F */

	const char  *demoList[MAX_DEMOS];       /* +0x25D40 UI_LoadDemos 0x4000A0C4 */
	int         demoCount;                  /* +0x26140 0x4000A01E */
	int         demoIndex;                  /* +0x26144 UI_FeederSelection 0x4000CE8A */

	const char  *movieList[MAX_MOVIES];     /* +0x26148 UI_LoadMovies 0x40009F84 */
	int         movieCount;                 /* +0x26548 0x40009F2A */
	int         movieIndex;                 /* +0x2654C UI_DrawPreviewCinematic 0x40008943 */
	int         previewMovie;               /* +0x26550 0x40008930; _UI_Init seeds -1 */

	serverStatus_t serverStatus;            /* +0x26554 */

	/* for the showing the status of a server */
	char        serverStatusAddress[MAX_ADDRESSLENGTH]; /* +0x28DF0 UI_BuildServerStatus 0x4000C568 */
	serverStatusInfo_t serverStatusInfo;    /* +0x28E30 */
	int         nextServerStatusRefresh;    /* +0x29B34 UI_BuildServerStatus 0x4000C515 */

	/* to retrieve the status of server to find a player */
	pendingServerStatus_t pendingServerStatus;  /* +0x29B38 UI_BuildFindPlayerList 0x4000C097 */
	char        findPlayerName[MAX_STRING_CHARS];   /* +0x2A3FC 0x4000C09E */
	char        foundPlayerServerAddresses[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];  /* +0x2A7FC UI_RunMenuScript 0x4000AD18 */
	char        foundPlayerServerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];      /* +0x2ABFC UI_FeederItemText 0x4000CA38 */
	int         currentFoundPlayerServer;   /* +0x2AFFC UI_BuildFindPlayerList 0x4000C0AE */
	int         numFoundPlayerServers;      /* +0x2B000 0x4000C0A8 */
	int         nextFindPlayerRefresh;      /* +0x2B004 0x4000C06D */

	/* RTCW has currentCrosshair and startPostGameTime here; CoD keeps one
	   int and nothing reads it. */
	int         unknown_0x2B008;            /* +0x2B008 */
	sfxHandle_t newHighScoreSound;          /* +0x2B00C UI_OwnerDrawVisible 0x400098A4 */

	int         activeFont;                 /* +0x2B010 Text_SetActiveFont 0x40007A74 */
} uiInfo_t;
UI_ASSERT_SIZE( uiInfo_t, 0x2B014 );

extern uiInfo_t uiInfo;                     /* 0x401C3CE0 */

/* ui_main.c's cvar table row: UI_RegisterCvars (0x4000E940) walks 16-byte
   rows from 0x40036AE8, 56 of them (0x40036E68), handing vmCvar, cvarName,
   defaultString and cvarFlags to trap_Cvar_Register in that order. */
typedef struct {
	vmCvar_t    *vmCvar;
	char        *cvarName;
	char        *defaultString;
	int         cvarFlags;
} cvarTable_t;
UI_ASSERT_SIZE( cvarTable_t, 16 );

/*
 * The 56 cvars of that table, in table order, each with the vmCvar_t the row
 * points at.  Variable names are RTCW's where RTCW registers the same cvar;
 * the rest are spelled after the cvar itself, as cg_local.h does for
 * cl_serverloadmap.  Defaults and flags are the table's.
 */
extern vmCvar_t ui_arenasFile;              /* 0x401C2D20  "g_arenasFile"  ""   CVAR_INIT|CVAR_ROM */
extern vmCvar_t ui_allowVote;               /* 0x401C0C80  "g_allowvote"   "1"  CVAR_ARCHIVE */
extern vmCvar_t ui_brass;                   /* 0x401C1B20  "cg_brass"      "1"  CVAR_ARCHIVE (RTCW: ui_brassTime/"cg_brassTime") */
extern vmCvar_t ui_marks;                   /* 0x401C1580  "cg_marks"      "1"  CVAR_ARCHIVE */
extern vmCvar_t ui_server1;                 /* 0x401C2F60 */
extern vmCvar_t ui_server2;                 /* 0x401C1220 */
extern vmCvar_t ui_server3;                 /* 0x401C2540 */
extern vmCvar_t ui_server4;                 /* 0x401C0B60 */
extern vmCvar_t ui_server5;                 /* 0x401EF720 */
extern vmCvar_t ui_server6;                 /* 0x401EFBA0 */
extern vmCvar_t ui_server7;                 /* 0x401C21E0 */
extern vmCvar_t ui_server8;                 /* 0x401EF4E0 */
extern vmCvar_t ui_server9;                 /* 0x401C2AE0 */
extern vmCvar_t ui_server10;                /* 0x401EF180 */
extern vmCvar_t ui_server11;                /* 0x401EF2A0 */
extern vmCvar_t ui_server12;                /* 0x401EFA80 */
extern vmCvar_t ui_server13;                /* 0x401C31A0 */
extern vmCvar_t ui_server14;                /* 0x401C29C0 */
extern vmCvar_t ui_server15;                /* 0x401C3BC0 */
extern vmCvar_t ui_server16;                /* 0x401C2E40  "server1".."server16"  ""  CVAR_ARCHIVE */
extern vmCvar_t ui_dedicated;               /* 0x401EED00  "0"     CVAR_ARCHIVE */
extern vmCvar_t ui_smallFont;               /* 0x401EEE20  "0.25"  CVAR_ARCHIVE */
extern vmCvar_t ui_bigFont;                 /* 0x401C1E80  "0.4"   CVAR_ARCHIVE */
extern vmCvar_t ui_extraBigFont;            /* 0x401C2420  "0.55"  CVAR_ARCHIVE */
extern vmCvar_t ui_cdkeychecked;            /* 0x401C3AA0  "0"     CVAR_ROM */
extern vmCvar_t ui_selectedPlayer;          /* 0x401C2300  "cg_selectedPlayer"  "0"  CVAR_ARCHIVE */
extern vmCvar_t ui_netSource;               /* 0x401C1460  "0"     CVAR_ARCHIVE */
extern vmCvar_t ui_menuFiles;               /* 0x401EF600  "ui_mp/menus.txt"  0 */
extern vmCvar_t ui_gameType;                /* 0x401C20C0  "ui_gametype"      "3"  CVAR_ARCHIVE */
extern vmCvar_t ui_joinGameType;            /* 0x401C3740  "ui_joinGametype"  "0"  CVAR_ARCHIVE */
extern vmCvar_t ui_netGameType;             /* 0x401C3620  "ui_netGametype"   "0"  CVAR_ARCHIVE */
extern vmCvar_t ui_netGameTypeName;         /* 0x401C1C40  "ui_netGametypeName"  ""  CVAR_ARCHIVE */
extern vmCvar_t ui_newScriptMenu;           /* 0x401C0A40  ""      CVAR_ROM */
extern vmCvar_t ui_newScriptMenuIndex;      /* 0x401C1100  "-1"    CVAR_ROM */
extern vmCvar_t ui_scriptMenu;              /* 0x401C17C0  ""      CVAR_ROM */
extern vmCvar_t ui_scriptMenuIndex;         /* 0x401C1FA0  "-1"    CVAR_ROM */
extern vmCvar_t ui_scriptMenuAllowResponse; /* 0x401C1D60  "1"     CVAR_ROM */
extern vmCvar_t ui_waitingScriptMenu;       /* 0x401C16A0  ""      CVAR_ROM */
extern vmCvar_t ui_waitingScriptMenuIndex;  /* 0x401EFDE0  "-1"    CVAR_ROM */
extern vmCvar_t ui_waitingScriptMenuNoMouse;/* 0x401EEF40  "0"     CVAR_ROM */
extern vmCvar_t ui_mapIndex;                /* 0x401C2C00  "0"     CVAR_ARCHIVE */
extern vmCvar_t ui_currentMap;              /* 0x401C3500  "0"     CVAR_ARCHIVE */
extern vmCvar_t ui_currentNetMap;           /* 0x401C28A0  "0"     CVAR_ARCHIVE */
extern vmCvar_t ui_browserMaster;           /* 0x401C32C0  "0"     CVAR_ARCHIVE */
extern vmCvar_t ui_browserGameType;         /* 0x401C0FE0  "0"     CVAR_ARCHIVE */
extern vmCvar_t ui_browserSortKey;          /* 0x401C3080  "4"     CVAR_ARCHIVE */
extern vmCvar_t ui_browserShowFull;         /* 0x401EF3C0  "1"     CVAR_ARCHIVE */
extern vmCvar_t ui_browserShowEmpty;        /* 0x401C3980  "1"     CVAR_ARCHIVE */
extern vmCvar_t ui_browserShowPassword;     /* 0x401EFCC0  "1"     CVAR_ARCHIVE */
extern vmCvar_t ui_browserShowNoPassword;   /* 0x401C3860  "1"     CVAR_ARCHIVE */
extern vmCvar_t ui_serverStatusTimeOut;     /* 0x401EF060  "7000"  CVAR_ARCHIVE */
extern vmCvar_t ui_cmd;                     /* 0x401C33E0  ""      0 */
extern vmCvar_t ui_isSpectator;             /* 0x401C2780  "1"     0 */
extern vmCvar_t ui_hudAlpha;                /* 0x401EF840  "cg_hudAlpha"  "1.0"  CVAR_ARCHIVE */
extern vmCvar_t cl_languagewarnings;        /* 0x401C1A00  "0"     0 */
extern vmCvar_t cl_languagewarningsaserrors;/* 0x401C18E0  "0"     0 */

/*
=============================================================================

        PROTOTYPES

    Grouped by the source file that defines them, in address order -- RTCW's
    ui_local.h is laid out the same way.

=============================================================================
*/

/*
 * forward: universal/q_shared.c and universal/q_parse.c.  The ui DLL compiles
 * its own copy of both, exactly as cgame does; ui_shared.h already declares
 * the handful its unit needs and these are the rest.
 */
void        Com_DPrintf( const char *fmt, ... );
int QDECL   Com_sprintf( char *dest, int size, const char *fmt, ... );
char       *Q_CleanStr( char *string );
char       *Info_ValueForKey( const char *s, const char *key );
void        Info_SetValueForKey( char *s, const char *key, const char *value );
void        Com_BeginParseSession( const char *name );
void        Com_EndParseSession( void );
char       *Com_Parse( char **data_p );

//
// ui_main_mp.c
//
// 0x400076A0 .. 0x4000ED76.  Statics stay in the unit; these are the externs.
//
/* forward: universal/q_shared.c, the rest of what this unit calls. */
float       Com_Clamp( float min, float max, float value );
char       *Com_SkipPath( char *pathname );
void        Com_StripFilename( char *in, char *out );
char       *Q_strupr( char *s1 );
extern const vec4_t colorRed;                       /* 0x4002D71C */
extern const vec4_t colorYellow;                    /* 0x4002D75C */
extern const vec4_t colorLtGrey;                    /* 0x4002D7EC */

int         vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 );
void        AssetCache( void );                         /* 0x400077F0 */
void        _UI_DrawSides( float x, float y, float w, float h, float size );                 /* 0x40007890 */
void        _UI_DrawTopBottom( float x, float y, float w, float h, float size );             /* 0x40007910 */
void        _UI_DrawRect( float x, float y, float width, float height, float size, const float *color ); /* 0x40007990 */
fontInfo_t *UI_GetFontInfo( int font, float scale );   /* 0x400079E0 */
void        Text_SetActiveFont( int font );             /* 0x40007A70 */
void        UI_ShowPostGame( qboolean newHigh );        /* 0x40007A80 */
void        UI_DrawCenteredPic( qhandle_t image, int w, int h );    /* 0x40007AC0 */
void        _UI_Refresh( int realtime );                /* 0x40007B20 */
void        _UI_Shutdown( void );                       /* 0x40007C20 */
char       *getMenuBuffer( const char *filename );      /* 0x40007C30 */
qboolean    Asset_Parse( int handle, int imageTrack );  /* 0x40007CC0 */
void        Font_Report( void );                        /* 0x400081E0 */
void        UI_Report( void );                          /* 0x40008230 */
qboolean    UI_ParseMenu( const char *menuFile, int imageTrack );   /* 0x40008240 */
qboolean    Load_ScriptMenu( const char *menuname, int imageTrack );/* 0x40008330 */
qboolean    Load_Menu( int handle, int imageTrack );    /* 0x40008480 */
void        UI_LoadMenus( const char *menuFile, qboolean reset, int imageTrack );   /* 0x400085B0 */
void        UI_Load( void );                            /* 0x400086C0 */
const char *UI_GetMapDisplayName( const char *mapname );            /* 0x40008CB0 */
const char *UI_GetGameTypeDisplayName( const char *gametype );      /* 0x40008D00 */
void        UI_ServersSort( int column, qboolean force );           /* 0x40009DE0 */
void        WM_setItemPic( char *name, const char *shader );        /* 0x4000A110 */
void        WM_setVisibility( char *name, qboolean show );          /* 0x4000A140 */
qboolean    UI_CheckExecKey( int key );                 /* 0x4000A170 */
void        UI_VerifyLanguage( void );                  /* 0x4000A3B0 */
void        UI_GetGameTypesList( void );                /* 0x4000CF80 */
void        _UI_Init( void );                           /* 0x4000D310 */
void        _UI_KeyEvent( int key, qboolean down );     /* 0x4000D6F0 */
void        _UI_MouseEvent( int dx, int dy );           /* 0x4000D790 */
uiMenuCommand_t _UI_GetActiveMenu( void );              /* 0x4000D800 */
qboolean    _UI_SetActiveMenu( uiMenuCommand_t menu );  /* 0x4000D810 */
qboolean    _UI_IsFullscreen( void );                   /* 0x4000DC00 */
void        Text_PaintCenter( float x, float y, int font, float scale, const vec4_t color, const char *text, float adjust );  /* 0x4000DE50 */
void        UI_DrawConnectScreen( qboolean overlay );   /* 0x4000E4E0 */
void        UI_RegisterCvars( void );                   /* 0x4000E940 */
void        UI_UpdateCvars( void );                     /* 0x4000E980 */
const char *UI_ConfigString( int index );               /* 0x4000EC00 */
const char *UI_SafeTranslateString( const char *reference );        /* 0x4000EC20 */
void        nullsub_5( void );                          /* 0x4000A100 */
void        nullsub_6( void );                          /* 0x4000ED20 */
void        nullsub_7( void );                          /* 0x4000ED30 */
float       sub_4000ED40( float a1 );                   /* 0x4000ED40 */
int         sub_4000ED50( float a1 );                   /* 0x4000ED50 */

//
// ui_atoms_mp.c
//
// 0x40006B30 .. 0x40007162.  UI_ConsoleCommand is RTCW's ui_atoms.c function;
// LTCG inlined it into vmMain as a tail chunk at 0x40006D00 (between UI_Cache_f
// and UI_Shutdown).  vmMain's UI_CONSOLE_COMMAND case calls it.
void QDECL  Com_Error( int level, const char *error, ... );
void QDECL  Com_Printf( const char *msg, ... );
float       UI_ClampCvar( float min, float max, float value );
void        UI_StartDemoLoop( void );
char       *UI_Argv( int arg );
char       *UI_Cvar_VariableString( const char *var_name );
qboolean    UI_ConsoleCommand( int realTime );
void        UI_Shutdown( void );
void        UI_AdjustFrom640( float *x, float *y, float *w, float *h );
/* imageTrack is a CoD addition -- the second argument of every
   trap_R_RegisterShaderNoMip call, passed through untouched (0x40006DE2). */
void        UI_DrawNamedPic( float x, float y, float width, float height, const char *picname, int imageTrack );
void        UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader );
void        UI_FillRect( float x, float y, float width, float height, const float *color );
void        UI_DrawSides( float x, float y, float w, float h );
void        UI_DrawTopBottom( float x, float y, float w, float h );
void        UI_DrawRect( float x, float y, float width, float height, const float *color );
void        UI_SetColor( const float *rgba );
void        UI_UpdateScreen( void );
void        UI_DrawTextBox( int x, int y, int width, int lines );
qboolean    UI_CursorInRect( int x, int y, int width, int height );

//
// ui_gameinfo_mp.c
//
// 0x40007170 .. 0x40007695.  RTCW's bot half of the file is absent from CoD.
int         UI_ParseInfos( char *buf, int max, char *infos[] );
void        UI_LoadArenas( void );

#endif  /* __UI_LOCAL_H__ */
