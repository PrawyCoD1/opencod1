/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"

extern int (*dword_1432918)();   /* 0x01432918, indirect call target */
extern int (*dword_143291C)();   /* 0x0143291C, indirect call target */
extern int (*dword_1432934)();   /* 0x01432934, indirect call target */
extern int (*dword_143293C)();   /* 0x0143293C, indirect call target */
extern int (*re_RegisterShader)();   /* 0x01432870, indirect call target */

extern char clc_demoName[64];              /* 0x015EEFC0 */
extern int clc_demofile;                   /* 0x015EF010 */
extern int clc_demorecording;              /* 0x015EF000 */
extern int cls_glconfig_vidHeight;         /* 0x015CA618 */
extern int cls_glconfig_vidWidth;          /* 0x015CA614 */
extern int cmd_argc;                       /* 0x008930F0 */
extern int dword_87A7D4[2048];             /* 0x0087A7D4 */
extern int dword_87C7D0;                   /* 0x0087C7D0 */
extern float flt_87A7D0[2049];             /* 0x0087A7D0 */
extern int whiteShader;                    /* 0x015CA630 */

extern cvar_t *cl_timegraph;
extern cvar_t *cl_debuggraph;
extern cvar_t *cl_graphheight;
extern cvar_t *cl_graphscale;
extern cvar_t *cl_graphshift;
extern cvar_t *cl_debugMove;

extern char *cmd_argv[];                   /* 0x00890BF0 */
extern char empty_string[];                /* 0x00559228 */
extern signed __int32 cl_serverTime;   /* 0x01434A64, 4 bytes */
extern int clc_demoplaying;   /* 0x015EF004, 4 bytes */
extern int com_skelInvalidated2;   /* 0x0140743C, 8 bytes */
extern const char *off_57B4B4[6];   /* 0x0057B4B4, 24 bytes */

extern void (*re_BeginFrame)( int stereoFrame );                /* 0x014328E8 */
extern void (*re_SetColor)( const float *rgba );                /* 0x014328CC */
extern void (*re_DrawStretchPic)( float x, float y, float w, float h,
                                  float s1, float t1, float s2, float t2,
                                  int hShader );                /* 0x014328D0 */
extern void (*dword_14328EC)( const char *ctx, int *frontEndMsec,
                              int *backEndMsec );

extern void Sys_LoadingKeepAlive( void );           /* 0x00463640 win32 */
extern void MSS_StopSounds( int flags );            /* 0x0044FA40 */
extern void CL_CGameRendering( int stereoFrame );   /* 0x00404BC0 cl_cgame_mp.c */
extern void CL_DrawLogo( void );                    /* 0x00411B00 cl_main_mp.c */

extern int Hunk_ClearTempMemory();
extern int j__atol();

void __cdecl SCR_AdjustFrom640( float *x, float *y, float *w, float *h );
void __cdecl SCR_DrawSmallStringExt( const char *str, const float *color, int x, int y );

/* ---- SCR_DrawNamedPic  0x004167C0 ----  VERIFIED */
void __cdecl SCR_DrawNamedPic( const char *picname, int type, float x, float y, float w, float h )
{
  int hShader;

  hShader = re_RegisterShader( picname, type );
  SCR_AdjustFrom640( &x, &y, &w, &h );
  re_DrawStretchPic( x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, hShader );
}

/* ---- SCR_AdjustFrom640  0x00416810 ----  VERIFIED */
void __cdecl SCR_AdjustFrom640( float *x, float *y, float *w, float *h )
{
  float xscale;
  float yscale;

  xscale = (float)( (double)cls_glconfig_vidWidth * 0.0015625 );
  yscale = (float)( (double)cls_glconfig_vidHeight * 0.0020833334 );

  if ( x )
    *x = xscale * *x;
  if ( y )
    *y = yscale * *y;
  if ( w )
    *w = xscale * *w;
  if ( h )
    *h = yscale * *h;
}

