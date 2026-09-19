/*
 * @fidelity: verified
 */

#include "server.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef struct DObj_s
{
	void            *tree;                  /* +0x00 XAnimTree * */
	byte            *evalStorage;           /* +0x04 */
	int skelCacheKey;                       /* +0x08 vs com_skelTimeStamp */
	byte unk_0C[4];                         /* +0x0C */
	unsigned short unknownState10;          /* +0x10 */
	byte unk_12[2];                         /* +0x12 */
	unsigned short tracePartRemapHandle;    /* +0x14 */
	byte childCount;                        /* +0x16 */
	byte partCount;                         /* +0x17 */
	unsigned int childRefs[8];              /* +0x18 */
	short childModelIndices[8];             /* +0x38 */
	byte childParentPartIndices[8];         /* +0x48 */
	byte childPartBaseIndices[8];           /* +0x50 */
} DObj_t;
SV_ASSERT_SIZE( DObj_t, 88 );

#define SVENT_NUMCLUSTERS( e )      ( *(int *)( (byte *)( e ) + 0x118 ) )
#define SVENT_CLUSTERNUMS( e )      (  (int *)( (byte *)( e ) + 0x11C ) )
#define SVENT_LASTCLUSTER( e )      ( *(int *)( (byte *)( e ) + 0x15C ) )
#define SVENT_AREANUM( e )          ( *(int *)( (byte *)( e ) + 0x160 ) )
#define SVENT_AREANUM2( e )         ( *(int *)( (byte *)( e ) + 0x164 ) )

/* entityShared_t +0x0C, RTCW/Q3/UO's `bmodel` (server.h spells it unknown_0x0C[4]). SV_SetBrushModel sets it and SV_LinkEntity reads it. */
#define GENT_BMODEL( g )            ( *(int *)( ( g )->r.unknown_0x0C ) )

#define SVF_NOCLIENT                0x00000001
#define SVF_VISIBILITY_BYPASS       0x00000018

extern int hunk_totalSize;
extern int hunk_highUsed;
/* XAnim server-time staging, retail 0x00A7A618 / 0x00A9CC5C. Plain values, one dword each, so no sizing hazard. */
extern int xanim_currentTree;
extern int xanim_currentTime;
#define HUNK_TOTAL          ( *(int *) hunk_totalSize )
#define HUNK_HIGH_PERM      ( *(int *) hunk_highUsed )
#define HUNK_LOW_MARK       ( *(int *) hunk_lowMark )
#define HUNK_LOW_PERM       ( *(int *) hunk_temp_permanent )
#define HUNK_LOW_TEMP       ( *(int *) hunk_temp_temp )

#include "../universal/com_sndalias.h"

extern int scrAnimPub_trees;
#define SV_XANIM_TREE( idx ) \
	( (int)( &scrAnimPub_trees )[ 128 * xanim_activePoolSlot + ( idx ) ] )

#define SV_XANIM_POOL_SERVER    1

extern int CM_PointLeafnum( const vec3_t p );
extern int CM_LeafCluster( int leafnum );
extern int CM_LeafArea( int leafnum );
extern byte *CM_ClusterPVS( int cluster );
extern qboolean CM_AreasConnected( int area1, int area2 );
extern void CM_ChangeAreaPortalState( int area1, int area2, qboolean open );
extern int CM_NumInlineModels( void );
extern const char *CM_EntityString( void );
extern float *CM_ClipHandleToModel( clipHandle_t handle );
extern int CM_AreaEntities( const vec3_t mins, const vec3_t maxs,
							int *entityList, int maxcount, int contentmask );
extern void CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						 const vec3_t mins, const vec3_t maxs,
						 clipHandle_t model, int brushmask, qboolean capsule );
extern int CM_BoxSightTrace( int oldHitNum, const vec3_t start,
							 const vec3_t end, const vec3_t mins,
							 const vec3_t maxs, clipHandle_t model,
							 int brushmask, qboolean capsule );
extern void CM_TransformedBoxTrace();

extern int Com_RealTime( void *qtime );
extern void Sys_SnapVector( float *v );
extern void Sys_LoadingKeepAlive( void );
extern void MatrixMultiply( float in1[3][3], float in2[3][3], float out[3][3] );
extern void PerpendicularVector( vec3_t dst, const vec3_t src );
extern void *Hunk_AllocLowAlignInternal( int size, int align );
extern int FS_Rename( const char *from, const char *to );
extern void *Scr_NearHook( void *callbacks );
extern void XModelClearData( void *high, void *low );
/* Retail takes (lowEnd, highEnd) and returns nothing; body at 0x0042CA60 in universal/com_files.c. */
extern void FS_ClearDataForFiles( void *lowEnd, void *highEnd );

extern int *Com_FindSoundAlias( const char *name, int source );
extern int Com_PickSoundAlias( const char *name, int source );
extern int Com_SoundAliasIndex( void *alias, int source );
extern void Com_ServerDObjCreate( void *models, unsigned short modelCount,
								  void *tree, int handle,
								  unsigned short scrNotifyId );
extern void Com_SafeServerDObjFree( int handle, unsigned int releaseTree );
extern void *Com_XAnimCreateTree( void *anims );
extern void *Com_XAnimCreateSmallTree( void *anims );
extern void Com_XAnimFreeSmallTree( void *tree );
extern void *Com_GetWeaponInfoMemory( int size, int *pPrevOwner, int owner );
extern void Com_FreeWeaponInfoMemory( int owner, int iSource );
extern int XModelExists( const char *name );
extern void *XModelPrecache( const char *name, int loadSurfaces,
							 void *(*alloc)( unsigned int ), int allocMesh );
extern void *Hunk_AllocXModelPrecache( unsigned int size );

extern void DObjDumpInfo( void *dobj );                     /* 0x00481BB0 */
extern int DObjCalcAnim( int dobj, void *partBits );        /* 0x00487F00 */
extern int DObjCalcSkel( void *partBits, int dobj );        /* 0x00481780 */
extern int DObjFindPartIndex( int dobj, int nameHandle );   /* 0x00481230 */
extern int SL_FindLowercaseString( const char *s );         /* 0x0046FA50 */
extern int DObjGetHierarchyBits( int boneIndex, int dobj, void *partBits );  /* 0x00481440 */
extern int DObjUpdateServerInfo( void *dobj, float serverTime, int stopOnNotetrack );
extern int XAnimInitServerTime_m( int unused );
extern int XAnimDisplay( void *tree, int a2, int a3 );
extern int DObjBuildPartCollisionTable( void *out, int dobj );

extern int XAnimClearAnimNode_m( void *tree, int node, float blend );
extern int XAnimClearAnim_m( int node, void *tree, float blend );
extern int XAnimClearAnimChildSubtrees_m( void *tree, int node, float blend );
extern int XAnimGetRelDelta_m( int a1, int a2, void *a3, float *a4, void *a5 );
extern int XAnimGetAbsDelta_m( int a1, void *a2, void *a3, float *a4 );
extern int XAnimGetRelDeltaForTime_m( int a1, int a2, void *a3, float *a4, float a5, float a6 );
extern int XAnimGetAbsDeltaForTime_m( int a1, int a2, float *a3, void *a4, float a5 );
extern int XAnimSetAnimKnob_m( void *a1, int a2, float a3, int a4, float a5,
					   short a6, int a7, int a8 );
extern int XAnimSetAnimKnobAll_m( int a1, void *a2, int a3, float a4, int a5, float a6,
					   short a7, int a8, int a9 );
extern int XAnimSetAnimLimited_m( int a1, void *a2, float a3, int a4, float a5,
					   short a6, int a7, int a8 );
extern int XAnimClearTree( void *tree );
extern int XAnimSetTime_m( void *a1, int a2, int a3 );
extern int XAnimSetAnimInternalLimited_m( int a1, short a2, int a3, int a4, float a5, int a6,
					   float a7, int a8 );
extern int XAnimSetAnimKnobInternal_m( short a1, void *a2, int a3, float a4, int a5, float a6,
					   int a7, int a8 );
