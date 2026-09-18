/*
 * cg_predict.c -- this file generates cg.predictedPlayerState by either
 * interpolating between snapshots from the server or locally predicting
 * ahead the client's movement.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_predict.c is the ancestor and every function
 * below is still its text; the CoD-specific differences are called out in the
 * comments where they occur.  The three that matter:
 *
 *   - cg.nextSnap is never NULL in 1.1.  CG_TransitionSnapshot assigns
 *     cg.snap = cg.nextSnap and leaves nextSnap alone, so "no next frame"
 *     is spelled `cg.nextSnap == cg.snap` and CG_BuildSolidList /
 *     CG_InterpolatePlayerState / CG_PredictPlayerState_Internal all read
 *     cg.nextSnap unconditionally where RTCW picks between the two.
 *   - the prediction pmove uses CG_TraceCapsule, not CG_Trace, for all three
 *     pmove_t trace slots (0x30029550).  CG_Trace survives for the view code.
 *   - CG_TouchTriggerPrediction still runs the bmodel trace but throws the
 *     result away: the push/teleport/objective trigger handling is gone.
 *
 * Functions are in address order, which is the original file order.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <math.h>

#include "cg_local.h"

#define PITCH                       0
#define YAW                         1
#define ROLL                        2

/* Owned by other units. */
void        CG_Printf( const char *msg, ... );                  /* cg_main.c 0x300206F0 */
void        CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime,
									   vec3_t out, vec3_t angles_out );  /* cg_ent.c 0x3001BAA0 */

/*
 * bg_pmove.c / bg_misc.c.  bg_public.h declares neither PM_UpdateViewAngles
 * nor the two item helpers.
 */
void        PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd,
								 void ( *trace )( trace_t *results, const vec3_t start,
												  const vec3_t mins, const vec3_t maxs,
												  const vec3_t end, int passEntityNum,
												  int contentMask ) );
qboolean    BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );
qboolean    BG_CanItemBeGrabbed( const entityState_t *ent, const playerState_t *ps,
								 qboolean onlyIfHave );

#define ENTITYNUM_WORLD             1022

#define SOLID_BMODEL                0xFFFFFF

#define ET_PLAYER                   1
#define ET_ITEM                     3

#define EF_NONSOLID_BMODEL          0x00000002
#define EF_CAPSULE                  0x00000010
#define EF_NODRAW                   0x00000100

#define CONTENTS_SOLID              0x00000001
#define CONTENTS_BODY               0x02000000

/*
 * The prediction clipmask (0x3002958A).  It is g_client_mp.c's MASK_CLIENTSOLID
 * and NOT bg_pmove.c's MASK_PLAYERSOLID (0x02810191): cgame predicts without
 * 0x80 and 0x100.  A spectator additionally loses CONTENTS_BODY and 0x10000
 * (0x3002959C masks with 0xFDFEFFFF).
 */
#define MASK_CLIENTSOLID            0x02810011
#define MASK_SPECTATORCLEAR         0x02010000

/* playerState_t.pm_flags; the two spellings the game module already uses. */
#define PMF_FOLLOW                  0x00010000
#define PMF_INWORLD                 0x00040000

/* bg_misc.c's eventnames[] index for "EV_ITEM_PICKUP". */
#define EV_ITEM_PICKUP              146


#define ANGLE2SHORT( x )            ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )

static pmove_t cg_pmove;

static int cg_numSolidEntities;
static centity_t   *cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static int cg_numTriggerEntities;
static centity_t   *cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];

/*
====================
CG_BuildSolidList

When a new cg.nextSnap has been set, this function builds a sublist
of the entities that are actually solid, to make for more
efficient collision detection
====================
*/
void CG_BuildSolidList( void ) {
	int i;
	centity_t   *cent;
	snapshot_t  *snap;
	entityState_t   *ent;

	cg_numSolidEntities = 0;
	cg_numTriggerEntities = 0;

	/* RTCW picks cg.snap when there is no next frame or a teleport is pending;
	   1.1 always takes cg.nextSnap (0x30028D54). */
	snap = cg.nextSnap;

	for ( i = 0 ; i < snap->numEntities ; i++ ) {
		cent = &cg_entities[ snap->entities[ i ].number ];
		ent = &cent->currentState;

		// dont clip again non-solid bmodels
		if ( cent->nextState.solid == SOLID_BMODEL && ( cent->nextState.eFlags & EF_NONSOLID_BMODEL ) ) {
			continue;
		}

		if ( ent->eType == ET_ITEM ) {
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}

		if ( cent->nextState.solid ) {
			cg_solidEntities[cg_numSolidEntities] = cent;
			cg_numSolidEntities++;
			continue;
		}
	}
}

