/*
 * cg_syscalls_mp.c -- every call the multiplayer client game module makes into
 * the engine.  cgame_mp_x86.dll is a native DLL, so this is the DLL half of the
 * RTCW cgame/cg_syscalls.c pattern: the engine hands dllEntry a single vararg
 * function pointer and each trap below is one line of body around it.
 *
 * Function order is binary order (0x30030460 .. 0x300322C0).  Signatures come
 * from CL_CgameSystemCalls in client_mp/cl_cgame_mp.c -- the argument each
 * `case` hands to the engine function is the argument the wrapper pushes -- and
 * from the refexport_t table in client_mp/cl_refapi.h for the renderer traps.
 *
 * PASSFLOAT compiles to a redundant store back into the parameter slot, so a
 * `mov [esp+arg_N], reg` ahead of the call marks argument N as a float; a
 * `mov [esp+arg_0], eax` after the call followed by `fld` marks a float
 * RETURN.
 *
 * NAMES.  121 of these 230 wrappers have recovered names; the other 109 carry
 * a trap_syscall_0xNN placeholder, with the engine's dispatch noted in a
 * comment.
 *
 * Fourteen of the recovered names sit one trap number too high: across three
 * windows (95-99, 188-197, 218-221) the wrapper that dispatches trap N carries
 * the name that belongs to trap N-1.  Those fourteen are moved down here, and
 * the five placeholders the shift left behind (95, 188, 189, 194, 218) take the
 * names it displaced.
 *
 * @fidelity: likely
 */

/* RTCW's cg_syscalls.c includes cg_local.h and so does this one: the three
   Register traps below call CG_DrawInformation (cg_info.c, 0x3001FA10), and
   cg_local.h is where that -- and every prototype in this file -- is declared. */
#include "cg_local.h"

static int ( QDECL * syscall )( int arg, ... ) = ( int ( QDECL * )( int, ... ) ) - 1;

void dllEntry( int ( QDECL *syscallptr )( int arg,... ) ) {
	syscall = syscallptr;
}

/*
 * PASSFLOAT and IntAsFloat compile to the same three instructions, so /OPT:ICF
 * folded them onto one address (0x30030470, which the Mac symbols name
 * PASSFLOAT).  Both are written out because both are used.
 */
