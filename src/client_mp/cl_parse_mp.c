/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "cl_records.h"
#include "cl_http.h"

extern int CL_AddReliableCommand();
extern int CL_InitDownloads();
extern int CL_NextDownload();
extern int CL_WritePacket();
struct cmSphereRecord_t;
extern void CM_Trace( trace_t *results, const vec3_t start, const vec3_t end,
                      const vec3_t mins, const vec3_t maxs, clipHandle_t model,
                      const vec3_t origin, int brushmask, qboolean capsule,
                      const struct cmSphereRecord_t *sphere );

extern float yaw;
extern float leanFrac;
extern int Con_Close();
extern char *MSG_ReadBigString( msg_t *msg );
extern int   MSG_ReadShort( msg_t *msg );
extern int   MSG_ReadBits( msg_t *msg, int bits );
extern void  CL_ClearState( void );
extern void  FS_Restart( int checksumFeed );
extern cvar_t *fs_gamedirvar;
extern int MSG_ReadBitsCompress( const byte *input, byte *outputBuf, int readsize, int capacity );
extern qboolean MSG_ReadDeltaClient( msg_t *msg, const byte *from, byte *to,
                                     int number );
extern void MSG_ReadDeltaPlayerstate( msg_t *msg, const byte *from, byte *to,
                                      int number );
extern int  MSG_ReadLong( msg_t *msg );
extern int  MSG_ReadByte( msg_t *msg );
extern qboolean MSG_ReadDeltaStruct( msg_t *msg, const byte *from, byte *to,
                                     unsigned int number, int numFields,
                                     int indexBits, const netField_t *stateFields );
extern int MSG_ReadString();
extern void VectorNormalizeFast( vec3_t v );
extern int j__atol( const char *s );

/* ---- SHOWNET  0x00414EC0 ----  VERIFIED */
void SHOWNET( msg_t *msg, const char *s )
{
	if ( cl_shownet->integer >= 2 ) {
		Com_Printf( "%3i:%s\n", msg->readcount - 1, s );
	}
}

/* ---- isEntVisible  0x00414EF0 ----  VERIFIED */
qboolean isEntVisible( const byte *ent )
{
	vec3_t  start;
	vec3_t  body;
	vec3_t  alt;
	vec3_t  dir;
	vec3_t  off10;
	vec3_t  off18;
	float   height;
	qboolean crouched;
	trace_t trace;

	start[0] = cgameClientLerpOrigin_x;
	start[1] = cgameClientLerpOrigin_y;
	start[2] = cgameClientLerpOrigin_z + ( flt_1432A50 - 1.0f );
	AddLeanToPosition( start, yaw, leanFrac, 16.0f, 20.0f );

	body[0] = *(const float *)( ent + 0x18 );
	body[1] = *(const float *)( ent + 0x1C );
	body[2] = *(const float *)( ent + 0x20 );

	dir[0] = body[0] - start[0];
	dir[1] = body[1] - start[1];
	dir[2] = body[2] - start[2];
	VectorNormalizeFast( dir );

	off18[0] = dir[1] - dir[2] * 0.0f;
	off18[1] = dir[2] * 0.0f - dir[0];
	off18[2] = dir[0] * 0.0f - dir[1] * 0.0f;
	VectorNormalizeFast( off18 );

	crouched = ( *(const int *)( ent + 0xE0 ) != 0 );

	off10[0] = off18[0] * 10.0f;
	off10[1] = off18[1] * 10.0f;
	off10[2] = off18[2] * 10.0f;

	off18[0] = off18[0] * 18.0f;
	off18[1] = off18[1] * 18.0f;
	off18[2] = off18[2] * 18.0f;

	height = crouched ? 16.0f : 40.0f;

	body[2] = height + body[2];
	trace.fraction = 1.0f;
	CM_Trace( &trace, start, body, NULL, NULL, 0, vec3_origin, 1, qfalse, NULL );
	if ( trace.fraction == 1.0f ) {
		return qtrue;
	}

	body[2] = body[2] + 16.0f;
	trace.fraction = 1.0f;
	CM_Trace( &trace, start, body, NULL, NULL, 0, vec3_origin, 1, qfalse, NULL );
	if ( trace.fraction == 1.0f ) {
		return qtrue;
	}

	body[2] = body[2] - 16.0f;
	body[2] = body[2] - height;
	trace.fraction = 1.0f;
	CM_Trace( &trace, start, body, NULL, NULL, 0, vec3_origin, 1, qfalse, NULL );
	if ( trace.fraction == 1.0f ) {
		return qtrue;
	}

	alt[0] = off18[0] + body[0];
	alt[1] = off18[1] + body[1];
	alt[2] = off18[2] + body[2];
	alt[2] = alt[2] + 8.0f;
	trace.fraction = 1.0f;
	CM_Trace( &trace, start, alt, NULL, NULL, 0, vec3_origin, 1, qfalse, NULL );
	if ( trace.fraction == 1.0f ) {
		return qtrue;
	}

	alt[0] = off10[0] + body[0];
	alt[1] = off10[1] + body[1];
	alt[2] = off10[2] + body[2];
	alt[2] = alt[2] + ( crouched ? 28.0f : 52.0f );
	trace.fraction = 1.0f;
	CM_Trace( &trace, start, alt, NULL, NULL, 0, vec3_origin, 1, qfalse, NULL );
	if ( trace.fraction == 1.0f ) {
		return qtrue;
	}

	off18[0] = off18[0] * -1.0f;
	off18[1] = off18[1] * -1.0f;
	off18[2] = off18[2] * -1.0f;
	off10[0] = off10[0] * -1.0f;
	off10[1] = off10[1] * -1.0f;
	off10[2] = off10[2] * -1.0f;

	alt[0] = off10[0] + body[0];
	alt[1] = off10[1] + body[1];
	alt[2] = off10[2] + body[2];
	alt[2] = alt[2] + 2.0f;
	trace.fraction = 1.0f;
	CM_Trace( &trace, start, alt, NULL, NULL, 0, vec3_origin, 1, qfalse, NULL );
	if ( trace.fraction == 1.0f ) {
		return qtrue;
	}

	alt[0] = off18[0] + body[0];
	alt[1] = off18[1] + body[1];
	alt[2] = off18[2] + body[2];
	alt[2] = alt[2] + ( crouched ? 16.0f : 36.0f );
	trace.fraction = 1.0f;
	CM_Trace( &trace, start, alt, NULL, NULL, 0, vec3_origin, 1, qfalse, NULL );

	return ( trace.fraction == 1.0f );
}

