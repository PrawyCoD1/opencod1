/*
 * g_public.h -- the game module / engine interface for game_mp_x86.dll.
 *
 * Call of Duty 1.1 multiplayer.  RTCW's game/g_public.h is the structural
 * model: the shared entity view, the trap numbers the module calls out on
 * (gameImport_t) and the entry points the engine calls in (gameExport_t).
 *
 * entityState_t and entityShared_t are declared here AND in
 * server_mp/server.h: the two modules each carry their own copy of the shared
 * ABI, exactly as the retail tree did, and neither header includes the other.
 * The layouts and the size assertions below are the same ones server.h
 * asserts.
 *
 * @fidelity: likely
 */

#ifndef __G_PUBLIC_H__
#define __G_PUBLIC_H__

#include "../universal/q_shared.h"

#ifndef QDECL
#define QDECL   __cdecl
#endif

typedef struct trajectory_t {
	int trType;
	int trTime;
	int trDuration;
	vec3_t trBase;
	vec3_t trDelta;
} trajectory_t;
COD1_ASSERT_SIZE( trajectory_t, 36 );

typedef struct entityState_s {
	int number;
	int eType;
	int eFlags;
	trajectory_t pos;
	trajectory_t apos;
	int time;
	int time2;
	vec3_t origin2;
	vec3_t angles2;
	int otherEntityNum;
	int attackerEntityNum;
	int groundEntityNum;
	int constantLight;
	int loopSound;
	int surfType;
	int index;
	int clientNum;
	int iHeadIcon;
	int iHeadIconTeam;
	int solid;
	int eventParm;
	int eventSequence;
	int events[4];
	int eventParms[4];
	int weapon;
	int legsAnim;
	int torsoAnim;
	float leanf;                    /* +212; BG_PlayerStateToEntityState copies ps.leanf straight across */
	int scale;
	int dmgFlags;
	int animMovetype;
	float fTorsoHeight;
	float fTorsoPitch;
	float fWaistPitch;
} entityState_t;
COD1_ASSERT_SIZE( entityState_t, 240 );

typedef struct entityShared_t {
	qboolean linked;                /* +0xF0 in gentity_t */
	int svFlags;
	int singleClient;
	qboolean bmodel;                /* +0x0C G_RadiusDamage 0x20020CA2, ScrCmd_IsTouching 0x200306F9 */
	vec3_t mins;
	vec3_t maxs;
	int contents;
	vec3_t absmin;
	vec3_t absmax;
	vec3_t currentOrigin;
	vec3_t currentAngles;
	int ownerNum;
	int eventTime;
} entityShared_t;
COD1_ASSERT_SIZE( entityShared_t, 100 );

/* scr_anim_t is in bg_public.h: the animation code needs it too. */

/* g_local.h owns the definition; the traps only ever pass the pointer on. */
typedef struct gentity_s gentity_t;

/*
 * System calls the game module makes into the engine.  These are the numbers
 * SV_GameSystemCalls dispatches on (server_mp/sv_game_mp.c).  200 and up are
 * the QVM math/memory helpers -- the native DLL never issues them, it uses the
 * CRT and q_math directly.
 */