/*
====================
CG_ClipMoveToEntities
====================
*/
static void CG_ClipMoveToEntities( const vec3_t start, const vec3_t mins, const vec3_t maxs,
								   const vec3_t end, int skipNumber, int mask, int capsule,
								   trace_t *tr ) {
	int i, x, zd, zu;
	int contents;
	trace_t trace;
	entityState_t   *ent;
	clipHandle_t cmodel;
	vec3_t bmins, bmaxs;
	vec3_t origin, angles;
	centity_t   *cent;

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if ( ent->solid == SOLID_BMODEL ) {
			// special value for bmodel
			cmodel = trap_CM_InlineModel( ent->index );
			BG_EvaluateTrajectory( &cent->currentState.apos, cg.physicsTime, angles );
			BG_EvaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin );
		} else {
			// encoded bbox
			x = ( ent->solid & 255 );
			zd = ( ( ent->solid >> 8 ) & 255 );
			zu = ( ( ent->solid >> 16 ) & 255 ) - 32;

			bmins[0] = bmins[1] = (float)-x;
			bmaxs[0] = bmaxs[1] = (float)x;
			bmins[2] = (float)-zd;
			bmaxs[2] = (float)zu;

			/* CoD addition: the trace only sees this entity if its own contents
			   are in the mask (0x30028ECF). */
			if ( ent->eType == ET_PLAYER ) {
				contents = CONTENTS_BODY;
			} else {
				contents = CONTENTS_SOLID;
			}
			if ( !( mask & contents ) ) {
				continue;
			}

			// use bbox or capsule
			/* Pass the entity contents as the third trap arg (0x300307A0/
			   0x300307B0 reach CM_TempBoxModel's contents slot).  Retail's
			   client left it unset and the engine clipped against a stale
			   code pointer; a build where that dword lacks
			   CONTENTS_SOLID|CONTENTS_BODY has CM_Trace gate the capsule away,
			   so the crosshair never hits a teammate.  See cg_syscalls_mp.c. */
			if ( ent->eFlags & EF_CAPSULE ) {
				cmodel = trap_CM_TempCapsuleModel( bmins, bmaxs, contents );
			} else {
				cmodel = trap_CM_TempBoxModel( bmins, bmaxs, contents );
			}
			VectorCopy( vec3_origin, angles );
			VectorCopy( cent->lerpOrigin, origin );
		}
		// use bbox of capsule
		if ( capsule ) {
			trap_CM_TransformedCapsuleTrace( &trace, start, end,
											 mins, maxs, cmodel, mask, origin, angles );
		} else {
			trap_CM_TransformedBoxTrace( &trace, start, end,
										 mins, maxs, cmodel, mask, origin, angles );
		}

		if ( trace.allsolid || trace.fraction < tr->fraction ) {
			trace.entityNum = (unsigned short)ent->number;
			*tr = trace;
		} else if ( trace.startsolid ) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}

/*
================
CG_Trace
================
*/
void    CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs,
				  const vec3_t end, int skipNumber, int mask ) {
	trace_t t;

	trap_CM_BoxTrace( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities( start, mins, maxs, end, skipNumber, mask, qfalse, &t );

	*result = t;
}

/*
================
CG_TraceCapsule
================
*/
void    CG_TraceCapsule( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs,
						 const vec3_t end, int skipNumber, int mask ) {
	trace_t t;

	trap_CM_CapsuleTrace( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities( start, mins, maxs, end, skipNumber, mask, qtrue, &t );

	*result = t;
}

/*
================
CG_PointContents
================
*/
int     CG_PointContents( const vec3_t point, int passEntityNum, int contentMask ) {
	int i;
	entityState_t   *ent;
	centity_t   *cent;
	clipHandle_t cmodel;
	int contents;

	contents = trap_CM_PointContents( point, 0 );

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];

		ent = &cent->currentState;

		if ( ent->number == passEntityNum ) {
			continue;
		}

		if ( ent->solid != SOLID_BMODEL ) { // special value for bmodel
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->index );
		if ( !cmodel ) {
			continue;
		}

		/* RTCW hands the trap ent->origin / ent->angles; 1.1 hands it the
		   interpolated pair (0x3002919A). */
		contents |= trap_CM_TransformedPointContents( point, cmodel,
													 cent->lerpOrigin, cent->lerpAngles );
	}

	return contents & contentMask;
}

