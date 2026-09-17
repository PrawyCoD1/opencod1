/*
 * @fidelity: likely
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"

#define QGL_OWNER
#include "../renderer/qgl.h"
#include "win_wgl.h"

#define QGL_PRINT_ALL       0
#define QGL_PRINT_WARNING   2

extern HMODULE glw_hinstOpenGL;     /* 0x019BFFEC, owned by win_glimp.c */

#define QGL_BIND_GL( n ) \
	( *(PROC *)&qgl##n = GetProcAddress( glw_hinstOpenGL, "gl" #n ) )
#define QGL_BIND_WGL( n ) \
	( *(PROC *)&qwgl##n = GetProcAddress( glw_hinstOpenGL, "wgl" #n ) )

/* ---- QGL_Shutdown  0x004C6AF0 ---- */
void QGL_Shutdown( void ) {
	ri_Printf( QGL_PRINT_ALL, "...shutting down QGL\n" );

	if ( glw_hinstOpenGL ) {
		ri_Printf( QGL_PRINT_ALL, "...unloading OpenGL DLL\n" );
		FreeLibrary( glw_hinstOpenGL );
	}
	glw_hinstOpenGL = NULL;

#define QGL_CORE_GL( n )    qgl##n  = NULL;
#define QGL_EXT_GL( n )     qgl##n  = NULL;
#define QGL_CORE_WGL( n )   qwgl##n = NULL;
#define QGL_EXT_WGL( n )    qwgl##n = NULL;
#include "../renderer/qgl_entries.h"

	qwglGetExtensionsStringEXT = NULL;
}

/* ---- QCL_Init  0x004C8300 ----  VERIFIED */
qboolean QCL_Init( const char *dllname ) {
	char	systemDir[1024];
	char	displayPath[1024];

	GetSystemDirectory( systemDir, sizeof( systemDir ) );

	ri_Printf( QGL_PRINT_ALL, "...initializing QGL\n" );

	if ( dllname[0] != '!' && strstr( "dllname", ".dll" ) == NULL ) {
		Com_sprintf( displayPath, sizeof( displayPath ), "%s\\%s",
					 systemDir, dllname );
	} else {
		strncpy( displayPath, dllname, sizeof( displayPath ) - 1 );
		displayPath[sizeof( displayPath ) - 1] = '\0';
	}

	ri_Printf( QGL_PRINT_ALL, "...calling LoadLibrary( '%s.dll' ): ",
			   displayPath );

	glw_hinstOpenGL = LoadLibrary( dllname );
	if ( !glw_hinstOpenGL ) {
		ri_Printf( QGL_PRINT_ALL, "failed\n" );
		return qfalse;
	}
	ri_Printf( QGL_PRINT_ALL, "succeeded\n" );

#define QGL_EXT_GL( n )     qgl##n  = NULL;
#define QGL_EXT_WGL( n )    qwgl##n = NULL;
#include "../renderer/qgl_entries.h"

#define QGL_CORE_GL( n )    QGL_BIND_GL( n );
#define QGL_CORE_WGL( n )   QGL_BIND_WGL( n );
#include "../renderer/qgl_entries.h"

	if ( !qwglCreateContext || !qwglDeleteContext || !qwglMakeCurrent ||
		 !qwglGetProcAddress || !qglGetString || !qglGetIntegerv ) {
		ri_Printf( QGL_PRINT_WARNING,
			"...GetProcAddress failed for a core GL/WGL entry point\n" );
		QGL_Shutdown();
		return qfalse;
	}

	return qtrue;
}

/* 0x00EA2CD8 and 0x00EA2CDC. */
static qboolean qgl_errorCheckingEnabled;
static qboolean qgl_drawProfilingEnabled;

/* ---- QGL_EnableErrorChecking  0x004CAEB0 ---- */
void QGL_EnableErrorChecking( qboolean enable ) {
	if ( ( qgl_errorCheckingEnabled != qfalse ) == ( enable != qfalse ) ) {
		return;
	}
	qgl_errorCheckingEnabled = enable;

	if ( enable ) {
#if defined( QGL_HAVE_CHECKED_WRAPPERS )
#define QGL_CORE_GL( n )    if ( qgl##n )  { qgl##n  = qglChecked##n;  }
#define QGL_EXT_GL( n )     if ( qgl##n )  { qgl##n  = qglChecked##n;  }
#define QGL_CORE_WGL( n )   if ( qwgl##n ) { qwgl##n = qwglChecked##n; }
#define QGL_EXT_WGL( n )    if ( qwgl##n ) { qwgl##n = qwglChecked##n; }
#include "../renderer/qgl_entries.h"
#endif
	} else {
#if defined( QGL_HAVE_DRIVER_SLOTS )
#define QGL_CORE_GL( n )    qgl##n  = dll##n;
#define QGL_EXT_GL( n )     qgl##n  = dll##n;
#define QGL_CORE_WGL( n )   qwgl##n = dllwgl##n;
#define QGL_EXT_WGL( n )    qwgl##n = dllwgl##n;
#include "../renderer/qgl_entries.h"
#endif
	}
}

/* ---- QGL_EnableDrawProfiling  0x004CAE60 ---- */
void QGL_EnableDrawProfiling( qboolean enable ) {
	if ( ( qgl_drawProfilingEnabled != qfalse ) == ( enable != qfalse ) ) {
		return;
	}
	qgl_drawProfilingEnabled = enable;

#if defined( QGL_HAVE_DRIVER_SLOTS ) && defined( QGL_HAVE_PROFILE_WRAPPERS )
	if ( enable ) {
		qglDrawElements         = QGL_ProfileDrawElements;
		qglDrawElementArrayATI  = QGL_ProfileDrawElementArrayATI;
	} else {
		qglDrawElements         = dllDrawElements;
		qglDrawElementArrayATI  = dllDrawElementArrayATI;
	}
#endif
}