int PASSFLOAT( float x ) {
	float floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

float IntAsFloat( int x ) {
	int intTemp;
	intTemp = x;
	return *(float *)&intTemp;
}

void trap_Print( const char *fmt ) {
	syscall( CG_PRINT, fmt );
}

void trap_Error( const char *fmt ) {
	syscall( CG_ERROR, fmt );
}

void trap_GameMessage( const char *msg, int width ) {
	syscall( CG_GAMEMESSAGE, msg, width );
}

void trap_BoldGameMessage( const char *msg, int width ) {
	syscall( CG_BOLDGAMEMESSAGE, msg, width );
}

void trap_DeathMessage( const char *attacker, const float *attackerColor, const char *victim, const float *victimColor, const char *icon, float iconWidth, float iconHeight, const float *iconColor ) {
	syscall( CG_DEATHMESSAGE, attacker, attackerColor, victim, victimColor, icon, PASSFLOAT( iconWidth ), PASSFLOAT( iconHeight ), iconColor );
}

/* the engine case is CL_SubtitlePrint( text, duration, channel ) */
void trap_Subtitle( const char *msg, int duration, int channel ) {
	syscall( CG_SUBTITLE, msg, duration, channel );
}

int trap_Milliseconds( void ) {
	return syscall( CG_MILLISECONDS );
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	syscall( CG_CVAR_REGISTER, cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall( CG_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( CG_CVAR_SET, var_name, value );
}

void trap_Cvar_Setvar( vmCvar_t *cvar, const char *var_name ) {
	syscall( CG_CVAR_SETVAR, cvar, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

int trap_Argc( void ) {
	return syscall( CG_ARGC );
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( CG_ARGV, n, buffer, bufferLength );
}

void trap_Args( char *buffer, int bufferLength ) {
	syscall( CG_ARGS, buffer, bufferLength );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( CG_FS_FOPENFILE, qpath, f, mode );
}

int trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	return syscall( CG_FS_READ, buffer, len, f );
}

int trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return syscall( CG_FS_WRITE, buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( CG_FS_FCLOSEFILE, f );
}

int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( CG_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

int trap_Com_SaveCvarsToBuffer( const char **cvars, int numCvars, char *buffer, int size ) {
	return syscall( CG_COM_SAVECVARSTOBUFFER, cvars, numCvars, buffer, size );
}

int trap_Com_LoadCvarsFromBuffer( const char **cvars, int numCvars, char *buffer, const char *text ) {
	return syscall( CG_COM_LOADCVARSFROMBUFFER, cvars, numCvars, buffer, text );
}

/* One argument, not the game DLL's ( exec_when, text ): the engine case is
   Cbuf_AddText( args[1] ) with no mode slot (0x004024CD). */
void trap_SendConsoleCommand( const char *text ) {
	syscall( CG_SENDCONSOLECOMMAND, text );
}

void trap_AddCommand( const char *cmdName ) {
	syscall( CG_ADDCOMMAND, cmdName );
}

void trap_SendClientCommand( const char *s ) {
	syscall( CG_SENDCLIENTCOMMAND, s );
}

void trap_UpdateScreen( void ) {
	syscall( CG_UPDATESCREEN );
}

/* Traps 26..29 all land in Con_DrawMessageWindow (CoDMP.exe 0x00409AD0); the
   recovered names for this family are shifted one slot, and the four slots
   are:
     arg1 -> Con_DrawStringOnHUD's x, the slot centering subtracts half the
             measured width from (0x004098C3);
     arg2 -> the y that walks +-12 per line (0x00409B1D, 0x00409D06);
     arg3 -> color[3] of the vec4 handed to the draw (0x00409877), i.e. an
             alpha, not a scale -- the 0.25 font scale is hardcoded beside it;
     arg4 -> the 4-way orientation switch (0x00409B00); 2 and 3 both go
             bottom-up and 3 sets the extra flag.  Con_DrawMiniConsole has no
             such slot -- the engine passes 0 (0x00409EF7). */
void trap_DrawNotifyLines( int x, int yStart, float alpha, int orientation ) {
	syscall( CG_DRAWNOTIFYLINES, x, yStart, PASSFLOAT( alpha ), orientation );
}

void trap_DrawBoldMessages( int x, int yStart, float alpha, int orientation ) {
	syscall( CG_DRAWBOLDMESSAGES, x, yStart, PASSFLOAT( alpha ), orientation );
}

void trap_DrawMiniConsole( int x, int yStart, float alpha ) {
	syscall( CG_DRAWMINICONSOLE, x, yStart, PASSFLOAT( alpha ) );
}

void trap_DrawSubtitles( int x, int yStart, float alpha, int orientation ) {
	syscall( CG_DRAWSUBTITLES, x, yStart, PASSFLOAT( alpha ), orientation );
}

void trap_DrawSay( int y ) {
	syscall( CG_DRAWSAY, y );
}

void trap_CM_LoadMap( const char *mapname ) {
	syscall( CG_CM_LOADMAP, mapname );
}

int trap_CM_NumInlineModels( void ) {
	return syscall( CG_CM_NUMINLINEMODELS );
}

int trap_CM_InlineModel( int index ) {
	return syscall( CG_CM_INLINEMODEL, index );
}

/*
 * TWO ARGUMENTS in retail.  0x300307A0 and 0x300307B0 are both `push eax /
 * push ecx / push <trap> / call syscall / add esp, 0Ch` -- twelve bytes
 * cleaned, so the trap number and exactly two arguments.  Pushed right to
 * left, args[1] = ecx = mins and args[2] = eax = maxs, which is the order the
 * engine reads them in.
 *
 * The engine reads a THIRD slot these wrappers never push -- retail's own
 * bug.  CL_CgameSystemCalls case 35 at 0x004025CD:
 *
 *     mov edx, [esi+0Ch]   ; args[3]
 *     mov eax, [esi+8]     ; args[2] -> maxs
 *     mov ecx, [esi+4]     ; args[1] -> mins
 *     push 0               ; capsule
 *     push edx             ; contents  <-- args[3]
 *     call CM_TempBoxModel
 *
 * and case 42 at 0x004025F7 is the same with capsule = 1.  CM_TempBoxModel
 * (0x0041A420) is __usercall( mins@<ecx>, maxs@<eax>, contents, capsule );
 * the server's own call in SV_ClipHandleForEntity 0x0045EC16 loads
 * gent->r.contents (`mov edx, [ecx+118h]`) into that same slot.
 *
 * So the server passes a real contents mask while the retail client drops it
 * and the engine reads whatever sits above the wrapper's pushes as
 * box_brush->contents.  That garbage is then AND-ed
 * against the trace clipmask in CM_Trace's CM_TEMP_CAPSULE_HANDLE branch
 * (0x00424A8C) before the capsule is traced at all; a box player entity is
 * gated the same way in CM_TraceThroughLeaf.
 *
 * What retail's slot 3 actually holds: LTCG inlines both wrappers into
 * CG_ClipMoveToEntities (0x30028EF1..0x30028F07 push only mins, maxs, cmd),
 * so args[3] is the dword under the pushes -- the edi that function's
 * prologue saved, which is CG_Trace's edi, `mov edi, ecx` at 0x30029045 =
 * &end.  A STACK address.  It works in retail because CoDMP.exe links
 * /STACK:0x800000 with no ASLR: an 8 MB stack cannot fit under the
 * 0x00400000 image, Windows places it just above the ~22 MB image around
 * 0x021xxxxx, and bit 25 of that address is CONTENTS_BODY, so the crosshair
 * trace (CONTENTS_SOLID|CONTENTS_BODY == 0x02000001) passes.  With a 1 MB
 * default stack and /DYNAMICBASE (esp 0x0093xxxx) the same retail cgame
 * shows no teammate name at all.
 *
 * A wrapper that is not inlined leaves a return address into
 * CG_ClipMoveToEntities (0x300127E2) in slot 3 -- bit 16 set, so prediction
 * traces (MASK_CLIENTSOLID 0x02810011) still clip, but bits 0 and 25 clear,
 * so the crosshair gate drops every teammate whatever the stack.  These two
 * wrappers therefore pass the entity contents CG_ClipMoveToEntities already
 * computed, which matches the server's own call and makes
 * box_brush->contents deterministic: one extra push per site against retail,
 * deliberately.
 */

/* CM_TempBoxModel( mins, maxs, contents, qfalse ) */
int trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int contents ) {
	return syscall( CG_CM_TEMPBOXMODEL, mins, maxs, contents );
}

/* CM_TempBoxModel( mins, maxs, contents, qtrue ) */
int trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs, int contents ) {
	return syscall( CG_CM_TEMPCAPSULEMODEL, mins, maxs, contents );
}

int trap_CM_PointContents( const vec3_t p, clipHandle_t model ) {
	return syscall( CG_CM_POINTCONTENTS, p, model );
}

int trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles ) {
	return syscall( CG_CM_TRANSFORMEDPOINTCONTENTS, p, model, origin, angles );
}

/* CM_BoxTrace( .., qfalse ) */
void trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask ) {
	syscall( CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask );
}

/* CM_TransformedBoxTraceExternal( .., qfalse ) */
void trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles ) {
	syscall( CG_CM_TRANSFORMEDBOXTRACE, results, start, end, mins, maxs, model, brushmask, origin, angles );
}

/* CM_BoxTrace( .., qtrue ) */
void trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask ) {
	syscall( CG_CM_CAPSULETRACE, results, start, end, mins, maxs, model, brushmask );
}

/* CM_TransformedBoxTraceExternal( .., qtrue ) */
void trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles ) {
	syscall( CG_CM_TRANSFORMEDCAPSULETRACE, results, start, end, mins, maxs, model, brushmask, origin, angles );
}

/* CoD's version fills the polyVert_t array itself -- xyz AND both st pairs --
   and reports the shader per fragment, so RTCW's markPoints/texCoordScale pair
   is gone.  Argument roles are CG_ImpactMark's (0x30023780). */
int trap_R_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection,
						  const vec3_t normal, float radius, int maxPoints, polyVert_t *pointBuffer,
						  int maxFragments, markFragment_t *fragmentBuffer, qhandle_t markShader ) {
	return syscall( CG_R_MARKFRAGMENTS, numPoints, points, projection, normal, PASSFLOAT( radius ),
					maxPoints, pointBuffer, maxFragments, fragmentBuffer, markShader );
}

void trap_R_LoadWorldMap( const char *mapname ) {
	syscall( CG_R_LOADWORLDMAP, mapname );
}

