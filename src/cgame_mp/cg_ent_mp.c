/*
 * cg_ent_mp.c -- present snapshot entities, happens every single frame.
 *
 * cgame/cg_ent.c of cgame_mp_x86.dll (CoD 1.1 multiplayer, imagebase
 * 0x30000000).  40 functions, 0x3001AAA0 .. 0x3001D5F0, in address order.
 *
 * RTCW's cgame/cg_ents.c is the ancestor of CG_EntityEffects, CG_Item,
 * CG_Missile, CG_Mover, CG_Beam, CG_Portal, CG_AdjustPositionForMover,
 * CG_InterpolateEntityPosition, CG_CalcEntityLerpPositions, CG_AddCEntity,
 * CG_ProcessEntity and CG_AddPacketEntities.  Everything that touches a DObj
 * -- the whole CG_DObj* block, CG_PreProcess_GetDObj, CG_DoControllers,
 * CG_mg42*, CG_SoundBlend, CG_LoopFx -- has no RTCW ancestor at all: CoD
 * replaced RTCW's frame/oldframe/backlerp model layer with XModel + XAnim, so
 * a refEntity is submitted with reType 1 and a DObj pointer instead of a
 * frame pair.
 *
 * THE HOLES.  Three records this unit uses are declared with unnamed ranges
 * in the headers -- refEntity_t (cg_public.h), centity_t / cg_t / cgs_t /
 * cgWeaponInfo_t (cg_local.h).  Fields inside those ranges are reached
 * through the macros and local overlays below, each carrying the offset and
 * a reference address.
 *
 * @fidelity: partial
 */

#include "cg_local.h"

#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

/* Declarations no header carries. */

/* game/bg_misc.c */
void            BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );

/* cg_main_mp.c / cg_servercmds_mp.c */
void QDECL     CG_Printf( const char *msg, ... );
void QDECL     CG_Error( const char *msg, ... );
const char     *CG_ConfigString( int index );

/* cg_players_mp.c */
void            CG_Player( centity_t *cent );
void            CG_Corpse( centity_t *cent );

/* cg_weapons_mp.c */
void            CG_RegisterItemVisuals( int itemNum );

/*
=============================================================================

	THE HOLES

=============================================================================
*/

/*
 * refEntity_t.  Every offset below is a store this unit makes into the 156
 * bytes it hands trap_R_AddRefEntityToScene, and every one of them lands in
 * one of cg_public.h's unknown_ ranges.  The layout that comes out is Q3's
 * head verbatim through +0x40 and CoD's own tail after +0x6C.
 */
#define RE_HMODEL( e )              ( *(qhandle_t *)&( e )->unknown_0x08[0] )    /* +0x08 CG_Mover 0x3001B6D9 */
#define RE_LIGHTINGORIGIN( e )      ( (float *)&( e )->unknown_0x08[4] )         /* +0x0C CG_LockLightingOrigin 0x3001AB10 */
#define RE_NONNORMALIZEDAXES( e )   ( *(float *)( e )->unknown_0x40 )            /* +0x40 CG_Item 0x3001AEA6 */
#define RE_FRAME( e )               ( *(int *)( e )->unknown_0x50 )              /* +0x50 CG_Portal 0x3001BA1C */
#define RE_ROTATION( e )            ( *(float *)&( e )->unknown_0x70[0x10] )     /* +0x80 CG_Portal 0x3001BA24 */
#define RE_DOBJ( e )                ( *(void **)&( e )->unknown_0x70[0x20] )     /* +0x90 CG_General 0x3001ABF5 */
#define RE_OWNER( e )               ( *(void **)&( e )->unknown_0x70[0x24] )     /* +0x94 CG_General 0x3001ABFC */

#define RT_XMODEL                   0       /* CG_Mover 0x3001B6DD */
#define RT_DOBJ                     1       /* CG_General 0x3001AC03 */
#define RT_BEAM                     6       /* CG_Beam 0x3001B905 */
#define RT_PORTALSURFACE            11      /* CG_Portal 0x3001BA10 */

#define RF_NOSHADOW                 0x40    /* CG_Mover 0x3001B688 */
#define RF_LIGHTING_ORIGIN          0x80    /* CG_LockLightingOrigin 0x3001AB25 */

/*
 * centity_t.  Three dwords cg_local.h carries as holes.
 */
#define CENT_TRAILSTARTED( c )      ( *(int *)&( c )->unknown_0x1F0[0] )         /* +0x1F0 CG_Missile 0x3001B553 */
#define CENT_LIGHTINGORIGIN( c )    ( (float *)( c )->unknown_0x210 )            /* +0x210 CG_LockLightingOrigin 0x3001AAAD */
#define CENT_FXTIME( c )            ( *(int *)( c )->unknown_0x21C )             /* +0x21C CG_LoopFx 0x3001CF26 */

/*
 * cg.unknown_0x273AC -- the 144-byte auto-rotation block CG_AddPacketEntities
 * rebuilds every frame (0x3001BDF7 .. 0x3001BF26).  RTCW's six members fit it
 * exactly, and the block ends where cg.refdef begins.
 */
typedef struct cgAutoRotate_t {
	vec3_t          autoAnglesSlow;         /* cg+0x273AC */
	vec3_t          autoAxisSlow[3];        /* cg+0x273B8 */
	vec3_t          autoAngles;             /* cg+0x273DC */
	vec3_t          autoAxis[3];            /* cg+0x273E8 */
	vec3_t          autoAnglesFast;         /* cg+0x2740C */
	vec3_t          autoAxisFast[3];        /* cg+0x27418 */
} cgAutoRotate_t;

#define CG_AUTOROTATE()             ( (cgAutoRotate_t *)cg.unknown_0x273AC )

/* cg.unknown_0x2AA40 -- the view-model placement CG_DObjGetViewModelTagMatrix
   reads (0x3001C40A .. 0x3001C476). */
#define CG_VIEWMODELORIGIN()        ( (float *)( cg.unknown_0x2AA40 + 0x2C ) )   /* cg+0x2AA6C */
#define CG_VIEWMODELAXIS()          ( (vec3_t *)( cg.unknown_0x2AA40 + 0x38 ) )  /* cg+0x2AA78 */

/* cgs.unknown_0x07520 -- the inline-model midpoints CG_EntityEffects adds to
   the lerp origin of a bmodel (0x3001AF58). */
#define CGS_INLINEMODELMIDPOINTS()  ( (vec3_t *)( cgs.unknown_0x07520 + 0x400 ) ) /* cgs+0x7920 */

/*
 * cg_items[] -- cg_weapons_mp.c's per-item record, 36 bytes (0x301DC3E0,
 * `lea esi,[eax+eax*8]` at 0x3001ADF8).  Only the two fields CG_Item reads
 * are named.
 */