typedef enum
{
	G_PRINT                     = 0,
	G_ERROR                     = 1,
	G_ERROR_NOPREFIX            = 2,
	G_MILLISECONDS              = 3,
	G_CVAR_REGISTER             = 4,
	G_CVAR_UPDATE               = 5,
	G_CVAR_SET                  = 6,
	G_CVAR_VARIABLE_INTEGER_VALUE = 7,
	G_CVAR_VARIABLE_VALUE       = 8,
	G_CVAR_VARIABLE_STRING_BUFFER = 9,
	G_ARGC                      = 10,
	G_ARGV                      = 11,
	G_HUNK_ALLOC                = 12,
	G_HUNK_ALLOC_LOW            = 13,
	G_HUNK_ALLOC_ALIGN          = 14,
	G_HUNK_ALLOC_LOW_ALIGN      = 15,
	G_HUNK_ALLOC_TEMP           = 16,
	G_HUNK_FREE_TEMP            = 17,
	G_FS_FOPEN_FILE             = 18,
	G_FS_READ                   = 19,
	G_FS_WRITE                  = 20,
	G_FS_RENAME                 = 21,
	G_FS_FCLOSE_FILE            = 22,
	G_SEND_CONSOLE_COMMAND      = 23,
	G_LOCATE_GAME_DATA          = 24,
	G_DROP_CLIENT               = 25,
	G_SEND_SERVER_COMMAND       = 26,
	G_SET_CONFIGSTRING          = 27,
	G_GET_CONFIGSTRING          = 28,
	G_GET_CONFIGSTRING_CONST    = 29,
	G_GET_USERINFO              = 30,
	G_SET_USERINFO              = 31,
	G_GET_SERVERINFO            = 32,
	G_SET_BRUSH_MODEL           = 33,
	G_TRACE                     = 34,
	G_TRACE_CAPSULE             = 35,
	G_SIGHT_TRACE               = 36,
	G_SIGHT_TRACE_CAPSULE       = 37,
	G_SIGHT_TRACE_TO_ENTITY     = 38,
	G_CM_BOX_TRACE              = 39,
	G_CM_BOX_TRACE_2            = 40,
	G_CM_BOX_SIGHT_TRACE        = 41,
	G_CM_BOX_SIGHT_TRACE_2      = 42,
	G_LOCATIONAL_TRACE          = 43,
	G_POINT_CONTENTS            = 44,
	G_IN_PVS                    = 45,
	G_IN_PVS_IGNORE_PORTALS     = 46,
	G_IN_SNAPSHOT               = 47,
	G_ADJUST_AREA_PORTAL_STATE  = 48,
	G_AREAS_CONNECTED           = 49,
	G_LINKENTITY                = 50,
	G_UNLINKENTITY              = 51,
	G_ENTITIES_IN_BOX           = 52,
	G_ENTITY_CONTACT            = 53,
	G_GET_USERCMD               = 54,
	G_GET_ENTITY_TOKEN          = 55,
	G_FS_GETFILELIST            = 56,
	G_REAL_TIME                 = 57,
	G_SNAPVECTOR                = 58,
	G_ENTITY_CONTACT_CAPSULE    = 59,
	G_FIND_SOUND_ALIAS          = 60,
	G_PICK_SOUND_ALIAS          = 61,
	G_SOUND_ALIAS_INDEX         = 62,
	G_SURFACE_TYPE_FROM_NAME    = 63,
	G_SURFACE_TYPE_TO_NAME      = 64,
	G_ADD_TEST_CLIENT           = 65,
	G_GET_ARCHIVED_CLIENT_INFO  = 66,
	G_ADD_DEBUG_STRING          = 67,
	G_ADD_DEBUG_LINE            = 68,
	G_ENABLE_ARCHIVED_SNAPSHOT  = 69,
	G_Z_MALLOC                  = 70,
	G_Z_FREE                    = 71,
	G_XANIM_CREATE_TREE         = 72,
	G_XANIM_CREATE_SMALL_TREE   = 73,
	G_XANIM_FREE_SMALL_TREE     = 74,
	G_XMODEL_EXISTS             = 75,
	G_XMODEL_GET                = 76,
	G_DOBJ_CREATE               = 77,
	G_DOBJ_EXISTS               = 78,
	G_DOBJ_FREE                 = 79,
	G_DEREF_DWORD               = 80,
	/* 81 is unused: the engine has no case for it and no wrapper issues it. */
	G_XANIM_CLEAR_ANIM          = 82,
	G_XANIM_CLEAR_ANIM_NODE     = 83,
	G_XANIM_CLEAR_ANIM_CHILDREN = 84,
	G_XANIM_SET_ANIM_KNOB       = 85,
	G_XANIM_SET_ANIM_KNOB_ALL   = 86,
	G_XANIM_SET_ANIM_RATE       = 87,
	G_XANIM_SET_TIME            = 88,
	G_XANIM_SET_ANIM_LIMITED    = 89,
	G_XANIM_CLEAR_TREE          = 90,
	G_XANIM_IS_PRIMITIVE        = 91,
	G_XANIM_IS_LEAF             = 92,
	G_XANIM_LENGTH_MSEC         = 93,
	G_XANIM_LENGTH_SEC          = 94,
	G_XANIM_SET_ANIM_KNOB_INT   = 95,
	G_XANIM_SET_ANIM_INT_LIMITED = 96,
	G_XANIM_GET_ABS_DELTA       = 97,
	G_XANIM_GET_REL_DELTA       = 98,
	G_XANIM_GET_REL_DELTA_TIME  = 99,
	G_XANIM_GET_ABS_DELTA_TIME  = 100,
	G_XANIM_IS_LOOPED           = 101,
	G_XANIM_HAS_NOTETRACK       = 102,
	G_XANIM_GET_TIME            = 103,
	G_XANIM_GET_WEIGHT          = 104,
	G_DOBJ_DUMP_INFO            = 105,
	G_DOBJ_CREATE_SKEL_FOR_BONE = 106,
	G_DOBJ_SKEL_UP_TO_DATE      = 107,
	G_DOBJ_UPDATE_SERVER_TIME   = 108,
	G_DOBJ_INIT_SERVER_TIME     = 109,
	G_DOBJ_GET_HIERARCHY_BITS   = 110,
	G_DOBJ_CALC_ANIM            = 111,
	G_DOBJ_CALC_SKEL            = 112,
	G_XANIM_RESTORE_TREE        = 113,
	G_XANIM_SAVE_TREE           = 114,
	G_XANIM_COPY_TREE           = 115,
	G_DOBJ_NUM_PARTS            = 116,
	G_DOBJ_GET_PART_INDEX       = 117,
	G_DOBJ_GET_SKEL_BASE        = 118,
	G_DOBJ_DISPLAY_ANIM         = 119,
	G_XANIM_HAS_TIME            = 120,
	G_XANIM_CHILD_COUNT         = 121,
	G_XANIM_CHILD_BASE          = 122,
	G_XANIM_ROOT_PART_COUNT     = 123,
	G_XANIM_ROOT_PART_LIST      = 124,
	G_DOBJ_GET_SKEL_END         = 125,
	G_DOBJ_SET_ROT_TRANS_INDEX  = 126,
	G_DOBJ_SET_CONTROL_ROT_TRANS_INDEX = 127,
	G_XANIM_GET_ANIM_NAME       = 128,
	G_DOBJ_GET_TREE             = 129,
	G_XANIM_TREE_FIELD          = 130,
	G_DOBJ_DEBUG_DRAW_BOUNDS    = 131,
	G_GET_WEAPON_INFO_MEMORY    = 132,
	G_FREE_WEAPON_INFO_MEMORY   = 133,
	G_FREE_CLIENT_SCRIPT_PERS   = 134,
	G_RESET_ENTITY_PARSE_POINT  = 135,
	G_MEMSET                    = 200,
	G_MEMCPY                    = 201,
	G_STRNCPY                   = 202,
	G_SIN                       = 203,
	G_COS                       = 204,
	G_ATAN2                     = 205,
	G_SQRT                      = 206,
	G_MATRIXMULTIPLY            = 207,
	G_ANGLEVECTORS              = 208,
	G_PERPENDICULARVECTOR       = 209,
	G_FLOOR                     = 210,
	G_CEIL                      = 211
} gameImport_t;

