/*
 * qcommon/cm_trace.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_trace.c
 *
 * Retail range 0x004222D0-0x00426750, 37 functions.
 *
 * @fidelity: likely
 */

#include "cm_local.h"

void CM_TraceThroughPatchCollide( traceWork_t *tw, const patchCollide_t *pc );
qboolean CM_SightTraceThroughPatchCollide( traceWork_t *tw,
										   const patchCollide_t *pc );
qboolean CM_PositionTestInPatchCollide( traceWork_t *tw,
										const patchCollide_t *pc );
void CM_TraceThroughTerrainCollide( traceWork_t *tw, void *tc );
qboolean CM_SightTraceThroughTerrainCollide( traceWork_t *tw, void *tc );
qboolean CM_PositionTestInTerrainCollide( traceWork_t *tw, void *tc );

vec_t VectorNormalize2( const vec3_t v, vec3_t out );

typedef struct cmSphereRecord_t
{
	sphere_t sphere;                /* +0x00 */
	vec3_t extents;                 /* +0x18 */
} cmSphereRecord_t;
COD1_ASSERT_SIZE( cmSphereRecord_t, 36 );

#define CM_TEMP_BOX_HANDLE      BOX_MODEL_HANDLE
#define CM_TEMP_CAPSULE_HANDLE  CAPSULE_MODEL_HANDLE

#define SURFACE_CLIP_EPSILON    0.125f

static void CM_TraceThroughBrush( traceWork_t *tw, const cbrush_t *brush );
static qboolean CM_TraceSphereThroughSphere( traceWork_t *tw,
											 const vec3_t start,
											 const vec3_t end,
											 const vec3_t sphereOrigin,
											 float sphereRadius );
static qboolean CM_TraceCylinderThroughCylinder( traceWork_t *tw,
												 const vec3_t cylinderOrigin,
												 float cylinderHalfHeight,
												 float cylinderRadius );
static int CM_SightTraceThroughBrush( const traceWork_t *tw,
									  const cbrush_t *brush );
static qboolean CM_SightTraceSphereThroughSphere( const traceWork_t *tw,
												  const vec3_t start,
												  const vec3_t end,
												  const vec3_t sphereOrigin,
												  float sphereRadius );
static qboolean CM_SightTraceCylinderThroughCylinder( const traceWork_t *tw,
													  const vec3_t origin,
													  float halfheight,
													  float radius );

/* ---- CM_Q_fabs  0x004222D0 ---- VERIFIED */
float CM_Q_fabs( float v ) {
	int i = *( int * )&v;

	i &= 0x7FFFFFFF;
	return *( float * )&i;
}

/* ---- RotatePoint  0x004222F0 ---- VERIFIED */
void RotatePoint( vec3_t point, vec3_t matrix[3] ) {
	float x = point[0];
	float y = point[1];
	float z = point[2];

	point[0] = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
	point[1] = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
	point[2] = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;
}

/* ---- TransposeMatrix  0x00422340 ---- VERIFIED */
void TransposeMatrix( vec3_t in[3], vec3_t out[3] ) {
	int i, j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			out[i][j] = in[j][i];
		}
	}
}

/* ---- CreateRotationMatrix  0x00422370 ---- VERIFIED */
void CreateRotationMatrix( const vec3_t angles, vec3_t matrix[3] ) {
	AngleVectors( angles, matrix[0], matrix[1], matrix[2] );

	matrix[1][0] = -matrix[1][0];
	matrix[1][1] = -matrix[1][1];
	matrix[1][2] = -matrix[1][2];
}

/* ---- CM_ProjectPointOntoVector  0x004223A0 ---- VERIFIED */
void CM_ProjectPointOntoVector( const vec3_t point, const vec3_t vStart,
								const vec3_t vDir, vec3_t vProj ) {
	vec3_t pVec;
	float d;

	VectorSubtract( point, vStart, pVec );
	d = DotProduct( pVec, vDir );
	VectorMA( vStart, d, vDir, vProj );
}

/* ---- CM_DistanceFromLineSquared  0x00422400 ---- VERIFIED */
float CM_DistanceFromLineSquared( const vec3_t p, const vec3_t lp1,
								  const vec3_t lp2, const vec3_t dir ) {
	vec3_t proj, t;
	int j;

	CM_ProjectPointOntoVector( p, lp1, dir, proj );

	for ( j = 0; j < 3; j++ ) {
		if ( ( proj[j] > lp1[j] && proj[j] > lp2[j] ) ||
			 ( proj[j] < lp1[j] && proj[j] < lp2[j] ) ) {
			break;
		}
	}

	if ( j < 3 ) {
		if ( fabs( proj[j] - lp1[j] ) < fabs( proj[j] - lp2[j] ) ) {
			VectorSubtract( p, lp1, t );
		} else {
			VectorSubtract( p, lp2, t );
		}
		return DotProduct( t, t );
	}

	VectorSubtract( p, proj, t );
	return DotProduct( t, t );
}

/* ---- CM_VectorDistanceSquared  0x004225B0 ---- VERIFIED */
float CM_VectorDistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	vec3_t dir;

	VectorSubtract( p2, p1, dir );
	return DotProduct( dir, dir );
}

/* ---- SquareRootFloat  0x004225E0 ---- VERIFIED */
float SquareRootFloat( float number ) {
	int i;
	float x, y;
	const float f = 1.5F;

	x = number * 0.5F;
	y = number;
	i = *( int * )&y;
	i = 0x5F3759DF - ( i >> 1 );
	y = *( float * )&i;
	y = y * ( f - ( x * y * y ) );
	y = y * ( f - ( x * y * y ) );
	return number * y;
}

/* ---- CM_TestBoxInBrush  0x00422630 ---- VERIFIED */
void CM_TestBoxInBrush( traceWork_t *tw, const cbrush_t *brush ) {
	const cbrushside_t *side;
	const cplane_t *plane;
	const float *offset;
	vec3_t testPoint;
	float planeDist, sphereOffset, d;
	int i;

	if ( tw->bounds[0][0] > brush->maxs[0] ) {
		return;
	}
	if ( tw->bounds[0][1] > brush->maxs[1] ) {
		return;
	}
	if ( tw->bounds[0][2] > brush->maxs[2] ) {
		return;
	}
	if ( brush->mins[0] > tw->bounds[1][0] ) {
		return;
	}
	if ( brush->mins[1] > tw->bounds[1][1] ) {
		return;
	}
	if ( brush->mins[2] > tw->bounds[1][2] ) {
		return;
	}

	side = brush->sides;

	if ( tw->sphere.use ) {
		for ( i = brush->numSides; i != 0; i--, side++ ) {
			plane = side->plane;
			planeDist = plane->dist + tw->sphere.radius;

			sphereOffset = DotProduct( plane->normal, tw->sphere.offset );
			if ( sphereOffset > 0.0f ) {
				VectorSubtract( tw->start, tw->sphere.offset, testPoint );
			} else {
				VectorAdd( tw->start, tw->sphere.offset, testPoint );
			}

			d = DotProduct( testPoint, plane->normal ) - planeDist;
			if ( d > 0.0f ) {
				return;
			}
		}
	} else {
		for ( i = brush->numSides; i != 0; i--, side++ ) {
			plane = side->plane;

			offset = tw->offsets[plane->signbits];
			planeDist = plane->dist - DotProduct( offset, plane->normal );

			d = DotProduct( tw->start, plane->normal ) - planeDist;
			if ( d > 0.0f ) {
				return;
			}
		}
	}

	tw->trace.allsolid = 1;
	tw->trace.startsolid = 1;
	tw->trace.fraction = 0.0f;
	tw->trace.contents = brush->contents;
}

/* ---- CM_TestInLeaf  0x004227F0 ---- VERIFIED */
void CM_TestInLeaf( traceWork_t *tw, const cLeaf_t *leaf ) {
	int i;
	int brushnum, patchnum;
	cbrush_t *b;
	cPatch_t *patch;
	qboolean hit;

	for ( i = 0; i < ( int )leaf->numLeafBrushes; i++ ) {
		brushnum = cm_leafbrushes[leaf->firstLeafBrush + i];
		b = &cm_brushes[brushnum];

		if ( b->checkcount == cm_checkcount ) {
			continue;
		}
		b->checkcount = cm_checkcount;

		if ( !( tw->contents & b->contents ) ) {
			continue;
		}

		CM_TestBoxInBrush( tw, b );
		if ( tw->trace.allsolid ) {
			return;
		}
	}

	if ( cm_noCurves->integer ) {
		return;
	}

	for ( i = 0; i < ( int )leaf->numLeafSurfaces; i++ ) {
		patchnum = cm_leafsurfaces[leaf->firstLeafSurface + i];
		patch = &cm_patches[patchnum];

		if ( patch->checkcount == cm_checkcount ) {
			continue;
		}
		patch->checkcount = cm_checkcount;

		if ( !( tw->contents & patch->contents ) ) {
			continue;
		}

		if ( patch->pc ) {
			hit = CM_PositionTestInPatchCollide( tw, patch->pc );
		} else {
			hit = CM_PositionTestInTerrainCollide( tw, patch->tc );
		}

		if ( hit ) {
			tw->trace.allsolid = 1;
			tw->trace.startsolid = 1;
			tw->trace.fraction = 0.0f;
			return;
		}
	}
}

