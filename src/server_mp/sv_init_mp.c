/*
 * @fidelity: verified
 */

#include "server.h"

#include <string.h>
#include <stdlib.h>

vm_t        *vm;                                /* retail 0x014073D0 */
cvar_t      *sv_master[MAX_MASTER_SERVERS];

cvar_t      *sv_debugSpawn;

/* Retail 0x004556E0 returns nothing -- see the note at its definition in sv_client_mp.c. Declared here for the NOT-RETAIL console command below. */
extern void SV_AddTestClient( void );
extern void SV_TestClientCommand_f( void );
extern void SV_TestClientMove_f( void );
extern void SV_TestDumpMissiles_f( void );

extern void FS_ClearMemory( void );

static const char       *sv_emptyString = "";

/* ---- SV_SetConfigstring  0x00457BE0 ---- */
void SV_SetConfigstring( unsigned int index, const char *val ) {
	int i;
	int len;
	int offset;
	const char  *cmd;
	client_t    *client;
	char chunk[MAX_STRING_CHARS];

	if ( index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "\x15SV_SetConfigstring: bad index %i\n", index );
	}

	if ( !val ) {
		val = sv_emptyString;
	}

	if ( !strcmp( val, sv.configstrings[index] ) ) {
		return;
	}

	free( sv.configstrings[index] );
	sv.configstrings[index] = CopyStringInternal( val );

	if ( sv.state != SS_GAME && !sv.restarting ) {
		return;
	}

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		client = &svs.clients[i];
		if ( client->state < CS_PRIMED ) {
			continue;
		}

		len = strlen( val );
		if ( len < 1000 ) {
			SV_SendServerCommand( client, qtrue, "d %i %s", index, val );
			continue;
		}

		offset = 0;
		while ( len > 0 ) {
			if ( offset == 0 ) {
				cmd = "x";
			} else if ( len >= 1000 ) {
				cmd = "y";
			} else {
				cmd = "z";
			}
			strncpy( chunk, val + offset, 999 );
			chunk[999] = 0;
			SV_SendServerCommand( client, qtrue, "%s %i %s", cmd, index, chunk );
			offset += 999;
			len -= 999;
		}
	}
}

/* ---- SV_GetConfigstring  0x00457E10 ---- */
void SV_GetConfigstring( unsigned int index, char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "\x15SV_GetConfigstring: bufferSize == %i", bufferSize );
	}
	if ( index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "\x15SV_GetConfigstring: bad index %i\n", index );
	}

	if ( !sv.configstrings[index] ) {
		buffer[0] = 0;
		return;
	}

	strncpy( buffer, sv.configstrings[index], bufferSize - 1 );
	buffer[bufferSize - 1] = 0;
}

/* ---- SV_GetConfigstringConst  0x00457E90 ---- */
const char *SV_GetConfigstringConst( int max, int start, const char *key ) {
	int i;
	char    **cs;

	if ( max <= 0 ) {
		return sv_emptyString;
	}

	i = 0;
	for ( cs = &sv.configstrings[start] ; **cs ; cs++ ) {
		if ( !Q_stricmp( key, *cs ) ) {
			return sv.configstrings[start + max + i];
		}
		if ( ++i >= max ) {
			return sv_emptyString;
		}
	}

	return sv_emptyString;
}

/* ---- SV_SetConfigValueForKey  0x00457EF0 ---- */
void SV_SetConfigValueForKey( int start, const char *key, const char *value, int max ) {
	int i;
	char    **cs;

	i = 0;
	if ( max > 0 ) {
		cs = &sv.configstrings[start];
		while ( **cs ) {
			if ( !Q_stricmp( key, *cs ) ) {
				break;
			}
			++i;
			++cs;
			if ( i >= max ) {
				break;
			}
		}
		if ( i < max && !**cs ) {
			SV_SetConfigstring( i + start, key );
		}
	}

	if ( i == max ) {
		Com_Error( ERR_DROP, "\x15SV_SetConfigValueForKey: overflow" );
	}

	SV_SetConfigstring( max + start + i, value );
}

