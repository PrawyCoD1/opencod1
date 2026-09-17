/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "cl_vm.h"
#include "cl_records.h"
#include "cl_conwindows.h"

/* CON_TEXTSIZE and DEFAULT_CONSOLE_WIDTH are RTCW cl_console.c:41 and :77;
 * MAX_EDIT_LINE is RTCW qcommon/qcommon.h:720.  CON_EMPTY_TEXT_CELL is the
 * coduo name (client/console.h:16) for the cell Con_Clear_f and Con_Linefeed
 * write: a space in colour 7. */
#define CON_TEXTSIZE            65536
#define DEFAULT_CONSOLE_WIDTH   78
#define MAX_EDIT_LINE           256
#define CON_EMPTY_TEXT_CELL     0x0720
#define CON_EMPTY_TEXT_PAIR     ( ( CON_EMPTY_TEXT_CELL << 16 ) | CON_EMPTY_TEXT_CELL )

extern int (*dword_143292C)();   /* 0x0143292C, indirect call target */
extern int (*dword_1432934)();   /* 0x01432934, indirect call target */
extern int (*dword_1432938)();   /* 0x01432938, indirect call target */
extern int (*dword_143293C)();   /* 0x0143293C, indirect call target */
extern int (*re_DrawStretchPic)();   /* 0x014328D0, indirect call target */
extern int (*re_SetColor)();   /* 0x014328CC, indirect call target */

extern cvar_t *con_boldgamemessagetime;   /* 0x0140EF2C */
extern cvar_t *con_gamemessagetime;       /* 0x0140EF30 */
extern cvar_t *con_minicontime;           /* 0x0142F5E0 */

extern cvar_t *con_restricted;

extern int com_version;   /* 0x0163A2C4, 252 bytes */
extern float con_displayFrac;   /* 0x0142EF60, 4 bytes */
extern float con_finalFrac;   /* 0x0142EF64, 4 bytes */
extern int cls_realFrametime;   /* 0x0155F3E4, 23584 bytes */
extern int whiteShader;   /* 0x015CA630, 16880 bytes */
extern cvar_t *cl_noprint;   /* 0x0161731C, 4 bytes */
extern int cls_glconfig_vidHeight;   /* 0x015CA618, 24 bytes */
extern signed __int32 cls_state;   /* 0x0155F2C0, 4 bytes */
extern int con_initialized;   /* 0x0140EF40, 4 bytes */
extern int con_prevChannel;   /* 0x0142EF50, 4 bytes */
extern int con_vislines;   /* 0x0142EF68, 100 bytes */
/* The vec4 Con_DrawInput and Con_DrawSolidConsole hand to re.SetColor: one
 * 16-byte object at 0x0142F5CC in retail.  TODO: these four are declared as
 * separate scalars and are not adjacent, so Con_OneTimeInit's four stores do
 * not fill one colour. */
extern char con_miniconColor[16];   /* 0x0142F5CC, 16 bytes */
extern int con_miniconColor_1_;   /* 0x0142F5D0, 12 bytes */
extern int con_miniconColor_2_;   /* 0x0142F5D4, 8 bytes */
extern int con_miniconColor_3_;   /* 0x0142F5D8, 4 bytes */
extern int dword_1432984;   /* 0x01432984, 60 bytes */
extern int dword_15CA634;   /* 0x015CA634, 16876 bytes */
extern float con_xadjust;   /* 0x0142EF5C, 4 bytes */
extern unsigned char con_miniconLineIndices[400];   /* 0x0142F418, 400 bytes */
extern int con_miniconlines;   /* 0x0142F5E4, 4 bytes */
extern unsigned char con_boldMessageStartTimes[96];   /* 0x0142EFF0, 96 bytes */
extern unsigned char con_boldMessageEndTimes[64];   /* 0x0142F010, 64 bytes */
extern unsigned char con_boldMessageLineIndices[32];   /* 0x0142F030, 32 bytes */
extern unsigned char con_subtitleStartTimes[96];   /* 0x0142F074, 96 bytes */
extern unsigned char con_subtitleEndTimes[64];   /* 0x0142F094, 64 bytes */
extern unsigned char con_subtitleLineIndices[32];   /* 0x0142F0B4, 32 bytes */
extern unsigned char con_miniconStartTimes[1200];   /* 0x0142F0F8, 1200 bytes */
extern unsigned char con_miniconEndTimes[800];   /* 0x0142F288, 800 bytes */
extern signed __int32 cl_serverTime;   /* 0x01434A64, 4 bytes */
extern signed __int32 con_display;   /* 0x0142EF48, 4 bytes */
extern int con_minicon;   /* 0x0140EF24, 4 bytes */
extern int dword_57C0FC;   /* 0x0057C0FC, 24 bytes */
extern int dword_57C100;   /* 0x0057C100, 20 bytes */
extern unsigned char con_gameMessageStartTimes[96];   /* 0x0142EF6C, 96 bytes */
extern unsigned char con_gameMessageEndTimes[64];   /* 0x0142EF8C, 64 bytes */
extern unsigned char con_gameMessageLineIndices[32];   /* 0x0142EFAC, 32 bytes */
extern int cls_glconfig_vidWidth;   /* 0x015CA614, 4 bytes */
extern int con_conspeed;   /* 0x0142F5DC, 4 bytes */
extern int con_debug;   /* 0x0140EF20, 4 bytes */
extern int dword_57C0F8;   /* 0x0057C0F8, 28 bytes */

extern int cls_keyCatchers;   /* 0x0155F2C4, 4 bytes */
extern signed __int32 con_current;   /* 0x0142EF44, 4 bytes */
extern signed __int32 con_displayLine;   /* 0x0142EF4C, 4 bytes */
extern signed __int32 con_linewidth;   /* 0x0142EF54, 4 bytes */
extern __int16 con_text[CON_TEXTSIZE];   /* 0x0140EF44, 131072 bytes */
extern int con_totallines;   /* 0x0142EF58, 8 bytes */
extern int dword_1430380;   /* 0x01430380, 9408 bytes */

extern void Field_Draw( void *edit, int x, int y );
extern int SCR_AdjustFrom640();
extern int SCR_DrawConsoleString( int maxChars, const float *color, const void *text, int x, int y );
extern void SCR_DrawPic( int x, int y, int w, int h, int hShader );
extern void SCR_DrawSmallChar( int ch, int x, int y );
extern void SCR_DrawSmallStringExt( const char *str, const float *color, int x, int y );
extern int SEH_PrintStrlen( char *text );
extern int SEH_SafeTranslateString();
void memset32( unsigned int *dest, unsigned int constant, unsigned int count );

int chat_playerNum;

/* ---- FastRound  0x00408370 ----  [CONFIRMED] */
int __cdecl FastRound(float a1)
{
  return (int)(a1 + 9.313225746154785e-10);
}

extern void Con_ToggleConsole_f( void );

