/*
 * g_combat_mp.c -- damage and death for the multiplayer game module.
 *
 * The skeleton is RTCW's game/g_combat.c: LookAtKiller, player_die, G_Damage,
 * CanDamage and G_RadiusDamage are all still recognisable.  What CoD replaced
 * is the top of the file and the client path -- the hit-location table
 * (G_HitLocStrcpy .. G_GetHitLocationIndexFromString, parsed out of
 * info/mp_lochit_dmgtable) and G_DamageClient, which scales by the hit
 * location and hands the whole event to the script's Callback_PlayerDamage
 * instead of touching armour or knockback.  RTCW's own scoring is gone:
 * AddScore is an empty stub and the score lives in the script.
 *
 * Function order is binary order (0x2001FF90 .. 0x20020FF0).
 *
 * @fidelity: likely
 */

#include "g_local.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Declarations that belong in a shared header. */

#define ENTITYNUM_WORLD             ( MAX_GENTITIES - 2 )

#define PITCH                       0
#define YAW                         1
#define ROLL                        2

/* playerState_t.stats[].  Only the one LookAtKiller writes (0x2002025A). */
#define STAT_DEAD_YAW               1

/* entityState_t.eType.  4 is the missile type (fire_grenade / fire_rocket,
 * G_RunMissile's post-impact check); 5 is the mover branch of G_Damage below
 * (0x2002068A), which fires the entity's use function; 11 is g_utils_mp.c's
 * ET_TURRET, re-tested by player_die (0x20020324). */
#define ET_MOVER                    5
#define ET_TURRET                   11

/* The two mounted-turret bits of playerState_t.eFlags; the game only ever
 * tests them together (player_die 0x20020310, FireWeapon 0x2003D164). */
#define EF_TURRET_ACTIVE            0x0000C000

/* The event player_die raises on the corpse (0x20020456).  Not inferred:
   bg_misc.c's eventnames[] (0x2006C8B8) spells index 189 "EV_DEATH". */
#define EV_DEATH                    189

#define CONTENTS_CORPSE             0x04000000      /* player_die 0x200204FB */
#define MASK_ALL                    ( -1 )
#define MASK_GRENADE_TRACE          0x00000011      /* G_RadiusDamage 0x20020E25 */
#define MASK_CANDAMAGE              0x02802091      /* CanDamage 0x20020A2B */

/* The floor G_Damage clamps a dead entity's health to (0x2002077C). */
#define GIB_HEALTH                  -999

/*
 * q_shared.h carries these in the original tree.  CoD divides by 32768, not
 * Q3's 32767 (the multiplier at 0x2005FB04 is exactly 1/32768; player_die
 * 0x20020354).  crandom() is spelled as the machine spells it -- 2*r - 1, one
 * add and one subtract.
 */
#define random()                    ( rand() * ( 1.0f / 32768.0f ) )
#define crandom()                   ( 2.0f * random() - 1.0f )

/*
 * One entry of the parse table ParseConfigStringToStruct walks.  The hitloc
 * table only ever uses the float type.
 */
typedef struct parseField_s {
	const char      *key;
	int ofs;
	int type;
} parseField_t;

#define PARSE_FIELD_FLOAT           6

/* forward: q_shared.c / q_math.c */
float           VectorNormalize2( const vec3_t v, vec3_t out );
float           vectoyaw( const vec3_t vec );
void            AddLeanToPosition( vec3_t pos, float yaw, float leanFrac,
								   float rightScale, float upScale );
int             ParseConfigStringToStruct( const parseField_t *fields, void *base, int count,
										   const char *buffer, int unused1, void *unused2,
										   void ( *copyString )( char *dest, const char *src ) );

/* forward: g_main_mp.c */
void            Com_Printf( const char *fmt, ... );
extern vmCvar_t g_debugDamage;                  /* 0x202375E0 */

/* forward: g_cmds_mp.c */
void            DeathmatchScoreboardMessage( gentity_t *ent );

/*
 * forward: g_scr_main_mp.c.  Scr_PlayerDamage / Scr_PlayerKilled are the two
 * script callbacks this unit raises.  Scr_PlayerDamage takes its first four
 * arguments in registers.
 */
qboolean        Scr_PlayerDamage( int hitLoc, const vec3_t point, int mod, const vec3_t dir,
								  gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
								  int damage, int dflags, int weapon );
void            Scr_PlayerKilled( gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
								  int damage, int mod, int weapon, const float *dir, int hitLoc );

/*
 * ---------------------------------------------------------------------------
 * Hit-location and means-of-death tables.
 * ---------------------------------------------------------------------------
 */