extern int XAnimSetAnimRate_m( int a1, int a2, float a3 );
extern int XAnimIsLooped_m( int a1, int a2 );
extern double      XAnimGetTime( int nodeIndex, void *tree );      /* 0x00487CB0, FLOAT */
extern double      XAnimGetWeight( int nodeIndex, void *tree );    /* 0x00487CE0, FLOAT */
extern int         XAnimHasTime( int nodeIndex, void *tree );      /* 0x00487D10 */
extern const char *XAnimGetAnimName( int nodeIndex, void *tree );  /* 0x00487D80 */
extern int cand_XAnimRestoreTree( int tree );
extern int cand_XAnimSaveTree( void *tree );
extern void XAnimCopyTree( const void *from, void *to );

/* Other server units. Retail 0x004556E0 is `void __cdecl SV_AddTestClient()` -- it returns nothing; declaring it `int` hands the game module a junk client number. */
extern void SV_AddTestClient( void );
extern void SV_FreeClientScriptPers( void );
extern qboolean SV_GetArchivedClientInfo( int *frameTime, void *clientInfo,
										  int clientNum );
extern clipHandle_t SV_ClipHandleForEntity( gentity_t *gent );
extern void SV_LinkEntity( gentity_t *gent );
extern void SV_UnlinkEntity( gentity_t *gent );
extern int SV_PointContents( const vec3_t p, int passEntityNum, int contentmask );

extern void SV_Trace( const vec3_t end, trace_t *results, const vec3_t start,
					  const vec3_t mins, const vec3_t maxs, int passEntityNum,
					  int contentmask, qboolean capsule, qboolean useDObj,
					  const byte *dobjTracePartState, qboolean locational );
extern void SV_SightTrace( const vec3_t end, const vec3_t start,
						   const vec3_t mins, int *hitOut, const vec3_t maxs,
						   int passEntityNum, int passOwnerNum,
						   int contentmask, qboolean capsule );
extern int SV_SightTraceToEntity( const vec3_t start, const vec3_t end,
								  const vec3_t mins, const vec3_t maxs,
								  int entityNum, int contentmask,
								  qboolean capsule );

/* ---- SV_NumForGentity  0x00455860 ---- */
int SV_NumForGentity( gentity_t *gent ) {
	return (int)( ( (byte *)gent - (byte *)sv.gentities ) / sv.gentitySize );
}

/* ---- SV_GentityNum  0x00455870 ---- */
gentity_t *SV_GentityNum( int num ) {
	return (gentity_t *)( (byte *)sv.gentities + sv.gentitySize * num );
}

/* ---- SV_GameClientNum  0x00455890 ---- */
playerState_t *SV_GameClientNum( int clientNum ) {
	return (playerState_t *)( (byte *)sv.gameClients
							  + sv.gameClientSize * clientNum );
}

/* ---- SV_SvEntityForGentity  0x004558B0 ---- */
svEntity_t *SV_SvEntityForGentity( gentity_t *gent ) {
	if ( !gent || gent->s.number < 0 || gent->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "\x15SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[gent->s.number];
}

/* ---- SV_GEntityForSvEntity  0x004558E0 ---- */
gentity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	return SV_GentityNum( (int)( svEnt - sv.svEntities ) );
}

/* ---- SV_GameSendServerCommand  0x00455910 ---- */
void SV_GameSendServerCommand( int clientNum, int reliable, const char *command ) {
	if ( sv_debugSpawn && sv_debugSpawn->integer ) {
		Com_Printf( "GCMD: cl %i rel %i [%s]\n", clientNum, reliable,
					command ? command : "(null)" );
	}

	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, reliable, "%s", command );
		return;
	}

	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}

	SV_SendServerCommand( &svs.clients[clientNum], reliable, "%s", command );
}

/* ---- SV_GameDropClient  0x00455960 ---- */
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( &svs.clients[clientNum], reason );
}

/* ---- SV_SetBrushModel  0x00455990 ---- */
void SV_SetBrushModel( gentity_t *gent ) {
	const float *bounds;
	int index;

	index = gent->s.index;
	if ( index < 0 || index >= CM_NumInlineModels() ) {
		Com_Error( ERR_DROP, "\x15" "CM_InlineModel: bad number" );
	}

	bounds = CM_ClipHandleToModel( index );

	gent->r.mins[0] = bounds[0];
	gent->r.mins[1] = bounds[1];
	gent->r.mins[2] = bounds[2];
	gent->r.maxs[0] = bounds[3];
	gent->r.maxs[1] = bounds[4];
	gent->r.maxs[2] = bounds[5];

	GENT_BMODEL( gent ) = 1;
	gent->r.contents = -1;

	SV_LinkEntity( gent );
}

/* ---- SV_inPVS  0x00455A40 ---- */
qboolean SV_inPVS( const vec3_t p1, const vec3_t p2 ) {
	int leafnum;
	int cluster;
	int area1;
	int area2;
	byte    *mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	area1 = CM_LeafArea( leafnum );
	mask = CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
	area2 = CM_LeafArea( leafnum );

	if ( mask && !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) {
		return qfalse;
	}
	if ( !CM_AreasConnected( area1, area2 ) ) {
		return qfalse;
	}
	return qtrue;
}

/* ---- SV_inSnapshot  0x00455AF0 ---- */
qboolean SV_inSnapshot( const vec3_t origin, int entityNum ) {
	gentity_t   *gent;
	svEntity_t     *svEnt;
	int svFlags;
	int leafnum;
	int cluster;
	int area;
	int numClusters;
	int lastCluster;
	int i;
	int testCluster;
	byte        *mask;

	gent = SV_GentityNum( entityNum );

	if ( !gent->r.linked ) {
		return qfalse;
	}

	svFlags = gent->r.svFlags;
	if ( svFlags & SVF_NOCLIENT ) {
		return qfalse;
	}
	if ( svFlags & SVF_VISIBILITY_BYPASS ) {
		return qtrue;
	}

	svEnt = SV_SvEntityForGentity( gent );

	numClusters = SVENT_NUMCLUSTERS( svEnt );
	if ( !numClusters ) {
		return qtrue;
	}

	leafnum = CM_PointLeafnum( origin );
	area = CM_LeafArea( leafnum );
	cluster = CM_LeafCluster( leafnum );
	mask = CM_ClusterPVS( cluster );

	if ( !CM_AreasConnected( area, SVENT_AREANUM( svEnt ) )
		 && !CM_AreasConnected( area, SVENT_AREANUM2( svEnt ) ) ) {
		return qfalse;
	}

	testCluster = 0;
	for ( i = 0 ; i < numClusters ; i++ ) {
		testCluster = SVENT_CLUSTERNUMS( svEnt )[i];
		if ( mask[testCluster >> 3] & ( 1 << ( testCluster & 7 ) ) ) {
			break;
		}
	}

	if ( i != numClusters ) {
		return qtrue;
	}

	lastCluster = SVENT_LASTCLUSTER( svEnt );
	if ( !lastCluster ) {
		return qfalse;
	}
	if ( testCluster > lastCluster ) {
		return qtrue;
	}

	while ( testCluster <= lastCluster ) {
		if ( mask[testCluster >> 3] & ( 1 << ( testCluster & 7 ) ) ) {
			break;
		}
		testCluster++;
	}

	return ( testCluster != lastCluster );
}

/* ---- SV_inPVSIgnorePortals  0x00455C20 ---- */
qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	int leafnum;
	int cluster;
	byte    *mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	mask = CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );

	if ( mask && !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) {
		return qfalse;
	}
	return qtrue;
}

/* ---- SV_AdjustAreaPortalState  0x00455CB0 ---- */
void SV_AdjustAreaPortalState( gentity_t *gent, qboolean open ) {
	svEntity_t *svEnt;

	svEnt = SV_SvEntityForGentity( gent );

	if ( SVENT_AREANUM2( svEnt ) == -1 ) {
		return;
	}

	CM_ChangeAreaPortalState( SVENT_AREANUM2( svEnt ),
							  SVENT_AREANUM( svEnt ), open );
}

/* ---- SV_EntityContact  0x00455D10 ---- */
qboolean SV_EntityContact( gentity_t *gent, const vec3_t mins, const vec3_t maxs, int capsule ) {
	clipHandle_t ch;
	trace_t trace;

	ch = SV_ClipHandleForEntity( gent );

	trace.fraction = 1.0f;
	CM_TransformedBoxTrace( &trace, vec3_origin, vec3_origin, mins, maxs,
							ch, -1, gent->r.currentOrigin,
							gent->r.currentAngles, capsule );

	return trace.startsolid;
}

