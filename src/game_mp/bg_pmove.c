/*
 * bg_pmove.c -- both games player movement code.
 *
 * Call of Duty 1.1 multiplayer (game_mp_x86.dll, 0x20006FF0-0x2000C7F3).  RTCW's
 * game/bg_pmove.c is the direct ancestor and most of what follows is its text;
 * CoD's changes are local and named where they appear -- prone, lean, the
 * viewheight lerp tables, foliage, ladders and the per-playerState speed scales
 * that replaced RTCW's flat pm_duckScale/pm_runScale pair.
 *
 * The movement constants below are the retail values read out of the .rdata
 * block at 0x20055D28; they are NOT RTCW's (pm_friction is 5.5, not 6, and
 * pm_accelerate is 9, not 10).  Declaration order is the order MSVC emitted
 * them in.
 *
 * @fidelity: likely
 */

#include "bg_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Every symbol below is a real function or global in this DLL; no header this
 * file includes declares it. */
float VectorNormalize2( const vec3_t v, vec3_t out );           /* 0x20013A60 */
float VectorNormalize2D( vec3_t v );                            /* 0x200138C0 */
float vectoyaw( const vec3_t vec );                             /* 0x20013EF0 */
void vectoangles( const vec3_t value1, vec3_t angles );         /* 0x20014110 */
float AngleNormalize360( float angle );                         /* 0x20015F40 */
float AngleNormalize180( float angle );                         /* 0x20015F70 */
float PitchOfVectorAlongYaw( float yaw, const vec3_t normal );    /* 0x20017060 */
void ProjectPointOntoVector( const vec3_t vDir, vec3_t vProj, const vec3_t point ); /* 0x20016830 */
float UnGetLeanFraction( float f );                             /* 0x2003F220 */
void AddLeanToPosition( vec3_t pos, float yaw, float leanFrac, float leanDist, float scale ); /* 0x2003F240 */
void Com_Printf( const char *fmt, ... );                        /* 0x200262E0 */
void trap_SnapVector( float *v );
void trap_AddDebugLine( const vec3_t start, const vec3_t end, const vec3_t color,
						int depthTest, int duration );

/*
 * The prone view clamp draws in both builds; only the spelling differs, so the
 * split is a conditional on the symbol.  CG_DebugArc is cg_drawtools.c
 * 0x3001A3B0 and PM_UpdateViewAngles calls it at 0x3000B743; g_debugProneCheck
 * is read at 0x3000A2EF (PM_CheckDuck) and 0x3000B5EF.
 *
 * Seven parameters: five stack dwords (`add esp,14h` at 0x3000B748) plus
 * org@eax and duration@ebx; CG_DebugArc's own trap call reads depthTest from
 * arg_10 and duration from ebx (0x3001A4A9).
 */
#ifdef CGAMEDLL
#define G_DebugArc          CG_DebugArc
#define g_debugProneCheck   cg_debugProneCheck
#endif

void G_DebugArc( const vec3_t origin, float radius, float startAngle, float endAngle,
				 const vec3_t color, int depthTest, int duration );    /* 0x200212C0 */

/* BG_AnimScriptAnimation 0x20003020, BG_AnimScriptEvent 0x200031F0 -- both
 * declared in bg_public.h. */
void BG_UpdateConditionValue( int client, int condition, int value, qboolean checkConversion ); /* 0x20003290 */
void BG_AnimUpdatePlayerStateConditions( pmove_t *pmove );      /* 0x200033E0 */
int BG_CheckProneValid( int clientNum, const vec3_t origin, float radius, float height,
						float yaw, float *groundOffset, float *pitchDown, float *pitchUp,
						qboolean skipInitialTrace, qboolean allowFallback,
						const vec3_t groundNormal,
						void ( *traceFunc )( trace_t *, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int ),
						void ( *traceDownFunc )( trace_t *, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int ),
						qboolean useAltContentMask );           /* 0x20006010 */

/* universal/q_shared.c */
int Q_ftol( float f );

/* g_main_mp.c cvar table */
extern vmCvar_t bg_viewheight_standing;
extern vmCvar_t bg_duck2prone_time;
extern vmCvar_t bg_prone2duck_time;
extern vmCvar_t bg_ladder_yawcap;
extern vmCvar_t bg_prone_yawcap;
extern vmCvar_t bg_prone_softyawedge;
extern vmCvar_t bg_foliagesnd_minspeed;
extern vmCvar_t bg_foliagesnd_maxspeed;
extern vmCvar_t bg_foliagesnd_slowinterval;
extern vmCvar_t bg_foliagesnd_fastinterval;
extern vmCvar_t bg_foliagesnd_resetinterval;
extern vmCvar_t bg_fallDamageMinHeight;
extern vmCvar_t bg_fallDamageMaxHeight;
extern vmCvar_t g_debugProneCheck;

/* Constants that belong in a header. */
#define PITCH   0
#define YAW     1
#define ROLL    2

#define MAX_CLIENTS         64
#define ENTITYNUM_NONE      1023
#define ENTITYNUM_WORLD     1022

#define SHORT2ANGLE( x )    ( ( x ) * ( 360.0f / 65536 ) )
#define ANGLE2SHORT( x )    ( (int)( ( x ) * ( 65536 / 360.0f ) ) & 65535 )

/* Q3/RTCW surfaceflags, unchanged in CoD; the surface type is the 5-bit field
 * at bit 20 (universal/surfaceflags.c). */
#define SURF_NODAMAGE       0x1
#define SURF_SLICK          0x2
#define SURF_LADDER         0x8
#define SURF_NOSTEPS        0x2000
#define SURF_TYPE( flags )  ( ( ( flags ) >> 20 ) & 0x1F )

#define CONTENTS_WATER      0x20
#define CONTENTS_BODY       0x2000000
#define MASK_PLAYERSOLID    0x2810191

/* The pm_flags names live in bg_local.h -- one spelling per bit, shared with
   bg_weapon.c. */

/* eFlags bits this unit touches. */
#define EF_FIRING           0x400
#define EF_CROUCHING        0x20
#define EF_PRONE            0x40
#define EF_UPDATE_BOUNDS    0x10
#define EF_TALK             0x40000
#define EF_TURRET_ACTIVE_PRONE      0x4000
#define EF_TURRET_ACTIVE_DUCK       0x8000
#define EF_TURRET_ACTIVE_MASK       ( EF_TURRET_ACTIVE_PRONE | EF_TURRET_ACTIVE_DUCK )

/* usercmd_t.buttons / .wbuttons bits used here. */
#define BUTTON_ATTACK       0x1
#define BUTTON_TALK         0x2
#define BUTTON_ADS          0x10
#define WBUTTON_STUNNED     0x4     /* PM_CmdScale_Walk 0x20007628 */
#define WBUTTON_LEAN_LEFT   0x10
#define WBUTTON_LEAN_RIGHT  0x20
#define WBUTTON_PRONE       0x40
#define WBUTTON_CROUCH      0x80
#define WBUTTON_NOSTANCEEVENT   0x2 /* held-stance repeat; suppresses the blocked-stance event */

/* Events this unit raises.  Only the numbers are recovered. */
#define EV_FOOTSTEP             1   /* + surface type */
#define EV_SWIM                 21
#define EV_FOOTSTEP_WALKING     24  /* + surface type */
#define EV_FOOTSTEP_WADE        44
#define EV_FOOTSTEP_PRONE       47  /* + surface type */
#define EV_SWIM_PRONE           67
#define EV_JUMP                 70  /* + surface type */
#define EV_JUMP_LADDER          83
#define EV_LANDING_HARD         93  /* + surface type */
#define EV_LANDING_DAMAGE       116 /* + surface type */
#define EV_FOLIAGE_SOUND        139
#define EV_STANCE_FORCE_STAND   140
#define EV_STANCE_FORCE_CROUCH  141
#define EV_STANCE_FORCE_PRONE   142
#define EV_STEP                 143
#define EV_WATER_TOUCH          144
#define EV_WATER_LEAVE          145

#define MIN_WALK_NORMAL     0.7f    /* can't walk on very steep slopes */
#define STEPSIZE            18
#define OVERCLIP            1.001f

/*
 * ---- file-local state ----------------------------------------------------
 */
pmove_t     *pm;                /* 0x2014ECD0 */
pml_t pml;                      /* 0x2014EB20 */

/* movement parameters -- retail values, .rdata 0x20055D28..0x20055D68 */
const float pm_stopspeed        = 100;
const float pm_ladderScale      = 0.5f;
const float pm_ladderPushOff    = 128;
const int pm_ladderJumpTime     = 300;
const float pm_waterSwimScale   = 0.5f;
const float pm_waterWadeScale   = 0.7f;
const float pm_prone_accelerate = 19;
const float pm_ducked_accelerate = 12;
const float pm_accelerate       = 9;
const float pm_airaccelerate    = 1;
const float pm_wateraccelerate  = 4;
const float pm_flyaccelerate    = 8;
const float pm_friction         = 5.5f;
const float pm_waterfriction    = 1;
const float pm_ladderfriction   = 16;
const float pm_spectatorfriction = 5.0f;
const float pm_shellshockScale  = 0.4f;

/* The 26 unit-cube neighbours PM_CorrectAllSolid jitters through, .rdata
 * 0x20055D70.  RTCW built them with three nested -1..1 loops; CoD tabled them
 * and dropped the {0,0,0} centre.  The name is inferred (United Offensive's);
 * it is not a recovered symbol. */
const vec3_t pm_correctSolidOffsets[26] = {
	{  0,  0,  1 }, { -1,  0,  1 }, {  0, -1,  1 }, {  1,  0,  1 },
	{  0,  1,  1 }, { -1,  0,  0 }, {  0, -1,  0 }, {  1,  0,  0 },
	{  0,  1,  0 }, {  0,  0, -1 }, { -1,  0, -1 }, {  0, -1, -1 },
	{  1,  0, -1 }, {  0,  1, -1 }, { -1, -1,  1 }, {  1, -1,  1 },
	{  1,  1,  1 }, { -1,  1,  1 }, { -1, -1,  0 }, {  1, -1,  0 },
	{  1,  1,  0 }, { -1,  1,  0 }, { -1, -1, -1 }, {  1, -1, -1 },
	{  1,  1, -1 }, { -1,  1, -1 }
};

int c_pmove = 0;                /* 0x200A06A0 */

/*
 * One row of a viewheight interpolation table.  File-local: nothing outside
 * this unit reaches the tables.
 */
typedef struct pmLerpEntry_t
{
	int percent;
	float height;
	int originAdjust;
} pmLerpEntry_t;

#define PM_LERP_TABLE_END   -1

static float PM_GetViewHeightLerp( int fromViewheight, int toViewheight );

/*
===============
PM_AddEvent
===============
*/
void PM_AddEvent( int newEvent ) {
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( const vec3_t in, const vec3_t normal, vec3_t out, float overbounce ) {
	float backoff;
	float change;
	int i;

	backoff = DotProduct( in, normal );

	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}
}

/* xoxor4d's slope projection, used only at the bounce hook's step-down site.
 * Preserve input when projection is rejected, including non-aliased outputs. */
void PM_ProjectVelocity( const vec3_t in, const vec3_t normal, vec3_t out ) {
	float speedXY = in[0] * in[0] + in[1] * in[1];
	float projectedZ, scale;
	if ( normal[2] < 0.001f || speedXY == 0.0f ) {
		VectorCopy( in, out );
		return;
	}
	projectedZ = -(normal[0] * in[0] + normal[1] * in[1]) / normal[2];
	scale = sqrtf( (in[2] * in[2] + speedXY) / (speedXY + projectedZ * projectedZ) );
	if ( scale < 1.0f || projectedZ < 0.0f || in[2] > 0.0f ) {
		out[0] = scale * in[0];
		out[1] = scale * in[1];
		out[2] = scale * projectedZ;
	} else {
		VectorCopy( in, out );
	}
}

void PM_Bounce( const vec3_t in, const vec3_t normal, vec3_t out ) {
#ifdef CGAMEDLL
	extern const char *CG_ConfigString( int index );
	/* Read the server's value, not a client-editable or stale local cvar. */
	int enabled = atoi( Info_ValueForKey( CG_ConfigString( 1 ), "x_cl_bounce" ) );
#else
	extern vmCvar_t g_bounce;
	int enabled = g_bounce.integer;
#endif
	if ( enabled ) {
		PM_ProjectVelocity( in, normal, out );
	} else {
		PM_ClipVelocity( in, normal, out, OVERCLIP );
	}
}

