/*
 * g_missile_mp.c -- the multiplayer game module's missile simulation.
 *
 * RTCW's game/g_missile.c with everything single-player stripped out and the
 * bounce reworked: G_MissileLandAngles is new (it rolls a grenade's angular
 * trajectory onto the surface it landed on), G_BounceMissile grew the
 * come-to-rest test that calls it, and the fire functions read their damage,
 * radius and speed out of weaponInfo_t instead of a switch on the weapon id.
 * G_MissileTrace never became a function here -- every trace is issued inline.
 *
 * Function order is binary order (0x200295D0 .. 0x2002A7A0).
 *
 * @fidelity: likely
 */

#include "g_local.h"
#include "bg_public.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Declarations that belong in a shared header. */

#define ENTITYNUM_WORLD             ( MAX_GENTITIES - 2 )

/* entityState_t.eType. */
#define ET_GENERAL                  0
#define ET_MISSILE                  4

/* trajectory_t.trType.  5 is the one fire_grenade uses; it is RTCW's
 * TR_GRAVITY_PAUSED slot, not TR_GRAVITY. */
#define TR_STATIONARY               0
#define TR_LINEAR                   2
#define TR_GRAVITY                  5

/* entityState_t.eFlags.  RTCW's EF_BOUNCE / EF_BOUNCE_HALF, moved up into the
 * top byte (G_BounceMissile 0x200298D2 / 0x200298F6). */
#define EF_BOUNCE                   0x01000000
#define EF_BOUNCE_HALF              0x02000000

/* Raised on an exploded missile so the client stops drawing the projectile
 * (G_ExplodeMissile 0x20029F4F). */
#define EF_EXPLODED                 0x00000100
/* Raised by fire_rocket only (0x2002A80D). */
#define EF_ROCKET                   0x00010000

/* entityShared_t.svFlags bits this unit sets.  0x88 is what a live missile is
 * spawned with; G_ExplodeMissile ors in 8 again on the corpse. */
#define SVF_MISSILE                 0x00000088
#define SVF_BROADCAST               0x00000008

/* gentity_t.flags: a grenade whose contents are ignored by the missile trace
 * (G_RunMissile 0x2002A26F).  The original name is not recovered; this one
 * is inferred. */
#define FL_MISSILE_PASSTHRU         0x00010000

#define CONTENTS_WATER              0x00000020
#define CONTENTS_BODY               0x02000000
#define SURF_NOIMPACT               0x00000010

/* trace_t.surfaceFlags carries the surface type in bits 20..24. */
#define SURF_TYPE_SHIFT             20
#define SURF_TYPE_MASK              0x1F
#define SURF_TYPE_WATER             0x14

#define MASK_MISSILESHOT            0x02802091      /* fire_grenade 0x2002A5F4 */
#define MASK_GRENADE_TRACE          0x00000011      /* G_ExplodeMissile 0x20029F95 */

/* Event names; each is the literal G_AddEvent is handed. */
#define EV_BULLET_HIT_SMALL         173             /* 0x2002A1D8; name from bg_misc.c's recovered eventnames[] */
#define EV_GRENADE_BOUNCE           177             /* 0x20029BC2 */
#define EV_MISSILE_MISS             178             /* 0x20029FC9 */
#define EV_MISSILE_HIT              179             /* 0x20029D26 */
#define EV_MISSILE_HIT_NOMARKS      180
#define EV_FLAMEBARREL_BOUNCE       194             /* 0x20029BA7 */

/* forward: q_shared.c / q_math.c */
float           VectorNormalize2( const vec3_t v, vec3_t out );
float           Com_RandFloatRange( float lo, float hi );   /* 0x200170E0; LTCG-inlined at 0x2002A6A4 / 0x2002A6E7 */
void            vectoangles( const vec3_t value1, vec3_t angles );
byte            DirToByte( const vec3_t dir );
float           AngleSubtract( float a1, float a2 );
float           AngleNormalize360( float angle );
float           AngleNormalize180( float angle );
float           PitchOfVectorAlongYaw( float yaw, const vec3_t normal );

/* forward: g_main_mp.c */
void            G_RunThink( gentity_t *ent );