/* ---- CL_DeltaEntity  0x00415350 ----  VERIFIED */
void CL_DeltaEntity( msg_t *msg, byte *frame, unsigned int newnum,
                     const byte *old, qboolean unchanged )
{
	byte *state;
	int   number;
	int   serverTime;

	state = &byte_14B9134[ ES_SIZE * ( cl_parseEntitiesNum & 0x7FF ) ];

	if ( unchanged ) {
		Com_Memcpy( state, old, ES_SIZE );
	} else if ( MSG_ReadDeltaStruct( msg, old, state, newnum, 0x3B, 10,
	                                 (const netField_t *)stateFields ) ) {
		return;
	}

	if ( dword_15CE990 && *(int *)( state + ES_NUMBER ) < 64 ) {
		number     = *(int *)( state + ES_NUMBER );
		serverTime = *(int *)( frame + SNAP_SERVERTIME );

		if ( isEntVisible( state ) ) {
			entLastVisible[ number ] = serverTime;
			*(int *)( state + ES_EFLAGS ) &= ~0x100;
		} else if ( entLastVisible[ number ] < serverTime - 600 ) {
			*(int *)( state + ES_EFLAGS ) |= 0x100;
		}
	}

	cl_parseEntitiesNum++;
	++*(int *)( frame + SNAP_NUMENTITIES );
}

/* ---- CL_DeltaClient  0x00415410 ----  VERIFIED */
void CL_DeltaClient( msg_t *msg, byte *frame, int number,
                     const byte *old, qboolean unchanged )
{
	byte *state;

	state = &byte_1531134[ CS_SIZE * ( cl_parseClientsNum & 0x7FF ) ];

	if ( unchanged ) {
		Com_Memcpy( state, old, CS_SIZE );
	} else if ( MSG_ReadDeltaClient( msg, old, state, number ) ) {
		return;
	}

	cl_parseClientsNum++;
	++*(int *)( frame + SNAP_NUMCLIENTS );
}

