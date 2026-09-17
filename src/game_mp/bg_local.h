/*
 * bg_local.h -- definitions private to the movement units.
 *
 * Call of Duty 1.1 (game_mp_x86.dll, imagebase 0x20000000).  RTCW's
 * game/bg_local.h is the model.  It exists here for the same reason it does
 * there: pm, pml and pml_t are not private to bg_pmove.c -- bg_slidemove.c
 * runs inside a Pmove and bg_weapon.c owns the PM_Weapon family, and all three
 * reach the same movement state.
 */

#ifndef __BG_LOCAL_H__
#define __BG_LOCAL_H__

#include "bg_public.h"

/*
 * ---- pm_flags ------------------------------------------------------------
 * bg_public.h deliberately leaves these unnamed, so the movement units name
 * them here -- ONE spelling per bit.  Each name is what the bit does at the
 * address given.  playerState_t::pm_flags is at +0x0C.
 */
#define PMF_PRONE               0x00000001  /* PM_CheckDuck 0x20009CA6 */
#define PMF_DUCKED              0x00000002  /* PM_CheckDuck 0x20009CC5 */
#define PMF_PRONE_DIVE          0x00000004  /* PM_CheckDuck 0x2000A205, shortens the duck->prone lerp */
#define PMF_JUMP_HELD           0x00000008  /* Pmove 0x2000C7AE, PM_CheckJump 0x20007902 */
#define PMF_LADDER              0x00000010  /* PM_CheckLadderMove 0x2000BD8C; forces the held weapon to 0 */
#define PMF_ADS                 0x00000020  /* PM_UpdateAimDownSightFlag 0x2000FAD9 sets/clears only this */
#define PMF_BACKWARDS_RUN       0x00000040  /* PmoveSingle 0x2000C47D */
#define PMF_WALKING             0x00000080  /* PM_UpdatePlayerWalkingFlag 0x2000BB7A */
#define PMF_TIME_LAND           0x00000100  /* PM_CrashLand 0x20008B3D */
#define PMF_TIME_KNOCKBACK      0x00000200  /* PM_Friction 0x200071F2 */
#define PMF_PRONE_MOVEOVERRIDE  0x00000400  /* PM_SetProneMovementOverride 0x2000BB1C */
#define PMF_RESPAWNED           0x00000800  /* PmoveSingle 0x2000C34C clears it when attack comes up */
#define PMF_MELEE_LATCH         0x00001000  /* PM_Weapon_CheckForMelee 0x2001189F, one swipe per press */
#define PMF_TIME_HARDLAND       0x00002000  /* PM_CrashLand 0x20008A64 */
#define PMF_IGNORE_INPUT        0x00004000  /* PmoveSingle 0x2000C1BD; also blocks a weapon change */
#define PMF_PRONE_BLOCKED       0x00008000  /* PM_CheckDuck 0x20009DB9, cleared every PmoveSingle */
#define PMF_FOLLOW              0x00010000  /* RTCW's bit unchanged -- g_cmds_mp.c Cmd_Follow_f,
                                               cgame CG_TransitionSnapshot 0x3002FC1C */

/*
 * pml_t -- the movement state one Pmove works in.  The object lives at
 * 0x2014EB20 and PmoveSingle clears 0x88 of it.  CoD1 dropped nothing from
 * RTCW's layout and appended weaponInfo; note that groundTrace is CoD's
 * 48-byte trace_t, which is what puts previous_origin at +0x68 -- PmoveSingle
 * writes ps->origin there.
 */
typedef struct pml_t
{
	vec3_t forward, right, up;
	float frametime;
	int msec;
	qboolean walking;
	qboolean groundPlane;
	trace_t groundTrace;
	float impactSpeed;
	vec3_t previous_origin;
	vec3_t previous_velocity;
	int previous_waterlevel;
	const struct weaponInfo_t *weaponInfo;
} pml_t;
COD1_ASSERT_SIZE( pml_t, 136 );

extern pmove_t  *pm;            /* 0x2014ECD0 */
extern pml_t pml;               /* 0x2014EB20 */
extern int c_pmove;             /* 0x200A06A0 -- the Pmove counter every debug print leads with */

//
// bg_pmove.c
//
/* Not functions in retail: every user carries the 0x7FFFFFFF mask and the
 * test/setl 1-2*neg sequence inline (PM_UpdatePronePitch 0x2000B9BD and
 * 0x2000B9D9), so they must be __inline -- the DLL is built /Ob1. */
static __inline float PM_FloatAbs( float value ) {
	int bits;

	bits = *(int *)&value & 0x7FFFFFFF;
	return *(float *)&bits;
}

static __inline int PM_FloatIsNegative( float value ) {
	return *(int *)&value < 0;
}

static __inline int PM_FloatSign( float value ) {
	return 1 - PM_FloatIsNegative( value ) * 2;
}

void        PM_AddEvent( int newEvent );
void        PM_AddTouchEnt( int entityNum );
void        PM_ClipVelocity( const vec3_t in, const vec3_t normal, vec3_t out, float overbounce );
int         PM_GetViewHeightLerpTime( const playerState_t *ps, int viewheight,
									  int viewHeightLerpDown );
void        PM_FootstepEvent( int oldBobCycle, qboolean footstep, int newBobCycle );
qboolean    PM_ShouldMakeFootsteps( void );
void        PM_SetProneMovementOverride( void );

//
// bg_slidemove.c
//
qboolean    PM_SlideMove( qboolean gravity );
void        PM_StepSlideMove( qboolean gravity );

//
// bg_weapon.c
//
void        PM_AdjustAimSpreadScale( void );
void        PM_UpdateAimDownSightFlag( void );
qboolean    PM_InteruptWeaponWithProneMove( void );
void        PM_Weapon( void );

#endif /* __BG_LOCAL_H__ */
