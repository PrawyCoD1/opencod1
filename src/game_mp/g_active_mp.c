/*
 * g_active_mp.c -- the per-client frame: ClientThink and everything it runs
 * through, plus ClientEndFrame and the presentation it rebuilds.
 *
 * Call of Duty 1.1 multiplayer (game_mp_x86.dll, 0x20017160 .. 0x20019124).
 * RTCW's game/g_active.c is the ancestor of P_DamageFeedback, ClientImpacts,
 * G_TouchTriggers, SpectatorThink, ClientInactivityTimer,
 * ClientIntermissionThink, ClientEvents, ClientThink_real, ClientThink,
 * G_RunClient and ClientEndFrame.  What CoD changed:
 *
 *   - the whole weapon/view kick layer: ClientThink_real builds bg_weapon.c's
 *     bgViewAngleState_t and pmWeaponAngleState_t out of the gclient_t tail
 *     before Pmove and writes the result back afterwards, and the aim angles
 *     the fire path uses (gclient_t +0x220C/+0x2210) come out of it;
 *   - the touch path notifies the script both ways rather than only calling
 *     the touch function;
 *   - ClientEndFrame drives the DObj/XAnim player model: it refreshes
 *     bg_clientinfo[] from the entity's model and attachments, frees the DObj
 *     when any of them changed, then runs BG_UpdatePlayerDObj and
 *     BG_PlayerAnimation;
 *   - spectators replay an archived playerState (SpectatorClientEndFrame /
 *     GetFollowPlayerState) instead of RTCW's live copy;
 *   - StuckInClient is new: two clients wedged in each other get pushed apart.
 *
 * Names not recovered from the binary are flagged where they are defined.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <math.h>

#include "g_local.h"

/*
 * Missing from universal/q_shared.h; every unit in this module open-codes
 * them.
 */
#define PITCH                       0
#define YAW                         1
#define ROLL                        2

/* usercmd_t.buttons.  0x01 is bg_animation.c's BUTTON_ATTACK; 0x20 and 0x40
   are Melee and the activate button, neither named. */
#define BUTTON_NONE        	0

#define BUTTON_FORWARD        		127
#define BUTTON_BACK           		-127
#define BUTTON_MOVERIGHT      		127
#define BUTTON_MOVELEFT       		-127
#define BUTTON_JUMP           		127 // upmove. prone and jump = -KEY_MASK_JUMP

#define BUTTON_ATTACK               0x01
#define BUTTON_RELOAD         		0x08
#define BUTTON_ADS		       		0x10
#define BUTTON_MELEE          		0x20
#define BUTTON_ACTIVATE       		0x40

#define WBUTTON_LEANLEFT       		0x10 // wbuttons
#define WBUTTON_LEANRIGHT      		0x20 // wbuttons
#define WBUTTON_PRONE          		0x40 // wbuttons
#define WBUTTON_CROUCH         		0x80 // wbuttons


/* playerState_t.pm_flags.  PMF_DUCKED and PMF_TIME_LAND are bg_pmove.c's; the
   three names below are inferred --
   0x10000 (SpectatorThink's "no pmove" gate), 0x20000 (raised while the
   spectator is locked to a forced client) and 0x40000 (raised for a client
   that is actually in the world -- GetFollowPlayerState refuses to hand out a
   playerState without it) have no recovered name. */
#define PMF_DUCKED                  0x00000002
#define PMF_TIME_LAND               0x00000100
#define PMF_NOMOVE                  0x00010000
#define PMF_FOLLOWLOCKED            0x00020000
#define PMF_INWORLD                 0x00040000

/* eFlags.  EF_DEAD/EF_FIRING are bg_animation.c's, EF_TALK bg_pmove.c's; the
   rest are unnamed: 0x1000 is RTCW's EF_CONNECTION slot, 0x80000 is the
   marker gclient_t.pingTime times out (ClientEndFrame 0x20018C5B) and 0x100000
   relays it to the viewer whose compass is showing that friendly. */
#define EF_DEAD                     0x00000001
#define EF_FIRING                   0x00000400
#define EF_CONNECTION               0x00001000
#define EF_TURRET_ACTIVE_MASK       0x0000C000
#define EF_TALK                     0x00040000
#define EF_COMPASS_MARK             0x00080000
#define EF_COMPASS_MARK_FRIEND      0x00100000

/* entityShared_t.svFlags; SVF_NOCLIENT is the engine side's (sv_game_mp.c:38),
   SVF_CAPSULE sv_world_mp.c:29.  Bit 1, which the end-frame paths set for a
   live player and clear for a dead one, has no name on either side. */
#define SVF_NOCLIENT                0x00000001
#define SVF_VISIBLE_PLAYER          0x00000002
#define SVF_CAPSULE                 0x00000200

#define CONTENTS_BODY               0x02000000
#define CONTENTS_CORPSE             0x04000000

/* The three player content masks this unit hands Pmove; all three names are
   inferred.  MASK_CLIENTSOLID is
   bg_pmove.c's MASK_PLAYERSOLID (0x02810191) without 0x80 and 0x100;
   MASK_DEADSOLID and MASK_SPECTATORSOLID drop CONTENTS_BODY and 0x800000 in
   turn.  No MASK_* name is recovered for any of them. */
#define MASK_CLIENTSOLID            0x02810011
#define MASK_DEADSOLID              0x00810011
#define MASK_SPECTATORSOLID         0x00800011

/* The trigger mask G_TouchTriggers scans with (0x2001748C).  0x40000000 is
   g_trigger_mp.c's CONTENTS_TRIGGER. */
#define MASK_TRIGGERSEARCH          0x405C0008

#define ET_PLAYER                   1
#define ET_ITEM                     3

#define TR_STATIONARY               0
#define TR_INTERPOLATE              1

#define CS_MODELS                   268

#define MOD_FALLING                 21
#define HITLOC_NONE                 0

