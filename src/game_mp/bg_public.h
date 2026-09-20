/*
 * bg_public.h -- definitions shared by the multiplayer game DLL and cgame.
 *
 * Call of Duty 1.1 (game_mp_x86.dll, imagebase 0x20000000).  Covers the five
 * "both games" units: bg_pmove.c, bg_slidemove.c, bg_animation.c, bg_weapon.c
 * and bg_misc.c.  RTCW's game/bg_public.h is the structural model; every
 * layout below carries the retail address that fixes it.
 *
 * COD1_ASSERT_SIZE appears only where an allocation site or an array stride
 * in the DLL fixes the retail size.
 */

#ifndef __BG_PUBLIC_H__
#define __BG_PUBLIC_H__

#include "../universal/q_shared.h"

/*
=============================================================================

						PLAYER MOVEMENT

=============================================================================
*/

/* pm->touchents capacity: PM_AddTouchEnt (0x20007080) stops at 32. */
#define MAXTOUCH 32

/*
 * pmove_t.  248 bytes -- ClientThink_real declares it as a 62-dword stack
 * object and Pmove/PmoveSingle reach 0xF4 at most.
 *
 * The three trace callbacks are distinct source members that /OPT:ICF folded:
 * ClientThink_real stores the same trap wrapper into all three.  trace is the
 * general one (PM_GroundTrace, PM_CorrectAllSolid, PM_FootstepEvent,
 * PM_FoliageSounds); trace2/trace3 are the pair handed to BG_CheckProneValid
 * by PM_VerifyPronePosition (0x2000C830) and PM_CheckDuck.  All three share
 * one signature.
 */
typedef struct pmove_t
{
	playerState_t *ps;
	usercmd_t cmd;
	usercmd_t oldcmd;
	int tracemask;
	int debugLevel;                 /* +0x38  PM_StepSlideMove tests it against 1 and 2 */
	int numtouch;
	int touchents[MAXTOUCH];
	vec3_t mins;
	vec3_t maxs;
	byte watertype;                 /* +0xD8  byte, not int -- ClientThink_real copies both as bytes */
	byte waterlevel;
	byte unknown_0xDA[2];
	float xyspeed;                  /* +0xDC  written by PM_Footsteps, read by PM_FoliageSounds */
	qboolean pmove_fixed;
	int pmove_msec;
	void ( *trace )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	void ( *trace2 )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	void ( *trace3 )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	int ( *pointcontents )( const vec3_t point, int passEntityNum, int contentMask );
} pmove_t;
COD1_ASSERT_SIZE( pmove_t, 248 );

/* pml_t, pm and pml are bg_local.h's: the movement units share them, the
   game/cgame interface does not. */

void Pmove( pmove_t *pmove );
void PmoveSingle( pmove_t *pmove );

/*
=============================================================================

						WEAPONS

=============================================================================
*/

/*
 * 64 weapon bits.  BG_GetConditionValue (0x20003300) walks the ANIM_COND_WEAPONS
 * bit set to 0x40, and bg_itemlist reserves exactly 64 weapon slots.
 */
#define MAX_WEAPONS WEAPON_SLOTS
#define BG_NUM_ITEMS 261
#define ANIM_MASK_WORDS WEAPON_MASK_WORDS

/* Keep stock health/ammo item IDs 65..69 unchanged. */
static int BG_WeaponItemIndex( int weapon ) { return weapon <= 64 ? weapon : weapon + 5; }

/*
 * The weapon-file parse table is bg_weaponInfoFields at 0x2006ADD0: 248
 * {const char *name; int offset; int type;} triples plus a {NULL,-1,-1}
 * sentinel, walked by ParseConfigStringToStruct (0x2003F070) from
 * BG_ParseWeaponInfoFiles.  Field types 0-7 are generic (0 string, 4 int,
 * 5 qboolean, 6 float, 7 seconds-to-milliseconds); 8-13 are dispatched to
 * BG_ParseWeaponInfoSpecificFieldType (0x2000D990) and name the enums below.
 *
 * Constant names in these enums are inferred from convention: only the string
 * spellings are recovered, from the name tables the parser matches against.
 */

/* bg_weaponTypeNames, 0x2006AD5C */
typedef enum weapType_t
{
	WEAPTYPE_BULLET     = 0x0,
	WEAPTYPE_GRENADE    = 0x1,
	WEAPTYPE_PROJECTILE = 0x2,
	WEAPTYPE_NUM        = 0x3,
} weapType_t;

/* bg_weaponClassNames, 0x2006ADA0 */
typedef enum weapClass_t
{
	WEAPCLASS_RIFLE          = 0x0,
	WEAPCLASS_MG             = 0x1,
	WEAPCLASS_SMG            = 0x2,
	WEAPCLASS_PISTOL         = 0x3,
	WEAPCLASS_GRENADE        = 0x4,
	WEAPCLASS_ROCKETLAUNCHER = 0x5,
	WEAPCLASS_TURRET         = 0x6,
	WEAPCLASS_NON_PLAYER     = 0x7,        /* "non-player" */
	WEAPCLASS_NUM            = 0x8,
} weapClass_t;

/* bg_weaponSlotNames, 0x2006AD7C */
typedef enum weapSlot_t
{
	WEAPSLOT_NONE         = 0x0,
	WEAPSLOT_PRIMARY      = 0x1,
	WEAPSLOT_PRIMARYB     = 0x2,
	WEAPSLOT_PISTOL       = 0x3,
	WEAPSLOT_GRENADE      = 0x4,
	WEAPSLOT_SMOKEGRENADE = 0x5,
	WEAPSLOT_NUM          = 0x6,
} weapSlot_t;

