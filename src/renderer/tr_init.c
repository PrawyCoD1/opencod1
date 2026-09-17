/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#define aSun aSun__cod1_hdr   /* cod1_globals.h types it int; real char[4] "sun" below */
#include "../qcommon/cod1_globals.h"
#undef aSun

/* === Merged from renderer/tr_init_rdata.c (retail linked it as tr_*.obj .rdata/.data). === */
/* ==========================================================================
 * reverseBitsTableNeedsInitialization        0x005716BC        retail 01 00 00 00        value 1
 * ==========================================================================
 *
 * The one-time-init latch for the 256-entry byte-reversal LUT at reverseBitsTable,
 * built inside `reverse_bits` (0x004B4DE0):
 *
 *     004B4DE1  mov  ecx, reverseBitsTableNeedsInitialization
 *     004B4DE9  cmp  ecx, eax            (eax = 0)
 *     004B4DEB  jz   loc_4B4E65          -- straight to the lookup
 *     004B4DED  mov  reverseBitsTableNeedsInitialization, eax   -- clear the latch, build the table
 *     ...
 *     004B4E5E  cmp  eax, 100h / jl loc_4B4E00
 *     004B4E65  mov  eax, [esp+4+arg_0]  -- four LUT indexes, one per byte
 *
 * The guard runs the builder exactly once, on the FIRST call, so the flag has
 * to arrive as 1.  Arriving as 0 the branch is taken immediately,
 * reverseBitsTable stays all zeros, and reverse_bits returns 0 for every
 * input for the life of the process.
 *
 * reverse_bits is the Z term of the light-visibility cache hash --
 * R_LightVisHash (0x004B53D0) and the copy inlined into
 * R_GetCachedVisibility both compute
 *
 *     bucket = (3137 * y - 3133 * x + (unsigned short) reverse_bits( z )) & 0x1FFF
 *
 * With the Z term identically zero, z drops out of the hash and every vertical
 * column of the world collapses onto one bucket.  Answers stay correct (the
 * full key is compared on hit) but the table thrashes and the eviction path
 * runs on nearly every query.
 *
 * 0x005716BC is the dword immediately before r_vidModes (0x005716C0) in
 * .data, which is why it is defined with the tr_init objects.
 */
int reverseBitsTableNeedsInitialization = 1;

/* ==========================================================================
 * screenShotTGA_lastNumber        0x00571794        retail ff ff ff ff        value -1
 * screenShotJPEG_lastNumber        0x00571798        retail ff ff ff ff        value -1
 * ==========================================================================
 *
 * The two `lastNumber` statics, one for TGA and one for JPEG screenshots.
 * renderer/tr_init.c:1490 (R_ScreenShot_f, 0x004B2670) and :1560
 * (R_ScreenShotJPEG_f, 0x004B2740) both open with
 *
 *     if ( lastNumber == -1 ) { lastNumber = 0; <scan for the first free name> }
 *
 * so -1 is the sentinel that triggers the initial directory scan.  Zeroed, the
 * scan never runs and the first screenshot of every session overwrites
 * shot0000.tga / shot0000.jpg.
 *
 * In retail these two dwords ARE dword_571790[1] and dword_571790[2] -- one
 * 16-byte row under three overlapping names.  Here they are three separate C
 * objects: client_mp/cl_refstorage.c gives `dword_571790` a strong definition
 * as a 24-row surface-type table whose row 0 is `{ (const char *)13, -1, -1, 0 }`,
 * i.e. it carries these same two -1s at the same offsets.  dword_571790 is
 * read only as `[0]` for the mode count and as `[4 * n]` for the surface
 * table, and the two lastNumbers only through their own names, so splitting
 * the storage is observationally identical, and R_ScreenShot_f's
 * `lastNumber++` never writes into a live table row.
 */
int screenShotTGA_lastNumber = -1;
int screenShotJPEG_lastNumber = -1;

/* ==========================================================================
 * aSun                0x00557A48        retail 73 75 6e 00        "sun"
 * ==========================================================================
 *
 * A .rdata STRING, declared in cod1_globals.h as `extern int aSun;` (its only
 * reference is `push offset aSun`, which carries no width).
 * renderer/tr_init.c:2167 (R_Register, 0x004B3300) is the sole reader:
 *
 *     r_sunsprite_shader = ri_Cvar_Get( "r_sunsprite_shader", &aSun, 0 );
 *
 * `&aSun` on a four-byte COMMON int is a pointer to four zero bytes, so the
 * cvar would default to "" and the sun sprite shader would never be
 * registered on a map that does not set it explicitly.  Four bytes is
 * exactly the retail extent -- 0x00557A4C begins the next literal,
 * "r_suntest".
 *
 * cod1_globals.h keeps `extern int aSun;`, so the call site still spells it
 * `&aSun` and still yields an `int *`, pointing at the real bytes.  Same
 * trick as cls_glconfig in cl_refstorage.c, and the reason this unit must
 * not include cod1_globals.h.
 */
char aSun[4] = "sun";
#include "tr_shaderregistry.h"
#include "tr_gl_types.h"
#include "tr_tess.h"

void GfxInfo_f( void );
extern unsigned char s_gammatable[256];
extern unsigned char s_intensitytable[256];

extern int FFT_Init();
extern int GL_TextureMode();
extern void GLimp_Init( void );
extern void GLimp_Shutdown( void );
extern int RB_ExecuteRenderCommands();
extern int RB_ShowImages();
extern int RE_AddCoronaToScene();
extern int RE_AddLightToScene();
extern int RE_AddPlume();
extern int RE_AddPolyToScene();
extern int RE_AddPolysToScene();
extern int RE_AddRefEntityToScene();
extern int RE_BeginFrame();
extern int RE_BeginRegistration();
extern int RE_BlendSavedScreen();
extern int RE_ClearFlares();
extern int RE_ClearScene();
extern int RE_DrawQuadPic();
extern int RE_EndFrame();
extern int RE_FinishLoadingModels();
extern int RE_GetFarPlaneDist();
extern int RE_GetIgnorePrecacheErrors();
extern int RE_GetImageMemory();
extern int RE_GetShaderFromModel();
extern int RE_GetShaderName();
extern int RE_GetXModelByHandle();
extern int RE_LoadWorldMap();
extern int RE_MarkFragments();
extern int RE_LocateDebugLines();
extern int RE_LocateDebugStrings();
extern int RE_PickShader();
extern int RE_RegisterFont();
extern int RE_RegisterModel();
extern int RE_RegisterShader();
extern int RE_RegisterShaderNoMip();
extern int RE_RenderScene();
extern int RE_RestoreFogState();
extern int RE_SaveFogState();
extern int RE_SaveScreen();
extern int RE_SetColor();
extern int RE_SetIgnorePrecacheErrors();
extern int RE_StretchPic();
extern int RE_StretchPicGradient();
extern int RE_StretchPicRotate();
extern int RE_StretchRaw();
extern int RE_Text_ConsolePaint();
extern int RE_Text_ConsoleWidth();
extern int RE_Text_Height();
extern int RE_Text_Paint();
extern int RE_Text_PaintWithCursor();
extern int RE_Text_Width();
extern int RE_TrackStatistics();
extern int RE_UploadCinematic();
extern int R_CreateBuiltinImages();
extern int R_DeleteBuffersARB();
extern int R_DeleteTextures();
extern int R_DeleteVertexPrograms();
extern int R_FreeImageAllocations();
extern int R_GammaCorrect();
extern int R_GetEntityToken();
extern int R_ImageList_f();
extern int R_InitAllocators();
extern int R_InitDebug();
extern int R_InitShaders();
extern int R_LoadSun_f();
void R_MemInfo_f( void );
extern int R_ModelBounds();
extern int R_RefreshOptimizedWorldSurfaces_ARB();
extern int R_RefreshStaticModels_ARB();
extern int R_RefreshXModels_ARB();
extern int R_ResetImageAllocations();
extern int R_SaveLightVisHistory();
extern int R_SaveSun_f();
extern int R_SetColorMappings();
extern int R_SetFog();
extern int R_SetHwLightGlobals();
extern int R_ShaderList_f();
void R_ShutdownAllocators( void );
extern int R_ShutdownDebug();
extern int R_ShutdownStaticModels();
extern int R_StaticModelCacheFlush_f();
extern int R_StaticModelCacheStats_f();
extern int R_SunHelp_f();
extern int R_VC_stats_f();
int __cdecl SaveJPG( int fileName, int quality, int width, unsigned int height,
                     int flipped, int pixels );
extern int RE_SetFarPlaneDist();

/* ---- AssertCvarRange  0x004B0A30 ----  VERIFIED */
void __cdecl AssertCvarRange(int a1, const char **a2, float a3, int a4)
{
  char *v4;
  char *v5;
  char *v6;

  if ( a1 )
  {
    if ( (const char *)(unsigned __int64)*((float *)a2 + 7) != a2[8] )
    {
      ri_Printf(2, "WARNING: cvar '%s' must be integral (%f)\n", *a2, *((float *)a2 + 7));
      v4 = va("%d", a2[8]);
      ri_Cvar_Set(*a2, v4);
    }
  }
  if ( (*((float *)a2 + 7) < (double)a3) | __UNORDERED__(*((float *)a2 + 7), a3) )
  {
    ri_Printf(2, "WARNING: cvar '%s' out of range (%f < %f)\n", *a2, *((float *)a2 + 7), a3);
    v5 = va("%f", a3);
    ri_Cvar_Set(*a2, v5);
  }
  else if ( *((float *)a2 + 7) > (double)*(float *)&a4 )
  {
    ri_Printf(2, "WARNING: cvar '%s' out of range (%f > %f)\n", *a2, *((float *)a2 + 7), *(float *)&a4);
    v6 = va("%f", *(float *)&a4);
    ri_Cvar_Set(*a2, v6);
  }
}

/* ---- GL_SetupVBO  0x004B0B30 ----  VERIFIED */
__int64 GL_SetupVBO()
{
  __int64 result; // rax

  tr_vboInterleaved = r_vbo_interleave->integer != 0;
  LODWORD(result) = r_vbo_smc_static_draw;
  HIDWORD(result) = r_vbo_stream_draw->integer != 0;
  tr_vboStreamDraw = HIDWORD(result);
  tr_vboUsage = 4 * (r_vbo_smc_static_draw == 0) + 35044;
  return result;
}

/* ---- InitOpenGL  0x004B0B80 ----  VERIFIED */
__int64 InitOpenGL()
{
  char *v0;
  _BYTE *v1;
  char v2;
  _BYTE *v3;
  int v5;
  int integer;
  __int64 result; // rax
  int v8; // [esp+10h] [ebp-408h] BYREF
  _BYTE v9[1024]; // [esp+14h] [ebp-404h] BYREF
  unsigned int v10;
  unsigned int retaddr;

  v10 = retaddr ^ _security_cookie;
  if ( !dwStyle )
  {
    GLimp_Init();
    v0 = (char *)glConfig_renderer_string;
    v1 = &v9[-glConfig_renderer_string];
    do
    {
      v2 = *v0;
      v0[(_DWORD)v1] = *v0;
      ++v0;
    }
    while ( v2 );
    v3 = v9;
    if ( v9[0] )
    {
      do
        *v3 = tolower((char)*v3);
      while ( *++v3 );
    }
    qglGetIntegerv(3379, &v8);
    v5 = v8;
    glConfig_maxTextureSize = v8;
    if ( v8 > 0 )
    {
      if ( v8 <= 0x2000 )
      {
LABEL_11:
        integer = r_maxTextureSize->integer;
        if ( integer >= 1024 && v5 > integer )
        {
          do
          {
            v5 >>= 1;
            glConfig_maxTextureSize = v5;
          }
          while ( v5 > r_maxTextureSize->integer );
        }
        qglGetIntegerv(3377, &v8);
        glConfig_maxLights = v8;
        if ( v8 > 0 )
        {
          if ( v8 > 16 )
            glConfig_maxLights = 16;
        }
        else
        {
          glConfig_maxLights = 8;
        }
        goto LABEL_18;
      }
      v5 = 0x2000;
    }
    else
    {
      v5 = 0;
    }
    glConfig_maxTextureSize = v5;
    goto LABEL_11;
  }
LABEL_18:
  GfxInfo_f();
  glState_numEnabledLights = 0;
  GL_SetDefaultState();
  tr_vboInterleaved = r_vbo_interleave->integer != 0;
  LODWORD(result) = r_vbo_smc_static_draw;
  HIDWORD(result) = r_vbo_stream_draw->integer != 0;
  tr_vboStreamDraw = HIDWORD(result);
  tr_vboUsage = 4 * (r_vbo_smc_static_draw == 0) + 35044;
  return result;
}

