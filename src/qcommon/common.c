/*
 * qcommon/common.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/common.c
 *
 * Retail range 0x004355B0-0x00438EB0.
 *
 * @fidelity: verified
 */

#include "qcommon.h"


#include <setjmp.h>
#include <time.h>
#include "../qcommon/hexrays_shim.h"

#define MAX_CONSOLE_LINES   32
#define MAX_PUSHED_EVENTS   256
#define MAXPRINTMSG         4096

cvar_t  *com_developer;
cvar_t  *com_developer_script;
cvar_t  *com_dedicated;
cvar_t  *com_speeds;
cvar_t  *com_timescale;
cvar_t  *com_fixedtime;
cvar_t  *com_viewlog;
cvar_t  *com_maxfps;
cvar_t  *com_logfile;
cvar_t  *com_statmon;
cvar_t  *com_journal;
cvar_t  *com_sv_running;
cvar_t  *com_cl_running;
cvar_t  *com_introplayed;
cvar_t  *com_animCheck;
cvar_t  *com_version;
cvar_t  *com_shortversion;
cvar_t  *com_recommendedSet;
cvar_t  *sv_paused;
cvar_t  *cl_paused;

int com_frameTime;
int com_frameNumber;
int com_lastFrameTime;
int com_lastStatmonTime;
qboolean com_errorEntered;
qboolean com_fullyInitialized;
qboolean com_animCheckCached;
char com_errorMessage[MAXPRINTMSG];

int com_skelTimeStamp;
int com_skelInvalidated;
int com_skelInvalidated2;

fileHandle_t com_journalFile;
fileHandle_t com_journalDataFile;

const char *com_quitReason;

static jmp_buf abortframe;
static int com_errorCode;
extern int scr_updatingScreen;

static char     *com_consoleLines[MAX_CONSOLE_LINES];
static int com_numConsoleLines;

static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];
static int com_pushedEventsHead;
static int com_pushedEventsTail;
static qboolean com_eventOverflowWarned;

static char     *rd_buffer;
static int rd_buffersize;
static void ( *rd_flush )( char *buffer );

static fileHandle_t logfile;
static qboolean opening_qconsole;

static int lastErrorTime;
static int errorCount;

static field_t  *completionField;
static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int matchCount;

/* ---- Com_BeginRedirect  0x004355B0 ---- */
void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char * ) ) {
	if ( !buffer || !buffersize || !flush ) {
		return;
	}
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

/* ---- Com_EndRedirect  0x004355E0 ---- */
void Com_EndRedirect( void ) {
	if ( rd_flush ) {
		rd_flush( rd_buffer );
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

/* ---- Com_PrintMessage  0x00435620 ---- */
void Com_PrintMessage( int channel, const char *msg ) {
	Cod1_HangWatchdogNote( msg );

	if ( rd_buffer ) {
		if ( channel != 4 ) {
			if ( ( strlen( msg ) + strlen( rd_buffer ) ) > ( rd_buffersize - 1 ) ) {
				rd_flush( rd_buffer );
				*rd_buffer = 0;
			}
			Q_strcat( rd_buffer, rd_buffersize, msg );
		}
		return;
	}

	if ( channel != 4 ) {
		if ( com_dedicated && !com_dedicated->integer ) {
			CL_ConsolePrint( msg, channel, 0, 0 );
		}
		Conbuf_AppendText( msg );
	}

	if ( com_logfile && com_logfile->integer ) {
		if ( !logfile && FS_Initialized() && !opening_qconsole ) {
			struct tm *newtime;
			time_t aclock;

			opening_qconsole = qtrue;

			time( &aclock );
			newtime = localtime( &aclock );

			logfile = FS_FOpenTextFileWrite( "console_mp.log" );
			Com_Printf( "logfile opened on %s\n", asctime( newtime ) );
			if ( com_logfile->integer > 1 ) {
				FS_ForceFlush( logfile );
			}

			opening_qconsole = qfalse;
		}
		if ( logfile && FS_Initialized() ) {
			FS_Write( msg, strlen( msg ), logfile );
		}
	}
}

/* ---- Com_Printf  0x004357B0 ---- */
void QDECL Com_Printf( const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG + 4];

	va_start( argptr, fmt );
	_vsnprintf( msg, MAXPRINTMSG, fmt, argptr );
	va_end( argptr );
	msg[MAXPRINTMSG - 1] = 0;

	Com_PrintMessage( 0, msg );
}

/* ---- Com_DPrintf  0x00435820 ---- */
void QDECL Com_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char msg[MAXPRINTMSG];

	if ( !com_developer || !com_developer->integer ) {
		return;
	}

	va_start( argptr, fmt );
	_vsnprintf( msg, MAXPRINTMSG, fmt, argptr );
	va_end( argptr );
	msg[MAXPRINTMSG - 1] = 0;

	Com_Printf( "%s", msg );
}

extern int ( *com_serverEndpoint )();

/* ---- Com_Shutdown  0x004358A0 ---- */
void Com_Shutdown( const char *finalmsg ) {
	CL_Disconnect( qtrue );
	CL_ShutdownCGame();
	CL_ShutdownUI();

	if ( com_serverEndpoint ) {
		com_serverEndpoint( 0 );
	}

	cls_loadingPlaque = 0;
	cls_rendererStarted = 0;

	SV_Shutdown( finalmsg );
	CL_ShutdownCGame();
	CL_ShutdownUI();
	SV_ShutdownGameProgs();
	CIN_CloseAllVideos();

	Hunk_ClearToStart();
	VM_Clear();
	CL_StartHunkUsers();
}

/* ---- Com_ErrorCleanup  0x00435910 ---- */
void Com_ErrorCleanup( void ) {
	switch ( com_errorCode ) {
	case ERR_SERVERDISCONNECT:
		Com_Shutdown( "EXE_DISCONNECTEDFROMOWNLISTENSERVER" );
		com_errorEntered = qfalse;
		Com_Restart();
		return;

	case ERR_AUTOUPDATE:
		Com_Shutdown( "EXE_ENDOFGAME" );
		com_errorEntered = qfalse;
		Com_Restart();
		return;

	case ERR_DROP:
	case ERR_DISCONNECT:
		Com_Printf( "********************\nERROR: %s\n********************\n", com_errorMessage );
		if ( com_errorCode == ERR_DROP && ( !com_dedicated || !com_dedicated->integer ) ) {
			CL_ConsoleFixPosition( 0 );
		}
		Com_Shutdown( com_errorMessage );
		com_errorEntered = qfalse;
		if ( com_errorCode == ERR_DROP && Cvar_Get( "r_vc_compile", "0", 0 )->integer == 2 ) {
			Com_Quit_f();
		}
		Com_Restart();
		return;

	case ERR_NEED_CD:
		Com_Shutdown( "EXE_SERVERDIDNTHAVECD" );
		if ( com_cl_running && com_cl_running->integer ) {
			com_errorEntered = qfalse;
			cls_cddialog = 1;               /* retail 0x0043597F */
		} else {
			Com_Printf( "Server didn't have CD\n" );
		}
		Com_Restart();
		return;

	default:
		Com_Restart();
		return;
	}
}

/* ---- Com_CleanupSkeletons  0x00435A20 ---- */
void Com_CleanupSkeletons( void ) {
	com_skelTimeStamp++;
	if ( !com_skelTimeStamp ) {
		com_skelTimeStamp = 1;
	}
	com_skelInvalidated = 0;
	com_skelInvalidated2 = 0;
}

/* ---- Com_SetErrorMessage  0x00435A40 ---- */
void Com_SetErrorMessage( const char *errorMessage ) {
	char *localized;

	Cvar_Get( "com_errorMessage", "", CVAR_ROM );

	if ( !errorMessage || !errorMessage[0] ) {
		Cvar_Set2( "com_errorMessage", "", qtrue );
		return;
	}

	localized = SEH_LocalizeTextMessage( errorMessage, "error message", 1 );
	if ( localized ) {
		Cvar_Set2( "com_errorMessage", localized, qtrue );
		Q_strncpyz( com_errorMessage, localized, sizeof( com_errorMessage ) );
	} else {
		Cvar_Set2( "com_errorMessage", errorMessage, qtrue );
	}
}

/* ---- Com_Error  0x00435AD0 ---- */
void QDECL Com_Error( int code, const char *fmt, ... ) {
	va_list argptr;
	static int lastErrorTimeLocal;
	int currentTime;

	if ( com_errorEntered ) {
		Sys_Error( "recursive error after: %s", com_errorMessage );
	}
	com_errorEntered = qtrue;

	va_start( argptr, fmt );
	vsprintf( com_errorMessage, fmt, argptr );
	va_end( argptr );

	if ( code == ERR_DISCONNECT || code == ERR_NEED_CD || code == ERR_AUTOUPDATE ) {
		char *localized;
		if ( com_errorMessage[0] ) {
			localized = SEH_LocalizeTextMessage( com_errorMessage, "error message", 1 );
			if ( localized ) {
				Q_strncpyz( com_errorMessage, localized, sizeof( com_errorMessage ) );
			}
		}
	} else {
		if ( uivm ) {
			VM_Call( uivm, UI_SET_ACTIVE_MENU, 0 );
		}
		Com_SetErrorMessage( com_errorMessage );
	}

	Hunk_ResetTempMark();
	if ( code != ERR_DISCONNECT ) {
		Scr_Shutdown();
	}
	Com_CleanupSkeletons();
	Com_ResetParseSessions();
	FS_ResetFiles();

	if ( code == ERR_DROP ) {
		Cbuf_Init();
	}

	FS_PureServerSetLoadedPaks( "", "" );

	currentTime = Sys_Milliseconds();
	if ( currentTime - lastErrorTime >= 100 ) {
		errorCount = 0;
	} else if ( ++errorCount > 3 ) {
		code = ERR_FATAL;
	}
	lastErrorTime = currentTime;

	if ( code != ERR_SERVERDISCONNECT && code != ERR_AUTOUPDATE && code != ERR_DROP
		 && code != ERR_DISCONNECT && code != ERR_NEED_CD ) {
		Com_Printf( "********************\nFATAL ERROR (code %i): %s\n"
					"********************\n", code, com_errorMessage );
		CL_Shutdown();
		SV_Shutdown( va( "EXE_SERVERFATALERROR\x15%s", com_errorMessage ) );
		Hunk_ClearToStart();
		Com_Close();
		Sys_Error( "%s", com_errorMessage );
	}

	com_errorCode = code;
	/* Errors raised while drawing bypass SCR_UpdateScreen's normal cleanup.
	 * Retail clears this guard before aborting so subsequent frames can draw. */
	scr_updatingScreen = qfalse;
	longjmp( abortframe, -1 );
}

/* ---- Com_Quit_f  0x00435D80 ---- */
void Com_Quit_f( void ) {
	Com_Printf( "----- Com_Quit_f (%s) -----\n",
				com_quitReason ? com_quitReason : "quit command" );
	com_quitReason = NULL;

	if ( !com_errorEntered ) {
		Hunk_ResetTempMark();
		Com_CleanupSkeletons();
		Sys_DestroySplashWindow();
		CL_Shutdown();
		SV_Shutdown( "EXE_SERVERQUIT" );
		Hunk_ClearToStart();
		Com_Close();
		FS_Shutdown( qtrue );
		FS_ShutdownServerPakNames();
		FS_ShutdownServerReferencedPaks();
	}

	Sys_ShutdownTimer();
	IN_Shutdown();
	Sys_DestroyConsole();
	Sys_DeleteInstanceMarker();

	Cvar_Shutdown();
	Cmd_Shutdown();
	Sys_PlatformExit();

	exit( 0 );
}

/* ---- Com_ParseCommandLine  0x00435E40 ---- */
void Com_ParseCommandLine( char *commandLine ) {
	com_consoleLines[0] = commandLine;
	com_numConsoleLines = 1;

	while ( *commandLine ) {
		if ( *commandLine == '+' || *commandLine == '\n' ) {
			if ( com_numConsoleLines == MAX_CONSOLE_LINES ) {
				return;
			}
			com_consoleLines[com_numConsoleLines] = commandLine + 1;
			com_numConsoleLines++;
			*commandLine = 0;
		}
		commandLine++;
	}
}

/* ---- Com_SafeMode  0x00435E90 ---- */
qboolean Com_SafeMode( void ) {
	int i;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( !Q_stricmp( Cmd_Argv( 0 ), "safe" )
			 || !Q_stricmp( Cmd_Argv( 0 ), "cvar_restart" ) ) {
			com_consoleLines[i][0] = 0;
			return qtrue;
		}
	}
	return qfalse;
}