/*
========================
CG_InterpolatePlayerState

Generates cg.predictedPlayerState by interpolating between
cg.snap->ps and cg.nextSnap->ps
========================
*/
static void CG_InterpolatePlayerState( qboolean grabAngles ) {
	float f;
	int i;
	playerState_t   *out;
	snapshot_t      *prev, *next;

	out = &cg.predictedPlayerState;
	prev = cg.snap;
	next = cg.nextSnap;

	/* RTCW copies prev->ps; 1.1 copies the newer one, so every field this
	   function does not lerp comes from cg.nextSnap (0x300291F0). */
	*out = cg.nextSnap->ps;

	cg.weaponInfo = bg_weaponInfo[ cg.predictedPlayerState.weapon ];

	// if we are still allowing local input, short circuit the view angles
	if ( grabAngles ) {
		usercmd_t cmd;
		int cmdNum;

		cmdNum = trap_GetCurrentCmdNumber();
		trap_GetUserCmd( cmdNum, &cmd );

		PM_UpdateViewAngles( out, &cmd, CG_TraceCapsule );
	}

	if ( next->serverTime <= prev->serverTime ) {
		return;
	}

	f = (float)( cg.time - prev->serverTime ) / ( next->serverTime - prev->serverTime );

	i = next->ps.bobCycle;
	if ( i < prev->ps.bobCycle ) {
		i += 256;       // handle wraparound
	}
	/* the truncation really is inside the parentheses (__ftol2 at 0x30029284
	   runs before the add). */
	out->bobCycle = prev->ps.bobCycle + (int)( f * ( i - prev->ps.bobCycle ) );

	out->aimSpreadScale = prev->ps.aimSpreadScale +
						  f * ( next->ps.aimSpreadScale - prev->ps.aimSpreadScale );

	for ( i = 0 ; i < 3 ; i++ ) {
		out->origin[i] = prev->ps.origin[i] + f * ( next->ps.origin[i] - prev->ps.origin[i] );
		if ( !grabAngles ) {
			out->viewangles[i] = LerpAngle(
				prev->ps.viewangles[i], next->ps.viewangles[i], f );
		}
		out->velocity[i] = prev->ps.velocity[i] +
						   f * ( next->ps.velocity[i] - prev->ps.velocity[i] );
	}

	out->viewHeightCurrent = prev->ps.viewHeightCurrent +
							 f * ( next->ps.viewHeightCurrent - prev->ps.viewHeightCurrent );
	out->leanf = prev->ps.leanf + f * ( next->ps.leanf - prev->ps.leanf );
	out->fWeaponPosFrac = prev->ps.fWeaponPosFrac +
						  f * ( next->ps.fWeaponPosFrac - prev->ps.fWeaponPosFrac );
}

/*
===================
CG_TouchItem
===================
*/
static void CG_TouchItem( centity_t *cent ) {
	if ( !cg_predictItems.integer ) {
		return;
	}

	if ( !BG_PlayerTouchesItem( &cg.predictedPlayerState, &cent->currentState, cg.time ) ) {
		return;
	}

	// never pick an item up twice in a prediction
	/* cent->miscTime; cg_local.h leaves it inside unknown_0x1F0 (0x300293C4). */
	if ( *(int *)&cent->unknown_0x1F0[4] == cg.time ) {
		return;
	}

	if ( !BG_CanItemBeGrabbed( &cent->currentState, &cg.predictedPlayerState, qtrue ) ) {
		return;     // can't hold it
	}

	// remove it from the frame so it won't be drawn
	cent->currentState.eFlags |= EF_NODRAW;

	// don't touch it again this prediction
	*(int *)&cent->unknown_0x1F0[4] = cg.time;

	// grab it
	BG_AddPredictableEventToPlayerstate( EV_ITEM_PICKUP, cent->currentState.index,
										 &cg.predictedPlayerState );
}

/*
=========================
CG_TouchTriggerPrediction

Predict items.  The bmodel trace below is still issued and its result still
discarded -- the push / teleport / objective trigger arms RTCW hangs off it are
not in 1.1.
=========================
*/
static void CG_TouchTriggerPrediction( void ) {
	int i;
	trace_t trace;
	entityState_t   *ent;
	clipHandle_t cmodel;
	centity_t       *cent;
	qboolean spectator;

	// dead clients don't activate triggers
	if ( cg.predictedPlayerState.pm_type >= PM_DEAD ) {
		return;
	}

	spectator = ( cg.predictedPlayerState.pm_type == PM_SPECTATOR );

	if ( cg.predictedPlayerState.pm_type != PM_NORMAL
		 && cg.predictedPlayerState.pm_type != PM_NORMAL_LINKED
		 && !spectator ) {
		return;
	}

	for ( i = 0 ; i < cg_numTriggerEntities ; i++ ) {
		cent = cg_triggerEntities[ i ];
		ent = &cent->currentState;

		if ( ent->eType == ET_ITEM && !spectator ) {
			CG_TouchItem( cent );
			continue;
		}

		if ( ent->solid != SOLID_BMODEL ) {
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->index );
		if ( !cmodel ) {
			continue;
		}

		trap_CM_BoxTrace( &trace, cg.predictedPlayerState.origin, cg.predictedPlayerState.origin,
						  cg_pmove.mins, cg_pmove.maxs, cmodel, -1 );
	}
}

