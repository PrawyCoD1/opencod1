/*
 * @fidelity: likely
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef LOBYTE
#undef LOWORD
#undef HIBYTE
#undef HIWORD

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

int g_wv_isMinimized;

int WIN_IsGameWindowActive( void )
{
  HWND hWnd = (HWND) g_wv_hWnd;
  return hWnd && GetForegroundWindow() == hWnd && !IsIconic( hWnd );
}

int IN_DeactivateWin32Mouse( void );        /* 0x004616B0 */
int IN_MouseEvent( int mstate );            /* 0x004617D0 */
extern int Key_ClearStates();
extern int Sys_QueEvent();
extern int MSS_SetDirectSoundHWND_m();

/* 0x008E58C4. The "we have taken Alt-Tab away" latch, so the pair below is idempotent in both directions. cod1_globals.h declares it `int`. */

extern cvar_t *Cvar_FindVar( const char *var_name );

/* ---- WIN_AltTabHotkey  no-address ----  VERIFIED */
static void WIN_AltTabHotkey( int disable ) {
	cvar_t		*arch;
	const char	*archString;
	BOOL		pvParam;

	arch = Cvar_FindVar( "arch" );
	archString = ( arch && arch->string ) ? arch->string : "";

	if ( !Q_stricmpn( archString, "winnt", 99999 ) ) {
		if ( disable ) {
			RegisterHotKey( NULL, 0, MOD_ALT, VK_TAB );
		} else {
			UnregisterHotKey( NULL, 0 );
		}
	} else {
		SystemParametersInfoA( SPI_SETSCREENSAVERRUNNING,
			disable ? 1 : 0, &pvParam, 0 );
	}
}

/* ---- WIN_DisableAltTab  0x00466A30 ---- */
void WIN_DisableAltTab( void ) {
	if ( s_altTabDisabled ) {
		return;
	}
	WIN_AltTabHotkey( 1 );
	s_altTabDisabled = 1;
}

/* ---- WIN_EnableAltTab  0x00466AB0 ---- */
void WIN_EnableAltTab( void ) {
	if ( !s_altTabDisabled ) {
		return;
	}
	WIN_AltTabHotkey( 0 );
	s_altTabDisabled = 0;
}

/* ---- VID_AppActivate  0x00466B20 ----  VERIFIED */
int VID_AppActivate( int minimized, int fActive )
{
  int result;

  g_wv_isMinimized = minimized;
  Key_ClearStates();
  result = 0;
  if ( !fActive || g_wv_isMinimized )
  {
    g_wv_activeApp = 0;
    in_appactive = 0;
    if ( s_wmv_mouseInitialized )
    {
      if ( s_wmv_mouseActive )
      {
        s_wmv_mouseActive = 0;
        return IN_DeactivateWin32Mouse();
      }
    }
  }
  else
  {
    result = 1;
    g_wv_activeApp = 1;
    in_appactive = 1;
  }
  return result;
}

/* 0x00575828 -- vkToEngineKey[VK][extended], 0x92 VKs x 2 bytes.  cod1_globals.c/.h still
   define a zeroed `char vkToEngineKeyTable[96]` at the same address; it is a dead duplicate,
   referenced nowhere live. */
static const unsigned char s_vkToKey[0x92 * 2] = {
  0x00,0x00, 0xC8,0xC8, 0xC9,0xC9, 0x00,0x00, 0xCA,0xCA, 0xCB,0xCB, 0xCC,0xCC, 0x00,0x00,
  0x7F,0x7F, 0x09,0x09, 0x00,0x00, 0x00,0x00, 0xBA,0x00, 0x0D,0xBF, 0x00,0x00, 0x00,0x00,
  0xA0,0xA0, 0x9F,0x9F, 0x9E,0x9E, 0x99,0x99, 0x97,0x97, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x1B,0x1B, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x20,0x20, 0xB8,0xA4, 0xBE,0xA3, 0xBC,0xA6, 0xB6,0xA5, 0xB9,0x9C, 0xB7,0x9A, 0xBB,0x9D,
  0xBD,0x9B, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xC0,0xA1, 0xC1,0xA2, 0x00,0x00,
  0x30,0x30, 0x31,0x31, 0x32,0x32, 0x33,0x33, 0x34,0x34, 0x35,0x35, 0x36,0x36, 0x37,0x37,
  0x38,0x38, 0x39,0x39, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x00,0x00, 0x61,0x41, 0x62,0x42, 0x63,0x43, 0x64,0x44, 0x65,0x45, 0x66,0x46, 0x67,0x47,
  0x68,0x48, 0x69,0x49, 0x6A,0x4A, 0x6B,0x4B, 0x6C,0x4C, 0x6D,0x4D, 0x6E,0x4E, 0x6F,0x4F,
  0x70,0x50, 0x71,0x51, 0x72,0x52, 0x73,0x53, 0x74,0x54, 0x75,0x55, 0x76,0x56, 0x77,0x57,
  0x78,0x58, 0x79,0x59, 0x7A,0x5A, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0xC0,0xC0, 0xBC,0xBC, 0xBD,0xBD, 0xBE,0xBE, 0xB9,0xB9, 0xBA,0xBA, 0xBB,0xBB, 0xB6,0xB6,
  0xB7,0xB7, 0xB8,0xB8, 0xC6,0xC6, 0xC4,0xC4, 0x00,0x00, 0xC3,0xC3, 0xC1,0xC1, 0xC2,0xC2,
  0xA7,0xA7, 0xA8,0xA8, 0xA9,0xA9, 0xAA,0xAA, 0xAB,0xAB, 0xAC,0xAC, 0xAD,0xAD, 0xAE,0xAE,
  0xAF,0xAF, 0xB0,0xB0, 0xB1,0xB1, 0xB2,0xB2, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0xC5,0xC5, 0x00,0x00
};

