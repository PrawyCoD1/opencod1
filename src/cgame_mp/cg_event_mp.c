/*
 * cg_event.c -- entity events and effects.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_event.c is the ancestor of CG_Obituary,
 * CG_ItemPickup, CG_EntityEvent and CG_CheckEvents, but only the skeletons
 * survived:
 *
 *   - the event enum is CoD's own 202-value list, recovered verbatim as
 *     eventnames[] in game_mp/bg_misc.c.  Six 23-wide footstep blocks (run,
 *     walk, prone, jump, landing, landing-pain) replace RTCW's EV_FOOTSTEP_*,
 *     and each one indexes a 23-entry surface-sound table in cgs.media.
 *   - every sound is a sound ALIAS name, not a handle: cgs.media's sound slots
 *     hold the string trap_Com_SoundAliasString returned, and cg_public.h types
 *     them sfxHandle_t, hence the casts.
 *   - explosions and impacts are FX-system effect ids (traps 227..229), not
 *     local entities.
 *   - CoD added CG_EntityPreEvent / CG_CheckPreEvents: a second event stream
 *     carried in centity_t::nextState that fires one frame early, so muzzle
 *     flashes and bullet impacts are not a frame late.
 *
 * Functions are in address order, which is the original file order.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdlib.h>

#include "cg_local.h"
#include "../game_mp/bg_public.h"

#define MAX_EVENTS                  4           /* entityState_t::events[] */
#define SURF_TYPE_NUM               23
#define STEP_TIME                   100

/*
 * The events this unit names.  Indices into eventnames[] (game_mp/bg_misc.c),
 * which is the authority on both the ordering and the spelling.
 */
#define EV_NONE                         0
#define EV_FOOTSTEP_RUN_DEFAULT         1
#define EV_FOOTSTEP_RUN_ASPHALT         23
#define EV_FOOTSTEP_WALK_DEFAULT        24
#define EV_FOOTSTEP_WALK_ASPHALT        46
#define EV_FOOTSTEP_PRONE_DEFAULT       47
#define EV_FOOTSTEP_PRONE_ASPHALT       69
#define EV_JUMP_DEFAULT                 70
#define EV_JUMP_ASPHALT                 92
#define EV_LANDING_DEFAULT              93
#define EV_LANDING_ASPHALT              115
#define EV_LANDING_PAIN_DEFAULT         116
#define EV_LANDING_PAIN_ASPHALT         138
#define EV_FOLIAGE_SOUND                139
#define EV_STANCE_FORCE_STAND           140
#define EV_STANCE_FORCE_CROUCH          141
#define EV_STANCE_FORCE_PRONE           142
#define EV_STEP_VIEW                    143
#define EV_WATER_TOUCH                  144
#define EV_WATER_LEAVE                  145
#define EV_ITEM_PICKUP                  146
#define EV_ITEM_PICKUP_QUIET            147
#define EV_AMMO_PICKUP                  148
#define EV_NOAMMO                       149
#define EV_EMPTYCLIP                    150
#define EV_RELOAD                       151
#define EV_RELOAD_FROM_EMPTY            152
#define EV_RELOAD_START                 153
#define EV_RELOAD_END                   154
#define EV_RAISE_WEAPON                 155
#define EV_PUTAWAY_WEAPON               156
#define EV_WEAPON_ALT                   157
#define EV_PULLBACK_WEAPON              158
#define EV_FIRE_WEAPON                  159
#define EV_FIRE_WEAPONB                 160
#define EV_FIRE_WEAPON_LASTSHOT         161
#define EV_RECHAMBER_WEAPON             162
#define EV_EJECT_BRASS                  163
#define EV_MELEE_SWIPE                  164
#define EV_FIRE_MELEE                   165
#define EV_MELEE_HIT                    166
#define EV_MELEE_MISS                   167
#define EV_FIRE_WEAPON_MG42             168
#define EV_FIRE_QUADBARREL_1            169
#define EV_FIRE_QUADBARREL_2            170
#define EV_BULLET_TRACER                171
#define EV_SOUND_ALIAS                  172
#define EV_BULLET_HIT_SMALL             173
#define EV_BULLET_HIT_LARGE             174
#define EV_BULLET_HIT_CLIENT_SMALL      175
#define EV_BULLET_HIT_CLIENT_LARGE      176
#define EV_GRENADE_BOUNCE               177
#define EV_GRENADE_EXPLODE              178
#define EV_ROCKET_EXPLODE               179
#define EV_ROCKET_EXPLODE_NOMARKS       180
#define EV_RAILTRAIL                    185
#define EV_PAIN                         187
#define EV_CROUCH_PAIN                  188
#define EV_DEATH                        189
#define EV_DEBUG_LINE                   190
#define EV_PLAY_FX                      191
#define EV_PLAY_FX_DIR                  192
#define EV_PLAY_FX_ON_TAG               193
#define EV_FLAMEBARREL_BOUNCE           194
#define EV_EARTHQUAKE                   195
#define EV_DROPWEAPON                   196
#define EV_ITEM_RESPAWN                 197
#define EV_ITEM_POP                     198
#define EV_PLAYER_TELEPORT_IN           199
#define EV_PLAYER_TELEPORT_OUT          200
#define EV_OBITUARY                     201

