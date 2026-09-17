/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "cl_vm.h"
#include "cl_records.h"

extern void CL_AddReliableCommand( const char *cmd );
extern int CL_Disconnect_f();
/* cl_input_mp.c, 0x0040C4E0 */
extern void CL_ClearKeys( void );
extern void Cmd_TokenizeString2( const char *text, int max_tokens );
extern int Con_PageDown();
extern int Con_PageUp();
extern void Con_ToggleConsole_f( void );   /* cl_console_mp.c 0x004083A0 */
extern int Con_Top();
extern int MSS_StopSounds();
extern int SCR_UpdateScreen();
extern int SEH_PrintStrlen();
char *Sys_GetClipboardData( void );     /* 0x00463300 */
extern int UI_checkKeyExec();

extern int chat_playerNum;

/* Unrenamed cells in this unit:
 *
 *   dword_142F628   retail's key_overstrikeMode (0x0142F628, read by
 *                   Key_GetOverstrikeMode at 0x0040D480).  TODO: this file also
 *                   carries a file-static key_overstrikeMode for the Get/Set
 *                   pair, so the two are separate storage here where retail
 *                   has one.
 *   dword_142FEF4   retail keys[K_CTRL].down  -- 0x0142F780 + 159*12.
 *   dword_142FF00   retail keys[K_SHIFT].down -- 0x0142F780 + 160*12.
 *                   TODO: cod1_globals.c defines both as standalone ints, not
 *                   aliases of the keys[] array, so every ctrl-/shift- modified
 *                   branch below reads a cell nothing ever writes.
 *   dword_57C0F8    the console field width; 0x0057C0FC and 0x0057C100 are the
 *                   8.0f/16.0f char metrics beside it.  RTCW cl_console.c calls
 *                   the first g_console_field_width (cl_keys.c:1156 assigns it
 *                   at exactly this point); the three are defined and driven
 *                   from cl_console_mp.c and cl_main_mp.c.
 *
 * Indirect call slots used by Field_Draw and Field_AdjustScroll: dword_143292C
 * is refexport_t +0xCC re.Text_Width and dword_1432940 is +0xE0
 * re.Text_PaintWithCursor (cl_refapi.c).
 */

/* Defined further down this unit. Retail __usercall(field@<eax>, ch@<ebx>), cdecl here. */
void __cdecl Field_CharEvent(void *field, int ch);

/* ---- Field_Draw  0x0040C500 ----  [HIGH] */
void __cdecl Field_Draw(void *edit, int x, int y)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  double v9;
  int v10;
  char v11;
  double v12;
  float v13;
  float v14;
  float v15;
  float v16;
  char v17;
  _DWORD v18[4];
  char Destination[1024];
  unsigned int v21;
  unsigned int retaddr;

  v21 = retaddr ^ _security_cookie;
  v2 = (int)edit;
  v3 = *(_DWORD *)(v2 + 4);
  v4 = 256 - v3;
  v18[0] = 1065353216;
  v18[1] = 1065353216;
  v18[2] = 1065353216;
  v18[3] = 1065353216;
  strncpy(Destination, (const char *)(v3 + v2 + 28), 256 - v3 - 1);
  v13 = (float)x;
  v14 = *(float *)(v2 + 16);
  v5 = *(_DWORD *)(v2 + 24);
  v6 = *(_DWORD *)v2 - *(_DWORD *)(v2 + 4);
  v7 = dword_142F628;
  v15 = (double)y + *(float *)(v2 + 20);
  Destination[v4 - 1] = 0;
  v16 = *(float *)(v2 + 20) * 0.020833334;
  if ( v5 )
  {
    v8 = 0;
    v9 = 640.0 / (double)cls_glconfig_vidWidth;
    v10 = 5;
    v11 = (v7 != 0) + 10;
    v13 = v13 * v9;
    v12 = 480.0 / (double)cls_glconfig_vidHeight;
    v15 = v15 * v12;
    v16 = v12 * v16;
    v14 = v9 * v14;
  }
  else
  {
    v10 = 0;
    v8 = 3;
    v11 = v7 == 0 ? 124 : 95;
  }
  v17 = v11;
  if ( !*(_DWORD *)(v2 + 8) )
    *(_DWORD *)(v2 + 8) = 256;
  dword_1432940(
    LODWORD(v13),
    LODWORD(v15),
    v10,
    LODWORD(v16),
    v18,
    Destination,
    v6,
    v17,
    LODWORD(v14),
    *(_DWORD *)(v2 + 8),
    v8);
}

/* ---- Field_AdjustScroll  0x0040C660 ----  [HIGH] */
void __cdecl Field_AdjustScroll(void *field)
{
  int v1;
  int v2;
  double v3;
  float v4;
  float v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  signed int v11;
  signed int v12;
  double v13;
  int v14;
  float v15;
  float v16;
  int v17;
  int v18;
  float v19;

  v1 = (int)field;
  v16 = *(float *)(v1 + 16);
  v19 = *(float *)(v1 + 20) * 0.020833334;
  v15 = (float)*(int *)(v1 + 12);
  if ( *(_DWORD *)(v1 + 24) )
  {
    v2 = 5;
    v19 = 480.0 / (double)cls_glconfig_vidHeight * v19;
    v3 = 640.0 / (double)cls_glconfig_vidWidth;
    v16 = v16 * v3;
    v15 = v3 * v15;
  }
  else
  {
    v2 = 0;
  }
  v4 = v16;
  v5 = v19;
  v17 = dword_143292C(v1 + 28, v2, LODWORD(v19), LODWORD(v16), 0);
  if ( ((double)v17 < v15) | __UNORDERED__((double)v17, v15) )
  {
    *(_DWORD *)(v1 + 4) = 0;
    *(_DWORD *)(v1 + 8) = SEH_PrintStrlen((char *)(v1 + 28));
    return;
  }
  if ( (v15 > 0.0) | __UNORDERED__(0.0, v15) )
  {
    while ( 1 )
    {
      v6 = *(_DWORD *)(v1 + 4);
      if ( v6 <= 0 || (double)dword_143292C(v6 + v1 + 27, v2, LODWORD(v19), LODWORD(v4), 0) >= v15 )
        break;
      --*(_DWORD *)(v1 + 4);
    }
  }
  while ( 1 )
  {
    v7 = dword_143292C(*(_DWORD *)(v1 + 4) + v1 + 28, v2, LODWORD(v5), LODWORD(v4), 0);
    v8 = v7 - dword_143292C(*(_DWORD *)v1 + v1 + 28, v2, LODWORD(v19), LODWORD(v4), 0);
    v18 = v8;
    if ( v8 < 0 )
    {
      v14 = *(_DWORD *)(v1 + 4);
      if ( !v14 )
      {
        v18 = 0;
        goto LABEL_17;
      }
      v9 = v14 - 1;
    }
    else
    {
      if ( (double)v8 < v15 )
        goto LABEL_16;
      v9 = *(_DWORD *)(v1 + 4) + 1;
    }
    *(_DWORD *)(v1 + 4) = v9;
LABEL_16:
    if ( v8 < 0 )
      goto LABEL_11;
LABEL_17:
    if ( (double)v18 < v15 )
      break;
LABEL_11:
    v5 = v19;
  }
  v10 = *(_DWORD *)(v1 + 4);
  v11 = strlen((const char *)(v10 + v1 + 28));
  *(_DWORD *)(v1 + 8) = *(_DWORD *)v1 - v10;
  if ( (v15 > 0.0) | __UNORDERED__(0.0, v15) )
  {
    while ( 1 )
    {
      v12 = *(_DWORD *)(v1 + 8);
      if ( v12 >= v11 )
        break;
      v13 = (double)dword_143292C(*(_DWORD *)(v1 + 4) + v1 + 28, v2, LODWORD(v19), LODWORD(v4), v12 + 1);
      if ( !((v13 < v15) | __UNORDERED__(v13, v15)) )
        break;
      ++*(_DWORD *)(v1 + 8);
    }
  }
}