/* ---- Com_ForceSafeMode  0x00435F30 ---- */
void Com_ForceSafeMode( void ) {
}

/* ---- Com_StartupVariable  0x00435F40 ---- */
void Com_StartupVariable( const char *match ) {
	int i;
	char    *s;
	cvar_t  *cv;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( strcmp( Cmd_Argv( 0 ), "set" ) ) {
			continue;
		}

		s = Cmd_Argv( 1 );
		if ( !match || !strcmp( s, match ) ) {
			Cvar_Set2( s, Cmd_Argv( 2 ), qtrue );
			cv = Cvar_Get( s, "", 0 );
			cv->flags |= CVAR_USER_CREATED;
		}
	}
}

/* ---- Com_AddStartupCommands  0x00436030 ---- */
qboolean Com_AddStartupCommands( void ) {
	int i;
	qboolean added;

	added = qfalse;
	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		if ( !com_consoleLines[i] || !com_consoleLines[i][0] ) {
			continue;
		}
		if ( Q_stricmpn( com_consoleLines[i], "set", 3 ) ) {
			added = qtrue;
		}
		Cbuf_AddText( com_consoleLines[i] );
		Cbuf_AddText( "\n" );
	}
	return added;
}

/* ---- Info_Print  0x00436090 ---- */
void Info_Print( const char *s ) {
	char key[512];
	char value[512];
	char    *o;
	int l;

	if ( *s == '\\' ) {
		s++;
	}
	while ( *s )
	{
		o = key;
		while ( *s && *s != '\\' )
			*o++ = *s++;

		l = o - key;
		if ( l < 20 ) {
			memset( o, ' ', 20 - l );
			key[20] = 0;
		} else {
			*o = 0;
		}
		Com_Printf( "%s", key );

		if ( !*s ) {
			Com_Printf( "MISSING VALUE\n" );
			return;
		}

		o = value;
		s++;
		while ( *s && *s != '\\' )
			*o++ = *s++;
		*o = 0;

		if ( *s ) {
			s++;
		}
		Com_Printf( "%s\n", value );
	}
}

/* ---- Com_InitJournaling  0x004361A0 ---- */
void Com_InitJournaling( void ) {
	Com_StartupVariable( "journal" );
	com_journal = Cvar_Get( "journal", "0", CVAR_INIT );
	if ( !com_journal->integer ) {
		return;
	}

	if ( com_journal->integer == 1 ) {
		Com_Printf( "Journaling events\n" );
		com_journalDataFile = FS_FOpenFileWrite( "journal.dat" );
		com_journalFile = FS_FOpenFileWrite( "journaldata.dat" );
	} else if ( com_journal->integer == 2 ) {
		Com_Printf( "Replaying journaled events\n" );
		FS_FOpenFileRead( "journal.dat", &com_journalDataFile, qtrue );
		FS_FOpenFileRead( "journaldata.dat", &com_journalFile, qtrue );
	}

	if ( !com_journalDataFile || !com_journalFile ) {
		Cvar_Set2( "com_journal", "0", qtrue );
		com_journalDataFile = 0;
		com_journalFile = 0;
		Com_Printf( "Couldn't open journal files\n" );
	}
}

/* ---- Com_GetRealEvent  0x004362A0 ---- VERIFIED */
sysEvent_t Com_GetRealEvent( void ) {
	int r;
	sysEvent_t ev;

	if ( com_journal->integer == 2 ) {
		r = FS_Read( &ev, sizeof( ev ), com_journalDataFile );
		if ( r != sizeof( ev ) ) {
			Com_Error( ERR_FATAL, "EXE_ERR_JOURNAL_FILE_READ" );
		}
		if ( ev.evPtrLength ) {
			ev.evPtr = Z_MallocInternal( ev.evPtrLength );
			r = FS_Read( ev.evPtr, ev.evPtrLength, com_journalDataFile );
			if ( r != ev.evPtrLength ) {
				Com_Error( ERR_FATAL, "EXE_ERR_JOURNAL_FILE_READ" );
			}
		}
	} else {
		ev = Sys_GetEvent();

		if ( com_journal->integer == 1 ) {
			r = FS_Write( &ev, sizeof( ev ), com_journalDataFile );
			if ( r != sizeof( ev ) ) {
				Com_Error( ERR_FATAL, "EXE_ERR_JOURNAL_FILE_WRITE" );
			}
			if ( ev.evPtrLength ) {
				r = FS_Write( ev.evPtr, ev.evPtrLength, com_journalDataFile );
				if ( r != ev.evPtrLength ) {
					Com_Error( ERR_FATAL, "EXE_ERR_JOURNAL_FILE_WRITE" );
				}
			}
		}
	}

	return ev;
}

