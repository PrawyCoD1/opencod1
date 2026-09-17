/*
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "cl_records.h"
#include "cl_vm.h"

extern void CL_ParseServerMessage( msg_t *msg );
extern void MSS_StopSounds( int flags );        /* 0x0044FA40 */

extern int CIN_PlayCinematic();
extern int CIN_StopCinematic();
extern int CIN_UploadCinematic();
extern int CL_CubeMapShot_f();
void CL_InitCGame( void );          /* 0x004049C0 */
extern int CL_InitInput();
int CL_InitUI( void );              /* 0x00418F10, cl_ui_mp.c */
extern int CL_Netchan_Decode();
void CL_Netchan_SendOOBPacket( int length, const void *data, netadr_t to );
extern int CL_PlayCinematic_f();
extern int CL_RestoreCgameState();
extern int CL_ShutdownInput();
extern void CL_UpdateLevelHunkUsage( void );
extern int CL_WritePacket();
extern int CM_GetPlaneNum();
extern int CM_SaveLump();
extern void Cmd_TokenizeString2( const char *text, int max_tokens );
qboolean Com_LoadCvarsFromBuffer( const char **cvarNames, int cvarCount,
                                  char *text, const char *filename );
qboolean Com_SaveCvarsToBuffer( const char **cvarNames, int cvarCount,
                                char *buffer, int bufferSize );
extern int Com_WriteLocalizedSoundAliasFiles();
extern int Con_Close();
extern int Con_Init();
extern int FS_ComparePaks();
extern int FS_FOpenFileRead_Internal();
extern int FS_FileExists();
extern int FS_FileIsInPAK();
extern int FS_LoadedPakNames();
extern int FS_ReferencedPakNames();
extern int FS_ReferencedPakPureChecksums();
extern int FS_ShiftStr();
extern int GetRefAPI();
extern void CL_InitRef( void );
extern void CL_ShutdownRef( void );
extern int Hunk_AllocInternal();
extern int Hunk_Clear();
extern int Hunk_FreeTempMemory();
extern char *MSG_ReadBigString( msg_t *msg );
extern int   MSG_ReadLong( msg_t *msg );
extern char *MSG_ReadString( msg_t *msg );
extern char *MSG_ReadStringLine( msg_t *msg );
void MSG_WriteBigString( const char *s, msg_t *sb );
int  MSG_WriteBitsCompress( const byte *datasrc, int bytecount, byte *buffdest );
extern int MSG_WriteDeltaStruct();
void MSG_WriteByte( msg_t *msg, int c );
void MSG_WriteShort( msg_t *msg, int c );
void MSG_WriteLong( msg_t *msg, int c );
void MSG_WriteDeltaEntity( msg_t *msg, const byte *from, const byte *to, qboolean force );

/* svc_* -- the canonical order of svc_strings at 0x0057B4D0, plus svc_EOF, which is the `8` CL_Record_f writes as its two end markers. */
#define svc_bad             0
#define svc_nop             1
#define svc_gamestate       2
#define svc_configstring    3
#define svc_baseline        4
#define svc_serverCommand   5
#define svc_download        6
#define svc_snapshot        7
#define svc_EOF             8
void MSG_WriteReliableCommandToBuffer( const char *pszCommand, int iBufferSize,
                                       char *pszBuffer );
extern int MSS_Init();
extern int MSS_Restore();
extern int MSS_Save();
extern int MSS_Shutdown();
extern const char *NET_AdrToString( netadr_t a );
extern qboolean    NET_CompareAdr( netadr_t a, netadr_t b );
extern qboolean    NET_CompareBaseAdr( netadr_t a, netadr_t b );
extern void        NET_OutOfBandData( netsrc_t sock, netadr_t adr, int len, const void *data );
extern void        NET_OutOfBandPrint( netsrc_t sock, netadr_t adr, const char *format, ... );
extern int NET_SendPacket();
extern qboolean    NET_StringToAdr( const char *s, netadr_t *a );
extern void        NetProf_AddPacket( void *pStream, int iSize, int bFragment );
extern void        NetProf_PrepProfiling( void *ppProfile );
extern qboolean    Netchan_Process( netchan_t *chan, msg_t *msg );
extern void        Netchan_Setup( netchan_t *chan, netsrc_t sock, netadr_t adr, int qport );
extern netProfileInfo_t *pProf;     /* qcommon/net_chan_mp.c, 0x008D8B44-ish */
/* Defined below at 0x00413430; called from CL_ConnectionlessPacket above it. */
void __cdecl CL_ServerStatusResponse( netadr_t from, void *msg );
/* Same shape: CL_DemoCompleted (0x0040E920) tail-jumps to CL_NextDemo (0x0040ED80), and CL_MapLoading (0x0040EE60) calls CL_CheckForResend (0x004103D0); retail places both callees after their callers. */
void CL_NextDemo( void );
void __cdecl CL_CheckForResend( void );
/* Both are dispatched by CL_ConnectionlessPacket (0x004109D0) and both live after it in retail's layout. */
void CL_UpdateInfoPacket( netadr_t from );
void CL_ServerInfoPacket( netadr_t from, msg_t *msg );
extern int SCR_AdjustFrom640();
extern int SCR_Init();
extern int SCR_UpdateScreen();
extern int Sys_LoadingKeepAlive();
extern int Sys_ShowIP();
void Sys_OpenURL( const char *url, int doexit );
extern int Sys_StartProcess();
extern int Z_FreeInternal();
extern int j__atol();
extern int putenv_m();

#define CA_DISCONNECTED     0
#define CA_CONNECTING       1
#define CA_CHALLENGING      2
#define CA_CONNECTED        3
#define CA_LOADING          4
#define CA_PRIMED           5
#define CA_ACTIVE           6
#define CA_CINEMATIC        7
#define CA_LOGO             8

#define MAX_VIDEO_HANDLES   16
#define RETRANSMIT_TIMEOUT  3000

#ifndef PROTOCOL_VERSION
#define PROTOCOL_VERSION    1
#endif

/* `demoName` @ 0x0087A790 is CL_Record_f's own scratch buffer, RTCW cl_main.c:289
 * `static char demoName[MAX_QPATH]`.  It is a different object from clc.demoName
 * below; RTCW carries the same pair of names. */

/* clc.demoName @ 0x015EEFC0, char[64]. `byte_15EEFFF` is base+0x3F, the terminator strncpy does not write, not a second object. */
#define clc_demoName        clc_demoName

/* cls.updateInfoString @ 0x015CA028, char[MAX_INFO_STRING]. The declaration is char[1044] because the 20-byte cls.authorizeServer (cls_authorizeServer) follows it inside the same gap. */
#define cls_updateInfoString    cls_updateInfoString

/* cls.autoupdateFilename @ 0x0155F260, char[64] -- the terminator at byte_155F29F is base+0x3F. TODO: cod1_globals.c gives it one byte. */
#define cls_autoupdateFilename  ( (char *)&byte_155F260 )

#define MAX_PINGREQUESTS    16

typedef struct ping_s
{
	netadr_t adr;
	int start;
	int time;
	char info[MAX_INFO_STRING];
} ping_t;
COD1_ASSERT_SIZE( ping_t, 1052 );

#define cl_pinglist     ( (ping_t *)unk_15CA660 )

typedef struct serverInfo_s
{
	netadr_t adr;                       /* +0x00 */
	char hostName[MAX_NAME_LENGTH];     /* +0x14 */
	char mapName[MAX_NAME_LENGTH];      /* +0x34 */
	char game[MAX_NAME_LENGTH];         /* +0x54 */
	int netType;                        /* +0x74 */
	char gameType[MAX_NAME_LENGTH];     /* +0x78 -- a string in CoD, not an int */
	int clients;                        /* +0x98 */
	int maxClients;                     /* +0x9C */
	int minPing;                        /* +0xA0 */
	int maxPing;                        /* +0xA4 */
	int ping;                           /* +0xA8 */
	qboolean visible;                   /* +0xAC */
	int allowAnonymous;                 /* +0xB0 */
	int password;                       /* +0xB4 -- CoD addition */
} serverInfo_t;
COD1_ASSERT_SIZE( serverInfo_t, 184 );

#define MAX_OTHER_SERVERS   128
#define MAX_GLOBAL_SERVERS  2048
/* `cmp ebp, 100h` at 0x0041088F in CL_ServersResponsePacket. */
#define MAX_SERVERSPERPACKET    256

#define AS_LOCAL            0
#define AS_GLOBAL           1
#define AS_FAVORITES        2

typedef struct serverAddress_s
{
	byte ip[4];
	unsigned short port;
} serverAddress_t;
COD1_ASSERT_SIZE( serverAddress_t, 6 );

#define cls_globalServerAddresses       ( (serverAddress_t *)unk_15C100C )
#define cls_numfavoriteservers          cls_numfavoriteservers           /* 0x015C400C */

#define NUM_SERVER_PORTS    4

/* (0x015CA57C - 0x015CA43C) / 0x40 in CL_CheckAutoUpdate's resolve loop; cls_autoupdateServerNames_N_ are the five objects. */
#define MAX_AUTOUPDATE_SERVERS  5

#define MAX_SERVERSTATUSREQUESTS    16

typedef struct serverStatus_s
{
	char string[BIG_INFO_STRING];       /* +0x0000 */
	netadr_t address;                   /* +0x2000 */
	int time;                           /* +0x2014 */
	int startTime;                      /* +0x2018 */
	qboolean pending;                   /* +0x201C */
	qboolean print;                     /* +0x2020 */
	qboolean retrieved;                 /* +0x2024 */
} serverStatus_t;
COD1_ASSERT_SIZE( serverStatus_t, 8232 );

#define cl_serverStatusList     ( (serverStatus_t *)&unk_15F7080 )

#define CL_SERVERSTATUS_BASE    ( (char *)&unk_15F7080 )
#define unk_15F9080             ( *(unsigned char *)( CL_SERVERSTATUS_BASE + 0x2000 ) )
#define unk_15F90A4             ( *(unsigned char *)( CL_SERVERSTATUS_BASE + 0x2024 ) )
#define unk_15FB0C0             ( *(unsigned char *)( CL_SERVERSTATUS_BASE + 0x4040 ) )
#define byte_1619300            ( (char *)( CL_SERVERSTATUS_BASE + 0x22280 ) )
#define dword_1619324           ( (unsigned int *)( CL_SERVERSTATUS_BASE + 0x222A4 ) )

#define cls_numlocalservers     cls_numlocalservers           /* 0x0155F400 */
#define cls_localServers        ( (serverInfo_t *)unk_155F404 )
#define cls_globalServers       ( (serverInfo_t *)&unk_1565008 )
#define cls_favoriteServers     ( (serverInfo_t *)unk_15C4010 )

/* ---- CL_CDDialog  0x0040E2E0 ----  [CONFIRMED] */
void __cdecl CL_CDDialog()
{
  cls_cddialog = 1;
}

/* ---- CL_AddReliableCommand  0x0040E2F0 ----  VERIFIED */
void CL_AddReliableCommand( const char *cmd ) {
	int index;

	if ( clc_reliableSequence - clc_reliableAcknowledge > MAX_RELIABLE_COMMANDS ) {
		Com_Error( ERR_DROP, "EXE_ERR_CLIENT_CMD_OVERFLOW" );
	}
	clc_reliableSequence++;
	index = clc_reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 );
	MSG_WriteReliableCommandToBuffer( cmd, MAX_STRING_CHARS,
									  &clc_reliableCommands[ MAX_STRING_CHARS * index ] );
}

/* ---- CL_ChangeReliableCommand  0x0040E340 ----  VERIFIED */
void CL_ChangeReliableCommand( void ) {
	int index, l;

	rand();
	index = clc_reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 );
	l = strlen( &clc_reliableCommands[ MAX_STRING_CHARS * index ] );
	if ( l >= MAX_STRING_CHARS - 1 ) {
		l = MAX_STRING_CHARS - 2;
	}
	clc_reliableCommands[ MAX_STRING_CHARS * index + l ] = '\n';
	clc_reliableCommands[ MAX_STRING_CHARS * index + l + 1 ] = '\0';
}

/* ---- CL_MakeMonkeyDoLaundry  0x0040E390 ----  VERIFIED */
void __cdecl CL_MakeMonkeyDoLaundry()
{
  int v0;

  if ( dword_8E3C48 )
  {
    if ( !(_BYTE)cls_framecount )
    {
      v0 = rand();
      if ( ((double)v0 * 0.000030517578 < 0.1) | __UNORDERED__((double)v0 * 0.000030517578, 0.1) )
        CL_ChangeReliableCommand();
    }
  }
}

/* ---- CL_WriteDemoMessage  0x0040E3D0 ----  VERIFIED */
void CL_WriteDemoMessage( msg_t *msg, int headerBytes )
{
	int Buffer;
	int len;

	Buffer = clc_serverMessageSequence;
	FS_Write( &Buffer, 4, clc_demofile );

	Buffer = msg->cursize - headerBytes;
	len = Buffer;
	FS_Write( &Buffer, 4, clc_demofile );

	FS_Write( msg->data + headerBytes, len, clc_demofile );
}

/* ---- CL_StopRecord_f  0x0040E430 ----  VERIFIED */
void CL_StopRecord_f( void )
{
  int Buffer;

  if ( clc_demorecording )
  {
    Buffer = -1;
    FS_Write(&Buffer, 4, clc_demofile);
    FS_Write(&Buffer, 4, clc_demofile);
    FS_FCloseFile(clc_demofile);
    clc_demofile = 0;
    clc_demorecording = 0;
    Com_Printf("Stopped demo.\n");
  }
  else
  {
    Com_Printf("Not recording a demo.\n");
  }
}

/* ---- CL_DemoFilename  0x0040E4B0 ----  VERIFIED */
void CL_DemoFilename( char *fileName, int number )
{
	if ( number < 0 || number > 9999 ) {
		Com_sprintf( fileName, 256, "demo9999.tga" );
		return;
	}

	Com_sprintf( fileName, 256, "demo%i%i%i%i",
				 number / 1000,
				 number % 1000 / 100,
				 number % 100 / 10,
				 number % 10 );
}

/* ---- CL_Record_f  0x0040E540 ----  VERIFIED */
void CL_Record_f( void ) {
	char name[MAX_OSPATH];
	byte bufData[MAX_MSGLEN];
	byte packet[4 + MAX_MSGLEN];
	msg_t buf;
	int i;
	int len;
	byte *ent;
	byte nullstate[ES_SIZE];
	char *s;
	int Buffer;

	if ( Cmd_Argc() > 2 ) {
		Com_Printf( "record <demoname>\n" );
		return;
	}

	if ( clc_demorecording ) {
		Com_Printf( "Already recording.\n" );
		return;
	}

	if ( cls_state != CA_ACTIVE ) {
		Com_Printf( "You must be in a level to record.\n" );
		return;
	}

	if ( Cmd_Argc() == 2 ) {
		s = Cmd_Argv( 1 );
		Q_strncpyz( demoName, s, 64 );
		Com_sprintf( name, sizeof( name ), "demos/%s.dm_%d", demoName,
					 PROTOCOL_VERSION );
	} else {
		int number;

		for ( number = 0; number <= 9999; number++ ) {
			CL_DemoFilename( demoName, number );
			Com_sprintf( name, sizeof( name ), "demos/%s.dm_%d", demoName,
						 PROTOCOL_VERSION );

			len = FS_ReadFile( name, NULL );
			if ( len <= 0 ) {
				break;
			}
		}
	}

	Com_Printf( "recording to %s.\n", name );
	clc_demofile = FS_FOpenFileWrite( name );
	if ( !clc_demofile ) {
		Com_Printf( "ERROR: couldn't open.\n" );
		return;
	}
	clc_demorecording = qtrue;
	Q_strncpyz( clc_demoName, demoName, sizeof( clc_demoName ) );

	clc_demowaiting = qtrue;

	MSG_Init( &buf, bufData, sizeof( bufData ) );

	MSG_WriteLong( &buf, clc_reliableSequence );

	MSG_WriteByte( &buf, svc_gamestate );
	MSG_WriteLong( &buf, clc_serverCommandSequence );

	for ( i = 0; i < MAX_CONFIGSTRINGS; i++ ) {
		if ( !cl_gameState_stringOffsets[i] ) {
			continue;
		}
		s = cl_gameState_stringData + cl_gameState_stringOffsets[i];
		MSG_WriteByte( &buf, svc_configstring );
		MSG_WriteShort( &buf, i );
		MSG_WriteBigString( s, &buf );
	}

	Com_Memset( nullstate, 0, sizeof( nullstate ) );
	for ( i = 0; i < MAX_GENTITIES; i++ ) {
		ent = &byte_147D134[ES_SIZE * i];
		if ( !*(int *)( ent + ES_NUMBER ) ) {
			continue;
		}
		MSG_WriteByte( &buf, svc_baseline );
		MSG_WriteDeltaEntity( &buf, nullstate, ent, qtrue );
	}

	MSG_WriteByte( &buf, svc_EOF );

	MSG_WriteLong( &buf, clc_clientNum );
	MSG_WriteLong( &buf, clc_checksumFeed );

	MSG_WriteByte( &buf, svc_EOF );

	*(int *)packet = *(int *)buf.data;
	len = MSG_WriteBitsCompress( buf.data + 4, buf.cursize - 4, packet + 4 ) + 4;

	Buffer = clc_serverMessageSequence;
	FS_Write( &Buffer, 4, clc_demofile );

	Buffer = len;
	FS_Write( &Buffer, 4, clc_demofile );

	FS_Write( packet, len, clc_demofile );

}

/* ---- CL_DemoCompleted  0x0040E920 ----  VERIFIED */
void CL_DemoCompleted( void ) {
	if ( timedemo && timedemo->integer ) {
		int time;

		time = Sys_Milliseconds() - clc_timeDemoStart;
		if ( time > 0 ) {
			Com_Printf( "%i frames, %3.1f seconds: %3.1f fps\n",
						*(int *)clc_timeDemoFrames,
						time / 1000.0,
						*(int *)clc_timeDemoFrames * 1000.0 / time );
		}
	}

	CL_Disconnect( qtrue );
	CL_NextDemo();
}

/* ---- CL_ReadDemoMessage  0x0040E9C0 ----  [CONFIRMED] */
void __cdecl CL_ReadDemoMessage()
{
  int v0;
  msg_t msg;
  int Buffer;
  char v3[0x4000];
  unsigned int v4;
  unsigned int retaddr;

  v4 = retaddr ^ _security_cookie;
  if ( !clc_demofile )
    goto LABEL_11;
  if ( FS_Read(&Buffer, 4, clc_demofile) != 4 )
    goto LABEL_11;
  clc_serverMessageSequence = Buffer;
  if ( msgInit == qfalse )
    MSG_initHuffman();
  memset(&msg, 0, sizeof(msg));
  msg.data = (unsigned __int8 *)&v3;
  msg.maxsize = 0x4000;
  if ( FS_Read(&msg.cursize, 4, clc_demofile) != 4 || msg.cursize == -1 )
    goto LABEL_11;
  if ( msg.cursize > msg.maxsize )
    Com_Error(ERR_DROP, &byte_5676E8);
  v0 = FS_Read(msg.data, msg.cursize, clc_demofile);
  if ( v0 != msg.cursize )
  {
    Com_Printf("Demo file was truncated.\n");
LABEL_11:
    CL_DemoCompleted();
    return;
  }
  clc_lastPacketTime = cls_realtime;
  msg.readcount = 0;
  clc_reliableAcknowledge = MSG_ReadLong(&msg);
  if ( clc_reliableAcknowledge >= clc_reliableSequence - 64 )
    CL_ParseServerMessage((int)&msg);
  else
    clc_reliableAcknowledge = clc_reliableSequence;
}