/* ---- GL_CheckErrors  0x004B0CF0 ----  [HIGH] */
int __cdecl GL_CheckErrors( const char *location )
{
	int		err;
	char	s[64];

	err = qglGetError();
	if ( !err ) {
		return err;
	}
	if ( r_ignoreGLErrors->integer ) {
		return err;
	}

	switch ( err ) {
	case 0x0500:	strcpy( s, "GL_INVALID_ENUM" );			break;
	case 0x0501:	strcpy( s, "GL_INVALID_VALUE" );		break;
	case 0x0502:	strcpy( s, "GL_INVALID_OPERATION" );	break;
	case 0x0503:	strcpy( s, "GL_STACK_OVERFLOW" );		break;
	case 0x0504:	strcpy( s, "GL_STACK_UNDERFLOW" );		break;
	case 0x0505:	strcpy( s, "GL_OUT_OF_MEMORY" );		break;
	default:		Com_sprintf( s, sizeof( s ), "%i", err );	break;
	}

	ri_Error( 0, "\025GL_CheckErrors: %s (location = %s)", s, location );
	return err;
}

/* ---- R_GetModeInfo  0x004B0EC0 ----  [HIGH] */
int __cdecl R_GetModeInfo(int a1, int *a2, float *a3, int *a4)
{
  int result;
  char **v5;
  double v6;
  double v7;

  if ( a1 < -1 )
    return 0;
  if ( a1 >= dword_571790[0] )
    return 0;
  if ( a1 == -1 )
  {
    *a4 = r_customwidth->integer;
    *a2 = r_customheight->integer;
    *a3 = r_customaspect->value;
    return 1;
  }
  else
  {
    v5 = &(&r_vidModes)[4 * a1];
    *a4 = (int)v5[1];
    *a2 = (int)v5[2];
    v6 = (double)(int)v5[1];
    v7 = (double)(int)v5[2] * *((float *)v5 + 3);
    result = 1;
    *a3 = v6 / v7;
  }
  return result;
}

/* ---- R_ModeList_f  0x004B0F30 ----  [HIGH] */
void R_ModeList_f()
{
  int v0;
  const char **v1;

  ri_Printf(0, "\n");
  v0 = 0;
  if ( dword_571790[0] > 0 )
  {
    v1 = (const char **)&r_vidModes;
    do
    {
      ri_Printf(0, "%s\n", *v1);
      ++v0;
      v1 += 4;
    }
    while ( v0 < dword_571790[0] );
  }
  ri_Printf(0, "\n");
}

/* ---- R_TakeScreenshot  0x004B0F90 ----  [HIGH] */
int __cdecl R_TakeScreenshot(GLsizei a1, GLint x, GLint y, GLsizei height, int a5)
{
  int v6;
  int v7;
  _BYTE *v8;
  unsigned int v9;
  char v10;
  signed int v11;
  signed int i;

  v6 = ri_Hunk_AllocateTempMemory(3 * (dwStyle * dwExStyle + 6));
  *(_DWORD *)v6 = 0;
  *(_DWORD *)(v6 + 4) = 0;
  *(_DWORD *)(v6 + 8) = 0;
  *(_DWORD *)(v6 + 12) = 0;
  *(_WORD *)(v6 + 16) = 0;
  *(_BYTE *)(v6 + 13) = BYTE1(a1);
  *(_BYTE *)(v6 + 14) = height;
  *(_BYTE *)(v6 + 2) = 2;
  *(_BYTE *)(v6 + 12) = a1;
  *(_BYTE *)(v6 + 15) = BYTE1(height);
  *(_BYTE *)(v6 + 16) = 24;
  glReadPixels(x, y, a1, height, 0x1907u, 0x1401u, (GLvoid *)(v6 + 18));
  v7 = 3 * (height * a1 + 6);
  if ( v7 > 18 )
  {
    v8 = (_BYTE *)(v6 + 20);
    v9 = (v7 - 19) / 3u + 1;
    do
    {
      v10 = *(v8 - 2);
      *(v8 - 2) = *v8;
      *v8 = v10;
      v8 += 3;
      --v9;
    }
    while ( v9 );
  }
  if ( tr_overbrightBits > 0 )
  {
    if ( glConfig_deviceSupportsGamma )
    {
      v11 = 3 * dwStyle * dwExStyle;
      for ( i = 0; i < v11; ++i )
        *(_BYTE *)(i + v6 + 18) = s_gammatable[*(unsigned __int8 *)(i + v6 + 18)];
    }
  }
  ri_FS_WriteFile(a5, v6, v7);
  return ri_Hunk_FreeTempMemory(v6);
}

/* ---- R_TakeScreenshotJPEG  0x004B10A0 ----  VERIFIED */
int __cdecl R_TakeScreenshotJPEG(int a1, GLint x, GLint y, GLsizei width, GLsizei height)
{
  GLvoid *v5;

  v5 = (GLvoid *)ri_Hunk_AllocateTempMemory(4 * dwStyle * dwExStyle);
  glReadPixels(x, y, width, height, 0x1908u, 0x1401u, v5);
  if ( tr_overbrightBits > 0 )
  {
    if ( glConfig_deviceSupportsGamma )
      R_GammaCorrect((int)v5, 4 * dwStyle * dwExStyle);
  }
  ri_FS_WriteFile(a1, v5, 1);
  SaveJPG(a1, 95, dwStyle, dwExStyle, 1, (int)v5);   /* ecx = glConfig_vidWidth */
  return ri_Hunk_FreeTempMemory(v5);
}

/* ---- R_ScreenshotFilename  0x004B1140 ----  VERIFIED */
void __cdecl R_ScreenshotFilename(unsigned int a1, char *fileName)
{
  if ( a1 >= 0x2710 )
    Com_sprintf(fileName, 256, "screenshots/shot9999.tga");
  else
    Com_sprintf(fileName, 256, "screenshots/shot%04i.tga", a1);
}

/* ---- R_ScreenshotFilenameJPEG  0x004B1180 ----  VERIFIED */
void __cdecl R_ScreenshotFilenameJPEG(unsigned int a1, char *fileName)
{
  if ( a1 >= 0x2710 )
    Com_sprintf(fileName, 256, "screenshots/shot9999.jpg");
  else
    Com_sprintf(fileName, 256, "screenshots/shot%04i.jpg", a1);
}

/* ---- R_LevelShot  0x004B11C0 ----  [HIGH] */
void R_LevelShot()
{
  char *v0;
  int v1;
  double v2;
  _BYTE *v3;
  double v4;
  int v5;
  int v6;
  int v7;
  int v8;
  char *v9;
  int v10;
  int v11;
  int v12;
  char *v13;
  int v14;
  int v15;
  int v16;
  char *v17;
  int v18;
  int v19;
  int v20;
  int v21;
  char *v22;
  int v23;
  char *v24;
  bool v25; // zf
  int v26;
  unsigned __int8 *v27;
  int v28;
  int v29;
  int v30;
  int v31;
  _BYTE *v32;
  int v33;
  int v34;
  int v35;
  int v36;
  char Buffer[256]; // [esp+44h] [ebp-104h] BYREF
  unsigned int v38;
  unsigned int retaddr;

  v38 = retaddr ^ _security_cookie;
  sprintf(Buffer, "levelshots/%s.tga", (const char *)(tr_world + 64));
  v0 = (char *)ri_Hunk_AllocateTempMemory(3 * dwStyle * dwExStyle);
  v1 = ri_Hunk_AllocateTempMemory(49170);
  *(_DWORD *)v1 = 0;
  *(_DWORD *)(v1 + 4) = 0;
  *(_DWORD *)(v1 + 8) = 0;
  *(_DWORD *)(v1 + 12) = 0;
  *(_WORD *)(v1 + 16) = 0;
  *(_BYTE *)(v1 + 2) = 2;
  *(_BYTE *)(v1 + 12) = 0x80;
  *(_BYTE *)(v1 + 14) = 0x80;
  *(_BYTE *)(v1 + 16) = 24;
  v34 = v1;
  glReadPixels(0, 0, dwStyle, dwExStyle, 0x1907u, 0x1401u, v0);
  v31 = 0;
  v2 = (double)(int)dwStyle * 0.001953125;
  v3 = (_BYTE *)(v1 + 20);
  v4 = (double)(int)dwExStyle * 0.0026041667;
  do
  {
    v30 = 2;
    v32 = v3;
    do
    {
      v5 = 0;
      v6 = 0;
      v7 = 0;
      v33 = v31;
      v29 = 3;
      do
      {
        v8 = dwStyle * (unsigned __int64)((double)v33 * v4);
        v35 = (unsigned __int64)((double)(v30 - 2) * v2);
        v9 = &v0[2 * v35 + 2 * v8 + v35 + v8];
        v10 = (unsigned __int8)*v9 + v7;
        v11 = (unsigned __int8)v9[1] + v6;
        v12 = (unsigned __int8)v9[2] + v5;
        v36 = (unsigned __int64)((double)(v30 - 1) * v2);
        v13 = &v0[2 * v36 + 2 * v8 + v36 + v8];
        v14 = (unsigned __int8)*v13 + v10;
        v15 = (unsigned __int8)v13[1] + v11;
        v16 = (unsigned __int8)v13[2] + v12;
        v17 = &v0[2 * (unsigned __int64)((double)v30 * v2) + 2 * v8 + (unsigned __int64)((double)v30 * v2) + v8];
        v18 = (unsigned __int8)*v17 + v14;
        v19 = (unsigned __int8)v17[1] + v15;
        v20 = (unsigned __int64)((double)(v30 + 1) * v2) + v8;
        v21 = (unsigned __int8)v17[2] + v16;
        v22 = &v0[2 * v20];
        v23 = (unsigned __int8)v22[v20];
        v24 = &v22[v20];
        v7 = v23 + v18;
        v6 = (unsigned __int8)v24[1] + v19;
        v5 = (unsigned __int8)v24[2] + v21;
        v25 = v29 == 1;
        ++v33;
        --v29;
      }
      while ( !v25 );
      *(v32 - 2) = (char)v5 / 12;
      *(v32 - 1) = (char)v6 / 12;
      *v32 = v7 / 12;
      v32 += 3;
      v30 += 4;
    }
    while ( v30 < 514 );
    v3 = v32;
    v31 += 3;
  }
  while ( v31 < 384 );
  if ( tr_overbrightBits > 0 && glConfig_deviceSupportsGamma )
  {
    v26 = v34;
    v27 = (unsigned __int8 *)(v34 + 19);
    v28 = 6144;
    do
    {
      *(v27 - 1) = s_gammatable[*(v27 - 1)];
      *v27 = s_gammatable[*v27];
      v27[1] = s_gammatable[v27[1]];
      v27[2] = s_gammatable[v27[2]];
      v27[3] = s_gammatable[v27[3]];
      v27[4] = s_gammatable[v27[4]];
      v27[5] = s_gammatable[v27[5]];
      v27[6] = s_gammatable[v27[6]];
      v27 += 8;
      --v28;
    }
    while ( v28 );
  }
  else
  {
    v26 = v34;
  }
  ri_FS_WriteFile(Buffer, v26, 49170);
  ri_Hunk_FreeTempMemory(v26);
  ri_Hunk_FreeTempMemory(v0);
  ri_Printf(0, "Wrote %s\n", Buffer);
}

/* ---- R_SaveGameShot  0x004B1520 ----  VERIFIED */
int __cdecl R_SaveGameShot(const char *a1)
{
  char *v1;
  int v2;
  double v3;
  _WORD *v4;
  double v5;
  int v6;
  int v7;
  int v8;
  int v9;
  char *v10;
  int v11;
  int v12;
  int v13;
  char *v14;
  char *v15;
  bool v16; // zf
  int i;
  int v19;
  int v20;
  int v21;
  _WORD *v22;
  int v23;
  int v24;
  int v25;
  char Buffer[256]; // [esp+44h] [ebp-104h] BYREF
  unsigned int v27;
  unsigned int retaddr;

  v27 = retaddr ^ _security_cookie;
  sprintf(Buffer, "%s.jpg", a1);
  v1 = (char *)ri_Hunk_AllocateTempMemory(4 * dwStyle * dwExStyle);
  glReadPixels(0, 0, dwStyle, dwExStyle, 0x1908u, 0x1401u, v1);
  v2 = ri_Hunk_AllocateTempMemory(0x100000);
  v3 = (double)(int)dwStyle * 0.00048828125;
  v24 = v2;
  v21 = 0;
  v4 = (_WORD *)(v2 + 2);
  v5 = (double)(int)dwExStyle * 0.00065104169;
  do
  {
    v20 = 2;
    v22 = v4;
    do
    {
      v6 = 0;
      v7 = 0;
      v8 = 0;
      v25 = (unsigned __int64)((double)(v20 - 2) * v3);
      v23 = v21;
      v19 = 3;
      do
      {
        v9 = dwStyle * (unsigned __int64)((double)v23 * v5);
        v10 = &v1[4 * v9 + 4 * (unsigned __int64)((double)(v20 - 1) * v3)];
        v11 = (unsigned __int8)*v10 + (unsigned __int8)v1[4 * v9 + 4 * v25] + v8;
        v12 = (unsigned __int8)v10[1] + (unsigned __int8)v1[4 * v9 + 1 + 4 * v25] + v7;
        v13 = (unsigned __int8)v10[2] + (unsigned __int8)v1[4 * v9 + 2 + 4 * v25] + v6;
        v14 = &v1[4 * v9 + 4 * (unsigned __int64)((double)v20 * v3)];
        v15 = &v1[4 * v9 + 4 * (unsigned __int64)((double)(v20 + 1) * v3)];
        v8 = (unsigned __int8)*v15 + (unsigned __int8)*v14 + v11;
        v6 = (unsigned __int8)v15[2] + (unsigned __int8)v14[2] + v13;
        v7 = (unsigned __int8)v15[1] + (unsigned __int8)v14[1] + v12;
        v16 = v19 == 1;
        ++v23;
        --v19;
      }
      while ( !v16 );
      *((_BYTE *)v22 - 2) = (char)v8 / 12;
      *((_BYTE *)v22 - 1) = (char)v7 / 12;
      *v22 = (unsigned __int8)(v6 / 12);
      v20 += 4;
      v22 += 2;
    }
    while ( v20 < 2050 );
    v4 = v22;
    v21 += 3;
  }
  while ( v21 < 1536 );
  if ( tr_overbrightBits > 0 )
  {
    if ( glConfig_deviceSupportsGamma )
    {
      for ( i = 0; i < 0x100000; ++i )
        *(_BYTE *)(i + v24) = s_gammatable[*(unsigned __int8 *)(i + v24)];
    }
  }
  SaveJPG((int)Buffer, 90, 0x200, 0x200u, 1, v24);   /* 0x004B17AC mov ecx, 200h */
  ri_Hunk_FreeTempMemory(v24);
  return ri_Hunk_FreeTempMemory(v1);
}