/* entityState_t::eType > ET_EVENTS is the event-only entity (0x3001EA10). */
#define ET_EVENTS                       12

/* configstrings.  524 is cg_ent_mp.c's CS_SOUNDALIAS; 844 is the block
   CG_PlayFxOnTag reads (0x3001DB86), one past CS_EFFECTS' 780..843. */
#define CS_SOUNDALIAS                   524
#define CS_EFFECTS_ONTAG                844
#define MAX_CONFIGSTRINGS               2048

/* CG_PlayFx' bounds test against cgs.gameEffects (0x3001DB2A). */
#define MAX_FX_CG                       64

/* CG_EntityEvent's world-sound entity number (0x3001E4AE); Q3's
   ENTITYNUM_WORLD, MAX_GENTITIES - 2. */
#define ENTITYNUM_WORLD                 ( MAX_GENTITIES - 2 )

/* cg_ent_mp.c spells this same 0x50000 pm_flags mask PMF_VIEWLOCKED
   (0x3001BF47); CG_EntityEvent tests it at 0x3001E0E2. */
#define PMF_VIEWLOCKED                  0x00050000

/* CG_Obituary's means-of-death byte carries this flag when the low bits are a
   meansOfDeath_t rather than a weapon index (0x3001D6F8). */
#define MOD_FLAG                        0x80
#define MOD_MELEE                       0x07
#define MOD_HEAD_SHOT                   0x08
#define MOD_WATER                       0x10
#define MOD_SLIME                       0x11
#define MOD_CRUSH                       0x13
#define MOD_FALLING                     0x15
#define MOD_SUICIDE                     0x16
#define MOD_TRIGGER_HURT                0x17

/*
 * cgs.media's ten 23-entry surface-sound tables live in the 920-byte hole
 * cg_local.h spells unknown_0x2F8, and the nine impact-effect tables in the
 * 832-byte unknown_0x70C (the same nine CG_RegisterImpactEffects fills).
 */
#define MEDIA_SURFACE_SOUNDS( k )   ( (const char **)&cgs.media.unknown_0x2F8[ (k) * SURF_TYPE_NUM * 4 ] )
#define MEDIA_IMPACT_EFFECTS( k )   ( (const int *)&cgs.media.unknown_0x70C[ (k) * SURF_TYPE_NUM * 4 ] )

#define footstepRunSounds       MEDIA_SURFACE_SOUNDS( 6 )   /* cgs+0x9F04; the jump block reads it too */
#define footstepWalkSounds      MEDIA_SURFACE_SOUNDS( 7 )   /* cgs+0x9F60 */
#define footstepProneSounds     MEDIA_SURFACE_SOUNDS( 8 )   /* cgs+0x9FBC */
#define landingSounds           MEDIA_SURFACE_SOUNDS( 9 )   /* cgs+0xA018; landing-pain reads it too */
#define grenadeBounceSounds     MEDIA_SURFACE_SOUNDS( 0 )   /* cgs+0x9CDC */
#define grenadeExplodeSounds    MEDIA_SURFACE_SOUNDS( 1 )   /* cgs+0x9D38 */
#define rocketExplodeSounds     MEDIA_SURFACE_SOUNDS( 2 )   /* cgs+0x9D94 */

#define grenadeBounceEffects    MEDIA_IMPACT_EFFECTS( 4 )   /* cgs+0xA260 */
#define grenadeExplodeEffects   MEDIA_IMPACT_EFFECTS( 5 )   /* cgs+0xA2BC */
#define rocketExplodeEffects    MEDIA_IMPACT_EFFECTS( 6 )   /* cgs+0xA318 */

/* centity_t::miscTime, +0x1F4 inside cg_local.h's unknown_0x1F0 hole. */
#define CENT_MISCTIME( c )  ( *(int *)&( c )->unknown_0x1F0[4] )

/* cg.markSuppress: CG_EntityEvent raises it around EV_ROCKET_EXPLODE_NOMARKS
   (0x3001E59F) and CG_ImpactMark returns early while it is set. */
#define cg_noMarks          ( *(int *)&cg.unknown_0x2A87C[4] )

/*
 * cg_items[] -- cgame's per-item visual record, 36 bytes, indexed by item
 * number (CG_RegisterItemVisuals 0x30036080, base 0x301DC3E0).  Only the two
 * pickup sound aliases this unit reads are named; cg_weapons.c owns the type.
 */