typedef struct cgItemInfo_t {
	qboolean        registered;             /* +0x00 CG_Item 0x3001AE15 */
	void            *models[MAX_ITEM_MODELS];       /* +0x04 CG_Item 0x3001AE37 */
	byte            unknown_0x0C[24];
} cgItemInfo_t;
CG_ASSERT_SIZE( cgItemInfo_t, 36 );

extern cgItemInfo_t cg_items[];                 /* 0x301DC3E0 */

/*
 * The six cg_weapons fields CG_Missile reads all sit inside cg_local.h's
 * cgWeaponInfo_t `unknown_0x0A4[244]` hole.  This overlay names them;
 * CG_Missile casts &cg_weapons[weapon] to it.
 */
typedef struct cgWeaponMissileInfo_t {
	byte            unknown_0x000[0x0C8];
	const char      *missileSound;          /* +0x0C8 0x3001B442 */
	byte            unknown_0x0CC[0x058];
	int             missileModel;           /* +0x124 0x3001B45E */
	byte            unknown_0x128[0x004];
	float           missileDlight;          /* +0x12C 0x3001B48B */
	vec3_t          missileDlightColor;     /* +0x130 0x3001B49E */
	int             missileRenderfx;        /* +0x13C 0x3001B4F6 */
	byte            unknown_0x140[0x010];
	int             missileTrailEffect;     /* +0x150 0x3001B546 */
} cgWeaponMissileInfo_t;

/*
 * One bone of the DObj pose, 32 bytes -- the same record the game DLL's
 * G_DObjSetLocalTagInternal writes.  trap_syscall_0x9E hands back the array.
 */
typedef struct DObjAnimMat_t {
	vec4_t          quat;                   /* +0x00 */
	float           accumulatedWeight;      /* +0x10 CG_DObjSetLocalTagInternal 0x3001C1AC */
	vec3_t          translation;            /* +0x14 0x3001C1AF */
} DObjAnimMat_t;
CG_ASSERT_SIZE( DObjAnimMat_t, 32 );

/*
 * The 64-byte matrices trap_syscall_0x9D hands back (`shl esi, 6`,
 * 0x3001C2F9), one per bone.
 */
#define DOBJ_BONEMATRIX( base, boneIndex )  ( (float *)( (byte *)( base ) + ( ( boneIndex ) << 6 ) ) )

/* Entity types, from CG_ProcessEntity's own switch (jump table at
   0x3001D602) plus the ET_EVENTS cutoff CG_AddCEntity tests. */
#define ET_GENERAL              0
#define ET_PLAYER               1
#define ET_CORPSE               2
#define ET_ITEM                 3
#define ET_MISSILE              4
#define ET_MOVER                5
#define ET_PORTAL               6
#define ET_INVISIBLE            7
#define ET_SCRIPTMOVER          8
#define ET_SOUND_BLEND          9
#define ET_FX                   10
#define ET_MG42                 11
#define ET_EVENTS               12

#define EF_NODRAW               0x00000100  /* CG_General 0x3001AB6F */
#define EF_MG42_FIRING          0x00000400  /* CG_mg42_DoControllers 0x3001B285 */
#define EF_LOCK_LIGHTING        0x00010000  /* CG_LockLightingOrigin 0x3001AAA0 */

#define PMF_VIEWLOCKED          0x00050000  /* CG_AddPacketEntities 0x3001BF47 */
#define PSEF_VIEWLOCKED         0x0000C000  /* CG_mg42_DoControllers 0x3001B266 */

#define TR_INTERPOLATE          1
#define TR_LINEAR_STOP          3

#define SOLID_BMODEL            0xFFFFFF

#define CS_SOUNDALIAS           524         /* CG_EntityEffects 0x3001AF3F */

#define ENTITYNUM_MAX_NORMAL    ( MAX_GENTITIES - 2 )   /* 0x3FE, CG_AdjustPositionForMover 0x3001BABD */

#define BG_NUM_ITEMS            70          /* CG_Item 0x3001ADD2 */

/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
==================
CG_LockLightingOrigin

EF_LOCK_LIGHTING freezes the point the model is lit from at the position it
first had, so a mover does not swim through the lightgrid.  CoD's replacement
for RTCW's per-entity lightingOrigin bookkeeping.
==================
*/
static void CG_LockLightingOrigin( centity_t *cent, refEntity_t *ent ) {
	float       *lightingOrigin;

	lightingOrigin = CENT_LIGHTINGORIGIN( cent );

	if ( !( cent->currentState.eFlags & EF_LOCK_LIGHTING ) ) {
		lightingOrigin[2] = 0;
		lightingOrigin[1] = 0;
		lightingOrigin[0] = 0;
		return;
	}

	if ( lightingOrigin[0] == vec3_origin[0]
		 && lightingOrigin[1] == vec3_origin[1]
		 && lightingOrigin[2] == vec3_origin[2] ) {
		VectorCopy( cent->lerpOrigin, lightingOrigin );
	}

	VectorCopy( lightingOrigin, RE_LIGHTINGORIGIN( ent ) );
	ent->renderfx |= RF_LIGHTING_ORIGIN;
}

/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) {
	refEntity_t     ent;
	entityState_t   *s1;
	int             obj;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if ( s1->eFlags & EF_NODRAW ) {
		return;
	}

	CG_PreProcess_GetDObj( s1->number, s1->eType, cgs.gameModels[ s1->index ] );

	obj = trap_syscall_0xA2( s1->number );
	if ( !obj ) {
		return;
	}

	memset( &ent, 0, sizeof( ent ) );

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );

	AnglesToAxis( cent->lerpAngles, ent.axis );

	RE_DOBJ( &ent ) = (void *)obj;
	RE_OWNER( &ent ) = cent;
	ent.reType = RT_DOBJ;

	CG_LockLightingOrigin( cent, &ent );

	// add to refresh list
	trap_R_AddRefEntityToScene( &ent );
}

/*
==================
sub_3001AC40

RTCW's CG_PlayerSeesItem, with the trace and the itemType argument dropped;
nothing in 1.1 calls it any more.  Unnamed in retail, so the address stands
in for the name.
==================
*/
static qboolean sub_3001AC40( playerState_t *ps, entityState_t *item, int atTime ) {
	vec3_t      vorigin, eorigin, viewa, dir;
	float       dot, dist, foo;
	float       sp, cp, sy, cy;

	BG_EvaluateTrajectory( &item->pos, atTime, eorigin );

	VectorCopy( ps->origin, vorigin );
	vorigin[2] += ps->viewHeightCurrent;

	VectorSubtract( vorigin, eorigin, dir );

	dist = VectorNormalize( dir );

	if ( dist > 255 ) {
		return qfalse;
	}

	SinCos( ps->viewangles[1] * ( (float)M_PI / 180.0f ), &sy, &cy );
	SinCos( ps->viewangles[0] * ( (float)M_PI / 180.0f ), &sp, &cp );

	viewa[0] = cp * cy;
	viewa[1] = cp * sy;
	viewa[2] = -sp;

	dot = DotProduct( viewa, dir );

	foo = -0.94f - ( dist * ( 1.0f / 255.0f ) ) * 0.057f;

	return (qboolean)( dot <= foo );
}

