/*
 * @fidelity: likely
 */

#include "server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* The rcon output sink is 0x3FF0 bytes on SVC_RemoteCommand's stack (0x0045A711 `mov dword_8AAB58, 3FF0h`), i.e. MAX_MSGLEN - 16. */
#define SV_OUTPUTBUF_LENGTH     ( MAX_MSGLEN - 16 )

/* 0x0045A6AF and 0x0045A5F8: two rcon packets inside 500 ms and the second is ignored outright, password or not. */
#define SV_RCON_THROTTLE_MS     500

/* ---- SVC_Status  0x00459EE0 ---- */
void SVC_Status( netadr_t from ) {
	char player[1024];
	char status[MAX_MSGLEN];
	int i;
	client_t    *cl;
	int statusLength;
	int playerLength;
	char infostring[MAX_INFO_STRING];
	char keywords[MAX_INFO_STRING];
	cvar_t      *var;

	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO ) );

	Info_SetValueForKey( infostring, "challenge", Cmd_Argv( 1 ) );

	var = Cvar_FindVar( "fs_restrict" );
	if ( var && var->value != 0.0f ) {
		Com_sprintf( keywords, sizeof( keywords ), "demo %s",
					 Info_ValueForKey( infostring, "sv_keywords" ) );
		Info_SetValueForKey( infostring, "sv_keywords", keywords );
	}

	status[0] = 0;
	statusLength = 0;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state < CS_CONNECTED ) {
			continue;
		}
		Com_sprintf( player, sizeof( player ), "%i %i \"%s\"\n",
					 SV_GetClientScore( cl ), cl->ping, cl->name );
		playerLength = (int)strlen( player );
		/* SIGNEDNESS: retail's test is UNSIGNED -- `cmp esi, 4000h` / `jnb` at 0x0045A045, esi being playerLength + statusLength. Do not add an `(int)` cast; that makes it a signed compare. */
		if ( (unsigned int)( statusLength + playerLength ) >= sizeof( status ) ) {
			break;
		}
		strcpy( status + statusLength, player );
		statusLength += playerLength;
	}

	var = Cvar_FindVar( "g_password" );
	if ( var && var->string && var->string[0] ) {
		Info_SetValueForKey( infostring, "pswrd", "1" );
	} else {
		Info_SetValueForKey( infostring, "pswrd", "0" );
	}

	NET_OutOfBandPrint( NS_SERVER, from, "statusResponse\n%s\n%s",
						infostring, status );
}

/* ---- SVC_GameCompleteStatus  0x0045A120 ---- */
void SVC_GameCompleteStatus( netadr_t from ) {
	char player[1024];
	char status[MAX_MSGLEN];
	int i;
	client_t    *cl;
	int statusLength;
	int playerLength;
	char infostring[MAX_INFO_STRING];
	char keywords[MAX_INFO_STRING];
	cvar_t      *var;

	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO ) );

	Info_SetValueForKey( infostring, "challenge", Cmd_Argv( 1 ) );

	var = Cvar_FindVar( "fs_restrict" );
	if ( var && var->value != 0.0f ) {
		Com_sprintf( keywords, sizeof( keywords ), "demo %s",
					 Info_ValueForKey( infostring, "sv_keywords" ) );
		Info_SetValueForKey( infostring, "sv_keywords", keywords );
	}

	status[0] = 0;
	statusLength = 0;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state < CS_CONNECTED ) {
			continue;
		}
		Com_sprintf( player, sizeof( player ), "%i %i \"%s\"\n",
					 SV_GetClientScore( cl ), cl->ping, cl->name );
		playerLength = (int)strlen( player );
		/* SIGNEDNESS: unsigned in retail -- `cmp esi, 4000h` / `jnb` at 0x0045A285, the twin of SVC_Status' test above. */
		if ( (unsigned int)( statusLength + playerLength ) >= sizeof( status ) ) {
			break;
		}
		strcpy( status + statusLength, player );
		statusLength += playerLength;
	}

	NET_OutOfBandPrint( NS_SERVER, from, "gameCompleteStatus\n%s\n%s",
						infostring, status );
}

