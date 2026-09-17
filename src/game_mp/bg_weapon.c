/*
 * bg_weapon.c -- the weapon definition unit shared by the multiplayer game DLL
 * and cgame.
 *
 * Call of Duty 1.1 (game_mp_x86.dll, imagebase 0x20000000), 0x2000D980 ..
 * 0x20013270.  RTCW has no ancestor for this file: CoD's weapon system is its
 * own.  Where a name is United Offensive's rather than a recovered symbol it
 * is called out in the comment above it.
 *
 * The unit owns four things:
 *   - the weapon-file parser (bg_weaponInfoFields and the enum name tables it
 *     dispatches through) and the post-parse index/alt/hint fixups;
 *   - bg_itemlist, the 70-row pickup table BG_FillInWeaponItems fills in from
 *     the parsed weapon files;
 *   - the weapon half of Pmove: PM_Weapon and everything it drives;
 *   - the first-person weapon/view angle solver (BG_CalculateWeaponAngles,
 *     BG_CalculateViewAngles, BG_CalculateWeaponPosition_Sway).
 *
 * @fidelity: likely
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Everything in the block below is public in the retail tree -- it lives in
   bg_public.h / q_shared.h and is shared with cgame, g_active_mp.c and
   g_items_mp.c.  None of it is a file-local invention. */

#include "bg_local.h"

/* q_shared.c's ParseConfigStringToStruct (0x2003F070) walks this. */
typedef struct parseField_t
{
	const char *name;
	int ofs;
	int type;
} parseField_t;

/*
 * Field discriminants.  0..7 are handled by ParseConfigStringToStruct itself,
 * 8..13 are handed to BG_ParseWeaponInfoSpecificFieldType; the constant names
 * are conventional, only the numbers are recovered.
 */
typedef enum weaponFieldType_t
{
	WFT_STRING         = 0,
	WFT_STRING1024     = 1,
	WFT_STRING64       = 2,
	WFT_STRING256      = 3,
	WFT_INT            = 4,
	WFT_QBOOLEAN       = 5,
	WFT_FLOAT          = 6,
	WFT_SECONDS        = 7,
	WFT_WEAPONTYPE     = 8,
	WFT_WEAPONCLASS    = 9,
	WFT_OVERLAYRETICLE = 10,
	WFT_WEAPONSLOT     = 11,
	WFT_WEAPONSTANCE   = 12,
	WFT_PROJEXPLOSION  = 13,
	WFT_NUM            = 14
} weaponFieldType_t;

/*
 * playerState_t::weapAnim.  The spellings are the retail ones -- they are the
 * strings PM_Weapon_PrintWeaponAnim (0x20011D60) prints.  1 is not printed and
 * has no recovered name.
 */
typedef enum weaponAnimNumber_t
{
	WEAP_IDLE                = 0,
	WEAP_ATTACK              = 2,
	WEAP_ATTACK_LASTSHOT     = 3,
	WEAP_RECHAMBER           = 4,
	WEAP_ADS_ATTACK          = 5,
	WEAP_ADS_ATTACK_LASTSHOT = 6,
	WEAP_ADS_RECHAMBER       = 7,
	WEAP_MELEE_ATTACK        = 8,
	WEAP_DROP                = 9,
	WEAP_RAISE               = 10,
	WEAP_RELOAD              = 11,
	WEAP_RELOAD_EMPTY        = 12,
	WEAP_RELOAD_START        = 13,
	WEAP_RELOAD_END          = 14,
	WEAP_ALTSWITCHFROM       = 15,
	WEAP_ALTSWITCHTO         = 16
} weaponAnimNumber_t;

#define ANIM_TOGGLEBIT 0x200

/*
 * entity_event_t values this unit raises.  The engine's full 202-entry name
 * table is at 0x2006C8B8; only the ones used here are spelled.
 */
enum
{
	EV_NOAMMO               = 149,
	EV_RELOAD               = 151,
	EV_RELOAD_FROM_EMPTY    = 152,
	EV_RELOAD_START         = 153,
	EV_RELOAD_END           = 154,
	EV_RAISE_WEAPON         = 155,
	EV_PUTAWAY_WEAPON       = 156,
	EV_WEAPON_ALT           = 157,
	EV_PULLBACK_WEAPON      = 158,
	EV_FIRE_WEAPON          = 159,
	EV_FIRE_WEAPON_LASTSHOT = 161,
	EV_RECHAMBER_WEAPON     = 162,
	EV_EJECT_BRASS          = 163,
	EV_MELEE_SWIPE          = 164,
	EV_FIRE_MELEE           = 165
};

/* The pm_flags names this unit reads or writes live in bg_local.h.  0x0800 is
   PMF_RESPAWNED, not PMF_FOLLOW: PM_Weapon's opening guard (0x200119CF) is
   RTCW's "don't allow attack until all buttons are up", and PmoveSingle
   0x2000C34C clears the same bit.  PMF_FOLLOW is 0x00010000. */

/* playerState_t::eFlags bits this unit reads. */
#define EF_CROUCHING                0x00000020
#define EF_PRONE                    0x00000040
#define EF_MOUNTED                  0x0000C000  /* the two mg42 bits */

#define ENTITYNUM_NONE              1023

/* usercmd_t::buttons / ::wbuttons bits this unit reads. */
#define BUTTON_ATTACK               0x01
#define BUTTON_ADS                  0x10
#define BUTTON_MELEE                0x20
#define WBUTTON_RELOAD              0x08

/* configstring index BG_SetupWeaponInfo publishes the weapon-file list into. */
#define CS_WEAPONFILES              7

/* g_main_mp.c cvar table */
extern vmCvar_t bg_debugWeaponState;
extern vmCvar_t bg_debugWeaponAnim;

/* bg_animation.c */
void BG_UpdateConditionValue( int client, int condition, int value, qboolean checkConversion );

/* g_main_mp.c */
void Com_Printf( const char *fmt, ... );
void Com_DPrintf( const char *fmt, ... );
void Com_Error( int level, const char *fmt, ... );

/*
 * Three things the CLIENT build of this file does not have, each compiled out
 * rather than renamed:
 *
 *   RegisterItem            BG_GivePlayerWeapon is 258 bytes in cgame
 *                           (0x3000F2D0) to the game's 306, and makes NO calls
 *                           at all; both RegisterItem sites are simply gone.
 *   G_GetHintStringIndex    BG_SetupUseHintStrings does not exist in the cgame
 *                           image and BG_SetupWeaponInfo does not call it.  Its
 *                           Com_Error string is absent from the image too.  The
 *                           26-byte function at 0x3000EEF0 is
 *                           compare_weaponfile_names with Q_stricmp inlined
 *                           (the s1&&s2 guard and the Q_stricmpn 99999 tail
 *                           call), not BG_SetupUseHintStrings.
 *   trap_SetConfigstring    the client does not publish configstrings; it
 *                           READS CS_WEAPONFILES back.  See BG_SetupWeaponInfo.
 */
#ifndef CGAMEDLL
/* g_items_mp.c / g_scr_main_mp.c */
void RegisterItem( int item, qboolean sendUpdate );
qboolean G_GetHintStringIndex( int *index, const char *string );
#endif

#ifdef CGAMEDLL
/* cg_main.c 0x30021B00; the client half of BG_SetupWeaponInfo reads the list
   the server published.  LTCG inlined it at 0x3000EFC6, constant index and
   all, so the retail body indexes cgs.gameState directly. */
const char *CG_ConfigString( int index );
#endif

/* universal/q_shared.c, universal/com_math.c */
int Q_stricmp( const char *s1, const char *s2 );
int Q_stricmpn( const char *s1, const char *s2, int n );
char *Q_strlwr( char *s1 );
qboolean ParseConfigStringToStruct( const parseField_t *fields, void *base, int numFields,
									const char *info, int numGenericTypes,
									qboolean ( *parseSpecific )( void *base, const char *value, int fieldType ),
									void ( *setString )( void *dest, const char *value ) );
float AngleSubtract( float a1, float a2 );
float AngleNormalize180( float angle );

/* game_mp/g_syscalls_mp.c */
void *trap_Hunk_AllocLowInternal( int size );
void *trap_Hunk_AllocLowAlignInternal( int size, int align );
int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void trap_FS_Read( void *buffer, int len, fileHandle_t f );
void trap_FS_FCloseFile( fileHandle_t f );
int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
#ifndef CGAMEDLL
void trap_SetConfigstring( int num, const char *string );
#endif
void *trap_GetWeaponInfoMemory( int size, int *pPrevOwner );

/* This unit's own entry points that are used above their definition. */
int BG_IsPlayerWeaponInSlot( const playerState_t *ps, int weapon, qboolean includeAltWeapons );
void PM_StartWeaponAnim( int anim );
void PM_ContinueWeaponAnim( int anim );
void PM_Weapon_PrintWeaponState( void );
void PM_Weapon_PrintWeaponAnim( void );

/*
=============================================================================

	WEAPON FILE PARSING

=============================================================================
*/

/* Printed by the two weapon debug tracers; cgame's copy of this file carries
 * "C".  Recovered spelling, inferred (invented) name. */
static const char *bg_szDebugModule = "G";

static const char *bg_szWeaponsFolder = "weapons/mp";

static const char *bg_weaponTypeNames[WEAPTYPE_NUM] =
{
	"bullet",
	"grenade",
	"projectile"
};

static const char *bg_weaponOverlayReticleNames[WEAPOVERLAYRETICLE_NUM] =
{
	"none",
	"crosshair",
	"FG42",
	"Springfield",
	"Gewehr43"
};

const char *bg_weaponSlotNames[WEAPSLOT_NUM] =
{
	"none",
	"primary",
	"primaryb",
	"pistol",
	"grenade",
	"smokegrenade"
};

static const char *bg_weaponStanceNames[WEAPSTANCE_NUM] =
{
	"stand",
	"duck",
	"prone"
};

static const char *bg_weaponClassNames[WEAPCLASS_NUM] =
{
	"rifle",
	"mg",
	"smg",
	"pistol",
	"grenade",
	"rocketlauncher",
	"turret",
	"non-player"
};

static const char *bg_weaponGrenadeTypeNames[WEAPPROJEXP_NUM] =
{
	"grenade",
	"rocket",
	"molotov",
	"none"
};

#define WFOFS( x ) ( (int)&( ( (weaponInfo_t *)0 )->x ) )
#define WEAPONINFO_NUM_FIELDS ( (int)( sizeof( bg_weaponInfoFields ) / sizeof( bg_weaponInfoFields[0] ) ) )

/*
 * The weapon-file parse table, bg_weaponInfoFields at 0x2006ADD0: 248 triples
 * in this order.  ParseConfigStringToStruct is given the count, not a NULL
 * sentinel.
 */
