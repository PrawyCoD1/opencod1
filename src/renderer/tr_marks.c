/*
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"
#include "tr_shaderregistry.h"


typedef struct markVert_s {
	vec3_t	xyz;			/* +0x00 */
	float	lightmapCoords[2];	/* +0x0C  carried through the chop */
} markVert_t;

typedef struct polyVert_s {
	vec3_t		xyz;			/* +0x00  R_AddMarkFragment */
	float		st[2];			/* +0x0C  RE_MarkFragments projects these */
	float		lightmapCoords[2];	/* +0x14  R_AddMarkFragment */
	unsigned char	modulate[4];		/* +0x1C  not touched here */
} polyVert_t;

typedef struct markFragment_s {
	int	shaderHandle;	/* +0x00  fragmentShader->index */
	int	firstPoint;	/* +0x04  index into pointBuffer */
	int	numPoints;	/* +0x08  written by R_AddMarkFragment */
} markFragment_t;

typedef struct markShader_s {
	char		name[64];		/* +0x00 */
	int		lightmapIndex;		/* +0x40  < 0 == surface has no lightmap */
	int		index;			/* +0x44  -> markFragment_t::shaderHandle */
	int		unk_48[5];		/* +0x48  sortedIndex, sort, ... unread here */
	unsigned int	surfaceParmFlags;	/* +0x5C  bit 0x20 == nomarks */
} markShader_t;

#define SURF_NOMARKS	0x20u

typedef struct msurface_s {
	int			viewCount;	/* +0x00  not read by this unit */
	markShader_t		*shader;	/* +0x04 */
	struct markSurface_s	*data;		/* +0x08 */
} msurface_t;

typedef struct markSurface_s {
	int		surfaceType;		/* +0x00 */
	int		storageMode;		/* +0x04  unread here */
	unsigned int	dlightBits;		/* +0x08  unread here */
	vec3_t		boundsMin;		/* +0x0C */
	vec3_t		boundsMax;		/* +0x18 */
	int		vertexCount;		/* +0x24  unread here */
	vec3_t		*tangents;		/* +0x28  unread here */
	vec3_t		*bitangents;		/* +0x2C  unread here */
	vec3_t		*normals;		/* +0x30  unread here */
	float		(*texCoords)[2];	/* +0x34  unread here */
	float		(*lightmapCoords)[2];	/* +0x38 */
	unsigned char	(*colors)[4];		/* +0x3C  unread here */
	vec3_t		*positions;		/* +0x40 */
	int		indexCount;		/* +0x44 */
	unsigned short	indices[1];		/* +0x48  runs to indexCount */
} markSurface_t;

typedef struct markCullGroup_s {
	vec3_t		mins;		/* +0x00 */
	vec3_t		maxs;		/* +0x0C */
	msurface_t	*surfaces;	/* +0x18 */
	int		surfaceCount;	/* +0x1C */
	int		viewCount;	/* +0x20  DPVS only; unread here */
} markCullGroup_t;

typedef struct markAabbTree_s markAabbTree_t;
struct markAabbTree_s {
	vec3_t		mins;		/* +0x00 */
	vec3_t		maxs;		/* +0x0C */
	msurface_t	*surfaces;	/* +0x18 */
	int		surfaceCount;	/* +0x1C */
	markAabbTree_t	*children;	/* +0x20 */
	int		childCount;	/* +0x24 */
};

typedef struct markCell_s {
	vec3_t			mins;			/* +0x00  unread here */
	vec3_t			maxs;			/* +0x0C  unread here */
	markAabbTree_t		*aabbTree;		/* +0x18 */
	void			*portals;		/* +0x1C  unread here */
	int			portalCount;		/* +0x20  unread here */
	markCullGroup_t		**cullGroups;		/* +0x24 */
	int			cullGroupCount;		/* +0x28 */
	void			**occluders;		/* +0x2C  unread here */
	int			occluderCount;		/* +0x30  unread here */
	int			markViewCount;		/* +0x34 */
	void			*modelLinks;		/* +0x38  unread here */
	void			*entityLinks;		/* +0x3C  unread here */
} markCell_t;