/*
==============
PM_GetEffectiveStance

0 standing, 1 prone, 2 crouched.  A prone->crouch lerp that has not started
coming down still counts as prone.
==============
*/
int PM_GetEffectiveStance( const playerState_t *ps ) {
	if ( ps->pm_flags & PMF_PRONE ) {
		return 1;
	}
	if ( ps->viewHeightLerpTarget == ps->proneViewHeight ) {
		return 1;
	}
	if ( ps->viewHeightLerpTime && ps->viewHeightLerpTarget == ps->crouchViewHeight
		 && !ps->viewHeightLerpDown ) {
		return 1;
	}
	if ( ps->pm_flags & PMF_DUCKED ) {
		return 2;
	}
	if ( ps->viewHeightLerpTime && ps->viewHeightLerpTarget == ps->crouchViewHeight ) {
		return 2;
	}
	return 0;
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t vec;
	float   *vel;
	float speed, newspeed, control;
	float drop;

	vel = pm->ps->velocity;

	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0; // ignore slope movement
	}

	speed = VectorLength( vec );
	if ( speed < 1 ) {
		vel[0] = 0;
		vel[1] = 0;     // allow sinking underwater
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( pml.walking && !( pml.groundTrace.surfaceFlags & SURF_SLICK ) ) {
			// if getting knocked back, no friction
			if ( !( pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) ) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				if ( pm->ps->pm_flags & PMF_TIME_LAND ) {
					control = control * 0.3f;
				} else if ( pm->ps->pm_flags & PMF_TIME_HARDLAND ) {
					control = control + control;
				}
				drop += control * pm_friction * pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		drop += speed * pm_spectatorfriction * pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if ( newspeed < 0 ) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( const vec3_t wishdir, float wishspeed, float accel ) {
	float addspeed, accelspeed, currentspeed, control;

	currentspeed = DotProduct( pm->ps->velocity, wishdir );
	addspeed = wishspeed - currentspeed;
	if ( addspeed <= 0 ) {
		return;
	}
	control = wishspeed < pm_stopspeed ? pm_stopspeed : wishspeed;
	accelspeed = control * pml.frametime * accel;
	if ( accelspeed > addspeed ) {
		accelspeed = addspeed;
	}
	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
		accelspeed *= 1.0f / pm->ps->friction;
	}
	if ( accelspeed > addspeed ) {
		accelspeed = addspeed;
	}

	pm->ps->velocity[0] += accelspeed * wishdir[0];
	pm->ps->velocity[1] += accelspeed * wishdir[1];
	pm->ps->velocity[2] += accelspeed * wishdir[2];
}

/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( const usercmd_t *cmd ) {
	int max;
	float total;
	float scale;

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( cmd->upmove ) > max ) {
		max = abs( cmd->upmove );
	}
	if ( !max ) {
		return 0;
	}

	total = (float)sqrt( (double)( cmd->forwardmove * cmd->forwardmove
								   + cmd->rightmove * cmd->rightmove
								   + cmd->upmove * cmd->upmove ) );
	scale = (float)pm->ps->speed * max / ( total * 127.0f );

	if ( pm->ps->pm_flags & PMF_WALKING ) {
		scale *= pm->ps->walkSpeedScale;
	} else {
		scale *= pm->ps->runSpeedScale;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		scale *= 3;
	}
	if ( pm->ps->pm_type == PM_UFO ) {
		scale *= 6;
	}

	return scale;
}

/*
============
PM_CmdScale_Walk

The walking counterpart of PM_CmdScale: no upmove, and the per-axis, per-stance
and per-weapon scales out of the playerState replace RTCW's flat pm_duckScale.
============
*/
static float PM_CmdScale_Walk( const usercmd_t *cmd ) {
	int fmove, smove;
	float forwardTerm, strafeTerm, moveMag;
	float scale;
	float lerp;
	int effectiveStance;
	const weaponInfo_t *weaponInfo;

	fmove = cmd->forwardmove;
	if ( fmove < 0 ) {
		forwardTerm = fmove * pm->ps->backSpeedScale;
	} else {
		forwardTerm = (float)fmove;
	}
	smove = cmd->rightmove;
	forwardTerm = PM_FloatAbs( forwardTerm );

	strafeTerm = PM_FloatAbs( smove * pm->ps->strafeSpeedScale );

	moveMag = forwardTerm;
	if ( moveMag <= strafeTerm ) {
		moveMag = strafeTerm;
	}
	if ( moveMag == 0 ) {
		return 0;
	}

	scale = (float)pm->ps->speed * moveMag
			/ ( (float)sqrt( (double)( fmove * fmove + smove * smove ) ) * 127.0f );

	if ( pm->ps->pm_flags & PMF_WALKING ) {
		scale *= pm->ps->walkSpeedScale;
	} else {
		scale *= pm->ps->runSpeedScale;
		if ( pm->ps->leanf != 0 ) {
			scale *= pm->ps->leanSpeedScale;
		}
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		scale *= 3;
	} else if ( pm->ps->pm_type == PM_UFO ) {
		scale *= 6;
	} else {
		effectiveStance = PM_GetEffectiveStance( pm->ps );

		lerp = PM_GetViewHeightLerp( pm->ps->proneViewHeight, pm->ps->crouchViewHeight );
		if ( lerp != 0 ) {
			scale = ( ( 1.0f - lerp ) * pm->ps->crouchSpeedScale
					  + lerp * pm->ps->proneSpeedScale ) * scale;
		} else {
			lerp = PM_GetViewHeightLerp( pm->ps->crouchViewHeight, pm->ps->proneViewHeight );
			if ( lerp != 0 ) {
				scale = ( ( 1.0f - lerp ) * pm->ps->proneSpeedScale
						  + lerp * pm->ps->crouchSpeedScale ) * scale;
			} else if ( effectiveStance == 1 ) {
				scale *= pm->ps->proneSpeedScale;
			} else if ( effectiveStance == 2 ) {
				scale *= pm->ps->crouchSpeedScale;
			}
		}

		if ( pm->waterlevel ) {
			scale *= 1.0f - pm->waterlevel * 0.33333334f * ( 1.0f - pm_waterSwimScale );
		}
	}

	if ( pm->ps->weapon ) {
		weaponInfo = bg_weaponInfo[pm->ps->weapon];
		if ( weaponInfo->moveSpeedScale > 0 ) {
			scale *= weaponInfo->moveSpeedScale;
		}
	}

	if ( cmd->wbuttons & WBUTTON_STUNNED ) {
		scale *= pm_shellshockScale;
	}

	return scale;
}

/*
================
PM_SetMovementDir

Determine the rotation of the legs relative to the facing dir.  RTCW picked one
of eight compass directions; CoD stores the signed angle, clamped to +-90.
================
*/
static void PM_SetMovementDir( void ) {
	vec3_t moved;
	vec3_t dir;
	vec3_t angles;
	float len;
	int movementDir;

	if ( ( pm->ps->pm_flags & PMF_PRONE ) && !( pm->ps->eFlags & EF_TURRET_ACTIVE_MASK ) ) {
		movementDir = (int)AngleNormalize180( pm->ps->proneDirection - pm->ps->viewangles[YAW] );
		if ( abs( movementDir ) > 90 ) {
			movementDir = movementDir > 0 ? 90 : -90;
		}
		pm->ps->movementDir = (signed char)movementDir;
		return;
	}

	if ( pm->ps->pm_flags & PMF_LADDER ) {
		movementDir = (int)AngleNormalize180( vectoyaw( pm->ps->vLadderVec ) + 180.0f
											  - pm->ps->viewangles[YAW] );
		if ( abs( movementDir ) > 90 ) {
			movementDir = movementDir > 0 ? 90 : -90;
		}
		pm->ps->movementDir = (signed char)movementDir;
		return;
	}

	if ( ( !pm->cmd.forwardmove && !pm->cmd.rightmove )
		 || pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		pm->ps->movementDir = 0;
		return;
	}

	VectorSubtract( pm->ps->origin, pml.previous_origin, moved );
	len = VectorLength( moved );
	if ( len == 0 || pml.frametime * 5.0f >= len ) {
		pm->ps->movementDir = 0;
		return;
	}

	VectorNormalize2( moved, dir );
	vectoangles( dir, angles );

	movementDir = (int)AngleNormalize180( angles[YAW] - pm->ps->viewangles[YAW] );
	if ( pm->cmd.forwardmove < 0 ) {
		movementDir = (int)AngleNormalize180( (float)movementDir + 180.0f );
	}
	if ( abs( movementDir ) > 90 ) {
		movementDir = movementDir > 0 ? 90 : -90;
	}
	pm->ps->movementDir = (signed char)movementDir;
}

/*
=============
PM_JumpForSurface
=============
*/
static int PM_JumpForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
	return SURF_TYPE( pml.groundTrace.surfaceFlags );
}

/*
=============
PM_Jump
=============
*/
static int PM_Jump( void ) {
	int surftype;

	if ( pm->ps->pm_flags & PMF_LADDER ) {
		return EV_JUMP_LADDER;
	}
	surftype = PM_JumpForSurface();
	if ( !surftype ) {
		return 0;
	}
	return EV_JUMP + surftype;
}

/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) {
	vec3_t flatforward;
	vec3_t pushoff;
	float dot;

	if ( pm->cmd.serverTime - pm->ps->jumpTime < 500 ) {
		return qfalse;
	}

	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return qfalse;
	}
	if ( pm->ps->pm_flags & PMF_TIME_HARDLAND ) {
		return qfalse;
	}
	if ( PM_GetEffectiveStance( pm->ps ) ) {
		return qfalse;
	}
	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	pml.groundPlane = qfalse;       // jumping away
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pm->ps->velocity[2] = (float)sqrt( (double)( (float)pm->ps->gravity * 78.0f ) );
	pm->ps->fJumpOriginZ = pm->ps->origin[2] + 39.0f;

	if ( pm->ps->pm_flags & PMF_LADDER ) {
		// push away from the ladder
		pm->ps->velocity[2] *= 0.75f;

		flatforward[0] = pml.forward[0];
		flatforward[1] = pml.forward[1];
		flatforward[2] = 0;
		VectorNormalize( flatforward );

		if ( DotProduct( pml.forward, pm->ps->vLadderVec ) >= 0 ) {
			VectorCopy( flatforward, pushoff );
		} else {
			dot = DotProduct( flatforward, pm->ps->vLadderVec ) * -2.0f;
			pushoff[0] = dot * pm->ps->vLadderVec[0] + flatforward[0];
			pushoff[1] = dot * pm->ps->vLadderVec[1] + flatforward[1];
			pushoff[2] = dot * pm->ps->vLadderVec[2] + flatforward[2];
			VectorNormalize( pushoff );
		}

		pm->ps->velocity[0] = pm_ladderPushOff * pushoff[0];
		pm->ps->velocity[1] = pm_ladderPushOff * pushoff[1];
		pm->ps->pm_flags &= ~PMF_LADDER;
	}

	PM_AddEvent( PM_Jump() );

	pm->ps->aimSpreadScale += 64.0f;
	if ( pm->ps->aimSpreadScale > 255.0f ) {
		pm->ps->aimSpreadScale = 255.0f;
	}

	BG_AnimScriptEvent( pm->ps, pm->cmd.forwardmove < 0 ? ANIM_ET_JUMPBK : ANIM_ET_JUMP,
						qfalse, qtrue );

	if ( pm->debugLevel ) {
		Com_Printf( "%i:jump base =%.1f top=%.1f vel=%.1f\n", c_pmove,
					pm->ps->origin[2], pm->ps->fJumpOriginZ, pm->ps->velocity[2] );
	}

	return qtrue;
}

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;

	// normal slowdown
	PM_Friction();

	scale = PM_CmdScale( &pm->cmd );

	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} else {
		for ( i = 0 ; i < 3 ; i++ ) {
			wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove
						 + scale * pml.right[i] * pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	wishvel[2] = wishvel[2]
				 - (float)( ( pm->cmd.wbuttons & WBUTTON_LEAN_RIGHT ) << 4 )
				 + (float)( ( pm->cmd.wbuttons & WBUTTON_LEAN_LEFT ) << 4 );

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	PM_Accelerate( wishdir, wishspeed, pm_flyaccelerate );

	PM_StepSlideMove( qfalse );
}

/*
===================
PM_AirMove
===================
*/
static void PM_AirMove( void ) {
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	usercmd_t cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize( pml.forward );
	VectorNormalize( pml.right );

	for ( i = 0 ; i < 2 ; i++ ) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}
	wishvel[2] = 0;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate( wishdir, wishspeed, pm_airaccelerate );

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) {
		PM_ClipVelocity( pm->ps->velocity, pml.groundTrace.normal,
						 pm->ps->velocity, OVERCLIP );
	}

	PM_StepSlideMove( qtrue );

	PM_SetMovementDir();
}

/*
===================
PM_WalkMove
===================
*/
static void PM_WalkMove( void ) {
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	usercmd_t cmd;
	float accelerate;
	float vel;
	vec3_t oldVelocity;
	int effectiveStance;

	if ( PM_CheckJump() ) {
		// jumped away
		PM_AirMove();
		pm->ps->jumpTime = pm->cmd.serverTime;
		return;
	}

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale_Walk( &cmd );

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity( pml.forward, pml.groundTrace.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity( pml.right, pml.groundTrace.normal, pml.right, OVERCLIP );
	VectorNormalize( pml.forward );
	VectorNormalize( pml.right );

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	effectiveStance = PM_GetEffectiveStance( pm->ps );

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK )
		 || ( pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) ) {
		accelerate = pm_airaccelerate;
	} else if ( effectiveStance == 1 ) {
		accelerate = pm_prone_accelerate;
	} else if ( effectiveStance == 2 ) {
		accelerate = pm_ducked_accelerate;
	} else {
		accelerate = pm_accelerate;
	}

	if ( pm->ps->pm_flags & PMF_TIME_LAND ) {
		accelerate = accelerate * 0.25f;
	}

	PM_Accelerate( wishdir, wishspeed, accelerate );

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK )
		 || ( pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) ) {
		pm->ps->velocity[2] -= (float)pm->ps->gravity * pml.frametime;
	}

	vel = VectorLength( pm->ps->velocity );

	VectorCopy( pm->ps->velocity, oldVelocity );

	// slide along the ground plane
	PM_ClipVelocity( pm->ps->velocity, pml.groundTrace.normal,
					 pm->ps->velocity, OVERCLIP );

	// don't decrease velocity when going up or down a slope, but only if the
	// clip did not turn the move around
	if ( DotProduct( oldVelocity, pm->ps->velocity ) > 0 ) {
		VectorNormalize( pm->ps->velocity );
		VectorScale( pm->ps->velocity, vel, pm->ps->velocity );
	}

	// don't do anything if standing still
	if ( pm->ps->velocity[0] != 0 || pm->ps->velocity[1] != 0 ) {
		PM_StepSlideMove( qfalse );
	}

	PM_SetMovementDir();
}

