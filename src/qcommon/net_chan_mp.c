/*
 * qcommon/net_chan_mp.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/net_chan_mp.c
 *
 * Retail range 0x004483F0-0x00449690, 22 functions.
 *
 * @fidelity: verified
 */

#include "qcommon.h"

void CL_Netchan_PrintProfileStats( qboolean bPrintToConsole );
void SV_Netchan_PrintProfileStats( qboolean bPrintToConsole );
void Sys_SendPacket( const void *data, int length, netadr_t to );
qboolean Sys_StringToAdr( const char *s, netadr_t *a );

const char *NET_AdrToString( netadr_t a );
void NET_SendPacket( netsrc_t sock, int length, const void *data, netadr_t to );
void NET_SendLoopPacket( netsrc_t sock, int length, const void *data );

cvar_t  *showpackets;
cvar_t  *showdrop;
cvar_t  *net_qport;
cvar_t  *net_profile;
cvar_t  *net_showprofile;
cvar_t  *net_lanauthorize;

int net_iProfilingOn;

/* 0x0057A1B4 */
const char *netsrcString[2] = { "client", "server" };

enum
{
	MAX_LOOPBACK      = 0x10,
	MAX_LOOPBACK_MASK = 0xF,
};
typedef struct loopmsg_t
{
	byte data[1400];
	int datalen;
} loopmsg_t;
COD1_ASSERT_SIZE( loopmsg_t, 1404 );

typedef struct loopback_t
{
	loopmsg_t msgs[MAX_LOOPBACK];
	int get, send;
} loopback_t;
COD1_ASSERT_SIZE( loopback_t, 22472 );

/* 0x016465C0.  loopbacks[NS_CLIENT] is what the server writes into. */
loopback_t loopbacks[2];

/* 0x008CC888.  NET_AdrToString's single static result buffer. */
char net_string[64];

netProfileInfo_t *pProf;

extern byte svs[];
#define SV_OOB_PROF_OFFSET  45184
#define SV_OOB_PROF()       ( (netProfileInfo_t **)&svs[SV_OOB_PROF_OFFSET] )

#define NET_PROFILE_PACKETS         60
#define NET_PROFILE_WINDOW_MS     1000
#define NET_PROFILE_RECALC_MS      100
#define NET_PROFILE_EMPTY_MIN     9999
#define NET_SHOWPROFILE_PACKETS   0x02

/* NetProf_PrepProfiling  0x004483F0  VERIFIED */
void NetProf_PrepProfiling( netProfileInfo_t **pProfile ) {
	int mode;

	if ( !net_profile->integer ) {
		if ( net_iProfilingOn ) {
			net_iProfilingOn = 0;
			Com_Printf( "Net Profiling turned off\n" );
		}
		if ( *pProfile ) {
			free( *pProfile );
			*pProfile = NULL;
		}
		return;
	}

	if ( !net_iProfilingOn ) {
		if ( !com_sv_running->integer
			 || ( com_cl_running->integer && net_profile->integer == 2 ) ) {
			mode = 1;
		} else {
			mode = 2;
		}
		net_iProfilingOn = mode;
		Com_Printf( "Net Profiling turned on: %s\n", netsrcString[mode - 1] );
	}

	if ( !*pProfile ) {
		*pProfile = (netProfileInfo_t *)malloc( sizeof( netProfileInfo_t ) );
		memset( *pProfile, 0, sizeof( netProfileInfo_t ) );
	}
}

/* NetProf_AddPacket  0x004484B0  VERIFIED */
void NetProf_AddPacket( netProfileStream_t *pStream, int iSize, int bFragment ) {
	netProfilePacket_t *p;

	pStream->iCurrPacket = ( pStream->iCurrPacket + 1 ) % NET_PROFILE_PACKETS;
	p = &pStream->packets[pStream->iCurrPacket];
	p->iTime = Sys_Milliseconds();
	p->iSize = iSize;
	p->bFragment = bFragment;
}

/* NetProf_NewSendPacket  0x00448510  VERIFIED */
void NetProf_NewSendPacket( netchan_t *chan, int iSize, int bFragment ) {
	if ( !net_iProfilingOn ) {
		return;
	}
	NetProf_AddPacket( &chan->pProf->send, iSize, bFragment );
	if ( net_showprofile->integer & NET_SHOWPROFILE_PACKETS ) {
		Com_Printf( "%s send%s: %i\n", netsrcString[chan->sock],
					bFragment ? " fragment" : "", iSize );
	}
}

