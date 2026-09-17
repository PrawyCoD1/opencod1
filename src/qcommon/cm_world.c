/*
 * qcommon/cm_world.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_world.c
 *
 * Retail range 0x00426AB0-0x00428130, 22 functions.
 *
 * @fidelity: verified
 */
#define sv                  cm_world_sv_placeholder
#define server_public_t     cm_world_server_public_t
#include "cm_local.h"
#undef server_public_t
#undef sv

typedef struct worldSector_t
{
	int axis;                               /* +0x00  0 = x, 1 = y     */
	int staticModelContents;                /* +0x04 */
	int sightStaticModelContents;           /* +0x08  big models only  */
	int entityContents;                     /* +0x0C */
	float dist;                             /* +0x10  split plane      */
	struct cmEntityLink_t   *entities;      /* +0x14 */
	struct cmStaticModelLink_t *staticModels; /* +0x18 */
	struct worldSector_t    *parent;        /* +0x1C  also the free list link */
	struct worldSector_t    *children[2];   /* +0x20  [0] above, [1] below */
} worldSector_t;
COD1_ASSERT_SIZE( worldSector_t, 40 );

typedef struct cmEntityLink_t
{
	worldSector_t           *worldSector;   /* +0x000  NULL when unlinked */
	struct cmEntityLink_t   *next;          /* +0x004 */
	byte owned_by_server[352];              /* +0x008  baseline, etc.     */
	int linkContents;                       /* +0x168 */
	float linkMins[2];                      /* +0x16C  x,y only           */
	float linkMaxs[2];                      /* +0x174 */
} cmEntityLink_t;
COD1_ASSERT_SIZE( cmEntityLink_t, 380 );

typedef struct cmStaticModelLink_t
{
	void                    *model;         /* +0x00  XModel *          */
	vec3_t origin;                          /* +0x04 */
	vec3_t inverseAxis[3];                  /* +0x10 */
	struct cmStaticModelLink_t *next;       /* +0x34 */
	vec3_t linkMins;                        /* +0x38 */
	vec3_t linkMaxs;                        /* +0x44 */
	qboolean isBig;                         /* +0x50  sight-trace eligible */
} cmStaticModelLink_t;
COD1_ASSERT_SIZE( cmStaticModelLink_t, 84 );

float sv_worldMinsX, sv_worldMinsY, sv_worldMinsZ;
float sv_worldMaxsX, sv_worldMaxsY, sv_worldMaxsZ;

worldSector_t sv_worldSectors = { 0 };      /* 0x016175A8  the ROOT, not an array */
worldSector_t *sv_freeWorldEntities = NULL; /* 0x016175D0  free-sector list head  */
worldSector_t sv_listHeadNull = { 0 };      /* 0x016175D4  the "no child" sentinel */

#define CM_MAX_WORLD_SECTORS    1024
worldSector_t sv_worldSectorPool[CM_MAX_WORLD_SECTORS];
#define sv_worldSectorPool      sv_worldSectorPool

#define CM_SV_HEAD_BYTES        ( 20 + 1028 + 4 * 0x800 )

typedef struct cmServerView_t
{
	byte head[CM_SV_HEAD_BYTES];
	cmEntityLink_t svEntities[MAX_GENTITIES];
	const char      *entityParsePoint;
	byte            *gentities;
	int gentitySize;
	byte tail[200];
} cmServerView_t;
COD1_ASSERT_SIZE( cmServerView_t, 398572 );

extern cmServerView_t sv;

#define CM_GENTITY_CONTENTS     ( 240 + 40 )
#define CM_GENTITY_ABSMIN       ( 240 + 44 )
#define CM_GENTITY_ABSMAX       ( 240 + 56 )

void SV_ClipMoveToEntity( moveclip_t *clip, cmEntityLink_t *touch );
qboolean SV_ClipSightToEntity( sightclip_t *clip, cmEntityLink_t *touch );
void SV_PointTraceToEntity( pointtrace_t *clip, cmEntityLink_t *touch );
qboolean SV_PointSightTraceToEntity( sightpointtrace_t *clip,
									 cmEntityLink_t *touch );

void CM_TraceStaticModel( cmStaticModelLink_t *link, trace_t *trace,
						  const vec3_t start, const vec3_t end,
						  int contentmask );

void Com_DPrintf( const char *fmt, ... );

static void CM_SortNode( worldSector_t *node, const float mins[2],
						 const float maxs[2] );