/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction
	forward = VectorLength( pm->ps->velocity );
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear( pm->ps->velocity );
	} else {
		VectorNormalize( pm->ps->velocity );
		VectorScale( pm->ps->velocity, forward, pm->ps->velocity );
	}
}

/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float speed, drop, friction, control, newspeed;
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;

	pm->ps->viewHeightTarget = bg_viewheight_standing.integer;
	pm->ps->standViewHeight = bg_viewheight_standing.integer;

	// friction
	speed = VectorLength( pm->ps->velocity );
	if ( speed < 1 ) {
		VectorCopy( vec3_origin, pm->ps->velocity );
	} else {
		drop = 0;

		friction = pm_friction * 1.5f;  // extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * friction * pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if ( newspeed < 0 ) {
			newspeed = 0;
		}
		newspeed /= speed;

		VectorScale( pm->ps->velocity, newspeed, pm->ps->velocity );
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}
	wishvel[2] += pm->cmd.upmove;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA( pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin );
}

/*
===============
PM_UFOMove

Noclip with the lean buttons bound to up/down and an absolute vertical mode when
neither forward nor strafe is held.
===============
*/
static void PM_UFOMove( void ) {
	float speed, drop, friction, control, newspeed;
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	vec3_t up;
	vec3_t forward;
	float wishspeed;
	float scale;

	pm->ps->viewHeightTarget = bg_viewheight_standing.integer;
	pm->ps->standViewHeight = bg_viewheight_standing.integer;

	/* The button is multiplied as a mask, not as a boolean, and the result is
	 * truncated back into the signed char: lean right nets +32, lean left -16. */
	pm->cmd.upmove = (signed char)( pm->cmd.upmove
									+ -127 * ( pm->cmd.wbuttons & WBUTTON_LEAN_RIGHT ) );
	pm->cmd.upmove = (signed char)( pm->cmd.upmove
									+ 127 * ( pm->cmd.wbuttons & WBUTTON_LEAN_LEFT ) );

	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
		// straight up and down: the velocity is the command
		if ( pm->cmd.upmove == 0 ) {
			VectorClear( pm->ps->velocity );
			speed = 0;
		} else {
			pm->ps->velocity[0] = 0;
			pm->ps->velocity[1] = 0;
			pm->ps->velocity[2] = (float)pm->cmd.upmove;
			speed = 127.0f;
		}
	} else {
		speed = VectorLength( pm->ps->velocity );
		if ( speed < 1 ) {
			VectorClear( pm->ps->velocity );
			speed = 0;
		}
	}

	if ( speed != 0 ) {
		drop = 0;

		friction = pm_friction * 1.5f;
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * friction * pml.frametime;

		newspeed = speed - drop;
		if ( newspeed < 0 ) {
			newspeed = 0;
		}
		newspeed /= speed;

		VectorScale( pm->ps->velocity, newspeed, pm->ps->velocity );
	}

	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	// forward in UFO mode is world up crossed with the strafe axis, so the view
	// pitch does not drive the move
	VectorSet( up, 0, 0, 1 );
	CrossProduct( up, pml.right, forward );

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.right[i] * smove + forward[i] * fmove;
	}
	wishvel[2] = wishvel[2] + (float)pm->cmd.upmove + (float)pm->cmd.upmove;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	VectorMA( pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin );
}

/*
=================
PM_FootstepForSurface

Returns an event number appropriate for the groundtrace surface
=================
*/
static int PM_FootstepForSurface( int flags ) {
	int surftype;

	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
	surftype = SURF_TYPE( pml.groundTrace.surfaceFlags );
	if ( !surftype ) {
		return 0;
	}
	if ( flags & PMF_PRONE ) {
		return EV_FOOTSTEP_PRONE + surftype;
	}
	if ( flags & PMF_WALKING ) {
		return EV_FOOTSTEP_WALKING + surftype;
	}
	return EV_FOOTSTEP + surftype;
}

/*
=================
PM_LightLandingForSurface
=================
*/
static int PM_LightLandingForSurface( void ) {
	int surftype;

	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
	surftype = SURF_TYPE( pml.groundTrace.surfaceFlags );
	if ( !surftype ) {
		return 0;
	}
	return EV_FOOTSTEP_WALKING + surftype;
}

/*
=================
PM_MediumLandingForSurface
=================
*/
static int PM_MediumLandingForSurface( void ) {
	int surftype;

	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
	surftype = SURF_TYPE( pml.groundTrace.surfaceFlags );
	if ( !surftype ) {
		return 0;
	}
	return EV_FOOTSTEP + surftype;
}

/*
=================
PM_HardLandingForSurface
=================
*/
static int PM_HardLandingForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return EV_LANDING_HARD;
	}
	return EV_LANDING_HARD + SURF_TYPE( pml.groundTrace.surfaceFlags );
}

/*
=================
PM_DamageLandingForSurface
=================
*/
static int PM_DamageLandingForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return EV_LANDING_DAMAGE;
	}
	return EV_LANDING_DAMAGE + SURF_TYPE( pml.groundTrace.surfaceFlags );
}

/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
	float gravity;
	float height;
	float vel, acc;
	float t;
	float a, b, c, den;
	float delta;
	int damage;
	int viewKick;
	int stunTime;
	float speedMult;

	if ( pm->waterlevel == 3 ) {
		return;
	}

	if ( !pm->ps->legsTimer && pml.previous_velocity[2] < -220.0f ) {
		BG_AnimScriptEvent( pm->ps, ANIM_ET_LAND, qfalse, qtrue );
	}

	// calculate the exact velocity on landing
	vel = pml.previous_velocity[2];
	gravity = (float)pm->ps->gravity;
	acc = -gravity;

	a = acc / 2;
	b = vel;
	c = pml.previous_origin[2] - pm->ps->origin[2];

	den = b * b - 4 * a * c;
	if ( den < 0 ) {
		return;
	}
	t = ( -b - (float)sqrt( (double)den ) ) / ( 2 * a );

	delta = -( vel + t * acc );
	height = delta * delta / ( 2 * gravity );

	if ( pm->debugLevel ) {
		Com_Printf( "landing vel: %.1f fall height: %.1f\n", delta, height );
	}

	if ( bg_fallDamageMaxHeight.value <= bg_fallDamageMinHeight.value
		 || bg_fallDamageMinHeight.value < 0 ) {
		Com_Printf( "bg_fallDamageMaxHeight and bg_fallDamageMinHeight have bad values\n" );
		damage = 0;
	} else if ( height > bg_fallDamageMinHeight.value
				&& !( pml.groundTrace.surfaceFlags & SURF_NODAMAGE )
				&& pm->ps->pm_type < PM_DEAD ) {
		if ( height >= bg_fallDamageMaxHeight.value ) {
			damage = 100;
		} else {
			damage = (int)( ( height - bg_fallDamageMinHeight.value )
							/ ( bg_fallDamageMaxHeight.value - bg_fallDamageMinHeight.value )
							* 100.0f );
			if ( damage < 0 ) {
				damage = 0;
			} else if ( damage > 100 ) {
				damage = 100;
			}
		}
	} else {
		damage = 0;
	}

	if ( pm->waterlevel == 2 ) {
		damage = (int)( (float)damage * 0.5f );
	}

	if ( height > 12.0f ) {
		viewKick = (int)( ( ( height - 12.0f ) * 0.038461540f + 1.0f ) * 4.0f );
		if ( viewKick > 24 ) {
			viewKick = 24;
		}
	} else {
		viewKick = 0;
	}

	if ( pm->ps->fJumpOriginZ < -0.001f || pm->ps->fJumpOriginZ > 0.001f ) {
		pm->ps->pm_time = 200;
		pm->ps->pm_flags |= PMF_TIME_HARDLAND;
	}

	if ( damage ) {
		if ( pm->debugLevel ) {
			Com_Printf( "falling damage: %i\n", damage );
		}
		if ( damage >= 100 || ( pml.groundTrace.surfaceFlags & SURF_SLICK ) ) {
			VectorScale( pm->ps->velocity, 0.67000002f, pm->ps->velocity );
		} else {
			stunTime = 35 * damage + 500;
			if ( stunTime > 2000 ) {
				stunTime = 2000;
				speedMult = 0.2f;
			} else if ( stunTime <= 500 ) {
				speedMult = 0.5f;
			} else if ( stunTime >= 1500 ) {
				speedMult = 0.2f;
			} else {
				speedMult = 0.5f - ( (float)stunTime - 500.0f ) * 0.001f * 0.30000001f;
			}
			if ( pm->debugLevel > 1 ) {
				Com_Printf( "landing stun time: %i speed mult: %.2f\n", stunTime, speedMult );
			}
			pm->ps->pm_time = stunTime;
			pm->ps->pm_flags |= PMF_TIME_LAND;
			VectorScale( pm->ps->velocity, speedMult, pm->ps->velocity );
		}
		BG_AddPredictableEventToPlayerstate( PM_DamageLandingForSurface(), damage, pm->ps );
		return;
	}

	if ( height > 4.0f ) {
		if ( height < 8.0f ) {
			PM_AddEvent( PM_LightLandingForSurface() );
		} else if ( height < 12.0f ) {
			PM_AddEvent( PM_MediumLandingForSurface() );
		} else {
			VectorScale( pm->ps->velocity, 0.67000002f, pm->ps->velocity );
			BG_AddPredictableEventToPlayerstate( PM_HardLandingForSurface(), viewKick, pm->ps );
		}
	}
}