/* forward: g_trigger_mp.c */
void            G_CheckHitTriggerDamage( gentity_t *ent, const vec3_t start, const vec3_t end,
										 int damage, int mod );
void            G_GrenadeTouchTriggerDamage( gentity_t *ent, const vec3_t start, const vec3_t end,
											 int damage, int mod );

/*
================
G_MissileLandAngles

Continues the missile's angular trajectory to the moment of impact, then --
unless forceAngles is set, in which case the missile has come to rest and only
the final orientation matters -- kicks the pitch spin for the next bounce.
================
*/
void G_MissileLandAngles( gentity_t *ent, trace_t *trace, vec3_t angles, qboolean forceAngles ) {
	int impactTime;
	float landPitch;
	float landShort;
	float pitchDelta;
	float pitchDeltaAbs;

	impactTime = level.previousTime
				 + (int)( (float)( level.time - level.previousTime ) * trace->fraction );
	BG_EvaluateTrajectory( &ent->s.apos, impactTime, angles );

	if ( trace->normal[2] > 0.1f ) {
		landPitch = PitchOfVectorAlongYaw( angles[1], trace->normal );
		pitchDelta = AngleSubtract( landPitch, angles[0] );
		pitchDeltaAbs = fabs( pitchDelta );

		if ( !forceAngles ) {
			VectorCopy( angles, ent->s.apos.trBase );
			ent->s.apos.trTime = impactTime;
			if ( pitchDeltaAbs < 80.0f ) {
				ent->s.apos.trDelta[0] *= -( rand() / 32768.0f * 0.3f + 0.85f );
			} else {
				ent->s.apos.trDelta[0] *= rand() / 32768.0f * 0.3f + 0.85f;
			}
		}

		angles[0] = AngleNormalize180( angles[0] );
		if ( forceAngles || pitchDeltaAbs < 45.0f ) {
			/* the half turn is added in short space, after the scale
			 * (0x20029764 fadd 32768 sits between the fmul and the ftol) */
			landShort = landPitch * ( 65536.0f / 360.0f );
			if ( fabs( angles[0] ) > 90.0 ) {
				landShort += 32768.0f;
			}
			angles[0] = ( 360.0f / 65536.0f ) * ( (int)landShort & 65535 );
		} else if ( pitchDeltaAbs < 80.0f ) {
			angles[0] = AngleNormalize360( angles[0] + pitchDelta * 0.25f );
		} else {
			angles[0] = AngleNormalize360( angles[0] );
		}
	} else if ( !forceAngles ) {
		ent->s.apos.trDelta[0] = AngleNormalize360( (float)( ( rand() & 0x7F ) - 0x3F )
													+ ent->s.apos.trDelta[0] );
	}
}

/*
================
G_BounceMissile

Returns qtrue when the bounce was hard enough to be worth an event.  qfalse
also means "the missile has come to rest" -- in which case its origin and
angles have already been fixed in place.
================
*/
qboolean G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t velocity;
	vec3_t delta;
	vec3_t nudge;
	vec3_t angles;
	float dot;
	int inWater;
	int impactTime;

	inWater = trap_PointContents( ent->r.currentOrigin, -1, CONTENTS_WATER );

	impactTime = level.previousTime
				 + (int)( (float)( level.time - level.previousTime ) * trace->fraction );
	BG_EvaluateTrajectoryDelta( &ent->s.pos, impactTime, velocity );

	// reflect the velocity on the trace plane
	dot = DotProduct( velocity, trace->normal );
	VectorMA( velocity, -2 * dot, trace->normal, ent->s.pos.trDelta );

	if ( trace->normal[2] > 0.7 ) {
		ent->s.groundEntityNum = trace->entityNum;
	}

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
		if ( inWater || ( trace->contents & CONTENTS_BODY ) ) {
			VectorScale( ent->s.pos.trDelta, 0.125f, ent->s.pos.trDelta );
		} else if ( ent->s.eFlags & EF_BOUNCE ) {
			// undo the reflection, damp, then put a third of it back
			VectorMA( ent->s.pos.trDelta, dot, trace->normal, ent->s.pos.trDelta );
			VectorScale( ent->s.pos.trDelta, 0.75f, ent->s.pos.trDelta );
			VectorMA( ent->s.pos.trDelta, -0.3f * dot, trace->normal, ent->s.pos.trDelta );
		} else {
			VectorScale( ent->s.pos.trDelta, 0.5f, ent->s.pos.trDelta );
		}

		// check for stop
		if ( trace->normal[2] > 0.7 && VectorLength( ent->s.pos.trDelta ) < 20 ) {
			G_SetOrigin( ent, ent->r.currentOrigin );
			G_MissileLandAngles( ent, trace, angles, qtrue );
			G_SetAngle( ent, angles );
			return qfalse;
		}
	}

	VectorScale( trace->normal, 0.1f, nudge );
	if ( nudge[2] > 0 ) {
		nudge[2] = 0;
	}
	VectorAdd( ent->r.currentOrigin, nudge, ent->r.currentOrigin );
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;

	G_MissileLandAngles( ent, trace, angles, qfalse );
	VectorCopy( angles, ent->s.apos.trBase );
	ent->s.apos.trTime = level.time;

	if ( inWater ) {
		return qfalse;
	}

	VectorSubtract( ent->s.pos.trDelta, velocity, delta );
	return VectorLength( delta ) > 100;
}