/* ---- Com_InitPushEvent  0x004363C0 ---- VERIFIED */
void Com_InitPushEvent( void ) {
	memset( com_pushedEvents, 0, sizeof( com_pushedEvents ) );
	com_pushedEventsHead = 0;
	com_pushedEventsTail = 0;
}

/* ---- Com_PushEvent  0x004363E0 ---- VERIFIED */
void Com_PushEvent( sysEvent_t *event ) {
	sysEvent_t  *ev;

	ev = &com_pushedEvents[com_pushedEventsHead & ( MAX_PUSHED_EVENTS - 1 )];

	if ( com_pushedEventsHead - com_pushedEventsTail >= MAX_PUSHED_EVENTS ) {
		if ( !com_eventOverflowWarned ) {
			com_eventOverflowWarned = qtrue;
			Com_Printf( "WARNING: Com_PushEvent overflow\n" );
		}
		if ( ev->evPtr ) {
			free( ev->evPtr );
		}
		com_pushedEventsTail++;
	} else {
		com_eventOverflowWarned = qfalse;
	}

	*ev = *event;
	com_pushedEventsHead++;
}

/* ---- Com_GetEvent  0x00436470 ---- VERIFIED */
sysEvent_t Com_GetEvent( void ) {
	if ( com_pushedEventsHead > com_pushedEventsTail ) {
		com_pushedEventsTail++;
		return com_pushedEvents[( com_pushedEventsTail - 1 ) & ( MAX_PUSHED_EVENTS - 1 )];
	}
	return Com_GetRealEvent();
}

/* ---- Com_RunAndTimeServerPacket  0x004364D0 ---- VERIFIED */
void Com_RunAndTimeServerPacket( netadr_t *evFrom, msg_t *buf ) {
	int t1, t2, msec;

	t1 = 0;
	if ( com_speeds->integer ) {
		t1 = Sys_Milliseconds();
	}

	SV_PacketEvent( *evFrom, buf );

	if ( com_speeds->integer ) {
		t2 = Sys_Milliseconds();
		msec = t2 - t1;
		if ( com_speeds->integer == 3 ) {
			Com_Printf( "SV_PacketEvent time: %i\n", msec );
		}
	}
}

/* ---- Com_EventLoop  0x00436580 ---- VERIFIED */
int Com_EventLoop( void ) {
	sysEvent_t ev;
	netadr_t evFrom;
	byte bufData[MAX_MSGLEN];
	msg_t buf;

	if ( !msgInit ) {
		MSG_initHuffman();
	}

	MSG_Init( &buf, bufData, sizeof( bufData ) );

	while ( 1 ) {
		ev = Com_GetEvent();

		if ( ev.evType == SE_NONE ) {
			while ( NET_GetLoopPacket( NS_CLIENT, &evFrom, &buf ) ) {
				CL_PacketEvent( evFrom, &buf );
			}
			while ( NET_GetLoopPacket( NS_SERVER, &evFrom, &buf ) ) {
				if ( com_sv_running->integer ) {
					Com_RunAndTimeServerPacket( &evFrom, &buf );
				}
			}
			return ev.evTime;
		}

		switch ( ev.evType ) {
		default:
			Com_Error( ERR_FATAL, "\x15" "Com_EventLoop: bad event type %i", ev.evType );
			break;
		case SE_NONE:
			break;
		case SE_KEY:
			CL_KeyEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CHAR:
			CL_CharEvent( ev.evValue );
			break;
		case SE_MOUSE:
			CL_MouseEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_JOYSTICK_AXIS:
			if ( ev.evValue >= MAX_JOYSTICK_AXIS ) {
				Com_Error( ERR_DROP, "\x15" "CL_JoystickEvent: bad axis %i", ev.evValue );
			}
			CL_JoystickEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CONSOLE:
			Com_Printf( "[con] %s\n", (char *)ev.evPtr );
			Cbuf_AddText( (char *)ev.evPtr );
			Cbuf_AddText( "\n" );
			free( ev.evPtr );
			break;
		case SE_PACKET:
			evFrom = *(netadr_t *)ev.evPtr;
			buf.cursize = ev.evPtrLength - sizeof( evFrom );
			if ( (unsigned int)buf.cursize > (unsigned int)buf.maxsize ) {
				Com_Printf( "Com_EventLoop: oversize packet\n" );
				free( ev.evPtr );
				continue;
			}
			Com_Memcpy( buf.data, (byte *)( (netadr_t *)ev.evPtr + 1 ), buf.cursize );
			free( ev.evPtr );
			if ( com_sv_running->integer ) {
				Com_RunAndTimeServerPacket( &evFrom, &buf );
			} else {
				CL_PacketEvent( evFrom, &buf );
			}
			break;
		}
	}
}

/* ---- Com_Milliseconds  0x00436850 ---- */
int Com_Milliseconds( void ) {
	sysEvent_t ev;

	do {
		ev = Com_GetRealEvent();
		if ( ev.evType != SE_NONE ) {
			Com_PushEvent( &ev );
		}
	} while ( ev.evType != SE_NONE );

	return ev.evTime;
}

/* ---- Com_Error_f  0x00436890 ---- */
void Com_Error_f( void ) {
	if ( Cmd_Argc() > 1 ) {
		Com_Error( ERR_DROP, "\x15" "Testing drop error" );
	}
	Com_Error( ERR_FATAL, "\x15" "Testing fatal error" );
}

/* ---- Com_Freeze_f  0x004368C0 ---- */
void Com_Freeze_f( void ) {
	float s;
	int start, now;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "freeze <seconds>\n" );
		return;
	}
	s = atof( Cmd_Argv( 1 ) );

	start = Com_Milliseconds();

	while ( 1 ) {
		now = Com_Milliseconds();
		if ( ( now - start ) * 0.001 > s ) {
			break;
		}
	}
}

/* ---- Com_Crash_f  0x00436920 ---- */
void Com_Crash_f( void ) {
	*(int *)0 = 0x12345678;
}

/* ---- Com_ReadCDKey  0x00436930 ---- */
void Com_ReadCDKey( const char *filename ) {
	fileHandle_t f;
	char buffer[33];
	char fbuffer[MAX_OSPATH];
	char checksum[9];

	sprintf( fbuffer, "%s/%s", filename, "codkey" );

	FS_SV_FOpenFileRead( fbuffer, &f );
	if ( !f ) {
		Q_strncpyz( cl_cdkey, "                ", 17 );
		Q_strncpyz( cl_cdkeychecksum, "    ", 5 );
		return;
	}

	Com_Memset( buffer, 0, sizeof( buffer ) );
	Com_Memset( checksum, 0, sizeof( checksum ) );

	FS_Read( buffer, 16, f );
	FS_Read( checksum, 4, f );
	FS_FCloseFile( f );

	if ( CL_CDKeyValidate( buffer, checksum ) ) {
		Q_strncpyz( cl_cdkey, buffer, 17 );
		Q_strncpyz( cl_cdkeychecksum, checksum, 5 );
	} else {
		Q_strncpyz( cl_cdkey, "                ", 17 );
		Q_strncpyz( cl_cdkeychecksum, "    ", 5 );
	}
}

/* ---- Com_AppendCDKey  0x00436A40 ---- */
void Com_AppendCDKey( const char *filename ) {
	fileHandle_t f;
	char buffer[33];
	char fbuffer[MAX_OSPATH];
	char checksum[9];

	sprintf( fbuffer, "%s/%s", filename, "codkey" );

	FS_SV_FOpenFileRead( fbuffer, &f );
	if ( !f ) {
		Q_strncpyz( &cl_cdkey[16], "                ", 17 );
		Q_strncpyz( &cl_cdkeychecksum[4], "    ", 5 );
		return;
	}

	Com_Memset( buffer, 0, sizeof( buffer ) );
	Com_Memset( checksum, 0, sizeof( checksum ) );

	FS_Read( buffer, 16, f );
	FS_Read( checksum, 4, f );
	FS_FCloseFile( f );

	if ( CL_CDKeyValidate( buffer, checksum ) ) {
		strcat( &cl_cdkey[16], buffer );
		strcat( &cl_cdkeychecksum[4], checksum );
	} else {
		Q_strncpyz( &cl_cdkey[16], "                ", 17 );
		Q_strncpyz( &cl_cdkeychecksum[4], "    ", 5 );
	}
}

