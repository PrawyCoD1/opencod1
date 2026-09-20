/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "cl_records.h"
#include "../botlib/l_script.h"
#include <string.h>
#include "cl_refapi.h"
#include "cl_vm.h"
#include "cl_conwindows.h"
#include "../universal/com_sndalias.h"

extern const char *VM_DllPath( const vm_t *vm );

extern int (*Material_RegisterHandle)();   /* 0x01432874, indirect call target */
extern int (*cgame_RestoreExportTable)();   /* 0x014328C0, indirect call target */
extern int (*cgame_SaveExportTable)();   /* 0x014328BC, indirect call target */
extern int (*dword_1432868)();   /* 0x01432868, indirect call target */
extern int (*dword_143286C)();   /* 0x0143286C, indirect call target */
extern int (*dword_1432878)();   /* 0x01432878, indirect call target */
extern int (*dword_1432880)();   /* 0x01432880, indirect call target */
extern int (*dword_1432888)();   /* 0x01432888, indirect call target */
extern int (*dword_14328A0)();   /* 0x014328A0, indirect call target */
extern int (*dword_14328A4)();   /* 0x014328A4, indirect call target */
extern int (*dword_14328A8)();   /* 0x014328A8, indirect call target */
extern int (*dword_14328AC)();   /* 0x014328AC, indirect call target */
extern int (*dword_14328B4)();   /* 0x014328B4, indirect call target */
extern int (*dword_14328B8)();   /* 0x014328B8, indirect call target */
extern int (*dword_14328C4)();   /* 0x014328C4, indirect call target */
extern int (*dword_14328C8)();   /* 0x014328C8, indirect call target */
extern int (*dword_14328D4)();   /* 0x014328D4, indirect call target */
extern int (*dword_14328D8)();   /* 0x014328D8, indirect call target */
extern int (*dword_14328DC)();   /* 0x014328DC, indirect call target */
extern int (*dword_14328F4)();   /* 0x014328F4, indirect call target */
extern int (*dword_14328F8)();   /* 0x014328F8, indirect call target */
extern int (*dword_14328FC)();   /* 0x014328FC, indirect call target */
extern int (*dword_1432900)();   /* 0x01432900, indirect call target */
extern int (*dword_1432904)();   /* 0x01432904, indirect call target */
extern int (*dword_1432908)();   /* 0x01432908, indirect call target */
extern int (*dword_143290C)();   /* 0x0143290C, indirect call target */
extern int (*dword_143292C)();   /* 0x0143292C, indirect call target */
extern int (*dword_1432930)();   /* 0x01432930, indirect call target */
extern int (*dword_1432934)();   /* 0x01432934, indirect call target */
extern int (*dword_1432940)();   /* 0x01432940, indirect call target */
extern int (*re_DrawStretchPic)();   /* 0x014328D0, indirect call target */
extern int (*re_RegisterShader)();   /* 0x01432870, indirect call target */
extern int (*re_SetColor)();   /* 0x014328CC, indirect call target */

vm_t    *cgvm = NULL;               /* retail 0x01617348 */

/* 0x015CA590, 160 bytes. Storage is cl_refstorage.c. TODO: cod1_globals.h declares it `char *` (132 bytes); a glconfig_t declaration there is a C2371. */
extern glconfig_t cls_glconfig;

/* 0x01432898. Indirect call target -- the cgame DLL entry point. */
extern int (*cgvm_dllEntry)();

extern signed __int32 cl_serverTime;   /* 0x01434A64, 4 bytes */
extern int clc_clientNum;   /* 0x015CE860, 12 bytes */
extern signed __int32 sys_sysMBValue;   /* 0x008E3B40, 4 bytes */
extern int clc_serverMessageSequence;   /* 0x015DE99C, 4 bytes */
extern int theFxHelper;   /* 0x01407508, 21788 bytes */
extern unsigned int dword_14075A0[5409];   /* 0x014075A0, 21636 bytes */
extern void *dword_1432894;   /* 0x01432894, 4 bytes */
extern int dword_143A980;   /* 0x0143A980, 8 bytes */
extern int dword_143A984;   /* 0x0143A984, 4 bytes */
extern int hunk_lowTempMark;   /* 0x008931C0, 4 bytes */
extern int scrMemTree_allocBuckets;   /* 0x008E6310, script/scr_memorytree.cpp:81 */
#define dword_8E6310            ( (char *)&scrMemTree_allocBuckets )
extern int cls_keyCatchers;   /* 0x0155F2C4, 4 bytes */
extern signed __int32 cls_state;   /* 0x0155F2C0, 4 bytes */
extern int cmd_argc;   /* 0x008930F0, 172 bytes */
extern int con_prevChannel;   /* 0x0142EF50, 4 bytes */
extern void *dword_143287C;   /* 0x0143287C, 28 bytes */
extern void *dword_143289C;   /* 0x0143289C, 32 bytes */
extern void *dword_14328F0;   /* 0x014328F0, 84 bytes */
extern signed __int32 cm_numSubModels;   /* 0x01617520, 4 bytes */
extern int com_hunkMeminfoRunningTotal;   /* 0x0163A204, 4 bytes */
extern int hunk_totalSize;   /* 0x0089319C, 4 bytes */
extern int hunk_highTemp;   /* 0x008931B0, 4 bytes */
extern void *scrAnimPub_trees[258];   /* 0x008E5AD8, 1032 bytes */
extern int xanim_numDeferredNotifies;   /* 0x00A9C638, 24 bytes */
extern int xanim_activePoolSlot;   /* 0x00A9CC58, 4 bytes */
extern unsigned char xanim_deferredNotifies[1536];   /* 0x00A9C650, 1536 bytes */
extern const char Source[4];   /* 0x005682F8, 4 bytes */
extern char byte_57C910[8192];   /* 0x0057C910, 8192 bytes */
extern cvar_t *cl_noprint;   /* 0x0161731C, 4 bytes */
extern cvar_t *cl_whitetext;   /*new*/
extern int con_boldgamemessagetime;   /* 0x0140EF2C, 4 bytes */
extern signed __int32 con_current;   /* 0x0142EF44, 4 bytes */
extern signed __int32 con_display;   /* 0x0142EF48, 4 bytes */
extern int con_gamemessagetime;   /* 0x0140EF30, 16 bytes */
extern int con_initialized;   /* 0x0140EF40, 4 bytes */
extern signed __int32 con_linewidth;   /* 0x0142EF54, 4 bytes */
extern __int16 con_text[65536];   /* 0x0140EF44, 131072 bytes */
extern int con_totallines;   /* 0x0142EF58, 8 bytes */
extern int hunk_temp_permanent;   /* 0x008931C4, 4 bytes */
extern int hunk_temp_temp;   /* 0x008931C8, 8 bytes */
extern float cgameClientLerpOrigin_x;   /* 0x0143A988, 4 bytes */
extern float cgameClientLerpOrigin_y;   /* 0x0143A98C, 4 bytes */
extern float cgameClientLerpOrigin_z;   /* 0x0143A990, 4 bytes */
extern int cgameUserAim_x;   /* 0x0143A994, 4 bytes */
extern int cgameUserAim_y;   /* 0x0143A998, 4 bytes */
extern int cgameUserAim_z;   /* 0x0143A99C, 1556 bytes */
extern signed __int32 cgameUserCmdInShellshock;   /* 0x0143A978, 4 bytes */
extern int cl_gameState_stringOffsets[1];   /* 0x01434A7C, 4 bytes */
extern cvar_t *cl_showServerCommands;   /* 0x0143294C, 8 bytes */
extern int clc_demoplaying;   /* 0x015EF004, 4 bytes */
extern int clc_serverCommandSequence;   /* 0x015DE9A0, 8 bytes */
extern char clc_serverCommands[65536];   /* 0x015DE9A8, 65536 bytes */
extern int clc_lastExecutedServerCommand;   /* 0x015DE9A4, 4 bytes */

extern unsigned char byte_14B9134[491520];   /* 0x014B9134, 491520 bytes */
extern unsigned char byte_1531134[188676];   /* 0x01531134, 188676 bytes */
extern float cgameSensitivity;   /* 0x0143A97C, 12 bytes */
extern signed __int32 cgameUserCmdValue;   /* 0x0143A974, 4 bytes */
extern int cl_cmdNumber;   /* 0x0143AFB0, 388 bytes */
extern int cl_parseClientsNum;   /* 0x0143A944, 48 bytes */
extern int cl_parseEntitiesNum;   /* 0x0143A940, 4 bytes */
extern char cl_snap_messageNum[4];   /* 0x01432970, 4 bytes */
extern int cl_snap_serverTime;   /* 0x0143296C, 4 bytes */
extern int cl_snapshots[67584 + 32 * PLAYERSTATE_EXTRA_BYTES / 4];   /* 0x0143B134, 270336 bytes */
extern unsigned char stru_143A9B0[UCMD_SIZE * CMD_BACKUP];

extern int CIN_DrawCinematic();
extern int CIN_PlayCinematic();
extern int CIN_RunCinematic( int handle );
extern void CIN_SetExtents( int handle, int x, int y, int w, int h );
extern int CIN_StopCinematic();
extern void CL_AddConsoleInfoColor( int base, const float *rgb );
extern void CL_AddDeathMessageText( const char *txt, int color );

extern int CL_AddReliableCommand();
extern int CL_ConsolePrint_AddLine( byte *txt, int channel, int duration,
                                    int width, int color );
extern int CL_GetServerIPAddress();
extern int CL_ReadDemoMessage();
extern void CL_SystemInfoChanged( void );
extern void CM_BoxTrace( trace_t *results, const float *start, const float *end,
                         const float *mins, const float *maxs, int model,
                         int brushmask, qboolean capsule );
extern int CM_InlineModel( int index );
extern void CM_LoadMap( const char *name, qboolean clientload, int *checksum );
extern int CM_PointContents( const float *p, int model );
extern int CM_TempBoxModel( const float *mins, const float *maxs, int contents,
                            qboolean capsule );
extern void CM_TransformedBoxTraceExternal( trace_t *results, const float *start,
                                            const float *end, const float *mins,
                                            const float *maxs, int model,
                                            int brushmask, const float *origin,
                                            const float *angles,
                                            qboolean capsule );
extern int CM_TransformedPointContents( const float *p, int model,
                                        const float *origin, const float *angles );
extern void Cmd_TokenizeString2( const char *text, int max_tokens );
extern void Com_ClientDObjCreate( void *models, unsigned short modelCount,
                                   void *tree, int handle, unsigned short scrNotifyId );
extern int *Com_FindSoundAlias( const char *name, int source );
extern void Com_FreeWeaponInfoMemory( int owner, int iSource );
extern void *Com_GetSoundAlias( int source, int index );
extern void *Com_GetWeaponInfoMemory( int size, int *pPrevOwner, int owner );
extern int Com_InitDObj();
qboolean Com_LoadCvarsFromBuffer( const char **cvarNames, int cvarCount,
                                  char *text, const char *filename );
qboolean Com_SaveCvarsToBuffer( const char **cvarNames, int cvarCount,
                                char *buffer, int bufferSize );
extern int Com_LoadSoundAliasFile( const char *csvPath, const char *sourceName,
                                   int count, int source );
