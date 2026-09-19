/*
 * cg_weapons_mp.c -- events and effects dealing with weapons.
 *
 * cgame/cg_weapons.c of cgame_mp_x86.dll (CoD 1.1 multiplayer, imagebase
 * 0x30000000).  49 functions, 0x300341B0 .. 0x30039750, in address order.
 *
 * RTCW's cgame/cg_weapons.c is the ancestor of CG_RegisterItemVisuals,
 * CG_RegisterItems, CG_RailTrail, CG_CalculateWeaponPosition, CG_AddViewWeapon,
 * CG_AddPlayerWeapon, CG_DrawWeaponSelect, CG_WeaponSelectable,
 * CG_FinishWeaponChange, CG_NextWeapon_f / CG_PrevWeapon_f / CG_Weapon_f,
 * CG_FireWeapon and the tracer family.  Everything else is CoD's own:
 *
 *   - the viewmodel is DObj + XAnim, not a frame/oldframe model, so
 *     CG_RegisterWeapon builds an anim tree of twenty animations and a
 *     two-part DObj, and CG_StartWeaponAnim / CG_PlayADSAnim /
 *     CG_WeaponRunXModelAnims drive it by goal weight instead of by frame;
 *   - weapon selection is CoD's six-slot model (CG_CycleWeap,
 *     CG_SelectFirstWeaponInSlot / NotInSlot, CG_WeaponSlot_f,
 *     CG_AltWeapon_f), not Q3's thirteen-bit weapon mask;
 *   - ADS, sway and recoil have no RTCW ancestor at all.  The heavy lifting
 *     lives in the shared game/bg_weapon.c -- this unit only assembles the
 *     pmWeaponAngleState_t, calls BG_CalculateWeaponAngles /
 *     BG_CalculateWeaponPosition_Sway and writes the results back into cg.
 *
 * TWO WEAPON RECORDS, DO NOT CONFLATE THEM.
 *   weaponInfo_t   1052 bytes, bg_public.h, the parsed weapon file.  cgame
 *                  reaches it through bg_weaponInfo[weapon], the pointer
 *                  table at 0x300EEF3C (recovered symbol misnamed
 *                  `cg_weapons`), with bg_numWeapons beside it at 0x300EEF38
 *                  (CG_RegisterWeapon 0x30034D19/0x30034D2E).
 *   cgWeaponInfo_t 408 bytes, cg_local.h, cgame's own per-weapon record.  That
 *                  is cg_weapons[64] at 0x301A6940, and it is this unit that
 *                  fills it in (CG_RegisterWeapon `imul ebp, 198h` 0x30034D21).
 *
 * THE HOLES.  cgWeaponInfo_t, cg_t, cgs_t and refEntity_t are all declared
 * with unnamed ranges in the headers.  Fields inside those ranges are reached
 * through the overlays and macros below, each carrying the offset and a
 * reference address.
 *
 * @fidelity: partial
 */

#include "cg_local.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Declarations no header carries. */

/* universal/com_math.c and universal/q_shared.c */
vec_t           VectorNormalize( vec3_t v );
/* DObjSkel2MatrixMultiply43's register mapping is eax = mat43, ecx = boneMatrix,
   edx = out (CG_AddPlayerWeapon 0x30037137). */

/* game/bg_misc.c and game/bg_weapon.c.  bg_public.h declares neither the two
   angle solvers nor the two name lookups. */
void            BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void            BG_CalculateWeaponAngles( pmWeaponAngleState_t *state, vec3_t angles );
void            BG_CalculateWeaponPosition_Sway( vec3_t swayOffsets, vec3_t swayAngles,
												 const playerState_t *ps,
												 vec3_t previousViewAngles, float scale, int msec );
int             BG_GetWeaponIndexForName( const char *name );
int             BG_GetWeaponSlotForName( const char *name );

/* Com_Error level 7, the one the localization failures below use
   (CG_RegisterWeapon 0x30035D1B).  q_shared.h's errorParm_t stops at 5. */
#define ERR_LOCALIZATION    7

/* cg_main_mp.c -- see the note in cg_local.h; the binary returns the length. */
int             CG_PlaySoundAliasByName( const char *name, int entnum, const vec3_t origin );

/* cg_localents_mp.c */
localEntity_t  *CG_AllocLocalEntity( void );

/* cg_draw_mp.c */
qboolean        CG_GetWeapReticleZoom( float *zoom );

/* cg_newDraw_mp.c / ui_shared.c */
void            Controls_GetConfig( void );
const char     *BindingFromName( const char *name );

/*
=============================================================================

	THE HOLES

=============================================================================
*/


/*
 * cg_items[] -- this unit's per-item record, 36 bytes (0x301DC3E0,
 * `lea ebp,[eax+eax*8]` in CG_RegisterItemVisuals at 0x30036082).  cg_ent_mp.c
 * already carries an extern for it under this name.
 */
typedef struct cgItemInfo_t {
	qboolean        registered;             /* +0x00 CG_RegisterItemVisuals 0x30036085 */
	qhandle_t       models[MAX_ITEM_MODELS];/* +0x04 0x300360E2 */
	qhandle_t       icon;                   /* +0x0C 0x30036103 */
	byte            unknown_0x10[12];
	const char     *pickupSound;            /* +0x1C 0x3003611F */
	const char     *ammoPickupSound;        /* +0x20 CG_RegisterWeapon 0x30035B4B */
} cgItemInfo_t;
CG_ASSERT_SIZE( cgItemInfo_t, 36 );

/*
 * The three DObjModel_t records CG_RegisterWeapon and CG_ChangeViewmodelDobj
 * hand to Com_ClientDObjCreate (trap 164).  12 bytes: the XModel pointer
 * trap_R_GetXModelByHandle returns, the tag the part hangs off, and the
 * qhandle_t the part was registered under (stored as a word, 0x30035181).
 */
typedef struct cgDObjModel_t {
	void           *xmodel;                 /* +0x00 */
	const char     *tagName;                /* +0x04 */
	short           hModel;                 /* +0x08 */
	short           pad;                    /* +0x0A */
} cgDObjModel_t;
CG_ASSERT_SIZE( cgDObjModel_t, 12 );

/*
 * refEntity_t.  Five offsets this unit stores into that land in cg_public.h's
 * unknown_ ranges.
 */
#define RE_LIGHTINGORIGIN( e )  ( (float *)&( e )->unknown_0x08[4] )        /* +0x0C CG_AddPlayerWeapon 0x30036D96 */
#define RE_UNKNOWN_0x18( e )    ( *(int *)&( e )->unknown_0x08[0x10] )      /* +0x18 CG_AddPlayerWeapon 0x30036DB3 */
#define RE_CUSTOMSHADER( e )    ( *(qhandle_t *)&( e )->unknown_0x60[8] )    /* +0x68 CG_RailTrail2 0x3003482D */
#define RE_SHADERTIME( e )      ( *(float *)&( e )->unknown_0x70[8] )        /* +0x78 CG_RailTrail2 0x30034821 */
#define RE_RADIUS( e )          ( *(float *)&( e )->unknown_0x70[0x0C] )     /* +0x7C CG_WaterRipple 0x30038DA7 */
#define RE_DOBJ( e )            ( *(int *)&( e )->unknown_0x70[0x20] )       /* +0x90 CG_AddPlayerWeapon 0x30036DE7 */

#define RT_DOBJ                 1       /* CG_AddPlayerWeapon 0x30036E27 */
#define RT_SPRITE_LOCAL         5       /* CG_WaterRipple 0x30038D72 */
#define RT_RAIL_CORE            7       /* CG_RailTrail2 0x3003480C */

#define RF_VIEWMODEL            0x0C    /* CG_AddViewWeapon 0x300375A2 */
#define RF_PLAYERMODEL          0x8C    /* CG_AddPlayerWeapon 0x30036E2F */

/*
 * localEntity_t.  cg_local.h carries the record from leType through lifeRate
 * and then one 212-byte hole; the hole is a vec4 of RGBA at +0x3C and a whole
 * refEntity_t at +0x50 (0x50 + 156 == 236, the record size exactly).
 */
#define LE_COLOR( le )          ( (float *)&( le )->unknown_0x018[0x24] )        /* +0x3C CG_RailTrail2 0x30034867 */
#define LE_REFENTITY( le )      ( (refEntity_t *)&( le )->unknown_0x018[0x38] )  /* +0x50 CG_RailTrail2 0x3003480C */

#define LE_FADE_RGB             0       /* CG_RailTrail2 0x300347DB */
#define LE_SCALE_FADE           1       /* CG_WaterRipple 0x30038D2E */
#define LE_MOVING_TRACER        2       /* CG_SpawnTracer 0x30039022 */

/*
 * localEntity_t's moving-tracer payload, the fields CG_SpawnTracer fills in
 * (0x3003906C..0x300390A9) that sit inside the same hole.
 */
#define LE_TRACERFLAGS( le )    ( *(int *)&( le )->unknown_0x018[0x00] )     /* +0x18 0x30039075 */
#define LE_TRACERTIME( le )     ( *(int *)&( le )->unknown_0x018[0x04] )     /* +0x1C 0x3003906C */
#define LE_TRACERORIGIN( le )   ( (float *)&( le )->unknown_0x018[0x0C] )    /* +0x24 0x3003906F */
#define LE_TRACERVELOCITY( le ) ( (float *)&( le )->unknown_0x018[0x18] )    /* +0x30 0x3003908F */

/*
 * cg_t.  Everything this unit touches that cg_local.h spells as a hole.
 *
 * cg.unknown_0x27314[48] is the ADS / weapon-position lerp block CG_Respawn
 * clears (0x30028B2C).
 */
#define CG_ADSLASTFRAC()        ( *(float *)&cg.unknown_0x27314[0x00] )      /* cg+0x27314 0x30036C6D */
#define CG_ADSGOINGUP()         ( *(int *)&cg.unknown_0x27314[0x04] )        /* cg+0x27318 0x30036C63 */
#define CG_WEAPONPOSBASE()      ( (float *)&cg.unknown_0x27314[0x08] )       /* cg+0x2731C 0x30036805 */
#define CG_IDLESCALE()          ( *(float *)&cg.unknown_0x27314[0x14] )      /* cg+0x27328 0x30037552 */
#define CG_POSTRACK()           ( (float *)&cg.unknown_0x27314[0x18] )       /* cg+0x2732C 0x30036497 */
#define CG_MOVEOFFSET()         ( (float *)&cg.unknown_0x27314[0x24] )       /* cg+0x27338 0x30037535 */

/* cg.unknown_0x27498[36] is exactly BG_CalculateWeaponPosition_Sway's three
   out-parameters, in the order it takes them (0x30036276..0x30036290). */
#define CG_PREVVIEWANGLES()     ( (float *)&cg.unknown_0x27498[0x00] )       /* cg+0x27498 */
#define CG_SWAYANGLES()         ( (float *)&cg.unknown_0x27498[0x0C] )       /* cg+0x274A4 */
#define CG_SWAYOFFSETS()        ( (float *)&cg.unknown_0x27498[0x18] )       /* cg+0x274B0 */

/* cg.unknown_0x2A7EC[24] is six per-slot fade fractions, indexed by weapon slot
   (CG_DrawWeaponSelect walks 0x3020C940 down to 0x3020C92C, 0x30037ED6). */
#define CG_SLOTFADE()           ( (float *)cg.unknown_0x2A7EC )              /* cg+0x2A7EC */
/* cg+0x2A804 is the cg.time of the previous CG_DrawWeaponSelect (0x300377A0). */
#define CG_WEAPONSELECTDRAWTIME()   ( cg.unknown_0x2A804 )

/* cg.unknown_0x2A8AC[8] -- RTCW's bobfracsin / xyspeed pair. */
#define CG_XYSPEED()            ( *(float *)&cg.unknown_0x2A8AC[4] )         /* cg+0x2A8B0 0x300362F8 */

/* cg.unknown_0x2A994 gates the whole of CG_AddViewWeapon (0x3003725E). */
#define CG_NOVIEWWEAPON()       ( cg.unknown_0x2A994 )

/*
 * cg.unknown_0x2AA40[96] -- the view-model placement block.  The first two are
 * the angles CG_CalcCrosshairPosition reads back (0x30015D73); then the ADS
 * world offset, the recoil state BG_CalculateWeaponAngles owns, and the tag
 * origin/axis CG_DObjGetViewModelTagMatrix reads (0x3001C40A).
 */
#define CG_GUNPITCH()           ( *(float *)&cg.unknown_0x2AA40[0x00] )      /* cg+0x2AA40 0x300374F8 */
#define CG_GUNYAW()             ( *(float *)&cg.unknown_0x2AA40[0x04] )      /* cg+0x2AA44 0x30037508 */
#define CG_ADSWORLDOFFSET()     ( (float *)&cg.unknown_0x2AA40[0x08] )       /* cg+0x2AA48 0x30036935 */
#define CG_RECOILPITCH()        ( *(float *)&cg.unknown_0x2AA40[0x14] )      /* cg+0x2AA54 0x3003755C */
#define CG_RECOILYAW()          ( *(float *)&cg.unknown_0x2AA40[0x18] )      /* cg+0x2AA58 */
#define CG_RECOILROLL()         ( *(float *)&cg.unknown_0x2AA40[0x1C] )      /* cg+0x2AA5C */
#define CG_RECOILPITCHSPEED()   ( *(float *)&cg.unknown_0x2AA40[0x20] )      /* cg+0x2AA60 0x30038A05 */
#define CG_RECOILYAWSPEED()     ( *(float *)&cg.unknown_0x2AA40[0x24] )      /* cg+0x2AA64 0x30038A16 */
#define CG_RECOILSTATE()        ( *(int *)&cg.unknown_0x2AA40[0x28] )        /* cg+0x2AA68 */
#define CG_VIEWMODELORIGIN()    ( (float *)&cg.unknown_0x2AA40[0x2C] )       /* cg+0x2AA6C 0x30037014 */
#define CG_VIEWMODELAXIS()      ( (vec3_t *)&cg.unknown_0x2AA40[0x38] )      /* cg+0x2AA78 0x30037031 */
#define CG_NOVIEWMODELTAG()     ( *(int *)&cg.unknown_0x2AA40[0x5C] )        /* cg+0x2AA9C 0x30036D57 */

/* Two of cgMedia_t's ten 23-entry surface-sound tables (media+0x2F8, 92-byte
   stride) and four of its nine impact-effect tables (media+0x70C), indexed by
   the surfaceType the bullet trace returned. */
#define CGS_BULLETSOUND_A()     ( (sfxHandle_t *)&cgs.media.unknown_0x2F8[0x170] )   /* cgs+0x9E4C */
#define CGS_BULLETSOUND_B()     ( (sfxHandle_t *)&cgs.media.unknown_0x2F8[0x1CC] )   /* cgs+0x9EA8 */
#define CGS_BULLETIMPACT_A()    ( (int *)&cgs.media.unknown_0x70C[0x000] )           /* cgs+0xA0F0 */
#define CGS_BULLETMARK_A()      ( (int *)&cgs.media.unknown_0x70C[0x05C] )           /* cgs+0xA14C */
#define CGS_BULLETIMPACT_B()    ( (int *)&cgs.media.unknown_0x70C[0x0B8] )           /* cgs+0xA1A8 */
#define CGS_BULLETMARK_B()      ( (int *)&cgs.media.unknown_0x70C[0x114] )           /* cgs+0xA204 */
#define CGS_FLESHHITNOBLOOD()   ( *(int *)&cgs.media.unknown_0x70C[0x33C] )          /* cgs+0xA42C */

/* The surfaceType of flesh; CG_BulletHitEvent special-cases it when cg_blood
   is off (0x30039683). */
#define SURF_TYPE_FLESH         7