/* ---- CL_ParsePacketEntities  0x00415460 ----  VERIFIED */
void CL_ParsePacketEntities( msg_t *msg, byte *oldframe, byte *newframe )
{
	int   newnum;
	byte *oldstate;
	int   oldindex;
	int   oldnum;

	*(int *)( newframe + SNAP_PARSEENTITIESNUM ) = cl_parseEntitiesNum;
	*(int *)( newframe + SNAP_NUMENTITIES )      = 0;

	oldindex = 0;
	oldstate = NULL;
	if ( !oldframe ) {
		oldnum = 99999;
	} else if ( oldindex >= *(int *)( oldframe + SNAP_NUMENTITIES ) ) {
		oldnum = 99999;
	} else {
		oldstate = &byte_14B9134[ ES_SIZE *
			( ( *(int *)( oldframe + SNAP_PARSEENTITIESNUM ) + oldindex ) & 0x7FF ) ];
		oldnum = *(int *)( oldstate + ES_NUMBER );
	}

	while ( 1 ) {
		newnum = MSG_ReadBits( msg, 10 );

		if ( newnum == 0x3FF ) {
			break;
		}

		if ( msg->readcount > msg->cursize ) {
			/* Split literal: C's \x escape is greedy, so "\x15CL_..." lexes as one character. */
			Com_Error( ERR_DROP, "\x15" "CL_ParsePacketEntities: end of message" );
		}

		while ( oldnum < newnum ) {
			if ( cl_shownet->integer == 3 ) {
				Com_Printf( "%3i:  unchanged: %i\n", msg->readcount, oldnum );
			}
			CL_DeltaEntity( msg, newframe, oldnum, oldstate, qtrue );

			oldindex++;
			if ( oldindex >= *(int *)( oldframe + SNAP_NUMENTITIES ) ) {
				oldnum = 99999;
			} else {
				oldstate = &byte_14B9134[ ES_SIZE *
					( ( *(int *)( oldframe + SNAP_PARSEENTITIESNUM ) + oldindex ) & 0x7FF ) ];
				oldnum = *(int *)( oldstate + ES_NUMBER );
			}
		}

		if ( oldnum == newnum ) {
			if ( cl_shownet->integer == 3 ) {
				Com_Printf( "%3i:  delta: %i\n", msg->readcount, newnum );
			}
			CL_DeltaEntity( msg, newframe, newnum, oldstate, qfalse );

			oldindex++;
			if ( oldindex >= *(int *)( oldframe + SNAP_NUMENTITIES ) ) {
				oldnum = 99999;
			} else {
				oldstate = &byte_14B9134[ ES_SIZE *
					( ( *(int *)( oldframe + SNAP_PARSEENTITIESNUM ) + oldindex ) & 0x7FF ) ];
				oldnum = *(int *)( oldstate + ES_NUMBER );
			}
			continue;
		}

		if ( cl_shownet->integer == 3 ) {
			Com_Printf( "%3i:  baseline: %i\n", msg->readcount, newnum );
		}
		CL_DeltaEntity( msg, newframe, newnum,
		                &byte_147D134[ ES_SIZE * newnum ], qfalse );
	}

	while ( oldnum != 99999 ) {
		if ( cl_shownet->integer == 3 ) {
			Com_Printf( "%3i:  unchanged: %i\n", msg->readcount, oldnum );
		}
		CL_DeltaEntity( msg, newframe, oldnum, oldstate, qtrue );

		oldindex++;
		if ( oldindex >= *(int *)( oldframe + SNAP_NUMENTITIES ) ) {
			oldnum = 99999;
		} else {
			oldstate = &byte_14B9134[ ES_SIZE *
				( ( *(int *)( oldframe + SNAP_PARSEENTITIESNUM ) + oldindex ) & 0x7FF ) ];
			oldnum = *(int *)( oldstate + ES_NUMBER );
		}
	}

	if ( cl_shownuments->integer ) {
		Com_Printf( "Entities in packet: %i\n",
		            *(int *)( newframe + SNAP_NUMENTITIES ) );
	}
}

