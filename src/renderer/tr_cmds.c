/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_shaderregistry.h"
#include "tr_gl_types.h"
#include "tr_tess.h"

void CM_Trace( trace_t *results, const float *start, const float *end,
               const float *mins, const float *maxs, int model,
               const float *origin, int brushmask, int capsule,
               const void *sphere );
extern int GL_TextureMode();
extern int RB_ExecuteRenderCommands();
extern int R_IncrementalRefreshOptimizedWorldSurfaces_ARB();
extern int R_IncrementalRefreshStaticModels_ARB();
extern int R_IncrementalRefreshXModels_ARB();
extern int R_SetColorMappings();
extern int R_SetHwLightGlobals();
extern int R_SetNVFogMode();
extern int R_SumOfUsedImages();

static const float colorWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

/* ---- R_PerformanceCounters  0x004DD920 ----  VERIFIED */
int R_PerformanceCounters()
{
  int v0;
  int integer;
  int result;
  int v3[2]; // [esp+28h] [ebp-10h] BYREF
  int v4; // [esp+30h] [ebp-8h] BYREF
  int v5; // [esp+34h] [ebp-4h] BYREF


  if ( com_statmon->integer )
    R_SumOfUsedImages(0, 0, 0);
  if ( tr_performanceCounters )
  {
    *(_DWORD *)tr_performanceCounters = backEnd_pc_indexCount;
    *(_DWORD *)(tr_performanceCounters + 4) = backEnd_pc_drawnIndexCount;
    *(_DWORD *)(tr_performanceCounters + 8) = backEnd_pc_vertexCount;
    *(_DWORD *)(tr_performanceCounters + 12) = backEnd_pc_drawCallCount;
    R_SumOfUsedImages((int *)(tr_performanceCounters + 16), (_DWORD *)(tr_performanceCounters + 20), (_DWORD *)(tr_performanceCounters + 24));
    v3[0] = dwStyle * dwExStyle;
    v0 = r_numentities;
    *(float *)(tr_performanceCounters + 32) = backEnd_pc_overdrawSum / (double)(int)(dwStyle * dwExStyle);
    *(_DWORD *)(tr_performanceCounters + 28) = v0;
  }
  integer = r_speeds->integer;
  if ( integer )
  {
    switch ( integer )
    {
      case 1:
        R_SumOfUsedImages(v3, &v4, &v5);
        v3[1] = dwStyle * dwExStyle;
        v4 = v3[0] - v4;
        ri_Printf(
          0,
          "%i/%i shaders/surfs %i leafs %i verts %i/%i tris %.2f/%.2f/%.2f MB %.2f dc\n",
          backEnd_pc_shaderCount,
          backEnd_pc_surfaceCount,
          tr_pc_leafCount,
          backEnd_pc_vertexCount,
          backEnd_pc_indexCount / 3,
          backEnd_pc_drawnIndexCount / 3,
          (double)v5 * 0.00000095367432,
          (double)v4 * 0.00000095367432,
          (double)v3[0] * 0.00000095367432,
          backEnd_pc_overdrawSum / (double)(int)(dwStyle * dwExStyle));
        break;
      case 2:
        ri_Printf(
          0,
          "(patch) %i sin %i sclip  %i sout %i bin %i bclip %i bout\n",
          (_DWORD)qword_16C57EC,
          HIDWORD(qword_16C57EC),
          (_DWORD)qword_16C57F4,
          HIDWORD(qword_16C57F4),
          (_DWORD)qword_16C57FC,
          HIDWORD(qword_16C57FC));
        break;
      case 3:
        ri_Printf(0, "viewcluster: %i\n", tr_viewCluster);
        break;
      case 4:
        if ( backEnd_pc_dlightVertexCount )
          ri_Printf(
            0,
            "dlight srf:%i  culled:%i  verts:%i  tris:%i\n",
            (_DWORD)qword_16C5808,
            HIDWORD(qword_16C5808),
            backEnd_pc_dlightVertexCount,
            backEnd_pc_dlightIndexCount / 3);
        break;
      case 6:
        ri_Printf(
          0,
          "flare adds:%i tests:%i renders:%i\n",
          backEnd_pc_flareAddCount,
          (_DWORD)backEnd_pc_flareTests,
          HIDWORD(backEnd_pc_flareTests));
        break;
    }
  }
  result = 0;
  qword_16C57EC = 0.0;  qword_16C57F4 = 0.0;  qword_16C57FC = 0.0;
  tr_pc_leafCount = 0;    qword_16C5808 = 0.0;
  backEnd_pc_surfaceCount = 0;  backEnd_pc_shaderCount = 0;  backEnd_pc_vertexCount = 0;  backEnd_pc_indexCount = 0;
  backEnd_pc_drawnIndexCount = 0;  backEnd_pc_drawCallCount = 0;  backEnd_pc_overdrawSum = 0;    backEnd_pc_dlightVertexCount = 0;
  backEnd_pc_dlightIndexCount = 0;  backEnd_pc_flareAddCount = 0;  backEnd_pc_flareTests = 0.0;
  backEnd_pc_msec = 0;
  return result;
}

