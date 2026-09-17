/*
 * @fidelity: verified
 */

#define NetProf_AddPacket   netprof_addpacket_stale_decl_unused
#include "server.h"
#undef NetProf_AddPacket

#include <string.h>

/* The client VM, retail 0x01617348. Only used for the on-screen profile draw, which a dedicated server never reaches. */
extern vm_t *cgvm;

extern void Netchan_Transmit( netchan_t *chan, int length, const byte *data );
extern void Netchan_TransmitNextFragment( netchan_t *chan );
extern void NET_SendPacket( netsrc_t sock, int length, const void *data,
							netadr_t to );
extern void NetProf_AddPacket( netProfileStream_t *pStream, int iSize,
							   int bFragment );
extern void NetProf_UpdateStatistics( netProfileStream_t *pStream );

#define CG_PROFILE_DRAW_TRAP        15

/* ---- SV_Netchan_Encode  0x0045B5A0 ---- */
void SV_Netchan_Encode( byte *data, client_t *client, int length ) {
	const char  *string;
	int i;
	int index;
	byte key;

	key = (byte)client->challenge ^ (byte)client->netchan.outgoingSequence;
	string = client->lastClientCommandString;

	for ( i = 0, index = 0 ; i < length ; i++, data++ ) {
		if ( !string[index] ) {
			index = 0;
		}
		key ^= (byte)( string[index] << ( i & 1 ) );
		index++;
		*data ^= key;
	}
}

/* ---- SV_Netchan_Decode  0x0045B600 ---- */
void SV_Netchan_Decode( client_t *client, byte *data, int length ) {
	const char  *string;
	int i;
	int index;
	byte key;

	string = client->reliableCommands[client->reliableAcknowledge
									  & ( MAX_RELIABLE_COMMANDS - 1 )].command;

	key = (byte)client->serverId
		  ^ (byte)client->challenge
		  ^ (byte)client->messageAcknowledge;

	for ( i = 0, index = 0 ; i < length ; i++, data++ ) {
		if ( !string[index] ) {
			index = 0;
		}
		key ^= (byte)( string[index] << ( i & 1 ) );
		index++;
		*data ^= key;
	}
}

/* ---- SV_Netchan_TransmitNextFragment  0x0045B670 ---- */
void SV_Netchan_TransmitNextFragment( netchan_t *chan ) {
	Netchan_TransmitNextFragment( chan );
}

/* ---- SV_Netchan_Transmit  0x0045B680 ---- */
void SV_Netchan_Transmit( client_t *client, byte *data, int length ) {
	SV_Netchan_Encode( data + 4, client, length - 4 );
	Netchan_Transmit( &client->netchan, length, data );
}

/* ---- SV_Netchan_AddOOBProfilePacket  0x0045B6B0 ---- */
void SV_Netchan_AddOOBProfilePacket( int iSize ) {
	if ( !net_profile->integer ) {
		return;
	}

	NetProf_PrepProfiling( &svs.pOOBProf );
	NetProf_AddPacket( (netProfileStream_t *)svs.pOOBProf, iSize, 0 );
}

/* ---- SV_Netchan_SendOOBPacket  0x0045B6E0 ---- */
void SV_Netchan_SendOOBPacket( int length, const void *data, netadr_t to ) {
	if ( *(const int *)data != -1 ) {
		Com_Printf( "SV_Netchan_SendOOBPacket used to send non-OOB packet.\n" );
	}

	NetProf_PrepProfiling( &svs.pOOBProf );
	NET_SendPacket( NS_SERVER, length, data, to );

	if ( net_profile->integer ) {
		NetProf_PrepProfiling( &svs.pOOBProf );
		NetProf_AddPacket( (netProfileStream_t *)svs.pOOBProf, length, 0 );
	}
}

/* ---- SV_Netchan_UpdateProfileStats  0x0045B760 ---- */
void SV_Netchan_UpdateProfileStats( void ) {
	client_t    *cl;
	int i;

	if ( !svs.clients ) {
		return;
	}

	if ( svs.pOOBProf ) {
		NetProf_UpdateStatistics( &( (netProfileInfo_t *)svs.pOOBProf )->send );
		NetProf_UpdateStatistics( &( (netProfileInfo_t *)svs.pOOBProf )->receive );
	}

	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( !cl->state ) {
			continue;
		}
		if ( !cl->netchan.pProf ) {
			continue;
		}
		NetProf_UpdateStatistics( &cl->netchan.pProf->send );
		NetProf_UpdateStatistics( &cl->netchan.pProf->receive );
	}
}

