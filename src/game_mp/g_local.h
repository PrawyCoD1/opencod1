/*
 * @fidelity: partial
 *
 * g_local.h -- local definitions for the multiplayer game module
 * (game_mp_x86.dll, CoD 1.1, imagebase 0x20000000).
 *
 * Offsets in the comments are binary offsets recovered from that DLL.  The
 * struct sizes asserted below are the retail sizes: gentity_t 788, gclient_t
 * 8900, clientSession_t 260, level_locals_t 0x2A14 (the length G_InitGame
 * at 0x20025DD0 hands to memset).
 *
 * Ranges named `unknown_0x...` are ranges no CoD 1.1 access has been found
 * for, or ranges whose role is known but whose original member name is not.
 */

#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

#include "../universal/q_shared.h"
/* g_public.h owns the engine-side half of the ABI: trajectory_t,
 * entityState_t, entityShared_t and the trap numbers. */
#include "g_public.h"
#include "bg_public.h"

#define G_ASSERT_SIZE( type, bytes ) COD1_ASSERT_SIZE( type, bytes )

/* G_InitGame prints `gamename: main` / `gamedate: Oct  8 2003`. */
#define GAMEVERSION             "main"

/* These belong in q_shared.h in the original tree; universal/q_shared.h does
 * not carry them, so the game module defines them here. */
#define MAX_CLIENTS             64
#define GENTITYNUM_BITS         10
#define MAX_GENTITIES           ( 1 << GENTITYNUM_BITS )
#define ENTITYNUM_NONE          ( MAX_GENTITIES - 1 )
#define MAX_STRING_CHARS        1024

/* G_InitGame seeds level.num_entities with MAX_CLIENTS + BODY_QUEUE_SIZE. */
#define BODY_QUEUE_SIZE         8

#define MAX_SPAWN_VARS          64
#define MAX_SPAWN_VARS_CHARS    2048

/* playerState_t.objective[] is 16 entries; level mirrors it one for one. */
#define MAX_OBJECTIVES          16

/* level.notifyWatch is 0xC00 bytes of 12-byte records (G_RunFrame 0x20026B20). */
#define MAX_NOTIFY_WATCHES      256

/* Six attached models per entity and per client (Scr_FreeEntityConstStrings
 * 0x20037630). */
#define MAX_ATTACHED_MODELS     6

/* G_RunFrameForEntity (0x20026A10) compares level.time - ent->eventTime. */
#define EVENT_VALID_MSEC        300

/* gentity_t.flags bits actually tested by the retail game module. */
#define FL_GODMODE              0x00000001      /* G_Damage 0x20020620 */
#define FL_NOCLIENT             0x00001000      /* G_RunFrameForEntity 0x20026A10 */
#define FL_SUPPORTS_LINKTO      0x00002000      /* ClientSpawn 0x2001A050 */

typedef enum moverState_e {
	MOVER_POS1        = 0x0,
	MOVER_POS2        = 0x1,
	MOVER_POS3        = 0x2,
	MOVER_1TO2        = 0x3,
	MOVER_2TO1        = 0x4,
	MOVER_2TO3        = 0x5,
	MOVER_3TO2        = 0x6,
	MOVER_POS1ROTATE  = 0x7,
	MOVER_POS2ROTATE  = 0x8,
	MOVER_1TO2ROTATE  = 0x9,
	MOVER_2TO1ROTATE  = 0xA,
} moverState_t;

/* Names and values from ClientScr_SetSessionTeam (0x20019150) and its error
 * string "Must be allies, axis, none, or spectator." */
typedef enum team_e {
	TEAM_FREE       = 0x0,
	TEAM_AXIS       = 0x1,
	TEAM_ALLIES     = 0x2,
	TEAM_SPECTATOR  = 0x3,

	TEAM_NUM_TEAMS  = 0x4,
} team_t;

typedef enum sessionState_e {
	STATE_PLAYING      = 0x0,
	STATE_DEAD         = 0x1,
	STATE_SPECTATOR    = 0x2,
	STATE_INTERMISSION = 0x3,
} sessionState_t;

typedef enum clientConnected_e {
	CON_DISCONNECTED = 0x0,
	CON_CONNECTING   = 0x1,
	CON_CONNECTED    = 0x2,
} clientConnected_t;

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;
typedef struct gitem_s gitem_t;
typedef struct entityLinkInfo_s entityLinkInfo_t;


struct gentity_s {
	entityState_t s;                        /* +0x000 communicated by the server to clients */
	entityShared_t r;                       /* +0x0F0 shared by the server system and the game */

	/* DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	   EXPECTS THE FIELDS IN THAT ORDER! */
	/*================================*/

	byte unknown_0x154[4];                  /* SP_worldspawn (0x20037C80) copies spawnflags here; no other access found */

	gclient_t       *client;                /* +0x158 NULL if not a client */
	void            *turret;                /* +0x15C turret state record; G_FreeEntity (0x2003C1E0) zeroes its first dword and this slot */

	byte inuse;                             /* +0x160 */
	byte physicsObject;                     /* +0x161 selects G_RunItem in G_RunFrameForEntity */

	/* Sound alias indices, each parsed from its key by DoorSetSounds
	   (0x2002D4B0); the store follows the *next* key load. */
	byte soundCloseEnd;                     /* +0x162 door_close_end */
	byte soundOpening;                      /* +0x163 door_opening */
	byte soundClosing;                      /* +0x164 door_closing */
	byte soundOpenEnd;                      /* +0x165 door_open_end */
	byte soundLoop;                         /* +0x166 fed to s.loopSound while a binary mover runs (0x2002BDB1); never written in 1.1 */
	byte soundOpenLoop;                     /* +0x167 door_open_loop */
	byte soundCloseLoop;                    /* +0x168 door_close_loop */
	byte soundLocked;                       /* +0x169 door_locked */
	byte soundOpeningQuiet;                 /* +0x16A door_opening_quiet */
	byte soundOpenQuietEnd;                 /* +0x16B door_open_quiet_end */
	byte soundClosingQuiet;                 /* +0x16C door_closing_quiet */
	byte soundCloseQuietEnd;                /* +0x16D door_close_quiet_end */
	byte noise_index;                       /* +0x16E pickup/hurt sound alias (G_SpawnItem 0x200247A1, hurt_touch 0x20039ED5) */
	byte watertype;                         /* +0x16F ClientThink_real 0x2001809F */
	byte waterlevel;                        /* +0x170 ClientThink_real 0x200180A5 */
	byte takedamage;                        /* +0x171 G_Damage 0x20020620 */
	byte active;                            /* +0x172 cleared by SetMoverState/G_FreeEntity */
	byte unknown_0x173;
	byte moverState;                        /* +0x174 moverState_t, stored as a byte */
	byte model;                             /* +0x175 field "model", type 8 */
	unsigned short classname;               /* +0x176 field "classname", const string id */
	int spawnflags;                         /* +0x178 field "spawnflags" */
	int flags;                              /* +0x17C FL_* */
	int eventTime;                          /* +0x180 events clear EVENT_VALID_MSEC after this */
	qboolean freeAfterEvent;                /* +0x184 */
	qboolean unlinkAfterEvent;              /* +0x188 */
	float physicsBounce;                    /* +0x18C */
	int clipmask;                           /* +0x190 */
	int lastFrameNum;                       /* +0x194 G_RunFrameForEntity's once-per-frame latch */

	gentity_t       *parent;                /* +0x198 */
	gentity_t       *nextTrain;             /* +0x19C also the target_location chain (target_location_linkup 0x20039590) */
	gentity_t       *prevTrain;             /* +0x1A0 no CoD 1.1 access found; RTCW ordering */

	vec3_t pos1;                            /* +0x1A4 */
	vec3_t pos2;                            /* +0x1B0 */
	vec3_t pos3;                            /* +0x1BC also the damage point stored by G_Damage */

	unsigned short message;                 /* +0x1C8 field "message" */
	byte unknown_0x1CA[2];
	int timestamp;                          /* +0x1CC hurt_touch's re-trigger stamp (0x20039E8E) */
	float angle;                            /* +0x1D0 field "degrees" */
	unsigned short target;                  /* +0x1D4 */
	unsigned short targetname;              /* +0x1D6 */
	unsigned short team;                    /* +0x1D8 field "teamname" */
	byte unknown_0x1DA[2];
	gentity_t       *target_ent;            /* +0x1DC no CoD 1.1 access found; RTCW ordering */

	float speed;                            /* +0x1E0 also the field "time" */
	float closespeed;                       /* +0x1E4 */
	vec3_t movedir;                         /* +0x1E8 */

	int gDuration;                          /* +0x1F4 */
	int gDurationBack;                      /* +0x1F8 */

	int nextthink;                          /* +0x1FC */
	void ( *think )( gentity_t *self );                                     /* +0x200 */
	void ( *reached )( gentity_t *self );                                   /* +0x204 movers call this when hitting endpoint */
	void ( *blocked )( gentity_t *self, gentity_t *other );                 /* +0x208 */
	void ( *touch )( gentity_t *self, gentity_t *other, int touchMode );    /* +0x20C */
	void ( *use )( gentity_t *self, gentity_t *other, gentity_t *activator );  /* +0x210 */
	void ( *pain )( gentity_t *self, gentity_t *attacker, int damage, const float *point, int mod, const float *dir, int hitLoc );      /* +0x214 */
	void ( *die )( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod, int weapon, const float *dir, int hitLoc );  /* +0x218 */

	byte unknown_0x21C[4];
	void ( *controller )( gentity_t *self, unsigned int *partBits );        /* +0x220 DObj bone controller (G_DObjCalcPose 0x2003B960) */
	int painDebounceTime;                   /* +0x224 P_DamageFeedback 0x200172EB, ClientEvents 0x2001797B */
	byte unknown_0x228[8];

	int health;                             /* +0x230 field "health" */
	int maxHealth;                          /* +0x234 Scr_SetHealth (0x20034E20) writes it for non-client entities */

	int damage;                             /* +0x238 field "dmg" */
	int splashDamage;                       /* +0x23C */
	int splashMinDamage;                    /* +0x240 */
	int splashRadius;                       /* +0x244 */
	int methodOfDeath;                      /* +0x248 */
	int splashMethodOfDeath;                /* +0x24C */

	int count;                              /* +0x250 also the field "shard" */

	gentity_t       *chain;                 /* +0x254 no CoD 1.1 access found; RTCW ordering */
	gentity_t       *enemy;                 /* +0x258 G_Damage stores the attacker here */
	gentity_t       *activator;             /* +0x25C Reached_BinaryMover 0x2002BF67 */
	gentity_t       *teamchain;             /* +0x260 G_FindTeams 0x20025BEF */
	gentity_t       *teammaster;            /* +0x264 G_FindTeams 0x20025C01 */

	float wait;                             /* +0x268 */
	float random;                           /* +0x26C */

	int radius;                             /* +0x270 */
	float delay;                            /* +0x274 */

	byte unknown_0x278[4];
	float duration;                         /* +0x27C */
	vec3_t rotate;                          /* +0x280 also the damage direction stored by G_Damage */
	vec3_t TargetAngles;                    /* +0x28C turret_use 0x20028E3C, Cmd_Activate_f 0x2001F977 */

	gitem_t         *item;                  /* +0x298 for bonus items */

	vec3_t dl_color;                        /* +0x29C fields "color" / "_color" */

	int key;                                /* +0x2A8 */

	float harc;                             /* +0x2AC */
	float varc;                             /* +0x2B0 */

	int missionLevel;                       /* +0x2B4 */

	byte unknown_0x2B8[4];
	int start_size;                         /* +0x2BC */
	int end_size;                           /* +0x2C0 */
	byte unknown_0x2C4[4];
	unsigned short spawnitem;               /* +0x2C8 */
	byte unknown_0x2CA[2];
	int clipCount;                          /* +0x2CC rounds in a dropped weapon's magazine (Drop_Weapon 0x20023F52) */
	byte unknown_0x2D0[12];
	unsigned short track;                   /* +0x2DC */
	byte unknown_0x2DE[6];

	entityLinkInfo_t *linkInfo;             /* +0x2E4 tag-link record; G_RunFrameForEntity routes ET 3 to G_GeneralLink when set */
	gentity_t       *firstChild;            /* +0x2E8 head of the list G_FreeEntity unlinks */
	byte attachModelIndex[MAX_ATTACHED_MODELS];              /* +0x2EC */
	unsigned short attachTagName[MAX_ATTACHED_MODELS];       /* +0x2F2 const string ids */
	byte unknown_0x2FE[2];

	int spawnCount;                         /* +0x300 incremented by G_FreeEntity and preserved across its memset */
	gentity_t       *nextFree;              /* +0x304 level.freeListHead/Tail chain */

	int voiceChatSquelch;                   /* +0x308 G_Voice 0x2001E86F */
	int voiceChatPreviousTime;              /* +0x30C */
	byte unknown_0x310[4];
};
G_ASSERT_SIZE( gentity_t, 788 );

/*
 * Client data that survives ClientSpawn's wipe of the rest of gclient_t
 * (ClientSpawn 0x2001A050 saves and restores exactly these 260 bytes).
 * Offsets in this struct are relative to gclient_t + 0x20D0.
 */