/* ---- R_IssueRenderCommands  0x004DDBA0 ----  VERIFIED */
int __cdecl R_IssueRenderCommands(int runPerformanceCounters)
{
  _DWORD *v2;
  int result;

  v2 = (_DWORD *)(backEndData + 1636096);
  *(_DWORD *)((char *)v2 + *(_DWORD *)(backEndData + 1898240)) = 0;
  v2[0x10000] = 0;
  if ( runPerformanceCounters )
    R_PerformanceCounters();
  result = r_skipBackEnd->integer;
  if ( !result )
    return RB_ExecuteRenderCommands(v2, 0);
  return result;
}

/* ---- R_SyncRenderThread  0x004DDBF0 ----  VERIFIED */
_DWORD *__cdecl R_SyncRenderThread(void)
{
  _DWORD *result;
  cvar_t *v2;

  result = (_DWORD *)tr_registered;
  if ( tr_registered )
  {
    v2 = r_skipBackEnd;
    result = (_DWORD *)(backEndData + 1636096);
    *(_DWORD *)((char *)result + *(_DWORD *)(backEndData + 1898240)) = 0;
    result[0x10000] = 0;
    if ( !v2->integer )
      return (_DWORD *)RB_ExecuteRenderCommands(result, 0);
  }
  return result;
}

/* ---- R_GetCommandBuffer  0x004DDC30 ----  VERIFIED */
unsigned int __fastcall R_GetCommandBuffer(int a1, unsigned int a2)
{
  int v2;
  unsigned int v3;

  v2 = backEndData + 1636096;
  v3 = a2 + *(_DWORD *)(backEndData + 1898240);
  if ( v3 <= 0x3FFF8 )
  {
    *(_DWORD *)(backEndData + 1898240) = v3;
    return v2 + v3 - a2;
  }
  else
  {
    if ( a2 > 0x3FFF8 )
      ri_Error(0, "\x15" "R_GetCommandBuffer: bad size %i", a2);
    return 0;
  }
}

/* ---- R_AddDrawSurfCmd  0x004DDC80 ----  VERIFIED */
_DWORD *__cdecl R_AddDrawSurfCmd(int a1, int a2)
{
  int v2;
  _DWORD *result;

  v2 = backEndData;
  result = (_DWORD *)(*(_DWORD *)(backEndData + 1898240) + 1012);
  if ( (unsigned int)result <= 0x3FFF8 )
  {
    *(_DWORD *)(backEndData + 1898240) = result;
    result = (_DWORD *)((char *)result + v2 + 1635084);
    if ( result )
    {
      result[251] = a2;
      *result = 7;
      result[252] = a1;
      qmemcpy(result + 1, &tr_refdef_x, 0x188u);
      qmemcpy(result + 99, &tr_viewParms_originX, 0x260u);
    }
  }
  return result;
}

