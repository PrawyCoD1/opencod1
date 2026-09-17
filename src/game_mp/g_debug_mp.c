/*
 * @fidelity: likely
 *
 * g_debug_mp.c -- CoD's debug-draw helpers over trap_AddDebugLine.
 *
 * No RTCW ancestor: these are the primitives the prone and view-clamp code
 * draws with (BG_CheckProneValid, PM_UpdateViewAngles).  Every one of them
 * ends in trap_AddDebugLine, which LTCG inlined at each site, so the retail
 * bodies call syscall 68 directly.
 *
 * Function order is binary order (0x20021010 .. 0x200213E0).
 */

#include <math.h>

#include "g_local.h"

/* The 12 edges of a box, as index pairs into the 8 corners G_DebugBox
 * builds (0x20055A30). */
static int boxEdges[12][2] = {
	{ 0, 1 }, { 0, 2 }, { 0, 4 }, { 1, 3 },
	{ 1, 5 }, { 2, 3 }, { 2, 6 }, { 3, 7 },
	{ 4, 5 }, { 4, 6 }, { 5, 7 }, { 6, 7 }
};

void G_DebugLine( const vec3_t start, const vec3_t end, const vec3_t color, int depthTest, int duration ) {
	trap_AddDebugLine( start, end, color, depthTest, duration );
}

void G_DebugBox( const vec3_t mins, const vec3_t maxs, const vec3_t color, int depthTest, int duration ) {
	vec3_t corners[8];
	int i;

	for ( i = 0 ; i < 8 ; i++ ) {
		corners[i][0] = ( i & 1 ) ? maxs[0] : mins[0];
		corners[i][1] = ( i & 2 ) ? maxs[1] : mins[1];
		corners[i][2] = ( i & 4 ) ? maxs[2] : mins[2];
	}

	for ( i = 0 ; i < 12 ; i++ ) {
		trap_AddDebugLine( corners[boxEdges[i][0]], corners[boxEdges[i][1]], color, depthTest, duration );
	}
}

/*
==================
G_DebugCircle

flat draws the circle on the ground; otherwise it is turned to face client 0's
eye, which is all a listen server ever needs.
==================
*/
void G_DebugCircle( const vec3_t org, qboolean flat, float radius, const vec3_t color, int depthTest, int duration ) {
	vec3_t dir;
	vec3_t viewOrigin;

	if ( flat ) {
		dir[0] = 0;
		dir[1] = 0;
		dir[2] = 1;
	} else {
		VectorCopy( level.clients->ps.origin, viewOrigin );
		viewOrigin[2] += level.clients->ps.viewHeightCurrent;
		VectorSubtract( org, viewOrigin, dir );
	}

	G_DebugCircleEx( dir, org, radius, color, depthTest, duration );
}

void G_DebugCircleEx( const vec3_t dir, const vec3_t org, float radius, const vec3_t color, int depthTest, int duration ) {
	vec3_t right, forward, up;
	vec3_t points[16];
	float angle, c, s;
	int i;

	VectorNormalize2( dir, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	for ( i = 0 ; i < 16 ; i++ ) {
		angle = i * ( M_PI * 2 / 16 );
		c = (float)cos( angle ) * radius;
		s = (float)sin( angle ) * radius;

		points[i][0] = right[0] * c + up[0] * s + org[0];
		points[i][1] = right[1] * c + up[1] * s + org[1];
		points[i][2] = right[2] * c + up[2] * s + org[2];
	}

	for ( i = 0 ; i < 16 ; i++ ) {
		trap_AddDebugLine( points[i], points[( i + 1 ) & 15], color, depthTest, duration );
	}
}

/*
==================
G_DebugArc

16 points around org in the xy plane, from startAngle to endAngle, joined by 15
segments -- unlike G_DebugCircleEx the ring is not closed.
==================
*/
void G_DebugArc( const vec3_t org, float radius, float startAngle, float endAngle, const vec3_t color, int depthTest, int duration ) {
	vec3_t points[16];
	float step, angle;
	int i;

	step = ( endAngle - startAngle ) / 15.0f;
	if ( step < 0 ) {
		startAngle = startAngle - 360.0f;
		step = ( endAngle - startAngle ) / 15.0f;
	}

	for ( i = 0 ; i < 16 ; i++ ) {
		angle = ( i * step + startAngle ) * M_PI / 180.0f;

		points[i][0] = org[0] + (float)cos( angle ) * radius;
		points[i][1] = org[1] + (float)sin( angle ) * radius;
		points[i][2] = org[2];
	}

	for ( i = 0 ; i < 15 ; i++ ) {
		trap_AddDebugLine( points[i], points[i + 1], color, depthTest, duration );
	}
}

/*
==================
Q_rint

0x200213E0.  Bare fistp under the default round-to-nearest control word --
retail never routes this through __ftol2, so the source was inline asm.  The
bias is 0.5 - 2^-30 so x.0 stays x and x.5 rounds up.
==================
*/
int Q_rint( float f ) {
	double bias = 0.4999999990686774;
	int i;

	__asm {
		fld f
		fadd bias
		fistp i
	}
	return i;
}
