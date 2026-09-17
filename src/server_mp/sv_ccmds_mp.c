/*
 * @fidelity: verified
 */

#include "server.h"

#include <string.h>
#include <stdlib.h>

/* Retail 0x008E1DD4: the file-static "commands already registered" latch. */
static qboolean sv_commandsInitialized;

extern void SV_RestartGameProgs( int matchState );
extern void Scr_DumpScriptThreads( void );
extern void MT_DumpTree( void );

/* svs.snapFlagServerBit toggles this bit on every spawn/restart; the retail constant is `xor edi, 4` at 0x00451F7E. */
#define SV_SNAPFLAG_SERVERCOUNT     4

client_t *SV_GetPlayerByName( void );
client_t *SV_GetPlayerByNum( void );
void SV_Map_f( void );
void SV_MapRestart_f( void );
char *SV_GetMapRotationToken( void );
void SV_MapRotate_f( void );
void SV_Kick_f( void );
void SV_Ban_f( void );
void SV_BanNum_f( void );
void SV_KickNum_f( void );
void SV_Status_f( void );
void SV_ConSay_f( void );
void SV_Heartbeat_f( void );
void SV_Serverinfo_f( void );
void SV_Systeminfo_f( void );
void SV_DumpUser_f( void );
void SV_KillServer_f( void );
void SV_ScriptUsage_f( void );
void SV_StringUsage_f( void );
void SV_RemoveOperatorCommands( void );

#define SV_STRICMP_N    99999

/* ---- SV_GetPlayerByName  0x00451A80 ---- */
client_t *SV_GetPlayerByName( void ) {
	client_t    *cl;
	int i;
	const char  *s;
	char cleanName[64];

	if ( !com_sv_running->integer ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv( 1 );

	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( !cl->state ) {
			continue;
		}
		if ( !Q_stricmpn( cl->name, s, SV_STRICMP_N ) ) {
			return cl;
		}

		strncpy( cleanName, cl->name, sizeof( cleanName ) - 1 );
		cleanName[sizeof( cleanName ) - 1] = 0;
		Q_CleanStr( cleanName );

		if ( !Q_stricmpn( cleanName, s, SV_STRICMP_N ) ) {
			return cl;
		}
	}

	Com_Printf( "Player %s is not on the server\n", s );
	return NULL;
}

