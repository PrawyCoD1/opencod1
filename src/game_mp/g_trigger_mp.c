/*
 * g_trigger_mp.c -- the trigger entities.
 *
 * Call of Duty 1.1 multiplayer (game_mp_x86.dll, 0x20039B30 .. 0x2003A7F8).
 * RTCW's game/g_trigger.c is the ancestor and trigger_multiple, trigger_once
 * and trigger_hurt survive nearly unchanged.  What CoD replaced is the firing
 * side: there is no G_UseTargets any more.  A trigger tells its script object
 * instead, through G_Trigger, which queues the trigger/toucher pair in
 * level.notifyWatch for G_RunFrame to drain.  trigger_damage, trigger_lookat
 * and the AI-less Touch_Multi are CoD's own; RTCW's push/teleport/timer/
 * aidoor/gas/flagonly triggers are gone.
 *
 * Function order is binary order.
 *
 * @fidelity: likely
 */

#include <stdlib.h>

#include "g_local.h"

/* Contents bits.  0x405C0008 is the value InitTrigger writes and is United
 * Offensive's MASK_TRIGGER; 0x40000000 is the "a client touching me counts"
 * bit InitSentientTrigger sets on its own.  The three spawnflag-selected bits
 * (0x40000/0x80000/0x100000) and 0x400000 have no recovered name. */
#define CONTENTS_TRIGGER            0x40000000
#define CONTENTS_TRIGGER_DAMAGE     0x00400000
#define MASK_TRIGGER                0x405C0008

#define SVF_NOCLIENT                1

/* level.time is advanced in 100 msec steps (multi_trigger 0x20039D24). */
#define FRAMETIME                   100

/* q_shared.h carries these in the original tree.  CoD divides by 32768, not
 * Q3's 32767 (the multiplier at 0x2005FB04 is exactly 1/32768). */
#define random()                    ( (float)rand() * ( 1.0f / 32768.0f ) )
#define crandom()                   ( 2.0f * random() - 1.0f )
#define VectorCompare( a,b )        ( (a)[0]==(b)[0] && (a)[1]==(b)[1] && (a)[2]==(b)[2] )

/* trigger_damage keeps a fixed pool of "health" and counts damage down from
 * it; every activation path resets it to this. */
#define TRIGGER_DAMAGE_HEALTH       32000

/* forward: universal/com_math.c */
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );


/*
==============
G_Trigger

Tell the script that `self` was triggered by `other`.  Normally the pair is
just queued -- G_RunFrame drains level.notifyWatch once the touch loop is done
walking the area links -- but when the queue is full the notify is issued on
the spot instead.
==============
*/
void G_Trigger( gentity_t *self, gentity_t *other ) {
	notifyWatch_t *watch;

	if ( !Scr_IsSystemActive( 1 ) ) {
		return;
	}

	if ( level.numNotifyWatches == MAX_NOTIFY_WATCHES ) {
		Scr_AddEntityNum( other->s.number, 0 );
		Scr_NotifyNum( self->s.number, 0, scr_const.trigger, 1 );
		return;
	}

	watch = &level.notifyWatch[level.numNotifyWatches++];
	watch->entNum = (unsigned short)self->s.number;
	watch->otherEntNum = (unsigned short)other->s.number;
	watch->spawnCount = self->spawnCount;
	watch->otherSpawnCount = other->spawnCount;
}


void InitTrigger( gentity_t *self ) {
	if ( !VectorCompare( self->r.currentAngles, vec3_origin ) ) {
		G_SetMovedir( self->r.currentAngles, self->movedir );
	}

	trap_SetBrushModel( self );

	self->s.eFlags |= 2;
	self->r.contents = MASK_TRIGGER;        /* replaces what trap_SetBrushModel left */
	self->r.svFlags = SVF_NOCLIENT;
}


/*
==============
InitSentientTrigger

Narrows a trigger's contents to the kinds of toucher its spawnflags name.
Exists standalone in the binary but LTCG inlined every call, so nothing
references it.
==============
*/
void InitSentientTrigger( gentity_t *self ) {
	self->r.contents = 0;

	if ( !( self->spawnflags & 8 ) ) {
		self->r.contents = CONTENTS_TRIGGER;
	}
	if ( self->spawnflags & 1 ) {
		self->r.contents |= 0x40000;
	}
	if ( self->spawnflags & 2 ) {
		self->r.contents |= 0x80000;
	}
	if ( self->spawnflags & 4 ) {
		self->r.contents |= 0x100000;
	}
}