/* bg_weaponStanceNames, 0x2006AD94 */
typedef enum weapStance_t
{
	WEAPSTANCE_STAND = 0x0,
	WEAPSTANCE_DUCK  = 0x1,
	WEAPSTANCE_PRONE = 0x2,
	WEAPSTANCE_NUM   = 0x3,
} weapStance_t;

/* bg_weaponOverlayReticleNames, 0x2006AD68 */
typedef enum weapOverlayReticle_t
{
	WEAPOVERLAYRETICLE_NONE        = 0x0,
	WEAPOVERLAYRETICLE_CROSSHAIR   = 0x1,
	WEAPOVERLAYRETICLE_FG42        = 0x2,
	WEAPOVERLAYRETICLE_SPRINGFIELD = 0x3,
	WEAPOVERLAYRETICLE_GEWEHR43    = 0x4,
	WEAPOVERLAYRETICLE_NUM         = 0x5,
} weapOverlayReticle_t;

/* bg_weaponGrenadeTypeNames, 0x2006ADC0 */
typedef enum weapProjExposion_t
{
	WEAPPROJEXP_GRENADE = 0x0,
	WEAPPROJEXP_ROCKET  = 0x1,
	WEAPPROJEXP_MOLOTOV = 0x2,
	WEAPPROJEXP_NONE    = 0x3,
	WEAPPROJEXP_NUM     = 0x4,
} weapProjExposion_t;

/*
 * weaponInfo_t.  1052 bytes -- InitWeaponInfo (0x2000DC80) allocates that
 * much.  Field names and offsets are the parse table's, verbatim; weapIndex
 * and szInternalName are the two members InitWeaponInfo /
 * BG_ParseWeaponInfoFiles fill in themselves.  Every unknown_ range below is
 * a slot the parse table never names and that this DLL never touches (they
 * are cgame-side, except where noted).
 */
