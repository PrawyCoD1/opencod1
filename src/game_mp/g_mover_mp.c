/*
 * g_mover_mp.c -- the multiplayer game module's movers.
 *
 * RTCW's game/g_mover.c is the direct ancestor and most of this unit is that
 * text: the push solver (G_TestEntityPosition .. G_MoverTeam), the binary
 * mover state machine (SetMoverState .. Use_BinaryMover), the door triggers,
 * and the func_* spawn functions.  CoD 1.1 cut everything RTCW's single player
 * needed and nothing else does -- the AI hooks, the AAS blocking, func_secret's
 * trinary mover, plats, buttons, trains and bats -- and replaced the hardcoded
 * door sound table with the spawn-key driven one DoorSetSounds parses.  The
 * FL_KICKACTIVATE half of RTCW's kicked/soft door pair is gone as well; only
 * FL_SOFTACTIVATE survives.
 *
 * SetMoverState / MatchTeam are shared with the script mover in
 * g_scr_mover_mp.c, which is why they are not static here.
 *
 * Function order is binary order (0x2002A980 .. 0x2002E2C0).
 *
 * @fidelity: likely
 */

#include "g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Declarations that belong in a shared header. */

#define PITCH   0
#define YAW     1
#define ROLL    2

#define ANGLE2SHORT( x )    ( (int)( ( x ) * ( 65536 / 360.0f ) ) & 65535 )

/* G_RunFrameForEntity's think period; finishSpawningKeyedMover and the door
   spawns all schedule level.time + 100. */
#define FRAMETIME               100

/* gentity_t.flags.  0x100 is the only one of RTCW's door activation flags CoD
   kept (SetMoverState 0x2002B90A, Use_BinaryMover 0x2002C468). */
#define FL_TEAMSLAVE            0x00000004
#define FL_TOGGLE               0x00000080
#define FL_SOFTACTIVATE         0x00000100

/* entityState_t.eType, off the G_MoverPush filter (0x2002B3B0) and InitMover
   (0x2002C7EA). */
#define ET_PLAYER               1
#define ET_ITEM                 3
#define ET_MISSILE              4
#define ET_MOVER                5

/* trajectory_t.trType */
#define TR_STATIONARY           0
#define TR_LINEAR               2
#define TR_LINEAR_STOP          3
#define TR_SINE                 4

/* entityShared_t.svFlags */
#define SVF_NOCLIENT            0x00000001
#define SVF_USE_CURRENT_ORIGIN  0x00000080

/* entityState_t.eFlags */
#define EF_MOVER_STOP           0x04000000

/* Content bits.  MASK_SOLID is the 0x11 G_TestEntityPosition falls back to
   when the entity has no clipmask (0x2002A99F); its two bits are not named. */
#define MASK_SOLID              0x00000011
#define CONTENTS_CORPSE         0x04000000
#define CONTENTS_TRIGGER        0x40000000

/* entity_event_t members this unit raises (bg_misc.c's eventnames[]). */
#define EV_SOUND_ALIAS          172
#define EV_ITEM_POP             198

/* meansOfDeath, emitted as a literal by every G_Damage call here. */
#define MOD_CRUSH               19

/* trigger_use's "hintstring" pool: 32 configstrings ending where
   CS_LOCALIZED_STRINGS (1244) begins.  0x2002E20A / 0x2002E266. */
#define CS_HINTSTRINGS          1212
#define MAX_HINTSTRINGS         32

/* entityState_t.dmgFlags values, named by predef_hintStrings[] below. */
#define HINT_ACTIVATE           2
#define HINT_NUM_HINTS          11

#define FOFS( x )   ( (int)&( ( (gentity_t *)0 )->x ) )

/* forward: q_math.c */
float   RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void    AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );
void    VectorInverse( vec3_t v );
void    vectoangles( const vec3_t value1, vec3_t angles );

/* forward: g_main_mp.c */
extern vmCvar_t g_gravity;                      /* 0x2016EFE0 */

/* forward: g_combat_mp.c.  Every call site in this unit passes dir/point
 * NULL, dflags 0, mod MOD_CRUSH, hitLoc 0. */
void    G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
				  const vec3_t dir, const vec3_t point, int damage, int dflags,
				  int mod, int hitLoc );

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

typedef struct {
	gentity_t   *ent;
	vec3_t origin;
	vec3_t angles;          /* never written or read in 1.1; only the 32-byte
							   stride is known */
	float deltayaw;         /* CoD stores amove[YAW], not the client's
							   delta_angles[YAW] (0x2002B4F6) */
} pushed_t;
pushed_t pushed[MAX_GENTITIES], *pushed_p;      /* 0x202C5340, 0x20089F30 */


/*
============
G_TestEntityPosition

============
*/
gentity_t *G_TestEntityPosition( gentity_t *ent, const vec3_t origin ) {
	trace_t tr;
	int mask;

	if ( ent->clipmask ) {
		// corpses aren't important
		if ( ent->r.contents == CONTENTS_CORPSE ) {
			return NULL;
		}
		mask = ent->clipmask;
	} else {
		mask = MASK_SOLID;
	}

	if ( ent->s.eType == ET_MISSILE ) {
		trap_Trace( &tr, origin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, mask );
	} else {
		trap_Trace( &tr, origin, ent->r.mins, ent->r.maxs, origin, ent->s.number, mask );
	}

	if ( tr.startsolid ) {
		return &g_entities[ tr.entityNum ];
	}

	return NULL;
}

/*
================
G_CreateRotationMatrix
================
*/
void G_CreateRotationMatrix( const vec3_t angles, vec3_t matrix[3] ) {
	AngleVectors( angles, matrix[0], matrix[1], matrix[2] );
	VectorInverse( matrix[1] );
}

