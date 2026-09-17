/*
 * cg_marks.c -- wall marks.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_marks.c is the ancestor and CG_InitMarkPolys,
 * CG_FreeMarkPoly, CG_AllocMark and CG_AddMarks are still its text.  Three
 * things changed:
 *
 *   - the mark lifetime is per-mark.  RTCW's MARK_TOTAL_TIME / MARK_FADE_TIME
 *     constants are gone; CG_ImpactMark takes a `duration` and CG_AddMarks
 *     fades over the last half of it (0x3002392C, 0x3002397A).
 *   - the renderer builds the vertices.  CoD's trap_R_MarkFragments fills a
 *     polyVert_t array directly -- xyz AND st -- so CG_ImpactMark only writes
 *     modulate; RTCW's markPoints[]/texCoordScale/DotProduct block is gone.
 *     polyVert_t is 32 bytes here, not Q3's 24, and markFragment_t 12 not 8;
 *     both live in cg_public.h.
 *   - the shader is per-fragment.  markFragment_t gained a leading handle that
 *     trap_R_MarkFragments fills in from the surface it hit, and that is what
 *     goes into the poly (0x30023837, 0x30023875) -- the markShader argument is
 *     only handed to the trap.
 *
 * Functions are in address order, which is the original file order.
 *
 * @fidelity: likely
 */

#include <string.h>

#include "cg_local.h"

/* CG_ImpactMark's two stack buffers: 1024 verts of 32 bytes at ebp-0x8000 and
   384 fragments of 12 bytes at ebp-0x9200 (0x30023719, 0x30023701). */
#define MAX_MARK_POINTS         1024
#define MAX_MARK_FRAGMENTS      384

markPoly_t cg_markPolys[MAX_MARK_POLYS];                        /* 0x30299B00 */
markPoly_t cg_activeMarkPolys;                                  /* 0x302F6B00, double linked list */
markPoly_t  *cg_freeMarkPolys;                                  /* 0x30299AF0, single linked list */

/*
===================
CG_InitMarkPolys     0x30023440

This is called at startup and for tournement restarts
===================
*/
void CG_InitMarkPolys( void ) {
	int i;

	memset( cg_markPolys, 0, sizeof( cg_markPolys ) );

	cg_activeMarkPolys.nextMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.prevMark = &cg_activeMarkPolys;
	cg_freeMarkPolys = cg_markPolys;
	for ( i = 0 ; i < MAX_MARK_POLYS - 1 ; i++ ) {
		cg_markPolys[i].nextMark = &cg_markPolys[i + 1];
	}
}