typedef struct weaponInfo_t
{
	int weapIndex;
	const char *szInternalName;
	const char *displayName;
	const char *AIOverlayDescription;
	const char *gunModel;
	const char *handModel;
	byte unknown_0x18[4];
	const char *idleAnim;
	const char *emptyIdleAnim;
	const char *fireAnim;
	const char *holdFireAnim;
	const char *lastShotAnim;
	const char *rechamberAnim;
	const char *meleeAnim;
	const char *reloadAnim;
	const char *reloadEmptyAnim;
	const char *reloadStartAnim;
	const char *reloadEndAnim;
	const char *raiseAnim;
	const char *dropAnim;
	const char *altRaiseAnim;
	const char *altDropAnim;
	const char *adsFireAnim;
	const char *adsLastShotAnim;
	const char *adsRechamberAnim;
	const char *adsUpAnim;
	const char *adsDownAnim;
	const char *modeName;
	weapType_t weaponType;
	weapClass_t weaponClass;
	weapSlot_t weaponSlot;
	qboolean slotStackable;
	weapStance_t stance;
	const char *viewFlashEffect;
	const char *worldFlashEffect;
	const char *pickupSound;
	const char *ammoPickupSound;
	const char *projectileSound;
	const char *pullbackSound;
	const char *fireSound;
	const char *loopFireSound;
	const char *stopFireSound;
	const char *fireEchoSound;
	const char *lastShotSound;
	const char *rechamberSound;
	const char *reloadSound;
	const char *reloadEmptySound;
	const char *reloadStartSound;
	const char *reloadEndSound;
	const char *raiseSound;
	const char *altSwitchSound;
	const char *putawaySound;
	const char *noteTrackSoundA;
	const char *noteTrackSoundB;
	const char *noteTrackSoundC;
	const char *noteTrackSoundD;
	const char *shellEjectEffect;
	const char *lastShotEjectEffect;
	const char *reticleCenter;
	const char *reticleSide;
	int reticleCenterSize;
	int reticleSideSize;
	int reticleMinOfs;
	float standMoveF;
	float standMoveR;
	float standMoveU;
	float standRotP;
	float standRotY;
	float standRotR;
	float duckedOfsF;
	float duckedOfsR;
	float duckedOfsU;
	float duckedMoveF;
	float duckedMoveR;
	float duckedMoveU;
	float duckedRotP;
	float duckedRotY;
	float duckedRotR;
	float proneOfsF;
	float proneOfsR;
	float proneOfsU;
	float proneMoveF;
	float proneMoveR;
	float proneMoveU;
	float proneRotP;
	float proneRotY;
	float proneRotR;
	float posMoveRate;
	float posProneMoveRate;
	float standMoveMinSpeed;
	float duckedMoveMinSpeed;
	float proneMoveMinSpeed;
	float posRotRate;
	float posProneRotRate;
	float standRotMinSpeed;
	float duckedRotMinSpeed;
	float proneRotMinSpeed;
	const char *radiantName;
	const char *worldModel;
	const char *hudIcon;
	const char *modeIcon;
	const char *ammoIcon;
	int startAmmo;
	const char *ammoName;
	int ammoIndex;                  /* +0x1A0  BG_SetupAmmoIndexes / BG_AmmoForWeapon */
	const char *clipName;
	int clipIndex;                  /* +0x1A8  BG_SetupClipIndexes / BG_ClipForWeapon */
	int maxAmmo;
	int clipSize;
	const char *sharedAmmoCapName;
	int sharedAmmoCapIndex;         /* +0x1B8  BG_SetupSharedAmmoIndexes */
	int sharedAmmoCap;
	int damage;
	int meleeDamage;
	byte unknown_0x1C8[4];
	int fireDelay;
	int meleeDelay;
	int fireTime;
	int rechamberTime;
	int rechamberBoltTime;
	int holdFireTime;
	int meleeTime;
	int reloadTime;
	int reloadEmptyTime;
	int reloadAddTime;
	int reloadStartTime;
	int reloadStartAddTime;
	int reloadEndTime;
	int dropTime;
	int raiseTime;
	int altDropTime;
	int altRaiseTime;
	int fuseTime;
	float moveSpeedScale;
	float adsZoomFov;
	float adsZoomInFrac;
	float adsZoomOutFrac;
	const char *adsOverlayShader;
	weapOverlayReticle_t adsOverlayReticle;
	float adsOverlayWidth;
	float adsOverlayHeight;
	float adsBobFactor;
	float adsViewBobMult;
	float hipSpreadStandMin;
	float hipSpreadDuckedMin;
	float hipSpreadProneMin;
	float hipSpreadMax;
	float hipSpreadDecayRate;
	float hipSpreadFireAdd;
	float hipSpreadTurnAdd;
	float hipSpreadMoveAdd;
	float hipSpreadDuckedDecay;
	float hipSpreadProneDecay;
	float hipReticleSidePos;
	int adsTransInTime;
	int adsTransOutTime;
	float adsIdleAmount;
	float hipIdleAmount;
	float idleCrouchFactor;
	float idleProneFactor;
	float gunMaxPitch;
	float gunMaxYaw;
	float swayMaxAngle;
	float swayLerpSpeed;
	float swayPitchScale;
	float swayYawScale;
	float swayHorizScale;
	float swayVertScale;
	float swayShellShockScale;
	float adsSwayMaxAngle;
	float adsSwayLerpSpeed;
	float adsSwayPitchScale;
	float adsSwayYawScale;
	float adsSwayHorizScale;
	float adsSwayVertScale;
	qboolean twoHanded;
	qboolean rifleBullet;
	qboolean semiAuto;
	qboolean boltAction;
	qboolean aimDownSight;
	qboolean cookOffHold;
	qboolean clipOnly;
	qboolean wideListIcon;
	qboolean adsFire;
	const char *killIcon;
	qboolean wideKillIcon;
	qboolean noPartialReload;
	qboolean segmentedReload;
	int reloadAmmoAdd;
	int reloadStartAdd;
	const char *altWeapon;
	int altWeaponIndex;             /* +0x2FC  BG_SetupWeaponAlts */
	int dropAmmoMin;
	int dropAmmoMax;
	int explosionRadius;
	int explosionInnerDamage;
	int explosionOuterDamage;
	int projectileSpeed;
	int projectileSpeedUp;
	const char *projectileModel;
	weapProjExposion_t projExplosionType;
	const char *projExplosionEffect;
	const char *projExplosionSound;
	qboolean projImpactExplode;
	const char *projTrailEffect;
	int projectileDLight;
	float projectileRed;
	float projectileGreen;
	float projectileBlue;
	float adsAimPitch;
	float adsCrosshairInFrac;
	float adsCrosshairOutFrac;
	float adsGunKickPitchMin;
	float adsGunKickPitchMax;
	float adsGunKickYawMin;
	float adsGunKickYawMax;
	float adsGunKickAccel;
	float adsGunKickSpeedMax;
	float adsGunKickSpeedDecay;
	float adsGunKickStaticDecay;
	float adsViewKickPitchMin;
	float adsViewKickPitchMax;
	float adsViewKickYawMin;
	float adsViewKickYawMax;
	float adsViewKickCenterSpeed;
	byte unknown_0x384[8];
	float adsSpread;
	float hipGunKickPitchMin;
	float hipGunKickPitchMax;
	float hipGunKickYawMin;
	float hipGunKickYawMax;
	float hipGunKickAccel;
	float hipGunKickSpeedMax;
	float hipGunKickSpeedDecay;
	float hipGunKickStaticDecay;
	float hipViewKickPitchMin;
	float hipViewKickPitchMax;
	float hipViewKickYawMin;
	float hipViewKickYawMax;
	float hipViewKickCenterSpeed;
	byte unknown_0x3C4[8];
	float aiEffectiveRange;
	float aiMissRange;
	int adsReloadTransTime;
	int adsTransBlendTime;
	float leftArc;
	float rightArc;
	float topArc;
	float bottomArc;
	float accuracy;
	float vertTurnSpeed;
	float horTurnSpeed;
	float convergenceTime;
	float maxRange;
	float animHorRotateInc;
	float playerPositionDist;
	const char *useHintString;
	int useHintStringIndex;         /* +0x40C  G_GetHintStringIndex out-param, 0x2000EC74 */
	const char *script;
	/* 1/adsTransInTime and 1/adsTransOutTime, derived by BG_SetupTransitionTimes
	 * (0x2000E060); no parse-table entry names them, so both names are
	 * inferred. */
	float adsTransInRate;           /* +0x414  BG_SetupTransitionTimes; 1/300 when unset */
	float adsTransOutRate;          /* +0x418  1/500 when unset */
} weaponInfo_t;
COD1_ASSERT_SIZE( weaponInfo_t, 1052 );

