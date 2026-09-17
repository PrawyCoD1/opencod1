/*
 * @fidelity: verified (except the DObj bounds branch of SV_LinkEntity)
 */

#define moveclip_t          cod1_opaque_moveclip_t
#define pointtrace_t        cod1_opaque_pointtrace_t
#define sightclip_t         cod1_opaque_sightclip_t
#define sightpointtrace_t   cod1_opaque_sightpointtrace_t

#include "server.h"

#undef moveclip_t
#undef pointtrace_t
#undef sightclip_t
#undef sightpointtrace_t

#include <string.h>
#include <math.h>

/* entityShared_t +0x0C, Q3/RTCW/UO's `bmodel` (server.h spells it unknown_0x0C[4]). Set by SV_SetBrushModel, read here. */
#define GENT_BMODEL( g )            ( *(int *)( ( g )->r.unknown_0x0C ) )

#define SVENT_NUMCLUSTERS( e )      ( *(int *)( (byte *)( e ) + 0x118 ) )
#define SVENT_CLUSTERNUMS( e )      (  (int *)( (byte *)( e ) + 0x11C ) )
#define SVENT_LASTCLUSTER( e )      ( *(int *)( (byte *)( e ) + 0x15C ) )
#define SVENT_AREANUM( e )          ( *(int *)( (byte *)( e ) + 0x160 ) )
#define SVENT_AREANUM2( e )         ( *(int *)( (byte *)( e ) + 0x164 ) )

#define SVF_CAPSULE                 0x00000200

/* The retail leaf-list size at 0x0045EF8C (`push 80h`). */
#define MAX_TOTAL_ENT_LEAFS         128

#define SVF_USE_CLIP_BOUNDS         0x00000002
#define SVF_USE_MODEL_BOUNDS        0x00000004
#define SVF_DOBJ_BOUNDS_MASK        ( SVF_USE_CLIP_BOUNDS | SVF_USE_MODEL_BOUNDS )

#define GAME_DOBJ_CALC_POSE         13

typedef struct moveclip_t {
	vec3_t mins;                    /* +0x00 -halfSize */
	vec3_t maxs;                    /* +0x0C +halfSize */
	vec3_t expandedHalfSize;        /* +0x18 halfSize + 1 */
	vec3_t start;                   /* +0x24 start + box centre */
	vec3_t end;                     /* +0x30 end + box centre */
	trace_t trace;                  /* +0x3C (48 bytes) */
	int passEntityNum;              /* +0x6C */
	int passOwnerNum;               /* +0x70 */
	int contentmask;                /* +0x74 */
	qboolean capsule;               /* +0x78 */
} moveclip_t;
SV_ASSERT_SIZE( moveclip_t, 124 );
COD1_STATIC_ASSERT( sizeof( moveclip_t ) == sizeof( cod1_opaque_moveclip_t ),
					"moveclip_t must match its proven size" );

typedef struct pointtrace_t {
	vec3_t start;                   /* +0x00 */
	vec3_t end;                     /* +0x0C */
	trace_t trace;                  /* +0x18 (48 bytes) */
	int passEntityNum;              /* +0x48 */
	int passOwnerNum;               /* +0x4C */
	int contentmask;                /* +0x50 */
	qboolean useDObj;               /* +0x54 */
	const byte      *dobjTracePartState; /* +0x58 */
	int unknown_0x5C;               /* +0x5C never written, never read */
	int unknown_0x60;               /* +0x60 never written, never read */
} pointtrace_t;
SV_ASSERT_SIZE( pointtrace_t, 100 );
COD1_STATIC_ASSERT( sizeof( pointtrace_t ) == sizeof( cod1_opaque_pointtrace_t ),
					"pointtrace_t must match its proven size" );

typedef struct sightclip_t {
	vec3_t mins;                    /* +0x00 -halfSize */
	vec3_t maxs;                    /* +0x0C +halfSize */
	vec3_t expandedHalfSize;        /* +0x18 halfSize + 1 */
	vec3_t start;                   /* +0x24 start + box centre */
	vec3_t end;                     /* +0x30 end + box centre */
	int passEntityNum;              /* +0x3C */
	int passOwnerNum;               /* +0x40 */
	int passEntityOwnerNum;         /* +0x44 */
	int passOwnerOwnerNum;          /* +0x48 */
	int contentmask;                /* +0x4C */
	qboolean capsule;               /* +0x50 */
} sightclip_t;
SV_ASSERT_SIZE( sightclip_t, 84 );
COD1_STATIC_ASSERT( sizeof( sightclip_t ) == sizeof( cod1_opaque_sightclip_t ),
					"sightclip_t must match its proven size" );

