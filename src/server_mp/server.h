/*
 * @fidelity: likely
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#ifdef __QCOMMON_H__
#error "server.h must be included before qcommon.h -- see the header comment"
#endif

#define sv sv_qcommon_placeholder_unused
#include "../qcommon/qcommon.h"
#undef sv

#define SV_ASSERT_SIZE( type, bytes ) COD1_ASSERT_SIZE( type, bytes )

/* Pushed as a literal by SV_Init (0x00459130) and SVC_Info (0x0045A320): `push 1` feeding va("%i", ...). CoD 1.1 really does advertise protocol 1. */
#define PROTOCOL_VERSION        1

#define MAX_MASTER_SERVERS      5
#define MAX_ENT_CLUSTERS        16

/* SV_Frame's map-restart guards; the exact constants the retail compares against (0x0045B28A onwards). */
#define SV_TIME_WRAP_LIMIT              0x70000000
#define SV_SNAPSHOT_COUNTER_LIMIT       0x7FFFFFFE
#define SV_CACHED_SNAP_ENTITIES_LIMIT   0x7FFFBFFE
#define SV_CACHED_SNAP_CLIENTS_LIMIT    0x7FFFEFFE
#define SV_ARCHIVED_SNAP_FRAMES_LIMIT   0x7FFFFB4E
#define SV_ARCHIVED_SNAP_BUFFER_LIMIT   0x7DFFFFFE
#define SV_CACHED_SNAP_FRAMES_LIMIT     0x7FFFFDFE

/* Sizes SV_EnableArchivedSnapshot (0x00458620) hands to Z_MallocInternal. */
#define SV_CACHED_SNAPSHOT_FRAMES_BYTES     0x00440000
#define SV_CACHED_SNAPSHOT_ENTITIES_BYTES   0x02130000
#define SV_ARCHIVED_SNAPSHOT_FRAMES_BYTES   0x00002580
#define SV_ARCHIVED_SNAPSHOT_BUFFER_BYTES   0x02000000
#define SV_CACHED_SNAPSHOT_CLIENTS_BYTES    0x00003800

#define CS_SCRIPT_KV_BASE       140
#define CS_SCRIPT_KV_COUNT      64

#define CS_SERVERINFO           0
#define CS_SYSTEMINFO           1

typedef enum clientConnectState_e {
	CS_FREE      = 0x0,
	CS_ZOMBIE    = 0x1,
	CS_CONNECTED = 0x2,
	CS_PRIMED    = 0x3,
	CS_ACTIVE    = 0x4,
} clientConnectState_t;

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

#define GAME_INIT               0
#define GAME_SHUTDOWN           1
#define GAME_CLIENT_CONNECT     2
#define GAME_CLIENT_BEGIN       3
#define GAME_CLIENT_USERINFO_CHANGED 4
#define GAME_CLIENT_DISCONNECT  5
#define GAME_CLIENT_COMMAND     6
#define GAME_CLIENT_THINK       7
#define GAME_UPDATE_CVARS       9
#define GAME_RUN_FRAME          10
#define GAME_CONSOLE_COMMAND_SV 11
#define GAME_SCRIPT_FAR_HOOK    12
#define GAME_GET_MATCH_STATE    16
#define GAME_GET_ARCHIVE_TIME   18
#define GAME_GET_CLIENT_SCORE   20

typedef struct trajectory_t {
	int trType;
	int trTime;
	int trDuration;
	vec3_t trBase;
	vec3_t trDelta;
} trajectory_t;
SV_ASSERT_SIZE( trajectory_t, 36 );

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
	unsigned int eventParms[4];
	int weapon;
	int legsAnim;
	int torsoAnim;
	int leanf;
	int scale;
	int dmgFlags;
	int animMovetype;
	float fTorsoHeight;
	float fTorsoPitch;
	float fWaistPitch;
} entityState_t;
SV_ASSERT_SIZE( entityState_t, 240 );

