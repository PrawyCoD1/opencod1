/*
 * @fidelity: likely
 */

#include "server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void MSG_WriteByte( msg_t *msg, int c );
void MSG_WriteShort( msg_t *msg, int c );
void MSG_WriteLong( msg_t *msg, int c );
void MSG_WriteData( msg_t *msg, int length, const void *data );
void MSG_WriteString( const char *s, msg_t *sb );
void MSG_WriteBigString( const char *s, msg_t *sb );
void MSG_WriteDeltaEntity( msg_t *msg, const byte *from, const byte *to, qboolean force );
char *MSG_ReadString( msg_t *msg );
void MSG_ReadDeltaUsercmdKey( msg_t *msg, int key, const usercmd_t *from, usercmd_t *to );
void MSG_SetDefaultUserCmd( const byte *ps, usercmd_t *ucmd );

void SV_UpdateServerCommandsToClient( msg_t *msg, client_t *client );
void SV_SendMessageToClient( msg_t *msg, client_t *client );

void Netchan_TransmitNextFragment( netchan_t *chan );
qboolean NET_CompareAdr( netadr_t a, netadr_t b );

void SV_CloseDownload( client_t *cl );
void SV_UserinfoChanged( client_t *cl );

int Com_HashKey( const char *string, int maxlen );
qboolean FS_idPak( const char *pak, const char *base );
qboolean FS_FileIsInPAK( const char *filename, int *pChecksum );
const char *FS_LoadedPakPureChecksums( void );
char *FS_ShiftStr( const char *string, int shift );

/* 0x01651554. cod1_globals.c types it `int`; four bytes either way. */
extern cvar_t   *net_lanauthorize;
/* 0x014073E0, the rolling bot number SV_AddTestClient names clients with. */
extern int sv_testClientCounter;

unsigned short SV_AllocClientScriptPers( void );

#define MAX_PACKET_USERCMDS     32          /* 0x00455307: cmp against 32 */
#define SV_AUTHORIZE_TIMEOUT    5000        /* 0x00452F9F */
#define SV_DOWNLOAD_BLOCK_SIZE  0x800

#define AUTHORIZE_SERVER_NAME   "codauthorize.activision.com"

#define svc_bad                 0
#define svc_nop                 1
#define svc_gamestate           2
#define svc_configstring        3
#define svc_baseline            4
#define svc_serverCommand       5
#define svc_download            6
#define svc_snapshot            7
#define svc_EOF                 8

/* ---- SV_GetChallenge  0x00452DA0 ---- */
void SV_GetChallenge( netadr_t from ) {
	int i;
	int oldest;
	int oldestTime;
	challenge_t *challenge;
	cvar_t      *var;
	char gameName[MAX_INFO_STRING];

	oldest = 0;
	oldestTime = 0x7fffffff;

	challenge = &svs.challenges[0];
	for ( i = 0 ; i < MAX_CHALLENGES ; i++, challenge++ ) {
		if ( !challenge->connected && NET_CompareAdr( from, challenge->adr ) ) {
			break;
		}
		if ( challenge->time < oldestTime ) {
			oldestTime = challenge->time;
			oldest = i;
		}
	}

	if ( i == MAX_CHALLENGES ) {
		challenge = &svs.challenges[oldest];
		challenge->challenge = ( ( rand() << 16 ) ^ rand() ) ^ svs.time;
		challenge->adr = from;
		challenge->firstTime = svs.time;
		challenge->firstPing = 0;
		challenge->time = svs.time;
		challenge->connected = qfalse;
		i = oldest;
	}

	if ( !net_lanauthorize->integer && Sys_IsLANAddress( from ) ) {
		challenge->pingTime = svs.time;
		if ( sv_onlyVisibleClients->integer ) {
			NET_OutOfBandPrint( NS_SERVER, from, "challengeResponse %i %i",
								challenge->challenge,
								sv_onlyVisibleClients->integer );
		} else {
			NET_OutOfBandPrint( NS_SERVER, from, "challengeResponse %i",
								challenge->challenge );
		}
		return;
	}

	if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress ) == qfalse ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}
		svs.authorizeAddress.port = BigShort( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
					svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
					svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
					BigShort( svs.authorizeAddress.port ) );
	}

	if ( svs.time - challenge->firstTime > SV_AUTHORIZE_TIMEOUT ) {
		Com_DPrintf( "authorize server timed out\n" );
		challenge->pingTime = svs.time;
		if ( sv_onlyVisibleClients->integer ) {
			NET_OutOfBandPrint( NS_SERVER, challenge->adr,
								"challengeResponse %i %i",
								challenge->challenge,
								sv_onlyVisibleClients->integer );
		} else {
			NET_OutOfBandPrint( NS_SERVER, challenge->adr,
								"challengeResponse %i",
								challenge->challenge );
		}
		return;
	}

	if ( svs.authorizeAddress.type == NA_BAD ) {
		return;
	}

	gameName[0] = 0;
	var = Cvar_Get( "fs_game", "", CVAR_SYSTEMINFO | CVAR_INIT );
	if ( var && var->string[0] ) {
		strcpy( gameName, var->string );
	}

	Com_DPrintf( "sending getIpAuthorize for %s\n", NET_AdrToString( from ) );

	NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,
						"getIpAuthorize %i %i.%i.%i.%i %s %i",
						svs.challenges[i].challenge,
						from.ip[0], from.ip[1], from.ip[2], from.ip[3],
						gameName,
						Cvar_Get( "sv_allowAnonymous", "0", CVAR_SERVERINFO )->integer );
}