/* The world-sound entity number CG_WhizbySound, CG_BulletHitEvent and
   CG_BulletHitClientEvent hand CG_PlaySoundAliasByName (0x30038EE3,
   0x30039691, 0x3003970C push 0x3FE); Q3's ENTITYNUM_WORLD, MAX_GENTITIES - 2. */
#define ENTITYNUM_WORLD         ( MAX_GENTITIES - 2 )

/* centity_t: the muzzle-flash latch CG_FireWeapon sets and CG_AddPlayerWeapon
   consumes (0x30038BAD / 0x30037165). */
#define CENT_MUZZLEFLASH( c )   ( *(int *)( c )->unknown_0x1E4 )             /* +0x1E4 */

/* playerState_t pm_flags bit that says the local client owns this entity, and
   the eFlags pair that suppresses the view weapon. */
#define PMF_FIRSTPERSON         0x50000     /* CG_AddPlayerWeapon 0x30036D14 */
#define PMF_WEAPONSELECTABLE    0x40000     /* CG_NextWeapon_f 0x3003807E */
#define EF_WEAPONLOCKED         0x00C000    /* CG_AltWeapon_f 0x30037FB6, ps.eFlags */
#define EF_NOVIEWWEAPON         0x00C000    /* CG_AddViewWeapon 0x30037271, `test ah, 0C0h` */
#define PMF_ADS                 0x20        /* CG_WeaponRunXModelAnims 0x30034464 */

/* Entity and event numbers this unit tests against. */
#define ET_PLAYER               1           /* CG_FireWeapon's flash entity check */
#define ET_TURRET               0x0B        /* CG_FireWeapon 0x30038BDD */
#define ET_EVENTS               0x0C        /* CG_EjectWeaponBrass 0x30038A40 */
#define EV_FIRE_WEAPON_LASTSHOT 0xA1        /* CG_FireWeapon 0x30038C1E */
#define EV_BULLET_HIT_SMALL     0xAD        /* CG_BulletHitEvent 0x30039641 */
#define EV_BULLET_HIT_CLIENT_SMALL 0xAF     /* CG_BulletHitClientEvent 0x300396F1 */

#define WEAPON_RELOADING        5           /* ps.weaponstate, 0x30034452 */

/*
 * The twenty viewmodel animations, in the order CG_RegisterWeapon registers
 * them: weaponInfo_t's idleAnim (+0x1C) through adsDownAnim (+0x68), one anim
 * index per field (0x30034E10).  Slot 0 of the tree is the additive root.
 */
#define WEAPANIM_IDLE           1
#define WEAPANIM_EMPTY_IDLE     2
#define WEAPANIM_FIRE           3
#define WEAPANIM_HOLDFIRE       4
#define WEAPANIM_LASTSHOT       5
#define WEAPANIM_RECHAMBER      6
#define WEAPANIM_MELEE          7
#define WEAPANIM_RELOAD         8
#define WEAPANIM_RELOAD_EMPTY   9
#define WEAPANIM_RELOAD_START   10
#define WEAPANIM_RELOAD_END     11
#define WEAPANIM_RAISE          12
#define WEAPANIM_DROP           13
#define WEAPANIM_ALTRAISE       14
#define WEAPANIM_ALTDROP        15
#define WEAPANIM_ADS_FIRE       16
#define WEAPANIM_ADS_LASTSHOT   17
#define WEAPANIM_ADS_RECHAMBER  18
#define WEAPANIM_ADS_UP         19
#define WEAPANIM_ADS_DOWN       20
#define WEAPANIM_NUM            21

/* bg_itemlist is 70 rows; CG_RegisterItems walks 1..69 (0x300361C5). */
#ifndef MAX_ITEMS
#define MAX_ITEMS               BG_NUM_ITEMS
#endif

#define CS_ITEMS                8           /* CG_RegisterItems 0x3003616D */

/* CG_FadeColor's total for the weapon-select overlay (0x300377C0). */
#define WEAPON_SELECT_TIME      1800

/*
=============================================================================

	GLOBALS

=============================================================================
*/

cgWeaponInfo_t  cg_weapons[MAX_WEAPONS_CG];             /* 0x301A6940 */
cgItemInfo_t    cg_items[MAX_ITEMS];                    /* 0x301DC3E0 */

/* The world-space tag_brass origin CG_AddPlayerWeapon leaves behind for
   CG_EjectWeaponBrass to play the shell effect at (0x305430C0). */
static vec3_t   cg_brassOrigin;

/* CG_DrawWeaponSelect's alt-weapon carousel state: the slot and weapon the
   spin was last armed for, and the spin fraction in [-1,1] that decays to zero
   (0x300EEF10, 0x300EEF14, 0x30096C20). */
static int      cg_weaponSelectSpinSlot;
static int      cg_weaponSelectSpinWeapon;
static float    cg_weaponSelectSpin;

/* 0x30094720.  Shared with cg_consolecmds_mp.c (CG_Fade_f, CG_ConsoleCommand);
   not static in the binary. */
char            cg_cmdArgBuffer[1024];

/* The tag every muzzle flash and bullet trajectory is bolted to, 0x3007486C.
   The binary indexes it (CG_FireWeapon 0x30038BE7), so it is a table; 1.1 only
   ever reaches element 0. */
static const char *cg_flashTags[] = {
	"tag_flash"
};

/*
=============================================================================

	VIEWMODEL ANIMATION

=============================================================================
*/

/*
==============
CG_StartWeaponAnim

Drive the viewmodel anim tree to `anim`: full goal weight on the one animation
asked for, zero on the other seventeen, all at this weapon's own rate.
==============
*/
static void CG_StartWeaponAnim( int weapon, int obj, int anim ) {
	const cgWeaponInfo_t    *cgWeapon;
	int i;

	cgWeapon = &cg_weapons[ weapon ];
	for ( i = 1; i < 19; i++ ) {
		if ( anim == i ) {
			trap_syscall_0x8C( obj, i, 1.0f, 0.0f, cgWeapon->animRate[i], 1, 1 );
		} else {
			trap_syscall_0x8C( obj, i, 0.0f, 0.0f, cgWeapon->animRate[i], 0, 0 );
		}
	}
}

/*
==============
CG_PlayADSAnim

Cross-fade the two aim-down-sight animations and scrub both to the transition
fraction Pmove keeps in ps.fWeaponPosFrac.  `weapon` is passed in and never
read -- the compiler proved it dead but the caller still pushes it
(0x3003447B / 0x30034240, which has no arg slot for it).
==============
*/
static void CG_PlayADSAnim( int weapon, int anim, int obj ) {
	if ( anim == WEAPANIM_ADS_UP ) {
		trap_syscall_0x8C( obj, WEAPANIM_ADS_UP, 1.0f, 0.5f, 0.0f, 1, 0 );
		trap_syscall_0x8C( obj, WEAPANIM_ADS_DOWN, 0.0f, 0.5f, 0.0f, 0, 0 );
	} else {
		trap_syscall_0x8C( obj, WEAPANIM_ADS_UP, 0.0f, 0.5f, 0.0f, 0, 0 );
		trap_syscall_0x8C( obj, WEAPANIM_ADS_DOWN, 1.0f, 0.5f, 0.0f, 1, 0 );
	}

	trap_XAnimSetTime( obj, WEAPANIM_ADS_UP, cg.predictedPlayerState.fWeaponPosFrac );
	trap_XAnimSetTime( obj, WEAPANIM_ADS_DOWN, 1.0f - cg.predictedPlayerState.fWeaponPosFrac );
}

/*
==============
CG_ProcessWeaponNoteTracks

Play the sound alias any note track the viewmodel anim tree fired this frame
names.  The engine hands back its deferred-notify array through trap 150.
==============
*/
static void CG_ProcessWeaponNoteTracks( void ) {
	const cgWeaponInfo_t    *cgWeapon;
	const char        **notes;
	const char         *alias;
	int numNotes;
	int i;

	if ( !cg.predictedPlayerState.weapon ) {
		return;
	}

	cgWeapon = &cg_weapons[ cg.predictedPlayerState.weapon ];
	numNotes = trap_syscall_0x96( (int)&notes );
	for ( i = 0; i < numNotes; i++ ) {
		alias = NULL;
		if ( !_stricmp( notes[i * 3], "noteTrackSoundA" ) ) {
			alias = cgWeapon->noteTrackSoundA;
		} else if ( !_stricmp( notes[i * 3], "noteTrackSoundB" ) ) {
			alias = cgWeapon->noteTrackSoundB;
		} else if ( !_stricmp( notes[i * 3], "noteTrackSoundC" ) ) {
			alias = cgWeapon->noteTrackSoundC;
		} else if ( !_stricmp( notes[i * 3], "noteTrackSoundD" ) ) {
			alias = cgWeapon->noteTrackSoundD;
		}
		if ( alias ) {
			CG_PlaySoundAliasByName( alias, cg.snap->ps.clientNum, cg.snap->ps.origin );
		}
	}
}

/*
==============
CG_WeaponRunXModelAnims

Translate ps.weapAnim into the viewmodel anim tree, once per change.  Bit 9 of
weapAnim is Pmove's restart toggle and is masked out of the dispatch but kept in
the cached value, so re-firing the same animation restarts it.
==============
*/
static void CG_WeaponRunXModelAnims( playerState_t *ps, cgWeaponInfo_t *cgWeapon ) {
	const weaponInfo_t  *weaponInfo;
	qboolean ads;
	int tree;
	int i;

	tree = trap_syscall_0xB1( cgWeapon->dobj );
	weaponInfo = bg_weaponInfo[ps->weapon];

	ads = qfalse;
	if ( ps->weaponstate != WEAPON_RELOADING
		 || ps->weaponTime - weaponInfo->adsReloadTransTime <= 0 ) {
		if ( ps->pm_flags & PMF_ADS ) {
			ads = qtrue;
		}
	}
	if ( weaponInfo->aimDownSight ) {
		CG_PlayADSAnim( ps->weapon, ads ? WEAPANIM_ADS_UP : WEAPANIM_ADS_DOWN, tree );
	}

	if ( ps->weapAnim == cgWeapon->lastWeapAnim ) {
		return;
	}

	switch ( ps->weapAnim & ~0x200 ) {
	case 0:
		/* Idle.  Hold off until every animation in the tree has a time --
		   until then the tree is not ready and the cache is invalidated. */
		for ( i = 1; i < 19; i++ ) {
			if ( !trap_syscall_0xB4( tree, i ) ) {
				cgWeapon->lastWeapAnim = -1;
				return;
			}
		}
		if ( ps->ammoclip[bg_weaponInfo[ps->weapon]->clipIndex] ) {
			CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_IDLE );
		} else {
			CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_EMPTY_IDLE );
		}
		break;
	case 2:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_FIRE );
		break;
	case 3:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_LASTSHOT );
		break;
	case 4:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_RECHAMBER );
		break;
	case 5:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_ADS_FIRE );
		break;
	case 6:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_ADS_LASTSHOT );
		break;
	case 7:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_ADS_RECHAMBER );
		break;
	case 8:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_MELEE );
		break;
	case 9:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_DROP );
		break;
	case 10:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_RAISE );
		break;
	case 11:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_RELOAD );
		break;
	case 12:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_RELOAD_EMPTY );
		break;
	case 13:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_RELOAD_START );
		break;
	case 14:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_RELOAD_END );
		break;
	case 15:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_ALTDROP );
		break;
	case 16:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_ALTRAISE );
		break;
	case 17:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_HOLDFIRE );
		break;
	default:
		CG_StartWeaponAnim( ps->weapon, tree, WEAPANIM_IDLE );
		Com_Printf( "CG_WeaponRunXModelAnims: Unknown weapon animation %i\n",
					ps->weapAnim & ~0x200 );
		break;
	}

	cgWeapon->lastWeapAnim = ps->weapAnim;
}

/*
=============================================================================

	DEBUG LINE DRAWING

=============================================================================
*/

/*
==============
CG_RailTrail2

One coloured line, as a fade-RGB local entity that lives for
cg_railTrailTime msec.  RTCW's CG_RailTrail core.
==============
*/
static void CG_RailTrail2( const vec3_t start, const vec3_t end, const vec3_t color ) {
	localEntity_t   *le;
	refEntity_t     *re;

	if ( cg_railTrailTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = LE_REFENTITY( le );

	le->leType = LE_FADE_RGB;
	le->endTime = cg.time + cg_railTrailTime.integer;
	le->lifeRate = 1.0f / (float)cg_railTrailTime.integer;

	re->reType = RT_RAIL_CORE;
	RE_SHADERTIME( re ) = (float)cg.time / 1000.0f;
	RE_CUSTOMSHADER( re ) = cgs.media.railCoreShader;

	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin );

	LE_COLOR( le )[0] = color[0];
	LE_COLOR( le )[1] = color[1];
	LE_COLOR( le )[2] = color[2];
	LE_COLOR( le )[3] = 1.0f;

	AxisClear( re->axis );
}

/*
==============
CG_RailTrail

The EV_RAILTRAIL debug primitive.  `type` picks both the colour and the shape:
0-1 and 'E'-'H' are a single line, 2 and 3 are a wireframe box between the two
points, 5 is a sixteen-segment circle of radius end[0] about start.
==============
*/
void CG_RailTrail( int type, const vec3_t start, const vec3_t end ) {
	vec3_t color;
	vec3_t up, perp, cross;
	vec3_t size;
	vec3_t c1, c2, c3, c4, c5, c6;
	vec3_t points[16];
	float radius;
	float angle, s, c;
	int i;

	color[0] = 0.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;

	switch ( type ) {
	case 0:
		CG_RailTrail2( end, start, color );
		return;
	case 1:
		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		CG_RailTrail2( end, start, color );
		return;
	case 'E':
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 0.0f;
		CG_RailTrail2( end, start, color );
		return;
	case 'F':
		color[0] = 0.0f;
		color[1] = 1.0f;
		color[2] = 0.0f;
		CG_RailTrail2( end, start, color );
		return;
	case 'G':
		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 1.0f;
		CG_RailTrail2( end, start, color );
		return;
	case 'H':
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
		CG_RailTrail2( end, start, color );
		return;
	case 3:
		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		break;
	case 5:
		radius = end[0];
		up[0] = 0.0f;
		up[1] = 0.0f;
		up[2] = 1.0f;
		PerpendicularVector( perp, up );
		CrossProduct( up, perp, cross );
		for ( i = 0; i < 16; i++ ) {
			angle = (float)i * 0.39269909f;         /* 2*PI / 16 */
			s = (float)sin( angle ) * radius;
			c = (float)cos( angle ) * radius;
			points[i][0] = perp[0] * c + cross[0] * s + start[0];
			points[i][1] = perp[1] * c + cross[1] * s + start[1];
			points[i][2] = perp[2] * c + cross[2] * s + start[2];
		}
		for ( i = 0; i < 16; i++ ) {
			CG_RailTrail2( points[i], points[( i + 1 ) & 15], color );
		}
		return;
	default:
		break;
	}

	/* The box.  Six corners are built off `end` and `start` and the twelve
	   edges are drawn one CG_RailTrail2 at a time. */
	size[0] = end[0] - start[0];
	size[1] = end[1] - start[1];
	size[2] = end[2] - start[2];

	c1[0] = end[0] - size[0];   c1[1] = end[1];             c1[2] = end[2];
	c2[0] = end[0];             c2[1] = end[1] - size[1];   c2[2] = end[2];
	c3[0] = end[0];             c3[1] = end[1];             c3[2] = end[2] - size[2];

	CG_RailTrail2( end, c1, color );
	CG_RailTrail2( end, c2, color );
	CG_RailTrail2( end, c3, color );

	c4[0] = start[0] + size[0]; c4[1] = start[1];           c4[2] = start[2];
	c5[0] = start[0];           c5[1] = start[1] + size[1]; c5[2] = start[2];
	c6[0] = start[0];           c6[1] = start[1];           c6[2] = start[2] + size[2];

	CG_RailTrail2( start, c4, color );
	CG_RailTrail2( start, c5, color );
	CG_RailTrail2( start, c6, color );

	CG_RailTrail2( c2, c6, color );
	CG_RailTrail2( c6, c1, color );
	CG_RailTrail2( c1, c5, color );
	CG_RailTrail2( c2, c4, color );
	CG_RailTrail2( c4, c3, color );
	CG_RailTrail2( c3, c5, color );
}