typedef struct sightpointtrace_t {
	vec3_t start;                   /* +0x00 */
	vec3_t end;                     /* +0x0C */
	int passEntityNum;              /* +0x18 */
	int passOwnerNum;               /* +0x1C */
	int passEntityOwnerNum;         /* +0x20 */
	int passOwnerOwnerNum;          /* +0x24 */
	int contentmask;                /* +0x28 */
} sightpointtrace_t;
SV_ASSERT_SIZE( sightpointtrace_t, 44 );
COD1_STATIC_ASSERT( sizeof( sightpointtrace_t ) == sizeof( cod1_opaque_sightpointtrace_t ),
					"sightpointtrace_t must match its proven size" );

typedef struct svOrientation_t {
	vec3_t axis[3];                 /* +0x00 */
	vec3_t origin;                  /* +0x24 */
} svOrientation_t;
SV_ASSERT_SIZE( svOrientation_t, 48 );

typedef struct svDObjTrace_t {
	float fraction;                 /* +0x00 */
	int surfaceFlags;               /* +0x04 */
	vec3_t normal;                  /* +0x08 model space */
	unsigned short partName;        /* +0x14 */
	unsigned short partGroup;       /* +0x16 */
	byte startsolid;                /* +0x18 */
	byte allsolid;                  /* +0x19 */
	byte unknown_0x1A[2];           /* +0x1A */
} svDObjTrace_t;
SV_ASSERT_SIZE( svDObjTrace_t, 28 );

extern int CM_NumInlineModels( void );
extern clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs,
									 int contents, qboolean capsule );
extern int CM_BoxLeafnums( const vec3_t mins, const vec3_t maxs, int *list,
						   int listsize, int *lastLeaf );
extern int CM_LeafCluster( int leafnum );
extern int CM_LeafArea( int leafnum );
extern int CM_PointContents( const vec3_t p, clipHandle_t model );
extern int CM_TransformedPointContents( const vec3_t p, clipHandle_t model,
										const vec3_t origin,
										const vec3_t angles );
extern int CM_AreaEntities( const vec3_t mins, const vec3_t maxs,
							int *entityList, int maxcount, int contentmask );
extern void CM_LinkEntity( void *ent, const vec3_t absmin, const vec3_t absmax );
extern void CM_UnlinkEntity( void *ent );

extern void CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						 const vec3_t mins, const vec3_t maxs,
						 clipHandle_t model, int brushmask, qboolean capsule );
extern void CM_TransformedBoxTrace( trace_t *results, const vec3_t start,
									const vec3_t end, const vec3_t mins,
									const vec3_t maxs, clipHandle_t model,
									int brushmask, const vec3_t origin,
									const vec3_t angles, qboolean capsule );
extern int CM_BoxSightTrace( int oldHitNum, const vec3_t start,
							 const vec3_t end, const vec3_t mins,
							 const vec3_t maxs, clipHandle_t model,
							 int brushmask, qboolean capsule );
extern int CM_TransformedBoxSightTrace( int oldHitNum, const vec3_t start,
										const vec3_t end, const vec3_t mins,
										const vec3_t maxs, clipHandle_t model,
										int brushmask, const vec3_t origin,
										const vec3_t angles, qboolean capsule );

extern void CM_PointTraceStaticModels( trace_t *results, const vec3_t start,
									   const vec3_t end, int contentmask );
extern qboolean CM_TraceBox( const vec3_t start, const vec3_t end,
							 const vec3_t mins, const vec3_t maxs,
							 float fraction );
extern void CM_ClipMoveToEntities( moveclip_t *clip );
extern void CM_PointTraceToEntities( pointtrace_t *clip );
extern qboolean CM_ClipSightTraceToEntities( sightclip_t *clip );
extern qboolean CM_PointSightTraceToEntities( sightpointtrace_t *clip );

extern float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );

extern void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
extern float *MatrixTransformVector( float *matrix, float *out, float *in );
extern float *MatrixTransposeTransformVector( float *matrix, float *worldPoint,
											  float *localOut );

extern int  DObjGetBounds( int dobj, float *mins, float *maxs );
extern char DObjTraceParts( svDObjTrace_t *trace, int dobj, float *start,
							float *end, const byte *partState );
