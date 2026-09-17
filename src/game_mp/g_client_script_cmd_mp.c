/*
 * @fidelity: likely
 *
 * g_client_script_cmd_mp.c -- the player script-method table.
 *
 * Every entry of player_methods[] is a method the script VM can call on an
 * entity that has a client: `player giveWeapon( "colt" )` lands in
 * PlayerCmd_giveWeapon.  A method takes nothing but the calling entity's
 * number, pulls its arguments off the VM stack through the Scr_Get* far
 * hooks, and either pushes a result with Scr_Add* or raises Scr_Error /
 * Scr_ParamError, which never return.
 *
 * CoD's script system has no RTCW ancestor, so nothing here is a port.
 *
 * Function order is binary order (0x2001A890 .. 0x2001D0E0).
 */

#include <string.h>

#include "g_local.h"

/* entity_event_t values; bg_misc.c owns the enum, no header carries it yet. */
#define EV_BULLET_HIT_SMALL         173
#define EV_BULLET_HIT_CLIENT_SMALL  175

/* entityShared_t.svFlags, spelled as server_mp/sv_snapshot_mp.c spells them
   (SVF_CAPSULE is sv_world_mp.c's). */
#define SVF_SINGLECLIENT            0x00000800
#define SVF_CAPSULE                 0x00000200
#define SVF_NOTSINGLECLIENT         0x00002000

/* gentity_t.flags */
#define FL_NO_KNOCKBACK             0x00000008

/* playerState_t.pm_flags, as bg_pmove.c names them. */
#define PMF_PRONE                   0x0001
#define PMF_DUCKED                  0x0002
#define PMF_TIME_KNOCKBACK          0x0200

/* usercmd_t.buttons; each bit is named by the method that reports it. */
#define BUTTON_ATTACK               0x00000001
#define BUTTON_MELEE                0x00000020
#define BUTTON_USE                  0x00000040

/* entityState_t.eFlags */
#define EF_TELEPORT_BIT             0x00000008
/* Raised on a corpse clone and cleared 250 msec later by BodyEnd; the client
   name for the bit is not recovered, so this one is inferred. */
#define EF_CLONE_TEMP               0x00000800
/* ps.eFlags bit PlayerCmd_pingPlayer raises for the compass ping. */
#define EF_PING                     0x00080000

#define ET_PLAYER_CORPSE            2

#define TR_STATIONARY               0
#define TR_GRAVITY                  5

/* trace_t surface type the blood effect is faked with. */
#define SURF_TYPE_FLESH             7

#define STAT_HEALTH                 0

#define ENTITYNUM_WORLD             1022

/* trap_GetConfigstringConst base (g_utils_mp.c's CS_MODELS, 268). */
#define CS_MODELS                   268

/* Script variable types Scr_GetType / Scr_GetPointerType return. */
#define VAR_LOCALIZED_STRING        2
#define VAR_POINTER                 7
#define VAR_ENTITY                  13

/* Scr_GetEntityNum / Scr_AddEntityNum class id, as g_spawn_mp.c spells it. */
#define SCR_OBJECT_ENTITY           0

/* Scr_ConstructMessageString's first argument -- the label it puts in front of
   a "%s is too long" error.  4 is "Client Cvar Value", 5 "Client Chat
   Message" (the switch at 0x2002F2E4). */
#define MSGTYPE_CLIENT_CVAR_VALUE   4
#define MSGTYPE_CLIENT_CHAT         5

/* The escape bytes Scr_ConstructMessageString emits ahead of a run: 20 marks a
   localized reference, 21 a literal. */
#define MSGCHAR_LOCALIZED           20

/* forward: bg_weapon.c */
int BG_GetWeaponIndexForName( const char *name );
int BG_GetWeaponSlotForName( const char *name );

/* forward: q_math.c */
byte DirToByte( const vec3_t dir );

/* forward: g_main_mp.c -- gameCvarTable's "g_knockback" row (0x20069F30). */
extern vmCvar_t g_knockback;                     /* 0x20234D60 */

/* forward: g_client_mp.c */
void ClientSpawn( gentity_t *ent, const vec3_t origin, const vec3_t angles );

/*
==============
GetPlayerEntity
	Validates the entity the method was called on.  Neither Scr_Error returns,
	so the NULL is never handed back in practice.  The binary has no standalone
	copy -- it is inlined at all 43 call sites in this file.
==============
*/
static gentity_t *GetPlayerEntity( unsigned int entnum ) {
	gentity_t   *ent;

	if ( entnum >= MAX_GENTITIES ) {
		Scr_Error( va( "%i is not a valid entity number", entnum ) );
		return NULL;
	}

	ent = &g_entities[entnum];
	if ( !ent->client ) {
		Scr_Error( va( "entity %i is not a player", entnum ) );
	}

	return ent;
}

/*
==============
PlayerCmd_giveWeapon
==============
*/
void PlayerCmd_giveWeapon( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;
	qboolean hadWeapon;
	int count;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	hadWeapon = ( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) != 0;

	if ( !BG_GetEmptySlotForWeapon( &ent->client->ps, weapon ) ) {
		Scr_ParamError( 0, "Can not give player weapon without having an empty weapon slot\n" );
	}

	BG_GivePlayerWeapon( &ent->client->ps, weapon );

	count = bg_weaponInfo[weapon]->startAmmo
			- ent->client->ps.ammo[bg_weaponInfo[weapon]->ammoIndex];
	if ( count > 0 ) {
		Add_Ammo( ent, weapon, count, !hadWeapon );
	}
}

