/*
 * qcommon/cm_patch.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_patch.c
 *
 * Retail range 0x0041A500-0x0041DB90, 27 functions.
 *
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */
#include "../qcommon/cm_patch_local.h"
#include "../qcommon/cm_polylib.h"
#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

winding_t *BaseWindingForPlane( const vec3_t normal, vec_t dist );
winding_t *CopyWinding( const winding_t *w );
void FreeWinding( winding_t *w );
void ChopWindingInPlace( winding_t **inout, const vec3_t normal,
						 vec_t dist, vec_t epsilon );
void WindingBounds( const winding_t *w, vec3_t mins, vec3_t maxs );
void *Hunk_AllocAlignInternal( int size, int align );

typedef struct cmPatchSphere_t
{
	qboolean use;                   /* +0x0F4 */
	float radius;                   /* +0x0F8 */
	float halfheight;               /* +0x0FC */
	vec3_t offset;                  /* +0x100 */
} cmPatchSphere_t;

typedef struct cmPatchTrace_t
{
	vec3_t start;                   /* +0x000 */
	vec3_t end;                     /* +0x00C */
	byte _pad_18[0x040 - 0x018];
	vec3_t offsets[8];              /* +0x040 */
	byte _pad_A0[0x0C0 - 0x0A0];
	qboolean isPoint;               /* +0x0C0 */
	trace_t trace;                  /* +0x0C4  fraction +0xC4, normal +0xD4 */
	cmPatchSphere_t sphere;         /* +0x0F4 */
	vec3_t sphereExtents;           /* +0x10C */
} cmPatchTrace_t;
COD1_ASSERT_SIZE( cmPatchTrace_t, 280 );

#define CM_PLAYER_CURVE_CLIP()  ( ( const cvar_t * )(size_t) cm_playerCurveClip )

#define MAX_GRID_SIZE   129

#define SURFACE_CLIP_EPSILON    0.125f

#define PLANE_TRI_EPSILON       0.1f

#define MAX_FACETS              1024

#define CM_MAX_MAP_BOUNDS       131072.0f

#define CM_BOUNDS_SEED          262144.0f

#define CM_SIDE_FRONT           0
#define CM_SIDE_BACK            1
#define CM_SIDE_ON              2

#define CM_FACET_LOWER          0
#define CM_FACET_UPPER          1
#define CM_FACET_QUAD           ( -1 )

/* CM_EdgePlaneNum's edge names, read off retail's jump table at 0x0041B250. */
#define CM_EDGE_BOTTOM          0
#define CM_EDGE_RIGHT           1
#define CM_EDGE_TOP             2
#define CM_EDGE_LEFT            3
#define CM_EDGE_DIAG_DESC       4
#define CM_EDGE_DIAG_ASC        5

#define CM_EDGE_PLANE_PUSH      4.0f

#define CM_BEVEL_MIN_LENGTH     0.5f

#define CM_DEFAULT_MAX_ERROR    16

typedef struct cGrid_t
{
	int width;                                      /* +0x00 */
	int height;                                     /* +0x04 */
	qboolean wrapWidth;                             /* +0x08 */
	qboolean wrapHeight;                            /* +0x0C */
	vec3_t points[MAX_GRID_SIZE][MAX_GRID_SIZE];    /* +0x10 */
} cGrid_t;

patchPlane_t cm_patchPlanes[MAX_PATCH_PLANES];
int cm_numPatchPlanes;

static cGrid_t cm_patchGrid;
static int cm_patchGridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2];
static facet_t cm_patchFacets[MAX_FACETS];

int CM_FindPlane2( const vec4_t plane, int *flipped );
int CM_FindPlane( const vec3_t p1, const vec3_t p2, const vec3_t p3 );
int CM_PointOnPlaneSide( const vec3_t p, int planeNum );
int CM_GridPlane( int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
				  int i, int j, int tri );
int CM_EdgePlaneNum( const cGrid_t *grid,
					 int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
					 int i, int j, int k );
void CM_SetBorderInward( facet_t *facet, const cGrid_t *grid,
						 int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
						 int i, int j, int which );
qboolean CM_ValidateFacet( const facet_t *facet );
void CM_AddFacetBevels( facet_t *facet );
void CM_PatchCollideFromGrid( const cGrid_t *grid, patchCollide_t *pf );
qboolean CM_PlaneEqual( float *p, float *plane, int *flipped );
int CM_SignbitsForNormal( const vec3_t normal );
qboolean CM_PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b,
							 const vec3_t c );
void CM_SnapVector( vec3_t normal );
qboolean CM_NeedsSubdivision( const vec3_t a, const vec3_t b, const vec3_t c,
							  int maxError );
void CM_Subdivide( const vec3_t a, const vec3_t b, const vec3_t c,
				   vec3_t out1, vec3_t out2, vec3_t out3 );
void CM_TransposeGrid( cGrid_t *grid );
void CM_SetGridWrapWidth( cGrid_t *grid );
void CM_SubdivideGridColumns( cGrid_t *grid, int maxError );
void CM_RemoveDegenerateColumns( cGrid_t *grid );
void CM_TracePointThroughPatchCollide( traceWork_t *tw,
									   const patchCollide_t *pc );
qboolean CM_SightTracePointThroughPatchCollide( traceWork_t *tw,
												const patchCollide_t *pc );
int CM_CheckFacetPlane( const float *plane, const vec3_t start,
						const vec3_t end, float *enterFrac, float *leaveFrac,
						int *hit );

/* ---- CM_SignbitsForNormal  0x0041A500 ---- VERIFIED */
/* ---- VERIFIED  0x0041A500 ---- */
int __cdecl CM_SignbitsForNormal(const vec3_t normal)
{
  float *v1 = (float *)normal;   /* edx in retail; the only argument */
  int v2;

  v2 = ((*v1 < 0.0) | __UNORDERED__(*v1, 0.0)) != 0;
  if ( (v1[1] < 0.0) | __UNORDERED__(v1[1], 0.0) )
    v2 |= 2u;
  if ( (v1[2] < 0.0) | __UNORDERED__(v1[2], 0.0) )
    return v2 | 4;
  return v2;
}

/* ---- CM_PlaneFromPoints  0x0041A540 ---- VERIFIED */
qboolean CM_PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b,
							 const vec3_t c )
{
	vec3_t d1, d2;

	VectorSubtract( b, a, d1 );
	VectorSubtract( c, a, d2 );
	CrossProduct( d2, d1, plane );

	if ( VectorNormalize( plane ) == 0.0f ) {
		return qfalse;
	}

	plane[3] = DotProduct( a, plane );
	return qtrue;
}

/* ---- CM_NeedsSubdivision  0x0041A600 ---- VERIFIED */
qboolean CM_NeedsSubdivision(const vec3_t a, const vec3_t b, const vec3_t c,
                             int maxError)
{
  float dx, dy, dz;
  double deviation;

  dx = a[0] + c[0] - ( b[0] + b[0] );
  dy = a[1] + c[1] - ( b[1] + b[1] );
  dz = a[2] + c[2] - ( b[2] + b[2] );

  deviation = sqrt( dx * dx + dy * dy + dz * dz ) * 0.25;

  return ( (double)maxError < deviation );
}

/* ---- CM_Subdivide  0x0041A660 ---- VERIFIED */
void CM_Subdivide(const vec3_t a, const vec3_t b, const vec3_t c,
                  vec3_t out1, vec3_t out2, vec3_t out3)
{
  int i;

  for ( i = 0; i < 3; i++ ) {
    out1[i] = ( a[i] + b[i] ) * 0.5f;
    out3[i] = ( b[i] + c[i] ) * 0.5f;
    out2[i] = ( out1[i] + out3[i] ) * 0.5f;
  }
}

