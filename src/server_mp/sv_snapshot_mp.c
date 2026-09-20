/*
 * @fidelity: verified
 */

#include "server.h"

#include <string.h>

void MSG_WriteByte( msg_t *msg, int c );
void MSG_WriteLong( msg_t *msg, int c );
void MSG_WriteBits( msg_t *msg, int value, int bits );
void MSG_WriteBit0( msg_t *msg );
void MSG_WriteBit1( msg_t *msg );
void MSG_WriteString( const char *s, msg_t *sb );
int MSG_WriteBitsCompress( const byte *datasrc, int bytecount, byte *buffdest, int capacity );
void MSG_WriteDeltaEntity( msg_t *msg, const byte *from, const byte *to, qboolean force );
void MSG_WriteDeltaClient( msg_t *msg, const byte *from, const byte *to, qboolean force );
void MSG_WriteDeltaPlayerstate( msg_t *msg, const byte *from, const byte *to, int number );

void MSG_WriteDeltaArchivedEntity( msg_t *msg, const byte *from, const byte *to,
								   qboolean force );

void MSG_WriteData( msg_t *msg, int length, const void *data );

int MSG_ReadBit( msg_t *msg );
qboolean MSG_ReadDeltaClient( msg_t *msg, const byte *from, byte *to, int number );
qboolean MSG_ReadDeltaArchivedEntity( msg_t *msg, const byte *from, byte *to,
									  int number );
void MSG_ReadDeltaPlayerstate( msg_t *msg, const byte *from, byte *to, int number );
void MSG_BeginReading( msg_t *msg );

/* qcommon/cm_test.c 0x00421E10. mins@ecx, maxs@eax in retail. */
int CM_BoxLeafnums( const vec3_t mins, const vec3_t maxs, int *list,
					int listsize, int *lastLeaf );

void Netchan_Transmit( netchan_t *chan, int length, const byte *data );
void Netchan_TransmitNextFragment( netchan_t *chan );

void SV_Netchan_Encode( byte *data, client_t *client, int length );

void SV_CloseDownload( client_t *cl );
void SV_WriteDownloadToClient( client_t *cl, msg_t *msg );

int CM_PointLeafnum( const vec3_t p );
int CM_LeafCluster( int leafnum );
int CM_LeafArea( int leafnum );
byte *CM_ClusterPVS( int cluster );
qboolean CM_AreasConnected( int area1, int area2 );

/* 0x01407444. cod1_globals.c types it `int`; four bytes either way. */
extern cvar_t   *cl_shownet;

void SV_AddEntToSnapshot( int *eNums, int gEntNum );
void SV_AddArchivedEntToSnapshot( int *eNums, int gEntNum );
cachedSnapshot_t *SV_GetCachedSnapshotInternal( int frameNum );
cachedSnapshot_t *SV_GetCachedSnapshot( int *frameTime );
void *SV_GetClientState( int clientNum );
static void SV_UpdateAverageBPS( int numclients );

int sv_archiveCachedBuilds;
int sv_archiveCachedClientInfos;
int sv_archiveCachedInfoChecks;
extern cvar_t   *sv_archiveForceTime;
qboolean SV_GetArchivedClientInfo( int *frameTime, void *clientInfo, int clientNum );

#define svc_nop                 1
#define svc_serverCommand       5
#define svc_snapshot            7
#define svc_EOF                 8

#define SNAPFLAG_RATE_DELAYED   1
#define SNAPFLAG_NOT_ACTIVE     2

#define MAX_SNAPSHOT_ENTITIES   1024

#define SVF_NOCLIENT            0x00000001
#define SVF_BROADCAST           0x00000008
#define SVF_PORTAL              0x00000010
#define SVF_SINGLECLIENT        0x00000800
#define SVF_NOTSINGLECLIENT     0x00002000

#define GAME_GET_CLIENT_INFO    8
#define GAME_GET_CLIENT_STATE   17
#define GAME_SET_ARCHIVE_TIME   19

#define SV_SNAPSHOT_CLIENT_BYTES 92

/* msg.maxsize the writer sets at 0x0045DEEF and the reader at 0x0045CBEB. Not MAX_MSGLEN -- eight times it. Both the archive writer and the archive reader put a buffer this size on the stack. */
#define SV_ARCHIVE_MSG_BYTES    0x20000

#define MSG_CLIENT_NUM_BITS     6

#define MAX_TOTAL_ENT_LEAFS     128

#define SV_NO_MORE_CLIENTS      99999

#define PMF_ARCHIVE_HIDE_VIEWENTITY 0x00010000

typedef struct svEntVis_s {
	int numClusters;
	int clusternums[MAX_ENT_CLUSTERS];
	int lastCluster;
	int areanum;
	int areanum2;
} svEntVis_t;

#define SV_ENT_VIS( svEnt )     ( (const svEntVis_t *)( (const byte *)( svEnt ) + 0x118 ) )

/* ---- SV_GentityNum  0x00455870 ---- */
static gentity_t *SV_GentityNum( int num ) {
	return (gentity_t *)( (byte *)sv.gentities + num * sv.gentitySize );
}

/* ---- SV_SvEntityNum  no-address ---- */
static svEntity_t *SV_SvEntityNum( int num ) {
	return &sv.svEntities[num];
}

/* ---- SV_EmitPacketEntities  0x0045BF80 ---- */
void SV_EmitPacketEntities( int from_first_entity, msg_t *msg,
							int from_num_entities, int to_num_entities,
							int to_first_entity ) {
	entityState_t   *oldent, *newent;
	int oldindex, newindex;
	int oldnum, newnum;

	oldent = NULL;
	newent = NULL;
	oldindex = 0;
	newindex = 0;

	while ( 1 ) {
		if ( newindex < to_num_entities ) {
			newent = &svs.snapshotEntities[to_first_entity % svs.numSnapshotEntities];
			newnum = newent->number;
		} else {
			if ( oldindex >= from_num_entities ) {
				break;
			}
			newnum = 9999;
		}

		if ( oldindex < from_num_entities ) {
			oldent = &svs.snapshotEntities[from_first_entity % svs.numSnapshotEntities];
			oldnum = oldent->number;
		} else {
			oldnum = 9999;
		}

		if ( newnum == oldnum ) {
			MSG_WriteDeltaEntity( msg, (const byte *)oldent,
								  (const byte *)newent, qfalse );
			oldindex++;
			from_first_entity++;
			newindex++;
			to_first_entity++;
			continue;
		}

		if ( newnum < oldnum ) {
			MSG_WriteDeltaEntity( msg,
								  (const byte *)&SV_SvEntityNum( newnum )->baseline,
								  (const byte *)newent, qtrue );
			newindex++;
			to_first_entity++;
			continue;
		}

		if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
			Com_Printf( "W|%3i: #%-3i remove\n", msg->cursize, oldent->number );
		}
		MSG_WriteBits( msg, oldent->number, 10 );
		MSG_WriteBit1( msg );
		oldindex++;
		from_first_entity++;
	}

	MSG_WriteBits( msg, ENTITYNUM_NONE, 10 );
}

/* ---- SV_EmitPacketClients  0x0045C0F0 ---- */
void SV_EmitPacketClients( int to_first_client, msg_t *msg,
						   int from_num_clients, int from_first_client,
						   int to_num_clients ) {
	const byte  *oldclient;
	const byte  *newclient;
	int oldindex, newindex;
	int oldnum, newnum;
	byte nullstate[SV_SNAPSHOT_CLIENT_BYTES];

	oldclient = NULL;
	newclient = NULL;
	oldindex = 0;
	newindex = 0;

	while ( 1 ) {
		if ( newindex < to_num_clients ) {
			newclient = (const byte *)svs.snapshotClients
						+ SV_SNAPSHOT_CLIENT_BYTES
						  * ( to_first_client % svs.numSnapshotClients );
			newnum = *(const int *)newclient;
		} else {
			if ( oldindex >= from_num_clients ) {
				break;
			}
			newnum = 9999;
		}

		if ( oldindex < from_num_clients ) {
			oldclient = (const byte *)svs.snapshotClients
						+ SV_SNAPSHOT_CLIENT_BYTES
						  * ( from_first_client % svs.numSnapshotClients );
			oldnum = *(const int *)oldclient;
		} else {
			oldnum = 9999;
		}

		if ( newnum == oldnum ) {
			if ( !oldclient ) {
				memset( nullstate, 0, sizeof( nullstate ) );
				oldclient = nullstate;
			}
			MSG_WriteDeltaClient( msg, oldclient, newclient, qfalse );
			oldindex++;
			from_first_client++;
			newindex++;
			to_first_client++;
			continue;
		}

		if ( newnum < oldnum ) {
			memset( nullstate, 0, sizeof( nullstate ) );
			MSG_WriteDeltaClient( msg, nullstate, newclient, qtrue );
			newindex++;
			to_first_client++;
			continue;
		}

		MSG_WriteDeltaClient( msg, oldclient, NULL, qtrue );
		oldindex++;
		from_first_client++;
	}

	MSG_WriteBit0( msg );
}

