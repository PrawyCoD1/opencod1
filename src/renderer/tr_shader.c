/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#define surfacetype_bark surfacetype_bark__cod1_hdr   /* cod1_globals.h types these int; real typed defs below */
#define off_540268       off_540268__cod1_hdr
#define off_5403E8       off_5403E8__cod1_hdr
#include "../qcommon/cod1_globals.h"
#undef surfacetype_bark
#undef off_540268
#undef off_5403E8
#include "tr_shaderregistry.h"
#include "tr_records.h"

static unsigned char tr_shaderParseScratch[0x79E8];

#define shaderParseTexMods        (*(unsigned char *)(tr_shaderParseScratch + 0x0000))
#define unk_11E2488        (*(unsigned char *)(tr_shaderParseScratch + 0x0110))
#define shaderParseName       ((unsigned char *)(tr_shaderParseScratch + 0x4400))
#define byte_11E67B7       (*(unsigned char *)(tr_shaderParseScratch + 0x443F))
#define shaderParseLightmapIndex      (*(int *)(tr_shaderParseScratch + 0x4440))
#define shaderParseFlags      (*(int *)(tr_shaderParseScratch + 0x444C))
#define shaderParseSurfaceFlags      (*(int *)(tr_shaderParseScratch + 0x4450))
#define shaderParseLightingFlags      (*(int *)(tr_shaderParseScratch + 0x4454))
#define shaderParseSort      (*(int *)(tr_shaderParseScratch + 0x4458))
#define shaderParseSurfaceParmFlags      (*(int *)(tr_shaderParseScratch + 0x445C))
#define shaderParseSkyCloudHeight        (*(float *)(tr_shaderParseScratch + 0x4460))
#define shaderParseFogColor        (*(unsigned char *)(tr_shaderParseScratch + 0x4494))
#define shaderParseFogDepthForOpaque        (*(float *)(tr_shaderParseScratch + 0x44A0))
#define shaderParsePortalRange      (*(int *)(tr_shaderParseScratch + 0x44A4))
#define shaderParseCullType      (*(int *)(tr_shaderParseScratch + 0x44A8))
#define shaderParseNumDeforms      (*(int *)(tr_shaderParseScratch + 0x44AC))
#define shaderParseDeforms        (*(unsigned char *)(tr_shaderParseScratch + 0x44B0))
#define byte_11E6888       (*(char *)(tr_shaderParseScratch + 0x4510))
#define shaderParseBoundsExpansion        (*(float *)(tr_shaderParseScratch + 0x454C))
#define shaderParseNumUnfoggedPasses      (*(int *)(tr_shaderParseScratch + 0x4550))
#define shaderParseStages      ((int *)(tr_shaderParseScratch + 0x4554))
#define shaderParseOptimalStageIteratorFunc      (*(int *)(tr_shaderParseScratch + 0x4574))
#define shaderParseClampTime        (*(float *)(tr_shaderParseScratch + 0x4578))
#define shaderParseOptimizedBackend      (*(int *)(tr_shaderParseScratch + 0x4588))
#define shaderParseStageStorage      ((int *)(tr_shaderParseScratch + 0x45A8))
#define dword_11E6924      ((int *)(tr_shaderParseScratch + 0x45AC))
#define unk_11E69A4        (*(unsigned char *)(tr_shaderParseScratch + 0x462C))
#define dword_11E69B0      (*(int *)(tr_shaderParseScratch + 0x4638))
#define dword_11E69B4      ((int *)(tr_shaderParseScratch + 0x463C))
#define unk_11E69C0        (*(unsigned char *)(tr_shaderParseScratch + 0x4648))
#define unk_11E69E0        (*(unsigned char *)(tr_shaderParseScratch + 0x4668))
#define byte_11E69E8       (*(unsigned char *)(tr_shaderParseScratch + 0x4670))
#define dword_11E69EC      (*(int *)(tr_shaderParseScratch + 0x4674))
#define dword_11E6A7C      (*(int *)(tr_shaderParseScratch + 0x4704))
#define unk_11E6AA8        (*(unsigned char *)(tr_shaderParseScratch + 0x4730))
#define byte_11E6AB0       (*(unsigned char *)(tr_shaderParseScratch + 0x4738))
#define dword_11E6AB4      (*(int *)(tr_shaderParseScratch + 0x473C))
#define unk_11E6F70        (*(unsigned char *)(tr_shaderParseScratch + 0x4BF8))
#define dword_11E6F84      (*(int *)(tr_shaderParseScratch + 0x4C0C))
#define unk_11E6F88        (*(unsigned char *)(tr_shaderParseScratch + 0x4C10))
#define dword_11E6F9C      (*(int *)(tr_shaderParseScratch + 0x4C24))
#define byte_11E6FA0       (*(unsigned char *)(tr_shaderParseScratch + 0x4C28))
#define byte_11E6FA1       (*(unsigned char *)(tr_shaderParseScratch + 0x4C29))
#define byte_11E6FA2       (*(unsigned char *)(tr_shaderParseScratch + 0x4C2A))
#define dword_11E6FA4      (*(int *)(tr_shaderParseScratch + 0x4C2C))
#define dword_11E6FA8      (*(int *)(tr_shaderParseScratch + 0x4C30))
#define dword_11E6FAC      (*(int *)(tr_shaderParseScratch + 0x4C34))
#define dword_11E7038      (*(int *)(tr_shaderParseScratch + 0x4CC0))
#define dword_11E703C      (*(int *)(tr_shaderParseScratch + 0x4CC4))
#define dword_11E7074      (*(int *)(tr_shaderParseScratch + 0x4CFC))
#define unk_11E75F8        (*(unsigned char *)(tr_shaderParseScratch + 0x5280))
#define dword_11E760C      (*(int *)(tr_shaderParseScratch + 0x5294))
#define unk_11E7610        (*(unsigned char *)(tr_shaderParseScratch + 0x5298))
#define dword_11E7624      (*(int *)(tr_shaderParseScratch + 0x52AC))
#define dword_11E762C      (*(int *)(tr_shaderParseScratch + 0x52B4))
#define unk_11E7630        (*(unsigned char *)(tr_shaderParseScratch + 0x52B8))
#define unk_11E96D8        (*(unsigned char *)(tr_shaderParseScratch + 0x7360))
#define skyOuterBox        ((int *)(tr_shaderParseScratch + 0x4464))
#define skyInnerBox        ((int *)(tr_shaderParseScratch + 0x447C))
#define rendererShaderRequirements ((unsigned char *)(tr_shaderParseScratch + 0x4598))
#define RENDERER_SHADER_REQUIREMENTS_COUNT 15

#include "tr_gl_types.h"
#include "tr_tess.h"

const char *skySuffix[6] = { "rt", "bk", "lf", "ft", "up", "dn" };  /* 0x005712D8 */

int shaderHashTable[4096];       /* 0x011E9D60, 16384 bytes */


typedef struct {
    const char  *token;
    unsigned int glRegister;
    unsigned int componentUsage;
} tr_nvRegisterDef_t;
#define tr_nvReadWriteRegisters  ((const char **)(void *)&off_540268)  /* 0x00540268, 32 x 12 */
#define tr_nvReadRegisters       ((const char **)(void *)&off_5403E8)  /* 0x005403E8, 20 x 12 */

/* === Merged from renderer/tr_shader_rdata.c (NV register tables) (retail linked it as tr_*.obj .rdata/.data). === */

/* ==========================================================================
 * off_540268              0x00540268        384 bytes (32 x 12)
 * off_5403E8              0x005403E8        240 bytes (20 x 12)
 * ==========================================================================
 *
 * The two GL_NV_register_combiners register-name tables.  Both are handed to
 * ParseNVRC_RegisterFromTable (0x004F4780), which walks them as `const char **`
 * with `i += 3` -- twelve bytes -- so the entry shape is
 *
 *     { const char *token; GLenum glRegister; GLenum componentUsage }
 *
 * Both call sites pass the count as an immediate: 32 in
 * ParseNVRC_ReadWriteRegister and `mov ebx, 14h` = 20 at 0x004F49AC in
 * ParseNVRC_ReadRegister.  32 x 12 = 384 ends the first table at 0x005403E8,
 * exactly where the second begins; 20 x 12 = 240 ends the second at
 * 0x005404D8, where the multitexture collapse table starts.  cod1_globals.h
 * sizes off_540268 at 972 bytes and off_5403E8 at 588 -- each measured to the
 * next NAMED symbol, straight through the other -- and neither declared size
 * is right.
 *
 * ParseNVRC_RegisterFromTable dereferences `*i` on its first iteration, so a
 * zeroed table does not merely fail to match: every register name in every
 * `nvRegCombiners` block reaches _stricmp with a NULL first argument.
 *
 * Component usage is GL_ALPHA 0x1906 / GL_RGB 0x1907 / GL_BLUE 0x1905, with
 * zero meaning "use the caller's default".  Registers are
 * GL_PRIMARY_COLOR_NV 0x852C, GL_SECONDARY_COLOR_NV 0x852D, GL_SPARE0_NV
 * 0x852E, GL_SPARE1_NV 0x852F and GL_TEXTURE0_ARB..+3 0x84C0..0x84C3 in the
 * read/write table; GL_FOG 0x0B60, GL_ZERO 0, GL_ONE 1 and
 * GL_CONSTANT_COLOR0/1_NV 0x852A/0x852B in the read-only table.
 */

tr_nvRegisterDef_t off_540268[32] = {
    { "col0",        0x852C, 0x0000 },   /* +0x000 */
    { "col0.a",      0x852C, 0x1906 },   /* +0x00C */
    { "col0.rgb",    0x852C, 0x1907 },   /* +0x018 */
    { "col0.b",      0x852C, 0x1905 },   /* +0x024 */
    { "col1",        0x852D, 0x0000 },   /* +0x030 */
    { "col1.a",      0x852D, 0x1906 },   /* +0x03C */
    { "col1.rgb",    0x852D, 0x1907 },   /* +0x048 */
    { "col1.b",      0x852D, 0x1905 },   /* +0x054 */
    { "spare0",      0x852E, 0x0000 },   /* +0x060 */
    { "spare0.a",    0x852E, 0x1906 },   /* +0x06C */
    { "spare0.rgb",  0x852E, 0x1907 },   /* +0x078 */
    { "spare0.b",    0x852E, 0x1905 },   /* +0x084 */
    { "spare1",      0x852F, 0x0000 },   /* +0x090 */
    { "spare1.a",    0x852F, 0x1906 },   /* +0x09C */
    { "spare1.rgb",  0x852F, 0x1907 },   /* +0x0A8 */
    { "spare1.b",    0x852F, 0x1905 },   /* +0x0B4 */
    { "tex0",        0x84C0, 0x0000 },   /* +0x0C0 */
    { "tex0.a",      0x84C0, 0x1906 },   /* +0x0CC */
    { "tex0.rgb",    0x84C0, 0x1907 },   /* +0x0D8 */
    { "tex0.b",      0x84C0, 0x1905 },   /* +0x0E4 */
    { "tex1",        0x84C1, 0x0000 },   /* +0x0F0 */
    { "tex1.a",      0x84C1, 0x1906 },   /* +0x0FC */
    { "tex1.rgb",    0x84C1, 0x1907 },   /* +0x108 */
    { "tex1.b",      0x84C1, 0x1905 },   /* +0x114 */
    { "tex2",        0x84C2, 0x0000 },   /* +0x120 */
    { "tex2.a",      0x84C2, 0x1906 },   /* +0x12C */
    { "tex2.rgb",    0x84C2, 0x1907 },   /* +0x138 */
    { "tex2.b",      0x84C2, 0x1905 },   /* +0x144 */
    { "tex3",        0x84C3, 0x0000 },   /* +0x150 */
    { "tex3.a",      0x84C3, 0x1906 },   /* +0x15C */
    { "tex3.rgb",    0x84C3, 0x1907 },   /* +0x168 */
    { "tex3.b",      0x84C3, 0x1905 }    /* +0x174 */
};

tr_nvRegisterDef_t off_5403E8[20] = {
    { "fog",         0x0B60, 0x0000 },   /* +0x000 */
    { "fog.a",       0x0B60, 0x1906 },   /* +0x00C */
    { "fog.rgb",     0x0B60, 0x1907 },   /* +0x018 */
    { "fog.b",       0x0B60, 0x1905 },   /* +0x024 */
    { "zero",        0x0000, 0x0000 },   /* +0x030 */
    { "zero.a",      0x0000, 0x1906 },   /* +0x03C */
    { "zero.rgb",    0x0000, 0x1907 },   /* +0x048 */
    { "zero.b",      0x0000, 0x1905 },   /* +0x054 */
    { "one",         0x0001, 0x0000 },   /* +0x060 */
    { "one.a",       0x0001, 0x1906 },   /* +0x06C */
    { "one.rgb",     0x0001, 0x1907 },   /* +0x078 */
    { "one.b",       0x0001, 0x1905 },   /* +0x084 */
    { "const0",      0x852A, 0x0000 },   /* +0x090 */
    { "const0.a",    0x852A, 0x1906 },   /* +0x09C */
    { "const0.rgb",  0x852A, 0x1907 },   /* +0x0A8 */
    { "const0.b",    0x852A, 0x1905 },   /* +0x0B4 */
    { "const1",      0x852B, 0x0000 },   /* +0x0C0 */
    { "const1.a",    0x852B, 0x1906 },   /* +0x0CC */
    { "const1.rgb",  0x852B, 0x1907 },   /* +0x0D8 */
    { "const1.b",    0x852B, 0x1905 }    /* +0x0E4 */
};

typedef struct {
    int srcStateBits;
    int dstStateBits;
    int texEnv;
    int collapsedStateBits;
} tr_collapseRule_t;

static const tr_collapseRule_t tr_collapseRules[9] = {
    { 0x00, 0x31, 0x2100, 0x00 },   /* 0x005404D8 */
    { 0x00, 0x13, 0x2100, 0x00 },   /* 0x005404E8 */
    { 0x13, 0x13, 0x2100, 0x13 },   /* 0x005404F8 */
    { 0x31, 0x13, 0x2100, 0x13 },   /* 0x00540508 */
    { 0x13, 0x31, 0x2100, 0x13 },   /* 0x00540518 */
    { 0x31, 0x31, 0x2100, 0x13 },   /* 0x00540528 */
    { 0x00, 0x22, 0x0104, 0x00 },   /* 0x00540538 */
    { 0x22, 0x22, 0x0104, 0x22 },   /* 0x00540548 */
    { 0x00, 0x65, 0x2101, 0x00 }    /* 0x00540558 */
};

static const int tr_texEnvArgSwap[6] = { 0, 1, 4, 5, 2, 3 };  /* 0x00540250 */

#define glConfig_maxActiveTextures  Value

extern int Com_ParseOnLine();
extern void Com_ParseReturnToMark( int *mark, char **data_p );
extern int Com_ScriptError();
extern int Com_SkipBracedSection();
extern int Com_SkipRestOfLine();
extern int Com_UngetToken();
extern int RB_ExecuteRenderCommands();
extern int RB_StageIteratorSky();
extern int __cdecl R_FindImageFile( char *name, int target, int flags, int track,
                                    int colorScale, float heightmapScale );
extern int R_FindImageInstance();
extern char *__cdecl R_FindVertexProgram( const char *name );
extern int R_GetWaterTexture();
extern int __cdecl R_InitSkyTexCoords( float cloudHeight );
extern int R_UpdateDelayLoadImage();
float VectorNormalize2D( float *v );
extern int j__atol();
extern int qsort_m();

/* ---- tr_shader_generateHashValue  0x004F0BA0 ----  VERIFIED */
int __cdecl tr_shader_generateHashValue(char *a1)
{
  char *v1;
  __int16 v2;
  __int16 v3;
  int v4;
  __int16 v5;
  char v6;

  v1 = a1;
  v2 = 0;
  if ( *a1 )
  {
    v3 = 119 - (_WORD)a1;
    do
    {
      v4 = tolower(*v1);
      if ( v4 == 46 )
        break;
      if ( v4 == 92 )
        LOWORD(v4) = 47;
      v5 = v4 * (v3 + (_WORD)v1);
      v6 = v1[1];
      v2 += v5;
      ++v1;
    }
    while ( v6 );
  }
  return v2 & 0xFFF;
}

/* ---- MergableShader  0x004F0BF0 ----  VERIFIED */
int __cdecl MergableShader(_DWORD *a1)
{
  int v1;
  int v3;
  int v4;
  unsigned int v5;
  int *v6;
  int v7;
  int v8;
  bool v9; // cc
  int v10;
  int v11;
  _DWORD *v12;
  int v13;
  _DWORD v14[256];

  v1 = a1[19];
  a1[97] = 0;
  if ( (v1 & 9) != 0 )
    return 0;
  v3 = a1[84];
  a1[19] = v1 | 0x800;
  v4 = 0;
  v13 = 0;
  if ( v3 > 0 )
  {
    v12 = a1 + 85;
    while ( !*(_DWORD *)(*v12 + 1612) )
    {
      v5 = 0;
      v6 = (int *)(*v12 + 4);
      do
      {
        v7 = *v6;
        if ( !*v6 )
          break;
        if ( v6[39] == 3 )
        {
          if ( v7 != tr_whiteImage || v6[32] > 1 )
          {
            v8 = a1[97];
            if ( v8 && v8 != v7 )
              goto LABEL_25;
            if ( v6[46] || v6[32] > 1 || v6[38] )
              return 0;
            a1[97] = v7;
          }
        }
        else
        {
          v14[v4++] = v7;
        }
        ++v5;
        v6 += 50;
      }
      while ( v5 < 8 );
      v9 = ++v13 < a1[84];
      ++v12;
      if ( !v9 )
        goto LABEL_20;
    }
LABEL_25:
    a1[97] = 0;
    return 0;
  }
LABEL_20:
  v10 = a1[97];
  if ( v10 )
  {
    v11 = 0;
    if ( v4 <= 0 )
      return 1;
    while ( v14[v11] != v10 )
    {
      if ( ++v11 >= v4 )
        return 1;
    }
    goto LABEL_25;
  }
  return 0;
}

/* ---- UpdateDelayLoadImagesForShader  0x004F0D20 ----  [HIGH] */
int __cdecl UpdateDelayLoadImagesForShader(_DWORD *a1, int a2)
{
  int result;
  _DWORD *v3;
  int v4;
  int v5;
  bool v6; // cc
  _DWORD *v7;
  unsigned int v8;
  int v9;

  result = a1[19];
  if ( (result & 0x800) != 0 )
  {
    if ( !tr_delayedImageGroup || tr_delayedImageGroupTileMode >= r_optimizeTextures->integer )
      a2 = 1;
    a1[19] = result & 0xFFFFF7FF;
    result = a1[84];
    v9 = 0;
    if ( result > 0 )
    {
      v7 = a1 + 85;
      do
      {
        v8 = 0;
        v3 = (_DWORD *)(*v7 + 4);
        do
        {
          if ( !*v3 )
            break;
          v4 = 0;
          do
          {
            v5 = v3[v4];
            R_UpdateDelayLoadImage(v5, (int)a1, a2);
            if ( *(char *)(v5 + 100) < 0 )
              a1[19] |= 0x800u;
            ++v4;
          }
          while ( v4 < v3[32] );
          v3 += 50;
          ++v8;
        }
        while ( v8 < 8 );
        result = v9 + 1;
        v6 = ++v9 < a1[84];
        ++v7;
      }
      while ( v6 );
    }
  }
  return result;
}

/* ---- CompareTexEnvCombineArgs  0x004F0E00 ----  [HIGH] */
int __fastcall CompareTexEnvCombineArgs(int a1, int a2)
{
  int result;

  result = *(_DWORD *)a2 - *(_DWORD *)a1;
  if ( *(_DWORD *)a2 == *(_DWORD *)a1 )
  {
    result = *(_DWORD *)(a2 + 4) - *(_DWORD *)(a1 + 4);
    if ( !result )
    {
      result = *(_DWORD *)(a2 + 8) - *(_DWORD *)(a1 + 8);
      if ( !result )
      {
        result = *(_DWORD *)(a2 + 12) - *(_DWORD *)(a1 + 12);
        if ( !result )
        {
          result = *(_DWORD *)(a2 + 16) - *(_DWORD *)(a1 + 16);
          if ( !result )
          {
            result = *(_DWORD *)(a2 + 20) - *(_DWORD *)(a1 + 20);
            if ( !result )
            {
              result = *(_DWORD *)(a2 + 24) - *(_DWORD *)(a1 + 24);
              if ( !result )
              {
                if ( (*(float *)(a2 + 28) == *(float *)(a1 + 28))
                   | __UNORDERED__(*(float *)(a2 + 28), *(float *)(a1 + 28)) )
                {
                  return 0;
                }
                else
                {
                  result = -1;
                  if ( !((*(float *)(a2 + 28) < (double)*(float *)(a1 + 28))
                       | __UNORDERED__(*(float *)(a2 + 28), *(float *)(a1 + 28))) )
                    return 1;
                }
              }
            }
          }
        }
      }
    }
  }
  return result;
}

/* ---- CompareTexEnvCombine  0x004F0E60 ----  [HIGH] */
int __cdecl CompareTexEnvCombine(float *a1, float *a2)
{
  int result;
  double v3;
  bool v4; // c0
  bool v5; // c2
  double v6;
  double v7;
  double v8;

  result = ((char *)a2 - (char *)a1) / 80;
  if ( !result )
  {
    if ( a2 )
    {
      result = CompareTexEnvCombineArgs((int)(a1 + 4), (int)(a2 + 4));
      if ( !result )
      {
        result = CompareTexEnvCombineArgs((int)(a1 + 12), (int)(a2 + 12));
        if ( !result )
        {
          if ( (*a2 == *a1) | __UNORDERED__(*a2, *a1) )
          {
            if ( (a2[1] == a1[1]) | __UNORDERED__(a2[1], a1[1]) )
            {
              if ( (a2[2] == a1[2]) | __UNORDERED__(a2[2], a1[2]) )
              {
                if ( (a2[3] == a1[3]) | __UNORDERED__(a2[3], a1[3]) )
                  return 0;
                v8 = a2[3];
                v4 = v8 < a1[3];
                v5 = __UNORDERED__(v8, a1[3]);
              }
              else
              {
                v7 = a2[2];
                v4 = v7 < a1[2];
                v5 = __UNORDERED__(v7, a1[2]);
              }
            }
            else
            {
              v6 = a2[1];
              v4 = v6 < a1[1];
              v5 = __UNORDERED__(v6, a1[1]);
            }
          }
          else
          {
            v3 = *a2;
            v4 = v3 < *a1;
            v5 = __UNORDERED__(v3, *a1);
          }
          result = -1;
          if ( !v4 && !v5 )
            return 1;
        }
      }
    }
  }
  return result;
}

/* ---- CompareNvTexShaders  0x004F0F10 ----  [HIGH] */
int __cdecl CompareNvTexShaders(char *a1, char *a2)
{
  int result;

  result = (a1 - a2) / 28;
  if ( !result )
  {
    if ( a1 )
    {
      result = *(_DWORD *)a1 - *(_DWORD *)a2;
      if ( *(_DWORD *)a1 == *(_DWORD *)a2 )
      {
        result = *((_DWORD *)a1 + 1) - *((_DWORD *)a2 + 1);
        if ( !result )
        {
          result = *((_DWORD *)a1 + 2) - *((_DWORD *)a2 + 2);
          if ( !result )
          {
            result = *((_DWORD *)a1 + 3) - *((_DWORD *)a2 + 3);
            if ( !result )
            {
              result = *((_DWORD *)a1 + 4) - *((_DWORD *)a2 + 4);
              if ( !result )
              {
                result = *((_DWORD *)a1 + 5) - *((_DWORD *)a2 + 5);
                if ( !result )
                  return *((_DWORD *)a1 + 6) - *((_DWORD *)a2 + 6);
              }
            }
          }
        }
      }
    }
  }
  return result;
}

/* ---- CompareNvRegisterCombiners  0x004F0F70 ----  [HIGH] */
int __cdecl CompareNvRegisterCombiners(const char *a1, const char *a2)
{
  int result;

  result = (a2 - a1) / 1536;
  if ( !result )
  {
    if ( a2 )
      return memcmp(a2, a1, 1536);
  }
  return result;
}

/* ---- CompareWaveForms  0x004F0FB0 ----  [HIGH] */
int __fastcall CompareWaveForms(int a1, int a2)
{
  int result;
  double v3;
  bool v4; // c0
  bool v5; // c2
  double v6;
  double v7;
  double v8;

  if ( *(_DWORD *)a2 != *(_DWORD *)a1 )
    return *(_DWORD *)a2 - *(_DWORD *)a1;
  if ( (*(float *)(a2 + 4) == *(float *)(a1 + 4)) | __UNORDERED__(*(float *)(a2 + 4), *(float *)(a1 + 4)) )
  {
    if ( (*(float *)(a2 + 8) == *(float *)(a1 + 8)) | __UNORDERED__(*(float *)(a2 + 8), *(float *)(a1 + 8)) )
    {
      if ( (*(float *)(a2 + 12) == *(float *)(a1 + 12)) | __UNORDERED__(*(float *)(a2 + 12), *(float *)(a1 + 12)) )
      {
        if ( (*(float *)(a2 + 16) == *(float *)(a1 + 16)) | __UNORDERED__(*(float *)(a2 + 16), *(float *)(a1 + 16)) )
          return 0;
        v8 = *(float *)(a2 + 16);
        v4 = v8 < *(float *)(a1 + 16);
        v5 = __UNORDERED__(v8, *(float *)(a1 + 16));
      }
      else
      {
        v7 = *(float *)(a2 + 12);
        v4 = v7 < *(float *)(a1 + 12);
        v5 = __UNORDERED__(v7, *(float *)(a1 + 12));
      }
    }
    else
    {
      v6 = *(float *)(a2 + 8);
      v4 = v6 < *(float *)(a1 + 8);
      v5 = __UNORDERED__(v6, *(float *)(a1 + 8));
    }
  }
  else
  {
    v3 = *(float *)(a2 + 4);
    v4 = v3 < *(float *)(a1 + 4);
    v5 = __UNORDERED__(v3, *(float *)(a1 + 4));
  }
  result = -1;
  if ( !v4 && !v5 )
    return 1;
  return result;
}

/* ---- CompareTextureBundles  0x004F1030 ----  [HIGH] */
int __cdecl CompareTextureBundles(int *a1, int *a2, int a3, int a4)
{
  float v5;
  float v6;
  int result;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  const char *v13;
  int v14;
  int i;
  int v16;

  v5 = *(float *)a1;
  v6 = *(float *)a2;
  result = (*a1 - *a2) / 120;
  if ( (!result || LODWORD(v5) == a3 && LODWORD(v6) == a4) && v5 != 0.0 && v6 != 0.0 )
  {
    v8 = a1[32];
    v9 = 1;
    if ( v8 > 1 || a2[32] > 1 )
    {
      v10 = a2[32];
      result = v8 - v10;
      if ( v8 != v10 )
        return result;
      if ( !((*((float *)a1 + 33) == *((float *)a2 + 33)) | __UNORDERED__(*((float *)a1 + 33), *((float *)a2 + 33))) )
      {
        result = -1;
        if ( !((*((float *)a1 + 33) < (double)*((float *)a2 + 33))
             | __UNORDERED__(*((float *)a1 + 33), *((float *)a2 + 33))) )
          return 1;
        return result;
      }
      if ( v8 > 1 )
      {
        do
        {
          v11 = (a1[v9] - a2[1]) / 120;
          if ( v11 )
            return v11;
        }
        while ( ++v9 < a1[32] );
      }
    }
    v11 = (a1[34] - a2[34]) / 52;
    if ( v11 )
      return v11;
    result = a1[35] - a2[35];
    if ( !result )
    {
      result = a1[36] - a2[36];
      if ( !result )
      {
        v12 = a1[39];
        result = v12 - a2[39];
        if ( !result )
        {
          if ( v12 != 3 )
          {
            if ( v12 == 6 )
            {
              result = memcmp((const char *)a1 + 160, (const char *)a2 + 160, 24);
              if ( result )
                return result;
            }
            result = a1[46] - a2[46];
            if ( result )
              return result;
            v16 = 0;
            if ( a1[46] > 0 )
            {
              v13 = (const char *)a1[47];
              v14 = a2[47] - (_DWORD)v13;
              for ( i = v14; ; v14 = i )
              {
                result = memcmp(v13, &v13[v14], 68);
                if ( result )
                  break;
                v13 += 68;
                if ( ++v16 >= a1[46] )
                  goto LABEL_29;
              }
              return result;
            }
LABEL_29:
            result = CompareNvTexShaders((char *)a1[38], (char *)a2[38]);
            if ( result )
              return result;
          }
          result = a1[48] - a2[48];
          if ( !result )
          {
            result = *((unsigned __int8 *)a1 + 196) - *((unsigned __int8 *)a2 + 196);
            if ( !result )
            {
              result = *((unsigned __int8 *)a1 + 197) - *((unsigned __int8 *)a2 + 197);
              if ( !result )
              {
                result = *((unsigned __int8 *)a1 + 198) - *((unsigned __int8 *)a2 + 198);
                if ( !result )
                  return CompareTexEnvCombine((float *)a2[37], (float *)a1[37]);
              }
            }
          }
        }
      }
    }
  }
  return result;
}

/* ---- CompareMergableShaders  0x004F1280 ----  [HIGH] */
int __fastcall CompareMergableShaders(int a1, int a2, int a3, int a4)
{
  int result;
  _DWORD *v5;
  int v6;
  _DWORD *v7;
  int v8;
  int *v9;
  int v10;
  bool v11; // cc
  _DWORD *v12;
  int v13;
  int v14;
  int v15;
  int v16;

  if ( a2 == a1 )
    return 0;
  result = *(_DWORD *)(a2 + 64) - *(_DWORD *)(a1 + 64);
  if ( !result )
  {
    result = *(_DWORD *)(a2 + 76) - *(_DWORD *)(a1 + 76);
    if ( !result )
    {
      result = *(_DWORD *)(a2 + 80) - *(_DWORD *)(a1 + 80);
      if ( !result )
      {
        result = *(_DWORD *)(a2 + 84) - *(_DWORD *)(a1 + 84);
        if ( !result )
        {
          if ( !((*(float *)(a2 + 88) == *(float *)(a1 + 88)) | __UNORDERED__(*(float *)(a2 + 88), *(float *)(a1 + 88))) )
          {
            result = -1;
            if ( !((*(float *)(a2 + 88) < (double)*(float *)(a1 + 88))
                 | __UNORDERED__(*(float *)(a2 + 88), *(float *)(a1 + 88))) )
              return 1;
            return result;
          }
          result = *(_DWORD *)(a2 + 168) - *(_DWORD *)(a1 + 168);
          if ( !result )
          {
            result = *(_DWORD *)(a2 + 336) - *(_DWORD *)(a1 + 336);
            v16 = *(_DWORD *)(a2 + 336);
            if ( !result )
            {
              v14 = 0;
              if ( *(int *)(a2 + 336) <= 0 )
                return 0;
              v5 = (_DWORD *)(a1 + 340);
              v6 = a2 - a1;
              v12 = (_DWORD *)(a1 + 340);
              v15 = v6;
              while ( 2 )
              {
                v7 = (_DWORD *)*v5;
                v8 = *(_DWORD *)((char *)v5 + v6);
                result = *(_DWORD *)v8 - *(_DWORD *)*v5;
                if ( !result )
                {
                  v13 = 0;
                  v9 = (int *)(v8 + 4);
                  do
                  {
                    result = CompareTextureBundles(v9, (int *)((char *)v7 + (_DWORD)v9 - v8), a3, a4);
                    if ( result )
                      return result;
                    v9 += 50;
                    ++v13;
                  }
                  while ( v13 < 8 );
                  result = *(_DWORD *)(v8 + 1604) - v7[401];
                  if ( result )
                    return result;
                  result = CompareNvRegisterCombiners((const char *)v7[402], *(const char **)(v8 + 1608));
                  if ( result )
                    return result;
                  v10 = *(_DWORD *)(v8 + 1636);
                  result = v10 - v7[409];
                  if ( result )
                    return result;
                  if ( v10 == 8 )
                  {
                    result = CompareWaveForms((int)(v7 + 404), v8 + 1616);
                    if ( result )
                      return result;
                  }
                  else if ( v10 != 12 )
                  {
                    goto LABEL_29;
                  }
                  result = *(unsigned __int8 *)(v8 + 1664) - *((unsigned __int8 *)v7 + 1664);
                  if ( result )
                    return result;
                  result = *(unsigned __int8 *)(v8 + 1665) - *((unsigned __int8 *)v7 + 1665);
                  if ( result )
                    return result;
                  result = *(unsigned __int8 *)(v8 + 1666) - *((unsigned __int8 *)v7 + 1666);
                  if ( result )
                    return result;
LABEL_29:
                  result = *(_DWORD *)(v8 + 1660) - v7[415];
                  if ( result )
                    return result;
                  if ( *(_DWORD *)(v8 + 1660) == 7 )
                  {
                    result = CompareWaveForms((int)(v7 + 410), v8 + 1640);
                    if ( result )
                      return result;
                  }
                  else if ( *(_DWORD *)(v8 + 1660) != 9 )
                  {
LABEL_35:
                    result = *(_DWORD *)(v8 + 1668) - v7[417];
                    if ( result )
                      return result;
                    v11 = ++v14 < v16;
                    ++v12;
                    if ( v11 )
                    {
                      v6 = v15;
                      v5 = v12;
                      continue;
                    }
                    return 0;
                  }
                  result = *(unsigned __int8 *)(v8 + 1667) - *((unsigned __int8 *)v7 + 1667);
                  if ( result )
                    return result;
                  goto LABEL_35;
                }
                break;
              }
            }
          }
        }
      }
    }
  }
  return result;
}

/* ---- MatchShaderTokenOnLine  0x004F14C0 ----  VERIFIED */
int __cdecl MatchShaderTokenOnLine(char **data_p, const char *a2, const char *a3)
{
  parseInfo_t *v3;
  const char *v4;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v3->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v3 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  v4 = (const char *)v3;
  if ( !_stricmp(v3->token, a2) )
    return 1;
  ri_Printf(2, "WARNING: %s missing '%s', found '%s' instead in shader '%s'\n", a3, a2, v4, shaderParseName);
  return 0;
}

/* ---- MatchShaderToken  0x004F1540 ----  VERIFIED */
int __cdecl MatchShaderToken(char **data_p, const char *a2, const char *a3)
{
  parseInfo_t *v3;
  char *v4;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v4 = Com_ParseExt(data_p, qtrue);
  if ( !_stricmp(v4, a2) )
    return 1;
  ri_Printf(2, "WARNING: %s missing '%s', found '%s' instead in shader '%s'\n", a3, a2, v4, shaderParseName);
  return 0;
}

/* ---- ParseVector  0x004F15B0 ----  VERIFIED */
int __cdecl ParseVector(int a1, char **data_p, int a3)
{
  parseInfo_t *v3;
  char *v4;
  int i;
  char *v6;
  double v7;
  char *v8;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v3->spaceDelimited == qfalse )
    {
      v4 = (char *)v3;
      goto LABEL_6;
    }
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v4 = Com_ParseExt(data_p, qfalse);
  v3 = parseInfo;
LABEL_6:
  if ( strcmp(v4, "(") )
  {
LABEL_22:
    ri_Printf(2, "WARNING: missing parenthesis in shader '%s'\n", shaderParseName);
    return 0;
  }
  for ( i = 0; i < a1; ++i )
  {
    if ( v3->ungetReady )
    {
      v3->ungetReady = qfalse;
      if ( v3->spaceDelimited == qfalse )
      {
        v6 = (char *)v3;
        goto LABEL_13;
      }
      *data_p = v3->ungetTokenSave;
      v3->currentLine = v3->ungetLineSave;
    }
    v6 = Com_ParseExt(data_p, qfalse);
LABEL_13:
    if ( !*v6 )
    {
      ri_Printf(2, "WARNING: missing vector element in shader '%s'\n", shaderParseName);
      return 0;
    }
    v7 = atof(v6);
    v3 = parseInfo;
    *(float *)(a3 + 4 * i) = v7;
  }
  if ( v3->ungetReady )
  {
    v3->ungetReady = qfalse;
    if ( v3->spaceDelimited == qfalse )
    {
      v8 = (char *)v3;
      goto LABEL_21;
    }
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v8 = Com_ParseExt(data_p, qfalse);
LABEL_21:
  if ( strcmp(v8, ")") )
    goto LABEL_22;
  return 1;
}

/* ---- NameToAFunc  0x004F1710 ----  VERIFIED */
int __cdecl NameToAFunc( const char *funcname )
{
	if ( funcname )
	{
		if ( !Q_stricmp( funcname, "GT0" ) )
			return 0x10000000;
		if ( !Q_stricmp( funcname, "LT128" ) )
			return 0x20000000;
		if ( !Q_stricmp( funcname, "GE128" ) )
			return 0x40000000;
	}

	ri_Printf( 2, "WARNING: invalid alphaFunc name '%s' in shader '%s'\n",
	           funcname, shaderParseName );
	return 0;
}

/* ---- NameToSrcBlendMode  0x004F1780 ----  VERIFIED */
int __cdecl NameToSrcBlendMode( const char *name )
{
	if ( name )
	{
		if ( !Q_stricmp( name, "GL_ONE" ) )
			return 2;
		if ( !Q_stricmp( name, "GL_ZERO" ) )
			return 1;
		if ( !Q_stricmp( name, "GL_DST_COLOR" ) )
			return 3;
		if ( !Q_stricmp( name, "GL_ONE_MINUS_DST_COLOR" ) )
			return 4;
		if ( !Q_stricmp( name, "GL_SRC_ALPHA" ) )
			return 5;
	}
	if ( !Q_stricmp( name, "GL_ONE_MINUS_SRC_ALPHA" ) )
		return 6;
	if ( !Q_stricmp( name, "GL_DST_ALPHA" ) )
		return 7;
	if ( !Q_stricmp( name, "GL_ONE_MINUS_DST_ALPHA" ) )
		return 8;
	if ( !Q_stricmp( name, "GL_SRC_ALPHA_SATURATE" ) )
		return 9;

	ri_Printf( 2, "WARNING: unknown blend mode '%s' in shader '%s', substituting GL_ONE\n",
	           name, shaderParseName );
	return 2;
}

