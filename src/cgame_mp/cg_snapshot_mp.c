/*
 * cg_snapshot.c -- things that happen on snapshot transition, not necessarily
 * every single rendered frame.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_snapshot.c is the ancestor; the seven functions
 * and their order are unchanged.  What CoD changed:
 *
 *   - cg.nextSnap is never cleared.  CG_TransitionSnapshot does
 *     cg.snap = cg.nextSnap and stops there, so CG_ProcessSnapshots spells
 *     "no next frame" as `cg.nextSnap == cg.snap` where RTCW writes
 *     `!cg.nextSnap`.  Nothing in 1.1 ever nulls either pointer once set.
 *   - centity_t has no `interpolate`; `currentValid` does both jobs.
 *   - CG_SetNextSnap grew the per-client half: it drives bg_clientinfo[] out of
 *     the snapshot's 64 clientState_t records (names, model and attach-model
 *     configstrings) and rebuilds each client's DObj.  That is where the
 *     92-byte clientState_t layout below comes from.
 *   - CG_ResetEntity is where nextState is promoted to currentState, and it
 *     addresses cent->nextState throughout, including the eType switch.
 *
 * Functions are in address order, which is the original file order.
 *
 * @fidelity: likely
 */

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>

#include "cg_local.h"

/* Owned by other units. */
void        CG_Printf( const char *msg, ... );                  /* cg_main.c 0x300206F0 */
void        CG_Error( const char *msg, ... );                   /* cg_main.c 0x30020750 */
const char *CG_ConfigString( int index );                       /* cg_main.c 0x30021B00 */
const char *CG_SafeTranslateString_Internal( const char *module, const char *reference );
                                                                /* cg_main.c 0x30022840 */
void        CG_SetFrameInterpolation( void );                   /* cg_ent.c 0x3001BBF0 */
void        CG_CheckEvents( centity_t *cent );                  /* cg_event.c 0x3001EA10 */
void        CG_CheckPreEvents( centity_t *cent );               /* cg_event.c 0x3001EB10 */
void        CG_AddLagometerSnapshotInfo( snapshot_t *snap );    /* cg_draw.c 0x30015400 */
void        CG_ResetPlayerEntity( centity_t *cent );            /* cg_players.c 0x300285C0 */
void        CG_ExecuteNewServerCommands( int latestSequence );  /* cg_servercmds.c 0x3002E580 */
void        CG_UpdateHandViewmodels( const char *modelName );   /* cg_weapons.c 0x30035FD0 */

/* bg_misc.c.  BG_UpdatePlayerDObj is bg_public.h's -- one source, two
   builds, behind bg_animation.c's CGAMEDLL conditional. */
void        BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );

/* bg_animation.c's clocks and the root script animation. */
extern int          bgs_time;
extern int          bgs_animTime;
extern scr_anim_t   bgs_rootAnim;

#define ET_PLAYER                   1
#define ET_CORPSE                   2

#define EF_TELEPORT_BIT             0x00000008
/* set on a corpse that inherits the client's live anim tree instead of
   restarting it (CG_ResetEntity 0x3002F986). */
#define EF_CORPSE_KEEPANIM          0x00000800

/* playerState_t.pm_flags; the two spellings the game module already uses. */
#define PMF_FOLLOW                  0x00010000
#define PMF_INWORLD                 0x00040000

/* playerState_t.stats[] index the server bumps on respawn (g_client_mp.c). */
#define STAT_SPAWN_COUNT            5

/* configstring bases; both are g_utils_mp.c's on the server side. */
#define CS_MODELS                   268
#define CS_TAGS                     108

#define SNAPFLAG_NOT_ACTIVE         2
#define SNAPFLAG_SERVERCOUNT        4

/*
 * The 92-byte per-client record cg_public.h carries size-only.  CG_SetNextSnap
 * (0x3002FCD0..0x3002FF5B) is the only reader in the module: it drives exactly
 * the bg_clientinfo[] members the server fills in ClientEndFrame, through the
 * same two configstring bases.
 */