/* 0x0057594C -- {from, to} pairs, zero-terminated; upper-ANSI characters from MapVirtualKeyA onto the engine's own >0x7F codes (the terminator pair is at +40). */
static const unsigned char s_charFixup[] = {
  0xB5,0x80, 0xBF,0x81, 0xDF,0x82, 0xE0,0x83, 0xE1,0x84, 0xE4,0x85, 0xE5,0x86,
  0xE6,0x87, 0xE7,0x88, 0xE8,0x89, 0xE9,0x8A, 0xEC,0x8B, 0xF1,0x8C, 0xF2,0x8D,
  0xF3,0x8E, 0xF6,0x8F, 0xF8,0x90, 0xF9,0x91, 0xFA,0x92, 0xFC,0x93, 0x00,0x00
};

/* ---- MapKey  0x00466B70 ---- */
int MapKey( unsigned int uCode, int lParam )
{
  unsigned int key;
  int i;

  if ( ( lParam & 0x00FF0000 ) == 0x00290000 )
  {
    return 126;
  }

  if ( uCode && uCode <= 0x91 )
  {
    key = s_vkToKey[2 * uCode + ( ( lParam >> 24 ) & 1 )];
    if ( key )
    {
      return (int) key;
    }
  }

  key = (unsigned char) MapVirtualKeyA( uCode, 2 );
  if ( key > 0x7F )
  {
    for ( i = 0; s_charFixup[2 * i]; ++i )
    {
      if ( key == s_charFixup[2 * i] )
      {
        key = s_charFixup[2 * i + 1];
        break;
      }
    }
  }
  return (int) key;
}