/* ---- RE_CubemapShot  0x004B17F0 ----  VERIFIED */
int __cdecl RE_CubemapShot(int a1, GLsizei width, int a3, float a4, float a5)
{
  char *v5;
  int v6;
  double v8;
  double v9;
  double v10;
  _BYTE *v11;
  int v12;
  _BYTE *v13;
  double v14;
  _BYTE *v15;
  double v16;
  double v17;
  double v18;
  double v19;
  long double v20;
  double v21;
  long double v22;
  long double v23;
  long double v24; // rt0
  long double v25;
  double v26;
  _BYTE *v27;
  char v28;
  GLsizei v29;
  bool v30; // zf
  GLsizei v32;
  _BYTE *v33;
  float v34;
  float v35;
  long double v36;
  char *v37;
  int v38;
  int v39;
  float v40[2]; // [esp+3Ch] [ebp-168h] BYREF
  float v41;
  GLsizei v42;
  float v43;
  float *v44;
  float v45;
  float *v46;
  float v47;
  _BYTE *v48;
  float v49;
  int v50;
  int v51;
  float *v52;
  int v53;
  float *v54;
  int v55;
  float v56;
  _BYTE *v57;
  float v58;
  float *v59;
  float *v60;
  long double v61;
  double v62;
  int v63;
  int v64;
  int v65;
  int v66; // [esp+B0h] [ebp-F4h] BYREF
  int v67; // [esp+B4h] [ebp-F0h] BYREF
  int v68; // [esp+B8h] [ebp-ECh] BYREF
  int v69; // [esp+BCh] [ebp-E8h] BYREF
  int v70; // [esp+C0h] [ebp-E4h] BYREF
  _DWORD v71[56]; // [esp+C4h] [ebp-E0h] BYREF

  v63 = 0;
  v64 = 0;
  v65 = 0;
  v66 = 0;
  v67 = 0;
  v68 = 0;
  v69 = 0;
  v70 = 0;
  memset(v71, 0, 12);
  v71[3] = 1065353216;
  v71[4] = 0;
  v71[5] = 1065353216;
  v71[6] = 0;
  v71[7] = -1082130432;
  memset(&v71[8], 0, 16);
  v71[12] = -1082130432;
  v71[13] = 0;
  v71[14] = 1065353216;
  v71[15] = 0;
  v71[16] = 1065353216;
  v71[17] = 0;
  v71[18] = 0;
  v71[19] = -1082130432;
  memset(&v71[20], 0, 12);
  v71[23] = -1082130432;
  memset(&v71[24], 0, 12);
  v71[27] = 1065353216;
  v71[28] = 1065353216;
  memset(&v71[29], 0, 12);
  v71[32] = 1065353216;
  memset(&v71[33], 0, 12);
  v71[36] = 1065353216;
  v71[37] = 0;
  v71[38] = -1082130432;
  v71[39] = 0;
  v71[40] = 1065353216;
  memset(&v71[41], 0, 16);
  v71[45] = 1065353216;
  v71[46] = 0;
  v71[47] = 1065353216;
  v71[48] = 0;
  v71[49] = -1082130432;
  memset(&v71[50], 0, 16);
  v71[54] = 1065353216;
  v5 = (char *)ri_Hunk_AllocateTempMemory(3 * width * width);
  v37 = v5;
  v55 = 4 * width * width + 18;
  v6 = ri_Hunk_AllocateTempMemory(v55);
  *(_DWORD *)v6 = 0;
  *(_DWORD *)(v6 + 4) = 0;
  *(_DWORD *)(v6 + 8) = 0;
  *(_DWORD *)(v6 + 12) = 0;
  *(_WORD *)(v6 + 16) = 0;
  v53 = v6;
  *(_BYTE *)(v6 + 2) = 2;
  *(_WORD *)(v6 + 12) = width;
  *(_WORD *)(v6 + 14) = width;
  *(_BYTE *)(v6 + 16) = 32;
  qglFinish();
  glReadPixels(1, dwExStyle - width - 1, width, width, 0x1907u, 0x1401u, v5);
  if ( tr_overbrightBits > 0 )
  {
    if ( glConfig_deviceSupportsGamma )
    {
      R_GammaCorrect((int)v5, 3 * width * width);
    }
  }
  if ( width > 0 )
  {
    v8 = (double)(width / 2) - 0.5;
    v60 = (float *)(&v67 + 9 * a3);
    v52 = (float *)(&v66 + 9 * a3);
    v9 = v8 * *((float *)&v63 + 9 * a3);
    v59 = (float *)(&v69 + 9 * a3);
    v56 = v9;
    v44 = (float *)(&v68 + 9 * a3);
    v46 = (float *)&v71[9 * a3];
    v10 = v8 * *((float *)&v64 + 9 * a3);
    v54 = (float *)&v71[9 * a3 - 1];
    v62 = 9.313225746154785e-10;
    v43 = v10;
    v11 = (_BYTE *)(v6 + 19);
    v32 = width;
    v58 = v8 * *((float *)&v65 + 9 * a3);
    v12 = width / -2;
    v50 = width / -2;
    v39 = width / -2;
    v13 = v5 + 1;
    while ( 1 )
    {
      v38 = v12;
      v14 = (double)v39 + 0.5;
      v15 = v11;
      v33 = v13;
      v16 = v14 * *v59;
      v42 = width;
      v45 = v16;
      v57 = &v11[4 * width];
      v48 = &v13[3 * width];
      v47 = v14 * *v54;
      v49 = v14 * *v46;
      do
      {
        v41 = v58;
        v17 = (double)v38 + 0.5;
        v35 = v17;
        v40[0] = v17 * *v52 + v56;
        v18 = v35 * *v60 + v43;
        v19 = v35 * *v44 + v58;
        v40[0] = v45 + v40[0];
        v40[1] = v47 + v18;
        v41 = v49 + v19;
        VectorNormalize(v40);
        if ( v41 <= 0.0 )
        {
          v36 = acos(-v41);
          v20 = sin(v36);
          v21 = a5 / a4;
        }
        else
        {
          v36 = acos(v41);
          v20 = sin(v36);
          v21 = a4 / a5;
        }
        v22 = asin(v20 * v21);
        v61 = v36 + v22;
        v23 = v36 - v22;
        v24 = sin(v23) / sin(v61);
        v25 = tan(v23) / tan(v61);
        v26 = (v25 * v25 + v24 * v24) * 0.5;
        if ( (v26 < 0.0) | __UNORDERED__(v26, 0.0) )
        {
          v26 = 0.0;
        }
        else if ( v26 > 1.0 )
        {
          v26 = 1.0;
        }
        v27 = v33;
        *(v15 - 1) = v33[1];
        v28 = *v33;
        v34 = v26 * 255.0;
        *v15 = v28;
        v15[1] = *(v27 - 1);
        v51 = (int)(v34 + v62);
        v33 = v27 + 3;
        v29 = v42;
        v15[2] = v51;
        v15 += 4;
        ++v38;
        v42 = v29 - 1;
      }
      while ( v29 != 1 );
      v30 = v32 == 1;
      ++v39;
      --v32;
      if ( v30 )
        break;
      v11 = v57;
      v13 = v48;
      v12 = v50;
    }
    v6 = v53;
    v5 = v37;
  }
  ri_FS_WriteFile(a1, v6, v55);
  ri_Hunk_FreeTempMemory(v6);
  return ri_Hunk_FreeTempMemory(v5);
}

/* ---- RE_CubemapWaterShot  0x004B1E60 ----  VERIFIED */
int __cdecl RE_CubemapWaterShot(int a1, int a2, int a3, float *a4, float *a5)
{
  int v5;
  int v6;
  double v7;
  double v8;
  double v9;
  double v10;
  _BYTE *v11;
  _BYTE *v12;
  double v13;
  double v14;
  double v15;
  double v16;
  double v17;
  double v18;
  float *v19;
  double v20;
  double v21;
  double v22;
  double v23;
  double v24;
  int v25;
  bool v26; // zf
  float v28;
  float v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;
  float v35;
  int v36;
  float v37[3]; // [ebp-0x180] BYREF  -- retail vec3_t
  int v40;
  int v41;
  int v42;
  float *v43;
  int v44;
  int v45;
  float v46;
  int v47;
  float v48;
  int v49;
  float *v50;
  int v51;
  float v52;
  float *v53;
  float v54;
  float v55;
  float v56;
  float v57;
  float v58;
  _BYTE *v59;
  float *v60;
  float *v61;
  float *v62;
  double v63;
  double v64;
  double v65;
  int v66;
  int v67;
  int v68;
  int v69; // [esp+B0h] [ebp-F0h] BYREF
  int v70; // [esp+B4h] [ebp-ECh] BYREF
  int v71; // [esp+B8h] [ebp-E8h] BYREF
  int v72; // [esp+BCh] [ebp-E4h] BYREF
  int v73; // [esp+C0h] [ebp-E0h] BYREF
  _DWORD v74[55]; // [esp+C4h] [ebp-DCh] BYREF

  v5 = 3 * (a2 * a2 + 6);
  v66 = 0;
  v67 = 0;
  v68 = 0;
  v69 = 0;
  v70 = 0;
  v71 = 0;
  v72 = 0;
  v73 = 0;
  memset(v74, 0, 12);
  v74[3] = 1065353216;
  v74[4] = 0;
  v74[5] = 1065353216;
  v74[6] = 0;
  v74[7] = -1082130432;
  memset(&v74[8], 0, 16);
  v74[12] = -1082130432;
  v74[13] = 0;
  v74[14] = 1065353216;
  v74[15] = 0;
  v74[16] = 1065353216;
  v74[17] = 0;
  v74[18] = 0;
  v74[19] = -1082130432;
  memset(&v74[20], 0, 12);
  v74[23] = -1082130432;
  memset(&v74[24], 0, 12);
  v74[27] = 1065353216;
  v74[28] = 1065353216;
  memset(&v74[29], 0, 12);
  v74[32] = 1065353216;
  memset(&v74[33], 0, 12);
  v74[36] = 1065353216;
  v74[37] = 0;
  v74[38] = -1082130432;
  v74[39] = 0;
  v74[40] = 1065353216;
  memset(&v74[41], 0, 16);
  v74[45] = 1065353216;
  v74[46] = 0;
  v74[47] = 1065353216;
  v74[48] = 0;
  v74[49] = -1082130432;
  memset(&v74[50], 0, 16);
  v74[54] = 1065353216;
  v51 = v5;
  v6 = ri_Hunk_AllocateTempMemory(v5);
  *(_DWORD *)v6 = 0;
  v49 = v6;
  *(_DWORD *)(v6 + 4) = 0;
  *(_DWORD *)(v6 + 8) = 0;
  *(_DWORD *)(v6 + 12) = 0;
  *(_WORD *)(v6 + 16) = 0;
  *(_BYTE *)(v6 + 2) = 2;
  *(_WORD *)(v6 + 12) = a2;
  *(_WORD *)(v6 + 14) = a2;
  *(_BYTE *)(v6 + 16) = 24;
  if ( a2 > 0 )
  {
    v7 = (double)(a2 / 2) - 0.5;
    v61 = (float *)(&v69 + 9 * a3);
    v43 = (float *)(&v71 + 9 * a3);
    v8 = v7 * *((float *)&v66 + 9 * a3);
    v50 = (float *)(&v70 + 9 * a3);
    v46 = v8;
    v60 = (float *)&v74[9 * a3 - 1];
    v53 = (float *)(&v72 + 9 * a3);
    v9 = v7 * *((float *)&v67 + 9 * a3);
    v63 = 9.313225746154785e-10;
    v64 = 9.313225746154785e-10;
    v57 = v9;
    v65 = 9.313225746154785e-10;
    v10 = v7 * *((float *)&v68 + 9 * a3);
    v62 = (float *)&v74[9 * a3];
    v48 = v10;
    v40 = a2 / -2;
    v11 = (_BYTE *)(v6 + 19);
    v36 = a2;
    while ( 1 )
    {
      v12 = v11;
      v13 = (double)v40 + 0.5;
      v41 = a2 / -2;
      v42 = a2;
      v52 = v13 * *v53;
      v56 = v13 * *v60;
      v14 = v13 * *v62;
      v59 = &v11[3 * a2];
      v58 = v14;
      do
      {
        v37[2] = v48;
        v15 = (double)v41 + 0.5;
        v28 = v15;
        v37[0] = v15 * *v61 + v46;
        v16 = v28 * *v50 + v57;
        v17 = v28 * *v43 + v48;
        v37[0] = v52 + v37[0];
        v37[1] = v56 + v16;
        v37[2] = v58 + v17;
        VectorNormalize(v37);
        if ( v37[2] >= 0.0 )
          v18 = v37[2];
        else
          v18 = 0.0;
        v33 = (1.0 - v18) * a4[1];
        v35 = (1.0 - v18) * a4[2];
        v32 = v18 * *a5 + (1.0 - v18) * *a4;
        v34 = v18 * a5[1] + v33;
        v19 = *(float **)(tr_world + 276);
        v20 = v18 * a5[2] + v35;
        if ( v19 )
        {
          v21 = v37[2] * v19[19] + v37[1] * v19[18] + v37[0] * v19[17];
          if ( v21 < 0.0 )
            v21 = 0.0;
          v22 = v19[5] + v19[9];
          v23 = v19[6] + v19[10];
          v55 = v19[7] + v19[11];
          v54 = v21 * v19[9] + v22;
          v24 = v21 * v19[10] + v23;
          v55 = v21 * v19[11] + v55;
          v32 = v54 * v32;
          v34 = v24 * v34;
          v20 = v20 * v55;
        }
        if ( (v32 > 2.0) | __UNORDERED__(2.0, v32) )
        {
          v32 = 2.0;
        }
        else if ( v32 < 0.0 )
        {
          v32 = 0.0;
        }
        if ( (v34 > 2.0) | __UNORDERED__(2.0, v34) )
        {
          v34 = 2.0;
        }
        else if ( v34 < 0.0 )
        {
          v34 = 0.0;
        }
        if ( (v20 > 2.0) | __UNORDERED__(2.0, v20) )
        {
          v20 = 2.0;
        }
        else if ( v20 < 0.0 )
        {
          v20 = 0.0;
        }
        v29 = v20 * 255.0;
        v44 = (int)(v29 + v63);
        *(v12 - 1) = v44;
        v30 = v34 * 255.0;
        v45 = (int)(v30 + v64);
        *v12 = v45;
        v31 = v32 * 255.0;
        v47 = (int)(v31 + v65);
        v25 = v41;
        v12[1] = v47;
        v12 += 3;
        v26 = v42 == 1;
        v41 = v25 + 1;
        --v42;
      }
      while ( !v26 );
      v26 = v36 == 1;
      ++v40;
      --v36;
      if ( v26 )
        break;
      v11 = v59;
    }
    v6 = v49;
    v5 = v51;
  }
  ri_FS_WriteFile(a1, v6, v5);
  return ri_Hunk_FreeTempMemory(v6);
}