/* ---- SV_GetServerinfo  0x00455D70 ---- */
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "\x15SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/* ---- SV_Hunk_AllocAlignInternal  0x00455E10 ---- */
void *SV_Hunk_AllocAlignInternal( int size, int align ) {
	if ( sv.state != SS_LOADING ) {
		Com_Error( ERR_DROP, "\x15trap_Hunk_AllocAlign can only be called in "
							 "G_InitGame and the first few frames of G_RunFrame\n" );
	}
	return Hunk_AllocAlignInternal( size, align );
}

/* ---- SV_Hunk_AllocLowAlignInternal  0x00455E40 ---- */
void *SV_Hunk_AllocLowAlignInternal( int size, int align ) {
	if ( sv.state != SS_LOADING ) {
		Com_Error( ERR_DROP, "\x15trap_Hunk_AllocLowAlign can only be called in "
							 "G_InitGame and the first few frames of G_RunFrame\n" );
	}
	return Hunk_AllocLowAlignInternal( size, align );
}

/* ---- SV_Hunk_AllocateTempMemoryInternal  0x00455E70 ---- */
void *SV_Hunk_AllocateTempMemoryInternal( int size ) {
	return Hunk_AllocateTempMemoryInternal( size );
}

/* ---- SV_Hunk_FreeTempMemoryInternal  0x00455E80 ---- */
void SV_Hunk_FreeTempMemoryInternal( void *buf ) {
	int     *header;

	if ( !s_hunkData ) {
		free( buf );
		return;
	}

	header = (int *)buf - 4;
	if ( header[0] != 0x89537892 ) {
		Com_Error( ERR_FATAL, "\x15Hunk_FreeTempMemory: bad magic" );
	}
	header[0] = 0x89537893;
	HUNK_LOW_TEMP -= header[1];
}

/* ---- SV_LocateGameData  0x00455EC0 ---- */
void SV_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity,
						playerState_t *clients, int sizeofGameClient ) {
	if ( sizeofGameClient != sizeof( gclient_t ) || sizeofGEntity != sizeof( gentity_t ) ) {
		Com_Error( ERR_DROP, "This server requires the updated game DLL" );
	}
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity;
	sv.num_entities = numGEntities;
	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;

	if ( sv_debugSpawn && sv_debugSpawn->integer ) {
		Com_Printf( "SPAWNDBG: LocateGameData gEnts %p n %i stride %i | "
					"gClients %p stride %i | engine sizeof(gentity_t) %i "
					"sizeof(playerState_t) %i | delta %i\n",
					(void *)gEnts, numGEntities, sizeofGEntity,
					(void *)clients, sizeofGameClient,
					(int)sizeof( gentity_t ), (int)sizeof( playerState_t ),
					(int)( (byte *)clients - (byte *)gEnts ) );
	}
}

/* ---- SV_GetUsercmd  0x00455EF0 ---- */
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "\x15SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

/* ---- FloatAsInt_1  0x00455F40 ---- */
int FloatAsInt_1( int bits ) {
	return bits;
}

/* ---- SV_XModelGet  0x00455F50 ---- */
void *SV_XModelGet( const char *name ) {
	int c;

	c = name[6];
	if ( _strnicmp( name, "xmodel", 6 ) || ( c != '/' && c != '\\' ) ) {
		Com_Error( ERR_DROP, "\x15" "bad model name '%s'", name );
	}

	return (void *)XModelPrecache( name + 7, 0, Hunk_AllocXModelPrecache, 0 );
}

/* ---- SV_DObjDumpInfo  0x00455FA0 ---- */
void SV_DObjDumpInfo( int gentNum ) {
	DObj_t  *obj;

	if ( !com_developer->integer ) {
		return;
	}

	obj = (DObj_t *)Com_GetServerDObj( gentNum );
	if ( obj ) {
		DObjDumpInfo( obj );
	} else {
		Com_Printf( "no model.\n" );
	}
}

/* ---- SV_DObjCreateSkelForBone  0x00455FF0 ---- */
qboolean SV_DObjCreateSkelForBone( int entityNum, int boneIndex ) {
	DObj_t  *obj;
	byte    *storage;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj ) {
		return qfalse;
	}

	if ( obj->skelCacheKey == com_skelTimeStamp ) {
		storage = obj->evalStorage;
		if ( storage ) {
			return ( storage[32 + ( boneIndex >> 3 )]
					 & ( 1 << ( boneIndex & 7 ) ) ) != 0;
		}
	} else {
		obj->skelCacheKey = com_skelTimeStamp;
		obj->unknownState10 = 0;
		obj->evalStorage = NULL;
	}

	storage = (byte *)Hunk_AllocateTempMemoryInternal( 96 * obj->partCount + 48 );
	obj->evalStorage = storage;
	Com_Memset( storage, 0, 48 );

	return qfalse;
}

/* ---- SV_DObjIsSkelUpToDate  0x00456090 ---- */
static qboolean SV_DObjIsSkelUpToDate( int entityNum, const int *partBits ) {
	DObj_t  *obj;
	byte    *storage;
	int     *upToDate;
	int i;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj ) {
		return qfalse;
	}

	if ( obj->skelCacheKey == com_skelTimeStamp && obj->evalStorage ) {
		upToDate = (int *)( obj->evalStorage + 32 );
		for ( i = 0 ; i < 4 ; i++ ) {
			if ( partBits[i] & ~upToDate[i] ) {
				return qfalse;
			}
		}
		return qtrue;
	}

	obj->skelCacheKey = com_skelTimeStamp;
	obj->unknownState10 = 0;
	storage = (byte *)Hunk_AllocateTempMemoryInternal( 96 * obj->partCount + 48 );
	obj->evalStorage = storage;
	Com_Memset( storage, 0, 48 );

	return qfalse;
}

/* ---- SV_DObjUpdateServerTime  0x00456120 ---- */
int SV_DObjUpdateServerTime( int entityNum, int stopOnNotetrack, float serverTime ) {
	DObj_t  *obj;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj ) {
		return 0;
	}
	return DObjUpdateServerInfo( obj, serverTime, stopOnNotetrack );
}

/* ---- SV_DObjInitServerTime  0x00456150 ---- */
void SV_DObjInitServerTime( int entityNum, int serverTime ) {
	DObj_t  *obj;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj || !obj->tree ) {
		return;
	}

	xanim_currentTime = serverTime;
	xanim_currentTree = (int)obj->tree;
	XAnimInitServerTime_m( 0 );
}

/* ---- SV_DObjGetHierarchyBits_m  0x00456190 ---- */
int SV_DObjGetHierarchyBits_m( int entityNum, int boneIndex, int *partBits ) {
	DObj_t  *obj;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	return DObjGetHierarchyBits( boneIndex, (int)obj, partBits );
}

/* ---- SV_DObjCalcAnim  0x004561D0 ---- */
int SV_DObjCalcAnim( int entityNum, void *partBits ) {
	DObj_t  *obj;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	return DObjCalcAnim( (int)obj, partBits );
}

/* ---- SV_DObjCalcSkel  0x00456200 ---- */
void SV_DObjCalcSkel( int entityNum, void *partBits ) {
	DObj_t  *obj;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	DObjCalcSkel( partBits, (int)obj );
}

/* ---- SV_DObjNumParts  0x00456240 ---- */
static int SV_DObjNumParts( int entityNum ) {
	DObj_t  *obj;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	return obj ? obj->partCount : 0;
}

/* ---- SV_DObjGetPartIndex  0x00456270 ---- */
static int SV_DObjGetPartIndex( int entityNum, const char *partName ) {
	DObj_t  *obj;
	int handle;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj ) {
		return -1;
	}

	handle = SL_FindLowercaseString( partName );
	if ( !handle ) {
		return -1;
	}
	return DObjFindPartIndex( (int)obj, handle );
}

/* ---- SV_DObjGetSkelBase  no-address ---- */
static void *SV_DObjGetSkelBase( int entityNum ) {
	DObj_t  *obj = (DObj_t *)Com_GetServerDObj( entityNum );

	if ( !obj || !obj->evalStorage ) {
		return NULL;
	}
	return obj->evalStorage + 64 * obj->childPartBaseIndices[0] + 48;
}

/* ---- SV_DObjGetSkelEnd  no-address ---- */
static void *SV_DObjGetSkelEnd( int entityNum ) {
	DObj_t  *obj = (DObj_t *)Com_GetServerDObj( entityNum );

	if ( !obj || !obj->evalStorage ) {
		return NULL;
	}
	return obj->evalStorage + 64 * obj->partCount + 48;
}