/* ---- CL_PlayDemo_f  0x0040EB40 ----  VERIFIED */
void CL_PlayDemo_f( void ) {
	char name[MAX_OSPATH], extension[32];
	char *arg;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "playdemo <demoname>\n" );
		return;
	}

	if ( com_sv_running->integer ) {
		Com_Printf( "listen server cannot play a demo.\n" );
		return;
	}

	CL_Disconnect( qtrue );

	arg = Cmd_Argv( 1 );
	Com_sprintf( extension, sizeof( extension ), ".dm_%d", PROTOCOL_VERSION );
	if ( !Q_stricmp( arg + strlen( arg ) - strlen( extension ), extension ) ) {
		Com_sprintf( name, sizeof( name ), "demos/%s", arg );
	} else {
		Com_sprintf( name, sizeof( name ), "demos/%s.dm_%d", arg, PROTOCOL_VERSION );
	}

	fs_loadingMode = 1;
	FS_FOpenFileRead_Internal( name, &clc_demofile, qtrue, 0 );
	if ( !clc_demofile ) {
		Com_Error( ERR_DROP, va( "EXE_ERR_CANT_WRITE" "\x15" "%s", name ) );
		return;
	}
	Q_strncpyz( clc_demoName, Cmd_Argv( 1 ), sizeof( clc_demoName ) );

	Con_Close();

	clc_demoplaying = qtrue;
	cls_state = CA_CONNECTED;
	Q_strncpyz( cls_servername, Cmd_Argv( 1 ), sizeof( cls_servername ) );

	while ( cls_state >= CA_CONNECTED && cls_state < CA_PRIMED ) {
		CL_ReadDemoMessage();
	}
	clc_firstDemoFrameSkipped = qfalse;
}

/* ---- CL_StartDemoLoop  0x0040ED60 ----  VERIFIED */
void CL_StartDemoLoop( void ) {
	Cbuf_AddText( "d1\n" );
	cls_keyCatchers = 0;
}

/* ---- CL_NextDemo  0x0040ED80 ----  VERIFIED */
void CL_NextDemo( void ) {
	char v[MAX_STRING_CHARS];

	Q_strncpyz( v, Cvar_VariableString( "nextdemo" ), sizeof( v ) );
	v[MAX_STRING_CHARS - 1] = 0;
	Com_DPrintf( "CL_NextDemo: %s\n", v );
	if ( !v[0] ) {
		return;
	}

	Cvar_Set2( "nextdemo", "", qtrue );
	Cbuf_AddText( v );
	Cbuf_AddText( "\n" );
	Cbuf_Execute();
}

/* ---- CL_ShutdownAll  0x0040EE30 ----  [CONFIRMED] */
void __cdecl CL_ShutdownAll()
{
  CL_ShutdownCGame();
  CL_ShutdownUI();
  if ( com_serverEndpoint )
    com_serverEndpoint(0);
  cls_loadingPlaque = 0;
  cls_rendererStarted = 0;
}

/* ---- CL_MapLoading  0x0040EE60 ----  VERIFIED */
void CL_MapLoading( void ) {
	if ( !com_cl_running->integer ) {
		return;
	}

	Con_Close();
	cls_keyCatchers = 0;

	if ( cls_state >= CA_CONNECTED && !Q_stricmp( cls_servername, "localhost" ) ) {
		cls_state = CA_CONNECTED;
		Com_Memset( cls_updateInfoString, 0, MAX_INFO_STRING );
		Com_Memset( clc_serverMessage, 0, 256 );
		Com_Memset( cl_gameState_stringOffsets, 0, GS_SIZE );  /* 0x01434A7C */
		clc_lastPacketSentTime = -9999;
		SCR_UpdateScreen();
	} else {
		Cvar_Set2( "nextmap", "", qtrue );
		CL_Disconnect( qtrue );
		Q_strncpyz( cls_servername, "localhost", sizeof( cls_servername ) );
		cls_state = CA_CHALLENGING;
		cls_keyCatchers = 0;
		SCR_UpdateScreen();
		clc_connectTime = -RETRANSMIT_TIMEOUT;
		NET_StringToAdr( cls_servername, (netadr_t *)clc_serverAddress );

		CL_CheckForResend();
	}

	mss_globalFadeTarget = 0;
	mss_globalFadeRate = -mss_globalFadeCur;
	MSS_StopSounds( 0 );
}

/* ---- CL_ClearState  0x0040EF70 ---- */
void CL_ClearState( void )
{
	Com_Memset( &cl_active, 0, 1231064 );                     /* 0x01432960 <- real extent, cl_refstorage.c */
	Com_Memset( &cl_snap_valid, 0, 8448 );                    /* 0x01432964 <- real extent, cl_refstorage.c */
	Com_Memset( cl_snap_snapFlags, 0, 4 );                    /* 0x01432968 */
	Com_Memset( &cl_snap_serverTime, 0, 4 );                  /* 0x0143296C */
	Com_Memset( cl_snap_messageNum, 0, 4 );                   /* 0x01432970 */
	Com_Memset( &cl_snap_deltaNum, 0, 4 );                    /* 0x01432974 */
	Com_Memset( &cl_snap_ping, 0, 4 );                        /* 0x01432978 */
	Com_Memset( &cl_snap_ps_commandTime, 0, 4 );              /* 0x01432980 */
	Com_Memset( &dword_1432984, 0, 4 );                       /* 0x01432984 */
	Com_Memset( &dword_143298C, 0, 4 );                       /* 0x0143298C */
	Com_Memset( &dword_14329C8, 0, 4 );                       /* 0x014329C8 */
	Com_Memset( &dword_1432A00, 0, 4 );                       /* 0x01432A00 */
	Com_Memset( &dword_1432A2C, 0, 4 );                       /* 0x01432A2C */
	Com_Memset( &flt_1432A50, 0, 4 );                         /* 0x01432A50 */
	Com_Memset( &dword_1434A54, 0, 4 );                       /* 0x01434A54 */
	Com_Memset( &dword_1434A5C, 0, 4 );                       /* 0x01434A5C */
	Com_Memset( &cl_serverTime, 0, 4 );                       /* 0x01434A64 */
	Com_Memset( &cl_oldServerTime, 0, 4 );                    /* 0x01434A68 */
	Com_Memset( &cl_oldFrameServerTime, 0, 4 );                       /* 0x01434A6C */
	Com_Memset( &cl_serverTimeDelta, 0, 4 );                  /* 0x01434A70 */
	Com_Memset( &cl_extrapolatedSnapshot, 0, 4 );             /* 0x01434A74 */
	Com_Memset( &cl_newSnapshots, 0, 4 );                     /* 0x01434A78 */
	Com_Memset( cl_gameState_stringOffsets, 0, 24196 );       /* 0x01434A7C <- real extent, cl_refstorage.c */
	Com_Memset( &cl_gameState_stringOffsets_CS_SYSTEMINFO_, 0, 4 ); /* 0x01434A80 */
	Com_Memset( cl_gameState_stringData, 0, 16000 );          /* 0x01436A7C */
	Com_Memset( &cl_gameState_dataCount, 0, 4 );              /* 0x0143A8FC */
	Com_Memset( cl_mapname, 0, 64 );                         /* 0x0143A900 */
	Com_Memset( &cl_parseEntitiesNum, 0, 4 );                 /* 0x0143A940 */
	Com_Memset( &cl_parseClientsNum, 0, 4 );                  /* 0x0143A944 */
	Com_Memset( cl_mouseDx, 0, 44 );                       /* 0x0143A948 */
	Com_Memset( &dword_143A94C, 0, 4 );                       /* 0x0143A94C */
	Com_Memset( cl_mouseDy, 0, 36 );                       /* 0x0143A950 */
	Com_Memset( &dword_143A954, 0, 4 );                       /* 0x0143A954 */
	Com_Memset( &cl_mouseIndex, 0, 4 );                       /* 0x0143A958 */
	Com_Memset( cl_joystickAxis, 0, 24 );                       /* 0x0143A95C */
	Com_Memset( &dword_143A960, 0, 4 );                       /* 0x0143A960 */
	Com_Memset( &dword_143A964, 0, 4 );                       /* 0x0143A964 */
	Com_Memset( &cgameUserCmdValue, 0, 4 );                   /* 0x0143A974 */
	Com_Memset( &cgameUserCmdInShellshock, 0, 4 );            /* 0x0143A978 */
	Com_Memset( &cgameSensitivity, 0, 4 );                    /* 0x0143A97C */
	Com_Memset( &dword_143A980, 0, 4 );                       /* 0x0143A980 */
	Com_Memset( &dword_143A984, 0, 4 );                       /* 0x0143A984 */
	Com_Memset( &cgameClientLerpOrigin_x, 0, 4 );             /* 0x0143A988 */
	Com_Memset( &cgameClientLerpOrigin_y, 0, 4 );             /* 0x0143A98C */
	Com_Memset( &cgameClientLerpOrigin_z, 0, 4 );             /* 0x0143A990 */
	Com_Memset( &cgameUserAim_x, 0, 4 );                      /* 0x0143A994 */
	Com_Memset( &cgameUserAim_y, 0, 4 );                      /* 0x0143A998 */
	Com_Memset( &cgameUserAim_z, 0, 4 );                      /* 0x0143A99C */
	Com_Memset( &cl_viewanglesPitch, 0, 4 );                       /* 0x0143A9A0 */
	Com_Memset( &cl_viewanglesYaw, 0, 4 );                       /* 0x0143A9A4 */
	Com_Memset( &cl_viewanglesRoll, 0, 4 );                         /* 0x0143A9A8 */
	Com_Memset( &cl_serverId, 0, 4 );                       /* 0x0143A9AC */
	Com_Memset( stru_143A9B0, 0, 1536 );                      /* 0x0143A9B0 */
	Com_Memset( &cl_cmdNumber, 0, 4 );                        /* 0x0143AFB0 */
	Com_Memset( dword_143AFB4, 0, 384 );                      /* 0x0143AFB4 */
	Com_Memset( dword_143AFB8, 0, 380 );                      /* 0x0143AFB8 */
	Com_Memset( dword_143AFBC, 0, 376 );                      /* 0x0143AFBC */
	Com_Memset( cl_snapshots, 0, 270336 );                    /* 0x0143B134 */
	Com_Memset( byte_147D134, 0, 245760 );                    /* 0x0147D134 */
	Com_Memset( byte_14B9134, 0, 491520 );                    /* 0x014B9134 */
	Com_Memset( byte_1531134, 0, 188676 );                    /* 0x01531134 */
}

/* ---- CL_Disconnect  0x0040EF90 ----  [HIGH] */
void __cdecl CL_Disconnect(qboolean showMainMenu)
{
  int v1;

  if ( com_cl_running )
  {
    if ( com_cl_running->integer )
    {
      Cvar_Set2("r_uiFullScreen", "1", qtrue);
      if ( clc_demorecording )
        CL_StopRecord_f();
      if ( clc_download )
      {
        FS_FCloseFile(clc_download);
        clc_download = 0;
      }
      cls_downloadName[0] = 0;
      cls_downloadTempName[0] = 0;
      Cvar_Set2("cl_downloadName", &empty_string, qtrue);
      autoupdateStarted = 0;
      byte_155F260 = 0;
      if ( clc_demofile )
      {
        FS_FCloseFile(clc_demofile);
        clc_demofile = 0;
      }
      if ( uivm )
      {
        if ( showMainMenu )
          VM_Call(uivm, UI_SET_ACTIVE_MENU, UIMENU_NONE);
      }
      if ( (unsigned int)CL_handle < MAX_VIDEO_HANDLES )
      {
        CIN_StopCinematic(CL_handle);
        MSS_StopSounds(0);
        CL_handle = -1;
      }
      if ( cls_state >= CA_CONNECTED )
      {
        CL_AddReliableCommand("disconnect");
        CL_WritePacket();
        CL_WritePacket();
        CL_WritePacket();
      }
      CL_ClearState();

      Com_Memset( &clc_clientNum, 0, 4 );                         /* 0x015CE860 */
      Com_Memset( &clc_lastPacketSentTime, 0, 4 );                /* 0x015CE864 */
      Com_Memset( &clc_lastPacketTime, 0, 4 );                         /* 0x015CE868 clc.lastPacketTime */
      Com_Memset( clc_serverAddress, 0, 20 );                     /* 0x015CE86C */
      Com_Memset( &clc_connectTime, 0, 4 );                       /* 0x015CE880 */
      Com_Memset( &clc_connectPacketCount, 0, 4 );                /* 0x015CE884 */
      Com_Memset( clc_serverMessage, 0, 260 );                    /* 0x015CE888 */
      Com_Memset( byte_15CE987, 0, 5 );                           /* 0x015CE987 */
      Com_Memset( clc_challenge, 0, 4 );                          /* 0x015CE988 */
      Com_Memset( &clc_checksumFeed, 0, 4 );                      /* 0x015CE98C */
      Com_Memset( &dword_15CE990, 0, 4 );                         /* 0x015CE990 */
      Com_Memset( &clc_reliableSequence, 0, 4 );                  /* 0x015CE994 */
      Com_Memset( &clc_reliableAcknowledge, 0, 4 );               /* 0x015CE998 */
      Com_Memset( clc_reliableCommands, 0, 65536 );               /* 0x015CE99C <- real extent, cl_refstorage.c */
      Com_Memset( byte_15CE99D, 0, 65535 );                       /* 0x015CE99D */
      Com_Memset( word_15CEC4E, 0, 64846 );                       /* 0x015CEC4E */
      Com_Memset( &clc_serverMessageSequence, 0, 4 );             /* 0x015DE99C */
      Com_Memset( &clc_serverCommandSequence, 0, 4 );             /* 0x015DE9A0 */
      Com_Memset( &clc_lastExecutedServerCommand, 0, 4 );         /* 0x015DE9A4 */
      Com_Memset( clc_serverCommands, 0, 65536 );                 /* 0x015DE9A8 */
      Com_Memset( &clc_download, 0, 4 );                          /* 0x015EE9A8 */
      Com_Memset( &cls_downloadTempName, 0, 256 );                /* 0x015EE9AC <- real extent, cl_refstorage.c */
      Com_Memset( &cls_downloadName, 0, 260 );                    /* 0x015EEAAC <- real extent, cl_refstorage.c */
      Com_Memset( &byte_15EEBAB, 0, 1 );                          /* 0x015EEBAB */
      Com_Memset( cls_downloadBlock, 0, 4 );                      /* 0x015EEBB0 */
      Com_Memset( &cls_downloadCount, 0, 4 );                     /* 0x015EEBB4 */
      Com_Memset( &clc_downloadSize, 0, 4 );                      /* 0x015EEBB8 */
      Com_Memset( clc_downloadList, 0, 1024 );                    /* 0x015EEBBC */
      Com_Memset( &byte_15EEFBB, 0, 1 );                          /* 0x015EEFBB */
      Com_Memset( &cls_downloadRestart, 0, 4 );                   /* 0x015EEFBC clc.downloadRestart */
      Com_Memset( clc_demoName, 0, 64 );                          /* 0x015EEFC0 */
      Com_Memset( &byte_15EEFFF, 0, 1 );                          /* 0x015EEFFF */
      Com_Memset( &clc_demorecording, 0, 4 );                     /* 0x015EF000 */
      Com_Memset( &clc_demoplaying, 0, 4 );                       /* 0x015EF004 */
      Com_Memset( &clc_demowaiting, 0, 4 );                       /* 0x015EF008 */
      Com_Memset( &clc_firstDemoFrameSkipped, 0, 4 );             /* 0x015EF00C */
      Com_Memset( &clc_demofile, 0, 4 );                          /* 0x015EF010 */
      Com_Memset( clc_timeDemoFrames, 0, 12 );                    /* 0x015EF014 */
      Com_Memset( &clc_timeDemoStart, 0, 4 );                     /* 0x015EF018 */
      Com_Memset( &clc_timeDemoBaseTime, 0, 4 );                  /* 0x015EF01C */
      Com_Memset( chan, 0, 32832 );                               /* 0x015EF020 clc.netchan */

      cls_state = CA_DISCONNECTED;
      cl_connectedToPureServer = 0;
    }
  }
}

/* CL_ForwardCommandToServer (0x0040F0C0) is defined further down this unit. */
#if 0
void __cdecl CL_ForwardCommandToServer(const char *string)
{
  char *v1;

  if ( cmd_argc )
  {
    v1 = cmd_argv[0];
    if ( *cmd_argv[0] == 45 )
      return;
  }
  else
  {
    v1 = &empty_string;
  }
  if ( clc_demoplaying || *(int *)cls_state < 3 || *v1 == 43 )
  {
    Com_Printf("Unknown command \"%s\"\n", v1);
  }
  else
  {
    if ( cmd_argc <= 1 )
      string = v1;
    CL_AddReliableCommand(string);
  }
}
#endif

static netadr_t cls_updateServer;                       /* 0x015C9C14 */
static char cls_updateChallenge[MAX_TOKEN_CHARS];       /* 0x015C9C28 */

#define UPDATE_SERVER_NAME  "codmotd.activision.com"
#define PORT_UPDATE         20520
#define AUTHORIZE_SERVER_NAME   "codauthorize.activision.com"

/* ---- CL_RequestMotd  0x0040F110 ----  VERIFIED */
void CL_RequestMotd( void ) {
	char info[MAX_INFO_STRING];

	if ( !cl_motd->integer ) {
		return;
	}
	Com_Printf( "Resolving %s\n", UPDATE_SERVER_NAME );
	if ( !NET_StringToAdr( UPDATE_SERVER_NAME, &cls_updateServer ) ) {
		Com_Printf( "Couldn't resolve address\n" );
		return;
	}

	cls_updateServer.port = BigShort( PORT_UPDATE );
	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", UPDATE_SERVER_NAME,
				cls_updateServer.ip[0], cls_updateServer.ip[1],
				cls_updateServer.ip[2], cls_updateServer.ip[3],
				BigShort( cls_updateServer.port ) );

	info[0] = 0;

	Com_sprintf( cls_updateChallenge, sizeof( cls_updateChallenge ), "%i", rand() );

	Info_SetValueForKey( info, "challenge", cls_updateChallenge );
	Info_SetValueForKey( info, "renderer", cls_glconfig );
	Info_SetValueForKey( info, "version", ( (cvar_t *)com_shortversion )->string );

	NET_OutOfBandPrint( NS_CLIENT, cls_updateServer, "getmotd \"%s\"\n", info );
}

/* ---- CL_RequestAuthorization  0x0040F260 ----  VERIFIED */
void CL_RequestAuthorization( void ) {
	char nums[64];
	int i, j, l;
	cvar_t *fs;

	if ( CL_CDKeyValidate( cl_cdkey, cl_cdkeychecksum ) == qfalse ) {
		Com_Error( ERR_DROP, "EXE_ERR_INVALID_CD_KEY" );
		return;
	}

	if ( !( (netadr_t *)cls_authorizeServer )->port ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, (netadr_t *)cls_authorizeServer ) ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}

		( (netadr_t *)cls_authorizeServer )->port = BigShort( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
					( (netadr_t *)cls_authorizeServer )->ip[0],
					( (netadr_t *)cls_authorizeServer )->ip[1],
					( (netadr_t *)cls_authorizeServer )->ip[2],
					( (netadr_t *)cls_authorizeServer )->ip[3],
					BigShort( ( (netadr_t *)cls_authorizeServer )->port ) );
	}
	if ( ( (netadr_t *)cls_authorizeServer )->type == NA_BAD ) {
		return;
	}

	if ( Cvar_VariableValue( "fs_restrict" ) ) {
		Q_strncpyz( nums, "demo", sizeof( nums ) );
	} else {
		j = 0;
		l = strlen( cl_cdkey );
		if ( l > 32 ) {
			l = 32;
		}
		for ( i = 0; i < l; i++ ) {
			if ( ( cl_cdkey[i] >= '0' && cl_cdkey[i] <= '9' )
				 || ( cl_cdkey[i] >= 'a' && cl_cdkey[i] <= 'z' )
				 || ( cl_cdkey[i] >= 'A' && cl_cdkey[i] <= 'Z' ) ) {
				nums[j] = cl_cdkey[i];
				j++;
			}
		}
		nums[j] = 0;
	}

	fs = Cvar_Get( "cl_anonymous", "0", CVAR_INIT | CVAR_SYSTEMINFO );
	NET_OutOfBandPrint( NS_CLIENT, *(netadr_t *)cls_authorizeServer,
						va( "getKeyAuthorize %i %s", fs->integer, nums ) );
}