typedef struct mnode_s mnode_t;
struct mnode_s {
	int		unk_00[2];	/* +0x00 */
	int		cellIndex;	/* +0x08 */
	cplane_t	*plane;		/* +0x0C */
	mnode_t		*children[2];	/* +0x10 */
};

#define MNODE_INTERNAL	(-2)
#define MNODE_NO_CELL	(-1)


#define TR_WORLD_NODES	(*(mnode_t **)(tr_world + 152))
#define TR_WORLD_CELLS	(*(markCell_t **)(tr_world + 292))

static struct {
	vec3_t	normal;
	float	dist;
} tr_markProjectionPlane;


#define MAX_VERTS_ON_POLY	64	/* 0x004E9070: cmp ebp, 40h            */
#define MARK_CLIP_SIDES		2
#define MAX_MARK_SURFACES	4096	/* 0x004E9234: push 1000h              */
#define MARK_EXTRA_PLANES	2
#define MARK_SHADER_USAGE	9	/* 0x004E9363: push 9 to R_FindShader  */

#define MARK_SIDE_FRONT	0
#define MARK_SIDE_BACK	1
#define MARK_SIDE_ON	2

#define BOX_SIDE_FRONT	1
#define BOX_SIDE_BACK	2


extern char *R_FindShader( char *name, int lightmapIndex, int mipRawImage, const char *usage );
extern int   BoxOnPlaneSide( const vec3_t emins, const vec3_t emaxs, cplane_t *p );