#if 0
int __cdecl MainWndProc(const char *a1, HWND hWnd, UINT Msg, unsigned int wParam, LPARAM lParam)
{
  int result;
  char *integer;
  LONG WindowLongA;
  cvar_t *v8;
  unsigned __int8 v9;
  unsigned __int8 v10;
  int v11;
  int v12;
  float value;
  float valuea;
  float valueb;
  struct tagRECT Rect; // [esp+10h] [ebp-10h] BYREF

  SetThreadExecutionState(2u);
  if ( Msg == s_mswheelRollMsg )
  {
    if ( (int)wParam <= 0 )
    {
      Sys_QueEvent(g_wv_sysMsgTime, 1, 205, 1, 0, 0);
      v12 = 205;
    }
    else
    {
      Sys_QueEvent(g_wv_sysMsgTime, 1, 206, 1, 0, 0);
      v12 = 206;
    }
    Sys_QueEvent(g_wv_sysMsgTime, 1, v12, 0, 0, 0);
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
  }
  if ( Msg > 0x104 )
  {
    if ( Msg > 0x205 )
    {
      switch ( Msg )
      {
        case 0x207u:
        case 0x208u:
          goto LABEL_44;
        case 0x20Au:
          if ( SHIWORD(wParam) <= 0 )
          {
            Sys_QueEvent(g_wv_sysMsgTime, 1, 205, 1, 0, 0);
            Sys_QueEvent(g_wv_sysMsgTime, 1, 205, 0, 0, 0);
          }
          else
          {
            Sys_QueEvent(g_wv_sysMsgTime, 1, 206, 1, 0, 0);
            Sys_QueEvent(g_wv_sysMsgTime, 1, 206, 0, 0, 0);
          }
          return DefWindowProcA(hWnd, Msg, wParam, lParam);
        case 0x218u:
          if ( wParam > 1 )
            return DefWindowProcA(hWnd, Msg, wParam, lParam);
          result = 1112363332;
          break;
        default:
          return DefWindowProcA(hWnd, Msg, wParam, lParam);
      }
    }
    else
    {
      if ( Msg >= 0x204 )
      {
LABEL_44:
        v11 = (wParam & 1) != 0;
        if ( (wParam & 2) != 0 )
          v11 |= 2u;
        if ( (wParam & 0x10) != 0 )
          v11 |= 4u;
        IN_MouseEvent(v11);
        return DefWindowProcA(hWnd, Msg, wParam, lParam);
      }
      switch ( Msg )
      {
        case 0x105u:
          goto LABEL_36;
        case 0x112u:
          if ( wParam != 61760 )
            return DefWindowProcA(hWnd, Msg, wParam, lParam);
          result = 0;
          break;
        case 0x200u:
        case 0x201u:
        case 0x202u:
          goto LABEL_44;
        default:
          return DefWindowProcA(hWnd, Msg, wParam, lParam);
      }
    }
  }
  else
  {
    if ( Msg != 260 )
    {
      if ( Msg <= 0x10 )
      {
        if ( Msg == 16 )
        {
          Cbuf_AddText(a1);
        }
        else
        {
          switch ( Msg )
          {
            case 1u:
              g_wv_hWnd = hWnd;
              MSS_SetDirectSoundHWND_m(hWnd);
              vid_xpos = Cvar_Get("vid_xpos", "3", CVAR_ARCHIVE);
              vid_ypos = Cvar_Get("vid_ypos", "22", CVAR_ARCHIVE);
              r_fullscreen = Cvar_Get("r_fullscreen", "1", CVAR_ARCHIVE|CVAR_LATCH);
              s_mswheelRollMsg = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
              if ( !r_fullscreen->integer )
                goto LABEL_14;
              WIN_DisableAltTab(integer);
              break;
            case 2u:
              g_wv_hWnd = 0;
              integer = (char *)r_fullscreen->integer;
              if ( integer )
LABEL_14:
                WIN_EnableAltTab(integer);
              break;
            case 3u:
              if ( !r_fullscreen->integer )
              {
                Rect.left = 0;
                Rect.top = 0;
                Rect.right = 1;
                Rect.bottom = 1;
                WindowLongA = GetWindowLongA(hWnd, -16);
                AdjustWindowRect(&Rect, WindowLongA, 0);
                value = (float)(Rect.left + (__int16)lParam);
                Cvar_SetValue("vid_xpos", value);
                valuea = (float)(Rect.top + SHIWORD(lParam));
                Cvar_SetValue("vid_ypos", valuea);
                v8 = vid_ypos;
                vid_xpos->modified = qfalse;
                v8->modified = qfalse;
                if ( g_wv_activeApp )
                  in_appactive = 1;
              }
              break;
            case 6u:
              VID_AppActivate(HIWORD(wParam), (_WORD)wParam != 0);
              break;
            default:
              return DefWindowProcA(hWnd, Msg, wParam, lParam);
          }
        }
        return DefWindowProcA(hWnd, Msg, wParam, lParam);
      }
      if ( Msg != 256 )
      {
        if ( Msg == 257 )
        {
LABEL_36:
          v10 = MapKey(wParam, lParam);
          if ( v10 )
            Sys_QueEvent(g_wv_sysMsgTime, 1, v10, 0, 0, 0);
        }
        else if ( Msg == 258 )
        {
          Sys_QueEvent(g_wv_sysMsgTime, 2, wParam, 0, 0, 0);
        }
        return DefWindowProcA(hWnd, Msg, wParam, lParam);
      }
LABEL_29:
      v9 = MapKey(wParam, lParam);
      if ( v9 )
        Sys_QueEvent(g_wv_sysMsgTime, 1, v9, 1, 0, 0);
      return DefWindowProcA(hWnd, Msg, wParam, lParam);
    }
    if ( wParam != 13 )
      goto LABEL_29;
    if ( *(_DWORD *)cls_state != 4 )
    {
      if ( r_fullscreen )
      {
        valueb = (float)(r_fullscreen->integer == 0);
        Cvar_SetValue("r_fullscreen", valueb);
        Cbuf_AddText(a1);
      }
    }
    return 0;
  }
  return result;
}
#endif