/* ---- SV_AuthorizeIpPacket  0x00453100 ---- */
void SV_AuthorizeIpPacket( netadr_t from ) {
	int challengeNum;
	int i;
	const char  *response;
	const char  *reason;
	const char  *reply;
	cvar_t      *var;
	char errorBuf[MAX_STRING_CHARS];

	if ( NET_CompareBaseAdr( from, svs.authorizeAddress ) == qfalse ) {
		Com_Printf( "SV_AuthorizeIpPacket: not from authorize server\n" );
		return;
	}

	challengeNum = atoi( Cmd_Argv( 1 ) );
	for ( i = 0 ; i < MAX_CHALLENGES ; i++ ) {
		if ( svs.challenges[i].challenge == challengeNum ) {
			break;
		}
	}
	if ( i == MAX_CHALLENGES ) {
		Com_Printf( "SV_AuthorizeIpPacket: challenge not found\n" );
		return;
	}

	svs.challenges[i].pingTime = svs.time;

	response = Cmd_Argv( 2 );
	reason = Cmd_Argv( 3 );

	if ( response && !Q_stricmpn( response, "demo", 99999 ) ) {
		var = Cvar_FindVar( "fs_restrict" );
		if ( var && var->value != 0.0f ) {
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr,
								"challengeResponse %i",
								svs.challenges[i].challenge );
			return;
		}
		reply = "error\nEXE_ERR_NOT_A_DEMO_SERVER";
	} else if ( response && !Q_stricmpn( response, "accept", 99999 ) ) {
		if ( sv_onlyVisibleClients->integer ) {
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr,
								"challengeResponse %i %i",
								svs.challenges[i].challenge,
								sv_onlyVisibleClients->integer );
		} else {
			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr,
								"challengeResponse %i",
								svs.challenges[i].challenge );
		}
		return;
	} else if ( response && !Q_stricmpn( response, "deny", 99999 ) ) {
		if ( !reason || !reason[0] ) {
			reply = "error\nEXE_ERR_CDKEY_IN_USE";
		} else if ( !Q_stricmpn( reason, "CLIENT_UNKNOWN_TO_AUTH", 99999 )
					|| !Q_stricmp( "BAD_CDKEY", reason ) ) {
			reply = "needcdkey";
		} else if ( !Q_stricmp( "INVALID_CDKEY", reason ) ) {
			reply = "error\nEXE_ERR_CDKEY_IN_USE";
		} else {
			/* 0x00453310 compares against "BANNED_CDKEY" and throws the answer away; every remaining reason lands here. */
			Q_stricmp( "BANNED_CDKEY", reason );
			reply = "error\nEXE_ERR_BAD_CDKEY";
		}
	} else if ( reason && reason[0] ) {
		sprintf( errorBuf, "error\n%s", reason );
		reply = errorBuf;
	} else {
		reply = "error\nEXE_ERR_BAD_CDKEY";
	}

	NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, reply );
	memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
}

/* ---- SV_ConnectMatchesClient  no-address ---- */
static qboolean SV_ConnectMatchesClient( netadr_t from, const client_t *cl, int qport ) {
	if ( !NET_CompareBaseAdr( from, cl->netchan.remoteAddress ) ) {
		return qfalse;
	}
	return ( cl->netchan.qport == qport
			 || from.port == cl->netchan.remoteAddress.port );
}

/* ---- SV_DirectConnect  0x004533A0 ---- */
void SV_DirectConnect( netadr_t from ) {
	char userinfo[MAX_INFO_STRING];
	int challengeNum;
	int qport;
	int protocol;
	int i;
	int clientNum;
	int ping;
	int count;
	int startIndex;
	client_t    *cl;
	client_t    *newcl;
	client_t emptyClient;
	const char  *denied;

	Com_DPrintf( "SVC_DirectConnect ()\n" );

	Q_strncpyz( userinfo, Cmd_Argv( 1 ), sizeof( userinfo ) );

	protocol = atoi( Info_ValueForKey( userinfo, "protocol" ) );
	if ( protocol != PROTOCOL_VERSION ) {
		if ( sv_debugSpawn && sv_debugSpawn->integer ) {
			Com_Printf( "CONNDBG: reject protocol %i (want %i) argc %i "
						"argv1 [%.120s]\n",
						protocol, PROTOCOL_VERSION, Cmd_Argc(), Cmd_Argv( 1 ) );
		}
		NET_OutOfBandPrint( NS_SERVER, from, "error\n%s",
							"EXE_SERVER_IS_DIFFERENT_VER" );
		Com_DPrintf( "    rejected connect from protocol version %i (should be %i)\n",
					 protocol, PROTOCOL_VERSION );
		return;
	}

	challengeNum = atoi( Info_ValueForKey( userinfo, "challenge" ) );
	qport = atoi( Info_ValueForKey( userinfo, "qport" ) );

	cl = svs.clients;
	for ( i = 0 ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( SV_ConnectMatchesClient( from, cl, qport ) ) {
			if ( svs.time - cl->lastConnectTime
				 < 1000 * sv_reconnectlimit->integer ) {
				Com_DPrintf( "%s:reconnect rejected : too soon\n",
							 NET_AdrToString( from ) );
				return;
			}
			break;
		}
	}

	if ( from.type == NA_LOOPBACK || from.type == NA_BOT ) {
		Info_SetValueForKey( userinfo, "ip", "localhost" );
	} else {
		for ( i = 0 ; i < MAX_CHALLENGES ; i++ ) {
			if ( NET_CompareAdr( from, svs.challenges[i].adr )
				 && challengeNum == svs.challenges[i].challenge ) {
				break;
			}
		}
		if ( i == MAX_CHALLENGES ) {
			NET_OutOfBandPrint( NS_SERVER, from, "error\nEXE_BAD_CHALLENGE" );
			return;
		}

		Info_SetValueForKey( userinfo, "ip", NET_AdrToString( from ) );

		ping = svs.challenges[i].firstPing;
		if ( !ping ) {
			ping = svs.time - svs.challenges[i].pingTime;
			svs.challenges[i].firstPing = ping;
		}
		Com_Printf( "Client %i connecting with %i challenge ping\n", i, ping );
		svs.challenges[i].connected = qtrue;

		if ( !Sys_IsLANAddress( from ) ) {
			if ( sv_minPing->value != 0.0f && (double)ping < sv_minPing->value ) {
				NET_OutOfBandPrint( NS_SERVER, from,
									"error\nEXE_ERR_HIGH_PING_ONLY" );
				Com_DPrintf( "Client %i rejected on a too low ping\n", i );
				return;
			}
			if ( sv_maxPing->value != 0.0f && (double)ping > sv_maxPing->value ) {
				NET_OutOfBandPrint( NS_SERVER, from,
									"error\nEXE_ERR_LOW_PING_ONLY" );
				Com_DPrintf( "Client %i rejected on a too high ping: %i\n",
							 i, ping );
				return;
			}
		}
	}

	memset( &emptyClient, 0, sizeof( emptyClient ) );

	newcl = NULL;
	cl = svs.clients;
	for ( i = 0 ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( SV_ConnectMatchesClient( from, cl, qport ) ) {
			Com_Printf( "%s:reconnect\n", NET_AdrToString( from ) );
			newcl = cl;
			if ( cl->state >= CS_CONNECTED ) {
				SV_FreeClientScriptId( cl );
			}
			break;
		}
	}

	if ( !newcl ) {
		if ( !strcmp( Info_ValueForKey( userinfo, "password" ),
					  sv_privatePassword->string ) ) {
			startIndex = 0;
		} else {
			startIndex = sv_privateClients->integer;
		}

		for ( i = startIndex ; i < sv_maxclients->integer ; i++ ) {
			if ( svs.clients[i].state == CS_FREE ) {
				newcl = &svs.clients[i];
				break;
			}
		}

		if ( !newcl ) {
			NET_OutOfBandPrint( NS_SERVER, from, "error\nEXE_SERVERISFULL" );
			Com_DPrintf( "Rejected a connection.\n" );
			return;
		}

		newcl->reliableAcknowledge = 0;
		newcl->reliableSequence = 0;
	}

	*newcl = emptyClient;
	clientNum = (int)( newcl - svs.clients );
	newcl->gentity = (gentity_t *)( (byte *)sv.gentities + clientNum * sv.gentitySize );
	newcl->scriptId = SV_AllocClientScriptPers();
	newcl->challenge = challengeNum;

	memset( &newcl->netchan, 0, sizeof( newcl->netchan ) );
	newcl->netchan.remoteAddress = from;
	newcl->netchan.sock = NS_SERVER;
	newcl->netchan.qport = qport;
	newcl->netchan.incomingSequence = 0;
	newcl->netchan.outgoingSequence = 1;
	NetProf_PrepProfiling( (void **)&newcl->netchan.pProf );

	Q_strncpyz( newcl->userinfo, userinfo, sizeof( newcl->userinfo ) );

	denied = (const char *)VM_Call( vm, GAME_CLIENT_CONNECT, clientNum,
									newcl->scriptId );
	if ( denied ) {
		NET_OutOfBandPrint( NS_SERVER, from, "error\n%s", denied );
		Com_DPrintf( "Game rejected a connection: %s.\n", denied );
		SV_FreeClientScriptId( newcl );
		return;
	}

	SV_UserinfoChanged( newcl );

	svs.challenges[clientNum].firstPing = 0;

	NET_OutOfBandPrint( NS_SERVER, from, "connectResponse" );
	Com_DPrintf( "Going from CS_FREE to CS_CONNECTED for %s\n", newcl->name );

	newcl->state = CS_CONNECTED;
	newcl->nextSnapshotTime = svs.time;
	newcl->lastPacketTime = svs.time;
	newcl->lastConnectTime = svs.time;
	newcl->gamestateMessageNum = -1;

	count = 0;
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			count++;
		}
	}
	if ( count == 1 || count == sv_maxclients->integer ) {
		svs.nextHeartbeatTime = -9999999;
	}
}