/* ---- SCR_FillRect  0x00416870 ----  VERIFIED */
void __cdecl SCR_FillRect( const float *color, float x, float y, float w, float h )
{
  re_SetColor( color );
  SCR_AdjustFrom640( &x, &y, &w, &h );
  re_DrawStretchPic( x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, whiteShader );
  re_SetColor( 0 );
}

/* ---- SCR_DrawPic  0x004168D0 ----  [CONFIRMED] */
void __cdecl SCR_DrawPic(int a1, int a2, int a3, int a4, int a5)
{
  SCR_AdjustFrom640((float *)&a1, (float *)&a2, (float *)&a3, (float *)&a4);
  re_DrawStretchPic( *(float *)&a1, *(float *)&a2, *(float *)&a3, *(float *)&a4,
                     0.0f, 0.0f, 1.0f, 1.0f, a5 );
}

/* ---- SCR_AdjustTo640  0x00416920 ----  VERIFIED */
void __cdecl SCR_AdjustTo640( float *x, float *y, float *w, float *h )
{
  if ( x )
    *x = (float)( 640.0 / (double)cls_glconfig_vidWidth * *x );
  if ( y )
    *y = (float)( 480.0 / (double)cls_glconfig_vidHeight * *y );
  if ( h )
    *h = (float)( 480.0 / (double)cls_glconfig_vidHeight * *h );
  if ( w )
    *w = (float)( 640.0 / (double)cls_glconfig_vidWidth * *w );
}

/* ---- SCR_DrawSmallChar  0x00416980 ----  [HIGH] */
void __cdecl SCR_DrawSmallChar(int ch, int x, int y)
{
  double v5;
  double v6;
  char *v7;
  float v8;
  float v9;
  _DWORD v10[4];
  float sourcea;
  float na;

  v10[0] = 1065353216;
  v5 = 640.0 / (double)cls_glconfig_vidWidth;
  v10[1] = 1065353216;
  v10[2] = 1065353216;
  v10[3] = 1065353216;
  v9 = (double)x * v5;
  v6 = 480.0 / (double)cls_glconfig_vidHeight;
  v8 = ((double)y + 16.0) * v6;
  na = v6 * 0.33333334;
  sourcea = v5 * 8.0;
  v7 = va("%c", ch);
  dword_1432934(LODWORD(v9), LODWORD(v8), 5, LODWORD(na), v10, v7, LODWORD(sourcea), 0, 0);
}

/* ---- SCR_DrawSmallStringExt  0x00416A30 ----  [HIGH] */
void __cdecl SCR_DrawSmallStringExt(const char *str, const float *color, int x, int y)
{
  double v4;
  double v5;
  float v6;
  float v7;
  float v8;
  float v9;

  v4 = 640.0 / (double)cls_glconfig_vidWidth;
  v5 = 480.0 / (double)cls_glconfig_vidHeight;
  v9 = v4 * 8.0;
  v8 = 0.33333334 * v5;
  v7 = ((double)y + 16.0) * v5;
  v6 = (double)x * v4;
  dword_1432934(LODWORD(v6), LODWORD(v7), 5, LODWORD(v8), color, str, LODWORD(v9), 0, 0);
}

/* ---- SCR_DrawConsoleString  0x00416AA0 ----  [HIGH] */
int __cdecl SCR_DrawConsoleString(int maxChars, const float *color, const void *text, int x, int y)
{
  double v5;
  double v6;
  float v8;
  float v9;
  float v10;
  float v11;

  v5 = 640.0 / (double)cls_glconfig_vidWidth;
  v6 = 480.0 / (double)cls_glconfig_vidHeight;
  v11 = v5 * 8.0;
  v10 = 0.33333334 * v6;
  v9 = ((double)y + 16.0) * v6;
  v8 = (double)x * v5;
  return dword_143293C(LODWORD(v8), LODWORD(v9), 5, LODWORD(v10), color, text, LODWORD(v11), maxChars, 0);
}