/* ---- SV_DObjDisplayAnim  0x00456300 ---- */
void SV_DObjDisplayAnim( int entityNum ) {
	DObj_t  *obj;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj ) {
		return;
	}

	if ( obj->tree ) {
		XAnimDisplay( obj->tree, 0, 0 );
		Com_Printf( "\n" );
	} else {
		Com_Printf( "NO TREE\n" );
	}
}

/* ---- SV_DObjSetRotTransIndex  0x00456390 ---- */
qboolean SV_DObjSetRotTransIndex( int entityNum, int boneIndex, const int *partBits ) {
	DObj_t  *obj;
	byte    *storage;
	int bit;
	int idx;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj || !obj->evalStorage ) {
		return qfalse;
	}

	bit = 1 << ( boneIndex & 7 );
	idx = boneIndex >> 3;

	if ( !( ( (const byte *)partBits )[idx] & bit ) ) {
		return qfalse;
	}

	storage = obj->evalStorage;
	if ( storage[idx + 32] & bit ) {
		return qfalse;
	}

	storage[idx] |= (byte)bit;
	return qtrue;
}

/* ---- SV_DObjSetControlRotTransIndex  0x004563F0 ---- */
qboolean SV_DObjSetControlRotTransIndex( int entityNum, int boneIndex,
										 const int *partBits ) {
	DObj_t  *obj;
	byte    *storage;
	int bit;
	int idx;

	obj = (DObj_t *)Com_GetServerDObj( entityNum );
	if ( !obj || !obj->evalStorage ) {
		return qfalse;
	}

	bit = 1 << ( boneIndex & 7 );
	idx = boneIndex >> 3;

	if ( !( ( (const byte *)partBits )[idx] & bit ) ) {
		return qfalse;
	}

	storage = obj->evalStorage;
	if ( storage[idx + 32] & bit ) {
		return qfalse;
	}

	storage[idx + 16] |= (byte)bit;
	storage[idx] |= (byte)bit;
	return qtrue;
}

/*
 * ---- MatrixTransformVector43  0x00456470 ----
 *
 * The exe carries this maths TWICE, at two addresses: universal/com_math.c's
 * copy at 0x0042F950 is the shared one every module calls, and this is the
 * server's own.  Static, because nothing outside this unit calls it and no
 * header declares it.  Whether retail really had two copies or the symbol at
 * 0x00456470 is mis-attached is unsettled.
 */
static float *MatrixTransformVector43( float *m, float *out, const float *in ) {
	out[0] = m[8] * in[2] + m[4] * in[1] + m[0] * in[0] + m[12];
	out[1] = m[9] * in[2] + m[5] * in[1] + m[1] * in[0] + m[13];
	out[2] = m[10] * in[2] + m[6] * in[1] + m[2] * in[0] + m[14];
	return m;
}

/* ---- SV_DObjDebugDrawBounds_m  0x004564C0 ---- */
void SV_DObjDebugDrawBounds_m( int entityNum ) {
	(void)entityNum;
}

/* ---- SV_ResetEntityParsePoint_m  0x004567F0 ---- */
void SV_ResetEntityParsePoint_m( void ) {
	sv.entityParsePoint = CM_EntityString();
}

    /* Trap numbers are the real ones; retail remaps them through the table at 0x0045789C. */

typedef enum
{
	G_PRINT                     = 0,
	G_ERROR                     = 1,
	G_ERROR_NOPREFIX            = 2,
	G_MILLISECONDS              = 3,
	G_CVAR_REGISTER             = 4,
	G_CVAR_UPDATE               = 5,
	G_CVAR_SET                  = 6,
	G_CVAR_VARIABLE_INTEGER_VALUE = 7,
	G_CVAR_VARIABLE_VALUE       = 8,
	G_CVAR_VARIABLE_STRING_BUFFER = 9,
	G_ARGC                      = 10,
	G_ARGV                      = 11,
	G_HUNK_ALLOC                = 12,
	G_HUNK_ALLOC_LOW            = 13,
	G_HUNK_ALLOC_ALIGN          = 14,
	G_HUNK_ALLOC_LOW_ALIGN      = 15,
	G_HUNK_ALLOC_TEMP           = 16,
	G_HUNK_FREE_TEMP            = 17,
	G_FS_FOPEN_FILE             = 18,
	G_FS_READ                   = 19,
	G_FS_WRITE                  = 20,
	G_FS_RENAME                 = 21,
	G_FS_FCLOSE_FILE            = 22,
	G_SEND_CONSOLE_COMMAND      = 23,
	G_LOCATE_GAME_DATA          = 24,
	G_DROP_CLIENT               = 25,
	G_SEND_SERVER_COMMAND       = 26,
	G_SET_CONFIGSTRING          = 27,
	G_GET_CONFIGSTRING          = 28,
	G_GET_CONFIGSTRING_CONST    = 29,
	G_GET_USERINFO              = 30,
	G_SET_USERINFO              = 31,
	G_GET_SERVERINFO            = 32,
	G_SET_BRUSH_MODEL           = 33,
	G_TRACE                     = 34,
	G_TRACE_CAPSULE             = 35,
	G_SIGHT_TRACE               = 36,
	G_SIGHT_TRACE_CAPSULE       = 37,
	G_SIGHT_TRACE_TO_ENTITY     = 38,
	G_CM_BOX_TRACE              = 39,
	G_CM_BOX_TRACE_2            = 40,
	G_CM_BOX_SIGHT_TRACE        = 41,
	G_CM_BOX_SIGHT_TRACE_2      = 42,
	G_LOCATIONAL_TRACE          = 43,
	G_POINT_CONTENTS            = 44,
	G_IN_PVS                    = 45,
	G_IN_PVS_IGNORE_PORTALS     = 46,
	G_IN_SNAPSHOT               = 47,
	G_ADJUST_AREA_PORTAL_STATE  = 48,
	G_AREAS_CONNECTED           = 49,
	G_LINKENTITY                = 50,
	G_UNLINKENTITY              = 51,
	G_ENTITIES_IN_BOX           = 52,
	G_ENTITY_CONTACT            = 53,
	G_GET_USERCMD               = 54,
	G_GET_ENTITY_TOKEN          = 55,
	G_FS_GETFILELIST            = 56,
	G_REAL_TIME                 = 57,
	G_SNAPVECTOR                = 58,
	G_ENTITY_CONTACT_CAPSULE    = 59,
	G_FIND_SOUND_ALIAS          = 60,
	G_PICK_SOUND_ALIAS          = 61,
	G_SOUND_ALIAS_INDEX         = 62,
	G_SURFACE_TYPE_FROM_NAME    = 63,
	G_SURFACE_TYPE_TO_NAME      = 64,
	G_ADD_TEST_CLIENT           = 65,
	G_GET_ARCHIVED_CLIENT_INFO  = 66,
	G_ADD_DEBUG_STRING          = 67,
	G_ADD_DEBUG_LINE            = 68,
	G_ENABLE_ARCHIVED_SNAPSHOT  = 69,
	G_Z_MALLOC                  = 70,
	G_Z_FREE                    = 71,
	G_XANIM_CREATE_TREE         = 72,
	G_XANIM_CREATE_SMALL_TREE   = 73,
	G_XANIM_FREE_SMALL_TREE     = 74,
	G_XMODEL_EXISTS             = 75,
	G_XMODEL_GET                = 76,
	G_DOBJ_CREATE               = 77,
	G_DOBJ_EXISTS               = 78,
	G_DOBJ_FREE                 = 79,
	G_DEREF_DWORD               = 80,
	G_XANIM_CLEAR_ANIM          = 82,
	G_XANIM_CLEAR_ANIM_NODE     = 83,
	G_XANIM_CLEAR_ANIM_CHILDREN = 84,
	G_XANIM_SET_ANIM_KNOB       = 85,
	G_XANIM_SET_ANIM_KNOB_ALL   = 86,
	G_XANIM_SET_ANIM_RATE       = 87,
	G_XANIM_SET_TIME            = 88,
	G_XANIM_SET_ANIM_LIMITED    = 89,
	G_XANIM_CLEAR_TREE          = 90,
	G_XANIM_IS_PRIMITIVE        = 91,
	G_XANIM_IS_LEAF             = 92,
	G_XANIM_LENGTH_MSEC         = 93,
	G_XANIM_LENGTH_SEC          = 94,
	G_XANIM_SET_ANIM_KNOB_INT   = 95,
	G_XANIM_SET_ANIM_INT_LIMITED = 96,
	G_XANIM_GET_ABS_DELTA       = 97,
	G_XANIM_GET_REL_DELTA       = 98,
	G_XANIM_GET_REL_DELTA_TIME  = 99,
	G_XANIM_GET_ABS_DELTA_TIME  = 100,
	G_XANIM_IS_LOOPED           = 101,
	G_XANIM_HAS_NOTETRACK       = 102,
	G_XANIM_GET_TIME            = 103,
	G_XANIM_GET_WEIGHT          = 104,
	G_DOBJ_DUMP_INFO            = 105,
	G_DOBJ_CREATE_SKEL_FOR_BONE = 106,
	G_DOBJ_SKEL_UP_TO_DATE      = 107,
	G_DOBJ_UPDATE_SERVER_TIME   = 108,
	G_DOBJ_INIT_SERVER_TIME     = 109,
	G_DOBJ_GET_HIERARCHY_BITS   = 110,
	G_DOBJ_CALC_ANIM            = 111,
	G_DOBJ_CALC_SKEL            = 112,
	G_XANIM_RESTORE_TREE        = 113,
	G_XANIM_SAVE_TREE           = 114,
	G_XANIM_COPY_TREE           = 115,
	G_DOBJ_NUM_PARTS            = 116,
	G_DOBJ_GET_PART_INDEX       = 117,
	G_DOBJ_GET_SKEL_BASE        = 118,
	G_DOBJ_DISPLAY_ANIM         = 119,
	G_XANIM_HAS_TIME            = 120,
	G_XANIM_CHILD_COUNT         = 121,
	G_XANIM_CHILD_BASE          = 122,
	G_XANIM_ROOT_PART_COUNT     = 123,
	G_XANIM_ROOT_PART_LIST      = 124,
	G_DOBJ_GET_SKEL_END         = 125,
	G_DOBJ_SET_ROT_TRANS_INDEX  = 126,
	G_DOBJ_SET_CONTROL_ROT_TRANS_INDEX = 127,
	G_XANIM_GET_ANIM_NAME       = 128,
	G_DOBJ_GET_TREE             = 129,
	G_XANIM_TREE_FIELD          = 130,
	G_DOBJ_DEBUG_DRAW_BOUNDS    = 131,
	G_GET_WEAPON_INFO_MEMORY    = 132,
	G_FREE_WEAPON_INFO_MEMORY   = 133,
	G_FREE_CLIENT_SCRIPT_PERS   = 134,
	G_RESET_ENTITY_PARSE_POINT  = 135,
	G_MEMSET                    = 200,
	G_MEMCPY                    = 201,
	G_STRNCPY                   = 202,
	G_SIN                       = 203,
	G_COS                       = 204,
	G_ATAN2                     = 205,
	G_SQRT                      = 206,
	G_MATRIXMULTIPLY            = 207,
	G_ANGLEVECTORS              = 208,
	G_PERPENDICULARVECTOR       = 209,
	G_FLOOR                     = 210,
	G_CEIL                      = 211
} gameImport_t;