/* ---- SV_FreeClientScriptPers  0x00453CB0 ---- */
void SV_FreeClientScriptPers( void ) {
	int i;
	client_t    *cl;

	cl = svs.clients;
	for ( i = 0 ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( cl->state >= CS_CONNECTED ) {
			SV_FreeClientScriptId( cl );
			cl->scriptId = SV_AllocClientScriptPers();
		}
	}
}

/* ---- SV_DropClient  0x00453D80 ---- */
void SV_DropClient( client_t *drop, const char *reason ) {
	int i;
	challenge_t *challenge;

	if ( drop->state == CS_ZOMBIE ) {
		return;
	}

	drop->dropReason = NULL;
	Com_DPrintf( "Going to CS_ZOMBIE for %s\n", drop->name );

	if ( !drop->gentity ) {
		challenge = &svs.challenges[0];
		for ( i = 0 ; i < MAX_CHALLENGES ; i++, challenge++ ) {
			if ( NET_CompareAdr( drop->netchan.remoteAddress, challenge->adr ) ) {
				challenge->connected = qfalse;
				break;
			}
		}
	}

	drop->state = CS_ZOMBIE;

	SV_CloseDownload( drop );

	/* 0x00453E1A: NULL reason falls straight into the broadcast. */
	if ( !reason || Q_stricmpn( reason, "EXE_DISCONNECTED", 99999 ) ) {
		SV_SendServerCommand( NULL, 0, "e \"\x15%s^7 \x14%s\"",
							  drop->name, reason );
	}

	Com_Printf( "%i:%s %s\n", (int)( drop - svs.clients ), drop->name, reason );

	if ( drop->download ) {
		FS_FCloseFile( drop->download );
		drop->download = 0;
	}

	if ( sv.state == SS_GAME ) {
		VM_Call( vm, GAME_CLIENT_DISCONNECT, (int)( drop - svs.clients ) );
	}

	SV_SendServerCommand( drop, 1, va( "w \"%s\"", reason ) );

	SV_SetUserinfo( (int)( drop - svs.clients ), "" );

	SV_FreeClientScriptId( drop );

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			break;
		}
	}
	if ( i == sv_maxclients->integer ) {
		svs.nextHeartbeatTime = -9999999;
	}
}

/* ---- SV_DelayDropClient  0x00453FA0 ---- */
void SV_DelayDropClient( client_t *cl, const char *reason ) {
	if ( cl->state != CS_ZOMBIE && !cl->dropReason ) {
		cl->dropReason = reason;
	}
}

/* ---- SV_SendClientGameState  0x00453FC0 ---- */
void SV_SendClientGameState( client_t *client ) {
	int start;
	entityState_t nullstate;
	svEntity_t     *svEnt;
	msg_t msg;
	byte msgBuffer[MAX_MSGLEN];

	while ( client->state != CS_FREE && client->netchan.unsentFragments ) {
		Netchan_TransmitNextFragment( &client->netchan );
	}

	Com_DPrintf( "SV_SendClientGameState() for %s\n", client->name );
	Com_DPrintf( "Going from CS_CONNECTED to CS_PRIMED for %s\n", client->name );

	client->state = CS_PRIMED;
	client->pureAuthentic = 0;

	client->gamestateMessageNum = client->netchan.outgoingSequence;

	if ( msgInit == qfalse ) {
		MSG_initHuffman();
	}
	MSG_Init( &msg, msgBuffer, sizeof( msgBuffer ) );

	MSG_WriteLong( &msg, client->lastClientCommand );

	SV_UpdateServerCommandsToClient( &msg, client );

	MSG_WriteByte( &msg, svc_gamestate );
	MSG_WriteLong( &msg, client->reliableSequence );

	for ( start = 0 ; start < MAX_CONFIGSTRINGS ; start++ ) {
		if ( sv.configstrings[start][0] ) {
			MSG_WriteByte( &msg, svc_configstring );
			MSG_WriteShort( &msg, start );
			MSG_WriteBigString( sv.configstrings[start], &msg );
		}
	}

	memset( &nullstate, 0, sizeof( nullstate ) );
	for ( start = 0 ; start < MAX_GENTITIES ; start++ ) {
		svEnt = &sv.svEntities[start];
		if ( !svEnt->baseline.number ) {
			continue;
		}
		MSG_WriteByte( &msg, svc_baseline );
		MSG_WriteDeltaEntity( &msg, (const byte *)&nullstate,
							  (const byte *)&svEnt->baseline, qtrue );
	}

	MSG_WriteByte( &msg, svc_EOF );

	MSG_WriteLong( &msg, (int)( client - svs.clients ) );

	MSG_WriteLong( &msg, sv.checksumFeed );

	MSG_WriteByte( &msg, svc_EOF );

	Com_DPrintf( "Sending %i bytes in gamestate to client: %i\n",
				 msg.cursize, (int)( client - svs.clients ) );

	SV_SendMessageToClient( &msg, client );
}