/* ---- SV_GetPlayerByNum  0x00451BA0 ---- */
client_t *SV_GetPlayerByNum( void ) {
	client_t    *cl;
	int i;
	int idnum;
	const char  *s;

	if ( !com_sv_running->integer ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv( 1 );

	for ( i = 0 ; s[i] ; i++ ) {
		if ( s[i] < '0' || s[i] > '9' ) {
			Com_Printf( "Bad slot number: %s\n", s );
			return NULL;
		}
	}

	idnum = atoi( s );
	if ( idnum < 0 || idnum >= sv_maxclients->integer ) {
		Com_Printf( "Bad client slot: %i\n", idnum );
		return NULL;
	}

	cl = &svs.clients[idnum];
	if ( cl->state == CS_FREE ) {
		Com_Printf( "Client %i is not active\n", idnum );
		return NULL;
	}
	return cl;
}

/* ---- SV_Map_f  0x00451C60 ---- */
void SV_Map_f( void ) {
	const char  *map;
	qboolean cheats;
	char expanded[MAX_QPATH];
	char mapname[MAX_QPATH];

	map = Cmd_Argv( 1 );
	if ( !map ) {
		return;
	}

	if ( Q_stricmpn( map, "mp/", 3 ) && Q_stricmpn( map, "mp\\", 3 ) ) {
		Com_sprintf( expanded, sizeof( expanded ), "maps/mp/%s.bsp", map );
	} else {
		Com_sprintf( expanded, sizeof( expanded ), "maps/%s.bsp", map );
	}

	if ( FS_ReadFile( expanded, NULL ) == -1 ) {
		Com_Printf( "Can't find map %s\n", expanded );
		return;
	}

	cheats = ( Q_stricmpn( Cmd_Argv( 0 ), "devmap", SV_STRICMP_N ) == 0 );

	if ( Q_stricmpn( map, "mp/", 3 ) && Q_stricmpn( map, "mp\\", 3 ) ) {
		strncpy( mapname, map, 63 );
		mapname[63] = 0;
	} else {
		strncpy( mapname, map + 3, 60 );
		mapname[60] = 0;
	}

	if ( com_sv_running->integer && !Q_stricmp( mapname, sv_mapname->string ) ) {
		SV_MapRestart_f();
	} else {
		SV_SpawnServer( mapname, qfalse );
	}

	Cvar_Set2( "sv_cheats", cheats ? "1" : "0", qtrue );
}

/* ---- SV_MapRestart_f  0x00451E20 ---- */
void SV_MapRestart_f( void ) {
	int i;
	client_t    *client;
	const char  *denied;
	char mapname[MAX_QPATH];
	int matchState;

	if ( com_frameTime == sv.serverId ) {
		return;
	}

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	/* Retail keeps this in ESI across the whole function and pushes it into SV_RestartGameProgs at 0x00451FF8; it is not just a predicate. */
	matchState = VM_Call( vm, GAME_GET_MATCH_STATE );

	if ( !matchState ) {
		if ( sv_gametype->latchedString
			 && Q_stricmp( sv_gametype->latchedString, sv_gametype->string ) ) {
			Com_Printf( "g_gametype variable change -- restarting.\n" );
			strncpy( mapname, Cvar_VariableString( "mapname" ), 63 );
			mapname[63] = 0;
			SV_SpawnServer( mapname, qfalse );
			return;
		}

		if ( sv_maxclients->modified ) {
			cvar_t  *var;

			Com_Printf( "sv_maxclients variable change -- restarting.\n" );
			var = Cvar_FindVar( "mapname" );
			strncpy( mapname, var ? var->string : "", 63 );
			mapname[63] = 0;
			SV_SpawnServer( mapname, qfalse );
			return;
		}
	}

	svs.nextSnapshotEntities = 0;
	svs.nextSnapshotClients = 0;
	svs.nextArchivedSnapshotBuffer = 0;
	svs.nextCachedSnapshotEntities = 0;
	svs.nextCachedSnapshotClients = 0;
	svs.nextCachedSnapshotFrames = 0;
	svs.snapFlagServerBit ^= SV_SNAPFLAG_SERVERCOUNT;

	sv_rollingServerId = ( sv_rollingServerId & 0xF0 ) + ( ( sv_rollingServerId + 1 ) & 0x0F );
	Cvar_Set2( "sv_serverid", va( "%i", sv_rollingServerId ), qtrue );

	sv.serverId = com_frameTime;
	sv.state = SS_LOADING;
	sv.restarting = qtrue;
	Cvar_Set2( "sv_serverRestarting", "1", qtrue );

	xanim_activePoolSlot = 1;
	SV_RestartGameProgs( matchState );

	for ( i = 0 ; i < 3 ; i++ ) {
		svs.time += 100;
		SV_RunFrame();
	}

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		client = &svs.clients[i];

		if ( client->state < CS_CONNECTED ) {
			continue;
		}

		SV_AddServerCommand( client, 1, "n" );

		denied = (const char *)VM_Call( vm, GAME_CLIENT_CONNECT, i,
										client->scriptId );
		if ( denied ) {
			SV_DropClient( client, denied );
			Com_Printf( "SV_MapRestart_f: dropped client %i - denied!\n", i );
			continue;
		}

		if ( client->state == CS_ACTIVE ) {
			SV_ClientEnterWorld( client, &client->lastUsercmd );
		}
	}

	sv.state = SS_GAME;
	sv.restarting = qfalse;
	Cvar_Set2( "sv_serverRestarting", "0", qtrue );
}