/* entity_event_t indices; the names are eventnames[]'s own strings
   (bg_misc.c, 0x2006C8B8), which is the only place the enum survives. */
#define EV_LANDING_PAIN_DEFAULT     116
#define EV_LANDING_PAIN_ASPHALT     138
#define EV_FIRE_WEAPON              159
#define EV_FIRE_WEAPONB             160
#define EV_FIRE_WEAPON_LASTSHOT     161
#define EV_FIRE_MELEE               165
#define EV_FIRE_WEAPON_MG42         168
#define EV_PAIN                     187
#define EV_DROPWEAPON               196

/* playerState_t.stats[] indices; see the note in g_client_mp.c. */
#define STAT_HEALTH                 0
#define STAT_MAX_HEALTH             2

/*
 * gclient_t fields g_local.h still spells `unknown_`: the aim pitch/yaw pair
 * the fire path builds its axis from, the spread lerp fraction, the int
 * ClientEndFrame clears and never reads, and the entity number
 * G_GetNonPVSFriendlyInfo resumes its scan from.
 */
#define CLIENT_AIMPITCH( c )        ( *(float *)&( c )->unknown_0x220C[0] )     /* +0x220C */
#define CLIENT_AIMYAW( c )          ( *(float *)&( c )->unknown_0x220C[4] )     /* +0x2210 */
#define CLIENT_SPREADFRAC( c )      ( *(float *)&( c )->unknown_0x223C[4] )     /* +0x2240 */
#define CLIENT_ENDFRAMERESET( c )   ( *(int *)&( c )->unknown_0x223C[16] )      /* +0x224C, cleared every frame */
#define CLIENT_COMPASSSCANENT( c )  ( *(int *)&( c )->unknown_0x2264[0] )       /* +0x2264 */


/* No header covers the q_shared / q_math half of the module, and bg_public.h
 * does not declare bg_misc.c's or bg_weapon.c's exports. */
void        Q_strncpyz( char *dest, const char *src, int destsize );
void        vectoangles( const vec3_t value1, vec3_t angles );
void        AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void        AxisToAngles( const vec3_t axis[3], vec3_t angles );
void        MatrixMultiply( const vec3_t in1[3], const vec3_t in2[3], vec3_t out[3] );
/* Destination is the MIDDLE argument; see g_utils_mp.c. */
void        MatrixTransformVector43( const void *mat, vec3_t out, const vec3_t in );
float       VectorNormalize2D( vec3_t v );

qboolean    BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );
float       BG_GetSpeed( const playerState_t *ps, int time );
void        BG_CalculateViewAngles( bgViewAngleState_t *state, vec3_t angles );
void        BG_CalculateWeaponAngles( pmWeaponAngleState_t *state, vec3_t angles );
void        BG_CalculateWeaponPosition_Sway( vec3_t swayOffsets, vec3_t swayAngles,
											 const playerState_t *ps, vec3_t previousViewAngles,
											 float scale, int msec );
/* BG_UpdatePlayerDObj, BG_PlayerAnimation and BG_Player_DoControllers are
   declared in bg_public.h and nowhere else: the DObj parameter is a
   gentity_t * here and an int obj in cgame, so no private prototype here. */

/* forward: g_client_mp.c */
void        ClientSpawn( gentity_t *ent, const vec3_t origin, const vec3_t angles );
void        G_AddLean( gentity_t *ent, vec3_t pos );
int         G_GetNonPVSFriendlyInfo( gentity_t *ent, const vec3_t origin, int lastEntNum );

/* forward: g_cmds_mp.c */
void        StopFollowing( gentity_t *ent );
qboolean    Cmd_FollowCycle_f( gentity_t *ent, int dir );
qboolean    Cmd_Activate_f( gentity_t *ent );

/* forward: g_main_mp.c -- the vmCvar_t objects G_RegisterCvars owns. */
extern vmCvar_t g_inactivity;                   /* 0x20235D20 */
extern vmCvar_t g_speed;                        /* 0x20236500 */
extern vmCvar_t g_gravity;                      /* 0x2016EFE0 */
extern vmCvar_t g_synchronousClients;           /* 0x20234C40 */
extern vmCvar_t g_debugMove;                    /* 0x20234340 */
extern vmCvar_t g_smoothClients;                /* 0x2016EEC0 */
extern vmCvar_t pmove_fixed;                    /* 0x20236F20 */
extern vmCvar_t pmove_msec;                     /* 0x202347C0 */
extern vmCvar_t g_debugLocDamage;               /* 0x20236E00 */

/*
===============
P_DamageFeedback

Called just before a snapshot is sent to the given player.  Totals up all
damage and generates both the player_pain sound and the damage kick the view
and the weapon inherit.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t   *client;
	float count;
	vec3_t angles;
	vec3_t axis[3];
	float kick;
	int percent;

	client = player->client;
	if ( client->ps.pm_type >= PM_DEAD ) {
		return;
	}

	/* total points of damage shot at the player this frame */
	if ( client->damage_blood <= 0 ) {
		return;
	}
	if ( client->sess.maxHealth <= 0 ) {
		return;
	}

	percent = 100 * client->damage_blood / client->sess.maxHealth;
	if ( percent > 127 ) {
		percent = 127;
	}
	count = (float)percent;

	client->ps.aimSpreadScale += count;
	if ( client->ps.aimSpreadScale > 255.0f ) {
		client->ps.aimSpreadScale = 255.0f;
	}

	kick = count * 0.2f;
	if ( kick < 5.0f ) {
		kick = 5.0f;
	} else if ( kick > 90.0f ) {
		kick = 90.0f;
	}

	/* play an apropriate pain sound */
	if ( client->damage_fromWorld ) {
		client->viewKickYaw = 0.0f;
		client->viewKickPitch = -kick;
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;
		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		AnglesToAxis( client->ps.viewangles, axis );
		client->viewKickYaw = -( DotProduct( axis[1], client->damage_from ) * kick );
		client->viewKickPitch = DotProduct( axis[0], client->damage_from ) * kick;
		client->ps.damagePitch = (int)( angles[PITCH] * ( 256.0f / 360.0f ) );
		client->ps.damageYaw = (int)( angles[YAW] * ( 256.0f / 360.0f ) );
	}

	if ( level.time > player->painDebounceTime && !( player->flags & FL_GODMODE ) ) {
		int healthPercent = (int)( (float)client->ps.stats[STAT_HEALTH]
						 / (float)client->ps.stats[STAT_MAX_HEALTH] * 100.0f );
		if ( healthPercent < 0 ) {
			healthPercent = 0;
		} else if ( healthPercent > 100 ) {
			healthPercent = 100;
		}
		G_AddEvent( player, EV_PAIN, healthPercent );
		player->painDebounceTime = level.time + 700;
	}

	client->ps.damageEvent++;
	client->ps.damageCount = percent;
	client->viewKickStartTime = level.time - 20;

	/* clear totals */
	client->damage_blood = 0;
}

