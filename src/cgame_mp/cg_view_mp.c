/*
 * cg_view.c -- setup all the parameters (position, angle, etc) for a 3D
 * rendering.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_view.c is the ancestor: CG_CalcVrect,
 * CG_OffsetThirdPersonView, CG_StepOffset, CG_OffsetFirstPersonView,
 * CG_CalcFov, CG_CalcViewValues and CG_DrawActiveFrame are all still
 * recognisably its text.  What CoD changed:
 *
 *   - the view angles come out of bg_weapon.c.  CG_OffsetFirstPersonView fills
 *     a bgViewAngleState_t and lets BG_CalculateView_DamageKick /
 *     BG_CalculateView_Velocity do the damage kick and the roll, so RTCW's
 *     CG_DamageBlendBlob / bobfracsin / CG_ZoomUp_f block is gone.
 *   - the bob is CoD's, driven by cg_bobAmplitude{Standing,Ducked,Prone} and
 *     ps.viewHeightTarget through the two helpers the Mac symbols never named
 *     (sub_30032A10 vertical, sub_30032A90 horizontal).
 *   - the prone/lean camera: AddLeanToPosition shifts the eye sideways and the
 *     eye is finally clamped to ps.origin[2] + 8.
 *   - shellshock: CG_DrawActiveFrame pulls ps.shellshock{Index,Time,Duration}
 *     into cg.shellshock and runs CG_UpdateShellShock before the view is built.
 *   - the mounted-weapon camera (CG_CalcTurretViewValues) rides the turret's
 *     tag_player bone, and a cubemap-shot path (CG_CalcCubemapViewValues)
 *     replaces the whole refdef with one of six axis-aligned faces.
 *   - the FX test console commands and the skybox portal are CoD additions.
 *
 * Functions are in address order, which is the original file order.
 * CG_CalcFov has no function of its own in retail: the compiler outlined
 * CG_CalcViewValues' tail to 0x30032F40, between CG_GetViewFov and
 * CG_CalcCubemapViewValues.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "cg_local.h"
#include "../game_mp/bg_public.h"

#define PITCH                   0
#define YAW                     1
#define ROLL                    2

#ifndef M_PI
#define M_PI                    3.14159265358979323846f
#endif

/* CG_OffsetThirdPersonView's focus ray (0x30032676). */
#define FOCUS_DISTANCE          512

/* CG_StepOffset / EV_STEP_VIEW (0x3003287C). */
#define STEP_TIME               100

/* CG_OffsetFirstPersonView's land deflection (0x30032D7F, 0x30032D99). */
#define LAND_DEFLECT_TIME       150
#define LAND_RETURN_TIME        300

/* CG_Trace's mask at both third-person calls (0x30032702); CONTENTS_WATER is
   CG_CalcFov's (0x30032F6B). */
#define MASK_SOLID              0x11
#define CONTENTS_WATER          0x20

/* refdef_t::rdflags, Q3's bits: CG_CalcFov toggles RDF_UNDERWATER and
   CG_DrawSkyBoxPortal sets RDF_NOWORLDMODEL|RDF_SKYBOXPORTAL. */
#define RDF_NOWORLDMODEL        0x0001
#define RDF_SKYBOXPORTAL        0x0008
#define RDF_DRAWSKYBOX          0x0010
#define RDF_UNDERWATER          0x0020

/* CG_CalcVrect / CG_GetViewFov gate on this pm_type (0x30032446, 0x30032E28). */
#define PM_INTERMISSION         5
/* CG_OffsetThirdPersonView / CG_DrawActiveFrame's "dead or spectating" test
   (0x3003256D, 0x30033D39). */
#define PM_DEAD_OR_SPECTATING   6

/* cg_ent_mp.c spells the 0x50000 pm_flags mask PMF_VIEWLOCKED; CG_DrawActiveFrame
   tests just the high bit of it (0x30033D4F). */
#define PMF_TURRET              0x00040000
/* ps.eFlags bits CG_OffsetFirstPersonView / CG_GetViewFov / CG_CalcTurretViewValues
   read as "riding a mounted weapon" (0x30032BB4, 0x30032F32, 0x30033220). */
#define EF_MOUNTED              0x0000C000
/* ps.pm_flags bit CG_CalcViewValues uses to pick the jump speed (0x30033485). */
#define PMF_LADDER              0x00000010

#define ENTITYNUM_NONE_CG       ( MAX_GENTITIES - 1 )

#define CS_SERVERINFO           0
/* CG_DrawSkyBoxPortal reads configstring 10, not 12.  0x30033643 is
   `mov eax, cgs.gameState.stringOffsets+28h` -- byte offset 0x28 over
   4-byte entries, so index 10.  RTCW spells it CS_SKYBOXORG (bg_public.h:170,
   34 there; CoD renumbered it) and cg_view.c:1318 reads it the same way.
   CS_FOGVARS (12) is the FOG description -- a different string with a
   different token count, and parsing it here fails with "error parsing
   skybox configstring" on any map that sets a skybox. */
#define CS_SKYBOXORG            10

/* RTCW's fogType_t slot for the portal sky scene; both trap_R_SetFog calls in
   CG_DrawSkyBoxPortal push it first (0x30033857, 0x30033882). */
#define FOG_PORTALVIEW          2

/* snapshot_t::snapFlags; Q3's bit, tested at 0x30033C26. */
#define SNAPFLAG_NOT_ACTIVE     2


/* CG_AddLagometerFrameInfo's ring is 128 deep (0x30033A07). */
#define LAG_SAMPLES             128

/* CG_DrawActiveFrame walks cvarTable's 173 entries (0x30033A2A). */
#define CVARTABLE_COUNT         173

/*
 * The /fxTest console command's state, 84 bytes at cg+0x2A998 inside
 * cg_local.h's unknown_0x2A998 hole (CG_FxTest 0x30032399..0x300323E4).
 */
