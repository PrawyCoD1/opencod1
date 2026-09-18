/*
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

extern void Sys_QueEvent( int time, int type, int value, int value2,
                          int ptrLength, void *ptr );

extern int  ClipCursor( const void *r );
extern int  ShowCursor( int show );
extern int  ReleaseCapture( void );
extern int  midiInClose( void *h );
extern int  GetSystemMetrics( int index );
extern int  GetWindowRect( void *hWnd, void *rect );
extern int  GetCursorPos( void *point );
extern int  SetCursorPos( int x, int y );
extern int  SetCapture( void *hWnd );

extern int  g_wv_sysMsgTime;
#ifndef DEDICATED
extern int  WIN_IsGameWindowActive( void );
#endif

void IN_JoyMove( void );
/* IN_Init (0x00461960) calls all three of these, which are defined below it. */
void IN_StartupJoystick( void );
void IN_StartupMIDI( void );
void MidiInfo_f( void );

/* ---- IN_InitWin32Mouse  0x004615C0 ----  VERIFIED */
void IN_InitWin32Mouse()
{
  ;
}

/* ---- IN_ShutdownWin32Mouse  0x004615D0 ----  VERIFIED */
void IN_ShutdownWin32Mouse()
{
  ;
}

/* ---- IN_ActivateWin32Mouse  0x004615E0 ----  VERIFIED */
int IN_ActivateWin32Mouse( void )
{
  int width;
  int height;
  int result;
  RECT Rect;

  width  = GetSystemMetrics( 0 );
  height = GetSystemMetrics( 1 );
  GetWindowRect( g_wv_hWnd, &Rect );
  if ( Rect.l < 0 )
    Rect.l = 0;
  if ( Rect.t < 0 )
    Rect.t = 0;
  if ( Rect.r >= width )
    Rect.r = width - 1;
  if ( Rect.b >= height - 1 )
    Rect.b = height - 1;
  X = ( Rect.l + Rect.r ) / 2;
  Y = ( Rect.b + Rect.t ) / 2;
  SetCursorPos( X, Y );
  SetCapture( g_wv_hWnd );
  if ( !com_developer->integer )
    ClipCursor( &Rect );
  do
    result = ShowCursor( 0 );
  while ( result >= 0 );
  return result;
}

/* ---- IN_DeactivateWin32Mouse  0x004616B0 ----  VERIFIED */
int IN_DeactivateWin32Mouse()
{
  int result;

  ClipCursor(0);
  ReleaseCapture();
  do
    result = ShowCursor(1);
  while ( result < 0 );
  return result;
}

/* ---- IN_Win32Mouse  0x004616E0 ----  VERIFIED */
void IN_Win32Mouse( int *mx, int *my )
{
  long Point[2];

  GetCursorPos( Point );
  SetCursorPos( X, Y );
  *mx = (int) Point[0] - X;
  *my = (int) Point[1] - Y;
}

/* ---- IN_ActivateMouse  0x00461730 ---- */
int IN_ActivateMouse( void )
{
  int result;

  result = s_wmv_mouseInitialized;
  if ( s_wmv_mouseInitialized )
  {
    result = in_mouse->integer;
    if ( in_mouse->integer )
    {
      result = s_wmv_mouseActive;
      if ( !s_wmv_mouseActive )
      {
        s_wmv_mouseActive = 1;
        return IN_ActivateWin32Mouse();
      }
    }
    else
    {
      s_wmv_mouseActive = 0;
    }
  }
  return result;
}

/* ---- IN_DeactivateMouse  0x00461770 ----  VERIFIED */
int IN_DeactivateMouse()
{
  int result;

  result = s_wmv_mouseInitialized;
  if ( s_wmv_mouseInitialized )
  {
    result = s_wmv_mouseActive;
    if ( s_wmv_mouseActive )
    {
      s_wmv_mouseActive = 0;
      return IN_DeactivateWin32Mouse();
    }
  }
  return result;
}