extern int Com_LoadSoundAliasSounds();
extern void Com_MakeSoundAliasesPermanent( int count, int source );
extern void *Com_PickSoundAlias( const char *name, int source );
extern int Com_RealTime();
extern void Com_SafeClientDObjFree( int handle, unsigned int releaseTree );
extern int Com_SoundList_f();
extern int Com_TouchMemory();
void Com_UnloadSoundAliasSounds( int source );   /* 0x00433E20 */
extern void Con_ClearNotify( void );
extern void Con_ClearSubtitles( void );
extern int Con_Close();
extern void Con_DrawBoldMessages( int charset, float alpha, int orientation, int yStart );
extern void Con_DrawMiniConsole( int yStart, int charset, float alpha );
extern void Con_DrawNotify( int charset, float alpha, int orientation, int yStart );
extern void Con_DrawSay( int y );
extern void Con_DrawSubtitles( int charset, float alpha, int orientation, int yStart );
extern void Con_Linefeed( int duration, int channel );
/* Retail takes no arguments (call at 0x00401DAD). */
extern void Con_OneTimeInit( void );
extern void DObjCalcAnim( int dobj, int partBits );
extern void DObjCalcSkel( int partBits, int dobj );
extern int DObjCreate();
extern int DObjDumpInfo();
extern int DObjFree();
extern int FS_ListFilteredFiles();
extern int Hunk_AllocAlignInternal();
extern int Hunk_AllocLowAlignInternal();
extern int Hunk_AllocXAnimPrecache();
extern int Key_GetBindingBuf();
extern int Key_GetKey( const char *binding );
extern int Key_IsDown( int keynum );
extern void Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
extern void Key_SetBinding( int keynum, const char *binding );
extern int Key_SetCatcher();
extern int MSS_FadeAllSounds();
extern int MSS_FadeSelectSounds();
extern int MSS_GetSoundOverlay( int *cpuOut, int maxCount, void *buffer, int which );
extern int MSS_PlayAmbientAlias();
extern int MSS_PlayBlendedSoundAliases();
extern int MSS_PlayMusicAlias();
extern int MSS_PlaySoundAlias();
extern int MSS_SetEnvironmentEffects( const char *roomType, float wetTarget, int msec );
extern int MSS_SetListener();
extern int MSS_StopBackground();
extern int MSS_StopSounds();
extern int MSS_UpdateLoopingSounds();
extern float Q_acos( float c );
extern int SCR_UpdateScreen();
extern int SEH_PrintStrlen();
extern int SEH_ReadCharFromString();
extern char *SEH_SafeTranslateString( const char *key );
extern int SEH_StringEd_GetString();
extern int Scr_NearHook();
extern int Sys_SnapVector();
extern int XAnimLoadFileData_m();
extern int XAnimPrecacheAnimTree_m();
extern int XModelExists();
extern int j__atol( const char *s );
extern int FastRound( float f );
extern int DObjGetHierarchyBits();
extern int DObjBuildPartCollisionTable();
int DObjMarkRotTransIndex( void *dobj, const unsigned char *partBits, int boneIndex );
int DObjMarkControlRotTransIndex( void *dobj, const unsigned char *partBits, int boneIndex );
extern int DObjGetBoneIndex();
extern int DObjGetBoneName();
extern int XAnimSetParentNode();
extern int XAnimAllocTree();
extern int XAnimAllocRuntimeTree();
extern double XAnimGetTime( int nodeIndex, void *tree );
extern double XAnimGetWeight( int nodeIndex, void *tree );
extern int XAnimHasTime();
extern int XAnimGetAnimName();
void DObjUpdateClientInfo_m( void *dobj, float dtime );
extern int DObjDisplayAnim();
extern int XAnimGetRelDelta_m();
extern int XAnimGetAbsDelta_m();
extern int XAnimGetRelDeltaForTime_m( int a1, int a2, void *a3, float *a4, float a5, float a6 );
extern int XAnimGetAbsDeltaForTime_m( int a1, int a2, float *a3, void *a4, float a5 );
extern int XAnimClearAnimNode_m( void *tree, int node, float blend );
extern int XAnimClearAnim_m( int node, void *tree, float blend );
void XAnimClearAnimChildSubtrees_m( void *tree, int nodeIndex, float blendTime );
extern int XAnimSetAnimKnob_m( void *a1, int a2, float a3, int a4, float a5,
					   short a6, int a7, int a8 );
extern int XAnimSetAnimKnobAll_m( int a1, void *a2, int a3, float a4, int a5, float a6,
					   short a7, int a8, int a9 );
extern void XAnimClearAnimChildren_m( int *a1, int nodeIndex, float weight );
extern int XAnimSetTime_m();
extern int XAnimSetAnimInternalLimited_m( int a1, short a2, int a3, int a4, float a5, int a6,
					   float a7, int a8 );
extern int XAnimSetAnimRate_m( int a1, int a2, float a3 );
extern int XAnimIsLooped_m();
extern int XAnimSetAnimKnobInternal_m( short a1, void *a2, int a3, float a4, int a5, float a6,
					   int a7, int a8 );
extern int XAnimCopyTree();
extern int CFxScheduler__RegisterEffect();
extern int CFxScheduler__PlayEffect_id_simple_m();
extern int CFxScheduler__PlayEffect_name_bolt_m();
extern int CFxScheduler__PlayEffect_id_axis();
extern int CFxScheduler__PlayEffect_name_simple_m();
extern int CFxScheduler__PlayEffect_name_m();
extern void CFxScheduler__AddScheduledEffects( _DWORD *self );
extern int SFxHelper__AdjustTime();
extern void SFxHelper__AdjustCamera( int self, int view, float zfar );
extern int FX_Free();
extern int FX_Init();
extern int  __cdecl FX_GetBoneIndex( int entnum, const char *boneName );
extern void __cdecl FX_PlayEffectID( int *axis, int effectID, float *origin );
/* SE_GetString 0x004ABF00, stringed/stringed_strtable.c. */
extern char *SE_GetString( const char *key, int wantReference );

static char cl_mapnameBsp[64];

void CL_FirstSnapshot( void );
void CL_AdjustTimeDelta( void );
void CL_SetCGameTime( void );

/* ---- CL_GetUserCmd  0x004010A0 ----  VERIFIED */
qboolean CL_GetUserCmd( int cmdNumber, byte *ucmd )
{
	if ( cmdNumber > cl_cmdNumber ) {
		Com_Error( ERR_DROP, "" "CL_GetUserCmd: %i >= %i",
		           cmdNumber, cl_cmdNumber );
	}

	if ( cmdNumber <= cl_cmdNumber - CMD_BACKUP ) {
		return qfalse;
	}

	Com_Memcpy( ucmd, &stru_143A9B0[ UCMD_SIZE * ( cmdNumber & CMD_MASK ) ],
	            UCMD_SIZE );

	return qtrue;
}

void CL_GetGameState( void *gs );

static void CL_GetExtendedGameState( void *gs, int size ) {
	if ( size != GS_SIZE ) {
		Com_Error( ERR_DROP, "Cgame gamestate layout mismatch" );
		return;
	}
	memcpy( gs, cl_gameState_stringOffsets, GS_SIZE );
}

/* ---- CL_GetCurrentCmdNumber  0x004010F0 ----  [CONFIRMED] */
int __cdecl CL_GetCurrentCmdNumber()
{
  return cl_cmdNumber;
}

/* ---- CL_GetCurrentSnapshotNumber  0x00401100 ----  VERIFIED */
void CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime )
{
  *snapshotNumber = *(_DWORD *)cl_snap_messageNum;
  *serverTime = cl_snap_serverTime;
}

/* ---- CL_GetSnapshot  0x00401120 ----  VERIFIED */
qboolean CL_GetSnapshot( int snapshotNumber, byte *snapshot )
{
	const byte *clSnap;
	int   count;
	int   i;

	if ( snapshotNumber > *(int *)cl_snap_messageNum ) {
		Com_Error( ERR_DROP,
		           "\x15" "CL_GetSnapshot: snapshotNumber > cl.snapshot.messageNum" );
	}

	if ( *(int *)cl_snap_messageNum - snapshotNumber >= 32 ) {
		return qfalse;
	}

	clSnap = (const byte *)cl_snapshots + SNAP_SIZE * ( snapshotNumber & 31 );

	if ( !*(const int *)( clSnap + SNAP_VALID ) ) {
		return qfalse;
	}

	if ( cl_parseEntitiesNum
	     - *(const int *)( clSnap + SNAP_PARSEENTITIESNUM ) >= 0x800 ) {
		return qfalse;
	}
	if ( cl_parseClientsNum
	     - *(const int *)( clSnap + SNAP_PARSECLIENTSNUM ) >= 0x800 ) {
		return qfalse;
	}

	*(int *)( snapshot + SNAP_OUT_SNAPFLAGS )   = *(const int *)( clSnap + SNAP_SNAPFLAGS );
	*(int *)( snapshot + SNAP_OUT_CMDSEQUENCE ) = *(const int *)( clSnap + SNAP_SERVERCOMMANDNUM );
	*(int *)( snapshot + SNAP_OUT_PING )        = *(const int *)( clSnap + SNAP_PING );
	*(int *)( snapshot + SNAP_OUT_SERVERTIME )  = *(const int *)( clSnap + SNAP_SERVERTIME );

	Com_Memcpy( snapshot + SNAP_OUT_PS, clSnap + SNAP_PS, sizeof( playerState_t ) );

	count = *(const int *)( clSnap + SNAP_NUMENTITIES );
	if ( count > MAX_ENTITIES_IN_SNAPSHOT ) {
		if ( com_statmon->integer ) {
			StatMon_Warning( 8, 3000, "gfx/2d/warning@snapshotents.jpg" );
		} else {
			Com_DPrintf( "CL_GetSnapshot: truncated %i entities to %i\n",
			             count, MAX_ENTITIES_IN_SNAPSHOT );
		}
		count = MAX_ENTITIES_IN_SNAPSHOT;
	}
	*(int *)( snapshot + SNAP_OUT_NUMENTITIES ) = count;

	for ( i = 0; i < count; i++ ) {
		Com_Memcpy( snapshot + SNAP_OUT_ENTITIES + ES_SIZE * i,
		            &byte_14B9134[ ES_SIZE *
		                ( ( *(const int *)( clSnap + SNAP_PARSEENTITIESNUM ) + i ) & 0x7FF ) ],
		            ES_SIZE );
	}

	count = *(const int *)( clSnap + SNAP_NUMCLIENTS );
	if ( count > MAX_CLIENTS_IN_SNAPSHOT ) {
		count = MAX_CLIENTS_IN_SNAPSHOT;
	}
	*(int *)( snapshot + SNAP_OUT_NUMCLIENTS ) = count;

	for ( i = 0; i < count; i++ ) {
		Com_Memcpy( snapshot + SNAP_OUT_CLIENTS + CS_SIZE * i,
		            &byte_1531134[ CS_SIZE *
		                ( ( *(const int *)( clSnap + SNAP_PARSECLIENTSNUM ) + i ) & 0x7FF ) ],
		            CS_SIZE );
	}

	return qtrue;
}

/* ---- CL_SetUserCmdValue  0x004012C0 ----  [CONFIRMED] */
void __cdecl CL_SetUserCmdValue(int userCmdValue, float sensitivityScale)
{
  cgameUserCmdValue = userCmdValue;
  cgameSensitivity = sensitivityScale;
}

/* ---- CL_SetUserCmdAimValues  0x004012D0 ----  [CONFIRMED] */
void __cdecl CL_SetUserCmdAimValues(int *aim)
{
  cgameUserAim_x = *aim;
  cgameUserAim_y = aim[1];
  cgameUserAim_z = aim[2];
}

/* ---- CL_SetUserCmdInShellshock  0x004012F0 ----  [CONFIRMED] */
void __cdecl CL_SetUserCmdInShellshock(int value)
{
  cgameUserCmdInShellshock = value;
}

/* ---- CL_SetClientLerpOrigin  0x00401300 ----  [CONFIRMED] */
void __cdecl CL_SetClientLerpOrigin(float x, float y, float z)
{
  cgameClientLerpOrigin_x = x;
  cgameClientLerpOrigin_y = y;
  cgameClientLerpOrigin_z = z;
}

/* ---- CL_AddCgameCommand  0x00401320 ----  [CONFIRMED] */
void __cdecl CL_AddCgameCommand(const char *cmdName)
{
  Cmd_AddCommand(cmdName, 0);
}

/* ---- CL_ConfigstringModified  0x00401330 ----  VERIFIED */
void CL_ConfigstringModified( void )
{
	const char *old;
	const char *s;
	const char *dup;
	int         index;
	int         i;
	int         len;
	byte        oldGameState[GS_SIZE];
	int         snapshotBefore = *(int *)cl_snap_messageNum;

	index = j__atol( Cmd_Argv( 1 ) );
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "\x15" "configstring > MAX_CONFIGSTRINGS" );
	}

	s = Cmd_Argv( 2 );

	old = &cl_gameState_stringData[ cl_gameState_stringOffsets[ index ] ];
	if ( !strcmp( old, s ) ) {
		return;
	}

	Com_Memcpy( oldGameState, cl_gameState_stringOffsets, GS_SIZE );
	Com_Memset( cl_gameState_stringOffsets, 0, GS_SIZE );

	cl_gameState_dataCount = 1;

	for ( i = 0; i < MAX_CONFIGSTRINGS; i++ ) {
		if ( i == index ) {
			dup = s;
		} else {
			dup = (const char *)oldGameState + GS_STRINGDATA
			    + ((const int *)oldGameState)[ i ];
		}

		if ( !dup[0] ) {
			continue;
		}

		len = strlen( dup );

		if ( len + 1 + cl_gameState_dataCount > Protocol_GameStateLimit( clc_serverBuild >= 0 ) ) {
			Com_Error( ERR_DROP, "\x15" "MAX_GAMESTATE_CHARS exceeded" );
		}

		cl_gameState_stringOffsets[ i ] = cl_gameState_dataCount;
		Com_Memcpy( &cl_gameState_stringData[ cl_gameState_dataCount ],
		            dup, len + 1 );
		cl_gameState_dataCount += len + 1;
	}

	if ( index == CS_SYSTEMINFO ) {
		CL_SystemInfoChanged();
	}
	if ( *(int *)cl_snap_messageNum != snapshotBefore ) {
		Com_Printf( "Configstring %i changed snapshot number: %i -> %i\n",
		            index, snapshotBefore, *(int *)cl_snap_messageNum );
	}
}