typedef struct clientSession_t {
	sessionState_t sessionState;    /* +0x00 script field "sessionstate" */
	int forceSpectatorClient;       /* +0x04 script field "spectatorclient" */
	int statusIcon;                 /* +0x08 script field "statusicon", configstring 20 + n */
	int archiveTime;                /* +0x0C script field "archivetime" */
	int score;                      /* +0x10 script field "score" */
	int deaths;                     /* +0x14 script field "deaths" */
	unsigned short pers;            /* +0x18 script field "pers", a script object id */
	byte unknown_0x1A[2];
	clientConnected_t connected;    /* +0x1C */
	usercmd_t cmd;                  /* +0x20 we would lose angles if not persistant */
	usercmd_t oldcmd;               /* +0x38 previous command processed by pmove() */
	qboolean localClient;           /* +0x50 true if the "ip" info key is "localhost" */
	qboolean predictItemPickup;     /* +0x54 based on the cg_predictItems userinfo */
	qboolean pmoveFixed;            /* +0x58 OR'd into pm.pmove_fixed (0x20017CA5); nothing in 1.1 ever writes it */
	char netname[32];               /* +0x5C ClientCleanName output */
	int handicap;                   /* +0x7C script field "handicap" */
	int maxHealth;                  /* +0x80 script field "maxhealth" */
	int enterTime;                  /* +0x84 PlayerCmd_setEnterTime 0x2001C900 */
	byte unknown_0x88[4];
	int voteCount;                  /* +0x8C read once (Cmd_CallVote_f 0x2001EBB9) and never written */
	byte unknown_0x90[4];
	int complaints;                 /* +0x94 Cmd_Vote_f 0x2001F454 */
	int complaintClient;            /* +0x98 PlayerCmd_allowComplaint 0x2001B5D0 */
	int complaintEndTime;           /* +0x9C level.time + 20500 */
	int viewmodel;                  /* +0xA0 PlayerCmd_SetViewmodel 0x2001B4D0, a model index */
	byte unknown_0xA4[4];
	int clientNum;                  /* +0xA8 */
	team_t sessionTeam;             /* +0xAC script field "sessionteam" */
	int modelIndex;                 /* +0xB0 ClientEndFrame 0x20018E9D */
	int attachModelIndex[MAX_ATTACHED_MODELS];       /* +0xB4 */
	int attachTagIndex[MAX_ATTACHED_MODELS];         /* +0xCC */
	char name[32];                  /* +0xE4 script field "name", the raw userinfo name */
} clientSession_t;
G_ASSERT_SIZE( clientSession_t, 260 );

struct gclient_s {
	/* ps MUST be the first element, because the server expects it */
	playerState_t ps;               /* +0x0000 communicated by the server to clients */

	/* the rest of the structure is private to the game */
	clientSession_t sess;           /* +0x20D0 */

	int spectatorClient;            /* +0x21D4 client being followed (ClientDisconnect 0x2001A460) */
	qboolean noclip;                /* +0x21D8 */
	qboolean ufo;                   /* +0x21DC */
	qboolean frozen;                /* +0x21E0 PlayerCmd_FreezeControls 0x2001CD00 */

	int lastCmdTime;                /* +0x21E4 ClientThink 0x200181BB writes it, ClientEndFrame 0x20018D21 tests it */
	int buttons;                    /* +0x21E8 */
	int oldbuttons;                 /* +0x21EC */
	int latched_buttons;            /* +0x21F0 */
	int wbuttons;                   /* +0x21F4 */
	int oldwbuttons;                /* +0x21F8 */
	int latched_wbuttons;           /* +0x21FC */
	vec3_t oldOrigin;               /* +0x2200 */

	byte unknown_0x220C[8];
	int damage_blood;               /* +0x2214 P_DamageFeedback 0x20017179 */
	vec3_t damage_from;             /* +0x2218 origin for vector calculation */
	qboolean damage_fromWorld;      /* +0x2224 */
	byte unknown_0x2228[12];
	int inactivityTime;             /* +0x2234 ClientSpawn sets level.time + 1000 * g_inactivity */
	qboolean inactivityWarning;     /* +0x2238 ClientInactivityTimer 0x20017790 */
	byte unknown_0x223C[36];        /* +0x2240 is the spread lerp fraction (FireWeapon 0x2003D1D1); +0x224C is cleared every ClientEndFrame and never read */
	gentity_t       *lookatEnt;     /* +0x2260 ScrCmd_IsLookingAt 0x2001CED6; cleared by G_FreeEntityRefs */
	byte unknown_0x2264[4];
	int pingTime;                   /* +0x2268 PlayerCmd_pingPlayer 0x2001B450 */
	/* The saved view and weapon angle state bg_weapon.c works in, marshalled
	   in and out by ClientThink_real (0x20017D1B..0x20017FB2). */
	int viewKickStartTime;          /* +0x226C */
	float viewKickYaw;              /* +0x2270 */
	float viewKickPitch;            /* +0x2274 */
	vec3_t previousViewAngles;      /* +0x2278 Sway reads/updates it; the delta drives the sway */
	vec3_t swayOffsets;             /* +0x2284 */
	vec3_t swayAngles;              /* +0x2290 the smoothed sway angle; ClientThink hands it to BG_CalculateWeaponAngles as baseAngles */
	vec3_t moveOffset;              /* +0x229C */
	float idleScale;                /* +0x22A8 */
	float recoilPitch;              /* +0x22AC */
	float recoilYaw;                /* +0x22B0 */
	float recoilRoll;               /* +0x22B4 */
	float recoilPitchSpeed;         /* +0x22B8 */
	float recoilYawSpeed;           /* +0x22BC */
	int recoilState;                /* +0x22C0 */
};
G_ASSERT_SIZE( gclient_t, 8900 );

/*
 * One entry of level.notifyWatch: a trigger/toucher pair queued by
 * G_TouchTriggers and drained by G_RunFrame (0x20026B20), which validates
 * both ends against gentity_t.spawnCount before firing the notify.
 * The type name and every member name are inferred; none is recovered.
 */
typedef struct notifyWatch_t {
	unsigned short entNum;          /* +0x00 the trigger */
	unsigned short otherEntNum;     /* +0x02 the entity that touched it */
	int spawnCount;                 /* +0x04 gentity_t.spawnCount of entNum when queued */
	int otherSpawnCount;            /* +0x08 gentity_t.spawnCount of otherEntNum when queued */
} notifyWatch_t;
G_ASSERT_SIZE( notifyWatch_t, 12 );

/*
 * Cleared as each map is entered: G_InitGame memsets exactly 0x2A14 bytes
 * from &level.
 */
typedef struct level_locals_s {
	gclient_t       *clients;       /* +0x0000 [maxclients] */

	gentity_t       *gentities;     /* +0x0004 */
	int gentitySize;                /* +0x0008 no CoD 1.1 access found */
	int num_entities;               /* +0x000C */

	gentity_t       *freeListHead;  /* +0x0010 G_Spawn / G_FreeEntity free list */
	gentity_t       *freeListTail;  /* +0x0014 */

	int logFile;                    /* +0x0018 */
	qboolean spawning;              /* +0x001C 1 while G_InitGame is running */

	objective_t objectives[MAX_OBJECTIVES];         /* +0x0020 */

	int maxclients;                 /* +0x01E0 */
	int framenum;                   /* +0x01E4 */
	int time;                       /* +0x01E8 in msec */
	int previousTime;               /* +0x01EC so movers can back up when blocked */
	int frameTime;                  /* +0x01F0 */
	int startTime;                  /* +0x01F4 level.time the map was spawned */

	int teamScores[TEAM_NUM_TEAMS]; /* +0x01F8 GScr_SetTeamScore 0x20034140 */
	int lastTeamLocationTime;       /* +0x0208 CheckTeamStatus 0x20039AC0 */
	int scoreboardChanged;          /* +0x020C CalculateRanks 0x200264B4 */
	int clientNameMode;             /* +0x0210 GScr_SetClientNameMode: 1 manual_change, 0 auto_change */
	int numConnectedClients;        /* +0x0214 CalculateRanks 0x200264C0 */
	int sortedClients[MAX_CLIENTS]; /* +0x0218 sorted by score */

	char voteString[MAX_STRING_CHARS];              /* +0x0318 */
	char voteDisplayString[MAX_STRING_CHARS];       /* +0x0718 */
	int voteTime;                   /* +0x0B18 level.time vote was called */
	int voteExecuteTime;            /* +0x0B1C time the vote is executed */
	int voteYes;                    /* +0x0B20 */
	int voteNo;                     /* +0x0B24 */
	int numVotingClients;           /* +0x0B28 set by CalculateRanks */

	/* No CoD 1.1 access found for the team-vote block; the RTCW/UO fields
	   fill it exactly (0x818 bytes) and the ordering follows the ancestor. */
	char teamVoteString[2][MAX_STRING_CHARS];       /* +0x0B2C */
	int teamVoteTime[2];            /* +0x132C */
	int teamVoteYes[2];             /* +0x1334 */
	int teamVoteNo[2];              /* +0x133C */

	qboolean spawningMapEntities;   /* +0x1344 set around the G_CallSpawn loop */
	int numSpawnVars;               /* +0x1348 */
	char            *spawnVars[MAX_SPAWN_VARS][2];  /* +0x134C key / value pairs */
	int numSpawnVarChars;           /* +0x154C */
	char spawnVarChars[MAX_SPAWN_VARS_CHARS];       /* +0x1550 */

	int matchState;                 /* +0x1D50 vmMain export 15 sets it, 16 returns it */
	qboolean locationLinked;        /* +0x1D54 target_location_linkup 0x20039590 */
	gentity_t       *locationHead;  /* +0x1D58 head of the target_location chain */
	int itemCue[32];                /* +0x1D5C walked by GetFreeCueSpot 0x20023890, filled by LaunchItem 0x20023A71 */

	float fogOpaqueDist;            /* +0x1DDC G_setfog 0x2001D630 */
	float fogOpaqueDistSq;          /* +0x1DE0 */
	byte unknown_0x1DE4[4];
	int playerCloneCursor;          /* +0x1DE8 G_SpawnPlayerClone 0x2003C040 */

	notifyWatch_t notifyWatch[MAX_NOTIFY_WATCHES];  /* +0x1DEC */
	int numNotifyWatches;           /* +0x29EC */

	int exitRequest;                /* +0x29F0 0 none, 1 map_restart, 2 exitLevel */
	int radiusDamageIgnorePlayers;  /* +0x29F4 G_RadiusDamage 0x20020D66, set and cleared by GScr_RadiusDamage */
	int scriptIgnoreRadiusDamage;   /* +0x29F8 the entity GScr_RadiusDamage excludes */

	/* The cvar values G_RunFrame (0x20026CB0..0x20026E2E) compares against
	   before re-pushing the player bounds and view heights. */
	float playerSizeWidth;          /* +0x29FC */
	float playerSizeHeight;         /* +0x2A00 */
	float viewHeightStanding;       /* +0x2A04 */
	float viewHeightCrouched;       /* +0x2A08 */
	float viewHeightProne;          /* +0x2A0C */
	int registeredItemsDirty;       /* +0x2A10 forces SaveRegisteredItems */
} level_locals_t;
G_ASSERT_SIZE( level_locals_t, 0x2A14 );

extern level_locals_t level;             /* 0x202C2920 */
extern gentity_t g_entities[MAX_GENTITIES];      /* 0x2016F220 */
extern gclient_t g_clients[MAX_CLIENTS];         /* 0x20237700 */

/*
 * Prototypes, grouped by the source file that defines them -- RTCW's g_local.h
 * is laid out the same way.
 */

//
// g_main_mp.c
//
int vmMain( int command, int arg0, int arg1, int arg2, int arg3 );
void G_Printf( const char *fmt, ... );
void G_DPrintf( const char *fmt, ... );
void G_Error( const char *fmt, ... );
void G_Error_Localized( const char *fmt, ... );
void G_CheckForCursorHints( gentity_t *ent );
void G_CheckForPreventFriendlyFire( gentity_t *ent );
void G_FindTeams( void );
void G_RegisterCvars( void );
void G_UpdateCvars( void );
void G_FreeEntities( void );
void G_InitGame( int levelTime, int randomSeed, int restart, int matchState );
void G_ShutdownGame( int restart );
void Com_Error( int level, const char *error, ... );
void Com_Printf( const char *msg, ... );
void Com_DPrintf( const char *msg, ... );
void SendScoreboardMessageToAllIntermissionClients( void );
int SortRanks( const void *a, const void *b );
void CalculateRanks( void );
void ExitLevel( void );
void G_LogPrintf( const char *fmt, ... );
void CheckVote( void );
void G_UpdateObjectiveToClients( void );
void G_UpdateHudElemsToClients( void );
void G_RunThink( gentity_t *ent );
void DebugDumpAnims( void );
void G_XAnimUpdateEnt( gentity_t *ent );
void G_XAnimUpdate( void );
void G_RunFrameForEntity( gentity_t *ent );
void G_RunFrame( int levelTime );

/* G_GetActivateEnt and G_CompareActivateEntScores are left out: their
   activateEnt_t is private to g_main_mp.c. */