/* ---- IN_StartupMouse  0x004617A0 ---- */
void IN_StartupMouse( void )
{
  s_wmv_mouseInitialized = 0;
  if ( in_mouse->integer )
    s_wmv_mouseInitialized = 1;
  else
    Com_Printf("Mouse control not active.\n");
}

/* ---- IN_MouseEvent  0x004617D0 ---- */
int IN_MouseEvent( int mstate )
{
  int i;

  if ( s_wmv_mouseInitialized )
  {
    for ( i = 0; i < 3; ++i )
    {
      if ( ( mstate & ( 1 << i ) ) != 0 && ( s_wmv_oldButtonState & ( 1 << i ) ) == 0 )
        Sys_QueEvent( g_wv_sysMsgTime, 1 , i + 200 , 1, 0, 0 );
      if ( ( mstate & ( 1 << i ) ) == 0 && ( s_wmv_oldButtonState & ( 1 << i ) ) != 0 )
        Sys_QueEvent( g_wv_sysMsgTime, 1 , i + 200 , 0, 0, 0 );
    }
    s_wmv_oldButtonState = mstate;
  }
  return 0;
}

/* ---- IN_MouseMove  0x00461850 ---- */
int IN_MouseMove( void )
{
  long Point[2];
  int dx, dy;

  GetCursorPos( Point );
  SetCursorPos( X, Y );
  dx = (int) Point[0] - X;
  dy = (int) Point[1] - Y;
  if ( dx || dy )
    Sys_QueEvent( 0, 3 , dx, dy, 0, 0 );
  return dx;
}

/* ---- IN_Shutdown  0x00461910 ----  VERIFIED */
void IN_Shutdown()
{
  if ( s_wmv_mouseInitialized )
  {
    if ( s_wmv_mouseActive )
    {
      s_wmv_mouseActive = 0;
      IN_DeactivateWin32Mouse();
    }
  }
  if ( hmi )
    midiInClose((void *)hmi);
  memset( s_midiInfo, 0, sizeof( s_midiInfo ) );
  memset( pmic, 0, sizeof( pmic ) );
  hmi = 0;
  Cmd_RemoveCommand("midiinfo");
}

/* ---- IN_Init  0x00461960 ----  VERIFIED */
void IN_Init( void )
{
  in_midi          = Cvar_Get( "in_midi",          "0",    CVAR_ARCHIVE );
  in_midiport      = Cvar_Get( "in_midiport",      "1",    CVAR_ARCHIVE );
  in_midichannel   = Cvar_Get( "in_midichannel",   "1",    CVAR_ARCHIVE );
  in_mididevice    = Cvar_Get( "in_mididevice",    "0",    CVAR_ARCHIVE );
  Cmd_AddCommand( "midiinfo", (xcommand_t) MidiInfo_f );
  in_mouse         = Cvar_Get( "in_mouse",         "1",    CVAR_ARCHIVE | CVAR_LATCH );
  in_joystick      = Cvar_Get( "in_joystick",      "0",    CVAR_ARCHIVE | CVAR_LATCH );
  in_joyBallScale  = Cvar_Get( "in_joyBallScale",  "0.02", CVAR_ARCHIVE );
  in_debugjoystick = Cvar_Get( "in_debugjoystick", "0",    CVAR_TEMP );
  joy_threshold    = Cvar_Get( "joy_threshold",    "0.15", CVAR_ARCHIVE );

  /* the shared tail at 0x004618B0 */
  IN_StartupMouse();
  IN_StartupJoystick();
  IN_StartupMIDI();
  in_mouse->modified = qfalse;
  in_joystick->modified = qfalse;
}

/* ---- IN_Activate  0x00461A50 ----  VERIFIED */
int __cdecl IN_Activate(int result)
{
  in_appactive = result;
  if ( !result )
  {
    result = s_wmv_mouseInitialized;
    if ( s_wmv_mouseInitialized )
    {
      result = s_wmv_mouseActive;
      if ( s_wmv_mouseActive )
      {
        s_wmv_mouseActive = 0;
        return IN_DeactivateWin32Mouse();
      }
    }
  }
  return result;
}