/*
================
G_MissileImpact
================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t   *other;
	gentity_t   *attacker;
	vec3_t velocity;
	qboolean hitClient;
	int event;

	other = &g_entities[trace->entityNum];
	hitClient = qfalse;

	// bouncing missiles that hit something they cannot hurt just bounce
	if ( !other->takedamage && ( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
		if ( !G_BounceMissile( ent, trace ) ) {
			return;
		}
		if ( trace->startsolid ) {
			return;
		}
		if ( ent->classname == scr_const.WP ) {
			return;
		}
		if ( ent->classname == scr_const.flamebarrel ) {
			G_AddEvent( ent, EV_FLAMEBARREL_BOUNCE, 0 );
			return;
		}
		G_AddEvent( ent, EV_GRENADE_BOUNCE,
					( trace->surfaceFlags >> SURF_TYPE_SHIFT ) & SURF_TYPE_MASK );
		return;
	}

	if ( other->takedamage ) {
		if ( !ent->damage ) {
			G_BounceMissile( ent, trace );
			return;
		}

		if ( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
			hitClient = qtrue;
		}

		BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
		if ( VectorLength( velocity ) == 0 ) {
			velocity[2] = 1;        // stepped on a grenade
		}

		if ( ent->r.ownerNum == ENTITYNUM_NONE ) {
			attacker = NULL;
		} else {
			attacker = &g_entities[ent->r.ownerNum];
		}

		G_Damage( other, ent, attacker, velocity, ent->r.currentOrigin, ent->damage,
				  0, ent->methodOfDeath, 0 );
	}

	if ( ent->damage ) {
		if ( ent->r.ownerNum == ENTITYNUM_NONE ) {
			attacker = &g_entities[ENTITYNUM_WORLD];
		} else {
			attacker = &g_entities[ent->r.ownerNum];
		}
		G_CheckHitTriggerDamage( attacker, ent->r.currentOrigin, trace->endpos,
								 ent->damage, ent->methodOfDeath );
	}

	// a hit on flesh or on a tagged part leaves no impact mark
	if ( !hitClient && trace->partName ) {
		hitClient = qtrue;
	}

	event = DirToByte( trace->normal );
	if ( hitClient ) {
		G_AddEvent( ent, EV_MISSILE_HIT_NOMARKS, event );
	} else {
		G_AddEvent( ent, EV_MISSILE_HIT, event );
	}

	ent->s.surfType = ( trace->surfaceFlags >> SURF_TYPE_SHIFT ) & SURF_TYPE_MASK;
	ent->freeAfterEvent = qtrue;
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );  // save net bandwidth

	VectorCopy( trace->endpos, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorClear( ent->s.pos.trDelta );
	VectorCopy( trace->endpos, ent->r.currentOrigin );

	if ( ent->splashDamage ) {
		G_RadiusDamage( trace->endpos, ent, ent->parent, ent->splashDamage,
						ent->splashMinDamage, ent->splashRadius, other,
						ent->splashMethodOfDeath );
	}

	trap_LinkEntity( ent );
}

/*
================
Concussive_think
================
*/
void Concussive_think( gentity_t *ent ) {
	if ( level.time > ent->delay ) {
		ent->think = G_FreeEntity;
	}
	ent->nextthink = level.time + 100;
}

