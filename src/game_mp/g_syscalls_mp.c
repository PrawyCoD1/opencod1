/*
 * g_syscalls_mp.c -- every call the multiplayer game module makes into the
 * engine.  game_mp_x86.dll is a native DLL, so this is the DLL half of the
 * RTCW game/g_syscalls.c pattern: the engine hands dllEntry a single vararg
 * function pointer and each trap below is one line of body around it.
 *
 * Function order is binary order (0x20038740 .. 0x20039580).  Signatures come
 * from SV_GameSystemCalls in server_mp/sv_game_mp.c -- the argument each `case`
 * hands to the engine function is the argument the wrapper pushes.
 *
 * Wrappers tagged "inferred" are not in the Mac symbol table; the name is taken
 * from the gameImport_t constant they dispatch.  The Hunk family keeps the
 * `Internal` suffix its recovered siblings carry.
 *
 * @fidelity: likely
 */

#include "g_local.h"

static int ( QDECL * syscall )( int arg, ... ) = ( int ( QDECL * )( int, ... ) ) - 1;

void dllEntry( int ( QDECL *syscallptr )( int arg,... ) ) {
	syscall = syscallptr;
}

float IntAsFloat( int x ) {
	int intTemp;
	intTemp = x;
	return *(float *)&intTemp;
}

int PASSFLOAT( float x ) {
	float floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void    trap_Printf( const char *fmt ) {
	syscall( G_PRINT, fmt );
}

void    trap_Error( const char *fmt ) {
	syscall( G_ERROR, fmt );
}

void    trap_ErrorNoPrefix( const char *fmt ) {      /* inferred */
	syscall( G_ERROR_NOPREFIX, fmt );
}

int     trap_Milliseconds( void ) {
	return syscall( G_MILLISECONDS );
}

int     trap_Argc( void ) {
	return syscall( G_ARGC );
}

void    trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( G_ARGV, n, buffer, bufferLength );
}

void    *trap_Hunk_AllocInternal( int size ) {       /* inferred */
	return (void *)syscall( G_HUNK_ALLOC, size );
}

void    *trap_Hunk_AllocLowInternal( int size ) {
	return (void *)syscall( G_HUNK_ALLOC_LOW, size );
}

void    *trap_Hunk_AllocAlignInternal( int size, int align ) {      /* inferred */
	return (void *)syscall( G_HUNK_ALLOC_ALIGN, size, align );
}

void    *trap_Hunk_AllocLowAlignInternal( int size, int align ) {
	return (void *)syscall( G_HUNK_ALLOC_LOW_ALIGN, size, align );
}

void    *trap_Hunk_AllocateTempMemoryInternal( int size ) {         /* inferred */
	return (void *)syscall( G_HUNK_ALLOC_TEMP, size );
}

void    trap_Hunk_FreeTempMemoryInternal( void *buf ) {             /* inferred */
	syscall( G_HUNK_FREE_TEMP, buf );
}

int     trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( G_FS_FOPEN_FILE, qpath, f, mode );
}

void    trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall( G_FS_READ, buffer, len, f );
}

int     trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return syscall( G_FS_WRITE, buffer, len, f );
}

void    trap_FS_Rename( const char *from, const char *to ) {        /* inferred */
	syscall( G_FS_RENAME, from, to );
}

void    trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( G_FS_FCLOSE_FILE, f );
}

int     trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

void    trap_SendConsoleCommand( int exec_when, const char *text ) {
	syscall( G_SEND_CONSOLE_COMMAND, exec_when, text );
}

void    trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	syscall( G_CVAR_REGISTER, cvar, var_name, value, flags );
}

void    trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall( G_CVAR_UPDATE, cvar );
}

void    trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( G_CVAR_SET, var_name, value );
}

int     trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return syscall( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

/* The engine writes the result through the second argument; nothing comes back in eax. */
float   trap_Cvar_VariableValue( const char *var_name ) {
	float value;

	syscall( G_CVAR_VARIABLE_VALUE, var_name, &value );
	return value;
}

void    trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void    trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
							 playerState_t *clients, int sizeofGameClient ) {
	syscall( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGameClient );
}

void    trap_DropClient( int clientNum, const char *reason ) {
	syscall( G_DROP_CLIENT, clientNum, reason );
}

void    trap_SendServerCommand( int clientNum, int reliable, const char *command ) {
	syscall( G_SEND_SERVER_COMMAND, clientNum, reliable, command );
}