/*
=============
G_SetClientSound

CoD cut RTCW's water/lava handling; nothing is left but the clear.  No call
site survives the inlining -- ClientEndFrame's is the only one.
=============
*/
void G_SetClientSound( gentity_t *ent ) {
	ent->s.loopSound = 0;
}

/*
==============
ClientImpacts

Run the touch callbacks and the script `touch` notifies for everything Pmove
bumped into.
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int i, j;
	gentity_t   *other;

	for ( i = 0; i < pm->numtouch; i++ ) {
		for ( j = 0; j < i; j++ ) {
			if ( pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if ( j != i ) {
			continue;       /* duplicated */
		}

		other = &g_entities[pm->touchents[i]];

		if ( Scr_IsSystemActive( 1 ) ) {
			Scr_AddEntityNum( other->s.number, 0 );
			Scr_NotifyNum( ent->s.number, 0, scr_const.touch, 1 );
			Scr_AddEntityNum( ent->s.number, 0 );
			Scr_NotifyNum( other->s.number, 0, scr_const.touch, 1 );
		}

		if ( other->touch ) {
			other->touch( other, ent, 1 );
		}

		if ( ent->touch ) {
			ent->touch( ent, other, 1 );
		}
	}
}

/*
============
G_TouchTriggers

Find all trigger entities that the player is touching.
============
*/
void G_TouchTriggers( gentity_t *ent ) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	vec3_t mins, maxs;
	static vec3_t range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	/* dead clients don't activate triggers! */
	if ( ent->client->ps.pm_type > PM_NORMAL_LINKED ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES, MASK_TRIGGERSEARCH );

	/* can't use ent->absmin, because that has a one unit pad */
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i = 0; i < num; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}

		/* use seperate code for determining if an item is picked up so you
		   don't have to actually contact its bounding box */
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		if ( Scr_IsSystemActive( 1 ) ) {
			Scr_AddEntityNum( ent->s.number, 0 );
			Scr_NotifyNum( hit->s.number, 0, scr_const.touch, 1 );
			Scr_AddEntityNum( hit->s.number, 0 );
			Scr_NotifyNum( ent->s.number, 0, scr_const.touch, 1 );
		}

		if ( hit->touch ) {
			hit->touch( hit, ent, 1 );
		}
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t pm;
	gclient_t   *client;

	client = ent->client;

	client->oldbuttons = client->buttons;
	client->buttons = client->sess.cmd.buttons;
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = client->sess.cmd.wbuttons;

	if ( client->sess.forceSpectatorClient < 0 && client->spectatorClient >= 0
		 && ( ( client->buttons ^ client->oldbuttons ) & BUTTON_ADS ) ) {
		StopFollowing( ent );
	}

	if ( ( client->buttons & BUTTON_ATTACK ) && !( client->oldbuttons & BUTTON_ATTACK ) ) {
		Cmd_FollowCycle_f( ent, 1 );
	} else if ( ( client->buttons & BUTTON_MELEE )
				&& !( client->oldbuttons & BUTTON_MELEE ) ) {
		Cmd_FollowCycle_f( ent, -1 );
	}

	if ( client->ps.pm_flags & PMF_NOMOVE ) {
		return;
	}

	client->ps.pm_type = PM_SPECTATOR;
	client->ps.speed = 400;

	memset( &pm, 0, sizeof( pm ) );
	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	pm.tracemask = MASK_SPECTATORSOLID;
	pm.trace = trap_TraceCapsule;
	pm.trace2 = trap_TraceCapsule;
	pm.trace3 = trap_TraceCapsule;
	pm.pointcontents = trap_PointContents;

	Pmove( &pm );

	VectorCopy( client->ps.origin, ent->r.currentOrigin );

	trap_UnlinkEntity( ent );
}

/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped.
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( !g_inactivity.integer ) {
		/* give everyone some time, so if the operator sets g_inactivity during
		   gameplay, everyone isn't kicked */
		client->inactivityTime = level.time + 60000;
		client->inactivityWarning = qfalse;
	} else if ( client->sess.cmd.forwardmove || client->sess.cmd.rightmove
				|| client->sess.cmd.upmove || ( client->sess.cmd.buttons & BUTTON_ATTACK ) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->sess.localClient ) {
		if ( level.time > client->inactivityTime ) {
			trap_DropClient( client - level.clients, "GAME_DROPPEDFORINACTIVITY" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, 0,
									"c \"GAME_INACTIVEDROPWARNING\"" );
		}
	}
	return qtrue;
}

/*
================
ClientIntermissionThink

CoD dropped RTCW's "any key exits" handling; only the button latching is left.
================
*/
void ClientIntermissionThink( gentity_t *ent ) {
	gclient_t   *client;

	client = ent->client;

	client->oldbuttons = client->buttons;
	client->buttons = client->sess.cmd.buttons;
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = client->sess.cmd.wbuttons;
}