/* ---- R_ChopPolyBehindPlane  0x004E86A0 ----  VERIFIED */
static void R_ChopPolyBehindPlane( int numInPoints, const markVert_t *inPoints,
                                   int *numOutPoints, markVert_t *outPoints,
                                   const vec3_t normal, float dist, float epsilon ) {
	float	dists[MAX_VERTS_ON_POLY + 4];
	int	sides[MAX_VERTS_ON_POLY + 4];
	int	counts[3];
	int	i, j;

	/* Q3's `MAX_VERTS_ON_POLY - 2` guard.  0x004E86B7: cmp edi, 3Eh. */
	if ( numInPoints >= MAX_VERTS_ON_POLY - 2 ) {
		*numOutPoints = 0;
		return;
	}

	counts[MARK_SIDE_FRONT] = 0;
	counts[MARK_SIDE_BACK] = 0;
	counts[MARK_SIDE_ON] = 0;

	for ( i = 0 ; i < numInPoints ; i++ ) {
		float d;

		d = inPoints[i].xyz[2] * normal[2] + inPoints[i].xyz[0] * normal[0]
		  + normal[1] * inPoints[i].xyz[1] - dist;
		dists[i] = d;
		if ( d > epsilon ) {
			sides[i] = MARK_SIDE_FRONT;
		} else if ( d < -epsilon ) {
			sides[i] = MARK_SIDE_BACK;
		} else {
			sides[i] = MARK_SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*numOutPoints = 0;

	if ( !counts[MARK_SIDE_FRONT] ) {
		return;
	}
	if ( !counts[MARK_SIDE_BACK] ) {
		*numOutPoints = numInPoints;
		/* 0x004E878A: rep movsd of 20 * numInPoints bytes. */
		memcpy( outPoints, inPoints, (unsigned int)numInPoints * sizeof( *outPoints ) );
		return;
	}

	for ( i = 0 ; i < numInPoints ; i++ ) {
		const markVert_t	*p1 = &inPoints[i];
		markVert_t		*clip = &outPoints[*numOutPoints];

		if ( sides[i] == MARK_SIDE_ON ) {
			*clip = *p1;
			( *numOutPoints )++;
			continue;
		}

		if ( sides[i] == MARK_SIDE_FRONT ) {
			*clip = *p1;
			( *numOutPoints )++;
			clip = &outPoints[*numOutPoints];
		}

		if ( sides[i + 1] == MARK_SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		{
			const markVert_t	*p2 = &inPoints[( i + 1 ) % numInPoints];
			float			d = dists[i] - dists[i + 1];
			float			frac;

			if ( d == 0.0f ) {
				frac = 0.0f;
			} else {
				frac = dists[i] / d;
			}

			for ( j = 0 ; j < 3 ; j++ ) {
				clip->xyz[j] = p1->xyz[j] + frac * ( p2->xyz[j] - p1->xyz[j] );
			}
			for ( j = 0 ; j < 2 ; j++ ) {
				clip->lightmapCoords[j] = p1->lightmapCoords[j]
					+ frac * ( p2->lightmapCoords[j] - p1->lightmapCoords[j] );
			}
			( *numOutPoints )++;
		}
	}
}


/* ---- R_AABBTreeSurfaces_r  0x004E8900 ----  VERIFIED */
static void R_AABBTreeSurfaces_r( const markAabbTree_t *tree,
                                  const vec3_t mins, const vec3_t maxs,
                                  msurface_t **surfaces, int maxSurfaces,
                                  int *numSurfaces ) {
	int	i;

	if ( tree->maxs[0] < mins[0] || tree->mins[0] > maxs[0]
	  || tree->maxs[1] < mins[1] || tree->mins[1] > maxs[1]
	  || tree->maxs[2] < mins[2] || tree->mins[2] > maxs[2] ) {
		return;
	}

	if ( tree->childCount != 0 ) {
		for ( i = 0 ; i < tree->childCount ; i++ ) {
			R_AABBTreeSurfaces_r( &tree->children[i], mins, maxs,
			                      surfaces, maxSurfaces, numSurfaces );
		}
		return;
	}

	for ( i = 0 ; i < tree->surfaceCount ; i++ ) {
		msurface_t		*surf;
		const markSurface_t	*data;

		if ( *numSurfaces >= maxSurfaces ) {
			break;
		}

		surf = &tree->surfaces[i];
		if ( ( surf->shader->surfaceParmFlags & SURF_NOMARKS ) != 0 ) {
			continue;
		}

		data = surf->data;
		if ( data->boundsMax[0] < mins[0] || data->boundsMin[0] > maxs[0]
		  || data->boundsMax[1] < mins[1] || data->boundsMin[1] > maxs[1]
		  || data->boundsMax[2] < mins[2] || data->boundsMin[2] > maxs[2] ) {
			continue;
		}

		surfaces[( *numSurfaces )++] = surf;
	}
}


/* ---- R_CellSurfaces  0x004E8A80 ----  VERIFIED */
static void R_CellSurfaces( markCell_t *cell, const vec3_t mins, const vec3_t maxs,
                            msurface_t **surfaces, int maxSurfaces,
                            int *numSurfaces ) {
	int	i, j, k;

	if ( cell->markViewCount == tr_viewCount ) {
		return;
	}
	cell->markViewCount = tr_viewCount;

	for ( i = 0 ; i < cell->cullGroupCount ; i++ ) {
		const markCullGroup_t	*group = cell->cullGroups[i];

		if ( group->maxs[0] < mins[0] || group->mins[0] > maxs[0]
		  || group->maxs[1] < mins[1] || group->mins[1] > maxs[1]
		  || group->maxs[2] < mins[2] || group->mins[2] > maxs[2] ) {
			continue;
		}

		for ( j = 0 ; j < group->surfaceCount ; j++ ) {
			msurface_t		*surf;
			const markSurface_t	*data;

			if ( *numSurfaces >= maxSurfaces ) {
				break;
			}

			surf = &group->surfaces[j];
			if ( ( surf->shader->surfaceParmFlags & SURF_NOMARKS ) != 0 ) {
				continue;
			}

			data = surf->data;
			if ( data->boundsMax[0] < mins[0] || data->boundsMin[0] > maxs[0]
			  || data->boundsMax[1] < mins[1] || data->boundsMin[1] > maxs[1]
			  || data->boundsMax[2] < mins[2] || data->boundsMin[2] > maxs[2] ) {
				continue;
			}

			for ( k = 0 ; k < *numSurfaces ; k++ ) {
				if ( surfaces[k] == surf ) {
					break;
				}
			}
			if ( k == *numSurfaces ) {
				surfaces[*numSurfaces] = surf;
				( *numSurfaces )++;
			}
		}
	}

	R_AABBTreeSurfaces_r( cell->aabbTree, mins, maxs,
	                      surfaces, maxSurfaces, numSurfaces );
}


/* ---- R_BoxSurfaces_r  0x004E8C50 ----  VERIFIED */
static void R_BoxSurfaces_r( mnode_t *node, const vec3_t mins, const vec3_t maxs,
                             msurface_t **surfaces, int maxSurfaces,
                             int *numSurfaces ) {
	while ( node->cellIndex == MNODE_INTERNAL ) {
		int	s = BoxOnPlaneSide( mins, maxs, node->plane );

		if ( s == BOX_SIDE_FRONT ) {
			node = node->children[0];
			continue;
		}
		if ( s != BOX_SIDE_BACK ) {
			R_BoxSurfaces_r( node->children[0], mins, maxs,
			                 surfaces, maxSurfaces, numSurfaces );
		}
		node = node->children[1];
	}

	if ( node->cellIndex != MNODE_NO_CELL ) {
		R_CellSurfaces( &TR_WORLD_CELLS[node->cellIndex], mins, maxs,
		                surfaces, maxSurfaces, numSurfaces );
	}
}


/* ---- R_AddMarkFragment  0x004E8CF0 ----  VERIFIED */
static qboolean R_AddMarkFragment( int numClipPoints,
                                   markVert_t clipPoints[MARK_CLIP_SIDES][MAX_VERTS_ON_POLY],
                                   int numPlanes, const vec3_t *normals, const float *dists,
                                   int maxPoints, polyVert_t *pointBuffer,
                                   markFragment_t *fragment,
                                   const vec3_t mins, const vec3_t maxs ) {
	int	pingPong = 0;
	int	i;

	(void)mins;
	(void)maxs;

	for ( i = 0 ; i < numPlanes ; i++ ) {
		R_ChopPolyBehindPlane( numClipPoints, clipPoints[pingPong],
		                       &numClipPoints, clipPoints[!pingPong],
		                       normals[i], dists[i], 0.5f );
		pingPong ^= 1;
		if ( numClipPoints == 0 ) {
			return qfalse;
		}
	}

	if ( numClipPoints > maxPoints ) {
		return qfalse;
	}

	fragment->numPoints = numClipPoints;
	for ( i = 0 ; i < numClipPoints ; i++ ) {
		const markVert_t	*src = &clipPoints[pingPong][i];
		polyVert_t		*dst = &pointBuffer[i];

		dst->xyz[0] = src->xyz[0];
		dst->xyz[1] = src->xyz[1];
		dst->xyz[2] = src->xyz[2];
		dst->lightmapCoords[0] = src->lightmapCoords[0];
		dst->lightmapCoords[1] = src->lightmapCoords[1];
	}

	return qtrue;
}


/* ---- RE_MarkFragments  0x004E8DB0 ----  VERIFIED */
int RE_MarkFragments( int numPoints, const vec3_t *points,
                      const vec3_t projectionOrigin, const vec3_t projectionAxis[3],
                      float projectionRadius, int maxPoints, polyVert_t *pointBuffer,
                      int maxFragments, markFragment_t *fragmentBuffer,
                      int hShader ) {
	msurface_t	*surfaces[MAX_MARK_SURFACES];
	markVert_t	clipPoints[MARK_CLIP_SIDES][MAX_VERTS_ON_POLY];
	vec3_t		normals[MAX_VERTS_ON_POLY + MARK_EXTRA_PLANES];
	float		dists[MAX_VERTS_ON_POLY + MARK_EXTRA_PLANES];
	vec3_t		mins, maxs;
	int		numsurfaces;
	int		numPlanes;
	int		numSourcePoints;
	int		returnedPoints;
	int		returnedFragments;
	markShader_t	*shader;
	int		i, j, k;

	tr_viewCount++;

	tr_markProjectionPlane.normal[0] = -projectionAxis[0][0];
	tr_markProjectionPlane.normal[1] = -projectionAxis[0][1];
	tr_markProjectionPlane.normal[2] = -projectionAxis[0][2];
	tr_markProjectionPlane.dist = 0.0f;

	mins[0] = mins[1] = mins[2] =  262144.0f;
	maxs[0] = maxs[1] = maxs[2] = -262144.0f;

	for ( i = 0 ; i < numPoints ; i++ ) {
		vec3_t	projected;
		vec3_t	extended;

		for ( j = 0 ; j < 3 ; j++ ) {
			if ( points[i][j] < mins[j] ) {
				mins[j] = points[i][j];
			}
			if ( points[i][j] > maxs[j] ) {
				maxs[j] = points[i][j];
			}
			projected[j] = points[i][j] + projectionRadius * projectionAxis[0][j];
			extended[j]  = points[i][j] + ( projectionRadius * -20.0f ) * projectionAxis[0][j];
		}

		for ( j = 0 ; j < 3 ; j++ ) {
			if ( projected[j] < mins[j] ) {
				mins[j] = projected[j];
			}
			if ( projected[j] > maxs[j] ) {
				maxs[j] = projected[j];
			}
			if ( extended[j] < mins[j] ) {
				mins[j] = extended[j];
			}
			if ( extended[j] > maxs[j] ) {
				maxs[j] = extended[j];
			}
		}
	}

	numSourcePoints = numPoints;
	if ( numSourcePoints > MAX_VERTS_ON_POLY ) {
		numSourcePoints = MAX_VERTS_ON_POLY;
	}

	for ( i = 0 ; i < numSourcePoints ; i++ ) {
		vec3_t	edge;

		for ( j = 0 ; j < 3 ; j++ ) {
			edge[j] = points[( i + 1 ) % numSourcePoints][j] - points[i][j];
		}

		normals[i][0] = edge[1] * projectionAxis[0][2] - edge[2] * projectionAxis[0][1];
		normals[i][1] = edge[2] * projectionAxis[0][0] - edge[0] * projectionAxis[0][2];
		normals[i][2] = edge[0] * projectionAxis[0][1] - edge[1] * projectionAxis[0][0];
		VectorNormalize( normals[i] );

		dists[i] = normals[i][1] * points[i][1]
		         + normals[i][2] * points[i][2]
		         + normals[i][0] * points[i][0];
	}

	normals[numSourcePoints][0] = projectionAxis[0][0];
	normals[numSourcePoints][1] = projectionAxis[0][1];
	normals[numSourcePoints][2] = projectionAxis[0][2];
	dists[numSourcePoints] = projectionAxis[0][2] * points[0][2]
	                       + projectionAxis[0][1] * points[0][1]
	                       + projectionAxis[0][0] * points[0][0]
	                       - projectionRadius;

	normals[numSourcePoints + 1][0] = -projectionAxis[0][0];
	normals[numSourcePoints + 1][1] = -projectionAxis[0][1];
	normals[numSourcePoints + 1][2] = -projectionAxis[0][2];
	dists[numSourcePoints + 1] = normals[numSourcePoints + 1][0] * points[0][0]
	                           + normals[numSourcePoints + 1][1] * points[0][1]
	                           + normals[numSourcePoints + 1][2] * points[0][2]
	                           - projectionRadius;

	numPlanes = numSourcePoints + MARK_EXTRA_PLANES;

	numsurfaces = 0;
	R_BoxSurfaces_r( TR_WORLD_NODES, mins, maxs,
	                 surfaces, MAX_MARK_SURFACES, &numsurfaces );

	tr_markProjectionPlane.normal[0] = -tr_markProjectionPlane.normal[0];
	tr_markProjectionPlane.normal[1] = -tr_markProjectionPlane.normal[1];
	tr_markProjectionPlane.normal[2] = -tr_markProjectionPlane.normal[2];

	returnedPoints = 0;
	returnedFragments = 0;

	if ( hShader < 0 || hShader >= tr_numShaders[0] ) {
		ri_Printf( 2, "R_GetShaderByHandle: out of range hShader '%d'\n", hShader );
		shader = (markShader_t *)tr_defaultShader;
	} else {
		shader = (markShader_t *)tr_shaders[hShader];
	}

	for ( i = 0 ; i < numsurfaces ; i++ ) {
		markSurface_t	*surf;
		markShader_t	*fragmentShader;
		vec3_t		*positions;
		float		( *lightmapCoords )[2];
		int		baseIndex;

		surf = surfaces[i]->data;

		if ( surf->surfaceType < 24 ) {
			continue;
		}

		baseIndex = surf->indices[0];
		positions = surf->positions - baseIndex;
		lightmapCoords = surf->lightmapCoords - baseIndex;

		if ( shader != (markShader_t *)tr_defaultShader
		  && surfaces[i]->shader->lightmapIndex >= 0 ) {
			fragmentShader = (markShader_t *)R_FindShader( shader->name,
			                                               surfaces[i]->shader->lightmapIndex,
			                                               1,
			                                               (const char *)MARK_SHADER_USAGE );
		} else {
			fragmentShader = shader;
		}

		for ( k = 0 ; k < surf->indexCount ; k += 3 ) {
			const unsigned short	*tri = &surf->indices[k];
			vec3_t			v1, v2, normal;
			markFragment_t		*mf;

			VectorCopy( positions[tri[0]], clipPoints[0][0].xyz );
			VectorCopy( positions[tri[1]], clipPoints[0][1].xyz );
			VectorCopy( positions[tri[2]], clipPoints[0][2].xyz );

			for ( j = 0 ; j < 3 ; j++ ) {
				v1[j] = clipPoints[0][0].xyz[j] - clipPoints[0][1].xyz[j];
				v2[j] = clipPoints[0][2].xyz[j] - clipPoints[0][1].xyz[j];
			}
			normal[0] = v2[2] * v1[1] - v2[1] * v1[2];
			normal[1] = v2[0] * v1[2] - v2[2] * v1[0];
			normal[2] = v2[1] * v1[0] - v2[0] * v1[1];
			VectorNormalize( normal );

			if ( !( normal[0] * projectionAxis[0][0]
			      + normal[2] * projectionAxis[0][2]
			      + normal[1] * projectionAxis[0][1] > 0.5f ) ) {
				continue;
			}

			clipPoints[0][0].lightmapCoords[0] = lightmapCoords[tri[0]][0];
			clipPoints[0][0].lightmapCoords[1] = lightmapCoords[tri[0]][1];
			clipPoints[0][1].lightmapCoords[0] = lightmapCoords[tri[1]][0];
			clipPoints[0][1].lightmapCoords[1] = lightmapCoords[tri[1]][1];
			clipPoints[0][2].lightmapCoords[0] = lightmapCoords[tri[2]][0];
			clipPoints[0][2].lightmapCoords[1] = lightmapCoords[tri[2]][1];

			mf = &fragmentBuffer[returnedFragments];
			if ( !R_AddMarkFragment( 3, clipPoints, numPlanes, normals, dists,
			                         maxPoints - returnedPoints,
			                         &pointBuffer[returnedPoints], mf,
			                         mins, maxs ) ) {
				continue;
			}

			if ( mf->numPoints > 0 ) {
				float	scale = 0.5f / projectionRadius;

				for ( j = 0 ; j < mf->numPoints ; j++ ) {
					polyVert_t	*out = &pointBuffer[returnedPoints + j];
					vec3_t		delta;

					delta[0] = out->xyz[0] - projectionOrigin[0];
					delta[1] = out->xyz[1] - projectionOrigin[1];
					delta[2] = out->xyz[2] - projectionOrigin[2];

					out->st[0] = ( delta[0] * projectionAxis[1][0]
					             + delta[2] * projectionAxis[1][2]
					             + delta[1] * projectionAxis[1][1] ) * scale + 0.5f;
					out->st[1] = ( delta[2] * projectionAxis[2][2]
					             + delta[1] * projectionAxis[2][1]
					             + delta[0] * projectionAxis[2][0] ) * scale + 0.5f;
				}
			}

			mf->firstPoint = returnedPoints;
			mf->shaderHandle = fragmentShader->index;
			returnedPoints += mf->numPoints;
			returnedFragments++;

			if ( returnedFragments == maxFragments
			  || returnedPoints > maxPoints - 3 ) {
				return returnedFragments;
			}
		}
	}

	return returnedFragments;
}