/* ---- CL_ParsePacketClients  0x004157F0 ----  VERIFIED */
void CL_ParsePacketClients( msg_t *msg, byte *oldframe, byte *newframe )
{
	int   newnum;
	byte *oldstate;
	int   oldindex;
	int   oldnum;

	*(int *)( newframe + SNAP_PARSECLIENTSNUM ) = cl_parseClientsNum;
	*(int *)( newframe + SNAP_NUMCLIENTS )      = 0;

	oldindex = 0;
	oldstate = NULL;
	if ( !oldframe ) {
		oldnum = 99999;
	} else if ( oldindex >= *(int *)( oldframe + SNAP_NUMCLIENTS ) ) {
		oldnum = 99999;
	} else {
		oldstate = &byte_1531134[ CS_SIZE *
			( ( *(int *)( oldframe + SNAP_PARSECLIENTSNUM ) + oldindex ) & 0x7FF ) ];
		oldnum = *(int *)( oldstate + CS_NUMBER );
	}

	while ( 1 ) {
		if ( !MSG_ReadBits( msg, 1 ) ) {
			break;
		}

		newnum = MSG_ReadBits( msg, 6 );

		if ( msg->readcount > msg->cursize ) {
			Com_Error( ERR_DROP, "\x15" "CL_ParsePacketClients: end of message" );
		}

		while ( oldnum < newnum ) {
			if ( cl_shownet->integer == 3 ) {
				Com_Printf( "%3i:  unchanged: %i\n", msg->readcount, oldnum );
			}
			CL_DeltaClient( msg, newframe, oldnum, oldstate, qtrue );

			oldindex++;
			if ( oldindex >= *(int *)( oldframe + SNAP_NUMCLIENTS ) ) {
				oldnum = 99999;
			} else {
				oldstate = &byte_1531134[ CS_SIZE *
					( ( *(int *)( oldframe + SNAP_PARSECLIENTSNUM ) + oldindex ) & 0x7FF ) ];
				oldnum = *(int *)( oldstate + CS_NUMBER );
			}
		}

		if ( oldnum == newnum ) {
			if ( cl_shownet->integer == 3 ) {
				Com_Printf( "%3i:  delta: %i\n", msg->readcount, newnum );
			}
			CL_DeltaClient( msg, newframe, newnum, oldstate, qfalse );

			oldindex++;
			if ( oldindex >= *(int *)( oldframe + SNAP_NUMCLIENTS ) ) {
				oldnum = 99999;
			} else {
				oldstate = &byte_1531134[ CS_SIZE *
					( ( *(int *)( oldframe + SNAP_PARSECLIENTSNUM ) + oldindex ) & 0x7FF ) ];
				oldnum = *(int *)( oldstate + CS_NUMBER );
			}
			continue;
		}

		if ( cl_shownet->integer == 3 ) {
			Com_Printf( "%3i:  baseline: %i\n", msg->readcount, newnum );
		}
		CL_DeltaClient( msg, newframe, newnum, NULL, qfalse );
	}

	while ( oldnum != 99999 ) {
		if ( cl_shownet->integer == 3 ) {
			Com_Printf( "%3i:  unchanged: %i\n", msg->readcount, oldnum );
		}
		CL_DeltaClient( msg, newframe, oldnum, oldstate, qtrue );

		oldindex++;
		if ( oldindex >= *(int *)( oldframe + SNAP_NUMCLIENTS ) ) {
			oldnum = 99999;
		} else {
			oldstate = &byte_1531134[ CS_SIZE *
				( ( *(int *)( oldframe + SNAP_PARSECLIENTSNUM ) + oldindex ) & 0x7FF ) ];
			oldnum = *(int *)( oldstate + CS_NUMBER );
		}
	}

	if ( cl_shownuments->integer ) {
		Com_Printf( "Clients in packet: %i\n",
		            *(int *)( newframe + SNAP_NUMCLIENTS ) );
	}
}

/* cl.outPackets[32], 12 bytes each, based at 0x0143AFB4; serverTime is +4 and realtime +8. */
#define OUTPACKET_SERVERTIME( i )  dword_143AFB8[ ( (i) & 31 ) * 3 ]
#define OUTPACKET_REALTIME( i )    dword_143AFBC[ ( (i) & 31 ) * 3 ]

/* ---- CL_PublishSnap  no-address ---- */
static void CL_PublishSnap( const byte *snap )
{
	Com_Memcpy( &cl_snap_valid, snap, SNAP_SIZE );

	*(int *)cl_snap_snapFlags = *(const int *)( snap + SNAP_SNAPFLAGS );
	cl_snap_serverTime     = *(const int *)( snap + SNAP_SERVERTIME );
	*(int *)cl_snap_messageNum = *(const int *)( snap + SNAP_MESSAGENUM );
	cl_snap_deltaNum       = *(const int *)( snap + SNAP_DELTANUM );
	cl_snap_ping           = *(const int *)( snap + SNAP_PING );
	cl_snap_ps_commandTime = *(const int *)( snap + SNAP_PS );
}