/*
=============================================================================

	WEAPON REGISTRATION

=============================================================================
*/

/*
==============
CG_WeaponDObjHandle

The DObj slot a weapon's viewmodel lives in.  1024 is where the client DObj
range for viewmodels starts; uncalled in 1.1, every call site inlines it.
==============
*/
static int CG_WeaponDObjHandle( int weapon ) {
	return weapon + 1024;
}

/*
==============
CG_RegisterWeapon

The one-time load of everything cgame needs for a weapon: the viewmodel DObj
and its anim tree, the world model, the icons, the sound aliases and the
effects.  Idempotent -- `registered` gates the whole body.
==============
*/
void CG_RegisterWeapon( int weaponNum ) {
	const weaponInfo_t  *weaponInfo;
	cgWeaponInfo_t          *cgWeapon;
	cgItemInfo_t        *itemInfo;
	gitem_t             *item;
	cgDObjModel_t dobjModels[2];
	char filename[64];
	int anims, tree;
	qhandle_t hModel;
	int i;

	if ( !weaponNum ) {
		return;
	}

	weaponInfo = bg_weaponInfo[weaponNum];
	cgWeapon = &cg_weapons[ weaponNum ];
	if ( cgWeapon->registered ) {
		return;
	}

	memset( cgWeapon, 0, sizeof( *cgWeapon ) );
	cgWeapon->registered = qtrue;

	item = &bg_itemlist[BG_WeaponItemIndex( weaponNum )];
	itemInfo = &cg_items[BG_WeaponItemIndex( weaponNum )];
	cgWeapon->item = item;
	CG_RegisterItemVisuals( BG_WeaponItemIndex( weaponNum ) );
	cgWeapon->lastWeapAnim = -1;

	if ( weaponInfo->gunModel[0] ) {
		if ( !weaponInfo->handModel || !weaponInfo->handModel[0] ) {
			/* "\x15" spliced: a following hex digit would extend the escape */
			Com_Error( ERR_DROP, "\x15" "CG_RegisterWeapon: No hand model specified for [%s]",
					   weaponInfo->displayName );
		}
		if ( !weaponInfo->idleAnim || !weaponInfo->idleAnim[0] ) {
			Com_Error( ERR_DROP, "\x15" "CG_RegisterWeapon: No idle anim specified for [%s]",
					   weaponInfo->displayName );
		}

		/* The anim tree.  Twenty animations, all children of "root", indexed
		   1..20 in the order weaponInfo_t declares them; a weapon that leaves
		   one blank gets its idle animation in that slot instead. */
		anims = trap_XAnimCreateAnims( (int)"VIEWMODEL", WEAPANIM_NUM );
		trap_XAnimBlend( anims, 0, (int)"root", 1, 20, 0 );
		for ( i = 1; i < 21; i++ ) {
			/* weaponInfo_t's twenty animation names are twenty consecutive
			   const char * at +0x1C; the binary walks them with a pointer
			   (0x30034E10). */
			const char *animName = ( &weaponInfo->idleAnim )[i - 1];

			if ( animName[0] ) {
				trap_XAnimPrecache( (int)animName );
			} else {
				trap_XAnimPrecache( (int)weaponInfo->idleAnim );
				animName = weaponInfo->idleAnim;
			}
			trap_XAnimCreate( anims, i, (int)animName );
		}
		tree = trap_XAnimCreateTree( anims );

		/* Play the animations that have a fixed duration in the weapon file at
		   the rate that makes them last exactly that long. */
		for ( i = 0; i < WEAPANIM_NUM; i++ ) {
			cgWeapon->animRate[i] = 1.0f;
		}
		if ( weaponInfo->holdFireTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_HOLDFIRE] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_HOLDFIRE ) / (float)weaponInfo->holdFireTime;
		} else {
			cgWeapon->animRate[WEAPANIM_HOLDFIRE] = 0.0f;
		}
		if ( weaponInfo->meleeTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_MELEE] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_MELEE ) / (float)weaponInfo->meleeTime;
		} else {
			cgWeapon->animRate[WEAPANIM_MELEE] = 0.0f;
		}
		if ( weaponInfo->reloadTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_RELOAD] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_RELOAD ) / (float)weaponInfo->reloadTime;
		} else {
			cgWeapon->animRate[WEAPANIM_RELOAD] = 0.0f;
		}
		if ( weaponInfo->reloadEmptyTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_RELOAD_EMPTY] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_RELOAD_EMPTY ) / (float)weaponInfo->reloadEmptyTime;
		} else {
			cgWeapon->animRate[WEAPANIM_RELOAD_EMPTY] = 0.0f;
		}
		if ( weaponInfo->reloadStartTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_RELOAD_START] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_RELOAD_START ) / (float)weaponInfo->reloadStartTime;
		} else {
			cgWeapon->animRate[WEAPANIM_RELOAD_START] = 0.0f;
		}
		/* reloadEnd divides by reloadTime, not reloadEndTime -- the binary
		   reloads [esi+1E8h] here, the same field the reload rate above uses
		   (0x30034F8C).  Retail quirk. */
		if ( weaponInfo->reloadTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_RELOAD_END] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_RELOAD_END ) / (float)weaponInfo->reloadTime;
		} else {
			cgWeapon->animRate[WEAPANIM_RELOAD_END] = 0.0f;
		}
		if ( weaponInfo->raiseTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_RAISE] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_RAISE ) / (float)weaponInfo->raiseTime;
		} else {
			cgWeapon->animRate[WEAPANIM_RAISE] = 0.0f;
		}
		if ( weaponInfo->dropTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_DROP] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_DROP ) / (float)weaponInfo->dropTime;
		} else {
			cgWeapon->animRate[WEAPANIM_DROP] = 0.0f;
		}
		if ( weaponInfo->altRaiseTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_ALTRAISE] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_ALTRAISE ) / (float)weaponInfo->altRaiseTime;
		} else {
			cgWeapon->animRate[WEAPANIM_ALTRAISE] = 0.0f;
		}
		if ( weaponInfo->altDropTime > 0 ) {
			cgWeapon->animRate[WEAPANIM_ALTDROP] =
				(float)trap_XAnimGetLength( anims, WEAPANIM_ALTDROP ) / (float)weaponInfo->altDropTime;
		} else {
			cgWeapon->animRate[WEAPANIM_ALTDROP] = 0.0f;
		}

		trap_XAnimClearTreeGoalWeights( tree, 0, 0.0f );
		trap_syscall_0x8C( tree, 0, 1.0f, 0.0f, cgWeapon->animRate[0], 1, 0 );

		if ( weaponInfo->adsUpAnim[0] && trap_syscall_0x90( anims, WEAPANIM_ADS_UP ) ) {
			Com_Error( ERR_DROP, "\x15" "CG_RegisterWeapon: ADS anim [%s] cannot be looping",
					   weaponInfo->adsUpAnim );
		}
		if ( weaponInfo->adsDownAnim[0] && trap_syscall_0x90( anims, WEAPANIM_ADS_DOWN ) ) {
			Com_Error( ERR_DROP, "\x15" "CG_RegisterWeapon: ADS anim [%s] cannot be looping",
					   weaponInfo->adsDownAnim );
		}

		/* The viewmodel DObj: part 0 is the hands, part 1 is the gun, bolted
		   to the hands' tag_weapon. */
		sprintf( filename, "%s%s", "xmodel/", weaponInfo->handModel );
		dobjModels[0].xmodel = NULL;
		dobjModels[0].tagName = NULL;
		dobjModels[0].hModel = 0;
		hModel = trap_R_RegisterModel( filename, 6 );
		dobjModels[0].hModel = (short)hModel;
		dobjModels[0].xmodel = (void *)trap_R_GetXModelByHandle( (short)hModel );
		dobjModels[0].tagName = NULL;

		sprintf( filename, "%s%s", "xmodel/", weaponInfo->gunModel );
		dobjModels[1].xmodel = NULL;
		dobjModels[1].tagName = NULL;
		dobjModels[1].hModel = 0;
		hModel = trap_R_RegisterModel( filename, 6 );
		dobjModels[1].hModel = (short)hModel;
		dobjModels[1].xmodel = (void *)trap_R_GetXModelByHandle( (short)hModel );
		dobjModels[1].tagName = "tag_weapon";

		trap_DObjCreate( dobjModels, 2, (void *)tree, CG_WeaponDObjHandle( weaponNum ) );
		cgWeapon->dobj = trap_syscall_0xA2( CG_WeaponDObjHandle( weaponNum ) );

		strncpy( cgWeapon->handModelName, weaponInfo->handModel,
				 sizeof( cgWeapon->handModelName ) - 1 );
		cgWeapon->handModelName[sizeof( cgWeapon->handModelName ) - 1] = 0;
	}

	if ( weaponInfo->worldModel[0] ) {
		cgWeapon->worldModel = trap_R_RegisterModel( weaponInfo->worldModel, 7 );
	}
	if ( weaponInfo->worldModel[0] && !cgWeapon->worldModel ) {
		Com_Printf( "WARNING: Weapon %s could not load world model\n", weaponInfo->worldModel );
	}

	cgWeapon->weaponIcon = trap_R_RegisterShader( item->icon, 5 );
	cgWeapon->weaponSelectIcon = trap_R_RegisterShader( va( "%s_select", item->icon ), 5 );
	cgWeapon->ammoIcon = trap_R_RegisterShader( item->ammoicon, 5 );

	if ( weaponInfo->reticleCenter[0] ) {
		cgWeapon->reticleCenter = trap_R_RegisterShaderNoMip( weaponInfo->reticleCenter, 5 );
	}
	if ( weaponInfo->reticleSide[0] ) {
		cgWeapon->reticleSide = trap_R_RegisterShaderNoMip( weaponInfo->reticleSide, 5 );
	}
	if ( weaponInfo->adsOverlayShader[0] ) {
		cgWeapon->adsOverlayShader = trap_R_RegisterShaderNoMip( weaponInfo->adsOverlayShader, 5 );
	}

	if ( weaponInfo->viewFlashEffect[0] ) {
		cgWeapon->viewFlashEffect = trap_syscall_0xDE( weaponInfo->viewFlashEffect );
	}
	if ( weaponInfo->worldFlashEffect[0] ) {
		cgWeapon->worldFlashEffect = trap_syscall_0xDE( weaponInfo->worldFlashEffect );
	}

	cgWeapon->projectileSound = (const char *)trap_Com_SoundAliasString( weaponInfo->projectileSound );
	cgWeapon->pullbackSound = (const char *)trap_Com_SoundAliasString( weaponInfo->pullbackSound );
	cgWeapon->fireSound = (const char *)trap_Com_SoundAliasString( weaponInfo->fireSound );
	cgWeapon->fireEchoSound = (const char *)trap_Com_SoundAliasString( weaponInfo->fireEchoSound );
	cgWeapon->lastShotSound = (const char *)trap_Com_SoundAliasString( weaponInfo->lastShotSound );
	cgWeapon->rechamberSound = (const char *)trap_Com_SoundAliasString( weaponInfo->rechamberSound );
	cgWeapon->reloadSound = (const char *)trap_Com_SoundAliasString( weaponInfo->reloadSound );
	cgWeapon->reloadEmptySound = (const char *)trap_Com_SoundAliasString( weaponInfo->reloadEmptySound );
	cgWeapon->reloadStartSound = (const char *)trap_Com_SoundAliasString( weaponInfo->reloadStartSound );
	cgWeapon->reloadEndSound = (const char *)trap_Com_SoundAliasString( weaponInfo->reloadEndSound );
	cgWeapon->raiseSound = (const char *)trap_Com_SoundAliasString( weaponInfo->raiseSound );
	if ( !cgWeapon->raiseSound ) {
		cgWeapon->raiseSound = (const char *)trap_Com_SoundAliasString( "weap_raise" );
	}
	cgWeapon->altSwitchSound = (const char *)trap_Com_SoundAliasString( weaponInfo->altSwitchSound );
	cgWeapon->putawaySound = (const char *)trap_Com_SoundAliasString( weaponInfo->putawaySound );
	if ( !cgWeapon->putawaySound ) {
		cgWeapon->putawaySound = (const char *)trap_Com_SoundAliasString( "weap_putaway" );
	}
	cgWeapon->noteTrackSoundA = (const char *)trap_Com_SoundAliasString( weaponInfo->noteTrackSoundA );
	cgWeapon->noteTrackSoundB = (const char *)trap_Com_SoundAliasString( weaponInfo->noteTrackSoundB );
	cgWeapon->noteTrackSoundC = (const char *)trap_Com_SoundAliasString( weaponInfo->noteTrackSoundC );
	cgWeapon->noteTrackSoundD = (const char *)trap_Com_SoundAliasString( weaponInfo->noteTrackSoundD );

	if ( !itemInfo->pickupSound ) {
		itemInfo->pickupSound = (const char *)trap_Com_SoundAliasString( "weap_pickup" );
	}
	itemInfo->ammoPickupSound = (const char *)trap_Com_SoundAliasString( weaponInfo->ammoPickupSound );
	if ( !itemInfo->ammoPickupSound ) {
		itemInfo->ammoPickupSound = (const char *)trap_Com_SoundAliasString( "weap_ammo_pickup" );
	}

	if ( weaponInfo->shellEjectEffect[0] ) {
		cgWeapon->shellEjectEffect = trap_syscall_0xDE( weaponInfo->shellEjectEffect );
	}
	if ( weaponInfo->lastShotEjectEffect[0] ) {
		cgWeapon->lastShotEjectEffect = trap_syscall_0xDE( weaponInfo->lastShotEjectEffect );
	} else {
		cgWeapon->lastShotEjectEffect = cgWeapon->shellEjectEffect;
	}

	if ( weaponInfo->projectileModel[0] ) {
		cgWeapon->missileModel = trap_R_RegisterModel( weaponInfo->projectileModel, 7 );
		if ( !cgWeapon->missileModel ) {
			CG_Error( "Weapon %s does not specify a valid projectile model (%s)\n",
					  weaponInfo->worldModel, weaponInfo->projectileModel );
		}
	}
	if ( weaponInfo->projExplosionEffect[0] ) {
		cgWeapon->projExplosionEffect = trap_syscall_0xDE( weaponInfo->projExplosionEffect );
	}
	cgWeapon->projExplosionSound =
		(const char *)trap_Com_SoundAliasString( weaponInfo->projExplosionSound );
	if ( weaponInfo->projTrailEffect[0] ) {
		cgWeapon->missileTrailEffect = trap_syscall_0xDE( weaponInfo->projTrailEffect );
	}
	cgWeapon->missileDlight = (float)weaponInfo->projectileDLight;

	if ( weaponInfo->hudIcon[0] ) {
		cgWeapon->hudIcon = trap_R_RegisterShader( weaponInfo->hudIcon, 5 );
		cgs.media.hintShaders[HINT_WEAPON_FIRST + weaponNum - 1] = cgWeapon->hudIcon;
	} else {
		cgs.media.hintShaders[HINT_WEAPON_FIRST + weaponNum - 1] = cgs.media.hintShaders[HINT_ACTIVATE];
	}
	if ( weaponInfo->killIcon[0] ) {
		trap_R_RegisterShader( weaponInfo->killIcon, 5 );
	}
	if ( weaponInfo->modeIcon[0] ) {
		cgWeapon->modeIcon = trap_R_RegisterShader( weaponInfo->modeIcon, 5 );
	}
	if ( weaponInfo->ammoIcon[0] ) {
		cgWeapon->ammoHudIcon = trap_R_RegisterShader( weaponInfo->ammoIcon, 5 );
		cgs.media.hintShaders[HINT_AMMO_FIRST + weaponNum - 1] = cgWeapon->ammoHudIcon;
	} else {
		cgs.media.hintShaders[HINT_AMMO_FIRST + weaponNum - 1] = cgs.media.hintShaders[HINT_ACTIVATE];
	}

	cgWeapon->displayName = trap_SE_TranslateReference( weaponInfo->displayName );
	if ( !cgWeapon->displayName ) {
		if ( cl_languagewarnings.integer ) {
			if ( cl_languagewarningsaserrors.integer ) {
				Com_Error( ERR_LOCALIZATION,
						   "Weapon %s: Could not translate display name \"%s\"",
						   weaponInfo->szInternalName, weaponInfo->displayName );
			} else {
				Com_Printf( "^3WARNING: Weapon %s: Could not translate display name \"%s\"\n",
							weaponInfo->szInternalName, weaponInfo->displayName );
			}
		}
		cgWeapon->displayName = weaponInfo->displayName;
	}
	cgWeapon->modeName = trap_SE_TranslateReference( weaponInfo->modeName );
	if ( !cgWeapon->modeName ) {
		if ( cl_languagewarnings.integer ) {
			if ( cl_languagewarningsaserrors.integer ) {
				Com_Error( ERR_LOCALIZATION, "Weapon %s: Could not translate mode name \"%s\"",
						   weaponInfo->szInternalName, weaponInfo->modeName );
			} else {
				Com_Printf( "^3WARNING: Weapon %s: Could not translate mode name \"%s\"\n",
							weaponInfo->szInternalName, weaponInfo->modeName );
			}
		}
		cgWeapon->modeName = weaponInfo->modeName;
	}
	cgWeapon->AIOverlayDescription = trap_SE_TranslateReference( weaponInfo->AIOverlayDescription );
	if ( !cgWeapon->AIOverlayDescription ) {
		if ( cl_languagewarnings.integer ) {
			if ( cl_languagewarningsaserrors.integer ) {
				Com_Error( ERR_LOCALIZATION,
						   "Weapon %s: Could not translate AI overlay description \"%s\"",
						   weaponInfo->szInternalName, weaponInfo->AIOverlayDescription );
			} else {
				Com_Printf( "^3WARNING: Weapon %s: Could not translate AI overlay description \"%s\"\n",
							weaponInfo->szInternalName, weaponInfo->AIOverlayDescription );
			}
		}
		cgWeapon->AIOverlayDescription = weaponInfo->AIOverlayDescription;
	}
}