/* 0x2006A510.  G_IndexForMeansOfDeath walks all MOD_NUM_MODS of them and
 * Scr_PlayerDamage indexes it to build the sMeansOfDeath script string. */
const char *modNames[MOD_NUM_MODS] = {
	"MOD_UNKNOWN",
	"MOD_PISTOL_BULLET",
	"MOD_RIFLE_BULLET",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_PROJECTILE",
	"MOD_PROJECTILE_SPLASH",
	"MOD_MELEE",
	"MOD_HEAD_SHOT",
	"MOD_MORTAR",
	"MOD_MORTAR_SPLASH",
	"MOD_KICKED",
	"MOD_GRABBER",
	"MOD_DYNAMITE",
	"MOD_DYNAMITE_SPLASH",
	"MOD_AIRSTRIKE",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TRIGGER_HURT",
	"MOD_EXPLOSIVE"
};

/* 0x2006A498 -- also the key each hitloc damage multiplier is parsed from. */
static const char *hitLocationNames[HITLOC_NUM] = {
	"none",
	"helmet",
	"head",
	"neck",
	"torso_upper",
	"torso_lower",
	"right_arm_upper",
	"left_arm_upper",
	"right_arm_lower",
	"left_arm_lower",
	"right_hand",
	"left_hand",
	"right_leg_upper",
	"left_leg_upper",
	"right_leg_lower",
	"left_leg_lower",
	"right_foot",
	"left_foot",
	"gun"
};

float g_fHitLocDamageMult[HITLOC_NUM];                  /* 0x2014F940 */
unsigned short hitLocationConstStrings[HITLOC_NUM];     /* 0x20089708 */

/*
==================
G_HitLocStrcpy

The string-copy hook ParseConfigStringToStruct calls back for string fields;
the hitloc table has none, so this only exists to fill the slot.
==================
*/
static void G_HitLocStrcpy( char *dest, const char *src ) {
	strcpy( dest, src );
}

/*
==================
G_ParseHitLocDmgTable
==================
*/
void G_ParseHitLocDmgTable( void ) {
	parseField_t fields[HITLOC_NUM];
	char buffer[8192];
	const char  *path = "info/mp_lochit_dmgtable";
	const char  *magic = "LOCDMGTABLE";
	fileHandle_t f;
	int len;
	int magicLen;
	int i;

	magicLen = strlen( magic );

	for ( i = 0 ; i < HITLOC_NUM ; i++ ) {
		g_fHitLocDamageMult[i] = 1.0;
		fields[i].key = hitLocationNames[i];
		fields[i].ofs = i * sizeof( float );
		fields[i].type = PARSE_FIELD_FLOAT;
		hitLocationConstStrings[i] = Scr_AllocString( hitLocationNames[i], 1 );
	}

	g_fHitLocDamageMult[HITLOC_GUN] = 0;

	len = trap_FS_FOpenFile( path, &f, FS_READ );
	if ( len <= 0 ) {
		Com_Error( ERR_DROP, "\x15" "Could not load hitloc damage table %s\n", path );
		return;
	}

	/* Only a positive length was required above: a file shorter than the magic
	   reads uninitialised stack here and makes the remainder below negative. */
	trap_FS_Read( buffer, magicLen, f );
	buffer[magicLen] = 0;
	if ( strncmp( buffer, magic, magicLen ) ) {
		Com_Error( ERR_DROP, "\x15" "\"%s\" does not appear to be a hitloc damage table\n", path );
		return;
	}

	len -= magicLen;
	if ( len >= 8192 ) {
		Com_Error( ERR_DROP, "\x15" "\"%s\" Is too long of a hitloc damage table to parse\n", path );
		return;
	}

	trap_FS_Read( buffer, len, f );
	buffer[len] = 0;
	trap_FS_FCloseFile( f );

	/* Info_Validate, inlined. */
	if ( strchr( buffer, '"' ) || strchr( buffer, ';' ) ) {
		Com_Error( ERR_DROP, "\x15" "\"%s\" is not a valid hitloc damage table\n", path );
		return;
	}

	if ( !ParseConfigStringToStruct( fields, g_fHitLocDamageMult, HITLOC_NUM, buffer,
									 0, NULL, G_HitLocStrcpy ) ) {
		G_Error( "Error parsing hitloc damage table %s\n", path );
	}
}