/* ---- IN_Frame  0x00461A80 ----  VERIFIED */
int IN_Frame()
{
#ifdef DEDICATED
  /* Dedicated servers have no game window or local input to capture. */
  return 0;
#else
  int result;
  cvar_t *Var;

  /* A background/minimized window must never capture or recenter the cursor,
   * even if its activation message has not updated in_appactive yet. */
  if ( !in_appactive || !WIN_IsGameWindowActive() )
    return IN_DeactivateMouse();

  IN_JoyMove();
  result = s_wmv_mouseInitialized;
  if ( s_wmv_mouseInitialized )
  {
    if ( ((cls_keyCatchers & 1) == 0
       || (Var = Cvar_FindVar("r_fullscreen")) != 0 && !((Var->value == 0.0) | __UNORDERED__(Var->value, 0.0)))
      && in_appactive )
    {
      IN_ActivateMouse();
      return s_wmv_mouseActive ? IN_MouseMove() : 0;
    }
    else
    {
      result = s_wmv_mouseInitialized;
      if ( s_wmv_mouseInitialized )
      {
        result = s_wmv_mouseActive;
        if ( s_wmv_mouseActive )
        {
          s_wmv_mouseActive = 0;
          return IN_DeactivateWin32Mouse();
        }
      }
    }
  }
  return result;
#endif
}

/* ---- IN_ClearStates  0x00461AF0 ----  VERIFIED */
void IN_ClearStates()
{
  s_wmv_oldButtonState = 0;
}

extern UINT __stdcall joyGetNumDevs( void );
extern UINT __stdcall joyGetPosEx( UINT uJoyID, void *pji );
extern UINT __stdcall joyGetDevCapsA( UINT uJoyID, void *pjc, UINT cbjc );

typedef struct {
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwXpos;
	unsigned int dwYpos;
	unsigned int dwZpos;
	unsigned int dwRpos;
	unsigned int dwUpos;
	unsigned int dwVpos;
	unsigned int dwButtons;
	unsigned int dwButtonNumber;
	unsigned int dwPOV;
	unsigned int dwReserved1;
	unsigned int dwReserved2;
} joyInfoEx_t;

typedef struct {
	unsigned short wMid;
	unsigned short wPid;
	char           szPname[32];
	unsigned int   wXmin;
	unsigned int   wXmax;
	unsigned int   wYmin;
	unsigned int   wYmax;
	unsigned int   wZmin;
	unsigned int   wZmax;
	unsigned int   wNumButtons;
	unsigned int   wPeriodMin;
	unsigned int   wPeriodMax;
	unsigned int   wRmin;
	unsigned int   wRmax;
	unsigned int   wUmin;
	unsigned int   wUmax;
	unsigned int   wVmin;
	unsigned int   wVmax;
	unsigned int   wCaps;
	unsigned int   wMaxAxes;
	unsigned int   wNumAxes;
	unsigned int   wMaxButtons;
	char           szRegKey[32];
	char           szOEMVxD[260];
} joyCaps_t;

#define JOY_JI   (*(joyInfoEx_t *) pji)   /* joy.ji at 0x008E26CC */
#define JOY_JC   (*(joyCaps_t   *) pjc)   /* joy.jc at 0x008E2530 */

static int joyDirectionKeys[16] = {
	156, 157,
	154, 155,
	222, 223, 224, 225, 226, 227,
	228, 229, 230, 231, 232, 233
};