/* ---- SVC_Info  0x0045A320 ---- */
void SVC_Info( netadr_t from ) {
	int i;
	int count;
	char infostring[MAX_INFO_STRING];
	cvar_t      *var;

	count = 0;
	for ( i = sv_privateClients->integer ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			count++;
		}
	}

	infostring[0] = 0;

	Info_SetValueForKey( infostring, "challenge", Cmd_Argv( 1 ) );

	Info_SetValueForKey( infostring, "protocol", va( "%i", PROTOCOL_VERSION ) );
	Info_SetValueForKey( infostring, "hostname", sv_hostname->string );
	Info_SetValueForKey( infostring, "mapname", sv_mapname->string );
	Info_SetValueForKey( infostring, "clients", va( "%i", count ) );
	Info_SetValueForKey( infostring, "sv_maxclients",
						 va( "%i", sv_maxclients->integer
									- sv_privateClients->integer ) );
	Info_SetValueForKey( infostring, "gametype", sv_gametype->string );
	Info_SetValueForKey( infostring, "pure", va( "%i", sv_pure->integer ) );

	if ( sv_minPing->integer ) {
		Info_SetValueForKey( infostring, "minPing", va( "%i", sv_minPing->integer ) );
	}
	if ( sv_maxPing->integer ) {
		Info_SetValueForKey( infostring, "maxPing", va( "%i", sv_maxPing->integer ) );
	}

	var = Cvar_FindVar( "fs_game" );
	if ( var && var->string[0] ) {
		Info_SetValueForKey( infostring, "game", var->string );
	}

	Info_SetValueForKey( infostring, "sv_allowAnonymous",
						 va( "%i", sv_allowAnonymous->integer ) );

	var = Cvar_FindVar( "g_password" );
	if ( var && var->string && var->string[0] ) {
		Info_SetValueForKey( infostring, "pswrd", "1" );
	} else {
		Info_SetValueForKey( infostring, "pswrd", "0" );
	}

	NET_OutOfBandPrint( NS_SERVER, from, "infoResponse\n%s", infostring );
}

/* ---- SVC_RemoteCommand  0x0045A5D0 ---- */
void SVC_RemoteCommand( netadr_t from, msg_t *msg ) {
	qboolean valid;
	int i;
	int len;
	int time;
	char remaining[1024];
	/* the size of the redirect buffer really is MAX_MSGLEN - 16, not MAX_MSGLEN: 0x0045A711 stores 0x3FF0. */
	char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

	(void)msg;

	time = Com_Milliseconds();
	if ( sv_rconLastTime && time - sv_rconLastTime < SV_RCON_THROTTLE_MS ) {
		return;
	}
	sv_rconLastTime = time;

	if ( !strlen( sv_rconPassword->string )
		 || strcmp( Cmd_Argv( 1 ), sv_rconPassword->string ) ) {
		valid = qfalse;
		Com_Printf( "Bad rcon from %s:\n%s\n", NET_AdrToString( from ),
					Cmd_Argv( 2 ) );
	} else {
		valid = qtrue;
		Com_Printf( "Rcon from %s:\n%s\n", NET_AdrToString( from ),
					Cmd_Argv( 2 ) );
	}

	svs.redirectAddress = from;
	Com_BeginRedirect( sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect );

	if ( !strlen( sv_rconPassword->string ) ) {
		Com_Printf( "No rconpassword set on the server.\n" );
	} else if ( !valid ) {
		Com_Printf( "Bad rconpassword.\n" );
	} else {
		len = 0;
		for ( i = 2 ; i < Cmd_Argc() ; i++ ) {
			len = Com_AddToString( Cmd_Argv( i ), remaining, len,
								   sizeof( remaining ), 1 );
			if ( len >= (int)sizeof( remaining ) ) {
				break;
			}
			remaining[len++] = ' ';
		}
		if ( len < (int)sizeof( remaining ) ) {
			remaining[len] = 0;
			Cmd_ExecuteString( remaining );
		}
	}

	Com_EndRedirect();
}

unsigned short  Scr_AllocArray( void );         /* script/scr_variable.cpp 0x00471580 */
void            RemoveRefToObject( unsigned short id );  /* 0x004716B0 */

/* ---- SV_AllocClientScriptPers  no-address ---- */
unsigned short SV_AllocClientScriptPers( void ) {
	return Scr_AllocArray();
}

/* ---- SV_FreeClientScriptId  0x0045AE20 ---- */
void SV_FreeClientScriptId( client_t *client ) {
	if ( !client->scriptId ) {
		return;
	}
	RemoveRefToObject( client->scriptId );
	client->scriptId = 0;
}

netadr_t sv_masterAdr[MAX_MASTER_SERVERS];
static netadr_t sv_masterGcsAdr[MAX_MASTER_SERVERS];