/*
==============
CG_ChangeViewmodelDobj

Rebuild a weapon's viewmodel DObj around a different pair of hands, keeping the
anim tree it already has.  Called when the hand model changes at runtime.
==============
*/
static void CG_ChangeViewmodelDobj( int weapon, const char *handModel ) {
	const weaponInfo_t  *weaponInfo;
	cgWeaponInfo_t          *cgWeapon;
	cgDObjModel_t dobjModels[2];
	char handFile[64];
	char gunFile[64];
	int tree;
	qhandle_t hModel;

	if ( !weapon ) {
		return;
	}

	weaponInfo = bg_weaponInfo[weapon];
	cgWeapon = &cg_weapons[ weapon ];
	if ( !cgWeapon->dobj ) {
		return;
	}
	if ( !weaponInfo->gunModel[0] ) {
		return;
	}

	tree = trap_syscall_0xB1( cgWeapon->dobj );
	trap_SafeDObjFree( CG_WeaponDObjHandle( weapon ), 0 );

	dobjModels[0].xmodel = NULL;
	dobjModels[0].tagName = NULL;
	dobjModels[0].hModel = 0;

	sprintf( handFile, "%s%s", "xmodel/", weaponInfo->handModel );
	if ( handModel[0] ) {
		dobjModels[0].hModel = (short)trap_R_RegisterModel( handModel, 6 );
	} else {
		dobjModels[0].hModel = (short)trap_R_RegisterModel( handFile, 6 );
	}
	hModel = trap_R_RegisterModel( handFile, 6 );
	dobjModels[0].xmodel = (void *)trap_R_GetXModelByHandle( (short)hModel );
	dobjModels[0].tagName = NULL;
	dobjModels[1].tagName = NULL;
	dobjModels[1].hModel = 0;

	sprintf( gunFile, "%s%s", "xmodel/", weaponInfo->gunModel );
	dobjModels[1].hModel = (short)trap_R_RegisterModel( gunFile, 6 );
	dobjModels[1].xmodel = (void *)trap_R_GetXModelByHandle( dobjModels[1].hModel );
	dobjModels[1].tagName = "tag_weapon";

	trap_DObjCreate( dobjModels, 2, (void *)tree, CG_WeaponDObjHandle( weapon ) );
	cgWeapon->dobj = trap_syscall_0xA2( CG_WeaponDObjHandle( weapon ) );

	strncpy( cgWeapon->handModelName, handModel, sizeof( cgWeapon->handModelName ) - 1 );
	cgWeapon->handModelName[sizeof( cgWeapon->handModelName ) - 1] = 0;
}

/*
==============
CG_UpdateHandViewmodels

Swap every registered weapon over to a new hand model, skipping the ones that
already have it.
==============
*/
void CG_UpdateHandViewmodels( const char *handModel ) {
	int i;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		if ( strcmp( cg_weapons[ i ].handModelName, handModel ) ) {
			CG_ChangeViewmodelDobj( i, handModel );
		}
	}
}

/*
==============
CG_FreeWeapons

Drop every viewmodel DObj at shutdown.
==============
*/
void CG_FreeWeapons( void ) {
	int i;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		trap_SafeDObjFree( CG_WeaponDObjHandle( i ), 0 );
	}
}

/*
==============
CG_RegisterItemVisuals

The models, icon and pickup sound one bg_itemlist row needs.  RTCW's, including
its `sizeof( &itemInfo )` -- the memset really does clear four bytes, which is
exactly the `registered` flag the function goes on to set.
==============
*/
void CG_RegisterItemVisuals( int itemNum ) {
	cgItemInfo_t    *itemInfo;
	gitem_t         *item;
	int i;
	if ( (unsigned)itemNum >= BG_NUM_ITEMS ) {
		CG_Error( "Item index out of range: %i", itemNum );
		return;
	}

	itemInfo = &cg_items[itemNum];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[itemNum];
	memset( itemInfo, 0, sizeof( *itemInfo ) );

	for ( i = 0; i < MAX_ITEM_MODELS; i++ ) {
		if ( item->world_model[i] && item->world_model[i][0] ) {
			itemInfo->models[i] = trap_R_RegisterModel( item->world_model[i], i ? 6 : 7 );
		}
	}

	itemInfo->icon = trap_R_RegisterShader( item->icon, 5 );

	if ( item->pickup_sound ) {
		itemInfo->pickupSound = (const char *)trap_Com_SoundAliasString( item->pickup_sound );
	}
	itemInfo->ammoPickupSound = itemInfo->pickupSound;

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	itemInfo->registered = qtrue;
}

/*
==============
CG_RegisterItems

The items configstring is one hex nibble per four item indexes; register the
visuals of every item the server says is in the level.
==============
*/
void CG_RegisterItems( void ) {
	char items[BG_NUM_ITEMS + 1];    /* RTCW's MAX_ITEMS + 1; frame is 0x108 (0x30036150) */
	int value;
	int i;

	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1; i < MAX_ITEMS; i++ ) {
		value = items[i / 4];
		if ( value > '9' ) {
			value -= 'W';
		} else {
			value -= '0';
		}
		if ( value & ( 1 << ( i & 3 ) ) ) {
			CG_RegisterItemVisuals( i );
		}
	}
}

/*
=============================================================================

	WEAPON POSITION

	bg_weapon.c owns the maths; this half owns the cg_t state it runs on.

=============================================================================
*/

/*
==============
CG_CalculateWeaponPosition_Sway

Feed the view-angle delta to the shared sway solver, scaled up while a
shellshock is running.
==============
*/
static void CG_CalculateWeaponPosition_Sway( void ) {
	const weaponInfo_t  *weaponInfo;
	float scale;
	float frac;
	int remaining;

	weaponInfo = bg_weaponInfo[cg.predictedPlayerState.weapon];

	remaining = cg.shellshock.duration - cg.time + cg.shellshock.startTime;
	if ( remaining > 0 ) {
		frac = 1.0f;
		if ( (float)remaining < cg.shellshock.parms->lerpTime ) {
			frac = (float)remaining / cg.shellshock.parms->lerpTime;
		}
		scale = ( weaponInfo->swayShellShockScale - 1.0f )
				* ( ( 3.0f - ( frac + frac ) ) * frac * frac ) + 1.0f;
	} else {
		scale = 1.0f;
	}

	BG_CalculateWeaponPosition_Sway( CG_SWAYOFFSETS(), CG_SWAYANGLES(),
									 &cg.predictedPlayerState, CG_PREVVIEWANGLES(),
									 scale, cg.frametime );
}

/*
==============
CG_CalculateWeaponPosition_SwayMovement
==============
*/
static void CG_CalculateWeaponPosition_SwayMovement( vec3_t offset ) {
	offset[1] = offset[1] - CG_SWAYOFFSETS()[1];
	offset[2] = CG_SWAYOFFSETS()[2] + offset[2];
}

/*
==============
CG_CalculateWeaponPosition_BasePosition_movement

The weapon's own idea of where it sits: a stance-dependent offset plus a
speed-scaled movement offset, each tracked towards its target at the weapon's
position rate and folded out again as the sights come up.
==============
*/
static void CG_CalculateWeaponPosition_BasePosition_movement( vec3_t offset ) {
	const weaponInfo_t  *weaponInfo;
	const playerState_t *ps;
	float move[3];
	float minSpeed;
	float frac;
	float rate;
	float step;
	float limit;
	float ads;
	int eFlags;
	int i;

	weaponInfo = cg.weaponInfo;
	ps = &cg.predictedPlayerState;
	eFlags = cg.predictedPlayerEntity.currentState.eFlags;

	if ( eFlags & 0x40 ) {
		minSpeed = cg_gun_move_minspeed.value + weaponInfo->proneMoveMinSpeed;
	} else if ( eFlags & 0x20 ) {
		minSpeed = cg_gun_move_minspeed.value + weaponInfo->duckedMoveMinSpeed;
	} else {
		minSpeed = cg_gun_move_minspeed.value + weaponInfo->standMoveMinSpeed;
	}

	if ( CG_XYSPEED() > minSpeed && ps->weaponstate != WEAPON_RELOADING ) {
		frac = ( CG_XYSPEED() - minSpeed ) / ( (float)ps->speed - minSpeed );
		if ( 0.0f > frac ) {
			frac = 0.0f;
		} else if ( frac > 1.0f ) {
			frac = 1.0f;
		}

		if ( eFlags & 0x40 ) {
			move[0] = frac * weaponInfo->proneMoveF;
			move[1] = frac * weaponInfo->proneMoveR;
			move[2] = frac * weaponInfo->proneMoveU;
		} else if ( eFlags & 0x20 ) {
			move[0] = frac * weaponInfo->duckedMoveF;
			move[1] = frac * weaponInfo->duckedMoveR;
			move[2] = frac * weaponInfo->duckedMoveU;
		} else {
			move[0] = frac * weaponInfo->standMoveF;
			move[1] = frac * weaponInfo->standMoveR;
			move[2] = frac * weaponInfo->standMoveU;
		}
		move[0] = cg_gun_move_f.value * frac + move[0];
		move[1] = cg_gun_move_r.value * frac + move[1];
		move[2] = cg_gun_move_u.value * frac + move[2];
	} else {
		move[0] = 0.0f;
		move[1] = 0.0f;
		move[2] = 0.0f;
	}

	if ( ps->viewHeightTarget == ps->crouchViewHeight ) {
		move[0] += weaponInfo->duckedOfsF;
		move[1] += weaponInfo->duckedOfsR;
		move[2] += weaponInfo->duckedOfsU;
		move[0] += cg_gun_ofs_f.value;
		move[1] += cg_gun_ofs_r.value;
		move[2] += cg_gun_ofs_u.value;
	} else if ( ps->viewHeightTarget == ps->proneViewHeight ) {
		move[0] += weaponInfo->proneOfsF;
		move[1] += weaponInfo->proneOfsR;
		move[2] += weaponInfo->proneOfsU;
		move[0] += cg_gun_ofs_f.value;
		move[1] += cg_gun_ofs_r.value;
		move[2] += cg_gun_ofs_u.value;
	}

	/* Track the three components towards their targets.  Not a function in the
	   binary -- the compiler inlined it three times at 0x30036497, 0x30037574
	   and 0x3003665E -- so it is written out once here. */
	for ( i = 0; i < 3; i++ ) {
		if ( CG_POSTRACK()[i] == move[i] ) {
			continue;
		}
		if ( (float)ps->proneViewHeight == ps->viewHeightCurrent ) {
			rate = cg_gun_move_rate.value + weaponInfo->posProneMoveRate;
		} else {
			rate = cg_gun_move_rate.value + weaponInfo->posMoveRate;
		}
		step = rate * ( move[i] - CG_POSTRACK()[i] ) * (float)cg.frametime * 0.001f;
		if ( CG_POSTRACK()[i] < move[i] ) {
			limit = (float)cg.frametime * 0.0001f;
			if ( step < limit ) {
				step = limit;
			}
			CG_POSTRACK()[i] = step + CG_POSTRACK()[i];
			if ( CG_POSTRACK()[i] > move[i] ) {
				CG_POSTRACK()[i] = move[i];
			}
		} else {
			limit = (float)cg.frametime * -0.0001f;
			if ( step > limit ) {
				step = limit;
			}
			CG_POSTRACK()[i] = step + CG_POSTRACK()[i];
			if ( CG_POSTRACK()[i] < move[i] ) {
				CG_POSTRACK()[i] = move[i];
			}
		}
	}

	ads = cg.predictedPlayerState.fWeaponPosFrac;
	if ( ads != 0.0f ) {
		if ( ads >= 0.5f ) {
			return;
		}
		frac = 1.0f - ( ads + ads );
		offset[0] += CG_POSTRACK()[0] * frac;
		offset[1] += CG_POSTRACK()[1] * frac;
		offset[2] += CG_POSTRACK()[2] * frac;
	} else {
		offset[0] += CG_POSTRACK()[0];
		offset[1] += CG_POSTRACK()[1];
		offset[2] += CG_POSTRACK()[2];
	}
}

/*
==============
CG_CalculateWeaponPosition_BasePosition
==============
*/
static void CG_CalculateWeaponPosition_BasePosition( vec3_t offset ) {
	vec3_t move;

	move[0] = 0.0f;
	move[1] = 0.0f;
	move[2] = 0.0f;
	CG_CalculateWeaponPosition_BasePosition_movement( move );

	VectorCopy( move, CG_WEAPONPOSBASE() );

	offset[0] += move[0];
	offset[1] += move[1];
	offset[2] += move[2];
}

/*
==============
CG_CalculateWeaponPosition_ToWorldPosition

Turn a forward/right/up offset into a world point off the view origin.
==============
*/
static void CG_CalculateWeaponPosition_ToWorldPosition( vec3_t pos ) {
	vec3_t offset;
	vec3_t forward, right, up;

	VectorCopy( pos, offset );
	AngleVectors( cg.refdefViewAngles, forward, right, up );

	VectorCopy( cg.refdef.vieworg, pos );
	pos[0] += right[0] * -offset[1] + up[0] * offset[2] + forward[0] * offset[0];
	pos[1] += right[1] * -offset[1] + up[1] * offset[2] + forward[1] * offset[0];
	pos[2] += right[2] * -offset[1] + up[2] * offset[2] + forward[2] * offset[0];
}