/* ---- Com_WriteCDKey  0x00436BC0 ---- */
void Com_WriteCDKey( const char *filename, const char *cdkey, const char *checksum ) {
	fileHandle_t f;
	char fbuffer[MAX_OSPATH];
	char key[17];
	char chk[5];
	char *s;

	sprintf( fbuffer, "%s/%s", filename, "codkey" );

	Q_strncpyz( key, cdkey, 17 );
	Q_strncpyz( chk, checksum, 5 );

	for ( s = key ; *s ; s++ ) {
		*s = toupper( *s );
	}
	for ( s = chk ; *s ; s++ ) {
		*s = toupper( *s );
	}

	if ( !CL_CDKeyValidate( key, chk ) ) {
		return;
	}

	f = FS_SV_FOpenFileWrite( fbuffer );
	if ( !f ) {
		Com_Printf( "Couldn't write %s.\n", filename );
		return;
	}

	FS_Write( key, 16, f );
	FS_Write( chk, 4, f );

	FS_Printf( f, "\r\n// generated by CoD, do not modify\r\n" );
	FS_Printf( f, "// Do not give this file to ANYONE.\r\n" );
	FS_Printf( f, "// Infinity Ward and Activision will NOT ask you to send this file to them.\r\n" );

	FS_FCloseFile( f );
}

/* ---- Com_ConfigureChecksum  0x00436D10 ---- */
int Com_ConfigureChecksum( const char *data, int len ) {
	int checksum;
	int i;

	checksum = 0;
	for ( i = 0 ; i < len ; i++ ) {
		checksum = data[i] + 31337 * checksum;
	}
	return ( checksum & 0x0FFFFFFF ) + 1;
}

/* ---- Com_ConfigureFileChanged  0x00436D40 ---- */
qboolean Com_ConfigureFileChanged( void ) {
	void *buffer;
	int len;
	int checksum;

	len = FS_ReadFile( "configure_mp.csv", &buffer );
	if ( len < 0 ) {
		Com_Error( ERR_FATAL, "EXE_ERR_NOTFOUND" );
	}

	checksum = Com_ConfigureChecksum( (const char *)buffer, len );
	FS_FreeFile( buffer );

	return Sys_ConfigureChecksumChanged( checksum );
}

extern double sys_cpuMHzValue;
extern signed __int32 sys_sysMBValue;
extern int sys_vidMBValue;

/* ---- Com_SetRecommended  0x00436DB0 ---- */
void Com_SetRecommended( int restart ) {
	void        *buffer;
	char        *data;
	int len, checksum;
	char cvarNames[256][128];
	char cvarValues[256][256];
	int numCvars;
	double cpuMhz;
	int sysMb, vidMb;
	int i;
	double bestCpuMhz;
	int bestSysMb, bestVidMb;
	qboolean rowIsNewBest;

	Com_Printf( "========= autoconfigure\n" );

	cpuMhz = Sys_GetCPUSpeed() * 1.02;
	sysMb = Sys_GetSystemRam();
	vidMb = sys_vidMBValue;
	if ( sysMb < 128 ) {
		sysMb = 128;
	} else {
		sysMb += 8;
	}
	if ( vidMb < 32 ) {
		vidMb = 32;
	}

	len = FS_ReadFile( "configure_mp.csv", &buffer );
	if ( len < 0 ) {
		Com_Error( ERR_FATAL, "EXE_ERR_NOT_FOUND" );
	}
	data = buffer;

	Com_BeginParseSession( "configure_mp.csv" );
	parseInfo->csv = qtrue;

	numCvars = 0;

	if ( Q_stricmp( Com_ParseExt( &data, qtrue ), "cpu mhz" ) ) {
		Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: \"cpu mhz\" should be the first column\n" );
	}
	if ( Q_stricmp( Com_ParseExt( &data, qfalse ), "sys mb" ) ) {
		Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: \"sys mb\" should be the second column\n" );
	}
	if ( Q_stricmp( Com_ParseExt( &data, qfalse ), "vid mb" ) ) {
		Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: \"vid mb\" should be the third column\n" );
	}
	while ( 1 ) {
		char *tok = Com_ParseExt( &data, qfalse );
		if ( !tok[0] ) {
			break;
		}
		if ( strlen( tok ) > 127 ) {
			Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: cvar name \"%s\" longer than %i\n", tok, 127 );
		}
		if ( numCvars >= 256 ) {
			Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: more than %i cvars\n", 256 );
		}
		strcpy( cvarNames[numCvars], tok );
		numCvars++;
	}

	bestCpuMhz = -2.0;
	bestSysMb = 0;
	bestVidMb = 0;

	while ( 1 ) {
		char *tok = Com_ParseExt( &data, qfalse );
		double rowCpuMhz;
		int rowSysMb, rowVidMb;

		if ( !data ) {
			break;
		}
		if ( !tok[0] || tok[0] == '#' ) {
			while ( *data && *data != '\n' ) {
				data++;
			}
			if ( *data == '\n' ) {
				data++;
				parseInfo->currentLine++;
			}
			continue;
		}

		rowCpuMhz = atof( tok );
		if ( rowCpuMhz < 0.0 ) {
			Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: cpu mhz %g not allowed to be less than 0\n", rowCpuMhz );
		}

		tok = Com_ParseExt( &data, qfalse );
		rowSysMb = atol( tok );
		if ( rowSysMb < 128 ) {
			Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: sys mb %i not allowed to be less than 128\n", rowSysMb );
		}

		tok = Com_ParseExt( &data, qfalse );
		rowVidMb = atol( tok );
		if ( rowVidMb < 32 ) {
			Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: vid mb %i not allowed to be less than 32\n", rowVidMb );
		}

		rowIsNewBest = qfalse;
		if ( cpuMhz >= rowCpuMhz && sysMb >= rowSysMb && vidMb >= rowVidMb
		     && ( bestCpuMhz < cpuMhz
		       || ( bestCpuMhz == cpuMhz
		         && ( bestVidMb < vidMb
		           || ( bestVidMb == vidMb && bestSysMb < sysMb ) ) ) ) ) {
			rowIsNewBest = qtrue;
			bestCpuMhz = rowCpuMhz;
			bestSysMb = rowSysMb;
			bestVidMb = rowVidMb;
		}

		for ( i = 0 ; i < numCvars ; i++ ) {
			tok = Com_ParseExt( &data, qfalse );
			if ( !data ) {
				Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: unexpected end-of-file" );
			}
			if ( !tok[0] ) {
				Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: missing entry for cvar '%s' in row %lg %i %i\n",
						   cvarNames[i], rowCpuMhz, rowSysMb, rowVidMb );
			}
			if ( strlen( tok ) > 255 ) {
				Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: entry '%s' for cvar '%s' in row %lg %i %i is longer than %i\n",
						   tok, cvarNames[i], rowCpuMhz, rowSysMb, rowVidMb, 255 );
			}
			if ( rowIsNewBest ) {
				strcpy( cvarValues[i], tok );
			}
		}

		tok = Com_ParseExt( &data, qfalse );
		if ( tok[0] ) {
			Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: extra cvar value column(s) in row %lg %i %i\n",
					   bestCpuMhz, bestSysMb, bestVidMb );
		}
	}

	Com_EndParseSession();

	checksum = Com_ConfigureChecksum( (const char *) buffer, len );

	if ( bestCpuMhz < 0.0 || !bestSysMb || !bestVidMb ) {
		Com_Error( ERR_FATAL, "\x15" "configure_mp.csv: \x14" "EXE_ERR_COULDNT_CONFIGURE" "\x15" " %.0f cpu MHz %i sys MB %i vid MB\n",
				   sys_cpuMHzValue, sys_sysMBValue, sys_vidMBValue );
	}

	Com_Printf( "configure_mp.csv: using configuration %.0f cpu MHz %i sys MB %i vid MB\n",
				bestCpuMhz, bestSysMb, bestVidMb );

	Cbuf_AddText( "exec configure_mp.cfg" );
	Cbuf_Execute();

	for ( i = 0 ; i < numCvars ; i++ ) {
		cvar_t *var;
		Cvar_Set2( cvarNames[i], cvarValues[i], qtrue );
		var = Cvar_FindVar( cvarNames[i] );
		if ( var ) {
			var->flags |= CVAR_ARCHIVE;
		}
	}

	FS_FreeFile( buffer );

	Sys_SetConfigCvars( checksum );

	if ( restart ) {
		Cbuf_AddText( "snd_restart\n" );
	}
}

extern int dword_140733C;
extern unsigned short SL_GetStringOfLen( const char *text, unsigned char user,
                                         unsigned int size, int type );