/* ---- SV_WriteSnapshotToClient  0x0045C2B0 ---- */
void SV_WriteSnapshotToClient( msg_t *msg, client_t *client ) {
	clientSnapshot_t    *frame;
	clientSnapshot_t    *oldframe;
	int lastframe;
	int snapFlags;
	int i;

	frame = &client->frames[client->netchan.outgoingSequence & PACKET_MASK];

	oldframe = NULL;
	lastframe = 0;

	if ( client->deltaMessage > 0 && client->state == CS_ACTIVE ) {
		lastframe = client->netchan.outgoingSequence - client->deltaMessage;
		if ( lastframe >= PACKET_BACKUP - 3 ) {
			Com_DPrintf( "%s: Delta request from out of date packet.\n",
						 client->name );
			lastframe = 0;
		} else {
			oldframe = &client->frames[client->deltaMessage & PACKET_MASK];
			if ( oldframe->first_entity
				 < svs.nextSnapshotEntities - svs.numSnapshotEntities ) {
				Com_DPrintf( "%s: Delta request from out of date entities.\n",
							 client->name );
				oldframe = NULL;
				lastframe = 0;
			}
		}
	}

	MSG_WriteByte( msg, svc_snapshot );

	MSG_WriteLong( msg, svs.time );

	MSG_WriteByte( msg, lastframe );

	snapFlags = svs.snapFlagServerBit;
	if ( client->rateDelayed ) {
		snapFlags |= SNAPFLAG_RATE_DELAYED;
	}
	if ( client->state == CS_ACTIVE ) {
		client->sendAsActive = qtrue;
	} else if ( client->state != CS_ZOMBIE ) {
		client->sendAsActive = qfalse;
	}
	if ( !client->sendAsActive ) {
		snapFlags |= SNAPFLAG_NOT_ACTIVE;
	}
	MSG_WriteByte( msg, snapFlags );

	if ( oldframe ) {
		MSG_WriteDeltaPlayerstate( msg, (const byte *)&oldframe->ps,
								   (const byte *)&frame->ps, 0 );
		SV_EmitPacketEntities( oldframe->first_entity, msg,
							   oldframe->num_entities, frame->num_entities,
							   frame->first_entity );
		SV_EmitPacketClients( frame->first_client, msg,
							  oldframe->num_clients, oldframe->first_client,
							  frame->num_clients );
	} else {
		MSG_WriteDeltaPlayerstate( msg, NULL, (const byte *)&frame->ps, 0 );
		SV_EmitPacketEntities( 0, msg, 0, frame->num_entities,
							   frame->first_entity );
		SV_EmitPacketClients( frame->first_client, msg, 0, 0,
							  frame->num_clients );
	}

	for ( i = 0 ; i < sv_padPackets->integer ; i++ ) {
		MSG_WriteByte( msg, svc_nop );
	}
}

/* ---- SV_UpdateServerCommandsToClient  0x0045C4A0 ---- */
void SV_UpdateServerCommandsToClient( msg_t *msg, client_t *client ) {
	int i;

	for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
		MSG_WriteByte( msg, svc_serverCommand );
		MSG_WriteLong( msg, i );
		MSG_WriteString(
			client->reliableCommands[i & ( MAX_RELIABLE_COMMANDS - 1 )].command,
			msg );
		if ( sv_debugSpawn && sv_debugSpawn->integer ) {
			Com_Printf( "SVCMD> cl%d seq%d cur%d [%.90s]\n",
						(int)( client - svs.clients ), i, msg->cursize,
						client->reliableCommands[i & ( MAX_RELIABLE_COMMANDS - 1 )].command );
		}
	}
	client->reliableSent = client->reliableSequence;
}

/* ---- SV_UpdateServerCommandsToClient_PreventOverflow  0x0045C530 ---- */
void SV_UpdateServerCommandsToClient_PreventOverflow( msg_t *msg, client_t *client,
													  int maxSize ) {
	int i;
	const char  *cmd;

	for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
		cmd = client->reliableCommands[i & ( MAX_RELIABLE_COMMANDS - 1 )].command;

		if ( msg->cursize + (int)strlen( cmd ) + 6 >= maxSize ) {
			break;
		}

		MSG_WriteByte( msg, svc_serverCommand );
		MSG_WriteLong( msg, i );
		MSG_WriteString( cmd, msg );
	}

	if ( i - 1 > client->reliableSent ) {
		client->reliableSent = i - 1;
	}
}

/* ---- SV_PrintServerCommandsForClient  0x0045C5E0 ---- */
void SV_PrintServerCommandsForClient( client_t *client ) {
	int i;

	Com_Printf( "-- Unacknowledged Server Commands for client %i:%s --\n",
				(int)( client - svs.clients ), client->name );

	for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
		Com_Printf( "cmd %5d: %8d: %s\n", i,
					client->reliableCommands[i & ( MAX_RELIABLE_COMMANDS - 1 )].cmdTime,
					client->reliableCommands[i & ( MAX_RELIABLE_COMMANDS - 1 )].command );
	}

	Com_Printf( "----------" );
}

/* ---- SV_AddEntToSnapshot  0x0045C670 ---- */
void SV_AddEntToSnapshot( int *eNums, int gEntNum ) {
	if ( eNums[0] == MAX_SNAPSHOT_ENTITIES ) {
		return;
	}
	eNums[eNums[0] + 1] = gEntNum;
	eNums[0]++;
}

/* ---- SV_AddArchivedEntToSnapshot  0x0045C690 ---- */
void SV_AddArchivedEntToSnapshot( int *eNums, int gEntNum ) {
	if ( eNums[0] == MAX_SNAPSHOT_ENTITIES ) {
		return;
	}
	eNums[eNums[0] + 1] = gEntNum;
	eNums[0]++;
}

/* ---- SV_AddEntitiesVisibleFromPoint  0x0045C6B0 ---- */
void SV_AddEntitiesVisibleFromPoint( int leafnum, int clientNum, int *eNums ) {
	int e, i;
	gentity_t   *ent;
	svEntity_t     *svEnt;
	const svEntVis_t *vis;
	int svFlags;
	int area;
	int cluster;
	const byte  *bitvector;
	int l;

	area = CM_LeafArea( leafnum );
	if ( area < 0 ) {
		return;
	}
	cluster = CM_LeafCluster( leafnum );
	bitvector = CM_ClusterPVS( cluster );

	for ( e = 0 ; e < sv.num_entities ; e++ ) {
		ent = SV_GentityNum( e );

		if ( !ent->r.linked ) {
			continue;
		}

		svFlags = ent->r.svFlags;

		if ( svFlags & SVF_NOCLIENT ) {
			continue;
		}
		if ( ( svFlags & SVF_SINGLECLIENT ) && ent->r.singleClient != clientNum ) {
			continue;
		}
		if ( ( svFlags & SVF_NOTSINGLECLIENT ) && ent->r.singleClient == clientNum ) {
			continue;
		}

		if ( e == clientNum ) {
			continue;
		}

		if ( svFlags & ( SVF_BROADCAST | SVF_PORTAL ) ) {
			SV_AddEntToSnapshot( eNums, e );
			continue;
		}

		svEnt = SV_SvEntityNum( ent->s.number );
		vis = SV_ENT_VIS( svEnt );

		if ( vis->areanum < 0 || !CM_AreasConnected( area, vis->areanum ) ) {
			if ( !CM_AreasConnected( area, vis->areanum2 ) ) {
				continue;
			}
		}

		if ( !vis->numClusters ) {
			continue;
		}

		l = 0;
		for ( i = 0 ; i < vis->numClusters ; i++ ) {
			l = vis->clusternums[i];
			if ( bitvector[l >> 3] & ( 1 << ( l & 7 ) ) ) {
				break;
			}
		}

		if ( i == vis->numClusters ) {
			if ( !vis->lastCluster ) {
				continue;
			}
			for ( ; l <= vis->lastCluster ; l++ ) {
				if ( bitvector[l >> 3] & ( 1 << ( l & 7 ) ) ) {
					break;
				}
			}
			if ( l == vis->lastCluster ) {
				continue;
			}
		}

		SV_AddEntToSnapshot( eNums, e );
	}
}

/* SV_GetClientArchiveTime 0x0045C8A0 */

cvar_t  *sv_archiveForceTime;