/* ---- Field_Paste  0x0040C870 ----  [CONFIRMED] */
void __cdecl Field_Paste(void *field)
{
  char *ClipboardData;
  char *v4;
  int v5;
  int v6;

  ClipboardData = Sys_GetClipboardData();
  v4 = ClipboardData;
  if ( ClipboardData )
  {
    v5 = strlen(ClipboardData);
    v6 = 0;
    if ( v5 > 0 )
    {
      do
      {
        Field_CharEvent(field, v4[v6]);
        ++v6;
      }
      while ( v6 < v5 );
    }
    free(v4);
  }
}

/* ---- Field_KeyDownEvent  0x0040C8C0 ----  [HIGH] */
void __cdecl Field_KeyDownEvent(void *field, int key)
{
  int v2;
  int v3;
  int *v4;
  signed int v5;
  int v6;
  int v7;
  int v8;
  bool v9;
  void *v10;

  v2 = (int)field;
  v3 = key;
  v10 = field;
  v4 = (int *)v2;
  v5 = strlen((const char *)(v2 + 28));
  if ( (v3 == 161 || v3 == 192) && dword_142FF00 )
  {
    Field_Paste(v4);
  }
  else
  {
    switch ( v3 )
    {
      case 162:
        if ( *v4 < v5 )
          memcpy((char *)v4 + *v4 + 28, (char *)v4 + *v4 + 29, v5 - *v4);
        break;
      case 157:
        if ( *v4 < v5 )
          ++*v4;
        if ( dword_142FEF4 )
        {
          if ( *v4 < v5 )
          {
            do
            {
              if ( !isalnum(*((char *)v4 + *v4 + 28)) )
                break;
              v6 = *v4 + 1;
              *v4 = v6;
            }
            while ( v6 < v5 );
            if ( *v4 < v5 )
            {
              do
              {
                if ( isalnum(*((char *)v4 + *v4 + 28)) )
                  break;
                v7 = *v4 + 1;
                *v4 = v7;
              }
              while ( v7 < v5 );
            }
          }
        }
        break;
      case 156:
        if ( *v4 > 0 )
          --*v4;
        if ( dword_142FEF4 )
        {
          if ( *v4 > 0 )
          {
            do
            {
              if ( !isalnum(*((char *)v4 + *v4 + 27)) )
                break;
              v8 = *v4 - 1;
              v9 = *v4 == 1;
              *v4 = v8;
            }
            while ( v8 >= 0 && !v9 );
          }
        }
        if ( *v4 < v4[1] )
          v4[1] = *v4;
        break;
      default:
        if ( v3 == 165 || tolower(v3) == 97 && dword_142FEF4 )
        {
          *v4 = 0;
        }
        else if ( v3 == 166 || tolower(v3) == 101 && dword_142FEF4 )
        {
          *v4 = v5;
        }
        else if ( v3 == 161 )
        {
          dword_142F628 = dword_142F628 == 0;
        }
        break;
    }
  }
  if ( uivm )
    Field_AdjustScroll(v10);
}

/* ---- Field_CharEvent  0x0040CA70 ----  VERIFIED */
void __cdecl Field_CharEvent(void *field, int ch)
{
  int v3;
  int *v4;
  void *v5;
  unsigned int v6;
  unsigned int v7;
  int v8;

  v4 = (int *)field;
  v3 = ch;
  v5 = (char *)field + 28;
  v6 = strlen((const char *)field + 28);
  v7 = v6;
  switch ( v3 )
  {
    case 22:
      Field_Paste(v4);
      Field_AdjustScroll(v4);
      return;
    case 3:
      memset(v5, 0, 0x100u);
      *v4 = 0;
      v4[1] = 0;
      v4[2] = 256;
      Field_AdjustScroll(v4);
      return;
    case 8:
      if ( *v4 > 0 )
      {
        memcpy((char *)v4 + *v4 + 27, (char *)v4 + *v4 + 28, v6 - *v4 + 1);
        --*v4;
        Field_AdjustScroll(v4);
        return;
      }
LABEL_20:
      Field_AdjustScroll(v4);
      return;
    case 1:
      *v4 = 0;
      v4[1] = 0;
      return;
    case 5:
      *v4 = v6;
      Field_AdjustScroll(v4);
      return;
  }
  if ( v3 >= 32 )
  {
    if ( dword_142F628 )
    {
      if ( *v4 == 255 )
        return;
      *((_BYTE *)v4 + *v4 + 28) = v3;
    }
    else
    {
      if ( v6 == 255 )
        return;
      memcpy((char *)v4 + *v4 + 29, (char *)v4 + *v4 + 28, v6 - *v4 + 1);
      *((_BYTE *)v4 + *v4 + 28) = v3;
    }
    v8 = *v4 + 1;
    *v4 = v8;
    if ( v8 == v7 + 1 )
      *((_BYTE *)v4 + v8 + 28) = 0;
    goto LABEL_20;
  }
}

/* ---- CL_FindMatches  0x0040CB80 ----  VERIFIED */
void __cdecl CL_FindMatches( const char *s )
{
	int		i;

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}

	matchCount++;
	if ( matchCount == 1 ) {
		strncpy( shortestMatch, s, MAX_TOKEN_CHARS - 1 );
		shortestMatch[MAX_TOKEN_CHARS - 1] = 0;
		return;
	}

	for ( i = 0 ; s[i] ; i++ ) {
		if ( tolower( shortestMatch[i] ) != tolower( s[i] ) ) {
			break;
		}
	}
	shortestMatch[i] = 0;
}

/* ---- CL_PrintMatches  0x0040CC20 ----  VERIFIED */
void __cdecl CL_PrintMatches( const char *s )
{
	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
	}
}

/* ---- keyConcatArgs  0x0040CC60 ----  VERIFIED */
static void keyConcatArgs( void )
{
	int		i;
	char	*arg;

	for ( i = 1 ; i < cmd_argc ; i++ ) {
		Q_strcat( (char *)text_in, 256, " " );

		arg = ( (unsigned int)i < (unsigned int)cmd_argc ) ? cmd_argv[i] : (char *)empty_string;
		while ( *arg ) {
			if ( *arg == ' ' ) {
				Q_strcat( (char *)text_in, 256, "\"" );
				break;
			}
			arg++;
		}

		Q_strcat( (char *)text_in, 256,
				  ( (unsigned int)i < (unsigned int)cmd_argc ) ? cmd_argv[i] : (char *)empty_string );

		if ( *arg == ' ' ) {
			Q_strcat( (char *)text_in, 256, "\"" );
		}
	}
}

/* ---- CL_ConcatRemaining  0x0040CE30 ----  VERIFIED */
void __cdecl CL_ConcatRemaining( const char *src, const char *start )
{
	char	*str;

	str = strstr( src, start );
	if ( !str ) {
		keyConcatArgs();
		return;
	}

	str += strlen( start );
	Q_strcat( (char *)text_in, 256, str );
}

