/*
 * g_scr_mover_mp.c -- the script-driven mover.
 *
 * script_brushmodel / script_model / script_origin are CoD's own entities;
 * nothing in RTCW or Q3 corresponds to them.  A script mover is not a binary
 * mover: it has no pos1/pos2 state machine and no use function.  Script code
 * calls moveto/movex/rotateto/... on it and the entity walks a three-phase
 * trajectory -- TR_ACCELERATE, then TR_LINEAR_STOP, then TR_DECCELERATE --
 * re-armed a phase at a time by Reached_ScriptMover, which fires the
 * "movedone" / "rotatedone" notifies when the last phase ends.
 *
 * The phase state lives in gentity_t fields the binary mover uses for other
 * things.  Linear side: pos1 is where the accel phase ends, pos2 where the
 * decel phase starts, pos3 the destination; speed / wait / delay hold the
 * cruise speed, the linear-phase duration and the decel duration.  Angular
 * side is the same six roles in movedir / rotate / TargetAngles and
 * closespeed / angle / random.  That overloading is the original's:
 * ScriptMover_Move (0x20035DF0) and ScriptMover_Rotate (0x20035EC0) differ
 * only in which field of each pair they pass.
 *
 * Function order is binary order (0x200354A0 .. 0x200369E0).
 *
 * @fidelity: likely
 */

#include "g_local.h"

#include <math.h>
#include <string.h>

/* Declarations that belong in a shared header. */

#define PITCH   0
#define YAW     1
#define ROLL    2

/* Reached_ScriptMover 0x20035754..0x20035787: ANGLE2SHORT is one
   `fmul 182.044` (65536/360 folded) then ftol and `and eax, 0FFFFh`, so the
   yaw round-trips through an unsigned short. */
#define ANGLE2SHORT( x )    ( (int)( ( x ) * ( 65536 / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )    ( ( x ) * ( 360.0f / 65536 ) )

/* trajectory_t.trType.  0/1/2/3/4 are g_mover_mp.c's set; 5/9/10 are
   used by InitScriptMover's siblings below. */
#define TR_STATIONARY           0
#define TR_LINEAR_STOP          3
#define TR_GRAVITY              5
#define TR_ACCELERATE           9
#define TR_DECCELERATE          10

/* entityState_t.eType.  InitScriptMover 0x200360D0. */
#define ET_SCRIPTMOVER          8

/* entityShared_t.svFlags.  4 is only ever set by SP_script_model
   (0x2003613E); the name is inferred from United Offensive, which calls the
   same bit SVF_DOBJ_USE_MODEL_BOUNDS --
   consistent with it riding along with G_DObjUpdate, but nothing in this DLL
   reads it. */
#define SVF_NOCLIENT                1
#define SVF_DOBJ_USE_MODEL_BOUNDS   4
#define SVF_USE_CURRENT_ORIGIN      0x00000080

/* entityState_t.eFlags.  SP_script_origin 0x2003618A is the only site in the
   module; the name is inferred (United Offensive's EF_NODRAW is
   0x80 there, so the bit was renumbered or this is a different flag). */
#define EF_NODRAW               0x00000100

/* gentity_t.flags.  0x2000 is set here and by ClientSpawn (0x2001A174) and
   read nowhere in this DLL -- the linkto path outside the module consumes it;
   the name is inferred from United Offensive's. */
#define FL_TEAMSLAVE            0x00000004
#define FL_SUPPORTS_LINKTO      0x00002000

/* entityShared_t.contents.  0x2080 has no recovered name. */
#define CONTENTS_SOLID          0x00000001
#define SCRIPT_MODEL_CONTENTS   0x00002080

/* forward: q_math.c */
float   AngleSubtract( float a1, float a2 );
float   AngleNormalize180( float angle );

/*
 * The script method table's entry type.  g_scr_main_mp.c's Scr_GetMethod
 * (0x20034D00) chains Player_GetMethod -> ScriptEnt_GetMethod ->
 * HudElem_GetMethod -> BuiltIn_GetMethod, all with this shape.
 */
typedef void ( *xmethod_t )( int entnum );

typedef struct {
	const char  *name;
	xmethod_t   func;
} scr_method_t;