static byte *CM_GEntityForEntityLink( const cmEntityLink_t *ent ) {
	int num = ( int )( ent - sv.svEntities );

	return sv.gentities + sv.gentitySize * num;
}

static int CM_EntityLinkContents( const cmEntityLink_t *ent ) {
	return *( int * )( CM_GEntityForEntityLink( ent ) + CM_GENTITY_CONTENTS );
}

/* CM_StaticModelContents  0x00428064  VERIFIED */
static int CM_StaticModelContents( void *model ) {
	return *( int * )( *( byte ** )( ( byte * )model + 4 ) + 72 );
}

/* ---- CM_AllocWorldSector  0x00426AB0 ---- VERIFIED */
static worldSector_t *CM_AllocWorldSector( const float mins[2],
										   const float maxs[2] ) {
	worldSector_t *sector;
	float size[2];
	int axis;

	sector = sv_freeWorldEntities;
	if ( !sector ) {
		return NULL;
	}

	size[0] = maxs[0] - mins[0];
	size[1] = maxs[1] - mins[1];
	axis = ( size[0] <= size[1] );

	if ( size[axis] <= 512.0f ) {
		return NULL;
	}

	sv_freeWorldEntities = sector->parent;
	sector->axis = axis;
	sector->dist = ( maxs[axis] + mins[axis] ) * 0.5f;
	sector->children[0] = &sv_listHeadNull;
	sector->children[1] = &sv_listHeadNull;

	return sector;
}

/* CM_ClearWorldSectors  0x00419D52 */
void CM_ClearWorldSectors( void ) {
	sv_worldMinsX = 0.0f;
	sv_worldMinsY = 0.0f;
	sv_worldMinsZ = 0.0f;
	sv_worldMaxsX = 0.0f;
	sv_worldMaxsY = 0.0f;
	sv_worldMaxsZ = 0.0f;

	Com_Memset( &sv_worldSectors, 0, sizeof( sv_worldSectors ) );
	sv_freeWorldEntities = NULL;
	Com_Memset( &sv_listHeadNull, 0, sizeof( sv_listHeadNull ) );
	Com_Memset( sv_worldSectorPool, 0, sizeof( sv_worldSectorPool ) );
}

/* ---- CM_ClearWorld  0x00426B30 ---- VERIFIED */
void CM_ClearWorld( void ) {
	cmodel_t *cmod;
	vec3_t mins, maxs;
	int i;
	int axis;

	cmod = CM_ClipHandleToModel( CM_InlineModel( 0 ) );
	VectorCopy( cmod->mins, mins );
	VectorCopy( cmod->maxs, maxs );

	sv_worldMinsX = mins[0];
	sv_worldMinsY = mins[1];
	sv_worldMinsZ = mins[2];
	sv_worldMaxsX = maxs[0];
	sv_worldMaxsY = maxs[1];
	sv_worldMaxsZ = maxs[2];

	sv_freeWorldEntities = &sv_worldSectorPool[0];
	for ( i = 0; i < CM_MAX_WORLD_SECTORS - 1; i++ ) {
		sv_worldSectorPool[i].parent = &sv_worldSectorPool[i + 1];
	}
	sv_worldSectorPool[CM_MAX_WORLD_SECTORS - 1].parent = NULL;

	axis = ( ( maxs[0] - mins[0] ) <= ( maxs[1] - mins[1] ) );
	sv_worldSectors.axis = axis;
	sv_worldSectors.children[0] = &sv_listHeadNull;
	sv_worldSectors.children[1] = &sv_listHeadNull;
	sv_worldSectors.dist = ( mins[axis] + maxs[axis] ) * 0.5f;
}