extern int  DObjTraceModelParts( svDObjTrace_t *trace, int dobj, float *start,
								 float *end, int contentmask );
extern int  DObjHasContents( int dobj, int contentmask );

extern gentity_t *SV_GentityNum( int num );
extern svEntity_t *SV_SvEntityForGentity( gentity_t *gent );

/* ---- SV_ClipHandleForEntity  0x0045EBD0 ---- */
clipHandle_t SV_ClipHandleForEntity( gentity_t *gent ) {
	int index;

	if ( GENT_BMODEL( gent ) ) {
		index = gent->s.index;
		if ( index < 0 || index >= CM_NumInlineModels() ) {
			Com_Error( ERR_DROP, "\x15" "CM_InlineModel: bad number" );
		}
		return index;
	}

	return CM_TempBoxModel( gent->r.mins, gent->r.maxs, gent->r.contents,
							( gent->r.svFlags & SVF_CAPSULE ) != 0 );
}

/* ---- SV_UnlinkEntity  0x0045EC40 ---- */
void SV_UnlinkEntity( gentity_t *gent ) {
	svEntity_t *svEnt;

	svEnt = SV_SvEntityForGentity( gent );
	gent->r.linked = qfalse;
	CM_UnlinkEntity( svEnt );
}

/* ---- SnapAngles  0x0045EC80 ---- */
void SnapAngles( float *v ) {
	int i;
	float rounded;
	float delta;

	for ( i = 0 ; i < 3 ; i++ ) {
		rounded = (float)(int)( v[i] + 9.313225746154785e-10f );
		delta = rounded - v[i];
		if ( delta * delta < 0.0000010000001f ) {
			v[i] = rounded;
		}
	}
}