/* ---- SCR_DrawDemoRecording  0x00416B10 ----  VERIFIED */
void __cdecl SCR_DrawDemoRecording( void )
{
  char string[1024];
  int pos;

  if ( !clc_demorecording )
    return;

  pos = FS_FTell( clc_demofile );
  sprintf( string, "RECORDING %s: %ik", clc_demoName, pos / 1024 );

  SCR_DrawSmallStringExt( string, g_color_table[7], 5, 470 );
}

/* ---- SCR_DebugGraph  0x00416BC0 ----  VERIFIED */
void __cdecl SCR_DebugGraph( float value, int color )
{
  flt_87A7D0[2 * ( dword_87C7D0 & 0x3FF )] = value;
  dword_87A7D4[2 * ( dword_87C7D0 & 0x3FF )] = color;
  dword_87C7D0++;
}

/* ---- SCR_DrawDebugGraph  0x00416BF0 ----  VERIFIED */
void __cdecl SCR_DrawDebugGraph( void )
{
  int a, x, y, w, h;
  float v;
  float fx, fy, fw, fh;

  w = cls_glconfig_vidWidth;
  x = 0;
  y = cls_glconfig_vidHeight;

  re_SetColor( g_color_table[0] );
  fh = (float)*(int *)( cl_graphheight + 32 );
  fw = (float)w;
  fy = (float)( (double)y - fh );
  re_DrawStretchPic( (float) x, fy, fw, fh, 0.0f, 0.0f, 0.0f, 0.0f, whiteShader );
  re_SetColor( 0 );

  for ( a = 0; a < w; a++ )
  {
    v = flt_87A7D0[2 * ( ( dword_87C7D0 - a - 1 ) & 0x3FF )];
    v = (float)( (double)v * *(int *)( cl_graphscale + 32 ) + *(int *)( cl_graphshift + 32 ) );

    if ( v < 0 )
    {
      v = (float)( (double)v + (double)( *(int *)( cl_graphheight + 32 )
              * ( 1 - (int)( (double)v / (double)*(int *)( cl_graphheight + 32 ) ) ) ) );
    }
    h = (int)v % *(int *)( cl_graphheight + 32 );

    fx = (float)( x + w - 1 - a );
    fy = (float)( y - h );
    fh = (float)h;
    re_DrawStretchPic( fx, fy, 1.0f, fh, 0.0f, 0.0f, 0.0f, 0.0f, whiteShader );
  }
}

/* ---- CL_CubemapShotUsage  0x004170B0 ----  [CONFIRMED] */
void __cdecl CL_CubemapShotUsage(int keynum, char *buf, int buflen)
{
  Com_Printf("Syntax: cubemapShot size basefilename [water r0 g0 b0 r90 g90 b90 | fresnel n0 n1]\n");
  Com_Printf("size must be a power of 2 >= 4 and <= 1024\n");
  Com_Printf("screenshots will be written to 'env/basefilename_*.tga'\n");
  Com_Printf("basefilename must not exceed %i chars\n", 40);
  Com_Printf("If 'water' is specified, a diffuse water color cubemap is generated using local lighting.\n");
  Com_Printf("The water has the given colors at the given angles, and blends between them in the middle.\n");
  Com_Printf("If 'fresnel' is specified, the alpha channel of the cubemap contains the reflection factor.\n");
  Com_Printf("n0 and n1 are the index of refraction of the 'air' and 'water' surfaces, respectively.\n");
  Com_Printf("The index of refraction must always be 1 or greater.\n");
  Com_Printf("This is always calculated, and defaults to air-water interface (n0 = 1, n1 = 1.333).\n");
}