/* ---- CompleteCommand  0x0040CE80 ----  VERIFIED */
void __cdecl CompleteCommand( void )
{
	char	temp[256];
	char	*cmd;

	Cmd_TokenizeString2( (const char *)text_in, 0 );

	cmd = (char *)empty_string;
	if ( (unsigned int)cmd_argc > 0 ) {
		cmd = cmd_argv[0];
	}
	completionString = cmd;
	if ( cmd[0] == '\\' || cmd[0] == '/' ) {
		completionString = ++cmd;
	}

	matchCount = 0;
	shortestMatch[0] = 0;

	if ( !strlen( cmd ) ) {
		return;
	}

	Cmd_CommandCompletion( CL_FindMatches );
	Cvar_CommandCompletion( CL_FindMatches );

	if ( matchCount == 0 ) {
		return;
	}

	Com_Memcpy( temp, text_in, sizeof( temp ) );

	if ( matchCount == 1 ) {
		Com_sprintf( (char *)text_in, 256, "\\%s", shortestMatch );
		if ( cmd_argc == 1 ) {
			Q_strcat( (char *)text_in, 256, " " );
		} else {
			CL_ConcatRemaining( temp, completionString );
		}
		g_consoleField.cursor = strlen( (const char *)text_in );
		Field_AdjustScroll( &g_consoleField );
		return;
	}

	Com_sprintf( (char *)text_in, 256, "\\%s", shortestMatch );
	g_consoleField.cursor = strlen( (const char *)text_in );
	CL_ConcatRemaining( temp, completionString );
	Field_AdjustScroll( &g_consoleField );

	Com_Printf( "]%s\n", text_in );

	Cmd_CommandCompletion( CL_PrintMatches );
	Cvar_CommandCompletion( CL_PrintMatches );
}

/* ---- Console_Key  0x0040D050 ----  [HIGH] */
void __cdecl Console_Key(int key)
{
  int v3;
  int ctrl;
  char Destination[1024];

  if ( key == 'l' )
  {
    if ( dword_142FEF4 )
    {
      Cbuf_AddText("clear\n");
      return;
    }
    goto LABEL_15;
  }

  if ( key != 13 && key != 191 )
  {
    switch ( key )
    {
      case 9:
        CompleteCommand();
        return;
      case 206:
        if ( dword_142FF00 )
          goto LABEL_10;
        break;
      case 154:
      case 183:
        goto LABEL_10;
    }
LABEL_15:
    if ( tolower(key) != 'p' || !dword_142FEF4 )
    {
      if ( key == 205 )
      {
        if ( dword_142FF00 )
        {
LABEL_19:
          if ( historyLine == nextHistoryLine )
            return;
          v3 = historyLine + 1;
          goto LABEL_21;
        }
      }
      else if ( key == 155 || key == 189 )
      {
        goto LABEL_19;
      }
      ctrl = dword_142FEF4;
      if ( tolower(key) == 'n' && ctrl )
        goto LABEL_19;
      switch ( key )
      {
        case 164:
LABEL_32:
          Con_PageUp();
          return;
        case 163:
LABEL_36:
          Con_PageDown();
          return;
        case 206:
          Con_PageUp();
          if ( !ctrl )
            return;
          Con_PageUp();
          goto LABEL_32;
        case 205:
          Con_PageDown();
          if ( !ctrl )
            return;
          Con_PageDown();
          goto LABEL_36;
        case 165:
          if ( dword_142FEF4 )
          {
            Con_Top();
            return;
          }
          break;
        default:
          if ( key == 166 && dword_142FEF4 )
          {
            con_displayLine = con_current;
            return;
          }
          break;
      }
      Field_KeyDownEvent(&g_consoleField, key);
      return;
    }
LABEL_10:
    v3 = historyLine;
    if ( nextHistoryLine - historyLine >= COMMAND_HISTORY || historyLine <= 0 )
      goto LABEL_22;
    v3 = historyLine - 1;
LABEL_21:
    historyLine = v3;
LABEL_22:
    g_consoleField = historyEditLines[v3 % COMMAND_HISTORY];
    Field_AdjustScroll(&g_consoleField);
    return;
  }

  if ( cls_state != CA_ACTIVE && text_in[0] != '\\' && text_in[0] != '/' )
  {
    strncpy(Destination, (const char *)text_in, 0x3FFu);
    Destination[1023] = 0;
    Com_sprintf((char *)text_in, 256, "\\%s", Destination);
    ++g_consoleField.cursor;
  }

  Com_Printf("]%s\n", text_in);

  if ( text_in[0] != '\\' && text_in[0] != '/' )
  {
    if ( !text_in[0] )
      return;
    Cbuf_AddText("cmd say ");
    Cbuf_AddText((const char *)text_in);
  }
  else
  {
    Cbuf_AddText((const char *)text_in + 1);
  }
  Cbuf_AddText("\n");

  historyEditLines[nextHistoryLine % COMMAND_HISTORY] = g_consoleField;
  ++nextHistoryLine;
  historyLine = nextHistoryLine;
  memset(text_in, 0, 0x100u);
  g_consoleField.cursor = 0;
  g_consoleField.scroll = 0;
  g_consoleField.widthInChars = 256;
  g_consoleField.widthInPixels = dword_57C0F8;
  *(int *)&g_consoleField.charWidth = dword_57C0FC;
  *(int *)&g_consoleField.charHeight = dword_57C100;
  g_consoleField.fixedWidth = 1;

  if ( cls_state == CA_DISCONNECTED )
    SCR_UpdateScreen();
}

/* ---- Message_Key  0x0040D380 ----  VERIFIED */
void __cdecl Message_Key( int key )
{
	char	buffer[MAX_STRING_CHARS];

	/* dword_1430380 below is RTCW cl_keys.c:45 `qboolean chat_team`; the three
	 * Con_MessageMode*_f in cl_console_mp.c drive it exactly as RTCW does. */
	if ( key == 27 ) {
		cls_keyCatchers &= ~KEYCATCH_MESSAGE;
	} else if ( key == 13 || key == 0xBF ) {
		if ( chatField.buffer[0] && cls_state == CA_ACTIVE ) {
			if ( chat_playerNum != -1 ) {
				Com_sprintf( buffer, sizeof( buffer ), "tell %i \"" "\x15" "%s\"\n",
							 chat_playerNum, chatField.buffer );
			} else if ( dword_1430380 ) {
				Com_sprintf( buffer, sizeof( buffer ), "say_team \"" "\x15" "%s\"\n",
							 chatField.buffer );
			} else {
				Com_sprintf( buffer, sizeof( buffer ), "say \"" "\x15" "%s\"\n",
							 chatField.buffer );
			}
			CL_AddReliableCommand( buffer );
		}
		cls_keyCatchers &= ~KEYCATCH_MESSAGE;
	} else {
		Field_KeyDownEvent( &chatField, key );
		return;
	}

	memset( chatField.buffer, 0, 256 );
	chatField.cursor = 0;
	chatField.scroll = 0;
	chatField.widthInChars = 256;
}

static qboolean key_overstrikeMode;

/* ---- Key_GetOverstrikeMode  0x0040D480 ----  [CONFIRMED] */
qboolean Key_GetOverstrikeMode(void)
{
    return key_overstrikeMode;
}

/* ---- Key_SetOverstrikeMode  0x0040D490 ----  [CONFIRMED] */
void Key_SetOverstrikeMode(qboolean enabled)
{
    key_overstrikeMode = enabled;
}

