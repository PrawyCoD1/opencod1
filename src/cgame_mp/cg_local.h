/*
 * @fidelity: partial
 *
 * cg_local.h -- local definitions for the multiplayer client game module
 * (cgame_mp_x86.dll, CoD 1.1, imagebase 0x30000000).
 *
 * Offsets in the comments are binary offsets in that DLL.  The big objects'
 * sizes are CG_Init's own memsets (0x30022F90):
 *
 *      memset( &cg,          0, 0x2B498 )      0x30022FAB   cg          @ 0x301E2140
 *      memset( &cgs,         0, 0x0C67C )      0x30022F9F   cgs         @ 0x301CC0A0
 *      memset( cg_entities,  0, 0x8A000 )      0x30022FB7   cg_entities @ 0x3020DB80
 *      memset( cg_weapons,   0, 0x06600 )      0x30022FC3   cg_weapons  @ 0x301A6940
 *
 * 0x8A000 / 1024 gives centity_t = 552, and CG_RegisterWeapon indexes
 * cg_weapons with `imul ebp, 198h` (0x30034D21), giving 64 x 408.
 *
 * Ranges named `unknown_0x...` are ranges no CoD 1.1 access has been found
 * for, or ranges whose role is known but whose original member name is not.
 * Where a name is taken from RTCW's ordering without a CoD 1.1 access to
 * prove it, the comment says so.
 */

#ifndef __CG_LOCAL_H__
#define __CG_LOCAL_H__

#include <stddef.h>

#include "../universal/q_shared.h"
#include "../qcommon/cmd_history.h"
#include "../game_mp/bg_public.h"

#define CG_ASSERT_SIZE( type, bytes )   COD1_ASSERT_SIZE( type, bytes )

/* cg_public.h owns the engine/cgame ABI: qhandle_t, trajectory_t,
 * entityState_t, clientState_t, gameState_t, snapshot_t, glconfig_t,
 * refdef_t, refEntity_t and the trap numbers. */
#include "cg_public.h"

#define MAX_CLIENTS             64      /* corpse entities start at 64 (CG_ResetEntity 0x3002F95D) */
#define GENTITYNUM_BITS         10
#define MAX_GENTITIES           ( 1 << GENTITYNUM_BITS )
#define ENTITYNUM_NONE          ( MAX_GENTITIES - 1 )
#define MAX_QPATH               64
#define MAX_STRING_CHARS        1024

/*
=============================================================================

        CGAME-PRIVATE TYPES

=============================================================================
*/

/* CG_InitLocalEntities 0x3001FECD: 0x7600 bytes of 236-byte records. */
#define MAX_LOCAL_ENTITIES      128
/* CG_InitMarkPolys 0x3002344D: 0x5D000 bytes of 372-byte records. */
#define MAX_MARK_POLYS          1024
/* CG_ImpactMark clamps a fragment to 10 points (0x300237F8), which is what
   makes markPoly_t's vertex block ten verts long. */
#define MAX_VERTS_ON_POLY       10
/* CG_ResetEntity indexes cgs.corpseinfo with (number - 64) (0x3002F95D) and
   the array runs to the end of cgs: 0x2240 / 1096. */
#define MAX_CORPSES             8
/* CG_RegisterWeapon's stride into cg_weapons (0x30034D21) against the 0x6600
   CG_Init clears. */
#define MAX_WEAPONS_CG          MAX_WEAPONS
/* configstrings 268..523 (CG_ConfigStringModified 0x3002C827). */
#define MAX_MODELS              256
/* configstrings 780..843 (0x3002C84B). */
#define MAX_FX                  64
/* configstrings 1100..1115 (0x3002C873). */
#define MAX_SHELLSHOCK_PARMS    16
/* CG_AddToTeamChat 0x3002C964: 8 lines of 271 chars, wrapped at 90. */
#define TEAMCHAT_HEIGHT         8
#define TEAMCHAT_WIDTH          90
/* CG_FadeColor 0x30019A46: RTCW's 200 became 100. */
#define FADE_TIME               100

/*
 * cgs.media.hintShaders[] -- cg.cursorHintIcon's index space.  0..9 are loaded
 * by name in CG_RegisterGraphics (0x30020DA0), 10..73 and 74..137 are the
 * per-weapon hud and ammo icons CG_RegisterWeapon fills in.
 */
#define HINT_ACTIVATE           2
#define HINT_NOACTIVATE         3
#define HINT_DOOR               4
#define HINT_NODOOR             5
#define HINT_MG42               6
#define HINT_HEALTH             7
#define HINT_LADDER             8
#define HINT_FRIENDLY           9
#define HINT_WEAPON_FIRST       10      /* + weapon - 1 */
#define HINT_WEAPON_LAST        73
#define HINT_AMMO_FIRST         74      /* + weapon - 1 */
#define HINT_AMMO_LAST          137
#define HINT_ICON_COUNT         138

typedef struct centity_s centity_t;

/*
 * The client-side view of one entity.  552 bytes.
 *
 * CoD dropped RTCW's clientEntity_t (`pe`) sub-record -- the player model
 * layer is DObj/XAnim and lives in bg_clientinfo -- so everything past
 * nextState fits in 72 bytes.
 */
struct centity_s {
	entityState_t currentState;             /* +0x000 from cg.frame */
	entityState_t nextState;                /* +0x0F0 from cg.nextFrame, if available */

	qboolean currentValid;                  /* +0x1E0 CG_TransitionEntity 0x3002FA24 sets it,
	                                                  CG_TransitionSnapshot 0x3002FAC8 clears it */
	byte unknown_0x1E4[4];
	int previousEventSequence;              /* +0x1E8 the currentState.eventSequence already fired
	                                                  (CG_CheckEvents 0x3001EA64) */
	int previousPreEventSequence;           /* +0x1EC set beside +0x1E8 by CG_ResetEntity
	                                                  (0x3002F8E4); CG_CheckPreEvents' counterpart */
	byte unknown_0x1F0[8];                  /* +0x1F0 cleared by CG_ResetEntity (0x3002F875) */
	vec3_t lerpOrigin;                      /* +0x1F8 CG_CalcEntityLerpPositions 0x3001D246 */
	vec3_t lerpAngles;                      /* +0x204 CG_CalcEntityLerpPositions 0x3001D25B */
	byte unknown_0x210[12];                 /* +0x210 cleared by CG_ResetEntity (0x3002F859) */
	byte unknown_0x21C[4];
	qhandle_t voiceChatSprite;              /* +0x220 CG_PlayVoiceChat 0x3002D4FE */
	int voiceChatSpriteTime;                /* +0x224 cg.time + cg_voiceSpriteTime,
	                                                  doubled for a non-default sprite */
};
CG_ASSERT_SIZE( centity_t, 552 );

/*
 * markPoly_t -- 372 bytes, CG_AllocMark memsets 0x174 (0x3002353A).  The
 * vertex block is ten 32-byte verts whose modulate[] is at +28 within the vert
 * (CG_AddMarks 0x300239E2), and the duration is the last dword (0x3002392C).
 */
typedef struct markPoly_s {
	struct markPoly_s *prevMark;            /* +0x000 CG_FreeMarkPoly 0x30023507 */
	struct markPoly_s *nextMark;            /* +0x004 */
	int time;                               /* +0x008 CG_AddMarks 0x30023930 */
	int markShader;                         /* +0x00C RTCW ordering; no CoD 1.1 access found */
	qboolean alphaFade;                     /* +0x010 CG_AddMarks 0x300239A0 */
	float color[4];                         /* +0x014 CG_AddMarks 0x300239E2 reads [0..2] */
	int polyShader;                         /* +0x024 RTCW ordering; no CoD 1.1 access found */
	int numVerts;                           /* +0x028 CG_AddMarks 0x300239B2 */
	void            *verts;                 /* +0x02C RTCW ordering; no CoD 1.1 access found */
	polyVert_t vertData[MAX_VERTS_ON_POLY]; /* +0x030 polyVert_t is 32 bytes here, not Q3's 24 */
	int duration;                           /* +0x170 CG_AddMarks 0x3002392C */
} markPoly_t;
CG_ASSERT_SIZE( markPoly_t, 372 );

/*
 * localEntity_t -- 236 bytes, CG_AllocLocalEntity memsets 0xEC (0x3001FFB3).
 * leType is dispatched by CG_AddLocalEntities (0x3002020F): 0 fade-RGB,
 * 1 scale-fade, 2 moving tracer, anything else is "Bad leType: %i".
 */
typedef struct localEntity_s {
	struct localEntity_s *prev;             /* +0x000 CG_FreeLocalEntity 0x3001FF8A */
	struct localEntity_s *next;             /* +0x004 */
	int leType;                             /* +0x008 CG_AddLocalEntities 0x3002020F */
	int leFlags;                            /* +0x00C bit 0 tested by CG_AddScaleFade 0x30020102 */
	int endTime;                            /* +0x010 CG_AddLocalEntities 0x300201CC */
	float lifeRate;                         /* +0x014 1.0 / lifetime (CG_AddScaleFade 0x300200E8) */
	byte unknown_0x018[212];                /* +0x018 carries a refEntity_t: origin at +0x94,
	                                                  radius at +0xCC, shaderRGBA[3] at +0xBF
	                                                  (CG_AddScaleFade 0x30020115..0x3002013F) */
} localEntity_t;
CG_ASSERT_SIZE( localEntity_t, 236 );

/*
 * One scoreboard row, 24 bytes.  CG_ParseScores (0x3002B920) memsets
 * 64 * 24 = 0x600 and walks the array six dwords at a time.
 */
typedef struct score_t {
	int client;                             /* +0x00 clamped to < 64 (0x3002BB13) */
	int score;                              /* +0x04 mirrored into bg_clientinfo[client]+0x30 */
	int ping;                               /* +0x08 summed per team, then averaged (0x3002BB55) */
	int deaths;                             /* +0x0C the CGAME_SB_DEATHS column, 0x3002AF3A */
	int team;                               /* +0x10 read back out of bg_clientinfo[client].team */
	int statusIcon;                         /* +0x14 configstring 20 + n, registered no-mip */
} score_t;
CG_ASSERT_SIZE( score_t, 24 );

/*
 * One camera shake in flight.  cg.cameraShakes is four of these
 * (CG_StartShakeCamera 0x30017E58 strides 36 bytes and stops at cg+0x2AF44).
 */
typedef struct cameraShake_t {
	int startTime;                          /* +0x00 = cg.time (0x30017E14) */
	float amplitude;                        /* +0x04 */
	/* duration is a FLOAT: CG_StartShakeCamera fild's its int argument into
	   this slot (0x30017DEA/0x30017DF7) and CG_UpdateCameraShake fadd's it to
	   (float)startTime. */
	float duration;                         /* +0x08 */
	float radius;                           /* +0x0C third argument of CG_StartShakeCamera */
	vec3_t origin;                          /* +0x10 */
	float weight;                           /* +0x1C the value the replacement search minimises */
	float frameAmplitude;                   /* +0x20 inferred name; CG_UpdateCameraShake
	                                                  recomputes it each frame (0x30017F4B) */
} cameraShake_t;
CG_ASSERT_SIZE( cameraShake_t, 36 );

/*
 * The engine's sound channels.  The indices are
 * CG_SetShellShockParmsFromCvars', which writes cg_shock_volume_<name> into
 * parms+0x34+4*index (0x3002EE94 .. 0x3002F0EF); the names are the cvars'.
 */
typedef enum {
	SND_CHANNEL_AUTO,
	SND_CHANNEL_MENU,
	SND_CHANNEL_BODY,
	SND_CHANNEL_ITEM,
	SND_CHANNEL_WEAPON,
	SND_CHANNEL_VOICE,
	SND_CHANNEL_LOCAL,
	SND_CHANNEL_MUSIC,
	SND_CHANNEL_ANNOUNCER,
	SND_CHANNEL_SHELLSHOCK,
	SND_CHANNEL_COUNT
} sndChannel_t;

/*
 * One entry of cgs.shellshockParms, 124 bytes -- configstrings 1100..1115 feed
 * it through CG_LoadShellShockCvars / CG_SetShellShockParmsFromCvars
 * (CG_ConfigStringModified 0x3002C89A, `imul 7Ch`).
 *
 * CG_SetShellShockParmsFromCvars (0x3002EB30) writes each field straight out
 * of the cg_shock_* cvar of the same name, except +0x00, a hardcoded 3000
 * (0x3002EC13), and +0x04, a FREQUENCY in cycles/ms, 0.001 / max( 0.001, the
 * cvar ), not the period the cvar is named after.
 */