/* ---- SV_GetMapRotationToken  0x00452110 ---- */
char *SV_GetMapRotationToken( void ) {
	char    *data_p;
	char    *token;
	char remainder[MAX_STRING_CHARS];

	data_p = sv_mapRotationCurrent->string;

	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}

	token = Com_ParseExt( &data_p, qtrue );

	if ( !data_p ) {
		Cvar_Set2( "sv_mapRotationCurrent", "", qtrue );
		return NULL;
	}

	Q_strncpyz( remainder, data_p, sizeof( remainder ) );
	Cvar_Set2( "sv_mapRotationCurrent", remainder, qtrue );
	return token;
}

/* ---- SV_MapRotate_f  0x004521E0 ---- */
void SV_MapRotate_f( void ) {
	char    *token;
	char    *value;

	Com_Printf( "map_rotate...\n\n" );
	Com_Printf( "\"sv_mapRotation\" is:\"%s\"\n\n", sv_mapRotation->string );
	Com_Printf( "\"sv_mapRotationCurrent\" is:\"%s\"\n\n",
				sv_mapRotationCurrent->string );

	if ( !sv_mapRotationCurrent->string[0] ) {
		Cvar_Set2( "sv_mapRotationCurrent", sv_mapRotation->string, qtrue );
	}

	token = SV_GetMapRotationToken();
	if ( !token ) {
		Cvar_Set2( "sv_mapRotationCurrent", sv_mapRotation->string, qtrue );
		token = SV_GetMapRotationToken();
	}

	while ( token ) {
		if ( !Q_stricmp( token, "gametype" ) ) {
			value = SV_GetMapRotationToken();
			if ( !value ) {
				Com_Printf( "No gametype specified after 'gametype' keyword "
							"in sv_mapRotation - forcing map_restart.\n" );
				SV_MapRestart_f();
				return;
			}

			Com_Printf( "Setting g_gametype: %s.\n", value );
			if ( com_sv_running->integer
				 && Q_stricmp( sv_gametype->string, value ) ) {
				VM_Call( vm, GAME_UPDATE_CVARS, 0 );
			}
			Cvar_Set2( "g_gametype", value, qtrue );

			token = SV_GetMapRotationToken();
			continue;
		}

		if ( Q_stricmp( token, "map" ) ) {
			Com_Printf( "Unknown keyword '%s' in sv_mapRotation.\n", token );
			token = SV_GetMapRotationToken();
			continue;
		}

		value = SV_GetMapRotationToken();
		if ( !value ) {
			Com_Printf( "No map specified after 'map' keyword in "
						"sv_mapRotation - forcing map_restart.\n" );
			SV_MapRestart_f();
			return;
		}

		Com_Printf( "Setting map: %s.\n", value );
		Cbuf_ExecuteText( EXEC_NOW, va( "map %s\n", value ) );
		return;
	}

	Com_Printf( "No map specified in sv_mapRotation - forcing map_restart.\n" );
	SV_MapRestart_f();
}

/* ---- SV_Kick_f  0x004523C0 ---- */
void SV_Kick_f( void ) {
	client_t    *cl;
	int i;

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: kick <player name>\nkick all = kick everyone\n" );
		return;
	}

	cl = SV_GetPlayerByName();
	if ( cl ) {
		if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
			SV_SendServerCommand( NULL, 0, "e \"EXE_CANNOTKICKHOSTPLAYER\"" );
			return;
		}
		SV_DropClient( cl, "EXE_PLAYERKICKED" );
		cl->lastPacketTime = svs.time;
		return;
	}

	if ( Q_stricmpn( Cmd_Argv( 1 ), "all", SV_STRICMP_N ) ) {
		return;
	}

	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( !cl->state ) {
			continue;
		}
		if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
			continue;
		}
		SV_DropClient( cl, "EXE_PLAYERKICKED" );
		cl->lastPacketTime = svs.time;
	}
}

