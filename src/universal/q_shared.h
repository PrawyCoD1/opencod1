/*
 * q_shared.h -- the engine-wide declarations every layer sees.
 *
 * Call of Duty 1.1 (Windows, CoDMP.exe).  RTCW's game/q_shared.h is the
 * structural model: everything below has a counterpart there unless a comment
 * says otherwise.
 *
 * Every layout here carries a static assertion against the size recorded in
 * the CoDMP.exe type library.  If one fires, the declaration is wrong, not the
 * assertion.
 *
 * The assertion macros live here because they have to be reachable from every
 * header a struct can land in -- q_shared.h, qcommon.h, server.h, cm_local.h,
 * cm_patch_local.h, files_local.h -- and this is the only header all of those
 * reach, directly or through qcommon.h.
 *
 * @fidelity-default: verified
 */

#ifndef __Q_SHARED_H__
#define __Q_SHARED_H__

/* NULL, size_t, offsetof.  RTCW's q_shared.h opens with the C headers for
 * the same reason; without it a unit that includes only <math.h> beside
 * cg_local.h has no NULL. */
#include <stddef.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define COD1_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
/* Pre-C11 fallback: a negative-width array typedef.  The name must be unique
 * per use, and __LINE__ only expands if it is passed through two levels of
 * macro before being pasted. */
#define COD1_SA_CAT_(a, b) a##b
#define COD1_SA_CAT(a, b) COD1_SA_CAT_(a, b)
#define COD1_STATIC_ASSERT(cond, msg) \
	typedef char COD1_SA_CAT(cod1_static_assert_, __LINE__)[(cond) ? 1 : -1]
#endif

#define COD1_ASSERT_SIZE(type, bytes) \
	COD1_STATIC_ASSERT(sizeof(type) == (bytes), #type " must be " #bytes " bytes")

typedef unsigned char byte;

typedef enum qboolean_e
{
	qfalse = 0x0,
	qtrue  = 0x1,
} qboolean;

/*
 * NOTE: the values below are the 1.1 ones; the loose cod1-types-common.h in
 * the workspace root carries an extra ERR_VID_FATAL at 1 and shifts the rest
 * up by one.  TODO: re-confirm against Com_Error's callers before anything
 * depends on the numeric values.
 */
typedef enum errorParm_e
{
	ERR_FATAL            = 0x0,
	ERR_DROP             = 0x1,
	ERR_SERVERDISCONNECT = 0x2,
	ERR_DISCONNECT       = 0x3,
	ERR_NEED_CD          = 0x4,
	ERR_AUTOUPDATE       = 0x5,
} errorParm_t;

/*
=============================================================================

	VECTOR / PLANE PRIMITIVES

	Q3/RTCW q_shared.h, unchanged except where noted.

=============================================================================
*/

typedef float vec_t;
typedef float vec3_t[3];
typedef float vec4_t[4];

#ifndef M_PI
#define M_PI        3.14159265358979323846f
#endif

extern vec3_t vec3_origin;

/*
 * g_color_table -- the eight colours the ^N escape selects, indexed by
 * ColorIndex().  Defined in q_shared.c, which every image compiles its own
 * copy of: 0x00541950 in the exe, 0x200555B0 in game_mp_x86.dll, 0x30060868
 * in cgame_mp_x86.dll.
 */
extern const vec4_t g_color_table[8];

#define DotProduct( x,y )           ( (x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] )
#define VectorSubtract( a,b,c )     ( (c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2] )
#define VectorAdd( a,b,c )          ( (c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2] )
#define VectorCopy( a,b )           ( (b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2] )
#define VectorScale( v,s,o )        ( (o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s) )
#define VectorMA( v,s,b,o )         ( (o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s) )
#define VectorClear( a )            ( (a)[0]=(a)[1]=(a)[2]=0 )
#define VectorNegate( a,b )         ( (b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2] )
#define VectorSet( v, x, y, z )     ( (v)[0]=(x),(v)[1]=(y),(v)[2]=(z) )

void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
vec_t VectorLength( const vec3_t v );
vec_t VectorNormalize( vec3_t v );
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );
void SinCos( float radians, float *sinOut, float *cosOut );

/*
 * SetPlaneSignbits (0x00430C40) writes signbits as a byte at +17, and
 * BoxOnPlaneSide (0x00430C80) reads that same byte to index its lazily built
 * 8-entry jump table; type is at +16 and the whole structure is 20 bytes --
 * identical to Q3/RTCW.
 */