/*
================
G_TransposeMatrix
================
*/
void G_TransposeMatrix( const vec3_t matrix[3], vec3_t transpose[3] ) {
	int i, j;

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
================
G_RotatePoint
================
*/
void G_RotatePoint( vec3_t point, const vec3_t matrix[3] ) {
	vec3_t tvec;

	VectorCopy( point, tvec );
	point[0] = DotProduct( matrix[0], tvec );
	point[1] = DotProduct( matrix[1], tvec );
	point[2] = DotProduct( matrix[2], tvec );
}

/*
==================
G_TryPushingEntity

Returns qfalse if the move is blocked
==================
*/
qboolean G_TryPushingEntity( gentity_t *check, gentity_t *pusher, vec3_t move, vec3_t amove ) {
	vec3_t org, org2, move2;
	vec3_t matrix[3], transpose[3];
	vec3_t pos;
	float x, fx, y, fy, z, fz;
#define JITTER_INC  4
#define JITTER_MAX  ( check->r.maxs[0] / 2.0 )

	// EF_MOVER_STOP will just stop when contacting another entity
	// instead of pushing it, but entities can still ride on top of it
	if ( ( pusher->s.eFlags & EF_MOVER_STOP ) &&
		 check->s.groundEntityNum != pusher->s.number ) {
		return qfalse;
	}

	// try moving the contacted entity
	VectorAdd( check->r.currentOrigin, move, pos );

	// figure movement due to the pusher's amove
	G_CreateRotationMatrix( amove, transpose );
	G_TransposeMatrix( (const vec3_t *)transpose, matrix );
	VectorSubtract( pos, pusher->r.currentOrigin, org );
	VectorCopy( org, org2 );
	G_RotatePoint( org2, (const vec3_t *)matrix );
	VectorSubtract( org2, org, move2 );
	VectorAdd( pos, move2, pos );

	if ( !G_TestEntityPosition( check, pos ) ) {
		// pushed ok
		// may have pushed them off an edge
		if ( check->s.groundEntityNum != pusher->s.number ) {
			check->s.groundEntityNum = ENTITYNUM_NONE;
		}
		VectorCopy( pos, check->r.currentOrigin );
		VectorCopy( pos, check->s.pos.trBase );
		if ( check->client ) {
			// make sure the client's view rotates when on a rotating mover
			check->client->ps.delta_angles[YAW] += ANGLE2SHORT( amove[YAW] );
			VectorCopy( pos, check->client->ps.origin );
		}
		pushed_p++;
		return qtrue;
	}

	// RF, if still not valid, move them around to see if we can find a good spot
	if ( JITTER_MAX > JITTER_INC ) {
		VectorCopy( pos, org );
		for ( z = 0; z < JITTER_MAX; z += JITTER_INC )
			for ( fz = -z; fz <= z; fz += 2 * z ) {
				for ( x = JITTER_INC; x < JITTER_MAX; x += JITTER_INC )
					for ( fx = -x; fx <= x; fx += 2 * x ) {
						for ( y = JITTER_INC; y < JITTER_MAX; y += JITTER_INC )
							for ( fy = -y; fy <= y; fy += 2 * y ) {
								VectorSet( move2, fx, fy, fz );
								VectorAdd( org, move2, org2 );

								//
								// do the test
								if ( !G_TestEntityPosition( check, org2 ) ) {
									// pushed ok
									if ( check->s.groundEntityNum != pusher->s.number ) {
										check->s.groundEntityNum = ENTITYNUM_NONE;
									}
									VectorCopy( org2, check->r.currentOrigin );
									VectorCopy( org2, check->s.pos.trBase );
									if ( check->client ) {
										check->client->ps.delta_angles[YAW] += ANGLE2SHORT( amove[YAW] );
										VectorCopy( org2, check->client->ps.origin );
									}
									pushed_p++;
									return qtrue;
								}
							}
					}
				if ( !fz ) {
					break;
				}
			}
	}

	// if it is ok to leave in the old position, do it
	// this is only relevent for riding entities, not pushed
	// Sliding trapdoors can cause this.
	if ( G_TestEntityPosition( check, check->r.currentOrigin ) ) {
		// blocked
		return qfalse;
	}

	check->s.groundEntityNum = ENTITYNUM_NONE;
	return qtrue;
}

/*
============
G_MoverPush

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
If qfalse is returned, *obstacle will be the blocking entity
============
*/
qboolean G_MoverPush( gentity_t *pusher, vec3_t move, vec3_t amove, gentity_t **obstacle ) {
	int i, e;
	gentity_t   *check;
	vec3_t mins, maxs;
	int entityList[MAX_GENTITIES];
	int moveList[MAX_GENTITIES];
	int listedEntities, moveEntities;
	vec3_t totalMins, totalMaxs;

	*obstacle = NULL;

	// mins/maxs are the bounds at the destination
	// totalMins / totalMaxs are the bounds for the entire move
	if ( pusher->r.currentAngles[0] || pusher->r.currentAngles[1] || pusher->r.currentAngles[2]
		 || amove[0] || amove[1] || amove[2] ) {
		float radius;

		radius = RadiusFromBounds( pusher->r.mins, pusher->r.maxs );
		for ( i = 0; i < 3; i++ ) {
			mins[i] = pusher->r.currentOrigin[i] - radius + move[i];
			maxs[i] = pusher->r.currentOrigin[i] + radius + move[i];
			totalMins[i] = pusher->r.currentOrigin[i] - radius;
			totalMaxs[i] = pusher->r.currentOrigin[i] + radius;
		}
	} else {
		for ( i = 0 ; i < 3 ; i++ ) {
			mins[i] = pusher->r.absmin[i] + move[i];
			maxs[i] = pusher->r.absmax[i] + move[i];
		}

		VectorCopy( pusher->r.absmin, totalMins );
		VectorCopy( pusher->r.absmax, totalMaxs );
	}
	for ( i = 0; i < 3; i++ ) {
		if ( move[i] > 0 ) {
			totalMaxs[i] += move[i];
		} else {
			totalMins[i] += move[i];
		}
	}

	// unlink the pusher so we don't get it in the entityList
	trap_UnlinkEntity( pusher );

	/* 0x02000180 is CONTENTS_BODY plus two bits with no recovered name
	   (0x2002B2FA). */
	listedEntities = trap_EntitiesInBox( totalMins, totalMaxs, entityList, MAX_GENTITIES, 0x02000180 );

	// move the pusher to it's final position
	VectorAdd( pusher->r.currentOrigin, move, pusher->r.currentOrigin );
	VectorAdd( pusher->r.currentAngles, amove, pusher->r.currentAngles );
	trap_LinkEntity( pusher );

	moveEntities = 0;
	// see if any solid entities are inside the final position
	for ( e = 0 ; e < listedEntities ; e++ ) {
		check = &g_entities[ entityList[ e ] ];

		// only push items and players
		if ( check->s.eType != ET_MISSILE && check->s.eType != ET_ITEM
			 && check->s.eType != ET_PLAYER && !check->physicsObject ) {
			continue;
		}

		// if the entity is standing on the pusher, it will definitely be moved
		if ( check->s.groundEntityNum != pusher->s.number ) {
			// see if the ent needs to be tested
			if ( check->r.absmin[0] >= maxs[0]
				 || check->r.absmin[1] >= maxs[1]
				 || check->r.absmin[2] >= maxs[2]
				 || check->r.absmax[0] <= mins[0]
				 || check->r.absmax[1] <= mins[1]
				 || check->r.absmax[2] <= mins[2] ) {
				continue;
			}
			// see if the ent's bbox is inside the pusher's final position
			// this does allow a fast moving object to pass through a thin entity...
			if ( G_TestEntityPosition( check, check->r.currentOrigin ) != pusher ) {
				continue;
			}
		}

		moveList[moveEntities++] = entityList[e];
	}

	// unlink all to be moved entities so they cannot get stuck in each other
	for ( e = 0; e < moveEntities; e++ ) {
		check = &g_entities[ moveList[e] ];

		trap_UnlinkEntity( check );
	}

	for ( e = 0; e < moveEntities; e++ ) {
		check = &g_entities[ moveList[e] ];

		// save off the old position
		pushed_p->ent = check;
		VectorCopy( check->r.currentOrigin, pushed_p->origin );
		pushed_p->deltayaw = amove[YAW];

		// the entity needs to be pushed
		if ( G_TryPushingEntity( check, pusher, move, amove ) ) {
			// link it in now so nothing else tries to clip into us
			trap_LinkEntity( check );
			continue;
		}

		// the move was blocked by an entity

		// items never block
		if ( check->s.eType == ET_ITEM ) {
			trap_LinkEntity( check );
			continue;
		}

		// bobbing entities are instant-kill and never get blocked
		if ( pusher->s.pos.trType == TR_SINE || pusher->s.apos.trType == TR_SINE ) {
			G_Damage( check, pusher, pusher, NULL, NULL, 99999, 0, MOD_CRUSH, 0 );
			continue;
		}

		// save off the obstacle so we can call the block function (crush, etc)
		*obstacle = check;

		// movement failed
		return qfalse;
	}

	// link all entities at their final position
	for ( e = 0; e < moveEntities; e++ ) {
		check = &g_entities[ moveList[e] ];

		trap_LinkEntity( check );
	}
	// movement was successfull
	return qtrue;
}

/*
=================
G_MoverTeam
=================
*/
void G_MoverTeam( gentity_t *ent ) {
	vec3_t move, amove;
	gentity_t   *part, *obstacle;
	pushed_t    *p;
	vec3_t origin, angles;

	obstacle = NULL;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;
	for ( part = ent ; part ; part = part->teamchain ) {
		// get current position
		BG_EvaluateTrajectory( &part->s.pos, level.time, origin );
		BG_EvaluateTrajectory( &part->s.apos, level.time, angles );
		VectorSubtract( origin, part->r.currentOrigin, move );
		VectorSubtract( angles, part->r.currentAngles, amove );

		if ( !G_MoverPush( part, move, amove, &obstacle ) ) {
			break;  // move was blocked
		}
	}

	if ( part ) {
		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for ( p = pushed_p - 1 ; p >= pushed ; p-- ) {
			VectorCopy( p->origin, p->ent->r.currentOrigin );
			VectorCopy( p->origin, p->ent->s.pos.trBase );
			if ( p->ent->client ) {
				p->ent->client->ps.delta_angles[YAW] -= ANGLE2SHORT( p->deltayaw );
				VectorCopy( p->origin, p->ent->client->ps.origin );
			}
			trap_LinkEntity( p->ent );
		}

		// go back to the previous position
		for ( part = ent ; part ; part = part->teamchain ) {
			part->s.pos.trTime += level.time - level.previousTime;
			part->s.apos.trTime += level.time - level.previousTime;
			BG_EvaluateTrajectory( &part->s.pos, level.time, part->r.currentOrigin );
			BG_EvaluateTrajectory( &part->s.apos, level.time, part->r.currentAngles );
			trap_LinkEntity( part );
		}

		// if the pusher has a "blocked" function, call it
		if ( ent->blocked ) {
			ent->blocked( ent, obstacle );
		}
		return;
	}

	// the move succeeded
	for ( part = ent ; part ; part = part->teamchain ) {
		// call the reached function if time is at or past end point

		// opening/closing sliding door
		if ( part->s.pos.trType != TR_STATIONARY ) {
			if ( level.time >= part->s.pos.trTime + part->s.pos.trDuration ) {
				if ( part->reached ) {
					part->reached( part );
				}
			}
		}
		// opening or closing rotating door
		if ( part->s.apos.trType != TR_STATIONARY ) {
			if ( level.time >= part->s.apos.trTime + part->s.apos.trDuration ) {
				if ( part->reached ) {
					part->reached( part );
				}
			}
		}
	}
}

/*
================
G_RunMover

================
*/
void G_RunMover( gentity_t *ent ) {
	if ( ent->linkInfo ) {
		G_GeneralLink( ent );
	} else if ( ent->flags & FL_TEAMSLAVE ) {
		// if not a team captain, don't do anything, because
		// the captain will handle everything
		// FIXME
		// hack to fix problem of tram car slaves being linked
		// after being unlinked in G_FindTeams
		if ( ent->r.linked ) {
			if ( ent->classname == scr_const.func_tramcar || ent->classname == scr_const.func_rotating ) {
				trap_UnlinkEntity( ent );
			}
		}
		return;
	}
	// if stationary at one of the positions, don't move anything
	else if ( ent->s.pos.trType != TR_STATIONARY || ent->s.apos.trType != TR_STATIONARY ) {
		G_MoverTeam( ent );
	}

	// check think function
	G_RunThink( ent );
}

/*
============================================================================

GENERAL MOVERS

Doors, plats, and buttons are all binary (two position) movers
Pos1 is "at rest", pos2 is "activated"
============================================================================
*/

/*
===============
SetMoverState
===============
*/
void SetMoverState( gentity_t *ent, moverState_t moverState, int time ) {
	vec3_t delta;
	float f;
	qboolean soft;

	soft = ( ent->flags & FL_SOFTACTIVATE );

	ent->moverState     = (byte)moverState;
	ent->s.pos.trTime   = time;
	ent->s.apos.trTime  = time;
	switch ( moverState ) {
	case MOVER_POS1:
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
		ent->active = qfalse;
		break;
	case MOVER_POS2:
		VectorCopy( ent->pos2, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
		break;

	case MOVER_POS3:
		VectorCopy( ent->pos3, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
		break;

	case MOVER_2TO3:
		VectorCopy( ent->pos2, ent->s.pos.trBase );
		VectorSubtract( ent->pos3, ent->pos2, delta );
		f = 1000.0f / ent->s.pos.trDuration;
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		break;
	case MOVER_3TO2:
		VectorCopy( ent->pos3, ent->s.pos.trBase );
		VectorSubtract( ent->pos2, ent->pos3, delta );
		f = 1000.0f / ent->s.pos.trDuration;
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		break;

	case MOVER_1TO2:        // opening
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		VectorSubtract( ent->pos2, ent->pos1, delta );
		ent->s.pos.trDuration = ent->gDuration;
		f = 1000.0f / ent->s.pos.trDuration;
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		break;
	case MOVER_2TO1:        // closing
		VectorCopy( ent->pos2, ent->s.pos.trBase );
		VectorSubtract( ent->pos1, ent->pos2, delta );
		if ( ent->closespeed ) {                        // handle doors with different close speeds
			ent->s.pos.trDuration = ent->gDurationBack;
			f = 1000.0f / ent->gDurationBack;
		} else {
			ent->s.pos.trDuration = ent->gDuration;
			f = 1000.0f / ent->s.pos.trDuration;
		}
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		break;

	case MOVER_POS1ROTATE:      // at close
		VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );
		ent->s.apos.trType = TR_STATIONARY;
		break;
	case MOVER_POS2ROTATE:      // at open
		VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );
		ent->s.apos.trType = TR_STATIONARY;
		break;
	case MOVER_1TO2ROTATE:      // opening
		VectorClear( ent->s.apos.trBase );              // set base to start position {0,0,0}

		if ( soft ) {
			f = 500.0f / ent->gDuration;         // 1/2 speed when soft opened
			ent->s.apos.trDuration = ent->gDuration * 2;
		} else {
			f = 1000.0f / ent->gDuration;
			ent->s.apos.trDuration = ent->gDuration;
		}
		VectorScale( ent->rotate, f * ent->angle, ent->s.apos.trDelta );
		ent->s.apos.trType = TR_LINEAR_STOP;
		break;
	case MOVER_2TO1ROTATE:      // closing
		VectorScale( ent->rotate, ent->angle, ent->s.apos.trBase );      // set base to end position
		// (soft closes at 1/2 speed)
		f = 1000.0f / ent->gDuration;
		ent->s.apos.trDuration = ent->gDuration;
		if ( soft ) {
			ent->s.apos.trDuration *= 2;
			f *= 0.5f;
		}
		VectorScale( ent->s.apos.trBase, -f, ent->s.apos.trDelta );
		ent->s.apos.trType = TR_LINEAR_STOP;
		ent->active = qfalse;
		break;
	}

	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

	// RF, added this for bats, but this is safe for all movers, since if they
	// aren't solid, and aren't visible to the client, they don't need to be linked
	if ( !( ent->r.svFlags & SVF_NOCLIENT ) || ( ent->r.contents ) ) {
		trap_LinkEntity( ent );
	}
}

/*
================
MatchTeam

All entities in a mover team will move from pos1 to pos2
in the same amount of time
================
*/
void MatchTeam( gentity_t *teamLeader, int moverState, int time ) {
	gentity_t       *slave;

	for ( slave = teamLeader ; slave ; slave = slave->teamchain ) {

		// pass along flags for how door was activated
		if ( teamLeader->flags & FL_SOFTACTIVATE ) {
			slave->flags |= FL_SOFTACTIVATE;
		}

		SetMoverState( slave, moverState, time );
	}
}

/*
MatchTeamReverseAngleOnSlaves

the activator was blocking the door so reverse its direction
*/
void MatchTeamReverseAngleOnSlaves( gentity_t *teamLeader, int moverState, int time ) {
	gentity_t       *slave;

	for ( slave = teamLeader ; slave ; slave = slave->teamchain ) {
		// reverse open dir for teamLeader and all slaves
		slave->angle *= -1;

		// pass along flags for how door was activated
		if ( teamLeader->flags & FL_SOFTACTIVATE ) {
			slave->flags |= FL_SOFTACTIVATE;
		}

		SetMoverState( slave, moverState, time );
	}
}

/*
================
ReturnToPos1
================
*/
void ReturnToPos1( gentity_t *ent ) {
	MatchTeam( ent, MOVER_2TO1, level.time );

	// play starting sound
	G_PlaySoundAlias( ent, ent->soundClosing );

	// set looping sound
	ent->s.loopSound = ent->soundCloseLoop;
}

/*
================
ReturnToPos2
================
*/
void ReturnToPos2( gentity_t *ent ) {
	MatchTeam( ent, MOVER_3TO2, level.time );

	// looping sound
	ent->s.loopSound = ent->soundLoop;

	// starting sound
	G_PlaySoundAlias( ent, ent->soundCloseLoop );
}

/*
================
GotoPos3
================
*/
void GotoPos3( gentity_t *ent ) {
	MatchTeam( ent, MOVER_2TO3, level.time );

	// looping sound
	ent->s.loopSound = ent->soundLoop;

	// starting sound
	G_PlaySoundAlias( ent, ent->soundOpenLoop );
}

/*
================
ReturnToPos1Rotate
	closing
================
*/
void ReturnToPos1Rotate( gentity_t *ent ) {
	qboolean inPVS = qfalse;
	gentity_t   *player;

	MatchTeam( ent, MOVER_2TO1ROTATE, level.time );

	player = G_Find( NULL, FOFS( classname ), scr_const.player );

	if ( player ) {
		inPVS = trap_InPVS( player->r.currentOrigin, ent->r.currentOrigin );
	}

	// play starting sound
	if ( inPVS ) {
		if ( ent->flags & FL_SOFTACTIVATE ) {
			G_PlaySoundAlias( ent, ent->soundClosingQuiet );
		} else {
			G_PlaySoundAlias( ent, ent->soundClosing );
		}
	}

	ent->s.loopSound = ent->soundCloseLoop;
}

/*
================
Reached_BinaryMover
================
*/
void Reached_BinaryMover( gentity_t *ent ) {
	qboolean soft;

	soft = ( ent->flags & FL_SOFTACTIVATE );

	// stop the looping sound
	ent->s.loopSound = 0;

	if ( ent->moverState == MOVER_1TO2 ) {
		// reached pos2
		SetMoverState( ent, MOVER_POS2, level.time );

		// play sound
		if ( soft ) {
			G_PlaySoundAlias( ent, ent->soundOpenQuietEnd );
		} else {
			G_PlaySoundAlias( ent, ent->soundOpenEnd );
		}

		if ( !ent->activator ) {
			ent->activator = ent;
		}

		if ( ent->flags & FL_TOGGLE ) {
			ent->active = qfalse;
			ent->think = ReturnToPos1;
			ent->nextthink = 0;
			return;
		}

		// return to pos1 after a delay
		if ( ent->wait != -1000 ) {
			ent->think = ReturnToPos1;
			ent->nextthink = level.time + (int)ent->wait;
		}
	} else if ( ent->moverState == MOVER_2TO1 ) {
		// reached pos1
		SetMoverState( ent, MOVER_POS1, level.time );

		// play sound
		if ( soft ) {
			G_PlaySoundAlias( ent, ent->soundCloseQuietEnd );
		} else {
			G_PlaySoundAlias( ent, ent->soundCloseEnd );
		}

		// close areaportals
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qfalse );
		}
	} else if ( ent->moverState == MOVER_1TO2ROTATE ) {
		// reached pos2
		SetMoverState( ent, MOVER_POS2ROTATE, level.time );

		// play sound
		if ( soft ) {
			G_PlaySoundAlias( ent, ent->soundOpenQuietEnd );
		} else {
			G_PlaySoundAlias( ent, ent->soundOpenEnd );
		}

		if ( !ent->activator ) {
			ent->activator = ent;
		}

		if ( ent->flags & FL_TOGGLE ) {
			ent->active = qfalse;
			ent->nextthink = 0;
			ent->think = ReturnToPos1Rotate;
			return;
		}

		// return to pos1 after a delay
		if ( ent->wait != -1000 ) {
			ent->think = ReturnToPos1Rotate;
			ent->nextthink = level.time + (int)ent->wait;
		}
	} else if ( ent->moverState == MOVER_2TO1ROTATE ) {
		// reached pos1
		SetMoverState( ent, MOVER_POS1ROTATE, level.time );

		// to stop sound from being requested if not in pvs anoying bug
		{
			qboolean inPVS = qfalse;
			gentity_t *player;

			player = G_Find( NULL, FOFS( classname ), scr_const.player );

			if ( player ) {
				inPVS = trap_InPVS( player->r.currentOrigin, ent->r.currentOrigin );
			}

			// play sound
			if ( inPVS ) {
				if ( soft ) {
					G_PlaySoundAlias( ent, ent->soundCloseQuietEnd );
				} else {
					G_PlaySoundAlias( ent, ent->soundCloseEnd );
				}
			}
		}

		// clear the 'soft' flag
		ent->flags &= ~FL_SOFTACTIVATE;

		// close areaportals
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qfalse );
		}
	} else {
		G_Error( "Reached_BinaryMover: bad moverState" );
	}
}