/* forward: g_syscalls_mp.c -- the traps this unit issues. */
void trap_Printf( const char *fmt );
void trap_Error( const char *fmt );
void trap_ErrorNoPrefix( const char *fmt );  /* inferred */
int trap_Milliseconds( void );
int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
int trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void trap_FS_FCloseFile( fileHandle_t f );
void trap_SendConsoleCommand( int exec_when, const char *text );
void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void trap_Cvar_Update( vmCvar_t *cvar );
void trap_Cvar_Set( const char *var_name, const char *value );
void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						  playerState_t *clients, int sizeofGameClient );
void trap_SendServerCommand( int clientNum, int reliable, const char *command );
void trap_SetConfigstring( int num, const char *string );
void trap_GetConfigstring( int num, char *buffer, int bufferSize );
void trap_GetServerinfo( char *buffer, int bufferSize );
void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
				 const vec3_t end, int passEntityNum, int contentmask );
void trap_LocationalTrace( trace_t *results, const vec3_t start, const vec3_t end,
						   int passEntityNum, int contentmask, const byte *dobjTracePartState );
void trap_LinkEntity( gentity_t *ent );
void trap_UnlinkEntity( gentity_t *ent );
int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList,
						int maxcount, int contentmask );
int trap_FindSoundAlias( const char *name );  /* inferred */
int trap_DObjUpdateServerTime( gentity_t *ent, float serverTime, int stopOnNotetrack );
void trap_DObjInitServerTime( gentity_t *ent, float serverTime );
void trap_DObjDisplayAnim( gentity_t *ent );
void trap_FreeWeaponInfoMemory( int iSource );
void trap_FreeClientScriptPers( void );

/* forward: q_shared.c / com_shared.c */
void Swap_Init( void );
int Com_sprintf( char *dest, int size, const char *fmt, ... );
char *va( const char *format, ... );
void Info_SetValueForKey( char *s, const char *key, const char *value );
qboolean Com_BitCheck( const int array[], int bitNum );
extern int com_randSeed;

/* forward: universal/com_math.c -- the DESTINATION is the middle argument.
   0x0042FB50 and both DLL copies pass a in eax, out in edx and b in ecx; see
   G_DObjSetLocalTagInternal 0x2003B887 / 0x2003B896. */
void QuatMultiply( const float a[4], float out[4], const float b[4] );

/* forward: bg_misc.c -- the player bounding box the game module pushes out. */
extern vec3_t playerMins;
extern vec3_t playerMaxs;

/* forward: bg_weapons.c / bg_animation.c */
void BG_SetupWeaponInfo( void );
qboolean BG_CanItemBeGrabbed( const entityState_t *ent, const playerState_t *ps, int arg );

/* forward: g_client_mp.c */
const char *ClientConnect( int clientNum, unsigned int scriptId );
void ClientBegin( int clientNum );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
int GetFollowPlayerState( int clientNum, playerState_t *ps );

/* forward: g_active_mp.c */
void ClientThink( int clientNum );
void ClientEndFrame( gentity_t *ent );
void G_RunClient( gentity_t *ent );

/* forward: g_cmds_mp.c */
void ClientCommand( int clientNum );
void DeathmatchScoreboardMessage( gentity_t *ent );

/* forward: g_svcmds_mp.c */
int ConsoleCommand( void );
void G_ProcessIPBans( void );

/* forward: g_spawn_mp.c */
qboolean G_ParseSpawnVars( void );
void SP_worldspawn( void );
void G_CallSpawn( void );

/* forward: g_utils_mp.c (G_FreeEntity, G_DObjCalcPose, G_DObjGetWorldTagMatrix
   and G_AnimScriptSound are already declared in that unit's own section). */
void G_GeneralLink( gentity_t *ent );

/* forward: g_items_mp.c */
void ClearRegisteredItems( void );
void SaveRegisteredItems( void );
void G_RunItem( gentity_t *ent );

/* forward: g_mover_mp.c / g_missile_mp.c / g_combat_mp.c */
void G_RunMover( gentity_t *ent );
void G_RunMissile( gentity_t *ent );
void G_ParseHitLocDmgTable( void );

void G_InitTurrets( void );
qboolean G_IsTurretUsable( gentity_t *turret, gentity_t *player );

/* forward: g_trigger_mp.c / g_team_mp.c / g_hud_mp.c / g_misc_mp.c */
void G_Trigger( gentity_t *trigger, gentity_t *other );
void CheckTeamStatus( void );
/* HudElem_DestroyAll and HudElem_UpdateClient are declared under the
   g_hud_mp.c marker; HudElem_UpdateClient takes a third argument (edx = 3 at
   0x2002688B in G_UpdateHudElemsToClients). */
/* g_setfog belongs to g_cmds_mp.c and takes the fog configstring; G_InitGame
   passes the literal "0" (0x200260E1).  See that unit's section below. */
void G_setfog( const char *fogString );

/* forward: g_scr_main_mp.c */
void *Scr_FarHook( void *engineCallbacks );
void Scr_ParseGameTypeList( void );
/* Takes the gametype name in ebx and returns that gametype's display name out
   of the parsed list, NULL when it is not one (0x200351D0); Cmd_CallVote_f
   uses the result both as the test and as the "%s" of the vote string. */
const char *Scr_IsValidGameType( const char *gameType );
void GScr_LoadScripts( void );
void GScr_LoadConsts( void );

//
// g_syscalls_mp.c
//
void dllEntry( int ( QDECL *syscallptr )( int arg,... ) );
float IntAsFloat( int x );
int PASSFLOAT( float x );
void trap_Printf( const char *fmt );
void trap_Error( const char *fmt );
void trap_ErrorNoPrefix( const char *fmt );  /* inferred */
int trap_Milliseconds( void );
int trap_Argc( void );
void trap_Argv( int n, char *buffer, int bufferLength );
void *trap_Hunk_AllocInternal( int size );  /* inferred */
void *trap_Hunk_AllocLowInternal( int size );
void *trap_Hunk_AllocAlignInternal( int size, int align );  /* inferred */
void *trap_Hunk_AllocLowAlignInternal( int size, int align );
void *trap_Hunk_AllocateTempMemoryInternal( int size );  /* inferred */
void trap_Hunk_FreeTempMemoryInternal( void *buf );  /* inferred */
int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void trap_FS_Read( void *buffer, int len, fileHandle_t f );
int trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void trap_FS_Rename( const char *from, const char *to );  /* inferred */
void trap_FS_FCloseFile( fileHandle_t f );
int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
void trap_SendConsoleCommand( int exec_when, const char *text );
void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
void trap_Cvar_Update( vmCvar_t *cvar );
void trap_Cvar_Set( const char *var_name, const char *value );
int trap_Cvar_VariableIntegerValue( const char *var_name );
float trap_Cvar_VariableValue( const char *var_name );
void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGameClient );
void trap_DropClient( int clientNum, const char *reason );
void trap_SendServerCommand( int clientNum, int reliable, const char *command );
void trap_SetConfigstring( int num, const char *string );
void trap_GetConfigstring( int num, char *buffer, int bufferSize );
const char *trap_GetConfigstringConst( int num );
void trap_GetUserinfo( int num, char *buffer, int bufferSize );
void trap_SetUserinfo( int num, const char *buffer );  /* inferred */
void trap_GetServerinfo( char *buffer, int bufferSize );
void trap_SetBrushModel( gentity_t *ent );
void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
void trap_SightTrace( int *hitOut, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int passOwnerNum, int contentmask );  /* inferred */
void trap_SightTraceCapsule( int *hitOut, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int passOwnerNum, int contentmask );  /* inferred */
int trap_SightTraceToEntity( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentmask );
void trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );  /* inferred */
void trap_CM_BoxTrace2( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );  /* inferred */
int trap_CM_BoxSightTrace( const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );  /* inferred */
int trap_CM_BoxSightTrace2( const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );  /* inferred */
void trap_LocationalTrace( trace_t *results, const vec3_t start, const vec3_t end, int passEntityNum, int contentmask, const byte *dobjTracePartState );
int trap_PointContents( const vec3_t point, int passEntityNum, int contentmask );
int trap_InPVS( const vec3_t p1, const vec3_t p2 );
int trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );  /* inferred */
int trap_InSnapshot( const vec3_t origin, int entityNum );
void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open );
int trap_AreasConnected( int area1, int area2 );  /* inferred */
void trap_LinkEntity( gentity_t *ent );
void trap_UnlinkEntity( gentity_t *ent );
int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount, int contentmask );
int trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
int trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
void trap_GetUsercmd( int clientNum, usercmd_t *cmd );
qboolean trap_GetEntityToken( char *buffer, int bufferSize );
void trap_AddDebugString( const vec3_t origin, const vec3_t color, float scale, const char *text );
void trap_AddDebugLine( const vec3_t start, const vec3_t end, const vec3_t color, int depthTest, int duration );
void trap_SetArchive( qboolean enable );
int trap_RealTime( void *qtime );  /* inferred */
void trap_SnapVector( float *v );
int trap_FindSoundAlias( const char *name );  /* inferred */
int trap_PickSoundAlias( const char *name );  /* inferred */
int trap_SoundAliasIndex( void *alias );  /* inferred */
int trap_SurfaceTypeFromName( const char *name );  /* inferred */
const char *trap_SurfaceTypeToName( int surfaceType );
void *trap_Z_MallocInternal( int size );  /* inferred */
void trap_Z_Free( void *ptr );  /* inferred */
int trap_AddTestClient( void );
qboolean trap_GetArchivedClientInfo( int clientNum, int *frameTime, void *clientInfo );
void *trap_XAnimCreateTree( void *anims );  /* inferred */
void *trap_XAnimCreateSmallTree( void *anims );  /* inferred */
void trap_XAnimFreeSmallTree( void *tree );  /* inferred */
int trap_XModelExists( const char *name );  /* inferred */
void *trap_XModelGet( const char *name );
void trap_DObjCreate( void *models, unsigned short modelCount, void *tree, int handle, unsigned short scrNotifyId );
void trap_SafeDObjFree( int handle, unsigned int releaseTree );
qboolean trap_DObjExists( gentity_t *ent );
int trap_DerefDword( const int *p );  /* inferred */
scr_anim_t trap_XAnimGetRootAnim( const int *tree );  /* inferred */
void trap_XAnimClearTreeGoalWeights( void *tree, unsigned short animIndex, float goalWeight );  /* inferred */
void trap_XAnimClearGoalWeight( void *tree, unsigned short animIndex, float goalWeight );
void trap_XAnimClearTreeGoalWeightsStrict( void *tree, unsigned short animIndex, float goalWeight );
void trap_XAnimSetAnimKnob( void *tree, unsigned short animIndex, float goalWeight, int goalTime, float rate, unsigned short notifyIndex, int notifyFlags );  /* inferred */
void trap_XAnimSetCompleteGoalWeightKnobAll( void *tree, unsigned short animIndex, unsigned short childIndex, float goalWeight, float goalTime, float rate, unsigned short notifyIndex, int notifyFlags );
void trap_XAnimSetAnimRate( void *tree, unsigned short animIndex, float rate );
void trap_XAnimSetTime( void *tree, unsigned short animIndex, float time );
void trap_XAnimSetAnimLimited( void *tree, unsigned short animIndex, float goalWeight, int goalTime, float rate, unsigned short notifyIndex, int notifyFlags );  /* inferred */
void trap_XAnimClearTree( void *tree );  /* inferred */
int trap_XAnimIsLeaf( scr_anim_t anim );  /* inferred */
int trap_XAnimIsPrimitive( scr_anim_t anim );
int trap_XAnimGetLength( void *tree, unsigned short animIndex );
float trap_XAnimGetLengthSeconds( scr_anim_t anim );
void trap_XAnimSetCompleteGoalWeight( void *tree, unsigned short animIndex, float goalWeight, float goalTime, float rate, unsigned short notifyIndex, int notifyFlags );
void trap_XAnimSetGoalWeight( void *tree, unsigned short animIndex, float goalWeight, float goalTime, float rate, unsigned short notifyIndex, int notifyFlags );
void trap_XAnimCalcAbsDelta( void *tree, unsigned short animIndex, float *deltaRot, float *deltaMove );
void trap_XAnimCalcRelDelta( void *tree, unsigned short animIndex, float *deltaPos, void *deltaQuat, int animIndexEnd );  /* inferred */
void trap_XAnimGetRelDelta( scr_anim_t anim, float *deltaRot, float *deltaMove, float startTime, float endTime );
void trap_XAnimGetAbsDelta( scr_anim_t anim, float *deltaPos, void *deltaQuat, float time );  /* inferred */
int trap_XAnimIsLooped( scr_anim_t anim );
int trap_XAnimNotetrackExists( scr_anim_t anim, unsigned short notetrack );
float trap_XAnimGetTime( void *tree, unsigned short animIndex );
float trap_XAnimGetWeight( void *tree, unsigned short animIndex );
void trap_DObjDumpInfo( gentity_t *ent );
qboolean trap_DObjCreateSkelForBone( gentity_t *ent, int boneIndex );
qboolean trap_DObjCreateSkelForBones( gentity_t *ent, const int *partBits );
int trap_DObjUpdateServerTime( gentity_t *ent, float serverTime, int stopOnNotetrack );
void trap_DObjInitServerTime( gentity_t *ent, float serverTime );
void trap_DObjGetHierarchyBits( gentity_t *ent, int boneIndex, int *partBits );
int trap_DObjCalcAnim( gentity_t *ent, void *partBits );
int trap_DObjCalcSkel( gentity_t *ent, void *partBits );
void trap_XAnimRestoreTree( int tree );  /* inferred */
void trap_XAnimSaveTree( void *tree );  /* inferred */
void trap_XAnimCopyTree( const void *from, void *to );  /* inferred */
int trap_DObjNumParts( gentity_t *ent );  /* inferred */
int trap_DObjGetBoneIndex( gentity_t *ent, const char *boneName );
void *trap_DObjGetMatrixArray( gentity_t *ent );
void trap_DObjDisplayAnim( gentity_t *ent );
int trap_XAnimHasTime( void *tree, unsigned short animIndex );  /* inferred */
int trap_XAnimGetNumChildren( scr_anim_t anim );
scr_anim_t trap_XAnimGetChildAt( scr_anim_t anim, int childIndex );
int trap_XModelNumBones( void *model );
unsigned short *trap_XModelGetBoneNames( void *model );
void *trap_DObjGetRotTransArray( gentity_t *ent );
qboolean trap_DObjSetRotTransIndex( gentity_t *ent, const int *partBits, int boneIndex );
qboolean trap_DObjSetControlRotTransIndex( gentity_t *ent, const int *partBits, int boneIndex );
const char *trap_XAnimGetAnimName( scr_anim_t anim );
void *trap_DObjGetTree( gentity_t *ent );  /* inferred */
int trap_XAnimGetAnimTreeSize( void *tree );
void trap_XModelDebugBoxes( gentity_t *ent );
void *trap_GetWeaponInfoMemory( int size, int *pPrevOwner );
void trap_FreeWeaponInfoMemory( int iSource );
void trap_FreeClientScriptPers( void );
void trap_ResetEntityParsePoint( void );  /* inferred */