void trap_R_FinishLoadingModels( void ) {
	syscall( CG_R_FINISHLOADINGMODELS );
}

/* re.SetIgnorePrecacheErrors */
void trap_syscall_0x2F( int ignore ) {
	syscall( CG_SYSCALL_0X2F, ignore );
}

qhandle_t trap_R_RegisterModel( const char *name, int arg2 ) {
	CG_DrawInformation( 0 );
	return syscall( CG_R_REGISTERMODEL, name, arg2 );
}

/* re.GetShaderFromModel */
int trap_syscall_0x31( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0X31, a1, a2 );
}

int trap_R_GetXModelByHandle( qhandle_t hModel ) {
	return syscall( CG_R_GETXMODELBYHANDLE, hModel );
}

qhandle_t trap_R_RegisterShader( const char *name, int arg2 ) {
	CG_DrawInformation( 0 );
	return syscall( CG_R_REGISTERSHADER, name, arg2 );
}

/* re.RegisterShaderNoMip -- Material_RegisterHandle */
qhandle_t trap_R_RegisterShaderNoMip( const char *name, int arg2 ) {
	CG_DrawInformation( 0 );
	return syscall( CG_R_REGISTERSHADERNOMIP, name, arg2 );
}

int trap_R_RegisterFont( const char *name, int a2, int a3, int a4 ) {
	return syscall( CG_R_REGISTERFONT, name, a2, a3, a4 );
}

int trap_R_Text_Width( const char *text, int font, float scale, int limit ) {
	return syscall( CG_R_TEXT_WIDTH, text, font, PASSFLOAT( scale ), limit );
}

int trap_R_Text_Height( int font, float scale ) {
	return syscall( CG_R_TEXT_HEIGHT, font, PASSFLOAT( scale ) );
}

void trap_R_Text_Paint( float x, float y, int font, float scale, const float *color, const char *text, float adjust, int limit, int style ) {
	syscall( CG_R_TEXT_PAINT, PASSFLOAT( x ), PASSFLOAT( y ), font, PASSFLOAT( scale ), color, text, PASSFLOAT( adjust ), limit, style );
}

/* argument 8 is a CHAR, not a float: 0x30030EAE is `movsx edx, [esp+arg_1C]`
   and there is no redundant store on that slot -- only x, y and scale carry
   one. */
void trap_R_Text_PaintWithCursor( float x, float y, int font, float scale, const float *color, const char *text, int cursorPos, char cursor, int limit, int style ) {
	syscall( CG_R_TEXT_PAINTWITHCURSOR, PASSFLOAT( x ), PASSFLOAT( y ), font, PASSFLOAT( scale ), color, text, cursorPos, cursor, limit, style );
}

const char *trap_SE_TranslateReference( const char *reference ) {
	return (const char *)syscall( CG_SE_TRANSLATEREFERENCE, reference );
}

const char *trap_SE_LocalizeMessage( const char *msg, const char *reference ) {
	return (const char *)syscall( CG_SE_LOCALIZEMESSAGE, msg, reference );
}

int trap_SE_PrintStrlen( const char *text ) {
	return syscall( CG_SE_PRINTSTRLEN, text );
}

int trap_SE_ReadCharFromString( const unsigned char **text, int *advance ) {
	return syscall( CG_SE_READCHARFROMSTRING, text, advance );
}

void trap_R_ClearScene( void ) {
	syscall( CG_R_CLEARSCENE );
}

void trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	syscall( CG_R_ADDREFENTITYTOSCENE, re );
}

void trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	syscall( CG_R_ADDPOLYTOSCENE, hShader, numVerts, verts );
}

/* re.AddPolysToScene */
/* 0x30030F88 pushes 0x41: the placeholder name trap_syscall_0x42 and
   cg_public.h's CG_SYSCALL_0X42 = 65 are off by one */
void trap_syscall_0x41( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys ) {
	syscall( CG_SYSCALL_0X41, hShader, numVerts, verts, numPolys );
}

void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	syscall( CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT( intensity ), PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ) );
}

/* re.AddCoronaToScene */
void trap_syscall_0x43( const vec3_t org, float r, float g, float b, float scale, int id, int visible ) {
	syscall( CG_SYSCALL_0X43, org, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( scale ), id, visible );
}

void trap_R_SetFog( int a1, int a2, int a3, float a4, float a5, float a6, float a7 ) {
	syscall( CG_R_SETFOG, a1, a2, a3, PASSFLOAT( a4 ), PASSFLOAT( a5 ), PASSFLOAT( a6 ), PASSFLOAT( a7 ) );
}

void trap_R_RenderScene( const refdef_t *fd ) {
	syscall( CG_R_RENDERSCENE, fd );
}

void trap_R_SaveScreen( void ) {
	syscall( CG_R_SAVESCREEN );
}

void trap_R_BlendSavedScreen( int a1 ) {
	syscall( CG_R_BLENDSAVEDSCREEN, a1 );
}

void trap_R_SetColor( const float *rgba ) {
	syscall( CG_R_SETCOLOR, rgba );
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	syscall( CG_R_DRAWSTRETCHPIC, PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader );
}

/* re.StretchPicGradient */
void trap_syscall_0x4A( float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, int a9, int a10, int a11 ) {
	syscall( CG_SYSCALL_0X4A, PASSFLOAT( a1 ), PASSFLOAT( a2 ), PASSFLOAT( a3 ), PASSFLOAT( a4 ), PASSFLOAT( a5 ), PASSFLOAT( a6 ), PASSFLOAT( a7 ), PASSFLOAT( a8 ), a9, a10, a11 );
}

void trap_R_DrawStretchPicRotate( float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, int a10 ) {
	syscall( CG_R_DRAWSTRETCHPICROTATE, PASSFLOAT( a1 ), PASSFLOAT( a2 ), PASSFLOAT( a3 ), PASSFLOAT( a4 ), PASSFLOAT( a5 ), PASSFLOAT( a6 ), PASSFLOAT( a7 ), PASSFLOAT( a8 ), PASSFLOAT( a9 ), a10 );
}

void trap_R_DrawQuadPic( int a1, int a2, int a3 ) {
	syscall( CG_R_DRAWQUADPIC, a1, a2, a3 );
}

void trap_R_ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs ) {
	syscall( CG_R_MODELBOUNDS, model, mins, maxs );
}

