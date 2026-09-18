/*
 * @fidelity: verified
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

extern char *CopyStringInternal( const char *in );
extern void *Cvar_Set2( const char *var_name, const char *value, int force );
extern void  Sys_QueEvent( int time, int type, int value, int value2,
                           int ptrLength, void *ptr );
extern char *SEH_GetLocalizedString_m( const char *key );
extern char *va( char *format, ... );

extern const char *com_quitReason;

void Conbuf_AppendText( const char *msg );

extern void  Sys_TtyConsole_Init( void );
extern void  Sys_TtyConsole_Shutdown( void );
extern void  Sys_TtyConsole_Print( const char *msg );
extern char *Sys_TtyConsole_Input( void );

extern void *g_wv_hInstance;
extern void *g_splashWnd;

extern void *com_dedicated;
extern void *com_viewlog;
#define CVAR_INTEGER( p )   ( (p) ? *(int *)( (char *)(p) + 32 ) : 0 )

/* qcommon.h's sysEventType_t: SE_CONSOLE is 5 (0x00466003, and Sys_GetEvent at 0x004636A0). */
#define SE_CONSOLE          5

#define SYSCON_DEFAULT_WIDTH    540
#define SYSCON_DEFAULT_HEIGHT   450

#define COPY_ID                 1
#define QUIT_ID                 2
#define CLEAR_ID                3
#define ERRORBOX_ID             10
#define EDIT_ID                 100
#define INPUT_ID                101

#define CONSOLE_BG_COLOR        0x00647556
#define ERRORBOX_BG_COLOR       0x00808080

#define CONSOLE_WINDOW_STYLE    ( WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX )

typedef struct {
	HWND        hWnd;
	HWND        hwndBuffer;
	HWND        hwndButtonClear;
	HWND        hwndButtonCopy;
	HWND        hwndButtonQuit;
	HWND        hwndErrorBox;
	HWND        hwndInputLine;

	HBRUSH      hbrEditBackground;
	HBRUSH      hbrErrorBackground;
	HFONT       hfBufferFont;

	WNDPROC     SysInputLineWndProc;

	char        errorString[512];
	char        consoleText[512];
	char        returnedText[512];

	int         visLevel;
	int         quitOnClose;
	int         windowWidth;
	int         windowHeight;
	int         timePolarity;
} WinConData;

static WinConData   s_wcd;

static int          s_totalChars;

static int          s_noWindow = -1;

static int          s_consoleInitialized;

static LRESULT CALLBACK ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

static LRESULT CALLBACK InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

/* ---- Sys_ConsoleWindowWanted  no-address ---- */
static int Sys_ConsoleWindowWanted( void ) {
	char    cmd[1024];
	LPSTR   raw;

	if ( s_noWindow >= 0 ) {
		return !s_noWindow;
	}

	s_noWindow = 0;
	raw = GetCommandLineA();
	if ( raw ) {
		strncpy( cmd, raw, sizeof( cmd ) - 1 );
		cmd[sizeof( cmd ) - 1] = 0;
		_strlwr( cmd );
		if ( strstr( cmd, "-nowindow" ) != NULL ) {
			s_noWindow = 1;
		}
	}

	return !s_noWindow;
}

