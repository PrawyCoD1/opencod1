/*
 * qcommon/cm_polylib.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_polylib.c
 *
 * Retail range 0x0041DE20-0x0041F758, 17 functions.
 *
 * @fidelity: likely
 */

#include "qcommon.h"
#include "cm_polylib.h"

#define MAX_POINTS_ON_WINDING   64
#define MAX_HULL_POINTS         128

#define WINDING_HUGE            131072.0f

#define ON_EPSILON              0.1f

#define WINDING_FREED           ( (int)0xDEADDEAD )

#define SIDE_FRONT              0
#define SIDE_BACK               1
#define SIDE_ON                 2
#define SIDE_CROSS              ( -2 )

extern int c_active_windings;
extern int c_peak_windings;

void *Z_MallocInternal( int size );
vec_t VectorNormalize2( const vec3_t v, vec3_t out );

/* ---- pw  0x0041DE20 ---- VERIFIED */
void pw( winding_t *w ) {
	int i;

	for ( i = 0; i < w->numpoints; i++ ) {
		printf( "(%5.1f, %5.1f, %5.1f)\n", w->p[i][0], w->p[i][1], w->p[i][2] );
	}
}

/* ---- AllocWinding  0x0041DE60 ---- VERIFIED */
winding_t *AllocWinding( int points ) {
	c_active_windings++;
	if ( c_active_windings > c_peak_windings ) {
		c_peak_windings = c_active_windings;
	}

	return ( winding_t * )Z_MallocInternal( sizeof( int ) +
											points * sizeof( vec3_t ) );
}

/* ---- FreeWinding  0x0041DE90 ---- VERIFIED */
void FreeWinding( winding_t *w ) {
	if ( w->numpoints == WINDING_FREED ) {
		Com_Error( ERR_FATAL, "\x15" "FreeWinding: freed a freed winding" );
	}
	w->numpoints = WINDING_FREED;
	c_active_windings--;
	free( w );
}

/* ---- RemoveColinearPoints  0x0041DED0 ---- VERIFIED */
void RemoveColinearPoints( winding_t *w ) {
	int i, j, k;
	vec3_t v1, v2;
	int nump;
	vec3_t p[MAX_POINTS_ON_WINDING];

	nump = 0;
	for ( i = 0; i < w->numpoints; i++ ) {
		j = ( i + 1 ) % w->numpoints;
		k = ( i + w->numpoints - 1 ) % w->numpoints;

		VectorSubtract( w->p[j], w->p[i], v1 );
		VectorSubtract( w->p[i], w->p[k], v2 );
		VectorNormalize2( v1, v1 );
		VectorNormalize2( v2, v2 );

		if ( DotProduct( v1, v2 ) < 0.999 ) {
			VectorCopy( w->p[i], p[nump] );
			nump++;
		}
	}

	if ( nump == w->numpoints ) {
		return;
	}

	w->numpoints = nump;
	Com_Memcpy( w->p, p, nump * sizeof( p[0] ) );
}

/* ---- WindingPlane  0x0041E020 ---- VERIFIED */
void WindingPlane( const winding_t *w, vec3_t normal, vec_t *dist ) {
	vec3_t v1, v2;

	VectorSubtract( w->p[1], w->p[0], v1 );
	VectorSubtract( w->p[2], w->p[0], v2 );

	normal[0] = v2[1] * v1[2] - v2[2] * v1[1];
	normal[1] = v2[2] * v1[0] - v2[0] * v1[2];
	normal[2] = v2[0] * v1[1] - v2[1] * v1[0];

	VectorNormalize2( normal, normal );
	*dist = DotProduct( w->p[0], normal );
}

/* ---- WindingArea  0x0041E0D0 ---- VERIFIED */
float WindingArea( const winding_t *w ) {
	int i;
	vec3_t d1, d2, cross;
	float total;

	total = 0.0f;
	for ( i = 2; i < w->numpoints; i++ ) {
		VectorSubtract( w->p[i - 1], w->p[0], d1 );
		VectorSubtract( w->p[i], w->p[0], d2 );
		CrossProduct( d1, d2, cross );
		total += 0.5f * ( float )sqrt( cross[0] * cross[0] +
									   cross[1] * cross[1] +
									   cross[2] * cross[2] );
	}

	return total;
}