typedef struct shellshockParms_t {
	int         lerpTime;                   /* +0x00 hardcoded 3000 (0x3002EC13) */
	float       viewKickFreq;               /* +0x04 0.001 / max( 0.001, cg_shock_viewKickPeriod ) */
	float       viewKickRadius;             /* +0x08 cg_shock_viewKickRadius */
	int         screenBlendFadeTime;        /* +0x0C ms */
	int         screenBlendTime;            /* +0x10 ms */
	qboolean    soundEnabled;               /* +0x14 cg_shock_sound != 0 */
	int         soundFadeInTime;            /* +0x18 ms */
	int         soundFadeOutTime;           /* +0x1C ms */
	float       soundWetLevel;              /* +0x20 */
	char        soundRoomType[16];          /* +0x24 strncpy 15 + NUL (0x3002EDD2) */
	float       soundVolume[SND_CHANNEL_COUNT];     /* +0x34 indexed by channel */
	int         soundModEndDelay;           /* +0x5C ms */
	int         soundLoopFadeTime;          /* +0x60 ms */
	int         soundLoopEndDelay;          /* +0x64 ms */
	qboolean    mouseEnabled;               /* +0x68 cg_shock_mouse != 0 */
	int         mouseFadeTime;              /* +0x6C ms */
	float       mouseSensitivityScale;      /* +0x70 */
	float       mouseMaxPitchSpeed;         /* +0x74 */
	float       mouseMaxYawSpeed;           /* +0x78 */
} shellshockParms_t;
CG_ASSERT_SIZE( shellshockParms_t, 124 );

/*
 * The shellshock state cg carries.  CG_UpdateShellShock (0x3002F7D0) drives
 * it from ps.shellshockIndex / shellshockTime / shellshockDuration, which
 * CG_DrawActiveFrame reads out of the snapshot at snap+1000..1008 (0x30033CD3).
 */
typedef struct shellshock_t {
	shellshockParms_t *parms;               /* +0x00 &cgs.shellshockParms[ps.shellshockIndex] */
	int startTime;                          /* +0x04 */
	int duration;                           /* +0x08 */
	int soundState;                         /* +0x0C CG_UpdateShellShockSound / CG_EndShellShockSound */
	float mouseSensitivityScale;            /* +0x10 CG_UpdateShellShockMouse 0x3002F5D9, trap 240 */
	float viewDelta[2];                     /* +0x14 CG_UpdateShellShockCamera 0x3002F77E */
	int savedScreenBlend;                   /* +0x1C CG_DrawShellShockSavedScreenBlend */
	int forcedStartTime;                    /* +0x20 cg.time when /shellshock ran (0x30013F41);
	                                                 NOT an index */
	int forcedDuration;                     /* +0x24 CG_ShellShock_f */
} shellshock_t;
CG_ASSERT_SIZE( shellshock_t, 40 );

/*
 * cg_weapons[] -- cgame's per-weapon record.  408 bytes, 64 entries.  This is
 * NOT bg_public.h's 1052-byte weaponInfo_t: that one is the shared weapon
 * definition and cgame reaches it through bg_weaponInfo[weapon]
 * (CG_RegisterWeapon 0x30034D2E).
 *
 * CG_RegisterWeapon (0x30034CF0) fills every field below unless another
 * address is named.
 */
typedef struct cgWeaponInfo_t {
	int             dobj;                   /* +0x000 Com_GetClientDObj( 1024 + weapon ), 0x30035223 */
	float           animRate[21];           /* +0x004 animLength / weaponTime, 1.0 when unset, 0x30034E81 */
	char            handModelName[64];      /* +0x058 strncpy of weaponInfo->handModel, 0x30035230 */
	int             lastWeapAnim;           /* +0x098 CG_WeaponRunXModelAnims 0x30034498 */
	qboolean        registered;             /* +0x09C */
	gitem_t        *item;                   /* +0x0A0 &bg_itemlist[weapon] */
	const char     *displayName;            /* +0x0A4 SE_TranslateReference( weaponInfo->displayName ) */
	const char     *modeName;               /* +0x0A8 */
	const char     *AIOverlayDescription;   /* +0x0AC */
	qhandle_t       worldModel;             /* +0x0B0 */
	int             viewFlashEffect;        /* +0x0B4 CG_WeaponFlash 0x30036C9E */
	int             worldFlashEffect;       /* +0x0B8 CG_WeaponFlash 0x30036CA6 */
	byte            unknown_0x0BC[12];
	const char     *projectileSound;        /* +0x0C8 CG_Missile reads it as missileSound */
	const char     *pullbackSound;          /* +0x0CC */
	const char     *fireSound;              /* +0x0D0 CG_FireWeapon 0x30038C16 */
	const char     *fireEchoSound;          /* +0x0D4 */
	const char     *lastShotSound;          /* +0x0D8 CG_FireWeapon 0x30038C0E */
	const char     *rechamberSound;         /* +0x0DC */
	const char     *reloadSound;            /* +0x0E0 */
	const char     *reloadEmptySound;       /* +0x0E4 */
	const char     *reloadStartSound;       /* +0x0E8 */
	const char     *reloadEndSound;         /* +0x0EC */
	const char     *raiseSound;             /* +0x0F0 falls back to "weap_raise" */
	const char     *altSwitchSound;         /* +0x0F4 */
	const char     *putawaySound;           /* +0x0F8 falls back to "weap_putaway" */
	const char     *noteTrackSoundA;        /* +0x0FC CG_ProcessWeaponNoteTracks 0x30034389 */
	const char     *noteTrackSoundB;        /* +0x100 */
	const char     *noteTrackSoundC;        /* +0x104 */
	const char     *noteTrackSoundD;        /* +0x108 */
	qhandle_t       weaponIcon;             /* +0x10C item->icon */
	qhandle_t       weaponSelectIcon;       /* +0x110 va( "%s_select", item->icon ) */
	qhandle_t       ammoIcon;               /* +0x114 item->ammoicon */
	qhandle_t       hudIcon;                /* +0x118 CG_DrawWeaponSelect 0x30037883 */
	qhandle_t       modeIcon;               /* +0x11C */
	qhandle_t       ammoHudIcon;            /* +0x120 */
	qhandle_t       missileModel;           /* +0x124 weaponInfo->projectileModel */
	byte            unknown_0x128[4];
	float           missileDlight;          /* +0x12C (float)weaponInfo->projectileDLight */
	vec3_t          missileDlightColor;     /* +0x130 CG_Missile 0x3001B49E; never written here */
	int             missileRenderfx;        /* +0x13C CG_Missile 0x3001B4F6; never written here */
	int             shellEjectEffect;       /* +0x140 CG_EjectWeaponBrass 0x30038AF1 */
	int             lastShotEjectEffect;    /* +0x144 CG_EjectWeaponBrass 0x30038A80 */
	int             projExplosionEffect;    /* +0x148 */
	const char     *projExplosionSound;     /* +0x14C */
	int             missileTrailEffect;     /* +0x150 CG_Missile 0x3001B546 */
	byte            unknown_0x154[8];
	qhandle_t       reticleCenter;          /* +0x15C */
	qhandle_t       reticleSide;            /* +0x160 */
	qhandle_t       adsOverlayShader;       /* +0x164 */
	byte            unknown_0x168[0x30];
} cgWeaponInfo_t;
CG_ASSERT_SIZE( cgWeaponInfo_t, 408 );

/*
=============================================================================

        cg_t -- the client game state, rebuilt every frame

=============================================================================
*/

typedef struct cg_s {
	int clientFrame;                        /* +0x00000 CG_DrawActiveFrame prints
	                                                    "cg.clientFrame:%i" (0x30033F78) */
	int clientNum;                          /* +0x00004 */
	qboolean demoPlayback;                  /* +0x00008 vmMain arg2 (0x300339B7) */
	byte unknown_0x0000C[4];                /* +0x0000C read by CG_Draw2D only */
	int unknown_0x00010;                    /* +0x00010 vmMain arg3; CG_CalcCubemapViewValues,
	                                                    CG_CalcViewValues, CG_Draw2D */
	int unknown_0x00014;                    /* +0x00014 vmMain arg4; CG_CalcCubemapViewValues */

	int latestSnapshotNum;                  /* +0x00018 CG_ProcessSnapshots' error message names it */
	int latestSnapshotTime;                 /* +0x0001C */
	snapshot_t      *snap;                  /* +0x00020 cg.snap->serverTime <= cg.time */
	snapshot_t      *nextSnap;              /* +0x00024 */
	snapshot_t activeSnapshots[2];          /* +0x00028 CG_ReadNextSnapshot 0x3003025A */

	float frameInterpolation;               /* +0x25000 CG_SetFrameInterpolation 0x3001BC31 */
	int frametime;                          /* +0x25004 cg.time - cg.oldTime (0x300339E6) */
	int time;                               /* +0x25008 */
	int oldTime;                            /* +0x2500C */
	int physicsTime;                        /* +0x25010 */
	qboolean mapRestart;                    /* +0x25014 CG_MapRestart 0x3002CADA */
	qboolean renderingThirdPerson;          /* +0x25018 0x30033D49 */

	playerState_t predictedPlayerState;     /* +0x2501C CG_Respawn copies snap->ps here (0x30028A8D) */
	centity_t predictedPlayerEntity;        /* +0x270EC 0x3020922C, CG_CalcEntityLerpPositions */

	byte unknown_0x27314[48];               /* +0x27314 cleared by CG_Respawn (0x30028B2C); the
	                                                    ADS / weapon-position lerp block */
	/* bg_public.h's 1052-byte record, NOT cgWeaponInfo_t: the store at
	   0x30028AA9 is bg_weaponInfo[ps.weapon], and every reader resolves against
	   that layout (reticleCenter +0xE8 .. adsCrosshairInFrac +0x348 in
	   cg_draw.c, the view-kick block +0x370..+0x3BC in cg_weapons.c). */
	weaponInfo_t    *weaponInfo;            /* +0x27344 bg_weaponInfo[ps.weapon] (0x30028AA9) */
	byte unknown_0x27348[4];
	vec3_t predictedError;                  /* +0x2734C CG_PredictPlayerState_Internal, CG_Respawn */
	int eventSequence;                      /* +0x27358 CG_CheckPlayerstateEvents 0x30028C54 */
	int predictableEvents[16];              /* +0x2735C indexed & 0xF (0x30028C44) */
	/* CG_StepOffset fmul's flt_302094DC and compares dword_302094E0 against
	   cg.time (0x3003288D, 0x30032867), and CG_OffsetFirstPersonView runs
	   RTCW's LAND_DEFLECT_TIME 150 / LAND_RETURN_TIME 300 curve on
	   flt_302094E4 / dword_302094E8 (0x30032DA3, 0x30032CA1). */
	float stepChange;                       /* +0x2739C */
	int stepTime;                           /* +0x273A0 */
	float landChange;                       /* +0x273A4 */
	int landTime;                           /* +0x273A8 */
	byte unknown_0x273AC[144];              /* +0x273AC six 24-byte records CG_AddPacketEntities
	                                                    fills; role not recovered */

	refdef_t refdef;                        /* +0x2743C */
	vec3_t refdefViewAngles;                /* +0x2748C AnglesToAxis source (0x30033DB8) */
	byte unknown_0x27498[36];               /* +0x27498 cleared by CG_Respawn (0x30028AE6..) */

	/* The DObj cache CG_SetDObjInfo (0x30013470) keys on.  Both arrays are
	   ENTITYNUM_NONE long: CG_FreeEntityDObjInfo walks 64..1022 (0x300134E2)
	   and cg_pEntityLastXModel begins at 0x3020A5F8, 0xFFC past the first. */
	int iEntityLastType[ENTITYNUM_NONE];    /* +0x274BC 0x302095FC */
	void            *pEntityLastXModel[ENTITYNUM_NONE];     /* +0x284B8 0x3020A5F8 */

	byte unknown_0x294B4[4];
	char infoScreenText[MAX_STRING_CHARS];  /* +0x294B8 CG_LoadingString, strncpy 0x3FF + NUL */

	int scoresRequestTime;                  /* +0x298B8 compared against cg.time - 2000
	                                                  (CG_DrawScoreboard 0x30013CF2) */
	int numScores;                          /* +0x298BC CG_ParseScores 0x3002B944, clamped to 64 */
	int teamScores[4];                      /* +0x298C0 [1] and [2] parsed = TEAM_AXIS, TEAM_ALLIES */
	int teamPing[4];                        /* +0x298D0 summed then divided by teamPlayers */
	int teamPlayers[4];                     /* +0x298E0 */
	score_t scores[MAX_CLIENTS];            /* +0x298F0 memset 0x600 at 0x3002B9C0 */

	qboolean showScores;                    /* +0x29EF0 CG_ScoresDown_f / CG_ScoresUp_f,
	                                                  CG_ScoreboardDisplayed 0x3002B6E0 */
	byte unknown_0x29EF4[4];
	int scoreFadeTime;                      /* +0x29EF8 = cg.time when showScores clears */
	int scoreboardScrollPos;                /* +0x29EFC CG_DrawScrollbar, CG_CheckDrawScoreboardLine */
	qboolean scoreboardListFull;            /* +0x29F00 inferred name: the "the list ran off the
	                                                  bottom" latch CG_ScrollScoreboardDown needs
	                                                  set before it will scroll */
	char killerName[32];                    /* +0x29F04 CG_Obituary writes, last byte +0x29F23 */
	byte unknown_0x29F24[1060];

	int centerPrintTime;                    /* +0x2A348 cg.time + 2000 (0x30015A82) */
	/* ints, not RTCW's floats: CG_PriorityCenterPrint __ftol2's its two float
	   arguments into them (0x30015A98, 0x30015AA6) and CG_DrawCenterString
	   fild's them back. */
	int centerPrintCharWidth;               /* +0x2A34C */
	int centerPrintY;                       /* +0x2A350 */
	char centerPrint[MAX_STRING_CHARS];     /* +0x2A354 strncpy 0x3FF, NUL at +0x2A753 */
	int centerPrintLines;                   /* +0x2A754 */
	int centerPrintPriority;                /* +0x2A758 CoD addition; gates the whole function */

	/* CG_ScreenFade (0x30017500) lerps fromColor -> toColor and snaps when the
	   deadline passes. */
	int fadeTime;                           /* +0x2A75C */
	float fadeRate;                         /* +0x2A760 1 / duration; 0 disables */
	float fadeFrom[4];                      /* +0x2A764 */
	float fadeTo[4];                        /* +0x2A774 */

	int lowAmmoWarning;                     /* +0x2A784 CG_CheckAmmo 0x300287D1 */
	int crosshairClientNum;                 /* +0x2A788 CG_ScanForCrosshairEntity 0x30016EF3 */
	int crosshairClientTime;                /* +0x2A78C */
	byte unknown_0x2A790[8];
	int unknown_0x2A798;                    /* +0x2A798 CG_DrawCrosshairNames, CG_SetNextSnap */
	int unknown_0x2A79C;
	byte unknown_0x2A7A0[4];
	int cursorHintIcon;                     /* +0x2A7A4 CG_CheckForCursorHints / CG_DrawCursorhint */
	int cursorHintTime;                     /* +0x2A7A8 */
	int cursorHintFade;                     /* +0x2A7AC cleared by CG_MapRestart (0x3002CAA3) */
	int cursorHintValue;                    /* +0x2A7B0 */
	int cursorHintString;                   /* +0x2A7B4 */
	byte unknown_0x2A7B8[8];
	int damageFeedbackTime;                 /* +0x2A7C0 = cg.time at the top of CG_DamageFeedback */
	int voicetime;                          /* +0x2A7C4 0x3020C904, CG_DrawTimedMenus */
	byte unknown_0x2A7C8[12];
	int itemPickup;                         /* +0x2A7D4 CG_ItemPickup 0x3001DA86 */
	int itemPickupTime;                     /* +0x2A7D8 */
	int itemPickupBlendTime;                /* +0x2A7DC */
	int weaponSelectTime;                   /* +0x2A7E0 */
	byte unknown_0x2A7E4[8];
	byte unknown_0x2A7EC[24];               /* +0x2A7EC cleared by CG_MapRestart (0x3002CB1D);
	                                                    CG_DrawWeaponSelect reads +0x2A800 */
	int unknown_0x2A804;

	/* Eight damage-direction indicators, 12 bytes each: snapshot serverTime,
	   the cg_hudDamageIconTime snapshot, and the yaw (CG_DamageFeedback
	   0x300289D1..0x30028A36).  CG_Respawn and CG_MapRestart clear all 96. */
	int damageIndicators[8][3];             /* +0x2A808 */

	int damageTime;                         /* +0x2A868 = cg.snap->serverTime (0x30028A58) */
	byte unknown_0x2A86C[16];               /* +0x2A87C CG_DrawCrosshair,
	                                                    +0x2A880 CG_EntityEvent / CG_ImpactMark */
	byte unknown_0x2A87C[36];
	int v_dmg_time;                         /* +0x2A8A0 cg.time + 500 (0x30028A4E) */
	float damageY;                          /* +0x2A8A4 -kick when yaw/pitch are both 255 */
	float damageX;                          /* +0x2A8A8 0 in that same case (0x30028860) */
	byte unknown_0x2A8AC[8];                /* +0x2A8AC/+0x2A8B0 CG_OffsetFirstPersonView */
	byte unknown_0x2A8B4[224];

	int unknown_0x2A994;                    /* +0x2A994 CG_AddViewWeapon */
	byte unknown_0x2A998[84];               /* +0x2A998..+0x2A9EC the cg_fx test-effect state
	                                                    (CG_FxTest / CG_FxSetTestPosition) */
	vec3_t kick_angles;                     /* +0x2A9EC CG_WeaponFireRecoil, cleared by CG_Respawn */
	vec3_t kick_origin;                     /* +0x2A9F8 CG_KickAngles 0x30033D58 */
	int unknown_0x2AA04;                    /* +0x2AA04 CG_KickAngles */
	byte unknown_0x2AA08[56];
	byte unknown_0x2AA40[96];               /* +0x2AA40..+0x2AAA0 the view-model position and
	                                                    tag-matrix cache (CG_AddViewWeapon,
	                                                    CG_DObjGetViewModelTagMatrix) */
	byte unknown_0x2AAA0[1044];

	cameraShake_t cameraShakes[4];          /* +0x2AEB4 CG_StartShakeCamera 0x30017ED8 */
	/* both floats (flt_3020D084 / flt_3020D088): CG_ShakeCamera fstp's the sine
	   phase into the first (0x30017FB6) and fld's the second as an always-on
	   floor for weight and amplitude (0x30017F61).  Names inferred. */
	float shakePhase;                       /* +0x2AF44 */
	float shakeMinAmplitude;                /* +0x2AF48 CG_AddPacketEntities clears it */

	int voiceChatTime;                      /* +0x2AF4C 0x3020D08C */
	int voiceChatBufferIn;                  /* +0x2AF50 CG_AddBufferedVoiceChat */
	int voiceChatBufferOut;                 /* +0x2AF54 CG_PlayBufferedVoiceChats */
	int unknown_0x2AF58;                    /* +0x2AF58 cleared by CG_MapRestart */

	float compassNorthYaw;                  /* +0x2AF5C atof(configstring 11), CG_Init 0x300233B7 */
	float compassPlayerAngle;               /* +0x2AF60 0x3020D0A0, CG_UpdateCompassOrientation */
	float unknown_0x2AF64;                  /* +0x2AF64 CG_UpdateCompassOrientation */
	float compassPointerAngle;              /* +0x2AF68 0x3020D0A8, CG_UpdateCompPointerOrientation */
	float unknown_0x2AF6C;
	byte unknown_0x2AF70[1280];             /* +0x2AF70..+0x2AF84 read by CG_DrawCompassFriendlies;
	                                                    the friendly/pointer tables live in here */

	shellshock_t shellshock;                /* +0x2B470 */
} cg_t;
CG_ASSERT_SIZE( cg_t, 0x2B498 + 3 * PLAYERSTATE_EXTRA_BYTES );