static int sv_csDumped;

/* ---- SV_DumpConfigstrings  no-address ---- */
static void SV_DumpConfigstrings( void ) {
	int i;
	int nonEmpty = 0;
	const char  *s;

	Com_Printf( "----- configstrings (non-empty) -----\n" );
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		s = sv.configstrings[i];
		if ( !s || !s[0] ) {
			continue;
		}
		nonEmpty++;
		Com_Printf( "  cs[%4d] %.140s\n", i, s );
	}
	Com_Printf( "----- %d non-empty; script KV block %d..%d -----\n",
				nonEmpty, CS_SCRIPT_KV_BASE,
				CS_SCRIPT_KV_BASE + 2 * CS_SCRIPT_KV_COUNT - 1 );
	for ( i = 0 ; i < CS_SCRIPT_KV_COUNT ; i++ ) {
		const char  *k = sv.configstrings[CS_SCRIPT_KV_BASE + i];
		const char  *v = sv.configstrings[CS_SCRIPT_KV_BASE + CS_SCRIPT_KV_COUNT + i];
		if ( !k || !k[0] ) {
			break;
		}
		Com_Printf( "  kv[%2d] %-28s = %.100s\n", i, k, v ? v : "" );
	}
	Com_Printf( "-------------------------------------\n" );
}

/* ---- SV_DebugMenuShot  no-address ---- */
static void SV_DebugMenuShot( void ) {
	static cvar_t   *sv_debugMenu;
	int i;
	char index[64];

	if ( !sv_debugMenu ) {
		sv_debugMenu = Cvar_Get( "sv_debugMenu", "", 0 );
	}
	if ( !sv_debugMenu->string[0] ) {
		return;
	}

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state == CS_ACTIVE ) {
			break;
		}
	}
	if ( i >= sv_maxclients->integer ) {
		return;
	}

	Q_strncpyz( index, sv_debugMenu->string, sizeof( index ) );
	Cvar_Set2( "sv_debugMenu", "", qtrue );

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state != CS_ACTIVE ) {
			continue;
		}
		Com_Printf( "SVCMD* forcing menu to cl%d: [m %s]\n", i, index );
		SV_SendServerCommand( &svs.clients[i], qtrue, "m %s", index );
	}
}

/* ---- SV_ExpandNewlines  0x004596C0 ---- */
char *SV_ExpandNewlines( const char *string ) {
	static char buf[1024];
	unsigned int l;

	l = 0;
	while ( *string ) {
		if ( l >= 1021 ) {
			break;
		}
		if ( *string == '\n' ) {
			buf[l++] = '\\';
			buf[l] = 'n';
		} else if ( *string == 20 || *string == 21 ) {
			string++;
			continue;
		} else {
			buf[l] = *string;
		}
		l++;
		string++;
	}
	buf[l] = 0;

	return buf;
}

/* ---- SV_IsFirstTokenEqual  0x00459710 ---- */
qboolean SV_IsFirstTokenEqual( const char *a, const char *b ) {
	while ( *b && *a && *b != ' ' && *a != ' ' ) {
		if ( *b != *a ) {
			return qfalse;
		}
		b++;
		a++;
	}

	if ( *b && *b != ' ' ) {
		return qfalse;
	}

	return ( !*a || *a == ' ' ) ? qtrue : qfalse;
}

/* ---- SV_CanReplaceServerCommand  0x00459760 ---- */
int SV_CanReplaceServerCommand( client_t *client, const char *cmd ) {
	int i;
	int last;
	char c;
	reliableCommands_t  *rc;

	last = client->reliableSequence;

	for ( i = client->reliableSent + 1 ; i <= last ; i++ ) {
		rc = &client->reliableCommands[i & ( MAX_RELIABLE_COMMANDS - 1 )];
		if ( !rc->cmdType ) {
			continue;
		}

		c = cmd[0];
		if ( c != rc->command[0] || ( c >= 'x' && c <= 'z' ) ) {
			continue;
		}

		if ( !strcmp( cmd + 1, rc->command + 1 ) ) {
			return i;
		}

		switch ( c ) {
		case 'a':
		case 'b':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 't':
			return i;
		case 'd':
		case 'v':
			if ( SV_IsFirstTokenEqual( rc->command + 2, cmd + 2 ) ) {
				return i;
			}
			break;
		default:
			break;
		}

		last = client->reliableSequence;
	}

	return -1;
}