static const parseField_t bg_weaponInfoFields[] =
{
	{ "displayName",            WFOFS( displayName ),            WFT_STRING },
	{ "AIOverlayDescription",   WFOFS( AIOverlayDescription ),   WFT_STRING },
	{ "modeName",               WFOFS( modeName ),               WFT_STRING },
	{ "gunModel",               WFOFS( gunModel ),               WFT_STRING },
	{ "handModel",              WFOFS( handModel ),              WFT_STRING },
	{ "idleAnim",               WFOFS( idleAnim ),               WFT_STRING },
	{ "emptyIdleAnim",          WFOFS( emptyIdleAnim ),          WFT_STRING },
	{ "fireAnim",               WFOFS( fireAnim ),               WFT_STRING },
	{ "holdFireAnim",           WFOFS( holdFireAnim ),           WFT_STRING },
	{ "lastShotAnim",           WFOFS( lastShotAnim ),           WFT_STRING },
	{ "rechamberAnim",          WFOFS( rechamberAnim ),          WFT_STRING },
	{ "meleeAnim",              WFOFS( meleeAnim ),              WFT_STRING },
	{ "reloadAnim",             WFOFS( reloadAnim ),             WFT_STRING },
	{ "reloadEmptyAnim",        WFOFS( reloadEmptyAnim ),        WFT_STRING },
	{ "reloadStartAnim",        WFOFS( reloadStartAnim ),        WFT_STRING },
	{ "reloadEndAnim",          WFOFS( reloadEndAnim ),          WFT_STRING },
	{ "raiseAnim",              WFOFS( raiseAnim ),              WFT_STRING },
	{ "dropAnim",               WFOFS( dropAnim ),               WFT_STRING },
	{ "altRaiseAnim",           WFOFS( altRaiseAnim ),           WFT_STRING },
	{ "altDropAnim",            WFOFS( altDropAnim ),            WFT_STRING },
	{ "adsFireAnim",            WFOFS( adsFireAnim ),            WFT_STRING },
	{ "adsLastShotAnim",        WFOFS( adsLastShotAnim ),        WFT_STRING },
	{ "adsRechamberAnim",       WFOFS( adsRechamberAnim ),       WFT_STRING },
	{ "adsUpAnim",              WFOFS( adsUpAnim ),              WFT_STRING },
	{ "adsDownAnim",            WFOFS( adsDownAnim ),            WFT_STRING },
	{ "script",                 WFOFS( script ),                 WFT_STRING },
	{ "weaponType",             WFOFS( weaponType ),             WFT_WEAPONTYPE },
	{ "weaponClass",            WFOFS( weaponClass ),            WFT_WEAPONCLASS },
	{ "weaponSlot",             WFOFS( weaponSlot ),             WFT_WEAPONSLOT },
	{ "slotStackable",          WFOFS( slotStackable ),          WFT_QBOOLEAN },
	{ "viewFlashEffect",        WFOFS( viewFlashEffect ),        WFT_STRING },
	{ "worldFlashEffect",       WFOFS( worldFlashEffect ),       WFT_STRING },
	{ "pickupSound",            WFOFS( pickupSound ),            WFT_STRING },
	{ "ammoPickupSound",        WFOFS( ammoPickupSound ),        WFT_STRING },
	{ "projectileSound",        WFOFS( projectileSound ),        WFT_STRING },
	{ "pullbackSound",          WFOFS( pullbackSound ),          WFT_STRING },
	{ "fireSound",              WFOFS( fireSound ),              WFT_STRING },
	{ "loopFireSound",          WFOFS( loopFireSound ),          WFT_STRING },
	{ "stopFireSound",          WFOFS( stopFireSound ),          WFT_STRING },
	{ "fireEchoSound",          WFOFS( fireEchoSound ),          WFT_STRING },
	{ "lastShotSound",          WFOFS( lastShotSound ),          WFT_STRING },
	{ "rechamberSound",         WFOFS( rechamberSound ),         WFT_STRING },
	{ "reloadSound",            WFOFS( reloadSound ),            WFT_STRING },
	{ "reloadEmptySound",       WFOFS( reloadEmptySound ),       WFT_STRING },
	{ "reloadStartSound",       WFOFS( reloadStartSound ),       WFT_STRING },
	{ "reloadEndSound",         WFOFS( reloadEndSound ),         WFT_STRING },
	{ "raiseSound",             WFOFS( raiseSound ),             WFT_STRING },
	{ "altSwitchSound",         WFOFS( altSwitchSound ),         WFT_STRING },
	{ "putawaySound",           WFOFS( putawaySound ),           WFT_STRING },
	{ "noteTrackSoundA",        WFOFS( noteTrackSoundA ),        WFT_STRING },
	{ "noteTrackSoundB",        WFOFS( noteTrackSoundB ),        WFT_STRING },
	{ "noteTrackSoundC",        WFOFS( noteTrackSoundC ),        WFT_STRING },
	{ "noteTrackSoundD",        WFOFS( noteTrackSoundD ),        WFT_STRING },
	{ "shellEjectEffect",       WFOFS( shellEjectEffect ),       WFT_STRING },
	{ "lastShotEjectEffect",    WFOFS( lastShotEjectEffect ),    WFT_STRING },
	{ "reticleCenter",          WFOFS( reticleCenter ),          WFT_STRING },
	{ "reticleSide",            WFOFS( reticleSide ),            WFT_STRING },
	{ "reticleCenterSize",      WFOFS( reticleCenterSize ),      WFT_INT },
	{ "reticleSideSize",        WFOFS( reticleSideSize ),        WFT_INT },
	{ "reticleMinOfs",          WFOFS( reticleMinOfs ),          WFT_INT },
	{ "standMoveF",             WFOFS( standMoveF ),             WFT_FLOAT },
	{ "standMoveR",             WFOFS( standMoveR ),             WFT_FLOAT },
	{ "standMoveU",             WFOFS( standMoveU ),             WFT_FLOAT },
	{ "standRotP",              WFOFS( standRotP ),              WFT_FLOAT },
	{ "standRotY",              WFOFS( standRotY ),              WFT_FLOAT },
	{ "standRotR",              WFOFS( standRotR ),              WFT_FLOAT },
	{ "duckedOfsF",             WFOFS( duckedOfsF ),             WFT_FLOAT },
	{ "duckedOfsR",             WFOFS( duckedOfsR ),             WFT_FLOAT },
	{ "duckedOfsU",             WFOFS( duckedOfsU ),             WFT_FLOAT },
	{ "duckedMoveF",            WFOFS( duckedMoveF ),            WFT_FLOAT },
	{ "duckedMoveR",            WFOFS( duckedMoveR ),            WFT_FLOAT },
	{ "duckedMoveU",            WFOFS( duckedMoveU ),            WFT_FLOAT },
	{ "duckedRotP",             WFOFS( duckedRotP ),             WFT_FLOAT },
	{ "duckedRotY",             WFOFS( duckedRotY ),             WFT_FLOAT },
	{ "duckedRotR",             WFOFS( duckedRotR ),             WFT_FLOAT },
	{ "proneOfsF",              WFOFS( proneOfsF ),              WFT_FLOAT },
	{ "proneOfsR",              WFOFS( proneOfsR ),              WFT_FLOAT },
	{ "proneOfsU",              WFOFS( proneOfsU ),              WFT_FLOAT },
	{ "proneMoveF",             WFOFS( proneMoveF ),             WFT_FLOAT },
	{ "proneMoveR",             WFOFS( proneMoveR ),             WFT_FLOAT },
	{ "proneMoveU",             WFOFS( proneMoveU ),             WFT_FLOAT },
	{ "proneRotP",              WFOFS( proneRotP ),              WFT_FLOAT },
	{ "proneRotY",              WFOFS( proneRotY ),              WFT_FLOAT },
	{ "proneRotR",              WFOFS( proneRotR ),              WFT_FLOAT },
	{ "posMoveRate",            WFOFS( posMoveRate ),            WFT_FLOAT },
	{ "posProneMoveRate",       WFOFS( posProneMoveRate ),       WFT_FLOAT },
	{ "standMoveMinSpeed",      WFOFS( standMoveMinSpeed ),      WFT_FLOAT },
	{ "duckedMoveMinSpeed",     WFOFS( duckedMoveMinSpeed ),     WFT_FLOAT },
	{ "proneMoveMinSpeed",      WFOFS( proneMoveMinSpeed ),      WFT_FLOAT },
	{ "posRotRate",             WFOFS( posRotRate ),             WFT_FLOAT },
	{ "posProneRotRate",        WFOFS( posProneRotRate ),        WFT_FLOAT },
	{ "standRotMinSpeed",       WFOFS( standRotMinSpeed ),       WFT_FLOAT },
	{ "duckedRotMinSpeed",      WFOFS( duckedRotMinSpeed ),      WFT_FLOAT },
	{ "proneRotMinSpeed",       WFOFS( proneRotMinSpeed ),       WFT_FLOAT },
	{ "radiantName",            WFOFS( radiantName ),            WFT_STRING },
	{ "worldModel",             WFOFS( worldModel ),             WFT_STRING },
	{ "hudIcon",                WFOFS( hudIcon ),                WFT_STRING },
	{ "modeIcon",               WFOFS( modeIcon ),               WFT_STRING },
	{ "ammoIcon",               WFOFS( ammoIcon ),               WFT_STRING },
	{ "startAmmo",              WFOFS( startAmmo ),              WFT_INT },
	{ "ammoName",               WFOFS( ammoName ),               WFT_STRING },
	{ "clipName",               WFOFS( clipName ),               WFT_STRING },
	{ "maxAmmo",                WFOFS( maxAmmo ),                WFT_INT },
	{ "clipSize",               WFOFS( clipSize ),               WFT_INT },
	{ "sharedAmmoCapName",      WFOFS( sharedAmmoCapName ),      WFT_STRING },
	{ "sharedAmmoCap",          WFOFS( sharedAmmoCap ),          WFT_INT },
	{ "damage",                 WFOFS( damage ),                 WFT_INT },
	{ "meleeDamage",            WFOFS( meleeDamage ),            WFT_INT },
	{ "fireDelay",              WFOFS( fireDelay ),              WFT_SECONDS },
	{ "meleeDelay",             WFOFS( meleeDelay ),             WFT_SECONDS },
	{ "fireTime",               WFOFS( fireTime ),               WFT_SECONDS },
	{ "rechamberTime",          WFOFS( rechamberTime ),          WFT_SECONDS },
	{ "rechamberBoltTime",      WFOFS( rechamberBoltTime ),      WFT_SECONDS },
	{ "holdFireTime",           WFOFS( holdFireTime ),           WFT_SECONDS },
	{ "meleeTime",              WFOFS( meleeTime ),              WFT_SECONDS },
	{ "reloadTime",             WFOFS( reloadTime ),             WFT_SECONDS },
	{ "reloadEmptyTime",        WFOFS( reloadEmptyTime ),        WFT_SECONDS },
	{ "reloadAddTime",          WFOFS( reloadAddTime ),          WFT_SECONDS },
	{ "reloadStartTime",        WFOFS( reloadStartTime ),        WFT_SECONDS },
	{ "reloadStartAddTime",     WFOFS( reloadStartAddTime ),     WFT_SECONDS },
	{ "reloadEndTime",          WFOFS( reloadEndTime ),          WFT_SECONDS },
	{ "dropTime",               WFOFS( dropTime ),               WFT_SECONDS },
	{ "raiseTime",              WFOFS( raiseTime ),              WFT_SECONDS },
	{ "altDropTime",            WFOFS( altDropTime ),            WFT_SECONDS },
	{ "altRaiseTime",           WFOFS( altRaiseTime ),           WFT_SECONDS },
	{ "fuseTime",               WFOFS( fuseTime ),               WFT_SECONDS },
	{ "moveSpeedScale",         WFOFS( moveSpeedScale ),         WFT_FLOAT },
	{ "idleCrouchFactor",       WFOFS( idleCrouchFactor ),       WFT_FLOAT },
	{ "idleProneFactor",        WFOFS( idleProneFactor ),        WFT_FLOAT },
	{ "gunMaxPitch",            WFOFS( gunMaxPitch ),            WFT_FLOAT },
	{ "gunMaxYaw",              WFOFS( gunMaxYaw ),              WFT_FLOAT },
	{ "swayMaxAngle",           WFOFS( swayMaxAngle ),           WFT_FLOAT },
	{ "swayLerpSpeed",          WFOFS( swayLerpSpeed ),          WFT_FLOAT },
	{ "swayPitchScale",         WFOFS( swayPitchScale ),         WFT_FLOAT },
	{ "swayYawScale",           WFOFS( swayYawScale ),           WFT_FLOAT },
	{ "swayHorizScale",         WFOFS( swayHorizScale ),         WFT_FLOAT },
	{ "swayVertScale",          WFOFS( swayVertScale ),          WFT_FLOAT },
	{ "swayShellShockScale",    WFOFS( swayShellShockScale ),    WFT_FLOAT },
	{ "adsSwayMaxAngle",        WFOFS( adsSwayMaxAngle ),        WFT_FLOAT },
	{ "adsSwayLerpSpeed",       WFOFS( adsSwayLerpSpeed ),       WFT_FLOAT },
	{ "adsSwayPitchScale",      WFOFS( adsSwayPitchScale ),      WFT_FLOAT },
	{ "adsSwayYawScale",        WFOFS( adsSwayYawScale ),        WFT_FLOAT },
	{ "adsSwayHorizScale",      WFOFS( adsSwayHorizScale ),      WFT_FLOAT },
	{ "adsSwayVertScale",       WFOFS( adsSwayVertScale ),       WFT_FLOAT },
	{ "rifleBullet",            WFOFS( rifleBullet ),            WFT_QBOOLEAN },
	{ "twoHanded",              WFOFS( twoHanded ),              WFT_QBOOLEAN },
	{ "semiAuto",               WFOFS( semiAuto ),               WFT_QBOOLEAN },
	{ "boltAction",             WFOFS( boltAction ),             WFT_QBOOLEAN },
	{ "aimDownSight",           WFOFS( aimDownSight ),           WFT_QBOOLEAN },
	{ "clipOnly",               WFOFS( clipOnly ),               WFT_QBOOLEAN },
	{ "cookOffHold",            WFOFS( cookOffHold ),            WFT_QBOOLEAN },
	{ "wideListIcon",           WFOFS( wideListIcon ),           WFT_QBOOLEAN },
	{ "adsFire",                WFOFS( adsFire ),                WFT_QBOOLEAN },
	{ "killIcon",               WFOFS( killIcon ),               WFT_STRING },
	{ "wideKillIcon",           WFOFS( wideKillIcon ),           WFT_QBOOLEAN },
	{ "noPartialReload",        WFOFS( noPartialReload ),        WFT_QBOOLEAN },
	{ "segmentedReload",        WFOFS( segmentedReload ),        WFT_QBOOLEAN },
	{ "reloadAmmoAdd",          WFOFS( reloadAmmoAdd ),          WFT_INT },
	{ "reloadStartAdd",         WFOFS( reloadStartAdd ),         WFT_INT },
	{ "altWeapon",              WFOFS( altWeapon ),              WFT_STRING },
	{ "dropAmmoMin",            WFOFS( dropAmmoMin ),            WFT_INT },
	{ "dropAmmoMax",            WFOFS( dropAmmoMax ),            WFT_INT },
	{ "explosionRadius",        WFOFS( explosionRadius ),        WFT_INT },
	{ "explosionInnerDamage",   WFOFS( explosionInnerDamage ),   WFT_INT },
	{ "explosionOuterDamage",   WFOFS( explosionOuterDamage ),   WFT_INT },
	{ "projectileSpeed",        WFOFS( projectileSpeed ),        WFT_INT },
	{ "projectileSpeedUp",      WFOFS( projectileSpeedUp ),      WFT_INT },
	{ "projectileModel",        WFOFS( projectileModel ),        WFT_STRING },
	{ "projExplosionType",      WFOFS( projExplosionType ),      WFT_PROJEXPLOSION },
	{ "projExplosionEffect",    WFOFS( projExplosionEffect ),    WFT_STRING },
	{ "projExplosionSound",     WFOFS( projExplosionSound ),     WFT_STRING },
	{ "projImpactExplode",      WFOFS( projImpactExplode ),      WFT_QBOOLEAN },
	{ "projTrailEffect",        WFOFS( projTrailEffect ),        WFT_STRING },
	{ "projectileDLight",       WFOFS( projectileDLight ),       WFT_INT },
	{ "projectileRed",          WFOFS( projectileRed ),          WFT_FLOAT },
	{ "projectileGreen",        WFOFS( projectileGreen ),        WFT_FLOAT },
	{ "projectileBlue",         WFOFS( projectileBlue ),         WFT_FLOAT },
	{ "adsTransInTime",         WFOFS( adsTransInTime ),         WFT_SECONDS },
	{ "adsTransOutTime",        WFOFS( adsTransOutTime ),        WFT_SECONDS },
	{ "adsIdleAmount",          WFOFS( adsIdleAmount ),          WFT_FLOAT },
	{ "adsZoomFov",             WFOFS( adsZoomFov ),             WFT_FLOAT },
	{ "adsZoomInFrac",          WFOFS( adsZoomInFrac ),          WFT_FLOAT },
	{ "adsZoomOutFrac",         WFOFS( adsZoomOutFrac ),         WFT_FLOAT },
	{ "adsOverlayShader",       WFOFS( adsOverlayShader ),       WFT_STRING },
	{ "adsOverlayReticle",      WFOFS( adsOverlayReticle ),      WFT_OVERLAYRETICLE },
	{ "adsOverlayWidth",        WFOFS( adsOverlayWidth ),        WFT_FLOAT },
	{ "adsOverlayHeight",       WFOFS( adsOverlayHeight ),       WFT_FLOAT },
	{ "adsBobFactor",           WFOFS( adsBobFactor ),           WFT_FLOAT },
	{ "adsViewBobMult",         WFOFS( adsViewBobMult ),         WFT_FLOAT },
	{ "adsAimPitch",            WFOFS( adsAimPitch ),            WFT_FLOAT },
	{ "adsCrosshairInFrac",     WFOFS( adsCrosshairInFrac ),     WFT_FLOAT },
	{ "adsCrosshairOutFrac",    WFOFS( adsCrosshairOutFrac ),    WFT_FLOAT },
	{ "adsReloadTransTime",     WFOFS( adsReloadTransTime ),     WFT_SECONDS },
	{ "adsTransBlendTime",      WFOFS( adsTransBlendTime ),      WFT_SECONDS },
	{ "adsGunKickPitchMin",     WFOFS( adsGunKickPitchMin ),     WFT_FLOAT },
	{ "adsGunKickPitchMax",     WFOFS( adsGunKickPitchMax ),     WFT_FLOAT },
	{ "adsGunKickYawMin",       WFOFS( adsGunKickYawMin ),       WFT_FLOAT },
	{ "adsGunKickYawMax",       WFOFS( adsGunKickYawMax ),       WFT_FLOAT },
	{ "adsGunKickAccel",        WFOFS( adsGunKickAccel ),        WFT_FLOAT },
	{ "adsGunKickSpeedMax",     WFOFS( adsGunKickSpeedMax ),     WFT_FLOAT },
	{ "adsGunKickSpeedDecay",   WFOFS( adsGunKickSpeedDecay ),   WFT_FLOAT },
	{ "adsGunKickStaticDecay",  WFOFS( adsGunKickStaticDecay ),  WFT_FLOAT },
	{ "adsViewKickPitchMin",    WFOFS( adsViewKickPitchMin ),    WFT_FLOAT },
	{ "adsViewKickPitchMax",    WFOFS( adsViewKickPitchMax ),    WFT_FLOAT },
	{ "adsViewKickYawMin",      WFOFS( adsViewKickYawMin ),      WFT_FLOAT },
	{ "adsViewKickYawMax",      WFOFS( adsViewKickYawMax ),      WFT_FLOAT },
	{ "adsViewKickCenterSpeed", WFOFS( adsViewKickCenterSpeed ), WFT_FLOAT },
	{ "adsSpread",              WFOFS( adsSpread ),              WFT_FLOAT },
	{ "hipSpreadStandMin",      WFOFS( hipSpreadStandMin ),      WFT_FLOAT },
	{ "hipSpreadDuckedMin",     WFOFS( hipSpreadDuckedMin ),     WFT_FLOAT },
	{ "hipSpreadProneMin",      WFOFS( hipSpreadProneMin ),      WFT_FLOAT },
	{ "hipSpreadMax",           WFOFS( hipSpreadMax ),           WFT_FLOAT },
	{ "hipSpreadDecayRate",     WFOFS( hipSpreadDecayRate ),     WFT_FLOAT },
	{ "hipSpreadFireAdd",       WFOFS( hipSpreadFireAdd ),       WFT_FLOAT },
	{ "hipSpreadTurnAdd",       WFOFS( hipSpreadTurnAdd ),       WFT_FLOAT },
	{ "hipSpreadMoveAdd",       WFOFS( hipSpreadMoveAdd ),       WFT_FLOAT },
	{ "hipSpreadDuckedDecay",   WFOFS( hipSpreadDuckedDecay ),   WFT_FLOAT },
	{ "hipSpreadProneDecay",    WFOFS( hipSpreadProneDecay ),    WFT_FLOAT },
	{ "hipReticleSidePos",      WFOFS( hipReticleSidePos ),      WFT_FLOAT },
	{ "hipIdleAmount",          WFOFS( hipIdleAmount ),          WFT_FLOAT },
	{ "hipGunKickPitchMin",     WFOFS( hipGunKickPitchMin ),     WFT_FLOAT },
	{ "hipGunKickPitchMax",     WFOFS( hipGunKickPitchMax ),     WFT_FLOAT },
	{ "hipGunKickYawMin",       WFOFS( hipGunKickYawMin ),       WFT_FLOAT },
	{ "hipGunKickYawMax",       WFOFS( hipGunKickYawMax ),       WFT_FLOAT },
	{ "hipGunKickAccel",        WFOFS( hipGunKickAccel ),        WFT_FLOAT },
	{ "hipGunKickSpeedMax",     WFOFS( hipGunKickSpeedMax ),     WFT_FLOAT },
	{ "hipGunKickSpeedDecay",   WFOFS( hipGunKickSpeedDecay ),   WFT_FLOAT },
	{ "hipGunKickStaticDecay",  WFOFS( hipGunKickStaticDecay ),  WFT_FLOAT },
	{ "hipViewKickPitchMin",    WFOFS( hipViewKickPitchMin ),    WFT_FLOAT },
	{ "hipViewKickPitchMax",    WFOFS( hipViewKickPitchMax ),    WFT_FLOAT },
	{ "hipViewKickYawMin",      WFOFS( hipViewKickYawMin ),      WFT_FLOAT },
	{ "hipViewKickYawMax",      WFOFS( hipViewKickYawMax ),      WFT_FLOAT },
	{ "hipViewKickCenterSpeed", WFOFS( hipViewKickCenterSpeed ), WFT_FLOAT },
	{ "leftArc",                WFOFS( leftArc ),                WFT_FLOAT },
	{ "rightArc",               WFOFS( rightArc ),               WFT_FLOAT },
	{ "topArc",                 WFOFS( topArc ),                 WFT_FLOAT },
	{ "bottomArc",              WFOFS( bottomArc ),              WFT_FLOAT },
	{ "accuracy",               WFOFS( accuracy ),               WFT_FLOAT },
	{ "vertTurnSpeed",          WFOFS( vertTurnSpeed ),          WFT_FLOAT },
	{ "horTurnSpeed",           WFOFS( horTurnSpeed ),           WFT_FLOAT },
	{ "convergenceTime",        WFOFS( convergenceTime ),        WFT_FLOAT },
	{ "maxRange",               WFOFS( maxRange ),               WFT_FLOAT },
	{ "animHorRotateInc",       WFOFS( animHorRotateInc ),       WFT_FLOAT },
	{ "playerPositionDist",     WFOFS( playerPositionDist ),     WFT_FLOAT },
	{ "stance",                 WFOFS( stance ),                 WFT_WEAPONSTANCE },
	{ "useHintString",          WFOFS( useHintString ),          WFT_STRING },
	{ "aiEffectiveRange",       WFOFS( aiEffectiveRange ),       WFT_FLOAT },
	{ "aiMissRange",            WFOFS( aiMissRange ),            WFT_FLOAT }
};

/*
=============================================================================

	ITEMS

	bg_itemlist, 0x2006BB68.  70 rows: [0] is the IT_BAD placeholder, [1..64]
	are the weapon placeholders BG_FillInWeaponItems overwrites from the parsed
	weapon files, [65..69] are the two grenade-ammo and three health items.

=============================================================================
*/

/* bg_misc.c defines bg_itemlist (0x2006BB68): every item ACCESSOR --
   BG_FindItemForWeapon, BG_FindItem, BG_PlayerTouchesItem,
   BG_CanItemBeGrabbed -- sits there at 0x20004F50..0x200050B0, immediately
   after the table, which is RTCW's arrangement; this unit holds only
   BG_FillInWeaponItems (0x2000E270), which WRITES the table.

   TODO: whether the 64 weapon placeholder rows carry "" or NULL for
   pickup_sound / world_model / icon / ammoicon / pickup_name is unresolved;
   bg_misc.c gives them NULL.  bg_misc.c's table carries a 71st all-NULL
   terminator row, so BG_NUM_ITEMS is the literal 70 that BG_FindItem's loop
   uses, not a sizeof. */
extern gitem_t bg_itemlist[];                   /* bg_misc.c, 0x2006BB68 */

#define BG_NUM_ITEMS                70

/*
=============================================================================

	AMMO / CLIP / SHARED-AMMO-CAP REGISTRIES

=============================================================================
*/

static char *bg_ammoClipNames[MAX_WEAPONS];
int bg_ammoTypeMax[MAX_WEAPONS];
static char *bg_sharedAmmoCapNames[MAX_WEAPONS];
static int bg_numAmmoClips;
static int bg_numSharedAmmoCaps;
static int bg_numAmmoTypes;
/* One shared "" every unset string field points at; InitWeaponInfo hands it out
 * so that no parsed field is ever NULL. */
static char *bg_szEmptyString;
static int bg_sharedAmmoCapSizes[MAX_WEAPONS];
static char *bg_ammoTypeNames[MAX_WEAPONS];
int bg_ammoClipSizes[MAX_WEAPONS];

int bg_numWeapons;
weaponInfo_t **bg_weaponInfo;

/*
==============
BG_GetWeaponTypeName

No range check in the retail leaf.
==============
*/
const char *BG_GetWeaponTypeName( weapType_t weaponType ) {
	return bg_weaponTypeNames[weaponType];
}

/*
==============
BG_ParseWeaponInfoSpecificFieldType

ParseConfigStringToStruct's callback for field types >= 8.
==============
*/
qboolean BG_ParseWeaponInfoSpecificFieldType( void *base, const char *value, int fieldType ) {
	weaponInfo_t *weaponInfo;
	int i;

	weaponInfo = (weaponInfo_t *)base;

	switch ( fieldType ) {
	case WFT_WEAPONTYPE:
		for ( i = 0; i < WEAPTYPE_NUM; i++ ) {
			if ( !_stricmp( value, bg_weaponTypeNames[i] ) ) {
				weaponInfo->weaponType = (weapType_t)i;
				break;
			}
		}
		if ( i == WEAPTYPE_NUM ) {
			Com_Error( ERR_DROP, "\x15" "Unknown weapon type \"%s\" in \"%s\"\n", value, weaponInfo->szInternalName );
		}
		return qtrue;

	case WFT_WEAPONCLASS:
		for ( i = 0; i < WEAPCLASS_NUM; i++ ) {
			if ( !_stricmp( value, bg_weaponClassNames[i] ) ) {
				weaponInfo->weaponClass = (weapClass_t)i;
				break;
			}
		}
		if ( i == WEAPCLASS_NUM ) {
			Com_Error( ERR_DROP, "\x15" "Unknown weapon class \"%s\" in \"%s\"\n", value, weaponInfo->szInternalName );
		}
		return qtrue;

	case WFT_OVERLAYRETICLE:
		for ( i = 0; i < WEAPOVERLAYRETICLE_NUM; i++ ) {
			if ( !_stricmp( value, bg_weaponOverlayReticleNames[i] ) ) {
				weaponInfo->adsOverlayReticle = (weapOverlayReticle_t)i;
				break;
			}
		}
		if ( i == WEAPOVERLAYRETICLE_NUM ) {
			Com_Error( ERR_DROP, "\x15" "Unknown weapon overlay reticle \"%s\" in \"%s\"\n", value, weaponInfo->szInternalName );
		}
		return qtrue;

	case WFT_WEAPONSLOT:
		for ( i = 0; i < WEAPSLOT_NUM; i++ ) {
			if ( !_stricmp( value, bg_weaponSlotNames[i] ) ) {
				weaponInfo->weaponSlot = (weapSlot_t)i;
				break;
			}
		}
		if ( i == WEAPSLOT_NUM ) {
			Com_Error( ERR_DROP, "\x15" "Unknown weapon slot \"%s\" in \"%s\"\n", value, weaponInfo->szInternalName );
		}
		return qtrue;

	case WFT_WEAPONSTANCE:
		for ( i = 0; i < WEAPSTANCE_NUM; i++ ) {
			if ( !_stricmp( value, bg_weaponStanceNames[i] ) ) {
				weaponInfo->stance = (weapStance_t)i;
				break;
			}
		}
		if ( i == WEAPSTANCE_NUM ) {
			Com_Error( ERR_DROP, "\x15" "Unknown weapon stance \"%s\" in \"%s\"\n", value, weaponInfo->szInternalName );
		}
		return qtrue;

	case WFT_PROJEXPLOSION:
		for ( i = 0; i < WEAPPROJEXP_NUM; i++ ) {
			if ( !_stricmp( value, bg_weaponGrenadeTypeNames[i] ) ) {
				weaponInfo->projExplosionType = (weapProjExposion_t)i;
				break;
			}
		}
		if ( i == WEAPPROJEXP_NUM ) {
			Com_Error( ERR_DROP, "\x15" "Unknown projectile explosion \"%s\" in \"%s\"\n", value, weaponInfo->szInternalName );
		}
		return qtrue;

	default:
		Com_Error( ERR_DROP, "\x15" "Bad field type %i in %s\n", fieldType, weaponInfo->szInternalName );
		return qfalse;
	}
}