//
// g_utils_mp.c
//
/*
 * A 4x3 transform: three axis rows then the translation.  The stack layouts of
 * G_CalcTagParentAxis (0x2003B2C0) and G_SetFixedLink (0x2003B530) both put
 * the origin at +0x24.  DObjModel is bg_public.h's -- bg_animation.c builds
 * the same model list.
 */
typedef struct matrix43_s {
	vec3_t axis[3];
	vec3_t origin;
} matrix43_t;

/* One entry of the DObj rot/trans array, stride 32 (0x2003B7A5). */
typedef struct DObjAnimMat_s {
	vec4_t quat;                            /* +0x00 */
	float accumulatedWeight;                /* +0x10 */
	vec3_t translation;                     /* +0x14 */
} DObjAnimMat;

/*
 * gentity_t.linkInfo.  112 bytes, the size G_EntLinkToInternal hands MT_Alloc
 * (0x2003AFB3) and MT_Free (0x2003B121).  Layout: G_EntLinkToWithOffset
 * (0x2003B077) and G_CalcTagParentAxis (0x2003B2E8).
 */
struct entityLinkInfo_s {
	gentity_t       *parent;                /* +0x00 */
	gentity_t       *nextChild;             /* +0x04 next sibling in parent->firstChild */
	unsigned short tagName;                 /* +0x08 const string id, 0 for an untagged link */
	int parentTagIndex;                     /* +0x0C bone index in the parent's DObj, -1 for none */
	matrix43_t relAxis;                     /* +0x10 */
	matrix43_t parentRelAxis;               /* +0x40 */
};
G_ASSERT_SIZE( entityLinkInfo_t, 112 );

int         G_FindConfigstringIndex( const char *name, int start, int max, qboolean create,
									 const char *fieldname );
int         G_LocalizedStringIndex( const char *name );
int         G_ShaderIndex( const char *name );
int         G_ModelIndex( const char *name );
const char  *G_ModelName( int modelIndex );
int         G_TagIndex( const char *name );
int         G_EffectIndex( const char *name );
int         G_ShellShockIndex( const char *name );
byte        G_SoundAliasIndex( const char *name );
unsigned short G_GetGameId( gentity_t *ent );
void        G_DObjUpdate( gentity_t *ent );
void        G_SetModel( gentity_t *ent, const char *modelName );
qboolean    G_EntAttach( gentity_t *ent, const char *modelName, const char *tagName );
qboolean    G_EntDetach( gentity_t *ent, const char *modelName, const char *tagName );
void        G_EntDetachAll( gentity_t *ent );
qboolean    G_EntLinkTo( gentity_t *child, gentity_t *parent, const char *tagName );
qboolean    G_EntLinkToWithOffset( gentity_t *child, gentity_t *parent, const char *tagName,
								   const vec3_t originOffset, const vec3_t anglesOffset );
void        G_EntUnlink( gentity_t *ent );
qboolean    G_EntIsLinkedTo( gentity_t *child, gentity_t *parent );
void        G_UpdateTagInfo( gentity_t *ent, qboolean updateBoneIndex );
void        G_UpdateTagInfoOfChildren( gentity_t *parent, qboolean updateBoneIndex );
void        G_CalcTagParentAxis( gentity_t *child, matrix43_t *outAxis );
void        G_CalcTagParentRelAxis( gentity_t *child, matrix43_t *outAxis );
void        G_CalcTagAxis( gentity_t *ent, int useLinkedAngles );
void        G_SetFixedLink( gentity_t *ent, int mode );
void        G_GeneralLink( gentity_t *ent );
void        Think_GeneralLink( gentity_t *ent );
void        G_SafeDObjFree( gentity_t *ent );
int         G_DObjUpdateServerTime( gentity_t *ent, int stopOnNotetrack );
qboolean    G_DObjSetLocalTag( gentity_t *ent, int *partBits, const char *tagName,
							   const vec3_t origin, const vec3_t angles );
qboolean    G_DObjSetControlTagAngles( gentity_t *ent, int *partBits, const char *tagName,
									   const vec3_t angles );
void        G_DObjCalcPose( gentity_t *ent );
void        G_DObjCalcBone( gentity_t *ent, int boneIndex );
float       *G_DObjGetLocalTagMatrix( gentity_t *ent, const char *tagName );
qboolean    G_DObjGetWorldTagMatrix( gentity_t *ent, const char *tagName, void *outMatrix );
gentity_t   *G_Find( gentity_t *from, int fieldofs, unsigned short match );
gentity_t   *G_FindStr( gentity_t *from, int fieldofs, const char *match );
gentity_t   *G_PickTarget( unsigned short targetname );
char        *vtos( const vec3_t v );
char        *vtosf( const vec3_t v );
void        G_SetMovedir( vec3_t angles, vec3_t movedir );
void        G_InitGentity( gentity_t *e );
gentity_t   *G_Spawn( void );
gentity_t   *G_SpawnPlayerClone( void );
void        G_FreeEntityRefs( gentity_t *ent );
void        G_FreeEntity( gentity_t *ent );
gentity_t   *G_TempEntity( const vec3_t origin, int event );
void        G_KillBox( gentity_t *ent );
void        G_AddPredictableEvent( gentity_t *ent, byte event, byte eventParm );
void        G_AddEvent( gentity_t *ent, int event, int eventParm );
void        G_PlaySoundAliasAtPoint( const vec3_t origin, byte alias );
void        G_PlaySoundAlias( gentity_t *ent, byte alias );
void        G_AnimScriptSound( int client, const char *name );
void        G_SetOrigin( gentity_t *ent, const vec3_t origin );
void        G_SetAngle( gentity_t *ent, const vec3_t angles );
qboolean    infront( gentity_t *self, gentity_t *other );
int         DebugLine( const vec3_t start, const vec3_t end, int color );
void        G_SetConstString( unsigned short *dest, const char *string );

//
// g_spawn_mp.c
//

/*
 * The game-side hudelem record, 124 bytes.  The first 112 bytes are the
 * networked hudelem_t of q_shared.h; that header still carries the record as
 * `byte opaque[112]`, so its fields are spelled out here.  The last three
 * words never reach a client --
 * HudElem_UpdateClient (0x20022760) copies exactly 0x70 bytes.
 *
 * Offsets: the field table at 0x200558D0 pins x, y, fontScale, font, alignX,
 * alignY, color, label, sortKey and archived; HudElem_SetDefaults (0x20021410),
 * HECmd_SetShader, HECmd_SetTimer_Internal, HECmd_SetClock_Internal,
 * HECmd_ScaleOverTime and HECmd_MoveOverTime pin the rest.  The stride is the
 * /124 in Scr_AddHudElem (0x20037750).  Member names are inferred from
 * United Offensive's copy of the same record; CoD 1.1 carries no symbols for
 * them.
 * owner: g_hud_mp.c
 */
typedef struct g_hudelem_s {
	int type;                       /* +0x00 hudElemType_t; 0 = free slot */
	int x;                          /* +0x04 */
	int y;                          /* +0x08 */
	float fontScale;                /* +0x0C */
	int font;                       /* +0x10 */
	int alignX;                     /* +0x14 */
	int alignY;                     /* +0x18 */
	int color;                      /* +0x1C packed rgba, byte 3 is the alpha */
	int fromColor;                  /* +0x20 fadeOverTime source colour */
	int fadeStartTime;              /* +0x24 */
	int fadeTime;                   /* +0x28 */
	int label;                      /* +0x2C localized string index */
	int width;                      /* +0x30 */
	int height;                     /* +0x34 */
	int materialIndex;              /* +0x38 shader index, setShader */
	int scaleFromWidth;             /* +0x3C */
	int scaleFromHeight;            /* +0x40 */
	int scaleStartTime;             /* +0x44 */
	int scaleTime;                  /* +0x48 */
	int moveFromX;                  /* +0x4C */
	int moveFromY;                  /* +0x50 */
	int moveStartTime;              /* +0x54 */
	int moveTime;                   /* +0x58 */
	int timerValue;                 /* +0x5C level.time the timer runs out */
	int rotationPeriodMs;           /* +0x60 setClock's total clock time */
	float value;                    /* +0x64 setValue */
	int text;                       /* +0x68 localized string index */
	float sortKey;                  /* +0x6C script field "sort" */

	/* server side only, never copied into ps.hud */
	int clientNum;                  /* +0x70 ENTITYNUM_NONE = shown to everyone */
	int team;                       /* +0x74 0 = shown to every team */
	qboolean archived;              /* +0x78 selects ps.hud.archival over .current */
} g_hudelem_t;
G_ASSERT_SIZE( g_hudelem_t, 124 );

qboolean G_SpawnString( const char *key, const char *defaultString, char **out );
qboolean G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean G_SpawnVector( const char *key, const char *defaultString, float *out );
void GScr_FieldError( gentity_t *ent, const char *error );
void G_ParseEntityFields( gentity_t *ent );
void G_DuplicateEntityFields( gentity_t *dest, gentity_t *source );
void G_DuplicateScriptFields( gentity_t *dest, gentity_t *source );
void G_CallSpawn( void );
qboolean G_CallSpawnEntity( gentity_t *ent );
void GScr_AddFieldsForEntity( void );
void GScr_AddFieldsForRadiant( void );
void Scr_SetEntityField( int entnum, unsigned int fieldnum );
void Scr_SetGenericField( void *base, int type, int ofs );
void Scr_SetObjectField( int classnum, int objectNum, int fieldnum );
void Scr_GetEntityField( int entnum, unsigned int fieldnum );
void Scr_GetGenericField( void *base, int type, int ofs );
void Scr_GetObjectField( int classnum, int objectNum, int fieldnum );
void Scr_FreeEntityConstStrings( gentity_t *ent );
void Scr_FreeEntity( gentity_t *ent );
void Scr_AddEntity( gentity_t *ent );
gentity_t *Scr_GetEntity( unsigned int num );
void Scr_FreeHudElem( g_hudelem_t *elem );
void Scr_AddHudElem( g_hudelem_t *elem );
g_hudelem_t *Scr_GetHudElem( unsigned int num );
unsigned short Scr_ExecEntThread( gentity_t *ent, int handle, unsigned int paramcount );
void Scr_AddExecEntThread( gentity_t *ent, int handle, unsigned int paramcount );
void Scr_Notify( gentity_t *ent, unsigned short event, unsigned int paramcount );
void Scr_getent( void );
void Scr_GetEntArray( void );
void GScr_SetDynamicEntityField( gentity_t *ent, int fieldnum );
void G_SpawnGEntityFromSpawnVars( void );
qboolean G_ParseSpawnVars( void );
void SP_worldspawn( void );
void G_SpawnEntitiesFromString( void );

/* forward: q_shared.c */
char *va( const char *format, ... );
int Q_stricmp( const char *s1, const char *s2 );

/* forward: g_main_mp.c */
void G_Printf( const char *fmt, ... );
void G_Error( const char *fmt, ... );
void Com_Error( int level, const char *fmt, ... );
extern vmCvar_t g_motd;                          /* 0x202374C0 */

/* forward: g_utils_mp.c */
gentity_t *G_Spawn( void );
int G_ModelIndex( const char *name );
const char *G_ModelName( int index );
void G_SetOrigin( gentity_t *ent, const vec3_t origin );
void G_SetAngle( gentity_t *ent, const vec3_t angle );

/* forward: g_items_mp.c */
void G_SpawnItem( gentity_t *ent, gitem_t *item );

/* forward: g_client_fields_mp.c */
void GScr_AddFieldsForClient( unsigned short classnum );

/* forward: g_hud_mp.c */
extern g_hudelem_t g_hudelems[];                 /* 0x2014F9A0 */
void Scr_SetHudElemField( int elemnum, int fieldnum );
void Scr_GetHudElemField( int elemnum, int fieldnum );
void Scr_FreeHudElemConstStrings( g_hudelem_t *elem );