/* ---- CM_UnlinkEntity  0x00426C10 ---- VERIFIED */
void CM_UnlinkEntity( cmEntityLink_t *ent ) {
	worldSector_t *sector;
	worldSector_t *parent;
	cmEntityLink_t *scan;
	int contents;

	sector = ent->worldSector;
	if ( !sector ) {
		return;
	}

	ent->worldSector = NULL;

	if ( sector->entities == ent ) {
		sector->entities = ent->next;
	} else {
		for ( scan = sector->entities; scan->next != ent; scan = scan->next ) {
			;
		}
		scan->next = ent->next;
	}

	while ( sector->entities == NULL &&
			sector->staticModels == NULL &&
			sector->children[0] == &sv_listHeadNull &&
			sector->children[1] == &sv_listHeadNull ) {
		sector->entityContents = 0;

		parent = sector->parent;
		if ( !parent ) {
			break;
		}

		sector->parent = sv_freeWorldEntities;
		sv_freeWorldEntities = sector;

		if ( parent->children[0] == sector ) {
			parent->children[0] = &sv_listHeadNull;
		} else {
			parent->children[1] = &sv_listHeadNull;
		}

		sector = parent;
	}

	for ( ; sector; sector = sector->parent ) {
		contents = sector->children[0]->entityContents |
				   sector->children[1]->entityContents;

		for ( scan = sector->entities; scan; scan = scan->next ) {
			contents |= CM_EntityLinkContents( scan );
		}

		sector->entityContents = contents;
	}
}

/* ---- CM_SortNode  0x00426D00 ---- VERIFIED */
static void CM_SortNode( worldSector_t *node, const float mins[2],
						 const float maxs[2] ) {
	int axis;
	float dist;
	cmEntityLink_t *prevEnt, *ent, *nextEnt;
	cmStaticModelLink_t *prevLink, *link, *nextLink;
	worldSector_t *child;
	int contents;

	axis = node->axis;
	dist = node->dist;

	prevEnt = NULL;
	ent = node->entities;
	while ( ent ) {
		if ( dist < ent->linkMins[axis] ) {
			child = node->children[0];
			if ( child == &sv_listHeadNull ) {
				child = CM_AllocWorldSector( mins, maxs );
				if ( !child ) {
					return;
				}
				node->children[0] = child;
				child->parent = node;
			}
		} else if ( ent->linkMaxs[axis] < dist ) {
			child = node->children[1];
			if ( child == &sv_listHeadNull ) {
				child = CM_AllocWorldSector( mins, maxs );
				if ( !child ) {
					return;
				}
				node->children[1] = child;
				child->parent = node;
			}
		} else {
			prevEnt = ent;
			ent = ent->next;
			continue;
		}

		nextEnt = ent->next;
		ent->worldSector = child;
		ent->next = child->entities;
		child->entities = ent;
		child->entityContents |= CM_EntityLinkContents( ent );

		if ( prevEnt ) {
			prevEnt->next = nextEnt;
		} else {
			node->entities = nextEnt;
		}
		ent = nextEnt;
	}

	prevLink = NULL;
	link = node->staticModels;
	while ( link ) {
		if ( dist < link->linkMins[axis] ) {
			child = node->children[0];
			if ( child == &sv_listHeadNull ) {
				child = CM_AllocWorldSector( mins, maxs );
				if ( !child ) {
					return;
				}
				node->children[0] = child;
				child->parent = node;
			}
		} else if ( link->linkMaxs[axis] < dist ) {
			child = node->children[1];
			if ( child == &sv_listHeadNull ) {
				child = CM_AllocWorldSector( mins, maxs );
				if ( !child ) {
					return;
				}
				node->children[1] = child;
				child->parent = node;
			}
		} else {
			prevLink = link;
			link = link->next;
			continue;
		}

		nextLink = link->next;
		contents = CM_StaticModelContents( link->model );
		link->next = child->staticModels;
		child->staticModels = link;
		child->staticModelContents |= contents;
		if ( link->isBig ) {
			child->sightStaticModelContents |= contents;
		}

		if ( prevLink ) {
			prevLink->next = nextLink;
		} else {
			node->staticModels = nextLink;
		}
		link = nextLink;
	}
}