/* ---- CL_ForwardToServer_f  0x0040F450 ----  [CONFIRMED] */
void __cdecl CL_ForwardToServer_f()
{
  char *v0;

  if ( cls_state != CA_ACTIVE || clc_demoplaying )
  {
    Com_Printf("Not connected to a server.\n");
  }
  else if ( cmd_argc > 1 )
  {
    v0 = Cmd_Args();
    CL_AddReliableCommand(v0);
  }
}

/* ---- CL_Setenv_f  0x0040F490 ----  [CONFIRMED] */
void __cdecl CL_Setenv_f()
{
  signed __int32 v0;
  char *v1;
  char *v2;
  int v4;
  bool v5;
  char *v6;
  char *v7;
  unsigned int v8;
  _BYTE *v9;
  char *v11;
  char *v13;
  char *v14;
  char *v15;
  _BYTE v16[15];
  char v17;
  CHAR MultiByteStr[1024];
  unsigned int v19;
  unsigned int retaddr;

  v0 = cmd_argc;
  v19 = retaddr ^ _security_cookie;
  if ( cmd_argc <= 2 )
  {
    if ( cmd_argc == 2 )
    {
      v13 = getenv(cmd_argv[1]);
      if ( v13 )
      {
        v14 = &empty_string;
        if ( (unsigned int)cmd_argc > 1 )
          v14 = cmd_argv[1];
        Com_Printf("%s=%s\n", v14, v13);
      }
      else
      {
        v15 = &empty_string;
        if ( (unsigned int)cmd_argc > 1 )
          v15 = cmd_argv[1];
        Com_Printf("%s undefined\n", v15);
      }
    }
  }
  else
  {
    v1 = &empty_string;
    if ( (unsigned int)cmd_argc > 1 )
      v1 = cmd_argv[1];
    strcpy(MultiByteStr, v1);
    v2 = &v17;
    while ( *++v2 )
      ;
    v4 = 2;
    *(_WORD *)v2 = 61;
    if ( v0 > 2 )
    {
      v5 = (unsigned int)v0 > 2;
      do
      {
        v6 = &empty_string;
        if ( v5 )
          v6 = cmd_argv[v4];
        v7 = v6;
        v8 = strlen(v6) + 1;
        v9 = &v16[15];
        while ( *++v9 )
          ;
        qmemcpy(v9, v7, v8);
        v11 = &v16[15];
        while ( *++v11 )
          ;
        v5 = ++v4 < (unsigned int)v0;
        strcpy(v11, " ");
      }
      while ( v4 < v0 );
    }
    putenv_m(MultiByteStr);
  }
}

/* ---- CL_Disconnect_f  0x0040F5F0 ----  VERIFIED */
void CL_Disconnect_f( void ) {
	if ( CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES ) {
		CIN_StopCinematic( CL_handle );
		MSS_StopSounds( 0 );
		CL_handle = -1;
	}

	if ( cls_state != CA_DISCONNECTED && cls_state != CA_CINEMATIC
		 && cls_state != CA_LOGO ) {
		Com_Error( ERR_DISCONNECT, "EXE_DISCONNECTED_FROM_SERVER" );
	}
}

/* ---- CL_Reconnect_f  0x0040F640 ----  VERIFIED */
void CL_Reconnect_f( void )
{
	if ( !strlen( cls_servername ) || !strcmp( cls_servername, "localhost" ) ) {
		Com_Printf( "Can't reconnect to localhost.\n" );
		return;
	}

	Cbuf_AddText( va( "connect %s\n", cls_servername ) );
}

/* ---- CL_Connect_f  0x0040F6A0 ----  [HIGH] */
void __cdecl CL_Connect_f( void )
{
	const char	*server;
	netadr_t	*adr = (netadr_t *) clc_serverAddress;

	if ( cmd_argc != 2 ) {
		Com_Printf( "usage: connect [server]\n" );
		return;
	}

	MSS_StopSounds( 0 );
	Cvar_Set2( "r_uiFullScreen", "0", qtrue );
	CL_RequestMotd();
	clc_serverMessage[0] = 0;

	server = ( cmd_argc > 1 ) ? cmd_argv[1] : (char *) empty_string;

	if ( com_sv_running->integer && !memcmp( server, "localhost", 10 ) ) {
		Com_Shutdown( "EXE_SERVERQUIT" );
	}

	Cvar_Set2( "sv_killserver", "1", qtrue );
	SV_Frame( 0 );

	CL_Disconnect( qtrue );
	Con_Close();

	strncpy( cls_servername, server, 0xFF );
	byte_155F3CB = 0;

	if ( !NET_StringToAdr( cls_servername, adr ) ) {
		Com_Printf( "Bad server address\n" );
		cls_state = CA_DISCONNECTED;
		return;
	}
	if ( !adr->port ) {
		adr->port = BigShort( 28960 );
	}
	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", cls_servername,
	            adr->ip[0], adr->ip[1], adr->ip[2], adr->ip[3],
	            BigShort( adr->port ) );

	if ( adr->type != NA_LOOPBACK && adr->type != NA_BOT ) {
		if ( CL_CDKeyValidate( cl_cdkey, cl_cdkeychecksum ) == qfalse ) {
			Com_Error( ERR_DROP, "EXE_ERR_INVALID_CD_KEY" );
			return;
		}
	}
	if ( adr->type == NA_LOOPBACK || adr->type == NA_BOT ) {
		cls_state = CA_CHALLENGING;
	} else {
		cls_state = CA_CONNECTING;
	}

	cls_keyCatchers = 0;
	clc_connectTime = -99999;
	clc_connectPacketCount = 0;
	Cvar_Set2( "cl_currentServerAddress", server, qtrue );
}

/* ---- CL_Rcon_f  0x0040F880 ----  VERIFIED */
void CL_Rcon_f( void ) {
	char message[MAX_STRING_CHARS];
	netadr_t to;
	const char *src;
	const char *arg;
	int i, j, room, len;
	qboolean quote;

	if ( !rconPassword->string ) {
		Com_Printf( "You must set 'rcon_password' before\n"
					"issuing an rcon command.\n" );
		return;
	}

	message[0] = -1;
	message[1] = -1;
	message[2] = -1;
	message[3] = -1;
	message[4] = 0;
	len = 4;

	src = "rcon ";
	while ( len < MAX_STRING_CHARS && *src ) {
		message[len++] = *src++;
	}

	src = rconPassword->string;
	while ( len < MAX_STRING_CHARS && *src ) {
		message[len++] = *src++;
	}

	for ( i = 1; i < cmd_argc; i++ ) {
		src = " ";
		while ( len < MAX_STRING_CHARS && *src ) {
			message[len++] = *src++;
		}

		arg = Cmd_Argv( i );

		quote = qfalse;
		if ( !arg[0] ) {
			quote = qtrue;
		} else {
			room = MAX_STRING_CHARS - len;
			for ( j = 0; j < room; j++ ) {
				if ( !arg[j] ) {
					break;
				}
				if ( arg[j] <= ' ' ) {
					quote = qtrue;
					break;
				}
			}
		}

		if ( quote ) {
			if ( len >= MAX_STRING_CHARS ) {
				continue;
			}
			message[len++] = '"';
		}

		src = arg;
		while ( len < MAX_STRING_CHARS && *src ) {
			message[len++] = *src++;
		}

		if ( quote && len < MAX_STRING_CHARS ) {
			message[len++] = '"';
		}
	}

	if ( len == MAX_STRING_CHARS ) {
		Com_Printf( "rcon command too long\n" );
		return;
	}
	message[len] = 0;

	if ( cls_state >= CA_CONNECTED ) {
		to = ( (netchan_t *)chan )->remoteAddress;
	} else {
		if ( !strlen( rconAddress->string ) ) {
			Com_Printf( "You must either be connected,\n"
						"or set the 'rconAddress' cvar\n"
						"to issue rcon commands\n" );
			return;
		}
		NET_StringToAdr( rconAddress->string, &to );
		if ( to.port == 0 ) {
			to.port = BigShort( PORT_SERVER );
		}
	}

	CL_Netchan_SendOOBPacket( strlen( message ) + 1, message, to );
}

/* ---- CL_SendPureChecksums  0x0040FAC0 ----  VERIFIED */
void CL_SendPureChecksums( void ) {
	const char *pChecksums;
	char cMsg[MAX_INFO_VALUE];
	int i;

	pChecksums = FS_ReferencedPakPureChecksums();

	Com_sprintf( cMsg, sizeof( cMsg ), "Va " );
	Q_strcat( cMsg, sizeof( cMsg ), pChecksums );
	for ( i = 0; i < 2; i++ ) {
		cMsg[i] += 13 + ( i * 2 );
	}
	CL_AddReliableCommand( cMsg );
}

/* ---- CL_ResetPureClientAtServer  0x0040FBC0 ----  [CONFIRMED] */
void __cdecl CL_ResetPureClientAtServer()
{
	CL_AddReliableCommand( "vdr" );
}

/* ---- CL_Vid_Restart_f  0x0040FBE0 ---- */
void __cdecl CL_Vid_Restart_f( void )
{
	extern cvar_t *fs_gamedirvar;
	extern int CL_SaveCgameState( int size, void *buffer );   /* 0x00404F80 */

	void *hunkBuf;
	int   maxSize;
	int   savedSize  = 0;
	void *savedState = NULL;

	if ( com_sv_running->integer ) {
		Com_Printf( "Listen server cannot video restart.\n" );
		return;
	}

	if ( cgvm ) {
		hunkBuf = Hunk_AllocateTempMemoryInternal( 0 );
		maxSize = *(int *)hunk_totalSize
		        - *(int *)hunk_highTemp
		        - *(int *)hunk_temp_temp;

		savedSize  = CL_SaveCgameState( maxSize, hunkBuf );
		savedState = Z_MallocInternal( savedSize );
		memcpy( savedState, hunkBuf, savedSize );
		Hunk_FreeTempMemory( hunkBuf );
	}

	Cvar_Set2( "com_expectedhunkusage", "-1", qtrue );
	MSS_StopSounds( 1 );

	CL_ShutdownCGame();
	CL_ShutdownUI();
	CL_ShutdownRef();

	CL_ResetPureClientAtServer();
	FS_ClearPakReferences( 0 );

	CL_ShutdownCGame();
	CL_ShutdownUI();
	SV_ShutdownGameProgs();
	CIN_CloseAllVideos();
	Hunk_ClearToStart();
	VM_Clear();

	Cvar_Get( "cl_language", "0", 33 );
	Cvar_Get( "cl_languagetranslate", "1", 32 );
	Cvar_Get( "fs_ignoreLozalized", "0", 544 );

	if ( !com_sv_running->integer
	  && ( fs_gamedirvar->modified || clc_checksumFeed != fs_checksumFeed ) ) {
		FS_Restart( clc_checksumFeed );
	}
	SEH_UpdateLanguageInfo();

	cls_rendererStarted = 0;
	cls_loadingPlaque = 0;

	Cvar_Set2( "cl_paused", "0", qtrue );
	CL_InitRef();
	CL_StartHunkUsers();

	if ( cls_state > CA_CONNECTED && cls_state != CA_CINEMATIC && cls_state != CA_LOGO ) {
		CL_InitCGame();
		CL_SendPureChecksums();
	}

	if ( savedState ) {
		if ( cgvm ) {
			CL_RestoreCgameState( savedSize, savedState );
		}
		free( savedState );
	}
}
#if 0
void __cdecl CL_Vid_Restart_f()
{
  const char *v0;
  size_t v1;
  char *v2;
  char *v3;
  int v4;
  int v5;
  int *v6;
  int *v7;
  unsigned int v8;
  char *v9;
  int v10;
  int *Block;

  v1 = 0;
  Block = 0;
  if ( com_sv_running->integer )
  {
    Com_Printf("Listen server cannot video restart.\n");
  }
  else
  {
    if ( cgvm )
    {
      Hunk_AllocateTempMemoryInternal(0);
      v3 = v2;
      v4 = *(_DWORD *)hunk_totalSize - *(_DWORD *)hunk_highTemp - *(_DWORD *)hunk_temp_temp;
      v5 = cgame_SaveExportTable(v2, v4);
      v1 = v5 + VM_Call(cgvm, 17, &v3[v5], v4 - v5);
      v6 = Z_MallocInternal(v1);
      qmemcpy(v6, v3, 4 * (v1 >> 2));
      v7 = &v6[v1 >> 2];
      v8 = v1 & 3;
      Block = v6;
      qmemcpy(v7, &v3[4 * (v1 >> 2)], v8);
      v0 = (char *)v7 + v8;
      Hunk_FreeTempMemory(v3);
    }
    Cvar_Set2("com_expectedhunkusage", "-1", qtrue);
    MSS_StopSounds(1);
    CL_ShutdownCGame();
    CL_ShutdownUI();
    /* Retail inlines CL_ShutdownRef here (0x0040FC90): endpoint(1), clear all
     * 57 slots, StatMon_Reset.  /vid_restart is the one path that reaches this. */
    CL_ShutdownRef();
    v9 = va(off_55EAE4);
    CL_AddReliableCommand(v9);
    FS_ClearPakReferences(v10);
    CL_ShutdownCGame();
    CL_ShutdownUI();
    SV_ShutdownGameProgs();
    CIN_CloseAllVideos();
    Hunk_ClearToStart();
    VM_Clear();
    Cvar_Get("cl_language", "0", 33);
    Cvar_Get("cl_languagetranslate", "1", 32);
    Cvar_Get("fs_ignoreLozalized", "0", 544);
    if ( !com_sv_running->integer && (*(_DWORD *)(fs_gamedirvar + 20) || clc_checksumFeed != fs_checksumFeed) )
      FS_Restart(clc_checksumFeed);
    SEH_UpdateLanguageInfo();
    cls_rendererStarted = 0;
    cls_loadingPlaque = 0;
    Cvar_Set2("cl_paused", "0", qtrue);
    CL_InitRef();
    CL_StartHunkUsers();
    if ( *(int *)cls_state > 3 && *(_DWORD *)cls_state != 7 && *(_DWORD *)cls_state != 8 )
    {
      CL_InitCGame((const char *)v1, v0);
      CL_SendPureChecksums();
    }
    if ( Block )
    {
      if ( cgvm )
        CL_RestoreCgameState(v1, Block);
      free(Block);
    }
  }
}
#endif

/* ---- CL_Snd_Restart_f  0x0040FDF0 ----  [CONFIRMED] */
void __cdecl CL_Snd_Restart_f()
{
  void *hunkBuf;
  int   savedSize;
  int  *Block;

  if ( com_sv_running->integer )
  {
    Com_Printf("Listen server cannot sound restart.\n");
  }
  else
  {
    hunkBuf = Hunk_AllocateTempMemoryInternal(0);

    savedSize = MSS_Save(*(_DWORD *)hunk_totalSize
                       - *(_DWORD *)hunk_highTemp
                       - *(_DWORD *)hunk_temp_temp, (int)hunkBuf);

    Block = Z_MallocInternal(savedSize);
    memcpy(Block, hunkBuf, savedSize);
    Hunk_FreeTempMemory(hunkBuf);

    MSS_Shutdown();
    MSS_Init();
    CL_Vid_Restart_f();
    MSS_Restore((int)Block, savedSize);
    free(Block);
  }
}

/* ---- CL_OpenedPK3List_f  0x0040FEC0 ----  [CONFIRMED] */
void __cdecl CL_OpenedPK3List_f()
{
  const char *v0;

  v0 = FS_LoadedPakNames();
  Com_Printf("Opened PK3 Names: %s\n", v0);
}

/* ---- CL_ReferencedPK3List_f  0x0040FEE0 ----  [CONFIRMED] */
void __cdecl CL_ReferencedPK3List_f()
{
  const char *v0;

  v0 = FS_ReferencedPakNames();
  Com_Printf("Referenced PK3 Names: %s\n", v0);
}

/* ---- CL_Configstrings_f  0x0040FF00 ----  VERIFIED */
void CL_Configstrings_f( void ) {
	int i;
	int ofs;

	if ( cls_state != CA_ACTIVE ) {
		Com_Printf( "Not connected to a server.\n" );
		return;
	}

	for ( i = 0; i < MAX_CONFIGSTRINGS; i++ ) {
		ofs = cl_gameState_stringOffsets[i];
		if ( !ofs ) {
			continue;
		}
		Com_Printf( "%4i: %s\n", i, &cl_gameState_stringData[ofs] );
	}
}

/* ---- CL_Clientinfo_f  0x0040FF50 ----  [CONFIRMED] */
void __cdecl CL_Clientinfo_f()
{
  char *v0;

  Com_Printf("--------- Client Information ---------\n");
  Com_Printf("state: %i\n", cls_state);
  Com_Printf("Server: %s\n", cls_servername);
  Com_Printf("User info settings:\n");
  v0 = Cvar_InfoString(CVAR_USERINFO);
  Info_Print(v0);
  Com_Printf("--------------------------------------\n");
}

/* ---- CL_DownloadsComplete  0x0040FFB0 ----  [HIGH] */
void __cdecl CL_DownloadsComplete()
{
  const char *v0;
  const char *v1;
  int v2;
  char *v3;
  char *v4;

  if ( autoupdateStarted )
  {
    if ( strlen(&byte_155F260) > 4 )
    {
      v3 = FS_ShiftStr("ni]Zm^l", 7);
      v4 = va("%s/%s", v3, &byte_155F260);
      /* retail pushes only the literal 1 */
      Sys_StartProcess(v4, 1);
    }
    autoupdateStarted = 0;
    CL_Disconnect(qtrue);
  }
  else
  {
    if ( cls_downloadRestart )
    {
      cls_downloadRestart = 0;
      FS_Restart(clc_checksumFeed);
      CL_AddReliableCommand("donedl");
      return;
    }
    cls_state = CA_LOADING;
    Com_EventLoop();
    if ( cls_state == CA_LOADING )
    {
      Cvar_Set2("r_uiFullScreen", "0", qtrue);
      if ( com_sv_running->integer )
      {
        if ( cgvm )
          return;
      }
      else
      {
        CL_ShutdownAll();
        Hunk_Clear();
      }
      CL_StartHunkUsers();
      Cvar_Set2("cl_paused", "1", qtrue);
      CL_InitCGame();
      CL_SendPureChecksums();
      CL_WritePacket();
      CL_WritePacket();
      CL_WritePacket();
    }
  }
}

/* ---- CL_BeginDownload  0x004100D0 ----  VERIFIED */
void CL_BeginDownload( const char *localName, const char *remoteName )
{
	Com_DPrintf(
		"***** CL_BeginDownload *****\nLocalname: %s\nRemotename: %s\n****************************\n",
		localName,
		remoteName );

	strncpy( (char *)&cls_downloadName, localName, 0xFFu );
	byte_15EEBAB = 0;
	Com_sprintf( (char *)&cls_downloadTempName, 256, "%s.tmp", localName );

	Cvar_Set2( "cl_downloadName", remoteName, qtrue );
	Cvar_Set2( "cl_downloadSize", "0", qtrue );
	Cvar_Set2( "cl_downloadCount", "0", qtrue );
	Cvar_SetValue( "cl_downloadTime", (float)cls_realtime );

	*(_DWORD *)cls_downloadBlock = 0;
	cls_downloadCount = 0;

	CL_AddReliableCommand( va( "download %s", remoteName ) );
}