/*
================
IsBinaryMoverBlocked
================
*/
qboolean IsBinaryMoverBlocked( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	vec3_t dir, angles;
	vec3_t pos;
	vec3_t vec;
	float dot;
	vec3_t forward;

	if ( ent->classname != scr_const.func_door_rotating ) {
		return qfalse;
	}

	if ( ent->spawnflags & 32 ) {
		return qfalse;
	}

	VectorAdd( ent->r.absmin, ent->r.absmax, pos );
	VectorScale( pos, 0.5f, pos );

	VectorSubtract( pos, ent->r.currentOrigin, dir );
	vectoangles( dir, angles );

	if ( ent->rotate[YAW] ) {
		angles[YAW] += ent->angle;
	} else if ( ent->rotate[PITCH] ) {
		angles[PITCH] += ent->angle;
	}

	AngleVectors( angles, forward, NULL, NULL );

	VectorSubtract( activator->r.currentOrigin, pos, vec );

	VectorNormalize( vec );
	dot = DotProduct( vec, forward );

	if ( dot >= 0 ) {
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
================
Use_BinaryMover
================
*/
void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	qboolean isblocked = qfalse;
	qboolean nosound = qfalse;
	qboolean soft;

	soft = ( ent->flags & FL_SOFTACTIVATE );

	if ( level.time <= 4000 ) { // hack.  don't play door sounds if in the first /four/ seconds of game
		nosound = qtrue;
	}

	// only the master should be used
	if ( ent->flags & FL_TEAMSLAVE ) {

		// pass along flags for how door was activated
		if ( ent->flags & FL_SOFTACTIVATE ) {
			ent->teammaster->flags |= FL_SOFTACTIVATE;
		}

		Use_BinaryMover( ent->teammaster, other, activator );
		return;
	}

	// only check for blocking when opening, otherwise the door has no choice
	if ( ent->moverState == MOVER_POS1 || ent->moverState == MOVER_POS1ROTATE ) {
		isblocked = IsBinaryMoverBlocked( ent, other, activator );
	}

	if ( isblocked ) {
		// start moving 50 msec later, becase if this was player
		// triggered, level.time hasn't been advanced yet
		MatchTeamReverseAngleOnSlaves( ent, MOVER_1TO2ROTATE, level.time + 50 );

		// starting sound
		if ( !nosound ) {
			if ( soft ) {
				G_PlaySoundAlias( ent, ent->soundOpeningQuiet );
			} else {
				G_PlaySoundAlias( ent, ent->soundOpening );
			}
		}

		ent->s.loopSound = 0;

		// looping sound
		if ( !nosound ) {
			ent->s.loopSound = ent->soundOpenLoop;
		}

		// open areaportal
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qtrue );
		}
		return;
	}

	ent->activator = activator;

	if ( ent->nextTrain && ent->nextTrain->wait == -1 && ent->nextTrain->count == 1 ) {
		ent->nextTrain->count = 0;
		return;
	}

	if ( ent->moverState == MOVER_POS1 ) {

		// start moving 50 msec later, becase if this was player
		// triggered, level.time hasn't been advanced yet
		MatchTeam( ent, MOVER_1TO2, level.time + 50 );

		// play starting sound
		if ( !nosound ) {
			G_PlaySoundAlias( ent, ent->soundOpening );
		}

		ent->s.loopSound = 0;

		// set looping sound
		if ( !nosound ) {
			ent->s.loopSound = ent->soundOpenLoop;
		}

		// open areaportal
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qtrue );
		}
		return;
	}

	if ( ent->moverState == MOVER_POS1ROTATE ) {

		MatchTeam( ent, MOVER_1TO2ROTATE, level.time + 50 );

		// play starting sound
		if ( !nosound ) {
			if ( soft ) {
				G_PlaySoundAlias( ent, ent->soundOpeningQuiet );
			} else {
				G_PlaySoundAlias( ent, ent->soundOpening );
			}
		}

		ent->s.loopSound = 0;

		// set looping sound
		if ( !nosound ) {
			ent->s.loopSound = ent->soundOpenLoop;
		}

		// open areaportal
		if ( ent->teammaster == ent || !ent->teammaster ) {
			trap_AdjustAreaPortalState( ent, qtrue );
		}
		return;
	}

	// if all the way up, just delay before coming down
	if ( ent->moverState == MOVER_POS2 ) {
		if ( ent->flags & FL_TOGGLE ) {
			ent->nextthink = level.time + 50;
			return;
		}

		if ( ent->wait != -1000 ) {
			ent->nextthink = level.time + (int)ent->wait;
		}
		return;
	}

	// if all the way up, just delay before coming down
	if ( ent->moverState == MOVER_POS2ROTATE ) {
		if ( ent->flags & FL_TOGGLE ) {
			ent->nextthink = level.time + 50;   // do it *now* for toggles
		} else {
			ent->nextthink = level.time + (int)ent->wait;
		}
		return;
	}

	// only partway down before reversing
	if ( ent->moverState == MOVER_2TO1 ) {
		Blocked_Door( ent, NULL );

		if ( !nosound ) {
			G_PlaySoundAlias( ent, ent->soundOpening );
		}
		return;
	}

	// only partway up before reversing
	if ( ent->moverState == MOVER_1TO2 ) {
		Blocked_Door( ent, NULL );

		if ( !nosound ) {
			G_PlaySoundAlias( ent, ent->soundClosing );
		}
		return;
	}

	// only partway closed before reversing
	if ( ent->moverState == MOVER_2TO1ROTATE ) {
		Blocked_DoorRotate( ent, NULL );

		if ( !nosound ) {
			G_PlaySoundAlias( ent, ent->soundOpening );
		}
		return;
	}

	// only partway open before reversing
	if ( ent->moverState == MOVER_1TO2ROTATE ) {
		Blocked_DoorRotate( ent, NULL );

		if ( !nosound ) {
			if ( soft ) {
				G_PlaySoundAlias( ent, ent->soundClosingQuiet );
			} else {
				G_PlaySoundAlias( ent, ent->soundClosing );
			}
		}
		return;
	}
}