/* ---- CM_TestCapsuleInCapsule  0x00422920 ---- VERIFIED */
void CM_TestCapsuleInCapsule( traceWork_t *tw ) {
	vec3_t startTop, startBottom;
	vec3_t center, offsetMaxs;
	vec3_t p, delta;
	float radius, halfheight, radiusSquared;
	float zdelta, cylinderHalfHeight;
	int i;

	VectorAdd( tw->start, tw->sphere.offset, startTop );
	VectorSubtract( tw->start, tw->sphere.offset, startBottom );

	for ( i = 0; i < 3; i++ ) {
		center[i] = ( cm_boxModel.mins[i] + cm_boxModel.maxs[i] ) * 0.5f;
		offsetMaxs[i] = cm_boxModel.maxs[i] - center[i];
	}

	radius = offsetMaxs[0] > offsetMaxs[2] ? offsetMaxs[2] : offsetMaxs[0];
	halfheight = offsetMaxs[2] - radius;
	radiusSquared = ( tw->sphere.radius + radius ) *
					( tw->sphere.radius + radius );

	p[0] = center[0];
	p[1] = center[1];
	p[2] = center[2] + halfheight;

	VectorSubtract( p, startTop, delta );
	if ( DotProduct( delta, delta ) < radiusSquared ) {
		tw->trace.allsolid = 1;
		tw->trace.startsolid = 1;
		tw->trace.fraction = 0.0f;
	}

	VectorSubtract( p, startBottom, delta );
	if ( DotProduct( delta, delta ) < radiusSquared ) {
		tw->trace.allsolid = 1;
		tw->trace.startsolid = 1;
		tw->trace.fraction = 0.0f;
	}

	p[2] = center[2] - halfheight;

	VectorSubtract( p, startTop, delta );
	if ( DotProduct( delta, delta ) < radiusSquared ) {
		tw->trace.allsolid = 1;
		tw->trace.startsolid = 1;
		tw->trace.fraction = 0.0f;
	}

	VectorSubtract( p, startBottom, delta );
	if ( DotProduct( delta, delta ) < radiusSquared ) {
		tw->trace.allsolid = 1;
		tw->trace.startsolid = 1;
		tw->trace.fraction = 0.0f;
	}

	zdelta = tw->start[2] - center[2];
	cylinderHalfHeight = halfheight + tw->sphere.halfheight - tw->sphere.radius;

	if ( zdelta <= cylinderHalfHeight && zdelta >= -cylinderHalfHeight ) {
		p[2] = 0.0f;
		startTop[2] = 0.0f;

		VectorSubtract( startTop, p, delta );
		if ( DotProduct( delta, delta ) < radiusSquared ) {
			tw->trace.allsolid = 1;
			tw->trace.startsolid = 1;
			tw->trace.fraction = 0.0f;
		}
	}
}

/* ---- CM_TestBoundingBoxInCapsule  0x00422BB0 ---- VERIFIED */
void CM_TestBoundingBoxInCapsule( traceWork_t *tw ) {
	vec3_t center, offsetMaxs;
	float radius;
	int i;

	for ( i = 0; i < 3; i++ ) {
		center[i] = ( cm_boxModel.mins[i] + cm_boxModel.maxs[i] ) * 0.5f;
		offsetMaxs[i] = cm_boxModel.maxs[i] - center[i];
		tw->start[i] -= center[i];
		tw->end[i] -= center[i];
	}

	tw->sphere.use = qtrue;
	radius = offsetMaxs[2] < offsetMaxs[0] ? offsetMaxs[2] : offsetMaxs[0];
	tw->sphere.radius = radius;
	tw->sphere.halfheight = offsetMaxs[2];
	tw->sphere.offset[0] = 0.0f;
	tw->sphere.offset[1] = 0.0f;
	tw->sphere.offset[2] = offsetMaxs[2] - radius;

	CM_TempBoxModel( tw->mins, tw->maxs, box_brush->contents, qfalse );
	CM_TestBoxInBrush( tw, box_brush );
}

/* ---- CM_PositionTest  0x00422D30 ---- VERIFIED */
void CM_PositionTest( traceWork_t *tw ) {
	leafList_t ll;
	int leafs[1024];
	vec3_t mins, maxs;
	int i;

	for ( i = 0; i < 3; i++ ) {
		mins[i] = tw->start[i] + tw->mins[i] - 1.0f;
		maxs[i] = tw->start[i] + tw->maxs[i] + 1.0f;
	}

	ll.count = 0;
	ll.maxcount = 1024;
	ll.list = leafs;
	ll.storeLeafs = CM_StoreLeafs;
	ll.lastLeaf = 0;
	ll.overflowed = qfalse;
	VectorCopy( mins, ll.bounds[0] );
	VectorCopy( maxs, ll.bounds[1] );

	cm_checkcount++;
	CM_BoxLeafnums_r( &ll, 0 );
	cm_checkcount++;

	for ( i = 0; i < ll.count; i++ ) {
		CM_TestInLeaf( tw, &cm_leafs[leafs[i]] );
		if ( tw->trace.allsolid ) {
			break;
		}
	}
}

/* ---- CM_TraceMayIntersectBox  0x00422E60 ---- VERIFIED */
qboolean CM_TraceMayIntersectBox( const traceWork_t *tw, const vec3_t mins,
								  const vec3_t maxs ) {
	vec3_t size, centerSum, centerDelta, traceSize;
	float cross, projected;
	int i;

	if ( maxs[0] < tw->bounds[0][0] || tw->bounds[1][0] < mins[0] ) {
		return qfalse;
	}
	if ( maxs[1] < tw->bounds[0][1] || tw->bounds[1][1] < mins[1] ) {
		return qfalse;
	}
	if ( maxs[2] < tw->bounds[0][2] || tw->bounds[1][2] < mins[2] ) {
		return qfalse;
	}

	if ( !tw->isPoint ) {
		return qtrue;
	}

	for ( i = 0; i < 3; i++ ) {
		size[i] = maxs[i] - mins[i];
		centerSum[i] = maxs[i] + mins[i];
		centerDelta[i] = ( tw->end[i] + tw->start[i] ) - centerSum[i];
		traceSize[i] = tw->bounds[1][i] - tw->bounds[0][i];
	}

	cross = tw->delta[1] * centerDelta[2] - tw->delta[2] * centerDelta[1];
	projected = size[1] * traceSize[2] + size[2] * traceSize[1];
	if ( projected * projected < cross * cross ) {
		return qfalse;
	}

	cross = tw->delta[2] * centerDelta[0] - tw->delta[0] * centerDelta[2];
	projected = size[0] * traceSize[2] + size[2] * traceSize[0];
	if ( projected * projected < cross * cross ) {
		return qfalse;
	}

	cross = tw->delta[0] * centerDelta[1] - tw->delta[1] * centerDelta[0];
	projected = size[0] * traceSize[1] + size[1] * traceSize[0];
	if ( projected * projected < cross * cross ) {
		return qfalse;
	}

	return qtrue;
}

/* ---- CM_TraceThroughPatch  0x00423030 ---- VERIFIED */
void CM_TraceThroughPatch( traceWork_t *tw, const cPatch_t *patch ) {
	float oldFrac;

	if ( !CM_TraceMayIntersectBox( tw, patch->mins, patch->maxs ) ) {
		return;
	}

	oldFrac = tw->trace.fraction;

	if ( patch->pc ) {
		CM_TraceThroughPatchCollide( tw, patch->pc );
	} else {
		CM_TraceThroughTerrainCollide( tw, patch->tc );
	}

	if ( tw->trace.fraction < oldFrac ) {
		tw->trace.surfaceFlags = cm_materials[patch->materialNum].surfaceFlags;
		tw->trace.contents = patch->contents;
		tw->trace.material = ( int )&cm_materials[patch->materialNum];
	}
}

