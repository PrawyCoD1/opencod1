/*
 * bg_misc.c -- the pieces of the game/cgame shared layer that are not movement,
 * weapons or animation: the item list, trajectory evaluation, the player-state
 * to entity-state conversions and the prone-position test.
 *
 * Call of Duty 1.1 multiplayer (game_mp_x86.dll, 0x20004F50 .. 0x20006FE4).
 * RTCW's game/bg_misc.c is the ancestor: BG_FindItem, BG_CanItemBeGrabbed,
 * BG_EvaluateTrajectory, BG_EvaluateTrajectoryDelta and
 * BG_AddPredictableEventToPlayerstate are ports of the original text; the two
 * PlayerStateToEntityState functions were reworked around CoD's entityState_t,
 * and BG_GetMarkDir is new.
 *
 * trType_t, the entity_event_t enum, DEFAULT_GRAVITY, SnapVector and the
 * eFlags/pm_flags bit names are all still missing from the headers, so the
 * numbers below are spelled out with the name in a comment rather than
 * invented here.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <math.h>

#include "g_public.h"
#include "bg_public.h"

/* No header covers the qcommon / q_math half of the module, and neither
 * bg_weapon.c's nor bg_pmove.c's exports are published. */
void QDECL Com_Error( int level, const char *fmt, ... );
int Q_stricmp( const char *s1, const char *s2 );
float VectorNormalize2( const vec3_t v, vec3_t out );
float AngleNormalize180( float angle );

float vectopitch( const vec3_t vec );
float AngleSubtract( float a1, float a2 );

int PM_GetEffectiveStance( const playerState_t *ps );
int PM_GetViewHeightLerpTime( const playerState_t *ps, int target, int down );

/*
 * The prone check draws itself, and BOTH builds draw: the game module through
 * g_debug_mp.c, the client through cg_drawtools.c.  The call shape is the same
 * on both sides -- only the spelling differs -- so the split is a conditional
 * on the symbol and BG_CheckProneValid's body below is untouched.
 *
 *   G_DebugBox       -> CG_DebugBox        (cgame 0x3001A1A0)
 *   G_DebugCircleEx  -> CG_DebugCircleEx   (cgame 0x3001A220)
 *   g_debugProneCheck / g_debugProneCheckDepthCheck  ->  cg_ ...
 *
 * cgame's BG_CheckProneValid (0x300062A0): 0x30006330 calls CG_DebugBox with
 * the same five arguments -- mins@eax, maxs@edx, color@ebx, depthTest@edi and
 * duration on the stack (`push 1` at 0x30006321, `add esp,4` at 0x30006335)
 * -- and 0x30006556 calls CG_DebugCircleEx with the same six.  The two cvar
 * reads are at 0x3000631B and 0x3000652A.
 *
 * Retail defect worth knowing: CG_DebugBox ACCEPTS duration and then passes a
 * hardcoded 0 to the trap (`push 0` at 0x3001A1EB), where G_DebugBox passes
 * the parameter through.  Same source, one build with a typo.
 */
#ifdef CGAMEDLL
#define G_DebugBox                      CG_DebugBox
#define G_DebugCircleEx                 CG_DebugCircleEx
#define g_debugProneCheck               cg_debugProneCheck
#define g_debugProneCheckDepthCheck     cg_debugProneCheckDepthCheck
#endif

void trap_AddDebugLine( const vec3_t start, const vec3_t end, const vec3_t color,
						int depthTest, int duration );
void G_DebugBox( const vec3_t mins, const vec3_t maxs, const vec3_t color,
				 int depthTest, int duration );
void G_DebugCircleEx( const vec3_t dir, const vec3_t org, float radius, const vec3_t color,
					  int depthTest, int duration );

/* g_main_mp.c cvar table (cg_main_mp.c's, in the client build) */
extern vmCvar_t g_debugProneCheck;                  /* 0x20235660, cgame 0x301E0D00 */
extern vmCvar_t g_debugProneCheckDepthCheck;        /* 0x20235420, cgame 0x301DC1A0 */

/*
 * q_shared.c's colour table, 0x20055480 .. 0x2005555F (colorMdCyan's name is
 * inferred; every other entry matches an exact Q3/RTCW value): black, red, green,
 * dkgreen, blue, yellow, {.75 .75 0}, {.5 .5 0}, magenta, cyan, {0 .75 .75},
 * {0 .5 .5}, {0 .25 .25}, white -- sixteen bytes apart, in that order.  Only
 * the six BG_CheckProneValid names are declared here.  colorMdCyan is the
 * {0, 0.5, 0.5, 1} entry at 0x20055530, named from its place in the Lt/Md/Dk
 * run.
 */
extern const vec4_t colorRed;                       /* 0x20055490 */
extern const vec4_t colorGreen;                     /* 0x200554A0 */
extern const vec4_t colorYellow;                    /* 0x200554D0 */
extern const vec4_t colorMagenta;                   /* 0x20055500 */
extern const vec4_t colorCyan;                      /* 0x20055510 */
extern const vec4_t colorMdCyan;                    /* 0x20055530 */

/*
 * bg_itemlist.  0x2006BB68, 71 entries of 48 bytes (70 items plus the NULL
 * terminator RTCW also carries).  [0] is the IT_BAD placeholder, [1..64] are
 * the placeholders BG_FillInWeaponItems (bg_weapon.c) overwrites from the
 * parsed weapon files, [65..69] are the two grenade-ammo and three health
 * items.  The stringized-argument spelling of the placeholder classnames is
 * the retail one: the strings in the image read  emptyitem_"w01"  with the
 * quotes embedded, which only a  "emptyitem_" #x  macro produces.
 */