typedef struct entityShared_t {
	qboolean linked;                /* +0xF0 in gentity_t */
	int svFlags;
	int singleClient;
	byte unknown_0x0C[4];
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
SV_ASSERT_SIZE( entityShared_t, 100 );

typedef struct clientSession_t {
	sessionState_t sessionState;
	int forceSpectatorClient;
	int statusIcon;
	int archiveTime;
	int score;
	int deaths;
	unsigned short pers;            /* +0x18 script object id, set by ClientConnect (0x20019E89) */
	byte unknown_0x1A[2];
	clientConnected_t connected;
	usercmd_t cmd;
	usercmd_t oldcmd;
	qboolean localClient;
	byte unknown_0x54[8];
	/* 32, not MAX_NETNAME: ClientUserinfoChanged (0x20019D10) copies 31 bytes
	   and terminates at sess+0x7B; handicap follows at +0x7C (gclient 0x214C
	   in the game's client field table). */
	char netname[32];
	int handicap;                   /* +0x7C */
	int maxHealth;
	byte unknown_0x84[128];
} clientSession_t;
SV_ASSERT_SIZE( clientSession_t, 260 );

typedef struct gclient_s {
	playerState_t ps;
	clientSession_t sess;
	int spectatorClient;
	qboolean noclip;
	qboolean ufo;
	qboolean frozen;                /* +0x21E0 PlayerCmd_FreezeControls 0x2001CD00 */
	int lastCmdTime;                /* +0x21E4 ClientThink 0x200181BB, ClientEndFrame 0x20018D21 */
	int buttons;                    /* +0x21E8 */
	int oldbuttons;                 /* +0x21EC */
	int latched_buttons;            /* +0x21F0 */
	int wbuttons;                   /* +0x21F4 */
	int oldwbuttons;                /* +0x21F8 */
	int latched_wbuttons;           /* +0x21FC */
	vec3_t oldOrigin;               /* +0x2200 */
	byte unknown_0x220C[8];
	int damage_blood;               /* +0x2214 P_DamageFeedback 0x20017179 */
	vec3_t damage_from;             /* +0x2218 */
	qboolean damage_fromWorld;      /* +0x2224 */
	byte unknown_0x2228[12];
	int inactivityTime;             /* +0x2234 */
	byte unknown_0x2238[40];
	struct gentity_s *lookatEnt;    /* +0x2260 ScrCmd_IsLookingAt 0x2001CED6 */
	byte unknown_0x2264[4];
	int pingTime;                   /* +0x2268 PlayerCmd_pingPlayer 0x2001B450 */
	byte unknown_0x226C[88];        /* bg_weapon.c's saved view/weapon angle state */
} gclient_t;
SV_ASSERT_SIZE( gclient_t, 8900 + PLAYERSTATE_EXTRA_BYTES );

typedef struct gentity_s {
	entityState_t s;
	entityShared_t r;
	byte unknown_0x154[4];
	gclient_t       *client;
	void            *turret;        /* +0x15C turret state record (G_FreeEntity 0x2003C1E0) */
	byte unknown_0x160[48];         /* the inuse/physicsObject, door-sound and water byte block */
	int clipmask;
	int lastFrameNum;               /* +0x194 G_RunFrameForEntity's once-per-frame latch */
	byte unknown_0x198[380];
} gentity_t;
SV_ASSERT_SIZE( gentity_t, 788 );

typedef struct svEntity_t {
	byte unknown_0x00[8];
	entityState_t baseline;
	int baselineSvFlags;            /* +0xF8, from gentity->r.svFlags */
	int baselineSingleClient;       /* +0xFC, from gentity->r.singleClient */
	float baselineAbsmin[3];        /* +0x100, from gentity->r.absmin */
	float baselineAbsmax[3];        /* +0x10C, from gentity->r.absmax */
	byte unknown_0x118[100];
} svEntity_t;
SV_ASSERT_SIZE( svEntity_t, 380 );

typedef struct server_t {
	serverState_t state;
	qboolean restarting;
	int serverId;
	int checksumFeed;
	int timeResidual;
	byte unknown_0x14[1028];
	char            *configstrings[MAX_CONFIGSTRINGS];
	svEntity_t svEntities[MAX_GENTITIES];

	const char      *entityParsePoint;

	gentity_t       *gentities;
	int gentitySize;
	int num_entities;
	playerState_t   *gameClients;
	int gameClientSize;

	int bpsWindow[MAX_BPS_WINDOW];
	int bpsWindowSteps;
	int bpsTotalBytes;
	int bpsMaxBytes;
	int ubpsWindow[MAX_BPS_WINDOW];
	int ubpsTotalBytes;
	int ubpsMaxBytes;
	float ucompAve;
	int ucompNum;
} server_t;
SV_ASSERT_SIZE( server_t, 398572 );

typedef struct clientSnapshot_t {
	playerState_t ps;
	int num_entities;
	int num_clients;
	int first_entity;
	int first_client;
	unsigned int messageSent;
	unsigned int messageAcked;
	int messageSize;
} clientSnapshot_t;
SV_ASSERT_SIZE( clientSnapshot_t, 8428 + PLAYERSTATE_EXTRA_BYTES );

typedef struct reliableCommands_t {
	char command[MAX_STRING_CHARS];
	int cmdTime;
	int cmdType;
} reliableCommands_t;
SV_ASSERT_SIZE( reliableCommands_t, 1032 );

typedef struct client_t {
	clientConnectState_t state;
	qboolean sendAsActive;
	const char      *dropReason;
	char userinfo[MAX_INFO_STRING];

	reliableCommands_t reliableCommands[MAX_RELIABLE_COMMANDS];
	int reliableSequence;
	int reliableAcknowledge;
	int reliableSent;
	int messageAcknowledge;

	int gamestateMessageNum;
	int challenge;

	usercmd_t lastUsercmd;
	int lastClientCommand;
	char lastClientCommandString[MAX_STRING_CHARS];
	gentity_t       *gentity;
	char name[MAX_NAME_LENGTH];

	char downloadName[MAX_QPATH];
	fileHandle_t download;
	int downloadSize;
	int downloadCount;
	int downloadClientBlock;
	int downloadCurrentBlock;
	int downloadXmitBlock;
	byte            *downloadBlocks[MAX_DOWNLOAD_WINDOW];
	int downloadBlockSize[MAX_DOWNLOAD_WINDOW];
	qboolean downloadEOF;
	int downloadSendTime;

	int deltaMessage;
	int nextReliableTime;
	int lastPacketTime;
	int lastConnectTime;
	int nextSnapshotTime;
	qboolean rateDelayed;
	int timeoutCount;

	clientSnapshot_t frames[PACKET_BACKUP];
	int ping;
	int rate;
	int snapshotMsec;
	int pureAuthentic;

	netchan_t netchan;

	unsigned short scriptId;
	int bIsTestClient;
	int serverId;
} client_t;
SV_ASSERT_SIZE( client_t, 370940 + PACKET_BACKUP * PLAYERSTATE_EXTRA_BYTES + 2 * ( MAX_MSGLEN - STOCK_MAX_MSGLEN ) + 4 );

typedef struct challenge_t {
	netadr_t adr;
	int challenge;
	int time;
	int pingTime;
	int firstTime;
	int firstPing;
	qboolean connected;
} challenge_t;
SV_ASSERT_SIZE( challenge_t, 44 );

/* 1200 entries x 8 bytes == SV_ARCHIVED_SNAPSHOT_FRAMES_BYTES (0x2580). Stride 8: `[eax+edx*8]` / `[eax+edx*8+4]` at 0x0045E797. */
typedef struct archivedSnapshot_s {
	int start;                      /* +0x00 svs.nextArchivedSnapshotBuffer before the write -- an ever-increasing byte cursor, NOT pre-wrapped */
	int len;                        /* +0x04 bytes written for this frame */
} archivedSnapshot_t;
SV_ASSERT_SIZE( archivedSnapshot_t, 8 );

/* 512 entries x 28 bytes == SV_CACHED_SNAPSHOT_CLIENTS_BYTES (0x3800). Stride 28: `imul esi, 1Ch` at 0x0045DF32 and 0x0045E36D. */
typedef struct cachedSnapshot_s {
	int archivedFrame;              /* +0x00 svs.nextArchivedSnapshotFrames when cached */
	int time;                       /* +0x04 svs.time */
	int num_entities;               /* +0x08 incremented per entity archived */
	int first_entity;               /* +0x0C svs.nextCachedSnapshotEntities at capture */
	int num_clients;                /* +0x10 incremented per client archived */
	int first_client;               /* +0x14 svs.nextCachedSnapshotClients at capture */
	int usesDelta;                  /* +0x18 name inferred; written 0 by the writer, a non-zero value disqualifies the frame as a delta base. The setter is presumably the reader SV_GetCachedSnapshotInternal. */
} cachedSnapshot_t;
SV_ASSERT_SIZE( cachedSnapshot_t, 28 );

typedef struct archivedEntity_s {
	entityState_t s;                /* +0x000 memcpy 0xF0 from gentity->s */
	int svFlags;                    /* +0x0F0 gentity->r.svFlags (ent+0x0F4) */
	int singleClient;               /* +0x0F4 gentity->r.singleClient (ent+0x0F8) */
	float absmin[3];                /* +0x0F8 gentity->r.absmin (ent+0x11C) */
	float absmax[3];                /* +0x104 gentity->r.absmax (ent+0x128) */
} archivedEntity_t;
SV_ASSERT_SIZE( archivedEntity_t, 272 );

/* 4096 entries x 8496 bytes == SV_CACHED_SNAPSHOT_ENTITIES_BYTES (0x2130000). Stride 8496: `imul ebx, 2130h` at 0x0045E032 and 0x0045E3E7. */
typedef struct cachedClient_s {
	int havePlayerState;            /* +0x0000 VM_Call(GAME_GET_CLIENT_INFO) result */
	byte cs[92];                    /* +0x0004 the 92-byte snapshot client record; its first dword is the client number */
	playerState_t ps;               /* +0x0060 8400 bytes */
} cachedClient_t;
SV_ASSERT_SIZE( cachedClient_t, 8496 + PLAYERSTATE_EXTRA_BYTES );

#define SV_NUM_ARCHIVED_SNAPSHOT_FRAMES     1200
#define SV_NUM_CACHED_SNAPSHOT_FRAMES       512
#define SV_NUM_CACHED_SNAPSHOT_ENTITIES     0x4000      /* 0x440000 / 272 */
#define SV_NUM_CACHED_SNAPSHOT_CLIENTS      4096        /* 0x2130000/ 8496 */

#define SV_SNAPSHOT_CLIENT_SIZE 92

typedef struct serverStatic_t {
	qboolean initialized;
	int time;
	int snapFlagServerBit;

	client_t        *clients;
	int numSnapshotEntities;
	int numSnapshotClients;
	int nextSnapshotEntities;
	int nextSnapshotClients;

	entityState_t   *snapshotEntities;      /* +0x20, Hunk_AllocAlign(240 * numSnapshotEntities, 32) */
	void            *snapshotClients;       /* +0x24, Hunk_AllocAlign( 92 * numSnapshotClients, 32) */
	qboolean archiveEnabled;                /* +0x28, SV_EnableArchivedSnapshot */
	int nextArchivedSnapshotFrames;         /* +0x2C */

	archivedSnapshot_t  *archivedSnapshotFrames;
	byte                *archivedSnapshotBuffer;
	int nextArchivedSnapshotBuffer;
	int nextCachedSnapshotEntities;
	int nextCachedSnapshotClients;
	int nextCachedSnapshotFrames;
	archivedEntity_t    *cachedSnapshotEntities;    /* +0x48 */
	cachedClient_t      *cachedSnapshotClients;     /* +0x4C */
	cachedSnapshot_t    *cachedSnapshotFrames;      /* +0x50 */

	int nextHeartbeatTime;
	challenge_t challenges[MAX_CHALLENGES];
	netadr_t redirectAddress;
	netadr_t authorizeAddress;
	void            *pOOBProf;
} serverStatic_t;
SV_ASSERT_SIZE( serverStatic_t, 45188 );

#define SVS_CACHED_ENTITIES     ( svs.cachedSnapshotEntities )
#define SVS_CACHED_CLIENTS      ( svs.cachedSnapshotClients )
#define SVS_CACHED_FRAMES       ( svs.cachedSnapshotFrames )

extern server_t sv;                     /* 0x016515A0, local to the server */
extern serverStatic_t svs;              /* 0x016B2AA0, persistent across maps */

extern cvar_t   *sv_gametype;           /* 0x016BDB78 */
extern cvar_t   *sv_mapname;            /* 0x016BDB7C */
extern cvar_t   *sv_privateClients;     /* 0x016BDB54 */
extern cvar_t   *sv_hostname;           /* 0x016BDB5C */
extern cvar_t   *sv_maxclients;         /* 0x016BDB58 */
extern cvar_t   *sv_maxRate;            /* 0x016BDB80 */
extern cvar_t   *sv_minPing;            /* 0x016BDB8C */
extern cvar_t   *sv_maxPing;            /* 0x01651584 */
extern cvar_t   *sv_floodProtect;       /* 0x016BDB6C */
extern cvar_t   *sv_allowAnonymous;     /* 0x016BDB64 */
extern cvar_t   *sv_showCommands;       /* 0x016BDB84 */
extern cvar_t   *sv_serverid;           /* 0x016BDBAC */
extern cvar_t   *sv_pure;               /* 0x016B2A8C */
extern cvar_t   *sv_rconPassword;       /* 0x016B2A90 */
extern cvar_t   *sv_privatePassword;    /* 0x016BDB94 */
extern cvar_t   *sv_fps;                /* 0x016BDBA8 */
extern cvar_t   *sv_timeout;            /* 0x016BDBA0 */
extern cvar_t   *sv_zombietime;         /* 0x016BDB90 */
extern cvar_t   *sv_allowDownload;      /* 0x016BDB70 */
extern cvar_t   *sv_reconnectlimit;     /* 0x016BDB68 */
extern cvar_t   *sv_showloss;           /* 0x016BDB60 */
extern cvar_t   *sv_debugSpawn;
extern cvar_t   *sv_padPackets;         /* 0x016BDBA4 */
extern cvar_t   *sv_killserver;         /* 0x016BDB74 */
extern cvar_t   *sv_onlyVisibleClients; /* 0x016BDB98 */
extern cvar_t   *sv_showAverageBPS;     /* 0x016BDB9C */
extern cvar_t   *sv_mapRotation;        /* 0x016BDBB0 */
extern cvar_t   *sv_mapRotationCurrent; /* 0x016BDB88 */
extern cvar_t   *sv_paused;             /* 0x0163B3C8 */
extern cvar_t   *cl_paused;             /* 0x0163A22C */

extern cvar_t   *sv_master_0_;          /* 0x016BDB40 */
extern cvar_t   *sv_master_1_;
extern cvar_t   *sv_master_2_;
extern cvar_t   *sv_master_3_;
extern cvar_t   *sv_master_4_;
extern cvar_t   *sv_master[MAX_MASTER_SERVERS];

/* Resolved master addresses, one per sv_masterN. Retail 0x008E21D8, stride 20; storage owned by this unit. */
extern netadr_t sv_masterAdr[MAX_MASTER_SERVERS];

/* The game VM. Retail 0x014073D0; storage owned by the server. */
extern vm_t     *vm;

extern int xanim_activePoolSlot;                /* 0x00A9CC58, XAnim pool selector; UO calls this XAnimSetUser */
extern int com_skelInvalidated;         /* 0x014073D8 */
extern int hunk_temp_permanent;         /* 0x008931C4, holds an address -- see com_memory_core.c */
extern int hunk_temp_temp;              /* 0x008931C8, ditto */
extern int hunk_lowMark;                /* 0x008931BC, game-VM hunk mark */
extern cvar_t   *net_profile;           /* 0x01651550 */
extern int xmodel_enforceExist;                /* 0x00572D68, cl_xmodelcheck latch */
extern int com_dobjPoolInited;                /* 0x00894240 */
extern int com_dobjInited;               /* 0x01407400 */
extern int sv_rollingServerId;               /* 0x014073DC, rolling sv_serverid */
extern int cls_loadingPlaque;           /* 0x0155F3D4 */
extern int cls_rendererStarted;         /* 0x0155F3CC */
extern int ( *com_serverEndpoint )();   /* 0x01432860 */
extern int sv_rconLastTime;               /* 0x014073D4, SVC_RemoteCommand throttle */

/* script/scr_vm_state.c owns scrVmGlob.loading (retail 0x00A7A5E0) and exposes it through this setter, which is what SV_Frame's inline store becomes. */
void Scr_SetLoading( int bLoading );

#define SV_HUNK_LOW_PERM        ( *(int *) hunk_temp_permanent )
#define SV_HUNK_LOW_TEMP        ( *(int *) hunk_temp_temp )
#define SV_HUNK_GAME_BASE       ( *(int *) hunk_lowMark )

void SV_SetConfigstring( unsigned int index, const char *val );
/* Retail is __usercall(index@edi, buffer@ebx, bufferSize@esi); see game syscall 28 in sv_game_mp.c. */
void SV_GetConfigstring( unsigned int index, char *buffer, int bufferSize );
/* __usercall(max@ebx, start, key). Semantically this is UO's SV_GetConfigValueForKey(base, count, key) with base and count swapped -- see the body. */
const char *SV_GetConfigstringConst( int max, int start, const char *key );
void SV_SetConfigValueForKey( int start, const char *key, const char *value, int max );
void SV_SetUserinfo( int index, const char *userinfo );
void SV_GetUserinfo( int index, char *buffer, int bufferSize );
void SV_CreateBaseline( void );
void SV_BoundMaxClients( int minimum );
void SV_Startup( void );
void SV_ChangeMaxClients( void );
void SV_SetExpectedHunkUsage( const char *mapname );
void SV_ClearServer( void );
void SV_EnableArchivedSnapshot( qboolean enable );
void SV_InitArchivedSnapshot( void );
void SV_FreeArchivedSnapshot( void );
void SV_SpawnServer( const char *server, qboolean killBots );
void SV_Init( void );
void SV_FinalMessage( const char *message );
void SV_FreeClients( void );
void SV_Shutdown( const char *finalmsg );

char *SV_ExpandNewlines( const char *string );
qboolean SV_IsFirstTokenEqual( const char *a, const char *b );
int SV_CanReplaceServerCommand( client_t *client, const char *cmd );
void SV_CullIgnorableServerCommands( client_t *client );
void SV_AddServerCommand( client_t *client, int isReliable, const char *cmd );
void QDECL SV_SendServerCommand( client_t *client, int isReliable, const char *fmt, ... );
void SV_MasterHeartbeat( const char *hbname );
void SV_MasterGameCompleteStatus( void );
void SV_MasterShutdown( void );
int SV_GetClientScore( client_t *client );
void SV_FlushRedirect( char *outputbuf );
void SV_ConnectionlessPacket( netadr_t from, msg_t *msg );
void SV_PacketEvent( netadr_t from, msg_t *msg );
void SV_CalcPings( void );
void SV_FreeClientScriptId( client_t *client );
void SV_CheckTimeouts( void );
qboolean SV_CheckPaused( void );
void SV_RunFrame( void );
void SV_BotUserMove( client_t *client );
void SV_UpdateBots( void );
void SV_Frame( int msec );

void SV_AddOperatorCommands( void );
void SV_DropClient( client_t *drop, const char *reason );
void SV_ClientThink( client_t *cl, usercmd_t *cmd );
void SV_ExecuteClientMessage( msg_t *msg, client_t *cl );
/* SV_UserMove is __usercall(delta@eax, unused@edi, cl, msg) in retail. */
qboolean SV_ClientCommand( msg_t *msg, client_t *cl );
void SV_UserMove( qboolean delta, client_t *cl, msg_t *msg );
void SV_SendClientGameState( client_t *client );
void SV_ClientEnterWorld( client_t *cl, usercmd_t *cmd );
int MSG_ReadBitsCompress( const byte *input, byte *outputBuf, int readsize, int capacity );
void SV_SendClientMessages( void );
void SV_SendClientSnapshot( client_t *client );
void SV_ArchiveSnapshot( void );
void SV_ShutdownGameProgs( void );
/* Two stack arguments at 0x00458E37: restart, then the GAME_GET_MATCH_STATE result SV_SpawnServer saved before the map load. Both forwarded into GAME_INIT. */
void SV_InitGameVM( qboolean restart, int matchState );
void SVC_Status( netadr_t from );
void SVC_GameCompleteStatus( netadr_t from );
void SVC_Info( netadr_t from );
void SVC_RemoteCommand( netadr_t from, msg_t *msg );
void SV_GetChallenge( netadr_t from );
void SV_DirectConnect( netadr_t from );
void SV_AuthorizeIpPacket( netadr_t from );
void SV_Netchan_Decode( client_t *client, byte *data, int len );

void MSG_BeginReading( msg_t *msg );
int MSG_ReadByte( msg_t *msg );
int MSG_ReadShort( msg_t *msg );
int MSG_ReadLong( msg_t *msg );
char *MSG_ReadStringLine( msg_t *msg );
void MSG_WriteReliableCommandToBuffer( const char *pszCommand, int iBufferSize, char *pszBuffer );

qboolean Netchan_Process( netchan_t *chan, msg_t *msg );
qboolean NET_CompareBaseAdr( netadr_t a, netadr_t b );
qboolean NET_StringToAdr( const char *s, netadr_t *a );
char *NET_AdrToString( netadr_t a );
qboolean QDECL NET_OutOfBandPrint( netsrc_t sock, netadr_t adr, const char *format, ... );
void NetProf_PrepProfiling( void **pProf );
int NetProf_AddPacket( netsrc_t sock, int length, const void *data, netadr_t to );

int SV_GameSystemCalls( int *parms );
void NET_Sleep( int msec );
void *Hunk_AllocAlignInternal( int size, int align );
void Cmd_TokenizeString2( const char *text_in, int max_tokens );
void CL_MapLoading( void );
void CIN_StopCinematic( int handle );
void CM_LoadMap( const char *name, qboolean clientload, int *checksum );
void Com_LoadSoundAliases( const char *name, int localized );
void FS_ClearPakReferences( int flags );
const char *FS_LoadedPakChecksums( void );
const char *FS_LoadedPakNames( void );
const char *FS_ReferencedPakChecksums( void );
const char *FS_ReferencedPakNames( void );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
void ClearObjectInternal( unsigned short id );
void RemoveRefToObject( unsigned short id );
void FreeVariable( unsigned short id );

#endif
