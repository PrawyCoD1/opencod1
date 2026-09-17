/*
 * cg_playerstate.c -- this file acts on changes in a new playerState_t.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_playerstate.c is the ancestor; CG_CheckAmmo,
 * CG_DamageFeedback, CG_Respawn, CG_CheckPlayerstateEvents,
 * CG_CheckChangedPredictableEvents and CG_TransitionPlayerState are all still
 * recognisably its text.  What CoD dropped: the viewDamage_t slot array (the
 * damage splats became cg.damageIndicators, an eight-entry direction-indicator
 * ring), CG_CheckLocalSounds (absent -- no function and no call), and
 * everything CG_TransitionSnapshot now does instead (respawn/map-restart
 * detection moved into cg_snapshot.c).
 *
 * Functions are in address order, which is the original file order.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdlib.h>

#include "cg_local.h"

#define PITCH                       0
#define YAW                         1
#define ROLL                        2

/* Owned by other units. */
void        CG_Printf( const char *msg, ... );                  /* cg_main.c 0x300206F0 */
                                                                /* cg_main.c 0x30021BF0 */
void        CG_EntityEvent( centity_t *cent, int event, qboolean predicted );
                                                                /* cg_event.c 0x3001DC10 */

/* universal/q_shared.c, universal/com_math.c */
void        AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );

/* playerState_t::events[] is four deep, cg.predictableEvents[] sixteen. */
#define MAX_EVENTS                  4
#define MAX_PREDICTED_EVENTS        16

/* cg.damageIndicators is eight 12-byte records (CG_Respawn memsets 0x60). */
#define MAX_DAMAGE_INDICATORS       8

/* cg.v_dmg_time = cg.time + DAMAGE_TIME (0x30028A47 adds 0x1F4). */
#define DAMAGE_TIME                 500

#define ANGLE2SHORT( x )            ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )            ( (float)( x ) * ( 360.0f / 65536.0f ) )

/*
==============
CG_CheckAmmo

If the ammo has gone low enough to generate the warning, play a sound
==============
*/
void CG_CheckAmmo( void ) {
	int i;
	int total;
	int weapons[MAX_WEAPONS / ( sizeof( int ) * 8 )];

	// see about how many seconds of ammo we have remaining
	memcpy( weapons, cg.snap->ps.weapons, sizeof( weapons ) );

	if ( !weapons[0] && !weapons[1] ) {     // we start out with no weapons, so don't click on startup
		return;
	}

	total = 0;

	// weapon 0 is "none"
	for ( i = 1 ; i < bg_numWeapons ; i++ )
	{
		if ( !( weapons[0] & ( 1 << i ) ) ) {
			continue;
		}

		total += cg.snap->ps.ammo[ BG_AmmoForWeapon( i ) ] * 1000;

		if ( total >= 5000 ) {
			cg.lowAmmoWarning = 0;
			return;
		}
	}

	if ( !cg.lowAmmoWarning ) {
		// play a sound on this transition
		/* cgs.media.noAmmoSound holds the alias STRING trap_Com_SoundAliasString
		   returned (0x30020AE4), not a handle -- cg_public.h types the media
		   sound slots sfxHandle_t, hence the cast. */
		CG_PlaySoundAliasByName( (const char *)cgs.media.noAmmoSound,
								 cg.snap->ps.clientNum, cg.snap->ps.origin );
	}

	if ( total == 0 ) {
		cg.lowAmmoWarning = 2;
	} else {
		cg.lowAmmoWarning = 1;
	}
}

/*
==============
CG_DamageFeedback
==============
*/
void CG_DamageFeedback( int yawByte, int pitchByte, int damage ) {
	float left, front;
	float kick;
	vec3_t dir;
	vec3_t angles;
	float yaw, pitch;
	int i;
	int slot;

	// show the attacking player's head and name in corner
	cg.damageFeedbackTime = cg.time;

	kick = damage * 0.2f;

	if ( kick < 5 ) {
		kick = 5;
	}
	if ( kick > 90 ) {
		kick = 90;
	}

	// if yaw and pitch are both 255, make the damage always centered (falling, etc)
	if ( yawByte == 255 && pitchByte == 255 ) {
		/* cg_local.h spells these two damageX (+0x2A8A8) and damageY (+0x2A8A4);
		   they are RTCW's v_dmg_roll and v_dmg_pitch. */
		cg.damageX = 0;
		cg.damageY = -kick;
	} else {
		// positional
		pitch = pitchByte / 255.0f * 360;
		yaw = yawByte / 255.0f * 360;

		angles[PITCH] = pitch;
		angles[YAW] = yaw;
		angles[ROLL] = 0;

		AngleVectors( angles, dir, NULL, NULL );
		VectorSubtract( vec3_origin, dir, dir );

		front = DotProduct( dir, cg.refdef.viewaxis[0] );
		left = DotProduct( dir, cg.refdef.viewaxis[1] );

		cg.damageX = kick * left;
		cg.damageY = -kick * front;

		// replace the oldest direction indicator
		slot = 0;
		for ( i = 1 ; i < MAX_DAMAGE_INDICATORS ; i++ ) {
			if ( cg.damageIndicators[i][0] < cg.damageIndicators[slot][0] ) {
				slot = i;
			}
		}

		cg.damageIndicators[slot][0] = cg.snap->serverTime;
		cg.damageIndicators[slot][1] = cg_hudDamageIconTime.integer;
		/* the third dword is the icon yaw, a float; cg_local.h types the whole
		   record int. */
		*(float *)&cg.damageIndicators[slot][2] =
			SHORT2ANGLE( ANGLE2SHORT( ( rand() / 32768.0f - 0.5f ) * 20.0f + yaw ) );
	}

	cg.v_dmg_time = cg.time + DAMAGE_TIME;
	cg.damageTime = cg.snap->serverTime;
}