/* ---- SV_CullIgnorableServerCommands  0x00459870 ---- */
void SV_CullIgnorableServerCommands( client_t *client ) {
	int i;
	int keep;

	keep = client->reliableSent + 1;

	for ( i = keep ; i <= client->reliableSequence ; i++ ) {
		int from = i & ( MAX_RELIABLE_COMMANDS - 1 );
		int to = keep & ( MAX_RELIABLE_COMMANDS - 1 );

		if ( !client->reliableCommands[from].cmdType ) {
			continue;
		}
		if ( to != from ) {
			Com_Memcpy( &client->reliableCommands[to],
						&client->reliableCommands[from],
						sizeof( reliableCommands_t ) );
		}
		keep++;
	}

	client->reliableSequence = keep - 1;
}

/* ---- SV_AddServerCommand  0x004598E0 ---- */
void SV_AddServerCommand( client_t *client, int isReliable, const char *cmd ) {
	int i, j;
	int replace;
	reliableCommands_t  *rc;

	if ( client->bIsTestClient ) {
		return;
	}

	if ( sv_debugSpawn && sv_debugSpawn->integer ) {
		Com_Printf( "SVCMD+ cl%d st%d rel%d seq%d ack%d sent%d [%.90s]\n",
					(int)( client - svs.clients ), client->state, isReliable,
					client->reliableSequence, client->reliableAcknowledge,
					client->reliableSent, cmd ? cmd : "(null)" );
	}

	if ( !( client->reliableSequence - client->reliableAcknowledge < 32
			&& client->state == CS_ACTIVE ) ) {
		SV_CullIgnorableServerCommands( client );
		if ( !isReliable ) {
			if ( sv_debugSpawn && sv_debugSpawn->integer ) {
				Com_Printf( "SVCMD! dropped ignorable cl%d [%.90s]\n",
							(int)( client - svs.clients ), cmd ? cmd : "(null)" );
			}
			return;
		}
	}

	replace = SV_CanReplaceServerCommand( client, cmd );
	if ( sv_debugSpawn && sv_debugSpawn->integer && replace >= 0 ) {
		Com_Printf( "SVCMD~ cl%d replaces seq%d [%.60s] with [%.60s]\n",
					(int)( client - svs.clients ), replace,
					client->reliableCommands[replace & ( MAX_RELIABLE_COMMANDS - 1 )].command,
					cmd ? cmd : "(null)" );
	}
	if ( replace < 0 ) {
		client->reliableSequence++;
	} else {
		i = replace + 1;
		while ( i <= client->reliableSequence ) {
			Com_Memcpy( &client->reliableCommands[replace & ( MAX_RELIABLE_COMMANDS - 1 )],
						&client->reliableCommands[i & ( MAX_RELIABLE_COMMANDS - 1 )],
						sizeof( reliableCommands_t ) );
			replace++;
			i++;
		}
	}

	if ( client->reliableSequence - client->reliableAcknowledge == 65 ) {
		Com_Printf( "===== pending server commands =====\n" );
		for ( j = client->reliableAcknowledge + 1 ; j <= client->reliableSequence ; j++ ) {
			rc = &client->reliableCommands[j & ( MAX_RELIABLE_COMMANDS - 1 )];
			Com_Printf( "cmd %5d: %8d: %s\n", j, rc->cmdTime, rc->command );
		}
		Com_Printf( "cmd %5d: %8d: %s\n", j, svs.time, cmd );

		NET_OutOfBandPrint( NS_SERVER, client->netchan.remoteAddress, "disconnect" );
		if ( client->state != CS_ZOMBIE && !client->dropReason ) {
			client->dropReason = "EXE_SERVERCOMMANDOVERFLOW";
		}
		isReliable = 1;
		cmd = "w \"EXE_SERVERCOMMANDOVERFLOW\"";
	}

	rc = &client->reliableCommands[client->reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 )];
	MSG_WriteReliableCommandToBuffer( cmd, sizeof( rc->command ), rc->command );
	rc->cmdTime = svs.time;
	rc->cmdType = isReliable;
}