void Scr_SetOrigin( gentity_t *ent, int fieldnum );
void Scr_SetAngles( gentity_t *ent, int fieldnum );
void Scr_SetHealth( gentity_t *ent, int fieldnum );

/* The classnames in spawns[] resolve to these; each lives in the unit named. */
void SP_info_null( gentity_t *ent );                    /* forward: g_misc_mp.c */
void SP_info_notnull( gentity_t *ent );                 /* forward: g_misc_mp.c */
void SP_light( gentity_t *ent );                        /* forward: g_misc_mp.c */
void SP_misc_teleporter_dest( gentity_t *ent );         /* forward: g_misc_mp.c */
void SP_misc_model( gentity_t *ent );                   /* forward: g_misc_mp.c */
void SP_corona( gentity_t *ent );                       /* forward: g_misc_mp.c */
void SP_turret( gentity_t *ent );                       /* forward: g_misc_mp.c */
void SP_misc_spawner( gentity_t *ent );                 /* forward: g_misc_mp.c */
void SP_func_door( gentity_t *ent );                    /* forward: g_mover_mp.c */
void SP_func_static( gentity_t *ent );                  /* forward: g_mover_mp.c */
void SP_func_rotating( gentity_t *ent );                /* forward: g_mover_mp.c */
void SP_func_bobbing( gentity_t *ent );                 /* forward: g_mover_mp.c */
void SP_func_pendulum( gentity_t *ent );                /* forward: g_mover_mp.c */
void SP_func_door_rotating( gentity_t *ent );           /* forward: g_mover_mp.c */
void trigger_use( gentity_t *ent );                     /* forward: g_mover_mp.c */
void SP_script_brushmodel( gentity_t *ent );            /* forward: g_scr_mover_mp.c */
void SP_script_model( gentity_t *ent );                 /* forward: g_scr_mover_mp.c */
void SP_script_origin( gentity_t *ent );                /* forward: g_scr_mover_mp.c */
void SP_target_location( gentity_t *ent );              /* forward: g_team_mp.c */
void SP_trigger_multiple( gentity_t *ent );             /* forward: g_trigger_mp.c */
void SP_trigger_hurt( gentity_t *ent );                 /* forward: g_trigger_mp.c */
void SP_trigger_once( gentity_t *ent );                 /* forward: g_trigger_mp.c */
void SP_trigger_damage( gentity_t *ent );               /* forward: g_trigger_mp.c */
void SP_trigger_lookat( gentity_t *ent );               /* forward: g_trigger_mp.c */

//
// g_client_mp.c
//
/* ClientCleanName is file-static there, as it is in RTCW. */
void        SetClientViewAngle( gentity_t *ent, const vec3_t angle );
void        ClientUserinfoChanged( int clientNum );
const char  *ClientConnect( int clientNum, unsigned int scriptId );
void        ClientBegin( int clientNum );
void        ClientSpawn( gentity_t *ent, const vec3_t origin, const vec3_t angles );
void        ClientDisconnect( int clientNum );
void        G_SetPlayerSize( void );
void        G_AddLean( gentity_t *ent, vec3_t pos );
int         G_GetNonPVSFriendlyInfo( gentity_t *ent, const vec3_t origin, int lastEntNum );

//
// g_client_fields_mp.c
//
/* clientField_t and fieldtype_t are declared in g_client_fields_mp.c itself
   (and again in g_spawn_mp.c), so the thirteen accessors clientFields[] holds
   are file-static there and only the three entry points are declared here. */
void GScr_AddFieldsForClient( unsigned short classnum );
void Scr_SetClientField( gclient_t *client, unsigned int fieldnum );
void Scr_GetClientField( gclient_t *client, unsigned int fieldnum );

int  GScr_GetStatusIconIndex( const char *name );
int  GScr_GetHeadIconIndex( const char *name );

//
// g_client_script_cmd_mp.c
//

/*
 * Every player script method has this shape: the VM hands it the number of the
 * entity the method was called on and nothing else.  The typedef name is
 * inferred -- the original's is not recovered.
 */
typedef void ( *PlayerCmd_t )( unsigned int entnum );

void        PlayerCmd_giveWeapon( unsigned int entnum );
void        PlayerCmd_takeWeapon( unsigned int entnum );
void        PlayerCmd_takeAllWeapons( unsigned int entnum );
void        PlayerCmd_getCurrentWeapon( unsigned int entnum );
void        PlayerCmd_hasWeapon( unsigned int entnum );
void        PlayerCmd_switchToWeapon( unsigned int entnum );
void        PlayerCmd_giveStartAmmo( unsigned int entnum );
void        PlayerCmd_giveMaxAmmo( unsigned int entnum );
void        PlayerCmd_getFractionStartAmmo( unsigned int entnum );
void        PlayerCmd_getFractionMaxAmmo( unsigned int entnum );
void        PlayerCmd_setOrigin( unsigned int entnum );
void        PlayerCmd_setAngles( unsigned int entnum );
void        PlayerCmd_useButtonPressed( unsigned int entnum );
void        PlayerCmd_attackButtonPressed( unsigned int entnum );
void        PlayerCmd_meleeButtonPressed( unsigned int entnum );
void        PlayerCmd_isOnGround( unsigned int entnum );
void        PlayerCmd_pingPlayer( unsigned int entnum );
void        PlayerCmd_SetViewmodel( unsigned int entnum );
void        PlayerCmd_GetViewmodel( unsigned int entnum );
void        PlayerCmd_allowComplaint( unsigned int entnum );
void        PlayerCmd_showScoreboard( unsigned int entnum );
void        PlayerCmd_setSpawnWeapon( unsigned int entnum );
void        PlayerCmd_dropItem( unsigned int entnum );
void        PlayerCmd_finishPlayerDamage( unsigned int entnum );
void        PlayerCmd_Suicide( unsigned int entnum );
void        PlayerCmd_OpenMenu( unsigned int entnum );
void        PlayerCmd_OpenMenuNoMouse( unsigned int entnum );
void        PlayerCmd_CloseMenu( unsigned int entnum );
qboolean    WeaponSlotsNotValid( gentity_t *ent );
void        PlayerCmd_GetWeaponSlotWeapon( unsigned int entnum );
void        PlayerCmd_SetWeaponSlotWeapon( unsigned int entnum );
void        PlayerCmd_GetWeaponSlotAmmo( unsigned int entnum );
void        PlayerCmd_SetWeaponSlotAmmo( unsigned int entnum );
void        PlayerCmd_GetWeaponSlotClipAmmo( unsigned int entnum );
void        PlayerCmd_SetWeaponSlotClipAmmo( unsigned int entnum );
void        iclientprintln( unsigned int entnum );
void        iclientprintlnbold( unsigned int entnum );
void        PlayerCmd_spawn( unsigned int entnum );
void        PlayerCmd_setEnterTime( unsigned int entnum );
void        BodyEnd( gentity_t *ent );
void        PlayerCmd_ClonePlayer( unsigned int entnum );
void        PlayerCmd_SetClientCvar( unsigned int entnum );
void        PlayerCmd_FreezeControls( unsigned int entnum );
void        PlayerCmd_SetReverb( unsigned int entnum );
void        ScrCmd_IsLookingAt( unsigned int entnum );
void        ScrCmd_PlayLocalSound( unsigned int entnum );
void        PlayerCmd_SayAll( unsigned int entnum );
void        PlayerCmd_SayTeam( unsigned int entnum );
PlayerCmd_t Player_GetMethod( const char **pName );

//
// g_active_mp.c
//
void        P_DamageFeedback( gentity_t *player );
void        G_SetClientSound( gentity_t *ent );
void        ClientImpacts( gentity_t *ent, pmove_t *pm );
void        G_TouchTriggers( gentity_t *ent );
void        SpectatorThink( gentity_t *ent, usercmd_t *ucmd );
qboolean    ClientInactivityTimer( gclient_t *client );
void        ClientIntermissionThink( gentity_t *ent );
void        ClientEvents( gentity_t *ent, int oldEventSequence );
void        G_SetClientContents( gentity_t *ent );
void        ClientThink_real( gentity_t *ent, usercmd_t *ucmd );
void        ClientThink( int clientNum );
void        G_RunClient( gentity_t *ent );
void        IntermissionClientEndFrame( gentity_t *ent );
void        SpectatorClientEndFrame( gentity_t *ent );
int         GetFollowPlayerState( int clientNum, playerState_t *ps );
qboolean    StuckInClient( gentity_t *ent );
void        G_PlayerController( gentity_t *self, unsigned int *partBits );
void        ClientEndFrame( gentity_t *ent );


//
// g_cmds_mp.c
//
void        DeathmatchScoreboardMessage( gentity_t *ent );
void        Cmd_Score_f( gentity_t *ent );
qboolean    CheatsOk( gentity_t *ent );
char        *ConcatArgs( int start );
int         ClientNumberFromString( gentity_t *to, char *s );
void        G_setfog( const char *fogString );
void        Cmd_Fogswitch_f( void );
void        Cmd_Give_f( gentity_t *ent );
void        Cmd_Take_f( gentity_t *ent );
void        Cmd_God_f( gentity_t *ent );
void        Cmd_Notarget_f( gentity_t *ent );
void        Cmd_Noclip_f( gentity_t *ent );
void        Cmd_UFO_f( gentity_t *ent );
void        Cmd_Kill_f( gentity_t *ent );
void        StopFollowing( gentity_t *ent );
qboolean    Cmd_FollowCycle_f( gentity_t *ent, int dir );
qboolean    G_IsPlaying( gentity_t *ent );
void        G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color,
					 const char *name, const char *message );
void        G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText );
void        Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 );
void        G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id,
					   qboolean voiceonly );
void        G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id,
					 qboolean voiceonly );
void        Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly );
void        Cmd_Where_f( gentity_t *ent );
void        Cmd_CallVote_f( gentity_t *ent );
void        Cmd_Vote_f( gentity_t *ent );
void        Cmd_SetViewpos_f( gentity_t *ent );
qboolean    Cmd_Activate_f( gentity_t *ent );
void        Cmd_EntityCount_f( gentity_t *ent );
void        Cmd_MenuResponse_f( gentity_t *ent );
void        ClientCommand( int clientNum );

/* forward: q_shared.c -- q_shared.h carries none of these yet. */
int         Q_stricmpn( const char *s1, const char *s2, int n );
void        Q_strncpyz( char *dest, const char *src, int destsize );
void        Q_CleanStr( char *string );
void        AddLeanToPosition( vec3_t pos, float yaw, float leanFrac,
							   float leanAngle, float leanDist );

/* forward: g_client_mp.c */
void        SetClientViewAngle( gentity_t *ent, const vec3_t angle );

/* forward: g_main_mp.c -- the cvars this unit reads. */
extern vmCvar_t sv_cheats;                              /* 0x20235E40 */
extern vmCvar_t dedicated;                              /* 0x20235300 */
extern vmCvar_t g_gametype;                             /* 0x20236CE0 */
extern vmCvar_t g_allowVote;                            /* 0x202350C0 */
extern vmCvar_t g_complaintlimit;                       /* 0x202C2800 */
extern vmCvar_t g_voiceChatsAllowed;                    /* 0x20234220 */

//
// g_combat_mp.c
//

/*
 * Means of death.  Every name is modNames[]'s own string (0x2006A510), and
 * G_IndexForMeansOfDeath (0x20020270) walks exactly MOD_NUM_MODS of them.
 */
typedef enum meansOfDeath_e {
	MOD_UNKNOWN             = 0x0,
	MOD_PISTOL_BULLET       = 0x1,
	MOD_RIFLE_BULLET        = 0x2,
	MOD_GRENADE             = 0x3,
	MOD_GRENADE_SPLASH      = 0x4,
	MOD_PROJECTILE          = 0x5,
	MOD_PROJECTILE_SPLASH   = 0x6,
	MOD_MELEE               = 0x7,
	MOD_HEAD_SHOT           = 0x8,
	MOD_MORTAR              = 0x9,
	MOD_MORTAR_SPLASH       = 0xA,
	MOD_KICKED              = 0xB,
	MOD_GRABBER             = 0xC,
	MOD_DYNAMITE            = 0xD,
	MOD_DYNAMITE_SPLASH     = 0xE,
	MOD_AIRSTRIKE           = 0xF,
	MOD_WATER               = 0x10,
	MOD_SLIME               = 0x11,
	MOD_LAVA                = 0x12,
	MOD_CRUSH               = 0x13,
	MOD_TELEFRAG            = 0x14,
	MOD_FALLING             = 0x15,
	MOD_SUICIDE             = 0x16,
	MOD_TRIGGER_HURT        = 0x17,
	MOD_EXPLOSIVE           = 0x18,

	MOD_NUM_MODS            = 0x19,
} meansOfDeath_t;

extern const char *modNames[MOD_NUM_MODS];

/*
 * Hit locations.  The names are hitLocationNames[]'s own strings (0x2006A498);
 * the count is the stride of the const-string table at 0x20089708 and the
 * length of the damage multiplier table at 0x2014F940.
 */