/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) {
	refEntity_t     ent;
	entityState_t   *es;
	gitem_t         *item;
	int             obj;

	es = &cent->currentState;

	// (item index is stored in es->index for item)
	if ( es->index >= BG_NUM_ITEMS ) {
		CG_Error( "Bad item index %i on entity", es->index );
	}

	// if set to invisible, skip
	if ( es->eFlags & EF_NODRAW ) {
		return;
	}

	item = &bg_itemlist[ es->index ];

	if ( !cg_items[ es->index ].registered ) {
		CG_RegisterItemVisuals( es->index );
		return;
	}

	if ( !cg_items[ es->index ].models[0] ) {
		CG_Error( "No XModel loaded for item index %i (%s)", es->index, item->pickup_name );
	}

	CG_PreProcess_GetDObj( es->number, es->eType, (qhandle_t)cg_items[ es->index ].models[0] );

	obj = trap_syscall_0xA2( es->number );
	if ( !obj ) {
		return;
	}

	memset( &ent, 0, sizeof( ent ) );

	if ( item->giType == IT_WEAPON ) {
		AnglesToAxis( cent->lerpAngles, ent.axis );
		RE_NONNORMALIZEDAXES( &ent ) = 1.0f;
	} else {
		AnglesToAxis( cent->lerpAngles, ent.axis );
	}

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );

	RE_DOBJ( &ent ) = (void *)obj;
	RE_OWNER( &ent ) = cent;
	ent.reType = RT_DOBJ;

	// add to refresh list
	trap_R_AddRefEntityToScene( &ent );
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {
	// add loop sound
	if ( cent->currentState.loopSound ) {
		if ( cent->currentState.solid == SOLID_BMODEL ) {
			vec3_t  origin;
			float   *v;

			v = CGS_INLINEMODELMIDPOINTS()[ cent->currentState.index ];
			VectorAdd( cent->lerpOrigin, v, origin );
			CG_PlaySoundAliasByName( CG_ConfigString( CS_SOUNDALIAS + cent->currentState.loopSound ),
									 cent->currentState.number, origin );
		} else {
			CG_PlaySoundAliasByName( CG_ConfigString( CS_SOUNDALIAS + cent->currentState.loopSound ),
									 cent->currentState.number, cent->lerpOrigin );
		}
	}

	// constant light glow
	if ( cent->currentState.constantLight ) {
		int cl;
		int i, r, g, b;

		cl = cent->currentState.constantLight;
		r = cl & 255;
		g = ( cl >> 8 ) & 255;
		b = ( cl >> 16 ) & 255;
		i = ( ( cl >> 24 ) & 255 ) * 4;

		trap_R_AddLightToScene( cent->lerpOrigin, (float)i,
								(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f );
	}
}

/*
==================
CG_GetMG42Anims

Build the two-animation tree an mg42 barrel plays: the weapon's idle anim
under child 1, its fire anim under child 2.
==================
*/
static int CG_GetMG42Anims( centity_t *cent ) {
	weaponInfo_t    *weapInfo;
	int             anims;

	weapInfo = bg_weaponInfo[ cent->currentState.weapon ];

	anims = trap_XAnimCreateAnims( (int)"MG42", 3 );
	trap_XAnimBlend( anims, 0, (int)"root", 1, 2, 0 );

	trap_XAnimPrecache( (int)weapInfo->idleAnim );
	trap_XAnimCreate( anims, 1, (int)weapInfo->idleAnim );

	trap_XAnimPrecache( (int)weapInfo->fireAnim );
	trap_XAnimCreate( anims, 2, (int)weapInfo->fireAnim );

	return anims;
}

/*
==================
CG_mg42_DoControllers

Aim the barrel: angles2[0]/[1] drive tag_aim and tag_aim_animated, angles2[2]
drives tag_flash, and the anim tree is knobbed to idle or fire.
==================
*/
static void CG_mg42_DoControllers( centity_t *cent, int *partBits ) {
	int         obj;
	int         boneIndex;
	vec3_t      angles;
	int         anim;

	obj = trap_syscall_0xA2( cent->currentState.number );

	angles[0] = LerpAngle( cent->currentState.angles2[0], cent->nextState.angles2[0],
						   cg.frameInterpolation );
	angles[1] = LerpAngle( cent->currentState.angles2[1], cent->nextState.angles2[1],
						   cg.frameInterpolation );
	angles[2] = 0;

	boneIndex = trap_syscall_0xAE( obj, (int)"tag_aim" );
	if ( boneIndex >= 0 && trap_syscall_0xA0( obj, (int)partBits, boneIndex ) ) {
		CG_DObjSetLocalTagInternal( obj, vec3_origin, angles, boneIndex );
	}

	boneIndex = trap_syscall_0xAE( obj, (int)"tag_aim_animated" );
	if ( boneIndex >= 0 && trap_syscall_0xA0( obj, (int)partBits, boneIndex ) ) {
		CG_DObjSetLocalTagInternal( obj, vec3_origin, angles, boneIndex );
	}

	angles[0] = LerpAngle( cent->currentState.angles2[2], cent->nextState.angles2[2],
						   cg.frameInterpolation );
	angles[1] = 0;

	boneIndex = trap_syscall_0xAE( obj, (int)"tag_flash" );
	if ( boneIndex >= 0 && trap_syscall_0xA0( obj, (int)partBits, boneIndex ) ) {
		CG_DObjSetLocalTagInternal( obj, vec3_origin, angles, boneIndex );
	}

	// the local player riding this gun sees it idle even while firing
	if ( ( cg.predictedPlayerState.eFlags & PSEF_VIEWLOCKED )
		 && cg.predictedPlayerState.viewlocked_entNum == cent->currentState.number
		 && !cg.renderingThirdPerson ) {
		anim = 1;
	} else {
		anim = ( cent->currentState.eFlags & EF_MG42_FIRING ) != 0;
		anim++;
	}

	trap_XAnimSetCompleteGoalWeightKnobAll( trap_syscall_0xB1( obj ), anim, 0, 1.0f, 0.1f, 1.0f, 0, 0 );
}

/*
==================
CG_mg42
==================
*/
static void CG_mg42( centity_t *cent ) {
	refEntity_t     ent;
	entityState_t   *s1;
	int             obj;

	s1 = &cent->currentState;

	if ( s1->eFlags & EF_NODRAW ) {
		return;
	}

	CG_PreProcess_GetDObj( s1->number, s1->eType, cgs.gameModels[ s1->index ] );

	obj = trap_syscall_0xA2( s1->number );
	if ( !obj ) {
		return;
	}

	memset( &ent, 0, sizeof( ent ) );

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );

	// light the gun from head height rather than from the floor it sits on
	RE_LIGHTINGORIGIN( &ent )[0] = cent->lerpOrigin[0];
	RE_LIGHTINGORIGIN( &ent )[1] = cent->lerpOrigin[1];
	RE_LIGHTINGORIGIN( &ent )[2] = cent->lerpOrigin[2] + 32.0f;
	ent.renderfx = RF_LIGHTING_ORIGIN;

	AnglesToAxis( cent->lerpAngles, ent.axis );

	RE_DOBJ( &ent ) = (void *)obj;
	RE_OWNER( &ent ) = cent;
	ent.reType = RT_DOBJ;

	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_Missile
===============
*/
static void CG_Missile( centity_t *cent ) {
	refEntity_t             ent;
	entityState_t           *s1;
	cgWeaponMissileInfo_t   *weapon;
	int                     obj;
	int                     bolt[2];
	int                     boneIndex;

	s1 = &cent->currentState;

	if ( s1->eFlags & EF_NODRAW ) {
		return;
	}

	if ( s1->weapon > bg_numWeapons ) {
		s1->weapon = 0;
	}
	weapon = (cgWeaponMissileInfo_t *)&cg_weapons[ s1->weapon ];

	// add missile sound
	if ( weapon->missileSound ) {
		CG_PlaySoundAliasByName( weapon->missileSound, s1->number, cent->lerpOrigin );
	}

	CG_PreProcess_GetDObj( s1->number, s1->eType, weapon->missileModel );

	obj = trap_syscall_0xA2( s1->number );
	if ( !obj ) {
		return;
	}

	// add dynamic light
	if ( weapon->missileDlight != 0 ) {
		trap_R_AddLightToScene( cent->lerpOrigin, weapon->missileDlight,
								weapon->missileDlightColor[0], weapon->missileDlightColor[1],
								weapon->missileDlightColor[2] );
	}

	// create the render entity
	memset( &ent, 0, sizeof( ent ) );

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );

	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	AnglesToAxis( cent->lerpAngles, ent.axis );

	RE_DOBJ( &ent ) = (void *)obj;
	RE_OWNER( &ent ) = cent;
	ent.reType = RT_DOBJ;

	trap_R_AddRefEntityToScene( &ent );

	// add trails -- the effect is bolted to tag_origin once and then runs itself
	if ( weapon->missileTrailEffect && !CENT_TRAILSTARTED( cent ) ) {
		bolt[0] = s1->number;
		boneIndex = trap_syscall_0xDF( s1->number, "tag_origin" );
		if ( boneIndex >= 0 ) {
			bolt[1] = boneIndex;
			trap_syscall_0xE5( weapon->missileTrailEffect, (int)cent->lerpOrigin, 0, (int)bolt );
		}
		CENT_TRAILSTARTED( cent ) = 1;
	}
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) {
	refEntity_t     ent;
	entityState_t   *s1;
	int             obj;

	s1 = &cent->currentState;

	if ( s1->eFlags & EF_NODRAW ) {
		return;
	}

	obj = trap_syscall_0xA2( s1->number );
	if ( s1->solid != SOLID_BMODEL && !obj ) {
		return;
	}

	// create the render entity
	memset( &ent, 0, sizeof( ent ) );

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );

	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	// get the model, either as a bmodel or a DObj
	if ( s1->solid == SOLID_BMODEL ) {
		RE_HMODEL( &ent ) = cgs.inlineDrawModel[ s1->index ];
		ent.reType = RT_XMODEL;
	} else {
		RE_DOBJ( &ent ) = (void *)obj;
		RE_OWNER( &ent ) = cent;
		ent.reType = RT_DOBJ;
	}

	// add to refresh list
	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_ScriptMover

