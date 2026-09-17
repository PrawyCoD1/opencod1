/*
 * g_weapon_mp.c -- the multiplayer game module's weapon fire path.
 *
 * Very little of RTCW's game/g_weapon.c survives: SnapVectorTowards,
 * LogAccuracyHit and CalcMuzzlePoint are recognisable, and everything else was
 * rebuilt around weaponInfo_t.  There is no per-weapon fire function any more
 * -- FireWeapon reads weaponType and branches to one of three, and a bullet is
 * a single recursive Bullet_Fire_Extended that carries its own penetration and
 * (for a rifle round) pass-through-the-body damage.
 *
 * Function order is binary order (0x2003C7E0 .. 0x2003D2E0).
 *
 * @fidelity: likely
 */

#include "g_local.h"
#include "bg_public.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Declarations that belong in a shared header. */

#define ENTITYNUM_WORLD             ( MAX_GENTITIES - 2 )

#define YAW                         1
#define ROLL                        2

/* The two mounted-turret bits of playerState_t.eFlags (FireWeapon 0x2003D164). */
#define EF_TURRET_ACTIVE            0x0000C000

#define SURF_NOIMPACT               0x00000010
#define SURF_SKY                    0x00000004
#define SURF_TYPE_SHIFT             20
#define SURF_TYPE_MASK              0x1F

/* trace_t.contents bit that makes a bullet continue past what it hit.  The
 * original name is not recovered, so this one is inferred
 * (Bullet_Fire_Extended 0x2003CD06). */
#define CONTENTS_PENETRATE          0x00000010

#define MASK_SHOT                   0x02802031      /* 0x2003CADF, 0x2003C7FF */

/* The literals G_AddEvent / G_TempEntity is handed; the names are the
 * matching entries of bg_misc.c's eventnames[] (0x2006C8B8). */
#define EV_MELEE_HIT                166             /* 0x2003C8A9, a client */
#define EV_MELEE_MISS               167             /* 0x2003C8B0 */
#define EV_BULLET_HIT_SMALL         173             /* 0x2003CCB5 */
#define EV_BULLET_HIT_LARGE         174
#define EV_RAILTRAIL                185             /* g_debugBullets only */

/* forward: q_shared.c / q_math.c */
byte            DirToByte( const vec3_t dir );
void            AddLeanToPosition( vec3_t pos, float yaw, float leanFrac,
								   float rightScale, float upScale );
void            gunrandom( float *right, float *up );

/* forward: g_main_mp.c */
void            Com_DPrintf( const char *fmt, ... );
extern vmCvar_t g_debugBullets;                 /* 0x20234B20 */

/* forward: g_team_mp.c */
qboolean        OnSameTeam( gentity_t *ent1, gentity_t *ent2 );

/* forward: g_trigger_mp.c */
void            G_CheckHitTriggerDamage( gentity_t *ent, const vec3_t start, const vec3_t end,
										 int damage, int mod );

/*
 * The hit-location priority maps trap_LocationalTrace picks its winning DObj
 * part with.  One entry per hit location; the rifle map weights the head and
 * torso far above the limbs.  The names are inferred -- the tables are
 * unnamed data at 0x2006A4E4 and 0x2006A4F8.
 */
const byte bulletPriorityMap[HITLOC_NUM] = {
	1,
	3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3,
	0
};

const byte rifleBulletPriorityMap[HITLOC_NUM] = {
	1,
	9, 9, 9, 8, 7, 6, 6, 6, 6,
	5, 5, 4, 4, 4, 4, 3, 3,
	0
};

/*
======================
Weapon_Melee
======================
*/
void Weapon_Melee( weaponFireInfo_t *fireInfo, gentity_t *self ) {
	vec3_t end;
	trace_t tr;
	gentity_t   *tent;
	gentity_t   *traceEnt;
	int damage;

	VectorMA( fireInfo->start, 64.0f, fireInfo->forward, end );
	trap_LocationalTrace( &tr, fireInfo->start, end, self->s.number, MASK_SHOT,
						  bulletPriorityMap );

	damage = bg_weaponInfo[self->s.weapon]->meleeDamage;
	G_CheckHitTriggerDamage( self, fireInfo->start, tr.endpos, damage, MOD_MELEE );

	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}
	if ( tr.fraction == 1.0f ) {
		return;
	}

	traceEnt = &g_entities[tr.entityNum];

	if ( traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MELEE_HIT );
	} else {
		tent = G_TempEntity( tr.endpos, EV_MELEE_MISS );
	}
	tent->s.otherEntityNum = tr.entityNum;
	tent->s.eventParm = DirToByte( tr.normal );
	tent->s.weapon = self->s.weapon;

	if ( tr.entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( !traceEnt->takedamage ) {
		return;
	}

	G_Damage( traceEnt, self, self, fireInfo->forward, tr.endpos, damage + rand() % 5,
			  0, MOD_MELEE, tr.partGroup );
}