/*
=============================================================================

        cgs_t -- the static client game state

=============================================================================
*/

/*
 * cgs.media.  2636 bytes, from cgs+0x99E4 (cgs.media.whiteShader at
 * 0x301D5A84) up to cgs+0xA430.  Shader and sound names are the strings
 * CG_RegisterGraphics (0x30020DA0) and CG_RegisterSounds (0x30020A00)
 * register into each slot.
 */
typedef struct cgMedia_t {
	qhandle_t whiteShader;                  /* +0x000 "white" */
	qhandle_t hudSoftLineShader;            /* +0x004 "hudSoftLine" */
	qhandle_t hudSoftLineHShader;           /* +0x008 "hudSoftLineH" */
	qhandle_t hudAxisIcon;                  /* +0x00C */
	qhandle_t hudAlliedIcon;                /* +0x010 */
	qhandle_t hudColorBar;                  /* +0x014 CG_DrawTeamBackground */
	qhandle_t unknown_0x018;
	qhandle_t railCoreShader;               /* +0x01C "railCore", CG_RailTrail2 */
	qhandle_t headiconVoiceChat;            /* +0x020 CG_PlayVoiceChat */
	qhandle_t headiconTalkBalloon;          /* +0x024 CG_PlayerSprites */
	qhandle_t headiconDisconnected;         /* +0x028 */
	qhandle_t headiconYouInKillCam;         /* +0x02C */
	qhandle_t selectShader;                 /* +0x030 "gfx/2d/select" */
	qhandle_t unknown_0x034;
	qhandle_t tracerShader;                 /* +0x038 "gfx/misc/tracer", CG_DrawTracer */
	qhandle_t disconnectShader;             /* +0x03C "gfx/2d/net.tga" */
	qhandle_t lagometerShader;              /* +0x040 "lagometer" */
	qhandle_t backTileShader;               /* +0x044 "gfx/2d/backtile", CG_TileClear */
	qhandle_t hudNoWeaponIcon;              /* +0x048 CG_DrawWeaponSelect */
	/*
	 * ONE table, not a run of named handles followed by two 64-entry tables:
	 * CG_DrawCursorhint indexes it with cg.cursorHintIcon straight off
	 * media+0x4C (0x30026F2B) and it ends exactly where hudStanceStand begins.
	 * 0..9 are the named hint icons CG_RegisterGraphics loads by name,
	 * 10..73 the per-weapon hud icons and 74..137 the per-weapon ammo icons --
	 * CG_RegisterWeapon writes media+0x70+weapon*4 and media+0x170+weapon*4
	 * (0x30035C71, 0x30035CDC), the compiler having folded the one-based
	 * index's -1 into each base.
	 */
	qhandle_t hintShaders[HINT_ICON_COUNT]; /* +0x04C .. +0x270 */
	qhandle_t hudStanceStand;               /* +0x274 */
	qhandle_t hudStanceCrouch;              /* +0x278 */
	qhandle_t hudStanceProne;               /* +0x27C */
	qhandle_t hudStanceFlash;               /* +0x280 */
	qhandle_t hudObjective;                 /* +0x284 */
	qhandle_t hudObjectiveUp;               /* +0x288 */
	qhandle_t hudObjectiveDown;             /* +0x28C */
	qhandle_t objectiveFriendly;            /* +0x290 "gfx/hud/hud@objective_friendly.tga" */
	qhandle_t objectiveFriendlyChat;        /* +0x294 "gfx/hud/hud@objective_friendly_chat.tga" */
	qhandle_t hudHitDirection;              /* +0x298 CG_DrawDamageDirectionIndicators */
	qhandle_t numberShaders[11];            /* +0x29C CG_DrawField, CG_DrawPlayerHealth */
	qhandle_t markShadowShader;             /* +0x2C8 "markShadow", CG_PlayerShadow */
	qhandle_t flareShader;                  /* +0x2CC */
	qhandle_t wakeMarkShader;               /* +0x2D0 "wake", CG_PlayerSplash */

	sfxHandle_t gibSound;                   /* +0x2D4 "player_gib" */
	sfxHandle_t gibBounceSound;             /* +0x2D8 "player_gib_bounce" */
	sfxHandle_t noAmmoSound;                /* +0x2DC "player_out_of_ammo", CG_CheckAmmo */
	sfxHandle_t landDamageSound;            /* +0x2E0 "land_damage" */
	sfxHandle_t unknown_0x2E4[4];           /* +0x2E4..+0x2F4 CG_EntityEvent only */
	sfxHandle_t talkSound;                  /* +0x2F4 "player_talk" */
	/* Ten surface-type sound/effect tables of 23 entries each, indexed by the
	   surfaceType the trace returns (CG_RegisterSurfaceTypeSounds 0x30020980,
	   CG_RegisterImpactEffects 0x3001A7E0).  Bases seen at cgs+0x9CDC,
	   +0x9D38, +0x9D94, +0x9DF0, +0x9E4C, +0x9EA8, +0x9F04, +0x9F60, +0x9FBC,
	   +0xA018 -- 92-byte stride. */
	byte unknown_0x2F8[920];

	sfxHandle_t gearRattleRun;              /* +0x690 "gear_rattle_run"  (cgs+0xA074) */
	sfxHandle_t gearRattleWalk;             /* +0x694 "gear_rattle_walk" */
	sfxHandle_t movementFoliage;            /* +0x698 "movement_foliage" */
	sfxHandle_t whizbySound;                /* +0x69C "whizby", CG_WhizbySound */
	sfxHandle_t meleeSwingLarge;            /* +0x6A0 */
	sfxHandle_t meleeSwingSmall;            /* +0x6A4 */
	sfxHandle_t meleeHit;                   /* +0x6A8 */
	sfxHandle_t gameMessageSound;           /* +0x6AC "game_message" */
	sfxHandle_t objectiveCompleteSound;     /* +0x6B0 "objective_complete" */
	sfxHandle_t announceGermanTwoMinutes;   /* +0x6B4 "mp_announce_g_twominutes" */
	sfxHandle_t announceAlliedTwoMinutes;   /* +0x6B8 */
	sfxHandle_t announceGermanThirtySeconds;/* +0x6BC */
	sfxHandle_t announceAlliedThirtySeconds;/* +0x6C0 */
	sfxHandle_t playerWaterIn;              /* +0x6C4 */
	sfxHandle_t playerWaterOut;             /* +0x6C8 */
	sfxHandle_t debrisBounce;               /* +0x6CC "debris_bounce" */
	sfxHandle_t grenadePulse[4];            /* +0x6D0 "player_grenade_pulse_0".."_3" */
	sfxHandle_t spotlightSpark;             /* +0x6E0 */
	sfxHandle_t flameSound;                 /* +0x6E4 */
	sfxHandle_t flameBlowSound;             /* +0x6E8 */
	sfxHandle_t flameStreamSound;           /* +0x6EC */
	sfxHandle_t flameCrackSound;            /* +0x6F0 */
	sfxHandle_t playerBoneBounce;           /* +0x6F4 */
	sfxHandle_t debrisHitPlayer;            /* +0x6F8 */
	sfxHandle_t flamebarrelBounce;          /* +0x6FC */
	qhandle_t checkboxClear;                /* +0x700 "ui/assets/checkbox_clear" */
	qhandle_t checkboxChecked;              /* +0x704 "ui/assets/checkbox_checked" */
	qhandle_t checkboxFail;                 /* +0x708 "ui/assets/checkbox_fail" */

	byte unknown_0x70C[832];                /* +0x70C..+0xA4C nine 23-entry impact-effect tables
	                                                  (CG_RegisterImpactEffects; bases cgs+0xA0F0,
	                                                  +0xA14C ... +0xA3D0, 92-byte stride, and a
	                                                  single handle at cgs+0xA42C =
	                                                  "fx/impacts/flesh_hit_noblood.efx") */
} cgMedia_t;
CG_ASSERT_SIZE( cgMedia_t, 2636 );

