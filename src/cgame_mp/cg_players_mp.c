/*
 * cg_players_mp.c -- adding a player, a corpse, and the sprites and marks that
 * go with them.
 * (original source: cgame/cg_players.c)
 *
 * cgame_mp_x86.dll 0x30027310 .. 0x30028732, nine functions.
 *
 * RTCW's cgame/cg_players.c is the ancestor of five of them --
 * CG_PlayerFloatSprite, CG_PlayerSprites, CG_PlayerShadow, CG_PlayerSplash and
 * CG_ResetPlayerEntity are all recognisably its text.
 * The rest of RTCW's file has NO counterpart here: CoD moved the whole
 * player-model and animation layer into the shared bg_animation.c and the
 * DObj/XAnim system, so CG_Player is thirty lines around one refEntity_t with
 * reType RT_DOBJ and BG_PlayerAnimation does what RTCW's
 * CG_PlayerAnimation/CG_PlayerAngles/CG_RunLerpFrame did.
 *
 * CG_PlayerTurretPositionAndBlend is CoD's own and is the biggest function in
 * the file: it puts a mounted player onto the turret's tag_weapon by blending
 * the two-dimensional grid of "mounted" animations -- rows by height, columns
 * by yaw -- until the tag the animation produces lands on the turret's grip.
 *
 * @fidelity: likely
 */

#include <math.h>
#include <string.h>

#include "cg_local.h"

/*
 * refEntity_t fields that land in cg_public.h's unknown_ ranges.
 * RE_SPRITESIZE is a second float CG_PlayerFloatSprite sets to the same value
 * as RE_RADIUS; its name is unrecovered (0x3002745A / 0x30027461).
 */
#define RE_SPRITESIZE( e )      ( *(float *)&( e )->unknown_0x60[0x04] )    /* +0x64 */
#define RE_CUSTOMSHADER( e )    ( *(qhandle_t *)&( e )->unknown_0x60[0x08] )/* +0x68 */
#define RE_LIGHTINGORIGIN( e )  ( (float *)&( e )->unknown_0x08[4] )        /* +0x0C */
#define RE_RADIUS( e )          ( *(float *)&( e )->unknown_0x70[0x0C] )    /* +0x7C */
#define RE_DOBJ( e )            ( *(int *)&( e )->unknown_0x70[0x20] )      /* +0x90 */
#define RE_OWNER( e )           ( *(void **)&( e )->unknown_0x70[0x24] )    /* +0x94 */

#define RT_DOBJ                 1
#define RT_SPRITE               4       /* CG_PlayerFloatSprite 0x30027480 */

#define RF_THIRD_PERSON         0x02
#define RF_LIGHTING_ORIGIN      0x80

/* entityState_t.eFlags. */
#define EF_DEAD                 0x000001
#define EF_CROUCHING            0x000020    /* inferred: the middle lighting-origin height */
#define EF_PRONE                0x000040    /* inferred: the lowest lighting-origin height */
#define EF_NODRAW               0x000100
#define EF_CONNECTION           0x001000    /* the "disconnected" head icon */
#define EF_MOUNTED              0x00C000
#define EF_TALKING              0x040000    /* the head balloon; cg_compass.c's EF_TALK is 0x80000 */

/* playerState_t.pm_flags; the same constant cg_weapons_mp.c carries. */
#define PMF_FIRSTPERSON         0x50000

#define ANIM_TOGGLEBIT          0x200

#define CS_HEADICONS            28      /* CG_PlayerSprites 0x30027540 */

#ifndef TEAM_SPECTATOR
#define TEAM_SPECTATOR          3
#endif

#define SHADOW_DISTANCE         64.0f

/* q_shared.h has no contents enum; these two are the masks this unit uses. */
#ifndef CONTENTS_SOLID
#define CONTENTS_SOLID          0x01
#define CONTENTS_WATER          0x20
#endif
/* the mask CG_PlayerShadow's drop trace and the turret drop trace both use */
#define MASK_PLAYERSOLID_CG     0x2810011