/* ---- SV_LinkEntity  0x0045ECE0 ---- */
void SV_LinkEntity( gentity_t *gent ) {
	svEntity_t *svEnt;
	int leafs[MAX_TOTAL_ENT_LEAFS];
	int lastLeaf;
	int num;
	int i;
	int area;
	int cluster;
	float radius;
	int packed;

	svEnt = SV_SvEntityForGentity( gent );

	if ( GENT_BMODEL( gent ) ) {
		gent->s.solid = 0x00FFFFFF;
	} else if ( gent->r.contents & 0x02000001 ) {
		int sx, sz, zmax;

		sx = (int)gent->r.maxs[0];
		if ( sx < 1 ) {
			sx = 1;
		} else if ( sx > 255 ) {
			sx = 255;
		}

		sz = (int)( -gent->r.mins[2] );
		if ( sz < 1 ) {
			sz = 1;
		} else if ( sz > 255 ) {
			sz = 255;
		}

		zmax = (int)( gent->r.maxs[2] + 32.0f );
		if ( zmax < 1 ) {
			zmax = 1;
		} else if ( zmax > 255 ) {
			zmax = 255;
		}

		packed = ( ( ( zmax << 8 ) | sz ) << 8 ) | sx;
		gent->s.solid = packed;
	} else {
		gent->s.solid = 0;
	}

	SnapAngles( gent->r.currentAngles );

	if ( GENT_BMODEL( gent )
		 && ( gent->r.currentAngles[0] != 0.0f
			  || gent->r.currentAngles[1] != 0.0f
			  || gent->r.currentAngles[2] != 0.0f ) ) {
		radius = RadiusFromBounds( gent->r.mins, gent->r.maxs );

		gent->r.absmin[0] = gent->r.currentOrigin[0] - radius;
		gent->r.absmin[1] = gent->r.currentOrigin[1] - radius;
		gent->r.absmin[2] = gent->r.currentOrigin[2] - radius;
		gent->r.absmax[0] = gent->r.currentOrigin[0] + radius;
		gent->r.absmax[1] = gent->r.currentOrigin[1] + radius;
		gent->r.absmax[2] = gent->r.currentOrigin[2] + radius;
	} else {
		gent->r.absmin[0] = gent->r.mins[0] + gent->r.currentOrigin[0];
		gent->r.absmin[1] = gent->r.mins[1] + gent->r.currentOrigin[1];
		gent->r.absmin[2] = gent->r.mins[2] + gent->r.currentOrigin[2];
		gent->r.absmax[0] = gent->r.maxs[0] + gent->r.currentOrigin[0];
		gent->r.absmax[1] = gent->r.maxs[1] + gent->r.currentOrigin[1];
		gent->r.absmax[2] = gent->r.maxs[2] + gent->r.currentOrigin[2];
	}

	gent->r.absmin[0] -= 1.0f;
	gent->r.absmin[1] -= 1.0f;
	gent->r.absmin[2] -= 1.0f;
	gent->r.absmax[0] += 1.0f;
	gent->r.absmax[1] += 1.0f;
	gent->r.absmax[2] += 1.0f;

	SVENT_NUMCLUSTERS( svEnt ) = 0;
	SVENT_LASTCLUSTER( svEnt ) = 0;
	SVENT_AREANUM( svEnt ) = -1;
	SVENT_AREANUM2( svEnt ) = -1;

	lastLeaf = 0;
	num = CM_BoxLeafnums( gent->r.absmin, gent->r.absmax, leafs,
						  MAX_TOTAL_ENT_LEAFS, &lastLeaf );
	if ( num == 0 ) {
		CM_UnlinkEntity( svEnt );
		return;
	}

	for ( i = 0 ; i < num ; i++ ) {
		area = CM_LeafArea( leafs[i] );
		if ( area == -1 ) {
			continue;
		}

		if ( SVENT_AREANUM( svEnt ) == -1 || SVENT_AREANUM( svEnt ) == area ) {
			SVENT_AREANUM( svEnt ) = area;
			continue;
		}

		if ( SVENT_AREANUM2( svEnt ) != -1 && SVENT_AREANUM2( svEnt ) != area
			 && sv.state == SS_LOADING ) {
			Com_DPrintf( "Object %i touching 3 areas at %f %f %f\n",
						 gent->s.number, gent->r.absmin[0],
						 gent->r.absmin[1], gent->r.absmin[2] );
		}
		SVENT_AREANUM2( svEnt ) = area;
	}

	for ( i = 0 ; i < num ; i++ ) {
		cluster = CM_LeafCluster( leafs[i] );
		if ( cluster == -1 ) {
			continue;
		}
		SVENT_CLUSTERNUMS( svEnt )[SVENT_NUMCLUSTERS( svEnt )] = cluster;
		SVENT_NUMCLUSTERS( svEnt )++;
		if ( SVENT_NUMCLUSTERS( svEnt ) == MAX_ENT_CLUSTERS ) {
			break;
		}
	}

	if ( i != num ) {
		SVENT_LASTCLUSTER( svEnt ) = CM_LeafCluster( lastLeaf );
	}

	gent->r.linked = qtrue;

	if ( sv_debugSpawn && sv_debugSpawn->integer
		 && gent->s.number < sv_maxclients->integer ) {
		void *dobj = Com_GetServerDObj( gent->s.number );

		Com_Printf( "SPAWNDBG: Link ent %i svFlags 0x%x contents 0x%x dobj %p "
					"solid 0x%x\n",
					gent->s.number, gent->r.svFlags, gent->r.contents,
					dobj, gent->s.solid );
		Com_Printf( "SPAWNDBG:   mins %.1f %.1f %.1f maxs %.1f %.1f %.1f "
					"org %.1f %.1f %.1f\n",
					gent->r.mins[0], gent->r.mins[1], gent->r.mins[2],
					gent->r.maxs[0], gent->r.maxs[1], gent->r.maxs[2],
					gent->r.currentOrigin[0], gent->r.currentOrigin[1],
					gent->r.currentOrigin[2] );
		if ( dobj && ( gent->r.svFlags & SVF_DOBJ_BOUNDS_MASK ) ) {
			Com_Printf( "SPAWNDBG:   *** retail would link this from DObj "
						"bounds; this build uses the axial box ***\n" );
		}
	}

	if ( gent->r.contents ) {
		CM_LinkEntity( svEnt, gent->r.absmin, gent->r.absmax );
	} else {
		CM_UnlinkEntity( svEnt );
	}
}

/* ---- SV_PointContents  0x00460150 ---- */
int SV_PointContents( const vec3_t p, int passEntityNum, int contentmask ) {
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	int num;
	int i;
	int contents;

	contents = CM_PointContents( p, 0 );

	num = CM_AreaEntities( p, p, touch, MAX_GENTITIES, contentmask );

	for ( i = 0 ; i < num ; i++ ) {
		if ( touch[i] == passEntityNum ) {
			continue;
		}
		hit = SV_GentityNum( touch[i] );
		contents |= CM_TransformedPointContents( p,
												 SV_ClipHandleForEntity( hit ),
												 hit->r.currentOrigin,
												 hit->r.currentAngles );
	}

	return contents & contentmask;
}

    /* 0x0045F1E0-0x0046014A: the six entity-collision functions, UO's sv_entity_collision.c under UO names. */

