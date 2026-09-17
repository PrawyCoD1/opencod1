/*
 * @fidelity: likely
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"

extern int      Sys_GetPacket( msg_t *msg, netadr_t *from );
extern char     *Sys_ConsoleInput( void );
extern void     Sys_In_Restart_f( void );
extern void     Sys_Net_Restart_f( void );
extern int      Sys_GetProcessorId( void );
extern int __stdcall MainWndProc( void *hWnd, unsigned Msg,
                                  unsigned wParam, long lParam );
extern int      FS_ExtractFromPakFile_m( char *ospath, int game, char *qpath );
extern void     FS_BuildOSPath_Internal( const char *base, const char *qpath,
                                         char *out, int allowOverflow );
/* The retail join, universal/com_files.c 0x00428EE0: three components, and an empty `game` defaults to fs_gamedir. */
extern void     FS_BuildOSPath3( const char *base, const char *game,
                                 const char *qpath, char *out,
                                 int allowOverflow );

extern int      g_wv_hInstance;

int             g_wv_sysMsgTime;

/* ---- Sys_Milliseconds  0x004659D0 ----  VERIFIED */
int Sys_Milliseconds( void ) {
	if ( !sys_timeBaseInit ) {
		sys_timeBase = (int) timeGetTime();
		sys_timeBaseInit = 1;
	}
	return (int) timeGetTime() - sys_timeBase;
}

#define MAX_QUED_EVENTS     256
#define MASK_QUED_EVENTS    ( MAX_QUED_EVENTS - 1 )

/* Retail's ring is at 0x016BDDA0, 256 * 24 bytes, sitting exactly between eventHead (0x016BDD9C) and eventTail (0x016BF5A4). */
static sysEvent_t   eventQue[MAX_QUED_EVENTS];

/* Retail receives packets into a 0x4000 buffer at 0x016BF5C0; the storage is owned here. */
static byte         sys_packetReceived[MAX_MSGLEN];

/* ---- Sys_QueEvent  0x00463580 ----  VERIFIED */
void Sys_QueEvent( int time, int type, int value, int value2,
                   int ptrLength, void *ptr ) {
	sysEvent_t  *ev;

	ev = &eventQue[eventHead & MASK_QUED_EVENTS];

	if ( eventHead - eventTail >= MAX_QUED_EVENTS ) {
		Com_Printf( "Sys_QueEvent: overflow\n" );
		if ( ev->evPtr ) {
			free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	if ( time == 0 ) {
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = (sysEventType_t) type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

/* ---- Sys_GetEvent  0x004636A0 ----  VERIFIED */
sysEvent_t Sys_GetEvent( void ) {
	MSG         msg;
	sysEvent_t  ev;
	char        *s;
	msg_t       netmsg;
	netadr_t    adr;

	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[( eventTail - 1 ) & MASK_QUED_EVENTS];
	}

	while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
		if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
			Com_Quit_f();
		}
		g_wv_sysMsgTime = msg.time;
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	s = Sys_ConsoleInput();
	if ( s ) {
		char    *b;
		int     len;

		len = strlen( s ) + 1;
		b = Z_MallocInternal( len );
		Q_strncpyz( b, s, len );
		/* len, not len - 1: win_syscon_console.c's Sys_ConsoleInput already strips the newline. */
		Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, b );
	}

	memset( &adr, 0, sizeof( adr ) );
	MSG_Init( &netmsg, sys_packetReceived, sizeof( sys_packetReceived ) );
	if ( Sys_GetPacket( &netmsg, &adr ) ) {
		netadr_t    *buf;
		int         len;

		len = netmsg.cursize - netmsg.readcount;
		buf = Z_MallocInternal( sizeof( netadr_t ) + len );
		*buf = adr;
		memcpy( buf + 1, &netmsg.data[netmsg.readcount], len );
		Sys_QueEvent( 0, SE_PACKET, 0, 0, sizeof( netadr_t ) + len, buf );
	}

	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[( eventTail - 1 ) & MASK_QUED_EVENTS];
	}

	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = (int) timeGetTime();

	return ev;
}