/* ---- CM_TraceThroughBrush  0x004230C0 ---- VERIFIED */
static void CM_TraceThroughBrush( traceWork_t *tw, const cbrush_t *brush ) {
	float enterFrac, leaveFrac;
	qboolean getout, startout;
	const cbrushside_t *side;
	const cbrushside_t *clipside;
	const cplane_t *plane;
	const float *offset;
	cplane_t axialPlane;
	cbrushside_t axialSide;
	vec3_t startPoint, endPoint;
	float startDist, endDist, denom, enterLimit;
	float planeDist, sphereOffset, workOffset, sign;
	const float *bounds;
	int pass, axis, i;

	enterFrac = 0.0f;
	leaveFrac = tw->trace.fraction;
	getout = qfalse;
	startout = qtrue;
	clipside = NULL;

	axialPlane.normal[0] = 0.0f;
	axialPlane.normal[1] = 0.0f;
	axialPlane.normal[2] = 0.0f;
	axialPlane.dist = 0.0f;
	axialPlane.type = 0;
	axialPlane.signbits = 0;
	axialPlane.pad[0] = 0;
	axialPlane.pad[1] = 0;
	axialSide.plane = &axialPlane;
	axialSide.materialNum = 0;
	( void )getout;

	for ( pass = 0; pass <= 1; pass++ ) {
		sign = ( pass == 0 ) ? -1.0f : 1.0f;
		bounds = ( pass == 0 ) ? brush->mins : brush->maxs;

		for ( axis = 0; axis <= 2; axis++ ) {
			if ( tw->sphere.use ) {
				workOffset = tw->sphereExtents[axis];
			} else {
				workOffset = tw->maxs[axis];
			}

			startDist = ( tw->start[axis] - bounds[axis] ) * sign - workOffset;
			endDist = ( tw->end[axis] - bounds[axis] ) * sign - workOffset;

			if ( startDist > 0.0f ) {
				denom = startDist - endDist;

				if ( endDist > 0.0f ) {
					if ( denom <= 0.0f ) {
						return;
					}
					if ( endDist >= SURFACE_CLIP_EPSILON ) {
						return;
					}
					startout = qfalse;
				}

				enterLimit = startDist - SURFACE_CLIP_EPSILON;

				if ( enterFrac * denom < enterLimit ) {
					enterFrac = enterLimit / denom;
					if ( leaveFrac <= enterFrac ) {
						return;
					}
				} else if ( clipside != NULL ) {
					continue;
				}

				axialPlane.normal[0] = 0.0f;
				axialPlane.normal[1] = 0.0f;
				axialPlane.normal[2] = 0.0f;
				axialPlane.normal[axis] = sign;
				axialSide.materialNum = brush->axialMaterialNum[pass * 3 + axis];
				clipside = &axialSide;
			} else if ( endDist > 0.0f ) {
				denom = startDist - endDist;

				startout = qfalse;
				if ( leaveFrac * denom < startDist ) {
					leaveFrac = startDist / denom;
					if ( leaveFrac <= enterFrac ) {
						return;
					}
				}
			}
		}
	}

	side = brush->sides;
	for ( i = brush->numSides; i != 0; i--, side++ ) {
		plane = side->plane;

		if ( tw->sphere.use ) {
			planeDist = plane->dist + tw->sphere.radius;
			sphereOffset = DotProduct( plane->normal, tw->sphere.offset );

			if ( sphereOffset > 0.0f ) {
				VectorSubtract( tw->start, tw->sphere.offset, startPoint );
				VectorSubtract( tw->end, tw->sphere.offset, endPoint );
			} else {
				VectorAdd( tw->start, tw->sphere.offset, startPoint );
				VectorAdd( tw->end, tw->sphere.offset, endPoint );
			}

			startDist = DotProduct( startPoint, plane->normal ) - planeDist;
			endDist = DotProduct( endPoint, plane->normal ) - planeDist;
		} else {
			offset = tw->offsets[plane->signbits];
			planeDist = plane->dist - DotProduct( offset, plane->normal );
			startDist = DotProduct( tw->start, plane->normal ) - planeDist;
			endDist = DotProduct( tw->end, plane->normal ) - planeDist;
		}

		if ( startDist > 0.0f ) {
			denom = startDist - endDist;

			if ( endDist > 0.0f ) {
				if ( denom <= 0.0f ) {
					return;
				}
				if ( endDist >= SURFACE_CLIP_EPSILON ) {
					return;
				}
				startout = qfalse;
			}

			enterLimit = startDist - SURFACE_CLIP_EPSILON;

			if ( enterFrac * denom < enterLimit ) {
				enterFrac = enterLimit / denom;
				if ( leaveFrac <= enterFrac ) {
					return;
				}
			} else if ( clipside != NULL ) {
				continue;
			}

			clipside = side;
		} else if ( endDist > 0.0f ) {
			denom = startDist - endDist;

			startout = qfalse;
			if ( leaveFrac * denom < startDist ) {
				leaveFrac = startDist / denom;
				if ( leaveFrac <= enterFrac ) {
					return;
				}
			}
		}
	}

	tw->trace.contents = brush->contents;

	if ( clipside == NULL ) {
		tw->trace.startsolid = 1;
		if ( startout ) {
			tw->trace.allsolid = 1;
			tw->trace.fraction = 0.0f;
		}
		return;
	}

	tw->trace.fraction = enterFrac;
	VectorCopy( clipside->plane->normal, tw->trace.normal );
	tw->trace.surfaceFlags = cm_materials[clipside->materialNum].surfaceFlags;
	tw->trace.material = ( int )&cm_materials[clipside->materialNum];
}

/* ---- CM_TraceThroughLeaf  0x00423870 ---- VERIFIED */
void CM_TraceThroughLeaf( traceWork_t *tw, const cLeaf_t *leaf ) {
	int i;
	int brushnum, patchnum;
	cbrush_t *b;
	cPatch_t *patch;

	for ( i = 0; i < ( int )leaf->numLeafBrushes; i++ ) {
		brushnum = cm_leafbrushes[leaf->firstLeafBrush + i];
		b = &cm_brushes[brushnum];

		if ( b->checkcount == cm_checkcount ) {
			continue;
		}
		b->checkcount = cm_checkcount;

		if ( !( tw->contents & b->contents ) ) {
			continue;
		}

		CM_TraceThroughBrush( tw, b );
		if ( tw->trace.fraction == 0.0f ) {
			return;
		}
	}

	if ( cm_noCurves->integer ) {
		return;
	}

	for ( i = 0; i < ( int )leaf->numLeafSurfaces; i++ ) {
		patchnum = cm_leafsurfaces[leaf->firstLeafSurface + i];
		patch = &cm_patches[patchnum];

		if ( patch->checkcount == cm_checkcount ) {
			continue;
		}
		patch->checkcount = cm_checkcount;

		if ( !( tw->contents & patch->contents ) ) {
			continue;
		}

		CM_TraceThroughPatch( tw, patch );
		if ( tw->trace.fraction == 0.0f ) {
			return;
		}
	}
}

/* ---- CM_TraceSphereThroughSphere  0x00423950 ---- VERIFIED */
static qboolean CM_TraceSphereThroughSphere( traceWork_t *tw,
											 const vec3_t start,
											 const vec3_t end,
											 const vec3_t sphereOrigin,
											 float sphereRadius ) {
	vec3_t delta, normal;
	float radiusSquared, startDist;
	float deltaDot, disc, dist, baseFrac, enterFrac;

	VectorSubtract( start, sphereOrigin, delta );

	radiusSquared = ( sphereRadius + tw->sphere.radius ) *
					( sphereRadius + tw->sphere.radius );
	startDist = DotProduct( delta, delta ) - radiusSquared;

	if ( startDist <= 0.0f ) {
		tw->trace.fraction = 0.0f;
		tw->trace.startsolid = 1;
		VectorNormalize2( delta, tw->trace.normal );
		tw->trace.contents = box_brush->contents;

		VectorSubtract( end, sphereOrigin, delta );
		if ( DotProduct( delta, delta ) <= radiusSquared ) {
			tw->trace.allsolid = 1;
		}
		return qfalse;
	}

	deltaDot = DotProduct( tw->delta, delta );
	if ( deltaDot >= 0.0f ) {
		return qtrue;
	}

	disc = deltaDot * deltaDot - tw->deltaLengthSquared * startDist;
	if ( disc < 0.0f ) {
		return qtrue;
	}

	dist = VectorNormalize2( delta, normal );
	baseFrac = ( -deltaDot - ( float )sqrt( ( double )disc ) ) /
			   tw->deltaLengthSquared;
	enterFrac = baseFrac + ( dist * SURFACE_CLIP_EPSILON ) / deltaDot;

	if ( enterFrac < tw->trace.fraction ) {
		tw->trace.fraction = enterFrac > 0.0f ? enterFrac : 0.0f;
		VectorCopy( normal, tw->trace.normal );
		tw->trace.contents = box_brush->contents;
		return qfalse;
	}

	return qtrue;
}