/*
=================
CG_PredictPlayerState_Internal

Generates cg.predictedPlayerState for the current cg.time
cg.predictedPlayerState is guaranteed to be valid after exiting.

For demo playback, this will be an interpolation between two valid
playerState_t.

For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.

Each new snapshot will usually have one or more new usercmd over the last,
but we simulate all unacknowledged commands each time, not just the new ones.

We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
=================
*/
static void CG_PredictPlayerState_Internal( void ) {
	int cmdNum, current;
	playerState_t oldPlayerState;
	qboolean moved;
	usercmd_t oldestCmd;
	usercmd_t latestCmd;
	vec3_t deltaAngles;

	// demo playback just copies the moves
	if ( cg.demoPlayback || ( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {
		CG_InterpolatePlayerState( qfalse );
		return;
	}

	// non-predicting local movement will grab the latest angles
	if ( cg_nopredict.integer || g_synchronousClients.integer ) {
		CG_InterpolatePlayerState( qtrue );
		return;
	}

	// prepare for pmove
	/* all three trace slots get the capsule trace (0x30029550). */
	cg_pmove.trace = CG_TraceCapsule;
	cg_pmove.trace2 = CG_TraceCapsule;
	cg_pmove.trace3 = CG_TraceCapsule;
	cg_pmove.ps = &cg.predictedPlayerState;
	cg_pmove.pointcontents = CG_PointContents;

	if ( cg_pmove.ps->pm_type >= PM_DEAD ) {
		cg_pmove.tracemask = MASK_CLIENTSOLID & ~CONTENTS_BODY;
	} else {
		cg_pmove.tracemask = MASK_CLIENTSOLID;
	}
	if ( cg.snap->ps.pm_type == PM_SPECTATOR ) {
		cg_pmove.tracemask &= ~MASK_SPECTATORCLEAR;     // spectators can fly through bodies
	}

	current = trap_GetCurrentCmdNumber();

	// if we don't have the commands right after the snapshot, we
	// can't accurately predict a current position, so just freeze at
	// the last good position we had
	cmdNum = current - CMD_BACKUP + 1;
	if ( !trap_GetUserCmd( cmdNum, &oldestCmd ) ) {
		if ( cg_showmiss.integer ) {
			CG_Printf( "exceeded PACKET_BACKUP on commands\n" );
		}
		return;
	}

	// save the state before the pmove so we can detect transitions
	oldPlayerState = cg.predictedPlayerState;

	// get the latest command so we can know which commands are from previous map_restarts
	trap_GetUserCmd( current, &latestCmd );

	// get the most recent information we have, even if
	// the server time is beyond our current cg.time
	cg.predictedPlayerState = cg.nextSnap->ps;
	cg.physicsTime = cg.nextSnap->serverTime;

	cg.weaponInfo = bg_weaponInfo[ cg.predictedPlayerState.weapon ];

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Setvar( &pmove_msec, "8" );
	} else if ( pmove_msec.integer > 33 )     {
		trap_Cvar_Setvar( &pmove_msec, "33" );
	}

	cg_pmove.pmove_fixed = pmove_fixed.integer;
	cg_pmove.pmove_msec = pmove_msec.integer;

	// run cmds
	moved = qfalse;
	for ( cmdNum = current - CMD_BACKUP + 1 ; cmdNum <= current ; cmdNum++ ) {
		// get the command
		trap_GetUserCmd( cmdNum, &cg_pmove.cmd );

		if ( cg_pmove.pmove_fixed ) {
			PM_UpdateViewAngles( cg_pmove.ps, &cg_pmove.cmd, CG_TraceCapsule );
		}

		// don't do anything if the time is before the snapshot player time
		if ( cg_pmove.cmd.serverTime <= cg.predictedPlayerState.commandTime ) {
			continue;
		}

		// don't do anything if the command was from a previous map_restart
		if ( cg_pmove.cmd.serverTime > latestCmd.serverTime ) {
			continue;
		}

		// get the previous command
		if ( !trap_GetUserCmd( cmdNum - 1, &cg_pmove.oldcmd ) ) {
			continue;
		}

		// check for a prediction error from last frame
		// on a lan, this will often be the exact value
		// from the snapshot, but on a wan we will have
		// to predict several commands to get to the point
		// we want to compare
		if ( cg.predictedPlayerState.commandTime == oldPlayerState.commandTime ) {
			vec3_t delta;
			float len;
			vec3_t adjusted;

			CG_AdjustPositionForMover( cg.predictedPlayerState.origin,
									   cg.predictedPlayerState.groundEntityNum,
									   cg.physicsTime, cg.oldTime, adjusted, deltaAngles );
			// add the deltaAngles (fixes jittery view while riding trains)
			cg.predictedPlayerState.delta_angles[YAW] += ANGLE2SHORT( deltaAngles[YAW] );

			if ( cg_showmiss.integer ) {
				if ( oldPlayerState.origin[0] != adjusted[0]
					 || oldPlayerState.origin[1] != adjusted[1]
					 || oldPlayerState.origin[2] != adjusted[2] ) {
					CG_Printf( "prediction error\n" );
				}
			}
			VectorSubtract( oldPlayerState.origin, adjusted, delta );
			len = (float)sqrt( delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2] );
			if ( len > 0.1f ) {
				if ( cg_showmiss.integer ) {
					CG_Printf( "Prediction miss: %f\n", len );
				}
				if ( cg_errordecay.integer ) {
					int t;
					float f;

					/* cg.predictedErrorTime; cg_local.h leaves it as
					   unknown_0x27348 (0x30029839). */
					t = cg.time - *(int *)cg.unknown_0x27348;
					f = ( cg_errordecay.value - t ) / cg_errordecay.value;
					if ( f < 0 ) {
						f = 0;
					}
					if ( f > 0 && cg_showmiss.integer ) {
						CG_Printf( "Double prediction decay: %f\n", f );
					}
					VectorScale( cg.predictedError, f, cg.predictedError );
				} else {
					VectorClear( cg.predictedError );
				}
				*(int *)cg.unknown_0x27348 = cg.oldTime;
				VectorAdd( delta, cg.predictedError, cg.predictedError );
			}
		}

		if ( cg_pmove.pmove_fixed ) {
			cg_pmove.cmd.serverTime = ( ( cg_pmove.cmd.serverTime + pmove_msec.integer - 1 )
										/ pmove_msec.integer ) * pmove_msec.integer;
		}

		// if we are not rendering, ignore all input
		if ( cg_norender.integer ) {
			cg_pmove.cmd.buttons = 0;
			cg_pmove.cmd.wbuttons &= 0xC0;
			cg_pmove.cmd.forwardmove = 0;
			cg_pmove.cmd.rightmove = 0;
			cg_pmove.cmd.upmove = 0;
			cg_pmove.cmd.angles[0] = cg_pmove.oldcmd.angles[0];
			cg_pmove.cmd.angles[1] = cg_pmove.oldcmd.angles[1];
			cg_pmove.cmd.angles[2] = cg_pmove.oldcmd.angles[2];
			if ( cg_pmove.cmd.serverTime - cg.predictedPlayerState.commandTime > 1 ) {
				cg_pmove.cmd.serverTime = cg.predictedPlayerState.commandTime + 1;
			}
		}

		Pmove( &cg_pmove );

		// add push trigger movement effects
		CG_TouchTriggerPrediction();

		moved = qtrue;
	}

	cg.weaponInfo = bg_weaponInfo[ cg.predictedPlayerState.weapon ];

	if ( cg_showmiss.integer > 1 ) {
		CG_Printf( "[%i : %i] ", cg_pmove.cmd.serverTime, cg.time );
	}

	if ( !moved ) {
		if ( cg_showmiss.integer ) {
			CG_Printf( "no prediction run\n" );
		}
		return;
	}

	// adjust for the movement of the groundentity
	CG_AdjustPositionForMover( cg.predictedPlayerState.origin,
							   cg.predictedPlayerState.groundEntityNum,
							   cg.physicsTime, cg.time,
							   cg.predictedPlayerState.origin, deltaAngles );

	// fire events and other transition triggered things
	CG_TransitionPlayerState( &cg.predictedPlayerState, &oldPlayerState );
}

/*
=================
CG_PredictPlayerState

Runs the prediction and republishes the result on the local client's entity so
the view and the player model see the predicted position this frame.
=================
*/
void CG_PredictPlayerState( void ) {
	centity_t   *cent;

	CG_PredictPlayerState_Internal();

	cent = &cg_entities[ cg.predictedPlayerState.clientNum ];
	VectorCopy( cg.predictedPlayerState.origin, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );
}