/* ---- Key_IsDown  0x0040D4A0 ----  VERIFIED */
qboolean __cdecl Key_IsDown( int keynum )
{
	if ( keynum == -1 ) {
		return qfalse;
	}
	return (qboolean)dword_142F780[3 * keynum];
}

/* Key_StringToKeynum (0x0040D4C0) is defined further down this unit, along with the keynames[] table at 0x0057B8D0. */

#if 0
char *__cdecl Key_KeynumToString(int keynum)
{
  int v1;
  int v2;
  int v3;
  char **v5;
  char *v6;
  int v7;
  int v8;
  char v9;
  char v10;

  v3 = v1;
  if ( v1 == -1 )
    return "<KEY NOT FOUND>";
  if ( (unsigned int)v1 >= 0x100 )
    return "<OUT OF RANGE>";
  if ( v2 && cl_language->integer == 1 && v1 >= 48 && v1 <= 57 )
    return (&off_57C010)[v1];
  if ( v1 <= 32 || v1 >= 127 || v1 == 34 )
  {
    v5 = &off_57BCD0;
    if ( v2 )
      goto LABEL_17;
  }
  else
  {
    byte_87A2FC = toupper(v1);
    byte_87A2FD = 0;
    if ( v3 != 59 || v2 )
      return &byte_87A2FC;
  }
  v5 = (char **)&off_57B8D0;
LABEL_17:
  if ( *v5 )
  {
    while ( (char *)v3 != v5[1] )
    {
      v6 = v5[2];
      v5 += 2;
      if ( !v6 )
        goto LABEL_20;
    }
    return *v5;
  }
  else
  {
LABEL_20:
    v7 = v3 >> 4;
    v8 = v3 & 0xF;
    byte_87A2FC = 48;
    byte_87A2FD = 120;
    if ( v7 <= 9 )
      v9 = v7 + 48;
    else
      v9 = v7 + 87;
    byte_87A2FE = v9;
    if ( v8 <= 9 )
      v10 = v8 + 48;
    else
      v10 = v8 + 87;
    byte_87A2FF = v10;
    byte_87A300 = 0;
    return &byte_87A2FC;
  }
}
#endif

/* Key_SetBinding (0x0040D6A0) is defined further down this unit, where the keys[] table lives. */
#if 0
void __cdecl Key_SetBinding(int keynum, const char *binding)
{
  int v2;
  const char *v3;
  int v4;
  char *v5;
  int *v6;

  if ( v2 != -1 )
  {
    v4 = 3 * v2;
    v5 = (&s1)[3 * v2];
    if ( v5 )
      free(v5);
    v6 = Z_MallocInternal(strlen(v3) + 1);
    strcpy((char *)v6, v3);
    (&s1)[v4] = (char *)v6;
    cvar_modifiedFlags |= 1u;
  }
}
#endif

/* Key_GetBinding (0x0040D700) is defined further down this unit. */
#if 0
char *__cdecl Key_GetBinding(int keynum)
{
  if ( keynum == -1 )
    return &empty_string;
  else
    return (&s1)[3 * keynum];
}
#endif

/* ---- Key_GetKey  0x0040D730 ----  VERIFIED */
int __cdecl Key_GetKey( const char *binding )
{
	int		i;

	if ( binding ) {
		for ( i = 0 ; i < MAX_KEYS ; i++ ) {
			if ( dword_142F780[3 * i + 2]
				 && !Q_stricmpn( binding, (const char *)dword_142F780[3 * i + 2], 99999 ) ) {
				return i;
			}
		}
	}

	return -1;
}
/* Key_Unbind_f (0x0040D770) is defined further down this unit. */

/* Key_UnbindAll_f (0x0040D7C0) is defined further down this unit. */
#if 0
void __cdecl Key_UnbindAll_f()
{
  int v0;
  char **v1;
  int v2;
  const char *v3;

  v0 = 0;
  v1 = &s1;
  do
  {
    if ( *v1 )
      Key_SetBinding(v2, v3);
    v1 += 3;
    ++v0;
  }
  while ( (int)v1 < (int)&unk_1430388 );
}
#endif

/* Key_Bind_f (0x0040D800) is defined further down this unit. */
#if 0
void __cdecl Key_Bind_f()
{
  const char *v0;
  const char *v1;
  unsigned __int32 v2;
  char *v3;
  int v4;
  char *v5;
  int v6;
  signed __int32 v7;
  const char *v8;
  char *v9;
  char *v10;
  unsigned int v11;
  signed __int32 v12;
  char *v13;
  char *v14;
  unsigned int v15;
  char *v16;
  char *v18;
  int v20;
  const char *v21;
  signed __int32 v22; // [esp+8h] [ebp-408h] BYREF
  char v23;
  unsigned int v24;
  unsigned int retaddr;

  v2 = cmd_argc;
  v24 = retaddr ^ _security_cookie;
  v22 = cmd_argc;
  if ( cmd_argc >= 2 )
  {
    v3 = cmd_argv[1];
    v21 = v0;
    v4 = Key_StringToKeynum(v1);
    if ( v4 == -1 )
    {
      v5 = &empty_string;
      if ( v2 > 1 )
        v5 = v3;
      Com_Printf("\"%s\" isn't a valid key\n", v5);
    }
    else
    {
      v6 = tolower(v4);
      v7 = v22;
      if ( v22 == 2 )
      {
        v8 = (&s1)[3 * v6];
        if ( v8 )
        {
          v9 = &empty_string;
          if ( (unsigned int)cmd_argc > 1 )
            v9 = cmd_argv[1];
          Com_Printf("\"%s\" = \"%s\"\n", v9, v8);
        }
        else
        {
          v10 = &empty_string;
          if ( (unsigned int)cmd_argc > 1 )
            v10 = cmd_argv[1];
          Com_Printf("\"%s\" is not bound\n", v10);
        }
      }
      else
      {
        v11 = 2;
        v23 = 0;
        if ( v22 > 2 )
        {
          v12 = v22 - 1;
          do
          {
            v13 = &empty_string;
            if ( v11 < cmd_argc )
              v13 = cmd_argv[v11];
            v14 = v13;
            v15 = strlen(v13) + 1;
            v16 = (char *)&v22 + 3;
            while ( *++v16 )
              ;
            qmemcpy(v16, v14, v15);
            if ( v11 != v12 )
            {
              v18 = (char *)&v22 + 3;
              while ( *++v18 )
                ;
              strcpy(v18, " ");
            }
            ++v11;
          }
          while ( (int)v11 < v7 );
        }
        Key_SetBinding(v20, v21);
      }
    }
  }
  else
  {
    Com_Printf("bind <key> [command] : attach a command to a key\n");
  }
}
#endif