/*
=================
AddScore

Empty in 1.1 -- multiplayer scoring lives in the script.  The binary is a
single ret, ICF-folded with every other empty function, so the parameters are
RTCW's.
=================
*/
void AddScore( gentity_t *ent, int score ) {
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t dir;

	if ( attacker && attacker != self ) {
		VectorSubtract( attacker->r.currentOrigin, self->r.currentOrigin, dir );
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract( inflictor->r.currentOrigin, self->r.currentOrigin, dir );
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->r.currentAngles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw( dir );
}

/*
==================
G_IndexForMeansOfDeath
==================
*/
int G_IndexForMeansOfDeath( const char *name ) {
	int i;

	for ( i = 0 ; i < MOD_NUM_MODS ; i++ ) {
		if ( !Q_stricmp( modNames[i], name ) ) {
			return i;
		}
	}

	Com_Printf( "Unknown means of death string '%s'\n", name );
	return 0;
}

/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage,
				 int mod, int weapon, const float *dir, int hitLoc ) {
	gentity_t   *turret;
	gclient_t   *client;
	vec3_t start, throwDir;
	int i;

	if ( self->client->ps.pm_type >= PM_DEAD ) {
		return;
	}

	Scr_AddEntity( attacker );
	Scr_Notify( self, scr_const.death, 1 );

	// a kill from a mounted turret is credited to the turret's weapon
	if ( weapon && attacker->client ) {
		if ( attacker->client->ps.eFlags & EF_TURRET_ACTIVE ) {
			turret = &g_entities[attacker->s.otherEntityNum];
			if ( turret->s.eType == ET_TURRET ) {
				weapon = turret->s.weapon;
			}
		}
	}

	self->enemy = attacker;

	// a cooking grenade goes off where the player fell
	if ( self->client->ps.grenadeTimeLeft ) {
		throwDir[0] = crandom() * 160;
		throwDir[1] = crandom() * 160;
		throwDir[2] = random() * 160;

		start[0] = self->r.currentOrigin[0];
		start[1] = self->r.currentOrigin[1];
		start[2] = self->r.currentOrigin[2] + 40;

		fire_grenade( self, start, throwDir, self->s.weapon );
	}

	client = self->client;
	BG_AnimScriptEvent( &client->ps, ANIM_ET_DEATH, qfalse, qtrue );

	G_AddEvent( self, EV_DEATH, 0 );

	Scr_PlayerKilled( self, inflictor, attacker, damage, mod, weapon, dir, hitLoc );

	// refresh the scoreboard of everyone spectating the dead player
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.connected != CON_CONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionState != STATE_SPECTATOR ) {
			continue;
		}
		if ( level.clients[i].spectatorClient != self->s.number ) {
			continue;
		}
		DeathmatchScoreboardMessage( &g_entities[i] );
	}

	self->takedamage = qtrue;               // can still be gibbed
	self->r.contents = CONTENTS_CORPSE;
	self->r.currentAngles[ROLL] = 0;

	LookAtKiller( self, inflictor, attacker );

	VectorCopy( self->r.currentAngles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	trap_UnlinkEntity( self );
	self->r.maxs[2] = 30;
	trap_LinkEntity( self );

	self->health = 0;
	self->die = NULL;

	trap_LinkEntity( self );
}

/*
============
G_DamageClient

Everything a client takes goes through the script: the hit location scales the
damage, then Callback_PlayerDamage decides what to do with it.
============
*/
void G_DamageClient( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, const vec3_t dir,
					 const vec3_t point, int damage, int dflags, int mod, int hitLoc ) {
	int weapon;

	if ( !targ->takedamage ) {
		return;
	}
	if ( targ->client->noclip || targ->client->ufo ) {
		return;
	}
	if ( targ->client->sess.connected != CON_CONNECTED ) {
		return;
	}

	if ( inflictor ) {
		weapon = inflictor->s.weapon;
	} else if ( attacker ) {
		weapon = attacker->s.weapon;
	} else {
		weapon = 0;
	}

	Scr_PlayerDamage( hitLoc, point, mod, dir, targ, inflictor, attacker,
					  (int)( damage * g_fHitLocDamageMult[hitLoc] ), dflags, weapon );
}