/*
================
ScriptMover_Updatemove

Advances one trajectory to its next phase.  Returns qtrue when the move is
finished, which is what makes Reached_ScriptMover fire the notify.

`current` is passed by every call site and read by none; the parameter slot
at [esp+20h] is never touched (0x200354A0).
================
*/
static qboolean ScriptMover_Updatemove( const vec3_t decelStart, trajectory_t *tr, const vec3_t accelEnd,
										vec3_t current, float speed, float linearTime, float decelTime,
										const vec3_t target ) {
	vec3_t delta;
	float f;

	if ( tr->trType == TR_ACCELERATE ) {
		if ( linearTime > 0 ) {
			tr->trTime = level.time;
			tr->trDuration = linearTime * 1000;
			VectorCopy( accelEnd, tr->trBase );
			VectorSubtract( decelStart, accelEnd, delta );
			tr->trType = TR_LINEAR_STOP;
			f = 1000.0f / tr->trDuration;
			VectorScale( delta, f, tr->trDelta );
			return qfalse;
		}
	} else if ( tr->trType != TR_LINEAR_STOP ) {
		goto stopped;
	}

	if ( decelTime > 0 ) {
		tr->trTime = level.time;
		tr->trDuration = decelTime * 1000;
		VectorCopy( decelStart, tr->trBase );
		VectorSubtract( target, decelStart, delta );
		VectorNormalize( delta );
		tr->trType = TR_DECCELERATE;
		VectorScale( delta, speed, tr->trDelta );
		return qfalse;
	}

stopped:
	// a gravity move has no destination to snap to; freeze it where it fell
	if ( tr->trType == TR_GRAVITY ) {
		BG_EvaluateTrajectory( tr, level.time, tr->trBase );
	} else {
		VectorCopy( target, tr->trBase );
	}

	tr->trTime = level.time;
	tr->trType = TR_STATIONARY;
	return qtrue;
}

/*
================
Reached_ScriptMover
================
*/
void Reached_ScriptMover( gentity_t *ent ) {
	qboolean done;

	if ( ent->s.pos.trType != TR_STATIONARY ) {
		if ( level.time >= ent->s.pos.trTime + ent->s.pos.trDuration ) {
			done = ScriptMover_Updatemove( ent->pos2, &ent->s.pos, ent->pos1, ent->r.currentOrigin,
										   ent->speed, ent->wait, ent->delay, ent->pos3 );
			BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
			trap_LinkEntity( ent );

			if ( done ) {
				Scr_NotifyNum( ent->s.number, 0, scr_const.movedone, 0 );
			}
		}
	}

	if ( ent->s.apos.trType != TR_STATIONARY ) {
		if ( level.time >= ent->s.apos.trTime + ent->s.apos.trDuration ) {
			done = ScriptMover_Updatemove( ent->rotate, &ent->s.apos, ent->movedir, ent->r.currentAngles,
										   ent->closespeed, ent->angle, ent->random, ent->TargetAngles );
			BG_EvaluateTrajectory( &ent->s.apos, level.time, ent->r.currentAngles );
			trap_LinkEntity( ent );

			if ( done ) {
				ent->r.currentAngles[PITCH] = AngleNormalize180( ent->r.currentAngles[PITCH] );
				ent->r.currentAngles[YAW] = SHORT2ANGLE( ANGLE2SHORT( ent->r.currentAngles[YAW] ) );
				ent->r.currentAngles[ROLL] = AngleNormalize180( ent->r.currentAngles[ROLL] );

				Scr_NotifyNum( ent->s.number, 0, scr_const.rotatedone, 0 );
			}
		}
	}
}