/* ---- Sys_ReadTSC  no-address ----  VERIFIED */
static unsigned __int64 Sys_ReadTSC( void ) {
	unsigned int lo, hi;

	__asm {
		_emit 0x0F
		_emit 0x31
		mov     lo, eax
		mov     hi, edx
	}

	return ( (unsigned __int64) hi << 32 ) | lo;
}

/* ---- Sys_GetCPUSpeed  0x00460B80 ---- */
float Sys_GetCPUSpeed( void ) {
	LARGE_INTEGER   freq, start, end;
	unsigned __int64 tscStart, tscEnd;
	double          elapsed;

	if ( sys_cpuMHzValue != 0.0 ) {
		return (float) sys_cpuMHzValue;
	}

	Sleep( 0 );

	QueryPerformanceFrequency( &freq );
	QueryPerformanceCounter( &start );
	tscStart = Sys_ReadTSC();
	QueryPerformanceCounter( &start );

	Sleep( 250 );

	tscEnd = Sys_ReadTSC();
	QueryPerformanceCounter( &end );

	elapsed = (double) ( end.QuadPart - start.QuadPart );
	if ( elapsed <= 0.0 || freq.QuadPart == 0 ) {
		return 0.0f;
	}

	sys_cpuMHzValue = (double) (__int64) ( tscEnd - tscStart )
	             * (double) freq.QuadPart * 0.000001 / elapsed;

	return (float) sys_cpuMHzValue;
}

/* Sys_GetSystemRam 0x00460C50 */

typedef struct {
	DWORD               dwLength;
	DWORD               dwMemoryLoad;
	unsigned __int64    ullTotalPhys;
	unsigned __int64    ullAvailPhys;
	unsigned __int64    ullTotalPageFile;
	unsigned __int64    ullAvailPageFile;
	unsigned __int64    ullTotalVirtual;
	unsigned __int64    ullAvailVirtual;
	unsigned __int64    ullAvailExtendedVirtual;
} sysMemoryStatusEx_t;

typedef BOOL ( WINAPI *pfnGlobalMemoryStatusEx_t )( sysMemoryStatusEx_t *lpBuffer );

#define SYS_LOW_MEMORY_BYTES    0x8000000

/* ---- Sys_FistpRound  no-address ---- */
static int Sys_FistpRound( double x ) {
	int i;

	__asm {
		fld     x
		fistp   i
	}

	return i;
}

/* ---- Sys_MegsFromBytes  no-address ---- */
static int Sys_MegsFromBytes( double bytes ) {
	float   megs;
	int     mb;

	megs = (float) ( bytes * ( 1.0 / 1048576.0 ) );
	mb = Sys_FistpRound( (double) megs + 0.4999999990686774 );

	if ( bytes > (double) mb * 1048576.0 || mb > 1024 ) {
		return 1024;
	}
	return mb;
}