/* ---- SV_BanClient  no-address ---- */
static void SV_BanClient( client_t *cl ) {
	if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand( NULL, 0, "e \"EXE_CANNOTKICKHOSTPLAYER\"" );
		return;
	}

	if ( !svs.authorizeAddress.ip[0] ) {
		if ( svs.authorizeAddress.type == NA_BAD ) {
			return;
		}
		Com_Printf( "Resolving %s\n", "codauthorize.activision.com" );
		if ( NET_StringToAdr( "codauthorize.activision.com",
							  &svs.authorizeAddress ) == qfalse ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}
		svs.authorizeAddress.port = BigShort( (short)PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n",
					"codauthorize.activision.com",
					svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
					svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
					BigShort( (short)svs.authorizeAddress.port ) );
	}

	if ( svs.authorizeAddress.type == NA_BAD ) {
		return;
	}

	NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,
						"banUser %i.%i.%i.%i",
						cl->netchan.remoteAddress.ip[0],
						cl->netchan.remoteAddress.ip[1],
						cl->netchan.remoteAddress.ip[2],
						cl->netchan.remoteAddress.ip[3] );
	Com_Printf( "%s was banned from coming back\n", cl->name );
}

/* ---- SV_Ban_f  0x004524D0 ---- */
void SV_Ban_f( void ) {
	client_t    *cl;

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: banUser <player name>\n" );
		return;
	}

	cl = SV_GetPlayerByName();
	if ( cl ) {
		SV_BanClient( cl );
	}
}

/* ---- SV_BanNum_f  0x00452640 ---- */
void SV_BanNum_f( void ) {
	client_t    *cl;

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: banClient <client number>\n" );
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( cl ) {
		SV_BanClient( cl );
	}
}

/* ---- SV_KickNum_f  0x004527B0 ---- */
void SV_KickNum_f( void ) {
	client_t    *cl;

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: kicknum <client number>\n" );
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}

	if ( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		SV_SendServerCommand( NULL, 0, "e \"EXE_CANNOTKICKHOSTPLAYER\"" );
		return;
	}

	SV_DropClient( cl, "EXE_PLAYERKICKED" );
	cl->lastPacketTime = svs.time;
}

/* ---- SV_Status_f  0x00452830 ---- */
void SV_Status_f( void ) {
	int i;
	int j;
	int l;
	int ping;
	int score;
	client_t        *cl;
	const char      *s;

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	Com_Printf( "map: %s\n", sv_mapname->string );
	Com_Printf( "num score ping name            lastmsg address               qport rate\n" );
	Com_Printf( "--- ----- ---- --------------- ------- --------------------- ----- -----\n" );

	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( !cl->state ) {
			continue;
		}

		Com_Printf( "%3i ", i );

		score = vm ? VM_Call( vm, GAME_GET_CLIENT_SCORE, cl - svs.clients ) : 0;
		Com_Printf( "%5i ", score );

		if ( cl->state == CS_CONNECTED ) {
			Com_Printf( "CNCT " );
		} else if ( cl->state == CS_ZOMBIE ) {
			Com_Printf( "ZMBI " );
		} else {
			ping = cl->ping;
			if ( ping > 9999 ) {
				ping = 9999;
			}
			Com_Printf( "%4i ", ping );
		}

		Com_Printf( "%s", cl->name );
		l = 16 - (int)strlen( cl->name );
		for ( j = 0 ; j < l ; j++ ) {
			Com_Printf( " " );
		}

		Com_Printf( "%7i ", svs.time - cl->lastPacketTime );

		s = NET_AdrToString( cl->netchan.remoteAddress );
		Com_Printf( "%s", s );
		l = 22 - (int)strlen( s );
		for ( j = 0 ; j < l ; j++ ) {
			Com_Printf( " " );
		}

		Com_Printf( "%5i", cl->netchan.qport );
		Com_Printf( " %5i", cl->rate );
		Com_Printf( "\n" );
	}

	Com_Printf( "\n" );
}