/* BG_SetupWeaponInfo (0x2000ECE0) stores trap_GetWeaponInfoMemory's result
 * into 0x200A069C and every reader loads that dword before indexing. */
extern weaponInfo_t **bg_weaponInfo;
extern int bg_numWeapons;

weaponInfo_t *BG_GetInfoForWeapon( int weaponIndex );
int BG_GetNumWeapons( void );
int BG_GetNumAmmoTypes( void );
int BG_GetAmmoTypeMax( int ammoIndex );
int BG_GetNumAmmoClips( void );
int BG_GetAmmoClipSize( int clipIndex );
int BG_GetSharedAmmoCapSize( int sharedAmmoCapIndex );
const char *BG_GetAmmoTypeName( int ammoIndex );
const char *BG_GetAmmoClipName( int clipIndex );
const char *BG_GetWeaponTypeName( weapType_t weaponType );
const char *BG_GetWeaponSlotNameForIndex( weapSlot_t weaponSlot );
int BG_AmmoForWeapon( int weaponIndex );
int BG_ClipForWeapon( int weaponIndex );
int BG_WeaponAmmo( int weaponIndex );
qboolean BG_IsAimDownSightWeapon( int weaponIndex );

/*
=============================================================================

						ITEMS

=============================================================================
*/

/*
 * bg_itemlist (0x2006BB68) is 70 entries of 48 bytes: [0] is the IT_BAD
 * placeholder, [1..64] are the emptyitem_"wNN" slots BG_FillInWeaponItems
 * overwrites from the parsed weapon files, [65..69] are the two grenade-ammo
 * and three health items.  BG_CanItemBeGrabbed rejects anything outside
 * [1,70).
 */
#define MAX_ITEM_MODELS 2

/* Only these four are observed: bg_itemlist uses 2 for ammo and 3 for health,
 * BG_FillInWeaponItems writes 1 for weapons, and BG_CanItemBeGrabbed errors
 * out on 0 with "BG_CanItemBeGrabbed: IT_BAD". */
typedef enum itemType_t
{
	IT_BAD    = 0x0,
	IT_WEAPON = 0x1,
	IT_AMMO   = 0x2,
	IT_HEALTH = 0x3,
} itemType_t;

typedef struct gitem_s
{
	const char *classname;
	const char *pickup_sound;
	const char *world_model[MAX_ITEM_MODELS];
	const char *icon;
	const char *ammoicon;
	const char *pickup_name;
	int quantity;
	itemType_t giType;
	int giTag;
	int giAmmoIndex;
	int giClipIndex;
} gitem_t;
COD1_ASSERT_SIZE( gitem_t, 48 );

extern gitem_t bg_itemlist[];

gitem_t *BG_FindItem( const char *pickupName );
gitem_t *BG_FindItemForWeapon( int weaponIndex );

/*
=============================================================================

						ANIMATION SCRIPT SYSTEM

=============================================================================
*/

/*
 * Sizes, and the strides in the DLL that fix them:
 *   animation_t                     92   BG_FinalizePlayerAnims (0x20001500)
 *   animScript_t                   516   BG_AnimScriptAnimation (0x20003020)
 *   animScriptItem_t               244   BG_ParseCommands (0x20001CE0)
 *   MAX_MODEL_ANIMATIONS           512   numAnimations sits at +47104
 *   NUM_ANIM_MOVETYPES              18   scriptAnims row stride 9288
 *   MAX_ANIMSCRIPT_ITEMS_PER_MODEL 2048  cmp [edi+9B6BCh], 800h
 */
#define MAX_MODEL_ANIMATIONS 512
#define MAX_ANIMSCRIPT_ANIMCOMMANDS 8
#define MAX_ANIMSCRIPT_ITEMS 128
#define MAX_ANIMSCRIPT_ITEMS_PER_MODEL 2048

/* animStateStr, 0x2006CBEC */
typedef enum aistateEnum_t
{
	AISTATE_RELAXED = 0x0,
	AISTATE_QUERY   = 0x1,
	AISTATE_ALERT   = 0x2,
	AISTATE_COMBAT  = 0x3,
	MAX_AISTATES    = 0x4,
} aistateEnum_t;

/* animMoveTypesStr, 0x2006CC18 */
typedef enum scriptAnimMoveTypes_t
{
	ANIM_MT_UNUSED      = 0x0,
	ANIM_MT_IDLE        = 0x1,
	ANIM_MT_IDLECR      = 0x2,
	ANIM_MT_IDLEPRONE   = 0x3,
	ANIM_MT_WALK        = 0x4,
	ANIM_MT_WALKBK      = 0x5,
	ANIM_MT_WALKCR      = 0x6,
	ANIM_MT_WALKCRBK    = 0x7,
	ANIM_MT_WALKPRONE   = 0x8,
	ANIM_MT_WALKPRONEBK = 0x9,
	ANIM_MT_RUN         = 0xA,
	ANIM_MT_RUNBK       = 0xB,
	ANIM_MT_RUNCR       = 0xC,
	ANIM_MT_RUNCRBK     = 0xD,
	ANIM_MT_TURNRIGHT   = 0xE,
	ANIM_MT_TURNLEFT    = 0xF,
	ANIM_MT_CLIMBUP     = 0x10,
	ANIM_MT_CLIMBDOWN   = 0x11,
	NUM_ANIM_MOVETYPES  = 0x12,
} scriptAnimMoveTypes_t;