/* ---- Sys_GetSystemRam  0x00460C50 ---- */
int Sys_GetSystemRam( void ) {
	HMODULE                     kernel32;
	pfnGlobalMemoryStatusEx_t   pGlobalMemoryStatusEx;
	sysMemoryStatusEx_t         statex;
	MEMORYSTATUS                stat;

	if ( sys_sysMBValue != 0 ) {
		return sys_sysMBValue;
	}

	pGlobalMemoryStatusEx = NULL;
	kernel32 = GetModuleHandleA( "kernel32.dll" );
	if ( kernel32 ) {
		pGlobalMemoryStatusEx = (pfnGlobalMemoryStatusEx_t)
		                        GetProcAddress( kernel32, "GlobalMemoryStatusEx" );
	}

	if ( pGlobalMemoryStatusEx ) {
		memset( &statex, 0, sizeof( statex ) );
		statex.dwLength = 64;
		pGlobalMemoryStatusEx( &statex );

		if ( statex.ullAvailVirtual < SYS_LOW_MEMORY_BYTES ) {
			Com_Printf( "WARNING: low memory -- only %i MB of address space free\n",
			            (int) ( statex.ullAvailVirtual >> 20 ) );
		}
		sys_sysMBValue = Sys_MegsFromBytes( (double) (__int64) statex.ullTotalPhys );
	} else {
		memset( &stat, 0, sizeof( stat ) );
		stat.dwLength = sizeof( stat );
		GlobalMemoryStatus( &stat );

		if ( stat.dwAvailVirtual < SYS_LOW_MEMORY_BYTES ) {
			Com_Printf( "WARNING: low memory -- only %i MB of address space free\n",
			            (int) ( stat.dwAvailVirtual >> 20 ) );
		}
		sys_sysMBValue = Sys_MegsFromBytes( (double) stat.dwTotalPhys );
	}

	return sys_sysMBValue;
}

extern int      Sys_VideoRamFromRenderer( const char *renderer );

#define GL_RENDERER     0x1F01

/* ---- Sys_GetVideoRam  0x00461390 ----  VERIFIED */
/* Returns BYTES (Sys_VideoRamFromRenderer's megs << 20), 0 when no GL
   context can be made or the renderer is not in the card table. */
int Sys_GetVideoRam( void ) {
	HWND        hWnd;
	HDC         hDC;
	HMODULE     hOpenGL;
	HGLRC       ( WINAPI *qwglCreateContext )( HDC );
	BOOL        ( WINAPI *qwglMakeCurrent )( HDC, HGLRC );
	BOOL        ( WINAPI *qwglDeleteContext )( HGLRC );
	const char  *( WINAPI *qglGetString )( unsigned int );
	PIXELFORMATDESCRIPTOR pfd;
	int         pixelformat;
	HGLRC       hGLRC;
	int         vidMem;
	int         ram;

	ram = 0;
	vidMem = 0;

	hWnd = CreateWindowEx( 0, "static", "dummy", 0, 0, 0, 1, 1,
	                       NULL, NULL, GetModuleHandle( NULL ), NULL );
	if ( hWnd ) {
		hDC = GetDC( hWnd );
		if ( hDC ) {
			hOpenGL = LoadLibrary( "opengl32.dll" );
			if ( hOpenGL ) {
				qwglCreateContext = (void *) GetProcAddress( hOpenGL, "wglCreateContext" );
				qwglMakeCurrent = (void *) GetProcAddress( hOpenGL, "wglMakeCurrent" );
				qwglDeleteContext = (void *) GetProcAddress( hOpenGL, "wglDeleteContext" );
				qglGetString = (void *) GetProcAddress( hOpenGL, "glGetString" );

				memset( &pfd, 0, sizeof( pfd ) );
				pfd.nSize = sizeof( pfd );
				pfd.nVersion = 1;
				pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
				pfd.iPixelType = PFD_TYPE_RGBA;
				pfd.cColorBits = 0;
				pfd.cDepthBits = 0;

				pixelformat = ChoosePixelFormat( hDC, &pfd );
				if ( pixelformat ) {
					DescribePixelFormat( hDC, pixelformat, sizeof( pfd ), &pfd );
					if ( SetPixelFormat( hDC, pixelformat, &pfd ) ) {
						hGLRC = qwglCreateContext( hDC );
						if ( hGLRC ) {
							qwglMakeCurrent( hDC, hGLRC );
							vidMem = Sys_VideoRamFromRenderer( qglGetString( GL_RENDERER ) );
							qwglMakeCurrent( NULL, NULL );
							qwglDeleteContext( hGLRC );
						}
					}
				}
				FreeLibrary( hOpenGL );
				ram = vidMem;
			}
			DeleteDC( hDC );
		}
		DestroyWindow( hWnd );
	}

	return ram;
}