/* Key_WriteBindings (0x0040D9A0) is defined further down this unit. */
#if 0
void __cdecl Key_WriteBindings(int f)
{
  int i;
  char *v2;
  const char *v3;
  const char **v4;
  const char *v5;
  int v6;
  char v7;
  char v8;
  char v9;

  FS_Printf(f, "unbindall\n");
  for ( i = 0; i < 256; ++i )
  {
    v2 = (&s1)[3 * i];
    if ( v2 && *v2 )
    {
      if ( i == -1 )
      {
        v3 = "<KEY NOT FOUND>";
      }
      else if ( i < 0 )
      {
        v3 = "<OUT OF RANGE>";
      }
      else if ( i <= 32 || i >= 127 || i == 34 || (byte_87A2FC = toupper(i), byte_87A2FD = 0, i == 59) )
      {
        v4 = (const char **)&off_57B8D0;
        if ( off_57B8D0 )
        {
          while ( (const char *)i != v4[1] )
          {
            v5 = v4[2];
            v4 += 2;
            if ( !v5 )
              goto LABEL_15;
          }
          v3 = *v4;
        }
        else
        {
LABEL_15:
          v6 = i >> 4;
          v7 = i & 0xF;
          byte_87A2FC = 48;
          byte_87A2FD = 120;
          if ( i >> 4 <= 9 )
            v8 = v6 + 48;
          else
            v8 = v6 + 87;
          byte_87A2FE = v8;
          if ( (i & 0xFu) <= 9 )
            v9 = v7 + 48;
          else
            v9 = v7 + 87;
          byte_87A2FF = v9;
          byte_87A300 = 0;
          v3 = &byte_87A2FC;
        }
      }
      else
      {
        v3 = &byte_87A2FC;
      }
      FS_Printf(f, "bind %s \"%s\"\n", v3, (&s1)[3 * i]);
    }
  }
}
#endif

/* Key_Bindlist_f (0x0040DAD0) is defined further down this unit. */
#if 0
void __cdecl Key_Bindlist_f()
{
  int i;
  char *v1;
  const char *v2;
  const char **v3;
  const char *v4;
  int v5;
  char v6;
  char v7;
  char v8;

  for ( i = 0; i < 256; ++i )
  {
    v1 = (&s1)[3 * i];
    if ( v1 && *v1 )
    {
      if ( i == -1 )
      {
        v2 = "<KEY NOT FOUND>";
      }
      else if ( i < 0 )
      {
        v2 = "<OUT OF RANGE>";
      }
      else if ( i <= 32 || i >= 127 || i == 34 || (byte_87A2FC = toupper(i), byte_87A2FD = 0, i == 59) )
      {
        v3 = (const char **)&off_57B8D0;
        if ( off_57B8D0 )
        {
          while ( (const char *)i != v3[1] )
          {
            v4 = v3[2];
            v3 += 2;
            if ( !v4 )
              goto LABEL_15;
          }
          v2 = *v3;
        }
        else
        {
LABEL_15:
          v5 = i >> 4;
          v6 = i & 0xF;
          byte_87A2FC = 48;
          byte_87A2FD = 120;
          if ( i >> 4 <= 9 )
            v7 = v5 + 48;
          else
            v7 = v5 + 87;
          byte_87A2FE = v7;
          if ( (i & 0xFu) <= 9 )
            v8 = v6 + 48;
          else
            v8 = v6 + 87;
          byte_87A2FF = v8;
          byte_87A300 = 0;
          v2 = &byte_87A2FC;
        }
      }
      else
      {
        v2 = &byte_87A2FC;
      }
      Com_Printf("%s \"%s\"\n", v2, (&s1)[3 * i]);
    }
  }
}
#endif

/* ---- CL_InitKeyCommands  0x0040DBF0 ----  [CONFIRMED] */
void __cdecl CL_InitKeyCommands()
{
  Cmd_AddCommand("bind", Key_Bind_f);
  Cmd_AddCommand("unbind", Key_Unbind_f);
  Cmd_AddCommand("unbindall", Key_UnbindAll_f);
  Cmd_AddCommand("bindlist", Key_Bindlist_f);
}

/* CL_KeyEvent (0x0040DC30) is defined further down this unit. */
#if 0
void __cdecl CL_KeyEvent(int key, qboolean down, unsigned int time)
{
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  bool v8; // sf
  int v9;
  int v10;
  char *v11;
  vm_s *v12;
  vm_s *v13;
  char *v14;
  int integer;
  int v16;
  bool v17; // zf
  int v18;
  const char *v19;
  char *v20;
  const char **v21;
  const char *v22;
  int v23;
  int v24;
  char v25;
  char v26;
  const char *v27;
  const char *v28;
  int v29;

  v4 = cls_keyCatchers;
  v5 = key;
  v6 = v3;
  dword_142F780[3 * key] = v3;
  if ( v3 )
  {
    v7 = dword_142F784[3 * key] + 1;
    v8 = dword_142F784[3 * key] < 0;
    dword_142F784[3 * key] = v7;
    if ( v7 == 1 )
    {
      ++anykeydown;
    }
    else if ( !(v8 ^ __OFSUB__(v7, 1) | (v7 == 1)) && ((v4 & 5) == 0 || key == 96 || key == 126 || key == 27) )
    {
      return;
    }
    if ( cl_waitForFire && cl_waitForFire->integer )
    {
      if ( (v4 & 1) != 0 )
        Con_ToggleConsole_f();
      v11 = (&s1)[3 * key];
      memset(KB_LEFT, 0, 0x2D0u);
      if ( v11 )
      {
        if ( !Q_stricmpn(v27, v28, v29) )
          Cvar_Set2("cl_waitForFire", "0", qtrue);
      }
      return;
    }
  }
  else
  {
    v9 = anykeydown - 1;
    v8 = anykeydown - 1 < 0;
    dword_142F784[3 * key] = 0;
    anykeydown = v9;
    if ( v8 )
      anykeydown = 0;
  }
  if ( key == 96 || key == 126 )
  {
    if ( v3 )
      Con_ToggleConsole_f();
    return;
  }
  v10 = *(_DWORD *)cls_state;
  if ( v3
    && (key < 128 || key == 200)
    && (clc_demoplaying || *(_DWORD *)cls_state == 7 || *(_DWORD *)cls_state == 8)
    && !v4 )
  {
    Cvar_Set2("nextdemo", &empty_string, qtrue);
    v10 = *(_DWORD *)cls_state;
    v4 = cls_keyCatchers;
    v5 = 27;
  }
  else if ( key != 27 )
  {
    goto LABEL_46;
  }
  if ( v6 )
  {
    if ( (v4 & 4) != 0 )
    {
LABEL_31:
      Message_Key((int)v27);
      return;
    }
    if ( (v4 & 8) != 0 )
    {
      cls_keyCatchers = v4 & 0xFFFFFFF7;
      VM_Call(cgvm, 8, 0);
      return;
    }
    if ( (v4 & 2) != 0 )
    {
      VM_Call(uivm, 3, 27, v6);
      return;
    }
    if ( v10 == 6 )
    {
      if ( clc_demoplaying )
      {
        VM_Call(uivm, 7, 1);
        return;
      }
      v12 = uivm;
      if ( !cl_serverloadwaiting->integer )
      {
        VM_Call(uivm, 7, 2);
        return;
      }
LABEL_40:
      VM_Call(v12, 7, 1);
      return;
    }
    if ( v10 > 6 && v10 <= 8 )
    {
      CL_Disconnect_f();
      MSS_StopSounds(0);
      VM_Call(uivm, 7, 1);
      return;
    }
    v12 = uivm;
    if ( uivm )
      goto LABEL_40;
    return;
  }
LABEL_46:
  v13 = cgvm;
  if ( cgvm )
  {
    if ( VM_Call(cgvm, 6, v5, v6) )
      return;
    v10 = *(_DWORD *)cls_state;
    LOBYTE(v4) = cls_keyCatchers;
    v13 = cgvm;
  }
  if ( !v6 )
  {
    v14 = (&s1)[3 * v5];
    if ( v14 )
    {
      if ( *v14 == 43 )
      {
        Com_sprintf("-%s %i %i\n", (int)(v14 + 1), (const char *)v5, down);
        Cbuf_AddText(v27);
        LOBYTE(v4) = cls_keyCatchers;
        v13 = cgvm;
      }
    }
    if ( (v4 & 2) != 0 && uivm )
    {
      VM_Call(uivm, 3, v5, 0);
    }
    else if ( (v4 & 8) != 0 )
    {
      if ( v13 )
        VM_Call(v13, 5, v5, 0);
    }
    return;
  }
  if ( cl_bypassMouseInput )
  {
    integer = cl_bypassMouseInput->integer;
    if ( integer )
    {
      if ( v5 == 200 || v5 == 201 || v5 == 202 )
      {
        v17 = integer == 1;
        v18 = 1;
        if ( v17 )
          goto LABEL_68;
      }
      else
      {
        v16 = UI_checkKeyExec((void *)v5);
        LOBYTE(v4) = cls_keyCatchers;
        v10 = *(_DWORD *)cls_state;
        v17 = v16 == 0;
        v13 = cgvm;
        if ( v17 )
        {
          v18 = 1;
          goto LABEL_68;
        }
      }
    }
  }
  v18 = 0;
LABEL_68:
  if ( (v4 & 1) != 0 )
    goto LABEL_69;
  if ( (v4 & 2) != 0 && !v18 )
  {
    v19 = (&s1)[3 * v5];
    if ( v19 )
    {
      if ( !Q_stricmp(v19, "help") && VM_Call(uivm, 8) == 7 )
        v5 = 27;
    }
    VM_Call(uivm, 3, v5, v6);
    return;
  }
  if ( (v4 & 8) != 0 )
  {
    if ( v13 )
      VM_Call(v13, 5, v5, v6);
    return;
  }
  if ( (v4 & 4) != 0 )
    goto LABEL_31;
  if ( !v10 )
  {
LABEL_69:
    Console_Key((int)v27);
    return;
  }
  v20 = (&s1)[3 * v5];
  if ( v20 )
  {
    if ( *v20 == 43 )
      Com_sprintf("%s %i %i\n", (int)v20, (const char *)v5, down);
    else
      Cbuf_AddText(v27);
    Cbuf_AddText(v27);
  }
  else if ( v5 >= 200 )
  {
    if ( v5 > 255 )
    {
      Com_Printf("%s is unbound, use controls menu to set.\n", "<OUT OF RANGE>");
    }
    else
    {
      v21 = (const char **)&off_57B8D0;
      if ( off_57B8D0 )
      {
        while ( (const char *)v5 != v21[1] )
        {
          v22 = v21[2];
          v21 += 2;
          if ( !v22 )
            goto LABEL_88;
        }
        Com_Printf("%s is unbound, use controls menu to set.\n", *v21);
      }
      else
      {
LABEL_88:
        v23 = v5 >> 4;
        v24 = v5 & 0xF;
        byte_87A2FC = 48;
        byte_87A2FD = 120;
        if ( v23 <= 9 )
          v25 = v23 + 48;
        else
          v25 = v23 + 87;
        byte_87A2FE = v25;
        if ( v24 <= 9 )
          v26 = v24 + 48;
        else
          v26 = v24 + 87;
        byte_87A2FF = v26;
        byte_87A300 = 0;
        Com_Printf("%s is unbound, use controls menu to set.\n", &byte_87A2FC);
      }
    }
  }
}
#endif