#define cg_fxTestName       ( (char *)&cg.unknown_0x2A998[0] )      /* cg+0x2A998, 64 bytes */
#define cg_fxTestPos        ( (float *)&cg.unknown_0x2A998[64] )    /* cg+0x2A9D8 */
#define cg_fxTestTime       ( *(int *)&cg.unknown_0x2A998[76] )     /* cg+0x2A9E4 */
#define cg_fxTestRestart    ( *(int *)&cg.unknown_0x2A998[80] )     /* cg+0x2A9E8 */

/* The rest of the cg_t slots this unit touches that cg_local.h carries as
   holes. */
#define cg_predictedErrorTime   ( *(int *)&cg.unknown_0x27348[0] )      /* cg+0x27348 */
#define cg_adsZoomToggle        ( *(int *)&cg.unknown_0x27314[4] )      /* cg+0x27318, CG_GetViewFov */
#define cg_fovScale             ( *(float *)&cg.unknown_0x294B4[0] )    /* cg+0x294B4 = fov_x / cg_fov */
#define cg_bobCycle             ( *(float *)&cg.unknown_0x2A8AC[0] )    /* cg+0x2A8AC */
#define cg_xyspeed              ( *(float *)&cg.unknown_0x2A8AC[4] )    /* cg+0x2A8B0 */
#define cg_cinematicCamera      ( *(int *)&cg.unknown_0x2AA40[0x5C] )   /* cg+0x2AA9C */

/* Owned by other units. */
void        CG_Error( const char *msg, ... );                   /* cg_main.c 0x30020750 */
void        Com_Error( int level, const char *fmt, ... );       /* cg_main.c 0x30020830 */
void        CG_AddLagometerFrameInfo( void );                   /* cg_draw.c 0x300153C0 */
void        CG_DrawActive( int stereoView );                    /* cg_draw.c 0x30018940 */
void        CG_ShakeCamera( void );                             /* cg_shellshock.c 0x30017F00 */
void        CG_PerturbCamera( void );                           /* cg_shellshock.c 0x3002E7A0 */
/* CG_UpdateShellShock, CG_PerturbCamera, CG_AddViewWeapon and
   CG_PlayBufferedVoiceChats come in from cg_local.h. */

/* cg_effects_mp.c / cg_localents_mp.c / cg_marks_mp.c */
void        CG_AddMarks( void );
void        CG_AddLocalEntities( void );

/*
 * cg_info.c's loading-screen re-entrancy guard.  The binary shares one dword
 * at 0x300EEF28 between CG_DrawInformation, CG_DrawActive and this unit, so it
 * is NOT static.
 */
extern int  cg_drawingInformation;

/* game_mp/bg_weapon.c -- bg_public.h has no prototype block for these two. */
void        BG_CalculateView_DamageKick( bgViewAngleState_t *state, vec3_t angles );
void        BG_CalculateView_Velocity( bgViewAngleState_t *state, vec3_t angles );

/* universal/com_math.c, universal/q_parse.c */
void        AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );

/*
 * bg_animation.c's shared time, compiled into this DLL as well as into the
 * game DLL -- cg_main_mp.c declares the same two.
 */
extern int  bgs_time;                                           /* 0x300F0310 */
extern int  bgs_frametime;                                      /* 0x300F0318 */

/* CG_RegisterCvars' table; CG_DrawActiveFrame updates all 173 every frame. */
typedef struct cvarTable_t {
	vmCvar_t    *vmCvar;
	char        *cvarName;
	char        *defaultString;
	int         cvarFlags;
} cvarTable_t;

extern cvarTable_t cvarTable[];                                 /* 0x300749A0 */

/* CG_DrawSkyBoxPortal only ever wants the fog set once (0x300EEF18). */
static int skyPortalFogSet;

/*
=================
CG_FxSetTestPosition     0x300322C0
=================
*/
void CG_FxSetTestPosition( void ) {
	cg_fxTestPos[0] = cg.refdef.viewaxis[0][0] * 100.0f + cg.refdef.vieworg[0];
	cg_fxTestPos[1] = cg.refdef.viewaxis[0][1] * 100.0f + cg.refdef.vieworg[1];
	cg_fxTestPos[2] = cg.refdef.viewaxis[0][2] * 100.0f + cg.refdef.vieworg[2];

	Com_Printf( "\n\nFX Testing position set to: (%f, %f, %f)\n\n",
				cg_fxTestPos[0], cg_fxTestPos[1], cg_fxTestPos[2] );
}

/*
=================
CG_FxRestart     0x30032330
=================
*/
void CG_FxRestart( void ) {
	Com_Printf( "FX Restarting so off-line changes are loaded.\n" );
	trap_syscall_0xE8();
	trap_syscall_0xE7();
}

/*
=================
CG_FxTest     0x30032360
=================
*/
void CG_FxTest( void ) {
	int effect;

	if ( trap_Argc() < 2 ) {
		Com_Printf( "Must supply filename from base path.  Optional restart time.\n" );
	}

	strncpy( cg_fxTestName, CG_Argv( 1 ), 63 );
	cg_fxTestName[63] = 0;

	effect = trap_syscall_0xDE( cg_fxTestName );
	Com_Printf( "Spawning Fx %s with ID: %d\n", cg_fxTestName, effect );
	trap_syscall_0xE3( effect, (int)cg_fxTestPos );

	cg_fxTestTime = cg.time;

	if ( trap_Argc() == 3 ) {
		cg_fxTestRestart = (int)( atof( CG_Argv( 2 ) ) * 1000.0 );
	} else {
		cg_fxTestRestart = 0;
	}
}