/* ---- SV_SetUserinfo  0x00457F70 ---- */
void SV_SetUserinfo( int index, const char *userinfo ) {
	client_t    *cl;

	if ( index < 0 || index >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "\x15SV_SetUserinfo: bad index %i\n", index );
	}

	if ( !userinfo ) {
		userinfo = sv_emptyString;
	}

	cl = &svs.clients[index];
	strncpy( cl->userinfo, userinfo, sizeof( cl->userinfo ) - 1 );
	cl->userinfo[sizeof( cl->userinfo ) - 1] = 0;

	strncpy( cl->name, Info_ValueForKey( userinfo, "name" ), sizeof( cl->name ) - 1 );
	cl->name[sizeof( cl->name ) - 1] = 0;
}

/* ---- SV_GetUserinfo  0x00458000 ---- */
void SV_GetUserinfo( int index, char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "\x15SV_GetUserinfo: bufferSize == %i", bufferSize );
	}
	if ( index < 0 || index >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "\x15SV_GetUserinfo: bad index %i\n", index );
	}

	strncpy( buffer, svs.clients[index].userinfo, bufferSize - 1 );
	buffer[bufferSize - 1] = 0;
}

/* ---- SV_CreateBaseline  0x00458060 ---- */
void SV_CreateBaseline( void ) {
	gentity_t   *svent;
	svEntity_t     *base;
	int entnum;
	int i;

	for ( entnum = 1 ; entnum < sv.num_entities ; entnum++ ) {
		svent = (gentity_t *)( (byte *)sv.gentities + entnum * sv.gentitySize );

		if ( !svent->r.linked ) {
			continue;
		}

		svent->s.number = entnum;

		base = &sv.svEntities[entnum];
		base->baseline = svent->s;
		base->baselineSvFlags = svent->r.svFlags;
		base->baselineSingleClient = svent->r.singleClient;
		for ( i = 0 ; i < 3 ; i++ ) {
			base->baselineAbsmin[i] = svent->r.absmin[i];
			base->baselineAbsmax[i] = svent->r.absmax[i];
		}
	}
}

/* ---- SV_BoundMaxClients  0x00458110 ---- */
void SV_BoundMaxClients( int minimum ) {
	Cvar_Get( "sv_maxclients", "20", 0 );
	sv_maxclients->modified = qfalse;

	if ( sv_maxclients->integer < minimum ) {
		Cvar_Set2( "sv_maxclients", va( "%i", minimum ), qtrue );
	} else if ( sv_maxclients->integer > MAX_CLIENTS ) {
		Cvar_Set2( "sv_maxclients", va( "%i", MAX_CLIENTS ), qtrue );
	}
}

/* ---- SV_SetSnapshotCounts  no-address ---- */
static void SV_SetSnapshotCounts( int clients ) {
	if ( com_dedicated->integer ) {
		svs.numSnapshotEntities = clients << 11;
		svs.numSnapshotClients = clients * clients * 32;
	} else {
		svs.numSnapshotEntities = clients << 8;
		svs.numSnapshotClients = clients * clients * 4;
	}
}

/* ---- SV_Startup  0x00458160 ---- */
void SV_Startup( void ) {
	if ( svs.initialized ) {
		Com_Error( ERR_FATAL, "\x15SV_Startup: svs.initialized" );
	}

	SV_BoundMaxClients( 1 );

	svs.clients = (client_t *)calloc( sv_maxclients->integer, sizeof( client_t ) );
	if ( !svs.clients ) {
		Com_Error( ERR_FATAL, "\x15SV_Startup: unable to allocate svs.clients" );
	}

	SV_SetSnapshotCounts( sv_maxclients->integer );
	svs.initialized = qtrue;
	Cvar_Set2( "sv_running", "1", qtrue );
}