/* re slot +0xA0 */
void trap_syscall_0x6B( int a1 ) {
	syscall( CG_SYSCALL_0X6B, a1 );
}

int trap_R_TrackStatistics( int a1, int a2, int a3, int a4, int a5, int a6 ) {
	return syscall( CG_R_TRACKSTATISTICS, a1, a2, a3, a4, a5, a6 );
}

void trap_GetGlconfig( glconfig_t *vidConfig ) {
	syscall( CG_GETGLCONFIG, vidConfig );
}

void trap_GetGameState( gameState_t *gamestate ) {
	syscall( CG_GETGAMESTATE, gamestate, sizeof( *gamestate ) );
}

void trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	syscall( CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime );
}

int trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	return syscall( CG_GETSNAPSHOT, snapshotNumber, snapshot, sizeof( *snapshot ) );
}

int trap_GetServerCommand( int serverCommandNumber ) {
	return syscall( CG_GETSERVERCOMMAND, serverCommandNumber );
}

int trap_GetCurrentCmdNumber( void ) {
	return syscall( CG_GETCURRENTCMDNUMBER );
}

int trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	return syscall( CG_GETUSERCMD, cmdNumber, ucmd );
}

void trap_SetUserCmdAimValues( const int *aim ) {
	syscall( CG_SETUSERCMDAIMVALUES, aim );
}

void trap_SetUserCmdInShellshock( int inShellshock ) {
	syscall( CG_SETUSERCMDINSHELLSHOCK, inShellshock );
}

/* Com_Printf( "%s%i\n", s, i ) */
void trap_syscall_0x76( const char *s, int i ) {
	syscall( CG_SYSCALL_0X76, s, i );
}

/* Com_Printf( "%s%f\n", s, f ) */
void trap_syscall_0x77( const char *s, float f ) {
	syscall( CG_SYSCALL_0X77, s, PASSFLOAT( f ) );
}

int trap_MemoryRemaining( void ) {
	return syscall( CG_MEMORYREMAINING );
}

/* Key_IsDown */
int trap_syscall_0x5A( int keynum ) {
	return syscall( CG_SYSCALL_0X5A, keynum );
}

/* cls.keyCatchers */
int trap_syscall_0x5B( void ) {
	return syscall( CG_SYSCALL_0X5B );
}

/* Key_SetCatcher */
void trap_syscall_0x5C( int catcher ) {
	syscall( CG_SYSCALL_0X5C, catcher );
}

/* Key_GetKey */
int trap_syscall_0x5D( const char *binding ) {
	return syscall( CG_SYSCALL_0X5D, binding );
}

/* the engine dispatches this to PC_AddGlobalDefine; see the file header */
int trap_CL_LookupColor( const char *name ) {
	return syscall( CG_CL_LOOKUPCOLOR, name );
}

/* unnamed in retail; named for what the engine case does (the recovered symbols in this window are one trap high) */
int trap_PC_LoadSource( const char *filename ) {
	return syscall( CG_PC_LOADSOURCE, filename );
}

/* recovered as trap_PC_LoadSource, which is trap 95's name; see the file header */
int trap_PC_FreeSource( int handle ) {
	return syscall( CG_PC_FREESOURCE, handle );
}

/* recovered as trap_PC_FreeSource, which is trap 96's name; see the file header */
int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( CG_PC_READTOKEN, handle, pc_token );
}

/* recovered as trap_PC_ReadToken, which is trap 97's name; see the file header */
int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( CG_PC_SOURCEFILEANDLINE, handle, filename, line );
}

/* recovered as trap_Z_FreeInternal, which is trap 189's name; see the file header */
void trap_Com_LoadSoundAliases( const char *mapname ) {
	syscall( CG_COM_LOADSOUNDALIASES, mapname );
}

/* recovered as trap_Com_LoadSoundAliases, which is trap 190's name; see the file header */
int trap_Com_SoundAliasString( const char *alias ) {
	return syscall( CG_COM_SOUNDALIASSTRING, alias );
}

/* recovered as trap_Com_SoundAliasString, which is trap 191's name; see the file header */
const char *trap_Com_PickSoundAlias( const char *alias ) {
	return (const char *)syscall( CG_COM_PICKSOUNDALIAS, alias );
}

/* recovered as trap_Com_PickSoundAlias, which is trap 192's name; see the file header */
const char *trap_Com_GetSoundAlias( int index ) {
	return (const char *)syscall( CG_COM_GETSOUNDALIAS, index );
}

/* unnamed in retail; named for what the engine case does (the recovered symbols in this window are one trap high) */
int trap_MSS_PlaySoundAlias( const char *alias, int entnum, const float *origin, int timeOffset ) {
	return syscall( CG_MSS_PLAYSOUNDALIAS, alias, entnum, origin, timeOffset );
}

/* recovered as trap_MSS_PlaySoundAlias, which is trap 194's name; see the file header */
void trap_MSS_PlayBlendedSoundAliases( const char *alias1, const char *alias2, float blend, int entnum, const float *origin, int timeOffset ) {
	syscall( CG_MSS_PLAYBLENDEDSOUNDALIASES, alias1, alias2, PASSFLOAT( blend ), entnum, origin, timeOffset );
}

/* recovered as trap_MSS_PlayBlendedSoundAliases, which is trap 195's name; see the file header */
int trap_SurfaceTypeFromName( const char *name ) {
	return syscall( CG_SURFACETYPEFROMNAME, name );
}

/* recovered as trap_SurfaceTypeFromName, which is trap 196's name; see the file header */
const char *trap_SurfaceTypeToName( int surfaceType ) {
	return (const char *)syscall( CG_SURFACETYPETONAME, surfaceType );
}

/* recovered as trap_PC_SourceFileAndLine, which is trap 98's name; see the file header
   Com_RealTime's out-parameter is Q3's qtime_t, which neither module declares;
   game_mp spells the slot `void *` and this one `int *`. */
int trap_RealTime( int *qtime ) {
	return syscall( CG_REALTIME, qtime );
}

void trap_SnapVector( float *v ) {
	syscall( CG_SNAPVECTOR, v );
}

int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return syscall( CG_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits );
}

int trap_CIN_StopCinematic( int handle ) {
	return syscall( CG_CIN_STOPCINEMATIC, handle );
}

int trap_CIN_RunCinematic( int handle ) {
	return syscall( CG_CIN_RUNCINEMATIC, handle );
}