/* ---- R_ScreenShot_f  0x004B25A0 ----  [HIGH] */
void __cdecl R_ScreenShot_f(const char *a1)
{
  const char *v1;
  BOOL v2;
  int v3;
  int v4;
  bool v5; // cc
  const char *v6;
  const char *v7;
  const char *v8;
  char v9[256]; // [esp+8h] [ebp-104h] BYREF
  unsigned int v10;
  unsigned int retaddr;

  v10 = retaddr ^ _security_cookie;
  if ( !strcmp((const char *)ri_Cmd_Argv(1), "levelshot") )
  {
    R_LevelShot();
    return;
  }
  if ( !strcmp((const char *)ri_Cmd_Argv(1), "savegame") && ri_Cmd_Argc() == 3 && *(_BYTE *)ri_Cmd_Argv(2) )
  {
    v1 = (const char *)ri_Cmd_Argv(2);
    R_SaveGameShot(v1);
    return;
  }
  v7 = a1;
  v2 = strcmp((const char *)ri_Cmd_Argv(1), "silent") == 0;
  if ( ri_Cmd_Argc() == 2 && !v2 )
  {
    v3 = ri_Cmd_Argv(1);
    Com_sprintf(v9, 256, "screenshots/%s.tga", v3);
    goto LABEL_21;
  }
  v4 = screenShotTGA_lastNumber;
  if ( screenShotTGA_lastNumber == -1 )
  {
    v4 = 0;
    screenShotTGA_lastNumber = 0;
  }
  else if ( screenShotTGA_lastNumber > 9999 )
  {
LABEL_19:
    ri_Printf(0, "ScreenShot: Couldn't create a file\n");
    return;
  }
  do
  {
    if ( (unsigned int)v4 >= 0x2710 )
      Com_sprintf(v9, 256, "screenshots/shot9999.tga");
    else
      Com_sprintf(v9, 256, "screenshots/shot%04i.tga", v4);
    v5 = ri_FS_ReadFile(v9, 0) <= 0;
    v4 = screenShotTGA_lastNumber;
    if ( v5 )
      break;
    v4 = screenShotTGA_lastNumber + 1;
    screenShotTGA_lastNumber = v4;
  }
  while ( v4 <= 9999 );
  if ( v4 >= 9999 )
    goto LABEL_19;
  screenShotTGA_lastNumber = v4 + 1;
LABEL_21:
  R_TakeScreenshot(dwStyle, 0, 0, dwExStyle, (int)v9);
  if ( !v2 )
    ri_Printf(0, "Wrote %s\n", v9);
}

/* ---- R_ScreenShotJPEG_f  0x004B2770 ----  [HIGH] */
void __cdecl R_ScreenShotJPEG_f(const char *a1)
{
  const char *v1;
  BOOL v2;
  int v3;
  int v4;
  bool v5; // cc
  const char *v6;
  const char *v7;
  const char *v8;
  char v9[256]; // [esp+8h] [ebp-104h] BYREF
  unsigned int v10;
  unsigned int retaddr;

  v10 = retaddr ^ _security_cookie;
  if ( !strcmp((const char *)ri_Cmd_Argv(1), "levelshot") )
  {
    R_LevelShot();
    return;
  }
  if ( !strcmp((const char *)ri_Cmd_Argv(1), "savegame") && ri_Cmd_Argc() == 3 && *(_BYTE *)ri_Cmd_Argv(2) )
  {
    v1 = (const char *)ri_Cmd_Argv(2);
    R_SaveGameShot(v1);
    return;
  }
  v7 = a1;
  v2 = strcmp((const char *)ri_Cmd_Argv(1), "silent") == 0;
  if ( ri_Cmd_Argc() == 2 && !v2 )
  {
    v3 = ri_Cmd_Argv(1);
    Com_sprintf(v9, 256, "screenshots/%s.jpg", v3);
    goto LABEL_21;
  }
  v4 = screenShotJPEG_lastNumber;
  if ( screenShotJPEG_lastNumber == -1 )
  {
    v4 = 0;
    screenShotJPEG_lastNumber = 0;
    goto LABEL_13;
  }
  if ( screenShotJPEG_lastNumber <= 9999 )
  {
    do
    {
LABEL_13:
      if ( (unsigned int)v4 >= 0x2710 )
        Com_sprintf(v9, 256, "screenshots/shot9999.jpg");
      else
        Com_sprintf(v9, 256, "screenshots/shot%04i.jpg", v4);
      v5 = ri_FS_ReadFile(v9, 0) <= 0;
      v4 = screenShotJPEG_lastNumber;
      if ( v5 )
        break;
      v4 = screenShotJPEG_lastNumber + 1;
      screenShotJPEG_lastNumber = v4;
    }
    while ( v4 <= 9999 );
  }
  if ( v4 == 10000 )
  {
    ri_Printf(0, "ScreenShot: Couldn't create a file\n");
    return;
  }
  screenShotJPEG_lastNumber = v4 + 1;
LABEL_21:
  R_TakeScreenshotJPEG((int)v9, 0, 0, dwStyle, dwExStyle);
  if ( !v2 )
    ri_Printf(0, "Wrote %s\n", v9);
}

#define GL_FOG_DISTANCE_MODE_NV     0x855A
#define GL_EYE_RADIAL_NV            0x855B
#define GL_EYE_PLANE_ABSOLUTE_NV    0x855C
#define GL_EYE_PLANE                0x2502

/* ---- R_SetNVFogMode  0x004B2950 ----  [HIGH] */
void R_SetNVFogMode( void )
{
	const char	*s = r_nv_fogdist_mode->string;

	if ( s && !Q_stricmpn( "GL_EYE_PLANE_ABSOLUTE_NV", s, 99999 ) ) {
		glConfig_NVFogMode = GL_EYE_PLANE_ABSOLUTE_NV;	/* glConfig +0x58 */
		qglFogi( GL_FOG_DISTANCE_MODE_NV, glConfig_NVFogMode );
		return;
	}
	if ( s && !Q_stricmpn( "GL_EYE_PLANE", s, 99999 ) ) {
		glConfig_NVFogMode = GL_EYE_PLANE;
		qglFogi( GL_FOG_DISTANCE_MODE_NV, glConfig_NVFogMode );
		return;
	}
	if ( s && !Q_stricmpn( "GL_EYE_RADIAL_NV", s, 99999 ) ) {
		glConfig_NVFogMode = GL_EYE_RADIAL_NV;
		qglFogi( GL_FOG_DISTANCE_MODE_NV, glConfig_NVFogMode );
		return;
	}

	glConfig_NVFogMode = GL_EYE_RADIAL_NV;
	ri_Cvar_Set( "r_nv_fogdist_mode", "GL_EYE_RADIAL_NV" );
	qglFogi( GL_FOG_DISTANCE_MODE_NV, glConfig_NVFogMode );
}

static const GLfloat s_matAmbientDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };	/* 0x00540604 */
static const GLfloat s_matSpecular[4]       = { 0.0f, 0.0f, 0.0f, 1.0f };	/* 0x00540614 */
static const GLfloat s_matEmission[4]       = { 0.0f, 0.0f, 0.0f, 1.0f };	/* 0x00540624 */
static const GLfloat s_lightModelAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	/* 0x00540634 */

/* ---- GL_SetDefaultState  0x004B2A40 ----  [HIGH] */
int GL_SetDefaultState()
{
  int v0;
  int v1;
  char *v2;
  int result = 0;
  int v4; // [esp+ECh] [ebp-4h] BYREF

  qglClearDepth(1.0);
  qglCullFace(1028);
  qglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  if ( qglActiveTextureARB )
  {
    v0 = 1;
    if ( Value > 1 )
    {
      do
      {
        if ( glState_currentTmu != v0 )
        {
          qglActiveTextureARB(v0 + 33984);
          glState_currentTmu = v0;
        }
        if ( glState_currentClientTmu != v0 )
        {
          qglClientActiveTextureARB(v0 + 33984);
          glState_currentClientTmu = v0;
        }
        GL_TextureMode((int)r_textureMode->string);
        if ( glState_texEnv[glState_currentTmu] != 8448 )
        {
          glState_texEnv[glState_currentTmu] = 8448;
          glTexEnvi(0x2300u, 0x2200u, 8448);
        }
        if ( glConfig_textureCubeMap )
          glDisable(0x8513u);
        glDisable(0xDE1u);
        glDisableClientState(0x8078u);
        v1 = Value;
        cap[v0++] = 0;
      }
      while ( v0 < v1 );
    }
    if ( glState_currentTmu )
    {
      qglActiveTextureARB(33984);
      glState_currentTmu = 0;
    }
    if ( glState_currentClientTmu )
    {
      qglClientActiveTextureARB(33984);
      glState_currentClientTmu = 0;
    }
  }
  if ( glConfig_textureCubeMap )
    glDisable(0x8513u);
  glEnable(0xDE1u);
  glDisableClientState(0x8078u);
  cap[0] = 3553;
  GL_TextureMode((int)r_textureMode->string);
  if ( glState_texEnv[glState_currentTmu] != 8448 )
  {
    glState_texEnv[glState_currentTmu] = 8448;
    glTexEnvi(0x2300u, 0x2200u, 8448);
  }
  qglShadeModel(7425);
  glDepthFunc(0x203u);
  glLightModelfv(0xB53u, s_lightModelAmbient);
  qglLightModelf(2897, 0.0f);
  qglLightModelf(2898, 0.0f);
  qglMaterialfv(1028, 5634, s_matAmbientDiffuse);
  qglMaterialfv(1028, 4610, s_matSpecular);
  qglMaterialf(1028, 5633, 0.0f);
  qglMaterialfv(1028, 5632, s_matEmission);
  glEnableClientState(0x8074u);
  glDisableClientState(0x8076u);
  glDisableClientState(0x8075u);
  glState_clientStateBits = 1024;
  glState_glStateBits = 65792;
  glPolygonMode(0x408u, 0x1B02u);
  qglDepthMask(1);
  glDisable(0xB71u);
  glEnable(0xC11u);
  glDisable(0xB44u);
  glDisable(0xBE2u);
  if ( lConfig_textureCompression )
  {
    qglGetIntegerv(34801, &v4);
    glConfig_maxPNTrianglesTessellationLevel = v4;
    if ( ((double)v4 < r_ati_truform_tess->value) | __UNORDERED__((double)v4, r_ati_truform_tess->value) )
    {
      v2 = va("%d", v4);
      ri_Cvar_Set("r_ati_truform_tess", v2);
    }
    lConfig_textureCompression(34804, r_ati_truform_tess->integer);
  }
  if ( glConfig_maxAnisotropy )
  {
    qglGetFloatv(34047, (GLfloat *)&v4);
    glConfig_maxTextureFilterAnisotropy = v4;
  }
  glState_fogMode = 2048;
  glState_fogHint = 4352;
  glState_fogColor = 0.0;
  flt_16C3964 = 0.0;
  flt_16C3968 = 0.0;
  dword_16C396C = 0;
  glState_fogStart = 0.0;
  glState_fogEnd = 1.0;
  glState_fogDensity = 1.0;
  qglFogi(2917, 2048);
  qglHint(3156, glState_fogHint);
  qglFogf(2914, glState_fogDensity);
  qglFogf(2915, glState_fogStart);
  qglFogf(2916, glState_fogEnd);
  if ( glConfig_NVFogAvailable )
    R_SetNVFogMode();
  glState_numEnabledLights = 0;
  glState_currentStorageMode = 3;
  return result;
}

