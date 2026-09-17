/*
 * ui_syscalls_mp.c -- every call the multiplayer user interface module makes
 * into the engine.  ui_mp_x86.dll is a native DLL, so this is the DLL half of
 * the RTCW ui/ui_syscalls.c pattern: the engine hands dllEntry a single vararg
 * function pointer and each trap below is one line of body around it.
 *
 * Function order is binary order (0x40018E20 .. 0x40019830).  Signatures
 * match CL_UISystemCalls in client_mp/cl_ui_mp.c (retail 0x00418300) -- the
 * argument each `case` hands to the engine function is the argument the
 * wrapper pushes -- and, for the traps _UI_Init stores into the DC table,
 * the slot types in ui_shared.h.
 *
 * PASSFLOAT is a real call here (0x40018E30): every float argument is a
 * `call PASSFLOAT` ahead of its push, and the one float RETURN
 * (trap_Cvar_VariableValue) is a store followed by `fld`.  Void zero-argument
 * wrappers end `pop ecx`; the ones whose result is used end `add esp, 4`.
 *
 * The retail wrappers take their last three arguments in edx, ecx and eax
 * (LTCG's register convention); the argument ORDER below is the push order,
 * i.e. the order the engine reads args[1..n].  There are no unsigned short
 * and no packed arguments in this file -- the only narrow load is
 * trap_R_Text_PaintWithCursor's cursor.
 *
 * NAMES.  77 of these 102 have recovered names; the other 25 carry a
 * trap_syscall_0xNN placeholder, with the engine's dispatch noted in a
 * comment.  0x400193F0 pushes 0x28 and is spelled trap_syscall_0x28
 * (0x27 is trap_S_StartLocalSound).
 *
 * @fidelity: likely
 */

/* RTCW's ui_syscalls.c includes ui_local.h; every prototype this file needs
   lives in ui_public.h, which ui_local.h will include. */
#include "ui_public.h"

static int ( QDECL * syscall )( int arg, ... ) = ( int ( QDECL * )( int, ... ) ) - 1;

void dllEntry( int ( QDECL *syscallptr )( int arg,... ) ) {
	syscall = syscallptr;
}

