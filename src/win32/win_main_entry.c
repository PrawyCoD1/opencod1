#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct cvar_s cvar_t;
extern int   Com_Init( char *commandLine );
extern void  Com_Frame( void );
extern void  Com_Printf( const char *fmt, ... );
extern void  IN_Frame( void );
extern void  Sys_CreateConsole( void );
extern void  Sys_CreateSplashWindow( void );
extern void  Sys_ShowConsole( int level, int quitOnClose );
extern int   SEH_LoadLocalizationFile_m( void );
extern void  Cod1_InstallCrashReporter( void );
extern void  Cod1_InstallHangWatchdog( void );
extern void  Cod1_HangWatchdogTick( void );
extern int   Sys_BuildInstanceMutexName_m( void );
extern int   Sys_CheckCrashOrRerun( void );
extern int   Sys_ChecksumModuleImage_m( int module );
extern float Sys_GetCPUSpeed( void );
extern int   Sys_GetSystemRam( void );
extern int   Sys_GetVideoRam( void );
extern int   Sys_DetectVideoRamMegs_m( void );
extern int   Sys_HasSSE_m( void );
extern int   Sys_DetectHardware_m( void );   /* 0x004626E0, win_main.c */

extern int  *com_dedicated;
extern int  *com_viewlog;
#define CVAR_INTEGER( p )   ( (p) ? *(int *)( (char *)(p) + 32 ) : 0 )

extern void *g_wv_hInstance;
extern void *g_wv_hWnd;
extern void *g_splashWnd;
extern int   g_wv_isMinimized;
extern char  commandLine[1024];
extern int   sys_timeBase;
extern int   sys_timeBaseInit;

/* ---- WinMain  0x004640B0 ---- */
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpCmdLine, int nCmdShow ) {
	char cwd[260];

	(void) nCmdShow;

#ifndef DEDICATED
	{
		extern int CL_UpdateHelper(const char *);
		int result = CL_UpdateHelper(lpCmdLine);
		if (result >= 0) return result;
	}
#endif

	Cod1_InstallCrashReporter();

	Cod1_InstallHangWatchdog();

	SEH_LoadLocalizationFile_m();

	if ( _strnicmp( lpCmdLine, "allowdupe", 9 ) || lpCmdLine[9] > 32 ) {
		Sys_BuildInstanceMutexName_m();
		if ( !Sys_CheckCrashOrRerun() ) {
			return 0;
		}
	}

	if ( hPrevInstance ) {
		return 0;
	}

	Sys_DetectHardware_m();
	Sys_ChecksumModuleImage_m( (int) hInstance );

	g_wv_hInstance = hInstance;
	strncpy( commandLine, lpCmdLine, sizeof( commandLine ) - 1 );
	commandLine[sizeof( commandLine ) - 1] = 0;

	Sys_CreateConsole();
	Sys_CreateSplashWindow();
	if ( g_splashWnd ) {
		ShowWindow( (HWND) g_splashWnd, SW_SHOW );
		UpdateWindow( (HWND) g_splashWnd );
	}

	SetErrorMode( SEM_FAILCRITICALERRORS );

	if ( !sys_timeBaseInit ) {
		sys_timeBase = (int) timeGetTime();
		sys_timeBaseInit = 1;
	}

	Com_Init( commandLine );

	_getcwd( cwd, sizeof( cwd ) );
	Com_Printf( "Working directory: %s\n", cwd );

	if ( !CVAR_INTEGER( com_dedicated ) && !CVAR_INTEGER( com_viewlog ) ) {
		Sys_ShowConsole( 0, 0 );
	}

	SetFocus( (HWND) g_wv_hWnd );

	while ( 1 ) {
		Cod1_HangWatchdogTick();
		if ( g_wv_isMinimized || CVAR_INTEGER( com_dedicated ) ) {
			Sleep( 5 );
		}
		IN_Frame();
		Com_Frame();
	}

	return 0;
}