/* ---- GfxInfo_f  0x004B2E40 ----  [HIGH] */
void GfxInfo_f()
{
  cvar_t *v0;
  int integer;
  const char *v2;
  _DWORD v3[2];
  _DWORD v4[2];

  v0 = ri_Cvar_Get("sys_cpustring", &empty_string, 0);
  v3[0] = "disabled";
  v3[1] = "enabled";
  v4[0] = "windowed";
  v4[1] = "fullscreen";
  ri_Printf(0, "\nGL_VENDOR: %s\n", (const char *)glConfig_vendor_string);
  ri_Printf(0, "GL_RENDERER: %s\n", (const char *)glConfig_renderer_string);
  ri_Printf(0, "GL_VERSION: %s\n", glConfig_version_string);
  ri_Printf(0, "GL_EXTENSIONS: %s\n", (const char *)glConfig_extensions_string);
  ri_Printf(0, "WGL_EXTENSIONS: %s\n", (const char *)glConfig_wextensions_string);
  ri_Printf(0, "GL_MAX_TEXTURE_SIZE: %d\n", glConfig_maxTextureSize);
  ri_Printf(0, "GL_MAX_ACTIVE_TEXTURES_ARB: %d\n", Value);
  ri_Printf(
    0,
    "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n",
    glConfig_colorBits,
    glConfig_depthBits,
    glConfig_stencilBits);
  ri_Printf(
    0,
    "MODE: %d, %d x %d %s hz:",
    r_mode->integer,
    dwStyle,
    dwExStyle,
    (const char *)v4[r_fullscreen->integer == 1]);
  if ( glConfig_displayFrequency )
    ri_Printf(0, "%d\n", glConfig_displayFrequency);
  else
    ri_Printf(0, "N/A\n");
  if ( glConfig_deviceSupportsGamma )
    v2 = "GAMMA: hardware w/ %d overbright bits\n";
  else
    v2 = "GAMMA: software w/ %d overbright bits\n";
  ri_Printf(0, v2, tr_overbrightBits);
  ri_Printf(0, "CPU: %s\n", v0->string);
  ri_Printf(0, "rendering primitives: ");
  integer = r_primitives->integer;
  switch ( integer )
  {
    case 0:
      if ( !qglLockArraysEXT )
      {
LABEL_9:
        ri_Printf(0, "multiple glArrayElement\n");
        break;
      }
LABEL_13:
      ri_Printf(0, "single glDrawElements\n");
      break;
    case -1:
      ri_Printf(0, "none\n");
      break;
    case 2:
      goto LABEL_13;
    case 1:
      goto LABEL_9;
    case 3:
      ri_Printf(0, "multiple glColor4ubv + glTexCoord2fv + glVertex3fv\n");
      break;
  }
  ri_Printf(0, "texturemode: %s\n", r_textureMode->string);
  ri_Printf(0, "picmip: %d\n", r_picmip->integer);
  ri_Printf(0, "picmip2: %d\n", r_picmip2->integer);
  ri_Printf(0, "texture bits: %d\n", r_texturebits->integer);
  ri_Printf(0, "multitexture: %s\n", (const char *)v3[qglActiveTextureARB != 0]);
  ri_Printf(0, "compiled vertex arrays: %s\n", (const char *)v3[qglLockArraysEXT != 0]);
  ri_Printf(0, "texenv add: %s\n", (const char *)v3[glConfig_textureEnvAddAvailable != 0]);
  ri_Printf(0, "ATI truform: %s\n", (const char *)v3[lConfig_textureCompression != 0]);
  if ( lConfig_textureCompression )
  {
    ri_Printf(0, "Truform Tess: %d\n", r_ati_truform_tess->integer);
    ri_Printf(0, "Truform Point Mode: %s\n", r_ati_truform_pointmode->string);
    ri_Printf(0, "Truform Normal Mode: %s\n", r_ati_truform_normalmode->string);
  }
  ri_Printf(0, "NV distance fog: %s\n", (const char *)v3[glConfig_NVFogAvailable != 0]);
  if ( glConfig_NVFogAvailable )
    ri_Printf(0, "Fog Mode: %s\n", r_nv_fogdist_mode->string);
  if ( r_finish->integer )
    ri_Printf(0, "Forcing glFinish\n");
}

/* ---- R_VboRefresh_f  0x004B31D0 ----  [HIGH] */
void R_VboRefresh_f()
{
  unsigned __int32 v0;
  int v1;
  signed __int32 v2;
  char *v3;

  if ( tr_world && glConfig_ARBVertexBufferObject )
  {
    v0 = cmd_argc;
    v1 = 1;
    v2 = cmd_argc;
    if ( cmd_argc > 1 )
    {
      while ( 1 )
      {
        v3 = &empty_string;
        if ( v1 < v0 )
          v3 = cmd_argv[v1];
        if ( !_stricmp(v3, "world") )
        {
          R_RefreshOptimizedWorldSurfaces_ARB();
        }
        else if ( !_stricmp(v3, "smodels") )
        {
          R_RefreshStaticModels_ARB(3);
        }
        else if ( !_stricmp(v3, "sverts") )
        {
          R_RefreshStaticModels_ARB(1);
        }
        else if ( !_stricmp(v3, "sindexes") )
        {
          R_RefreshStaticModels_ARB(2);
        }
        else if ( !_stricmp(v3, "xmodels") )
        {
          R_RefreshXModels_ARB(3);
        }
        else if ( !_stricmp(v3, "xverts") )
        {
          R_RefreshXModels_ARB(1);
        }
        else
        {
          if ( _stricmp(v3, "xindexes") )
            break;
          R_RefreshXModels_ARB(2);
        }
        if ( ++v1 >= v2 )
          break;
        v0 = cmd_argc;
      }
    }
    if ( v2 == 1 || v1 < v2 )
      ri_Printf(
        0,
        "usage: r_vbo_refresh [list], where list is one or more of world, smodels, sverts, sindexes, xmodels, xverts, and xindexes\n");
  }
}