/* ---- Key_ClearStates  0x0040E260 ----  VERIFIED */
void __cdecl Key_ClearStates( void )
{
  int v0;
  int *v1;

  anykeydown = 0;
  v0 = 0;
  v1 = dword_142F780;
  do
  {
    if ( *v1 )
      CL_KeyEvent(v0, qfalse, 0);
    *v1 = 0;
    v1[1] = 0;
    v1 += 3;
    ++v0;
  }
  while ( v0 < MAX_KEYS );
}

                    /* KEY BINDINGS 0x0040D4C0-0x0040DBF0 */

typedef struct {
	qboolean down;
	int repeats;
	char *binding;
} qkey_t;

/* RTCW cl_keys.c:52 `qkey_t keys[MAX_KEYS];`.  Key_IsDown, Key_GetKey and
 * Key_ClearStates above index dword_142F780[3*keynum] by hand; keys[n].down
 * emits a different multiply. */
#define keys        ( (qkey_t *)dword_142F780 )
#define anykeydown  anykeydown           /* 0x0142F75C */

typedef struct {
	const char *name;
	int keynum;
} keyname_t;

static keyname_t keynames[] = {
	{ "TAB", 9 }, { "ENTER", 13 }, { "ESCAPE", 27 }, { "SPACE", 32 },
	{ "BACKSPACE", 127 }, { "UPARROW", 154 }, { "DOWNARROW", 155 },
	{ "LEFTARROW", 156 }, { "RIGHTARROW", 157 },
	{ "ALT", 158 }, { "CTRL", 159 }, { "SHIFT", 160 }, { "CAPSLOCK", 151 },
	{ "F1", 167 }, { "F2", 168 }, { "F3", 169 }, { "F4", 170 }, { "F5", 171 },
	{ "F6", 172 }, { "F7", 173 }, { "F8", 174 }, { "F9", 175 }, { "F10", 176 },
	{ "F11", 177 }, { "F12", 178 },
	{ "INS", 161 }, { "DEL", 162 }, { "PGDN", 163 }, { "PGUP", 164 },
	{ "HOME", 165 }, { "END", 166 },
	{ "MOUSE1", 200 }, { "MOUSE2", 201 }, { "MOUSE3", 202 }, { "MOUSE4", 203 },
	{ "MOUSE5", 204 }, { "MWHEELUP", 206 }, { "MWHEELDOWN", 205 },
	{ "JOY1", 207 }, { "JOY2", 208 }, { "JOY3", 209 }, { "JOY4", 210 },
	{ "JOY5", 211 }, { "JOY6", 212 }, { "JOY7", 213 }, { "JOY8", 214 },
	{ "JOY9", 215 }, { "JOY10", 216 }, { "JOY11", 217 }, { "JOY12", 218 },
	{ "JOY13", 219 }, { "JOY14", 220 }, { "JOY15", 221 }, { "JOY16", 222 },
	{ "JOY17", 223 }, { "JOY18", 224 }, { "JOY19", 225 }, { "JOY20", 226 },
	{ "JOY21", 227 }, { "JOY22", 228 }, { "JOY23", 229 }, { "JOY24", 230 },
	{ "JOY25", 231 }, { "JOY26", 232 }, { "JOY27", 233 }, { "JOY28", 234 },
	{ "JOY29", 235 }, { "JOY30", 236 }, { "JOY31", 237 }, { "JOY32", 238 },
	{ "AUX1", 239 }, { "AUX2", 240 }, { "AUX3", 241 }, { "AUX4", 242 },
	{ "AUX5", 243 }, { "AUX6", 244 }, { "AUX7", 245 }, { "AUX8", 246 },
	{ "AUX9", 247 }, { "AUX10", 248 }, { "AUX11", 249 }, { "AUX12", 250 },
	{ "AUX13", 251 }, { "AUX14", 252 }, { "AUX15", 253 }, { "AUX16", 254 },
	{ "KP_HOME", 182 }, { "KP_UPARROW", 183 }, { "KP_PGUP", 184 },
	{ "KP_LEFTARROW", 185 }, { "KP_5", 186 }, { "KP_RIGHTARROW", 187 },
	{ "KP_END", 188 }, { "KP_DOWNARROW", 189 }, { "KP_PGDN", 190 },
	{ "KP_ENTER", 191 }, { "KP_INS", 192 }, { "KP_DEL", 193 },
	{ "KP_SLASH", 194 }, { "KP_MINUS", 195 }, { "KP_PLUS", 196 },
	{ "KP_NUMLOCK", 197 }, { "KP_STAR", 198 }, { "KP_EQUALS", 199 },
	{ "PAUSE", 153 },
	{ "SEMICOLON", 59 },
	{ "COMMAND", 150 },
	{ "181", 128 }, { "191", 129 }, { "223", 130 }, { "224", 131 },
	{ "225", 132 }, { "228", 133 }, { "229", 134 }, { "230", 135 },
	{ "231", 136 }, { "232", 137 }, { "233", 138 }, { "236", 139 },
	{ "241", 140 }, { "242", 141 }, { "243", 142 }, { "246", 143 },
	{ "248", 144 }, { "249", 145 }, { "250", 146 }, { "252", 147 },
	{ NULL, 0 }
};