/*
=============
PM_CorrectAllSolid
=============
*/
static qboolean PM_CorrectAllSolid( trace_t *trace ) {
	int i;
	vec3_t point;

	// jitter around
	for ( i = 0 ; i < 26 ; i++ ) {
		point[0] = pm->ps->origin[0] + pm_correctSolidOffsets[i][0];
		point[1] = pm->ps->origin[1] + pm_correctSolidOffsets[i][1];
		point[2] = pm->ps->origin[2] + pm_correctSolidOffsets[i][2];

		pm->trace( trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
		if ( !trace->startsolid ) {
			VectorCopy( point, pm->ps->origin );

			point[0] = pm->ps->origin[0];
			point[1] = pm->ps->origin[1];
			point[2] = pm->ps->origin[2] - 1.0f;

			pm->trace( trace, pm->ps->origin, pm->mins, pm->maxs, point,
					   pm->ps->clientNum, pm->tracemask );
			pml.groundTrace = *trace;

			VectorCopy( trace->endpos, pm->ps->origin );
			return qtrue;
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
	pm->ps->fJumpOriginZ = 0;

	return qfalse;
}

/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
	trace_t trace;
	vec3_t point;

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
		// we just transitioned into freefall
		if ( pm->debugLevel ) {
			Com_Printf( "%i:lift\n", c_pmove );
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point,
				   pm->ps->clientNum, pm->tracemask );
		if ( trace.fraction == 1.0 ) {
			BG_AnimScriptEvent( pm->ps,
								pm->cmd.forwardmove < 0 ? ANIM_ET_JUMPBK : ANIM_ET_JUMP,
								qfalse, qtrue );
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}

/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
	vec3_t start;
	vec3_t point;
	trace_t trace;

	start[0] = pm->ps->origin[0];
	start[1] = pm->ps->origin[1];
	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	if ( pm->ps->eFlags & EF_TURRET_ACTIVE_MASK ) {
		start[2] = pm->ps->origin[2];
		point[2] = pm->ps->origin[2] - 1.0f;
	} else {
		start[2] = pm->ps->origin[2] + 0.25f;
		point[2] = pm->ps->origin[2] - 0.25f;
	}

	pm->trace( &trace, start, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
	pml.groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid ) {
		if ( !PM_CorrectAllSolid( &trace ) ) {
			return;
		}
	}

	if ( trace.startsolid ) {
		start[2] = pm->ps->origin[2] - 0.001f;
		pm->trace( &trace, start, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask );
		if ( trace.startsolid ) {
			pm->ps->groundEntityNum = ENTITYNUM_NONE;
			pml.groundPlane = qfalse;
			pml.walking = qfalse;
			return;
		}
		pml.groundTrace = trace;
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 ) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	pm->ps->fJumpOriginZ = 0;

	// check if getting thrown off the ground
	if ( !( pm->ps->pm_flags & PMF_LADDER ) && pm->ps->velocity[2] > 0
		 && DotProduct( pm->ps->velocity, trace.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf( "%i:kickoff\n", c_pmove );
		}
		BG_AnimScriptEvent( pm->ps,
							pm->cmd.forwardmove < 0 ? ANIM_ET_JUMPBK : ANIM_ET_JUMP,
							qfalse, qfalse );

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// slopes that are too steep will not be considered onground
	if ( trace.normal[2] < MIN_WALK_NORMAL ) {
		if ( pm->debugLevel ) {
			Com_Printf( "%i:steep\n", c_pmove );
		}
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.walking = qfalse;
		pml.groundPlane = qtrue;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// just hit the ground
		if ( pm->debugLevel ) {
			Com_Printf( "%i:Land\n", c_pmove );
		}
		PM_CrashLand();
	}

	pm->ps->groundEntityNum = trace.entityNum;

	PM_AddTouchEnt( trace.entityNum );
}

/*
=============
PM_SetWaterLevel
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t point;
	int cont;
	int sample1;
	int sample2;

	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + pm->ps->mins[2] + 1;
	cont = pm->pointcontents( point, pm->ps->clientNum, CONTENTS_WATER );

	if ( cont ) {
		sample2 = (int)( pm->ps->viewHeightCurrent - pm->ps->mins[2] );
		sample1 = sample2 / 2;

		pm->watertype = (byte)cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + pm->ps->mins[2] + sample1;
		cont = pm->pointcontents( point, pm->ps->clientNum, CONTENTS_WATER );
		if ( cont ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + pm->ps->mins[2] + sample2;
			cont = pm->pointcontents( point, pm->ps->clientNum, CONTENTS_WATER );
			if ( cont ) {
				pm->waterlevel = 3;
			}
		}
	}
}

/*
=============
PM_GetViewHeightLerpTime

How long the lerp to viewheight takes, in msec.
=============
*/
int PM_GetViewHeightLerpTime( const playerState_t *ps, int viewheight, int viewHeightLerpDown ) {
	if ( viewheight == ps->proneViewHeight ) {
		if ( ps->pm_flags & PMF_PRONE_DIVE ) {
			return 200;
		}
		return bg_duck2prone_time.integer;
	}
	if ( viewheight != ps->crouchViewHeight ) {
		return 200;
	}
	if ( viewHeightLerpDown ) {
		return ( ps->pm_flags & PMF_PRONE_DIVE ) ? 100 : 150;
	}
	return bg_prone2duck_time.integer;
}

/*
=============
PM_ViewHeightTableLerp
=============
*/
static float PM_ViewHeightTableLerp( int percent, float *originAdjust, const pmLerpEntry_t *table ) {
	int i;
	float frac;

	if ( percent ) {
		for ( i = 1 ; table[i].percent != PM_LERP_TABLE_END ; i++ ) {
			if ( table[i].percent == percent ) {
				*originAdjust = (float)table[i].originAdjust;
				return table[i].height;
			}
			if ( table[i].percent > percent ) {
				frac = (float)( percent - table[i - 1].percent )
					   / (float)( table[i].percent - table[i - 1].percent );
				*originAdjust = ( table[i].originAdjust - table[i - 1].originAdjust ) * frac
								+ table[i - 1].originAdjust;
				return frac * ( table[i].height - table[i - 1].height ) + table[i - 1].height;
			}
		}
	}

	*originAdjust = (float)table[0].originAdjust;
	return table[0].height;
}

/*
=============
PM_GetViewHeightLerp

0..1 through the viewheight lerp that is currently running from fromViewheight to
toViewheight, or 0 if that is not the lerp in flight.  -1 for either end means
"whatever is running".
=============
*/
static float PM_GetViewHeightLerp( int fromViewheight, int toViewheight ) {
	playerState_t *ps;
	int duration;
	float frac;

	ps = pm->ps;

	if ( !ps->viewHeightLerpTime ) {
		return 0;
	}

	if ( fromViewheight != -1 && toViewheight != -1 ) {
		if ( fromViewheight != ps->viewHeightLerpTarget ) {
			return 0;
		}
		if ( fromViewheight == ps->crouchViewHeight
			 && !( toViewheight == ps->proneViewHeight && !ps->viewHeightLerpDown )
			 && !( toViewheight == ps->standViewHeight && ps->viewHeightLerpDown ) ) {
			return 0;
		}
	}

	duration = PM_GetViewHeightLerpTime( ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown );

	frac = (float)( pm->cmd.serverTime - ps->viewHeightLerpTime ) / (float)duration;
	if ( frac < 0 ) {
		return 0;
	}
	if ( frac > 1 ) {
		return 1;
	}
	return frac;
}

/* .rdata 0x2006B980, 0x2006B9F0, 0x2006BA60, 0x2006BAE4, 0x2006BB08.  The first
 * four carry the spellings the UO Linux build exports; the two-point prone table
 * is CoD1-only and its name here is inferred. */
static const pmLerpEntry_t pmViewHeightLerpCrouchedRising[9] = {
	{ 0, 60.0f, 0 },  { 1, 59.5f, 0 },  { 4, 58.5f, 0 },
	{ 30, 56.0f, 0 }, { 80, 44.0f, 0 }, { 90, 41.5f, 0 },
	{ 95, 40.5f, 0 }, { 100, 40.0f, 0 },
	{ PM_LERP_TABLE_END, 0.0f, 0 }
};

static const pmLerpEntry_t pmViewHeightLerpStanding[9] = {
	{ 0, 40.0f, 0 },  { 5, 40.5f, 0 },  { 10, 41.5f, 0 },
	{ 20, 44.0f, 0 }, { 70, 56.0f, 0 }, { 96, 58.5f, 0 },
	{ 99, 59.5f, 0 }, { 100, 60.0f, 0 },
	{ PM_LERP_TABLE_END, 0.0f, 0 }
};

static const pmLerpEntry_t pmViewHeightLerpProne[11] = {
	{ 0, 40.0f, 0 },  { 11, 38.0f, 0 }, { 22, 33.0f, 0 },
	{ 34, 25.0f, 0 }, { 45, 16.0f, 0 }, { 50, 15.0f, 0 },
	{ 55, 16.0f, 0 }, { 70, 18.0f, 0 }, { 90, 17.0f, 0 },
	{ 100, 11.0f, 0 },
	{ PM_LERP_TABLE_END, 0.0f, 0 }
};

static const pmLerpEntry_t pmViewHeightLerpProneDive[3] = {
	{ 0, 40.0f, 0 }, { 100, 11.0f, 0 },
	{ PM_LERP_TABLE_END, 0.0f, 0 }
};

static const pmLerpEntry_t pmViewHeightLerpCrouchedFalling[8] = {
	{ 0, 11.0f, 0 },  { 5, 10.0f, 0 },  { 30, 21.0f, 0 },
	{ 50, 25.0f, 0 }, { 67, 31.0f, 0 }, { 83, 34.0f, 0 },
	{ 100, 40.0f, 0 },
	{ PM_LERP_TABLE_END, 0.0f, 0 }
};

/*
=============
PM_ViewHeightAdjust

Drive viewHeightCurrent toward viewHeightTarget, through the stance tables when
the target is one of the three stance heights and linearly otherwise.
=============
*/
static void PM_ViewHeightAdjust( void ) {
	int target;
	int percent;
	int duration;
	int lerpTarget;
	float originAdjust;
	float delta;
	float speed;
	vec3_t saveVelocity;
	vec3_t dir;
	const pmLerpEntry_t *table;

	target = pm->ps->viewHeightTarget;

	if ( !target || pm->ps->viewHeightCurrent == 0 ) {
		if ( pm->ps->pm_type == PM_SPECTATOR ) {
			pm->ps->viewHeightCurrent = 0;
		} else {
			pm->ps->viewHeightCurrent = (float)target;
		}
		return;
	}

	if ( (float)target == pm->ps->viewHeightCurrent && !pm->ps->viewHeightLerpTime ) {
		return;
	}

	percent = 0;

	if ( target != pm->ps->proneViewHeight && target != pm->ps->crouchViewHeight
		 && target != pm->ps->standViewHeight ) {
		// not a stance height: move at a flat 180 units/sec
		pm->ps->viewHeightLerpTime = 0;
		if ( (float)pm->ps->viewHeightTarget > pm->ps->viewHeightCurrent ) {
			pm->ps->viewHeightCurrent += pml.frametime * 180.0f;
			if ( (float)pm->ps->viewHeightTarget <= pm->ps->viewHeightCurrent ) {
				pm->ps->viewHeightCurrent = (float)pm->ps->viewHeightTarget;
			}
		} else {
			pm->ps->viewHeightCurrent -= pml.frametime * 180.0f;
			if ( (float)pm->ps->viewHeightTarget >= pm->ps->viewHeightCurrent ) {
				pm->ps->viewHeightCurrent = (float)pm->ps->viewHeightTarget;
			}
		}
		return;
	}

	if ( pm->ps->viewHeightLerpTime ) {
		lerpTarget = pm->ps->viewHeightLerpTarget;
		duration = PM_GetViewHeightLerpTime( pm->ps, lerpTarget, pm->ps->viewHeightLerpDown );
		percent = 100 * ( pm->cmd.serverTime - pm->ps->viewHeightLerpTime ) / duration;
		if ( percent < 0 ) {
			percent = 0;
		} else if ( percent > 100 ) {
			percent = 100;
		}

		if ( percent == 100 ) {
			pm->ps->viewHeightCurrent = (float)lerpTarget;
			pm->ps->viewHeightLerpTime = 0;
			pm->ps->viewHeightLerpPosAdj = 0;
		} else {
			if ( lerpTarget == pm->ps->proneViewHeight ) {
				if ( pm->ps->pm_flags & PMF_PRONE_DIVE ) {
					table = pmViewHeightLerpProneDive;
				} else {
					table = pmViewHeightLerpProne;
				}
			} else if ( lerpTarget == pm->ps->crouchViewHeight ) {
				if ( pm->ps->viewHeightLerpDown ) {
					table = pmViewHeightLerpCrouchedRising;
				} else {
					table = pmViewHeightLerpCrouchedFalling;
				}
			} else {
				table = pmViewHeightLerpStanding;
			}

			pm->ps->viewHeightCurrent = PM_ViewHeightTableLerp( percent, &originAdjust, table );

			if ( fabs( pm->ps->viewHeightLerpPosAdj - originAdjust ) > 0.05f ) {
				VectorCopy( pm->ps->velocity, saveVelocity );
				delta = originAdjust - pm->ps->viewHeightLerpPosAdj;
				if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
					delta = delta * 0.5f;
				}
				dir[0] = pml.forward[0];
				dir[1] = pml.forward[1];
				dir[2] = 0;
				speed = delta / pml.frametime;
				VectorNormalize( dir );
				pm->ps->velocity[0] = dir[0] * speed;
				pm->ps->velocity[1] = dir[1] * speed;
				pm->ps->velocity[2] = dir[2] * speed;
				PM_StepSlideMove( qtrue );
				VectorCopy( saveVelocity, pm->ps->velocity );
				pm->ps->viewHeightLerpPosAdj = originAdjust;
			}
		}
	}

	if ( !pm->ps->viewHeightLerpTime ) {
		// start a new lerp
		if ( (float)pm->ps->viewHeightTarget == pm->ps->viewHeightCurrent ) {
			return;
		}
		pm->ps->viewHeightLerpTime = pm->cmd.serverTime;
		target = pm->ps->viewHeightTarget;
		if ( target == pm->ps->proneViewHeight ) {
			pm->ps->viewHeightLerpDown = 1;
			if ( (float)pm->ps->crouchViewHeight >= pm->ps->viewHeightCurrent ) {
				pm->ps->viewHeightLerpTarget = pm->ps->proneViewHeight;
				return;
			}
			pm->ps->viewHeightLerpTarget = pm->ps->crouchViewHeight;
		} else if ( target == pm->ps->crouchViewHeight ) {
			if ( (float)target < pm->ps->viewHeightCurrent ) {
				pm->ps->viewHeightLerpDown = 1;
				pm->ps->viewHeightLerpTarget = pm->ps->crouchViewHeight;
				return;
			}
			pm->ps->viewHeightLerpDown = 0;
			pm->ps->viewHeightLerpTarget = pm->ps->crouchViewHeight;
		} else if ( target == pm->ps->standViewHeight ) {
			pm->ps->viewHeightLerpDown = 0;
			if ( (float)pm->ps->crouchViewHeight <= pm->ps->viewHeightCurrent ) {
				pm->ps->viewHeightLerpTarget = pm->ps->standViewHeight;
				return;
			}
			pm->ps->viewHeightLerpTarget = pm->ps->crouchViewHeight;
		}
		return;
	}

	// the target moved the other way mid-lerp: turn the lerp around and pick up
	// at the mirrored percentage
	if ( pm->ps->viewHeightTarget != pm->ps->viewHeightLerpTarget
		 && ( ( pm->ps->viewHeightTarget < pm->ps->viewHeightLerpTarget
				&& !pm->ps->viewHeightLerpDown )
			  || ( pm->ps->viewHeightTarget > pm->ps->viewHeightLerpTarget
				   && pm->ps->viewHeightLerpDown ) ) ) {
		pm->ps->viewHeightLerpDown = pm->ps->viewHeightLerpDown ^ 1;
		percent = 100 - percent;

		lerpTarget = pm->ps->viewHeightLerpTarget;
		if ( !pm->ps->viewHeightLerpDown ) {
			if ( lerpTarget == pm->ps->proneViewHeight ) {
				pm->ps->viewHeightLerpTarget = pm->ps->crouchViewHeight;
			} else if ( lerpTarget == pm->ps->crouchViewHeight ) {
				pm->ps->viewHeightLerpTarget = pm->ps->standViewHeight;
			}
		} else {
			if ( lerpTarget == pm->ps->standViewHeight ) {
				pm->ps->viewHeightLerpTarget = pm->ps->crouchViewHeight;
			} else if ( lerpTarget == pm->ps->crouchViewHeight ) {
				pm->ps->viewHeightLerpTarget = pm->ps->proneViewHeight;
			}
		}

		if ( percent == 100 ) {
			pm->ps->viewHeightCurrent = (float)pm->ps->viewHeightLerpTarget;
			pm->ps->viewHeightLerpTime = 0;
			pm->ps->viewHeightLerpPosAdj = 0;
			return;
		}

		duration = PM_GetViewHeightLerpTime( pm->ps, pm->ps->viewHeightLerpTarget,
											 pm->ps->viewHeightLerpDown );
		pm->ps->viewHeightLerpTime = pm->cmd.serverTime
									 - (int)( (float)duration * (float)percent * 0.0099999998f );

		if ( pm->ps->viewHeightLerpTarget == pm->ps->proneViewHeight ) {
			table = pmViewHeightLerpProne;
		} else if ( pm->ps->viewHeightLerpTarget == pm->ps->crouchViewHeight ) {
			if ( pm->ps->viewHeightLerpDown ) {
				table = pmViewHeightLerpCrouchedRising;
			} else {
				table = pmViewHeightLerpCrouchedFalling;
			}
		} else {
			table = pmViewHeightLerpStanding;
		}

		PM_ViewHeightTableLerp( percent, &originAdjust, table );
		pm->ps->viewHeightLerpPosAdj = originAdjust;
	}
}

/*
==============
PM_CheckDuck

Sets mins, maxs and viewHeightTarget
==============
*/
static void PM_CheckDuck( void ) {
	trace_t trace;
	vec3_t start;
	vec3_t end;
	int wasProne;
	int wasStanding;
	float jumpHeight;
	float pitch;

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		pm->mins[0] = -8;
		pm->mins[1] = -8;
		pm->mins[2] = -8;
		pm->maxs[0] = 8;
		pm->maxs[1] = 8;
		pm->maxs[2] = 16;
		pm->ps->pm_flags &= ~( PMF_PRONE | PMF_DUCKED );
		if ( pm->cmd.wbuttons & WBUTTON_PRONE ) {
			pm->cmd.wbuttons &= ~WBUTTON_PRONE;
			PM_AddEvent( EV_STANCE_FORCE_STAND );
		}
		pm->trace = pm->trace3;
		pm->ps->eFlags |= EF_UPDATE_BOUNDS;
		pm->ps->viewHeightTarget = 0;
		pm->ps->viewHeightCurrent = 0;
		return;
	}

	pm->mins[0] = pm->ps->mins[0];
	wasProne = pm->ps->pm_flags & PMF_PRONE;
	wasStanding = ( pm->ps->pm_flags & ( PMF_PRONE | PMF_DUCKED ) ) == 0;
	pm->mins[1] = pm->ps->mins[1];
	pm->maxs[0] = pm->ps->maxs[0];
	pm->maxs[1] = pm->ps->maxs[1];
	pm->mins[2] = pm->ps->mins[2];

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->maxs[2] = pm->ps->maxs[2];
		pm->ps->viewHeightTarget = pm->ps->deadViewHeight;
		goto setTrace;
	}

	if ( pm->ps->eFlags & EF_TURRET_ACTIVE_MASK ) {
		// a mounted weapon dictates the stance
		if ( ( pm->ps->eFlags & EF_TURRET_ACTIVE_PRONE )
			 && !( pm->ps->eFlags & EF_TURRET_ACTIVE_DUCK ) ) {
			pm->ps->pm_flags |= PMF_PRONE;
			pm->ps->pm_flags &= ~PMF_DUCKED;
		} else if ( ( pm->ps->eFlags & EF_TURRET_ACTIVE_DUCK )
					&& !( pm->ps->eFlags & EF_TURRET_ACTIVE_PRONE ) ) {
			pm->ps->pm_flags |= PMF_DUCKED;
			pm->ps->pm_flags &= ~PMF_PRONE;
		} else {
			pm->ps->pm_flags &= ~( PMF_PRONE | PMF_DUCKED );
		}
	} else if ( !( pm->ps->pm_flags & PMF_IGNORE_INPUT ) ) {
		if ( pm->ps->pm_flags & PMF_LADDER ) {
			// no stance changes on a ladder
			if ( pm->cmd.wbuttons & ( WBUTTON_PRONE | WBUTTON_CROUCH ) ) {
				pm->cmd.wbuttons &= ~( WBUTTON_PRONE | WBUTTON_CROUCH );
				PM_AddEvent( EV_STANCE_FORCE_STAND );
			}
		}

		if ( pm->cmd.wbuttons & WBUTTON_PRONE ) {
			if ( ( pm->ps->pm_flags & PMF_PRONE )
				 || BG_CheckProneValid( pm->ps->clientNum, pm->ps->origin, pm->maxs[0], 30.0f,
										pm->ps->viewangles[YAW],
										&pm->ps->fTorsoHeight, &pm->ps->fTorsoPitch,
										&pm->ps->fWaistPitch, qfalse,
										pm->ps->groundEntityNum != ENTITYNUM_NONE, NULL,
										pm->trace3, pm->trace2, qfalse ) ) {
				pm->ps->pm_flags |= PMF_PRONE;
			} else {
				pm->ps->pm_flags |= PMF_PRONE_BLOCKED;
				pm->ps->pm_flags &= ~PMF_PRONE_DIVE;
				if ( !( pm->cmd.wbuttons & WBUTTON_NOSTANCEEVENT ) ) {
					if ( pm->ps->pm_flags & PMF_DUCKED ) {
						PM_AddEvent( EV_STANCE_FORCE_CROUCH );
					} else {
						PM_AddEvent( EV_STANCE_FORCE_STAND );
					}
				}
			}
		} else if ( pm->cmd.wbuttons & WBUTTON_CROUCH ) {
			pm->ps->pm_flags &= ~PMF_PRONE_DIVE;
			if ( !( pm->ps->pm_flags & PMF_PRONE ) ) {
				pm->ps->pm_flags |= PMF_DUCKED;
			} else {
				pm->maxs[2] = 50;
				pm->trace3( &trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin,
							pm->ps->clientNum, pm->tracemask );
				if ( !trace.allsolid ) {
					pm->ps->pm_flags &= ~PMF_PRONE;
					pm->ps->pm_flags |= PMF_DUCKED;
				} else if ( !( pm->cmd.wbuttons & WBUTTON_NOSTANCEEVENT ) ) {
					PM_AddEvent( EV_STANCE_FORCE_PRONE );
				}
			}
		} else {
			pm->ps->pm_flags &= ~PMF_PRONE_DIVE;
			if ( pm->ps->pm_flags & PMF_PRONE ) {
				pm->maxs[2] = pm->ps->maxs[2];
				pm->trace3( &trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin,
							pm->ps->clientNum, pm->tracemask );
				if ( !trace.allsolid ) {
					pm->ps->pm_flags &= ~( PMF_PRONE | PMF_DUCKED );
				} else {
					pm->maxs[2] = 50;
					pm->trace3( &trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin,
								pm->ps->clientNum, pm->tracemask );
					if ( !trace.allsolid ) {
						pm->ps->pm_flags &= ~PMF_PRONE;
						pm->ps->pm_flags |= PMF_DUCKED;
					} else if ( !( pm->cmd.wbuttons & WBUTTON_NOSTANCEEVENT ) ) {
						PM_AddEvent( EV_STANCE_FORCE_PRONE );
					}
				}
			} else if ( pm->ps->pm_flags & PMF_DUCKED ) {
				pm->maxs[2] = pm->ps->maxs[2];
				pm->trace3( &trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin,
							pm->ps->clientNum, pm->tracemask );
				if ( !trace.allsolid ) {
					pm->ps->pm_flags &= ~PMF_DUCKED;
				} else if ( !( pm->cmd.wbuttons & WBUTTON_NOSTANCEEVENT ) ) {
					PM_AddEvent( EV_STANCE_FORCE_CROUCH );
				}
			}
		}
	}

	if ( pm->ps->pm_flags & PMF_PRONE ) {
		if ( g_debugProneCheck.integer == 2 ) {
			BG_CheckProneValid( pm->ps->clientNum, pm->ps->origin, pm->maxs[0], 30.0f,
								pm->ps->viewangles[YAW], NULL, NULL, NULL, qfalse,
								pm->ps->groundEntityNum != ENTITYNUM_NONE, NULL,
								pm->trace3, pm->trace2, qfalse );
		}
		pm->ps->eFlags |= EF_PRONE;
		pm->ps->eFlags &= ~EF_CROUCHING;
		pm->ps->viewHeightTarget = pm->ps->proneViewHeight;

		if ( pm->ps->viewHeightCurrent == (float)pm->ps->viewHeightTarget
			 || PM_GetViewHeightLerp( -1, -1 ) > 0.5f ) {
			pm->maxs[2] = 30;
		} else {
			pm->maxs[2] = 50;
		}

		if ( !wasProne ) {
			// just went prone: slide the body to a clear spot and pick a
			// direction to lie in
			if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
				pm->ps->pm_flags &= ~PMF_PRONE_MOVEOVERRIDE;
				pm->ps->pm_flags &= ~PMF_ADS;
			}

			if ( !( pm->ps->pm_flags & PMF_ADS ) && pm->cmd.forwardmove ) {
				pm->ps->pm_flags |= PMF_PRONE_DIVE;
				if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
					jumpHeight = wasStanding ? 34.0f : 24.0f;
					pm->ps->velocity[2] = (float)sqrt( (double)( ( jumpHeight + jumpHeight )
																 * (float)pm->ps->gravity ) );
					pml.groundPlane = qfalse;
					pml.walking = qfalse;
					pm->ps->groundEntityNum = ENTITYNUM_NONE;
				}
				pm->ps->aimSpreadScale = 255.0f;
			}

			start[0] = pm->ps->origin[0];
			start[1] = pm->ps->origin[1];
			start[2] = pm->ps->origin[2] + 10.0f;
			pm->trace2( &trace, pm->ps->origin, pm->mins, pm->maxs, start,
						pm->ps->clientNum, pm->tracemask );
			VectorCopy( trace.endpos, start );
			pm->trace2( &trace, start, pm->mins, pm->maxs, pm->ps->origin,
						pm->ps->clientNum, pm->tracemask );
			VectorCopy( trace.endpos, pm->ps->origin );

			pm->ps->proneDirection = pm->ps->viewangles[YAW];

			end[0] = pm->ps->origin[0];
			end[1] = pm->ps->origin[1];
			end[2] = pm->ps->origin[2] - 0.25f;
			pm->trace2( &trace, pm->ps->origin, pm->mins, pm->maxs, end,
						pm->ps->clientNum, pm->tracemask );
			if ( trace.startsolid || trace.fraction >= 1.0f ) {
				pm->ps->proneDirectionPitch = 0;
			} else {
				pm->ps->proneDirectionPitch = PitchOfVectorAlongYaw( pm->ps->proneDirection,
																   trace.normal );
			}

			pitch = AngleNormalize180( pm->ps->proneDirectionPitch - pm->ps->viewangles[PITCH] );
			if ( pitch < -45.0f ) {
				pm->ps->proneTorsoPitch = pm->ps->viewangles[PITCH] - 45.0f;
			} else if ( pitch > 45.0f ) {
				pm->ps->proneTorsoPitch = pm->ps->viewangles[PITCH] + 45.0f;
			} else {
				pm->ps->proneTorsoPitch = pm->ps->proneDirectionPitch;
			}
		}
	} else if ( pm->ps->pm_flags & PMF_DUCKED ) {
		pm->ps->eFlags |= EF_CROUCHING;
		pm->ps->eFlags &= ~EF_PRONE;
		pm->maxs[2] = 50;
		pm->ps->viewHeightTarget = pm->ps->crouchViewHeight;
	} else {
		pm->ps->eFlags &= ~( EF_CROUCHING | EF_PRONE );
		pm->maxs[2] = pm->ps->maxs[2];
		pm->ps->viewHeightTarget = pm->ps->standViewHeight;
	}