/* ---- RE_SetColor  0x004DDCF0 ----  VERIFIED */
int __cdecl RE_SetColor(float *a1)
{
  int v1;
  unsigned __int64 v2; // rax
  int v3;
  float *v4;

  v1 = backEndData;
  LODWORD(v2) = *(_DWORD *)(backEndData + 1898240) + 8;
  if ( (unsigned int)v2 <= 0x3FFF8 )
  {
    v3 = v2 + backEndData + 1636088;
    *(_DWORD *)(backEndData + 1898240) = v2;
    if ( (_DWORD)v2 + v1 != -1636088 )
    {
      v4 = a1;
      *(_DWORD *)v3 = 1;
      if ( !a1 )
        v4 = (float *)colorWhite;
      *(_BYTE *)(v3 + 4) = (unsigned __int64)(*v4 * 255.0);
      *(_BYTE *)(v3 + 5) = (unsigned __int64)(v4[1] * 255.0);
      *(_BYTE *)(v3 + 6) = (unsigned __int64)(v4[2] * 255.0);
      v2 = (unsigned __int64)(v4[3] * 255.0);
      *(_BYTE *)(v3 + 7) = v2;
    }
  }
  return v2;
}

/* ---- RE_StretchPic  0x004DDD80 ----  VERIFIED */
unsigned int __cdecl RE_StretchPic(int a1, int a2, int a3, int a4, int a5, int a6, unsigned int a7, int a8, int a9)
{
  int v9;
  unsigned int result;
  _DWORD *v11;
  char *v12;

  v9 = backEndData;
  result = *(_DWORD *)(backEndData + 1898240) + 40;
  if ( result <= 0x3FFF8 )
  {
    v11 = (_DWORD *)(result + backEndData + 1636056);
    *(_DWORD *)(backEndData + 1898240) = result;
    if ( result + v9 != -1636056 )
    {
      *v11 = 2;
      if ( a9 >= 0 && a9 < tr_numShaders[0] )
      {
        v12 = (char *)tr_shaders[a9];
      }
      else
      {
        ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", a9);
        v12 = tr_defaultShader;
      }
      v11[1] = v12;
      v11[2] = a1;
      v11[3] = a2;
      v11[4] = a3;
      v11[5] = a4;
      result = a7;
      v11[6] = a5;
      v11[7] = a6;
      v11[8] = a7;
      v11[9] = a8;
    }
  }
  return result;
}

/* ---- RE_StretchPicGradient  0x004DDE20 ----  VERIFIED */
int __cdecl RE_StretchPicGradient(
        int a1,
        int a2,
        int a3,
        int a4,
        int a5,
        int a6,
        int a7,
        int a8,
        int a9,
        float *a10,
        int a11)
{
  int v11;
  unsigned __int64 v12; // rax
  int v13;
  char *v14;
  float *v15;

  v11 = backEndData;
  LODWORD(v12) = *(_DWORD *)(backEndData + 1898240) + 48;
  if ( (unsigned int)v12 <= 0x3FFF8 )
  {
    v13 = v12 + backEndData + 1636048;
    *(_DWORD *)(backEndData + 1898240) = v12;
    if ( (_DWORD)v12 + v11 != -1636048 )
    {
      *(_DWORD *)v13 = 3;
      if ( a9 >= 0 && a9 < tr_numShaders[0] )
      {
        v14 = (char *)tr_shaders[a9];
      }
      else
      {
        ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", a9);
        v14 = tr_defaultShader;
      }
      *(_DWORD *)(v13 + 4) = v14;
      *(_DWORD *)(v13 + 8) = a1;
      *(_DWORD *)(v13 + 12) = a2;
      v15 = a10;
      *(_DWORD *)(v13 + 16) = a3;
      *(_DWORD *)(v13 + 20) = a4;
      *(_DWORD *)(v13 + 24) = a5;
      *(_DWORD *)(v13 + 28) = a6;
      *(_DWORD *)(v13 + 32) = a7;
      *(_DWORD *)(v13 + 36) = a8;
      if ( !a10 )
        v15 = (float *)colorWhite;
      *(_BYTE *)(v13 + 40) = (unsigned __int64)(*v15 * 255.0);
      *(_BYTE *)(v13 + 41) = (unsigned __int64)(v15[1] * 255.0);
      *(_BYTE *)(v13 + 42) = (unsigned __int64)(v15[2] * 255.0);
      v12 = (unsigned __int64)(v15[3] * 255.0);
      *(_BYTE *)(v13 + 43) = v12;
      *(_DWORD *)(v13 + 44) = a11;
    }
  }
  return v12;
}