/* ---- ConWndProc  0x00465E80 ---- */
static LRESULT CALLBACK ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	int     cx, cy;
	int     bw;
	int     x, y, w, h;
	char    *cmdString;
	HDC     hdc;

	switch ( uMsg ) {

	case WM_CREATE:
		s_wcd.hbrEditBackground  = CreateSolidBrush( CONSOLE_BG_COLOR );
		s_wcd.hbrErrorBackground = CreateSolidBrush( ERRORBOX_BG_COLOR );
		SetTimer( hWnd, 1, 1000, NULL );
		break;

	case WM_SIZE:
		cx = (int) LOWORD( lParam );
		cy = (int) HIWORD( lParam );

		x = 5;
		y = 40;
		w = cx - 15;
		h = cy - 100;
		SetWindowPos( s_wcd.hwndBuffer, NULL, x, y, w, h, 0 );

		y = h + 48;
		SetWindowPos( s_wcd.hwndInputLine, NULL, x, y, w, 20, 0 );

		y = h + 72;
		bw = (int) ( (double) cx * ( 1.0 / (double) SYSCON_DEFAULT_WIDTH ) * 72.0 );
		SetWindowPos( s_wcd.hwndButtonCopy,  NULL, 5,             y, bw, 24, 0 );
		SetWindowPos( s_wcd.hwndButtonClear, NULL, bw + 7,        y, bw, 24, 0 );
		SetWindowPos( s_wcd.hwndButtonQuit,  NULL, cx - bw - 15,  y, bw, 24, 0 );

		s_wcd.windowHeight = cy;
		s_wcd.windowWidth  = cx;
		break;

	case WM_ACTIVATE:
		if ( LOWORD( wParam ) != WA_INACTIVE ) {
			SetFocus( s_wcd.hwndInputLine );
		}
		if ( !com_dedicated || CVAR_INTEGER( com_dedicated ) || !com_viewlog ) {
			break;
		}
		if ( CVAR_INTEGER( com_viewlog ) == 1 ) {
			if ( HIWORD( wParam ) ) {
				Cvar_Set2( "viewlog", "2", 1 );
			}
		} else if ( CVAR_INTEGER( com_viewlog ) == 2 ) {
			if ( !HIWORD( wParam ) ) {
				Cvar_Set2( "viewlog", "1", 1 );
			}
		}
		break;

	case WM_CLOSE:
		if ( com_dedicated && CVAR_INTEGER( com_dedicated ) ) {
			com_quitReason = "console window closed";
			cmdString = CopyStringInternal( "quit" );
			Sys_QueEvent( 0, SE_CONSOLE, 0, 0, (int) strlen( cmdString ) + 1, cmdString );
			return 0;
		}
		if ( s_wcd.quitOnClose ) {
			PostQuitMessage( 0 );
			return 0;
		}
		s_wcd.quitOnClose = 0;
		if ( s_wcd.visLevel ) {
			s_wcd.visLevel = 0;
			if ( s_wcd.hWnd ) {
				ShowWindow( s_wcd.hWnd, SW_HIDE );
			}
		}
		Cvar_Set2( "viewlog", "0", 1 );
		return 0;

	case WM_COMMAND:
		switch ( wParam ) {
		case COPY_ID:
			SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, (LPARAM) -1 );
			SendMessage( s_wcd.hwndBuffer, WM_COPY, 0, 0 );
			break;
		case QUIT_ID:
			if ( s_wcd.quitOnClose ) {
				PostQuitMessage( 0 );
			} else {
				com_quitReason = "console quit button";
				cmdString = CopyStringInternal( "quit" );
				Sys_QueEvent( 0, SE_CONSOLE, 0, 0, (int) strlen( cmdString ) + 1, cmdString );
			}
			break;
		case CLEAR_ID:
			SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, (LPARAM) -1 );
			SendMessage( s_wcd.hwndBuffer, EM_REPLACESEL, 0, (LPARAM) "" );
			UpdateWindow( s_wcd.hwndBuffer );
			break;
		}
		break;

	case WM_TIMER:
		if ( wParam == 1 ) {
			s_wcd.timePolarity = !s_wcd.timePolarity;
			if ( s_wcd.hwndErrorBox ) {
				InvalidateRect( s_wcd.hwndErrorBox, NULL, FALSE );
			}
		}
		break;

	case WM_CTLCOLORSTATIC:
		hdc = (HDC) wParam;
		if ( (HWND) lParam == s_wcd.hwndBuffer ) {
			SetBkColor( hdc, CONSOLE_BG_COLOR );
			SetTextColor( hdc, 0x00FFFFFF );
			return (LRESULT) s_wcd.hbrEditBackground;
		}
		if ( (HWND) lParam == s_wcd.hwndErrorBox ) {
			SetBkColor( hdc, ERRORBOX_BG_COLOR );
			SetTextColor( hdc, s_wcd.timePolarity ? 0x000000FF : 0x00000000 );
			return (LRESULT) s_wcd.hbrErrorBackground;
		}
		break;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

/* ---- InputLineWndProc  0x004662C0 ---- */
static LRESULT CALLBACK InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	char    inputBuffer[1024];

	switch ( uMsg ) {

	case WM_KILLFOCUS:
		if ( (HWND) wParam == s_wcd.hWnd || (HWND) wParam == s_wcd.hwndErrorBox ) {
			SetFocus( hWnd );
			return 0;
		}
		break;

	case WM_CHAR:
		if ( wParam == VK_RETURN ) {
			GetWindowText( s_wcd.hwndInputLine, inputBuffer, sizeof( inputBuffer ) );
			strncat( s_wcd.consoleText, inputBuffer,
			         507 - strlen( s_wcd.consoleText ) );
			strcat( s_wcd.consoleText, "\n" );
			SetWindowText( s_wcd.hwndInputLine, "" );
			Conbuf_AppendText( va( "]%s\n", inputBuffer ) );
			return 0;
		}
		break;
	}

	return CallWindowProc( s_wcd.SysInputLineWndProc, hWnd, uMsg, wParam, lParam );
}