/*
 * cg+0x2AA9C, the last dword of cg_local.h's unknown_0x2AA40[96].  CG_Player
 * clears RF_THIRD_PERSON when it is set (0x30028318), i.e. it is the "draw the
 * local player's own model anyway" latch the view-model code raises.
 */
#define cg_drawOwnPlayerModel   ( *(int *)&cg.unknown_0x2AA40[0x5C] )

/* universal/com_math.c. */
extern vec3_t vec3_origin;
/* 0x3003AE70 / 0x3003C810 here.  MatrixMultiply43 is NOT MatrixMultiply34_m:
   this one walks four rows of three, that one three rows of four. */

/* game_mp/bg_animation.c, compiled into this DLL too. */
extern animScriptData_t bgs_animScriptData;             /* 0x300F0520 */
extern int  ( *Scr_GetAnimsIndex )( int anims );        /* 0x300AAC80 */

/* cg_ent_mp.c. */
float      *CG_DObjGetLocalTagMatrix( int obj, const char *tagName, centity_t *cent );
qboolean    CG_DObjGetWorldTagMatrix( int obj, const char *tagName, centity_t *cent,
									  float *outMatrix );

/* cg_weapons_mp.c. */
void        CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent,
								int drawGun );

/* cg_marks_mp.c.  Not declared in cg_local.h -- cg_main_mp.c's vmMain carries
   an all-int prototype for it and the two spellings cannot coexist. */
void        CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
						   float orientation, float red, float green, float blue,
						   float alpha, qboolean alphaFade, float radius,
						   qboolean temporary, int duration );

/* game_mp/bg_animation.c's root player animation; cg_main_mp.c carries the
   same extern.  The three resolved ones live in bgs_animScriptData. */
extern scr_anim_t bgs_rootAnim;                         /* 0x3018BBFC */

/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head.  RTCW's, with the head tag and the
distance-scaled radius added: `scaleByDistance` is what the kill-cam icon uses
so it stays readable across the map.
===============
*/
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader, int height,
								  qboolean scaleByDistance ) {
	int         rf;
	int         obj;
	refEntity_t ent;
	float       tagMatrix[16];
	vec3_t      origin;
	float       radius;
	float       dist;

	if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
		 && cent->currentState.number == cg.snap->ps.clientNum
		 && !cg.renderingThirdPerson ) {
		rf = RF_THIRD_PERSON;       // only show in mirrors
	} else {
		rf = 0;
	}

	obj = trap_syscall_0xA2( cent->currentState.number );
	if ( obj && CG_DObjGetWorldTagMatrix( obj, "Bip01 Head", cent, tagMatrix ) ) {
		origin[0] = tagMatrix[12];
		origin[1] = tagMatrix[13];
		origin[2] = tagMatrix[14] + height + 18.0f;
	} else {
		origin[0] = cent->lerpOrigin[0];
		origin[1] = cent->lerpOrigin[1];
		origin[2] = cent->lerpOrigin[2] + height + 72.0f;
	}

	if ( scaleByDistance ) {
		dist = (float)sqrt( ( origin[0] - cg.refdef.vieworg[0] )
							* ( origin[0] - cg.refdef.vieworg[0] )
							+ ( origin[1] - cg.refdef.vieworg[1] )
							* ( origin[1] - cg.refdef.vieworg[1] )
							+ ( origin[2] - cg.refdef.vieworg[2] )
							* ( origin[2] - cg.refdef.vieworg[2] ) );

		radius = dist * ( 1.0f / 256.0f ) + 0.2f;
		if ( radius < 0.6f ) {
			radius = 0.6f;
		}
		origin[2] += 8.0f * radius - 8.0f;
		radius *= 6.66f;
	} else {
		radius = 6.66f;
	}

	memset( &ent, 0, sizeof( ent ) );
	ent.reType = RT_SPRITE;
	ent.renderfx = rf;
	ent.origin[0] = origin[0];
	ent.origin[1] = origin[1];
	ent.origin[2] = origin[2];
	RE_SPRITESIZE( &ent ) = radius;
	RE_RADIUS( &ent ) = radius;
	RE_CUSTOMSHADER( &ent ) = shader;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;

	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_PlayerSprites