/*
============
G_Damage

targ        entity that is being damaged
inflictor   entity that is causing the damage
attacker    entity that caused the inflictor to damage targ
dir         direction of the attack
point       point at which the damage is being inflicted
damage      amount of damage being inflicted
dflags      passed straight on to the script as iDFlags; 1.1 never tests it
mod         means-of-death index
hitLoc      hit location index, indexes the hitloc damage multiplier table

`attacker` and `dflags` are the two arguments LTCG register-allocated (eax and
ecx); call sites: pain and die (0x20020875, 0x200207DB), G_DamageClient's
forwarding (0x20020655).
============
*/
void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, const vec3_t dir,
			   const vec3_t point, int damage, int dflags, int mod, int hitLoc ) {
	vec3_t normalizedDir;
	int take;

	if ( targ->client ) {
		G_DamageClient( targ, inflictor, attacker, dir, point, damage, dflags, mod, hitLoc );
		return;
	}

	if ( !targ->takedamage ) {
		return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) {
		if ( targ->use && !targ->moverState ) {
			Scr_AddEntity( attacker );
			Scr_Notify( targ, scr_const.trigger, 1 );
			targ->use( targ, inflictor, attacker );
		}
		return;
	}

	// unconditional, and ahead of the godmode test -- a NULL dir reaches this
	VectorNormalize2( dir, normalizedDir );

	if ( targ->flags & FL_GODMODE ) {
		return;
	}

	take = damage;
	if ( take < 1 ) {
		take = 1;
	}

	if ( g_debugDamage.integer ) {
		G_Printf( "target:%i health:%i damage:%i\n", targ->s.number, targ->health, take );
	}

	// do the damage
	targ->health -= take;

	Scr_AddEntity( attacker );
	Scr_AddInt( take );
	Scr_Notify( targ, scr_const.damage, 2 );

	if ( targ->health <= 0 ) {
		if ( targ->health < GIB_HEALTH ) {
			targ->health = GIB_HEALTH;
		}

		Scr_AddEntity( attacker );
		Scr_Notify( targ, scr_const.death, 1 );

		targ->enemy = attacker;
		if ( targ->die ) {
			targ->die( targ, inflictor, attacker, take, mod, inflictor->s.weapon,
					   normalizedDir, hitLoc );
		}
	} else if ( targ->pain ) {
		if ( dir ) {
			VectorCopy( normalizedDir, targ->rotate );
			VectorCopy( point, targ->pos3 );
		} else {
			VectorClear( targ->rotate );
			VectorClear( targ->pos3 );
		}
		targ->pain( targ, attacker, take, point, mod, normalizedDir, hitLoc );
	}
}

/*
============
CanDamage

Returns the fraction of the target an explosion at `origin` can see.  A client
is sampled at five points spread across its body, so it can be partly covered;
anything else keeps Q3's all-or-nothing answer.
============
*/
float CanDamage( gentity_t *targ, const vec3_t origin ) {
	vec3_t dest[5];
	vec3_t midpoint;
	vec3_t eyePos;
	vec3_t dir, perp;
	trace_t tr;
	float halfHeight;
	int hits;
	int i;

	if ( targ->client ) {
		VectorCopy( targ->r.currentOrigin, eyePos );
		eyePos[2] += targ->client->ps.viewHeightCurrent;
		AddLeanToPosition( eyePos, targ->client->ps.viewangles[YAW], targ->client->ps.leanf,
						   16.0, 20.0 );

		halfHeight = ( eyePos[2] - targ->r.currentOrigin[2] ) * 0.5f;

		dir[0] = origin[0] - targ->r.currentOrigin[0];
		dir[1] = origin[1] - targ->r.currentOrigin[1];
		dir[2] = 0;
		VectorNormalize( dir );

		perp[0] = -dir[1];
		perp[1] = dir[0];
		perp[2] = dir[2];

		midpoint[0] = ( eyePos[0] + targ->r.currentOrigin[0] ) * 0.5f;
		midpoint[1] = ( eyePos[1] + targ->r.currentOrigin[1] ) * 0.5f;
		midpoint[2] = ( eyePos[2] + targ->r.currentOrigin[2] ) * 0.5f;

		VectorCopy( midpoint, dest[0] );

		VectorMA( midpoint, 15.0f, perp, dest[1] );
		dest[1][2] += halfHeight;
		VectorMA( midpoint, 15.0f, perp, dest[2] );
		dest[2][2] -= halfHeight;
		VectorMA( midpoint, -15.0f, perp, dest[3] );
		dest[3][2] += halfHeight;
		VectorMA( midpoint, -15.0f, perp, dest[4] );
		dest[4][2] -= halfHeight;

		hits = 0;
		for ( i = 0 ; i < 5 ; i++ ) {
			trap_LocationalTrace( &tr, dest[i], origin, targ->s.number, MASK_CANDAMAGE,
								  bulletPriorityMap );
			if ( tr.fraction == 1.0 ) {
				hits++;
			}
		}

		if ( !hits ) {
			return 0;
		}
		if ( hits <= 3 ) {
			return hits * 0.33333334f;
		}
		return 1.0;
	}

	midpoint[0] = ( targ->r.absmin[0] + targ->r.absmax[0] ) * 0.5f;
	midpoint[1] = ( targ->r.absmin[1] + targ->r.absmax[1] ) * 0.5f;
	midpoint[2] = ( targ->r.absmin[2] + targ->r.absmax[2] ) * 0.5f;

	VectorCopy( midpoint, dest[0] );
	VectorSet( dest[1], midpoint[0] + 15.0f, midpoint[1] + 15.0f, midpoint[2] );
	VectorSet( dest[2], midpoint[0] + 15.0f, midpoint[1] - 15.0f, midpoint[2] );
	VectorSet( dest[3], midpoint[0] - 15.0f, midpoint[1] + 15.0f, midpoint[2] );
	VectorSet( dest[4], midpoint[0] - 15.0f, midpoint[1] - 15.0f, midpoint[2] );

	for ( i = 0 ; i < 5 ; i++ ) {
		trap_LocationalTrace( &tr, dest[i], origin, targ->s.number, MASK_CANDAMAGE,
							  bulletPriorityMap );
		if ( tr.fraction == 1.0 ) {
			return 1.0;
		}
	}

	return 0;
}