static const vec3_t sv_defaultEntityClipMins = { -64.0f, -64.0f, -32.0f };
static const vec3_t sv_defaultEntityClipMaxs = {  64.0f,  64.0f,  72.0f };

/* ---- SV_EntityNumForSvEntity  no-address ---- */
static int SV_EntityNumForSvEntity( svEntity_t *svEnt ) {
	return (int)( svEnt - sv.svEntities );
}

/* ---- SV_TraceOwnerNum  no-address ---- */
static int SV_TraceOwnerNum( int entityNum ) {
	int ownerNum;

	if ( entityNum == ENTITYNUM_NONE ) {
		return -1;
	}
	ownerNum = SV_GentityNum( entityNum )->r.ownerNum;
	if ( ownerNum == ENTITYNUM_NONE ) {
		return -1;
	}
	return ownerNum;
}

/* ---- SV_TraceEntitySkipped  no-address ---- */
static qboolean SV_TraceEntitySkipped( int entityNum, gentity_t *gent,
									   int passEntityNum, int passOwnerNum ) {
	if ( passEntityNum == ENTITYNUM_NONE ) {
		return qfalse;
	}
	if ( entityNum == passEntityNum ) {
		return qtrue;
	}
	if ( gent->r.ownerNum == passEntityNum ) {
		return qtrue;
	}
	if ( gent->r.ownerNum == passOwnerNum ) {
		return qtrue;
	}
	return qfalse;
}

/* ---- SV_TraceAnglesForEntity  no-address ---- */
static const float *SV_TraceAnglesForEntity( gentity_t *gent ) {
	if ( !GENT_BMODEL( gent ) ) {
		return vec3_origin;
	}
	return gent->r.currentAngles;
}

/* ---- SV_ClipMoveToEntity  0x0045F1E0 ---- */
void SV_ClipMoveToEntity( moveclip_t *clip, svEntity_t *touch ) {
	int entityNum;
	gentity_t *gent;
	trace_t trace;

	entityNum = SV_EntityNumForSvEntity( touch );
	gent = SV_GentityNum( entityNum );

	if ( ( gent->r.contents & clip->contentmask ) == 0 ) {
		return;
	}
	if ( SV_TraceEntitySkipped( entityNum, gent, clip->passEntityNum,
								clip->passOwnerNum ) ) {
		return;
	}

	trace.fraction = clip->trace.fraction;
	CM_TransformedBoxTrace( &trace, clip->start, clip->end,
							clip->mins, clip->maxs,
							SV_ClipHandleForEntity( gent ),
							clip->contentmask, gent->r.currentOrigin,
							SV_TraceAnglesForEntity( gent ), clip->capsule );

	if ( trace.fraction >= clip->trace.fraction ) {
		clip->trace.allsolid |= trace.allsolid;
		clip->trace.startsolid |= trace.startsolid;
		return;
	}

	trace.allsolid |= clip->trace.allsolid;
	trace.startsolid |= clip->trace.startsolid;
	trace.entityNum = (unsigned short)gent->s.number;
	clip->trace = trace;
}