typedef struct cgs_s {
	stockGameState_t legacyGameState;       /* +0x00000 reserved stock layout */
	glconfig_t glconfig;                    /* +0x05E84 trap 78 (CG_Init 0x3002305D) */
	float screenXScale;                     /* +0x05F24 vidWidth / 640 */
	float screenYScale;                     /* +0x05F28 vidHeight / 480 */
	float screenXBias;                      /* +0x05F2C RTCW ordering; no CoD 1.1 access found */
	int serverCommandSequence;              /* +0x05F30 */
	int processedSnapshotNum;               /* +0x05F34 */
	qboolean localServer;                   /* +0x05F38 atoi of "sv_running" (0x30020652) */

	/* parsed from the serverinfo configstring (CG_ParseServerinfo 0x3002BBB0) */
	char gametype[32];                      /* +0x05F3C key "g_gametype", a string, not an enum */
	char sv_hostname[256];                  /* +0x05F5C key "sv_hostname", strncpy 0x100 */
	int maxclients;                         /* +0x0605C key "sv_maxclients" */
	char mapname[64];                       /* +0x06060 "maps/mp/%s.bsp" */
	byte unknown_0x060A0[128];              /* +0x060A0 two MAX_QPATH slots; RTCW puts redTeam and
	                                                    blueTeam here and 1.1 never touches them */

	int voteTime;                           /* +0x06120 configstring 15 */
	int voteYes;                            /* +0x06124 configstring 17 */
	int voteNo;                             /* +0x06128 configstring 18 */
	qboolean voteModified;                  /* +0x0612C set by all four (0x3002C792) */
	char voteString[256];                   /* +0x06130 configstring 16, NUL at +0x0622F */
	byte unknown_0x06230[544];              /* +0x06230 RTCW's team-vote block is this size */

	int levelStartTime;                     /* +0x06450 configstring 13 */
	int scores1;                            /* +0x06454 configstring 5 */
	int scores2;                            /* +0x06458 configstring 6 */

	qhandle_t gameModels[MAX_MODELS];       /* +0x0645C configstrings 268..523; vmMain export 9
	                                                    returns gameModels[arg0] (0x300204B7) */
	int gameEffects[MAX_FX];                /* +0x0685C configstrings 780..843, trap 222 */
	shellshockParms_t shellshockParms[MAX_SHELLSHOCK_PARMS];        /* +0x0695C configstrings 1100..1115 */

	int unknown_0x0711C;
	qhandle_t inlineDrawModel[MAX_MODELS];  /* +0x07120 trap_R_RegisterModel(va("*%i", i)) */
	byte unknown_0x07520[7168];             /* +0x07520 the inline-model midpoints/bounds
	                                                    (CG_EntityEffects reads +0x07920,
	                                                    CG_RegisterGraphics writes +0x0792C) */

	char teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH * 3 + 1];      /* +0x09120 0x3002C964 */
	int teamChatMsgTimes[TEAMCHAT_HEIGHT];  /* +0x09998 */
	int teamChatPos;                        /* +0x099B8 */
	int teamLastChatPos;                    /* +0x099BC */

	/* vmMain case 7 stores 0x301D5A60 (+0x099C0) into cgDC+0xF4 = cursorx and
	   0x301D5A64 (+0x099C4) into cgDC+0xF8 = cursory (0x3002048E..0x300204A4).
	   int, not float: that case is a plain
	   dword copy in and out of the int cgDC pair, with no fistp and no __ftol
	   between them.  Both slots are read-only in 1.1 -- one xref each, that
	   case, never written -- so the pair is inert. */
	int cursorX;                            /* +0x099C0 0x301D5A60 */
	int cursorY;                            /* +0x099C4 0x301D5A64 */
	byte unknown_0x099C8[12];
	/* CG_Fade / CG_Fade_f / CG_DrawFlashFade.  [0]/[1] are floats, [2] is
	   cg.time and [3] an int duration -- not four floats. */
	float scriptFade[2];                    /* +0x099D4 */
	int scriptFadeStartTime;                /* +0x099DC */
	int scriptFadeDuration;                 /* +0x099E0 */

	cgMedia_t media;                        /* +0x099E4 */

	int unknown_0x0A430;                    /* +0x0A430 CG_MapRestart sets it to -1 (0x3002CAA9);
	                                                    CG_DrawVote / CG_ServerCommand read it */
	int unknown_0x0A434;                    /* +0x0A434 cleared beside it */
	int unknown_0x0A438;                    /* +0x0A438 CG_Init */

	/* ET_CORPSE entities keep a frozen copy of the client's animation record:
	   CG_ResetEntity copies bg_clientinfo[es.clientNum] over
	   corpseinfo[es.number - 64] (0x3002F97B, 0x448 bytes). */
	byte legacyCorpseinfo[MAX_CORPSES * 1096];   /* +0x0A43C */
} cgs_t;
CG_ASSERT_SIZE( cgs_t, 0xC67C );
typedef char cgs_glconfig_offset_check[offsetof( cgs_t, glconfig ) == 0x5E84 ? 1 : -1];
typedef char cgs_media_offset_check[offsetof( cgs_t, media ) == 0x99E4 ? 1 : -1];

/*
=============================================================================

        GLOBALS

=============================================================================
*/

extern cg_t cg;                                         /* 0x301E2140 */
extern cgs_t cgs;                                       /* 0x301CC0A0 */
extern gameState_t cg_gameState;
extern clientInfo_t cg_corpseinfo[MAX_CORPSES];
extern centity_t cg_entities[MAX_GENTITIES];            /* 0x3020DB80 */
/* RTCW's name for the array CG_RegisterWeapon fills; the recovered `cg_weapons`
   symbol at 0x300EEF3C is a different thing -- the pointer CG_RegisterWeapon
   dereferences to reach bg_public.h's 1052-byte weaponInfo_t (0x30034D2E),
   i.e. bg_weaponInfo. */
extern cgWeaponInfo_t cg_weapons[MAX_WEAPONS_CG];       /* 0x301A6940 */
extern markPoly_t cg_markPolys[MAX_MARK_POLYS];         /* 0x30299B00 */
extern localEntity_t cg_localEntities[MAX_LOCAL_ENTITIES];      /* 0x3019EB60 */

/*
 * cgame links the same bg_animation.c / bg_pmove.c the game DLL does, so
 * bg_clientinfo[] is the SAME 1096-byte clientInfo_t bg_public.h declares --
 * CG_TransitionSnapshot memsets 0x448 of an entry and saves animTree at +0x440
 * across it (0x3002FB0D), the array stride is 1096 (0x3002FAFD), team is read
 * back at +0x2C and score written at +0x30 (CG_ParseScores 0x3002BB2B).  In
 * this module it lives at 0x3018BC0C.
 */

/*
=============================================================================

        CVARS

    CG_RegisterCvars (0x300205E0) walks 173 cvarTable_t entries at 0x300749A0;
    the record is { vmCvar_t *vmCvar; char *cvarName; char *defaultString;
    int cvarFlags; }.  The address after each name is that cvar's vmCvar_t.

=============================================================================
*/