typedef struct cgItemInfo_t {
	byte        unknown_0x00[0x1C];
	const char  *pickupSound;               /* +0x1C EV_ITEM_PICKUP  0x3001E0BD */
	const char  *ammoPickupSound;           /* +0x20 EV_AMMO_PICKUP  0x3001E0CE */
} cgItemInfo_t;

extern cgItemInfo_t cg_items[];                                 /* 0x301DC3E0 */

/* Owned by other units. */
void        CG_Printf( const char *msg, ... );                  /* cg_main.c 0x300206F0 */
void        CG_Error( const char *msg, ... );                   /* cg_main.c 0x30020750 */
void        Com_DPrintf( const char *msg, ... );                /* cg_main.c 0x300208F0 */
int         CG_PlaySoundAliasByName( const char *name, int entnum, const vec3_t origin );
                                                                /* cg_main.c 0x30021BF0 */
void        CG_PriorityCenterPrint( const char *str, float y, float charWidth, int priority );
                                                                /* cg_draw.c 0x300159E0 */
void        CG_DrawScoreboard_GetTeamColor( vec4_t color, int team );
                                                                /* cg_scoreboard.c 0x3002A810 */
/* CG_FireWeapon, CG_EjectWeaponBrass, CG_RailTrail, CG_BulletHitEvent,
   CG_BulletHitClientEvent and CG_OutOfAmmoChange all come in from cg_local.h's
   cg_weapons_mp.c section. */
void        CG_StartShakeCamera( float amplitude, int duration, float radius, const vec3_t origin );
                                                                /* cg_shellshock.c 0x30017DD0 */

/* game_mp/bg_misc.c -- the 202-value event-name table, recovered verbatim. */
extern char *eventnames[];

/*
=================
CG_Obituary     0x3001D6C0
=================
*/
static void CG_Obituary( entityState_t *ent ) {
	int         mod;
	int         target, attacker;
	const char  *icon;
	float       iconWidth;
	float       iconHeight;
	vec4_t      attackerColor;
	vec4_t      targetColor;
	vec4_t      iconColor;
	char        attackerName[36];
	char        targetName[36];
	clientInfo_t *targetInfo;
	clientInfo_t *attackerInfo;
	const char  *s;

	target = ent->otherEntityNum;
	attacker = ent->attackerEntityNum;
	mod = ent->eventParm;

	attackerColor[0] = attackerColor[1] = attackerColor[2] = attackerColor[3] = 1.0f;
	targetColor[0] = targetColor[1] = targetColor[2] = targetColor[3] = 1.0f;
	iconColor[0] = iconColor[1] = iconColor[2] = iconColor[3] = 1.0f;

	icon = "killIconDied";
	iconWidth = 1.4f;
	iconHeight = 1.4f;

	if ( mod & MOD_FLAG ) {
		switch ( mod & ~MOD_FLAG ) {
		case MOD_MELEE:         icon = "killIconMelee";     break;
		case MOD_HEAD_SHOT:     icon = "killIconHeadShot";  break;
		case MOD_WATER:         icon = "killIconDrown";     break;
		case MOD_SLIME:         icon = "killIconSlime";     break;
		case MOD_CRUSH:         icon = "killIconCrush";     break;
		case MOD_FALLING:       icon = "killIconFalling";   break;
		case MOD_SUICIDE:       icon = "killIconSuicide";   break;
		case MOD_TRIGGER_HURT:  icon = "killIconDied";      break;
		default:                                            break;
		}
	} else {
		if ( bg_weaponInfo[mod]->killIcon[0] ) {
			icon = bg_weaponInfo[mod]->killIcon;
			if ( bg_weaponInfo[mod]->wideKillIcon ) {
				iconWidth = 2.8f;
			}
		}
	}

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}

	targetInfo = &bg_clientinfo[ target ];
	if ( !targetInfo->infoValid ) {
		return;
	}

	strncpy( targetName, targetInfo->name, 31 );
	targetName[31] = 0;
	strcpy( targetName + strlen( targetName ), "^7" );
	CG_DrawScoreboard_GetTeamColor( targetColor, targetInfo->team );

	if ( !bg_clientinfo[ cg.clientNum ].infoValid ) {
		return;
	}

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
		attackerName[0] = 0;
	} else {
		attackerInfo = &bg_clientinfo[ attacker ];
		if ( !attackerInfo->infoValid ) {
			return;
		}
		strncpy( attackerName, attackerInfo->name, 31 );
		attackerName[31] = 0;
		strcpy( attackerName + strlen( attackerName ), "^7" );
		CG_DrawScoreboard_GetTeamColor( attackerColor, attackerInfo->team );

		if ( target == cg.snap->ps.clientNum ) {
			strncpy( cg.killerName, attackerName, 31 );
			cg.killerName[31] = 0;
		}
	}

	if ( attacker == target ) {
		attackerName[0] = 0;
	} else if ( attacker == cg.snap->ps.clientNum ) {
		if ( attackerInfo->team && targetInfo->team == attackerInfo->team ) {
			s = va( "CGAME_YOUKILLED\x15^1%%s^7 %s\x14%s", targetName, "CGAME_TEAMMATE" );
		} else {
			s = va( "CGAME_YOUKILLED\x15%s", targetName );
		}
		CG_PriorityCenterPrint( s, 400.0f, 9.6f, 1 );
	}

	CG_DreathMessage( attackerName, attackerColor, targetName, targetColor,
					  icon, iconWidth, iconHeight, iconColor );
}