/* ---- SV_ClientEnterWorld  0x004542B0 ---- */
void SV_ClientEnterWorld( client_t *cl, usercmd_t *cmd ) {
	int clientNum;
	gentity_t   *ent;

	Com_DPrintf( "Going from CS_PRIMED to CS_ACTIVE for %s\n", cl->name );
	cl->state = CS_ACTIVE;

	clientNum = (int)( cl - svs.clients );
	ent = (gentity_t *)( (byte *)sv.gentities + clientNum * sv.gentitySize );
	ent->s.number = clientNum;
	cl->gentity = ent;

	cl->deltaMessage = -1;
	cl->nextSnapshotTime = svs.time;
	cl->lastUsercmd = *cmd;

	VM_Call( vm, GAME_CLIENT_BEGIN, clientNum );

	if ( sv_debugSpawn && sv_debugSpawn->integer ) {
		playerState_t *ps = (playerState_t *)( (byte *)sv.gameClients
											   + clientNum * sv.gameClientSize );
		Com_Printf( "SPAWNDBG: EnterWorld cl %i  gEnts %p/%i  gClients %p/%i\n",
					clientNum, (void *)sv.gentities, sv.gentitySize,
					(void *)sv.gameClients, sv.gameClientSize );
		Com_Printf( "SPAWNDBG:   ps clientNum %i pm_type %i org %.1f %.1f %.1f "
					"vhC %.1f vhT %i ent->linked %i ent->s.number %i\n",
					ps->clientNum, (int)ps->pm_type,
					ps->origin[0], ps->origin[1], ps->origin[2],
					ps->viewHeightCurrent, ps->viewHeightTarget,
					ent->r.linked, ent->s.number );
	}
}

/* ---- SV_CloseDownload  0x00454360 ---- */
void SV_CloseDownload( client_t *cl ) {
	int i;

	if ( cl->download ) {
		FS_FCloseFile( cl->download );
	}
	cl->download = 0;
	cl->downloadName[0] = 0;

	/* 1.1 uses malloc/free for the block buffers (0x004546C5 / 0x0045439A) -- they are not on the zone (UO's are Z_Malloc/Z_Free). */
	for ( i = 0 ; i < MAX_DOWNLOAD_WINDOW ; i++ ) {
		if ( cl->downloadBlocks[i] ) {
			free( cl->downloadBlocks[i] );
			cl->downloadBlocks[i] = NULL;
		}
	}
}

/* ---- SV_StopDownload_f  0x004543C0 ---- */
void SV_StopDownload_f( client_t *cl ) {
	if ( cl->downloadName[0] ) {
		Com_DPrintf( "clientDownload: %d : file \"%s\" aborted\n",
					 (int)( cl - svs.clients ), cl->downloadName );
	}
	SV_CloseDownload( cl );
}

/* ---- SV_DoneDownload_f  0x00454410 ---- */
void SV_DoneDownload_f( client_t *cl ) {
	Com_DPrintf( "clientDownload: %s Done\n", cl->name );
	SV_SendClientGameState( cl );
}

/* ---- SV_RetransmitDownload_f  0x00454440 ---- */
void SV_RetransmitDownload_f( client_t *cl ) {
	int block;

	block = atoi( Cmd_Argv( 1 ) );
	if ( block == cl->downloadClientBlock ) {
		cl->downloadXmitBlock = cl->downloadClientBlock;
	}
}

/* ---- SV_NextDownload_f  0x00454480 ---- */
void SV_NextDownload_f( client_t *cl ) {
	int block;

	block = atoi( Cmd_Argv( 1 ) );
	if ( block != cl->downloadClientBlock ) {
		SV_DropClient( cl, "broken download" );
		return;
	}

	Com_DPrintf( "clientDownload: %d : client acknowledge of block %d\n",
				 (int)( cl - svs.clients ), block );

	if ( !cl->downloadBlockSize[cl->downloadClientBlock % MAX_DOWNLOAD_WINDOW] ) {
		Com_Printf( "clientDownload: %d : file \"%s\" completed\n",
					(int)( cl - svs.clients ), cl->downloadName );
		SV_CloseDownload( cl );
		return;
	}

	cl->downloadSendTime = svs.time;
	cl->downloadClientBlock++;
}

/* ---- SV_BeginDownload_f  0x00454560 ---- */
void SV_BeginDownload_f( client_t *cl ) {
	SV_CloseDownload( cl );

	Q_strncpyz( cl->downloadName, Cmd_Argv( 1 ), sizeof( cl->downloadName ) );
}