/*
==============
PlayerCmd_takeWeapon
==============
*/
void PlayerCmd_takeWeapon( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	ent->client->ps.ammo[bg_weaponInfo[weapon]->ammoIndex] = 0;
	ent->client->ps.ammoclip[bg_weaponInfo[weapon]->clipIndex] = 0;

	BG_TakePlayerWeapon( &ent->client->ps, weapon );
}

/*
==============
PlayerCmd_takeAllWeapons
==============
*/
void PlayerCmd_takeAllWeapons( unsigned int entnum ) {
	gentity_t   *ent;
	int i;

	ent = GetPlayerEntity( entnum );

	ent->client->ps.weapon = 0;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		ent->client->ps.ammo[bg_weaponInfo[i]->ammoIndex] = 0;
		ent->client->ps.ammoclip[bg_weaponInfo[i]->clipIndex] = 0;
		BG_TakePlayerWeapon( &ent->client->ps, i );
	}
}

/*
==============
PlayerCmd_getCurrentWeapon
==============
*/
void PlayerCmd_getCurrentWeapon( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	if ( (int)ent->client->ps.weapon > 0 ) {
		Scr_AddString( bg_weaponInfo[ent->client->ps.weapon]->szInternalName );
	} else {
		Scr_AddString( "none" );
	}
}

/*
==============
PlayerCmd_hasWeapon
==============
*/
void PlayerCmd_hasWeapon( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	if ( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) {
		Scr_AddBool( qtrue );
	} else {
		Scr_AddBool( qfalse );
	}
}

/*
==============
PlayerCmd_switchToWeapon
==============
*/
void PlayerCmd_switchToWeapon( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	if ( !( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
		Scr_AddBool( qfalse );
		return;
	}

	trap_SendServerCommand( ent - g_entities, 1, va( "a %i", weapon ) );
	Scr_AddBool( qtrue );
}

/*
==============
PlayerCmd_giveStartAmmo
==============
*/
void PlayerCmd_giveStartAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;
	int count;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	if ( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) {
		count = bg_weaponInfo[weapon]->startAmmo
				- ent->client->ps.ammo[bg_weaponInfo[weapon]->ammoIndex];
		if ( count > 0 ) {
			Add_Ammo( ent, weapon, count, qfalse );
		}
	}
}

/*
==============
PlayerCmd_giveMaxAmmo
==============
*/
void PlayerCmd_giveMaxAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;
	int ammoIndex;
	int count;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	if ( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) {
		ammoIndex = bg_weaponInfo[weapon]->ammoIndex;
		count = bg_ammoTypeMax[ammoIndex] - ent->client->ps.ammo[ammoIndex];
		if ( count > 0 ) {
			Add_Ammo( ent, weapon, count, qfalse );
		}
	}
}

/*
==============
PlayerCmd_getFractionStartAmmo
==============
*/
void PlayerCmd_getFractionStartAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;
	int ammo;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	if ( !( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
		Scr_AddFloat( 1.0 );
		return;
	}

	if ( bg_weaponInfo[weapon]->startAmmo < 1 ) {
		Scr_AddFloat( 1.0 );
		return;
	}

	ammo = ent->client->ps.ammo[bg_weaponInfo[weapon]->ammoIndex];
	if ( ammo < 1 ) {
		Scr_AddFloat( 0 );
		return;
	}

	Scr_AddFloat( (float)ammo / (float)bg_weaponInfo[weapon]->startAmmo );
}

/*
==============
PlayerCmd_getFractionMaxAmmo
==============
*/
void PlayerCmd_getFractionMaxAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;
	int ammoIndex;
	int ammo;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	if ( !( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
		Scr_AddFloat( 1.0 );
		return;
	}

	ammoIndex = bg_weaponInfo[weapon]->ammoIndex;
	if ( bg_ammoTypeMax[ammoIndex] < 1 ) {
		Scr_AddFloat( 1.0 );
		return;
	}

	ammo = ent->client->ps.ammo[ammoIndex];
	if ( ammo < 1 ) {
		Scr_AddFloat( 0 );
		return;
	}

	Scr_AddFloat( (float)ammo / (float)bg_ammoTypeMax[ammoIndex] );
}

/*
==============
PlayerCmd_setOrigin
==============
*/
void PlayerCmd_setOrigin( unsigned int entnum ) {
	gentity_t   *ent;
	vec3_t origin;

	ent = GetPlayerEntity( entnum );

	Scr_GetVector( 0, origin );

	trap_UnlinkEntity( ent );

	VectorCopy( origin, ent->client->ps.origin );
	ent->client->ps.origin[2] += 1.0;

	/* toggle the teleport bit so the client doesn't lerp the jump */
	ent->client->ps.eFlags ^= EF_TELEPORT_BIT;

	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	trap_LinkEntity( ent );
}

/*
==============
PlayerCmd_setAngles
==============
*/
void PlayerCmd_setAngles( unsigned int entnum ) {
	gentity_t   *ent;
	vec3_t angles;

	ent = GetPlayerEntity( entnum );

	Scr_GetVector( 0, angles );
	SetClientViewAngle( ent, angles );
}