/* ---- Con_MessageMode_f  0x00408440 ----  VERIFIED */
void __cdecl Con_MessageMode_f( void )
{
  memset( chatField.buffer, 0, MAX_EDIT_LINE );
  chatField.charWidth = 0.0f;
  chat_playerNum = -1;
  dword_1430380 = 0;
  chatField.cursor = 0;
  chatField.scroll = 0;
  chatField.widthInChars = MAX_EDIT_LINE;
  chatField.widthInPixels = 588;
  chatField.charHeight = 16.0f;
  chatField.fixedWidth = 0;
  cls_keyCatchers ^= KEYCATCH_MESSAGE;
}

/* ---- Con_MessageMode2_f  0x004084B0 ----  VERIFIED */
void __cdecl Con_MessageMode2_f( void )
{
  memset( chatField.buffer, 0, MAX_EDIT_LINE );
  chatField.cursor = 0;
  chatField.scroll = 0;
  chatField.charWidth = 0.0f;
  chatField.fixedWidth = 0;
  chat_playerNum = -1;
  dword_1430380 = 1;
  chatField.widthInChars = MAX_EDIT_LINE;
  chatField.widthInPixels = 543;
  chatField.charHeight = 16.0f;
  cls_keyCatchers ^= KEYCATCH_MESSAGE;
}

/* ---- Con_MessageMode3_f  0x00408520 ----  VERIFIED */
void __cdecl Con_MessageMode3_f( void )
{
  chat_playerNum = VM_Call( cgvm, 4 );
  if ( chat_playerNum < 0 || chat_playerNum >= 64 )
  {
    chat_playerNum = -1;
    return;
  }
  memset( chatField.buffer, 0, MAX_EDIT_LINE );
  chatField.charWidth = 0.0f;
  dword_1430380 = 0;
  chatField.cursor = 0;
  chatField.scroll = 0;
  chatField.widthInChars = MAX_EDIT_LINE;
  chatField.widthInPixels = 588;
  chatField.charHeight = 16.0f;
  chatField.fixedWidth = 0;
  cls_keyCatchers ^= KEYCATCH_MESSAGE;
}

/* ---- Con_Clear_f  0x004085B0 ----  [CONFIRMED] */
void __cdecl Con_Clear_f()
{
  memset32( (unsigned int *)con_text, CON_EMPTY_TEXT_PAIR, CON_TEXTSIZE / 2 );
  con_displayLine = con_current;
}

/* ---- Con_Dump_f  0x004085D0 ----  VERIFIED */
void __cdecl Con_Dump_f( void )
{
  signed __int32 l;
  signed __int32 x;
  signed __int32 i;
  __int16 *line;
  fileHandle_t f;
  char buffer[1024];

  if ( Cmd_Argc() != 2 )
  {
    Com_Printf( "usage: condump <filename>\n" );
    return;
  }

  Com_Printf( "Dumped console text to %s.\n", Cmd_Argv( 1 ) );

  f = FS_FOpenFileWrite( Cmd_Argv( 1 ) );
  if ( !f )
  {
    Com_Printf( "ERROR: couldn't open.\n" );
    return;
  }

  for ( l = con_current - con_totallines + 1; l <= con_current; l++ )
  {
    line = con_text + ( l % con_totallines ) * con_linewidth;
    for ( x = 0; x < con_linewidth; x++ )
    {
      if ( ( line[x] & 0xff ) != ' ' )
        break;
    }
    if ( x != con_linewidth )
      break;
  }

  buffer[con_linewidth] = 0;
  for ( ; l <= con_current; l++ )
  {
    line = con_text + ( l % con_totallines ) * con_linewidth;
    for ( i = 0; i < con_linewidth; i++ )
      buffer[i] = (char)line[i];
    for ( x = con_linewidth - 1; x >= 0; x-- )
    {
      if ( buffer[x] == ' ' )
        buffer[x] = 0;
      else
        break;
    }
    strcat( buffer, "\n" );
    FS_Write( buffer, strlen( buffer ), f );
  }

  FS_FCloseFile( f );
}

/* ---- Con_ClearMessageWindow  0x00408770 ----  [CONFIRMED] */
void __fastcall Con_ClearMessageWindow(int unused, void *window)
{
  memset(*(void **)window, 0, 4 * *((_DWORD *)window + 4));
  memset(*((void **)window + 1), 0, 4 * *((_DWORD *)window + 4));
  *((_DWORD *)window + 3) = 0;
}

/* ---- Con_ClearNotify  0x004087B0 ----  [CONFIRMED] */
void __cdecl Con_ClearNotify()
{
  memset((void *)con_notifyWindow, 0, 4 * con_gamemessagelines);
  memset((void *)dword_142EFD0, 0, 4 * con_gamemessagelines);
  dword_142EFD8 = 0;
  memset((void *)con_boldWindow, 0, 4 * dword_142F060);
  memset((void *)dword_142F054, 0, 4 * dword_142F060);
  dword_142F05C = 0;
}

/* ---- Con_ClearMiniConsole  0x00408800 ----  [CONFIRMED] */
void __cdecl Con_ClearMiniConsole()
{
  memset((void *)con_miniconsoleWindow, 0, 4 * dword_142F5B8);
  memset((void *)dword_142F5AC, 0, 4 * dword_142F5B8);
  dword_142F5B4 = 0;
}

/* ---- Con_ClearSubtitles  0x00408830 ----  [CONFIRMED] */
void __cdecl Con_ClearSubtitles()
{
  memset((void *)con_subtitleWindow, 0, 4 * dword_142F0E4);
  memset((void *)dword_142F0D8, 0, 4 * dword_142F0E4);
  dword_142F0E0 = 0;
}

/* ---- Con_CheckResize  0x00408860 ----  VERIFIED */
void __cdecl Con_CheckResize( void )
{
  signed __int32 i, j, width, oldwidth, oldtotallines, numlines, numchars;
  __int16 tbuf[CON_TEXTSIZE];
  int vidWidth;

  vidWidth = cls_glconfig_vidWidth;
  if ( vidWidth < 640 )
    vidWidth = 640;

  width = vidWidth / 8 - 2;

  if ( width == con_linewidth )
    return;

  if ( width < 1 )
  {
    con_linewidth = DEFAULT_CONSOLE_WIDTH;
    con_totallines = CON_TEXTSIZE / DEFAULT_CONSOLE_WIDTH;
    memset32( (unsigned int *)con_text, CON_EMPTY_TEXT_PAIR, CON_TEXTSIZE / 2 );
  }
  else
  {
    oldwidth = con_linewidth;
    con_linewidth = width;
    oldtotallines = con_totallines;
    con_totallines = CON_TEXTSIZE / con_linewidth;
    numlines = oldtotallines;
    if ( con_totallines < numlines )
      numlines = con_totallines;

    numchars = oldwidth;
    if ( con_linewidth < numchars )
      numchars = con_linewidth;

    qmemcpy( tbuf, con_text, sizeof( tbuf ) );
    memset32( (unsigned int *)con_text, CON_EMPTY_TEXT_PAIR, CON_TEXTSIZE / 2 );

    for ( i = 0; i < numlines; i++ )
    {
      for ( j = 0; j < numchars; j++ )
      {
        con_text[( con_totallines - 1 - i ) * con_linewidth + j] =
            tbuf[( ( con_current - i + oldtotallines ) % oldtotallines ) * oldwidth + j];
      }
    }

    Con_ClearNotify();
    Con_ClearMiniConsole();
    Con_ClearSubtitles();
  }

  con_current = con_totallines - 1;
  con_displayLine = con_current;
}