/* ---- CM_TransposeGrid  0x0041A6E0 ---- VERIFIED */
void CM_TransposeGrid(cGrid_t *grid)
{
  int *v1 = (int *)grid;   /* eax in retail; the only argument */
  int v2;
  int v3;
  int v4;
  int v5;
  float *v6;
  float *v7;
  double v8;
  double v9;
  int v10;
  int v11;
  int v12;
  float *v13;
  float *v14;
  double v15;
  double v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;

  v2 = v1[1];
  if ( *v1 <= v2 )
  {
    if ( *v1 > 0 )
    {
      v10 = 0;
      v11 = 1;
      do
      {
        v12 = v11;
        if ( v11 < v1[1] )
        {
          v13 = (float *)&v1[3 * v11 + 5 + 3 * v10];
          v14 = (float *)&v1[3 * v11 + 389 + 3 * v10];
          do
          {
            if ( v12 >= *v1 )
            {
              *(v14 - 1) = *(v13 - 1);
              *v14 = *v13;
              v14[1] = v13[1];
            }
            else
            {
              v21 = *((_DWORD *)v14 + 1);
              v15 = *(v14 - 1);
              v16 = *v14;
              *(v14 - 1) = *(v13 - 1);
              *v14 = *v13;
              v14[1] = v13[1];
              *(v13 - 1) = v15;
              *v13 = v16;
              *((_DWORD *)v13 + 1) = v21;
            }
            ++v12;
            v13 += 3;
            v14 += 387;
          }
          while ( v12 < v1[1] );
        }
        ++v11;
        v10 += 129;
      }
      while ( v11 - 1 < *v1 );
    }
  }
  else if ( v2 > 0 )
  {
    v3 = 0;
    v4 = 1;
    do
    {
      v5 = v4;
      if ( v4 < *v1 )
      {
        v6 = (float *)&v1[3 * v4 + 5 + 3 * v3];
        v7 = (float *)&v1[3 * v4 + 389 + 3 * v3];
        do
        {
          if ( v5 >= v1[1] )
          {
            *(v6 - 1) = *(v7 - 1);
            *v6 = *v7;
            v6[1] = v7[1];
          }
          else
          {
            v20 = *((_DWORD *)v6 + 1);
            v8 = *(v6 - 1);
            v9 = *v6;
            *(v6 - 1) = *(v7 - 1);
            *v6 = *v7;
            v6[1] = v7[1];
            *(v7 - 1) = v8;
            *v7 = v9;
            *((_DWORD *)v7 + 1) = v20;
          }
          ++v5;
          v6 += 3;
          v7 += 387;
        }
        while ( v5 < *v1 );
      }
      ++v4;
      v3 += 129;
    }
    while ( v4 - 1 < v1[1] );
  }
  v17 = *v1;
  *v1 = v1[1];
  v18 = v1[3];
  v1[1] = v17;
  v19 = v1[2];
  v1[2] = v18;
  v1[3] = v19;
}

/* ---- CM_SetGridWrapWidth  0x0041A840 ---- VERIFIED */
void CM_SetGridWrapWidth(cGrid_t *grid)
{
  _DWORD *v1 = (_DWORD *)grid;   /* edx in retail; the only argument */
  int v2;
  int v3;
  int v4;
  double v5;
  double v6;
  double v7;
  float *v8;
  float *v9;
  int v10;
  float *v11;
  int v12;

  v2 = 0;
  if ( (int)v1[1] > 0 )
  {
    v12 = 387 * *v1;
    v11 = (float *)(v1 + 4);
    v3 = 0;
    v10 = (int)&v1[387 * *v1 - 383];
    do
    {
      v9 = (float *)v10;
      v4 = 0;
      v8 = v11;
      do
      {
        v5 = *v8 - *v9;
        if ( (v5 < -0.1) | __UNORDERED__(v5, -0.1) || v5 > 0.1 )
          break;
        v6 = *(float *)&v1[v3 + 5 + v4] - *(float *)&v1[v4 - 382 + v12];
        if ( (v6 < -0.1) | __UNORDERED__(v6, -0.1) )
        {
          ++v4;
          break;
        }
        if ( v6 > 0.1 )
        {
          ++v4;
          break;
        }
        v7 = *(float *)&v1[v3 + 6 + v4] - *(float *)&v1[258 * *v1 - 381 + 129 * *v1 + 2 * v2 + v4 + v2];
        if ( (v7 < -0.1) | __UNORDERED__(v7, -0.1) )
        {
          v4 += 2;
          break;
        }
        if ( v7 > 0.1 )
        {
          v4 += 2;
          break;
        }
        v4 += 3;
        v8 += 3;
        v9 += 3;
      }
      while ( v4 < 3 );
      if ( v4 != 3 )
        break;
      v3 += 3;
      v10 += 12;
      ++v2;
      v11 += 3;
      v12 += 3;
    }
    while ( v2 < (int)v1[1] );
  }
  v1[2] = ( v2 == (int)v1[1] );   /* 0x0041A997 `cmp edi,eax / setz cl` */
}

/* ---- CM_SubdivideGridColumns  0x0041A9C0 ---- VERIFIED */
void CM_SubdivideGridColumns( cGrid_t *grid, int maxError )
{
	int i, j, k;
	vec3_t prev, mid, next;

	for ( i = 0; i < grid->width - 2; ) {
		for ( j = 0; j < grid->height; j++ ) {
			if ( CM_NeedsSubdivision( grid->points[i][j], grid->points[i + 1][j],
									  grid->points[i + 2][j], maxError ) ) {
				break;
			}
		}

		if ( j == grid->height ) {
			i += 2;
			continue;
		}

		if ( grid->width + 2 > MAX_GRID_SIZE ) {
			break;
		}

		for ( j = 0; j < grid->height; j++ ) {
			VectorCopy( grid->points[i][j], prev );
			VectorCopy( grid->points[i + 1][j], mid );
			VectorCopy( grid->points[i + 2][j], next );

			for ( k = grid->width - 1; k > i + 1; k-- ) {
				VectorCopy( grid->points[k][j], grid->points[k + 2][j] );
			}

			CM_Subdivide( prev, mid, next, grid->points[i + 1][j],
						  grid->points[i + 2][j], grid->points[i + 3][j] );
		}

		grid->width += 2;

	}
}

/* ---- CM_ComparePoints  0x0041AC30 ---- VERIFIED */
/* ---- VERIFIED  0x0041AC30 ---- */
qboolean __cdecl CM_ComparePoints(const vec3_t a, const vec3_t b)
{
  float *v2 = (float *)a;   /* edx in retail */
  float *v3 = (float *)b;   /* ecx in retail; both arrive in registers,
                             and the |da-db| <= 0.1 test is symmetric */
  double v4;
  double v5;
  double v6;
  qboolean result;

  v4 = *v2 - *v3;
  result = qfalse;
  if ( !((v4 < -0.1) | __UNORDERED__(v4, -0.1)) && v4 <= 0.1 )
  {
    v5 = v2[1] - v3[1];
    if ( !((v5 < -0.1) | __UNORDERED__(v5, -0.1)) && v5 <= 0.1 )
    {
      v6 = v2[2] - v3[2];
      if ( !((v6 < -0.1) | __UNORDERED__(v6, -0.1)) && v6 <= 0.1 )
        return 1;
    }
  }
  return result;
}

/* ---- CM_RemoveDegenerateColumns  0x0041ACA0 ---- VERIFIED */
void CM_RemoveDegenerateColumns(cGrid_t *grid)
{
  int *v1 = (int *)grid;   /* ecx in retail; the only argument */
  int v2;
  float *v3;
  int v4;
  int v5;
  float *v6;
  double v7;
  double v8;
  double v9;
  int v10;
  int v11;
  int v12;
  int *v13;
  float *v14;

  v2 = 0;
  if ( *v1 - 1 > 0 )
  {
    v3 = (float *)(v1 + 392);
    v14 = (float *)(v1 + 392);
    do
    {
      v4 = v1[1];
      v5 = 0;
      if ( v4 > 0 )
      {
        v6 = v3;
        do
        {
          v7 = *(v6 - 388) - *(v6 - 1);
          if ( (v7 < -0.1) | __UNORDERED__(v7, -0.1) )
            break;
          if ( v7 > 0.1 )
            break;
          v8 = *(v6 - 387) - *v6;
          if ( (v8 < -0.1) | __UNORDERED__(v8, -0.1) )
            break;
          if ( v8 > 0.1 )
            break;
          v9 = *(v6 - 386) - v6[1];
          if ( (v9 < -0.1) | __UNORDERED__(v9, -0.1) )
            break;
          if ( v9 > 0.1 )
            break;
          ++v5;
          v6 += 3;
        }
        while ( v5 < v1[1] );
      }
      if ( v5 == v4 )
      {
        v10 = 0;
        if ( v4 > 0 )
        {
          v11 = v2 + 2;
          do
          {
            v12 = v2 + 2;
            if ( v11 < *v1 )
            {
              v13 = &v1[387 * v11 - 383 + 3 * v10];
              do
              {
                *v13 = v13[387];
                v13[1] = v13[388];
                v13[2] = v13[389];
                ++v12;
                v13 += 387;
              }
              while ( v12 < *v1 );
            }
            ++v10;
          }
          while ( v10 < v1[1] );
          v3 = v14;
        }
        --v2;
        --*v1;
        v3 -= 387;
      }
      v3 += 387;
      ++v2;
      v14 = v3;
    }
    while ( v2 < *v1 - 1 );
  }
}