/* ---- Com_Init  0x004375C0 ---- */
void Com_Init( char *commandLine ) {
	char    *s;
	extern void Com_BindHunkGlobals( void );
	extern void FX_StaticInit( void );

	Com_BindHunkGlobals();

	FX_StaticInit();

	Com_Printf( "%s %s build %s %s\n", "COD MP", "1.1", "win-x86", "Oct  8 2003" );

	if ( setjmp( abortframe ) ) {
		Com_ErrorCleanup();
		Sys_Error( "Error during initialization" );
	}

	Com_InitPushEvent();

	cvar_cheats = Cvar_Get( "sv_cheats", "0", CVAR_ROM | CVAR_SYSTEMINFO );
	Cvar_AddCommands();

	Com_ParseCommandLine( commandLine );

	Swap_Init();
	Cbuf_Init();

	Cmd_Init();

	Com_StartupVariable( NULL );
	Com_StartupVariable( "developer" );

	Cmd_AddCommand( "bind", Key_Bind_f );
	Cmd_AddCommand( "unbind", Key_Unbind_f );
	Cmd_AddCommand( "unbindall", Key_UnbindAll_f );
	Cmd_AddCommand( "bindlist", Key_Bindlist_f );

	FS_InitFilesystem();

	Com_InitJournaling();

	Cbuf_AddText( "exec default_mp.cfg\n" );
	Cbuf_AddText( "exec language.cfg\n" );
	Cbuf_AddText( "exec config_mp.cfg\n" );
	Cbuf_AddText( "exec autoexec_mp.cfg\n" );

	if ( Com_SafeMode() ) {
		Cbuf_AddText( "exec default_mp.cfg\n" );
	}

	Cbuf_Execute();

	com_recommendedSet = Cvar_Get( "com_recommendedSet", "0", CVAR_ARCHIVE );
	if ( !com_recommendedSet->integer || Com_ConfigureFileChanged() ) {
		Com_SetRecommended( 0 );
		Cvar_Set2( "com_recommendedSet", "1", qtrue );
	}
	if ( Sys_ShouldReconfigure() ) {
		Com_SetRecommended( 0 );
	}

	Com_StartupVariable( NULL );

	SEH_UpdateLanguageInfo();

	/* Retail ships one Windows binary and defaults this off; `dedicated` is
	 * the cvar that makes it a server.  The literal is inferred -- from
	 * Q3/RTCW, and from there being no second Windows binary to default it the
	 * other way.  The dedicated build (cod_lnxded's shape) pins it, as Q3 did. */
#ifdef DEDICATED
	com_dedicated = Cvar_Get( "dedicated", "1", CVAR_ROM );
#else
	com_dedicated = Cvar_Get( "dedicated", "0", CVAR_LATCH );
#endif
	if ( com_dedicated->integer ) {
		Sys_DestroySplashWindow();
		Sys_ShowConsole( 1, qtrue );
		Sys_DeleteInstanceMarker();
	}

	Com_InitHunkMemory();

	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	com_maxfps = Cvar_Get( "com_maxfps", "85", CVAR_ARCHIVE );
	com_developer = Cvar_Get( "developer", "0", CVAR_TEMP );
	com_developer_script = Cvar_Get( "developer_script", "0", CVAR_TEMP );
	com_logfile = Cvar_Get( "logfile", "0", 0 );
	com_statmon = Cvar_Get( "com_statmon", "0", 0 );
	com_timescale = Cvar_Get( "timescale", "1", CVAR_CHEAT | CVAR_SYSTEMINFO );
	com_fixedtime = Cvar_Get( "fixedtime", "0", CVAR_CHEAT );
	com_viewlog = Cvar_Get( "viewlog", "0", CVAR_CHEAT );
	com_speeds = Cvar_Get( "com_speeds", "0", 0 );

	sv_paused = Cvar_Get( "sv_paused", "0", CVAR_ROM );
	cl_paused = Cvar_Get( "cl_paused", "0", CVAR_ROM );
	com_sv_running = Cvar_Get( "sv_running", "0", CVAR_ROM );
	com_cl_running = Cvar_Get( "cl_running", "0", CVAR_ROM );
	com_introplayed = Cvar_Get( "com_introplayed", "0", CVAR_ARCHIVE );
	com_animCheck = Cvar_Get( "com_animCheck", "0", 0 );

	if ( com_dedicated->integer && !com_viewlog->integer ) {
		Cvar_Set2( "viewlog", "1", qtrue );
	}

	if ( com_developer && com_developer->integer ) {
		Cmd_AddCommand( "error", Com_Error_f );
		Cmd_AddCommand( "crash", Com_Crash_f );
		Cmd_AddCommand( "freeze", Com_Freeze_f );
	}
	Cmd_AddCommand( "quit", Com_Quit_f );
	Cmd_AddCommand( "writeconfig", Com_WriteConfig_f );
	Cmd_AddCommand( "writedefaults", Com_WriteDefaults_f );

	s = va( "%s %s build %s %s", "COD MP", "1.1", getBuildNumber(), "win-x86" );
	com_version = Cvar_Get( "version", s, CVAR_ROM | CVAR_SERVERINFO );
	com_shortversion = Cvar_Get( "shortversion", "1.1", CVAR_ROM | CVAR_SERVERINFO );

	Sys_Init();

	Netchan_Init( Com_Milliseconds() & 0xffff );

	Scr_Init();
	VM_Init();

	{
		extern int XAnimInit( void );          /* XAnimInit, 0x00482F00 */
		char emptyString[20];

		XAnimInit();
		Com_Memset( emptyString, 0, sizeof( emptyString ) );
		*(unsigned short *) &dword_140733C =
			(unsigned short) SL_GetStringOfLen( emptyString, 0, 17, 12 );
	}

	SV_Init();
	NET_Init();

	com_dedicated->modified = qfalse;
	if ( !com_dedicated->integer ) {
		CL_Init();
		Sys_ShowConsole( com_viewlog->integer, qfalse );
	}

	com_frameTime = Com_Milliseconds();

	Com_AddStartupCommands();

	Cvar_Set2( "r_uiFullScreen", "1", qtrue );

	CL_StartHunkUsers();

	if ( !com_dedicated->integer ) {
		Sys_ShowConsole( com_viewlog->integer, qfalse );
	}

	if ( !com_dedicated->integer && !com_introplayed->integer ) {
		Cvar_Set2( "com_introplayed", "1", qtrue );
		Cvar_Set2( "nextmap", "cinematic iw_logo.roq", qtrue );
	}

	Cvar_Set2( "com_statmon", "0", qtrue );

	com_fullyInitialized = qtrue;
	Com_Printf( "--- Common Initialization Complete ---\n" );
}

/* ---- Com_WriteConfigToFile  0x00437C20 ---- */
void Com_WriteConfigToFile( const char *filename ) {
	fileHandle_t f;

	f = FS_FOpenFileWrite( filename );
	if ( !f ) {
		Com_Printf( "Couldn't write %s.\n", filename );
		return;
	}

	FS_Printf( f, "// generated by Call of Duty, do not modify\n" );
	Key_WriteBindings( f );
	Cvar_WriteVariables( f );
	FS_FCloseFile( f );
}

/* ---- Com_WriteDefaultsToFile  0x00437C70 ---- */
void Com_WriteDefaultsToFile( const char *filename ) {
	fileHandle_t f;

	f = FS_FOpenFileWrite( filename );
	if ( !f ) {
		Com_Printf( "Couldn't write %s.\n", filename );
		return;
	}

	FS_Printf( f, "// generated by Call of Duty, do not modify\n" );
	Cvar_WriteDefaults( f );
	FS_FCloseFile( f );
}

/* ---- Com_WriteConfiguration  0x00437CB0 ---- */
void Com_WriteConfiguration( void ) {
	if ( !com_fullyInitialized ) {
		return;
	}

	if ( !( cvar_modifiedFlags & CVAR_ARCHIVE ) ) {
		return;
	}
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	Com_WriteConfigToFile( "config_mp.cfg" );
}

/* ---- Com_WriteConfig_f  0x00437CE0 ---- */
void Com_WriteConfig_f( void ) {
	char filename[MAX_QPATH];

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: writeconfig <filename>\n" );
		return;
	}

	Q_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	Com_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	Com_Printf( "Writing %s.\n", filename );
	Com_WriteConfigToFile( filename );
}

/* ---- Com_WriteDefaults_f  0x00437DC0 ---- */
void Com_WriteDefaults_f( void ) {
	char filename[MAX_QPATH];

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: writedefaults <filename>\n" );
		return;
	}

	Q_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	Com_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	Com_Printf( "Writing %s.\n", filename );
	Com_WriteDefaultsToFile( filename );
}