/*
=================
CG_CalcVrect     0x30032440

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect( void ) {
	int size;
	int xsize, ysize;

	// the intermission should allways be full screen
	if ( cg.nextSnap->ps.pm_type == PM_INTERMISSION ) {
		xsize = 100;
		ysize = 100;
	} else {
		size = cg_viewsize.integer;
		if ( size < 30 ) {
			trap_Cvar_Setvar( &cg_viewsize, "30" );
			xsize = 30;
			ysize = 30;
		} else if ( size > 100 ) {
			trap_Cvar_Setvar( &cg_viewsize, "100" );
			xsize = 100;
			ysize = 100;
		} else {
			xsize = size;
			ysize = size;
		}
	}

	if ( cg_letterbox.integer ) {
		xsize = (int)( xsize * 0.85f );
	}

	cg.refdef.width = cgs.glconfig.vidWidth * ysize / 100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight * xsize / 100;
	cg.refdef.height &= ~1;

	cg.refdef.x = ( cgs.glconfig.vidWidth - cg.refdef.width ) / 2;
	cg.refdef.y = ( cgs.glconfig.vidHeight - cg.refdef.height ) / 2;
}

/*
===============
CG_OffsetThirdPersonView     0x30032540
===============
*/
static void CG_OffsetThirdPersonView( void ) {
	vec3_t          forward, right, up;
	vec3_t          view;
	vec3_t          focusAngles;
	trace_t         trace;
	static vec3_t   mins = { -4, -4, -4 };                  /* 0x3007487C */
	static vec3_t   maxs = { 4, 4, 4 };                     /* 0x30074888 */
	vec3_t          focusPoint;
	float           focusDist;

	cg.refdef.vieworg[2] += cg.predictedPlayerState.viewHeightCurrent;

	focusAngles[PITCH] = cg.refdefViewAngles[PITCH];
	focusAngles[YAW] = cg.refdefViewAngles[YAW];

	// if dead, look at killer
	if ( cg.predictedPlayerState.pm_type >= PM_DEAD_OR_SPECTATING ) {
		focusAngles[YAW] = (float)cg.predictedPlayerState.stats[1];
		cg.refdefViewAngles[YAW] = (float)cg.predictedPlayerState.stats[1];
	}

	if ( focusAngles[PITCH] > 45 ) {
		focusAngles[PITCH] = 45;        // don't go too far overhead
	}
	AngleVectors( focusAngles, forward, NULL, NULL );

	VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );

	VectorCopy( cg.refdef.vieworg, view );
	view[2] += 8;

	cg.refdefViewAngles[PITCH] *= 0.5f;
	cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;

	AngleVectors( cg.refdefViewAngles, forward, right, up );

	VectorMA( view, -cg_thirdPersonRange.value, forward, view );

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near
	// clipping anything
	CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view,
			  cg.predictedPlayerState.clientNum, MASK_SOLID );

	if ( trace.fraction != 1.0f ) {
		VectorCopy( trace.endpos, view );
		view[2] += ( 1.0f - trace.fraction ) * 32;
		// try another trace to this position, because a tunnel may have the
		// ceiling close enough that this is poking out
		CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view,
				  cg.predictedPlayerState.clientNum, MASK_SOLID );
		VectorCopy( trace.endpos, view );
	}

	VectorCopy( view, cg.refdef.vieworg );

	// select pitch to look at focus point from vieword
	VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
	focusDist = (float)sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1 ) {
		focusDist = 1;      // should never happen
	}
	cg.refdefViewAngles[PITCH] = (float)( -180 / M_PI * atan2( focusPoint[2], focusDist ) );
}

/*
===============
CG_StepOffset     0x30032860

Smooth out stair climbing
===============
*/
static void CG_StepOffset( void ) {
	int timeDelta;

	timeDelta = cg.time - cg.stepTime;
	if ( timeDelta < 0 ) {
		cg.stepTime = cg.time;
	}
	if ( timeDelta < STEP_TIME ) {
		cg.refdef.vieworg[2] -= cg.stepChange * ( STEP_TIME - timeDelta ) * 0.01f;
	}
}

/*
===============
CG_KickAngles     0x300328B0

Integrates the mounted-weapon kick in 5 ms steps.  cg.kick_angles is the
velocity and cg.kick_origin the angle it drives; both are cleared by
CG_DrawActiveFrame whenever the player is not on a turret.
===============
*/
static void CG_KickAngles( void ) {
	int     msec;
	int     i;
	float   step;
	float   dir;
	float   accel;
	float   delta;
	float   v;

	for ( msec = cg.frametime ; msec > 0 ; msec -= 5 ) {
		step = ( msec > 5 ? 5 : msec ) * 0.001f;

		for ( i = 0 ; i < 3 ; i++ ) {
			if ( cg.kick_angles[i] == 0.0f && cg.kick_origin[i] == 0.0f ) {
				continue;
			}

			if ( cg.kick_origin[i] != 0.0f ) {
				if ( cg.kick_origin[i] > 0.0f ) {
					dir = -1.0f;
				} else {
					dir = 1.0f;
				}

				if ( cg.predictedPlayerState.weapon ) {
					if ( cg.predictedPlayerState.fWeaponPosFrac > 0.5f ) {
						accel = dir * cg.weaponInfo->adsViewKickCenterSpeed;
					} else {
						accel = dir * cg.weaponInfo->hipViewKickCenterSpeed;
					}
				} else {
					accel = dir * 2400.0f;
				}

				cg.kick_angles[i] += accel * step;
			}

			delta = step * cg.kick_angles[i];
			if ( delta * cg.kick_origin[i] < 0.0f ) {
				delta *= 0.06f;
			}

			v = delta + cg.kick_origin[i];
			if ( v * cg.kick_origin[i] < 0.0f ) {
				cg.kick_origin[i] = 0.0f;
			} else {
				cg.kick_origin[i] = v;
				if ( v != 0.0f ) {
					if ( fabs( v ) <= 10.0 ) {
						continue;
					}
					if ( v > 0.0f ) {
						cg.kick_origin[i] = 10.0f;
					} else {
						cg.kick_origin[i] = -10.0f;
					}
				}
			}
			cg.kick_angles[i] = 0.0f;
		}
	}
}

/*
===============
sub_30032A10     0x30032A10

The vertical view bob.  Unnamed in the Mac symbol table, so the address stands
in for the name.
===============
*/
static float sub_30032A10( float bobCycle, float speed, float bobMax ) {
	float amplitude;
	float amount;

	if ( cg.predictedPlayerState.viewHeightTarget == cg.predictedPlayerState.proneViewHeight ) {
		amplitude = cg_bobAmplitudeProne.value;
	} else if ( cg.predictedPlayerState.viewHeightTarget == cg.predictedPlayerState.crouchViewHeight ) {
		amplitude = cg_bobAmplitudeDucked.value;
	} else {
		amplitude = cg_bobAmplitudeStanding.value;
	}

	amount = amplitude * speed;
	if ( amount > bobMax ) {
		amount = bobMax;
	}

	return (float)( ( sin( bobCycle * 4.0f + M_PI * 0.5f ) * 0.2 + sin( bobCycle + bobCycle ) )
					* amount * 0.75 );
}