/* ---- NameToDstBlendMode  0x004F1890 ----  VERIFIED */
int __cdecl NameToDstBlendMode( const char *name )
{
	if ( name )
	{
		if ( !Q_stricmp( name, "GL_ONE" ) )
			return 0x20;
		if ( !Q_stricmp( name, "GL_ZERO" ) )
			return 0x10;
		if ( !Q_stricmp( name, "GL_SRC_ALPHA" ) )
			return 0x50;
		if ( !Q_stricmp( name, "GL_ONE_MINUS_SRC_ALPHA" ) )
			return 0x60;
		if ( !Q_stricmp( name, "GL_DST_ALPHA" ) )
			return 0x70;
	}
	if ( !Q_stricmp( name, "GL_ONE_MINUS_DST_ALPHA" ) )
		return 0x80;
	if ( !Q_stricmp( name, "GL_SRC_COLOR" ) )
		return 0x30;
	if ( !Q_stricmp( name, "GL_ONE_MINUS_SRC_COLOR" ) )
		return 0x40;

	ri_Printf( 2, "WARNING: unknown blend mode '%s' in shader '%s', substituting GL_ONE\n",
	           name, shaderParseName );
	return 0x20;
}

/* ---- NameToGenFunc  0x004F1980 ----  VERIFIED */
int __cdecl NameToGenFunc( const char *funcname )
{
	if ( funcname )
	{
		if ( !Q_stricmp( funcname, "sin" ) )
			return 1;
		if ( !Q_stricmp( funcname, "square" ) )
			return 2;
		if ( !Q_stricmp( funcname, "triangle" ) )
			return 3;
		if ( !Q_stricmp( funcname, "sawtooth" ) )
			return 4;
		if ( !Q_stricmp( funcname, "inversesawtooth" ) )
			return 5;
	}
	if ( !Q_stricmp( funcname, "noise" ) )
		return 6;

	ri_Printf( 2, "WARNING: invalid genfunc name '%s' in shader '%s'\n",
	           funcname, shaderParseName );
	return 1;
}

/* ---- ParseWaveForm  0x004F1A40 ----  [HIGH] */
void __cdecl ParseWaveForm(int a1, char **a2)
{
  parseInfo_t *v2;
  char *v3;
  char *v4;
  char *v5;
  char *v6;

  v2 = parseInfo;
  if ( parseInfo->ungetReady == qfalse )
    goto LABEL_4;
  parseInfo->ungetReady = qfalse;
  if ( v2->spaceDelimited )
  {
    *a2 = v2->ungetTokenSave;
    v2->currentLine = v2->ungetLineSave;
LABEL_4:
    v2 = (parseInfo_t *)Com_ParseExt(a2, qfalse);
  }
  if ( !v2->token[0] )
    goto LABEL_10;
  *(_DWORD *)a1 = NameToGenFunc( v2->token );
  v3 = Com_ParseOnLine(a2);
  if ( !*v3 )
    goto LABEL_10;
  *(float *)(a1 + 4) = atof(v3);
  v4 = Com_ParseOnLine(a2);
  if ( *v4
    && (*(float *)(a1 + 8) = atof(v4), v5 = Com_ParseOnLine(a2), *v5)
    && (*(float *)(a1 + 12) = atof(v5), v6 = Com_ParseOnLine(a2), *v6) )
  {
    *(float *)(a1 + 16) = atof(v6);
  }
  else
  {
LABEL_10:
    ri_Printf(2, "WARNING: missing waveform parm in shader '%s'\n", shaderParseName);
  }
}

/* ---- MaxWaveFormDeformation  0x004F1B10 ----  VERIFIED */
double __cdecl MaxWaveFormDeformation(int a1)
{
  double result;
  double v2;

  result = fabs(*(float *)(a1 + 4) - *(float *)(a1 + 8));
  v2 = fabs(*(float *)(a1 + 8) + *(float *)(a1 + 4));
  if ( result <= v2 )
    return v2;
  return result;
}

/* ---- ParseTexMod  0x004F1B40 ----  [HIGH] */
void __cdecl ParseTexMod(int a1, int a2, char *data_p)
{
  _DWORD *v4;
  int v5;
  int v6;
  char *v7;
  int v8;
  char *v9;
  char *v10;
  char *v11;
  char *v12;
  char *v13;
  char *v14;
  char *v15;
  char *v16;
  char *v17;
  char *v18;
  char *v19;
  char *v20;
  char *v21;
  char *v22;
  char *v23;
  char *v24;
  char *v25;
  char *v26;
  char *v27;

  v4 = (_DWORD *)(200 * a1 + a2);
  v5 = v4[47];
  if ( v5 == 4 )
  {
    ri_Error(1, "\x15" "ERROR: too many tcMod stages in shader '%s'\n", shaderParseName);
    return;
  }
  v6 = v4[48] + 68 * v5;
  v7 = Com_ParseOnLine(&data_p);
  v8 = v4[1];
  if ( !v8 )
  {
    ri_Error(1, "\x15" "ERROR: tcMod stage before texture in shader '%s'\n", shaderParseName);
    return;
  }
  if ( *(_DWORD *)(v8 + 84) == 34067 )
  {
    if ( v7 && (!Q_stricmpn("negate", v7, 99999) || !Q_stricmpn("reverse", v7, 99999)) )
    {
      *(_DWORD *)v6 = 9;
      ++v4[47];
      return;
    }
    if ( !Q_stricmp("bumpmapFrame", v7) )
    {
      *(_DWORD *)v6 = 10;
      shaderParseSurfaceFlags |= 2u;
      ++v4[47];
      return;
    }
LABEL_56:
    ri_Printf(2, "WARNING: unknown or invalid tcMod '%s' in shader '%s'\n", v7, shaderParseName);
    ++v4[47];
    return;
  }
  if ( !v7 )
    goto LABEL_59;
  if ( !Q_stricmpn("swap", v7, 99999) )
  {
    *(_DWORD *)v6 = 8;
    ++v4[47];
    return;
  }
  if ( Q_stricmpn("turb", v7, 99999) )
  {
LABEL_59:
    if ( Q_stricmp("scale", v7) )
    {
      if ( Q_stricmp("scroll", v7) )
      {
        if ( Q_stricmp("stretch", v7) )
        {
          if ( Q_stricmp("transform", v7) )
          {
            if ( Q_stricmp("rotate", v7) )
            {
              if ( !Q_stricmp("entityTranslate", v7) )
              {
                *(_DWORD *)v6 = 7;
                ++v4[47];
                return;
              }
              goto LABEL_56;
            }
            v27 = Com_ParseOnLine(&data_p);
            if ( *v27 )
            {
              *(float *)(v6 + 64) = atof(v27);
              *(_DWORD *)v6 = 6;
              shaderParseSurfaceFlags |= 1u;
              ++v4[47];
            }
            else
            {
              ri_Printf(2, "WARNING: missing tcMod rotate parms in shader '%s'\n", shaderParseName);
            }
          }
          else
          {
            v21 = Com_ParseOnLine(&data_p);
            if ( !*v21 )
              goto LABEL_43;
            *(float *)(v6 + 24) = atof(v21);
            v22 = Com_ParseOnLine(&data_p);
            if ( !*v22 )
              goto LABEL_43;
            *(float *)(v6 + 28) = atof(v22);
            v23 = Com_ParseOnLine(&data_p);
            if ( !*v23 )
              goto LABEL_43;
            *(float *)(v6 + 32) = atof(v23);
            v24 = Com_ParseOnLine(&data_p);
            if ( *v24
              && (*(float *)(v6 + 36) = atof(v24), v25 = Com_ParseOnLine(&data_p), *v25)
              && (*(float *)(v6 + 40) = atof(v25), v26 = Com_ParseOnLine(&data_p), *v26) )
            {
              *(float *)(v6 + 44) = atof(v26);
              *(_DWORD *)v6 = 1;
              ++v4[47];
            }
            else
            {
LABEL_43:
              ri_Printf(2, "WARNING: missing transform parms in shader '%s'\n", shaderParseName);
            }
          }
        }
        else
        {
          v17 = Com_ParseOnLine(&data_p);
          if ( !*v17 )
            goto LABEL_35;
          *(_DWORD *)(v6 + 4) = NameToGenFunc( v17 );
          v17 = Com_ParseOnLine(&data_p);
          if ( !*v17 )
            goto LABEL_35;
          *(float *)(v6 + 8) = atof(v17);
          v18 = Com_ParseOnLine(&data_p);
          if ( *v18
            && (*(float *)(v6 + 12) = atof(v18), v19 = Com_ParseOnLine(&data_p), *v19)
            && (*(float *)(v6 + 16) = atof(v19), v20 = Com_ParseOnLine(&data_p), *v20) )
          {
            *(float *)(v6 + 20) = atof(v20);
            *(_DWORD *)v6 = 5;
            shaderParseSurfaceFlags |= 1u;
            ++v4[47];
          }
          else
          {
LABEL_35:
            ri_Printf(2, "WARNING: missing stretch parms in shader '%s'\n", shaderParseName);
          }
        }
      }
      else
      {
        v15 = Com_ParseOnLine(&data_p);
        if ( *v15 && (*(float *)(v6 + 56) = atof(v15), v16 = Com_ParseOnLine(&data_p), *v16) )
        {
          *(float *)(v6 + 60) = atof(v16);
          *(_DWORD *)v6 = 3;
          shaderParseSurfaceFlags |= 1u;
          ++v4[47];
        }
        else
        {
          ri_Printf(2, "WARNING: missing scale scroll parms in shader '%s'\n", shaderParseName);
        }
      }
    }
    else
    {
      v13 = Com_ParseOnLine(&data_p);
      if ( *v13 && (*(float *)(v6 + 48) = atof(v13), v14 = Com_ParseOnLine(&data_p), *v14) )
      {
        *(float *)(v6 + 52) = atof(v14);
        *(_DWORD *)v6 = 4;
        ++v4[47];
      }
      else
      {
        ri_Printf(2, "WARNING: missing scale parms in shader '%s'\n", shaderParseName);
      }
    }
  }
  else
  {
    v9 = Com_ParseOnLine(&data_p);
    if ( *v9 )
    {
      *(float *)(v6 + 8) = atof(v9);
      v10 = Com_ParseOnLine(&data_p);
      if ( *v10
        && (*(float *)(v6 + 12) = atof(v10), v11 = Com_ParseOnLine(&data_p), *v11)
        && (*(float *)(v6 + 16) = atof(v11), v12 = Com_ParseOnLine(&data_p), *v12) )
      {
        *(float *)(v6 + 20) = atof(v12);
        *(_DWORD *)v6 = 2;
        shaderParseSurfaceFlags |= 1u;
        ++v4[47];
      }
      else
      {
        ri_Printf(2, "WARNING: missing tcMod turb in shader '%s'\n", shaderParseName);
      }
    }
    else
    {
      ri_Printf(2, "WARNING: missing tcMod turb parms in shader '%s'\n", shaderParseName);
    }
  }
}

/* ---- ParseTexEnvCombineFunction  0x004F20A0 ----  VERIFIED */
int __cdecl ParseTexEnvCombineFunction(const char *a1, float *a2, int a3, char **data_p)
{
  char **v4;
  int result;
  char *v6;
  float *v7;
  parseInfo_t *v8;
  int v9;
  char v10;
  char v11;
  int v12;
  int v13;
  bool v14; // cc
  parseInfo_t *v15;
  char *v16;
  char *v17;
  double v18;
  unsigned __int8 v20; // c2
  unsigned __int8 v21; // c3
  const char *v23;
  const char *v24;
  const char *v25;
  int v26;
  int v27;
  char v28[4]; // [esp+Ch] [ebp-10h] BYREF
  int v29;
  int v30;
  float *i;

  v4 = data_p;
  v28[0] = 0;
  v28[1] = 0;
  result = MatchShaderTokenOnLine(data_p, "=", "texEnvCombine funxtion");
  if ( !result )
    return result;
  v6 = Com_ParseOnLine(data_p);
  if ( !*v6 )
  {
    ri_Printf(2, "WARNING: missing equation name in texEnvCombine function in shader '%s'\n", shaderParseName);
    return 0;
  }
  if ( !Q_stricmpn("REPLACE", v6, 99999) )
  {
    *(_DWORD *)a2 = 7681;
    v29 = 1;
LABEL_29:
    if ( !MatchShaderTokenOnLine(data_p, "(", "texEnvCombine function") )
      return 0;
    v30 = 0;
    v7 = a2 + 1;
    for ( i = a2 + 1; ; v7 = i )
    {
      v8 = parseInfo;
      v9 = 0;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        if ( v8->spaceDelimited == qfalse )
          goto LABEL_36;
        *v4 = v8->ungetTokenSave;
        v8->currentLine = v8->ungetLineSave;
      }
      v8 = (parseInfo_t *)Com_ParseExt(v4, qfalse);
LABEL_36:
      v10 = v8->token[0];
      if ( !v8->token[0] )
      {
        ri_Printf(2, "WARNING: missing argument in texEnvCombine function in shader '%s'\n", shaderParseName);
        return 0;
      }
      if ( !Q_stricmpn("1", v8->token, 99999) )
      {
        v9 = 1;
        if ( !MatchShaderTokenOnLine(
                data_p,
                "-",
                "after 1 for 1 - source in texEnvCombine function:") )
          return 0;
        if ( v30 == 2 )
        {
          ri_Printf(
            2,
            "WARNING: the third argument cannot be 1 - source for texEnvCombine function in shader '%s'\n",
            shaderParseName);
          return 0;
        }
        v8 = (parseInfo_t *)Com_ParseOnLine(data_p);
        v10 = v8->token[0];
        if ( !v8->token[0] )
        {
          ri_Printf(
            2,
            "WARNING: missing source for 1 - source in texEnvCombine function in shader '%s'\n",
            shaderParseName);
          return 0;
        }
        v7 = i;
      }
      if ( !v8->token[1] || v8->token[2] )
      {
LABEL_72:
        ri_Printf(
          2,
          "WARNING: '%s' is not a valid source for texEnvCombine function in shader '%s'\n",
          v8,
          shaderParseName);
        return 0;
      }
      if ( v10 == 67 )
      {
        if ( a3 )
        {
          ri_Printf(
            2,
            "WARNING: '%s' is not a valid source for texEnvCombine alpha function in shader '%s'\n",
            v8,
            shaderParseName);
          return 0;
        }
        if ( v30 == 2 )
        {
          ri_Printf(
            2,
            "WARNING: the third argument must be an alpha for texEnvCombine function in shader '%s'\n",
            shaderParseName);
          return 0;
        }
        *((_DWORD *)v7 + 3) = (v9 != 0) + 768;
      }
      else
      {
        if ( v10 != 65 )
          goto LABEL_72;
        *((_DWORD *)v7 + 3) = (v9 != 0) + 770;
      }
      v11 = v8->token[1];
      switch ( v11 )
      {
        case 't':
          *(_DWORD *)v7 = 5890;
          break;
        case 'c':
          *(_DWORD *)v7 = 34166;
          break;
        case 'f':
          *(_DWORD *)v7 = 34167;
          break;
        case 'p':
          *(_DWORD *)v7 = 34168;
          break;
        default:
          goto LABEL_72;
      }
      v12 = v29;
      v13 = v30 + 1;
      v28[0] = v30 + 1 != v29 ? 44 : 41;
      if ( !MatchShaderTokenOnLine(data_p, v28, "texEnvCombine function") )
        return 0;
      v14 = v13 < v12;
      v4 = data_p;
      v30 = v13;
      ++i;
      if ( !v14 )
      {
        v15 = parseInfo;
        a2[7] = 1.0;
        if ( v15->ungetReady == qfalse )
          goto LABEL_63;
        v15->ungetReady = qfalse;
        if ( v15->spaceDelimited )
        {
          *data_p = v15->ungetTokenSave;
          v15->currentLine = v15->ungetLineSave;
LABEL_63:
          v15 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
        }
        if ( v15->token[0] )
        {
          if ( v15->token[0] != 42 )
          {
            ri_Printf(
              2,
              "WARNING: unknown token '%s' following texEnvCombine function in shader '%s'\n",
              v15,
              shaderParseName);
            return 0;
          }
          v16 = Com_ParseOnLine(data_p);
          v17 = v16;
          if ( !*v16 )
          {
            ri_Printf(2, "WARNING: missing scale for texEnvCombine function in shader '%s'\n", shaderParseName);
            return 0;
          }
          v18 = atof(v16);
          a2[7] = v18;
          if ( v18 != 1.0 && v18 != 2.0 && v18 != 4.0 )
          {
            ri_Printf(2, "WARNING: bad scale '%s' for texEnvCombine function in shader '%s'\n", v17, shaderParseName);
            return 0;
          }
        }
        return 1;
      }
    }
  }
  if ( !Q_stricmpn("MODULATE", v6, 99999) )
  {
    *(_DWORD *)a2 = 8448;
LABEL_28:
    v29 = 2;
    goto LABEL_29;
  }
  if ( !Q_stricmpn("ADD", v6, 99999) )
  {
    *(_DWORD *)a2 = 260;
    goto LABEL_28;
  }
  if ( !Q_stricmp("ADD_SIGNED_ARB", v6) )
  {
    *(_DWORD *)a2 = 34164;
    goto LABEL_28;
  }
  if ( !Q_stricmp("SUBTRACT_ARB", v6) )
  {
    *(_DWORD *)a2 = 34023;
    goto LABEL_28;
  }
  if ( !Q_stricmp("INTERPOLATE_ARB", v6) )
  {
    *(_DWORD *)a2 = 34165;
    v29 = 3;
    goto LABEL_29;
  }
  if ( !Q_stricmp("DOT3_RGB_ARB", v6) )
  {
    if ( !rendererShaderRequirements[9] )
    {
      ri_Printf(2, "WARNING: shader '%s' uses DOT3_RGB_ARB without 'requires GL_ARB_texture_env_dot3'\n", shaderParseName);
      return 0;
    }
    if ( a3 )
    {
      ri_Printf(2, "WARNING: DOT3_RGB_ARB only valid for rgb channel\n");
      return 0;
    }
    *(_DWORD *)a2 = 34478;
    goto LABEL_28;
  }
  if ( !Q_stricmp("DOT3_RGBA_ARB", v6) )
  {
    if ( !rendererShaderRequirements[9] )
    {
      ri_Printf(2, "WARNING: shader '%s' uses DOT3_RGBA_ARB without 'requires GL_ARB_texture_env_dot3'\n", shaderParseName);
      return 0;
    }
    if ( a3 )
    {
      ri_Printf(2, "WARNING: DOT3_RGBA_ARB only valid for rgb channel\n");
      return 0;
    }
    *(_DWORD *)a2 = 34479;
    goto LABEL_28;
  }
  ri_Printf(2, "WARNING: unknown equation name '%s' in texEnvCombine in shader '%s'\n", v6, shaderParseName);
  return 0;
}

/* ---- ParseTextureEnvCombine  0x004F2630 ----  [HIGH] */
int __cdecl ParseTextureEnvCombine(int a1, char **a2)
{
  int v3;
  char *v4;
  parseInfo_t *v5;
  int v6;
  char *v7;
  char *v8;
  char *v10;
  char *v11;
  char *v12;
  char *v13;
  char *v14;
  char *v15;
  char *v16;
  int v17;
  const char *v18;
  const char *v19;
  const char *v20;
  const char *v21;
  int v22;
  int v23;

  *(_DWORD *)(a1 + 144) = 34160;
  v3 = ri_Hunk_Alloc(80);
  *(_DWORD *)(a1 + 148) = v3;
  *(_DWORD *)(v3 + 16) = 7681;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 44) = 1065353216;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 20) = 5890;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 24) = 5890;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 28) = 5890;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 32) = 768;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 36) = 768;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 40) = 770;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 48) = 7681;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 76) = 1065353216;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 52) = 5890;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 56) = 5890;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 60) = 5890;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 64) = 770;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 68) = 770;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 72) = 770;
  if ( !MatchShaderToken(a2, "{", "texEnvCombine") )
    return 0;
  v4 = *a2;
  v5 = parseInfo;
  if ( *a2 )
  {
    v6 = *v4;
    if ( *v4 )
    {
      while ( 1 )
      {
        ++v4;
        if ( v6 == 10 )
          break;
        v6 = *v4;
        if ( !*v4 )
          goto LABEL_8;
      }
      ++parseInfo->currentLine;
    }
LABEL_8:
    *a2 = v4;
  }
  **(_DWORD **)(a1 + 148) = 1065353216;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 4) = 1065353216;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 8) = 1065353216;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 12) = 1065353216;
  if ( v5->ungetReady )
  {
    v5->ungetReady = qfalse;
    *a2 = v5->ungetTokenSave;
    v5->currentLine = v5->ungetLineSave;
  }
  v7 = Com_ParseExt(a2, qtrue);
  if ( !*v7 )
    goto LABEL_45;
  if ( !Q_stricmpn("const", v7, 99999) )
  {
    if ( !MatchShaderTokenOnLine(a2, "=", "texEnvCombine const")
      || !MatchShaderTokenOnLine(a2, "(", "texEnvCombine const") )
    {
      return 0;
    }
    v8 = Com_ParseOnLine(a2);
    if ( !*v8 )
    {
      ri_Printf(2, "WARNING: missing color in texEnvCombine const in shader '%s'\n", shaderParseName);
      return 0;
    }
    **(float **)(a1 + 148) = atof(v8);
    v10 = Com_ParseOnLine(a2);
    if ( !*v10 || (*(float *)(*(_DWORD *)(a1 + 148) + 4) = atof(v10), v11 = Com_ParseOnLine(a2), !*v11) )
    {
      ri_Printf(2, "WARNING: truncated color in texEnvCombine const in shader '%s'\n", shaderParseName);
      return 0;
    }
    *(float *)(*(_DWORD *)(a1 + 148) + 8) = atof(v11);
    v12 = Com_ParseOnLine(a2);
    if ( *v12 )
    {
      if ( *v12 == 41 )
      {
LABEL_25:
        v13 = Com_ParseOnLine(a2);
        if ( *v13 )
        {
          if ( *v13 != 42 )
          {
            ri_Printf(2, "WARNING: unexpected '%s' in texEnvCombine const in shader '%s'\n", v13, shaderParseName);
            return 0;
          }
          v14 = Com_ParseOnLine(a2);
          if ( Q_stricmp("identityLighting", v14) )
          {
            ri_Printf(2, "WARNING: expected 'identityLighting' in texEnvCombine const in shader '%s'\n", shaderParseName);
            return 0;
          }
          **(float **)(a1 + 148) = tr_identityLight * **(float **)(a1 + 148);
          *(float *)(*(_DWORD *)(a1 + 148) + 4) = tr_identityLight * *(float *)(*(_DWORD *)(a1 + 148) + 4);
          *(float *)(*(_DWORD *)(a1 + 148) + 8) = tr_identityLight * *(float *)(*(_DWORD *)(a1 + 148) + 8);
        }
        v7 = Com_Parse(a2);
        goto LABEL_32;
      }
      *(float *)(*(_DWORD *)(a1 + 148) + 12) = atof(v12);
      v12 = Com_Parse(a2);
    }
    if ( *v12 != 41 )
    {
      ri_Printf(2, "WARNING: missing ')' in texEnvCombine const in shader '%s'\n", shaderParseName);
      return 0;
    }
    goto LABEL_25;
  }
LABEL_32:
  if ( !*v7 || Q_stricmpn("rgb", v7, 99999) )
  {
LABEL_45:
    ri_Printf(2, "WARNING: missing 'rgb' equation in texEnvCombine in shader '%s'\n", shaderParseName);
    return 0;
  }
  if ( !ParseTexEnvCombineFunction((const char *)a1, (float *)(*(_DWORD *)(a1 + 148) + 16), 0, a2) )
    return 0;
  v15 = Com_Parse(a2);
  v16 = v15;
  if ( *v15 && !Q_stricmp("alpha", v15) )
  {
    v17 = *(_DWORD *)(a1 + 148);
    if ( *(_DWORD *)(v17 + 16) == 34479 )
    {
      ri_Printf(2, "WARNING: alpha not allowed in texEnvCombine if rgb is DOT3_RGBA_ARB in shader '%s'\n", shaderParseName);
      return 0;
    }
    if ( ParseTexEnvCombineFunction((const char *)(v17 + 48), (float *)(v17 + 48), 1, a2) )
    {
      v16 = Com_Parse(a2);
      goto LABEL_42;
    }
    return 0;
  }
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 48) = 7681;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 76) = 1065353216;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 52) = 5890;
  *(_DWORD *)(*(_DWORD *)(a1 + 148) + 64) = 770;
LABEL_42:
  if ( *v16 == 125 )
    return 1;
  ri_Printf(2, "WARNING: missing '}' in texEnvCombine in shader '%s'\n", shaderParseName);
  return 0;
}

/* ---- ParseStageRequirementsOperand  0x004F2A90 ----  [HIGH] */
int __cdecl ParseStageRequirementsOperand(char **a1, int a2)
{
  parseInfo_t *v2;
  int v3;
  const char *string;
  char *v6;
  char v7;
  bool v8; // zf
  char *v9;
  cvar_t *v10;
  signed int v12;
  __int16 v13;
  double v14;
  char v15;
  double v16;
  const char *v17;
  const char *v18;
  const char *v19;
  const char *v20;
  const char *v21;
  const char *v22;
  const char *v23;
  const char *v24;
  const char *v25;
  const char *v26;
  int v27;
  int v28;
  int v29;
  int v30;
  int v31;

  v2 = parseInfo;
  v3 = 0;
  *(_DWORD *)a2 = 0;
  *(_WORD *)(a2 + 4) = -1;
  *(_WORD *)(a2 + 6) = 0;
LABEL_2:
  if ( v2->ungetReady )
  {
    v2->ungetReady = qfalse;
    if ( v2->spaceDelimited == qfalse )
    {
      string = (const char *)v2;
      goto LABEL_7;
    }
    *a1 = v2->ungetTokenSave;
    v2->currentLine = v2->ungetLineSave;
  }
  v6 = Com_ParseExt(a1, qfalse);
  v2 = parseInfo;
  string = v6;
LABEL_7:
  if ( !*string )
    return 0;
  if ( *string == 33 )
  {
    while ( ++*(_WORD *)(a2 + 6) )
    {
      v7 = *++string;
      if ( v7 != 33 )
      {
        if ( !v7 )
          goto LABEL_2;
        goto LABEL_12;
      }
    }
    return 0;
  }
LABEL_12:
  if ( !Q_stricmpn("GL_MAX_TEXTURE_UNITS_ARB", string, 99999) )
  {
    v8 = qglActiveTextureARB == 0;
    *(_WORD *)(a2 + 4) = 0;
    *(_DWORD *)a2 = 2;
    if ( v8 )
    {
      string = "1";
    }
    else
    {
      string = (const char *)(a2 + 8);
      _itoa(Value, (char *)(a2 + 8), 10);
    }
    goto LABEL_40;
  }
  if ( !Q_stricmpn("GL_ARB_texture_cube_map", string, 99999) )
  {
    *(_WORD *)(a2 + 4) = 6;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_textureCubeMap, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmpn("GL_ARB_texture_env_add", string, 99999) )
  {
    *(_WORD *)(a2 + 4) = 7;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_textureEnvAddAvailable, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmpn("GL_ARB_texture_env_combine", string, 99999) )
  {
    *(_WORD *)(a2 + 4) = 8;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_textureEnvCombine, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmpn("GL_ARB_texture_env_dot3", string, 99999) )
  {
    *(_WORD *)(a2 + 4) = 9;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_textureEnvDot3, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmp("GL_ARB_vertex_program", string) )
  {
    *(_WORD *)(a2 + 4) = 10;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_ARBVertexProgram, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmp("GL_NV_register_combiners", string) )
  {
    *(_WORD *)(a2 + 4) = 11;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_NVRegisterCombiners >= 1, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmp("GL_NV_register_combiners2", string) )
  {
    *(_WORD *)(a2 + 4) = 12;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_NVRegisterCombiners >= 2, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmp("GL_NV_texture_shader", string) )
  {
    *(_WORD *)(a2 + 4) = 13;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_NVTextureShader, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmp("GL_ATI_fragment_shader", string) )
  {
    *(_WORD *)(a2 + 4) = 14;
    *(_DWORD *)a2 = 2;
    string = (const char *)(a2 + 8);
    _itoa(glConfig_ATIFragmentShader, (char *)(a2 + 8), 10);
    goto LABEL_40;
  }
  if ( !Q_stricmp("cvar", string) )
  {
    v9 = Com_ParseOnLine(a1);
    if ( isalpha(*v9) || *v9 == 95 )
    {
      v10 = ri_Cvar_Get(v9, &empty_string, 32);
      if ( Q_stricmp("BADNAME", v10->name) )
      {
        string = v10->string;
        goto LABEL_40;
      }
    }
    return 0;
  }
LABEL_40:
  v12 = strlen(string);
  if ( !v12 )
  {
LABEL_49:
    *(_DWORD *)a2 = 1;
    goto LABEL_50;
  }
  *(_DWORD *)a2 = 2;
  if ( *string == 45 || *string == 43 )
    v3 = 1;
  if ( v3 < v12 )
  {
    while ( isdigit(string[v3]) || string[v3] == 46 )
    {
      if ( ++v3 >= v12 )
        goto LABEL_50;
    }
    goto LABEL_49;
  }
LABEL_50:
  v13 = *(_WORD *)(a2 + 6);
  if ( v13 )
  {
    *(_DWORD *)a2 = 2;
    *(_BYTE *)(a2 + 9) = 0;
    if ( (v13 & 1) != 0 )
    {
      v14 = atof(string);
      v15 = 49;
      if ( !((v14 == 0.0) | __UNORDERED__(v14, 0.0)) )
        v15 = 48;
      goto LABEL_54;
    }
    v16 = atof(string);
    v15 = 49;
    if ( !((v16 == 0.0) | __UNORDERED__(v16, 0.0)) )
    {
LABEL_54:
      *(_BYTE *)(a2 + 8) = v15;
      return 1;
    }
    *(_BYTE *)(a2 + 8) = 48;
    return 1;
  }
  else
  {
    strncpy((char *)(a2 + 8), string, 0xF7u);
    *(_BYTE *)(a2 + 255) = 0;
    return 1;
  }
}

/* ---- UpdateRequiresCondition  0x004F2EB0 ----  VERIFIED */
void __cdecl UpdateRequiresCondition(int a1, int a2, int a3)
{
  int v3;
  int v4;
  double v5;
  int v6;
  int v16;
  float v17;
  int v18;

  v3 = a2;
  v4 = a3;
  if ( *(__int16 *)(a2 + 4) >= 0 )
  {
    if ( *(__int16 *)(a1 + 4) >= 0 )
      return;
  }
  else
  {
    if ( *(__int16 *)(a1 + 4) < 0 )
      return;
    v4 = tr_texEnvArgSwap[a3];   /* 0x00540250 */
    v3 = a1;
    a1 = a2;
  }
  if ( *(_DWORD *)a1 != 2 )
    return;
  v5 = atof((const char *)(a1 + 8));
  v17 = v5;
  if ( !(((double)(int)(unsigned __int64)v5 == v17) | __UNORDERED__((double)(int)(unsigned __int64)v5, v17)) )
    return;
  v6 = *(__int16 *)(v3 + 4);
  if ( !*(_WORD *)(v3 + 4) )
  {
    if ( *(_WORD *)(v3 + 6) )
      return;
    if ( v4 )
    {
      if ( v4 == 4 )
      {
        v16 = 3;
        while ( !(((double)(v16 - 1) == v17) | __UNORDERED__((double)(v16 - 1), v17)) )
        {
          if ( ++v16 > 8 )
            return;
        }
        goto LABEL_28;
      }
      if ( v4 != 5 )
        return;
    }
    v16 = 3;
    v18 = 3;
    while ( !(((double)v18 == v17) | __UNORDERED__((double)v18, v17)) )
    {
      v18 = ++v16;
      if ( v16 > 8 )
        return;
    }
LABEL_28:
    rendererShaderRequirements[v16 - 3] = 1;
    return;
  }
  {
    int negated = (*(_BYTE *)(v3 + 6) & 1) != 0;
    int isK     = (v17 == flt_568E64);        /* 0x00568E64 == 0.0f */
    int record;

    if ( v4 == 1 )
      record = negated ? !isK : isK;
    else if ( v4 == 0 )
      record = negated ? isK : !isK;
    else
      return;                                 /* 0x004F2F28 `dec edi / jnz` */

    if ( record )
      rendererShaderRequirements[v6] = 1;
  }
}

typedef struct shaderRequiresOperand_s
{
  int    kind;         /* +0x00  0 unresolved, 1 non-numeric, 2 numeric      */
  short  dependency;   /* +0x04  index into rendererShaderRequirements, -1 none */
  short  notCount;     /* +0x06  count of leading '!' -- only bit 0 is read   */
  char   value[248];   /* +0x08  strncpy'd 247 + NUL                          */
} shaderRequiresOperand_t;

/* ---- ParseStageRequirements  0x004F3000 ----  [HIGH] */
int __cdecl ParseStageRequirements(char **data_p, int a2)
{
  char **v2;
  char *v3;
  const char *v4;
  double v6;
  BOOL v7;
  double v8;
  char *v9;
  double v10;
  bool v11; // zf
  double v12;
  BOOL v13;
  const char *v14;
  int v15;
  double v16;
  double v17;
  double v18;
  double v19;
  double v20;
  double v21;
  char v22[8]; // [esp+18h] [ebp-210h] BYREF
  shaderRequiresOperand_t op1; // [esp+20h] [ebp-208h] BYREF
  shaderRequiresOperand_t op2; // [esp+120h] [ebp-108h] BYREF
  unsigned int v29;
  unsigned int retaddr;

  v2 = data_p;
  v29 = retaddr ^ _security_cookie;
  if ( !ParseStageRequirementsOperand(data_p, (int)&op1) )
  {
    v14 = "\x15" "^1bad or missing arguments to 'requires' in shader '%s'\n";
LABEL_9:
    ri_Error(1, v14, shaderParseName);
    return 1;
  }
  v3 = Com_ParseOnLine(data_p);
  v4 = v3;
  if ( *v3 )
  {
    if ( strcmp(v3, "||") )
    {
      if ( strlen(v3) > 7 )
      {
        v14 = "\x15" "^1bad operator '%s' in 'requires' in shader '%s'\n";
        goto LABEL_9;
      }
      strcpy(v22, v4);
      if ( !ParseStageRequirementsOperand(data_p, (int)&op2) )
      {
        v14 = "\x15" "^1bad or missing second operand to 'requires' in shader '%s'\n";
        goto LABEL_9;
      }
      if ( op1.kind != op2.kind )
        ri_Printf(
          2,
          "WARNING: operands to 'requires' in shader '%s' have different types, comparing as strings\n",
          shaderParseName);
      if ( !strcmp(v22, "==") )
      {
        v15 = 0;
        v16 = atof(op1.value);
        v6 = atof(op2.value);
        v7 = ((v6 == v16) | __UNORDERED__(v6, v16)) != 0;
LABEL_17:
        v9 = Com_ParseOnLine(data_p);
        if ( *v9 && !strcmp(v9, "||") )
          return ParseStageRequirements(data_p, 1) | v7;
        if ( !a2 )
          UpdateRequiresCondition((int)&op2, (int)&op1, v15);
        return v7;
      }
      if ( !strcmp(v22, "!=") )
      {
        v7 = 1;
        v15 = 1;
        v17 = atof(op1.value);
        v8 = atof(op2.value);
        if ( !((v8 == v17) | __UNORDERED__(v8, v17)) )
          goto LABEL_17;
      }
      else if ( !strcmp(v22, ">") )
      {
        v15 = 4;
        v18 = atof(op1.value);
        v10 = atof(op2.value);
        if ( (v10 < v18) | __UNORDERED__(v10, v18) )
        {
          v7 = 1;
          goto LABEL_17;
        }
      }
      else if ( !strcmp(v22, ">=") )
      {
        v15 = 5;
        v19 = atof(op1.value);
        if ( atof(op2.value) <= v19 )
        {
          v7 = 1;
          goto LABEL_17;
        }
      }
      else
      {
        if ( !strcmp(v22, "<") )
        {
          v15 = 2;
          v20 = atof(op1.value);
          v11 = atof(op2.value) > v20;
        }
        else
        {
          if ( strcmp(v22, "<=") )
          {
            ri_Error(1, "\x15" "^1unknown operator '%s' in 'requires' for shader '%s'\n", v22, shaderParseName);
            return 1;
          }
          v15 = 3;
          v21 = atof(op1.value);
          v11 = atof(op2.value) >= v21;
        }
        if ( v11 )
        {
          v7 = 1;
          goto LABEL_17;
        }
      }
      v7 = 0;
      goto LABEL_17;
    }
    v2 = data_p;
  }
  v12 = atof(op1.value);
  v13 = ((v12 == 0.0) | __UNORDERED__(v12, 0.0)) == 0;
  if ( *v4 )
    return ParseStageRequirements(v2, 1) | v13;
  if ( !a2 && op1.dependency >= 2 && (op1.notCount & 1) == 0 )
    rendererShaderRequirements[op1.dependency] = 1;
  return v13;
}