/* ---- CM_LinkEntity  0x00426EE0 ---- VERIFIED */
void CM_LinkEntity( cmEntityLink_t *ent, const vec3_t absmin,
					const vec3_t absmax ) {
	int contents;
	worldSector_t *sector;
	int axis;
	float dist;
	float sectorMins[2], sectorMaxs[2];
	qboolean stoppedAtNull;

	contents = CM_EntityLinkContents( ent );

	for ( ;; ) {
		sectorMins[0] = sv_worldMinsX;
		sectorMins[1] = sv_worldMinsY;
		sectorMaxs[0] = sv_worldMaxsX;
		sectorMaxs[1] = sv_worldMaxsY;
		sector = &sv_worldSectors;
		stoppedAtNull = qfalse;

		for ( ;; ) {
			sector->entityContents |= contents;
			axis = sector->axis;
			dist = sector->dist;

			if ( absmin[axis] > dist ) {
				sectorMins[axis] = dist;
				if ( sector->children[0] == &sv_listHeadNull ) {
					stoppedAtNull = qtrue;
					break;
				}
				sector = sector->children[0];
				continue;
			}

			if ( !( dist > absmax[axis] ) ) {
				break;
			}

			sectorMaxs[axis] = dist;
			if ( sector->children[1] == &sv_listHeadNull ) {
				stoppedAtNull = qtrue;
				break;
			}
			sector = sector->children[1];
		}

		if ( !stoppedAtNull && sector == ent->worldSector &&
			 ( ~contents & ent->linkContents ) == 0 ) {
			ent->linkContents = contents;
			ent->linkMins[0] = absmin[0];
			ent->linkMins[1] = absmin[1];
			ent->linkMaxs[0] = absmax[0];
			ent->linkMaxs[1] = absmax[1];
			return;
		}

		if ( ent->worldSector ) {
			if ( sector == ent->worldSector &&
				 ( ~contents & ent->linkContents ) == 0 ) {
				break;
			}
			CM_UnlinkEntity( ent );
			continue;
		}

		ent->worldSector = sector;
		ent->next = sector->entities;
		sector->entities = ent;
		break;
	}

	ent->linkContents = contents;
	ent->linkMins[0] = absmin[0];
	ent->linkMins[1] = absmin[1];
	ent->linkMaxs[0] = absmax[0];
	ent->linkMaxs[1] = absmax[1];
	CM_SortNode( sector, sectorMins, sectorMaxs );
}

/* ---- CM_LinkStaticModel  0x00427050 ---- VERIFIED */
void CM_LinkStaticModel( cmStaticModelLink_t *link ) {
	int contents;
	worldSector_t *sector;
	int axis;
	float dist;
	float sectorMins[2], sectorMaxs[2];

	link->isBig = ( link->linkMaxs[2] - link->linkMins[2] >= 64.0f &&
					link->linkMaxs[1] - link->linkMins[1] >= 32.0f &&
					link->linkMaxs[0] - link->linkMins[0] >= 32.0f );

	contents = CM_StaticModelContents( link->model );

	sectorMins[0] = sv_worldMinsX;
	sectorMins[1] = sv_worldMinsY;
	sectorMaxs[0] = sv_worldMaxsX;
	sectorMaxs[1] = sv_worldMaxsY;
	sector = &sv_worldSectors;

	for ( ;; ) {
		sector->staticModelContents |= contents;
		if ( link->isBig ) {
			sector->sightStaticModelContents |= contents;
		}

		axis = sector->axis;
		dist = sector->dist;

		if ( dist < link->linkMins[axis] ) {
			sectorMins[axis] = dist;
			if ( sector->children[0] == &sv_listHeadNull ) {
				break;
			}
			sector = sector->children[0];
		} else {
			if ( dist <= link->linkMaxs[axis] ) {
				break;
			}
			sectorMaxs[axis] = dist;
			if ( sector->children[1] == &sv_listHeadNull ) {
				break;
			}
			sector = sector->children[1];
		}
	}

	link->next = sector->staticModels;
	sector->staticModels = link;
	CM_SortNode( sector, sectorMins, sectorMaxs );
}

typedef struct areaParms_t
{
	int unused;                     /* +0x00  never read */
	const float     *mins;          /* +0x04 */
	const float     *maxs;          /* +0x08 */
	int             *entityList;    /* +0x0C */
	int count;                      /* +0x10 */
	int maxcount;                   /* +0x14 */
	int contentmask;                /* +0x18 */
} areaParms_t;

/* ---- CM_AreaEntities_r  0x00427150 ---- VERIFIED */
static void CM_AreaEntities_r( worldSector_t *node, areaParms_t *ap ) {
	cmEntityLink_t *ent;
	const byte *gent;
	const float *absmin;
	const float *absmax;

	if ( ( ap->contentmask & node->entityContents ) == 0 ) {
		return;
	}

	for ( ent = node->entities; ent; ent = ent->next ) {
		gent = CM_GEntityForEntityLink( ent );

		if ( ( ap->contentmask &
			   *( const int * )( gent + CM_GENTITY_CONTENTS ) ) == 0 ) {
			continue;
		}

		absmin = ( const float * )( gent + CM_GENTITY_ABSMIN );
		absmax = ( const float * )( gent + CM_GENTITY_ABSMAX );

		if ( absmin[0] > ap->maxs[0] || ap->mins[0] > absmax[0] ||
			 absmin[1] > ap->maxs[1] || ap->mins[1] > absmax[1] ||
			 absmin[2] > ap->maxs[2] || ap->mins[2] > absmax[2] ) {
			continue;
		}

		if ( ap->count == ap->maxcount ) {
			Com_DPrintf( "CM_AreaEntities: MAXCOUNT\n" );
			return;
		}

		ap->entityList[ap->count] = ( int )( ent - sv.svEntities );
		ap->count++;
	}

	if ( node->dist < ap->maxs[node->axis] ) {
		CM_AreaEntities_r( node->children[0], ap );
	}
	if ( ap->mins[node->axis] < node->dist ) {
		CM_AreaEntities_r( node->children[1], ap );
	}
}