/*
================
InitMover

"pos1", "pos2", and "speed" should be set before calling,
so the movement delta can be calculated
================
*/
void InitMover( gentity_t *ent ) {
	vec3_t move;
	float distance;
	float light;
	vec3_t color;
	qboolean lightSet, colorSet;
	char        *sound;

	// if the "loopsound" key is set, use a constant looping sound when moving
	if ( G_SpawnString( "noise", "100", &sound ) ) {
		ent->s.loopSound = G_SoundAliasIndex( sound );
	}

	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnVector( "color", "1 1 1", color );
	if ( lightSet || colorSet ) {
		int r, g, b, i;

		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
	}

	if ( ent->classname == scr_const.func_rotating ) {
		ent->use = Use_Func_Rotate;
		ent->reached = NULL; // rotating can never reach
	} else {
		ent->use = Use_BinaryMover;
		ent->reached = Reached_BinaryMover;
	}

	ent->moverState = MOVER_POS1;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_MOVER;

	VectorCopy( ent->pos1, ent->r.currentOrigin );
	trap_LinkEntity( ent );

	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( ent->pos1, ent->s.pos.trBase );

	// calculate time to reach second position from speed
	VectorSubtract( ent->pos2, ent->pos1, move );
	distance = VectorLength( move );
	if ( !ent->speed ) {
		ent->speed = 100;
	}

	// open time based on speed
	ent->s.pos.trDuration = distance * 1000 / ent->speed;
	if ( ent->s.pos.trDuration <= 0 ) {
		ent->s.pos.trDuration = 1;
	}
	ent->gDurationBack = ent->gDuration = ent->s.pos.trDuration;

	// close time based on speed
	if ( ent->closespeed ) {
		ent->gDurationBack = distance * 1000 / ent->closespeed;
		if ( ent->gDurationBack <= 0 ) {
			ent->gDurationBack = 1;
		}
	}
}