/*
================
ClientEvents

Events will be passed on to the clients for presentation, but any server game
effects are handled here.
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int i;
	int event;
	gclient_t   *client;
	int damage;
	float ratio;
	int weapon;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - 4 ) {
		oldEventSequence = client->ps.eventSequence - 4;
	}

	for ( i = oldEventSequence; i < client->ps.eventSequence; i++ ) {
		event = client->ps.events[i & 3];

		if ( event >= EV_LANDING_PAIN_DEFAULT && event <= EV_LANDING_PAIN_ASPHALT ) {
			/* the retail loop returns here rather than skipping the event */
			if ( ent->s.eType != ET_PLAYER ) {
				return;
			}
			if ( client->ps.eventParms[i & 3] >= 100 ) {
				ratio = 1.1f;
			} else {
				ratio = client->ps.eventParms[i & 3] * 0.01f;
			}
			if ( ratio != 0.0f ) {
				damage = (int)( ratio * client->ps.stats[STAT_MAX_HEALTH] );
				ent->painDebounceTime = level.time + 200;
				G_Damage( ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING, HITLOC_NONE );
			}
			continue;
		}

		switch ( event ) {
		case EV_FIRE_WEAPON:
		case EV_FIRE_WEAPONB:
		case EV_FIRE_WEAPON_LASTSHOT:
		case EV_FIRE_WEAPON_MG42:
			FireWeapon( ent );
			break;

		case EV_FIRE_MELEE:
			FireWeaponMelee( ent );
			break;

		case EV_DROPWEAPON:
			weapon = ent->client->ps.weapon;
			if ( weapon && ( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
				Drop_Weapon( ent, weapon, "tag_weapon_right" );
			}
			break;

		default:
			break;
		}
	}
}

/*
==============
G_SetClientContents
==============
*/
void G_SetClientContents( gentity_t *ent ) {
	if ( ent->client->noclip || ent->client->ufo ) {
		ent->r.contents = 0;
	} else if ( ent->client->sess.sessionState == STATE_DEAD ) {
		ent->r.contents = 0;
	} else {
		ent->r.contents = CONTENTS_BODY;
	}
}