/* animEventTypesStr, 0x2006CCB0 */
typedef enum scriptAnimEventTypes_t
{
	ANIM_ET_PAIN            = 0x0,
	ANIM_ET_DEATH           = 0x1,
	ANIM_ET_FIREWEAPON      = 0x2,
	ANIM_ET_JUMP            = 0x3,
	ANIM_ET_JUMPBK          = 0x4,
	ANIM_ET_LAND            = 0x5,
	ANIM_ET_DROPWEAPON      = 0x6,
	ANIM_ET_RAISEWEAPON     = 0x7,
	ANIM_ET_CLIMBMOUNT      = 0x8,
	ANIM_ET_CLIMBDISMOUNT   = 0x9,
	ANIM_ET_RELOAD          = 0xA,
	ANIM_ET_CROUCH_TO_PRONE = 0xB,
	ANIM_ET_PRONE_TO_CROUCH = 0xC,
	ANIM_ET_MELEEATTACK     = 0xD,
	NUM_ANIM_EVENTTYPES     = 0xE,
} scriptAnimEventTypes_t;

/* animBodyPartsStr, 0x2006CD28 */
typedef enum animBodyPart_t
{
	ANIM_BP_UNUSED     = 0x0,
	ANIM_BP_LEGS       = 0x1,
	ANIM_BP_TORSO      = 0x2,
	ANIM_BP_BOTH       = 0x3,
	NUM_ANIM_BODYPARTS = 0x4,
} animBodyPart_t;

/* animConditionsStr, 0x2006CE18 -- CoD1 cut RTCW's list to these nine. */
typedef enum scriptAnimConditions_t
{
	ANIM_COND_WEAPONS         = 0x0,
	ANIM_COND_WEAPONCLASS     = 0x1,
	ANIM_COND_MOUNTED         = 0x2,
	ANIM_COND_MOVETYPE        = 0x3,
	ANIM_COND_UNDERHAND       = 0x4,
	ANIM_COND_CROUCHING       = 0x5,
	ANIM_COND_FIRING          = 0x6,
	ANIM_COND_WEAPON_POSITION = 0x7,
	ANIM_COND_STRAFING        = 0x8,
	NUM_ANIM_CONDITIONS       = 0x9,
} scriptAnimConditions_t;

/* animConditionsTable, 0x2006CE68: {int type; animStringItem_t *values;} */
typedef enum animScriptConditionTypes_t
{
	ANIM_CONDTYPE_BITFLAGS = 0x0,
	ANIM_CONDTYPE_VALUE    = 0x1,
	NUM_ANIM_CONDTYPES     = 0x2,
} animScriptConditionTypes_t;

/* animMountedStr, 0x2006CD80 */
typedef enum animScriptMountedType_t
{
	MOUNTED_UNUSED = 0x0,
	MOUNTED_MG42   = 0x1,
} animScriptMountedType_t;

/* animWeaponPositionStr, 0x2006CDE0 */
typedef enum animScriptWeaponPosition_t
{
	WEAPON_POSITION_HIP = 0x0,
	WEAPON_POSITION_ADS = 0x1,
} animScriptWeaponPosition_t;

/* animStrafingStr, 0x2006CDF8 */
typedef enum animScriptStrafing_t
{
	STRAFING_NOT   = 0x0,
	STRAFING_LEFT  = 0x1,
	STRAFING_RIGHT = 0x2,
} animScriptStrafing_t;

typedef struct animStringItem_t
{
	const char *string;
	int hash;
} animStringItem_t;
COD1_ASSERT_SIZE( animStringItem_t, 8 );

/*
 * animation_t.  92 bytes; BG_FinalizePlayerAnims fills every named member and
 * BG_GetAnimString returns &animations[i] as the name.
 */
/*
 * A script animation reference, passed and returned by value in a single
 * dword.  The XAnim traps unpack it into two syscall arguments (`shr 16` for
 * anims, `movzx word` for index -- 0x20039090 onwards), and trap_XAnimGetChildAt
 * rewrites only the low half before handing the whole dword back (0x20039450).
 */
typedef struct scr_anim_s {
	unsigned short index;
	unsigned short anims;
} scr_anim_t;
COD1_ASSERT_SIZE( scr_anim_t, 4 );

typedef struct animation_t
{
	char name[64];
	int blendTime;                  /* +0x40  -1 when the script gave none */
	int moveSpeed;                  /* +0x44  1000 * distance / duration */
	int duration;                   /* +0x48  clamped to >= 500 */
	int hash;
	int flags;
	int movetype;                   /* +0x54  scriptAnimMoveTypes_t bit set, BG_ParseCommands 0x20001E90 */
	int noteType;                   /* +0x58  BG_SetupAnimNoteTypes 0x200014C1 */
} animation_t;
COD1_ASSERT_SIZE( animation_t, 92 );

typedef struct animScriptCondition_t
{
	int index;
	int value[ANIM_MASK_WORDS];
} animScriptCondition_t;
COD1_ASSERT_SIZE( animScriptCondition_t, 36 );

/* soundIndex is a full int here; RTCW's is a short. */
typedef struct animScriptCommand_t
{
	short bodyPart[2];
	short animIndex[2];
	short animDuration[2];
	int soundIndex;
} animScriptCommand_t;
COD1_ASSERT_SIZE( animScriptCommand_t, 16 );

typedef struct animScriptItem_t
{
	int numConditions;
	animScriptCondition_t conditions[NUM_ANIM_CONDITIONS];
	int numCommands;
	animScriptCommand_t commands[MAX_ANIMSCRIPT_ANIMCOMMANDS];
} animScriptItem_t;
COD1_ASSERT_SIZE( animScriptItem_t, 244 + 24 * NUM_ANIM_CONDITIONS );