/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage( const vec3_t origin, gentity_t *inflictor, gentity_t *attacker,
						 float damage, float minDamage, float radius, gentity_t *ignore,
						 int mod ) {
	float points, dist;
	gentity_t   *ent;
	int entityList[MAX_GENTITIES];
	int numListedEntities;
	vec3_t mins, maxs;
	vec3_t v;
	vec3_t dir;
	vec3_t midpoint;
	trace_t tr;
	qboolean hitClient;
	float visible;
	int i, e;

	hitClient = qfalse;

	if ( !attacker ) {
		return qfalse;
	}

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius * 1.4142135f;
		maxs[i] = origin[i] + radius * 1.4142135f;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES, MASK_ALL );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[e]];

		if ( ent == ignore ) {
			continue;
		}
		if ( !ent->takedamage ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		if ( !ent->r.bmodel ) {
			VectorSubtract( ent->r.currentOrigin, origin, v );
		} else {
			for ( i = 0 ; i < 3 ; i++ ) {
				if ( origin[i] < ent->r.absmin[i] ) {
					v[i] = ent->r.absmin[i] - origin[i];
				} else if ( origin[i] > ent->r.absmax[i] ) {
					v[i] = origin[i] - ent->r.absmax[i];
				} else {
					v[i] = 0;
				}
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}
		if ( ent->client && level.radiusDamageIgnorePlayers ) {
			continue;
		}

		points = minDamage + ( 1.0f - dist / radius ) * ( damage - minDamage );

		visible = CanDamage( ent, origin );
		if ( visible > 0 ) {
			if ( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract( ent->r.currentOrigin, origin, dir );
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage( ent, inflictor, attacker, dir, origin, (int)( points * visible ),
					  DAMAGE_RADIUS, mod, 0 );
			continue;
		}

		// no line of sight to the body: let a tenth of the damage through thin
		// geometry if the box centre is close enough
		midpoint[0] = ( ent->r.absmin[0] + ent->r.absmax[0] ) * 0.5;
		midpoint[1] = ( ent->r.absmin[1] + ent->r.absmax[1] ) * 0.5;
		midpoint[2] = ( ent->r.absmin[2] + ent->r.absmax[2] ) * 0.5;

		trap_Trace( &tr, origin, vec3_origin, vec3_origin, midpoint, ENTITYNUM_NONE,
					MASK_GRENADE_TRACE );
		if ( tr.fraction >= 1.0 ) {
			continue;
		}

		VectorSubtract( midpoint, origin, v );
		if ( VectorLength( v ) >= radius * 0.2f ) {
			continue;
		}

		if ( LogAccuracyHit( ent, attacker ) ) {
			hitClient = qtrue;
		}
		VectorSubtract( ent->r.currentOrigin, origin, dir );
		dir[2] += 24;
		G_Damage( ent, inflictor, attacker, dir, origin, (int)( points * 0.1f ),
				  DAMAGE_RADIUS, mod, 0 );
	}

	return hitClient;
}

/*
==================
G_GetHitLocationString
==================
*/
unsigned short G_GetHitLocationString( int hitLoc ) {
	return hitLocationConstStrings[hitLoc];
}

/*
==================
G_GetHitLocationIndexFromString
==================
*/
int G_GetHitLocationIndexFromString( unsigned short name ) {
	int i;

	for ( i = 0 ; i < HITLOC_NUM ; i++ ) {
		if ( hitLocationConstStrings[i] == name ) {
			return i;
		}
	}

	return 0;
}