/*
==============
ClientThink_real

This will be called once for each client frame, which will usually be a couple
times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly once for each
server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent, usercmd_t *ucmd ) {
	gclient_t   *client;
	pmove_t pm;
	int oldEventSequence;
	int msec;
	bgViewAngleState_t viewState;
	pmWeaponAngleState_t weaponState;
	vec3_t angles;
	vec3_t aimAngles;
	vec3_t weaponAxis[3], aimAxis[3], axis[3];

	client = ent->client;

	/* don't think if the client is not yet connected (and thus not yet
	   spawned in) */
	if ( client->sess.connected != CON_CONNECTED ) {
		return;
	}

	/* mark the time, so the connection sprite can be removed */
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
	}

	msec = ucmd->serverTime - client->ps.commandTime;
	/* following others may result in bad times, but we still want to check for
	   follow toggles */
	if ( msec < 1 && client->ps.clientNum == ent - g_entities ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set( "pmove_msec", "8" );
	} else if ( pmove_msec.integer > 33 ) {
		trap_Cvar_Set( "pmove_msec", "33" );
	}

	if ( pmove_fixed.integer || client->sess.pmoveFixed ) {
		ucmd->serverTime = ( ( ucmd->serverTime + pmove_msec.integer - 1 ) / pmove_msec.integer )
						   * pmove_msec.integer;
	}

	if ( client->sess.sessionState == STATE_INTERMISSION ) {
		ClientIntermissionThink( ent );
		return;
	}

	if ( client->sess.sessionState == STATE_SPECTATOR ) {
		SpectatorThink( ent, ucmd );
		return;
	}

	/* check for inactivity timer, but never drop the local client of a
	   non-dedicated server */
	if ( !ClientInactivityTimer( client ) ) {
		return;
	}

	oldEventSequence = client->ps.eventSequence;

	memset( &pm, 0, sizeof( pm ) );
	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	pm.oldcmd = client->sess.oldcmd;
	pm.pointcontents = trap_PointContents;
	pm.trace = trap_TraceCapsule;
	pm.trace2 = trap_TraceCapsule;
	pm.trace3 = trap_TraceCapsule;
	pm.debugLevel = g_debugMove.integer;
	if ( client->ps.pm_type < PM_DEAD ) {
		pm.tracemask = MASK_CLIENTSOLID;
	} else {
		pm.tracemask = MASK_DEADSOLID;
	}
	pm.pmove_msec = pmove_msec.integer;
	pm.pmove_fixed = pmove_fixed.integer | client->sess.pmoveFixed;

	VectorCopy( client->ps.origin, client->oldOrigin );

	client->oldbuttons = client->buttons;
	client->buttons = client->sess.cmd.buttons;
	client->latched_buttons = client->buttons & ~client->oldbuttons;
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = client->sess.cmd.wbuttons;
	client->latched_wbuttons = client->wbuttons & ~client->oldwbuttons;

	/* the damage kick the view inherits, and the weapon offsets and recoil the
	   fire path aims down */
	viewState.ps = &client->ps;
	viewState.viewKickStartTime = client->viewKickStartTime;
	viewState.time = level.time;
	viewState.viewKickPitch = client->viewKickPitch;
	viewState.viewKickRoll = client->viewKickYaw;
	viewState.speed = BG_GetSpeed( &client->ps, level.time );

	BG_CalculateViewAngles( &viewState, angles );

	aimAngles[PITCH] = angles[PITCH] + client->ps.viewangles[PITCH];
	aimAngles[YAW] = angles[YAW] + client->ps.viewangles[YAW];
	aimAngles[ROLL] = angles[ROLL] + client->ps.viewangles[ROLL];

	BG_CalculateWeaponPosition_Sway( client->swayOffsets, client->swayAngles,
									 &client->ps, client->previousViewAngles, 1.0f, msec );

	weaponState.ps = &client->ps;
	weaponState.speed = viewState.speed;
	weaponState.frametime = (float)msec * 0.001f;
	VectorCopy( client->moveOffset, weaponState.moveOffset );
	weaponState.idleScale = client->idleScale;
	weaponState.time = viewState.time;
	weaponState.viewKickStartTime = client->viewKickStartTime;
	weaponState.viewKickPitch = client->viewKickPitch;
	weaponState.viewKickYaw = client->viewKickYaw;
	weaponState.recoilPitch = client->recoilPitch;
	weaponState.recoilYaw = client->recoilYaw;
	weaponState.recoilRoll = client->recoilRoll;
	weaponState.recoilPitchSpeed = client->recoilPitchSpeed;
	weaponState.recoilYawSpeed = client->recoilYawSpeed;
	weaponState.recoilState = client->recoilState;
	/* retail 0x20017E90..0x20017EB6 loads baseAngles from the sway angle (+0x2290),
	   the small turn-rate offset -- NOT previousViewAngles (+0x2278). BG_Calculate
	   WeaponAngles subtracts baseAngles from the accumulated weapon angle, so the
	   remainder is a small view-relative offset. */
	VectorCopy( client->swayAngles, weaponState.baseAngles );

	BG_CalculateWeaponAngles( &weaponState, angles );

	/* an aim-down-sight weapon aims along the gun, not along the view */
	if ( bg_weaponInfo[client->ps.weapon]->aimDownSight && client->ps.fWeaponPosFrac != 0.0f ) {
		AnglesToAxis( angles, weaponAxis );
		AnglesToAxis( aimAngles, aimAxis );
		/* retail 0x20017F24: in1=weaponAxis (ecx), in2=aimAxis (eax) -- composes the
		   weapon's view-relative offset onto the view frame; the reversed operand
		   order would aim every ADS shot at a fixed world direction */
		MatrixMultiply( weaponAxis, aimAxis, axis );
		AxisToAngles( axis, aimAngles );
	}

	VectorCopy( weaponState.moveOffset, client->moveOffset );
	client->idleScale = weaponState.idleScale;
	client->recoilPitch = weaponState.recoilPitch;
	client->recoilYaw = weaponState.recoilYaw;
	client->recoilRoll = weaponState.recoilRoll;
	client->recoilPitchSpeed = weaponState.recoilPitchSpeed;
	client->recoilYawSpeed = weaponState.recoilYawSpeed;
	client->recoilState = weaponState.recoilState;

	CLIENT_AIMPITCH( client ) = aimAngles[PITCH];
	CLIENT_AIMYAW( client ) = aimAngles[YAW];

	Pmove( &pm );

	ent->s.animMovetype = ( client->ps.pm_flags & PMF_DUCKED ) != 0;

	/* execute client events */
	if ( client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
		ent->r.eventTime = level.time;
	}

	/* save results of pmove */
	if ( g_smoothClients.integer ) {
		BG_PlayerStateToEntityStateExtrapolate( &client->ps, &ent->s, client->ps.commandTime, qtrue );
	} else {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
	}

	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
	VectorCopy( pm.mins, ent->r.mins );
	VectorCopy( pm.maxs, ent->r.maxs );

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	ClientEvents( ent, oldEventSequence );

	/* link entity now, after any personal teleporters have been used */
	trap_LinkEntity( ent );

	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	/* NOTE: now copy the exact origin over otherwise clients can be snapped
	   into solid */
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	VectorClear( ent->r.currentAngles );
	ent->r.currentAngles[YAW] = ent->client->ps.viewangles[YAW];

	/* touch other objects */
	ClientImpacts( ent, &pm );

	/* execute client events */
	if ( client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}

	if ( client->latched_buttons & BUTTON_ACTIVATE ) {
		Cmd_Activate_f( ent );
	}
}

/*
==================
ClientThink

A new command has arrived from the client.
==================
*/
void ClientThink( int clientNum ) {
	gentity_t   *ent;

	ent = g_entities + clientNum;

	ent->client->sess.oldcmd = ent->client->sess.cmd;
	trap_GetUsercmd( clientNum, &ent->client->sess.cmd );

	ent->client->lastCmdTime = level.time;

	if ( !g_synchronousClients.integer ) {
		ClientThink_real( ent, &ent->client->sess.cmd );
	}
}

/*
==================
G_RunClient
==================
*/
void G_RunClient( gentity_t *ent ) {
	entityLinkInfo_t *linkInfo;
	matrix43_t parentAxis;
	vec3_t origin;

	if ( g_synchronousClients.integer ) {
		ent->client->sess.cmd.serverTime = level.time;
		ClientThink_real( ent, &ent->client->sess.cmd );
	}

	if ( ent->client->noclip ) {
		return;
	}

	linkInfo = (entityLinkInfo_t *)ent->linkInfo;
	if ( linkInfo ) {
		if ( ent->client->sess.sessionState == STATE_DEAD ) {
			ent->client->ps.pm_type = PM_DEAD_LINKED;
		} else {
			ent->client->ps.pm_type = PM_NORMAL_LINKED;
		}

		G_CalcTagParentAxis( ent, &parentAxis );
		MatrixTransformVector43( &parentAxis, origin, linkInfo->relAxis.origin );

		G_SetOrigin( ent, origin );
		G_SetAngle( ent, ent->r.currentAngles );
		ent->s.pos.trType = TR_INTERPOLATE;
		ent->s.apos.trType = TR_INTERPOLATE;

		trap_LinkEntity( ent );

		VectorCopy( ent->r.currentOrigin, ent->client->ps.origin );
	} else if ( ent->client->ps.pm_type == PM_NORMAL_LINKED
				|| ent->client->ps.pm_type == PM_DEAD_LINKED ) {
		ent->client->ps.pm_type--;
	}
}

