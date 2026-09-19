/*
 * g_misc_mp.c -- the odds and ends: the positional info_* spawns, the
 * teleporter, the corona, misc_spawner, and CoD's player-operated turret.
 *
 * Call of Duty 1.1 multiplayer (game_mp_x86.dll, 0x20027180 .. 0x200295CC).
 * The head of the file is RTCW's game/g_misc.c almost verbatim -- info_camp /
 * info_null / info_notnull / light / TeleportPlayer / misc_teleporter_dest /
 * misc_model / corona -- with every RTCW-specific misc entity (the grabber,
 * the gamemodels, the portal surfaces, the spotlights, the flak/mg42 mounts)
 * cut.  Everything from G_InitTurrets down is CoD's own and has no RTCW
 * ancestor: a fixed pool of 32 turret_t records, a player who is view-locked
 * to one while he uses it, and the arc/rate clamping that keeps the barrel
 * inside the weapon file's limits.
 *
 * Function order is binary order.
 *
 * @fidelity: likely
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bg_public.h"
#include "g_local.h"

/* bg_misc.c's eventnames[] indices. */
#define EV_STANCE_FORCE_STAND       140
#define EV_STANCE_FORCE_CROUCH      141
#define EV_STANCE_FORCE_PRONE       142
#define EV_FIRE_WEAPON_MG42         168
#define EV_PLAYER_TELEPORT_IN       199
#define EV_PLAYER_TELEPORT_OUT      200

/* g_public.h carries these in the original tree. */
#define ET_TURRET                   11
#define ENTITYNUM_WORLD             1022
#define TR_LINEAR_STOP              3

#define SVF_BROADCAST               128

/* playerState_t.eFlags.  0x4000 and 0x8000 are the pair turret_use sets from
 * turret_t.useMode and G_ClientStopUsingTurret clears; TeleportPlayer gates
 * its knockback on the same pair. */
#define EF_TELEPORT_BIT             0x00000008
#define EF_TURRET_STANCE_MASK       0x0000C000

/* playerState_t.pm_flags */
#define PMF_TIME_KNOCKBACK          0x00000200

/* bg_animation.c's copies; no header carries either. */
#define ANIM_TOGGLEBIT              0x200
#define ANIMFL_TURRET               0x004

/* trap_DObjGetMatrixArray's entries are 64 bytes -- three 4-float rows and
   then the translation at +0x30 -- so a matrix43_t does not overlay one.
   G_DObjGetLocalTagMatrix hands back a float * into that array. */
#define DOBJ_TAG_AXIS               0
#define DOBJ_TAG_ORIGIN             12

/* The content mask G_PlayerTurretPositionAndBlend traces with (0x20027FDA).
   No name for it is recovered in this module. */
#define MASK_TURRET_PLAYER          0x02810011

/* level.time is advanced in 100 msec steps, but the turret code runs off the
 * 50 msec client frame. */
#define FRAMETIME                   100
#define TURRET_FRAMETIME            50

/* The pool G_InitTurrets clears, 0x20089730 .. 0x20089F30. */
#define MAX_TURRETS                 32

/*
 * The per-turret state record.  gentity_t.turret points at one of the 32 in
 * the fixed pool; G_SpawnTurret takes the first free slot and G_FreeTurret
 * releases it.  Field names are inferred from United Offensive, which has this
 * same record with a heat/overheating pair appended (0x48 there against 0x40
 * here).
 *
 * minArc/maxArc are one float pair each rather than the four separately named
 * arcs UO uses, because turret_clientaim (0x2002810F) and turret_use
 * (0x20028E95) index them: [0] is the pitch pair, [1] the yaw pair.  The
 * spawn keys that fill them are "toparc"/"rightarc" (min, forced <= 0) and
 * "bottomarc"/"leftarc" (max, forced >= 0).
 */
typedef struct turret_s {
	int inUse;                      /* +0x00 pool slot taken (G_SpawnTurret 0x20029000) */
	int flags;                      /* +0x04 3 at spawn; 0x100/0x200/0x400 drive the pitch
									       return, 0x800 means "eat the next teleport bit" */
	int fireTimeRemaining;          /* +0x08 counts down by 50 in turret_track */
	float minArc[2];                /* +0x0C pitch, +0x10 yaw */
	float maxArc[2];                /* +0x14 pitch, +0x18 yaw */
	float restPitch;                /* +0x1C -90 until turret_think_init traces for it */
	int useMode;                    /* +0x20 weaponInfo_t.stance (G_SpawnTurret 0x2002907F) */
	int stopUseEventType;           /* +0x24 -1 / 0 / 1 / 2 -> EV_STANCE_FORCE_* */
	int fireSoundTime;              /* +0x28 counts the sustained-fire loop down */
	vec3_t stopUseOrigin;           /* +0x2C where the player is put back */
	float restPitchClamp;           /* +0x38 turret_UpdateTargetAngles 0x20028590 */
	byte sustainedFireLoopSound;    /* +0x3C weaponInfo_t.loopFireSound, as an alias index */
	byte sustainedFireStopSound;    /* +0x3D weaponInfo_t.stopFireSound */
	byte pad_0x3E[2];
} turret_t;
G_ASSERT_SIZE( turret_t, 64 );

turret_t g_turrets[MAX_TURRETS];        /* 0x20089730 */

/* forward: bg_weapon.c */
byte BG_GetWeaponIndexForName( const char *name );

/* forward: g_utils_mp.c -- the bone-space form of G_DObjSetLocalTag, which
   takes a resolved bone index instead of a tag name (0x2003B790). */
void G_DObjSetLocalTagInternal( gentity_t *ent, const vec3_t origin, const vec3_t angles,
								int boneIndex );

/* forward: bg_animation.c's globals -- 0x200A1C80. */
extern animScriptData_t bgs_animScriptData;