/* ---- R_Register  0x004B3300 ----  [HIGH] */
int R_Register()
{
  char *v0;
  char *v1;

  sv_cheats = Cvar_Get("sv_cheats", "0", 72);
  r_maxActiveTextures = ri_Cvar_Get("r_maxActiveTextures", "0", 32);
  r_maxTextureSize = ri_Cvar_Get("r_maxTextureSize", "0", 32);
  r_ext_compiled_vertex_array = ri_Cvar_Get("r_ext_compiled_vertex_array", "1", 32);
  r_ext_rescale_normal = ri_Cvar_Get("r_ext_rescale_normal", "1", 32);
  r_ext_draw_range_elements = ri_Cvar_Get("r_ext_draw_range_elements", "1", 32);
  r_ati_pntriangles = ri_Cvar_Get("r_ati_pntriangles", "0", 33);
  r_ati_truform_tess = ri_Cvar_Get("r_ati_truform_tess", "1", 1);
  r_ati_truform_normalmode = ri_Cvar_Get("r_ati_truform_normalmode", "QUADRATIC", 1);
  r_ati_truform_pointmode = ri_Cvar_Get("r_ati_truform_pointmode", "CUBIC", 1);
  r_ati_fsaa_samples = ri_Cvar_Get("r_ati_fsaa_samples", "1", 1);
  r_ext_texture_filter_anisotropic = ri_Cvar_Get("r_ext_texture_filter_anisotropic", "0", 1);
  r_nv_fog_dist = ri_Cvar_Get("r_nv_fog_dist", "1", 33);
  r_nv_fogdist_mode = ri_Cvar_Get("r_nv_fogdist_mode", "GL_EYE_RADIAL_NV", 1);
  ri_Cvar_Get("r_nv_fog_available", "1", 64);
  r_arb_texture_env_add = ri_Cvar_Get("r_arb_texture_env_add", "1", 32);
  r_arb_texture_cube_map = ri_Cvar_Get("r_arb_texture_cube_map", "1", 32);
  r_arb_texture_env_combine = ri_Cvar_Get("r_arb_texture_env_combine", "1", 32);
  r_arb_texture_env_dot3 = ri_Cvar_Get("r_arb_texture_env_dot3", "1", 32);
  r_arb_vertex_buffer_object = ri_Cvar_Get("r_arb_vertex_buffer_object", "1", 32);
  r_arb_vertex_program = ri_Cvar_Get("r_arb_vertex_program", "1", 32);
  r_nv_register_combiners = ri_Cvar_Get("r_nv_register_combiners", "2", 32);
  r_nv_texture_shader = ri_Cvar_Get("r_nv_texture_shader", "1", 32);
  r_nv_fence = ri_Cvar_Get("r_nv_fence", "1", 32);
  r_nv_vertex_array_range = ri_Cvar_Get("r_nv_vertex_array_range", "2", 32);
  r_ati_vertex_array_object = ri_Cvar_Get("r_ati_vertex_array_object", "1", 32);
  r_ati_element_array = ri_Cvar_Get("r_ati_element_array", "1", 32);
  r_ati_fragment_shader = ri_Cvar_Get("r_ati_fragment_shader", "1", 32);
  r_vbo_smc_static_draw = ri_Cvar_Get("r_vbo_smc_static_draw", "1", 33);
  r_vbo_stream_draw = ri_Cvar_Get("r_vbo_stream_draw", "1", 33);
  r_vbo_interleave = ri_Cvar_Get("r_vbo_interleave", "0", 33);
  r_vbo_paranoia = ri_Cvar_Get("r_vbo_paranoia", "0", 1);
  r_skip_auto_config = ri_Cvar_Get("r_skip_auto_config", "0", 33);
  r_picmip = ri_Cvar_Get("r_picmip", "1", 33);
  r_picmip2 = ri_Cvar_Get("r_picmip2", "2", 33);
  r_colorMipLevels = ri_Cvar_Get("r_colorMipLevels", "0", 32);
  AssertCvarRange(1, (const char **)&r_picmip->name, 0.0, 1077936128);
  AssertCvarRange(1, (const char **)&r_picmip2->name, 0.0, 1077936128);
  r_detailtextures = ri_Cvar_Get("r_detailtextures", "1", 33);
  r_texturebits = ri_Cvar_Get("r_texturebits", "0", 33);
  r_colorbits = ri_Cvar_Get("r_colorbits", "32", 33);
  r_stencilbits = ri_Cvar_Get("r_stencilbits", "8", 33);
  r_depthbits = ri_Cvar_Get("r_depthbits", "0", 33);
  r_overBrightBits = ri_Cvar_Get("r_overBrightBits", "1", 33);
  r_ignorehwgamma = ri_Cvar_Get("r_ignorehwgamma", "0", 33);
  r_mode = ri_Cvar_Get("r_mode", "3", 33);
  r_fullscreen = ri_Cvar_Get("r_fullscreen", "1", 33);
  r_customwidth = ri_Cvar_Get("r_customwidth", "1600", 33);
  r_customheight = ri_Cvar_Get("r_customheight", "1024", 33);
  r_customaspect = ri_Cvar_Get("r_customaspect", "1", 33);
  r_simpleMipMaps = ri_Cvar_Get("r_simpleMipMaps", "1", 33);
  r_weightMipMaps = ri_Cvar_Get("r_weightMipMaps", "0", 32);
  r_uifullscreen = ri_Cvar_Get("r_uifullscreen", "0", 0);
  r_displayRefresh = ri_Cvar_Get("r_displayRefresh", "0", 32);
  AssertCvarRange(1, (const char **)&r_displayRefresh->name, 0.0, 1128792064);
  r_fullbright = ri_Cvar_Get("r_fullbright", "0", 544);
  r_intensity = ri_Cvar_Get("r_intensity", "1", 32);
  r_singleShader = ri_Cvar_Get("r_singleShader", "0", 544);
  r_lodbias = ri_Cvar_Get("r_lodbias", "0", 1);
  r_flares = ri_Cvar_Get("r_flares", "1", 512);
  r_znear = ri_Cvar_Get("r_znear", "4", 512);
  AssertCvarRange(1, (const char **)&r_znear->name, 0.001, 1128792064);
  r_zfar = ri_Cvar_Get("r_zfar", "0", 512);
  r_znear_depthhack = ri_Cvar_Get("r_znear_depthhack", "0.1", 512);
  AssertCvarRange(1, (const char **)&r_znear_depthhack->name, 0.001, 1120403456);
  r_ignoreGLErrors = ri_Cvar_Get("r_ignoreGLErrors", "1", 1);
  r_fastsky = ri_Cvar_Get("r_fastsky", "0", 1);
  X_r_inGameVideo = ri_Cvar_Get("r_inGameVideo", "1", 1);
  r_drawSun = ri_Cvar_Get("r_drawSun", "1", 1);
  r_dynamiclight = ri_Cvar_Get("r_dynamiclight", "1", 1);
  r_dlightQuality = ri_Cvar_Get("r_dlightQuality", "1", 1);
  r_finish = ri_Cvar_Get("r_finish", "0", 1);
  r_textureMode = ri_Cvar_Get("r_textureMode", "GL_LINEAR_MIPMAP_NEAREST", 1);
  r_swapDelay = ri_Cvar_Get("r_swapDelay", "0", 1);
  r_swapInterval = ri_Cvar_Get("r_swapInterval", "0", 1);
  r_gamma = ri_Cvar_Get("r_gamma", "1.0", 1);
  r_railWidth = ri_Cvar_Get("r_railWidth", "16", 1);
  r_railCoreWidth = ri_Cvar_Get("r_railCoreWidth", "1", 1);
  r_railSegmentLength = ri_Cvar_Get("r_railSegmentLength", "32", 1);
  r_primitives = ri_Cvar_Get("r_primitives", "0", 1);
  r_showImages = ri_Cvar_Get("r_showImages", "0", 256);
  r_debugSort = ri_Cvar_Get("r_debugSort", "0", 512);
  r_printShaders = ri_Cvar_Get("r_printShaders", "0", 0);
  r_saveFontData = ri_Cvar_Get("r_saveFontData", "0", 0);
  r_showLeafLights = ri_Cvar_Get("r_showLeafLights", "0", 512);
  r_debugEntLight = ri_Cvar_Get("r_debugEntLight", "0", 512);
  r_maxEntLights = ri_Cvar_Get("r_maxEntLights", "8", 1);
  r_minEntLightIntensity = ri_Cvar_Get("r_minEntLightIntensity", "0.02", 1);
  r_entLightCutoff = ri_Cvar_Get("r_entLightCutoff", "0.2", 1);
  r_entFullbright = ri_Cvar_Get("r_entFullbright", "0", 512);
  r_entMinLight = ri_Cvar_Get("r_entMinLight", ".15", 512);
  r_diffuseSunSteps = ri_Cvar_Get("r_diffuseSunSteps", "3", 1);
  r_diffuseSunQuality = ri_Cvar_Get("r_diffuseSunQuality", "2", 1);
  r_vc_makelog = ri_Cvar_Get("r_vc_makelog", "0", 32);
  r_vc_showlog = ri_Cvar_Get("r_vc_showlog", "0", 0);
  r_vc_compile = ri_Cvar_Get("r_vc_compile", "0", 32);
  r_fog = ri_Cvar_Get("r_fog", "1", 512);
  r_drawworld = ri_Cvar_Get("r_drawworld", "1", 512);
  r_lightmap = ri_Cvar_Get("r_lightmap", "0", 512);
  r_graymap = ri_Cvar_Get("r_graymap", "0", 544);
  r_portalOnly = ri_Cvar_Get("r_portalOnly", "0", 512);
  r_flareSize = ri_Cvar_Get("r_flareSize", "96", 512);
  r_flareFadeIn = ri_Cvar_Get("r_flareFadeIn", ".2", 512);
  r_flareFadeOut = ri_Cvar_Get("r_flareFadeOut", ".2", 512);
  r_skipBackEnd = ri_Cvar_Get("r_skipBackEnd", "0", 512);
  r_measureOverdraw = ri_Cvar_Get("r_measureOverdraw", "0", 512);
  r_lodscale = ri_Cvar_Get("r_lodscale", "1", 1);
  r_norefresh = ri_Cvar_Get("r_norefresh", "0", 512);
  r_drawentities = ri_Cvar_Get("r_drawentities", "1", 512);
  r_drawBModels = ri_Cvar_Get("r_drawBModels", "1", 512);
  r_drawSModels = ri_Cvar_Get("r_drawSModels", "1", 512);
  r_drawXModels = ri_Cvar_Get("r_drawXModels", "1", 512);
  r_ignore = ri_Cvar_Get("r_ignore", "1", 0);
  r_nocull = ri_Cvar_Get("r_nocull", "0", 512);
  outsideMapEnts = ri_Cvar_Get("outsideMapEnts", "0", 512);
  r_speeds = ri_Cvar_Get("r_speeds", "0", 512);
  r_verbose = ri_Cvar_Get("r_verbose", "0", 0);
  r_logFile = ri_Cvar_Get("r_logFile", "0", 0);
  r_debugGLErrors = ri_Cvar_Get("r_debugGLErrors", "0", 0);
  r_profileDrawElements = ri_Cvar_Get("r_profileDrawElements", "0", 512);
  r_nobind = ri_Cvar_Get("r_nobind", "0", 512);
  r_showtris = ri_Cvar_Get("r_showtris", "0", 512);
  r_showtricounts = ri_Cvar_Get("r_showtricounts", "0", 512);
  r_showsurfcounts = ri_Cvar_Get("r_showsurfcounts", "0", 512);
  r_showsky = ri_Cvar_Get("r_showsky", "0", 512);
  r_shownormals = ri_Cvar_Get("r_shownormals", &empty_string, 512);
  r_clear = ri_Cvar_Get("r_clear", "0", 0);
  r_offsetfactor = ri_Cvar_Get("r_offsetfactor", "-1", 512);
  r_offsetunits = ri_Cvar_Get("r_offsetunits", "-2", 512);
  r_drawBuffer = ri_Cvar_Get("r_drawBuffer", "GL_BACK", 512);
  r_lockpvs = ri_Cvar_Get("r_lockpvs", "0", 512);
  r_noportals = ri_Cvar_Get("r_noportals", "0", 512);
  cg_shadows = ri_Cvar_Get("cg_shadows", "0", 513);
  cg_skybox = ri_Cvar_Get("cg_skybox", "1", 0);
  v0 = va("%d", 0x1000);
  r_maxpolys = ri_Cvar_Get("r_maxpolys", v0, 0);
  v1 = va("%d", 0x2000);
  r_maxpolyverts = ri_Cvar_Get("r_maxpolyverts", v1, 0);
  r_showportals = ri_Cvar_Get("r_showportals", "0", 512);
  r_showaabbtrees = ri_Cvar_Get("r_showaabbtrees", "0", 512);
  r_cullBModels = ri_Cvar_Get("r_cullBModels", "1", 0);
  r_showCullBModels = ri_Cvar_Get("r_showCullBModels", "0", 512);
  r_showCullSModels = ri_Cvar_Get("r_showCullSModels", "0", 512);
  r_cullXModels = ri_Cvar_Get("r_cullXModels", "1", 0);
  r_showCullXModels = ri_Cvar_Get("r_showCullXModels", "0", 512);
  r_singlecell = ri_Cvar_Get("r_singlecell", "0", 512);
  r_portalbevels = ri_Cvar_Get("r_portalbevels", "0.7", 1);
  r_xdebug = ri_Cvar_Get("r_xdebug", &empty_string, 512);
  r_errorOnConflicts = ri_Cvar_Get("r_errorOnConflicts", "1", 0);
  r_highLodDist = ri_Cvar_Get("r_highLodDist", "-1", 512);
  r_mediumLodDist = ri_Cvar_Get("r_mediumLodDist", "0", 512);
  r_lowLodDist = ri_Cvar_Get("r_lowLodDist", "0", 512);
  r_lodViewDist = ri_Cvar_Get("r_lodViewDist", "0", 512);
  r_suntest = ri_Cvar_Get("r_suntest", "0", 512);
  r_sunsprite_shader = ri_Cvar_Get("r_sunsprite_shader", &aSun, 0);
  r_sunsprite_size = ri_Cvar_Get("r_sunsprite_size", "16", 0);
  r_sunflare_shader = ri_Cvar_Get("r_sunflare_shader", "sunFlareShader", 0);
  r_sunflare_min_size = ri_Cvar_Get("r_sunflare_min_size", "0", 0);
  r_sunflare_min_angle = ri_Cvar_Get("r_sunflare_min_angle", "45", 0);
  r_sunflare_max_size = ri_Cvar_Get("r_sunflare_max_size", "2500", 0);
  r_sunflare_max_angle = ri_Cvar_Get("r_sunflare_max_angle", "2", 0);
  r_sunflare_max_alpha = ri_Cvar_Get("r_sunflare_max_alpha", "1", 0);
  r_sunflare_fadein = ri_Cvar_Get("r_sunflare_fadein", "1", 0);
  r_sunflare_fadeout = ri_Cvar_Get("r_sunflare_fadeout", "1", 0);
  r_sunblind_min_angle = ri_Cvar_Get("r_sunblind_min_angle", "30", 0);
  r_sunblind_max_angle = ri_Cvar_Get("r_sunblind_max_angle", "5", 0);
  r_sunblind_max_darken = ri_Cvar_Get("r_sunblind_max_darken", ".75", 0);
  r_sunblind_fadein = ri_Cvar_Get("r_sunblind_fadein", ".5", 0);
  r_sunblind_fadeout = ri_Cvar_Get("r_sunblind_fadeout", "3", 0);
  r_sunglare_min_angle = ri_Cvar_Get("r_sunglare_min_angle", "30", 0);
  r_sunglare_max_angle = ri_Cvar_Get("r_sunglare_max_angle", "5", 0);
  r_sunglare_max_lighten = ri_Cvar_Get("r_sunglare_max_lighten", ".75", 0);
  r_sunglare_fadein = ri_Cvar_Get("r_sunglare_fadein", ".5", 0);
  r_sunglare_fadeout = ri_Cvar_Get("r_sunglare_fadeout", "3", 0);
  r_optimize = ri_Cvar_Get("r_optimize", "1", 32);
  r_optimizeBackend = ri_Cvar_Get("r_optimizeBackend", "1", 32);
  r_optimizeSModels = ri_Cvar_Get("r_optimizeSModels", "1", 32);
  r_optimizeXModels = ri_Cvar_Get("r_optimizeXModels", "100", 32);
  r_optimizeWorld = ri_Cvar_Get("r_optimizeWorld", "1", 32);
  r_optimizeTextures = ri_Cvar_Get("r_optimizeTextures", "2", 32);
  r_debugOptTex = ri_Cvar_Get("r_debugOptTex", "0", 0);
  r_mem_manual = ri_Cvar_Get("r_mem_manual", "0", 33);
  r_mem_agp = ri_Cvar_Get("r_mem_agp", "8", 33);
  r_mem_video = ri_Cvar_Get("r_mem_video", "2", 33);
  r_mem_backend = ri_Cvar_Get("r_mem_backend", "0.5", 33);
  r_smc_enable = ri_Cvar_Get("r_smc_enable", "1", 0);
  ri_Cmd_AddCommand("imagelist", R_ImageList_f);
  ri_Cmd_AddCommand("shaderlist", R_ShaderList_f);
  ri_Cmd_AddCommand("modelist", R_ModeList_f);
  ri_Cmd_AddCommand("screenshot", R_ScreenShot_f);
  ri_Cmd_AddCommand("screenshotJPEG", R_ScreenShotJPEG_f);
  ri_Cmd_AddCommand("gfxinfo", GfxInfo_f);
  ri_Cmd_AddCommand("r_meminfo", R_MemInfo_f);
  ri_Cmd_AddCommand("r_vc_stats", R_VC_stats_f);
  ri_Cmd_AddCommand("r_smc_stats", R_StaticModelCacheStats_f);
  ri_Cmd_AddCommand("r_smc_flush", R_StaticModelCacheFlush_f);
  ri_Cmd_AddCommand("r_loadsun", R_LoadSun_f);
  ri_Cmd_AddCommand("r_savesun", R_SaveSun_f);
  ri_Cmd_AddCommand("r_sunhelp", R_SunHelp_f);
  return ri_Cmd_AddCommand("r_vbo_refresh", R_VboRefresh_f);
}