/*
===============
CG_ItemPickup     0x3001DA80

A new item was picked up this frame
===============
*/
static void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;

	// see if it should be the grabbed weapon
	if ( bg_itemlist[itemNum].giType != IT_WEAPON ) {
		return;
	}
	if ( cg_weaponSelect.integer ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	trap_Cvar_Set( "cg_weaponSelect", va( "%i", bg_itemlist[itemNum].giTag ) );
}

/*
===============
CG_EquipmentSound     0x3001DAE0
===============
*/
static void CG_EquipmentSound( int entnum, qboolean running ) {
	if ( running ) {
		CG_PlayEntitySoundAliasByName( entnum, (const char *)cgs.media.gearRattleRun );
	} else {
		CG_PlayEntitySoundAliasByName( entnum, (const char *)cgs.media.gearRattleWalk );
	}
}

/*
===============
CG_PlayFx     0x3001DB20
===============
*/
static void CG_PlayFx( centity_t *cent, const vec3_t dir ) {
	if ( cent->currentState.eventParm <= 0 || cent->currentState.eventParm >= MAX_FX_CG ) {
		CG_Printf( "ERROR: CG_PlayFx called with invalid effect id %i\n",
				   cent->currentState.eventParm );
		return;
	}

	if ( dir ) {
		trap_syscall_0xE4( cgs.gameEffects[ cent->currentState.eventParm ],
						   (int)cent->lerpOrigin, (int)dir );
	} else {
		trap_syscall_0xE3( cgs.gameEffects[ cent->currentState.eventParm ],
						   (int)cent->lerpOrigin );
	}
}

/*
===============
CG_PlayFxOnTag     0x3001DB80

The configstring is "%02i" of the effect index followed by the tag name.
===============
*/
static void CG_PlayFxOnTag( centity_t *cent, int eventParm ) {
	const char  *cs;
	int         effect;
	int         boneIndex;
	int         bolt[2];

	cs = CG_ConfigString( CS_EFFECTS_ONTAG + eventParm );
	effect = cgs.gameEffects[ ( cs[0] - '0' ) * 10 + ( cs[1] - '0' ) ];

	boneIndex = trap_syscall_0xDF( cent->currentState.number, cs + 2 );
	if ( boneIndex < 0 ) {
		return;
	}

	bolt[0] = cent->currentState.number;
	bolt[1] = boneIndex;
	trap_syscall_0xE5( effect, (int)cent->lerpOrigin, 0, (int)bolt );
}