/* ---- Com_ModifyMsec  0x00437E90 ---- */
int Com_ModifyMsec( int msec ) {
	int clampTime;

	if ( com_fixedtime->integer ) {
		msec = com_fixedtime->integer;
	} else if ( com_timescale->value ) {
		msec *= com_timescale->value;
	}

	if ( msec < 1 && com_timescale->value ) {
		msec = 1;
	}

	if ( com_dedicated->integer ) {
		if ( msec > 500 && msec < 500000 ) {
			Com_Printf( "Hitch warning: %i msec frame time\n", msec );
		}
		clampTime = 5000;
	} else if ( !com_sv_running->integer ) {
		clampTime = 5000;
	} else {
		clampTime = 200;
	}

	if ( msec > clampTime ) {
		msec = clampTime;
	}

	return msec;
}

/* ---- Com_Frame  0x00437F40 ---- VERIFIED */
void Com_Frame( void ) {
	int msec, minMsec;
	int timeBeforeFirstEvents, timeBeforeServer, timeBeforeEvents, timeBeforeClient, timeAfter;

	if ( setjmp( abortframe ) ) {
		Com_ErrorCleanup();
		return;
	}

	timeBeforeFirstEvents = 0;
	timeBeforeServer = 0;
	timeBeforeEvents = 0;
	timeBeforeClient = 0;
	timeAfter = 0;

	Com_WriteConfiguration();

	if ( com_statmon->integer && fs_loadingMode ) {
		StatMon_Warning( 1, 3000, "gfx/2d/warning@file.jpg" );
		fs_loadingMode = 0;
	}

	if ( com_viewlog->modified ) {
		if ( !com_dedicated->value ) {
			Sys_ShowConsole( com_viewlog->integer, qfalse );
		}
		com_viewlog->modified = qfalse;
	}

	com_animCheckCached = com_animCheck->integer;

	if ( com_speeds->integer ) {
		timeBeforeFirstEvents = Sys_Milliseconds();
	}

	if ( !com_dedicated->integer && com_maxfps->integer > 0 ) {
		minMsec = 1000 / com_maxfps->integer;
	} else {
		minMsec = 1;
	}
	do {
		com_frameTime = Com_EventLoop();
		if ( com_lastFrameTime > com_frameTime ) {
			com_lastFrameTime = com_frameTime;
		}
		msec = com_frameTime - com_lastFrameTime;
	} while ( msec < minMsec );

	Cbuf_Execute();

	com_lastFrameTime = com_frameTime;

	msec = Com_ModifyMsec( msec );

	if ( com_speeds->integer ) {
		timeBeforeServer = Sys_Milliseconds();
	}

	SV_Frame( msec );

	if ( com_dedicated->modified ) {
		int wasDedicated = com_dedicated->integer;

		Cvar_Get( "dedicated", "0", 0 );
		com_dedicated->modified = qfalse;
		if ( com_dedicated->integer ) {
			CL_Shutdown();
			Sys_ShowConsole( 1, qtrue );
			Sys_DeleteInstanceMarker();
		} else if ( Sys_CheckCrashOrRerun() ) {
			CL_Init();
			CL_StartHunkUsers();
			Sys_ShowConsole( com_viewlog->integer, qfalse );
		} else {
			Com_Printf( "cannot become non-dedicated, since a non-dedicated game is already running\n" );
			Cvar_Set2( "dedicated", va( "%i", wasDedicated ), qtrue );
			Cvar_Get( "dedicated", "1", 0 );
			com_dedicated->modified = qfalse;
		}
	}

	if ( !com_dedicated->integer ) {
		if ( com_speeds->integer ) {
			timeBeforeEvents = Sys_Milliseconds();
		}
		Com_EventLoop();
		Cbuf_Execute();

		if ( com_speeds->integer ) {
			timeBeforeClient = Sys_Milliseconds();
		}
		CL_Frame( msec );

		if ( com_statmon->integer || com_speeds->integer ) {
			int now = Sys_Milliseconds();
			if ( com_statmon->integer && com_lastStatmonTime
				 && ( now - com_lastStatmonTime ) > 33 ) {
				StatMon_Warning( 0, 3000, "gfx/2d/warning@fps.jpg" );
			}
			com_lastStatmonTime = now;
			timeAfter = now;
		}
	}

	if ( com_speeds->integer ) {
		int all, sv, ev, cl;

		all = timeAfter - timeBeforeServer;
		sv = timeBeforeEvents - time_frontend - timeBeforeServer;
		ev = timeBeforeServer - timeBeforeFirstEvents + timeBeforeClient - timeBeforeEvents;
		cl = timeAfter - time_game - time_backend - timeBeforeClient;

		Com_Printf( "frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
					com_frameNumber, all, sv, ev, cl, time_frontend, time_backend, time_game );
	}

	com_frameNumber++;
}

/* ---- Com_Close  0x00438330 ---- */
void Com_Close( void ) {
	Scr_Shutdown();

	if ( logfile ) {
		FS_FCloseFile( logfile );
		logfile = 0;
	}

	if ( com_journalDataFile ) {
		FS_FCloseFile( com_journalDataFile );
		com_journalDataFile = 0;
	}
}

/* ---- Field_Clear  0x00438390 ---- */
void Field_Clear( field_t *edit ) {
	memset( edit->buffer, 0, sizeof( edit->buffer ) );
	edit->cursor = 0;
	edit->scroll = 0;
	edit->widthInChars = sizeof( edit->buffer );
}

/* ---- FindMatches  0x004383B0 ---- */
static void FindMatches( const char *s ) {
	int i;

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		Q_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	for ( i = 0 ; s[i] ; i++ ) {
		if ( tolower( shortestMatch[i] ) != tolower( s[i] ) ) {
			shortestMatch[i] = 0;
			break;
		}
	}
	shortestMatch[i] = 0;
}

/* ---- PrintMatches  0x00438440 ---- */
static void PrintMatches( const char *s ) {
	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
	}
}

/* ---- keyConcatArg  0x00438480 ---- */
static void keyConcatArg( char *dest, int destsize, const char *src ) {
	int i;

	for ( i = 1 ; i < Cmd_Argc() ; i++ ) {
		char *arg;
		qboolean quoted;

		Q_strcat( dest, destsize, " " );

		arg = Cmd_Argv( i );
		quoted = ( strchr( arg, ' ' ) != NULL );
		if ( quoted ) {
			Q_strcat( dest, destsize, "\"" );
		}
		Q_strcat( dest, destsize, arg );
		if ( quoted ) {
			Q_strcat( dest, destsize, "\"" );
		}
	}
}

/* ---- ConcatRemaining  0x00438660 ---- */
static void ConcatRemaining( char *dest, int destsize, const char *src, const char *start ) {
	char *str;

	str = strstr( src, start );
	if ( !str ) {
		keyConcatArg( dest, destsize, src );
		return;
	}

	str += strlen( start );
	Q_strcat( dest, destsize, str );
}

/* ---- Field_CompleteCommand  0x004386B0 ---- */
void Field_CompleteCommand( field_t *field ) {
	field_t temp;
	char    *cmd;

	completionField = field;

	Cmd_TokenizeString( completionField->buffer );

	cmd = Cmd_Argv( 0 );
	if ( cmd[0] == '\\' || cmd[0] == '/' ) {
		cmd++;
	}
	completionString = cmd;

	if ( !strlen( completionString ) ) {
		return;
	}

	matchCount = 0;
	shortestMatch[0] = 0;

	Cmd_CommandCompletion( FindMatches );
	Cvar_CommandCompletion( FindMatches );

	if ( matchCount == 0 ) {
		return;
	}

	Com_Memcpy( &temp, completionField, sizeof( field_t ) );

	if ( matchCount == 1 ) {
		Com_sprintf( completionField->buffer, sizeof( completionField->buffer ), "\\%s", shortestMatch );
		if ( Cmd_Argc() == 1 ) {
			Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		} else {
			ConcatRemaining( completionField->buffer, sizeof( completionField->buffer ),
							 temp.buffer, completionString );
		}
		completionField->cursor = strlen( completionField->buffer );
		return;
	}

	Com_sprintf( completionField->buffer, sizeof( completionField->buffer ), "\\%s", shortestMatch );
	completionField->cursor = strlen( completionField->buffer );
	ConcatRemaining( completionField->buffer, sizeof( completionField->buffer ),
					 temp.buffer, completionString );

	Com_Printf( "]%s\n", completionField->buffer );

	Cmd_CommandCompletion( PrintMatches );
	Cvar_CommandCompletion( PrintMatches );
}