CG_Mover with a DObj built for it and its lighting origin locked -- the
script-driven movers carry XModels, not bmodels, most of the time.
===============
*/
static void CG_ScriptMover( centity_t *cent ) {
	refEntity_t     ent;
	entityState_t   *s1;
	int             obj;

	s1 = &cent->currentState;

	if ( s1->eFlags & EF_NODRAW ) {
		return;
	}

	CG_PreProcess_GetDObj( s1->number, s1->eType, cgs.gameModels[ s1->index ] );

	obj = trap_syscall_0xA2( s1->number );
	if ( s1->solid != SOLID_BMODEL && !obj ) {
		return;
	}

	memset( &ent, 0, sizeof( ent ) );

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );

	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	if ( s1->solid == SOLID_BMODEL ) {
		RE_HMODEL( &ent ) = cgs.inlineDrawModel[ s1->index ];
		ent.reType = RT_XMODEL;
	} else {
		RE_DOBJ( &ent ) = (void *)obj;
		RE_OWNER( &ent ) = cent;
		ent.reType = RT_DOBJ;
		CG_LockLightingOrigin( cent, &ent );
	}

	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent ) {
	refEntity_t     ent;
	entityState_t   *s1;

	s1 = &cent->currentState;

	// create the render entity
	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );

	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) {
	refEntity_t     ent;
	entityState_t   *s1;

	s1 = &cent->currentState;

	// create the render entity
	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	ByteToDir( s1->eventParm, ent.axis[0] );
	PerpendicularVector( ent.axis[1], ent.axis[0] );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
	RE_FRAME( &ent ) = 0;                                   // rotation speed
	RE_ROTATION( &ent ) = s1->angles2[2];                   // roll offset

	// add to refresh list
	trap_R_AddRefEntityToScene( &ent );
}

/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime,
								vec3_t out, vec3_t outDeltaAngles ) {
	centity_t   *cent;
	vec3_t      oldOrigin, origin, deltaOrigin;
	vec3_t      oldAngles, angles, deltaAngles;

	if ( outDeltaAngles ) {
		VectorClear( outDeltaAngles );
	}

	if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
		VectorCopy( in, out );
		return;
	}

	cent = &cg_entities[ moverNum ];

	if ( cent->currentState.eType != ET_MOVER && cent->currentState.eType != ET_SCRIPTMOVER ) {
		VectorCopy( in, out );
		return;
	}

	BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	VectorAdd( in, deltaOrigin, out );
	if ( outDeltaAngles ) {
		VectorCopy( deltaAngles, outDeltaAngles );
	}
}