/*
==============
CG_CalculateWeaponPosition_SaveOffsetMovement

Remember how far the weapon has drifted from the view origin while the sights
are up, scaled by the ADS fraction; CG_AddViewWeapon feeds it back to the
crosshair.
==============
*/
static void CG_CalculateWeaponPosition_SaveOffsetMovement( const vec3_t origin ) {
	float frac;

	frac = cg.predictedPlayerState.fWeaponPosFrac;
	if ( bg_weaponInfo[cg.predictedPlayerState.weapon]->aimDownSight && frac != 0.0f ) {
		CG_ADSWORLDOFFSET()[0] = ( origin[0] - cg.refdef.vieworg[0] ) * frac;
		CG_ADSWORLDOFFSET()[1] = ( origin[1] - cg.refdef.vieworg[1] ) * frac;
		CG_ADSWORLDOFFSET()[2] = ( origin[2] - cg.refdef.vieworg[2] ) * frac;
		return;
	}

	CG_ADSWORLDOFFSET()[0] = 0.0f;
	CG_ADSWORLDOFFSET()[1] = 0.0f;
	CG_ADSWORLDOFFSET()[2] = 0.0f;
}

/*
==============
CG_CalculateWeaponPosition

The view weapon's world origin: lean, the weapon's own base position, sway, and
RTCW's landing drop.
==============
*/
static void CG_CalculateWeaponPosition( vec3_t origin ) {
	int delta;

	origin[0] = 0.0f;
	origin[1] = 0.0f;
	origin[2] = 0.0f;

	if ( cg.predictedPlayerState.leanf != 0.0f
		 && cg.predictedPlayerState.fWeaponPosFrac < 1.0f ) {
		/* Not the eye's 16/20: retail inlines AddLeanToPosition here with the
		   angle at frac * -2.0 (0x30036A0E) and the offset at
		   frac * 1.6 * (1 - adsFrac) (0x30036A28). */
		AddLeanToPosition( origin, 0.0f, cg.predictedPlayerState.leanf, -2.0f,
						   1.6f * ( 1.0f - cg.predictedPlayerState.fWeaponPosFrac ) );
	}

	CG_CalculateWeaponPosition_BasePosition( origin );
	CG_CalculateWeaponPosition_SwayMovement( origin );
	CG_CalculateWeaponPosition_ToWorldPosition( origin );

	/* drop the weapon when landing */
	delta = cg.time - cg.landTime;
	if ( delta < 150 ) {
		origin[2] += cg.landChange * 0.25f * (float)delta / 150.0f;
	} else if ( delta < 150 + 300 ) {
		origin[2] += cg.landChange * 0.25f * (float)( 150 + 300 - delta ) / 300.0f;
	}

	CG_CalculateWeaponPosition_SaveOffsetMovement( origin );
}

/*
==============
CG_CalculateWeaponPostion_PositionToADS

Latch which way the sights are travelling, but only on a transition that starts
from a settled position -- once the fraction is moving, the direction is kept.
==============
*/
static void CG_CalculateWeaponPostion_PositionToADS( void ) {
	float frac;

	if ( !bg_weaponInfo[cg.predictedPlayerState.weapon]->aimDownSight ) {
		return;
	}

	frac = cg.predictedPlayerState.fWeaponPosFrac;
	if ( frac != 1.0f && frac != 0.0f
		 && ( CG_ADSLASTFRAC() == 1.0f || CG_ADSLASTFRAC() == 0.0f )
		 && frac != CG_ADSLASTFRAC() ) {
		CG_ADSGOINGUP() = ( frac > CG_ADSLASTFRAC() );
	}

	CG_ADSLASTFRAC() = frac;
}

/*
=============================================================================

	MUZZLE FLASH AND THE WEAPON MODELS

=============================================================================
*/

/*
==============
CG_WeaponFlash

Play a weapon's flash effect off a bone of the DObj it is attached to.
==============
*/
static void CG_WeaponFlash( int weapon, qboolean viewmodel, int obj, const vec3_t origin,
							const char *tagName ) {
	int effect;
	int bolt[2];

	if ( viewmodel ) {
		effect = cg_weapons[ weapon ].viewFlashEffect;
	} else {
		effect = cg_weapons[ weapon ].worldFlashEffect;
	}
	if ( !effect ) {
		return;
	}

	bolt[0] = obj;
	bolt[1] = trap_syscall_0xDF( obj, tagName );
	if ( bolt[1] < 0 ) {
		return;
	}

	trap_syscall_0xE5( effect, (int)origin, 0, (int)bolt );
}

/*
==============
CG_AddPlayerWeapon

Add the weapon model to the scene, either as the local client's viewmodel
(ps != NULL) or as another player's held weapon.  Also caches the tag_brass
world position for the shell effect and fires any pending muzzle flash.
==============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int drawGun ) {
	refEntity_t gun;
	cgWeaponInfo_t  *cgWeapon;
	qboolean firstPersonOwner;
	qboolean isViewWeapon;
	int partBits[4];
	int weaponNum;
	int boneIndex;
	int boneBase;
	float gunMatrix[12];
	float tagMatrix[16];

	firstPersonOwner = qfalse;
	if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
		 && cent->currentState.number == cg.snap->ps.clientNum ) {
		firstPersonOwner = qtrue;
	}

	weaponNum = cent->currentState.weapon;
	isViewWeapon = ( ps != NULL );

	if ( isViewWeapon && CG_NOVIEWMODELTAG() ) {
		return;
	}
	if ( cent->currentState.eFlags & EF_NOVIEWWEAPON ) {
		return;
	}

	CG_RegisterWeapon( weaponNum );
	cgWeapon = &cg_weapons[ weaponNum ];

	memset( &gun, 0, sizeof( gun ) );
	gun.renderfx = parent->renderfx;
	VectorCopy( RE_LIGHTINGORIGIN( parent ), RE_LIGHTINGORIGIN( &gun ) );
	RE_UNKNOWN_0x18( &gun ) = RE_UNKNOWN_0x18( parent );

	if ( isViewWeapon ) {
		gun.shaderRGBA[0] = 255;
		gun.shaderRGBA[1] = 255;
		gun.shaderRGBA[2] = 255;
		gun.shaderRGBA[3] = 255;

		RE_DOBJ( &gun ) = cgWeapon->dobj;
		RE_LIGHTINGORIGIN( &gun )[0] = ps->origin[0];
		RE_LIGHTINGORIGIN( &gun )[1] = ps->origin[1];
		RE_LIGHTINGORIGIN( &gun )[2] = ps->origin[2] + ps->viewHeightCurrent;
		gun.reType = RT_DOBJ;
		gun.renderfx = RF_PLAYERMODEL;
		AddLeanToPosition( RE_LIGHTINGORIGIN( &gun ), ps->viewangles[1], ps->leanf,
						   16.0f, 20.0f );

		trap_syscall_0x95( cgWeapon->dobj, (float)cg.frametime * 0.001f );

		partBits[0] = -1;
		partBits[1] = -1;
		partBits[2] = -1;
		partBits[3] = -1;
		if ( !trap_syscall_0xA9( cgWeapon->dobj, (int)partBits ) ) {
			trap_syscall_0x97( cgWeapon->dobj, (int)partBits );
			trap_syscall_0xAB( cgWeapon->dobj, (int)partBits );
		}

		CG_ProcessWeaponNoteTracks();

		/* THE isViewWeapon BLOCK RUNS TO HERE, not to CG_ProcessWeaponNoteTracks.
		   0x30036DAD is `test edi, edi` with edi = isViewWeapon and 0x30036DBB is
		   `jz loc_30037162`, which skips everything below in one jump: the gun
		   matrix, the viewmodel origin/axis stores and the tag_brass lookup. */
		/* The pose goes into the gun's OWN origin and axis -- 0x30036EE6..0x30036FE0
		   write the refEntity being built -- and cg's viewmodel origin/axis are
		   copied out of it afterwards. */
		VectorCopy( parent->axis[0], gun.axis[0] );
		VectorCopy( parent->axis[1], gun.axis[1] );
		VectorCopy( parent->axis[2], gun.axis[2] );
		VectorCopy( parent->origin, gun.origin );

		gun.origin[0] += cg.refdef.viewaxis[0][0] * cg_gunX.value;
		gun.origin[0] += cg.refdef.viewaxis[1][0] * cg_gunY.value;
		gun.origin[0] += cg.refdef.viewaxis[2][0] * cg_gunZ.value;
		gun.origin[1] += cg.refdef.viewaxis[0][1] * cg_gunX.value;
		gun.origin[1] += cg.refdef.viewaxis[1][1] * cg_gunY.value;
		gun.origin[1] += cg.refdef.viewaxis[2][1] * cg_gunZ.value;
		gun.origin[2] += cg.refdef.viewaxis[0][2] * cg_gunX.value;
		gun.origin[2] += cg.refdef.viewaxis[1][2] * cg_gunY.value;
		gun.origin[2] += cg.refdef.viewaxis[2][2] * cg_gunZ.value;

		if ( drawGun ) {
			trap_R_AddRefEntityToScene( &gun );
		}

		VectorCopy( gun.origin, CG_VIEWMODELORIGIN() );
		VectorCopy( gun.axis[0], CG_VIEWMODELAXIS()[0] );
		VectorCopy( gun.axis[1], CG_VIEWMODELAXIS()[1] );
		VectorCopy( gun.axis[2], CG_VIEWMODELAXIS()[2] );

		boneIndex = trap_syscall_0xAE( cgWeapon->dobj, (int)"tag_brass" );
		if ( boneIndex > -1 ) {
			boneBase = trap_syscall_0x9D( cgWeapon->dobj, 0 );
			/* the gun pose as nine axis floats followed by the origin (0x300370A5) */
			gunMatrix[0] = gun.axis[0][0];
			gunMatrix[1] = gun.axis[0][1];
			gunMatrix[2] = gun.axis[0][2];
			gunMatrix[3] = gun.axis[1][0];
			gunMatrix[4] = gun.axis[1][1];
			gunMatrix[5] = gun.axis[1][2];
			gunMatrix[6] = gun.axis[2][0];
			gunMatrix[7] = gun.axis[2][1];
			gunMatrix[8] = gun.axis[2][2];
			gunMatrix[9] = gun.origin[0];
			gunMatrix[10] = gun.origin[1];
			gunMatrix[11] = gun.origin[2];
			DObjSkel2MatrixMultiply43( gunMatrix, tagMatrix,
									   (const float *)( (byte *)boneBase + ( boneIndex << 6 ) ) );
			cg_brassOrigin[0] = tagMatrix[12];
			cg_brassOrigin[1] = tagMatrix[13];
			cg_brassOrigin[2] = tagMatrix[14];
		}
	}

	if ( !CENT_MUZZLEFLASH( cent ) ) {
		return;
	}
	if ( firstPersonOwner && !isViewWeapon ) {
		return;
	}
	CENT_MUZZLEFLASH( cent ) = 0;
	if ( !drawGun ) {
		return;
	}

	if ( isViewWeapon ) {
		CG_WeaponFlash( weaponNum, qtrue, CG_WeaponDObjHandle( weaponNum ),
						CG_VIEWMODELORIGIN(), cg_flashTags[0] );
	} else {
		CG_WeaponFlash( weaponNum, qfalse, cent->currentState.number,
						cent->lerpOrigin, cg_flashTags[0] );
	}
}

/*
==============
CG_AddViewWeapon

Add the local client's weapon to the scene: place it, run the sway and recoil
solvers over it, drive its animations, and hand it to CG_AddPlayerWeapon.
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t hand;
	pmWeaponAngleState_t state;
	cgWeaponInfo_t      *cgWeapon;
	vec3_t angles;
	vec3_t viewAxis[3];
	vec3_t weaponAxis[3];
	float zoom;
	qboolean drawGun;

	drawGun = qtrue;

	/* no gun if in third person view or dead */
	if ( ps->pm_type == 4 || ps->pm_type == 5 ) {
		return;
	}
	if ( cg.renderingThirdPerson ) {
		return;
	}

	if ( cg_drawGun.integer != 2
		 && ( cg_drawGun.integer == 0 || CG_GetWeapReticleZoom( &zoom ) ) ) {
		drawGun = qfalse;
	}

	if ( CG_NOVIEWWEAPON() ) {
		return;
	}
	if ( ps->eFlags & EF_NOVIEWWEAPON ) {
		return;
	}

	if ( ps->weapon <= 0 ) {
		CG_GUNPITCH() = cg.refdefViewAngles[0];
		CG_GUNYAW() = cg.refdefViewAngles[1];
		CG_ADSWORLDOFFSET()[0] = 0.0f;
		CG_ADSWORLDOFFSET()[1] = 0.0f;
		CG_ADSWORLDOFFSET()[2] = 0.0f;
		return;
	}

	CG_RegisterWeapon( ps->weapon );
	cgWeapon = &cg_weapons[ ps->weapon ];

	CG_CalculateWeaponPosition_Sway();
	CG_CalculateWeaponPostion_PositionToADS();

	memset( &hand, 0, sizeof( hand ) );
	CG_CalculateWeaponPosition( hand.origin );

	VectorMA( hand.origin, cg_gunX.value, cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, cg_gunY.value, cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, cg_gunZ.value, cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( cg.refdefViewAngles, viewAxis );

	state.ps = ps;
	state.speed = CG_XYSPEED();
	state.frametime = (float)cg.frametime * 0.001f;
	state.moveOffset[0] = CG_MOVEOFFSET()[0];
	state.moveOffset[1] = CG_MOVEOFFSET()[1];
	state.moveOffset[2] = CG_MOVEOFFSET()[2];
	state.idleScale = CG_IDLESCALE();
	state.time = cg.time - ps->deltaTime;
	if ( cg.damageTime ) {
		state.viewKickStartTime = cg.damageTime - ps->deltaTime;
	} else {
		state.viewKickStartTime = 0;
	}
	state.viewKickPitch = cg.damageY;
	state.viewKickYaw = cg.damageX;
	state.recoilPitch = CG_RECOILPITCH();
	state.recoilYaw = CG_RECOILYAW();
	state.recoilRoll = CG_RECOILROLL();
	state.recoilPitchSpeed = CG_RECOILPITCHSPEED();
	state.recoilYawSpeed = CG_RECOILYAWSPEED();
	state.recoilState = CG_RECOILSTATE();
	state.baseAngles[0] = CG_SWAYANGLES()[0];
	state.baseAngles[1] = CG_SWAYANGLES()[1];
	state.baseAngles[2] = CG_SWAYANGLES()[2];

	BG_CalculateWeaponAngles( &state, angles );

	AnglesToAxis( angles, weaponAxis );
	MatrixMultiply( weaponAxis, viewAxis, hand.axis );

	if ( bg_weaponInfo[ps->weapon]->aimDownSight && ps->fWeaponPosFrac != 0.0f ) {
		/* angles is the OUT parameter and arrives in eax, the axis in ecx
		   (AxisToAngles 0x3003C815).  That is the register mapping, not the
		   source order -- the axis is written first, matching the single
		   __cdecl definition in universal/com_math_raw.c.  0x300374EE reads
		   angles[0] and [1] back out after the call, so angles is the
		   destination and hand.axis must survive. */
		AxisToAngles( hand.axis, angles );
		CG_GUNPITCH() = AngleNormalize360( angles[0] );
		CG_GUNYAW() = AngleNormalize360( angles[1] );
	} else {
		CG_GUNPITCH() = cg.refdefViewAngles[0];
		CG_GUNYAW() = cg.refdefViewAngles[1];
	}

	CG_MOVEOFFSET()[0] = state.moveOffset[0];
	CG_MOVEOFFSET()[1] = state.moveOffset[1];
	CG_MOVEOFFSET()[2] = state.moveOffset[2];
	CG_IDLESCALE() = state.idleScale;
	CG_RECOILPITCH() = state.recoilPitch;
	CG_RECOILYAW() = state.recoilYaw;
	CG_RECOILROLL() = state.recoilRoll;
	CG_RECOILPITCHSPEED() = state.recoilPitchSpeed;
	CG_RECOILYAWSPEED() = state.recoilYawSpeed;
	CG_RECOILSTATE() = state.recoilState;

	CG_WeaponRunXModelAnims( ps, cgWeapon );

	hand.renderfx = RF_VIEWMODEL;
	CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, drawGun );
}