/* ---- CM_PlaneEqual  0x0041ADD0 ---- VERIFIED */
/* ---- VERIFIED  0x0041ADD0 ---- */
qboolean __cdecl CM_PlaneEqual(float *p, float *plane, int *flipped)
{
  double v3;
  double v4;
  double v5;
  double v6;
  double v8;
  double v9;
  double v10;
  double v11;
  float v12;
  float v13;
  float v14;

  v3 = fabs(*p - *plane);
  if ( (v3 < 0.0001) | __UNORDERED__(v3, 0.0001)
    && (v4 = fabs(p[1] - plane[1]), (v4 < 0.0001) | __UNORDERED__(v4, 0.0001))
    && (v5 = fabs(p[2] - plane[2]), (v5 < 0.0001) | __UNORDERED__(v5, 0.0001))
    && (v6 = fabs(p[3] - plane[3]), (v6 < 0.02) | __UNORDERED__(v6, 0.02)) )
  {
    *flipped = 0;
    return 1;
  }
  else
  {
    v8 = fabs(*p - -*plane);
    if ( (v8 < 0.0001) | __UNORDERED__(v8, 0.0001)
      && (v12 = -plane[1], v9 = fabs(p[1] - v12), (v9 < 0.0001) | __UNORDERED__(v9, 0.0001))
      && (v13 = -plane[2], v10 = fabs(p[2] - v13), (v10 < 0.0001) | __UNORDERED__(v10, 0.0001))
      && (v14 = -plane[3], v11 = fabs(p[3] - v14), (v11 < 0.02) | __UNORDERED__(v11, 0.02)) )
    {
      *flipped = 1;
      return 1;
    }
    else
    {
      return 0;
    }
  }
}

/* ---- CM_SnapVector  0x0041AEC0 ---- VERIFIED */
/* ---- VERIFIED  0x0041AEC0 ---- */
void __cdecl CM_SnapVector(vec3_t normal)
{
  int v1;
  double v2;
  double v3;

  v1 = 0;
  while ( 1 )
  {
    v2 = fabs(normal[v1] - 1.0);
    if ( (v2 < 0.0001) | __UNORDERED__(v2, 0.0001) )
    {
      normal[2] = 0.0;
      normal[1] = 0.0;
      *normal = 0.0;
      normal[v1] = 1.0;
      return;
    }
    v3 = fabs(normal[v1] - -1.0);
    if ( (v3 < 0.0001) | __UNORDERED__(v3, 0.0001) )
      break;
    if ( ++v1 >= 3 )
      return;
  }
  normal[2] = 0.0;
  normal[1] = 0.0;
  *normal = 0.0;
  normal[v1] = -1.0;
}

/* ---- CM_FindPlane2  0x0041AF20 ---- VERIFIED */
int CM_FindPlane2( const vec4_t plane, int *flipped )
{
	int i;

	for ( i = 0; i < cm_numPatchPlanes; i++ ) {
		if ( CM_PlaneEqual( cm_patchPlanes[i].plane, ( float * ) plane,
							flipped ) ) {
			return i;
		}
	}

	if ( cm_numPatchPlanes == MAX_PATCH_PLANES ) {
		Com_Error( ERR_DROP, "\x15" "MAX_PATCH_PLANES" );
	}

	cm_patchPlanes[cm_numPatchPlanes].plane[0] = plane[0];
	cm_patchPlanes[cm_numPatchPlanes].plane[1] = plane[1];
	cm_patchPlanes[cm_numPatchPlanes].plane[2] = plane[2];
	cm_patchPlanes[cm_numPatchPlanes].plane[3] = plane[3];
	cm_patchPlanes[cm_numPatchPlanes].signbits = CM_SignbitsForNormal( plane );

	*flipped = qfalse;
	return cm_numPatchPlanes++;
}

/* ---- CM_FindPlane  0x0041B000 ---- VERIFIED */
int CM_FindPlane( const vec3_t p1, const vec3_t p2, const vec3_t p3 )
{
	const float *pts[3];
	vec4_t plane;
	float d;
	int i, j;

	if ( !CM_PlaneFromPoints( plane, p1, p2, p3 ) ) {
		return -1;
	}

	pts[0] = p1;
	pts[1] = p2;
	pts[2] = p3;

	for ( i = 0; i < cm_numPatchPlanes; i++ ) {
		if ( DotProduct( plane, cm_patchPlanes[i].plane ) < 0.0f ) {
			continue;
		}

		for ( j = 0; j < 3; j++ ) {
			d = DotProduct( pts[j], cm_patchPlanes[i].plane )
				- cm_patchPlanes[i].plane[3];
			if ( d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON ) {
				break;
			}
		}
		if ( j == 3 ) {
			return i;
		}
	}

	if ( cm_numPatchPlanes == MAX_PATCH_PLANES ) {
		Com_Error( ERR_DROP, "\x15" "MAX_PATCH_PLANES" );
	}

	cm_patchPlanes[cm_numPatchPlanes].plane[0] = plane[0];
	cm_patchPlanes[cm_numPatchPlanes].plane[1] = plane[1];
	cm_patchPlanes[cm_numPatchPlanes].plane[2] = plane[2];
	cm_patchPlanes[cm_numPatchPlanes].plane[3] = plane[3];
	cm_patchPlanes[cm_numPatchPlanes].signbits = CM_SignbitsForNormal( plane );

	return cm_numPatchPlanes++;
}

/* ---- CM_PointOnPlaneSide  0x0041B1C0 ---- VERIFIED */
int CM_PointOnPlaneSide( const vec3_t p, int planeNum )
{
	const float *plane;
	float d;

	if ( planeNum == -1 ) {
		return CM_SIDE_ON;
	}

	plane = cm_patchPlanes[planeNum].plane;
	d = DotProduct( p, plane ) - plane[3];

	if ( d > PLANE_TRI_EPSILON ) {
		return CM_SIDE_FRONT;
	}
	if ( d < -PLANE_TRI_EPSILON ) {
		return CM_SIDE_BACK;
	}
	return CM_SIDE_ON;
}

/* ---- CM_GridPlane  0x0041B220 ---- VERIFIED */
int CM_GridPlane(int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
                 int i, int j, int tri)
{
  int p;

  p = gridPlanes[i][j][tri];
  if ( p != -1 ) {
    return p;
  }

  return gridPlanes[i][j][!tri];
}

