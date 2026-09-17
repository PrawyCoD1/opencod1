/*
 * @fidelity: likely
 *
 * g_items_mp.c -- CoD 1.1 MP's descendant of RTCW's game/g_items.c.
 *
 * Items are any object a player can touch to gain some effect.  Pickup returns
 * the number of seconds until the item should respawn; respawnable items are
 * not removed, only made invisible and untouchable so they can ride movers.
 *
 * CoD kept RTCW's shape and cut every RTCW item type but weapons, ammo and
 * health, so the hardcoded ammoTable/weapBanks tables are gone: the pickup path
 * is driven by weaponInfo_t and the two ammo registries instead.  What is new
 * is the dropped-weapon cue (GetFreeCueSpot), the script `trigger` notify every
 * pickup fires, and the orientation pass that stands a dropped weapon up on the
 * surface it landed on.
 *
 * Function order is binary order (0x20022990 .. 0x20024DB0).
 */

#include <stdlib.h>
#include <string.h>

#include "g_local.h"

#define RESPAWN_AMMO            40

/* playerState_t.stats indices, as g_client_fields_mp.c spells them. */
#define STAT_HEALTH             0
#define STAT_MAX_HEALTH         2

/*
 * bg_itemlist's row count is a link-time constant in the binary:
 * SaveRegisteredItems' loop ends at bg_itemlist + 70 * 48 (0x2016EAB8), and
 * ClearRegisteredItems memsets 1024 bytes of the flag array.
 */
#define BG_NUM_ITEMS            70
#define MAX_ITEMS               256

/* trap_SetConfigstring slot SaveRegisteredItems writes (0x200245B1). */
#define CS_ITEMS                8

/* entityState_t.eType */
#define ET_ITEM                 3

/* trajectory types this unit uses. */
#define TR_STATIONARY           0
#define TR_LINEAR               2
#define TR_GRAVITY              5
#define TR_GRAVITY_PAUSED       8

#define ENTITYNUM_WORLD         1022

/* gentity_t.flags */
#define FL_DROPPED_ITEM         0x00000010
#define FL_NODRAW               0x00001000
#define FL_ITEM_SPAWNED         0x00002000  /* set by G_SpawnItem, never read here */

/* entityShared_t.svFlags */
#define SVF_NOCLIENT            0x00000001
#define SVF_ITEM                0x00000200  /* every item raises it; name inferred */

/* entityState_t.eFlags.  0x10 additionally selects the capsule trace. */
#define EF_ITEM                 0x00000010
#define EF_NODRAW               0x00000100

/* g_spawn "spawnflags" bits G_SpawnItem / FinishSpawningItem read. */
#define ITEM_SUSPENDED          1
#define ITEM_WEAPON_RESPAWN     8

/*
 * Content masks, all emitted as literals.  ITEM_CONTENTS is what an item
 * carries while it is grabbable; RespawnItem restores it without bit 0x100.
 */
#define ITEM_CONTENTS           0x407C0108
#define ITEM_CONTENTS_RESPAWN   0x407C0008
#define MASK_ITEM               0x00000411
#define MASK_ITEM_CLIP          0x00000491
#define MASK_DROPPED_ITEM       0x00000081  /* LaunchItem's clipmask, 0x20023B26 */
#define CONTENTS_NODROP         0x80000000

/* entity_event_t values; bg_misc.c owns the enum, no header carries it yet. */
#define EV_ITEM_PICKUP          146
#define EV_ITEM_PICKUP_QUIET    147
#define EV_AMMO_PICKUP          148
#define EV_ITEM_RESPAWN         197

#define ITEM_RADIUS             15

/* q_shared.h still lacks these; every unit that needs them spells them itself. */
#define PITCH                   0
#define YAW                     1
#define ROLL                    2

/* Seven of this unit's nine rand() sites (Pickup_Weapon 0x20022D32, Touch_Item
   0x200237B6, Drop_Weapon 0x20023EC3..0x20024126) are `fild; fmul [1/32768]`;
   only Drop_Item 0x20023CF7 and G_BounceItem 0x20024B3D carry the (float) cast
   round-trip and a true fdiv [32768.0f], so those two are spelled inline. */
#define random()                ( rand() * ( 1.0f / 32768.0f ) )
#define crandom()               ( 2.0f * random() - 1.0f )

/* level.itemCue is the ring of dropped-item entity numbers GetFreeCueSpot
   walks; gentity_t.clipCount is the rounds a dropped weapon carries in its
   magazine, against ent->count's reserve. */
#define MAX_ITEM_CUE            32

/* forward: bg_slidemove.c / universal */
int Q_ftol( float f );
void AxisToAngles( vec3_t axis[3], vec3_t angles );
void Axis4ToAngles( const float *axis, vec3_t angles );

/* forward: g_main_mp.c */
extern vmCvar_t g_weaponrespawn;                /* 0x20234FA0 */
extern vmCvar_t g_weaponAmmoPools;              /* 0x202358A0 */

qboolean itemRegistered[MAX_ITEMS];             /* 0x2016E9A0 */

/*
==============
Fill_Clip
	push reserve ammo into available space in the clip
==============
*/
void Fill_Clip( playerState_t *ps, int weapon ) {
	int ammoIndex;
	int clipIndex;
	int ammomove;

	ammoIndex = bg_weaponInfo[weapon]->ammoIndex;
	clipIndex = bg_weaponInfo[weapon]->clipIndex;

	if ( weapon <= 0 || weapon > bg_numWeapons ) {
		return;
	}

	ammomove = bg_ammoClipSizes[clipIndex] - ps->ammoclip[clipIndex];

	if ( ammomove > ps->ammo[ammoIndex] ) {
		ammomove = ps->ammo[ammoIndex];
	}

	if ( ammomove ) {
		ps->ammo[ammoIndex] -= ammomove;
		ps->ammoclip[clipIndex] += ammomove;
	}
}