/*
==============
CG_EntityEvent     0x3001DC10

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
void CG_EntityEvent( centity_t *cent, int event, qboolean predicted ) {
	entityState_t   *es;
	int             clientNum;
	int             eventParm;
	int             item;
	int             kick;
	float           height;
	vec3_t          dir;

	if ( !event ) {
		if ( cg_debugevents.integer ) {
			CG_Printf( "CG_EntityEvent:ZERO EVENT\n" );
		}
		return;
	}

	es = &cent->currentState;
	eventParm = es->eventParm;

	if ( cg_debugevents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
		if ( cg_debugevents.integer ) {
			CG_Printf( "CG_EntityEvent:%s\n", eventnames[event] );
		}
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}

	if ( event >= EV_FOOTSTEP_RUN_DEFAULT && event < EV_FOOTSTEP_WALK_DEFAULT ) {
		if ( cg_footsteps.integer ) {
			CG_PlayEntitySoundAliasByName( es->number,
										   footstepRunSounds[ event - EV_FOOTSTEP_RUN_DEFAULT ] );
		}
		CG_EquipmentSound( es->number, qtrue );
		return;
	}

	if ( event >= EV_FOOTSTEP_WALK_DEFAULT && event < EV_FOOTSTEP_PRONE_DEFAULT ) {
		if ( cg_footsteps.integer ) {
			CG_PlayEntitySoundAliasByName( es->number,
										   footstepWalkSounds[ event - EV_FOOTSTEP_WALK_DEFAULT ] );
		}
		CG_EquipmentSound( es->number, qfalse );
		return;
	}

	if ( event >= EV_FOOTSTEP_PRONE_DEFAULT && event < EV_JUMP_DEFAULT ) {
		if ( cg_footsteps.integer ) {
			CG_PlayEntitySoundAliasByName( es->number,
										   footstepProneSounds[ event - EV_FOOTSTEP_PRONE_DEFAULT ] );
		}
		CG_EquipmentSound( es->number, qfalse );
		return;
	}

	if ( event >= EV_JUMP_DEFAULT && event < EV_LANDING_DEFAULT ) {
		CG_PlayEntitySoundAliasByName( es->number,
									   footstepRunSounds[ event - EV_JUMP_DEFAULT ] );
		CG_EquipmentSound( es->number, qtrue );
		return;
	}

	if ( event >= EV_LANDING_DEFAULT && event < EV_LANDING_PAIN_DEFAULT ) {
		CG_PlayEntitySoundAliasByName( es->number,
									   landingSounds[ event - EV_LANDING_DEFAULT ] );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landTime = cg.time;
			cg.landChange = (float)-eventParm;
		}
		return;
	}

	if ( event >= EV_LANDING_PAIN_DEFAULT && event < EV_FOLIAGE_SOUND ) {
		CG_PlayEntitySoundAliasByName( es->number,
									   landingSounds[ event - EV_LANDING_PAIN_DEFAULT ] );
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.landDamageSound );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			height = ( bg_fallDamageMaxHeight.value - bg_fallDamageMinHeight.value )
					 * ( eventParm * 0.01f ) + bg_fallDamageMinHeight.value;
			if ( height > 12.0f ) {
				kick = (int)( ( ( height - 12.0f ) * ( 1.0f / 26.0f ) + 1.0f ) * 4.0f );
				if ( kick > 24 ) {
					kick = 24;
				} else if ( kick <= 0 ) {
					return;
				}
				cg.landTime = cg.time;
				cg.landChange = (float)-kick;
			}
		}
		return;
	}

	switch ( event ) {
	case EV_FOLIAGE_SOUND:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.movementFoliage );
		break;

	case EV_STANCE_FORCE_STAND:
		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			Com_DPrintf( "Event %s just for client %i was sent to other clients\n",
						 eventnames[event], clientNum );
			break;
		}
		if ( !cl_stanceTemp.integer ) {
			trap_Cvar_Setvar( &cl_stance, "0" );
		}
		break;

	case EV_STANCE_FORCE_CROUCH:
		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			Com_DPrintf( "Event %s just for client %i was sent to other clients\n",
						 eventnames[event], clientNum );
			break;
		}
		if ( !cl_stanceTemp.integer ) {
			trap_Cvar_Setvar( &cl_stance, "1" );
		}
		break;

	case EV_STANCE_FORCE_PRONE:
		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			Com_DPrintf( "Event %s just for client %i was sent to other clients\n",
						 eventnames[event], clientNum );
			break;
		}
		if ( !cl_stanceTemp.integer ) {
			trap_Cvar_Setvar( &cl_stance, "2" );
		}
		break;

	case EV_STEP_VIEW:
		{
			int     delta;
			float   oldStep;
			float   step;

			if ( clientNum != cg.predictedPlayerState.clientNum ) {
				Com_DPrintf( "Event %s just for client %i was sent to other clients\n",
							 eventnames[event], clientNum );
				break;
			}
			if ( cg.demoPlayback || cg_nopredict.integer || g_synchronousClients.integer ) {
				break;
			}

			// smooth out stair climbing
			delta = cg.time - cg.stepTime;
			if ( delta < STEP_TIME ) {
				oldStep = ( cg.stepChange * ( STEP_TIME - delta ) / STEP_TIME ) * 0.9f;
			} else {
				oldStep = 0.0f;
			}

			step = (float)( eventParm - 128 );
			cg.stepChange = step + oldStep;
			if ( cg.stepChange > 24.0f ) {
				cg.stepChange = 24.0f;
			} else if ( cg.stepChange < -16.0f ) {
				cg.stepChange = -16.0f;
			}
			cg.stepTime = cg.time;
		}
		break;

	case EV_WATER_TOUCH:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.playerWaterIn );
		break;

	case EV_WATER_LEAVE:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.playerWaterOut );
		break;

	case EV_ITEM_PICKUP:
	case EV_ITEM_PICKUP_QUIET:
	case EV_AMMO_PICKUP:
		item = es->eventParm;
		if ( item < 1 || item >= BG_NUM_ITEMS ) {
			break;
		}
		if ( event == EV_ITEM_PICKUP ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_items[item].pickupSound );
		} else if ( event == EV_AMMO_PICKUP ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_items[item].ammoPickupSound );
		}
		// powerups and team items will have a separate global sound, this one
		// is for items and weapons
		if ( ( cg.snap->ps.pm_flags & PMF_VIEWLOCKED ) && es->number == cg.snap->ps.clientNum ) {
			CG_ItemPickup( item );
		}
		break;

	case EV_NOAMMO:
		if ( !bg_weaponInfo[ es->weapon ]->clipOnly ) {
			CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.noAmmoSound );
		}
		if ( ( cg.snap->ps.pm_flags & PMF_VIEWLOCKED ) && es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;

	case EV_DROPWEAPON:
		if ( cg_weapons[ es->weapon ].putawaySound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].putawaySound );
		}
		if ( ( cg.snap->ps.pm_flags & PMF_VIEWLOCKED ) && es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;

	case EV_EMPTYCLIP:
		break;

	case EV_RELOAD:
		if ( cg_weapons[ es->weapon ].reloadSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].reloadSound );
		} else if ( cg_weapons[ es->weapon ].reloadEmptySound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].reloadEmptySound );
		}
		break;

	case EV_RELOAD_FROM_EMPTY:
		if ( cg_weapons[ es->weapon ].reloadEmptySound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].reloadEmptySound );
		} else if ( cg_weapons[ es->weapon ].reloadSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].reloadSound );
		}
		break;

	case EV_RELOAD_START:
		if ( cg_weapons[ es->weapon ].reloadStartSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].reloadStartSound );
		}
		break;

	case EV_RELOAD_END:
		if ( cg_weapons[ es->weapon ].reloadEndSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].reloadEndSound );
		}
		break;

	case EV_RAISE_WEAPON:
		if ( cg_weapons[ es->weapon ].raiseSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].raiseSound );
		}
		break;

	case EV_PUTAWAY_WEAPON:
		if ( cg_weapons[ es->weapon ].putawaySound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].putawaySound );
		}
		break;

	case EV_WEAPON_ALT:
		if ( cg_weapons[ es->weapon ].altSwitchSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].altSwitchSound );
		}
		break;

	case EV_PULLBACK_WEAPON:
		if ( cg_weapons[ es->weapon ].pullbackSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].pullbackSound );
		}
		break;

	case EV_FIRE_WEAPON:
	case EV_FIRE_WEAPONB:
	case EV_FIRE_WEAPON_LASTSHOT:
	case EV_FIRE_WEAPON_MG42:
		if ( predicted ) {
			CG_FireWeapon( cent, es, event, 0 );
		}
		break;

	case EV_FIRE_QUADBARREL_1:
		if ( predicted ) {
			CG_FireWeapon( cent, es, event, 0 );
			CG_FireWeapon( cent, es, event, 1 );
		}
		break;

	case EV_FIRE_QUADBARREL_2:
		if ( predicted ) {
			CG_FireWeapon( cent, es, event, 2 );
			CG_FireWeapon( cent, es, event, 3 );
		}
		break;

	case EV_RECHAMBER_WEAPON:
		if ( predicted && cg_weapons[ es->weapon ].rechamberSound ) {
			CG_PlayEntitySoundAliasByName( es->number, cg_weapons[ es->weapon ].rechamberSound );
		}
		break;

	case EV_EJECT_BRASS:
		if ( predicted ) {
			CG_EjectWeaponBrass( es, event );
		}
		break;

	case EV_MELEE_SWIPE:
		if ( bg_weaponInfo[ es->weapon ]->rifleBullet ) {
			CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.meleeSwingLarge );
		} else {
			CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.meleeSwingSmall );
		}
		break;

	case EV_FIRE_MELEE:
	case EV_MELEE_MISS:
		break;

	case EV_MELEE_HIT:
		ByteToDir( es->eventParm, dir );
		CG_PlayEntitySoundAliasByName( es->otherEntityNum, (const char *)cgs.media.meleeHit );
		break;

	case EV_SOUND_ALIAS:
		CG_PlaySoundAliasByName( CG_ConfigString( CS_SOUNDALIAS + es->eventParm ),
								 es->number, es->pos.trBase );
		break;

	case EV_BULLET_HIT_SMALL:
	case EV_BULLET_HIT_LARGE:
	case EV_BULLET_HIT_CLIENT_SMALL:
	case EV_BULLET_HIT_CLIENT_LARGE:
		break;

	case EV_GRENADE_BOUNCE:
		ByteToDir( es->eventParm, dir );
		CG_PlaySoundAliasByName( grenadeBounceSounds[ es->surfType ],
								 ENTITYNUM_WORLD, cent->lerpOrigin );
		if ( grenadeBounceEffects[ es->surfType ] ) {
			trap_syscall_0xE4( grenadeBounceEffects[ es->surfType ],
							   (int)cent->lerpOrigin, (int)dir );
		}
		break;

	case EV_GRENADE_EXPLODE:
		ByteToDir( es->eventParm, dir );
		CG_PlaySoundAliasByName( grenadeExplodeSounds[ es->surfType ],
								 ENTITYNUM_WORLD, cent->lerpOrigin );
		if ( grenadeExplodeEffects[ es->surfType ] ) {
			trap_syscall_0xE4( grenadeExplodeEffects[ es->surfType ],
							   (int)cent->lerpOrigin, (int)dir );
		}
		if ( cg_weapons[ es->weapon ].projExplosionEffect ) {
			trap_syscall_0xE4( cg_weapons[ es->weapon ].projExplosionEffect,
							   (int)cent->lerpOrigin, (int)dir );
		}
		if ( cg_weapons[ es->weapon ].projExplosionSound
			 && cg_weapons[ es->weapon ].projExplosionSound[0] ) {
			CG_PlaySoundAliasByName( cg_weapons[ es->weapon ].projExplosionSound,
									 ENTITYNUM_WORLD, cent->lerpOrigin );
		}
		break;

	case EV_ROCKET_EXPLODE_NOMARKS:
		cg_noMarks = 1;
		/* fall through */
	case EV_ROCKET_EXPLODE:
		ByteToDir( es->eventParm, dir );
		CG_PlaySoundAliasByName( rocketExplodeSounds[ es->surfType ],
								 ENTITYNUM_WORLD, cent->lerpOrigin );
		if ( rocketExplodeEffects[ es->surfType ] ) {
			trap_syscall_0xE4( rocketExplodeEffects[ es->surfType ],
							   (int)cent->lerpOrigin, (int)dir );
		}
		if ( cg_weapons[ es->weapon ].projExplosionEffect ) {
			trap_syscall_0xE4( cg_weapons[ es->weapon ].projExplosionEffect,
							   (int)cent->lerpOrigin, (int)dir );
		}
		if ( cg_weapons[ es->weapon ].projExplosionSound
			 && cg_weapons[ es->weapon ].projExplosionSound[0] ) {
			CG_PlaySoundAliasByName( cg_weapons[ es->weapon ].projExplosionSound,
									 ENTITYNUM_WORLD, cent->lerpOrigin );
		}
		cg_noMarks = 0;
		break;

	case EV_RAILTRAIL:
		CG_RailTrail( es->dmgFlags, es->pos.trBase, es->origin2 );
		break;

	case EV_PAIN:
	case EV_CROUCH_PAIN:
		break;

	case EV_DEATH:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.unknown_0x2E4[0] );
		break;

	case EV_OBITUARY:
		CG_Obituary( es );
		break;

	case EV_DEBUG_LINE:
		CG_Beam( cent );
		break;

	case EV_PLAY_FX:
		CG_PlayFx( cent, NULL );
		break;

	case EV_PLAY_FX_DIR:
		/* cg_public.h names entityState_t +0xD8 `scale`; this event uses it as
		   the packed direction byte (0x3001E6CE). */
		ByteToDir( es->scale, dir );
		CG_PlayFx( cent, dir );
		break;

	case EV_PLAY_FX_ON_TAG:
		CG_PlayFxOnTag( cent, es->eventParm );
		break;

	case EV_FLAMEBARREL_BOUNCE:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.flamebarrelBounce );
		break;

	case EV_EARTHQUAKE:
		break;

	case EV_ITEM_RESPAWN:
		CENT_MISCTIME( cent ) = cg.time;    // scale up from this
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.unknown_0x2E4[3] );
		break;

	case EV_ITEM_POP:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.unknown_0x2E4[3] );
		break;

	case EV_PLAYER_TELEPORT_IN:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.unknown_0x2E4[1] );
		break;

	case EV_PLAYER_TELEPORT_OUT:
		CG_PlayEntitySoundAliasByName( es->number, (const char *)cgs.media.unknown_0x2E4[2] );
		break;

	default:
		CG_Error( "Unknown event: '%s'", eventnames[event] );
		break;
	}
}