/* ---- CL_NextDownload  0x00410190 ----  VERIFIED */
void CL_NextDownload( void )
{
  char *remoteName;
  char *localName;
  char *rest;
  char *at;

  if ( !clc_downloadList[0] ) {
    CL_DownloadsComplete();
    return;
  }

  remoteName = (char *)&clc_downloadList[0];
  if ( clc_downloadList[0] == '@' ) {
    remoteName = (char *)&unk_15EEBBD[0];
  }

  at = strchr( remoteName, '@' );
  if ( at )
  {
    *at = 0;
    localName = at + 1;

    at = strchr( localName, '@' );
    if ( at )
    {
      *at = 0;
      rest = at + 1;
    }
    else
    {
      rest = &localName[strlen( localName )];
    }

    CL_BeginDownload( localName, remoteName );
    cls_downloadRestart = 1;
    strcpy( (char *)&clc_downloadList[0], rest );
  }
  else
  {
    CL_DownloadsComplete();
  }
}

/* ---- CL_InitDownloads  0x00410240 ----  VERIFIED */
void CL_InitDownloads( void ) {
	char missingfiles[1024];
	char *dir = FS_ShiftStr( "ni]Zm^l", 7 );

	if ( autoupdateStarted
		 && NET_CompareAdr( *(netadr_t *)cls_autoupdateServer,
							*(netadr_t *)clc_serverAddress ) ) {
		if ( strlen( cl_updatefiles->string ) > 4 ) {
			Q_strncpyz( cls_autoupdateFilename, cl_updatefiles->string, 64 );
			Q_strncpyz( (char *)clc_downloadList,
						va( "@%s/%s@%s/%s", dir, cl_updatefiles->string,
							dir, cl_updatefiles->string ),
						sizeof( clc_downloadList ) );
			cls_state = CA_CONNECTED;
			CL_NextDownload();
			return;
		}
	} else if ( com_sv_running->integer || !cl_allowDownload->integer ) {
		if ( FS_ComparePaks( missingfiles, sizeof( missingfiles ), qfalse ) ) {
			Com_Printf( "\nWARNING: You are missing some files referenced by the server:\n"
						"%sYou might not be able to join the game\n"
						"Go to the settings menu to turn on autodownload, or get the file elsewhere\n\n",
						missingfiles );
		}
	} else if ( FS_ComparePaks( (char *)clc_downloadList,
								sizeof( clc_downloadList ), qtrue ) ) {
		Com_Printf( "Need paks: %s\n", clc_downloadList );

		if ( *clc_downloadList ) {
			cls_state = CA_CONNECTED;
			CL_NextDownload();
			return;
		}
	}

	CL_DownloadsComplete();
}

/* ---- CL_CheckForResend  0x004103D0 ----  [HIGH] */
void __cdecl CL_CheckForResend( void )
{
	cvar_t	*var;
	int		qport;
	char	*info;
	int		len, i;
	char	Destination[1024];
	char	format[1024 + 16];

	if ( clc_demoplaying ) {
		return;
	}
	if ( cls_state != CA_CONNECTING && cls_state != CA_CHALLENGING ) {
		return;
	}
	if ( cls_realtime - clc_connectTime < 3000 ) {
		return;
	}

	clc_connectTime = cls_realtime;
	clc_connectPacketCount++;

	if ( cls_state == CA_CONNECTING ) {
		if ( net_lanauthorize->integer
		     || !Sys_IsLANAddress( *(netadr_t *) clc_serverAddress ) ) {
			CL_RequestAuthorization();
		}
		NET_OutOfBandPrint( NS_CLIENT, *(netadr_t *) clc_serverAddress,
		                    "getchallenge" );
		return;
	}
	if ( cls_state != CA_CHALLENGING ) {
		Com_Error( ERR_FATAL, "\x15" "CL_CheckForResend: bad cls.state" );
		return;
	}

	var = Cvar_FindVar( "net_qport" );
	qport = var ? (int) var->value : 0;

	info = Cvar_InfoString( CVAR_USERINFO );
	strncpy( Destination, info, 0x3FF );
	Destination[1023] = 0;
	Info_SetValueForKey( Destination, "protocol", va( "%i", 1 ) );
	Info_SetValueForKey( Destination, "qport", va( "%i", qport ) );
	Info_SetValueForKey( Destination, "challenge", va( "%i", *(int *) clc_challenge ) );

	memcpy( format, "connect \"", 9 );
	len = (int) strlen( Destination );
	for ( i = 0; i < len; i++ ) {
		format[9 + i] = Destination[i];
	}
	format[9 + len] = '"';
	format[10 + len] = 0;

	NET_OutOfBandData( NS_CLIENT, *(netadr_t *) clc_serverAddress,
	                   len + 10, format );

	cvar_modifiedFlags &= ~CVAR_USERINFO;
}

/* ---- CL_DisconnectPacket  0x00410620 ----  VERIFIED */
void CL_DisconnectPacket( netadr_t from ) {
	if ( cls_state == CA_DISCONNECTED ) {
		return;
	}

	if ( !NET_CompareAdr( from, ( (netchan_t *)chan )->remoteAddress ) ) {
		return;
	}

	if ( cls_realtime - clc_lastPacketTime >= 3000 ) {
		Com_Error( ERR_DROP, "EXE_SERVER_DISCONNECTED" );
	}
}

/* ---- CL_MotdPacket  0x00410680 ----  VERIFIED */
void CL_MotdPacket( netadr_t from ) {
	char *challenge;
	char *info;

	if ( !NET_CompareAdr( from, cls_updateServer ) ) {
		return;
	}

	info = Cmd_Argv( 1 );

	challenge = Info_ValueForKey( info, "challenge" );
	if ( strcmp( challenge, cls_updateChallenge ) ) {
		return;
	}

	challenge = Info_ValueForKey( info, "motd" );

	Q_strncpyz( cls_updateInfoString, info, MAX_INFO_STRING );
	Cvar_Set2( "cl_motdString", challenge, qtrue );
}

/* ---- CL_InitServerInfo  0x00410750 ----  VERIFIED */
void CL_InitServerInfo( serverInfo_t *server, serverAddress_t *address ) {
	server->adr.type = NA_IP;
	server->adr.ip[0] = address->ip[0];
	server->adr.ip[1] = address->ip[1];
	server->adr.ip[2] = address->ip[2];
	server->adr.ip[3] = address->ip[3];
	server->adr.port = address->port;

	server->clients = 0;
	server->hostName[0] = '\0';
	server->mapName[0] = '\0';
	server->maxClients = 0;
	server->maxPing = 0;
	server->minPing = 0;
	server->ping = -1;
	server->game[0] = '\0';
	server->gameType[0] = '\0';
	server->netType = 0;
	server->allowAnonymous = 0;
}

/* CoD 1.5: CL_FindServerInfo (0x001133E0 in Call of Duty MP.c). */
static qboolean CL_FindServerInfo( serverAddress_t *address ) {
	int low = 0, high = cls_numglobalservers, mid, cmp;
	netadr_t adr = { 0 };
	adr.type = NA_IP;
	memcpy( adr.ip, address->ip, sizeof( adr.ip ) );
	adr.port = address->port;
	while ( low < high ) {
		mid = ( low + high ) / 2;
		cmp = NET_CompareAdrSigned( &adr, &cls_globalServers[mid].adr );
		if ( cmp < 0 ) {
			high = mid;
		} else if ( cmp > 0 ) {
			low = mid + 1;
		} else {
			while ( mid > 0 && !NET_CompareAdrSigned( &adr, &cls_globalServers[mid - 1].adr ) ) {
				mid--;
			}
			do {
				CL_InitServerInfo( &cls_globalServers[mid++], address );
			} while ( mid < cls_numglobalservers && !NET_CompareAdrSigned( &adr, &cls_globalServers[mid].adr ) );
			return qtrue;
		}
	}
	return qfalse;
}

/* CoD 1.5: CL_CompareAdrSigned / CL_SortGlobalServers (0x00113520). */
static int __cdecl CL_CompareAdrSigned( const void *a, const void *b ) {
	return NET_CompareAdrSigned( &((const serverInfo_t *)a)->adr, &((const serverInfo_t *)b)->adr );
}

void CL_SortGlobalServers( void ) {
	qsort( cls_globalServers, cls_numglobalservers, sizeof( serverInfo_t ), CL_CompareAdrSigned );
}

/* ---- CL_ServersResponsePacket  0x004107B0 ---- */
void CL_ServersResponsePacket( netadr_t from, msg_t *msg ) {
	int i, count;
	serverAddress_t addresses[MAX_SERVERSPERPACKET];
	int numservers;
	byte *buffptr;
	byte *buffend;

	Com_Printf( "CL_ServersResponsePacket\n" );

	if ( cls_numglobalservers == -1 ) {
		cls_numglobalservers = 0;
		cls_numGlobalServerAddresses = 0;
	}

	numservers = 0;
	buffptr = msg->data;
	buffend = buffptr + msg->cursize;
	while ( buffptr + 1 < buffend ) {
		do {
			if ( *buffptr++ == '\\' ) {
				break;
			}
		} while ( buffptr < buffend );

		if ( buffptr >= buffend - 6 ) {
			break;
		}

		addresses[numservers].ip[0] = *buffptr++;
		addresses[numservers].ip[1] = *buffptr++;
		addresses[numservers].ip[2] = *buffptr++;
		addresses[numservers].ip[3] = *buffptr++;

		addresses[numservers].port = (unsigned short)( *buffptr++ << 8 );
		addresses[numservers].port += *buffptr++;
		addresses[numservers].port = BigShort( addresses[numservers].port );

		if ( *buffptr != '\\' ) {
			break;
		}

		Com_DPrintf( "server: %d ip: %d.%d.%d.%d:%d\n", numservers,
		             addresses[numservers].ip[0],
		             addresses[numservers].ip[1],
		             addresses[numservers].ip[2],
		             addresses[numservers].ip[3],
		             addresses[numservers].port );

		numservers++;
		if ( numservers >= MAX_SERVERSPERPACKET ) {
			break;
		}

		if ( buffptr[1] == 'E' && buffptr[2] == 'O' && buffptr[3] == 'T' ) {
			break;
		}
	}

	count = cls_numglobalservers;

	/* 1.5 searches the previously sorted list, then sorts after the packet. */
	for ( i = 0; i < numservers && count < MAX_GLOBAL_SERVERS; i++ ) {
		if ( !CL_FindServerInfo( &addresses[i] ) ) {
			CL_InitServerInfo( &cls_globalServers[count++], &addresses[i] );
		}
	}
	cls_numglobalservers = count;
	CL_SortGlobalServers();
	Com_Printf( "%d servers parsed (total %d)\n", numservers, count );
}

/* ---- CL_ConnectionlessPacket  0x004109D0 ----  [HIGH] */
void CL_ConnectionlessPacket( netadr_t from, msg_t *msg )
{
	char	*s;
	char	*c;

	msg->readcount = 0;
	msg->bit = 0;
	if ( msg->cursize >= 4 ) {
		msg->readcount = 4;
	}

	if ( net_profile->integer ) {
		NetProf_PrepProfiling( &pProf );
		NetProf_AddPacket( (void *) pProf, msg->cursize, 0 );
	}

	s = MSG_ReadStringLine( msg );
	Cmd_TokenizeString2( s, 0 );

	c = ( cmd_argc > 0 ) ? cmd_argv[0] : (char *) empty_string;

	Com_DPrintf( "CL packet %s: %s\n", NET_AdrToString( from ), c );

	if ( c ) {
		if ( !Q_stricmpn( c, "challengeResponse", 99999 ) ) {
			if ( cls_state != CA_CONNECTING  ) {
				Com_Printf( "Unwanted challenge response received.  Ignored.\n" );
				return;
			}
			*(int *) clc_challenge = atol( ( cmd_argc > 1 ) ? cmd_argv[1] : (char *) empty_string );
			if ( cmd_argc > 2 ) {
				dword_15CE990 = atol( cmd_argv[2] );
			} else {
				dword_15CE990 = 0;
			}
			clc_connectPacketCount = 0;
			clc_connectTime = -99999;
			memcpy( clc_serverAddress, &from, sizeof( netadr_t ) );
			cls_state = CA_CHALLENGING;
			Com_DPrintf( "challenge: %d\n", *(int *) clc_challenge );
			return;
		}
		if ( !Q_stricmpn( c, "connectResponse", 99999 ) ) {
			if ( cls_state >= CA_CONNECTED ) {
				Com_Printf( "Dup connect received.  Ignored.\n" );
				return;
			}
			if ( cls_state != CA_CHALLENGING ) {
				Com_Printf( "connectResponse packet while not connecting.  Ignored.\n" );
				return;
			}
			if ( !NET_CompareBaseAdr( from, *(netadr_t *) clc_serverAddress ) ) {
				Com_Printf( "connectResponse from a different address.  Ignored.\n" );
				Com_Printf( "%s should have been %s\n",
				            NET_AdrToString( from ),
				            NET_AdrToString( *(netadr_t *) clc_serverAddress ) );
				return;
			}
			/* CoD autoupdate arm, 0x00410BF9 */
			if ( autoupdateChecked
			     && NET_CompareAdr( *(netadr_t *) cls_autoupdateServer,
			                        *(netadr_t *) clc_serverAddress )
			     && cl_updateavailable->integer ) {
				autoupdateStarted = 1;
			}
			Netchan_Setup( (netchan_t *) chan, NS_CLIENT, from,
			               (int) Cvar_VariableValue( "net_qport" ) );
			cls_state = CA_CONNECTED;
			clc_lastPacketTime = cls_realtime;
			clc_lastPacketSentTime = -9999;
			return;
		}
		if ( !Q_stricmpn( c, "infoResponse", 99999 ) ) {
			CL_ServerInfoPacket( from, msg );
			return;
		}
		if ( !Q_stricmpn( c, "statusResponse", 99999 ) ) {
			CL_ServerStatusResponse( from, msg );
			return;
		}
		if ( !Q_stricmpn( c, "disconnect", 99999 ) ) {
			CL_DisconnectPacket( from );
			return;
		}
	}
	if ( !Q_stricmp( c, "echo" ) ) {
		NET_OutOfBandPrint( NS_CLIENT, from, "%s",
		                    ( cmd_argc > 1 ) ? cmd_argv[1] : (char *) empty_string );
		return;
	}
	if ( !Q_stricmp( c, "keyAuthorize" ) ) {
		return;
	}
	if ( !Q_stricmp( c, "motd" ) ) {
		CL_MotdPacket( from );
		return;
	}
	if ( !Q_stricmp( c, "print" ) ) {
		s = MSG_ReadBigString( msg );
		strncpy( clc_serverMessage, s, 0xFF );
		byte_15CE987[0] = 0;
		Com_Printf( "%s", s );
		return;
	}
	if ( !Q_stricmp( c, "error" ) ) {
		if ( cls_state
		     && NET_CompareBaseAdr( from, *(netadr_t *) clc_serverAddress ) ) {
			s = MSG_ReadBigString( msg );
			Com_Error( ERR_DROP, "%s",
			           SEH_LocalizeTextMessage( s, "server error", 0 ) );
		}
		return;
	}
	if ( !Q_stricmp( c, "updateResponse" ) ) {
		CL_UpdateInfoPacket( from );
		return;
	}
	if ( !strncmp( c, "getserversResponse", 18 ) ) {
		CL_ServersResponsePacket( from, msg );
		return;
	}
	if ( !strncmp( c, "needcdkey", 9 ) ) {
		strncpy( clc_serverMessage, "EXE_AWAITINGCDKEYAUTH", 0xFF );
		byte_15CE987[0] = 0;
		Com_Printf( "%s\n",
		            SEH_LocalizeTextMessage( "EXE_AWAITINGCDKEYAUTH",
		                                     "need cd key message", 0 ) );
		CL_RequestAuthorization();
		return;
	}
	if ( !Q_stricmp( c, "loadingnewmap" ) ) {
		if ( NET_CompareBaseAdr( from, *(netadr_t *) clc_serverAddress ) ) {
			const char *mapname = va( "%s", MSG_ReadStringLine( msg ) );
			const char *gametype = MSG_ReadStringLine( msg );
			CL_SetupForNewServerMap( mapname, gametype );
		}
		return;
	}

	Com_DPrintf( "Unknown connectionless packet command.\n" );
}

/* CL_PacketEvent (0x00410FF0) is defined further down this unit. */
#if 0
void __cdecl CL_PacketEvent(netadr_t from, void *msg)
{
  int v2;
  int v3;
  int v4;
  const char *v5;
  const char *v6;
  _BYTE v7[40]; // [esp-28h] [ebp-34h] BYREF
  void *v8;
  int v9;

  v3 = v2;
  v4 = *(_DWORD *)(v2 + 12);
  if ( v4 >= 4 && **(_DWORD **)(v2 + 4) == -1 )
  {
    qmemcpy(&v7[16], &from, 0x14u);
    CL_ConnectionlessPacket(*(netadr_t *)&v7[16], (void *)v2);
  }
  else if ( *(int *)cls_state >= 3 )
  {
    if ( v4 >= 4 )
    {
      qmemcpy(&v7[20], &chan.remoteAddress, 0x14u);
      qmemcpy(v7, &from, 0x14u);
      if ( NET_CompareAdr(*(netadr_t *)v7, *(netadr_t *)&v7[20]) )
      {
        dword_15CE868 = cls_realtime;
        if ( Netchan_Process(&chan, (msg_t *)v3) )
        {
          clc_serverMessageSequence = **(_DWORD **)(v3 + 4);
          clc_reliableAcknowledge = MSG_ReadLong((msg_t *)v3);
          if ( clc_reliableAcknowledge >= clc_reliableSequence - 64 )
          {
            CL_Netchan_Decode(
              (_BYTE *)(*(_DWORD *)(v3 + 16) + *(_DWORD *)(v3 + 4)),
              *(_DWORD *)(v3 + 12) - *(_DWORD *)(v3 + 16));
            CL_ParseServerMessage(v3);
            if ( clc_demorecording )
            {
              if ( !clc_demowaiting )
                CL_WriteDemoMessage(v8, v9);
            }
          }
          else
          {
            clc_reliableAcknowledge = clc_reliableSequence;
          }
        }
      }
      else
      {
        qmemcpy(&v7[20], &from, 0x14u);
        v6 = NET_AdrToString(*(netadr_t *)&v7[20]);
        Com_DPrintf("%s:sequenced packet without connection\n", v6);
      }
    }
    else
    {
      qmemcpy(&v7[20], &from, 0x14u);
      v5 = NET_AdrToString(*(netadr_t *)&v7[20]);
      Com_Printf("%s: Runt packet\n", v5);
    }
  }
}
#endif

/* ---- CL_CheckTimeout  0x00411130 ----  VERIFIED */
void CL_CheckTimeout( void ) {
	if ( ( !cl_paused->integer || !sv_paused->integer )
		 && cls_state >= CA_CONNECTED && cls_state != CA_CINEMATIC
		 && cls_state != CA_LOGO
		 && cls_realtime - clc_lastPacketTime > cl_timeout->value * 1000 ) {
		if ( ++cl_active > 5 ) {
			Com_Error( ERR_DROP, "EXE_ERR_SERVER_TIMEOUT" );
			return;
		}
	} else {
		cl_active = 0;
	}
}

/* ---- CL_CheckUserinfo  0x004111C0 ----  VERIFIED */
void CL_CheckUserinfo( void ) {
	if ( cls_state < CA_CHALLENGING ) {
		return;
	}
	if ( cl_paused->integer ) {
		return;
	}
	if ( cvar_modifiedFlags & CVAR_USERINFO ) {
		cvar_modifiedFlags &= ~CVAR_USERINFO;
		CL_AddReliableCommand( va( "userinfo \"%s\"", Cvar_InfoString( CVAR_USERINFO ) ) );
	}
}

/* ---- CL_UpdateInGameState  0x00411220 ----  VERIFIED */
void CL_UpdateInGameState( void ) {
	if ( cls_state == CA_ACTIVE ) {
		if ( !cl_ingame->integer ) {
			Cvar_Set2( "cl_ingame", "1", qtrue );
		}
	} else {
		if ( cl_ingame->integer ) {
			Cvar_Set2( "cl_ingame", "0", qtrue );
		}
	}
}