/*
================
ScriptMover_SetupMove

Arms the first phase of a move to `point` over `time` seconds, and records
what the later phases need in the caller's six fields.
================
*/
static void ScriptMover_SetupMove( trajectory_t *tr, const vec3_t point, float time, float accelTime,
								   float decelTime, vec3_t current, float *speed, float *linearTime,
								   float *decelTimeOut, vec3_t accelEnd, vec3_t decelStart, vec3_t target ) {
	vec3_t delta;
	vec3_t dir;
	vec3_t move;
	float dist;
	float f;

	/* the delta is taken against the OLD current position and the trajectory
	   snap below then moves it -- 0x200357CE..0x200357FA, the loads are all
	   ahead of the call */
	VectorSubtract( point, current, delta );

	if ( tr->trType != TR_STATIONARY ) {
		BG_EvaluateTrajectory( tr, level.time, current );
	}

	if ( accelTime == 0 && decelTime == 0 ) {
		tr->trTime = level.time;
		tr->trDuration = time * 1000;
		*linearTime = time;
		*decelTimeOut = 0;
		VectorCopy( point, target );
		VectorCopy( current, tr->trBase );
		f = 1000.0f / tr->trDuration;
		tr->trType = TR_LINEAR_STOP;
		VectorScale( delta, f, tr->trDelta );
		BG_EvaluateTrajectory( tr, level.time, current );
		return;
	}

	*linearTime = time - accelTime - decelTime;
	*decelTimeOut = decelTime;

	// cruise speed: the accel and decel phases each cover half their time's worth
	dist = VectorLength( delta );
	*speed = ( dist + dist ) / ( time + time - accelTime - decelTime );

	VectorNormalize2( delta, dir );
	VectorScale( dir, *speed, dir );

	if ( accelTime != 0 ) {
		tr->trTime = level.time;
		tr->trDuration = accelTime * 1000;
		VectorCopy( current, tr->trBase );
		VectorCopy( dir, tr->trDelta );
		tr->trType = TR_ACCELERATE;
		BG_EvaluateTrajectory( tr, level.time + tr->trDuration, accelEnd );
	} else {
		VectorCopy( current, accelEnd );

		if ( *linearTime != 0 ) {
			tr->trTime = level.time;
			tr->trDuration = *linearTime * 1000;
			VectorCopy( current, tr->trBase );
			VectorScale( dir, *linearTime, move );
			tr->trType = TR_LINEAR_STOP;
			f = 1000.0f / tr->trDuration;
			VectorScale( move, f, tr->trDelta );
		} else {
			tr->trTime = level.time;
			tr->trDuration = *decelTimeOut * 1000;
			VectorCopy( current, tr->trBase );
			VectorCopy( dir, tr->trDelta );
			tr->trType = TR_DECCELERATE;
		}
	}

	VectorMA( accelEnd, *linearTime, dir, decelStart );
	VectorCopy( point, target );

	BG_EvaluateTrajectory( tr, level.time, current );
}

/*
================
ScriptMover_SetupMoveSpeed

Same three phases, but driven by a velocity instead of a destination, so the
destination has to be integrated out of the trajectories instead.
================
*/
static void ScriptMover_SetupMoveSpeed( trajectory_t *tr, const vec3_t velocity, float time, float accelTime,
										float decelTime, vec3_t current, float *speed, float *linearTime,
										float *decelTimeOut, vec3_t accelEnd, vec3_t decelStart, vec3_t target ) {
	trajectory_t decel;

	if ( tr->trType != TR_STATIONARY ) {
		BG_EvaluateTrajectory( tr, level.time, current );
	}

	if ( accelTime == 0 && decelTime == 0 ) {
		tr->trTime = level.time;
		tr->trDuration = time * 1000;
		*linearTime = time;
		*decelTimeOut = 0;
		VectorCopy( current, tr->trBase );
		VectorCopy( velocity, tr->trDelta );
		tr->trType = TR_LINEAR_STOP;
		BG_EvaluateTrajectory( tr, level.time, current );
		BG_EvaluateTrajectory( tr, level.time + tr->trDuration, target );
		return;
	}

	*linearTime = time - accelTime - decelTime;
	*decelTimeOut = decelTime;
	*speed = VectorLength( velocity );

	if ( accelTime != 0 ) {
		tr->trTime = level.time;
		tr->trDuration = accelTime * 1000;
		VectorCopy( current, tr->trBase );
		VectorCopy( velocity, tr->trDelta );
		tr->trType = TR_ACCELERATE;
		BG_EvaluateTrajectory( tr, level.time + tr->trDuration, accelEnd );
	} else {
		VectorCopy( current, accelEnd );

		if ( *linearTime != 0 ) {
			tr->trTime = level.time;
			tr->trDuration = *linearTime * 1000;
			VectorCopy( current, tr->trBase );
			VectorCopy( velocity, tr->trDelta );
			tr->trType = TR_LINEAR_STOP;
		} else {
			tr->trTime = level.time;
			tr->trDuration = *decelTimeOut * 1000;
			VectorCopy( current, tr->trBase );
			VectorCopy( velocity, tr->trDelta );
			tr->trType = TR_DECCELERATE;
		}
	}

	VectorMA( accelEnd, *linearTime, velocity, decelStart );

	if ( *decelTimeOut == 0 ) {
		VectorCopy( decelStart, target );
	} else {
		decel.trType = TR_DECCELERATE;
		decel.trTime = level.time;
		decel.trDuration = *decelTimeOut * 1000;
		VectorCopy( decelStart, decel.trBase );
		VectorCopy( velocity, decel.trDelta );
		BG_EvaluateTrajectory( &decel, level.time + decel.trDuration, target );
	}

	BG_EvaluateTrajectory( tr, level.time, current );
}