typedef struct cgClientState_t {
	int clientNum;                                  /* +0x00 -> ci->clientNum, 0x3002FCF3 */
	int team;                                       /* +0x04 -> ci->team,      0x3002FD00 */
	int modelIndex;                                 /* +0x08 CS_MODELS + this, 0x3002FD7E */
	int attachModelIndex[MAX_ATTACHED_MODELS];      /* +0x0C CS_MODELS + this, 0x3002FE15 */
	int attachTagIndex[MAX_ATTACHED_MODELS];        /* +0x24 CS_TAGS   + this, 0x3002FE96 */
	char name[32];                                  /* +0x3C strncpy 31,       0x3002FD6D */
} cgClientState_t;
CG_ASSERT_SIZE( cgClientState_t, 92 );

/*
==================
CG_ResetEntity

Promotes cent->nextState and restarts everything that must not interpolate
across the gap.  Every read below is of nextState, including the eType switch
(0x3002F8A9).
==================
*/
static void CG_ResetEntity( centity_t *cent ) {
	clientInfo_t    *ci;
	int num;
	void            *corpseTree;

	cent->currentState = cent->nextState;

	memset( cent->unknown_0x210, 0, 12 );
	cent->currentValid = qtrue;
	*(int *)&cent->unknown_0x1F0[0] = 0;

	BG_EvaluateTrajectory( &cent->nextState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.time, cent->lerpAngles );

	switch ( cent->nextState.eType ) {
	case 0:
	case 4:
		cent->previousEventSequence = 0;
		cent->previousPreEventSequence = 0;
		break;

	case ET_PLAYER:
		cent->previousEventSequence = cent->nextState.eventSequence;
		cent->previousPreEventSequence = cent->nextState.eventSequence;

		// hand the live view state to the animation layer before it is zeroed
		ci = &bg_clientinfo[ cent->currentState.clientNum ];
		ci->leanAmount = cent->nextState.angles2[1];
		ci->leanFraction = cent->nextState.leanf;
		ci->viewPitch = cent->lerpAngles[0];
		ci->viewYaw = cent->lerpAngles[1];
		ci->viewRoll = cent->lerpAngles[2];

		cent->lerpAngles[0] = 0;
		cent->lerpAngles[2] = 0;

		CG_ResetPlayerEntity( cent );
		break;

	case ET_CORPSE:
		// freeze a copy of the client's animation record on the corpse
		num = cent->currentState.number - MAX_CLIENTS;
		ci = &bg_clientinfo[ cent->currentState.clientNum ];
		corpseTree = cgs.corpseinfo[num].animTree;
		cgs.corpseinfo[num] = *ci;
		cgs.corpseinfo[num].animTree = corpseTree;

		if ( cent->currentState.eFlags & EF_CORPSE_KEEPANIM ) {
			cent->previousEventSequence = 0;
			cent->previousPreEventSequence = 0;
			trap_syscall_0xB8( (int)ci->animTree, (int)corpseTree );
		} else {
			cent->previousEventSequence = cent->nextState.eventSequence;
			cent->previousPreEventSequence = cent->nextState.eventSequence;
			/* the whole dword, not bgs_rootAnim.index: 0x3002F9C6 is a plain
			   `mov ecx, bgs_rootAnimHandle` with no movzx, and the wrapper
			   narrows it.  Same value -- index is the low half -- so the C4761
			   this raises is retail's, not a slip.  Do not "fix" it to .index. */
			trap_XAnimClearTreeGoalWeights( (int)ci->animTree, *(int *)&bgs_rootAnim, 0 );
		}
		break;

	default:
		cent->previousEventSequence = cent->nextState.eventSequence;
		cent->previousPreEventSequence = cent->nextState.eventSequence;
		break;
	}
}

/*
===============
CG_TransitionEntity

cent->nextState is moved to cent->currentState and events are fired.  Nothing
in 1.1 reaches the standalone copy; every call site got it inlined.
===============
*/
static void CG_TransitionEntity( centity_t *cent ) {
	cent->currentState = cent->nextState;
	cent->currentValid = qtrue;

	// check for events
	CG_CheckEvents( cent );
}