/* ---- CL_GetServerCommand  0x004014E0 ----  VERIFIED */
qboolean CL_GetServerCommand( int serverCommandNumber )
{
	const char *s;
	const char *cmd;
	const char *arg;
	int         i;

	if ( serverCommandNumber <= clc_serverCommandSequence - 64 ) {
		if ( clc_demoplaying ) {
			return qfalse;
		}
		Com_Printf( "===== CL_GetServerCommand =====\n" );
		Com_Printf( "serverCommandNumber: %d\n", serverCommandNumber & 63 );
		for ( i = 0; i < 64; i++ ) {
			Com_Printf( "cmd %5d: %s\n", i,
			            &clc_serverCommands[ 1024 * ( i & 63 ) ] );
		}
		Com_Error( ERR_DROP,
		           "\x15" "CL_GetServerCommand: " "\x14" "EXE_ERR_RELIABLE_CYCLED_OUT" );
	}

	if ( serverCommandNumber > clc_serverCommandSequence ) {
		Com_Error( ERR_DROP,
		           "\x15" "CL_GetServerCommand: " "\x14" "EXE_ERR_NOT_RECEIVED" );
	}

	s = &clc_serverCommands[ 1024 * ( serverCommandNumber & 63 ) ];
	clc_lastExecutedServerCommand = serverCommandNumber;

	if ( cl_showServerCommands->integer ) {
		Com_DPrintf( "serverCommand: %i : %s\n", serverCommandNumber, s );
	}

rescan:
	Cmd_TokenizeString2( s, 0 );
	cmd = Cmd_Argv( 0 );

	switch ( cmd[0] ) {

	case 'd':
		Cmd_TokenizeString2( s, 3 );
		CL_ConfigstringModified();
		Cmd_TokenizeString2( s, 3 );
		return qtrue;

	case 'n':
		Con_ClearNotify();
		Con_ClearSubtitles();
		Com_Memset( stru_143A9B0, 0, UCMD_SIZE * CMD_BACKUP );
		dword_14328C8();
		return qtrue;

	case 'w':
		if ( Cmd_Argc() >= 2 ) {
			if ( !strcmp( Cmd_Argv( 1 ), "EXE_UNPURECLIENTDETECTED" ) ) {
				char paks[1024];
				int count = FS_ListUnapprovedReferencedPaks( paks, sizeof( paks ) );
				Com_Error( ERR_SERVERDISCONNECT, "%s\n\n%s%s",
					va( SEH_SafeTranslateString( "EXE_SERVERDISCONNECTREASON" ),
						SEH_SafeTranslateString( "EXE_UNPURECLIENTDETECTED" ) ),
					count ? "PK3s not allowed by this server: " :
						"No specific PK3 could be identified from the server's allowed list.",
					count ? paks : "" );
			}
			Com_Error( ERR_SERVERDISCONNECT,
			           va( SEH_SafeTranslateString( "EXE_SERVERDISCONNECTREASON" ),
			               SEH_SafeTranslateString( Cmd_Argv( 1 ) ) ) );
		} else {
			Com_Error( ERR_SERVERDISCONNECT, "EXE_SERVER_DISCONNECTED" );
		}
		return qtrue;

	case 'x':
		/* Configstring chunks use a raw remainder, including spaces. Match
		 * the short 'd' command format; synthetic quotes corrupt systeminfo. */
		Cmd_TokenizeString2( s, 3 );
		Com_sprintf( byte_57C910, 0x2000, "d %s %s",
		             Cmd_Argv( 1 ), Cmd_Argv( 2 ) );
		return qfalse;

	case 'y':
		Cmd_TokenizeString2( s, 3 );
		arg = Cmd_Argv( 2 );
		if ( strlen( byte_57C910 ) + strlen( arg ) >= 0x2000 ) {
			Com_Error( ERR_DROP, "\x15" "bcs exceeded BIG_INFO_STRING" );
		}
		strcat( byte_57C910, arg );
		return qfalse;

	case 'z':
		Cmd_TokenizeString2( s, 3 );
		arg = Cmd_Argv( 2 );
		if ( strlen( byte_57C910 ) + strlen( arg ) >= 0x2000 ) {
			Com_Error( ERR_DROP, "\x15" "bcs exceeded BIG_INFO_STRING" );
		}
		strcat( byte_57C910, arg );
		s = byte_57C910;
		goto rescan;

	default:
		return qtrue;
	}
}

/* ---- CL_SetExpectedHunkUsage  0x00401840 ----  VERIFIED */
void CL_SetExpectedHunkUsage( const char *mapname ) {
	fileHandle_t f;
	char        *buf;
	char        *buftrav;
	char        *token;
	int         len;

	len = FS_FOpenFileByMode( "hunkusage.dat", &f, FS_READ );
	if ( len >= 0 ) {
		buf = (char *)Z_MallocInternal( len + 1 );
		memset( buf, 0, len + 1 );

		FS_Read( buf, len, f );
		FS_FCloseFile( f );

		buftrav = buf;
		while ( ( token = Com_Parse( &buftrav ) ) != 0 && token[0] ) {
			if ( !Q_stricmp( token, mapname ) ) {
				token = Com_Parse( &buftrav );
				if ( token && token[0] ) {
					Cvar_Set2( "com_expectedhunkusage", token, qtrue );
					free( buf );
					return;
				}
			}
		}

		free( buf );
	}

	Cvar_Set2( "com_expectedhunkusage", "-1", qtrue );
}

/* ---- CL_CM_LoadMap  0x004019F0 ----  VERIFIED */
void CL_CM_LoadMap( const char *mapname ) {
	int checksum;

	if ( !com_sv_running->integer ) {
		CL_SetExpectedHunkUsage( mapname );
	}

	CM_LoadMap( mapname, qtrue, &checksum );
}

#if 0
void __cdecl CL_ShutdownCGame()
{
  int *v0;

  cls_keyCatchers &= ~8u;
  if ( cgvm )
  {
    VM_Call(cgvm, 1);
    v0 = (int *)cgvm;
    if ( *(_DWORD *)&cgvm->gap0[136] )
    {
      if ( !FreeLibrary(*(HMODULE *)&cgvm->gap0[136]) )
        Com_Error(ERR_FATAL, "\x15Sys_UnloadDll FreeLibrary failed");
    }
    Com_Memset(v0, 0, 0x90u);
    currentVM = 0;
    cgvm = 0;
    if ( snd_aliasLoaded_ui )
    {
      Com_UnloadSoundAliasSounds(1);
      if ( Block )
      {
        if ( !snd_aliasList_uiOwned )
          free(Block);
        Block = 0;
        dword_893DE8 = 0;
        memset(&unk_8935D8, 0, 0x400u);
      }
      snd_aliasLoaded_ui = 0;
      if ( !LOBYTE(snd_aliasLoaded[0]) )
        Cmd_RemoveCommand("snd_list");
    }
  }
}
#endif

/* ---- CL_DObjInvalidateSkels  0x00401B00 ----  VERIFIED */
void CL_DObjInvalidateSkels( void )
{
  if ( !++com_skelTimeStamp )
    com_skelTimeStamp = 1;
  if ( s_hunkData )
    *(_DWORD *)hunk_temp_temp = *(_DWORD *)hunk_temp_permanent;
}

/* ---- CL_DObjCalcAnim  0x00401B30 ----  VERIFIED */
void CL_DObjCalcAnim( int dobj, int partBits )
{
  DObjCalcAnim( dobj, partBits );
}

/* ---- FloatAsInt_0  0x00401B40 ----  [CONFIRMED] */
int __cdecl FloatAsInt_0(int val)
{
  return val;
}

/* ---- CL_DObjCreateSkelForBone  0x00401B50 ----  VERIFIED */
qboolean CL_DObjCreateSkelForBone( int dobj, int boneIndex )
{
  int   skel;
  int  *p;
  int   i;

  if ( *(_DWORD *)( dobj + 8 ) == com_skelTimeStamp )
  {
    skel = *(_DWORD *)( dobj + 4 );
    if ( skel )
      return ( ( ( (byte *)skel )[ 0x20 + ( boneIndex >> 3 ) ]
                 & ( 1 << ( boneIndex & 7 ) ) ) != 0 );
  }
  else
  {
    *(_DWORD *)( dobj + 8 ) = com_skelTimeStamp;
    *(_WORD *)( dobj + 16 ) = 0;
    *(_DWORD *)( dobj + 4 ) = 0;
  }

  skel = (int)Hunk_AllocateTempMemoryInternal(
                96 * *(unsigned __int8 *)( dobj + 23 ) + 48 );
  *(_DWORD *)( dobj + 4 ) = skel;

  /* three parallel four-word masks at +0x00, +0x10 and +0x20 */
  p = (int *)( skel + 32 );
  for ( i = 4; i; i-- )
  {
    *( p - 8 ) = 0;
    *( p - 4 ) = 0;
    *p++ = 0;
  }

  return qfalse;
}

/* ---- CL_DObjCreateSkelForBones  0x00401BE0 ----  VERIFIED */
qboolean CL_DObjCreateSkelForBones( int dobj, int partBits )
{
  const int *want;
  const int *have;
  int   skel;
  int  *p;
  int   i;

  if ( *(_DWORD *)( dobj + 8 ) == com_skelTimeStamp )
  {
    skel = *(_DWORD *)( dobj + 4 );
    if ( skel )
    {
      want = (const int *)partBits;
      have = (const int *)( skel + 32 );
      for ( i = 0; i < 4; i++ )
      {
        if ( want[i] & ~have[i] )
          return qfalse;
      }
      return qtrue;
    }
  }
  else
  {
    *(_DWORD *)( dobj + 8 ) = com_skelTimeStamp;
    *(_WORD *)( dobj + 16 ) = 0;
    *(_DWORD *)( dobj + 4 ) = 0;
  }

  skel = (int)Hunk_AllocateTempMemoryInternal(
                96 * *(unsigned __int8 *)( dobj + 23 ) + 48 );
  *(_DWORD *)( dobj + 4 ) = skel;

  p = (int *)( skel + 32 );
  for ( i = 4; i; i-- )
  {
    *( p - 8 ) = 0;
    *( p - 4 ) = 0;
    *p++ = 0;
  }

  return qfalse;
}

/* ---- CL_DObjCalcSkel  0x00401C50 ----  VERIFIED */
void CL_DObjCalcSkel( int dobj, int partBits )
{
  DObjCalcSkel( partBits, dobj );
}

typedef struct {
	const char  *name;
	int         a;
	int         surfaceBits;
	int         b;
} cl_surfacetype_t;

extern cl_surfacetype_t dword_571790[24];

#define CL_SURFACETYPE_FIRST    1       /* table[1] == "bark" 0x005717A0 */
#define CL_SURFACETYPE_LIMIT    23      /* table[23] == the bound 0x00571900 */

/* ---- CL_SurfaceTypeFromName  0x00401C60 ----  VERIFIED */
int CL_SurfaceTypeFromName( const char *name ) {
	int i;

	if ( !_stricmp( name, "default" ) ) {
		return 0;
	}

	for ( i = CL_SURFACETYPE_FIRST ; i < CL_SURFACETYPE_LIMIT ; i++ ) {
		if ( !_stricmp( name, dword_571790[i].name ) ) {
			return ( dword_571790[i].surfaceBits >> 20 ) & 0x1F;
		}
	}

	return -1;
}

/* ---- CL_SurfaceTypeToName  0x00401CC0 ----  VERIFIED */
const char *CL_SurfaceTypeToName( int type ) {
	if ( type <= 0 || type >= CL_SURFACETYPE_LIMIT ) {
		return "default";
	}

	return dword_571790[type].name;
}

/* ---- Hunk_AllocXAnimClientCreate  0x00401CE0 ----  [CONFIRMED] */
void *__cdecl Hunk_AllocXAnimClientCreate(unsigned int size)
{
  return Hunk_AllocAlignInternal(size, 32);
}

/* ---- Hunk_AllocXAnimClientCreateTree  0x00401CF0 ----  [CONFIRMED] */
void *__cdecl Hunk_AllocXAnimClientCreateTree(unsigned int size)
{
  return Hunk_AllocAlignInternal(size, 32);
}

extern cvar_t *cl_languagetranslate;          /* 0x01432954 */
extern cvar_t *cl_languagewarnings;           /* 0x01617350 */
extern cvar_t *cl_languagewarningsaserrors;   /* 0x01432844 */

/* ---- CL_SubtitlePrint  0x00401D00 ----  VERIFIED */
void CL_SubtitlePrint( const char *text, int duration, int channel ) {
	byte    *out;
	int     msec;

	if ( cl_languagetranslate && cl_languagetranslate->integer
		 && text[0] && text[1] ) {
		out = (byte *)SE_GetString( text, 0 );
	} else {
		out = (byte *)text;
	}

	if ( !out ) {
		if ( cl_languagewarnings->integer ) {
			if ( cl_languagewarningsaserrors->integer ) {
				Com_Error( 7, "Could not translate subtitle text: \"%s\"", text );
			}
			Com_Printf( "^3WARNING: Could not translate subtitle text: \"%s\"\n",
						text );
			out = (byte *)va( "^1UNLOCALIZED(^7%s^1)^7", text );
		} else {
			out = (byte *)text;
		}
	}

	if ( cl_noprint && cl_noprint->integer ) {
		return;
	}

	if ( con_initialized == qfalse ) {
		Con_OneTimeInit();
	}

	msec = duration;
	if ( !duration ) {
		msec = 5000;
	} else if ( duration < 0 ) {
		msec = 0;
	}

	CL_ConsolePrint_AddLine( out, 3, msec, channel, 7 );
}