/* ---- SV_ReturnFloat  no-address ---- */
static int SV_ReturnFloat( float f ) {
	int bits;

	memcpy( &bits, &f, sizeof( bits ) );
	return bits;
}

/* ---- SV_ArgFloat  no-address ---- */
static float SV_ArgFloat( const int *args, int n ) {
	float f;

	memcpy( &f, &args[n], sizeof( f ) );
	return f;
}

/* ---- SV_GameSystemCalls  0x00456800 ---- */
int SV_GameSystemCalls( int *args ) {
	switch ( args[0] ) {

	case G_PRINT:
		Com_Printf( "%s", (const char *)args[1] );
		return 0;

	case G_ERROR:
		Com_Error( ERR_DROP, "\x15%s", (const char *)args[1] );
		return 0;

	case G_ERROR_NOPREFIX:
		Com_Error( ERR_DROP, "%s", (const char *)args[1] );
		return 0;

	case G_MILLISECONDS:
		return Sys_Milliseconds();

	case G_CVAR_REGISTER:
		Cvar_Register( (vmCvar_t *)args[1], (const char *)args[2],
					   (const char *)args[3], args[4] );
		return 0;

	case G_CVAR_UPDATE:
		Cvar_Update( (vmCvar_t *)args[1] );
		return 0;

	case G_CVAR_SET:
		if ( sv_debugSpawn && sv_debugSpawn->integer > 1 ) {
			Com_Printf( "GCVAR: %s = \"%s\"\n", (const char *)args[1],
						(const char *)args[2] );
		}
		Cvar_Set2( (const char *)args[1], (const char *)args[2], qtrue );
		return 0;

	case G_CVAR_VARIABLE_INTEGER_VALUE:
		return Cvar_VariableIntegerValue( (const char *)args[1] );

	case G_CVAR_VARIABLE_VALUE:
		*(float *)args[2] = Cvar_VariableValue( (const char *)args[1] );
		return 0;

	case G_CVAR_VARIABLE_STRING_BUFFER:
		Cvar_VariableStringBuffer( (const char *)args[1], (char *)args[2],
								   args[3] );
		return 0;

	case G_ARGC:
		return Cmd_Argc();

	case G_ARGV:
		Cmd_ArgvBuffer( args[1], (char *)args[2], args[3] );
		return 0;

	case G_HUNK_ALLOC:
		if ( sv.state != SS_LOADING ) {
			Com_Error( ERR_DROP, "\x15trap_Hunk_Alloc can only be called in "
								 "G_InitGame and the first few frames of G_RunFrame\n" );
		}
		return (int)Hunk_AllocAlignInternal( args[1], 32 );

	case G_HUNK_ALLOC_LOW:
		if ( sv.state != SS_LOADING ) {
			Com_Error( ERR_DROP, "\x15trap_Hunk_AllocLow can only be called in "
								 "G_InitGame and the first few frames of G_RunFrame\n" );
		}
		return (int)Hunk_AllocLowAlignInternal( args[1], 32 );

	case G_HUNK_ALLOC_ALIGN:
		return (int)SV_Hunk_AllocAlignInternal( args[1], args[2] );

	case G_HUNK_ALLOC_LOW_ALIGN:
		return (int)SV_Hunk_AllocLowAlignInternal( args[1], args[2] );

	case G_HUNK_ALLOC_TEMP:
		return (int)Hunk_AllocateTempMemoryInternal( args[1] );

	case G_HUNK_FREE_TEMP:
		SV_Hunk_FreeTempMemoryInternal( (void *)args[1] );
		return 0;

	case G_FS_FOPEN_FILE:
		return FS_FOpenFileByMode( (const char *)args[1],
								   (fileHandle_t *)args[2], (fsMode_t)args[3] );

	case G_FS_READ:
		FS_Read( (void *)args[1], args[2], args[3] );
		return 0;

	case G_FS_WRITE:
		return FS_Write( (const void *)args[1], args[2], args[3] );

	case G_FS_RENAME:
		FS_Rename( (const char *)args[1], (const char *)args[2] );
		return 0;

	case G_FS_FCLOSE_FILE:
		FS_FCloseFile( args[1] );
		return 0;

	case G_FS_GETFILELIST:
	{
		int nfound = FS_GetFileList( (const char *)args[1], (const char *)args[2],
									 (char *)args[3], args[4] );
		if ( sv_debugSpawn && sv_debugSpawn->integer ) {
			Com_Printf( "FLIST: \"%s\" ext \"%s\" bufsz %i -> %i\n",
						(const char *)args[1], (const char *)args[2],
						args[4], nfound );
			if ( nfound > 0 ) {
				const char *p = (const char *)args[3];
				int k;
				for ( k = 0 ; k < nfound && k < 8 ; k++ ) {
					Com_Printf( "FLIST:    [%s]\n", p );
					p += strlen( p ) + 1;
				}
			}
		}
		return nfound;
	}

	case G_SEND_CONSOLE_COMMAND:
		Cbuf_ExecuteText( args[1], (const char *)args[2] );
		return 0;

	case G_LOCATE_GAME_DATA:
		SV_LocateGameData( (gentity_t *)args[1], args[2], args[3],
						   (playerState_t *)args[4], args[5] );
		return 0;

	case G_DROP_CLIENT:
		SV_GameDropClient( args[1], (const char *)args[2] );
		return 0;

	case G_SEND_SERVER_COMMAND:
		SV_GameSendServerCommand( args[1], args[2], (const char *)args[3] );
		return 0;

	case G_SET_CONFIGSTRING:
		if ( sv_debugSpawn && sv_debugSpawn->integer > 1 ) {
			Com_Printf( "GCS: %i = \"%.70s\"\n", args[1],
						args[2] ? (const char *)args[2] : "(null)" );
		}
		SV_SetConfigstring( args[1], (const char *)args[2] );
		return 0;

	case G_GET_CONFIGSTRING:
		SV_GetConfigstring( args[1], (char *)args[2], args[3] );
		return 0;

	case G_GET_CONFIGSTRING_CONST:
		/* Retail jumps into SV_GetConfigstring's tail at 0x00457E70. */
		if ( sv.configstrings[args[1]] ) {
			return (int)sv.configstrings[args[1]];
		}
		return (int)"";

	case G_GET_USERINFO:
		SV_GetUserinfo( args[1], (char *)args[2], args[3] );
		return 0;

	case G_SET_USERINFO:
		SV_SetUserinfo( args[1], (const char *)args[2] );
		return 0;

	case G_GET_SERVERINFO:
		SV_GetServerinfo( (char *)args[1], args[2] );
		return 0;

	case G_GET_USERCMD:
		SV_GetUsercmd( args[1], (usercmd_t *)args[2] );
		return 0;

	case G_ADD_TEST_CLIENT:
		SV_AddTestClient();
		return 0;

	case G_GET_ARCHIVED_CLIENT_INFO:
		return SV_GetArchivedClientInfo( (int *)args[2], (void *)args[3],
										 args[1] );

	case G_FREE_CLIENT_SCRIPT_PERS:
		SV_FreeClientScriptPers();
		return 0;

	case G_ENABLE_ARCHIVED_SNAPSHOT:
		SV_EnableArchivedSnapshot( args[1] );
		return 0;

	case G_SET_BRUSH_MODEL:
		SV_SetBrushModel( (gentity_t *)args[1] );
		return 0;

	case G_TRACE:
		SV_Trace( (const float *)args[5], (trace_t *)args[1],
				  (const float *)args[2], (const float *)args[3],
				  (const float *)args[4], args[6], args[7], 0, 0, NULL, 0 );
		return 0;

	case G_TRACE_CAPSULE:
		SV_Trace( (const float *)args[5], (trace_t *)args[1],
				  (const float *)args[2], (const float *)args[3],
				  (const float *)args[4], args[6], args[7], 1, 0, NULL, 0 );
		return 0;

	case G_LOCATIONAL_TRACE:
		SV_Trace( (const float *)args[3], (trace_t *)args[1],
				  (const float *)args[2], NULL, NULL, args[4], args[5],
				  0, 1, (const byte *)args[6], 1 );
		return 0;

	case G_SIGHT_TRACE:
		SV_SightTrace( (const float *)args[5], (const float *)args[2],
					   (const float *)args[3], (int *)args[1],
					   (const float *)args[4], args[6], args[7], args[8], 0 );
		return 0;

	case G_SIGHT_TRACE_CAPSULE:
		SV_SightTrace( (const float *)args[5], (const float *)args[2],
					   (const float *)args[3], (int *)args[1],
					   (const float *)args[4], args[6], args[7], args[8], 1 );
		return 0;

	case G_SIGHT_TRACE_TO_ENTITY:
		return SV_SightTraceToEntity( (const float *)args[1],
									  (const float *)args[4],
									  (const float *)args[2],
									  (const float *)args[3],
									  args[5], args[6], 1 );

	case G_CM_BOX_TRACE:
		CM_BoxTrace( (trace_t *)args[1], (const float *)args[2],
					 (const float *)args[3], (const float *)args[4],
					 (const float *)args[5], args[6], args[7], 0 );
		return 0;

	case G_CM_BOX_TRACE_2:
		CM_BoxTrace( (trace_t *)args[1], (const float *)args[2],
					 (const float *)args[3], (const float *)args[4],
					 (const float *)args[5], args[6], args[7], 1 );
		return 0;

	/* Same shape: oldHitNum is a literal 0 and the capsule flag lands in eax (`xor eax,eax` at 0x00456C14 vs `mov eax,1` at 0x00456C3B). */
	case G_CM_BOX_SIGHT_TRACE:
		return CM_BoxSightTrace( 0, (const float *)args[1],
								 (const float *)args[2],
								 (const float *)args[3],
								 (const float *)args[4], args[5], args[6], 0 );

	case G_CM_BOX_SIGHT_TRACE_2:
		return CM_BoxSightTrace( 0, (const float *)args[1],
								 (const float *)args[2],
								 (const float *)args[3],
								 (const float *)args[4], args[5], args[6], 1 );

	case G_POINT_CONTENTS:
		return SV_PointContents( (const float *)args[1], args[2], args[3] );

	case G_IN_PVS:
		return SV_inPVS( (const float *)args[1], (const float *)args[2] );

	case G_IN_PVS_IGNORE_PORTALS:
		return SV_inPVSIgnorePortals( (const float *)args[1],
									  (const float *)args[2] );

	case G_IN_SNAPSHOT:
		return SV_inSnapshot( (const float *)args[1], args[2] );

	case G_ADJUST_AREA_PORTAL_STATE:
		SV_AdjustAreaPortalState( (gentity_t *)args[1], args[2] );
		return 0;

	case G_AREAS_CONNECTED:
		return CM_AreasConnected( args[1], args[2] );

	case G_LINKENTITY:
		SV_LinkEntity( (gentity_t *)args[1] );
		return 0;

	case G_UNLINKENTITY:
		SV_UnlinkEntity( (gentity_t *)args[1] );
		return 0;

	case G_ENTITIES_IN_BOX:
		return CM_AreaEntities( (const float *)args[1], (const float *)args[2],
								(int *)args[3], args[4], args[5] );

	case G_ENTITY_CONTACT:
		return SV_EntityContact( (gentity_t *)args[3], (const float *)args[1],
								 (const float *)args[2], 0 );

	case G_ENTITY_CONTACT_CAPSULE:
		return SV_EntityContact( (gentity_t *)args[3], (const float *)args[1],
								 (const float *)args[2], 1 );

	case G_GET_ENTITY_TOKEN:
	{
		char        *token;
		char        *buffer = (char *)args[1];
		int size = args[2];

		token = Com_Parse( (char **)&sv.entityParsePoint );
		strncpy( buffer, token, size - 1 );
		buffer[size - 1] = 0;

		if ( !sv.entityParsePoint && !token[0] ) {
			return 0;
		}
		return 1;
	}

	case G_RESET_ENTITY_PARSE_POINT:
		sv.entityParsePoint = CM_EntityString();
		return 0;

	case G_REAL_TIME:
		return Com_RealTime( (void *)args[1] );

	case G_SNAPVECTOR:
		Sys_SnapVector( (float *)args[1] );
		return 0;

	case G_SURFACE_TYPE_FROM_NAME:
		return Com_SurfaceTypeFromName( (const char *)args[1] );

	case G_SURFACE_TYPE_TO_NAME:
		return (int)Com_SurfaceTypeToName( args[1] );

	case G_ADD_DEBUG_STRING:
		CL_AddDebugString( (const float *)args[1], (const float *)args[2],
						   SV_ArgFloat( args, 3 ), (const char *)args[4], 1 );
		return 0;

	case G_ADD_DEBUG_LINE:
		CL_AddDebugLine( (const float *)args[1], (const float *)args[2],
						 (const float *)args[3], args[4], args[5], 1 );
		return 0;

	case G_Z_MALLOC:
		return (int)Z_MallocInternal( args[1] );

	case G_Z_FREE:
		free( (void *)args[1] );
		return 0;

	case G_FIND_SOUND_ALIAS:
	{
		int *alias = Com_FindSoundAlias( (const char *)args[1], 2 );

		return alias ? *alias : 0;
	}

	case G_PICK_SOUND_ALIAS:
		return Com_PickSoundAlias( (const char *)args[1], 2 );

	case G_SOUND_ALIAS_INDEX:
		return Com_SoundAliasIndex( (void *)args[1], 2 );

	case G_GET_WEAPON_INFO_MEMORY:
		return (int) Com_GetWeaponInfoMemory( args[1], (int *)args[2], 1 );

	case G_FREE_WEAPON_INFO_MEMORY:
		Com_FreeWeaponInfoMemory( 1, args[1] );
		return 0;

	case G_XMODEL_EXISTS:
		return XModelExists( (const char *)args[1] );

	case G_XMODEL_GET:
		return (int)SV_XModelGet( (const char *)args[1] );

	case G_DOBJ_CREATE:
		Com_ServerDObjCreate( (void *)args[1], (unsigned short)args[2],
							  (void *)args[3], args[4],
							  (unsigned short)args[5] );
		return 0;

	case G_DOBJ_EXISTS:
		return Com_GetServerDObj( args[1] ) != NULL;

	case G_DOBJ_FREE:
		Com_SafeServerDObjFree( args[1], args[2] );
		return 0;

	case G_DEREF_DWORD:
		return *(int *)args[1];

	case G_DOBJ_DUMP_INFO:
		SV_DObjDumpInfo( args[1] );
		return 0;

	case G_DOBJ_CREATE_SKEL_FOR_BONE:
		return SV_DObjCreateSkelForBone( args[1], args[2] );

	case G_DOBJ_SKEL_UP_TO_DATE:
		return SV_DObjIsSkelUpToDate( args[1], (const int *)args[2] );

	case G_DOBJ_UPDATE_SERVER_TIME:
		return SV_DObjUpdateServerTime( args[1], args[3],
										SV_ArgFloat( args, 2 ) );

	case G_DOBJ_INIT_SERVER_TIME:
		SV_DObjInitServerTime( args[1], args[2] );
		return 0;

	case G_DOBJ_GET_HIERARCHY_BITS:
		SV_DObjGetHierarchyBits_m( args[1], args[2], (int *)args[3] );
		return 0;

	case G_DOBJ_CALC_ANIM:
		SV_DObjCalcAnim( args[1], (void *)args[2] );
		return 0;

	case G_DOBJ_CALC_SKEL:
		SV_DObjCalcSkel( args[1], (void *)args[2] );
		return 0;

	case G_DOBJ_NUM_PARTS:
		return SV_DObjNumParts( args[1] );

	case G_DOBJ_GET_PART_INDEX:
		return SV_DObjGetPartIndex( args[1], (const char *)args[2] );

	case G_DOBJ_GET_SKEL_BASE:
		return (int)SV_DObjGetSkelBase( args[1] );

	case G_DOBJ_GET_SKEL_END:
		return (int)SV_DObjGetSkelEnd( args[1] );

	case G_DOBJ_DISPLAY_ANIM:
		SV_DObjDisplayAnim( args[1] );
		return 0;

	case G_DOBJ_SET_ROT_TRANS_INDEX:
		return SV_DObjSetRotTransIndex( args[1], args[3], (const int *)args[2] );

	case G_DOBJ_SET_CONTROL_ROT_TRANS_INDEX:
		return SV_DObjSetControlRotTransIndex( args[1], args[3],
											   (const int *)args[2] );

	case G_DOBJ_GET_TREE:
	{
		DObj_t *obj = (DObj_t *)Com_GetServerDObj( args[1] );

		return obj ? (int)obj->tree : 0;
	}

	case G_DOBJ_DEBUG_DRAW_BOUNDS:
		SV_DObjDebugDrawBounds_m( args[1] );
		return 0;

	case G_XANIM_CLEAR_ANIM:
		XAnimClearAnim_m( args[2], (void *)args[1], SV_ArgFloat( args, 3 ) );
		return 0;

	case G_XANIM_CLEAR_ANIM_NODE:
		XAnimClearAnimNode_m( (void *)args[1], args[2], SV_ArgFloat( args, 3 ) );
		return 0;

	case G_XANIM_CLEAR_ANIM_CHILDREN:
		XAnimClearAnimChildSubtrees_m( (void *)args[1], args[2], SV_ArgFloat( args, 3 ) );
		return 0;

	case G_XANIM_SET_ANIM_KNOB:
		XAnimSetAnimKnob_m( (void *)args[1], args[2], SV_ArgFloat( args, 3 ), args[4],
					SV_ArgFloat( args, 5 ), (short)args[6], 0, args[7] );
		return 0;

	case G_XANIM_SET_ANIM_KNOB_ALL:
		return XAnimSetAnimKnobAll_m( args[2], (void *)args[1], args[3],
						   SV_ArgFloat( args, 4 ), args[5],
						   SV_ArgFloat( args, 6 ), (short)args[7], 0, args[8] );

	case G_XANIM_SET_ANIM_LIMITED:
		XAnimSetAnimLimited_m( args[2], (void *)args[1], SV_ArgFloat( args, 3 ), args[4],
					SV_ArgFloat( args, 5 ), (short)args[6], 0, args[7] );
		return 0;

	case G_XANIM_SET_ANIM_KNOB_INT:
		XAnimSetAnimKnobInternal_m( (short)args[6], (void *)args[1], args[2],
					SV_ArgFloat( args, 3 ), args[4], SV_ArgFloat( args, 5 ),
					0, args[7] );
		return 0;

	case G_XANIM_SET_ANIM_INT_LIMITED:
		XAnimSetAnimInternalLimited_m( 0, (short)args[6], args[1], args[2],
					SV_ArgFloat( args, 3 ), args[4], SV_ArgFloat( args, 5 ),
					args[7] );
		return 0;

	case G_XANIM_SET_ANIM_RATE:
		XAnimSetAnimRate_m( args[1], args[2], SV_ArgFloat( args, 3 ) );
		return 0;

	case G_XANIM_SET_TIME:
		XAnimSetTime_m( (void *)args[2], args[1], args[3] );
		return 0;

	case G_XANIM_CLEAR_TREE:
		XAnimClearTree( (void *)args[1] );
		return 0;

	case G_XANIM_GET_ABS_DELTA:
		XAnimGetAbsDelta_m( args[2], (void *)args[1], (void *)args[4],
					(float *)args[3] );
		return 0;

	case G_XANIM_GET_REL_DELTA:
		XAnimGetRelDelta_m( args[5], args[2], (void *)args[4], (float *)args[3],
					(void *)args[1] );
		return 0;

	case G_XANIM_GET_REL_DELTA_TIME:
		XAnimGetRelDeltaForTime_m( args[2], SV_XANIM_TREE( args[1] ), (void *)args[4],
					(float *)args[3], SV_ArgFloat( args, 5 ),
					SV_ArgFloat( args, 6 ) );
		return 0;

	case G_XANIM_GET_ABS_DELTA_TIME:
		XAnimGetAbsDeltaForTime_m( args[2], SV_XANIM_TREE( args[1] ), (float *)args[3],
					(void *)args[4], SV_ArgFloat( args, 5 ) );
		return 0;

	case G_XANIM_RESTORE_TREE:
		cand_XAnimRestoreTree( args[1] );
		return 0;

	case G_XANIM_SAVE_TREE:
		cand_XAnimSaveTree( (void *)args[1] );
		return 0;

	case G_XANIM_COPY_TREE:
		XAnimCopyTree( (const void *)args[1], (void *)args[2] );
		return 0;

	case G_XANIM_IS_LOOPED:
		return XAnimIsLooped_m( args[2], SV_XANIM_TREE( args[1] ) );

	case G_XANIM_GET_ANIM_NAME:
		return (int)XAnimGetAnimName( args[2], (void *)SV_XANIM_TREE( args[1] ) );

	case G_XANIM_GET_TIME:
		return SV_ReturnFloat( (float)XAnimGetTime( args[2], (void *)args[1] ) );

	case G_XANIM_GET_WEIGHT:
		return SV_ReturnFloat( (float)XAnimGetWeight( args[2], (void *)args[1] ) );

	case G_XANIM_HAS_TIME:
		return XAnimHasTime( args[2], (void *)args[1] );

	case G_XANIM_IS_PRIMITIVE:
	{
		int tree = SV_XANIM_TREE( args[1] );

		if ( !tree ) {
			return 1;
		}
		return !*(short *)( tree + 8 * args[2] + 8 )
			   || ( *(byte *)( tree + 8 * args[2] + 12 ) & 3 ) != 0;
	}

	case G_XANIM_IS_LEAF:
	{
		int tree = SV_XANIM_TREE( args[1] );

		if ( !tree ) {
			return 1;
		}
		return *(short *)( tree + 8 * args[2] + 8 ) == 0;
	}

	case G_XANIM_CHILD_COUNT:
	{
		int tree = SV_XANIM_TREE( args[1] );

		if ( !tree ) {
			return 0;
		}
		return *(unsigned short *)( tree + 8 * args[2] + 8 );
	}

	case G_XANIM_CHILD_BASE:
	{
		int tree = SV_XANIM_TREE( args[1] );

		if ( !tree ) {
			return args[3];
		}
		return args[3] + *(unsigned short *)( tree + 8 * args[2] + 14 );
	}

	case G_XANIM_LENGTH_MSEC:
	{
		float *parts = *(float **)( *(int *)( args[1] + 8 * args[2] + 12 ) + 4 );

		return (int)( (double)*(unsigned short *)parts / parts[1] * 1000.0 );
	}

	case G_XANIM_LENGTH_SEC:
	{
		int tree = SV_XANIM_TREE( args[1] );
		float *parts;

		if ( !tree ) {
			return SV_ReturnFloat( 0.0f );
		}
		parts = *(float **)( *(int *)( tree + 8 * args[2] + 12 ) + 4 );
		return SV_ReturnFloat( (float)( (double)*(unsigned short *)parts
										/ parts[1] ) );
	}

	case G_XANIM_HAS_NOTETRACK:
	{
		int tree = SV_XANIM_TREE( args[1] );
		short *note;

		if ( !tree ) {
			return 0;
		}
		note = *(short **)( *(int *)( *(int *)( tree + 8 * args[2] + 12 ) + 4 )
							+ 20 );
		while ( note && *note ) {
			if ( *note == (short)args[3] ) {
				return 1;
			}
			note += 4;
		}
		return 0;
	}

	case G_XANIM_ROOT_PART_COUNT:
		return ****(short ****)( **(int **)( args[1] + 4 ) + 4 );

	case G_XANIM_ROOT_PART_LIST:
		return ***(int ***)( **(int **)( args[1] + 4 ) + 4 ) + 2;

	case G_XANIM_TREE_FIELD:
		return *(int *)( args[1] + 4 );

	/* XAnim tree creation ---------------- ONE argument each, not two -- see the prototypes at the top of the file. Retail loads a single register at 0x00457001 / 0x0045700E / 0x0045701B and pushes nothing. */
	case G_XANIM_CREATE_TREE:
		return (int)Com_XAnimCreateTree( (void *)args[1] );

	case G_XANIM_CREATE_SMALL_TREE:
		return (int)Com_XAnimCreateSmallTree( (void *)args[1] );

	case G_XANIM_FREE_SMALL_TREE:
		Com_XAnimFreeSmallTree( (void *)args[1] );
		return 0;

	case G_MEMSET:
		memset( (void *)args[1], args[2], args[3] );
		return 0;

	case G_MEMCPY:
		memcpy( (void *)args[1], (const void *)args[2], args[3] );
		return 0;

	case G_STRNCPY:
		return (int)strncpy( (char *)args[1], (const char *)args[2], args[3] );

	case G_SIN:
		return SV_ReturnFloat( (float)sin( SV_ArgFloat( args, 1 ) ) );

	case G_COS:
		return SV_ReturnFloat( (float)cos( SV_ArgFloat( args, 1 ) ) );

	case G_ATAN2:
		return SV_ReturnFloat( (float)atan2( SV_ArgFloat( args, 1 ),
											 SV_ArgFloat( args, 2 ) ) );

	case G_SQRT:
		return SV_ReturnFloat( (float)sqrt( SV_ArgFloat( args, 1 ) ) );

	case G_FLOOR:
		return SV_ReturnFloat( (float)floor( SV_ArgFloat( args, 1 ) ) );

	case G_CEIL:
		return SV_ReturnFloat( (float)ceil( SV_ArgFloat( args, 1 ) ) );

	case G_MATRIXMULTIPLY:
		MatrixMultiply( (float (*)[3])args[1], (float (*)[3])args[2],
						(float (*)[3])args[3] );
		return 0;

	case G_ANGLEVECTORS:
		AngleVectors( (const float *)args[1], (float *)args[2],
					  (float *)args[3], (float *)args[4] );
		return 0;

	case G_PERPENDICULARVECTOR:
		PerpendicularVector( (float *)args[1], (const float *)args[2] );
		return 0;

	default:
		Com_Error( ERR_DROP, "\x15" "Bad game system trap: %i", args[0] );
		return -1;
	}
}