/* forward: universal/com_math.c */
float AngleSubtract( float a1, float a2 );
float AngleNormalize180( float angle );
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void AxisToAngles( vec3_t axis[3], vec3_t angles );
void MatrixTransformVector43( const matrix43_t *mat, vec3_t out, const vec3_t in );
void MatrixMultiply43( const void *in0, const void *in1, void *out );
float vectosignedyaw( const vec3_t vec );
void RotateVector2D( vec3_t v, float degrees );
float RotationToYaw( const vec4_t rotation );
void YawToAxis( float yaw, vec3_t axis[3] );   /* universal/com_math.c: yaw on the stack, axis in edx (retail 0x20016330 __usercall) -- the register order is not the parameter order */


void nullsub_2( void ) {
}


void nullsub_3( void ) {
}


/*
 * 0x200271A0.  Two instructions, `fld [esp+4]; retn`: a float returned
 * unchanged.  Nothing in the module references it and it is byte-identical to
 * q_shared.c's FloatNoSwap (0x2003E420), so the original name is not
 * recoverable; the address placeholder is kept rather than a name inferred.
 */
float sub_200271A0( float f ) {
	return f;
}


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( gentity_t *self ) {
	G_SetOrigin( self, self->r.currentOrigin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
	G_FreeEntity( self );
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
*/
void SP_info_notnull( gentity_t *self ) {
	G_SetOrigin( self, self->r.currentOrigin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) nonlinear angle negative_spot negative_point q3map_non-dynamic
Non-displayed light.
*/
void SP_light( gentity_t *self ) {
	G_FreeEntity( self );
}


/*
=================================================================================

TELEPORTERS

=================================================================================
*/
void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles ) {
	gentity_t   *tent;
	qboolean linked;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( player->client->sess.sessionState == STATE_PLAYING ) {
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	linked = player->r.linked;

	// unlink to make sure it can't possibly interfere with anything
	trap_UnlinkEntity( player );

	VectorCopy( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	/* The original wrote `|` where it meant `&` (0x20027333 is an OR, not a
	   TEST), so the guard is always true and the block below is unreachable in
	   the shipped binary.  Reproduced as-is. */
	if ( !( player->client->ps.eFlags | EF_TURRET_STANCE_MASK ) ) {
		// spit the player out
		AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
		VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
		player->client->ps.pm_time = 160;       // hold time
		player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	}

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// set angles
	SetClientViewAngle( player, angles );

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( linked ) {
		trap_LinkEntity( player );
	}
}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
}


/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
Turned into a static brush model by the utilities; removed at load.
*/
void SP_misc_model( gentity_t *ent ) {
	G_FreeEntity( ent );
}


void use_corona( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	} else {
		ent->active = 0;
		trap_LinkEntity( ent );
	}
}


/*QUAKED corona (0 1 0) (-4 -4 -4) (4 4 4) START_OFF
Client-side only in 1.1; the game module just drops it.
*/
void SP_corona( gentity_t *ent ) {
	G_FreeEntity( ent );
}


/*
=================================================================================

TURRETS

=================================================================================
*/

void G_InitTurrets( void ) {
	int i;

	for ( i = 0; i < MAX_TURRETS; i++ ) {
		g_turrets[i].inUse = 0;
	}
}


/*
==============
Turret_FillWeaponParms

Build the fire block for a shot from the barrel: the axis comes from where the
gunner is looking, and the muzzle is pushed out along it by the distance from
the player tag to the flash tag, so a rotated barrel still fires from its own
tip.
==============
*/
static qboolean Turret_FillWeaponParms( gentity_t *self, gentity_t *player,
										weaponFireInfo_t *fireInfo ) {
	/* G_DObjGetWorldTagMatrix writes a full 4x4 (DObjSkel2MatrixMultiply43 sets
	   the padding column, [3][3]=1.0 at +0x3C and the translation at row 3,
	   +0x30); a 48-byte matrix43_t would overflow by 16 bytes and its .origin
	   at +0x24 would read a rotation term.  Retail frames these two as 64-byte
	   matrices (sub esp,0x8c) and reads the muzzle at row 3 -- v17[12]/v14[11],
	   0x200275f7/0x2002763c. */
	float flashMat[4][4];
	float playerMat[4][4];
	vec3_t dir;
	float dist;

	if ( !G_DObjGetWorldTagMatrix( self, "tag_flash", flashMat ) ) {
		Com_Printf( "Couldn't find %s on turret (entity %d, classname '%s').\n",
					"tag_flash", self->s.number, SL_ConvertToString( self->classname ) );
		return qfalse;
	}

	if ( !G_DObjGetWorldTagMatrix( self, "tag_player", playerMat ) ) {
		Com_Printf( "Couldn't find %s on turret (entity %d, classname '%s').\n",
					"tag_player", self->s.number, SL_ConvertToString( self->classname ) );
		return qfalse;
	}

	AngleVectors( player->client->ps.viewangles, fireInfo->forward, fireInfo->right,
				  fireInfo->up );

	/* +0x30 of weaponFireInfo_t is a vec3_t; g_local.h still calls it
	   unknown_0x30 (the store is 0x20027606). */
	VectorCopy( fireInfo->forward, (float *)fireInfo->unknown_0x30 );

	VectorSubtract( flashMat[3], playerMat[3], dir );
	dist = VectorNormalize( dir );

	VectorMA( playerMat[3], dist, fireInfo->forward, fireInfo->start );

	return qtrue;
}


static void Fire_Lead( gentity_t *self, gentity_t *attacker, int damage ) {
	weaponFireInfo_t fireInfo;
	vec3_t end;

	if ( attacker == &g_entities[ENTITYNUM_NONE] ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	if ( !Turret_FillWeaponParms( self, attacker, &fireInfo ) ) {
		return;
	}

	fireInfo.weapInfo = bg_weaponInfo[self->s.weapon];

	if ( fireInfo.weapInfo->weaponType ) {
		Weapon_RocketLauncher_Fire( self, &fireInfo, 0 );
		G_AddEvent( self, EV_FIRE_WEAPON_MG42, 0 );
	} else {
		Bullet_Endpos( &fireInfo, end, 0 );
		Bullet_Fire_Extended( self, attacker, fireInfo.start, end, damage, 0, &fireInfo, self );
		G_AddEvent( self, EV_FIRE_WEAPON_MG42, 0 );
	}
}


/*
==============
G_PlayerTurretPositionAndBlend

Stand the gunner where his own turret animation says he stands.

The animation is a two level XAnim tree.  The root's children are the mount's
vertical stops and each of those children's children are its horizontal stops,
so the barrel's yaw picks a grandchild pair and the height the player has to
reach picks a child pair; the four weights between them are the blend.  Walking
the root's children until one of them lifts the player high enough gives the
bracketing pair, and the pose that comes out of it -- its absolute delta folded
through tag_weapon and then through the turret's own transform -- is the
player's new origin and facing.

Every goal weight is driven at the rate that closes what is left of the gap in
one server frame.
==============
*/
void G_PlayerTurretPositionAndBlend( gentity_t *player, gentity_t *turret ) {
	clientInfo_t        *ci;
	const weaponInfo_t  *weapInfo;
	void                *animTree;
	const float         *weaponMat;
	scr_anim_t rootAnim;
	scr_anim_t childAnim;
	scr_anim_t prevChildAnim;
	scr_anim_t grandChildAnim;
	scr_anim_t nextGrandChildAnim;
	matrix43_t turretMat;
	matrix43_t barrelMat;
	matrix43_t playerMat;
	trace_t tr;
	vec3_t start, end;
	vec3_t delta;
	vec4_t deltaRot;
	vec3_t deltaMove;
	float weaponYaw;
	float alongUp, targetAlongUp;
	float yawPos, frac, lastFrac;
	float lastAlongUp, blend;
	float goalWeight, goalTime;
	/* the x87 stack never rounds this to a float; only goalTime is spilled */
	double rate;
	int yawIndex, lastYawIndex;
	int i, numRootChildren, numChildren;

	ci = &bg_clientinfo[player->s.clientNum];

	if ( !ci->legs.animationNumber ) {
		return;
	}
	if ( !ci->legs.animation ) {
		return;
	}
	if ( !( ci->legs.animation->flags & ANIMFL_TURRET ) ) {
		return;
	}

	weaponMat = G_DObjGetLocalTagMatrix( turret, "tag_weapon" );
	if ( !weaponMat ) {
		Com_Printf( "WARNING: aborting player positioning on turret since 'tag_weapon' does not exist\n" );
		return;
	}

	weapInfo = bg_weaponInfo[turret->s.weapon];
	animTree = ci->animTree;

	rootAnim.anims = (unsigned short)Scr_GetAnimsIndex( bgs_animScriptData.animTreeIndex );
	rootAnim.index = (unsigned short)( ci->legs.animationNumber & ~ANIM_TOGGLEBIT );

	weaponYaw = vectosignedyaw( &weaponMat[DOBJ_TAG_AXIS] );

	AnglesToAxis( turret->r.currentAngles, turretMat.axis );
	VectorCopy( turret->r.currentOrigin, turretMat.origin );

	/* How far up the turret's own up axis the player already sits, and how far
	   up it the pose has to put him: the barrel's tag is the reference. */
	VectorSubtract( player->r.currentOrigin, turretMat.origin, delta );
	alongUp = DotProduct( delta, turretMat.axis[2] );
	targetAlongUp = alongUp - weaponMat[DOBJ_TAG_ORIGIN + 2];

	trap_XAnimClearTreeGoalWeightsStrict( animTree, rootAnim.index, 0.0f );

	numRootChildren = trap_XAnimGetNumChildren( rootAnim );

	lastAlongUp = 0;
	lastFrac = 0;
	lastYawIndex = 0;
	nextGrandChildAnim.index = 0;
	nextGrandChildAnim.anims = 0;

	if ( !numRootChildren ) {
		Com_Error( ERR_DROP, "\x15" "Player anim '%s' has no children",
				   trap_XAnimGetAnimName( rootAnim ) );
	}

	for ( i = 0; i < numRootChildren; i++ ) {
		childAnim = trap_XAnimGetChildAt( rootAnim, i );
		trap_XAnimSetGoalWeight( animTree, childAnim.index, 1.0f, 1.0f, 1.0f, 0, 0 );

		numChildren = trap_XAnimGetNumChildren( childAnim );
		if ( !numChildren ) {
			Com_Error( ERR_DROP, "\x15" "Player anim '%s' has no children",
					   trap_XAnimGetAnimName( childAnim ) );
		}

		/* The horizontal stops run either side of centre, one every
		   animHorRotateInc degrees. */
		yawPos = numChildren * 0.5f - weaponYaw / weapInfo->animHorRotateInc;
		if ( yawPos < 0 ) {
			yawPos = 0;
		} else if ( yawPos >= (float)( numChildren - 1 ) ) {
			yawPos = (float)( numChildren - 1 );
		}

		yawIndex = (int)yawPos;
		frac = yawPos - yawIndex;

		grandChildAnim = trap_XAnimGetChildAt( childAnim, yawIndex );
		trap_XAnimSetGoalWeight( animTree, grandChildAnim.index, 1.0f - frac,
								 1.0f, 1.0f, 0, 0 );

		if ( frac != 0 ) {
			nextGrandChildAnim = trap_XAnimGetChildAt( childAnim, yawIndex + 1 );
			trap_XAnimSetGoalWeight( animTree, nextGrandChildAnim.index, frac,
									 1.0f, 1.0f, 0, 0 );
		}

		trap_XAnimCalcAbsDelta( animTree, childAnim.index, deltaRot, deltaMove );

		if ( deltaMove[2] >= targetAlongUp ) {
			break;
		}

		lastAlongUp = deltaMove[2];
		lastFrac = frac;
		lastYawIndex = yawIndex;
	}

	trap_XAnimClearTreeGoalWeightsStrict( animTree, rootAnim.index, 0.0f );

	goalWeight = 1.0f - frac;
	rate = fabs( trap_XAnimGetWeight( animTree, grandChildAnim.index ) - goalWeight )
		   * ( 1000.0f / level.frameTime );
	if ( rate > 0 ) {
		goalTime = (float)( 1.0f / rate );
	} else {
		goalTime = 0;
	}
	trap_XAnimSetGoalWeight( animTree, grandChildAnim.index, goalWeight,
							 goalTime, 1.0f, 0, 0 );

	if ( frac != 0 ) {
		rate = fabs( trap_XAnimGetWeight( animTree, nextGrandChildAnim.index ) - frac )
			   * ( 1000.0f / level.frameTime );
		if ( rate > 0 ) {
			goalTime = (float)( 1.0f / rate );
		} else {
			goalTime = 0;
		}
		trap_XAnimSetGoalWeight( animTree, nextGrandChildAnim.index, frac,
								 goalTime, 1.0f, 0, 0 );
	}

	if ( i != 0 && i != numRootChildren ) {
		/* The walk stopped on a child that overshoots: split the two vertical
		   stops it sits between, and give the lower one the yaw pair the walk
		   was holding when it left it. */
		blend = ( targetAlongUp - lastAlongUp ) / ( deltaMove[2] - lastAlongUp );

		rate = fabs( trap_XAnimGetWeight( animTree, childAnim.index ) - blend )
			   * ( 1000.0f / level.frameTime );
		if ( rate > 0 ) {
			goalTime = (float)( 1.0f / rate );
		} else {
			goalTime = 0;
		}
		trap_XAnimSetGoalWeight( animTree, childAnim.index, blend,
								 goalTime, 1.0f, 0, 0 );

		prevChildAnim = trap_XAnimGetChildAt( rootAnim, i - 1 );
		goalWeight = 1.0f - blend;
		rate = fabs( trap_XAnimGetWeight( animTree, prevChildAnim.index ) - goalWeight )
			   * ( 1000.0f / level.frameTime );
		if ( rate > 0 ) {
			goalTime = (float)( 1.0f / rate );
		} else {
			goalTime = 0;
		}
		trap_XAnimSetGoalWeight( animTree, prevChildAnim.index, goalWeight,
								 goalTime, 1.0f, 0, 0 );

		grandChildAnim = trap_XAnimGetChildAt( prevChildAnim, lastYawIndex );
		goalWeight = 1.0f - lastFrac;
		rate = fabs( trap_XAnimGetWeight( animTree, grandChildAnim.index ) - goalWeight )
			   * ( 1000.0f / level.frameTime );
		if ( rate > 0 ) {
			goalTime = (float)( 1.0f / rate );
		} else {
			goalTime = 0;
		}
		trap_XAnimSetGoalWeight( animTree, grandChildAnim.index, goalWeight,
								 goalTime, 1.0f, 0, 0 );

		if ( lastFrac != 0 ) {
			grandChildAnim = trap_XAnimGetChildAt( prevChildAnim, lastYawIndex + 1 );
			rate = fabs( trap_XAnimGetWeight( animTree, grandChildAnim.index ) - lastFrac )
				   * ( 1000.0f / level.frameTime );
			if ( rate > 0 ) {
				goalTime = (float)( 1.0f / rate );
			} else {
				goalTime = 0;
			}
			trap_XAnimSetGoalWeight( animTree, grandChildAnim.index, lastFrac,
									 goalTime, 1.0f, 0, 0 );
		}
	} else {
		/* The walk ran off either end: there is nothing to split, so the last
		   child takes the whole weight.  The tag lookup is only here for the
		   bone recalculation it does on the way. */
		if ( !G_DObjGetLocalTagMatrix( turret, "tag_aim" ) ) {
			Com_Printf( "WARNING: aborting player positioning on turret since 'tag_aim' does not exist\n" );
			return;
		}

		rate = fabs( trap_XAnimGetWeight( animTree, childAnim.index ) - 1.0f )
			   * ( 1000.0f / level.frameTime );
		if ( rate > 0 ) {
			goalTime = (float)( 1.0f / rate );
		} else {
			goalTime = 0;
		}
		trap_XAnimSetGoalWeight( animTree, childAnim.index, 1.0f,
								 goalTime, 1.0f, 0, 0 );
	}

	trap_XAnimCalcAbsDelta( animTree, rootAnim.index, deltaRot, deltaMove );

	/* The pose's delta is in the barrel's own frame; tag_weapon's yaw turns it
	   back into the turret's, and its origin puts it on the barrel.  The
	   height stays the one the player already had. */
	RotateVector2D( deltaMove, weaponYaw );
	barrelMat.origin[0] = deltaMove[0] + weaponMat[DOBJ_TAG_ORIGIN + 0];
	barrelMat.origin[1] = deltaMove[1] + weaponMat[DOBJ_TAG_ORIGIN + 1];
	barrelMat.origin[2] = alongUp;

	YawToAxis( RotationToYaw( deltaRot ) + weaponYaw, barrelMat.axis );

	MatrixMultiply43( &barrelMat, &turretMat, &playerMat );

	player->client->ps.origin[0] = playerMat.origin[0];
	player->client->ps.origin[1] = playerMat.origin[1];
	player->client->ps.origin[2] = playerMat.origin[2];

	/* Come up from the turret's own base rather than dropping the player
	   through whatever the pose put him inside of. */
	VectorCopy( player->client->ps.origin, start );
	VectorCopy( player->client->ps.origin, end );
	start[2] = turret->r.currentOrigin[2];

	trap_Trace( &tr, start, NULL, NULL, end, player->s.number, MASK_TURRET_PLAYER );
	if ( tr.fraction < 1.0f ) {
		player->client->ps.origin[2] = tr.endpos[2];
	}

	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	VectorCopy( player->client->ps.origin, player->r.currentOrigin );
	AxisToAngles( playerMat.axis, player->r.currentAngles );

	trap_LinkEntity( player );
}


/*
==============
turret_clientaim

Clamp the gunner's view into the turret's arcs and into a 15 degree per frame
slew, store the result as the barrel's aim (s.angles2), and snap his view back
whenever the clamp actually bit.
==============
*/
static void turret_clientaim( gentity_t *player, gentity_t *turret ) {
	turret_t *t;
	vec3_t angles;
	float aim[2];
	float diff;
	qboolean clamped;
	int i;

	t = (turret_t *)turret->turret;

	player->client->ps.viewlocked = 1;
	player->client->ps.viewlocked_entNum = turret->s.number;
	player->client->ps.gunfx = 0;

	clamped = qfalse;

	for ( i = 0; i < 2; i++ ) {
		aim[i] = AngleSubtract( player->client->ps.viewangles[i], turret->r.currentAngles[i] );

		if ( aim[i] > t->maxArc[i] ) {
			aim[i] = t->maxArc[i];
			clamped = qtrue;
		} else if ( aim[i] < t->minArc[i] ) {
			aim[i] = t->minArc[i];
			clamped = qtrue;
		}

		diff = AngleSubtract( aim[i], turret->s.angles2[i] );
		if ( fabs( diff ) > 15.0 ) {
			clamped = qtrue;
			if ( diff > 0 ) {
				aim[i] = turret->s.angles2[i] + 15.0f;
			} else {
				aim[i] = turret->s.angles2[i] - 15.0f;
			}
		}
	}

	turret->s.angles2[0] = aim[0];
	turret->s.angles2[1] = aim[1];
	turret->s.angles2[2] = 0;

	if ( t->flags & 0x800 ) {
		t->flags &= ~0x800;
		turret->s.eFlags ^= EF_TELEPORT_BIT;
	}

	if ( clamped ) {
		angles[2] = 0;
		angles[0] = aim[0] + turret->r.currentAngles[0];
		angles[1] = aim[1] + turret->r.currentAngles[1];
		SetClientViewAngle( player, angles );
	}
}


static void turret_shoot_internal( gentity_t *self, gentity_t *player ) {
	( (turret_t *)self->turret )->fireSoundTime =
		3 * bg_weaponInfo[self->s.weapon]->fireTime;

	if ( player->client ) {
		Fire_Lead( self, player, self->damage );
		player->client->ps.viewlocked = 2;
	}
}


static void turret_track( gentity_t *player, gentity_t *turret ) {
	turret_t *t;
	const weaponInfo_t *weapon;

	t = (turret_t *)turret->turret;

	turret_clientaim( player, turret );
	G_PlayerTurretPositionAndBlend( player, turret );

	weapon = bg_weaponInfo[turret->s.weapon];

	player->client->ps.viewlocked = 1;
	turret->s.eFlags &= ~0x400;

	t->fireTimeRemaining -= TURRET_FRAMETIME;
	if ( t->fireTimeRemaining <= 0 ) {
		t->fireTimeRemaining = 0;

		if ( player->client->buttons & 1 ) {
			t->fireTimeRemaining = weapon->fireTime;
			turret_shoot_internal( turret, player );
			turret->s.eFlags |= 0x400;
		}
	}
}


static void turret_UpdateSound( gentity_t *self ) {
	turret_t *turret;

	turret = (turret_t *)self->turret;

	self->s.loopSound = 0;

	if ( turret->fireSoundTime > 0 ) {
		self->s.loopSound = turret->sustainedFireLoopSound;

		turret->fireSoundTime -= TURRET_FRAMETIME;
		if ( turret->fireSoundTime <= 0 && turret->sustainedFireStopSound ) {
			self->s.loopSound = 0;
			G_PlaySoundAlias( self, turret->sustainedFireStopSound );
		}
	}
}


void G_ClientStopUsingTurret( gentity_t *self ) {
	gentity_t *player;
	turret_t *turret;
	int event;

	player = &g_entities[self->r.ownerNum];
	turret = (turret_t *)self->turret;

	turret->fireSoundTime = 0;
	self->s.loopSound = 0;

	if ( turret->stopUseEventType != -1 ) {
		if ( turret->stopUseEventType == 2 ) {
			event = EV_STANCE_FORCE_PRONE;
		} else if ( turret->stopUseEventType == 1 ) {
			event = EV_STANCE_FORCE_CROUCH;
		} else {
			event = EV_STANCE_FORCE_STAND;
		}
		G_AddEvent( player, event, 0 );
		turret->stopUseEventType = -1;
	}

	TeleportPlayer( player, turret->stopUseOrigin, player->r.currentAngles );

	player->client->ps.eFlags &= ~EF_TURRET_STANCE_MASK;
	player->client->ps.viewlocked = 0;
	player->client->ps.viewlocked_entNum = ENTITYNUM_NONE;
	player->client->ps.gunfx = 0;
	player->active = 0;
	player->s.otherEntityNum = 0;

	self->active = 0;
	self->r.ownerNum = ENTITYNUM_NONE;

	turret->flags &= ~0x800;
}


void turret_think_client( gentity_t *self ) {
	gentity_t *player;

	player = &g_entities[self->r.ownerNum];

	if ( player->active != 1 || player->client->sess.sessionState != STATE_PLAYING ) {
		G_ClientStopUsingTurret( self );
		return;
	}

	turret_track( player, self );
	turret_UpdateSound( self );
}


/*
==============
turret_UpdateTargetAngles

Step the barrel towards targetAngles, no faster than the weapon's turn rates
(or 200 deg/sec when driven without them).  s.angles2[2] carries the pitch
that has not been applied yet, so that the pitch can be re-clamped against
restPitchClamp after the step.  Returns true once nothing had to be limited.
==============
*/
static qboolean turret_UpdateTargetAngles( gentity_t *ent, const float *targetAngles,
										   int useWeaponRates ) {
	turret_t *turret;
	float savePitch;
	float maxStep[2];
	float target;
	float diff;
	qboolean atRest;
	int i;

	turret = (turret_t *)ent->turret;

	savePitch = ent->s.angles2[0];
	ent->s.angles2[0] = ent->s.angles2[0] + ent->s.angles2[2];

	atRest = qtrue;

	if ( useWeaponRates ) {
		maxStep[0] = bg_weaponInfo[ent->s.weapon]->vertTurnSpeed;
		maxStep[1] = bg_weaponInfo[ent->s.weapon]->horTurnSpeed;
	} else {
		maxStep[0] = 200.0f;
		maxStep[1] = 200.0f;
	}

	if ( ( turret->flags & 0x200 ) && ( turret->flags & 0x100 ) && maxStep[0] < 360.0f ) {
		maxStep[0] = 360.0f;
	}

	for ( i = 0; i < 2; i++ ) {
		maxStep[i] = maxStep[i] * 0.05f;

		diff = AngleSubtract( targetAngles[i], ent->s.angles2[i] );
		if ( diff > maxStep[i] ) {
			diff = maxStep[i];
			atRest = qfalse;
		} else if ( diff < -maxStep[i] ) {
			diff = -maxStep[i];
			atRest = qfalse;
		}

		ent->s.angles2[i] = ent->s.angles2[i] + diff;
	}

	target = ent->s.angles2[0];
	ent->s.angles2[2] = ent->s.angles2[0];

	if ( turret->flags & 0x200 ) {
		if ( turret->flags & 0x400 ) {
			if ( ent->s.angles2[0] < turret->restPitchClamp ) {
				target = turret->restPitchClamp;
			} else {
				turret->flags &= ~0x100;
			}
		} else {
			if ( ent->s.angles2[0] > turret->restPitchClamp ) {
				target = turret->restPitchClamp;
			} else {
				turret->flags &= ~0x100;
			}
		}
	}

	diff = AngleSubtract( target, savePitch );
	if ( diff > maxStep[0] ) {
		diff = maxStep[0];
		atRest = qfalse;
	} else if ( diff < -maxStep[0] ) {
		diff = -maxStep[0];
		atRest = qfalse;
	}

	ent->s.angles2[0] = savePitch + diff;
	ent->s.angles2[2] = ent->s.angles2[2] - ent->s.angles2[0];

	return atRest;
}


static qboolean turret_ReturnToDefaultPos( gentity_t *ent, int useWeaponRates ) {
	float targetAngles[2];

	if ( useWeaponRates ) {
		targetAngles[0] = 0;
	} else {
		targetAngles[0] = ( (turret_t *)ent->turret )->restPitch;
	}
	targetAngles[1] = 0;

	return turret_UpdateTargetAngles( ent, targetAngles, useWeaponRates );
}


void turret_think( gentity_t *self ) {
	float targetAngles[2];

	self->nextthink = level.time + TURRET_FRAMETIME;

	if ( self->linkInfo ) {
		G_GeneralLink( self );
	}

	if ( g_entities[self->r.ownerNum].client ) {
		return;         // a gunner is driving it from ClientEndFrame instead
	}

	turret_UpdateSound( self );
	self->s.eFlags &= ~0x400;

	targetAngles[0] = ( (turret_t *)self->turret )->restPitch;
	targetAngles[1] = 0;
	turret_UpdateTargetAngles( self, targetAngles, 0 );
}


/*
==============
turret_think_init

Find how far the barrel can drop before the stock hits something, and keep
that as the rest pitch.  Sweeps down in 3 degree steps from level, tracing
tag_aim to where tag_butt would end up at each one.
==============
*/
void turret_think_init( gentity_t *self ) {
	turret_t *turret;
	matrix43_t entMat;
	const float *aimMat;
	const float *buttMat;
	vec3_t angles, forward, right, up, left;
	vec3_t delta, offset, start, end;
	trace_t trace;
	int aimBone, buttBone;
	int i;

	turret = (turret_t *)self->turret;

	self->think = turret_think;
	self->nextthink = level.time + TURRET_FRAMETIME;

	aimBone = trap_DObjGetBoneIndex( self, "tag_aim" );
	if ( aimBone < 0 ) {
		return;
	}
	G_DObjCalcBone( self, aimBone );
	aimMat = (const float *)trap_DObjGetMatrixArray( self ) + 16 * aimBone;
	if ( !aimMat ) {
		return;
	}

	buttBone = trap_DObjGetBoneIndex( self, "tag_butt" );
	if ( buttBone < 0 ) {
		return;
	}
	G_DObjCalcBone( self, buttBone );
	buttMat = (const float *)trap_DObjGetMatrixArray( self ) + 16 * buttBone;
	if ( !buttMat ) {
		return;
	}

	AnglesToAxis( self->r.currentAngles, entMat.axis );
	VectorCopy( self->r.currentOrigin, entMat.origin );

	/* the bone matrices are 4x4; the translation is the fourth row */
	VectorSubtract( buttMat + 12, aimMat + 12, delta );

	MatrixTransformVector43( &entMat, start, aimMat + 12 );

	for ( i = 0; i <= 30; i++ ) {
		angles[0] = (float)i * -3.0f;
		angles[1] = 0;
		angles[2] = 0;

		AngleVectors( angles, forward, right, up );
		VectorSubtract( vec3_origin, right, left );

		offset[0] = up[0] * delta[2] + left[0] * delta[1] + forward[0] * delta[0];
		offset[1] = up[1] * delta[2] + left[1] * delta[1] + forward[1] * delta[0];
		offset[2] = up[2] * delta[2] + left[2] * delta[1] + forward[2] * delta[0];

		VectorAdd( offset, aimMat + 12, offset );

		MatrixTransformVector43( &entMat, end, offset );

		trap_LocationalTrace( &trace, start, end, self->s.number, 0x11, bulletPriorityMap );
		if ( trace.fraction < 1.0 ) {
			turret->restPitch = angles[0];
			return;
		}
	}
}


/*
==============
turret_controller

DObj bone controller: aim the barrel bones by s.angles2.  tag_aim and
tag_aim_animated take the pitch/yaw pair; the third bone takes the pitch that
turret_UpdateTargetAngles has not applied yet.
==============
*/
void turret_controller( gentity_t *self, unsigned int *partBits ) {
	vec3_t angles;
	int boneIndex;

	angles[0] = self->s.angles2[0];
	angles[1] = self->s.angles2[1];
	angles[2] = 0;

	boneIndex = trap_DObjGetBoneIndex( self, "tag_aim" );
	if ( boneIndex >= 0 && trap_DObjSetControlRotTransIndex( self, (const int *)partBits,
															 boneIndex ) ) {
		G_DObjSetLocalTagInternal( self, vec3_origin, angles, boneIndex );
	}

	boneIndex = trap_DObjGetBoneIndex( self, "tag_aim_animated" );
	if ( boneIndex >= 0 && trap_DObjSetControlRotTransIndex( self, (const int *)partBits,
															 boneIndex ) ) {
		G_DObjSetLocalTagInternal( self, vec3_origin, angles, boneIndex );
	}

	angles[0] = self->s.angles2[2];
	angles[1] = 0;

	boneIndex = trap_DObjGetBoneIndex( self, "tag_flash" );
	if ( boneIndex >= 0 && trap_DObjSetControlRotTransIndex( self, (const int *)partBits,
															 boneIndex ) ) {
		G_DObjSetLocalTagInternal( self, vec3_origin, angles, boneIndex );
	}
}


/*
==============
turret_behind

True when the player is standing inside the half-angle the turret's yaw arc
spans, measured from the middle of that arc -- i.e. behind the gun where he
could take hold of it.
==============
*/
static qboolean turret_behind( gentity_t *player, gentity_t *turret ) {
	turret_t *t;
	float yaw;
	float halfArc;
	float angle;
	vec3_t dir;
	vec3_t delta;

	t = (turret_t *)turret->turret;

	yaw = turret->r.currentAngles[1] + t->minArc[1];
	halfArc = ( (float)fabs( t->maxArc[1] ) + (float)fabs( t->minArc[1] ) ) * 0.5f;
	yaw = AngleNormalize180( yaw + halfArc );

	dir[0] = (float)cos( yaw * 0.017453292f );
	dir[1] = (float)sin( yaw * 0.017453292f );
	dir[2] = 0;
	VectorNormalize( dir );

	delta[0] = turret->r.currentOrigin[0] - player->r.currentOrigin[0];
	delta[1] = turret->r.currentOrigin[1] - player->r.currentOrigin[1];
	delta[2] = 0;
	VectorNormalize( delta );

	angle = (float)acos( DotProduct( dir, delta ) );
	if ( angle > 3.1415927f ) {
		angle = 3.1415927f;
	} else if ( angle < -3.1415927f ) {
		angle = 3.1415927f;
	}

	angle = angle * 180.0f / 3.1415927f;

	return angle <= halfArc;
}


void G_FreeTurret( gentity_t *self ) {
	if ( g_entities[self->r.ownerNum].client ) {
		G_ClientStopUsingTurret( self );
	}

	self->active = 0;
	( (turret_t *)self->turret )->inUse = 0;
	self->turret = NULL;
}


qboolean G_IsTurretUsable( gentity_t *turret, gentity_t *player ) {
	if ( turret->active ) {
		return qfalse;
	}
	if ( !turret->turret ) {
		return qfalse;
	}
	if ( !turret->takedamage ) {
		return qfalse;
	}
	if ( !turret_behind( player, turret ) ) {
		return qfalse;
	}
	if ( player->client->ps.grenadeTimeLeft ) {
		return qfalse;
	}

	return player->client->ps.groundEntityNum != ENTITYNUM_NONE;
}


void turret_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	turret_t *turret;
	vec3_t angles;
	int i;

	turret = (turret_t *)self->turret;

	other->active = 1;
	self->active = 1;
	self->r.ownerNum = other->s.number;

	other->client->ps.viewlocked = 1;
	other->client->ps.viewlocked_entNum = self->s.number;

	turret->flags |= 0x800;
	VectorCopy( other->r.currentOrigin, turret->stopUseOrigin );

	other->s.otherEntityNum = self->s.number;

	if ( other->client->ps.pm_flags & 1 ) {
		turret->stopUseEventType = 2;
	} else {
		turret->stopUseEventType = ( other->client->ps.pm_flags & 2 ) >> 1;
	}

	if ( turret->useMode == 2 ) {
		other->client->ps.eFlags |= 0x4000;
		other->client->ps.eFlags &= ~0x8000;
	} else if ( turret->useMode == 1 ) {
		other->client->ps.eFlags |= 0x8000;
		other->client->ps.eFlags &= ~0x4000;
	} else {
		other->client->ps.eFlags |= EF_TURRET_STANCE_MASK;
	}

	VectorCopy( self->r.currentAngles, self->TargetAngles );

	for ( i = 0; i < 2; i++ ) {
		self->s.angles2[i] = AngleSubtract( other->client->ps.viewangles[i],
											self->r.currentAngles[i] );

		if ( self->s.angles2[i] > turret->maxArc[i] ) {
			self->s.angles2[i] = turret->maxArc[i];
		} else if ( self->s.angles2[i] < turret->minArc[i] ) {
			self->s.angles2[i] = turret->minArc[i];
		}
	}

	angles[2] = 0;
	angles[0] = self->r.currentAngles[0] + self->s.angles2[0];
	angles[1] = self->s.angles2[1] + self->r.currentAngles[1];
	SetClientViewAngle( other, angles );
}


void G_SpawnTurret( const char *weaponName, gentity_t *ent ) {
	turret_t *turret;
	const weaponInfo_t *weapon;
	char *arc, *damage;
	qboolean fromMap;
	int i;

	for ( i = 0; i < MAX_TURRETS; i++ ) {
		if ( !g_turrets[i].inUse ) {
			break;
		}
	}
	if ( i == MAX_TURRETS ) {
		Com_Error( ERR_DROP, "\x15" "G_SpawnTurret: max number of turrets (%d) exceeded",
				   MAX_TURRETS );
	}

	turret = &g_turrets[i];
	memset( turret, 0, sizeof( *turret ) );

	ent->turret = turret;
	turret->inUse = 1;

	ent->s.weapon = BG_GetWeaponIndexForName( weaponName );
	if ( !ent->s.weapon ) {
		Com_Error( ERR_DROP, "\x15" "bad weaponinfo '%s' specified for turret", weaponName );
	}

	weapon = bg_weaponInfo[ent->s.weapon];

	if ( !level.spawning && !itemRegistered[BG_WeaponItemIndex( ent->s.weapon )] ) {
		Scr_Error( va( "turret '%s' not precached", weaponName ) );
	}

	RegisterItem( BG_WeaponItemIndex( ent->s.weapon ), 1 );

	turret->fireTimeRemaining = 0;
	turret->useMode = weapon->stance;
	turret->stopUseEventType = -1;
	turret->fireSoundTime = 0;

	if ( weapon->loopFireSound && *weapon->loopFireSound ) {
		turret->sustainedFireLoopSound = G_SoundAliasIndex( weapon->loopFireSound );
	} else {
		turret->sustainedFireLoopSound = 0;
	}

	if ( weapon->stopFireSound && *weapon->stopFireSound ) {
		turret->sustainedFireStopSound = G_SoundAliasIndex( weapon->stopFireSound );
	} else {
		turret->sustainedFireStopSound = 0;
	}

	/* the key wins if the map set it, otherwise the weapon file's arc; the
	   two "min" arcs are negated on the way in so that both pairs read
	   min <= 0 <= max */
	fromMap = G_SpawnString( "rightarc", "", &arc );
	turret->minArc[1] = (float)atof( arc );
	if ( !fromMap ) {
		turret->minArc[1] = weapon->rightArc;
	}
	turret->minArc[1] = turret->minArc[1] * -1.0f;
	if ( turret->minArc[1] > 0 ) {
		turret->minArc[1] = 0;
	}

	fromMap = G_SpawnString( "leftarc", "", &arc );
	turret->maxArc[1] = (float)atof( arc );
	if ( !fromMap ) {
		turret->maxArc[1] = weapon->leftArc;
	}
	if ( turret->maxArc[1] < 0 ) {
		turret->maxArc[1] = 0;
	}

	fromMap = G_SpawnString( "toparc", "", &arc );
	turret->minArc[0] = (float)atof( arc );
	if ( !fromMap ) {
		turret->minArc[0] = weapon->topArc;
	}
	turret->minArc[0] = turret->minArc[0] * -1.0f;
	if ( turret->minArc[0] > 0 ) {
		turret->minArc[0] = 0;
	}

	fromMap = G_SpawnString( "bottomarc", "", &arc );
	turret->maxArc[0] = (float)atof( arc );
	if ( !fromMap ) {
		turret->maxArc[0] = weapon->bottomArc;
	}
	if ( turret->maxArc[0] < 0 ) {
		turret->maxArc[0] = 0;
	}

	turret->restPitch = -90.0f;

	if ( !ent->health ) {
		ent->health = 100;
	}

	fromMap = G_SpawnString( "damage", "0", &damage );
	ent->damage = atoi( damage );
	if ( !fromMap ) {
		ent->damage = weapon->damage;
	}
	if ( ent->damage < 0 ) {
		ent->damage = 0;
	}

	turret->flags = 3;

	ent->flags |= FL_SUPPORTS_LINKTO;
	ent->clipmask = 1;
	ent->r.contents = 0x200004;
	ent->r.svFlags = SVF_BROADCAST;
	ent->s.eType = ET_TURRET;

	G_DObjUpdate( ent );

	ent->r.mins[0] = -32;
	ent->r.mins[1] = -32;
	ent->r.mins[2] = 0;
	ent->r.maxs[0] = 32;
	ent->r.maxs[1] = 32;
	ent->r.maxs[2] = 56;

	G_SetOrigin( ent, ent->r.currentOrigin );
	G_SetAngle( ent, ent->r.currentAngles );
	VectorClear( ent->s.angles2 );

	ent->think = turret_think_init;
	ent->nextthink = level.time + FRAMETIME;
	ent->controller = turret_controller;
	ent->use = turret_use;

	ent->s.apos.trType = TR_LINEAR_STOP;
	ent->takedamage = 1;

	trap_LinkEntity( ent );
}


/*QUAKED turret (0 0 1) (-16 -16 0) (16 16 40)
"weaponinfo"	mandatory, the weapon file the mount fires
"toparc" "bottomarc" "leftarc" "rightarc"	override the weapon file's arcs
"damage"		override the weapon file's damage
*/
void SP_turret( gentity_t *ent ) {
	char *weaponName;

	if ( !G_SpawnString( "weaponinfo", "", &weaponName ) ) {
		Com_Error( ERR_DROP, "\x15" "no weaponinfo specified for turret" );
	}

	G_SpawnTurret( weaponName, ent );
}


/*
=================================================================================

misc_spawner

=================================================================================
*/

void misc_spawner_think( gentity_t *ent ) {
	if ( !Drop_Item( ent, BG_FindItem( SL_ConvertToString( ent->spawnitem ) ), 0, 0 ) ) {
		G_Printf( "-----> WARNING <-------\n" );
		G_Printf( "misc_spawner used at %s failed to drop!\n", vtos( ent->r.currentOrigin ) );
	}
}


void misc_spawner_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	self->think = misc_spawner_think;
	self->nextthink = level.time + FRAMETIME;
	trap_LinkEntity( self );
}


/*QUAKED misc_spawner (1 0 0) (-16 -16 -16) (16 16 16)
Drops its "spawnitem" every time it is used.
*/
void SP_misc_spawner( gentity_t *ent ) {
	if ( !ent->spawnitem ) {
		G_Printf( "-----> WARNING <-------\n" );
		G_Printf( "misc_spawner at loc %s has no spawnitem!\n", vtos( ent->r.currentOrigin ) );
		return;
	}

	ent->use = misc_spawner_use;
	trap_LinkEntity( ent );
}


/*
==============
miscGunnerEnemyScan

Take the first live client inside the gunner's radius as its enemy.
==============
*/
void miscGunnerEnemyScan( gentity_t *self ) {
	gentity_t *ent;
	vec3_t delta;
	float dist;
	int i;

	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];

		if ( !ent->inuse ) {
			continue;
		}
		if ( ent->health < 0 ) {
			continue;
		}

		VectorSubtract( ent->r.currentOrigin, self->r.currentOrigin, delta );
		dist = VectorLength( delta );

		if ( dist <= (float)self->radius ) {
			self->enemy = ent;
			return;
		}
	}
}