/*
==================
IntermissionClientEndFrame
==================
*/
void IntermissionClientEndFrame( gentity_t *ent ) {
	gclient_t   *client;

	client = ent->client;

	ent->r.svFlags = ( ent->r.svFlags & ~( SVF_NOCLIENT | SVF_VISIBLE_PLAYER ) ) | SVF_NOCLIENT;
	ent->takedamage = qfalse;
	ent->r.contents = 0;

	client->ps.pm_flags &= ~PMF_INWORLD;
	client->ps.eFlags &= ~( EF_TALK | EF_FIRING );
	client->ps.pm_type = PM_INTERMISSION;
	client->ps.viewmodelIndex = 0;
}

/*
==================
SpectatorClientEndFrame

A spectator does not run a playerState of its own: it replays the archived one
of the client it is following, keeping only its own follow-locked flag.
==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	gclient_t   *client;
	playerState_t archived;
	int savedEFlags;

	ent->takedamage = qfalse;
	ent->r.contents = 0;
	ent->r.svFlags = ( ent->r.svFlags & ~SVF_VISIBLE_PLAYER ) | SVF_NOCLIENT;

	/* retail 0x20018417 loads ent->client once for these four stores, then
	   reloads it at every later access (0x20018437..0x20018574): only this
	   group goes through the local */
	client = ent->client;
	client->ps.pm_flags &= ~PMF_INWORLD;
	client->ps.viewmodelIndex = 0;
	CLIENT_AIMPITCH( client ) = 0.0f;
	CLIENT_AIMYAW( client ) = 0.0f;

	if ( ent->client->sess.forceSpectatorClient >= 0 ) {
		ent->client->spectatorClient = ent->client->sess.forceSpectatorClient;

		while ( 1 ) {
			if ( ent->client->sess.archiveTime < 0 ) {
				ent->client->sess.archiveTime = 0;
			}
			if ( trap_GetArchivedClientInfo( ent->client->sess.forceSpectatorClient,
											 &ent->client->sess.archiveTime, &archived ) ) {
				break;
			}
			if ( !ent->client->sess.archiveTime ) {
				ent->client->sess.forceSpectatorClient = -1;
				ent->client->spectatorClient = -1;
				goto follow;
			}
			ent->client->sess.archiveTime -= 50;
		}
	} else {
follow:
		if ( ent->client->spectatorClient < 0
			 || !trap_GetArchivedClientInfo( ent->client->spectatorClient,
											 &ent->client->sess.archiveTime, &archived ) ) {
			StopFollowing( ent );
			return;
		}
	}

	savedEFlags = archived.eFlags ^ ( ( archived.eFlags ^ ent->client->ps.eFlags ) & PMF_FOLLOWLOCKED );

	ent->client->ps = archived;

	HudElem_UpdateClient( ent->client, ent->s.number, HE_UPDATE_CURRENT );

	ent->client->ps.pm_flags &= ~PMF_INWORLD;
	ent->client->ps.pm_flags |= PMF_NOMOVE;
	if ( ent->client->sess.forceSpectatorClient >= 0 ) {
		ent->client->ps.pm_flags |= PMF_FOLLOWLOCKED;
	} else {
		ent->client->ps.pm_flags &= ~PMF_FOLLOWLOCKED;
	}

	ent->client->ps.eFlags = savedEFlags;
}

/*
==================
GetFollowPlayerState

vmMain export 17.  Hand the engine a copy of one client's playerState for the
snapshot archive, with the hud stripped.
==================
*/
int GetFollowPlayerState( int clientNum, playerState_t *ps ) {
	gclient_t   *client;

	client = g_entities[clientNum].client;

	if ( !( client->ps.pm_flags & PMF_INWORLD ) ) {
		return qfalse;
	}

	*ps = client->ps;
	memset( ps->hud.current, 0, sizeof( ps->hud.current ) );

	return qtrue;
}