#define EMPTY_WEAPON_ITEM( x ) \
	{ "emptyitem_" #x, NULL, {NULL, NULL}, NULL, NULL, NULL, 0, IT_BAD, 0, 0, 0 }

gitem_t bg_itemlist[] =
{
	{ NULL, NULL, {NULL, NULL}, NULL, NULL, NULL, 0, IT_BAD, 0, 0, 0 },     /* leave index 0 alone */

	EMPTY_WEAPON_ITEM( "w01" ),
	EMPTY_WEAPON_ITEM( "w02" ),
	EMPTY_WEAPON_ITEM( "w03" ),
	EMPTY_WEAPON_ITEM( "w04" ),
	EMPTY_WEAPON_ITEM( "w05" ),
	EMPTY_WEAPON_ITEM( "w06" ),
	EMPTY_WEAPON_ITEM( "w07" ),
	EMPTY_WEAPON_ITEM( "w08" ),
	EMPTY_WEAPON_ITEM( "w09" ),
	EMPTY_WEAPON_ITEM( "w10" ),
	EMPTY_WEAPON_ITEM( "w11" ),
	EMPTY_WEAPON_ITEM( "w12" ),
	EMPTY_WEAPON_ITEM( "w13" ),
	EMPTY_WEAPON_ITEM( "w14" ),
	EMPTY_WEAPON_ITEM( "w15" ),
	EMPTY_WEAPON_ITEM( "w16" ),
	EMPTY_WEAPON_ITEM( "w17" ),
	EMPTY_WEAPON_ITEM( "w18" ),
	EMPTY_WEAPON_ITEM( "w19" ),
	EMPTY_WEAPON_ITEM( "w20" ),
	EMPTY_WEAPON_ITEM( "w21" ),
	EMPTY_WEAPON_ITEM( "w22" ),
	EMPTY_WEAPON_ITEM( "w23" ),
	EMPTY_WEAPON_ITEM( "w24" ),
	EMPTY_WEAPON_ITEM( "w25" ),
	EMPTY_WEAPON_ITEM( "w26" ),
	EMPTY_WEAPON_ITEM( "w27" ),
	EMPTY_WEAPON_ITEM( "w28" ),
	EMPTY_WEAPON_ITEM( "w29" ),
	EMPTY_WEAPON_ITEM( "w30" ),
	EMPTY_WEAPON_ITEM( "w31" ),
	EMPTY_WEAPON_ITEM( "w32" ),
	EMPTY_WEAPON_ITEM( "w33" ),
	EMPTY_WEAPON_ITEM( "w34" ),
	EMPTY_WEAPON_ITEM( "w35" ),
	EMPTY_WEAPON_ITEM( "w36" ),
	EMPTY_WEAPON_ITEM( "w37" ),
	EMPTY_WEAPON_ITEM( "w38" ),
	EMPTY_WEAPON_ITEM( "w39" ),
	EMPTY_WEAPON_ITEM( "w40" ),
	EMPTY_WEAPON_ITEM( "w41" ),
	EMPTY_WEAPON_ITEM( "w42" ),
	EMPTY_WEAPON_ITEM( "w43" ),
	EMPTY_WEAPON_ITEM( "w44" ),
	EMPTY_WEAPON_ITEM( "w45" ),
	EMPTY_WEAPON_ITEM( "w46" ),
	EMPTY_WEAPON_ITEM( "w47" ),
	EMPTY_WEAPON_ITEM( "w48" ),
	EMPTY_WEAPON_ITEM( "w49" ),
	EMPTY_WEAPON_ITEM( "w50" ),
	EMPTY_WEAPON_ITEM( "w51" ),
	EMPTY_WEAPON_ITEM( "w52" ),
	EMPTY_WEAPON_ITEM( "w53" ),
	EMPTY_WEAPON_ITEM( "w54" ),
	EMPTY_WEAPON_ITEM( "w55" ),
	EMPTY_WEAPON_ITEM( "w56" ),
	EMPTY_WEAPON_ITEM( "w57" ),
	EMPTY_WEAPON_ITEM( "w58" ),
	EMPTY_WEAPON_ITEM( "w59" ),
	EMPTY_WEAPON_ITEM( "w60" ),
	EMPTY_WEAPON_ITEM( "w61" ),
	EMPTY_WEAPON_ITEM( "w62" ),
	EMPTY_WEAPON_ITEM( "w63" ),
	EMPTY_WEAPON_ITEM( "w64" ),

	{
		"item_ammo_stielhandgranate_open",
		"grenade_pickup",
		{ "xmodel/ammo_stielhandgranate1", NULL },
		"gfx/icons/hud@steilhandgrenate",
		"gfx/icons/hud@steilhandgrenate",
		"Stielhandgranate_mp Ammo Open",
		10, IT_AMMO, -1, -1, -1
	},
	{
		"item_ammo_stielhandgranate_closed",
		"grenade_pickup",
		{ "xmodel/ammo_stielhandgranate2", NULL },
		"gfx/icons/hud@steilhandgrenate",
		"gfx/icons/hud@steilhandgrenate",
		"Stielhandgranate_mp Ammo Closed",
		10, IT_AMMO, -1, -1, -1
	},
	{
		"item_health_small",
		"health_pickup_small",
		{ "xmodel/health_small", NULL },
		"icons/iconh_small",
		NULL,
		"Small Health",
		10, IT_HEALTH, 0, 0, 0
	},
	{
		"item_health",
		"health_pickup_medium",
		{ "xmodel/health_medium", NULL },
		"icons/iconh_med",
		NULL,
		"Med Health",
		25, IT_HEALTH, 0, 0, 0
	},
	{
		"item_health_large",
		"health_pickup_large",
		{ "xmodel/health_large", NULL },
		"icons/iconh_large",
		NULL,
		"Large Health",
		50, IT_HEALTH, 0, 0, 0
	},

	{ NULL, NULL, {NULL, NULL}, NULL, NULL, NULL, 0, IT_BAD, 0, 0, 0 }
};

/*
 * eventnames.  0x2006C8B8, 202 entries, no terminator.  Nothing in the retail
 * game module reads it -- its only reference is a dead .data pointer slot at
 * 0x2006CBE0 -- but it is bg_misc.c's table (RTCW keeps it here too) and it is
 * the authority on the entity_event_t ordering this DLL uses.
 */
char *eventnames[] =
{
	"EV_NONE",
	"EV_FOOTSTEP_RUN_DEFAULT",
	"EV_FOOTSTEP_RUN_BARK",
	"EV_FOOTSTEP_RUN_BRICK",
	"EV_FOOTSTEP_RUN_CARPET",
	"EV_FOOTSTEP_RUN_CLOTH",
	"EV_FOOTSTEP_RUN_CONCRETE",
	"EV_FOOTSTEP_RUN_DIRT",
	"EV_FOOTSTEP_RUN_FLESH",
	"EV_FOOTSTEP_RUN_FOLIAGE",
	"EV_FOOTSTEP_RUN_GLASS",
	"EV_FOOTSTEP_RUN_GRASS",
	"EV_FOOTSTEP_RUN_GRAVEL",
	"EV_FOOTSTEP_RUN_ICE",
	"EV_FOOTSTEP_RUN_METAL",
	"EV_FOOTSTEP_RUN_MUD",
	"EV_FOOTSTEP_RUN_PAPER",
	"EV_FOOTSTEP_RUN_PLASTER",
	"EV_FOOTSTEP_RUN_ROCK",
	"EV_FOOTSTEP_RUN_SAND",
	"EV_FOOTSTEP_RUN_SNOW",
	"EV_FOOTSTEP_RUN_WATER",
	"EV_FOOTSTEP_RUN_WOOD",
	"EV_FOOTSTEP_RUN_ASPHALT",
	"EV_FOOTSTEP_WALK_DEFAULT",
	"EV_FOOTSTEP_WALK_BARK",
	"EV_FOOTSTEP_WALK_BRICK",
	"EV_FOOTSTEP_WALK_CARPET",
	"EV_FOOTSTEP_WALK_CLOTH",
	"EV_FOOTSTEP_WALK_CONCRETE",
	"EV_FOOTSTEP_WALK_DIRT",
	"EV_FOOTSTEP_WALK_FLESH",
	"EV_FOOTSTEP_WALK_FOLIAGE",
	"EV_FOOTSTEP_WALK_GLASS",
	"EV_FOOTSTEP_WALK_GRASS",
	"EV_FOOTSTEP_WALK_GRAVEL",
	"EV_FOOTSTEP_WALK_ICE",
	"EV_FOOTSTEP_WALK_METAL",
	"EV_FOOTSTEP_WALK_MUD",
	"EV_FOOTSTEP_WALK_PAPER",
	"EV_FOOTSTEP_WALK_PLASTER",
	"EV_FOOTSTEP_WALK_ROCK",
	"EV_FOOTSTEP_WALK_SAND",
	"EV_FOOTSTEP_WALK_SNOW",
	"EV_FOOTSTEP_WALK_WATER",
	"EV_FOOTSTEP_WALK_WOOD",
	"EV_FOOTSTEP_WALK_ASPHALT",
	"EV_FOOTSTEP_PRONE_DEFAULT",
	"EV_FOOTSTEP_PRONE_BARK",
	"EV_FOOTSTEP_PRONE_BRICK",
	"EV_FOOTSTEP_PRONE_CARPET",
	"EV_FOOTSTEP_PRONE_CLOTH",
	"EV_FOOTSTEP_PRONE_CONCRETE",
	"EV_FOOTSTEP_PRONE_DIRT",
	"EV_FOOTSTEP_PRONE_FLESH",
	"EV_FOOTSTEP_PRONE_FOLIAGE",
	"EV_FOOTSTEP_PRONE_GLASS",
	"EV_FOOTSTEP_PRONE_GRASS",
	"EV_FOOTSTEP_PRONE_GRAVEL",
	"EV_FOOTSTEP_PRONE_ICE",
	"EV_FOOTSTEP_PRONE_METAL",
	"EV_FOOTSTEP_PRONE_MUD",
	"EV_FOOTSTEP_PRONE_PAPER",
	"EV_FOOTSTEP_PRONE_PLASTER",
	"EV_FOOTSTEP_PRONE_ROCK",
	"EV_FOOTSTEP_PRONE_SAND",
	"EV_FOOTSTEP_PRONE_SNOW",
	"EV_FOOTSTEP_PRONE_WATER",
	"EV_FOOTSTEP_PRONE_WOOD",
	"EV_FOOTSTEP_PRONE_ASPHALT",
	"EV_JUMP_DEFAULT",
	"EV_JUMP_BARK",
	"EV_JUMP_BRICK",
	"EV_JUMP_CARPET",
	"EV_JUMP_CLOTH",
	"EV_JUMP_CONCRETE",
	"EV_JUMP_DIRT",
	"EV_JUMP_FLESH",
	"EV_JUMP_FOLIAGE",
	"EV_JUMP_GLASS",
	"EV_JUMP_GRASS",
	"EV_JUMP_GRAVEL",
	"EV_JUMP_ICE",
	"EV_JUMP_METAL",
	"EV_JUMP_MUD",
	"EV_JUMP_PAPER",
	"EV_JUMP_PLASTER",
	"EV_JUMP_ROCK",
	"EV_JUMP_SAND",
	"EV_JUMP_SNOW",
	"EV_JUMP_WATER",
	"EV_JUMP_WOOD",
	"EV_JUMP_ASPHALT",
	"EV_LANDING_DEFAULT",
	"EV_LANDING_BARK",
	"EV_LANDING_BRICK",
	"EV_LANDING_CARPET",
	"EV_LANDING_CLOTH",
	"EV_LANDING_CONCRETE",
	"EV_LANDING_DIRT",
	"EV_LANDING_FLESH",
	"EV_LANDING_FOLIAGE",
	"EV_LANDING_GLASS",
	"EV_LANDING_GRASS",
	"EV_LANDING_GRAVEL",
	"EV_LANDING_ICE",
	"EV_LANDING_METAL",
	"EV_LANDING_MUD",
	"EV_LANDING_PAPER",
	"EV_LANDING_PLASTER",
	"EV_LANDING_ROCK",
	"EV_LANDING_SAND",
	"EV_LANDING_SNOW",
	"EV_LANDING_WATER",
	"EV_LANDING_WOOD",
	"EV_LANDING_ASPHALT",
	"EV_LANDING_PAIN_DEFAULT",
	"EV_LANDING_PAIN_BARK",
	"EV_LANDING_PAIN_BRICK",
	"EV_LANDING_PAIN_CARPET",
	"EV_LANDING_PAIN_CLOTH",
	"EV_LANDING_PAIN_CONCRETE",
	"EV_LANDING_PAIN_DIRT",
	"EV_LANDING_PAIN_FLESH",
	"EV_LANDING_PAIN_FOLIAGE",
	"EV_LANDING_PAIN_GLASS",
	"EV_LANDING_PAIN_GRASS",
	"EV_LANDING_PAIN_GRAVEL",
	"EV_LANDING_PAIN_ICE",
	"EV_LANDING_PAIN_METAL",
	"EV_LANDING_PAIN_MUD",
	"EV_LANDING_PAIN_PAPER",
	"EV_LANDING_PAIN_PLASTER",
	"EV_LANDING_PAIN_ROCK",
	"EV_LANDING_PAIN_SAND",
	"EV_LANDING_PAIN_SNOW",
	"EV_LANDING_PAIN_WATER",
	"EV_LANDING_PAIN_WOOD",
	"EV_LANDING_PAIN_ASPHALT",
	"EV_FOLIAGE_SOUND",
	"EV_STANCE_FORCE_STAND",
	"EV_STANCE_FORCE_CROUCH",
	"EV_STANCE_FORCE_PRONE",
	"EV_STEP_VIEW",
	"EV_WATER_TOUCH",
	"EV_WATER_LEAVE",
	"EV_ITEM_PICKUP",
	"EV_ITEM_PICKUP_QUIET",
	"EV_AMMO_PICKUP",
	"EV_NOAMMO",
	"EV_EMPTYCLIP",
	"EV_RELOAD",
	"EV_RELOAD_FROM_EMPTY",
	"EV_RELOAD_START",
	"EV_RELOAD_END",
	"EV_RAISE_WEAPON",
	"EV_PUTAWAY_WEAPON",
	"EV_WEAPON_ALT",
	"EV_PULLBACK_WEAPON",
	"EV_FIRE_WEAPON",
	"EV_FIRE_WEAPONB",
	"EV_FIRE_WEAPON_LASTSHOT",
	"EV_RECHAMBER_WEAPON",
	"EV_EJECT_BRASS",
	"EV_MELEE_SWIPE",
	"EV_FIRE_MELEE",
	"EV_MELEE_HIT",
	"EV_MELEE_MISS",
	"EV_FIRE_WEAPON_MG42",
	"EV_FIRE_QUADBARREL_1",
	"EV_FIRE_QUADBARREL_2",
	"EV_BULLET_TRACER",
	"EV_SOUND_ALIAS",
	"EV_BULLET_HIT_SMALL",
	"EV_BULLET_HIT_LARGE",
	"EV_BULLET_HIT_CLIENT_SMALL",
	"EV_BULLET_HIT_CLIENT_LARGE",
	"EV_GRENADE_BOUNCE",
	"EV_GRENADE_EXPLODE",
	"EV_ROCKET_EXPLODE",
	"EV_ROCKET_EXPLODE_NOMARKS",
	"EV_MOLOTOV_EXPLODE",
	"EV_MOLOTOV_EXPLODE_NOMARKS",
	"EV_CUSTOM_EXPLODE",
	"EV_CUSTOM_EXPLODE_NOMARKS",
	"EV_RAILTRAIL",
	"EV_BULLET",
	"EV_PAIN",
	"EV_CROUCH_PAIN",
	"EV_DEATH",
	"EV_DEBUG_LINE",
	"EV_PLAY_FX",
	"EV_PLAY_FX_DIR",
	"EV_PLAY_FX_ON_TAG",
	"EV_FLAMEBARREL_BOUNCE",
	"EV_EARTHQUAKE",
	"EV_DROPWEAPON",
	"EV_ITEM_RESPAWN",
	"EV_ITEM_POP",
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",
	"EV_OBITUARY"
};

/*
 * The events the client predicts for itself; BG_PlayerStateToEntityState never
 * copies them out of the playerState.  Indices into eventnames[] above:
 * EV_STANCE_FORCE_STAND, EV_STANCE_FORCE_CROUCH, EV_STANCE_FORCE_PRONE,
 * EV_STEP_VIEW, EV_BULLET_HIT_CLIENT_SMALL, EV_BULLET_HIT_CLIENT_LARGE.
 *
 * The retail build reaches the table through a .data pointer slot (0x2006CBE4)
 * that holds the .rdata address 0x20055EAC.
 */
static const int unpredictableEvents[] = { 140, 141, 142, 143, 175, 176, -1 };

/*
==============
BG_FindItemForWeapon
==============
*/
gitem_t *BG_FindItemForWeapon( int weaponIndex ) {
	if ( weaponIndex < 0 || weaponIndex > bg_numWeapons ) {
		Com_Error( ERR_DROP, "\x15" "BG_FindItemForWeapon: weapon out of range %i", weaponIndex );
	}

	return &bg_itemlist[weaponIndex];
}

/*
===============
BG_FindItem

  The weapon slots answer to the parsed weapon's internal name; everything past
  bg_numWeapons answers to its pickup name or its classname.
===============
*/
gitem_t *BG_FindItem( const char *pickupName ) {
	int i;
	gitem_t *it;

	for ( i = 1, it = bg_itemlist + 1; it < bg_itemlist + 70; i++, it++ ) {
		if ( i <= bg_numWeapons ) {
			if ( !Q_stricmp( bg_weaponInfo[i]->szInternalName, pickupName ) ) {
				return &bg_itemlist[i];
			}
			continue;
		}

		if ( it->pickup_name && !Q_stricmp( pickupName, it->pickup_name ) ) {
			return it;
		}
		if ( it->classname && !Q_stricmp( pickupName, it->classname ) ) {
			return it;
		}
	}

	return NULL;
}

/*
============
BG_PlayerTouchesItem

  Items can be picked up without actually touching their physical bounds to make
  grabbing them easier
============
*/
qboolean BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime ) {
	vec3_t origin;

	BG_EvaluateTrajectory( &item->pos, atTime, origin );

	/* we are ignoring ducked differences here */
	if ( ps->origin[0] - origin[0] > 36
		 || ps->origin[0] - origin[0] < -36
		 || ps->origin[1] - origin[1] > 36
		 || ps->origin[1] - origin[1] < -36
		 || ps->origin[2] - origin[2] > 18
		 || ps->origin[2] - origin[2] < -88 ) {
		return qfalse;
	}

	return qtrue;
}