setTrace:
	if ( pm->ps->pm_flags & PMF_PRONE ) {
		pm->trace = pm->trace2;
	} else {
		pm->trace = pm->trace3;
	}
	pm->ps->eFlags |= EF_UPDATE_BOUNDS;

	PM_ViewHeightAdjust();
}

/*
================
PM_FootstepEvent

RTCW raised this inline in PM_Footsteps; CoD split it out because the ladder and
the step-up in PM_StepSlideMove both drive the bob cycle.
================
*/
void PM_FootstepEvent( int oldBobCycle, qboolean footstep, int newBobCycle ) {
	trace_t trace;
	vec3_t mins, maxs, end;
	int surftype;

	if ( !( ( ( newBobCycle + 64 ) ^ ( oldBobCycle + 64 ) ) & 128 ) ) {
		return;
	}

	if ( !pm->waterlevel ) {
		if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
			if ( footstep && ( pm->ps->pm_flags & PMF_LADDER ) ) {
				// the ladder is behind us; find what it is made of
				mins[0] = pm->mins[0] + 6.0f;
				mins[1] = pm->mins[1] + 6.0f;
				mins[2] = 8.0f;
				maxs[0] = pm->maxs[0] - 6.0f;
				maxs[1] = pm->maxs[1] - 6.0f;
				maxs[2] = pm->maxs[2];
				if ( maxs[2] < 8.0f ) {
					maxs[2] = 8.0f;
				}
				end[0] = pm->ps->origin[0] - pm->ps->vLadderVec[0] * 31.0f;
				end[1] = pm->ps->origin[1] - pm->ps->vLadderVec[1] * 31.0f;
				end[2] = pm->ps->origin[2] - pm->ps->vLadderVec[2] * 31.0f;
				pm->trace( &trace, pm->ps->origin, mins, maxs, end, pm->ps->clientNum,
						   pm->tracemask & ~0x2010000 );
				surftype = SURF_TYPE( trace.surfaceFlags );
				if ( trace.fraction == 1.0f || !surftype ) {
					surftype = 13;      // metal
				}
				PM_AddEvent( EV_FOOTSTEP + surftype );
			}
		} else if ( footstep ) {
			PM_AddEvent( PM_FootstepForSurface( pm->ps->pm_flags ) );
		}
	} else if ( pm->waterlevel == 1 || pm->waterlevel == 2 ) {
		if ( pm->ps->pm_flags & PMF_PRONE ) {
			PM_AddEvent( EV_SWIM_PRONE );
		} else if ( pm->ps->pm_flags & PMF_WALKING ) {
			PM_AddEvent( EV_FOOTSTEP_WADE );
		} else {
			PM_AddEvent( EV_SWIM );
		}
	}
}