/*
 * Entry points the engine calls in through vmMain (0x20024F40).  The numbering
 * is vmMain's own switch; the names are the ones the server side already uses
 * on its VM_Call sites.  vmMain also answers 13, 14, 15, 17 and 19 -- 17 and 19
 * are named below because sv_snapshot_mp.c calls them, but 13 (G_DObjCalcPose),
 * 14 (weapon-index validation) and 15 (the match-state setter) have no name on
 * the engine side and are deliberately left out rather than invented.
 */
typedef enum
{
	GAME_INIT                    = 0,
	GAME_SHUTDOWN                = 1,
	GAME_CLIENT_CONNECT          = 2,
	GAME_CLIENT_BEGIN            = 3,
	GAME_CLIENT_USERINFO_CHANGED = 4,
	GAME_CLIENT_DISCONNECT       = 5,
	GAME_CLIENT_COMMAND          = 6,
	GAME_CLIENT_THINK            = 7,
	GAME_GET_CLIENT_INFO         = 8,
	GAME_UPDATE_CVARS            = 9,
	GAME_RUN_FRAME               = 10,
	GAME_CONSOLE_COMMAND_SV      = 11,
	GAME_SCRIPT_FAR_HOOK         = 12,
	GAME_GET_MATCH_STATE         = 16,
	GAME_GET_CLIENT_STATE        = 17,
	GAME_GET_ARCHIVE_TIME        = 18,
	GAME_SET_ARCHIVE_TIME        = 19,
	GAME_GET_CLIENT_SCORE        = 20
} gameExport_t;

#endif  /* __G_PUBLIC_H__ */