/* ---- RE_StretchPicRotate  0x004DDF20 ----  VERIFIED */
unsigned int __cdecl RE_StretchPicRotate(
        int a1,
        int a2,
        int a3,
        int a4,
        int a5,
        int a6,
        int a7,
        int a8,
        float a9,
        int a10)
{
  int v10;
  unsigned int result;
  int v12;
  char *v13;
  float v14;

  v10 = backEndData;
  result = *(_DWORD *)(backEndData + 1898240) + 44;
  if ( result <= 0x3FFF8 )
  {
    v12 = result + backEndData + 1636052;
    *(_DWORD *)(backEndData + 1898240) = result;
    if ( result + v10 != -1636052 )
    {
      *(_DWORD *)v12 = 4;
      if ( a10 >= 0 && a10 < tr_numShaders[0] )
      {
        v13 = (char *)tr_shaders[a10];
      }
      else
      {
        ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", a10);
        v13 = tr_defaultShader;
      }
      *(_DWORD *)(v12 + 4) = v13;
      *(_DWORD *)(v12 + 8) = a1;
      *(_DWORD *)(v12 + 12) = a2;
      *(_DWORD *)(v12 + 16) = a3;
      *(_DWORD *)(v12 + 20) = a4;
      *(_DWORD *)(v12 + 24) = a5;
      *(_DWORD *)(v12 + 28) = a6;
      *(_DWORD *)(v12 + 32) = a7;
      *(_DWORD *)(v12 + 36) = a8;
      result = (unsigned __int16)(unsigned __int64)(a9 * 182.04445);
      v14 = (float)(int)result;
      *(float *)(v12 + 40) = v14 * 0.0054931641;
    }
  }
  return result;
}

/* ---- RE_DrawQuadPic  0x004DE000 ----  VERIFIED */
unsigned int __cdecl RE_DrawQuadPic(_DWORD *a1, _DWORD *a2, int a3)
{
  int v3;
  unsigned int result;
  _DWORD *v5;
  char *v6;

  v3 = backEndData;
  result = *(_DWORD *)(backEndData + 1898240) + 72;
  if ( result <= 0x3FFF8 )
  {
    v5 = (_DWORD *)(result + backEndData + 1636024);
    *(_DWORD *)(backEndData + 1898240) = result;
    if ( result + v3 != -1636024 )
    {
      *v5 = 5;
      if ( a3 >= 0 && a3 < tr_numShaders[0] )
      {
        v6 = (char *)tr_shaders[a3];
      }
      else
      {
        ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", a3);
        v6 = tr_defaultShader;
      }
      v5[1] = v6;
      v5[2] = *a1;
      v5[3] = a1[1];
      v5[10] = *a2;
      v5[11] = a2[1];
      v5[4] = a1[2];
      v5[5] = a1[3];
      v5[12] = a2[2];
      v5[13] = a2[3];
      v5[6] = a1[4];
      v5[7] = a1[5];
      v5[14] = a2[4];
      v5[15] = a2[5];
      v5[8] = a1[6];
      v5[9] = a1[7];
      v5[16] = a2[6];
      result = a2[7];
      v5[17] = result;
    }
  }
  return result;
}