/* ---- SV_ChangeMaxClients  0x00458270 ---- */
void SV_ChangeMaxClients( void ) {
	int i;
	int count;
	int oldMaxClients;
	client_t    *oldClients;

	count = 0;
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED && count < i ) {
			count = i;
		}
	}
	count++;

	oldMaxClients = sv_maxclients->integer;
	SV_BoundMaxClients( count );
	if ( sv_maxclients->integer == oldMaxClients ) {
		return;
	}

	oldClients = (client_t *)Hunk_AllocateTempMemoryInternal( count * sizeof( client_t ) );
	for ( i = 0 ; i < count ; i++ ) {
		if ( svs.clients[i].state < CS_CONNECTED ) {
			Com_Memset( &oldClients[i], 0, sizeof( client_t ) );
		} else {
			Com_Memcpy( &oldClients[i], &svs.clients[i], sizeof( client_t ) );
		}
	}

	free( svs.clients );
	svs.clients = (client_t *)calloc( sv_maxclients->integer, sizeof( client_t ) );
	if ( !svs.clients ) {
		Com_Error( ERR_FATAL, "\x15SV_Startup: unable to allocate svs.clients" );
	}
	Com_Memset( svs.clients, 0, sv_maxclients->integer * sizeof( client_t ) );

	for ( i = 0 ; i < count ; i++ ) {
		if ( oldClients[i].state >= CS_CONNECTED ) {
			Com_Memcpy( &svs.clients[i], &oldClients[i], sizeof( client_t ) );
		}
	}

	Hunk_FreeTempMemoryInternal( oldClients );
	SV_SetSnapshotCounts( sv_maxclients->integer );
}

/* ---- SV_SetExpectedHunkUsage  0x00458450 ---- */
void SV_SetExpectedHunkUsage( const char *mapname ) {
	int len;
	fileHandle_t f;
	char        *buf;
	char        *parse;
	char        *token;

	len = FS_FOpenFileByMode( "hunkusage.dat", &f, FS_READ );
	if ( len < 0 ) {
		Cvar_Set2( "com_expectedhunkusage", "-1", qtrue );
		return;
	}

	buf = (char *)Z_MallocInternal( len + 1 );
	memset( buf, 0, len + 1 );
	FS_Read( buf, len, f );
	FS_FCloseFile( f );

	parse = buf;
	while ( 1 ) {
		token = Com_ParseExt( &parse, qtrue );
		if ( !token || !token[0] ) {
			break;
		}

		if ( Q_stricmp( token, mapname ) ) {
			continue;
		}

		token = Com_ParseExt( &parse, qtrue );
		if ( token && token[0] ) {
			Cvar_Set2( "com_expectedhunkusage", token, qtrue );
			free( buf );
			return;
		}
	}

	free( buf );
	Cvar_Set2( "com_expectedhunkusage", "-1", qtrue );
}

/* ---- SV_ClearServer  0x00458600 ---- */
void SV_ClearServer( void ) {
	int i;

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( sv.configstrings[i] ) {
			free( sv.configstrings[i] );
		}
	}

	Com_Memset( &sv, 0, sizeof( sv ) );
}

/* ---- SV_EnableArchivedSnapshot  0x00458640 ---- */
void SV_EnableArchivedSnapshot( qboolean enable ) {
	svs.archiveEnabled = enable;
	if ( !enable ) {
		return;
	}
	if ( svs.archivedSnapshotFrames ) {
		return;
	}

	svs.cachedSnapshotFrames   = (cachedSnapshot_t *)Z_MallocInternal( SV_NUM_CACHED_SNAPSHOT_FRAMES * sizeof( cachedSnapshot_t ) );
	svs.cachedSnapshotEntities = (archivedEntity_t *)Z_MallocInternal( SV_NUM_CACHED_SNAPSHOT_ENTITIES * sizeof( archivedEntity_t ) );
	svs.archivedSnapshotFrames = (archivedSnapshot_t *)Z_MallocInternal( SV_ARCHIVED_SNAPSHOT_FRAMES_BYTES );
	svs.archivedSnapshotBuffer = (byte *)Z_MallocInternal( SV_ARCHIVED_SNAPSHOT_BUFFER_BYTES );
	svs.cachedSnapshotClients  = (cachedClient_t *)Z_MallocInternal( SV_NUM_CACHED_SNAPSHOT_CLIENTS * sizeof( cachedClient_t ) );
}