#define DOBJ_SIZE 88

unsigned short com_clientDObjHandles[MAX_CLIENT_DOBJS];
unsigned short com_serverDObjHandles[MAX_DOBJS];
byte com_dobjAllocBits[MAX_DOBJS / 4];
byte com_dobjPool[MAX_DOBJS * DOBJ_SIZE];
int com_dobjLastIndex;

extern int com_dobjInited;
extern int com_dobjPoolInited;

extern int scrVarPub_developer;
extern int scrVarPub_developerScript;
extern int dword_140733C;

extern int Scr_FreeGameStringRef_m( void );
extern int XAnimShutdown( void );
extern int XAnimInit( void );
extern unsigned short SL_GetStringOfLen( const char *text, unsigned char user,
										 unsigned int size, int type );

/* ---- Com_GetClientDObj  0x00438890 ---- */
void *Com_GetClientDObj( int handle ) {
	unsigned short index;

	index = com_clientDObjHandles[handle];
	if ( !index ) {
		return NULL;
	}
	return &com_dobjPool[index * DOBJ_SIZE];
}

/* ---- Com_GetServerDObj  0x004388B0 ---- */
void *Com_GetServerDObj( int handle ) {
	unsigned short index;

	index = com_serverDObjHandles[handle];
	if ( !index ) {
		return NULL;
	}
	return &com_dobjPool[index * DOBJ_SIZE];
}

/* ---- Com_GetFreeDObjIndex  0x004388D0 ---- */
int Com_GetFreeDObjIndex( void ) {
	int index;

	index = com_dobjLastIndex + 1;

	if ( index < MAX_DOBJS ) {
		while ( ( com_dobjAllocBits[index >> 2] & ( 3 << ( 2 * ( index & 3 ) ) ) ) != 0 ) {
			index++;
			if ( index >= MAX_DOBJS ) {
				goto wrap;
			}
		}
		com_dobjLastIndex = index;
		return index;
	}

wrap:
	index = 1;
	if ( com_dobjLastIndex < 1 ) {
		Com_Error( ERR_DROP, "\x15" "No free DObjs" );
	}
	while ( ( com_dobjAllocBits[index >> 2] & ( 3 << ( 2 * ( index & 3 ) ) ) ) != 0 ) {
		index++;
		if ( index > com_dobjLastIndex ) {
			Com_Error( ERR_DROP, "\x15" "No free DObjs" );
		}
	}
	com_dobjLastIndex = index;
	return index;
}

/* ---- Com_ShutdownDObj  0x00438BC0 ---- */
void Com_ShutdownDObj( void ) {
	if ( com_dobjInited ) {
		com_dobjInited = 0;
	}
}

/* ---- Com_InitDObj  0x00438B70 ---- */
void Com_InitDObj( void ) {
	Com_Memset( com_dobjAllocBits, 0, sizeof( com_dobjAllocBits ) );
	Com_Memset( com_clientDObjHandles, 0, sizeof( com_clientDObjHandles ) );
	Com_Memset( com_serverDObjHandles, 0, sizeof( com_serverDObjHandles ) );
	com_dobjPoolInited = 1;
	com_dobjLastIndex = 1;
	com_dobjInited = 1;
}

/* ---- Com_Restart  0x00438BE0 ---- VERIFIED */
void Com_Restart( void ) {
	char emptyString[20];

	Com_ShutdownDObj();

	Scr_FreeGameStringRef_m();
	XAnimShutdown();
	Scr_Shutdown();

	scrVarPub_developer = com_developer ? com_developer->integer : 0;
	scrVarPub_developerScript = com_developer_script ? com_developer_script->integer : 0;
	Scr_Init();

	XAnimInit();

	Com_Memset( emptyString, 0, sizeof( emptyString ) );
	*(unsigned short *) &dword_140733C =
		(unsigned short) SL_GetStringOfLen( emptyString, 0, 17, 12 );

	Com_InitDObj();
}


/* ==========================================================================
 * Merged from common_raw.c (retail linked it as a separate translation unit).
 * ========================================================================== */

/* globals cod1_globals.h declared for the merged unit; kept local here so
   that header (int-typed com_version etc.) stays out of this TU. */
extern int com_weaponInfoBlockPtr;      /* 0x01407414 */
extern int com_weaponInfoBlockOwner;    /* 0x01407418 */
extern unsigned char empty_string[4];   /* 0x00559228 */

void *Hunk_AllocAlignInternal( int size, int align );
void *Hunk_AllocLowAlignInternal( int size, int align );

extern char DObjCreate( short scrNotifyId, void *dobj, void *models,
						unsigned short modelCount, void *tree );
extern int  DObjFree( unsigned int releaseTree, void *dobj );

unsigned short  MT_AllocIndex( int size, int type );
void            MT_FreeIndex( unsigned short blockIndex, unsigned int size );
extern unsigned char scrMemTree_blocks[];

char *Com_ParseOnLine( char **data_p );

#define MT_TYPE_ANIMTREE    5

#define DOBJ_STRIDE 88

#define DOBJ_CLIENT_HANDLE_OK( h ) \
	( (unsigned int)( h ) < (unsigned int)MAX_CLIENT_DOBJS )
#define DOBJ_SERVER_HANDLE_OK( h ) \
	( (unsigned int)( h ) < (unsigned int)MAX_DOBJS )

/* ---- Com_ClientDObjCreate  0x00438950 ---- VERIFIED */
void Com_ClientDObjCreate( void *models, unsigned short modelCount, void *tree,
						   int handle, unsigned short scrNotifyId ) {
	int index;

	if ( !DOBJ_CLIENT_HANDLE_OK( handle ) ) {
		Com_Printf( "Com_ClientDObjCreate: handle %i out of range\n", handle );
		return;
	}
	index = Com_GetFreeDObjIndex();

	com_dobjAllocBits[index >> 2] |= (byte)( 1 << ( 2 * ( index & 3 ) ) );
	com_clientDObjHandles[handle] = (unsigned short)index;

	DObjCreate( (short)scrNotifyId, &com_dobjPool[index * DOBJ_STRIDE],
				models, modelCount, tree );
}

/* ---- Com_ServerDObjCreate  0x004389B0 ---- VERIFIED */
void Com_ServerDObjCreate( void *models, unsigned short modelCount, void *tree,
						   int handle, unsigned short scrNotifyId ) {
	int index;

	if ( !DOBJ_SERVER_HANDLE_OK( handle ) ) {
		Com_Printf( "Com_ServerDObjCreate: handle %i out of range\n", handle );
		return;
	}
	index = Com_GetFreeDObjIndex();

	com_dobjAllocBits[index >> 2] |= (byte)( 2 << ( 2 * ( index & 3 ) ) );
	com_serverDObjHandles[handle] = (unsigned short)index;

	DObjCreate( (short)scrNotifyId, &com_dobjPool[index * DOBJ_STRIDE],
				models, modelCount, tree );
}

/* ---- Com_SafeClientDObjFree  0x00438A10 ---- VERIFIED */
void Com_SafeClientDObjFree( int handle, unsigned int releaseTree ) {
	int  index;
	int  shift;
	byte bits;

	if ( !DOBJ_CLIENT_HANDLE_OK( handle ) ) {
		Com_Printf( "Com_SafeClientDObjFree: handle %i out of range\n", handle );
		return;
	}
	index = (short)com_clientDObjHandles[handle];
	if ( !index ) {
		return;
	}
	com_clientDObjHandles[handle] = 0;

	shift = 2 * ( index & 3 );
	bits  = com_dobjAllocBits[index >> 2] & (byte)~( 1 << shift );
	com_dobjAllocBits[index >> 2] = bits;

	if ( ( bits & (byte)( 3 << shift ) ) == 0 ) {
		DObjFree( releaseTree, &com_dobjPool[index * DOBJ_STRIDE] );
	}
}

/* ---- Com_SafeServerDObjFree  0x00438A70 ---- VERIFIED */
void Com_SafeServerDObjFree( int handle, unsigned int releaseTree ) {
	int  index;
	int  shift;
	byte bits;

	if ( !DOBJ_SERVER_HANDLE_OK( handle ) ) {
		Com_Printf( "Com_SafeServerDObjFree: handle %i out of range\n", handle );
		return;
	}
	index = (short)com_serverDObjHandles[handle];
	if ( !index ) {
		return;
	}
	com_serverDObjHandles[handle] = 0;

	shift = 2 * ( index & 3 );
	bits  = com_dobjAllocBits[index >> 2] & (byte)~( 2 << shift );
	com_dobjAllocBits[index >> 2] = bits;

	if ( ( bits & (byte)( 3 << shift ) ) == 0 ) {
		DObjFree( releaseTree, &com_dobjPool[index * DOBJ_STRIDE] );
	}
}