/* ---- Sys_SetConfigCvars  0x00462850 ----  VERIFIED */
void Sys_SetConfigCvars( int configSum ) {
	Cvar_Get( "sys_cpuMHz", "0", CVAR_ARCHIVE | CVAR_ROM );
	Cvar_Get( "sys_sysMB", "0", CVAR_ARCHIVE | CVAR_ROM );
	Cvar_Get( "sys_vidMB", "0", CVAR_ARCHIVE | CVAR_ROM );

	Sys_GetCPUSpeed();

	Cvar_Set2( "sys_cpuMHz", va( "%lg", sys_cpuMHzValue ), qtrue );
	Cvar_Set2( "sys_sysMB", va( "%i", Sys_GetSystemRam() ), qtrue );
	Cvar_Set2( "sys_vidMB", va( "%i", sys_vidMBValue ), qtrue );
	Cvar_Set2( "sys_configSum", va( "%i", configSum ), qtrue );
}

/* ---- Sys_ShouldReconfigure  0x00462750 ----  VERIFIED */
qboolean Sys_ShouldReconfigure( void ) {
	cvar_t  *cpuMHz, *sysMB, *vidMB;
	double  measuredMHz;
	int     measuredSysMB, measuredVidMB;

	cpuMHz = Cvar_Get( "sys_cpuMHz", "0", CVAR_ARCHIVE | CVAR_ROM );
	sysMB = Cvar_Get( "sys_sysMB", "0", CVAR_ARCHIVE | CVAR_ROM );
	vidMB = Cvar_Get( "sys_vidMB", "0", CVAR_ARCHIVE | CVAR_ROM );

	measuredMHz = (double) Sys_GetCPUSpeed();
	measuredSysMB = Sys_GetSystemRam();
	measuredVidMB = sys_vidMBValue;

	if ( cpuMHz->value <= measuredMHz * 1.100000023841858
	     && cpuMHz->value >= measuredMHz * 0.8999999761581421
	     && sysMB->integer <= measuredSysMB + 32
	     && sysMB->integer >= measuredSysMB - 32
	     && vidMB->integer <= measuredVidMB + 16
	     && vidMB->integer >= measuredVidMB - 16 ) {
		return qfalse;
	}

	Sys_SetConfigCvars( 0 );
	Com_Printf( "Hardware configuration changed since the last run.\n" );

	return qfalse;
}

/* ---- Sys_GetCurrentUser  0x00465C20 ----  VERIFIED */
char *Sys_GetCurrentUser( void ) {
	static char s_userName[1024];
	DWORD       size;

	size = sizeof( s_userName );
	if ( !GetUserNameA( s_userName, &size ) ) {
		strcpy( s_userName, "player" );
	}
	if ( !s_userName[0] ) {
		strcpy( s_userName, "player" );
	}
	return s_userName;
}

#define CPUID_GENERIC               0x00
#define CPUID_AXP                   0x10
#define CPUID_INTEL_UNSUPPORTED     0x20
#define CPUID_INTEL_PENTIUM         0x21
#define CPUID_INTEL_MMX             0x22
#define CPUID_INTEL_KATMAI          0x23
#define CPUID_AMD_3DNOW             0x30