void    trap_SetConfigstring( int num, const char *string ) {
	syscall( G_SET_CONFIGSTRING, num, string );
}

void    trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

const char *trap_GetConfigstringConst( int num ) {
	return (const char *)syscall( G_GET_CONFIGSTRING_CONST, num );
}

void    trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_USERINFO, num, buffer, bufferSize );
}

void    trap_SetUserinfo( int num, const char *buffer ) {           /* inferred */
	syscall( G_SET_USERINFO, num, buffer );
}

void    trap_GetServerinfo( char *buffer, int bufferSize ) {
	syscall( G_GET_SERVERINFO, buffer, bufferSize );
}

void    trap_SetBrushModel( gentity_t *ent ) {
	syscall( G_SET_BRUSH_MODEL, ent );
}

void    trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
					const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void    trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
						   const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACE_CAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void    trap_SightTrace( int *hitOut, const vec3_t start, const vec3_t mins, const vec3_t maxs,      /* inferred */
						 const vec3_t end, int passEntityNum, int passOwnerNum, int contentmask ) {
	syscall( G_SIGHT_TRACE, hitOut, start, mins, maxs, end, passEntityNum, passOwnerNum, contentmask );
}

void    trap_SightTraceCapsule( int *hitOut, const vec3_t start, const vec3_t mins, const vec3_t maxs,   /* inferred */
								const vec3_t end, int passEntityNum, int passOwnerNum, int contentmask ) {
	syscall( G_SIGHT_TRACE_CAPSULE, hitOut, start, mins, maxs, end, passEntityNum, passOwnerNum, contentmask );
}

int     trap_SightTraceToEntity( const vec3_t start, const vec3_t mins, const vec3_t maxs,
								 const vec3_t end, int entityNum, int contentmask ) {
	return syscall( G_SIGHT_TRACE_TO_ENTITY, start, mins, maxs, end, entityNum, contentmask );
}

void    trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,        /* inferred */
						  const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask ) {
	syscall( G_CM_BOX_TRACE, results, start, end, mins, maxs, model, brushmask );
}

void    trap_CM_BoxTrace2( trace_t *results, const vec3_t start, const vec3_t end,       /* inferred */
						   const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask ) {
	syscall( G_CM_BOX_TRACE_2, results, start, end, mins, maxs, model, brushmask );
}

int     trap_CM_BoxSightTrace( const vec3_t start, const vec3_t end, const vec3_t mins,  /* inferred */
							   const vec3_t maxs, clipHandle_t model, int brushmask ) {
	return syscall( G_CM_BOX_SIGHT_TRACE, start, end, mins, maxs, model, brushmask );
}

int     trap_CM_BoxSightTrace2( const vec3_t start, const vec3_t end, const vec3_t mins, /* inferred */
								const vec3_t maxs, clipHandle_t model, int brushmask ) {
	return syscall( G_CM_BOX_SIGHT_TRACE_2, start, end, mins, maxs, model, brushmask );
}

void    trap_LocationalTrace( trace_t *results, const vec3_t start, const vec3_t end,
							  int passEntityNum, int contentmask, const byte *dobjTracePartState ) {
	syscall( G_LOCATIONAL_TRACE, results, start, end, passEntityNum, contentmask, dobjTracePartState );
}

int     trap_PointContents( const vec3_t point, int passEntityNum, int contentmask ) {
	return syscall( G_POINT_CONTENTS, point, passEntityNum, contentmask );
}

int     trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return syscall( G_IN_PVS, p1, p2 );
}

int     trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {        /* inferred */
	return syscall( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

int     trap_InSnapshot( const vec3_t origin, int entityNum ) {
	return syscall( G_IN_SNAPSHOT, origin, entityNum );
}

void    trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	syscall( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

int     trap_AreasConnected( int area1, int area2 ) {                       /* inferred */
	return syscall( G_AREAS_CONNECTED, area1, area2 );
}

void    trap_LinkEntity( gentity_t *ent ) {
	syscall( G_LINKENTITY, ent );
}

void    trap_UnlinkEntity( gentity_t *ent ) {
	syscall( G_UNLINKENTITY, ent );
}

int     trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList,
							int maxcount, int contentmask ) {
	return syscall( G_ENTITIES_IN_BOX, mins, maxs, entityList, maxcount, contentmask );
}

int     trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACT, mins, maxs, ent );
}

int     trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACT_CAPSULE, mins, maxs, ent );
}