/*
==============
SetConfigString

Nothing calls this in the retail DLL any more -- LTCG inlined every call site
-- but the standalone body survives at 0x2000DBE0.
==============
*/
char *SetConfigString( char **dest, const char *value ) {
	char *out;

	if ( !*value ) {
		*dest = bg_szEmptyString;
		return bg_szEmptyString;
	}
	out = trap_Hunk_AllocLowAlignInternal( strlen( value ) + 1, 1 );
	strcpy( out, value );
	*dest = out;
	return out;
}

/*
==============
SetConfigString2

The same thing as a ParseConfigStringToStruct callback: the destination
arrives as base + field offset, so it is typed void *.
==============
*/
void SetConfigString2( void *dest, const char *value ) {
	char *out;

	if ( !*value ) {
		*(char **)dest = bg_szEmptyString;
		return;
	}
	out = trap_Hunk_AllocLowAlignInternal( strlen( value ) + 1, 1 );
	strcpy( out, value );
	*(char **)dest = out;
}

/*
==============
InitWeaponInfo

Allocate one weaponInfo_t and point every string field at the shared "".
==============
*/
weaponInfo_t *InitWeaponInfo( int weapIndex, const parseField_t *fields, int numFields ) {
	weaponInfo_t *weaponInfo;
	int i;

	weaponInfo = trap_Hunk_AllocLowInternal( sizeof( weaponInfo_t ) );
	bg_weaponInfo[weapIndex] = weaponInfo;
	weaponInfo->weapIndex = weapIndex;
	SetConfigString( (char **)&weaponInfo->szInternalName, "" );
	for ( i = 0; i < numFields; i++ ) {
		if ( fields[i].type == WFT_STRING ) {
			SetConfigString2( (byte *)weaponInfo + fields[i].ofs, "" );
		}
	}
	return weaponInfo;
}

/*
==============
BG_ParseWeaponInfoFiles
==============
*/
int BG_ParseWeaponInfoFiles( char **weaponFiles, int numWeaponFiles ) {
	weaponInfo_t *weaponInfo;
	char filename[64];
	char buffer[8196];
	fileHandle_t f;
	int prefixLen;
	int len;
	int i;

	prefixLen = strlen( "WEAPONFILE" );
	bg_szEmptyString = trap_Hunk_AllocLowAlignInternal( 1, 1 );
	bg_szEmptyString[0] = 0;

	/* index 0 is the "none" weapon; it has no file */
	weaponInfo = InitWeaponInfo( 0, bg_weaponInfoFields, WEAPONINFO_NUM_FIELDS );
	SetConfigString( (char **)&weaponInfo->szInternalName, "none" );

	bg_numWeapons = 0;
	for ( i = 0; i < numWeaponFiles; i++ ) {
		weaponInfo = InitWeaponInfo( ++bg_numWeapons, bg_weaponInfoFields, WEAPONINFO_NUM_FIELDS );

		sprintf( filename, "%s/", bg_szWeaponsFolder );
		strcat( filename, weaponFiles[i] );
		Com_DPrintf( "Parsing weapon file \"%s\"...\n", filename );

		len = trap_FS_FOpenFile( filename, &f, FS_READ );
		if ( len <= 0 ) {
			Com_Error( ERR_DROP, "\x15" "Could not load weapon file '%s'", filename );
		}

		trap_FS_Read( buffer, prefixLen, f );
		buffer[prefixLen] = 0;
		if ( strncmp( buffer, "WEAPONFILE", prefixLen ) ) {
			Com_Error( ERR_DROP, "\x15" "\"%s\" does not appear to be a weapon file", filename );
		}

		len -= prefixLen;
		if ( len >= 8192 ) {
			Com_Error( ERR_DROP, "\x15" "\"%s\" Is too long of a weapon file to parse", filename );
		}

		memset( buffer, 0, 8192 );
		trap_FS_Read( buffer, len, f );
		buffer[len] = 0;
		trap_FS_FCloseFile( f );

		if ( strchr( buffer, '"' ) || strchr( buffer, ';' ) ) {
			Com_Error( ERR_DROP, "\x15" "\"%s\" is not a valid weapon file", filename );
		}

		SetConfigString( (char **)&weaponInfo->szInternalName, weaponFiles[i] );

		if ( !ParseConfigStringToStruct( bg_weaponInfoFields, weaponInfo, WEAPONINFO_NUM_FIELDS, buffer,
										 WFT_NUM, BG_ParseWeaponInfoSpecificFieldType, SetConfigString2 ) ) {
			bg_weaponInfo[bg_numWeapons] = NULL;
			bg_numWeapons--;
		}
	}
	return i;
}

/*
==============
BG_SetupTransitionTimes

The two reciprocals at +0x414 and +0x418 have no parse-table entry; they are
derived here so the ADS lerp is a multiply.  The fallbacks are 1/300 and 1/500.
==============
*/
void BG_SetupTransitionTimes( void ) {
	weaponInfo_t *weaponInfo;
	int i;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		weaponInfo = bg_weaponInfo[i];
		if ( weaponInfo->adsTransInTime > 0 ) {
			weaponInfo->adsTransInRate = 1.0f / weaponInfo->adsTransInTime;
		} else {
			weaponInfo->adsTransInRate = 0.0033333334f;
		}
		if ( weaponInfo->adsTransOutTime > 0 ) {
			weaponInfo->adsTransOutRate = 1.0f / weaponInfo->adsTransOutTime;
		} else {
			weaponInfo->adsTransOutRate = 0.0020000001f;
		}
	}
}

/*
==============
BG_FillInWeaponItems

Overwrite the weapon placeholder rows of bg_itemlist from the parsed weapon
files, then bind every remaining IT_AMMO row to the weapon whose internal name
prefixes its pickup name.
==============
*/
void BG_FillInWeaponItems( void ) {
	const weaponInfo_t *weaponInfo;
	gitem_t *item;
	int i, j;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		item = &bg_itemlist[i];
		weaponInfo = bg_weaponInfo[i];

		item->classname = weaponInfo->radiantName;
		item->pickup_sound = weaponInfo->pickupSound;
		item->world_model[0] = weaponInfo->worldModel;
		item->world_model[1] = NULL;
		item->icon = weaponInfo->hudIcon;
		item->ammoicon = weaponInfo->ammoIcon;
		item->pickup_name = weaponInfo->displayName;
		item->quantity = weaponInfo->startAmmo;
		item->giType = IT_WEAPON;
		item->giTag = i;
		item->giAmmoIndex = weaponInfo->ammoIndex;
		item->giClipIndex = weaponInfo->clipIndex;
	}

	for ( ; i < BG_NUM_ITEMS; i++ ) {
		item = &bg_itemlist[i];
		if ( item->giType != IT_AMMO ) {
			continue;
		}
		for ( j = 1; j <= bg_numWeapons; j++ ) {
			weaponInfo = bg_weaponInfo[j];
			if ( !Q_stricmpn( weaponInfo->szInternalName, item->pickup_name, strlen( weaponInfo->szInternalName ) ) ) {
				item->giTag = j;
				item->giAmmoIndex = weaponInfo->ammoIndex;
				item->giClipIndex = weaponInfo->clipIndex;
				break;
			}
		}
		if ( item->giTag == -1 ) {
			Com_Printf( "^3WARNING^7: Could not find weapon for ammo item %s\n", item->pickup_name );
			item->giTag = 1;
			item->giAmmoIndex = bg_weaponInfo[1]->ammoIndex;
			item->giClipIndex = bg_weaponInfo[1]->clipIndex;
		}
	}
}

/*
==============
BG_SetupAmmoIndexes
==============
*/
void BG_SetupAmmoIndexes( void ) {
	weaponInfo_t *weaponInfo;
	weaponInfo_t *other;
	int i, j, k;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		weaponInfo = bg_weaponInfo[i];
		Q_strlwr( (char *)weaponInfo->ammoName );

		for ( j = 0; j < bg_numAmmoTypes; j++ ) {
			if ( Q_stricmp( bg_ammoTypeNames[j], weaponInfo->ammoName ) ) {
				continue;
			}
			weaponInfo->ammoIndex = j;
			if ( bg_ammoTypeMax[j] != weaponInfo->maxAmmo && j ) {
				for ( k = 1; k < i; k++ ) {
					other = bg_weaponInfo[k];
					if ( !Q_stricmp( bg_ammoTypeNames[j], other->ammoName ) && other->maxAmmo == bg_ammoTypeMax[j] ) {
						Com_Error( ERR_DROP,
								   "\x15" "Max ammo mismatch for \"%s\" ammo: '%s\" set it to %i, but \"%s\" already set it to %i.\n",
								   weaponInfo->ammoName, weaponInfo->szInternalName, weaponInfo->maxAmmo,
								   other->szInternalName, other->maxAmmo );
					}
				}
			}
			break;
		}

		if ( j == bg_numAmmoTypes ) {
			bg_ammoTypeNames[j] = (char *)weaponInfo->ammoName;
			bg_ammoTypeMax[j] = weaponInfo->maxAmmo;
			weaponInfo->ammoIndex = j;
			bg_numAmmoTypes++;
		}
	}
}

/*
==============
BG_SetupSharedAmmoIndexes
==============
*/
void BG_SetupSharedAmmoIndexes( void ) {
	weaponInfo_t *weaponInfo;
	weaponInfo_t *other;
	int i, j, k;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		weaponInfo = bg_weaponInfo[i];
		weaponInfo->sharedAmmoCapIndex = -1;
		if ( !weaponInfo->sharedAmmoCapName[0] ) {
			continue;
		}

		Com_DPrintf( "%s: %s\n", weaponInfo->szInternalName, weaponInfo->sharedAmmoCapName );
		Q_strlwr( (char *)weaponInfo->sharedAmmoCapName );

		for ( j = 0; j < bg_numSharedAmmoCaps; j++ ) {
			if ( Q_stricmp( weaponInfo->sharedAmmoCapName, bg_sharedAmmoCapNames[j] ) ) {
				continue;
			}
			weaponInfo->sharedAmmoCapIndex = j;
			if ( bg_sharedAmmoCapSizes[j] != weaponInfo->sharedAmmoCap && j ) {
				for ( k = 1; k < i; k++ ) {
					other = bg_weaponInfo[k];
					if ( !Q_stricmp( other->sharedAmmoCapName, bg_sharedAmmoCapNames[j] )
						 && other->sharedAmmoCap == bg_sharedAmmoCapSizes[j] ) {
						Com_Error( ERR_DROP,
								   "\x15" "Shared ammo cap mismatch for \"%s\" shared ammo cap: '%s\" set it to %i, but \"%s\" already set it to %i.\n",
								   weaponInfo->sharedAmmoCapName, weaponInfo->szInternalName, weaponInfo->sharedAmmoCap,
								   other->szInternalName, other->sharedAmmoCap );
					}
				}
			}
			break;
		}

		if ( j == bg_numSharedAmmoCaps ) {
			bg_sharedAmmoCapNames[j] = (char *)weaponInfo->sharedAmmoCapName;
			bg_sharedAmmoCapSizes[j] = weaponInfo->sharedAmmoCap;
			weaponInfo->sharedAmmoCapIndex = j;
			bg_numSharedAmmoCaps++;
		}
	}
}

/*
==============
BG_SetupClipIndexes

The mismatch diagnostic prints ammoName where it says "clip"; retail does the
same thing.
==============
*/
void BG_SetupClipIndexes( void ) {
	weaponInfo_t *weaponInfo;
	weaponInfo_t *other;
	int i, j, k;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		weaponInfo = bg_weaponInfo[i];
		Q_strlwr( (char *)weaponInfo->clipName );

		for ( j = 0; j < bg_numAmmoClips; j++ ) {
			if ( Q_stricmp( bg_ammoClipNames[j], weaponInfo->clipName ) ) {
				continue;
			}
			weaponInfo->clipIndex = j;
			if ( bg_ammoClipSizes[j] != weaponInfo->clipSize && j ) {
				for ( k = 1; k < i; k++ ) {
					other = bg_weaponInfo[k];
					if ( !Q_stricmp( bg_ammoClipNames[j], other->clipName ) && other->clipSize == bg_ammoClipSizes[j] ) {
						Com_Error( ERR_DROP,
								   "\x15" "Clip Size mismatch for \"%s\" clip: '%s\" set it to %i, but \"%s\" already set it to %i.\n",
								   weaponInfo->ammoName, weaponInfo->szInternalName, weaponInfo->clipSize,
								   other->szInternalName, other->clipSize );
					}
				}
			}
			break;
		}

		if ( j == bg_numAmmoClips ) {
			bg_ammoClipNames[j] = (char *)weaponInfo->clipName;
			bg_ammoClipSizes[j] = weaponInfo->clipSize;
			weaponInfo->clipIndex = j;
			bg_numAmmoClips++;
		}
	}
}

/*
==============
BG_SetupWeaponAlts

Resolve altWeapon names into indexes and check that the ring each weapon starts
closes back on itself.
==============
*/
void BG_SetupWeaponAlts( void ) {
	weaponInfo_t *weaponInfo;
	weaponInfo_t *cur;
	weaponInfo_t *alt;
	int i, j;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		bg_weaponInfo[i]->altWeaponIndex = 0;
	}

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		weaponInfo = bg_weaponInfo[i];
		if ( weaponInfo->altWeaponIndex || !weaponInfo->altWeapon[0] ) {
			continue;
		}

		cur = weaponInfo;
		while ( !cur->altWeaponIndex ) {
			for ( j = 1; j <= bg_numWeapons; j++ ) {
				alt = bg_weaponInfo[j];
				if ( _stricmp( cur->altWeapon, alt->szInternalName ) ) {
					continue;
				}
				cur->altWeaponIndex = j;
				if ( cur->weaponSlot != alt->weaponSlot ) {
					Com_Error( ERR_DROP, "\x15" "weapon '%s' does not have same weaponSlot setting as its alt weapon '%s'",
							   cur->szInternalName, alt->szInternalName );
				}
				if ( cur->slotStackable != alt->slotStackable ) {
					Com_Error( ERR_DROP, "\x15" "weapon '%s' does not have same slotStackable setting as its alt weapon '%s'",
							   cur->szInternalName, alt->szInternalName );
				}
				break;
			}
			if ( !cur->altWeaponIndex ) {
				Com_Error( ERR_DROP, "\x15" "could not find altWeapon '%s' for weapon '%s'",
						   cur->altWeapon, cur->szInternalName );
			}
			cur = bg_weaponInfo[j];
		}

		if ( cur != weaponInfo ) {
			Com_Error( ERR_DROP, "\x15" "weapon '%s' has a bad altWeapon '%s'",
					   weaponInfo->szInternalName, weaponInfo->altWeapon );
		}
	}
}

/*
==============
BG_SetupUseHintStrings

The index G_GetHintStringIndex fills in is weaponInfo_t +0x40C, which the parse
table never names (0x2000EC74).

Game build only: neither this function nor its call site survives into
cgame_mp_x86.dll, and neither does its Com_Error string.
==============
*/
#ifndef CGAMEDLL
void BG_SetupUseHintStrings( void ) {
	weaponInfo_t *weaponInfo;
	int i;

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		weaponInfo = bg_weaponInfo[i];
		if ( !weaponInfo->useHintString[0] ) {
			continue;
		}
		if ( !G_GetHintStringIndex( &weaponInfo->useHintStringIndex, weaponInfo->useHintString ) ) {
			Com_Error( ERR_DROP, "\x15" "Too many different hintstring values on weapons. Max allowed is %i different strings", 32 );
		}
	}
}
#endif

/*
==============
compare_weaponfile_names
==============
*/
static int compare_weaponfile_names( const void *a, const void *b ) {
	return Q_stricmp( *(const char **)a, *(const char **)b );
}