/* ---- SV_SendServerCommand  0x00459A90 ---- */
void QDECL SV_SendServerCommand( client_t *client, int isReliable, const char *fmt, ... ) {
	va_list argptr;
	client_t    *cl;
	int i;
	char message[MAX_MSGLEN];

	va_start( argptr, fmt );
	vsprintf( message, fmt, argptr );
	va_end( argptr );

	if ( client ) {
		SV_AddServerCommand( client, isReliable, message );
		return;
	}

	if ( com_dedicated->integer && !strncmp( message, "print", 5 ) ) {
		Com_Printf( "broadcast: %s\n", SV_ExpandNewlines( message ) );
	}

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state < CS_PRIMED ) {
			continue;
		}
		SV_AddServerCommand( cl, isReliable, message );
	}
}

/* ---- SV_ResolveMaster  no-address ---- */
static qboolean SV_ResolveMaster( cvar_t *master, netadr_t *adr ) {
	if ( !master->string[0] ) {
		return qfalse;
	}

	if ( !master->modified ) {
		return qtrue;
	}
	master->modified = qfalse;

	Com_Printf( "Resolving %s\n", master->string );
	if ( !NET_StringToAdr( master->string, adr ) ) {
		Com_Printf( "Couldn't resolve address: %s\n", master->string );
		Cvar_Set2( master->name, "", qtrue );
		master->modified = qfalse;
		return qfalse;
	}

	if ( !strstr( ":", master->string ) ) {
		adr->port = BigShort( PORT_MASTER );
	}

	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", master->string,
				adr->ip[0], adr->ip[1], adr->ip[2], adr->ip[3],
				BigShort( adr->port ) );
	return qtrue;
}

/* ---- SV_MasterHeartbeat  0x00459BA0 ---- */
void SV_MasterHeartbeat( const char *hbname ) {
	int i;

	if ( !com_dedicated || com_dedicated->integer != 2 ) {
		return;
	}
	if ( svs.time < svs.nextHeartbeatTime ) {
		return;
	}
	svs.nextHeartbeatTime = svs.time + 180000;

	for ( i = 0 ; i < MAX_MASTER_SERVERS ; i++ ) {
		if ( !sv_master[i] ) {
			continue;
		}
		if ( !SV_ResolveMaster( sv_master[i], &sv_masterAdr[i] ) ) {
			continue;
		}

		Com_Printf( "Sending heartbeat to %s\n", sv_master[i]->string );
		NET_OutOfBandPrint( NS_SERVER, sv_masterAdr[i], "heartbeat %s\n", hbname );
	}
}

/* ---- SV_MasterGameCompleteStatus  0x00459D20 ---- */
void SV_MasterGameCompleteStatus( void ) {
	int i;

	if ( !com_dedicated || com_dedicated->integer != 2 ) {
		return;
	}

	for ( i = 0 ; i < MAX_MASTER_SERVERS ; i++ ) {
		if ( !sv_master[i] ) {
			continue;
		}
		if ( !SV_ResolveMaster( sv_master[i], &sv_masterGcsAdr[i] ) ) {
			continue;
		}

		Com_Printf( "Sending gameCompleteStatus to %s\n", sv_master[i]->string );
		SVC_GameCompleteStatus( sv_masterGcsAdr[i] );
	}
}

/* ---- SV_MasterShutdown  0x00459E90 ---- */
void SV_MasterShutdown( void ) {
	svs.nextHeartbeatTime = -9999;
	SV_MasterHeartbeat( "flatline" );
}

/* ---- SV_GetClientScore  0x00459EA0 ---- */
int SV_GetClientScore( client_t *client ) {
	if ( !vm ) {
		return 0;
	}
	return VM_Call( vm, GAME_GET_CLIENT_SCORE, (int)( client - svs.clients ) );
}

/* ---- SV_FlushRedirect  0x0045A5A0 ---- */
void SV_FlushRedirect( char *outputbuf ) {
	NET_OutOfBandPrint( NS_SERVER, svs.redirectAddress, "print\n%s", outputbuf );
}