/* ---- CM_TraceCylinderThroughCylinder  0x00423B30 ---- VERIFIED */
static qboolean CM_TraceCylinderThroughCylinder( traceWork_t *tw,
												 const vec3_t cylinderOrigin,
												 float cylinderHalfHeight,
												 float cylinderRadius ) {
	vec3_t delta, normal;
	float radiusSquared, startDist;
	float halfheight, deltaDot, lengthSquared, disc;
	float dist, epsFrac, baseFrac, hitFrac, z;

	VectorSubtract( tw->start, cylinderOrigin, delta );

	radiusSquared = ( cylinderRadius + tw->sphere.radius ) *
					( cylinderRadius + tw->sphere.radius );
	startDist = ( delta[0] * delta[0] + delta[1] * delta[1] ) - radiusSquared;

	if ( startDist <= 0.0f ) {
		halfheight = tw->sphere.halfheight - tw->sphere.radius +
					 cylinderHalfHeight;

		if ( delta[2] > halfheight || delta[2] < -halfheight ) {
			return qtrue;
		}

		tw->trace.fraction = 0.0f;
		tw->trace.startsolid = 1;
		delta[2] = 0.0f;
		VectorNormalize2( delta, tw->trace.normal );
		tw->trace.contents = box_brush->contents;

		VectorSubtract( tw->end, cylinderOrigin, delta );
		if ( delta[2] <= halfheight && delta[2] >= -halfheight ) {
			tw->trace.allsolid = 1;
		}
		return qfalse;
	}

	deltaDot = tw->delta[0] * delta[0] + tw->delta[1] * delta[1];
	if ( deltaDot >= 0.0f ) {
		return qtrue;
	}

	lengthSquared = tw->delta[0] * tw->delta[0] + tw->delta[1] * tw->delta[1];
	disc = deltaDot * deltaDot - lengthSquared * startDist;
	if ( disc < 0.0f ) {
		return qtrue;
	}

	delta[2] = 0.0f;
	dist = VectorNormalize2( delta, normal );
	epsFrac = ( dist * SURFACE_CLIP_EPSILON ) / deltaDot;
	baseFrac = ( -deltaDot - ( float )sqrt( ( double )disc ) ) / lengthSquared;
	hitFrac = baseFrac + epsFrac;

	if ( !( tw->trace.fraction > hitFrac ) ) {
		return qtrue;
	}

	halfheight = tw->sphere.halfheight - tw->sphere.radius + cylinderHalfHeight;
	z = ( ( hitFrac - epsFrac ) * tw->delta[2] + tw->start[2] ) -
		cylinderOrigin[2];

	if ( z > halfheight || z < -halfheight ) {
		return qtrue;
	}

	tw->trace.fraction = hitFrac > 0.0f ? hitFrac : 0.0f;
	VectorCopy( normal, tw->trace.normal );
	tw->trace.contents = box_brush->contents;
	return qfalse;
}

/* ---- CM_TraceCapsuleThroughCapsule  0x00423D90 ---- VERIFIED */
void CM_TraceCapsuleThroughCapsule( traceWork_t *tw ) {
	vec3_t startTop, startBottom, endTop, endBottom;
	vec3_t center, offsetMaxs, topOrigin, bottomOrigin;
	float radius, halfheight;
	int i;

	if ( tw->bounds[0][0] > cm_boxModel.maxs[0] + 1.0f ) {
		return;
	}
	if ( tw->bounds[0][1] > cm_boxModel.maxs[1] + 1.0f ) {
		return;
	}
	if ( tw->bounds[0][2] > cm_boxModel.maxs[2] + 1.0f ) {
		return;
	}
	if ( cm_boxModel.mins[0] - 1.0f > tw->bounds[1][0] ) {
		return;
	}
	if ( cm_boxModel.mins[1] - 1.0f > tw->bounds[1][1] ) {
		return;
	}
	if ( cm_boxModel.mins[2] - 1.0f > tw->bounds[1][2] ) {
		return;
	}

	VectorAdd( tw->start, tw->sphere.offset, startTop );
	VectorSubtract( tw->start, tw->sphere.offset, startBottom );
	VectorAdd( tw->end, tw->sphere.offset, endTop );
	VectorSubtract( tw->end, tw->sphere.offset, endBottom );

	for ( i = 0; i < 3; i++ ) {
		center[i] = ( cm_boxModel.mins[i] + cm_boxModel.maxs[i] ) * 0.5f;
		offsetMaxs[i] = cm_boxModel.maxs[i] - center[i];
	}

	radius = offsetMaxs[2] < offsetMaxs[0] ? offsetMaxs[2] : offsetMaxs[0];
	halfheight = offsetMaxs[2] - radius;

	topOrigin[0] = center[0];
	topOrigin[1] = center[1];
	topOrigin[2] = center[2] + halfheight;

	bottomOrigin[0] = center[0];
	bottomOrigin[1] = center[1];
	bottomOrigin[2] = center[2] - halfheight;

	if ( topOrigin[2] < startBottom[2] ) {
		if ( !CM_TraceSphereThroughSphere( tw, startBottom, endBottom,
										   topOrigin, radius ) ) {
			return;
		}
		if ( tw->delta[2] >= 0.0f ) {
			return;
		}
	} else if ( startTop[2] < bottomOrigin[2] ) {
		if ( !CM_TraceSphereThroughSphere( tw, startTop, endTop,
										   bottomOrigin, radius ) ) {
			return;
		}
		if ( tw->delta[2] <= 0.0f ) {
			return;
		}
	}

	if ( CM_TraceCylinderThroughCylinder( tw, center, halfheight, radius ) ) {
		if ( topOrigin[2] < endBottom[2] ) {
			if ( startBottom[2] <= topOrigin[2] ) {
				CM_TraceSphereThroughSphere( tw, startBottom, endBottom,
											 topOrigin, radius );
			}
		} else if ( endTop[2] < bottomOrigin[2] &&
					bottomOrigin[2] <= startTop[2] ) {
			CM_TraceSphereThroughSphere( tw, startTop, endTop,
										 bottomOrigin, radius );
		}
	}
}

/* ---- CM_TraceBoundingBoxThroughCapsule  0x004240A0 ---- VERIFIED */
void CM_TraceBoundingBoxThroughCapsule( traceWork_t *tw ) {
	vec3_t center, offsetMaxs;
	float radius;
	int i;

	for ( i = 0; i < 3; i++ ) {
		center[i] = ( cm_boxModel.mins[i] + cm_boxModel.maxs[i] ) * 0.5f;
		offsetMaxs[i] = cm_boxModel.maxs[i] - center[i];
		tw->start[i] -= center[i];
		tw->end[i] -= center[i];
	}

	tw->sphere.use = qtrue;
	radius = offsetMaxs[2] < offsetMaxs[0] ? offsetMaxs[2] : offsetMaxs[0];
	tw->sphere.radius = radius;
	tw->sphere.halfheight = offsetMaxs[2];
	tw->sphere.offset[0] = 0.0f;
	tw->sphere.offset[1] = 0.0f;
	tw->sphere.offset[2] = offsetMaxs[2] - radius;
	tw->sphereExtents[0] = radius;
	tw->sphereExtents[1] = radius;
	tw->sphereExtents[2] = tw->sphere.halfheight;

	CM_TempBoxModel( tw->mins, tw->maxs, box_brush->contents, qfalse );
	CM_TraceThroughBrush( tw, box_brush );
}

/* ---- CM_TraceThroughTree  0x00424230 ---- VERIFIED */
void CM_TraceThroughTree( traceWork_t *tw, int num, float p1f, float p2f,
						  const vec3_t p1, const vec3_t p2 ) {
	const cNode_t *node;
	const cplane_t *plane;
	float t1, t2, offset;
	float idist, frac, frac2;
	vec3_t mid;
	int side;

	if ( tw->trace.fraction <= p1f ) {
		return;
	}

	if ( num < 0 ) {
		CM_TraceThroughLeaf( tw, &cm_leafs[-1 - num] );
		return;
	}

	node = cm_nodes + num;
	plane = node->plane;

	if ( plane->type < 3 ) {
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = tw->maxs[plane->type];
	} else {
		t1 = DotProduct( plane->normal, p1 ) - plane->dist;
		t2 = DotProduct( plane->normal, p2 ) - plane->dist;
		offset = tw->isPoint ? 0.0f : 2048.0f;
	}

	if ( t1 >= offset + 1.0f && t2 >= offset + 1.0f ) {
		CM_TraceThroughTree( tw, node->children[0], p1f, p2f, p1, p2 );
		return;
	}
	if ( t1 < -1.0f - offset && t2 < -1.0f - offset ) {
		CM_TraceThroughTree( tw, node->children[1], p1f, p2f, p1, p2 );
		return;
	}

	if ( t1 < t2 ) {
		idist = 1.0f / ( t1 - t2 );
		side = 1;
		frac2 = ( t1 + offset + SURFACE_CLIP_EPSILON ) * idist;
		frac = ( t1 - offset + SURFACE_CLIP_EPSILON ) * idist;
	} else if ( t2 < t1 ) {
		idist = 1.0f / ( t1 - t2 );
		side = 0;
		frac2 = ( t1 - offset - SURFACE_CLIP_EPSILON ) * idist;
		frac = ( t1 + offset + SURFACE_CLIP_EPSILON ) * idist;
	} else {
		side = 0;
		frac = 1.0f;
		frac2 = 0.0f;
	}

	if ( frac < 0.0f ) {
		frac = 0.0f;
	} else if ( frac > 1.0f ) {
		frac = 1.0f;
	}

	mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac;
	mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac;
	mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac;

	CM_TraceThroughTree( tw, node->children[side], p1f,
						 p1f + ( p2f - p1f ) * frac, p1, mid );

	if ( frac2 < 0.0f ) {
		frac2 = 0.0f;
	} else if ( frac2 > 1.0f ) {
		frac2 = 1.0f;
	}

	mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac2;
	mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac2;
	mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac2;

	CM_TraceThroughTree( tw, node->children[side ^ 1],
						 p1f + ( p2f - p1f ) * frac2, p2f, mid, p2 );
}