/*
=============================================================================

	WEAPON SELECTION

=============================================================================
*/

/*
==============
CG_DrawWeapSlotBinding

The key bound to "weaponslot <name>", drawn with a one-pixel drop shadow beside
a weapon-select entry.
==============
*/
static void CG_DrawWeapSlotBinding( int slot, const float *color, float x, float y ) {
	const char *binding;
	vec4_t textColor;

	binding = BindingFromName( va( "weaponslot %s", BG_GetWeaponSlotNameForIndex( slot ) ) );

	textColor[0] = 0.0f;
	textColor[1] = 0.0f;
	textColor[2] = 0.0f;
	textColor[3] = color[3];
	trap_R_Text_Paint( x + 1.0f, y + 1.0f - 4.0f + 9.6f, 5, 0.25f, textColor,
					   binding, 6.0f, 0, 0 );

	textColor[0] = 1.0f;
	textColor[1] = 1.0f;
	textColor[2] = 1.0f;
	trap_R_Text_Paint( x, y - 4.0f + 9.6f, 5, 0.25f, textColor,
					   binding, 6.0f, 0, 0 );

	trap_R_SetColor( color );
}

/*
==============
CG_DrawWeaponSelect

The weapon-select overlay: one icon per occupied slot, the selected slot grown
and lit, and its alternate weapons spread out beside it with a spin that decays
back to the selected one.
==============
*/
void CG_DrawWeaponSelect( void ) {
	const weaponInfo_t  *weaponInfo;
	const cgWeaponInfo_t    *cgWeapon;
	float       *color;
	vec4_t drawColor;
	float positions[8];
	float x, width, height, size, pos;
	float spinAbs;
	int msec;
	int selectedSlot;
	int weapon, alt;
	int numAlts;
	int slot;
	int index;

	msec = cg.time - CG_WEAPONSELECTDRAWTIME();
	CG_WEAPONSELECTDRAWTIME() = cg.time;

	if ( cg.predictedPlayerState.pm_type >= 6 ) {
		return;
	}

	color = CG_FadeColor( cg.weaponSelectTime, WEAPON_SELECT_TIME );
	if ( !color ) {
		return;
	}

	Controls_GetConfig();

	drawColor[0] = 0.5f;
	drawColor[1] = 0.5f;
	drawColor[2] = 0.5f;
	drawColor[3] = color[3];
	trap_R_SetColor( drawColor );

	weapon = cg_weaponSelect.integer;
	selectedSlot = BG_IsPlayerWeaponInSlot( &cg.predictedPlayerState, weapon, qtrue );

	x = 632.0f;
	for ( slot = 5; slot >= 1; slot-- ) {
		int slotWeapon = (signed char)cg.predictedPlayerState.weaponslots[slot];

		if ( !slotWeapon
			 || !( cg.predictedPlayerState.weapons[slotWeapon >> 5] & ( 1 << ( slotWeapon & 31 ) ) ) ) {
			x -= 32.0f;
			CG_SLOTFADE()[slot] = 0.0f;
			x -= 2.0f;
			continue;
		}

		weaponInfo = bg_weaponInfo[slotWeapon];
		cgWeapon = &cg_weapons[ slotWeapon ];

		if ( !cgWeapon->hudIcon ) {
			x -= 32.0f;
			CG_DrawPic( x, 8.0f, 32.0f, 32.0f, cgs.media.hudNoWeaponIcon );
			CG_SLOTFADE()[slot] = 0.0f;
			x -= 2.0f;
			continue;
		}

		if ( selectedSlot != slot ) {
			if ( CG_SLOTFADE()[slot] > 0.0f && msec ) {
				CG_SLOTFADE()[slot] -= (float)msec * 0.01f;
				if ( CG_SLOTFADE()[slot] < 0.0f ) {
					CG_SLOTFADE()[slot] = 0.0f;
				}
			}
			height = ( CG_SLOTFADE()[slot] + 1.0f ) * 32.0f;
			width = height;
			if ( weaponInfo->wideListIcon ) {
				width = height + height;
			}
			x -= width;
			CG_DrawPic( x, 8.0f, width, height, cgWeapon->hudIcon );
			CG_DrawWeapSlotBinding( slot, drawColor, x, 8.0f );
			x -= 2.0f;
			continue;
		}

		/* the selected slot */
		if ( CG_SLOTFADE()[slot] < 1.0f && msec ) {
			CG_SLOTFADE()[slot] += (float)msec * 0.01f;
			if ( CG_SLOTFADE()[slot] > 1.0f ) {
				CG_SLOTFADE()[slot] = 1.0f;
			}
		}

		numAlts = 1;
		alt = bg_weaponInfo[weapon]->altWeaponIndex;
		while ( alt && alt != weapon ) {
			alt = bg_weaponInfo[alt]->altWeaponIndex;
			numAlts++;
		}

		if ( numAlts == 1 ) {
			cg_weaponSelectSpin = 0.0f;
			cg_weaponSelectSpinSlot = selectedSlot;
			cg_weaponSelectSpinWeapon = weapon;

			drawColor[0] = 1.0f;
			drawColor[1] = 1.0f;
			drawColor[2] = 1.0f;
			trap_R_SetColor( drawColor );

			height = ( CG_SLOTFADE()[slot] + 1.0f ) * 32.0f;
			width = height;
			if ( weaponInfo->wideListIcon ) {
				width = height + height;
			}
			x -= width;
			CG_DrawPic( x, 8.0f, width, height, cgWeapon->hudIcon );

			drawColor[0] = 0.5f;
			drawColor[1] = 0.5f;
			drawColor[2] = 0.5f;
			trap_R_SetColor( drawColor );
			CG_DrawWeapSlotBinding( slot, drawColor, x, 8.0f );
			x -= 2.0f;
			continue;
		}

		/* more than one weapon on the alt chain: arm and decay the spin */
		if ( cg_weaponSelectSpinSlot != selectedSlot ) {
			cg_weaponSelectSpinSlot = selectedSlot;
			cg_weaponSelectSpinWeapon = weapon;
			cg_weaponSelectSpin = 0.0f;
		} else {
			if ( cg_weaponSelectSpinWeapon != weapon ) {
				if ( bg_weaponInfo[cg_weaponSelectSpinWeapon]->altWeaponIndex == weapon ) {
					cg_weaponSelectSpin = 1.0f;
				} else {
					cg_weaponSelectSpin = -1.0f;
				}
				cg_weaponSelectSpinWeapon = weapon;
			}
			if ( cg_weaponSelectSpin != 0.0f ) {
				if ( cg_weaponSelectSpin < 0.0f ) {
					cg_weaponSelectSpin += (float)msec * 0.0066666668f;
					if ( cg_weaponSelectSpin > 0.0f ) {
						cg_weaponSelectSpin = 0.0f;
					}
				} else {
					cg_weaponSelectSpin -= (float)msec * 0.0066666668f;
					if ( cg_weaponSelectSpin < 0.0f ) {
						cg_weaponSelectSpin = 0.0f;
					}
				}
			}
		}

		size = ( CG_SLOTFADE()[slot] + 1.0f ) * 32.0f;
		width = size;
		if ( weaponInfo->wideListIcon ) {
			width = size + size;
		}

		positions[1] = 8.0f;
		positions[2] = size + 10.0f;
		for ( index = 2; index < numAlts; index++ ) {
			positions[index + 1] = positions[index] + 34.0f;
		}

		x -= width * 0.5f;

		/* the alternates, walking the chain forwards from the selected one */
		alt = bg_weaponInfo[weapon]->altWeaponIndex;
		while ( alt && alt != weapon ) {
			const weaponInfo_t  *altInfo = bg_weaponInfo[alt];
			const cgWeaponInfo_t    *altWeapon = &cg_weapons[ alt ];
			float iconWidth = 32.0f;

			height = 32.0f;
			if ( altInfo->wideListIcon ) {
				iconWidth = 64.0f;
			}

			if ( cg_weaponSelectSpin != 0.0f ) {
				spinAbs = (float)fabs( cg_weaponSelectSpin );
				index = ( cg_weaponSelectSpin < 0.0f ? 0 : 2 ) % numAlts;
				pos = positions[2] - ( positions[2] - positions[1 + index] ) * spinAbs;
				if ( cg_weaponSelectSpin > 0.0f
					 || ( numAlts - 1 == 1 && cg_weaponSelectSpin < 0.0f ) ) {
					height = ( spinAbs * CG_SLOTFADE()[slot] + 1.0f ) * 32.0f;
					iconWidth = iconWidth * ( spinAbs * CG_SLOTFADE()[slot] + 1.0f );
				}
			} else {
				pos = positions[2];
			}

			trap_R_DrawStretchPic( ( x - iconWidth * 0.5f ) * cgs.screenXScale,
								   pos * cgs.screenYScale,
								   iconWidth * cgs.screenXScale,
								   height * cgs.screenYScale,
								   0.0f, 0.0f, 1.0f, 1.0f, altWeapon->hudIcon );

			alt = altInfo->altWeaponIndex;
		}

		/* and the selected weapon itself, on top */
		drawColor[0] = 1.0f;
		drawColor[1] = 1.0f;
		drawColor[2] = 1.0f;
		trap_R_SetColor( drawColor );

		size = 32.0f;
		if ( weaponInfo->wideListIcon ) {
			size = 64.0f;
		}
		if ( cg_weaponSelectSpin != 0.0f ) {
			spinAbs = (float)fabs( cg_weaponSelectSpin );
			index = ( 1 - ( cg_weaponSelectSpin < 0.0f ? 2 : 0 ) ) % numAlts;
			pos = positions[1] - ( positions[1] - positions[1 + index] ) * spinAbs;
			height = ( 1.0f - spinAbs ) * CG_SLOTFADE()[slot] + 1.0f;
		} else {
			pos = positions[1];
			height = CG_SLOTFADE()[slot] + 1.0f;
		}
		CG_DrawPic( x - size * height * 0.5f, pos, size * height, height * 32.0f,
					cgWeapon->hudIcon );
		x -= width;

		drawColor[0] = 0.5f;
		drawColor[1] = 0.5f;
		drawColor[2] = 0.5f;
		trap_R_SetColor( drawColor );
		CG_DrawWeapSlotBinding( slot, drawColor, x, 8.0f );
		x -= 2.0f;
	}
}

/*
==============
CG_WeaponSelectable
==============
*/
static qboolean CG_WeaponSelectable( int weapon ) {
	return ( cg.predictedPlayerState.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) != 0;
}

/*
==============
CG_FinishWeaponChange

Hand the choice to the engine through cg_weaponSelect, and drop out of walk
mode unless this is a switch to the same weapon's alternate.
==============
*/
/* NOT static: cg_servercmds_mp.c's CG_ServerCommand calls it too (0x3002E139). */
void CG_FinishWeaponChange( int oldWeapon, int newWeapon ) {
	qboolean isAlt;

	cg.weaponSelectTime = cg.time;
	if ( oldWeapon == newWeapon ) {
		return;
	}

	isAlt = qfalse;
	if ( newWeapon && bg_weaponInfo[oldWeapon]->altWeaponIndex == newWeapon ) {
		isAlt = qtrue;
	}

	trap_Cvar_Set( "cg_weaponSelect", va( "%i", newWeapon ) );

	if ( !isAlt ) {
		trap_Cvar_Set( "cl_run", "1" );
	}
}

/*
==============
CG_AltWeapon_f
==============
*/
void CG_AltWeapon_f( void ) {
	int alt;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.predictedPlayerState.pm_flags & 0x4000 ) {
		return;
	}
	if ( !( cg.snap->ps.pm_flags & PMF_WEAPONSELECTABLE ) ) {
		return;
	}
	if ( cg.predictedPlayerState.eFlags & EF_WEAPONLOCKED ) {
		return;
	}
	if ( cg.time - cg.weaponSelectTime < cg_weaponCycleDelay.integer ) {
		return;
	}

	alt = bg_weaponInfo[cg_weaponSelect.integer]->altWeaponIndex;
	if ( !alt ) {
		CG_GameMessage( trap_SE_LocalizeMessage(
							"CGAME_THIS_WEAPON_HAS_NO_ALTERNATE", "game message" ) );
		return;
	}

	if ( CG_WeaponSelectable( alt ) ) {
		CG_FinishWeaponChange( cg_weaponSelect.integer, alt );
	}
}

/*
==============
CG_SelectWeaponIndex
==============
*/
static void CG_SelectWeaponIndex( int weapon ) {
	CG_FinishWeaponChange( cg_weaponSelect.integer, weapon );
}