/* ---- Sys_CreateConsole  0x004663F0 ---- */
void Sys_CreateConsole( void ) {
	WNDCLASS    wc;
	RECT        rect;
	HDC         hDC;
	int         swidth, sheight;
	int         nHeight;

	if ( s_consoleInitialized ) {
		return;
	}
	s_consoleInitialized = 1;

	Sys_TtyConsole_Init();

	if ( !Sys_ConsoleWindowWanted() ) {
		return;
	}

	memset( &wc, 0, sizeof( wc ) );
	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) ConWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = (HINSTANCE) g_wv_hInstance;
	wc.hIcon         = LoadIcon( (HINSTANCE) g_wv_hInstance, MAKEINTRESOURCE( 1 ) );
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "CoD WinConsole";

	if ( !RegisterClass( &wc ) ) {
		return;
	}

	rect.left   = 0;
	rect.right  = SYSCON_DEFAULT_WIDTH;
	rect.top    = 0;
	rect.bottom = SYSCON_DEFAULT_HEIGHT;
	AdjustWindowRect( &rect, CONSOLE_WINDOW_STYLE, FALSE );

	hDC     = GetDC( GetDesktopWindow() );
	swidth  = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	s_wcd.windowWidth  = rect.right - rect.left + 1;
	s_wcd.windowHeight = rect.bottom - rect.top + 1;

	s_wcd.hWnd = CreateWindowEx( 0,
	                             "CoD WinConsole",
	                             "CoD Console",
	                             CONSOLE_WINDOW_STYLE,
	                             ( swidth - 600 ) / 2,
	                             ( sheight - SYSCON_DEFAULT_HEIGHT ) / 2,
	                             s_wcd.windowWidth,
	                             s_wcd.windowHeight,
	                             NULL, NULL,
	                             (HINSTANCE) g_wv_hInstance, NULL );
	if ( s_wcd.hWnd == NULL ) {
		return;
	}

	hDC     = GetDC( s_wcd.hWnd );
	nHeight = -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );
	s_wcd.hfBufferFont = CreateFont( nHeight, 0, 0, 0, FW_LIGHT, 0, 0, 0,
	                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
	                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                                 FF_MODERN | FIXED_PITCH, "Courier New" );
	ReleaseDC( s_wcd.hWnd, hDC );

	s_wcd.hwndInputLine = CreateWindowEx( 0, "edit", NULL,
	                                      WS_CHILD | WS_VISIBLE | WS_BORDER |
	                                      ES_LEFT | ES_AUTOHSCROLL,
	                                      6, 400, 528, 20,
	                                      s_wcd.hWnd, (HMENU) INPUT_ID,
	                                      (HINSTANCE) g_wv_hInstance, NULL );

	s_wcd.hwndButtonCopy = CreateWindowEx( 0, "button", NULL,
	                                       WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
	                                       5, 425, 72, 24,
	                                       s_wcd.hWnd, (HMENU) COPY_ID,
	                                       (HINSTANCE) g_wv_hInstance, NULL );
	SendMessage( s_wcd.hwndButtonCopy, WM_SETTEXT, 0, (LPARAM) "copy" );

	s_wcd.hwndButtonClear = CreateWindowEx( 0, "button", NULL,
	                                        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
	                                        82, 425, 72, 24,
	                                        s_wcd.hWnd, (HMENU) CLEAR_ID,
	                                        (HINSTANCE) g_wv_hInstance, NULL );
	SendMessage( s_wcd.hwndButtonClear, WM_SETTEXT, 0, (LPARAM) "clear" );

	s_wcd.hwndButtonQuit = CreateWindowEx( 0, "button", NULL,
	                                       WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
	                                       462, 425, 72, 24,
	                                       s_wcd.hWnd, (HMENU) QUIT_ID,
	                                       (HINSTANCE) g_wv_hInstance, NULL );
	SendMessage( s_wcd.hwndButtonQuit, WM_SETTEXT, 0, (LPARAM) "quit" );

	s_wcd.hwndBuffer = CreateWindowEx( 0, "edit", NULL,
	                                   WS_CHILD | WS_VISIBLE | WS_BORDER |
	                                   WS_VSCROLL | ES_MULTILINE |
	                                   ES_AUTOVSCROLL | ES_READONLY,
	                                   6, 70, 526, 324,
	                                   s_wcd.hWnd, (HMENU) EDIT_ID,
	                                   (HINSTANCE) g_wv_hInstance, NULL );
	SendMessage( s_wcd.hwndBuffer, WM_SETFONT, (WPARAM) s_wcd.hfBufferFont, 0 );

	s_wcd.SysInputLineWndProc =
	        (WNDPROC) SetWindowLong( s_wcd.hwndInputLine, GWL_WNDPROC,
	                                 (LONG) InputLineWndProc );
	SendMessage( s_wcd.hwndInputLine, WM_SETFONT, (WPARAM) s_wcd.hfBufferFont, 0 );

	SetFocus( s_wcd.hwndInputLine );

	s_wcd.visLevel = 0;
}