/* ---- ParseImage  0x004F3410 ----  [HIGH] */
int __cdecl ParseImage(char **data_p, int a2, int a3, int a4, int a5, int *a6)
{
  unsigned int v6;
  parseInfo_t *v7;
  char *v8;
  char *v9;
  char *v10;
  double v11;
  int v13;
  char *v14;
  int ImageFile;
  float v28;
  int v29;
  int v30;

  *a6 = 0;
  v6 = ((unsigned int)shaderParseFlags >> 7) & 4;
  if ( (shaderParseFlags & 0x100) == 0 )
    v6 |= 2u;
  if ( (char)shaderParseFlags >= 0 )
    v6 |= 1u;
  v30 = 0;
  v29 = 0;
LABEL_6:
  v28 = 1.0;
LABEL_7:
  v7 = parseInfo;
  while ( 1 )
  {
    while ( 1 )
    {
      while ( 1 )
      {
        while ( 1 )
        {
          while ( 1 )
          {
            while ( 1 )
            {
              while ( 1 )
              {
                while ( 1 )
                {
                  while ( 1 )
                  {
                    while ( 1 )
                    {
                      if ( v7->ungetReady )
                      {
                        v7->ungetReady = qfalse;
                        if ( v7->spaceDelimited == qfalse )
                        {
                          v8 = (char *)v7;
                          goto LABEL_13;
                        }
                        *data_p = v7->ungetTokenSave;
                        v7->currentLine = v7->ungetLineSave;
                      }
                      v9 = Com_ParseExt(data_p, qfalse);
                      v7 = parseInfo;
                      v8 = v9;
LABEL_13:
                      if ( !*v8 )
                      {
                        ri_Printf(2, "WARNING: missing image name in shader %s\n", shaderParseName);
                        return 0;
                      }
                      if ( Q_stricmpn("nopicmip", v8, 99999) )
                        break;
                      v6 &= ~2u;
                    }
                    if ( Q_stricmpn("picmip", v8, 99999) )
                      break;
                    v6 |= 2u;
                  }
                  if ( Q_stricmpn("nomipmaps", v8, 99999) )
                    break;
                  v6 &= ~1u;
                }
                if ( Q_stricmpn("mipmaps", v8, 99999) )
                  break;
                v6 |= 1u;
              }
              if ( Q_stricmpn("clamp", v8, 99999) )
                break;
              v6 |= 0x30u;
            }
            if ( Q_stricmpn("clampx", v8, 99999) && Q_stricmpn("clamps", v8, 99999) )
              break;
            v6 |= 0x10u;
          }
          if ( Q_stricmpn("clampy", v8, 99999) && Q_stricmpn("clampt", v8, 99999) )
            break;
          v6 |= 0x20u;
        }
        if ( !Q_stricmp("heightToNormal", v8) )
        {
          v6 |= 0x40u;
          v10 = Com_ParseOnLine(data_p);
          v11 = atof(v10);
          v28 = v11;
          if ( !((v11 == 0.0) | __UNORDERED__(v11, 0.0)) )
            goto LABEL_7;
          Com_UngetToken();
          goto LABEL_6;
        }
        if ( Q_stricmp("only16bit", v8) )
          break;
        v30 = 1;
      }
      if ( Q_stricmp("only32bit", v8) )
        break;
      v29 = 1;
    }
    if ( Q_stricmp("noopt", v8) )
      break;
    v6 |= 0x100u;
  }
  if ( Q_stricmpn("$lightmap", v8, 99999) )
  {
    if ( !Q_stricmpn("$whiteimage", v8, 99999) || !Q_stricmpn("*white", v8, 99999) )
    {
      if ( a5 )
        goto LABEL_53;
      shaderParseSurfaceFlags |= 0x10u;
      *a6 = tr_whiteImage;
      return 1;
    }
    if ( !Q_stricmpn("$dlight", v8, 99999) )
    {
      if ( !a5 )
      {
        *a6 = tr_dlightImage;
        return 1;
      }
      goto LABEL_53;
    }
    if ( !Q_stricmp("$screen", v8) )
    {
      if ( !a5 )
      {
        *a6 = tr_screenImage;
        return 1;
      }
LABEL_53:
      ri_Printf(2, "WARNING: shader '%s' tried to use multiplyImage with image '%s'\n", shaderParseName, v8);
      return 0;
    }
    if ( (v6 & 0x40) != 0 || !r_graymap->integer || shaderParseLightmapIndex == -4 || shaderParseLightmapIndex == -2 )
    {
      if ( Q_stricmp("$texturename", v8) )
      {
LABEL_68:
        if ( v30 )
        {
          if ( v29 )
          {
            ri_Printf(2, "WARNING: only16bit and only32bit are mutually exclusive in shader %s\n", shaderParseName);
            return 0;
          }
          if ( glConfig_colorBits > 16 )
            return 1;
LABEL_75:
          if ( a3 )
          {
            if ( !*a6 )
            {
              if ( tr_delayedImageGroup )
              {
                if ( shaderParseLightmapIndex == -1 )
                  v6 &= ~0x80u;
              }
              ImageFile = R_FindImageFile(v8, 3553, v6, a4, a5, v28);
              *a6 = ImageFile;
              if ( !ImageFile )
              {
                ri_Printf(2, "WARNING: Couldn't load image '%s' in shader %s\n", v8, shaderParseName);
                return 0;
              }
            }
          }
          return 1;
        }
        if ( !v29 || glConfig_colorBits > 16 )
          goto LABEL_75;
        return 1;
      }
      shaderParseSurfaceFlags |= 0x10u;
      if ( !a2 )
      {
        ri_Printf(2, "WARNING: $texturename used in shader '%s', which is not a shader type file\n", shaderParseName);
        return 0;
      }
      v14 = Com_ParseOnLine(data_p);
      if ( Q_stricmp("+", v14) )
        v8 = &empty_string;
      else
        v8 = Com_ParseOnLine(data_p);
      v13 = R_FindImageFile(v8, 3553, v6, 10, a5, v28);
    }
    else
    {
      shaderParseSurfaceFlags |= 0x10u;
      v13 = tr_grayImage;
    }
    *a6 = v13;
    goto LABEL_68;
  }
  if ( a5 )
    goto LABEL_53;
  shaderParseSurfaceFlags |= 0x20u;
  if ( shaderParseLightmapIndex >= 0 )
    *a6 = tr_lightmaps[shaderParseLightmapIndex];
  else
    *a6 = tr_identityLightImage;
  return 1;
}

/* ---- ParseWaterMapPositiveFloat  0x004F3940 ----  [HIGH] */
double __cdecl ParseWaterMapPositiveFloat(char **data_p, int a2, float *a3)
{
  parseInfo_t *v3;
  double v4;
  const char *v6;
  int v7;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v3->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v3 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  if ( !v3->token[0] )
  {
    v7 = a2;
    v6 = "WARNING: waterMap missing %s in shader %s\n";
LABEL_9:
    ri_Printf(2, v6, v7, shaderParseName);
    return 0.0;
  }
  v4 = atof(v3->token);
  *a3 = v4;
  if ( v4 <= 0.0 )
  {
    v7 = a2;
    v6 = "WARNING: %s must be > 0 in waterMap in shader %s\n";
    goto LABEL_9;
  }
  return 1.0;
}

/* ---- ParseWaterMapFloat  0x004F39E0 ----  VERIFIED */
double __cdecl ParseWaterMapFloat(const char *a1, float *a2, char **v2)
{
  parseInfo_t *v3;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v3->spaceDelimited == qfalse )
      goto LABEL_5;
    *v2 = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v3 = (parseInfo_t *)Com_ParseExt(v2, qfalse);
LABEL_5:
  if ( v3->token[0] )
  {
    *a2 = atof(v3->token);
    return 1.0;
  }
  else
  {
    ri_Printf(2, "WARNING: waterMap missing %s in shader %s\n", a1, shaderParseName);
    return 0.0;
  }
}

/* ---- ParseWaterMapInt  0x004F3A60 ----  [HIGH] */
int __cdecl ParseWaterMapInt(char **data_p, const char *a2, int a3, int a4, int *a5)
{
  parseInfo_t *v5;
  int v7;

  v5 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v5->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = v5->ungetTokenSave;
    v5->currentLine = v5->ungetLineSave;
  }
  v5 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  if ( v5->token[0] )
  {
    v7 = j__atol(v5->token);
    *a5 = v7;
    if ( v7 < a4 || v7 > a3 )
    {
      ri_Printf(2, "WARNING: %s must be >= %i and <= %i in waterMap in shader %s\n", a2, a4, a3, shaderParseName);
      return 0;
    }
    else if ( ((v7 - 1) & v7) != 0 )
    {
      ri_Printf(2, "WARNING: %s must be a power of 2 in waterMap in shader %s\n", a2, shaderParseName);
      return 0;
    }
    else
    {
      return 1;
    }
  }
  else
  {
    ri_Printf(2, "WARNING: waterMap missing %s in shader %s\n", a2, shaderParseName);
    return 0;
  }
}

/* ---- ParseWaterMap  0x004F3B20 ----  [HIGH] */
int __cdecl ParseWaterMap(int a1, char **a2, int a3)
{
  double v3;
  double v4;
  double v5;
  double v6;
  double v7;
  double v8;
  double v10;
  int *WaterTexture;
  int v12;
  int v13; // [esp+10h] [ebp-54h] BYREF
  int v14; // [esp+14h] [ebp-50h] BYREF
  int v15; // [esp+18h] [ebp-4Ch] BYREF
  int v16; // [esp+1Ch] [ebp-48h] BYREF
  int v17; // [esp+20h] [ebp-44h] BYREF
  int v18; // [esp+24h] [ebp-40h] BYREF
  int v19[2]; // [esp+28h] [ebp-3Ch] BYREF  wind direction x,y -- ONE vec2
  _DWORD v21[13]; // [esp+30h] [ebp-34h] BYREF

  if ( !ParseWaterMapInt(a2, "texture width", 256, 4, &v13) )
    return 0;
  if ( !ParseWaterMapInt(a2, "texture width", 256, 4, &v14) )
    return 0;
  v3 = ParseWaterMapPositiveFloat(a2, (int)"horizontal world length", (float *)&v15);
  if ( (v3 == 0.0) | __UNORDERED__(v3, 0.0) )
    return 0;
  v4 = ParseWaterMapPositiveFloat(a2, (int)"vertical world length", (float *)&v16);
  if ( (v4 == 0.0) | __UNORDERED__(v4, 0.0) )
    return 0;
  v5 = ParseWaterMapPositiveFloat(a2, (int)"wind velocity", (float *)&v17);
  if ( (v5 == 0.0) | __UNORDERED__(v5, 0.0) )
    return 0;
  v6 = ParseWaterMapFloat("wind x direction", (float *)&v19[0], a2);
  if ( (v6 == 0.0) | __UNORDERED__(v6, 0.0) )
    return 0;
  /* retail 0x004F3C00: same. */
  v7 = ParseWaterMapFloat("wind y direction", (float *)&v19[1], a2);
  if ( (v7 == 0.0) | __UNORDERED__(v7, 0.0) )
    return 0;
  v8 = VectorNormalize2D((float *)v19);
  if ( (v8 == 0.0) | __UNORDERED__(v8, 0.0) )
  {
    ri_Printf(2, "WARNING: wind direction is 0 0 in waterMap in shader %s\n", shaderParseName);
    return 0;
  }
  v10 = ParseWaterMapPositiveFloat(a2, (int)"amplitude", (float *)&v18);
  if ( (v10 == 0.0) | __UNORDERED__(v10, 0.0) )
    return 0;
  v21[5] = v14;
  v21[4] = v13;
  v21[9] = v17;
  v21[6] = v15;
  v21[7] = v16;
  v21[12] = v18;
  v21[8] = 1145569280;
  v21[10] = v19[0];
  v21[11] = v19[1];
  WaterTexture = R_GetWaterTexture(v21);
  v12 = a1 + 200 * a3;
  *(_DWORD *)(v12 + 140) = WaterTexture;
  if ( !WaterTexture )
    return 0;
  *(_DWORD *)(v12 + 4) = *WaterTexture;
  return 1;
}

/* ---- ParseNVCullParm  0x004F3CF0 ----  [HIGH] */
int __cdecl ParseNVCullParm(char **data_p, int a2, int a3, int a4)
{
  parseInfo_t *v5;
  const char *v6;

  v5 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v5->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = v5->ungetTokenSave;
    v5->currentLine = v5->ungetLineSave;
  }
  v5 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  v6 = (const char *)v5;
  if ( !_stricmp(v5->token, "LESS_THAN_ZERO") )
  {
    *(_DWORD *)(*(_DWORD *)(200 * a2 + a4 + 156) + 4 * a3 + 12) = 513;
    return 1;
  }
  else if ( !_stricmp(v6, "GEQUAL_TO_ZERO") )
  {
    *(_DWORD *)(*(_DWORD *)(200 * a2 + a4 + 156) + 4 * a3 + 12) = 518;
    return 1;
  }
  else
  {
    ri_Printf(2, "WARNING: shader '%s' - cull argument must be 'LESS_THAN_ZERO' or 'GEQUAL_TO_ZERO'\n", shaderParseName);
    return 0;
  }
}

/* ---- ParseNVFloatParm  0x004F3DC0 ----  [HIGH] */
int __cdecl ParseNVFloatParm(char **data_p, int a2, int a3, int a4)
{
  parseInfo_t *v5;
  int result;
  double v7;

  v5 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v5->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = v5->ungetTokenSave;
    v5->currentLine = v5->ungetLineSave;
  }
  v5 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  if ( v5->token[0] )
  {
    v7 = atof(v5->token);
    result = 1;
    *(float *)(*(_DWORD *)(200 * a2 + a3 + 156) + 4 * a4 + 12) = v7;
  }
  else
  {
    ri_Printf(2, "WARNING: shader '%s' - missing value in 'nvTexShader'\n", shaderParseName);
    return 0;
  }
  return result;
}

/* ---- ParseNVTextureInput  0x004F3E50 ----  [HIGH] */
int __cdecl ParseNVTextureInput(int a1, int a2, char **data_p, int a4)
{
  int result;
  int v6;
  _DWORD *v7;
  unsigned int v8;
  _DWORD *v9;
  int v10;
  char *v11;
  int v12;
  char *v13;

  if ( a1 )
  {
    v6 = 200 * a1 + a2;
    v7 = *(_DWORD **)(v6 - 44);
    if ( v7 && *v7 == 34540 )
    {
LABEL_9:
      v8 = **(_DWORD **)(v6 + 156);
      if ( v8 < 0x86F2 || v8 > 0x86F3 || a1 >= 2 && (v9 = *(_DWORD **)(v6 - 244)) != 0 && *v9 == 34540 )
      {
        v11 = Com_ParseOnLine(data_p);
        if ( a4 )
        {
          *(_DWORD *)(*(_DWORD *)(v6 + 156) + 8) = 34102;
          if ( !_stricmp(v11, "expand") )
          {
            *(_DWORD *)(*(_DWORD *)(v6 + 156) + 8) = 34104;
            v11 = Com_ParseOnLine(data_p);
          }
        }
        if ( !_strnicmp(v11, "tex", 3u) && isdigit(v11[3]) && (v12 = v11[3], v12 - 48 < a1) && !v11[4] )
        {
          *(_DWORD *)(*(_DWORD *)(v6 + 156) + 4) = v12 + 33936;
          return 1;
        }
        else
        {
          v13 = va(" to tex%i", a1);
          ri_Printf(
            2,
            "WARNING: input source to texture op in 'nvTexShader' in shader %s must be tex0%s\n",
            shaderParseName,
            v13);
          return 0;
        }
      }
      else
      {
        v10 = a1 - 2;
LABEL_25:
        ri_Printf(2, "WARNING: bundle %i (from 0) must use 'nvTexShader dot3' in shader %s\n", v10, shaderParseName);
        return 0;
      }
    }
    else
    {
      switch ( **(_DWORD **)(v6 + 156) )
      {
        case 0x86E9:
        case 0x86EA:
        case 0x86EC:
          goto LABEL_9;
        case 0x86F2:
        case 0x86F3:
          if ( v7 && *v7 == 34545 )
            goto LABEL_9;
          ri_Printf(
            2,
            "WARNING: bundle %i (from 0) must use 'nvTexShader dot3' or 'nvTexShader dot3_diffuse_cubemap' in shader %s\n",
            a1 - 1,
            shaderParseName);
          result = 0;
          break;
        default:
          v10 = a1 - 1;
          goto LABEL_25;
      }
    }
  }
  else
  {
    ri_Printf(2, "WARNING: texture op in 'nvTexShader' invalid for first bundle in shader %s\n", shaderParseName);
    return 0;
  }
  return result;
}

/* ---- ParseNVTexShaderArgs  0x004F4010 ----  [HIGH] */
int __cdecl ParseNVTexShaderArgs(char **a1, int a2, int a3, int a4, int a5)
{
  int result;
  int v6;
  int v7;
  int v8;
  int v9;

  result = MatchShaderTokenOnLine(a1, "(", "nvTexShader function");
  if ( result )
  {
    v6 = 0;
    v9 = 0;
    if ( a4 <= 0 )
    {
      return MatchShaderTokenOnLine(a1, ")", "nvTexShader function") != 0;
    }
    else
    {
      while ( 2 )
      {
        if ( !v6 || MatchShaderTokenOnLine(a1, ",", "nvTexShader argument list") )
        {
          v7 = *(_DWORD *)(a5 + 4 * v6);
          switch ( v7 )
          {
            case 0:
              v8 = ParseNVTextureInput(a3, a2, a1, 0);
              goto LABEL_11;
            case 1:
              v8 = ParseNVTextureInput(a3, a2, a1, 1);
              goto LABEL_11;
            case 2:
            case 3:
            case 4:
            case 5:
              if ( !ParseNVCullParm(a1, a3, v7 - 2, a2) )
                return 0;
              v6 = v9;
              goto LABEL_12;
            case 6:
            case 7:
            case 8:
            case 9:
              v8 = ParseNVFloatParm(a1, a3, a2, v7 - 6);
LABEL_11:
              if ( v8 )
                goto LABEL_12;
              return 0;
            default:
LABEL_12:
              v9 = ++v6;
              if ( v6 >= a4 )
                return MatchShaderTokenOnLine(a1, ")", "nvTexShader function") != 0;
              continue;
          }
        }
        return 0;
      }
    }
  }
  return result;
}

/* ---- ParseNVTexShader  0x004F4140 ----  VERIFIED */
int __cdecl ParseNVTexShader(char **a1, int a2, int a3)
{
  _DWORD *v5;
  char *v6;
  bool v7; // zf
  _DWORD *v8;
  _DWORD *v9;
  _DWORD *v10;
  _DWORD *v11;
  char *String1; // [esp+8h] [ebp-14h] BYREF
  int v13; // [esp+Ch] [ebp-10h] BYREF
  int v14;
  int v15;
  int v16;

  if ( !rendererShaderRequirements[13] )
  {
    ri_Printf(2, "WARNING: used nvTexShader without 'requires GL_NV_texture_shader' in shader '%s'\n", shaderParseName);
    return 0;
  }
  v5 = (_DWORD *)(200 * a3 + a2 + 156);
  *v5 = ri_Hunk_Alloc(28);
  v6 = Com_ParseOnLine(a1);
  v7 = *v6 == 0;
  String1 = v6;
  if ( v7 )
  {
    ri_Printf(2, "WARNING: missing arguments to nvTexShader in shader '%s'\n", shaderParseName);
    return 0;
  }
  if ( !_stricmp(v6, "texture_2d") )
  {
    *(_DWORD *)*v5 = 3553;
    return ParseNVTexShaderArgs(a1, a2, a3, 0, 0);
  }
  if ( !_stricmp(String1, "texture_cube_map") )
  {
    *(_DWORD *)*v5 = 34067;
    return ParseNVTexShaderArgs(a1, a2, a3, 0, 0);
  }
  if ( !_stricmp(String1, "cull_fragment") )
  {
    v8 = (_DWORD *)*v5;
    v13 = 2;
    v14 = 3;
    v15 = 4;
    v16 = 5;
    *v8 = 34535;
    return ParseNVTexShaderArgs(a1, a2, a3, 4, (int)&v13);
  }
  if ( !_stricmp(String1, "pass_through") )
  {
    *(_DWORD *)*v5 = 34534;
    return ParseNVTexShaderArgs(a1, a2, a3, 0, 0);
  }
  if ( !_stricmp(String1, "dependent_ar") )
  {
    v9 = (_DWORD *)*v5;
    String1 = 0;
    *v9 = 34537;
    return ParseNVTexShaderArgs(a1, a2, a3, 1, (int)&String1);
  }
  if ( !_stricmp(String1, "dependent_gb") )
  {
    v10 = (_DWORD *)*v5;
    String1 = 0;
    *v10 = 34538;
    return ParseNVTexShaderArgs(a1, a2, a3, 1, (int)&String1);
  }
  if ( !_stricmp(String1, "dot_product_2d_1of2")
    || !_stricmp(String1, "dot_product_cube_map_1of3")
    || !_stricmp(String1, "dot_product_cube_map_2of3")
    || !_stricmp(String1, "dot_product_depth_replace_1of2")
    || !_stricmp(String1, "dot_product_reflect_cube_map_const_eye_1of3")
    || !_stricmp(String1, "dot_product_reflect_cube_map_const_eye_2of3")
    || !_stricmp(String1, "dot_product_reflect_cube_map_eye_from_qs_1of3")
    || !_stricmp(String1, "dot_product_reflect_cube_map_eye_from_qs_2of3")
    || !_stricmp(String1, "dot_product_cube_map_and_reflect_cube_map_const_eye_1of3")
    || !_stricmp(String1, "dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3") )
  {
    *(_DWORD *)*v5 = 34540;
    goto LABEL_42;
  }
  if ( !_stricmp(String1, "dot_product_2d_2of2") )
  {
    *(_DWORD *)*v5 = 34542;
LABEL_42:
    String1 = (char *)1;
    return ParseNVTexShaderArgs(a1, a2, a3, 1, (int)&String1);
  }
  if ( !_stricmp(String1, "dot_product_cube_map_3of3") )
  {
    *(_DWORD *)*v5 = 34544;
    goto LABEL_42;
  }
  if ( !_stricmp(String1, "dot_product_cube_map_and_reflect_cube_map_const_eye_2of3")
    || !_stricmp(String1, "dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3") )
  {
    *(_DWORD *)*v5 = 34545;
    goto LABEL_42;
  }
  if ( !_stricmp(String1, "dot_product_reflect_cube_map_eye_from_qs_3of3")
    || !_stricmp(String1, "dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3") )
  {
    *(_DWORD *)*v5 = 34546;
    goto LABEL_42;
  }
  if ( !_stricmp(String1, "dot_product_reflect_cube_map_const_eye_3of3")
    || !_stricmp(String1, "dot_product_cube_map_and_reflect_cube_map_const_eye_3of3") )
  {
    v11 = (_DWORD *)*v5;
    v13 = 1;
    v14 = 6;
    v15 = 7;
    v16 = 8;
    *v11 = 34547;
    return ParseNVTexShaderArgs(a1, a2, a3, 4, (int)&v13);
  }
  else
  {
    ri_Printf(2, "WARNING: unknown nvTexShader function '%s' in shader '%s'\n", String1, shaderParseName);
    return 0;
  }
}

/* ---- ParseNVRC_ConstColor  0x004F45A0 ----  [HIGH] */
int __cdecl ParseNVRC_ConstColor(_DWORD *a1, char **a2, int a3)
{
  parseInfo_t *v4;
  int v5;
  char *v6;
  int v7;
  float *i;
  parseInfo_t *v9;
  char *v10;
  double v11;

  v4 = parseInfo;
  v5 = 0;
  *a1 = 0;
  if ( v4->ungetReady )
  {
    v4->ungetReady = qfalse;
    *a2 = v4->ungetTokenSave;
    v4->currentLine = v4->ungetLineSave;
  }
  v6 = Com_ParseExt(a2, qtrue);
  if ( !_stricmp(v6, "const0") )
  {
    v7 = 0;
  }
  else
  {
    if ( _stricmp(v6, "const1") )
    {
      *a1 = 1;
      Com_UngetToken();
      return 0;
    }
    v7 = 1;
  }
  if ( !MatchShaderToken(a2, "=", "nvRegCombiners const") || !MatchShaderToken(a2, "(", "nvRegCombiners const") )
    return 0;
  for ( i = (float *)(a3 + 16 * v7); ; ++i )
  {
    v9 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a2 = v9->ungetTokenSave;
      v9->currentLine = v9->ungetLineSave;
    }
    v10 = Com_ParseExt(a2, qtrue);
    v11 = atof(v10);
    *i = v11;
    if ( (v11 == 0.0) | __UNORDERED__(v11, 0.0) && !isdigit(*v10) )
    {
      ri_Printf(2, "WARNING: unexpected token '%s' in nvRegCombiners const in shader '%s'\n", v10, shaderParseName);
      return 0;
    }
    if ( v5 == 3 )
      break;
    if ( !MatchShaderToken(a2, ",", "nvRegCombiners const") )
      return 0;
    ++v5;
  }
  return MatchShaderToken(a2, ")", "nvRegCombiners const");
}

/* ---- ParseNVRC_ConstColors  0x004F4720 ----  VERIFIED */
int __cdecl ParseNVRC_ConstColors(char **a1, int a2, int a3)
{
  int v5;
  int v6; // [esp+8h] [ebp-4h] BYREF

  if ( !ParseNVRC_ConstColor(&v6, a1, a3) )
    return v6;
  while ( !a2 || rendererShaderRequirements[12] )
  {
    if ( !ParseNVRC_ConstColor(&v6, a1, a3) )
      return v6;
  }
  ri_Printf(
    2,
    "WARNING: bundle uses per-combiner constants without 'requires GL_NV_register_combiners2' in shader '%s'\n",
    shaderParseName);
  return 0;
}

/* ---- ParseNVRC_RegisterFromTable  0x004F4780 ----  [HIGH] */
int __cdecl ParseNVRC_RegisterFromTable(
        int a1,
        char *String2,
        const char **a3,
        _DWORD *a4,
        const char **a5,
        const char *a6)
{
  int v6;
  const char **i;
  const char *v9;

  v6 = 0;
  if ( a1 <= 0 )
    return 0;
  for ( i = a3; _stricmp(*i, String2); i += 3 )
  {
    if ( ++v6 >= a1 )
      return 0;
  }
  *a4 = a3[3 * v6 + 1];
  v9 = a3[3 * v6 + 2];
  *a5 = v9;
  if ( !v9 )
    *a5 = a6;
  return 1;
}

/* ---- ParseNVRC_ReadWriteRegister  0x004F47E0 ----  [HIGH] */
int __cdecl ParseNVRC_ReadWriteRegister(
        char **data_p,
        int *a2,
        _DWORD *a3,
        const char **a4,
        const char *a5)
{
  parseInfo_t *v6;
  char *v7;
  int v8;
  const char *v10;

  v6 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v6->ungetTokenSave;
    v6->currentLine = v6->ungetLineSave;
  }
  v7 = Com_ParseExt(data_p, qtrue);
  if ( ParseNVRC_RegisterFromTable(32, v7, tr_nvReadWriteRegisters, a2, a4, a5) )
  {
    v8 = *a2;
    if ( v8 == 33986 )
    {
      if ( !rendererShaderRequirements[0] && !rendererShaderRequirements[1] )
      {
        v10 = "WARNING: register combiner uses tex2 without suitable 'requires GL_MAX_TEXTURE_UNITS_ARB' in shader '%s'\n";
LABEL_10:
        ri_Printf(2, v10, shaderParseName);
        *a3 = 0;
        return 0;
      }
    }
    else if ( v8 == 33987 && !rendererShaderRequirements[1] )
    {
      v10 = "WARNING: register combiner uses tex3 without suitable 'requires GL_MAX_TEXTURE_UNITS_ARB' in shader '%s'\n";
      goto LABEL_10;
    }
    *a3 = 1;
    return 1;
  }
  else
  {
    Com_UngetToken();
    *a3 = 1;
    return 0;
  }
}

/* ---- ParseNVRC_GeneralOutputRegister  0x004F48C0 ----  VERIFIED */
int __cdecl ParseNVRC_GeneralOutputRegister(_DWORD *a1, int *a2, char **a3, const char *a4)
{
  parseInfo_t *v5;
  char *v6;
  const char *v8;
  const char *v9; // [esp+8h] [ebp-4h] BYREF

  v5 = parseInfo;
  *a1 = 1;
  if ( v5->ungetReady )
  {
    v5->ungetReady = qfalse;
    *a3 = v5->ungetTokenSave;
    v5->currentLine = v5->ungetLineSave;
  }
  v6 = Com_ParseExt(a3, qtrue);
  if ( !_stricmp(v6, "discard") )
  {
    *a2 = 34096;
    return 1;
  }
  Com_UngetToken();
  if ( ParseNVRC_ReadWriteRegister(a3, a2, a1, &v9, a4) )
  {
    if ( a4 == v9 )
      return 1;
    *a1 = 0;
    ri_Printf(2, "WARNING: output type must be %s in general combiner function in shader '%s'\n", shaderParseName, v8);
  }
  return 0;
}

/* ---- ParseNVRC_ReadRegister  0x004F4970 ----  VERIFIED */
int __cdecl ParseNVRC_ReadRegister(const char **a1, const char *a2, char **data_p, int *a4)
{
  char *v5;
  const char *v7;
  int v8; // [esp+10h] [ebp-4h] BYREF

  if ( ParseNVRC_ReadWriteRegister(data_p, a4, &v8, a1, a2) )
    return 1;
  if ( v8 )
  {
    v5 = Com_Parse(data_p);
    if ( ParseNVRC_RegisterFromTable(20, v5, tr_nvReadRegisters, a4, a1, a2) )
      return 1;
    ri_Printf(2, "WARNING: missing register in nvRegCombiners in shader '%s'\n", shaderParseName);
    Com_UngetToken();
  }
  return 0;
}

/* ---- ParseNVRC_FinalRegister  0x004F49F0 ----  [HIGH] */
int __cdecl ParseNVRC_FinalRegister(const char **a1, char **a2, int *a3, const char *a4)
{
  parseInfo_t *v4;
  char *v5;

  if ( a4 == (const char *)6407 )
  {
    v4 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a2 = v4->ungetTokenSave;
      v4->currentLine = v4->ungetLineSave;
    }
    v5 = Com_ParseExt(a2, qtrue);
    if ( !_stricmp(v5, "final_product") )
    {
      *a3 = 34097;
      *a1 = (const char *)6407;
      return 1;
    }
    if ( !_stricmp(v5, "color_sum") )
    {
      *a3 = 34098;
      *a1 = (const char *)6407;
      return 1;
    }
    Com_UngetToken();
  }
  return ParseNVRC_ReadRegister(a1, a4, a2, a3);
}

/* ---- ParseNVRC_GeneralMappedRegister  0x004F4AB0 ----  [HIGH] */
int __cdecl ParseNVRC_GeneralMappedRegister(int a1, char **data_p, const char *a3)
{
  parseInfo_t *v3;
  char *v4;
  parseInfo_t *v5;
  int v6;
  int v7;
  int result;
  int v9;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v4 = Com_ParseExt(data_p, qtrue);
  if ( !strcmp(v4, "-") )
  {
    v5 = parseInfo;
    v6 = 1;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v5->ungetTokenSave;
      v5->currentLine = v5->ungetLineSave;
    }
    v4 = Com_ParseExt(data_p, qtrue);
  }
  else
  {
    v6 = 0;
  }
  if ( !_stricmp(v4, "expand") )
  {
    *(_DWORD *)(a1 + 4) = (v6 != 0) + 34104;
  }
  else if ( !_stricmp(v4, "half_bias") )
  {
    *(_DWORD *)(a1 + 4) = (v6 != 0) + 34106;
  }
  else if ( !_stricmp(v4, "unsigned") )
  {
    if ( v6 )
    {
      ri_Printf(2, "'-' not valid with 'unsigned' in nvRegCombiners in shader '%s'\n", shaderParseName);
      return 0;
    }
    *(_DWORD *)(a1 + 4) = 34102;
  }
  else
  {
    if ( _stricmp(v4, "unsigned_invert") )
    {
      v7 = 0;
      *(_DWORD *)(a1 + 4) = (v6 != 0) + 34108;
      Com_UngetToken();
      goto LABEL_12;
    }
    if ( v6 )
    {
      ri_Printf(2, "'-' not valid with 'unsigned_invert' in nvRegCombiners in shader '%s'\n", shaderParseName);
      return 0;
    }
    *(_DWORD *)(a1 + 4) = 34103;
  }
  if ( !MatchShaderToken(data_p, "(", "nvRegCombiners register mapping") )
    return 0;
  v7 = 1;
LABEL_12:
  if ( !ParseNVRC_ReadRegister((const char **)(a1 + 8), a3, data_p, (int *)a1) )
    return 0;
  if ( *(_DWORD *)a1 == 2912 && *(_DWORD *)(a1 + 8) == 6406 )
  {
    ri_Printf(2, "WARNING: tried to use non-existent fog.a in nvRegCombiners in shader '%s'\n", shaderParseName);
    return 0;
  }
  if ( v7 && !MatchShaderToken(data_p, ")", "nvRegCombiners register mapping") )
    return 0;
  if ( *(_DWORD *)a1 != 1 )
    return 1;
  v9 = *(_DWORD *)(a1 + 4);
  *(_DWORD *)a1 = 0;
  switch ( v9 )
  {
    case 34102:
      *(_DWORD *)(a1 + 4) = 34103;
      result = 1;
      break;
    case 34103:
      *(_DWORD *)(a1 + 4) = 34102;
      result = 1;
      break;
    case 34104:
    case 34108:
      *(_DWORD *)(a1 + 4) = 34105;
      return 1;
    case 34105:
    case 34109:
      *(_DWORD *)(a1 + 4) = 34104;
      result = 1;
      break;
    case 34106:
      *(_DWORD *)(a1 + 4) = 34107;
      result = 1;
      break;
    case 34107:
      *(_DWORD *)(a1 + 4) = 34106;
      result = 1;
      break;
    default:
      return 1;
  }
  return result;
}

/* ---- ParseNVRC_FinalMappedRegister  0x004F4D80 ----  [HIGH] */
int __cdecl ParseNVRC_FinalMappedRegister(int a1, char **data_p, const char *a3)
{
  parseInfo_t *v3;
  int v4;
  char *v5;
  int v7;

  v3 = parseInfo;
  v4 = 0;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v5 = Com_ParseExt(data_p, qtrue);
  if ( !_stricmp(v5, "unsigned") )
  {
    *(_DWORD *)(a1 + 4) = 34102;
  }
  else
  {
    if ( _stricmp(v5, "unsigned_invert") )
    {
      *(_DWORD *)(a1 + 4) = 34102;
      Com_UngetToken();
      goto LABEL_6;
    }
    *(_DWORD *)(a1 + 4) = 34103;
  }
  v4 = 1;
  if ( !MatchShaderToken(data_p, "(", "nvRegCombiners final combiner register mapping") )
    return 0;
LABEL_6:
  if ( !ParseNVRC_FinalRegister((const char **)(a1 + 8), data_p, (int *)a1, a3) )
    return 0;
  if ( a3 == (const char *)6406 && *(_DWORD *)(a1 + 8) == 6407 )
  {
    ri_Printf(
      2,
      "WARNING: alpha component of final combiner can only access alpha and blue in nvRegCombiners in shader '%s'\n",
      shaderParseName);
    return 0;
  }
  if ( v4 && !MatchShaderToken(data_p, ")", "nvRegCombiners register mapping") )
    return 0;
  if ( *(_DWORD *)a1 == 1 )
  {
    v7 = *(_DWORD *)(a1 + 4) - 34102;
    *(_DWORD *)a1 = 0;
    if ( v7 )
    {
      if ( v7 == 1 )
      {
        *(_DWORD *)(a1 + 4) = 34102;
        return 1;
      }
    }
    else
    {
      *(_DWORD *)(a1 + 4) = 34103;
    }
  }
  return 1;
}

/* ---- ParseNVRC_SingleGeneralFunction  0x004F4EC0 ----  [HIGH] */
int __cdecl ParseNVRC_SingleGeneralFunction(char **data_p, int a2, const char *a3, int a4, _DWORD *a5)
{
  int v5;
  char *v6;

  if ( !ParseNVRC_GeneralOutputRegister(a5, (int *)(a2 + 4 * a4 + 48), data_p, a3) )
    return 0;
  *a5 = 0;
  if ( !MatchShaderToken(data_p, "=", "nvRegCombiners general combiner function") )
    return 0;
  v5 = a2 + 24 * a4;
  if ( !ParseNVRC_GeneralMappedRegister(v5, data_p, a3) )
    return 0;
  v6 = Com_Parse(data_p);
  if ( !strcmp(v6, ".") )
  {
    *(_BYTE *)(a2 + 2 * a4 + 68) = 1;
  }
  else
  {
    if ( strcmp(v6, "*") )
    {
      ri_Printf(2, "WARNING: expected '.' or '*' operator in general combiner function in shader '%s'\n", shaderParseName);
      return 0;
    }
    *(_BYTE *)(a2 + 2 * a4 + 68) = 0;
  }
  if ( ParseNVRC_GeneralMappedRegister(v5 + 12, data_p, a3)
    && MatchShaderToken(data_p, ";", "nvRegCombiners general combiner function") )
  {
    *a5 = 1;
    return 1;
  }
  return 0;
}

/* ---- ParseNVRC_GeneralFunction  0x004F4FE0 ----  [HIGH] */
BOOL __cdecl ParseNVRC_GeneralFunction(char **data_p, const char *a2, int a3)
{
  char *v6;
  int v7; // [esp+Ch] [ebp-4h] BYREF

  if ( !ParseNVRC_SingleGeneralFunction(data_p, a3, a2, 0, &v7) )
  {
    if ( v7 )
    {
      ri_Printf(2, "WARNING: general combiner doesn't specify any function in shader '%s'\n", shaderParseName);
      return 0;
    }
    return 0;
  }
  if ( !ParseNVRC_SingleGeneralFunction(data_p, a3, a2, 1, &v7) )
    return v7;
  if ( !*(_BYTE *)(a3 + 68) && !*(_BYTE *)(a3 + 69) )
  {
    if ( !ParseNVRC_GeneralOutputRegister(&v7, (int *)(a3 + 56), data_p, a2) )
      return v7;
    if ( MatchShaderToken(data_p, "=", "sum/mux function in nvRegCombiners") )
    {
      v6 = Com_Parse(data_p);
      if ( !_stricmp(v6, "sum") )
      {
        *(_BYTE *)(a3 + 70) = 0;
LABEL_14:
        if ( MatchShaderToken(data_p, "(", "sum/mux function in nvRegCombiners")
          && MatchShaderToken(data_p, ")", "sum/mux function in nvRegCombiners") )
        {
          return MatchShaderToken(data_p, ";", "sum/mux function in nvRegCombiners") != 0;
        }
        return 0;
      }
      if ( !_stricmp(v6, "mux") )
      {
        *(_BYTE *)(a3 + 70) = 1;
        goto LABEL_14;
      }
      ri_Printf(2, "WARNING: final function must be mux or sum in nvRegCombiners in shader '%s'\n", shaderParseName);
    }
    return 0;
  }
  return 1;
}