/* ---- WindingBounds  0x0041E1C0 ---- VERIFIED */
void WindingBounds( const winding_t *w, vec3_t mins, vec3_t maxs ) {
	float v;
	int i, j;

	mins[0] = mins[1] = mins[2] = WINDING_HUGE;
	maxs[0] = maxs[1] = maxs[2] = -WINDING_HUGE;

	for ( i = 0; i < w->numpoints; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			v = w->p[i][j];
			if ( v < mins[j] ) {
				mins[j] = v;
			}
			if ( v > maxs[j] ) {
				maxs[j] = v;
			}
		}
	}
}

/* ---- WindingCenter  0x0041E260 ---- VERIFIED */
void WindingCenter( const winding_t *w, vec3_t center ) {
	int i;
	float scale;

	VectorCopy( vec3_origin, center );
	for ( i = 0; i < w->numpoints; i++ ) {
		VectorAdd( w->p[i], center, center );
	}

	scale = 1.0f / ( float )w->numpoints;
	VectorScale( center, scale, center );
}

/* ---- BaseWindingForPlane  0x0041E2D0 ---- VERIFIED */
winding_t *BaseWindingForPlane( const vec3_t normal, vec_t dist ) {
	int i, x;
	vec_t max, v;
	vec3_t org, vright, vup;
	winding_t *w;

	max = -WINDING_HUGE;
	x = -1;
	for ( i = 0; i < 3; i++ ) {
		v = ( vec_t )fabs( normal[i] );
		if ( v > max ) {
			x = i;
			max = v;
		}
	}
	if ( x == -1 ) {
		Com_Error( ERR_DROP, "\x15" "BaseWindingForPlane: no axis found" );
	}

	VectorCopy( vec3_origin, vup );
	switch ( x ) {
	case 0:
	case 1:
		vup[2] = 1.0f;
		break;
	case 2:
		vup[0] = 1.0f;
		break;
	}

	v = DotProduct( vup, normal );
	VectorMA( vup, -v, normal, vup );
	VectorNormalize2( vup, vup );

	VectorScale( normal, dist, org );

	CrossProduct( vup, normal, vright );

	VectorScale( vup, WINDING_HUGE, vup );
	VectorScale( vright, WINDING_HUGE, vright );

	w = AllocWinding( 4 );
	w->numpoints = 4;

	VectorSubtract( org, vright, w->p[0] );
	VectorAdd( w->p[0], vup, w->p[0] );

	VectorAdd( org, vright, w->p[1] );
	VectorAdd( w->p[1], vup, w->p[1] );

	VectorAdd( org, vright, w->p[2] );
	VectorSubtract( w->p[2], vup, w->p[2] );

	VectorSubtract( org, vright, w->p[3] );
	VectorSubtract( w->p[3], vup, w->p[3] );

	return w;
}

/* ---- CopyWinding  0x0041E5B0 ---- VERIFIED */
winding_t *CopyWinding( const winding_t *w ) {
	size_t size;
	winding_t *c;

	c = AllocWinding( w->numpoints );
	size = ( size_t )( sizeof( int ) + w->numpoints * sizeof( vec3_t ) );
	Com_Memcpy( c, w, size );

	return c;
}

/* ---- ReverseWinding  0x0041E600 ---- VERIFIED */
winding_t *ReverseWinding( const winding_t *w ) {
	int i;
	winding_t *c;

	c = AllocWinding( w->numpoints );
	for ( i = 0; i < w->numpoints; i++ ) {
		VectorCopy( w->p[w->numpoints - 1 - i], c->p[i] );
	}
	c->numpoints = w->numpoints;

	return c;
}