/*
==============
BG_SetupWeaponInfo

The weaponInfo array is engine memory: trap_GetWeaponInfoMemory hands back the
previous owner's block when there is one, and in that case the files have
already been parsed -- only the count has to be recovered.

The two builds diverge in the !prevOwner arm, and the divergence is the whole
reason the client copy is 461 bytes (0x3000EF10) to the game's 732.  The server
enumerates the weapons folder, sorts it, joins it with spaces and PUBLISHES it
as CS_WEAPONFILES; the client SPLITS that configstring back apart and parses the
same list.  So the client has no trap_SetConfigstring, no trap_FS_GetFileList,
no qsort and no filelist[4096] -- its frame is weaponFiles[63], prevOwner and
bigstring[8196] and nothing else (alloca 0x2110 at 0x3000EF16, buffer at
frame+0x108).  It also has neither Com_DPrintf: "----------------------" and
"Game: BG_SetupWeaponInfo" are absent from the image altogether.
==============
*/
void BG_SetupWeaponInfo( void ) {
	char *weaponFiles[MAX_WEAPONS - 1];
#ifndef CGAMEDLL
	char filelist[4096];
#endif
	char bigstring[8196];
#ifndef CGAMEDLL
	char *fileptr;
	int filelen;
#endif
#ifdef CGAMEDLL
	char *p;
#endif
	int numWeaponFiles;
	int prevOwner;
	int i;

#ifndef CGAMEDLL
	Com_DPrintf( "----------------------\n" );
	Com_DPrintf( "Game: BG_SetupWeaponInfo\n" );
#endif

	bg_weaponInfo = trap_GetWeaponInfoMemory( MAX_WEAPONS * sizeof( weaponInfo_t * ), &prevOwner );
	if ( !bg_weaponInfo ) {
		Com_Error( ERR_DROP, "\x15" "Could not allocate WeaponInfo array\n" );
	}

	memset( bg_ammoTypeNames, 0, sizeof( bg_ammoTypeNames ) );
	memset( bg_ammoClipNames, 0, sizeof( bg_ammoClipNames ) );
	memset( weaponFiles, 0, sizeof( weaponFiles ) );

	bg_ammoTypeNames[0] = "none";
	bg_ammoClipNames[0] = "none";
	bg_ammoTypeMax[0] = 0;
	bg_numAmmoTypes = 1;
	bg_ammoClipSizes[0] = 0;
	bg_numAmmoClips = 1;

	if ( prevOwner ) {
		bg_numWeapons = 0;
		for ( i = 1; i < MAX_WEAPONS; i++ ) {
			if ( !bg_weaponInfo[i] ) {
				break;
			}
			bg_numWeapons++;
		}
	} else {
#ifdef CGAMEDLL
		/*
		 * 0x3000EFC6..0x3000F040.  The list arrives as one space-separated
		 * configstring; the split is destructive and runs in place.  A run of
		 * two spaces produces no entry, and a trailing space ends the scan.
		 */
		strcpy( bigstring, CG_ConfigString( CS_WEAPONFILES ) );

		weaponFiles[0] = bigstring;
		numWeaponFiles = 1;
		p = bigstring;
		while ( *p ) {
			if ( *p == ' ' ) {
				*p++ = 0;
				if ( !*p ) {
					break;
				}
				if ( *p != ' ' ) {
					weaponFiles[numWeaponFiles++] = p;
				}
			} else {
				p++;
			}
		}
#else
		numWeaponFiles = trap_FS_GetFileList( bg_szWeaponsFolder, "", filelist, sizeof( filelist ) );
		if ( numWeaponFiles < 1 ) {
			Com_Error( ERR_DROP, "\x15" "No weapon files found in %s.\n", bg_szWeaponsFolder );
		}
		if ( numWeaponFiles > MAX_WEAPONS - 1 ) {
			Com_Error( ERR_DROP, "\x15" "Max number of weapons allowed is %i, found %i.\n", MAX_WEAPONS - 1, numWeaponFiles );
		}

		fileptr = filelist;
		for ( i = 0; i < numWeaponFiles; i++ ) {
			if ( i >= MAX_WEAPONS - 1 ) {
				break;
			}
			filelen = strlen( fileptr );
			Com_DPrintf( "Getting weapon file \"%s/%s\" for parsing\n", bg_szWeaponsFolder, fileptr );
			weaponFiles[i] = fileptr;
			fileptr += filelen + 1;
		}

		qsort( weaponFiles, numWeaponFiles, sizeof( weaponFiles[0] ), compare_weaponfile_names );

		bigstring[0] = 0;
		for ( i = 0; i < numWeaponFiles; i++ ) {
			if ( i > 0 ) {
				strcat( bigstring, " " );
			}
			strcat( bigstring, weaponFiles[i] );
		}
		trap_SetConfigstring( CS_WEAPONFILES, bigstring );
#endif

		BG_ParseWeaponInfoFiles( weaponFiles, numWeaponFiles );
	}

	BG_SetupTransitionTimes();
	BG_SetupAmmoIndexes();
	BG_SetupSharedAmmoIndexes();
	BG_SetupClipIndexes();
	BG_FillInWeaponItems();
	BG_SetupWeaponAlts();
#ifndef CGAMEDLL
	BG_SetupUseHintStrings();

	Com_DPrintf( "----------------------\n" );
#endif
}

/*
=============================================================================

	REGISTRY ACCESSORS

=============================================================================
*/

weaponInfo_t *BG_GetInfoForWeapon( int weaponIndex ) {
	return bg_weaponInfo[weaponIndex];
}

/* Name inferred from United Offensive; the retail symbol was not
   recovered. */
int BG_GetWeaponForInfo( const weaponInfo_t *weaponInfo ) {
	return weaponInfo->weapIndex;
}

int BG_GetNumWeapons( void ) {
	return bg_numWeapons;
}

int BG_GetNumAmmoTypes( void ) {
	return bg_numAmmoTypes;
}

int BG_GetAmmoTypeMax( int ammoIndex ) {
	return bg_ammoTypeMax[ammoIndex];
}

int BG_GetNumAmmoClips( void ) {
	return bg_numAmmoClips;
}

int BG_GetAmmoClipSize( int clipIndex ) {
	return bg_ammoClipSizes[clipIndex];
}

int BG_GetSharedAmmoCapSize( int sharedAmmoCapIndex ) {
	return bg_sharedAmmoCapSizes[sharedAmmoCapIndex];
}

const char *BG_GetAmmoTypeName( int ammoIndex ) {
	return bg_ammoTypeNames[ammoIndex];
}

const char *BG_GetAmmoClipName( int clipIndex ) {
	return bg_ammoClipNames[clipIndex];
}

int BG_GetAmmoTypeForName( const char *name ) {
	int i;

	for ( i = 0; i < bg_numAmmoTypes; i++ ) {
		if ( !Q_stricmp( name, bg_ammoTypeNames[i] ) ) {
			return i;
		}
	}
	Com_DPrintf( "Couldn't find ammo type \"%s\"\n", name );
	return 0;
}

int BG_GetAmmoClipForName( const char *name ) {
	int i;

	for ( i = 0; i < bg_numAmmoClips; i++ ) {
		if ( !Q_stricmp( name, bg_ammoClipNames[i] ) ) {
			return i;
		}
	}
	Com_DPrintf( "Couldn't find ammo clip \"%s\"\n", name );
	return 0;
}

int BG_GetWeaponSlotForName( const char *name ) {
	int i;

	for ( i = 0; i < WEAPSLOT_NUM; i++ ) {
		if ( !_stricmp( name, bg_weaponSlotNames[i] ) ) {
			return i;
		}
	}
	return WEAPSLOT_NONE;
}

const char *BG_GetWeaponSlotNameForIndex( weapSlot_t weaponSlot ) {
	return bg_weaponSlotNames[weaponSlot];
}

/*
==============
BG_GetWeaponIndexForName

The retail leaf returns only the low byte of the match.
==============
*/
int BG_GetWeaponIndexForName( const char *name ) {
	int i;

	for ( i = 0; i <= bg_numWeapons; i++ ) {
		if ( !Q_stricmp( bg_weaponInfo[i]->szInternalName, name ) ) {
			return i & 0xFF;
		}
	}
	Com_DPrintf( "Couldn't find weapon \"%s\"\n", name );
	return 0;
}

qboolean BG_IsAimDownSightWeapon( int weaponIndex ) {
	return bg_weaponInfo[weaponIndex]->aimDownSight;
}

/*
=============================================================================

	PLAYER WEAPON INVENTORY

=============================================================================
*/

/*
==============
BG_GivePlayerWeapon
==============
*/
qboolean BG_GivePlayerWeapon( playerState_t *ps, int weapon ) {
	const weaponInfo_t *weaponInfo;
	unsigned int bit;
	int word;
	int alt;

	bit = 1 << ( weapon & 31 );
	word = weapon >> 5;
	if ( ps->weapons[word] & bit ) {
		return qfalse;
	}

	weaponInfo = bg_weaponInfo[weapon];
	if ( weaponInfo->weaponClass == WEAPCLASS_TURRET || weaponInfo->weaponClass == WEAPCLASS_NON_PLAYER ) {
		return qfalse;
	}

#ifndef CGAMEDLL
	RegisterItem( weapon, qtrue );
#endif
	ps->weapons[word] |= bit;
	ps->weaponrechamber[word] &= ~bit;

	switch ( weaponInfo->weaponSlot ) {
	case WEAPSLOT_PRIMARY:
	case WEAPSLOT_PRIMARYB:
		if ( !ps->weaponslots[WEAPSLOT_PRIMARY] ) {
			ps->weaponslots[WEAPSLOT_PRIMARY] = (byte)weapon;
		} else if ( !ps->weaponslots[WEAPSLOT_PRIMARYB] ) {
			ps->weaponslots[WEAPSLOT_PRIMARYB] = (byte)weapon;
		}
		break;
	case WEAPSLOT_PISTOL:
	case WEAPSLOT_GRENADE:
	case WEAPSLOT_SMOKEGRENADE:
		if ( !ps->weaponslots[weaponInfo->weaponSlot] ) {
			ps->weaponslots[weaponInfo->weaponSlot] = (byte)weapon;
		}
		break;
	default:
		break;
	}

	for ( alt = weaponInfo->altWeaponIndex; alt; alt = bg_weaponInfo[alt]->altWeaponIndex ) {
		if ( ps->weapons[alt >> 5] & ( 1 << ( alt & 31 ) ) ) {
			break;
		}
#ifndef CGAMEDLL
		RegisterItem( alt, qtrue );
#endif
		ps->weapons[alt >> 5] |= 1 << ( alt & 31 );
		/* retail clears the bit of the weapon that was granted first, not the
		 * alternate's own bit */
		ps->weaponrechamber[word] &= ~bit;
	}
	return qtrue;
}

/*
==============
BG_TakePlayerWeapon
==============
*/
int BG_TakePlayerWeapon( playerState_t *ps, int weapon ) {
	const weaponInfo_t *weaponInfo;
	unsigned int bit;
	int word;
	int slot;
	int i;
	int alt;

	bit = 1 << ( weapon & 31 );
	word = weapon >> 5;
	if ( !( ps->weapons[word] & bit ) ) {
		return 0;
	}

	weaponInfo = bg_weaponInfo[weapon];
	slot = BG_IsPlayerWeaponInSlot( ps, weapon, qtrue );
	if ( slot ) {
		if ( !weaponInfo->slotStackable ) {
			ps->weaponslots[slot] = 0;
		} else {
			for ( i = 1; i <= bg_numWeapons; i++ ) {
				/* retail re-tests the weapon being removed, not the candidate */
				if ( !weaponInfo->slotStackable ) {
					continue;
				}
				if ( !( ps->weapons[i >> 5] & ( 1 << ( i & 31 ) ) ) ) {
					continue;
				}
				if ( BG_IsPlayerWeaponInSlot( ps, i, qtrue ) ) {
					continue;
				}
				ps->weaponslots[slot] = (byte)i;
				break;
			}
			if ( i > bg_numWeapons ) {
				ps->weaponslots[slot] = 0;
			}
		}
	}

	ps->weapons[word] &= ~bit;

	for ( alt = weaponInfo->altWeaponIndex; alt; alt = bg_weaponInfo[alt]->altWeaponIndex ) {
		if ( !( ps->weapons[alt >> 5] & ( 1 << ( alt & 31 ) ) ) ) {
			break;
		}
		ps->weapons[alt >> 5] &= ~( 1 << ( alt & 31 ) );
	}
	return 1;
}

/*
==============
BG_SetPlayerWeaponForSlot
==============
*/
qboolean BG_SetPlayerWeaponForSlot( playerState_t *ps, int slot, int weapon ) {
	int weaponSlot;

	if ( !( ps->weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
		return qfalse;
	}

	weaponSlot = bg_weaponInfo[weapon]->weaponSlot;
	switch ( weaponSlot ) {
	case WEAPSLOT_PRIMARY:
	case WEAPSLOT_PRIMARYB:
		if ( slot != WEAPSLOT_PRIMARY && slot != WEAPSLOT_PRIMARYB ) {
			return qfalse;
		}
		break;
	case WEAPSLOT_PISTOL:
	case WEAPSLOT_GRENADE:
	case WEAPSLOT_SMOKEGRENADE:
		if ( slot != weaponSlot ) {
			return qfalse;
		}
		break;
	default:
		return qfalse;
	}

	ps->weaponslots[slot] = (byte)weapon;
	return qtrue;
}

/*
==============
BG_IsPlayerWeaponInSlot

Returns the slot the weapon (or, with includeAltWeapons, any weapon on its alt
ring) occupies, or WEAPSLOT_NONE.
==============
*/
int BG_IsPlayerWeaponInSlot( const playerState_t *ps, int weapon, qboolean includeAltWeapons ) {
	const weaponInfo_t *weaponInfo;
	int cur;
	int alt;

	if ( !( ps->weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
		return WEAPSLOT_NONE;
	}

	cur = weapon;
	do {
		weaponInfo = bg_weaponInfo[cur];
		switch ( weaponInfo->weaponSlot ) {
		case WEAPSLOT_PRIMARY:
		case WEAPSLOT_PRIMARYB:
			if ( (signed char)ps->weaponslots[WEAPSLOT_PRIMARY] == cur ) {
				return WEAPSLOT_PRIMARY;
			}
			if ( (signed char)ps->weaponslots[WEAPSLOT_PRIMARYB] == cur ) {
				return WEAPSLOT_PRIMARYB;
			}
			break;
		case WEAPSLOT_PISTOL:
		case WEAPSLOT_GRENADE:
		case WEAPSLOT_SMOKEGRENADE:
			if ( (signed char)ps->weaponslots[weaponInfo->weaponSlot] == cur ) {
				return weaponInfo->weaponSlot;
			}
			break;
		default:
			return WEAPSLOT_NONE;
		}

		if ( includeAltWeapons ) {
			alt = weaponInfo->altWeaponIndex;
			if ( alt ) {
				cur = alt;
			}
		}
	} while ( cur != weapon );

	return WEAPSLOT_NONE;
}

/*
==============
BG_GetEmptySlotForWeapon
==============
*/
int BG_GetEmptySlotForWeapon( const playerState_t *ps, int weapon ) {
	int weaponSlot;

	weaponSlot = bg_weaponInfo[weapon]->weaponSlot;
	switch ( weaponSlot ) {
	case WEAPSLOT_PRIMARY:
	case WEAPSLOT_PRIMARYB:
		if ( !ps->weaponslots[WEAPSLOT_PRIMARY] ) {
			return WEAPSLOT_PRIMARY;
		}
		if ( !ps->weaponslots[WEAPSLOT_PRIMARYB] ) {
			return WEAPSLOT_PRIMARYB;
		}
		return WEAPSLOT_NONE;
	case WEAPSLOT_PISTOL:
	case WEAPSLOT_GRENADE:
	case WEAPSLOT_SMOKEGRENADE:
		if ( ps->weaponslots[weaponSlot] ) {
			return WEAPSLOT_NONE;
		}
		return weaponSlot;
	default:
		return WEAPSLOT_NONE;
	}
}

/*
==============
BG_GetStackSlotForWeapon

The slot a stackable weapon may share; preferredSlot wins when it holds another
stackable.
==============
*/
int BG_GetStackSlotForWeapon( const playerState_t *ps, int weapon, int preferredSlot ) {
	const weaponInfo_t *weaponInfo;
	int weaponSlot;
	int occupant;

	weaponInfo = bg_weaponInfo[weapon];
	if ( !weaponInfo->slotStackable ) {
		return WEAPSLOT_NONE;
	}

	weaponSlot = weaponInfo->weaponSlot;
	switch ( weaponSlot ) {
	case WEAPSLOT_PRIMARY:
	case WEAPSLOT_PRIMARYB:
		if ( preferredSlot == WEAPSLOT_PRIMARY || preferredSlot == WEAPSLOT_PRIMARYB ) {
			occupant = (signed char)ps->weaponslots[preferredSlot];
			if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
				return preferredSlot;
			}
		}
		occupant = (signed char)ps->weaponslots[WEAPSLOT_PRIMARY];
		if ( !occupant || bg_weaponInfo[occupant]->slotStackable ) {
			return WEAPSLOT_PRIMARY;
		}
		occupant = (signed char)ps->weaponslots[WEAPSLOT_PRIMARYB];
		if ( occupant && !bg_weaponInfo[occupant]->slotStackable ) {
			return WEAPSLOT_NONE;
		}
		return WEAPSLOT_PRIMARYB;
	case WEAPSLOT_PISTOL:
	case WEAPSLOT_GRENADE:
	case WEAPSLOT_SMOKEGRENADE:
		occupant = (signed char)ps->weaponslots[weaponSlot];
		if ( occupant && !bg_weaponInfo[occupant]->slotStackable ) {
			return WEAPSLOT_NONE;
		}
		return weaponSlot;
	default:
		return WEAPSLOT_NONE;
	}
}

/*
==============
BG_IsPlayerWeaponAnAlt
==============
*/
qboolean BG_IsPlayerWeaponAnAlt( int weapon, int altWeapon ) {
	int cur;

	for ( cur = bg_weaponInfo[weapon]->altWeaponIndex; cur; cur = bg_weaponInfo[cur]->altWeaponIndex ) {
		if ( cur == altWeapon ) {
			return qtrue;
		}
		if ( cur == weapon ) {
			return qfalse;
		}
	}
	return qfalse;
}

/*
==============
BG_GetMaxPickupableAmmo

Headroom left in the weapon's own pool, or in the shared pool once every other
owned weapon that draws on it has been subtracted.
==============
*/
int BG_GetMaxPickupableAmmo( const playerState_t *ps, int weapon ) {
	const weaponInfo_t *weaponInfo;
	const weaponInfo_t *other;
	int seenAmmo[MAX_WEAPONS];
	int seenClip[MAX_WEAPONS];
	int ammoIndex, clipIndex, sharedIndex;
	int remaining;
	int i;

	weaponInfo = bg_weaponInfo[weapon];
	ammoIndex = weaponInfo->ammoIndex;
	clipIndex = weaponInfo->clipIndex;
	memset( seenAmmo, 0, sizeof( seenAmmo ) );
	memset( seenClip, 0, sizeof( seenClip ) );

	sharedIndex = weaponInfo->sharedAmmoCapIndex;
	if ( sharedIndex < 0 ) {
		if ( weaponInfo->clipOnly ) {
			return bg_ammoClipSizes[clipIndex] - ps->ammoclip[clipIndex];
		}
		return bg_ammoTypeMax[ammoIndex] - ps->ammo[ammoIndex];
	}

	remaining = bg_sharedAmmoCapSizes[sharedIndex];
	for ( i = 1; i <= bg_numWeapons; i++ ) {
		if ( !( ps->weapons[i >> 5] & ( 1 << ( i & 31 ) ) ) ) {
			continue;
		}
		other = bg_weaponInfo[i];
		if ( other->sharedAmmoCapIndex != sharedIndex ) {
			continue;
		}
		if ( other->clipOnly ) {
			if ( !seenClip[other->clipIndex] ) {
				seenClip[other->clipIndex] = 1;
				remaining -= ps->ammoclip[other->clipIndex];
			}
		} else {
			if ( !seenAmmo[other->ammoIndex] ) {
				seenAmmo[other->ammoIndex] = 1;
				remaining -= ps->ammo[other->ammoIndex];
			}
		}
	}
	return remaining;
}

/*
==============
BG_GetTotalAmmoReserve
==============
*/
int BG_GetTotalAmmoReserve( const playerState_t *ps, int weapon ) {
	const weaponInfo_t *weaponInfo;
	const weaponInfo_t *other;
	int seenAmmo[MAX_WEAPONS];
	int seenClip[MAX_WEAPONS];
	int ammoIndex, clipIndex, sharedIndex;
	int total;
	int i;

	weaponInfo = bg_weaponInfo[weapon];
	clipIndex = weaponInfo->clipIndex;
	ammoIndex = weaponInfo->ammoIndex;
	memset( seenAmmo, 0, sizeof( seenAmmo ) );
	memset( seenClip, 0, sizeof( seenClip ) );

	sharedIndex = weaponInfo->sharedAmmoCapIndex;
	total = 0;
	if ( sharedIndex < 0 ) {
		if ( weaponInfo->clipOnly ) {
			return ps->ammoclip[clipIndex];
		}
		return ps->ammo[ammoIndex];
	}

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		if ( !( ps->weapons[i >> 5] & ( 1 << ( i & 31 ) ) ) ) {
			continue;
		}
		other = bg_weaponInfo[i];
		if ( other->sharedAmmoCapIndex != sharedIndex ) {
			continue;
		}
		if ( other->clipOnly ) {
			if ( !seenClip[other->clipIndex] ) {
				seenClip[other->clipIndex] = 1;
				total += ps->ammoclip[other->clipIndex];
			}
		} else {
			if ( !seenAmmo[other->ammoIndex] ) {
				seenAmmo[other->ammoIndex] = 1;
				total += ps->ammo[other->ammoIndex];
			}
		}
	}
	return total;
}

