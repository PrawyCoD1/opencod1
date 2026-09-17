/*
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "cl_vm.h"

extern void NET_SendPacket( netsrc_t sock, int length, const void *data,
							netadr_t to );
extern void NetProf_AddPacket( netProfileStream_t *pStream, int iSize,
							   int bFragment );
extern void NetProf_PrepProfiling( netProfileInfo_t **pProfile );
extern void NetProf_UpdateStatistics( netProfileStream_t *pStream );
extern void Netchan_Transmit( netchan_t *chan, int length, const byte *data );
extern void Netchan_TransmitNextFragment( netchan_t *chan );

/* qcommon/net_chan_mp.c, 0x015F7060 -- the CLIENT's out-of-band profile block. The server's counterpart is svs.pOOBProf. */
extern netProfileInfo_t *pProf;

#define clc_netchan             ( *(netchan_t *)chan )

#define MAX_RELIABLE_COMMANDS   64
#define MAX_STRING_CHARS_CMD    1024

#define CG_PROFILE_DRAW_TRAP    15

#define PROFILE_LINE_SIZE       1024

/* ---- CL_Netchan_Encode  0x00414710 ----  VERIFIED */
void CL_Netchan_Encode( byte *data, int length ) {
	const char *string;
	int         index;
	int         i;
	byte        key;

	string = &clc_serverCommands[ MAX_STRING_CHARS_CMD *
								  ( clc_serverCommandSequence & ( MAX_RELIABLE_COMMANDS - 1 ) ) ];
	index = 0;
	key = (byte) ( clc_serverMessageSequence ^ cl_serverId ^ clc_challenge[0] );

	for ( i = 0; i < length; i++ ) {
		if ( !string[index] ) {
			index = 0;
		}
		key ^= (byte) ( string[index] << ( i & 1 ) );
		index++;
		data[i] ^= key;
	}
}

/* ---- CL_Netchan_Decode  0x00414780 ----  VERIFIED */
void CL_Netchan_Decode( byte *data, int length ) {
	const char *string;
	int         index;
	int         i;
	byte        key;

	string = &clc_reliableCommands[ MAX_STRING_CHARS_CMD *
							( clc_reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ) ];
	index = 0;
	key = (byte) ( clc_serverMessageSequence ^ clc_challenge[0] );

	for ( i = 0; i < length; i++ ) {
		if ( !string[index] ) {
			index = 0;
		}
		key ^= (byte) ( string[index] << ( i & 1 ) );
		index++;
		data[i] ^= key;
	}
}

/* ---- CL_Netchan_TransmitNextFragment  0x004147E0 ----  VERIFIED */
void CL_Netchan_TransmitNextFragment( netchan_t *chan ) {
	Netchan_TransmitNextFragment( chan );
}

/* ---- CL_Netchan_Transmit  0x004147F0 ----  VERIFIED */
void CL_Netchan_Transmit( byte *data, int length, netchan_t *chan ) {
	CL_Netchan_Encode( data + 9, length - 9 );
	Netchan_Transmit( chan, length, data );
}

/* ---- CL_Netchan_AddOOBProfilePacket  0x00414810 ----  VERIFIED */
void CL_Netchan_AddOOBProfilePacket( int iSize ) {
	if ( !net_profile->integer ) {
		return;
	}

	NetProf_PrepProfiling( &pProf );
	NetProf_AddPacket( (netProfileStream_t *)pProf, iSize, 0 );
}

/* ---- CL_Netchan_SendOOBPacket  0x00414840 ----  VERIFIED */
void CL_Netchan_SendOOBPacket( int length, const void *data, netadr_t to ) {
	if ( *(const int *)data != -1 ) {
		Com_Printf( "CL_Netchan_SendOOBPacket used to send non-OOB packet.\n" );
	}

	NetProf_PrepProfiling( &pProf );
	NET_SendPacket( NS_CLIENT, length, data, to );

	if ( net_profile->integer ) {
		NetProf_PrepProfiling( &pProf );
		NetProf_AddPacket( (netProfileStream_t *)pProf, length, 0 );
	}
}