/*
==============
PlayerCmd_useButtonPressed
==============
*/
void PlayerCmd_useButtonPressed( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	if ( ent->client->buttons & BUTTON_USE ) {
		Scr_AddInt( 1 );
	} else {
		Scr_AddInt( 0 );
	}
}

/*
==============
PlayerCmd_attackButtonPressed
==============
*/
void PlayerCmd_attackButtonPressed( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	if ( ent->client->buttons & BUTTON_ATTACK ) {
		Scr_AddInt( 1 );
	} else {
		Scr_AddInt( 0 );
	}
}

/*
==============
PlayerCmd_meleeButtonPressed
==============
*/
void PlayerCmd_meleeButtonPressed( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	if ( ent->client->buttons & BUTTON_MELEE ) {
		Scr_AddInt( 1 );
	} else {
		Scr_AddInt( 0 );
	}
}

/*
==============
PlayerCmd_isOnGround
==============
*/
void PlayerCmd_isOnGround( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	if ( ent->client->ps.groundEntityNum == ENTITYNUM_NONE ) {
		Scr_AddInt( 0 );
	} else {
		Scr_AddInt( 1 );
	}
}

/*
==============
PlayerCmd_pingPlayer
==============
*/
void PlayerCmd_pingPlayer( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	ent->client->ps.eFlags |= EF_PING;
	ent->client->pingTime = level.time + 3000;
}

/*
==============
PlayerCmd_SetViewmodel
==============
*/
void PlayerCmd_SetViewmodel( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	ent->client->sess.viewmodel = G_ModelIndex( Scr_GetString( 0 ) );
}

/*
==============
PlayerCmd_GetViewmodel
==============
*/
void PlayerCmd_GetViewmodel( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	Scr_AddString( trap_GetConfigstringConst( CS_MODELS + ent->client->sess.viewmodel ) );
}

/*
==============
PlayerCmd_allowComplaint
	Arms the "file a complaint about this player" prompt on ent's client.
==============
*/
void PlayerCmd_allowComplaint( unsigned int entnum ) {
	gentity_t   *ent;
	gentity_t   *other;

	ent = GetPlayerEntity( entnum );

	other = Scr_GetEntity( 0 );
	if ( ent == other ) {
		return;
	}

	/* you cannot complain about the listen-server host */
	if ( other->client->sess.localClient ) {
		trap_SendServerCommand( ent - g_entities, 1, "m -4" );
		return;
	}

	trap_SendServerCommand( ent - g_entities, 1, va( "m %i", other->s.number ) );
	ent->client->sess.complaintClient = other->s.clientNum;
	ent->client->sess.complaintEndTime = level.time + 20500;
}

/*
==============
PlayerCmd_showScoreboard
==============
*/
void PlayerCmd_showScoreboard( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	DeathmatchScoreboardMessage( ent );
}

/*
==============
PlayerCmd_setSpawnWeapon
==============
*/
void PlayerCmd_setSpawnWeapon( unsigned int entnum ) {
	gentity_t   *ent;
	int weapon;

	ent = GetPlayerEntity( entnum );

	weapon = BG_GetWeaponIndexForName( Scr_GetString( 0 ) );

	if ( ent->client->ps.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) {
		ent->client->ps.weapon = weapon;
		ent->client->ps.weaponstate = WEAPON_READY;
	}
}

/*
==============
PlayerCmd_dropItem
==============
*/
void PlayerCmd_dropItem( unsigned int entnum ) {
	gentity_t   *ent;
	gentity_t   *drop;
	const char  *name;
	int weapon;
	gitem_t     *item;

	ent = GetPlayerEntity( entnum );

	name = Scr_GetString( 0 );
	weapon = BG_GetWeaponIndexForName( name );

	if ( weapon ) {
		if ( Scr_GetNumParam() > 1 ) {
			drop = Drop_Weapon( ent, weapon, Scr_GetString( 1 ) );
		} else {
			drop = Drop_Weapon( ent, weapon, "tag_weapon_right" );
		}
	} else {
		item = BG_FindItem( name );
		if ( !item ) {
			Scr_AddUndefined();
			return;
		}
		drop = Drop_Item( ent, item, 0, qfalse );
	}

	if ( drop ) {
		Scr_AddEntity( drop );
	} else {
		Scr_AddUndefined();
	}
}