/* ---- IN_StartupJoystick  0x00461B00 ----  VERIFIED */
void IN_StartupJoystick( void )
{
  int  numdevs;
  UINT mmr;

  s_wmv_joyAvail = 0;

  if ( !in_joystick->integer )
    return;

  if ( ( numdevs = joyGetNumDevs() ) == 0 )
  {
    Com_DPrintf( "joystick not found -- driver not present\n" );
    return;
  }

  mmr = 0;
  for ( uJoyID = 0; uJoyID < numdevs; uJoyID++ )
  {
    memset( &JOY_JI, 0, sizeof( joyInfoEx_t ) );
    JOY_JI.dwSize  = sizeof( joyInfoEx_t );
    JOY_JI.dwFlags = 0x400;

    if ( ( mmr = joyGetPosEx( uJoyID, &JOY_JI ) ) == 0 )
      break;
  }

  if ( mmr != 0 )
  {
    Com_Printf( "joystick not found -- no valid joysticks (%x)\n", mmr );
    return;
  }

  memset( &JOY_JC, 0, sizeof( joyCaps_t ) );
  if ( ( mmr = joyGetDevCapsA( uJoyID, &JOY_JC, sizeof( joyCaps_t ) ) ) != 0 )
  {
    Com_Printf( "joystick not found -- invalid joystick capabilities (%x)\n", mmr );
    return;
  }

  Com_DPrintf( "Joystick found.\n" );
  Com_DPrintf( "Pname: %s\n", JOY_JC.szPname );
  Com_DPrintf( "OemVxD: %s\n", JOY_JC.szOEMVxD );
  Com_DPrintf( "RegKey: %s\n", JOY_JC.szRegKey );
  Com_DPrintf( "Numbuttons: %i / %i\n", JOY_JC.wNumButtons, JOY_JC.wMaxButtons );
  Com_DPrintf( "Axis: %i / %i\n", JOY_JC.wNumAxes, JOY_JC.wMaxAxes );
  Com_DPrintf( "Caps: 0x%x\n", JOY_JC.wCaps );
  if ( JOY_JC.wCaps & 0x10 )
    Com_DPrintf( "HASPOV\n" );
  else
    Com_DPrintf( "no POV\n" );

  s_wmv_joyOldButtonState = 0;
  s_wmv_joyOldPovState = 0;

  s_wmv_joyAvail = 1;
}

/* ---- JoyToF  0x00461CA0 ----  VERIFIED */
double __cdecl JoyToF(int a1)
{
  double result;

  result = (double)(a1 - 0x8000) * ( 1.0 / 32768.0 );
  if ( (result < -1.0) | __UNORDERED__(result, -1.0) )
    return -1.0;
  if ( result > 1.0 )
    return 1.0;
  return result;
}

/* ---- JoyToI  0x00461CF0 ----  VERIFIED */
int __cdecl JoyToI(int a1)
{
  return a1 - 0x8000;
}