/*
================
Concussive_fx

A short-lived entity parked at an explosion so the client keeps shaking after
the missile itself is gone.
================
*/
gentity_t *Concussive_fx( const vec3_t origin ) {
	gentity_t   *concussive;

	concussive = G_Spawn();
	VectorCopy( origin, concussive->r.currentOrigin );
	concussive->think = Concussive_think;
	concussive->nextthink = level.time + 100;
	concussive->delay = level.time + 500.0f;

	return concussive;
}

/*
================
G_ExplodeMissile

Explode a missile without an impact.
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t origin;
	vec3_t end;
	trace_t tr;
	int dirByte;

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	origin[0] = (int)origin[0];
	origin[1] = (int)origin[1];
	origin[2] = (int)origin[2];

	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( origin, ent->s.pos.trBase );
	ent->r.svFlags |= SVF_BROADCAST;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorCopy( origin, ent->r.currentOrigin );
	VectorClear( ent->s.pos.trDelta );
	ent->s.eType = ET_GENERAL;
	ent->s.eFlags |= EF_EXPLODED;
	ent->flags |= FL_NOCLIENT;

	if ( ent->classname == scr_const.flamebarrel ) {
		ent->freeAfterEvent = qtrue;
		trap_LinkEntity( ent );
		return;
	}

	// find the surface underneath so the explosion mark picks the right decal
	VectorCopy( origin, end );
	end[2] -= 16;
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, end, ent->s.number, MASK_GRENADE_TRACE );

	dirByte = DirToByte( tr.normal );
	G_AddEvent( ent, EV_MISSILE_MISS, dirByte );

	if ( trap_PointContents( origin, -1, CONTENTS_WATER ) ) {
		ent->s.surfType = SURF_TYPE_WATER;
	} else {
		ent->s.surfType = ( tr.surfaceFlags >> SURF_TYPE_SHIFT ) & SURF_TYPE_MASK;
	}

	ent->freeAfterEvent = qtrue;

	if ( ent->splashDamage ) {
		G_RadiusDamage( origin, ent, ent->parent, ent->splashDamage, ent->splashMinDamage,
						ent->splashRadius, ent, ent->splashMethodOfDeath );
	}

	trap_LinkEntity( ent );

	Concussive_fx( origin );
}

/*
================
G_MissileDie

Name inferred from RTCW: the body is RTCW's G_MissileDie (1.1 has no
G_MissileTrace at all -- every missile trace is issued inline).  Only
self and attacker survive in registers -- the rest of the die signature is
unused and LTCG dropped it.
================
*/
void G_MissileDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage,
				   int mod, int weapon, const float *dir, int hitLoc ) {
	if ( attacker != self ) {
		self->takedamage = qfalse;
		self->think = G_ExplodeMissile;
		self->nextthink = level.time + 10;
	}
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent ) {
	vec3_t origin;
	vec3_t end;
	vec3_t dir;
	trace_t tr;
	gentity_t   *tent;
	gentity_t   *hit;
	int contents;

	VectorCopy( ent->r.currentOrigin, origin );

	// get the current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, end );

	VectorSubtract( end, ent->r.currentOrigin, dir );
	if ( VectorNormalize( dir ) < 0.001f ) {
		G_RunThink( ent );
		return;
	}

	// a fast-falling missile also collides with the water surface, so it can
	// raise a splash before it carries on underneath
	if ( fabs( ent->s.pos.trDelta[2] ) > 30.0
		 && !trap_PointContents( ent->r.currentOrigin, -1, CONTENTS_WATER ) ) {
		trap_LocationalTrace( &tr, ent->r.currentOrigin, end, ent->r.ownerNum,
							  ent->clipmask | CONTENTS_WATER, bulletPriorityMap );
	} else {
		trap_LocationalTrace( &tr, ent->r.currentOrigin, end, ent->r.ownerNum,
							  ent->clipmask, bulletPriorityMap );
	}

	if ( ( tr.surfaceFlags & ( SURF_TYPE_MASK << SURF_TYPE_SHIFT ) )
		 == ( SURF_TYPE_WATER << SURF_TYPE_SHIFT ) ) {
		VectorNormalize2( ent->s.pos.trDelta, dir );
		if ( dir[2] < 0 ) {
			dir[2] *= -1.0f;
		}

		tent = G_TempEntity( ent->r.currentOrigin, EV_BULLET_HIT_SMALL );
		tent->s.eventParm = DirToByte( tr.normal );
		tent->s.scale = DirToByte( dir );
		tent->s.surfType = ( tr.surfaceFlags >> SURF_TYPE_SHIFT ) & SURF_TYPE_MASK;
		tent->s.otherEntityNum = ent->s.number;

		// and now for real
		trap_LocationalTrace( &tr, ent->r.currentOrigin, end, ent->r.ownerNum,
							  ent->clipmask, bulletPriorityMap );
	}

	// a grenade is allowed through anything flagged for it
	if ( ent->methodOfDeath == MOD_GRENADE ) {
		hit = &g_entities[tr.entityNum];
		if ( hit->flags & FL_MISSILE_PASSTHRU ) {
			contents = hit->r.contents;
			hit->r.contents = 0;
			trap_LocationalTrace( &tr, ent->r.currentOrigin, end, ent->r.ownerNum,
								  ent->clipmask, bulletPriorityMap );
			hit->r.contents = contents;
		}
	}

	VectorCopy( tr.endpos, ent->r.currentOrigin );
	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	// a resting bouncer is dropped onto whatever is under it
	if ( ( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) )
		 && ( tr.fraction == 1.0f || ( tr.fraction < 1.0f && tr.normal[2] > 0.7f ) ) ) {
		VectorCopy( ent->r.currentOrigin, end );
		end[2] -= 1.5f;
		trap_LocationalTrace( &tr, ent->r.currentOrigin, end, ent->r.ownerNum,
							  ent->clipmask, bulletPriorityMap );
		if ( tr.fraction != 1.0f ) {
			ent->s.pos.trBase[2] += ( tr.endpos[2] + 1.5f ) - ent->r.currentOrigin[2];
			ent->r.currentOrigin[0] = tr.endpos[0];
			ent->r.currentOrigin[1] = tr.endpos[1];
			ent->r.currentOrigin[2] = tr.endpos[2] + 1.5f;
		}
	}

	trap_LinkEntity( ent );

	if ( ent->methodOfDeath == MOD_GRENADE ) {
		G_GrenadeTouchTriggerDamage( ent, origin, ent->r.currentOrigin, ent->splashDamage,
									 MOD_GRENADE );
	}

	if ( tr.fraction == 1.0f ) {
		if ( VectorLength( ent->s.pos.trDelta ) != 0 ) {
			ent->s.groundEntityNum = ENTITYNUM_NONE;
		}
		G_RunThink( ent );
		return;
	}

	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		G_FreeEntity( ent );
		return;
	}

	G_MissileImpact( ent, &tr );
	if ( ent->s.eType != ET_MISSILE ) {
		// the missile exploded and turned into something else
		return;
	}

	G_RunThink( ent );
}