/*
==============
CG_NextWeapon_f

Doubles as the scoreboard scroll-up when the scoreboard has focus.
==============
*/
void CG_NextWeapon_f( void ) {
	if ( cg.showScores ) {
		if ( cg.scoreboardScrollPos <= 0 ) {
			return;
		}
		cg.scoreboardScrollPos -= cg_scoreboardScrollStep.integer;
		if ( cg.scoreboardScrollPos < 0 ) {
			cg.scoreboardScrollPos = 0;
		}
		return;
	}

	if ( !cg.snap ) {
		return;
	}
	if ( cg.predictedPlayerState.pm_flags & 0x4000 ) {
		return;
	}
	if ( !( cg.snap->ps.pm_flags & PMF_WEAPONSELECTABLE ) ) {
		return;
	}
	if ( cg.time - cg.weaponSelectTime < cg_weaponCycleDelay.integer ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	CG_CycleWeap( qtrue, qfalse );
}

/*
==============
CG_PrevWeapon_f
==============
*/
void CG_PrevWeapon_f( void ) {
	if ( cg.showScores ) {
		if ( !cg.scoreboardListFull ) {
			return;
		}
		cg.scoreboardScrollPos += cg_scoreboardScrollStep.integer;
		if ( cg.scoreboardScrollPos > cg.numScores - 1 ) {
			cg.scoreboardScrollPos = cg.numScores - 1;
		}
		return;
	}

	if ( !cg.snap ) {
		return;
	}
	if ( cg.predictedPlayerState.pm_flags & 0x4000 ) {
		return;
	}
	if ( !( cg.snap->ps.pm_flags & PMF_WEAPONSELECTABLE ) ) {
		return;
	}
	if ( cg.time - cg.weaponSelectTime < cg_weaponCycleDelay.integer ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	CG_CycleWeap( qfalse, qfalse );
}

/*
==============
CG_Weapon_f

"weapon <name|index>"
==============
*/
void CG_Weapon_f( void ) {
	int weapon;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.predictedPlayerState.pm_flags & 0x4000 ) {
		return;
	}
	if ( !( cg.snap->ps.pm_flags & PMF_WEAPONSELECTABLE ) ) {
		return;
	}

	trap_Argv( 1, cg_cmdArgBuffer, sizeof( cg_cmdArgBuffer ) );
	weapon = (unsigned char)BG_GetWeaponIndexForName( cg_cmdArgBuffer );
	if ( !weapon ) {
		trap_Argv( 1, cg_cmdArgBuffer, sizeof( cg_cmdArgBuffer ) );
		weapon = atoi( cg_cmdArgBuffer );
		if ( !weapon ) {
			return;
		}
	}

	CG_FinishWeaponChange( cg_weaponSelect.integer, weapon );
}

/*
==============
CG_OutOfAmmoChange            0x300381B0..0x300382E6

The weapon just ran dry.  Prefer another weapon of the same slot when the slot
stacks; otherwise walk the five stackable slots for anything with ammo; failing
both, hand it to CG_CycleWeap.

Two tail chunks of CG_EntityEvent in retail (0x300381B0..0x30038278 and
0x30038280..0x300382E6): EV_NOAMMO and EV_DROPWEAPON tail-jump into it at
0x3001E241, so it has no function entry of its own.

CG_WeaponSelectable is inlined at 0x300381E5 and the ammo test is inlined
twice.  The five slot numbers really are a local array: MSVC materialises
{1,2,3,4,5} on the stack (0x3003824C..0x30038274) and indexes it, then reloads
slots[i] at the hit (0x300382C9) rather than keeping the weapon in a register.
==============
*/
void CG_OutOfAmmoChange( void ) {
	const weaponInfo_t  *weaponInfo;
	int slots[5];
	int i;
	int weapon;

	if ( !cg.snap ) {
		return;
	}

	weaponInfo = cg.weaponInfo;

	if ( weaponInfo->slotStackable ) {
		for ( i = 1; i <= bg_numWeapons; i++ ) {
			if ( !CG_WeaponSelectable( i ) ) {
				continue;
			}
			if ( !bg_weaponInfo[i]->slotStackable ) {
				continue;
			}
			if ( bg_weaponInfo[i]->weaponSlot != weaponInfo->weaponSlot ) {
				continue;
			}
			if ( cg.predictedPlayerState.ammoclip[bg_weaponInfo[i]->clipIndex]
				 + cg.predictedPlayerState.ammo[bg_weaponInfo[i]->ammoIndex] ) {
				CG_FinishWeaponChange( cg_weaponSelect.integer, i );
				return;
			}
		}
	}

	if ( BG_IsPlayerWeaponInSlot( &cg.predictedPlayerState, weaponInfo->weapIndex, qtrue ) ) {
		slots[0] = 1;
		slots[1] = 2;
		slots[2] = 3;
		slots[3] = 4;
		slots[4] = 5;

		for ( i = 0; i < 5; i++ ) {
			weapon = (signed char)cg.predictedPlayerState.weaponslots[slots[i]];
			if ( !weapon ) {
				continue;
			}
			if ( cg.predictedPlayerState.ammoclip[bg_weaponInfo[weapon]->clipIndex]
				 + cg.predictedPlayerState.ammo[bg_weaponInfo[weapon]->ammoIndex] ) {
				CG_FinishWeaponChange( cg_weaponSelect.integer,
									   (signed char)cg.predictedPlayerState.weaponslots[slots[i]] );
				return;
			}
		}
	}

	CG_CycleWeap( qtrue, qtrue );
}

/*
==============
CG_WeaponSlot_f

"weaponslot <name|index>"
==============
*/
void CG_WeaponSlot_f( void ) {
	int slot;
	int weapon;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.predictedPlayerState.pm_flags & 0x4000 ) {
		return;
	}
	if ( !( cg.snap->ps.pm_flags & PMF_WEAPONSELECTABLE ) ) {
		return;
	}
	if ( cg.time - cg.weaponSelectTime < cg_weaponCycleDelay.integer ) {
		return;
	}
	cg.weaponSelectTime = cg.time;

	trap_Argv( 1, cg_cmdArgBuffer, sizeof( cg_cmdArgBuffer ) );
	slot = BG_GetWeaponSlotForName( cg_cmdArgBuffer );
	if ( !slot ) {
		slot = atoi( CG_Argv( 1 ) );
	}
	if ( slot <= 0 || slot >= WEAPSLOT_NUM ) {
		return;
	}

	weapon = (signed char)cg.predictedPlayerState.weaponslots[slot];
	if ( !weapon ) {
		return;
	}
	if ( !CG_WeaponSelectable( weapon ) ) {
		return;
	}

	CG_FinishWeaponChange( cg_weaponSelect.integer, weapon );
}

/*
==============
CG_SelectFirstWeaponInSlot

Walk the six slots in `next`'s direction and select the first one holding a
weapon; with checkAmmo set, one that still has ammunition.
==============
*/
static qboolean CG_SelectFirstWeaponInSlot( qboolean next, qboolean checkAmmo ) {
	const weaponInfo_t  *weaponInfo;
	int slot;
	int step;
	int weapon;

	if ( next ) {
		step = 1;
		slot = 1;
	} else {
		step = -1;
		slot = 5;
	}

	do {
		weapon = (signed char)cg.predictedPlayerState.weaponslots[slot];
		if ( weapon ) {
			if ( !checkAmmo ) {
				break;
			}
			weaponInfo = bg_weaponInfo[weapon];
			if ( cg.predictedPlayerState.ammoclip[weaponInfo->clipIndex]
				 + cg.predictedPlayerState.ammo[weaponInfo->ammoIndex] ) {
				break;
			}
		}
		slot += step;
	} while ( slot && slot != WEAPSLOT_NUM );

	if ( !slot || slot == WEAPSLOT_NUM ) {
		return qfalse;
	}

	CG_FinishWeaponChange( cg_weaponSelect.integer,
						   (signed char)cg.predictedPlayerState.weaponslots[slot] );
	return qtrue;
}

/*
==============
CG_SelectFirstWeaponNotInSlot

The fallback when nothing in the slot bar will do: walk every weapon the player
owns and take the first one that is not already occupying a slot -- unless the
slot its class would use is filled by a non-stackable weapon.
==============
*/
static qboolean CG_SelectFirstWeaponNotInSlot( qboolean next, qboolean checkAmmo ) {
	const weaponInfo_t  *weaponInfo;
	int weapon;
	int step;
	int occupant;

	if ( next ) {
		weapon = 1;
		step = 1;
	} else {
		weapon = bg_numWeapons;
		step = -1;
	}

	do {
		if ( !( cg.predictedPlayerState.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
			weapon += step;
			continue;
		}
		if ( BG_IsPlayerWeaponInSlot( &cg.predictedPlayerState, weapon, qtrue ) ) {
			weapon += step;
			continue;
		}

		weaponInfo = bg_weaponInfo[weapon];
		if ( weaponInfo->slotStackable ) {
			switch ( weaponInfo->weaponSlot ) {
			case WEAPSLOT_PRIMARY:
			case WEAPSLOT_PRIMARYB:
				occupant = (signed char)cg.predictedPlayerState.weaponslots[WEAPSLOT_PRIMARY];
				if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
					break;
				}
				occupant = (signed char)cg.predictedPlayerState.weaponslots[WEAPSLOT_PRIMARYB];
				if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
					break;
				}
				weapon += step;
				continue;
			case WEAPSLOT_PISTOL:
			case WEAPSLOT_GRENADE:
			case WEAPSLOT_SMOKEGRENADE:
				occupant = (signed char)cg.predictedPlayerState.weaponslots[weaponInfo->weaponSlot];
				if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
					break;
				}
				weapon += step;
				continue;
			default:
				break;
			}
		}

		if ( checkAmmo
			 && !( cg.predictedPlayerState.ammoclip[weaponInfo->clipIndex]
				   + cg.predictedPlayerState.ammo[weaponInfo->ammoIndex] ) ) {
			weapon += step;
			continue;
		}

		CG_FinishWeaponChange( cg_weaponSelect.integer, weapon );
		return qtrue;
	} while ( weapon && weapon != bg_numWeapons );

	return qfalse;
}

/*
==============
CG_CycleWeap

The weapon cycle behind "weapnext" / "weapprev".  If the current weapon sits in
a slot, step round the slot bar; otherwise step round the weapon list itself,
skipping every weapon that is only an alternate of one already there.
==============
*/
void CG_CycleWeap( qboolean next, qboolean checkAmmo ) {
	const weaponInfo_t  *weaponInfo;
	int slot, endSlot, step, endWeapon;
	int weapon;
	int alt;
	int occupant;

	if ( !cg.snap ) {
		return;
	}
	if ( !( cg.snap->ps.pm_flags & PMF_WEAPONSELECTABLE ) ) {
		return;
	}

	if ( next ) {
		step = 1;
		endSlot = 1;
		endWeapon = 1;
	} else {
		step = -1;
		endSlot = 5;
		endWeapon = bg_numWeapons;
	}

	weapon = cg_weaponSelect.integer;
	slot = BG_IsPlayerWeaponInSlot( &cg.predictedPlayerState, weapon, qtrue );
	if ( !slot ) {
		slot = BG_GetStackSlotForWeapon( &cg.predictedPlayerState, weapon, WEAPSLOT_NONE );
	}

	if ( slot ) {
		slot = ( slot + step + 4 ) % 5 + 1;
		while ( slot != endSlot ) {
			int slotWeapon = (signed char)cg.predictedPlayerState.weaponslots[slot];

			if ( slotWeapon ) {
				if ( !checkAmmo ) {
					CG_FinishWeaponChange( weapon, slotWeapon );
					return;
				}
				weaponInfo = bg_weaponInfo[slotWeapon];
				if ( cg.predictedPlayerState.ammoclip[weaponInfo->clipIndex]
					 + cg.predictedPlayerState.ammo[weaponInfo->ammoIndex] ) {
					CG_FinishWeaponChange( weapon, slotWeapon );
					return;
				}
			}
			slot = ( slot + step + 4 ) % 5 + 1;
		}

		if ( CG_SelectFirstWeaponNotInSlot( next, checkAmmo ) ) {
			return;
		}
		CG_SelectFirstWeaponInSlot( next, checkAmmo );
	} else {
		while ( 1 ) {
			weapon = ( bg_numWeapons + step - 1 + weapon ) % bg_numWeapons + 1;
			if ( weapon == endWeapon ) {
				break;
			}
			if ( !( cg.predictedPlayerState.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
				continue;
			}

			weaponInfo = bg_weaponInfo[weapon];

			/* skip a weapon that is only an alternate of the one selected */
			alt = weaponInfo->altWeaponIndex;
			while ( alt ) {
				if ( alt == cg_weaponSelect.integer ) {
					break;
				}
				if ( alt == weapon ) {
					break;
				}
				alt = bg_weaponInfo[alt]->altWeaponIndex;
			}
			if ( alt && alt == cg_weaponSelect.integer ) {
				continue;
			}

			if ( BG_IsPlayerWeaponInSlot( &cg.predictedPlayerState, weapon, qtrue ) ) {
				continue;
			}

			if ( weaponInfo->slotStackable ) {
				switch ( weaponInfo->weaponSlot ) {
				case WEAPSLOT_PRIMARY:
				case WEAPSLOT_PRIMARYB:
					occupant = (signed char)cg.predictedPlayerState.weaponslots[WEAPSLOT_PRIMARY];
					if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
						break;
					}
					occupant = (signed char)cg.predictedPlayerState.weaponslots[WEAPSLOT_PRIMARYB];
					if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
						break;
					}
					continue;
				case WEAPSLOT_PISTOL:
				case WEAPSLOT_GRENADE:
				case WEAPSLOT_SMOKEGRENADE:
					occupant = (signed char)cg.predictedPlayerState.weaponslots[weaponInfo->weaponSlot];
					if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
						break;
					}
					continue;
				default:
					break;
				}
			}

			if ( checkAmmo
				 && !( cg.predictedPlayerState.ammoclip[weaponInfo->clipIndex]
					   + cg.predictedPlayerState.ammo[weaponInfo->ammoIndex] ) ) {
				continue;
			}

			CG_FinishWeaponChange( cg_weaponSelect.integer, weapon );
			return;
		}

		if ( CG_SelectFirstWeaponInSlot( next, checkAmmo ) ) {
			return;
		}
		CG_SelectFirstWeaponNotInSlot( next, checkAmmo );
	}

	/* Nothing selectable is left: clear the selection outright. */
	weapon = cg_weaponSelect.integer;
	if ( cg.predictedPlayerState.weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) {
		return;
	}
	cg.weaponSelectTime = cg.time;
	if ( !weapon ) {
		return;
	}
	trap_Cvar_Set( "cg_weaponSelect", va( "%i", 0 ) );
	trap_Cvar_Set( "cl_run", "1" );
}

/*
=============================================================================

	FIRING

=============================================================================
*/

/*
==============
CG_WeaponFireRecoil

One shot's worth of view kick and gun kick, each a random draw between the
weapon's min and max for the stance the sights are in.
==============
*/
static void CG_WeaponFireRecoil( void ) {
	const weaponInfo_t  *weaponInfo;
	float frac;
	float pitch, yaw;

	weaponInfo = cg.weaponInfo;
	frac = cg.predictedPlayerState.fWeaponPosFrac;

	if ( frac == 1.0f ) {
		pitch = ( (float)rand() * ( 1.0f / 32768.0f ) )
				* ( weaponInfo->adsViewKickPitchMax - weaponInfo->adsViewKickPitchMin )
				+ weaponInfo->adsViewKickPitchMin;
		yaw = ( (float)rand() * ( 1.0f / 32768.0f ) )
			  * ( weaponInfo->adsViewKickYawMax - weaponInfo->adsViewKickYawMin )
			  + weaponInfo->adsViewKickYawMin;
	} else {
		pitch = ( (float)rand() * ( 1.0f / 32768.0f ) )
				* ( weaponInfo->hipViewKickPitchMax - weaponInfo->hipViewKickPitchMin )
				+ weaponInfo->hipViewKickPitchMin;
		yaw = ( (float)rand() * ( 1.0f / 32768.0f ) )
			  * ( weaponInfo->hipViewKickYawMax - weaponInfo->hipViewKickYawMin )
			  + weaponInfo->hipViewKickYawMin;
	}

	cg.kick_angles[0] = -pitch;
	cg.kick_angles[1] = yaw;
	cg.kick_angles[2] = yaw * -0.5f;

	if ( frac > 0.0f ) {
		pitch = ( (float)rand() * ( 1.0f / 32768.0f ) )
				* ( weaponInfo->adsGunKickPitchMax - weaponInfo->adsGunKickPitchMin )
				+ weaponInfo->adsGunKickPitchMin;
		yaw = ( (float)rand() * ( 1.0f / 32768.0f ) )
			  * ( weaponInfo->adsGunKickYawMax - weaponInfo->adsGunKickYawMin )
			  + weaponInfo->adsGunKickYawMin;
	} else {
		pitch = ( (float)rand() * ( 1.0f / 32768.0f ) )
				* ( weaponInfo->hipGunKickPitchMax - weaponInfo->hipGunKickPitchMin )
				+ weaponInfo->hipGunKickPitchMin;
		yaw = ( (float)rand() * ( 1.0f / 32768.0f ) )
			  * ( weaponInfo->hipGunKickYawMax - weaponInfo->hipGunKickYawMin )
			  + weaponInfo->hipGunKickYawMin;
	}

	CG_RECOILPITCHSPEED() = CG_RECOILPITCHSPEED() + pitch;
	CG_RECOILYAWSPEED() = CG_RECOILYAWSPEED() + yaw;
}