// the wait time has passed, so set back up for another activation
void multi_wait( gentity_t *ent ) {
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger( gentity_t *ent, gentity_t *activator ) {
	ent->activator = activator;

	if ( ent->think == Think_GeneralLink || ent->nextthink ) {
		return;     // can't retrigger until the wait is over
	}

	if ( ent->wait > 0 ) {
		ent->think = multi_wait;
		ent->nextthink = level.time + (int)( ( ent->wait + ent->random * crandom() ) * 1000.0f );
	} else {
		// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = 0;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEntity;
	}
}


void Use_Multi( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	multi_trigger( ent, activator );
}


void Touch_Multi( gentity_t *self, gentity_t *other, int touchMode ) {
	G_Trigger( self, other );
	multi_trigger( self, other );
}


/*QUAKED trigger_multiple (.5 .5 .5) ?
"wait" : Seconds between triggerings, 0.5 default, -1 = one time only.
"random"	wait variance, default is 0
Variable sized repeatable trigger.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)
*/
void SP_trigger_multiple( gentity_t *ent ) {
	G_SpawnFloat( "wait", "0.5", &ent->wait );
	G_SpawnFloat( "random", "0", &ent->random );

	if ( ent->random >= ent->wait && ent->wait >= 0 ) {
		ent->random = ent->wait - FRAMETIME;
		G_Printf( "trigger_multiple has random >= wait\n" );
	}

	ent->touch = Touch_Multi;
	ent->use = Use_Multi;

	InitTrigger( ent );
	InitSentientTrigger( ent );
	trap_LinkEntity( ent );
}


/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF - SILENT NO_PROTECTION SLOW ONCE
Any entity that touches this will be hurt.
It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

"life"	time this brush will exist if value is zero will live for ever ei 0.5 sec 2.sec
default is zero

the entity must be used first before it will count down its life
*/
void hurt_touch( gentity_t *self, gentity_t *other, int touchMode ) {
	int dflags;

	if ( !other->takedamage ) {
		return;
	}

	if ( self->timestamp > level.time ) {
		return;
	}

	G_Trigger( self, other );

	if ( self->spawnflags & 16 ) {
		self->timestamp = level.time + 1000;
	} else {
		self->timestamp = level.time + FRAMETIME;
	}

	// play sound
	if ( !( self->spawnflags & 4 ) ) {
		G_PlaySoundAliasAtPoint( other->r.currentOrigin, self->noise_index );
	}

	/* the bit is 0x10 -- (spawnflags & 8) << 1 at 0x20039F13 -- which is the
	   slot g_local.h spells DAMAGE_NO_TEAM_PROTECTION; RTCW's line here reads
	   DAMAGE_NO_PROTECTION. */
	if ( self->spawnflags & 8 ) {
		dflags = DAMAGE_NO_TEAM_PROTECTION;
	} else {
		dflags = 0;
	}
	G_Damage( other, self, self, NULL, NULL, self->damage, dflags, MOD_TRIGGER_HURT, 0 );

	if ( self->spawnflags & 32 ) {
		self->touch = NULL;
	}
}


void hurt_think( gentity_t *ent ) {
	ent->nextthink = level.time + FRAMETIME;

	if ( ent->wait < level.time ) {
		G_FreeEntity( ent );
	}
}


void hurt_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( self->touch ) {
		self->touch = NULL;
	} else {
		self->touch = hurt_touch;
	}

	if ( self->delay ) {
		self->nextthink = level.time + 50;
		self->think = hurt_think;
		self->wait = level.time + ( self->delay * 1000 );
	}
}


/*
==============
SP_trigger_hurt
==============
*/
void SP_trigger_hurt( gentity_t *self ) {
	char    *life, *sound;

	InitTrigger( self );

	G_SpawnString( "sound", "world_hurt_me", &sound );

	self->noise_index = G_SoundAliasIndex( sound );

	if ( !self->damage ) {
		self->damage = 5;
	}

	self->r.contents = MASK_TRIGGER;

	self->use = hurt_use;

	// link in to the world if starting active
	if ( !( self->spawnflags & 1 ) ) {
		self->touch = hurt_touch;
	}

	G_SpawnString( "life", "0", &life );
	self->delay = (float)atof( life );
}


/*QUAKED trigger_once (.5 .5 .5) ?
Once triggered, this entity is destroyed
(you can actually do the same thing with trigger_multiple with a wait of -1)
*/
void SP_trigger_once( gentity_t *ent ) {
	ent->wait   = -1;           // this will remove itself after one use
	ent->touch  = Touch_Multi;
	ent->use    = Use_Multi;

	InitTrigger( ent );
	InitSentientTrigger( ent );
	trap_LinkEntity( ent );
}


/*
==============================================================================

trigger_damage

A brush that answers to weapon damage rather than to touch.  Its spawnflags
name the means of death it will NOT respond to.

==============================================================================
*/