/* ---- ParseNVRC_GeneralPortion  0x004F5140 ----  [HIGH] */
int __cdecl ParseNVRC_GeneralPortion(char **a1, int a2, int *a3)
{
  parseInfo_t *v3;
  char *v4;
  int v5;
  int v6;
  char *v7;
  int result;

  v3 = parseInfo;
  *a3 = 0;
  if ( v3->ungetReady )
  {
    v3->ungetReady = qfalse;
    *a1 = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v4 = Com_ParseExt(a1, qtrue);
  if ( !_stricmp(v4, "rgb") )
  {
    v5 = a2 + 32;
    v6 = 6407;
  }
  else
  {
    if ( _stricmp(v4, "alpha") )
    {
      *a3 = 1;
      Com_UngetToken();
      return 0;
    }
    v5 = a2 + 104;
    v6 = 6406;
  }
  if ( MatchShaderToken(a1, "{", "nvRegCombiners general combiner")
    && ParseNVRC_GeneralFunction(a1, (const char *)v6, v5) )
  {
    v7 = Com_Parse(a1);
    if ( !_stricmp(v7, "bias_by_negative_one_half_scale_by_two") )
    {
      *(_DWORD *)(v5 + 60) = 34110;
      *(_DWORD *)(v5 + 64) = 34113;
    }
    else if ( !_stricmp(v7, "bias_by_negative_one_half") )
    {
      *(_DWORD *)(v5 + 60) = 0;
      *(_DWORD *)(v5 + 64) = 34113;
    }
    else
    {
      if ( !_stricmp(v7, "scale_by_one_half") )
      {
        *(_DWORD *)(v5 + 60) = 34112;
      }
      else if ( !_stricmp(v7, "scale_by_two") )
      {
        *(_DWORD *)(v5 + 60) = 34110;
      }
      else
      {
        if ( _stricmp(v7, "scale_by_four") )
        {
          Com_UngetToken();
          result = MatchShaderToken(a1, "}", "nvRegCombiners general combiner");
          *a3 = result;
          return result;
        }
        *(_DWORD *)(v5 + 60) = 34111;
      }
      *(_DWORD *)(v5 + 64) = 0;
    }
    if ( MatchShaderToken(a1, "(", "nvRegCombiners general combiner scale-and-bias")
      && MatchShaderToken(a1, ")", "nvRegCombiners general combiner scale-and-bias")
      && MatchShaderToken(a1, ";", "nvRegCombiners general combiner scale-and-bias") )
    {
      result = MatchShaderToken(a1, "}", "nvRegCombiners general combiner");
      *a3 = result;
      return result;
    }
  }
  return 0;
}

/* ---- ParseNVRC_General  0x004F5330 ----  [HIGH] */
int __cdecl ParseNVRC_General(int a1, char **a2)
{
  int v5; // [esp+Ch] [ebp-4h] BYREF

  if ( !ParseNVRC_ConstColors(a2, 1, a1) )
    return 0;
  if ( !ParseNVRC_GeneralPortion(a2, a1, &v5) )
  {
    if ( v5 )
      ri_Printf(2, "WARNING: general combiner doesn't specify alpha or rgb in shader '%s'\n", shaderParseName);
    return 0;
  }
  ParseNVRC_GeneralPortion(a2, a1, &v5);
  if ( !v5 )
    return 0;
  return MatchShaderToken(a2, "}", "nvRegCombiners general combiner");
}

/* ---- ParseNVRC_FinalMulSum  0x004F53B0 ----  [HIGH] */
int __cdecl ParseNVRC_FinalMulSum(char **data_p, int a2)
{
  parseInfo_t *v2;
  char *v3;
  parseInfo_t *v4;
  char *v5;
  int result;
  parseInfo_t *v7;
  char *v8;
  parseInfo_t *v9;
  char *v10;
  parseInfo_t *v11;
  char *v12;
  const char *v13;
  const char *v14;
  const char *v15;
  int v16;
  int v17;

  v16 = 0;
  v17 = 0;
  while ( 1 )
  {
    while ( 1 )
    {
      v2 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v2->ungetTokenSave;
        v2->currentLine = v2->ungetLineSave;
      }
      v3 = Com_ParseExt(data_p, qtrue);
      if ( !_stricmp(v3, "final_product") )
        break;
      if ( _stricmp(v3, "clamp_color_sum") )
      {
        Com_UngetToken();
        return 1;
      }
      if ( v17 )
      {
        ri_Printf(2, "WARNING: duplicate clamp_color_sum in nvRegCombiners in shader '%s'\n", shaderParseName);
        return 0;
      }
      v7 = parseInfo;
      v17 = 1;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v7->ungetTokenSave;
        v7->currentLine = v7->ungetLineSave;
      }
      v8 = Com_ParseExt(data_p, qtrue);
      if ( _stricmp(v8, "(") )
      {
        v15 = v8;
        v14 = "(";
LABEL_31:
        v13 = "nvRegCombiners final combiner clamp_color_sum";
        goto LABEL_32;
      }
      v9 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v9->ungetTokenSave;
        v9->currentLine = v9->ungetLineSave;
      }
      v10 = Com_ParseExt(data_p, qtrue);
      if ( _stricmp(v10, ")") )
      {
        v15 = v10;
        v14 = ")";
        goto LABEL_31;
      }
      v11 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v11->ungetTokenSave;
        v11->currentLine = v11->ungetLineSave;
      }
      v12 = Com_ParseExt(data_p, qtrue);
      if ( _stricmp(v12, ";") )
      {
        v15 = v12;
        v14 = ";";
        goto LABEL_31;
      }
      *(_BYTE *)(a2 + 84) = 1;
    }
    if ( v16 )
      break;
    v16 = 1;
    if ( !ParseNVRC_FinalMappedRegister(a2 + 48, data_p, (const char *)0x1907) )
      return 0;
    v4 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v4->ungetTokenSave;
      v4->currentLine = v4->ungetLineSave;
    }
    v5 = Com_ParseExt(data_p, qtrue);
    if ( _stricmp(v5, "*") )
    {
      v15 = v5;
      v14 = "*";
      v13 = "nvRegCombiners final combiner final_product";
LABEL_32:
      ri_Printf(2, "WARNING: %s missing '%s', found '%s' instead in shader '%s'\n", v13, v14, v15, shaderParseName);
      return 0;
    }
    if ( !ParseNVRC_FinalMappedRegister(a2 + 60, data_p, (const char *)0x1907) )
      return 0;
    if ( *(_DWORD *)(a2 + 56) != 6407 || *(_DWORD *)(a2 + 68) != 6407 )
    {
      ri_Printf(2, "WARNING: final_product can only multiply rgb in nvRegCombiners in shader '%s'\n", shaderParseName);
      return 0;
    }
    result = MatchShaderToken(data_p, ";", "nvRegCombiners final combiner final_product");
    if ( !result )
      return result;
  }
  ri_Printf(2, "WARNING: duplicate final_product in nvRegCombiners in shader '%s'\n", shaderParseName);
  return 0;
}

/* ---- ParseNVRC_FinalLerp  0x004F56B0 ----  [HIGH] */
int __cdecl ParseNVRC_FinalLerp(char **a1, _DWORD *a2)
{
  int result;

  result = MatchShaderToken(a1, "(", "final rgb lerp function in nvRegCombiners");
  if ( result )
  {
    if ( !ParseNVRC_FinalMappedRegister((int)a2, a1, (const char *)0x1907) )
      return 0;
    if ( *a2 == 34097 )
    {
      ri_Printf(
        2,
        "Cannot use final_product as first argument to lerp in nvRegCombiners in shader '%s'\n",
        shaderParseName);
      return 0;
    }
    if ( !MatchShaderToken(a1, ",", "final rgb lerp function in nvRegCombiners")
      || !ParseNVRC_FinalMappedRegister((int)(a2 + 3), a1, (const char *)0x1907)
      || !MatchShaderToken(a1, ",", "final rgb lerp function in nvRegCombiners")
      || !ParseNVRC_FinalMappedRegister((int)(a2 + 6), a1, (const char *)0x1907) )
    {
      return 0;
    }
    return MatchShaderToken(a1, ")", "final rgb lerp function in nvRegCombiners") != 0;
  }
  return result;
}

/* ---- ParseNVRC_FinalRgbFunc  0x004F5780 ----  [HIGH] */
int __cdecl ParseNVRC_FinalRgbFunc(char **a1, _DWORD *a2)
{
  parseInfo_t *v3;
  char *v4;
  char *v5;
  char *v6;
  int v7;
  int v8;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;
  int v22;
  int v23;
  int v24;
  int v25;
  int v26;
  int v27;
  int v28;
  int v29;
  int v30;
  int v31;
  int v32_[3]; // [esp+10h] [ebp-24h] BYREF
  int v35_[3]; // [esp+1Ch] [ebp-18h] BYREF
  int v38_[3]; // [esp+28h] [ebp-Ch]  BYREF
#define v32 v32_[0]
#define v33 v32_[1]
#define v34 v32_[2]
#define v35 v35_[0]
#define v36 v35_[1]
#define v37 v35_[2]
#define v38 v38_[0]
#define v39 v38_[1]
#define v40 v38_[2]

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *a1 = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v4 = Com_ParseExt(a1, qtrue);
  if ( _stricmp(v4, "lerp") )
  {
    Com_UngetToken();
    if ( !ParseNVRC_FinalMappedRegister((int)&v32, a1, (const char *)0x1907) )
      return 0;
    v5 = Com_Parse(a1);
    if ( !strcmp(v5, "+") )
    {
      v6 = Com_Parse(a1);
      if ( !_stricmp(v6, "lerp") )
      {
        v7 = v33;
        a2[9] = v32;
        v8 = v34;
        a2[10] = v7;
        a2[11] = v8;
        if ( ParseNVRC_FinalLerp(a1, a2) )
          return MatchShaderToken(a1, ";", "final rgb function in nvRegCombiners");
        return 0;
      }
      Com_UngetToken();
      if ( !ParseNVRC_FinalMappedRegister((int)&v35, a1, (const char *)0x1907) )
        return 0;
      v5 = Com_Parse(a1);
      if ( !strcmp(v5, "*") )
      {
        if ( !ParseNVRC_FinalMappedRegister((int)&v38, a1, (const char *)0x1907) )
          return 0;
        if ( v35 != 34097 )
        {
          v11 = v37;
          *a2 = v35;
          a2[1] = v36;
          v12 = v38;
          a2[2] = v11;
          v13 = v39;
          a2[3] = v12;
          v14 = v40;
          a2[4] = v13;
          a2[5] = v14;
          goto LABEL_18;
        }
        if ( v38 != 34097 )
        {
          *a2 = v38;
          a2[1] = v39;
          a2[2] = v40;
          v10 = v37;
          a2[3] = 34097;
          a2[4] = v36;
          a2[5] = v10;
LABEL_18:
          v15 = v32;
          v16 = v33;
          v17 = v34;
          a2[6] = 0;
          a2[7] = 34102;
          a2[8] = 6407;
          a2[9] = v15;
          a2[10] = v16;
          a2[11] = v17;
          return MatchShaderToken(a1, ";", "final rgb function in nvRegCombiners");
        }
LABEL_25:
        ri_Printf(2, "cannot multiply final_product by itself in nvRegcombiners in shader '%s'\n", shaderParseName);
        return 0;
      }
      if ( !strcmp(v5, ";") )
      {
        a2[1] = 34102;
        a2[4] = 34102;
        v18 = v32;
        a2[2] = 6407;
        a2[5] = 6407;
        *a2 = 0;
        a2[3] = 0;
        v19 = v33;
        a2[6] = v18;
        v20 = v34;
        a2[7] = v19;
        v21 = v35;
        a2[8] = v20;
        v22 = v36;
        v23 = v37;
        a2[9] = v21;
        a2[10] = v22;
        a2[11] = v23;
        return 1;
      }
    }
    else
    {
      if ( !strcmp(v5, "*") )
      {
        if ( !ParseNVRC_FinalMappedRegister((int)&v38, a1, (const char *)0x1907) )
          return 0;
        if ( v32 == 34097 )
        {
          if ( v38 == 34097 )
            goto LABEL_25;
          *a2 = v38;
          a2[1] = v39;
          a2[2] = v40;
          v24 = v34;
          a2[3] = 34097;
          a2[4] = v33;
          a2[5] = v24;
        }
        else
        {
          v25 = v34;
          *a2 = v32;
          a2[1] = v33;
          v26 = v38;
          a2[2] = v25;
          v27 = v39;
          a2[3] = v26;
          v28 = v40;
          a2[4] = v27;
          a2[5] = v28;
        }
        a2[6] = 0;
        a2[7] = 34102;
        a2[8] = 6407;
LABEL_29:
        v5 = Com_Parse(a1);
        if ( !strcmp(v5, "+") )
        {
          if ( ParseNVRC_FinalMappedRegister((int)(a2 + 9), a1, (const char *)0x1907) )
            return MatchShaderToken(a1, ";", "final rgb function in nvRegCombiners");
          return 0;
        }
        if ( !strcmp(v5, ";") )
        {
          a2[9] = 0;
          a2[10] = 34102;
          a2[11] = 6407;
          return 1;
        }
        goto LABEL_36;
      }
      if ( !strcmp(v5, ";") )
      {
        a2[2] = 6407;
        a2[5] = 6407;
        a2[8] = 6407;
        v29 = v32;
        *a2 = 0;
        a2[1] = 34102;
        a2[3] = 0;
        a2[4] = 34102;
        a2[6] = 0;
        v30 = v34;
        a2[7] = 34102;
        v31 = v33;
        a2[9] = v29;
        a2[10] = v31;
        a2[11] = v30;
        return 1;
      }
    }
LABEL_36:
    ri_Printf(2, "unexpected token '%s' in final rgb function in nvRegCombiners in shader '%s'\n", v5, shaderParseName);
    return 0;
  }
  if ( ParseNVRC_FinalLerp(a1, a2) )
    goto LABEL_29;
  return 0;
}
#undef v32
#undef v33
#undef v34
#undef v35
#undef v36
#undef v37
#undef v38
#undef v39
#undef v40

/* ---- ParseNVRC_FinalRgbAlpha  0x004F5BC0 ----  [HIGH] */
int __cdecl ParseNVRC_FinalRgbAlpha(char **a1, _DWORD *a2)
{
  parseInfo_t *v2;
  char *v3;
  parseInfo_t *v4;
  char *v5;
  int result;
  parseInfo_t *v7;
  char *v8;
  const char *v9;
  const char *v10;
  int v11;
  int v12;

  v11 = 0;
  v12 = 0;
  while ( 1 )
  {
    while ( 1 )
    {
      v2 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *a1 = v2->ungetTokenSave;
        v2->currentLine = v2->ungetLineSave;
      }
      v3 = Com_ParseExt(a1, qtrue);
      if ( !strcmp(v3, "}") )
        return 1;
      if ( _stricmp(v3, "out.rgb") )
        break;
      if ( v11 )
      {
        ri_Printf(2, "WARNING: duplicate rgb in final stage in nvRegCombiners in shader '%s'\n", shaderParseName);
        return 0;
      }
      v4 = parseInfo;
      v11 = 1;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *a1 = v4->ungetTokenSave;
        v4->currentLine = v4->ungetLineSave;
      }
      v5 = Com_ParseExt(a1, qtrue);
      if ( _stricmp(v5, "=") )
      {
        v10 = v5;
        v9 = "out.rgb in nvRegCombiners";
LABEL_22:
        ri_Printf(
          2,
          "WARNING: %s missing '%s', found '%s' instead in shader '%s'\n",
          v9,
          "=",
          v10,
          shaderParseName);
        return 0;
      }
      result = ParseNVRC_FinalRgbFunc(a1, a2);
      if ( !result )
        return result;
    }
    if ( _stricmp(v3, "out.a") )
      break;
    if ( v12 )
    {
      ri_Printf(2, "WARNING: duplicate alpha in final stage nvRegCombiners in shader '%s'\n", shaderParseName);
      return 0;
    }
    v7 = parseInfo;
    v12 = 1;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a1 = v7->ungetTokenSave;
      v7->currentLine = v7->ungetLineSave;
    }
    v8 = Com_ParseExt(a1, qtrue);
    if ( _stricmp(v8, "=") )
    {
      v10 = v8;
      v9 = "out.a in nvRegCombiners";
      goto LABEL_22;
    }
    if ( !ParseNVRC_FinalMappedRegister((int)(a2 + 18), a1, (const char *)0x1906) )
      return 0;
    result = MatchShaderToken(a1, ";", "out.a in nvRegCombiners");
    if ( !result )
      return result;
  }
  ri_Printf(2, "WARNING: unexpected '%s' in final stage in nvRegCombiners in shader '%s'\n", v3, shaderParseName);
  return 0;
}

/* ---- ParseNVRC_Final  0x004F5E10 ----  VERIFIED */
int __cdecl ParseNVRC_Final(char **a1, _DWORD *a2)
{
  int result;
  int v4;

  result = ParseNVRC_FinalMulSum(a1, a2);
  if ( result )
    return ParseNVRC_FinalRgbAlpha(a1, a2);
  return result;
}

/* ---- ParseNVRC_ClearGeneralCombinerPortion  0x004F5E30 ----  [HIGH] */
int __cdecl ParseNVRC_ClearGeneralCombinerPortion(int a1, int a2)
{
  _DWORD *v2;
  int v3;
  int result;

  v2 = (_DWORD *)(a1 + 8);
  v3 = 4;
  do
  {
    *(v2 - 2) = 0;
    *(v2 - 1) = 34102;
    *v2 = a2;
    v2 += 3;
    --v3;
  }
  while ( v3 );
  result = 34096;
  *(_DWORD *)(a1 + 60) = 0;
  *(_DWORD *)(a1 + 64) = 0;
  *(_BYTE *)(a1 + 68) = 0;
  *(_BYTE *)(a1 + 69) = 0;
  *(_BYTE *)(a1 + 70) = 0;
  *(_DWORD *)(a1 + 48) = 34096;
  *(_DWORD *)(a1 + 52) = 34096;
  *(_DWORD *)(a1 + 56) = 34096;
  return result;
}

/* ---- ParseNVRegCombiners  0x004F5E80 ----  [HIGH] */
int __cdecl ParseNVRegCombiners(int a1, char **data_p)
{
  _DWORD *v3;
  _DWORD *v4;
  int v5;
  _DWORD *v6;
  int v7;
  int v8;
  _DWORD *v9;
  int v10;
  parseInfo_t *v11;
  char *v12;
  int v13;
  _DWORD *v14;
  void *v15;
  int v16; // [esp+Ch] [ebp-604h] BYREF
  _DWORD v17[384]; // [esp+10h] [ebp-600h] BYREF

  /* retail 0x004F5E8C/0x004F5E9A: two byte tests, folded into a word compare. */
  if ( rendererShaderRequirements[11] || rendererShaderRequirements[12] )
  {
    if ( !MatchShaderToken(data_p, "{", "nvRegCombiners") )
      return 0;
    v3 = &v17[36];
    v16 = 8;
    do
    {
      v4 = v3 - 18;
      v5 = 4;
      do
      {
        *(v4 - 2) = 0;
        *(v4 - 1) = 34102;
        *v4 = 6407;
        v4 += 3;
        --v5;
      }
      while ( v5 );
      *(v3 - 8) = 34096;
      *(v3 - 7) = 34096;
      *(v3 - 6) = 34096;
      *(v3 - 5) = 0;
      *(v3 - 4) = 0;
      *((_BYTE *)v3 - 12) = 0;
      *((_BYTE *)v3 - 11) = 0;
      *((_BYTE *)v3 - 10) = 0;
      v6 = v3;
      v7 = 4;
      do
      {
        *(v6 - 2) = 0;
        *(v6 - 1) = 34102;
        *v6 = 6406;
        v6 += 3;
        --v7;
      }
      while ( v7 );
      v8 = v16;
      v3[10] = 34096;
      v3[11] = 34096;
      v3[12] = 34096;
      v3[13] = 0;
      v3[14] = 0;
      *((_BYTE *)v3 + 60) = 0;
      *((_BYTE *)v3 + 61) = 0;
      *((_BYTE *)v3 + 62) = 0;
      v3 += 44;
      v16 = v8 - 1;
    }
    while ( v8 != 1 );
    v9 = &v17[361];
    v10 = 7;
    do
    {
      *(v9 - 1) = 0;
      *v9 = 34102;
      v9[1] = 6407;
      v9 += 3;
      --v10;
    }
    while ( v10 );
    v17[380] = 6406;
    LOBYTE(v17[381]) = 0;
    memset(v17, 0, 0x20u);
    while ( ParseNVRC_ConstColor(&v16, data_p, (int)v17) )
      ;
    if ( !v16 )
      return 0;
    qmemcpy(&v17[8], v17, 0x20u);
    qmemcpy(&v17[52], v17, 0x20u);
    qmemcpy(&v17[96], v17, 0x20u);
    qmemcpy(&v17[140], v17, 0x20u);
    qmemcpy(&v17[184], v17, 0x20u);
    qmemcpy(&v17[228], v17, 0x20u);
    qmemcpy(&v17[272], v17, 0x20u);
    qmemcpy(&v17[316], v17, 0x20u);
    for ( v17[382] = 0; ; ++v17[382] )
    {
      v11 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v11->ungetTokenSave;
        v11->currentLine = v11->ungetLineSave;
      }
      if ( strcmp(Com_ParseExt(data_p, qtrue), "{") )
        break;
      if ( v17[382] == 2 )
      {
        if ( !rendererShaderRequirements[12] )
        {
          ri_Printf(
            v17[382],
            "WARNING: stage uses more that 2 general combiners without 'requires GL_NV_register_combiners2' in shader '%s'\n",
            shaderParseName);
          return 0;
        }
      }
      else if ( v17[382] == 8 )
      {
        ri_Printf(2, "WARNING: stage uses more that %i general combiners in shader '%s'\n", 8, shaderParseName);
        return 0;
      }
      if ( !ParseNVRC_General((int)&v17[44 * v17[382] + 8], data_p) )
        return 0;
    }
    Com_UngetToken();
    if ( ParseNVRC_FinalMulSum(data_p, (int)&v17[360]) && ParseNVRC_FinalRgbAlpha(data_p, &v17[360]) )
    {
      v13 = 0;
      v17[383] = 0;
      if ( v17[382] > 0 )
      {
        v14 = &v17[8];
        while ( !memcmp(v17, v14, 0x20u) )
        {
          ++v13;
          v14 += 44;
          if ( v13 >= v17[382] )
            goto LABEL_35;
        }
        v17[383] = 1;
      }
LABEL_35:
      v15 = (void *)ri_Hunk_Alloc(1536);
      *(_DWORD *)(a1 + 1608) = v15;
      qmemcpy(v15, v17, 0x600u);
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    ri_Printf(
      2,
      "WARNING: stage uses nvRegCombiners without 'requires GL_NV_register_combiners(2)' in shader '%s'\n",
      shaderParseName);
    return 0;
  }
}

/* ---- ParseATIFS_ConstReg  0x004F6240 ----  [HIGH] */
int __cdecl ParseATIFS_ConstReg(char **data_p)
{
  parseInfo_t *v1;
  char *v2;
  char v3;

  v1 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v1->ungetTokenSave;
    v1->currentLine = v1->ungetLineSave;
  }
  v2 = Com_ParseExt(data_p, qtrue);
  if ( !_strnicmp(v2, "const", 5u) )
  {
    v3 = v2[5];
    if ( v3 >= 48 && v3 <= 56 && !v2[6] )
      return v3 + 35089;
  }
  Com_UngetToken();
  return 0;
}

/* ---- ParseATIFS_Reg  0x004F62C0 ----  [HIGH] */
int __cdecl ParseATIFS_Reg(char **data_p)
{
  parseInfo_t *v1;
  char *v2;
  char v3;

  v1 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v1->ungetTokenSave;
    v1->currentLine = v1->ungetLineSave;
  }
  v2 = Com_ParseExt(data_p, qtrue);
  if ( *v2 == 114 || *v2 == 82 )
  {
    v3 = v2[1];
    if ( v3 >= 48 && v3 < 54 && !v2[2] )
      return v3 + 35057;
  }
  Com_UngetToken();
  return 0;
}

/* ---- ParseATIFS_TexCoord  0x004F6330 ----  [HIGH] */
int __cdecl ParseATIFS_TexCoord(char **data_p)
{
  parseInfo_t *v1;
  char *v2;
  char v3;

  v1 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v1->ungetTokenSave;
    v1->currentLine = v1->ungetLineSave;
  }
  v2 = Com_ParseExt(data_p, qtrue);
  if ( !_strnicmp(v2, "tc", 2u) )
  {
    v3 = v2[2];
    if ( v3 >= 48 && v3 < 56 && !v2[3] )
      return v3 + 33936;
  }
  Com_UngetToken();
  return 0;
}

/* ---- ParseATIFS_ConstDefs  0x004F63B0 ----  [HIGH] */
int __cdecl ParseATIFS_ConstDefs(char **a1, int a2)
{
  int v2;
  int result;
  int v4;
  parseInfo_t *v5;
  char *v6;
  double v7;
  const char *v8;
  float *v9;
  int v10;

  v2 = ParseATIFS_ConstReg(a1);
  if ( !v2 )
    return 1;
  if ( !MatchShaderToken(a1, "=", "atiFragmentShader constant definitions")
    || !MatchShaderToken(a1, "(", "atiFragmentShader constant definitions") )
  {
    return 0;
  }
  v4 = 0;
  v10 = a2 + 4 * (5 * v2 - 175685);
  v9 = (float *)v10;
  do
  {
    v5 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a1 = v5->ungetTokenSave;
      v5->currentLine = v5->ungetLineSave;
    }
    v6 = Com_ParseExt(a1, qtrue);
    v7 = atof(v6);
    *v9 = v7;
    if ( (v7 == 0.0) | __UNORDERED__(v7, 0.0) && *v6 != 48 && *v6 != 46
      || (v7 < 0.0) | __UNORDERED__(v7, 0.0)
      || v7 > 1.0 )
    {
      ri_Printf(
        2,
        "WARNING: expected a number from 0 to 1, found '%s' instead in atiFragmentShader constant in shader %s\n",
        v6,
        shaderParseName);
      return 0;
    }
    v8 = ",";
    if ( v4 >= 3 )
      v8 = ")";
    if ( !MatchShaderToken(a1, v8, "atiFragmentShader constant definitions") )
      return 0;
    ++v4;
    ++v9;
  }
  while ( v4 < 4 );
  if ( !MatchShaderToken(a1, ";", "atiFragmentShader constant definitions") )
    return 0;
  result = 1;
  *(_DWORD *)(v10 + 16) = 1;
  return result;
}

typedef struct shaderParseMark_s
{
  int       currentLine;      /* +0x00  ebp-14h */
  char     *data;             /* +0x04  ebp-10h  the saved *data_p */
  qboolean  ungetReady;       /* +0x08  ebp-0Ch */
  int       ungetLineSave;    /* +0x0C  ebp-08h */
  char     *ungetTokenSave;   /* +0x10  ebp-04h */
} shaderParseMark_t;

/* ---- ParseATIFS_TexReads  0x004F6520 ----  [HIGH] */
int __cdecl ParseATIFS_TexReads(char **a1, int a2, int a3)
{
  int ungetLineSave;
  char *v5;
  int currentLine;
  char *ungetTokenSave;
  int i;
  int v9;
  parseInfo_t *v10;
  char *v11;
  parseInfo_t *v12;
  char *v13;
  _DWORD *v14;
  parseInfo_t *v15;
  char *v16;
  int v17;
  int v18;
  parseInfo_t *v19;
  char *v20;
  parseInfo_t *v21;
  char *v22;
  int v23;
  char *v24;
  char *v26;
  parseInfo_t *v27;
  qboolean v28;
  int v29;
  char *v30;
  int v31;
  unsigned int *j;
  unsigned int v33;
  int v34;
  _DWORD *v35;
  const char *v36;
  const char *v37;
  shaderParseMark_t mark; // [esp+10h] [ebp-14h] BYREF, 20 bytes

  ungetLineSave = parseInfo->ungetLineSave;
  v5 = *a1;
  currentLine = parseInfo->currentLine;
  ungetTokenSave = parseInfo->ungetTokenSave;
  mark.ungetReady = parseInfo->ungetReady;
  mark.currentLine = currentLine;
  mark.data = v5;
  mark.ungetLineSave = ungetLineSave;
  mark.ungetTokenSave = ungetTokenSave;
  for ( i = ParseATIFS_Reg(a1); i; i = ParseATIFS_Reg(a1) )
  {
    v9 = i - 35105;
    v10 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a1 = v10->ungetTokenSave;
      v10->currentLine = v10->ungetLineSave;
    }
    v11 = Com_ParseExt(a1, qtrue);
    if ( *v11 != 61 || v11[1] )
    {
      v27 = parseInfo;
      v28 = mark.ungetReady;
      v29 = mark.ungetLineSave;
      parseInfo->currentLine = currentLine;
      *a1 = v5;
      v27->ungetReady = v28;
      v30 = mark.ungetTokenSave;
      v27->ungetLineSave = v29;
      v27->ungetTokenSave = v30;
      break;
    }
    v12 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a1 = v12->ungetTokenSave;
      v12->currentLine = v12->ungetLineSave;
    }
    v13 = Com_ParseExt(a1, qtrue);
    if ( !_stricmp(v13, "copy") )
    {
      v14 = (_DWORD *)(a2 + 12 * v9);
      *v14 = 0;
      v14[2] = 35190;
      goto LABEL_25;
    }
    if ( !_stricmp(v13, "copy_dr") )
    {
      v14 = (_DWORD *)(a2 + 12 * v9);
      *v14 = 0;
      v14[2] = 35192;
      goto LABEL_25;
    }
    if ( !_stricmp(v13, "tex") )
    {
      v14 = (_DWORD *)(a2 + 12 * v9);
      v14[2] = 35190;
    }
    else if ( !_stricmp(v13, "tex_dr") )
    {
      v14 = (_DWORD *)(a2 + 12 * v9);
      v14[2] = 35192;
    }
    else
    {
      if ( !_stricmp(v13, "copy_stq") )
      {
        v14 = (_DWORD *)(a2 + 12 * v9);
        *v14 = 0;
        v14[2] = 35191;
        goto LABEL_25;
      }
      if ( !_stricmp(v13, "copy_stq_dq") )
      {
        v14 = (_DWORD *)(a2 + 12 * v9);
        *v14 = 0;
        v14[2] = 35193;
        goto LABEL_25;
      }
      if ( !_stricmp(v13, "tex_stq") )
      {
        v14 = (_DWORD *)(a2 + 12 * v9);
        v14[2] = 35191;
      }
      else
      {
        if ( _stricmp(v13, "tex_stq_dq") )
        {
          Com_ParseReturnToMark(&mark.currentLine, a1);
          break;
        }
        v14 = (_DWORD *)(a2 + 12 * v9);
        v14[2] = 35193;
      }
    }
    *v14 = 1;
LABEL_25:
    if ( v14[1] )
    {
      ri_Printf(
        2,
        "WARNING: already encountered a routing instruction for r%i in atiFragmentShader in shader '%s'\n",
        v9,
        shaderParseName);
      return 0;
    }
    v15 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a1 = v15->ungetTokenSave;
      v15->currentLine = v15->ungetLineSave;
    }
    v16 = Com_ParseExt(a1, qtrue);
    if ( _stricmp(v16, "(") )
    {
      v37 = v16;
      v36 = "(";
LABEL_43:
      ri_Printf(
        2,
        "WARNING: %s missing '%s', found '%s' instead in shader '%s'\n",
        "routing instruction of atiFragmentShader",
        v36,
        v37,
        shaderParseName);
      return 0;
    }
    v17 = ParseATIFS_TexCoord(a1);
    if ( !v17 )
    {
      v17 = ParseATIFS_Reg(a1);
      if ( !v17 )
      {
        v26 = Com_Parse(a1);
        ri_Printf(
          2,
          "WARNING: expected r# or tc# instead of '%s' in routing instruction of atiFragmentShader in shader '%s'\n",
          v26,
          shaderParseName);
        return 0;
      }
      v18 = v14[2];
      if ( v18 == 35191 || v18 == 35193 )
      {
        ri_Printf(
          2,
          "WARNING: cannot use _stq forms with r# in routing instruction of atiFragmentShader in shader '%s'\n",
          v13);
        return 0;
      }
      if ( !a3 )
      {
        ri_Printf(
          2,
          "WARNING: cannot use r# in routing instruction of first phase of atiFragmentShader in shader '%s'\n",
          v13);
        return 0;
      }
    }
    v14[1] = v17;
    v19 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a1 = v19->ungetTokenSave;
      v19->currentLine = v19->ungetLineSave;
    }
    v20 = Com_ParseExt(a1, qtrue);
    if ( _stricmp(v20, ")") )
    {
      v37 = v20;
      v36 = ")";
      goto LABEL_43;
    }
    v21 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *a1 = v21->ungetTokenSave;
      v21->currentLine = v21->ungetLineSave;
    }
    v22 = Com_ParseExt(a1, qtrue);
    if ( _stricmp(v22, ";") )
    {
      v37 = v22;
      v36 = ";";
      goto LABEL_43;
    }
    currentLine = parseInfo->currentLine;
    v23 = parseInfo->ungetLineSave;
    v5 = *a1;
    v24 = parseInfo->ungetTokenSave;
    mark.ungetReady = parseInfo->ungetReady;
    mark.currentLine = currentLine;
    mark.data = v5;
    mark.ungetLineSave = v23;
    mark.ungetTokenSave = v24;
  }
  v31 = 0;
  for ( j = (unsigned int *)(a2 + 4); ; j += 3 )
  {
    v33 = *j;
    if ( *j )
    {
      v34 = 0;
      if ( v31 > 0 )
        break;
    }
LABEL_62:
    if ( ++v31 >= 6 )
      return 1;
  }
  v35 = (_DWORD *)(a2 + 4);
  while ( !*v35 || v33 != *v35 || j[1] == v35[1] || v33 < 0x84C0 || v33 > 0x84C8 )
  {
    ++v34;
    v35 += 3;
    if ( v34 >= v31 )
      goto LABEL_62;
  }
  ri_Printf(
    2,
    "WARNING: cannot use tc%c with different instruction suffixes (such as _stq and _stq_dq) in atiFragmentShader in shader '%s'\n",
    *(_DWORD *)(a2 + 12 * v31 + 4) - 33936,
    shaderParseName);
  return 0;
}

/* ---- ParseATIFS_DestReg  0x004F6A50 ----  [HIGH] */
void __cdecl ParseATIFS_DestReg(
        char **data_p,
        _DWORD *a2,
        _DWORD *a3,
        _DWORD *a4,
        _DWORD *a5)
{
  parseInfo_t *v6;
  char *v7;
  char v8;
  char v9;
  char v10;
  char v11;
  char v12;
  const char *v13;

  v6 = parseInfo;
  *a2 = 0;
  *a4 = 0;
  *a3 = 0;
  *a5 = 0;
  if ( v6->ungetReady )
  {
    v6->ungetReady = qfalse;
    *data_p = v6->ungetTokenSave;
    v6->currentLine = v6->ungetLineSave;
  }
  v7 = Com_ParseExt(data_p, qtrue);
  if ( *v7 == 114 || *v7 == 82 )
  {
    v8 = v7[1];
    if ( v8 >= 48 && v8 < 54 )
    {
      *a2 = v8 + 35057;
      v9 = v7[2];
      if ( !v9 )
      {
        *a4 = 0;
        *a3 = 1;
        return;
      }
      if ( v9 == 46 )
      {
        v10 = v7[4];
        if ( !v10 )
        {
          v11 = v7[3];
          if ( v11 != 114 && v11 != 82 )
          {
            if ( v11 != 103 && v11 != 71 )
            {
              if ( v11 != 98 && v11 != 66 )
              {
                if ( v11 == 97 || v11 == 65 )
                {
                  *a4 = 0;
                  *a5 = 1;
                  return;
                }
                goto LABEL_36;
              }
LABEL_29:
              *a4 = 4;
              *a3 = 1;
              return;
            }
LABEL_30:
            *a4 = 2;
            *a3 = 1;
            return;
          }
LABEL_31:
          *a4 = 1;
          *a3 = 1;
          return;
        }
        if ( (v10 == 97 || v10 == 65) && !v7[5] )
        {
          *a5 = 1;
          v12 = v7[3];
          switch ( v12 )
          {
            case 'r':
            case 'R':
              goto LABEL_31;
            case 'g':
            case 'G':
              goto LABEL_30;
            case 'b':
            case 'B':
              goto LABEL_29;
          }
        }
        else
        {
          v13 = v7 + 3;
          if ( !_stricmp(v7 + 3, "rgba") )
          {
            *a4 = 6407;
            *a3 = 1;
            *a5 = 1;
            return;
          }
          if ( !_stricmp(v13, "rgb") )
          {
            *a4 = 6407;
            *a3 = 1;
            return;
          }
        }
      }
    }
  }
LABEL_36:
  Com_UngetToken();
}

/* ---- ParseATIFS_ArgReg  0x004F6BF0 ----  [HIGH] */
int __cdecl ParseATIFS_ArgReg(char **data_p, _DWORD *a2)
{
  parseInfo_t *v2;
  char *v3;
  char v4;
  int v5;
  char v6;
  char v7;
  const char *v8;
  char v9;

  v2 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v2->ungetTokenSave;
    v2->currentLine = v2->ungetLineSave;
  }
  v3 = Com_ParseExt(data_p, qtrue);
  if ( *v3 == 114 || *v3 == 82 )
  {
    v6 = v3[1];
    if ( v6 < 48 || v6 > 54 )
      goto LABEL_33;
    *a2 = v6 + 35057;
    v5 = 2;
  }
  else
  {
    if ( !_strnicmp(v3, "const", 5u) )
    {
      v4 = v3[5];
      if ( v4 >= 48 && v4 <= 56 )
      {
        *a2 = v4 + 35089;
        v5 = 6;
        goto LABEL_16;
      }
LABEL_33:
      ri_Printf(
        2,
        "WARNING: expected r#, const#, col0 or col1 instead of '%s' in function argument in atiFragmentShader in shader '%s'\n",
        v3,
        shaderParseName);
      return 0;
    }
    if ( !_strnicmp(v3, "col0", 4u) )
    {
      *a2 = 34167;
      v5 = 4;
    }
    else
    {
      if ( _strnicmp(v3, "col1", 4u) )
        goto LABEL_33;
      *a2 = 35181;
      v5 = 4;
    }
  }
LABEL_16:
  v7 = v3[v5];
  if ( !v7 )
    return 1;
  if ( v7 != 46 )
    goto LABEL_33;
  v8 = &v3[v5 + 1];
  if ( !v3[v5 + 2] )
  {
    v9 = *v8;
    if ( *v8 == 114 || v9 == 82 )
    {
      a2[1] = 6403;
      return 1;
    }
    switch ( v9 )
    {
      case 'g':
      case 'G':
        a2[1] = 6404;
        return 1;
      case 'b':
      case 'B':
        a2[1] = 6405;
        return 1;
      case 'a':
      case 'A':
        a2[1] = 6406;
        return 1;
    }
  }
  ri_Printf(
    2,
    "WARNING: expected .r, .g, .b or .a instead of '.%s' in function argument in atiFragmentShader in shader '%s'\n",
    v8,
    shaderParseName);
  return 0;
}