/* ---- SV_PointTraceToEntity  0x0045F300 ---- */
void SV_PointTraceToEntity( pointtrace_t *clip, svEntity_t *touch ) {
	int entityNum;
	gentity_t *gent;
	void *dobj;
	trace_t trace;
	vec3_t dobjMins, dobjMaxs;
	vec3_t localStart, localEnd;
	svOrientation_t orient;
	svDObjTrace_t dobjTrace;
	int i;

	entityNum = SV_EntityNumForSvEntity( touch );
	gent = SV_GentityNum( entityNum );

	if ( ( gent->r.contents & clip->contentmask ) == 0 ) {
		return;
	}
	if ( SV_TraceEntitySkipped( entityNum, gent, clip->passEntityNum,
								clip->passOwnerNum ) ) {
		return;
	}

	/* Com_GetServerDObj is retail's dobj_serverHandles[]/dobj_pool[] pair inlined at 0x0045F385-0x0045F39C; it returns NULL for handle 0. */
	dobj = NULL;
	if ( clip->useDObj ) {
		dobj = Com_GetServerDObj( gent->s.number );
	}

	if ( dobj != NULL && ( gent->r.svFlags & SVF_DOBJ_BOUNDS_MASK ) ) {
		if ( gent->r.svFlags & SVF_USE_MODEL_BOUNDS ) {
			if ( !DObjHasContents( (int)dobj, clip->contentmask ) ) {
				return;
			}
			DObjGetBounds( (int)dobj, dobjMins, dobjMaxs );
		} else {
			VectorCopy( sv_defaultEntityClipMins, dobjMins );
			VectorCopy( sv_defaultEntityClipMaxs, dobjMaxs );
		}

		for ( i = 0 ; i < 3 ; i++ ) {
			dobjMins[i] += gent->r.currentOrigin[i];
			dobjMaxs[i] += gent->r.currentOrigin[i];
		}

		if ( CM_TraceBox( clip->start, clip->end, dobjMins, dobjMaxs,
						  clip->trace.fraction ) ) {
			return;
		}

		VM_Call( vm, GAME_DOBJ_CALC_POSE, gent->s.number );

		AnglesToAxis( gent->r.currentAngles, orient.axis );
		VectorCopy( gent->r.currentOrigin, orient.origin );
		MatrixTransposeTransformVector( (float *)&orient, clip->start,
										localStart );
		MatrixTransposeTransformVector( (float *)&orient, clip->end,
										localEnd );

		dobjTrace.fraction = clip->trace.fraction;
		if ( gent->r.svFlags & SVF_USE_MODEL_BOUNDS ) {
			DObjTraceModelParts( &dobjTrace, (int)dobj, localStart, localEnd,
						clip->contentmask );
		} else {
			DObjTraceParts( &dobjTrace, (int)dobj, localStart, localEnd,
						clip->dobjTracePartState );
		}

		if ( dobjTrace.fraction >= clip->trace.fraction ) {
			clip->trace.allsolid |= dobjTrace.allsolid;
			clip->trace.startsolid |= dobjTrace.startsolid;
			return;
		}

		trace.fraction = dobjTrace.fraction;
		trace.surfaceFlags = dobjTrace.surfaceFlags;
		trace.partName = dobjTrace.partName;
		trace.partGroup = dobjTrace.partGroup;
		trace.startsolid = dobjTrace.startsolid;
		/* Retail reads a stale stack byte here (0x0045F66B); zeroed instead -- the conservative direction. */
		trace.allsolid = 0;

		MatrixTransformVector( (float *)&orient, trace.normal,
							   dobjTrace.normal );

		trace.endpos[0] = clip->start[0]
						  + ( clip->end[0] - clip->start[0] ) * trace.fraction;
		trace.endpos[1] = clip->start[1]
						  + ( clip->end[1] - clip->start[1] ) * trace.fraction;
		trace.endpos[2] = clip->start[2]
						  + ( clip->end[2] - clip->start[2] ) * trace.fraction;
	} else {
		trace.fraction = clip->trace.fraction;
		CM_TransformedBoxTrace( &trace, clip->start, clip->end,
								vec3_origin, vec3_origin,
								SV_ClipHandleForEntity( gent ),
								clip->contentmask, gent->r.currentOrigin,
								SV_TraceAnglesForEntity( gent ), qfalse );

		if ( trace.fraction >= clip->trace.fraction ) {
			clip->trace.allsolid |= trace.allsolid;
			clip->trace.startsolid |= trace.startsolid;
			return;
		}
	}

	trace.allsolid |= clip->trace.allsolid;
	trace.startsolid |= clip->trace.startsolid;
	trace.contents = gent->r.contents;
	trace.material = 0;
	trace.entityNum = (unsigned short)gent->s.number;
	clip->trace = trace;
}

/* ---- SV_ClipSightToEntity  0x0045F730 ---- */
qboolean SV_ClipSightToEntity( sightclip_t *clip, svEntity_t *touch ) {
	int entityNum;
	gentity_t *gent;

	entityNum = SV_EntityNumForSvEntity( touch );
	gent = SV_GentityNum( entityNum );

	if ( ( gent->r.contents & clip->contentmask ) == 0 ) {
		return 0;
	}
	if ( SV_TraceEntitySkipped( entityNum, gent, clip->passEntityNum,
								clip->passEntityOwnerNum ) ) {
		return 0;
	}
	if ( SV_TraceEntitySkipped( entityNum, gent, clip->passOwnerNum,
								clip->passOwnerOwnerNum ) ) {
		return 0;
	}

	if ( !CM_TransformedBoxSightTrace( 0, clip->start, clip->end,
									   clip->mins, clip->maxs,
									   SV_ClipHandleForEntity( gent ),
									   clip->contentmask,
									   gent->r.currentOrigin,
									   SV_TraceAnglesForEntity( gent ),
									   clip->capsule ) ) {
		return 0;
	}
	return -1;
}