void trap_CIN_DrawCinematic( int handle ) {
	syscall( CG_CIN_DRAWCINEMATIC, handle );
}

void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	syscall( CG_CIN_SETEXTENTS, handle, x, y, w, h );
}

/* re.GetEntityToken */
int trap_syscall_0x3E( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0X3E, a1, a2 );
}

int trap_hunkUsed( void ) {
	return syscall( CG_HUNKUSED );
}

int trap_UI_LoadMenu( const char *menuname ) {
	return syscall( CG_UI_LOADMENU, menuname );
}

int trap_UI_Popup( const char *menuname ) {
	return syscall( CG_UI_POPUP, menuname );
}

void trap_UI_ClosePopup( const char *menuname ) {
	syscall( CG_UI_CLOSEPOPUP, menuname );
}

void trap_UI_CloseAllMenus( void ) {
	syscall( CG_UI_CLOSEALLMENUS );
}

const char *trap_UI_GetMapDisplayName( const char *mapname ) {
	return (const char *)syscall( CG_UI_GETMAPDISPLAYNAME, mapname );
}

const char *trap_UI_GetGameTypeDisplayName( const char *gametype ) {
	return (const char *)syscall( CG_UI_GETGAMETYPEDISPLAYNAME, gametype );
}

const char *trap_CL_GetServerIPAddress( void ) {
	return (const char *)syscall( CG_CL_GETSERVERIPADDRESS );
}

void trap_XAnimPrecache( int a1 ) {
	syscall( CG_XANIMPRECACHE, a1 );
}

int trap_XAnimCreateAnims( int a1, int a2 ) {
	return syscall( CG_XANIMCREATEANIMS, a1, a2 );
}

void trap_XAnimCreate( int a1, int a2, int a3 ) {
	syscall( CG_XANIMCREATE, a1, a2, a3 );
}

int trap_XAnimCreateTree( int a1 ) {
	return syscall( CG_XANIMCREATETREE, a1 );
}

void trap_XAnimBlend( int a1, int a2, int a3, int a4, int a5, int a6 ) {
	syscall( CG_XANIMBLEND, a1, a2, a3, a4, a5, a6 );
}

/* DObjFree */
void trap_syscall_0x85( int a1 ) {
	syscall( CG_SYSCALL_0X85, a1 );
}

/* Every anim-index slot below, and the notify-index slots of 0x89/0x8A/
   0x8C/0x8D, is `movzx word` inside the retail wrapper (0x300316D0 ..
   0x30031D50): the originals took unsigned short.  It matters: MSVC leaves
   the upper half of a short argument slot undefined, so a caller compiled
   against bg_animation.c's unsigned short declaration pushes a whole
   scr_anim_t (tree in the high word), and an int definition here would
   forward all 32 bits. */
/* XAnimClearAnimNode_m */
void trap_XAnimClearGoalWeight( int a1, unsigned short a2, float a3 ) {
	syscall( CG_XANIMCLEARGOALWEIGHT, a1, a2, PASSFLOAT( a3 ) );
}

/*
 * The tree/index pairs.  `animIndex` is loaded with `movzx r32, word` (0x87 at
 * 0x30031704, 0x88 at 0x30031734, 0x8B at 0x30031814, 0x8C at 0x30031869), i.e.
 * the low half of a scr_anim_t -- the same spelling game_mp/g_local.h uses
 * for these traps.
 *
 * `tree` is a pointer, not an index: the engine dereferences the slot -- trap
 * 179 does `mov edx, [ecx+eax*8+0Ch]` on it (0x00403DAD), 183 does
 * `mov eax, [edx+4]` (0x00403E70), and 145 lands in XAnimSetTime_m's
 * `mov ax, [ecx+eax*2+8]` (0x004891F0); 131 gets it back from
 * XAnimAllocRuntimeTree, which returns an allocation.  game_mp/bg_animation.c
 * declares these traps `void *tree` unconditionally and is compiled into this
 * DLL, so its declarations bind to the definitions below; `int` and `void *`
 * are the same 4-byte slot on x86.  Kept as one `int` convention here, with
 * casts at the call sites in cg_animtree_mp.c, cg_ent_mp.c, cg_players_mp.c,
 * cg_snapshot_mp.c and cg_weapons_mp.c.
 */
void trap_XAnimClearTreeGoalWeights( int tree, unsigned short animIndex, float goalWeight ) {
	syscall( CG_XANIMCLEARTREEGOALWEIGHTS, tree, animIndex, PASSFLOAT( goalWeight ) );
}

void trap_XAnimClearTreeGoalWeightsStrict( int tree, unsigned short animIndex, float goalWeight ) {
	syscall( CG_XANIMCLEARTREEGOALWEIGHTSSTRICT, tree, animIndex, PASSFLOAT( goalWeight ) );
}

/* XAnimSetAnimKnob_m */
void trap_syscall_0x89( int a1, unsigned short a2, float a3, float a4, float a5, unsigned short a6, int a7 ) {
	syscall( CG_SYSCALL_0X89, a1, a2, PASSFLOAT( a3 ), PASSFLOAT( a4 ), PASSFLOAT( a5 ), a6, a7 );
}

/* XAnimSetAnimKnobAll_m */
int trap_XAnimSetCompleteGoalWeightKnobAll( int a1, unsigned short a2, unsigned short a3, float a4, float a5, float a6, unsigned short a7, int a8 ) {
	return syscall( CG_XANIMSETCOMPLETEGOALWEIGHTKNOBALL, a1, a2, a3, PASSFLOAT( a4 ), PASSFLOAT( a5 ), PASSFLOAT( a6 ), a7, a8 );
}

/* XAnimClearAnimChildren_m */
void trap_syscall_0x8B( int a1, unsigned short a2, float a3 ) {
	syscall( CG_SYSCALL_0X8B, a1, a2, PASSFLOAT( a3 ) );
}

/* XAnimSetAnimInternalLimited_m */
void trap_syscall_0x8C( int a1, unsigned short a2, float a3, float a4, float a5, unsigned short a6, int a7 ) {
	syscall( CG_SYSCALL_0X8C, a1, a2, PASSFLOAT( a3 ), PASSFLOAT( a4 ), PASSFLOAT( a5 ), a6, a7 );
}

void trap_XAnimSetCompleteGoalWeight( int a1, unsigned short a2, float a3, float a4, float a5, unsigned short a6, int a7 ) {
	syscall( CG_XANIMSETCOMPLETEGOALWEIGHT, a1, a2, PASSFLOAT( a3 ), PASSFLOAT( a4 ), PASSFLOAT( a5 ), a6, a7 );
}