extern vmCvar_t cg_ignore;                      /* 0x301E0640 */
extern vmCvar_t cg_drawGun;                     /* 0x301DBAE0 */
extern vmCvar_t cg_cursorHints;                 /* 0x30299560 */
extern vmCvar_t cg_hintFadeTime;                /* 0x30298FC0 */
extern vmCvar_t cg_fov;                         /* 0x30298C60 */
extern vmCvar_t cg_viewsize;                    /* 0x30298360 */
extern vmCvar_t cg_letterbox;                   /* 0x301DFB00 */
extern vmCvar_t cg_stereoSeparation;            /* 0x301E0400 */
extern vmCvar_t cg_shadows;                     /* 0x301D9380 */
extern vmCvar_t cg_draw2D;                      /* 0x301DB420 */
extern vmCvar_t cg_drawStatus;                  /* 0x301D9EC0 */
extern vmCvar_t cg_drawFPS;                     /* 0x301D8720 */
extern vmCvar_t cg_drawSoundOverlay;            /* 0x301DFE60 */
extern vmCvar_t cg_drawScriptUsage;             /* 0x301E0520 */
extern vmCvar_t cg_drawShader;                  /* 0x301E2020 */
extern vmCvar_t cg_drawSnapshot;                /* 0x301D9FE0 */
extern vmCvar_t cg_drawCrosshair;               /* 0x301D9020 */
extern vmCvar_t cg_drawCrosshairNames;          /* 0x301E0BE0 */
extern vmCvar_t cg_drawCrosshairPickups;        /* 0x301E1600 */
extern vmCvar_t cg_drawRewards;                 /* 0x301CBE60 */
extern vmCvar_t cg_hudAlpha;                    /* 0x301E14E0 */
extern vmCvar_t cg_hudCompassSize;              /* 0x3020D940 */
extern vmCvar_t cg_hudCompassMaxRange;          /* 0x301AD180 */
extern vmCvar_t cg_hudCompassMinRange;          /* 0x301DB8A0 */
extern vmCvar_t cg_hudCompassMinRadius;         /* 0x301DF8C0 */
extern vmCvar_t cg_hudCompassSpringyPointers;   /* 0x301DA460 */
extern vmCvar_t cg_hudObjectiveMinHeight;       /* 0x301D9140 */
extern vmCvar_t cg_hudObjectiveMaxHeight;       /* 0x301DF680 */
extern vmCvar_t cg_hudObjectiveMaxRange;        /* 0x301DAB20 */
extern vmCvar_t cg_hudObjectiveMinAlpha;        /* 0x301D9260 */
extern vmCvar_t cg_hudStanceFlash_r;            /* 0x301DBD20 */
extern vmCvar_t cg_hudStanceFlash_g;            /* 0x30298000 */
extern vmCvar_t cg_hudStanceFlash_b;            /* 0x301DF0E0 */
extern vmCvar_t cg_hudStanceHintPrints;         /* 0x301DAC40 */
extern vmCvar_t cg_hudDamageIconWidth;          /* 0x301DAFA0 */
extern vmCvar_t cg_hudDamageIconHeight;         /* 0x301D9DA0 */
extern vmCvar_t cg_hudDamageIconOffset;         /* 0x301D9920 */
extern vmCvar_t cg_hudDamageIconTime;           /* 0x30297EE0 */
extern vmCvar_t cg_hudDamageIconInScope;        /* 0x301E0AC0 */
extern vmCvar_t cg_weaponCycleDelay;            /* 0x301DF9E0 */
extern vmCvar_t cg_weaponSelect;                /* 0x301CBB00 */
extern vmCvar_t cg_crosshairAlpha;              /* 0x301DB780 */
extern vmCvar_t cg_crosshairAlphaMin;           /* 0x301DF200 */
extern vmCvar_t cg_crosshairDynamic;            /* 0x301E09A0 */
extern vmCvar_t cg_crosshairNoGun;              /* 0x3020D5E0 */
extern vmCvar_t cg_brass;                       /* 0x301D8960 */
extern vmCvar_t cg_marks;                       /* 0x301A6280 */
extern vmCvar_t cg_lagometer;                   /* 0x301E1DE0 */
extern vmCvar_t cg_railTrailTime;               /* 0x301DA7C0 */
extern vmCvar_t cg_gunX;                        /* 0x301DB540 */
extern vmCvar_t cg_gunY;                        /* 0x301AD2A0 */
extern vmCvar_t cg_gunZ;                        /* 0x301D9800 */
extern vmCvar_t cg_gun_move_f;                  /* 0x301E1180 */
extern vmCvar_t cg_gun_move_r;                  /* 0x301E0E20 */
extern vmCvar_t cg_gun_move_u;                  /* 0x30298EA0 */
extern vmCvar_t cg_gun_ofs_f;                   /* 0x302997A0 */
extern vmCvar_t cg_gun_ofs_r;                   /* 0x301E00A0 */
extern vmCvar_t cg_gun_ofs_u;                   /* 0x301DA6A0 */
extern vmCvar_t cg_gun_move_rate;               /* 0x301DE7E0 */
extern vmCvar_t cg_gun_move_minspeed;           /* 0x301DA580 */
extern vmCvar_t cg_centertime;                  /* 0x301DBF60 */
extern vmCvar_t cg_bobAmplitudeStanding;        /* 0x301DEC60 */
extern vmCvar_t cg_bobAmplitudeDucked;          /* 0x301AD600 */
extern vmCvar_t cg_bobAmplitudeProne;           /* 0x301DF7A0 */
extern vmCvar_t cg_bobMax;                      /* 0x301DB0C0 */
extern vmCvar_t cg_skybox;                      /* 0x301DB1E0 */
extern vmCvar_t cg_debugProneCheck;             /* 0x301E0D00 */
extern vmCvar_t cg_debugProneCheckDepthCheck;   /* 0x301DC1A0 */
extern vmCvar_t cg_debugposition;               /* 0x301DB9C0 */
extern vmCvar_t cg_debugevents;                 /* 0x301E02E0 */
extern vmCvar_t cg_errordecay;                  /* 0x301E1F00 */
extern vmCvar_t cg_nopredict;                   /* 0x301E1720 */
extern vmCvar_t cg_noplayeranims;               /* 0x302986C0 */
extern vmCvar_t cg_showmiss;                    /* 0x302985A0 */
extern vmCvar_t cg_footsteps;                   /* 0x301E0F40 */
extern vmCvar_t cg_tracerchance;                /* 0x301DF440 */
extern vmCvar_t cg_tracerwidth;                 /* 0x3020D820 */
extern vmCvar_t cg_tracerSpeed;                 /* 0x301DE900 */
extern vmCvar_t cg_tracerlength;                /* 0x302999E0 */
extern vmCvar_t cg_thirdPersonRange;            /* 0x301DFD40 */
extern vmCvar_t cg_thirdPersonAngle;            /* 0x30299680 */
extern vmCvar_t cg_thirdPerson;                 /* 0x301D8840 */
extern vmCvar_t cg_chatTime;                    /* 0x30297CA0 */
extern vmCvar_t cg_chatHeight;                  /* 0x301DC080 */
extern vmCvar_t cg_predictItems;                /* 0x302987E0 */
extern vmCvar_t cg_drawTeamOverlay;             /* 0x301AD060 */
extern vmCvar_t cg_stats;                       /* 0x301CBC20 */
extern vmCvar_t cg_timescale;                   /* 0x301D96E0  registered as "timescale" */
extern vmCvar_t pmove_fixed;                    /* 0x30297B80 */
extern vmCvar_t pmove_msec;                     /* 0x301D9A40 */
extern vmCvar_t cg_hudFiles;                    /* 0x301DEEA0 */
extern vmCvar_t cl_stance;                      /* 0x301A64C0 */
extern vmCvar_t cl_stanceTemp;                  /* 0x301D8F00 */
extern vmCvar_t cg_noTaunt;                     /* 0x301AD3C0 */
extern vmCvar_t cg_voiceSpriteTime;             /* 0x301DA220 */
extern vmCvar_t cg_teamChatsOnly;               /* 0x30298240 */
extern vmCvar_t cg_noVoiceChats;                /* 0x301E13C0 */
extern vmCvar_t cg_noVoiceText;                 /* 0x301D9B60 */
extern vmCvar_t cl_paused;                      /* 0x30299440 */
extern vmCvar_t g_synchronousClients;           /* 0x301E1BA0 */
extern vmCvar_t cg_currentSelectedPlayer;       /* 0x301D8BA0 */
extern vmCvar_t cg_currentSelectedPlayerName;   /* 0x302998C0 */
extern vmCvar_t cg_deadbodyque;                 /* 0x301A6700 */
extern vmCvar_t cg_gametype;                    /* 0x301DA8E0  registered as "g_gametype" */
extern vmCvar_t cg_norender;                    /* 0x301DF560 */
extern vmCvar_t cg_animState;                   /* 0x301DB300 */
extern vmCvar_t cl_waitForFire;                 /* 0x301DA340 */
extern vmCvar_t cg_dumpAnims;                   /* 0x301E1CC0 */
extern vmCvar_t cg_developer;                   /* 0x30298D80  registered as "developer" */
extern vmCvar_t con_minicon;                    /* 0x301DC2C0 */
extern vmCvar_t cg_version;                     /* 0x301E1840  registered as "version" */
extern vmCvar_t cg_subtitles;                   /* 0x301DAA00 */
extern vmCvar_t cg_subtitleMinTime;             /* 0x30299320 */
extern vmCvar_t cg_subtitleWidth;               /* 0x3020DA60 */
extern vmCvar_t cg_gameMessageWidth;            /* 0x301DFC20 */
extern vmCvar_t cg_gameBoldMessageWidth;        /* 0x3020D700 */
extern vmCvar_t cl_languagewarnings;            /* 0x301DBE40 */
extern vmCvar_t cl_languagewarningsaserrors;    /* 0x301DAE80 */
extern vmCvar_t cg_objectiveText;               /* 0x301DB660 */
extern vmCvar_t cg_scoreboardScrollStep;        /* 0x301AD4E0 */
extern vmCvar_t cg_descriptiveText;             /* 0x30298B40 */
extern vmCvar_t cg_r_optimize;                  /* 0x301DEFC0  registered as "r_optimize" */
extern vmCvar_t cg_r_optimizeXModels;           /* 0x301D8DE0  registered as "r_optimizeXModels" */
extern vmCvar_t bg_viewheight_standing;         /* 0x3019D7C0 */
extern vmCvar_t bg_viewheight_crouched;         /* 0x3019DC40 */
extern vmCvar_t bg_viewheight_prone;            /* 0x3019D060 */
extern vmCvar_t bg_duck2prone_time;             /* 0x3019DA00 */
extern vmCvar_t bg_prone2duck_time;             /* 0x3019DE80 */
extern vmCvar_t bg_ladder_yawcap;               /* 0x3019DB20 */
extern vmCvar_t bg_prone_yawcap;                /* 0x3019D460 */
extern vmCvar_t bg_prone_softyawedge;           /* 0x3019CE20 */
extern vmCvar_t bg_foliagesnd_minspeed;         /* 0x3019CF40 */
extern vmCvar_t bg_foliagesnd_maxspeed;         /* 0x3019DD60 */
extern vmCvar_t bg_foliagesnd_slowinterval;     /* 0x3019D6A0 */
extern vmCvar_t bg_foliagesnd_fastinterval;     /* 0x3019D180 */
extern vmCvar_t bg_foliagesnd_resetinterval;    /* 0x3019D580 */
extern vmCvar_t bg_fallDamageMinHeight;         /* 0x3019D8E0 */
extern vmCvar_t bg_fallDamageMaxHeight;         /* 0x3019D2A0 */
extern vmCvar_t bg_debugWeaponAnim;             /* 0x3019E0C0 */
extern vmCvar_t bg_debugWeaponState;            /* 0x3019DFA0 */
extern vmCvar_t cg_shock_screenBlendTime;       /* 0x301E01C0 */
extern vmCvar_t cg_shock_screenBlendFadeTime;   /* 0x301DEA20 */
extern vmCvar_t cg_shock_viewKickPeriod;        /* 0x301DEB40 */
extern vmCvar_t cg_shock_viewKickRadius;        /* 0x301E1A80 */
extern vmCvar_t cg_shock_sound;                 /* 0x301DAD60 */
extern vmCvar_t cg_shock_soundFadeInTime;       /* 0x301D8CC0 */
extern vmCvar_t cg_shock_soundFadeOutTime;      /* 0x302990E0 */
extern vmCvar_t cg_shock_soundLoopFadeTime;     /* 0x301E0760 */
extern vmCvar_t cg_shock_soundLoopEndDelay;     /* 0x301DFF80 */
extern vmCvar_t cg_shock_soundRoomType;         /* 0x30298900 */
extern vmCvar_t cg_shock_soundWetLevel;         /* 0x301DA100 */
extern vmCvar_t cg_shock_soundModEndDelay;      /* 0x301DBC00 */
extern vmCvar_t cg_shock_volume_auto;           /* 0x301D8A80 */
extern vmCvar_t cg_shock_volume_menu;           /* 0x301E1060 */
extern vmCvar_t cg_shock_volume_weapon;         /* 0x301A65E0 */
extern vmCvar_t cg_shock_volume_voice;          /* 0x301ACF40 */
extern vmCvar_t cg_shock_volume_item;           /* 0x30297DC0 */
extern vmCvar_t cg_shock_volume_body;           /* 0x301DF320 */
extern vmCvar_t cg_shock_volume_local;          /* 0x301DED80 */
extern vmCvar_t cg_shock_volume_music;          /* 0x301E0880 */
extern vmCvar_t cg_shock_volume_announcer;      /* 0x301CBF80 */
extern vmCvar_t cg_shock_volume_shellshock;     /* 0x301CBD40 */
extern vmCvar_t cg_shock_mouse;                 /* 0x30298A20 */
extern vmCvar_t cg_shock_mouse_maxpitchspeed;   /* 0x301E12A0 */
extern vmCvar_t cg_shock_mouse_maxyawspeed;     /* 0x301A63A0 */
extern vmCvar_t cg_shock_mouse_sensitivityscale;/* 0x30299200 */
extern vmCvar_t cg_shock_mouse_fadeTime;        /* 0x301D9C80 */
extern vmCvar_t cg_debuganim;                   /* 0x30298480 */
extern vmCvar_t bg_swingSpeed;                  /* 0x301D94A0 */
extern vmCvar_t cg_blood;                       /* 0x30298120 */
extern vmCvar_t cl_serverloadmap;               /* 0x301D95C0 */
extern vmCvar_t cl_serverloadgametype;          /* 0x301E1960 */
extern vmCvar_t cl_serverloadwaiting;           /* 0x301A6820 */

/*
=============================================================================

        PROTOTYPES

    Grouped by the source file that defines them, in address order -- RTCW's
    cg_local.h is laid out the same way.

=============================================================================
*/

/*
 * forward: universal/q_shared.c, universal/q_parse.c and universal/com_math.c.
 * cgame compiles its own copy of all three, exactly as the game DLL does.
 */
int         Q_stricmp( const char *s1, const char *s2 );
int         Q_stricmpn( const char *s1, const char *s2, int n );
char       *Q_strlwr( char *s1 );
char       *Q_CleanStr( char *string );
void        Q_strncpyz( char *dest, const char *src, int destsize );
int         Q_ftol( float f );
int  QDECL  Com_sprintf( char *dest, int size, const char *fmt, ... );
char       *va( const char *format, ... );
float       Com_Clamp( float min, float max, float value );
char       *Info_ValueForKey( const char *s, const char *key );
void        Com_BeginParseSession( const char *name );
void        Com_EndParseSession( void );
void        Com_SetCSV( qboolean enabled );
char       *Com_Parse( char **data_p );
char       *Com_ParseExt( char **data_p, qboolean allowLineBreaks );
char       *Com_ParseOnLine( char **data_p );
void        Com_SkipRestOfLine( int unused, char **data_p );
void        Com_UngetToken( void );
int         Com_Compress( char *data_p );

void        AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void        YawToAxis( float yaw, vec3_t axis[3] );
void        AxisClear( vec3_t axis[3] );
void        AxisCopy( vec3_t in[3], vec3_t out[3] );
void        CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
void        PerpendicularVector( vec3_t dst, const vec3_t src );
void        ByteToDir( int b, vec3_t dir );
float       LerpAngle( float from, float to, float frac );
float       AngleSubtract( float a1, float a2 );
float       AngleNormalize360( float angle );
float       AngleNormalize180( float angle );
void        SinCos( float radians, float *sinOut, float *cosOut );
vec_t       VectorNormalize2( const vec3_t v, vec3_t out );
vec_t       VectorNormalize2D( float *v );
vec_t       Distance( const vec3_t p1, const vec3_t p2 );
float       vectoyaw( const vec3_t vec );
float       vectosignedyaw( const vec3_t vec );
float       RotationToYaw( const float v[2] );
void        RotateVector2D( vec3_t v, float degrees );
void        RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
								     float degrees );
void        AddLeanToPosition( float *pos, float yaw, float leanFrac, float leanDist, float scale );
/* the DESTINATION is the middle argument: 0x0042FB50 and both DLL copies pass
   a in eax, out in edx and b in ecx. */
void        QuatMultiply( const float a[4], float out[4], const float b[4] );
void        MatrixMultiply( const vec3_t in1[3], const vec3_t in2[3], vec3_t out[3] );
void        MatrixMultiply43( const float in1[4][3], const float in2[4][3], float out[4][3] );
/* the AXIS is first and the angles are the destination: the one definition is
   universal/com_math_raw.c's `AxisToAngles( const vec3_t axis[3], vec3_t
   angles )`.  The DLL copy is __usercall( axis@<ecx>, angles@<eax> ). */
void        AxisToAngles( const float axis[4][3], vec3_t angles );
void        DObjSkel2MatrixMultiply43( const float *mat43, float *out, const float *boneMatrix );

//
// cg_animtree_mp.c
//
void       *Hunk_AllocXAnimCreate( int size );
void        CGScr_LoadAnimTrees( void );
void        CG_FreeClientDObjInfo( void );
/* the three below have no call site in 1.1 -- every user inlined them */
void        CG_SetDObjInfo( int entityNum, int eType, void *xmodel );
qboolean    CG_CheckDObjInfoMatches( int entityNum, int eType, void *xmodel );
void        CG_SafeDObjFree( int entityNum );
void        CG_FreeEntityDObjInfo( void );