/* NetProf_NewRecievePacket  0x00448560  VERIFIED */
void NetProf_NewRecievePacket( netchan_t *chan, int iSize, int bFragment ) {
	if ( !net_iProfilingOn ) {
		return;
	}
	NetProf_AddPacket( &chan->pProf->receive, iSize, bFragment );
	if ( net_showprofile->integer & NET_SHOWPROFILE_PACKETS ) {
		Com_Printf( "%s recieve%s: %i\n", netsrcString[chan->sock],
					bFragment ? " fragment" : "", iSize );
	}
}

/* NetProf_UpdateStatistics  0x004485C0  VERIFIED */
void NetProf_UpdateStatistics( netProfileStream_t *pStream ) {
	int i, now;
	int iNumPackets, iNumFragments, iCountedPackets;
	int iTotalBytes, iSmallestSize, iLargestSize;
	int iOldestPacket, iOldestPacketTime, iTimeSpan;

	iNumPackets = 0;
	iNumFragments = 0;
	iOldestPacket = -1;
	iOldestPacketTime = Sys_Milliseconds();
	iTotalBytes = 0;
	iSmallestSize = NET_PROFILE_EMPTY_MIN;
	iLargestSize = 0;

	for ( i = 0; i < NET_PROFILE_PACKETS; i++ ) {
		if ( !pStream->packets[i].iTime ) {
			continue;
		}
		now = Sys_Milliseconds();
		if ( now > pStream->packets[i].iTime + NET_PROFILE_WINDOW_MS ) {
			continue;
		}
		iNumPackets++;
		if ( pStream->packets[i].bFragment ) {
			iNumFragments++;
		}
		if ( pStream->packets[i].iTime < iOldestPacketTime ) {
			iOldestPacket = i;
			iOldestPacketTime = pStream->packets[i].iTime;
		}
		iTotalBytes += pStream->packets[i].iSize;
		if ( pStream->packets[i].iSize < iSmallestSize ) {
			iSmallestSize = pStream->packets[i].iSize;
		}
		if ( pStream->packets[i].iSize > iLargestSize ) {
			iLargestSize = pStream->packets[i].iSize;
		}
	}

	if ( !iNumPackets ) {
		pStream->iBytesPerSecond = 0;
		pStream->iLastBPSCalcTime = 0;
		pStream->iCountedPackets = 0;
		pStream->iCountedFragments = 0;
		pStream->iFragmentPercentage = 0;
		pStream->iLargestPacket = 0;
		pStream->iSmallestPacket = 0;
		return;
	}

	iCountedPackets = iNumPackets;
	pStream->iFragmentPercentage = iNumFragments ? 100 * iNumFragments / iNumPackets : 0;
	pStream->iLargestPacket = iLargestSize;
	pStream->iSmallestPacket = iSmallestSize;

	if ( pStream->iLastBPSCalcTime + NET_PROFILE_RECALC_MS < Sys_Milliseconds() ) {
		iTimeSpan = Sys_Milliseconds() - iOldestPacketTime;
		if ( iOldestPacket != -1 ) {
			iCountedPackets = iNumPackets - 1;
			iTotalBytes -= pStream->packets[iOldestPacket].iSize;
			if ( pStream->packets[iOldestPacket].bFragment ) {
				iNumFragments--;
			}
		}
		if ( iTimeSpan >= 1 && iCountedPackets ) {
			pStream->iBytesPerSecond = iTotalBytes
				? (int)( (double)iTotalBytes / ( (double)iTimeSpan * 0.001 ) )
				: 0;
			pStream->iLastBPSCalcTime = Sys_Milliseconds();
		} else {
			pStream->iBytesPerSecond = 0;
		}
	}

	pStream->iCountedPackets = iCountedPackets;
	pStream->iCountedFragments = iNumFragments;
}

/* Net_DumpProfile_f  0x00448820  VERIFIED */
void Net_DumpProfile_f( void ) {
	if ( !net_iProfilingOn ) {
		Com_Printf( "Network profiling is not on. Set net_profile to turn on network profiling\n" );
		return;
	}
	if ( net_iProfilingOn == 1 ) {
		CL_Netchan_PrintProfileStats( qtrue );
	} else {
		SV_Netchan_PrintProfileStats( qtrue );
	}
}