/*
=================
fire_grenade

`self` and `dir` are the two arguments LTCG register-allocated.
=================
*/
gentity_t *fire_grenade( gentity_t *self, const vec3_t start, const vec3_t dir, int weapon ) {
	gentity_t   *grenade;
	const weaponInfo_t  *weapInfo;

	grenade = G_Spawn();

	if ( self->client && self->client->ps.grenadeTimeLeft ) {
		grenade->nextthink = level.time + self->client->ps.grenadeTimeLeft;
	} else {
		grenade->nextthink = level.time + 2500;
	}

	if ( self->client ) {
		self->client->ps.grenadeTimeLeft = 0;       // reset the grenade timer
	}

	grenade->s.weapon = weapon;
	grenade->think = G_ExplodeMissile;
	grenade->s.eType = ET_MISSILE;
	grenade->r.svFlags = SVF_MISSILE;
	grenade->r.ownerNum = self->s.number;
	grenade->parent = self;

	weapInfo = bg_weaponInfo[weapon];
	Scr_SetString( &grenade->classname, scr_const.grenade );

	grenade->damage = weapInfo->damage;
	grenade->splashDamage = weapInfo->explosionInnerDamage;
	grenade->splashMinDamage = weapInfo->explosionOuterDamage;
	grenade->splashRadius = weapInfo->explosionRadius;
	grenade->methodOfDeath = MOD_GRENADE;
	grenade->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	grenade->s.eFlags = EF_BOUNCE | EF_BOUNCE_HALF;
	grenade->clipmask = MASK_MISSILESHOT;

	grenade->s.pos.trType = TR_GRAVITY;
	grenade->s.pos.trTime = level.time;
	VectorCopy( start, grenade->s.pos.trBase );
	VectorCopy( dir, grenade->s.pos.trDelta );
	grenade->s.pos.trDelta[0] = (int)grenade->s.pos.trDelta[0];
	grenade->s.pos.trDelta[1] = (int)grenade->s.pos.trDelta[1];
	grenade->s.pos.trDelta[2] = (int)grenade->s.pos.trDelta[2];

	grenade->s.apos.trType = TR_LINEAR;
	grenade->s.apos.trTime = level.time;
	vectoangles( dir, grenade->s.apos.trBase );
	grenade->s.apos.trBase[0] = AngleNormalize360( grenade->s.apos.trBase[0] - 120 );
	grenade->s.apos.trDelta[0] = Com_RandFloatRange( -45, 45 ) + 720;
	grenade->s.apos.trDelta[1] = 0;
	grenade->s.apos.trDelta[2] = Com_RandFloatRange( -45, 45 ) + 360;

	VectorCopy( start, grenade->r.currentOrigin );
	VectorCopy( grenade->s.apos.trBase, grenade->r.currentAngles );

	return grenade;
}