/*
==============
PlayerCmd_finishPlayerDamage
	The back half of G_Damage for players: the script gets the damage callback
	first, decides on a final amount, then calls this to apply it.
==============
*/
void PlayerCmd_finishPlayerDamage( unsigned int entnum ) {
	gentity_t   *ent;
	gentity_t   *inflictor;
	gentity_t   *attacker;
	gentity_t   *tent;
	const weaponInfo_t  *weapInfo;
	vec3_t point;
	vec3_t vDir;
	vec3_t dir;
	vec3_t kvel;
	const float *pPoint;
	const float *pDir;
	int damage;
	int dflags;
	int mod;
	int weapon;
	int hitLoc;
	int knockback;
	int t;
	float scale;

	inflictor = &g_entities[ENTITYNUM_WORLD];
	attacker = &g_entities[ENTITYNUM_WORLD];
	pPoint = NULL;
	pDir = NULL;

	ent = GetPlayerEntity( entnum );

	damage = Scr_GetInt( 2 );
	if ( damage <= 0 ) {
		return;
	}

	/* retail quirk: the inflictor is gated on parameter 0 but read from
	   parameter 1, so `inflictor` can only ever be the attacker or world */
	if ( Scr_GetType( 0 ) && Scr_GetPointerType( 0 ) == VAR_ENTITY ) {
		inflictor = Scr_GetEntity( 1 );
	}
	if ( Scr_GetType( 1 ) && Scr_GetPointerType( 1 ) == VAR_ENTITY ) {
		attacker = Scr_GetEntity( 1 );
	}

	dflags = Scr_GetInt( 3 );
	mod = G_IndexForMeansOfDeath( Scr_GetString( 4 ) );
	weapon = BG_GetWeaponIndexForName( Scr_GetString( 5 ) );

	if ( Scr_GetType( 6 ) ) {
		Scr_GetVector( 6, point );
		pPoint = point;
	}
	if ( Scr_GetType( 7 ) ) {
		Scr_GetVector( 7, vDir );
		pDir = vDir;
	}

	hitLoc = G_GetHitLocationIndexFromString( Scr_GetConstString( 8 ) );

	if ( pDir ) {
		VectorNormalize2( pDir, dir );
	} else {
		VectorClear( dir );
	}

	if ( !( ent->flags & FL_NO_KNOCKBACK ) && !( dflags & DAMAGE_NO_KNOCKBACK ) ) {
		scale = 0.3f;
		if ( ent->client->ps.pm_flags & PMF_PRONE ) {
			scale = 0.02f;
		} else if ( ent->client->ps.pm_flags & PMF_DUCKED ) {
			scale = 0.15f;
		}

		knockback = (int)( (float)damage * scale );
		if ( knockback > 60 ) {
			knockback = 60;
		}

		if ( knockback ) {
			/* the reciprocal is the compiler's; the source divides by 250 */
			VectorScale( dir, (float)knockback * g_knockback.value * 0.004f, kvel );
			VectorAdd( ent->client->ps.velocity, kvel, ent->client->ps.velocity );

			/* set the timer so that the other client can't cancel out the
			   movement immediately */
			if ( !ent->client->ps.pm_time ) {
				t = knockback * 2;
				if ( t < 50 ) {
					t = 50;
				}
				if ( t > 200 ) {
					t = 200;
				}
				ent->client->ps.pm_time = t;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
			}
		}
	}

	if ( ent->flags & FL_GODMODE ) {
		return;
	}

	if ( weapon ) {
		weapInfo = bg_weaponInfo[weapon];
		if ( weapInfo->weaponType == WEAPTYPE_BULLET ) {
			/* the blood puff everyone but the victim sees, then the hit
			   feedback only the victim sees.  `point` is uninitialized when
			   parameter 6 was left out -- retail does this too. */
			tent = G_TempEntity( point, EV_BULLET_HIT_SMALL + ( weapInfo->rifleBullet != 0 ) );
			tent->s.eventParm = DirToByte( dir ) & 0xFF;
			tent->s.scale = DirToByte( dir ) & 0xFF;
			tent->s.surfType = SURF_TYPE_FLESH;
			tent->s.otherEntityNum = attacker->s.number;
			tent->r.svFlags |= SVF_NOTSINGLECLIENT;
			tent->r.singleClient = ent->client->ps.clientNum;

			tent = G_TempEntity( point, EV_BULLET_HIT_CLIENT_SMALL
								 + ( bg_weaponInfo[weapon]->rifleBullet != 0 ) );
			tent->s.surfType = SURF_TYPE_FLESH;
			tent->s.otherEntityNum = attacker->s.number;
			tent->s.clientNum = ent->client->ps.clientNum;
			tent->r.svFlags = SVF_SINGLECLIENT;
			tent->r.singleClient = ent->client->ps.clientNum;
		}
	}

	ent->client->damage_blood += damage;

	if ( pDir ) {
		VectorCopy( dir, ent->client->damage_from );
		ent->client->damage_fromWorld = qfalse;
	} else {
		VectorCopy( ent->r.currentOrigin, ent->client->damage_from );
		ent->client->damage_fromWorld = qtrue;
	}

	ent->health -= damage;

	Scr_AddEntity( attacker );
	Scr_AddInt( damage );
	Scr_Notify( ent, scr_const.damage, 2 );

	if ( ent->health > 0 ) {
		if ( ent->pain ) {
			ent->pain( ent, attacker, damage, pPoint, mod, dir, hitLoc );
		}
	} else {
		if ( ent->health < -999 ) {
			ent->health = -999;
		}

		ent->enemy = attacker;

		if ( ent->die ) {
			ent->die( ent, inflictor, attacker, damage, mod, weapon, dir, hitLoc );
		}

		if ( !ent->inuse ) {
			return;
		}
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;
}

/*
==============
PlayerCmd_Suicide
==============
*/
void PlayerCmd_Suicide( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	ent->client->ps.stats[STAT_HEALTH] = 0;

	player_die( ent, ent, ent, 100000, MOD_SUICIDE, 0, NULL, 0 );
}

/*
==============
PlayerCmd_OpenMenu
==============
*/
void PlayerCmd_OpenMenu( unsigned int entnum ) {
	gentity_t   *ent;
	int menuIndex;

	ent = GetPlayerEntity( entnum );

	if ( ent->client->sess.connected != CON_CONNECTED ) {
		Scr_AddInt( 0 );
		return;
	}

	menuIndex = GScr_GetScriptMenuIndex( Scr_GetString( 0 ) );
	trap_SendServerCommand( entnum, 1, va( "t %i", menuIndex ) );
	Scr_AddInt( 1 );
}

/*
==============
PlayerCmd_OpenMenuNoMouse
==============
*/
void PlayerCmd_OpenMenuNoMouse( unsigned int entnum ) {
	gentity_t   *ent;
	int menuIndex;

	ent = GetPlayerEntity( entnum );

	if ( ent->client->sess.connected != CON_CONNECTED ) {
		Scr_AddInt( 0 );
		return;
	}

	menuIndex = GScr_GetScriptMenuIndex( Scr_GetString( 0 ) );
	trap_SendServerCommand( entnum, 1, va( "t %i 1", menuIndex ) );
	Scr_AddInt( 1 );
}

/*
==============
PlayerCmd_CloseMenu
	The one method that does not validate its entity.
==============
*/
void PlayerCmd_CloseMenu( unsigned int entnum ) {
	trap_SendServerCommand( entnum, 1, "u" );
}

/*
==============
WeaponSlotsNotValid
	The weapon slots only mean anything while the client is in the game.
==============
*/
qboolean WeaponSlotsNotValid( gentity_t *ent ) {
	return ent->client->sess.sessionState != STATE_PLAYING;
}

/*
==============
PlayerCmd_GetWeaponSlotWeapon
==============
*/
void PlayerCmd_GetWeaponSlotWeapon( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned short name;
	int slot;

	ent = GetPlayerEntity( entnum );

	if ( WeaponSlotsNotValid( ent ) ) {
		Scr_AddConstString( scr_const.none );
		return;
	}

	name = Scr_GetConstString( 0 );
	slot = BG_GetWeaponSlotForName( SL_ConvertToString( name ) );
	if ( !slot ) {
		Scr_ParamError( 0, va( "Unknown weaponslot name %s. Valid weaponslots are "
							   "\"primary\", \"primaryb\", \"pistol\", \"grenade\", and \"smokegrenade\"",
							   SL_ConvertToString( name ) ) );
	}

	if ( !ent->client->ps.weaponslots[slot] ) {
		Scr_AddConstString( scr_const.none );
	} else {
		Scr_AddString( bg_weaponInfo[(char)ent->client->ps.weaponslots[slot]]->szInternalName );
	}
}

/*
==============
PlayerCmd_SetWeaponSlotWeapon
==============
*/
void PlayerCmd_SetWeaponSlotWeapon( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned short name;
	int slot;
	int weaponSlot;
	const char  *weaponName;
	int weapon;
	qboolean moveToPrimaryB;

	ent = GetPlayerEntity( entnum );

	name = Scr_GetConstString( 0 );
	slot = BG_GetWeaponSlotForName( SL_ConvertToString( name ) );
	if ( !slot ) {
		Scr_ParamError( 0, va( "Unknown weaponslot name %s. Valid weaponslots are "
							   "\"primary\", \"primaryb\", \"pistol\", \"grenade\", and \"smokegrenade\"",
							   SL_ConvertToString( name ) ) );
	}

	weaponName = Scr_GetString( 1 );
	if ( !Q_stricmp( weaponName, "none" ) ) {
		weapon = 0;
	} else {
		weapon = BG_GetWeaponIndexForName( weaponName );
		if ( !weapon ) {
			Scr_ParamError( 1, va( "Unknown weapon %s.", weaponName ) );
		}

		/* primary and primaryb hold the same class of weapon */
		weaponSlot = bg_weaponInfo[weapon]->weaponSlot;
		if ( weaponSlot != slot
			 && ( ( weaponSlot != WEAPSLOT_PRIMARY && weaponSlot != WEAPSLOT_PRIMARYB )
				  || ( slot != WEAPSLOT_PRIMARY && slot != WEAPSLOT_PRIMARYB ) ) ) {
			Scr_ParamError( 1, va( "Weapon %s goes in the %s weaponslot, not the %s weaponslot.",
								   weaponName, bg_weaponSlotNames[weaponSlot],
								   bg_weaponSlotNames[slot] ) );
		}
	}

	if ( ent->client->ps.weaponslots[slot] ) {
		BG_TakePlayerWeapon( &ent->client->ps, (char)ent->client->ps.weaponslots[slot] );
	}

	if ( weapon ) {
		/* BG_GivePlayerWeapon fills the first free slot, so a primaryb with an
		   empty primary lands in the wrong one and has to be moved across */
		moveToPrimaryB = ( slot == WEAPSLOT_PRIMARYB
						   && !ent->client->ps.weaponslots[WEAPSLOT_PRIMARY] );

		BG_GivePlayerWeapon( &ent->client->ps, weapon );

		if ( moveToPrimaryB ) {
			ent->client->ps.weaponslots[WEAPSLOT_PRIMARYB] =
				ent->client->ps.weaponslots[WEAPSLOT_PRIMARY];
			ent->client->ps.weaponslots[WEAPSLOT_PRIMARY] = 0;
		}
	}
}

/*
==============
PlayerCmd_GetWeaponSlotAmmo
==============
*/
void PlayerCmd_GetWeaponSlotAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned short name;
	int slot;
	const weaponInfo_t  *weapInfo;

	ent = GetPlayerEntity( entnum );

	if ( WeaponSlotsNotValid( ent ) ) {
		Scr_AddInt( 0 );
		return;
	}

	name = Scr_GetConstString( 0 );
	slot = BG_GetWeaponSlotForName( SL_ConvertToString( name ) );
	if ( !slot ) {
		Scr_ParamError( 0, va( "Unknown weaponslot name %s. Valid weaponslots are "
							   "\"primary\", \"primaryb\", \"pistol\", \"grenade\", and \"smokegrenade\"",
							   SL_ConvertToString( name ) ) );
	}

	if ( !ent->client->ps.weaponslots[slot] ) {
		Scr_AddInt( 0 );
		return;
	}

	weapInfo = bg_weaponInfo[(char)ent->client->ps.weaponslots[slot]];
	if ( weapInfo->clipOnly ) {
		Scr_AddInt( ent->client->ps.ammoclip[weapInfo->clipIndex] );
	} else {
		Scr_AddInt( ent->client->ps.ammo[weapInfo->ammoIndex] );
	}
}