/* XAnimSetAnimRate_m */
void trap_XAnimSetAnimRate( int a1, unsigned short a2, float a3 ) {
	syscall( CG_XANIMSETANIMRATE, a1, a2, PASSFLOAT( a3 ) );
}

/*
 * XAnimIsLooped_m( .., scrAnimPub.trees[..] ).  0x30031910 is `movzx eax,
 * word [esp+arg_0]` then `mov ecx, [esp+arg_0]; shr ecx, 16` -- ONE 4-byte
 * scr_anim_t split into (anims, index), not two arguments.  Same shape as
 * 0xA1/0xB5/0xB6, and what game_mp/bg_animation.c declares.
 */
int trap_XAnimIsLooped( scr_anim_t anim ) {
	return syscall( CG_XANIMISLOOPED, anim.anims, anim.index );
}

/* XAnimIsLooped_m */
int trap_syscall_0x90( int a1, unsigned short a2 ) {
	return syscall( CG_SYSCALL_0X90, a1, a2 );
}

/* XAnimSetTime_m */
void trap_XAnimSetTime( int a1, unsigned short a2, float a3 ) {
	syscall( CG_XANIMSETTIME, a1, a2, PASSFLOAT( a3 ) );
}

float trap_XAnimGetTime( int a1, unsigned short a2 ) {
	return IntAsFloat( syscall( CG_XANIMGETTIME, a1, a2 ) );
}

/* `tree` arrives in ecx; animIndex is `movzx word` at 0x300319A0 */
float trap_XAnimGetWeight( int tree, unsigned short animIndex ) {
	return IntAsFloat( syscall( CG_XANIMGETWEIGHT, tree, animIndex ) );
}

/* CL_DObjInvalidateSkels */
void trap_syscall_0x94( void ) {
	syscall( CG_SYSCALL_0X94 );
}

/* DObjUpdateClientInfo_m */
void trap_syscall_0x95( int a1, float a2 ) {
	syscall( CG_SYSCALL_0X95, a1, PASSFLOAT( a2 ) );
}

/* xanim.numDeferredNotifies + the notify array */
int trap_syscall_0x96( int a1 ) {
	return syscall( CG_SYSCALL_0X96, a1 );
}

/* DObjCalcAnim */
void trap_syscall_0x97( int a1, int a2 ) {
	syscall( CG_SYSCALL_0X97, a1, a2 );
}

/* DObjDisplayAnim */
void trap_syscall_0x98( int a1 ) {
	syscall( CG_SYSCALL_0X98, a1 );
}

/* XAnimGetRelDelta_m */
void trap_syscall_0x99( int a1, unsigned short a2, int a3, int a4, int a5 ) {
	syscall( CG_SYSCALL_0X99, a1, a2, a3, a4, a5 );
}

/* XAnimGetAbsDelta_m */
void trap_syscall_0x9A( int a1, unsigned short a2, int a3, int a4 ) {
	syscall( CG_SYSCALL_0X9A, a1, a2, a3, a4 );
}

/* XAnimGetRelDeltaForTime_m.  Same packed scr_anim_t as 0x8F:
   0x30031A86/0x30031A9A read the low word and the high half of ONE slot. */
void trap_XAnimGetRelDelta( scr_anim_t anim, float *deltaRot, float *deltaMove,
							float startTime, float endTime ) {
	syscall( CG_XANIMGETRELDELTA, anim.anims, anim.index, deltaRot, deltaMove,
			 PASSFLOAT( startTime ), PASSFLOAT( endTime ) );
}

/* XAnimGetAbsDeltaForTime_m */
void trap_syscall_0x9C( int a1, int a2, int a3, int a4, float a5 ) {
	syscall( CG_SYSCALL_0X9C, a1, a2, a3, a4, PASSFLOAT( a5 ) );
}

/* skel bone address from the DObj */
int trap_syscall_0x9D( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0X9D, a1, a2 );
}

/* skel bone address from the DObj */
int trap_syscall_0x9E( int a1 ) {
	return syscall( CG_SYSCALL_0X9E, a1 );
}

/* DObjMarkRotTransIndex */
int trap_syscall_0x9F( int a1, int a2, int a3 ) {
	return syscall( CG_SYSCALL_0X9F, a1, a2, a3 );
}

/* DObjMarkControlRotTransIndex */
int trap_syscall_0xA0( int a1, int a2, int a3 ) {
	return syscall( CG_SYSCALL_0XA0, a1, a2, a3 );
}

/* ONE packed argument, not two: 0x30031B50 splits it into (anim.anims =
   arg >> 16, anim.index = arg & 0xFFFF) itself */
int trap_XAnimGetAnimName( scr_anim_t anim ) {
	return syscall( CG_XANIMGETANIMNAME, anim.anims, anim.index );
}

/* Com_GetClientDObj */
int trap_syscall_0xA2( int a1 ) {
	return syscall( CG_SYSCALL_0XA2, a1 );
}

/* DObjCreate */
void trap_syscall_0xA3( int a1, int a2, int a3, int a4 ) {
	syscall( CG_SYSCALL_0XA3, a1, a2, a3, a4 );
}

/*
 * Com_ClientDObjCreate.  The game DLL's trap_DObjCreate takes a fifth
 * scrNotifyId; this one takes four and the ENGINE supplies the zero
 * (cl_cgame_mp.c case 164 calls Com_ClientDObjCreate( args[1],
 * (unsigned short)args[2], args[3], args[4], 0 )); bg_animation.c's CGAMEDLL
 * half calls it with four.  0x30031BA0, byte-identical to BG_DObjCreate at
 * 0x300046F0.
 */
void trap_DObjCreate( void *models, unsigned short modelCount, void *tree, int handle ) {
	syscall( CG_SYSCALL_0XA4, models, modelCount, tree, handle );
}

/* Com_SafeClientDObjFree */
void trap_SafeDObjFree( int a1, int a2 ) {
	syscall( CG_SAFEDOBJFREE, a1, a2 );
}

/* deref dword */
int trap_syscall_0xA6( int a1 ) {
	return syscall( CG_SYSCALL_0XA6, a1 );
}

/* 96 * dobj[23] + 48 */
int trap_syscall_0xA7( int a1 ) {
	return syscall( CG_SYSCALL_0XA7, a1 );
}