/*
===============
sub_30032A90     0x30032A90

The horizontal view bob.  Unnamed in the Mac symbol table.
===============
*/
static float sub_30032A90( float bobCycle, float speed, float bobMax ) {
	float amplitude;
	float amount;

	if ( cg.predictedPlayerState.viewHeightTarget == cg.predictedPlayerState.proneViewHeight ) {
		amplitude = cg_bobAmplitudeProne.value;
	} else if ( cg.predictedPlayerState.viewHeightTarget == cg.predictedPlayerState.crouchViewHeight ) {
		amplitude = cg_bobAmplitudeDucked.value;
	} else {
		amplitude = cg_bobAmplitudeStanding.value;
	}

	amount = amplitude * speed;
	if ( amount > bobMax ) {
		amount = bobMax;
	}

	return (float)( amount * sin( bobCycle ) );
}

/*
===============
CG_OffsetFirstPersonView     0x30032AE0
===============
*/
static void CG_OffsetFirstPersonView( void ) {
	bgViewAngleState_t  viewState;
	vec3_t              angles;
	vec3_t              forward, right, up;
	float               bob;
	float               hbob;
	float               delta;
	float               f;
	float               floorZ;

	if ( cg.nextSnap->ps.pm_type == PM_INTERMISSION ) {
		return;
	}

	/* BG_CalculateViewAngles inlined (0x30012E80). */
	viewState.ps = &cg.predictedPlayerState;
	viewState.viewKickStartTime = cg.damageTime
								  ? cg.damageTime - cg.predictedPlayerState.deltaTime : 0;
	viewState.time = cg.time - cg.predictedPlayerState.deltaTime;
	viewState.viewKickPitch = cg.damageY;
	viewState.viewKickRoll = cg.damageX;
	viewState.speed = cg_xyspeed;

	angles[0] = 0.0f;
	angles[1] = 0.0f;
	angles[2] = 0.0f;
	BG_CalculateView_DamageKick( &viewState, angles );
	BG_CalculateView_Velocity( &viewState, angles );

	cg.refdefViewAngles[PITCH] += angles[PITCH];
	cg.refdefViewAngles[YAW] += angles[YAW];
	cg.refdefViewAngles[ROLL] += angles[ROLL];

	// add view height
	cg.refdef.vieworg[2] += cg.predictedPlayerState.viewHeightCurrent;

	// a mounted weapon owns the eye entirely
	if ( cg.predictedPlayerState.eFlags & EF_MOUNTED ) {
		return;
	}

	// add bob height
	bob = sub_30032A10( cg_bobCycle, cg_xyspeed, cg_bobMax.value );
	cg.refdef.vieworg[2] += bob;

	// and the sideways half of it, along the view right
	hbob = sub_30032A90( cg_bobCycle, cg_xyspeed, cg_bobMax.value );
	AngleVectors( cg.refdefViewAngles, forward, right, up );
	VectorMA( cg.refdef.vieworg, hbob, right, cg.refdef.vieworg );

	// add fall height
	delta = cg.time - cg.landTime;
	if ( delta < 0 ) {
		cg.landTime = cg.time - ( LAND_DEFLECT_TIME + LAND_RETURN_TIME );
	}
	if ( delta < LAND_DEFLECT_TIME ) {
		f = delta / LAND_DEFLECT_TIME;
		cg.refdef.vieworg[2] += cg.landChange * f;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		delta -= LAND_DEFLECT_TIME;
		f = 1.0f - ( delta / LAND_RETURN_TIME );
		cg.refdef.vieworg[2] += cg.landChange * f;
	}

	// smooth out stair climbing
	CG_StepOffset();

	// lean the eye out sideways
	AddLeanToPosition( cg.refdef.vieworg, cg.refdefViewAngles[YAW],
					   cg.predictedPlayerState.leanf, 16.0f, 20.0f );

	// never let the eye sink into the floor
	floorZ = cg.predictedPlayerState.origin[2] + 8.0f;
	if ( cg.refdef.vieworg[2] < floorZ ) {
		cg.refdef.vieworg[2] = floorZ;
	}
}

/*
====================
CG_GetViewFov     0x30032E20

The unzoomed fov, lerped towards the weapon's ADS fov by ps.fWeaponPosFrac.
====================
*/
static float CG_GetViewFov( void ) {
	float fov;
	float frac;
	float t;

	if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fov = 90;
		goto mounted;
	}

	fov = cg_fov.value;
	if ( fov < 80 ) {
		fov = 80;
	} else if ( fov > 160 ) {
		fov = 160;
	}

	if ( bg_weaponInfo[ cg.predictedPlayerState.weapon ]->aimDownSight ) {
		if ( cg.predictedPlayerState.fWeaponPosFrac == 1.0f ) {
			fov = cg.weaponInfo->adsZoomFov;
			goto mounted;
		}
		if ( cg.predictedPlayerState.fWeaponPosFrac != 0.0f ) {
			if ( cg_adsZoomToggle ) {
				t = cg.predictedPlayerState.fWeaponPosFrac
					- ( 1.0f - cg.weaponInfo->adsZoomInFrac );
				if ( t <= 0.0f ) {
					goto mounted;
				}
				frac = t / cg.weaponInfo->adsZoomInFrac;
			} else {
				t = cg.predictedPlayerState.fWeaponPosFrac
					- ( 1.0f - cg.weaponInfo->adsZoomOutFrac );
				if ( t <= 0.0f ) {
					goto mounted;
				}
				frac = t / cg.weaponInfo->adsZoomOutFrac;
			}
			if ( frac > 0.0f ) {
				fov = fov - ( fov - cg.weaponInfo->adsZoomFov ) * frac;
			}
		}
	}