/*
==============
BG_GetMinSpreadForWeapon

While the view height is lerping between stances the minimum hip spread lerps
with it.
==============
*/
float BG_GetMinSpreadForWeapon( int time, const playerState_t *ps, int weaponIndex ) {
	const weaponInfo_t *weaponInfo;
	int target;
	float frac;

	weaponInfo = bg_weaponInfo[weaponIndex];
	target = ps->viewHeightLerpTarget;

	if ( (float)target != ps->viewHeightCurrent && ps->viewHeightLerpTime ) {
		frac = (float)( (double)( time - ps->viewHeightLerpTime )
						/ (double)PM_GetViewHeightLerpTime( ps, target, ps->viewHeightLerpDown ) );
		if ( frac < 0.0f ) {
			frac = 0.0f;
		} else if ( frac > 1.0f ) {
			frac = 1.0f;
		}

		if ( target == ps->proneViewHeight ) {
			return ( weaponInfo->hipSpreadProneMin - weaponInfo->hipSpreadDuckedMin ) * frac + weaponInfo->hipSpreadDuckedMin;
		}
		if ( target == ps->standViewHeight ) {
			return ( weaponInfo->hipSpreadStandMin - weaponInfo->hipSpreadDuckedMin ) * frac + weaponInfo->hipSpreadDuckedMin;
		}
		if ( ps->viewHeightLerpDown ) {
			return ( weaponInfo->hipSpreadDuckedMin - weaponInfo->hipSpreadStandMin ) * frac + weaponInfo->hipSpreadStandMin;
		}
		return ( weaponInfo->hipSpreadDuckedMin - weaponInfo->hipSpreadProneMin ) * frac + weaponInfo->hipSpreadProneMin;
	}

	if ( ps->pm_flags & PMF_PRONE ) {
		return weaponInfo->hipSpreadProneMin;
	}
	if ( ps->pm_flags & PMF_DUCKED ) {
		return weaponInfo->hipSpreadDuckedMin;
	}
	return weaponInfo->hipSpreadStandMin;
}

/*
=============================================================================

	AIM DOWN SIGHT

=============================================================================
*/

/*
==============
PM_UpdateAimDownSightFlag
==============
*/
void PM_UpdateAimDownSightFlag( void ) {
	playerState_t *ps;

	ps = pm->ps;
	if ( ps->pm_type >= PM_DEAD
		 || !( pm->cmd.buttons & BUTTON_ADS )
		 || !pml.weaponInfo->aimDownSight
		 || ps->weaponstate == WEAPON_DROPPING
		 || ps->weaponstate == WEAPON_RAISING
		 || ps->weaponstate == WEAPON_MELEE_INIT
		 || ps->weaponstate == WEAPON_MELEE_FIRE
		 || ( !pml.groundPlane && ps->pm_type != PM_NORMAL_LINKED ) ) {
		ps->pm_flags &= ~PMF_ADS;
	} else if ( !( ps->pm_flags & PMF_PRONE ) ) {
		ps->pm_flags |= PMF_ADS;
	} else if ( !( pm->oldcmd.buttons & BUTTON_ADS ) || ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) ) {
		ps->pm_flags |= PMF_ADS;
		pm->ps->pm_flags |= PMF_PRONE_MOVEOVERRIDE;
	}

	BG_UpdateConditionValue( ps->clientNum, ANIM_COND_WEAPON_POSITION,
							 ( ps->pm_flags & PMF_ADS ) != 0, qtrue );
}

/*
==============
PM_ClearAimDownSightFlag
==============
*/
void PM_ClearAimDownSightFlag( void ) {
	pm->ps->pm_flags &= ~PMF_ADS;
}

/*
==============
PM_UpdateAimDownSightLerp
==============
*/
void PM_UpdateAimDownSightLerp( void ) {
	const weaponInfo_t *weaponInfo;
	playerState_t *ps;
	qboolean allowAds;
	qboolean bAds;

	weaponInfo = pml.weaponInfo;
	bAds = qfalse;
	if ( !weaponInfo->aimDownSight ) {
		pm->ps->fWeaponPosFrac = 0.0f;
		return;
	}

	allowAds = qtrue;
	ps = pm->ps;
	if ( !weaponInfo->segmentedReload ) {
		if ( ps->weaponstate == WEAPON_RELOADING
			 && ps->weaponTime - weaponInfo->adsReloadTransTime > 0 ) {
			allowAds = qfalse;
		}
	} else if ( ps->weaponstate == WEAPON_RELOADING
				|| ps->weaponstate == WEAPON_RELOADING_INTERUPT
				|| ps->weaponstate == WEAPON_RELOAD_START
				|| ps->weaponstate == WEAPON_RELOAD_START_INTERUPT
				|| ( ps->weaponstate == WEAPON_RELOAD_END
					 && ps->weaponTime - weaponInfo->adsReloadTransTime > 0 ) ) {
		allowAds = qfalse;
	}

	ps = pm->ps;
	if ( allowAds && ( ps->pm_flags & PMF_ADS ) ) {
		bAds = qtrue;
	}

	if ( ( weaponInfo->adsFire && ps->weaponDelay && ps->weaponstate == WEAPON_FIRING ) || bAds ) {
		if ( ps->fWeaponPosFrac == 1.0f ) {
			return;
		}
		ps->fWeaponPosFrac = (float)( ps->fWeaponPosFrac
									  + (double)pml.msec * weaponInfo->adsTransInRate );
	} else {
		if ( ps->fWeaponPosFrac == 0.0f ) {
			return;
		}
		ps->fWeaponPosFrac = (float)( ps->fWeaponPosFrac
									  - (double)pml.msec * weaponInfo->adsTransOutRate );
	}

	ps = pm->ps;
	if ( ps->fWeaponPosFrac >= 1.0f ) {
		ps->fWeaponPosFrac = 1.0f;
	} else if ( ps->fWeaponPosFrac <= 0.0f ) {
		ps->fWeaponPosFrac = 0.0f;
	}
}

/*
==============
PM_InteruptWeaponWithProneMove

qtrue when the stance change may proceed.
==============
*/
qboolean PM_InteruptWeaponWithProneMove( void ) {
	playerState_t *ps;

	ps = pm->ps;
	switch ( ps->weaponstate ) {
	case WEAPON_READY:
	case WEAPON_RAISING:
	case WEAPON_DROPPING:
	case WEAPON_RECHAMBERING:
	case WEAPON_RELOADING:
	case WEAPON_RELOADING_INTERUPT:
	case WEAPON_RELOAD_START:
	case WEAPON_RELOAD_START_INTERUPT:
	case WEAPON_RELOAD_END:
		return qtrue;
	case WEAPON_FIRING:
	case WEAPON_MELEE_FIRE:
		return qfalse;
	default:
		ps->weaponTime = 0;
		ps->weaponDelay = 0;
		ps->weaponstate = WEAPON_READY;
		PM_ContinueWeaponAnim( WEAP_IDLE );
		return qtrue;
	}
}

/*
=============================================================================

	AMMO PRIMITIVES

=============================================================================
*/

int BG_ClipForWeapon( int weaponIndex ) {
	return bg_weaponInfo[weaponIndex]->clipIndex;
}

int BG_AmmoForWeapon( int weaponIndex ) {
	return bg_weaponInfo[weaponIndex]->ammoIndex;
}

int BG_WeaponAmmo( int weaponIndex ) {
	return bg_weaponInfo[weaponIndex]->clipOnly;
}

int PM_ReloadClip( const playerState_t *ps, int weapon ) {
	const weaponInfo_t *weaponInfo;

	weaponInfo = bg_weaponInfo[weapon];
	return ps->ammoclip[weaponInfo->clipIndex] + ps->ammo[weaponInfo->ammoIndex];
}

/*
==============
PM_Weapon_ReloadClipAmmo

0x2000FDB0, no retail symbol: the tail PM_Weapon_ReloadDelayedAction jumps
to.  Moves one reload increment from the reserve into the clip and returns it.
==============
*/
int PM_Weapon_ReloadClipAmmo( void ) {
	const weaponInfo_t *weaponInfo;
	playerState_t *ps;
	int ammoIndex, clipIndex;
	int reserve, add, cap;

	ps = pm->ps;
	if ( ( ps->weaponstate == WEAPON_RELOAD_START || ps->weaponstate == WEAPON_RELOAD_START_INTERUPT )
		 && !pml.weaponInfo->reloadStartAdd ) {
		return pml.weaponInfo->reloadStartAdd;
	}

	weaponInfo = bg_weaponInfo[ps->weapon];
	ammoIndex = weaponInfo->ammoIndex;
	clipIndex = weaponInfo->clipIndex;
	reserve = ps->ammo[ammoIndex];
	add = bg_ammoClipSizes[clipIndex] - ps->ammoclip[clipIndex];
	if ( add > reserve ) {
		add = ps->ammo[ammoIndex];
	}

	if ( ps->weaponstate == WEAPON_RELOAD_START || ps->weaponstate == WEAPON_RELOAD_START_INTERUPT ) {
		cap = pml.weaponInfo->reloadStartAdd;
	} else {
		cap = pml.weaponInfo->reloadAmmoAdd;
		if ( !cap ) {
			goto transfer;
		}
	}
	if ( cap < bg_ammoClipSizes[clipIndex] && add > cap ) {
		add = cap;
	}

transfer:
	if ( add ) {
		ps->ammo[ammoIndex] = reserve - add;
		pm->ps->ammoclip[clipIndex] += add;
	}
	return add;
}

void PM_WeaponUseAmmo( int weapon, int amount ) {
	pm->ps->ammoclip[bg_weaponInfo[weapon]->clipIndex] -= amount;
}

int PM_WeaponAmmoAvailable( int weapon ) {
	return pm->ps->ammoclip[bg_weaponInfo[weapon]->clipIndex];
}

qboolean PM_WeaponClipEmpty( int weapon ) {
	return pm->ps->ammoclip[bg_weaponInfo[weapon]->clipIndex] == 0;
}

/*
=============================================================================

	WEAPON ANIMATION

=============================================================================
*/

void PM_StartWeaponAnim( int anim ) {
	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	if ( !pm->cmd.weapon ) {
		return;
	}
	pm->ps->weapAnim = ( ( ~pm->ps->weapAnim ) & ANIM_TOGGLEBIT ) | anim;
}