/* CL_DObjCreateSkelForBone */
int trap_syscall_0xA8( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0XA8, a1, a2 );
}

/* CL_DObjCreateSkelForBones */
int trap_syscall_0xA9( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0XA9, a1, a2 );
}

/* DObjGetHierarchyBits */
void trap_syscall_0xAA( int a1, int a2, int a3 ) {
	syscall( CG_SYSCALL_0XAA, a1, a2, a3 );
}

/* DObjCalcSkel */
void trap_syscall_0xAB( int a1, int a2 ) {
	syscall( CG_SYSCALL_0XAB, a1, a2 );
}

/* XModelExists */
int trap_syscall_0xAC( int a1 ) {
	return syscall( CG_SYSCALL_0XAC, a1 );
}

/* dobj[23] -- part count */
int trap_syscall_0xAD( int a1 ) {
	return syscall( CG_SYSCALL_0XAD, a1 );
}

/* DObjGetBoneIndex */
int trap_syscall_0xAE( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0XAE, a1, a2 );
}

/* DObjGetBoneName */
int trap_syscall_0xAF( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0XAF, a1, a2 );
}

/* DObjBuildPartCollisionTable */
void trap_syscall_0xB0( int a1, int a2 ) {
	syscall( CG_SYSCALL_0XB0, a1, a2 );
}

/* deref dword */
int trap_syscall_0xB1( int a1 ) {
	return syscall( CG_SYSCALL_0XB1, a1 );
}

/* anim tree child count == 0.  One packed scr_anim_t: 0x30031D10 is
   byte-identical in shape to 0x8F's. */
int trap_XAnimIsPrimitive( scr_anim_t anim ) {
	return syscall( CG_XANIMISPRIMITIVE, anim.anims, anim.index );
}

/* anim length in msec.  `tree` arrives in ecx, animIndex as `movzx eax, ax`
   (0x30031D30) -- two real arguments, unlike 0xB2 just above. */
int trap_XAnimGetLength( int tree, unsigned short animIndex ) {
	return syscall( CG_XANIMGETLENGTH, tree, animIndex );
}

/* XAnimHasTime */
int trap_syscall_0xB4( int a1, unsigned short a2 ) {
	return syscall( CG_SYSCALL_0XB4, a1, a2 );
}

/* anim tree child count */
/* one packed argument (0x30031D70); game_mp calls this trap_XAnimGetNumChildren */
int trap_syscall_0xB5( scr_anim_t anim ) {
	return syscall( CG_SYSCALL_0XB5, anim.anims, anim.index );
}

/* anim tree child base */
/* one packed argument plus childIndex@<eax>, and the result is re-packed into
   the low half before returning (0x30031DAA); game_mp calls this
   trap_XAnimGetChildAt */
scr_anim_t trap_syscall_0xB6( scr_anim_t anim, int childIndex ) {
	anim.index = (unsigned short)syscall( CG_SYSCALL_0XB6, anim.anims, anim.index, childIndex );
	return anim;
}

/* dobj[1] -- the tree */
int trap_XAnimGetAnimTreeSize( int tree ) {
	return syscall( CG_XANIMGETANIMTREESIZE, tree );
}

/* XAnimCopyTree */
void trap_syscall_0xB8( int a1, int a2 ) {
	syscall( CG_SYSCALL_0XB8, a1, a2 );
}

/* DObjDumpInfo */
void trap_syscall_0xB9( int a1 ) {
	syscall( CG_SYSCALL_0XB9, a1 );
}

/* StatMon_Warning */
void trap_syscall_0xBA( int a1, int a2, const char *a3 ) {
	syscall( CG_SYSCALL_0XBA, a1, a2, a3 );
}

/* StatMon_GetStatsArray */
void trap_syscall_0xBB( int a1, int a2 ) {
	syscall( CG_SYSCALL_0XBB, a1, a2 );
}

/* unnamed in retail; named for what the engine case does (the recovered symbols in this window are one trap high) */
void *trap_Z_MallocInternal( int size ) {
	return (void *)syscall( CG_Z_MALLOCINTERNAL, size );
}

/* unnamed in retail; named for what the engine case does (the recovered symbols in this window are one trap high) */
void trap_Z_FreeInternal( void *ptr ) {
	syscall( CG_Z_FREEINTERNAL, ptr );
}

/* CL_AddDebugLine.  game_mp/bg_misc.c names the last two.  Spelled const vec3_t
   because bg_misc.c and bg_pmove.c -- both compiled into this DLL -- declare it
   that way and bind to this definition. */
void trap_AddDebugLine( const vec3_t start, const vec3_t end, const vec3_t color,
						int depthTest, int duration ) {
	syscall( CG_ADD_DEBUG_LINE, start, end, color, depthTest, duration );
}

void *trap_GetWeaponInfoMemory( int size, int *pPrevOwner ) {
	return (void *)syscall( CG_GETWEAPONINFOMEMORY, size, pPrevOwner );
}

/* Com_FreeWeaponInfoMemory */
void trap_syscall_0xC8( int a1 ) {
	syscall( CG_SYSCALL_0XC8, a1 );
}

/* Hunk_AllocAlignInternal( size, 32 ) */
int trap_syscall_0xC9( int a1 ) {
	return syscall( CG_SYSCALL_0XC9, a1 );
}

/* Hunk_AllocLowAlignInternal( size, 32 ) -- the alignment is the engine case's,
   which is why the DLL side takes only the size. */
void *trap_Hunk_AllocLowInternal( int size ) {
	return (void *)syscall( CG_HUNKALLOCLOWINTERNAL, size );
}

/* Hunk_AllocAlignInternal */
int trap_syscall_0xCB( int a1, int a2 ) {
	return syscall( CG_SYSCALL_0XCB, a1, a2 );
}

void *trap_Hunk_AllocLowAlignInternal( int size, int align ) {
	return (void *)syscall( CG_HUNKALLOCLOWALIGNINTERNAL, size, align );
}

/* no-op in retail, but CG_DrawScriptUsage consumes the return (0x3001480E) */
int trap_syscall_0xCD( void ) {
	return syscall( CG_SYSCALL_0XCD );
}

/* no-op in retail; return consumed at 0x3001488C */
int trap_syscall_0xCE( void ) {
	return syscall( CG_SYSCALL_0XCE );
}

/* deref a global dword */
int trap_syscall_0xCF( void ) {
	return syscall( CG_SYSCALL_0XCF );
}