/* ---- CL_ParseSnapshot  0x00415BB0 ----  VERIFIED */
void CL_ParseSnapshot( msg_t *msg )
{
	byte  newSnap[SNAP_SIZE];
	byte *old;
	int   deltaNum;
	int   oldMessageNum;
	int   packetNum;
	int   i;

	Com_Memset( newSnap, 0, sizeof( newSnap ) );

	*(int *)( newSnap + SNAP_SERVERCOMMANDNUM ) = clc_serverCommandSequence;
	*(int *)( newSnap + SNAP_SERVERTIME )       = MSG_ReadLong( msg );
	*(int *)( newSnap + SNAP_MESSAGENUM )       = clc_serverMessageSequence;

	deltaNum = MSG_ReadByte( msg );
	if ( !deltaNum ) {
		*(int *)( newSnap + SNAP_DELTANUM ) = -1;
	} else {
		*(int *)( newSnap + SNAP_DELTANUM ) =
			*(int *)( newSnap + SNAP_MESSAGENUM ) - deltaNum;
	}

	*(int *)( newSnap + SNAP_SNAPFLAGS ) = MSG_ReadByte( msg );

	if ( *(int *)( newSnap + SNAP_DELTANUM ) <= 0 ) {
		*(int *)( newSnap + SNAP_VALID ) = qtrue;
		old = NULL;
		clc_demowaiting = qfalse;
	} else {
		old = (byte *)cl_snapshots
		    + SNAP_SIZE * ( *(int *)( newSnap + SNAP_DELTANUM ) & 31 );

		if ( !*(int *)( old + SNAP_VALID ) ) {
			Com_Printf( "Delta from invalid frame (not supposed to happen!).\n" );
		} else if ( *(int *)( old + SNAP_MESSAGENUM )
		            != *(int *)( newSnap + SNAP_DELTANUM ) ) {
			Com_DPrintf( "Delta frame too old.\n" );
		} else if ( cl_parseEntitiesNum
		            - *(int *)( old + SNAP_PARSEENTITIESNUM ) > 0x780 ) {
			Com_DPrintf( "Delta parseEntitiesNum too old.\n" );
		} else if ( cl_parseClientsNum
		            - *(int *)( old + SNAP_PARSECLIENTSNUM ) > 0x780 ) {
			Com_DPrintf( "Delta parseClientsNum too old.\n" );
		} else {
			*(int *)( newSnap + SNAP_VALID ) = qtrue;
		}
	}

	SHOWNET( msg, "playerstate" );
	if ( old ) {
		MSG_ReadDeltaPlayerstate( msg, old + SNAP_PS, newSnap + SNAP_PS, 0 );
	} else {
		MSG_ReadDeltaPlayerstate( msg, NULL, newSnap + SNAP_PS, 0 );
	}

	SHOWNET( msg, "packet entities" );
	CL_ParsePacketEntities( msg, old, newSnap );

	SHOWNET( msg, "packet clients" );
	CL_ParsePacketClients( msg, old, newSnap );

	if ( !*(int *)( newSnap + SNAP_VALID ) ) {
		return;
	}

	oldMessageNum = *(int *)cl_snap_messageNum + 1;

	if ( *(int *)( newSnap + SNAP_MESSAGENUM ) - oldMessageNum >= 32 ) {
		oldMessageNum = *(int *)( newSnap + SNAP_MESSAGENUM ) - 31;
	}
	for ( ; oldMessageNum < *(int *)( newSnap + SNAP_MESSAGENUM ); oldMessageNum++ ) {
		*(int *)( (byte *)cl_snapshots + SNAP_SIZE * ( oldMessageNum & 31 )
		          + SNAP_VALID ) = qfalse;
	}

	CL_PublishSnap( newSnap );

	cl_snap_ping = 999;
	*(int *)( (byte *)&cl_snap_valid + SNAP_PING ) = 999;

	for ( i = 0; i < 32; i++ ) {
		packetNum = *(int *)( chan + 0x24 ) - 1 - i;
		if ( cl_snap_ps_commandTime >= OUTPACKET_SERVERTIME( packetNum ) ) {
			cl_snap_ping = cls_realtime - OUTPACKET_REALTIME( packetNum );
			*(int *)( (byte *)&cl_snap_valid + SNAP_PING ) = cl_snap_ping;
			break;
		}
	}

	Com_Memcpy( (byte *)cl_snapshots + SNAP_SIZE * ( *(int *)cl_snap_messageNum & 31 ),
	            &cl_snap_valid, SNAP_SIZE );

	if ( cl_shownet->integer == 3 ) {
		Com_Printf( "   snapshot:%i  delta:%i  ping:%i\n",
		            *(int *)cl_snap_messageNum, cl_snap_deltaNum, cl_snap_ping );
	}
	
	cl_newSnapshots = qtrue;
}