/* ---- CM_EdgePlaneNum  0x0041B250 ---- VERIFIED */
int CM_EdgePlaneNum( const cGrid_t *grid,
					 int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
					 int i, int j, int k )
{
	const float *p1, *p2;
	vec3_t up;
	int p;

	switch ( k ) {
	case CM_EDGE_BOTTOM:
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		if ( p < 0 ) {
			return -1;
		}
		p1 = grid->points[i][j];
		p2 = grid->points[i + 1][j];
		VectorMA( p1, CM_EDGE_PLANE_PUSH, cm_patchPlanes[p].plane, up );
		return CM_FindPlane( p1, p2, up );

	case CM_EDGE_RIGHT:
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		if ( p < 0 ) {
			return -1;
		}
		p1 = grid->points[i + 1][j];
		p2 = grid->points[i + 1][j + 1];
		VectorMA( p1, CM_EDGE_PLANE_PUSH, cm_patchPlanes[p].plane, up );
		return CM_FindPlane( p1, p2, up );

	case CM_EDGE_TOP:
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		if ( p < 0 ) {
			return -1;
		}
		p1 = grid->points[i][j + 1];
		p2 = grid->points[i + 1][j + 1];
		VectorMA( p1, CM_EDGE_PLANE_PUSH, cm_patchPlanes[p].plane, up );
		return CM_FindPlane( p2, p1, up );

	case CM_EDGE_LEFT:
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		if ( p < 0 ) {
			return -1;
		}
		p1 = grid->points[i][j];
		p2 = grid->points[i][j + 1];
		VectorMA( p1, CM_EDGE_PLANE_PUSH, cm_patchPlanes[p].plane, up );
		return CM_FindPlane( p2, p1, up );

	case CM_EDGE_DIAG_DESC:
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		if ( p < 0 ) {
			return -1;
		}
		p1 = grid->points[i + 1][j + 1];
		p2 = grid->points[i][j];
		VectorMA( p1, CM_EDGE_PLANE_PUSH, cm_patchPlanes[p].plane, up );
		return CM_FindPlane( p1, p2, up );

	case CM_EDGE_DIAG_ASC:
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		if ( p < 0 ) {
			return -1;
		}
		p1 = grid->points[i][j];
		p2 = grid->points[i + 1][j + 1];
		VectorMA( p1, CM_EDGE_PLANE_PUSH, cm_patchPlanes[p].plane, up );
		return CM_FindPlane( p1, p2, up );
	}

	Com_Error( ERR_DROP, "\x15" "CM_EdgePlaneNum: bad edge name" );
	return -1;
}

/* ---- CM_SetBorderInward  0x0041B4D0 ---- VERIFIED */
void CM_SetBorderInward( facet_t *facet, const cGrid_t *grid,
						 int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
						 int i, int j, int which )
{
	const float *points[4];
	int numPoints;
	int k, l, side;
	int front, back;

	( void ) gridPlanes;

	switch ( which ) {
	case CM_FACET_LOWER:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i + 1][j];
		points[2] = grid->points[i + 1][j + 1];
		numPoints = 3;
		break;

	case CM_FACET_UPPER:
		points[0] = grid->points[i + 1][j + 1];
		points[1] = grid->points[i][j + 1];
		points[2] = grid->points[i][j];
		numPoints = 3;
		break;

	case CM_FACET_QUAD:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i + 1][j];
		points[2] = grid->points[i + 1][j + 1];
		points[3] = grid->points[i][j + 1];
		numPoints = 4;
		break;

	default:
		Com_Error( ERR_FATAL, "\x15" "CM_SetBorderInward: bad parameter" );
		return;
	}

	for ( k = 0; k < facet->numBorders; k++ ) {
		front = 0;
		back = 0;

		for ( l = 0; l < numPoints; l++ ) {
			side = CM_PointOnPlaneSide( points[l], facet->borderPlanes[k] );
			if ( side == CM_SIDE_FRONT ) {
				front++;
			} else if ( side == CM_SIDE_BACK ) {
				back++;
			}
		}

		if ( front && !back ) {
			facet->borderInward[k] = qtrue;
		} else if ( back && !front ) {
			facet->borderInward[k] = qfalse;
		} else if ( !front && !back ) {
			facet->borderPlanes[k] = -1;
		} else {
			Com_DPrintf( "WARNING: CM_SetBorderInward: mixed plane sides\n" );
			facet->borderInward[k] = qfalse;
		}
	}
}

/* CM_FacetPlane  VERIFIED */
static void CM_FacetPlane( int planeNum, qboolean inward, vec4_t plane )
{
	plane[0] = cm_patchPlanes[planeNum].plane[0];
	plane[1] = cm_patchPlanes[planeNum].plane[1];
	plane[2] = cm_patchPlanes[planeNum].plane[2];
	plane[3] = cm_patchPlanes[planeNum].plane[3];

	if ( !inward ) {
		plane[0] = -plane[0];
		plane[1] = -plane[1];
		plane[2] = -plane[2];
		plane[3] = -plane[3];
	}
}

/* ---- CM_ValidateFacet  0x0041B820 ---- VERIFIED */
qboolean CM_ValidateFacet( const facet_t *facet )
{
	vec4_t plane;
	vec3_t bounds[2];
	winding_t *w;
	int j;

	if ( facet->surfacePlane == -1 ) {
		return qfalse;
	}

	CM_FacetPlane( facet->surfacePlane, qtrue, plane );
	w = BaseWindingForPlane( plane, plane[3] );

	for ( j = 0; j < facet->numBorders && w; j++ ) {
		if ( facet->borderPlanes[j] == -1 ) {
			FreeWinding( w );
			return qfalse;
		}
		CM_FacetPlane( facet->borderPlanes[j], facet->borderInward[j], plane );
		ChopWindingInPlace( &w, plane, plane[3], PLANE_TRI_EPSILON );
	}

	if ( !w ) {
		return qfalse;
	}

	WindingBounds( w, bounds[0], bounds[1] );
	FreeWinding( w );

	for ( j = 0; j < 3; j++ ) {
		if ( bounds[1][j] - bounds[0][j] > CM_MAX_MAP_BOUNDS ) {
			return qfalse;
		}
		if ( bounds[0][j] >= CM_MAX_MAP_BOUNDS ) {
			return qfalse;
		}
		if ( bounds[1][j] <= -CM_MAX_MAP_BOUNDS ) {
			return qfalse;
		}
	}

	return qtrue;
}

/* ---- CM_AddFacetBevels  0x0041BA10 ---- VERIFIED */
void CM_AddFacetBevels( facet_t *facet )
{
	vec4_t plane, newplane;
	vec3_t mins, maxs, vec, vec2;
	winding_t *w, *w2;
	qboolean flipped, hasBack;
	float d;
	int i, j, k, l, next;
	int axis, dir;

	CM_FacetPlane( facet->surfacePlane, qtrue, plane );
	w = BaseWindingForPlane( plane, plane[3] );

	for ( j = 0; j < facet->numBorders && w; j++ ) {
		if ( facet->borderPlanes[j] == facet->surfacePlane ) {
			continue;
		}
		CM_FacetPlane( facet->borderPlanes[j], facet->borderInward[j], plane );
		ChopWindingInPlace( &w, plane, plane[3], PLANE_TRI_EPSILON );
	}

	if ( !w ) {
		return;
	}

	WindingBounds( w, mins, maxs );

	for ( axis = 0; axis < 3; axis++ ) {
		for ( dir = -1; dir <= 1; dir += 2 ) {
			VectorClear( plane );
			plane[axis] = ( float ) dir;
			if ( dir == 1 ) {
				plane[3] = maxs[axis];
			} else {
				plane[3] = -mins[axis];
			}

			if ( CM_PlaneEqual( cm_patchPlanes[facet->surfacePlane].plane,
								plane, &flipped ) ) {
				continue;
			}

			for ( i = 0; i < facet->numBorders; i++ ) {
				if ( CM_PlaneEqual( cm_patchPlanes[facet->borderPlanes[i]].plane,
									plane, &flipped ) ) {
					break;
				}
			}
			if ( i != facet->numBorders ) {
				continue;
			}

			if ( facet->numBorders > MAX_FACET_BORDERS ) {
				Com_Printf( "ERROR: too many bevels\n" );
			}
			facet->borderPlanes[facet->numBorders] =
				CM_FindPlane2( plane, &flipped );
			facet->borderNoAdjust[facet->numBorders] = qfalse;
			facet->borderInward[facet->numBorders] = flipped;
			facet->numBorders++;
		}
	}

	for ( j = 0; j < w->numpoints; j++ ) {
		next = ( j + 1 ) % w->numpoints;
		VectorSubtract( w->p[j], w->p[next], vec );

		if ( VectorNormalize( vec ) < CM_BEVEL_MIN_LENGTH ) {
			continue;
		}

		CM_SnapVector( vec );

		for ( k = 0; k < 3; k++ ) {
			if ( vec[k] == -1.0f || vec[k] == 1.0f ) {
				break;
			}
		}
		if ( k < 3 ) {
			continue;
		}

		for ( axis = 0; axis < 3; axis++ ) {
			for ( dir = -1; dir <= 1; dir += 2 ) {
				VectorClear( vec2 );
				vec2[axis] = ( float ) dir;
				CrossProduct( vec, vec2, plane );
				if ( VectorNormalize( plane ) < CM_BEVEL_MIN_LENGTH ) {
					continue;
				}
				plane[3] = DotProduct( w->p[j], plane );

				hasBack = qfalse;
				for ( l = 0; l < w->numpoints; l++ ) {
					d = DotProduct( w->p[l], plane ) - plane[3];
					if ( d > PLANE_TRI_EPSILON ) {
						break;
					}
					if ( d < -PLANE_TRI_EPSILON ) {
						hasBack = qtrue;
					}
				}
				if ( l != w->numpoints || !hasBack ) {
					continue;
				}

				for ( i = 0; i < facet->numBorders; i++ ) {
					if ( CM_PlaneEqual(
							 cm_patchPlanes[facet->borderPlanes[i]].plane,
							 plane, &flipped ) ) {
						break;
					}
				}
				if ( i != facet->numBorders ) {
					continue;
				}

				if ( facet->numBorders > MAX_FACET_BORDERS ) {
					Com_Printf( "ERROR: too many bevels\n" );
				}
				facet->borderPlanes[facet->numBorders] =
					CM_FindPlane2( plane, &flipped );

				for ( i = 0; i < facet->numBorders; i++ ) {
					if ( facet->borderPlanes[facet->numBorders]
						 == facet->borderPlanes[i] ) {
						Com_Printf( "WARNING: bevel plane already used\n" );
					}
				}

				facet->borderNoAdjust[facet->numBorders] = qfalse;
				facet->borderInward[facet->numBorders] = flipped;

				w2 = CopyWinding( w );
				CM_FacetPlane( facet->borderPlanes[facet->numBorders],
							   facet->borderInward[facet->numBorders],
							   newplane );
				ChopWindingInPlace( &w2, newplane, newplane[3],
									PLANE_TRI_EPSILON );
				if ( !w2 ) {
					Com_DPrintf(
						"WARNING: CM_AddFacetBevels... invalid bevel\n" );
					continue;
				}
				FreeWinding( w2 );
				facet->numBorders++;
			}
		}
	}

	FreeWinding( w );

	facet->borderPlanes[facet->numBorders] = facet->surfacePlane;
	facet->borderNoAdjust[facet->numBorders] = qfalse;
	facet->borderInward[facet->numBorders] = qtrue;
	facet->numBorders++;
}