/* ---- SV_ProfDraw  0x0045B7E0 ---- */
void SV_ProfDraw( int y, const char *text ) {
	if ( !cgvm ) {
		return;
	}
	VM_Call( cgvm, CG_PROFILE_DRAW_TRAP, 32, y, text, 0, 6, 8, 0 );
}

/* ---- SV_Netchan_PrintProfileStats  0x0045B800 ---- */
void SV_Netchan_PrintProfileStats( qboolean bPrintToConsole ) {
	netProfileInfo_t    *oob;
	netProfileInfo_t    *prof;
	client_t            *cl;
	int i;
	int y;
	int sendBps, sendPackets, sendFragments;
	int recvBps, recvPackets, recvFragments;
	int sendLargest, sendSmallest;
	int recvLargest, recvSmallest;
	int totalLargest, totalSmallest, totalFragPct;
	int sendFragPct, recvFragPct;
	char name[32];
	char line[1024];

	if ( !svs.clients ) {
		return;
	}

	SV_Netchan_UpdateProfileStats();

	sendBps = 0;
	sendPackets = 0;
	sendFragments = 0;
	recvBps = 0;
	recvPackets = 0;
	recvFragments = 0;
	sendLargest = 0;
	sendSmallest = 9999;
	recvLargest = 0;
	recvSmallest = 9999;
	y = 80;

	if ( bPrintToConsole ) {
		Com_Printf( "\n\n" );
	} else {
		y = 90;
	}

	Com_sprintf( line, sizeof( line ), "====================" );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		SV_ProfDraw( y, line );
		y += 10;
	}

	Com_sprintf( line, sizeof( line ), "Server Network Profile:" );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n\n", line );
	} else {
		SV_ProfDraw( y, line );
		y += 10;
	}

	Com_sprintf( line, sizeof( line ),
				 "                    | Sent To                "
				 "| Recieved From          | Total Source Traffic   |" );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		SV_ProfDraw( y, line );
		y += 10;
	}

	Com_sprintf( line, sizeof( line ),
				 "              Source|   bps|  max|  min|frag%%"
				 "|   bps|  max|  min|frag%%|   bps|  max|  min|frag%%|" );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		SV_ProfDraw( y, line );
		y += 10;
	}

	oob = (netProfileInfo_t *)svs.pOOBProf;
	if ( oob ) {
		sendBps = oob->send.iBytesPerSecond;
		sendPackets = oob->send.iCountedPackets;
		sendFragments = oob->send.iCountedFragments;
		recvBps = oob->receive.iBytesPerSecond;
		recvPackets = oob->receive.iCountedPackets;
		recvFragments = oob->receive.iCountedFragments;

		if ( oob->send.iLargestPacket > 0 ) {
			sendLargest = oob->send.iLargestPacket;
		}
		if ( oob->send.iSmallestPacket < 9999 ) {
			sendSmallest = oob->send.iSmallestPacket;
		}
		if ( oob->receive.iLargestPacket > 0 ) {
			recvLargest = oob->receive.iLargestPacket;
		}
		if ( oob->receive.iSmallestPacket < 9999 ) {
			recvSmallest = oob->receive.iSmallestPacket;
		}
	}

	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( !cl->state ) {
			continue;
		}
		prof = cl->netchan.pProf;
		if ( !prof ) {
			continue;
		}

		sendBps += prof->send.iBytesPerSecond;
		sendPackets += prof->send.iCountedPackets;
		sendFragments += prof->send.iCountedFragments;
		recvBps += prof->receive.iBytesPerSecond;
		recvPackets += prof->receive.iCountedPackets;
		recvFragments += prof->receive.iCountedFragments;

		if ( prof->send.iLargestPacket > sendLargest ) {
			sendLargest = prof->send.iLargestPacket;
		}
		if ( prof->send.iSmallestPacket < sendSmallest ) {
			sendSmallest = prof->send.iSmallestPacket;
		}
		if ( prof->receive.iLargestPacket > recvLargest ) {
			recvLargest = prof->receive.iLargestPacket;
		}
		if ( prof->receive.iSmallestPacket < recvSmallest ) {
			recvSmallest = prof->receive.iSmallestPacket;
		}
	}

	if ( recvPackets + sendPackets <= 0 || recvFragments + sendFragments <= 0 ) {
		totalFragPct = 0;
	} else {
		totalFragPct = 100 * ( recvFragments + sendFragments )
					   / ( recvPackets + sendPackets );
	}

	totalSmallest = ( sendSmallest < recvSmallest ) ? sendSmallest : recvSmallest;
	totalLargest = ( sendLargest > recvLargest ) ? sendLargest : recvLargest;

	recvFragPct = recvPackets ? ( 100 * recvFragments / recvPackets ) : 0;
	sendFragPct = sendPackets ? ( 100 * sendFragments / sendPackets ) : 0;

	Com_sprintf( line, sizeof( line ),
				 "              Totals:%6i|%5i|%5i| %3i%%"
				 "|%6i|%5i|%5i| %3i%%|%6i|%5i|%5i| %3i%%|",
				 sendBps, sendLargest, sendSmallest, sendFragPct,
				 recvBps, recvLargest, recvSmallest, recvFragPct,
				 recvBps + sendBps, totalLargest, totalSmallest, totalFragPct );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		SV_ProfDraw( y, line );
		y += 10;
	}

	if ( oob ) {
		int oobPackets = oob->receive.iCountedPackets + oob->send.iCountedPackets;
		int oobFragments = oob->receive.iCountedFragments
						   + oob->send.iCountedFragments;
		int oobFragPct;
		int oobSmallest;
		int oobLargest;

		if ( oobPackets <= 0 || oobFragments <= 0 ) {
			oobFragPct = 0;
		} else {
			oobFragPct = 100 * oobFragments / oobPackets;
		}

		oobSmallest = ( oob->send.iSmallestPacket < oob->receive.iSmallestPacket )
					  ? oob->send.iSmallestPacket : oob->receive.iSmallestPacket;
		oobLargest = ( oob->send.iLargestPacket > oob->receive.iLargestPacket )
					 ? oob->send.iLargestPacket : oob->receive.iLargestPacket;

		Com_sprintf( line, sizeof( line ),
					 "  OutOfBand Messages: %5i|%5i|%5i| %3i%%"
					 "| %5i|%5i|%5i| %3i%%| %5i|%5i|%5i| %3i%%|",
					 oob->send.iBytesPerSecond, oob->send.iLargestPacket,
					 oob->send.iSmallestPacket, oob->send.iFragmentPercentage,
					 oob->receive.iBytesPerSecond, oob->receive.iLargestPacket,
					 oob->receive.iSmallestPacket,
					 oob->receive.iFragmentPercentage,
					 oob->receive.iBytesPerSecond + oob->send.iBytesPerSecond,
					 oobLargest, oobSmallest, oobFragPct );
	} else {
		Com_sprintf( line, sizeof( line ),
					 "  OutOfBand Messages:     0|    0|    0|   - "
					 "|     0|    0|    0|   - |     0|    0|    0|   - |" );
	}

	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		SV_ProfDraw( y, line );
		y += 10;
	}

	for ( i = 0, cl = svs.clients ; i < sv_maxclients->integer ; i++, cl++ ) {
		if ( !cl->state ) {
			continue;
		}

		strncpy( name, cl->name, 17 );
		name[16] = 0;

		prof = cl->netchan.pProf;
		if ( prof ) {
			int packets = prof->receive.iCountedPackets
						  + prof->send.iCountedPackets;
			int fragments = prof->receive.iCountedFragments
							+ prof->send.iCountedFragments;
			int fragPct;
			int smallest;
			int largest;

			if ( packets <= 0 || fragments <= 0 ) {
				fragPct = 0;
			} else {
				fragPct = 100 * fragments / packets;
			}

			smallest = ( prof->send.iSmallestPacket
						 < prof->receive.iSmallestPacket )
					   ? prof->send.iSmallestPacket
					   : prof->receive.iSmallestPacket;
			largest = ( prof->send.iLargestPacket > prof->receive.iLargestPacket )
					  ? prof->send.iLargestPacket : prof->receive.iLargestPacket;

			Com_sprintf( line, sizeof( line ),
						 "#%2i-%16s: %5i|%5i|%5i| %3i%%"
						 "| %5i|%5i|%5i| %3i%%| %5i|%5i|%5i| %3i%%|",
						 i, name,
						 prof->send.iBytesPerSecond, prof->send.iLargestPacket,
						 prof->send.iSmallestPacket,
						 prof->send.iFragmentPercentage,
						 prof->receive.iBytesPerSecond,
						 prof->receive.iLargestPacket,
						 prof->receive.iSmallestPacket,
						 prof->receive.iFragmentPercentage,
						 prof->receive.iBytesPerSecond
						 + prof->send.iBytesPerSecond,
						 largest, smallest, fragPct );
		} else {
			Com_sprintf( line, sizeof( line ),
						 "#%2i-%16s:     0|    0|    0|   0%%"
						 "|     0|    0|    0|   0%%|     0|    0|    0|   0%%|",
						 i, name );
		}

		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			SV_ProfDraw( y, line );
			y += 10;
		}
	}
}
