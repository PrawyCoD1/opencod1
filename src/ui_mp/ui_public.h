/*
 * ui_public.h -- the user interface module / engine interface for ui_mp_x86.dll.
 *
 * Call of Duty 1.1 multiplayer.  RTCW's ui/ui_public.h is the structural
 * model: the records the two sides exchange, the trap numbers the module calls
 * out on (uiImport_t) and the entry points the engine calls in (uiExport_t).
 *
 * The import side is the wrapper bodies in ui_syscalls_mp.c
 * (0x40018E20..0x4001983B in ui_mp_x86.dll) and CL_UISystemCalls in
 * client_mp/cl_ui_mp.c (retail 0x00418300), the other half of this ABI.
 * The export side is vmMain (0x400076A0) and the VM_Call( uivm, .. ) sites
 * in client_mp.
 *
 * connstate_t, qtime_t and e_status belong in q_shared.h; universal/q_shared.h
 * does not carry them yet, so they are defined here, where their only
 * consumers in this module are.
 *
 * @fidelity: likely
 */

#ifndef __UI_PUBLIC_H__
#define __UI_PUBLIC_H__

#include "../universal/q_shared.h"
#include "ui_shared.h"

#ifndef QDECL
#define QDECL   __cdecl
#endif

/* vmMain( UI_GETAPIVERSION ) returns 7 (0x400076B7); CL_InitUI checks it. */
#define UI_API_VERSION  7

/*
 * cls.state, passed unchanged through uiClientState_t.  CoD's numbering, not
 * Q3's: CA_DISCONNECTED is 0 and CA_ACTIVE is 6 (client_mp/cl_vm.h, and the
 * `cmp cls_state, 6` trap 0x65 answers with).  coduo client_connection_types.h
 * names the whole run.
 */
typedef enum {
	CA_DISCONNECTED,
	CA_CONNECTING,
	CA_CHALLENGING,
	CA_CONNECTED,
	CA_LOADING,
	CA_PRIMED,
	CA_ACTIVE,
	CA_CINEMATIC,
	CA_LOGO
} connstate_t;

/*
 * trap_GetClientState's target.  RTCW's record verbatim: the engine's
 * GetClientState (0x00417410) stores connState at +0, connectPacketCount at
 * +4, clientNum at +8 and the three 1024-byte strings at +12, +1036, +2060.
 */
typedef struct {
	connstate_t connState;
	int connectPacketCount;
	int clientNum;
	char servername[MAX_STRING_CHARS];
	char updateInfoString[MAX_STRING_CHARS];
	char messageString[MAX_STRING_CHARS];
} uiClientState_t;
UI_ASSERT_SIZE( uiClientState_t, 3084 );

/*
 * trap_GetGlconfig's target.  CL_UISystemCalls case 0x34 copies 0xA0 bytes out
 * of cls.glconfig; the layout is client_mp/cl_refapi.h's, repeated from
 * cgame_mp/cg_public.h.
 */
typedef struct glconfig_t {
	/* +0x00 */ char   *renderer_string;
	/* +0x04 */ char   *vendor_string;
	/* +0x08 */ char   *version_string;
	/* +0x0C */ char   *extensions_string;
	/* +0x10 */ char   *wextensions_string;    /* CoD1 addition */
	/* +0x14 */ int     maxTextureSize;
	/* +0x18 */ int     maxActiveTextures;
	/* +0x1C */ int     maxLights;             /* CoD1 addition */
	/* +0x20 */ int     colorBits;
	/* +0x24 */ int     depthBits;
	/* +0x28 */ int     stencilBits;
	/* +0x2C */ int     deviceSupportsGamma;
	/* +0x30 */ int     unknown_0x30[2];
	/* +0x38 */ int     textureEnvAddAvailable;
	/* +0x3C */ int     unknown_0x3C[6];
	/* +0x54 */ int     NVFogAvailable;
	/* +0x58 */ int     unknown_0x58[11];
	/* +0x84 */ int     vidWidth;
	/* +0x88 */ int     vidHeight;
	/* +0x8C */ float   windowAspect;
	/* +0x90 */ int     displayFrequency;
	/* +0x94 */ int     isFullscreen;
	/* +0x98 */ int     stereoEnabled;
	/* +0x9C */ int     unknown_0x9C;
} glconfig_t;
UI_ASSERT_SIZE( glconfig_t, 160 );