/* ---- CM_PatchCollideFromGrid  0x0041C120 ---- VERIFIED */
void CM_PatchCollideFromGrid( const cGrid_t *grid, patchCollide_t *pf )
{
	const float *p1, *p2, *p3;
	facet_t *facet;
	int borders[4];
	qboolean noAdjust[4];
	int numFacets;
	int i, j, size;

	cm_numPatchPlanes = 0;
	numFacets = 0;

	for ( i = 0; i < grid->width - 1; i++ ) {
		for ( j = 0; j < grid->height - 1; j++ ) {
			p1 = grid->points[i][j];
			p2 = grid->points[i + 1][j];
			p3 = grid->points[i + 1][j + 1];
			cm_patchGridPlanes[i][j][0] = CM_FindPlane( p1, p2, p3 );

			p1 = grid->points[i + 1][j + 1];
			p2 = grid->points[i][j + 1];
			p3 = grid->points[i][j];
			cm_patchGridPlanes[i][j][1] = CM_FindPlane( p1, p2, p3 );
		}
	}

	for ( i = 0; i < grid->width - 1; i++ ) {
		for ( j = 0; j < grid->height - 1; j++ ) {

			borders[CM_EDGE_BOTTOM] = -1;
			if ( j > 0 ) {
				borders[CM_EDGE_BOTTOM] = cm_patchGridPlanes[i][j - 1][1];
			} else if ( grid->wrapHeight ) {
				borders[CM_EDGE_BOTTOM] =
					cm_patchGridPlanes[i][grid->height - 2][1];
			}
			noAdjust[CM_EDGE_BOTTOM] =
				( borders[CM_EDGE_BOTTOM] == cm_patchGridPlanes[i][j][0] );
			if ( borders[CM_EDGE_BOTTOM] == -1 || noAdjust[CM_EDGE_BOTTOM] ) {
				borders[CM_EDGE_BOTTOM] =
					CM_EdgePlaneNum( grid, cm_patchGridPlanes, i, j,
									 CM_EDGE_BOTTOM );
			}

			borders[CM_EDGE_TOP] = -1;
			if ( j < grid->height - 2 ) {
				borders[CM_EDGE_TOP] = cm_patchGridPlanes[i][j + 1][0];
			} else if ( grid->wrapHeight ) {
				borders[CM_EDGE_TOP] = cm_patchGridPlanes[i][0][0];
			}
			noAdjust[CM_EDGE_TOP] =
				( borders[CM_EDGE_TOP] == cm_patchGridPlanes[i][j][1] );
			if ( borders[CM_EDGE_TOP] == -1 || noAdjust[CM_EDGE_TOP] ) {
				borders[CM_EDGE_TOP] =
					CM_EdgePlaneNum( grid, cm_patchGridPlanes, i, j,
									 CM_EDGE_TOP );
			}

			borders[CM_EDGE_LEFT] = -1;
			if ( i > 0 ) {
				borders[CM_EDGE_LEFT] = cm_patchGridPlanes[i - 1][j][0];
			} else if ( grid->wrapWidth ) {
				borders[CM_EDGE_LEFT] =
					cm_patchGridPlanes[grid->width - 2][j][0];
			}
			noAdjust[CM_EDGE_LEFT] =
				( borders[CM_EDGE_LEFT] == cm_patchGridPlanes[i][j][1] );
			if ( borders[CM_EDGE_LEFT] == -1 || noAdjust[CM_EDGE_LEFT] ) {
				borders[CM_EDGE_LEFT] =
					CM_EdgePlaneNum( grid, cm_patchGridPlanes, i, j,
									 CM_EDGE_LEFT );
			}

			borders[CM_EDGE_RIGHT] = -1;
			if ( i < grid->width - 2 ) {
				borders[CM_EDGE_RIGHT] = cm_patchGridPlanes[i + 1][j][1];
			} else if ( grid->wrapWidth ) {
				borders[CM_EDGE_RIGHT] = cm_patchGridPlanes[0][j][1];
			}
			noAdjust[CM_EDGE_RIGHT] =
				( borders[CM_EDGE_RIGHT] == cm_patchGridPlanes[i][j][0] );
			if ( borders[CM_EDGE_RIGHT] == -1 || noAdjust[CM_EDGE_RIGHT] ) {
				borders[CM_EDGE_RIGHT] =
					CM_EdgePlaneNum( grid, cm_patchGridPlanes, i, j,
									 CM_EDGE_RIGHT );
			}

			if ( numFacets == MAX_FACETS ) {
				Com_Error( ERR_DROP, "\x15" "MAX_FACETS" );
			}
			facet = &cm_patchFacets[numFacets];
			Com_Memset( facet, 0, sizeof( *facet ) );

			if ( cm_patchGridPlanes[i][j][0] == cm_patchGridPlanes[i][j][1] ) {
				if ( cm_patchGridPlanes[i][j][0] == -1 ) {
					continue;
				}
				facet->surfacePlane = cm_patchGridPlanes[i][j][0];
				facet->numBorders = 4;
				facet->borderPlanes[0] = borders[CM_EDGE_BOTTOM];
				facet->borderNoAdjust[0] = noAdjust[CM_EDGE_BOTTOM];
				facet->borderPlanes[1] = borders[CM_EDGE_RIGHT];
				facet->borderNoAdjust[1] = noAdjust[CM_EDGE_RIGHT];
				facet->borderPlanes[2] = borders[CM_EDGE_TOP];
				facet->borderNoAdjust[2] = noAdjust[CM_EDGE_TOP];
				facet->borderPlanes[3] = borders[CM_EDGE_LEFT];
				facet->borderNoAdjust[3] = noAdjust[CM_EDGE_LEFT];
				CM_SetBorderInward( facet, grid, cm_patchGridPlanes, i, j,
									CM_FACET_QUAD );
				if ( CM_ValidateFacet( facet ) ) {
					CM_AddFacetBevels( facet );
					numFacets++;
				}
				continue;
			}

			facet->surfacePlane = cm_patchGridPlanes[i][j][0];
			facet->numBorders = 3;
			facet->borderPlanes[0] = borders[CM_EDGE_BOTTOM];
			facet->borderNoAdjust[0] = noAdjust[CM_EDGE_BOTTOM];
			facet->borderPlanes[1] = borders[CM_EDGE_RIGHT];
			facet->borderNoAdjust[1] = noAdjust[CM_EDGE_RIGHT];
			facet->borderPlanes[2] = cm_patchGridPlanes[i][j][1];
			if ( facet->borderPlanes[2] == -1 ) {
				facet->borderPlanes[2] = borders[CM_EDGE_TOP];
				if ( facet->borderPlanes[2] == -1 ) {
					facet->borderPlanes[2] =
						CM_EdgePlaneNum( grid, cm_patchGridPlanes, i, j,
										 CM_EDGE_DIAG_DESC );
				}
			}
			CM_SetBorderInward( facet, grid, cm_patchGridPlanes, i, j,
								CM_FACET_LOWER );
			if ( CM_ValidateFacet( facet ) ) {
				CM_AddFacetBevels( facet );
				numFacets++;
			}

			if ( numFacets == MAX_FACETS ) {
				Com_Error( ERR_DROP, "\x15" "MAX_FACETS" );
			}
			facet = &cm_patchFacets[numFacets];
			Com_Memset( facet, 0, sizeof( *facet ) );

			facet->surfacePlane = cm_patchGridPlanes[i][j][1];
			facet->numBorders = 3;
			facet->borderPlanes[0] = borders[CM_EDGE_TOP];
			facet->borderNoAdjust[0] = noAdjust[CM_EDGE_TOP];
			facet->borderPlanes[1] = borders[CM_EDGE_LEFT];
			facet->borderNoAdjust[1] = noAdjust[CM_EDGE_LEFT];
			facet->borderPlanes[2] = cm_patchGridPlanes[i][j][0];
			if ( facet->borderPlanes[2] == -1 ) {
				facet->borderPlanes[2] = borders[CM_EDGE_BOTTOM];
				if ( facet->borderPlanes[2] == -1 ) {
					facet->borderPlanes[2] =
						CM_EdgePlaneNum( grid, cm_patchGridPlanes, i, j,
										 CM_EDGE_DIAG_ASC );
				}
			}
			CM_SetBorderInward( facet, grid, cm_patchGridPlanes, i, j,
								CM_FACET_UPPER );
			if ( CM_ValidateFacet( facet ) ) {
				CM_AddFacetBevels( facet );
				numFacets++;
			}
		}
	}

	pf->numPlanes = cm_numPatchPlanes;
	pf->numFacets = numFacets;

	size = numFacets * sizeof( facet_t );
	pf->facets = ( facet_t * ) Hunk_AllocAlignInternal( size, 32 );
	Com_Memcpy( pf->facets, cm_patchFacets, size );

	size = cm_numPatchPlanes * sizeof( patchPlane_t );
	pf->planes = ( patchPlane_t * ) Hunk_AllocAlignInternal( size, 32 );
	Com_Memcpy( pf->planes, cm_patchPlanes, size );
}