/* ---- SV_ConSay_f  0x00452A60 ---- */
void SV_ConSay_f( void ) {
	char    *p;
	char text[1024];

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() < 2 ) {
		return;
	}

	strcpy( text, "console: " );
	p = Cmd_Args();

	if ( *p == '"' ) {
		p++;
		p[strlen( p ) - 1] = 0;
	}

	strcat( text, p );

	SV_SendServerCommand( NULL, 0, "h \"\x15%s\"", text );
}

/* ---- SV_Heartbeat_f  0x00452B60 ---- */
void SV_Heartbeat_f( void ) {
	svs.nextHeartbeatTime = -9999999;
}

/* ---- SV_Serverinfo_f  0x00452B70 ---- */
void SV_Serverinfo_f( void ) {
	Com_Printf( "Server info settings:\n" );
	Info_Print( Cvar_InfoString( CVAR_SERVERINFO ) );
}

/* ---- SV_Systeminfo_f  0x00452B90 ---- */
void SV_Systeminfo_f( void ) {
	Com_Printf( "System info settings:\n" );
	Info_Print( Cvar_InfoString( CVAR_SYSTEMINFO ) );
}

/* ---- SV_DumpUser_f  0x00452BB0 ---- */
void SV_DumpUser_f( void ) {
	client_t    *cl;

	if ( !com_sv_running->integer ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: info <userid>\n" );
		return;
	}

	cl = SV_GetPlayerByName();
	if ( !cl ) {
		return;
	}

	Com_Printf( "userinfo\n" );
	Com_Printf( "--------\n" );
	Info_Print( cl->userinfo );
}

/* ---- SV_KillServer_f  0x00452C10 ---- */
void SV_KillServer_f( void ) {
	Com_Shutdown( "EXE_SERVERKILLED" );
}

/* ---- SV_GameCompleteStatus_f  0x00452C20 ---- */
static void SV_GameCompleteStatus_f( void ) {
	SV_MasterGameCompleteStatus();
}

/* ---- SV_ScriptUsage_f  0x00452C30 ---- */
void SV_ScriptUsage_f( void ) {
	Scr_DumpScriptThreads();
}

/* ---- SV_StringUsage_f  0x00452C40 ---- */
void SV_StringUsage_f( void ) {
	MT_DumpTree();
}

/* ---- SV_AddOperatorCommands  0x00452C50 ---- */
void SV_AddOperatorCommands( void ) {
	if ( sv_commandsInitialized ) {
		return;
	}
	sv_commandsInitialized = qtrue;

	Cmd_AddCommand( "heartbeat", SV_Heartbeat_f );
	Cmd_AddCommand( "kick", SV_Kick_f );
	Cmd_AddCommand( "banUser", SV_Ban_f );
	Cmd_AddCommand( "banClient", SV_BanNum_f );
	Cmd_AddCommand( "clientkick", SV_KickNum_f );
	Cmd_AddCommand( "status", SV_Status_f );
	Cmd_AddCommand( "serverinfo", SV_Serverinfo_f );
	Cmd_AddCommand( "systeminfo", SV_Systeminfo_f );
	Cmd_AddCommand( "dumpuser", SV_DumpUser_f );
	Cmd_AddCommand( "map_restart", SV_MapRestart_f );
	Cmd_AddCommand( "map", SV_Map_f );
	Cmd_AddCommand( "map_rotate", SV_MapRotate_f );
	Cmd_AddCommand( "gameCompleteStatus", SV_GameCompleteStatus_f );
	Cmd_AddCommand( "devmap", SV_Map_f );
	Cmd_AddCommand( "killserver", SV_KillServer_f );

	if ( com_dedicated->integer ) {
		Cmd_AddCommand( "say", SV_ConSay_f );
	}

	Cmd_AddCommand( "scriptUsage", SV_ScriptUsage_f );
	Cmd_AddCommand( "stringUsage", SV_StringUsage_f );
}

/* ---- SV_RemoveOperatorCommands  0x00452D90 ---- */
void SV_RemoveOperatorCommands( void ) {
}