mounted:
	if ( cg.predictedPlayerState.eFlags & EF_MOUNTED ) {
		return 55.0f;
	}
	return fov;
}

/*
====================
CG_CalcFov     0x30032F40

Fixed fov at intermissions, otherwise account for fov variable and zooms.
Returns qtrue if the view is underwater.

No function of its own in retail: the compiler outlined CG_CalcViewValues'
tail here.
====================
*/
static int CG_CalcFov( void ) {
	float   fov_x, fov_y;
	float   x;
	float   phase;
	float   v;
	int     inwater;

	fov_x = CG_GetViewFov();

	x = cg.refdef.width / (float)tan( fov_x * ( M_PI / 360.0f ) );
	fov_y = (float)( atan2( cg.refdef.height, x ) * ( 360.0f / M_PI ) );

	// warp if underwater
	if ( CG_PointContents( cg.refdef.vieworg, -1, CONTENTS_WATER ) ) {
		phase = cg.time * 0.0025132743f;
		v = (float)sin( phase );
		fov_x += v;
		fov_y -= v;
		inwater = qtrue;
	} else {
		inwater = qfalse;
	}

	if ( CG_PointContents( cg.refdef.vieworg, -1, CONTENTS_WATER ) ) {
		cg.refdef.rdflags |= RDF_UNDERWATER;
	} else {
		cg.refdef.rdflags &= ~RDF_UNDERWATER;
	}

	if ( CG_PointContents( cg.refdef.vieworg, -1, CONTENTS_WATER ) ) {
		cg.refdef.rdflags |= RDF_UNDERWATER;
	} else {
		cg.refdef.rdflags &= ~RDF_UNDERWATER;
	}

	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;

	cg_fovScale = fov_x / cg_fov.value;

	return inwater;
}

/*
===============
CG_CalcCubemapViewValues     0x30032FF0

/r_cubemapshot's six axis-aligned faces.  cg.unknown_0x00010 is the face
number and cg.unknown_0x00014 the edge length, both straight off vmMain.
===============
*/
static void CG_CalcCubemapViewValues( void ) {
	int size;

	size = cg.unknown_0x00014;

	cg.refdef.x = 0;
	cg.refdef.y = 0;
	cg.refdef.width = size + 2;
	cg.refdef.height = size + 2;

	cg.refdef.vieworg[0] = cg.predictedPlayerState.origin[0];
	cg.refdef.vieworg[1] = cg.predictedPlayerState.origin[1];
	cg.refdef.vieworg[2] = cg.predictedPlayerState.origin[2]
						   + cg.predictedPlayerState.viewHeightCurrent;

	cg.refdef.fov_x = (float)( atan2( size + 2, size ) * ( 360.0f / M_PI ) );
	cg.refdef.fov_y = cg.refdef.fov_x;

	switch ( cg.unknown_0x00010 ) {
	case 1:
		cg.refdef.viewaxis[0][0] = 0;   cg.refdef.viewaxis[0][1] = 0;   cg.refdef.viewaxis[0][2] = 1;
		cg.refdef.viewaxis[1][0] = 0;   cg.refdef.viewaxis[1][1] = 1;   cg.refdef.viewaxis[1][2] = 0;
		cg.refdef.viewaxis[2][0] = -1;  cg.refdef.viewaxis[2][1] = 0;   cg.refdef.viewaxis[2][2] = 0;
		break;
	case 2:
		cg.refdef.viewaxis[0][0] = 0;   cg.refdef.viewaxis[0][1] = 0;   cg.refdef.viewaxis[0][2] = -1;
		cg.refdef.viewaxis[1][0] = 0;   cg.refdef.viewaxis[1][1] = 1;   cg.refdef.viewaxis[1][2] = 0;
		cg.refdef.viewaxis[2][0] = 1;   cg.refdef.viewaxis[2][1] = 0;   cg.refdef.viewaxis[2][2] = 0;
		break;
	case 3:
		cg.refdef.viewaxis[0][0] = -1;  cg.refdef.viewaxis[0][1] = 0;   cg.refdef.viewaxis[0][2] = 0;
		cg.refdef.viewaxis[1][0] = 0;   cg.refdef.viewaxis[1][1] = -1;  cg.refdef.viewaxis[1][2] = 0;
		cg.refdef.viewaxis[2][0] = 0;   cg.refdef.viewaxis[2][1] = 0;   cg.refdef.viewaxis[2][2] = 1;
		break;
	case 4:
		cg.refdef.viewaxis[0][0] = 1;   cg.refdef.viewaxis[0][1] = 0;   cg.refdef.viewaxis[0][2] = 0;
		cg.refdef.viewaxis[1][0] = 0;   cg.refdef.viewaxis[1][1] = 1;   cg.refdef.viewaxis[1][2] = 0;
		cg.refdef.viewaxis[2][0] = 0;   cg.refdef.viewaxis[2][1] = 0;   cg.refdef.viewaxis[2][2] = 1;
		break;
	case 5:
		cg.refdef.viewaxis[0][0] = 0;   cg.refdef.viewaxis[0][1] = -1;  cg.refdef.viewaxis[0][2] = 0;
		cg.refdef.viewaxis[1][0] = 1;   cg.refdef.viewaxis[1][1] = 0;   cg.refdef.viewaxis[1][2] = 0;
		cg.refdef.viewaxis[2][0] = 0;   cg.refdef.viewaxis[2][1] = 0;   cg.refdef.viewaxis[2][2] = 1;
		break;
	case 6:
		cg.refdef.viewaxis[0][0] = 0;   cg.refdef.viewaxis[0][1] = 1;   cg.refdef.viewaxis[0][2] = 0;
		cg.refdef.viewaxis[1][0] = -1;  cg.refdef.viewaxis[1][1] = 0;   cg.refdef.viewaxis[1][2] = 0;
		cg.refdef.viewaxis[2][0] = 0;   cg.refdef.viewaxis[2][1] = 0;   cg.refdef.viewaxis[2][2] = 1;
		break;
	default:
		break;
	}
}