/* ---- SV_ShutdownGameProgs  0x00457970 ---- */
void SV_ShutdownGameProgs( void ) {
	void    *lowEnd;
	void    *highEnd;

	if ( !vm ) {
		return;
	}

	xanim_activePoolSlot = SV_XANIM_POOL_SERVER;
	VM_Call( vm, GAME_SHUTDOWN, qfalse );

	lowEnd = (char *)s_hunkData + HUNK_LOW_MARK;
	highEnd = (char *)s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM;

	HUNK_LOW_TEMP = HUNK_LOW_MARK;
	HUNK_LOW_PERM = HUNK_LOW_MARK;

	XModelClearData( highEnd, lowEnd );
	FS_ClearDataForFiles( lowEnd, highEnd );

	VM_Free( vm );
	vm = NULL;

	if ( snd_aliasSetLoaded[SND_LOCALE_LOCALIZED] ) {
		snd_aliasLocalizedName[0] = '\0';
		if ( snd_aliasTable[SND_LOCALE_LOCALIZED] ) {
			free( (void *)snd_aliasTable[SND_LOCALE_LOCALIZED] );
			snd_aliasTable[SND_LOCALE_LOCALIZED] = NULL;
			snd_aliasTableCount[SND_LOCALE_LOCALIZED] = 0;
			memset( snd_aliasHashTable[SND_LOCALE_LOCALIZED], 0,
					SND_ALIAS_HASH_SLICE_BYTES );
		}
		snd_aliasSetLoaded[SND_LOCALE_LOCALIZED] = 0;
	}
}