/* ---- Sys_DestroyConsole  0x00466710 ---- */
void Sys_DestroyConsole( void ) {
	if ( s_wcd.hWnd ) {
		ShowWindow( s_wcd.hWnd, SW_HIDE );
		CloseWindow( s_wcd.hWnd );
		DestroyWindow( s_wcd.hWnd );
		s_wcd.hWnd = NULL;
	}
	Sys_TtyConsole_Shutdown();
}

/* ---- Sys_ShowConsole  0x00466750 ---- */
void Sys_ShowConsole( int visLevel, int quitOnClose ) {
	s_wcd.quitOnClose = quitOnClose;

	if ( visLevel == s_wcd.visLevel ) {
		return;
	}
	s_wcd.visLevel = visLevel;

	if ( !s_wcd.hWnd ) {
		return;
	}

	switch ( visLevel ) {
	case 0:
		ShowWindow( s_wcd.hWnd, SW_HIDE );
		break;
	case 1:
		ShowWindow( s_wcd.hWnd, SW_SHOWNORMAL );
		SendMessage( s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff );
		break;
	case 2:
		ShowWindow( s_wcd.hWnd, SW_MINIMIZE );
		break;
	default:
		Sys_TtyConsole_Print( "Invalid visLevel sent to Sys_ShowConsole\n" );
		ShowWindow( s_wcd.hWnd, SW_SHOWNORMAL );
		break;
	}
}

/* ---- Sys_ConsoleInput  0x004667C0 ---- */
char *Sys_ConsoleInput( void ) {
	int len;

	if ( s_wcd.consoleText[0] ) {
		strcpy( s_wcd.returnedText, s_wcd.consoleText );
		s_wcd.consoleText[0] = 0;

		len = strlen( s_wcd.returnedText );
		while ( len > 0 && ( s_wcd.returnedText[len - 1] == '\n' ||
		                     s_wcd.returnedText[len - 1] == '\r' ) ) {
			s_wcd.returnedText[--len] = 0;
		}
		return s_wcd.returnedText;
	}

	return Sys_TtyConsole_Input();
}

/* ---- Conbuf_AppendText  0x004667F0 ---- */
void Conbuf_AppendText( const char *msg ) {
	char        buffer[0x8004];
	char        *b;
	const char  *p;
	int         i;
	int         n;
	size_t      msgLen;

	if ( !msg ) {
		return;
	}

	Sys_TtyConsole_Print( msg );

	if ( !s_wcd.hwndBuffer ) {
		return;
	}

	p = msg;
	msgLen = strlen( p );
	if ( msgLen > 0x3fff ) {
		p += msgLen - 0x3fff;
	}

	b = buffer;
	n = 0;
	if ( *p ) {
		i = 0;
		do {
			if ( n >= 0x7fff ) {
				break;
			}
			if ( p[i] == '\n' && p[i + 1] == '\r' ) {
				*b++ = '\r'; *b++ = '\n';
				n += 2;
				i++;
			} else if ( p[i] == '\r' ) {
				*b++ = '\r'; *b++ = '\n';
				n += 2;
			} else if ( p[i] == '\n' ) {
				*b++ = '\r'; *b++ = '\n';
				n += 2;
			} else if ( p[i] == '^' && p[i + 1] && p[i + 1] != '^' &&
			            p[i + 1] >= '0' && p[i + 1] <= '7' ) {
				i++;
			} else {
				*b++ = p[i];
				n++;
			}
			i++;
		} while ( p[i] );
	}
	*b = 0;

	n = b - buffer;
	s_totalChars += n;

	if ( (unsigned int) s_totalChars > 0x4000 ) {
		SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, (LPARAM) -1 );
		s_totalChars = n;
	} else {
		SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0xffff, 0xffff );
	}

	SendMessage( s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff );
	SendMessage( s_wcd.hwndBuffer, EM_SCROLLCARET, 0, 0 );
	SendMessage( s_wcd.hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer );
}