typedef enum hitLocation_e {
	HITLOC_NONE             = 0x0,
	HITLOC_HELMET           = 0x1,
	HITLOC_HEAD             = 0x2,
	HITLOC_NECK             = 0x3,
	HITLOC_TORSO_UPPER      = 0x4,
	HITLOC_TORSO_LOWER      = 0x5,
	HITLOC_R_ARM_UPPER      = 0x6,
	HITLOC_L_ARM_UPPER      = 0x7,
	HITLOC_R_ARM_LOWER      = 0x8,
	HITLOC_L_ARM_LOWER      = 0x9,
	HITLOC_R_HAND           = 0xA,
	HITLOC_L_HAND           = 0xB,
	HITLOC_R_LEG_UPPER      = 0xC,
	HITLOC_L_LEG_UPPER      = 0xD,
	HITLOC_R_LEG_LOWER      = 0xE,
	HITLOC_L_LEG_LOWER      = 0xF,
	HITLOC_R_FOOT           = 0x10,
	HITLOC_L_FOOT           = 0x11,
	HITLOC_GUN              = 0x12,

	HITLOC_NUM              = 0x13,
} hitLocation_t;

extern float g_fHitLocDamageMult[HITLOC_NUM];           /* 0x2014F940 */
extern unsigned short hitLocationConstStrings[HITLOC_NUM];      /* 0x20089708 */

/*
 * G_Damage's dflags.  1.1's game module never tests them -- they go straight
 * to the script as Callback_PlayerDamage's iDFlags -- so only the two the
 * module emits are known: DAMAGE_RADIUS (G_RadiusDamage 0x20020FAA) and
 * DAMAGE_PASSTHRU (Bullet_Fire_Extended 0x2003CAAC).  G_KillBox's 0x10 sits
 * in RTCW's DAMAGE_NO_TEAM_PROTECTION bit; the rest are RTCW's assignments.
 */
#define DAMAGE_RADIUS               0x00000001
#define DAMAGE_NO_ARMOR             0x00000002
#define DAMAGE_NO_KNOCKBACK         0x00000004
#define DAMAGE_NO_PROTECTION        0x00000008
#define DAMAGE_NO_TEAM_PROTECTION   0x00000010
#define DAMAGE_PASSTHRU             0x00000020

void        G_ParseHitLocDmgTable( void );
void        AddScore( gentity_t *ent, int score );
void        LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker );
int         G_IndexForMeansOfDeath( const char *name );
void        player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage,
						int mod, int weapon, const float *dir, int hitLoc );
void        G_DamageClient( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
							const vec3_t dir, const vec3_t point, int damage, int dflags,
							int mod, int hitLoc );
void        G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
					  const vec3_t dir, const vec3_t point, int damage, int dflags,
					  int mod, int hitLoc );
float       CanDamage( gentity_t *targ, const vec3_t origin );
qboolean    G_RadiusDamage( const vec3_t origin, gentity_t *inflictor, gentity_t *attacker,
							float damage, float minDamage, float radius, gentity_t *ignore,
							int mod );
unsigned short G_GetHitLocationString( int hitLoc );
int         G_GetHitLocationIndexFromString( unsigned short name );

//
// g_items_mp.c
//
extern qboolean itemRegistered[];                       /* 0x2016E9A0 */

void        Fill_Clip( playerState_t *ps, int weapon );
int         Add_Ammo( gentity_t *ent, int weapon, int count, qboolean fillClip );
int         Pickup_Ammo( gentity_t *ent, gentity_t *other );
int         Pickup_Weapon( gentity_t *ent, gentity_t *other, int *event, int touchMode );
int         Pickup_Health( gentity_t *ent, gentity_t *other );
void        RespawnItem( gentity_t *ent );
void        Touch_Item_Auto( gentity_t *ent, gentity_t *other, int touchMode );
void        Touch_Item( gentity_t *ent, gentity_t *other, int touchMode );
int         GetFreeCueSpot( void );
void        DroppedItemClearOwner( gentity_t *ent );
gentity_t   *LaunchItem( gitem_t *item, const vec3_t origin, const vec3_t velocity, int ownerNum );
gentity_t   *Drop_Item( gentity_t *ent, gitem_t *item, float angle, qboolean novelocity );
gentity_t   *Drop_Weapon( gentity_t *ent, int weapon, const char *tagName );
void        Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        FinishSpawningItem( gentity_t *ent );
void        ClearRegisteredItems( void );
void        SaveRegisteredItems( void );
void        RegisterItem( int itemIndex, qboolean updateConfigString );
qboolean    IsItemRegistered( int itemIndex );
void        G_SpawnItem( gentity_t *ent, gitem_t *item );
void        G_BounceItem( gentity_t *ent, trace_t *trace );
void        G_RunItem( gentity_t *ent );

//
// g_missile_mp.c
//
void        G_MissileLandAngles( gentity_t *ent, trace_t *trace, vec3_t angles,
								 qboolean forceAngles );
qboolean    G_BounceMissile( gentity_t *ent, trace_t *trace );
void        G_MissileImpact( gentity_t *ent, trace_t *trace );
void        Concussive_think( gentity_t *ent );
gentity_t   *Concussive_fx( const vec3_t origin );
void        G_ExplodeMissile( gentity_t *ent );
void        G_MissileDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
						  int damage, int mod, int weapon, const float *dir, int hitLoc );
void        G_RunMissile( gentity_t *ent );
gentity_t   *fire_grenade( gentity_t *self, const vec3_t start, const vec3_t dir, int weapon );
gentity_t   *fire_rocket( gentity_t *self, const vec3_t start, vec3_t dir );

//
// g_weapon_mp.c
//

/*
 * The block FireWeapon builds and hands down the whole fire path: three axis
 * vectors, the muzzle point and the weapon's info record.  It is one object,
 * not four locals -- Bullet_Endpos reads the muzzle at +0x24 off the pointer
 * it is given (0x2003C9CB) and Bullet_Fire_Extended the weaponInfo at +0x3C
 * (0x2003CA97).  The original's name and the 12 bytes at +0x30 are not
 * recovered; the size is the 0x40 bytes G_CheckForPreventFriendlyFire's
 * frame gives it (0x20025A26).
 */
struct weaponInfo_t;                    /* bg_public.h owns it */

typedef struct weaponFireInfo_s {
	vec3_t forward;                 /* +0x00 */
	vec3_t right;                   /* +0x0C */
	vec3_t up;                      /* +0x18 */
	vec3_t start;                   /* +0x24 muzzle point */
	byte unknown_0x30[12];
	const struct weaponInfo_t   *weapInfo;  /* +0x3C */
} weaponFireInfo_t;
G_ASSERT_SIZE( weaponFireInfo_t, 64 );

/* Hit-location priority maps for trap_LocationalTrace; 0x2006A4E4 and
   0x2006A4F8.  Both names inferred. */
extern const byte bulletPriorityMap[HITLOC_NUM];
extern const byte rifleBulletPriorityMap[HITLOC_NUM];

void        Weapon_Melee( weaponFireInfo_t *fireInfo, gentity_t *self );
void        SnapVectorTowards( vec3_t v, const vec3_t to );
void        Bullet_Endpos( const weaponFireInfo_t *fireInfo, vec3_t end, float spread );
void        Bullet_Fire( weaponFireInfo_t *fireInfo, gentity_t *self, gentity_t *attacker,
						 float spread, int damage );
void        Bullet_Fire_Extended( gentity_t *passEnt, gentity_t *attacker, vec3_t start,
								  vec3_t end, int damage, int depth,
								  const weaponFireInfo_t *fireInfo, gentity_t *owner );
gentity_t   *weapon_grenadelauncher_fire( const weaponFireInfo_t *fireInfo, gentity_t *self,
										  int weapon );
void        Weapon_RocketLauncher_Fire( gentity_t *ent, const weaponFireInfo_t *fireInfo,
										float spread );
qboolean    LogAccuracyHit( gentity_t *target, gentity_t *attacker );
void        CalcMuzzlePoint( gentity_t *ent, vec3_t muzzlePoint );
void        CalcMuzzlePoints( weaponFireInfo_t *fireInfo, gentity_t *ent );
void        FireWeapon( gentity_t *ent );
void        FireWeaponMelee( gentity_t *ent );