/* ---- Con_InitMessageWindow  0x00408A50 ----  [CONFIRMED] */
void __cdecl Con_InitMessageWindow(
        void *window,
        int x,
        int *timesPtr,
        int *endTimesPtr,
        int *linesPtr,
        int size,
        int y,
        int msgtime,
        int extra)
{
  *(_DWORD *)window = timesPtr;
  *((_DWORD *)window + 1) = x;
  *((_DWORD *)window + 2) = endTimesPtr;
  *((_DWORD *)window + 4) = linesPtr;
  *((_DWORD *)window + 5) = size;
  *((_DWORD *)window + 6) = y;
  *((_DWORD *)window + 3) = 0;
  *((_DWORD *)window + 7) = msgtime;
  *((_DWORD *)window + 8) = extra;
}

/* ---- Con_Init  0x00408A90 ----  [CONFIRMED] */
void __cdecl Con_Init()
{
  cvar_t *v0;
  int v1;

  con_conspeed = (int)Cvar_Get("scr_conspeed", "3", 0);
  con_debug = (int)Cvar_Get("con_debug", "0", 1);
  v0 = Cvar_Get("con_restricted", "0", 16);
  v1 = dword_57C0F8;
  con_restricted = v0;
  memset(g_consoleField.buffer, 0, MAX_EDIT_LINE);
  g_consoleField.cursor = 0;
  g_consoleField.scroll = 0;
  g_consoleField.widthInChars = MAX_EDIT_LINE;
  g_consoleField.widthInPixels = dword_57C0F8;
  *(int *)&g_consoleField.charWidth = dword_57C0FC;
  *(int *)&g_consoleField.charHeight = dword_57C100;
  g_consoleField.fixedWidth = 1;
  {
    int i;
    for ( i = 0; i < COMMAND_HISTORY; ++i )
    {
      field_t *f = &historyEditLines[i];

      memset( f->buffer, 0, MAX_EDIT_LINE );
      f->cursor        = 0;
      f->scroll        = 0;
      f->widthInChars  = MAX_EDIT_LINE;
      f->widthInPixels = v1;
      *(int *)&f->charWidth  = dword_57C0FC;
      *(int *)&f->charHeight = dword_57C100;
      f->fixedWidth    = 1;
    }
  }
  Cmd_AddCommand("toggleconsole", Con_ToggleConsole_f);
  Cmd_AddCommand("messagemode", Con_MessageMode_f);
  Cmd_AddCommand("messagemode2", Con_MessageMode2_f);
  Cmd_AddCommand("messagemode3", Con_MessageMode3_f);
  Cmd_AddCommand("clear", Con_Clear_f);
  Cmd_AddCommand("condump", Con_Dump_f);
}

/* ---- Con_UpdateMessageWindowLine  0x00408BE0 ----  VERIFIED */
void __cdecl Con_UpdateMessageWindowLine( int *window, int extra, int duration )
{
  signed __int32 i;
  int j;
  int fadeOut;

  *(int *)( window[0] + 4 * window[3] ) = cl_serverTime;
  *(int *)( window[1] + 4 * window[3] ) = cl_serverTime + duration;
  *(int *)( window[2] + 4 * window[3] ) = con_current;

  if ( !extra )
    return;
  if ( window[4] <= 0 )
    return;

  window[3] = ( window[3] + 1 ) % window[4];

  for ( i = 0; i < window[5]; i++ )
  {
    j = ( window[3] + i ) % window[4];
    fadeOut = window[8];
    if ( *(int *)( window[1] + 4 * j ) - fadeOut > cl_serverTime )
    {
      *(int *)( window[0] + 4 * j ) +=
          cl_serverTime + fadeOut - *(int *)( window[1] + 4 * j );
      *(int *)( window[1] + 4 * j ) = cl_serverTime + window[8];
    }
  }
}

/* ---- Con_UpdateNotifyLine  0x00408C90 ----  VERIFIED */
void __cdecl Con_UpdateNotifyLine( int duration, int channel, int extra )
{
  if ( con_current < 0 )
    return;

  switch ( channel )
  {
    case 0:
      Con_UpdateMessageWindowLine( (int *)&con_miniconsoleWindow, extra, duration );
      break;
    case 1:
      Con_UpdateMessageWindowLine( (int *)&con_notifyWindow, extra, duration );
      break;
    case 2:
      Con_UpdateMessageWindowLine( (int *)&con_boldWindow, extra, duration );
      break;
    case 3:
      Con_UpdateMessageWindowLine( (int *)&con_subtitleWindow, extra, duration );
      break;
    default:
      return;
  }
}

/* ---- Con_Linefeed  0x00408D00 ----  VERIFIED */
void __cdecl Con_Linefeed( int duration, int channel )
{
  signed __int32 i;

  Con_UpdateNotifyLine( duration, channel, 1 );

  con_display = 0;
  if ( con_displayLine == con_current )
    con_displayLine++;
  con_current++;

  for ( i = 0; i < con_linewidth; i++ )
    con_text[( con_current % con_totallines ) * con_linewidth + i] = CON_EMPTY_TEXT_CELL;
}

conMessageWindow_t con_notifyWindowRec;
conMessageWindow_t con_boldWindowRec;
conMessageWindow_t con_subtitleWindowRec;
conMessageWindow_t con_miniconsoleWindowRec;