/* ---- ParseATIFS_FunctionArgs  0x004F6D80 ----  [HIGH] */
int __cdecl ParseATIFS_FunctionArgs(char **data_p, int a2, int a3, _DWORD *a4)
{
  parseInfo_t *v4;
  char *v5;
  parseInfo_t *v6;
  char *v7;
  parseInfo_t *v8;
  int *v9;
  char *v10;
  parseInfo_t *v11;
  char *v12;
  parseInfo_t *v13;
  char *v14;
  parseInfo_t *v15;
  parseInfo_t *v16;
  parseInfo_t *v17;
  char *v18;
  char *v19;
  char *v20;
  parseInfo_t *v21;
  const char *v22;
  int v23;
  int v25;

  v25 = 0;
  if ( a3 <= 0 )
    return 1;
  while ( 1 )
  {
    v4 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v4->ungetTokenSave;
      v4->currentLine = v4->ungetLineSave;
    }
    v5 = Com_ParseExt(data_p, qtrue);
    if ( strcmp(v5, "-") )
      break;
    v6 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v6->ungetTokenSave;
      v6->currentLine = v6->ungetLineSave;
    }
    v7 = Com_ParseExt(data_p, qtrue);
    if ( !strcmp(v7, "2") )
    {
      v8 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v8->ungetTokenSave;
        v8->currentLine = v8->ungetLineSave;
      }
      if ( !strcmp(Com_ParseExt(data_p, qtrue), "*") )
      {
        v9 = (int *)(a2 + 12 * v25);
        if ( !ParseATIFS_ArgReg(data_p, v9) )
          return 0;
        v9[2] = 5;
        goto LABEL_91;
      }
      Com_UngetToken();
      v9 = (int *)(a2 + 12 * v25);
      v9[2] = 5;
LABEL_90:
      *v9 = 1;
      goto LABEL_91;
    }
    if ( !strcmp(v7, "0.5") || !strcmp(v7, ".5") )
    {
      v9 = (int *)(a2 + 12 * v25);
      *v9 = 0;
LABEL_24:
      v9[2] = 8;
      goto LABEL_91;
    }
    if ( !strcmp(v7, "1") )
    {
      v9 = (int *)(a2 + 12 * v25);
      v9[2] = 4;
      goto LABEL_90;
    }
    Com_UngetToken();
    v9 = (int *)(a2 + 12 * v25);
    if ( !ParseATIFS_ArgReg(data_p, v9) )
      return 0;
    if ( !strcmp(Com_Parse(data_p), "*") )
    {
      v10 = Com_Parse(data_p);
      if ( strcmp(v10, "2") )
      {
        ri_Printf(
          2,
          "WARNING: expected 2 instead of '%s' in function argument in atiFragmentShader in shader '%s'\n",
          v10,
          shaderParseName);
        return 0;
      }
      v9[2] = 5;
    }
    else
    {
      Com_UngetToken();
      v9[2] = 4;
    }
LABEL_91:
    v22 = ")";
    if ( v25 != a3 - 1 )
      v22 = ",";
    if ( !MatchShaderToken(data_p, v22, "instruction argument list in atiFragmentShader") )
      return 0;
    v23 = *v9;
    if ( v23 == 34167 || v23 == 35181 )
      *a4 = 1;
    if ( ++v25 >= a3 )
      return 1;
  }
  if ( !strcmp(v5, "2") )
  {
    v11 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v11->ungetTokenSave;
      v11->currentLine = v11->ungetLineSave;
    }
    v12 = Com_ParseExt(data_p, qtrue);
    if ( !strcmp(v12, "*") )
    {
      v9 = (int *)(a2 + 12 * v25);
      if ( !ParseATIFS_ArgReg(data_p, v9) )
        return 0;
      v13 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v13->ungetTokenSave;
        v13->currentLine = v13->ungetLineSave;
      }
      if ( strcmp(Com_ParseExt(data_p, qtrue), "-") )
        goto LABEL_37;
      v14 = Com_Parse(data_p);
      if ( !strcmp(v14, "1") )
      {
        v9[2] = 9;
      }
      else
      {
        if ( strcmp(v14, "2") )
        {
          ri_Printf(
            2,
            "WARNING: expected N to be 1 or 2 instead of '%s' in 2 * arg - N in atiFragmentShader in shader '%s'\n",
            v14,
            shaderParseName);
          return 0;
        }
        v9[2] = 3;
      }
    }
    else
    {
      if ( strcmp(v12, "-") )
      {
        v9 = (int *)(a2 + 12 * v25);
        v9[2] = 1;
        goto LABEL_90;
      }
      v15 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v15->ungetTokenSave;
        v15->currentLine = v15->ungetLineSave;
      }
      if ( !strcmp(Com_ParseExt(data_p, qtrue), "2") )
      {
        if ( !MatchShaderToken(data_p, "*", "2 - 2 * arg in fragment argument in atiFragmentShader") )
          return 0;
        v9 = (int *)(a2 + 12 * v25);
        if ( !ParseATIFS_ArgReg(data_p, v9) )
          return 0;
        v9[2] = 3;
      }
      else
      {
        Com_UngetToken();
        v9 = (int *)(a2 + 12 * v25);
        if ( !ParseATIFS_ArgReg(data_p, v9)
          || !MatchShaderToken(data_p, "*", "2 - arg * 2 in fragment argument in atiFragmentShader")
          || !MatchShaderToken(data_p, "2", "2 - arg * 2 in fragment argument in atiFragmentShader") )
        {
          return 0;
        }
        v9[2] = 3;
      }
    }
    goto LABEL_91;
  }
  if ( !strcmp(v5, "1") )
  {
    v16 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v16->ungetTokenSave;
      v16->currentLine = v16->ungetLineSave;
    }
    if ( strcmp(Com_ParseExt(data_p, qtrue), "-") )
    {
      Com_UngetToken();
      v9 = (int *)(a2 + 12 * v25);
      goto LABEL_90;
    }
    v17 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v17->ungetTokenSave;
      v17->currentLine = v17->ungetLineSave;
    }
    if ( !strcmp(Com_ParseExt(data_p, qtrue), "2") )
    {
      if ( !MatchShaderToken(data_p, "*", "1 - 2 * arg in fragment argument in atiFragmentShader") )
        return 0;
      v9 = (int *)(a2 + 12 * v25);
      if ( !ParseATIFS_ArgReg(data_p, v9) )
        return 0;
      v9[2] = 7;
    }
    else
    {
      v9 = (int *)(a2 + 12 * v25);
      if ( !ParseATIFS_ArgReg(data_p, v9) )
        return 0;
      if ( !strcmp(Com_Parse(data_p), "*") )
      {
        if ( !MatchShaderToken(data_p, "2", "1 - arg * 2 in fragment argument in atiFragmentShader") )
          return 0;
        v9[2] = 7;
      }
      else
      {
        Com_UngetToken();
        v9[2] = 2;
      }
    }
    goto LABEL_91;
  }
  if ( !strcmp(v5, ".5") || !strcmp(v5, "0.5") )
  {
    v21 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v21->ungetTokenSave;
      v21->currentLine = v21->ungetLineSave;
    }
    if ( strcmp(Com_ParseExt(data_p, qtrue), "-") )
    {
      Com_UngetToken();
      v9 = (int *)(a2 + 12 * v25);
      v9[2] = 8;
      goto LABEL_90;
    }
    v9 = (int *)(a2 + 12 * v25);
    if ( !ParseATIFS_ArgReg(data_p, v9) )
      return 0;
    v9[2] = 12;
    goto LABEL_91;
  }
  if ( !strcmp(v5, "0") )
  {
    v9 = (int *)(a2 + 12 * v25);
    *v9 = 0;
    goto LABEL_91;
  }
  Com_UngetToken();
  v9 = (int *)(a2 + 12 * v25);
  if ( !ParseATIFS_ArgReg(data_p, v9) )
    return 0;
  v18 = Com_Parse(data_p);
  if ( !strcmp(v18, "-") )
  {
    v19 = Com_Parse(data_p);
    if ( !strcmp(v19, "0.5") || !strcmp(v19, ".5") )
      goto LABEL_24;
    if ( strcmp(v19, "1") )
    {
      ri_Printf(
        2,
        "WARNING: expected N to be 1 or .5 instead of '%s' in arg - N in atiFragmentShader in shader '%s'\n",
        v19,
        shaderParseName);
      return 0;
    }
    v9[2] = 6;
    goto LABEL_91;
  }
  if ( strcmp(v18, "*") )
  {
    Com_UngetToken();
    goto LABEL_91;
  }
  if ( !MatchShaderToken(data_p, "2", "arg * 2 in function argument in atiFragmentShader") )
    return 0;
  if ( strcmp(Com_Parse(data_p), "-") )
  {
LABEL_37:
    Com_UngetToken();
    v9[2] = 1;
    goto LABEL_91;
  }
  v20 = Com_Parse(data_p);
  if ( !strcmp(v20, "1") )
  {
    v9[2] = 9;
    goto LABEL_91;
  }
  if ( !strcmp(v20, "2") )
  {
    v9[2] = 3;
    goto LABEL_91;
  }
  ri_Printf(
    2,
    "WARNING: expected N to be 1 or 2 instead of '%s' in arg * 2 - N in atiFragmentShader in shader '%s'\n",
    v20,
    shaderParseName);
  return 0;
}

/* ---- ParseATIFS_FragOps  0x004F7670 ----  [HIGH] */
int __cdecl ParseATIFS_FragOps(char **data_p, int a2, int a3, _DWORD *a4)
{
  char **v4;
  char *v5;
  qboolean ungetReady;
  int ungetLineSave;
  char *ungetTokenSave;
  parseInfo_t *v9;
  char *v10;
  parseInfo_t *v11;
  char *v12;
  parseInfo_t *v13;
  bool v14; // zf
  char *v15;
  parseInfo_t *v16;
  char v17;
  int v18;
  char v19;
  parseInfo_t *v20;
  int v21;
  int v22;
  bool v23; // cc
  parseInfo_t *v24;
  char **v25;
  char *v26;
  int *v27;
  parseInfo_t *v28;
  char *v29;
  char v30;
  parseInfo_t *v31;
  BOOL v32;
  char *v33;
  char v34;
  char *v35;
  char *v36;
  char *v37;
  int v38;
  char *v39;
  const char *v41;
  const char *v42;
  const char *v43;
  int v44;
  int v45; // [esp+14h] [ebp-3Ch] BYREF
  _DWORD *v46;
  int v47; // [esp+1Ch] [ebp-34h] BYREF
  int v48; // [esp+20h] [ebp-30h] BYREF
  int v49;
  int v50;
  int v51;
  int v52; // [esp+30h] [ebp-20h] BYREF
  int v53;
  int v54;
  shaderParseMark_t mark; // [esp+3Ch] [ebp-14h] BYREF, 20 bytes

  v4 = data_p;
  v5 = *data_p;
  mark.currentLine = parseInfo->currentLine;
  ungetReady = parseInfo->ungetReady;
  mark.data = v5;
  ungetLineSave = parseInfo->ungetLineSave;
  ungetTokenSave = parseInfo->ungetTokenSave;
  mark.ungetReady = ungetReady;
  mark.ungetTokenSave = ungetTokenSave;
  v54 = 0;
  v50 = 0;
  mark.ungetLineSave = ungetLineSave;
  ParseATIFS_DestReg(data_p, &v48, &v45, &v52, &v47);
  if ( !v48 )
    return 1;
  v49 = 0;
  v46 = (_DWORD *)(a2 + 8);
  while ( 1 )
  {
    v9 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *v4 = v9->ungetTokenSave;
      v9->currentLine = v9->ungetLineSave;
    }
    v10 = Com_ParseExt(v4, qtrue);
    if ( _stricmp(v10, "=") )
    {
      v43 = v10;
      v42 = "=";
LABEL_100:
      v41 = "fragment instruction of atiFragmentShader";
      goto LABEL_101;
    }
    v11 = parseInfo;
    v44 = 0;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *v4 = v11->ungetTokenSave;
      v11->currentLine = v11->ungetLineSave;
    }
    v12 = Com_ParseExt(v4, qtrue);
    if ( !_stricmp(v12, "clamp") )
    {
      v13 = parseInfo;
      v14 = parseInfo->ungetReady == qfalse;
      v51 = 1;
      if ( !v14 )
      {
        parseInfo->ungetReady = qfalse;
        *v4 = v13->ungetTokenSave;
        v13->currentLine = v13->ungetLineSave;
      }
      v15 = Com_ParseExt(v4, qtrue);
      if ( _stricmp(v15, "(") )
      {
        v43 = v15;
        v42 = "(";
        v41 = "clamp fragment instruction of atiFragmentShader";
LABEL_101:
        ri_Printf(2, "WARNING: %s missing '%s', found '%s' instead in shader '%s'\n", v41, v42, v43, shaderParseName);
        return 0;
      }
      v16 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *v4 = v16->ungetTokenSave;
        v16->currentLine = v16->ungetLineSave;
      }
      v12 = Com_ParseExt(v4, qtrue);
    }
    else
    {
      v51 = 0;
    }
    v17 = v12[1];
    v18 = 0;
    v53 = 0;
    if ( !v17 )
    {
      v19 = *v12;
      if ( *v12 == 50 )
      {
        v44 = 1;
      }
      else
      {
        if ( v19 != 52 && v19 != 56 )
          goto LABEL_25;
        v44 = 2 * (v19 != 52) + 2;
      }
      v18 = 1;
      v53 = 1;
      if ( !MatchShaderToken(v4, "*", "prescaled fragment instruction of atiFragmentShader") )
        return 0;
      v20 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *v4 = v20->ungetTokenSave;
        v20->currentLine = v20->ungetLineSave;
      }
      v12 = Com_ParseExt(v4, qtrue);
    }
LABEL_25:
    if ( !_stricmp(v12, "mov") )
    {
      v21 = 35169;
      v22 = 1;
      goto LABEL_52;
    }
    if ( !_stricmp(v12, "add") )
    {
      v21 = 35171;
      v22 = 2;
      goto LABEL_52;
    }
    if ( !_stricmp(v12, "sub") )
    {
      v21 = 35173;
      v22 = 2;
      goto LABEL_52;
    }
    if ( !_stricmp(v12, "mul") )
    {
      v21 = 35172;
      v22 = 2;
      goto LABEL_52;
    }
    if ( !_stricmp(v12, "dot3") )
    {
      if ( !v45 )
      {
        ri_Printf(2, "WARNING: dot3 cannot write only alpha in atiTexShader in shader '%s'\n", shaderParseName);
        return 0;
      }
      v21 = 35174;
      v22 = 2;
      goto LABEL_52;
    }
    if ( _stricmp(v12, "dot4") )
      break;
    if ( !v45 )
    {
      ri_Printf(2, "WARNING: dot4 cannot write only alpha in atiTexShader in shader '%s'\n", shaderParseName);
      return 0;
    }
    v21 = 35175;
    v22 = 2;
LABEL_52:
    if ( v45 && v54 || v50 )
    {
      v23 = v49 + 2 < 16;
      v49 += 2;
      v46 += 26;
      if ( !v23 )
      {
        ri_Printf(2, "WARNING: more than %i merged operations in atiFragmentShader in shader '%s'\n", 8, shaderParseName);
        return 0;
      }
    }
    v24 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *v4 = v24->ungetTokenSave;
      v24->currentLine = v24->ungetLineSave;
    }
    v25 = data_p;
    v26 = Com_ParseExt(data_p, qtrue);
    if ( _stricmp(v26, "(") )
    {
      v43 = v26;
      v42 = "(";
      goto LABEL_100;
    }
    v27 = (int *)(52 * (v47 + v49) + a2);
    if ( !ParseATIFS_FunctionArgs(data_p, (int)(v27 + 4), v22, a4) )
      return 0;
    v28 = parseInfo;
    if ( !v53 )
    {
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        *data_p = v28->ungetTokenSave;
        v28->currentLine = v28->ungetLineSave;
      }
      v29 = Com_ParseExt(data_p, qtrue);
      if ( !v29[1] && ((v30 = *v29, v30 == 42) || v30 == 47) )
      {
        v14 = v30 == 42;
        v31 = parseInfo;
        v32 = v14;
        if ( parseInfo->ungetReady )
        {
          parseInfo->ungetReady = qfalse;
          *data_p = v31->ungetTokenSave;
          v31->currentLine = v31->ungetLineSave;
        }
        v33 = Com_ParseExt(data_p, qtrue);
        if ( v33[1] )
        {
LABEL_106:
          ri_Printf(2, "WARNING: bad scale '%s' in atiFragmentShader in shader '%s'\n", v33, shaderParseName);
          return 0;
        }
        v34 = *v33;
        if ( *v33 == 50 )
        {
          v28 = parseInfo;
          v44 = v32 ? 1 : 8;
          v25 = data_p;
        }
        else if ( v34 == 52 )
        {
          v28 = parseInfo;
          v44 = v32 ? 2 : 16;
          v25 = data_p;
        }
        else
        {
          if ( v34 != 56 )
            goto LABEL_106;
          v28 = parseInfo;
          v44 = v32 ? 4 : 32;
          v25 = data_p;
        }
      }
      else
      {
        v28 = parseInfo;
        Com_UngetToken();
      }
    }
    if ( v51 )
    {
      if ( v28->ungetReady )
      {
        v28->ungetReady = qfalse;
        *v25 = v28->ungetTokenSave;
        v28->currentLine = v28->ungetLineSave;
      }
      v36 = Com_ParseExt(v25, qtrue);
      if ( _stricmp(v36, ")") )
      {
        v43 = v36;
        v42 = ")";
        v41 = "clamped fragment instruction of atiFragmentShader";
        goto LABEL_101;
      }
      v44 |= 0x40u;
      v28 = parseInfo;
    }
    if ( v28->ungetReady )
    {
      v28->ungetReady = qfalse;
      *v25 = v28->ungetTokenSave;
      v28->currentLine = v28->ungetLineSave;
    }
    v37 = Com_ParseExt(v25, qtrue);
    if ( _stricmp(v37, ";") )
    {
      v43 = v37;
      v42 = ";";
      goto LABEL_100;
    }
    v38 = v48;
    *v27 = v21;
    v27[1] = v38;
    v27[3] = v44;
    if ( v45 )
    {
      if ( v47 )
        qmemcpy(v46 - 2, v46 + 11, 0x34u);
      *v46 = v52 == 6407 ? 0 : v52;
    }
    v54 = v45;
    if ( v47 || (v50 = 0, v21 == 35175) )
      v50 = 1;
    mark.currentLine = parseInfo->currentLine;
    mark.data = *data_p;
    mark.ungetReady = parseInfo->ungetReady;
    v39 = parseInfo->ungetTokenSave;
    mark.ungetLineSave = parseInfo->ungetLineSave;
    mark.ungetTokenSave = v39;
    ParseATIFS_DestReg(data_p, &v48, &v45, &v52, &v47);
    if ( !v48 )
      return 1;
    v4 = data_p;
  }
  if ( !_stricmp(v12, "mad") )
  {
    v21 = 35176;
LABEL_51:
    v22 = 3;
    goto LABEL_52;
  }
  if ( !_stricmp(v12, "lerp") )
  {
    v21 = 35177;
    goto LABEL_51;
  }
  if ( !_stricmp(v12, "cnd") )
  {
    v21 = 35178;
    goto LABEL_51;
  }
  if ( !_stricmp(v12, "cnd0") )
  {
    v21 = 35179;
    goto LABEL_51;
  }
  if ( !_stricmp(v12, "dot2_add") )
  {
    if ( !v45 )
    {
      ri_Printf(2, "WARNING: dot2_add cannot write only alpha in atiTexShader in shader '%s'\n", shaderParseName);
      return 0;
    }
    v21 = 35180;
    goto LABEL_51;
  }
  if ( !v51 && !v18 && !v52 )
  {
    Com_ParseReturnToMark(&mark.currentLine, v4);
    return 1;
  }
  ri_Printf(2, "WARNING: unknown function '%s' in atiFragmentShader in shader '%s'\n", v12, shaderParseName);
  return 0;
}

/* ---- ParseATIFS_Phase  0x004F7EC0 ----  VERIFIED */
int __cdecl ParseATIFS_Phase(int a1, char **a2, int a3, _DWORD *a4)
{
  int result;
  int v6;

  result = ParseATIFS_TexReads(a2, a1, a3);
  if ( result )
    return ParseATIFS_FragOps(a2, a1 + 72, a3, a4) != 0;
  return result;
}

/* ---- ATIFS_ColorOp  0x004F7EF0 ----  [HIGH] */
int *__cdecl ATIFS_ColorOp(int *result)
{
  int v1;

  v1 = *result;
  switch ( *result )
  {
    case 35169:
      qglColorFragmentOp1ATI(v1, result[1], result[2], result[3], result[4], result[5], result[6]);
      break;
    case 35171:
    case 35172:
    case 35173:
    case 35174:
    case 35175:
      qglColorFragmentOp2ATI(
                        v1,
                        result[1],
                        result[2],
                        result[3],
                        result[4],
                        result[5],
                        result[6],
                        result[7],
                        result[8],
                        result[9]);
      break;
    case 35176:
    case 35177:
    case 35178:
    case 35179:
    case 35180:
      qglColorFragmentOp3ATI(
                        v1,
                        result[1],
                        result[2],
                        result[3],
                        result[4],
                        result[5],
                        result[6],
                        result[7],
                        result[8],
                        result[9],
                        result[10],
                        result[11],
                        result[12]);
      break;
    default:
      return result;
  }
  return result;
}

/* ---- ATIFS_AlphaOp  0x004F7FB0 ----  VERIFIED */
int *__cdecl ATIFS_AlphaOp(int *result)
{
  int v1;

  v1 = *result;
  switch ( *result )
  {
    case 35169:
      qglAlphaFragmentOp1ATI(v1, result[1], result[3], result[4], result[5], result[6]);
      break;
    case 35171:
    case 35172:
    case 35173:
    case 35174:
    case 35175:
      qglAlphaFragmentOp2ATI(
                        v1,
                        result[1],
                        result[3],
                        result[4],
                        result[5],
                        result[6],
                        result[7],
                        result[8],
                        result[9]);
      break;
    case 35176:
    case 35177:
    case 35178:
    case 35179:
    case 35180:
      qglAlphaFragmentOp3ATI(
                        v1,
                        result[1],
                        result[3],
                        result[4],
                        result[5],
                        result[6],
                        result[7],
                        result[8],
                        result[9],
                        result[10],
                        result[11],
                        result[12]);
      break;
    default:
      return result;
  }
  return result;
}

/* ---- ParseATIFragmentShader  0x004F8070 ----  VERIFIED */
int __cdecl ParseATIFragmentShader(char **a1, int a2, int a3)
{
  int v5;
  int v6;
  _BYTE *v7;
  int v8;
  _BYTE *v9;
  int v10;
  int *v11;
  int v12;
  int v13;
  int *v14;
  int i;
  bool v16; // zf
  int v17;
  int v18;
  int v19; // [esp+18h] [ebp-7BCh] BYREF
  int v20;
  int v21;
  _BYTE v22[1968]; // [esp+24h] [ebp-7B0h] BYREF

  if ( !rendererShaderRequirements[14] )
  {
    ri_Printf(
      2,
      "WARNING: shader '%s' uses atiFragmentShader without 'requires GL_ATI_fragment_shader'\n",
      shaderParseName);
    return 0;
  }
  if ( !MatchShaderToken(a1, "{", "start of atiFragmentShader") )
    return 0;
  memset(v22, 0, sizeof(v22));
  v19 = 0;
  if ( !ParseATIFS_ConstDefs(a1, (int)v22)
    || !ParseATIFS_TexReads(a1, (int)&v22[160], 0)
    || !ParseATIFS_FragOps(a1, (int)&v22[232], 0, &v19) )
  {
    return 0;
  }
  if ( v19 )
  {
    if ( strcmp(Com_Parse(a1), "}") )
    {
      ri_Printf(
        2,
        "WARNING: missing '}' at end of atiFragmentShader... might be because 'col0' or 'col1' was referenced in first ph"
        "ase in shader %s\n",
        shaderParseName);
      return 0;
    }
    v19 = 1;
  }
  else
  {
    if ( !ParseATIFS_Phase((int)&v22[1064], a1, 1, &v19) || !MatchShaderToken(a1, "}", "end of atiFragmentShader") )
      return 0;
    v19 = 2;
  }
  if ( a3 )
  {
    v5 = dword_16D384C + 1;
    v21 = dword_16D384C + 1;
    qglGetError_0();
    glBindFragmentShaderATI(v5);
    qglBeginFragmentShaderATI();
    v6 = 0;
    v7 = v22;
    do
    {
      if ( *((_DWORD *)v7 + 4) )
        qglSetFragmentShaderConstantATI(v6 + 35137, v7);
      ++v6;
      v7 += 20;
    }
    while ( v6 < 8 );
    v8 = v19;
    if ( v19 > 0 )
    {
      v9 = &v22[284];
      v19 = (int)&v22[284];
      v20 = v8;
      do
      {
        v10 = 35105;
        v11 = (int *)(v9 - 120);
        v12 = 6;
        do
        {
          v13 = *v11;
          if ( *v11 )
          {
            if ( *(v11 - 1) )
              qglSampleMapATI(v10, v13, v11[1]);
            else
              qglPassTexCoordATI(v10, v13, v11[1]);
          }
          v11 += 3;
          ++v10;
          --v12;
        }
        while ( v12 );
        v14 = (int *)v19;
        for ( i = 0; i < 8; ++i )
        {
          if ( !*(v14 - 13) && !*v14 )
            break;
          ATIFS_ColorOp(v14 - 13);
          ATIFS_AlphaOp(v14);
          v14 += 26;
        }
        v9 = (_BYTE *)(v19 + 904);
        v16 = v20 == 1;
        v19 += 904;
        --v20;
      }
      while ( !v16 );
      v5 = v21;
    }
    qglEndFragmentShaderATI();
    v17 = qglGetError_0();
    if ( v17 )
    {
      glBindFragmentShaderATI(0);
      qglDeleteFragmentShaderATI(v5);
      ri_Printf(
        2,
        "WARNING: glGetError() returned 0x%04X when compiling atiFragmentShader in shader %s\n",
        v17,
        shaderParseName);
      return 0;
    }
    *(_DWORD *)(a2 + 1604) = v5;
    glBindFragmentShaderATI(0);
    ++dword_16D384C;
  }
  return 1;
}

/* ---- ParseVertexProgram  0x004F8310 ----  [HIGH] */
int __cdecl ParseVertexProgram(char **data_p, int a3)
{
  parseInfo_t *v3;
  const char *v4;
  char *VertexProgram;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v3->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v3 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  v4 = (const char *)v3;
  VertexProgram = R_FindVertexProgram(v3->token);
  *(_DWORD *)(a3 + 1612) = VertexProgram;
  if ( VertexProgram )
    return 1;
  ri_Printf(2, "WARNING: couldn't load vertex program '%s' in shader '%s'\n", v4, shaderParseName);
  return 0;
}

