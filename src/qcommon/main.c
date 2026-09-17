/*
 * qcommon/main.c
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                              LPSTR lpCmdLine, int nCmdShow );

extern void Cod1_InstallCrashReporter( void );

int main( int argc, char **argv ) {
	char *cmd;
	HMODULE k32;
	HWND ( __stdcall *pGetConsoleWindow )( void );
	HWND con;

	(void) argc;
	(void) argv;

	/* This entry runs only for the /subsystem:console dedicated build.  That
	 * console subsystem is what makes the win_syscon "CoD Console" GUI scroll
	 * correctly and keeps stdout as a real server log -- but the OS terminal
	 * window itself is redundant next to that GUI, so hide it.  The console
	 * (and stdout) stay live; only the window goes.  GetConsoleWindow is
	 * resolved at runtime because it is not declared by every SDK this tree
	 * compiles against. */
	k32 = GetModuleHandleA( "kernel32.dll" );
	if ( k32 ) {
		pGetConsoleWindow = ( HWND ( __stdcall * )( void ) )
			GetProcAddress( k32, "GetConsoleWindow" );
		if ( pGetConsoleWindow ) {
			con = pGetConsoleWindow();
			if ( con ) {
				ShowWindow( con, SW_HIDE );
			}
		}
	}

	Cod1_InstallCrashReporter();

	cmd = GetCommandLineA();

	if ( *cmd == '"' ) {
		cmd++;
		while ( *cmd && *cmd != '"' ) {
			cmd++;
		}
		if ( *cmd == '"' ) {
			cmd++;
		}
	} else {
		while ( *cmd && *cmd != ' ' && *cmd != '\t' ) {
			cmd++;
		}
	}
	while ( *cmd == ' ' || *cmd == '\t' ) {
		cmd++;
	}

	return WinMain( GetModuleHandleA( NULL ), NULL, cmd, SW_SHOWNORMAL );
}