/*
================
BG_CanItemBeGrabbed

  Returns false if the item should not be picked up.
================
*/
qboolean BG_CanItemBeGrabbed( const entityState_t *ent, const playerState_t *ps, qboolean onlyIfHave ) {
	gitem_t *item;
	int weaponIndex;

	if ( ent->index < 1 || ent->index >= 70 ) {
		Com_Error( ERR_DROP, "\x15" "BG_CanItemBeGrabbed: index out of range" );
	}

	item = &bg_itemlist[ent->index];

	if ( ent->clientNum == ps->clientNum ) {
		return qfalse;
	}

	switch ( item->giType ) {
	case IT_WEAPON:
		weaponIndex = item->giTag;
		if ( ps->weapons[weaponIndex / 32] & ( 1 << ( weaponIndex % 32 ) ) ) {
			/* already carried: only worth taking if there is room for its ammo */
			if ( BG_GetMaxPickupableAmmo( ps, weaponIndex ) <= 0 ) {
				return qfalse;
			}
		} else if ( onlyIfHave ) {
			return qfalse;
		}
		return qtrue;

	case IT_AMMO:
		weaponIndex = item->giTag;
		if ( ps->weapons[weaponIndex / 32] & ( 1 << ( weaponIndex % 32 ) ) ) {
			return BG_GetMaxPickupableAmmo( ps, weaponIndex ) > 0;
		}
		if ( bg_weaponInfo[weaponIndex]->clipOnly && BG_GetMaxPickupableAmmo( ps, weaponIndex ) > 0 ) {
			return qtrue;
		}
		return qfalse;

	case IT_HEALTH:
		return ps->stats[0] < ps->stats[2];

	case IT_BAD:
		Com_Error( ERR_DROP, "\x15" "BG_CanItemBeGrabbed: IT_BAD" );
		return qfalse;

	default:
		return qfalse;
	}
}