/* ---- SV_InitArchivedSnapshot  0x004586B0 ---- */
void SV_InitArchivedSnapshot( void ) {
	svs.archiveEnabled = qfalse;
	svs.nextArchivedSnapshotFrames = 0;
	svs.nextArchivedSnapshotBuffer = 0;
	svs.nextCachedSnapshotEntities = 0;
	svs.nextCachedSnapshotClients = 0;
	svs.nextCachedSnapshotFrames = 0;
}

/* ---- SV_FreeArchivedSnapshot  0x004586E0 ---- */
void SV_FreeArchivedSnapshot( void ) {
	if ( svs.cachedSnapshotFrames ) {
		free( svs.cachedSnapshotFrames );
		svs.cachedSnapshotFrames = NULL;
	}
	if ( svs.cachedSnapshotEntities ) {
		free( svs.cachedSnapshotEntities );
		svs.cachedSnapshotEntities = NULL;
	}
	if ( svs.archivedSnapshotFrames ) {
		free( svs.archivedSnapshotFrames );
		svs.archivedSnapshotFrames = NULL;
	}
	if ( svs.archivedSnapshotBuffer ) {
		free( svs.archivedSnapshotBuffer );
		svs.archivedSnapshotBuffer = NULL;
	}
	if ( svs.cachedSnapshotClients ) {
		free( svs.cachedSnapshotClients );
		svs.cachedSnapshotClients = NULL;
	}
}