/* ---- SV_GetClientArchiveTime  0x0045C8A0 ---- */
int SV_GetClientArchiveTime( int clientNum ) {
	if ( sv_archiveForceTime && sv_archiveForceTime->integer ) {
		return sv_archiveForceTime->integer;
	}
	return VM_Call( vm, GAME_GET_ARCHIVE_TIME, clientNum );
}

/* ---- SV_AddCachedEntitiesVisibleFromPoint  0x0045C8C0 ---- */
void SV_AddCachedEntitiesVisibleFromPoint( int leafnum, int count, int firstEnt,
										   int clientNum, int *eNums,
										   const playerState_t *ps ) {
	const archivedEntity_t  *ae;
	const byte  *bitvector;
	int leafs[MAX_TOTAL_ENT_LEAFS];
	int lastLeaf;
	int area;
	int i, j, num;
	int svFlags;
	int leafArea, leafCluster;

	area = CM_LeafArea( leafnum );
	if ( area < 0 ) {
		return;
	}
	bitvector = CM_ClusterPVS( CM_LeafCluster( leafnum ) );

	for ( i = 0 ; i < count ; i++ ) {
		ae = &SVS_CACHED_ENTITIES[ ( firstEnt + i )
								   % SV_NUM_CACHED_SNAPSHOT_ENTITIES ];
		svFlags = ae->svFlags;

		if ( ( svFlags & SVF_SINGLECLIENT ) && ae->singleClient != clientNum ) {
			continue;
		}
		if ( ( svFlags & SVF_NOTSINGLECLIENT ) && ae->singleClient == clientNum ) {
			continue;
		}

		if ( ae->s.number == clientNum
			 && ( ps->pm_flags & PMF_ARCHIVE_HIDE_VIEWENTITY ) ) {
			continue;
		}

		if ( svFlags & ( SVF_BROADCAST | SVF_PORTAL ) ) {
			SV_AddArchivedEntToSnapshot( eNums, i );
			continue;
		}

		num = CM_BoxLeafnums( ae->absmin, ae->absmax, leafs,
							  MAX_TOTAL_ENT_LEAFS, &lastLeaf );
		if ( !num ) {
			continue;
		}

		for ( j = 0 ; j < num ; j++ ) {
			leafArea = CM_LeafArea( leafs[j] );
			if ( leafArea < 0 ) {
				continue;
			}
			if ( CM_AreasConnected( area, leafArea ) ) {
				break;
			}
		}
		if ( j == num ) {
			continue;
		}

		for ( j = 0 ; j < num ; j++ ) {
			leafCluster = CM_LeafCluster( leafs[j] );
			if ( leafCluster == -1 ) {
				continue;
			}
			if ( bitvector[leafCluster >> 3] & ( 1 << ( leafCluster & 7 ) ) ) {
				break;
			}
		}
		if ( j == num ) {
			continue;
		}

		SV_AddArchivedEntToSnapshot( eNums, i );
	}
}

/* ---- SV_GetClientState  0x0045CAC0 ---- */
void *SV_GetClientState( int clientNum ) {
	return (void *)VM_Call( vm, GAME_GET_CLIENT_STATE, clientNum );
}

/* ---- SV_SetClientArchiveTime  0x0045CAE0 ---- */
void SV_SetClientArchiveTime( int clientNum, int time ) {
	VM_Call( vm, GAME_SET_ARCHIVE_TIME, clientNum, time );
}