/*
================
BG_EvaluateTrajectory

  trType_t values, RTCW's enum unchanged:
    0 TR_STATIONARY   1 TR_INTERPOLATE  2 TR_LINEAR    3 TR_LINEAR_STOP
    4 TR_SINE         5 TR_GRAVITY      6 TR_GRAVITY_LOW
    7 TR_GRAVITY_FLOAT   8 TR_GRAVITY_PAUSED
    9 TR_ACCELERATE  10 TR_DECCELERATE
================
*/
void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
	float deltaTime;
	float phase;
	vec3_t v;

	switch ( tr->trType ) {
	case 0:     /* TR_STATIONARY */
	case 1:     /* TR_INTERPOLATE */
	case 8:     /* TR_GRAVITY_PAUSED -- stopped, but gravity still applies later */
		VectorCopy( tr->trBase, result );
		break;
	case 2:     /* TR_LINEAR */
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case 3:     /* TR_LINEAR_STOP */
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case 4:     /* TR_SINE */
		deltaTime = ( atTime - tr->trTime ) / (float)tr->trDuration;
		phase = sin( deltaTime * M_PI * 2 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
	case 5:     /* TR_GRAVITY */
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * 800 * deltaTime * deltaTime;             /* DEFAULT_GRAVITY */
		break;
	case 6:     /* TR_GRAVITY_LOW */
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * ( 800 * 0.3 ) * deltaTime * deltaTime;
		break;
	case 7:     /* TR_GRAVITY_FLOAT */
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * ( 800 * 0.2 ) * deltaTime;
		break;
	case 9:     /* TR_ACCELERATE -- trDelta is the ultimate speed */
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		phase = VectorLength( tr->trDelta ) / ( tr->trDuration * 0.001f );
		VectorNormalize2( tr->trDelta, v );
		deltaTime = deltaTime * deltaTime * 0.5 * phase;
		VectorMA( tr->trBase, deltaTime, v, result );
		break;
	case 10:    /* TR_DECCELERATE -- trDelta is the starting speed */
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		phase = VectorLength( tr->trDelta ) / ( tr->trDuration * 0.001f );
		VectorNormalize2( tr->trDelta, v );
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		deltaTime = deltaTime * deltaTime * 0.5 * -phase;
		VectorMA( result, deltaTime, v, result );
		break;
	default:
		/* RTCW prints trTime here, not trType; the retail build kept the bug */
		Com_Error( ERR_DROP, "\x15" "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime );
		break;
	}
}

/*
================
BG_EvaluateTrajectoryDelta

  For determining velocity at a given time.  TR_GRAVITY_PAUSED has no case
  here, so it drops into the error.
================
*/
void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
	float deltaTime;
	float phase;

	switch ( tr->trType ) {
	case 0:     /* TR_STATIONARY */
	case 1:     /* TR_INTERPOLATE */
		VectorClear( result );
		break;
	case 2:     /* TR_LINEAR */
		VectorCopy( tr->trDelta, result );
		break;
	case 4:     /* TR_SINE */
		deltaTime = ( atTime - tr->trTime ) / (float)tr->trDuration;
		phase = cos( deltaTime * M_PI * 2 );        /* derivative of sin = cos */
		phase *= 0.5;
		VectorScale( tr->trDelta, phase, result );
		break;
	case 3:     /* TR_LINEAR_STOP */
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		VectorCopy( tr->trDelta, result );
		break;
	case 5:     /* TR_GRAVITY */
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorCopy( tr->trDelta, result );
		result[2] -= 800 * deltaTime;
		break;
	case 6:     /* TR_GRAVITY_LOW */
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorCopy( tr->trDelta, result );
		result[2] -= ( 800 * 0.3 ) * deltaTime;
		break;
	case 7:     /* TR_GRAVITY_FLOAT */
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorCopy( tr->trDelta, result );
		result[2] -= ( 800 * 0.2 ) * deltaTime;
		break;
	case 9:     /* TR_ACCELERATE */
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		phase = deltaTime * deltaTime;
		VectorScale( tr->trDelta, phase, result );
		break;
	case 10:    /* TR_DECCELERATE */
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001f;
		VectorScale( tr->trDelta, deltaTime, result );
		break;
	default:
		Com_Error( ERR_DROP, "\x15" "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
		break;
	}
}

/*
================
BG_GetMarkDir

  Tip the impact direction away from the surface until it is at least a fixed
  angle off it, so a decal is never laid down edge on.
================
*/
void BG_GetMarkDir( const vec3_t dir, const vec3_t normal, vec3_t out ) {
	vec3_t ndir;
	vec3_t dirNorm;
	float minDot;

	minDot = 0.3f;

	if ( VectorLength( normal ) < 1.0 ) {
		VectorSet( ndir, 0, 0, 1 );
	} else {
		VectorCopy( normal, ndir );
	}

	VectorNegate( dir, dirNorm );
	VectorNormalize( dirNorm );

	if ( normal[2] > 0.8 ) {
		minDot = 0.7f;
	}

	while ( DotProduct( dirNorm, ndir ) < minDot ) {
		VectorMA( dirNorm, 0.5, ndir, dirNorm );
		VectorNormalize( dirNorm );
	}

	VectorCopy( dirNorm, out );
}

/*
================
BG_AddPredictableEventToPlayerstate

  Handles the sequence numbers

  __usercall: retail (0x20005800) takes newEvent in ecx and ps in eax with
  eventParm on the stack.  The parameter order is RTCW's (bg_misc.c:3855,
  bg_public.h:1397).
================
*/
void BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps ) {
	if ( !newEvent ) {
		return;
	}

	/* both halves go through a byte: the retail code zero-extends from the low
	 * byte before the dword store into the int arrays. */
	ps->events[ps->eventSequence & 3] = (byte)newEvent;
	ps->eventParms[ps->eventSequence & 3] = (byte)eventParm;
	ps->eventSequence++;
}