/* ---- CL_Netchan_UpdateProfileStats  0x004148B0 ----  VERIFIED */
void CL_Netchan_UpdateProfileStats( void ) {
	if ( clc_netchan.pProf ) {
		NetProf_UpdateStatistics( &clc_netchan.pProf->send );
		NetProf_UpdateStatistics( &clc_netchan.pProf->receive );
	}

	if ( pProf ) {
		NetProf_UpdateStatistics( &pProf->send );
		NetProf_UpdateStatistics( &pProf->receive );
	}
}

/* ---- CL_ProfDraw  0x00414900 ----  VERIFIED */
void CL_ProfDraw( int y, const char *text ) {
	if ( !cgvm ) {
		return;
	}
	VM_Call( cgvm, CG_PROFILE_DRAW_TRAP, 32, y, text, 0, 8, 10, 0 );
}

/* ---- CL_Netchan_PrintProfileStats  0x00414920 ----  VERIFIED */
void CL_Netchan_PrintProfileStats( qboolean bPrintToConsole ) {
	netProfileInfo_t    *oob;
	netProfileInfo_t    *prof;
	int y;
	int sendBps;
	int recvBps;
	char line[PROFILE_LINE_SIZE];

	sendBps = 0;
	recvBps = 0;
	y = 80;

	CL_Netchan_UpdateProfileStats();

	if ( bPrintToConsole ) {
		Com_Printf( "\n\n" );
	} else {
		y = 90;
	}

	Com_sprintf( line, PROFILE_LINE_SIZE, "====================" );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		CL_ProfDraw( y, line );
		y += 10;
	}

	Com_sprintf( line, PROFILE_LINE_SIZE, "Client Network Profile:" );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n\n", line );
	} else {
		CL_ProfDraw( y, line );
		y += 10;
		y += 10;
	}

	Com_sprintf( line, PROFILE_LINE_SIZE,
				 "      Source    bps   max   min frag%%" );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		CL_ProfDraw( y, line );
		y += 10;
	}

	oob = pProf;
	if ( oob ) {
		sendBps = oob->send.iBytesPerSecond;
		recvBps = oob->receive.iBytesPerSecond;

		Com_sprintf( line, PROFILE_LINE_SIZE, "    OOB Sent: %5i %5i %5i    -",
					 oob->send.iBytesPerSecond,
					 oob->send.iLargestPacket,
					 oob->send.iSmallestPacket );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}

		Com_sprintf( line, PROFILE_LINE_SIZE, "OOB Recieved: %5i %5i %5i    -",
					 oob->receive.iBytesPerSecond,
					 oob->receive.iLargestPacket,
					 oob->receive.iSmallestPacket );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}
	} else {
		Com_sprintf( line, PROFILE_LINE_SIZE,
					 "    OOB Sent:     0     0     0    -" );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}

		Com_sprintf( line, PROFILE_LINE_SIZE,
					 "OOB Recieved:     0     0     0    -" );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}
	}

	prof = clc_netchan.pProf;
	if ( prof ) {
		sendBps += prof->send.iBytesPerSecond;
		recvBps += prof->receive.iBytesPerSecond;

		Com_sprintf( line, PROFILE_LINE_SIZE,
					 "        Sent: %5i %5i %5i  %3i%%",
					 prof->send.iBytesPerSecond,
					 prof->send.iLargestPacket,
					 prof->send.iSmallestPacket,
					 prof->send.iFragmentPercentage );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}

		Com_sprintf( line, PROFILE_LINE_SIZE,
					 "    Recieved: %5i %5i %5i  %3i%%",
					 prof->receive.iBytesPerSecond,
					 prof->receive.iLargestPacket,
					 prof->receive.iSmallestPacket,
					 prof->receive.iFragmentPercentage );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}
	} else {
		Com_sprintf( line, PROFILE_LINE_SIZE,
					 "        Sent:     0     0     0    0%" );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}

		Com_sprintf( line, PROFILE_LINE_SIZE,
					 "    Recieved:     0     0     0    0%" );
		if ( bPrintToConsole ) {
			Com_Printf( "%s\n", line );
		} else {
			CL_ProfDraw( y, line );
			y += 10;
		}
	}

	Com_sprintf( line, PROFILE_LINE_SIZE, "       Total: %5i",
				 sendBps + recvBps );
	if ( bPrintToConsole ) {
		Com_Printf( "%s\n", line );
	} else {
		CL_ProfDraw( y, line );
	}
}