/* ---- CM_GeneratePatchCollide  0x0041C940 ---- VERIFIED */
patchCollide_t *CM_GeneratePatchCollide( int width, int height, int maxError,
										   const vec3_t *points,
										   vec3_t *bounds )
{
	patchCollide_t *pf;
	cGrid_t *grid;
	int i, j, k;

	if ( width <= 2 || height <= 2 || points == NULL ) {
		Com_Error( ERR_DROP,
				   "\x15" "CM_GeneratePatchFacets: bad parameters: (%i, %i, %p)",
				   width, height, points );
	}

	if ( !( width & 1 ) || !( height & 1 ) ) {
		Com_Error( ERR_DROP, "\x15"
				   "CM_GeneratePatchFacets: even sizes are invalid for quadratic meshes" );
	}

	if ( width > MAX_GRID_SIZE || height > MAX_GRID_SIZE ) {
		Com_Error( ERR_DROP,
				   "\x15" "CM_GeneratePatchFacets: source is > MAX_GRID_SIZE" );
	}

	grid = &cm_patchGrid;
	grid->width = width;
	grid->height = height;
	grid->wrapWidth = qfalse;
	grid->wrapHeight = qfalse;
	for ( i = 0; i < width; i++ ) {
		for ( j = 0; j < height; j++ ) {
			VectorCopy( points[j * width + i], grid->points[i][j] );
		}
	}

	CM_SetGridWrapWidth( grid );
	CM_SubdivideGridColumns( grid, maxError );
	CM_RemoveDegenerateColumns( grid );

	CM_TransposeGrid( grid );

	CM_SetGridWrapWidth( grid );
	CM_SubdivideGridColumns( grid, maxError );
	CM_RemoveDegenerateColumns( grid );

	pf = ( patchCollide_t * )
		 Hunk_AllocAlignInternal( sizeof( patchCollide_t ), 32 );

	for ( k = 0; k < 3; k++ ) {
		bounds[0][k] = CM_BOUNDS_SEED;
		bounds[1][k] = -CM_BOUNDS_SEED;
	}
	for ( i = 0; i < grid->width; i++ ) {
		for ( j = 0; j < grid->height; j++ ) {
			for ( k = 0; k < 3; k++ ) {
				if ( grid->points[i][j][k] < bounds[0][k] ) {
					bounds[0][k] = grid->points[i][j][k];
				}
				if ( grid->points[i][j][k] > bounds[1][k] ) {
					bounds[1][k] = grid->points[i][j][k];
				}
			}
		}
	}

	CM_PatchCollideFromGrid( grid, pf );

	for ( k = 0; k < 3; k++ ) {
		bounds[0][k] -= 1.0f;
		bounds[1][k] += 1.0f;
	}

	return pf;
}

/* ---- CM_TracePointThroughPatchCollide  0x0041CBD0 ---- VERIFIED */
void CM_TracePointThroughPatchCollide( traceWork_t *twOpaque,
									   const patchCollide_t *pc )
{
	cmPatchTrace_t *tw = ( cmPatchTrace_t * ) twOpaque;
	const patchPlane_t *planes;
	const facet_t *facet;
	const float *offset;
	float d1, d2, planeOffset;
	float frac[MAX_PATCH_PLANES];
	qboolean side[MAX_PATCH_PLANES];
	int i, j, k;

	if ( !CM_PLAYER_CURVE_CLIP()->integer || !tw->isPoint ) {
		return;
	}

	planes = pc->planes;
	for ( i = 0; i < pc->numPlanes; i++, planes++ ) {
		offset = tw->offsets[planes->signbits];
		planeOffset = DotProduct( offset, planes->plane );
		d1 = DotProduct( tw->start, planes->plane ) - planes->plane[3]
			 + planeOffset;
		d2 = DotProduct( tw->end, planes->plane ) - planes->plane[3]
			 + planeOffset;

		side[i] = ( d1 > 0.0f );

		if ( d1 == d2 ) {
			frac[i] = 99999.0f;
		} else {
			frac[i] = d1 / ( d1 - d2 );
			if ( frac[i] <= 0.0f ) {
				frac[i] = 99999.0f;
			}
		}
	}

	facet = pc->facets;
	for ( i = 0; i < pc->numFacets; i++, facet++ ) {
		if ( !side[facet->surfacePlane] ) {
			continue;
		}
		if ( frac[facet->surfacePlane] < 0.0f
			 || frac[facet->surfacePlane] > tw->trace.fraction ) {
			continue;
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			k = facet->borderPlanes[j];
			if ( side[k] == facet->borderInward[j] ) {
				if ( frac[k] < frac[facet->surfacePlane] ) {
					break;
				}
			} else {
				if ( frac[k] > frac[facet->surfacePlane] ) {
					break;
				}
			}
		}
		if ( j != facet->numBorders ) {
			continue;
		}

		planes = &pc->planes[facet->surfacePlane];
		offset = tw->offsets[planes->signbits];
		planeOffset = DotProduct( offset, planes->plane );
		d1 = DotProduct( tw->start, planes->plane ) - planes->plane[3]
			 + planeOffset;
		d2 = DotProduct( tw->end, planes->plane ) - planes->plane[3]
			 + planeOffset;

		tw->trace.fraction = ( d1 - SURFACE_CLIP_EPSILON ) / ( d1 - d2 );
		if ( tw->trace.fraction < 0.0f ) {
			tw->trace.fraction = 0.0f;
		}
		VectorCopy( planes->plane, tw->trace.normal );
	}
}