/*
================
InitMoverRotate

"pos1", "pos2", and "speed" should be set before calling,
so the movement delta can be calculated
================
*/
void InitMoverRotate( gentity_t *ent ) {
	float light;
	vec3_t color;
	qboolean lightSet, colorSet;

	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnVector( "color", "1 1 1", color );
	if ( lightSet || colorSet ) {
		int r, g, b, i;

		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
	}

	ent->use = Use_BinaryMover;

	if ( !( ent->spawnflags & 64 ) ) { // STAYOPEN
		ent->reached = Reached_BinaryMover;
	}

	ent->moverState = MOVER_POS1ROTATE;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_MOVER;
	trap_LinkEntity( ent );

	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );

	if ( !ent->speed ) {
		ent->speed = 100;
	}

	ent->s.apos.trDuration = ent->speed;
	if ( ent->s.apos.trDuration <= 0 ) {
		ent->s.apos.trDuration = 1;
	}

	// store 'real' durations so doors can be opened/closed at different speeds
	ent->gDuration = ent->gDurationBack = ent->s.apos.trDuration;
}


/*
===============================================================================

DOOR

A use can be triggered either by a touch function, by being shot, or by being
targeted by another entity.

===============================================================================
*/

/*
================
Blocked_Door
================
*/
void Blocked_Door( gentity_t *ent, gentity_t *other ) {
	gentity_t *slave;
	int time;

	// remove anything other than a client
	if ( other ) {
		if ( !other->client ) {
			G_TempEntity( other->r.currentOrigin, EV_ITEM_POP );
			G_FreeEntity( other );
			return;
		}

		if ( ent->damage ) {
			G_Damage( other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH, 0 );
		}
	}

	if ( ent->spawnflags & 4 ) {
		return;     // crushers don't reverse
	}

	// reverse direction
	for ( slave = ent ; slave ; slave = slave->teamchain ) {
		time = level.time - ( slave->s.pos.trDuration - ( level.time - slave->s.pos.trTime ) );

		if ( slave->moverState == MOVER_1TO2 ) {
			SetMoverState( slave, MOVER_2TO1, time );
		} else {
			SetMoverState( slave, MOVER_1TO2, time );
		}
		trap_LinkEntity( slave );
	}
}