/* ---- SV_CachedClientAt  no-address ---- */
static const cachedClient_t *SV_CachedClientAt( const cachedSnapshot_t *base,
												int index, int *number ) {
	const cachedClient_t *cc;

	if ( index >= base->num_clients ) {
		*number = SV_NO_MORE_CLIENTS;
		return NULL;
	}
	cc = &SVS_CACHED_CLIENTS[ ( base->first_client + index )
							  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
	*number = *(const int *)cc->cs;
	return cc;
}

/* ---- SV_ArchivedFrameIsLive  no-address ---- */
static qboolean SV_ArchivedFrameIsLive( int frameNum ) {
	const archivedSnapshot_t *archived;

	archived = &svs.archivedSnapshotFrames[ frameNum
											% SV_NUM_ARCHIVED_SNAPSHOT_FRAMES ];
	return (qboolean)( archived->start >= svs.nextArchivedSnapshotBuffer
										 - SV_ARCHIVED_SNAPSHOT_BUFFER_BYTES );
}

/* ---- SV_GetCachedSnapshotInternal  0x0045CB00 ---- */
cachedSnapshot_t *SV_GetCachedSnapshotInternal( int frameNum ) {
	msg_t msg;
	const archivedSnapshot_t    *archived;
	cachedSnapshot_t    *cache;
	cachedSnapshot_t    *base;
	const cachedClient_t    *oldcc;
	cachedClient_t      *cc;
	archivedEntity_t    *ae;
	int oldestFrame, frameIndex;
	int baseFrame;
	int number, cachedNum, cachedIndex;
	qboolean matched;
	int offset, room, len;
	byte buf[SV_ARCHIVE_MSG_BYTES];

	if ( !SV_ArchivedFrameIsLive( frameNum ) ) {
		return NULL;
	}
	archived = &svs.archivedSnapshotFrames[ frameNum
											% SV_NUM_ARCHIVED_SNAPSHOT_FRAMES ];

	oldestFrame = svs.nextCachedSnapshotFrames - SV_NUM_CACHED_SNAPSHOT_FRAMES;
	if ( oldestFrame < 0 ) {
		oldestFrame = 0;
	}
	for ( frameIndex = svs.nextCachedSnapshotFrames - 1
		; frameIndex >= oldestFrame
		; frameIndex-- ) {
		cache = &SVS_CACHED_FRAMES[ frameIndex % SV_NUM_CACHED_SNAPSHOT_FRAMES ];
		if ( cache->archivedFrame != frameNum ) {
			continue;
		}
		if ( cache->first_entity
			 >= svs.nextCachedSnapshotEntities - SV_NUM_CACHED_SNAPSHOT_ENTITIES
			 && cache->first_client
				>= svs.nextCachedSnapshotClients - SV_NUM_CACHED_SNAPSHOT_CLIENTS ) {
			return cache;
		}
		break;
	}

	MSG_Init( &msg, buf, SV_ARCHIVE_MSG_BYTES );
	msg.extended = qtrue;
	len = archived->len;
	msg.cursize = len;

	offset = archived->start % SV_ARCHIVED_SNAPSHOT_BUFFER_BYTES;
	room = SV_ARCHIVED_SNAPSHOT_BUFFER_BYTES - offset;
	if ( len > room ) {
		memcpy( buf, svs.archivedSnapshotBuffer + offset, (size_t)room );
		memcpy( buf + room, svs.archivedSnapshotBuffer, (size_t)( len - room ) );
	} else {
		memcpy( buf, svs.archivedSnapshotBuffer + offset, (size_t)len );
	}

	if ( !MSG_ReadBit( &msg ) ) {
		baseFrame = MSG_ReadLong( &msg );
		if ( baseFrame < svs.nextArchivedSnapshotFrames
						 - SV_NUM_ARCHIVED_SNAPSHOT_FRAMES ) {
			return NULL;
		}
		if ( !SV_ArchivedFrameIsLive( baseFrame ) ) {
			return NULL;
		}
		base = SV_GetCachedSnapshotInternal( baseFrame );
		if ( !base ) {
			return NULL;
		}

		cache = &SVS_CACHED_FRAMES[ svs.nextCachedSnapshotFrames
									% SV_NUM_CACHED_SNAPSHOT_FRAMES ];
		cache->archivedFrame = frameNum;
		cache->num_entities  = 0;
		cache->first_entity  = svs.nextCachedSnapshotEntities;
		cache->num_clients   = 0;
		cache->first_client  = svs.nextCachedSnapshotClients;
		cache->usesDelta     = 1;
		cache->time          = MSG_ReadLong( &msg );

		cachedIndex = 0;
		oldcc = SV_CachedClientAt( base, cachedIndex, &cachedNum );

		while ( MSG_ReadBit( &msg ) ) {
			number = MSG_ReadBits( &msg, MSG_CLIENT_NUM_BITS );
			if ( msg.readcount > msg.cursize ) {
				Com_Error( ERR_DROP, "\x15SV_GetCachedSnapshot: end of message" );
			}

			while ( cachedNum < number ) {
				cachedIndex++;
				oldcc = SV_CachedClientAt( base, cachedIndex, &cachedNum );
			}

			cc = &SVS_CACHED_CLIENTS[ svs.nextCachedSnapshotClients
									  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
			matched = (qboolean)( cachedNum == number );

			if ( matched ) {
				MSG_ReadDeltaClient( &msg, oldcc->cs, cc->cs, number );
				cc->havePlayerState = MSG_ReadBit( &msg );
				if ( cc->havePlayerState ) {
					MSG_ReadDeltaPlayerstate( &msg, (const byte *)&oldcc->ps,
											  (byte *)&cc->ps, 0 );
				}
			} else {
				MSG_ReadDeltaClient( &msg, NULL, cc->cs, number );
				cc->havePlayerState = MSG_ReadBit( &msg );
				if ( cc->havePlayerState ) {
					MSG_ReadDeltaPlayerstate( &msg, NULL, (byte *)&cc->ps, 0 );
				}
			}

			svs.nextCachedSnapshotClients++;
			if ( svs.nextCachedSnapshotClients >= SV_SNAPSHOT_COUNTER_LIMIT ) {
				Com_Error( ERR_FATAL, "\x15svs.nextCachedSnapshotClients wrapped" );
			}
			cache->num_clients++;

			if ( matched ) {
				cachedIndex++;
				oldcc = SV_CachedClientAt( base, cachedIndex, &cachedNum );
			}
		}
	} else {
		cache = &SVS_CACHED_FRAMES[ svs.nextCachedSnapshotFrames
									% SV_NUM_CACHED_SNAPSHOT_FRAMES ];
		cache->archivedFrame = frameNum;
		cache->num_entities  = 0;
		cache->first_entity  = svs.nextCachedSnapshotEntities;
		cache->num_clients   = 0;
		cache->first_client  = svs.nextCachedSnapshotClients;
		cache->usesDelta     = 0;
		cache->time          = MSG_ReadLong( &msg );

		while ( MSG_ReadBit( &msg ) ) {
			number = MSG_ReadBits( &msg, MSG_CLIENT_NUM_BITS );
			if ( msg.readcount > msg.cursize ) {
				Com_Error( ERR_DROP, "\x15SV_GetCachedSnapshot: end of message" );
			}

			cc = &SVS_CACHED_CLIENTS[ svs.nextCachedSnapshotClients
									  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
			MSG_ReadDeltaClient( &msg, NULL, cc->cs, number );
			cc->havePlayerState = MSG_ReadBit( &msg );
			if ( cc->havePlayerState ) {
				MSG_ReadDeltaPlayerstate( &msg, NULL, (byte *)&cc->ps, 0 );
			}

			svs.nextCachedSnapshotClients++;
			if ( svs.nextCachedSnapshotClients >= SV_SNAPSHOT_COUNTER_LIMIT ) {
				Com_Error( ERR_FATAL, "\x15svs.nextCachedSnapshotClients wrapped" );
			}
			cache->num_clients++;
		}
	}

	while ( ( number = MSG_ReadBits( &msg, 10 ) ) != ENTITYNUM_NONE ) {
		if ( msg.readcount > msg.cursize ) {
			Com_Error( ERR_DROP, "\x15SV_GetCachedSnapshot: end of message" );
		}

		ae = &SVS_CACHED_ENTITIES[ svs.nextCachedSnapshotEntities
								   % SV_NUM_CACHED_SNAPSHOT_ENTITIES ];
		MSG_ReadDeltaArchivedEntity( &msg,
			(const byte *)&SV_SvEntityNum( number )->baseline,
			(byte *)ae, number );

		svs.nextCachedSnapshotEntities++;
		if ( svs.nextCachedSnapshotEntities >= SV_SNAPSHOT_COUNTER_LIMIT ) {
			Com_Error( ERR_FATAL, "\x15svs.nextCachedSnapshotEntities wrapped" );
		}
		cache->num_entities++;
	}

	svs.nextCachedSnapshotFrames++;
	if ( svs.nextCachedSnapshotFrames >= SV_SNAPSHOT_COUNTER_LIMIT ) {
		Com_Error( ERR_FATAL, "\x15svs.nextCachedSnapshotFrames wrapped" );
	}

	return cache;
}

/* ---- SV_GetCachedSnapshot  0x0045D400 ---- */
cachedSnapshot_t *SV_GetCachedSnapshot( int *frameTime ) {
	cachedSnapshot_t    *cache;
	int frameNum;
	int oldestFrame;
	int fps;

	if ( !svs.archiveEnabled ) {
		return NULL;
	}
	if ( *frameTime <= 0 ) {
		return NULL;
	}

	fps = sv_fps->integer;
	frameNum = svs.nextArchivedSnapshotFrames - ( *frameTime * fps ) / 1000;

	oldestFrame = svs.nextArchivedSnapshotFrames - SV_NUM_ARCHIVED_SNAPSHOT_FRAMES;
	if ( frameNum < oldestFrame ) {
		frameNum = oldestFrame;
		*frameTime = ( svs.nextArchivedSnapshotFrames - frameNum ) * 1000 / fps;
	}
	if ( frameNum < 0 ) {
		frameNum = 0;
		*frameTime = ( svs.nextArchivedSnapshotFrames - frameNum ) * 1000 / fps;
	}

	for ( ; frameNum < svs.nextArchivedSnapshotFrames ; frameNum++ ) {
		cache = SV_GetCachedSnapshotInternal( frameNum );
		if ( cache ) {
			return cache;
		}
	}

	*frameTime = 0;
	return NULL;
}

/* ---- SV_GetFollowPlayerState  0x0045D4B0 ---- */
void *SV_GetFollowPlayerState( int clientNum, void *playerState ) {
	return (void *)VM_Call( vm, GAME_GET_CLIENT_INFO, clientNum, playerState );
}

/* ---- SV_GetCurrentClientInfo  0x0045D4D0 ---- */
void *SV_GetCurrentClientInfo( int clientNum, void *clientInfo ) {
	if ( svs.clients[clientNum].state != CS_ACTIVE ) {
		return NULL;
	}
	return (void *)VM_Call( vm, GAME_GET_CLIENT_INFO, clientNum, clientInfo );
}

#define HUDELEM_FADE_START_TIME     0x24
#define HUDELEM_SCALE_START_TIME    0x44
#define HUDELEM_MOVE_START_TIME     0x54
#define HUDELEM_TIMER_VALUE         0x5C

#define SV_PS_HUD_ELEMS             31

/* ---- SV_RebaseTime  no-address ---- */
static void SV_RebaseTime( int *t, int timeDelta ) {
	if ( *t ) {
		*t += timeDelta;
	}
}

/* ---- SV_RebaseArchivedPlayerState  no-address ---- */
static void SV_RebaseArchivedPlayerState( playerState_t *ps, int timeDelta ) {
	byte *he;
	int i;

	SV_RebaseTime( &ps->commandTime, timeDelta );           /* +0x000 */
	SV_RebaseTime( &ps->pm_time, timeDelta );               /* +0x010 */
	SV_RebaseTime( &ps->iFoliageSoundTime, timeDelta );     /* +0x038 */
	SV_RebaseTime( &ps->jumpTime, timeDelta );              /* +0x064 */
	SV_RebaseTime( &ps->viewHeightLerpTime, timeDelta );    /* +0x0D4 */
	SV_RebaseTime( &ps->shellshockTime, timeDelta );        /* +0x3E0 */

	/* Only the ARCHIVAL hud array is rebased, not the current one. Retail walks it from ps+0x135C, which is &hud.archival[0] + 0x24. */
	for ( i = 0 ; i < SV_PS_HUD_ELEMS ; i++ ) {
		he = (byte *)&ps->hud.archival[i];
		SV_RebaseTime( (int *)( he + HUDELEM_TIMER_VALUE ), timeDelta );
		SV_RebaseTime( (int *)( he + HUDELEM_FADE_START_TIME ), timeDelta );
		SV_RebaseTime( (int *)( he + HUDELEM_SCALE_START_TIME ), timeDelta );
		SV_RebaseTime( (int *)( he + HUDELEM_MOVE_START_TIME ), timeDelta );
	}

	/* +0x20CC, and this one is NOT guarded -- retail adds unconditionally. */
	ps->deltaTime += timeDelta;
}

/* ---- SV_GetArchivedClientInfo  0x0045D500 ---- */
qboolean SV_GetArchivedClientInfo( int *frameTime, void *clientInfo, int clientNum ) {
	const cachedSnapshot_t  *cache;
	const cachedClient_t    *cc;
	playerState_t   *ps;
	int timeDelta;
	int i;

	cache = SV_GetCachedSnapshot( frameTime );
	if ( cache ) {
		sv_archiveCachedClientInfos++;
		timeDelta = svs.time - cache->time;

		for ( i = 0 ; i < cache->num_clients ; i++ ) {
			cc = &SVS_CACHED_CLIENTS[ ( cache->first_client + i )
									  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
			if ( *(const int *)cc->cs != clientNum ) {
				continue;
			}
			if ( !cc->havePlayerState ) {
				return qfalse;
			}
			ps = (playerState_t *)clientInfo;
			*ps = cc->ps;
			SV_RebaseArchivedPlayerState( ps, timeDelta );
			return qtrue;
		}
		return qfalse;
	}

	if ( *frameTime > 0 ) {
		return qfalse;
	}
	if ( svs.clients[clientNum].state != CS_ACTIVE ) {
		return qfalse;
	}
	return (qboolean)VM_Call( vm, GAME_GET_CLIENT_INFO, clientNum, clientInfo );
}

/* ---- SV_BuildClientSnapshot  0x0045D650 ---- */
void SV_BuildClientSnapshot( client_t *cl ) {
	clientSnapshot_t    *frame;
	const cachedSnapshot_t  *cache;
	const cachedClient_t    *cc;
	const archivedEntity_t  *ae;
	entityState_t       *es;
	playerState_t       *ps;
	int clientNum;
	int viewEnt;
	int frameTime;
	int timeDelta;
	int i;
	int leafnum;
	vec3_t org;
	int eNums[MAX_SNAPSHOT_ENTITIES + 1];

	frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
	frame->num_entities = 0;
	frame->num_clients = 0;

	if ( !cl->gentity ) {
		return;
	}
	if ( cl->state == CS_ZOMBIE ) {
		return;
	}

	frame->first_entity = svs.nextSnapshotEntities;
	frame->first_client = svs.nextSnapshotClients;

	if ( sv.state == SS_DEAD ) {
		return;
	}

	clientNum = (int)( cl - svs.clients );
	eNums[0] = 0;

	frameTime = SV_GetClientArchiveTime( clientNum );
	cache = SV_GetCachedSnapshot( &frameTime );
	SV_SetClientArchiveTime( clientNum, frameTime );

	timeDelta = 0;
	if ( cache ) {
		timeDelta = svs.time - cache->time;
	}

	ps = (playerState_t *)( (byte *)sv.gameClients + clientNum * sv.gameClientSize );
	frame->ps = *ps;

	viewEnt = frame->ps.clientNum;
	if ( viewEnt < 0 || viewEnt >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "\x15SV_BuildClientSnapshot: bad gEnt" );
	}

	org[0] = frame->ps.origin[0];
	org[1] = frame->ps.origin[1];
	org[2] = frame->ps.origin[2] + frame->ps.viewHeightCurrent;
	AddLeanToPosition( org, frame->ps.viewangles[1], frame->ps.leanf,
					   16.0f, 20.0f );

	leafnum = CM_PointLeafnum( org );

	if ( cache ) {
		sv_archiveCachedBuilds++;
		SV_AddCachedEntitiesVisibleFromPoint( leafnum, cache->num_entities,
											  cache->first_entity, viewEnt,
											  eNums, &frame->ps );

		for ( i = 0 ; i < eNums[0] ; i++ ) {
			ae = &SVS_CACHED_ENTITIES[ ( eNums[i + 1] + cache->first_entity )
									   % SV_NUM_CACHED_SNAPSHOT_ENTITIES ];

			es = &svs.snapshotEntities[ svs.nextSnapshotEntities
										% svs.numSnapshotEntities ];
			*es = ae->s;

			SV_RebaseTime( &es->pos.trTime, timeDelta );
			SV_RebaseTime( &es->apos.trTime, timeDelta );
			SV_RebaseTime( &es->time, timeDelta );
			SV_RebaseTime( &es->time2, timeDelta );

			svs.nextSnapshotEntities++;
			if ( svs.nextSnapshotEntities >= SV_SNAPSHOT_COUNTER_LIMIT ) {
				Com_Error( ERR_FATAL, "\x15svs.nextSnapshotEntities wrapped" );
			}
			frame->num_entities++;
		}

		for ( i = 0 ; i < cache->num_clients ; i++ ) {
			cc = &SVS_CACHED_CLIENTS[ ( cache->first_client + i )
									  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
			memcpy( (byte *)svs.snapshotClients
					+ SV_SNAPSHOT_CLIENT_BYTES
					  * ( svs.nextSnapshotClients % svs.numSnapshotClients ),
					cc->cs,
					SV_SNAPSHOT_CLIENT_BYTES );

			svs.nextSnapshotClients++;
			if ( svs.nextSnapshotClients >= SV_SNAPSHOT_COUNTER_LIMIT ) {
				Com_Error( ERR_FATAL, "\x15svs.nextSnapshotClients wrapped" );
			}
			frame->num_clients++;
		}
		return;
	}

	SV_AddEntitiesVisibleFromPoint( leafnum, viewEnt, eNums );

	for ( i = 0 ; i < eNums[0] ; i++ ) {
		svs.snapshotEntities[svs.nextSnapshotEntities % svs.numSnapshotEntities] =
			*(const entityState_t *)SV_GentityNum( eNums[i + 1] );

		svs.nextSnapshotEntities++;
		if ( svs.nextSnapshotEntities >= SV_SNAPSHOT_COUNTER_LIMIT ) {
			Com_Error( ERR_FATAL, "\x15svs.nextSnapshotEntities wrapped" );
		}
		frame->num_entities++;
	}

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state < CS_CONNECTED ) {
			continue;
		}
		memcpy( (byte *)svs.snapshotClients
				+ SV_SNAPSHOT_CLIENT_BYTES
				  * ( svs.nextSnapshotClients % svs.numSnapshotClients ),
				SV_GetClientState( i ),
				SV_SNAPSHOT_CLIENT_BYTES );

		svs.nextSnapshotClients++;
		if ( svs.nextSnapshotClients >= SV_SNAPSHOT_COUNTER_LIMIT ) {
			Com_Error( ERR_FATAL, "\x15svs.nextSnapshotClients wrapped" );
		}
		frame->num_clients++;
	}
}

/* ---- SV_RateMsec  0x0045DA80 ---- */
int SV_RateMsec( client_t *client, int messageSize ) {
	int rate;

	if ( messageSize > 1500 ) {
		messageSize = 1500;
	}

	rate = client->rate;
	if ( sv_maxRate->integer ) {
		if ( sv_maxRate->integer < 1000 ) {
			Cvar_Set2( "sv_MaxRate", "1000", qtrue );
		}
		if ( sv_maxRate->integer < rate ) {
			rate = sv_maxRate->integer;
		}
	}

	return ( messageSize + 48 ) * 1000 / rate;
}

/* ---- SV_SendMessageToClient  0x0045DAE0 ---- */
void SV_SendMessageToClient( msg_t *msg, client_t *client ) {
	int rateMsec;
	int len;
	byte outBuf[MAX_MSGLEN];

	*(int *)outBuf = *(const int *)msg->data;
	len = 4 + MSG_WriteBitsCompress( msg->data + 4, msg->cursize - 4, outBuf + 4, sizeof( outBuf ) - 4 );

	if ( client->dropReason ) {
		SV_DropClient( client, client->dropReason );
	}

	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSize = len;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSent = svs.time;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageAcked = -1;

	SV_Netchan_Encode( outBuf + 4, client, len - 4 );
	Netchan_Transmit( &client->netchan, len, outBuf );

	if ( client->netchan.remoteAddress.type == NA_LOOPBACK
		 || Sys_IsLANAddress( client->netchan.remoteAddress ) ) {
		client->nextSnapshotTime = svs.time - 1;
		return;
	}

	rateMsec = SV_RateMsec( client, len );
	if ( rateMsec < client->snapshotMsec ) {
		rateMsec = client->snapshotMsec;
		client->rateDelayed = qfalse;
	} else {
		client->rateDelayed = qtrue;
	}

	client->nextSnapshotTime = svs.time + rateMsec;

	if ( client->state != CS_ACTIVE && !client->downloadName[0]
		 && client->nextSnapshotTime < svs.time + 1000 ) {
		client->nextSnapshotTime = svs.time + 1000;
	}

	sv.bpsTotalBytes += len;
}

/* ---- SV_SendClientSnapshot  0x0045DC70 ---- */
void SV_SendClientSnapshot( client_t *cl ) {
	msg_t msg;
	byte msgBuffer[MAX_MSGLEN];

	if ( cl->state == CS_ACTIVE || cl->state == CS_ZOMBIE ) {
		SV_BuildClientSnapshot( cl );
	}

	MSG_Init( &msg, msgBuffer, cl->netchan.extended ? MAX_MSGLEN : STOCK_MAX_MSGLEN );
	msg.extended = cl->netchan.extended;

	MSG_WriteLong( &msg, cl->lastClientCommand );

	if ( cl->state == CS_ACTIVE || cl->state == CS_ZOMBIE ) {
		SV_UpdateServerCommandsToClient( &msg, cl );
		SV_WriteSnapshotToClient( &msg, cl );
		if ( cl->state != CS_ZOMBIE ) {
			SV_WriteDownloadToClient( cl, &msg );
		}
	} else {
		SV_WriteDownloadToClient( cl, &msg );
	}

	MSG_WriteByte( &msg, svc_EOF );

	if ( msg.overflowed ) {
		Com_Printf( "WARNING: msg overflowed for %s, trying to recover\n",
					cl->name );

		if ( cl->state == CS_ACTIVE || cl->state == CS_ZOMBIE ) {
			SV_PrintServerCommandsForClient( cl );

			MSG_Init( &msg, msgBuffer, cl->netchan.extended ? MAX_MSGLEN : STOCK_MAX_MSGLEN );
	msg.extended = cl->netchan.extended;
			MSG_WriteLong( &msg, cl->lastClientCommand );
			SV_UpdateServerCommandsToClient_PreventOverflow( &msg, cl, MAX_MSGLEN );
			MSG_WriteByte( &msg, svc_EOF );
		}

		if ( msg.overflowed ) {
			Com_Printf( "WARNING: client disconnected for msg overflow: %s\n",
						cl->name );
			NET_OutOfBandPrint( NS_SERVER, cl->netchan.remoteAddress,
								"disconnect" );
			SV_DropClient( cl, "EXE_SERVERMESSAGEOVERFLOW" );
		}
	}

	SV_SendMessageToClient( &msg, cl );
}

/* SV_ArchiveSnapshot 0x0045DE80 */

/* svFlags bits that force an entity into the archive regardless of whether it is in any PVS cluster. Retail tests `& 18h` (0x0045E5D9, 0x0045E20C). */
#define SVF_ARCHIVE_ALWAYS      ( SVF_BROADCAST | SVF_PORTAL )

/* ---- SV_FillArchivedEntity  no-address ---- */
static void SV_FillArchivedEntity( archivedEntity_t *ae, const gentity_t *ent ) {
	ae->s = ent->s;
	ae->svFlags = ent->r.svFlags;
	ae->singleClient = ent->r.singleClient;
	ae->absmin[0] = ent->r.absmin[0];
	ae->absmin[1] = ent->r.absmin[1];
	ae->absmin[2] = ent->r.absmin[2];
	ae->absmax[0] = ent->r.absmax[0];
	ae->absmax[1] = ent->r.absmax[1];
	ae->absmax[2] = ent->r.absmax[2];
}

/* ---- SV_ArchiveEntityVisible  no-address ---- */
static qboolean SV_ArchiveEntityVisible( const gentity_t *ent ) {
	const svEntity_t *svEnt;

	if ( !ent->r.linked ) {
		return qfalse;
	}
	if ( ent->r.svFlags & SVF_NOCLIENT ) {
		return qfalse;
	}
	if ( ent->s.number < 0 || ent->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "\x15SV_SvEntityForGentity: bad gEnt" );
	}
	svEnt = SV_SvEntityNum( ent->s.number );
	if ( ( ent->r.svFlags & SVF_ARCHIVE_ALWAYS ) == 0
		 && !SV_ENT_VIS( svEnt )->numClusters ) {
		return qfalse;
	}
	return qtrue;
}

static cvar_t   *sv_archiveSelfTest;

static byte     st_scratch[0x8000];
static playerState_t    st_canonPs;
static archivedEntity_t st_canonEnt;

/* ---- SV_CanonicalPlayerState  no-address ---- */
static void SV_CanonicalPlayerState( const playerState_t *live ) {
	msg_t m;

	MSG_Init( &m, st_scratch, sizeof( st_scratch ) );
	MSG_WriteDeltaPlayerstate( &m, NULL, (const byte *)live, 0 );
	MSG_BeginReading( &m );
	MSG_ReadDeltaPlayerstate( &m, NULL, (byte *)&st_canonPs, 0 );
}

/* ---- SV_CanonicalArchivedEntity  no-address ---- */
static void SV_CanonicalArchivedEntity( const byte *baseline,
										const archivedEntity_t *live ) {
	msg_t m;
	int number;

	MSG_Init( &m, st_scratch, sizeof( st_scratch ) );
	MSG_WriteDeltaArchivedEntity( &m, baseline, (const byte *)live, qtrue );
	MSG_BeginReading( &m );

	number = MSG_ReadBits( &m, 10 );

	MSG_ReadDeltaArchivedEntity( &m, baseline, (byte *)&st_canonEnt, number );
}

/* ---- SV_ArchiveSelfTest  no-address ---- */
static void SV_ArchiveSelfTest( int frameNum ) {
	const cachedSnapshot_t  *decoded;
	cachedSnapshot_t        *writerEntry;
	const cachedClient_t    *cc;
	const archivedEntity_t  *ae;
	const gentity_t         *ent;
	archivedEntity_t expectEnt;
	playerState_t expectPs;
	int savedArchivedFrame;
	int frameIndex, oldestFrame;
	int i, k, bad;

	writerEntry = NULL;
	savedArchivedFrame = 0;
	oldestFrame = svs.nextCachedSnapshotFrames - SV_NUM_CACHED_SNAPSHOT_FRAMES;
	if ( oldestFrame < 0 ) {
		oldestFrame = 0;
	}
	for ( frameIndex = svs.nextCachedSnapshotFrames - 1
		; frameIndex >= oldestFrame
		; frameIndex-- ) {
		writerEntry = &SVS_CACHED_FRAMES[ frameIndex
										  % SV_NUM_CACHED_SNAPSHOT_FRAMES ];
		if ( writerEntry->archivedFrame == frameNum ) {
			break;
		}
		writerEntry = NULL;
	}
	if ( writerEntry ) {
		savedArchivedFrame = writerEntry->archivedFrame;
		writerEntry->archivedFrame = -1;
	}

	decoded = SV_GetCachedSnapshotInternal( frameNum );

	if ( writerEntry ) {
		writerEntry->archivedFrame = savedArchivedFrame;
	}

	if ( !decoded ) {
		Com_Printf( "archiveSelfTest %d: FAIL -- decode returned NULL\n",
					frameNum );
		return;
	}

	bad = 0;

	k = 0;
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state < CS_CONNECTED ) {
			continue;
		}
		if ( k >= decoded->num_clients ) {
			Com_Printf( "archiveSelfTest %d: FAIL -- client %d missing"
						" (decoded %d)\n", frameNum, i, decoded->num_clients );
			bad++;
			break;
		}
		cc = &SVS_CACHED_CLIENTS[ ( decoded->first_client + k )
								  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];

		if ( memcmp( cc->cs, SV_GetClientState( i ),
					 SV_SNAPSHOT_CLIENT_BYTES ) != 0 ) {
			Com_Printf( "archiveSelfTest %d: FAIL -- client %d record differs\n",
						frameNum, i );
			bad++;
		}

		memset( &expectPs, 0, sizeof( expectPs ) );
		if ( VM_Call( vm, GAME_GET_CLIENT_INFO, i, &expectPs ) ) {
			if ( !cc->havePlayerState ) {
				Com_Printf( "archiveSelfTest %d: FAIL -- client %d lost its"
							" playerState\n", frameNum, i );
				bad++;
			} else if ( memcmp( &cc->ps, &expectPs, sizeof( expectPs ) ) != 0 ) {
				const int *got;
				const int *want;
				const int *canon;
				int wrong = 0;
				int w;

				SV_CanonicalPlayerState( &expectPs );
				got = (const int *)&cc->ps;
				want = (const int *)&expectPs;
				canon = (const int *)&st_canonPs;

				for ( w = 0 ; w < (int)( sizeof( expectPs ) / 4 ) ; w++ ) {
					if ( got[w] == want[w] ) {
						continue;
					}
					if ( got[w] == canon[w] ) {
						continue;
					}
					if ( canon[w] != want[w] ) {
						continue;
					}
					if ( wrong < 8 ) {
						Com_Printf( "archiveSelfTest %d: FAIL -- client %d"
									" playerState +0x%04X got %08X want %08X"
									" canon %08X\n",
									frameNum, i, w * 4, got[w], want[w],
									canon[w] );
					}
					wrong++;
				}
				if ( wrong ) {
					Com_Printf( "    %d words wrong of %d\n",
								wrong, (int)( sizeof( expectPs ) / 4 ) );
					bad++;
				}
			}
		} else if ( cc->havePlayerState ) {
			Com_Printf( "archiveSelfTest %d: FAIL -- client %d gained a"
						" playerState\n", frameNum, i );
			bad++;
		}
		k++;
	}
	if ( k != decoded->num_clients ) {
		Com_Printf( "archiveSelfTest %d: FAIL -- %d clients decoded, %d live\n",
					frameNum, decoded->num_clients, k );
		bad++;
	}

	k = 0;
	for ( i = 0 ; i < sv.num_entities ; i++ ) {
		ent = SV_GentityNum( i );
		if ( !SV_ArchiveEntityVisible( ent ) ) {
			continue;
		}
		if ( k >= decoded->num_entities ) {
			Com_Printf( "archiveSelfTest %d: FAIL -- entity %d missing"
						" (decoded %d)\n", frameNum, i, decoded->num_entities );
			bad++;
			break;
		}
		ae = &SVS_CACHED_ENTITIES[ ( decoded->first_entity + k )
								   % SV_NUM_CACHED_SNAPSHOT_ENTITIES ];
		SV_FillArchivedEntity( &expectEnt, ent );

		SV_CanonicalArchivedEntity(
			(const byte *)&SV_SvEntityNum( ent->s.number )->baseline,
			&expectEnt );

		if ( memcmp( ae, &st_canonEnt, sizeof( st_canonEnt ) ) != 0 ) {
			const int *got = (const int *)ae;
			const int *canon = (const int *)&st_canonEnt;
			const int *want = (const int *)&expectEnt;
			int shown = 0;
			int w;

			for ( w = 0 ; w < (int)( sizeof( expectEnt ) / 4 ) ; w++ ) {
				if ( got[w] == canon[w] ) {
					continue;
				}
				if ( shown < 6 ) {
					Com_Printf( "archiveSelfTest %d: FAIL -- entity %d"
								" +0x%04X got %08X canon %08X live %08X\n",
								frameNum, i, w * 4, got[w], canon[w], want[w] );
				}
				shown++;
			}
			bad++;
		}
		k++;
	}
	if ( k != decoded->num_entities ) {
		Com_Printf( "archiveSelfTest %d: FAIL -- %d entities decoded, %d live\n",
					frameNum, decoded->num_entities, k );
		bad++;
	}

	if ( sv_archiveForceTime && sv_archiveForceTime->integer && !bad ) {
		static playerState_t probePs;
		const cachedSnapshot_t *served;
		int askTime, lookTime;

		for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
			if ( svs.clients[i].state < CS_CONNECTED ) {
				continue;
			}

			lookTime = sv_archiveForceTime->integer;
			served = SV_GetCachedSnapshot( &lookTime );
			if ( !served ) {
				continue;
			}
			cc = NULL;
			for ( k = 0 ; k < served->num_clients ; k++ ) {
				const cachedClient_t *probe =
					&SVS_CACHED_CLIENTS[ ( served->first_client + k )
										 % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
				if ( *(const int *)probe->cs == i ) {
					cc = probe;
					break;
				}
			}
			if ( !cc || !cc->havePlayerState ) {
				continue;
			}

			askTime = sv_archiveForceTime->integer;
			memset( &probePs, 0, sizeof( probePs ) );
			if ( !SV_GetArchivedClientInfo( &askTime, &probePs, i ) ) {
				Com_Printf( "archiveSelfTest %d: FAIL -- GetArchivedClientInfo"
							" refused client %d\n", frameNum, i );
				bad++;
				continue;
			}

			{
				const int *served_ps = (const int *)&cc->ps;
				const int *out = (const int *)&probePs;
				int stray = 0;
				int w, off;

				for ( w = 0 ; w < (int)( sizeof( probePs ) / 4 ) ; w++ ) {
					off = w * 4;
					if ( out[w] == served_ps[w] ) {
						continue;
					}
					if ( off == 0x000 || off == 0x010 || off == 0x038
						 || off == 0x064 || off == 0x0D4 || off == 0x3E0
						 || off == 0x20CC ) {
						continue;
					}
					if ( off >= 4920 && off < 4920 + 31 * 112 ) {
						int within = ( off - 4920 ) % 112;
						if ( within == 0x24 || within == 0x44
							 || within == 0x54 || within == 0x5C ) {
							continue;
						}
					}
					if ( stray < 6 ) {
						Com_Printf( "archiveSelfTest %d: FAIL -- client %d"
									" archived info +0x%04X got %08X cached"
									" %08X\n", frameNum, i, off, out[w],
									served_ps[w] );
					}
					stray++;
				}
				if ( stray ) {
					bad++;
				}
			}
			sv_archiveCachedInfoChecks++;
		}
	}

	if ( !bad ) {
		Com_Printf( "archiveSelfTest %d: PASS %s -- %d clients, %d entities,"
					" time %d, cachedBuilds %d, cachedInfos %d, infoChecks %d\n",
					frameNum, writerEntry ? "keyframe" : "delta",
					decoded->num_clients, decoded->num_entities,
					decoded->time,
					sv_archiveCachedBuilds, sv_archiveCachedClientInfos,
					sv_archiveCachedInfoChecks );
	}
}