/*
================
PM_ShouldMakeFootsteps
================
*/
qboolean PM_ShouldMakeFootsteps( void ) {
	int effectiveStance;

	effectiveStance = PM_GetEffectiveStance( pm->ps );
	if ( effectiveStance == 1 || effectiveStance == 2 ) {
		return qfalse;
	}
	if ( pm->ps->pm_flags & PMF_WALKING ) {
		return qfalse;
	}
	return qtrue;
}

/*
================
PM_Footsteps
================
*/
static void PM_Footsteps( void ) {
	float bobmove;
	int old;
	qboolean footstep;
	int effectiveStance;
	int walking;
	int anim;
	int strafing;
	float speed;
	float lerp;

	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = (float)sqrt( (double)( pm->ps->velocity[0] * pm->ps->velocity[0]
										 + pm->ps->velocity[1] * pm->ps->velocity[1] ) );

	if ( pm->ps->eFlags & EF_TURRET_ACTIVE_MASK ) {
		if ( pm->ps->pm_flags & PMF_PRONE ) {
			BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLEPRONE, qtrue );
		} else if ( pm->ps->pm_flags & PMF_DUCKED ) {
			BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLECR, qtrue );
		} else {
			BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLE, qtrue );
		}
		return;
	}

	effectiveStance = PM_GetEffectiveStance( pm->ps );

	/* CoD 1.5 PM_Footsteps (Linux 0x24AE8): linkto owns the origin and
	 * clears groundEntityNum, but a living linked player still needs an idle. */
	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE && pm->ps->pm_type != PM_NORMAL_LINKED ) {
		if ( ( pm->ps->pm_flags & PMF_LADDER )
			 && pm->cmd.serverTime - pm->ps->jumpTime >= pm_ladderJumpTime ) {
			// airborne on a ladder: the bob cycle follows the climb speed
			speed = pm_ladderScale * 1.5f * 127.0f;
			if ( pm->ps->pm_flags & PMF_WALKING ) {
				bobmove = pm->ps->velocity[2] / ( speed * pm->ps->walkSpeedScale ) * 0.34999999f;
			} else {
				bobmove = pm->ps->velocity[2] / ( speed * pm->ps->runSpeedScale ) * 0.44999999f;
			}

			if ( pm->ps->velocity[2] < 0 ) {
				BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_CLIMBDOWN, qtrue );
			} else {
				BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_CLIMBUP, qtrue );
			}

			old = pm->ps->bobCycle;
			pm->ps->bobCycle = (int)( (float)pml.msec * bobmove + (float)old ) & 255;
			PM_FootstepEvent( old, qtrue, pm->ps->bobCycle );
		}
		return;
	}

	walking = pm->ps->pm_flags & PMF_WALKING;

	if ( pm->xyspeed < 10 || pm->ps->pm_type == PM_NORMAL_LINKED ) {
		// standing still
		if ( pm->xyspeed < 1 ) {
			pm->ps->bobCycle = 0;
		}
		if ( pm->ps->viewHeightLerpTarget == pm->ps->proneViewHeight ) {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLEPRONE, qtrue );
		} else if ( pm->ps->viewHeightLerpTarget == pm->ps->crouchViewHeight ) {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLECR, qtrue );
		} else {
			BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLE, qtrue );
			return;
		}
		if ( anim < 0 ) {
			BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLE, qtrue );
		}
		return;
	}

	// the animation rate is driven by the speed the command asks for, not by
	// the speed we actually got
	speed = (float)pm->ps->speed;
	strafing = STRAFING_NOT;
	if ( pm->cmd.forwardmove ) {
		if ( pm->cmd.rightmove ) {
			speed = speed * ( ( pm->ps->strafeSpeedScale - 1.0f ) * 0.75f + 1.0f + 1.0f ) * 0.5f;
			if ( pm->cmd.forwardmove < 0 ) {
				speed = speed * ( pm->ps->backSpeedScale + 1.0f ) * 0.5f;
			}
		} else if ( pm->cmd.forwardmove < 0 ) {
			speed = speed * pm->ps->backSpeedScale;
		}
		BG_UpdateConditionValue( pm->ps->clientNum, ANIM_COND_STRAFING, strafing, qtrue );
	} else if ( pm->cmd.rightmove ) {
		speed = speed * ( ( pm->ps->strafeSpeedScale - 1.0f ) * 0.75f + 1.0f );
		strafing = pm->cmd.rightmove > 0 ? STRAFING_RIGHT : STRAFING_LEFT;
		BG_UpdateConditionValue( pm->ps->clientNum, ANIM_COND_STRAFING, strafing, qtrue );
	}

	if ( walking ) {
		speed = speed * pm->ps->walkSpeedScale;
	} else {
		speed = speed * pm->ps->runSpeedScale;
	}

	lerp = PM_GetViewHeightLerp( pm->ps->proneViewHeight, pm->ps->crouchViewHeight );
	if ( lerp != 0 ) {
		speed = ( ( 1.0f - lerp ) * pm->ps->crouchSpeedScale
				  + lerp * pm->ps->proneSpeedScale ) * speed;
	} else {
		lerp = PM_GetViewHeightLerp( pm->ps->crouchViewHeight, pm->ps->proneViewHeight );
		if ( lerp != 0 ) {
			speed = ( ( 1.0f - lerp ) * pm->ps->proneSpeedScale
					  + lerp * pm->ps->crouchSpeedScale ) * speed;
		} else if ( effectiveStance == 1 ) {
			speed = speed * pm->ps->proneSpeedScale;
		} else if ( effectiveStance == 2 ) {
			speed = speed * pm->ps->crouchSpeedScale;
		}
	}

	if ( effectiveStance == 1 ) {
		bobmove = pm->xyspeed / speed;
		bobmove *= walking ? 0.23999999f : 0.25f;
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_WALKPRONEBK, qtrue );
		} else {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_WALKPRONE, qtrue );
		}
	} else if ( effectiveStance == 2 ) {
		bobmove = pm->xyspeed / speed;
		bobmove *= walking ? 0.315f : 0.34f;
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT,
										   walking ? ANIM_MT_WALKCRBK : ANIM_MT_RUNCRBK, qtrue );
		} else {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT,
										   walking ? ANIM_MT_WALKCR : ANIM_MT_RUNCR, qtrue );
		}
	} else {
		bobmove = pm->xyspeed / speed;
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
			if ( walking ) {
				bobmove *= 0.32499999f;
				anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_WALKBK, qtrue );
			} else {
				bobmove *= 0.36000001f;
				anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_RUNBK, qtrue );
			}
		} else {
			if ( walking ) {
				bobmove *= 0.30500001f;
				anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_WALK, qtrue );
			} else {
				bobmove *= 0.33500001f;
				anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_RUN, qtrue );
			}
		}
	}

	footstep = PM_ShouldMakeFootsteps();

	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)( (float)pml.msec * bobmove + (float)old ) & 255;

	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
		// sliding to a stop: no footsteps
		if ( pm->xyspeed > 120 ) {
			return;
		}
		if ( pm->ps->viewHeightLerpTarget == pm->ps->proneViewHeight ) {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLEPRONE, qtrue );
		} else if ( pm->ps->viewHeightLerpTarget == pm->ps->crouchViewHeight ) {
			anim = BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLECR, qtrue );
		}
		if ( anim < 0 ) {
			BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLE, qtrue );
		}
		return;
	}

	if ( anim < 0 ) {
		BG_AnimScriptAnimation( pm->ps, AISTATE_COMBAT, ANIM_MT_IDLE, qtrue );
	}

	PM_FootstepEvent( old, footstep, pm->ps->bobCycle );
}