Float sprites over the player's head.  The objective icon stacks: when one is
drawn the rest move up by its height.
===============
*/
static void CG_PlayerSprites( centity_t *cent ) {
	float       height;
	int         team;
	int         myTeam;
	qhandle_t   shader;

	height = 0.0f;

	if ( !bg_clientinfo[cent->currentState.clientNum].infoValid ) {
		return;
	}
	team = bg_clientinfo[cent->currentState.clientNum].team;

	if ( !bg_clientinfo[cg.snap->ps.clientNum].infoValid ) {
		return;
	}
	myTeam = bg_clientinfo[cg.snap->ps.clientNum].team;

	if ( cent->currentState.iHeadIcon ) {
		if ( !cent->currentState.iHeadIconTeam
			 || myTeam == TEAM_SPECTATOR
			 || cent->currentState.iHeadIconTeam == myTeam ) {
			shader = trap_R_RegisterShader(
						 CG_ConfigString( cent->currentState.iHeadIcon + CS_HEADICONS ), 5 );
			if ( shader ) {
				CG_PlayerFloatSprite( cent, shader, 0, qfalse );
				height = 16.0f;
			}
		}
	}

	if ( cent->currentState.number == cg.clientNum
		 && cg.snap->ps.clientNum != cg.clientNum ) {
		CG_PlayerFloatSprite( cent, cgs.media.headiconYouInKillCam, (int)height, qtrue );
		return;
	}

	if ( cent->currentState.eFlags & EF_CONNECTION ) {
		CG_PlayerFloatSprite( cent, cgs.media.headiconDisconnected, (int)height, qfalse );
		return;
	}

	if ( team != myTeam && myTeam != TEAM_SPECTATOR ) {
		return;
	}

	// show the voice chat signal so players know who is talking
	if ( cent->voiceChatSpriteTime > cg.time ) {
		CG_PlayerFloatSprite( cent, cent->voiceChatSprite, (int)height, qfalse );
		return;
	}

	if ( cent->currentState.eFlags & EF_TALKING ) {
		CG_PlayerFloatSprite( cent, cgs.media.headiconTalkBalloon, (int)( height - 5.0f ),
							  qfalse );
	}
}