typedef struct animScript_t
{
	int numItems;
	animScriptItem_t *items[MAX_ANIMSCRIPT_ITEMS];
} animScript_t;
COD1_ASSERT_SIZE( animScript_t, 516 );

/*
 * animScriptData_t.  CoD1 folded RTCW's animModelInfo_t into this and dropped
 * the per-model array -- multiplayer has one player model.  Offsets:
 * animations 0, numAnimations 47104, scriptAnims 47108,
 * scriptCannedAnims 84260, scriptStateChange 121412, scriptEvents 129668
 * (BG_AnimScriptEvent, 0x200031F0), scriptItems 136892, numScriptItems 636604,
 * soundIndex 636624 (BG_ParseCommands), playSound 636628 (BG_ExecuteCommand).
 *
 * No size assertion: nothing in this DLL reads past playSound.  RTCW's
 * animScriptData_t ends there.
 */
typedef struct animScriptData_t
{
	animation_t animations[MAX_MODEL_ANIMATIONS];
	int numAnimations;

	animScript_t scriptAnims[MAX_AISTATES][NUM_ANIM_MOVETYPES];
	animScript_t scriptCannedAnims[MAX_AISTATES][NUM_ANIM_MOVETYPES];
	animScript_t scriptStateChange[MAX_AISTATES][MAX_AISTATES];
	animScript_t scriptEvents[NUM_ANIM_EVENTTYPES];

	animScriptItem_t scriptItems[MAX_ANIMSCRIPT_ITEMS_PER_MODEL];
	int numScriptItems;

	int animTreeIndex;              /* +0x9B6C0  handed to Scr_GetAnimsIndex by BG_FinalizePlayerAnims */
	scr_anim_t torsoAnim;           /* +0x9B6C4  BG_FindAnimTrees 0x20004EC0 */
	scr_anim_t legsAnim;            /* +0x9B6C8 */
	scr_anim_t turningAnim;         /* +0x9B6CC */
	/* Returns the alias record's first int, which both engines hand back as-is
	   (G_FIND_SOUND_ALIAS / cgame trap 191, both `*Com_FindSoundAlias(name)`)
	   and which both playSound implementations then treat as a name string --
	   so this is very likely `const char *(*)( const char * )`.  Left as int
	   here because correcting it cascades into trap_FindSoundAlias,
	   trap_Com_SoundAliasString and animScriptCommand_t.soundIndex. */
	int ( *soundIndex )( const char *name );
	/* Two arguments, clientNum FIRST, in both builds: bg_animation.c's single
	   call site is `push soundIndex; push clientNum; call [+0x9B6D4]; add
	   esp,8` (game 0x20002FF0, cgame 0x30003000).  ps->origin is never
	   passed; RTCW's three-argument shape does not apply.  The two stored
	   implementations, G_AnimScriptSound 0x2003C5E0 and
	   CG_PlayEntitySoundAliasByName 0x30021BC0, are both ( int, const char * ). */
	void ( *playSound )( int clientNum, const char *name );
} animScriptData_t;
COD1_ASSERT_SIZE( animScriptData_t, 636632 + 24 * NUM_ANIM_CONDITIONS * MAX_ANIMSCRIPT_ITEMS_PER_MODEL );

/*
 * bg_clientinfo's capacity: BG_CreateClientAnimTrees walks the array from the
 * first entry's animTree (0x2013D7AC) to 0x2014E9AC in 0x448 steps
 * (0x20004F3B), which is exactly 64 entries.
 */
#ifndef MAX_CLIENTS
#define MAX_CLIENTS             64
#endif

/* attachModelNames / attachTagNames are six slots each: BG_UpdatePlayerDObj
 * (0x20004D9C) loops six times with a 0x40 stride, and the tag table sits
 * 0x180 bytes past the model table (0x20004DBD). */
#ifndef MAX_ATTACHED_MODELS
#define MAX_ATTACHED_MODELS     6
#endif

/*
 * lerpFrame_t -- one animation slot.  48 bytes; clientInfo_t carries two of
 * them back to back (legs at +0x37C, torso at +0x3AC) and BG_PlayerAnimation
 * hands both to the same BG_RunLerpFrameRate / BG_PlayerAnimation_VerifyAnim
 * (0x20004C9D, 0x20004CA9).
 *
 * This is RTCW's lerpFrame_t with the frame-interpolation head removed: CoD
 * plays XAnims, so oldFrame/frame/backlerp and oldAnimationNumber are gone and
 * yawAngle moved to the front.  pitchAngle/pitching are only ever used through
 * the torso slot (BG_PlayerAngles 0x20004319); nothing in this DLL reads the
 * legs slot's copy.
 */
typedef struct lerpFrame_t
{
	float yawAngle;                 /* +0x00  BG_SwingAngles target, 0x200041E0 */
	qboolean yawing;                /* +0x04  0x20004102 */
	float pitchAngle;               /* +0x08  0x2000431F */
	qboolean pitching;              /* +0x0C  0x20004155 */
	int animationNumber;            /* +0x10  includes ANIM_TOGGLEBIT, 0x200036E8 */
	animation_t *animation;         /* +0x14  0x20003739 */
	int animationTime;              /* +0x18  blend time in ms, 0x2000373F */
	vec3_t oldFramePos;             /* +0x1C  0x20003CB7 */
	float animSpeedScale;           /* +0x28  0x20003C9D */
	int oldFrameSnapshotTime;       /* +0x2C  0x20003CB1 */
} lerpFrame_t;
COD1_ASSERT_SIZE( lerpFrame_t, 48 );