/* ---- ParseStage  0x004F8390 ----  [HIGH] */
int __cdecl ParseStage(
    int    *stage,
    char  **data_p,
    int     allowTextureName,
    int     imageTrack,
    int    *outStageActive )
{
  int    *bundle;
  char   *token;
  int     bundleIndex   = 0;
  int     stageActive   = 1;
  int     multiplyImage = 0;
  int     explicitDepthWrite = 0;

  unsigned int depthMaskBits       = 256;
  unsigned int srcBlendBits        = 0;
  unsigned int dstBlendBits        = 0;
  unsigned int alphaTestBits       = 0;
  unsigned int depthFuncBits       = 0;
  unsigned int lightingBits        = 0;
  unsigned int textureShaderBits   = 0;          /* 0x00400000 GLS_TEXTURE_SHADER_NV     */
  unsigned int registerCombinerBits= 0;          /* 0x00800000 GLS_REGISTER_COMBINERS_NV */
  unsigned int fragmentShaderBits  = 0;          /* 0x01000000 GLS_FRAGMENT_SHADER_ATI   */
  unsigned int vertexProgramBits   = 0;          /* 0x02000000 GLS_VERTEX_PROGRAM_ARB    */

  float imageColorScale[4];
  float constColor[3];

  int   i, j, k;
  int   parsedSrcBlend, parsedDstBlend;
  int   cubeImageFlags, cubeImageTrack;
  int   image;
  int   oneShot;
  char  tcModText[1024];
  char *tcModCursor;

  imageColorScale[0] = 1.0f;
  imageColorScale[1] = 1.0f;
  imageColorScale[2] = 1.0f;
  imageColorScale[3] = 1.0f;

  *stage |= 1u;
  stage[36] = 2;
  stage[37] = 8448;
  memset( rendererShaderRequirements, 0, RENDERER_SHADER_REQUIREMENTS_COUNT );

  for ( ;; )
  {
    bundle = &stage[50 * bundleIndex];

    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p             = parseInfo->ungetTokenSave;
      parseInfo->currentLine = parseInfo->ungetLineSave;
    }
    token = Com_ParseExt( data_p, qtrue );

    if ( !*token )
    {
      ri_Printf( 2, "WARNING: no matching '}' found\n" );
      return 0;
    }

    if ( *token == '}' )
    {
      if ( stageActive && !bundle[1] )
      {
        ri_Printf( 2, "WARNING: shader '%s' has bundle with no map\n", shaderParseName );
        return 0;
      }

      if ( textureShaderBits )
      {
        for ( i = 0; i < glConfig_maxActiveTextures; ++i )
        {
          int *b = &stage[50 * i];
          if ( !b[39] )
          {
            int *ts = (int *)ri_Hunk_Alloc( 28 );
            int  img = b[1];
            b[39] = (int)ts;
            *ts = img ? *(int *)( img + 84 ) : 0;
          }
        }
      }

      if ( !stage[409] )
      {
        if ( !srcBlendBits || srcBlendBits == 2 || srcBlendBits == 5 )
          stage[409] = 1;
        else
          stage[409] = 2;
      }

      if ( srcBlendBits == 2 && dstBlendBits == 16 )
      {
        srcBlendBits  = 0;
        dstBlendBits  = 0;
        depthMaskBits = 256;
      }

      if ( !stage[415] )
      {
        int rgbGen = stage[409];
        if ( rgbGen == 2 || rgbGen == 9 || rgbGen == 10 )
          stage[415] = 1;
      }

      stage[417] = depthMaskBits | srcBlendBits | dstBlendBits | alphaTestBits
                 | depthFuncBits | textureShaderBits | registerCombinerBits
                 | fragmentShaderBits | vertexProgramBits | lightingBits;
      *outStageActive = stageActive;
      return 1;
    }

    if ( registerCombinerBits )
    {
      ri_Printf( 2, "WARNING: nvRegCombiners must be the last thing in a stage\n" );
      return 0;
    }

    if ( !Q_stricmpn( "map", token, 99999 ) )
    {
      int *outImage = &stage[50 * bundleIndex + 1 + bundle[33]];

      if ( !ParseImage( data_p, allowTextureName, stageActive, imageTrack,
                        multiplyImage ? (int)imageColorScale : 0, outImage ) )
        return 0;

      image = *outImage;
      if ( image )
      {
        if ( ( shaderParseLightmapIndex >= 0 && image == tr_lightmaps[shaderParseLightmapIndex] )
          || image == tr_identityLightImage
          || image == tr_dlightImage )
        {
          *( (char *)bundle + 200 ) = 1;
        }
        ++bundle[33];
        if ( bundle[33] > 1 && *(float *)&bundle[34] == 0.0f )
        {
          ri_Printf( 2,
            "WARNING: multiple 'map' specifications without preceding 'animmap' in shader'%s'\n",
            shaderParseName );
          return 0;
        }
      }
      continue;
    }

    if ( !Q_stricmpn( "animMap", token, 99999 )
      || !Q_stricmpn( "oneshotanimMap", token, 99999 ) )
    {
      oneShot = Q_stricmpn( "oneshotanimMap", token, 99999 ) == 0;
      shaderParseSurfaceFlags |= 0x10u;
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2,
          "WARNING: missing parameter for 'animMmap' keyword in shader '%s'\n",
          shaderParseName );
        return 0;
      }
      *(float *)&stage[50 * bundleIndex + 34] = (float)atof( token );
      *( (char *)stage + 202 ) = (char)oneShot;
      Com_SkipRestOfLine( 0, data_p );
      continue;
    }

    if ( !Q_stricmpn( "videoMap", token, 99999 ) )
    {
      int handle;
      shaderParseSurfaceFlags |= 0x11u;
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2,
          "WARNING: missing parameter for 'videoMap' keyword in shader '%s'\n",
          shaderParseName );
        return 0;
      }
      if ( multiplyImage )
      {
        ri_Printf( 2,
          "WARNING: tried to use multiplyImage with videoMap in shader '%s'\n",
          shaderParseName );
        return 0;
      }
      handle = ri_CIN_PlayCinematic( token, 0, 0, 256, 256, 26 );
      bundle[49] = handle;
      if ( handle != -1 )
      {
        *( (char *)bundle + 201 ) = 1;
        bundle[1] = tr_scratchImages[handle];
      }
      continue;
    }

    if ( !Q_stricmpn( "cubeMap", token, 99999 ) )
    {
      cubeImageFlags = 51;
      if ( ( shaderParseFlags & 0x200 ) != 0 )
        cubeImageFlags = 55;
      if ( ( shaderParseFlags & 0x80u ) != 0 )
        cubeImageFlags &= ~1u;
      if ( ( shaderParseFlags & 0x100 ) != 0 )
        cubeImageFlags &= ~2u;

      if ( !rendererShaderRequirements[6] )
      {
        ri_Printf( 2,
          "WARNING: shader '%s' uses cubeMap without 'requires GL_ARB_texture_cube_map'\n",
          shaderParseName );
        return 0;
      }

      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2,
          "WARNING: missing parameter for 'cubeMap' in shader '%s'\n",
          shaderParseName );
        return 0;
      }

      cubeImageTrack = imageTrack;
      if ( !Q_stricmp( "$texturename", token ) )
      {
        if ( !allowTextureName )
        {
          ri_Printf( 2,
            "WARNING: $texturename used in shader '%s', which is not a shader type file\n",
            shaderParseName );
          return 0;
        }
        if ( !Q_stricmp( "+", Com_ParseOnLine( data_p ) ) )
          token = Com_ParseOnLine( data_p );
        else
          token = (char *)empty_string;
        cubeImageTrack = 10;
      }

      if ( stageActive )
      {
        image = R_FindImageFile( token, 34067 ,
                                 cubeImageFlags, cubeImageTrack,
                                 multiplyImage ? (int)imageColorScale : 0,
                                 1.0f );
        if ( !image )
        {
          ri_Printf( 2,
            "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n",
            token, shaderParseName );
          return 0;
        }
        stage[50 * bundleIndex + 1 + bundle[33]] = image;
        if ( !bundle[33] )
          bundle[36] = 3;
        ++bundle[33];
        if ( bundle[33] > 1 && *(float *)&bundle[34] == 0.0f )
        {
          ri_Printf( 2,
            "WARNING: multiple 'cubemap' specifications without preceding 'animmap' in shader'%s'\n",
            shaderParseName );
          return 0;
        }
      }
      Com_SkipRestOfLine( 0, data_p );
      continue;
    }

    if ( !Q_stricmpn( "waterMap", token, 99999 ) )
    {
      if ( bundle[33] )
      {
        ri_Printf( 2,
          "WARNING: multiple 'waterMap' must be only map in shader '%s'\n",
          shaderParseName );
        return 0;
      }
      if ( !ParseWaterMap( (int)stage, data_p, bundleIndex ) )
        return 0;
      Com_SkipRestOfLine( 0, data_p );
      continue;
    }

    if ( !Q_stricmp( "nvTexShader", token ) )
    {
      int ok;
      parseInfo->spaceDelimited = qfalse;
      ok = ParseNVTexShader( data_p, (int)stage, bundleIndex );
      parseInfo->spaceDelimited = qtrue;
      if ( !ok )
        return 0;
      Com_SkipRestOfLine( 0, data_p );
      textureShaderBits = 0x400000;
      continue;
    }

    if ( !Q_stricmp( "nvRegCombiners", token ) )
    {
      int ok;
      parseInfo->spaceDelimited = qfalse;
      ok = ParseNVRegCombiners( (int)stage, data_p );
      parseInfo->spaceDelimited = qtrue;
      if ( !ok )
        return 0;
      Com_SkipRestOfLine( 0, data_p );
      registerCombinerBits = 0x800000;
      continue;
    }

    if ( !Q_stricmp( "atiFragmentShader", token ) )
    {
      int ok;
      parseInfo->spaceDelimited        = qfalse;
      parseInfo->parseNegativeNumbers  = qfalse;
      ok = ParseATIFragmentShader( data_p, (int)stage, stageActive );
      parseInfo->spaceDelimited        = qtrue;
      parseInfo->parseNegativeNumbers  = qtrue;
      if ( !ok )
        return 0;
      Com_SkipRestOfLine( 0, data_p );
      fragmentShaderBits = 0x1000000;
      continue;
    }

    if ( !Q_stricmp( "alphaFunc", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'alphaFunc' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2,
          "WARNING: missing parameter for 'alphaFunc' keyword in shader '%s'\n",
          shaderParseName );
        return 0;
      }
      alphaTestBits = NameToAFunc( token );
      continue;
    }

    if ( !Q_stricmp( "depthfunc", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'depthFunc' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2,
          "WARNING: missing parameter for 'depthfunc' keyword in shader '%s'\n",
          shaderParseName );
        return 0;
      }
      if ( !Q_stricmp( "lequal", token ) )
        depthFuncBits = 0;
      else if ( !Q_stricmp( "equal", token ) )
        depthFuncBits = 0x20000;
      else if ( !Q_stricmp( "always", token ) )
        depthFuncBits = 0x40000;
      else
        ri_Printf( 2, "WARNING: unknown depthfunc '%s' in shader '%s'\n",
                   token, shaderParseName );
      continue;
    }

    if ( !Q_stricmp( "detail", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'detail' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      *stage |= 2u;
      continue;
    }

    if ( !Q_stricmp( "perLight", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'perLight' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      *stage |= 0x80u;
      continue;
    }

    if ( !Q_stricmp( "fog", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'fog' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2, "WARNING: missing parm for fog in shader '%s'\n", shaderParseName );
      }
      else if ( !Q_stricmp( "on", token ) )
      {
        *stage |= 4u;
      }
      else
      {
        *stage &= ~4;
      }
      continue;
    }

    if ( !Q_stricmp( "blendfunc", token ) )
    {
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2, "WARNING: missing parm for blendFunc in shader '%s'\n",
                   shaderParseName );
        continue;
      }

      if ( !Q_stricmp( "add", token ) )
      {
        parsedSrcBlend = 2;  parsedDstBlend = 32;
      }
      else if ( !Q_stricmp( "filter", token ) )
      {
        parsedSrcBlend = 3;  parsedDstBlend = 16;
      }
      else if ( !Q_stricmp( "blend", token ) )
      {
        parsedSrcBlend = 5;  parsedDstBlend = 96;
      }
      else
      {
        parsedSrcBlend = NameToSrcBlendMode( token );
        token = Com_ParseOnLine( data_p );
        if ( !*token )
        {
          ri_Printf( 2, "WARNING: missing parm for localFunc in shader '%s'\n",
                     shaderParseName );
          continue;
        }
        parsedDstBlend = NameToDstBlendMode( token );
      }

      if ( bundleIndex )
      {
        if ( parsedSrcBlend == 2 && parsedDstBlend == 32 )
        {
          if ( !rendererShaderRequirements[7] )
          {
            ri_Printf( 2,
              "WARNING: shader '%s' uses optional GL_ARB_texture_env_add without suitable 'requires' statement\n",
              shaderParseName );
            return 0;
          }
          bundle[37] = 260;
        }
        else if ( ( parsedSrcBlend == 3 && parsedDstBlend == 16 )
               || ( parsedSrcBlend == 1 && parsedDstBlend == 48 ) )
        {
          bundle[37] = 8448;
        }
        else if ( parsedSrcBlend == 5 && parsedDstBlend == 96 )
        {
          bundle[37] = 8449;
        }
        else
        {
          ri_Printf( 2, "WARNING: bad blendFunc for nextbundle %i in shader '%s'\n",
                     bundleIndex, shaderParseName );
          return 0;
        }
      }
      else
      {
        srcBlendBits = parsedSrcBlend;
        dstBlendBits = parsedDstBlend;
      }

      if ( !explicitDepthWrite )
        depthMaskBits = 0;
      continue;
    }

    if ( !Q_stricmp( "rgbGen", token ) )
    {
      int constantLighting;

      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'rgbGen' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2, "WARNING: missing parameters for rgbGen in shader '%s'\n",
                   shaderParseName );
        continue;
      }

      lightingBits = 0;
      if ( !Q_stricmp( "wave", token ) )
      {
        shaderParseSurfaceFlags |= 1u;
        ParseWaveForm( (int)&stage[404], data_p );
        stage[409] = 8;
        continue;
      }

      constantLighting = Q_stricmp( "constLighting", token ) == 0;
      if ( !Q_stricmp( "const", token ) || constantLighting )
      {
        ParseVector( 3, data_p, (int)constColor );
        if ( constantLighting )
        {
          constColor[0] = tr_identityLight * constColor[0];
          constColor[1] = tr_identityLight * constColor[1];
          constColor[2] = tr_identityLight * constColor[2];
        }
        for ( i = 0; i < 3; ++i )
          *( (char *)stage + 1664 + i ) = (char)(int)( constColor[i] * 255.0f );
        stage[409] = 12;
      }
      else if ( !Q_stricmp( "identity", token ) )
      {
        stage[409] = 2;
      }
      else if ( !Q_stricmp( "identityLighting", token ) )
      {
        stage[409] = 1;
      }
      else if ( !Q_stricmp( "entity", token ) )
      {
        stage[409] = 3;
      }
      else if ( !Q_stricmp( "oneMinusEntity", token ) )
      {
        stage[409] = 4;
      }
      else if ( !Q_stricmp( "vertex", token ) )
      {
        shaderParseSurfaceFlags |= 0x40u;
        stage[409] = 6;
        if ( !stage[415] )
          stage[415] = 4;
      }
      else if ( !Q_stricmp( "exactVertex", token ) )
      {
        shaderParseSurfaceFlags |= 0x40u;
        stage[409] = 5;
        if ( !stage[415] )
          stage[415] = 4;
      }
      else if ( !Q_stricmp( "lightingAmbient", token ) )
      {
        stage[409]   = 9;
        lightingBits = 0x100000;
      }
      else if ( !Q_stricmp( "lightingDiffuse", token ) )
      {
        stage[409]   = 10;
        lightingBits = 0x100000;
      }
      else if ( !Q_stricmp( "oneMinusVertex", token ) )
      {
        shaderParseSurfaceFlags |= 0x40u;
        stage[409] = 7;
      }
      else if ( !Q_stricmp( "lightingPrecalc", token ) )
      {
        stage[409] = 11;
      }
      else
      {
        ri_Printf( 2, "WARNING: unknown rgbGen parameter '%s' in shader '%s'\n",
                   token, shaderParseName );
        Com_SkipRestOfLine( 0, data_p );
      }
      continue;
    }

    if ( !Q_stricmp( "alphaGen", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'alphaGen' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2, "WARNING: missing parameters for alphaGen in shader '%s'\n",
                   shaderParseName );
        continue;
      }

      if ( !Q_stricmp( "wave", token ) )
      {
        shaderParseSurfaceFlags |= 1u;
        ParseWaveForm( (int)&stage[410], data_p );
        stage[415] = 7;
      }
      else if ( !Q_stricmp( "const", token ) )
      {
        *( (char *)stage + 1667 ) =
            (char)(int)( atof( Com_ParseOnLine( data_p ) ) * 255.0 );
        stage[415] = 9;
      }
      else if ( !Q_stricmp( "constLighting", token ) )
      {
        *( (char *)stage + 1667 ) =
            (char)(int)( atof( Com_ParseOnLine( data_p ) ) * tr_identityLight * 255.0 );
        stage[415] = 9;
      }
      else if ( !Q_stricmp( "identity", token ) )
      {
        stage[415] = 0;
      }
      else if ( !Q_stricmp( "entity", token ) )
      {
        stage[415] = 2;
      }
      else if ( !Q_stricmp( "oneMinusEntity", token ) )
      {
        stage[415] = 3;
      }
      else if ( !Q_stricmp( "vertex", token ) )
      {
        shaderParseSurfaceFlags |= 0x40u;
        stage[415] = 4;
      }
      else if ( !Q_stricmp( "lightingSpecular", token ) )
      {
        stage[415] = 6;
      }
      else if ( !Q_stricmp( "oneMinusVertex", token ) )
      {
        shaderParseSurfaceFlags |= 0x40u;
        stage[415] = 5;
      }
      else if ( !Q_stricmp( "portal", token ) )
      {
        stage[415] = 8;
        token = Com_ParseOnLine( data_p );
        if ( *token )
        {
          *(float *)&shaderParsePortalRange = (float)atof( token );
        }
        else
        {
          shaderParsePortalRange = 1132462080;
          ri_Printf( 2,
            "WARNING: missing range parameter for alphaGen portal in shader '%s', defaulting to 256\n",
            shaderParseName );
        }
      }
      else if ( !Q_stricmp( "dot", token ) )
      {
        shaderParseSurfaceFlags |= 2u;
        stage[415] = 10;
      }
      else if ( !Q_stricmp( "oneMinusDot", token ) )
      {
        shaderParseSurfaceFlags |= 2u;
        stage[415] = 11;
      }
      else if ( !Q_stricmp( "onePlusDot", token ) )
      {
        shaderParseSurfaceFlags |= 2u;
        stage[415] = 12;
      }
      else if ( !Q_stricmp( "negativeDot", token ) )
      {
        shaderParseSurfaceFlags |= 2u;
        stage[415] = 13;
      }
      else
      {
        ri_Printf( 2, "WARNING: unknown alphaGen parameter '%s' in shader '%s'\n",
                   token, shaderParseName );
      }
      continue;
    }

    if ( !Q_stricmp( "texgen", token ) || !Q_stricmp( "tcGen", token ) )
    {
      if ( !stageActive )
      {
        Com_SkipRestOfLine( 0, data_p );
        continue;
      }
      if ( !bundle[1] )
      {
        ri_Printf( 2, "WARNING: texgen before image in shader '%s'\n", shaderParseName );
        return 0;
      }
      token = Com_ParseOnLine( data_p );
      if ( !*token )
      {
        ri_Printf( 2, "WARNING: missing texgen parm in shader '%s'\n", shaderParseName );
        continue;
      }

      if ( !Q_stricmp( "normal", token ) )
      {
        shaderParseSurfaceFlags |= 2u;   bundle[40] = 7;  bundle[36] = 3;
      }
      else if ( !Q_stricmp( "tangent", token ) )
      {
        shaderParseSurfaceFlags |= 4u;   bundle[40] = 8;  bundle[36] = 3;
      }
      else if ( !Q_stricmp( "binormal", token ) )
      {
        shaderParseSurfaceFlags |= 8u;   bundle[40] = 9;  bundle[36] = 3;
      }
      else if ( !Q_stricmp( "tbn_x", token ) )
      {
        shaderParseSurfaceFlags |= 0xEu; bundle[40] = 10; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "tbn_y", token ) )
      {
        shaderParseSurfaceFlags |= 0xEu; bundle[40] = 11; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "tbn_z", token ) )
      {
        shaderParseSurfaceFlags |= 0xEu; bundle[40] = 12; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "eyeToVertex", token ) )
      {
        bundle[40] = 14; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "vertexToEye", token ) )
      {
        bundle[40] = 13; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "reflection", token ) )
      {
        shaderParseSurfaceFlags |= 2u;   bundle[40] = 15; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "lightvector", token ) )
      {
        bundle[40] = 16; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "lighthalfangle", token ) )
      {
        bundle[40] = 17; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "sunhalfangle", token ) )
      {
        bundle[40] = 18; bundle[36] = 3;
      }
      else if ( !Q_stricmp( "nv_dot_product_reflect_cube_map_eye_from_qs_1of3", token ) )
      {
        shaderParseSurfaceFlags |= 0xEu; bundle[40] = 19; bundle[36] = 4;
      }
      else if ( !Q_stricmp( "nv_dot_product_reflect_cube_map_eye_from_qs_2of3", token ) )
      {
        shaderParseSurfaceFlags |= 0xEu; bundle[40] = 20; bundle[36] = 4;
      }
      else if ( !Q_stricmp( "nv_dot_product_reflect_cube_map_eye_from_qs_3of3", token ) )
      {
        shaderParseSurfaceFlags |= 0xEu; bundle[40] = 21; bundle[36] = 4;
      }
      else if ( !Q_stricmp( "environment", token ) )
      {
        shaderParseSurfaceFlags |= 2u;   bundle[40] = 4;
      }
      else if ( !Q_stricmp( "lightmap", token ) )
      {
        shaderParseSurfaceFlags |= 0x20u; bundle[40] = 2;
      }
      else if ( !Q_stricmp( "texture", token ) || !Q_stricmp( "base", token ) )
      {
        shaderParseSurfaceFlags |= 0x10u; bundle[40] = 3;
      }
      else if ( !Q_stricmp( "vector", token ) )
      {
        ParseVector( 3, data_p, (int)&bundle[41] );
        ParseVector( 3, data_p, (int)&bundle[44] );
        bundle[40] = 6;
      }
      else
      {
        ri_Printf( 2, "WARNING: unknown or invalid texgen parm '%s' in shader '%s'\n",
                   token, shaderParseName );
      }
      continue;
    }

    if ( !Q_stricmp( "tcMod", token ) )
    {
      memset( tcModText, 0, sizeof( tcModText ) );
      for ( ;; )
      {
        token = Com_ParseOnLine( data_p );
        if ( !*token )
          break;
        strcat( tcModText, token );
        strcat( tcModText, " " );
      }
      if ( stageActive )
      {
        tcModCursor = tcModText;
        ParseTexMod( bundleIndex, (int)stage, tcModCursor );
      }
      continue;
    }

    if ( !Q_stricmp( "depthwrite", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'depthwrite' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      depthMaskBits      = 256;
      explicitDepthWrite = 1;
      continue;
    }

    if ( !Q_stricmp( "nextbundle", token ) )
    {
      int haveRequirement;

      if ( stageActive && !bundle[1] )
      {
        ri_Printf( 2, "WARNING: shader '%s' has bundle with no map\n", shaderParseName );
        return 0;
      }

      ++bundleIndex;
      if ( bundleIndex >= 2 )
      {
        haveRequirement = 0;
        for ( k = bundleIndex; k < 8; ++k )
        {
          if ( rendererShaderRequirements[k - 2] )
          {
            haveRequirement = 1;
            break;
          }
        }
        if ( !haveRequirement )
        {
          ri_Printf( 2,
            "WARNING: shader '%s' has %i or more bundles without suitable 'requires GL_MAX_TEXTURE_UNITS_ARB\n",
            shaderParseName, bundleIndex + 1 );
          return 0;
        }
      }

      bundle = &stage[50 * bundleIndex];
      bundle[36] = 2;
      bundle[37] = 8448;
      imageColorScale[0] = 1.0f;
      imageColorScale[1] = 1.0f;
      imageColorScale[2] = 1.0f;
      imageColorScale[3] = 1.0f;
      multiplyImage = 0;
      continue;
    }

    if ( !Q_stricmp( "requires", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'requires' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      if ( stage[1] )
      {
        ri_Printf( 2,
          "WARNING: 'requires' should be before textures are specified in shader '%s'\n",
          shaderParseName );
        return 0;
      }
      stageActive &= ParseStageRequirements( data_p, 0 );
      continue;
    }

    if ( !Q_stricmp( "texEnvCombine", token ) )
    {
      int ok;
      if ( !rendererShaderRequirements[8] )
      {
        ri_Printf( 2,
          "WARNING: shader '%s' uses texEnvCombine without 'requires GL_ARB_texture_env_combine'\n",
          shaderParseName );
        return 0;
      }
      parseInfo->spaceDelimited = qfalse;
      ok = ParseTextureEnvCombine( (int)&stage[50 * bundleIndex + 1], data_p );
      parseInfo->spaceDelimited = qtrue;
      if ( !ok )
        return 0;
      continue;
    }

    if ( !Q_stricmp( "vertexProgram", token ) )
    {
      if ( bundleIndex )
      {
        ri_Printf( 2, "WARNING: 'vertexProgram' not allowed in nextbundle in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      if ( !rendererShaderRequirements[10] )
      {
        ri_Printf( 2,
          "WARNING: shader '%s' uses vertexProgram without 'requires GL_ARB_vertex_program'\n",
          shaderParseName );
        return 0;
      }
      vertexProgramBits = 0x2000000;
      if ( !ParseVertexProgram( data_p, (int)stage ) )
        return 0;
      shaderParseSurfaceFlags |= 0x102u;
      *stage |= 0x20000u;
      continue;
    }

    if ( !Q_stricmp( "multiplyImage", token ) )
    {
      if ( bundle[1] )
      {
        ri_Printf( 2,
          "WARNING: shader '%s' uses 'multiplyImage' after the images are defined\n",
          shaderParseName );
        return 0;
      }

      for ( j = 0; j < 4; ++j )
      {
        token = Com_ParseOnLine( data_p );
        if ( !*token )
          break;
        if ( !_stricmp( token, "identityLighting" ) )
        {
          imageColorScale[j] = tr_identityLight;
        }
        else
        {
          char c = *token;
          if ( ( c < '0' || c > '9' ) && c != '.' && c != '+' )
          {
            ri_Printf( 2,
              "WARNING: argument '%s' to multiplyImage in shader '%s' should be 'identityLighting' or a number >= 0\n",
              token, shaderParseName );
            return 0;
          }
          imageColorScale[j] = (float)atof( token );
        }
      }

      if ( !j )
      {
        ri_Printf( 2, "WARNING: missing arguments to 'multiplyImage' in shader '%s'\n",
                   shaderParseName );
        return 0;
      }
      if ( j == 2 )
        imageColorScale[3] = imageColorScale[2];
      else if ( j != 4 )
        imageColorScale[3] = 1.0f;
      if ( j < 3 )
      {
        imageColorScale[1] = imageColorScale[0];
        imageColorScale[2] = imageColorScale[0];
      }
      multiplyImage = 1;
      continue;
    }

    ri_Printf( 2, "WARNING: unknown parameter '%s' in shader '%s'\n",
               token, shaderParseName );
    return 0;
  }
}


/* ---- ParseDeform  0x004F9D20 ----  [HIGH] */
void __cdecl ParseDeform(char **data_p)
{
  parseInfo_t *v3;
  int v4;
  float *v5;
  unsigned int v6;
  char *v7;
  char *v8;
  char *v9;
  char *v10;
  const char *v11;
  double v12;
  char *v13;
  const char *v14;
  char *v15;
  double v16;
  char *v17;
  char *v18;
  char *v19;
  double v20;
  int v21;
  float *i;
  char *v23;
  long double v24;
  const char *v26;
  const char *v27;
  const char *v28;
  const char *v30;
  const char *v31;
  const char *v32;
  int v33;
  int v34;
  int v35;
  int v36;

  v3 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v3->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v3 = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  if ( !v3->token[0] )
  {
    ri_Printf(2, "WARNING: missing deform parm in shader '%s'\n", shaderParseName);
    return;
  }
  v4 = shaderParseNumDeforms;
  if ( shaderParseNumDeforms == 3 )
  {
    ri_Printf(2, "WARNING: MAX_SHADER_DEFORMS in '%s'\n", shaderParseName);
    return;
  }
  v5 = (float *)((char *)&shaderParseDeforms + 52 * shaderParseNumDeforms);
  if ( !Q_stricmpn("projectionShadow", v3->token, 99999) )
  {
    *(_DWORD *)v5 = 8;
    ++shaderParseNumDeforms;
    return;
  }
  if ( !Q_stricmpn("autosprite", v3->token, 99999) )
  {
    *(_DWORD *)v5 = 9;
    ++shaderParseNumDeforms;
    return;
  }
  if ( !Q_stricmpn("autosprite2", v3->token, 99999) )
  {
    *(_DWORD *)v5 = 10;
    ++shaderParseNumDeforms;
    return;
  }
  if ( !Q_stricmpn("text", v3->token, 4) )
  {
    v6 = v3->token[4] - 48;
    if ( v6 >= 8 )
      v6 = 0;
    *(_DWORD *)v5 = v6 + 11;
    ++shaderParseNumDeforms;
    return;
  }
  if ( !Q_stricmp("bulge", v3->token) )
  {
    shaderParseSurfaceFlags |= 0x13u;
    v7 = Com_ParseOnLine(data_p);
    if ( *v7
      && (v5[10] = atof(v7), v8 = Com_ParseOnLine(data_p), *v8)
      && (v5[11] = atof(v8), v9 = Com_ParseOnLine(data_p), *v9) )
    {
      v5[12] = atof(v9);
      shaderParseBoundsExpansion = fabs(v5[11]) + shaderParseBoundsExpansion;
      *(_DWORD *)v5 = 6;
      ++shaderParseNumDeforms;
    }
    else
    {
      ri_Printf(2, "WARNING: missing deformVertexes bulge parm in shader '%s'\n", shaderParseName);
    }
    return;
  }
  if ( Q_stricmp("wave", v3->token) )
  {
    if ( Q_stricmp("flap", v3->token) )
    {
      if ( Q_stricmp("normal", v3->token) )
      {
        if ( !Q_stricmp("syncnormal", v3->token) )
        {
          shaderParseSurfaceFlags |= 3u;
          if ( v4 && *((_DWORD *)v5 - 13) == 1 )
          {
            qmemcpy(v5, v5 - 13, 0x34u);
            v19 = Com_ParseOnLine(data_p);
            if ( *v19 )
            {
              v20 = atof(v19) * v5[6];
              *(_DWORD *)v5 = 5;
              v5[6] = v20;
              ++shaderParseNumDeforms;
            }
            else
            {
              ri_Printf(2, "WARNING: missing scale to syncNormal in shader '%s'\n", shaderParseName);
            }
          }
          else
          {
            ri_Printf(
              2,
              "WARNING: deformVertexes syncNormal must follow deformVertexes wave in shader '%s'\n",
              shaderParseName);
          }
          return;
        }
        if ( Q_stricmp("move", v3->token) )
        {
          ri_Printf(2, "WARNING: unknown deformVertexes subtype '%s' found in shader '%s'\n", v3->token, shaderParseName);
          return;
        }
        shaderParseSurfaceFlags |= 1u;
        v21 = 0;
        for ( i = v5 + 1; ; ++i )
        {
          v23 = Com_ParseOnLine(data_p);
          if ( !*v23 )
            break;
          *i = atof(v23);
          if ( ++v21 >= 3 )
          {
            ParseWaveForm((int)(v5 + 4), data_p);
            v24 = sqrt(v5[1] * v5[1] + v5[2] * v5[2] + v5[3] * v5[3]);
            shaderParseBoundsExpansion = MaxWaveFormDeformation((int)(v5 + 4)) * v24 + shaderParseBoundsExpansion;
            *(_DWORD *)v5 = 7;
            ++shaderParseNumDeforms;
            return;
          }
        }
      }
      else
      {
        shaderParseSurfaceFlags |= 3u;
        v17 = Com_ParseOnLine(data_p);
        if ( *v17 )
        {
          v5[6] = atof(v17);
          v18 = Com_ParseOnLine(data_p);
          if ( *v18 )
          {
            v5[8] = atof(v18);
            *(_DWORD *)v5 = 4;
            ++shaderParseNumDeforms;
            return;
          }
        }
      }
LABEL_27:
      ri_Printf(2, "WARNING: missing deformVertexes parm in shader '%s'\n", shaderParseName);
      return;
    }
    shaderParseSurfaceFlags |= 0x13u;
    v13 = Com_ParseOnLine(data_p);
    v14 = v13;
    if ( !*v13 )
    {
      ri_Printf(2, "WARNING: missing flap axis in shader '%s'\n", shaderParseName);
      return;
    }
    if ( Q_stricmp("s", v13) && Q_stricmp("x", v14) )
    {
      if ( Q_stricmp("t", v14) && Q_stricmp("y", v14) )
      {
        ri_Printf(2, "WARNING: flap axis must be one of (s, t, x, y) in shader '%s'\n", shaderParseName);
        return;
      }
      *(_DWORD *)v5 = 3;
    }
    else
    {
      *(_DWORD *)v5 = 2;
    }
    v15 = Com_ParseOnLine(data_p);
    v16 = atof(v15);
    if ( (v16 == 0.0) | __UNORDERED__(v16, 0.0) )
    {
      v5[9] = 100.0;
      ri_Printf(2, "WARNING: illegal div value of 0 in deformVertexes command for shader '%s'\n", shaderParseName);
    }
    else
    {
      v5[9] = 1.0 / atof(v15);
    }
    ParseWaveForm((int)(v5 + 4), data_p);
    shaderParseBoundsExpansion = MaxWaveFormDeformation((int)(v5 + 4)) + shaderParseBoundsExpansion;
    ++shaderParseNumDeforms;
  }
  else
  {
    shaderParseSurfaceFlags |= 3u;
    v10 = Com_ParseOnLine(data_p);
    v11 = v10;
    if ( !*v10 )
      goto LABEL_27;
    v12 = atof(v10);
    if ( (v12 == 0.0) | __UNORDERED__(v12, 0.0) )
    {
      v5[9] = 100.0;
      ri_Printf(2, "WARNING: illegal div value of 0 in deformVertexes command for shader '%s'\n", shaderParseName);
    }
    else
    {
      v5[9] = 1.0 / atof(v11);
    }
    ParseWaveForm((int)(v5 + 4), data_p);
    shaderParseBoundsExpansion = MaxWaveFormDeformation((int)(v5 + 4)) + shaderParseBoundsExpansion;
    *(_DWORD *)v5 = 1;
    ++shaderParseNumDeforms;
  }
}

/* ---- ParseSkyParms  0x004FA320 ----  VERIFIED */
void __cdecl ParseSkyParms(char **data_p, int a2)
{
  parseInfo_t *v2;
  char **v3;
  char *v4;
  int i;
  int ImageFile;
  double v7;
  parseInfo_t *v8;
  int j;
  int v10;
  char v11[68]; // [esp+10h] [ebp-4Ch] BYREF
  unsigned int v12;
  unsigned int retaddr;

  v12 = retaddr ^ _security_cookie;
  v2 = parseInfo;
  v3 = data_p;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v2->spaceDelimited == qfalse )
    {
      v4 = (char *)v2;
      goto LABEL_6;
    }
    *data_p = v2->ungetTokenSave;
    v2->currentLine = v2->ungetLineSave;
  }
  v4 = Com_ParseExt(data_p, qfalse);
  v2 = parseInfo;
LABEL_6:
  if ( !*v4 )
  {
LABEL_25:
    ri_Printf(2, "WARNING: 'skyParms' missing parameter in shader '%s'\n", shaderParseName);
    return;
  }
  if ( strcmp(v4, "-") )
  {
    for ( i = 0; i < 6; ++i )
    {
      Com_sprintf(v11, 64, "%s_%s.tga", v4, skySuffix[i]);
      ImageFile = R_FindImageFile(v11, 3553, 51, a2, 0, 1.0);
      skyOuterBox[i] = ImageFile;
      if ( !ImageFile )
        skyOuterBox[i] = tr_defaultImage;
    }
    v2 = parseInfo;
    v3 = data_p;
  }
  if ( v2->ungetReady )
  {
    v2->ungetReady = qfalse;
    if ( v2->spaceDelimited == qfalse )
      goto LABEL_17;
    *v3 = v2->ungetTokenSave;
    v2->currentLine = v2->ungetLineSave;
  }
  v2 = (parseInfo_t *)Com_ParseExt(v3, qfalse);
LABEL_17:
  if ( !v2->token[0] )
    goto LABEL_25;
  v7 = atof(v2->token);
  shaderParseSkyCloudHeight = v7;
  if ( (v7 == 0.0) | __UNORDERED__(v7, 0.0) )
    shaderParseSkyCloudHeight = 512.0;
  R_InitSkyTexCoords(shaderParseSkyCloudHeight);
  v8 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( v8->spaceDelimited == qfalse )
      goto LABEL_24;
    *v3 = v8->ungetTokenSave;
    v8->currentLine = v8->ungetLineSave;
  }
  v8 = (parseInfo_t *)Com_ParseExt(v3, qfalse);
LABEL_24:
  if ( !v8->token[0] )
    goto LABEL_25;
  if ( strcmp(v8->token, "-") )
  {
    for ( j = 0; j < 6; ++j )
    {
      Com_sprintf(v11, 64, "%s_%s.tga", (char *)v8, skySuffix[j]);
      v10 = R_FindImageFile(v11, 3553, 51, a2, 0, 1.0);
      skyInnerBox[j] = v10;
      if ( !v10 )
        skyInnerBox[j] = tr_defaultImage;
    }
  }
  shaderParseFlags |= 8u;
}

/* ---- ParseSort  0x004FA580 ----  VERIFIED */
int ParseSort( char **data_p )                              /* EAX */
{
	char *token;

	token = Com_ParseOnLine( data_p );                  /* 0x004FA581 */
	if ( !token[0] ) {
		ri_Printf( 2, "WARNING: missing sort parameter in shader '%s'\n",
		           shaderParseName );
		return 0;
	}

	if ( !Q_stricmpn( token, "portal",     99999 ) ) { shaderParseSort = 0x3F800000; return 1; }
	if ( !Q_stricmpn( token, "sky",        99999 ) ) { shaderParseSort = 0x40000000; return 1; }
	if ( !Q_stricmpn( token, "ocean",      99999 ) ) { shaderParseSort = 0x40400000; return 1; }
	if ( !Q_stricmpn( token, "boathull",   99999 ) ) { shaderParseSort = 0x40800000; return 1; }
	if ( !Q_stricmp ( token, "opaque"          ) )   { shaderParseSort = 0x40A00000; return 1; }
	if ( !Q_stricmp ( token, "decal"           ) )   { shaderParseSort = 0x40C00000; return 1; }
	if ( !Q_stricmp ( token, "seeThrough"      ) )   { shaderParseSort = 0x40E00000; return 1; }
	if ( !Q_stricmp ( token, "banner"          ) )   { shaderParseSort = 0x41000000; return 1; }
	if ( !Q_stricmp ( token, "underwater"      ) )   { shaderParseSort = 0x41200000; return 1; }
	if ( !Q_stricmp ( token, "water"           ) )   { shaderParseSort = 0x41300000; return 1; }
	if ( !Q_stricmp ( token, "corona"          ) )   { shaderParseSort = 0x41400000; return 1; }
	if ( !Q_stricmp ( token, "innerBlend"      ) )   { shaderParseSort = 0x41500000; return 1; }
	if ( !Q_stricmp ( token, "outerBlend"      ) )   { shaderParseSort = 0x41600000; return 1; }
	if ( !Q_stricmp ( token, "blend"           ) )   { shaderParseSort = 0x41700000; return 1; }
	if ( !Q_stricmp ( token, "blend2"          ) )   { shaderParseSort = 0x41800000; return 1; }
	if ( !Q_stricmp ( token, "blend3"          ) )   { shaderParseSort = 0x41880000; return 1; }
	if ( !Q_stricmp ( token, "blend4"          ) )   { shaderParseSort = 0x41900000; return 1; }
	if ( !Q_stricmp ( token, "additive"        ) )   { shaderParseSort = 0x41980000; return 1; }
	if ( !Q_stricmp ( token, "nearest"         ) )   { shaderParseSort = 0x41B00000; return 1; }

	/* 0x004FA86B */
	*(float *)&shaderParseSort = (float)atof( token );
	if ( *(float *)&shaderParseSort != 0.0f ) {
		return 1;
	}
	if ( token[0] == '0' ) {
		return 1;
	}
	ri_Printf( 2, "WARNING: bad shader sort '%s'\n", token );
	return 0;
}

typedef struct {
    const char *name;
    int         clearSolid;
    int         surfaceFlags;
    int         contents;
} tr_infoParm_t;
#define tr_surfaceParms   ((const tr_infoParm_t *)(const void *)&surfacetype_bark)

/* === Merged from renderer/tr_shader_rdata.c (surfaceParm table) (retail linked it as tr_*.obj .rdata/.data). === */

/* ==========================================================================
 * surfacetype_bark        0x005717A0        848 bytes (53 x 16)
 * ==========================================================================
 *
 * The `surfaceparm` keyword table.  ParseSurfaceParm (0x004FA8B0) walks it:
 *
 *   0x004FA903  mov esi, offset surfacetype_bark   -- walk cursor
 *   0x004FA920  mov ecx, [esi+10h]                 -- next entry's NAME
 *   0x004FA923  add esi, 10h                       -- stride is 16
 *   0x004FA927  test ecx,ecx / jnz                 -- NULL name terminates
 *   0x004FA935  shl ebx, 4                         -- index * 16
 *   0x004FA938  mov eax, dword_5717A8[ebx]         -- base+8 of entry ebx
 *   0x004FA93E  or  ecx, eax                       -- into shaderParseSurfaceParmFlags
 *
 * So: stride 16, name at +0, and the field the parser consumes is at +8 --
 * RTCW's infoParm_t { name, clearSolid, surfaceFlags, contents }.
 * CL_SurfaceTypeFromName (client_mp/cl_cgame_mp.c) and the brushmask
 * formatter in renderer/tr_cmds.c walk the same table.
 *
 * RTCW's ParseSurfaceParm applies all three payload fields:
 *
 *     if ( infoParms[i].clearSolid ) shader.contentFlags &= ~CONTENTS_SOLID;
 *     shader.surfaceFlags |= infoParms[i].surfaceFlags;
 *     shader.contentFlags |= infoParms[i].contents;
 *
 * CoD1 0x004FA92F-0x004FA946 applies ONLY the +8 word, into a single global.
 * `clearSolid` and `contents` have no reader in the renderer -- they are what
 * the BSP compiler and the collision side of the engine consume.
 *
 * ENTRIES 0..22 are the CoD surface types: the +8 word is (type << 20), so
 * bark is 1 and asphalt is 22, matching CL_SurfaceTypeFromName's
 * `(surfaceFlags >> 20) & 0x1F`.  Entry 22 "opaqueglass" reuses glass's type 9
 * -- the binary holds 0x00900000 twice.  Entries 23..51 are the flag
 * keywords.  Entry 52 is the NULL terminator the walk tests for at 0x004FA927.
 *
 * SIZE.  0x00571900 also carries the name `surfacetype_Opaqueglass`, which
 * is why cod1_globals.h sizes surfacetype_bark at 352 bytes and declares the
 * +8 column a second time as `int dword_5717A8[86]`; both of those
 * declarations overlap this object in retail and cannot in C.  The true
 * extent is 0x005717A0..0x00571AEF inclusive: the sixteen bytes at 0x00571AE0
 * are the terminator, and 0x00571AF0 holds 0x00000080, unrelated .data.
 */

tr_infoParm_t surfacetype_bark[53] = {
    { "bark",          0, 0x00100000, 0x00000000 },   /* 0x005717A0 */
    { "brick",         0, 0x00200000, 0x00000000 },   /* 0x005717B0 */
    { "carpet",        0, 0x00300000, 0x00000000 },   /* 0x005717C0 */
    { "cloth",         0, 0x00400000, 0x00000000 },   /* 0x005717D0 */
    { "concrete",      0, 0x00500000, 0x00000000 },   /* 0x005717E0 */
    { "dirt",          0, 0x00600000, 0x00000000 },   /* 0x005717F0 */
    { "flesh",         0, 0x00700000, 0x00000000 },   /* 0x00571800 */
    { "foliage",       1, 0x00800000, 0x00000002 },   /* 0x00571810 */
    { "glass",         1, 0x00900000, 0x00000010 },   /* 0x00571820 */
    { "grass",         0, 0x00A00000, 0x00000000 },   /* 0x00571830 */
    { "gravel",        0, 0x00B00000, 0x00000000 },   /* 0x00571840 */
    { "ice",           0, 0x00C00000, 0x00000000 },   /* 0x00571850 */
    { "metal",         0, 0x00D00000, 0x00000000 },   /* 0x00571860 */
    { "mud",           0, 0x00E00000, 0x00000000 },   /* 0x00571870 */
    { "paper",         0, 0x00F00000, 0x00000000 },   /* 0x00571880 */
    { "plaster",       0, 0x01000000, 0x00000000 },   /* 0x00571890 */
    { "rock",          0, 0x01100000, 0x00000000 },   /* 0x005718A0 */
    { "sand",          0, 0x01200000, 0x00000000 },   /* 0x005718B0 */
    { "snow",          0, 0x01300000, 0x00000000 },   /* 0x005718C0 */
    { "water",         1, 0x01400000, 0x00000020 },   /* 0x005718D0 */
    { "wood",          0, 0x01500000, 0x00000000 },   /* 0x005718E0 */
    { "asphalt",       0, 0x01600000, 0x00000000 },   /* 0x005718F0 */
    { "opaqueglass",   0, 0x00900000, 0x00000000 },   /* 0x00571900 */
    { "clipmissile",   1, 0x00000000, 0x00000080 },   /* 0x00571910 */
    { "ai_nosight",    1, 0x00000000, 0x00001000 },   /* 0x00571920 */
    { "clipshot",      1, 0x00000000, 0x00002000 },   /* 0x00571930 */
    { "playerclip",    1, 0x00000000, 0x00010000 },   /* 0x00571940 */
    { "monsterclip",   1, 0x00000000, 0x00020000 },   /* 0x00571950 */
    { "vehicleclip",   1, 0x00000000, 0x00000200 },   /* 0x00571960 */
    { "itemclip",      1, 0x00000000, 0x00000400 },   /* 0x00571970 */
    { "nodrop",        1, 0x00000000, (int)0x80000000 }, /* 0x00571980 */
    { "nonsolid",      1, 0x00004000, 0x00000000 },   /* 0x00571990 */
    { "origin",        1, 0x00000000, 0x01000000 },   /* 0x005719A0 */
    { "detail",        0, 0x00000000, 0x08000000 },   /* 0x005719B0 */
    { "structural",    0, 0x00000000, 0x10000000 },   /* 0x005719C0 */
    { "portal",        1, (int)0x80000000, 0x00000000 }, /* 0x005719D0 */
    { "canshootclip",  0, 0x00000000, 0x00000040 },   /* 0x005719E0 */
    { "sky",           0, 0x00000004, 0x00000800 },   /* 0x005719F0 */
    { "lightfilter",   0, 0x00008000, 0x00000000 },   /* 0x00571A00 */
    { "alphashadow",   0, 0x00010000, 0x00000000 },   /* 0x00571A10 */
    { "castshadow",    0, 0x00040000, 0x00000000 },   /* 0x00571A20 */
    { "hint",          0, 0x00000100, 0x00000000 },   /* 0x00571A30 */
    { "slick",         0, 0x00000002, 0x00000000 },   /* 0x00571A40 */
    { "noimpact",      0, 0x00000010, 0x00000000 },   /* 0x00571A50 */
    { "nomarks",       0, 0x00000020, 0x00000000 },   /* 0x00571A60 */
    { "ladder",        0, 0x00000008, 0x00000000 },   /* 0x00571A70 */
    { "nodamage",      0, 0x00000001, 0x00000000 },   /* 0x00571A80 */
    { "nosteps",       0, 0x00002000, 0x00000000 },   /* 0x00571A90 */
    { "nodraw",        0, 0x00000080, 0x00000000 },   /* 0x00571AA0 */
    { "pointlight",    0, 0x00000800, 0x00000000 },   /* 0x00571AB0 */
    { "nolightmap",    0, 0x00000400, 0x00000000 },   /* 0x00571AC0 */
    { "nodlight",      0, 0x00020000, 0x00000000 },   /* 0x00571AD0 */
    { 0,               0, 0x00000000, 0x00000000 }    /* 0x00571AE0  terminator */
};

/* ---- ParseSurfaceParm  0x004FA8B0 ----  VERIFIED */
parseInfo_t *__cdecl ParseSurfaceParm(char **data_p)
{
  parseInfo_t *result;
  const char *v3;
  int v4;
  parseInfo_t *v5;
  const tr_infoParm_t *i;

  result = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    if ( result->spaceDelimited == qfalse )
      goto LABEL_5;
    *data_p = result->ungetTokenSave;
    result->currentLine = result->ungetLineSave;
  }
  result = (parseInfo_t *)Com_ParseExt(data_p, qfalse);
LABEL_5:
  v3 = tr_surfaceParms[0].name;
  v4 = 0;
  v5 = result;
  if ( v3 )
  {
    for ( i = tr_surfaceParms; ; ++i )
    {
      if ( v5 )
      {
        if ( v3 )
        {
          result = (parseInfo_t *)Q_stricmpn(v3, (const char *)v5, 99999);
          if ( !result )
            break;
        }
      }
      v3 = i[1].name;
      ++v4;
      if ( !v3 )
        return result;
    }
    result = (parseInfo_t *)tr_surfaceParms[v4].surfaceFlags;
    shaderParseSurfaceParmFlags |= (unsigned int)result;
  }
  return result;
}