/*
========================
BG_PlayerStateToEntityState

  This is done after each set of usercmd_t on the server, and after local
  prediction on the client.

  eType 1/7 and the eFlags bits are spelled numerically: neither enum has been
  recovered into g_public.h yet.  0x0001 is the dead flag, 0x0200 the crouching
  flag, 0xC000 the mounted/viewlocked pair.

  __usercall: the call sites pass snap in eax, s in esi and ps in edi.  The
  parameter order below is RTCW's (bg_misc.c:3884, bg_public.h:1401), declared
  once in bg_public.h.
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap ) {
	int i;
	int movementDir;
	const int *ev;
	float frac;

	s->eType = ( ps->pm_flags & 0x50000 ) ? 1 : 7;
#ifdef CGAMEDLL
	/* Only the cgame copy stores the number (0x30005AEB movzx word; the game
	   copy 0x20005840 never writes s+0).  cg.predictedPlayerEntity is built
	   from nothing else, and CG_Player hides the local model only when
	   es->number == ps.clientNum -- without this every client but 0 sees
	   its own body from the inside. */
	s->number = (unsigned short)ps->clientNum;
#endif

	s->pos.trType = 1;              /* TR_INTERPOLATE */
	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) {
		/* SnapVector */
		s->pos.trBase[0] = (float)(int)s->pos.trBase[0];
		s->pos.trBase[1] = (float)(int)s->pos.trBase[1];
		s->pos.trBase[2] = (float)(int)s->pos.trBase[2];
	}

	s->apos.trType = 1;             /* TR_INTERPOLATE */
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) {
		s->apos.trBase[0] = (float)(int)s->apos.trBase[0];
		s->apos.trBase[1] = (float)(int)s->apos.trBase[1];
		s->apos.trBase[2] = (float)(int)s->apos.trBase[2];
	}

	movementDir = ps->movementDir;
	if ( movementDir > 128 ) {
		movementDir -= 256;
	}
	s->angles2[1] = movementDir;    /* YAW */

	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;

	s->eFlags = ps->eFlags;
	if ( ps->eFlags & 0xC000 ) {
		s->otherEntityNum = ps->viewlocked_entNum;
	}
	if ( ps->pm_type >= PM_DEAD ) {
		s->eFlags |= 0x1;
	} else {
		s->eFlags &= ~0x1;
	}
	if ( ps->pm_flags & 0x20 ) {
		s->eFlags |= 0x200;
	} else {
		s->eFlags &= ~0x200;
	}

	s->leanf = ps->leanf;

	if ( PM_GetEffectiveStance( ps ) == 1 ) {       /* prone */
		if ( ps->viewHeightLerpTime ) {
			frac = (float)( ps->commandTime - ps->viewHeightLerpTime )
				   / PM_GetViewHeightLerpTime( ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown );
			if ( frac < 0.0 ) {
				frac = 0.0;
			} else if ( frac > 1.0 ) {
				frac = 1.0;
			}
			if ( !ps->viewHeightLerpDown ) {
				frac = 1.0 - frac;
			}
		} else {
			frac = 1.0;
		}
		s->fTorsoHeight = frac * ps->fTorsoHeight;
		s->fTorsoPitch = frac * AngleNormalize180( ps->fTorsoPitch );
		s->fWaistPitch = frac * AngleNormalize180( ps->fWaistPitch );
	} else {
		s->fTorsoHeight = 0;
		s->fTorsoPitch = 0;
		s->fWaistPitch = 0;
	}

	if ( ps->entityEventSequence < ps->eventSequence ) {
		if ( ps->eventSequence - ps->entityEventSequence > 4 ) {
			ps->entityEventSequence = ps->eventSequence - 4;
		}
		s->eventParm = (byte)ps->eventParms[ps->entityEventSequence++ & 3];
	} else {
		s->eventParm = 0;
	}

	for ( i = ps->oldEventSequence; i != ps->eventSequence; i++ ) {
		for ( ev = unpredictableEvents; *ev > 0; ev++ ) {
			if ( *ev == (byte)ps->events[i & 3] ) {
				break;
			}
		}
		if ( *ev < 0 ) {
			s->events[s->eventSequence & 3] = (byte)ps->events[i & 3];
			s->eventParms[s->eventSequence++ & 3] = (byte)ps->eventParms[i & 3];
		}
	}
	ps->oldEventSequence = ps->eventSequence;

	s->weapon = (byte)ps->weapon;
	s->groundEntityNum = (unsigned short)ps->groundEntityNum;
}