/* ---- R_ClearTrShards  no-address ----  [HIGH] */
static void R_ClearTrShards( void )
{
	memset( &tr_frameCount, 0, sizeof( tr_frameCount ) );                          /* 0x016C4D44 */
	memset( &tr_viewCount, 0, sizeof( tr_viewCount ) );                            /* 0x016C4D48 */
	memset( &tr_frameSceneNum, 0, sizeof( tr_frameSceneNum ) );                    /* 0x016C4D4C */
	memset( &tr_worldMapLoaded, 0, sizeof( tr_worldMapLoaded ) );                  /* 0x016C4D50 */
	memset( &tr_world, 0, sizeof( tr_world ) );                                    /* 0x016C4D54 */
	memset( &tr_defaultImage, 0, sizeof( tr_defaultImage ) );                      /* 0x016C4D58 */
	memset( &tr_scratchImages, 0, sizeof( tr_scratchImages ) );                    /* 0x016C4D5C */
	memset( &tr_dlightImage, 0, sizeof( tr_dlightImage ) );                        /* 0x016C4DDC */
	memset( &tr_whiteImage, 0, sizeof( tr_whiteImage ) );                          /* 0x016C4DE4 */
	memset( &tr_identityLightImage, 0, sizeof( tr_identityLightImage ) );          /* 0x016C4DE8 */
	memset( &tr_grayImage, 0, sizeof( tr_grayImage ) );                            /* 0x016C4DEC */
	memset( &tr_screenImage, 0, sizeof( tr_screenImage ) );                        /* 0x016C4DF0 */
	memset( &tr_screenImageWidth, 0, sizeof( tr_screenImageWidth ) );              /* 0x016C4DF4 */
	memset( &tr_screenImageHeight, 0, sizeof( tr_screenImageHeight ) );            /* 0x016C4DF8 */
	memset( &tr_screenImageSaveTime, 0, sizeof( tr_screenImageSaveTime ) );        /* 0x016C4DFC */
	memset( &tr_screenImageSMax, 0, sizeof( tr_screenImageSMax ) );                /* 0x016C4E00 */
	memset( &tr_screenImageTMax, 0, sizeof( tr_screenImageTMax ) );                /* 0x016C4E04 */
	memset( &tr_stencilShadowShader, 0, sizeof( tr_stencilShadowShader ) );        /* 0x016C4E0C */
	memset( &tr_screenImageShader, 0, sizeof( tr_screenImageShader ) );            /* 0x016C4E10 */
	memset( &tr_dlightShader, 0, sizeof( tr_dlightShader ) );                      /* 0x016C4E14 */
	memset( &tr_flareShader, 0, sizeof( tr_flareShader ) );                        /* 0x016C4E18 */
	memset( &tr_spotLightShader, 0, sizeof( tr_spotLightShader ) );                /* 0x016C4E1C */
	memset( &tr_ignorePrecacheErrorCount, 0, sizeof( tr_ignorePrecacheErrorCount ) ); /* 0x016C4E60 */
	memset( &tr_modelsFinishedLoading, 0, sizeof( tr_modelsFinishedLoading ) );    /* 0x016C4E64 */
	memset( &tr_delayedImageGroup, 0, sizeof( tr_delayedImageGroup ) );            /* 0x016C4E68 */
	memset( &tr_delayedImageGroupTriCount, 0, sizeof( tr_delayedImageGroupTriCount ) ); /* 0x016C4E6C */
	memset( &tr_delayedImageGroupTileMode, 0, sizeof( tr_delayedImageGroupTileMode ) ); /* 0x016C4E70 */
	memset( &tr_delayedImageGroupSequence, 0, sizeof( tr_delayedImageGroupSequence ) ); /* 0x016C4E74 */
	memset( &tr_delayedImageCount, 0, sizeof( tr_delayedImageCount ) );            /* 0x016C4E78 */
	memset( &tr_showTrisShader, 0, sizeof( tr_showTrisShader ) );                  /* 0x016C4E7C */
	memset( &tr_showImagesShader, 0, sizeof( tr_showImagesShader ) );              /* 0x016C4E80 */
	memset( &tr_lightmapCount, 0, sizeof( tr_lightmapCount ) );                    /* 0x016C4E84 */
	memset( &tr_lightmaps, 0, sizeof( tr_lightmaps ) );                            /* 0x016C4E88 */
	memset( &tr_currentEntity, 0, sizeof( tr_currentEntity ) );                    /* 0x016C5088 */
	memset( &tr_worldEntity, 0, sizeof( tr_worldEntity ) );                        /* 0x016C508C */
	memset( &tr_currentEntityNumber, 0, sizeof( tr_currentEntityNumber ) );        /* 0x016C5344 */
	memset( &tr_shiftedEntityNumber, 0, sizeof( tr_shiftedEntityNumber ) );        /* 0x016C5348 */
	memset( &tr_identityLight, 0, sizeof( tr_identityLight ) );                    /* 0x016C55B0 */
	memset( &tr_identityLightByte, 0, sizeof( tr_identityLightByte ) );            /* 0x016C55B4 */
	memset( &tr_overbrightBits, 0, sizeof( tr_overbrightBits ) );                  /* 0x016C55B8 */
	memset( &tr_viewCluster, 0, sizeof( tr_viewCluster ) );                        /* 0x016C57C0 */
	memset( &tr_maxEntityLights, 0, sizeof( tr_maxEntityLights ) );                /* 0x016C57DC */
	memset( &tr_diffuseSunQuality, 0, sizeof( tr_diffuseSunQuality ) );            /* 0x016C57E0 */
	memset( &tr_diffuseSunSteps, 0, sizeof( tr_diffuseSunSteps ) );                /* 0x016C57E4 */
	memset( &tr_diffuseSunSampleScale, 0, sizeof( tr_diffuseSunSampleScale ) );    /* 0x016C57E8 */
	memset( &qword_16C57EC, 0, sizeof( qword_16C57EC ) );                          /* 0x016C57EC */
	memset( &qword_16C57F4, 0, sizeof( qword_16C57F4 ) );                          /* 0x016C57F4 */
	memset( &qword_16C57FC, 0, sizeof( qword_16C57FC ) );                          /* 0x016C57FC */
	memset( &tr_pc_leafCount, 0, sizeof( tr_pc_leafCount ) );                      /* 0x016C5804 */
	memset( &qword_16C5808, 0, sizeof( qword_16C5808 ) );                          /* 0x016C5808 */
	memset( &tr_frontEndMsec, 0, sizeof( tr_frontEndMsec ) );                      /* 0x016C5810 */
	memset( &tr_performanceCounters, 0, sizeof( tr_performanceCounters ) );        /* 0x016C5814 */
	memset( &tr_models, 0, sizeof( tr_models ) );                                  /* 0x016C5818 */
	memset( &tr_numModels, 0, sizeof( tr_numModels ) );                            /* 0x016C7818 */
	memset( &tr_xmodelRefreshModelIndex, 0, sizeof( tr_xmodelRefreshModelIndex ) ); /* 0x016C781C */
	memset( &tr_xmodelRefreshLodIndex, 0, sizeof( tr_xmodelRefreshLodIndex ) );    /* 0x016C7820 */
	memset( &tr_xmodelRefreshSurfaceIndex, 0, sizeof( tr_xmodelRefreshSurfaceIndex ) ); /* 0x016C7824 */
	memset( &tr_staticModelRefreshModelIndex, 0, sizeof( tr_staticModelRefreshModelIndex ) ); /* 0x016C7828 */
	memset( &tr_staticModelRefreshLodIndex, 0, sizeof( tr_staticModelRefreshLodIndex ) ); /* 0x016C782C */
	memset( &tr_staticModelRefreshSurfaceIndex, 0, sizeof( tr_staticModelRefreshSurfaceIndex ) ); /* 0x016C7830 */
	memset( &tr_worldRefreshSurfaceIndex, 0, sizeof( tr_worldRefreshSurfaceIndex ) ); /* 0x016C7834 */
	memset( &tr_registeredStaticModels, 0, sizeof( tr_registeredStaticModels ) );  /* 0x016C7838 */
	memset( &tr_registeredStaticModelCount, 0, sizeof( tr_registeredStaticModelCount ) ); /* 0x016C9838 */
	memset( &tr_numImages, 0, sizeof( tr_numImages ) );                            /* 0x016C983C */
	memset( &dword_16C9840, 0, sizeof( dword_16C9840 ) );                          /* 0x016C9840 */
	memset( &tr_imageMemory, 0, sizeof( tr_imageMemory ) );                        /* 0x016CB840 */
	memset( &dword_16D384C, 0, sizeof( dword_16D384C ) );                          /* 0x016D384C */
	memset( &flt_16D3850, 0, sizeof( flt_16D3850 ) );                              /* 0x016D3850 */
	memset( &flt_16D4850, 0, sizeof( flt_16D4850 ) );                              /* 0x016D4850 */
	memset( &flt_16D5050, 0, sizeof( flt_16D5050 ) );                              /* 0x016D5050 */
	memset( &flt_16D5450, 0, sizeof( flt_16D5450 ) );                              /* 0x016D5450 */
	memset( &flt_16D5850, 0, sizeof( flt_16D5850 ) );                              /* 0x016D5850 */
	memset( &flt_16D6850, 0, sizeof( flt_16D6850 ) );                              /* 0x016D6850 */
	memset( &flt_16D7850, 0, sizeof( flt_16D7850 ) );                              /* 0x016D7850 */
	memset( &tr_staticVertexMemorySecondary, 0, sizeof( tr_staticVertexMemorySecondary ) ); /* 0x016D8850 */
	memset( &tr_staticVertexMemorySecondaryLimit, 0, sizeof( tr_staticVertexMemorySecondaryLimit ) ); /* 0x016D8854 */
	memset( &tr_staticVertexMemorySecondaryUsed, 0, sizeof( tr_staticVertexMemorySecondaryUsed ) ); /* 0x016D8858 */
	memset( &tr_staticVertexMemoryPrimary, 0, sizeof( tr_staticVertexMemoryPrimary ) ); /* 0x016D885C */
	memset( &tr_staticVertexMemoryPrimaryLimit, 0, sizeof( tr_staticVertexMemoryPrimaryLimit ) ); /* 0x016D8860 */
	memset( &tr_staticVertexMemoryPrimaryUsed, 0, sizeof( tr_staticVertexMemoryPrimaryUsed ) ); /* 0x016D8864 */
	memset( &storageClass, 0, sizeof( storageClass ) );                            /* 0x016D8868 */
	memset( &tr_dynamicBufferFrameSerial, 0, sizeof( tr_dynamicBufferFrameSerial ) ); /* 0x016D886C */
	memset( &tr_dynamicBufferMaxFrameSerial, 0, sizeof( tr_dynamicBufferMaxFrameSerial ) ); /* 0x016D8870 */
	memset( &tr_cachedStaticModelStorageSource, 0, sizeof( tr_cachedStaticModelStorageSource ) ); /* 0x016D8874 */
	memset( &tr_cachedStaticModelSurfaceType, 0, sizeof( tr_cachedStaticModelSurfaceType ) ); /* 0x016D8878 */
	memset( &tr_cachedStaticModelStorage, 0, sizeof( tr_cachedStaticModelStorage ) ); /* 0x016D887C */
	memset( &tr_cachedStaticModelStorageOffset, 0, sizeof( tr_cachedStaticModelStorageOffset ) ); /* 0x016D8880 */
	memset( &tr_vboStreamDraw, 0, sizeof( tr_vboStreamDraw ) );                    /* 0x016D8884 */
	memset( &tr_vboInterleaved, 0, sizeof( tr_vboInterleaved ) );                  /* 0x016D8888 */
	memset( &tr_vboUsage, 0, sizeof( tr_vboUsage ) );                              /* 0x016D888C */
	memset( &rbDebug_polygonVertexCapacity, 0, sizeof( rbDebug_polygonVertexCapacity ) ); /* 0x016D8894 */
	memset( &rbDebug_polygonVertexCount, 0, sizeof( rbDebug_polygonVertexCount ) ); /* 0x016D8898 */
	memset( &rbDebug_polygonCapacity, 0, sizeof( rbDebug_polygonCapacity ) );      /* 0x016D88A0 */
	memset( &rbDebug_polygonCount, 0, sizeof( rbDebug_polygonCount ) );            /* 0x016D88A4 */
	memset( &rbDebug_stringCapacity, 0, sizeof( rbDebug_stringCapacity ) );        /* 0x016D88AC */
	memset( &rbDebug_stringCount, 0, sizeof( rbDebug_stringCount ) );              /* 0x016D88B0 */
	memset( &rbDebug_locatedStringCount, 0, sizeof( rbDebug_locatedStringCount ) ); /* 0x016D88BC */
	memset( &rbDebug_locatedStrings, 0, sizeof( rbDebug_locatedStrings ) );        /* 0x016D88C0 */
	memset( &rbDebug_lineCapacity, 0, sizeof( rbDebug_lineCapacity ) );            /* 0x016D88C4 */
	memset( &rbDebug_lineCount, 0, sizeof( rbDebug_lineCount ) );                  /* 0x016D88C8 */
	memset( &rbDebug_locatedLineCount, 0, sizeof( rbDebug_locatedLineCount ) );    /* 0x016D88D0 */
	memset( &rbDebug_locatedLines, 0, sizeof( rbDebug_locatedLines ) );            /* 0x016D88D4 */
	memset( &rbDebug_plumeCount, 0, sizeof( rbDebug_plumeCount ) );                /* 0x016D88D8 */
	memset( &rbDebug_plumeCapacity, 0, sizeof( rbDebug_plumeCapacity ) );          /* 0x016D88DC */
	memset( &rbDebug_immediateModeActive, 0, sizeof( rbDebug_immediateModeActive ) ); /* 0x016D88E4 */
	memset( &rbDebug_immediateLineWidth, 0, sizeof( rbDebug_immediateLineWidth ) ); /* 0x016D88EC */
	memset( &rbDebug_immediateTexCoordS, 0, sizeof( rbDebug_immediateTexCoordS ) ); /* 0x016D88F0 */
	memset( &rbDebug_immediateTexCoordT, 0, sizeof( rbDebug_immediateTexCoordT ) ); /* 0x016D88F4 */
	memset( &rbDebug_immediateColorR, 0, sizeof( rbDebug_immediateColorR ) );      /* 0x016D88F8 */
	memset( &rbDebug_immediateColorG, 0, sizeof( rbDebug_immediateColorG ) );      /* 0x016D88F9 */
	memset( &rbDebug_immediateColorB, 0, sizeof( rbDebug_immediateColorB ) );      /* 0x016D88FA */
	memset( &rbDebug_immediateColorA, 0, sizeof( rbDebug_immediateColorA ) );      /* 0x016D88FB */
	memset( &rbDebug_immediateVertexCount, 0, sizeof( rbDebug_immediateVertexCount ) ); /* 0x016D8908 */
	memset( &rbDebug_immediateVertexCapacity, 0, sizeof( rbDebug_immediateVertexCapacity ) ); /* 0x016D890C */
}