/* ---- CM_Trace  0x00424530 ---- VERIFIED */
void CM_Trace( trace_t *results, const vec3_t start, const vec3_t end,
			   const vec3_t mins, const vec3_t maxs, clipHandle_t model,
			   const vec3_t origin, int brushmask, qboolean capsule,
			   const cmSphereRecord_t *sphere ) {
	cmodel_t *cmod;
	traceWork_t tw;
	vec3_t offset;
	float t;
	int i;

	( void )origin;

	cmod = CM_ClipHandleToModel( model );

	cm_checkcount++;
	Com_Memset( &tw, 0, sizeof( tw ) );
	tw.trace.fraction = results->fraction;

	if ( !mins ) {
		mins = vec3_origin;
	}
	if ( !maxs ) {
		maxs = vec3_origin;
	}

	tw.contents = brushmask;

	for ( i = 0; i < 3; i++ ) {
		offset[i] = ( mins[i] + maxs[i] ) * 0.5f;
		tw.mins[i] = mins[i] - offset[i];
		tw.maxs[i] = maxs[i] - offset[i];
		tw.start[i] = start[i] + offset[i];
		tw.end[i] = end[i] + offset[i];
		tw.delta[i] = tw.end[i] - tw.start[i];
	}
	tw.deltaLengthSquared = DotProduct( tw.delta, tw.delta );

	if ( sphere ) {
		tw.sphere = sphere->sphere;
		VectorCopy( sphere->extents, tw.sphereExtents );
	} else {
		tw.sphere.use = capsule;
		tw.sphere.radius = tw.maxs[0] <= tw.maxs[2] ? tw.maxs[0] : tw.maxs[2];
		tw.sphere.halfheight = tw.maxs[2];
		tw.sphere.offset[0] = 0.0f;
		tw.sphere.offset[1] = 0.0f;
		tw.sphere.offset[2] = tw.maxs[2] - tw.sphere.radius;
	}

	tw.maxsSum = tw.maxs[0] + tw.maxs[1] + tw.maxs[2];

	tw.offsets[0][0] = tw.mins[0];  tw.offsets[0][1] = tw.mins[1];  tw.offsets[0][2] = tw.mins[2];
	tw.offsets[1][0] = tw.maxs[0];  tw.offsets[1][1] = tw.mins[1];  tw.offsets[1][2] = tw.mins[2];
	tw.offsets[2][0] = tw.mins[0];  tw.offsets[2][1] = tw.maxs[1];  tw.offsets[2][2] = tw.mins[2];
	tw.offsets[3][0] = tw.maxs[0];  tw.offsets[3][1] = tw.maxs[1];  tw.offsets[3][2] = tw.mins[2];
	tw.offsets[4][0] = tw.mins[0];  tw.offsets[4][1] = tw.mins[1];  tw.offsets[4][2] = tw.maxs[2];
	tw.offsets[5][0] = tw.maxs[0];  tw.offsets[5][1] = tw.mins[1];  tw.offsets[5][2] = tw.maxs[2];
	tw.offsets[6][0] = tw.mins[0];  tw.offsets[6][1] = tw.maxs[1];  tw.offsets[6][2] = tw.maxs[2];
	tw.offsets[7][0] = tw.maxs[0];  tw.offsets[7][1] = tw.maxs[1];  tw.offsets[7][2] = tw.maxs[2];

	if ( tw.sphere.use ) {
		for ( i = 0; i < 3; i++ ) {
			t = ( float )fabs( tw.sphere.offset[i] );

			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] - t - tw.sphere.radius;
				tw.bounds[1][i] = t + tw.end[i] + tw.sphere.radius;
			} else {
				tw.bounds[0][i] = tw.end[i] - t - tw.sphere.radius;
				tw.bounds[1][i] = t + tw.start[i] + tw.sphere.radius;
			}
		}
	} else {
		for ( i = 0; i < 3; i++ ) {
			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] + tw.mins[i];
				tw.bounds[1][i] = tw.end[i] + tw.maxs[i];
			} else {
				tw.bounds[0][i] = tw.end[i] + tw.mins[i];
				tw.bounds[1][i] = tw.start[i] + tw.maxs[i];
			}
		}
	}

	if ( start[0] == end[0] && start[1] == end[1] && start[2] == end[2] ) {
		if ( model == 0 ) {
			CM_PositionTest( &tw );
		} else if ( model == CM_TEMP_CAPSULE_HANDLE ) {
			if ( tw.contents & box_brush->contents ) {
				if ( tw.sphere.use ) {
					CM_TestCapsuleInCapsule( &tw );
				} else {
					CM_TestBoundingBoxInCapsule( &tw );
				}
			}
		} else {
			CM_TestInLeaf( &tw, &cmod->leaf );
		}
	} else {
		tw.isPoint = ( tw.maxs[0] + tw.maxs[1] + tw.maxs[2] == 0.0f );

		if ( tw.sphere.use ) {
			for ( i = 0; i < 3; i++ ) {
				tw.sphereExtents[i] = tw.sphere.radius +
									  CM_Q_fabs( tw.sphere.offset[i] );
			}
		}

		if ( model == 0 ) {
			CM_TraceThroughTree( &tw, 0, 0.0f, tw.trace.fraction,
								 tw.start, tw.end );
		} else if ( model == CM_TEMP_CAPSULE_HANDLE ) {
			if ( tw.contents & box_brush->contents ) {
				if ( tw.sphere.use ) {
					CM_TraceCapsuleThroughCapsule( &tw );
				} else {
					CM_TraceBoundingBoxThroughCapsule( &tw );
				}
			}
		} else {
			CM_TraceThroughLeaf( &tw, &cmod->leaf );
		}
	}

	tw.trace.endpos[0] = start[0] + tw.delta[0] * tw.trace.fraction;
	tw.trace.endpos[1] = start[1] + tw.delta[1] * tw.trace.fraction;
	tw.trace.endpos[2] = start[2] + tw.delta[2] * tw.trace.fraction;

	*results = tw.trace;
}

/* ---- CM_BoxTrace  0x00424B50 ---- VERIFIED */
void CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
				  const vec3_t mins, const vec3_t maxs, clipHandle_t model,
				  int brushmask, qboolean capsule ) {
	results->fraction = 1.0f;
	CM_Trace( results, start, end, mins, maxs, model, vec3_origin,
			  brushmask, capsule, NULL );
}

/* ---- CM_TransformedBoxTrace  0x00424B80 ---- VERIFIED */
void CM_TransformedBoxTrace( trace_t *results, const vec3_t start,
							 const vec3_t end, const vec3_t mins,
							 const vec3_t maxs, clipHandle_t model,
							 int brushmask, const vec3_t origin,
							 const vec3_t angles, qboolean capsule ) {
	vec3_t start_l, end_l, offset;
	vec3_t symMins, symMaxs;
	vec3_t matrix[3], transpose[3];
	qboolean rotated;
	float cylinderOffset;
	cmSphereRecord_t sphere;
	trace_t trace;
	int i;

	Com_Memset( &sphere, 0, sizeof( sphere ) );

	if ( !mins ) {
		mins = vec3_origin;
	}
	if ( !maxs ) {
		maxs = vec3_origin;
	}

	for ( i = 0; i < 3; i++ ) {
		offset[i] = ( mins[i] + maxs[i] ) * 0.5f;
		symMins[i] = mins[i] - offset[i];
		symMaxs[i] = maxs[i] - offset[i];
		start_l[i] = start[i] + offset[i] - origin[i];
		end_l[i] = end[i] + offset[i] - origin[i];
	}

	if ( model == CM_TEMP_BOX_HANDLE ||
		 ( angles[0] == 0.0f && angles[1] == 0.0f && angles[2] == 0.0f ) ) {
		rotated = qfalse;
	} else {
		rotated = qtrue;
	}

	sphere.sphere.use = capsule;
	sphere.sphere.radius = symMaxs[2] < symMaxs[0] ? symMaxs[2] : symMaxs[0];
	sphere.sphere.halfheight = symMaxs[2];
	cylinderOffset = symMaxs[2] - sphere.sphere.radius;

	if ( rotated ) {
		CreateRotationMatrix( angles, matrix );
		RotatePoint( start_l, matrix );
		RotatePoint( end_l, matrix );
		sphere.sphere.offset[0] = matrix[0][2] * cylinderOffset;
		sphere.sphere.offset[1] = -matrix[1][2] * cylinderOffset;
		sphere.sphere.offset[2] = matrix[2][2] * cylinderOffset;
	} else {
		sphere.sphere.offset[0] = 0.0f;
		sphere.sphere.offset[1] = 0.0f;
		sphere.sphere.offset[2] = cylinderOffset;
	}

	trace.fraction = results->fraction;
	CM_Trace( &trace, start_l, end_l, symMins, symMaxs, model, origin,
			  brushmask, capsule, &sphere );

	if ( rotated && trace.fraction < results->fraction ) {
		TransposeMatrix( matrix, transpose );
		RotatePoint( trace.normal, transpose );
	}

	trace.endpos[0] = start[0] + ( end[0] - start[0] ) * trace.fraction;
	trace.endpos[1] = start[1] + ( end[1] - start[1] ) * trace.fraction;
	trace.endpos[2] = start[2] + ( end[2] - start[2] ) * trace.fraction;

	*results = trace;
}