/*
========================
BG_PlayerStateToEntityStateExtrapolate

  This is done after each set of usercmd_t on the server, and after local
  prediction on the client

  __usercall here too (s in edi, ps in esi), so the order is RTCW's
  BG_PlayerStateToEntityStateExtraPolate, not the register order.
========================
*/
void BG_PlayerStateToEntityStateExtrapolate( playerState_t *ps, entityState_t *s, int time, qboolean snap ) {
	int i;
	const int *ev;
	int lerpTime;
	float frac;

#ifdef CGAMEDLL
	s->number = (unsigned short)ps->clientNum;      /* 0x30005E30, cgame only */
#endif
	s->pos.trType = 3;              /* TR_LINEAR_STOP */
	VectorCopy( ps->origin, s->pos.trBase );
	VectorCopy( ps->velocity, s->pos.trDelta );
	s->pos.trTime = time;
	s->pos.trDuration = 50;         /* 1000 / sv_fps */

	s->apos.trType = 1;             /* TR_INTERPOLATE */
	VectorCopy( ps->viewangles, s->apos.trBase );

	s->angles2[1] = ps->movementDir;        /* YAW; not folded to +-128 here */
	s->eFlags = ps->eFlags;

	if ( ps->entityEventSequence < ps->eventSequence ) {
		if ( ps->eventSequence - ps->entityEventSequence > 4 ) {
			ps->entityEventSequence = ps->eventSequence - 4;
		}
		s->eventParm = (byte)ps->eventParms[ps->entityEventSequence++ & 3];
	} else {
		s->eventParm = 0;
	}

	if ( ps->oldEventSequence > ps->eventSequence ) {
		ps->oldEventSequence = ps->eventSequence;
	}
	for ( i = ps->oldEventSequence; i != ps->eventSequence; i++ ) {
		for ( ev = unpredictableEvents; *ev > 0; ev++ ) {
			if ( *ev == (byte)ps->events[i & 3] ) {
				break;
			}
		}
		if ( *ev < 0 ) {
			s->events[s->eventSequence & 3] = (byte)ps->events[i & 3];
			s->eventParms[s->eventSequence++ & 3] = (byte)ps->eventParms[i & 3];
		}
	}
	ps->oldEventSequence = ps->eventSequence;

	s->weapon = (byte)ps->weapon;
	s->groundEntityNum = (unsigned short)ps->groundEntityNum;

	s->eType = ( ps->pm_flags & 0x50000 ) ? 1 : 7;

	if ( snap ) {
		s->pos.trBase[0] = (float)(int)s->pos.trBase[0];
		s->pos.trBase[1] = (float)(int)s->pos.trBase[1];
		s->pos.trBase[2] = (float)(int)s->pos.trBase[2];
		s->apos.trBase[0] = (float)(int)s->apos.trBase[0];
		s->apos.trBase[1] = (float)(int)s->apos.trBase[1];
		s->apos.trBase[2] = (float)(int)s->apos.trBase[2];
	}

	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;

	if ( ps->eFlags & 0xC000 ) {
		s->otherEntityNum = ps->viewlocked_entNum;
	}
	if ( ps->pm_type >= PM_DEAD ) {
		s->eFlags |= 0x1;
	} else {
		s->eFlags &= ~0x1;
	}
	if ( ps->pm_flags & 0x20 ) {
		s->eFlags |= 0x200;
	} else {
		s->eFlags &= ~0x200;
	}

	s->leanf = ps->leanf;

	/* PM_GetEffectiveStance is inlined by the retail build; this is its
	 * "not prone, and not lerping into or out of prone" early out. */
	if ( !( ps->pm_flags & 0x1 ) ) {
		if ( ps->viewHeightLerpTarget != ps->proneViewHeight
			 && ( !ps->viewHeightLerpTime
				  || ps->viewHeightLerpTarget != ps->crouchViewHeight
				  || ps->viewHeightLerpDown ) ) {
			s->fTorsoHeight = 0;
			s->fTorsoPitch = 0;
			s->fWaistPitch = 0;
			return;
		}
	}

	/* PM_GetViewHeightLerpTime, also inlined */
	lerpTime = PM_GetViewHeightLerpTime( ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown );

	frac = (float)( ps->commandTime - ps->viewHeightLerpTime ) / (float)lerpTime;
	if ( frac < 0.0 ) {
		frac = 0.0;
	} else if ( frac > 1.0 ) {
		frac = 1.0;
	}
	if ( !ps->viewHeightLerpDown ) {
		frac = 1.0 - frac;
	}

	s->fTorsoHeight = frac * ps->fTorsoHeight;
	s->fTorsoPitch = frac * AngleNormalize180( ps->fTorsoPitch );
	s->fWaistPitch = frac * AngleNormalize180( ps->fWaistPitch );
}

