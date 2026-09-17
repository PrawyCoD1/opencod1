#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

static HANDLE   s_conin;
static HANDLE   s_conout;
static int      s_consoleActive;

static char     s_inputLine[1024];
static int      s_inputLen;

static char     s_returnedLine[1024];

/* ---- Sys_TtyConsole_Init  no-address ---- */
void Sys_TtyConsole_Init( void ) {
	if ( s_consoleActive ) {
		return;
	}

	s_conout = GetStdHandle( STD_OUTPUT_HANDLE );
	s_conin  = GetStdHandle( STD_INPUT_HANDLE );

	if ( s_conout == NULL || s_conout == INVALID_HANDLE_VALUE ) {
		s_conout = NULL;
		s_conin  = NULL;
	}

	setvbuf( stdout, NULL, _IONBF, 0 );

	s_consoleActive = 1;
	s_inputLen = 0;
	s_inputLine[0] = 0;
}

/* ---- Sys_TtyConsole_Shutdown  no-address ---- */
void Sys_TtyConsole_Shutdown( void ) {
	s_consoleActive = 0;
}

/* ---- Sys_TtyConsole_Print  no-address ---- */
void Sys_TtyConsole_Print( const char *msg ) {
	char buf[4096];
	int n = 0;

	if ( !msg ) {
		return;
	}
	if ( !s_consoleActive ) {
		Sys_TtyConsole_Init();
	}

	while ( *msg && n < (int) sizeof( buf ) - 2 ) {
		if ( msg[0] == '^' && msg[1] >= '0' && msg[1] <= '9' ) {
			msg += 2;
			continue;
		}
		if ( (unsigned char) *msg == 0x15 ) {
			msg++;
			continue;
		}
		buf[n++] = *msg++;
	}
	buf[n] = 0;

	fputs( buf, stdout );
}

/* ---- Sys_TtyConsole_Input  no-address ---- */
char *Sys_TtyConsole_Input( void ) {
	INPUT_RECORD rec;
	DWORD nevents, nread;
	char ch;

	if ( !s_consoleActive || s_conin == NULL || s_conin == INVALID_HANDLE_VALUE ) {
		return NULL;
	}

	for ( ;; ) {
		if ( !GetNumberOfConsoleInputEvents( s_conin, &nevents ) ) {
			return NULL;
		}
		if ( nevents == 0 ) {
			return NULL;
		}
		if ( !ReadConsoleInputA( s_conin, &rec, 1, &nread ) || nread != 1 ) {
			return NULL;
		}
		if ( rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown ) {
			continue;
		}

		ch = rec.Event.KeyEvent.uChar.AsciiChar;

		if ( ch == '\r' || ch == '\n' ) {
			fputs( "\n", stdout );
			s_inputLine[s_inputLen] = 0;
			strcpy( s_returnedLine, s_inputLine );
			s_inputLen = 0;
			s_inputLine[0] = 0;
			return s_returnedLine;
		}

		if ( ch == '\b' ) {
			if ( s_inputLen > 0 ) {
				s_inputLen--;
				s_inputLine[s_inputLen] = 0;
				fputs( "\b \b", stdout );
			}
			continue;
		}

		if ( ch >= ' ' && s_inputLen < (int) sizeof( s_inputLine ) - 1 ) {
			s_inputLine[s_inputLen++] = ch;
			s_inputLine[s_inputLen] = 0;
			fputc( ch, stdout );
		}
	}
}