/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket( gentity_t *self, const vec3_t start, vec3_t dir ) {
	gentity_t   *bolt;
	const weaponInfo_t  *weapInfo;
	vec3_t angles;

	VectorNormalize( dir );

	weapInfo = bg_weaponInfo[self->s.weapon];

	bolt = G_Spawn();
	Scr_SetString( &bolt->classname, scr_const.rocket );
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_MISSILE;
	bolt->s.eFlags |= EF_ROCKET;
	bolt->nextthink = level.time + 30000;
	bolt->s.weapon = self->s.weapon;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = weapInfo->damage;
	bolt->splashDamage = weapInfo->explosionInnerDamage;
	bolt->splashMinDamage = weapInfo->explosionOuterDamage;
	bolt->splashRadius = weapInfo->explosionRadius;
	bolt->methodOfDeath = MOD_PROJECTILE;
	bolt->splashMethodOfDeath = MOD_PROJECTILE_SPLASH;
	bolt->clipmask = MASK_MISSILESHOT;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - 50;       // move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, weapInfo->projectileSpeed, bolt->s.pos.trDelta );
	bolt->s.pos.trDelta[0] = (int)bolt->s.pos.trDelta[0];
	bolt->s.pos.trDelta[1] = (int)bolt->s.pos.trDelta[1];
	bolt->s.pos.trDelta[2] = (int)bolt->s.pos.trDelta[2];

	VectorCopy( start, bolt->r.currentOrigin );

	/* `angles` is coalesced onto r.currentAngles, so the final copy below is a
	   self-assignment in the binary. */
	vectoangles( bolt->s.pos.trDelta, angles );
	VectorCopy( angles, bolt->s.apos.trBase );
	bolt->s.apos.trType = TR_STATIONARY;
	bolt->s.apos.trTime = 0;
	bolt->s.apos.trDuration = 0;
	VectorClear( bolt->s.apos.trDelta );
	VectorCopy( angles, bolt->r.currentAngles );

	return bolt;
}