/* ---- Sys_Init  0x004638C0 ----  VERIFIED */
void Sys_Init( void ) {
	OSVERSIONINFOA  osversion;
	const char      *arch;
	char            *forced;
	int             cpuid;

	timeBeginPeriod( 1 );

	Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
	Cmd_AddCommand( "net_restart", Sys_Net_Restart_f );

	memset( &osversion, 0, sizeof( osversion ) );
	osversion.dwOSVersionInfoSize = sizeof( osversion );
	if ( !GetVersionExA( &osversion ) ) {
		Sys_Error( "Couldn't get OS info" );
	}
	memcpy( VersionInformation, &osversion, sizeof( osversion ) );

	if ( osversion.dwMajorVersion < 4 ) {
		Sys_Error( "Call of Duty Multiplayer requires Windows version 4 or greater" );
	}
	if ( osversion.dwPlatformId == VER_PLATFORM_WIN32s ) {
		Sys_Error( "Call of Duty Multiplayer doesn't run on Win32s" );
	}

	if ( osversion.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
		arch = "winnt";
	} else if ( osversion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
		if ( LOWORD( osversion.dwBuildNumber ) >= 0x7CE ) {
			arch = "win98";
		} else if ( LOWORD( osversion.dwBuildNumber ) >= 0x457 ) {
			arch = "win95 osr2.x";
		} else {
			arch = "win95";
		}
	} else {
		arch = "unknown Windows variant";
	}
	Cvar_Set2( "arch", arch, qtrue );

	Cvar_Get( "win_hinstance", va( "%i", g_wv_hInstance ), CVAR_ROM );
#ifndef DEDICATED
	Cvar_Get( "win_wndproc", va( "%i", (int) MainWndProc ), CVAR_ROM );
#endif

	Cvar_Get( "sys_cpustring", "detect", 0 );

	forced = Cvar_VariableString( "sys_cpustring" );
	if ( !Q_stricmp( forced, "detect" ) ) {
		Com_Printf( "...detecting CPU, found " );

		cpuid = Sys_GetProcessorId();

		switch ( cpuid ) {
		case CPUID_GENERIC:
			Cvar_Set2( "sys_cpustring", "generic", qtrue );
			break;
		case CPUID_INTEL_UNSUPPORTED:
			Cvar_Set2( "sys_cpustring", "x86 (pre-Pentium)", qtrue );
			break;
		case CPUID_INTEL_PENTIUM:
			Cvar_Set2( "sys_cpustring", "x86 (P5/PPro, non-MMX)", qtrue );
			break;
		case CPUID_INTEL_MMX:
			Cvar_Set2( "sys_cpustring", "x86 (P5/Pentium2, MMX)", qtrue );
			break;
		case CPUID_INTEL_KATMAI:
			Cvar_Set2( "sys_cpustring", "Intel Pentium III", qtrue );
			break;
		case CPUID_AMD_3DNOW:
			Cvar_Set2( "sys_cpustring", "AMD w/ 3DNow!", qtrue );
			break;
		case CPUID_AXP:
			Cvar_Set2( "sys_cpustring", "Alpha AXP", qtrue );
			break;
		default:
			Com_Error( ERR_FATAL, "\x15" "Unknown cpu type %d\n", cpuid );
			break;
		}
	} else {
		Com_Printf( "...forcing CPU type to " );

		if ( !Q_stricmp( forced, "generic" ) ) {
			cpuid = CPUID_GENERIC;
		} else if ( !Q_stricmp( forced, "x87" ) ) {
			cpuid = CPUID_INTEL_PENTIUM;
		} else if ( !Q_stricmp( forced, "mmx" ) ) {
			cpuid = CPUID_INTEL_MMX;
		} else if ( !Q_stricmp( forced, "3dnow" ) ) {
			cpuid = CPUID_AMD_3DNOW;
		} else if ( !Q_stricmp( forced, "PentiumIII" ) ) {
			cpuid = CPUID_INTEL_KATMAI;
		} else if ( !Q_stricmp( forced, "axp" ) ) {
			cpuid = CPUID_AXP;
		} else {
			Com_Printf( "WARNING: unknown sys_cpustring '%s'\n", forced );
			cpuid = CPUID_GENERIC;
		}
	}
	Cvar_SetValue( "sys_cpuid", (float) cpuid );
	Com_Printf( "%s\n", Cvar_VariableString( "sys_cpustring" ) );

	Com_Printf( "Measured CPU speed is %.2lf GHz\n",
	            (double) Sys_GetCPUSpeed() * 0.001 );
	Com_Printf( "System memory is %i MB (capped at 1 GB)\n", Sys_GetSystemRam() );
	Com_Printf( "Video card memory is %i MB\n", sys_vidMBValue );
	Com_Printf( "Streaming SIMD Extensions (SSE) %ssupported\n",
	            sys_hasSSE ? "" : "not " );
	Com_Printf( "\n" );

	Cvar_Set2( "username", Sys_GetCurrentUser(), qtrue );

	IN_Init();
}