/* ---- Key_StringToKeynum  0x0040D4C0 ----  [CONFIRMED] */
int Key_StringToKeynum( const char *str ) {
	keyname_t *kn;

	if ( !str || !str[0] ) {
		return -1;
	}
	if ( !str[1] ) {
		return (unsigned char)str[0];
	}

	if ( str[0] == '0' && str[1] == 'x' && strlen( str ) == 4 ) {
		int n1, n2;

		n1 = str[2];
		if ( n1 >= '0' && n1 <= '9' ) {
			n1 -= '0';
		} else if ( n1 >= 'a' && n1 <= 'f' ) {
			n1 = n1 - 'a' + 10;
		} else {
			n1 = 0;
		}

		n2 = str[3];
		if ( n2 >= '0' && n2 <= '9' ) {
			n2 -= '0';
		} else if ( n2 >= 'a' && n2 <= 'f' ) {
			n2 = n2 - 'a' + 10;
		} else {
			n2 = 0;
		}

		return n1 * 16 + n2;
	}

	for ( kn = keynames; kn->name; kn++ ) {
		if ( !Q_stricmp( str, kn->name ) ) {
			return kn->keynum;
		}
	}
	return -1;
}

/* ---- Key_KeynumToString  0x0040D4E0 ----  [CONFIRMED] */
char *Key_KeynumToString( int keynum ) {
	static char tinystr[5];
	keyname_t *kn;
	int i, j;

	if ( keynum == -1 ) {
		return "<KEY NOT FOUND>";
	}
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return "<OUT OF RANGE>";
	}

	if ( keynum > 32 && keynum < 127 && keynum != '"' && keynum != ';' ) {
		tinystr[0] = (char)toupper( keynum );
		tinystr[1] = 0;
		return tinystr;
	}

	for ( kn = keynames; kn->name; kn++ ) {
		if ( keynum == kn->keynum ) {
			return (char *)kn->name;
		}
	}

	i = keynum >> 4;
	j = keynum & 15;
	tinystr[0] = '0';
	tinystr[1] = 'x';
	tinystr[2] = (char)( i > 9 ? i + 'a' - 10 : i + '0' );
	tinystr[3] = (char)( j > 9 ? j + 'a' - 10 : j + '0' );
	tinystr[4] = 0;
	return tinystr;
}

/* ---- Key_SetBinding  0x0040D6A0 ----  [HIGH] */
void Key_SetBinding( int keynum, const char *binding ) {
	if ( keynum == -1 ) {
		return;
	}

	if ( keys[keynum].binding ) {
		Z_FreeInternal( keys[keynum].binding );
	}

	keys[keynum].binding = (char *)Z_MallocInternal( (int)strlen( binding ) + 1 );
	strcpy( keys[keynum].binding, binding );

	cvar_modifiedFlags |= CVAR_ARCHIVE;
}

/* ---- Key_GetBinding  0x0040D700 ----  [CONFIRMED] */
char *Key_GetBinding( int keynum ) {
	if ( keynum == -1 ) {
		return "";
	}
	return keys[keynum].binding;
}

/* ---- Key_IsDownLocal  no-address ----  [HIGH] */
qboolean Key_IsDownLocal( int keynum ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return qfalse;
	}
	return keys[keynum].down;
}

/* ---- Key_Unbind_f  0x0040D770 ----  [CONFIRMED] */
void Key_Unbind_f( void ) {
	int b;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "unbind <key> : remove commands from a key\n" );
		return;
	}

	b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if ( b == -1 ) {
		Com_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	Key_SetBinding( b, "" );
}

/* ---- Key_UnbindAll_f  0x0040D7C0 ----  [CONFIRMED] */
void Key_UnbindAll_f( void ) {
	int i;

	for ( i = 0; i < MAX_KEYS; i++ ) {
		if ( keys[i].binding ) {
			Key_SetBinding( i, "" );
		}
	}
}