/* ---- CL_SystemInfoChanged  0x00415EB0 ----  VERIFIED */
void CL_SystemInfoChanged( void )
{
	const char *systemInfo;
	const char *s, *t;
	char        key[8192];
	char        value[8192];
	cvar_t     *var;
	qboolean    gameDirSeen = qfalse;

	systemInfo = &cl_gameState_stringData[ cl_gameState_stringOffsets[1] ];

	cl_serverId = j__atol( Info_ValueForKey( systemInfo, "sv_serverid" ) );

	Com_Memset( entLastVisible, 0, sizeof( entLastVisible ) );

	if ( clc_demoplaying ) {
		return;
	}

	s = Info_ValueForKey( systemInfo, "sv_cheats" );
	if ( !j__atol( s ) ) {
		Cvar_SetCheatState();
	}

	s = Info_ValueForKey( systemInfo, "sv_paks" );
	t = Info_ValueForKey( systemInfo, "sv_pakNames" );
	FS_PureServerSetLoadedPaks( s, t );

	s = Info_ValueForKey( systemInfo, "sv_referencedPaks" );
	t = Info_ValueForKey( systemInfo, "sv_referencedPakNames" );
	FS_PureServerSetReferencedPaks( s, t );

	if ( !com_sv_running->integer ) {
		s = systemInfo;
		while ( s ) {
			Info_NextPair( &s, key, value );
			if ( !key[0] ) {
				break;
			}
			if ( !Q_stricmp( key, "fs_game" ) ) {
				gameDirSeen = qtrue;
			}
			Cvar_Set2( key, value, qtrue );
		}
		/* Empty cvars are omitted from systeminfo. A server without fs_game
		 * uses main, not the mod selected locally before connecting.
		 * CL_ParseGamestate restarts the filesystem when this changes it. */
		if ( !gameDirSeen ) {
			Cvar_Set2( "fs_game", "", qtrue );
		}
	}

	var = Cvar_FindVar( "sv_pure" );
	cl_connectedToPureServer = (int)( var ? var->value : 0.0f );
}

/* ---- CL_ParseGamestate  0x00416050 ----  VERIFIED */
void CL_ParseGamestate( msg_t *msg )
{
	int   cmd;
	int   i;
	int   newnum;
	int   len;
	char *s;
	byte  nullstate[ES_SIZE];

	Con_Close();

	CL_HTTPGamestate();
	CL_ClearState();
	clc_connectPacketCount = 0;

	clc_serverCommandSequence = MSG_ReadLong( msg );

	cl_gameState_dataCount = 1;

	while ( 1 ) {
		cmd = MSG_ReadByte( msg );

		if ( cmd == 8 ) {
			break;
		}

		if ( cmd == 3 ) {
			i = MSG_ReadShort( msg );
			if ( i < 0 || i >= 0x800 ) {
				Com_Error( ERR_DROP, "\x15" "configstring > MAX_CONFIGSTRINGS" );
			}

			s   = MSG_ReadBigString( msg );
			len = strlen( s );

			if ( len + 1 + cl_gameState_dataCount > Protocol_GameStateLimit( clc_serverBuild >= 0 ) ) {
				Com_Error( ERR_DROP, "\x15" "MAX_GAMESTATE_CHARS exceeded" );
			}

			cl_gameState_stringOffsets[ i ] = cl_gameState_dataCount;
			Com_Memcpy( &cl_gameState_stringData[ cl_gameState_dataCount ],
			            s, len + 1 );
			cl_gameState_dataCount += len + 1;

		} else if ( cmd == 4 ) {
			newnum = MSG_ReadBits( msg, 10 );
			if ( newnum < 0 || newnum >= 0x400 ) {
				Com_Error( ERR_DROP, "\x15" "Baseline number out of range: %i",
				           newnum );
			}
			Com_Memset( nullstate, 0, sizeof( nullstate ) );
			MSG_ReadDeltaStruct( msg, nullstate,
			                     &byte_147D134[ ES_SIZE * newnum ],
			                     (unsigned int)newnum, 0x3B, 10,
			                     (const netField_t *)stateFields );

		} else {
			Com_Error( ERR_DROP, "\x15" "CL_ParseGamestate: bad command byte" );
		}
	}

	clc_clientNum    = MSG_ReadLong( msg );
	clc_checksumFeed = MSG_ReadLong( msg );

	CL_SystemInfoChanged();

	if ( !com_sv_running->integer
	  && ( fs_gamedirvar->modified || clc_checksumFeed != fs_checksumFeed ) ) {
		FS_Restart( clc_checksumFeed );
	}

	CL_InitDownloads();

	Cvar_Set2( "cl_paused", "0", qtrue );
}