/*
==================
CG_SetInitialSnapshot

This will only happen on the very first snapshot, or on a server restart.
All other times will use CG_TransitionSnapshot instead.
==================
*/
static void CG_SetInitialSnapshot( snapshot_t *snap ) {
	centity_t   *cent;

	cg.snap = snap;

	cg.time = snap->serverTime;
	bgs_time = cg.time;
	cg.oldTime = cg.time;

	for ( cent = cg_entities ; cent < cg_entities + MAX_GENTITIES ; cent++ ) {
		cent->currentValid = qfalse;
	}

	trap_syscall_0xD7( 1.0f, 0 );

	// set our local weapon selection pointer to
	// what the server has indicated the current weapon is
	CG_Respawn();
}

/*
===================
CG_TransitionSnapshot

The transition point from snap to nextSnap has passed
===================
*/
static void CG_TransitionSnapshot( void ) {
	centity_t           *cent;
	clientInfo_t        *ci;
	const cgClientState_t   *cs;
	snapshot_t          *oldFrame;
	void                *animTree;
	int i;

	// clear the currentValid flag for all entities in the existing snapshot
	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cg_entities[ cg.snap->entities[ i ].number ].currentValid = qfalse;
	}

	// retire the clientInfo of anyone who was in the old frame and did not
	// re-announce himself in it
	for ( i = 0 ; i < cg.snap->numClients ; i++ ) {
		cs = (const cgClientState_t *)&cg.snap->clients[ i ];
		ci = &bg_clientinfo[ cs->clientNum ];

		if ( *(int *)ci->unknown_0x004 ) {
			*(int *)ci->unknown_0x004 = 0;
		} else {
			animTree = ci->animTree;
			memset( ci, 0, sizeof( clientInfo_t ) );
			ci->animTree = animTree;

			trap_SafeDObjFree( cs->clientNum, 1 );
			cg.iEntityLastType[ cs->clientNum ] = 0;
			cg.pEntityLastXModel[ cs->clientNum ] = NULL;
		}
	}

	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	if ( cg.snap->ps.pm_flags & ( PMF_FOLLOW | PMF_INWORLD ) ) {
		cent = &cg_entities[ cg.snap->ps.clientNum ];
		cent->currentState = cent->nextState;
		cent->currentValid = qtrue;
	}

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		CG_TransitionEntity( cent );
	}

	// if we are not doing client side movement prediction for any
	// reason, then the client events and view changes will be issued now
	if ( cg.demoPlayback || ( cg.snap->ps.pm_flags & PMF_FOLLOW )
		 || cg_nopredict.integer || g_synchronousClients.integer ) {
		CG_TransitionPlayerState( &cg.snap->ps, &oldFrame->ps );
	}
}