/* ---- SV_ArchiveSnapshot  0x0045DE80 ---- */
void SV_ArchiveSnapshot( void ) {
	msg_t msg;
	cachedSnapshot_t    *cache;
	cachedSnapshot_t    *candidate;
	archivedSnapshot_t  *archived;
	cachedClient_t      *cc;
	archivedEntity_t    *ae;
	archivedEntity_t localEnt;
	const gentity_t     *ent;
	const byte          *liveClient;
	playerState_t ps;
	int clientIndex, cachedIndex, cachedNum;
	int frameIndex, oldestFrame, staleBefore;
	int i, len, offset, room;
	byte buf[SV_ARCHIVE_MSG_BYTES];

	if ( sv.state == SS_DEAD ) {
		return;
	}
	if ( !svs.archiveEnabled ) {
		return;
	}

	/* MSG_Init is retail's own lazy MSG_initHuffman gate plus the memset, data and maxsize stores -- exactly what is inlined at 0x0045DEBC. */
	MSG_Init( &msg, buf, SV_ARCHIVE_MSG_BYTES );
	msg.extended = qtrue;

	oldestFrame = svs.nextCachedSnapshotFrames - SV_NUM_CACHED_SNAPSHOT_FRAMES;
	if ( oldestFrame < 0 ) {
		oldestFrame = 0;
	}
	staleBefore = svs.nextArchivedSnapshotFrames - sv_fps->integer;

	cache = NULL;
	for ( frameIndex = svs.nextCachedSnapshotFrames - 1
		; frameIndex >= oldestFrame
		; frameIndex-- ) {
		candidate = &SVS_CACHED_FRAMES[ frameIndex % SV_NUM_CACHED_SNAPSHOT_FRAMES ];
		if ( candidate->archivedFrame >= staleBefore && !candidate->usesDelta ) {
			cache = candidate;
			break;
		}
	}

	if ( cache
		 && ( cache->first_entity
			  < svs.nextCachedSnapshotEntities - SV_NUM_CACHED_SNAPSHOT_ENTITIES
			  || cache->first_client
				 < svs.nextCachedSnapshotClients - SV_NUM_CACHED_SNAPSHOT_CLIENTS ) ) {
		cache = NULL;
	}

	if ( cache ) {
		MSG_WriteBit0( &msg );
		MSG_WriteData( &msg, 4, &cache->archivedFrame );
		MSG_WriteData( &msg, 4, &svs.time );

		clientIndex = 0;
		cachedIndex = 0;

		while ( 1 ) {
			if ( clientIndex < sv_maxclients->integer ) {
				if ( svs.clients[clientIndex].state < CS_CONNECTED ) {
					clientIndex++;
					continue;
				}
			} else if ( cachedIndex >= cache->num_clients ) {
				break;
			}

			if ( cachedIndex < cache->num_clients ) {
				cc = &SVS_CACHED_CLIENTS[ ( cache->first_client + cachedIndex )
										  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
				cachedNum = *(const int *)cc->cs;
			} else {
				cc = NULL;
				cachedNum = 9999;
			}

			if ( clientIndex > cachedNum ) {
				cachedIndex++;
				continue;
			}

			liveClient = (const byte *)SV_GetClientState( clientIndex );

			if ( clientIndex == cachedNum ) {
				MSG_WriteDeltaClient( &msg, cc->cs, liveClient, qtrue );
				if ( VM_Call( vm, GAME_GET_CLIENT_INFO, clientIndex, &ps ) ) {
					MSG_WriteBit1( &msg );
					MSG_WriteDeltaPlayerstate( &msg, (const byte *)&cc->ps,
											   (const byte *)&ps, 0 );
				} else {
					MSG_WriteBit0( &msg );
				}
				cachedIndex++;
			} else {
				MSG_WriteDeltaClient( &msg, NULL, liveClient, qtrue );
				if ( VM_Call( vm, GAME_GET_CLIENT_INFO, clientIndex, &ps ) ) {
					MSG_WriteBit1( &msg );
					MSG_WriteDeltaPlayerstate( &msg, NULL, (const byte *)&ps, 0 );
				} else {
					MSG_WriteBit0( &msg );
				}
			}
			clientIndex++;
		}

		MSG_WriteBit0( &msg );

		for ( i = 0 ; i < sv.num_entities ; i++ ) {
			ent = SV_GentityNum( i );
			if ( !SV_ArchiveEntityVisible( ent ) ) {
				continue;
			}
			SV_FillArchivedEntity( &localEnt, ent );
			MSG_WriteDeltaArchivedEntity( &msg,
				(const byte *)&SV_SvEntityNum( ent->s.number )->baseline,
				(const byte *)&localEnt, qtrue );
		}
	} else {
		MSG_WriteBit1( &msg );
		MSG_WriteData( &msg, 4, &svs.time );

		cache = &SVS_CACHED_FRAMES[ svs.nextCachedSnapshotFrames
									% SV_NUM_CACHED_SNAPSHOT_FRAMES ];
		cache->archivedFrame = svs.nextArchivedSnapshotFrames;
		cache->num_entities  = 0;
		cache->first_entity  = svs.nextCachedSnapshotEntities;
		cache->num_clients   = 0;
		cache->first_client  = svs.nextCachedSnapshotClients;
		cache->usesDelta     = 0;
		cache->time          = svs.time;

		for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
			if ( svs.clients[i].state < CS_CONNECTED ) {
				continue;
			}

			cc = &SVS_CACHED_CLIENTS[ svs.nextCachedSnapshotClients
									  % SV_NUM_CACHED_SNAPSHOT_CLIENTS ];
			memcpy( cc->cs, SV_GetClientState( i ), sizeof( cc->cs ) );
			MSG_WriteDeltaClient( &msg, NULL, cc->cs, qtrue );

			cc->havePlayerState = VM_Call( vm, GAME_GET_CLIENT_INFO, i, &cc->ps );
			if ( cc->havePlayerState ) {
				MSG_WriteBit1( &msg );
				MSG_WriteDeltaPlayerstate( &msg, NULL, (const byte *)&cc->ps, 0 );
			} else {
				MSG_WriteBit0( &msg );
			}

			svs.nextCachedSnapshotClients++;
			if ( svs.nextCachedSnapshotClients >= SV_SNAPSHOT_COUNTER_LIMIT ) {
				Com_Error( ERR_FATAL, "\x15svs.nextCachedSnapshotClients wrapped" );
			}
			cache->num_clients++;
		}

		MSG_WriteBit0( &msg );

		for ( i = 0 ; i < sv.num_entities ; i++ ) {
			ent = SV_GentityNum( i );
			if ( !SV_ArchiveEntityVisible( ent ) ) {
				continue;
			}

			ae = &SVS_CACHED_ENTITIES[ svs.nextCachedSnapshotEntities
									   % SV_NUM_CACHED_SNAPSHOT_ENTITIES ];
			SV_FillArchivedEntity( ae, ent );
			MSG_WriteDeltaArchivedEntity( &msg,
				(const byte *)&SV_SvEntityNum( ent->s.number )->baseline,
				(const byte *)ae, qtrue );

			svs.nextCachedSnapshotEntities++;
			if ( svs.nextCachedSnapshotEntities >= SV_SNAPSHOT_COUNTER_LIMIT ) {
				Com_Error( ERR_FATAL, "\x15svs.nextCachedSnapshotEntities wrapped" );
			}
			cache->num_entities++;
		}

		svs.nextCachedSnapshotFrames++;
		if ( svs.nextCachedSnapshotFrames >= SV_SNAPSHOT_COUNTER_LIMIT ) {
			Com_Error( ERR_FATAL, "\x15svs.nextCachedSnapshotFrames wrapped" );
		}
	}

	MSG_WriteBits( &msg, ENTITYNUM_NONE, 10 );
	len = msg.cursize;

	archived = &svs.archivedSnapshotFrames[ svs.nextArchivedSnapshotFrames
											% SV_NUM_ARCHIVED_SNAPSHOT_FRAMES ];
	archived->start = svs.nextArchivedSnapshotBuffer;
	archived->len = len;

	offset = svs.nextArchivedSnapshotBuffer % SV_ARCHIVED_SNAPSHOT_BUFFER_BYTES;

	svs.nextArchivedSnapshotBuffer += len;
	if ( svs.nextArchivedSnapshotBuffer >= SV_SNAPSHOT_COUNTER_LIMIT ) {
		Com_Error( ERR_FATAL, "\x15svs.nextArchivedSnapshotBuffer wrapped" );
	}

	room = SV_ARCHIVED_SNAPSHOT_BUFFER_BYTES - offset;
	if ( len > room ) {
		memcpy( svs.archivedSnapshotBuffer + offset, msg.data, (size_t)room );
		memcpy( svs.archivedSnapshotBuffer, msg.data + room, (size_t)( len - room ) );
	} else {
		memcpy( svs.archivedSnapshotBuffer + offset, msg.data, (size_t)len );
	}

	svs.nextArchivedSnapshotFrames++;
	if ( svs.nextArchivedSnapshotFrames >= SV_SNAPSHOT_COUNTER_LIMIT ) {
		Com_Error( ERR_FATAL, "\x15svs.nextArchivedSnapshotFrames wrapped" );
	}

	if ( !sv_archiveSelfTest ) {
		sv_archiveSelfTest = Cvar_Get( "sv_archiveSelfTest", "0", 0 );
		sv_archiveForceTime = Cvar_Get( "sv_archiveForceTime", "0", 0 );
	}
	if ( sv_archiveSelfTest->integer ) {
		SV_ArchiveSelfTest( svs.nextArchivedSnapshotFrames - 1 );
	}
}