/* ---- SV_ConnectionlessPacket  0x0045A820 ---- */
void SV_ConnectionlessPacket( netadr_t from, msg_t *msg ) {
	char    *s;
	char    *c;

	MSG_BeginReading( msg );
	MSG_ReadLong( msg );

	if ( net_profile->integer ) {
		NetProf_PrepProfiling( &svs.pOOBProf );
		/* An incoming packet is added to the OOB profile's SEND stream. Retail bug; preserved. */
		NetProf_AddPacket( (netsrc_t)msg->cursize, 0, NULL, from );
	}

	if ( !Q_strncmp( "connect", (const char *)msg->data + 4, 7 ) ) {
		Huff_Decompress( msg, 12 );
	}

	s = MSG_ReadStringLine( msg );
	Cmd_TokenizeString2( s, 0 );
	c = Cmd_Argv( 0 );

	Com_DPrintf( "SV packet %s : %s\n", NET_AdrToString( from ), c );

	if ( !c ) {
		return;
	}

	if ( !Q_stricmpn( c, "getstatus", 99999 ) ) {
		SVC_Status( from );
	} else if ( !Q_stricmpn( c, "getinfo", 99999 ) ) {
		SVC_Info( from );
	} else if ( !Q_stricmpn( c, "getchallenge", 99999 ) ) {
		SV_GetChallenge( from );
	} else if ( !Q_stricmpn( c, "connect", 99999 ) ) {
		SV_DirectConnect( from );
	} else if ( !Q_stricmpn( c, "ipAuthorize", 99999 ) ) {
		SV_AuthorizeIpPacket( from );
	} else if ( !Q_stricmp( c, "rcon" ) ) {
		SVC_RemoteCommand( from, msg );
	} else if ( Q_stricmp( c, "disconnect" ) ) {
		Com_DPrintf( "bad connectionless packet from %s:\n%s\n",
					 NET_AdrToString( from ), s );
	}
}

/* ---- SV_PacketEvent  0x0045AA70 ---- */
void SV_PacketEvent( netadr_t from, msg_t *msg ) {
	int i;
	int qport;
	client_t    *cl;

	if ( msg->cursize >= 4 && *(int *)msg->data == -1 ) {
		SV_ConnectionlessPacket( from, msg );
		return;
	}

	if ( !++com_skelTimeStamp ) {
		com_skelTimeStamp = 1;
	}
	com_skelInvalidated = 1;

	msg->readcount = 0;
	msg->bit = 0;
	if ( msg->cursize >= 4 ) {
		msg->readcount = 4;
	}
	qport = MSG_ReadShort( msg ) & 0xffff;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];

		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( !NET_CompareBaseAdr( from, cl->netchan.remoteAddress ) ) {
			continue;
		}
		if ( cl->netchan.qport != qport ) {
			continue;
		}

		if ( cl->netchan.remoteAddress.port != from.port ) {
			Com_Printf( "SV_ReadPackets: fixing up a translated port\n" );
			cl->netchan.remoteAddress.port = from.port;
		}

		if ( Netchan_Process( &cl->netchan, msg ) ) {
			cl->serverId = MSG_ReadByte( msg );
			cl->messageAcknowledge = MSG_ReadLong( msg );
			if ( cl->messageAcknowledge >= 0 ) {
				cl->reliableAcknowledge = MSG_ReadLong( msg );

				if ( cl->reliableSequence - cl->reliableAcknowledge
					 < MAX_RELIABLE_COMMANDS ) {
					SV_Netchan_Decode( cl, msg->data + msg->readcount,
									   msg->cursize - msg->readcount );
					if ( cl->state != CS_ZOMBIE ) {
						cl->lastPacketTime = svs.time;
						SV_ExecuteClientMessage( msg, cl );
					}
				} else {
					cl->reliableAcknowledge = cl->reliableSequence;
				}
			}
		}

		goto done;
	}

	NET_OutOfBandPrint( NS_SERVER, from, "disconnect" );

done:
	com_skelInvalidated = 0;
	if ( s_hunkData ) {
		SV_HUNK_LOW_TEMP = SV_HUNK_LOW_PERM;
	}
}

/* ---- SV_CalcPings  0x0045ACB0 ---- */
void SV_CalcPings( void ) {
	int i, j;
	int total, count;
	client_t    *cl;
	clientSnapshot_t    *frame;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];

		if ( cl->state != CS_ACTIVE || !cl->gentity ) {
			cl->ping = 999;
			continue;
		}

		total = 0;
		count = 0;
		for ( j = 0 ; j < PACKET_BACKUP ; j++ ) {
			frame = &cl->frames[j];
			if ( (int)frame->messageAcked <= 0 ) {
				continue;
			}
			count++;
			total += frame->messageAcked - frame->messageSent;
		}

		if ( !count ) {
			cl->ping = 999;
		} else {
			cl->ping = total / count;
			if ( cl->ping > 999 ) {
				cl->ping = 999;
			}
		}

		( (playerState_t *)( (byte *)sv.gameClients + i * sv.gameClientSize ) )->ping
			= cl->ping;
	}
}