/*
==============
CG_EntityPreEvent     0x3001E820

The nextState half of the event stream: the same event fires one frame early so
the muzzle flash and the bullet impact are not a snapshot late.
==============
*/
static void CG_EntityPreEvent( centity_t *cent, int event ) {
	entityState_t   *ns;
	vec3_t          dir;
	vec3_t          dir2;

	if ( !event ) {
		if ( cg_debugevents.integer ) {
			CG_Printf( "CG_EntityPreEvent:ZERO EVENT\n" );
		}
		return;
	}

	ns = &cent->nextState;

	if ( cg_debugevents.integer ) {
		CG_Printf( "ent:%3i  preevent:%3i CG_EntityPreEvent:%s\n",
				   ns->number, event, eventnames[event] );
	}

	switch ( event ) {
	case EV_BULLET_HIT_SMALL:
	case EV_BULLET_HIT_LARGE:
		ByteToDir( ns->eventParm, dir );
		ByteToDir( ns->scale, dir2 );
		CG_BulletHitEvent( event, cent->lerpOrigin, ns->surfType, ns->otherEntityNum,
						   dir, dir2 );
		break;

	case EV_BULLET_HIT_CLIENT_SMALL:
	case EV_BULLET_HIT_CLIENT_LARGE:
		CG_BulletHitClientEvent( event, cent->lerpOrigin, ns->surfType, ns->otherEntityNum );
		break;

	case EV_FIRE_QUADBARREL_1:
		CG_FireWeapon( cent, ns, event, 0 );
		CG_FireWeapon( cent, ns, event, 1 );
		break;

	case EV_FIRE_QUADBARREL_2:
		CG_FireWeapon( cent, ns, event, 2 );
		CG_FireWeapon( cent, ns, event, 3 );
		break;

	case EV_FIRE_WEAPON_MG42:
		CG_StartShakeCamera( 0.05f, 100, 100.0f, cent->lerpOrigin );
		/* fall through */
	case EV_FIRE_WEAPON:
	case EV_FIRE_WEAPONB:
	case EV_FIRE_WEAPON_LASTSHOT:
		CG_FireWeapon( cent, ns, event, 0 );
		break;

	case EV_RECHAMBER_WEAPON:
		if ( cg_weapons[ ns->weapon ].rechamberSound ) {
			CG_PlayEntitySoundAliasByName( ns->number, cg_weapons[ ns->weapon ].rechamberSound );
		}
		break;

	case EV_EJECT_BRASS:
		CG_EjectWeaponBrass( ns, event );
		break;

	case EV_EARTHQUAKE:
		CG_StartShakeCamera( ns->angles2[0], ns->time, ns->angles2[1], cent->lerpOrigin );
		break;

	default:
		break;
	}
}