/*
===============
CG_PlayerShadow

Returns the plane the shadow is on, and draws the blob mark when cg_shadows is
1.  The blob fades in from 250 to 512 units and back out to 1024, so the close
range is left to whatever cg_shadows 2 does instead.
===============
*/
static qboolean CG_PlayerShadow( float *shadowPlane, centity_t *cent ) {
	vec3_t  end;
	trace_t trace;
	float   alpha;
	float   dist;

	*shadowPlane = 0;

	if ( !cg_shadows.integer ) {
		return qfalse;
	}

	end[0] = cent->lerpOrigin[0];
	end[1] = cent->lerpOrigin[1];
	end[2] = cent->lerpOrigin[2] - SHADOW_DISTANCE;

	trap_CM_BoxTrace( &trace, cent->lerpOrigin, end, NULL, NULL, 0, MASK_PLAYERSOLID_CG );

	// no shadow if the trace didn't hit anything
	if ( trace.fraction == 1.0 ) {
		return qfalse;
	}
	if ( trace.fraction == 0.0 ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1.0f;

	if ( cg_shadows.integer != 1 ) {
		return qtrue;
	}
	if ( cent->currentState.eFlags & EF_DEAD ) {
		return qfalse;
	}

	alpha = (float)( 1.0 - trace.fraction );

	dist = Distance( cent->lerpOrigin, cg.snap->ps.origin );
	if ( dist <= 250.0 ) {
		return qtrue;
	}

	if ( dist < 512.0 ) {
		alpha *= (float)( ( dist - 250.0 ) * ( 1.0 / 262.0 ) );
	} else if ( dist <= 1024.0 ) {
		alpha *= (float)( 1.0 - ( dist - 512.0 ) * ( 1.0 / 512.0 ) );
	} else {
		return qfalse;
	}

	CG_ImpactMark( cgs.media.markShadowShader, trace.endpos, trace.normal, 0.0f,
				   alpha, alpha, alpha, 1.0f, qfalse, 16.0f, qtrue, -1 );

	return qtrue;
}

/*
===============
CG_PlayerSplash

Draw a mark at the water surface.  RTCW's, with CoD's single CONTENTS_WATER in
place of RTCW's water|slime|lava.
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	vec3_t      start, end;
	trace_t     trace;
	int         contents;
	polyVert_t  verts[4];

	if ( !cg_shadows.integer ) {
		return;
	}

	end[0] = cent->lerpOrigin[0];
	end[1] = cent->lerpOrigin[1];
	end[2] = cent->lerpOrigin[2] - 24.0f;

	// if the feet aren't in liquid, don't make a mark
	contents = trap_CM_PointContents( end, 0 );
	if ( !( contents & CONTENTS_WATER ) ) {
		return;
	}

	start[0] = cent->lerpOrigin[0];
	start[1] = cent->lerpOrigin[1];
	start[2] = cent->lerpOrigin[2] + 32.0f;

	// if the head isn't out of liquid, don't make a mark
	contents = trap_CM_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER ) ) {
		return;
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	verts[0].xyz[0] = trace.endpos[0] - 32.0f;
	verts[0].xyz[1] = trace.endpos[1] - 32.0f;
	verts[0].xyz[2] = trace.endpos[2];
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	verts[1].xyz[0] = trace.endpos[0] - 32.0f;
	verts[1].xyz[1] = trace.endpos[1] + 32.0f;
	verts[1].xyz[2] = trace.endpos[2];
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	verts[2].xyz[0] = trace.endpos[0] + 32.0f;
	verts[2].xyz[1] = trace.endpos[1] + 32.0f;
	verts[2].xyz[2] = trace.endpos[2];
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	verts[3].xyz[0] = trace.endpos[0] + 32.0f;
	verts[3].xyz[1] = trace.endpos[1] - 32.0f;
	verts[3].xyz[2] = trace.endpos[2];
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}

/*
===============
CG_PlayerTurretGoalTime

The blend time every goal weight below is given: enough for one frame's worth
of change at the rate the last frame achieved, and 0 when the weight is already
there.  Inlined into all seven of CG_PlayerTurretPositionAndBlend's goal sets.
===============
*/
static float CG_PlayerTurretGoalTime( int animTree, unsigned short animIndex, float goalWeight ) {
	float rate;

	rate = (float)fabs( trap_XAnimGetWeight( animTree, animIndex ) - goalWeight )
		   * ( 1000.0f / cg.frametime );

	if ( rate > 0.0f ) {
		return 1.0f / rate;
	}
	return 0.0f;
}

/*
===============
CG_PlayerTurretPositionAndBlend

Snap a mounted player onto the turret he is holding.

The player's "mounted" animation is a two-level tree: each child of the root is
one stand height, and each child of THAT is one yaw step.  The yaw picks the
column outright; the height is found by walking the rows until the tag the
animation puts the hands on rises above the turret's tag_weapon, then blending
the two rows either side of it.  Finally the whole player is moved so the
animation's own root delta lines up with the turret grip and dropped onto the
floor underneath.
===============
*/
static void CG_PlayerTurretPositionAndBlend( centity_t *cent ) {
	centity_t       *turret;
	clientInfo_t    *ci;
	animation_t     *anim;
	weaponInfo_t    *weapon;
	scr_anim_t      rootAnim;
	scr_anim_t      row;
	scr_anim_t      col;
	scr_anim_t      prevRow;
	scr_anim_t      prevCol;
	int             animTree;
	int             turretNum;
	int             obj;
	float           *tagMatrix;
	float           yaw;
	vec3_t          axis[4];
	float           localZ;
	float           targetZ;
	float           lastZ;
	int             numRows;
	int             numCols;
	int             row_i;
	int             col_i;
	int             savedCol;
	float           frac;
	float           savedFrac;
	float           colValue;
	float           weight;
	float           rot[2];
	vec3_t          delta;
	float           localMatrix[4][3];
	float           worldMatrix[4][3];
	vec3_t          start, end;
	trace_t         trace;

	turretNum = cent->currentState.otherEntityNum;
	if ( turretNum < MAX_CLIENTS ) {
		return;
	}
	if ( turretNum == ENTITYNUM_NONE ) {
		return;
	}

	ci = &bg_clientinfo[cent->currentState.clientNum];
	if ( !ci->infoValid ) {
		return;
	}
	// retail 0x300279F1/0x300279FF read legs (+0x38C/+0x390), not torso
	if ( !ci->legs.animationNumber ) {
		return;
	}
	anim = ci->legs.animation;
	if ( !anim ) {
		return;
	}
	if ( !( anim->flags & 4 ) ) {
		return;
	}

	turret = &cg_entities[turretNum];
	if ( !turret->currentValid ) {
		return;
	}

	obj = trap_syscall_0xA2( turret->currentState.number );
	if ( !obj ) {
		return;
	}

	tagMatrix = CG_DObjGetLocalTagMatrix( obj, "tag_weapon", turret );
	if ( !tagMatrix ) {
		Com_Printf( "WARNING: aborting player positioning on turret since 'tag_weapon' does not exist\n" );
		return;
	}

	weapon = bg_weaponInfo[turret->currentState.weapon];
	animTree = (int)ci->animTree;

	rootAnim.index = (unsigned short)( ci->legs.animationNumber & ~ANIM_TOGGLEBIT ); // retail 0x30027AA6
	rootAnim.anims = (unsigned short)Scr_GetAnimsIndex( bgs_animScriptData.animTreeIndex );

	yaw = vectosignedyaw( tagMatrix );

	AnglesToAxis( turret->lerpAngles, axis );
	axis[3][0] = turret->lerpOrigin[0];
	axis[3][1] = turret->lerpOrigin[1];
	axis[3][2] = turret->lerpOrigin[2];

	delta[0] = cent->lerpOrigin[0] - axis[3][0];
	delta[1] = cent->lerpOrigin[1] - axis[3][1];
	delta[2] = cent->lerpOrigin[2] - axis[3][2];
	localZ = axis[2][0] * delta[0] + axis[2][1] * delta[1] + axis[2][2] * delta[2];

	targetZ = localZ - tagMatrix[14];

	trap_XAnimClearTreeGoalWeightsStrict( animTree, rootAnim.index, 0.0f );

	numRows = trap_syscall_0xB5( rootAnim );
	if ( !numRows ) {
		Com_Error( 1, "\x15Player anim '%s' has no children",
				   (const char *)trap_XAnimGetAnimName( rootAnim ) );
	}

	lastZ = 0.0f;
	savedFrac = 0.0f;
	savedCol = 0;
	prevCol.index = 0;
	prevCol.anims = 0;
	row_i = 0;
	frac = 0.0f;
	col.index = 0;
	col.anims = 0;
	row.index = 0;
	row.anims = 0;

	do {
		row = trap_syscall_0xB6( rootAnim, row_i );
		trap_syscall_0x8C( animTree, row.index, 1.0f, 1.0f, 1.0f, 0, 0 );

		numCols = trap_syscall_0xB5( row );
		if ( !numCols ) {
			Com_Error( 1, "\x15Player anim '%s' has no children",
					   (const char *)trap_XAnimGetAnimName( row ) );
		}

		colValue = numCols * 0.5f - yaw / weapon->animHorRotateInc;
		if ( colValue < 0.0f ) {
			colValue = 0.0f;
		} else if ( colValue >= (float)( numCols - 1 ) ) {
			colValue = (float)( numCols - 1 );
		}

		col_i = (int)colValue;
		frac = colValue - col_i;

		col = trap_syscall_0xB6( row, col_i );
		trap_syscall_0x8C( animTree, col.index, 1.0f - frac, 1.0f, 1.0f, 0, 0 );

		if ( frac != 0.0f ) {
			prevCol = trap_syscall_0xB6( row, col_i + 1 );
			trap_syscall_0x8C( animTree, prevCol.index, frac, 1.0f, 1.0f, 0, 0 );
		}

		trap_syscall_0x9A( animTree, rootAnim.index, (int)rot, (int)delta );

		if ( delta[2] >= targetZ ) {
			break;
		}

		lastZ = delta[2];
		savedFrac = frac;
		savedCol = col_i;
		row_i++;
	} while ( row_i < numRows );

	trap_XAnimClearTreeGoalWeightsStrict( animTree, rootAnim.index, 0.0f );

	weight = 1.0f - frac;
	trap_syscall_0x8C( animTree, col.index, weight,
					   CG_PlayerTurretGoalTime( animTree, col.index, weight ), 1.0f, 0, 0 );

	if ( frac != 0.0f ) {
		trap_syscall_0x8C( animTree, prevCol.index, frac,
						   CG_PlayerTurretGoalTime( animTree, prevCol.index, frac ),
						   1.0f, 0, 0 );
	}

	if ( row_i && row_i != numRows ) {
		// blend the two rows the target height falls between
		weight = ( targetZ - lastZ ) / ( delta[2] - lastZ );
		trap_syscall_0x8C( animTree, row.index, weight,
						   CG_PlayerTurretGoalTime( animTree, row.index, weight ), 1.0f, 0, 0 );

		prevRow = trap_syscall_0xB6( rootAnim, row_i - 1 );
		trap_syscall_0x8C( animTree, prevRow.index, 1.0f - weight,
						   CG_PlayerTurretGoalTime( animTree, prevRow.index, 1.0f - weight ),
						   1.0f, 0, 0 );

		col = trap_syscall_0xB6( prevRow, savedCol );
		trap_syscall_0x8C( animTree, col.index, 1.0f - savedFrac,
						   CG_PlayerTurretGoalTime( animTree, col.index, 1.0f - savedFrac ),
						   1.0f, 0, 0 );

		if ( savedFrac != 0.0f ) {
			prevCol = trap_syscall_0xB6( prevRow, savedCol + 1 );
			trap_syscall_0x8C( animTree, prevCol.index, savedFrac,
							   CG_PlayerTurretGoalTime( animTree, prevCol.index, savedFrac ),
							   1.0f, 0, 0 );
		}
	} else {
		if ( !CG_DObjGetLocalTagMatrix( obj, "tag_aim", turret ) ) {
			Com_Printf( "WARNING: aborting player positioning on turret since 'tag_aim' does not exist\n" );
			return;
		}
		trap_syscall_0x8C( animTree, row.index, 1.0f,
						   CG_PlayerTurretGoalTime( animTree, row.index, 1.0f ), 1.0f, 0, 0 );
	}

	trap_syscall_0x9A( animTree, rootAnim.index, (int)rot, (int)delta );

	RotateVector2D( delta, yaw );

	localMatrix[3][0] = delta[0] + tagMatrix[12];
	localMatrix[3][1] = delta[1] + tagMatrix[13];
	localMatrix[3][2] = localZ;
	YawToAxis( RotationToYaw( rot ) + yaw, localMatrix );

	// retail 0x3002813A loads eax=axis, ecx=localMatrix, edx=worldMatrix; with
	// MatrixMultiply43's (in1,in2,out)->(ecx,eax,edx) mapping that is
	// (localMatrix, axis, worldMatrix): localOrigin.turretRot, not
	// turretOrigin.localRot
	MatrixMultiply43( localMatrix, axis, worldMatrix );
	AxisToAngles( worldMatrix, cent->lerpAngles );

	if ( cg_debuganim.integer == 5 ) {
		return;
	}

	cent->lerpOrigin[0] = worldMatrix[3][0];
	cent->lerpOrigin[1] = worldMatrix[3][1];
	cent->lerpOrigin[2] = worldMatrix[3][2];

	// retail 0x300281BA start[2] = turret z, 0x300281B0 end[2] = world z:
	// a downward trace that drops the player onto the floor under the turret
	start[0] = worldMatrix[3][0];
	start[1] = worldMatrix[3][1];
	start[2] = turret->lerpOrigin[2];
	end[0] = worldMatrix[3][0];
	end[1] = worldMatrix[3][1];
	end[2] = worldMatrix[3][2];

	CG_Trace( &trace, start, NULL, NULL, end, cent->currentState.number,
			  MASK_PLAYERSOLID_CG );
	if ( trace.fraction < 1.0f ) {
		cent->lerpOrigin[2] = trace.endpos[2];
	}
}

/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) {
	entityState_t   *es;
	clientInfo_t    *ci;
	refEntity_t     ent;
	int             obj;
	int             renderfx;
	float           shadowPlane;

	es = &cent->nextState;

	if ( es->eFlags & EF_NODRAW ) {
		return;
	}
	if ( es->eFlags & EF_DEAD ) {
		return;
	}

	obj = trap_syscall_0xA2( es->number );
	if ( !obj ) {
		return;
	}

	memset( &ent, 0, sizeof( ent ) );
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;

	ci = &bg_clientinfo[es->clientNum];
	BG_PlayerAnimation( ci, obj, es );

	if ( es->eFlags & EF_MOUNTED ) {
		CG_PlayerTurretPositionAndBlend( cent );
	}

	AnglesToAxis( cent->lerpAngles, ent.axis );

	CG_PlayerSprites( cent );
	CG_PlayerSplash( cent );

	renderfx = 0;
	if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
		 && es->number == cg.snap->ps.clientNum
		 && !cg.renderingThirdPerson ) {
		renderfx = RF_THIRD_PERSON;         // only draw in mirrors
	}
	if ( cg_drawOwnPlayerModel ) {
		renderfx &= ~RF_THIRD_PERSON;
	}
	renderfx |= RF_LIGHTING_ORIGIN;         // use the same origin for all parts

	ent.origin[0] = cent->lerpOrigin[0];
	ent.origin[1] = cent->lerpOrigin[1];
	ent.origin[2] = cent->lerpOrigin[2];
	ent.oldorigin[0] = cent->lerpOrigin[0];
	ent.oldorigin[1] = cent->lerpOrigin[1];
	ent.oldorigin[2] = cent->lerpOrigin[2];

	RE_LIGHTINGORIGIN( &ent )[0] = cent->lerpOrigin[0];
	RE_LIGHTINGORIGIN( &ent )[1] = cent->lerpOrigin[1];
	if ( es->eFlags & EF_PRONE ) {
		RE_LIGHTINGORIGIN( &ent )[2] = cent->lerpOrigin[2] + es->fTorsoHeight + 12.0f;
	} else if ( es->eFlags & EF_CROUCHING ) {
		RE_LIGHTINGORIGIN( &ent )[2] = cent->lerpOrigin[2] + es->fTorsoHeight + 20.0f;
	} else {
		RE_LIGHTINGORIGIN( &ent )[2] = cent->lerpOrigin[2] + es->fTorsoHeight + 32.0f;
	}

	ent.reType = RT_DOBJ;
	ent.renderfx = renderfx;
	RE_DOBJ( &ent ) = obj;
	RE_OWNER( &ent ) = cent;

	trap_R_AddRefEntityToScene( &ent );

	CG_PlayerShadow( &shadowPlane, cent );

	if ( !( es->eFlags & EF_DEAD ) ) {
		CG_AddPlayerWeapon( &ent, NULL, cent, 1 );
	}
}