/* ---- SV_SpawnServer  0x00458760 ---- */
void SV_SpawnServer( const char *server, qboolean killBots ) {
	int i;
	int checksum;
	int matchState;
	char systemInfo[BIG_INFO_STRING];
	client_t    *client;
	cvar_t      *var;

	( void )killBots;

	xmodel_enforceExist = Cvar_Get( "cl_xmodelcheck", "0", CVAR_ARCHIVE | CVAR_LATCH )->integer;

	var = Cvar_FindVar( "sv_running" );
	if ( var && var->value != 0.0f ) {
		matchState = VM_Call( vm, GAME_GET_MATCH_STATE );

		for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
			client = &svs.clients[i];
			if ( client->state >= CS_PRIMED ) {
				NET_OutOfBandPrint( NS_SERVER, client->netchan.remoteAddress,
									"loadingnewmap\n%s\n%s", server,
									sv_gametype->string );
			}
		}

		NET_Sleep( 250 );
	} else {
		matchState = 0;
	}

	CL_MapLoading();
	CL_ShutdownCGame();
	CL_ShutdownUI();
	if ( com_serverEndpoint ) {
		com_serverEndpoint( 0 );
	}
	cls_loadingPlaque = 0;
	cls_rendererStarted = 0;
	SV_ShutdownGameProgs();

	Com_Printf( "------ Server Initialization ------\n" );
	Com_Printf( "Server: %s\n", server );

	CL_ShutdownCGame();
	CL_ShutdownUI();
	SV_ShutdownGameProgs();

	Hunk_ClearToStart();
	VM_Clear();
	SV_ClearServer();

	FS_Shutdown( qtrue );
	FS_ClearPakReferences( 0 );

	srand( Sys_Milliseconds() );
	checksum = Sys_Milliseconds();
	checksum ^= rand() << 16;
	checksum ^= rand();
	sv.checksumFeed = checksum;
	FS_Restart( sv.checksumFeed );

	FS_ClearMemory();

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		sv.configstrings[i] = CopyStringInternal( sv_emptyString );
	}

	var = Cvar_FindVar( "sv_running" );
	if ( var && var->value != 0.0f ) {
		com_dobjPoolInited = 1;
		com_dobjInited = 1;
		if ( sv_maxclients->modified ) {
			SV_ChangeMaxClients();
		}
	} else {
		Com_Restart();
		SV_Startup();
	}

	svs.snapshotEntities = (entityState_t *)Hunk_AllocAlignInternal(
		sizeof( entityState_t ) * svs.numSnapshotEntities, 32 );
	svs.nextSnapshotEntities = 0;
	svs.snapshotClients = Hunk_AllocAlignInternal(
		SV_SNAPSHOT_CLIENT_SIZE * svs.numSnapshotClients, 32 );
	svs.nextSnapshotClients = 0;
	SV_InitArchivedSnapshot();

	svs.snapFlagServerBit ^= 4;

	Cvar_Set2( "nextmap", "map_restart", qtrue );
	SV_SetExpectedHunkUsage( va( "maps/mp/%s.bsp", server ) );
	Cvar_Set2( "cl_paused", "0", qtrue );

	CM_LoadMap( va( "maps/mp/%s.bsp", server ), qfalse, &checksum );
	Cvar_Set2( "mapname", server, qtrue );

	sv_rollingServerId = (unsigned char)( sv_rollingServerId + 16 );
	if ( ( sv_rollingServerId & 0xF0 ) == 0 ) {
		sv_rollingServerId += 16;
	}
	Cvar_Set2( "sv_serverid", va( "%i", sv_rollingServerId ), qtrue );

	sv.serverId = com_frameTime;
	sv.state = SS_LOADING;
	Cvar_Set2( "sv_serverRestarting", "1", qtrue );

	Com_LoadSoundAliases( va( "maps/mp/%s.bsp", server ), 2 );

	xanim_activePoolSlot = 1;
	vm = VM_Create( "game", (int ( * )( int * ))SV_GameSystemCalls );
	if ( !vm ) {
		Com_Error( ERR_FATAL, "\x15VM_Create on game failed" );
	}
	SV_HUNK_GAME_BASE = SV_HUNK_LOW_PERM;
	SV_InitGameVM( qfalse, matchState );

	for ( i = 0 ; i < 3 ; i++ ) {
		svs.time += 100;
		VM_Call( vm, GAME_UPDATE_CVARS );
		if ( !++com_skelTimeStamp ) {
			com_skelTimeStamp = 1;
		}
		com_skelInvalidated = 1;
		VM_Call( vm, GAME_RUN_FRAME, svs.time );
		com_skelInvalidated = 0;
		if ( s_hunkData ) {
			SV_HUNK_LOW_TEMP = SV_HUNK_LOW_PERM;
		}
	}

	SV_CreateBaseline();

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		const char  *reject;

		client = &svs.clients[i];
		if ( client->state < CS_CONNECTED ) {
			continue;
		}

		reject = (const char *)VM_Call( vm, GAME_CLIENT_CONNECT, i, client->scriptId );
		if ( reject ) {
			SV_DropClient( client, reject );
		} else {
			client->state = CS_CONNECTED;
		}
	}

	if ( sv_pure->integer ) {
		const char  *sums = FS_LoadedPakChecksums();

		Cvar_Set2( "sv_paks", sums, qtrue );
		if ( !sums[0] ) {
			Com_Printf( "WARNING: sv_pure set but no PK3 files loaded\n" );
		}
		Cvar_Set2( "sv_pakNames", FS_LoadedPakNames(), qtrue );
	} else {
		Cvar_Set2( "sv_paks", sv_emptyString, qtrue );
		Cvar_Set2( "sv_pakNames", sv_emptyString, qtrue );
	}

	Cvar_Set2( "sv_referencedPaks", FS_ReferencedPakChecksums(), qtrue );
	Cvar_Set2( "sv_referencedPakNames", FS_ReferencedPakNames(), qtrue );

	strncpy( systemInfo, Cvar_InfoString_Big( CVAR_SYSTEMINFO ), sizeof( systemInfo ) - 1 );
	systemInfo[sizeof( systemInfo ) - 1] = 0;
	cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
	SV_SetConfigstring( CS_SYSTEMINFO, systemInfo );

	SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
	cvar_modifiedFlags &= ~CVAR_SERVERINFO;

	SV_SetConfig( CS_SCRIPT_KV_BASE, CS_SCRIPT_KV_COUNT, CVAR_WOLFINFO );
	cvar_modifiedFlags &= ~CVAR_WOLFINFO;

	sv.state = SS_GAME;
	svs.nextHeartbeatTime = -9999999;
	Cvar_Set2( "sv_serverRestarting", "0", qtrue );
	Com_Printf( "-----------------------------------\n" );
}