//
// cg_compass_mp.c
//
/* `rect` is CG_OwnerDraw's rectangle (four floats).  The second parameter is
   CG_OwnerDraw's SHADER, not its scale -- case 88 pushes arg_34 (0x30026FDC);
   the callee never reads it. */
void        CG_DrawCompassFriendlies( float *rect, qhandle_t shader, const float *color );

//
// cg_consolecmds_mp.c
//
qboolean    CG_ConsoleCommand( void );
void        CG_InitConsoleCommands( void );

//
// cg_draw_mp.c
//
/* `width` comes in through ecx in both; neither is called in 1.1. */
int         CG_DrawFieldWidth( int x, int y, int width, int value, int charWidth,
							   int charHeight );
int         CG_DrawField( int x, int y, int width, int value, int charWidth, int charHeight,
						  qboolean dodrawpic, qboolean leftAlign );
/* `team` comes in through eax; the rectangle is floats, not RTCW's ints
   (0x30014796 onward are `fmul dword ptr [esp+arg_*]`). */
void        CG_DrawTeamBackground( float x, float y, float w, float h, float alpha, int team );
void        CG_DrawScriptUsage( void );
void        CG_AddLagometerFrameInfo( void );
void        CG_AddLagometerSnapshotInfo( snapshot_t *snap );
/* `y` and `charWidth` really are floats; the binary __ftol2's both and stores
   ints into cg.centerPrintY / cg.centerPrintCharWidth (0x30015A98, 0x30015AA6).
   `priority` comes in through eax, `str` through edx in CG_CenterPrint. */
void        CG_PriorityCenterPrint( const char *str, float y, float charWidth, int priority );
void        CG_CenterPrint( const char *str, float y, float charWidth );
void        CG_CheckForCursorHints( void );
/* r, g and b are accepted and never read; startTime comes in through ecx and
   duration through eax.  Uncalled in 1.1. */
void        CG_Fade( int r, int g, int b, int a, int startTime, int duration );
void        CG_DrawSoundOverlay( void );
/* `origin` comes in through ecx. */
void        CG_StartShakeCamera( float amplitude, int duration, float radius,
								 const vec3_t origin );
void        CG_ShakeCamera( void );
void        CG_DrawSavedScreenBlend( void );    /* uncalled in 1.1 */
/* stereoView spelled as a plain int, matching CG_DrawActiveFrame's. */
void        CG_DrawActive( int stereoView );

//
// cg_drawtools_mp.c
//
void        CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void        G_AdjustCoordinates( float *x, float *y );
void        CG_FillRect( float x, float y, float width, float height, const float *color );
void        CG_FillRectGradient( float x, float y, float width, float height,
								 const float *color, const float *gradcolor, int gradientType );
void        CG_FilledBar( float x, float y, float w, float h, const float *startColor,
						  float *endColor, const float *bgColor, float frac, int flags );
void        CG_HorizontalPercentBar( float x, float y, float width, float height, float percent );
void        CG_DrawSides( float x, float y, float w, float h, float size );
void        CG_DrawTopBottom( float x, float y, float w, float h, float size );
void        CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void        CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void        CG_DrawRotatedPic( float x, float y, float width, float height, float angle,
							   qhandle_t hShader );
void        CG_DrawRotatedQuadPic( float x, float y, const float *verts, const float *texCoords,
								   float angle, qhandle_t hShader );
/* vmMain entry point 15 dispatches to this one (0x3002042B). */
void        CG_DrawStringExt( float x, float y, const char *string, const float *setColor,
							  qboolean forceColor, qboolean shadow, float charWidth,
							  float charHeight, int maxChars, qboolean virtualScreen );
void        CG_DrawBigString( float x, float y, const char *s, float alpha );
void        CG_DrawBigStringColor( float x, float y, const char *s, const float *color );
void        CG_DrawSmallString( float x, float y, const char *s, float alpha );
void        CG_DrawSmallStringColor( float x, float y, const char *s, const float *color );
int         Q_DrawStrlen( const char *str );
void        CG_TileClear( void );
float      *CG_FadeColor( int startMsec, int totalMsec );
void        CG_GetColorForHealth( int health, int armor, vec4_t hcolor );
void        CG_ColorForHealth( vec4_t hcolor );
void        CG_UpdateCompassOrientation( void );
void        CG_UpdateCompPointerOrientation( void );
/* duration is accepted and ignored -- see cg_drawtools_mp.c */
void        CG_DebugBox( const vec3_t mins, const vec3_t maxs, const float *color, int depthTest, int duration );
void        CG_DebugCircleEx( const vec3_t dir, const vec3_t org, float radius, const float *color,
							  int depthTest, int duration );
void        CG_DebugArc( const vec3_t org, float radius, float startAngle, float endAngle,
						 const float *color, int depthTest, int duration );

//
// cg_effects_mp.c
//
void        CG_RegisterImpactEffects( void );

//
// cg_ent_mp.c
//
void        CG_Beam( centity_t *cent );
void        CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime,
									   vec3_t out, vec3_t outDeltaAngles );
void        CG_AddCEntity( centity_t *cent );
void        CG_SetFrameInterpolation( void );
void        CG_AddPacketEntities( void );
void        CG_DObjUpdateInfo( int obj );
void        CG_DObjSetLocalTagInternal( int obj, const vec3_t origin, const vec3_t angles,
										int boneIndex );
qboolean    CG_DObjSetLocalTag( int obj, int *partBits, const char *tagName,
								const vec3_t origin, const vec3_t angles );
qboolean    CG_DObjSetControlTagAngles( int obj, int *partBits, const char *tagName,
										const vec3_t angles );
float      *CG_DObjGetLocalTagMatrix( int obj, const char *tagName, centity_t *cent );
qboolean    CG_DObjGetWorldTagMatrix( int obj, const char *tagName, centity_t *cent,
									  float *outMatrix );
qboolean    CG_DObjGetViewModelTagMatrix( int obj, const char *tagName, float *outMatrix );
void        CG_CalcEntityLerpPositions( centity_t *cent );
int         CG_GetAnimations( int eType, int entityNum );
int         CG_PreProcess_GetDObj( int entityNum, int eType, qhandle_t hModel );
void        CG_Player_DoControllers( centity_t *cent, int *partBits );
void        CG_DoControllers( centity_t *cent, int *partBits );
void        CG_DObjCalcPose( centity_t *cent, int obj, int *partBits );
void        CG_DObjCalcBone( int obj, int boneIndex, centity_t *cent );
void        CG_DObjCalcBoneGeneric( int entityNum, int boneIndex );
void        CG_ProcessEntity( centity_t *cent );

//
// cg_event_mp.c
//
/* cent in eax, event in edi.  cg_ent.c passes qfalse, cg_playerstate.c
   passes qtrue. */
void        CG_EntityEvent( centity_t *cent, int event, qboolean predicted );
void        CG_CheckEvents( centity_t *cent );
void        CG_CheckPreEvents( centity_t *cent );

//
// cg_hudelem_mp.c
//
/* the only external entry point: CG_Draw2D calls it (0x300188B3).  The other
   twenty functions of the unit are file-static. */
void        CG_DrawHudElems( void );

//
// cg_info_mp.c
//
void        CG_LoadingString( const char *s );
/* trap_R_RegisterModel, trap_R_RegisterShader and trap_R_RegisterShaderNoMip
   all call this before their syscall, and it calls two of them back. */
void        CG_DrawInformation( int localized );

//
// cg_localents_mp.c
//
void            CG_InitLocalEntities( void );
void            CG_FreeLocalEntity( localEntity_t *le );
localEntity_t  *CG_AllocLocalEntity( void );
void            CG_AddLocalEntities( void );
extern int      localEntCount;                  /* 0x300EEF24 */

//
// cg_main_mp.c
//
int         vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
					int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 );
void        CG_RegisterCvars( void );
void        CG_UpdateCvars( void );
int         CG_CrosshairPlayer( void );
void QDECL  CG_Printf( const char *msg, ... );
void QDECL  CG_Error( const char *msg, ... );
void        CG_GameMessage( const char *msg );
void        CG_BoldGameMessage( const char *msg );
void        CG_DreathMessage( const char *attacker, const float *attackerColor, const char *victim,
							  const float *victimColor, const char *icon, float iconWidth,
							  float iconHeight, const float *iconColor );
void QDECL  Com_Error( int level, const char *error, ... );
void QDECL  Com_Printf( const char *msg, ... );
void QDECL  Com_DPrintf( const char *msg, ... );
const char *CG_Argv( int arg );
const char *CG_ConfigString( int index );
void        CG_StartAmbient( void );
/* takes the alias NAME (it forwards it to CG_PlaySoundAliasByName, which hands
   it to trap_Com_PickSoundAlias).  Four call sites pass a resolved
   sfxHandle_t instead and cast -- retail quirk. */
void        CG_PlayClientSoundAliasByName( const char *name );
void        CG_PlayEntitySoundAliasByName( int entnum, const char *name );
/* returns the sound's length in msec (`mov eax, edi` 0x30021C9D, which
   CG_EquipmentSound and CG_LocalSound tail-return); `name` arrives in eax and
   is the alias trap_Com_PickSoundAlias resolves, entnum and origin follow. */
int         CG_PlaySoundAliasByName( const char *name, int entnum, const vec3_t origin );
void        CG_ParseMenu( const char *menuFile, int loadMode );
qboolean    CG_Load_Menu( char **p, int loadMode );
void        CG_LoadMenus( const char *menuFile, int loadMode );
const char *CG_SafeTranslateString_Internal( const char *domain, const char *reference );
const char *CG_SafeTranslateString( const char *reference );
const char *CG_SafeTranslateHudElemString( int index );
const char *CG_TranslateMessage( const char *src, const char *reference );
void        CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void        CG_Shutdown( void );

//
// cg_marks_mp.c
//
void        CG_InitMarkPolys( void );
void        CG_FreeMarkPoly( markPoly_t *le );
/* `expireTime` is cg.time + duration at the one call site and is unread in 1.1. */
markPoly_t *CG_AllocMark( int expireTime );
void        CG_AddMarks( void );
/* vmMain case 14 forwards the engine's twelve raw dwords into this; the ones
   below that are floats reach it as bit patterns, which is why that call site
   reinterprets them in place. */
void        CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
						   float orientation, float red, float green, float blue, float alpha,
						   qboolean alphaFade, float radius, qboolean temporary, int duration );

//
// cg_newDraw_mp.c
//
/* The ownerdraw handlers themselves are static; these are the entry points the
   rest of the module and the menu system reach.  CG_OwnerDraw's argument list
   IS ui_shared.h's displayContextDef_t::ownerDrawItem, including CoD's extra
   `font` between `special` and `scale` (CG_OwnerDraw 0x30026BB0: arg_28 font,
   arg_2C scale, arg_30 color, arg_34 shader, arg_38 textStyle). */
extern int  sortedTeamPlayers[8];               /* 0x3019E800 */
extern int  numSortedTeamPlayers;               /* 0x3019E920 */

/* 0x30023A80, unnamed in retail: the only writer of the three chat buffers,
   and dead in 1.1. */
void        CG_SetPrintString( int type, const char *p );
int         CG_GetSelectedPlayer( void );
void        CG_SelectNextPlayer( void );
void        CG_SelectPrevPlayer( void );
float       CG_GetValue( int ownerDraw, int type );
qboolean    CG_OwnerDrawVisible( int flags );
const char *CG_GetKillerText( void );
const char *CG_GameTypeString( void );
void        CG_OwnerDraw( float x, float y, float w, float h, float text_x, float text_y,
						  int ownerDraw, int ownerDrawFlags, int align, float special,
						  int font, float scale, vec4_t color, qhandle_t shader, int textStyle );
void        CG_MouseEvent( int x, int y );
void        CG_EventHandling( int type );
void        CG_KeyEvent( int key, qboolean down );
qboolean    CG_KeyInterceptEvent( int key, qboolean down );
int         CG_ClientNumFromName( const char *p );
void        CG_ShowResponseHead( void );
void        CG_RunMenuScript( char **args );
void        CG_GetTeamColor( vec4_t *color );

//
// cg_players_mp.c
//
void        CG_Player( centity_t *cent );
void        CG_Corpse( centity_t *cent );
void        CG_UpdatePlayerDObj( centity_t *cent );     /* uncalled in 1.1 */
void        CG_ResetPlayerEntity( centity_t *cent );

//
// cg_playerstate_mp.c
//
void        CG_CheckAmmo( void );
void        CG_DamageFeedback( int yawByte, int pitchByte, int damage );
void        CG_Respawn( void );
void        CG_CheckPlayerstateEvents( playerState_t *ps, playerState_t *ops );
void        CG_CheckChangedPredictableEvents( playerState_t *ps );      /* uncalled in 1.1 */
void        CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );

//
// cg_predict_mp.c
//
void        CG_BuildSolidList( void );
void        CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs,
                      const vec3_t end, int skipNumber, int mask );
void        CG_TraceCapsule( trace_t *result, const vec3_t start, const vec3_t mins,
                             const vec3_t maxs, const vec3_t end, int skipNumber, int mask );