/* ---- CM_AreaEntities  0x004272A0 ---- VERIFIED */
int CM_AreaEntities( const vec3_t mins, const vec3_t maxs, int *entityList,
					 int maxcount, int contentmask ) {
	areaParms_t ap;

	ap.unused = 0;
	ap.mins = mins;
	ap.maxs = maxs;
	ap.entityList = entityList;
	ap.count = 0;
	ap.maxcount = maxcount;
	ap.contentmask = contentmask;

	CM_AreaEntities_r( &sv_worldSectors, &ap );

	return ap.count;
}

/* ---- CM_TraceBox  0x004272E0 ---- VERIFIED */
qboolean CM_TraceBox( const vec3_t start, const vec3_t end, const vec3_t mins,
					  const vec3_t maxs, float fraction ) {
	float enterFrac, leaveFrac, sign;
	const float *bounds;
	float d1, d2, denom;
	int axis;

	enterFrac = 0.0f;
	leaveFrac = fraction;
	sign = -1.0f;
	bounds = mins;

	for ( ;; ) {
		for ( axis = 0; axis < 3; axis++ ) {
			d1 = ( start[axis] - bounds[axis] ) * sign;
			d2 = ( end[axis] - bounds[axis] ) * sign;

			if ( d1 > 0.0f ) {
				if ( d2 > 0.0f ) {
					return qtrue;
				}
				denom = d1 - d2;
				if ( enterFrac * denom < d1 ) {
					enterFrac = d1 / denom;
					if ( leaveFrac <= enterFrac ) {
						return qtrue;
					}
				}
			} else if ( d2 > 0.0f ) {
				denom = d1 - d2;
				if ( leaveFrac * denom < d1 ) {
					leaveFrac = d1 / denom;
					if ( leaveFrac <= enterFrac ) {
						return qtrue;
					}
				}
			}
		}

		if ( sign == 1.0f ) {
			return qfalse;
		}
		sign = 1.0f;
		bounds = maxs;
	}
}

typedef struct pointStaticModelWork_t
{
	trace_t trace;                  /* +0x00 */
	int contentmask;                /* +0x30 */
	vec3_t start;                   /* +0x34 */
	vec3_t end;                     /* +0x40 */
} pointStaticModelWork_t;
COD1_ASSERT_SIZE( pointStaticModelWork_t, 76 );

/* ---- CM_PointTraceStaticModels_r  0x004273E0 ---- VERIFIED */
static void CM_PointTraceStaticModels_r( pointStaticModelWork_t *work,
										 worldSector_t *node, float p1f,
										 float p2f, const vec3_t p1,
										 const vec3_t p2 ) {
	int axis;
	float d1, d2, frac, midf;
	vec3_t mid;
	int side;
	cmStaticModelLink_t *link;

	if ( work->trace.fraction <= p1f ) {
		return;
	}
	if ( ( work->contentmask & node->staticModelContents ) == 0 ) {
		return;
	}

	axis = node->axis;
	d1 = p1[axis] - node->dist;
	d2 = p2[axis] - node->dist;

	if ( d1 >= 0.0f && d2 >= 0.0f ) {
		CM_PointTraceStaticModels_r( work, node->children[0], p1f, p2f,
									 p1, p2 );
	} else if ( d1 <= 0.0f && d2 <= 0.0f ) {
		CM_PointTraceStaticModels_r( work, node->children[1], p1f, p2f,
									 p1, p2 );
	} else {
		frac = d1 / ( d1 - d2 );
		midf = p1f + ( p2f - p1f ) * frac;
		mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac;
		mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac;
		mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac;

		side = ( d1 < 0.0f );
		CM_PointTraceStaticModels_r( work, node->children[side], p1f, midf,
									 p1, mid );
		CM_PointTraceStaticModels_r( work, node->children[1 - side], midf,
									 p2f, mid, p2 );
	}

	for ( link = node->staticModels; link; link = link->next ) {
		if ( ( CM_StaticModelContents( link->model ) &
			   work->contentmask ) == 0 ) {
			continue;
		}
		if ( CM_TraceBox( work->start, work->end, link->linkMins,
						  link->linkMaxs, work->trace.fraction ) ) {
			continue;
		}
		CM_TraceStaticModel( link, &work->trace, work->start, work->end,
							 work->contentmask );
	}
}