/* ---- CL_ParseDownload  0x004162A0 ----  [HIGH] */
void __cdecl CL_ParseDownload( msg_t *v0 )
{
  msg_t *v1;
  int cursize;
  int v3;
  unsigned __int8 *data;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  unsigned __int8 *v10;
  __int16 v11;
  int v12;
  char *String;
  char *v14;
  fileHandle_t v15;
  char *v16;
  float value;
  float valuea;
  const char *valueb;
  int v20;
  int v21;
  _BYTE Buffer[16388];

  if ( CL_HTTPRedirect( v0 ) ) return;
  v1 = v0;
  cursize = v0->cursize;
  v3 = v0->readcount + 2;
  if ( v3 > cursize )
  {
    v21 = -1;
  }
  else
  {
    data = v0->data;
    /* The wire carries the low 16 bits of an increasing block counter. */
    v5 = *(unsigned short *)&data[v1->readcount];
    v5 = *(int *)cls_downloadBlock + (short)( v5 - *(int *)cls_downloadBlock );
    v1->readcount = v3;
    v21 = v5;
    if ( !v5 )
    {
      v6 = v3 + 4;
      if ( v3 + 4 > cursize )
      {
        v7 = -1;
      }
      else
      {
        v7 = *(_DWORD *)&data[v3];
        v1->readcount = v6;
      }
      clc_downloadSize = v7;
      value = (float)v7;
      Cvar_SetValue("cl_downloadSize", value);
      if ( clc_downloadSize < 0 )
      {
        String = MSG_ReadString(v1);
        Com_Error(ERR_DROP, "%s", String);
      }
    }
  }
  v8 = v1->cursize;
  v9 = v1->readcount + 2;
  if ( v9 > v8 )
  {
    v20 = -1;
    v12 = -1;
  }
  else
  {
    v10 = v1->data;
    v11 = *(_WORD *)&v10[v1->readcount];
    v12 = v11;
    v1->readcount = v9;
    v20 = v11;
    if ( v11 < 0 || v11 > sizeof( Buffer ) || v11 > v8 - v9 )
      Com_Error(ERR_DROP, "Invalid download block size");
    if ( v11 > 0 )
    {
      if ( v9 + v11 > v8 )
      {
        Com_Error(ERR_DROP, "Truncated download block");
      }
      else
      {
        qmemcpy(Buffer, &v10[v9], v11);
        v1->readcount = v9 + v11;
      }
    }
  }
  if ( v20 < 0 || v3 > cursize )
    Com_Error(ERR_DROP, "Truncated download packet");
  if ( *(_DWORD *)cls_downloadBlock != v21 )
  {
    Com_DPrintf("CL_ParseDownload: Expected block %d, got %d\n", *(_DWORD *)cls_downloadBlock, v21);
    if ( v21 > *(int *)cls_downloadBlock )
    {
      Com_DPrintf("CL_ParseDownload: Sending retransmitt request to get the missed block\n");
      v14 = va("retransdl %d", *(_DWORD *)cls_downloadBlock);
      CL_AddReliableCommand(v14);
    }
    return;
  }
  v15 = clc_download;
  if ( !clc_download )
  {
    if ( !cls_downloadTempName[0] )
    {
      Com_Printf("Server sending download, but no download was requested\n");
      CL_AddReliableCommand("stopdl");
      return;
    }
    v15 = FS_SV_FOpenFileWrite(&cls_downloadTempName);
    clc_download = v15;
    if ( !v15 )
    {
      Com_Printf("Could not create %s\n", &cls_downloadTempName);
      CL_AddReliableCommand("stopdl");
LABEL_30:
      CL_NextDownload();
      return;
    }
    v12 = v20;
  }
  if ( v12 > clc_downloadSize - cls_downloadCount ||
       ( !v12 && cls_downloadCount != clc_downloadSize ) )
    Com_Error(ERR_DROP, "Download size does not match server header");
  if ( v12 && FS_Write(Buffer, v12, v15) != v12 )
    Com_Error(ERR_DROP, "Could not write download file");
  v16 = va("nextdl %d", *(_DWORD *)cls_downloadBlock);
  CL_AddReliableCommand(v16);
  cls_downloadCount += v20;
  valuea = (float)cls_downloadCount;
  ++*(_DWORD *)cls_downloadBlock;
  Cvar_SetValue("cl_downloadCount", valuea);
  if ( !v20 )
  {
    if ( clc_download )
    {
      FS_FCloseFile(clc_download);
      clc_download = 0;
      FS_SV_Rename((const char *)&cls_downloadTempName,
                   (const char *)&cls_downloadName);
    }
    cls_downloadName[0] = 0;
    cls_downloadTempName[0] = 0;
    Cvar_Set2("cl_downloadName", &empty_string, qtrue);
    CL_WritePacket();
    CL_WritePacket();
    goto LABEL_30;
  }
}