int         CG_PointContents( const vec3_t point, int passEntityNum, int contentMask );
void        CG_PredictPlayerState( void );

//
// cg_scoreboard_mp.c
//
qboolean    CG_DrawScoreboard( void );
qboolean    CG_ScoreboardDisplayed( void );
void        CG_ScrollScoreboardUp( void );
void        CG_ScrollScoreboardDown( void );
void        CG_RegisterScoreboardGraphics( void );

//
// cg_scr_main_mp.c
//
/* The 102-pointer script import block (0x300AAAF8) is NOT declared here: the
   four units that reach into it -- cg_animtree_mp.c, cg_players_mp.c and the
   shared game_mp/bg_animation.c, which cannot see this header -- each extern
   the slots they call, and cg_scr_main_mp.c's comment block is the authority
   on the other 96.  Only the entry point is shared.
   Declared identically in cg_main_mp.c, whose vmMain tail-calls it. */
int         *Scr_FarHook( const void *hooks );
/* the five VM callbacks Scr_FarHook publishes, all empty in cgame */
void        *Scr_GetFunction( const char **pName, int *pDeveloper );
void        *Scr_GetMethod( const char **pName, int *pDeveloper );
void        Scr_SetObjectField( int classnum, int objectNum, int fieldnum );
void        Scr_GetObjectField( int classnum, int objectNum, int fieldnum );
int         Scr_LoadRead( int len );

//
// cg_servercmds_mp.c
//
/*
 * VOICE CHATS.  Here rather than in cg_servercmds_mp.c because CG_RegisterSounds
 * (cg_main.c) issues the same two CG_ParseVoiceChats calls CG_LoadVoiceChats
 * does, against the same storage -- voiceChatLists[0] is 0x302F7D80 and [1] is
 * 0x30340EC8, so voiceChatList_t is 299336 == 72 + 64 * 4676.
 */
#define MAX_VOICEFILES          2
#define MAX_VOICECHATS          64
#define MAX_VOICESOUNDS         64
#define MAX_CHATSIZE            64

typedef struct voiceChat_s
{
	char id[64];
	int numSounds;
	/* trap_Com_SoundAliasString hands back the alias' own name pointer, which
	   is what CG_PlaySoundAliasByName takes (0x3002D48E). */
	const char  *sounds[MAX_VOICESOUNDS];
	char chats[MAX_VOICESOUNDS][MAX_CHATSIZE];
	qhandle_t sprite[MAX_VOICESOUNDS];
} voiceChat_t;
CG_ASSERT_SIZE( voiceChat_t, 4676 );

typedef struct voiceChatList_s
{
	char name[64];
	int gender;
	int numVoiceChats;
	voiceChat_t voiceChats[MAX_VOICECHATS];
} voiceChatList_t;
CG_ASSERT_SIZE( voiceChatList_t, 299336 );

extern voiceChatList_t voiceChatLists[MAX_VOICEFILES];          /* 0x302F7D80 */
int         CG_ParseVoiceChats( const char *filename, voiceChatList_t *voiceChatList,
								int maxVoiceChats );
void        CG_ParseServerinfo( void );
void        CG_ParseWolfinfo( void );
void        CG_SetConfigValues( void );
void        CG_AddToTeamChat( const char *str );
void        CG_LoadVoiceChats( void );
/* uncalled in 1.1 -- CG_VoiceChatListForClient picks the list by team */
int         CG_HeadModelVoiceChats( char *filename );
void        CG_PlayBufferedVoiceChats( void );
void        CG_CloseScriptMenu( void );
void        CG_ExecuteNewServerCommands( int latestSequence );

//
// cg_shellshock_mp.c
//
void        CG_PerturbCamera( void );
qboolean    CG_DrawShellShockSavedScreenBlend( int startTime, int duration,
											   shellshockParms_t *parms );
/* returns qboolean, not void: 0x3002EA52 is `mov eax, 1` on the success path. */
qboolean    CG_SaveShellShockCvars( const char *name );
int         CG_LoadShellShockCvars( const char *name );
void        CG_SetShellShockParmsFromCvars( shellshockParms_t *parms );
void        CG_EndShellShockSound( void );
void        CG_EndShellShockMouse( void );
void        CG_EndShellShockCamera( void );        /* uncalled in 1.1 */
void        CG_EndShellShock( void );
/* the three updaters are __usercall; parms/msec/duration is the stack order
   at 0x3002F2D4 and 0x3002F5F0. */
void        CG_UpdateShellShockSound( shellshockParms_t *parms, int msec, int duration );
void        CG_UpdateShellShockMouse( shellshockParms_t *parms, int msec, int duration );
void        CG_UpdateShellShockCamera( shellshockParms_t *parms, int msec, int duration );
void        CG_UpdateShellShock( int startTime, shellshockParms_t *parms, int duration );
void        CG_UpdateShellShockSavedScreenBlend( shellshockParms_t *parms, int startTime,
												 int duration );   /* uncalled in 1.1 */

//
// cg_snapshot_mp.c
//
void        CG_ProcessSnapshots( void );

//
// cg_syscalls_mp.c
//
// Every call this module makes into the engine, in the order cg_syscalls_mp.c
// defines them (0x30030460 .. 0x300322C0).  Fourteen of the recovered names sit
// one trap number too high and are corrected here, as they are in that unit;
// a trap_syscall_0xNN spelling is a placeholder for an unrecovered name.
//
// PASSFLOAT and IntAsFloat are the same three instructions and /OPT:ICF folded
// them onto one address (0x30030470); both are used, so both are written out.
int         PASSFLOAT( float x );
float       IntAsFloat( int x );
void trap_Print( const char *fmt );
void trap_Error( const char *fmt );
void trap_GameMessage( const char *msg, int width );
void trap_BoldGameMessage( const char *msg, int width );
void trap_DeathMessage( const char *attacker, const float *attackerColor, const char *victim, const float *victimColor, const char *icon, float iconWidth, float iconHeight, const float *iconColor );
void trap_Subtitle( const char *msg, int a2, int a3 );
int trap_Milliseconds( void );
void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void trap_Cvar_Update( vmCvar_t *cvar );
void trap_Cvar_Set( const char *var_name, const char *value );
void trap_Cvar_Setvar( vmCvar_t *cvar, const char *var_name );
void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
int trap_Argc( void );
void trap_Argv( int n, char *buffer, int bufferLength );
void trap_Args( char *buffer, int bufferLength );
int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
int trap_FS_Read( void *buffer, int len, fileHandle_t f );
int trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void trap_FS_FCloseFile( fileHandle_t f );
int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int trap_Com_SaveCvarsToBuffer( const char **cvars, int numCvars, char *buffer, int size );  /* Com_SaveCvarsToBuffer */
int trap_Com_LoadCvarsFromBuffer( const char **cvars, int numCvars, char *buffer, const char *text );  /* Com_LoadCvarsFromBuffer */
void trap_SendConsoleCommand( const char *text );
void trap_AddCommand( const char *cmdName );
void trap_SendClientCommand( const char *s );
void trap_UpdateScreen( void );
/* ( x, yStart, alpha, orientation ) -- see cg_syscalls_mp.c; the recovered
   names for this family sit one slot off. */
void trap_DrawNotifyLines( int x, int yStart, float alpha, int orientation );
void trap_DrawBoldMessages( int x, int yStart, float alpha, int orientation );
void trap_DrawMiniConsole( int x, int yStart, float alpha );
void trap_DrawSubtitles( int x, int yStart, float alpha, int orientation );
void trap_DrawSay( int y );
void trap_CM_LoadMap( const char *mapname );
int trap_CM_NumInlineModels( void );
int trap_CM_InlineModel( int index );
int trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int contents );  /* CM_TempBoxModel( mins, maxs, contents, qfalse ) */
int trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs, int contents );  /* CM_TempBoxModel( mins, maxs, contents, qtrue ) */
int trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );  /* CM_BoxTrace( .., qfalse ) */
void trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles );  /* CM_TransformedBoxTraceExternal( .., qfalse ) */
void trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );  /* CM_BoxTrace( .., qtrue ) */
void trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles );  /* CM_TransformedBoxTraceExternal( .., qtrue ) */
int trap_R_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection,
						  const vec3_t normal, float radius, int maxPoints, polyVert_t *pointBuffer,
						  int maxFragments, markFragment_t *fragmentBuffer, qhandle_t markShader );
void trap_R_LoadWorldMap( const char *mapname );
void trap_R_FinishLoadingModels( void );
void trap_syscall_0x2F( int ignore );  /* re.SetIgnorePrecacheErrors */
qhandle_t trap_R_RegisterModel( const char *name, int arg2 );
int trap_syscall_0x31( int a1, int a2 );  /* re.GetShaderFromModel */
int trap_R_GetXModelByHandle( qhandle_t hModel );
qhandle_t trap_R_RegisterShader( const char *name, int arg2 );
qhandle_t trap_R_RegisterShaderNoMip( const char *name, int arg2 );  /* re.RegisterShaderNoMip -- Material_RegisterHandle */
int trap_R_RegisterFont( const char *name, int a2, int a3, int a4 );
int trap_R_Text_Width( const char *text, int font, float scale, int limit );
int trap_R_Text_Height( int font, float scale );
void trap_R_Text_Paint( float x, float y, int font, float scale, const float *color,
						const char *text, float adjust, int limit, int style );
void trap_R_Text_PaintWithCursor( float x, float y, int font, float scale, const float *color,
								  const char *text, int cursorPos, char cursor, int limit,
								  int style );
const char *trap_SE_TranslateReference( const char *reference );
const char *trap_SE_LocalizeMessage( const char *msg, const char *reference );
int trap_SE_PrintStrlen( const char *text );
int trap_SE_ReadCharFromString( const unsigned char **text, int *advance );
void trap_R_ClearScene( void );
void trap_R_AddRefEntityToScene( const refEntity_t *re );
void trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts );
void trap_syscall_0x41( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys );
void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void trap_syscall_0x43( const vec3_t org, float r, float g, float b, float scale, int id, int visible );  /* re.AddCoronaToScene */
void trap_R_SetFog( int a1, int a2, int a3, float a4, float a5, float a6, float a7 );
void trap_R_RenderScene( const refdef_t *fd );
void trap_R_SaveScreen( void );
void trap_R_BlendSavedScreen( int a1 );
void trap_R_SetColor( const float *rgba );
void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void trap_syscall_0x4A( float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, int a9, int a10, int a11 );  /* re.StretchPicGradient */
void trap_R_DrawStretchPicRotate( float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, int a10 );
void trap_R_DrawQuadPic( int a1, int a2, int a3 );  /* re.DrawQuadPic */
void trap_R_ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs );
void trap_syscall_0x6B( int a1 );  /* re slot +0xA0 */
int trap_R_TrackStatistics( int a1, int a2, int a3, int a4, int a5, int a6 );  /* re.TrackStatistics */
void trap_GetGlconfig( glconfig_t *vidConfig );
void trap_GetGameState( gameState_t *gamestate );
void trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );
int trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );
int trap_GetServerCommand( int serverCommandNumber );
int trap_GetCurrentCmdNumber( void );
int trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );
void trap_SetUserCmdAimValues( const int *aim );
void trap_SetUserCmdInShellshock( int inShellshock );
void trap_syscall_0x76( const char *s, int i );  /* Com_Printf( "%s%i\n", s, i ) */
void trap_syscall_0x77( const char *s, float f );  /* Com_Printf( "%s%f\n", s, f ) */
int trap_MemoryRemaining( void );
int trap_syscall_0x5A( int keynum );  /* Key_IsDown */
int trap_syscall_0x5B( void );  /* cls.keyCatchers */
void trap_syscall_0x5C( int catcher );  /* Key_SetCatcher */
int trap_syscall_0x5D( const char *binding );  /* Key_GetKey */
int trap_CL_LookupColor( const char *name );
int trap_PC_LoadSource( const char *filename );
int trap_PC_FreeSource( int handle );
int trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int trap_PC_SourceFileAndLine( int handle, char *filename, int *line );
void trap_Com_LoadSoundAliases( const char *mapname );
int trap_Com_SoundAliasString( const char *alias );
const char *trap_Com_PickSoundAlias( const char *alias );
const char *trap_Com_GetSoundAlias( int index );
int trap_MSS_PlaySoundAlias( const char *alias, int entnum, const float *origin, int timeOffset );
void trap_MSS_PlayBlendedSoundAliases( const char *alias1, const char *alias2, float blend, int entnum, const float *origin, int timeOffset );
int trap_SurfaceTypeFromName( const char *name );
const char *trap_SurfaceTypeToName( int surfaceType );
int trap_RealTime( int *qtime );
void trap_SnapVector( float *v );
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits );
int trap_CIN_StopCinematic( int handle );
int trap_CIN_RunCinematic( int handle );
void trap_CIN_DrawCinematic( int handle );
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h );
int trap_syscall_0x3E( int a1, int a2 );  /* re.GetEntityToken */
int trap_hunkUsed( void );
int trap_UI_LoadMenu( const char *menuname );
int trap_UI_Popup( const char *menuname );
void trap_UI_ClosePopup( const char *menuname );
void trap_UI_CloseAllMenus( void );
const char *trap_UI_GetMapDisplayName( const char *mapname );
const char *trap_UI_GetGameTypeDisplayName( const char *gametype );
const char *trap_CL_GetServerIPAddress( void );
void trap_XAnimPrecache( int a1 );
int trap_XAnimCreateAnims( int a1, int a2 );
void trap_XAnimCreate( int a1, int a2, int a3 );
int trap_XAnimCreateTree( int a1 );
void trap_XAnimBlend( int a1, int a2, int a3, int a4, int a5, int a6 );
void trap_syscall_0x85( int a1 );  /* DObjFree */
void trap_XAnimClearGoalWeight( int a1, unsigned short a2, float a3 );  /* XAnimClearAnimNode_m */
void trap_XAnimClearTreeGoalWeights( int tree, unsigned short animIndex, float goalWeight );
void trap_XAnimClearTreeGoalWeightsStrict( int tree, unsigned short animIndex, float goalWeight );
void trap_syscall_0x89( int a1, unsigned short a2, float a3, float a4, float a5, unsigned short a6, int a7 );  /* XAnimSetAnimKnob_m */
int trap_XAnimSetCompleteGoalWeightKnobAll( int a1, unsigned short a2, unsigned short a3, float a4, float a5, float a6, unsigned short a7, int a8 );  /* XAnimSetAnimKnobAll_m */
void trap_syscall_0x8B( int a1, unsigned short a2, float a3 );  /* XAnimClearAnimChildren_m */
void trap_syscall_0x8C( int a1, unsigned short a2, float a3, float a4, float a5, unsigned short a6, int a7 );  /* XAnimSetAnimInternalLimited_m */
void trap_XAnimSetCompleteGoalWeight( int a1, unsigned short a2, float a3, float a4, float a5, unsigned short a6, int a7 );
void trap_XAnimSetAnimRate( int a1, unsigned short a2, float a3 );  /* XAnimSetAnimRate_m */
int trap_XAnimIsLooped( scr_anim_t anim );  /* XAnimIsLooped_m( .., scrAnimPub.trees[..] ) */
int trap_syscall_0x90( int a1, unsigned short a2 );  /* XAnimIsLooped_m */
void trap_XAnimSetTime( int a1, unsigned short a2, float a3 );  /* XAnimSetTime_m */
float trap_XAnimGetTime( int a1, unsigned short a2 );  /* XAnimGetTime */
float trap_XAnimGetWeight( int tree, unsigned short animIndex );  /* `tree` arrives in ecx; animIndex is `movzx word` at 0x300319A0 */
void trap_syscall_0x94( void );  /* CL_DObjInvalidateSkels */
void trap_syscall_0x95( int a1, float a2 );  /* DObjUpdateClientInfo_m */
int trap_syscall_0x96( int a1 );  /* xanim.numDeferredNotifies + the notify array */
void trap_syscall_0x97( int a1, int a2 );  /* DObjCalcAnim */
void trap_syscall_0x98( int a1 );  /* DObjDisplayAnim */
void trap_syscall_0x99( int a1, unsigned short a2, int a3, int a4, int a5 );  /* XAnimGetRelDelta_m */
void trap_syscall_0x9A( int a1, unsigned short a2, int a3, int a4 );  /* XAnimGetAbsDelta_m */
void trap_XAnimGetRelDelta( scr_anim_t anim, float *deltaRot, float *deltaMove,
							float startTime, float endTime );  /* XAnimGetRelDeltaForTime_m */