/*
===================
CG_SetNextSnap

A new snapshot has just been read in from the client system.
===================
*/
static void CG_SetNextSnap( snapshot_t *snap ) {
	int num;
	int i;
	entityState_t       *es;
	centity_t           *cent;
	clientInfo_t        *ci;
	const cgClientState_t   *cs;
	const char          *s;

	cg.nextSnap = snap;

	trap_syscall_0x94();
	CG_SetFrameInterpolation();

	// execute any server string commands before transitioning entities
	CG_ExecuteNewServerCommands( snap->serverCommandSequence );

	for ( num = 0 ; num < snap->numClients ; num++ ) {
		cs = (const cgClientState_t *)&snap->clients[ num ];
		ci = &bg_clientinfo[ cs->clientNum ];

		ci->infoValid = qtrue;
		*(int *)ci->unknown_0x004 = 1;
		ci->clientNum = cs->clientNum;
		ci->team = cs->team;

		if ( strcmp( ci->name, cs->name ) ) {
			if ( ci->name[0] ) {
				trap_GameMessage( va( "%s^7 %s %s", ci->name,
									  CG_SafeTranslateString_Internal( "cgame",
																	   "CGAME_PLAYERRENAMES" ),
									  cs->name ),
								  cg_gameMessageWidth.integer );
			}
			strncpy( ci->name, cs->name, sizeof( ci->name ) - 1 );
			ci->name[ sizeof( ci->name ) - 1 ] = 0;
		}

		s = CG_ConfigString( CS_MODELS + cs->modelIndex );
		if ( strcmp( ci->modelName, s ) ) {
			strncpy( ci->modelName, s, sizeof( ci->modelName ) - 1 );
			ci->modelName[ sizeof( ci->modelName ) - 1 ] = 0;
			ci->dobjNeedsUpdate = 1;
		}

		for ( i = 0 ; i < MAX_ATTACHED_MODELS ; i++ ) {
			s = CG_ConfigString( CS_MODELS + cs->attachModelIndex[i] );
			if ( strcmp( ci->attachModelNames[i], s ) ) {
				strncpy( ci->attachModelNames[i], s, sizeof( ci->attachModelNames[i] ) - 1 );
				ci->attachModelNames[i][ sizeof( ci->attachModelNames[i] ) - 1 ] = 0;
				ci->dobjNeedsUpdate = 1;
			}

			s = CG_ConfigString( CS_TAGS + cs->attachTagIndex[i] );
			if ( strcmp( ci->attachTagNames[i], s ) ) {
				strncpy( ci->attachTagNames[i], s, sizeof( ci->attachTagNames[i] ) - 1 );
				ci->attachTagNames[i][ sizeof( ci->attachTagNames[i] ) - 1 ] = 0;
				ci->dobjNeedsUpdate = 1;
			}
		}
	}

	CG_UpdateHandViewmodels( CG_ConfigString( CS_MODELS + snap->ps.viewmodelIndex ) );

	cg.unknown_0x2A798 = snap->ps.stats[3];
	cg.unknown_0x2A79C = snap->ps.stats[4];

	// if changing follow mode, don't interpolate
	if ( snap->ps.clientNum != cg.snap->ps.clientNum ) {
		cg_entities[ cg.snap->ps.clientNum ].currentValid = qfalse;
	}

	if ( snap->ps.pm_flags & ( PMF_FOLLOW | PMF_INWORLD ) ) {
		cent = &cg_entities[ snap->ps.clientNum ];
		BG_PlayerStateToEntityState( &snap->ps, &cent->nextState, qfalse );

		if ( cg.mapRestart
			 || snap->ps.stats[STAT_SPAWN_COUNT] != cg.snap->ps.stats[STAT_SPAWN_COUNT]
			 || snap->ps.clientNum != cg.snap->ps.clientNum ) {
			cg.snap->ps = snap->ps;
			CG_ResetEntity( cent );
			CG_Respawn();
		} else if ( !cent->currentValid
					|| ( ( cent->nextState.eFlags ^ cent->currentState.eFlags ) & EF_TELEPORT_BIT ) ) {
			cg.snap->ps = snap->ps;
			CG_ResetEntity( cent );
			// a teleport will not cause an error decay
			VectorClear( cg.predictedError );
		}
	} else {
		if ( cg.mapRestart
			 || snap->ps.stats[STAT_SPAWN_COUNT] != cg.snap->ps.stats[STAT_SPAWN_COUNT]
			 || snap->ps.clientNum != cg.snap->ps.clientNum ) {
			cg.snap->ps = snap->ps;
			CG_Respawn();
		}
	}

	// check for extrapolation errors
	for ( num = 0 ; num < snap->numEntities ; num++ ) {
		es = &snap->entities[num];
		cent = &cg_entities[ es->number ];

		cent->nextState = *es;

		// if this frame is a teleport, or the entity wasn't in the
		// previous frame, don't interpolate
		if ( !cent->currentValid
			 || ( ( es->eFlags ^ cent->currentState.eFlags ) & EF_TELEPORT_BIT ) ) {
			CG_ResetEntity( cent );
		}
	}

	for ( num = 0 ; num < snap->numClients ; num++ ) {
		cs = (const cgClientState_t *)&snap->clients[ num ];
		cent = &cg_entities[ cs->clientNum ];

		if ( cent->currentValid ) {
			es = &cent->nextState;
			BG_UpdatePlayerDObj( trap_syscall_0xA2( es->clientNum ),
								 es, &bg_clientinfo[ es->clientNum ] );
		}
	}

	// sort out solid entities
	CG_BuildSolidList();

	for ( num = 0 ; num < snap->numEntities ; num++ ) {
		CG_CheckPreEvents( &cg_entities[ snap->entities[num].number ] );
	}
}