/* ---- IN_JoyMove  0x00461D00 ----  VERIFIED */
void IN_JoyMove( void )
{
  float        fAxisValue;
  int          i;
  unsigned int buttonstate, povstate;
  int          x, y;

  if ( !s_wmv_joyAvail )
    return;

  memset( &JOY_JI, 0, sizeof( joyInfoEx_t ) );
  JOY_JI.dwSize  = sizeof( joyInfoEx_t );
  JOY_JI.dwFlags = 0xFF;

  if ( joyGetPosEx( uJoyID, &JOY_JI ) != 0 )
    return;

  if ( in_debugjoystick->integer )
  {
    Com_Printf( "%8x %5i %5.2f %5.2f %5.2f %5.2f %6i %6i\n",
                JOY_JI.dwButtons,
                JOY_JI.dwPOV,
                JoyToF( JOY_JI.dwXpos ), JoyToF( JOY_JI.dwYpos ),
                JoyToF( JOY_JI.dwZpos ), JoyToF( JOY_JI.dwRpos ),
                JoyToI( JOY_JI.dwUpos ), JoyToI( JOY_JI.dwVpos ) );
  }

  buttonstate = JOY_JI.dwButtons;
  for ( i = 0; (unsigned int) i < JOY_JC.wNumButtons; i++ )
  {
    if ( ( buttonstate & ( 1 << i ) ) && !( s_wmv_joyOldButtonState & ( 1 << i ) ) )
      Sys_QueEvent( g_wv_sysMsgTime, 1 , 207  + i, 1, 0, 0 );
    if ( !( buttonstate & ( 1 << i ) ) && ( s_wmv_joyOldButtonState & ( 1 << i ) ) )
      Sys_QueEvent( g_wv_sysMsgTime, 1 , 207  + i, 0, 0, 0 );
  }
  s_wmv_joyOldButtonState = buttonstate;

  povstate = 0;

  for ( i = 0; (unsigned int) i < JOY_JC.wNumAxes && i < 4; i++ )
  {
    fAxisValue = (float) JoyToF( ( &JOY_JI.dwXpos )[i] );

    if ( fAxisValue < -joy_threshold->value )
      povstate |= ( 1 << ( i * 2 ) );
    else if ( fAxisValue > joy_threshold->value )
      povstate |= ( 1 << ( i * 2 + 1 ) );
  }

  if ( JOY_JC.wCaps & 0x10 )
  {
    if ( JOY_JI.dwPOV != 0xFFFF )
    {
      if ( JOY_JI.dwPOV == 0 )
        povstate |= 1 << 12;
      if ( JOY_JI.dwPOV == 0x4650 )
        povstate |= 1 << 13;
      if ( JOY_JI.dwPOV == 0x2328 )
        povstate |= 1 << 14;
      if ( JOY_JI.dwPOV == 0x6978 )
        povstate |= 1 << 15;
    }
  }

  for ( i = 0; i < 16; i++ )
  {
    if ( ( povstate & ( 1 << i ) ) && !( s_wmv_joyOldPovState & ( 1 << i ) ) )
      Sys_QueEvent( g_wv_sysMsgTime, 1 , joyDirectionKeys[i], 1, 0, 0 );
    if ( !( povstate & ( 1 << i ) ) && ( s_wmv_joyOldPovState & ( 1 << i ) ) )
      Sys_QueEvent( g_wv_sysMsgTime, 1 , joyDirectionKeys[i], 0, 0, 0 );
  }
  s_wmv_joyOldPovState = povstate;

  if ( JOY_JC.wNumAxes >= 6 )
  {
    x = (int) ( JoyToI( JOY_JI.dwUpos ) * in_joyBallScale->value );
    y = (int) ( JoyToI( JOY_JI.dwVpos ) * in_joyBallScale->value );
    if ( x || y )
      Sys_QueEvent( g_wv_sysMsgTime, 3 , x, y, 0, 0 );
  }
}

extern UINT __stdcall midiInGetNumDevs( void );
extern UINT __stdcall midiInGetDevCapsA( UINT uDeviceID, void *pmic, UINT cbmic );
extern UINT __stdcall midiInOpen( HMIDIIN *phmi, UINT uDeviceID, unsigned long dwCallback,
                                  unsigned long dwInstance, unsigned long fdwOpen );
extern UINT __stdcall midiInStart( HMIDIIN hmi );

typedef struct {
	unsigned short wMid;
	unsigned short wPid;
	unsigned int   vDriverVersion;
	char           szPname[32];
	unsigned int   dwSupport;
} midiInCaps_t;

/* cod1_globals.h gives `pmic` as a flat unsigned char[352]; retail's type is MIDIINCAPSA[8] at 0x008E23B4. One cast, in one place, rather than eight offset expressions. */
#define MIDI_CAPS   ((midiInCaps_t *) pmic)

#define MIDI_NUM_DEVICES   (*(int *) s_midiInfo)

/* ---- MIDI_NoteOff  0x00462010 ----  VERIFIED */
void MIDI_NoteOff( int note )
{
  int qkey;

  qkey = note + 179;

  if ( qkey > 255 || qkey < 239 )
    return;

  Sys_QueEvent( g_wv_sysMsgTime, 1 , qkey, 0 , 0, 0 );
}