/* ---- Sys_SetErrorText  0x00466980 ---- */
void Sys_SetErrorText( const char *text ) {
	strncpy( s_wcd.errorString, text, sizeof( s_wcd.errorString ) - 1 );
	s_wcd.errorString[sizeof( s_wcd.errorString ) - 1] = 0;

	if ( !s_wcd.hWnd ) {
		Sys_TtyConsole_Print( "\n" );
		Sys_TtyConsole_Print( s_wcd.errorString );
		Sys_TtyConsole_Print( "\n" );
		return;
	}

	if ( s_wcd.hwndErrorBox ) {
		return;
	}

	s_wcd.hwndErrorBox = CreateWindowEx( 0, "static", NULL,
	                                     WS_CHILD | WS_VISIBLE | SS_SUNKEN,
	                                     6, 5, 526, 60,
	                                     s_wcd.hWnd, (HMENU) ERRORBOX_ID,
	                                     (HINSTANCE) g_wv_hInstance, NULL );
	SendMessage( s_wcd.hwndErrorBox, WM_SETFONT, (WPARAM) s_wcd.hfBufferFont, 0 );
	SetWindowText( s_wcd.hwndErrorBox, s_wcd.errorString );

	DestroyWindow( s_wcd.hwndInputLine );
	s_wcd.hwndInputLine = NULL;

	MessageBox( NULL, text, SEH_GetLocalizedString_m( "WIN_ERROR" ), MB_ICONERROR );
}

/* ---- Sys_CreateSplashWindow  0x00465C90 ---- */
void Sys_CreateSplashWindow( void ) {
	WNDCLASS    wc;
	RECT        rect;
	HBITMAP     bm;
	HWND        image;
	int         swidth, sheight;

	if ( !Sys_ConsoleWindowWanted() ) {
		return;
	}
	if ( g_splashWnd ) {
		return;
	}

	memset( &wc, 0, sizeof( wc ) );
	wc.lpfnWndProc   = (WNDPROC) DefWindowProc;
	wc.hInstance     = (HINSTANCE) g_wv_hInstance;
	wc.hIcon         = LoadIcon( (HINSTANCE) g_wv_hInstance, MAKEINTRESOURCE( 1 ) );
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH) COLOR_WINDOWFRAME;
	wc.lpszClassName = "CoD Splash Screen";

	if ( !RegisterClass( &wc ) ) {
		return;
	}

	swidth  = GetSystemMetrics( SM_CXSCREEN );
	sheight = GetSystemMetrics( SM_CYSCREEN );

	bm = (HBITMAP) LoadImage( NULL, "cod.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
	if ( !bm ) {
		return;
	}

	g_splashWnd = CreateWindowEx( WS_EX_APPWINDOW,
	                              "CoD Splash Screen",
	                              "Call of Duty 1.1x Multiplayer",
	                              WS_POPUP | WS_BORDER | WS_SYSMENU,
	                              ( swidth - 320 ) / 2,
	                              ( sheight - 100 ) / 2,
	                              320, 100,
	                              NULL, NULL,
	                              (HINSTANCE) g_wv_hInstance, NULL );
	if ( !g_splashWnd ) {
		return;
	}

	image = CreateWindowEx( 0, "Static", NULL,
	                        WS_CHILD | WS_VISIBLE | SS_BITMAP,
	                        0, 0, 320, 100,
	                        (HWND) g_splashWnd, NULL,
	                        (HINSTANCE) g_wv_hInstance, NULL );
	if ( !image ) {
		return;
	}

	SendMessage( image, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm );

	GetWindowRect( image, &rect );
	SetWindowPos( (HWND) g_splashWnd, NULL,
	              ( swidth - ( rect.right - rect.left ) ) / 2,
	              ( sheight - ( rect.bottom - rect.top ) ) / 2,
	              rect.right - rect.left,
	              rect.bottom - rect.top,
	              SWP_NOZORDER );
}

/* ---- Sys_DestroySplashWindow  0x00465E10 ---- */
void Sys_DestroySplashWindow( void ) {
	if ( g_splashWnd ) {
		ShowWindow( (HWND) g_splashWnd, SW_HIDE );
		DestroyWindow( (HWND) g_splashWnd );
		g_splashWnd = NULL;
	}
}