typedef struct cplane_t
{
	vec3_t normal;
	float dist;
	byte type;                      // for fast side tests
	byte signbits;                  // signx + (signy << 1) + (signz << 2)
	byte pad[2];
} cplane_t;
COD1_ASSERT_SIZE( cplane_t, 20 );

/*
 * There is only one trace in 1.1 and it is the 48-byte one below, not RTCW's
 * 56-byte form with an embedded cplane_t: entityNum and partName are 16-bit,
 * allsolid/startsolid are bytes, and there is no cplane_t.
 *
 * client_t.frames[32] spans 269696 bytes, so clientSnapshot_t is 8428 and
 * playerState_t is 8400 -- which only holds if the embedded
 * serverCursorHintTrace is 48 bytes rather than 56.
 */
typedef struct trace_t
{
	float fraction;                 /* +0x00  time completed, 1.0 = hit nothing */
	vec3_t endpos;                  /* +0x04  final position */
	vec3_t normal;                  /* +0x10  surface normal at impact */
	int surfaceFlags;               /* +0x1C */
	int contents;                   /* +0x20  UO name */
	int material;                   /* +0x24  UO name */
	unsigned short entityNum;       /* +0x28 */
	unsigned short partName;        /* +0x2A */
	unsigned short partGroup;       /* +0x2C  UO name */
	byte allsolid;                  /* +0x2E */
	byte startsolid;                /* +0x2F */
} trace_t;
COD1_ASSERT_SIZE( trace_t, 48 );

/*
=============================================================================

						HANDLES AND FILE ACCESS

=============================================================================
*/

typedef int clipHandle_t;
typedef int fileHandle_t;

typedef enum fsMode_t
{
	FS_READ        = 0x0,
	FS_WRITE       = 0x1,
	FS_APPEND      = 0x2,
	FS_APPEND_SYNC = 0x3,
} fsMode_t;

typedef enum fsOrigin_t
{
	FS_SEEK_CUR = 0x0,
	FS_SEEK_END = 0x1,
	FS_SEEK_SET = 0x2,
} fsOrigin_t;

/*
=============================================================================

						CVARS

=============================================================================
*/

typedef int cvarHandle_t;

typedef enum cvarFlags_e
{
	CVAR_ARCHIVE             = 0x1,
	CVAR_USERINFO            = 0x2,
	CVAR_SERVERINFO          = 0x4,
	CVAR_SYSTEMINFO          = 0x8,
	CVAR_INIT                = 0x10,
	CVAR_LATCH               = 0x20,
	CVAR_ROM                 = 0x40,
	CVAR_USER_CREATED        = 0x80,
	CVAR_TEMP                = 0x100,
	CVAR_CHEAT               = 0x200,
	CVAR_NORESTART           = 0x400,
	CVAR_WOLFINFO            = 0x800,
	CVAR_UNSAFE              = 0x1000,
	CVAR_SERVERINFO_NOUPDATE = 0x2000,
} cvarFlags_t;

enum
{
	MAX_CVAR_VALUE_STRING = 0x100,
	MAX_CVAR_NAME_LENGTH  = 0x40,
	MAX_CVARS             = 0x400,
};