/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network transmission, but make
sure that it rounds towards a given point rather than blindly truncating.
This prevents it from truncating into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, const vec3_t to ) {
	int i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = floor( v[i] );
		} else {
			v[i] = ceil( v[i] );
		}
	}
}

/*
======================
Bullet_Endpos

Pushes the aim point out to the far clip and scatters it inside the cone the
current spread angle describes.
======================
*/
void Bullet_Endpos( const weaponFireInfo_t *fireInfo, vec3_t end, float spread ) {
	float r, u;
	float spreadDist;

	spreadDist = (float)tan( spread * M_PI / 180.0f ) * 8192.0f;
	gunrandom( &r, &u );
	r = r * spreadDist;
	u = u * spreadDist;

	end[0] = fireInfo->forward[0] * 8192.0f + fireInfo->start[0];
	end[1] = fireInfo->forward[1] * 8192.0f + fireInfo->start[1];
	end[2] = fireInfo->forward[2] * 8192.0f + fireInfo->start[2];

	end[0] = r * fireInfo->right[0] + end[0];
	end[1] = r * fireInfo->right[1] + end[1];
	end[2] = r * fireInfo->right[2] + end[2];

	end[0] = u * fireInfo->up[0] + end[0];
	end[1] = u * fireInfo->up[1] + end[1];
	end[2] = u * fireInfo->up[2] + end[2];
}

/*
======================
Bullet_Fire

No caller survives in 1.1; FireWeapon builds the endpoint itself.
======================
*/
void Bullet_Fire( weaponFireInfo_t *fireInfo, gentity_t *self, gentity_t *attacker,
				  float spread, int damage ) {
	vec3_t end;

	Bullet_Endpos( fireInfo, end, spread );
	Bullet_Fire_Extended( self, attacker, fireInfo->start, end, damage, 0, fireInfo, self );
}

/*
======================
Bullet_Fire_Extended

`depth` counts the recursions: a bullet re-enters here once for every surface
it penetrates and, if it is a rifle round, once more after it has passed
through a body at half damage.
======================
*/
void Bullet_Fire_Extended( gentity_t *passEnt, gentity_t *attacker, vec3_t start, vec3_t end,
						   int damage, int depth, const weaponFireInfo_t *fireInfo,
						   gentity_t *owner ) {
	trace_t tr;
	gentity_t   *tent;
	gentity_t   *traceEnt;
	vec3_t dir;
	vec3_t boxMin, boxMax;
	const byte  *priorityMap;
	float dot, scale;
	int mod;
	int dflags;

	dflags = 0;

	if ( depth > 12 ) {
		Com_DPrintf( "Bullet_Fire_Extended: Too many resursions, bullet aborted\n" );
		return;
	}

	if ( fireInfo->weapInfo->rifleBullet ) {
		mod = MOD_RIFLE_BULLET;
		dflags = DAMAGE_PASSTHRU;
		priorityMap = rifleBulletPriorityMap;
	} else {
		mod = MOD_PISTOL_BULLET;
		priorityMap = bulletPriorityMap;
	}

	trap_LocationalTrace( &tr, start, end, passEnt->s.number, MASK_SHOT, priorityMap );

	if ( g_debugBullets.integer & 1 ) {
		tent = G_TempEntity( start, EV_RAILTRAIL );
		VectorCopy( tr.endpos, tent->s.origin2 );
		tent->s.attackerEntityNum = attacker->s.number;
	}

	G_CheckHitTriggerDamage( attacker, start, tr.endpos, damage, mod );

	traceEnt = &g_entities[tr.entityNum];

	if ( g_debugBullets.integer <= -2 ) {
		VectorAdd( traceEnt->r.currentOrigin, traceEnt->r.mins, boxMin );
		VectorAdd( traceEnt->r.currentOrigin, traceEnt->r.maxs, boxMax );
		tent = G_TempEntity( boxMin, EV_RAILTRAIL );
		VectorCopy( boxMax, tent->s.origin2 );
		tent->s.dmgFlags = 2;
	}

	// the impact effect is drawn along the reflected shot direction
	VectorSubtract( end, start, dir );
	VectorNormalize( dir );
	dot = DotProduct( dir, tr.normal ) * -2.0f;
	dir[0] = tr.normal[0] * dot + dir[0];
	dir[1] = tr.normal[1] * dot + dir[1];
	dir[2] = tr.normal[2] * dot + dir[2];

	if ( !( tr.surfaceFlags & SURF_SKY ) && !traceEnt->client ) {
		if ( fireInfo->weapInfo->rifleBullet ) {
			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_LARGE );
		} else {
			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_SMALL );
		}
		tent->s.eventParm = DirToByte( tr.normal ) & 0xFF;
		tent->s.scale = DirToByte( dir ) & 0xFF;
		tent->s.surfType = ( tr.surfaceFlags >> SURF_TYPE_SHIFT ) & SURF_TYPE_MASK;
		tent->s.otherEntityNum = owner->s.number;
	}

	if ( tr.contents & CONTENTS_PENETRATE ) {
		// come out the far side, further along the more oblique the hit was
		VectorSubtract( end, start, dir );
		VectorNormalize( dir );
		dot = -DotProduct( dir, tr.normal );
		if ( dot < 0.125f ) {
			scale = 0;
		} else {
			scale = 0.25f / dot;
		}
		start[0] = dir[0] * scale + tr.endpos[0];
		start[1] = dir[1] * scale + tr.endpos[1];
		start[2] = dir[2] * scale + tr.endpos[2];
		Bullet_Fire_Extended( passEnt, attacker, start, end, damage, depth + 1, fireInfo, owner );
		return;
	}

	if ( !traceEnt->takedamage ) {
		return;
	}

	G_Damage( traceEnt, attacker, attacker, fireInfo->forward, tr.endpos, damage, dflags,
			  mod, tr.partGroup );

	// a rifle round carries half its damage out the back of a body
	if ( traceEnt->client && ( dflags & DAMAGE_PASSTHRU ) && damage / 2 > 0 ) {
		Bullet_Fire_Extended( traceEnt, attacker, tr.endpos, end, damage / 2, depth + 1,
							  fireInfo, owner );
	}
}