/* ---- Con_OneTimeInit  0x00408D70 ----  [CONFIRMED] */
void __cdecl Con_OneTimeInit(int extra)
{
  cvar_t *v1;
  int integer;
  char *v3;
  int v4;

  con_gamemessagetime = Cvar_Get("con_gamemessagetime", "5", 0);
  con_boldgamemessagetime = Cvar_Get("con_boldgamemessagetime", "8", 0);
  con_minicontime = Cvar_Get("con_minicontime", "4", 1);
  con_minicon = (int)Cvar_Get("con_minicon", "0", 1);
  v1 = Cvar_Get("con_miniconlines", "5", 1);
  con_notifyWindow = (int)&con_gameMessageStartTimes;
  dword_142EFD0 = (int)&con_gameMessageEndTimes;
  dword_142EFD4 = (int)&con_gameMessageLineIndices;
  dword_142EFD8 = 0;
  con_gamemessagelines = 8;
  dword_142EFE0 = 3;
  dword_142EFE4 = 250;
  dword_142EFE8 = 250;
  dword_142EFEC = 500;
  con_boldWindow = (int)&con_boldMessageStartTimes;
  dword_142F054 = (int)&con_boldMessageEndTimes;
  dword_142F058 = (int)&con_boldMessageLineIndices;
  dword_142F05C = 0;
  dword_142F060 = 8;
  dword_142F064 = 3;
  dword_142F068 = 250;
  dword_142F06C = 250;
  dword_142F070 = 500;
  con_subtitleWindow = (int)&con_subtitleStartTimes;
  dword_142F0D8 = (int)&con_subtitleEndTimes;
  dword_142F0DC = (int)&con_subtitleLineIndices;
  dword_142F0E0 = 0;
  dword_142F0E4 = 8;
  dword_142F0E8 = 3;
  dword_142F0EC = 250;
  dword_142F0F0 = 250;
  dword_142F0F4 = 500;
  integer = v1->integer;
  con_miniconlines = (int)v1;
  if ( integer > 100 )
  {
    v3 = va("%d", 100);
    Cvar_Set2("con_miniconlines", v3, qtrue);
    v1 = (cvar_t *)con_miniconlines;
  }
  v4 = v1->integer;
  con_miniconsoleWindow = (int)&con_miniconStartTimes;
  dword_142F5AC = (int)&con_miniconEndTimes;
  dword_142F5B0 = (int)&con_miniconLineIndices;
  dword_142F5B4 = 0;
  dword_142F5B8 = v4;
  dword_142F5BC = 0;
  dword_142F5C0 = 0;
  dword_142F5C4 = 0;
  dword_142F5C8 = 0;
  con_miniconColor_3_ = 1065353216;
  con_miniconColor_2_ = 1065353216;
  con_miniconColor_1_ = 1065353216;
  *(_DWORD *)con_miniconColor = 1065353216;
  con_linewidth = -1;
  Con_CheckResize();
  con_initialized = qtrue;
}

/* ---- CL_ConsolePrint_AddLine  0x00408F80 ----  VERIFIED */
int __cdecl CL_ConsolePrint_AddLine(
        byte *txt,
        int channel,
        int duration,
        int width,
        int color )
{
  const byte *s;
  signed __int32 l;
  signed __int32 softWidth;
  signed __int32 len;
  signed __int32 lines;
  int wrapped;
  int c;

  if ( width <= 0 || width > con_linewidth )
    width = con_linewidth;

  softWidth = width;

  if ( channel == 1 || channel == 2 )
  {
    len = SEH_PrintStrlen( (char *)txt );
    if ( len > width )
    {
      lines = (signed __int32)ceil( (double)(float)len / (double)width );
      softWidth = (signed __int32)( (double)(float)len / (double)lines );
    }
  }

  if ( channel != con_prevChannel && con_display > 0 )
    Con_Linefeed( duration, con_prevChannel );

  wrapped = 0;
  s = txt;

  while ( ( c = *s ) != 0 )
  {
    if ( *s == '^' && s[1] != 0 && s[1] != '^' && s[1] >= '0' && s[1] <= '7' )
    {
      color = s[1] & 7;
      s += 2;
      continue;
    }

    for ( l = 0; l < width; l++ )
    {
      if ( s[l] <= ' ' )
        break;
    }

    if ( l != width && con_display + l > width )
    {
      Con_Linefeed( duration, channel );
      wrapped = 1;
    }

    s++;

    if ( c == '\n' )
    {
      Con_Linefeed( duration, channel );
    }
    else if ( c == '\r' )
    {
      con_display = 0;
    }
    else if ( con_display != 0 || c != ' ' || !wrapped )
    {
      con_text[( con_current % con_totallines ) * con_linewidth + con_display] =
          (__int16)( ( color << 8 ) | c );
      con_display++;
      if ( con_display >= width || ( con_display >= softWidth && c == ' ' ) )
      {
        Con_Linefeed( duration, channel );
        wrapped = 1;
      }
    }
  }

  if ( con_display <= 0 )
  {
    con_prevChannel = channel;
  }
  else if ( channel )
  {
    Con_Linefeed( duration, channel );
    con_prevChannel = channel;
  }
  else
  {
    Con_UpdateNotifyLine( duration, 0, 0 );
    con_prevChannel = 0;
  }

  return color;
}

/* CL_ConsolePrint (0x00409190) is defined further down this unit. */
#if 0
void __cdecl CL_ConsolePrint(int channel, const char *txt)
{
  char *v2;
  int v3;
  char *v4;
  bool v5; // sf
  double v6;
  char *v7;
  unsigned int v8;
  int v9;
  int v10;
  float v11;
  int v12;
  int v13;
  int v14;
  int defaultColor;
  char Destination[4100]; // [esp+14h] [ebp-100Ch] BYREF
  unsigned int v17;
  unsigned int retaddr;

  v17 = retaddr ^ _security_cookie;
  v4 = v2;
  if ( (!cl_noprint || !cl_noprint->integer) && v3 != 4 )
  {
    if ( con_initialized == qfalse )
      Con_OneTimeInit(v13);
    v5 = channel < 0;
    if ( !channel )
    {
      switch ( v3 )
      {
        case 0:
          v6 = *(float *)(con_minicontime + 28);
          goto LABEL_11;
        case 1:
          v6 = *(float *)(con_gamemessagetime + 28);
          goto LABEL_11;
        case 2:
          v6 = *(float *)(con_boldgamemessagetime + 28);
LABEL_11:
          v11 = v6 * 1000.0;
          channel = sub_408370(v11);
          v5 = channel < 0;
          break;
        case 3:
          channel = 5000;
          goto LABEL_14;
        default:
          goto LABEL_14;
      }
    }
    if ( v5 )
      channel = 0;
LABEL_14:
    defaultColor = 7;
    if ( v3 == 1 || v3 == 2 )
    {
      if ( strstr(v4, "\n") )
      {
        v7 = strstr(v4, "\n");
        while ( 1 )
        {
          if ( *v4 == 10 )
          {
            Con_Linefeed(v13, v14);
            ++v4;
          }
          else
          {
            v8 = v7 - v4 + 1;
            if ( v8 >= 0x1000 )
            {
              Com_Printf("Text line too long. Clipping to fit\n");
              v8 = 4096;
            }
            strncpy(Destination, v4, v8 - 1);
            v9 = defaultColor;
            v12 = defaultColor;
            Destination[v8 - 1] = 0;
            CL_ConsolePrint_AddLine((byte *)Destination, v9, v3, channel, (int)txt, v12);
            defaultColor = v10;
            v4 = v7;
            if ( *v7 == 10 )
              v4 = v7 + 1;
          }
          if ( !strstr(v4, "\n") )
            break;
          if ( v4 )
          {
            if ( *v4 )
            {
              v7 = strstr(v4, "\n");
              if ( v7 )
                continue;
            }
          }
          return;
        }
        CL_ConsolePrint_AddLine((byte *)v4, defaultColor, v3, channel, (int)txt, defaultColor);
      }
      else
      {
        CL_ConsolePrint_AddLine((byte *)v4, (int)txt, v3, channel, (int)txt, 7);
      }
    }
    else
    {
      CL_ConsolePrint_AddLine((byte *)v4, channel, v3, channel, (int)txt, 7);
    }
  }
}
#endif

