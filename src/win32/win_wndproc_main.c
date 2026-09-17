/*
 * @fidelity: likely
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"

extern int  MapKey( UINT uCode, int lParam );
extern int  VID_AppActivate( int minimized, int active );
extern int  WIN_DisableAltTab( void );
extern int  WIN_EnableAltTab( void );

/* win32/win_input.c -- one cdecl stack argument (0x004617D8). */
extern int  IN_MouseEvent( int mstate );

extern void Sys_QueEvent( int time, int type, int value, int value2,
                          int ptrLength, void *ptr );
extern int  g_wv_sysMsgTime;

/* miles/snd_miles.c, 0x00450260 -- retail hands the new HWND to the sound layer from WM_CREATE. */
extern int  MSS_SetDirectSoundHWND_m( void *hWnd );

/* MSWHEEL_ROLLMSG, registered in WM_CREATE. 0x008E58C0. s_mswheelRollMsg, g_wv_hWnd, g_wv_activeApp, in_appactive, cls_state, vid_xpos, vid_ypos and r_fullscreen all come from cod1_globals.h. */

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL   0x020A
#endif
#ifndef MK_MBUTTON
#define MK_MBUTTON      0x0010
#endif

#define K_MWHEELDOWN    205
#define K_MWHEELUP      206
#define SE_KEY          1
#define SE_CHAR         2
#define CA_LOADING      4

/* ---- MainWnd_Wheel  no-address ---- */
static void MainWnd_Wheel( int up )
{
    int key = up ? K_MWHEELUP : K_MWHEELDOWN;

    Sys_QueEvent( g_wv_sysMsgTime, SE_KEY, key, 1, 0, 0 );
    Sys_QueEvent( g_wv_sysMsgTime, SE_KEY, key, 0, 0, 0 );
}

/* ---- MainWnd_Mouse  no-address ---- */
static void MainWnd_Mouse( WPARAM wParam )
{
    int mstate = 0;

    if ( wParam & MK_LBUTTON ) { mstate |= 1; }
    if ( wParam & MK_RBUTTON ) { mstate |= 2; }
    if ( wParam & MK_MBUTTON ) { mstate |= 4; }

    IN_MouseEvent( mstate );
}

/* ---- MainWndProc  0x00466BE0 ---- */
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam )
{
    SetThreadExecutionState( 2 );

    if ( s_mswheelRollMsg && Msg == (UINT) s_mswheelRollMsg ) {
        MainWnd_Wheel( (int) wParam > 0 );
        return DefWindowProc( hWnd, Msg, wParam, lParam );
    }

    switch ( Msg ) {

    case WM_CREATE:
        g_wv_hWnd = (void *) hWnd;
        MSS_SetDirectSoundHWND_m( (void *) hWnd );
        vid_xpos     = Cvar_Get( "vid_xpos", "3", CVAR_ARCHIVE );
        vid_ypos     = Cvar_Get( "vid_ypos", "22", CVAR_ARCHIVE );
        r_fullscreen = Cvar_Get( "r_fullscreen", "1",
                                 CVAR_ARCHIVE | CVAR_LATCH );
        s_mswheelRollMsg = (int) RegisterWindowMessage( "MSWHEEL_ROLLMSG" );
        if ( r_fullscreen->integer ) {
            WIN_DisableAltTab();
        } else {
            WIN_EnableAltTab();
        }
        break;

    case WM_DESTROY:
        g_wv_hWnd = NULL;
        if ( r_fullscreen && r_fullscreen->integer ) {
            WIN_EnableAltTab();
        }
        break;

    case WM_CLOSE:
        Cbuf_AddText( "quit\n" );
        break;

    case WM_MOVE:
        if ( r_fullscreen && !r_fullscreen->integer ) {
            RECT r;
            LONG style;
            int  x = (int) (short) LOWORD( lParam );
            int  y = (int) (short) HIWORD( lParam );

            r.left = 0; r.top = 0; r.right = 1; r.bottom = 1;
            style = GetWindowLong( hWnd, GWL_STYLE );
            AdjustWindowRect( &r, style, FALSE );

            Cvar_SetValue( "vid_xpos", (float) ( r.left + x ) );
            Cvar_SetValue( "vid_ypos", (float) ( r.top  + y ) );
            vid_xpos->modified = qfalse;
            vid_ypos->modified = qfalse;
            if ( g_wv_activeApp ) {
                in_appactive = 1;
            }
        }
        break;

    case WM_ACTIVATE:
        VID_AppActivate( HIWORD( wParam ), LOWORD( wParam ) != 0 );
        break;

    case WM_SYSCOMMAND:
        if ( wParam == SC_SCREENSAVE ) {
            return 0;
        }
        break;

    case WM_POWERBROADCAST:
        if ( wParam <= 1 ) {
            return 0x424D5144;
        }
        break;

    case WM_SYSKEYDOWN:
        if ( wParam == 13 ) {
            if ( cls_state != CA_LOADING && r_fullscreen ) {
                Cvar_SetValue( "r_fullscreen",
                               (float) ( r_fullscreen->integer == 0 ) );
                Cbuf_AddText( "vid_restart\n" );
            }
            return 0;
        }
    case WM_KEYDOWN:
        {
            int key = MapKey( (UINT) wParam, (int) lParam ) & 0xFF;
            if ( key ) {
                Sys_QueEvent( g_wv_sysMsgTime, SE_KEY, key, 1, 0, 0 );
            }
        }
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        {
            int key = MapKey( (UINT) wParam, (int) lParam ) & 0xFF;
            if ( key ) {
                Sys_QueEvent( g_wv_sysMsgTime, SE_KEY, key, 0, 0, 0 );
            }
        }
        break;

    case WM_CHAR:
        Sys_QueEvent( g_wv_sysMsgTime, SE_CHAR, (int) wParam, 0, 0, 0 );
        break;

    case WM_MOUSEWHEEL:
        MainWnd_Wheel( (short) HIWORD( wParam ) > 0 );
        break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        MainWnd_Mouse( wParam );
        break;

    default:
        break;
    }

    return DefWindowProc( hWnd, Msg, wParam, lParam );
}