/*
======================
weapon_grenadelauncher_fire
======================
*/
gentity_t *weapon_grenadelauncher_fire( const weaponFireInfo_t *fireInfo, gentity_t *self,
										int weapon ) {
	gentity_t   *grenade;
	vec3_t dir;
	float dot;

	dir[0] = fireInfo->weapInfo->projectileSpeed * fireInfo->forward[0];
	dir[1] = fireInfo->weapInfo->projectileSpeed * fireInfo->forward[1];
	dir[2] = fireInfo->weapInfo->projectileSpeed * fireInfo->forward[2]
			 + fireInfo->weapInfo->projectileSpeedUp;

	grenade = fire_grenade( self, fireInfo->start, dir, weapon );

	// add whatever the thrower's own velocity contributes along the throw
	VectorNormalize( dir );
	dot = DotProduct( dir, self->client->ps.velocity );
	grenade->s.pos.trDelta[0] = dir[0] * dot + grenade->s.pos.trDelta[0];
	grenade->s.pos.trDelta[1] = dir[1] * dot + grenade->s.pos.trDelta[1];
	grenade->s.pos.trDelta[2] = dot * dir[2] + grenade->s.pos.trDelta[2];

	return grenade;
}

/*
======================
Weapon_RocketLauncher_Fire
======================
*/
void Weapon_RocketLauncher_Fire( gentity_t *ent, const weaponFireInfo_t *fireInfo, float spread ) {
	vec3_t dir;
	vec3_t start;
	float r, u;
	float spreadDist;

	spreadDist = (float)tan( spread * M_PI / 180.0f ) * 16.0f;
	gunrandom( &r, &u );
	r = r * spreadDist;
	u = u * spreadDist;

	dir[0] = fireInfo->forward[0] * 16.0f;
	dir[1] = fireInfo->forward[1] * 16.0f;
	dir[2] = fireInfo->forward[2] * 16.0f;

	dir[0] = r * fireInfo->right[0] + dir[0];
	dir[1] = r * fireInfo->right[1] + dir[1];
	dir[2] = r * fireInfo->right[2] + dir[2];

	dir[0] = u * fireInfo->up[0] + dir[0];
	dir[1] = u * fireInfo->up[1] + dir[1];
	dir[2] = u * fireInfo->up[2] + dir[2];

	VectorNormalize( dir );

	VectorCopy( fireInfo->start, start );
	fire_rocket( ent, start, dir );

	if ( ent->client ) {
		ent->client->ps.velocity[0] -= fireInfo->forward[0] * 64.0f;
		ent->client->ps.velocity[1] -= fireInfo->forward[1] * 64.0f;
		ent->client->ps.velocity[2] -= fireInfo->forward[2] * 64.0f;
	}
}