/*
========================
BG_CheckProneValid

  Test a proposed prone body -- a 60 unit long, 6 unit thick capsule laid along
  yaw at `origin` -- against the world through the caller's two trace callbacks,
  and report the ground it would lie on.

  The body is sampled at three points: the rear (the origin), the middle (24
  units forward) and the front.  Each one is dropped onto the ground; the three
  hits must be connected, must not fold too sharply, and the front of the body
  must have room.  On success groundOffset/pitchDown/pitchUp describe the pose.

  allowFallback is the caller's "the player is standing on something" flag: with
  it clear the ground sampling is skipped entirely and only the forward body
  trace has to pass, which is how a prone player in mid-air is judged.

  0x20006010.  height, and the parameters after it, are modified in place the
  way the retail code does.  The traces run through the caller's callbacks
  rather than pm->trace so that cgame can call this during prediction.
========================
*/
int BG_CheckProneValid( int clientNum, const vec3_t origin, float radius, float height,
						float yaw, float *groundOffset, float *pitchDown, float *pitchUp,
						qboolean skipInitialTrace, qboolean allowFallback,
						const vec3_t groundNormal,
						void ( *traceFunc )( trace_t *, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int ),
						void ( *traceDownFunc )( trace_t *, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int ),
						qboolean useAltContentMask ) {
	trace_t trace;
	vec3_t mins, maxs, start, end;
	vec3_t angles, forward, right, up;
	vec3_t frontHit, middleHit, rearHit;
	vec3_t delta;
	float frontClearance;
	float minForwardClearance;
	float groundTraceDepth;
	float traceDist;
	float slopeDrop;
	float rearPitch;
	float pitchDelta;
	qboolean blockedForward;
	int contentMask;

	blockedForward = qfalse;

	if ( g_debugProneCheck.integer ) {
		/* the whole proposed prone volume, before anything is tested */
		mins[0] = origin[0] - radius;
		mins[1] = origin[1] - radius;
		mins[2] = origin[2];
		maxs[0] = origin[0] + radius;
		maxs[1] = origin[1] + radius;
		maxs[2] = origin[2] + height;
		G_DebugBox( mins, maxs, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}

	/* MASK_PLAYERSOLID without the body and the two step bits; the alternate
	 * mask swaps 0x10000 for 0x20000.  Neither has a name in the headers yet. */
	contentMask = useAltContentMask ? 0x820011 : 0x810011;

	if ( !skipInitialTrace ) {
		mins[0] = -radius;
		mins[1] = -radius;
		mins[2] = 0;
		maxs[0] = radius;
		maxs[1] = radius;
		maxs[2] = height;
		VectorCopy( origin, start );
		end[0] = origin[0];
		end[1] = origin[1];
		end[2] = origin[2] + 10;
		traceDownFunc( &trace, start, mins, maxs, end, clientNum, contentMask );
		if ( trace.allsolid ) {
			return qfalse;
		}
	}

	/* a supplied fallback normal has to be a walkable plane */
	if ( allowFallback && groundNormal && groundNormal[2] < 0.7f ) {
		return qfalse;
	}

	VectorSet( mins, -6, -6, -6 );
	VectorSet( maxs, 6, 6, 6 );
	angles[0] = 0;
	angles[1] = yaw - 180;
	angles[2] = 0;
	AngleVectors( angles, forward, right, up );
	height -= 6;

	/* the body itself: 60 units long, less the 6 unit trace radius at each end */
	start[0] = origin[0];
	start[1] = origin[1];
	start[2] = origin[2] + height;
	VectorMA( start, 54, forward, end );
	traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );

	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( right, start, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( up, start, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}

	if ( trace.fraction < 1.0f ) {
		if ( !allowFallback ) {
			if ( g_debugProneCheck.integer ) {
				trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
			}
			return qfalse;
		}

		blockedForward = qtrue;
		frontClearance = trace.fraction * 54 + 6;
		if ( frontClearance < radius + 2 ) {
			if ( g_debugProneCheck.integer ) {
				trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
			}
			return qfalse;
		}

		minForwardClearance = height * 0.7f + 24.0f;
		if ( frontClearance < minForwardClearance ) {
			/* not enough room straight ahead: try again with the far end lifted */
			if ( g_debugProneCheck.integer ) {
				trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
			}
			blockedForward = qfalse;
			end[2] = end[2] + 22;
			delta[0] = end[0] - start[0];
			delta[1] = end[1] - start[1];
			delta[2] = end[2] - start[2];
			traceDist = VectorNormalize2( delta, forward );
			traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );

			if ( trace.fraction < 1.0f ) {
				blockedForward = qtrue;
				frontClearance = trace.fraction * traceDist + 6;
				if ( frontClearance < minForwardClearance ) {
					if ( g_debugProneCheck.integer ) {
						trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
					}
					return qfalse;
				}
				if ( g_debugProneCheck.integer ) {
					trap_AddDebugLine( start, trace.endpos, colorYellow, g_debugProneCheckDepthCheck.integer, 1 );
				}
			} else {
				if ( g_debugProneCheck.integer ) {
					trap_AddDebugLine( start, trace.endpos, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
				}
				frontClearance = 60;
			}
		} else {
			if ( g_debugProneCheck.integer ) {
				trap_AddDebugLine( start, trace.endpos, colorYellow, g_debugProneCheckDepthCheck.integer, 1 );
			}
		}
	} else {
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( start, trace.endpos, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
		}
		frontClearance = 60;
	}

	VectorCopy( trace.endpos, frontHit );

	if ( !allowFallback ) {
		/* off the ground: the body fits, and there is no ground to describe */
		if ( groundOffset ) {
			*groundOffset = 0;
		}
		if ( pitchDown ) {
			*pitchDown = 0;
		}
		if ( pitchUp ) {
			*pitchUp = 0;
		}
		return qtrue;
	}

	/* middle sample, 24 units along the facing direction */
	start[0] = origin[0] + forward[0] * 24;
	start[1] = origin[1] + forward[1] * 24;
	start[2] = origin[2] + forward[2] * 24 + height;
	end[0] = start[0];
	end[1] = start[1];
	groundTraceDepth = radius * 2.5f + height - 6;
	end[2] = start[2] - groundTraceDepth;
	traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );
	if ( trace.fraction == 1.0f ) {
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
		}
		return qfalse;
	}
	if ( g_debugProneCheck.integer ) {
		trap_AddDebugLine( start, trace.endpos, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
	}
	VectorCopy( trace.endpos, middleHit );

	if ( blockedForward ) {
		/* the forward trace was cut short, so the front of the body is resting
		 * on whatever stopped it: reject too steep a rise and re-aim the body
		 * along the line that actually connects the two. */
		slopeDrop = groundTraceDepth * trace.fraction + 6;
		if ( frontClearance - slopeDrop < slopeDrop * -0.75f ) {
			if ( g_debugProneCheck.integer ) {
				trap_AddDebugLine( frontHit, middleHit, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
			}
			return qfalse;
		}
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( frontHit, middleHit, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
		}

		delta[0] = frontHit[0] - middleHit[0] + forward[0] * 6;
		delta[1] = frontHit[1] - middleHit[1] + forward[1] * 6;
		delta[2] = frontHit[2] - middleHit[2] + forward[2] * 6 + 6;
		VectorNormalize( delta );

		/* halfway between where the body wanted to end and where the connector
		 * reaches; 30 is the 54 unit body less the 24 unit middle offset */
		end[2] = start[2] + delta[2] * 30;
		end[0] = ( origin[0] + forward[0] * 54 + ( start[0] + delta[0] * 30 ) ) * 0.5f;
		end[1] = ( origin[1] + forward[1] * 54 + ( start[1] + delta[1] * 30 ) ) * 0.5f;
		traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );
		if ( trace.fraction < 1.0f ) {
			if ( g_debugProneCheck.integer ) {
				trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
			}
			return qfalse;
		}
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( start, trace.endpos, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
		}
		VectorCopy( trace.endpos, frontHit );
	}

	/* the ground under the front of the body */
	VectorCopy( frontHit, start );
	end[0] = frontHit[0];
	end[1] = frontHit[1];
	end[2] = frontHit[2] - ( ( frontHit[2] - middleHit[2] ) * 2 + radius );
	traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );
	if ( trace.fraction == 1.0f ) {
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
		}
		return qfalse;
	}
	if ( g_debugProneCheck.integer ) {
		trap_AddDebugLine( start, trace.endpos, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
	}
	VectorCopy( trace.endpos, frontHit );

	/* and the ground under the rear */
	start[0] = origin[0];
	start[1] = origin[1];
	start[2] = origin[2] + height;
	end[0] = origin[0];
	end[1] = origin[1];
	end[2] = origin[2] - radius * 1.5f;
	traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );
	if ( trace.fraction == 1.0f ) {
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( start, trace.endpos, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
		}
		return qfalse;
	}
	if ( g_debugProneCheck.integer ) {
		trap_AddDebugLine( start, trace.endpos, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
	}
	VectorCopy( trace.endpos, rearHit );

	/* the body may not fold more than this between its two halves */
	delta[0] = middleHit[0] - rearHit[0];
	delta[1] = middleHit[1] - rearHit[1];
	delta[2] = middleHit[2] - rearHit[2];
	rearPitch = vectopitch( delta );

	delta[0] = frontHit[0] - middleHit[0];
	delta[1] = frontHit[1] - middleHit[1];
	delta[2] = frontHit[2] - middleHit[2];
	pitchDelta = AngleSubtract( vectopitch( delta ), rearPitch );

	if ( pitchDelta < -50 || pitchDelta > 70 ) {
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( rearHit, middleHit, colorMagenta, g_debugProneCheckDepthCheck.integer, 1 );
		}
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( middleHit, frontHit, colorMagenta, g_debugProneCheckDepthCheck.integer, 1 );
		}
		return qfalse;
	}

	/* point traces along the body, five units above the ground it found */
	start[0] = rearHit[0];
	start[1] = rearHit[1];
	start[2] = rearHit[2] + 5;
	end[0] = middleHit[0];
	end[1] = middleHit[1];
	end[2] = middleHit[2] + 5;
	VectorClear( mins );
	VectorClear( maxs );
	traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );
	if ( trace.fraction < 1.0f ) {
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( start, end, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
		}
		return qfalse;
	}
	if ( g_debugProneCheck.integer ) {
		trap_AddDebugLine( start, end, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
	}

	VectorCopy( end, start );
	end[0] = frontHit[0];
	end[1] = frontHit[1];
	end[2] = frontHit[2] + 5;
	traceFunc( &trace, start, mins, maxs, end, clientNum, contentMask );
	if ( trace.fraction < 1.0f ) {
		if ( g_debugProneCheck.integer ) {
			trap_AddDebugLine( start, end, colorRed, g_debugProneCheckDepthCheck.integer, 1 );
		}
		return qfalse;
	}
	if ( g_debugProneCheck.integer ) {
		trap_AddDebugLine( start, end, colorGreen, g_debugProneCheckDepthCheck.integer, 1 );
	}

	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( right, rearHit, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( up, rearHit, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( right, middleHit, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( up, middleHit, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( right, frontHit, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		G_DebugCircleEx( up, frontHit, 6, colorMdCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		trap_AddDebugLine( rearHit, middleHit, colorCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}
	if ( g_debugProneCheck.integer ) {
		trap_AddDebugLine( middleHit, frontHit, colorCyan, g_debugProneCheckDepthCheck.integer, 1 );
	}

	if ( groundOffset ) {
		*groundOffset = rearHit[2] - origin[2] - 6;
	}
	if ( pitchDown ) {
		delta[0] = rearHit[0] - middleHit[0];
		delta[1] = rearHit[1] - middleHit[1];
		delta[2] = rearHit[2] - middleHit[2];
		*pitchDown = AngleNormalize180( vectopitch( delta ) );
	}
	if ( pitchUp ) {
		delta[0] = middleHit[0] - frontHit[0];
		delta[1] = middleHit[1] - frontHit[1];
		delta[2] = middleHit[2] - frontHit[2];
		*pitchUp = AngleNormalize180( vectopitch( delta ) );
	}

	return qtrue;
}

/*
========================
BG_CheckProne

  0x20006FE0.  Five bytes: the retail build turned the whole body into a tail
  jump to BG_CheckProneValid, so nothing but the name survives.
========================
*/
int BG_CheckProne( int clientNum, const vec3_t origin, float radius, float height,
				   float yaw, float *groundOffset, float *pitchDown, float *pitchUp,
				   qboolean skipInitialTrace, qboolean allowFallback,
				   const vec3_t groundNormal,
				   void ( *traceFunc )( trace_t *, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int ),
				   void ( *traceDownFunc )( trace_t *, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int ),
				   qboolean useAltContentMask ) {
	return BG_CheckProneValid( clientNum, origin, radius, height, yaw, groundOffset, pitchDown,
							   pitchUp, skipInitialTrace, allowFallback, groundNormal,
							   traceFunc, traceDownFunc, useAltContentMask );
}