/* ---- CM_TransformedBoxTraceExternal  0x00425040 ---- VERIFIED */
void CM_TransformedBoxTraceExternal( trace_t *results, const vec3_t start,
									 const vec3_t end, const vec3_t mins,
									 const vec3_t maxs, clipHandle_t model,
									 int brushmask, const vec3_t origin,
									 const vec3_t angles, qboolean capsule ) {
	results->fraction = 1.0f;
	CM_TransformedBoxTrace( results, start, end, mins, maxs, model, brushmask,
							origin, angles, capsule );
}

/* ---- CM_SightTraceThroughPatch  0x00425080 ---- VERIFIED */
qboolean CM_SightTraceThroughPatch( const traceWork_t *tw,
									const cPatch_t *patch ) {
	if ( !CM_TraceMayIntersectBox( tw, patch->mins, patch->maxs ) ) {
		return qtrue;
	}

	if ( patch->pc ) {
		return CM_SightTraceThroughPatchCollide( ( traceWork_t * )tw,
												 patch->pc );
	}
	return CM_SightTraceThroughTerrainCollide( ( traceWork_t * )tw,
											   patch->tc );
}

/* ---- CM_SightTraceThroughBrush  0x004250C0 ---- VERIFIED */
static int CM_SightTraceThroughBrush( const traceWork_t *tw,
									  const cbrush_t *brush ) {
	float enterFrac, leaveFrac;
	const cbrushside_t *side;
	const cplane_t *plane;
	const float *offset;
	const float *bounds;
	vec3_t startPoint, endPoint;
	float startDist, endDist, denom;
	float planeDist, sphereOffset, workOffset, sign;
	int pass, axis, i;

	enterFrac = 0.0f;
	leaveFrac = 1.0f;

	for ( pass = 0; pass <= 1; pass++ ) {
		sign = ( pass == 0 ) ? -1.0f : 1.0f;
		bounds = ( pass == 0 ) ? brush->mins : brush->maxs;

		for ( axis = 0; axis <= 2; axis++ ) {
			if ( tw->sphere.use ) {
				workOffset = tw->sphereExtents[axis];
			} else {
				workOffset = tw->maxs[axis];
			}

			startDist = ( tw->start[axis] - bounds[axis] ) * sign - workOffset;
			endDist = ( tw->end[axis] - bounds[axis] ) * sign - workOffset;

			if ( startDist > 0.0f ) {
				if ( endDist > 0.0f ) {
					return 0;
				}
				denom = startDist - endDist;
				if ( enterFrac * denom < startDist ) {
					enterFrac = startDist / denom;
					if ( leaveFrac <= enterFrac ) {
						return 0;
					}
				}
			} else if ( endDist > 0.0f ) {
				denom = startDist - endDist;
				if ( leaveFrac * denom < startDist ) {
					leaveFrac = startDist / denom;
					if ( leaveFrac <= enterFrac ) {
						return 0;
					}
				}
			}
		}
	}

	side = brush->sides;
	for ( i = brush->numSides; i != 0; i--, side++ ) {
		plane = side->plane;

		if ( tw->sphere.use ) {
			planeDist = plane->dist + tw->sphere.radius;
			sphereOffset = DotProduct( plane->normal, tw->sphere.offset );

			if ( sphereOffset > 0.0f ) {
				VectorSubtract( tw->start, tw->sphere.offset, startPoint );
				VectorSubtract( tw->end, tw->sphere.offset, endPoint );
			} else {
				VectorAdd( tw->start, tw->sphere.offset, startPoint );
				VectorAdd( tw->end, tw->sphere.offset, endPoint );
			}

			startDist = DotProduct( startPoint, plane->normal ) - planeDist;
			endDist = DotProduct( endPoint, plane->normal ) - planeDist;
		} else {
			offset = tw->offsets[plane->signbits];
			planeDist = plane->dist - DotProduct( offset, plane->normal );
			startDist = DotProduct( tw->start, plane->normal ) - planeDist;
			endDist = DotProduct( tw->end, plane->normal ) - planeDist;
		}

		if ( startDist > 0.0f ) {
			if ( endDist > 0.0f ) {
				return 0;
			}
			denom = startDist - endDist;
			if ( enterFrac * denom < startDist ) {
				enterFrac = startDist / denom;
				if ( leaveFrac <= enterFrac ) {
					return 0;
				}
			}
		} else if ( endDist > 0.0f ) {
			denom = startDist - endDist;
			if ( leaveFrac * denom < startDist ) {
				leaveFrac = startDist / denom;
				if ( leaveFrac <= enterFrac ) {
					return 0;
				}
			}
		}
	}

	return ( int )( brush - cm_brushes ) + 1;
}

/* ---- CM_SightTraceThroughLeaf  0x004255E0 ---- VERIFIED */
int CM_SightTraceThroughLeaf( const traceWork_t *tw, const cLeaf_t *leaf ) {
	int i, hit;
	int brushnum, patchnum;
	cbrush_t *b;
	cPatch_t *patch;

	for ( i = 0; i < ( int )leaf->numLeafBrushes; i++ ) {
		brushnum = cm_leafbrushes[leaf->firstLeafBrush + i];
		b = &cm_brushes[brushnum];

		if ( b->checkcount == cm_checkcount ) {
			continue;
		}
		b->checkcount = cm_checkcount;

		if ( !( tw->contents & b->contents ) ) {
			continue;
		}

		hit = CM_SightTraceThroughBrush( tw, b );
		if ( hit ) {
			return hit;
		}
	}

	if ( cm_noCurves->integer ) {
		return 0;
	}

	for ( i = 0; i < ( int )leaf->numLeafSurfaces; i++ ) {
		patchnum = cm_leafsurfaces[leaf->firstLeafSurface + i];
		patch = &cm_patches[patchnum];

		if ( patch->checkcount == cm_checkcount ) {
			continue;
		}
		patch->checkcount = cm_checkcount;

		if ( !( tw->contents & patch->contents ) ) {
			continue;
		}

		if ( !CM_SightTraceThroughPatch( tw, patch ) ) {
			return cm_numBrushes + ( int )( patch - cm_patches ) + 1;
		}
	}

	return 0;
}

/* ---- CM_SightTraceSphereThroughSphere  0x00425720 ---- VERIFIED */
static qboolean CM_SightTraceSphereThroughSphere( const traceWork_t *tw,
												  const vec3_t start,
												  const vec3_t end,
												  const vec3_t sphereOrigin,
												  float sphereRadius ) {
	vec3_t delta, normal;
	float radiusSquared, startDist;
	float deltaDot, disc, dist, baseFrac, enterFrac;

	( void )end;

	VectorSubtract( start, sphereOrigin, delta );

	radiusSquared = ( sphereRadius + tw->sphere.radius ) *
					( sphereRadius + tw->sphere.radius );
	startDist = DotProduct( delta, delta ) - radiusSquared;

	if ( startDist <= 0.0f ) {
		return qfalse;
	}

	deltaDot = DotProduct( tw->delta, delta );
	if ( deltaDot >= 0.0f ) {
		return qtrue;
	}

	disc = deltaDot * deltaDot - tw->deltaLengthSquared * startDist;
	if ( disc < 0.0f ) {
		return qtrue;
	}

	dist = VectorNormalize2( delta, normal );
	baseFrac = ( -deltaDot - ( float )sqrt( ( double )disc ) ) /
			   tw->deltaLengthSquared;
	enterFrac = baseFrac + ( deltaDot * SURFACE_CLIP_EPSILON ) / dist;

	if ( enterFrac < tw->trace.fraction ) {
		return qfalse;
	}
	return qtrue;
}