/*
================
Blocked_DoorRotate
================
*/
void Blocked_DoorRotate( gentity_t *ent, gentity_t *other ) {
	gentity_t       *slave;
	int time;

	// remove anything other than a client
	if ( other ) {
		if ( !other->client ) {
			G_TempEntity( other->r.currentOrigin, EV_ITEM_POP );
			G_FreeEntity( other );
			return;
		}

		if ( other->health <= 0 ) {
			G_Damage( other, ent, ent, NULL, NULL, 99999, 0, MOD_CRUSH, 0 );
		}

		if ( ent->damage ) {
			G_Damage( other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH, 0 );
		}
	}

	for ( slave = ent ; slave ; slave = slave->teamchain ) {
		// RF, trying to fix "stuck in door" bug
		time = level.time - ( slave->s.apos.trDuration - ( level.time - slave->s.apos.trTime ) );

		if ( slave->moverState == MOVER_1TO2ROTATE ) {
			SetMoverState( slave, MOVER_2TO1ROTATE, time );
		} else {
			SetMoverState( slave, MOVER_1TO2ROTATE, time );
		}
		trap_LinkEntity( slave );
	}
}

/*
================
Touch_DoorTrigger
================
*/
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, int touchMode ) {
	if ( !ent->parent->key ) {
		if ( ent->parent->moverState == MOVER_POS1
			 || ent->parent->moverState == MOVER_POS1ROTATE
			 || ent->parent->moverState == MOVER_2TO1ROTATE ) {
			Use_BinaryMover( ent->parent, ent, other );
		}
	}
}

/*
======================
Think_SpawnNewDoorTriggerInternal

All of the parts of a door have been spawned, so create
a trigger that encloses all of them
======================
*/
void Think_SpawnNewDoorTriggerInternal( gentity_t *ent ) {
	gentity_t       *other;
	vec3_t mins, maxs;
	int i, best;

	// set all of the slaves as shootable
	for ( other = ent ; other ; other = other->teamchain ) {
		other->takedamage = qtrue;
	}

	// find the bounds of everything on the team
	VectorCopy( ent->r.absmin, mins );
	VectorCopy( ent->r.absmax, maxs );

	for ( other = ent->teamchain ; other ; other = other->teamchain ) {
		AddPointToBounds( other->r.absmin, mins, maxs );
		AddPointToBounds( other->r.absmax, mins, maxs );
	}

	// find the thinnest axis, which will be the one we expand
	best = 0;
	for ( i = 1 ; i < 3 ; i++ ) {
		if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) {
			best = i;
		}
	}
	maxs[best] += 120;
	mins[best] -= 120;

	// create a trigger with this size
	other = G_Spawn();
	VectorCopy( mins, other->r.mins );
	VectorCopy( maxs, other->r.maxs );
	other->parent = ent;
	other->r.contents = CONTENTS_TRIGGER;
	other->touch = Touch_DoorTrigger;
	trap_LinkEntity( other );

	MatchTeam( ent, ent->moverState, level.time );
}

/*
======================
Think_SpawnNewDoorTrigger
======================
*/
void Think_SpawnNewDoorTrigger( gentity_t *ent ) {
	Think_SpawnNewDoorTriggerInternal( ent );
}

/*
======================
DoorRotateStartOpen
	START_OPEN rotating doors spawn already swung open and close on their own
======================
*/
void DoorRotateStartOpen( gentity_t *ent ) {
	ent->think = ReturnToPos1Rotate;

	ent->r.currentAngles[YAW] += ent->angle;

	SetMoverState( ent, MOVER_POS2ROTATE, level.time );

	// open areaportal
	if ( ent->teammaster == ent || !ent->teammaster ) {
		trap_AdjustAreaPortalState( ent, qtrue );
	}
}

/*
======================
Think_SpawnNewAutoDoorTrigger
======================
*/
void Think_SpawnNewAutoDoorTrigger( gentity_t *ent ) {
	if ( ent->spawnflags & 1 ) {    // START_OPEN
		DoorRotateStartOpen( ent );
	}
}

/*
======================
Think_MatchTeam
======================
*/
void Think_MatchTeam( gentity_t *ent ) {
	MatchTeam( ent, ent->moverState, level.time );
}

/*
==============
finishSpawningKeyedMover
==============
*/
void finishSpawningKeyedMover( gentity_t *ent ) {
	gentity_t       *slave;

	ent->nextthink = level.time + FRAMETIME;

	if ( !( ent->flags & FL_TEAMSLAVE ) ) {
		if ( ent->takedamage ) {                                    // non touch/shoot doors
			ent->think = Think_MatchTeam;
		} else if ( ent->classname == scr_const.func_door_rotating ) {
			ent->think = Think_SpawnNewAutoDoorTrigger;
		} else if ( ent->spawnflags & 8 ) {
			ent->think = Think_SpawnNewDoorTrigger;
		} else {
			ent->think = Think_MatchTeam;
		}

		// slaves have been marked as FL_TEAMSLAVE now, so they won't
		// finish their think on their own.  So set keys for teamed doors
		for ( slave = ent ; slave ; slave = slave->teamchain ) {
			if ( slave == ent ) {
				continue;
			}

			slave->key = ent->key;
		}
	}
}

/*
==============
Door_reverse_sounds
	The door has been marked as "START_OPEN" which means the open/closed
	positions have been swapped.
	This swaps the sounds around as well
==============
*/
void Door_reverse_sounds( gentity_t *ent ) {
	byte stemp;

	stemp = ent->soundOpening;
	ent->soundOpening = ent->soundClosing;
	ent->soundClosing = stemp;

	stemp = ent->soundCloseEnd;
	ent->soundCloseEnd = ent->soundOpenEnd;
	ent->soundOpenEnd = stemp;

	stemp = ent->soundOpenLoop;
	ent->soundOpenLoop = ent->soundCloseLoop;
	ent->soundCloseLoop = stemp;

	stemp = ent->soundOpeningQuiet;
	ent->soundOpeningQuiet = ent->soundClosingQuiet;
	ent->soundClosingQuiet = stemp;

	stemp = ent->soundOpenQuietEnd;
	ent->soundOpenQuietEnd = ent->soundCloseQuietEnd;
	ent->soundCloseQuietEnd = stemp;
}

/*
==============
DoorSetSounds
	get the sound alias indexes for the various door sounds
	(used by SP_func_door() and SP_func_door_rotating() )
==============
*/
void DoorSetSounds( gentity_t *ent ) {
	ent->soundOpening       = G_SoundAliasIndex( "door_opening" );
	ent->soundOpenEnd       = G_SoundAliasIndex( "door_open_end" );
	ent->soundClosing       = G_SoundAliasIndex( "door_closing" );
	ent->soundCloseEnd      = G_SoundAliasIndex( "door_close_end" );
	ent->soundOpenLoop      = G_SoundAliasIndex( "door_open_loop" );
	ent->soundCloseLoop     = G_SoundAliasIndex( "door_close_loop" );
	ent->soundLocked        = G_SoundAliasIndex( "door_locked" );

	ent->soundOpeningQuiet  = G_SoundAliasIndex( "door_opening_quiet" );
	ent->soundOpenQuietEnd  = G_SoundAliasIndex( "door_open_quiet_end" );
	ent->soundClosingQuiet  = G_SoundAliasIndex( "door_closing_quiet" );
	ent->soundCloseQuietEnd = G_SoundAliasIndex( "door_close_quiet_end" );
}