/*
===============
CG_CalcTurretViewValues     0x30033220

The eye rides the turret's tag_player bone.
===============
*/
static void CG_CalcTurretViewValues( void ) {
	centity_t   *cent;
	/* CG_DObjGetWorldTagMatrix writes a full 64-byte 4x4 (DObjSkel2MatrixMultiply43
	   0x3003B0D0 stores through +0x3C incl [3][3]=1.0), so a 48-byte matrix[12]
	   would overflow 16 bytes onto the return address.  Retail frames it as
	   48 bytes plus three floats (ebp-40h..ebp-8h, 0x30033220). */
	float       matrix[16];
	float       pitch, yaw;

	if ( !( cg.predictedPlayerState.eFlags & EF_MOUNTED ) ) {
		return;
	}
	if ( cg.predictedPlayerState.viewlocked_entNum == ENTITYNUM_NONE_CG ) {
		return;
	}
	if ( !cg.predictedPlayerState.viewlocked ) {
		return;
	}

	cent = &cg_entities[ cg.predictedPlayerState.viewlocked_entNum ];

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cg.refdefViewAngles );

	if ( !trap_syscall_0xA2( cent->currentState.number ) ) {
		return;
	}

	CG_CalcEntityLerpPositions( cent );

	if ( !CG_DObjGetWorldTagMatrix( trap_syscall_0xA2( cent->currentState.number ),
									"tag_player", cent, matrix ) ) {
		Com_Error( ERR_DROP, "\x15" "Turret has no bone: tag_player\n" );
	}

	pitch = LerpAngle( cent->currentState.angles2[0], cent->nextState.angles2[0],
					   cg.frameInterpolation );
	yaw = LerpAngle( cent->currentState.angles2[1], cent->nextState.angles2[1],
					 cg.frameInterpolation );

	cg.refdefViewAngles[PITCH] += pitch;
	cg.refdefViewAngles[YAW] += yaw;

	if ( !cg.renderingThirdPerson && cg.predictedPlayerState.viewlocked == 2 ) {
		float turretShake;

		turretShake = rand() * 0.000030517578f;
		cg.refdefViewAngles[PITCH] += turretShake + turretShake - 1.0f;
		turretShake = rand() * 0.000030517578f;
		cg.refdefViewAngles[YAW] += turretShake + turretShake - 1.0f;
	}

	// the eye origin is row 3 of the 4x4 (matrix[12..14] at
	// 0x3003338A/0x3003338F/0x30033395), not the matrix43 row [9..11]
	cg.refdef.vieworg[0] = matrix[12];
	cg.refdef.vieworg[1] = matrix[13];
	cg.refdef.vieworg[2] = matrix[14] - cg.predictedPlayerState.viewHeightCurrent;
}

/*
===============
CG_CalcViewValues     0x300333B0

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_t   *ps;
	float           f;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	if ( cg.unknown_0x00010 ) {
		CG_CalcCubemapViewValues();
		return 0;
	}

	// calculate size of 3D view
	CG_CalcVrect();

	if ( cg_cinematicCamera ) {
		CG_Error( "Cinimatic Cameras are not available in the MP exe.\n" );
	}

	ps = &cg.predictedPlayerState;

	// intermission view
	if ( ps->pm_type == PM_INTERMISSION ) {
		VectorCopy( ps->origin, cg.refdef.vieworg );
		VectorCopy( ps->viewangles, cg.refdefViewAngles );
	} else {

		cg_bobCycle = ps->bobCycle / 255.0f * ( 2 * M_PI ) + ( 2 * M_PI );

		if ( ps->pm_flags & PMF_LADDER ) {
			if ( cg.time - ps->jumpTime < 500 ) {
				cg_xyspeed = 0.0f;
			} else {
				cg_xyspeed = ps->velocity[2];
			}
		} else {
			cg_xyspeed = (float)sqrt( ps->velocity[1] * ps->velocity[1]
									+ ps->velocity[0] * ps->velocity[0] );
		}

		VectorCopy( ps->origin, cg.refdef.vieworg );
		VectorCopy( ps->viewangles, cg.refdefViewAngles );

		// add error decay
		if ( cg_errordecay.value > 0 ) {
			int t;

			t = cg.time - cg_predictedErrorTime;
			f = ( cg_errordecay.value - t ) / cg_errordecay.value;
			if ( f > 0 && f < 1 ) {
				VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
			} else {
				cg_predictedErrorTime = 0;
			}
		}

		CG_CalcTurretViewValues();

		if ( cg.renderingThirdPerson ) {
			// back away from character
			CG_OffsetThirdPersonView();
		} else {
			// offset for local bobbing and kicks
			CG_OffsetFirstPersonView();
		}
	}
	// position eye relative to origin
	AngleVectors( cg.refdefViewAngles, cg.refdef.viewaxis[0], cg.refdef.viewaxis[1],
				  cg.refdef.viewaxis[2] );
	VectorSubtract( vec3_origin, cg.refdef.viewaxis[1], cg.refdef.viewaxis[1] );

	// field of view
	return CG_CalcFov();
}

/*
===============
CG_DrawSkyBoxPortal     0x30033640

Configstring 10 (CS_SKYBOXORG) is the skybox portal description:
"x y z fov fogstate [fog0 fog1 fog2 fognear fogfar]".
===============
*/
static void CG_DrawSkyBoxPortal( void ) {
	char        *cstr;
	char        *token;
	float       fov;
	refdef_t    backupRefdef;

	cstr = (char *)CG_ConfigString( CS_SKYBOXORG );
	if ( !cstr || !strlen( cstr ) ) {
		return;
	}

	backupRefdef = cg.refdef;

	if ( !cg_skybox.integer ) {
		cg.refdef.rdflags &= ~( RDF_SKYBOXPORTAL | RDF_DRAWSKYBOX );
		cg.refdef.rdflags |= RDF_SKYBOXPORTAL;
		cg.refdef.time = cg.time;
		trap_R_RenderScene( &cg.refdef );
		cg.refdef = backupRefdef;
		return;
	}

	token = Com_ParseOnLine( &cstr );
	if ( !token || !token[0] ) {
		CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring\n" );
	}
	cg.refdef.vieworg[0] = (float)atof( token );

	token = Com_ParseOnLine( &cstr );
	if ( !token || !token[0] ) {
		CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring\n" );
	}
	cg.refdef.vieworg[1] = (float)atof( token );

	token = Com_ParseOnLine( &cstr );
	if ( !token || !token[0] ) {
		CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring\n" );
	}
	cg.refdef.vieworg[2] = (float)atof( token );

	token = Com_ParseOnLine( &cstr );
	if ( !token || !token[0] ) {
		CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring\n" );
	}
	atoi( token );          /* the portal fov; 1.1 parses and drops it */

	token = Com_ParseOnLine( &cstr );
	if ( !token || !token[0] ) {
		CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring.  No fog state\n" );
	} else if ( atoi( token ) ) {
		float fogColor[3];
		int fogStart, fogEnd;

		token = Com_ParseOnLine( &cstr );
		if ( !token || !token[0] ) {
			CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring.  No fog[0]\n" );
		}
		fogColor[0] = (float)atof( token );

		token = Com_ParseOnLine( &cstr );
		if ( !token || !token[0] ) {
			CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring.  No fog[1]\n" );
		}
		fogColor[1] = (float)atof( token );

		token = Com_ParseOnLine( &cstr );
		if ( !token || !token[0] ) {
			CG_Error( "CG_DrawSkyBoxPortal: error parsing skybox configstring.  No fog[2]\n" );
		}
		fogColor[2] = (float)atof( token );

		token = Com_ParseOnLine( &cstr );
		if ( !token || !token[0] ) {
			fogStart = 0;
		} else {
			fogStart = atoi( token );
		}

		token = Com_ParseOnLine( &cstr );
		if ( !token || !token[0] ) {
			fogEnd = 0;
		} else {
			fogEnd = atoi( token );
		}

		trap_R_SetFog( FOG_PORTALVIEW, fogStart, fogEnd, fogColor[0], fogColor[1], fogColor[2], 1.1f );
		skyPortalFogSet = 1;
	} else if ( !skyPortalFogSet ) {
		trap_R_SetFog( FOG_PORTALVIEW, 0, 0, 0, 0, 0, 0 );
		skyPortalFogSet = 1;
	}

	fov = CG_GetViewFov();
	cg.refdef.fov_x = fov;
	cg.refdef.rdflags |= ( RDF_SKYBOXPORTAL | RDF_DRAWSKYBOX );
	cg.refdef.fov_y = (float)( atan2( cg.refdef.height,
									  cg.refdef.width / (float)tan( fov * ( M_PI / 360.0f ) ) )
							   * ( 360.0f / M_PI ) );

	cg.refdef.time = cg.time;
	trap_R_RenderScene( &cg.refdef );
	cg.refdef = backupRefdef;
}

