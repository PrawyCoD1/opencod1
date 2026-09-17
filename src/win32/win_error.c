#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

extern void Conbuf_AppendText( const char *msg );
extern void Sys_SetErrorText( const char *text );
extern void Sys_DestroyConsole( void );
extern void CL_Shutdown( void );
extern void Sys_DestroySplashWindow( void );

/* ---- Sys_Error  0x00463050 ----  VERIFIED */
void Sys_Error( const char *error, ... ) {
	va_list argptr;
	char text[4096];

	va_start( argptr, error );
	_vsnprintf( text, sizeof( text ) - 1, error, argptr );
	va_end( argptr );
	text[sizeof( text ) - 1] = 0;

	Sys_DestroySplashWindow();

	Conbuf_AppendText( "\n" );
	Conbuf_AppendText( text );
	Conbuf_AppendText( "\n" );
	Sys_SetErrorText( text );

	fflush( stdout );
	fflush( stderr );

	exit( 1 );
}

/* ---- Sys_Quit  no-address ---- */
void Sys_Quit( void ) {
	Sys_DestroyConsole();
	exit( 0 );
}