/* Net_DisplayProfile  0x00448850  VERIFIED */
void Net_DisplayProfile( void ) {
	if ( !net_iProfilingOn ) {
		return;
	}
	if ( net_iProfilingOn == 1 ) {
		CL_Netchan_PrintProfileStats( qfalse );
	} else {
		SV_Netchan_PrintProfileStats( qfalse );
	}
}

/* Netchan_Init  0x00448870  VERIFIED */
void Netchan_Init( int qport ) {
	qport &= 0xFFFF;

	showpackets = Cvar_Get( "showpackets", "0", CVAR_TEMP );
	showdrop = Cvar_Get( "showdrop", "0", CVAR_TEMP );
	net_qport = Cvar_Get( "net_qport", va( "%i", qport ), CVAR_INIT );
	net_profile = Cvar_Get( "net_profile", "0", CVAR_TEMP );
	net_showprofile = Cvar_Get( "net_showprofile", "0", CVAR_TEMP );
	net_lanauthorize = Cvar_Get( "net_lanauthorize", "0", 0 );

	Cmd_AddCommand( "net_dumpprofile", Net_DumpProfile_f );
}

/* Netchan_Setup  0x00448930  VERIFIED */
void Netchan_Setup( netchan_t *chan, netsrc_t sock, netadr_t adr, int qport ) {
	memset( chan, 0, sizeof( netchan_t ) );

	chan->sock = sock;
	chan->remoteAddress = adr;
	chan->qport = qport;
	chan->incomingSequence = 0;
	chan->outgoingSequence = 1;

	NetProf_PrepProfiling( &chan->pProf );
}

/* Netchan_TransmitNextFragment  0x00448980  VERIFIED */
void Netchan_TransmitNextFragment( netchan_t *chan ) {
	msg_t send;
	byte send_buf[MAX_PACKETLEN];
	int fragmentLength;

	NetProf_PrepProfiling( &chan->pProf );

	MSG_Init( &send, send_buf, sizeof( send_buf ) );
	MSG_WriteLong( &send, chan->outgoingSequence | FRAGMENT_BIT );

	if ( chan->sock == NS_CLIENT ) {
		MSG_WriteShort( &send, net_qport->integer );
	}

	fragmentLength = FRAGMENT_SIZE;
	if ( chan->unsentFragmentStart + FRAGMENT_SIZE > chan->unsentLength ) {
		fragmentLength = chan->unsentLength - chan->unsentFragmentStart;
	}

	if ( chan->extended ) {
		MSG_WriteLong( &send, chan->unsentFragmentStart );
	} else {
		MSG_WriteShort( &send, chan->unsentFragmentStart );
	}
	MSG_WriteShort( &send, fragmentLength );
	MSG_WriteData( &send, fragmentLength, &chan->unsentBuffer[chan->unsentFragmentStart] );

	NET_SendPacket( chan->sock, send.cursize, send.data, chan->remoteAddress );
	NetProf_NewSendPacket( chan, send.cursize, qtrue );

	if ( showpackets->integer ) {
		Com_Printf( "%s send %4i : s=%i fragment=%i,%i\n",
					netsrcString[chan->sock], send.cursize,
					chan->outgoingSequence - 1, chan->unsentFragmentStart,
					fragmentLength );
	}

	chan->unsentFragmentStart += fragmentLength;
	if ( chan->unsentFragmentStart == chan->unsentLength
		 && fragmentLength != FRAGMENT_SIZE ) {
		chan->outgoingSequence++;
		chan->unsentFragments = qfalse;
	}
}