/* ---- CL_CubeMapShot_f  0x00417120 ----  [HIGH] */
int __cdecl CL_CubeMapShot_f(float f)
{
  char *v1;
  int v2;
  int result;
  unsigned __int32 v4;
  char *v5;
  char *v6;
  int v7;
  char *v8;
  int v9;
  int v10;
  char *v11;
  char *v12;
  char *v13;
  char *v14;
  char *v15;
  char *v16;
  char *v17;
  int v18;
  const char **v19;
  char *v20;
  char *v21;
  char *v22;
  char *v23;
  const char **v24;
  char *v25;
  int v26;
  char *v27;
  int v28;
  float v29;
  float v30;
  char v31[64];
  unsigned int v32;
  unsigned int retaddr;

  v32 = retaddr ^ _security_cookie;
  if ( !cgvm )
  {
    Com_Printf("must be in a map to use this command\n");
    return result;
  }
  v27 = v1;   /* caller's incoming EBX; CL_CubemapShotUsage never reads it */
  v4 = cmd_argc;
  v26 = v2;   /* caller's incoming ESI; same as v27 */
  if ( cmd_argc < 3 )
    goto LABEL_23;
  v5 = cmd_argv[2];
  v6 = &empty_string;
  if ( (unsigned int)cmd_argc > 2 )
    v6 = cmd_argv[2];
  if ( strlen(v6) > 0x28 )
    goto LABEL_23;
  if ( (unsigned int)cmd_argc <= 2 )
    v5 = &empty_string;
  strcpy(v31, v5);
  v7 = 1;
  v8 = &empty_string;
  if ( v4 > 1 )
    v8 = cmd_argv[1];
  v9 = j__atol(v8);
  v10 = v9;
  if ( v9 < 4 || v9 > 1024 || ((v9 - 1) & v9) != 0 )
    goto LABEL_23;
  switch ( cmd_argc )
  {
    case 10:
      v11 = Cmd_Argv(3);
      if ( !_stricmp(v11, "water") )
      {
        v12 = Cmd_Argv(4);
        atof(v12);
        v13 = Cmd_Argv(5);
        atof(v13);
        v14 = Cmd_Argv(6);
        atof(v14);
        v15 = Cmd_Argv(7);
        atof(v15);
        v16 = Cmd_Argv(8);
        atof(v16);
        v17 = Cmd_Argv(9);
        atof(v17);
        v18 = 1;
        v19 = off_57B4B4;
        do
        {
          v20 = va("env/%s%s.tga", v31, *v19);
          result = dword_143291C(v20);
          ++v19;
          ++v18;
        }
        while ( v19 <= &off_57B4B4[5] );   /* cmp esi, offset off_57B4C8 @0x004172C5 */
        return result;
      }
LABEL_23:
      CL_CubemapShotUsage(v26, v27, v28);
      return result;
    case 6:
      v21 = Cmd_Argv(3);
      if ( _stricmp(v21, "fresnel") )
        goto LABEL_23;
      v22 = Cmd_Argv(4);
      v29 = atof(v22);
      v23 = Cmd_Argv(5);
      v30 = atof(v23);
      if ( (v29 < 1.0) | __UNORDERED__(v29, 1.0) || (v30 < 1.0) | __UNORDERED__(v30, 1.0) )
        goto LABEL_23;
      break;
    case 3:
      break;
    default:
      goto LABEL_23;
  }
  if ( !++com_skelTimeStamp )
    com_skelTimeStamp = 1;
  com_skelInvalidated2 = 1;
  v24 = off_57B4B4;
  do
  {
    re_BeginFrame(0);
    VM_Call(cgvm, 3, cl_serverTime, 0, clc_demoplaying, v7, v10);
    dword_14328EC(NULL, NULL, NULL);
    v25 = va("env/%s%s.tga", v31, *v24);
    dword_1432918(v25);
    ++v24;
    ++v7;
  }
  while ( v24 <= &off_57B4B4[5] );   /* cmp edi, offset off_57B4C8 @0x004173E4 */
  com_skelInvalidated2 = 0;
  Hunk_ClearTempMemory();
  return result;
}