/*
==============
G_TryDoor
	seemed better to have this isolated.  this way i can get func_invisible_user's using the
	regular rules of doors.
==============
*/
void G_TryDoor( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	qboolean walking = qfalse;

	walking = ( ent->flags & FL_SOFTACTIVATE );

	if ( ( ent->s.apos.trType == TR_STATIONARY && ent->s.pos.trType == TR_STATIONARY ) ) {
		if ( ent->active == qfalse ) {
			if ( ent->key ) {   // door locked
				G_PlaySoundAlias( ent, ent->soundLocked );
				return;
			}

			Scr_AddEntityNum( activator->s.number, 0 );
			Scr_NotifyNum( ent->s.number, 0, scr_const.trigger, 1 );

			if ( ent->teammaster && ent->team && ent != ent->teammaster ) {
				ent->teammaster->active = qtrue;
				if ( walking ) {
					ent->teammaster->flags |= FL_SOFTACTIVATE;      // no noise generated
				}

				Use_BinaryMover( ent->teammaster, activator, activator );
			} else {
				ent->active = qtrue;
				if ( walking ) {
					ent->flags |= FL_SOFTACTIVATE;      // no noise
				}

				Use_BinaryMover( ent, activator, activator );
			}
		}
	}
}

/*QUAKED func_door (0 .5 .8) ? START_OPEN TOGGLE CRUSHER TOUCH
TOGGLE      wait in both the start and end states for a trigger event.
START_OPEN  the door to moves to its destination when spawned, and operate in reverse.
*/
void SP_func_door( gentity_t *ent ) {
	vec3_t abs_movedir;
	float distance;
	vec3_t size;
	float lip;
	int key;
	int health;

	DoorSetSounds( ent );

	ent->blocked = Blocked_Door;

	// default speed of 400
	if ( !ent->speed ) {
		ent->speed = 400;
	}

	// default wait of 2 seconds
	if ( !ent->wait ) {
		ent->wait = 2;
	}
	ent->wait *= 1000;

	// door keys.  1.1 keeps only "was a key given", not the number (0x2002D6E7)
	ent->key = G_SpawnInt( "key", "", &key );

	// default lip of 8 units
	G_SpawnFloat( "lip", "8", &lip );

	// default damage of 2 points
	G_SpawnInt( "dmg", "2", &ent->damage );

	// first position at start
	VectorCopy( ent->r.currentOrigin, ent->pos1 );

	// calculate second position
	trap_SetBrushModel( ent );
	G_SetMovedir( ent->r.currentAngles, ent->movedir );
	abs_movedir[0] = fabs( ent->movedir[0] );
	abs_movedir[1] = fabs( ent->movedir[1] );
	abs_movedir[2] = fabs( ent->movedir[2] );
	VectorSubtract( ent->r.maxs, ent->r.mins, size );
	distance = DotProduct( abs_movedir, size ) - lip;
	VectorMA( ent->pos1, distance, ent->movedir, ent->pos2 );

	if ( ent->spawnflags & 1 ) {    // START_OPEN - reverse position 1 and 2
		vec3_t temp;
		float tempf;

		VectorCopy( ent->pos2, temp );
		VectorCopy( ent->r.currentOrigin, ent->pos2 );
		VectorCopy( temp, ent->pos1 );

		// swap speeds if door has 'closespeed'
		if ( ent->closespeed ) {
			tempf = ent->speed;
			ent->speed = ent->closespeed;
			ent->closespeed = tempf;
		}

		// swap sounds
		Door_reverse_sounds( ent );
	}

	// TOGGLE
	if ( ent->spawnflags & 2 ) {
		ent->flags |= FL_TOGGLE;
	}

	InitMover( ent );

	if ( !( ent->flags & FL_TEAMSLAVE ) ) {
		G_SpawnInt( "health", "0", &health );
		if ( health ) {
			ent->takedamage = qtrue;
		}
	}

	ent->nextthink = level.time + FRAMETIME;
	ent->think = finishSpawningKeyedMover;
}

/*
==============
Use_Static
	toggle hide or show (including collisions) this entity
==============
*/
void Use_Static( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	} else {
		trap_LinkEntity( ent );
	}
}

/*
==============
Static_Pain
==============
*/
void Static_Pain( gentity_t *ent, gentity_t *attacker, int damage, const float *point,
				  int mod, const float *dir, int hitLoc ) {
	if ( level.time > ent->wait + ent->delay + rand() % 1000 + 500 ) {
		ent->wait = level.time;
	}
}

/*QUAKED func_leaky (0 .5 .8) ?
"type" - leaks particles of this type
*/
void SP_func_leaky( gentity_t *ent ) {
	trap_SetBrushModel( ent );
	trap_LinkEntity( ent );
	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
}

/*QUAKED func_static (0 .5 .8) ? start_invis pain painEFX
A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
*/
void SP_func_static( gentity_t *ent ) {
	int health;

	trap_SetBrushModel( ent );
	InitMover( ent );
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->use = Use_Static;

	if ( ent->spawnflags & 1 ) {
		trap_UnlinkEntity( ent );
	}

	if ( !( ent->flags & FL_TEAMSLAVE ) ) {
		G_SpawnInt( "health", "0", &health );
		if ( health ) {
			ent->takedamage = qtrue;
		}
	}

	if ( ent->spawnflags & 2 || ent->spawnflags & 4 ) {
		ent->pain = Static_Pain;

		if ( !ent->delay ) {
			ent->delay = 1000;
		} else {
			ent->delay *= 1000;
		}

		ent->takedamage = qtrue;
		ent->health = 9999;

		if ( !ent->count ) {
			ent->count = 4;
		}
	}
}

/*QUAKED func_rotating (0 .5 .8) ? START_ON STARTINVIS X_AXIS Y_AXIS
*/
void Use_Func_Rotate( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	if ( ent->spawnflags & 4 ) {
		ent->s.apos.trDelta[2] = ent->speed;
	} else if ( ent->spawnflags & 8 ) {
		ent->s.apos.trDelta[0] = ent->speed;
	} else {
		ent->s.apos.trDelta[1] = ent->speed;
	}

	if ( ent->spawnflags & 2 ) {
		ent->flags &= ~FL_TEAMSLAVE;
	}

	trap_LinkEntity( ent );
}

void SP_func_rotating( gentity_t *ent ) {
	if ( !ent->speed ) {
		ent->speed = 100;
	}

	// set the axis of rotation
	ent->s.apos.trType = TR_LINEAR;

	if ( ent->spawnflags & 1 ) {
		if ( ent->spawnflags & 4 ) {
			ent->s.apos.trDelta[2] = ent->speed;
		} else if ( ent->spawnflags & 8 ) {
			ent->s.apos.trDelta[0] = ent->speed;
		} else {
			ent->s.apos.trDelta[1] = ent->speed;
		}
	}

	if ( !ent->damage ) {
		ent->damage = 2;
	}

	trap_SetBrushModel( ent );
	InitMover( ent );

	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );

	if ( ent->spawnflags & 2 ) {
		ent->flags |= FL_TEAMSLAVE;
		trap_UnlinkEntity( ent );
	} else {
		trap_LinkEntity( ent );
	}
}


/*
===============================================================================

BOBBING

===============================================================================
*/