/* ---- ClipWindingEpsilon  0x0041E680 ---- VERIFIED */
void ClipWindingEpsilon( const winding_t *in, const vec3_t normal, vec_t dist,
						 vec_t epsilon, winding_t **front, winding_t **back ) {
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	vec_t dot;
	int i, j;
	const vec_t *p1;
	const vec_t *p2;
	vec3_t mid;
	winding_t *f;
	winding_t *b;
	int maxpts;

	counts[0] = counts[1] = counts[2] = 0;

	for ( i = 0; i < in->numpoints; i++ ) {
		dot = DotProduct( in->p[i], normal ) - dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		} else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		} else {
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*front = NULL;
	*back = NULL;

	if ( !counts[SIDE_FRONT] ) {
		*back = CopyWinding( in );
		return;
	}
	if ( !counts[SIDE_BACK] ) {
		*front = CopyWinding( in );
		return;
	}

	maxpts = in->numpoints + 4;

	*front = f = AllocWinding( maxpts );
	*back = b = AllocWinding( maxpts );

	for ( i = 0; i < in->numpoints; i++ ) {
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
			VectorCopy( p1, b->p[b->numpoints] );
			b->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
		}
		if ( sides[i] == SIDE_BACK ) {
			VectorCopy( p1, b->p[b->numpoints] );
			b->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		p2 = in->p[( i + 1 ) % in->numpoints];

		dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( j = 0; j < 3; j++ ) {
			if ( normal[j] == 1.0f ) {
				mid[j] = dist;
			} else if ( normal[j] == -1.0f ) {
				mid[j] = -dist;
			} else {
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		VectorCopy( mid, f->p[f->numpoints] );
		f->numpoints++;
		VectorCopy( mid, b->p[b->numpoints] );
		b->numpoints++;
	}

	if ( f->numpoints > maxpts || b->numpoints > maxpts ) {
		Com_Error( ERR_DROP, "\x15" "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING ||
		 b->numpoints > MAX_POINTS_ON_WINDING ) {
		Com_Error( ERR_DROP, "\x15" "ClipWinding: MAX_POINTS_ON_WINDING" );
	}
}

/* ---- ChopWindingInPlace  0x0041EAF0 ---- VERIFIED */
void ChopWindingInPlace( winding_t **inout, const vec3_t normal, vec_t dist,
						 vec_t epsilon ) {
	winding_t *in;
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	vec_t dot;
	int i, j;
	const vec_t *p1;
	const vec_t *p2;
	vec3_t mid;
	winding_t *f;
	int maxpts;

	in = *inout;
	counts[0] = counts[1] = counts[2] = 0;

	for ( i = 0; i < in->numpoints; i++ ) {
		dot = DotProduct( in->p[i], normal ) - dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		} else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		} else {
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if ( !counts[SIDE_FRONT] ) {
		FreeWinding( in );
		*inout = NULL;
		return;
	}
	if ( !counts[SIDE_BACK] ) {
		return;
	}

	maxpts = in->numpoints + 4;
	f = AllocWinding( maxpts );

	for ( i = 0; i < in->numpoints; i++ ) {
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		p2 = in->p[( i + 1 ) % in->numpoints];

		dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( j = 0; j < 3; j++ ) {
			if ( normal[j] == 1.0f ) {
				mid[j] = dist;
			} else if ( normal[j] == -1.0f ) {
				mid[j] = -dist;
			} else {
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		VectorCopy( mid, f->p[f->numpoints] );
		f->numpoints++;
	}

	if ( f->numpoints > maxpts ) {
		Com_Error( ERR_DROP, "\x15" "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING ) {
		Com_Error( ERR_DROP, "\x15" "ClipWinding: MAX_POINTS_ON_WINDING" );
	}

	FreeWinding( in );
	*inout = f;
}

/* ---- ChopWinding  0x0041EE30 ---- VERIFIED */
winding_t *ChopWinding( winding_t *in, const vec3_t normal, vec_t dist ) {
	winding_t *f;
	winding_t *b;

	ClipWindingEpsilon( in, normal, dist, ON_EPSILON, &f, &b );
	FreeWinding( in );
	if ( b ) {
		FreeWinding( b );
	}

	return f;
}

/* ---- CheckWinding  0x0041EED0 ---- VERIFIED */
void CheckWinding( const winding_t *w ) {
	int i, j;
	const vec_t *p1;
	const vec_t *p2;
	vec_t d, edgedist;
	vec3_t dir, edgenormal, facenormal;
	vec_t area;
	vec_t facedist;

	if ( w->numpoints < 3 ) {
		Com_Error( ERR_DROP, "\x15" "CheckWinding: %i points", w->numpoints );
	}

	area = WindingArea( w );
	if ( area < 1.0f ) {
		Com_Error( ERR_DROP, "\x15" "CheckWinding: %f area", area );
	}

	WindingPlane( w, facenormal, &facedist );

	for ( i = 0; i < w->numpoints; i++ ) {
		p1 = w->p[i];

		for ( j = 0; j < 3; j++ ) {
			if ( p1[j] > WINDING_HUGE || p1[j] < -WINDING_HUGE ) {
				Com_Error( ERR_DROP, "\x15" "CheckFace: BUGUS_RANGE: %f",
						   p1[j] );
			}
		}

		j = ( i + 1 == w->numpoints ) ? 0 : i + 1;

		d = DotProduct( p1, facenormal ) - facedist;
		if ( d < -ON_EPSILON || d > ON_EPSILON ) {
			Com_Error( ERR_DROP, "\x15" "CheckWinding: point off plane" );
		}

		p2 = w->p[j];
		VectorSubtract( p2, p1, dir );

		if ( ( vec_t )sqrt( DotProduct( dir, dir ) ) < ON_EPSILON ) {
			Com_Error( ERR_DROP, "\x15" "CheckWinding: degenerate edge" );
		}

		CrossProduct( facenormal, dir, edgenormal );
		VectorNormalize2( edgenormal, edgenormal );
		edgedist = DotProduct( p1, edgenormal ) + ON_EPSILON;

		for ( j = 0; j < w->numpoints; j++ ) {
			if ( j == i ) {
				continue;
			}
			d = DotProduct( w->p[j], edgenormal );
			if ( d > edgedist ) {
				Com_Error( ERR_DROP, "\x15" "CheckWinding: non-convex" );
			}
		}
	}
}

/* ---- WindingOnPlaneSide  0x0041F170 ---- VERIFIED */
int WindingOnPlaneSide( const winding_t *w, const vec3_t normal, vec_t dist ) {
	qboolean front, back;
	int i;
	vec_t d;

	front = qfalse;
	back = qfalse;

	if ( w->numpoints <= 0 ) {
		return SIDE_ON;
	}

	for ( i = 0; i < w->numpoints; i++ ) {
		d = DotProduct( w->p[i], normal ) - dist;
		if ( d < -ON_EPSILON ) {
			if ( front ) {
				return SIDE_CROSS;
			}
			back = qtrue;
		} else if ( d > ON_EPSILON ) {
			if ( back ) {
				return SIDE_CROSS;
			}
			front = qtrue;
		}
	}

	if ( back ) {
		return SIDE_BACK;
	}
	if ( front ) {
		return SIDE_FRONT;
	}
	return SIDE_ON;
}

/* ---- AddWindingToConvexHull  0x0041F210 ---- VERIFIED */
void AddWindingToConvexHull( const winding_t *w, winding_t **hull,
							 const vec3_t normal ) {
	int i, j, k;
	const float *p;
	const float *copy;
	vec3_t dir;
	float d;
	int numHullPoints, numNew;
	vec3_t hullPoints[MAX_HULL_POINTS];
	vec3_t newHullPoints[MAX_HULL_POINTS];
	vec3_t hullDirs[MAX_HULL_POINTS];
	qboolean hullSide[MAX_HULL_POINTS];
	qboolean outside;
	winding_t *out;

	if ( !*hull ) {
		*hull = CopyWinding( w );
		return;
	}

	numHullPoints = ( *hull )->numpoints;
	Com_Memcpy( hullPoints, ( *hull )->p,
				numHullPoints * sizeof( vec3_t ) );

	for ( i = 0; i < w->numpoints; i++ ) {
		p = w->p[i];

		for ( j = 0; j < numHullPoints; j++ ) {
			k = ( j + 1 ) % numHullPoints;
			VectorSubtract( hullPoints[k], hullPoints[j], dir );
			VectorNormalize2( dir, dir );
			CrossProduct( normal, dir, hullDirs[j] );
		}

		outside = qfalse;
		for ( j = 0; j < numHullPoints; j++ ) {
			VectorSubtract( p, hullPoints[j], dir );
			d = DotProduct( dir, hullDirs[j] );
			if ( d >= ON_EPSILON ) {
				outside = qtrue;
			}
			hullSide[j] = ( d >= -ON_EPSILON );
		}

		if ( !outside ) {
			continue;
		}

		for ( j = 0; j < numHullPoints; j++ ) {
			if ( !hullSide[j % numHullPoints] &&
				 hullSide[( j + 1 ) % numHullPoints] ) {
				break;
			}
		}
		if ( j == numHullPoints ) {
			continue;
		}

		VectorCopy( p, newHullPoints[0] );
		numNew = 1;

		j = ( j + 1 ) % numHullPoints;
		for ( k = 0; k < numHullPoints; k++ ) {
			if ( hullSide[( j + k ) % numHullPoints] &&
				 hullSide[( j + k + 1 ) % numHullPoints] ) {
				continue;
			}
			copy = hullPoints[( j + k + 1 ) % numHullPoints];
			VectorCopy( copy, newHullPoints[numNew] );
			numNew++;
		}

		numHullPoints = numNew;
		Com_Memcpy( hullPoints, newHullPoints,
					numHullPoints * sizeof( vec3_t ) );
	}

	FreeWinding( *hull );
	out = AllocWinding( numHullPoints );
	out->numpoints = numHullPoints;
	*hull = out;
	Com_Memcpy( out->p, hullPoints, numHullPoints * sizeof( vec3_t ) );
}