/* ---- SV_WriteDownloadToClient  0x004545A0 ---- */
void SV_WriteDownloadToClient( client_t *cl, msg_t *msg ) {
	int curindex;
	int rate;
	int blockspersnap;
	int idPack;
	char errorMessage[MAX_STRING_CHARS];

	if ( !cl->downloadName[0] ) {
		return;
	}

	if ( !cl->download ) {
		Com_Printf( "clientDownload: %d : begining \"%s\"\n",
					(int)( cl - svs.clients ), cl->downloadName );

		idPack = FS_idPak( cl->downloadName, "main" );

		if ( !sv_allowDownload->integer || idPack ) {
			if ( idPack ) {
				Com_Printf( "clientDownload: %d : \"%s\" cannot download id pk3 files\n",
							(int)( cl - svs.clients ), cl->downloadName );
				Com_sprintf( errorMessage, sizeof( errorMessage ),
							 "EXE_CANTAUTODLGAMEPAK\x15%s", cl->downloadName );
			} else {
				Com_Printf( "clientDownload: %d : \"%s\" download disabled",
							(int)( cl - svs.clients ), cl->downloadName );
				if ( sv_pure->integer ) {
					Com_sprintf( errorMessage, sizeof( errorMessage ),
								 "EXE_AUTODL_SERVERDISABLED_PURE\x15%s",
								 cl->downloadName );
				} else {
					Com_sprintf( errorMessage, sizeof( errorMessage ),
								 "EXE_AUTODL_SERVERDISABLED\x15%s",
								 cl->downloadName );
				}
			}

			MSG_WriteByte( msg, svc_download );
			MSG_WriteShort( msg, 0 );
			MSG_WriteLong( msg, -1 );
			MSG_WriteString( errorMessage, msg );

			cl->downloadName[0] = 0;
			return;
		}

		cl->downloadSize = FS_SV_FOpenFileRead( cl->downloadName, &cl->download );
		if ( cl->downloadSize <= 0 ) {
			Com_Printf( "clientDownload: %d : \"%s\" file not found on server\n",
						(int)( cl - svs.clients ), cl->downloadName );
			Com_sprintf( errorMessage, sizeof( errorMessage ),
						 "EXE_AUTODL_FILENOTONSERVER\x15%s", cl->downloadName );

			MSG_WriteByte( msg, svc_download );
			MSG_WriteShort( msg, 0 );
			MSG_WriteLong( msg, -1 );
			MSG_WriteString( errorMessage, msg );

			cl->downloadName[0] = 0;
			return;
		}

		cl->downloadXmitBlock = 0;
		cl->downloadClientBlock = 0;
		cl->downloadCurrentBlock = 0;
		cl->downloadCount = 0;
		cl->downloadEOF = qfalse;
	}

	while ( cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW
			&& cl->downloadSize != cl->downloadCount ) {

		curindex = cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW;

		if ( !cl->downloadBlocks[curindex] ) {
			cl->downloadBlocks[curindex] = (byte *)malloc( SV_DOWNLOAD_BLOCK_SIZE );
			if ( !cl->downloadBlocks[curindex] ) {
				Sys_OutOfMemoryError();
			}
			Com_Memset( cl->downloadBlocks[curindex], 0, SV_DOWNLOAD_BLOCK_SIZE );
		}

		cl->downloadBlockSize[curindex] = FS_Read( cl->downloadBlocks[curindex],
												   SV_DOWNLOAD_BLOCK_SIZE,
												   cl->download );
		if ( cl->downloadBlockSize[curindex] < 0 ) {
			cl->downloadCount = cl->downloadSize;
			break;
		}

		cl->downloadCount += cl->downloadBlockSize[curindex];
		cl->downloadCurrentBlock++;
	}

	if ( cl->downloadCount == cl->downloadSize
		 && !cl->downloadEOF
		 && cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW ) {
		cl->downloadBlockSize[cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW] = 0;
		cl->downloadCurrentBlock++;
		cl->downloadEOF = qtrue;
	}

	rate = cl->rate;
	if ( sv_maxRate->integer ) {
		if ( sv_maxRate->integer < 1000 ) {
			Cvar_Set2( "sv_MaxRate", "1000", qtrue );
		}
		if ( sv_maxRate->integer < rate ) {
			rate = sv_maxRate->integer;
		}
	}

	if ( !rate ) {
		blockspersnap = 1;
	} else {
		blockspersnap = ( ( rate * cl->snapshotMsec ) / 1000 + SV_DOWNLOAD_BLOCK_SIZE )
						/ SV_DOWNLOAD_BLOCK_SIZE;
		if ( blockspersnap < 0 ) {
			blockspersnap = 1;
		}
	}

	while ( blockspersnap-- ) {
		if ( cl->downloadClientBlock == cl->downloadCurrentBlock ) {
			return;
		}

		if ( cl->downloadXmitBlock == cl->downloadCurrentBlock ) {
			if ( svs.time - cl->downloadSendTime <= 1000 ) {
				return;
			}
			cl->downloadXmitBlock = cl->downloadClientBlock;
		}

		curindex = cl->downloadXmitBlock % MAX_DOWNLOAD_WINDOW;

		MSG_WriteByte( msg, svc_download );
		MSG_WriteShort( msg, cl->downloadXmitBlock );

		if ( !cl->downloadXmitBlock ) {
			MSG_WriteLong( msg, cl->downloadSize );
		}

		MSG_WriteShort( msg, cl->downloadBlockSize[curindex] );

		if ( cl->downloadBlockSize[curindex] ) {
			MSG_WriteData( msg, cl->downloadBlockSize[curindex],
						   cl->downloadBlocks[curindex] );
		}

		Com_DPrintf( "clientDownload: %d : writing block %d\n",
					 (int)( cl - svs.clients ), cl->downloadXmitBlock );

		cl->downloadXmitBlock++;
		cl->downloadSendTime = svs.time;
	}
}

/* ---- SV_Disconnect_f  0x00454B30 ---- */
void SV_Disconnect_f( client_t *cl ) {
	SV_DropClient( cl, "EXE_DISCONNECTED" );
}

/* ---- SV_VerifyPaks_f  0x00454B50 ---- */
void SV_VerifyPaks_f( client_t *cl ) {
	int nChkSum1, nChkSum2;
	int nClientPaks, nServerPaks;
	int i, j, nCurArg;
	int nClientChkSum[1024];
	int nServerChkSum[1024];
	const char  *pPaks;
	char        *pArg;
	qboolean bGood;

	nChkSum1 = nChkSum2 = 0;

	bGood = ( FS_FileIsInPAK( FS_ShiftStr( "eicogaoraz:80fnn", -2 ), &nChkSum1 ) == 1 );
	if ( bGood ) {
		bGood = ( FS_FileIsInPAK( FS_ShiftStr( "zndrud}=;3iqq", -5 ), &nChkSum2 ) == 1 );
	}

	nClientPaks = Cmd_Argc();

	nCurArg = 1;

	if ( !bGood || nClientPaks < 6 ) {
		cl->pureAuthentic = 2;
		return;
	}

	pArg = Cmd_Argv( nCurArg++ );
	if ( !pArg || *pArg == '@' ) {
		cl->pureAuthentic = 2;
		return;
	}

	if ( atoi( pArg ) != nChkSum1 ) {
		cl->pureAuthentic = 2;
		return;
	}

	pArg = Cmd_Argv( nCurArg++ );
	if ( !pArg || *pArg == '@' || atoi( pArg ) != nChkSum2 ) {
		cl->pureAuthentic = 2;
		return;
	}

	if ( *Cmd_Argv( nCurArg++ ) != '@' ) {
		cl->pureAuthentic = 2;
		return;
	}

	nClientChkSum[0] = 0;
	j = 0;
	for ( i = nCurArg ; i < nClientPaks ; i++ ) {
		nClientChkSum[++j] = atoi( Cmd_Argv( i ) );
	}

	nClientPaks = j - 1;

	for ( i = 0 ; i < nClientPaks ; i++ ) {
		for ( j = 0 ; j < nClientPaks ; j++ ) {
			if ( i == j ) {
				continue;
			}
			if ( nClientChkSum[i + 1] == nClientChkSum[j + 1] ) {
				cl->pureAuthentic = 2;
				return;
			}
		}
	}

	pPaks = FS_LoadedPakPureChecksums();
	Cmd_TokenizeString2( pPaks, 0 );
	nServerPaks = Cmd_Argc();
	if ( nServerPaks > 1024 ) {
		nServerPaks = 1024;
	}
	for ( i = 0 ; i < nServerPaks ; i++ ) {
		nServerChkSum[i] = atoi( Cmd_Argv( i ) );
	}

	for ( i = 0 ; i < nClientPaks ; i++ ) {
		for ( j = 0 ; j < nServerPaks ; j++ ) {
			if ( nClientChkSum[i + 1] == nServerChkSum[j] ) {
				break;
			}
		}
		if ( j >= nServerPaks ) {
			cl->pureAuthentic = 2;
			return;
		}
	}

	nChkSum1 = sv.checksumFeed;
	for ( i = 0 ; i < nClientPaks ; i++ ) {
		nChkSum1 ^= nClientChkSum[i + 1];
	}
	nChkSum1 ^= nClientPaks;
	if ( nChkSum1 != nClientChkSum[nClientPaks + 1] ) {
		cl->pureAuthentic = 2;
		return;
	}

	cl->pureAuthentic = 1;
}