/*
==============
Respond_trigger_damage
==============
*/
qboolean Respond_trigger_damage( gentity_t *ent, int mod ) {
	if ( ( ent->spawnflags & 1 ) && mod == MOD_PISTOL_BULLET ) {
		return qfalse;
	}
	if ( ( ent->spawnflags & 2 ) && mod == MOD_RIFLE_BULLET ) {
		return qfalse;
	}
	if ( ( ent->spawnflags & 4 ) && mod >= MOD_GRENADE && mod <= MOD_PROJECTILE_SPLASH ) {
		return qfalse;
	}
	if ( ent->spawnflags & 8 ) {
		switch ( mod ) {
		case MOD_GRENADE:
		case MOD_GRENADE_SPLASH:
		case MOD_PROJECTILE:
		case MOD_PROJECTILE_SPLASH:
		case MOD_MORTAR:
		case MOD_MORTAR_SPLASH:
		case MOD_DYNAMITE:
		case MOD_DYNAMITE_SPLASH:
		case MOD_AIRSTRIKE:
		case MOD_EXPLOSIVE:
			return qfalse;
		}
	}
	if ( ent->spawnflags & 16 ) {
		switch ( mod ) {
		case MOD_GRENADE_SPLASH:
		case MOD_PROJECTILE_SPLASH:
		case MOD_MORTAR_SPLASH:
		case MOD_DYNAMITE_SPLASH:
			return qfalse;
		}
	}
	if ( ( ent->spawnflags & 32 ) && mod == MOD_MELEE ) {
		return qfalse;
	}
	if ( ( ent->spawnflags & 64 ) && mod == MOD_LAVA ) {
		return qfalse;
	}
	if ( ent->spawnflags & 256 ) {
		switch ( mod ) {
		case MOD_UNKNOWN:
		case MOD_KICKED:
		case MOD_GRABBER:
		case MOD_WATER:
		case MOD_SLIME:
		case MOD_CRUSH:
		case MOD_TELEFRAG:
		case MOD_FALLING:
		case MOD_SUICIDE:
		case MOD_TRIGGER_HURT:
			return qfalse;
		}
	}

	return qtrue;
}


/*
==============
Activate_trigger_damage

`mod` of -1 is the explicit "use" path: it activates the trigger without
notifying the script.
==============
*/
void Activate_trigger_damage( gentity_t *ent, gentity_t *other, int damage, int mod ) {
	if ( ent->nextthink && ent->think != Think_GeneralLink ) {
		return;
	}

	if ( ent->key > 0 && damage < ent->key ) {
		return;         // below the "threshold" this brush answers to
	}

	if ( !Respond_trigger_damage( ent, mod ) ) {
		return;
	}

	if ( ent->count && TRIGGER_DAMAGE_HEALTH - ent->health < ent->count ) {
		return;         // has not accumulated enough damage yet
	}

	ent->activator = other;

	if ( mod != -1 ) {
		G_Trigger( ent, other );
	}

	if ( ent->think != Think_GeneralLink ) {
		if ( ent->wait > 0 ) {
			ent->think = multi_wait;
			ent->nextthink = level.time + (int)( ( ent->wait + ent->random * crandom() ) * 1000.0f );
		} else {
			ent->touch = NULL;
			ent->nextthink = level.time + FRAMETIME;
			ent->think = G_FreeEntity;
		}
	}

	ent->health = TRIGGER_DAMAGE_HEALTH;
}


void Use_trigger_damage( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	Activate_trigger_damage( self, other, self->count + 1, -1 );
}


void Pain_trigger_damage( gentity_t *self, gentity_t *attacker, int damage, const float *point,
						  int mod, const float *dir, int hitLoc ) {
	Activate_trigger_damage( self, attacker, damage, mod );

	if ( !self->count ) {
		self->health = TRIGGER_DAMAGE_HEALTH;
	}
}


void Die_trigger_damage( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage,
						 int mod, int weapon, const float *dir, int hitLoc ) {
	Activate_trigger_damage( self, attacker, damage, mod );

	if ( !self->count ) {
		self->health = TRIGGER_DAMAGE_HEALTH;
	}
}