/* ---- CM_PointTraceStaticModels  0x004275A0 ---- VERIFIED */
void CM_PointTraceStaticModels( trace_t *results, const vec3_t start,
								const vec3_t end, int contentmask ) {
	pointStaticModelWork_t work;

	Com_Memset( &work.trace, 0, sizeof( work.trace ) );
	work.trace.fraction = results->fraction;
	work.contentmask = contentmask;
	VectorCopy( start, work.start );
	VectorCopy( end, work.end );

	CM_PointTraceStaticModels_r( &work, &sv_worldSectors, 0.0f,
								 work.trace.fraction, work.start, work.end );

	if ( work.trace.fraction < results->fraction ) {
		work.trace.endpos[0] = start[0] +
							   ( end[0] - start[0] ) * work.trace.fraction;
		work.trace.endpos[1] = start[1] +
							   ( end[1] - start[1] ) * work.trace.fraction;
		work.trace.endpos[2] = start[2] +
							   ( end[2] - start[2] ) * work.trace.fraction;
		*results = work.trace;
	}
}

/* ---- CM_ClipMoveToEntities_r  0x00427670 ---- VERIFIED */
static void CM_ClipMoveToEntities_r( moveclip_t *clip, worldSector_t *node,
									 float p1f, float p2f, const vec3_t p1,
									 const vec3_t p2 ) {
	int axis;
	float d1, d2, offset, frac, midf;
	vec3_t mid;
	int side;
	cmEntityLink_t *touch;

	if ( clip->trace.fraction <= p1f ) {
		return;
	}
	if ( ( clip->contentmask & node->entityContents ) == 0 ) {
		return;
	}

	axis = node->axis;
	d1 = p1[axis] - node->dist;
	d2 = p2[axis] - node->dist;
	offset = clip->expandedHalfSize[axis];

	if ( d1 >= offset && d2 >= offset ) {
		CM_ClipMoveToEntities_r( clip, node->children[0], p1f, p2f, p1, p2 );
	} else if ( d1 <= -offset && d2 <= -offset ) {
		CM_ClipMoveToEntities_r( clip, node->children[1], p1f, p2f, p1, p2 );
	} else {
		if ( d1 < d2 ) {
			side = 1;
			frac = ( d1 + offset ) / ( d1 - d2 );
		} else if ( d1 > d2 ) {
			side = 0;
			frac = ( d1 - offset ) / ( d1 - d2 );
		} else {
			side = 0;
			frac = 0.0f;
		}

		CM_ClipMoveToEntities_r( clip, node->children[side], p1f, p2f,
								 p1, p2 );

		if ( frac < 0.0f ) {
			frac = 0.0f;
		}
		midf = p1f + ( p2f - p1f ) * frac;
		mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac;
		mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac;
		mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac;

		CM_ClipMoveToEntities_r( clip, node->children[1 - side], midf, p2f,
								 mid, p2 );
	}

	for ( touch = node->entities; touch; touch = touch->next ) {
		SV_ClipMoveToEntity( clip, touch );
	}
}

/* ---- CM_ClipMoveToEntities  0x004278D0 ---- VERIFIED */
void CM_ClipMoveToEntities( moveclip_t *clip ) {
	CM_ClipMoveToEntities_r( clip, &sv_worldSectors, 0.0f,
							 clip->trace.fraction, clip->start, clip->end );
}