/*
========================
CG_ReadNextSnapshot

This is the only place new snapshots are requested
This may increment cgs.processedSnapshotNum multiple
times if the client system fails to return a
valid snapshot.
========================
*/
static snapshot_t *CG_ReadNextSnapshot( void ) {
	qboolean r;
	snapshot_t  *dest;

	if ( cg.latestSnapshotNum > cgs.processedSnapshotNum + 1000 ) {
		CG_Printf( "WARNING: CG_ReadNextSnapshot: way out of range, %i > %i\n",
				   cg.latestSnapshotNum, cgs.processedSnapshotNum );
	}

	while ( cgs.processedSnapshotNum < cg.latestSnapshotNum ) {
		// decide which of the two slots to load it into
		if ( cg.snap == &cg.activeSnapshots[0] ) {
			dest = &cg.activeSnapshots[1];
		} else {
			dest = &cg.activeSnapshots[0];
		}

		// try to read the snapshot from the client system
		cgs.processedSnapshotNum++;
		r = trap_GetSnapshot( cgs.processedSnapshotNum, dest );

		// if it succeeded, return
		if ( r ) {
			CG_AddLagometerSnapshotInfo( dest );
			return dest;
		}

		// a GetSnapshot will return failure if the snapshot
		// never arrived, or is so old that its entities
		// have been shoved off the end of the circular
		// buffer in the client system.

		// record as a dropped packet
		CG_AddLagometerSnapshotInfo( NULL );

		// If there are additional snapshots, continue trying to read them.
	}

	// nothing left to read
	return NULL;
}

/*
============
CG_ProcessSnapshots

We are trying to set up a renderable view, so determine
what the simulated time is, and try to get snapshots
both before and after that time if available.
============
*/
void CG_ProcessSnapshots( void ) {
	snapshot_t      *snap;
	int n;

	// see what the latest snapshot the client system has is
	trap_GetCurrentSnapshotNumber( &n, &cg.latestSnapshotTime );
	if ( n != cg.latestSnapshotNum ) {
		if ( n < cg.latestSnapshotNum ) {
			// this should never happen
			CG_Error( "CG_ProcessSnapshots: snapshot went backwards (%i < %i), processed %i, time %i",
			          n, cg.latestSnapshotNum, cgs.processedSnapshotNum, cg.latestSnapshotTime );
		}
		cg.latestSnapshotNum = n;
	}

	bgs_animTime = cg.latestSnapshotTime;

	// If we have yet to receive a snapshot, check for it.
	// Once we have gotten the first snapshot, cg.snap will
	// always have valid data for the rest of the game
	while ( !cg.snap ) {
		snap = CG_ReadNextSnapshot();
		if ( !snap ) {
			// we can't continue until we get a snapshot
			return;
		}

		if ( !( snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
			CG_SetInitialSnapshot( snap );
			CG_SetNextSnap( snap );
			CG_TransitionSnapshot();
		}
	}

	CG_SetFrameInterpolation();

	// loop until we either have a valid nextSnap with a serverTime
	// greater than cg.time to interpolate towards, or we run
	// out of available snapshots
	do {
		// if we don't have a nextframe, try and read a new one in
		if ( cg.nextSnap == cg.snap ) {
			snap = CG_ReadNextSnapshot();

			// if we still don't have a nextframe, we will just have to
			// extrapolate
			if ( !snap ) {
				break;
			}

			// server has been restarted
			if ( ( snap->snapFlags ^ cg.snap->snapFlags ) & SNAPFLAG_SERVERCOUNT ) {
				CG_SetInitialSnapshot( snap );
				CG_SetNextSnap( snap );
				CG_TransitionSnapshot();
				continue;
			}

			// if time went backwards, we have a level restart
			if ( snap->serverTime < cg.snap->serverTime ) {
				CG_Error( "CG_ProcessSnapshots: Server time went backwards" );
			}

			CG_SetNextSnap( snap );
		}

		// if our time is < nextFrame's, we have a nice interpolating state
		if ( cg.time >= cg.snap->serverTime && cg.time < cg.nextSnap->serverTime ) {
			break;
		}

		// we have passed the transition from nextFrame to frame
		CG_TransitionSnapshot();
	} while ( 1 );

	if ( cg.time < cg.snap->serverTime ) {
		// this can happen right after a vid_restart
		cg.time = cg.snap->serverTime;
		bgs_time = cg.time;
	}
}