/*
==============
PlayerCmd_SetWeaponSlotAmmo
==============
*/
void PlayerCmd_SetWeaponSlotAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned short name;
	int slot;
	int ammo;
	const weaponInfo_t  *weapInfo;
	int index;

	ent = GetPlayerEntity( entnum );

	name = Scr_GetConstString( 0 );
	slot = BG_GetWeaponSlotForName( SL_ConvertToString( name ) );
	if ( !slot ) {
		Scr_ParamError( 0, va( "Unknown weaponslot name %s. Valid weaponslots are "
							   "\"primary\", \"primaryb\", \"pistol\", \"grenade\", and \"smokegrenade\"",
							   SL_ConvertToString( name ) ) );
	}

	ammo = Scr_GetInt( 1 );

	if ( !ent->client->ps.weaponslots[slot] ) {
		return;
	}

	weapInfo = bg_weaponInfo[(char)ent->client->ps.weaponslots[slot]];
	if ( weapInfo->clipOnly ) {
		index = weapInfo->clipIndex;
		if ( index ) {
			if ( ammo < 0 ) {
				ammo = 0;
			}
			if ( ammo > bg_ammoClipSizes[index] ) {
				ammo = bg_ammoClipSizes[index];
			}
			ent->client->ps.ammoclip[index] = ammo;
		}
	} else {
		index = weapInfo->ammoIndex;
		if ( index ) {
			if ( ammo < 0 ) {
				ammo = 0;
			}
			if ( ammo > bg_ammoTypeMax[index] ) {
				ammo = bg_ammoTypeMax[index];
			}
			ent->client->ps.ammo[index] = ammo;
		}
	}
}