/*
===============
CG_Corpse

An ET_CORPSE keeps its own frozen clientInfo_t in cg_corpseinfo, so the DObj
has to be brought up to date here rather than in CG_UpdatePlayerDObj.
===============
*/
void CG_Corpse( centity_t *cent ) {
	entityState_t   *es;
	clientInfo_t    *ci;
	refEntity_t     ent;
	int             obj;

	es = &cent->nextState;

	if ( es->eFlags & EF_NODRAW ) {
		return;
	}

	ci = &cg_corpseinfo[es->number - MAX_CLIENTS];

	obj = trap_syscall_0xA2( es->number );
	BG_UpdatePlayerDObj( obj, es, ci );

	obj = trap_syscall_0xA2( es->number );
	if ( !obj ) {
		return;
	}

	memset( &ent, 0, sizeof( ent ) );
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;

	BG_PlayerAnimation( ci, obj, es );

	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.origin[0] = cent->lerpOrigin[0];
	ent.origin[1] = cent->lerpOrigin[1];
	ent.origin[2] = cent->lerpOrigin[2];
	ent.oldorigin[0] = cent->lerpOrigin[0];
	ent.oldorigin[1] = cent->lerpOrigin[1];
	ent.oldorigin[2] = cent->lerpOrigin[2];

	RE_LIGHTINGORIGIN( &ent )[0] = cent->lerpOrigin[0];
	RE_LIGHTINGORIGIN( &ent )[1] = cent->lerpOrigin[1];
	if ( es->eFlags & EF_PRONE ) {
		RE_LIGHTINGORIGIN( &ent )[2] = cent->lerpOrigin[2] + es->fTorsoHeight + 12.0f;
	} else if ( es->eFlags & EF_CROUCHING ) {
		RE_LIGHTINGORIGIN( &ent )[2] = cent->lerpOrigin[2] + es->fTorsoHeight + 20.0f;
	} else {
		RE_LIGHTINGORIGIN( &ent )[2] = cent->lerpOrigin[2] + es->fTorsoHeight + 32.0f;
	}

	ent.reType = RT_DOBJ;
	ent.renderfx = RF_LIGHTING_ORIGIN;
	RE_DOBJ( &ent ) = obj;
	RE_OWNER( &ent ) = cent;

	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_UpdatePlayerDObj

No call site in 1.1.
===============
*/
void CG_UpdatePlayerDObj( centity_t *cent ) {
	int obj;

	if ( !cent->currentValid ) {
		return;
	}

	obj = trap_syscall_0xA2( cent->nextState.clientNum );
	BG_UpdatePlayerDObj( obj, &cent->nextState,
						 &bg_clientinfo[cent->nextState.clientNum] );
}

/*
===============
CG_ResetPlayerEntity

A player just teleported or connected: clear the animation blend and put the
swing angles where the view already is.  RTCW's, with the goal weights added
for the XAnim tree.
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	clientInfo_t *ci;

	ci = &bg_clientinfo[cent->currentState.clientNum];

	if ( !( cent->currentState.eFlags & EF_DEAD ) ) {
		trap_XAnimClearTreeGoalWeights( (int)ci->animTree, bgs_rootAnim.index, 0.0f );
		trap_XAnimSetCompleteGoalWeight( (int)ci->animTree,
										 bgs_animScriptData.torsoAnim.index,
										 0.0f, 0.0f, 1.0f, 0, 0 );
		trap_XAnimSetCompleteGoalWeight( (int)ci->animTree,
										 bgs_animScriptData.legsAnim.index,
										 1.0f, 0.0f, 1.0f, 0, 0 );
		trap_XAnimSetCompleteGoalWeight( (int)ci->animTree,
										 bgs_animScriptData.turningAnim.index,
										 0.0f, 0.0f, 1.0f, 0, 0 );

		memset( &ci->legs, 0, sizeof( ci->legs ) );
		ci->legs.yawAngle = ci->viewYaw;
		ci->legs.yawing = qfalse;
		ci->legs.pitchAngle = 0;
		ci->legs.pitching = qfalse;

		memset( &ci->torso, 0, sizeof( ci->torso ) );
		ci->torso.yawAngle = ci->viewYaw;
		ci->torso.yawing = qfalse;
		ci->torso.pitchAngle = ci->viewPitch;
		ci->torso.pitching = qfalse;
	}

	if ( cg_debugposition.integer ) {
		/* the retail build really does hand a float to %i here */
		CG_Printf( "%i ResetPlayerEntity yaw=%i\n", cent->currentState.number,
				   ci->torso.yawAngle );
	}
}