//
// g_mover_mp.c
//
gentity_t   *G_TestEntityPosition( gentity_t *ent, const vec3_t origin );
void        G_CreateRotationMatrix( const vec3_t angles, vec3_t matrix[3] );
void        G_TransposeMatrix( const vec3_t matrix[3], vec3_t transpose[3] );
void        G_RotatePoint( vec3_t point, const vec3_t matrix[3] );
qboolean    G_TryPushingEntity( gentity_t *check, gentity_t *pusher, vec3_t move, vec3_t amove );
qboolean    G_MoverPush( gentity_t *pusher, vec3_t move, vec3_t amove, gentity_t **obstacle );
void        G_MoverTeam( gentity_t *ent );
void        G_RunMover( gentity_t *ent );
void        SetMoverState( gentity_t *ent, moverState_t moverState, int time );
void        MatchTeam( gentity_t *teamLeader, int moverState, int time );
void        MatchTeamReverseAngleOnSlaves( gentity_t *teamLeader, int moverState, int time );
void        ReturnToPos1( gentity_t *ent );
void        ReturnToPos2( gentity_t *ent );
void        GotoPos3( gentity_t *ent );
void        ReturnToPos1Rotate( gentity_t *ent );
void        Reached_BinaryMover( gentity_t *ent );
qboolean    IsBinaryMoverBlocked( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        InitMover( gentity_t *ent );
void        InitMoverRotate( gentity_t *ent );
void        Blocked_Door( gentity_t *ent, gentity_t *other );
void        Blocked_DoorRotate( gentity_t *ent, gentity_t *other );
void        Touch_DoorTrigger( gentity_t *ent, gentity_t *other, int touchMode );
void        Think_SpawnNewDoorTriggerInternal( gentity_t *ent );
void        Think_SpawnNewDoorTrigger( gentity_t *ent );
void        DoorRotateStartOpen( gentity_t *ent );
void        Think_SpawnNewAutoDoorTrigger( gentity_t *ent );
void        Think_MatchTeam( gentity_t *ent );
void        finishSpawningKeyedMover( gentity_t *ent );
void        Door_reverse_sounds( gentity_t *ent );
void        DoorSetSounds( gentity_t *ent );
void        G_TryDoor( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        SP_func_door( gentity_t *ent );
void        Use_Static( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        Static_Pain( gentity_t *ent, gentity_t *attacker, int damage, const float *point,
						 int mod, const float *dir, int hitLoc );
void        SP_func_leaky( gentity_t *ent );
void        SP_func_static( gentity_t *ent );
void        Use_Func_Rotate( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        SP_func_rotating( gentity_t *ent );
void        SP_func_bobbing( gentity_t *ent );
void        SP_func_pendulum( gentity_t *ent );
void        SP_func_door_rotating( gentity_t *ent );
void        use_trigger_use( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        trigger_use( gentity_t *ent );
void        G_Activate( gentity_t *ent, gentity_t *activator );

//
// g_scr_mover_mp.c
//
void        Reached_ScriptMover( gentity_t *ent );
void        InitScriptMover( gentity_t *ent );
void        SP_script_brushmodel( gentity_t *ent );
void        SP_script_model( gentity_t *ent );
void        SP_script_origin( gentity_t *ent );
void        ScriptEntCmd_MoveTo( int entnum );
void        ScriptEntCmd_GravityMove( int entnum );
void        ScriptEntCmd_MoveX( int entnum );
void        ScriptEntCmd_MoveY( int entnum );
void        ScriptEntCmd_MoveZ( int entnum );
void        ScriptEntCmd_RotateTo( int entnum );
void        ScriptEntCmd_RotatePitch( int entnum );
void        ScriptEntCmd_RotateYaw( int entnum );
void        ScriptEntCmd_RotateRoll( int entnum );
void        ScriptEntCmd_RotateVelocity( int entnum );
void        ScriptEntCmd_Solid( int entnum );
void        ScriptEntCmd_NotSolid( int entnum );

/* g_scr_main_mp.c's Scr_GetMethod (0x20034D00) chains into this.  The return
   type is spelled out because the xmethod_t typedef belongs to that unit. */
void ( *ScriptEnt_GetMethod( const char **pName, int *pDeveloper ) )( int entnum );

//
// g_trigger_mp.c
//
void        G_Trigger( gentity_t *self, gentity_t *other );
void        InitTrigger( gentity_t *self );
void        InitSentientTrigger( gentity_t *self );
void        multi_wait( gentity_t *ent );
void        multi_trigger( gentity_t *ent, gentity_t *activator );
void        Use_Multi( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        Touch_Multi( gentity_t *self, gentity_t *other, int touchMode );
void        SP_trigger_multiple( gentity_t *ent );
void        hurt_touch( gentity_t *self, gentity_t *other, int touchMode );
void        hurt_think( gentity_t *ent );
void        hurt_use( gentity_t *self, gentity_t *other, gentity_t *activator );
void        SP_trigger_hurt( gentity_t *self );
void        SP_trigger_once( gentity_t *ent );
qboolean    Respond_trigger_damage( gentity_t *ent, int mod );
void        Activate_trigger_damage( gentity_t *ent, gentity_t *other, int damage, int mod );
void        Use_trigger_damage( gentity_t *self, gentity_t *other, gentity_t *activator );
void        Pain_trigger_damage( gentity_t *self, gentity_t *attacker, int damage,
								 const float *point, int mod, const float *dir, int hitLoc );
void        Die_trigger_damage( gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
								int damage, int mod, int weapon, const float *dir, int hitLoc );
void        SP_trigger_damage( gentity_t *ent );
void        G_CheckHitTriggerDamage( gentity_t *attacker, const vec3_t start, const vec3_t end,
									 int damage, int mod );
void        G_GrenadeTouchTriggerDamage( gentity_t *attacker, const vec3_t start, const vec3_t end,
										 int damage, int mod );
void        explosive_indicator_think( gentity_t *ent );
void        SP_trigger_lookat( gentity_t *ent );

//
// g_misc_mp.c
//
void        SP_info_camp( gentity_t *self );
void        SP_info_null( gentity_t *self );
void        SP_info_notnull( gentity_t *self );
void        SP_light( gentity_t *self );
void        TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles );
void        SP_misc_teleporter_dest( gentity_t *ent );
void        SP_misc_model( gentity_t *ent );
void        use_corona( gentity_t *ent, gentity_t *other, gentity_t *activator );
void        SP_corona( gentity_t *ent );
void        G_InitTurrets( void );
void        G_PlayerTurretPositionAndBlend( gentity_t *player, gentity_t *turret );
void        G_ClientStopUsingTurret( gentity_t *self );
void        turret_think_client( gentity_t *self );
void        turret_think( gentity_t *self );
void        turret_think_init( gentity_t *self );
void        turret_controller( gentity_t *self, unsigned int *partBits );
void        G_FreeTurret( gentity_t *self );
qboolean    G_IsTurretUsable( gentity_t *turret, gentity_t *player );
void        turret_use( gentity_t *self, gentity_t *other, gentity_t *activator );
void        G_SpawnTurret( const char *weaponName, gentity_t *ent );
void        SP_turret( gentity_t *ent );
void        misc_spawner_think( gentity_t *ent );
void        misc_spawner_use( gentity_t *self, gentity_t *other, gentity_t *activator );
void        SP_misc_spawner( gentity_t *ent );
void        miscGunnerEnemyScan( gentity_t *self );

/* forward: g_client_mp.c */
void        SetClientViewAngle( gentity_t *ent, const vec3_t angle );

//
// g_team_mp.c
//
void        target_location_linkup( gentity_t *ent );
void        SP_target_location( gentity_t *self );
qboolean    OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
gentity_t   *Team_GetLocation( gentity_t *ent );
qboolean    Team_GetLocationMsg( gentity_t *ent, char *loc, int loclen );
void        TeamplayInfoMessage( gentity_t *ent );
void        CheckTeamStatus( void );

/* forward: g_main_mp.c.  Registered against "sv_maxclients"; G_InitGame seeds
   level.maxclients from its .integer (0x20025FC3). */
/* g_main_mp.c defines this object under the name it registers,
   sv_maxclients (RTCW's g_maxclients). */
extern vmCvar_t sv_maxclients;                   /* 0x202346A0 */

//
// g_hud_mp.c
//

/* HudElem_UpdateClient's third argument: which of the two ps.hud arrays it
   rebuilds.  G_UpdateHudElemsToClients passes both (0x2002688B), the spectator
   path only the current one (SpectatorClientEndFrame 0x20018529). */
#define HE_UPDATE_ARCHIVAL      0x0001
#define HE_UPDATE_CURRENT       0x0002

void        HudElem_ClientDisconnect( gentity_t *ent );
void        HudElem_DestroyAll( void );
void        Scr_GetHudElemField( int elemnum, int fieldnum );
void        Scr_SetHudElemField( int elemnum, int fieldnum );
void        Scr_FreeHudElemConstStrings( g_hudelem_t *elem );
void        GScr_NewHudElem( void );
void        GScr_NewClientHudElem( void );
void        GScr_NewTeamHudElem( void );
void        GScr_AddFieldsForHudElems( void );
void        ( *HudElem_GetMethod( const char **pName, int *type ) )( int elemnum );
void        HudElem_UpdateClient( gclient_t *client, int entNum, int flags );

extern g_hudelem_t g_hudelems[];                 /* 0x2014F9A0 */


//
// g_svcmds_mp.c
//
qboolean    G_FilterPacket( char *from );
void        G_ProcessIPBans( void );
void        Svcmd_EntityList_f( void );
gclient_t   *ClientForString( const char *s );
qboolean    ConsoleCommand( void );

/* forward: g_main_mp.c -- G_RegisterCvars owns the vmCvar_t objects.  g_banIPs
   is gameCvarTable's "g_banIPs" row (0x20069ED0) and dedicated its
   "dedicated" row (0x20069EE8).  g_filterBan is unregistered; see
   g_svcmds_mp.c. */
extern vmCvar_t g_banIPs;                        /* 0x20236BC0 */
extern vmCvar_t g_filterBan;                     /* 0x2016F100 */
/* `dedicated` is the spelling the cvar is registered under. */
extern vmCvar_t dedicated;                       /* 0x20235300 */

/* forward: g_cmds_mp.c */
char        *ConcatArgs( int start );

/* forward: q_shared.c */
void        Q_strncpyz( char *dest, const char *src, int destsize );

//
// g_scr_main_mp.c
//

/* Scr_SetClassMap's record.  Eight bytes; the VM overwrites classnum with the
   handle it allocated for the class root.  g_entityClassnum and
   g_hudElemClassnum above are the `classnum` words of its two entries. */
typedef struct scr_classStruct_s {
	unsigned short classnum;
	byte pad[2];
	const char      *name;
} scr_classStruct_t;
G_ASSERT_SIZE( scr_classStruct_t, 8 );

/* The two class ids Scr_SetClassMap fills in are g_scr_data.classMap[].classnum
   (0x202CD4E4 and 0x202CD4EC = g_scr_data + 0x1A4 / + 0x1AC); g_spawn_mp.c
   and g_hud_mp.c reach them as g_entityClassnum / g_hudElemClassnum. */
#define SCR_CLASS_ENTITY            0
#define SCR_CLASS_HUDELEM           1
#define SCR_CLASS_NUM               2

/* gameTypes[] is 0x180 bytes of 12-byte records; the overflow guard compares
   against the end of the array, and the message it prints says 31. */
#define MAX_GAMETYPES               32

typedef struct scr_gameTypeEntry_s {
	char            *name;          /* +0x00 the script's file name, lowercased */
	char            *displayName;   /* +0x04 first token of the .txt, or name */
	int teamBased;                  /* +0x08 second token is "team" */
} scr_gameTypeEntry_t;

typedef struct scr_data_s {
	unsigned int levelScriptMain;   /* +0x000 */
	unsigned int unknown_0x004;
	unsigned int gameTypeMain;      /* +0x008 */
	unsigned int gameTypeStartup;   /* +0x00C CodeCallback_StartGameType */
	unsigned int playerConnect;     /* +0x010 CodeCallback_PlayerConnect */
	unsigned int playerDisconnect;  /* +0x014 CodeCallback_PlayerDisconnect */
	unsigned int playerDamage;      /* +0x018 CodeCallback_PlayerDamage */
	unsigned int playerKilled;      /* +0x01C CodeCallback_PlayerKilled */
	int gameTypeCount;              /* +0x020 */
	scr_gameTypeEntry_t gameTypes[MAX_GAMETYPES];   /* +0x024 */
	scr_classStruct_t classMap[SCR_CLASS_NUM];      /* +0x1A4 */
	byte unknown_0x1B4[12];         /* runs to scr_const at 0x202CD500 */
} scr_data_t;
G_ASSERT_SIZE( scr_data_t, 0x1C0 );

extern scr_data_t g_scr_data;                    /* 0x202CD340 */

/*
 * scr_const -- 128 interned script strings, in the order GScr_LoadConsts
 * allocates them.  The struct/typedef name scr_const_s / scr_const_t is
 * inferred; only the member spellings below are recovered, from the strings
 * they intern.  Member order is the struct's, which is not quite the
 * allocation order (`count` / `current` / `croutch`, and `worldspawn` /
 * `xmodel` / `wehrmacht`, are transposed).  Several names are the original's
 * own typos and are kept: `activate` interns "active", `croutch` interns
 * "crouch", `pisol` "pistol", `place_waypoit` "plane_waypoint".
 */
typedef struct scr_const_s {
	unsigned short activate;                        /* +0x00 */
	unsigned short air_strike;
	unsigned short allies;
	unsigned short animdone;
	unsigned short axis;
	unsigned short bodyque;
	unsigned short combat;
	unsigned short connected;
	unsigned short connecting;
	unsigned short count;
	unsigned short current;
	unsigned short croutch;
	unsigned short crowbar;
	unsigned short damage;
	unsigned short death;
	unsigned short disconnected;
	unsigned short dlight;
	unsigned short done;
	unsigned short empty;
	unsigned short enemy;
	unsigned short enemyhidden;
	unsigned short enemyvisible;
	unsigned short entity;
	unsigned short failed;
	unsigned short flamebarrel;
	unsigned short fraction;
	unsigned short func_door;
	unsigned short func_door_rotating;
	unsigned short func_rotating;
	unsigned short func_tramcar;
	unsigned short goal;
	unsigned short grenade;
	unsigned short info_notnull;
	unsigned short invisible;
	unsigned short key1;
	unsigned short key2;
	unsigned short killanimscript;
	unsigned short left;
	unsigned short misc_flak;
	unsigned short misc_mg42;
	unsigned short misc_tagemitter;
	unsigned short mortar;
	unsigned short movedone;
	unsigned short noclass;
	unsigned short noenemy;
	unsigned short noncombat;
	unsigned short normal;
	unsigned short pisol;
	unsigned short place_waypoit;
	unsigned short player;
	unsigned short position;
	unsigned short primary;
	unsigned short primaryb;
	unsigned short prone;
	unsigned short right;
	unsigned short rocket;
	unsigned short rotatedone;
	unsigned short script_brushmodel;
	unsigned short script_model;
	unsigned short script_origin;
	unsigned short scriptcamera;
	unsigned short spawned;
	unsigned short spectator;
	unsigned short stand;
	unsigned short surfacetype;
	unsigned short tag_engine1;
	unsigned short tag_engine2;
	unsigned short target_location;
	unsigned short target_script_trigger;
	unsigned short tempEntity;
	unsigned short muzzleEntity;
	unsigned short smokegrenade;
	unsigned short touch;
	unsigned short trigger;
	unsigned short trigger_use;
	unsigned short trigger_damage;
	unsigned short trigger_lookat;
	unsigned short truck_cam;
	unsigned short worldspawn;                      /* +0x9C */
	unsigned short xmodel;
	unsigned short wehrmacht;
	unsigned short begin;
	unsigned short dynamite;
	unsigned short explosive_indicator;
	unsigned short flamechunk;
	unsigned short follow;
	unsigned short free;
	unsigned short freed;
	unsigned short func_leaky;
	unsigned short info_player_checkpoint;
	unsigned short initialize;
	unsigned short intermission;                    /* +0xB6 */
	unsigned short item_stamina_brandy;
	unsigned short menuresponse;
	unsigned short misc_gunner_gun;
	unsigned short misc_gunner_ring;
	unsigned short mp_info_player_deathmatch;
	unsigned short mp_info_player_intermission;
	unsigned short mp_team_alliedplayer_respawn;
	unsigned short mp_team_alliedplayer_start;
	unsigned short mp_team_axisplayer_respawn;
	unsigned short mp_team_axisplayer_start;
	unsigned short nail;
	unsigned short not;
	unsigned short playing;                         /* +0xD0 */
	unsigned short prox_mine;
	unsigned short reset;
	unsigned short script_mover;
	unsigned short script_multiplayer;
	unsigned short spear;
	unsigned short tag_hand;
	unsigned short tag_rider;
	unsigned short tag_ring;
	unsigned short team_CTF_blueflag;
	unsigned short team_CTF_redflag;
	unsigned short team_WOLF_checkpoint;
	unsigned short team_WOLF_objective;
	unsigned short trigger_aidoor;
	unsigned short trigger_flagonly;
	unsigned short trigger_multiple;
	unsigned short trigger_objective_info;
	unsigned short waiting_for_players;
	unsigned short WP;
	unsigned short zombiespit;
	unsigned short none;                            /* +0xF8 */
	unsigned short dead;                            /* +0xFA */
	unsigned short auto_change;
	unsigned short manual_change;                   /* +0xFE */
} scr_const_t;
G_ASSERT_SIZE( scr_const_t, 256 );

extern scr_const_t scr_const;                   /* 0x202CD500 */

/*
 * The script VM import block: 102 function pointers installed by Scr_FarHook
 * (0x2003FB40) out of the engine's export table, in Scr_NearHook's order.
 * Slots 54, 55, 75 and 78 are holes neither side writes.  The other units'
 * scattered copies of individual slots are duplicates of these.
 */
extern int         ( *Scr_GetBool )( unsigned int num );                                       /* 0x200A04F8 */
extern int         ( *Scr_GetInt )( unsigned int num );                                        /* 0x200A04FC */
extern scr_anim_t  ( *Scr_GetAnim )( unsigned int num, void *anims );                          /* 0x200A0500 */
extern void        ( *Scr_GetAnimTree )();                                                     /* 0x200A0504 */
extern float       ( *Scr_GetFloat )( unsigned int num );                                      /* 0x200A0508 */
extern const char  *( *Scr_GetString )( unsigned int num );                                    /* 0x200A050C */
extern unsigned short ( *Scr_GetConstString )( unsigned int num );                             /* 0x200A0510 */
extern const char  *( *Scr_GetDebugString )( unsigned int num );                               /* 0x200A0514 */
extern const char  *( *Scr_GetIString )( unsigned int num );                                   /* 0x200A0518 */
extern void        ( *Scr_GetConstIString )();                                                 /* 0x200A051C */
extern void        ( *Scr_GetVector )( unsigned int num, vec3_t out );                         /* 0x200A0520 */
extern void        ( *Scr_GetFunc )();                                                         /* 0x200A0524 */
extern int         ( *Scr_GetType )( unsigned int num );                                       /* 0x200A0528 */
extern int         ( *Scr_GetPointerType )( unsigned int num );                                /* 0x200A052C */
extern unsigned int ( *Scr_GetEntityNum )( unsigned int num, int *classnum );                  /* 0x200A0530 */
extern unsigned int ( *Scr_GetNumParam )( void );                                              /* 0x200A0534 */
extern void        ( *Scr_AddBool )( int value );                                              /* 0x200A0538 */
extern void        ( *Scr_AddInt )( int value );                                               /* 0x200A053C */
extern void        ( *Scr_AddFloat )( float value );                                           /* 0x200A0540 */
extern void        ( *Scr_AddAnim )();                                                         /* 0x200A0544 */
extern void        ( *Scr_AddUndefined )( void );                                              /* 0x200A0548 */
extern void        ( *Scr_AddEntityNum )( int entnum, int classnum );                          /* 0x200A054C */
extern void        ( *Scr_AddStruct )( void );                                                 /* 0x200A0550 */
extern void        ( *Scr_AddString )( const char *string );                                   /* 0x200A0554 */
extern void        ( *Scr_AddIString )();                                                      /* 0x200A0558 */
extern void        ( *Scr_AddConstString )( unsigned short id );                               /* 0x200A055C */
extern void        ( *Scr_AddVector )( const float *value );                                   /* 0x200A0560 */
extern void        ( *Scr_AddObject )( unsigned short id );                                    /* 0x200A0564 */
extern void        ( *Scr_AddArray )( void );                                                  /* 0x200A0568 */
extern void        ( *Scr_AddArrayStringIndexed )( unsigned short id );                        /* 0x200A056C */
extern void        ( *Scr_MakeArray )( void );                                                 /* 0x200A0570 */
extern void        ( *Scr_BeginLoadScripts )( void );                                          /* 0x200A0574 */
extern void        ( *Scr_BeginLoadAnimTrees )();                                              /* 0x200A0578 */
extern void        ( *Scr_EndLoadScripts )( void );                                            /* 0x200A057C */
extern void        ( *Scr_EndLoadAnimTrees )( void );                                          /* 0x200A0580 */
extern void        ( *Scr_PrecacheAnimTrees )( void *( *allocFn )( int size ) );               /* 0x200A0584 */
extern void        ( *Scr_FreeScripts )( int bComplete );                                      /* 0x200A0588 */
extern void        ( *Scr_FreeGameVariable )( int bComplete );                                 /* 0x200A058C */
extern void        ( *Scr_ShutdownSystem )( int bComplete );                                   /* 0x200A0590 */
extern int         ( *Scr_IsSystemActive )( int bComplete );                                   /* 0x200A0594 */
extern void        ( *Scr_AddExecThread )();                                                   /* 0x200A0598 */
extern void        ( *Scr_AddExecEntThreadNum )( int entnum, int classnum, int handle, unsigned int paramcount );      /* 0x200A059C */
extern unsigned short ( *Scr_ExecThread )( unsigned int handle, unsigned int paramcount );     /* 0x200A05A0 */
extern unsigned short ( *Scr_ExecEntThreadNum )( int entnum, int classnum, int handle, unsigned int paramcount );      /* 0x200A05A4 */
extern void        ( *Scr_IsThreadAlive )();                                                   /* 0x200A05A8 */
extern void        ( *Scr_Error )( const char *error );                                        /* 0x200A05AC */
extern void        ( *Scr_ErrorWithDialogMessage )();                                          /* 0x200A05B0 */
extern void        ( *Scr_ParamError )( unsigned int num, const char *error );                 /* 0x200A05B4 */
extern void        ( *Scr_ObjectError )( const char *error );                                  /* 0x200A05B8 */
extern void        ( *Scr_SetDynamicEntityField )( int entnum, int classnum, int fieldnum );   /* 0x200A05BC */
extern void        ( *Scr_FreeEntityNum )( int entnum, int classnum );                         /* 0x200A05C0 */
extern unsigned short ( *Scr_GetEntityId )( int entnum, int classnum );                        /* 0x200A05C4 */
extern void        ( *Scr_SetClassMap )( scr_classStruct_t *classMap, unsigned int count );    /* 0x200A05C8 */
extern void        ( *Scr_RemoveClassMap )( void );                                            /* 0x200A05CC */
extern void        ( *Scr_Unused36 )();                                                        /* 0x200A05D0 -- never written */
extern void        ( *Scr_Unused37 )();                                                        /* 0x200A05D4 -- never written */
extern void        ( *Scr_AddClassField )( unsigned short classnum, const char *name, unsigned short fieldnum );       /* 0x200A05D8 */
extern void        ( *Scr_AddFields )( const char *path, const char *extension );              /* 0x200A05DC */
extern unsigned short ( *Scr_FindField )( const char *name, int *type );                       /* 0x200A05E0 */
extern int         ( *Scr_GetOffset )( unsigned short classnum, const char *name );            /* 0x200A05E4 */
extern void        ( *Scr_CopyEntityNum )( int sourceEntnum, int destEntnum, int classnum );   /* 0x200A05E8 */
extern void        ( *Scr_Init )();                                                            /* 0x200A05EC */
extern void        ( *Scr_Shutdown )();                                                        /* 0x200A05F0 */
extern void        ( *Scr_Abort )();                                                           /* 0x200A05F4 */
extern void        ( *Scr_SetLoading )( int loading );                                         /* 0x200A05F8 */
extern void        ( *Scr_AllocGameVariable )( int classnum, int time );                       /* 0x200A05FC */
extern void        ( *Scr_InitSystem )( void );                                                /* 0x200A0600 */
extern void        ( *Scr_GetChecksum )();                                                     /* 0x200A0604 */
extern void        ( *Scr_HasSourceFiles )();                                                  /* 0x200A0608 */
extern void        ( *Scr_SaveSource )();                                                      /* 0x200A060C */
extern void        ( *Scr_LoadSource )();                                                      /* 0x200A0610 */
extern void        ( *Scr_SkipSource )();                                                      /* 0x200A0614 */
extern void        ( *Scr_SavePre )();                                                         /* 0x200A0618 */
extern void        ( *Scr_SavePost )();                                                        /* 0x200A061C */
extern void        ( *Scr_SaveShutdown )();                                                    /* 0x200A0620 */
extern void        ( *Scr_Unused4B )();                                                        /* 0x200A0624 -- never written */
extern void        ( *Scr_LoadPre )();                                                         /* 0x200A0628 */
extern void        ( *Scr_LoadShutdown )();                                                    /* 0x200A062C */
extern void        ( *Scr_Unused4E )();                                                        /* 0x200A0630 -- never written */
extern int         ( *Scr_LoadScript )( const char *scriptName );                              /* 0x200A0634 */
extern void        *( *Scr_FindAnimTree )( const char *treeName );                             /* 0x200A0638 */
extern void        ( *Scr_FindAnim )( const char *treeName, const char *animName, scr_anim_t *anim );  /* 0x200A063C */
extern int         ( *Scr_GetFunctionHandle )( const char *scriptName, const char *label );    /* 0x200A0640 */
extern void        ( *Scr_FreeThread )( unsigned short thread );                               /* 0x200A0644 */
extern void        ( *Scr_ConvertThreadToSave )();                                             /* 0x200A0648 */
extern void        ( *Scr_ConvertThreadFromLoad )();                                           /* 0x200A064C */
extern void        ( *Scr_SetString )( unsigned short *dest, unsigned short id );              /* 0x200A0650 */
extern unsigned short ( *Scr_AllocString )( const char *string, unsigned int user );           /* 0x200A0654 */
extern void        ( *Scr_NotifyNum )( int entnum, int classnum, unsigned short event, unsigned int paramcount );      /* 0x200A0658 */
extern void        ( *Scr_NotifyId )();                                                        /* 0x200A065C */
extern const char  *( *SL_ConvertToString )( unsigned short id );                              /* 0x200A0660 */
extern unsigned short ( *SL_GetString )( const char *string, unsigned int user );              /* 0x200A0664 */
extern unsigned short ( *SL_GetLowercaseString )( const char *string, unsigned int user );     /* 0x200A0668 */
extern unsigned short ( *SL_FindLowercaseString )( const char *string );                       /* 0x200A066C */
extern void        ( *Scr_CreateCanonicalFilename )();                                         /* 0x200A0670 */
extern void        ( *Scr_SetTime )( int time );                                               /* 0x200A0674 */
extern void        ( *Scr_RunCurrentThreads )( void );                                         /* 0x200A0678 */
extern void        ( *Scr_ResetTimeout )( void );                                              /* 0x200A067C */
extern int         ( *Scr_GetAnimsIndex )( int anims );                                        /* 0x200A0680 */
extern void        ( *Scr_GetAnims )();                                                        /* 0x200A0684 */
extern void        *( *MT_Alloc )( int size, int type );                                       /* 0x200A0688 */
extern void        ( *MT_Free )( void *p, int size );                                          /* 0x200A068C */

/* xfunction_t / xmethod_t and the two dispatch tables are private to
   g_scr_main_mp.c; only the three entry points the VM asks through are here. */
struct objective_t;

void        GScr_LoadConsts( void );
void        Scr_LoadLevel( void );
int         GScr_LoadScriptAndLabel( const char *scriptName, const char *label,
                                     qboolean mandatory );
void        GScr_LoadGameTypeScript( void );
void        GScr_LoadLevelScript( void );
void        GScr_PostLoadScripts( void );
void        *Hunk_AllocXAnimCreate( int size );
void        GScr_LoadScripts( void );
void        GScr_FreeScripts( void );
gentity_t   *GetEntity( unsigned int entnum );
gentity_t   *GetPlayerEntity( unsigned int entnum );
unsigned short GScr_AllocString( const char *string );
void        Scr_LocalizationError( const char *error );
void        Scr_ConstructMessageString( int msgType, int firstParam, char *string,
                                        int maxLen );
void        Scr_MakeGameMessage( int clientNum, const char *command );
qboolean    G_GetHintStringIndex( int *index, const char *string );
void        G_InitObjectives( void );
qboolean    ObjectiveStateIndexFromString( int *outState, unsigned short stateName );
void        ClearObjective_OnEntity( objective_t *objective );
void        ClearObjective( objective_t *objective );
void        SetObjectiveIcon( unsigned int paramIndex, objective_t *objective );
int         GScr_GetScriptMenuIndex( const char *name );
int         GScr_GetStatusIconIndex( const char *name );
int         GScr_GetHeadIconIndex( const char *name );
void        Scr_SetFog( const char *cmdName, float nearDist, float farDist, float density,
                        float red, float green, float blue, float transitionTime );
void        Scr_SetOrigin( gentity_t *ent, int fieldnum );
void        Scr_SetAngles( gentity_t *ent, int fieldnum );
void        Scr_SetHealth( gentity_t *ent, int fieldnum );
void        GScr_AddVector( const float *value );
void        GScr_AddEntity( gentity_t *ent );
void        Scr_ParseGameTypeList( void );
const char  *Scr_IsValidGameType( const char *gameType );
void        Scr_LoadGameType( void );
void        Scr_StartupGameType( void );
void        Scr_PlayerConnect( gentity_t *ent );
void        Scr_PlayerDisconnect( gentity_t *ent );
/* Scr_PlayerDamage is deliberately not declared here: g_combat_mp.c declares
   it returning qboolean and the retail function returns nothing. */
void        Scr_PlayerKilled( gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
                              int damage, int mod, int weapon, const float *dir, int hitLoc );
int         Scr_LoadRead( int len );
void        *Scr_FarHook( void *scrImports );

/* The 145 script builtins and object methods this unit defines are reached
   only through scr_functions[] / scr_methods[], which are file-static there,
   so they are deliberately not declared here. */


//
// g_debug_mp.c
//
void        G_DebugLine( const vec3_t start, const vec3_t end, const vec3_t color,
						 int depthTest, int duration );
void        G_DebugBox( const vec3_t mins, const vec3_t maxs, const vec3_t color,
						int depthTest, int duration );
void        G_DebugCircle( const vec3_t org, qboolean flat, float radius, const vec3_t color,
						   int depthTest, int duration );
void        G_DebugCircleEx( const vec3_t dir, const vec3_t org, float radius, const vec3_t color,
							 int depthTest, int duration );
void        G_DebugArc( const vec3_t org, float radius, float startAngle, float endAngle,
						const vec3_t color, int depthTest, int duration );
int         Q_rint( float in );

/* forward: q_math.c */
vec_t       VectorNormalize2( const vec3_t v, vec3_t out );
void        PerpendicularVector( vec3_t dst, const vec3_t src );

#endif