/* ---- MIDI_NoteOn  0x00462040 ----  VERIFIED */
void MIDI_NoteOn( int note, int velocity )
{
  int qkey;

  if ( velocity == 0 )
    MIDI_NoteOff( note );

  qkey = note + 179;

  if ( qkey > 255 || qkey < 239 )
    return;

  Sys_QueEvent( g_wv_sysMsgTime, 1 , qkey, 1 , 0, 0 );
}

/* ---- MidiInProc  0x004620A0 ----  VERIFIED */
void __stdcall MidiInProc( HMIDIIN hMidiIn, UINT uMsg, unsigned long dwInstance,
                           unsigned long dwParam1, unsigned long dwParam2 )
{
  int message;

  (void) hMidiIn;
  (void) dwInstance;
  (void) dwParam2;

  if ( uMsg != 0x3C3  )
    return;

  message = dwParam1 & 0xFF;

  if ( ( message & 0xF0 ) == 0x90 )
  {
    if ( ( ( message & 0x0F ) + 1 ) == in_midichannel->integer )
      MIDI_NoteOn( ( dwParam1 & 0xFF00 ) >> 8, ( dwParam1 & 0xFF0000 ) >> 16 );
  }
  else if ( ( message & 0xF0 ) == 0x80 )
  {
    if ( ( ( message & 0x0F ) + 1 ) == in_midichannel->integer )
      MIDI_NoteOff( ( dwParam1 & 0xFF00 ) >> 8 );
  }
}

/* ---- MidiInfo_f  0x00462110 ----  VERIFIED */
void MidiInfo_f( void )
{
  int i;
  const char *enabled[2];

  enabled[0] = "disabled";
  enabled[1] = "enabled";

  Com_Printf( "\nMIDI control:       %s\n", enabled[ in_midi->integer != 0 ] );
  Com_Printf( "port:               %d\n", in_midiport->integer );
  Com_Printf( "channel:            %d\n", in_midichannel->integer );
  Com_Printf( "current device:     %d\n", in_mididevice->integer );
  Com_Printf( "number of devices:  %d\n", MIDI_NUM_DEVICES );

  for ( i = 0; i < MIDI_NUM_DEVICES; i++ )
  {
    if ( i == Cvar_VariableValue( "in_mididevice" ) )
      Com_Printf( "***" );
    else
      Com_Printf( "..." );

    Com_Printf( "device %2d:       %s\n", i, MIDI_CAPS[i].szPname );
    Com_Printf( "...manufacturer ID: 0x%hx\n", MIDI_CAPS[i].wMid );
    Com_Printf( "...product ID:      0x%hx\n", MIDI_CAPS[i].wPid );
    Com_Printf( "\n" );
  }
}

/* ---- IN_StartupMIDI  0x00462250 ----  VERIFIED */
void IN_StartupMIDI( void )
{
  int i;

  if ( !Cvar_VariableValue( "in_midi" ) )
    return;

  MIDI_NUM_DEVICES = midiInGetNumDevs();

  for ( i = 0; i < MIDI_NUM_DEVICES; i++ )
    midiInGetDevCapsA( i, &MIDI_CAPS[i], sizeof( midiInCaps_t ) );

  if ( midiInOpen( (HMIDIIN *) &hmi, in_mididevice->integer,
                   (unsigned long) MidiInProc, 0, 0x30000  ) )
  {
    /* Retail bug reproduced: the whole 44-byte MIDIINCAPS goes into the %s, not .szPname. */
    Com_Printf( "WARNING: could not open MIDI device %d: '%s'\n",
                in_mididevice->integer,
                MIDI_CAPS[ (int) in_mididevice->value ] );
    return;
  }

  midiInStart( (HMIDIIN) hmi );
}

/* ---- IN_ShutdownMIDI  0x00462320 ----  VERIFIED */
int IN_ShutdownMIDI()
{
  int result;

  if ( hmi )
    midiInClose((void *)hmi);
  result = 0;
  memset( s_midiInfo, 0, sizeof( s_midiInfo ) );
  memset( pmic, 0, sizeof( pmic ) );
  hmi = 0;
  return result;
}