/* ---- Com_SyncClientDObjsFromServer_m  0x00438AD0 ---- VERIFIED */
void Com_SyncClientDObjsFromServer_m( void ) {
	int  i;
	int  index;
	int  shift;
	byte bits;

	for ( i = 0; i < MAX_DOBJS; i++ ) {
		index = (short)com_clientDObjHandles[i];
		if ( index ) {
			shift = 2 * ( index & 3 );
			bits  = com_dobjAllocBits[index >> 2] & (byte)~( 1 << shift );
			com_dobjAllocBits[index >> 2] = bits;
			com_clientDObjHandles[i] = 0;

			if ( ( bits & (byte)( 3 << shift ) ) == 0 ) {
				DObjFree( 0, &com_dobjPool[index * DOBJ_STRIDE] );
			}
		}

		index = (short)com_serverDObjHandles[i];
		com_clientDObjHandles[i] = (unsigned short)index;
		if ( index ) {
			com_dobjAllocBits[index >> 2] |=
				(byte)( 1 << ( 2 * ( index & 3 ) ) );
		}
	}
}

/* ---- Hunk_AllocXAnimCreateTree  0x00438CD0 ---- VERIFIED */
void *__cdecl Hunk_AllocXAnimCreateTree(unsigned int size)
{
  return Hunk_AllocLowAlignInternal(size, 32);
}

/* ---- Hunk_AllocXModelPrecache  0x00438CE0 ---- VERIFIED */
void *__cdecl Hunk_AllocXModelPrecache(unsigned int size)
{
  return Hunk_AllocAlignInternal(size, 32);
}

/* ---- Hunk_AllocXModelPrecacheMesh  0x00438CF0 ---- VERIFIED */
void *__cdecl Hunk_AllocXModelPrecacheMesh(unsigned int size)
{
  return Hunk_AllocAlignInternal(size, 32);
}

/* ---- Com_XAnimCreateTree  0x00438D00 ---- VERIFIED */
void *__cdecl Com_XAnimCreateTree(void *anims, void *tag)
{
  int v2 = (int)anims;
  unsigned int v3;
  int *v4;

  v3 = 2 * *(_DWORD *)(v2 + 4) + 8 + 2 * (3 * *(_DWORD *)(v2 + 4) + 1);
  v4 = (int *)Hunk_AllocLowAlignInternal(v3, 32);
  Com_Memset(v4, 0, v3);
  *v4 = v2;
  return v4;
}

/* ---- MT_AllocAnimTree  0x00438D40 ---- VERIFIED */
void *__cdecl MT_AllocAnimTree(int size)
{
  return (void *)&scrMemTree_blocks[8 * (unsigned __int16)MT_AllocIndex(size, MT_TYPE_ANIMTREE)];
}

/* ---- Com_XAnimCreateSmallTree  0x00438D60 ---- VERIFIED */
void *__cdecl Com_XAnimCreateSmallTree(void *anims, void *tag)
{
  int v2 = (int)anims;
  int v3;
  int *v4;

  v3 = 2 * *(_DWORD *)(v2 + 4) + 8 + 2 * (3 * *(_DWORD *)(v2 + 4) + 1);
  v4 = (int *)&scrMemTree_blocks[8 * (unsigned __int16)MT_AllocIndex(v3, MT_TYPE_ANIMTREE)];
  Com_Memset(v4, 0, v3);
  *v4 = v2;
  return v4;
}

/* ---- Com_XAnimFreeSmallTree  0x00438DA0 ---- VERIFIED */
void __cdecl Com_XAnimFreeSmallTree(void *tree)
{
  int v1 = (int)tree;

  MT_FreeIndex(
    (unsigned short)( ( (unsigned char *)tree - scrMemTree_blocks ) >> 3 ),
    2 * *(_DWORD *)(*(_DWORD *)v1 + 4) + 8 + 2 * (3 * *(_DWORD *)(*(_DWORD *)v1 + 4) + 1));
}

/* ---- Com_GetWeaponInfoMemory  0x00438DC0 ---- VERIFIED */
void *__cdecl Com_GetWeaponInfoMemory( int size, int *pPrevOwner, int owner )
{
  void *result;

  if ( size <= 0 )
    return 0;

  result = (void *)com_weaponInfoBlockPtr;
  if ( com_weaponInfoBlockPtr )
  {
    *pPrevOwner = com_weaponInfoBlockOwner;
    if ( !com_weaponInfoBlockOwner )
      com_weaponInfoBlockOwner = owner;
  }
  else
  {
    result = (void *)Hunk_AllocLowAlignInternal(size, 32);
    com_weaponInfoBlockPtr = (int)result;
    *pPrevOwner = 0;
    com_weaponInfoBlockOwner = owner;
  }
  return result;
}

/* ---- Com_FreeWeaponInfoMemory  0x00438E10 ---- VERIFIED */
void __cdecl Com_FreeWeaponInfoMemory( int owner, int iSource )
{
  if ( owner == com_weaponInfoBlockOwner )
  {
    if ( !iSource )
      com_weaponInfoBlockPtr = 0;
    com_weaponInfoBlockOwner = 0;
  }
}

/* ---- Com_SaveCvarsToBuffer  0x00438E30 ---- VERIFIED */
qboolean __cdecl Com_SaveCvarsToBuffer(const char **cvarNames, int cvarCount, char *buffer, int bufferSize)
{
  int v3;
  int v4;  /* SIGNED in retail (`sub ebx,eax`), so keep it signed */
  cvar_t *Var;
  char *string;
  int v8;

  v3 = 0;
  if ( cvarCount > 0 )
  {
    v4 = bufferSize;
    do
    {
      Var = Cvar_FindVar(cvarNames[v3]);
      if ( Var )
        string = Var->string;
      else
        string = (char *)&empty_string;
      v8 = _snprintf(buffer, (size_t)v4, "%s \"%s\"\n", cvarNames[v3], string);
      if ( v8 < 0 )
        return qfalse;      /* 0x00438E9B: buffer full, `xor eax,eax` */
      buffer += v8;
      v4 -= v8;
      ++v3;
    }
    while ( v3 < cvarCount );
  }
  return qtrue;             /* 0x00438E94: `mov eax,1` */
}

/* ---- Com_LoadCvarsFromBuffer  0x00438EB0 ---- VERIFIED */
qboolean __cdecl Com_LoadCvarsFromBuffer( const char **cvarNames, int cvarCount,
										  char *text, const char *filename )
{
	/* Retail's own 0x10004-byte stack array; only cvarCount bytes are used. */
	byte seen[65540];
	char *token;
	char *p;
	int found;
	int i;
	int c;

	Com_Memset( seen, 0, cvarCount );
	found = 0;

	Com_BeginParseSession( filename );

	for ( ;; ) {
		token = Com_Parse( &text );          /* inlined at 0x00438F06 */
		if ( !*token ) {
			break;
		}

		for ( i = 0; i < cvarCount; i++ ) {
			if ( !_stricmp( token, cvarNames[i] ) ) {
				break;
			}
		}

		if ( i == cvarCount ) {
			Com_Printf( "^3WARNING: unknown cvar '%s' in file '%s'\n",
						token, filename );
		} else {
			/* inlined at 0x00438FB9; qfalse = stay on this line */
			Cvar_Set2( cvarNames[i], Com_ParseOnLine( &text ), qtrue );
			if ( !seen[i] ) {
				seen[i] = 1;
				found++;
			}
		}

		p = text;
		if ( !p ) {
			continue;
		}
		c = *p;
		if ( c ) {
			for ( ;; ) {
				p++;
				if ( c == '\n' ) {
					parseInfo->currentLine++;
					break;
				}
				c = *p;
				if ( !c ) {
					break;
				}
			}
		}
		text = p;
	}

	Com_EndParseSession();                   /* inlined at 0x0043903B */

	if ( found != cvarCount ) {
		Com_Printf(
			"^1ERROR: the following cvars were not specified in file '%s'\n",
			filename );
		for ( i = 0; i < cvarCount; i++ ) {
			if ( !seen[i] ) {
				Com_Printf( "^1  %s\n", cvarNames[i] );
			}
		}
		return qfalse;                       /* 0x004390D8 */
	}
	return qtrue;                            /* 0x00439078 */
}
