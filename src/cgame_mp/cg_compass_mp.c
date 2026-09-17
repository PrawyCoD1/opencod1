/*
 * cg_compass_mp.c -- the compass friendly blips.
 * (original source: cgame/cg_compass.c)
 *
 * cgame_mp_x86.dll 0x30013540 .. 0x30013BB9, one function.  No RTCW
 * counterpart: the compass is a CoD addition, and this is the ownerdraw the
 * menu system dispatches for it (CG_OwnerDraw case 88, 0x30026FD8).
 *
 * The function does two jobs.  First it refreshes cg's 64-entry friendly
 * table from cg.nextSnap -- every ET_PLAYER entity in the snapshot that is on
 * the local player's team, plus whatever the local player's own
 * ps.iCompassFriendInfo names -- then it draws the surviving entries around
 * the compass rose.
 *
 * IT READS cg.nextSnap, NOT cg.snap (0x301E2164 at 0x30013540), and it reads
 * each entity's nextState rather than currentState (centity+0xF4 at
 * 0x300135A4).
 *
 * @fidelity: likely
 */

#include <math.h>

#include "cg_local.h"

#ifndef YAW
#define PITCH   0
#define YAW     1
#define ROLL    2
#endif

/* g_local.h's team_t, which the cgame side cannot see; the two values this
 * unit rejects are the ones ClientScr_SetSessionTeam calls none and
 * spectator. */
#ifndef TEAM_SPECTATOR
#define TEAM_FREE       0
#define TEAM_SPECTATOR  3
#endif

#define ET_PLAYER       1       /* nextState.eType tested at 0x300135A4 */
#define EF_DEAD         1       /* nextState.eFlags bit tested at 0x300135B3 */
#define EF_TALK         0x80000 /* the bit that starts the 3-second flash (0x300135E1) */
#define EF_TALK_LOCAL   0x100000/* the same thing off the local ps.eFlags (0x3001378C) */

#define ANGLE2SHORT( x )    ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )    ( (float)( x ) * ( 360.0f / 65536.0f ) )

/*
 * cg.unknown_0x2AF70 is this table: 64 entries of 20 bytes, walked from
 * 0x3020D0B0 to 0x3020D5B4.  origin[2] is not a Z -- it is the blip's yaw,
 * which is why the local-player branch overwrites it after storing a
 * direction into all three (0x30013780).
 */
typedef struct compassFriendly_s {
	int time;                       /* +0x00 cg.time when this entry was refreshed */
	vec3_t origin;                  /* +0x04 world x, world y, yaw */
	int flashEndTime;               /* +0x10 cg.time + 3000 while the client is talking */
} compassFriendly_t;

#define cg_compassFriendlies    ( (compassFriendly_t *)cg.unknown_0x2AF70 )