/* ---- CL_SetRecommended_f  0x00411450 ----  [CONFIRMED] */
void __cdecl CL_SetRecommended_f()
{
  Com_SetRecommended(1);
}

/* ---- CL_RefPrintf  0x00411460 ----  [CONFIRMED] */
void CL_RefPrintf(int print_level, const char *fmt, ...)
{
  char Buffer[4096];
  unsigned int v3;
  unsigned int retaddr;
  va_list ArgList;

  va_start(ArgList, fmt);
  v3 = retaddr ^ _security_cookie;
  _vsnprintf(Buffer, sizeof(Buffer) - 1, fmt, ArgList);
  Buffer[sizeof(Buffer) - 1] = 0;
  if ( print_level )
  {
    if ( print_level == 2 )
    {
      Com_Printf("^3%s", Buffer);
    }
    else if ( print_level == 1 )
    {
      Com_DPrintf("^1%s", Buffer);
    }
  }
  else
  {
    Com_Printf("%s", Buffer);
  }
}

#if 0
void __cdecl CL_InitRenderer()
{
  re_BeginRegistration(&cls_glconfig);
  whiteShader = re_RegisterShader("white", 2);
  dword_15CA634 = re_RegisterShader("console", 2);
  dword_57C0F8 = cls_glconfig_vidWidth - 32;
  dword_142F64C = cls_glconfig_vidWidth - 32;
  dword_142F654 = dword_57C100;
  dword_142F650 = dword_57C0FC;
  memset(stats, 0, 0x200u);
  dword_142F658 = 1;
  statCount = 0;
}
#endif

/* ---- CL_StartHunkUsers  0x004115C0 ----  [CONFIRMED] */
void __cdecl CL_StartHunkUsers()
{
  if ( com_cl_running )
  {
    if ( com_cl_running->integer )
    {
      xmodel_enforceExist = Cvar_Get("cl_xmodelcheck", "0", 33)->integer;
      if ( !cls_rendererStarted )
      {
        cls_rendererStarted = 1;
        CL_InitRenderer();
        Sys_LoadingKeepAlive();
      }
      if ( !cls_soundStarted )
      {
        cls_soundStarted = 1;
        MSS_Init();
        Sys_LoadingKeepAlive();
      }
      if ( !cls_loadingPlaque )
      {
        cls_loadingPlaque = 1;
        CL_InitUI();
        Sys_LoadingKeepAlive();
      }
    }
  }
}

/* ---- CL_CheckAutoUpdate  0x00411650 ----  VERIFIED */
void CL_CheckAutoUpdate( void ) {
	int validServerNum = 0;
	int i = 0, rnd = 0;
	netadr_t temp;
	char *servername[MAX_AUTOUPDATE_SERVERS];
	char *cls_autoupdateServerNames[MAX_AUTOUPDATE_SERVERS];

	cls_autoupdateServerNames[0] = cls_autoupdateServerNames_0_;
	cls_autoupdateServerNames[1] = cls_autoupdateServerNames_1_;
	cls_autoupdateServerNames[2] = cls_autoupdateServerNames_2_;
	cls_autoupdateServerNames[3] = cls_autoupdateServerNames_3_;
	cls_autoupdateServerNames[4] = cls_autoupdateServerNames_4_;

	if ( autoupdateChecked ) {
		return;
	}

	srand( Com_Milliseconds() );

	for ( i = 0; i < MAX_AUTOUPDATE_SERVERS; i++ ) {
		if ( NET_StringToAdr( cls_autoupdateServerNames[i], &temp ) ) {
			servername[validServerNum++] = cls_autoupdateServerNames[i];
		}
	}

	if ( !validServerNum ) {
		Com_DPrintf( "Couldn't resolve an AutoUpdate Server address.\n" );
	} else {
		rnd = rand() % validServerNum;

		Com_DPrintf( "Resolving AutoUpdate Server... " );
		if ( !NET_StringToAdr( servername[rnd], (netadr_t *)cls_autoupdateServer ) ) {
			Com_DPrintf( "\nCouldn't resolve first address, trying others... " );

			for ( i = 1; i < validServerNum; i++ ) {
				if ( NET_StringToAdr( servername[( i + rnd ) % validServerNum],
									  (netadr_t *)cls_autoupdateServer ) ) {
					Com_DPrintf( "\nAlternate server address resolved... " );
					break;
				}
			}

			if ( i == validServerNum ) {
				Com_DPrintf( "\nFailed to resolve any Auto-update servers.\n" );
				autoupdateChecked = qtrue;
				return;
			}
		}

		( (netadr_t *)cls_autoupdateServer )->port = BigShort( PORT_SERVER );
		Com_DPrintf( "%i.%i.%i.%i:%i\n",
					 ( (netadr_t *)cls_autoupdateServer )->ip[0],
					 ( (netadr_t *)cls_autoupdateServer )->ip[1],
					 ( (netadr_t *)cls_autoupdateServer )->ip[2],
					 ( (netadr_t *)cls_autoupdateServer )->ip[3],
					 BigShort( ( (netadr_t *)cls_autoupdateServer )->port ) );

		NET_OutOfBandPrint( NS_CLIENT, *(netadr_t *)cls_autoupdateServer,
							"getUpdateInfo \"%s\" \"%s\"\n", "1.1", "win-x86" );

		CL_RequestMotd();
	}

	autoupdateChecked = qtrue;
}

/* ---- CL_GetAutoUpdate  0x004117E0 ----  VERIFIED */
void CL_GetAutoUpdate( void ) {
	if ( !autoupdateChecked ) {
		return;
	}
	if ( !strlen( cl_updatefiles->string ) ) {
		return;
	}

	Sys_OpenURL( cl_updatefiles->string, "quit\n" );
}

/* ---- CL_ScaledMilliseconds  0x00411860 ----  VERIFIED */
int __cdecl CL_ScaledMilliseconds()
{
  if ( !sys_timeBaseInit )
  {
    sys_timeBase = timeGetTime();
    sys_timeBaseInit = 1;
  }
  return (int)( (float)(int)( timeGetTime() - sys_timeBase ) * com_timescale->value );
}

/* ---- CG_GetGameModel  0x004118B0 ----  [CONFIRMED] */
void *__cdecl CG_GetGameModel(int handle)
{
  return (void *)VM_Call(cgvm, 9, (__int16)handle);
}

/* ---- CG_DObjCalcPose  0x004118D0 ----  VERIFIED */
void __cdecl CG_DObjCalcPose( int dobj, int partBits, int a3 )
{
  VM_Call( cgvm, 10, dobj, partBits, a3 );
}

/* ---- CL_GetFontInfo  0x004118F0 ----  VERIFIED */
int __cdecl CL_GetFontInfo( void *font, float scale )
{
  return VM_Call( uivm, 16, font, (int)( scale * 100.0f ) );
}

/* ---- CL_startSingleplayer_f  0x00411AE0 ----  VERIFIED */
void CL_startSingleplayer_f( void )
{
	Sys_StartProcess( "CoDSP.exe", 1 );
}

/* ---- CL_DrawLogo  0x00411B00 ----  [HIGH] */
void __cdecl CL_DrawLogo()
{
  int v0;
  double v1;
  double v2;
  float v3;
  float v4;
  float v5;
  float v6[4];

  v0 = cls_realtime - cls_logoStartTime;
  if ( cls_realtime - cls_logoStartTime >= cls_logoFadeInDuration )
  {
    if ( v0 <= cls_logoTotalDuration - cls_logoFadeOutDuration )
    {
LABEL_8:
      v1 = 1.0;
      goto LABEL_9;
    }
    v1 = (double)(cls_logoTotalDuration - v0) / (double)cls_logoFadeOutDuration;
  }
  else
  {
    v1 = (double)(cls_realtime - cls_logoStartTime) / (double)cls_logoFadeInDuration;
  }
  if ( (v1 < 0.0) | __UNORDERED__(v1, 0.0) )
  {
    v1 = 0.0;
    goto LABEL_9;
  }
  if ( v1 > 1.0 )
    goto LABEL_8;
LABEL_9:
  v6[0] = v1;
  v6[1] = v1;
  v6[2] = v1;
  v6[3] = 1.0;
  v4 = (float)cls_glconfig_vidWidth;
  v2 = (double)cls_glconfig_vidHeight;
  v3 = (v2 + v2) * 0.33333334;
  v5 = v2 - v3;
  re_SetColor(v6);
  re_DrawStretchPic(0, 0, LODWORD(v4), LODWORD(v3), 0, 0, 1065353216, 1065353216, cls_logoShaderTop);
  re_DrawStretchPic(0, LODWORD(v3), LODWORD(v4), LODWORD(v5), 0, 0, 1065353216, 1065353216, cls_logoShaderBottom);
  re_SetColor(0);
  if ( v0 > cls_logoTotalDuration )
    cls_state = CA_DISCONNECTED;
}

/* ---- CL_StopLogo  0x00411C40 ----  [CONFIRMED] */
void __cdecl CL_StopLogo()
{
  cls_state = CA_DISCONNECTED;
}

/* ---- CL_PlayLogo_f  0x00411C50 ----  [CONFIRMED] */
void __cdecl CL_PlayLogo_f()
{
  char *v0;
  char *v1;
  char *v2;
  char *v3;
  char *v4;
  char *v5;
  float v7;
  float v8;
  float v9;

  if ( cmd_argc != 5 )
  {
    Com_Printf("USAGE: logo <image name> <fadein seconds> <full duration seconds> <fadeout seconds>\n");
    return;
  }
  Com_DPrintf("CL_PlayLogo_f\n");
  if ( cls_state == CA_CINEMATIC )
  {
    if ( (unsigned int)CL_handle < MAX_VIDEO_HANDLES )
    {
      CIN_StopCinematic(CL_handle);
      MSS_StopSounds(0);
      CL_handle = -1;
    }
LABEL_8:
    cls_state = CA_LOGO;
    if ( uivm )
      VM_Call(uivm, UI_SET_ACTIVE_MENU, UIMENU_NONE);
    MSS_StopSounds(0);
    mss_globalFadeRate = 1.0 - mss_globalFadeCur;
    mss_globalFadeTarget = 1065353216;
    v0 = &empty_string;
    if ( (unsigned int)cmd_argc > 1 )
      v0 = cmd_argv[1];
    v1 = &empty_string;
    if ( (unsigned int)cmd_argc > 2 )
      v1 = cmd_argv[2];
    v7 = atof(v1) * 1000.0;
    cls_logoFadeInDuration = (int)(v7 + 9.313225746154785e-10);
    v2 = &empty_string;
    if ( (unsigned int)cmd_argc > 3 )
      v2 = cmd_argv[3];
    v8 = atof(v2) * 1000.0;
    cls_logoTotalDuration = (int)(v8 + 9.313225746154785e-10);
    v3 = &empty_string;
    if ( (unsigned int)cmd_argc > 4 )
      v3 = cmd_argv[4];
    v9 = atof(v3) * 1000.0;
    cls_logoFadeOutDuration = (int)(v9 + 9.313225746154785e-10);
    cls_logoTotalDuration += cls_logoFadeOutDuration + cls_logoFadeInDuration;
    v4 = va("%s1", v0);
    cls_logoShaderTop = Material_RegisterHandle(v4, 2);
    v5 = va("%s2", v0);
    cls_logoShaderBottom = Material_RegisterHandle(v5, 2);
    cls_logoStartTime = cls_realtime + 100;
    return;
  }
  if ( cls_state == CA_LOGO || !cls_state )
    goto LABEL_8;
}