/*
==================
CG_FreeMarkPoly     0x30023490
==================
*/
void CG_FreeMarkPoly( markPoly_t *le ) {
	if ( !le->prevMark ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prevMark->nextMark = le->nextMark;
	le->nextMark->prevMark = le->prevMark;

	// the free list is only singly linked
	le->nextMark = cg_freeMarkPolys;
	cg_freeMarkPolys = le;
}

/*
===================
CG_AllocMark     0x300234D0

Will allways succeed, even if it requires freeing an old active mark

`expireTime` is cg.time + duration at the one call site (0x3002385C); nothing in
the 1.1 body reads it.
===================
*/
markPoly_t *CG_AllocMark( int expireTime ) {
	markPoly_t  *le;
	int time;

	if ( !cg_freeMarkPolys ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		time = cg_activeMarkPolys.prevMark->time;
		while ( cg_activeMarkPolys.prevMark && time == cg_activeMarkPolys.prevMark->time ) {
			CG_FreeMarkPoly( cg_activeMarkPolys.prevMark );
		}
	}

	le = cg_freeMarkPolys;
	cg_freeMarkPolys = cg_freeMarkPolys->nextMark;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->nextMark = cg_activeMarkPolys.nextMark;
	le->prevMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.nextMark->prevMark = le;
	cg_activeMarkPolys.nextMark = le;
	return le;
}

/*
=================
CG_ImpactMark     0x30023560

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================
*/
void CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
					float orientation, float red, float green, float blue, float alpha,
					qboolean alphaFade, float radius, qboolean temporary, int duration ) {
	vec3_t          axis[3];
	vec3_t          originalPoints[4];
	byte            colors[4];
	int             i, j;
	int             numFragments;
	markFragment_t  markFragments[MAX_MARK_FRAGMENTS], *mf;
	polyVert_t      markPoints[MAX_MARK_POINTS];
	polyVert_t      *v;
	markPoly_t      *mark;

	if ( !cg_marks.integer ) {
		return;
	}

	/* CG_EntityEvent brackets EV_ROCKET_EXPLODE_NOMARKS with this flag
	   (0x3001E59F / 0x3001E64D). */
	if ( *(int *)&cg.unknown_0x2A87C[4] ) {
		return;
	}

	if ( duration < 0 ) {
		duration = 20000;
	}

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	PerpendicularVector( axis[1], axis[0] );
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
		originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
		originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
		originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
	}

	// get the fragments
	numFragments = trap_R_MarkFragments( 4, originalPoints, origin, axis[0],
										 radius, MAX_MARK_POINTS, markPoints,
										 MAX_MARK_FRAGMENTS, markFragments, markShader );

	colors[0] = (byte)( red * 255 );
	colors[1] = (byte)( green * 255 );
	colors[2] = (byte)( blue * 255 );
	colors[3] = (byte)( alpha * 255 );

	for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ ) {
		// we have an upper limit on the complexity of polygons
		// that we store persistantly
		if ( mf->numPoints > MAX_VERTS_ON_POLY ) {
			mf->numPoints = MAX_VERTS_ON_POLY;
		}

		// the renderer already laid down xyz and st; only modulate is ours
		v = &markPoints[ mf->firstPoint ];
		for ( j = 0 ; j < mf->numPoints ; j++, v++ ) {
			*(int *)v->modulate = *(int *)colors;
		}

		// if it is a temporary (shadow) mark, add it immediately and forget about it
		if ( temporary ) {
			trap_R_AddPolyToScene( mf->markShader, mf->numPoints,
								   &markPoints[ mf->firstPoint ] );
			continue;
		}

		// otherwise save it persistantly
		mark = CG_AllocMark( cg.time + duration );
		mark->time = cg.time;
		mark->alphaFade = alphaFade;
		mark->markShader = mf->markShader;
		mark->numVerts = mf->numPoints;
		mark->color[0] = red;
		mark->color[1] = green;
		mark->color[2] = blue;
		mark->color[3] = alpha;
		mark->duration = duration;
		memcpy( mark->vertData, &markPoints[ mf->firstPoint ],
				mf->numPoints * sizeof( polyVert_t ) );
	}
}

/*
===============
CG_AddMarks     0x300238F0
===============
*/
void CG_AddMarks( void ) {
	int         j;
	markPoly_t  *mp, *next;
	int         t;
	float       fadeTime;
	float       fade;

	if ( !cg_marks.integer ) {
		return;
	}

	mp = cg_activeMarkPolys.nextMark;
	for ( ; mp != &cg_activeMarkPolys ; mp = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = mp->nextMark;

		// see if it is time to completely remove it
		if ( cg.time > mp->time + mp->duration ) {
			CG_FreeMarkPoly( mp );
			continue;
		}

		// fade all marks out with time
		t = mp->time + mp->duration - cg.time;
		fadeTime = mp->duration * 0.5f;
		if ( t < fadeTime ) {
			fade = t * 255.0f / fadeTime;
			if ( mp->alphaFade ) {
				for ( j = 0 ; j < mp->numVerts ; j++ ) {
					mp->vertData[j].modulate[3] = (byte)fade;
				}
			} else {
				for ( j = 0 ; j < mp->numVerts ; j++ ) {
					mp->vertData[j].modulate[0] = (byte)( fade * mp->color[0] );
					mp->vertData[j].modulate[1] = (byte)( fade * mp->color[1] );
					mp->vertData[j].modulate[2] = (byte)( fade * mp->color[2] );
				}
			}
		}

		trap_R_AddPolyToScene( mp->markShader, mp->numVerts, mp->vertData );
	}
}