/* ---- RE_BeginFrame  0x004DE0D0 ----  VERIFIED */
void __cdecl RE_BeginFrame( int stereoFrame )
{
  cvar_t *cv;              /* eax / ecx -- the cvar under test */
  const char *s;           /* esi */
  int err;                 /* eax */
  int base;                /* ecx -- backEndData */
  unsigned int used;       /* eax */
  _DWORD *cmd;             /* esi */

  if ( !tr_registered )
    return;

  ++tr_frameCount;
  glState_finishCalled = 0;
  tr_frameSceneNum = 0;

  if ( r_measureOverdraw->integer )
  {
    if ( glConfig_stencilBits < 4 )
    {
      ri_Printf(0, "Warning: not enough stencil bits to measure overdraw: %d\n", glConfig_stencilBits);
      ri_Cvar_Set("r_measureOverdraw", "0");
    }
    else if ( cg_shadows->integer == 2 )
    {
      ri_Printf(0, "Warning: stencil shadows and overdraw measurement are mutually exclusive\n");
      ri_Cvar_Set("r_measureOverdraw", "0");
    }
    else
    {
      R_SyncRenderThread();
      qglEnable(0xB90u);
      qglStencilMask(0xFFFFFFFFu);
      qglClearStencil(0);
      qglStencilFunc(0x207u, 0, 0xFFFFFFFFu);
      qglStencilOp(0x1E00u, 0x1E02u, 0x1E02u);
    }
  }
  else if ( r_measureOverdraw->modified )
  {
    R_SyncRenderThread();
    qglDisable(0xB90u);
  }
  r_measureOverdraw->modified = 0;

  if ( r_textureMode->modified )
  {
    R_SyncRenderThread();
    GL_TextureMode((int)r_textureMode->string);
    r_textureMode->modified = 0;
  }

  R_SetHwLightGlobals();

  if ( qglPNTrianglesiATI )
  {
    cv = r_ati_truform_tess;
    if ( cv->modified )
    {
      cv->modified = 0;
      if ( (double)glConfig_maxPNTrianglesTessellationLevel < cv->value )
      {
        ri_Cvar_Set("r_ati_truform_tess", va("%d", glConfig_maxPNTrianglesTessellationLevel));
        cv = r_ati_truform_tess;
      }
      /* 0x87F4 GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI */
      qglPNTrianglesiATI(0x87F4u, cv->integer);
    }

    cv = r_ati_truform_pointmode;
    if ( cv->modified )
    {
      cv->modified = 0;
      s = cv->string;
      if ( s && !Q_stricmpn(s, "LINEAR", 99999) )
      {
        glConfig_pnTrianglesPointMode = 0x87F5;
      }
      else if ( s && !Q_stricmpn(s, "CUBIC", 99999) )
      {
        glConfig_pnTrianglesPointMode = 0x87F6;
      }
      else
      {
        glConfig_pnTrianglesPointMode = 0x87F6;
        ri_Cvar_Set("r_ati_truform_pointmode", "LINEAR");
      }
      qglPNTrianglesiATI(0x87F2u, glConfig_pnTrianglesPointMode);
    }

    cv = r_ati_truform_normalmode;
    if ( cv->modified )
    {
      cv->modified = 0;
      s = cv->string;
      if ( s && !Q_stricmpn(s, "LINEAR", 99999) )
      {
        glConfig_pnTrianglesNormalMode = 0x87F7;
      }
      else if ( s && !Q_stricmpn(s, "QUADRATIC", 99999) )
      {
        glConfig_pnTrianglesNormalMode = 0x87F8;
      }
      else
      {
        glConfig_pnTrianglesNormalMode = 0x87F7;
        ri_Cvar_Set("r_ati_truform_normalmode", "LINEAR");
      }
      qglPNTrianglesiATI(0x87F3u, glConfig_pnTrianglesNormalMode);
    }
  }

  if ( glConfig_NVFogAvailable && r_nv_fogdist_mode->modified )
  {
    r_nv_fogdist_mode->modified = 0;
    R_SetNVFogMode();
  }

  if ( r_gamma->modified )
  {
    r_gamma->modified = 0;
    R_SyncRenderThread();
    R_SetColorMappings();
  }

  if ( glConfig_ARBVertexBufferObject && r_vbo_paranoia->integer )
  {
    R_IncrementalRefreshOptimizedWorldSurfaces_ARB();
    R_IncrementalRefreshStaticModels_ARB(3);
    R_IncrementalRefreshXModels_ARB(3);
  }

  if ( !r_ignoreGLErrors->integer )
  {
    R_SyncRenderThread();
    err = qglGetError();
    if ( err )
      ri_Error(0, "\x15" "RE_BeginFrame() - glGetError() failed (0x%x)!\n", err);
  }

  base = backEndData;
  used = *(_DWORD *)(backEndData + 1898240) + 8;
  if ( used > 0x3FFF8 )
    return;
  cmd = (_DWORD *)(used + base + 1636088);
  *(_DWORD *)(base + 1898240) = used;
  if ( !cmd )                          /* `cmp esi,edi / jz` at 0x004DE415 */
    return;

  *cmd = 8;

  if ( glConfig_stereoEnabled )
  {
    if ( stereoFrame == 1 )
      cmd[1] = 0x402;
    else if ( stereoFrame == 2 )
      cmd[1] = 0x403;
    else
      ri_Error(0, "\x15" "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame);
  }
  else
  {
    if ( stereoFrame )
      ri_Error(0, "\x15" "RE_BeginFrame: Stereo is disabled, but stereoFrame was %i", stereoFrame);
    if ( r_drawBuffer->string && !Q_stricmpn(r_drawBuffer->string, "GL_FRONT", 99999) )
      cmd[1] = 0x404;
    else
      cmd[1] = 0x405;
  }
}

/* ---- RE_EndFrame  0x004DE4B0 ----  VERIFIED */
_DWORD *__cdecl RE_EndFrame(const char *a1, _DWORD *a2, _DWORD *a3)
{
  _DWORD *result;
  int v4;
  _DWORD *v5;

  result = (_DWORD *)tr_registered;
  if ( tr_registered )
  {
    v4 = backEndData;
    *(_DWORD *)(*(_DWORD *)(backEndData + 1898240) + backEndData + 1636096) = 11;
    v5 = (_DWORD *)(v4 + 1636096);
    *(_DWORD *)(v4 + 1898240) += 4;
    *(_DWORD *)(v4 + 1636096 + *(_DWORD *)(v4 + 1898240)) = 0;
    *(_DWORD *)(v4 + 1898240) = 0;
    R_PerformanceCounters();
    if ( !r_skipBackEnd->integer )
      RB_ExecuteRenderCommands(v5, a1);
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
    if ( a2 )
      *a2 = tr_frontEndMsec;
    result = a3;
    tr_frontEndMsec = 0;
    if ( a3 )
      *a3 = backEnd_pc_msec;
    backEnd_pc_msec = 0;
  }
  return result;
}

/* ---- RE_SaveScreen  0x004DE590 ----  VERIFIED */
_DWORD *RE_SaveScreen()
{
  int v0;
  _DWORD *result;

  v0 = backEndData;
  result = (_DWORD *)(*(_DWORD *)(backEndData + 1898240) + 4);
  if ( (unsigned int)result <= 0x3FFF8 )
  {
    *(_DWORD *)(backEndData + 1898240) = result;
    result = (_DWORD *)((char *)result + v0 + 1636092);
    if ( result )
      *result = 9;
  }
  return result;
}

/* ---- RE_BlendSavedScreen  0x004DE5C0 ----  VERIFIED */
void __cdecl RE_BlendSavedScreen(int a1)
{
  int v1;
  unsigned int v2;
  _DWORD *v3;

  if ( a1 > 0 )
  {
    v1 = backEndData;
    v2 = *(_DWORD *)(backEndData + 1898240) + 8;
    if ( v2 <= 0x3FFF8 )
    {
      *(_DWORD *)(backEndData + 1898240) = v2;
      v3 = (_DWORD *)(v2 + v1 + 1636088);
      if ( v3 )
      {
        *v3 = 10;
        v3[1] = a1;
      }
    }
  }
}

/* ---- RE_TrackStatistics  0x004DE600 ----  VERIFIED */
int __cdecl RE_TrackStatistics(int a1)
{
  int result;

  result = a1;
  tr_performanceCounters = a1;
  return result;
}

typedef struct {
    const char *name;
    int         clearSolid;
    int         surfaceFlags;
    int         contents;
} tr_infoParm_t;

#define tr_surfaceParms   ((const tr_infoParm_t *)(void *)&surfacetype_bark)
#define SURFACEPARM_FIRST 22    /* 0x00571900 = surfacetype_bark + 22*16 */

/* ---- RE_PickShader  0x004DE610 ----  VERIFIED */
int __cdecl RE_PickShader( float *start, float *dir, char *shaderName,
                           char *surfaceNames, char *contentsNames, size_t size )
{
  trace_t trace;
  float   end[3];
  const tr_infoParm_t *parm;
  int     surfaceType;
  size_t  surfaceLen;
  size_t  contentsLen;

  end[0] = dir[0] * 262144.0 + start[0];
  trace.fraction = 1.0;
  end[1] = dir[1] * 262144.0 + start[1];
  end[2] = dir[2] * 262144.0 + start[2];

  CM_Trace( &trace, start, end, 0, 0, 0, vec3_origin, 260308983, 0, 0 );

  if ( trace.allsolid || trace.startsolid )
    return 0;
  if ( trace.fraction == 1.0 )
    return 0;
  if ( !trace.material )
    return 0;

  /* +0x24 holds the shader NAME POINTER in 1.1. */
  strcpy( shaderName, (const char *)trace.material );

  surfaceNames[0] = 0;
  surfaceNames[size - 1] = 0;
  contentsNames[0] = 0;
  contentsNames[size - 1] = 0;

  surfaceType = ( trace.surfaceFlags >> 20 ) & 0x1F;
  if ( surfaceType && surfaceType < 23 )
    strncpy( surfaceNames, (const char *)dword_571790[4 * surfaceType], size );
  else
    strncpy( surfaceNames, "^1default^7", size );

  if ( surfaceNames[size - 1] )
    return 0;
  surfaceLen = strlen( surfaceNames );

  if ( ( trace.contents & 1 ) != 0 )
    strncpy( contentsNames, "solid", size );
  else
    strncpy( contentsNames, "^3nonsolid^7", size );

  if ( contentsNames[size - 1] )
    return 0;
  contentsLen = strlen( contentsNames );

  for ( parm = &tr_surfaceParms[SURFACEPARM_FIRST]; parm->name; parm++ )
  {
    if ( ( trace.surfaceFlags & parm->surfaceFlags ) != 0 )
    {
      surfaceNames[surfaceLen] = ' ';
      surfaceLen++;
      strncpy( surfaceNames + surfaceLen, parm->name, size - surfaceLen );
      if ( surfaceNames[size - 1] )
        return 0;
      surfaceLen += strlen( surfaceNames + surfaceLen );
    }

    if ( ( trace.contents & parm->contents ) != 0 )
    {
      contentsNames[contentsLen] = ' ';
      contentsLen++;
      strncpy( contentsNames + contentsLen, parm->name, size - contentsLen );
      if ( contentsNames[size - 1] )
        return 0;
      contentsLen += strlen( contentsNames + contentsLen );
    }
  }

  return 1;
}