/*
===============
CG_AddCEntity

Inlined into CG_AddPacketEntities by the retail build, which is why nothing
calls the standalone copy.
===============
*/
void CG_AddCEntity( centity_t *cent ) {
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	// call the appropriate function which will add this entity to the view accordingly
	CG_ProcessEntity( cent );
}

/*
===============
CG_SetFrameInterpolation
===============
*/
void CG_SetFrameInterpolation( void ) {
	int delta;

	delta = cg.nextSnap->serverTime - cg.snap->serverTime;
	if ( delta == 0 ) {
		cg.frameInterpolation = 0;
	} else {
		cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		if ( cg.frameInterpolation < 0 ) {
			cg.frameInterpolation = 0;
		}
	}
}

/*
===============
CG_ProcessClientNoteTracks

The animation system hands back the note tracks that fired this frame; the
only ones cgame acts on are the two that move the weapon between hands.
===============
*/
static void CG_ProcessClientNoteTracks( int clientNum ) {
	struct noteTrack_s {
		const char      *name;
		short           notifyType;
		short           unknown_0x06;
		float           unknown_0x08;   /* 12-byte stride: retail 0x3001BCE7 add esi, 0xc */
	} *notes;
	int         numNotes;
	int         i;

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		return;
	}

	numNotes = trap_syscall_0x96( (int)&notes );
	if ( numNotes <= 0 ) {
		return;
	}

	for ( i = 0 ; i < numNotes ; i++ ) {
		if ( notes[i].notifyType != 1 ) {
			continue;
		}

		if ( !_stricmp( notes[i].name, "anim_gunhand = \"left\"" ) ) {
			bg_clientinfo[ clientNum ].gunHandLeft = 1;
		} else if ( !_stricmp( notes[i].name, "anim_gunhand = \"right\"" ) ) {
			bg_clientinfo[ clientNum ].gunHandLeft = 0;
		} else {
			continue;
		}

		bg_clientinfo[ clientNum ].dobjNeedsUpdate = 1;
	}
}

/*
===============
CG_AddPacketEntities

NOTE: every snapshot read below is cg.nextSnap, not cg.snap.  cg.snap is
cg+0x20 and cg.nextSnap is cg+0x24 (CG_SetFrameInterpolation 0x3001BBF3 /
0x3001BBF8, CG_InterpolateEntityPosition 0x3001D098 / 0x3001D0B3), and this
function reads cg+0x24 at all seven of its snapshot accesses.  CoD's own
change; RTCW walks cg.snap here.
===============
*/
void CG_AddPacketEntities( void ) {
	int             num;
	centity_t       *cent;
	playerState_t   *ps;
	cgAutoRotate_t  *ar;
	int             obj;

	trap_syscall_0x94();

	// step every visible client's animation and collect its note tracks
	for ( num = 0 ; num < cg.nextSnap->numEntities ; num++ ) {
		obj = trap_syscall_0xA2( cg.nextSnap->entities[ num ].number );
		if ( obj ) {
			trap_syscall_0x95( obj, cg.frametime * 0.001f );
			CG_ProcessClientNoteTracks( cg.nextSnap->entities[ num ].number );
		}
	}

	cg.shakeMinAmplitude = 0;

	// the auto-rotating items will all have the same axis
	ar = CG_AUTOROTATE();

	ar->autoAnglesSlow[0] = 0;
	ar->autoAnglesSlow[1] = ( cg.time & 4095 ) * 360 / 4095.0f;
	ar->autoAnglesSlow[2] = 0;

	ar->autoAngles[0] = 0;
	ar->autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0f;
	ar->autoAngles[2] = 0;

	ar->autoAnglesFast[0] = 0;
	ar->autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	ar->autoAnglesFast[2] = 0;

	AnglesToAxis( ar->autoAnglesSlow, ar->autoAxisSlow );
	AnglesToAxis( ar->autoAngles, ar->autoAxis );
	AnglesToAxis( ar->autoAnglesFast, ar->autoAxisFast );

	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.nextState, qfalse );
	cg.predictedPlayerEntity.currentState = cg.predictedPlayerEntity.nextState;

	// riding an mg42 puts the local player's own model in the view
	if ( cg.nextSnap->ps.pm_flags & PMF_VIEWLOCKED ) {
		obj = trap_syscall_0xA2( cg.nextSnap->ps.clientNum );
		if ( obj ) {
			trap_syscall_0x95( obj, cg.frametime * 0.001f );
			CG_ProcessClientNoteTracks( cg.nextSnap->ps.clientNum );
		}

		if ( cg.predictedPlayerEntity.currentState.eType < ET_EVENTS ) {
			CG_CalcEntityLerpPositions( &cg.predictedPlayerEntity );
			CG_ProcessEntity( &cg.predictedPlayerEntity );
		}
	}

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.nextSnap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.nextSnap->entities[ num ].number ];
		if ( cent->currentState.eType < ET_EVENTS ) {
			CG_CalcEntityLerpPositions( cent );
			CG_ProcessEntity( cent );
		}
	}

	if ( cg_dumpAnims.integer >= 0 && cg_dumpAnims.integer < MAX_GENTITIES
		 && !cl_paused.integer ) {
		obj = trap_syscall_0xA2( cg_dumpAnims.integer );
		if ( obj ) {
			trap_syscall_0x98( obj );
		}
	}
}

/*
=============================================================================

	THE DObj TAG LAYER

	The cgame half of the game DLL's g_utils_mp.c block: same four functions,
	same order, with a DObj handle where the game passes a gentity_t *.

=============================================================================
*/

void CG_DObjUpdateInfo( int obj ) {
	trap_syscall_0x95( obj, cg.frametime * 0.001f );
}

/*
================
CG_DObjSetLocalTagInternal

Overwrite one bone of the pose with an explicit origin/angles.  The three
angle-to-quaternion helpers are inlined here by the retail build, exactly as
they are in G_DObjSetLocalTagInternal.
================
*/
void CG_DObjSetLocalTagInternal( int obj, const vec3_t origin, const vec3_t angles,
								 int boneIndex ) {
	DObjAnimMat_t   *rotTrans;
	vec4_t          yawQuat;
	vec4_t          pitchQuat;
	vec4_t          rollQuat;
	vec4_t          pitchYawQuat;
	float           halfAngle;

	rotTrans = &( (DObjAnimMat_t *)trap_syscall_0x9E( obj ) )[boneIndex];

	halfAngle = angles[1] * ( (float)M_PI / 360.0f );
	yawQuat[0] = 0.0f;
	yawQuat[1] = 0.0f;
	SinCos( halfAngle, &yawQuat[2], &yawQuat[3] );

	halfAngle = angles[0] * ( (float)M_PI / 360.0f );
	pitchQuat[0] = 0.0f;
	pitchQuat[2] = 0.0f;
	SinCos( halfAngle, &pitchQuat[1], &pitchQuat[3] );

	halfAngle = angles[2] * ( (float)M_PI / 360.0f );
	rollQuat[1] = 0.0f;
	rollQuat[2] = 0.0f;
	SinCos( halfAngle, &rollQuat[0], &rollQuat[3] );

	QuatMultiply( yawQuat, pitchYawQuat, pitchQuat );
	QuatMultiply( pitchYawQuat, rotTrans->quat, rollQuat );

	rotTrans->accumulatedWeight = 0.0f;
	VectorCopy( origin, rotTrans->translation );
}