void trap_syscall_0x9C( int a1, int a2, int a3, int a4, float a5 );  /* XAnimGetAbsDeltaForTime_m */
int trap_syscall_0x9D( int a1, int a2 );  /* skel bone address from the DObj */
int trap_syscall_0x9E( int a1 );  /* skel bone address from the DObj */
int trap_syscall_0x9F( int a1, int a2, int a3 );  /* DObjMarkRotTransIndex */
int trap_syscall_0xA0( int a1, int a2, int a3 );  /* DObjMarkControlRotTransIndex */
int trap_XAnimGetAnimName( scr_anim_t anim );
int trap_syscall_0xA2( int a1 );  /* Com_GetClientDObj */
void trap_syscall_0xA3( int a1, int a2, int a3, int a4 );  /* DObjCreate */
/* Com_ClientDObjCreate; the client's four-argument DObjCreate -- see the
   wrapper in cg_syscalls_mp.c.  game_mp/bg_animation.c calls it too. */
void trap_DObjCreate( void *models, unsigned short modelCount, void *tree, int handle );
void trap_SafeDObjFree( int a1, int a2 );  /* Com_SafeClientDObjFree */
int trap_syscall_0xA6( int a1 );  /* deref dword */
int trap_syscall_0xA7( int a1 );  /* 96 * dobj[23] + 48 */
int trap_syscall_0xA8( int a1, int a2 );  /* CL_DObjCreateSkelForBone */
int trap_syscall_0xA9( int a1, int a2 );  /* CL_DObjCreateSkelForBones */
void trap_syscall_0xAA( int a1, int a2, int a3 );  /* DObjGetHierarchyBits */
void trap_syscall_0xAB( int a1, int a2 );  /* DObjCalcSkel */
int trap_syscall_0xAC( int a1 );  /* XModelExists */
int trap_syscall_0xAD( int a1 );  /* dobj[23] -- part count */
int trap_syscall_0xAE( int a1, int a2 );  /* DObjGetBoneIndex */
int trap_syscall_0xAF( int a1, int a2 );  /* DObjGetBoneName */
void trap_syscall_0xB0( int a1, int a2 );  /* DObjBuildPartCollisionTable */
int trap_syscall_0xB1( int a1 );  /* deref dword */
int trap_XAnimIsPrimitive( scr_anim_t anim );  /* anim tree child count == 0 */
int trap_XAnimGetLength( int tree, unsigned short animIndex );  /* anim length in msec */
int trap_syscall_0xB4( int a1, unsigned short a2 );  /* XAnimHasTime */
int trap_syscall_0xB5( scr_anim_t anim );
scr_anim_t trap_syscall_0xB6( scr_anim_t anim, int childIndex );
int trap_XAnimGetAnimTreeSize( int tree );  /* dobj[1] -- the tree */
void trap_syscall_0xB8( int a1, int a2 );  /* XAnimCopyTree */
void trap_syscall_0xB9( int a1 );  /* DObjDumpInfo */
void trap_syscall_0xBA( int a1, int a2, const char *a3 );  /* StatMon_Warning */
void trap_syscall_0xBB( int a1, int a2 );  /* StatMon_GetStatsArray */
void *trap_Z_MallocInternal( int size );
void trap_Z_FreeInternal( void *ptr );
void trap_AddDebugLine( const vec3_t start, const vec3_t end, const vec3_t color,
						int depthTest, int duration );  /* CL_AddDebugLine */
void *trap_GetWeaponInfoMemory( int size, int *pPrevOwner );  /* Com_GetWeaponInfoMemory */
void trap_syscall_0xC8( int a1 );  /* Com_FreeWeaponInfoMemory */
int trap_syscall_0xC9( int a1 );  /* Hunk_AllocAlignInternal( size, 32 ) */
void *trap_Hunk_AllocLowInternal( int size );  /* Hunk_AllocLowAlignInternal( size, 32 ) */
int trap_syscall_0xCB( int a1, int a2 );  /* Hunk_AllocAlignInternal */
void *trap_Hunk_AllocLowAlignInternal( int size, int align );  /* Hunk_AllocLowAlignInternal */
int trap_syscall_0xCD( void );
int trap_syscall_0xCE( void );  /* no-op in retail; return consumed at 0x3001488C */
int trap_syscall_0xCF( void );  /* deref a global dword */
void trap_syscall_0xD1( int a1, int a2, int a3 );  /* MSS_SetListener */
void trap_syscall_0xD2( void );  /* MSS_UpdateLoopingSounds */
void trap_syscall_0xD3( int a1 );  /* MSS_StopSounds */
void trap_syscall_0xD4( const float *a1 );  /* MSS_PlayMusicAlias */
void trap_syscall_0xD5( int a1 );  /* MSS_StopBackground */
void trap_syscall_0xD6( int a1, int a2 );  /* MSS_PlayAmbientAlias */
void trap_syscall_0xD7( float a1, int a2 );  /* MSS_FadeAllSounds */
void trap_syscall_0xD8( const float *a1, int a2 );  /* MSS_FadeSelectSounds */
void trap_syscall_0xD9( const char *a1, float a2, int a3 );  /* MSS_SetEnvironmentEffects */
int trap_MSS_GetSoundOverlay( int a1, int a2, int a3, int a4 );
void trap_syscall_0x55( int userCmdValue, float sensitivityScale );
void trap_syscall_0xD0( float a1, float a2, float a3 );  /* cgameClientLerpOrigin */
void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void trap_Key_SetBinding( int keynum, const char *binding );
void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
int trap_syscall_0xDE( const char *name );  /* CFxScheduler::RegisterEffect */
int trap_syscall_0xDF( int a1, const char *boneName );  /* FX_GetBoneIndex */
void trap_syscall_0xE0( int a1, int a2 );  /* CFxScheduler::PlayEffect( name, simple ) */
void trap_syscall_0xE1( int a1, int a2, int a3 );  /* CFxScheduler::PlayEffect( name ) */
void trap_syscall_0xE2( int a1, int a2, int a3, int a4 );  /* CFxScheduler::PlayEffect( name, bolt ) */
void trap_syscall_0xE3( int a1, int a2 );  /* CFxScheduler::PlayEffect( id, simple ) */
void trap_syscall_0xE4( int a1, int a2, int a3 );  /* FX_PlayEffectID */
void trap_syscall_0xE5( int a1, int a2, int a3, int a4 );  /* CFxScheduler::PlayEffect( id, axis ) */
void trap_syscall_0xE6( void );  /* CFxScheduler::AddScheduledEffects */
int trap_syscall_0xE7( void );  /* FX_Init */
int trap_syscall_0xE8( void );  /* FX_Free( 1 ) */
int trap_syscall_0xE9( void );  /* FX_Free( 0 ) */
void trap_syscall_0xEA( int a1 );  /* SFxHelper::AdjustTime */
void trap_syscall_0xEB( int a1 );  /* SFxHelper::AdjustCamera */
int trap_syscall_0xEC( int a1 );  /* NO ENGINE CASE -- retail Com_Errors on this trap number */
int trap_syscall_0xED( int a1 );  /* NO ENGINE CASE -- retail Com_Errors on this trap number */
int trap_syscall_0xEE( int a1 );  /* NO ENGINE CASE -- retail Com_Errors on this trap number */
int trap_syscall_0xEF( int a1 );  /* NO ENGINE CASE -- retail Com_Errors on this trap number */
void trap_syscall_0xF0( float a1, float a2 );  /* two cgame-owned engine dwords */
void trap_syscall_0xF1( void );  /* CL_FirstSnapshot */

//
// cg_view_mp.c
//
void        CG_FxSetTestPosition( void );
void        CG_FxRestart( void );
void        CG_FxTest( void );
/* vmMain entry point 3 (0x300203B0); spelled with plain ints. */
void        CG_DrawActiveFrame( int serverTime, int stereoView, int demoPlayback,
								int cubemapShot, int cubemapSize );

//
// cg_weapons_mp.c
//
void        CG_RegisterWeapon( int weaponNum );
void        CG_UpdateHandViewmodels( const char *handModel );
void        CG_FreeWeapons( void );
void        CG_RegisterItemVisuals( int itemNum );
void        CG_RegisterItems( void );
void        CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent,
								int drawGun );
void        CG_AddViewWeapon( playerState_t *ps );
void        CG_DrawWeaponSelect( void );
void        CG_AltWeapon_f( void );
void        CG_NextWeapon_f( void );
void        CG_PrevWeapon_f( void );
void        CG_Weapon_f( void );
void        CG_WeaponSlot_f( void );
void        CG_CycleWeap( qboolean next, qboolean checkAmmo );
/* 0x300381B0, a tail chunk of CG_EntityEvent in retail: EV_NOAMMO /
   EV_DROPWEAPON jump into it. */
void        CG_OutOfAmmoChange( void );
void        CG_RailTrail( int type, const vec3_t start, const vec3_t end );
/* `es` arrives in esi and the body uses it (es->pos at 0x30038CC8,
   es->number at 0x30038CD4), distinct from `cent` in eax; CG_EntityPreEvent's
   nextState reaches it that way. */
void        CG_FireWeapon( centity_t *cent, entityState_t *es, int event, int tagIndex );
void        CG_EjectWeaponBrass( entityState_t *es, int event );
void        CG_DrawTracer( const vec3_t start, const vec3_t end );
/* dir / dir2 are the two ByteToDir vectors CG_EntityPreEvent unpacks; they
   reach the effect trap as raw pointers (0x300396A9). */
void        CG_BulletHitEvent( int event, const vec3_t origin, int surfaceType, int entityNum,
							   const vec3_t dir, const vec3_t dir2 );
void        CG_BulletHitClientEvent( int event, const vec3_t origin, int surfaceType,
									 int entityNum );

#endif  /* __CG_LOCAL_H__ */