/* ---- CL_CgameSystemCalls  0x00401DF0 ----  VERIFIED */
int __cdecl CL_CgameSystemCalls(int *args)
{
  int v1;
  int v2;
  int v3;
  int v4;
  char *v5;
  int v6;
  int v7;
  char *v8;
  unsigned int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  char *v14;
  int v15;
  int v16;
  char *v17;
  unsigned int v18;
  int v19;
  int v20;
  int v21;
  int v22;
  char *v23;
  int v24;
  int v25;
  double v26;
  char *v27;
  __int64 v28;
  const char *v29;
  char *v30;
  char v31;
  unsigned int v32;
  char **v33;
  const char *v34;
  char **v35;
  int v36;
  int v37;
  char *v38;
  char **v39;
  _DWORD *SoundAlias;
  _DWORD *v41;
  signed __int32 v42;
  int *v43;
  double v44;
  double v45;
  char *v46;
  const char *v47;
  const char *v48;
  float *v49;
  int *v50;
  int v51;
  int msgtime;
  int v53;
  int v54;
  int v55;
  int v56;
  int v57;
  float v58;
  int v59;
  float v60;
  int v61;
  float v62;
  int numfiles;
  int source;
  int duration;
  int width;
  char *txt;
  float v73;
  char String1[4];
  char filename[64];
  char Destination[4100];
  unsigned int v77;

  switch ( *args )
  {
    case 0:
      Com_Printf("%s", (const char *)args[1]);
      goto LABEL_326;
    case 1:
      Com_Error(ERR_DROP, "\x15%s", (const char *)args[1]);
    case 2:
      v4 = args[2];
      v5 = (char *)args[1];
      width = v4;
      if ( cl_noprint && cl_noprint->integer )
        goto LABEL_326;
      if ( con_initialized == qfalse )
        Con_OneTimeInit();
      v58 = *(float *)(con_gamemessagetime + 28) * 1000.0;
      v6 = FastRound(v58);
      duration = v6;
      if ( v6 < 0 )
      {
        v6 = 0;
        *(float *)&duration = 0.0;
      }
      source = 7;
      if ( strstr(v5, "\n") )
      {
        v8 = strstr(v5, "\n");
        do
        {
          if ( *v5 == 10 )
          {
            Con_Linefeed(duration, 1);      /* 0x00401EDC: eax=edi, edx=1 */
            ++v5;
          }
          else
          {
            v9 = v8 - v5 + 1;
            if ( v9 >= 0x1000 )
            {
              Com_Printf("Text line too long. Clipping to fit\n");
              v9 = 4096;
            }
            strncpy(Destination, v5, v9 - 1);
            v10 = duration;
            v59 = source;
            v55 = width;
            v53 = duration;
            filename[v9 + 63] = 0;
            source = CL_ConsolePrint_AddLine((byte *)Destination, 1, v53, v55, v59);
            v6 = duration;
            v5 = v8;
            if ( *v8 == 10 )
              v5 = v8 + 1;
          }
          if ( !strstr(v5, "\n") )
          {
            CL_ConsolePrint_AddLine((byte *)v5, 1, v6, width, source);
            goto LABEL_326;
          }
          if ( !v5 )
            break;
          if ( !*v5 )
            break;
          v8 = strstr(v5, "\n");
        }
        while ( v8 );
      }
      
      else
      {
          if (cl_whitetext && cl_whitetext->integer) {
              CL_ConsolePrint_AddLine((byte*)v5, 1, v6, v4, 7);
          }
          else {
              CL_ConsolePrint_AddLine((byte*)v5, 4, v6, v4, 7);
          }
      }
      goto LABEL_326;
    case 3:
      v13 = args[2];
      v14 = (char *)args[1];
      width = v13;
      if ( cl_noprint && cl_noprint->integer )
        goto LABEL_326;
      if ( con_initialized == qfalse )
        Con_OneTimeInit();
      v60 = *(float *)(con_boldgamemessagetime + 28) * 1000.0;
      v15 = FastRound(v60);
      duration = v15;
      if ( v15 < 0 )
      {
        v15 = 0;
        *(float *)&duration = 0.0;
      }
      source = 7;
      if ( strstr(v14, "\n") )
      {
        v17 = strstr(v14, "\n");
        do
        {
          if ( *v14 == 10 )
          {
            Con_Linefeed(duration, 2);      /* 0x0040204C: eax=edi, edx=2 */
            ++v14;
          }
          else
          {
            v18 = v17 - v14 + 1;
            if ( v18 >= 0x1000 )
            {
              Com_Printf("Text line too long. Clipping to fit\n");
              v18 = 4096;
            }
            strncpy(Destination, v14, v18 - 1);
            v19 = source;
            v61 = source;
            v56 = width;
            v54 = duration;
            filename[v18 + 63] = 0;
            source = CL_ConsolePrint_AddLine((byte *)Destination, 2, v54, v56, v61);
            v15 = duration;
            v14 = v17;
            if ( *v17 == 10 )
              v14 = v17 + 1;
          }
          if ( !strstr(v14, "\n") )
          {
            CL_ConsolePrint_AddLine((byte *)v14, 2, v15, width, source);
            goto LABEL_326;
          }
          if ( !v14 )
            break;
          if ( !*v14 )
            break;
          v17 = strstr(v14, "\n");
        }
        while ( v17 );
      }
      else
      {
        CL_ConsolePrint_AddLine((byte *)v14, 2, v15, v13, 7);
      }
      goto LABEL_326;
    case 4:
      v21 = args[7];
      v22 = args[6];
      v23 = (char *)args[1];
      source = args[8];
      txt = (char *)args[5];
      width = args[2];
      v73 = *(float *)&v21;
      v24 = args[4];
      duration = v22;
      v25 = args[3];
      *(_DWORD *)String1 = v24;
      numfiles = v25;
      if ( !cl_noprint || !cl_noprint->integer )
      {
        if ( con_initialized == qfalse )
          Con_OneTimeInit();
        /* retail keeps this in EBX for the whole block */
        v62 = *(float *)(con_gamemessagetime + 28) * 1000.0;
        msgtime = FastRound(v62);
        if ( msgtime < 0 )
          msgtime = 0;
        if ( con_display > 0 )
          Con_Linefeed(msgtime, con_prevChannel);   /* 0x004021B4 */
        if ( *v23 )
        {
          /* 0x004021C6: esi = args[2], eax = 10 */
          CL_AddConsoleInfoColor(10, (const float *)width);
          CL_AddDeathMessageText(v23, 7);
          CL_AddDeathMessageText(" ", 7);
        }
        /* 0x004021EF: esi = args[8], eax = 13 */
        CL_AddConsoleInfoColor(13, (const float *)source);
        v26 = v73 * 32.0;
        con_text[con_display + con_linewidth * (con_current % con_totallines)] = (unsigned __int64)(*(float *)&duration
                                                                                                  * 32.0)
                                                                               | 0x1000;
        ++con_display;
        v27 = txt;
        con_text[con_display + con_linewidth * (con_current % con_totallines)] = (unsigned __int64)v26 | 0x1100;
        ++con_display;
        CL_AddDeathMessageText(v27, 18);
        CL_AddDeathMessageText(" ", 7);
        /* 0x0040229E: esi = args[4], eax = 10 */
        CL_AddConsoleInfoColor(10, (const float *)*(_DWORD *)String1);
        CL_AddDeathMessageText((char *)numfiles, 7);
        Con_Linefeed(msgtime, 1);                   /* 0x004022BD */
        con_prevChannel = 1;
      }
      goto LABEL_326;
    case 5:
      CL_SubtitlePrint((const char *)args[1], args[2], args[3]);
      goto LABEL_326;
    case 6:
      LODWORD(v28) = Sys_Milliseconds();
      return v28;
    case 7:
      Cvar_Register((vmCvar_t *)args[1], (const char *)args[2], (const char *)args[3], args[4]);
      goto LABEL_326;
    case 8:
      Cvar_Update((vmCvar_t *)args[1]);
      goto LABEL_326;
    case 9:
      Cvar_Set2((const char *)args[1], (const char *)args[2], qtrue);
      goto LABEL_326;
    case 10:
      Cvar_VMSet((const char *)args[2], (vmCvar_t *)args[1]);
      goto LABEL_326;
    case 11:
      Cvar_VariableStringBuffer((const char *)args[1], (char *)args[2], args[3]);
      goto LABEL_326;
    case 12:
      LODWORD(v28) = cmd_argc;
      return v28;
    case 13:
      Cmd_ArgvBuffer(args[1], (char *)args[2], args[3]);
      goto LABEL_326;
    case 14:
      Cmd_ArgsBuffer((char *)args[1], args[2]);
      goto LABEL_326;
    case 15:
      LODWORD(v28) = FS_FOpenFileByMode((const char *)args[1], (fileHandle_t *)args[2], (fsMode_t)args[3]);
      return v28;
    case 16:
      LODWORD(v28) = FS_Read((void *)args[1], args[2], args[3]);
      return v28;
    case 17:
      LODWORD(v28) = FS_Write((const void *)args[1], args[2], args[3]);
      return v28;
    case 18:
      FS_FCloseFile(args[1]);
      goto LABEL_326;
    case 19:
      /* case 19: retail loads args[1] into EAX and pushes args[2..4] (0x00402444). */
      LODWORD(v28) = FS_GetFileList((const char *)args[1], (const char *)args[2],
                                    (char *)args[3], args[4]);
      return v28;
    case 20:
      return Com_SaveCvarsToBuffer((const char **)args[1], args[2],
                                   (char *)args[3], args[4]);
    case 21:
      return Com_LoadCvarsFromBuffer((const char **)args[1], args[2],
                                     (char *)args[3], (const char *)args[4]);
    case 22:
      Cbuf_AddText((const char *)args[1]);   /* text@<eax>, 0x004024CD */
      goto LABEL_326;
    case 23:
      Cmd_AddCommand((const char *)args[1], 0);
      goto LABEL_326;
    case 24:
      CL_AddReliableCommand((const char *)args[1]);
      goto LABEL_326;
    case 25:
/* case 25 is one instruction in retail (0x0040250F is the call). SCR_UpdateScreen takes no arguments. */
      SCR_UpdateScreen();
      goto LABEL_326;
    /* cases 26-30: yStart/y is args[1], loaded into ECX (EDI for case 30) at the retail sites. */
    case 26:
      Con_DrawNotify(args[2], *((float *)args + 3), args[4], args[1]);
      goto LABEL_326;
    case 27:
      Con_DrawBoldMessages(args[2], *((float *)args + 3), args[4], args[1]);
      goto LABEL_326;
    case 28:
      Con_DrawMiniConsole(args[1], args[2], *((float *)args + 3));
      goto LABEL_326;
    case 29:
      Con_DrawSubtitles(args[2], *((float *)args + 3), args[4], args[1]);
      goto LABEL_326;
    case 30:
      Con_DrawSay(args[1]);
      goto LABEL_326;
    case 31:
      CL_CM_LoadMap((const char *)args[1]);   /* mapname@<esi>, 0x00402587 */
      goto LABEL_326;
    case 32:
      LODWORD(v28) = cm_numSubModels;
      return v28;
    case 33:
      LODWORD(v28) = CM_InlineModel(args[1]);   /* index@<esi>, 0x004025AF */
      return v28;
    case 35:
      LODWORD(v28) = CM_TempBoxModel((const float *)args[1], (const float *)args[2],
                                     args[3], qfalse);
      return v28;
    case 36:
      LODWORD(v28) = CM_PointContents((const float *)args[1], args[2]);
      return v28;
    case 37:
/* p@<ecx>=args[1], model@<stack>=args[2], origin@<eax>=args[3], angles@<edx>=args[4] -- 0x00402642. */
      LODWORD(v28) = CM_TransformedPointContents((const float *)args[1], args[2],
                                                 (const float *)args[3],
                                                 (const float *)args[4]);
      return v28;
    case 38:
      CM_BoxTrace(
        (trace_t *)args[1],
        (const float *)args[2],
        (const float *)args[3],
        (const float *)args[4],
        (const float *)args[5],
        args[6],
        args[7],
        qfalse);
      goto LABEL_326;
    case 40:
      CM_BoxTrace(
        (trace_t *)args[1],
        (const float *)args[2],
        (const float *)args[3],
        (const float *)args[4],
        (const float *)args[5],
        args[6],
        args[7],
        qtrue);
      goto LABEL_326;
    case 39:
      CM_TransformedBoxTraceExternal(
        (trace_t *)args[1],
        (const float *)args[2],
        (const float *)args[3],
        (const float *)args[4],
        (const float *)args[5],
        args[6],
        args[7],
        (const float *)args[8],
        (const float *)args[9],
        qfalse);
      goto LABEL_326;
    case 41:
      CM_TransformedBoxTraceExternal(
        (trace_t *)args[1],
        (const float *)args[2],
        (const float *)args[3],
        (const float *)args[4],
        (const float *)args[5],
        args[6],
        args[7],
        (const float *)args[8],
        (const float *)args[9],
        qtrue);
      goto LABEL_326;
    case 42:
/* case 42 is case 35 with capsule = 1 (0x004025FD pushes 1). */
      LODWORD(v28) = CM_TempBoxModel((const float *)args[1], (const float *)args[2],
                                     args[3], qtrue);
      return v28;
    case 43:
      LODWORD(v28) = dword_14328F8(
                       args[1],
                       args[2],
                       args[3],
                       args[4],
                       args[5],
                       args[6],
                       args[7],
                       args[8],
                       args[9],
                       args[10]);
      return v28;
    case 44:
      dword_1432878(args[1], 0);
      goto LABEL_326;
    case 45:
      LODWORD(v28) = dword_143286C(args[1], args[2]);
      return v28;
    case 46:
      ((void (__cdecl *)())dword_143287C)();
      goto LABEL_326;
    case 47:
      dword_1432880(args[1]);
      goto LABEL_326;
    case 48:
      LODWORD(v28) = re_RegisterShader(args[1], args[2]);
      return v28;
    case 49:
      LODWORD(v28) = dword_1432888(args[1], args[2]);
      return v28;
    case 50:
      LODWORD(v28) = dword_1432868(args[1]);
      return v28;
    case 51:
      dword_1432908(args[1], args[2], args[3], args[4]);
      goto LABEL_141;
    case 52:
      LODWORD(v28) = dword_143292C(args[1], args[2], args[3], 0, args[4]);
      return v28;
    case 53:
      LODWORD(v28) = dword_1432930(args[1], args[2]);
      return v28;
    case 54:
      dword_1432934(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
      goto LABEL_326;
    case 55:
      dword_1432940(
        args[1],
        args[2],
        args[3],
        args[4],
        args[5],
        args[6],
        args[7],
        *((unsigned __int8 *)args + 32),
        0,
        args[9],
        args[10]);
      goto LABEL_326;
    case 56:
      LODWORD(v28) = SEH_StringEd_GetString((const char *)args[1]);
      return v28;
    case 57:
      LODWORD(v28) = SEH_LocalizeTextMessage((char *)args[1], (const char *)args[2], 0);
      return v28;
    case 58:
      LODWORD(v28) = SEH_PrintStrlen((char *)args[1]);
      return v28;
    case 59:
      LODWORD(v28) = SEH_ReadCharFromString((unsigned __int8 **)args[1], (_DWORD *)args[2]);
      return v28;
    case 60:
      ((void (__cdecl *)())dword_143289C)();
      goto LABEL_326;
    case 61:
      dword_14328A0(args[1], 0);
      goto LABEL_326;
    case 62:
      LODWORD(v28) = dword_143290C(args[1], args[2]);
      return v28;
    case 63:
      LODWORD(v28) = com_hunkMeminfoRunningTotal;
      return v28;
    case 64:
      dword_14328A4(args[1], args[2], args[3]);
      goto LABEL_326;
    case 65:
      dword_14328A8(args[1], args[2], args[3], args[4]);
      goto LABEL_326;
    case 66:
      dword_14328AC(args[1], args[2], args[3], args[4], args[5]);
      goto LABEL_326;
    case 67:
      dword_14328B4(args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
      goto LABEL_326;
    case 68:
      dword_14328B8(args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
      goto LABEL_326;
    case 69:
      dword_14328C4(args[1]);
      goto LABEL_326;
    case 70:
      ((void (__cdecl *)())dword_14328F0)();
      goto LABEL_326;
    case 71:
      dword_14328F4(args[1]);
      goto LABEL_326;
    case 72:
      re_SetColor(args[1]);
      goto LABEL_326;
    case 73:
      re_DrawStretchPic(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
      goto LABEL_326;
    case 74:
      dword_14328D4(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11]);
      goto LABEL_326;
    case 75:
      dword_14328D8(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
      goto LABEL_326;
    case 76:
      dword_14328DC(args[1], args[2], args[3]);
      goto LABEL_326;
    case 77:
      dword_14328FC(args[1], args[2], args[3]);
      goto LABEL_326;
    case 78:
      qmemcpy((void *)args[1], &cls_glconfig, 0xA0u);
      goto LABEL_326;
    case 300:
      CL_GetExtendedGameState((void *)args[1], args[2]);
      goto LABEL_326;
    case 79:
      CL_GetGameState((void *)args[1]);
      goto LABEL_326;
    case 80:
      v41 = (_DWORD *)args[2];
      *(_DWORD *)args[1] = *(_DWORD *)cl_snap_messageNum;
      *v41 = cl_snap_serverTime;
      goto LABEL_326;
    case 301:
      if ( args[3] != 0x127EC + PLAYERSTATE_EXTRA_BYTES ) {
        Com_Error( ERR_DROP, "Cgame snapshot layout mismatch" );
      }
      LODWORD(v28) = CL_GetSnapshot(args[1], (byte *)args[2]);   /* snapshotNumber@<eax>, 0x00402F71 */
      return v28;
    case 81:
      Com_Error( ERR_DROP, "Outdated cgame snapshot interface (81). Loaded DLL:\n%s\nInstall the updated cgame in the active game folder/PK3.", VM_DllPath( cgvm ) );
      return 0;
    case 82:
      LODWORD(v28) = CL_GetServerCommand(args[1]);   /* @<eax>, 0x00402F96 */
      return v28;
    case 83:
      LODWORD(v28) = cl_cmdNumber;
      return v28;
    case 84:
      LODWORD(v28) = CL_GetUserCmd(args[1], (byte *)args[2]);   /* cmdNumber@<eax>, 0x00402FCF */
      return v28;
    case 85:
      v42 = args[1];
      cgameSensitivity = *((float *)args + 2);
      cgameUserCmdValue = v42;
      goto LABEL_326;
    case 86:
      v43 = (int *)args[1];
      cgameUserAim_x = *v43;
      cgameUserAim_y = v43[1];
      cgameUserAim_z = v43[2];
      goto LABEL_326;
    case 87:
      cgameUserCmdInShellshock = args[1];
      goto LABEL_326;
    case 88:
LABEL_141:
      LODWORD(v28) = Material_RegisterHandle(args[1], args[2]);
      return v28;
    case 89:
      LODWORD(v28) = *(_DWORD *)hunk_totalSize - *(_DWORD *)hunk_highTemp - *(_DWORD *)hunk_temp_temp;
      return v28;
    case 90:
      LODWORD(v28) = Key_IsDown(args[1]);   /* @<eax>, 0x00403084 */
      return v28;
    case 91:
      LODWORD(v28) = cls_keyCatchers;
      return v28;
    case 92:
      Key_SetCatcher(args[1]);
      goto LABEL_326;
    case 93:
      LODWORD(v28) = Key_GetKey((const char *)args[1]);   /* binding@<ebx>, 0x004030CA */
      return v28;
    case 94:
      LODWORD(v28) = PC_AddGlobalDefine(0, (const char *)args[1]);
      return v28;
    case 95:
      LODWORD(v28) = PC_LoadSourceHandle((const char *)args[1]);
      return v28;
    case 96:
      LODWORD(v28) = PC_FreeSourceHandle(args[1]);
      return v28;
    case 97:
      LODWORD(v28) = PC_ReadTokenHandle(args[1], (pc_token_t *)args[2]);
      return v28;
    case 98:
      LODWORD(v28) = PC_SourceFileAndLine(args[1], (char *)args[2], (int *)args[3]);
      return v28;
    case 99:
      LODWORD(v28) = Com_RealTime((int *)args[1]);
      return v28;
    case 100:
      Sys_SnapVector((float *)args[1]);
      goto LABEL_326;
    case 101:
      Cmd_RemoveCommand((const char *)args[1]);
      goto LABEL_326;
    case 102:
      LODWORD(v28) = CIN_PlayCinematic((const char *)args[1], args[2], args[3], args[4], args[5], args[6]);
      return v28;
    case 103:
      LODWORD(v28) = CIN_StopCinematic(args[1]);   /* handle@<ecx>, 0x004033A0 */
      return v28;
    case 104:
      LODWORD(v28) = CIN_RunCinematic(args[1]);
      return v28;
    case 105:
      CIN_DrawCinematic(args[1]);
      goto LABEL_326;
    case 106:
/* retail loads eax=args[1] and edx=args[3] and pushes args[2], args[4], args[5]; EDX is y. Q3's order is (handle,x,y,w,h). */
      CIN_SetExtents(args[1], args[2], args[3], args[4], args[5]);
      goto LABEL_326;
    case 107:
      dword_1432900(args[1]);
      goto LABEL_326;
    case 108:
      LODWORD(v28) = dword_1432904(args[1], args[2], args[3], args[4], args[5], args[6]);
      return v28;
    case 109:
      v46 = (char *)args[1];
      LOBYTE(v2) = *((_BYTE *)args + 8);
      BYTE1(v2) = v2;
      HIDWORD(v28) = args[3];
      LODWORD(v28) = v2 << 16;
      LOWORD(v28) = v2;
      memset32(v46, v28, HIDWORD(v28) >> 2);
      memset(&v46[4 * (HIDWORD(v28) >> 2)], v2, BYTE4(v28) & 3);
      LODWORD(v28) = v46;
      return v28;
    case 110:
      LODWORD(v28) = args[1];
      qmemcpy((void *)v28, (const void *)args[2], args[3]);
      return v28;
    case 111:
      LODWORD(v28) = strncpy((char *)args[1], (const char *)args[2], args[3]);
      return v28;
    case 112:
      *(float *)&numfiles = sin(*((float *)args + 1));
      LODWORD(v28) = numfiles;
      return v28;
    case 113:
      *(float *)&numfiles = cos(*((float *)args + 1));
      LODWORD(v28) = numfiles;
      return v28;
    case 114:
      *(float *)&numfiles = atan2(*((float *)args + 1), *((float *)args + 2));
      LODWORD(v28) = numfiles;
      return v28;
    case 115:
      *(float *)&numfiles = sqrt(*((float *)args + 1));
      LODWORD(v28) = numfiles;
      return v28;
    case 116:
      *(float *)&numfiles = floor(*((float *)args + 1));
      LODWORD(v28) = numfiles;
      return v28;
    case 117:
      *(float *)&numfiles = ceil(*((float *)args + 1));
      LODWORD(v28) = numfiles;
      return v28;
    case 118:
      Com_Printf("%s%i\n", (const char *)args[1], args[2]);
      goto LABEL_326;
    case 119:
      Com_Printf("%s%f\n", (const char *)args[1], *((float *)args + 2));
      goto LABEL_326;
    case 120:
      *(float *)&numfiles = Q_acos(*((float *)args + 1));
      LODWORD(v28) = numfiles;
      return v28;
    case 121:
      LODWORD(v28) = VM_Call(uivm, 15, args[1], 5);
      return v28;
    case 122:
      if ( cls_state != 6 )
        goto LABEL_216;
      if ( clc_demoplaying )
        goto LABEL_216;
      v47 = (const char *)args[1];
      if ( !v47 )
        goto LABEL_216;
      if ( !Q_stricmp("UIMENU_WM_QUICKMESSAGE", v47) )
      {
        VM_Call(uivm, 7, 8);
LABEL_216:
        LODWORD(v28) = 1;
        return v28;
      }
      if ( !Q_stricmp("UIMENU_WM_AUTOUPDATE", v47) )
      {
        VM_Call(uivm, 7, 9);
        goto LABEL_216;
      }
/* 0x00403562: eax=0x13, ecx="UIMENU_SCRIPT_POPUP", edx=esi. Q_stricmpn is (s1@<edx>, s2@<ecx>, n@<eax>) with NO stack arguments. */
      if ( Q_stricmpn(v47, "UIMENU_SCRIPT_POPUP", 19) )
        goto LABEL_216;
      if ( Q_stricmp("UIMENU_SCRIPT_POPUP_NO_MOUSE", v47) )
        LODWORD(v28) = VM_Call(uivm, 7, 10);
      else
        LODWORD(v28) = VM_Call(uivm, 7, 11);
      return v28;
    case 123:
      v48 = (const char *)args[1];
      if ( v48
        && (!Q_stricmp("UIMENU_SCRIPT_POPUP_NO_MOUSE", v48) && VM_Call(uivm, 8) == 10
         || !Q_stricmp("UIMENU_SCRIPT_POPUP", (const char *)args[1]) && VM_Call(uivm, 8) == 10) )
      {
        goto LABEL_231;
      }
      goto LABEL_326;
    case 124:
      if ( VM_Call(uivm, 8) == 1 )
        goto LABEL_326;
      VM_Call(uivm, 3, 27, 1);
      VM_Call(uivm, 3, 27, 1);
LABEL_231:
      VM_Call(uivm, 3, 27, 1);
      goto LABEL_326;
    case 125:
      LODWORD(v28) = VM_Call(uivm, 9, args[1]);
      return v28;
    case 126:
      LODWORD(v28) = VM_Call(uivm, 10, args[1]);
      return v28;
    case 127:
/* 0x004036D1 is the whole case: no arguments, and the returned pointer is the trap's result. */
      LODWORD(v28) = CL_GetServerIPAddress();
      return v28;
    case 128:
      XAnimPrecacheAnimTree_m((int *)args[1], (int (__cdecl *)(int))Hunk_AllocXAnimPrecache);
      goto LABEL_326;
    case 129:
      LODWORD(v28) = XAnimAllocTree(args[2], args[1], (int (__cdecl *)(int))Hunk_AllocXAnimClientCreate);
      return v28;
    case 130:
      XAnimLoadFileData_m(args[3], args[1], *((_WORD *)args + 4));
      goto LABEL_326;
    case 131:
      LODWORD(v28) = XAnimAllocRuntimeTree(args[1], (int (__cdecl *)(int))Hunk_AllocXAnimClientCreateTree);
      return v28;
    case 132:
      XAnimSetParentNode(
        *((_WORD *)args + 10),
        *((_WORD *)args + 12),
        args[1],
        *((_WORD *)args + 4),
        args[3],
        *((_WORD *)args + 8));
      goto LABEL_326;
    case 133:
      DObjFree(1u, args[1]);
      goto LABEL_326;
    case 134:
      XAnimClearAnimNode_m((_DWORD *)args[1], args[2], *((float *)args + 3));
      goto LABEL_326;
    case 135:
      XAnimClearAnim_m(args[2], (_DWORD *)args[1], *((float *)args + 3));
      goto LABEL_326;
    case 136:
      XAnimClearAnimChildSubtrees_m((void *)args[1], args[2], *((float *)args + 3));
      goto LABEL_326;
    case 137:
      XAnimSetAnimKnob_m(
        (_DWORD *)args[1],
        args[2],
        *((float *)args + 3),
        args[4],
        *((float *)args + 5),
        *((_WORD *)args + 12),
        0,
        args[7]);
      goto LABEL_326;
    case 138:
      LODWORD(v28) = XAnimSetAnimKnobAll_m(
                       args[2],
                       (_DWORD *)args[1],
                       args[3],
                       *((float *)args + 4),
                       args[5],
                       *((float *)args + 6),
                       *((_WORD *)args + 14),
                       0,
                       args[8]);
      return v28;
    case 139:
      /* nodeIndex (args[2]) is live-in via ECX in retail. */
      XAnimClearAnimChildren_m((_DWORD *)args[1], args[2], *((float *)args + 3));
      goto LABEL_326;
    case 140:
      XAnimSetAnimInternalLimited_m(
        *((_WORD *)args + 12),
        0,
        args[1],
        args[2],
        *((float *)args + 3),
        args[4],
        *((float *)args + 5),
        args[7]);
      goto LABEL_326;
    case 141:
      XAnimSetAnimKnobInternal_m(
        0,
        (_DWORD *)args[1],
        args[2],
        *((float *)args + 3),
        args[4],
        *((float *)args + 5),
        *((_WORD *)args + 12),
        args[7]);
      goto LABEL_326;
    case 142:
      XAnimSetAnimRate_m(args[1], args[2], *((float *)args + 3));
      goto LABEL_326;
    case 143:
      LODWORD(v28) = XAnimIsLooped_m(args[2], scrAnimPub_trees[128 * xanim_activePoolSlot + args[1]]);
      return v28;
    case 144:
      LODWORD(v28) = XAnimIsLooped_m(args[2], args[1]);
      return v28;
    case 145:
      XAnimSetTime_m((int *)args[2], args[1], args[3]);
      goto LABEL_326;
    case 146:
      *(float *)&numfiles = XAnimGetTime(args[2], args[1]);
      LODWORD(v28) = numfiles;
      return v28;
    case 147:
      *(float *)&numfiles = XAnimGetWeight(args[2], args[1]);
      LODWORD(v28) = numfiles;
      return v28;
    case 148:
      CL_DObjInvalidateSkels();   /* 0x004039E3 sets nothing; no arguments */
      goto LABEL_326;
    case 149:
      DObjUpdateClientInfo_m((void *)args[1], *((float *)args + 2));
      goto LABEL_326;
    case 150:
      LODWORD(v28) = xanim_numDeferredNotifies;
      *(_DWORD *)args[1] = &xanim_deferredNotifies;
      return v28;
    case 151:
      DObjCalcAnim(args[1], args[2]);   /* dobj@<eax>, partBits@<edx>, 0x00403A25 */
      goto LABEL_326;
    case 152:
      DObjDisplayAnim((int **)args[1]);
      goto LABEL_326;
    case 153:
      XAnimGetRelDelta_m(args[5], args[2], (int *)args[4], (float *)args[3], (_DWORD *)args[1]);
      goto LABEL_326;
    case 154:
      XAnimGetAbsDelta_m(args[2], (_DWORD *)args[1], (_DWORD *)args[4], (float *)args[3]);
      goto LABEL_326;
    case 155:
      XAnimGetRelDeltaForTime_m(
        args[2],
        scrAnimPub_trees[128 * xanim_activePoolSlot + args[1]],
        (_DWORD *)args[4],
        (float *)args[3],
        *((float *)args + 5),
        *((float *)args + 6));
      goto LABEL_326;
    case 156:
      XAnimGetAbsDeltaForTime_m(
        args[2],
        scrAnimPub_trees[128 * xanim_activePoolSlot + args[1]],
        (float *)args[3],
        (_DWORD *)args[4],
        *((float *)args + 5));
      goto LABEL_326;
    case 157:
      LODWORD(v28) = (*(unsigned __int8 *)(args[2] + args[1] + 80) << 6) + *(_DWORD *)(args[1] + 4) + 48;
      return v28;
    case 158:
      LODWORD(v28) = (*(unsigned __int8 *)(args[1] + 23) << 6) + *(_DWORD *)(args[1] + 4) + 48;
      return v28;
    case 159:
      LODWORD(v28) = DObjMarkRotTransIndex((void *)args[1], (const unsigned char *)args[2], args[3]);
      return v28;
    case 160:
      LODWORD(v28) = DObjMarkControlRotTransIndex((void *)args[1], (const unsigned char *)args[2], args[3]);
      return v28;
    case 161:
      LODWORD(v28) = XAnimGetAnimName(args[2], scrAnimPub_trees[128 * xanim_activePoolSlot + args[1]]);
      return v28;
    case 162:
      LODWORD(v28) = (int)Com_GetClientDObj(args[1]);   /* handle@<eax>, 0x00403BB3 */
      return v28;
    case 163:
      DObjCreate(0, args[4], args[1], *((_WORD *)args + 4), (_DWORD *)args[3]);
      goto LABEL_326;
    case 164:
      Com_ClientDObjCreate((void *)args[1], (unsigned __int16)args[2],
                            (void *)args[3], args[4], 0);
      goto LABEL_326;
    case 165:
      Com_SafeClientDObjFree(args[1], args[2]);
      goto LABEL_326;
    case 166:
    case 177:
      LODWORD(v28) = *(_DWORD *)args[1];
      return v28;
    case 167:
      LODWORD(v28) = 96 * *(unsigned __int8 *)(args[1] + 23) + 48;
      return v28;
    case 168:
      LODWORD(v28) = CL_DObjCreateSkelForBone(args[1], args[2]);
      return v28;
    case 169:
      LODWORD(v28) = CL_DObjCreateSkelForBones(args[1], args[2]);
      return v28;
    case 170:
      DObjGetHierarchyBits(args[2], args[1], (_DWORD *)args[3]);
      goto LABEL_326;
    case 171:
      DObjCalcSkel(args[2], args[1]);   /* partBits@<eax>, dobj@<stack>, 0x00403CA5 */
      goto LABEL_326;
    case 172:
      LODWORD(v28) = XModelExists((int *)args[1]);
      return v28;
    case 173:
      LODWORD(v28) = *(unsigned __int8 *)(args[1] + 23);
      return v28;
    case 174:
      LODWORD(v28) = DObjGetBoneIndex((const char *)args[2], args[1]);
      return v28;
    case 175:
      LODWORD(v28) = DObjGetBoneName(args[1], args[2]);
      return v28;
    case 176:
      DObjBuildPartCollisionTable((_DWORD *)args[2], args[1]);
      goto LABEL_326;
    case 178:
      LODWORD(v28) = *(_WORD *)((int)scrAnimPub_trees[128 * xanim_activePoolSlot + args[1]] + 8 * args[2] + 8) == 0;
      return v28;
    case 179:
      v49 = *(float **)(*(_DWORD *)(args[1] + 8 * args[2] + 12) + 4);
      return (unsigned __int64)((double)*(unsigned __int16 *)v49 / v49[1] * 1000.0);
    case 180:
      LODWORD(v28) = XAnimHasTime(args[2], args[1]);
      return v28;
    case 181:
      LODWORD(v28) = *(unsigned __int16 *)((int)scrAnimPub_trees[128 * xanim_activePoolSlot + args[1]] + 8 * args[2] + 8);
      return v28;
    case 182:
      LODWORD(v28) = args[3] + *(unsigned __int16 *)((int)scrAnimPub_trees[128 * xanim_activePoolSlot + args[1]] + 8 * args[2] + 14);
      return v28;
    case 183:
      LODWORD(v28) = *(_DWORD *)(args[1] + 4);
      return v28;
    case 184:
      XAnimCopyTree(args[1], args[2]);
      goto LABEL_326;
    case 185:
      if ( com_developer->integer )
        DObjDumpInfo((void *)args[1]);
      goto LABEL_326;
    case 186:
      StatMon_Warning(args[1], args[2], (const char *)args[3]);
      goto LABEL_326;
    case 187:
      /* StatMon_GetStatsArray inlined in retail at 0x00403ED6 -- it is four instructions, so the compiler pasted it. `stats` is its private array and is not declared outside statmonitor.c. */
      StatMon_GetStatsArray( (const statmonitor_t **)args[1], (int *)args[2] );
      goto LABEL_326;
    case 188:
      LODWORD(v28) = Z_MallocInternal(args[1]);
      return v28;
    case 189:
      free((void *)args[1]);
      goto LABEL_326;
    case 190:
      v29 = (const char *)args[1];
      source = 0;
      /* The locale-1-aliases-locale-2 fast path of Com_LoadSoundAliases (0x00433EA6..0x00433EEE), inlined. 0x00893DF4 is the localized set name; `snd_subtitleStr` is a misnomer. */
      if ( !_stricmp(snd_aliasLocalizedName, v29) )
      {
        snd_aliasTable[SND_LOCALE_INGAME]      = snd_aliasTable[SND_LOCALE_LOCALIZED];
        snd_aliasTableCount[SND_LOCALE_INGAME] = snd_aliasTableCount[SND_LOCALE_LOCALIZED];
        Com_Memcpy(snd_aliasHashTable[SND_LOCALE_INGAME],
                   snd_aliasHashTable[SND_LOCALE_LOCALIZED],
                   SND_ALIAS_HASH_SLICE_BYTES);
      }
      else
      {
        if ( !_strnicmp(v29, "maps/", 5u) )
        {
          v30 = (char *)(v29 + 5);
          do
          {
            v31 = *v30;
            v30[filename - (v29 + 5)] = *v30;
            ++v30;
          }
          while ( v31 );
          v32 = strlen(filename);
          if ( !_stricmp(&String1[v32], ".bsp") )
            String1[v32] = 0;
        }
        else
        {
          strcpy(filename, v29);
        }
        strlwr(filename);
        v33 = FS_ListFilteredFiles("soundaliases", "csv", 0, &numfiles);
        v34 = (const char *)numfiles;
        v35 = v33;
        if ( !numfiles )                /* 0x004028EC is `test edi,edi` */
        {
          Com_Printf("WARNING: can't find any sound alias files (soundaliases/*.csv)\n");
          goto LABEL_326;
        }
        v36 = 0;
        hunk_lowTempMark = *(_DWORD *)hunk_temp_temp;
        if ( numfiles > 0 )
        {
          do
          {
            snd_currentFile = v35[v36];
            source = Com_LoadSoundAliasFile(
                         va("soundaliases/%s", snd_currentFile),
                         filename, source, 1 );
            ++v36;
          }
          while ( v36 < (int)v34 );
          if ( source )
            Com_MakeSoundAliasesPermanent(source, 1);
        }
        *(_DWORD *)hunk_temp_temp = hunk_lowTempMark;
        if ( v35 )
        {
          v38 = *v35;
          if ( *v35 )
          {
            v39 = v35;
            do
            {
              free(v38);
              v38 = v39[1];
              ++v39;
            }
            while ( v38 );
          }
          free(v35);
        }
      }
      if ( !snd_aliasSetLoaded[SND_LOCALE_INGAME] && !snd_aliasSetLoaded[SND_LOCALE_MENU] )
        Cmd_AddCommand("snd_list", Com_SoundList_f);
      snd_aliasSetLoaded[SND_LOCALE_INGAME] = 1;
      Com_LoadSoundAliasSounds(1);
LABEL_326:
      LODWORD(v28) = 0;
      return v28;
    case 191:
      SoundAlias = (_DWORD *)Com_FindSoundAlias((const char *)args[1], 1);
      if ( !SoundAlias )
        goto LABEL_326;
      LODWORD(v28) = *SoundAlias;
      return v28;
    case 192:
      LODWORD(v28) = (int)Com_PickSoundAlias((const char *)args[1], 1);
      return v28;
    case 193:
      LODWORD(v28) = (int)Com_GetSoundAlias(1, args[1]);
      return v28;
    case 194:
      LODWORD(v28) = MSS_PlaySoundAlias((float *)args[1], args[2], (float *)args[3], args[4]);
      return v28;
    case 195:
      MSS_PlayBlendedSoundAliases((float *)args[2], (_DWORD *)args[1], args[3], args[4], (float *)args[5], args[6]);
      goto LABEL_326;
    case 196:
      LODWORD(v28) = CL_SurfaceTypeFromName((const char *)args[1]);
      return v28;
    case 197:
      LODWORD(v28) = (int)CL_SurfaceTypeToName(args[1]);
      return v28;
    /* CG_ADD_DEBUG_LINE -- CL_CgameSystemCalls case 198, retail 0x00403F22 */

    case 198:
      CL_AddDebugLine(
        (const float *)args[1], (const float *)args[2], (const float *)args[3],
        args[4], args[5], 0 );
      goto LABEL_326;
    case 199:
/* 0x00403F42: edx=args[1] size, esi=args[2] pPrevOwner, edi=2 owner. */
      LODWORD(v28) = (int)Com_GetWeaponInfoMemory(args[1], (int *)args[2], 2);
      return v28;
    case 200:
      Com_FreeWeaponInfoMemory(2, args[1]);
      goto LABEL_326;
    case 201:
      LODWORD(v28) = Hunk_AllocAlignInternal(args[1], 32);
      return v28;
    case 202:
      LODWORD(v28) = Hunk_AllocLowAlignInternal(args[1], 32);
      return v28;
    case 203:
      LODWORD(v28) = Hunk_AllocAlignInternal(args[1], args[2]);
      return v28;
    case 204:
      LODWORD(v28) = Hunk_AllocLowAlignInternal(args[1], args[2]);
      return v28;
    case 205:
    case 206:
      goto LABEL_326;
    case 207:
      LODWORD(v28) = *(_DWORD *)dword_8E6310;
      return v28;
    case 208:
      v44 = *((float *)args + 3);
      v45 = *((float *)args + 2);
      LODWORD(cgameClientLerpOrigin_x) = args[1];
      cgameClientLerpOrigin_y = v45;
      cgameClientLerpOrigin_z = v44;
      goto LABEL_326;
    case 209:
      MSS_SetListener((int *)args[3], (int *)args[2], args[1]);
      goto LABEL_326;
    case 210:
      MSS_UpdateLoopingSounds();
      goto LABEL_326;
    case 211:
      MSS_StopSounds(args[1]);
      goto LABEL_326;
    case 212:
      MSS_PlayMusicAlias((float *)args[1]);
      goto LABEL_326;
    case 213:
      MSS_StopBackground(0, args[1]);
      goto LABEL_326;
    case 214:
      MSS_PlayAmbientAlias(args[1], args[2]);
      goto LABEL_326;
    case 215:
      MSS_FadeAllSounds(args[1], args[2]);
      goto LABEL_326;
    case 216:
      MSS_FadeSelectSounds((float *)args[1], args[2]);
      goto LABEL_326;
    case 217:
      MSS_SetEnvironmentEffects((const char *)args[1], *(float *)&args[2], args[3]);
      goto LABEL_326;
    case 218:
      LODWORD(v28) = MSS_GetSoundOverlay((int *)args[4], args[3], (void *)args[2], args[1]);
      return v28;
    case 219:
      Key_GetBindingBuf(args[1], args[3], (char *)args[2]);
      goto LABEL_326;
    case 220:
      Key_SetBinding(args[1], (const char *)args[2]);
      goto LABEL_326;
    case 221:
      Key_KeynumToStringBuf( args[1], (char *)args[2], args[3] );
      goto LABEL_326;
    case 222:
      LODWORD(v28) = CFxScheduler__RegisterEffect(dword_14075A0, (char *)args[1], 0);
      return v28;
    case 223:
      LODWORD(v28) = FX_GetBoneIndex(args[1], (const char *)args[2]);
      return v28;
    case 224:
      CFxScheduler__PlayEffect_name_simple_m((char *)args[1], dword_14075A0, (float *)args[2]);
      goto LABEL_326;
    case 225:
      CFxScheduler__PlayEffect_name_m((int *)args[3], (char *)args[1], dword_14075A0, (float *)args[2]);
      goto LABEL_326;
    case 226:
      CFxScheduler__PlayEffect_name_bolt_m((char *)args[1], dword_14075A0, (float *)args[2], (int *)args[3], (int *)args[4]);
      goto LABEL_326;
    case 227:
      CFxScheduler__PlayEffect_id_simple_m((float *)args[2], args[1], dword_14075A0);
      goto LABEL_326;
    case 228:
      FX_PlayEffectID((int *)args[3], args[1], (float *)args[2]);
      goto LABEL_326;
    case 229:
      CFxScheduler__PlayEffect_id_axis(dword_14075A0, args[1], (float *)args[2], (int *)args[3], (int *)args[4]);
      goto LABEL_326;
    case 230:
      CFxScheduler__AddScheduledEffects((_DWORD *)dword_14075A0);
      goto LABEL_326;
    case 231:
      LODWORD(v28) = FX_Init();
      return v28;
    case 232:
      LODWORD(v28) = (unsigned __int8)FX_Free(1);
      return v28;
    case 233:
      LODWORD(v28) = (unsigned __int8)FX_Free(0);
      return v28;
    case 234:
      SFxHelper__AdjustTime(&theFxHelper, args[1]);
      goto LABEL_326;
    case 235:
      *(float *)&numfiles = ((double (__cdecl *)())dword_1432894)();
      SFxHelper__AdjustCamera((int)&theFxHelper, args[1], *(float *)&numfiles);
      goto LABEL_326;
    case 240:
      dword_143A980 = args[1];
      dword_143A984 = args[2];
      goto LABEL_326;
    case 241:
      if ( cls_state == 6 )                     /* retail 0x004041B6 cmp cls_state, 6 */
        CL_FirstSnapshot();
      goto LABEL_326;
    default:
      Com_Error(ERR_DROP, "\x15" "Bad cgame system trap: %i", *args);
  }
}

/* ---- CL_UpdateLevelHunkUsage  0x004045C0 ----  VERIFIED */
void CL_UpdateLevelHunkUsage( void ) {
	const char      *memlistfile = "hunkusage.dat";
	fileHandle_t    handle;
	char            *buf;
	char            *outbuf;
	char            *buftrav;
	char            *token;
	char            outstr[256];
	int             len;
	int             memusage;

	memusage = com_hunkMeminfoRunningTotal;

	len = FS_FOpenFileByMode( memlistfile, &handle, FS_READ );
	if ( len >= 0 ) {
		buf = (char *)Z_MallocInternal( len + 1 );
		memset( buf, 0, len + 1 );
		outbuf = (char *)Z_MallocInternal( len + 1 );
		memset( outbuf, 0, len + 1 );

		FS_Read( buf, len, handle );
		FS_FCloseFile( handle );

		buftrav = buf;
		outbuf[0] = '\0';

		while ( ( token = Com_Parse( &buftrav ) ) != 0 && token[0] ) {
			if ( !Q_stricmp( token, cl_mapnameBsp ) ) {
				token = Com_Parse( &buftrav );
				if ( token && token[0] ) {
					if ( atol( token ) == memusage ) {
						free( buf );
						free( outbuf );
						return;
					}
				}
			} else {
				Q_strcat( outbuf, len + 1, token );
				Q_strcat( outbuf, len + 1, " " );
				token = Com_Parse( &buftrav );
				if ( token && token[0] ) {
					Q_strcat( outbuf, len + 1, token );
					Q_strcat( outbuf, len + 1, "\n" );
				} else {
					Com_Error( ERR_DROP, "EXE_ERR_HUNGUSAGE_CORRUPT" );
				}
			}
		}

		handle = FS_FOpenFileWrite( memlistfile );
		if ( handle < 0 ) {
			Com_Error( ERR_DROP,
					   va( "EXE_ERR_CANT_CREATE" "\x15" "%s", memlistfile ) );
		}

		len = strlen( outbuf );
		if ( FS_Write( outbuf, len, handle ) != len ) {
			Com_Error( ERR_DROP,
					   va( "EXE_ERR_CANT_WRITE" "\x15" "%s", memlistfile ) );
		}
		FS_FCloseFile( handle );

		free( buf );
		free( outbuf );
	}

	FS_FOpenFileByMode( memlistfile, &handle, FS_APPEND );
	if ( handle < 0 ) {
		Com_Error( ERR_DROP, "EXE_ERR_HUNKUSAGE_CANT_WRITE" );
	}
	Com_sprintf( outstr, sizeof( outstr ), "%s %i\n", cl_mapnameBsp, memusage );
	FS_Write( outstr, strlen( outstr ), handle );
	FS_FCloseFile( handle );

	if ( FS_FOpenFileByMode( memlistfile, &handle, FS_READ ) >= 0 ) {
		FS_FCloseFile( handle );
	}
}

#define CG_INIT                 0   /* 0x00404AA8 */
#define CG_SCRIPT_HOOK          16  /* 0x00404A86, takes Scr_NearHook(NULL) */

/* ---- CL_InitCGame  0x004049C0 ----  VERIFIED */
void CL_InitCGame( void )
{
	int start;
	int msec;
	const char *info;
	const char *mapname;

	start = Sys_Milliseconds();

	if ( !com_sv_running->integer ) {
		Com_InitDObj();
	}

	/* 0x00A9CC58. qcommon/cmd.c sets the same word before handing a command to the game module. */
	xanim_activePoolSlot = 0;

	Con_Close();

	info = (const char *)&cl_gameState_stringData[cl_gameState_stringOffsets[0]];
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf( cl_mapnameBsp, sizeof( cl_mapnameBsp ), "maps/mp/%s.bsp", mapname );

	cgvm = VM_Create( "cgame", CL_CgameSystemCalls );
	if ( !cgvm ) {
		Com_Error( ERR_DROP, "\x15VM_Create on cgame failed" );
	}

	cls_state = 4;

	VM_Call( cgvm, CG_SCRIPT_HOOK, Scr_NearHook( 0 ) );
	VM_Call( cgvm, CG_INIT, clc_serverMessageSequence, clc_lastExecutedServerCommand, clc_clientNum );

	cls_state = 5;

	msec = Sys_Milliseconds() - start;
	Com_Printf( "CL_InitCGame: %5.2f seconds\n", (double)msec * 0.001 );

	/* retail: unguarded `call ds:[0x01432898]` */
	if ( cgvm_dllEntry ) {
		cgvm_dllEntry();
	}

	if ( sys_sysMBValue > 100663296 ) {   /* 0x6000000 */
		Com_TouchMemory();
	}

	memset( (void *)con_notifyWindow, 0, 4 * con_gamemessagelines );
	memset( (void *)dword_142EFD0, 0, 4 * con_gamemessagelines );
	dword_142EFD8 = 0;
	memset( (void *)con_boldWindow, 0, 4 * dword_142F060 );
	memset( (void *)dword_142F054, 0, 4 * dword_142F060 );
	dword_142F05C = 0;
	memset( (void *)con_subtitleWindow, 0, 4 * dword_142F0E4 );
	memset( (void *)dword_142F0D8, 0, 4 * dword_142F0E4 );
	dword_142F0E0 = 0;

	CL_UpdateLevelHunkUsage();
}
#if 0
void __cdecl CL_InitCGame(const char *systemInfo, const char *mapKey)
{
  unsigned __int32 v2;
  char *v3;
  int *v4;
  vm_s *v5;
  int (__cdecl *v6)(int *);
  const char *ArgList_4;
  int v8;

  if ( !sys_timeBaseInit )
  {
    sys_timeBase = timeGetTime();
    sys_timeBaseInit = 1;
  }
  v2 = timeGetTime() - sys_timeBase;
  if ( !com_sv_running->integer )
    Com_InitDObj();
  xanim_activePoolSlot = 0;
  Con_Close();
  v3 = Info_ValueForKey(systemInfo, mapKey);
  /* Com_sprintf is __usercall(dest@<edi>, size@<esi>, fmt, ...); retail sets
     esi = 0x40 and edi = 0x0143A900. */
  Com_sprintf((char *)&cl_mapname, 64, "maps/mp/%s.bsp", v3);
  cgvm = VM_Create((const char *)CL_CgameSystemCalls, v6);
  if ( !cgvm )
    Com_Error(ERR_DROP, "\x15VM_Create on cgame failed");
  *(_DWORD *)cls_state = 4;
  v4 = Scr_NearHook(0);
  VM_Call(v5, 16, v4);
  VM_Call(cgvm, 0, clc_serverMessageSequence, clc_lastExecutedServerCommand, clc_clientNum);
  *(_DWORD *)cls_state = 5;
  if ( !sys_timeBaseInit )
  {
    sys_timeBase = timeGetTime();
    sys_timeBaseInit = 1;
  }
  v8 = timeGetTime() - sys_timeBase - v2;
  Com_Printf("CL_InitCGame: %5.2f seconds\n", (double)v8 * 0.001);
  cgvm_dllEntry();
  /* Retail 0x004B4A1C is `cmp sys_sysMBValue, 6000000h`, a compare of the value. */
  if ( sys_sysMBValue > 100663296 )       /* 0x6000000 */
    Com_TouchMemory();
  memset((void *)con_notifyWindow, 0, 4 * con_gamemessagelines);
  memset((void *)dword_142EFD0, 0, 4 * con_gamemessagelines);
  dword_142EFD8 = 0;
  memset((void *)con_boldWindow, 0, 4 * dword_142F060);
  memset((void *)dword_142F054, 0, 4 * dword_142F060);
  dword_142F05C = 0;
  memset((void *)con_subtitleWindow, 0, 4 * dword_142F0E4);
  memset((void *)dword_142F0D8, 0, 4 * dword_142F0E4);
  dword_142F0E0 = 0;
  CL_UpdateLevelHunkUsage();
}
#endif

/* ---- CL_GameCommand  0x00404BA0 ----  [CONFIRMED] */
int __cdecl CL_GameCommand()
{
  int result;

  result = (int)cgvm;
  if ( cgvm )
    return VM_Call(cgvm, 2);
  return result;
}

/* ---- CL_CGameRendering  0x00404BC0 ----  [CONFIRMED] */
void __cdecl CL_CGameRendering(int stereoFrame)
{
  VM_Call(cgvm, 3, cl_serverTime, stereoFrame, clc_demoplaying, 0, 0);
}

extern int cl_newSnapshots;             /* cl.newSnapshots 0x01434A78 */
extern int cl_serverTimeDelta;          /* cl.serverTimeDelta 0x01434A70 */
extern int cl_oldServerTime;            /* cl.oldServerTime 0x01434A68 */
extern int cl_oldFrameServerTime;               /* cl.oldFrameServerTime 0x01434A6C */
extern int cl_extrapolatedSnapshot;     /* cl.extrapolatedSnapshot 0x01434A74 */
extern int cls_realtime;                /* cls.realtime 0x0155F3E0 */
extern int cl_snap_valid;               /* cl.snap.valid 0x01432964 */
extern unsigned char cl_snap_snapFlags[4];  /* cl.snap.snapFlags 0x01432968 */
extern char cls_servername[256];        /* cls.servername 0x0155F2CC */
extern int clc_firstDemoFrameSkipped;               /* clc.firstDemoFrameSkipped 0x015EF00C */
extern char clc_timeDemoFrames[12];          /* clc.timeDemoFrames 0x015EF014 */
extern int clc_timeDemoStart;               /* clc.timeDemoStart 0x015EF018 */
extern int clc_timeDemoBaseTime;               /* clc.timeDemoBaseTime 0x015EF01C */
extern cvar_t *activeAction;            /* cl_activeAction 0x0155F24C */
extern cvar_t *cl_freezeDemo;           /* 0x015F7074 */
extern cvar_t *cl_paused;               /* 0x0163A22C */
extern cvar_t *cl_showTimeDelta;        /* 0x015F7078 */
extern cvar_t *cl_timeNudge;            /* 0x01617308 */
extern cvar_t *sv_paused;               /* 0x0163B3C8 */
extern cvar_t *timedemo;                /* cl_timedemo 0x015F706C */

#define RESET_TIME              500

#define SNAPFLAG_NOT_ACTIVE     2

#define CA_LOADING_UNUSED       4
#define CA_PRIMED_STATE         5

/* ---- CL_AdjustTimeDelta  0x00404BF0 ----  VERIFIED */
void CL_AdjustTimeDelta( void ) {
	int newDelta;
	int deltaDelta;

	cl_newSnapshots = qfalse;

	if ( clc_demoplaying ) {
		return;
	}

	newDelta = cl_snap_serverTime - cls_realtime;
	deltaDelta = abs( newDelta - cl_serverTimeDelta );

	if ( deltaDelta > RESET_TIME ) {
		cl_serverTimeDelta = newDelta;
		cl_oldServerTime = cl_snap_serverTime;
		cl_serverTime = cl_snap_serverTime;
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<RESET> " );
		}
	} else if ( deltaDelta > 100 ) {
		if ( cl_showTimeDelta->integer ) {
			Com_Printf( "<FAST> " );
		}
		cl_serverTimeDelta = ( cl_serverTimeDelta + newDelta ) >> 1;
	} else {
		if ( com_timescale->value == 0 || com_timescale->value == 1 ) {
			if ( cl_extrapolatedSnapshot ) {
				cl_extrapolatedSnapshot = qfalse;
				cl_serverTimeDelta -= 2;
			} else {
				cl_serverTimeDelta++;
			}
		}
	}

	if ( cl_showTimeDelta->integer ) {
		Com_Printf( "%i ", cl_serverTimeDelta );
	}
}

/* ---- CL_FirstSnapshot  0x00404D00 ----  VERIFIED */
void CL_FirstSnapshot( void ) {
	if ( *(int *)cl_snap_snapFlags & SNAPFLAG_NOT_ACTIVE ) {
		return;
	}

	cls_state = CA_ACTIVE;

	cl_serverTimeDelta = cl_snap_serverTime - cls_realtime;
	cl_oldServerTime = cl_snap_serverTime;

	clc_timeDemoBaseTime = cl_snap_serverTime;

	if ( *activeAction->string ) {
		Cbuf_AddText( activeAction->string );
		Cvar_Set2( "activeAction", "", qtrue );
	}
}

/* ---- CL_SetCGameTime  0x00404D60 ----  VERIFIED */
void CL_SetCGameTime( void ) {
	int tn;

	if ( cls_state != CA_ACTIVE ) {
		if ( cls_state != CA_PRIMED_STATE ) {
			return;
		}
		if ( clc_demoplaying ) {
			if ( !clc_firstDemoFrameSkipped ) {
				clc_firstDemoFrameSkipped = qtrue;
				return;
			}
			CL_ReadDemoMessage();
		}
		if ( cl_newSnapshots ) {
			cl_newSnapshots = qfalse;
			CL_FirstSnapshot();
		}
		if ( cls_state != CA_ACTIVE ) {
			return;
		}
	}

	if ( !cl_snap_valid ) {
		Com_Error( ERR_DROP, "\x15" "CL_SetCGameTime: !cl.snap.valid" );
	}

	if ( sv_paused->integer && cl_paused->integer && com_sv_running->integer ) {
		return;
	}

	if ( cl_snap_serverTime < cl_oldFrameServerTime ) {
		if ( !Q_stricmpn( cls_servername, "localhost", 99999 ) ) {
			CL_FirstSnapshot();
		} else {
			Com_Error( ERR_DROP,
					   "\x15" "cl.snap.serverTime < cl.oldFrameServerTime" );
		}
	}
	cl_oldFrameServerTime = cl_snap_serverTime;

	if ( clc_demoplaying && cl_freezeDemo->integer ) {
	} else {
		tn = cl_timeNudge->integer;
		if ( tn < -30 ) {
			tn = -30;
		} else if ( tn > 30 ) {
			tn = 30;
		}

		cl_serverTime = cls_realtime + cl_serverTimeDelta - tn;

		if ( cl_serverTime < cl_oldServerTime ) {
			cl_serverTime = cl_oldServerTime;
		}
		cl_oldServerTime = cl_serverTime;

		if ( cls_realtime + cl_serverTimeDelta >= cl_snap_serverTime - 5 ) {
			cl_extrapolatedSnapshot = qtrue;
		}
	}

	if ( cl_newSnapshots ) {
		CL_AdjustTimeDelta();
	}

	if ( !clc_demoplaying ) {
		return;
	}

	if ( timedemo->integer ) {
		if ( !clc_timeDemoStart ) {
			clc_timeDemoStart = Sys_Milliseconds();
		}
		( *(int *)clc_timeDemoFrames )++;
		cl_serverTime = clc_timeDemoBaseTime + *(int *)clc_timeDemoFrames * 50;
	}

	while ( cl_serverTime >= cl_snap_serverTime ) {
		CL_ReadDemoMessage();
		if ( cls_state != CA_ACTIVE ) {
			return;
		}
	}
}

#define CG_PROFILE_DRAW_TRAP    15  /* 0x00404F74; cl_net_chan_mp.c spells it the same */

/* ---- CL_DrawString  0x00404F50 ----  VERIFIED
 *
 * Retail pushes ECX, EDX, [esp+14h], [esp+10h], [esp+0Ch], [esp+8], [esp+4], 0Fh.
 * Under /LTCG the register pair is chosen freely, so ECX/EDX are not params 1 and 2.
 *
 * vmMain export 15 (0x3002042B): arg0 -> x, arg1 -> y, arg2 -> string,
 * arg3 -> EAX, arg4 -> charWidth, arg5 -> charHeight, arg6 -> maxChars, with
 * arg0/1/4/5 converted by `fild`.  So the five stack slots are x, y, text, arg3,
 * charWidth and the two register slots are charHeight (EDX) and maxChars (ECX).
 * CL_ProfDraw (0x00414900) reaches the same trap with 32, y, text, 0, 8, 10, 0.
 *
 * arg3 is SHADOW, not forceColor: CG_DrawStringExt takes it in EAX and does
 * neg/sbb/and 3 (0x300196C7) to pick text style 3 or 0.  forceColor is the dead
 * constant 0 in the cgame's fourth stack slot.
 *
 * 0x00404F50 has no code or data reference anywhere in the image, so the source
 * order of the seven parameters is unrecoverable; they are written here in the
 * trap's order with __fastcall dropped, as cl_net_chan_mp.c does for the
 * same register pair.
 */
int CL_DrawString(int x, int y, const char *text, int shadow, int charWidth, int charHeight, int maxChars)
{
  int result;

  result = (int)cgvm;
  if ( cgvm )
    return VM_Call(cgvm, CG_PROFILE_DRAW_TRAP, x, y, text, shadow, charWidth, charHeight, maxChars);
  return result;
}

/* ---- CL_SaveCgameState  0x00404F80 ----  VERIFIED */
int CL_SaveCgameState( int size, void *buffer )
{
  int used;

  used = cgame_SaveExportTable( buffer, size );
  return used + VM_Call( cgvm, 17, (char *)buffer + used, size - used );
}

/* ---- CL_RestoreCgameState  0x00404FB0 ----  VERIFIED */
int CL_RestoreCgameState( int size, void *buffer )
{
  int used;

  used = cgame_RestoreExportTable( buffer, size );
  return used + VM_Call( cgvm, 18, (char *)buffer + used, size - used );
}

/* 0x015CA590, 160 bytes. Storage is client_mp/cl_refstorage.c:122. */

extern int cl_gameState_stringOffsets[];

/* ---- nullsub_8  0x00401030 ----  VERIFIED */
void nullsub_8( void ) {
}

/* ---- nullsub_9  0x00401040 ----  VERIFIED */
void nullsub_9( void ) {
}

/* ---- sub_401050  0x00401050 ----  VERIFIED */
float sub_401050( float f ) {
	return f;
}

/* ---- CL_GetGameState  0x00401060 ----  VERIFIED */
void CL_GetGameState( void *gs ) {
	/* Legacy cgame syscall: never copy the expanded structure into a stock DLL. */
	if ( cl_gameState_dataCount > STOCK_MAX_GAMESTATE_CHARS ) {
		Com_Error( ERR_DROP, "This server requires the updated cgame DLL" );
		return;
	}
	memcpy( gs, cl_gameState_stringOffsets, 0x5E80 );
	*(int *)( (byte *)gs + 0x5E80 ) = cl_gameState_dataCount;
}

/* ---- CL_GetGlconfig_cgame  0x00401080 ----  VERIFIED */
void CL_GetGlconfig_cgame( glconfig_t *config ) {
	*config = cls_glconfig;
}

/* ---- CL_ShutdownCGame  0x00401A20 ----  [CONFIRMED] */
void CL_ShutdownCGame( void ) {
	cls_keyCatchers &= ~KEYCATCH_CGAME;

	if ( !cgvm ) {
		return;
	}

	VM_Call( cgvm, CG_SHUTDOWN );
	VM_Free( cgvm );
	cgvm = NULL;

	if ( snd_aliasSetLoaded[SND_LOCALE_INGAME] ) {
		Com_UnloadSoundAliasSounds( SND_LOCALE_INGAME );

		if ( snd_aliasTable[SND_LOCALE_INGAME] ) {
			if ( !snd_aliasTable[SND_LOCALE_LOCALIZED] ) {
				free( (void *)snd_aliasTable[SND_LOCALE_INGAME] );
			}
			snd_aliasTable[SND_LOCALE_INGAME]      = NULL;
			snd_aliasTableCount[SND_LOCALE_INGAME] = 0;
			memset( snd_aliasHashTable[SND_LOCALE_INGAME], 0,
					SND_ALIAS_HASH_SLICE_BYTES );
		}

		snd_aliasSetLoaded[SND_LOCALE_INGAME] = 0;

		if ( !snd_aliasSetLoaded[SND_LOCALE_MENU] ) {
			Cmd_RemoveCommand( "snd_list" );
		}
	}
}