#define CA_DISCONNECTED     0
#define CA_CONNECTING       1
#define CA_CHALLENGING      2
#define CA_CONNECTED        3
#define CA_LOADING          4
#define CA_PRIMED           5
#define CA_ACTIVE           6
#define CA_CINEMATIC        7
#define CA_LOGO             8

#define STEREO_CENTER       0
#define STEREO_LEFT         1
#define STEREO_RIGHT        2

#define KEYCATCH_UI         0x0002

#define UI_REFRESH              5
#define UI_IS_FULLSCREEN        6
#define UI_SET_ACTIVE_MENU      7
#define UI_DRAW_CONNECT_SCREEN  12

#define UIMENU_MAIN         1

extern int cls_state;                   /* 0x0155F2C0 connstate_t */
extern int cls_keyCatchers;             /* 0x0155F2C4 */
extern int cls_realtime;                /* 0x0155F3E0 */
extern int cls_glconfig_vidWidth;       /* 0x015CA614 */
extern int cls_glconfig_vidHeight;      /* 0x015CA618 */

extern int dword_15CA628;               /* 0x015CA628 cls.glconfig.stereoEnabled */
extern int whiteShader;                 /* 0x015CA630 cls.whiteShader */

extern int scr_initialized;             /* 0x01617478 */
extern int scr_updatingScreen;               /* 0x01407440 scr_updatingScreen */
extern int com_skelInvalidated2;        /* 0x0140743C qcommon/common.c */


extern cvar_t *net_showprofile;         /* 0x0165155C qcommon/net_chan_mp.c */
extern int net_iProfilingOn;            /* 0x014073EC ditto */

extern int hunk_temp_permanent;         /* 0x008931C4 hunk_lowUsed */
extern int hunk_temp_temp;              /* 0x008931C8 hunk_lowTemp */
#define HUNK_LOW_PERM   ( *(int *) hunk_temp_permanent )
#define HUNK_LOW_TEMP   ( *(int *) hunk_temp_temp )

extern void SCR_DrawCinematic( void );              /* 0x00408240 cl_cin_mp.c */
extern void Con_DrawConsole( void );                /* 0x0040A1E0 cl_console_mp.c */
extern void SCR_DrawDemoRecording( void );          /* 0x00416B10 cl_scrn_mp.c */
extern void SCR_DrawDebugGraph( void );             /* 0x00416BF0 cl_scrn_mp.c */
extern void CL_Netchan_PrintProfileStats( qboolean bPrintToConsole ); /* 0x00414920 */
extern void SV_Netchan_PrintProfileStats( qboolean bPrintToConsole ); /* 0x0045B800 */

/* ---- SCR_Init  0x00416D40 ----  [CONFIRMED] */
void SCR_Init( void ) {
	cl_timegraph   = Cvar_Get( "timegraph",   "0",  CVAR_CHEAT );
	cl_debuggraph  = Cvar_Get( "debuggraph",  "0",  CVAR_CHEAT );
	cl_graphheight = Cvar_Get( "graphheight", "32", CVAR_CHEAT );
	cl_graphscale  = Cvar_Get( "graphscale",  "1",  CVAR_CHEAT );
	cl_graphshift  = Cvar_Get( "graphshift",  "0",  CVAR_CHEAT );

	scr_initialized = qtrue;
}