/*
==============
Add_Ammo
	Try to always add ammo here unless you have specific needs.

	fillClip will push the ammo straight through into the clip and leave the
	rest in reserve.  Returns how much of the count actually landed.
==============
*/
int Add_Ammo( gentity_t *ent, int weapon, int count, qboolean fillClip ) {
	int ammoIndex;
	int clipIndex;
	int oldAmmo;
	int oldClip;
	int maxAmmo;
	int overflow;
	qboolean clipOnly;

	ammoIndex = bg_weaponInfo[weapon]->ammoIndex;
	clipIndex = bg_weaponInfo[weapon]->clipIndex;

	oldClip = ent->client->ps.ammoclip[clipIndex];
	oldAmmo = ent->client->ps.ammo[ammoIndex];
	ent->client->ps.ammo[ammoIndex] = oldAmmo + count;

	clipOnly = qfalse;
	if ( bg_weaponInfo[weapon]->clipOnly ) {
		BG_GivePlayerWeapon( &ent->client->ps, weapon );
		clipOnly = qtrue;
	}

	if ( fillClip || clipOnly ) {
		Fill_Clip( &ent->client->ps, weapon );
	}

	if ( clipOnly ) {
		ent->client->ps.ammo[ammoIndex] = 0;
	} else {
		maxAmmo = bg_ammoTypeMax[ammoIndex];
		if ( ent->client->ps.ammo[ammoIndex] > maxAmmo ) {
			ent->client->ps.ammo[ammoIndex] = maxAmmo;
		}
	}

	if ( ent->client->ps.ammoclip[clipIndex] > bg_ammoClipSizes[clipIndex] ) {
		ent->client->ps.ammoclip[clipIndex] = bg_ammoClipSizes[clipIndex];
	}

	if ( bg_weaponInfo[weapon]->sharedAmmoCapIndex >= 0 ) {
		overflow = BG_GetMaxPickupableAmmo( &ent->client->ps, weapon );
		if ( overflow < 0 ) {
			if ( bg_weaponInfo[weapon]->clipOnly ) {
				ent->client->ps.ammoclip[clipIndex] += overflow;
				if ( ent->client->ps.ammoclip[clipIndex] <= 0 ) {
					ent->client->ps.ammoclip[clipIndex] = 0;
					BG_TakePlayerWeapon( &ent->client->ps, weapon );
					return 0;
				}
			} else {
				ent->client->ps.ammo[ammoIndex] += overflow;
				if ( ent->client->ps.ammo[ammoIndex] < 0 ) {
					ent->client->ps.ammo[ammoIndex] = 0;
				}
			}
		}
	}

	return ( ent->client->ps.ammo[ammoIndex] - oldAmmo )
		 + ( ent->client->ps.ammoclip[clipIndex] - oldClip );
}

/*
==============
Pickup_Ammo
==============
*/
int Pickup_Ammo( gentity_t *ent, gentity_t *other ) {
	int quantity;
	int added;
	weaponInfo_t *weapInfo;

	quantity = ent->count;
	if ( !quantity ) {
		quantity = ent->item->quantity;
	}

	added = Add_Ammo( other, ent->item->giTag, quantity, qfalse );
	if ( !added ) {
		return 0;
	}

	weapInfo = bg_weaponInfo[ ent->item->giTag ];
	if ( weapInfo->clipOnly ) {
		trap_SendServerCommand( other - g_entities, 0,
								va( "f \"GAME_PICKUP_CLIPONLY_AMMO\x14%s\"", weapInfo->displayName ) );
	} else {
		trap_SendServerCommand( other - g_entities, 0,
								va( "f \"GAME_PICKUP_AMMO\x14%s\"", weapInfo->displayName ) );
	}

	Scr_AddEntityNum( other->s.number, 0 );
	Scr_NotifyNum( ent->s.number, 0, scr_const.trigger, 1 );

	return RESPAWN_AMMO;
}

