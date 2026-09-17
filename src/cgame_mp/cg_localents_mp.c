/*
 * cg_localents.c -- every frame, generate renderer commands for locally
 * processed entities.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_localents.c is the ancestor and the list
 * plumbing -- CG_InitLocalEntities, CG_FreeLocalEntity, CG_AllocLocalEntity,
 * CG_AddLocalEntities -- is its text verbatim, `localEntCount` debugging
 * counter included.  What CoD dropped is everything the FX system took over:
 * of RTCW's fifteen leTypes only three survive, and CG_AddLocalEntities'
 * switch is down to LE_FADE_RGB / LE_SCALE_FADE / LE_MOVING_TRACER.
 *
 * Functions are in address order, which is the original file order.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <math.h>

#include "cg_local.h"
#include "../game_mp/bg_public.h"

/*
 * CoD's leType_t.  CG_AddLocalEntities dispatches 0/1/2 and errors on anything
 * else (0x3002020F); the spellings are RTCW's for the three that survived.
 */
#define LE_FADE_RGB             0
#define LE_SCALE_FADE           1
#define LE_MOVING_TRACER        2

/* CG_AddScaleFade tests bit 0 of leFlags (0x30020102); RTCW's LEF_PUFF_DONT_SCALE. */
#define LEF_PUFF_DONT_SCALE     0x0001

/*
 * cg_local.h's localEntity_t stops at lifeRate and carries the rest of the
 * 236-byte record as unknown_0x018[212].  The tail is RTCW's, minus the fields
 * CoD dropped:
 *
 *      +0x018  trajectory_t pos        BG_EvaluateTrajectory 0x3001FFF4, trDelta 0x30020000
 *      +0x03C  float        color[4]   CG_AddFadeRGB 0x3002008D..
 *      +0x04C  float        radius     CG_AddScaleFade 0x30020115 / 0x30020155
 *      +0x050  refEntity_t  refEntity  shaderRGBA at +0x0BC, origin at +0x094
 */
#define LE_POS( le )        ( *(trajectory_t *)&( le )->unknown_0x018[0x000] )
#define LE_COLOR( le )      ( (float *)&( le )->unknown_0x018[0x024] )
#define LE_RADIUS( le )     ( *(float *)&( le )->unknown_0x018[0x034] )
#define LE_REFENTITY( le )  ( *(refEntity_t *)&( le )->unknown_0x018[0x038] )
/* refEntity_t's radius is at +0x7C in CoD's 156-byte record, inside the tail
   cg_public.h leaves as unknown_0x70 (CG_AddScaleFade 0x30020115). */
#define RE_RADIUS( re )     ( *(float *)&( re )->unknown_0x70[0x0C] )

/* Owned by other units. */
void        CG_Error( const char *msg, ... );                   /* cg_main.c 0x30020750 */
void        CG_DrawTracer( const vec3_t start, const vec3_t end );      /* cg_weapons.c 0x300390C0 */

localEntity_t cg_localEntities[MAX_LOCAL_ENTITIES];             /* 0x3019EB60 */
localEntity_t cg_activeLocalEntities;                           /* 0x301A6180, double linked list */
localEntity_t   *cg_freeLocalEntities;                          /* 0x301A6160, single linked list */

/* Ridah, debugging -- the counter survived into CoD unchanged (0x300EEF24). */
int localEntCount = 0;

/*
===================
CG_InitLocalEntities     0x3001FEC0

This is called at startup and for tournement restarts
===================
*/
void CG_InitLocalEntities( void ) {
	int i;

	memset( cg_localEntities, 0, sizeof( cg_localEntities ) );
	cg_activeLocalEntities.next = &cg_activeLocalEntities;
	cg_activeLocalEntities.prev = &cg_activeLocalEntities;
	cg_freeLocalEntities = cg_localEntities;
	for ( i = 0 ; i < MAX_LOCAL_ENTITIES - 1 ; i++ ) {
		cg_localEntities[i].next = &cg_localEntities[i + 1];
	}

	// Ridah, debugging
	localEntCount = 0;
}

/*
==================
CG_FreeLocalEntity     0x3001FF10
==================
*/
void CG_FreeLocalEntity( localEntity_t *le ) {
	if ( !le->prev ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// Ridah, debugging
	localEntCount--;

	// remove from the doubly linked active list
	le->prev->next = le->next;
	le->next->prev = le->prev;

	// the free list is only singly linked
	le->next = cg_freeLocalEntities;
	cg_freeLocalEntities = le;
}

/*
===================
CG_AllocLocalEntity     0x3001FF50

Will allways succeed, even if it requires freeing an old active entity
===================
*/
localEntity_t *CG_AllocLocalEntity( void ) {
	localEntity_t   *le;

	if ( !cg_freeLocalEntities ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		CG_FreeLocalEntity( cg_activeLocalEntities.prev );
	}

	// Ridah, debugging
	localEntCount++;

	le = cg_freeLocalEntities;
	cg_freeLocalEntities = cg_freeLocalEntities->next;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->next = cg_activeLocalEntities.next;
	le->prev = &cg_activeLocalEntities;
	cg_activeLocalEntities.next->prev = le;
	cg_activeLocalEntities.next = le;
	return le;
}

/*
================
CG_AddMovingTracer     0x3001FFE0
================
*/
static void CG_AddMovingTracer( localEntity_t *le ) {
	vec3_t start, end, dir;

	BG_EvaluateTrajectory( &LE_POS( le ), cg.time, start );
	VectorNormalize2( LE_POS( le ).trDelta, dir );
	VectorMA( start, cg_tracerlength.value, dir, end );

	CG_DrawTracer( start, end );
}

/*
====================
CG_AddFadeRGB     0x30020060
====================
*/
static void CG_AddFadeRGB( localEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &LE_REFENTITY( le );

	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	re->shaderRGBA[0] = (byte)( LE_COLOR( le )[0] * c );
	re->shaderRGBA[1] = (byte)( LE_COLOR( le )[1] * c );
	re->shaderRGBA[2] = (byte)( LE_COLOR( le )[2] * c );
	re->shaderRGBA[3] = (byte)( LE_COLOR( le )[3] * c );

	trap_R_AddRefEntityToScene( re );
}

/*
===================
CG_AddScaleFade     0x300200D0

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
	refEntity_t *re;
	float c;
	vec3_t delta;
	float len;

	re = &LE_REFENTITY( le );

	// fade / grow time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = (byte)( 0xff * c * LE_COLOR( le )[3] );
	if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
		RE_RADIUS( re ) = LE_RADIUS( le ) * ( 1.0f - c ) + 8;
	}

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < LE_RADIUS( le ) ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}

/*
===================
CG_AddLocalEntities     0x300201B0
===================
*/
void CG_AddLocalEntities( void ) {
	localEntity_t   *le, *next;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	le = cg_activeLocalEntities.prev;
	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = le->prev;

		if ( cg.time >= le->endTime ) {
			CG_FreeLocalEntity( le );
			continue;
		}
		switch ( le->leType ) {
		default:
			CG_Error( "Bad leType: %i", le->leType );
			break;

		case LE_FADE_RGB:
			CG_AddFadeRGB( le );
			break;

		case LE_SCALE_FADE:
			CG_AddScaleFade( le );
			break;

		case LE_MOVING_TRACER:
			CG_AddMovingTracer( le );
			break;
		}
	}
}