/* ---- CM_SightTraceCylinderThroughCylinder  0x00425840 ---- VERIFIED */
static qboolean CM_SightTraceCylinderThroughCylinder( const traceWork_t *tw,
													  const vec3_t origin,
													  float halfheightIn,
													  float radiusIn ) {
	vec3_t delta, normal;
	float radiusSquared, startDist, halfheight;
	float deltaDot, disc, dist, epsFrac, baseFrac, hitFrac, z;

	VectorSubtract( tw->start, origin, delta );

	radiusSquared = ( radiusIn + tw->sphere.radius ) *
					( radiusIn + tw->sphere.radius );
	startDist = ( delta[0] * delta[0] + delta[1] * delta[1] ) - radiusSquared;

	if ( startDist <= 0.0f ) {
		halfheight = tw->sphere.halfheight - tw->sphere.radius + halfheightIn;

		if ( delta[2] > halfheight || delta[2] < -halfheight ) {
			return qtrue;
		}
		return qfalse;
	}

	deltaDot = tw->delta[0] * delta[0] + tw->delta[1] * delta[1];
	if ( deltaDot >= 0.0f ) {
		return qtrue;
	}

	disc = deltaDot * deltaDot - tw->deltaLengthSquared * startDist;
	if ( disc < 0.0f ) {
		return qtrue;
	}

	delta[2] = 0.0f;
	dist = VectorNormalize2( delta, normal );
	epsFrac = ( deltaDot * SURFACE_CLIP_EPSILON ) / dist;
	baseFrac = ( -deltaDot - ( float )sqrt( ( double )disc ) ) /
			   tw->deltaLengthSquared;
	hitFrac = baseFrac + epsFrac;

	if ( !( tw->trace.fraction > hitFrac ) ) {
		return qtrue;
	}

	halfheight = tw->sphere.halfheight - tw->sphere.radius + halfheightIn;
	z = ( ( hitFrac - epsFrac ) * tw->delta[2] + tw->start[2] ) - origin[2];

	if ( z > halfheight || z < -halfheight ) {
		return qtrue;
	}

	return qfalse;
}

/* ---- CM_SightTraceCapsuleThroughCapsule  0x004259C0 ---- VERIFIED */
int CM_SightTraceCapsuleThroughCapsule( const traceWork_t *tw ) {
	vec3_t startTop, startBottom, endTop, endBottom;
	vec3_t center, offsetMaxs, topOrigin, bottomOrigin;
	float radius, halfheight;
	int i;

	if ( tw->bounds[0][0] > cm_boxModel.maxs[0] + 1.0f ) {
		return 0;
	}
	if ( tw->bounds[0][1] > cm_boxModel.maxs[1] + 1.0f ) {
		return 0;
	}
	if ( tw->bounds[0][2] > cm_boxModel.maxs[2] + 1.0f ) {
		return 0;
	}
	if ( cm_boxModel.mins[0] - 1.0f > tw->bounds[1][0] ) {
		return 0;
	}
	if ( cm_boxModel.mins[1] - 1.0f > tw->bounds[1][1] ) {
		return 0;
	}
	if ( cm_boxModel.mins[2] - 1.0f > tw->bounds[1][2] ) {
		return 0;
	}

	VectorAdd( tw->start, tw->sphere.offset, startTop );
	VectorSubtract( tw->start, tw->sphere.offset, startBottom );
	VectorAdd( tw->end, tw->sphere.offset, endTop );
	VectorSubtract( tw->end, tw->sphere.offset, endBottom );

	for ( i = 0; i < 3; i++ ) {
		center[i] = ( cm_boxModel.mins[i] + cm_boxModel.maxs[i] ) * 0.5f;
		offsetMaxs[i] = cm_boxModel.maxs[i] - center[i];
	}

	radius = offsetMaxs[2] < offsetMaxs[0] ? offsetMaxs[2] : offsetMaxs[0];
	halfheight = offsetMaxs[2] - radius;

	topOrigin[0] = center[0];
	topOrigin[1] = center[1];
	topOrigin[2] = center[2] + halfheight;

	bottomOrigin[0] = center[0];
	bottomOrigin[1] = center[1];
	bottomOrigin[2] = center[2] - halfheight;

	if ( topOrigin[2] < startBottom[2] ) {
		if ( !CM_SightTraceSphereThroughSphere( tw, startBottom, endBottom,
												topOrigin, radius ) ) {
			return -1;
		}
		if ( tw->delta[2] >= 0.0f ) {
			return 0;
		}
	} else if ( startTop[2] < bottomOrigin[2] ) {
		if ( !CM_SightTraceSphereThroughSphere( tw, startTop, endTop,
												bottomOrigin, radius ) ) {
			return -1;
		}
		if ( tw->delta[2] <= 0.0f ) {
			return 0;
		}
	}

	if ( !CM_SightTraceCylinderThroughCylinder( tw, center, halfheight,
												radius ) ) {
		return -1;
	}

	if ( topOrigin[2] < endBottom[2] ) {
		if ( startBottom[2] <= topOrigin[2] &&
			 !CM_SightTraceSphereThroughSphere( tw, startBottom, endBottom,
												topOrigin, radius ) ) {
			return -1;
		}
	} else if ( endTop[2] < bottomOrigin[2] &&
				bottomOrigin[2] <= startTop[2] &&
				!CM_SightTraceSphereThroughSphere( tw, startTop, endTop,
												   bottomOrigin, radius ) ) {
		return -1;
	}

	return 0;
}

/* ---- CM_SightTraceBoundingBoxThroughCapsule  0x00425CA0 ---- VERIFIED */
int CM_SightTraceBoundingBoxThroughCapsule( traceWork_t *tw ) {
	vec3_t center, offsetMaxs;
	float radius;
	int i;

	for ( i = 0; i < 3; i++ ) {
		center[i] = ( cm_boxModel.mins[i] + cm_boxModel.maxs[i] ) * 0.5f;
		offsetMaxs[i] = cm_boxModel.maxs[i] - center[i];
		tw->start[i] -= center[i];
		tw->end[i] -= center[i];
	}

	tw->sphere.use = qtrue;
	radius = offsetMaxs[2] < offsetMaxs[0] ? offsetMaxs[2] : offsetMaxs[0];
	tw->sphere.radius = radius;
	tw->sphere.halfheight = offsetMaxs[2];
	tw->sphere.offset[0] = 0.0f;
	tw->sphere.offset[1] = 0.0f;
	tw->sphere.offset[2] = offsetMaxs[2] - radius;
	tw->sphereExtents[0] = radius;
	tw->sphereExtents[1] = radius;
	tw->sphereExtents[2] = tw->sphere.halfheight;

	CM_TempBoxModel( tw->mins, tw->maxs, box_brush->contents, qfalse );
	return CM_SightTraceThroughBrush( tw, box_brush );
}

/* ---- CM_SightTraceThroughTree  0x00425E30 ---- VERIFIED */
int CM_SightTraceThroughTree( const traceWork_t *tw, int num, float p1f,
							  float p2f, const vec3_t p1, const vec3_t p2 ) {
	const cNode_t *node;
	const cplane_t *plane;
	float t1, t2, offset;
	float idist, frac, frac2;
	vec3_t mid;
	int side, hit;

	if ( num < 0 ) {
		return CM_SightTraceThroughLeaf( tw, &cm_leafs[-1 - num] );
	}

	node = cm_nodes + num;
	plane = node->plane;

	if ( plane->type < 3 ) {
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = tw->maxs[plane->type];
	} else {
		t1 = DotProduct( plane->normal, p1 ) - plane->dist;
		t2 = DotProduct( plane->normal, p2 ) - plane->dist;
		offset = tw->isPoint ? 0.0f : 2048.0f;
	}

	if ( t1 >= offset + 1.0f && t2 >= offset + 1.0f ) {
		return CM_SightTraceThroughTree( tw, node->children[0], p1f, p2f,
										 p1, p2 );
	}
	if ( t1 < -1.0f - offset && t2 < -1.0f - offset ) {
		return CM_SightTraceThroughTree( tw, node->children[1], p1f, p2f,
										 p1, p2 );
	}

	if ( t1 < t2 ) {
		idist = 1.0f / ( t1 - t2 );
		side = 1;
		frac = ( t1 + offset + SURFACE_CLIP_EPSILON ) * idist;
		frac2 = ( t1 - offset + SURFACE_CLIP_EPSILON ) * idist;
	} else if ( t2 < t1 ) {
		idist = 1.0f / ( t1 - t2 );
		side = 0;
		frac = ( t1 - offset - SURFACE_CLIP_EPSILON ) * idist;
		frac2 = ( t1 + offset + SURFACE_CLIP_EPSILON ) * idist;
	} else {
		side = 0;
		frac = 0.0f;
		frac2 = 1.0f;
	}

	if ( frac2 < 0.0f ) {
		frac2 = 0.0f;
	} else if ( frac2 > 1.0f ) {
		frac2 = 1.0f;
	}

	mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac2;
	mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac2;
	mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac2;

	hit = CM_SightTraceThroughTree( tw, node->children[side], p1f,
									p1f + ( p2f - p1f ) * frac2, p1, mid );
	if ( hit ) {
		return hit;
	}

	if ( frac < 0.0f ) {
		frac = 0.0f;
	} else if ( frac > 1.0f ) {
		frac = 1.0f;
	}

	mid[0] = p1[0] + ( p2[0] - p1[0] ) * frac;
	mid[1] = p1[1] + ( p2[1] - p1[1] ) * frac;
	mid[2] = p1[2] + ( p2[2] - p1[2] ) * frac;

	return CM_SightTraceThroughTree( tw, node->children[side ^ 1],
									 p1f + ( p2f - p1f ) * frac, p2f,
									 mid, p2 );
}