/*
================
ScriptMover_Move
================
*/
static void ScriptMover_Move( gentity_t *ent, const vec3_t point, float time, float accelTime, float decelTime ) {
	ScriptMover_SetupMove( &ent->s.pos, point, time, accelTime, decelTime, ent->r.currentOrigin,
						   &ent->speed, &ent->wait, &ent->delay, ent->pos1, ent->pos2, ent->pos3 );
	trap_LinkEntity( ent );
}

/*
================
ScriptMover_GravityMove
================
*/
static void ScriptMover_GravityMove( gentity_t *ent, const vec3_t velocity, float time ) {
	ent->s.pos.trTime = level.time;
	ent->s.pos.trDuration = time * 1000;
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	VectorCopy( velocity, ent->s.pos.trDelta );
	ent->s.pos.trType = TR_GRAVITY;

	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
	trap_LinkEntity( ent );
}

/*
================
ScriptMover_Rotate
================
*/
static void ScriptMover_Rotate( gentity_t *ent, const vec3_t angles, float time, float accelTime, float decelTime ) {
	ScriptMover_SetupMove( &ent->s.apos, angles, time, accelTime, decelTime, ent->r.currentAngles,
						   &ent->closespeed, &ent->angle, &ent->random, ent->movedir, ent->rotate,
						   ent->TargetAngles );
	trap_LinkEntity( ent );
}

/*
================
ScriptMover_RotateSpeed
================
*/
static void ScriptMover_RotateSpeed( gentity_t *ent, const vec3_t velocity, float time, float accelTime,
									 float decelTime ) {
	ScriptMover_SetupMoveSpeed( &ent->s.apos, velocity, time, accelTime, decelTime, ent->r.currentAngles,
								&ent->closespeed, &ent->angle, &ent->random, ent->movedir, ent->rotate,
								ent->TargetAngles );
	trap_LinkEntity( ent );
}

/*
================
InitScriptMover
================
*/
void InitScriptMover( gentity_t *ent ) {
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

	ent->reached = Reached_ScriptMover;

	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_SCRIPTMOVER;

	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;

	VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );
	ent->s.apos.trType = TR_STATIONARY;

	ent->flags |= FL_SUPPORTS_LINKTO;
}

/*
================
SP_script_brushmodel
================
*/
void SP_script_brushmodel( gentity_t *ent ) {
	trap_SetBrushModel( ent );
	InitScriptMover( ent );
	ent->r.contents = CONTENTS_SOLID;
	trap_LinkEntity( ent );
}

/*
================
SP_script_model
================
*/
void SP_script_model( gentity_t *ent ) {
	G_DObjUpdate( ent );
	InitScriptMover( ent );
	ent->r.svFlags |= SVF_DOBJ_USE_MODEL_BOUNDS;
	ent->r.contents = SCRIPT_MODEL_CONTENTS;
	trap_LinkEntity( ent );
}

/*
================
SP_script_origin
================
*/
void SP_script_origin( gentity_t *ent ) {
	InitScriptMover( ent );
	ent->r.contents = 0;
	trap_LinkEntity( ent );

	// an origin with a light on it still has to reach the client
	if ( ent->s.constantLight ) {
		ent->s.eFlags |= EF_NODRAW;
	} else {
		ent->r.svFlags |= SVF_NOCLIENT;
	}
}

/*
================
ScriptEnt_GetScriptMover

The entity-number check and the classname check every method below starts
with.  LTCG inlined it into all eight call sites, so it has no address of its
own; the name is inferred.
================
*/
static gentity_t *ScriptEnt_GetScriptMover( int entnum ) {
	gentity_t   *ent;

	if ( (unsigned int)entnum >= MAX_GENTITIES ) {
		Scr_Error( va( "%i is not a valid entity number", entnum ) );
		return NULL;
	}

	ent = &g_entities[entnum];

	if ( ent->classname != scr_const.script_brushmodel
		 && ent->classname != scr_const.script_model
		 && ent->classname != scr_const.script_origin ) {
		Scr_Error( va( "entity %i is not a script_brushmodel, script_model, or script_origin", entnum ) );
	}

	return ent;
}