/* ---- SV_PointSightTraceToEntity  0x0045F7F0 ---- */
qboolean SV_PointSightTraceToEntity( sightpointtrace_t *clip,
									 svEntity_t *touch ) {
	int entityNum;
	gentity_t *gent;

	entityNum = SV_EntityNumForSvEntity( touch );
	gent = SV_GentityNum( entityNum );

	if ( ( gent->r.contents & clip->contentmask ) == 0 ) {
		return 0;
	}
	if ( SV_TraceEntitySkipped( entityNum, gent, clip->passEntityNum,
								clip->passEntityOwnerNum ) ) {
		return 0;
	}
	if ( SV_TraceEntitySkipped( entityNum, gent, clip->passOwnerNum,
								clip->passOwnerOwnerNum ) ) {
		return 0;
	}

	if ( !CM_TransformedBoxSightTrace( 0, clip->start, clip->end,
									   vec3_origin, vec3_origin,
									   SV_ClipHandleForEntity( gent ),
									   clip->contentmask,
									   gent->r.currentOrigin,
									   SV_TraceAnglesForEntity( gent ),
									   qfalse ) ) {
		return 0;
	}
	return -1;
}

/* ---- SV_Trace  0x0045F8B0 ---- */
void SV_Trace( const vec3_t end, trace_t *results, const vec3_t start,
			   const vec3_t mins, const vec3_t maxs, int passEntityNum,
			   int contentmask, qboolean capsule, qboolean useDObj,
			   const byte *dobjTracePartState, qboolean locational ) {
	const float *traceMins;
	const float *traceMaxs;
	trace_t worldTrace;
	moveclip_t clip;
	pointtrace_t pclip;
	float halfSize;
	float centre;
	int i;

	traceMins = mins ? mins : vec3_origin;
	traceMaxs = maxs ? maxs : vec3_origin;

	Com_Memset( &worldTrace, 0, sizeof( worldTrace ) );
	CM_BoxTrace( &worldTrace, start, end, traceMins, traceMaxs, 0,
				 contentmask, capsule );

	worldTrace.entityNum = ENTITYNUM_WORLD;
	if ( worldTrace.fraction == 1.0f ) {
		worldTrace.entityNum = ENTITYNUM_NONE;
	}

	if ( worldTrace.fraction == 0.0f ) {
		*results = worldTrace;
		return;
	}

	if ( locational ) {
		CM_PointTraceStaticModels( &worldTrace, start, end, contentmask );
		if ( worldTrace.fraction == 0.0f ) {
			*results = worldTrace;
			return;
		}
	}

	if ( traceMaxs[0] - traceMins[0]
		 + traceMaxs[1] - traceMins[1]
		 + traceMaxs[2] - traceMins[2] == 0.0f ) {
		VectorCopy( start, pclip.start );
		VectorCopy( end, pclip.end );
		pclip.trace = worldTrace;
		pclip.passEntityNum = passEntityNum;
		pclip.passOwnerNum = SV_TraceOwnerNum( passEntityNum );
		pclip.contentmask = contentmask;
		pclip.useDObj = useDObj;
		pclip.dobjTracePartState = dobjTracePartState;

		CM_PointTraceToEntities( &pclip );

		*results = pclip.trace;
		return;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		halfSize = ( traceMaxs[i] - traceMins[i] ) * 0.5f;
		centre = ( traceMaxs[i] + traceMins[i] ) * 0.5f;

		clip.mins[i] = -halfSize;
		clip.maxs[i] = halfSize;
		clip.expandedHalfSize[i] = halfSize + 1.0f;
		clip.start[i] = start[i] + centre;
		clip.end[i] = end[i] + centre;
	}

	clip.trace = worldTrace;
	clip.passEntityNum = passEntityNum;
	clip.passOwnerNum = SV_TraceOwnerNum( passEntityNum );
	clip.contentmask = contentmask;
	clip.capsule = capsule;

	CM_ClipMoveToEntities( &clip );

	if ( clip.trace.fraction < worldTrace.fraction ) {
		clip.trace.endpos[0] = start[0]
							   + ( end[0] - start[0] ) * clip.trace.fraction;
		clip.trace.endpos[1] = start[1]
							   + ( end[1] - start[1] ) * clip.trace.fraction;
		clip.trace.endpos[2] = start[2]
							   + ( end[2] - start[2] ) * clip.trace.fraction;
	}

	*results = clip.trace;
}