/* MSS_SetListener */
void trap_syscall_0xD1( int a1, int a2, int a3 ) {
	syscall( CG_SYSCALL_0XD1, a1, a2, a3 );
}

/* MSS_UpdateLoopingSounds */
void trap_syscall_0xD2( void ) {
	syscall( CG_SYSCALL_0XD2 );
}

/* MSS_StopSounds */
void trap_syscall_0xD3( int a1 ) {
	syscall( CG_SYSCALL_0XD3, a1 );
}

/* MSS_PlayMusicAlias */
void trap_syscall_0xD4( const float *a1 ) {
	syscall( CG_SYSCALL_0XD4, a1 );
}

/* MSS_StopBackground */
void trap_syscall_0xD5( int a1 ) {
	syscall( CG_SYSCALL_0XD5, a1 );
}

/* MSS_PlayAmbientAlias */
void trap_syscall_0xD6( int a1, int a2 ) {
	syscall( CG_SYSCALL_0XD6, a1, a2 );
}

/* MSS_FadeAllSounds */
void trap_syscall_0xD7( float a1, int a2 ) {
	syscall( CG_SYSCALL_0XD7, PASSFLOAT( a1 ), a2 );
}

/* MSS_FadeSelectSounds */
void trap_syscall_0xD8( const float *a1, int a2 ) {
	syscall( CG_SYSCALL_0XD8, a1, a2 );
}

/* MSS_SetEnvironmentEffects */
void trap_syscall_0xD9( const char *a1, float a2, int a3 ) {
	syscall( CG_SYSCALL_0XD9, a1, PASSFLOAT( a2 ), a3 );
}

/* unnamed in retail; named for what the engine case does (the recovered symbols in this window are one trap high) */
int trap_MSS_GetSoundOverlay( int a1, int a2, int a3, int a4 ) {
	return syscall( CG_MSS_GETSOUNDOVERLAY, a1, a2, a3, a4 );
}

void trap_syscall_0x55( int userCmdValue, float sensitivityScale ) {
	syscall( CG_SYSCALL_0X55, userCmdValue, PASSFLOAT( sensitivityScale ) );
}

/* cgameClientLerpOrigin */
void trap_syscall_0xD0( float a1, float a2, float a3 ) {
	syscall( CG_SYSCALL_0XD0, PASSFLOAT( a1 ), PASSFLOAT( a2 ), PASSFLOAT( a3 ) );
}

/* recovered as trap_MSS_GetSoundOverlay, which is trap 218's name; see the file header */
void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	syscall( CG_KEY_GETBINDINGBUF, keynum, buf, buflen );
}

/* recovered as trap_Key_GetBindingBuf, which is trap 219's name; see the file header */
void trap_Key_SetBinding( int keynum, const char *binding ) {
	syscall( CG_KEY_SETBINDING, keynum, binding );
}

/* recovered as trap_Key_SetBinding, which is trap 220's name; see the file header */
void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	syscall( CG_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen );
}

/* CFxScheduler::RegisterEffect */
int trap_syscall_0xDE( const char *name ) {
	return syscall( CG_SYSCALL_0XDE, name );
}

/* FX_GetBoneIndex */
int trap_syscall_0xDF( int a1, const char *boneName ) {
	return syscall( CG_SYSCALL_0XDF, a1, boneName );
}

/* CFxScheduler::PlayEffect( name, simple ) */
void trap_syscall_0xE0( int a1, int a2 ) {
	syscall( CG_SYSCALL_0XE0, a1, a2 );
}

/* CFxScheduler::PlayEffect( name ) */
void trap_syscall_0xE1( int a1, int a2, int a3 ) {
	syscall( CG_SYSCALL_0XE1, a1, a2, a3 );
}

/* CFxScheduler::PlayEffect( name, bolt ) */
void trap_syscall_0xE2( int a1, int a2, int a3, int a4 ) {
	syscall( CG_SYSCALL_0XE2, a1, a2, a3, a4 );
}

/* CFxScheduler::PlayEffect( id, simple ) */
void trap_syscall_0xE3( int a1, int a2 ) {
	syscall( CG_SYSCALL_0XE3, a1, a2 );
}

/* FX_PlayEffectID */
void trap_syscall_0xE4( int a1, int a2, int a3 ) {
	syscall( CG_SYSCALL_0XE4, a1, a2, a3 );
}

/* CFxScheduler::PlayEffect( id, axis ) */
void trap_syscall_0xE5( int a1, int a2, int a3, int a4 ) {
	syscall( CG_SYSCALL_0XE5, a1, a2, a3, a4 );
}

/* CFxScheduler::AddScheduledEffects */
void trap_syscall_0xE6( void ) {
	syscall( CG_SYSCALL_0XE6 );
}

/* FX_Init */
int trap_syscall_0xE7( void ) {
	return syscall( CG_SYSCALL_0XE7 );
}

/* FX_Free( 1 ) */
int trap_syscall_0xE8( void ) {
	return syscall( CG_SYSCALL_0XE8 );
}

/* FX_Free( 0 ) */
int trap_syscall_0xE9( void ) {
	return syscall( CG_SYSCALL_0XE9 );
}

/* SFxHelper::AdjustTime */
void trap_syscall_0xEA( int a1 ) {
	syscall( CG_SYSCALL_0XEA, a1 );
}

/* SFxHelper::AdjustCamera */
void trap_syscall_0xEB( int a1 ) {
	syscall( CG_SYSCALL_0XEB, a1 );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number */
int trap_syscall_0xEC( int a1 ) {
	return syscall( CG_SYSCALL_0XEC, a1 );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number */
int trap_syscall_0xED( int a1 ) {
	return syscall( CG_SYSCALL_0XED, a1 );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number */
int trap_syscall_0xEE( int a1 ) {
	return syscall( CG_SYSCALL_0XEE, a1 );
}

/* NO ENGINE CASE -- retail Com_Errors on this trap number */
int trap_syscall_0xEF( int a1 ) {
	return syscall( CG_SYSCALL_0XEF, a1 );
}

/* two cgame-owned engine dwords */
void trap_syscall_0xF0( float a1, float a2 ) {
	syscall( CG_SYSCALL_0XF0, PASSFLOAT( a1 ), PASSFLOAT( a2 ) );
}

/* CL_FirstSnapshot */
void trap_syscall_0xF1( void ) {
	syscall( CG_SYSCALL_0XF1 );
}