/* CL_ConsoleFixPosition (0x004093B0) is defined further down this unit. */
#if 0
void __fastcall CL_ConsoleFixPosition(int unused)
{
  int v1;
  int v2;
  float v3;
  unsigned int retaddr;

  if ( cl_noprint && cl_noprint->integer )
  {
    con_displayLine = con_current - 1;
  }
  else
  {
    if ( con_initialized == qfalse )
      Con_OneTimeInit(retaddr ^ _security_cookie);
    v3 = *(float *)(con_minicontime + 28) * 1000.0;
    v1 = sub_408370(v3);
    if ( v1 < 0 )
      v1 = 0;
    CL_ConsolePrint_AddLine((byte *)"\n", v2, 0, v1, 0, 7);
    con_displayLine = con_current - 1;
  }
}
#endif

/* ---- CL_AddConsoleInfoChar  0x00409450 ----  VERIFIED */
void __cdecl CL_AddConsoleInfoChar( int ch, char color )
{
  con_text[( con_current % con_totallines ) * con_linewidth + con_display] =
      (__int16)( ( color << 8 ) | ch );
  con_display++;
}

/* ---- CL_AddConsoleInfoColor  0x00409490 ----  VERIFIED */
void __cdecl CL_AddConsoleInfoColor( int base, const float *rgb )
{
  int i;
  int v;

  for ( i = 0; i < 3; i++ )
  {
    v = (int)( rgb[i] * 255.0 );
    if ( v < 0 )
      v = 0;
    else if ( v > 255 )
      v = 255;

    con_text[( con_current % con_totallines ) * con_linewidth + con_display] =
        (__int16)( ( ( base + i ) << 8 ) | v );
    con_display++;
  }
}

/* ---- CL_AddDeathMessageText  0x00409590 ----  VERIFIED */
void __cdecl CL_AddDeathMessageText( const char *txt, int color )
{
  signed __int32 row;
  int c;
  char col;

  /* `movsx ecx, al` at 0x004095B3 sign-extends first, so a byte >= 0x80 leaves 0xFF in the colour half. Retail quirk; keep it signed. */
  col = (char)color;
  if ( color < 0 )
    col = 7;

  row = con_current % con_totallines;

  while ( *txt )
  {
    if ( *txt == '^' && txt[1] != 0 && txt[1] != '^' && txt[1] >= '0' && txt[1] <= '7' )
    {
      if ( color < 0 )
        col = (char)( txt[1] & 7 );
      txt += 2;
    }
    else
    {
      c = *txt;
      txt++;
      if ( c != '\n' && c != '\r' )
      {
        con_text[row * con_linewidth + con_display] = (__int16)( ( col << 8 ) | c );
        con_display++;
      }
    }
  }
}

/* ---- CL_DeathMessagePrint  0x00409630 ----  VERIFIED */
void __cdecl CL_DeathMessagePrint(
        const char *attacker,
        int channel,
        const float *attackerColor,
        const char *victim,
        const float *victimColor,
        const char *iconName,
        float iconWidth,
        float iconHeight,
        const float *iconColor,
        int duration )
{
  float f;

  if ( cl_noprint && cl_noprint->integer )
    return;
  if ( channel == 4 )
    return;

  if ( con_initialized == qfalse )
    Con_OneTimeInit( 0 );

  if ( !duration )
  {
    switch ( channel )
    {
      case 0:
        f = *(float *)( con_minicontime + 28 ) * 1000.0f;
        duration = FastRound( f );
        break;
      case 1:
        f = *(float *)( con_gamemessagetime + 28 ) * 1000.0f;
        duration = FastRound( f );
        break;
      case 2:
        f = *(float *)( con_boldgamemessagetime + 28 ) * 1000.0f;
        duration = FastRound( f );
        break;
      case 3:
        duration = 5000;
        break;
      default:
        break;
    }
  }
  if ( duration < 0 )
    duration = 0;

  if ( con_display > 0 )
    Con_Linefeed( duration, con_prevChannel );

  if ( *attacker )
  {
    CL_AddConsoleInfoColor( 10, attackerColor );
    CL_AddDeathMessageText( attacker, 7 );
    CL_AddDeathMessageText( " ", 7 );
  }

  CL_AddConsoleInfoColor( 13, iconColor );

  con_text[( con_current % con_totallines ) * con_linewidth + con_display] =
      (__int16)( (unsigned __int16)(int)( iconWidth * 32.0 ) | 0x1000 );
  con_display++;
  con_text[( con_current % con_totallines ) * con_linewidth + con_display] =
      (__int16)( (unsigned __int16)(int)( iconHeight * 32.0 ) | 0x1100 );
  con_display++;

  CL_AddDeathMessageText( iconName, 18 );
  CL_AddDeathMessageText( " ", 7 );

  CL_AddConsoleInfoColor( 10, victimColor );
  CL_AddDeathMessageText( victim, 7 );

  if ( channel )
  {
    Con_Linefeed( duration, channel );
    con_prevChannel = channel;
  }
  else
  {
    Con_UpdateNotifyLine( duration, 0, 0 );
    con_prevChannel = channel;
  }
}

/* ---- Con_DrawInput  0x00409810 ----  [CONFIRMED] */
void __cdecl Con_DrawInput()
{
  int v1;

  if ( !cls_state || (cls_keyCatchers & KEYCATCH_CONSOLE) != 0 )
  {
    v1 = con_vislines - 32;
    re_SetColor(con_miniconColor);
    SCR_DrawSmallChar(']', (int)(unsigned __int64)con_xadjust, v1);
    Field_Draw(&g_consoleField, (int)(unsigned __int64)con_xadjust + 8, v1);
  }
}

/* ---- Con_DrawStringOnHUD  0x00409870 ----  [HIGH] */
int __cdecl Con_DrawStringOnHUD(int len, int charset, int x, int y, int color, int center)
{
  int v6;
  float v8;
  float v9;
  int v10;
  _DWORD v11[4];

  v11[3] = color;
  v6 = 0;
  v11[0] = 1065353216;
  v11[1] = 1065353216;
  v11[2] = 1065353216;
  v10 = 1048576000;
  if ( center )
  {
    v6 = 4;
    v10 = 1051372203;
    x += dword_1432938(len, 4, 1051372203, 0, charset) / -2;
  }
  v9 = (float)(y + 12);
  v8 = (float)x;
  return dword_143293C(LODWORD(v8), LODWORD(v9), v6, v10, v11, len, 0, charset, 3);
}