/* ---- CM_ClipSightTraceToEntities_r  0x004278F0 ---- VERIFIED */
static qboolean CM_ClipSightTraceToEntities_r( sightclip_t *clip,
											   worldSector_t *node,
											   const vec3_t p1,
											   const vec3_t p2 ) {
	int axis;
	float d1, d2, offset;
	int side;
	qboolean result;
	cmEntityLink_t *touch;

	if ( ( clip->contentmask & node->entityContents ) == 0 ) {
		return qfalse;
	}

	axis = node->axis;
	d1 = p1[axis] - node->dist;
	d2 = p2[axis] - node->dist;
	offset = clip->expandedHalfSize[axis];

	if ( d1 >= offset && d2 >= offset ) {
		result = CM_ClipSightTraceToEntities_r( clip, node->children[0],
											   p1, p2 );
		if ( result ) {
			return result;
		}
	} else if ( d1 <= -offset && d2 <= -offset ) {
		result = CM_ClipSightTraceToEntities_r( clip, node->children[1],
											   p1, p2 );
		if ( result ) {
			return result;
		}
	} else {
		side = ( d1 < d2 );

		result = CM_ClipSightTraceToEntities_r( clip, node->children[side],
											   p1, p2 );
		if ( result ) {
			return result;
		}
		result = CM_ClipSightTraceToEntities_r( clip, node->children[1 - side],
											   p1, p2 );
		if ( result ) {
			return result;
		}
	}

	for ( touch = node->entities; touch; touch = touch->next ) {
		result = SV_ClipSightToEntity( clip, touch );
		if ( result ) {
			return result;
		}
	}

	return qfalse;
}

/* ---- CM_ClipSightTraceToEntities  0x00427B60 ---- VERIFIED */
qboolean CM_ClipSightTraceToEntities( sightclip_t *clip ) {
	return CM_ClipSightTraceToEntities_r( clip, &sv_worldSectors,
										  clip->start, clip->end );
}

/* ---- CM_PointTraceToEntities_r  0x00427B80 ---- VERIFIED */
static void CM_PointTraceToEntities_r( pointtrace_t *clip, worldSector_t *node,
									   float p1f, float p2f, const vec3_t p1,
									   const vec3_t p2 ) {
	int axis;
	float d1, d2, frac, midf;
	vec3_t mid;
	int side;
	cmEntityLink_t *touch;

	if ( clip->trace.fraction <= p1f ) {
		return;
	}
	if ( ( clip->contentmask & node->entityContents ) == 0 ) {
		return;
	}

	axis = node->axis;
	d1 = p1[axis] - node->dist;
	d2 = p2[axis] - node->dist;

	if ( d1 >= 0.0f && d2 >= 0.0f ) {
		CM_PointTraceToEntities_r( clip, node->children[0], p1f, p2f, p1, p2 );
	} else if ( d1 <= 0.0f && d2 <= 0.0f ) {
		CM_PointTraceToEntities_r( clip, node->children[1], p1f, p2f, p1, p2 );
	} else {
		frac = d1 / ( d1 - d2 );
		midf = p1f + ( p2f - p1f ) * frac;
		mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac;
		mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac;
		mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac;

		side = ( d1 < 0.0f );
		CM_PointTraceToEntities_r( clip, node->children[side], p1f, midf,
								   p1, mid );
		CM_PointTraceToEntities_r( clip, node->children[1 - side], midf, p2f,
								   mid, p2 );
	}

	for ( touch = node->entities; touch; touch = touch->next ) {
		SV_PointTraceToEntity( clip, touch );
	}
}

/* ---- CM_PointTraceToEntities  0x00427D10 ---- VERIFIED */
void CM_PointTraceToEntities( pointtrace_t *clip ) {
	CM_PointTraceToEntities_r( clip, &sv_worldSectors, 0.0f,
							   clip->trace.fraction, clip->start, clip->end );
}

/* ---- CM_PointSightTraceToEntities_r  0x00427D30 ---- VERIFIED */
static qboolean CM_PointSightTraceToEntities_r( sightpointtrace_t *clip,
												worldSector_t *node,
												const vec3_t p1,
												const vec3_t p2 ) {
	int axis;
	float d1, d2;
	int side;
	qboolean result;
	cmEntityLink_t *touch;

	if ( ( clip->contentmask & node->entityContents ) == 0 ) {
		return qfalse;
	}

	axis = node->axis;
	d1 = p1[axis] - node->dist;
	d2 = p2[axis] - node->dist;

	if ( d1 >= 0.0f && d2 >= 0.0f ) {
		result = CM_PointSightTraceToEntities_r( clip, node->children[0],
												 p1, p2 );
		if ( result ) {
			return result;
		}
	} else if ( d1 <= 0.0f && d2 <= 0.0f ) {
		result = CM_PointSightTraceToEntities_r( clip, node->children[1],
												 p1, p2 );
		if ( result ) {
			return result;
		}
	} else {
		side = ( d1 < 0.0f );

		result = CM_PointSightTraceToEntities_r( clip, node->children[side],
												 p1, p2 );
		if ( result ) {
			return result;
		}
		result = CM_PointSightTraceToEntities_r( clip,
												 node->children[1 - side],
												 p1, p2 );
		if ( result ) {
			return result;
		}
	}

	for ( touch = node->entities; touch; touch = touch->next ) {
		result = SV_PointSightTraceToEntity( clip, touch );
		if ( result ) {
			return result;
		}
	}

	return qfalse;
}