/* ---- SV_CheckTimeouts  0x0045AE80 ---- */
void SV_CheckTimeouts( void ) {
	int i;
	int droppoint;
	int zombiepoint;
	client_t    *cl;

	droppoint = svs.time - 1000 * sv_timeout->integer;
	zombiepoint = svs.time - 1000 * sv_zombietime->integer;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];

		if ( cl->lastPacketTime > svs.time ) {
			cl->lastPacketTime = svs.time;
		}

		if ( cl->bIsTestClient ) {
			continue;
		}

		if ( cl->state == CS_ZOMBIE && cl->lastPacketTime < zombiepoint ) {
			Com_DPrintf( "Going from CS_ZOMBIE to CS_FREE for %s\n", cl->name );
			cl->state = CS_FREE;
		} else if ( cl->state < CS_CONNECTED || cl->lastPacketTime >= droppoint ) {
			cl->timeoutCount = 0;
		} else {
			cl->timeoutCount++;
			if ( cl->timeoutCount > 5 ) {
				SV_DropClient( cl, "EXE_TIMEDOUT" );
				cl->state = CS_FREE;
			}
		}
	}
}

/* ---- SV_CheckPaused  0x0045AF70 ---- */
qboolean SV_CheckPaused( void ) {
	int i;
	int count;

	if ( !cl_paused->integer ) {
		return qfalse;
	}

	count = 0;
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			count++;
		}
	}

	if ( count > 1 ) {
		sv_paused->integer = 0;
		return qfalse;
	}

	sv_paused->integer = 1;
	return qtrue;
}

extern void SV_TestClientFrame( void );   /* TEMPORARY scaffolding, sv_client_mp.c */

/* ---- SV_RunFrame  0x0045AFD0 ---- */
void SV_RunFrame( void ) {
	VM_Call( vm, GAME_UPDATE_CVARS );

	if ( !++com_skelTimeStamp ) {
		com_skelTimeStamp = 1;
	}
	com_skelInvalidated = 1;

	SV_TestClientFrame();   /* TEMPORARY scaffolding, see sv_client_mp.c */
	VM_Call( vm, GAME_RUN_FRAME, svs.time );

	com_skelInvalidated = 0;
	if ( s_hunkData ) {
		SV_HUNK_LOW_TEMP = SV_HUNK_LOW_PERM;
	}
}

/* ---- SV_BotUserMove  0x0045B030 ---- */
void SV_BotUserMove( client_t *client ) {
	int clientNum;
	usercmd_t cmd;

	if ( !client->gentity ) {
		return;
	}

	memset( &cmd, 0, sizeof( cmd ) );
	clientNum = (int)( client - svs.clients );

	cmd.weapon = (byte)( (playerState_t *)( (byte *)sv.gameClients
											+ clientNum * sv.gameClientSize ) )->weapon;

	if ( !VM_Call( vm, GAME_GET_ARCHIVE_TIME, clientNum ) ) {
		if ( rand() * ( 1.0 / 32768.0 ) < 0.5 ) {
			cmd.buttons |= 1;
		}
		if ( rand() * ( 1.0 / 32768.0 ) < 0.5 ) {
			cmd.buttons |= 0x40;
		}
		if ( rand() * ( 1.0 / 32768.0 ) < 0.33000001 ) {
			cmd.forwardmove = 127;
		} else if ( rand() * ( 1.0 / 32768.0 ) < 0.5 ) {
			cmd.forwardmove = -127;
		}
	}

	client->deltaMessage = client->netchan.outgoingSequence - 1;
	SV_ClientThink( client, &cmd );
}

/* ---- SV_UpdateBots  0x0045B150 ---- */
void SV_UpdateBots( void ) {
	int i;
	client_t    *cl;

	if ( !++com_skelTimeStamp ) {
		com_skelTimeStamp = 1;
	}
	com_skelInvalidated = 1;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];
		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( cl->netchan.remoteAddress.type == NA_BOT ) {
			SV_BotUserMove( cl );
		}
	}

	com_skelInvalidated = 0;
	if ( s_hunkData ) {
		SV_HUNK_LOW_TEMP = SV_HUNK_LOW_PERM;
	}
}

/* ---- SV_RestartAfterCounterWrap  no-address ---- */
static void SV_RestartAfterCounterWrap( const char *message ) {
	char mapname[MAX_QPATH];

	strncpy( mapname, sv_mapname->string, MAX_QPATH - 1 );
	mapname[MAX_QPATH - 1] = 0;
	Com_Shutdown( message );
	Cbuf_AddText( va( "map %s\n", mapname ) );
}