/* ---- R_Init  0x004B4590 ---- */
void __cdecl R_Init(const char *a1)
{
  int v1;
  double v2;
  double v3;
  double v4;
  double v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  float v11;

  ri_Printf(0, "----- R_Init -----\n");
  memset(&tr_registered, 0, 0x13BD4u);
  R_ClearTrShards();
  memset(&unk_16D89C0, 0, 0x1A4Cu);
  memset(tess_indexes, 0, 0x218068u);
  Swap_Init();
  if ( ((unsigned __int8)tess_xyz & 0xF) != 0 )
    Com_Printf("WARNING: tess.xyz not 16 byte aligned\n");
  memset(tess_constantColor255, 0xFFu, sizeof(tess_constantColor255));
  v1 = 0;
  v10 = 0;
  do
  {
    v2 = (double)v10;
    v11 = v2;
    flt_16D3850[v1] = sin(v2 * 0.3515625 * 3.1415927 * 0.0055555557);
    if ( v1 >= 512 )
      v3 = -1.0;
    else
      v3 = 1.0;
    flt_16D4850[v1] = v3;
    v4 = v11 * 0.0009765625;
    flt_16D6850[v1] = v4;
    flt_16D7850[v1] = 1.0 - v4;
    if ( v1 >= 512 )
    {
      v5 = -flt_16D5050[v1];
    }
    else if ( v1 >= 256 )
    {
      v5 = 1.0 - flt_16D5450[v1];
    }
    else
    {
      v5 = v11 * 0.00390625;
    }
    flt_16D5850[v1++] = v5;
    v10 = v1;
  }
  while ( v1 < 1024 );
  Com_NoiseInit();
  R_Register();
  max_polys = r_maxpolys->integer;
  v6 = max_polys;
  if ( max_polys < 4096 )
  {
    v6 = 4096;
    max_polys = 4096;
  }
  max_polyverts = r_maxpolyverts->integer;
  v7 = max_polyverts;
  if ( max_polyverts < 0x2000 )
  {
    v7 = 0x2000;
    max_polyverts = 0x2000;
  }
  backEndData = ri_Hunk_Alloc(16 * (v6 + 2 * v7) + 1898244);
  *(_DWORD *)(backEndData + 1898240) = 0;
  r_firstSceneDrawSurf = 0;
  r_numdlights = 0;
  r_firstSceneDlight = 0;
  r_numcoronas = 0;
  r_firstSceneCorona = 0;
  r_numentities = 0;
  r_firstSceneEntity = 0;
  r_numpolys = 0;
  r_firstScenePoly = 0;
  r_numpolyverts = 0;
  InitOpenGL();
  R_InitAllocators();
  memset(hashtable, 0, sizeof(hashtable));
  R_SetColorMappings();
  R_CreateBuiltinImages();
  R_DeleteVertexPrograms();
  R_InitShaders(a1);
  tr_numModels = 0;
  v8 = ri_Hunk_Alloc(96);
  *(_DWORD *)(v8 + 68) = tr_numModels;
  tr_models[tr_numModels++] = v8;
  *(_DWORD *)(v8 + 64) = 0;
  dword_14072F4 = 0;
  dword_14072F8 = 0;
  R_SetHwLightGlobals();
  memset(&tr_lightVisCache, 0, 0x200000u);
  lightVisCache_maxAssociativity = 0;
  lightVisCache_entriesUsed = 0;
  lightVisCache_entriesFlushed = 0;
  lightVisCache_entriesFilledAtRuntime = 0;
  v9 = qglGetError_0();
  if ( v9 )
    ri_Printf(0, "glGetError() = 0x%x\n", v9);
  R_InitDebug();
  xmodel_animCheck = 0;
  s_numWaterMaps = 0;
  FFT_Init();
  ri_Printf(0, "----- finished R_Init -----\n");
}

/* ---- R_DeleteFragmentShaders  0x004B4830 ----  [HIGH] */
int R_DeleteFragmentShaders()
{
  int result;
  int i;

  result = glConfig_ATIFragmentShader;
  if ( glConfig_ATIFragmentShader )
  {
    if ( glState_boundFragmentShader )
    {
      glBindFragmentShaderATI(0);
      glState_boundFragmentShader = 0;
    }
    result = dword_16D384C;
    for ( i = 1; i <= dword_16D384C; ++i )
    {
      qglDeleteFragmentShaderATI(i);
      result = dword_16D384C;
    }
    dword_16D384C = 0;
    glState_boundFragmentShader = 0;
  }
  return result;
}

/* ---- RE_Shutdown  0x004B4890 ----  [HIGH] */
void __cdecl RE_Shutdown(int a1)
{
  int v1;

  ri_Printf(0, "RE_Shutdown( %i )\n", a1);
  ri_Cmd_RemoveCommand("modellist");
  ri_Cmd_RemoveCommand("screenshotJPEG");
  ri_Cmd_RemoveCommand("screenshot");
  ri_Cmd_RemoveCommand("imagelist");
  ri_Cmd_RemoveCommand("shaderlist");
  ri_Cmd_RemoveCommand("skinlist");
  ri_Cmd_RemoveCommand("gfxinfo");
  ri_Cmd_RemoveCommand("modelist");
  ri_Cmd_RemoveCommand("shaderstate");
  ri_Cmd_RemoveCommand("taginfo");
  ri_Cmd_RemoveCommand("cropimages");
  ri_Cmd_RemoveCommand("r_meminfo");
  ri_Cmd_RemoveCommand("r_vc_stats");
  ri_Cmd_RemoveCommand("r_smc_stats");
  ri_Cmd_RemoveCommand("r_smc_flush");
  ri_Cmd_RemoveCommand("r_loadsun");
  ri_Cmd_RemoveCommand("r_savesun");
  ri_Cmd_RemoveCommand("r_sunhelp");
  ri_Cmd_RemoveCommand("r_vbo_refresh");
  if ( tr_registered )
  {
    R_DeleteTextures();
    R_DeleteVertexPrograms();
    R_DeleteFragmentShaders();
    R_DeleteBuffersARB(v1);
  }
  R_SaveLightVisHistory();
  dword_14072F4 = 0;
  dword_14072F8 = 0;
  R_ShutdownAllocators();
  R_ShutdownStaticModels();
  if ( a1 )
    GLimp_Shutdown();
  R_ShutdownDebug();
  tr_registered = 0;
}

/* ---- RE_EndRegistration  0x004B49E0 ----  [HIGH] */
void __cdecl RE_EndRegistration(const char *a1)
{
  cvar_t *v1;
  _DWORD *v2;

  if ( tr_registered )
  {
    v1 = r_skipBackEnd;
    v2 = (_DWORD *)(backEndData + 1636096);
    *(_DWORD *)((char *)v2 + *(_DWORD *)(backEndData + 1898240)) = 0;
    v2[0x10000] = 0;
    if ( !v1->integer )
      RB_ExecuteRenderCommands(v2, a1);
  }
  if ( sys_sysMBValue > 100663296 )   /* 0x6000000 == 96 MB */
    RB_ShowImages();
}

/* ---- GetRefAPI  0x004B4A30 ----  [CONFIRMED] */
int *__cdecl GetRefAPI(const void *a1, int a2)
{
  (void)a1;
  if ( a2 == 14 )
  {
    re_Shutdown = (int)RE_Shutdown;
    BeginRegistration = (int)RE_BeginRegistration;
    GetXModelByHandle = (int)RE_GetXModelByHandle;
    RegisterModel = (int)RE_RegisterModel;
    GetShaderFromModel = (int)RE_GetShaderFromModel;
    GetImageMemory = (int)RE_GetImageMemory;
    RegisterShader = (int)RE_RegisterShader;
    RegisterShaderNoMip = (int)RE_RegisterShaderNoMip;
    LoadWorldMap = (int)RE_LoadWorldMap;
    FinishLoadingModels = (int)RE_FinishLoadingModels;
    SetIgnorePrecacheErrors = (int)RE_SetIgnorePrecacheErrors;
    GetIgnorePrecacheErrors = (int)RE_GetIgnorePrecacheErrors;
    EndRegistration = (int)RE_EndRegistration;
    GetShaderName = (int)RE_GetShaderName;
    GetFarPlaneDist = (int)RE_GetFarPlaneDist;
    BeginFrame = (int)&RE_BeginFrame;
    EndFrame = (int)RE_EndFrame;
    SaveScreen = (int)RE_SaveScreen;
    BlendSavedScreen = (int)RE_BlendSavedScreen;
    MarkFragments = (int)RE_MarkFragments;
    ModelBounds = (int)R_ModelBounds;
    ClearScene = (int)RE_ClearScene;
    AddRefEntityToScene = (int)RE_AddRefEntityToScene;
    AddPolyToScene = (int)RE_AddPolyToScene;
    AddPolysToScene = (int)RE_AddPolysToScene;
    AddLightToScene = (int)RE_AddLightToScene;
    AddCoronaToScene = (int)RE_AddCoronaToScene;
    SetFarPlaneDist = (int)RE_SetFarPlaneDist;   /* retail 0x004B4B78 */
    SetFog = (int)R_SetFog;
    SaveFogState = (int)RE_SaveFogState;
    RestoreFogState = (int)RE_RestoreFogState;
    RenderScene = (int)RE_RenderScene;
    ClearFlares = (int)RE_ClearFlares;
    SetColor = (int)RE_SetColor;
    StretchPic = (int)RE_StretchPic;
    StretchPicGradient = (int)RE_StretchPicGradient;
    StretchPicRotate = (int)RE_StretchPicRotate;
    DrawQuadPic = (int)RE_DrawQuadPic;
    StretchRaw = (int)RE_StretchRaw;
    UploadCinematic = (int)RE_UploadCinematic;
    RegisterFont = (int)RE_RegisterFont;
    GetEntityToken = (int)R_GetEntityToken;
    ResetImageAllocations = (int)R_ResetImageAllocations;
    FreeImageAllocations = (int)R_FreeImageAllocations;
    CubemapShot = (int)RE_CubemapShot;
    CubemapWaterShot = (int)RE_CubemapWaterShot;
    LocateDebugStrings = (int)RE_LocateDebugStrings;
    LocateDebugLines = (int)RE_LocateDebugLines;
    AddPlume = (int)RE_AddPlume;
    TrackStatistics = (int)RE_TrackStatistics;
    PickShader = (int)RE_PickShader;
    Text_Width = (int)RE_Text_Width;
    Text_Height = (int)RE_Text_Height;
    Text_Paint = (int)RE_Text_Paint;
    Text_ConsoleWidth = (int)RE_Text_ConsoleWidth;
    Text_ConsolePaint = (int)RE_Text_ConsolePaint;
    Text_PaintWithCursor = (int)RE_Text_PaintWithCursor;
    return &re_Shutdown;
  }
  else
  {
    ri_Printf(0, "Mismatched REF_API_VERSION: expected %i, got %i\n", 14, a2);
    return 0;
  }
}