/* ---- Con_DrawMessageWindowBottomUp  0x00409920 ----  [HIGH] */
void __cdecl Con_DrawMessageWindowBottomUp(int *window, int charset, int yStart, float alpha, int isSubtitle)
{
  int *v4 = window;
  int v5;
  int v6;
  signed __int32 *v7;
  signed __int32 v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int color;
  float colorb;
  int colora;
  int v18;
  int v19;

  v5 = v4[4];
  v6 = v4[3];
  v18 = 4 * (isSubtitle != 0) + 12;
  if ( v6 < v5 + v6 )
  {
    do
    {
      v7 = (signed __int32 *)(*v4 + 4 * (v6 % v5));
      v8 = *v7;
      if ( *v7 )
      {
        if ( v8 <= cl_serverTime )
        {
          color = v4[6];
          if ( cl_serverTime - v8 < color )
          {
            colorb = (1.0 - (double)(cl_serverTime - v8) / (double)color) * (double)v18;
            yStart += (int)(colorb + 9.313225746154785e-10);
          }
        }
        else
        {
          *v7 = 0;
        }
      }
      v5 = v4[4];
      ++v6;
    }
    while ( v6 < v5 + v4[3] );
  }
  v9 = v4[3];
  v10 = v9 + v4[4] - 1;
  if ( v10 >= v9 )
  {
    do
    {
      v11 = v10 % v4[4];
      v12 = *(int *)(*v4 + 4 * v11);
      if ( v12 )
      {
        v13 = *(int *)(v4[1] + 4 * v11);
        if ( cl_serverTime - v13 < 0 )
        {
          v19 = v4[7];
          if ( cl_serverTime - v12 >= v19 )
          {
            v14 = v4[8];
            if ( v13 - cl_serverTime >= v14 )
              *(float *)&colora = alpha;
            else
              *(float *)&colora = (double)(v13 - cl_serverTime) * alpha / (double)v14;
          }
          else
          {
            *(float *)&colora = (double)(cl_serverTime - v12) * alpha / (double)v19;
          }
          yStart -= v18;
          Con_DrawStringOnHUD(
            (int)&con_text[con_linewidth * (*(int *)(v4[2] + 4 * v11) % con_totallines)],
            con_linewidth,
            charset,
            yStart,
            colora,
            isSubtitle);
        }
        else
        {
          *(int *)(*v4 + 4 * v11) = 0;
        }
      }
      --v10;
    }
    while ( v10 >= v4[3] );
  }
}

/* ---- Con_DrawMessageWindow  0x00409AD0 ----  [HIGH] */
void __cdecl Con_DrawMessageWindow(
        int yStart,
        int *window,
        int orientation,
        int charset,
        float alpha)
{
  int v6;
  int v7;
  int v8;
  int v9;
  signed __int32 v10;
  signed __int32 v11;
  double v12;
  int v13;
  int v14;
  int v15;
  int v16;
  float v17;
  int y;
  int v19;
  int v20;
  int v21;
  int color;
  int v23;
  int v24;
  int v25;
  int v26;
  int v27;
  int v28;
  int len;
  signed __int32 lena;

  v27 = 0;
  if ( dword_1432984 == 5 || (cls_keyCatchers & (KEYCATCH_UI | KEYCATCH_CGAME)) == 0 )
  {
    switch ( orientation )
    {
      case 0:
        v20 = 0;
        goto LABEL_6;
      case 1:
        v20 = 1;
LABEL_6:
        v19 = 0;
        y = yStart;
        if ( v20 )
          y = yStart - 12;
        v6 = window[3];
        v7 = window[4];
        v23 = v6;
        v24 = v7;
        v26 = v7 + v6;
        if ( v6 >= v7 + v6 )
          return;
        v21 = v6 + 1;
        break;
      case 2:
      case 3:
        Con_DrawMessageWindowBottomUp(window, charset, yStart, alpha, orientation == 3);
        return;
      default:
        return;
    }
    while ( 1 )
    {
      v8 = *window;
      v9 = v6 % v7;
      v10 = *(int *)(*window + 4 * v9);
      if ( !v10 )
        goto LABEL_40;
      if ( v10 > cl_serverTime )
      {
        *(_DWORD *)(v8 + 4 * v9) = 0;
        goto LABEL_40;
      }
      v11 = *(int *)(window[1] + 4 * v9);
      if ( cl_serverTime > v11 )
      {
        v25 = window[6];
        if ( v11 + v25 - cl_serverTime > 0 )
        {
          v17 = (double)(v11 + v25 - cl_serverTime) / (double)v25 * 12.0;
          if ( v20 )
            y -= FastRound(v17);
          else
            y += FastRound(v17);
        }
        goto LABEL_40;
      }
      *(float *)&color = alpha;
      len = window[7];
      if ( cl_serverTime - v10 >= len )
      {
        v28 = window[8];
        if ( v11 - cl_serverTime >= v28 )
          goto LABEL_24;
        v12 = (double)(v11 - cl_serverTime) / (double)v28;
      }
      else
      {
        v12 = (double)(cl_serverTime - v10) / (double)len;
      }
      *(float *)&color = v12 * alpha;
LABEL_24:
      /* absolute retail VA 0x0140EF44 relocated onto con_text */
      lena = (int)&con_text[con_linewidth * (*(int *)(window[2] + 4 * v9) % con_totallines)];
      if ( v20 )
      {
        if ( v19 )
        {
          v15 = y + 24;
        }
        else
        {
          v13 = v21;
          v14 = *(_DWORD *)(v8 + 4 * v9);
          if ( v21 >= v26 )
            goto LABEL_33;
          do
          {
            if ( *(_DWORD *)(v8 + 4 * (v13 % v24)) != v14 )
              break;
            ++v13;
            ++v19;
          }
          while ( v13 < v26 );
          if ( !v19 )
            goto LABEL_33;
          v15 = -12 * v19 + y;
          v27 = v15;
          ++v19;
        }
        y = v15;
      }
LABEL_33:
      Con_DrawStringOnHUD(lena, con_linewidth, charset, y, color, 0);
      if ( v20 )
      {
        if ( v19 )
        {
          if ( !--v19 )
            y = v27;
        }
        v16 = y - 12;
      }
      else
      {
        v16 = y + 12;
      }
      v6 = v23;
      y = v16;
LABEL_40:
      ++v6;
      v24 = window[4];
      v23 = v6;
      ++v21;
      v26 = window[3] + v24;
      if ( v6 >= v26 )
        return;
      v7 = window[4];
    }
  }
}

/* ---- Con_DrawSay  0x00409D80 ----  [CONFIRMED] */
void __cdecl Con_DrawSay(int y)
{
  int *v1;
  char *v2;
  int v3;
  float v4;
  _DWORD v5[4];

  v5[0] = 1065353216;
  v5[1] = 1065353216;
  v5[2] = 1065353216;
  v5[3] = 1065353216;
  if ( (cls_keyCatchers & KEYCATCH_MESSAGE) != 0 )
  {
    if ( dword_1430380 )
      v1 = SEH_SafeTranslateString((int *)"EXE_SAYTEAM");
    else
      v1 = SEH_SafeTranslateString((int *)"EXE_SAY");
    v2 = va("%s:", (const char *)v1);
    v4 = (float)(y + 16);
    dword_1432934(1090519040, LODWORD(v4), 0, 1051372203, v5, v2, 0, 0, 3);
    v3 = dword_143292C(v2, 0, 1051372203, 0, 0);
    Field_Draw(&chatField, v3 + 8, y);
  }
}