/* ---- CM_PointSightTraceToEntities  0x00427EB0 ---- VERIFIED */
qboolean CM_PointSightTraceToEntities( sightpointtrace_t *clip ) {
	return CM_PointSightTraceToEntities_r( clip, &sv_worldSectors,
										   clip->start, clip->end );
}

typedef struct sightStaticModelWork_t
{
	int contentmask;                /* +0x00 */
	vec3_t start;                   /* +0x04 */
	vec3_t end;                     /* +0x10 */
} sightStaticModelWork_t;
COD1_ASSERT_SIZE( sightStaticModelWork_t, 28 );

/* ---- CM_SightTraceStaticModels_r  0x00427ED0 ---- VERIFIED */
static qboolean CM_SightTraceStaticModels_r( sightStaticModelWork_t *work,
											 worldSector_t *node, float p1f,
											 float p2f, const vec3_t p1,
											 const vec3_t p2 ) {
	int axis;
	float d1, d2, frac, midf;
	vec3_t mid;
	int side;
	cmStaticModelLink_t *link;
	trace_t trace;

	if ( ( work->contentmask & node->sightStaticModelContents ) == 0 ) {
		return qtrue;
	}

	axis = node->axis;
	d1 = p1[axis] - node->dist;
	d2 = p2[axis] - node->dist;

	if ( d1 >= 0.0f && d2 >= 0.0f ) {
		if ( !CM_SightTraceStaticModels_r( work, node->children[0], p1f, p2f,
										   p1, p2 ) ) {
			return qfalse;
		}
	} else if ( d1 <= 0.0f && d2 <= 0.0f ) {
		if ( !CM_SightTraceStaticModels_r( work, node->children[1], p1f, p2f,
										   p1, p2 ) ) {
			return qfalse;
		}
	} else {
		frac = d1 / ( d1 - d2 );
		midf = p1f + ( p2f - p1f ) * frac;
		mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac;
		mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac;
		mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac;

		side = ( d1 < 0.0f );
		if ( !CM_SightTraceStaticModels_r( work, node->children[side], p1f,
										   midf, p1, mid ) ) {
			return qfalse;
		}
		if ( !CM_SightTraceStaticModels_r( work, node->children[1 - side],
										   midf, p2f, mid, p2 ) ) {
			return qfalse;
		}
	}

	for ( link = node->staticModels; link; link = link->next ) {
		if ( !link->isBig ) {
			continue;
		}
		if ( ( work->contentmask &
			   CM_StaticModelContents( link->model ) ) == 0 ) {
			continue;
		}
		if ( CM_TraceBox( work->start, work->end, link->linkMins,
						  link->linkMaxs, 1.0f ) ) {
			continue;
		}

		trace.fraction = 1.0f;
		CM_TraceStaticModel( link, &trace, work->start, work->end,
							 work->contentmask );
		if ( trace.fraction != 1.0f ) {
			return qfalse;
		}
	}

	return qtrue;
}

/* ---- CM_SightTraceStaticModels  0x004280D0 ---- VERIFIED */
qboolean CM_SightTraceStaticModels( const vec3_t start, const vec3_t end,
									int contentmask ) {
	sightStaticModelWork_t work;

	work.contentmask = contentmask;
	VectorCopy( start, work.start );
	VectorCopy( end, work.end );

	return CM_SightTraceStaticModels_r( &work, &sv_worldSectors, 0.0f, 1.0f,
										work.start, work.end );
}

/* ---- CM_GetPlaneNum  0x00428130 ---- VERIFIED */
cplane_t *CM_GetPlaneNum( int num ) {
	return &cm_planes[num];
}