/* ---- SCR_DrawScreenField  0x00416DD0 ----  VERIFIED */
static void SCR_DrawScreenField( int stereoFrame ) {
	re_BeginFrame( stereoFrame );

	/* Cgame draws the levelshot during asset registration. The retail
	 * 0x00416E36 full-screen clear can cover it on loading-screen updates. */
	if ( cls_state != CA_ACTIVE && cls_state != CA_LOADING && cls_state != CA_PRIMED ) {
		if ( cls_glconfig_vidWidth * 480 > cls_glconfig_vidHeight * 640 ) {
			re_SetColor( g_color_table[0] );
			re_DrawStretchPic( 0.0f, 0.0f,
			                   (float) cls_glconfig_vidWidth,
			                   (float) cls_glconfig_vidHeight,
			                   0.0f, 0.0f, 0.0f, 0.0f, whiteShader );
			re_SetColor( NULL );
		}
	}

	if ( !uivm ) {
		Com_DPrintf( "draw screen without UI loaded\n" );
		return;
	}

	if ( !VM_Call( uivm, UI_IS_FULLSCREEN ) ) {
		switch ( cls_state ) {

		case CA_CINEMATIC:
			SCR_DrawCinematic();
			break;

		case CA_LOGO:
			CL_DrawLogo();
			if ( cls_state != CA_LOGO ) {
				return;
			}
			break;

		case CA_DISCONNECTED:
			MSS_StopSounds( 0 );
			VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
			break;

		case CA_CONNECTING:
		case CA_CHALLENGING:
		case CA_CONNECTED:
			VM_Call( uivm, UI_REFRESH, cls_realtime );
			VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, qfalse );
			break;

		case CA_LOADING:
		case CA_PRIMED:
			CL_CGameRendering( stereoFrame );
			VM_Call( uivm, UI_REFRESH, cls_realtime );
			VM_Call( uivm, UI_DRAW_CONNECT_SCREEN, qtrue );
			break;

		case CA_ACTIVE:
			CL_CGameRendering( stereoFrame );
			SCR_DrawDemoRecording();
			break;

		default:
			/* the literal at 0x005659B4 carries a leading 0x15, CoD's do-not-localise marker, exactly as the UI_ERROR trap's format string does */
			Com_Error( ERR_FATAL, "\x15SCR_DrawScreenField: bad cls.state" );
			break;
		}
	}

	if ( ( cls_keyCatchers & KEYCATCH_UI ) && uivm ) {
		VM_Call( uivm, UI_REFRESH, cls_realtime );
	}

	Con_DrawConsole();

	if ( ( cl_debuggraph && cl_debuggraph->integer )
	  || ( cl_timegraph  && cl_timegraph->integer  )
	  || ( cl_debugMove  && cl_debugMove->integer  ) ) {
		SCR_DrawDebugGraph();
	}

	if ( net_showprofile->integer && net_iProfilingOn ) {
		if ( net_iProfilingOn == 1 ) {
			CL_Netchan_PrintProfileStats( qfalse );
		} else {
			SV_Netchan_PrintProfileStats( qfalse );
		}
	}
}

/* ---- SCR_UpdateScreen  0x00416FE0 ----  VERIFIED */
void SCR_UpdateScreen( void ) {
	if ( cls_state == CA_LOADING ) {
		Sys_LoadingKeepAlive();
	}

	if ( !scr_initialized ) {
		return;
	}

	if ( !re_BeginFrame ) {
		static qboolean reported = qfalse;
		if ( !reported ) {
			reported = qtrue;
			Com_Printf( "SCR_UpdateScreen: refexport_t is not bound "
			            "(CL_InitRef has not run) -- nothing will draw\n" );
		}
		return;
	}

	if ( scr_updatingScreen ) {
		return;
	}
	scr_updatingScreen = qtrue;

	if ( cls_state == CA_ACTIVE ) {
		if ( ++com_skelTimeStamp == 0 ) {
			com_skelTimeStamp = 1;
		}
		com_skelInvalidated2 = qtrue;
	}

	if ( dword_15CA628 ) {
		SCR_DrawScreenField( STEREO_LEFT );
		SCR_DrawScreenField( STEREO_RIGHT );
	} else {
		SCR_DrawScreenField( STEREO_CENTER );
	}

	if ( com_speeds->integer ) {
		dword_14328EC( NULL, &time_backend, &time_game );
	} else {
		dword_14328EC( NULL, NULL, NULL );
	}

	if ( cls_state == CA_ACTIVE ) {
		com_skelInvalidated2 = qfalse;
		if ( s_hunkData ) {
			HUNK_LOW_TEMP = HUNK_LOW_PERM;
		}
	}

	scr_updatingScreen = qfalse;
}