/* ---- ParseShader  0x004FA950 ----  [HIGH] */
int __cdecl ParseShader(char **data_p, int a2, int a3)
{
  parseInfo_t *v3;
  char *v4;
  int *v5;
  parseInfo_t *v6;
  char *v7;
  char v8;
  int v9;
  int *v10;
  _BYTE *v11;
  int v12;
  char *v13;
  _DWORD *v14;
  int v15;
  int v16;
  char *v17;
  char *v18;
  char *v19;
  char *v20;
  double v21;
  char *v22;
  char *v23;
  long double v24;
  long double v25;
  char *v26;
  char *v27;
  int v28;
  char *v29;
  const char *v30;
  char *v31;
  const char *v32;
  const char *v34;
  const char *v35;
  int v36;
  int *v37;
  int v38;
  float v39;
  char *v40;
  int v41; // [esp+24h] [ebp-F14h] BYREF
  int v42[418]; // [esp+28h] [ebp-F10h] BYREF
  _BYTE v43[2180]; // [esp+6B0h] [ebp-888h] BYREF
  unsigned int v44;
  unsigned int retaddr;

  v44 = retaddr ^ _security_cookie;
  v3 = parseInfo;
  v38 = 0;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    *data_p = v3->ungetTokenSave;
    v3->currentLine = v3->ungetLineSave;
  }
  v4 = Com_ParseExt(data_p, qtrue);
  if ( *v4 != 123 )
  {
    ri_Printf(2, "WARNING: expecting '{', found '%s' instead in shader '%s'\n", v4, shaderParseName);
    return 0;
  }
  v5 = shaderParseStageStorage;
  v40 = (char *)&shaderParseTexMods;
LABEL_6:
  v37 = v5;
  while ( 1 )
  {
    v6 = parseInfo;
    if ( parseInfo->ungetReady )
    {
      parseInfo->ungetReady = qfalse;
      *data_p = v6->ungetTokenSave;
      v6->currentLine = v6->ungetLineSave;
    }
    v7 = Com_ParseExt(data_p, qtrue);
    v8 = *v7;
    if ( !*v7 )
    {
      ri_Printf(2, "WARNING: no concluding '}' in shader %s\n", shaderParseName);
      return 0;
    }
    if ( v8 == 125 )
      break;
    if ( v8 == 123 )
    {
      v9 = shaderParseFlags;
      memset(v42, 0, sizeof(v42));
      v10 = &v42[48];
      v11 = v43;
      v12 = 8;
      do
      {
        *v10 = (int)v11;
        v11 += 272;
        v10 += 50;
        --v12;
      }
      while ( v12 );
      if ( !ParseStage(v42, data_p, a2, a3, &v41) )
        return 0;
      if ( v41 )
      {
        v13 = v40;
        qmemcpy(v40, v43, 0x880u);
        qmemcpy(v5, v42, 0x688u);
        v14 = v5 + 48;
        v15 = 8;
        do
        {
          *v14 = v13;
          v13 += 272;
          v14 += 50;
          --v15;
        }
        while ( v15 );
        *v5 |= 1u;
        ++v38;
        v40 = v13;
        v5 += 418;
        goto LABEL_6;
      }
      shaderParseFlags = v9;
    }
    else if ( Q_stricmpn("qer", v7, 3) && Q_stricmpn("radialNormals", v7, 99999) )
    {
      if ( Q_stricmpn("q3map_sun", v7, 99999) )
      {
        if ( Q_stricmpn("deformVertexes", v7, 99999) )
        {
          if ( !Q_stricmpn("tesssize", v7, 99999) )
            goto LABEL_32;
          if ( Q_stricmp("clampTime", v7) )
          {
            if ( !Q_stricmpn("q3map", v7, 5) )
              goto LABEL_32;
            if ( Q_stricmp("surfaceParm", v7) )
            {
              if ( Q_stricmp("nomipmaps", v7) )
              {
                if ( Q_stricmp("nopicmip", v7) )
                {
                  if ( Q_stricmp("picmip2", v7) )
                  {
                    if ( Q_stricmp("polygonOffset", v7) )
                    {
                      if ( Q_stricmp("polygonOffset2", v7) )
                      {
                        if ( Q_stricmp("polygonOffsetConst", v7) )
                        {
                          if ( Q_stricmp("entityMergable", v7) )
                          {
                            if ( Q_stricmp("fogParms", v7) )
                            {
                              if ( Q_stricmp("portal", v7) )
                              {
                                if ( Q_stricmp("skyparms", v7) )
                                {
                                  if ( Q_stricmp("sunfile", v7) )
                                  {
                                    if ( Q_stricmp("nofog", v7) )
                                    {
                                      if ( Q_stricmp("light", v7) )
                                      {
                                        if ( Q_stricmp("cull", v7) )
                                        {
                                          if ( Q_stricmp("sort", v7) )
                                          {
                                            ri_Printf(
                                              2,
                                              "WARNING: unknown general shader parameter '%s' in '%s'\n",
                                              v7,
                                              shaderParseName);
                                            return 0;
                                          }
                                          if ( !ParseSort(data_p) )
                                            return 0;
                                        }
                                        else
                                        {
                                          v31 = Com_ParseOnLine(data_p);
                                          v32 = v31;
                                          if ( *v31 )
                                          {
                                            if ( Q_stricmp("none", v31)
                                              && Q_stricmp("twosided", v32)
                                              && Q_stricmp("disable", v32) )
                                            {
                                              if ( Q_stricmp("back", v32)
                                                && Q_stricmp("backside", v32)
                                                && Q_stricmp("backsided", v32) )
                                              {
                                                ri_Printf(
                                                  2,
                                                  "WARNING: invalid cull parm '%s' in shader '%s'\n",
                                                  v32,
                                                  shaderParseName);
                                              }
                                              else
                                              {
                                                shaderParseCullType = 1;
                                              }
                                            }
                                            else
                                            {
                                              shaderParseCullType = 2;
                                            }
                                          }
                                          else
                                          {
                                            ri_Printf(2, "WARNING: missing cull parms in shader '%s'\n", shaderParseName);
                                          }
                                        }
                                      }
                                      else
                                      {
                                        Com_ParseOnLine(data_p);
                                      }
                                    }
                                    else
                                    {
                                      shaderParseFlags |= 0x400u;
                                    }
                                  }
                                  else
                                  {
                                    v29 = Com_ParseOnLine(data_p);
                                    v30 = v29;
                                    if ( *v29 )
                                    {
                                      byte_16C4E5F = 0;
                                      strncpy(&tr_sunName, v29, 0x40u);
                                      if ( byte_16C4E5F )
                                      {
                                        ri_Printf(2, "WARNING: name '%s' too long for sunfile\n", v30);
                                        tr_sunName = 0;
                                      }
                                    }
                                    else
                                    {
                                      ri_Printf(2, "WARNING: missing sun name for 'sunfile'\n");
                                    }
                                  }
                                }
                                else
                                {
                                  ParseSkyParms(data_p, a3);
                                }
                              }
                              else
                              {
                                shaderParseSort = 1065353216;
                                shaderParseSurfaceFlags |= 2u;
                              }
                            }
                            else
                            {
                              if ( !ParseVector(3, data_p, (int)&shaderParseFogColor) )
                                return 0;
                              v27 = Com_ParseOnLine(data_p);
                              if ( *v27 )
                              {
                                shaderParseFogDepthForOpaque = atof(v27);
                                Com_SkipRestOfLine(0, data_p);
                              }
                              else
                              {
                                ri_Printf(
                                  2,
                                  "WARNING: missing parm for 'fogParms' keyword in shader '%s'\n",
                                  shaderParseName);
                              }
                              v5 = v37;
                            }
                          }
                          else
                          {
                            shaderParseFlags |= 4u;
                          }
                        }
                        else
                        {
                          shaderParseFlags = shaderParseFlags & 0xFFFFFF8F | 0x40;
                        }
                      }
                      else
                      {
                        shaderParseFlags = shaderParseFlags & 0xFFFFFF8F | 0x20;
                      }
                    }
                    else
                    {
                      shaderParseFlags = shaderParseFlags & 0xFFFFFF8F | 0x10;
                    }
                  }
                  else
                  {
                    shaderParseFlags |= 0x200u;
                  }
                }
                else
                {
                  shaderParseFlags |= 0x100u;
                }
              }
              else
              {
                shaderParseFlags |= 0x80u;
              }
            }
            else
            {
              ParseSurfaceParm(data_p);
            }
          }
          else
          {
            v26 = Com_ParseOnLine(data_p);
            if ( *v26 )
              shaderParseClampTime = atof(v26);
          }
        }
        else
        {
          ParseDeform(data_p);
        }
      }
      else
      {
        v17 = Com_ParseOnLine(data_p);
        flt_16C57C4 = atof(v17);
        v18 = Com_ParseOnLine(data_p);
        flt_16C57C8 = atof(v18);
        v19 = Com_ParseOnLine(data_p);
        flt_16C57CC = atof(v19);
        VectorNormalize(&tr_sunLight);
        v20 = Com_ParseOnLine(data_p);
        v21 = atof(v20);
        flt_16C57C4 = flt_16C57C4 * v21;
        flt_16C57C8 = flt_16C57C8 * v21;
        flt_16C57CC = flt_16C57CC * v21;
        v22 = Com_ParseOnLine(data_p);
        v39 = atof(v22) * 0.017453292;
        v23 = Com_ParseOnLine(data_p);
        v24 = atof(v23) * 0.017453292;
        v25 = cos(v24);
        *(float *)&tr_sunDirection = cos(v39) * v25;
        *(float *)&dword_16C57D4 = sin(v39) * v25;
        *(float *)&dword_16C57D8 = sin(v24);
      }
    }
    else
    {
LABEL_32:
      Com_SkipRestOfLine(0, data_p);
    }
  }
  if ( v38 || (shaderParseFlags & 8) != 0 )
  {
    shaderParseFlags |= 2u;
    return 1;
  }
  return 0;
}

/* ---- nullsub_91  0x004FB0ED ----  VERIFIED */
void nullsub_91()
{
  ;
}

/* ---- ComputeHardwareNeeds  0x004FB0F0 ----  [HIGH] */
void ComputeHardwareNeeds()
{
  int v0;
  int v1;
  int *v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  unsigned int v8;
  char v9;
  int v10;
  int *v11;
  int v12;
  int v13;
  int *v14;
  int v15;
  _DWORD *v16;
  int v17;
  int v18;
  _DWORD *v19;
  int v20;
  int v21;

  v0 = shaderParseSurfaceFlags;
  v1 = 0;
  shaderParseLightingFlags = 0;
  v2 = &dword_11E6F84;
  while ( 2 )
  {
    v3 = *(v2 - 409);
    if ( (v3 & 1) != 0 )
    {
      v4 = *v2;
      v5 = v3 | 0x40000;
      v6 = *v2 - 1;
      *(v2 - 409) = v5;
      switch ( v6 )
      {
        case 0:
        case 10:
        case 11:
          v0 |= 0x20000u;
          goto LABEL_11;
        case 1:
          goto LABEL_12;
        case 4:
        case 5:
        case 6:
          v0 |= 0x20000u;
          goto LABEL_9;
        case 8:
          v0 |= 0x102u;
          v7 = v5 | 0x20008;
          goto LABEL_10;
        case 9:
          v0 |= 0x102u;
          v7 = v5 | 0x20010;
          goto LABEL_10;
        default:
          v0 |= 0x10020000u;
LABEL_9:
          v7 = v5 | 0x10000;
LABEL_10:
          *(v2 - 409) = v7;
LABEL_11:
          shaderParseSurfaceFlags = v0;
LABEL_12:
          v8 = v2[6];
          switch ( v8 )
          {
            case 0u:
            case 1u:
              goto LABEL_18;
            case 4u:
            case 5u:
              v0 |= 0x20000u;
              goto LABEL_16;
            case 9u:
              v0 |= 0x20000u;
              goto LABEL_17;
            default:
              v0 |= 0x10020000u;
LABEL_16:
              *(v2 - 409) |= 0x10000u;
LABEL_17:
              shaderParseSurfaceFlags = v0;
LABEL_18:
              if ( (*(v2 - 409) & 0x10000) == 0 )
              {
                if ( v4 == 1 )
                {
                  v9 = tr_identityLightByte;
                  *((_BYTE *)v2 + 28) = tr_identityLightByte;
                  *((_BYTE *)v2 + 29) = v9;
                  *((_BYTE *)v2 + 30) = v9;
                }
                else if ( v4 == 2 )
                {
                  *((_BYTE *)v2 + 28) = -1;
                  *((_BYTE *)v2 + 29) = -1;
                  *((_BYTE *)v2 + 30) = -1;
                }
                if ( v8 <= 1 )
                  *((_BYTE *)v2 + 31) = -1;
              }
              v10 = 0;
              v11 = v2 - 372;
              do
              {
                if ( !*v11 )
                  break;
                v11 += 50;
                v0 |= 512 << v10;
                v12 = (256 << v10++) | *(v2 - 409);
                *(v2 - 409) = v12;
              }
              while ( v10 < 8 );
              v13 = 0;
              shaderParseSurfaceFlags = v0;
              v21 = 0;
              if ( v10 > 0 )
              {
                v14 = v2 - 376;
                do
                {
                  if ( *v14 <= 1 )
                  {
                    *v14 = 0;
                  }
                  else
                  {
                    v0 |= 0x80001u;
                    shaderParseSurfaceFlags = v0;
                  }
                  v15 = v14[7];
                  if ( v15 <= 0 || v15 > 3 )
                  {
                    v0 |= 0x100000 << v13;
                    shaderParseSurfaceFlags = v0;
                  }
                  if ( v14[14] > 0 )
                  {
                    v16 = (_DWORD *)v14[15];
                    v20 = v14[14];
                    do
                    {
                      switch ( *v16 )
                      {
                        case 2:
                        case 3:
                        case 5:
                        case 6:
                        case 7:
                          v17 = 0x100000 << v13;
                          v13 = v21;
                          v0 |= v17;
                          shaderParseSurfaceFlags = v0;
                          break;
                        default:
                          break;
                      }
                      v16 += 17;
                      --v20;
                    }
                    while ( v20 );
                  }
                  ++v13;
                  v14 += 50;
                  v21 = v13;
                }
                while ( v13 < v10 );
                v1 = shaderParseLightingFlags;
              }
              v1 |= *(v2 - 409);
              v2 += 418;
              shaderParseLightingFlags = v1;
              if ( v2 >= (_DWORD *)((char *)&dword_11E6F84 + 8 * 1672) )  /* retail 0x011EA3C4 == start + 8*1672 */
                goto LABEL_44;
              continue;
          }
      }
    }
    break;
  }
LABEL_44:
  if ( (v1 & 0x18) != 0 && (((v1 & 0x18) - 1) & v1 & 0x18) != 0 )
  {
    ri_Printf(
      0,
      "WARNING: shader '%s' uses more than one of rgbGen lightingAmbient, lightingDiffuse, and lightingSpecular\n",
      shaderParseName);
    v0 = shaderParseSurfaceFlags;
  }
  v18 = shaderParseNumDeforms;
  if ( shaderParseNumDeforms > 0 )
  {
    v19 = &shaderParseDeforms;
    do
    {
      if ( *v19 == 4 )
        v0 |= 0x40000u;
      else
        v0 |= 0x20000000u;
      v19 += 13;
      --v18;
      shaderParseSurfaceFlags = v0;
    }
    while ( v18 );
  }
}

/* ---- CreateDlightStage  0x004FB3C0 ----  [HIGH] */
void CreateDlightStage()
{
  int v0;
  int v1;

  if ( shaderParseNumUnfoggedPasses == 1 )
  {
    if ( dword_11E6924[0] )
    {
      if ( dword_11E69EC )
      {
        if ( !dword_11E6AB4 && !byte_11E69E8 )
        {
          if ( byte_11E6AB0 )
          {
            if ( dword_11E69B4[0] == 8448 && dword_11E6A7C == 8448 )
            {
              switch ( (char)dword_11E6FA4 )
              {
                case 0:
                case 18:
                case 34:
                case 37:
                case 101:
                  qmemcpy(&dword_11E6FA8, shaderParseStageStorage, 0x688u);
                  LOBYTE(dword_11E762C) = 0;
                  dword_11E7074 = tr_dlightImage;
                  if ( (dword_11E6FA4 & 0xF) == 5 )
                    v0 = dword_11E762C | 0x25;
                  else
                    v0 = dword_11E762C | 0x22;
                  dword_11E762C = v0;
                  v1 = dword_11E6FA8 | 0x80 | shaderParseLightingFlags;
                  dword_11E6FA8 |= 0x80u;
                  shaderParseLightingFlags = v1;
                  shaderParseNumUnfoggedPasses = 2;
                  break;
                default:
                  return;
              }
            }
          }
        }
      }
    }
  }
}

/* ---- CollapseMultitexture  0x004FB530 ----  VERIFIED */
int CollapseMultitexture()
{
  unsigned __int8 v0;
  unsigned __int8 v1;
  int v2;
  int *v3;
  int v4;
  int v5;
  int v6;
  unsigned int v7;
  _DWORD *v8;
  int v9;
  int v10;
  bool v11; // zf
  int *v12;
  int *v13;
  int v14;
  _DWORD v16[2]; // [esp+10h] [ebp-D0h] BYREF
  _BYTE v17[200]; // [esp+18h] [ebp-C8h] BYREF

  if ( !qglActiveTextureARB )
    return 0;
  if ( (shaderParseStageStorage[0] & 1) == 0 )
    return 0;
  if ( (dword_11E6FA8 & 1) == 0 )
    return 0;
  v0 = dword_11E762C;
  v1 = dword_11E6FA4;
  if ( ((dword_11E6FA4 ^ dword_11E762C) & 0xFFFFFE00) != 0 )
    return 0;
  v2 = 0;
  v3 = v16;
  do
  {
    v4 = dword_11E69B4[v2];
    *v3 = 0;
    if ( v4 )
    {
      v5 = *v3;
      do
        ++v5;
      while ( dword_11E69B4[50 * v5 + v2] );
      *v3 = v5;
    }
    v2 += 418;
    ++v3;
  }
  while ( v2 < 836 );
  if ( v16[1] != 1 )
    return 0;
  v6 = v16[0];
  if ( v16[0] + 1 > Value )
    return 0;
  v7 = 0;
  do
  {
    if ( v1 == (unsigned __int8)tr_collapseRules[v7].srcStateBits
      && v0 == (unsigned __int8)tr_collapseRules[v7].dstStateBits )
      break;
    ++v7;
  }
  while ( v7 < 9 );
  if ( v7 == 9 )
    return 0;
  v9 = 4 * v7;
  v10 = tr_collapseRules[v7].texEnv;
  v11 = v10 == 260;
  if ( v10 == 260 )
  {
    if ( !glConfig_textureEnvAddAvailable )
      return 0;
    v11 = 1;
  }
  if ( v11 && dword_11E6F84 != 2
    || (dword_11E6F84 != dword_11E760C || dword_11E6F9C != dword_11E7624)
    && (v10 != 8448 || dword_11E760C != 2 || (unsigned int)dword_11E7624 > 1) )
  {
    return 0;
  }
  if ( dword_11E6F84 == 8 )
  {
    if ( !memcmp(&unk_11E6F70, &unk_11E75F8, 0x14u) )
    {
      v6 = v16[0];
      goto LABEL_31;
    }
    return 0;
  }
LABEL_31:
  if ( dword_11E6F9C == 7 && memcmp(&unk_11E6F88, &unk_11E7610, 0x14u) )
    return 0;
  if ( v6 == 1 && byte_11E69E8 )
  {
    qmemcpy(v17, dword_11E6924, sizeof(v17));
    qmemcpy(dword_11E6924, &dword_11E6FAC, 0xC8u);
    v12 = (int *)v17;
    v13 = &dword_11E69EC;
  }
  else
  {
    v13 = &dword_11E6924[50 * v6];
    v12 = &dword_11E6FAC;
  }
  qmemcpy(v13, v12, 0xC8u);
  dword_11E69B4[50 * v6] = v10;
  v14 = tr_collapseRules[v7].collapsedStateBits;
  LOBYTE(dword_11E6FA4) = 0;
  dword_11E6FA4 |= v14;
  /* Removing stage 1 shifts the remaining stages within the same buffer. */
  memmove(&dword_11E6FA8, &unk_11E7630, 0x2730u);
  memset(&unk_11E96D8, 0, 0x688u);
  return 1;
}

/* ---- SortNewShader  0x004FB790 ----  [HIGH] */
void SortNewShader()
{
  int v0;
  int v1;
  unsigned int v2;
  unsigned int v3;
  int v4;
  int v5;
  int i;
  int v7;
  int v8;
  int v9;
  unsigned int v10;
  int v11;
  float v12;

  v0 = tr_numShaders[tr_numShaders[0]];
  v12 = *(float *)(v0 + 88);
  v1 = *(_DWORD *)(v0 + 340);
  v2 = 0;
  v3 = 0;
  if ( v1 )
  {
    v4 = *(_DWORD *)(v1 + 4);
    if ( v4 )
      v2 = *(_DWORD *)(v4 + 88);
    v5 = *(_DWORD *)(v1 + 204);
    if ( v5 )
      v3 = *(_DWORD *)(v5 + 88);
  }
  for ( i = tr_numShaders[0] - 2; i >= 0; ++*(_DWORD *)(v7 + 72) )
  {
    v7 = dword_16CF84C[i];
    if ( (*(float *)(v7 + 88) < (double)v12) | __UNORDERED__(*(float *)(v7 + 88), v12) )
      break;
    if ( (*(float *)(v7 + 88) == v12) | __UNORDERED__(*(float *)(v7 + 88), v12) )
    {
      v8 = *(_DWORD *)(v7 + 340);
      if ( v8 )
      {
        v9 = *(_DWORD *)(v8 + 4);
        if ( !v9 )
          goto LABEL_14;
        v10 = *(_DWORD *)(v9 + 88);
        if ( v10 < v2 )
          break;
        if ( v10 == v2 )
        {
LABEL_14:
          v11 = *(_DWORD *)(v8 + 204);
          if ( v11 )
          {
            if ( *(_DWORD *)(v11 + 88) <= v3 )
              break;
          }
        }
      }
    }
    dword_16CF850[i--] = v7;
  }
  *(_DWORD *)(v0 + 72) = i + 1;
  dword_16CF850[i] = v0;
}

/* ---- GeneratePermanentShader  0x004FB850 ----  VERIFIED */
char *GeneratePermanentShader()
{
  _DWORD *v1;
  int *v2;
  _DWORD *v3;
  int *v4;
  int v5;
  int v6;
  unsigned int v7;
  bool v8; // cc
  int v9;
  int HashValue;
  int *v11;
  const void **v12;
  _DWORD *v13;
  int v14;

  if ( tr_numShaders[0] == 4096 )
  {
    ri_Printf(2, "WARNING: GeneratePermanentShader - MAX_SHADERS hit\n");
    UpdateDelayLoadImagesForShader(shaderParseName, 1);
    return tr_defaultShader;
  }
  else
  {
    v1 = (_DWORD *)ri_Hunk_Alloc(408);
    qmemcpy(v1, shaderParseName, 0x198u);
    tr_shaders[tr_numShaders[0]] = (int)v1;
    v1[17] = tr_numShaders[0];
    dword_16CF84C[tr_numShaders[0]] = (int)v1;
    v1[18] = tr_numShaders[0]++;
    v13 = v1;
    v14 = 0;
    if ( (int)v1[84] > 0 )
    {
      v2 = shaderParseStageStorage;
      v11 = shaderParseStageStorage;
      v3 = v1 + 85;
      while ( (*(_BYTE *)v2 & 1) != 0 )
      {
        v4 = (int *)ri_Hunk_Alloc(1672);
        *v3 = v4;
        qmemcpy(v4, v2, 0x688u);
        v5 = 0;
        v12 = (const void **)(v11 + 48);
        do
        {
          v6 = *(_DWORD *)(*v3 + v5 + 188);
          if ( v6 )
          {
            v7 = 68 * v6;
            *(_DWORD *)(*v3 + v5 + 192) = ri_Hunk_Alloc(68 * v6);
            qmemcpy(*(void **)(*v3 + v5 + 192), *v12, v7);
          }
          else
          {
            *(_DWORD *)(*v3 + v5 + 192) = 0;
          }
          v5 += 200;
          v12 += 50;
        }
        while ( v5 < 1600 );
        ++v3;
        v8 = ++v14 < v13[84];
        v11 += 418;
        v1 = v13;
        if ( !v8 )
          goto LABEL_15;
        v2 = v11;
      }
      v1[v14 + 85] = 0;
    }
LABEL_15:
    v9 = MergableShader(v1);
    UpdateDelayLoadImagesForShader(v1, v9 == 0);
    SortNewShader();
    HashValue = tr_shader_generateHashValue((char *)v1);
    v1[101] = shaderHashTable[HashValue];
    shaderHashTable[HashValue] = (int)v1;
    return (char *)v1;
  }
}

/* ---- FinishShader  0x004FBA30 ----  [HIGH] */
char *FinishShader()
{
  int v0;
  _DWORD *v1;
  int *v2;
  int *v3;
  int v4;
  _BYTE *v5;
  _DWORD *v6;
  int v7;
  _DWORD *v8;
  int v9;
  char v10;
  int v11;
  int v12;
  int v13;
  int v15;
  char *v16;
  int v17;
  int *v18;
  int v19;

  v0 = 0;
  v19 = 0;
  if ( (shaderParseFlags & 8) != 0 )
    shaderParseSort = 0x40000000;
  if ( (shaderParseFlags & 0x70) != 0 )
  {
    if ( (*(float *)&shaderParseSort == 0.0) | __UNORDERED__(*(float *)&shaderParseSort, 0.0) )
      shaderParseSort = 1086324736;
  }
  v1 = &unk_11E69C0;
  v2 = &dword_11E6FA8;
  v15 = 0;
  v3 = shaderParseStageStorage;
  v16 = (char *)&unk_11E69C0;
  v18 = &dword_11E6FA8;
  do
  {
    if ( (*v3 & 1) == 0 )
      break;
    if ( (*v3 & 2) == 0 || r_detailtextures->integer )
    {
      v17 = 0;
      while ( 1 )
      {
        v6 = v1 - 39;
        v7 = 0;
        if ( (int)*(v1 - 7) > 0 )
          break;
LABEL_25:
        if ( *((_BYTE *)v6 + 196) )
        {
          if ( !*v1 )
            *v1 = 2;
          v19 = 1;
        }
        else
        {
          v9 = *v6;
          if ( v9 && *(_DWORD *)(v9 + 84) == 34067 )
          {
            if ( !*v1 )
              *v1 = 15;
          }
          else if ( !*v1 )
          {
            *v1 = 3;
          }
        }
        v1 += 50;
        if ( ++v17 >= 8 )
        {
          v10 = BYTE1(shaderParseFlags);
          if ( (shaderParseFlags & 0x20) != 0 )
          {
            v11 = v3[417] | 0x4000;
            goto LABEL_44;
          }
          if ( (shaderParseFlags & 0x10) != 0 )
          {
            v11 = v3[417] | 0x2000;
            goto LABEL_44;
          }
          if ( (shaderParseFlags & 0x40) != 0 )
          {
            v11 = v3[417] | 0x8000;
LABEL_44:
            v3[417] = v11;
          }
          if ( (v10 & 4) == 0 || (*(_BYTE *)v3 & 4) != 0 )
            v3[417] |= 0x200000u;
          v12 = v3[417];
          if ( (v12 & 0x70000000) == 0 && ((unsigned __int8)v12 == 37 || (unsigned __int8)v12 == 101) )
            v3[417] = v12 | 0x10000000;
          v13 = v3[417];
          if ( (_BYTE)v13 )
          {
            if ( (_BYTE)dword_11E6FA4 )
            {
              if ( (*(float *)&shaderParseSort == 0.0) | __UNORDERED__(*(float *)&shaderParseSort, 0.0) )
              {
                if ( (v13 & 0x100) != 0 )
                {
                  shaderParseSort = 1088421888;
                }
                else
                {
                  shaderParseSort = 1100480512;
                  if ( (dword_11E6FA4 & 0xF0) != 0x20 )
                    shaderParseSort = 1097859072;
                }
              }
            }
          }
          goto LABEL_59;
        }
      }
      v8 = v1 - 39;
      while ( 1 )
      {
        if ( !*v8 )
        {
          ri_Printf(2, "Shader %s has a missing image\n", shaderParseName);
          *v3 &= ~1u;
          goto LABEL_59;
        }
        if ( *(_DWORD *)(*v8 + 84) != *(_DWORD *)(*(v1 - 39) + 84) )
          break;
        v0 = v15;
        ++v7;
        ++v8;
        if ( v7 >= v6[32] )
          goto LABEL_25;
      }
      ri_Printf(2, "Shader %s has non-uniform image types in an animMap\n", shaderParseName);
      v0 = v15;
      *v3 &= ~1u;
LABEL_59:
      v2 = v18;
    }
    else if ( v0 >= 7 )
    {
      memset(v3, 0, 0x688u);
    }
    else
    {
      /* Source and destination overlap when more than one stage remains. */
      memmove(v3, v2, 1672 * (7 - v0));
      v4 = 7;
      v5 = &unk_11E96D8;
      while ( (*v5 & 1) == 0 )
      {
        --v4;
        v5 -= 1672;
        if ( v4 <= v0 )
          goto LABEL_17;
      }
      memset(&shaderParseStageStorage[418 * v4], 0, 0x688u);
LABEL_17:
      --v0;
      v3 -= 418;
      v2 -= 418;
      v16 -= 1672;
    }
    ++v0;
    v2 += 418;
    v1 = v16 + 1672;
    v3 += 418;
    v15 = v0;
    v18 = v2;
    v16 += 1672;
  }
  while ( v0 < 8 );
  if ( (*(float *)&shaderParseSort == 0.0) | __UNORDERED__(*(float *)&shaderParseSort, 0.0) )
    shaderParseSort = 1084227584;
  for ( ; v0 > 1; --v0 )
  {
    if ( !CollapseMultitexture() )
      break;
  }
  if ( shaderParseLightmapIndex >= 0 && !v19 )
  {
    ri_Printf(1, "WARNING: shader '%s' has lightmap but no lightmap stage!\n", shaderParseName);
    shaderParseLightmapIndex = -1;
  }
  shaderParseNumUnfoggedPasses = v0;
  if ( !v0 )
    shaderParseSort = 1091567616;
  if ( (char)shaderParseLightingFlags >= 0 )
    CreateDlightStage();
  ComputeHardwareNeeds();
  shaderParseOptimalStageIteratorFunc = (int)tr_stageIteratorFunc;
  if ( (shaderParseFlags & 8) != 0 )
    shaderParseOptimalStageIteratorFunc = (int)RB_StageIteratorSky;
  return GeneratePermanentShader();
}

/* ---- FindShaderInShaderText  0x004FBDE0 ----  VERIFIED */
char *FindShaderInShaderText( const char *shadername )      /* EDI */
{
	int     h;
	char   *text;
	int    *node;                   /* esi, once the walk leaves the head */
	char   *p;                      /* [esp+8+s2] */
	char   *token;

	/* 0x004FBDE1: no shader text loaded at all -> nothing to find. */
	if ( !s_shaderText ) {
		return NULL;
	}

	h    = 2 * (unsigned short)tr_shader_generateHashValue( (char *)shadername );
	text = (char *)shaderTextHashTable[h];
	node = (int *)dword_179FF64[h];

	for ( ;; ) {
		/* 0x004FBE04: an empty bucket ends the search, it does not skip. */
		p = text;
		if ( !p ) {
			return NULL;
		}

		if ( parseInfo->ungetReady ) {
			parseInfo->ungetReady = qfalse;
			p = parseInfo->ungetTokenSave;
			parseInfo->currentLine = parseInfo->ungetLineSave;
		}
		token = Com_ParseExt( &p, qtrue );

		if ( token[0] && shadername
		     && !Q_stricmpn( token, shadername, 99999 ) ) {
			return p;
		}

		if ( !node ) {
			return NULL;
		}
		text = (char *)node[0];
		node = (int *)node[1];
	}
}

/* ---- R_FindShaderByName  0x004FBE80 ----  VERIFIED */
char *__fastcall R_FindShaderByName(const char *name)
{
  char  strippedName[64];
  char *d;
  char  c;
  int   sh;

  if ( !name )
    return tr_defaultShader;
  if ( !*name )
    return tr_defaultShader;

  /* COM_StripExtension, inlined by retail at 0x004FBEA5. */
  d = strippedName;
  c = *name;
  while ( c && c != '.' )
  {
    *d++ = c;
    c = *++name;
  }
  *d = 0;

  sh = shaderHashTable[ tr_shader_generateHashValue(strippedName) ];
  while ( sh )
  {
    if ( !Q_stricmpn(strippedName, (const char *)sh, 99999) )
      return (char *)sh;
    sh = *(_DWORD *)(sh + 404);
  }
  return tr_defaultShader;
}

/* ---- R_LoadShaderType  0x004FBF20 ----  [HIGH] */
char *__cdecl R_LoadShaderType(const char *a1, int a2, int a3)
{
  unsigned int v4;
  char *v5;
  char *v7;
  char *v9; // [esp+8h] [ebp-54h] BYREF
  char *v10; // [esp+Ch] [ebp-50h] BYREF
  char v11[68]; // [esp+10h] [ebp-4Ch] BYREF
  unsigned int v12;
  unsigned int retaddr;

  v12 = retaddr ^ _security_cookie;
  strcpy(v11, "shadertypes/");
  if ( strlen(a1 + 1) + 17 < 0x40 )
  {
    v4 = strlen(a1 + 1) + 1;
    v5 = v11 + strlen(v11);
    qmemcpy(v5, a1 + 1, v4);
    v7 = v11 + strlen(v11);
    strcpy(v7, ".stype");
    if ( ri_FS_ReadFile(v11, &v9) >= 0 )
    {
      v10 = v9;
      if ( ParseShader(&v10, 1, a3) )
      {
        ri_FS_FreeFile(v9);
        strcpy(shaderParseName, a1);
        return FinishShader();
      }
      else
      {
        ri_FS_FreeFile(v9);
        ri_Error(1, "\x15" "ERROR: invalid shader type in file '%s'\n", v11);
        return 0;
      }
    }
    else
    {
      ri_Printf(2, "WARNING: could not read shader type file '%s'\n", v11);
      return 0;
    }
  }
  else
  {
    ri_Printf(2, "WARNING: shader type '%s' is too long\n", a1);
    return 0;
  }
}

/* ---- ShaderFromShaderType  0x004FC0C0 ----  [HIGH] */
char *__cdecl ShaderFromShaderType(char *Str, int a2, int a3)
{
  const char *v3;
  char *v4;
  char v5;
  char *v6;
  char *v7;
  const char *v9;
  size_t v10;
  int v11;
  __int16 v12;
  unsigned int v13; // kr00_4
  int v14;
  char *ShaderByName;
  int v16;
  _DWORD *v17;
  int v18;
  int v19;
  int *v20;
  int v21;
  int ImageInstance;
  bool v23; // cc
  const char *v24;
  int v25;
  const void **v26;
  int v27;
  _DWORD *v28;
  _DWORD *v29;
  int v30;
  char Destination[64]; // [esp+28h] [ebp-48h] BYREF
  unsigned int v35;
  unsigned int retaddr;

  v3 = Str;
  v35 = retaddr ^ _security_cookie;
  v4 = strrchr(Str, 64);
  v5 = *Str;
  v6 = Str;
  if ( *Str )
  {
    v7 = Str;
    do
    {
      if ( v5 == 47 || v5 == 92 )
        v6 = v7;
      v5 = *++v7;
    }
    while ( v5 );
  }
  if ( v6 > v4 )
  {
    ri_Printf(2, "WARNING: found '/' or '\\' after '@' in shader '%s'\n", Str);
    return 0;
  }
  v9 = v6 + 1;
  v10 = v4 - v9;
  Destination[0] = '@';
  if ( a2 == -1 )
  {
    v11 = 1701080941;
    v12 = 12140;
  }
  else
  {
    if ( a2 == -4 )
    {
      qmemcpy(&Destination[1], "2d/", 4);
      goto LABEL_18;
    }
    if ( a2 != -2 && a2 < 0 )
    {
      ri_Error(1, "\x15" "Bad lightmap index %i loading shader %s\n", a2, Str);
      goto LABEL_18;
    }
    v11 = 1819438967;
    v12 = 12132;
  }
  *(int *)&Destination[1] = v11;
  *(__int16 *)&Destination[5] = v12;
  Destination[7] = 0;
LABEL_18:
  v13 = strlen(Destination);
  if ( v13 + v10 < 0x40 )
  {
    strncpy(Destination + v13, v9, v10);
    Destination[v13 + v10] = 0;
    ShaderByName = R_FindShaderByName(Destination);
    if ( ShaderByName == tr_defaultShader && (ShaderByName = R_LoadShaderType(Destination, a2, a3)) == 0 )
    {
      return 0;
    }
    else
    {
      qmemcpy(shaderParseName, ShaderByName, 0x198u);
      v30 = 0;
      if ( shaderParseNumUnfoggedPasses > 0 )
      {
        v16 = shaderParseLightmapIndex;
        v17 = &unk_11E69A4;
        v27 = 0;
        v28 = &unk_11E69A4;
        v26 = (const void **)(ShaderByName + 340);
        while ( 2 )
        {
          qmemcpy(v17 - 33, *v26, 0x688u);
          v18 = 0;
          v25 = v27;
          v29 = v17;
          while ( 2 )
          {
            v19 = *v17 - 1;
            if ( v19 < 0 )
              v19 = 0;
            v20 = &dword_11E6924[v25 + v19];
            do
            {
              v21 = *v20;
              if ( *v20 && *(_DWORD *)(v21 + 80) == 10 )
              {
                ImageInstance = R_FindImageInstance(tr_delayedImageGroup == 0, Str, a3, (char *)v21, 0);
                *v20 = ImageInstance;
                if ( !ImageInstance )
                {
                  ri_Printf(2, "WARNING: shadertype '%s' couldn't load image for '%s'\n", Destination, Str);
                  return 0;
                }
                v16 = shaderParseLightmapIndex;
              }
              else if ( v16 >= 0 && v21 == tr_lightmaps[v16] )
              {
                if ( a2 < 0 )
                {
                  switch ( a2 )
                  {
                    case -1:
                      v24 = "used as a model skin";
                      break;
                    case -2:
                      v24 = "not used as a world shader";
                      break;
                    case -3:
                      v24 = "used as a vertex-lit surface";
                      break;
                    default:
                      v24 = "used as a 2D image";
                      if ( a2 != -4 )
                        v24 = "used some other way";
                      break;
                  }
                  ri_Printf(
                    2,
                    "WARNING: shader type '%s' is a world shader type, but '%s' is %s\n",
                    Destination,
                    Str,
                    v24);
                  return 0;
                }
                *v20 = tr_lightmaps[a2];
              }
              --v19;
              --v20;
            }
            while ( v19 > 0 );
            ++v18;
            v17 = v29 + 50;
            v29 += 50;
            v25 += 50;
            if ( v18 < 8 )
              continue;
            break;
          }
          v23 = ++v30 < shaderParseNumUnfoggedPasses;
          ++v26;
          v27 += 418;
          v28 += 418;
          if ( v23 )
          {
            v17 = v28;
            continue;
          }
          break;
        }
        v3 = Str;
      }
      strcpy(shaderParseName, v3);
      shaderParseLightmapIndex = a2;
      return GeneratePermanentShader();
    }
  }
  else
  {
    ri_Printf(2, "WARNING: shader type in '%s' is too long\n", Str);
    return 0;
  }
}