/* ---- SV_ResetPureClient_f  0x00454D60 ---- */
void SV_ResetPureClient_f( client_t *cl ) {
	cl->pureAuthentic = 0;
}

/* ---- SV_UserinfoChanged  0x00454D70 ---- */
void SV_UserinfoChanged( client_t *cl ) {
	const char  *val;
	int i;

	Q_strncpyz( cl->name, Info_ValueForKey( cl->userinfo, "name" ),
				sizeof( cl->name ) );

	if ( Sys_IsLANAddress( cl->netchan.remoteAddress ) && com_dedicated->integer != 2 ) {
		cl->rate = 99999;
	} else {
		val = Info_ValueForKey( cl->userinfo, "rate" );
		if ( strlen( val ) ) {
			i = atoi( val );
			cl->rate = i;
			if ( cl->rate < 1000 ) {
				cl->rate = 1000;
			} else if ( cl->rate > 90000 ) {
				cl->rate = 90000;
			}
		} else {
			cl->rate = 5000;
		}
	}

	val = Info_ValueForKey( cl->userinfo, "handicap" );
	if ( strlen( val ) ) {
		i = atoi( val );
		if ( i <= 0 || i > 100 || strlen( val ) > 4 ) {
			Info_SetValueForKey( cl->userinfo, "handicap", "100" );
		}
	}

	val = Info_ValueForKey( cl->userinfo, "snaps" );
	if ( strlen( val ) ) {
		i = atoi( val );
		if ( i < 1 ) {
			i = 1;
		} else if ( i > 30 ) {
			i = 30;
		}
		cl->snapshotMsec = 1000 / i;
	} else {
		cl->snapshotMsec = 50;
	}
}

/* ---- SV_UpdateUserinfo_f  0x00454F10 ---- */
void SV_UpdateUserinfo_f( client_t *cl ) {
	Q_strncpyz( cl->userinfo, Cmd_Argv( 1 ), sizeof( cl->userinfo ) );

	SV_UserinfoChanged( cl );

	VM_Call( vm, GAME_CLIENT_USERINFO_CHANGED, (int)( cl - svs.clients ) );
}

typedef struct {
	const char  *name;
	void ( *func )( client_t *cl );
} ucmd_t;

static ucmd_t ucmds[] = {
	{ "userinfo",   SV_UpdateUserinfo_f },
	{ "disconnect", SV_Disconnect_f },
	{ "cp",         SV_VerifyPaks_f },
	{ "vdr",        SV_ResetPureClient_f },
	{ "download",   SV_BeginDownload_f },
	{ "nextdl",     SV_NextDownload_f },
	{ "stopdl",     SV_StopDownload_f },
	{ "donedl",     SV_DoneDownload_f },
	{ "retransdl",  SV_RetransmitDownload_f },
	{ NULL,         NULL }
};

/* ---- SV_ExecuteClientCommand  0x00454F80 ---- */
qboolean SV_ExecuteClientCommand( const char *cmd, client_t *cl, qboolean clientOK ) {
	ucmd_t  *u;

	xanim_activePoolSlot = 1;

	Cmd_TokenizeString2( cmd, 0 );

	for ( u = ucmds ; u->name ; u++ ) {
		if ( !strcmp( Cmd_Argv( 0 ), u->name ) ) {
			u->func( cl );
			break;
		}
	}

	if ( clientOK && !u->name && sv.state == SS_GAME ) {
		int r = VM_Call( vm, GAME_CLIENT_COMMAND, (int)( cl - svs.clients ) );

		if ( sv_debugSpawn && sv_debugSpawn->integer ) {
			Com_Printf( "SPAWNDBG: GAME_CLIENT_COMMAND cl %i \"%s\" -> %i\n",
						(int)( cl - svs.clients ), cmd, r );
		}
		return r;
	}

	return clientOK;
}

/* ---- SV_ClientCommand  0x00455060 ---- */
qboolean SV_ClientCommand( msg_t *msg, client_t *cl ) {
	int seq;
	const char  *s;
	qboolean clientOk;
	qboolean floodprotect;

	clientOk = qtrue;
	floodprotect = qtrue;

	seq = MSG_ReadLong( msg );
	s = MSG_ReadString( msg );

	if ( cl->lastClientCommand >= seq ) {
		return qtrue;
	}

	if ( sv_showCommands->integer ) {
		Com_Printf( "clientCommand: %i : %s\n", seq, s );
	}

	if ( seq > cl->lastClientCommand + 1 ) {
		Com_Printf( "Client %s lost %i clientCommands\n", cl->name,
					seq - cl->lastClientCommand + 1 );
		SV_DropClient( cl, "EXE_LOSTRELIABLECOMMANDS" );
		return qfalse;
	}

	if ( !Q_strncmp( s, "team ", 5 )
		 || !Q_strncmp( s, "score ", 6 )
		 || !Q_strncmp( s, "mr ", 3 ) ) {
		floodprotect = qfalse;
	}

	if ( !com_cl_running->integer
		 && cl->state >= CS_ACTIVE
		 && sv_floodProtect->integer
		 && svs.time < cl->nextReliableTime
		 && floodprotect ) {
		clientOk = qfalse;
		Com_DPrintf( "client text ignored for %s: %s\n", cl->name, Cmd_Argv( 0 ) );
	}

	if ( floodprotect ) {
		cl->nextReliableTime = svs.time + 800;
	}

	SV_ExecuteClientCommand( s, cl, clientOk );

	cl->lastClientCommand = seq;
	Com_sprintf( cl->lastClientCommandString,
				 sizeof( cl->lastClientCommandString ), "%s", s );

	return qtrue;
}