/*
==================
StuckInClient

Two live clients wedged inside one another get pushed apart along the line
between them.  Boxes overlap by their bounds; a capsule against a box is
tested by walking the capsule's radius along the separating direction, and two
capsules by the sum of their radii.
==================
*/
qboolean StuckInClient( gentity_t *ent ) {
	int i;
	gentity_t   *other;
	vec3_t normal;
	float adjX, adjY;
	float dx, dy;
	float radius;
	vec3_t dir;
	float angle;
	float speed, otherSpeed;

	if ( !( ent->client->ps.pm_flags & PMF_INWORLD ) ) {
		return qfalse;
	}
	if ( ent->client->sess.sessionState != STATE_PLAYING ) {
		return qfalse;
	}
	if ( ent->r.contents != CONTENTS_BODY && ent->r.contents != CONTENTS_CORPSE ) {
		return qfalse;
	}

	for ( i = 0; i < level.maxclients; i++ ) {
		other = &g_entities[i];

		if ( !other->inuse ) {
			continue;
		}
		/* retail quirk: this one bails out of the whole scan instead of
		   skipping the entity */
		if ( !( other->client->ps.pm_flags & PMF_INWORLD ) ) {
			return qfalse;
		}
		if ( other->client->sess.sessionState != STATE_PLAYING ) {
			continue;
		}
		if ( other == ent ) {
			continue;
		}
		if ( !other->client ) {
			continue;
		}
		if ( other->health <= 0 ) {
			continue;
		}
		if ( other->r.contents != CONTENTS_BODY && other->r.contents != CONTENTS_CORPSE ) {
			continue;
		}

		if ( other->r.mins[0] + other->r.currentOrigin[0]
			 > ent->r.maxs[0] + ent->r.currentOrigin[0] ) {
			continue;
		}
		if ( other->r.maxs[0] + other->r.currentOrigin[0]
			 < ent->r.mins[0] + ent->r.currentOrigin[0] ) {
			continue;
		}
		if ( other->r.mins[1] + other->r.currentOrigin[1]
			 > ent->r.maxs[1] + ent->r.currentOrigin[1] ) {
			continue;
		}
		if ( other->r.maxs[1] + other->r.currentOrigin[1]
			 < ent->r.mins[1] + ent->r.currentOrigin[1] ) {
			continue;
		}
		if ( other->r.mins[2] + other->r.currentOrigin[2]
			 > ent->r.maxs[2] + ent->r.currentOrigin[2] ) {
			continue;
		}
		if ( other->r.maxs[2] + other->r.currentOrigin[2]
			 < ent->r.mins[2] + ent->r.currentOrigin[2] ) {
			continue;
		}

		if ( ent->r.svFlags & SVF_CAPSULE ) {
			dx = other->r.currentOrigin[0] - ent->r.currentOrigin[0];
			dy = other->r.currentOrigin[1] - ent->r.currentOrigin[1];

			if ( other->r.svFlags & SVF_CAPSULE ) {
				radius = other->r.maxs[0] + ent->r.maxs[0];
				if ( dx * dx + dy * dy > radius * radius ) {
					continue;
				}
			} else {
				normal[0] = dx;
				normal[1] = dy;
				VectorNormalize2D( normal );
				adjX = ent->r.maxs[0] * -1.0f * normal[0] + dx;
				adjY = ent->r.maxs[0] * -1.0f * normal[1] + dy;
				if ( fabs( adjX ) > other->r.maxs[0] && fabs( adjY ) > other->r.maxs[1] ) {
					continue;
				}
			}
		} else if ( other->r.svFlags & SVF_CAPSULE ) {
			dx = ent->r.currentOrigin[0] - other->r.currentOrigin[0];
			dy = ent->r.currentOrigin[1] - other->r.currentOrigin[1];

			normal[0] = dx;
			normal[1] = dy;
			VectorNormalize2D( normal );
			adjX = other->r.maxs[0] * -1.0f * normal[0] + dx;
			adjY = other->r.maxs[0] * -1.0f * normal[1] + dy;
			if ( fabs( adjX ) > ent->r.maxs[0] && fabs( adjY ) > ent->r.maxs[1] ) {
				continue;
			}
		}

		dir[0] = other->r.currentOrigin[0] - ent->r.currentOrigin[0];
		dir[1] = other->r.currentOrigin[1] - ent->r.currentOrigin[1];
		if ( fabs( dir[0] ) < 0.01 && fabs( dir[1] ) < 0.01 ) {
			/* exactly on top of each other: pick a direction off the entity
			   number so the two ends disagree */
			angle = (float)ent->s.number * 0.1f;
			dir[1] = (float)cos( angle );
			dir[0] = (float)sin( angle );
		}
		VectorNormalize2D( dir );

		if ( (float)sqrt( other->client->ps.velocity[0] * other->client->ps.velocity[0]
						  + other->client->ps.velocity[1] * other->client->ps.velocity[1]
						  + other->client->ps.velocity[2] * other->client->ps.velocity[2] ) > 0.0f ) {
			otherSpeed = (float)other->client->ps.speed;
		} else {
			otherSpeed = 0.0f;
		}
		if ( (float)sqrt( ent->client->ps.velocity[0] * ent->client->ps.velocity[0]
						  + ent->client->ps.velocity[1] * ent->client->ps.velocity[1]
						  + ent->client->ps.velocity[2] * ent->client->ps.velocity[2] ) > 0.0f ) {
			speed = (float)ent->client->ps.speed;
		} else {
			speed = 0.0f;
		}
		if ( otherSpeed < 0.0001f && speed < 0.0001f ) {
			otherSpeed = (float)other->client->ps.speed;
			speed = (float)ent->client->ps.speed;
		}

		/* retail quirk: dir[2] is never assigned, so the vertical component of
		   both pushes is whatever was on the stack */
		VectorScale( dir, otherSpeed, other->client->ps.velocity );
		other->client->ps.pm_time = 300;
		other->client->ps.pm_flags |= PMF_TIME_LAND;

		VectorScale( dir, -speed, ent->client->ps.velocity );
		ent->client->ps.pm_time = 300;
		ent->client->ps.pm_flags |= PMF_TIME_LAND;

		return qtrue;
	}

	return qfalse;
}