/*
 * clientInfo_t -- the per-client animation and DObj record shared by the game
 * module and cgame.  1096 bytes: ClientConnect memsets 0x448 of an entry
 * (0x20019E5E) and every index site multiplies the client number by 0x448.
 *
 * The array is really a member of one larger object: G_InitGame's single
 * `rep stosd` (0x20025EB6) clears 0x200A1C80 .. 0x2014E56C in one go -- the
 * animScriptData_t, then the four resolved script animations, then these 64
 * entries -- so the retail spelling is `bgs.clientinfo[]`.  There is no
 * bgs_t here, so the array is declared on its own under an inferred name.
 *
 * ClientConnect saves and restores animTree across its memset (0x20019E51 /
 * 0x20019E60); everything else is cleared on connect.
 */
typedef struct clientInfo_t
{
	qboolean infoValid;                             /* +0x000  gate in BG_UpdatePlayerDObj 0x20004D2A */
	byte unknown_0x004[4];                          /* +0x004  ClientConnect writes 1 (0x20019E6D) and
	                                                           nothing here reads it; UO calls this slot
	                                                           `active` in cgame / `pmType` in game */
	int clientNum;                                  /* +0x008  0x20019DB8; the animation phase seed in
	                                                           BG_SetNewAnimation 0x200038A8 */
	char name[32];                                  /* +0x00C  strncpy 31 + terminate at +43, 0x20019DC0 */
	int team;                                       /* +0x02C  = sess->team, 0x20019DE1 */
	int score;                                      /* +0x030  the game DLL never touches these three;
	                                                           cgame does (CG_ParseScores 0x3002BB2B
	                                                           writes score) */
	int location;                                   /* +0x034  CG_DrawPlayerLocation 0x300260DF */
	int health;                                     /* +0x038  CG_DrawSelectedPlayerHealth 0x30025DD0 */
	char modelName[64];                             /* +0x03C  ClientEndFrame 0x20018EE4 */
	char attachModelNames[MAX_ATTACHED_MODELS][64]; /* +0x07C  0x20018F12 */
	char attachTagNames[MAX_ATTACHED_MODELS][64];   /* +0x1FC  0x20018FED */
	lerpFrame_t legs;                               /* +0x37C  BG_PlayerAnimation 0x20004C9D */
	lerpFrame_t torso;                              /* +0x3AC  BG_PlayerAnimation 0x20004CA9 */
	float leanAmount;                               /* +0x3DC  = es +0x6C, ClientEndFrame 0x20018E48;
	                                                           name inferred from UO */
	float leanFraction;                             /* +0x3E0  = es->leanf, 0x20018E54 */
	float viewPitch;                                /* +0x3E4  = ps->viewangles[0], 0x20018E60 */
	float viewYaw;                                  /* +0x3E8  = ps->viewangles[1], 0x20018E6C */
	float viewRoll;                                 /* +0x3EC  = ps->viewangles[2], 0x20018E78 */
	int gunHandLeft;                                /* +0x3F0  cleared with the line below whenever the
	                                                           torso animation changes (0x20003933);
	                                                           never read here.  Name inferred from UO. */
	int dobjNeedsUpdate;                            /* +0x3F4  raised 0x2000393D, tested and cleared by
	                                                           BG_UpdatePlayerDObj 0x20004D51/0x20004E0A */
	int conditions[NUM_ANIM_CONDITIONS][ANIM_MASK_WORDS];         /* +0x3F8  BG_UpdateConditionValue 0x200032E9 */
	void *animTree;                                 /* +0x440  survives ClientConnect's memset; filled by
	                                                           GScr_LoadScripts 0x20004F39 */
	int dobjWeapon;                                 /* +0x444  the es->weapon the DObj was built for,
	                                                           BG_UpdatePlayerDObj 0x20004D49/0x20004E04 */
} clientInfo_t;
COD1_ASSERT_SIZE( clientInfo_t, 1096 + 24 * NUM_ANIM_CONDITIONS );

extern clientInfo_t bg_clientinfo[MAX_CLIENTS];     /* 0x2013D36C */

/*
 * One entry of the model list trap_DObjCreate takes: 12 bytes, and
 * G_DObjUpdate (0x2003B1E0) and BG_UpdatePlayerDObj both build an array of
 * MAX_ATTACHED_MODELS + 1 of them on the stack.  UO grew the record to 16
 * bytes with an ignoreCollision dword; 1.1 has neither the field nor the
 * attach flag that feeds it.
 */
typedef struct DObjModel_s {
	void            *model;                 /* +0x00 */
	const char      *tagName;               /* +0x04 NULL for the base model */
	short boneName;                         /* +0x08 negated model index: the engine
	                                                 reads a negative part name as
	                                                 "root bone of model n" */
	short unused_0x0A;
} DObjModel;

/*
 * The two angle-state blocks ClientThink_real (0x20017AA0) builds on its stack
 * out of the gclient_t tail and hands to bg_weapon.c's calculators.
 * pmWeaponAngleState_t is 80 bytes; baseAngles is a vec3 here (UO's is a vec2
 * and its record is 0x4C).
 */