qboolean CG_DObjSetLocalTag( int obj, int *partBits, const char *tagName,
							 const vec3_t origin, const vec3_t angles ) {
	int boneIndex;

	boneIndex = trap_syscall_0xAE( obj, (int)tagName );
	if ( boneIndex < 0 ) {
		return qfalse;
	}

	if ( !trap_syscall_0x9F( obj, (int)partBits, boneIndex ) ) {
		return qfalse;
	}

	CG_DObjSetLocalTagInternal( obj, origin, angles, boneIndex );

	return qtrue;
}

qboolean CG_DObjSetControlTagAngles( int obj, int *partBits, const char *tagName,
									 const vec3_t angles ) {
	int boneIndex;

	boneIndex = trap_syscall_0xAE( obj, (int)tagName );
	if ( boneIndex < 0 ) {
		return qfalse;
	}

	if ( !trap_syscall_0xA0( obj, (int)partBits, boneIndex ) ) {
		return qfalse;
	}

	CG_DObjSetLocalTagInternal( obj, vec3_origin, angles, boneIndex );

	return qtrue;
}

/*
================
CG_DObjGetLocalTagMatrix

The tag's matrix in the model's own space -- no entity transform applied.
================
*/
float *CG_DObjGetLocalTagMatrix( int obj, const char *tagName, centity_t *cent ) {
	int boneIndex;

	boneIndex = trap_syscall_0xAE( obj, (int)tagName );
	if ( boneIndex < 0 ) {
		return NULL;
	}

	CG_DObjCalcBone( obj, boneIndex, cent );

	return DOBJ_BONEMATRIX( trap_syscall_0x9D( obj, 0 ), boneIndex );
}

/*
================
CG_DObjGetWorldTagMatrix
================
*/
qboolean CG_DObjGetWorldTagMatrix( int obj, const char *tagName, centity_t *cent,
								   float *outMatrix ) {
	int     boneIndex;
	float   *boneMatrix;
	vec3_t  entMatrix[4];
	vec3_t  right;

	boneIndex = trap_syscall_0xAE( obj, (int)tagName );
	if ( boneIndex < 0 ) {
		return qfalse;
	}

	CG_DObjCalcBone( obj, boneIndex, cent );

	boneMatrix = DOBJ_BONEMATRIX( trap_syscall_0x9D( obj, 0 ), boneIndex );
	if ( !boneMatrix ) {
		return qfalse;
	}

	AngleVectors( cent->lerpAngles, entMatrix[0], right, entMatrix[2] );
	VectorSubtract( vec3_origin, right, entMatrix[1] );
	VectorCopy( cent->lerpOrigin, entMatrix[3] );

	DObjSkel2MatrixMultiply43( entMatrix[0], outMatrix, boneMatrix );

	return qtrue;
}

/*
================
CG_DObjGetViewModelTagMatrix

Same thing for the first-person weapon, which is placed by cg.viewModel*
rather than by an entity's lerp origin.
================
*/
qboolean CG_DObjGetViewModelTagMatrix( int obj, const char *tagName, float *outMatrix ) {
	int     boneIndex;
	int     partBits[4];
	vec3_t  viewMatrix[4];

	boneIndex = trap_syscall_0xAE( obj, (int)tagName );
	if ( boneIndex < 0 ) {
		return qfalse;
	}

	if ( !trap_syscall_0xA8( obj, boneIndex ) ) {
		trap_syscall_0xAA( obj, boneIndex, (int)partBits );
		trap_syscall_0x97( obj, (int)partBits );
		trap_syscall_0xAB( obj, (int)partBits );
	}

	VectorCopy( CG_VIEWMODELAXIS()[0], viewMatrix[0] );
	VectorCopy( CG_VIEWMODELAXIS()[1], viewMatrix[1] );
	VectorCopy( CG_VIEWMODELAXIS()[2], viewMatrix[2] );
	VectorCopy( CG_VIEWMODELORIGIN(), viewMatrix[3] );

	DObjSkel2MatrixMultiply43( viewMatrix[0], outMatrix,
							   DOBJ_BONEMATRIX( trap_syscall_0x9D( obj, 0 ), boneIndex ) );

	return qtrue;
}

/*
================
sub_3001C4A0

Transform one point by a 4x4 bone matrix.  Unnamed and unreachable in 1.1 --
nothing calls it.  (com_math.c's MatrixTransformVector43_m is the
three-float-stride version; this one strides four.)
================
*/
static void sub_3001C4A0( const float *matrix, vec3_t out, const vec3_t in ) {
	out[0] = matrix[0] * in[0] + matrix[4] * in[1] + matrix[8] * in[2] + matrix[12];
	out[1] = matrix[1] * in[0] + matrix[5] * in[1] + matrix[9] * in[2] + matrix[13];
	out[2] = matrix[2] * in[0] + matrix[6] * in[1] + matrix[10] * in[2] + matrix[14];
}

/*
================
sub_3001C4F0

The twelve edges of one DObj part's collision box, as 24 endpoints in model
space.  Unnamed; only sub_3001CA90 calls it, and nothing calls that -- this
is the debug box-draw the retail build kept but never runs.
================
*/
static const int cg_boxEdgeCorners[24][3] = {         /* 0x30060DF0 */
	{ 0, 0, 0 }, { 1, 0, 0 },
	{ 0, 0, 0 }, { 0, 1, 0 },
	{ 1, 1, 0 }, { 1, 0, 0 },
	{ 1, 1, 0 }, { 0, 1, 0 },
	{ 0, 0, 1 }, { 1, 0, 1 },
	{ 0, 0, 1 }, { 0, 1, 1 },
	{ 1, 1, 1 }, { 1, 0, 1 },
	{ 1, 1, 1 }, { 0, 1, 1 },
	{ 0, 0, 0 }, { 0, 0, 1 },
	{ 1, 0, 0 }, { 1, 0, 1 },
	{ 0, 1, 0 }, { 0, 1, 1 },
	{ 1, 1, 0 }, { 1, 1, 1 }
};