/* ---- SV_Frame  0x0045B1D0 ---- */
void SV_Frame( int msec ) {
	int frameMsec;
	int startTime;
	int i;

	if ( sv_killserver->integer ) {
		Com_Shutdown( "EXE_SERVERKILLED" );
		Cvar_Set2( "sv_killserver", "0", qtrue );
		return;
	}

	if ( !com_sv_running->integer ) {
		return;
	}
	if ( SV_CheckPaused() ) {
		return;
	}

	if ( sv_fps->integer < 1 ) {
		Cvar_Set2( "sv_fps", "10", qtrue );
	}
	frameMsec = 1000 / sv_fps->integer;

	sv.timeResidual += msec;
	if ( sv.timeResidual < frameMsec ) {
		return;
	}

	if ( svs.time > SV_TIME_WRAP_LIMIT ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTTIMEWRAP" );
		return;
	}
	if ( svs.nextSnapshotEntities >= SV_SNAPSHOT_COUNTER_LIMIT - svs.numSnapshotEntities ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTMISC\x15numSnapshotEntities" );
		return;
	}
	if ( svs.nextCachedSnapshotEntities >= SV_CACHED_SNAP_ENTITIES_LIMIT ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTMISC\x15nextCachedSnapshotEntities" );
		return;
	}
	if ( svs.nextCachedSnapshotClients >= SV_CACHED_SNAP_CLIENTS_LIMIT ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTMISC\x15nextCachedSnapshotClients" );
		return;
	}
	if ( svs.nextArchivedSnapshotFrames >= SV_ARCHIVED_SNAP_FRAMES_LIMIT ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTMISC\x15nextArchivedSnapshotFrames" );
		return;
	}
	if ( svs.nextArchivedSnapshotBuffer >= SV_ARCHIVED_SNAP_BUFFER_LIMIT ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTMISC\x15nextArchivedSnapshotBuffer" );
		return;
	}
	if ( svs.nextCachedSnapshotFrames >= SV_CACHED_SNAP_FRAMES_LIMIT ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTMISC\x15nextCachedSnapshotFrames" );
		return;
	}
	if ( svs.nextSnapshotClients >= SV_SNAPSHOT_COUNTER_LIMIT - svs.numSnapshotClients ) {
		SV_RestartAfterCounterWrap( "EXE_SERVERRESTARTMISC\x15numSnapshotClients" );
		return;
	}

	xanim_activePoolSlot = 1;

	if ( cvar_modifiedFlags & CVAR_SERVERINFO ) {
		SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
		cvar_modifiedFlags &= ~CVAR_SERVERINFO;
	}
	if ( cvar_modifiedFlags & CVAR_SYSTEMINFO ) {
		SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString_Big( CVAR_SYSTEMINFO ) );
		cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
	}
	if ( cvar_modifiedFlags & CVAR_WOLFINFO ) {
		SV_SetConfig( CS_SCRIPT_KV_BASE, CS_SCRIPT_KV_COUNT, CVAR_WOLFINFO );
		cvar_modifiedFlags &= ~CVAR_WOLFINFO;
	}

	if ( sv_debugSpawn && sv_debugSpawn->integer && !sv_csDumped ) {
		for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
			if ( svs.clients[i].state == CS_ACTIVE ) {
				break;
			}
		}
		if ( i < sv_maxclients->integer ) {
			sv_csDumped = 1;
			SV_DumpConfigstrings();
		}
	}

	SV_DebugMenuShot();

	SV_UpdateBots();

	startTime = com_speeds->integer ? Sys_Milliseconds() : 0;

	SV_CalcPings();

	for ( ;; ) {
		sv.timeResidual -= frameMsec;
		svs.time += frameMsec;

		CL_FlushDebugData( 1 );
		SV_RunFrame();
		/* retail stores 0 to scrVmGlob.loading (0x00A7A5E0) inline; script/scr_vm_state.c owns that word and exposes this setter. */
		Scr_SetLoading( qfalse );

		if ( sv.timeResidual < frameMsec ) {
			break;
		}
		SV_ArchiveSnapshot();
	}

	if ( com_speeds->integer ) {
		time_frontend = Sys_Milliseconds() - startTime;
	}

	SV_CheckTimeouts();
	SV_SendClientMessages();
	SV_ArchiveSnapshot();

	SV_MasterHeartbeat( "COD-1" );
}