/*
==============
CG_CheckEvents     0x3001EA10
==============
*/
void CG_CheckEvents( centity_t *cent ) {
	int     i;
	int     event;
	byte    oldEventParm;

	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEventSequence ) {
			return;         // already fired
		}
		cent->previousEventSequence = 1;
		CG_CalcEntityLerpPositions( cent );
		CG_EntityEvent( cent, cent->currentState.eType - ET_EVENTS, qfalse );
		return;
	}

	if ( !cent->currentState.eventSequence ) {
		cent->previousEventSequence = 0;
		return;
	}

	// eventSequence is sent as an 8-bit value through the network
	if ( cent->currentState.eventSequence < cent->previousEventSequence ) {
		cent->previousEventSequence -= ( 1 << 8 );
	}
	if ( cent->currentState.eventSequence - cent->previousEventSequence > MAX_EVENTS ) {
		cent->previousEventSequence = cent->currentState.eventSequence - MAX_EVENTS;
	}

	if ( cent->previousEventSequence < cent->currentState.eventSequence ) {
		CG_CalcEntityLerpPositions( cent );

		oldEventParm = (byte)cent->currentState.eventParm;
		for ( i = cent->previousEventSequence ; i != cent->currentState.eventSequence ; i++ ) {
			event = cent->currentState.events[ i & ( MAX_EVENTS - 1 ) ];
			cent->currentState.eventParm = cent->currentState.eventParms[ i & ( MAX_EVENTS - 1 ) ];
			CG_EntityEvent( cent, event, qfalse );
		}
		cent->currentState.eventParm = oldEventParm;
	}

	cent->previousEventSequence = cent->currentState.eventSequence;
}