/* CL_Init (0x00411E60) is defined further down this unit. */
#if 0
void __cdecl CL_Init()
{
  Com_Printf("----- Client Initialization -----\n");
  Con_Init();
  memset(&cl_active, 0, 0x12C8D8u);
  *(_DWORD *)cls_state = 0;
  cls_realtime = 0;
  CL_InitInput();
  cl_noprint = Cvar_Get("cl_noprint", "0", 0);
  cl_motd = Cvar_Get("cl_motd", "1", 0);
  cl_timeout = Cvar_Get("cl_timeout", "200", 0);
  cl_timeNudge = Cvar_Get("cl_timeNudge", "0", CVAR_TEMP);
  cl_shownet = Cvar_Get("cl_shownet", "0", CVAR_TEMP);
  cl_shownuments = Cvar_Get("cl_shownuments", "0", CVAR_TEMP);
  cl_visibleClients = Cvar_Get("cl_visibleClients", "0", CVAR_TEMP);
  cl_showServerCommands = Cvar_Get("cl_showServerCommands", "0", 0);
  cl_showSend = Cvar_Get("cl_showSend", "0", CVAR_TEMP);
  cl_showTimeDelta = Cvar_Get("cl_showTimeDelta", "0", CVAR_TEMP);
  cl_freezeDemo = Cvar_Get("cl_freezeDemo", "0", CVAR_TEMP);
  rconPassword = Cvar_Get("rconPassword", &empty_string, CVAR_TEMP);
  activeAction = Cvar_Get("activeAction", &empty_string, CVAR_TEMP);
  timedemo = Cvar_Get("timedemo", "0", 0);
  cl_avidemo = Cvar_Get("cl_avidemo", "0", 0);
  cl_forceavidemo = Cvar_Get("cl_forceavidemo", "0", 0);
  rconAddress = Cvar_Get("rconAddress", &empty_string, 0);
  cl_yawspeed = Cvar_Get("cl_yawspeed", "140", CVAR_ARCHIVE);
  cl_pitchspeed = Cvar_Get("cl_pitchspeed", "140", CVAR_ARCHIVE);
  cl_anglespeedkey = Cvar_Get("cl_anglespeedkey", "1.5", 0);
  cl_maxpackets = Cvar_Get("cl_maxpackets", "30", CVAR_ARCHIVE);
  cl_packetdup = Cvar_Get("cl_packetdup", "1", CVAR_ARCHIVE);
  cl_run = Cvar_Get("cl_run", "1", CVAR_TEMP);
  cl_stance = Cvar_Get("cl_stance", "0", CVAR_TEMP);
  cl_stanceTemp = Cvar_Get("cl_stanceTemp", "0", CVAR_TEMP);
  cl_goStandJumpTime = Cvar_Get("cl_goStandJumpTime", "0", CVAR_ARCHIVE);
  cl_sensitivity = Cvar_Get("sensitivity", "5", CVAR_ARCHIVE);
  cl_mouseAccel = Cvar_Get("cl_mouseAccel", "0", CVAR_ARCHIVE);
  cl_freelook = Cvar_Get("cl_freelook", "1", CVAR_ARCHIVE);
  cl_showmouserate = Cvar_Get("cl_showmouserate", "0", 0);
  cl_allowDownload = Cvar_Get("cl_allowDownload", "0", CVAR_ARCHIVE);
  cl_conXOffset = Cvar_Get("cl_conXOffset", "0", 0);
  r_inGameVideo = Cvar_Get("r_inGameVideo", "1", CVAR_ARCHIVE);
  cl_serverStatusResendTime = Cvar_Get("cl_serverStatusResendTime", "750", 0);
  cl_viewPitchCompensate = Cvar_Get("cl_viewPitchCompensate", "0", CVAR_ROM);
  cl_viewYawCompensate = Cvar_Get("cl_viewYawCompensate", "0", CVAR_ROM);
  cl_bypassMouseInput = Cvar_Get("cl_bypassMouseInput", "0", 0);
  m_pitch = Cvar_Get("m_pitch", "0.022", CVAR_ARCHIVE);
  m_yaw = Cvar_Get("m_yaw", "0.022", CVAR_ARCHIVE);
  m_forward = Cvar_Get("m_forward", "0.25", CVAR_ARCHIVE);
  m_side = Cvar_Get("m_side", "0.25", CVAR_ARCHIVE);
  m_filter = Cvar_Get("m_filter", "0", CVAR_ARCHIVE);
  cl_motdString = Cvar_Get("cl_motdString", &empty_string, CVAR_ROM);
  cl_ingame = Cvar_Get("cl_ingame", "0", CVAR_ROM);
  Cvar_Get("cl_maxPing", "800", CVAR_ARCHIVE);
  Cvar_Get("cg_drawCompass", "1", CVAR_ARCHIVE);
  Cvar_Get("cg_drawNotifyText", "1", CVAR_ARCHIVE);
  Cvar_Get("cg_descriptiveText", "1", CVAR_ARCHIVE);
  Cvar_Get("cg_drawTeamOverlay", "2", CVAR_ARCHIVE);
  Cvar_Get("cg_drawGun", "1", CVAR_ARCHIVE);
  Cvar_Get("cg_cursorHints", "4", CVAR_ARCHIVE);
  Cvar_Get("cg_voiceSpriteTime", "6000", CVAR_ARCHIVE);
  Cvar_Get("cg_teamChatsOnly", "0", CVAR_ARCHIVE);
  Cvar_Get("cg_noVoiceChats", "0", CVAR_ARCHIVE);
  Cvar_Get("cg_noVoiceText", "0", CVAR_ARCHIVE);
  Cvar_Get("cg_crosshairSize", "48", CVAR_ARCHIVE);
  Cvar_Get("cg_drawCrosshair", "1", CVAR_ARCHIVE);
  Cvar_Get("name", "Unknown Soldier", CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("rate", "5000", CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("snaps", "20", CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("model", &empty_string, CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("head", &empty_string, CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("handicap", "100", CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("cl_anonymous", "0", CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("password", &empty_string, CVAR_USERINFO);
  Cvar_Get("cg_predictItems", "1", CVAR_ARCHIVE|CVAR_USERINFO);
  Cvar_Get("cg_viewsize", "100", CVAR_ARCHIVE);
  cl_waitForFire = Cvar_Get("cl_waitForFire", "0", CVAR_ROM);
  fx_enable = Cvar_Get("fx_enable", "1", CVAR_CHEAT);
  fx_draw = Cvar_Get("fx_draw", "1", CVAR_CHEAT);
  fx_cull = Cvar_Get("fx_cull", "1", 0);
  fx_freeze = Cvar_Get("fx_freeze", "0", CVAR_CHEAT);
  fx_debug = Cvar_Get("fx_debug", "0", CVAR_CHEAT);
  fx_debugBolt = Cvar_Get("fx_debugBolt", "0", CVAR_CHEAT);
  fx_count = Cvar_Get("fx_count", "0", CVAR_CHEAT);
  cl_updateavailable = Cvar_Get("cl_updateavailable", "0", CVAR_ROM);
  cl_updatefiles = Cvar_Get("cl_updatefiles", &empty_string, CVAR_ROM);
  cl_updateoldversion = Cvar_Get("cl_updateoldversion", &empty_string, CVAR_ROM);
  cl_updateversion = Cvar_Get("cl_updateversion", &empty_string, CVAR_ROM);
  cl_serverloadmap = Cvar_Get("cl_serverloadmap", &empty_string, CVAR_ROM);
  cl_serverloadgametype = Cvar_Get("cl_serverloadgametype", &empty_string, CVAR_ROM);
  cl_serverloadwaiting = Cvar_Get("cl_serverloadwaiting", "0", CVAR_ROM);
  strncpy(
    cls_autoupdateServerNames_0_,
    "au2cod1.activision.com",
    CVAR_ARCHIVE|CVAR_USERINFO|CVAR_SERVERINFO|CVAR_SYSTEMINFO|CVAR_INIT|CVAR_LATCH);
  byte_15CA47B = 0;
  strncpy(
    cls_autoupdateServerNames_1_,
    "au2cod2.activision.com",
    CVAR_ARCHIVE|CVAR_USERINFO|CVAR_SERVERINFO|CVAR_SYSTEMINFO|CVAR_INIT|CVAR_LATCH);
  byte_15CA4BB = 0;
  strncpy(
    cls_autoupdateServerNames_2_,
    "au2cod3.activision.com",
    CVAR_ARCHIVE|CVAR_USERINFO|CVAR_SERVERINFO|CVAR_SYSTEMINFO|CVAR_INIT|CVAR_LATCH);
  byte_15CA4FB = 0;
  strncpy(
    cls_autoupdateServerNames_3_,
    "au2cod4.activision.com",
    CVAR_ARCHIVE|CVAR_USERINFO|CVAR_SERVERINFO|CVAR_SYSTEMINFO|CVAR_INIT|CVAR_LATCH);
  byte_15CA53B = 0;
  strncpy(
    cls_autoupdateServerNames_4_,
    "au2cod5.activision.com",
    CVAR_ARCHIVE|CVAR_USERINFO|CVAR_SERVERINFO|CVAR_SYSTEMINFO|CVAR_INIT|CVAR_LATCH);
  byte_15CA57B = 0;
  Cmd_AddCommand(cmd_name, CL_ForwardToServer_f);
  Cmd_AddCommand("configstrings", CL_Configstrings_f);
  Cmd_AddCommand("clientinfo", CL_Clientinfo_f);
  Cmd_AddCommand("snd_restart", CL_Snd_Restart_f);
  Cmd_AddCommand("vid_restart", CL_Vid_Restart_f);
  Cmd_AddCommand("disconnect", CL_Disconnect_f);
  Cmd_AddCommand("record", CL_Record_f);
  Cmd_AddCommand("demo", CL_PlayDemo_f);
  Cmd_AddCommand("cinematic", CL_PlayCinematic_f);
  Cmd_AddCommand("logo", CL_PlayLogo_f);
  Cmd_AddCommand("stoprecord", CL_StopRecord_f);
  Cmd_AddCommand("connect", CL_Connect_f);
  Cmd_AddCommand("reconnect", CL_Reconnect_f);
  Cmd_AddCommand("localservers", CL_LocalServers_f);
  Cmd_AddCommand("globalservers", CL_GlobalServers_f);
  Cmd_AddCommand("rcon", CL_Rcon_f);
  Cmd_AddCommand("setenv", CL_Setenv_f);
  Cmd_AddCommand("ping", CL_Ping_f);
  Cmd_AddCommand("serverstatus", CL_ServerStatus_f);
  Cmd_AddCommand("showip", CL_ShowIP_f);
  Cmd_AddCommand("fs_openedList", CL_OpenedPK3List_f);
  Cmd_AddCommand("fs_referencedList", CL_ReferencedPK3List_f);
  Cmd_AddCommand("updatehunkusage", CL_UpdateLevelHunkUsage);
  Cmd_AddCommand("updatescreen", (xcommand_t)SCR_UpdateScreen);
  Cmd_AddCommand("startSingleplayer", CL_startSingleplayer_f);
  Cmd_AddCommand("setRecommended", CL_SetRecommended_f);
  Cmd_AddCommand("cubemapShot", (xcommand_t)CL_CubeMapShot_f);
  Cmd_AddCommand("localizeSoundAliasFiles", Com_WriteLocalizedSoundAliasFiles);
  autoupdateChecked = 0;
  dword_1432848 = 0;
  CL_CheckAutoUpdate();
  CL_InitRef();
  SCR_Init();
  Cbuf_Execute();
  Cvar_Set2("cl_running", "1", qtrue);
  Com_Printf("----- Client Initialization Complete -----\n");
}
#endif

/* ---- CL_SetServerInfo  0x004129F0 ----  VERIFIED */
void CL_SetServerInfo( serverInfo_t *server, const char *info, int ping ) {
	if ( server ) {
		if ( info ) {
			server->clients = atoi( Info_ValueForKey( info, "clients" ) );
			Q_strncpyz( server->hostName, Info_ValueForKey( info, "hostname" ), MAX_NAME_LENGTH );
			Q_strncpyz( server->mapName, Info_ValueForKey( info, "mapname" ), MAX_NAME_LENGTH );
			server->maxClients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
			Q_strncpyz( server->game, Info_ValueForKey( info, "game" ), MAX_NAME_LENGTH );
			Q_strncpyz( server->gameType, Info_ValueForKey( info, "gametype" ), MAX_NAME_LENGTH );
			server->netType = atoi( Info_ValueForKey( info, "nettype" ) );
			server->minPing = atoi( Info_ValueForKey( info, "minping" ) );
			server->maxPing = atoi( Info_ValueForKey( info, "maxping" ) );
			server->allowAnonymous = atoi( Info_ValueForKey( info, "sv_allowAnonymous" ) );
			server->password = atoi( Info_ValueForKey( info, "pswrd" ) );
		}
		server->ping = ping;
	}
}

/* ---- CL_SetServerInfoByAddress  0x00412B30 ----  VERIFIED */
void CL_SetServerInfoByAddress( netadr_t from, const char *info, int ping ) {
	int i;

	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		if ( NET_CompareAdr( from, cls_localServers[i].adr ) ) {
			CL_SetServerInfo( &cls_localServers[i], info, ping );
		}
	}

	for ( i = 0; i < MAX_GLOBAL_SERVERS; i++ ) {
		if ( NET_CompareAdr( from, cls_globalServers[i].adr ) ) {
			CL_SetServerInfo( &cls_globalServers[i], info, ping );
		}
	}

	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		if ( NET_CompareAdr( from, cls_favoriteServers[i].adr ) ) {
			CL_SetServerInfo( &cls_favoriteServers[i], info, ping );
		}
	}
}

/* ---- CL_ServerInfoPacket  0x00412C30 ----  VERIFIED */
void CL_ServerInfoPacket( netadr_t from, msg_t *msg ) {
	int i, type;
	char info[MAX_INFO_STRING];
	char *infoString;
	int prot;

	infoString = MSG_ReadString( msg );

	prot = atoi( Info_ValueForKey( infoString, "protocol" ) );
	if ( prot != PROTOCOL_VERSION ) {
		Com_DPrintf( "Different protocol info packet: %s\n", infoString );
		return;
	}

	for ( i = 0; i < MAX_PINGREQUESTS; i++ ) {
		if ( cl_pinglist[i].adr.port && !cl_pinglist[i].time
			 && NET_CompareAdr( from, cl_pinglist[i].adr ) ) {
			cl_pinglist[i].time = cls_realtime - cl_pinglist[i].start + 1;
			Com_DPrintf( "ping time %dms from %s\n", cl_pinglist[i].time,
						 NET_AdrToString( from ) );

			Q_strncpyz( cl_pinglist[i].info, infoString,
						sizeof( cl_pinglist[i].info ) );

			switch ( from.type ) {
			case NA_BROADCAST:
			case NA_IP:
				type = 1;
				break;

			case NA_IPX:
			case NA_BROADCAST_IPX:
				type = 2;
				break;

			default:
				type = 0;
				break;
			}
			Info_SetValueForKey( cl_pinglist[i].info, "nettype", va( "%d", type ) );
			CL_SetServerInfoByAddress( from, infoString, cl_pinglist[i].time );

			return;
		}
	}

	if ( cls_pingUpdateSource != AS_LOCAL ) {
		return;
	}

	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		if ( cls_localServers[i].adr.port == 0 ) {
			break;
		}

		if ( NET_CompareAdr( from, cls_localServers[i].adr ) ) {
			return;
		}
	}

	if ( i == MAX_OTHER_SERVERS ) {
		Com_DPrintf( "MAX_OTHER_SERVERS hit, dropping infoResponse\n" );
		return;
	}

	cls_numlocalservers = i + 1;
	cls_localServers[i].adr = from;
	cls_localServers[i].clients = 0;
	cls_localServers[i].hostName[0] = '\0';
	cls_localServers[i].mapName[0] = '\0';
	cls_localServers[i].maxClients = 0;
	cls_localServers[i].maxPing = 0;
	cls_localServers[i].minPing = 0;
	cls_localServers[i].ping = -1;
	cls_localServers[i].game[0] = '\0';
	cls_localServers[i].gameType[0] = '\0';
	cls_localServers[i].netType = from.type;
	cls_localServers[i].allowAnonymous = 0;

	Q_strncpyz( info, MSG_ReadString( msg ), MAX_INFO_STRING );
	if ( strlen( info ) ) {
		if ( info[strlen( info ) - 1] != '\n' ) {
			strncat( info, "\n", sizeof( info ) );
		}
		Com_Printf( "%s: %s", NET_AdrToString( from ), info );
	}
}

/* ---- CL_UpdateInfoPacket  0x00412F70 ----  VERIFIED */
void CL_UpdateInfoPacket( netadr_t from ) {

	if ( ( (netadr_t *)cls_autoupdateServer )->type == NA_BAD ) {
		Com_DPrintf( "CL_UpdateInfoPacket:  Auto-Updater has bad address\n" );
		return;
	}

	Com_DPrintf( "Auto-Updater resolved to %i.%i.%i.%i:%i\n",
				 ( (netadr_t *)cls_autoupdateServer )->ip[0],
				 ( (netadr_t *)cls_autoupdateServer )->ip[1],
				 ( (netadr_t *)cls_autoupdateServer )->ip[2],
				 ( (netadr_t *)cls_autoupdateServer )->ip[3],
				 BigShort( ( (netadr_t *)cls_autoupdateServer )->port ) );

	if ( !NET_CompareAdr( from, *(netadr_t *)cls_autoupdateServer ) ) {
		Com_DPrintf( "CL_UpdateInfoPacket:  Received packet from %i.%i.%i.%i:%i\n",
					 from.ip[0], from.ip[1], from.ip[2], from.ip[3],
					 BigShort( from.port ) );
		return;
	}

	Cvar_Set2( "cl_updateavailable", Cmd_Argv( 1 ), qtrue );

	if ( cl_updateavailable->string
		 && !Q_stricmp( cl_updateavailable->string, "1" ) ) {
		Cvar_Set2( "cl_updatefiles", Cmd_Argv( 2 ), qtrue );
		Cvar_Set2( "cl_updateversion", Cmd_Argv( 3 ), qtrue );
		Cvar_Set2( "cl_updateoldversion", "1.1", qtrue );
	}
}

/* ---- CL_GetServerStatus  0x004130E0 ----  [HIGH] */
int __cdecl CL_GetServerStatus(netadr_t from)
{
  int v1;
  char *v2;
  __int32 v3;
  _DWORD *v4;
  int v5;
  int v6;
  int *v7;
  netadr_t v9;
  netadr_t v10;

  v1 = 0;
  v2 = (char *)&unk_15F9080;
  do
  {
    qmemcpy(&v10, v2, sizeof(v10));
    qmemcpy(&v9, &from, sizeof(v9));
    v3 = NET_CompareAdr(v9, v10);
    if ( v3 )
      return (int)&unk_15F7080 + 8232 * v1;
    v2 += 8232;
    ++v1;
  }
  while ( (int)v2 < (int)byte_1619300 );
  v4 = &unk_15F90A4;
  while ( !*v4 )
  {
    v4 += 2058;
    ++v3;
    if ( (int)v4 >= (int)dword_1619324 )
    {
      v3 = -1;
      v5 = 0;
      v6 = 2;
      v7 = (int *)&unk_15FB0C0;
      do
      {
        if ( v3 == -1 || *(v7 - 2058) < v5 )
        {
          v5 = *(v7 - 2058);
          v3 = v6 - 2;
        }
        if ( v3 == -1 || *v7 < v5 )
        {
          v5 = *v7;
          v3 = v6 - 1;
        }
        if ( v3 == -1 || v7[2058] < v5 )
        {
          v5 = v7[2058];
          v3 = v6;
        }
        if ( v3 == -1 || v7[4116] < v5 )
        {
          v5 = v7[4116];
          v3 = v6 + 1;
        }
        if ( v3 == -1 || v7[6174] < v5 )
        {
          v5 = v7[6174];
          v3 = v6 + 2;
        }
        if ( v3 == -1 || v7[8232] < v5 )
        {
          v5 = v7[8232];
          v3 = v6 + 3;
        }
        if ( v3 == -1 || v7[10290] < v5 )
        {
          v5 = v7[10290];
          v3 = v6 + 4;
        }
        if ( v3 == -1 || v7[12348] < v5 )
        {
          v5 = v7[12348];
          v3 = v6 + 5;
        }
        v6 += 8;
        v7 += 16464;
      }
      while ( v6 - 2 < 16 );
      if ( v3 == -1 )
        v3 = ++serverStatusCount & 0xF;
      return (int)&unk_15F7080 + 8232 * v3;
    }
  }
  return (int)&unk_15F7080 + 8232 * v3;
}

/* ---- CL_ServerStatus  0x00413260 ----  VERIFIED */
qboolean CL_ServerStatus( char *serverAddress, char *serverStatusString, int maxLen ) {
	int i;
	netadr_t to;
	serverStatus_t *serverStatus;

	if ( !serverAddress ) {
		for ( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ ) {
			cl_serverStatusList[i].address.port = 0;
			cl_serverStatusList[i].retrieved = qtrue;
		}
		return qfalse;
	}
	if ( !NET_StringToAdr( serverAddress, &to ) ) {
		return qfalse;
	}
	serverStatus = (serverStatus_t *)CL_GetServerStatus( to );
	if ( !serverStatusString ) {
		serverStatus->retrieved = qtrue;
		return qfalse;
	}

	if ( NET_CompareAdr( to, serverStatus->address ) ) {
		if ( !serverStatus->pending ) {
			Q_strncpyz( serverStatusString, serverStatus->string, maxLen );
			serverStatus->retrieved = qtrue;
			serverStatus->startTime = 0;
			return qtrue;
		}
		else if ( serverStatus->startTime
				  < Sys_Milliseconds() - cl_serverStatusResendTime->integer ) {
			serverStatus->print = qfalse;
			serverStatus->pending = qtrue;
			serverStatus->retrieved = qfalse;
			serverStatus->time = 0;
			serverStatus->startTime = Sys_Milliseconds();
			NET_OutOfBandPrint( NS_CLIENT, to, "getstatus" );
			return qfalse;
		}
	}
	else if ( serverStatus->retrieved ) {
		serverStatus->address = to;
		serverStatus->print = qfalse;
		serverStatus->pending = qtrue;
		serverStatus->retrieved = qfalse;
		serverStatus->time = 0;
		serverStatus->startTime = Sys_Milliseconds();
		NET_OutOfBandPrint( NS_CLIENT, to, "getstatus" );
		return qfalse;
	}
	return qfalse;
}

/* ---- CL_ServerStatusResponse  0x00413430 ----  VERIFIED */
void __cdecl CL_ServerStatusResponse(netadr_t from, void *msg)
{
  int v2;
  char *v3;
  char *v4;
  char *StringLine;
  int v6;
  char v7;
  int v8;
  char *v9;
  char *v10;
  char *v11;
  const char *v12;
  char v13;
  int v14;
  unsigned int len;
  netadr_t v15;
  netadr_t v16;
  char v19[4];
  int v20;
  int v21;
  char ArgList[1024];
  unsigned int v23;
  unsigned int retaddr;

  v23 = retaddr ^ _security_cookie;
  v2 = 0;
  v3 = (char *)&unk_15F9080;
  while ( 1 )
  {
    qmemcpy(&v16, v3, sizeof(v16));
    qmemcpy(&v15, &from, sizeof(v15));
    if ( NET_CompareAdr(v15, v16) )
      break;
    v3 += 8232;
    ++v2;
    if ( (int)v3 >= (int)byte_1619300 )
      return;
  }
  v4 = (char *)&unk_15F7080 + 8232 * v2;
  if ( v4 )
  {
    StringLine = MSG_ReadStringLine((msg_t *)msg);
    Com_sprintf(v4, 0x2000, "%s", StringLine);
    if ( *((_DWORD *)v4 + 2056) )
    {
      Com_Printf("Server settings:\n");
      if ( *StringLine )
      {
LABEL_8:
        v6 = 0;
        while ( *StringLine )
        {
          if ( *StringLine == 92 )
            ++StringLine;
          v7 = *StringLine;
          v8 = 0;
          while ( v7 )
          {
            ArgList[v8++] = v7;
            if ( v8 >= 1023 )
              break;
            v7 = *++StringLine;
            if ( v7 == 92 )
              break;
          }
          ArgList[v8] = 0;
          *(_DWORD *)&v16.ipx[8] = ArgList;
          if ( v6 )
            *(_DWORD *)&v16.ipx[4] = "%s\n";
          else
            *(_DWORD *)&v16.ipx[4] = "%-24s";
          Com_Printf(*(const char **)&v16.ipx[4], *(_DWORD *)&v16.ipx[8]);
          if ( ++v6 >= 2 )
          {
            if ( *StringLine )
              goto LABEL_8;
            break;
          }
        }
      }
    }
    len = strlen(v4);
    Com_sprintf(&v4[len], 0x2000 - len, "\\");
    if ( *((_DWORD *)v4 + 2056) )
    {
      Com_Printf("\nPlayers:\n");
      Com_Printf("num: score: ping: name:\n");
    }
    *(_DWORD *)v19 = 0;
    v9 = MSG_ReadStringLine((msg_t *)msg);
    if ( *v9 )
    {
      do
      {
        len = strlen(v4);
        /* off_5679D0 (0x005679D0) is "\\%s" in retail .rdata */
        Com_sprintf(&v4[len], 0x2000 - len, "\\%s", v9);
        if ( *((_DWORD *)v4 + 2056) )
        {
          v20 = 0;
          v21 = 0;
          sscanf(v9, "%d %d", &v21, &v20);
          v10 = strchr(v9, 32);
          if ( v10 && (v11 = strchr(v10 + 1, 32)) != 0 )
            v12 = v11 + 1;
          else
            v12 = "unknown";
          Com_Printf("%-2d   %-3d    %-3d   %s\n", *(_DWORD *)v19, v21, v20, v12);
        }
        v9 = MSG_ReadStringLine((msg_t *)msg);
        v13 = *v9;
        ++*(_DWORD *)v19;
      }
      while ( v13 );
    }
    len = strlen(v4);
    Com_sprintf(&v4[len], 0x2000 - len, "\\");
    if ( !sys_timeBaseInit )
    {
      sys_timeBase = timeGetTime();
      sys_timeBaseInit = 1;
    }
    *((_DWORD *)v4 + 2053) = timeGetTime() - sys_timeBase;
    v14 = *((_DWORD *)v4 + 2056);
    qmemcpy(v4 + 0x2000, &from, 0x14u);
    *((_DWORD *)v4 + 2055) = 0;
    if ( v14 )
      *((_DWORD *)v4 + 2057) = 1;
  }
}

/* ---- CL_LocalServers_f  0x00413710 ----  VERIFIED */
void CL_LocalServers_f( void ) {
	char *message;
	int i, j, len;
	netadr_t to;

	Com_Printf( "Scanning for servers on the local network...\n" );

	cls_numlocalservers = 0;
	cls_pingUpdateSource = AS_LOCAL;

	for ( i = 0; i < MAX_OTHER_SERVERS; i++ ) {
		qboolean b = cls_localServers[i].visible;
		Com_Memset( &cls_localServers[i], 0, sizeof( cls_localServers[i] ) );
		cls_localServers[i].visible = b;
	}
	Com_Memset( &to, 0, sizeof( to ) );

	message = "\377\377\377\377getinfo xxx";

	for ( i = 0; i < 2; i++ ) {
		for ( j = 0; j < NUM_SERVER_PORTS; j++ ) {
			to.port = BigShort( (short)( PORT_SERVER + j ) );
			to.type = NA_BROADCAST;

			len = strlen( message );
			NetProf_PrepProfiling( &pProf );
			NET_SendPacket( NS_CLIENT, len, message, to );
			if ( net_profile->integer ) {
				NetProf_PrepProfiling( &pProf );
				NetProf_AddPacket( &pProf->send, len, qfalse );
			}
		}
	}
}

/* ---- CL_GlobalServers_f  0x00413890 ----  [CONFIRMED] */
void __cdecl CL_GlobalServers_f()
{
  unsigned int v0;
  char *v1;
  char *v2;
  unsigned __int32 v3;
  signed __int32 v4;
  bool v5;
  char *v6;
  cvar_t *Var;
  netadr_t v8;
  netadr_t a;
  char Buffer[1024];
  unsigned int v11;
  unsigned int retaddr;

  v11 = retaddr ^ _security_cookie;
  v0 = 3;
  if ( cmd_argc >= 3 )
  {
    Com_Printf("Requesting servers from the master...\n");
    NET_StringToAdr("codmaster.activision.com", &a);
    cls_numglobalservers = -1;
    cls_pingUpdateSource = 1;
    a.type = NA_IP;
    a.port = BigShort( 20510 );
    v1 = &empty_string;
    if ( (unsigned int)cmd_argc > 2 )
      v1 = cmd_argv[2];
    sprintf(Buffer, "getservers %s", v1);
    v2 = &Buffer[strlen(Buffer)];
    v3 = cmd_argc;
    v4 = cmd_argc;
    if ( cmd_argc > 3 )
    {
      while ( 1 )
      {
        v5 = v0 < v3;
        v6 = &empty_string;
        if ( v5 )
          v6 = cmd_argv[v0];
        v2 += sprintf(v2, " %s", v6);
        if ( (int)++v0 >= v4 )
          break;
        v3 = cmd_argc;
      }
    }
    Var = Cvar_FindVar("fs_restrict");
    if ( Var )
    {
      if ( !((Var->value == 0.0) | __UNORDERED__(Var->value, 0.0)) )
        sprintf(v2, " demo");
    }
    qmemcpy(&v8, &a, sizeof(v8));
    NET_OutOfBandPrint(NS_SERVER, v8, Buffer);
  }
  else
  {
    Com_Printf("usage: globalservers <master# 0-1> <protocol> [keywords]\n");
  }
}

/* ---- CL_GetPing  0x004139F0 ----  VERIFIED */
void CL_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	const char *str;
	int time;
	int maxPing;

	if ( !cl_pinglist[n].adr.port ) {
		buf[0] = '\0';
		*pingtime = 0;
		return;
	}

	str = NET_AdrToString( cl_pinglist[n].adr );
	Q_strncpyz( buf, str, buflen );

	time = cl_pinglist[n].time;
	if ( !time ) {
		time = cls_realtime - cl_pinglist[n].start;
		maxPing = Cvar_VariableIntegerValue( "cl_maxPing" );
		if ( maxPing < 100 ) {
			maxPing = 100;
		}
		if ( time < maxPing ) {
			time = 0;
		}
	}

	CL_SetServerInfoByAddress( cl_pinglist[n].adr, cl_pinglist[n].info,
							   cl_pinglist[n].time );

	*pingtime = time;
}

/* ---- CL_UpdateServerInfo  0x00413AC0 ----  VERIFIED */
void CL_UpdateServerInfo( int n ) {
	if ( !cl_pinglist[n].adr.port ) {
		return;
	}

	CL_SetServerInfoByAddress( cl_pinglist[n].adr, cl_pinglist[n].info,
							   cl_pinglist[n].time );
}

/* ---- CL_GetPingInfo  0x00413B00 ----  VERIFIED */
void CL_GetPingInfo( int n, char *buf, int buflen ) {
	if ( !cl_pinglist[n].adr.port ) {
		if ( buflen ) {
			buf[0] = '\0';
		}
		return;
	}

	Q_strncpyz( buf, cl_pinglist[n].info, buflen );
}

/* ---- CL_ClearPing  0x00413B40 ----  VERIFIED */
void CL_ClearPing( int n ) {
	if ( n < 0 || n >= MAX_PINGREQUESTS ) {
		return;
	}

	cl_pinglist[n].adr.port = 0;
}

/* ---- CL_GetPingQueueCount  0x00413B60 ---- */
int __cdecl CL_GetPingQueueCount()
{
  int i;
  int count;
  ping_t *pingptr;

  count = 0;
  pingptr = cl_pinglist;

  for ( i = 0; i < MAX_PINGREQUESTS; i++, pingptr++ )
  {
    if ( pingptr->adr.port )
      count++;
  }
  return count;
}

/* ---- CL_GetFreePing  0x00413BD0 ----  [HIGH] */
void *__cdecl CL_GetFreePing()
{
  void *result;
  int i;
  int v2;
  _DWORD *v3;
  signed int v4;
  int v5;
  int v6;
  int v7;
  _DWORD *v8;
  int v9;
  _DWORD *v10;
  int v11;
  _DWORD *v12;
  int v13;
  _DWORD *v14;
  int v15;
  _DWORD *v16;
  int v17;
  _DWORD *v18;
  int v19;
  _DWORD *v20;

  result = &unk_15CA660;
  for ( i = 0; i < 16; ++i )
  {
    if ( !*((_WORD *)result + 9) )
      goto LABEL_26;
    v2 = *((_DWORD *)result + 6);
    if ( !v2 )
      v2 = cls_realtime - *((_DWORD *)result + 5);
    if ( v2 >= 500 )
    {
LABEL_26:
      *((_WORD *)result + 9) = 0;
      return result;
    }
    result = (char *)result + 1052;
  }
  v3 = &unk_15CA660;
  result = &unk_15CA660;
  v4 = 0x80000000;
  v5 = 2;
  do
  {
    v6 = v3[5];
    if ( cls_realtime - v6 > v4 )
    {
      v4 = cls_realtime - v6;
      result = v3;
    }
    v7 = v3[268];
    v8 = v3 + 263;
    if ( cls_realtime - v7 > v4 )
    {
      v4 = cls_realtime - v7;
      result = v8;
    }
    v9 = v8[268];
    v10 = v8 + 263;
    if ( cls_realtime - v9 > v4 )
    {
      v4 = cls_realtime - v9;
      result = v10;
    }
    v11 = v10[268];
    v12 = v10 + 263;
    if ( cls_realtime - v11 > v4 )
    {
      v4 = cls_realtime - v11;
      result = v12;
    }
    v13 = v12[268];
    v14 = v12 + 263;
    if ( cls_realtime - v13 > v4 )
    {
      v4 = cls_realtime - v13;
      result = v14;
    }
    v15 = v14[268];
    v16 = v14 + 263;
    if ( cls_realtime - v15 > v4 )
    {
      v4 = cls_realtime - v15;
      result = v16;
    }
    v17 = v16[268];
    v18 = v16 + 263;
    if ( cls_realtime - v17 > v4 )
    {
      v4 = cls_realtime - v17;
      result = v18;
    }
    v19 = v18[268];
    v20 = v18 + 263;
    if ( cls_realtime - v19 > v4 )
    {
      v4 = cls_realtime - v19;
      result = v20;
    }
    v3 = v20 + 263;
    --v5;
  }
  while ( v5 );
  return result;
}

/* ---- CL_Ping_f  0x00413D00 ----  [CONFIRMED] */
void __cdecl CL_Ping_f()
{
  _DWORD *FreePing;
  _BYTE v1[24];
  int v2;
  netadr_t a;
  unsigned int v4;
  unsigned int retaddr;

  v4 = retaddr ^ _security_cookie;
  if ( cmd_argc == 2 )
  {
    memset(&a, 0, sizeof(a));
    if ( NET_StringToAdr(cmd_argv[1], &a) )
    {
      FreePing = CL_GetFreePing();
      v2 = 0;
      *(_DWORD *)&v1[20] = 0;
      qmemcpy(FreePing, &a, 0x14u);
      FreePing[5] = cls_realtime;
      FreePing[6] = 0;
      qmemcpy(v1, &a, 0x14u);
      CL_SetServerInfoByAddress(*(netadr_t *)v1, *(const char **)&v1[20], v2);
      qmemcpy(&v1[4], &a, 0x14u);
      NET_OutOfBandPrint(NS_CLIENT, *(netadr_t *)&v1[4], "getinfo xxx");
    }
  }
  else
  {
    Com_Printf("usage: ping [server]\n");
  }
}

/* ---- CL_UpdateVisiblePings_f  0x00413DE0 ----  VERIFIED */
qboolean CL_UpdateVisiblePings_f( int source ) {
	int slots, i;
	char buff[MAX_STRING_CHARS];
	int pingTime;
	int max;
	qboolean status = qfalse;

	if ( source < AS_LOCAL || source > AS_FAVORITES ) {
		return qfalse;
	}

	cls_pingUpdateSource = source;

	slots = CL_GetPingQueueCount();
	if ( slots < MAX_PINGREQUESTS ) {
		serverInfo_t *server = NULL;

		max = ( source == AS_GLOBAL ) ? MAX_GLOBAL_SERVERS : MAX_OTHER_SERVERS;
		switch ( source ) {
		case AS_LOCAL:
			server = &cls_localServers[0];
			max = cls_numlocalservers;
			break;
		case AS_GLOBAL:
			server = &cls_globalServers[0];
			max = cls_numglobalservers;
			break;
		case AS_FAVORITES:
			server = &cls_favoriteServers[0];
			max = cls_numfavoriteservers;
			break;
		}
		for ( i = 0; i < max; i++ ) {
			if ( server[i].visible ) {
				if ( server[i].ping == -1 ) {
					int j;

					if ( slots >= MAX_PINGREQUESTS ) {
						break;
					}
					for ( j = 0; j < MAX_PINGREQUESTS; j++ ) {
						if ( !cl_pinglist[j].adr.port ) {
							continue;
						}
						if ( NET_CompareAdr( cl_pinglist[j].adr, server[i].adr ) ) {
							break;
						}
					}
					if ( j >= MAX_PINGREQUESTS ) {
						status = qtrue;
						for ( j = 0; j < MAX_PINGREQUESTS; j++ ) {
							if ( !cl_pinglist[j].adr.port ) {
								break;
							}
						}
						Com_Memcpy( &cl_pinglist[j].adr, &server[i].adr,
									sizeof( netadr_t ) );
						cl_pinglist[j].start = cls_realtime;
						cl_pinglist[j].time = 0;
						NET_OutOfBandPrint( NS_CLIENT, cl_pinglist[j].adr,
											"getinfo xxx" );
						slots++;
					}
				}
			}
		}
	}

	if ( slots ) {
		status = qtrue;
	}
	for ( i = 0; i < MAX_PINGREQUESTS; i++ ) {
		if ( !cl_pinglist[i].adr.port ) {
			continue;
		}
		CL_GetPing( i, buff, MAX_STRING_CHARS, &pingTime );
		if ( pingTime != 0 ) {
			CL_ClearPing( i );
			status = qtrue;
		}
	}

	return status;
}

/* ---- CL_ServerStatus_f  0x004140F0 ----  [CONFIRMED] */
void __cdecl CL_ServerStatus_f()
{
  const char *v0;
  _DWORD *ServerStatus;
  _BYTE v2[24];
  netadr_t a;
  unsigned int v4;
  unsigned int retaddr;

  v4 = retaddr ^ _security_cookie;
  Com_Memset((int *)&a, 0, 0x14u);
  if ( cmd_argc == 2 )
  {
    v0 = cmd_argv[1];
  }
  else
  {
    if ( cls_state != CA_ACTIVE || clc_demoplaying )
    {
      Com_Printf("Not connected to a server.\n");
      Com_Printf("Usage: serverstatus [server]\n");
      return;
    }
    v0 = cls_servername;
  }
  if ( NET_StringToAdr(v0, &a) )
  {
    qmemcpy(v2, &a, 0x14u);
    NET_OutOfBandPrint(NS_CLIENT, *(netadr_t *)v2, "getstatus");
    qmemcpy(&v2[4], &a, 0x14u);
    ServerStatus = (_DWORD *)CL_GetServerStatus(*(netadr_t *)&v2[4]);
    qmemcpy(ServerStatus + 2048, &a, 0x14u);
    ServerStatus[2056] = 1;
    ServerStatus[2055] = 1;
  }
}

/* ---- CL_ShowIP_f  0x004141E0 ----  [CONFIRMED] */
void __cdecl CL_ShowIP_f()
{
  Sys_ShowIP();
}

/* ---- CL_GetServerIPAddress  0x004141F0 ----  VERIFIED */
char *CL_GetServerIPAddress( void ) {
	char *s = (char *)unk_87A710;

	if ( cls_state < CA_CONNECTED ) {
		Com_Memset( s, 0, 128 );
		return s;
	}

	Com_sprintf( s, 128, "%i.%i.%i.%i:%i",
				 ( (netadr_t *)clc_serverAddress )->ip[0],
				 ( (netadr_t *)clc_serverAddress )->ip[1],
				 ( (netadr_t *)clc_serverAddress )->ip[2],
				 ( (netadr_t *)clc_serverAddress )->ip[3],
				 BigShort( ( (netadr_t *)clc_serverAddress )->port ) );
	return s;
}

/* ---- CL_CDKeyValidate  0x00414260 ----  [HIGH] */
qboolean __cdecl CL_CDKeyValidate( const char *key, const char *checksum )
{
	unsigned int crc;
	int i, bit;
	char buffer[16];

	crc = 0;
	for ( i = 0; i < 16; ++i ) {
		crc ^= (unsigned int) (int) (signed char) key[i];
		for ( bit = 8; bit; --bit ) {
			if ( crc & 1 ) {
				crc ^= 0x14002u;
			}
			crc >>= 1;
		}
	}

	Com_sprintf( buffer, sizeof( buffer ), "%04x", crc );

	if ( !checksum ) {
		return qtrue;
	}
	/* CoD 1.5 compares only this key's four checksum characters. fs_game
	 * appends the mod checksum at +4, replacing the base slot's terminator. */
	return Q_stricmpn( checksum, buffer, 4 ) == 0 ? qtrue : qfalse;
}

/* ---- CL_SetupForNewServerMap  0x004142F0 ----  VERIFIED */
void CL_SetupForNewServerMap( const char *mapname, const char *gametype )
{
	Com_Printf( "Server changing map %s, gametype %s\n", mapname, gametype );
	Cvar_Set2( "cl_serverloadmap", mapname, qtrue );
	Cvar_Set2( "cl_serverloadgametype", gametype, qtrue );
	Cvar_Set2( "cl_serverloadwaiting", "0", qtrue );
}

/* ---- CL_AddDebugString  0x00414340 ----  VERIFIED */
void CL_AddDebugString( const float *origin, const float *color, float scale,
                        const char *text, int fromServer )
{
	int index;
	char *rec;
	float *str;

	if ( !cls_rendererStarted ) {
		return;
	}

	index = cls_debugStringCount;
	cls_debugStringCapacity = 256;
	if ( index + 1 > 256 ) {
		return;
	}

	if ( !dword_15CA640 ) {
		dword_15CA640 = Z_MallocInternal( 0x8000 );
		cls_debugStringFromServer = Z_MallocInternal( cls_debugStringCapacity );
		index = 0;
		cls_debugStringCount = 0;
	}

	rec = (char *)dword_15CA640 + 128 * index;
	str = (float *)rec;

	str[0] = origin[0];
	str[1] = origin[1];
	str[2] = origin[2];

	str[3] = color[0];
	str[4] = color[1];
	str[5] = color[2];
	str[6] = color[3];

	str[7] = scale;

	strncpy( rec + 0x20, text, 0x5F );
	rec[0x7F] = 0;

	((unsigned char *)cls_debugStringFromServer)[cls_debugStringCount] = (unsigned char)fromServer;
	++cls_debugStringCount;
}

/* ---- CL_AddDebugLine  0x00414410 ----  VERIFIED */
void CL_AddDebugLine( const float *start, const float *end, const float *color,
                      int depthTest, int duration, int fromServer )
{
	int index;
	char *rec;
	float *line;

	if ( !cls_rendererStarted ) {
		return;
	}

	index = cls_debugLineCount;
	cls_debugLineCapacity = 4096;
	if ( index + 1 > 4096 ) {
		return;
	}

	if ( !dword_15CA650 ) {
		dword_15CA650 = Z_MallocInternal( 0x2C000 );
		cls_debugLineFromServer = Z_MallocInternal( cls_debugLineCapacity );
		cls_debugLineDurations = Z_MallocInternal( 4 * cls_debugLineCapacity );
		index = 0;
		cls_debugLineCount = 0;
	}

	rec = (char *)dword_15CA650 + 44 * index;
	line = (float *)rec;

	line[0] = start[0];
	line[1] = start[1];
	line[2] = start[2];

	line[3] = end[0];
	line[4] = end[1];
	line[5] = end[2];

	line[6] = color[0];
	line[7] = color[1];
	line[8] = color[2];
	line[9] = color[3];

	*(int *)( rec + 0x28 ) = depthTest;

	((unsigned char *)cls_debugLineFromServer)[cls_debugLineCount] = (unsigned char)fromServer;
	((int *)cls_debugLineDurations)[cls_debugLineCount] = duration;
	++cls_debugLineCount;
}

/* ---- CL_FlushDebugData  0x00414500 ----  VERIFIED */
void CL_FlushDebugData( int fromServer )
{
	int i;
	int stringCount;
	int lineCount;
	unsigned char *strFromServer;

	if ( !cls_rendererStarted ) {
		return;
	}

	if ( dword_15CA640 ) {
		stringCount = cls_debugStringCount;
		i = 0;
		if ( stringCount > 0 ) {
			strFromServer = (unsigned char *)cls_debugStringFromServer;
			do {
				if ( strFromServer[i] == fromServer ) {
					cls_debugStringCount = stringCount - 1;
					strFromServer[i] = strFromServer[stringCount - 1];
					memcpy( (char *)dword_15CA640 + 128 * i,
							(char *)dword_15CA640 + 128 * cls_debugStringCount, 0x80 );
					strFromServer = (unsigned char *)cls_debugStringFromServer;
					stringCount = cls_debugStringCount;
				} else {
					++i;
				}
			} while ( i < stringCount );
		}
		dword_1432920( dword_15CA640, stringCount );
	}

	if ( dword_15CA650 ) {
		lineCount = cls_debugLineCount;
		i = 0;
		if ( lineCount > 0 ) {
			do {
				if ( ( (unsigned char *)cls_debugLineFromServer )[i] == fromServer
					&& --( (int *)cls_debugLineDurations )[i] <= 0 ) {
					--cls_debugLineCount;
					( (unsigned char *)cls_debugLineFromServer )[i] =
						( (unsigned char *)cls_debugLineFromServer )[cls_debugLineCount];
					( (int *)cls_debugLineDurations )[i] =
						( (int *)cls_debugLineDurations )[cls_debugLineCount];
					memcpy( (char *)dword_15CA650 + 44 * i,
							(char *)dword_15CA650 + 44 * cls_debugLineCount, 0x2C );
				} else {
					++i;
				}
				lineCount = cls_debugLineCount;
			} while ( i < lineCount );
		}
		dword_1432924( dword_15CA650, lineCount );
	}
}

/* ---- CL_UpdateDebugData  0x00414640 ----  [CONFIRMED] */
void __cdecl CL_UpdateDebugData()
{
  if ( cls_rendererStarted )
  {
    if ( dword_15CA640 )
      dword_1432920(dword_15CA640, cls_debugStringCount);
    if ( dword_15CA650 )
      dword_1432924(dword_15CA650, cls_debugLineCount);
  }
}

/* ---- CL_ShutdownDebugData  0x00414680 ----  [CONFIRMED] */
void __cdecl CL_ShutdownDebugData(void *msg)
{
  if ( dword_15CA650 )
  {
    free(dword_15CA650);
    dword_15CA650 = 0;
  }
  if ( cls_debugLineFromServer )
  {
    free(cls_debugLineFromServer);
    cls_debugLineFromServer = 0;
  }
  if ( cls_debugLineDurations )
  {
    free(cls_debugLineDurations);
    cls_debugLineDurations = 0;
  }
  if ( dword_15CA640 )
  {
    free(dword_15CA640);
    dword_15CA640 = 0;
  }
  if ( cls_debugStringFromServer )
    free(cls_debugStringFromServer);
  cls_debugStringCapacity = 0;
  cls_debugStringCount = 0;
  dword_15CA640 = 0;
  cls_debugStringFromServer = 0;
  cls_debugLineCapacity = 0;
  cls_debugLineCount = 0;
  dword_15CA650 = 0;
  cls_debugLineFromServer = 0;
  cls_debugLineDurations = 0;
}

/* ---- CL_ForwardCommandToServer  0x0040F0C0 ----  [CONFIRMED] */
void CL_ForwardCommandToServer( const char *string ) {
	const char *cmd;

	cmd = Cmd_Argc() ? Cmd_Argv( 0 ) : "";

	if ( Cmd_Argc() && cmd[0] == '-' ) {
		return;
	}

	if ( clc_demoplaying || cls_state < CA_CONNECTED || cmd[0] == '+' ) {
		Com_Printf( "Unknown command \"%s\"\n", cmd );
		return;
	}

	if ( Cmd_Argc() > 1 ) {
		CL_AddReliableCommand( string );
	} else {
		CL_AddReliableCommand( cmd );
	}
}

extern unsigned char chan[32832];       /* 0x015EF020 clc.netchan */
extern int  clc_lastPacketTime;
extern int  clc_reliableAcknowledge;
extern int  clc_serverMessageSequence;
extern int  clc_reliableSequence;
extern int  clc_demorecording;
extern int  clc_demowaiting;

extern qboolean    NET_CompareAdr( netadr_t a, netadr_t b );
extern qboolean    Netchan_Process( netchan_t *chan, msg_t *msg );
extern int         MSG_ReadLong( msg_t *msg );
extern void        CL_ConnectionlessPacket( netadr_t from, msg_t *msg );
extern int         CL_Netchan_Decode();
extern void        CL_WriteDemoMessage( msg_t *msg, int headerBytes );

/* ---- CL_PacketEvent  0x00410FF0 ----  VERIFIED */
void CL_PacketEvent( netadr_t from, msg_t *msg ) {
	int headerBytes;

	if ( !msg ) {
		return;
	}

	if ( msg->cursize >= 4 && *(int *)msg->data == -1 ) {
		CL_ConnectionlessPacket( from, msg );
		return;
	}

	if ( cls_state < CA_CONNECTED ) {
		return;
	}

	if ( msg->cursize < 4 ) {
		Com_Printf( "%s: Runt packet\n", NET_AdrToString( from ) );
		return;
	}

	if ( !NET_CompareAdr( from, *(netadr_t *)( chan + 8 ) ) ) {
		Com_DPrintf( "%s:sequenced packet without connection\n",
		             NET_AdrToString( from ) );
		return;
	}

	clc_lastPacketTime = cls_realtime;

	if ( !Netchan_Process( (netchan_t *) chan, msg ) ) {
		return;
	}

	clc_serverMessageSequence = *(int *)msg->data;

	headerBytes = msg->readcount;
	clc_reliableAcknowledge = MSG_ReadLong( msg );
	if ( clc_reliableAcknowledge < clc_reliableSequence - 64 ) {
		clc_reliableAcknowledge = clc_reliableSequence;
		return;
	}

	CL_Netchan_Decode( msg->data + msg->readcount,
	                   msg->cursize - msg->readcount );
	CL_ParseServerMessage( msg );

	if ( clc_demorecording && !clc_demowaiting ) {
		CL_WriteDemoMessage( msg, headerBytes );
	}
}

/* ---- CL_Frame  0x00411280 ----  VERIFIED */
void CL_Frame( int msec ) {
	extern cvar_t *com_cl_running;
	extern cvar_t *com_sv_running;
	extern cvar_t *com_timescale;

	extern cvar_t *cl_avidemo;              /* 0x0143285C */
	extern cvar_t *cl_forceavidemo;         /* 0x0155F23C */

	extern int cls_cddialog;                /* 0x0155F2C8 */
	extern int xanim_activePoolSlot;
#define xanim_activePoolSlot xanim_activePoolSlot
	extern int dword_8E3C48;                /* 0x008E3C48, reliable-cmd probe gate */
	extern int cls_frametime;               /* 0x0155F3DC, msec mirror */
	extern int cls_realFrametime;               /* 0x0155F3E4, msec mirror -- Con_RunConsole reads this one */
	extern int CL_handle;                   /* 0x0057C118, current cinematic handle */
	extern int con_conspeed;                /* 0x0142F5E0-ish; scr_conspeed, set by Con_Init */

	extern void CL_ChangeReliableCommand( void );   /* 0x0040E340 */
	extern void CL_CheckUserinfo( void );           /* 0x004111C0 */
	extern void CL_CheckTimeout( void );            /* 0x00411130 */
	extern void CL_SendCmd( void );                 /* 0x0040BF30 */
	extern void CL_CheckForResend( void );          /* 0x004103D0 */
	extern void CL_SetCGameTime( void );            /* 0x00404D60 */
	extern void CL_UpdateInGameState( void );       /* 0x00411220 */
	extern void MSS_Update( void );                 /* 0x0044F920 */
	extern int CIN_RunCinematic( int handle );      /* 0x00407680, returns e_status */
	extern void Con_RunConsole( void );             /* 0x0040A260 */

	if ( !com_cl_running || !com_cl_running->integer ) {
		return;
	}

	xanim_activePoolSlot = 0;

	if ( cls_cddialog ) {
		cls_cddialog = 0;
		if ( uivm ) {
			VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_NEED_CD );
		}
	} else if ( cls_state == CA_DISCONNECTED
	         && !( cls_keyCatchers & KEYCATCH_UI )
	         && com_sv_running && !com_sv_running->integer ) {
		MSS_StopSounds( 0 );
		if ( uivm ) {
			VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
		}
	}

	if ( cl_avidemo && cl_avidemo->integer && msec ) {
		if ( cls_state == CA_ACTIVE
		  || ( cl_forceavidemo && cl_forceavidemo->integer ) ) {
			Cbuf_ExecuteText( EXEC_NOW, "screenshot silent\n" );
		}
		/* flt_569090 = 0x447A0000 = 1000.0f, read out of .rdata */
		msec = (int) ( 1000.0f / cl_avidemo->integer
		               * ( com_timescale ? com_timescale->value : 1.0f ) );
		if ( !msec ) {
			msec = 1;
		}
	}

	if ( dword_8E3C48 && ( cls_framecount & 0xFF ) == 0 ) {
		if ( rand() * ( 1.0f / 32768.0f ) < 0.1 ) {
			CL_ChangeReliableCommand();
		}
	}

	cls_realtime += msec;
	cls_realFrametime = msec;
	cls_frametime = msec;

	if ( cl_timegraph && ((cvar_t *)cl_timegraph)->integer ) {
		/* TODO: omitted. The ring at 0x0087A7D0 is 8 KB and the header reserves four bytes for it. */
	}

	CL_CheckUserinfo();
	CL_CheckTimeout();
	CL_SendCmd();
	CL_CheckForResend();
	CL_SetCGameTime();
	CL_UpdateInGameState();

	SCR_UpdateScreen();

	MSS_Update();

	if ( CL_handle >= 0 && CL_handle < MAX_VIDEO_HANDLES ) {
		CIN_RunCinematic( CL_handle );
	}

	if ( con_conspeed ) {
		Con_RunConsole();
	}

	cls_framecount++;
}

/* ---- CL_Init  0x00411E60 ----  [CONFIRMED] */
void CL_Init( void ) {
	Com_Printf( "----- Client Initialization -----\n" );

	Con_Init();

	cls_state = CA_DISCONNECTED;
	cls_realtime = 0;

	CL_InitInput();

	cl_noprint = Cvar_Get( "cl_noprint", "0", 0 );
	cl_motd = Cvar_Get( "cl_motd", "1", 0 );
	cl_timeout = Cvar_Get( "cl_timeout", "200", 0 );
	cl_timeNudge = Cvar_Get( "cl_timeNudge", "0", CVAR_TEMP );
	cl_shownet = Cvar_Get( "cl_shownet", "0", CVAR_TEMP );
	cl_shownuments = Cvar_Get( "cl_shownuments", "0", CVAR_TEMP );
	cl_visibleClients = Cvar_Get( "cl_visibleClients", "0", CVAR_TEMP );
	cl_showServerCommands = Cvar_Get( "cl_showServerCommands", "0", 0 );
	cl_showSend = Cvar_Get( "cl_showSend", "0", CVAR_TEMP );
	cl_showTimeDelta = Cvar_Get( "cl_showTimeDelta", "0", CVAR_TEMP );
	cl_freezeDemo = Cvar_Get( "cl_freezeDemo", "0", CVAR_TEMP );
	rconPassword = Cvar_Get( "rconPassword", "", CVAR_TEMP );
	activeAction = Cvar_Get( "activeAction", "", CVAR_TEMP );
	timedemo = Cvar_Get( "timedemo", "0", 0 );
	cl_avidemo = Cvar_Get( "cl_avidemo", "0", 0 );
	cl_forceavidemo = Cvar_Get( "cl_forceavidemo", "0", 0 );
	rconAddress = Cvar_Get( "rconAddress", "", 0 );
	cl_yawspeed = Cvar_Get( "cl_yawspeed", "140", CVAR_ARCHIVE );
	cl_pitchspeed = Cvar_Get( "cl_pitchspeed", "140", CVAR_ARCHIVE );
	cl_anglespeedkey = Cvar_Get( "cl_anglespeedkey", "1.5", 0 );
	cl_maxpackets = Cvar_Get( "cl_maxpackets", "30", CVAR_ARCHIVE );
	cl_packetdup = Cvar_Get( "cl_packetdup", "1", CVAR_ARCHIVE );
	cl_run = Cvar_Get( "cl_run", "1", CVAR_TEMP );
	cl_stance = Cvar_Get( "cl_stance", "0", CVAR_TEMP );
	cl_stanceTemp = Cvar_Get( "cl_stanceTemp", "0", CVAR_TEMP );
	cl_goStandJumpTime = Cvar_Get( "cl_goStandJumpTime", "0", CVAR_ARCHIVE );
	cl_sensitivity = Cvar_Get( "sensitivity", "5", CVAR_ARCHIVE );
	cl_mouseAccel = Cvar_Get( "cl_mouseAccel", "0", CVAR_ARCHIVE );
	cl_freelook = Cvar_Get( "cl_freelook", "1", CVAR_ARCHIVE );
	cl_showmouserate = Cvar_Get( "cl_showmouserate", "0", 0 );
	cl_allowDownload = Cvar_Get( "cl_allowDownload", "0", CVAR_ARCHIVE );
	cl_conXOffset = Cvar_Get( "cl_conXOffset", "0", 0 );
	r_inGameVideo = Cvar_Get( "r_inGameVideo", "1", CVAR_ARCHIVE );
	cl_serverStatusResendTime = Cvar_Get( "cl_serverStatusResendTime", "750", 0 );
	cl_viewPitchCompensate = Cvar_Get( "cl_viewPitchCompensate", "0", CVAR_ROM );
	cl_viewYawCompensate = Cvar_Get( "cl_viewYawCompensate", "0", CVAR_ROM );
	cl_bypassMouseInput = Cvar_Get( "cl_bypassMouseInput", "0", 0 );
	m_pitch = Cvar_Get( "m_pitch", "0.022", CVAR_ARCHIVE );
	m_yaw = Cvar_Get( "m_yaw", "0.022", CVAR_ARCHIVE );
	m_forward = Cvar_Get( "m_forward", "0.25", CVAR_ARCHIVE );
	m_side = Cvar_Get( "m_side", "0.25", CVAR_ARCHIVE );
	m_filter = Cvar_Get( "m_filter", "0", CVAR_ARCHIVE );
	cl_motdString = Cvar_Get( "cl_motdString", "", CVAR_ROM );
	cl_ingame = Cvar_Get( "cl_ingame", "0", CVAR_ROM );
	Cvar_Get( "cl_maxPing", "800", CVAR_ARCHIVE );

	Cvar_Get( "cg_drawCompass", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_drawNotifyText", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_descriptiveText", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_drawTeamOverlay", "2", CVAR_ARCHIVE );
	Cvar_Get( "cg_drawGun", "1", CVAR_ARCHIVE );
	Cvar_Get( "cg_cursorHints", "4", CVAR_ARCHIVE );
	Cvar_Get( "cg_voiceSpriteTime", "6000", CVAR_ARCHIVE );
	Cvar_Get( "cg_teamChatsOnly", "0", CVAR_ARCHIVE );
	Cvar_Get( "cg_noVoiceChats", "0", CVAR_ARCHIVE );
	Cvar_Get( "cg_noVoiceText", "0", CVAR_ARCHIVE );
	Cvar_Get( "cg_crosshairSize", "48", CVAR_ARCHIVE );
	Cvar_Get( "cg_drawCrosshair", "1", CVAR_ARCHIVE );

	Cvar_Get( "name", "Unknown Soldier", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "rate", "5000", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "snaps", "20", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "model", "", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "head", "", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "handicap", "100", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "cl_anonymous", "0", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "password", "", CVAR_USERINFO );
	Cvar_Get( "cg_predictItems", "1", CVAR_ARCHIVE | CVAR_USERINFO );
	Cvar_Get( "cg_viewsize", "100", CVAR_ARCHIVE );

	cl_waitForFire = Cvar_Get( "cl_waitForFire", "0", CVAR_ROM );

	fx_enable = Cvar_Get( "fx_enable", "1", CVAR_CHEAT );
	fx_draw = Cvar_Get( "fx_draw", "1", CVAR_CHEAT );
	fx_cull = Cvar_Get( "fx_cull", "1", 0 );
	fx_freeze = Cvar_Get( "fx_freeze", "0", CVAR_CHEAT );
	fx_debug = Cvar_Get( "fx_debug", "0", CVAR_CHEAT );
	fx_debugBolt = Cvar_Get( "fx_debugBolt", "0", CVAR_CHEAT );
	fx_count = Cvar_Get( "fx_count", "0", CVAR_CHEAT );

	cl_updateavailable = Cvar_Get( "cl_updateavailable", "0", CVAR_ROM );
	cl_updatefiles = Cvar_Get( "cl_updatefiles", "", CVAR_ROM );
	cl_updateoldversion = Cvar_Get( "cl_updateoldversion", "", CVAR_ROM );
	cl_updateversion = Cvar_Get( "cl_updateversion", "", CVAR_ROM );

	cl_serverloadmap = Cvar_Get( "cl_serverloadmap", "", CVAR_ROM );
	cl_serverloadgametype = Cvar_Get( "cl_serverloadgametype", "", CVAR_ROM );
	cl_serverloadwaiting = Cvar_Get( "cl_serverloadwaiting", "0", CVAR_ROM );

	Cmd_AddCommand( "cmd", CL_ForwardToServer_f );
	Cmd_AddCommand( "configstrings", CL_Configstrings_f );
	Cmd_AddCommand( "clientinfo", CL_Clientinfo_f );
	Cmd_AddCommand( "snd_restart", CL_Snd_Restart_f );
	Cmd_AddCommand( "vid_restart", CL_Vid_Restart_f );
	Cmd_AddCommand( "disconnect", CL_Disconnect_f );
	Cmd_AddCommand( "record", CL_Record_f );
	Cmd_AddCommand( "demo", CL_PlayDemo_f );
	Cmd_AddCommand( "cinematic", CL_PlayCinematic_f );
	Cmd_AddCommand( "logo", CL_PlayLogo_f );
	Cmd_AddCommand( "stoprecord", CL_StopRecord_f );
	Cmd_AddCommand( "connect", CL_Connect_f );
	Cmd_AddCommand( "reconnect", CL_Reconnect_f );
	Cmd_AddCommand( "localservers", CL_LocalServers_f );
	Cmd_AddCommand( "globalservers", CL_GlobalServers_f );
	Cmd_AddCommand( "rcon", CL_Rcon_f );
	Cmd_AddCommand( "setenv", CL_Setenv_f );
	Cmd_AddCommand( "ping", CL_Ping_f );
	Cmd_AddCommand( "serverstatus", CL_ServerStatus_f );
	Cmd_AddCommand( "showip", CL_ShowIP_f );
	Cmd_AddCommand( "fs_openedList", CL_OpenedPK3List_f );
	Cmd_AddCommand( "fs_referencedList", CL_ReferencedPK3List_f );
	Cmd_AddCommand( "updatehunkusage", CL_UpdateLevelHunkUsage );
	Cmd_AddCommand( "updatescreen", SCR_UpdateScreen );
	Cmd_AddCommand( "startSingleplayer", CL_startSingleplayer_f );
	Cmd_AddCommand( "setRecommended", CL_SetRecommended_f );
	Cmd_AddCommand( "cubemapShot", CL_CubeMapShot_f );
	Cmd_AddCommand( "localizeSoundAliasFiles", Com_WriteLocalizedSoundAliasFiles );

	CL_InitRef();
	SCR_Init();

	Cvar_Set2( "cl_running", "1", qtrue );

	Com_Printf( "----- Client Initialization Complete -----\n" );
}

/* ---- CL_Shutdown  0x004127F0 ----  [CONFIRMED] */
void CL_Shutdown( void ) {
	static int recursive = 0;

	Com_Printf( "----- CL_Shutdown -----\n" );

	if ( recursive ) {
		printf( "recursive shutdown\n" );
		return;
	}
	recursive = 1;

	/* 0x00412826: retail disconnects the client before tearing it down -- push 1;
	 * call CL_Disconnect, right after CL_ShutdownDebugData and before
	 * CL_ShutdownCGame.  CL_Disconnect(qtrue) queues the "disconnect" reliable
	 * command and flushes it with three CL_WritePacket calls, so a quitting
	 * player drops from the server at once instead of lingering until
	 * SV_CheckTimeouts fires the sv_timeout path. */
	CL_Disconnect( qtrue );

	CL_ShutdownCGame();
	/* Retail inlines CL_ShutdownRef here -- 0x00412838 loads the refexport
	 * slot and 0x00412841 is `push 1`, so RE_Shutdown(1) reaches GLimp_Shutdown
	 * and WG_RestoreGamma.  CL_Shutdown is the ONLY site in the binary passing
	 * 1; every other caller passes 0 and leaves the window up. */
	CL_ShutdownRef();
	CL_ShutdownUI();

	Cmd_RemoveCommand( "cmd" );
	Cmd_RemoveCommand( "configstrings" );
	Cmd_RemoveCommand( "clientinfo" );
	Cmd_RemoveCommand( "snd_restart" );
	Cmd_RemoveCommand( "vid_restart" );
	Cmd_RemoveCommand( "disconnect" );
	Cmd_RemoveCommand( "record" );
	Cmd_RemoveCommand( "demo" );
	Cmd_RemoveCommand( "cinematic" );
	Cmd_RemoveCommand( "stoprecord" );
	Cmd_RemoveCommand( "connect" );
	Cmd_RemoveCommand( "reconnect" );
	Cmd_RemoveCommand( "localservers" );
	Cmd_RemoveCommand( "globalservers" );
	Cmd_RemoveCommand( "rcon" );
	Cmd_RemoveCommand( "setenv" );
	Cmd_RemoveCommand( "ping" );
	Cmd_RemoveCommand( "serverstatus" );
	Cmd_RemoveCommand( "showip" );
	Cmd_RemoveCommand( "fs_openedList" );
	Cmd_RemoveCommand( "fs_referencedList" );
	Cmd_RemoveCommand( "updatehunkusage" );
	Cmd_RemoveCommand( "updatescreen" );
	Cmd_RemoveCommand( "SaveTranslations" );
	Cmd_RemoveCommand( "SaveNewTranslations" );
	Cmd_RemoveCommand( "LoadTranslations" );
	Cmd_RemoveCommand( "startSingleplayer" );
	Cmd_RemoveCommand( "buyNow" );
	Cmd_RemoveCommand( "singlePlayLink" );
	Cmd_RemoveCommand( "setRecommended" );
	Cmd_RemoveCommand( "cubemapShot" );

	Cvar_Set2( "cl_running", "0", qtrue );

	cls_state = CA_DISCONNECTED;
	cls_keyCatchers = 0;

	recursive = 0;
	Com_Printf( "-----------------------\n" );
}