/*
=================
CG_DrawCompassFriendlies   0x30013540

`rect` is the ownerdraw rectangle {x,y,w,h}; the second argument is
CG_OwnerDraw's shader (0x30026FDC pushes arg_34) and is never read here.
=================
*/
void CG_DrawCompassFriendlies( float *rect, qhandle_t shader, const float *color ) {
	snapshot_t *snap;
	playerState_t *ps;
	centity_t *cent;
	compassFriendly_t *friendly;
	vec4_t drawColor;
	vec3_t pos, dir;
	float centerX, centerY;
	float x, y, size, radius, dist, alpha, angle, iconAngle;
	float s, c;
	int team;
	int i, num;
	int flash;

	snap = cg.nextSnap;
	ps = &snap->ps;

	if ( !bg_clientinfo[ps->clientNum].infoValid ) {
		return;
	}
	team = bg_clientinfo[ps->clientNum].team;
	if ( team == TEAM_SPECTATOR || team == TEAM_FREE ) {
		return;
	}

	//
	// refresh every friendly the snapshot can see
	//
	for ( i = 0; i < snap->numEntities; i++ ) {
		num = snap->entities[i].number;
		cent = &cg_entities[num];
		if ( cent->nextState.eType != ET_PLAYER ) {
			continue;
		}
		if ( cent->nextState.eFlags & EF_DEAD ) {
			continue;
		}
		if ( num < 0 ) {
			continue;
		}
		if ( !bg_clientinfo[num].infoValid ) {
			continue;
		}
		if ( bg_clientinfo[num].team != team ) {
			continue;
		}

		friendly = &cg_compassFriendlies[num];
		friendly->time = cg.time;
		friendly->origin[0] = cent->lerpOrigin[0];
		friendly->origin[1] = cent->lerpOrigin[1];
		friendly->origin[2] = cent->lerpAngles[YAW];

		if ( ( cent->nextState.eFlags & EF_TALK ) && friendly->flashEndTime <= cg.time ) {
			friendly->flashEndTime = cg.time + 3000;
		}
	}

	//
	// and the one the server packed into the local player state: client number
	// in the low 6 bits, then two 9-bit coordinates, then a yaw byte
	//
	if ( ps->iCompassFriendInfo ) {
		friendly = &cg_compassFriendlies[ps->iCompassFriendInfo & 0x3F];
		friendly->time = cg.time;

		dir[0] = (float)( 4 * ( ( ps->iCompassFriendInfo >> 6 ) & 0x1FF ) - 1020 );
		dir[1] = (float)( 4 * ( ( ps->iCompassFriendInfo >> 15 ) & 0x1FF ) - 1020 );

		if ( dir[0] == 1024.0f || dir[0] == -1020.0f || dir[1] == 1024.0f || dir[1] == -1020.0f ) {
			/* saturated: the pair is a direction, not an offset */
			VectorNormalize2D( dir );
			friendly->origin[0] = dir[0];
			friendly->origin[1] = dir[1];
			friendly->origin[2] = dir[2];
		} else {
			pos[0] = ps->origin[0];
			pos[1] = ps->origin[1];
			pos[2] = ps->origin[2] + ps->viewHeightCurrent;
			AddLeanToPosition( pos, ps->viewangles[YAW], ps->leanf, 16.0f, 20.0f );
			friendly->origin[0] = pos[0] + dir[0];
			friendly->origin[1] = pos[1] + dir[1];
		}

		friendly->origin[2] = ( ps->iCompassFriendInfo >> 24 ) * ( 360.0f / 256.0f );

		if ( ( ps->eFlags & EF_TALK_LOCAL ) && friendly->flashEndTime <= cg.time ) {
			friendly->flashEndTime = cg.time + 3000;
		}
	}

	CG_UpdateCompPointerOrientation();

	centerX = rect[0] + cg_hudCompassSize.value * rect[2] * 0.5f;
	centerY = rect[1] + cg_hudCompassSize.value * rect[3] * 0.5f
			  - ( cg_hudCompassSize.value - 1.0f ) * 160.0f;

	drawColor[0] = color[0];
	drawColor[1] = color[1];
	drawColor[2] = color[2];
	drawColor[3] = color[3];

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		friendly = &cg_compassFriendlies[i];

		if ( friendly->time > cg.time ) {
			friendly->time = 0;
		}
		if ( friendly->time < cg.time - 800 ) {
			continue;
		}
		if ( snap->ps.clientNum == i ) {
			continue;
		}

		if ( fabs( friendly->origin[0] ) > 1.0 || fabs( friendly->origin[1] ) > 1.0 ) {
			// a world position: bearing and range from the view origin
			dir[0] = friendly->origin[0] - cg.refdef.vieworg[0];
			dir[1] = friendly->origin[1] - cg.refdef.vieworg[1];
			angle = (float)SHORT2ANGLE( ANGLE2SHORT( vectoyaw( dir ) - cg.compassPointerAngle ) );

			dist = (float)sqrt( dir[1] * dir[1] + dir[0] * dir[0] );

			alpha = dist;
			if ( alpha > cg_hudObjectiveMaxRange.value ) {
				alpha = cg_hudObjectiveMaxRange.value;
			} else if ( alpha < cg_hudCompassMaxRange.value ) {
				alpha = cg_hudCompassMaxRange.value;
			}
			alpha = ( alpha - cg_hudCompassMaxRange.value )
					/ ( cg_hudObjectiveMaxRange.value - cg_hudCompassMaxRange.value );
			alpha = ( cg_hudObjectiveMinAlpha.value - 1.0f ) * alpha + 1.0f;

			if ( dist > cg_hudCompassMaxRange.value ) {
				dist = cg_hudCompassMaxRange.value;
			} else if ( dist < cg_hudCompassMinRange.value ) {
				dist = cg_hudCompassMinRange.value;
			}
		} else {
			// a bare direction: pin it to the rim at half fade
			angle = (float)SHORT2ANGLE( ANGLE2SHORT( vectoyaw( friendly->origin ) - cg.compassPointerAngle ) );
			dist = cg_hudCompassMaxRange.value;
			alpha = ( 1.0f - cg_hudObjectiveMinAlpha.value ) * 0.5f + cg_hudObjectiveMinAlpha.value;
		}

		radius = ( ( dist - cg_hudCompassMinRange.value )
				   / ( cg_hudCompassMaxRange.value - cg_hudCompassMinRange.value )
				   * ( 1.0f - cg_hudCompassMinRadius.value )
				   + cg_hudCompassMinRadius.value )
				 * cg_hudCompassSize.value * 43.75f;

		angle = angle * (float)M_PI / 180.0f;
		c = (float)cos( angle );
		s = (float)sin( angle );

		size = cg_hudCompassSize.value * 10.0f;
		x = centerX - size * 0.5f - s * radius;
		y = centerY - size * 0.5f - c * radius;

		iconAngle = (float)SHORT2ANGLE( ANGLE2SHORT( cg.refdefViewAngles[YAW] - friendly->origin[2] ) );

		flash = 0;
		if ( friendly->flashEndTime > cg.time ) {
			flash = ( ( friendly->flashEndTime - cg.time ) % 500 ) >= 250;
		}

		trap_R_SetColor( drawColor );

		if ( flash == 1 ) {
			CG_DrawPic( x, y, size, size, cgs.media.objectiveFriendlyChat );
		} else {
			/* the binary indexes the objectiveFriendly / objectiveFriendlyChat
			   pair with `flash` (0x30013B6F), so cgs.media wants these two as
			   one two-element array. */
			CG_DrawRotatedPic( x, y, size, size, iconAngle,
							   ( &cgs.media.objectiveFriendly )[flash] );
		}
	}

	trap_R_SetColor( NULL );
}