static qboolean sub_3001C4F0( int obj, const char *tagName, vec3_t *points ) {
	int         boneIndex;
	int         numParts;
	float       **partBounds;
	float       *bounds;
	float       *matrix;
	int         i;

	boneIndex = trap_syscall_0xAE( obj, (int)tagName );
	if ( boneIndex < 0 ) {
		return qfalse;
	}

	numParts = trap_syscall_0xAD( obj );
	partBounds = (float **)_alloca( numParts * sizeof( float * ) );
	trap_syscall_0xB0( obj, (int)partBounds );

	matrix = DOBJ_BONEMATRIX( trap_syscall_0x9D( obj, 0 ), boneIndex );
	bounds = partBounds[ boneIndex ];

	for ( i = 0 ; i < 24 ; i++ ) {
		float x = bounds[ cg_boxEdgeCorners[i][0] * 3 + 0 ];
		float y = bounds[ cg_boxEdgeCorners[i][1] * 3 + 1 ];
		float z = bounds[ cg_boxEdgeCorners[i][2] * 3 + 2 ];

		points[i][0] = x * matrix[0] + y * matrix[4] + z * matrix[8] + matrix[12];
		points[i][1] = x * matrix[1] + y * matrix[5] + z * matrix[9] + matrix[13];
		points[i][2] = x * matrix[2] + y * matrix[6] + z * matrix[10] + matrix[14];
	}

	return qtrue;
}

/*
================
sub_3001CA90

...and the same 24 points rotated and translated into world space.  Unnamed,
and nothing calls it.
================
*/
static qboolean sub_3001CA90( int obj, centity_t *cent, const char *tagName, vec3_t *points ) {
	vec3_t  forward, right, up;
	int     i;

	if ( !sub_3001C4F0( obj, tagName, points ) ) {
		return qfalse;
	}

	AngleVectors( cent->lerpAngles, forward, right, up );

	for ( i = 0 ; i < 24 ; i++ ) {
		float x = points[i][0];
		float y = points[i][1];
		float z = points[i][2];

		points[i][0] = forward[0] * x - right[0] * y + up[0] * z + cent->lerpOrigin[0];
		points[i][1] = forward[1] * x - right[1] * y + up[1] * z + cent->lerpOrigin[1];
		points[i][2] = forward[2] * x - right[2] * y + up[2] * z + cent->lerpOrigin[2];
	}

	return qtrue;
}

/*
===============
CG_SoundBlend

ET_SOUND_BLEND crossfades two sound aliases; leanf carries the blend factor.
===============
*/
static void CG_SoundBlend( centity_t *cent ) {
	const char  *alias1;
	const char  *alias2;
	float       blend;

	if ( !cent->currentState.eventParm ) {
		return;
	}
	if ( !cent->currentState.scale ) {
		return;
	}

	alias1 = trap_Com_PickSoundAlias( CG_ConfigString( CS_SOUNDALIAS + cent->currentState.eventParm ) );
	alias2 = trap_Com_PickSoundAlias( CG_ConfigString( CS_SOUNDALIAS + cent->currentState.scale ) );

	if ( !alias1 || !alias2 ) {
		return;
	}

	blend = cent->currentState.leanf
			+ cg.frameInterpolation * ( cent->nextState.leanf - cent->currentState.leanf );

	trap_MSS_PlayBlendedSoundAliases( alias1, alias2, blend,
									  cent->currentState.number, cent->lerpOrigin, 0 );
}

/*
===============
CG_LoopFx

ET_FX replays one effect on an interval.  angles2[0] is the cull range,
angles2[1] the interval in msec, origin2 the optional direction.
===============
*/
static void CG_LoopFx( centity_t *cent ) {
	int     interval;
	float   range;
	vec3_t  delta;
	int     fxIndex;

	if ( cg.time > CENT_FXTIME( cent ) ) {
		interval = (int)cent->currentState.angles2[1];
		if ( cg.time - CENT_FXTIME( cent ) < interval ) {
			return;
		}
		do {
			CENT_FXTIME( cent ) += interval;
		} while ( cg.time - CENT_FXTIME( cent ) > interval );
	} else {
		CENT_FXTIME( cent ) = cg.time;
	}

	range = cent->currentState.angles2[0];
	if ( range != 0 ) {
		delta[0] = cent->lerpOrigin[0] - cg.predictedPlayerState.origin[0];
		delta[1] = cent->lerpOrigin[1] - cg.predictedPlayerState.origin[1];
		delta[2] = cent->lerpOrigin[2] - cg.predictedPlayerState.origin[2];

		if ( range * range <= DotProduct( delta, delta ) ) {
			return;
		}
	}

	fxIndex = cent->currentState.scale;
	if ( fxIndex <= 0 || fxIndex >= MAX_FX ) {
		CG_Printf( "ERROR: CG_PlayFx called with invalid effect id %i\n", fxIndex );
		return;
	}

	if ( DotProduct( cent->currentState.origin2, cent->currentState.origin2 ) != 0 ) {
		trap_syscall_0xE4( cgs.gameEffects[ fxIndex ], (int)cent->lerpOrigin,
						   (int)cent->currentState.origin2 );
	} else {
		trap_syscall_0xE3( cgs.gameEffects[ fxIndex ], (int)cent->lerpOrigin );
	}
}

/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
	vec3_t          current, next;
	float           f;
	clientInfo_t    *ci;

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

	// a player's view angles drive the animation system, not the model's axis
	if ( cent->nextState.eType == ET_PLAYER ) {
		ci = &bg_clientinfo[ cent->nextState.clientNum ];

		ci->leanAmount = LerpAngle( cent->currentState.angles2[1], cent->nextState.angles2[1], f );
		ci->viewPitch = cent->lerpAngles[0];
		ci->viewYaw = cent->lerpAngles[1];
		ci->viewRoll = cent->lerpAngles[2];

		cent->lerpAngles[0] = 0;
		cent->lerpAngles[2] = 0;

		ci->leanFraction = LerpAngle( cent->currentState.leanf, cent->nextState.leanf, f );
	}
}

/*
===============
CG_CalcEntityLerpPositions
===============
*/
void CG_CalcEntityLerpPositions( centity_t *cent ) {
	clientInfo_t    *ci;

	if ( cent->currentState.pos.trType == TR_INTERPOLATE ) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// fix for jittery clients in multiplayer
	if ( cent->currentState.pos.trType == TR_LINEAR_STOP
		 && cent->currentState.number < MAX_CLIENTS ) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// just use the current frame and evaluate as best we can
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	ci = NULL;
	if ( cent->currentState.eType == ET_PLAYER ) {
		ci = &bg_clientinfo[ cent->currentState.clientNum ];
	} else if ( cent->currentState.eType == ET_CORPSE ) {
		ci = &cgs.corpseinfo[ cent->currentState.number - MAX_CLIENTS ];
	}

	if ( ci ) {
		ci->leanAmount = cent->currentState.angles2[1];
		ci->viewPitch = cent->lerpAngles[0];
		ci->viewYaw = cent->lerpAngles[1];
		ci->viewRoll = cent->lerpAngles[2];

		cent->lerpAngles[2] = 0;
		cent->lerpAngles[0] = 0;

		ci->leanFraction = cent->currentState.leanf;
	}

	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if ( cent != &cg.predictedPlayerEntity ) {
		CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum,
								   cg.snap->serverTime, cg.time, cent->lerpOrigin, NULL );
	}
}