/*QUAKED trigger_damage (.5 .5 .5) ?
"wait"			seconds between triggerings, 0.5 default
"random"		wait variance, default is 0
"accumulate"	total damage needed before it fires, 0 = fire on any hit
"threshold"	minimum damage of a single hit, 0 = any
*/
void SP_trigger_damage( gentity_t *ent ) {
	char *accumulate, *threshold;

	G_SpawnFloat( "wait", "0.5", &ent->wait );
	G_SpawnFloat( "random", "0", &ent->random );

	if ( ent->random >= ent->wait && ent->wait >= 0 ) {
		ent->random = ent->wait - FRAMETIME;
		G_Printf( "trigger_damage has random >= wait\n" );
	}

	G_SpawnString( "accumulate", "0", &accumulate );
	ent->count = atoi( accumulate );

	G_SpawnString( "threshold", "0", &threshold );
	ent->key = atoi( threshold );

	ent->health = TRIGGER_DAMAGE_HEALTH;
	ent->takedamage = 1;

	ent->use = Use_trigger_damage;
	ent->pain = Pain_trigger_damage;
	ent->die = Die_trigger_damage;

	InitTrigger( ent );
	trap_LinkEntity( ent );
}


/*
==============
G_CheckHitTriggerDamage

Fire every trigger_damage brush the shot passed through.  Bullets and melee
come here; the trace is a sight trace against the brush itself so that a
trigger behind a wall does not answer.
==============
*/
void G_CheckHitTriggerDamage( gentity_t *attacker, const vec3_t start, const vec3_t end,
							  int damage, int mod ) {
	vec3_t mins, maxs;
	int entityList[MAX_GENTITIES];
	int numListedEntities;
	gentity_t *ent;
	int i;

	VectorCopy( start, mins );
	VectorCopy( start, maxs );
	AddPointToBounds( end, mins, maxs );

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES,
											CONTENTS_TRIGGER_DAMAGE );

	for ( i = 0; i < numListedEntities; i++ ) {
		ent = &g_entities[entityList[i]];

		if ( ent->classname != scr_const.trigger_damage ) {
			continue;
		}
		if ( !trap_SightTraceToEntity( start, vec3_origin, vec3_origin, end, ent->s.number, -1 ) ) {
			continue;
		}

		Scr_AddEntityNum( attacker->s.number, 0 );
		Scr_AddInt( damage );
		Scr_NotifyNum( ent->s.number, 0, scr_const.damage, 2 );

		Activate_trigger_damage( ent, attacker, damage, mod );

		if ( !ent->count ) {
			ent->health = TRIGGER_DAMAGE_HEALTH;
		}
	}
}


/*
==============
G_GrenadeTouchTriggerDamage

The same sweep for a grenade that bounced through, but only triggers that
carry the 0x8000 flag bit answer to one.
==============
*/
void G_GrenadeTouchTriggerDamage( gentity_t *attacker, const vec3_t start, const vec3_t end,
								  int damage, int mod ) {
	vec3_t mins, maxs;
	int entityList[MAX_GENTITIES];
	int numListedEntities;
	gentity_t *ent;
	int i;

	VectorCopy( start, mins );
	VectorCopy( start, maxs );
	AddPointToBounds( end, mins, maxs );

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES,
											CONTENTS_TRIGGER_DAMAGE );

	for ( i = 0; i < numListedEntities; i++ ) {
		ent = &g_entities[entityList[i]];

		if ( ent->classname != scr_const.trigger_damage ) {
			continue;
		}
		if ( !( ent->flags & 0x8000 ) ) {
			continue;
		}
		if ( !trap_SightTraceToEntity( start, vec3_origin, vec3_origin, end, ent->s.number, -1 ) ) {
			continue;
		}

		Scr_AddEntityNum( attacker->s.number, 0 );
		Scr_AddInt( damage );
		Scr_NotifyNum( ent->s.number, 0, scr_const.damage, 2 );

		Activate_trigger_damage( ent, attacker, damage, mod );

		if ( !ent->count ) {
			ent->health = TRIGGER_DAMAGE_HEALTH;
		}
	}
}


/*
==============
explosive_indicator_think

RTCW's trigger_objective_info marker.  CoD 1.1 kept the think but no longer
spawns the indicator, so nothing in the module references this.
==============
*/
void explosive_indicator_think( gentity_t *ent ) {
	gentity_t *parent;

	parent = &g_entities[ent->r.ownerNum];

	if ( !parent->inuse || parent->classname != scr_const.trigger_objective_info ) {
		ent->think = G_FreeEntity;
		ent->nextthink = level.time + FRAMETIME;
		return;
	}

	ent->nextthink = level.time + FRAMETIME;
}


/*QUAKED trigger_lookat (.5 .5 .5) ?
A brush the friendly-fire check traces against; G_CheckForPreventFriendlyFire
notifies it when a player aims through it.
*/
void SP_trigger_lookat( gentity_t *ent ) {
	trap_SetBrushModel( ent );

	ent->s.eFlags |= 2;
	ent->r.contents = 0x20000000;
	ent->r.svFlags = SVF_NOCLIENT;

	trap_LinkEntity( ent );
}