/*
==============
Pickup_Weapon
	`event` is Touch_Item's pickup event; it is raised to EV_AMMO_PICKUP when
	the player already owns the weapon and only the ammo is taken.
==============
*/
int Pickup_Weapon( gentity_t *ent, gentity_t *other, int *event, int touchMode ) {
	int weapon;
	weaponInfo_t *weapInfo;
	gclient_t *client;
	gentity_t *dropped;
	int reserveCount;
	int clipCount;
	int clipIndex;
	int clipSize;
	int currentWeapon;
	int currentSlot;
	int slot;
	int min;
	int max;
	int swap;
	int added;
	int total;
	vec3_t origin;

	weapon = ent->item->giTag;
	weapInfo = bg_weaponInfo[weapon];
	dropped = NULL;

	// how much reserve ammo the pickup carries
	if ( ent->count < 0 ) {
		reserveCount = 0;
	} else {
		if ( !ent->count ) {
			min = weapInfo->dropAmmoMin;
			max = weapInfo->dropAmmoMax;
			if ( max < min ) {
				swap = min;
				min = max;
				max = swap;
			}
			if ( !max && !min ) {
				ent->count = Q_ftol( ( random() + 1.0f )
									 * ( bg_ammoClipSizes[weapInfo->clipIndex] - 1 ) * 0.5f ) + 1;
			} else if ( max < 0 ) {
				ent->count = 0;
			} else {
				if ( max == min ) {
					ent->count = min;
				} else {
					ent->count = min + rand() % ( max - min );
				}
				if ( ent->count <= 0 ) {
					ent->count = 0;
				}
			}
		}
		max = bg_ammoTypeMax[ bg_weaponInfo[weapon]->ammoIndex ];
		if ( ent->count > max ) {
			ent->count = max;
		}
		reserveCount = ent->count;
	}

	// and how much is in its magazine
	if ( ent->clipCount < 0 ) {
		clipCount = 0;
	} else {
		if ( !ent->clipCount ) {
			if ( ent->count >= 0 ) {
				ent->clipCount = bg_ammoClipSizes[ bg_weaponInfo[weapon]->clipIndex ];
				if ( ent->clipCount > ent->count ) {
					ent->clipCount = ent->count;
				}
				ent->count -= ent->clipCount;
				reserveCount = ent->count;
			} else {
				ent->clipCount = 0;
			}
		}
		clipSize = bg_ammoClipSizes[ bg_weaponInfo[weapon]->clipIndex ];
		if ( ent->clipCount > clipSize ) {
			ent->clipCount = clipSize;
		}
		clipCount = ent->clipCount;
	}

	client = other->client;

	if ( Com_BitCheck( (const int *)client->ps.weapons, weapon ) ) {
		// already carrying it: this is an ammo pickup
		*event = EV_AMMO_PICKUP;
		total = clipCount + reserveCount;
		added = Add_Ammo( other, weapon, total, qfalse );
		if ( added ) {
			if ( bg_weaponInfo[weapon]->clipOnly ) {
				trap_SendServerCommand( other - g_entities, 0,
										va( "f \"GAME_PICKUP_CLIPONLY_AMMO\x14%s\"", weapInfo->displayName ) );
			} else {
				trap_SendServerCommand( other - g_entities, 0,
										va( "f \"GAME_PICKUP_AMMO\x14%s\"", weapInfo->displayName ) );
			}
		}
		if ( added != total ) {
			ent->count -= added;
			if ( ent->count <= 0 ) {
				ent->clipCount += ent->count;
				ent->count = -1;
				if ( ent->clipCount <= 0 ) {
					ent->clipCount = -1;
				}
			}
			// something is left in the pool, so leave the item in the world
			if ( ( ent->count > 0 || ent->clipCount > 0 ) && g_weaponAmmoPools.integer ) {
				return 0;
			}
		}
	} else {
		currentWeapon = client->ps.weapon;
		if ( currentWeapon ) {
			if ( !Com_BitCheck( (const int *)client->ps.weapons, currentWeapon ) ) {
				return 0;
			}
			if ( !BG_IsPlayerWeaponInSlot( &client->ps, currentWeapon, qtrue )
				 && !BG_GetStackSlotForWeapon( &client->ps, currentWeapon,
											   bg_weaponInfo[ client->ps.weapon ]->weaponSlot )
				 && !BG_GetEmptySlotForWeapon( &client->ps, weapon ) ) {
				Com_Printf( "WARNING: cannot swap out a debug weapon (can result from too many weapons given to the player)\n" );
				return 0;
			}
		}

		currentWeapon = client->ps.weapon;
		currentSlot = bg_weaponInfo[currentWeapon]->weaponSlot;

		if ( !BG_GetEmptySlotForWeapon( &client->ps, weapon )
			 && !BG_GetStackSlotForWeapon( &client->ps, weapon, currentSlot ) ) {
			// no room: something has to go
			if ( weapInfo->weaponSlot == currentSlot ) {
				dropped = Drop_Weapon( other, currentWeapon, NULL );
			} else if ( weapInfo->weaponSlot == WEAPSLOT_PISTOL
						|| weapInfo->weaponSlot == WEAPSLOT_GRENADE
						|| weapInfo->weaponSlot == WEAPSLOT_SMOKEGRENADE ) {
				dropped = Drop_Weapon( other, (signed char)client->ps.weaponslots[ weapInfo->weaponSlot ], NULL );
			} else {
				/* the loop tests the *incoming* weapon's pools while dropping
				   whatever occupies the slot -- the binary reloads
				   bg_weaponInfo[weapon] once, outside the loop (0x20022F7B). */
				for ( slot = WEAPSLOT_PRIMARY ; slot <= WEAPSLOT_PRIMARYB ; slot++ ) {
					if ( !client->ps.ammo[ weapInfo->ammoIndex ]
						 && !client->ps.ammoclip[ weapInfo->clipIndex ] ) {
						dropped = Drop_Weapon( other, (signed char)client->ps.weaponslots[slot], NULL );
						break;
					}
				}
				if ( slot > WEAPSLOT_PRIMARYB ) {
					trap_SendServerCommand( other - g_entities, 0,
											va( "f \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"" ) );
					return 0;
				}
			}

			if ( !dropped ) {
				return 0;
			}

			// leave the replacement where the pickup was
			VectorCopy( ent->r.currentOrigin, origin );
			G_SetOrigin( dropped, origin );
			G_SetAngle( dropped, ent->r.currentAngles );
			trap_LinkEntity( dropped );
		}

		BG_GivePlayerWeapon( &client->ps, weapon );
		if ( !touchMode ) {
			trap_SendServerCommand( other - g_entities, 1, va( "a %i", weapon ) );
		}

		if ( clipCount >= 0 ) {
			clipIndex = bg_weaponInfo[weapon]->clipIndex;
			clipSize = bg_ammoClipSizes[clipIndex];
			if ( clipCount > clipSize ) {
				reserveCount += clipCount - clipSize;
				clipCount = clipSize;
			}
			client->ps.ammoclip[clipIndex] = clipCount;
		}

		Add_Ammo( other, weapon, reserveCount, clipCount == -1 );
	}

	if ( dropped ) {
		Scr_AddEntityNum( dropped->s.number, 0 );
	} else {
		Scr_AddUndefined();
	}
	Scr_AddEntityNum( other->s.number, 0 );
	Scr_NotifyNum( ent->s.number, 0, scr_const.trigger, 2 );

	if ( ent->spawnflags & ITEM_WEAPON_RESPAWN ) {
		return g_weaponrespawn.integer;
	}
	return -1;
}

/*
==============
Pickup_Health
	item->quantity is a percentage of the player's maximum health, not an
	absolute amount.
==============
*/
int Pickup_Health( gentity_t *ent, gentity_t *other ) {
	int max;
	int maxHealth;
	int quantity;
	int oldHealth;
	int oldPercent;
	int newPercent;

	if ( ent->item->quantity == 5 || ent->item->quantity == 100 ) {
		maxHealth = other->client->ps.stats[STAT_MAX_HEALTH];
		max = 2 * maxHealth;
	} else {
		max = other->client->ps.stats[STAT_MAX_HEALTH];
		maxHealth = max;
	}

	quantity = ent->count;
	if ( !quantity ) {
		quantity = ent->item->quantity;
	}

	oldHealth = other->health;
	other->health -= (int)( maxHealth * quantity * -0.01f );

	if ( other->health > max ) {
		other->health = max;
	} else {
		maxHealth = other->client->ps.stats[STAT_MAX_HEALTH];

		newPercent = (int)( 100 * other->health / (float)maxHealth );
		if ( newPercent < 1 ) {
			newPercent = 1;
		} else if ( newPercent > 100 ) {
			newPercent = 100;
		}

		oldPercent = (int)( 100 * oldHealth / (float)maxHealth );
		if ( oldPercent < 1 ) {
			oldPercent = 1;
		}
		oldPercent += quantity;
		if ( oldPercent > 100 ) {
			oldPercent = 100;
		}

		if ( newPercent != oldPercent ) {
			other->health = oldPercent * maxHealth / 100;
		}
	}

	other->client->ps.stats[STAT_HEALTH] = other->health;

	trap_SendServerCommand( other - g_entities, 0,
							va( "f \"GAME_PICKUP_HEALTH\x15%i\"", quantity ) );

	Scr_AddEntityNum( other->s.number, 0 );
	Scr_NotifyNum( ent->s.number, 0, scr_const.trigger, 1 );

	return -1;
}

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {
	// randomly select from teamed entities
	if ( ent->team ) {
		gentity_t   *master;
		int count;
		int choice;

		if ( !ent->teammaster ) {
			G_Error( "RespawnItem: bad teammaster" );
		}
		master = ent->teammaster;

		for ( count = 0, ent = master; ent; ent = ent->teamchain, count++ )
			;

		choice = rand() % count;

		for ( count = 0, ent = master; count < choice; ent = ent->teamchain, count++ )
			;
	}

	ent->r.contents = ITEM_CONTENTS_RESPAWN;
	ent->flags &= ~FL_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	trap_LinkEntity( ent );

	// play the normal respawn sound only to nearby clients
	G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );

	ent->nextthink = 0;
}

/*
==============
Touch_Item_Auto
	touching an item activates it; Touch_Item alone only fires for +activate.
==============
*/
void Touch_Item_Auto( gentity_t *ent, gentity_t *other, int touchMode ) {
	ent->active = qtrue;
	Touch_Item( ent, other, touchMode );
}

/*
===============
Touch_Item
===============
*/
void Touch_Item( gentity_t *ent, gentity_t *other, int touchMode ) {
	int respawn;
	int makenoise = EV_ITEM_PICKUP;
	int weapon;

	// only activated items can be picked up
	if ( !ent->active ) {
		return;
	}
	ent->active = qfalse;

	if ( !other->client ) {
		return;
	}
	if ( other->health < 1 ) {
		return;     // dead people can't pickup
	}

	// the same pickup rules are used for client side and server side
	if ( !BG_CanItemBeGrabbed( &ent->s, &other->client->ps, touchMode ) ) {
		// tell the toucher why he cannot have it, but not for his own drop
		if ( touchMode ) {
			return;
		}
		if ( ent->s.clientNum == other->s.number ) {
			return;
		}
		if ( ent->item->giType != IT_WEAPON ) {
			return;
		}

		weapon = ent->item->giTag;
		if ( Com_BitCheck( (const int *)other->client->ps.weapons, weapon ) ) {
			trap_SendServerCommand( other - g_entities, 0,
									va( "f \"GAME_PICKUP_CANTCARRYMOREAMMO\x14%s\"",
										bg_weaponInfo[weapon]->displayName ) );
			return;
		}

		switch ( bg_weaponInfo[weapon]->weaponSlot ) {
		case WEAPSLOT_PRIMARY:
		case WEAPSLOT_PRIMARYB:
			trap_SendServerCommand( other - g_entities, 0,
									va( "f \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"" ) );
			break;
		case WEAPSLOT_PISTOL:
			trap_SendServerCommand( other - g_entities, 0,
									va( "f \"GAME_CANT_GET_PISTOL_WEAP_MESSAGE\"" ) );
			break;
		case WEAPSLOT_GRENADE:
			trap_SendServerCommand( other - g_entities, 0,
									va( "f \"GAME_CANT_GET_GRENADE_WEAP_MESSAGE\"" ) );
			break;
		case WEAPSLOT_SMOKEGRENADE:
			trap_SendServerCommand( other - g_entities, 0,
									va( "f \"GAME_CANT_GET_SMOKER_WEAP_MESSAGE\"" ) );
			break;
		default:
			break;
		}
		return;
	}

	if ( ent->item->giType == IT_WEAPON ) {
		G_LogPrintf( "Weapon: %i %s\n", other->s.number,
					 bg_weaponInfo[ ent->item->giTag ]->szInternalName );
	} else {
		G_LogPrintf( "Item: %i %s\n", other->s.number, ent->item->classname );
	}

	// call the item-specific pickup function
	switch ( ent->item->giType ) {
	case IT_WEAPON:
		respawn = Pickup_Weapon( ent, other, &makenoise, touchMode );
		break;
	case IT_AMMO:
		respawn = Pickup_Ammo( ent, other );
		break;
	case IT_HEALTH:
		respawn = Pickup_Health( ent, other );
		break;
	default:
		return;
	}

	if ( !respawn ) {
		return;
	}

	// play sounds
	if ( ent->noise_index ) {
		// a sound was specified on the entity, so play that one and send the
		// pickup as the quiet variant
		makenoise = EV_ITEM_PICKUP_QUIET;
		G_PlaySoundAlias( other, ent->noise_index );
	}

	// send the pickup event
	if ( other->client->sess.predictItemPickup ) {
		G_AddPredictableEvent( other, makenoise, ent->s.index );
	} else {
		G_AddEvent( other, makenoise, ent->s.index );
	}

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) {
		ent->s.eFlags |= EF_NODRAW;
		ent->flags |= FL_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	// non zero wait overrides respawn time
	if ( ent->wait ) {
		respawn = ent->wait;
	}

	// random can be used to vary the respawn time
	if ( ent->random ) {
		respawn += crandom() * ent->random;
		if ( respawn < 1 ) {
			respawn = 1;
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->freeAfterEvent = qtrue;
	}

	// picked up items still stay around, they just don't draw anything, so
	// respawnable items can be placed on movers
	ent->flags |= FL_NODRAW;
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->r.contents = 0;

	// a negative respawn time means never respawn this item, but don't delete it
	if ( respawn > 0 ) {
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
	} else {
		ent->nextthink = 0;
		ent->think = NULL;
	}

	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->think = G_FreeEntity;
		ent->nextthink = level.time + 100;
	}

	trap_LinkEntity( ent );
}

/*
================
GetFreeCueSpot

Pick a slot of the dropped-item cue.  An empty slot wins; otherwise the item
whose nearest player is furthest away is scheduled to be freed and its slot is
reused.
================
*/
int GetFreeCueSpot( void ) {
	gentity_t *ent;
	vec3_t delta;
	float len;
	float dist;
	float farthest;
	int farthestSpot;
	int i;
	int j;

	farthestSpot = 0;
	farthest = 0.0f;

	for ( i = 0 ; i < MAX_ITEM_CUE ; i++ ) {
		if ( !level.itemCue[i] ) {
			return i;
		}

		ent = &g_entities[ level.itemCue[i] ];
		if ( !ent->inuse ) {
			level.itemCue[i] = 0;
			return i;
		}

		dist = 99999.0f;
		for ( j = 0 ; j < level.maxclients ; j++ ) {
			if ( level.clients[j].sess.connected != CON_CONNECTED ) {
				continue;
			}
			if ( level.clients[j].ps.pm_type != PM_INTERMISSION ) {
				continue;
			}
			VectorSubtract( ent->r.currentOrigin, g_entities[j].r.currentOrigin, delta );
			len = VectorLength( delta );
			if ( len < dist ) {
				dist = len;
			}
		}

		if ( dist > farthest ) {
			farthest = dist;
			farthestSpot = i;
		}
	}

	g_entities[ level.itemCue[farthestSpot] ].think = G_FreeEntity;
	g_entities[ level.itemCue[farthestSpot] ].nextthink = level.time + 1;

	return farthestSpot;
}

/*
================
DroppedItemClearOwner
================
*/
void DroppedItemClearOwner( gentity_t *ent ) {
	ent->s.clientNum = ENTITYNUM_WORLD;
}

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, const vec3_t origin, const vec3_t velocity, int ownerNum ) {
	gentity_t   *dropped;
	int itemIndex;

	itemIndex = item - bg_itemlist;
	RegisterItem( itemIndex, qtrue );

	dropped = G_Spawn();

	level.itemCue[ GetFreeCueSpot() ] = dropped - g_entities;

	dropped->s.eType = ET_ITEM;
	dropped->s.index = itemIndex;       // store item number in index

	G_SetConstString( &dropped->classname, item->classname );
	dropped->item = item;

	dropped->r.mins[0] = -1;
	dropped->r.mins[1] = -1;
	if ( item->giType == IT_WEAPON ) {
		dropped->r.mins[2] = -1;
		dropped->r.maxs[2] = 1;
	} else {
		dropped->r.mins[2] = 0;
		dropped->r.maxs[2] = 2;
	}
	dropped->r.maxs[0] = 1;
	dropped->r.maxs[1] = 1;

	dropped->r.svFlags |= SVF_ITEM;
	dropped->s.eFlags |= EF_ITEM;
	dropped->r.contents = ITEM_CONTENTS;
	dropped->clipmask = MASK_DROPPED_ITEM;
	dropped->s.clientNum = ownerNum;

	if ( *item->world_model[0] ) {
		dropped->model = G_ModelIndex( item->world_model[0] );
	} else {
		dropped->model = 0;
	}
	G_DObjUpdate( dropped );

	dropped->touch = Touch_Item_Auto;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	// the dropper keeps ownership just long enough to get clear of it
	dropped->think = DroppedItemClearOwner;
	dropped->nextthink = level.time + 1000;

	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity( dropped );

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle, qboolean novelocity ) {
	vec3_t velocity;
	vec3_t angles;
	vec3_t origin;

	angles[YAW] = ent->r.currentAngles[YAW] + angle;

	if ( novelocity ) {
		VectorClear( velocity );
	} else {
		angles[PITCH] = 0;  // always forward
		AngleVectors( angles, velocity, NULL, NULL );
		VectorScale( velocity, 150, velocity );
		velocity[2] += 200 + ( 2.0f * ( (float)rand() / 32768.0f ) - 1.0f ) * 50;
	}

	origin[0] = ent->r.currentOrigin[0];
	origin[1] = ent->r.currentOrigin[1];
	origin[2] = ent->r.currentOrigin[2] + ( ent->r.maxs[2] - ent->r.mins[2] ) * 0.5f;

	return LaunchItem( item, origin, velocity, ent->s.number );
}

/*
================
Drop_Weapon

Drop `weapon` out of `ent`, either at ent's tag (tagName) or in front of it.
When ent has no client the ammo the drop carries is rolled from the weapon's
dropAmmoMin/dropAmmoMax; otherwise the player's own pools are emptied into it.
================
*/
gentity_t *Drop_Weapon( gentity_t *ent, int weapon, const char *tagName ) {
	gclient_t *client;
	weaponInfo_t *weapInfo;
	gentity_t *dropped;
	int ammoIndex;
	int clipIndex;
	int ammoCount;
	int clipCount;
	int total;
	int clipSize;
	int min;
	int max;
	int swap;
	float tagMatrix[16];
	vec3_t start;
	vec3_t angles;
	trace_t tr;

	client = ent->client;

	if ( client && !Com_BitCheck( (const int *)client->ps.weapons, weapon ) ) {
		BG_TakePlayerWeapon( &client->ps, weapon );
		return NULL;
	}

	weapInfo = bg_weaponInfo[weapon];
	ammoIndex = weapInfo->ammoIndex;
	clipIndex = weapInfo->clipIndex;

	if ( weapInfo->clipOnly && !client->ps.ammoclip[clipIndex] ) {
		BG_TakePlayerWeapon( &client->ps, weapon );
		return NULL;
	}

	dropped = Drop_Item( ent, &bg_itemlist[weapon], 0, qfalse );

	if ( ent->client ) {
		ammoCount = ent->client->ps.ammo[ammoIndex];
		ent->client->ps.ammo[ammoIndex] = 0;
		clipCount = ent->client->ps.ammoclip[clipIndex];
		ent->client->ps.ammoclip[clipIndex] = 0;
		BG_TakePlayerWeapon( &ent->client->ps, weapon );
	} else {
		min = bg_weaponInfo[weapon]->dropAmmoMin;
		max = bg_weaponInfo[weapon]->dropAmmoMax;
		if ( max < min ) {
			swap = min;
			min = max;
			max = swap;
		}
		if ( !max && !min ) {
			clipSize = bg_ammoClipSizes[clipIndex];
			total = Q_ftol( ( random() + 1.0f ) * ( clipSize - 1 ) * 0.5f ) + 1;
			clipCount = Q_ftol( ( random() * 0.5f + 0.25f ) * total );
			ammoCount = total - clipCount;
		} else if ( max < 0 ) {
			ammoCount = 0;
			clipCount = 0;
		} else {
			if ( max == min ) {
				total = min;
			} else {
				total = min + rand() % ( max - min );
			}
			if ( total <= 0 ) {
				ammoCount = 0;
				clipCount = 0;
			} else {
				clipSize = bg_ammoClipSizes[clipIndex];
				if ( clipSize ) {
					clipCount = rand() % clipSize;
				} else {
					clipCount = 0;
				}
				if ( clipCount >= total ) {
					clipCount = total;
					ammoCount = 0;
				} else {
					ammoCount = total - clipCount;
				}
			}
		}
	}

	dropped->count = ammoCount;
	dropped->clipCount = clipCount;
	if ( !ammoCount ) {
		dropped->count = -1;
	}
	if ( !clipCount ) {
		dropped->clipCount = -1;
	}

	if ( tagName ) {
		if ( G_DObjGetWorldTagMatrix( ent, tagName, tagMatrix ) ) {
			start[0] = ( ent->r.mins[0] + ent->r.maxs[0] ) * 0.5f + ent->r.currentOrigin[0];
			start[1] = ( ent->r.mins[1] + ent->r.maxs[1] ) * 0.5f + ent->r.currentOrigin[1];
			start[2] = ( ent->r.mins[2] + ent->r.maxs[2] ) * 0.5f + ent->r.currentOrigin[2];

			// &tagMatrix[12] is the matrix' translation row
			trap_TraceCapsule( &tr, start, dropped->r.mins, dropped->r.maxs,
							   &tagMatrix[12], ent->s.number, MASK_ITEM );

			VectorCopy( tr.endpos, dropped->s.pos.trBase );
			VectorCopy( tr.endpos, dropped->r.currentOrigin );
			dropped->s.pos.trTime = level.time;

			// the tag's own orientation is computed and then thrown away
			Axis4ToAngles( tagMatrix, angles );
		}

		G_SetAngle( dropped, ent->r.currentAngles );

		dropped->s.apos.trType = TR_LINEAR;
		dropped->s.apos.trTime = level.time;
		dropped->s.apos.trDelta[0] = crandom() * 50;
		dropped->s.apos.trDelta[1] = crandom() * 40;
		dropped->s.apos.trDelta[2] = crandom() * 60;
	}

	return dropped;
}

/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	RespawnItem( ent );
}

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points, and lays it flat on what it landed on.
================
*/
void FinishSpawningItem( gentity_t *ent ) {
	trace_t tr;
	vec3_t dest;
	vec3_t start;
	vec3_t mins;
	vec3_t maxs;
	vec3_t axis[3];
	vec3_t angles;

	if ( ent->spawnflags & ITEM_SUSPENDED ) {
		G_SetOrigin( ent, ent->r.currentOrigin );
	} else {
		mins[0] = -1;
		mins[1] = -1;
		if ( ent->item->giType == IT_WEAPON ) {
			mins[2] = -1;
			maxs[2] = 1;
		} else {
			mins[2] = 0;
			maxs[2] = 2;
		}
		maxs[0] = 1;
		maxs[1] = 1;

		ent->r.svFlags |= SVF_ITEM;
		ent->s.eFlags |= EF_ITEM;

		VectorSet( dest, ent->r.currentOrigin[0], ent->r.currentOrigin[1],
				   ent->r.currentOrigin[2] - 4096 );
		trap_TraceCapsule( &tr, ent->r.currentOrigin, mins, maxs, dest, ent->s.number, MASK_ITEM );

		if ( tr.startsolid ) {
			VectorCopy( ent->r.currentOrigin, start );
			start[2] -= ITEM_RADIUS;

			VectorSet( dest, ent->r.currentOrigin[0], ent->r.currentOrigin[1],
					   ent->r.currentOrigin[2] - 4096 );
			trap_TraceCapsule( &tr, start, mins, maxs, dest, ent->s.number, MASK_ITEM );

			if ( tr.startsolid ) {
				G_Printf( "FinishSpawningItem: %s startsolid at %s\n",
						  SL_ConvertToString( ent->classname ), vtos( ent->r.currentOrigin ) );
				G_FreeEntity( ent );
				return;
			}
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;
		G_SetOrigin( ent, tr.endpos );

		if ( tr.fraction < 1.0f ) {
			VectorCopy( tr.normal, axis[2] );
			// retail 0x20024421 builds axis[0] from the FORWARD vector (arg
			// passed in esi), not up: right/up are NULL (edi/ebx=0).  Using up
			// makes axis[0] parallel to the floor normal, so the surface-align
			// cross products degenerate and the yaw is decided by fp residue --
			// which flipped roll-180 ground weapons (mp_depot/mp_carentan).
			AngleVectors( ent->r.currentAngles, axis[0], NULL, NULL );
			CrossProduct( axis[2], axis[0], axis[1] );
			CrossProduct( axis[1], axis[2], axis[0] );
			AxisToAngles( axis, angles );
			if ( bg_itemlist[ ent->s.index ].giType == IT_WEAPON ) {
				angles[ROLL] += 90;
			}
			G_SetAngle( ent, angles );
		}
	}

	trap_LinkEntity( ent );
}

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {
	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// the placeholder row is always "registered"
	itemRegistered[0] = qtrue;
}

/*
===============
SaveRegisteredItems

Write the needed items to a config string so the client will know which ones to
precache.  The set is packed four items to a hexadecimal digit.
===============
*/
void SaveRegisteredItems( void ) {
	char string[MAX_ITEMS + 1];
	int i;
	int n;
	int bit;
	int nibble;

	level.registeredItemsDirty = qfalse;

	n = 0;
	bit = 0;
	nibble = 0;
	for ( i = 0 ; i < BG_NUM_ITEMS ; i++ ) {
		if ( itemRegistered[i] ) {
			nibble += 1 << bit;
		}
		if ( ++bit == 4 ) {
			string[n++] = nibble + ( nibble < 10 ? '0' : 'a' - 10 );
			nibble = 0;
			bit = 0;
		}
	}
	if ( bit ) {
		string[n++] = nibble + ( nibble < 10 ? '0' : 'a' - 10 );
	}
	string[n] = 0;

	trap_SetConfigstring( CS_ITEMS, string );
}

/*
===============
RegisterItem

The item and its models will be added to the precache list.
===============
*/
void RegisterItem( int itemIndex, qboolean updateConfigString ) {
	const char *name;
	weaponInfo_t *weapInfo;
	int weapon;

	if ( itemRegistered[itemIndex] ) {
		return;
	}

	if ( !level.spawning ) {
		name = bg_itemlist[itemIndex].pickup_name;
		if ( ( !name || !*name ) && itemIndex <= bg_numWeapons ) {
			name = bg_weaponInfo[itemIndex]->szInternalName;
		}
		if ( !name || !*name ) {
			name = "<<unknown>>";
		}
		Scr_Error( va( "game tried to register the item '%s' after initialization finished\n", name ) );
	}

	itemRegistered[itemIndex] = qtrue;

	if ( bg_itemlist[itemIndex].giType == IT_WEAPON ) {
		// register every weapon of the alt chain, not just this one
		weapon = itemIndex;
		do {
			itemRegistered[weapon] = qtrue;
			weapInfo = bg_weaponInfo[weapon];
			G_ModelIndex( weapInfo->worldModel );
			G_ModelIndex( weapInfo->projectileModel );
			weapon = weapInfo->altWeaponIndex;
		} while ( weapon && weapon != itemIndex );
	} else {
		if ( bg_itemlist[itemIndex].world_model[0] ) {
			G_ModelIndex( bg_itemlist[itemIndex].world_model[0] );
		}
		if ( bg_itemlist[itemIndex].world_model[1] ) {
			G_ModelIndex( bg_itemlist[itemIndex].world_model[1] );
		}
	}

	if ( updateConfigString ) {
		level.registeredItemsDirty = qtrue;
	}
}

/*
===============
IsItemRegistered
===============
*/
qboolean IsItemRegistered( int itemIndex ) {
	return itemRegistered[itemIndex];
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might be on an entity
that hasn't spawned yet.
============
*/
void G_SpawnItem( gentity_t *ent, gitem_t *item ) {
	char    *noise;

	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item - bg_itemlist, qfalse );
	ent->item = item;

	if ( *item->world_model[0] ) {
		ent->model = G_ModelIndex( item->world_model[0] );
	} else {
		ent->model = 0;
	}

	if ( G_SpawnString( "noise", 0, &noise ) ) {
		ent->noise_index = G_SoundAliasIndex( noise );
	}

	ent->physicsBounce = 0;

	ent->r.mins[0] = -1;
	ent->r.mins[1] = -1;
	if ( item->giType == IT_WEAPON ) {
		ent->r.mins[2] = -1;
		ent->r.maxs[2] = 1;
	} else {
		ent->r.mins[2] = 0;
		ent->r.maxs[2] = 2;
	}
	ent->r.maxs[0] = 1;
	ent->r.maxs[1] = 1;

	ent->r.svFlags |= SVF_ITEM;
	ent->s.eFlags |= EF_ITEM;
	ent->r.contents = ITEM_CONTENTS;

	ent->touch = Touch_Item_Auto;
	ent->s.eType = ET_ITEM;
	ent->s.index = ent->item - bg_itemlist;     // store item number in index
	G_DObjUpdate( ent );

	ent->s.clientNum = ENTITYNUM_WORLD;
	ent->use = Use_Item;
	ent->flags |= FL_ITEM_SPAWNED;

	if ( level.spawningMapEntities ) {
		// some movers spawn on the second frame, so delay item spawns until
		// the third frame so they can ride trains
		G_SetAngle( ent, ent->r.currentAngles );
		ent->nextthink = level.time + 200;
		ent->think = FinishSpawningItem;
	} else {
		if ( !( ent->spawnflags & ITEM_SUSPENDED ) ) {
			ent->s.groundEntityNum = ENTITYNUM_NONE;
			if ( item->giType == IT_WEAPON ) {
				ent->r.currentAngles[ROLL] += 90;
			}
		}
		G_SetAngle( ent, ent->r.currentAngles );
		G_SetOrigin( ent, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}
}

/*
================
G_BounceItem
================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t velocity;
	vec3_t angles;
	vec3_t axis[3];
	vec3_t end;
	float dot;
	int hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + (int)( ( level.time - level.previousTime ) * trace->fraction );
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->normal );
	VectorMA( velocity, -2 * dot, trace->normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	if ( trace->startsolid ) {
		// squeezed into something: drop it straight out instead
		VectorCopy( ent->r.currentOrigin, end );
		end[2] -= 128;
		VectorClear( ent->s.pos.trDelta );
		if ( ent->s.eFlags & EF_ITEM ) {
			trap_TraceCapsule( trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, end,
							   ent->s.number, MASK_ITEM );
		} else {
			trap_Trace( trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, end,
						ent->s.number, MASK_ITEM );
		}
	}

	// check for stop
	if ( trace->normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		trace->endpos[2] += ( (float)rand() / 32768.0f + 1.0f ) * 0.5f;     // make sure it is off ground
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;

		// lay it flat on the surface it settled against
		VectorCopy( trace->normal, axis[2] );
		AngleVectors( ent->r.currentAngles, axis[0], NULL, NULL );
		CrossProduct( axis[2], axis[0], axis[1] );
		CrossProduct( axis[1], axis[2], axis[0] );
		AxisToAngles( axis, angles );
		if ( bg_itemlist[ ent->s.index ].giType == IT_WEAPON ) {
			angles[ROLL] += 90;
		}
		G_SetAngle( ent, angles );
		trap_LinkEntity( ent );
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->normal, ent->r.currentOrigin );
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}

/*
================
G_RunItem
================
*/
void G_RunItem( gentity_t *ent ) {
	vec3_t origin;
	trace_t tr;
	int contents;
	int mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == ENTITYNUM_NONE ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY || ent->s.pos.trType == TR_GRAVITY_PAUSED ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_ITEM_CLIP;
	}
	if ( ent->s.eFlags & EF_ITEM ) {
		trap_TraceCapsule( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin,
						   ent->r.ownerNum, mask );
	} else {
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin,
					ent->r.ownerNum, mask );
	}

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	trap_LinkEntity( ent );     // FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( !ent->inuse ) {
		return;
	}
	if ( tr.fraction == 1 ) {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1, CONTENTS_NODROP );
	if ( contents ) {
		G_FreeEntity( ent );
		return;
	}

	G_BounceItem( ent, &tr );
}