/*QUAKED func_bobbing (0 .5 .8) ? X_AXIS Y_AXIS
Normally bobs on the Z axis
*/
void SP_func_bobbing( gentity_t *ent ) {
	float height;
	float phase;

	G_SpawnFloat( "speed", "4", &ent->speed );
	G_SpawnFloat( "height", "32", &height );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "phase", "0", &phase );

	trap_SetBrushModel( ent );
	InitMover( ent );

	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );

	ent->s.pos.trDuration = ent->speed * 1000;
	ent->s.pos.trTime = ent->s.pos.trDuration * phase;
	ent->s.pos.trType = TR_SINE;

	// set the axis of bobbing
	if ( ent->spawnflags & 1 ) {
		ent->s.pos.trDelta[0] = height;
	} else if ( ent->spawnflags & 2 ) {
		ent->s.pos.trDelta[1] = height;
	} else {
		ent->s.pos.trDelta[2] = height;
	}
}

/*QUAKED func_pendulum (0 .5 .8) ?
"speed"		the number of degrees each way the pendulum swings, (30 default)
*/
void SP_func_pendulum( gentity_t *ent ) {
	float freq;
	float length;
	float phase;
	float speed;

	G_SpawnFloat( "speed", "30", &speed );
	G_SpawnInt( "dmg", "2", &ent->damage );
	G_SpawnFloat( "phase", "0", &phase );

	trap_SetBrushModel( ent );

	// find pendulum length
	length = fabs( ent->r.mins[2] );
	if ( length < 8 ) {
		length = 8;
	}

	freq = 1 / ( M_PI * 2 ) * sqrt( g_gravity.value / ( 3 * length ) );

	ent->s.pos.trDuration = ( 1000 / freq );

	InitMover( ent );

	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );

	ent->s.apos.trDuration = 1000 / freq;
	ent->s.apos.trTime = ent->s.apos.trDuration * phase;
	ent->s.apos.trType = TR_SINE;
	ent->s.apos.trDelta[2] = speed;
}

/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN TOGGLE X_AXIS Y_AXIS REVERSE FORCE STAYOPEN
*/
void SP_func_door_rotating( gentity_t *ent ) {
	int key;
	int health;

	if ( ent->spawnflags & 1 ) {    // START_OPEN implies TOGGLE
		ent->spawnflags |= 2;
	}
	ent->spawnflags &= ~64;

	DoorSetSounds( ent );

	// set the duration
	if ( !ent->speed ) {
		ent->speed = 1000;
	}

	// degrees door will open
	if ( !ent->angle ) {
		ent->angle = 90;
	}

	// reverse direction
	if ( ent->spawnflags & 16 ) {
		ent->angle *= -1;
	}

	// TOGGLE
	if ( ent->spawnflags & 2 ) {
		ent->flags |= FL_TOGGLE;
	}

	// door keys.  as in SP_func_door, only the presence of the key is kept
	ent->key = G_SpawnInt( "key", "", &key );

	// set the rotation axis
	VectorClear( ent->rotate );
	if ( ( ent->spawnflags & ( 4 | 8 ) ) == ( 4 | 8 ) ) {
		ent->rotate[1] = 1;
	} else if ( ent->spawnflags & 4 ) {
		ent->rotate[2] = 1;
	} else if ( ent->spawnflags & 8 ) {
		ent->rotate[0] = 1;
	} else {
		ent->rotate[1] = 1;
	}

	if ( VectorLength( ent->rotate ) > 1 ) { // check that rotation is only set for one axis
		G_Error( "Too many axis marked in func_door_rotating entity.  Only choose one axis of rotation. (defaulting to standard door rotation)" );
		VectorClear( ent->rotate );
		ent->rotate[1] = 1;
	}

	if ( !ent->wait ) {
		ent->wait = 2;
	}
	ent->wait *= 1000;

	trap_SetBrushModel( ent );

	InitMoverRotate( ent );

	if ( !( ent->flags & FL_TEAMSLAVE ) ) {
		G_SpawnInt( "health", "0", &health );
		if ( health ) {
			ent->takedamage = qtrue;
		}
	}

	ent->nextthink = level.time + FRAMETIME;
	ent->think = finishSpawningKeyedMover;

	ent->blocked = Blocked_DoorRotate;
}


/*
===============================================================================

TRIGGER_USE

===============================================================================
*/

/* entityState_t.dmgFlags values.  The trailing NULL is in the image
   (0x20069DC0) and the loop below walks past the last name into it. */
/* Not static: g_scr_main_mp.c walks the same table (0x20069D98) from
   GScr_SetCursorHint. */
char *predef_hintStrings[HINT_NUM_HINTS] = {
	"",
	"HINT_NONE",
	"HINT_ACTIVATE",
	"HINT_NOACTIVATE",
	"HINT_DOOR",
	"HINT_DOOR_LOCKED",
	"HINT_MG42",
	"HINT_HEALTH",
	"HINT_LADDER",
	"HINT_FRIENDLY",
	NULL
};

void use_trigger_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	if ( level.time > ent->wait ) {
		ent->wait = level.time + ent->delay;

		if ( !other->client ) {
			if ( ent->spawnflags & 1 ) {
				ent->spawnflags &= ~1;
			} else {
				ent->spawnflags |= 1;
			}
		}
	}
}

/*QUAKED trigger_use (.5 .5 .5) ?
"cursorhint"	one of the HINT_ names
"hintstring"	the localized string shown while the player is looking at it
*/
void trigger_use( gentity_t *ent ) {
	char        *s;
	char buffer[MAX_STRING_CHARS];
	int i;

	trap_SetBrushModel( ent );
	trap_LinkEntity( ent );

	ent->delay *= 1000;

	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;

	ent->r.contents = 0x00200000;       /* trigger_use's own content bit */
	ent->r.svFlags = SVF_NOCLIENT;
	ent->use = use_trigger_use;

	ent->s.dmgFlags = HINT_ACTIVATE;

	if ( G_SpawnString( "cursorhint", "", &s ) ) {
		if ( !Q_stricmp( "HINT_INHERIT", s ) ) {
			ent->s.dmgFlags = -1;
		} else {
			for ( i = 1 ; i < HINT_NUM_HINTS ; i++ ) {
				if ( !Q_stricmp( predef_hintStrings[i], s ) ) {
					ent->s.dmgFlags = i;
					break;
				}
			}
		}
	}

	ent->s.scale = 255;

	if ( G_SpawnString( "hintstring", "", &s ) ) {
		for ( i = 0 ; i < MAX_HINTSTRINGS ; i++ ) {
			trap_GetConfigstring( CS_HINTSTRINGS + i, buffer, sizeof( buffer ) );
			if ( !buffer[0] ) {
				trap_SetConfigstring( CS_HINTSTRINGS + i, s );
				ent->s.scale = (byte)i;
				break;
			}
			if ( !strcmp( s, buffer ) ) {
				ent->s.scale = (byte)i;
				break;
			}
		}

		if ( i == MAX_HINTSTRINGS ) {
			Com_Error( ERR_DROP, "\x15" "Too many different hintstring key values on trigger_use entities. Max allowed is %i", MAX_HINTSTRINGS );
		}
	}
}

/*
==========
G_Activate

  Generic activation routine for doors.  Nothing in the retail 1.1 module
  references it; the name is inferred from RTCW.
==========
*/
void G_Activate( gentity_t *ent, gentity_t *activator ) {
	if ( ( ent->s.apos.trType == TR_STATIONARY && ent->s.pos.trType == TR_STATIONARY )
		 && ent->active == qfalse ) {
		// trigger the ent if possible, if not, then we'll just wait at the marker until it opens
		if ( ent->key ) {  // ent locked
			return;
		}

		if ( ent->teammaster && ent->team && ent != ent->teammaster ) {
			ent->teammaster->active = qtrue;
			Use_BinaryMover( ent->teammaster, activator, activator );
		} else {
			ent->active = qtrue;
			Use_BinaryMover( ent, activator, activator );
		}
	}
}