/*
================
ScriptEntCmdGetCommandTimes

Parameter 1 is the total time, 2 the accel time and 3 the decel time; the
last two are optional and default to zero.
================
*/
static void ScriptEntCmdGetCommandTimes( float *totalTime, float *accelTime, float *decelTime ) {
	int numParam;

	*totalTime = Scr_GetFloat( 1 );
	if ( *totalTime <= 0 ) {
		Scr_ParamError( 1, "total time must be positive" );
	}

	numParam = Scr_GetNumParam();
	if ( numParam < 3 ) {
		*accelTime = 0;
		*decelTime = 0;
	} else {
		*accelTime = Scr_GetFloat( 2 );
		if ( *accelTime < 0 ) {
			Scr_ParamError( 2, "accel time must be nonnegative" );
		}

		if ( numParam < 4 ) {
			*decelTime = 0;
		} else {
			*decelTime = Scr_GetFloat( 3 );
			if ( *decelTime < 0 ) {
				Scr_ParamError( 3, "decel time must be nonnegative" );
			}
		}
	}

	if ( *accelTime + *decelTime > *totalTime ) {
		Scr_Error( "accel time plus decel time is greater than total time" );
	}
}

/*
================
ScriptEntCmd_MoveTo
================
*/
void ScriptEntCmd_MoveTo( int entnum ) {
	gentity_t   *ent;
	vec3_t point;
	float time, accelTime, decelTime;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	Scr_GetVector( 0, point );
	ScriptEntCmdGetCommandTimes( &time, &accelTime, &decelTime );

	ScriptMover_Move( ent, point, time, accelTime, decelTime );
}

/*
================
ScriptEntCmd_GravityMove
================
*/
void ScriptEntCmd_GravityMove( int entnum ) {
	gentity_t   *ent;
	vec3_t velocity;
	float time;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	Scr_GetVector( 0, velocity );
	time = Scr_GetFloat( 1 );

	ScriptMover_GravityMove( ent, velocity, time );
}

/*
================
ScriptEnt_MoveAxis
================
*/
static void ScriptEnt_MoveAxis( int entnum, int axis ) {
	gentity_t   *ent;
	vec3_t point;
	float dist;
	float time, accelTime, decelTime;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	dist = Scr_GetFloat( 0 );
	ScriptEntCmdGetCommandTimes( &time, &accelTime, &decelTime );

	VectorCopy( ent->r.currentOrigin, point );
	point[axis] += dist;

	ScriptMover_Move( ent, point, time, accelTime, decelTime );
}

/*
================
ScriptEntCmd_MoveX
================
*/
void ScriptEntCmd_MoveX( int entnum ) {
	ScriptEnt_MoveAxis( entnum, 0 );
}

/*
================
ScriptEntCmd_MoveY
================
*/
void ScriptEntCmd_MoveY( int entnum ) {
	ScriptEnt_MoveAxis( entnum, 1 );
}

/*
================
ScriptEntCmd_MoveZ
================
*/
void ScriptEntCmd_MoveZ( int entnum ) {
	ScriptEnt_MoveAxis( entnum, 2 );
}

/*
================
ScriptEntCmd_RotateTo
================
*/
void ScriptEntCmd_RotateTo( int entnum ) {
	gentity_t   *ent;
	vec3_t angles;
	vec3_t dest;
	float time, accelTime, decelTime;
	int i;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	Scr_GetVector( 0, angles );
	ScriptEntCmdGetCommandTimes( &time, &accelTime, &decelTime );

	// take the short way round on each axis
	for ( i = 0; i < 3; i++ ) {
		dest[i] = AngleSubtract( angles[i], ent->r.currentAngles[i] ) + ent->r.currentAngles[i];
	}

	ScriptMover_SetupMove( &ent->s.apos, dest, time, accelTime, decelTime, ent->r.currentAngles,
						   &ent->closespeed, &ent->angle, &ent->random, ent->movedir, ent->rotate,
						   ent->TargetAngles );
	trap_LinkEntity( ent );
}