/*
==============
PlayerCmd_GetWeaponSlotClipAmmo
==============
*/
void PlayerCmd_GetWeaponSlotClipAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned short name;
	int slot;
	int clipIndex;

	ent = GetPlayerEntity( entnum );

	if ( WeaponSlotsNotValid( ent ) ) {
		Scr_AddInt( 0 );
		return;
	}

	name = Scr_GetConstString( 0 );
	slot = BG_GetWeaponSlotForName( SL_ConvertToString( name ) );
	if ( !slot ) {
		Scr_ParamError( 0, va( "Unknown weaponslot name %s. Valid weaponslots are "
							   "\"primary\", \"primaryb\", \"pistol\", \"grenade\", and \"smokegrenade\"",
							   SL_ConvertToString( name ) ) );
	}

	if ( !ent->client->ps.weaponslots[slot] ) {
		Scr_AddInt( 0 );
		return;
	}

	clipIndex = bg_weaponInfo[(char)ent->client->ps.weaponslots[slot]]->clipIndex;
	if ( !clipIndex ) {
		Scr_AddInt( 0 );
		return;
	}

	Scr_AddInt( ent->client->ps.ammoclip[clipIndex] );
}

/*
==============
PlayerCmd_SetWeaponSlotClipAmmo
	The odd one out: a setter that still pushes a return value when the slot
	is empty.
==============
*/
void PlayerCmd_SetWeaponSlotClipAmmo( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned short name;
	int slot;
	int ammo;
	int clipIndex;

	ent = GetPlayerEntity( entnum );

	name = Scr_GetConstString( 0 );
	slot = BG_GetWeaponSlotForName( SL_ConvertToString( name ) );
	if ( !slot ) {
		Scr_ParamError( 0, va( "Unknown weaponslot name %s. Valid weaponslots are "
							   "\"primary\", \"primaryb\", \"pistol\", \"grenade\", and \"smokegrenade\"",
							   SL_ConvertToString( name ) ) );
	}

	ammo = Scr_GetInt( 1 );

	if ( !ent->client->ps.weaponslots[slot] ) {
		Scr_AddInt( 0 );
		return;
	}

	clipIndex = bg_weaponInfo[(char)ent->client->ps.weaponslots[slot]]->clipIndex;
	if ( clipIndex ) {
		if ( ammo < 0 ) {
			ammo = 0;
		}
		if ( ammo > bg_ammoClipSizes[clipIndex] ) {
			ammo = bg_ammoClipSizes[clipIndex];
		}
		ent->client->ps.ammoclip[clipIndex] = ammo;
	}
}

/*
==============
iclientprintln
==============
*/
void iclientprintln( unsigned int entnum ) {
	Scr_MakeGameMessage( entnum, "f" );
}

/*
==============
iclientprintlnbold
==============
*/
void iclientprintlnbold( unsigned int entnum ) {
	Scr_MakeGameMessage( entnum, "g" );
}