/* Netchan_Transmit  0x00448B70  VERIFIED */
void Netchan_Transmit( netchan_t *chan, int length, const byte *data ) {
	msg_t send;
	byte send_buf[MAX_PACKETLEN];

	if ( length < 0 || length > ( chan->extended ? MAX_MSGLEN : STOCK_MAX_MSGLEN ) ) {
		Com_Error( ERR_DROP, "\x15" "Netchan_Transmit: length = %i", length );
	}

	chan->unsentFragmentStart = 0;

	if ( length >= FRAGMENT_SIZE ) {
		chan->unsentFragments = qtrue;
		chan->unsentLength = length;
		Com_Memcpy( chan->unsentBuffer, data, (size_t)length );
		Netchan_TransmitNextFragment( chan );
		return;
	}

	NetProf_PrepProfiling( &chan->pProf );

	MSG_Init( &send, send_buf, sizeof( send_buf ) );
	MSG_WriteLong( &send, chan->outgoingSequence );
	chan->outgoingSequence++;

	if ( chan->sock == NS_CLIENT ) {
		MSG_WriteShort( &send, net_qport->integer );
	}

	MSG_WriteData( &send, length, data );

	NET_SendPacket( chan->sock, send.cursize, send.data, chan->remoteAddress );
	NetProf_NewSendPacket( chan, send.cursize, qfalse );

	if ( showpackets->integer ) {
		Com_Printf( "%s send %4i : s=%i ack=%i\n",
					netsrcString[chan->sock], send.cursize,
					chan->outgoingSequence - 1, chan->incomingSequence );
	}
}

/* Netchan_Process  0x00448D40  VERIFIED */
qboolean Netchan_Process( netchan_t *chan, msg_t *msg ) {
	int sequence;
	int fragmentStart, fragmentLength;
	qboolean fragmented;

	NetProf_PrepProfiling( &chan->pProf );

	MSG_BeginReading( msg );
	sequence = MSG_ReadLong( msg );

	fragmented = qfalse;
	if ( sequence & FRAGMENT_BIT ) {
		sequence &= ~FRAGMENT_BIT;
		fragmented = qtrue;
	}

	if ( chan->sock == NS_SERVER ) {
		MSG_ReadShort( msg );
	}

	if ( fragmented ) {
		fragmentStart = chan->extended ? MSG_ReadLong( msg ) : MSG_ReadShort( msg );
		fragmentLength = MSG_ReadShort( msg );
	} else {
		fragmentStart = 0;
		fragmentLength = 0;
	}

	NetProf_NewRecievePacket( chan, msg->cursize, fragmented );

	if ( showpackets->integer ) {
		if ( fragmented ) {
			Com_Printf( "%s recv %4i : s=%i fragment=%i,%i\n",
						netsrcString[chan->sock], msg->cursize, sequence,
						fragmentStart, fragmentLength );
		} else {
			Com_Printf( "%s recv %4i : s=%i\n",
						netsrcString[chan->sock], msg->cursize, sequence );
		}
	}

	if ( sequence <= chan->incomingSequence ) {
		if ( showdrop->integer || showpackets->integer ) {
			Com_Printf( "%s:Out of order packet %i at %i\n",
						NET_AdrToString( chan->remoteAddress ), sequence,
						chan->incomingSequence );
		}
		return qfalse;
	}

	chan->dropped = sequence - chan->incomingSequence - 1;
	if ( chan->dropped > 0 && ( showdrop->integer || showpackets->integer ) ) {
		Com_Printf( "%s:Dropped %i packets at %i\n",
					NET_AdrToString( chan->remoteAddress ), chan->dropped,
					sequence );
	}

	if ( !fragmented ) {
		chan->incomingSequence = sequence;
		return qtrue;
	}

	if ( sequence != chan->fragmentSequence ) {
		chan->fragmentSequence = sequence;
		chan->fragmentLength = 0;
	}

	if ( fragmentStart != chan->fragmentLength ) {
		if ( showdrop->integer || showpackets->integer ) {
			Com_Printf( "%s:Dropped a message fragment\n",
						NET_AdrToString( chan->remoteAddress ) );
		}
		return qfalse;
	}

	if ( fragmentStart < 0 || fragmentLength < 0
		 || msg->readcount + fragmentLength > msg->cursize
		 || (unsigned int)( chan->fragmentLength + fragmentLength ) > ( chan->extended ? MAX_MSGLEN : STOCK_MAX_MSGLEN ) ) {
		if ( showdrop->integer || showpackets->integer ) {
			Com_Printf( "%s:illegal fragment length\n",
						NET_AdrToString( chan->remoteAddress ) );
		}
		return qfalse;
	}

	memcpy( &chan->fragmentBuffer[chan->fragmentLength],
			&msg->data[msg->readcount], (size_t)fragmentLength );
	chan->fragmentLength += fragmentLength;

	if ( fragmentLength == FRAGMENT_SIZE ) {
		return qfalse;
	}

	if ( msg->maxsize < 4 || chan->fragmentLength > msg->maxsize - 4 ) {
		Com_Printf( "%s:fragmentLength %i > msg->maxsize\n",
					NET_AdrToString( chan->remoteAddress ), chan->fragmentLength );
		return qfalse;
	}

	*(int *)msg->data = sequence;
	memcpy( msg->data + 4, chan->fragmentBuffer, (size_t)chan->fragmentLength );
	msg->cursize = chan->fragmentLength + 4;
	chan->fragmentLength = 0;

	MSG_BeginReading( msg );
	MSG_ReadLong( msg );

	return qtrue;
}