/* ---- Key_Bind_f  0x0040D800 ----  [CONFIRMED] */
void Key_Bind_f( void ) {
	int i, c, b;
	char cmd[MAX_STRING_CHARS];

	c = Cmd_Argc();
	if ( c < 2 ) {
		Com_Printf( "bind <key> [command] : attach a command to a key\n" );
		return;
	}

	b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if ( b == -1 ) {
		Com_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	/* Retail folds the keynum here (0x0040D891); Key_Unbind_f (0x0040D770) deliberately does not. */
	b = tolower( b );

	if ( c == 2 ) {
		if ( keys[b].binding ) {
			Com_Printf( "\"%s\" = \"%s\"\n", Key_KeynumToString( b ), keys[b].binding );
		} else {
			Com_Printf( "\"%s\" is not bound\n", Key_KeynumToString( b ) );
		}
		return;
	}

	cmd[0] = 0;
	for ( i = 2; i < c; i++ ) {
		strcat( cmd, Cmd_Argv( i ) );
		if ( i != ( c - 1 ) ) {
			strcat( cmd, " " );
		}
	}

	Key_SetBinding( b, cmd );
}

/* ---- Key_WriteBindings  0x0040D9A0 ----  [CONFIRMED] */
void Key_WriteBindings( fileHandle_t f ) {
	int i;

	FS_Printf( f, "unbindall\n" );

	for ( i = 0; i < MAX_KEYS; i++ ) {
		if ( keys[i].binding && keys[i].binding[0] ) {
			FS_Printf( f, "bind %s \"%s\"\n", Key_KeynumToString( i ), keys[i].binding );
		}
	}
}

/* ---- Key_Bindlist_f  0x0040DAD0 ----  [CONFIRMED] */
void Key_Bindlist_f( void ) {
	int i;

	for ( i = 0; i < MAX_KEYS; i++ ) {
		if ( keys[i].binding && keys[i].binding[0] ) {
			Com_Printf( "%s \"%s\"\n", Key_KeynumToString( i ), keys[i].binding );
		}
	}
}

/* ---- Key_ClearStatesLocal  no-address ----  [HIGH] */
void Key_ClearStatesLocal( void ) {
	int i;

	anykeydown = 0;
	for ( i = 0; i < MAX_KEYS; i++ ) {
		keys[i].down = qfalse;
		keys[i].repeats = 0;
	}
}

/* ---- CL_KeyEvent  0x0040DC30 ----  VERIFIED */
void CL_KeyEvent( int key, int down, unsigned time ) {
	char *kb;
	char cmd[1024];
	qboolean bypassMenu;

	keys[key].down = (qboolean)( down != 0 );

	if ( down ) {
		keys[key].repeats++;
		if ( keys[key].repeats == 1 ) {
			anykeydown++;
		} else if ( !( cls_keyCatchers & ( KEYCATCH_CONSOLE | KEYCATCH_MESSAGE ) )
		            || key == '`' || key == '~' || key == 27 ) {
			/* retail 0x0040DD4C: auto-repeats are swallowed unless a text field (console or chat) is up, and the toggle/escape keys never repeat */
			return;
		}

		if ( cl_waitForFire && cl_waitForFire->integer ) {
			if ( cls_keyCatchers & KEYCATCH_CONSOLE ) {
				Con_ToggleConsole_f();
			}
			kb = keys[key].binding;
			CL_ClearKeys();
			if ( kb && !Q_stricmp( kb, "+attack" ) ) {
				Cvar_Set2( "cl_waitForFire", "0", qtrue );
			}
			return;
		}
	} else {
		keys[key].repeats = 0;
		anykeydown--;
		if ( anykeydown < 0 ) {
			anykeydown = 0;
		}
	}

	if ( key == '`' || key == '~' ) {
		if ( down ) {
			Con_ToggleConsole_f();
		}
		return;
	}

	if ( down && ( key < 128 || key == 200  )
	     && ( clc_demoplaying || cls_state == CA_CINEMATIC || cls_state == CA_LOGO  )
	     && !cls_keyCatchers ) {
		Cvar_Set2( "nextdemo", "", qtrue );
		key = 27;
	}

	/* Escape is always handled specially, retail 0x0040DDC1-0x0040DE9B -- but only on the way down; an ESC key-up (or any other key) falls through to the CG_CHECK_EXEC_KEY gate below like normal. */
	if ( key == 27 && down ) {
		if ( cls_keyCatchers & KEYCATCH_MESSAGE ) {
			Message_Key( key );
			return;
		}
		if ( cls_keyCatchers & KEYCATCH_CGAME ) {
			cls_keyCatchers &= ~KEYCATCH_CGAME;
			VM_Call( cgvm, CG_EVENT_HANDLING, 0  );
			return;
		}
		if ( cls_keyCatchers & KEYCATCH_UI ) {
			VM_Call( uivm, UI_KEY_EVENT, key, down );
			return;
		}
		if ( cls_state == CA_ACTIVE ) {
			if ( clc_demoplaying ) {
				VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
			} else if ( cl_serverloadwaiting->integer ) {
				VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
			} else {
				VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_INGAME );
			}
			return;
		}
		if ( cls_state > CA_ACTIVE && cls_state <= CA_LOGO ) {
			CL_Disconnect_f();
			MSS_StopSounds( 0 );
			VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
			return;
		}
		/* CoD 1.5 CL_KeyEvent: Escape cancels states 1..3, including
		 * UDP downloads, instead of merely opening the main menu. */
		if ( cls_state >= CA_CONNECTING && cls_state <= CA_CONNECTED ) {
			CL_Disconnect( qtrue );
			if ( com_sv_running->integer ) {
				Cvar_Set( "sv_killserver", "1" );
			}
			return;
		}
		if ( uivm ) {
			VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
		}
		return;
	}

	/* retail 0x0040DEB7-0x0040DECE: cgame gets first refusal on every key that isn't a down-ESC, regardless of catcher state -- a non-zero return consumes the key entirely before catcher dispatch even runs. */
	if ( cgvm && VM_Call( cgvm, CG_CHECK_EXEC_KEY, key, down ) ) {
		return;
	}

	if ( !down ) {
		kb = keys[key].binding;
		if ( kb && kb[0] == '+' ) {
			Com_sprintf( cmd, sizeof( cmd ), "-%s %i %i\n", kb + 1, key, time );
			Cbuf_AddText( cmd );
		}

		if ( cls_keyCatchers & KEYCATCH_UI ) {
			VM_Call( uivm, UI_KEY_EVENT, key, down );
		} else if ( ( cls_keyCatchers & KEYCATCH_CGAME ) && cgvm ) {
			VM_Call( cgvm, CG_KEY_EVENT, key, down );
		}
		return;
	}

	bypassMenu = qfalse;
	if ( cl_bypassMouseInput && cl_bypassMouseInput->integer ) {
		if ( key == 200 || key == 201 || key == 202  ) {
			if ( cl_bypassMouseInput->integer == 1 ) {
				bypassMenu = qtrue;
			}
		} else if ( !UI_checkKeyExec( key ) ) {
			bypassMenu = qtrue;
		}
	}

	if ( cls_keyCatchers & KEYCATCH_CONSOLE ) {
		Console_Key( key );
		return;
	}
	if ( ( cls_keyCatchers & KEYCATCH_UI ) && !bypassMenu ) {
		kb = keys[key].binding;
		if ( kb && !Q_stricmp( "help", kb ) && VM_Call( uivm, 8  ) == 7 ) {
			key = 27;
		}
		VM_Call( uivm, UI_KEY_EVENT, key, down );
		return;
	}
	if ( cls_keyCatchers & KEYCATCH_CGAME ) {
		if ( cgvm ) {
			VM_Call( cgvm, CG_KEY_EVENT, key, down );
		}
		return;
	}
	if ( cls_keyCatchers & KEYCATCH_MESSAGE ) {
		Message_Key( key );
		return;
	}
	if ( cls_state == CA_DISCONNECTED ) {
		Console_Key( key );
		return;
	}

	/* send the bound action, retail 0x0040E073-0x0040E1DA. Unlike the key-up tail above, a plain (non-'+') binding fires unconditionally here since this whole tail only runs on the way down. */
	kb = keys[key].binding;
	if ( !kb ) {
		if ( key >= 200 ) {
			Com_Printf( "%s is unbound, use controls menu to set.\n", Key_KeynumToString( key ) );
		}
		return;
	}

	if ( kb[0] == '+' ) {
		Com_sprintf( cmd, sizeof( cmd ), "%s %i %i\n", kb, key, time );
		Cbuf_AddText( cmd );
	} else {
		Cbuf_AddText( kb );
		Cbuf_AddText( "\n" );
	}
}

/* ---- CL_CharEvent  0x0040E200 ----  VERIFIED */
void CL_CharEvent( int key ) {
	if ( key == '`' || key == '~' ) {
		return;
	}

	if ( cls_keyCatchers & KEYCATCH_CONSOLE ) {
		Field_CharEvent( &g_consoleField, key );
		return;
	}

	if ( cls_keyCatchers & KEYCATCH_UI ) {
		if ( uivm ) {
			VM_Call( uivm, UI_KEY_EVENT, key | K_CHAR_FLAG, qtrue );
		}
		return;
	}

	if ( cls_keyCatchers & KEYCATCH_MESSAGE ) {
		Field_CharEvent( &chatField, key );
		return;
	}

	if ( cls_state == CA_DISCONNECTED ) {
		Field_CharEvent( &g_consoleField, key );
	}
}

#define cl_mouseDx      cl_mouseDx       /* 0x0143A948, int[2] */
#define cl_mouseDy      cl_mouseDy       /* 0x0143A950, int[2] */
#define cl_mouseIndex   cl_mouseIndex       /* 0x0143A958 */
#define cl_joystickAxis cl_joystickAxis       /* 0x0143A95C, int[6] */

/* ---- Key_Shutdown  0x0040E2B0 ----  VERIFIED */
void Key_Shutdown( void ) {
	int i;

	for ( i = 0 ; i < MAX_KEYS ; i++ ) {
		if ( keys[i].binding ) {
			Z_FreeInternal( keys[i].binding );
			keys[i].binding = NULL;
		}
	}
}