/* ---- SV_SightTrace  0x0045FCA0 ---- */
void SV_SightTrace( const vec3_t end, const vec3_t start, const vec3_t mins,
					int *hitOut, const vec3_t maxs, int passEntityNum,
					int passOwnerNum, int contentmask, qboolean capsule ) {
	const float *traceMins;
	const float *traceMaxs;
	sightclip_t clip;
	sightpointtrace_t pclip;
	float halfSize;
	float centre;
	int i;

	traceMins = mins ? mins : vec3_origin;
	traceMaxs = maxs ? maxs : vec3_origin;

	*hitOut = CM_BoxSightTrace( *hitOut, start, end, traceMins, traceMaxs, 0,
								contentmask, capsule );
	if ( *hitOut ) {
		return;
	}

	if ( traceMaxs[0] - traceMins[0]
		 + traceMaxs[1] - traceMins[1]
		 + traceMaxs[2] - traceMins[2] == 0.0f ) {
		VectorCopy( start, pclip.start );
		VectorCopy( end, pclip.end );
		pclip.passEntityNum = passEntityNum;
		pclip.passOwnerNum = passOwnerNum;
		pclip.passEntityOwnerNum = SV_TraceOwnerNum( passEntityNum );
		pclip.passOwnerOwnerNum = SV_TraceOwnerNum( passOwnerNum );
		pclip.contentmask = contentmask;

		*hitOut = CM_PointSightTraceToEntities( &pclip );
		return;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		halfSize = ( traceMaxs[i] - traceMins[i] ) * 0.5f;
		centre = ( traceMaxs[i] + traceMins[i] ) * 0.5f;

		clip.mins[i] = -halfSize;
		clip.maxs[i] = halfSize;
		clip.expandedHalfSize[i] = halfSize + 1.0f;
		clip.start[i] = start[i] + centre;
		clip.end[i] = end[i] + centre;
	}

	clip.passEntityNum = passEntityNum;
	clip.passOwnerNum = passOwnerNum;
	clip.passEntityOwnerNum = SV_TraceOwnerNum( passEntityNum );
	clip.passOwnerOwnerNum = SV_TraceOwnerNum( passOwnerNum );
	clip.contentmask = contentmask;
	clip.capsule = capsule;

	*hitOut = CM_ClipSightTraceToEntities( &clip );
}

/* ---- SV_SightTraceToEntity  0x0045FFA0 ---- */
int SV_SightTraceToEntity( const vec3_t start, const vec3_t end,
						   const vec3_t mins, const vec3_t maxs,
						   int entityNum, int contentmask,
						   qboolean capsule ) {
	gentity_t *gent;
	vec3_t traceAbsMin;
	vec3_t traceAbsMax;
	int i;

	gent = SV_GentityNum( entityNum );

	if ( ( gent->r.contents & contentmask ) == 0 ) {
		return 0;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( start[i] < end[i] ) {
			traceAbsMin[i] = start[i] + mins[i] - 1.0f;
			traceAbsMax[i] = end[i] + maxs[i] + 1.0f;
		} else {
			traceAbsMin[i] = end[i] + mins[i] - 1.0f;
			traceAbsMax[i] = start[i] + maxs[i] + 1.0f;
		}
	}

	if ( traceAbsMax[0] < gent->r.absmin[0]
		 || traceAbsMax[1] < gent->r.absmin[1]
		 || traceAbsMax[2] < gent->r.absmin[2]
		 || gent->r.absmax[0] < traceAbsMin[0]
		 || gent->r.absmax[1] < traceAbsMin[1]
		 || gent->r.absmax[2] < traceAbsMin[2] ) {
		return 0;
	}

	if ( !CM_TransformedBoxSightTrace( 0, start, end, mins, maxs,
									   SV_ClipHandleForEntity( gent ),
									   contentmask, gent->r.currentOrigin,
									   SV_TraceAnglesForEntity( gent ),
									   capsule ) ) {
		return 0;
	}
	return -1;
}