/* NET_CompareBaseAdr  0x004490C0  VERIFIED */
qboolean NET_CompareBaseAdr( netadr_t a, netadr_t b ) {
	if ( a.type != b.type ) {
		return qfalse;
	}

	if ( a.type == NA_LOOPBACK ) {
		return qtrue;
	}
	if ( a.type == NA_BOT ) {
		return (qboolean)( a.port == b.port );
	}
	if ( a.type == NA_IP ) {
		return (qboolean)( a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1]
						   && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] );
	}
	if ( a.type == NA_IPX ) {
		return (qboolean)( memcmp( a.ipx, b.ipx, 10 ) == 0 );
	}

	Com_Printf( "NET_CompareBaseAdr: bad address type\n" );
	return qfalse;
}

/* NET_AdrToString  0x00449150  VERIFIED */
const char *NET_AdrToString( netadr_t a ) {
	if ( a.type == NA_LOOPBACK ) {
		Com_sprintf( net_string, sizeof( net_string ), "loopback" );
	} else if ( a.type == NA_IP ) {
		Com_sprintf( net_string, sizeof( net_string ), "%i.%i.%i.%i:%i",
					 a.ip[0], a.ip[1], a.ip[2], a.ip[3],
					 BigShort( (short)a.port ) );
	} else {
		Com_sprintf( net_string, sizeof( net_string ),
					 "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%i",
					 a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4],
					 a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9],
					 BigShort( (short)a.port ) );
	}
	return net_string;
}

/* NET_CompareAdr  0x00449230  VERIFIED */
qboolean NET_CompareAdr( netadr_t a, netadr_t b ) {
	if ( a.type != b.type ) {
		return qfalse;
	}

	if ( a.type == NA_LOOPBACK ) {
		return qtrue;
	}
	if ( a.type == NA_IP ) {
		return (qboolean)( a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1]
						   && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3]
						   && a.port == b.port );
	}
	if ( a.type == NA_IPX ) {
		return (qboolean)( memcmp( a.ipx, b.ipx, 10 ) == 0 && a.port == b.port );
	}

	Com_Printf( "NET_CompareAdr: bad address type\n" );
	return qfalse;
}

/* CoD 1.5: NET_CompareAdrSigned (Call of Duty MP.c). */
int NET_CompareAdrSigned( const netadr_t *a, const netadr_t *b ) {
	if ( a->type != b->type ) {
		return (int)a->type - (int)b->type;
	}
	switch ( a->type ) {
	case NA_LOOPBACK:
		return 0;
	case NA_IP:
		return a->port != b->port ? (int)a->port - (int)b->port : memcmp( a->ip, b->ip, 4 );
	case NA_IPX:
		return a->port != b->port ? (int)a->port - (int)b->port : memcmp( a->ipx, b->ipx, 10 );
	default:
		Com_Printf( "NET_CompareAdrSigned: bad address type\n" );
		return 0;
	}
}

/* NET_IsLocalAddress  0x004492D0  VERIFIED */
qboolean NET_IsLocalAddress( netadr_t adr ) {
	return (qboolean)( adr.type == NA_LOOPBACK || adr.type == NA_BOT );
}

/* NET_GetLoopPacket  0x004492F0  VERIFIED */
qboolean NET_GetLoopPacket( netsrc_t sock, netadr_t *net_from, msg_t *net_message ) {
	loopback_t *loop;
	loopmsg_t *packet;
	int i;

	loop = &loopbacks[sock];

	if ( loop->send - loop->get > MAX_LOOPBACK ) {
		loop->get = loop->send - MAX_LOOPBACK;
	}
	if ( loop->get >= loop->send ) {
		return qfalse;
	}

	i = loop->get & MAX_LOOPBACK_MASK;
	loop->get++;

	packet = &loop->msgs[i];
	memcpy( net_message->data, packet->data, (size_t)(unsigned int)packet->datalen );
	net_message->cursize = packet->datalen;

	memset( net_from, 0, sizeof( *net_from ) );
	net_from->type = NA_LOOPBACK;
	return qtrue;
}