/*
================
CG_Respawn

A respawn happened this snapshot
================
*/
void CG_Respawn( void ) {
	// no error decay on player movement
	cg.mapRestart = qfalse;

	cg.predictedPlayerState = cg.snap->ps;

	// display weapons available
	cg.weaponSelectTime = cg.time;

	cg.weaponInfo = bg_weaponInfo[ cg.predictedPlayerState.weapon ];

	cg.cursorHintIcon = 0;
	cg.cursorHintTime = 0;

	*(int *)&cg.unknown_0x2AA40[0x5C] = 0;           /* cg+0x2AA9C */

	memset( cg.unknown_0x27498, 0, sizeof( cg.unknown_0x27498 ) );

	VectorClear( cg.kick_origin );
	VectorClear( cg.kick_angles );
	*(float *)&cg.unknown_0x2A8AC[4] = 0;            /* cg+0x2A8B0 */

	// the ADS / weapon-position lerp block
	memset( cg.unknown_0x27314, 0, sizeof( cg.unknown_0x27314 ) );

	cg.damageTime = 0;
	cg.damageY = 0;
	cg.damageX = 0;

	memset( &cg.unknown_0x2AA40[0x14], 0, 24 );      /* cg+0x2AA54..0x2AA6C */

	memset( cg.damageIndicators, 0, sizeof( cg.damageIndicators ) );
	memset( cg.cameraShakes, 0, sizeof( cg.cameraShakes ) );

	VectorClear( cg.predictedError );

	// select the weapon the server says we are using
	trap_Cvar_Set( "cg_weaponSelect", va( "%i", cg.snap->ps.weapon ) );
	trap_Cvar_Set( "cl_stance", "0" );
	trap_Cvar_Set( "cl_run", "1" );
}

/*
==============
CG_CheckPlayerstateEvents
==============
*/
void CG_CheckPlayerstateEvents( playerState_t *ps, playerState_t *ops ) {
	int i;
	int event;
	centity_t   *cent;

	cent = &cg.predictedPlayerEntity;

	// go through the predictable events buffer
	for ( i = ps->eventSequence - MAX_EVENTS ; i < ps->eventSequence ; i++ ) {
		// if we have a new predictable event
		if ( i >= ops->eventSequence
			 // or the server told us to play another event instead of a predicted event we already issued
			 // or something the server told us changed our prediction causing a different event
			 || ( i > ops->eventSequence - MAX_EVENTS && ps->events[i & ( MAX_EVENTS - 1 )] != ops->events[i & ( MAX_EVENTS - 1 )] ) ) {

			event = ps->events[ i & ( MAX_EVENTS - 1 ) ];
			cent->currentState.eventParm = ps->eventParms[ i & ( MAX_EVENTS - 1 ) ];
			CG_EntityEvent( cent, event, qtrue );

			cg.predictableEvents[ i & ( MAX_PREDICTED_EVENTS - 1 ) ] = event;

			cg.eventSequence++;
		}
	}
}

/*
==================
CG_CheckChangedPredictableEvents

Nothing in 1.1 calls this -- the linker kept it.
==================
*/
void CG_CheckChangedPredictableEvents( playerState_t *ps ) {
	int i;
	int event;
	centity_t   *cent;

	cent = &cg.predictedPlayerEntity;
	for ( i = ps->eventSequence - MAX_EVENTS ; i < ps->eventSequence ; i++ ) {
		//
		if ( i >= cg.eventSequence ) {
			continue;
		}
		// if this event is not further back in than the maximum predictable events we remember
		if ( i > cg.eventSequence - MAX_PREDICTED_EVENTS ) {
			// if the new playerstate event is different from a previously predicted one
			if ( ps->events[i & ( MAX_EVENTS - 1 )] != cg.predictableEvents[i & ( MAX_PREDICTED_EVENTS - 1 ) ] ) {

				event = ps->events[ i & ( MAX_EVENTS - 1 ) ];
				cent->currentState.eventParm = ps->eventParms[ i & ( MAX_EVENTS - 1 ) ];
				CG_EntityEvent( cent, event, qtrue );

				cg.predictableEvents[ i & ( MAX_PREDICTED_EVENTS - 1 ) ] = event;

				if ( cg_showmiss.integer ) {
					CG_Printf( "WARNING: changed predicted event\n" );
				}
			}
		}
	}
}

/*
===============
CG_TransitionPlayerState
===============
*/
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops ) {
	// damage events (player is getting wounded)
	if ( ps->damageEvent != ops->damageEvent && ps->damageCount ) {
		CG_DamageFeedback( ps->damageYaw, ps->damagePitch, ps->damageCount );
	}

	// check for going low on ammo
	CG_CheckAmmo();

	// run events
	CG_CheckPlayerstateEvents( ps, ops );
}