typedef struct vmCvar_s
{
	cvarHandle_t handle;
	int modificationCount;
	float value;
	int integer;
	char string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;
COD1_ASSERT_SIZE( vmCvar_t, 272 );

typedef struct cvar_s
{
	char            *name;
	char            *string;
	char            *resetString;
	char            *latchedString;
	cvarFlags_t flags;
	qboolean modified;
	int modificationCount;
	float value;
	int integer;
	struct cvar_s   *next;
	struct cvar_s   *hashNext;
} cvar_t;
COD1_ASSERT_SIZE( cvar_t, 44 );

/*
=============================================================================

						CONFIGSTRINGS

=============================================================================
*/

enum
{
	MAX_CONFIGSTRINGS = 0x800,
};

/*
=============================================================================

						PLAYER STATE

=============================================================================
*/

typedef enum pmtype_t
{
	PM_NORMAL        = 0x0,
	PM_NORMAL_LINKED = 0x1,
	PM_NOCLIP        = 0x2,
	PM_UFO           = 0x3,
	PM_SPECTATOR     = 0x4,
	PM_INTERMISSION  = 0x5,
	PM_DEAD          = 0x6,
	PM_DEAD_LINKED   = 0x7,
} pmtype_t;

typedef enum weaponstate_t
{
	WEAPON_READY                 = 0x0,
	WEAPON_RAISING               = 0x1,
	WEAPON_DROPPING              = 0x2,
	WEAPON_FIRING                = 0x3,
	WEAPON_RECHAMBERING          = 0x4,
	WEAPON_RELOADING             = 0x5,
	WEAPON_RELOADING_INTERUPT    = 0x6,
	WEAPON_RELOAD_START          = 0x7,
	WEAPON_RELOAD_START_INTERUPT = 0x8,
	WEAPON_RELOAD_END            = 0x9,
	WEAPON_MELEE_INIT            = 0xA,
	WEAPON_MELEE_FIRE            = 0xB,
	WEAPONSTATES_NUM             = 0xC,
} weaponstate_t;

/*
 * 24 bytes, naturally aligned (the #pragma pack is not load-bearing).  The
 * 30-byte packed form with int buttons, offHandIndex, selectedLoc and
 * remoteControlAngles is the UO/CoD2 shape, not 1.1's: client_t places
 * downloadName at 0x10A64, which walking client_t from offset 0 reaches only
 * when usercmd_t is 24 bytes (a 30-byte one lands at 0x10A6A).
 */
typedef struct usercmd_t
{
	int serverTime;
	byte buttons;                   /* console, chat, ads, attack, use */
	byte wbuttons;                  /* lean left, lean right, reload */
	byte weapon;
	byte flags;
	int angles[3];
	signed char forwardmove;
	signed char rightmove;
	signed char upmove;
	byte unknown;                   /* TODO: unidentified trailing byte */
} usercmd_t;
COD1_ASSERT_SIZE( usercmd_t, 24 );

typedef struct objective_t
{
	int state;
	vec3_t origin;
	int entNum;
	int teamNum;
	int icon;
} objective_t;
COD1_ASSERT_SIZE( objective_t, 28 );

/*
 * One script-created HUD element as it travels in playerState_t.hud.
 *
 * These 112 bytes are the first 112 of game_mp/g_local.h's 124-byte
 * g_hudelem_t (the field table at 0x200558D0, HudElem_SetDefaults and the
 * HECmd_* setters); HudElem_UpdateClient (0x20022760) copies exactly 0x70 of
 * it into ps.hud, dropping the three server-only words.  Member names follow
 * United Offensive's; CoD 1.1 carries no symbols for them.
 *
 * color / fromColor are four bytes here because that is how the client reads
 * them (0x3001F31E / 0x3001F326, one channel at a time); g_hudelem_t spells
 * the same four bytes as one packed int, which is how the server writes them.
 */
typedef struct hudelem_t
{
	int type;                       /* +0x00 hudElemType_t; 0 = free slot */
	int x;                          /* +0x04 CG_HudElemX 0x3001EFCF */
	int y;                          /* +0x08 CG_HudElemY 0x3001F03F */
	float fontScale;                /* +0x0C CG_GetHudElemInfo 0x3001F140 */
	int font;                       /* +0x10 0x3001F129 */
	int alignX;                     /* +0x14 CG_HudElemX 0x3001EFF8 */
	int alignY;                     /* +0x18 CG_HudElemY 0x3001F068 */
	byte color[4];                  /* +0x1C */
	byte fromColor[4];              /* +0x20 the fadeOverTime source colour */
	int fadeStartTime;              /* +0x24 0x3001F30F */
	int fadeTime;                   /* +0x28 0x3001F2FB */
	int label;                      /* +0x2C localized string index, 0x3001F1AE */
	int width;                      /* +0x30 CG_HudElemShaderWidth 0x3001EE23 */
	int height;                     /* +0x34 CG_HudElemShaderHeight 0x3001EE93 */
	int materialIndex;              /* +0x38 CG_DrawHudElemShader 0x3001F70B */
	int scaleFromWidth;             /* +0x3C 0x3001EE53 */
	int scaleFromHeight;            /* +0x40 0x3001EEC3 */
	int scaleStartTime;             /* +0x44 0x3001EE48 */
	int scaleTime;                  /* +0x48 0x3001EE37 */
	int moveFromX;                  /* +0x4C 0x3001EFCC */
	int moveFromY;                  /* +0x50 0x3001F03C */
	int moveStartTime;              /* +0x54 0x3001EFC5 */
	int moveTime;                   /* +0x58 0x3001EFB3 */
	int timerValue;                 /* +0x5C CG_GetHudElemTime 0x3001EC8A */
	int rotationPeriodMs;           /* +0x60 CG_DrawHudElemClock 0x3001F5BD */
	float value;                    /* +0x64 CG_GetHudElemInfo 0x3001F237 */
	int text;                       /* +0x68 localized string index, 0x3001F20E */
	float sortKey;                  /* +0x6C compare_hudelems 0x3001F8EA */
} hudelem_t;
COD1_ASSERT_SIZE( hudelem_t, 112 );

typedef struct hudElemState_t
{
	hudelem_t current[31];
	hudelem_t archival[31];
} hudElemState_t;
COD1_ASSERT_SIZE( hudElemState_t, 6944 );

typedef struct playerState_s
{
	int commandTime;
	pmtype_t pm_type;
	int bobCycle;                   // for view bobbing and footstep generation
	int pm_flags;
	int pm_time;
	vec3_t origin;
	vec3_t velocity;
	int weaponTime;
	int weaponDelay;
	int grenadeTimeLeft;
	int iFoliageSoundTime;
	int gravity;
	float leanf;
	int speed;
	// add to command angles to get view direction.  SHORT angle units held in
	// dwords, not floats: PM_UpdateViewAngles stores `16000 - cmd->angles[i]`
	// as an integer (0x2000B1CE) and accumulates `+= ANGLE2SHORT(...)` with an
	// integer add (0x2000B2EB).  msg_mp.c nets all three at 16 bits.
	int delta_angles[3];
	int groundEntityNum;
	vec3_t vLadderVec;
	int jumpTime;
	float fJumpOriginZ;
	int legsTimer;
	int legsAnim;
	int torsoTimer;
	int torsoAnim;
	int movementDir;
	int eFlags;
	int eventSequence;
	int events[4];
	int eventParms[4];
	int oldEventSequence;
	int clientNum;
	unsigned int weapon;
	weaponstate_t weaponstate;
	float fWeaponPosFrac;
	int viewmodelIndex;
	vec3_t viewangles;
	int viewHeightTarget;
	float viewHeightCurrent;
	int viewHeightLerpTime;
	int viewHeightLerpTarget;
	int viewHeightLerpDown;
	float viewHeightLerpPosAdj;     // PM_ViewHeightAdjust fld/fsub/fstp's it
	                                // (0x20009792, 0x20009852); msg_mp.c nets
	                                // ps+224 as a float too.
	int damageEvent;
	int damageYaw;
	int damagePitch;
	int damageCount;
	int stats[6];
	int ammo[64];
	int ammoclip[64];
	unsigned int weapons[2];
	byte weaponslots[8];
	unsigned int weaponrechamber[2];
	vec3_t mins;
	vec3_t maxs;
	int proneViewHeight;
	int crouchViewHeight;
	int standViewHeight;
	int deadViewHeight;
	float walkSpeedScale;
	float runSpeedScale;
	float proneSpeedScale;
	float crouchSpeedScale;
	float strafeSpeedScale;
	float backSpeedScale;
	float leanSpeedScale;           /* +0x364  named by the retail playerState field table */
	float proneDirection;
	float proneDirectionPitch;
	float proneTorsoPitch;
	int viewlocked;
	int viewlocked_entNum;
	float friction;
	int gunfx;
	int serverCursorHint;
	int serverCursorHintVal;
	/* +908 is serverCursorHintString (retail playerState field table), which
	 * puts serverCursorHintTrace at 912-960. */
	int serverCursorHintString;
	trace_t serverCursorHintTrace;
	int iCompassFriendInfo;
	float fTorsoHeight;
	float fTorsoPitch;
	float fWaistPitch;
	int entityEventSequence;
	int weapAnim;
	float aimSpreadScale;
	int shellshockIndex;
	int shellshockTime;
	int shellshockDuration;
	objective_t objective[16];
	hudElemState_t hud;
	int ping;
	int deltaTime;
} playerState_t;
COD1_ASSERT_SIZE( playerState_t, 8400 );

/*
=============================================================================

						PARSER

	universal/q_parse.c.  RTCW keeps the parse session file-static in
	q_shared.c and only exports the API; CoD 1.1 has a real global session
	record that the renderer reads directly (tr_bsp.c, tr_shader.c), so the
	layout has to be visible and lives with the API that owns it.

=============================================================================
*/

typedef struct parseInfo_t
{
	char token[1024];
	int currentLine;
	qboolean ungetReady;
	qboolean spaceDelimited;
	qboolean csv;
	qboolean parseNegativeNumbers;
	int ungetLineSave;
	char            *ungetTokenSave;
	char filename[64];
} parseInfo_t;
COD1_ASSERT_SIZE( parseInfo_t, 1116 );

#endif  /* __Q_SHARED_H__ */