/* ---- SV_Init  0x00459130 ---- */
void SV_Init( void ) {
	SV_AddOperatorCommands();

	sv_gametype = Cvar_Get( "g_gametype", "dm", CVAR_LATCH | CVAR_SERVERINFO );
	Cvar_Get( "sv_keywords", "", CVAR_SERVERINFO );
	Cvar_Get( "protocol", va( "%i", PROTOCOL_VERSION ), CVAR_ROM | CVAR_SERVERINFO );
	Cvar_Get( "xtndedbuild", va( "%i", OPENCOD_EXTENDED_BUILD ),
		CVAR_SERVERINFO | CVAR_ROM | CVAR_NORESTART );
	sv_mapname = Cvar_Get( "mapname", "nomap", CVAR_ROM | CVAR_SERVERINFO );
	sv_privateClients = Cvar_Get( "sv_privateClients", "0", CVAR_SERVERINFO );
	sv_hostname = Cvar_Get( "sv_hostname", "CoDHost", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maxclients = Cvar_Get( "sv_maxclients", "20", CVAR_LATCH | CVAR_SERVERINFO );

	sv_maxRate = Cvar_Get( "sv_maxRate", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_minPing = Cvar_Get( "sv_minPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maxPing = Cvar_Get( "sv_maxPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_floodProtect = Cvar_Get( "sv_floodProtect", "1", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_allowAnonymous = Cvar_Get( "sv_allowAnonymous", "0", CVAR_SERVERINFO );
	sv_showCommands = Cvar_Get( "sv_showCommands", "0", 0 );

	Cvar_Get( "sv_cheats", "1", CVAR_ROM | CVAR_SYSTEMINFO );
	sv_serverid = Cvar_Get( "sv_serverid", "0", CVAR_ROM | CVAR_SYSTEMINFO );
	sv_pure = Cvar_Get( "sv_pure", "1", CVAR_SERVERINFO | CVAR_SYSTEMINFO );
	Cvar_Get( "sv_paks", "", CVAR_ROM | CVAR_SYSTEMINFO );
	Cvar_Get( "sv_pakNames", "", CVAR_ROM | CVAR_SYSTEMINFO );
	Cvar_Get( "sv_referencedPaks", "", CVAR_ROM | CVAR_SYSTEMINFO );
	Cvar_Get( "sv_referencedPakNames", "", CVAR_ROM | CVAR_SYSTEMINFO );

	sv_rconPassword = Cvar_Get( "rconPassword", "", CVAR_TEMP );
	sv_privatePassword = Cvar_Get( "sv_privatePassword", "", CVAR_TEMP );
	sv_fps = Cvar_Get( "sv_fps", "20", CVAR_TEMP );
	sv_timeout = Cvar_Get( "sv_timeout", "240", CVAR_TEMP );
	sv_zombietime = Cvar_Get( "sv_zombietime", "2", CVAR_TEMP );
	Cvar_Get( "nextmap", "", CVAR_TEMP );

	sv_allowDownload = Cvar_Get( "sv_allowDownload", "1", CVAR_ARCHIVE );
	Cvar_Get( "sv_wwwDownload", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	Cvar_Get( "sv_wwwBaseURL", "", CVAR_ARCHIVE | CVAR_SERVERINFO );

	sv_master[0] = sv_master_0_ = Cvar_Get( "sv_master1", "codmaster.activision.com", 0 );
	sv_master[1] = sv_master_1_ = Cvar_Get( "sv_master2", "", CVAR_ARCHIVE );
	sv_master[2] = sv_master_2_ = Cvar_Get( "sv_master3", "", CVAR_ARCHIVE );
	sv_master[3] = sv_master_3_ = Cvar_Get( "sv_master4", "", CVAR_ARCHIVE );
	sv_master[4] = sv_master_4_ = Cvar_Get( "sv_master5", "", CVAR_ARCHIVE );

	sv_reconnectlimit = Cvar_Get( "sv_reconnectlimit", "3", 0 );
	sv_showloss = Cvar_Get( "sv_showloss", "0", 0 );
	sv_padPackets = Cvar_Get( "sv_padPackets", "0", 0 );
	sv_killserver = Cvar_Get( "sv_killserver", "0", 0 );
	sv_onlyVisibleClients = Cvar_Get( "sv_onlyVisibleClients", "0", 0 );
	sv_showAverageBPS = Cvar_Get( "sv_showAverageBPS", "0", 0 );

	Cvar_Get( "g_complaintlimit", "3", CVAR_ARCHIVE );
	sv_mapRotation = Cvar_Get( "sv_mapRotation", "", 0 );
	sv_mapRotationCurrent = Cvar_Get( "sv_mapRotationCurrent", "", 0 );

	sv_debugSpawn = Cvar_Get( "sv_debugSpawn", "0", 0 );
	if ( sv_debugSpawn->integer ) {
		Com_Printf( "SPAWNDBG: tracing ON\n" );
	}

	Cmd_AddCommand( "addtestclient", SV_AddTestClient );

	Cmd_AddCommand( "testclientcmd", SV_TestClientCommand_f );
	Cmd_AddCommand( "testclientmove", SV_TestClientMove_f );
	Cmd_AddCommand( "testdumpmissiles", SV_TestDumpMissiles_f );
}

/* ---- SV_FinalMessage  0x00459550 ---- */
void SV_FinalMessage( const char *message ) {
	int i, j;
	client_t    *cl;

	for ( j = 0 ; j < 2 ; j++ ) {
		for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
			cl = &svs.clients[i];
			if ( cl->state < CS_CONNECTED ) {
				continue;
			}

			if ( cl->netchan.remoteAddress.type != NA_LOOPBACK ) {
				SV_SendServerCommand( cl, qtrue, "e \"%s\"", message );
				SV_SendServerCommand( cl, qtrue, "w" );
			}

			cl->nextSnapshotTime = -1;
			SV_SendClientSnapshot( cl );
		}
	}
}

/* ---- SV_FreeClients  0x004595B0 ---- */
void SV_FreeClients( void ) {
	int i;
	client_t    *cl;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state < CS_CONNECTED ) {
			continue;
		}
		SV_FreeClientScriptId( cl );
	}

	free( svs.clients );
}

/* ---- SV_Shutdown  0x00459600 ---- */
void SV_Shutdown( const char *finalmsg ) {
	if ( !com_sv_running || !com_sv_running->integer ) {
		return;
	}

	Com_Printf( "----- Server Shutdown -----\n" );

	if ( svs.clients && !com_errorEntered ) {
		SV_FinalMessage( finalmsg );
	}

	svs.nextHeartbeatTime = -9999;
	SV_MasterHeartbeat( "flatline" );

	SV_ShutdownGameProgs();
	SV_ClearServer();

	if ( svs.clients ) {
		SV_FreeClients();
	}
	SV_FreeArchivedSnapshot();

	Com_Memset( &svs, 0, sizeof( svs ) );

	Cvar_Set2( "sv_running", "0", qtrue );
	Com_Printf( "---------------------------\n" );

	CL_Disconnect( qfalse );
}