/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if ( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if ( !target->client ) {
		return qfalse;
	}
	if ( !attacker->client ) {
		return qfalse;
	}

	if ( target->client->ps.pm_type >= PM_DEAD ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CalcMuzzlePoint

Set muzzle location relative to pivoting eye.
===============
*/
void CalcMuzzlePoint( gentity_t *ent, vec3_t muzzlePoint ) {
	VectorCopy( ent->r.currentOrigin, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewHeightCurrent;
	AddLeanToPosition( muzzlePoint, ent->client->ps.viewangles[YAW], ent->client->ps.leanf,
					   16.0, 20.0 );

	// snap to integer coordinates for more efficient network bandwidth usage
	muzzlePoint[0] = (int)muzzlePoint[0];
	muzzlePoint[1] = (int)muzzlePoint[1];
	muzzlePoint[2] = (int)muzzlePoint[2];
}

/*
===============
CalcMuzzlePoints
===============
*/
void CalcMuzzlePoints( weaponFireInfo_t *fireInfo, gentity_t *ent ) {
	vec3_t angles;

	angles[0] = *(float *)&ent->client->unknown_0x220C[0];
	angles[1] = *(float *)&ent->client->unknown_0x220C[4];
	angles[2] = ent->client->ps.viewangles[ROLL];

	AngleVectors( angles, fireInfo->forward, fireInfo->right, fireInfo->up );
	CalcMuzzlePoint( ent, fireInfo->start );
}

/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {
	weaponFireInfo_t fireInfo;
	vec3_t angles;
	vec3_t end;
	float spread;
	float minSpread;

	if ( ( ent->client->ps.eFlags & EF_TURRET_ACTIVE ) && ent->active ) {
		return;
	}

	fireInfo.weapInfo = bg_weaponInfo[ent->s.weapon];

	angles[0] = *(float *)&ent->client->unknown_0x220C[0];
	angles[1] = *(float *)&ent->client->unknown_0x220C[4];
	angles[2] = ent->client->ps.viewangles[ROLL];

	AngleVectors( angles, fireInfo.forward, fireInfo.right, fireInfo.up );
	CalcMuzzlePoint( ent, fireInfo.start );

	if ( ent->client->ps.fWeaponPosFrac == 1.0f ) {
		spread = ( fireInfo.weapInfo->hipSpreadMax - fireInfo.weapInfo->adsSpread )
				 * *(float *)&ent->client->unknown_0x223C[4] + fireInfo.weapInfo->adsSpread;
	} else {
		minSpread = BG_GetMinSpreadForWeapon( level.time, &ent->client->ps, ent->s.weapon );
		spread = ( fireInfo.weapInfo->hipSpreadMax - minSpread )
				 * *(float *)&ent->client->unknown_0x223C[4] + minSpread;
	}

	if ( !fireInfo.weapInfo->weaponType ) {
		Bullet_Endpos( &fireInfo, end, spread );
		Bullet_Fire_Extended( ent, ent, fireInfo.start, end, fireInfo.weapInfo->damage, 0,
							  &fireInfo, ent );
	} else if ( fireInfo.weapInfo->weaponType == 1 ) {
		weapon_grenadelauncher_fire( &fireInfo, ent, ent->s.weapon );
	} else if ( fireInfo.weapInfo->weaponType == 2 ) {
		Weapon_RocketLauncher_Fire( ent, &fireInfo, spread );
	} else {
		G_Error( "Unknown weapon type %i for %s\n", fireInfo.weapInfo->weaponType,
				 fireInfo.weapInfo->szInternalName );
	}
}

/*
===============
FireWeaponMelee
===============
*/
void FireWeaponMelee( gentity_t *ent ) {
	weaponFireInfo_t fireInfo;

	if ( ( ent->client->ps.eFlags & EF_TURRET_ACTIVE ) && ent->active ) {
		return;
	}

	fireInfo.weapInfo = bg_weaponInfo[ent->s.weapon];

	AngleVectors( ent->client->ps.viewangles, fireInfo.forward, fireInfo.right, fireInfo.up );
	CalcMuzzlePoint( ent, fireInfo.start );

	Weapon_Melee( &fireInfo, ent );
}