/*
================
ScriptEnt_RotateAxis
================
*/
static void ScriptEnt_RotateAxis( int entnum, int axis ) {
	gentity_t   *ent;
	vec3_t angles;
	float degrees;
	float time, accelTime, decelTime;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	degrees = Scr_GetFloat( 0 );
	ScriptEntCmdGetCommandTimes( &time, &accelTime, &decelTime );

	VectorCopy( ent->r.currentAngles, angles );
	angles[axis] += degrees;

	ScriptMover_Rotate( ent, angles, time, accelTime, decelTime );
}

/*
================
ScriptEntCmd_RotatePitch
================
*/
void ScriptEntCmd_RotatePitch( int entnum ) {
	ScriptEnt_RotateAxis( entnum, PITCH );
}

/*
================
ScriptEntCmd_RotateYaw
================
*/
void ScriptEntCmd_RotateYaw( int entnum ) {
	ScriptEnt_RotateAxis( entnum, YAW );
}

/*
================
ScriptEntCmd_RotateRoll
================
*/
void ScriptEntCmd_RotateRoll( int entnum ) {
	ScriptEnt_RotateAxis( entnum, ROLL );
}

/*
================
ScriptEntCmd_RotateVelocity
================
*/
void ScriptEntCmd_RotateVelocity( int entnum ) {
	gentity_t   *ent;
	vec3_t velocity;
	float time, accelTime, decelTime;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->flags & FL_TEAMSLAVE ) {
		return;
	}

	Scr_GetVector( 0, velocity );
	ScriptEntCmdGetCommandTimes( &time, &accelTime, &decelTime );

	ScriptMover_RotateSpeed( ent, velocity, time, accelTime, decelTime );
}

/*
================
ScriptEntCmd_Solid
================
*/
void ScriptEntCmd_Solid( int entnum ) {
	gentity_t   *ent;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->classname == scr_const.script_origin ) {
		G_DPrintf( "cannot use the solid/notsolid commands on a script_origin entity" );
	} else if ( ent->classname == scr_const.script_model ) {
		G_DPrintf( "cannot use the solid/notsolid commands on a script_model entity" );
	} else {
		ent->r.contents = CONTENTS_SOLID;
	}
}

/*
================
ScriptEntCmd_NotSolid
================
*/
void ScriptEntCmd_NotSolid( int entnum ) {
	gentity_t   *ent;

	ent = ScriptEnt_GetScriptMover( entnum );

	if ( ent->classname == scr_const.script_origin ) {
		G_DPrintf( "cannot use the solid/notsolid commands on a script_origin entity" );
	} else if ( ent->classname == scr_const.script_model ) {
		G_DPrintf( "cannot use the solid/notsolid commands on a script_model entity" );
	} else {
		ent->r.contents = 0;
	}
}

static const scr_method_t scriptEntMethods[] = {      /* 0x20055860 */
	{ "moveto",         ScriptEntCmd_MoveTo         },
	{ "movex",          ScriptEntCmd_MoveX          },
	{ "movey",          ScriptEntCmd_MoveY          },
	{ "movez",          ScriptEntCmd_MoveZ          },
	{ "movegravity",    ScriptEntCmd_GravityMove    },
	{ "rotateto",       ScriptEntCmd_RotateTo       },
	{ "rotatepitch",    ScriptEntCmd_RotatePitch    },
	{ "rotateyaw",      ScriptEntCmd_RotateYaw      },
	{ "rotateroll",     ScriptEntCmd_RotateRoll     },
	{ "rotatevelocity", ScriptEntCmd_RotateVelocity },
	{ "solid",          ScriptEntCmd_Solid          },
	{ "notsolid",       ScriptEntCmd_NotSolid       }
};

/*
================
ScriptEnt_GetMethod

pDeveloper is never touched -- none of these methods is developer-only, and
Scr_GetMethod has already cleared it before entering the chain.
================
*/
xmethod_t ScriptEnt_GetMethod( const char **pName, int *pDeveloper ) {
	unsigned int i;

	for ( i = 0; i < sizeof( scriptEntMethods ) / sizeof( scriptEntMethods[0] ); i++ ) {
		if ( !strcmp( *pName, scriptEntMethods[i].name ) ) {
			*pName = scriptEntMethods[i].name;
			return scriptEntMethods[i].func;
		}
	}

	return NULL;
}