/*
===============
updateTestFX     0x30033930

Respawns the /fxTest effect every cg_fxTestRestart milliseconds.
===============
*/
static void updateTestFX( void ) {
	if ( cg_fxTestRestart < 1 ) {
		return;
	}
	if ( cg.time <= cg_fxTestRestart + cg_fxTestTime ) {
		return;
	}

	trap_syscall_0xE3( trap_syscall_0xDE( cg_fxTestName ), (int)cg_fxTestPos );
	cg_fxTestTime = cg.time;
}

/*
=================
CG_DrawActiveFrame     0x30033980

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, int stereoView, int demoPlayback,
						 int cubemapShot, int cubemapSize ) {
	int         i;
	int         inwater;
	int         cmdNum;
	usercmd_t   cmd;
	const char  *mapname;
	qhandle_t   levelshot;
	char        expectedHunk[64];
	int         expected;
	float       frac;
	float       color[4];
	float       sensitivityScale;

	if ( stereoView != 2 ) {
		cg.oldTime = cg.time;
	}

	cg.demoPlayback = demoPlayback;
	cg.time = serverTime;
	bgs_time = serverTime;
	cg.unknown_0x00010 = cubemapShot;
	cg.unknown_0x00014 = cubemapSize;

	if ( stereoView != 2 ) {
		cg.frametime = cg.time - cg.oldTime;
		if ( cg.frametime < 0 ) {
			cg.frametime = 0;
			cg.oldTime = cg.time;
		}
		CG_AddLagometerFrameInfo();
	}
	bgs_frametime = cg.frametime;

	// update cvars
	for ( i = 0 ; i < CVARTABLE_COUNT ; i++ ) {
		trap_Cvar_Update( cvarTable[i].vmCvar );
	}

	// if we are running the loading screen, that is all we draw
	if ( cg.infoScreenText[0] ) {
		if ( !cg.snap && !cg_drawingInformation ) {
			cg_drawingInformation = 1;

			if ( cl_serverloadmap.string[0] ) {
				trap_Cvar_Set( "cl_serverloadmap", "" );
			}
			if ( cl_serverloadgametype.string[0] ) {
				trap_Cvar_Set( "cl_serverloadgametype", "" );
			}
			if ( cl_serverloadwaiting.integer ) {
				trap_Cvar_Set( "cl_serverloadwaiting", "0" );
			}

			levelshot = 0;
			mapname = Info_ValueForKey( CG_ConfigString( CS_SERVERINFO ), "mapname" );
			if ( mapname && mapname[0] ) {
				levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", mapname ), 2 );
			}
			if ( !levelshot ) {
				levelshot = trap_R_RegisterShaderNoMip( "menu/art/unknownmap", 2 );
			}
			trap_R_SetColor( NULL );
			CG_DrawPic( 0, 0, 640, 480, levelshot );

			trap_Cvar_VariableStringBuffer( "com_expectedhunkusage", expectedHunk,
											sizeof( expectedHunk ) );
			expected = atoi( expectedHunk );
			color[0] = color[1] = color[2] = color[3] = 0.8f;
			if ( expected > 0 ) {
				frac = trap_hunkUsed() / (float)expected;
				if ( frac > 1.0f ) {
					frac = 1.0f;
				}
				CG_HorizontalPercentBar( 200, 468, 240, 10, frac );
			}

			trap_UpdateScreen();
			cg_drawingInformation--;
		}
		return;
	}

	// any looped sounds will be respecified as entities
	// are added to the render list
	trap_R_ClearScene();

	// update cg.predictedPlayerState
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap ) {
		if ( !cg_drawingInformation ) {
			cg_drawingInformation = 1;

			if ( cl_serverloadmap.string[0] ) {
				trap_Cvar_Set( "cl_serverloadmap", "" );
			}
			if ( cl_serverloadgametype.string[0] ) {
				trap_Cvar_Set( "cl_serverloadgametype", "" );
			}
			if ( cl_serverloadwaiting.integer ) {
				trap_Cvar_Set( "cl_serverloadwaiting", "0" );
			}

			levelshot = 0;
			mapname = Info_ValueForKey( CG_ConfigString( CS_SERVERINFO ), "mapname" );
			if ( mapname && mapname[0] ) {
				levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", mapname ), 2 );
			}
			if ( !levelshot ) {
				levelshot = trap_R_RegisterShaderNoMip( "menu/art/unknownmap", 2 );
			}
			trap_R_SetColor( NULL );
			CG_DrawPic( 0, 0, 640, 480, levelshot );

			trap_Cvar_VariableStringBuffer( "com_expectedhunkusage", expectedHunk,
											sizeof( expectedHunk ) );
			expected = atoi( expectedHunk );
			color[0] = color[1] = color[2] = color[3] = 0.8f;
			if ( expected > 0 ) {
				frac = trap_hunkUsed() / (float)expected;
				if ( frac > 1.0f ) {
					frac = 1.0f;
				}
				CG_HorizontalPercentBar( 200, 468, 240, 10, frac );
			}

			trap_UpdateScreen();
			cg_drawingInformation--;
		}
		return;
	}

	if ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) {
		return;
	}

	// the connection screen stays up until the first command we sent has been
	// acknowledged
	if ( cl_serverloadmap.string[0] && cl_serverloadgametype.string[0] ) {
		cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
		trap_GetUserCmd( cmdNum, &cmd );
		if ( cmd.serverTime > cg.snap->ps.commandTime && cmd.serverTime <= cg.time ) {
			CG_DrawInformation( 1 );
			return;
		}
	}

	if ( cl_serverloadwaiting.integer ) {
		trap_Cvar_Set( "cl_serverloadwaiting", "0" );
	}

	if ( cg_norender.integer ) {
		return;
	}

	cg.clientFrame++;

	// let the client system know what our weapon and zoom settings are
	CG_PredictPlayerState();

	// pull the shellshock the server picked for us into cg.shellshock
	if ( cg.snap->ps.shellshockIndex ) {
		cg.shellshock.parms = &cgs.shellshockParms[ cg.snap->ps.shellshockIndex ];
		cg.shellshock.startTime = cg.snap->ps.shellshockTime;
		cg.shellshock.duration = cg.snap->ps.shellshockDuration;
	} else {
		cg.shellshock.parms = &cgs.shellshockParms[0];
		cg.shellshock.startTime = cg.shellshock.forcedStartTime;
		cg.shellshock.duration = cg.shellshock.forcedDuration;
	}
	CG_UpdateShellShock( cg.shellshock.startTime, cg.shellshock.parms, cg.shellshock.duration );

	// decide on third person view
	cg.renderingThirdPerson = qfalse;
	if ( cg_thirdPerson.integer || cg.nextSnap->ps.pm_type >= PM_DEAD_OR_SPECTATING ) {
		cg.renderingThirdPerson = qtrue;
	}

	if ( cg.nextSnap->ps.pm_flags & PMF_TURRET ) {
		CG_KickAngles();
	} else {
		VectorClear( cg.kick_angles );
		VectorClear( cg.kick_origin );
	}

	// build cg.refdef
	inwater = CG_CalcViewValues();

	if ( !cg.unknown_0x00010 ) {
		CG_ShakeCamera();
		AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
		CG_PerturbCamera();
	}

	// build the render lists
	CG_DrawSkyBoxPortal();

	trap_syscall_0xEA( cg.time );
	trap_syscall_0xEB( (int)&cg.refdef );

	updateTestFX();

	CG_AddPacketEntities();         // adder all entities to the scene
	CG_AddMarks();                  // add wall marks
	CG_AddLocalEntities();
	CG_AddViewWeapon( &cg.predictedPlayerState );
	CG_PlayBufferedVoiceChats();

	trap_syscall_0xE6();

	// finish up the rest of the refdef
	cg.refdef.time = cg.time;

	if ( cg_weaponSelect.integer < 0 || cg_weaponSelect.integer > bg_numWeapons ) {
		CG_Printf( "WARNING: Invalid cg_weaponSelect setting %i (out of range 0 - %i)\n",
				   cg_weaponSelect.integer, bg_numWeapons );
		for ( i = 1 ; i <= 5 ; i++ ) {
			if ( cg.predictedPlayerState.weaponslots[i] ) {
				trap_Cvar_Set( "cg_weaponSelect",
							   va( "%i", cg.predictedPlayerState.weaponslots[i] ) );

			}
		}
		trap_Cvar_Set( "cg_weaponSelect", "0" );
	}

	sensitivityScale = cg_fovScale;
	if ( cg.shellshock.mouseSensitivityScale != 0.0f ) {
		sensitivityScale = cg.shellshock.mouseSensitivityScale * sensitivityScale;
	}
	trap_SetUserCmdAimValues( (const int *)cg.kick_origin );
	trap_syscall_0x55( cg_weaponSelect.integer, sensitivityScale );

	trap_syscall_0xD0( cg.refdef.vieworg[0], cg.refdef.vieworg[1], cg.refdef.vieworg[2] );

	// actually issue the rendering calls
	CG_DrawActive( stereoView );

	trap_syscall_0xD1( cg.snap->ps.clientNum, (int)cg.refdef.vieworg,
					   (int)cg.refdef.viewaxis );
	trap_syscall_0xD2();

	if ( cg_stats.integer ) {
		CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
	}
}