/*
==============
CG_EjectWeaponBrass
==============
*/
void CG_EjectWeaponBrass( entityState_t *es, int event ) {
	const cgWeaponInfo_t    *cgWeapon;
	int bolt[2];
	int weapon;
	int effect;

	if ( !cg_brass.integer ) {
		return;
	}
	if ( es->eType >= ET_EVENTS ) {
		return;
	}

	weapon = es->weapon;
	if ( !weapon ) {
		return;
	}
	if ( weapon > bg_numWeapons ) {
		CG_Error( "CG_FireWeapon: ent->weapon > BG_GetNumWeapons()" );
		return;
	}

	cgWeapon = &cg_weapons[ weapon ];

	if ( cgWeapon->lastShotEjectEffect && event == EV_FIRE_WEAPON_LASTSHOT ) {
		effect = cgWeapon->lastShotEjectEffect;
	} else if ( cgWeapon->shellEjectEffect ) {
		effect = cgWeapon->shellEjectEffect;
	} else {
		return;
	}

	if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
		 && es->number == cg.snap->ps.clientNum ) {
		bolt[0] = CG_WeaponDObjHandle( weapon );
	} else {
		bolt[0] = es->number;
	}

	bolt[1] = trap_syscall_0xDF( bolt[0], "tag_brass" );
	if ( bolt[1] < 0 ) {
		return;
	}

	trap_syscall_0xE5( effect, (int)cg_brassOrigin, 0, (int)bolt );
}

/*
==============
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event.
==============
*/
void CG_FireWeapon( centity_t *cent, entityState_t *es, int event, int tagIndex ) {
	const cgWeaponInfo_t    *cgWeapon;
	const char *sound;
	vec3_t origin;
	float tagMatrix[16];
	int obj;

	if ( !es->weapon ) {
		return;
	}
	if ( es->weapon > bg_numWeapons ) {
		CG_Error( "CG_FireWeapon: ent->weapon > BG_GetNumWeapons()" );
		return;
	}

	cgWeapon = &cg_weapons[ es->weapon ];

	/* mark the entity as muzzle flashing, so when it is added it will
	   append the flash to the weapon model */
	CENT_MUZZLEFLASH( cent ) = 1;

	if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
		 && es->number == cg.snap->ps.clientNum ) {
		CG_WeaponFireRecoil();
	}

	if ( es->eType == ET_TURRET ) {
		CG_WeaponFlash( es->weapon, qfalse, es->number, cent->lerpOrigin,
						cg_flashTags[tagIndex] );
	}

	sound = cgWeapon->fireSound;
	if ( cgWeapon->lastShotSound && event == EV_FIRE_WEAPON_LASTSHOT ) {
		sound = cgWeapon->lastShotSound;
	}

	if ( sound ) {
		if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
			 && es->number == cg.snap->ps.clientNum
			 && cgWeapon->dobj
			 && CG_DObjGetViewModelTagMatrix( cgWeapon->dobj, "tag_flash", tagMatrix ) ) {
			origin[0] = tagMatrix[12];
			origin[1] = tagMatrix[13];
			origin[2] = tagMatrix[14];
		} else if ( ( obj = trap_syscall_0xA2( es->number ) ) != 0
					&& CG_DObjGetWorldTagMatrix( obj, "tag_flash", cent, tagMatrix ) ) {
			origin[0] = tagMatrix[12];
			origin[1] = tagMatrix[13];
			origin[2] = tagMatrix[14];
		} else {
			BG_EvaluateTrajectory( &es->pos, cg.time, origin );
		}
		CG_PlaySoundAliasByName( sound, es->number, origin );
	}

	if ( !bg_weaponInfo[es->weapon]->boltAction ) {
		CG_EjectWeaponBrass( es, event );
	}
}

/*
=============================================================================

	BULLET EFFECTS

=============================================================================
*/

/*
==============
CG_WaterRipple

Uncalled in 1.1 -- kept because the linker kept it.
==============
*/
static void CG_WaterRipple( qhandle_t shader, const vec3_t origin, int radius, int duration ) {
	localEntity_t   *le;
	refEntity_t     *re;

	le = CG_AllocLocalEntity();
	re = LE_REFENTITY( le );

	le->leType = LE_SCALE_FADE;
	le->leFlags = 1;
	le->endTime = cg.time + duration;
	le->lifeRate = 1.0f / (float)duration;

	VectorCopy( origin, re->origin );
	re->reType = RT_SPRITE_LOCAL;
	RE_CUSTOMSHADER( re ) = shader;
	re->shaderRGBA[0] = 255;
	re->shaderRGBA[1] = 255;
	re->shaderRGBA[2] = 255;
	re->shaderRGBA[3] = 255;
	RE_SHADERTIME( re ) = (float)cg.time * 0.001f;
	RE_RADIUS( re ) = (float)radius;
	LE_COLOR( le )[3] = 1.0f;
}

/*
==============
CG_WhizbySound

A bullet that passes close to the listener gets a whizby, played 16 units back
along its path from the point of closest approach.
==============
*/
static void CG_WhizbySound( const vec3_t start, const vec3_t end ) {
	vec3_t delta;
	vec3_t dir;
	vec3_t toEar;
	vec3_t point;
	float along;
	float length;
	float dist;

	VectorSubtract( end, start, delta );
	VectorNormalize2( delta, dir );

	VectorSubtract( cg.refdef.vieworg, start, toEar );
	along = DotProduct( toEar, dir );
	if ( along < 64.0f ) {
		return;
	}

	/* the point of closest approach has to be far enough inside the trace */
	length = DotProduct( dir, delta );
	if ( length < along + 64.0f ) {
		return;
	}

	point[0] = along * dir[0] + start[0];
	point[1] = along * dir[1] + start[1];
	point[2] = along * dir[2] + start[2];

	dist = (float)sqrt( ( point[0] - cg.refdef.vieworg[0] ) * ( point[0] - cg.refdef.vieworg[0] )
						+ ( point[1] - cg.refdef.vieworg[1] ) * ( point[1] - cg.refdef.vieworg[1] )
						+ ( point[2] - cg.refdef.vieworg[2] ) * ( point[2] - cg.refdef.vieworg[2] ) );
	if ( dist >= 140.0f ) {
		return;
	}

	point[0] = point[0] - dir[0] * 16.0f;
	point[1] = point[1] - dir[1] * 16.0f;
	point[2] = point[2] - dir[2] * 16.0f;

	CG_PlaySoundAliasByName( (const char *)cgs.media.whizbySound, ENTITYNUM_WORLD, point );
}

/*
==============
CG_SpawnTracer

A tracer that has to travel: a moving-tracer local entity that starts
cg_tracerlength behind the impact and reaches it at cg_tracerSpeed.
==============
*/
static void CG_SpawnTracer( const vec3_t start, const vec3_t end ) {
	localEntity_t   *le;
	vec3_t tail;
	vec3_t dir;
	vec3_t delta;
	float dist;
	int offset;
	int spawnTime;

	VectorSubtract( end, start, dir );
	VectorNormalize( dir );

	/* the tracer stops cg_tracerlength short of the impact */
	tail[0] = -cg_tracerlength.value * dir[0] + end[0];
	tail[1] = -cg_tracerlength.value * dir[1] + end[1];
	tail[2] = -cg_tracerlength.value * dir[2] + end[2];

	VectorSubtract( tail, start, delta );
	dist = (float)sqrt( delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2] );

	le = CG_AllocLocalEntity();
	le->leType = LE_MOVING_TRACER;

	/* spread the spawn back over the frame so a burst does not arrive at once */
	if ( cg.frametime ) {
		offset = ( rand() % cg.frametime ) / 2;
	} else {
		offset = 0;
	}
	spawnTime = cg.time - offset;

	LE_TRACERTIME( le ) = spawnTime;
	LE_TRACERORIGIN( le )[0] = start[0];
	LE_TRACERORIGIN( le )[1] = start[1];
	LE_TRACERORIGIN( le )[2] = start[2];
	LE_TRACERFLAGS( le ) = 2;
	le->endTime = spawnTime - (int)( dist * -1000.0f / cg_tracerSpeed.value );
	LE_TRACERVELOCITY( le )[0] = cg_tracerSpeed.value * dir[0];
	LE_TRACERVELOCITY( le )[1] = cg_tracerSpeed.value * dir[1];
	LE_TRACERVELOCITY( le )[2] = cg_tracerSpeed.value * dir[2];
}

/*
==============
CG_DrawTracer

One camera-facing quad between two points, cg_tracerwidth wide.
==============
*/
void CG_DrawTracer( const vec3_t start, const vec3_t end ) {
	polyVert_t verts[4];
	vec3_t dir;
	vec3_t right;
	float dot1, dot2;
	float width;

	VectorSubtract( end, start, dir );
	dot1 = DotProduct( dir, cg.refdef.viewaxis[1] );
	dot2 = DotProduct( dir, cg.refdef.viewaxis[2] );

	right[0] = cg.refdef.viewaxis[1][0] * dot2 - cg.refdef.viewaxis[2][0] * dot1;
	right[1] = cg.refdef.viewaxis[1][1] * dot2 - cg.refdef.viewaxis[2][1] * dot1;
	right[2] = cg.refdef.viewaxis[1][2] * dot2 - cg.refdef.viewaxis[2][2] * dot1;
	VectorNormalize( right );

	width = cg_tracerwidth.value;

	VectorMA( end, width, right, verts[0].xyz );
	verts[0].st[0] = 1.0f;
	verts[0].st[1] = 1.0f;
	verts[0].st2[0] = 1.0f;
	verts[0].st2[1] = 1.0f;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( end, -width, right, verts[1].xyz );
	verts[1].st[0] = 1.0f;
	verts[1].st[1] = 0.0f;
	verts[1].st2[0] = 1.0f;
	verts[1].st2[1] = 0.0f;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -width, right, verts[2].xyz );
	verts[2].st[0] = 0.0f;
	verts[2].st[1] = 0.0f;
	verts[2].st2[0] = 0.0f;
	verts[2].st2[1] = 0.0f;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, width, right, verts[3].xyz );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = 1.0f;
	verts[3].st2[0] = 0.0f;
	verts[3].st2[1] = 1.0f;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );
}

/*
==============
CG_Tracer

A tracer for a bullet that arrives inside one frame: drawn straight away, at a
random length between 50 and 60 units, unless the shot was too short to see.
==============
*/
static void CG_Tracer( const vec3_t start, const vec3_t end, int forceDraw ) {
	vec3_t dir;
	vec3_t head, tail;
	float length;
	/* not `near`/`far`: MSVC reserves both (C4226, then C2143 on every use). */
	float nearDist;
	float farDist;

	VectorSubtract( end, start, dir );
	length = VectorNormalize( dir );

	if ( length <= 100.0f && !forceDraw ) {
		return;
	}

	nearDist = ( (float)rand() * ( 1.0f / 32768.0f ) ) * ( length - 60.0f ) + 50.0f;
	farDist = cg_tracerlength.value + nearDist;
	if ( farDist >= length ) {
		farDist = length;
	}

	head[0] = nearDist * dir[0] + start[0];
	head[1] = nearDist * dir[1] + start[1];
	head[2] = nearDist * dir[2] + start[2];

	tail[0] = dir[0] * farDist + start[0];
	tail[1] = dir[1] * farDist + start[1];
	tail[2] = dir[2] * farDist + start[2];

	CG_DrawTracer( head, tail );
}

/*
==============
CG_CalcMuzzlePoint

Where a bullet from `entityNum` left the world.  The local client's own shots
come out of the eye, everything else off the firing model's tag.
==============
*/
static qboolean CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle, const char *tagName ) {
	centity_t   *cent;
	float tagMatrix[16];
	int obj;

	if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
		 && entityNum == cg.snap->ps.clientNum ) {
		muzzle[0] = cg.snap->ps.origin[0];
		muzzle[1] = cg.snap->ps.origin[1];
		muzzle[2] = cg.snap->ps.origin[2] + cg.snap->ps.viewHeightCurrent;
		AddLeanToPosition( muzzle, cg.refdefViewAngles[1], cg.snap->ps.leanf, 16.0f, 20.0f );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	obj = trap_syscall_0xA2( cent->currentState.number );
	if ( !obj ) {
		return qfalse;
	}

	if ( CG_DObjGetWorldTagMatrix( obj, tagName, cent, tagMatrix ) ) {
		muzzle[0] = tagMatrix[12];
		muzzle[1] = tagMatrix[13];
		muzzle[2] = tagMatrix[14];
		return qtrue;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );
	if ( entityNum < MAX_CLIENTS ) {
		Com_Printf( "No %s in CG_CalcMuzzlePoint on entity %d.\n", tagName, entityNum );
		if ( cent->currentState.eFlags & 0x40 ) {
			muzzle[2] += (float)bg_viewheight_prone.integer;
		} else if ( cent->currentState.eFlags & 0x20 ) {
			muzzle[2] += (float)bg_viewheight_crouched.integer;
		} else {
			muzzle[2] += (float)bg_viewheight_standing.integer;
		}
	}
	return qtrue;
}

/*
==============
CG_BulletTrajectoryEffects

The tracer and whizby for one bullet.  The local client's own shots get the
whizby only; everyone else's get a tracer cg_tracerchance of the time.
==============
*/
static void CG_BulletTrajectoryEffects( int entityNum, const vec3_t end, int surfaceType,
										const char *tagName ) {
	vec3_t muzzle;

	if ( cg_tracerchance.value <= 0.0f ) {
		return;
	}
	if ( !CG_CalcMuzzlePoint( entityNum, muzzle, tagName ) ) {
		return;
	}

	if ( ( cg.snap->ps.pm_flags & PMF_FIRSTPERSON )
		 && entityNum == cg.snap->ps.clientNum ) {
		CG_WhizbySound( muzzle, end );
		return;
	}

	if ( cg_tracerchance.value * 32768.0f <= (float)rand() ) {
		CG_WhizbySound( muzzle, end );
		return;
	}

	if ( surfaceType == SURF_TYPE_FLESH ) {
		CG_Tracer( muzzle, end, 0 );
		CG_WhizbySound( muzzle, end );
		return;
	}

	CG_SpawnTracer( muzzle, end );
	CG_WhizbySound( muzzle, end );
}

/*
==============
CG_BulletHitEvent

A bullet that hit the world: the impact sound, the surface's impact and mark
effects, and the trajectory effects behind it.
==============
*/
void CG_BulletHitEvent( int event, const vec3_t origin, int surfaceType, int entityNum,
						const vec3_t dir, const vec3_t dir2 ) {
	const char *sound;
	int impact;
	int mark;

	if ( event == EV_BULLET_HIT_SMALL ) {
		sound = (const char *)CGS_BULLETSOUND_A()[surfaceType];
		impact = CGS_BULLETIMPACT_A()[surfaceType];
		mark = CGS_BULLETMARK_A()[surfaceType];
	} else {
		sound = (const char *)CGS_BULLETSOUND_B()[surfaceType];
		impact = CGS_BULLETIMPACT_B()[surfaceType];
		mark = CGS_BULLETMARK_B()[surfaceType];
	}

	if ( !cg_blood.integer && surfaceType == SURF_TYPE_FLESH ) {
		impact = CGS_FLESHHITNOBLOOD();
		mark = 0;
	}

	CG_PlaySoundAliasByName( sound, ENTITYNUM_WORLD, origin );

	if ( impact ) {
		trap_syscall_0xE4( impact, (int)origin, (int)dir );
	}
	if ( mark ) {
		trap_syscall_0xE4( mark, (int)origin, (int)dir2 );
	}

	CG_BulletTrajectoryEffects( entityNum, origin, surfaceType, cg_flashTags[0] );
}

/*
==============
CG_BulletHitClientEvent

A bullet that hit a player: only the sound and the trajectory effects; the
impact effects come from the hit player's own event.
==============
*/
void CG_BulletHitClientEvent( int event, const vec3_t origin, int surfaceType, int entityNum ) {
	const char *sound;

	if ( event == EV_BULLET_HIT_CLIENT_SMALL ) {
		sound = (const char *)CGS_BULLETSOUND_A()[surfaceType];
	} else {
		sound = (const char *)CGS_BULLETSOUND_B()[surfaceType];
	}

	CG_PlaySoundAliasByName( sound, ENTITYNUM_WORLD, origin );
	CG_BulletTrajectoryEffects( entityNum, origin, surfaceType, cg_flashTags[0] );
}