/* ---- CL_ParseCommandString  0x00416590 ----  [CONFIRMED] */
char *__cdecl CL_ParseCommandString(msg_t *a1)
{
  int readcount;
  int v3;
  int v4;
  int v5;
  char *result;
  char *v7;

  readcount = a1->readcount;
  v3 = readcount + 4;
  if ( readcount + 4 > a1->cursize )
  {
    v5 = -1;
  }
  else
  {
    v4 = *(_DWORD *)&a1->data[readcount];
    a1->readcount = v3;
    v5 = v4;
  }
  result = MSG_ReadString(a1);
  if ( clc_serverCommandSequence < v5 )
  {
    clc_serverCommandSequence = v5;
    v7 = &clc_serverCommands[1024 * (v5 & 0x3F)];
    result = strncpy(v7, result, 0x3FFu);
    v7[1023] = 0;
  }
  return result;
}

/* ---- CL_ParseServerMessage  0x004165F0 ----  VERIFIED */
void CL_ParseServerMessage( msg_t *source )
{
	msg_t msg;
	byte  to[MAX_MSGLEN];
	int   cmd;

	if ( cl_shownet->integer == 1 ) {
		Com_Printf( "%i ", source->cursize );
	} else if ( cl_shownet->integer >= 2 ) {
		Com_Printf( "------------------\n" );
	}

	if ( !msgInit ) {
		MSG_initHuffman();
	}

	Com_Memset( &msg, 0, sizeof( msg ) );
	msg.data    = to;
	msg.maxsize = sizeof( to );
	msg.extended = clc_serverBuild >= 0;
	msg.cursize = MSG_ReadBitsCompress( source->data + source->readcount, to,
	                                    source->cursize - source->readcount, sizeof( to ) );

	while ( 1 ) {
		if ( msg.readcount > msg.cursize ) {
			Com_Error( ERR_DROP,
			           "\x15" "CL_ParseServerMessage: read past end of server message" );
		}

		cmd = MSG_ReadByte( &msg );

		if ( cmd == 8 ) {
			SHOWNET( &msg, "END OF MESSAGE" );
			break;
		}

		if ( cl_shownet->integer >= 2 ) {
			if ( !( (const char **)&svc_strings )[ cmd ] ) {
				Com_Printf( "%3i:BAD CMD %i\n", msg.readcount - 1, cmd );
			} else {
				SHOWNET( &msg, ( (const char **)&svc_strings )[ cmd ] );
			}
		}

		switch ( cmd ) {
		default:
			Com_Error( ERR_DROP,
			           "\x15" "CL_ParseServerMessage: Illegible server message %d\n",
			           cmd );
			break;
		case 1:
			break;
		case 2:
			CL_ParseGamestate( &msg );
			break;
		case 5:
			CL_ParseCommandString( &msg );
			break;
		case 6:
			CL_ParseDownload( &msg );
			break;
		case 7:
			CL_ParseSnapshot( &msg );
			break;
		}
	}
}