int PASSFLOAT( float x ) {
	float floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void trap_Print( const char *string ) {
	syscall( UI_PRINT, string );
}

void trap_Error( const char *string ) {
	syscall( UI_ERROR, string );
}

/* SEH_GetLanguageName: g_languages[language].name, entry 0 when out of range */
const char *trap_GetLanguagename( int language ) {
	return (const char *)syscall( UI_GETLANGUAGENAME, language );
}

/* SEH_VerifyLanguageSelection; UI_VerifyLanguage compares the result with ui_language */
int trap_VerifyLanguageSelection( int language ) {
	return syscall( UI_VERIFYLANGUAGESELECTION, language );
}

int trap_Milliseconds( void ) {
	return syscall( UI_MILLISECONDS );
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	syscall( UI_CVAR_REGISTER, cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall( UI_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( UI_CVAR_SET, var_name, value );
}

float trap_Cvar_VariableValue( const char *var_name ) {
	int temp;
	temp = syscall( UI_CVAR_VARIABLEVALUE, var_name );
	return ( *(float*)&temp );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( UI_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_SetValue( const char *var_name, float value ) {
	syscall( UI_CVAR_SETVALUE, var_name, PASSFLOAT( value ) );
}

/* Cvar_Set2( name, NULL, qfalse ) -- RTCW's trap_Cvar_Reset */
void trap_syscall_0xB( const char *name ) {
	syscall( UI_SYSCALL_0XB, name );
}

/* Cvar_Get -- RTCW's trap_Cvar_Create */
void trap_syscall_0xC( const char *var_name, const char *var_value, int flags ) {
	syscall( UI_SYSCALL_0XC, var_name, var_value, flags );
}

/* Cvar_InfoStringBuffer */
void trap_syscall_0xD( int bit, char *buffer, int bufsize ) {
	syscall( UI_SYSCALL_0XD, bit, buffer, bufsize );
}

/* Cmd_Argc */
int trap_syscall_0xE( void ) {
	return syscall( UI_SYSCALL_0XE );
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( UI_ARGV, n, buffer, bufferLength );
}

void trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	syscall( UI_CMD_EXECUTETEXT, exec_when, text );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( UI_FS_FOPENFILE, qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall( UI_FS_READ, buffer, len, f );
}

/* FS_Seek */
void trap_syscall_0x13( fileHandle_t f, int offset, int origin ) {
	syscall( UI_SYSCALL_0X13, f, offset, origin );
}

/* FS_Write */
void trap_syscall_0x14( const void *buffer, int len, fileHandle_t f ) {
	syscall( UI_SYSCALL_0X14, buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( UI_FS_FCLOSEFILE, f );
}

int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( UI_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

/* FS_Delete */
int trap_syscall_0x17( const char *filename ) {
	return syscall( UI_SYSCALL_0X17, filename );
}

qhandle_t trap_R_RegisterModel( const char *name, int imageTrack ) {
	return syscall( UI_R_REGISTERMODEL, name, imageTrack );
}

/* Asset_Parse (0x40007D92) pushes ( name, pointSize, &uiInfo.uiDC.Assets.<font>, imageTrack ) */
void trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font, int imageTrack ) {
	syscall( UI_R_REGISTERFONT, fontName, pointSize, font, imageTrack );
}

int trap_R_Text_Width( const char *text, int font, float scale, int limit ) {
	return syscall( UI_R_TEXT_WIDTH, text, font, PASSFLOAT( scale ), limit );
}

int trap_R_Text_Height( int font, float scale ) {
	return syscall( UI_R_TEXT_HEIGHT, font, PASSFLOAT( scale ) );
}

void trap_R_Text_Paint( float x, float y, int font, float scale, const vec4_t color, const char *text, float adjust, int limit, int style ) {
	syscall( UI_R_TEXT_PAINT, PASSFLOAT( x ), PASSFLOAT( y ), font, PASSFLOAT( scale ), color, text, PASSFLOAT( adjust ), limit, style );
}

/* argument 8 is a CHAR: 0x40019128 is `movsx edx, byte ptr [esp+arg_1C]`, and
   only x, y and scale go through PASSFLOAT.  Same shape as cgame's. */
void trap_R_Text_PaintWithCursor( float x, float y, int font, float scale, const vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style ) {
	syscall( UI_R_TEXT_PAINTWITHCURSOR, PASSFLOAT( x ), PASSFLOAT( y ), font, PASSFLOAT( scale ), color, text, cursorPos, cursor, limit, style );
}

const char *trap_SE_TranslateReference( const char *reference ) {
	return (const char *)syscall( UI_SE_TRANSLATEREFERENCE, reference );
}

const char *trap_SE_LocalizeMessage( const char *msg, const char *reference ) {
	return (const char *)syscall( UI_SE_LOCALIZEMESSAGE, msg, reference );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name, int imageTrack ) {
	return syscall( UI_R_REGISTERSHADERNOMIP, name, imageTrack );
}

void trap_R_ClearScene( void ) {
	syscall( UI_R_CLEARSCENE );
}

void trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	syscall( UI_R_ADDREFENTITYTOSCENE, re );
}

/* re.AddPolyToScene */
void trap_syscall_0x1C( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	syscall( UI_SYSCALL_0X1C, hShader, numVerts, verts );
}

/* five arguments, not RTCW's six: CoD's AddLightToScene has no overdraw */
void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	syscall( UI_R_ADDLIGHTTOSCENE, org, PASSFLOAT( intensity ), PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ) );
}

/* re.AddCoronaToScene.  The wrapper pushes seven (`add esp, 20h` at
   0x400192B7: trap + org + four floats + ecx + eax), RTCW's argument list.
   The engine case (0x004185C9) reads only args[1..6] and pushes a literal 1
   where `visible` would go, so the seventh argument never reaches the
   renderer.  Reproduced, not fixed. */
void trap_syscall_0x1F( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible ) {
	syscall( UI_SYSCALL_0X1F, org, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( scale ), id, visible );
}

void trap_R_RenderScene( const refdef_t *fd ) {
	syscall( UI_R_RENDERSCENE, fd );
}

void trap_R_SetColor( const float *rgba ) {
	syscall( UI_R_SETCOLOR, rgba );
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	syscall( UI_R_DRAWSTRETCHPIC, PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader );
}

void trap_R_ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs ) {
	syscall( UI_R_MODELBOUNDS, model, mins, maxs );
}

void trap_UpdateScreen( void ) {
	syscall( UI_UPDATESCREEN );
}

/* Com_FindSoundAlias( sample, 0 ), the alias index or 0 */
sfxHandle_t trap_S_RegisterSound( const char *sample ) {
	return syscall( UI_S_REGISTERSOUND, sample );
}

/* one argument, not RTCW's ( sfx, channelNum ), and it is an alias NAME: the
   engine's case 0x27 is MSS_PlayLocalSoundAlias( 0, args[1] ), which hands it
   to Com_PickSoundAlias as a string.  That is why _UI_Init can store it in
   the displayContextDef_t's playClientSoundAliasByName slot untouched. */
void trap_S_StartLocalSound( const char *aliasName ) {
	syscall( UI_S_STARTLOCALSOUND, aliasName );
}

/* MSS_FadeAllSounds.  The immediate at 0x400193FF is 0x28.  Argument 1 goes
   through PASSFLOAT, 2 arrives in eax. */
void trap_syscall_0x28( float a1, int a2 ) {
	syscall( UI_SYSCALL_0X28, PASSFLOAT( a1 ), a2 );
}

void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	syscall( UI_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	syscall( UI_KEY_GETBINDINGBUF, keynum, buf, buflen );
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
	syscall( UI_KEY_SETBINDING, keynum, binding );
}

/* Key_IsDown */
qboolean trap_syscall_0x2C( int keynum ) {
	return syscall( UI_SYSCALL_0X2C, keynum );
}

qboolean trap_Key_GetOverstrikeMode( void ) {
	return syscall( UI_KEY_GETOVERSTRIKEMODE );
}

void trap_Key_SetOverstrikeMode( qboolean state ) {
	syscall( UI_KEY_SETOVERSTRIKEMODE, state );
}

void trap_Key_ClearStates( void ) {
	syscall( UI_KEY_CLEARSTATES );
}

int trap_Key_GetCatcher( void ) {
	return syscall( UI_KEY_GETCATCHER );
}

void trap_Key_SetCatcher( int catcher ) {
	syscall( UI_KEY_SETCATCHER, catcher );
}

/* GetClipboardData */
void trap_syscall_0x32( char *buf, int bufsize ) {
	syscall( UI_SYSCALL_0X32, buf, bufsize );
}

void trap_GetClientState( uiClientState_t *state ) {
	syscall( UI_GETCLIENTSTATE, state );
}

void trap_GetGlconfig( glconfig_t *glconfig ) {
	syscall( UI_GETGLCONFIG, glconfig );
}

int trap_GetConfigString( int index, char *buff, int buffsize ) {
	return syscall( UI_GETCONFIGSTRING, index, buff, buffsize );
}

/* GetClientname( args[2], args[1], args[3] ): the engine walks the snapshot's
   client records for clientNum and copies the name at +60.  UI_BuildPlayerList
   (0x40008F40) passes ( i, name, 32 ). */
int trap_GetClientName( int clientNum, char *buf, int buflen ) {
	return syscall( UI_GETCLIENTNAME, clientNum, buf, buflen );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number.  RTCW's
   LAN_GetLocalServerCount slot; uncalled. */
int trap_syscall_0x37( void ) {
	return syscall( UI_SYSCALL_0X37 );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number.  RTCW's
   LAN_GetLocalServerAddressString slot; uncalled. */
void trap_syscall_0x38( int n, char *buf, int buflen ) {
	syscall( UI_SYSCALL_0X38, n, buf, buflen );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number.  RTCW's
   LAN_GetGlobalServerCount slot; uncalled. */
int trap_syscall_0x39( void ) {
	return syscall( UI_SYSCALL_0X39 );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number.  RTCW's
   LAN_GetGlobalServerAddressString slot; uncalled. */
void trap_syscall_0x3A( int n, char *buf, int buflen ) {
	syscall( UI_SYSCALL_0X3A, n, buf, buflen );
}

/* LAN_GetPingQueueCount */
int trap_syscall_0x3B( void ) {
	return syscall( UI_SYSCALL_0X3B );
}

/* LAN_ClearPing */
void trap_syscall_0x3C( int n ) {
	syscall( UI_SYSCALL_0X3C, n );
}

/* LAN_GetPing */
void trap_syscall_0x3D( int n, char *buf, int buflen, int *pingtime ) {
	syscall( UI_SYSCALL_0X3D, n, buf, buflen, pingtime );
}

/* LAN_GetPingInfo */
void trap_syscall_0x3E( int n, char *buf, int buflen ) {
	syscall( UI_SYSCALL_0X3E, n, buf, buflen );
}

qboolean trap_LAN_UpdateVisiblePings( int source ) {
	return syscall( UI_LAN_UPDATEVISIBLEPINGS, source );
}

int trap_LAN_GetServerCount( int source ) {
	return syscall( UI_LAN_GETSERVERCOUNT, source );
}

/* the engine reads ( source, sortKey, sortDir, s1, s2 ) as args[1..5] and
   hands LAN_CompareServers ( s1, sortKey, source, sortDir, s2 ) -- its own
   parameter order, not the trap's */
int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return syscall( UI_LAN_COMPARESERVERS, source, sortKey, sortDir, s1, s2 );
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	syscall( UI_LAN_GETSERVERADDRESSSTRING, source, n, buf, buflen );
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	syscall( UI_LAN_GETSERVERINFO, source, n, buf, buflen );
}

int trap_LAN_AddServer( int source, const char *name, const char *addr ) {
	return syscall( UI_LAN_ADDSERVER, source, name, addr );
}

void trap_LAN_RemoveServer( int source, const char *addr ) {
	syscall( UI_LAN_REMOVESERVER, source, addr );
}

/* LAN_GetServerPtr( n, source )->ping, -1 when the slot is empty */
int trap_LAN_GetServerPing( int source, int n ) {
	return syscall( UI_LAN_GETSERVERPING, source, n );
}

/* LAN_GetServerPtr( n, source )->visible -- RTCW's trap_LAN_ServerIsVisible */
int trap_LAN_ServerIsDirty( int source, int n ) {
	return syscall( UI_LAN_SERVERISDIRTY, source, n );
}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return syscall( UI_LAN_SERVERSTATUS, serverAddress, serverStatus, maxLen );
}

void trap_LAN_LoadCachedServers( void ) {
	syscall( UI_LAN_LOADCACHEDSERVERS );
}

/* 0x40019690: `push 56h; call UI_syscall; pop ecx; retn`, a chunk
   _UI_Shutdown tail-jumps to. */
void trap_LAN_SaveCachedServers( void ) {
	syscall( UI_LAN_SAVECACHEDSERVERS );
}

/* LAN_MarkServerVisible( n, source, visible ) -- RTCW's trap_LAN_MarkServerVisible */
void trap_ClearDisplayedServers( int source, int n, qboolean visible ) {
	syscall( UI_CLEARDISPLAYEDSERVERS, source, n, visible );
}

/* the engine case is `return 0`: RTCW's SetPbClStatus slot with nothing behind it */
void trap_syscall_0x63( int status ) {
	syscall( UI_SYSCALL_0X63, status );
}

void trap_LAN_ResetPings( int n ) {
	syscall( UI_LAN_RESETPINGS, n );
}

/* hunk_totalSize - hunk_highTemp - hunk_temp_temp -- RTCW's trap_MemoryRemaining */
int trap_syscall_0x3F( void ) {
	return syscall( UI_SYSCALL_0X3F );
}

/* four arguments, not RTCW's two: CLUI_GetCDKey copies the key and its checksum */
void trap_GetCDKey( char *buf, int buflen, char *checksum, int checksumlen ) {
	syscall( UI_GETCDKEY, buf, buflen, checksum, checksumlen );
}

/* two arguments, not RTCW's one: CLUI_SetCDKey( key, checksum ) */
void trap_SetCDKey( const char *key, const char *checksum ) {
	syscall( UI_SETCDKEY, key, checksum );
}

/* PC_AddGlobalDefine( 0, define ).  ui_shared.h declares this slot as
   trap_PC_AddGlobalDefine; nothing in the DLL calls it. */
int trap_syscall_0x49( char *define ) {
	return syscall( UI_SYSCALL_0X49, define );
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall( UI_PC_LOADSOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return syscall( UI_PC_FREESOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( UI_PC_READTOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( UI_PC_SOURCEFILEANDLINE, handle, filename, line );
}

/* UI_StartServerRefresh (0x4000EA93) passes a stack qtime_t and reads tm_min.. back */
int trap_RealTime( qtime_t *qtime ) {
	return syscall( UI_REALTIME, qtime );
}

/* this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate) */
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return syscall( UI_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits );
}

/* stops playing the cinematic and ends it.  should always return FMV_EOF
   cinematics must be stopped in reverse order of when they are started */
e_status trap_CIN_StopCinematic( int handle ) {
	return syscall( UI_CIN_STOPCINEMATIC, handle );
}

/* will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached. */
e_status trap_CIN_RunCinematic( int handle ) {
	return syscall( UI_CIN_RUNCINEMATIC, handle );
}

/* 0x400197C0: `push eax; push 5Ch; call UI_syscall; add esp, 8; retn` -- the
   handle arrives in eax.  A chunk UI_DrawPreviewCinematic, UI_DrawCinematic
   and UI_DrawNetMapCinematic jump to. */
void trap_CIN_DrawCinematic( int handle ) {
	syscall( UI_CIN_DRAWCINEMATIC, handle );
}

/* allows you to resize the animation dynamically */
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	syscall( UI_CIN_SETEXTENTS, handle, x, y, w, h );
}

/* Z_MallocInternal */
void *trap_syscall_0x6F( int size ) {
	return (void *)syscall( UI_SYSCALL_0X6F, size );
}

/* free */
void trap_syscall_0x70( void *ptr ) {
	syscall( UI_SYSCALL_0X70, ptr );
}

qboolean trap_VerifyCDKey( const char *key, const char *chksum ) {
	return syscall( UI_VERIFYCDKEY, key, chksum );
}

/* RTCW's names, CoD's meaning.  _UI_Init stores trap_CheckAutoUpdate into
   uiDC.getAutoUpdate (+0xA4, 0x4000D5C0) and trap_GetAutoUpdate into
   uiDC.runningGame (+0xA8, 0x4000D5CA); the engine's 0x64 is CL_GetAutoUpdate
   and its 0x65 is `cls.state == CA_ACTIVE`, which is what ui_shared.c tests
   through DC->runningGame().  0x40019838 is `add esp, 4` after the call, the
   returning shape, where RTCW's is void. */
void trap_CheckAutoUpdate( void ) {
	syscall( UI_CHECKAUTOUPDATE );
}

qboolean trap_GetAutoUpdate( void ) {
	return syscall( UI_GETAUTOUPDATE );
}