/*
================
PM_FoliageSounds

Brush up against the foliage volume often enough and it rustles; the interval
scales from bg_foliagesnd_slowinterval down to bg_foliagesnd_fastinterval over
the speed band the two speed cvars name.
================
*/
static void PM_FoliageSounds( void ) {
	trace_t trace;
	vec3_t mins, maxs;
	float frac;
	int interval;

	if ( (float)bg_foliagesnd_minspeed.integer > pm->xyspeed ) {
		if ( bg_foliagesnd_resetinterval.integer + pm->ps->iFoliageSoundTime
			 < pm->cmd.serverTime ) {
			pm->ps->iFoliageSoundTime = 0;
		}
		return;
	}

	frac = ( pm->xyspeed - (float)bg_foliagesnd_minspeed.integer )
		   / (float)( bg_foliagesnd_maxspeed.integer - bg_foliagesnd_minspeed.integer );
	if ( frac > 1.0f ) {
		frac = 1.0f;
	}

	interval = (int)( (float)( bg_foliagesnd_fastinterval.integer
							   - bg_foliagesnd_slowinterval.integer ) * frac
					  + (float)bg_foliagesnd_slowinterval.integer );

	if ( pm->ps->iFoliageSoundTime + interval >= pm->cmd.serverTime ) {
		return;
	}

	mins[0] = pm->mins[0] * 0.75f;
	mins[1] = pm->mins[1] * 0.75f;
	mins[2] = pm->mins[2] * 0.75f;
	maxs[0] = pm->maxs[0] * 0.75f;
	maxs[1] = pm->maxs[1] * 0.75f;
	maxs[2] = pm->maxs[2] * 0.75f;
	maxs[2] = pm->maxs[2] * 0.89999998f;

	/* 2 is the foliage-only content bit; q_shared.h has no CONTENTS_ set yet. */
	pm->trace( &trace, pm->ps->origin, mins, maxs, pm->ps->origin, pm->ps->clientNum, 2 );
	if ( trace.startsolid ) {
		PM_AddEvent( EV_FOLIAGE_SOUND );
		pm->ps->iFoliageSoundTime = pm->cmd.serverTime;
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {
	//
	// if just entered a water volume, play a sound
	//
	if ( !pml.previous_waterlevel && pm->waterlevel ) {
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if ( pml.previous_waterlevel && !pm->waterlevel ) {
		PM_AddEvent( EV_WATER_LEAVE );
	}
}

/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
	// drop misc timing counter
	if ( pm->ps->pm_time ) {
		if ( pml.msec >= pm->ps->pm_time ) {
			pm->ps->pm_flags &= ~( PMF_TIME_LAND | PMF_TIME_KNOCKBACK | PMF_TIME_HARDLAND );
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop animation counter
	if ( pm->ps->legsTimer > 0 ) {
		pm->ps->legsTimer -= pml.msec;
		if ( pm->ps->legsTimer < 0 ) {
			pm->ps->legsTimer = 0;
		}
	}

	if ( pm->ps->torsoTimer > 0 ) {
		pm->ps->torsoTimer -= pml.msec;
		if ( pm->ps->torsoTimer < 0 ) {
			pm->ps->torsoTimer = 0;
		}
	}
}

/*
================
PM_UpdateLean
================
*/
void PM_UpdateLean( playerState_t *ps, const usercmd_t *cmd,
					void ( *trace )( trace_t *results, const vec3_t start, const vec3_t mins,
									 const vec3_t maxs, const vec3_t end, int passEntityNum,
									 int contentMask ) ) {
	trace_t leanTrace;
	vec3_t start, end;
	vec3_t mins, maxs;
	int leaning;
	float maxLean;
	float leanFrac;
	float blockedFrac;
	int effectiveStance;

	leaning = 0;
	if ( ( cmd->wbuttons & ( WBUTTON_LEAN_LEFT | WBUTTON_LEAN_RIGHT ) )
		 && !( ps->pm_flags & PMF_IGNORE_INPUT )
		 && ps->pm_type < PM_DEAD
		 && ( ps->groundEntityNum != ENTITYNUM_NONE || ps->pm_type == PM_NORMAL_LINKED ) ) {
		if ( cmd->wbuttons & WBUTTON_LEAN_LEFT ) {
			leaning = -1;
		}
		if ( cmd->wbuttons & WBUTTON_LEAN_RIGHT ) {
			leaning++;
		}
	}

	if ( ps->eFlags & EF_TURRET_ACTIVE_MASK ) {
		leaning = 0;
	}

	effectiveStance = PM_GetEffectiveStance( ps );
	if ( effectiveStance == 1 ) {
		maxLean = 0.25f;
	} else if ( effectiveStance == 2 ) {
		maxLean = 0.5f;
	} else {
		maxLean = 1.0f;
	}

	leanFrac = ps->leanf;

	if ( !leaning ) {
		if ( leanFrac > 0 ) {
			leanFrac = leanFrac - (float)pml.msec * 0.0035714286f * maxLean;
			if ( leanFrac < 0 ) {
				leanFrac = 0;
			}
		} else if ( leanFrac < 0 ) {
			leanFrac = leanFrac + (float)pml.msec * 0.0035714286f * maxLean;
			if ( leanFrac > 0 ) {
				leanFrac = 0;
			}
		}
	} else if ( leaning > 0 ) {
		if ( leanFrac < maxLean ) {
			leanFrac = leanFrac + (float)pml.msec * 0.0028571428f * maxLean;
		}
		if ( leanFrac > maxLean ) {
			leanFrac = maxLean;
		}
	} else {
		if ( leanFrac > -maxLean ) {
			leanFrac = leanFrac - (float)pml.msec * 0.0028571428f * maxLean;
		}
		if ( leanFrac < -maxLean ) {
			leanFrac = -maxLean;
		}
	}

	ps->leanf = leanFrac;

	if ( ps->leanf != 0 && ps->pm_type != PM_NORMAL_LINKED ) {
		// trace out to the fully leaned position and give back however much
		// clearance there was
		end[0] = ps->origin[0];
		end[1] = ps->origin[1];
		end[2] = ps->origin[2] + ps->viewHeightCurrent;
		VectorCopy( end, start );

		AddLeanToPosition( end, ps->viewangles[YAW], (float)PM_FloatSign( ps->leanf ),
						   16.0f, 20.0f );

		mins[0] = -8;
		mins[1] = -8;
		mins[2] = -8;
		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		trace( &leanTrace, start, mins, maxs, end, ps->clientNum, MASK_PLAYERSOLID );

		blockedFrac = UnGetLeanFraction( leanTrace.fraction );
		if ( blockedFrac < PM_FloatAbs( ps->leanf ) ) {
			ps->leanf = (float)PM_FloatSign( ps->leanf ) * blockedFrac;
		}
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles are being
updated instead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd,
						  void ( *trace )( trace_t *results, const vec3_t start, const vec3_t mins,
										   const vec3_t maxs, const vec3_t end, int passEntityNum,
										   int contentMask ) ) {
	short temp;
	int i;
	float ladderYaw;
	float diff;
	float yawcap;
	float newDirection;
	float step;
	float limit;

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;     // no view changes at all
	}

	if ( ps->pm_type >= PM_DEAD ) {
		if ( ps->stats[1] == 999 ) {
			ps->stats[1] = (int)SHORT2ANGLE( (short)( cmd->angles[YAW] + ps->delta_angles[YAW] ) );
		}
		PM_UpdateLean( ps, cmd, trace );
		return;
	}

	// circularly clamp the angles with deltas
	for ( i = 0 ; i < 3 ; i++ ) {
		temp = (short)( cmd->angles[i] + ps->delta_angles[i] );
		if ( i == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		ps->viewangles[i] = SHORT2ANGLE( temp );
	}

	// on a ladder the view is capped around the ladder's facing
	if ( ( ps->pm_flags & PMF_LADDER ) && ps->groundEntityNum == ENTITYNUM_NONE ) {
		if ( bg_ladder_yawcap.integer ) {
			yawcap = (float)bg_ladder_yawcap.integer;
			ladderYaw = vectoyaw( ps->vLadderVec ) + 180.0f;
			diff = AngleNormalize180( ladderYaw - ps->viewangles[YAW] );
			if ( diff > yawcap || diff < -yawcap ) {
				if ( diff > yawcap ) {
					diff = diff - yawcap;
				} else {
					diff = yawcap + diff;
				}
				ps->delta_angles[YAW] += ANGLE2SHORT( diff );
				if ( diff <= 0 ) {
					ps->viewangles[YAW] = AngleNormalize360( ladderYaw + yawcap );
				} else {
					ps->viewangles[YAW] = AngleNormalize360( ladderYaw - yawcap );
				}
			}
		}
	}

	// prone caps the view around the direction the body is lying in
	if ( ( ps->pm_flags & PMF_PRONE ) && !( ps->eFlags & EF_TURRET_ACTIVE_MASK ) ) {
		if ( g_debugProneCheck.integer ) {
			static const vec3_t debugColor = { 1, 1, 1 };
			vec3_t debugStart, debugEnd, debugForward;
			float sp, cp, sy, cy, angle;

			debugStart[0] = ps->origin[0];
			debugStart[1] = ps->origin[1];
			debugStart[2] = (float)ps->proneViewHeight + ps->origin[2];

			angle = ps->viewangles[YAW] * 0.017453292f;
			cy = (float)cos( angle );
			sy = (float)sin( angle );
			angle = ps->viewangles[PITCH] * 0.017453292f;
			cp = (float)cos( angle );
			sp = (float)sin( angle );

			debugForward[0] = cp * cy;
			debugForward[1] = cp * sy;
			debugForward[2] = -sp;

			debugEnd[0] = debugForward[0] * 18.0f + debugStart[0];
			debugEnd[1] = debugForward[1] * 18.0f + debugStart[1];
			debugEnd[2] = debugForward[2] * 18.0f + debugStart[2];

			trap_AddDebugLine( debugStart, debugEnd, debugColor, 1, 1 );
			G_DebugArc( debugStart, 16.0f,
						ps->proneDirection - bg_prone_yawcap.value,
						bg_prone_yawcap.value + ps->proneDirection,
						debugColor, 1, 1 );
		}

		diff = AngleNormalize180( ps->proneDirection - ps->viewangles[YAW] );

		if ( ( bg_prone_softyawedge.integer
			   && ( diff > (float)( bg_prone_yawcap.integer - 5 )
					|| diff < (float)( 5 - bg_prone_yawcap.integer ) ) )
			 || ( ( cmd->forwardmove || cmd->rightmove ) && diff != 0 ) ) {
			// turn the body toward the view if there is room for it
			step = pml.frametime * 55.0f;
			if ( step > PM_FloatAbs( diff ) ) {
				newDirection = ps->viewangles[YAW];
			} else if ( diff <= 0 ) {
				newDirection = step + ps->proneDirection;
			} else {
				newDirection = ps->proneDirection - step;
			}

			if ( BG_CheckProneValid( ps->clientNum, ps->origin, ps->maxs[0], 30.0f, newDirection,
									 &ps->fTorsoHeight, &ps->fTorsoPitch, &ps->fWaistPitch,
									 qtrue, ps->groundEntityNum != ENTITYNUM_NONE, NULL,
									 trace, NULL, qfalse ) ) {
				ps->proneDirection = newDirection;
			} else {
				limit = (float)bg_prone_yawcap.integer + 0.1f;
				if ( diff > limit || -limit > diff ) {
					ps->pm_flags |= PMF_PRONE_BLOCKED;
				}
			}
		}

		yawcap = (float)bg_prone_yawcap.integer;
		if ( diff > yawcap || diff < -yawcap ) {
			if ( diff > yawcap ) {
				diff = diff - yawcap;
			} else {
				diff = yawcap + diff;
			}
			ps->delta_angles[YAW] += ANGLE2SHORT( diff );
			if ( diff <= 0 ) {
				ps->viewangles[YAW] = SHORT2ANGLE( ANGLE2SHORT( ps->proneDirection + yawcap ) );
			} else {
				ps->viewangles[YAW] = SHORT2ANGLE( ANGLE2SHORT( ps->proneDirection - yawcap ) );
			}
		}

		diff = AngleNormalize180( ps->proneTorsoPitch - ps->viewangles[PITCH] );
		if ( diff > 45.0f || diff < -45.0f ) {
			if ( diff <= 0 ) {
				diff = diff + 45.0f;
			} else {
				diff = diff - 45.0f;
			}
			ps->delta_angles[PITCH] += ANGLE2SHORT( diff );
			if ( diff <= 0 ) {
				ps->viewangles[PITCH] = AngleNormalize180( ps->proneTorsoPitch + 45.0f );
			} else {
				ps->viewangles[PITCH] = AngleNormalize180( ps->proneTorsoPitch - 45.0f );
			}
		}
	}

	if ( ps->pm_type != PM_UFO && ps->pm_type != PM_NOCLIP && ps->pm_type != PM_SPECTATOR ) {
		PM_UpdateLean( ps, cmd, trace );
	}
}

/*
================
PM_UpdatePronePitch

Track the ground under a prone body: the pitch it is lying at and the pitch the
torso should point down.
================
*/
static void PM_UpdatePronePitch( void ) {
	float target;
	float diff;
	float step;

	if ( !( pm->ps->pm_flags & PMF_PRONE ) ) {
		return;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		if ( !BG_CheckProneValid( pm->ps->clientNum, pm->ps->origin, pm->ps->maxs[0], 30.0f,
								  pm->ps->proneDirection,
								  &pm->ps->fTorsoHeight, &pm->ps->fTorsoPitch,
								  &pm->ps->fWaistPitch, qtrue, qfalse,
								  pml.groundPlane ? pml.groundTrace.normal : NULL,
								  pm->trace3, pm->trace2, qfalse ) ) {
			PM_AddEvent( EV_STANCE_FORCE_CROUCH );
			pm->ps->pm_flags |= PMF_PRONE_BLOCKED;
		}
	} else if ( pml.groundPlane && pml.groundTrace.normal[2] < MIN_WALK_NORMAL ) {
		PM_AddEvent( EV_STANCE_FORCE_CROUCH );
	}

	if ( pml.groundPlane ) {
		target = PitchOfVectorAlongYaw( pm->ps->proneDirection, pml.groundTrace.normal );
	} else {
		target = 0;
	}

	diff = AngleNormalize180( target - pm->ps->proneDirectionPitch );
	if ( diff != 0 ) {
		step = pml.frametime * 70.0f;
		if ( PM_FloatAbs( diff ) > step ) {
			diff = (float)PM_FloatSign( diff ) * step;
		}
		pm->ps->proneDirectionPitch = diff + pm->ps->proneDirectionPitch;
		pm->ps->proneDirectionPitch = AngleNormalize180( pm->ps->proneDirectionPitch );
	}

	if ( pml.groundPlane ) {
		target = PitchOfVectorAlongYaw( pm->ps->viewangles[YAW], pml.groundTrace.normal );
	} else {
		target = 0;
	}

	diff = AngleNormalize180( target - pm->ps->proneTorsoPitch );
	if ( diff != 0 ) {
		step = pml.frametime * 70.0f;
		if ( PM_FloatAbs( diff ) > step ) {
			diff = (float)PM_FloatSign( diff ) * step;
		}
		pm->ps->proneTorsoPitch = diff + pm->ps->proneTorsoPitch;
		pm->ps->proneTorsoPitch = AngleNormalize180( pm->ps->proneTorsoPitch );
	}
}

/*
================
PM_SetProneMovementOverride
================
*/
void PM_SetProneMovementOverride( void ) {
	if ( pm->ps->pm_flags & PMF_PRONE ) {
		pm->ps->pm_flags |= PMF_PRONE_MOVEOVERRIDE;
	}
}

/*
================
PM_UpdatePlayerWalkingFlag

Holding the aim-down-sight button walks, unless prone or reloading.
================
*/
static void PM_UpdatePlayerWalkingFlag( void ) {
	pm->ps->pm_flags &= ~PMF_WALKING;

	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	if ( !( pm->cmd.buttons & BUTTON_ADS ) ) {
		return;
	}
	if ( pm->ps->pm_flags & PMF_PRONE ) {
		return;
	}
	if ( !( pm->ps->pm_flags & PMF_ADS ) ) {
		return;
	}
	if ( pm->ps->weaponstate == WEAPON_RELOADING
		 || pm->ps->weaponstate == WEAPON_RELOAD_START
		 || pm->ps->weaponstate == WEAPON_RELOAD_END
		 || pm->ps->weaponstate == WEAPON_RELOAD_START_INTERUPT
		 || pm->ps->weaponstate == WEAPON_RELOADING_INTERUPT ) {
		return;
	}

	pm->ps->pm_flags |= PMF_WALKING;
}

/*
==============
PM_CheckLadderMove

Checks to see if we are on a ladder
==============
*/
static void PM_CheckLadderMove( void ) {
	vec3_t spot;
	vec3_t flatforward;
	vec3_t mins, maxs;
	trace_t trace;
	float tracedist;

	if ( pm->ps->pm_time ) {
		return;
	}

	if ( pml.walking ) {
		tracedist = 8.0f;
	} else {
		tracedist = 30.0f;
	}

	if ( ( pm->ps->pm_flags & PMF_LADDER ) && pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// keep looking at the ladder we are already on
		flatforward[0] = -pm->ps->vLadderVec[0];
		flatforward[1] = -pm->ps->vLadderVec[1];
		flatforward[2] = -pm->ps->vLadderVec[2];
	} else {
		flatforward[0] = pml.forward[0];
		flatforward[1] = pml.forward[1];
		flatforward[2] = 0;
		VectorNormalize( flatforward );
	}

	pm->ps->pm_flags &= ~PMF_LADDER;

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	if ( PM_GetEffectiveStance( pm->ps ) == 1 ) {
		return;
	}
	if ( pm->cmd.serverTime - pm->ps->jumpTime < pm_ladderJumpTime ) {
		return;
	}

	mins[0] = pm->mins[0] + 6.0f;
	mins[1] = pm->mins[1] + 6.0f;
	mins[2] = 8.0f;
	maxs[0] = pm->maxs[0] - 6.0f;
	maxs[1] = pm->maxs[1] - 6.0f;
	maxs[2] = pm->maxs[2];
	if ( maxs[2] < 8.0f ) {
		maxs[2] = 8.0f;
	}

	VectorMA( pm->ps->origin, tracedist, flatforward, spot );
	pm->trace( &trace, pm->ps->origin, mins, maxs, spot, pm->ps->clientNum, pm->tracemask );

	if ( trace.fraction < 1.0f && ( trace.surfaceFlags & SURF_LADDER ) ) {
		if ( !pml.walking || pm->cmd.forwardmove > 0 ) {
			VectorCopy( trace.normal, pm->ps->vLadderVec );
			pm->ps->pm_flags |= PMF_LADDER;
		}
	}
}

/*
============
PM_LadderMove
============
*/
static void PM_LadderMove( void ) {
	float wishspeed, scale;
	vec3_t wishdir, wishvel;
	vec3_t ladder_right;
	vec3_t rt;
	float upscale;
	float dot, drop;
	float push;
	int movementDir;

	if ( PM_CheckJump() ) {
		PM_AirMove();
		pm->ps->jumpTime = pm->cmd.serverTime;
		return;
	}

	upscale = ( pml.forward[2] + 0.25f ) * 2.5f;
	if ( upscale > 1.0f ) {
		upscale = 1.0f;
	} else if ( upscale < -1.0f ) {
		upscale = -1.0f;
	}

	// forward/right should be horizontal only
	pml.forward[2] = 0;
	VectorNormalize( pml.forward );
	pml.right[2] = 0;
	VectorNormalize2( pml.right, ladder_right );

	// the strafe axis is the part of right that lies in the ladder plane
	ProjectPointOntoVector( pml.right, ladder_right, pm->ps->vLadderVec );

	scale = PM_CmdScale( &pm->cmd );

	VectorClear( wishvel );

	if ( pm->cmd.forwardmove ) {
		wishvel[2] = (float)pm->cmd.forwardmove * pm_ladderScale * scale * upscale;
	}

	if ( pm->cmd.rightmove ) {
		// strafe, so we can jump off the ladder
		drop = (float)pm->cmd.rightmove * scale * 0.2f;
		wishvel[0] = drop * pml.right[0];
		wishvel[1] = drop * pml.right[1];
		wishvel[2] = drop * pml.right[2] + wishvel[2];
	}

	wishspeed = VectorNormalize2( wishvel, wishdir );

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	if ( !pm->cmd.forwardmove ) {
		if ( pm->ps->velocity[2] > 0 ) {
			pm->ps->velocity[2] -= (float)pm->ps->gravity * pml.frametime;
			if ( pm->ps->velocity[2] < 0 ) {
				pm->ps->velocity[2] = 0;
			}
		} else {
			pm->ps->velocity[2] += (float)pm->ps->gravity * pml.frametime;
			if ( pm->ps->velocity[2] > 0 ) {
				pm->ps->velocity[2] = 0;
			}
		}
	}

	if ( !pm->cmd.rightmove ) {
		// sideways friction along the ladder
		rt[0] = pml.right[0];
		rt[1] = pml.right[1];
		VectorNormalize2D( rt );
		dot = rt[0] * pm->ps->velocity[0] + rt[1] * pm->ps->velocity[1];
		if ( dot != 0 ) {
			pm->ps->velocity[0] += -dot * rt[0];
			pm->ps->velocity[1] += -dot * rt[1];
			drop = pm_ladderfriction * pml.frametime * dot;
			if ( PM_FloatAbs( dot ) > PM_FloatAbs( drop ) ) {
				if ( PM_FloatAbs( drop ) < 1.0f ) {
					drop = (float)PM_FloatSign( drop );
				}
				pm->ps->velocity[0] += ( dot - drop ) * rt[0];
				pm->ps->velocity[1] += ( dot - drop ) * rt[1];
			}
		}
	}

	if ( !pml.walking ) {
		// hold on to the ladder
		dot = pm->ps->vLadderVec[0] * pm->ps->velocity[0]
			  + pm->ps->vLadderVec[1] * pm->ps->velocity[1];
		pm->ps->velocity[0] += -dot * pm->ps->vLadderVec[0];
		pm->ps->velocity[1] += -dot * pm->ps->vLadderVec[1];

		push = wishvel[2] > 0 ? -500.0f : -250.0f;
		pm->ps->velocity[0] += push * pm->ps->vLadderVec[0];
		pm->ps->velocity[1] += push * pm->ps->vLadderVec[1];
	}

	PM_StepSlideMove( qfalse );     // no gravity while going up ladder

	// always point the legs at the ladder
	movementDir = (int)AngleNormalize180( vectoyaw( pm->ps->vLadderVec ) + 180.0f
										  - pm->ps->viewangles[YAW] );
	if ( abs( movementDir ) > 75 ) {
		movementDir = movementDir <= 0 ? -75 : 75;
	}
	pm->ps->movementDir = (signed char)movementDir;
}

/*
================
PmoveSingle
================
*/
void PmoveSingle( pmove_t *pmove ) {
	vec3_t delta;

	BG_AnimUpdatePlayerStateConditions( pmove );

	pmove->watertype = 0;
	pmove->waterlevel = 0;

	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint for the previous frame
	c_pmove++;

	if ( pm->ps->pm_flags & PMF_IGNORE_INPUT ) {
		pm->cmd.buttons = 0;
		pm->cmd.wbuttons &= ( WBUTTON_CROUCH | WBUTTON_PRONE | WBUTTON_NOSTANCEEVENT );
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	pm->ps->pm_flags &= ~PMF_PRONE_BLOCKED;

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->tracemask &= ~CONTENTS_BODY;    // corpses can fly through bodies
	}

	// a prone player who pushes harder on the stick than last frame breaks out
	// of whatever the weapon was doing
	if ( pm->ps->pm_flags & PMF_PRONE ) {
		if ( ( pm->cmd.forwardmove != pm->oldcmd.forwardmove
			   && PM_FloatAbs( (float)pm->oldcmd.forwardmove )
				  < PM_FloatAbs( (float)pm->cmd.forwardmove ) )
			 || ( pm->cmd.rightmove != pm->oldcmd.rightmove
				  && PM_FloatAbs( (float)pm->oldcmd.rightmove )
					 < PM_FloatAbs( (float)pm->cmd.rightmove ) ) ) {
			if ( PM_InteruptWeaponWithProneMove() ) {
				pm->ps->pm_flags &= ~PMF_PRONE_MOVEOVERRIDE;
				pm->ps->pm_flags &= ~PMF_ADS;
			}
		} else if ( !( pm->ps->pm_flags & PMF_ADS )
					&& ( pm->ps->weaponstate <= WEAPON_DROPPING
						 || pm->ps->weaponstate == WEAPON_RELOADING ) ) {
			pm->ps->pm_flags &= ~PMF_PRONE_MOVEOVERRIDE;
		}
	} else {
		pm->ps->pm_flags &= ~PMF_PRONE_MOVEOVERRIDE;
	}

	pml.weaponInfo = bg_weaponInfo[pm->ps->weapon];

	if ( ( pm->ps->pm_flags & PMF_ADS ) && ( pm->ps->pm_flags & PMF_PRONE ) ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
	}

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) {
		pm->ps->eFlags |= EF_TALK;
	} else {
		pm->ps->eFlags &= ~EF_TALK;
	}

	// set the firing flag for continuous beam weapons
	pm->ps->eFlags &= ~EF_FIRING;
	if ( pm->ps->pm_type != PM_INTERMISSION && !( pm->ps->pm_flags & PMF_RESPAWNED ) ) {
		if ( pm->ps->weaponstate == WEAPON_READY || pm->ps->weaponstate == WEAPON_FIRING ) {
			if ( pm->ps->ammoclip[ bg_weaponInfo[pm->ps->weapon]->clipIndex ] ) {
				if ( ( pm->cmd.buttons & BUTTON_ATTACK ) && !( pm->cmd.buttons & BUTTON_TALK ) ) {
					pm->ps->eFlags |= EF_FIRING;
				}
			}
		}
	}

	// clear the respawned flag if attack is cleared
	if ( pm->ps->pm_type < PM_DEAD && !( pm->cmd.buttons & BUTTON_ATTACK ) ) {
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pm->cmd.buttons & BUTTON_TALK ) {
		pm->cmd.buttons = BUTTON_TALK;
		pm->cmd.wbuttons &= ( WBUTTON_CROUCH | WBUTTON_PRONE | WBUTTON_NOSTANCEEVENT );
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset( &pml, 0, sizeof( pml ) );

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy( pm->ps->origin, pml.previous_origin );

	// save old velocity for crashlanding
	VectorCopy( pm->ps->velocity, pml.previous_velocity );

	pml.frametime = pml.msec * 0.001f;

	pml.weaponInfo = bg_weaponInfo[pm->ps->weapon];

	PM_AdjustAimSpreadScale();

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd, pm->trace3 );

	AngleVectors( pm->ps->viewangles, pml.forward, pml.right, pml.up );

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || pm->cmd.rightmove ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if ( ( pm->ps->pm_flags & PMF_PRONE ) && ( pm->ps->pm_flags & PMF_PRONE_MOVEOVERRIDE ) ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
	}

	switch ( pm->ps->pm_type ) {
	case PM_SPECTATOR:
		PM_UpdateAimDownSightFlag();
		PM_UpdatePlayerWalkingFlag();
		PM_CheckDuck();
		PM_FlyMove();
		PM_DropTimers();
		return;

	case PM_NOCLIP:
		PM_UpdateAimDownSightFlag();
		PM_UpdatePlayerWalkingFlag();
		PM_NoclipMove();
		PM_DropTimers();
		return;

	case PM_UFO:
		PM_UpdateAimDownSightFlag();
		PM_UpdatePlayerWalkingFlag();
		PM_UFOMove();
		PM_DropTimers();
		return;

	case PM_NORMAL_LINKED:
	case PM_DEAD_LINKED:
		// the mover owns the origin
		pm->ps->pm_flags &= ~PMF_LADDER;
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		PM_UpdateAimDownSightFlag();
		PM_UpdatePlayerWalkingFlag();
		PM_CheckDuck();
		PM_DropTimers();
		/* Update linked players too; PM_Footsteps leaves dead players alone. */
		PM_Footsteps();
		PM_Weapon();
		return;

	case PM_INTERMISSION:
		return;     // no movement at all

	default:
		break;
	}

	if ( pm->ps->eFlags & EF_TURRET_ACTIVE_MASK ) {
		// a mounted weapon holds us in place
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		PM_UpdateAimDownSightFlag();
		PM_UpdatePlayerWalkingFlag();
		PM_CheckDuck();
		PM_DropTimers();
		PM_Footsteps();
		return;
	}

	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	PM_CheckDuck();

	// set groundentity
	PM_GroundTrace();

	PM_UpdateAimDownSightFlag();
	PM_UpdatePlayerWalkingFlag();
	PM_UpdatePronePitch();

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove();
	}

	PM_CheckLadderMove();

	PM_DropTimers();

	if ( pm->ps->pm_flags & PMF_LADDER ) {
		PM_LadderMove();
	} else if ( pml.walking ) {
		// walking on ground
		PM_WalkMove();
	} else {
		// airborne
		PM_AirMove();
	}

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();

	// weapons
	PM_Weapon();

	// footstep events / legs animations
	PM_Footsteps();

	PM_FoliageSounds();

	// entering / leaving water splashes
	PM_WaterEvents();

	// if the move came out a lot shorter than the velocity asked for, take the
	// velocity from the move that actually happened
	VectorSubtract( pm->ps->origin, pml.previous_origin, delta );
	if ( DotProduct( pm->ps->velocity, pm->ps->velocity ) * 0.25f
		 > DotProduct( delta, delta ) / ( pml.frametime * pml.frametime ) ) {
		pm->ps->velocity[0] = 1.0f / pml.frametime * delta[0];
		pm->ps->velocity[1] = 1.0f / pml.frametime * delta[1];
		pm->ps->velocity[2] = 1.0f / pml.frametime * delta[2];
	}

	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector( pm->ps->velocity );
}

/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove( pmove_t *pmove ) {
	int finalTime;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;     // should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	pmove->numtouch = 0;

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		} else {
			if ( msec > 66 ) {
				msec = 66;
			}
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle( pmove );

		if ( pmove->ps->pm_flags & PMF_JUMP_HELD ) {
			pmove->cmd.upmove = 20;
		}
	}
}

/*
================
BG_GetSpeed

The speed the animation system and the HUD should see: the climb rate on a
ladder, the ground speed otherwise.
================
*/
float BG_GetSpeed( const playerState_t *ps, int time ) {
	if ( !( ps->pm_flags & PMF_LADDER ) ) {
		return (float)sqrt( (double)( ps->velocity[0] * ps->velocity[0]
									  + ps->velocity[1] * ps->velocity[1] ) );
	}
	if ( time - ps->jumpTime < 500 ) {
		return 0;
	}
	return ps->velocity[2];
}