typedef struct pmWeaponAngleState_t
{
	playerState_t *ps;              /* +0x00 */
	float speed;                    /* +0x04 */
	float frametime;                /* +0x08 */
	vec3_t moveOffset;              /* +0x0C */
	float idleScale;                /* +0x18 */
	int time;                       /* +0x1C */
	int viewKickStartTime;          /* +0x20 */
	float viewKickPitch;            /* +0x24 */
	float viewKickYaw;              /* +0x28 */
	float recoilPitch;              /* +0x2C */
	float recoilYaw;                /* +0x30 */
	float recoilRoll;               /* +0x34 */
	float recoilPitchSpeed;         /* +0x38 */
	float recoilYawSpeed;           /* +0x3C */
	int recoilState;                /* +0x40 */
	vec3_t baseAngles;              /* +0x44 */
} pmWeaponAngleState_t;

typedef struct bgViewAngleState_t
{
	playerState_t *ps;              /* +0x00 */
	int viewKickStartTime;          /* +0x04 */
	int time;                       /* +0x08 */
	float viewKickPitch;            /* +0x0C */
	float viewKickRoll;             /* +0x10 */
	float speed;                    /* +0x14 */
} bgViewAngleState_t;

//
// bg_animation.c
//
/* These two are the file's genuine client/server split -- the game build
   resolves the DObj from the entity, the cgame build is handed it already
   resolved (CG_TransitionSnapshot -> Com_GetClientDObj, trap 162).  The
   bodies live behind the same CGAMEDLL conditional in bg_animation.c.
     BG_Player_DoControllers  game 0x200046B0  cgame 0x30004710
     BG_UpdatePlayerDObj      game 0x20004CF0  cgame 0x30004EB0 */
/* entityState_t is g_public.h's in the game build and cg_public.h's in cgame,
   and this header includes neither -- same reason trajectory_t is a bare tag
   below.  Both spell it `struct entityState_s`, so the tag is enough. */
struct entityState_s;
#ifdef CGAMEDLL
void        BG_Player_DoControllers( clientInfo_t *ci, int obj,
									 struct entityState_s *es, int *partBits );
void        BG_UpdatePlayerDObj( int obj, struct entityState_s *es, clientInfo_t *ci );
void        BG_PlayerAnimation( clientInfo_t *ci, int obj,
                                struct entityState_s *es );
#else
struct gentity_s;
void        BG_Player_DoControllers( clientInfo_t *ci, struct gentity_s *ent,
									 struct entityState_s *es, int *partBits );
void        BG_UpdatePlayerDObj( struct gentity_s *ent, struct entityState_s *es,
								 clientInfo_t *ci );
void        BG_PlayerAnimation( clientInfo_t *ci, struct gentity_s *ent,
                                struct entityState_s *es );
#endif

/* The runtime animation list is animScriptAnim_t, which is private to
   bg_animation.c: neither caller outside that file can name it, so the slot is
   void * on both sides.  GScr_LoadScripts (0x2002F0D0) and CGScr_LoadAnimTrees
   (0x300133E0) hand it a 512-entry stack array with the count by reference;
   BG_FinalizePlayerAnims' re-parse (0x2000174B) passes NULL/NULL. */
void        BG_AnimParseAnimScript( animScriptData_t *scriptData, void *anims, int *numAnims );
int         BG_AnimScriptAnimation( playerState_t *ps, aistateEnum_t state,
                                    scriptAnimMoveTypes_t movetype, qboolean isContinue );
int         BG_AnimScriptEvent( playerState_t *ps, scriptAnimEventTypes_t event,
                                qboolean isContinue, qboolean force );

//
// bg_misc.c
//
/* trajectory_t belongs to g_public.h, which a bg unit does not include; the
   evaluators only ever take a pointer to one. */
struct trajectory_t;
void        BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm,
                                                 playerState_t *ps );
void        BG_EvaluateTrajectory( const struct trajectory_t *tr, int atTime, vec3_t result );
void        BG_EvaluateTrajectoryDelta( const struct trajectory_t *tr, int atTime, vec3_t result );
/* Both are __usercall; the parameter order is RTCW's (bg_misc.c:3884 and
   :3991, bg_public.h:1401), not the register order. */
void        BG_PlayerStateToEntityState( playerState_t *ps, struct entityState_s *s,
                                         qboolean snap );
void        BG_PlayerStateToEntityStateExtrapolate( playerState_t *ps, struct entityState_s *s,
                                                    int time, qboolean snap );

//
// bg_weapon.c
//
extern int bg_ammoTypeMax[MAX_WEAPONS];             /* 0x20088DD0 */
extern int bg_ammoClipSizes[MAX_WEAPONS];           /* 0x200891E0 */
extern const char *bg_weaponSlotNames[WEAPSLOT_NUM];        /* 0x2006AD7C */

qboolean    BG_GivePlayerWeapon( playerState_t *ps, int weapon );
int         BG_TakePlayerWeapon( playerState_t *ps, int weapon );
int         BG_IsPlayerWeaponInSlot( const playerState_t *ps, int weapon, qboolean includeAltWeapons );
int         BG_GetEmptySlotForWeapon( const playerState_t *ps, int weapon );
int         BG_GetStackSlotForWeapon( const playerState_t *ps, int weapon, int preferredSlot );
int         BG_GetMaxPickupableAmmo( const playerState_t *ps, int weapon );
float       BG_GetMinSpreadForWeapon( int time, const playerState_t *ps, int weaponIndex );

#endif /* __BG_PUBLIC_H__ */