/* ---- SV_InitGameVM  0x00457A60 ---- */
void SV_InitGameVM( qboolean restart, int matchState ) {
	void    *engineCallbacks;
	void    *gameCallbacks;
	int i;

	sv.entityParsePoint = CM_EntityString();

	engineCallbacks = Scr_NearHook( NULL );
	gameCallbacks = (void *)VM_Call( vm, GAME_SCRIPT_FAR_HOOK, engineCallbacks );
	Scr_NearHook( gameCallbacks );

	Sys_LoadingKeepAlive();
	VM_Call( vm, GAME_INIT, svs.time, Com_Milliseconds(), restart, matchState );
	Sys_LoadingKeepAlive();

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}

	if ( com_dedicated->integer ) {
		Com_CvarDump( 4 );
	}
}

/* ---- SV_RestartGameProgs  0x00457B00 ---- */
void SV_RestartGameProgs( int matchState ) {
	void    *lowEnd;
	void    *highEnd;

	VM_Call( vm, GAME_SHUTDOWN, qtrue );

	lowEnd = (char *)s_hunkData + HUNK_LOW_MARK;
	highEnd = (char *)s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM;

	HUNK_LOW_TEMP = HUNK_LOW_MARK;
	HUNK_LOW_PERM = HUNK_LOW_MARK;

	XModelClearData( highEnd, lowEnd );
	FS_ClearDataForFiles( lowEnd, highEnd );

	SV_InitGameVM( qtrue, matchState );
}

/* ---- SV_InitGameProgs  0x00457B60 ---- */
void SV_InitGameProgs( void ) {
	vm = VM_Create( "game", SV_GameSystemCalls );
	if ( !vm ) {
		Com_Error( ERR_FATAL, "\x15VM_Create on game failed" );
	}

	HUNK_LOW_MARK = HUNK_LOW_PERM;

	SV_InitGameVM( qfalse, 0 );
}

/* ---- SV_GameCommand  0x00457BB0 ---- */
qboolean SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return qfalse;
	}

	xanim_activePoolSlot = SV_XANIM_POOL_SERVER;
	return VM_Call( vm, GAME_CONSOLE_COMMAND_SV );
}