/*
==============
G_PlayerController

gentity_t.controller for a player; G_DObjCalcPose calls it once the bone list
is known.
==============
*/
void G_PlayerController( gentity_t *self, unsigned int *partBits ) {
	BG_Player_DoControllers( &bg_clientinfo[self->s.clientNum], self, &self->s, (int *)partBits );
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client.  A fast
client will have multiple ClientThink for each ClientEndFrame, while a slow
client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	gclient_t   *client;
	clientInfo_t *ci;
	const char  *name;
	const char  *tagName;
	qboolean dobjChanged;
	int i;
	int info;
	vec3_t origin;
	vec3_t angles;

	client = ent->client;

	ent->controller = NULL;
	client->ps.deltaTime = 0;

	if ( client->sess.connected != CON_CONNECTED ) {
		return;
	}

	if ( client->sess.sessionState == STATE_INTERMISSION ) {
		IntermissionClientEndFrame( ent );
	} else if ( client->sess.sessionState == STATE_SPECTATOR ) {
		SpectatorClientEndFrame( ent );
	} else if ( client->ps.clientNum == ent->s.number ) {
		ent->takedamage = qtrue;
		ent->r.svFlags = ( ent->r.svFlags & ~SVF_NOCLIENT ) | SVF_VISIBLE_PLAYER;
		client->ps.pm_flags |= PMF_INWORLD;
		client->ps.viewmodelIndex = client->sess.viewmodel;

		G_SetClientContents( ent );

		CLIENT_ENDFRAMERESET( client ) = 0;
		if ( client->pingTime <= level.time ) {
			client->ps.eFlags &= ~EF_COMPASS_MARK;
		}

		if ( client->noclip ) {
			client->ps.pm_type = PM_NOCLIP;
		} else if ( client->ufo ) {
			client->ps.pm_type = PM_UFO;
		} else if ( client->sess.sessionState == STATE_DEAD ) {
			client->ps.pm_type = ent->linkInfo ? PM_DEAD_LINKED : PM_DEAD;
			ent->r.svFlags = ( ent->r.svFlags & ~( SVF_NOCLIENT | SVF_VISIBLE_PLAYER ) )
							 | SVF_NOCLIENT;
			ent->takedamage = qfalse;
		} else {
			client->ps.pm_type = ent->linkInfo ? PM_NORMAL_LINKED : PM_NORMAL;
		}

		client->ps.gravity = (int)g_gravity.value;
		client->ps.speed = (int)g_speed.value;
		CLIENT_SPREADFRAC( client ) = client->ps.aimSpreadScale * ( 1.0 / 255.0 );

		G_CheckForPreventFriendlyFire( ent );
		G_CheckForCursorHints( ent );

		/* burn from lava, etc */
		P_DamageFeedback( ent );

		/* if the end of unit layout is displayed, don't give the player any
		   normal movement attributes */
		if ( level.time - client->lastCmdTime > 1000 ) {
			ent->s.eFlags |= EF_CONNECTION;
		} else {
			ent->s.eFlags &= ~EF_CONNECTION;
		}

		client->ps.stats[STAT_HEALTH] = ent->health;

		G_SetClientSound( ent );

		/* set the latest infor */
		if ( g_smoothClients.integer ) {
			BG_PlayerStateToEntityStateExtrapolate( &client->ps, &ent->s,
													client->ps.commandTime, qtrue );
		} else {
			BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		}

		if ( ent->health > 0 && StuckInClient( ent ) ) {
			ent->r.contents = CONTENTS_CORPSE;
		}

		VectorCopy( client->ps.origin, origin );
		origin[2] += client->ps.viewHeightCurrent;
		G_AddLean( ent, origin );

		info = G_GetNonPVSFriendlyInfo( ent, origin, CLIENT_COMPASSSCANENT( client ) );
		client->ps.iCompassFriendInfo = info;
		if ( info ) {
			CLIENT_COMPASSSCANENT( client ) = info & 0x3F;
			if ( g_entities[info & 0x3F].s.eFlags & EF_COMPASS_MARK ) {
				client->ps.eFlags |= EF_COMPASS_MARK_FRIEND;
			} else {
				client->ps.eFlags &= ~EF_COMPASS_MARK_FRIEND;
			}
		} else {
			CLIENT_COMPASSSCANENT( client ) = ENTITYNUM_NONE;
		}

		if ( ent->s.eType == ET_PLAYER ) {
			ci = &bg_clientinfo[ent->s.clientNum];

			ent->controller = G_PlayerController;

			ci->leanAmount = ent->s.angles2[YAW];
			ci->leanFraction = ent->s.leanf;
			ci->viewPitch = client->ps.viewangles[PITCH];
			ci->viewYaw = client->ps.viewangles[YAW];
			ci->viewRoll = client->ps.viewangles[ROLL];

			dobjChanged = qfalse;

			name = G_ModelName( ent->model );
			client->sess.modelIndex = ent->model;
			if ( strcmp( ci->modelName, name ) ) {
				dobjChanged = qtrue;
				Q_strncpyz( ci->modelName, name, sizeof( ci->modelName ) );
			}

			for ( i = 0; i < MAX_ATTACHED_MODELS; i++ ) {
				if ( ent->attachModelIndex[i] ) {
					name = trap_GetConfigstringConst( CS_MODELS + ent->attachModelIndex[i] );
					client->sess.attachModelIndex[i] = ent->attachModelIndex[i];
					if ( strcmp( ci->attachModelNames[i], name ) ) {
						dobjChanged = qtrue;
						Q_strncpyz( ci->attachModelNames[i], name,
									sizeof( ci->attachModelNames[i] ) );
					}

					tagName = SL_ConvertToString( ent->attachTagName[i] );
					client->sess.attachTagIndex[i] = G_TagIndex( tagName );
					if ( strcmp( ci->attachTagNames[i], tagName ) ) {
						dobjChanged = qtrue;
						Q_strncpyz( ci->attachTagNames[i], tagName,
									sizeof( ci->attachTagNames[i] ) );
					}
				} else {
					ci->attachModelNames[i][0] = 0;
					ci->attachTagNames[i][0] = 0;
					client->sess.attachModelIndex[i] = 0;
					client->sess.attachTagIndex[i] = 0;
				}
			}

			if ( dobjChanged ) {
				trap_SafeDObjFree( ent->s.number, qfalse );
			}

			BG_UpdatePlayerDObj( ent, &ent->s, ci );
			BG_PlayerAnimation( ci, ent, &ent->s );

			if ( ( client->ps.pm_flags & PMF_INWORLD )
				 && ( client->ps.eFlags & EF_TURRET_ACTIVE_MASK ) ) {
				turret_think_client( &level.gentities[client->ps.viewlocked_entNum] );
			}

			if ( g_debugLocDamage.integer ) {
				if ( trap_DObjExists( ent ) ) {
					G_DObjCalcPose( ent );
					trap_XModelDebugBoxes( ent );
				}
			}
		}
	} else {
		/* the playerState still belongs to whoever this client was following:
		   put him back in the world */
		VectorCopy( client->ps.origin, origin );
		angles[PITCH] = 0.0f;
		angles[YAW] = client->ps.viewangles[YAW];
		angles[ROLL] = 0.0f;
		ClientSpawn( ent, origin, angles );
	}
}