/* NET_SendLoopPacket  0x00449390  VERIFIED */
void NET_SendLoopPacket( netsrc_t sock, int length, const void *data ) {
	loopback_t *loop;
	int i;

	loop = &loopbacks[sock ^ 1];
	i = loop->send & MAX_LOOPBACK_MASK;
	loop->send++;

	memcpy( loop->msgs[i].data, data, (size_t)(unsigned int)length );
	loop->msgs[i].datalen = length;
}

/* NET_SendPacket  0x004493E0  VERIFIED */
void NET_SendPacket( netsrc_t sock, int length, const void *data, netadr_t to ) {
	if ( showpackets->integer && *(const int *)data == -1 ) {
		Com_Printf( "send packet %4i\n", length );
	}

	if ( to.type == NA_LOOPBACK ) {
		NET_SendLoopPacket( sock, length, data );
		return;
	}

	if ( to.type != NA_BOT && to.type != NA_BAD ) {
		Sys_SendPacket( data, length, to );
	}
}

static void NET_ProfileOOBPacket( netsrc_t sock, int length ) {
	netProfileInfo_t **block;

	if ( !net_profile->integer ) {
		return;
	}
	block = ( sock == NS_SERVER ) ? SV_OOB_PROF() : &pProf;
	NetProf_PrepProfiling( block );
	if ( *block ) {
		NetProf_AddPacket( &( *block )->send, length, qfalse );
	}
}

/* NET_OutOfBandPrint  0x00449490  VERIFIED */
void NET_OutOfBandPrint( netsrc_t sock, netadr_t adr, const char *format, ... ) {
	byte string[MAX_MSGLEN + 4];    /* retail: 0x4004 */
	va_list argptr;
	int length;

	string[0] = 0xFF;
	string[1] = 0xFF;
	string[2] = 0xFF;
	string[3] = 0xFF;

	va_start( argptr, format );
	vsprintf( (char *)string + 4, format, argptr );
	va_end( argptr );

	length = strlen( (char *)string );
	NET_SendPacket( sock, length, string, adr );
	NET_ProfileOOBPacket( sock, length );
}

/* NET_OutOfBandData  0x00449590  VERIFIED */
void NET_OutOfBandData( netsrc_t sock, netadr_t adr, int len, const void *data ) {
	byte string[MAX_MSGLEN * 2 + 4];
	msg_t mbuf;
	int i;

	string[0] = 0xFF;
	string[1] = 0xFF;
	string[2] = 0xFF;
	string[3] = 0xFF;

	for ( i = 0; i < len; i++ ) {
		string[i + 4] = ( (const byte *)data )[i];
	}

	mbuf.data = string;
	mbuf.cursize = len + 4;
	Huff_Compress( &mbuf, 12 );

	NET_SendPacket( sock, mbuf.cursize, mbuf.data, adr );
	NET_ProfileOOBPacket( sock, mbuf.cursize );
}

/* NET_StringToAdr  0x00449690  VERIFIED */
qboolean NET_StringToAdr( const char *s, netadr_t *a ) {
	char base[MAX_STRING_CHARS];
	char *port;

	if ( !strcmp( s, "localhost" ) ) {
		memset( a, 0, sizeof( *a ) );
		a->type = NA_LOOPBACK;
		return qtrue;
	}

	strncpy( base, s, sizeof( base ) - 1 );
	base[sizeof( base ) - 1] = 0;

	port = strstr( base, ":" );
	if ( port ) {
		*port = 0;
		port++;
	}

	if ( !Sys_StringToAdr( base, a )
		 || ( a->ip[0] == 0xFF && a->ip[1] == 0xFF
			  && a->ip[2] == 0xFF && a->ip[3] == 0xFF ) ) {
		a->type = NA_BAD;
		return qfalse;
	}

	if ( port ) {
		a->port = (unsigned short)BigShort( (short)atoi( port ) );
	} else {
		a->port = (unsigned short)BigShort( (short)PORT_SERVER );
	}

	return qtrue;
}