/* ---- SV_ClientThink  0x00455270 ---- */
void SV_ClientThink( client_t *cl, usercmd_t *cmd ) {
	cl->lastUsercmd = *cmd;

	if ( cl->state != CS_ACTIVE ) {
		return;
	}

	xanim_activePoolSlot = 1;
	VM_Call( vm, GAME_CLIENT_THINK, (int)( cl - svs.clients ) );

	if ( sv_debugSpawn && sv_debugSpawn->integer ) {
		static int nextDump[MAX_CLIENTS];
		int slot = (int)( cl - svs.clients );
		if ( slot >= 0 && slot < MAX_CLIENTS && svs.time >= nextDump[slot] ) {
			int n = slot;
			playerState_t *ps = (playerState_t *)( (byte *)sv.gameClients
												   + n * sv.gameClientSize );
			nextDump[slot] = svs.time + 500;
			Com_Printf( "SPAWNDBG: think cl %i psNum %i pm %i org %.1f %.1f %.1f "
						"vhC %.1f linked %i solid %x eFlags %x pmf %x va %.0f %.0f hint %i\n",
						n, ps->clientNum, (int)ps->pm_type,
						ps->origin[0], ps->origin[1], ps->origin[2],
						ps->viewHeightCurrent,
						cl->gentity ? cl->gentity->r.linked : -1,
						cl->gentity ? cl->gentity->s.solid : 0,
						ps->eFlags, ps->pm_flags, ps->viewangles[0], ps->viewangles[1],
						ps->serverCursorHint );
			Com_Printf( "SPAWNDBG:   weapon %i weapons %x %x weaponstate %i ammo %i cursorHintEnt %i\n",
						ps->weapon, ps->weapons[0], ps->weapons[1], ps->weaponstate,
						ps->ammo[0], ps->serverCursorHintTrace.entityNum );
		}
	}
}

/* ---- SV_UserMove  0x004552C0 ---- */
void SV_UserMove( qboolean delta, client_t *cl, msg_t *msg ) {
	int i;
	int key;
	int cmdCount;
	usercmd_t nullcmd;
	usercmd_t cmds[MAX_PACKET_USERCMDS];
	usercmd_t   *cmd, *oldcmd;
	const byte  *ps;

	if ( delta ) {
		cl->deltaMessage = cl->messageAcknowledge;
	} else {
		cl->deltaMessage = -1;
	}

	if ( cl->reliableSequence - cl->reliableAcknowledge >= MAX_RELIABLE_COMMANDS ) {
		return;
	}

	cmdCount = MSG_ReadByte( msg );

	if ( cmdCount < 1 ) {
		Com_Printf( "cmdCount < 1\n" );
		return;
	}
	if ( cmdCount > MAX_PACKET_USERCMDS ) {
		Com_Printf( "cmdCount > MAX_PACKET_USERCMDS\n" );
		return;
	}

	key = sv.checksumFeed;
	key ^= cl->messageAcknowledge;
	key ^= Com_HashKey(
		cl->reliableCommands[cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 )].command,
		32 );

	ps = (const byte *)sv.gameClients
		 + sv.gameClientSize * (int)( cl - svs.clients );

	memset( &nullcmd, 0, sizeof( nullcmd ) );
	MSG_SetDefaultUserCmd( ps, &nullcmd );
	oldcmd = &nullcmd;

	for ( i = 0 ; i < cmdCount ; i++ ) {
		cmd = &cmds[i];
		MSG_ReadDeltaUsercmdKey( msg, key, oldcmd, cmd );
		if ( !VM_Call( vm, 14, cmd->weapon ) ) {
			cmd->weapon = ps[176];
		}
		oldcmd = cmd;
	}

	cl->frames[cl->messageAcknowledge & PACKET_MASK].messageAcked = svs.time;

	if ( cl->state == CS_PRIMED ) {
		xanim_activePoolSlot = 1;
		SV_ClientEnterWorld( cl, &cmds[0] );
	}

	if ( sv_pure->integer && !cl->pureAuthentic ) {
		SV_DropClient( cl, "EXE_CANNOTVALIDATEPURECLIENT" );
		return;
	}

	if ( cl->state != CS_ACTIVE ) {
		cl->deltaMessage = -1;
		return;
	}

	for ( i = 0 ; i < cmdCount ; i++ ) {
		if ( cmds[i].serverTime > cmds[cmdCount - 1].serverTime ) {
			continue;
		}
		if ( cmds[i].serverTime <= cl->lastUsercmd.serverTime ) {
			continue;
		}
		SV_ClientThink( cl, &cmds[i] );
	}
}

/* ---- SV_AddTestClient  0x004556E0 ---- */
void SV_AddTestClient( void ) {
	int i;
	client_t    *cl;
	netadr_t adr;
	usercmd_t cmd;
	char userinfo[MAX_INFO_STRING];

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state == CS_FREE ) {
			break;
		}
	}
	if ( i == sv_maxclients->integer ) {
		return;
	}

	sprintf( userinfo,
			 "connect \"\\cg_predictItems\\1\\cl_anonymous\\0\\handicap\\100"
			 "\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000"
			 "\\name\\bot%d\\protocol\\%d\"",
			 sv_testClientCounter, PROTOCOL_VERSION );
	Cmd_TokenizeString2( userinfo, 0 );

	memset( &adr, 0, sizeof( adr ) );
	adr.type = NA_BOT;
	adr.port = (unsigned short)sv_testClientCounter;
	sv_testClientCounter++;
	SV_DirectConnect( adr );

	cl = svs.clients;
	for ( i = 0 ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( NET_CompareBaseAdr( adr, cl->netchan.remoteAddress ) ) {
			break;
		}
	}
	if ( i == sv_maxclients->integer ) {
		return;
	}

	xanim_activePoolSlot = 1;
	cl->bIsTestClient = 1;
	SV_SendClientGameState( cl );

	memset( &cmd, 0, sizeof( cmd ) );
	SV_ClientEnterWorld( cl, &cmd );
}