/*
==============
CG_CheckPreEvents     0x3001EB10

Note the eventParm save/restore: the loop clobbers nextState.eventParm
(cent+0x190) but the save and the restore both hit currentState.eventParm
(cent+0x0A0) -- 0x3001EBAD / 0x3001EBF4.  Retail quirk.
==============
*/
void CG_CheckPreEvents( centity_t *cent ) {
	int     i;
	int     event;
	byte    oldEventParm;

	if ( cent->nextState.eType > ET_EVENTS ) {
		if ( cent->previousPreEventSequence ) {
			return;         // already fired
		}
		cent->previousPreEventSequence = 1;
		CG_CalcEntityLerpPositions( cent );
		CG_EntityPreEvent( cent, cent->nextState.eType - ET_EVENTS );
		return;
	}

	if ( !cent->nextState.eventSequence ) {
		cent->previousPreEventSequence = 0;
		return;
	}

	if ( cent->nextState.eventSequence < cent->previousPreEventSequence ) {
		cent->previousPreEventSequence -= ( 1 << 8 );
	}
	if ( cent->nextState.eventSequence - cent->previousPreEventSequence > MAX_EVENTS ) {
		cent->previousPreEventSequence = cent->nextState.eventSequence - MAX_EVENTS;
	}

	if ( cent->previousPreEventSequence < cent->nextState.eventSequence ) {
		CG_CalcEntityLerpPositions( cent );

		oldEventParm = (byte)cent->currentState.eventParm;
		for ( i = cent->previousPreEventSequence ; i != cent->nextState.eventSequence ; i++ ) {
			event = cent->nextState.events[ i & ( MAX_EVENTS - 1 ) ];
			cent->nextState.eventParm = cent->nextState.eventParms[ i & ( MAX_EVENTS - 1 ) ];
			CG_EntityPreEvent( cent, event );
		}
		cent->currentState.eventParm = oldEventParm;
	}

	cent->previousPreEventSequence = cent->nextState.eventSequence;
}