/* ---- Sys_ShutdownTimer  no-address ---- */
void Sys_ShutdownTimer( void ) {
	timeEndPeriod( 1 );
}

/* ---- Sys_BuildDllPath  no-address ----  VERIFIED */
static void Sys_BuildDllPath( const char *base, const char *game,
                              const char *dllName, char *out ) {
	/* retail 0x00463434 / 0x004634D3: FS_BuildOSPath3( fs_basepath@<eax>, game, dllName, out, 0 ). Empty game defaults to fs_gamedir inside. */
	FS_BuildOSPath3( base, game, dllName, out, 0 );
}

/* ---- Sys_LoadDll  0x004633A0 ---- */
void *Sys_LoadDll( const char *name, char *fqpath,
                   int ( **entryPoint )( int, ... ),
                   int ( *systemcalls )( int, ... ) ) {
	char        dllName[64];
	char        ospath[MAX_OSPATH];
	char        basedir[MAX_OSPATH];
	const char  *game;
	cvar_t      *var;
	HMODULE     libHandle;
	const char  *loadedFrom;
	void        ( *dllEntry )( int ( *syscallptr )( int, ... ) );

	*fqpath = 0;

	Com_sprintf( dllName, sizeof( dllName ), "%s_mp_x86.dll", name );

	var = Cvar_FindVar( "fs_basepath" );
	Q_strncpyz( basedir, ( var && var->string ) ? var->string : "", sizeof( basedir ) );

	Cvar_FindVar( "fs_cdpath" );

	game = "";
	var = Cvar_FindVar( "fs_game" );
	if ( var ) {
		game = var->string;
	}

	Sys_BuildDllPath( basedir, game, dllName, ospath );

	if ( strncmp( name, "game", 4 ) != 0 ) {
		if ( !FS_ExtractFromPakFile_m( ospath, (int) game, dllName ) ) {
			if ( cl_connectedToPureServer ) {
				Com_Error( ERR_DROP,
				           va( "EXE_ERR_GAME_FAILED_PURE_CHECK\x15%s", dllName ) );
			}
		}
	}

	Com_Printf( "Sys_LoadDll(%s)... ", ospath );
	libHandle = LoadLibraryA( ospath );
	loadedFrom = ospath;

	if ( !libHandle ) {
		Com_Printf( "failed\n" );

		Sys_BuildDllPath( basedir, "main", dllName, ospath );
		Com_Printf( "Sys_LoadDll(%s)... ", ospath );
		libHandle = LoadLibraryA( ospath );
		loadedFrom = ospath;

		if ( !libHandle ) {
			Com_Printf( "failed\n" );

			Com_Printf( "Sys_LoadDll(%s)... ", dllName );
			libHandle = LoadLibraryA( dllName );
			loadedFrom = dllName;

			if ( !libHandle ) {
				Com_Printf( "failed with Windows error %i\n",
				            (int) GetLastError() );
				return NULL;
			}
		}
	}
	Com_Printf( "ok\n" );

	strncpy( fqpath, loadedFrom, 63 );
	fqpath[63] = 0;

	dllEntry = (void ( * )( int ( * )( int, ... ) ))
	           GetProcAddress( libHandle, "dllEntry" );
	*entryPoint = (int ( * )( int, ... )) GetProcAddress( libHandle, "vmMain" );

	if ( !*entryPoint || !dllEntry ) {
		Com_Printf( "Sys_LoadDll(%s) failed to find dllEntry/vmMain\n", name );
		FreeLibrary( libHandle );
		return NULL;
	}

	dllEntry( systemcalls );

	return libHandle;
}