/* ---- CM_SightTrace  0x00426130 ---- VERIFIED */
int CM_SightTrace( int oldHitNum, const vec3_t start, const vec3_t end,
				   const vec3_t mins, const vec3_t maxs, clipHandle_t model,
				   const vec3_t origin, int brushmask, qboolean capsule,
				   const cmSphereRecord_t *sphere ) {
	cmodel_t *cmod;
	traceWork_t tw;
	vec3_t offset;
	float t;
	int i, hit, index;

	( void )origin;

	cmod = CM_ClipHandleToModel( model );

	cm_checkcount++;
	Com_Memset( &tw, 0, sizeof( tw ) );
	tw.trace.fraction = 1.0f;

	if ( !mins ) {
		mins = vec3_origin;
	}
	if ( !maxs ) {
		maxs = vec3_origin;
	}

	tw.contents = brushmask;

	for ( i = 0; i < 3; i++ ) {
		offset[i] = ( mins[i] + maxs[i] ) * 0.5f;
		tw.mins[i] = mins[i] - offset[i];
		tw.maxs[i] = maxs[i] - offset[i];
		tw.start[i] = start[i] + offset[i];
		tw.end[i] = end[i] + offset[i];
		tw.delta[i] = tw.end[i] - tw.start[i];
	}
	tw.deltaLengthSquared = DotProduct( tw.delta, tw.delta );

	if ( sphere ) {
		tw.sphere = sphere->sphere;
		VectorCopy( sphere->extents, tw.sphereExtents );
	} else {
		tw.sphere.use = capsule;
		tw.sphere.radius = tw.maxs[0] <= tw.maxs[2] ? tw.maxs[0] : tw.maxs[2];
		tw.sphere.halfheight = tw.maxs[2];
		tw.sphere.offset[0] = 0.0f;
		tw.sphere.offset[1] = 0.0f;
		tw.sphere.offset[2] = tw.maxs[2] - tw.sphere.radius;
	}

	tw.maxsSum = tw.maxs[0] + tw.maxs[1] + tw.maxs[2];

	tw.offsets[0][0] = tw.mins[0];  tw.offsets[0][1] = tw.mins[1];  tw.offsets[0][2] = tw.mins[2];
	tw.offsets[1][0] = tw.maxs[0];  tw.offsets[1][1] = tw.mins[1];  tw.offsets[1][2] = tw.mins[2];
	tw.offsets[2][0] = tw.mins[0];  tw.offsets[2][1] = tw.maxs[1];  tw.offsets[2][2] = tw.mins[2];
	tw.offsets[3][0] = tw.maxs[0];  tw.offsets[3][1] = tw.maxs[1];  tw.offsets[3][2] = tw.mins[2];
	tw.offsets[4][0] = tw.mins[0];  tw.offsets[4][1] = tw.mins[1];  tw.offsets[4][2] = tw.maxs[2];
	tw.offsets[5][0] = tw.maxs[0];  tw.offsets[5][1] = tw.mins[1];  tw.offsets[5][2] = tw.maxs[2];
	tw.offsets[6][0] = tw.mins[0];  tw.offsets[6][1] = tw.maxs[1];  tw.offsets[6][2] = tw.maxs[2];
	tw.offsets[7][0] = tw.maxs[0];  tw.offsets[7][1] = tw.maxs[1];  tw.offsets[7][2] = tw.maxs[2];

	if ( tw.sphere.use ) {
		for ( i = 0; i < 3; i++ ) {
			t = ( float )fabs( tw.sphere.offset[i] );

			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] - t - tw.sphere.radius;
				tw.bounds[1][i] = t + tw.end[i] + tw.sphere.radius;
			} else {
				tw.bounds[0][i] = tw.end[i] - t - tw.sphere.radius;
				tw.bounds[1][i] = t + tw.start[i] + tw.sphere.radius;
			}
		}
	} else {
		for ( i = 0; i < 3; i++ ) {
			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] + tw.mins[i];
				tw.bounds[1][i] = tw.end[i] + tw.maxs[i];
			} else {
				tw.bounds[0][i] = tw.end[i] + tw.mins[i];
				tw.bounds[1][i] = tw.start[i] + tw.maxs[i];
			}
		}
	}

	tw.isPoint = ( tw.maxs[0] + tw.maxs[1] + tw.maxs[2] == 0.0f );

	if ( tw.sphere.use ) {
		for ( i = 0; i < 3; i++ ) {
			tw.sphereExtents[i] = tw.sphere.radius +
								  CM_Q_fabs( tw.sphere.offset[i] );
		}
	}

	if ( model == 0 ) {
		hit = 0;

		if ( oldHitNum > 0 ) {
			index = oldHitNum - 1;

			if ( index < cm_numBrushes ) {
				hit = CM_SightTraceThroughBrush( &tw, &cm_brushes[index] );
			} else if ( index - cm_numBrushes < cm_numPatches ) {
				hit = CM_SightTraceThroughPatch(
						  &tw, &cm_patches[index - cm_numBrushes] );
			}
		}

		if ( !hit ) {
			hit = CM_SightTraceThroughTree( &tw, 0, 0.0f, 1.0f,
											tw.start, tw.end );
		}
		return hit;
	}

	if ( model != CM_TEMP_CAPSULE_HANDLE ) {
		return CM_SightTraceThroughLeaf( &tw, &cmod->leaf );
	}

	if ( !( tw.contents & box_brush->contents ) ) {
		return 0;
	}
	if ( tw.sphere.use ) {
		return CM_SightTraceCapsuleThroughCapsule( &tw );
	}
	return CM_SightTraceBoundingBoxThroughCapsule( &tw );
}

/* ---- CM_BoxSightTrace  0x00426720 ---- VERIFIED */
int CM_BoxSightTrace( int oldHitNum, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs, clipHandle_t model,
					  int brushmask, qboolean capsule ) {
	return CM_SightTrace( oldHitNum, start, end, mins, maxs, model,
						  vec3_origin, brushmask, capsule, NULL );
}

/* ---- CM_TransformedBoxSightTrace  0x00426750 ---- VERIFIED */
int CM_TransformedBoxSightTrace( int oldHitNum, const vec3_t start,
								 const vec3_t end, const vec3_t mins,
								 const vec3_t maxs, clipHandle_t model,
								 int brushmask, const vec3_t origin,
								 const vec3_t angles, qboolean capsule ) {
	vec3_t start_l, end_l, offset;
	vec3_t symMins, symMaxs;
	vec3_t matrix[3];
	qboolean rotated;
	float cylinderOffset;
	cmSphereRecord_t sphere;
	int i;

	Com_Memset( &sphere, 0, sizeof( sphere ) );

	if ( !mins ) {
		mins = vec3_origin;
	}
	if ( !maxs ) {
		maxs = vec3_origin;
	}

	for ( i = 0; i < 3; i++ ) {
		offset[i] = ( mins[i] + maxs[i] ) * 0.5f;
		symMins[i] = mins[i] - offset[i];
		symMaxs[i] = maxs[i] - offset[i];
		start_l[i] = start[i] + offset[i] - origin[i];
		end_l[i] = end[i] + offset[i] - origin[i];
	}

	if ( model == CM_TEMP_BOX_HANDLE ||
		 ( angles[0] == 0.0f && angles[1] == 0.0f && angles[2] == 0.0f ) ) {
		rotated = qfalse;
	} else {
		rotated = qtrue;
	}

	sphere.sphere.use = capsule;
	sphere.sphere.radius = symMaxs[2] < symMaxs[0] ? symMaxs[2] : symMaxs[0];
	sphere.sphere.halfheight = symMaxs[2];
	cylinderOffset = symMaxs[2] - sphere.sphere.radius;

	if ( rotated ) {
		CreateRotationMatrix( angles, matrix );
		RotatePoint( start_l, matrix );
		RotatePoint( end_l, matrix );
		sphere.sphere.offset[0] = matrix[0][2] * cylinderOffset;
		sphere.sphere.offset[1] = -matrix[1][2] * cylinderOffset;
		sphere.sphere.offset[2] = matrix[2][2] * cylinderOffset;
	} else {
		sphere.sphere.offset[0] = 0.0f;
		sphere.sphere.offset[1] = 0.0f;
		sphere.sphere.offset[2] = cylinderOffset;
	}

	return CM_SightTrace( oldHitNum, start_l, end_l, symMins, symMaxs, model,
						  origin, brushmask, capsule, &sphere );
}