/* ---- SV_TestClientCommand_f  no-address ---- */
void SV_TestClientCommand_f( void ) {
	int      clientNum;
	client_t *cl;
	char     cmd[MAX_STRING_CHARS];
	int      i;

	if ( Cmd_Argc() < 3 ) {
		Com_Printf( "usage: testclientcmd <clientnum> <command ...>\n" );
		return;
	}

	clientNum = atoi( Cmd_Argv( 1 ) );
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Printf( "testclientcmd: bad client number %i\n", clientNum );
		return;
	}

	cl = &svs.clients[clientNum];
	if ( cl->state < CS_ACTIVE ) {
		Com_Printf( "testclientcmd: client %i is not active\n", clientNum );
		return;
	}

	cmd[0] = '\0';
	for ( i = 2 ; i < Cmd_Argc() ; i++ ) {
		if ( i > 2 ) {
			Q_strcat( cmd, sizeof( cmd ), " " );
		}
		if ( !strcmp( Cmd_Argv( i ), "@sid" ) ) {   /* the serverId "mr" wants */
			Q_strcat( cmd, sizeof( cmd ), va( "%i", Cvar_VariableIntegerValue( "sv_serverId" ) ) );
		} else {
			Q_strcat( cmd, sizeof( cmd ), Cmd_Argv( i ) );
		}
	}

	Com_Printf( "SPAWNDBG: injecting client command for %i: \"%s\"\n",
				clientNum, cmd );

	SV_ExecuteClientCommand( cmd, cl, qtrue );
}

/* ---- SV_TestClientMove_f  no-address ----  TEMPORARY scaffolding
   testclientmove <clientnum> <forwardmove> <rightmove> <buttons> <wbuttons> <weapon> <frames>
   Feeds a test client one usercmd per server frame for <frames> frames so a
   crash that needs a moving, firing or activating player can be reproduced
   on a dedicated server with no real client attached.  Angles are left at
   zero, so `setviewpos x y z yaw` (sv_cheats 1) fixes the view first. */
typedef struct {
	int         frames;
	usercmd_t   cmd;
} svTestMove_t;
static svTestMove_t sv_testMove[MAX_CLIENTS];

void SV_TestClientMove_f( void ) {
	int clientNum;

	if ( Cmd_Argc() < 8 ) {
		Com_Printf( "usage: testclientmove <clientnum> <forwardmove> <rightmove> <buttons> <wbuttons> <weapon> <frames>\n" );
		return;
	}
	clientNum = atoi( Cmd_Argv( 1 ) );
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	memset( &sv_testMove[clientNum], 0, sizeof( sv_testMove[clientNum] ) );
	sv_testMove[clientNum].cmd.forwardmove = (signed char)atoi( Cmd_Argv( 2 ) );
	sv_testMove[clientNum].cmd.rightmove   = (signed char)atoi( Cmd_Argv( 3 ) );
	sv_testMove[clientNum].cmd.buttons     = (byte)atoi( Cmd_Argv( 4 ) );
	sv_testMove[clientNum].cmd.wbuttons    = (byte)atoi( Cmd_Argv( 5 ) );
	sv_testMove[clientNum].cmd.weapon      = (byte)atoi( Cmd_Argv( 6 ) );
	sv_testMove[clientNum].frames          = atoi( Cmd_Argv( 7 ) );
	Com_Printf( "SPAWNDBG: testclientmove %i for %i frames\n", clientNum, sv_testMove[clientNum].frames );
}

/* testdumpmissiles -- TEMPORARY scaffolding: every ET_MISSILE entity's
   trajectory and angles, to see what the server hands the client. */
void SV_TestDumpMissiles_f( void ) {
	int i;
	gentity_t *ent;

	for ( i = 0 ; i < sv.num_entities ; i++ ) {
		ent = (gentity_t *)( (byte *)sv.gentities + sv.gentitySize * i );
		if ( ent->s.eType != 4 ) {
			continue;
		}
		Com_Printf( "SPAWNDBG: missile %i weapon %i pos type %i time %i base %.1f %.1f %.1f delta %.1f %.1f %.1f apos %.1f %.1f %.1f cur %.1f %.1f %.1f\n",
					i, ent->s.weapon, ent->s.pos.trType, ent->s.pos.trTime,
					ent->s.pos.trBase[0], ent->s.pos.trBase[1], ent->s.pos.trBase[2],
					ent->s.pos.trDelta[0], ent->s.pos.trDelta[1], ent->s.pos.trDelta[2],
					ent->s.apos.trBase[0], ent->s.apos.trBase[1], ent->s.apos.trBase[2],
					ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2] );
	}
}

void SV_TestClientFrame( void ) {
	int i;
	client_t *cl;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( sv_testMove[i].frames <= 0 ) {
			continue;
		}
		cl = &svs.clients[i];
		if ( !cl->bIsTestClient || cl->state != CS_ACTIVE ) {
			sv_testMove[i].frames = 0;
			continue;
		}
		sv_testMove[i].frames--;
		sv_testMove[i].cmd.serverTime = svs.time;
		SV_ClientThink( cl, &sv_testMove[i].cmd );
	}
}

/* ---- SV_ExecuteClientMessage  0x004554F0 ----  [LOW] */
void SV_ExecuteClientMessage( msg_t *msg, client_t *cl ) {
	int c;
	int serverId;
	msg_t decoded;
	byte decodedBuf[MAX_MSGLEN];

	if ( msgInit == qfalse ) {
		MSG_initHuffman();
	}

	memset( &decoded, 0, sizeof( decoded ) );
	decoded.data = decodedBuf;
	decoded.maxsize = MAX_MSGLEN;
	decoded.cursize = MSG_ReadBitsCompress( &msg->data[msg->readcount], decodedBuf,
											msg->cursize - msg->readcount );

	serverId = cl->serverId;

	if ( serverId != sv_rollingServerId && !cl->downloadName[0] ) {
		if ( ( ( (byte)sv_rollingServerId ^ (byte)serverId ) & 0xF0 ) != 0 ) {
			if ( cl->messageAcknowledge > cl->gamestateMessageNum ) {
				Com_DPrintf( "%s : dropped gamestate, resending\n", cl->name );
				SV_SendClientGameState( cl );
			}
		} else if ( cl->state == CS_PRIMED ) {
			xanim_activePoolSlot = 1;
			SV_ClientEnterWorld( cl, &cl->lastUsercmd );
		}
		return;
	}

	c = MSG_ReadBits( &decoded, 2 );
	while ( c == 2 ) {
		if ( !SV_ClientCommand( &decoded, cl ) ) {
			return;
		}
		if ( cl->state == CS_ZOMBIE ) {
			return;
		}
		c = MSG_ReadBits( &decoded, 2 );
	}

	if ( sv_pure->integer && cl->pureAuthentic == 2 ) {
		cl->nextSnapshotTime = -1;
		cl->state = CS_ACTIVE;
		SV_SendClientSnapshot( cl );
		SV_DropClient( cl, "EXE_UNPURECLIENTDETECTED" );
	}

	if ( c == 0 ) {
		SV_UserMove( qtrue, cl, &decoded );
	} else if ( c == 1 ) {
		SV_UserMove( qfalse, cl, &decoded );
	} else if ( c != 3 ) {
		Com_Printf( "WARNING: bad command byte %i for client %i\n",
					c, (int)( cl - svs.clients ) );
	}
}