/* ---- Con_DrawNotify  0x00409E70 ----  [CONFIRMED] */
void __cdecl Con_DrawNotify(int charset, float alpha, int orientation, int yStart)
{
  Con_DrawMessageWindow(charset, &con_notifyWindow, orientation, yStart, alpha);
}

/* ---- Con_DrawBoldMessages  0x00409E90 ----  [CONFIRMED] */
void __cdecl Con_DrawBoldMessages(int charset, float alpha, int orientation, int yStart)
{
  Con_DrawMessageWindow(charset, &con_boldWindow, orientation, yStart, alpha);
}

/* ---- Con_DrawMiniConsole  0x00409EB0 ----  [CONFIRMED] */
void __cdecl Con_DrawMiniConsole(int yStart, int charset, float alpha)
{
  char *v3;

  if ( *(int *)(con_miniconlines + 32) > 100 )
  {
    v3 = va("%d", 100);
    Cvar_Set2("con_miniconlines", v3, qtrue);
  }
  dword_142F5B8 = *(_DWORD *)(con_miniconlines + 32);
  Con_DrawMessageWindow(charset, &con_miniconsoleWindow, 0, yStart, alpha);
}

/* ---- Con_DrawSubtitles  0x00409F10 ----  [CONFIRMED] */
void __cdecl Con_DrawSubtitles(int charset, float alpha, int orientation, int yStart)
{
  Con_DrawMessageWindow(charset, &con_subtitleWindow, orientation, yStart, alpha);
}

/* ---- Con_DrawSolidConsole  0x00409F30 ----  [HIGH] */
void __cdecl Con_DrawSolidConsole(float frac)
{
  int v3;
  int v4;
  int v5;
  signed __int32 i;
  signed __int32 v7;
  signed __int32 v8;
  int v9;
  int v10;
  int v15;
  int v16;
  int v17;
  _DWORD v18[4];

  v3 = (unsigned __int64)((double)cls_glconfig_vidHeight * frac);
  if ( v3 > 0 )
  {
    if ( v3 > cls_glconfig_vidHeight )
      v3 = cls_glconfig_vidHeight;
    con_xadjust = (double)cls_glconfig_vidWidth * 0.0015625 * 8.0;
    LODWORD(frac) = (unsigned __int64)(frac * 480.0 - 2.0);
    if ( SLODWORD(frac) >= 1 )
    {
      *(float *)&v10 = (float)SLODWORD(frac);
      SCR_DrawPic(0, 0, 1142947840, v10, dword_15CA634);
    }
    else
    {
      frac = 0.0;
    }
    frac = (float)SLODWORD(frac);
    memset(v18, 0, 12);
    v18[3] = 1058642330;
    v15 = 0x40000000;
    v16 = 1142947840;
    v17 = 0;
    re_SetColor(v18);
    SCR_AdjustFrom640((float *)&v17, &frac, (float *)&v16, (float *)&v15);
    re_DrawStretchPic(v17, LODWORD(frac), v16, v15, 0, 0, 0, 0, whiteShader);
    re_SetColor(0);
    SCR_DrawSmallStringExt(*(const char **)(com_version + 4), g_color_table[7],
                           cls_glconfig_vidWidth - 8 * (int)strlen(*(const char **)(com_version + 4)), v3 - 18);
    con_vislines = v3;
    v4 = (v3 - 8) / 8;
    v5 = v3 - 48;
    if ( con_displayLine != con_current )
    {
      re_SetColor(g_color_table[7]);
      for ( i = 0; i < con_linewidth; i += 4 )
        SCR_DrawSmallChar('^', (int)(unsigned __int64)con_xadjust + 8 * i + 8, v5);
      v5 -= 16;
      --v4;
    }
    v7 = con_displayLine;
    if ( !con_display )
      v7 = con_displayLine - 1;
    frac = 0.0;
    if ( v4 > 0 )
    {
      v8 = con_totallines;
      do
      {
        if ( v7 < 0 )
          break;
        if ( con_current - v7 < v8 )
        {
          SCR_DrawConsoleString(
            con_linewidth,
            g_color_table[7],
            &con_text[con_linewidth * (v7 % v8)],
            (int)(unsigned __int64)con_xadjust,
            v5);
          v8 = con_totallines;
        }
        v5 -= 16;
        --v7;
        ++LODWORD(frac);
      }
      while ( SLODWORD(frac) < v4 );
    }
    if ( !cls_state || (cls_keyCatchers & KEYCATCH_CONSOLE) != 0 )
    {
      v9 = con_vislines - 32;
      re_SetColor(con_miniconColor);
      SCR_DrawSmallChar(']', (int)(unsigned __int64)con_xadjust, v9);
      Field_Draw(&g_consoleField, (int)(unsigned __int64)con_xadjust + 8, v9);
    }
    re_SetColor(0);
  }
}

/* ---- Con_DrawConsole  0x0040A1E0 ----  [CONFIRMED] */
void __cdecl Con_DrawConsole()
{
  float frac;

  Con_CheckResize();
  if ( cls_state )
  {
    if ( cls_state == CA_ACTIVE )
    {
      if ( (con_displayFrac == 0.0) | __UNORDERED__(con_displayFrac, 0.0) )
        return;
      if ( *(_DWORD *)(con_debug + 32) == 2 )
      {
        frac = con_displayFrac + con_displayFrac;
        Con_DrawSolidConsole(frac);
        return;
      }
    }
  }
  else if ( (cls_keyCatchers & (KEYCATCH_UI | KEYCATCH_CGAME)) == 0 )
  {
    Con_DrawSolidConsole(1.0);
    return;
  }
  if ( !((con_displayFrac == 0.0) | __UNORDERED__(con_displayFrac, 0.0)) )
    Con_DrawSolidConsole(con_displayFrac);
}

/* ---- Con_RunConsole  0x0040A260 ----  [CONFIRMED] */
void __cdecl Con_RunConsole()
{
  con_finalFrac = 0.5;
  if ( (cls_keyCatchers & KEYCATCH_CONSOLE) == 0 )
    con_finalFrac = 0.0;
  if ( (con_finalFrac < (double)con_displayFrac) | __UNORDERED__(con_finalFrac, con_displayFrac) )
  {
    con_displayFrac = con_displayFrac - (double)cls_realFrametime * *(float *)(con_conspeed + 28) * 0.001;
    if ( con_finalFrac > (double)con_displayFrac )
      con_displayFrac = con_finalFrac;
  }
  else if ( con_finalFrac > (double)con_displayFrac )
  {
    con_displayFrac = (double)cls_realFrametime * *(float *)(con_conspeed + 28) * 0.001 + con_displayFrac;
    if ( (con_finalFrac < (double)con_displayFrac) | __UNORDERED__(con_finalFrac, con_displayFrac) )
      con_displayFrac = con_finalFrac;
  }
}