/*
==============
PlayerCmd_spawn
==============
*/
void PlayerCmd_spawn( unsigned int entnum ) {
	gentity_t   *ent;
	vec3_t origin;
	vec3_t angles;

	ent = GetPlayerEntity( entnum );

	Scr_GetVector( 0, origin );
	Scr_GetVector( 1, angles );

	ClientSpawn( ent, origin, angles );
}

/*
==============
PlayerCmd_setEnterTime
==============
*/
void PlayerCmd_setEnterTime( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	ent->client->sess.enterTime = Scr_GetInt( 0 );
}

/*
==============
BodyEnd
	The clone's think: drop the temp flag so the client stops drawing it.
==============
*/
void BodyEnd( gentity_t *ent ) {
	ent->s.eFlags &= ~EF_CLONE_TEMP;
}

/*
==============
PlayerCmd_ClonePlayer
	Leaves a ragdolling copy of the player behind, out of the body queue.
==============
*/
void PlayerCmd_ClonePlayer( unsigned int entnum ) {
	gentity_t   *ent;
	gentity_t   *body;

	ent = GetPlayerEntity( entnum );

	body = G_SpawnPlayerClone();

	body->s.clientNum = ent->client->ps.clientNum;
	body->s.eFlags = ( body->s.eFlags & EF_TELEPORT_BIT )
					 | ( ent->client->ps.eFlags & ~EF_TELEPORT_BIT )
					 | EF_CLONE_TEMP;

	G_SetOrigin( body, ent->r.currentOrigin );
	G_SetAngle( body, ent->r.currentAngles );

	body->s.pos.trType = TR_GRAVITY;
	body->s.pos.trTime = level.time;
	VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );

	body->s.eType = ET_PLAYER_CORPSE;
	body->physicsObject = qtrue;
	body->s.groundEntityNum = ENTITYNUM_NONE;

	body->r.svFlags = SVF_CAPSULE;
	VectorCopy( ent->r.mins, body->r.mins );
	VectorCopy( ent->r.maxs, body->r.maxs );
	VectorCopy( ent->r.absmin, body->r.absmin );
	VectorCopy( ent->r.absmax, body->r.absmax );

	body->s.legsAnim = ent->client->ps.legsAnim;
	body->s.torsoAnim = ent->client->ps.torsoAnim;

	body->clipmask = 0x10001;

	trap_LinkEntity( body );

	body->nextthink = level.time + 250;
	body->think = BodyEnd;

	Scr_AddEntity( body );
}

/*
==============
PlayerCmd_SetClientCvar
==============
*/
void PlayerCmd_SetClientCvar( unsigned int entnum ) {
	const char  *cvarName;
	const char  *string;
	char value[1024];
	char message[1024];
	int i;

	GetPlayerEntity( entnum );

	cvarName = Scr_GetString( 0 );

	if ( Scr_GetType( 1 ) == VAR_LOCALIZED_STRING ) {
		Scr_ConstructMessageString( MSGTYPE_CLIENT_CVAR_VALUE, 1, message, sizeof( message ) );
		string = message;
	} else {
		string = Scr_GetString( 1 );
	}

	/* the destination is 1024 bytes but the loop bound is 8192 -- retail */
	memset( value, 0, sizeof( value ) );
	for ( i = 0; i < 8192; i++ ) {
		char c;

		c = string[i];
		if ( !c ) {
			break;
		}
		if ( c == (char)0x92 ) {
			c = '\'';
		} else if ( (byte)c > 0x7F ) {
			c = '.';
		}
		value[i] = c;
		if ( value[i] == '"' ) {
			value[i] = '\'';
		}
	}

	trap_SendServerCommand( entnum, 1, va( "v %s \"%s\"", cvarName, value ) );
}

/*
==============
PlayerCmd_FreezeControls
==============
*/
void PlayerCmd_FreezeControls( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	ent->client->frozen = Scr_GetBool( 0 );
}

/*
==============
PlayerCmd_SetReverb
==============
*/
void PlayerCmd_SetReverb( unsigned int entnum ) {
	const char  *roomType;
	float wetLevel;
	float fadeTime;

	GetPlayerEntity( entnum );

	fadeTime = 0;
	wetLevel = 0.5f;

	switch ( Scr_GetNumParam() ) {
	case 3:
		fadeTime = Scr_GetFloat( 2 );
	case 2:
		wetLevel = Scr_GetFloat( 1 );
	case 1:
		break;
	default:
		Scr_Error( "USAGE: player setReverb(\"roomtype\", wetlevel = 0.5, fadetime = 1);\n"
				   "wetlevel is a float from 0 (no effect) to 1 (full effect), "
				   "fadetime is in sec and just modifies wetlevel\n" );
		break;
	}

	roomType = Scr_GetString( 0 );
	trap_SendServerCommand( entnum, 1, va( "r \"%s\" %g %g", roomType, wetLevel, fadeTime ) );
}

/*
==============
ScrCmd_IsLookingAt
==============
*/
void ScrCmd_IsLookingAt( unsigned int entnum ) {
	gentity_t   *ent;
	gentity_t   *other;

	ent = GetPlayerEntity( entnum );

	other = Scr_GetEntity( 0 );

	Scr_AddInt( ent->client->lookatEnt == other );
}