/* ---- CM_SightTracePointThroughPatchCollide  0x0041CE60 ---- VERIFIED */
qboolean CM_SightTracePointThroughPatchCollide( traceWork_t *twOpaque,
												const patchCollide_t *pc )
{
	const cmPatchTrace_t *tw = ( const cmPatchTrace_t * ) twOpaque;
	const patchPlane_t *planes;
	const facet_t *facet;
	const float *offset;
	float d1, d2, planeOffset;
	float frac[MAX_PATCH_PLANES];
	qboolean side[MAX_PATCH_PLANES];
	int i, j, k;

	if ( !CM_PLAYER_CURVE_CLIP()->integer || !tw->isPoint ) {
		return qtrue;
	}

	planes = pc->planes;
	for ( i = 0; i < pc->numPlanes; i++, planes++ ) {
		offset = tw->offsets[planes->signbits];
		planeOffset = DotProduct( offset, planes->plane );
		d1 = DotProduct( tw->start, planes->plane ) - planes->plane[3]
			 + planeOffset;
		d2 = DotProduct( tw->end, planes->plane ) - planes->plane[3]
			 + planeOffset;

		side[i] = ( d1 > 0.0f );

		if ( d1 == d2 ) {
			frac[i] = 99999.0f;
		} else {
			frac[i] = d1 / ( d1 - d2 );
			if ( frac[i] <= 0.0f ) {
				frac[i] = 99999.0f;
			}
		}
	}

	facet = pc->facets;
	for ( i = 0; i < pc->numFacets; i++, facet++ ) {
		if ( !side[facet->surfacePlane] ) {
			continue;
		}
		if ( frac[facet->surfacePlane] < 0.0f ) {
			continue;
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			k = facet->borderPlanes[j];
			if ( side[k] == facet->borderInward[j] ) {
				if ( frac[k] < frac[facet->surfacePlane] ) {
					break;
				}
			} else {
				if ( frac[k] > frac[facet->surfacePlane] ) {
					break;
				}
			}
		}
		if ( j != facet->numBorders ) {
			continue;
		}

		planes = &pc->planes[facet->surfacePlane];
		offset = tw->offsets[planes->signbits];
		planeOffset = DotProduct( offset, planes->plane );
		d1 = DotProduct( tw->start, planes->plane ) - planes->plane[3]
			 + planeOffset;
		d2 = DotProduct( tw->end, planes->plane ) - planes->plane[3]
			 + planeOffset;

		if ( d1 - d2 > 0.0f
			 && ( d1 - SURFACE_CLIP_EPSILON ) / ( d1 - d2 ) < 1.0f ) {
			return qfalse;
		}
	}

	return qtrue;
}

/* ---- CM_CheckFacetPlane  0x0041D0E0 ---- VERIFIED */
int CM_CheckFacetPlane(const float *plane, const vec3_t start, const vec3_t end,
                       float *enterFrac, float *leaveFrac, int *hit)
{
  float d1, d2, f;

  *hit = qfalse;

  d1 = start[0] * plane[0] + start[1] * plane[1] + start[2] * plane[2] - plane[3];
  d2 = end[0] * plane[0] + end[1] * plane[1] + end[2] * plane[2] - plane[3];

  if ( d1 > 0.0f && ( d2 >= SURFACE_CLIP_EPSILON || d2 >= d1 ) ) {
    return qfalse;
  }

  if ( d1 <= 0.0f && d2 <= 0.0f ) {
    return qtrue;
  }

  if ( d1 > d2 ) {
    f = ( d1 - SURFACE_CLIP_EPSILON ) / ( d1 - d2 );
    if ( f < 0.0f ) {
      f = 0.0f;
    }
    if ( f > *enterFrac ) {
      *enterFrac = f;
      *hit = qtrue;
    }
  } else {
    f = ( d1 + SURFACE_CLIP_EPSILON ) / ( d1 - d2 );
    if ( f > 1.0f ) {
      f = 1.0f;
    }
    if ( f < *leaveFrac ) {
      *leaveFrac = f;
    }
  }

  return qtrue;
}

/* ---- CM_TraceThroughPatchCollide  0x0041D210 ---- VERIFIED */
void CM_TraceThroughPatchCollide( traceWork_t *twOpaque,
								  const patchCollide_t *pc )
{
	cmPatchTrace_t *tw = ( cmPatchTrace_t * ) twOpaque;
	const patchPlane_t *planes;
	const facet_t *facet;
	vec4_t plane;
	vec3_t startp, endp, bestplane;
	float enterFrac, leaveFrac, offset, t;
	int hit, hitnum;
	int i, j;

	if ( tw->isPoint ) {
		CM_TracePointThroughPatchCollide( twOpaque, pc );
		return;
	}

	facet = pc->facets;
	for ( i = 0; i < pc->numFacets; i++, facet++ ) {
		enterFrac = -1.0f;
		leaveFrac = tw->trace.fraction;
		hitnum = -1;
		VectorClear( bestplane );

		planes = &pc->planes[facet->surfacePlane];
		VectorCopy( planes->plane, plane );
		plane[3] = planes->plane[3];

		if ( tw->sphere.use ) {
			plane[3] += tw->sphere.radius;

			t = DotProduct( plane, tw->sphere.offset );
			if ( t > 0.0f ) {
				VectorSubtract( tw->start, tw->sphere.offset, startp );
				VectorSubtract( tw->end, tw->sphere.offset, endp );
			} else {
				VectorAdd( tw->start, tw->sphere.offset, startp );
				VectorAdd( tw->end, tw->sphere.offset, endp );
			}
		} else {
			offset = DotProduct( tw->offsets[planes->signbits], plane );
			plane[3] -= offset;
			VectorCopy( tw->start, startp );
			VectorCopy( tw->end, endp );
		}

		if ( !CM_CheckFacetPlane( plane, startp, endp, &enterFrac, &leaveFrac,
								  &hit ) ) {
			continue;
		}
		if ( hit ) {
			VectorCopy( plane, bestplane );
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			planes = &pc->planes[facet->borderPlanes[j]];

			if ( facet->borderInward[j] ) {
				VectorNegate( planes->plane, plane );
				plane[3] = -planes->plane[3];
			} else {
				VectorCopy( planes->plane, plane );
				plane[3] = planes->plane[3];
			}

			if ( tw->sphere.use ) {
				plane[3] += tw->sphere.radius;
				t = DotProduct( plane, tw->sphere.offset );
				if ( t > 0.0f ) {
					VectorSubtract( tw->start, tw->sphere.offset, startp );
					VectorSubtract( tw->end, tw->sphere.offset, endp );
				} else {
					VectorAdd( tw->start, tw->sphere.offset, startp );
					VectorAdd( tw->end, tw->sphere.offset, endp );
				}
			} else {
				offset = DotProduct( tw->offsets[planes->signbits], plane );
				plane[3] += ( float ) fabs( offset );
				VectorCopy( tw->start, startp );
				VectorCopy( tw->end, endp );
			}

			if ( !CM_CheckFacetPlane( plane, startp, endp, &enterFrac,
									  &leaveFrac, &hit ) ) {
				break;
			}
			if ( hit ) {
				hitnum = j;
				VectorCopy( plane, bestplane );
			}
		}

		if ( j < facet->numBorders ) {
			continue;
		}
		if ( hitnum == facet->numBorders - 1 ) {
			continue;
		}

		if ( enterFrac < leaveFrac && enterFrac >= 0.0f
			 && enterFrac < tw->trace.fraction ) {
			if ( enterFrac < 0.0f ) {
				enterFrac = 0.0f;
			}
			tw->trace.fraction = enterFrac;
			VectorCopy( bestplane, tw->trace.normal );
		}
	}
}