/* ---- Con_PageUp  0x0040A330 ----  [CONFIRMED] */
void __cdecl Con_PageUp()
{
  signed __int32 v0;

  v0 = con_current - (con_displayLine - 2);
  con_displayLine -= 2;
  if ( v0 >= con_totallines )
    con_displayLine = con_current - con_totallines + 1;
}

/* ---- Con_PageDown  0x0040A360 ----  [CONFIRMED] */
void __cdecl Con_PageDown()
{
  con_displayLine += 2;
  if ( con_displayLine > con_current )
    con_displayLine = con_current;
}

/* ---- Con_Top  0x0040A380 ----  [CONFIRMED] */
void __cdecl Con_Top()
{
  con_displayLine = con_totallines;
  if ( con_current - con_totallines >= con_totallines )
    con_displayLine = con_current - con_totallines + 1;
}

/* ---- Con_Bottom  0x0040A3A0 ----  [CONFIRMED] */
void __cdecl Con_Bottom()
{
  con_displayLine = con_current;
}

/* ---- Con_Close  0x0040A3B0 ----  [CONFIRMED] */
void __cdecl Con_Close()
{
  if ( com_cl_running->integer )
  {
    memset(g_consoleField.buffer, 0, MAX_EDIT_LINE);
    g_consoleField.cursor = 0;
    g_consoleField.scroll = 0;
    g_consoleField.widthInChars = MAX_EDIT_LINE;
    Con_ClearNotify();
    memset((void *)con_miniconsoleWindow, 0, 4 * dword_142F5B8);
    memset((void *)dword_142F5AC, 0, 4 * dword_142F5B8);
    dword_142F5B4 = 0;
    memset((void *)con_subtitleWindow, 0, 4 * dword_142F0E4);
    memset((void *)dword_142F0D8, 0, 4 * dword_142F0E4);
    dword_142F0E0 = 0;
    cls_keyCatchers &= ~KEYCATCH_CONSOLE;
    con_finalFrac = 0.0;
    con_displayFrac = 0.0;
  }
}

extern int cls_state;
extern int cls_keyCatchers;

/* 0x0142FF00 */
extern int dword_142FF00;

int dword_57C0F8 = 620;
int dword_57C0FC = 0x41000000;
int dword_57C100 = 0x41800000;

#define KEYCATCH_CONSOLE    0x0001

/* ---- Con_ToggleConsole_f  0x004083A0 ----  VERIFIED */
void Con_ToggleConsole_f( void )
{
	if ( cls_state == 0 && cls_keyCatchers == KEYCATCH_CONSOLE ) {
		Cbuf_AddText( "d1\n" );
		cls_keyCatchers = 0;
		return;
	}

	if ( con_restricted->integer && !dword_142FF00
	     && !( cls_keyCatchers & KEYCATCH_CONSOLE ) ) {
		return;
	}

	memset( g_consoleField.buffer, 0, MAX_EDIT_LINE );
	g_consoleField.cursor = 0;
	g_consoleField.scroll = 0;
	g_consoleField.widthInPixels = dword_57C0F8;
	g_consoleField.widthInChars = MAX_EDIT_LINE;
	*(int *)&g_consoleField.charWidth = dword_57C0FC;
	*(int *)&g_consoleField.charHeight = dword_57C100;
	g_consoleField.fixedWidth = 1;

	cls_keyCatchers ^= KEYCATCH_CONSOLE;
}

/* ---- CL_ConsolePrint  0x00409190 ----  VERIFIED */
void CL_ConsolePrint( const char *txt, int channel, int msgtime, int width ) {
	char        line[0x1000];
	const char *s;
	const char *nl;
	int         defaultColor;
	int         len;
	float       secs;
	qboolean    clampNegative;

	if ( cl_noprint && cl_noprint->integer ) {
		return;
	}
	if ( channel == CON_CHANNEL_DROP ) {
		return;
	}
	if ( !con_initialized ) {
		Con_OneTimeInit( 0 );   /* `extra` is EAX == con_initialized, zero here */
	}

	clampNegative = ( msgtime != 0 ) || ( (unsigned int)channel <= 2 );

	if ( !msgtime ) {
		secs = 0.0f;
		switch ( channel ) {
		case 0:  secs = con_minicontime->value;         break;
		case 1:  secs = con_gamemessagetime->value;     break;
		case 2:  secs = con_boldgamemessagetime->value; break;
		case 3:  msgtime = 5000;                        break;
		default:                                        break;
		}
		if ( (unsigned int)channel <= 2 ) {
			msgtime = FastRound( secs * 1000.0f );
		}
	}
	if ( clampNegative && msgtime < 0 ) {
		msgtime = 0;
	}

	defaultColor = 7;

	if ( channel != 1 && channel != 2 ) {
		CL_ConsolePrint_AddLine( (byte *)txt, channel, msgtime, width, 7 );
		return;
	}

	s = txt;
	if ( !strstr( s, "\n" ) ) {
		CL_ConsolePrint_AddLine( (byte *)s, channel, msgtime, width, 7 );
		return;
	}

	nl = strstr( s, "\n" );
	while ( 1 ) {
		if ( *s == '\n' ) {
			Con_Linefeed( msgtime, channel );
			s++;
		} else {
			len = (int)( nl - s ) + 1;
			if ( len >= 0x1000 ) {
				Com_Printf( "Text line too long. Clipping to fit\n" );
				len = 0x1000;
			}
			strncpy( line, s, len - 1 );
			line[len - 1] = 0;

			defaultColor = CL_ConsolePrint_AddLine( (byte *)line, channel,
			                                        msgtime, width,
			                                        defaultColor );
			s = ( *nl == '\n' && channel ) ? nl + 1 : nl;
		}

		if ( !strstr( s, "\n" ) ) {
			CL_ConsolePrint_AddLine( (byte *)s, channel, msgtime, width,
			                         defaultColor );
			return;
		}
		if ( !s || !*s ) {
			return;
		}
		nl = strstr( s, "\n" );
		if ( !nl ) {
			return;
		}
	}
}

/* ---- CL_ConsoleFixPosition  0x004093B0 ----  VERIFIED */
/* CL_ConsolePrint( "\n", 0, 0, 0 ) then con.display = con.current - 1, so an
 * ERR_DROP message is not left scrolled off (Com_ErrorCleanup, !dedicated).
 * Retail has the print inlined and folded for channel 0 -- the cl_noprint
 * early return falls through to the assignment, and the /GS cookie is the
 * inlined line[0x1000] the folding removed.  Takes no real argument. */
void CL_ConsoleFixPosition( int unused ) {
	int msgtime;

	(void)unused;

	if ( cl_noprint && cl_noprint->integer ) {
		con_displayLine = con_current - 1;
		return;
	}
	if ( !con_initialized ) {
		Con_OneTimeInit( 0 );   /* `extra` is EAX == con_initialized, zero here */
	}

	msgtime = FastRound( con_minicontime->value * 1000.0f );
	if ( msgtime < 0 ) {
		msgtime = 0;
	}

	CL_ConsolePrint_AddLine( (byte *)"\n", 0, msgtime, 0, 7 );
	con_displayLine = con_current - 1;
}