/*
==============
ScrCmd_PlayLocalSound
==============
*/
void ScrCmd_PlayLocalSound( unsigned int entnum ) {
	byte alias;

	GetPlayerEntity( entnum );

	alias = G_SoundAliasIndex( Scr_GetString( 0 ) );
	trap_SendServerCommand( entnum, 1, va( "s %i", alias ) );
}

/*
==============
PlayerCmd_SayAll
==============
*/
void PlayerCmd_SayAll( unsigned int entnum ) {
	gentity_t   *ent;
	char text[1024];

	ent = GetPlayerEntity( entnum );

	Scr_ConstructMessageString( MSGTYPE_CLIENT_CHAT, 0, &text[1], sizeof( text ) - 1 );
	text[0] = MSGCHAR_LOCALIZED;

	G_Say( ent, NULL, 0, text );
}

/*
==============
PlayerCmd_SayTeam
==============
*/
void PlayerCmd_SayTeam( unsigned int entnum ) {
	gentity_t   *ent;
	char text[1024];

	ent = GetPlayerEntity( entnum );

	Scr_ConstructMessageString( MSGTYPE_CLIENT_CHAT, 0, &text[1], sizeof( text ) - 1 );
	text[0] = MSGCHAR_LOCALIZED;

	G_Say( ent, NULL, 1, text );
}

typedef struct {
	const char  *name;
	PlayerCmd_t call;
} playerMethod_t;

static const playerMethod_t player_methods[] = {          /* 0x20055A90 */
	{ "giveweapon",             PlayerCmd_giveWeapon },
	{ "takeweapon",             PlayerCmd_takeWeapon },
	{ "takeallweapons",         PlayerCmd_takeAllWeapons },
	{ "getcurrentweapon",       PlayerCmd_getCurrentWeapon },
	{ "hasweapon",              PlayerCmd_hasWeapon },
	{ "switchtoweapon",         PlayerCmd_switchToWeapon },
	{ "givestartammo",          PlayerCmd_giveStartAmmo },
	{ "givemaxammo",            PlayerCmd_giveMaxAmmo },
	{ "getfractionstartammo",   PlayerCmd_getFractionStartAmmo },
	{ "getfractionmaxammo",     PlayerCmd_getFractionMaxAmmo },
	{ "setorigin",              PlayerCmd_setOrigin },
	{ "setplayerangles",        PlayerCmd_setAngles },
	{ "usebuttonpressed",       PlayerCmd_useButtonPressed },
	{ "attackbuttonpressed",    PlayerCmd_attackButtonPressed },
	{ "meleebuttonpressed",     PlayerCmd_meleeButtonPressed },
	{ "isonground",             PlayerCmd_isOnGround },
	{ "pingplayer",             PlayerCmd_pingPlayer },
	{ "setviewmodel",           PlayerCmd_SetViewmodel },
	{ "getviewmodel",           PlayerCmd_GetViewmodel },
	{ "sayall",                 PlayerCmd_SayAll },
	{ "sayteam",                PlayerCmd_SayTeam },
	{ "allowcomplaint",         PlayerCmd_allowComplaint },
	{ "showscoreboard",         PlayerCmd_showScoreboard },
	{ "setspawnweapon",         PlayerCmd_setSpawnWeapon },
	{ "dropitem",               PlayerCmd_dropItem },
	{ "finishplayerdamage",     PlayerCmd_finishPlayerDamage },
	{ "suicide",                PlayerCmd_Suicide },
	{ "openmenu",               PlayerCmd_OpenMenu },
	{ "openmenunomouse",        PlayerCmd_OpenMenuNoMouse },
	{ "closemenu",              PlayerCmd_CloseMenu },
	{ "freezecontrols",         PlayerCmd_FreezeControls },
	{ "setreverb",              PlayerCmd_SetReverb },
	{ "getweaponslotweapon",    PlayerCmd_GetWeaponSlotWeapon },
	{ "setweaponslotweapon",    PlayerCmd_SetWeaponSlotWeapon },
	{ "getweaponslotammo",      PlayerCmd_GetWeaponSlotAmmo },
	{ "setweaponslotammo",      PlayerCmd_SetWeaponSlotAmmo },
	{ "getweaponslotclipammo",  PlayerCmd_GetWeaponSlotClipAmmo },
	{ "setweaponslotclipammo",  PlayerCmd_SetWeaponSlotClipAmmo },
	{ "iprintln",               iclientprintln },
	{ "iprintlnbold",           iclientprintlnbold },
	{ "spawn",                  PlayerCmd_spawn },
	{ "setentertime",           PlayerCmd_setEnterTime },
	{ "cloneplayer",            PlayerCmd_ClonePlayer },
	{ "setclientcvar",          PlayerCmd_SetClientCvar },
	{ "islookingat",            ScrCmd_IsLookingAt },
	{ "playlocalsound",         ScrCmd_PlayLocalSound },
};

/*
==============
Player_GetMethod
	Resolves a method name for Scr_GetMethod and re-points *pName at the
	table's own copy of the string.
==============
*/
PlayerCmd_t Player_GetMethod( const char **pName ) {
	unsigned int i;

	for ( i = 0; i < sizeof( player_methods ) / sizeof( player_methods[0] ); i++ ) {
		if ( !strcmp( *pName, player_methods[i].name ) ) {
			*pName = player_methods[i].name;
			return player_methods[i].call;
		}
	}

	return NULL;
}