/* ---- CM_SightTraceThroughPatchCollide  0x0041D710 ---- VERIFIED */
qboolean CM_SightTraceThroughPatchCollide( traceWork_t *twOpaque,
										   const patchCollide_t *pc )
{
	const cmPatchTrace_t *tw = ( const cmPatchTrace_t * ) twOpaque;
	const patchPlane_t *planes;
	const facet_t *facet;
	vec4_t plane;
	vec3_t startp, endp;
	float enterFrac, leaveFrac, offset, t;
	int hit, hitnum;
	int i, j;

	if ( tw->isPoint ) {
		return CM_SightTracePointThroughPatchCollide( twOpaque, pc );
	}

	facet = pc->facets;
	for ( i = 0; i < pc->numFacets; i++, facet++ ) {
		enterFrac = -1.0f;
		leaveFrac = 1.0f;
		hitnum = -1;

		planes = &pc->planes[facet->surfacePlane];
		VectorCopy( planes->plane, plane );
		plane[3] = planes->plane[3];

		if ( tw->sphere.use ) {
			plane[3] += tw->sphere.radius;
			t = DotProduct( plane, tw->sphere.offset );
			if ( t > 0.0f ) {
				VectorSubtract( tw->start, tw->sphere.offset, startp );
				VectorSubtract( tw->end, tw->sphere.offset, endp );
			} else {
				VectorAdd( tw->start, tw->sphere.offset, startp );
				VectorAdd( tw->end, tw->sphere.offset, endp );
			}
		} else {
			offset = DotProduct( tw->offsets[planes->signbits], plane );
			plane[3] -= offset;
			VectorCopy( tw->start, startp );
			VectorCopy( tw->end, endp );
		}

		if ( !CM_CheckFacetPlane( plane, startp, endp, &enterFrac, &leaveFrac,
								  &hit ) ) {
			continue;
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			planes = &pc->planes[facet->borderPlanes[j]];

			if ( facet->borderInward[j] ) {
				VectorNegate( planes->plane, plane );
				plane[3] = -planes->plane[3];
			} else {
				VectorCopy( planes->plane, plane );
				plane[3] = planes->plane[3];
			}

			if ( tw->sphere.use ) {
				plane[3] += tw->sphere.radius;
				t = DotProduct( plane, tw->sphere.offset );
				if ( t > 0.0f ) {
					VectorSubtract( tw->start, tw->sphere.offset, startp );
					VectorSubtract( tw->end, tw->sphere.offset, endp );
				} else {
					VectorAdd( tw->start, tw->sphere.offset, startp );
					VectorAdd( tw->end, tw->sphere.offset, endp );
				}
			} else {
				offset = DotProduct( tw->offsets[planes->signbits], plane );
				plane[3] += ( float ) fabs( offset );
				VectorCopy( tw->start, startp );
				VectorCopy( tw->end, endp );
			}

			if ( !CM_CheckFacetPlane( plane, startp, endp, &enterFrac,
									  &leaveFrac, &hit ) ) {
				break;
			}
			if ( hit ) {
				hitnum = j;
			}
		}

		if ( j >= facet->numBorders && hitnum != facet->numBorders - 1
			 && enterFrac < leaveFrac && enterFrac >= 0.0f ) {
			return qfalse;
		}
	}

	return qtrue;
}

/* ---- CM_PositionTestInPatchCollide  0x0041DB90 ---- VERIFIED */
qboolean CM_PositionTestInPatchCollide(traceWork_t *twOpaque, const patchCollide_t *pc)
{
  const cmPatchTrace_t *tw = ( const cmPatchTrace_t * ) twOpaque;
  const patchPlane_t *planes;
  const facet_t *facet;
  float plane[4];
  vec3_t startp;
  float offset, t;
  int i, j;

  if ( tw->isPoint ) {
    return qfalse;
  }

  facet = pc->facets;
  for ( i = 0; i < pc->numFacets; i++, facet++ ) {
    planes = &pc->planes[facet->surfacePlane];
    plane[0] = planes->plane[0];
    plane[1] = planes->plane[1];
    plane[2] = planes->plane[2];
    plane[3] = planes->plane[3];

    if ( tw->sphere.use ) {
      plane[3] += tw->sphere.radius;
      t = plane[0] * tw->sphere.offset[0] + plane[1] * tw->sphere.offset[1]
          + plane[2] * tw->sphere.offset[2];
      if ( t > 0.0f ) {
        VectorSubtract( tw->start, tw->sphere.offset, startp );
      } else {
        VectorAdd( tw->start, tw->sphere.offset, startp );
      }
    } else {
      offset = plane[0] * tw->offsets[planes->signbits][0]
               + plane[1] * tw->offsets[planes->signbits][1]
               + plane[2] * tw->offsets[planes->signbits][2];
      plane[3] -= offset;
      VectorCopy( tw->start, startp );
    }

    if ( startp[0] * plane[0] + startp[1] * plane[1] + startp[2] * plane[2]
         - plane[3] > 0.0f ) {
      continue;
    }

    for ( j = 0; j < facet->numBorders; j++ ) {
      planes = &pc->planes[facet->borderPlanes[j]];

      if ( facet->borderInward[j] ) {
        plane[0] = -planes->plane[0];
        plane[1] = -planes->plane[1];
        plane[2] = -planes->plane[2];
        plane[3] = -planes->plane[3];
      } else {
        plane[0] = planes->plane[0];
        plane[1] = planes->plane[1];
        plane[2] = planes->plane[2];
        plane[3] = planes->plane[3];
      }

      if ( tw->sphere.use ) {
        plane[3] += tw->sphere.radius;
        t = plane[0] * tw->sphere.offset[0] + plane[1] * tw->sphere.offset[1]
            + plane[2] * tw->sphere.offset[2];
        if ( t > 0.0f ) {
          VectorSubtract( tw->start, tw->sphere.offset, startp );
        } else {
          VectorAdd( tw->start, tw->sphere.offset, startp );
        }
      } else {
        offset = plane[0] * tw->offsets[planes->signbits][0]
                 + plane[1] * tw->offsets[planes->signbits][1]
                 + plane[2] * tw->offsets[planes->signbits][2];
        plane[3] += (float)fabs( offset );
        VectorCopy( tw->start, startp );
      }

      if ( startp[0] * plane[0] + startp[1] * plane[1] + startp[2] * plane[2]
           - plane[3] > 0.0f ) {
        break;
      }
    }

    if ( j < facet->numBorders ) {
      continue;
    }

    return qtrue;
  }

  return qfalse;
}

#if 0   /* the machine output this replaces */
qboolean __cdecl CM_PositionTestInPatchCollide(traceWork_t *tw, const patchCollide_t *pc)
{
  int v2;
  int v3;
  int v5;
  int v6;
  unsigned __int8 v8; // c0
  unsigned __int8 v9; // c3
  int v10;
  int v11;
  int v12;
  unsigned __int8 v14; // c0
  unsigned __int8 v15; // c3
  bool v16; // cc
  int v17;
  int v18;

  if ( *(_DWORD *)(v3 + 192) )
    return 0;
  v5 = *(_DWORD *)(v2 + 12);
  v6 = *(_DWORD *)(v2 + 8);
  v18 = v5;
  v17 = 0;
  if ( v6 <= 0 )
    return 0;
  while ( !(v8 | v9) )
  {
LABEL_12:
    v5 += 320;
    v16 = ++v17 < v6;
    v18 = v5;
    if ( !v16 )
      return 0;
  }
  v10 = *(_DWORD *)(v5 + 4);
  v11 = 0;
  if ( v10 > 0 )
  {
    v12 = v5 + 112;
    do
    {
      if ( !(v14 | v15) )
        break;
      ++v11;
      v12 += 4;
    }
    while ( v11 < v10 );
    v5 = v18;
  }
  if ( v11 < v10 )
  {
    v6 = *(_DWORD *)(v2 + 8);
    goto LABEL_12;
  }
  return 1;
}

#endif  /* the machine output this replaces */