void    trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	syscall( G_GET_USERCMD, clientNum, cmd );
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return syscall( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

void    trap_AddDebugString( const vec3_t origin, const vec3_t color, float scale, const char *text ) {
	syscall( G_ADD_DEBUG_STRING, origin, color, PASSFLOAT( scale ), text );
}

void    trap_AddDebugLine( const vec3_t start, const vec3_t end, const vec3_t color,
						   int depthTest, int duration ) {
	syscall( G_ADD_DEBUG_LINE, start, end, color, depthTest, duration );
}

/* Recovered name; the constant it dispatches is G_ENABLE_ARCHIVED_SNAPSHOT. */
void    trap_SetArchive( qboolean enable ) {
	syscall( G_ENABLE_ARCHIVED_SNAPSHOT, enable );
}

int     trap_RealTime( void *qtime ) {                                      /* inferred */
	return syscall( G_REAL_TIME, qtime );
}

void    trap_SnapVector( float *v ) {
	syscall( G_SNAPVECTOR, v );
}

int     trap_FindSoundAlias( const char *name ) {                           /* inferred */
	return syscall( G_FIND_SOUND_ALIAS, name );
}

int     trap_PickSoundAlias( const char *name ) {                           /* inferred */
	return syscall( G_PICK_SOUND_ALIAS, name );
}

int     trap_SoundAliasIndex( void *alias ) {                               /* inferred */
	return syscall( G_SOUND_ALIAS_INDEX, alias );
}

int     trap_SurfaceTypeFromName( const char *name ) {                      /* inferred */
	return syscall( G_SURFACE_TYPE_FROM_NAME, name );
}

const char *trap_SurfaceTypeToName( int surfaceType ) {
	return (const char *)syscall( G_SURFACE_TYPE_TO_NAME, surfaceType );
}

void    *trap_Z_MallocInternal( int size ) {                                /* inferred */
	return (void *)syscall( G_Z_MALLOC, size );
}

void    trap_Z_Free( void *ptr ) {                                          /* inferred */
	syscall( G_Z_FREE, ptr );
}

/*
 * Retail (0x20038D70) turns the client number into `level.gentities + clientNum`
 * before returning it.  level_locals_t belongs to g_local.h, so the wrapper
 * hands the number back and the caller resolves the entity; the trap itself is
 * unchanged.  The engine's SV_AddTestClient returns void, so this is always 0.
 */
int     trap_AddTestClient( void ) {
	return syscall( G_ADD_TEST_CLIENT );
}

qboolean trap_GetArchivedClientInfo( int clientNum, int *frameTime, void *clientInfo ) {
	return syscall( G_GET_ARCHIVED_CLIENT_INFO, clientNum, frameTime, clientInfo );
}

void    *trap_XAnimCreateTree( void *anims ) {                              /* inferred */
	return (void *)syscall( G_XANIM_CREATE_TREE, anims );
}

void    *trap_XAnimCreateSmallTree( void *anims ) {                         /* inferred */
	return (void *)syscall( G_XANIM_CREATE_SMALL_TREE, anims );
}

void    trap_XAnimFreeSmallTree( void *tree ) {                             /* inferred */
	syscall( G_XANIM_FREE_SMALL_TREE, tree );
}

int     trap_XModelExists( const char *name ) {                             /* inferred */
	return syscall( G_XMODEL_EXISTS, name );
}

void    *trap_XModelGet( const char *name ) {
	return (void *)syscall( G_XMODEL_GET, name );
}

void    trap_DObjCreate( void *models, unsigned short modelCount, void *tree,
						 int handle, unsigned short scrNotifyId ) {
	syscall( G_DOBJ_CREATE, models, modelCount, tree, handle, scrNotifyId );
}

void    trap_SafeDObjFree( int handle, unsigned int releaseTree ) {
	syscall( G_DOBJ_FREE, handle, releaseTree );
}

/*
 * The DObj traps take the entity and hand the engine its number.  gentity_t
 * starts with its entityState_t, so `ent->s.number` is the first dword of the
 * entity; g_local.h owns the full definition, hence the cast onto the shared
 * head declared in g_public.h.
 */
qboolean trap_DObjExists( gentity_t *ent ) {
	return syscall( G_DOBJ_EXISTS, ( (const entityState_t *)ent )->number );
}

int     trap_DerefDword( const int *p ) {                                   /* inferred */
	return syscall( G_DEREF_DWORD, p );
}

/*
 * inferred.  Reads the anims pointer out of an engine-owned tree and turns it
 * into a script anims handle, yielding a reference to that tree's root anim.
 */
scr_anim_t trap_XAnimGetRootAnim( const int *tree ) {
	scr_anim_t anim;

	anim.index = 0;
	anim.anims = (unsigned short)Scr_GetAnimsIndex( syscall( G_DEREF_DWORD, tree ) );
	return anim;
}

void    trap_XAnimClearTreeGoalWeights( void *tree, unsigned short animIndex, float goalWeight ) {  /* inferred */
	syscall( G_XANIM_CLEAR_ANIM, tree, animIndex, PASSFLOAT( goalWeight ) );
}

void    trap_XAnimClearGoalWeight( void *tree, unsigned short animIndex, float goalWeight ) {
	syscall( G_XANIM_CLEAR_ANIM_NODE, tree, animIndex, PASSFLOAT( goalWeight ) );
}

void    trap_XAnimClearTreeGoalWeightsStrict( void *tree, unsigned short animIndex, float goalWeight ) {
	syscall( G_XANIM_CLEAR_ANIM_CHILDREN, tree, animIndex, PASSFLOAT( goalWeight ) );
}

/*
 * The four set-weight traps below share one shape; past goalWeight only the
 * types are known, the argument names are not recovered.
 */
void    trap_XAnimSetAnimKnob( void *tree, unsigned short animIndex, float goalWeight,      /* inferred */
							   int goalTime, float rate, unsigned short notifyIndex, int notifyFlags ) {
	syscall( G_XANIM_SET_ANIM_KNOB, tree, animIndex, PASSFLOAT( goalWeight ), goalTime,
			 PASSFLOAT( rate ), notifyIndex, notifyFlags );
}

void    trap_XAnimSetCompleteGoalWeightKnobAll( void *tree, unsigned short animIndex,
												unsigned short childIndex, float goalWeight,
												float goalTime, float rate,
												unsigned short notifyIndex, int notifyFlags ) {
	syscall( G_XANIM_SET_ANIM_KNOB_ALL, tree, animIndex, childIndex, PASSFLOAT( goalWeight ),
			 PASSFLOAT( goalTime ), PASSFLOAT( rate ), notifyIndex, notifyFlags );
}

void    trap_XAnimSetAnimRate( void *tree, unsigned short animIndex, float rate ) {
	syscall( G_XANIM_SET_ANIM_RATE, tree, animIndex, PASSFLOAT( rate ) );
}

void    trap_XAnimSetTime( void *tree, unsigned short animIndex, float time ) {
	syscall( G_XANIM_SET_TIME, tree, animIndex, PASSFLOAT( time ) );
}

void    trap_XAnimSetAnimLimited( void *tree, unsigned short animIndex, float goalWeight,   /* inferred */
								  int goalTime, float rate, unsigned short notifyIndex, int notifyFlags ) {
	syscall( G_XANIM_SET_ANIM_LIMITED, tree, animIndex, PASSFLOAT( goalWeight ), goalTime,
			 PASSFLOAT( rate ), notifyIndex, notifyFlags );
}

void    trap_XAnimClearTree( void *tree ) {                                 /* inferred */
	syscall( G_XANIM_CLEAR_TREE, tree );
}

/*
 * inferred.  The engine enum labels 91/92 the other way round; the Mac symbol
 * for 92 is trap_XAnimIsPrimitive, so this one takes the remaining label.
 */
int     trap_XAnimIsLeaf( scr_anim_t anim ) {
	return syscall( G_XANIM_IS_PRIMITIVE, anim.anims, anim.index );
}

int     trap_XAnimIsPrimitive( scr_anim_t anim ) {
	return syscall( G_XANIM_IS_LEAF, anim.anims, anim.index );
}

int     trap_XAnimGetLength( void *tree, unsigned short animIndex ) {
	return syscall( G_XANIM_LENGTH_MSEC, tree, animIndex );
}

float   trap_XAnimGetLengthSeconds( scr_anim_t anim ) {
	return IntAsFloat( syscall( G_XANIM_LENGTH_SEC, anim.anims, anim.index ) );
}

void    trap_XAnimSetCompleteGoalWeight( void *tree, unsigned short animIndex, float goalWeight,
										 float goalTime, float rate, unsigned short notifyIndex,
										 int notifyFlags ) {
	syscall( G_XANIM_SET_ANIM_KNOB_INT, tree, animIndex, PASSFLOAT( goalWeight ),
			 PASSFLOAT( goalTime ), PASSFLOAT( rate ), notifyIndex, notifyFlags );
}

void    trap_XAnimSetGoalWeight( void *tree, unsigned short animIndex, float goalWeight,
								 float goalTime, float rate, unsigned short notifyIndex,
								 int notifyFlags ) {
	syscall( G_XANIM_SET_ANIM_INT_LIMITED, tree, animIndex, PASSFLOAT( goalWeight ),
			 PASSFLOAT( goalTime ), PASSFLOAT( rate ), notifyIndex, notifyFlags );
}

/* The third syscall pointer is the rotation quaternion and the fourth the
   movement delta (turret_think_init 0x20027AED / 0x20027F26). */
void    trap_XAnimCalcAbsDelta( void *tree, unsigned short animIndex, float *deltaRot, float *deltaMove ) {
	syscall( G_XANIM_GET_ABS_DELTA, tree, animIndex, deltaRot, deltaMove );
}

void    trap_XAnimCalcRelDelta( void *tree, unsigned short animIndex, float *deltaPos,       /* inferred */
								void *deltaQuat, int animIndexEnd ) {
	syscall( G_XANIM_GET_REL_DELTA, tree, animIndex, deltaPos, deltaQuat, animIndexEnd );
}

/* Rotation second, translation third, as GScr_GetMoveDelta pushes them
   (0x200333F5). */
void    trap_XAnimGetRelDelta( scr_anim_t anim, float *deltaRot, float *deltaMove,
							   float startTime, float endTime ) {
	syscall( G_XANIM_GET_REL_DELTA_TIME, anim.anims, anim.index, deltaRot, deltaMove,
			 PASSFLOAT( startTime ), PASSFLOAT( endTime ) );
}

void    trap_XAnimGetAbsDelta( scr_anim_t anim, float *deltaPos, void *deltaQuat, float time ) {  /* inferred */
	syscall( G_XANIM_GET_ABS_DELTA_TIME, anim.anims, anim.index, deltaPos, deltaQuat,
			 PASSFLOAT( time ) );
}

int     trap_XAnimIsLooped( scr_anim_t anim ) {
	return syscall( G_XANIM_IS_LOOPED, anim.anims, anim.index );
}

int     trap_XAnimNotetrackExists( scr_anim_t anim, unsigned short notetrack ) {
	return syscall( G_XANIM_HAS_NOTETRACK, anim.anims, anim.index, notetrack );
}

float   trap_XAnimGetTime( void *tree, unsigned short animIndex ) {
	return IntAsFloat( syscall( G_XANIM_GET_TIME, tree, animIndex ) );
}

float   trap_XAnimGetWeight( void *tree, unsigned short animIndex ) {
	return IntAsFloat( syscall( G_XANIM_GET_WEIGHT, tree, animIndex ) );
}

void    trap_DObjDumpInfo( gentity_t *ent ) {
	syscall( G_DOBJ_DUMP_INFO, ( (const entityState_t *)ent )->number );
}

qboolean trap_DObjCreateSkelForBone( gentity_t *ent, int boneIndex ) {
	return syscall( G_DOBJ_CREATE_SKEL_FOR_BONE, ( (const entityState_t *)ent )->number, boneIndex );
}

qboolean trap_DObjCreateSkelForBones( gentity_t *ent, const int *partBits ) {
	return syscall( G_DOBJ_SKEL_UP_TO_DATE, ( (const entityState_t *)ent )->number, partBits );
}

int     trap_DObjUpdateServerTime( gentity_t *ent, float serverTime, int stopOnNotetrack ) {
	return syscall( G_DOBJ_UPDATE_SERVER_TIME, ( (const entityState_t *)ent )->number,
					PASSFLOAT( serverTime ), stopOnNotetrack );
}

void    trap_DObjInitServerTime( gentity_t *ent, float serverTime ) {
	syscall( G_DOBJ_INIT_SERVER_TIME, ( (const entityState_t *)ent )->number,
			 PASSFLOAT( serverTime ) );
}

void    trap_DObjGetHierarchyBits( gentity_t *ent, int boneIndex, int *partBits ) {
	syscall( G_DOBJ_GET_HIERARCHY_BITS, ( (const entityState_t *)ent )->number, boneIndex, partBits );
}

int     trap_DObjCalcAnim( gentity_t *ent, void *partBits ) {
	return syscall( G_DOBJ_CALC_ANIM, ( (const entityState_t *)ent )->number, partBits );
}

int     trap_DObjCalcSkel( gentity_t *ent, void *partBits ) {
	return syscall( G_DOBJ_CALC_SKEL, ( (const entityState_t *)ent )->number, partBits );
}

void    trap_XAnimRestoreTree( int tree ) {                                 /* inferred */
	syscall( G_XANIM_RESTORE_TREE, tree );
}

void    trap_XAnimSaveTree( void *tree ) {                                  /* inferred */
	syscall( G_XANIM_SAVE_TREE, tree );
}

void    trap_XAnimCopyTree( const void *from, void *to ) {                  /* inferred */
	syscall( G_XANIM_COPY_TREE, from, to );
}

int     trap_DObjNumParts( gentity_t *ent ) {                               /* inferred */
	return syscall( G_DOBJ_NUM_PARTS, ( (const entityState_t *)ent )->number );
}

int     trap_DObjGetBoneIndex( gentity_t *ent, const char *boneName ) {
	return syscall( G_DOBJ_GET_PART_INDEX, ( (const entityState_t *)ent )->number, boneName );
}

void    *trap_DObjGetMatrixArray( gentity_t *ent ) {
	return (void *)syscall( G_DOBJ_GET_SKEL_BASE, ( (const entityState_t *)ent )->number );
}

void    trap_DObjDisplayAnim( gentity_t *ent ) {
	syscall( G_DOBJ_DISPLAY_ANIM, ( (const entityState_t *)ent )->number );
}

int     trap_XAnimHasTime( void *tree, unsigned short animIndex ) {         /* inferred */
	return syscall( G_XANIM_HAS_TIME, tree, animIndex );
}

int     trap_XAnimGetNumChildren( scr_anim_t anim ) {
	return syscall( G_XANIM_CHILD_COUNT, anim.anims, anim.index );
}

/* Only the index half is replaced; the anims handle rides through untouched. */
scr_anim_t trap_XAnimGetChildAt( scr_anim_t anim, int childIndex ) {
	anim.index = (unsigned short)syscall( G_XANIM_CHILD_BASE, anim.anims, anim.index, childIndex );
	return anim;
}

int     trap_XModelNumBones( void *model ) {
	return syscall( G_XANIM_ROOT_PART_COUNT, model );
}

unsigned short *trap_XModelGetBoneNames( void *model ) {
	return (unsigned short *)syscall( G_XANIM_ROOT_PART_LIST, model );
}

void    *trap_DObjGetRotTransArray( gentity_t *ent ) {
	return (void *)syscall( G_DOBJ_GET_SKEL_END, ( (const entityState_t *)ent )->number );
}

qboolean trap_DObjSetRotTransIndex( gentity_t *ent, const int *partBits, int boneIndex ) {
	return syscall( G_DOBJ_SET_ROT_TRANS_INDEX, ( (const entityState_t *)ent )->number,
					partBits, boneIndex );
}

qboolean trap_DObjSetControlRotTransIndex( gentity_t *ent, const int *partBits, int boneIndex ) {
	return syscall( G_DOBJ_SET_CONTROL_ROT_TRANS_INDEX, ( (const entityState_t *)ent )->number,
					partBits, boneIndex );
}

const char *trap_XAnimGetAnimName( scr_anim_t anim ) {
	return (const char *)syscall( G_XANIM_GET_ANIM_NAME, anim.anims, anim.index );
}

void    *trap_DObjGetTree( gentity_t *ent ) {                               /* inferred */
	return (void *)syscall( G_DOBJ_GET_TREE, ( (const entityState_t *)ent )->number );
}

int     trap_XAnimGetAnimTreeSize( void *tree ) {
	return syscall( G_XANIM_TREE_FIELD, tree );
}

void    trap_XModelDebugBoxes( gentity_t *ent ) {
	syscall( G_DOBJ_DEBUG_DRAW_BOUNDS, ( (const entityState_t *)ent )->number );
}

void    *trap_GetWeaponInfoMemory( int size, int *pPrevOwner ) {
	return (void *)syscall( G_GET_WEAPON_INFO_MEMORY, size, pPrevOwner );
}

void    trap_FreeWeaponInfoMemory( int iSource ) {
	syscall( G_FREE_WEAPON_INFO_MEMORY, iSource );
}

void    trap_FreeClientScriptPers( void ) {
	syscall( G_FREE_CLIENT_SCRIPT_PERS );
}

void    trap_ResetEntityParsePoint( void ) {                                /* inferred */
	syscall( G_RESET_ENTITY_PARSE_POINT );
}