/* ---- SV_SendClientMessages  0x0045E860 ---- */
void SV_SendClientMessages( void ) {
	int i;
	client_t    *cl;
	int numclients;

	sv.bpsTotalBytes = 0;
	sv.ubpsTotalBytes = 0;

	numclients = 0;

	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		cl = &svs.clients[i];

		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( svs.time < cl->nextSnapshotTime ) {
			continue;
		}

		numclients++;

		if ( cl->netchan.unsentFragments ) {
			cl->nextSnapshotTime = svs.time
								   + SV_RateMsec( cl,
												  cl->netchan.unsentLength
												  - cl->netchan.unsentFragmentStart );
			Netchan_TransmitNextFragment( &cl->netchan );
			continue;
		}

		SV_SendClientSnapshot( cl );
	}

	if ( !sv_showAverageBPS->integer || numclients <= 0 ) {
		return;
	}

	SV_UpdateAverageBPS( numclients );
}

/* ---- SV_UpdateAverageBPS  no-address ---- */
static void SV_UpdateAverageBPS( int numclients ) {
	int i;
	float total;
	float utotal;
	float ave, uave, comp;

	total = 0.0f;
	utotal = 0.0f;

	for ( i = 0 ; i < MAX_BPS_WINDOW - 1 ; i++ ) {
		sv.bpsWindow[i] = sv.bpsWindow[i + 1];
		total += (float)sv.bpsWindow[i];
		sv.ubpsWindow[i] = sv.ubpsWindow[i + 1];
		utotal += (float)sv.ubpsWindow[i];
	}

	sv.bpsWindow[MAX_BPS_WINDOW - 1] = sv.bpsTotalBytes;
	total += (float)sv.bpsTotalBytes;
	sv.ubpsWindow[MAX_BPS_WINDOW - 1] = sv.ubpsTotalBytes;
	utotal += (float)sv.ubpsTotalBytes;

	if ( sv.bpsTotalBytes >= sv.bpsMaxBytes ) {
		sv.bpsMaxBytes = sv.bpsTotalBytes;
	}
	if ( sv.ubpsTotalBytes >= sv.ubpsMaxBytes ) {
		sv.ubpsMaxBytes = sv.ubpsTotalBytes;
	}

	sv.bpsWindowSteps++;
	if ( sv.bpsWindowSteps < MAX_BPS_WINDOW ) {
		return;
	}
	sv.bpsWindowSteps = 0;

	ave = total * ( 1.0f / MAX_BPS_WINDOW );
	uave = utotal * ( 1.0f / MAX_BPS_WINDOW );
	comp = (float)( ( 1.0 - (double)ave / (double)uave ) * 100.0 );

	sv.ucompNum++;
	sv.ucompAve += comp;

	Com_DPrintf( "bpspc(%2.0f) bps(%2.0f) pk(%i) ubps(%2.0f) upk(%i) cr(%2.2f) acr(%2.2f)\n",
				 (double)ave / (double)numclients, (double)ave,
				 sv.bpsMaxBytes, (double)uave, sv.ubpsMaxBytes,
				 (double)comp, (double)sv.ucompAve / (double)sv.ucompNum );
}