void PM_ContinueWeaponAnim( int anim ) {
	if ( !pm->cmd.weapon ) {
		return;
	}
	if ( ( pm->ps->weapAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	PM_StartWeaponAnim( anim );
}

/*
=============================================================================

	RECHAMBER

=============================================================================
*/

void PM_Weapon_FinishRechamber( void ) {
	PM_ContinueWeaponAnim( WEAP_IDLE );
	pm->ps->weaponstate = WEAPON_READY;
}

/*
==============
PM_Weapon_CheckForRechamber
==============
*/
qboolean PM_Weapon_CheckForRechamber( qboolean weaponTimeExpired ) {
	playerState_t *ps;
	int weapon;

	if ( !pml.weaponInfo->boltAction ) {
		return qfalse;
	}

	ps = pm->ps;
	weapon = ps->weapon;
	if ( !( ps->weaponrechamber[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
		return qfalse;
	}

	if ( ps->weaponstate == WEAPON_RECHAMBERING && weaponTimeExpired ) {
		ps->weaponrechamber[weapon >> 5] &= ~( 1 << ( weapon & 31 ) );
		PM_AddEvent( EV_EJECT_BRASS );
		ps = pm->ps;
		if ( ps->weaponTime ) {
			return qtrue;
		}
	} else if ( ps->weaponTime ) {
		if ( ps->weaponstate == WEAPON_FIRING
			 || ps->weaponstate == WEAPON_RECHAMBERING
			 || ps->weaponstate == WEAPON_MELEE_INIT
			 || ps->weaponstate == WEAPON_MELEE_FIRE
			 || ps->weaponDelay ) {
			return qfalse;
		}
	}

	if ( ps->weaponstate == WEAPON_RECHAMBERING ) {
		PM_Weapon_FinishRechamber();
		return qfalse;
	}

	if ( ps->weaponstate == WEAPON_READY ) {
		if ( ps->fWeaponPosFrac <= 0.75f ) {
			PM_StartWeaponAnim( WEAP_RECHAMBER );
		} else {
			PM_StartWeaponAnim( WEAP_ADS_RECHAMBER );
		}
		pm->ps->weaponstate = WEAPON_RECHAMBERING;
		pm->ps->weaponTime = pml.weaponInfo->rechamberTime;
		if ( pml.weaponInfo->rechamberBoltTime
			 && pml.weaponInfo->rechamberBoltTime < pml.weaponInfo->rechamberTime ) {
			pm->ps->weaponDelay = pml.weaponInfo->rechamberBoltTime;
		} else {
			pm->ps->weaponDelay = 1;
		}
		PM_AddEvent( EV_RECHAMBER_WEAPON );
	}
	return qfalse;
}

/*
=============================================================================

	RELOAD

=============================================================================
*/

/*
==============
PM_SetReloadDelay

0x20010160, no retail symbol: the tail PM_SetReloadingState and
PM_BeginWeaponReload both jump to.  Returns the delay it armed.
==============
*/
int PM_SetReloadDelay( void ) {
	playerState_t *ps;
	int delay;

	ps = pm->ps;
	if ( ps->weaponstate == WEAPON_RELOAD_START || ps->weaponstate == WEAPON_RELOAD_START_INTERUPT ) {
		delay = pml.weaponInfo->reloadStartAddTime;
		if ( delay && delay >= pml.weaponInfo->reloadStartTime ) {
			delay = pml.weaponInfo->reloadStartTime;
		}
	} else {
		if ( ps->ammoclip[bg_weaponInfo[ps->weapon]->clipIndex] || pml.weaponInfo->weaponType ) {
			delay = pml.weaponInfo->reloadTime;
		} else {
			delay = pml.weaponInfo->reloadEmptyTime;
		}
		if ( pml.weaponInfo->reloadAddTime && pml.weaponInfo->reloadAddTime < delay ) {
			delay = pml.weaponInfo->reloadAddTime;
		}
	}

	if ( pml.weaponInfo->boltAction
		 && ( ps->weaponrechamber[ps->weapon >> 5] & ( 1 << ( ps->weapon & 31 ) ) ) ) {
		if ( !delay ) {
			delay = ps->weaponTime;
		}
		if ( pml.weaponInfo->rechamberBoltTime >= delay || !pml.weaponInfo->rechamberBoltTime ) {
			ps->weaponDelay = 1;
			return 1;
		}
		delay = pml.weaponInfo->rechamberBoltTime;
	}
	if ( delay ) {
		ps->weaponDelay = delay;
	}
	return delay;
}

/*
==============
PM_SetReloadingState
==============
*/
void PM_SetReloadingState( void ) {
	playerState_t *ps;

	ps = pm->ps;
	if ( PM_WeaponAmmoAvailable( ps->weapon ) || pml.weaponInfo->weaponType ) {
		PM_StartWeaponAnim( WEAP_RELOAD );
		pm->ps->weaponTime = pml.weaponInfo->reloadTime;
		PM_AddEvent( EV_RELOAD );
	} else {
		PM_StartWeaponAnim( WEAP_RELOAD_EMPTY );
		pm->ps->weaponTime = pml.weaponInfo->reloadEmptyTime;
		PM_AddEvent( EV_RELOAD_FROM_EMPTY );
	}
	pm->ps->weaponstate = ( pm->ps->weaponstate == WEAPON_RELOAD_START_INTERUPT )
						  ? WEAPON_RELOADING_INTERUPT : WEAPON_RELOADING;
	PM_SetReloadDelay();
}

/*
==============
PM_BeginWeaponReload
==============
*/
void PM_BeginWeaponReload( void ) {
	playerState_t *ps;

	ps = pm->ps;
	if ( ps->weaponstate != WEAPON_READY && ps->weaponstate != WEAPON_FIRING
		 && ps->weaponstate != WEAPON_RECHAMBERING ) {
		return;
	}
	if ( !ps->weapon || (int)ps->weapon > bg_numWeapons ) {
		return;
	}

	if ( !bg_weaponInfo[ps->weapon]->clipOnly ) {
		BG_AnimScriptEvent( ps, ANIM_ET_RELOAD, qfalse, qtrue );
	}

	if ( !pml.weaponInfo->segmentedReload || !pml.weaponInfo->reloadStartTime ) {
		PM_SetReloadingState();
		return;
	}

	PM_StartWeaponAnim( WEAP_RELOAD_START );
	pm->ps->weaponTime = pml.weaponInfo->reloadStartTime;
	pm->ps->weaponstate = WEAPON_RELOAD_START;
	PM_AddEvent( EV_RELOAD_START );
	PM_SetReloadDelay();
}

/*
=============================================================================

	WEAPON CHANGE

=============================================================================
*/

/*
==============
PM_BeginWeaponChange
==============
*/
void PM_BeginWeaponChange( int oldWeapon, int newWeapon ) {
	const weaponInfo_t *weaponInfo;
	playerState_t *ps;
	qboolean isAltSwitch;
	qboolean playPutaway;
	int slot;

	if ( newWeapon < 0 || newWeapon > bg_numWeapons ) {
		return;
	}
	if ( newWeapon && !( pm->ps->weapons[newWeapon >> 5] & ( 1 << ( newWeapon & 31 ) ) ) ) {
		return;
	}
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		return;
	}

	pm->ps->weaponDelay = 0;

	ps = pm->ps;
	if ( !oldWeapon
		 || !( ps->weapons[oldWeapon >> 5] & ( 1 << ( oldWeapon & 31 ) ) )
		 || ps->grenadeTimeLeft > 0 ) {
		ps->weaponTime = 0;
		ps->weaponstate = WEAPON_DROPPING;
		ps->grenadeTimeLeft = 0;
		if ( ps->pm_flags & PMF_PRONE ) {
			ps->pm_flags |= PMF_PRONE_MOVEOVERRIDE;
		}
		return;
	}

	weaponInfo = bg_weaponInfo[oldWeapon];
	isAltSwitch = ( newWeapon && newWeapon == weaponInfo->altWeaponIndex ) ? qtrue : qfalse;

	playPutaway = qtrue;
	if ( weaponInfo->clipOnly ) {
		playPutaway = ps->ammoclip[weaponInfo->clipIndex] != 0;
	}
	ps->grenadeTimeLeft = 0;

	if ( isAltSwitch ) {
		PM_AddEvent( EV_WEAPON_ALT );
		PM_StartWeaponAnim( WEAP_ALTSWITCHFROM );
	} else {
		if ( playPutaway ) {
			PM_AddEvent( EV_PUTAWAY_WEAPON );
			PM_StartWeaponAnim( WEAP_DROP );
		}
		BG_AnimScriptEvent( pm->ps, ANIM_ET_DROPWEAPON, qfalse, qfalse );
	}

	pm->ps->weaponstate = WEAPON_DROPPING;
	PM_SetProneMovementOverride();

	if ( isAltSwitch ) {
		pm->ps->weaponTime = weaponInfo->altDropTime;
		ps = pm->ps;
		slot = BG_IsPlayerWeaponInSlot( ps, oldWeapon, qtrue );
		if ( slot ) {
			BG_SetPlayerWeaponForSlot( ps, slot, newWeapon );
		}
	} else {
		pm->ps->weaponTime = weaponInfo->dropTime;
	}
}

/*
==============
PM_Weapon_FinishWeaponChange
==============
*/
qboolean PM_Weapon_FinishWeaponChange( void ) {
	playerState_t *ps;
	int oldWeapon;
	int newWeapon;

	ps = pm->ps;
	if ( ps->weaponstate != WEAPON_DROPPING ) {
		return qfalse;
	}

	newWeapon = pm->cmd.weapon;
	if ( ( ps->pm_flags & PMF_LADDER )
		 || !( ps->weapons[newWeapon >> 5] & ( 1 << ( newWeapon & 31 ) ) )
		 || newWeapon > bg_numWeapons ) {
		newWeapon = 0;
	}
	if ( !( ps->weapons[newWeapon >> 5] & ( 1 << ( newWeapon & 31 ) ) ) ) {
		newWeapon = 0;
	}

	oldWeapon = ps->weapon;
	ps->weapon = (byte)newWeapon;
	pml.weaponInfo = bg_weaponInfo[pm->ps->weapon];

	if ( oldWeapon == newWeapon ) {
		pm->ps->weaponstate = WEAPON_READY;
		PM_StartWeaponAnim( WEAP_IDLE );
		return qtrue;
	}

	pm->ps->weaponstate = WEAPON_RAISING;

	if ( !oldWeapon ) {
		pm->ps->weaponTime = bg_weaponInfo[newWeapon]->raiseTime;
		pm->ps->aimSpreadScale = 255.0f;
		PM_StartWeaponAnim( WEAP_RAISE );
		ps = pm->ps;
		if ( ps->pm_flags & PMF_PRONE ) {
			ps->pm_flags |= PMF_PRONE_MOVEOVERRIDE;
		}
		return qtrue;
	}

	ps = pm->ps;
	if ( ps->pm_flags & PMF_PRONE ) {
		ps->pm_flags |= PMF_PRONE_MOVEOVERRIDE;
	}

	if ( newWeapon && newWeapon == bg_weaponInfo[oldWeapon]->altWeaponIndex ) {
		pm->ps->weaponTime = bg_weaponInfo[newWeapon]->altRaiseTime;
	} else {
		PM_AddEvent( EV_RAISE_WEAPON );
		pm->ps->weaponTime = bg_weaponInfo[newWeapon]->raiseTime;
	}

	BG_UpdateConditionValue( pm->ps->clientNum, ANIM_COND_WEAPONS, newWeapon, qtrue );
	BG_UpdateConditionValue( pm->ps->clientNum, ANIM_COND_WEAPONCLASS, bg_weaponInfo[newWeapon]->weaponClass, qtrue );

	if ( newWeapon && newWeapon == bg_weaponInfo[oldWeapon]->altWeaponIndex ) {
		ps = pm->ps;
		if ( ps->aimSpreadScale < 128.0f ) {
			ps->aimSpreadScale = 128.0f;
		}
		PM_StartWeaponAnim( WEAP_ALTSWITCHTO );
		return qtrue;
	}

	BG_AnimScriptEvent( pm->ps, ANIM_ET_RAISEWEAPON, qfalse, qfalse );
	pm->ps->aimSpreadScale = 255.0f;
	PM_StartWeaponAnim( WEAP_RAISE );
	return qtrue;
}

/*
==============
PM_Weapon_FinishWeaponRaise
==============
*/
qboolean PM_Weapon_FinishWeaponRaise( void ) {
	if ( pm->ps->weaponstate != WEAPON_RAISING ) {
		return qfalse;
	}
	pm->ps->weaponstate = WEAPON_READY;
	PM_StartWeaponAnim( WEAP_IDLE );
	return qtrue;
}

/*
==============
PM_Weapon_AllowReload
==============
*/
qboolean PM_Weapon_AllowReload( void ) {
	const weaponInfo_t *weaponInfo;
	int clipIndex;
	int inClip;
	int clipSize;

	weaponInfo = bg_weaponInfo[pm->ps->weapon];
	clipIndex = weaponInfo->clipIndex;
	if ( !pm->ps->ammo[weaponInfo->ammoIndex] ) {
		return qfalse;
	}

	inClip = pm->ps->ammoclip[clipIndex];
	clipSize = bg_ammoClipSizes[clipIndex];
	if ( inClip >= clipSize ) {
		return qfalse;
	}

	if ( !pml.weaponInfo->noPartialReload ) {
		return qtrue;
	}
	if ( pml.weaponInfo->reloadAmmoAdd && pml.weaponInfo->reloadAmmoAdd < clipSize ) {
		if ( clipSize - inClip >= pml.weaponInfo->reloadAmmoAdd ) {
			return qtrue;
		}
	} else if ( !inClip ) {
		return qtrue;
	}
	return qfalse;
}

/*
==============
PM_Weapon_ReloadDelayedAction

Move one reload increment out of the reserve and into the clip; boltAction
weapons eject their case and re-arm the delay first.
==============
*/
int PM_Weapon_ReloadDelayedAction( void ) {
	playerState_t *ps;
	int weapon;
	int cap;
	int delay;

	if ( pml.weaponInfo->boltAction ) {
		ps = pm->ps;
		weapon = ps->weapon;
		if ( ps->weaponrechamber[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) {
			ps->weaponrechamber[weapon >> 5] &= ~( 1 << ( weapon & 31 ) );
			PM_AddEvent( EV_EJECT_BRASS );

			ps = pm->ps;
			if ( ( ps->weaponstate == WEAPON_RELOAD_START || ps->weaponstate == WEAPON_RELOAD_START_INTERUPT )
				 && !pml.weaponInfo->reloadStartAddTime ) {
				return ps->weaponstate;
			}
			if ( ps->weaponTime ) {
				if ( ps->weaponstate == WEAPON_RELOAD_START || ps->weaponstate == WEAPON_RELOAD_START_INTERUPT ) {
					delay = pml.weaponInfo->reloadStartAddTime;
					if ( delay >= pml.weaponInfo->reloadStartTime ) {
						delay = pml.weaponInfo->reloadStartTime;
					}
				} else {
					if ( ps->ammoclip[bg_weaponInfo[ps->weapon]->clipIndex] || pml.weaponInfo->weaponType ) {
						delay = pml.weaponInfo->reloadTime;
					} else {
						delay = pml.weaponInfo->reloadEmptyTime;
					}
					if ( pml.weaponInfo->reloadAddTime && pml.weaponInfo->reloadAddTime < delay ) {
						delay = pml.weaponInfo->reloadAddTime;
					}
				}
				cap = pml.weaponInfo->rechamberBoltTime;
				if ( cap >= delay ) {
					cap = 1;
				}
				delay -= cap;
				if ( delay >= 1 ) {
					ps->weaponDelay = delay;
					return delay;
				}
			}
		}
	}

	return PM_Weapon_ReloadClipAmmo();
}

/*
==============
PM_Weapon_FinishReload
==============
*/
qboolean PM_Weapon_FinishReload( qboolean weaponTimeExpired ) {
	playerState_t *ps;
	int weapon;

	ps = pm->ps;
	if ( ps->weaponstate == WEAPON_RELOAD_START || ps->weaponstate == WEAPON_RELOAD_START_INTERUPT ) {
		if ( weaponTimeExpired ) {
			PM_Weapon_ReloadDelayedAction();
		}
		ps = pm->ps;
		if ( ps->weaponTime ) {
			return qtrue;
		}
		if ( ( ps->weaponstate != WEAPON_RELOAD_START_INTERUPT
			   || !ps->ammoclip[bg_weaponInfo[ps->weapon]->clipIndex] )
			 && PM_Weapon_AllowReload() ) {
			PM_SetReloadingState();
			return qtrue;
		}
		weapon = ps->weapon;
		ps->weaponrechamber[weapon >> 5] &= ~( 1 << ( weapon & 31 ) );
		if ( pml.weaponInfo->reloadEndTime ) {
			pm->ps->weaponstate = WEAPON_RELOAD_END;
			PM_StartWeaponAnim( WEAP_RELOAD_END );
			pm->ps->weaponTime = pml.weaponInfo->reloadEndTime;
			PM_AddEvent( EV_RELOAD_END );
			return qtrue;
		}
		pm->ps->weaponstate = WEAPON_READY;
		PM_StartWeaponAnim( WEAP_IDLE );
		return qfalse;
	}

	if ( ps->weaponstate == WEAPON_RELOAD_END ) {
		ps->weaponstate = WEAPON_READY;
		PM_StartWeaponAnim( WEAP_IDLE );
		return qfalse;
	}

	if ( ps->weaponstate != WEAPON_RELOADING && ps->weaponstate != WEAPON_RELOADING_INTERUPT ) {
		return qfalse;
	}

	if ( weaponTimeExpired ) {
		PM_Weapon_ReloadDelayedAction();
		if ( pm->ps->weaponTime ) {
			return qtrue;
		}
	} else if ( ps->weaponTime ) {
		return qfalse;
	}

	ps = pm->ps;
	weapon = ps->weapon;
	ps->weaponrechamber[weapon >> 5] &= ~( 1 << ( weapon & 31 ) );

	if ( pml.weaponInfo->segmentedReload ) {
		if ( pm->ps->weaponstate != WEAPON_RELOADING_INTERUPT && PM_Weapon_AllowReload() ) {
			PM_SetReloadingState();
			return qtrue;
		}
		if ( pml.weaponInfo->reloadEndTime ) {
			pm->ps->weaponstate = WEAPON_RELOAD_END;
			PM_StartWeaponAnim( WEAP_RELOAD_END );
			pm->ps->weaponTime = pml.weaponInfo->reloadEndTime;
			PM_AddEvent( EV_RELOAD_END );
			return qtrue;
		}
		pm->ps->weaponstate = WEAPON_READY;
	} else {
		pm->ps->weaponstate = WEAPON_READY;
	}
	PM_StartWeaponAnim( WEAP_IDLE );
	return weaponTimeExpired;
}

/*
==============
PM_Weapon_CheckForReload
==============
*/
void PM_Weapon_CheckForReload( void ) {
	const weaponInfo_t *weaponInfo;
	playerState_t *ps;
	qboolean requested;
	qboolean wantReload;

	requested = ( pm->cmd.wbuttons & WBUTTON_RELOAD ) ? qtrue : qfalse;
	wantReload = qfalse;

	if ( pml.weaponInfo->segmentedReload ) {
		ps = pm->ps;
		if ( ( ps->weaponstate == WEAPON_RELOAD_START || ps->weaponstate == WEAPON_RELOADING )
			 && ( pm->cmd.buttons & BUTTON_ATTACK ) && !( pm->oldcmd.buttons & BUTTON_ATTACK ) ) {
			if ( ps->weaponstate == WEAPON_RELOAD_START ) {
				ps->weaponstate = WEAPON_RELOAD_START_INTERUPT;
			} else {
				ps->weaponstate = WEAPON_RELOADING_INTERUPT;
			}
		}
	}

	ps = pm->ps;
	switch ( ps->weaponstate ) {
	case WEAPON_RAISING:
	case WEAPON_DROPPING:
	case WEAPON_RELOADING:
	case WEAPON_RELOADING_INTERUPT:
	case WEAPON_RELOAD_START:
	case WEAPON_RELOAD_START_INTERUPT:
	case WEAPON_RELOAD_END:
	case WEAPON_MELEE_INIT:
	case WEAPON_MELEE_FIRE:
		return;
	default:
		break;
	}

	weaponInfo = bg_weaponInfo[ps->weapon];
	if ( requested && PM_Weapon_AllowReload() ) {
		wantReload = qtrue;
	}

	if ( ( !ps->ammoclip[weaponInfo->clipIndex]
		   && ps->ammo[weaponInfo->ammoIndex]
		   && ps->weaponstate != WEAPON_FIRING
		   && ( !( ps->pm_flags & PMF_PRONE ) || ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) ) )
		 || wantReload ) {
		PM_BeginWeaponReload();
	}
}

/*
==============
PM_Weapon_CheckEmptyWeapon

0x20010EE0, no retail symbol: the tail PM_Weapon_FireWeapon jumps to.  Retail
reaches BG_TakePlayerWeapon here through an LTCG register convention (ecx, eax).
==============
*/
void PM_Weapon_CheckEmptyWeapon( void ) {
	const weaponInfo_t *weaponInfo;
	playerState_t *ps;

	ps = pm->ps;
	weaponInfo = bg_weaponInfo[ps->weapon];
	if ( weaponInfo->clipOnly && !ps->ammoclip[weaponInfo->clipIndex] && !ps->ammo[weaponInfo->ammoIndex] ) {
		BG_TakePlayerWeapon( ps, ps->weapon );
		PM_AddEvent( EV_NOAMMO );
	}
}

/*
=============================================================================

	AIM SPREAD

=============================================================================
*/

/*
==============
PM_AdjustAimSpreadScale
==============
*/
void PM_AdjustAimSpreadScale( void ) {
	playerState_t *ps;
	float decay;
	float increase;
	float invFrametime;
	int i;

	decay = pml.weaponInfo->hipSpreadDecayRate;
	if ( decay == 0.0f ) {
		decay = 1.0f;
		increase = 0.0f;
	} else {
		ps = pm->ps;
		if ( ps->groundEntityNum == ENTITYNUM_NONE && ps->pm_type != PM_NORMAL_LINKED ) {
			decay = decay * 0.5f;
		} else if ( ps->eFlags & EF_PRONE ) {
			decay = decay * pml.weaponInfo->hipSpreadProneDecay;
		} else if ( ps->eFlags & EF_CROUCHING ) {
			decay = decay * pml.weaponInfo->hipSpreadDuckedDecay;
		}
		decay = pml.frametime * decay;

		increase = 0.0f;
		if ( ps->fWeaponPosFrac != 1.0f ) {
			if ( pml.weaponInfo->hipSpreadTurnAdd != 0.0f ) {
				invFrametime = 1.0f / pml.frametime;
				for ( i = 0; i < 2; i++ ) {
					increase = increase
							   + (float)fabs( AngleSubtract( pm->cmd.angles[i] * 0.0054931641f,
															 pm->oldcmd.angles[i] * 0.0054931641f ) )
								 * pml.weaponInfo->hipSpreadTurnAdd * 0.01f * invFrametime;
				}
			}
			if ( pml.weaponInfo->hipSpreadMoveAdd != 0.0f && ( pm->cmd.forwardmove || pm->cmd.rightmove ) ) {
				increase = increase + pml.weaponInfo->hipSpreadMoveAdd;
			}
			if ( ps->groundEntityNum == ENTITYNUM_NONE && ps->pm_type != PM_NORMAL_LINKED ) {
				/* two separate adds of the same constant in retail */
				increase = increase + 1.28f;
				increase = increase + 1.28f;
			}
			increase = increase * pml.frametime;
		}
	}

	pm->ps->aimSpreadScale = pm->ps->aimSpreadScale + ( increase - decay ) * 255.0f;
	if ( pm->ps->aimSpreadScale < 0.0f ) {
		pm->ps->aimSpreadScale = 0.0f;
	} else if ( pm->ps->aimSpreadScale > 255.0f ) {
		pm->ps->aimSpreadScale = 255.0f;
	}
}

/*
=============================================================================

	WEAPON FRAME

=============================================================================
*/

/*
==============
PM_Weapon_WeaponTimeAdjust

Returns qtrue on the frame the weapon delay expires.
==============
*/
qboolean PM_Weapon_WeaponTimeAdjust( void ) {
	playerState_t *ps;

	ps = pm->ps;
	if ( ps->weaponTime ) {
		ps->weaponTime -= pml.msec;
		ps = pm->ps;
		if ( ps->weaponTime <= 0 ) {
			if ( pml.weaponInfo->semiAuto
				 && ( pm->cmd.buttons & BUTTON_ATTACK )
				 && ps->weapon == (unsigned int)pm->cmd.weapon
				 && PM_WeaponAmmoAvailable( ps->weapon ) ) {
				ps->weaponTime = 1;
				if ( pm->ps->weaponstate == WEAPON_RECHAMBERING ) {
					PM_Weapon_FinishRechamber();
				} else if ( pm->ps->weaponstate == WEAPON_FIRING
							|| pm->ps->weaponstate == WEAPON_MELEE_INIT
							|| pm->ps->weaponstate == WEAPON_MELEE_FIRE ) {
					PM_ContinueWeaponAnim( WEAP_IDLE );
					pm->ps->weaponstate = WEAPON_READY;
				}
			} else {
				ps->weaponTime = 0;
			}
		}
	}

	if ( !pm->ps->weaponDelay ) {
		return qfalse;
	}
	pm->ps->weaponDelay -= pml.msec;
	ps = pm->ps;
	if ( ps->weaponDelay > 0 ) {
		return qfalse;
	}
	ps->weaponDelay = 0;
	return qtrue;
}

/*
==============
PM_Weapon_CheckForChangeWeapon
==============
*/
void PM_Weapon_CheckForChangeWeapon( void ) {
	playerState_t *ps;
	int weapon;
	int requested;

	ps = pm->ps;
	if ( ps->weaponTime ) {
		switch ( ps->weaponstate ) {
		case WEAPON_RELOADING:
		case WEAPON_RELOADING_INTERUPT:
		case WEAPON_RELOAD_START:
		case WEAPON_RELOAD_START_INTERUPT:
		case WEAPON_RELOAD_END:
		case WEAPON_RECHAMBERING:
			break;
		case WEAPON_FIRING:
		case WEAPON_MELEE_INIT:
		case WEAPON_MELEE_FIRE:
			return;
		default:
			if ( ps->weaponDelay ) {
				return;
			}
			break;
		}
	}

	weapon = ps->weapon;
	if ( ps->pm_flags & PMF_LADDER ) {
		if ( weapon ) {
			PM_BeginWeaponChange( weapon, 0 );
		}
		return;
	}

	requested = (byte)pm->cmd.weapon;
	if ( weapon != requested ) {
		if ( !( ps->pm_flags & PMF_IGNORE_INPUT ) || !weapon ) {
			if ( !requested || ( ps->weapons[requested >> 5] & ( 1 << ( requested & 31 ) ) ) ) {
				PM_BeginWeaponChange( weapon, requested );
				return;
			}
		}
	}
	if ( !weapon ) {
		return;
	}
	if ( !( ps->weapons[weapon >> 5] & ( 1 << ( weapon & 31 ) ) ) ) {
		PM_BeginWeaponChange( weapon, 0 );
	}
}

/*
==============
PM_Weapon_FinishFiring
==============
*/
qboolean PM_Weapon_FinishFiring( qboolean weaponDelayExpired ) {
	if ( ( pm->cmd.buttons & BUTTON_ATTACK ) || weaponDelayExpired ) {
		return qfalse;
	}
	if ( pm->ps->weaponstate == WEAPON_FIRING ) {
		PM_ContinueWeaponAnim( WEAP_IDLE );
	}
	pm->ps->weaponstate = WEAPON_READY;
	return qtrue;
}

/*
==============
PM_Weapon_StartFiring
==============
*/
void PM_Weapon_StartFiring( qboolean weaponDelayExpired ) {
	playerState_t *ps;
	int weapon;

	if ( pml.weaponInfo->weaponType == WEAPTYPE_GRENADE ) {
		if ( !weaponDelayExpired ) {
			if ( PM_WeaponAmmoAvailable( pm->ps->weapon ) ) {
				pm->ps->grenadeTimeLeft = pml.weaponInfo->fuseTime;
				PM_StartWeaponAnim( 17 );   /* no recovered name; not in the debug table */
				PM_AddEvent( EV_PULLBACK_WEAPON );
			}
			pm->ps->weaponDelay = pml.weaponInfo->holdFireTime;
			pm->ps->weaponTime = 0;
		}
	} else {
		pm->ps->weaponDelay = pml.weaponInfo->fireDelay;
		pm->ps->weaponTime = pml.weaponInfo->fireTime;
		if ( pml.weaponInfo->adsFire ) {
			pm->ps->weaponDelay = (int)( ( 1.0f - pm->ps->fWeaponPosFrac )
										 * ( 1.0f / pml.weaponInfo->adsTransInRate ) );
		}

		BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );

		if ( pml.weaponInfo->boltAction ) {
			ps = pm->ps;
			weapon = ps->weapon;
			ps->weaponrechamber[weapon >> 5] |= 1 << ( weapon & 31 );
		}
	}

	pm->ps->weaponstate = WEAPON_FIRING;
	ps = pm->ps;
	if ( ps->pm_flags & PMF_PRONE ) {
		ps->pm_flags |= PMF_PRONE_MOVEOVERRIDE;
	}
}

int PM_Weapon_GetAmmoRequired( void ) {
	return 1;
}

/*
==============
PM_Weapon_CheckFiringAmmo

qfalse means the shot did not happen: either a reload was started or the dry
click was played.
==============
*/
qboolean PM_Weapon_CheckFiringAmmo( void ) {
	const weaponInfo_t *weaponInfo;
	playerState_t *ps;
	qboolean hasReserve;

	ps = pm->ps;
	weaponInfo = bg_weaponInfo[ps->weapon];
	if ( ps->ammoclip[weaponInfo->clipIndex] >= 1 ) {
		return qtrue;
	}

	hasReserve = ps->ammo[weaponInfo->ammoIndex] >= 1;
	if ( pml.weaponInfo->weaponType != WEAPTYPE_GRENADE ) {
		if ( hasReserve ) {
			PM_BeginWeaponReload();
			return qfalse;
		}
		PM_AddEvent( EV_NOAMMO );
	} else if ( hasReserve ) {
		PM_BeginWeaponReload();
		return qfalse;
	}

	PM_ContinueWeaponAnim( WEAP_IDLE );
	pm->ps->weaponTime += 500;
	return qfalse;
}

/*
==============
PM_Weapon_SetFPSFireAnim
==============
*/
int PM_Weapon_SetFPSFireAnim( void ) {
	playerState_t *ps;
	int anim;

	ps = pm->ps;
	if ( ps->fWeaponPosFrac <= 0.75f ) {
		anim = ps->ammoclip[bg_weaponInfo[ps->weapon]->clipIndex] ? WEAP_ATTACK : WEAP_ATTACK_LASTSHOT;
	} else {
		anim = ps->ammoclip[bg_weaponInfo[ps->weapon]->clipIndex] ? WEAP_ADS_ATTACK : WEAP_ADS_ATTACK_LASTSHOT;
	}
	PM_StartWeaponAnim( anim );
	return anim;
}

/*
==============
PM_Weapon_AddFiringAimSpreadScale
==============
*/
void PM_Weapon_AddFiringAimSpreadScale( void ) {
	playerState_t *ps;

	ps = pm->ps;
	if ( ps->fWeaponPosFrac == 1.0f ) {
		return;
	}
	ps->aimSpreadScale = pml.weaponInfo->hipSpreadFireAdd * 255.0f + ps->aimSpreadScale;
	ps = pm->ps;
	if ( ps->aimSpreadScale > 255.0f ) {
		ps->aimSpreadScale = 255.0f;
	}
}

/*
==============
PM_Weapon_FireWeapon
==============
*/
void PM_Weapon_FireWeapon( qboolean weaponDelayExpired ) {
	playerState_t *ps;
	int clipIndex;

	PM_Weapon_StartFiring( weaponDelayExpired );
	if ( !PM_Weapon_CheckFiringAmmo() ) {
		return;
	}

	ps = pm->ps;
	if ( ps->weaponDelay ) {
		return;
	}

	clipIndex = bg_weaponInfo[ps->weapon]->clipIndex;
	if ( ps->ammoclip[clipIndex] != -1 && !( ps->eFlags & EF_MOUNTED ) ) {
		ps->ammoclip[clipIndex]--;
	}
	if ( pml.weaponInfo->weaponType == WEAPTYPE_GRENADE ) {
		pm->ps->weaponTime = pml.weaponInfo->fireTime;
	}

	PM_Weapon_SetFPSFireAnim();

	ps = pm->ps;
	if ( ps->ammoclip[bg_weaponInfo[ps->weapon]->clipIndex] ) {
		PM_AddEvent( EV_FIRE_WEAPON );
	} else {
		PM_AddEvent( EV_FIRE_WEAPON_LASTSHOT );
	}

	PM_Weapon_AddFiringAimSpreadScale();

	PM_Weapon_CheckEmptyWeapon();
}

/*
=============================================================================

	MELEE

=============================================================================
*/

void PM_Weapon_FireMelee( void ) {
	playerState_t *ps;
	int time;

	time = pml.weaponInfo->meleeTime - pml.weaponInfo->meleeDelay;
	if ( pm->ps->weaponTime < time ) {
		pm->ps->weaponTime = time;
	}
	PM_AddEvent( EV_FIRE_MELEE );
	pm->ps->weaponstate = WEAPON_MELEE_FIRE;
	ps = pm->ps;
	if ( ps->pm_flags & PMF_PRONE ) {
		ps->pm_flags |= PMF_PRONE_MOVEOVERRIDE;
	}
}

qboolean PM_Weapon_FinishMelee( void ) {
	if ( pm->ps->weaponstate == WEAPON_MELEE_INIT ) {
		PM_Weapon_FireMelee();
		return qtrue;
	}
	if ( pm->ps->weaponstate == WEAPON_MELEE_FIRE ) {
		PM_ContinueWeaponAnim( WEAP_IDLE );
		pm->ps->weaponstate = WEAPON_READY;
		return qtrue;
	}
	return qfalse;
}

/*
==============
PM_Weapon_CheckForMelee
==============
*/
void PM_Weapon_CheckForMelee( qboolean weaponDelayExpired ) {
	playerState_t *ps;

	if ( !pml.weaponInfo->meleeDamage ) {
		return;
	}
	if ( weaponDelayExpired ) {
		return;
	}

	ps = pm->ps;
	if ( ps->weaponDelay ) {
		switch ( ps->weaponstate ) {
		case WEAPON_RELOADING:
		case WEAPON_RELOADING_INTERUPT:
		case WEAPON_RELOAD_START:
		case WEAPON_RELOAD_START_INTERUPT:
		case WEAPON_RELOAD_END:
			break;
		default:
			return;
		}
	}

	if ( !( pm->cmd.buttons & BUTTON_MELEE ) ) {
		ps->pm_flags &= ~PMF_MELEE_LATCH;
		return;
	}
	if ( ps->pm_flags & PMF_MELEE_LATCH ) {
		return;
	}
	ps->pm_flags |= PMF_MELEE_LATCH;

	switch ( pm->ps->weaponstate ) {
	case WEAPON_RAISING:
	case WEAPON_DROPPING:
	case WEAPON_MELEE_INIT:
	case WEAPON_MELEE_FIRE:
		return;
	default:
		break;
	}

	BG_AnimScriptEvent( pm->ps, ANIM_ET_MELEEATTACK, qfalse, qtrue );
	PM_StartWeaponAnim( WEAP_MELEE_ATTACK );
	PM_AddEvent( EV_MELEE_SWIPE );

	if ( pml.weaponInfo->meleeDelay ) {
		pm->ps->weaponTime = pml.weaponInfo->meleeTime;
		pm->ps->weaponDelay = pml.weaponInfo->meleeDelay;
		pm->ps->weaponstate = WEAPON_MELEE_INIT;
		PM_SetProneMovementOverride();
	} else {
		PM_Weapon_FireMelee();
	}
}

/*
==============
PM_Weapon

Generates weapon events and modifies the weapon counter.
==============
*/
void PM_Weapon( void ) {
	playerState_t *ps;
	qboolean weaponDelayExpired;

	ps = pm->ps;
	if ( ps->pm_flags & PMF_RESPAWNED ) {
		return;
	}
	if ( ps->pm_type >= PM_DEAD ) {
		ps->weapon = 0;
		return;
	}
	if ( ps->eFlags & EF_MOUNTED ) {
		return;
	}

	if ( bg_debugWeaponState.integer && bg_debugWeaponState.integer != 2 ) {
		PM_Weapon_PrintWeaponState();
	}
	if ( bg_debugWeaponAnim.integer && bg_debugWeaponAnim.integer != 2 ) {
		PM_Weapon_PrintWeaponAnim();
	}

	PM_UpdateAimDownSightLerp();

	if ( pml.weaponInfo->weaponType == WEAPTYPE_GRENADE && pm->ps->grenadeTimeLeft > 0 ) {
		if ( pml.weaponInfo->cookOffHold ) {
			pm->ps->grenadeTimeLeft -= pml.msec;
		}
		ps = pm->ps;
		if ( ps->grenadeTimeLeft <= 50 ) {
			ps->grenadeTimeLeft = 50;
			PM_AddEvent( EV_FIRE_WEAPON );
			pm->ps->weaponTime = 1600;
			return;
		}
		if ( ( pm->cmd.buttons & BUTTON_ATTACK ) && ps->weaponDelay - pml.msec <= 0 ) {
			ps->weaponDelay = pml.msec + 1;
		}
		if ( !( pm->cmd.buttons & BUTTON_ATTACK ) && pm->ps->weaponDelay - pml.msec <= 0 ) {
			BG_AnimScriptEvent( pm->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
		}
	}

	weaponDelayExpired = PM_Weapon_WeaponTimeAdjust();
	PM_Weapon_CheckForChangeWeapon();
	PM_Weapon_CheckForReload();
	PM_Weapon_CheckForMelee( weaponDelayExpired );
	if ( PM_Weapon_CheckForRechamber( weaponDelayExpired ) ) {
		return;
	}

	ps = pm->ps;
	if ( ( ( ps->pm_flags & PMF_PRONE ) && ( pm->cmd.forwardmove || pm->cmd.rightmove ) )
		 || ps->weaponstate == WEAPON_MELEE_INIT
		 || ps->weaponstate == WEAPON_MELEE_FIRE ) {
		ps->aimSpreadScale = 255.0f;
	}

	if ( !weaponDelayExpired && ( pm->ps->weaponTime || pm->ps->weaponDelay ) ) {
		return;
	}
	if ( PM_Weapon_FinishReload( weaponDelayExpired ) ) {
		return;
	}
	if ( PM_Weapon_FinishMelee() ) {
		return;
	}
	if ( PM_Weapon_FinishWeaponChange() ) {
		return;
	}
	if ( PM_Weapon_FinishWeaponRaise() ) {
		return;
	}
	if ( !pm->ps->weapon ) {
		return;
	}
	if ( PM_Weapon_FinishFiring( weaponDelayExpired ) ) {
		return;
	}
	PM_Weapon_FireWeapon( weaponDelayExpired );
}

/*
=============================================================================

	WEAPON DEBUG TRACERS

=============================================================================
*/

void PM_Weapon_PrintWeaponState( void ) {
	static int lastWeaponState = -1;
	int weaponState;

	weaponState = pm->ps->weaponstate;
	if ( lastWeaponState == weaponState ) {
		return;
	}
	Com_Printf( " %i %s_", pm->cmd.serverTime, bg_szDebugModule );
	lastWeaponState = weaponState;
	Com_Printf( "WEAP_STATE -- " );
	switch ( weaponState ) {
	case WEAPON_READY:                 Com_Printf( "WEAPON_READY\n" ); break;
	case WEAPON_RAISING:               Com_Printf( "WEAPON_RAISING\n" ); break;
	case WEAPON_DROPPING:              Com_Printf( "WEAPON_DROPPING\n" ); break;
	case WEAPON_FIRING:                Com_Printf( "WEAPON_FIRING\n" ); break;
	case WEAPON_RECHAMBERING:          Com_Printf( "WEAPON_RECHAMBERING\n" ); break;
	case WEAPON_RELOADING:             Com_Printf( "WEAPON_RELOADING\n" ); break;
	case WEAPON_RELOADING_INTERUPT:    Com_Printf( "WEAPON_RELOADING_INTERUPT\n" ); break;
	case WEAPON_RELOAD_START:          Com_Printf( "WEAPON_RELOAD_START\n" ); break;
	case WEAPON_RELOAD_START_INTERUPT: Com_Printf( "WEAPON_RELOAD_START_INTERUPT\n" ); break;
	case WEAPON_RELOAD_END:            Com_Printf( "WEAPON_RELOAD_END\n" ); break;
	case WEAPON_MELEE_INIT:            Com_Printf( "WEAPON_MELEE_WINDUP\n" ); break;
	case WEAPON_MELEE_FIRE:            Com_Printf( "WEAPON_MELEE_RELAX\n" ); break;
	default:                           Com_Printf( "UNKNOWN\n" ); break;
	}
}

void PM_Weapon_PrintWeaponAnim( void ) {
	static int lastWeaponAnim = -1;
	int weaponAnim;

	weaponAnim = pm->ps->weapAnim & ~ANIM_TOGGLEBIT;
	Com_Printf( " %i %s_", pm->cmd.serverTime, bg_szDebugModule );
	lastWeaponAnim = weaponAnim;
	Com_Printf( "WEAP_ANIM -- " );
	switch ( weaponAnim ) {
	case WEAP_IDLE:                Com_Printf( "WEAP_IDLE\n" ); break;
	case WEAP_ATTACK:              Com_Printf( "WEAP_ATTACK\n" ); break;
	case WEAP_ATTACK_LASTSHOT:     Com_Printf( "WEAP_ATTACK_LASTSHOT\n" ); break;
	case WEAP_RECHAMBER:           Com_Printf( "WEAP_RECHAMBER\n" ); break;
	case WEAP_ADS_ATTACK:          Com_Printf( "WEAP_ADS_ATTACK\n" ); break;
	case WEAP_ADS_ATTACK_LASTSHOT: Com_Printf( "WEAP_ADS_ATTACK_LASTSHOT\n" ); break;
	case WEAP_ADS_RECHAMBER:       Com_Printf( "WEAP_ADS_RECHAMBER\n" ); break;
	case WEAP_MELEE_ATTACK:        Com_Printf( "WEAP_MELEE_ATTACK\n" ); break;
	case WEAP_DROP:                Com_Printf( "WEAP_DROP\n" ); break;
	case WEAP_RAISE:               Com_Printf( "WEAP_RAISE\n" ); break;
	case WEAP_RELOAD:              Com_Printf( "WEAP_RELOAD\n" ); break;
	case WEAP_RELOAD_EMPTY:        Com_Printf( "WEAP_RELOAD_EMPTY\n" ); break;
	case WEAP_RELOAD_START:        Com_Printf( "WEAP_RELOAD_START\n" ); break;
	case WEAP_RELOAD_END:          Com_Printf( "WEAP_RELOAD_END\n" ); break;
	case WEAP_ALTSWITCHFROM:       Com_Printf( "WEAP_ALTSWITCHFROM\n" ); break;
	case WEAP_ALTSWITCHTO:         Com_Printf( "WEAP_ALTSWITCHTO\n" ); break;
	default:                       Com_Printf( "UNKNOWN\n" ); break;
	}
}

/*
=============================================================================

	FIRST PERSON WEAPON / VIEW ANGLES

	The bob phase both bob helpers are fed is
	( ps->bobCycle & 255 ) * ( 2*M_PI / 255 ) + 2*M_PI, with an extra 2.25*M_PI
	for the weapon-position pass.

=============================================================================
*/

/*
==============
BG_GetVerticalBobFactor
==============
*/
float BG_GetVerticalBobFactor( const playerState_t *ps, float phase, float amplitude, float maxAmplitude ) {
	float scaled;

	if ( ps->viewHeightTarget == ps->proneViewHeight ) {
		scaled = amplitude * 0.029999999f;
	} else if ( ps->viewHeightTarget == ps->crouchViewHeight ) {
		scaled = amplitude * 0.0074999998f;
	} else {
		scaled = amplitude * 0.0070000002f;
	}
	if ( scaled > maxAmplitude ) {
		scaled = maxAmplitude;
	}
	return (float)( ( sin( phase * 4.0f + 1.5707964f ) * 0.2f + sin( phase + phase ) ) * scaled * 0.75f );
}

/*
==============
BG_GetHorizontalBobFactor
==============
*/
float BG_GetHorizontalBobFactor( const playerState_t *ps, float phase, float amplitude, float maxAmplitude ) {
	float scaled;

	if ( ps->viewHeightTarget == ps->proneViewHeight ) {
		scaled = amplitude * 0.029999999f;
	} else if ( ps->viewHeightTarget == ps->crouchViewHeight ) {
		scaled = amplitude * 0.0074999998f;
	} else {
		scaled = amplitude * 0.0070000002f;
	}
	if ( scaled > maxAmplitude ) {
		scaled = maxAmplitude;
	}
	return (float)( scaled * sin( phase ) );
}

/*
==============
BG_CalculateWeaponPosition_BasePosition_angles

Ease state->moveOffset towards the stance rotation the current speed asks for,
then add the hip-fire share of it to the running weapon angles.  Each axis is
forced to move at least 0.1 * frametime per frame and snaps on overshoot.
==============
*/
void BG_CalculateWeaponPosition_BasePosition_angles( pmWeaponAngleState_t *state, vec3_t angles ) {
	const weaponInfo_t *weaponInfo;
	const playerState_t *ps;
	float target[3];
	float minSpeed;
	float frac;
	float rate;
	float delta;
	float limit;
	int eFlags;
	int i;

	ps = state->ps;
	eFlags = ps->eFlags;
	weaponInfo = bg_weaponInfo[ps->weapon];

	if ( eFlags & EF_PRONE ) {
		minSpeed = weaponInfo->proneRotMinSpeed;
	} else if ( eFlags & EF_CROUCHING ) {
		minSpeed = weaponInfo->duckedRotMinSpeed;
	} else {
		minSpeed = weaponInfo->standRotMinSpeed;
	}

	if ( minSpeed >= state->speed || ps->weaponstate == WEAPON_RELOADING ) {
		target[0] = 0.0f;
		target[1] = 0.0f;
		target[2] = 0.0f;
	} else {
		frac = (float)( ( state->speed - minSpeed ) / ( (double)ps->speed - minSpeed ) );
		if ( frac < 0.0f ) {
			frac = 0.0f;
		} else if ( frac > 1.0f ) {
			frac = 1.0f;
		}
		if ( ps->eFlags & EF_PRONE ) {
			target[0] = frac * weaponInfo->proneRotP;
			target[1] = frac * weaponInfo->proneRotY;
			target[2] = frac * weaponInfo->proneRotR;
		} else if ( eFlags & EF_CROUCHING ) {
			target[0] = frac * weaponInfo->duckedRotP;
			target[1] = frac * weaponInfo->duckedRotY;
			target[2] = frac * weaponInfo->duckedRotR;
		} else {
			target[0] = frac * weaponInfo->standRotP;
			target[1] = frac * weaponInfo->standRotY;
			target[2] = frac * weaponInfo->standRotR;
		}
	}

	if ( ps->fWeaponPosFrac != 0.0f ) {
		float hipFrac = 1.0f - ps->fWeaponPosFrac;
		target[0] = target[0] * hipFrac;
		target[1] = target[1] * hipFrac;
		target[2] = target[2] * hipFrac;
	}

	for ( i = 0; i < 3; i++ ) {
		if ( state->moveOffset[i] == target[i] ) {
			continue;
		}
		if ( (float)ps->proneViewHeight == ps->viewHeightCurrent ) {
			rate = weaponInfo->posProneRotRate;
		} else {
			rate = weaponInfo->posRotRate;
		}
		delta = ( target[i] - state->moveOffset[i] ) * rate * state->frametime;
		if ( state->moveOffset[i] < target[i] ) {
			limit = state->frametime * 0.1f;
			if ( delta < limit ) {
				delta = limit;
			}
			state->moveOffset[i] = state->moveOffset[i] + delta;
			if ( state->moveOffset[i] > target[i] ) {
				state->moveOffset[i] = target[i];
			}
		} else {
			limit = state->frametime * -0.1f;
			if ( delta > limit ) {
				delta = limit;
			}
			state->moveOffset[i] = state->moveOffset[i] + delta;
			if ( state->moveOffset[i] < target[i] ) {
				state->moveOffset[i] = target[i];
			}
		}
	}

	if ( ps->fWeaponPosFrac == 0.0f ) {
		angles[0] = state->moveOffset[0] + angles[0];
		angles[1] = state->moveOffset[1] + angles[1];
		angles[2] = state->moveOffset[2] + angles[2];
	} else if ( ps->fWeaponPosFrac < 0.5f ) {
		float fade = 1.0f - ( ps->fWeaponPosFrac + ps->fWeaponPosFrac );
		angles[0] = fade * state->moveOffset[0] + angles[0];
		angles[1] = fade * state->moveOffset[1] + angles[1];
		angles[2] = fade * state->moveOffset[2] + angles[2];
	}
}

/*
==============
BG_CalculateWeaponPosition_BaseAngles
==============
*/
void BG_CalculateWeaponPosition_BaseAngles( pmWeaponAngleState_t *state, vec3_t angles ) {
	const weaponInfo_t *weaponInfo;

	weaponInfo = bg_weaponInfo[state->ps->weapon];
	if ( weaponInfo->aimDownSight ) {
		angles[0] = weaponInfo->adsAimPitch * state->ps->fWeaponPosFrac + angles[0];
	}
	BG_CalculateWeaponPosition_BasePosition_angles( state, angles );
}

/*
==============
BG_CalculateWeaponPosition_IdleAngles

The idle wander; its amount lerps from the hip figure to the ADS figure and is
scaled again by the stance factor.
==============
*/
void BG_CalculateWeaponPosition_IdleAngles( pmWeaponAngleState_t *state, vec3_t angles ) {
	const weaponInfo_t *weaponInfo;
	const playerState_t *ps;
	float amount;
	float stanceScale;
	float step;
	float idle;

	ps = state->ps;
	weaponInfo = bg_weaponInfo[ps->weapon];

	if ( weaponInfo->aimDownSight ) {
		amount = ( weaponInfo->adsIdleAmount - weaponInfo->hipIdleAmount ) * ps->fWeaponPosFrac
				 + weaponInfo->hipIdleAmount;
	} else if ( weaponInfo->hipIdleAmount == 0.0f ) {
		amount = 80.0f;
	} else {
		amount = weaponInfo->hipIdleAmount;
	}

	if ( ps->eFlags & EF_PRONE ) {
		stanceScale = weaponInfo->idleProneFactor;
	} else if ( ps->eFlags & EF_CROUCHING ) {
		stanceScale = weaponInfo->idleCrouchFactor;
	} else {
		stanceScale = 1.0f;
	}

	if ( stanceScale != state->idleScale ) {
		step = state->frametime * 0.5f;
		if ( stanceScale < state->idleScale ) {
			state->idleScale = state->idleScale - step;
			if ( state->idleScale < stanceScale ) {
				state->idleScale = stanceScale;
			}
		} else {
			state->idleScale = state->idleScale + step;
			if ( state->idleScale > stanceScale ) {
				state->idleScale = stanceScale;
			}
		}
	}

	idle = amount * state->idleScale;
	angles[2] = (float)( sin( state->time * 0.0005f ) * idle * 0.04f + angles[2] );
	angles[1] = (float)( sin( state->time * 0.0007f ) * idle * 0.01f + angles[1] );
	angles[0] = (float)( sin( state->time * 0.001f ) * idle * 0.01f + angles[0] );
}

/*
==============
BG_CalculateWeaponPosition_BobOffset

The roll term is retail's min( horizontal bob, 0 ): only the negative half of
the cycle rolls the weapon.
==============
*/
void BG_CalculateWeaponPosition_BobOffset( pmWeaponAngleState_t *state, vec3_t angles ) {
	const weaponInfo_t *weaponInfo;
	const playerState_t *ps;
	float phase;
	float amplitude;
	float pitch;
	float yaw;
	float roll;
	float adsScale;

	ps = state->ps;
	weaponInfo = bg_weaponInfo[ps->weapon];

	phase = (byte)ps->bobCycle * 0.024639944f + 6.2831855f + 7.0685835f;
	amplitude = state->speed * 0.16f;

	pitch = BG_GetVerticalBobFactor( ps, phase, amplitude, 10.0f ) * -1.0f;
	yaw = BG_GetHorizontalBobFactor( ps, phase, amplitude, 10.0f ) * -1.0f;
	roll = BG_GetHorizontalBobFactor( ps, phase - 0.47123894f, amplitude * 1.5f, 10.0f );
	if ( roll >= 0.0f ) {
		roll = 0.0f;
	}

	if ( ps->fWeaponPosFrac != 0.0f ) {
		adsScale = 1.0f - ps->fWeaponPosFrac * ( 1.0f - weaponInfo->adsBobFactor );
		pitch = pitch * adsScale;
		yaw = yaw * adsScale;
		roll = roll * adsScale;
	}

	angles[0] = pitch + angles[0];
	angles[1] = yaw + angles[1];
	angles[2] = roll + angles[2];
}

/*
==============
BG_CalculateWeaponPosition_DamageKick

100 ms in, 400 ms out, both eased; ADS keeps more of the kick on scoped
weapons.
==============
*/
void BG_CalculateWeaponPosition_DamageKick( pmWeaponAngleState_t *state, vec3_t angles ) {
	const playerState_t *ps;
	float scale;
	float inTime;
	float outTime;
	float elapsed;
	float ease;
	float kick;

	if ( !state->viewKickStartTime ) {
		return;
	}

	ps = state->ps;
	scale = ( ps->fWeaponPosFrac + 1.0f ) * 0.5f;
	inTime = 100.0f * scale;
	outTime = scale * 400.0f;
	if ( ps->fWeaponPosFrac != 0.0f ) {
		if ( bg_weaponInfo[ps->weapon]->adsOverlayReticle ) {
			scale = scale * ( 1.0f - ps->fWeaponPosFrac * 0.75f );
		}
	}

	elapsed = (float)( state->time - state->viewKickStartTime );
	if ( elapsed >= inTime ) {
		ease = 1.0f - ( elapsed - inTime ) / outTime;
		if ( ease <= 0.0f ) {
			return;
		}
		ease = 1.0f - ease;
		ease = 1.0f - ( 2.0f - (float)fabs( ease ) ) * ease;
	} else {
		ease = elapsed / inTime;
		ease = ( 2.0f - (float)fabs( ease ) ) * ease;
	}

	kick = scale * ease;
	angles[0] = kick * state->viewKickPitch * 0.5f + angles[0];
	angles[1] = angles[1] - kick * state->viewKickYaw;
	angles[2] = kick * state->viewKickYaw * 0.5f + angles[2];
}

/*
==============
BG_CalculateWeaponPosition_GunRecoil_SingleAngle

One axis of the gun kick spring.  Returns qtrue once the axis has settled.
==============
*/
qboolean BG_CalculateWeaponPosition_GunRecoil_SingleAngle( float *speed, float *angle, float frametime,
														   float maxAngle, float accel, float maxSpeed,
														   float speedDecay, float staticDecay ) {
	if ( fabs( *angle ) < 0.25f && fabs( *speed ) < 1.0f ) {
		*angle = 0.0f;
		*speed = 0.0f;
		return qtrue;
	}

	*angle = frametime * *speed + *angle;
	if ( *angle > maxAngle ) {
		*angle = maxAngle;
		if ( *speed > 0.0f ) {
			*speed = 0.0f;
		}
	} else if ( *angle < -maxAngle ) {
		*angle = -maxAngle;
		if ( *speed < 0.0f ) {
			*speed = 0.0f;
		}
	}

	if ( *angle > 0.0f ) {
		*speed = *speed - frametime * accel;
	} else if ( *angle < 0.0f ) {
		*speed = frametime * accel + *speed;
	}

	*speed = *speed - frametime * *speed * speedDecay;
	if ( *speed > 0.0f ) {
		*speed = *speed - frametime * staticDecay;
		if ( *speed < 0.0f ) {
			*speed = 0.0f;
		}
	} else {
		*speed = *speed + frametime * staticDecay;
		if ( *speed > 0.0f ) {
			*speed = 0.0f;
		}
	}

	if ( *speed > maxSpeed ) {
		*speed = maxSpeed;
	} else if ( -maxSpeed > *speed ) {
		*speed = -maxSpeed;
	}
	return qfalse;
}

/*
==============
BG_CalculateWeaponPosition_GunRecoil

Integrated in 5 ms steps.  Weapons without aimDownSight get no gun recoil at
all -- the whole body is inside that test in retail.
==============
*/
void BG_CalculateWeaponPosition_GunRecoil( pmWeaponAngleState_t *state, vec3_t angles ) {
	const weaponInfo_t *weaponInfo;
	float adsFrac;
	float remaining;
	float step;
	float accel;
	float maxSpeed;
	float speedDecay;
	float staticDecay;
	qboolean donePitch;
	qboolean doneYaw;

	weaponInfo = bg_weaponInfo[state->ps->weapon];
	if ( !weaponInfo->aimDownSight ) {
		return;
	}

	adsFrac = state->ps->fWeaponPosFrac;
	remaining = state->frametime;
	speedDecay = ( weaponInfo->adsGunKickSpeedDecay - weaponInfo->hipGunKickSpeedDecay ) * adsFrac
				 + weaponInfo->hipGunKickSpeedDecay;
	staticDecay = ( weaponInfo->adsGunKickStaticDecay - weaponInfo->hipGunKickStaticDecay ) * adsFrac
				  + weaponInfo->hipGunKickStaticDecay;

	if ( remaining > 0.0f ) {
		accel = ( weaponInfo->adsGunKickAccel - weaponInfo->hipGunKickAccel ) * adsFrac
				+ weaponInfo->hipGunKickAccel;
		maxSpeed = ( weaponInfo->adsGunKickSpeedMax - weaponInfo->hipGunKickSpeedMax ) * adsFrac
				   + weaponInfo->hipGunKickSpeedMax;
		while ( 1 ) {
			if ( remaining > 0.005f ) {
				step = 0.0049999999f;
				remaining = remaining - 0.005f;
			} else {
				step = remaining;
				remaining = 0.0f;
			}
			donePitch = BG_CalculateWeaponPosition_GunRecoil_SingleAngle( &state->recoilPitchSpeed, &state->recoilPitch,
																		  step, weaponInfo->gunMaxPitch, accel,
																		  maxSpeed, speedDecay, staticDecay );
			doneYaw = BG_CalculateWeaponPosition_GunRecoil_SingleAngle( &state->recoilYawSpeed, &state->recoilYaw,
																		step, weaponInfo->gunMaxYaw, accel,
																		maxSpeed, speedDecay, staticDecay );
			if ( doneYaw && donePitch ) {
				break;
			}
			if ( remaining <= 0.0f ) {
				break;
			}
		}
	}

	angles[0] = state->recoilPitch + angles[0];
	angles[1] = state->recoilYaw + angles[1];
	angles[2] = state->recoilRoll + angles[2];
}

/*
==============
BG_CalculateWeaponAngles
==============
*/
void BG_CalculateWeaponAngles( pmWeaponAngleState_t *state, vec3_t angles ) {
	const weaponInfo_t *weaponInfo;
	const playerState_t *ps;

	ps = state->ps;
	angles[0] = 0.0f;
	angles[1] = 0.0f;
	angles[2] = 0.0f;

	if ( ps->leanf != 0.0f ) {
		angles[2] = ( 2.0f - (float)fabs( ps->leanf ) ) * ps->leanf * -2.0f;
	}

	weaponInfo = bg_weaponInfo[ps->weapon];
	if ( weaponInfo->aimDownSight ) {
		angles[0] = weaponInfo->adsAimPitch * ps->fWeaponPosFrac;
	}

	BG_CalculateWeaponPosition_BasePosition_angles( state, angles );
	BG_CalculateWeaponPosition_IdleAngles( state, angles );
	BG_CalculateWeaponPosition_BobOffset( state, angles );
	BG_CalculateWeaponPosition_DamageKick( state, angles );
	BG_CalculateWeaponPosition_GunRecoil( state, angles );

	angles[0] = AngleSubtract( angles[0], state->baseAngles[0] );
	angles[1] = AngleSubtract( angles[1], state->baseAngles[1] );
}

/*
==============
BG_CalculateView_DamageKick
==============
*/
void BG_CalculateView_DamageKick( bgViewAngleState_t *state, vec3_t angles ) {
	const playerState_t *ps;
	float scale;
	float half;
	float elapsed;
	float ease;
	float kick;

	if ( !state->viewKickStartTime ) {
		return;
	}

	ps = state->ps;
	half = ps->fWeaponPosFrac * 0.5f;
	scale = 1.0f - half;
	if ( ps->fWeaponPosFrac != 0.0f ) {
		if ( bg_weaponInfo[ps->weapon]->adsOverlayReticle ) {
			scale = scale * ( half + 1.0f );
		}
	}

	elapsed = (float)( state->time - state->viewKickStartTime );
	if ( elapsed >= 100.0f ) {
		ease = 1.0f - ( elapsed - 100.0f ) * 0.0024999999f;
		if ( ease <= 0.0f ) {
			return;
		}
		ease = 1.0f - ease;
		ease = 1.0f - ( 2.0f - (float)fabs( ease ) ) * ease;
	} else {
		ease = elapsed * 0.0099999998f;
		ease = ( 2.0f - (float)fabs( ease ) ) * ease;
	}

	kick = scale * ease;
	angles[0] = kick * state->viewKickPitch + angles[0];
	angles[2] = kick * state->viewKickRoll + angles[2];
}

/*
==============
BG_CalculateView_Velocity

The ADS view bob; nothing happens at the hip or on a mounted weapon.
==============
*/
void BG_CalculateView_Velocity( bgViewAngleState_t *state, vec3_t angles ) {
	const weaponInfo_t *weaponInfo;
	const playerState_t *ps;
	float phase;

	ps = state->ps;
	weaponInfo = bg_weaponInfo[ps->weapon];
	if ( ( ps->eFlags & EF_MOUNTED ) || ps->fWeaponPosFrac == 0.0f || weaponInfo->adsViewBobMult == 0.0f ) {
		return;
	}

	phase = (byte)ps->bobCycle * 0.024639944f + 6.2831855f;
	angles[0] = angles[0] - BG_GetVerticalBobFactor( ps, phase, state->speed, 45.0f )
				* weaponInfo->adsViewBobMult * ps->fWeaponPosFrac;
	angles[1] = angles[1] - BG_GetHorizontalBobFactor( ps, phase, state->speed, 45.0f )
				* weaponInfo->adsViewBobMult * ps->fWeaponPosFrac;
}

/*
==============
BG_CalculateViewAngles
==============
*/
void BG_CalculateViewAngles( bgViewAngleState_t *state, vec3_t angles ) {
	angles[0] = 0.0f;
	angles[1] = 0.0f;
	angles[2] = 0.0f;
	BG_CalculateView_DamageKick( state, angles );
	BG_CalculateView_Velocity( state, angles );
}

/*
==============
BG_WeaponTrackValue

Move current towards target at rate; snap when the remaining distance is under
a thousandth or the step would overshoot.
==============
*/
float BG_WeaponTrackValue( float target, float current, float rate, int msec ) {
	float diff;
	float step;

	diff = target - current;
	step = msec * 0.001f * diff * rate;
	// the qword at 0x2005f8a8 is (double)0.001f, 0x3F50624DE0000000 -- not 0.001
	if ( fabs( diff ) <= 0.001f || fabs( diff ) < fabs( step ) ) {
		return target;
	}
	return step + current;
}

/*
==============
BG_CalculateWeaponPosition_Sway

Track the view-angle delta into the weapon sway offsets and angles.  Scoped
weapons do not sway once they are up.
==============
*/
void BG_CalculateWeaponPosition_Sway( vec3_t swayOffsets, vec3_t swayAngles, const playerState_t *ps,
									  vec3_t previousViewAngles, float scale, int msec ) {
	const weaponInfo_t *weaponInfo;
	float adsFrac;
	float maxAngle;
	float lerpSpeed;
	float pitchScale, yawScale, horizScale, vertScale;
	float pitchDelta, yawDelta;
	float target;

	weaponInfo = bg_weaponInfo[ps->weapon];
	adsFrac = ps->fWeaponPosFrac;

	if ( weaponInfo->aimDownSight ) {
		if ( adsFrac > 0.0f && weaponInfo->adsOverlayReticle ) {
			return;
		}
		maxAngle = ( weaponInfo->adsSwayMaxAngle - weaponInfo->swayMaxAngle ) * adsFrac + weaponInfo->swayMaxAngle;
		lerpSpeed = ( weaponInfo->adsSwayLerpSpeed - weaponInfo->swayLerpSpeed ) * adsFrac + weaponInfo->swayLerpSpeed;
		pitchScale = ( weaponInfo->adsSwayPitchScale - weaponInfo->swayPitchScale ) * adsFrac + weaponInfo->swayPitchScale;
		yawScale = ( weaponInfo->adsSwayYawScale - weaponInfo->swayYawScale ) * adsFrac + weaponInfo->swayYawScale;
		horizScale = ( weaponInfo->adsSwayHorizScale - weaponInfo->swayHorizScale ) * adsFrac + weaponInfo->swayHorizScale;
		vertScale = ( weaponInfo->adsSwayVertScale - weaponInfo->swayVertScale ) * adsFrac + weaponInfo->swayVertScale;
	} else {
		maxAngle = weaponInfo->swayMaxAngle;
		lerpSpeed = weaponInfo->swayLerpSpeed;
		pitchScale = weaponInfo->swayPitchScale;
		yawScale = weaponInfo->swayYawScale;
		horizScale = weaponInfo->swayHorizScale;
		vertScale = weaponInfo->swayVertScale;
	}

	pitchScale = pitchScale * scale;
	yawScale = yawScale * scale;
	horizScale = horizScale * scale;
	vertScale = vertScale * scale;

	pitchDelta = AngleSubtract( ps->viewangles[0], previousViewAngles[0] );
	yawDelta = AngleSubtract( ps->viewangles[1], previousViewAngles[1] );

	if ( pitchDelta < -maxAngle ) {
		pitchDelta = -maxAngle;
	} else if ( pitchDelta > maxAngle ) {
		pitchDelta = maxAngle;
	}
	if ( yawDelta < -maxAngle ) {
		yawDelta = -maxAngle;
	} else if ( yawDelta > maxAngle ) {
		yawDelta = maxAngle;
	}

	swayOffsets[1] = BG_WeaponTrackValue( yawDelta * horizScale, swayOffsets[1], lerpSpeed, msec );
	swayOffsets[2] = BG_WeaponTrackValue( pitchDelta * vertScale, swayOffsets[2], lerpSpeed, msec );

	target = pitchDelta * pitchScale;
	while ( target - swayAngles[0] > 180.0f ) {
		target = target - 360.0f;
	}
	swayAngles[0] = AngleNormalize180( BG_WeaponTrackValue( target, swayAngles[0], lerpSpeed, msec ) );

	target = yawDelta * yawScale;
	while ( target - swayAngles[1] > 180.0f ) {
		target = target - 360.0f;
	}
	swayAngles[1] = AngleNormalize180( BG_WeaponTrackValue( target, swayAngles[1], lerpSpeed, msec ) );

	previousViewAngles[0] = ps->viewangles[0];
	previousViewAngles[1] = ps->viewangles[1];
	previousViewAngles[2] = ps->viewangles[2];
}