/*
 * trap_syscall_0x1C (re.AddPolyToScene)'s vertex.  32 bytes, CoD's second
 * texcoord pair included; the layout is cgame_mp/cg_public.h's.
 */
typedef struct polyVert_t {
	/* +0x00 */ vec3_t xyz;
	/* +0x0C */ float st[2];
	/* +0x14 */ float st2[2];
	/* +0x1C */ byte modulate[4];
} polyVert_t;
UI_ASSERT_SIZE( polyVert_t, 32 );

/* trap_RealTime's out-record: Com_RealTime fills a struct tm.  Q3's qtime_t. */
typedef struct {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
} qtime_t;

/* The cinematic status the CIN_ traps return.  Q3's e_status unchanged
   (client_mp/cl_cinematic.h carries the engine's copy). */
typedef enum {
	FMV_IDLE,
	FMV_PLAY,
	FMV_EOF,
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

/*
 * trap_LAN_CompareServers' sortKey.  CL_UISystemCalls case 0x62 lands in
 * LAN_CompareServers (0x00417D40), whose switch compares serverInfo_t.password
 * (+0xB4) for 0, hostName (+0x14) for 1, mapName (+0x34) for 2, clients
 * (+0x98) for 3, gameType (+0x78, a string in CoD) for 4 and ping (+0xA8)
 * for 5.  CoD prepended the lock column to RTCW's five.
 */
#define SORT_PASSWORD       0
#define SORT_HOST           1
#define SORT_MAP            2
#define SORT_CLIENTS        3
#define SORT_GAME           4
#define SORT_PING           5

/*
=============================================================================

	THE TRAP NUMBERS

	Each constant is the immediate the matching wrapper in ui_syscalls_mp.c
	pushes before `call syscall`, and the comment is what CL_UISystemCalls
	does with it.  A UI_SYSCALL_0xNN spelling means the Mac symbol table did
	not name that wrapper; the number is kept rather than a name invented,
	as in cg_public.h.

	Traps the engine dispatcher answers but no wrapper issues: 0x1D
	(re.AddPolysToScene), 0x56 (LAN_SaveServersToCache), 0x5C
	(CIN_DrawCinematic) and 0x66..0x6E -- the QVM memory/math block (memset,
	memcpy, strncpy, sin, cos, atan2, sqrt, floor, ceil), unused because a
	native DLL calls the CRT directly.

	Traps a wrapper issues that the engine has NO case for: 0x37, 0x38, 0x39,
	0x3A -- RTCW's four LAN_GetLocal/GlobalServer* slots.  The exe's jump
	table (0x00418C68) sends all four to the default `Bad UI system trap`
	Com_Error, and nothing in the DLL calls those wrappers.

	0x25 is absent on BOTH sides: no case, no wrapper.

=============================================================================
*/

typedef enum
{
	UI_ERROR                               =   0,   /* Com_Error( ERR_DROP ) */
	UI_PRINT                               =   1,   /* Com_Printf */
	UI_GETLANGUAGENAME                     =   2,   /* SEH_GetLanguageName */
	UI_VERIFYLANGUAGESELECTION             =   3,   /* SEH_VerifyLanguageSelection */
	UI_MILLISECONDS                        =   4,
	UI_CVAR_REGISTER                       =   5,
	UI_CVAR_UPDATE                         =   6,
	UI_CVAR_SET                            =   7,   /* Cvar_Set2( name, value, qtrue ) */
	UI_CVAR_VARIABLEVALUE                  =   8,
	UI_CVAR_VARIABLESTRINGBUFFER           =   9,
	UI_CVAR_SETVALUE                       =  10,
	UI_SYSCALL_0XB                         =  11,   /* Cvar_Set2( name, NULL, qfalse ) -- Cvar_Reset */
	UI_SYSCALL_0XC                         =  12,   /* Cvar_Get -- Cvar_Create */
	UI_SYSCALL_0XD                         =  13,   /* Cvar_InfoStringBuffer */
	UI_SYSCALL_0XE                         =  14,   /* Cmd_Argc */
	UI_ARGV                                =  15,   /* Cmd_ArgvBuffer */
	UI_CMD_EXECUTETEXT                     =  16,   /* Cbuf_ExecuteText */
	UI_FS_FOPENFILE                        =  17,   /* FS_FOpenFileByMode */
	UI_FS_READ                             =  18,
	UI_SYSCALL_0X13                        =  19,   /* FS_Seek */
	UI_SYSCALL_0X14                        =  20,   /* FS_Write */
	UI_FS_FCLOSEFILE                       =  21,
	UI_FS_GETFILELIST                      =  22,
	UI_SYSCALL_0X17                        =  23,   /* FS_Delete */
	UI_R_REGISTERMODEL                     =  24,   /* re slot +0x0C */
	UI_R_REGISTERSHADERNOMIP               =  25,   /* re slot +0x14 */
	UI_R_CLEARSCENE                        =  26,   /* re slot +0x3C */
	UI_R_ADDREFENTITYTOSCENE               =  27,   /* re slot +0x40, the engine appends a literal 0 */
	UI_SYSCALL_0X1C                        =  28,   /* re.AddPolyToScene */
	/* 29 (re.AddPolysToScene) has an engine case but no wrapper. */
	UI_R_ADDLIGHTTOSCENE                   =  30,   /* re slot +0x4C */
	UI_SYSCALL_0X1F                        =  31,   /* re.AddCoronaToScene; see the wrapper */
	UI_R_RENDERSCENE                       =  32,   /* re slot +0x64 */
	UI_R_SETCOLOR                          =  33,   /* re slot +0x6C */
	UI_R_DRAWSTRETCHPIC                    =  34,   /* re slot +0x70 */
	UI_R_MODELBOUNDS                       =  35,   /* re slot +0x9C */
	UI_UPDATESCREEN                        =  36,   /* SCR_UpdateScreen */
	/* 37 is unused: the engine has no case for it and no wrapper issues it. */
	UI_S_REGISTERSOUND                     =  38,   /* Com_FindSoundAlias( name, 0 ) */
	UI_S_STARTLOCALSOUND                   =  39,   /* MSS_PlayLocalSoundAlias( 0, alias ) */
	/* the wrapper pushes 0x28 (0x400193FF), not 0x27 */
	UI_SYSCALL_0X28                        =  40,   /* MSS_FadeAllSounds */
	UI_KEY_KEYNUMTOSTRINGBUF               =  41,
	UI_KEY_GETBINDINGBUF                   =  42,
	UI_KEY_SETBINDING                      =  43,
	UI_SYSCALL_0X2C                        =  44,   /* Key_IsDown */
	UI_KEY_GETOVERSTRIKEMODE               =  45,
	UI_KEY_SETOVERSTRIKEMODE               =  46,
	UI_KEY_CLEARSTATES                     =  47,
	UI_KEY_GETCATCHER                      =  48,   /* cls.keyCatchers */
	UI_KEY_SETCATCHER                      =  49,
	UI_SYSCALL_0X32                        =  50,   /* GetClipboardData */
	UI_GETCLIENTSTATE                      =  51,
	UI_GETGLCONFIG                         =  52,   /* memcpy( .., &cls.glconfig, 0xA0 ) */
	UI_GETCONFIGSTRING                     =  53,
	UI_GETCLIENTNAME                       =  54,   /* GetClientname */
	UI_SYSCALL_0X37                        =  55,   /* NO ENGINE CASE */
	UI_SYSCALL_0X38                        =  56,   /* NO ENGINE CASE */
	UI_SYSCALL_0X39                        =  57,   /* NO ENGINE CASE */
	UI_SYSCALL_0X3A                        =  58,   /* NO ENGINE CASE */
	UI_SYSCALL_0X3B                        =  59,   /* LAN_GetPingQueueCount */
	UI_SYSCALL_0X3C                        =  60,   /* LAN_ClearPing */
	UI_SYSCALL_0X3D                        =  61,   /* LAN_GetPing */
	UI_SYSCALL_0X3E                        =  62,   /* LAN_GetPingInfo */
	UI_SYSCALL_0X3F                        =  63,   /* hunk memory remaining */
	UI_GETCDKEY                            =  64,   /* CLUI_GetCDKey */
	UI_SETCDKEY                            =  65,   /* CLUI_SetCDKey */
	UI_R_REGISTERFONT                      =  66,   /* re slot +0xA8 */
	UI_R_TEXT_WIDTH                        =  67,   /* re slot +0xCC, the engine inserts a literal 0 fourth */
	UI_R_TEXT_HEIGHT                       =  68,   /* re slot +0xD0 */
	UI_R_TEXT_PAINT                        =  69,   /* re slot +0xD4 */
	UI_R_TEXT_PAINTWITHCURSOR              =  70,   /* re slot +0xE0 */
	UI_SE_TRANSLATEREFERENCE               =  71,   /* SEH_StringEd_GetString */
	UI_SE_LOCALIZEMESSAGE                  =  72,   /* SEH_LocalizeTextMessage( msg, ref, 0 ) */
	UI_SYSCALL_0X49                        =  73,   /* PC_AddGlobalDefine( 0, define ) */
	UI_PC_LOADSOURCE                       =  74,
	UI_PC_FREESOURCE                       =  75,
	UI_PC_READTOKEN                        =  76,
	UI_PC_SOURCEFILEANDLINE                =  77,
	UI_REALTIME                            =  78,   /* Com_RealTime */
	UI_LAN_GETSERVERCOUNT                  =  79,   /* cls.num{local,global,favorite}servers by source */
	UI_LAN_GETSERVERADDRESSSTRING          =  80,
	UI_LAN_GETSERVERINFO                   =  81,
	UI_CLEARDISPLAYEDSERVERS               =  82,   /* LAN_MarkServerVisible */
	UI_LAN_UPDATEVISIBLEPINGS              =  83,   /* CL_UpdateVisiblePings_f */
	UI_LAN_RESETPINGS                      =  84,
	UI_LAN_LOADCACHEDSERVERS               =  85,
	UI_LAN_SAVECACHEDSERVERS               =  86,   /* LAN_SaveServersToCache; the wrapper is the chunk at 0x40019690 that _UI_Shutdown tail-jumps to */
	UI_LAN_ADDSERVER                       =  87,
	UI_LAN_REMOVESERVER                    =  88,
	UI_CIN_PLAYCINEMATIC                   =  89,
	UI_CIN_STOPCINEMATIC                   =  90,
	UI_CIN_RUNCINEMATIC                    =  91,
	UI_CIN_DRAWCINEMATIC                   =  92,   /* the chunk at 0x400197C0 that UI_DrawPreviewCinematic jumps to */
	UI_CIN_SETEXTENTS                      =  93,
	UI_VERIFYCDKEY                         =  94,   /* CL_CDKeyValidate */
	UI_LAN_SERVERSTATUS                    =  95,   /* CL_ServerStatus */
	UI_LAN_GETSERVERPING                   =  96,   /* serverInfo_t.ping via LAN_GetServerPtr, -1 when absent */
	UI_LAN_SERVERISDIRTY                   =  97,   /* serverInfo_t.visible via LAN_GetServerPtr, 0 when absent */
	UI_LAN_COMPARESERVERS                  =  98,
	UI_SYSCALL_0X63                        =  99,   /* returns 0 -- RTCW's SetPbClStatus slot, empty here */
	UI_CHECKAUTOUPDATE                     = 100,   /* CL_GetAutoUpdate */
	UI_GETAUTOUPDATE                       = 101,   /* cls.state == CA_ACTIVE */
	/* 102..110: the QVM memory/math block.  Engine cases, no wrappers. */
	UI_SYSCALL_0X6F                        = 111,   /* Z_MallocInternal */
	UI_SYSCALL_0X70                        = 112    /* free */
} uiImport_t;

/*
 * vmMain( UI_SET_ACTIVE_MENU, menu ).  0..5, 8 and 9 are the cases
 * _UI_SetActiveMenu (0x4000D810) switches on; 10 and 11 share a case and are
 * the two the engine's CG_UI_POPUP maps "UIMENU_SCRIPT_POPUP" and
 * "UIMENU_SCRIPT_POPUP_NO_MOUSE" to (cl_cgame_mp.c case 122); 8 and 9 come
 * from the same site's "UIMENU_WM_QUICKMESSAGE" / "UIMENU_WM_AUTOUPDATE"
 * strings; 7 is what CL_KeyEvent's "help" bind compares UI_GET_ACTIVE_MENU
 * against (RTCW cl_keys.c:1823 spells it UIMENU_HELP).  6 has no case
 * anywhere and keeps RTCW's slot only so 7 onward stay put.
 */
typedef enum {
	UIMENU_NONE,
	UIMENU_MAIN,
	UIMENU_INGAME,
	UIMENU_NEED_CD,
	UIMENU_BAD_CD_KEY,
	UIMENU_TEAM,
	UIMENU_POSTGAME,
	UIMENU_HELP,
	UIMENU_WM_QUICKMESSAGE,
	UIMENU_WM_AUTOUPDATE,
	UIMENU_SCRIPT_POPUP,
	UIMENU_SCRIPT_POPUP_NO_MOUSE
} uiMenuCommand_t;

/*
 * The vmMain commands.  0..7 are client_mp/cl_vm.h's, which CL_InitUI,
 * CL_KeyEvent, CL_Frame and the cinematic code issue; 8..16 are the rest of
 * vmMain's switch (0x400076AF), named for what each case calls and matched
 * to the VM_Call( uivm, N ) sites in client_mp: 8, 9, 10 and 15 from
 * CL_CgameSystemCalls (cases 121..126), 12 from SCR_DrawScreenField, 16 from
 * cl_main_mp.c:2237.
 */
typedef enum {
	UI_GETAPIVERSION = 0,   /* system reserved */

	UI_INIT,
/*	void	UI_Init( void ); */

	UI_SHUTDOWN,
/*	void	UI_Shutdown( void ); */

	UI_KEY_EVENT,
/*	void	UI_KeyEvent( int key, int down ); */

	UI_MOUSE_EVENT,
/*	void	UI_MouseEvent( int dx, int dy ); */

	UI_REFRESH,
/*	void	UI_Refresh( int time ); */

	UI_IS_FULLSCREEN,
/*	qboolean UI_IsFullscreen( void ); */

	UI_SET_ACTIVE_MENU,
/*	void	UI_SetActiveMenu( uiMenuCommand_t menu ); */

	UI_GET_ACTIVE_MENU,
/*	uiMenuCommand_t UI_GetActiveMenu( void ); */

	UI_GET_MAP_DISPLAY_NAME,
/*	const char *UI_GetMapDisplayName( const char *mapname ); */

	UI_GET_GAME_TYPE_DISPLAY_NAME,
/*	const char *UI_GetGameTypeDisplayName( const char *gametype ); */

	UI_CONSOLE_COMMAND,
/*	qboolean UI_ConsoleCommand( int realTime ); */

	UI_DRAW_CONNECT_SCREEN,
/*	void	UI_DrawConnectScreen( qboolean overlay ); */

	UI_HASUNIQUECDKEY,
/*	returns 1 (0x4000775C) */

	UI_CHECKEXECKEY,
/*	qboolean UI_CheckExecKey( void ); */

	UI_LOAD_SCRIPT_MENU,
/*	qboolean Load_ScriptMenu( const char *menuname ); the engine passes 5 second */

	UI_GET_FONT_INFO
/*	fontInfo_t *UI_GetFontInfo( int font, float scale ); scale arrives as an int percentage */
} uiExport_t;

/*
=============================================================================

	THE TRAP WRAPPERS -- ui_syscalls_mp.c

	Argument order is the order CL_UISystemCalls reads args[1..n]; the retail
	wrappers pass their last three arguments in edx, ecx, eax (an LTCG
	register convention).  The one narrow argument in the whole file is
	trap_R_Text_PaintWithCursor's
	cursor, a `movsx r32, byte` (0x40019128).

=============================================================================
*/

void        dllEntry( int ( QDECL *syscallptr )( int arg,... ) );
int         PASSFLOAT( float x );

void        trap_Print( const char *string );
void        trap_Error( const char *string );
const char  *trap_GetLanguagename( int language );
int         trap_VerifyLanguageSelection( int language );
int         trap_Milliseconds( void );
void        trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void        trap_Cvar_Update( vmCvar_t *cvar );
void        trap_Cvar_Set( const char *var_name, const char *value );
float       trap_Cvar_VariableValue( const char *var_name );
void        trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void        trap_Cvar_SetValue( const char *var_name, float value );
void        trap_syscall_0xB( const char *name );
void        trap_syscall_0xC( const char *var_name, const char *var_value, int flags );
void        trap_syscall_0xD( int bit, char *buffer, int bufsize );
int         trap_syscall_0xE( void );
void        trap_Argv( int n, char *buffer, int bufferLength );
void        trap_Cmd_ExecuteText( int exec_when, const char *text );
int         trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void        trap_FS_Read( void *buffer, int len, fileHandle_t f );
void        trap_syscall_0x13( fileHandle_t f, int offset, int origin );
void        trap_syscall_0x14( const void *buffer, int len, fileHandle_t f );
void        trap_FS_FCloseFile( fileHandle_t f );
int         trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int         trap_syscall_0x17( const char *filename );
qhandle_t   trap_R_RegisterModel( const char *name, int imageTrack );
void        trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font, int imageTrack );
int         trap_R_Text_Width( const char *text, int font, float scale, int limit );
int         trap_R_Text_Height( int font, float scale );
void        trap_R_Text_Paint( float x, float y, int font, float scale, const vec4_t color, const char *text, float adjust, int limit, int style );
void        trap_R_Text_PaintWithCursor( float x, float y, int font, float scale, const vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style );
const char  *trap_SE_TranslateReference( const char *reference );
const char  *trap_SE_LocalizeMessage( const char *msg, const char *reference );
qhandle_t   trap_R_RegisterShaderNoMip( const char *name, int imageTrack );
void        trap_R_ClearScene( void );
void        trap_R_AddRefEntityToScene( const refEntity_t *re );
void        trap_syscall_0x1C( qhandle_t hShader, int numVerts, const polyVert_t *verts );
void        trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void        trap_syscall_0x1F( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible );
void        trap_R_RenderScene( const refdef_t *fd );
void        trap_R_SetColor( const float *rgba );
void        trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void        trap_R_ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs );
void        trap_UpdateScreen( void );
sfxHandle_t trap_S_RegisterSound( const char *sample );
void        trap_S_StartLocalSound( const char *aliasName );   /* MSS_PlayLocalSoundAlias( 0, name ) */
void        trap_syscall_0x28( float a1, int a2 );
void        trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void        trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void        trap_Key_SetBinding( int keynum, const char *binding );
qboolean    trap_syscall_0x2C( int keynum );
qboolean    trap_Key_GetOverstrikeMode( void );
void        trap_Key_SetOverstrikeMode( qboolean state );
void        trap_Key_ClearStates( void );
int         trap_Key_GetCatcher( void );
void        trap_Key_SetCatcher( int catcher );
void        trap_syscall_0x32( char *buf, int bufsize );
void        trap_GetClientState( uiClientState_t *state );
void        trap_GetGlconfig( glconfig_t *glconfig );
int         trap_GetConfigString( int index, char *buff, int buffsize );
int         trap_GetClientName( int clientNum, char *buf, int buflen );
int         trap_syscall_0x37( void );
void        trap_syscall_0x38( int n, char *buf, int buflen );
int         trap_syscall_0x39( void );
void        trap_syscall_0x3A( int n, char *buf, int buflen );
int         trap_syscall_0x3B( void );
void        trap_syscall_0x3C( int n );
void        trap_syscall_0x3D( int n, char *buf, int buflen, int *pingtime );
void        trap_syscall_0x3E( int n, char *buf, int buflen );
qboolean    trap_LAN_UpdateVisiblePings( int source );
int         trap_LAN_GetServerCount( int source );
int         trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
void        trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void        trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int         trap_LAN_AddServer( int source, const char *name, const char *addr );
void        trap_LAN_RemoveServer( int source, const char *addr );
int         trap_LAN_GetServerPing( int source, int n );
int         trap_LAN_ServerIsDirty( int source, int n );
int         trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
void        trap_LAN_LoadCachedServers( void );
void        trap_LAN_SaveCachedServers( void );
void        trap_ClearDisplayedServers( int source, int n, qboolean visible );
void        trap_syscall_0x63( int status );
void        trap_LAN_ResetPings( int n );
int         trap_syscall_0x3F( void );
void        trap_GetCDKey( char *buf, int buflen, char *checksum, int checksumlen );
void        trap_SetCDKey( const char *key, const char *checksum );
int         trap_syscall_0x49( char *define );
int         trap_PC_LoadSource( const char *filename );
int         trap_PC_FreeSource( int handle );
int         trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int         trap_PC_SourceFileAndLine( int handle, char *filename, int *line );
int         trap_RealTime( qtime_t *qtime );
int         trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits );
e_status    trap_CIN_StopCinematic( int handle );
void        trap_CIN_DrawCinematic( int handle );
e_status    trap_CIN_RunCinematic( int handle );
void        trap_CIN_SetExtents( int handle, int x, int y, int w, int h );
void        *trap_syscall_0x6F( int size );
void        trap_syscall_0x70( void *ptr );
qboolean    trap_VerifyCDKey( const char *key, const char *chksum );
void        trap_CheckAutoUpdate( void );
qboolean    trap_GetAutoUpdate( void );

#endif  /* __UI_PUBLIC_H__ */