/*
===============
CG_GetAnimations
===============
*/
int CG_GetAnimations( int eType, int entityNum ) {
	if ( eType != ET_MG42 ) {
		return 0;
	}
	return CG_GetMG42Anims( &cg_entities[ entityNum ] );
}

/*
===============
CG_PreProcess_GetDObj

Keep one DObj alive per entity, rebuilding it whenever the entity's type or
XModel changes.  cg.iEntityLastType / cg.pEntityLastXModel are the cache.
===============
*/
int CG_PreProcess_GetDObj( int entityNum, int eType, qhandle_t hModel ) {
	int             obj;
	void            *xmodel;
	int             anims;
	int             tree;
	DObjModel       models[1];

	obj = trap_syscall_0xA2( entityNum );
	xmodel = (void *)trap_R_GetXModelByHandle( hModel );

	if ( obj ) {
		if ( xmodel
			 && cg.iEntityLastType[ entityNum ] == eType
			 && cg.pEntityLastXModel[ entityNum ] == xmodel ) {
			return obj;
		}

		trap_SafeDObjFree( entityNum, 1 );
		obj = 0;
		cg.iEntityLastType[ entityNum ] = 0;
		cg.pEntityLastXModel[ entityNum ] = 0;
	}

	if ( !xmodel ) {
		return obj;
	}

	tree = 0;
	if ( eType == ET_MG42 ) {
		anims = CG_GetMG42Anims( &cg_entities[ entityNum ] );
		if ( anims ) {
			tree = trap_XAnimCreateTree( anims );
		}
	}

	models[0].model = xmodel;
	models[0].tagName = NULL;
	models[0].boneName = 0;
	models[0].unused_0x0A = 0;
	/* the base model's part name is the model handle here, not the negated
	   model index the game DLL writes (0x3001D3E3) */
	models[0].boneName = (short)hModel;

	trap_DObjCreate( models, 1, (void *)tree, entityNum );

	cg.iEntityLastType[ entityNum ] = eType;
	cg.pEntityLastXModel[ entityNum ] = xmodel;

	return trap_syscall_0xA2( entityNum );
}

/*
===============
CG_Player_DoControllers

Duplicated inline into CG_DoControllers by the retail build, which is why
nothing calls this copy.
===============
*/
void CG_Player_DoControllers( centity_t *cent, int *partBits ) {
	int             obj;
	clientInfo_t    *ci;

	obj = trap_syscall_0xA2( cent->currentState.number );
	ci = &bg_clientinfo[ cent->currentState.clientNum ];

	if ( !ci->infoValid ) {
		return;
	}

	BG_Player_DoControllers( ci, obj, &cent->currentState, partBits );
}

/*
===============
CG_DoControllers
===============
*/
void CG_DoControllers( centity_t *cent, int *partBits ) {
	int             obj;
	clientInfo_t    *ci;

	switch ( cent->currentState.eType ) {
	case ET_PLAYER:
		obj = trap_syscall_0xA2( cent->currentState.number );
		ci = &bg_clientinfo[ cent->currentState.clientNum ];
		if ( ci->infoValid ) {
			BG_Player_DoControllers( ci, obj, &cent->currentState, partBits );
		}
		break;
	case ET_MG42:
		CG_mg42_DoControllers( cent, partBits );
		break;
	default:
		break;
	}
}

/*
===============
CG_DObjCalcPose

vmMain export 10.  The renderer hands back the refEntity's owner and dobj.
0x3001D4B0 takes cent on the stack and obj/partBits in edi/eax, and pushes
them to the traps in that order.
===============
*/
void CG_DObjCalcPose( centity_t *cent, int obj, int *partBits ) {
	if ( trap_syscall_0xA9( obj, (int)partBits ) ) {
		return;
	}

	trap_syscall_0x97( obj, (int)partBits );
	CG_DoControllers( cent, partBits );
	trap_syscall_0xAB( obj, (int)partBits );
}

/*
===============
CG_DObjCalcBone
===============
*/
void CG_DObjCalcBone( int obj, int boneIndex, centity_t *cent ) {
	int partBits[4];

	if ( trap_syscall_0xA8( obj, boneIndex ) ) {
		return;
	}

	trap_syscall_0xAA( obj, boneIndex, (int)partBits );
	trap_syscall_0x97( obj, (int)partBits );
	CG_DoControllers( cent, partBits );
	trap_syscall_0xAB( obj, (int)partBits );
}

/*
===============
CG_DObjCalcBoneGeneric

vmMain export 11.  Takes the entity number rather than a centity_t, and
looks the DObj up itself.
===============
*/
void CG_DObjCalcBoneGeneric( int entityNum, int boneIndex ) {
	int obj;
	int partBits[4];

	obj = trap_syscall_0xA2( entityNum );

	if ( trap_syscall_0xA8( obj, boneIndex ) ) {
		return;
	}

	trap_syscall_0xAA( obj, boneIndex, (int)partBits );
	trap_syscall_0x97( obj, (int)partBits );

	if ( entityNum < MAX_GENTITIES ) {
		CG_DoControllers( &cg_entities[ entityNum ], partBits );
	}

	trap_syscall_0xAB( obj, (int)partBits );
}

/*
===============
CG_ProcessEntity
===============
*/
void CG_ProcessEntity( centity_t *cent ) {
	// add automatic effects
	CG_EntityEffects( cent );

	switch ( cent->currentState.eType ) {
	case ET_GENERAL:
		CG_General( cent );
		break;
	case ET_PLAYER:
		CG_Player( cent );
		break;
	case ET_CORPSE:
		CG_Corpse( cent );
		break;
	case ET_ITEM:
		CG_Item( cent );
		break;
	case ET_MISSILE:
		CG_Missile( cent );
		break;
	case ET_MOVER:
		CG_Mover( cent );
		break;
	case ET_PORTAL:
		CG_Portal( cent );
		break;
	case ET_INVISIBLE:
		break;
	case ET_SCRIPTMOVER:
		CG_ScriptMover( cent );
		break;
	case ET_SOUND_BLEND:
		CG_SoundBlend( cent );
		break;
	case ET_FX:
		CG_LoopFx( cent );
		break;
	case ET_MG42:
		CG_mg42( cent );
		break;
	default:
		CG_Error( "Bad entity type: %i\n", cent->currentState.eType );
		break;
	}
}