/* ---- R_BuildShaderFromImage  0x004FC440 ----  [HIGH] */
int __cdecl R_BuildShaderFromImage(int result, int a2)
{
  if ( result != -1 )
  {
    if ( result == -3 )
    {
      result = 1;
      shaderParseSurfaceFlags = 131152;
      dword_11E6924[0] = a2;
      dword_11E69B0 = 2;
      shaderParseStageStorage[0] = 1;
      dword_11E6F84 = 5;
      dword_11E6F9C = 1;
    }
    else
    {
      if ( result == -4 )
      {
        shaderParseSurfaceFlags = 131152;
        dword_11E6924[0] = a2;
        dword_11E69B0 = 2;
        shaderParseStageStorage[0] = 1;
        dword_11E6F84 = 6;
        dword_11E6F9C = 4;
        dword_11E6FA4 = 65637;
        shaderParseCullType = 2;
        goto LABEL_12;
      }
      dword_11E703C = 8448;
      dword_11E6FAC = a2;
      shaderParseSurfaceFlags = 48;
      if ( result == -2 )
      {
        dword_11E6924[0] = tr_identityLightImage;
        dword_11E6F84 = 1;
      }
      else
      {
        dword_11E6924[0] = tr_lightmaps[result];
        byte_11E69E8 = 1;
        dword_11E6F84 = 2;
      }
      dword_11E6F9C = 1;
      dword_11E6FA8 = 1;
      shaderParseStageStorage[0] = 1;
      result = dword_11E762C | 0x13;
      dword_11E69B0 = 2;
      dword_11E7038 = 2;
      dword_11E760C = 2;
      dword_11E762C |= 0x13u;
    }
    dword_11E6FA4 = 256;
    goto LABEL_12;
  }
  result = 1;
  shaderParseSurfaceFlags = 16;
  dword_11E6924[0] = a2;
  dword_11E69B0 = 2;
  shaderParseStageStorage[0] = 1;
  dword_11E6F84 = 10;
  dword_11E6F9C = 1;
  dword_11E6FA4 = 1048832;
LABEL_12:
  shaderParseFlags = 0;
  dword_11E69B4[0] = 8448;
  return result;
}

/* ---- R_FindShader  0x004FC5C0 ----  [HIGH] */
char *__cdecl R_FindShader(char *a1, int a2, int a3, const char *a4)
{
  char v4;
  bool v5; // zf
  int v6;
  char *v7;
  _DWORD *v8;
  int v9;
  cvar_t *v10;
  _DWORD *v11;
  char *v12;
  _DWORD *v13;
  _DWORD *v14;
  int v15;
  char *result;
  int ImageFile;
  int v18;
  const char *v19;
  const char *v20;
  int v21;
  char *ShaderInShaderText; // [esp+14h] [ebp-90h] BYREF
  char Source[64]; // [esp+18h] [ebp-8Ch] BYREF
  char Destination[68]; // [esp+58h] [ebp-4Ch] BYREF
  unsigned int v25;
  unsigned int retaddr;

  v4 = *a1;
  v5 = *a1 == 0;
  v25 = retaddr ^ _security_cookie;
  if ( v5 )
    return tr_defaultShader;
  v6 = a2;
  if ( a2 >= 0 && a2 >= tr_lightmapCount )
    v6 = -2;
  v7 = Source;
  do
  {
    if ( v4 == 46 )
      break;
    *v7 = v4;
    v4 = (v7++)[a1 - Source + 1];
  }
  while ( v4 );
  *v7 = 0;
  v8 = (_DWORD *)shaderHashTable[tr_shader_generateHashValue(Source)];
  if ( !v8 )
  {
LABEL_15:
    if ( tr_registered )
    {
      v10 = r_skipBackEnd;
      v11 = (_DWORD *)(backEndData + 1636096);
      *(_DWORD *)((char *)v11 + *(_DWORD *)(backEndData + 1898240)) = 0;
      v11[0x10000] = 0;
      if ( !v10->integer )
        RB_ExecuteRenderCommands(v11, a4);
    }
    memset(shaderParseName, 0, 0x198u);
    memset(shaderParseStageStorage, 0, 0x3440u);
    strncpy(shaderParseName, Source, 0x3Fu);
    byte_11E67B7 = 0;
    shaderParseLightmapIndex = v6;
    if ( v6 == -1 )
      shaderParseFlags = 512;
    v12 = (char *)&shaderParseTexMods;
    v13 = &unk_11E69E0;
    do
    {
      v14 = v13;
      v15 = 8;
      do
      {
        *v14 = v12;
        v12 += 272;
        v14 += 50;
        --v15;
      }
      while ( v15 );
      v13 += 418;
    }
    while ( v13 < (int *)((char *)&unk_11E69E0 + 8 * 1672) );  /* retail 0x011E9E20 == start + 8*1672 */
    if ( strrchr(Source, 64) )
    {
      result = ShaderFromShaderType(Source, v6, (int)a4);
      if ( result )
        return result;
      return tr_defaultShader;
    }
    ShaderInShaderText = FindShaderInShaderText(Source);
    if ( ShaderInShaderText )
    {
      if ( r_printShaders->integer )
        ri_Printf(0, "*SHADER* %s\n", a1);
      if ( !ParseShader(&ShaderInShaderText, 0, (int)a4) )
      {
        shaderParseFlags = 3;
        shaderParseSurfaceFlags = 16;
      }
      return FinishShader();
    }
    if ( !r_graymap->integer || shaderParseLightmapIndex == -4 || shaderParseLightmapIndex == -2 )
    {
      v18 = a3 != 0 ? 3 : 48;
      if ( tr_delayedImageGroup )
      {
        if ( shaderParseLightmapIndex == -1 )
          v18 &= ~0x80u;
      }
      strncpy(Destination, a1, 0x3Fu);
      Destination[63] = 0;
      Com_DefaultExtension(Destination, 0x40, ".tga");
      ImageFile = R_FindImageFile(Destination, 3553, v18, (int)a4, 0, 1.0);
      if ( !ImageFile )
      {
        ri_Printf(1, "Couldn't find image for shader %s\n", a1);
        shaderParseFlags |= 1u;
        return FinishShader();
      }
    }
    else
    {
      ImageFile = tr_grayImage;
    }
    R_BuildShaderFromImage(v6, ImageFile);
    return FinishShader();
  }
  while ( 1 )
  {
    v9 = v8[16];
    if ( (v9 == v6 || v9 == -1 && v6 >= 0 && a4 == (const char *)9) && !Q_stricmpn(Source, (const char *)v8, 99999) )
      break;
    v8 = (_DWORD *)v8[101];
    if ( !v8 )
      goto LABEL_15;
  }
  UpdateDelayLoadImagesForShader(v8, 0);
  return (char *)v8;
}

/* ---- R_RegisterShaderFromImage  0x004FC8C0 ----  VERIFIED */
int __cdecl R_RegisterShaderFromImage(char *a1, int a2, int a3)
{
  int v3;
  cvar_t *v4;
  _DWORD *v5;
  char **v6;
  char *v7;
  const char *v9;
  const char *v10;
  int v11;

  v3 = shaderHashTable[tr_shader_generateHashValue(a1)];
  if ( v3 )
  {
    while ( *(_DWORD *)(v3 + 64) != a2 && (*(_BYTE *)(v3 + 76) & 1) == 0 || !a1 || Q_stricmpn(a1, (const char *)v3, 99999) )
    {
      v3 = *(_DWORD *)(v3 + 404);
      if ( !v3 )
        goto LABEL_7;
    }
    UpdateDelayLoadImagesForShader((_DWORD *)v3, 0);
    return *(_DWORD *)(v3 + 68);
  }
  else
  {
LABEL_7:
    if ( tr_registered )
    {
      v4 = r_skipBackEnd;
      v5 = (_DWORD *)(backEndData + 1636096);
      *(_DWORD *)((char *)v5 + *(_DWORD *)(backEndData + 1898240)) = 0;
      v5[0x10000] = 0;
      if ( !v4->integer )
        RB_ExecuteRenderCommands(v5, 0);
    }
    Com_Memset(shaderParseName, 0, 0x198u);
    Com_Memset(shaderParseStageStorage, 0, 0x3440u);
    strncpy((char *)shaderParseName, a1, 0x3Fu);
    byte_11E67B7 = 0;
    shaderParseLightmapIndex = a2;
    v6 = (char **)&unk_11E6AA8;
    v7 = (char *)&unk_11E2488;
    do
    {
      *(v6 - 50) = v7 - 272;
      v6[50] = v7 + 272;
      v6[100] = v7 + 544;
      v6[150] = v7 + 816;
      v6[200] = v7 + 1088;
      v6[250] = v7 + 1360;
      *v6 = v7;
      v6[300] = v7 + 1632;
      v7 += 2176;
      v6 += 418;
    }
    while ( (int)v7 < (int)byte_11E6888 );
    R_BuildShaderFromImage(a2, a3);
    return *((_DWORD *)FinishShader() + 17);
  }
}

/* ---- RE_RegisterShaderLightMap  0x004FCA30 ----  VERIFIED */
int __cdecl RE_RegisterShaderLightMap(char *a1, int a2, const char *a3)
{
  char *Shader;

  if ( strlen(a1) >= 0x40 )
  {
    Com_Printf("Shader name exceeds MAX_QPATH\n");
    return 0;
  }
  Shader = R_FindShader(a1, a2, 1, a3);
  if ( (Shader[76] & 1) != 0 )
    return 0;
  return *((_DWORD *)Shader + 17);
}

/* ---- RE_RegisterShader  0x004FCA80 ----  [HIGH] */
int __cdecl RE_RegisterShader(char *a1, const char *a2)
{
  char *Shader;

  if ( !tr_registered )
    return 0;
  if ( strlen(a1) >= 0x40 )
  {
    Com_Printf("Shader name exceeds MAX_QPATH\n");
    return 0;
  }
  Shader = R_FindShader(a1, -4, 1, a2);
  if ( (Shader[76] & 1) != 0 )
    return 0;
  return *((_DWORD *)Shader + 17);
}

/* ---- RE_RegisterShaderNoMip  0x004FCAE0 ----  [HIGH] */
int __cdecl RE_RegisterShaderNoMip(char *a1, const char *a2)
{
  char *Shader;

  if ( !tr_registered )
    return 0;
  if ( strlen(a1) >= 0x40 )
  {
    Com_Printf("Shader name exceeds MAX_QPATH\n");
    return 0;
  }
  Shader = R_FindShader(a1, -4, 0, a2);
  if ( (Shader[76] & 1) != 0 )
    return 0;
  return *((_DWORD *)Shader + 17);
}

/* ---- R_CanOptimizeStaticModelStage  0x004FCB40 ----  [HIGH] */
int __cdecl R_CanOptimizeStaticModelStage(int a1, int a2)
{
  int v2;
  int v3;
  int result;

  if ( *(_DWORD *)(a1 + 188) )
    return 0;
  v2 = *(_DWORD *)(a1 + 1636);
  switch ( v2 )
  {
    case 1:
    case 2:
    case 10:
    case 11:
    case 12:
      v3 = *(_DWORD *)(a1 + 1660);
      if ( v3 < 0
        || v3 > 1 && v3 != 9
        || a2
        && (v2 != *(_DWORD *)(a2 + 1636)
         || v2 == 12
         && (*(_BYTE *)(a1 + 1664) != *(_BYTE *)(a2 + 1664)
          || *(_BYTE *)(a1 + 1665) != *(_BYTE *)(a2 + 1665)
          || *(_BYTE *)(a1 + 1666) != *(_BYTE *)(a2 + 1666))
         || v3 != *(_DWORD *)(a2 + 1660)
         || v3 == 9 && *(_BYTE *)(a1 + 1667) != *(_BYTE *)(a2 + 1667)) )
      {
        return 0;
      }
      result = 1;
      break;
    default:
      return 0;
  }
  return result;
}

/* ---- CloneShader  0x004FCC00 ----  [HIGH] */
int __cdecl CloneShader(const void **a1)
{
  __int64 v1; // rax
  const void **v2;

  qmemcpy(shaderParseName, a1, 0x198u);
  memset(shaderParseStageStorage, 0, 0x3440u);
  v1 = (unsigned int)a1[84];
  if ( (int)v1 > 0 )
  {
    LODWORD(v1) = shaderParseStageStorage;
    v2 = a1 + 85;
    do
    {
      shaderParseStages[HIDWORD(v1)] = v1;
      qmemcpy((void *)v1, *v2, 0x688u);
      ++HIDWORD(v1);
      ++v2;
      LODWORD(v1) = v1 + 1672;
    }
    while ( SHIDWORD(v1) < (int)a1[84] );
  }
  return v1;
}

/* ---- R_CacheableStaticModelShader  0x004FCC70 ----  VERIFIED */
char *__cdecl R_CacheableStaticModelShader(int a1)
{
  int v1;
  char *result;
  int v3;
  int v5;
  int *v6;
  int v7;
  char v8;
  int v9;
  int *v10;
  unsigned int v11;
  char name[68]; // [esp+0h] [ebp-48h] BYREF
  unsigned int v14;
  unsigned int retaddr;

  v14 = retaddr ^ _security_cookie;
  if ( *(_DWORD *)(a1 + 172) )
    return 0;
  if ( *(char *)(a1 + 84) < 0 )
    return 0;
  v1 = *(_DWORD *)(a1 + 80);
  if ( (v1 & 0x3FF5FD00) != 0 && (v1 & 0x3FF7FC00) != 0 )
    return 0;
  v3 = *(_DWORD *)(a1 + 336);
  if ( !v3 || !R_CanOptimizeStaticModelStage(*(_DWORD *)(a1 + 340), 0) )
    return 0;
  v5 = 1;
  if ( v3 > 1 )
  {
    v6 = (int *)(a1 + 344);
    while ( R_CanOptimizeStaticModelStage(*v6, *(_DWORD *)(a1 + 340)) )
    {
      ++v5;
      ++v6;
      if ( v5 >= *(_DWORD *)(a1 + 336) )
        goto LABEL_13;
    }
    return 0;
  }
LABEL_13:
  name[0] = 63;
  strcpy(&name[1], (const char *)a1);
  result = R_FindShaderByName(name);
  if ( result == tr_defaultShader )
  {
    CloneShader((const void **)a1);
    v7 = 0;
    do
    {
      v8 = name[v7];
      shaderParseName[v7++] = v8;
    }
    while ( v8 );
    shaderParseFlags |= 4u;
    shaderParseLightingFlags &= 0xFFFFFFE7;
    shaderParseOptimizedBackend = tr_cachedStaticModelSurfaceType;
    v9 = 0;
    if ( *(int *)(a1 + 336) > 0 )
    {
      v10 = &dword_11E6FA4;
      do
      {
        v11 = *v10 & 0xFFEFFFFF;
        *(v10 - 417) &= 0xFFFFFFE7;
        *v10 = v11;
        ++v9;
        v10 += 418;
      }
      while ( v9 < *(_DWORD *)(a1 + 336) );
    }
    return GeneratePermanentShader();
  }
  return result;
}

/* ---- R_MergeShaderList  0x004FCDF0 ----  [HIGH] */
void __cdecl R_MergeShaderList(char **a1, int a2, int a3)
{
  char *PermanentShader;
  int v4;
  _DWORD *v5;
  unsigned int v6;
  _DWORD *v7;
  int i;
  int v9;
  int v10;

  if ( a2 == 1 )
  {
    PermanentShader = *a1;
  }
  else
  {
    CloneShader((const void **)*a1);
    sprintf(shaderParseName, "*sheet%03i", a3);
    PermanentShader = GeneratePermanentShader();
    if ( PermanentShader == tr_defaultShader )
    {
      ri_Printf(2, "WARNING: too many shaders after building texture sheets\n");
      return;
    }
  }
  *((_DWORD *)PermanentShader + 19) &= ~0x800u;
  v9 = *((_DWORD *)*a1 + 97);
  v10 = *(_DWORD *)(v9 + 104);
  v4 = 0;
  if ( *((int *)PermanentShader + 84) > 0 )
  {
    v5 = PermanentShader + 340;
    do
    {
      v6 = 0;
      v7 = (_DWORD *)(*v5 + 4);
      do
      {
        if ( !*v7 )
          break;
        if ( *v7 == v9 )
          *v7 = v10;
        ++v6;
        v7 += 50;
      }
      while ( v6 < 8 );
      ++v4;
      ++v5;
    }
    while ( v4 < *((_DWORD *)PermanentShader + 84) );
  }
  if ( r_debugOptTex->integer )
    ri_Printf(0, "merging %i shader(s):\n", a2);
  for ( i = 0; i < a2; ++i )
  {
    if ( r_debugOptTex->integer )
      ri_Printf(0, "-> %s\n", a1[i]);
    *((_DWORD *)a1[i] + 19) |= 0x1000u;
    *((_DWORD *)a1[i] + 96) = PermanentShader;
  }
}

/* ---- compare_mergable_shaders  0x004FCF20 ----  VERIFIED */
int __cdecl compare_mergable_shaders(int *a1, int *a2)
{
  int v2;
  int result;

  v2 = *(_DWORD *)(*a1 + 388);
  result = (*(_DWORD *)(v2 + 104) - *(_DWORD *)(*(_DWORD *)(*a2 + 388) + 104)) / 120;
  if ( !result )
    return CompareMergableShaders(*a2, *a1, v2, *(_DWORD *)(*a2 + 388));
  return result;
}

/* ---- R_MergeShadersForImageSheets  0x004FCF70 ----  [HIGH] */
void R_MergeShadersForImageSheets()
{
  signed int v0;
  int v1;
  int v2;
  int v3;
  int v4;
  signed int v5;
  signed int v6;
  int v7;
  int v8;
  int v9;
  signed int v10;
  signed int v11;
  int v12;
  int v13;
  _DWORD v14[4096]; // [esp+20h] [ebp-4000h] BYREF

  v0 = 0;
  v1 = 0;
  v10 = 0;
  if ( tr_numShaders[0] > 0 )
  {
    do
    {
      v2 = tr_shaders[v1];
      v3 = *(_DWORD *)(v2 + 76);
      if ( (v3 & 0x800) != 0 )
      {
        *(_DWORD *)(v2 + 76) = v3 & 0xFFFFF7FF;
        v4 = tr_shaders[v1];
        if ( *(_DWORD *)(*(_DWORD *)(v4 + 388) + 104) )
          v14[v0++] = v4;
        else
          *(_DWORD *)(v4 + 388) = 0;
      }
      ++v1;
    }
    while ( v1 < tr_numShaders[0] );
    v10 = v0;
  }
  v12 = 0;
  qsort_m((unsigned int)v14, v0, 4u, (int (__cdecl *)(unsigned int, _BYTE *))compare_mergable_shaders);
  v5 = 0;
  v11 = 0;
  if ( v0 > 0 )
  {
    while ( 1 )
    {
      v6 = v5 + 1;
      if ( v5 + 1 < v0 )
      {
        v7 = v14[v5];
        v13 = *(_DWORD *)(*(_DWORD *)(v7 + 388) + 104);
        do
        {
          v8 = v14[v6];
          v9 = *(_DWORD *)(v8 + 388);
          if ( (v13 - *(_DWORD *)(v9 + 104)) / 120 )
            break;
          if ( CompareMergableShaders(v8, v7, *(_DWORD *)(v7 + 388), v9) )
            break;
          ++v6;
        }
        while ( v6 < v10 );
        v0 = v10;
        v5 = v11;
      }
      R_MergeShaderList((char **)&v14[v5], v6 - v5, v12++);
      v11 = v6;
      if ( v6 >= v0 )
        break;
      v5 = v6;
    }
  }
}

/* ---- R_SetupTextureCoordinateRemap  0x004FD0B0 ----  [HIGH] */
unsigned __int16 __cdecl R_SetupTextureCoordinateRemap(
        int a1,
        float *a2,
        BOOL *a3,
        float *a4,
        _DWORD *a5)
{
  int v5;
  int v6;
  BOOL v7;
  int v8;
  cvar_t *v9;
  int integer;
  int v11;
  int v12;
  unsigned __int16 result;

  v5 = *(_DWORD *)(a1 + 388);
  v6 = *(_DWORD *)(v5 + 104);
  v7 = *(_WORD *)(v5 + 64) >= *(_WORD *)(v5 + 66);
  *a5 = *(_WORD *)(v5 + 64) < *(_WORD *)(v5 + 66);
  *a3 = v7;
  v8 = *(_DWORD *)(v5 + 100);
  v9 = r_picmip2;
  if ( (v8 & 4) == 0 )
    v9 = r_picmip;
  integer = v9->integer;
  if ( (v8 & 2) != 0 )
  {
    v12 = 3;
    if ( integer <= 3 )
      v12 = integer;
    v11 = v12 & ~(v12 >> 31);
  }
  else
  {
    LOBYTE(v11) = 0;
  }
  *a2 = 1.0 / (double)*(unsigned __int16 *)(v6 + 64);
  a2[1] = 1.0 / (double)*(unsigned __int16 *)(v6 + 66);
  *a4 = (double)*(unsigned __int16 *)(v5 + 108) * *a2;
  a4[1] = (double)*(unsigned __int16 *)(v5 + 110) * a2[1];
  a2[*a5] = (double)(unsigned __int16)(*(_WORD *)(v5 + 64) >> v11) * a2[*a5];
  result = *(_WORD *)(v5 + 66) >> v11;
  a2[*a3] = (double)result * a2[*a3];
  return result;
}

/* ---- R_RemapTextureCoordinatesForSheet  0x004FD1A0 ----  [HIGH] */
int __cdecl R_RemapTextureCoordinatesForSheet(int a1, int a2, int a3)
{
  int result;
  float *v4;
  float *v5;
  double v6;
  double v7;
  int v8; // [esp+4h] [ebp-18h] BYREF
  BOOL v9; // [esp+8h] [ebp-14h] BYREF
  float v10[2]; // [esp+Ch] [ebp-10h] BYREF
  float v11[2]; // [esp+14h] [ebp-8h] BYREF

  R_SetupTextureCoordinateRemap(a3, v10, &v9, v11, &v8);
  result = 0;
  if ( a1 > 0 )
  {
    v4 = (float *)(a2 + 4 * v8);
    v5 = (float *)(a2 + 4 * v9);
    do
    {
      v6 = *v5;
      ++result;
      v5 += 2;
      v7 = v10[0] * *v4;
      v4 += 2;
      *(float *)(a2 + 8 * result - 8) = v7 + v11[0];
      *(float *)(a2 + 8 * result - 4) = v10[1] * v6 + v11[1];
    }
    while ( result < a1 );
  }
  return result;
}

/* ---- RE_GetShaderName  0x004FD210 ----  VERIFIED */
char *__cdecl RE_GetShaderName(int a1)
{
  char *result;

  if ( a1 >= 0 && a1 < tr_numShaders[0] )
  {
    result = (char *)tr_shaders[a1];
  }
  else
  {
    ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", a1);
    result = tr_defaultShader;
  }
  if ( result == tr_defaultShader )
    return &empty_string;
  return result;
}

/* ---- R_GetShaderByHandle  0x004FD250 ----  VERIFIED */
char *__cdecl R_GetShaderByHandle(int a1)
{
  if ( a1 >= 0 && a1 < tr_numShaders[0] )
    return (char *)tr_shaders[a1];
  ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", a1);
  return tr_defaultShader;
}

/* ---- R_ShaderList_f  0x004FD280 ----  [HIGH] */
void R_ShaderList_f()
{
  int v0;
  int v1;
  int v2;
  int v3;
  int v4;
  void (*v5)();
  const char *v6;
  const char *v7;
  const char *v8;
  const char *v9;
  const char *v10;

  ri_Printf(0, "-----------------------\n");
  v0 = 0;
  v1 = 0;
  if ( tr_numShaders[0] > 0 )
  {
    while ( 1 )
    {
      v2 = ri_Cmd_Argc() <= 1 ? tr_shaders[v1] : dword_16CF84C[v1];
      ri_Printf(0, "%i ", *(_DWORD *)(v2 + 336));
      v7 = *(int *)(v2 + 64) < 0 ? "  " : "L ";
      ri_Printf(0, v7);
      v3 = *(_DWORD *)(v2 + 340);
      if ( !v3 )
        break;
      v4 = *(_DWORD *)(v3 + 348);
      switch ( v4 )
      {
        case 260:
          v8 = "MT(a) ";
          break;
        case 8448:
          v8 = "MT(m) ";
          break;
        case 8449:
          v8 = "MT(d) ";
          break;
        default:
          goto LABEL_15;
      }
LABEL_16:
      ri_Printf(0, v8);
      if ( (*(_BYTE *)(v2 + 76) & 2) != 0 )
        v9 = "E ";
      else
        v9 = "  ";
      ri_Printf(0, v9);
      v5 = *(void (**)())(v2 + 372);
      if ( (char *)v5 == (char *)tr_stageIteratorFunc )
      {
        v10 = "gen ";
      }
      else if ( v5 == RB_StageIteratorSky )
      {
        v10 = "sky ";
      }
      else
      {
        v10 = "    ";
      }
      ri_Printf(0, v10);
      if ( (*(_BYTE *)(v2 + 76) & 1) != 0 )
        v6 = ": %s (DEFAULTED)\n";
      else
        v6 = ": %s\n";
      ri_Printf(0, v6, v2);
      ++v0;
      if ( ++v1 >= tr_numShaders[0] )
        goto LABEL_28;
    }
LABEL_15:
    v8 = "      ";
    goto LABEL_16;
  }
LABEL_28:
  ri_Printf(0, "%i total shaders\n", v0);
  ri_Printf(0, "------------------\n");
}

static int tr_shaderStringPointers[2 * 100000];   /* 0x016DCA60 */

/* ---- BuildShaderChecksumLookup  0x004FD3F0 ----  VERIFIED */
char __cdecl BuildShaderChecksumLookup(int a1, const char *a2, const char *a3)
{
  char result;
  int v4;
  int *v5;
  parseInfo_t *v6;
  char *v7;
  int ungetLineSave;
  char *v9;
  char *v10;
  int v11;
  int v12;
  char **v13;
  const char *v14;
  const char *v15;
  int v16;
  char *data_p; // [esp+4h] [ebp-4h] BYREF

  result = 0;
  data_p = (char *)s_shaderText;
  memset(shaderTextHashTable, 0, 0x8000u);
  memset(dword_179FF64, 0, sizeof( dword_179FF64 ));
  if ( s_shaderText )
  {
    v16 = a1;
    v15 = a2;
    v14 = a3;
    v4 = 0;
    v5 = tr_shaderStringPointers;
    while ( 1 )
    {
      v6 = parseInfo;
      v7 = data_p;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        ungetLineSave = v6->ungetLineSave;
        data_p = v6->ungetTokenSave;
        v6->currentLine = ungetLineSave;
      }
      v9 = Com_ParseExt(&data_p, qtrue);
      result = *v9;
      if ( !*v9 )
        break;
      if ( Q_stricmpn("{", v9, 99999) )
      {
        if ( v4 )
        {
          ri_Printf(2, "Look before shader '%s'\n", v9);
          v4 = 0;
        }
        v11 = 2 * (unsigned __int16)tr_shader_generateHashValue(v9);
        if ( shaderTextHashTable[v11] )
        {
          if ( v5 >= tr_shaderStringPointers + 2 * 100000 )
            ri_Error(1, "\x15" "MAX_SHADER_STRING_POINTERS exceeded, too many shaders");
          v12 = dword_179FF64[v11];
          v13 = (char **)v5;
          v5 += 2;
          *v13 = v7;
          v13[1] = (char *)v12;
          dword_179FF64[v11] = (int)v13;
        }
        else
        {
          shaderTextHashTable[v11] = (int)v7;
        }
      }
      else
      {
        Com_UngetToken();
        if ( Com_SkipBracedSection(&data_p, 5) )
        {
          ri_Printf(2, "Shader brace nesting depth exceeded... missing '}'?\n");
          v4 = 1;
        }
      }
    }
  }
  return result;
}

/* ---- ScanAndLoadShaderFiles  0x004FD560 ----  [HIGH] */
void __cdecl ScanAndLoadShaderFiles(const char *a1)
{
  int v1;
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  _DWORD *v9;
  int v10;
  bool v11; // zf
  char *v12;
  char *v13;
  const char *v14;
  int j;
  char *v16;
  int v17;
  const char *v18;
  int v19;
  int v20; // [esp+10h] [ebp-405Ch] BYREF
  int v21;
  int v22;
  int v23;
  int i;
  int v25; // [esp+24h] [ebp-4048h] BYREF
  char v26[64]; // [esp+28h] [ebp-4044h] BYREF
  _DWORD v27[4097]; // [esp+68h] [ebp-4004h] BYREF
  unsigned int retaddr;

  v27[4096] = retaddr ^ _security_cookie;
  v1 = 0;
  v21 = 0;
  v2 = ri_FS_ListFiles("scripts", ".shader", &v20);
  v3 = v20;
  v4 = v2;
  v23 = v2;
  v5 = ri_FS_ListFiles("fxshaders", ".shader", &v25);
  v6 = v25 + v3;
  v22 = v5;
  v19 = v6;
  if ( v4 && (v7 = v20) != 0 )
  {
    if ( v6 > 4096 )
    {
      v19 = 4096;
      v6 = 4096;
    }
    v18 = a1;
    if ( v6 > 0 )
    {
      v8 = v4 - (_DWORD)v27;
      v9 = v27;
      for ( i = v4 - (_DWORD)v27; ; v8 = i )
      {
        if ( v1 >= v7 )
        {
          v17 = *(_DWORD *)(v22 + 4 * (v1 - v7));
          v16 = "fxshaders/%s";
        }
        else
        {
          v17 = *(_DWORD *)((char *)v9 + v8);
          v16 = "scripts/%s";
        }
        Com_sprintf(v26, 64, v16, v17);
        ri_Printf(0, "...loading '%s'\n", v26);
        v10 = ri_FS_ReadFile(v26, v9);
        v11 = *v9 == 0;
        v21 += v10;
        if ( v11 )
        {
          v12 = va("EXE_ERR_COULDNT_LOAD\x15%s", v26);
          ri_Error(1, v12);
        }
        ++v1;
        ++v9;
        if ( v1 >= v19 )
          break;
        v7 = v20;
      }
      v4 = v23;
      v6 = v19;
    }
    v13 = (char *)ri_Hunk_Alloc(v21 + 2 * v6);
    v14 = 0;
    s_shaderText = (int)v13;
    if ( v6 > 0 )
    {
      do
      {
        strcat(v13, "\n");
        qmemcpy(
          &v13[strlen(v13)],
          (const void *)v27[(_DWORD)v14],
          v27[(_DWORD)v14] + strlen((const char *)v27[(_DWORD)v14]) + 1 - v27[(_DWORD)v14]);
        ++v14;
      }
      while ( (int)v14 < v19 );
      v4 = v23;
      v6 = v19;
    }
    for ( j = v6 - 1; j >= 0; v27[j + 1] = 0 )
      ri_FS_FreeFile(v27[j--]);
    ri_FS_FreeFileList(v22);
    ri_FS_FreeFileList(v4);
    BuildShaderChecksumLookup(0, v14, (const char *)j);
  }
  else
  {
    ri_Printf(2, "WARNING: no shader files found\n");
  }
}

/* ---- CreateDefaultShader  0x004FD780 ----  [HIGH] */
char *CreateDefaultShader()
{
  char *result;

  memset(shaderParseName, 0, 0x198u);
  memset(shaderParseStageStorage, 0, 0x3440u);
  strncpy(shaderParseName, "<default>", 0x3Fu);
  byte_11E67B7 = 0;
  shaderParseLightmapIndex = -1;
  shaderParseFlags = 1;
  shaderParseSurfaceFlags = 16;
  dword_11E6924[0] = tr_defaultImage;
  dword_11E69B0 = 2;
  dword_11E69B4[0] = 8448;
  shaderParseStageStorage[0] = 1;
  dword_11E6FA4 = 256;
  result = FinishShader();
  tr_defaultShader = result;
  return result;
}

/* ---- CreateShadowShader  0x004FD810 ----  [HIGH] */
char *CreateShadowShader()
{
  char *result;

  memset(shaderParseName, 0, 0x198u);
  memset(shaderParseStageStorage, 0, 0x3440u);
  strncpy(shaderParseName, "<stencil shadow>", 0x3Fu);
  byte_11E67B7 = 0;
  shaderParseLightmapIndex = -1;
  shaderParseSort = 1101004800;
  result = FinishShader();
  tr_stencilShadowShader = (int)result;
  return result;
}

/* ---- CreateShowTrisShader  0x004FD870 ----  [HIGH] */
char *CreateShowTrisShader()
{
  int integer;
  char *result;

  memset(shaderParseName, 0, 0x198u);
  memset(shaderParseStageStorage, 0, 0x3440u);
  strncpy(shaderParseName, "<showtris>", 0x3Fu);
  shaderParseCullType = 2;
  dword_11E69B0 = 2;
  byte_11E67B7 = 0;
  shaderParseLightmapIndex = -1;
  shaderParseFlags = 1024;
  shaderParseSurfaceFlags = 16;
  dword_11E6924[0] = tr_whiteImage;
  dword_11E69B4[0] = 8448;
  shaderParseStageStorage[0] = 1;
  dword_11E6FA4 = 4352;
  dword_11E6F84 = 13;
  dword_11E6F9C = 0;
  integer = r_optimize->integer;
  byte_11E6FA1 = tr_identityLightByte;
  byte_11E6FA0 = tr_identityLightByte;
  byte_11E6FA2 = 0;
  if ( !integer )
    byte_11E6FA2 = tr_identityLightByte;
  result = FinishShader();
  tr_showTrisShader = (int)result;
  return result;
}

/* ---- CreateShowImagesShader  0x004FD940 ----  [HIGH] */
char *CreateShowImagesShader()
{
  char *result;

  memset(shaderParseName, 0, 0x198u);
  memset(shaderParseStageStorage, 0, 0x3440u);
  strncpy(shaderParseName, "<showimages>", 0x3Fu);
  shaderParseCullType = 2;
  dword_11E69B0 = 2;
  shaderParseStageStorage[0] = 1;
  dword_11E6F84 = 1;
  byte_11E67B7 = 0;
  shaderParseLightmapIndex = -1;
  shaderParseFlags = 1024;
  shaderParseSurfaceFlags = 16;
  dword_11E6924[0] = tr_whiteImage;
  dword_11E69B4[0] = 8448;
  dword_11E6FA4 = 0x10000;
  dword_11E6F9C = 0;
  byte_11E6FA0 = tr_identityLightByte;
  byte_11E6FA1 = tr_identityLightByte;
  byte_11E6FA2 = tr_identityLightByte;
  result = FinishShader();
  tr_showImagesShader = (int)result;
  return result;
}

/* ---- CreateScreenShader  0x004FDA00 ----  [HIGH] */
char *CreateScreenShader()
{
  char *result;

  memset(shaderParseName, 0, 0x198u);
  memset(shaderParseStageStorage, 0, 0x3440u);
  strncpy(shaderParseName, "<screen>", 0x3Fu);
  byte_11E67B7 = 0;
  shaderParseLightmapIndex = -4;
  shaderParseFlags = 1024;
  shaderParseSurfaceFlags = 80;
  shaderParseCullType = 2;
  dword_11E6924[0] = tr_screenImage;
  dword_11E69B0 = 2;
  dword_11E69B4[0] = 8448;
  shaderParseStageStorage[0] = 1;
  dword_11E6FA4 = 101;
  dword_11E6F84 = 5;
  dword_11E6F9C = 4;
  result = FinishShader();
  tr_screenImageShader = (int)result;
  return result;
}

/* ---- CreateInternalShaders  0x004FDAB0 ----  [HIGH] */
char *CreateInternalShaders()
{
  tr_numShaders[0] = 0;
  CreateDefaultShader();
  CreateShadowShader();
  CreateShowTrisShader();
  CreateShowImagesShader();
  return CreateScreenShader();
}

/* ---- CreateExternalShaders  0x004FDAE0 ----  [HIGH] */
char *CreateExternalShaders()
{
  char *result;

  tr_flareShader = (int)R_FindShader("flareShader", -1, 1, (const char *)4);
  tr_spotLightShader = (int)R_FindShader("spotLight", -1, 1, (const char *)4);
  result = R_FindShader("dlightshader", -1, 1, (const char *)4);
  tr_dlightShader = (int)result;
  return result;
}

/* ---- R_InitShaders  0x004FDB30 ----  [HIGH] */
char *__cdecl R_InitShaders(const char *a1)
{
  char *result;

  glfogNum = 0;
  ri_Printf(0, "Initializing Shaders\n");
  memset(shaderHashTable, 0, 0x4000u);
  tr_numShaders[0] = 0;
  CreateDefaultShader();
  CreateShadowShader();
  CreateShowTrisShader();
  CreateShowImagesShader();
  CreateScreenShader();
  ScanAndLoadShaderFiles(a1);
  tr_flareShader = (int)R_FindShader("flareShader", -1, 1, (const char *)4);
  tr_spotLightShader = (int)R_FindShader("spotLight", -1, 1, (const char *)4);
  result = R_FindShader("dlightshader", -1, 1, (const char *)4);
  tr_dlightShader = (int)result;
  return result;
}
