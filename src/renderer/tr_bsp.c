/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_gl_types.h"
#include "tr_tess.h"


#define r_fullbright               ((cvar_t *)(r_fullbright))
#define r_optimize                 ((cvar_t *)(r_optimize))
#define r_optimizeWorld            ((cvar_t *)(r_optimizeWorld))
#define r_vc_compile               ((cvar_t *)(r_vc_compile))


void AddPointToBounds( const float *point, float *mins, float *maxs );
extern int ExpandBounds();
extern int Hunk_AllocAlignInternal();
extern float NormalizeColorWhite_m( const float *in, float *out );
extern int RB_CalcTangentSpace();
extern void __cdecl RB_ComputeColors( int stage );   /* ONE argument -- 0x004DA88A pushes one dword */
extern int RB_ExecuteRenderCommands();
extern int RB_StageIteratorSky();
extern int R_AllocImage();
extern int R_CreateBufferARB();
extern int R_CreateImageInternal();
void __fastcall R_CreateStaticModel(const float *scale, char *name, int unused, const float *origin, const float *angles, const float *lightingPrecalc);
extern int R_FindShader();
extern int R_InitLightVisCacheFromBuffer();
extern int R_InitLightVisHistory();
extern int R_InitStaticModelCache();
extern int R_LoadSunThroughCvars();
extern int R_PrecalcLightVisCache();
extern int tr_image_generateHashValue();

/* ---- R_ColorShiftLightingBytes  0x004D9AF0 ----  VERIFIED */
int __cdecl R_ColorShiftLightingBytes(unsigned __int8 *a1, _BYTE *a2)
{
  int v2;
  int result;
  int v4;
  int v5;

  v2 = a1[2] << (1 - tr_overbrightBits);
  result = a1[1] << (1 - tr_overbrightBits);
  v4 = *a1 << (1 - tr_overbrightBits);
  if ( (v4 | result | v2) > 255 )
  {
    v5 = *a1 << (1 - tr_overbrightBits);
    if ( v4 <= result )
      v5 = a1[1] << (1 - tr_overbrightBits);
    if ( v5 <= v2 )
      v5 = a1[2] << (1 - tr_overbrightBits);
    v4 = 255 * v4 / v5;
    result = 255 * (a1[1] << (1 - tr_overbrightBits)) / v5;
    v2 = 255 * (a1[2] << (1 - tr_overbrightBits)) / v5;
  }
  *a2 = v4;
  a2[1] = result;
  a2[2] = v2;
  a2[3] = a1[3];
  return result;
}

/* ---- R_BuildLightmapMergability  0x004D9B90 ----  [HIGH] */
int __cdecl R_BuildLightmapMergability(int a1, int a2, int a3)
{
  _DWORD *v3;
  int v4;
  int v5;
  int v6;
  __int16 *v7;
  int v8;
  int v9;
  __int16 *v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  _DWORD *v17;
  int v18;
  _DWORD *v19;
  int v20;
  bool v21; // sf
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
  _DWORD *v32;
  _DWORD *v33;
  _DWORD *v34;
  int v35;
  int v36;
  _DWORD *v37;
  bool v38; // zf
  int v40;
  int v41;
  int v42;
  int v43;
  int i;
  int j;
  int v46;
  int v47;
  int v48;
  int v49;
  int v50;
  int v51;
  int v52;
  int v53;
  _DWORD *v54;
  _DWORD v55[128]; // [esp+30h] [ebp-10200h] BYREF
  _DWORD v56[16384]; // [esp+230h] [ebp-10000h] BYREF

  v3 = (_DWORD *)a1;
  v4 = bspImageBase + ((_DWORD *)a1)[1];
  v50 = v4;
  if ( (*(_BYTE *)a1 & 0xF) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v5 = *v3 >> 4;
  v6 = 0;
  v41 = v5;
  v46 = 0;
  if ( v5 )
  {
    v7 = (__int16 *)(v4 + 2);
    v8 = v5;
    do
    {
      v9 = *v7;
      if ( v6 <= v9 )
      {
        v46 = v9 + 1;
        v6 = v9 + 1;
      }
      v7 += 8;
      --v8;
    }
    while ( v8 );
  }
  memset(v55, 0, sizeof(v55));
  memset(v56, 0, sizeof(v56));
  for ( i = 0; i < s_worldData_numShaders; ++i )
  {
    if ( v5 > 0 )
    {
      v10 = (__int16 *)(v4 + 8);
      v11 = v5;
      do
      {
        if ( *(v10 - 4) == i )
        {
          v12 = *(v10 - 3);
          if ( v12 >= 0 )
            v55[v12] += *v10;
        }
        v10 += 8;
        --v11;
      }
      while ( v11 );
      v5 = v41;
    }
    v13 = 0;
    v47 = 0;
    if ( v6 > 0 )
    {
      v14 = 0;
      do
      {
        v15 = v55[v13];
        if ( v15 )
        {
          v16 = v13 + 1;
          if ( v13 + 1 < v6 )
          {
            v17 = &v56[v14 + 128 + v13];
            do
            {
              v18 = v55[v16];
              if ( v18 )
              {
                v19 = &v56[v14 + v16];
                v20 = v15 + v18;
                v21 = v20 + *v19 < 0;
                *v19 += v20;
                if ( v21 )
                  *v19 = 0x7FFFFFFF;
                *v17 = *v19;
                v13 = v47;
              }
              ++v16;
              v17 += 128;
            }
            while ( v16 < v46 );
            v6 = v46;
            v5 = v41;
          }
          v55[v13] = 0;
        }
        ++v13;
        v14 += 128;
        v47 = v13;
      }
      while ( v13 < v6 );
      v4 = v50;
    }
  }
  memset(v55, 0, sizeof(v55));
  v22 = 0;
  v23 = 0;
  v24 = glConfig_maxTextureSize / 512;
  v25 = glConfig_maxTextureSize / 512;
  v51 = 0;
  v52 = glConfig_maxTextureSize / 512;
  v53 = glConfig_maxTextureSize / 512;
  for ( j = 0; j < v6; ++v51 )
  {
    if ( v24 * v25 > v6 - v23 )
    {
      do
      {
        if ( v24 < v25 )
          v25 >>= 1;
        else
          v24 >>= 1;
      }
      while ( v24 * v25 > v6 - v23 );
      v53 = v25;
      v52 = v24;
    }
    v26 = v24 * v25;
    v48 = v24 * v25;
    if ( v24 * v25 < 2 )
    {
      v40 = 0;
      while ( v55[v40] )
      {
        if ( ++v40 >= v6 )
          goto LABEL_61;
      }
      *(_DWORD *)(a3 + 4 * v23) = v40;
      j = v23 + 1;
      v55[v40] = 1;
    }
    else
    {
      v27 = -1;
      v28 = -1;
      v29 = 0;
      v42 = 0;
      do
      {
        if ( !v55[v29] )
        {
          v30 = v29 + 1;
          if ( v29 + 1 < v46 )
          {
            do
            {
              if ( !v55[v30] && (v27 < 0 || v56[v30 + v42] > v56[128 * v27 + v28]) )
              {
                v27 = v29;
                v28 = v30;
              }
              ++v30;
            }
            while ( v30 < v46 );
            v26 = v48;
            v23 = j;
          }
        }
        v42 += 128;
        ++v29;
      }
      while ( v29 < v46 );
      *(_DWORD *)(a3 + 4 * v23) = v28;
      v31 = v23 + 1;
      *(_DWORD *)(a3 + 4 * v31) = v27;
      v55[v28] = 1;
      j = v31 + 1;
      v55[v27] = 1;
      if ( v26 > 2 )
      {
        v54 = &v56[v27];
        v43 = v27 << 9;
        v49 = v26 - 2;
        do
        {
          v32 = v54;
          v33 = (_DWORD *)((char *)v56 + v43);
          v34 = &v56[128 * v28];
          v35 = v46;
          do
          {
            *v33 += *v34;
            *v32 = *v33;
            ++v34;
            ++v33;
            v32 += 128;
            --v35;
          }
          while ( v35 );
          v28 = -1;
          v36 = 0;
          v37 = (_DWORD *)((char *)v56 + v43);
          do
          {
            if ( !v55[v36] && (v28 < 0 || *v37 > *(_DWORD *)((char *)&v56[v28] + v43)) )
              v28 = v36;
            ++v36;
            ++v37;
          }
          while ( v36 < v46 );
          *(_DWORD *)(a3 + 4 * j++) = v28;
          v38 = v49 == 1;
          v55[v28] = 1;
          --v49;
        }
        while ( !v38 );
      }
      v6 = v46;
    }
LABEL_61:
    v24 = v52;
    v25 = v53;
    v23 = j;
    *(_DWORD *)(a2 + 8 * v51) = v52;
    *(_DWORD *)(a2 + 8 * v51 + 4) = v53;
    v22 = v51 + 1;
  }
  ri_Printf(0, "%i merged lightmaps from %i original lightmaps\n", v22, v6);
  return v6;
}

/* ---- R_CopyLightmap  0x004D9F30 ----  VERIFIED */
int __cdecl R_CopyLightmap(int a1, int a2, int a3, int a4, unsigned __int8 *a5)
{
  _BYTE *v5;
  int v7;
  int result;
  int v9;
  int v10;

  v5 = (_BYTE *)(a4 + 4 * (a2 + a3 * a1));
  v9 = 4 * a3 - 2048;
  v10 = 512;
  do
  {
    v7 = 512;
    do
    {
      R_ColorShiftLightingBytes(a5, v5);
      v5[3] = -1;
      a5 += 3;
      v5 += 4;
      --v7;
    }
    while ( v7 );
    v5 += v9;
    result = --v10;
  }
  while ( v10 );
  return result;
}

/* ---- R_LoadLightmaps  0x004D9FA0 ----  [HIGH] */
int __cdecl R_LoadLightmaps(int *a1, const char *a2, int a3, int a4)
{
  int v4;
  cvar_t *v5;
  _DWORD *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  _BYTE *v11;
  unsigned int v12;
  int v13;
  int v14;
  int v15;
  __int16 v16;
  int v17;
  double v18;
  double v19;
  int v20;
  int v21;
  int v22;
  int v23; // et2
  int v24;
  char *v25;
  int v26;
  int HashValue;
  int v28;
  int v29;
  bool v30; // cc
  __int16 v32;
  int v33;
  unsigned __int8 *v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  __int16 v40;
  int v41;
  int v42[256]; // [esp+38h] [ebp-600h] BYREF -- 128 x (width, height)
  _DWORD v44[128]; // [esp+438h] [ebp-200h] BYREF -- merge order, closes on retaddr

  v4 = *a1;
  v41 = bspImageBase + a1[1];
  v35 = *a1;
  if ( tr_registered )
  {
    v5 = r_skipBackEnd;
    v6 = (_DWORD *)(backEndData + 1636096);
    *(_DWORD *)((char *)v6 + *(_DWORD *)(backEndData + 1898240)) = 0;
    v6[0x10000] = 0;
    if ( !v5->integer )
      RB_ExecuteRenderCommands(v6, a2);
  }
  v7 = R_BuildLightmapMergability((int)a3, (int)v42, (int)v44);
  v8 = v7;
  v38 = v7;
  if ( v4 )
  {
    if ( 786432 * v7 != v4 )
      ri_Error(1, "\x15" "R_LoadLightmaps: incorrect lightmap lump size");
  }
  if ( r_fullbright->integer )
  {
    v35 = 0;
    v4 = 0;
  }
  v9 = (v42[0] * v42[1]) << 20;
  v10 = ri_Hunk_AllocateTempMemory(v9);
  v34 = (unsigned __int8 *)v10;
  if ( !v4 && v9 > 0 )
  {
    v11 = (_BYTE *)(v10 + 1);
    v12 = ((unsigned int)(v9 - 1) >> 2) + 1;
    do
    {
      *(v11 - 1) = tr_identityLightByte;
      *v11 = tr_identityLightByte;
      v11[1] = tr_identityLightByte;
      v11[2] = -1;
      v11 += 4;
      --v12;
    }
    while ( v12 );
    v8 = v38;
  }
  v32 = 0;
  v33 = 0;
  if ( v8 > 0 )
  {
    do
    {
      v13 = v42[2 * v32];
      v14 = v42[2 * v32 + 1];
      v39 = 0;
      v15 = v13 * v14;
      v16 = (_WORD)v14 << 9;
      v40 = (_WORD)v14 << 9;
      if ( v13 * v14 > 0 )
      {
        v17 = 0;
        v18 = 1.0 / (double)v42[2 * v32];
        v19 = 1.0 / (double)v42[2 * v32 + 1];
        do
        {
          v20 = v44[v17 + v33];
          v23 = v17 % v13;
          v21 = v17 / v13;
          v22 = v23;
          v36 = v23;
          v37 = v21;
          if ( v35 )
            R_CopyLightmap(v21 << 9, v22 << 9, v13 << 9, (int)v34, (unsigned __int8 *)(v41 + 786432 * v20));
          *(float *)(a4 + 20 * v20 + 12) = v18;
          v24 = a4 + 20 * v20;
          *(_WORD *)v24 = v32;
          *(float *)(v24 + 16) = v19;
          *(float *)(v24 + 4) = (double)v36 * v18;
          *(float *)(v24 + 8) = (double)v37 * v19;
          v17 = (__int16)++v39;
        }
        while ( (__int16)v39 < v15 );
        v16 = v40;
      }
      v25 = va("*lightmap%d", v32);
      v26 = R_AllocImage(v25, 3553, (_WORD)v13 << 9, v16, 56, 3);
      if ( !R_CreateImageInternal(v26, 0, v34, 0xDE1u, 6408) )
      {
        HashValue = tr_image_generateHashValue((char *)v26);
        v28 = *(_DWORD *)(v26 + 116);
        --tr_numImages;
        v29 = glState_currentTmu;
        hashtable[HashValue] = v28;
        if ( glState_currentTextures[v29] == *(_DWORD *)(v26 + 88) )
        {
          glBindTexture(*(_DWORD *)(v26 + 84), 0);
          glState_currentTextures[glState_currentTmu] = 0;
        }
        qglDeleteTextures(1, v26 + 88);
        v26 = 0;
      }
      tr_lightmaps[v32] = v26;
      v30 = v15 + v33 < v38;
      v33 += v15;
      ++v32;
    }
    while ( v30 );
  }
  tr_lightmapCount = v32;
  return ri_Hunk_FreeTempMemory(v34);
}

/* ---- ShaderForShaderNum  0x004DA280 ----  VERIFIED */
char *__cdecl ShaderForShaderNum(__int16 a1, __int16 a2, const char *a3)
{
  int v3;
  char *result;

  if ( a1 < 0 || (v3 = a1, a1 >= s_worldData_numShaders) )
  {
    v3 = a1;
    ri_Error(1, "\x15" "ShaderForShaderNum: bad num %i", a1);
  }
  result = R_FindShader((char *)(s_worldData_shaders + 72 * v3), a2, 1, a3);
  if ( (result[76] & 1) != 0 )
    return tr_defaultShader;
  return result;
}

/* ---- LittleVertices_T2T2C4V3  0x004DA2E0 ----  VERIFIED */
void __cdecl LittleVertices_T2T2C4V3(int a1, int a2, int a3, float *a4, __int16 a5)
{
  int v5;
  int v6;
  int v7;
  int v8;
  float *v9;
  int v11;
  _WORD *v12;
  int v13;

  v5 = 0;
  if ( a3 > 0 )
  {
    v6 = 0;
    v13 = 0;
    v7 = a1 + 32;
    while ( 1 )
    {
      v8 = *(_DWORD *)(a2 + 48);
      if ( v8 )
      {
        *(float *)(v6 + v8) = *(float *)(v7 - 4);
        *(_DWORD *)(v6 + *(_DWORD *)(a2 + 48) + 4) = *(_DWORD *)v7;
        *(float *)(v6 + *(_DWORD *)(a2 + 48) + 8) = *(float *)(v7 + 4);
      }
      *(float *)(*(_DWORD *)(a2 + 52) + 8 * v5) = *(float *)(v7 - 20);
      *(float *)(*(_DWORD *)(a2 + 52) + 8 * v5 + 4) = *(float *)(v7 - 16);
      *(float *)(*(_DWORD *)(a2 + 56) + 8 * v5) = *(float *)(v7 - 12);
      *(float *)(*(_DWORD *)(a2 + 56) + 8 * v5 + 4) = *(float *)(v7 - 8);
      if ( a4 )
      {
        *(float *)(*(_DWORD *)(a2 + 56) + 8 * v5) = a4[3] * *(float *)(*(_DWORD *)(a2 + 56) + 8 * v5) + a4[1];
        v9 = (float *)(*(_DWORD *)(a2 + 56) + 8 * v5 + 4);
        *v9 = a4[4] * *v9 + a4[2];
      }
      *(_BYTE *)(*(_DWORD *)(a2 + 60) + 4 * v5) = *(_BYTE *)(v7 + 8);
      *(_BYTE *)(*(_DWORD *)(a2 + 60) + 4 * v5 + 1) = *(_BYTE *)(v7 + 9);
      *(_BYTE *)(*(_DWORD *)(a2 + 60) + 4 * v5 + 2) = *(_BYTE *)(v7 + 10);
      *(_BYTE *)(*(_DWORD *)(a2 + 60) + 4 * v5 + 3) = *(_BYTE *)(v7 + 11);
      *(float *)(v6 + *(_DWORD *)(a2 + 64)) = *(float *)(v7 - 32);
      *(float *)(*(_DWORD *)(a2 + 64) + v6 + 4) = *(float *)(v7 - 28);
      *(float *)(*(_DWORD *)(a2 + 64) + v6 + 8) = *(float *)(v7 - 24);
      AddPointToBounds((float *)(v6 + *(_DWORD *)(a2 + 64)), (float *)(a2 + 12), (float *)(a2 + 24));
      ++v5;
      v7 += 44;
      v13 += 12;
      if ( v5 >= a3 )
        break;
      v6 = v13;
    }
  }
  if ( *(_DWORD *)(a2 + 40) )
  {
    if ( *(_DWORD *)(a2 + 44) )
    {
      if ( *(_DWORD *)(a2 + 48) )
      {
        tess_numVertexes = *(_DWORD *)(a2 + 36);
        tess_numIndexes = *(_DWORD *)(a2 + 68);
        tess_stageTangentsValid = 0;
        tess_stageBitangentsValid = 0;
        tess_requiresVertexBasis = 1;
        Com_Memcpy((int)tess_stageNormals, *(int **)(a2 + 48), 12 * *(_DWORD *)(a2 + 36));
        Com_Memcpy((int)tess_texCoords0, *(int **)(a2 + 52), 8 * *(_DWORD *)(a2 + 36));
        Com_Memcpy((int)tess_xyz, *(int **)(a2 + 64), 12 * *(_DWORD *)(a2 + 36));
        v11 = 0;
        if ( *(int *)(a2 + 68) > 0 )
        {
          v12 = (_WORD *)(a2 + 72);
          do
            word_17A7F5E[++v11] = *v12++ - a5;
          while ( v11 < *(_DWORD *)(a2 + 68) );
        }
        RB_CalcTangentSpace();
        Com_Memcpy(*(_DWORD *)(a2 + 40), tess_stageTangents, 12 * *(_DWORD *)(a2 + 36));
        Com_Memcpy(*(_DWORD *)(a2 + 44), tess_stageBitangents, 12 * *(_DWORD *)(a2 + 36));
        tess_numVertexes = 0;
        tess_numIndexes = 0;
        tess_stageTangentsValid = 0;
        tess_stageBitangentsValid = 0;
        tess_requiresVertexBasis = 0;
      }
    }
  }
}

/* ---- OptimizeVertices_T2T2C4V3_GenericOrNV  0x004DA500 ----  [HIGH] */
int __cdecl OptimizeVertices_T2T2C4V3_GenericOrNV(_DWORD *a1, int a2)
{
  int result;
  int v3;
  int v4;

  result = a1[9];
  v3 = 0;
  if ( result > 0 )
  {
    v4 = 0;
    result = a2 + 8;
    do
    {
      *(_DWORD *)(result - 8) = *(_DWORD *)(a1[13] + 8 * v3);
      *(_DWORD *)(result - 4) = *(_DWORD *)(a1[13] + 8 * v3 + 4);
      *(_DWORD *)result = *(_DWORD *)(a1[14] + 8 * v3);
      *(_DWORD *)(result + 4) = *(_DWORD *)(a1[14] + 8 * v3 + 4);
      *(_BYTE *)(result + 8) = *(_BYTE *)(a1[15] + 4 * v3);
      *(_BYTE *)(result + 9) = *(_BYTE *)(a1[15] + 4 * v3 + 1);
      *(_BYTE *)(result + 10) = *(_BYTE *)(a1[15] + 4 * v3 + 2);
      *(_BYTE *)(result + 11) = *(_BYTE *)(a1[15] + 4 * v3 + 3);
      *(_DWORD *)(result + 12) = *(_DWORD *)(v4 + a1[16]);
      *(_DWORD *)(result + 16) = *(_DWORD *)(v4 + a1[16] + 4);
      *(_DWORD *)(result + 20) = *(_DWORD *)(v4 + a1[16] + 8);
      ++v3;
      v4 += 12;
      result += 32;
    }
    while ( v3 < a1[9] );
  }
  return result;
}

/* ---- OptimizeVertices_T2T2C4V3_ATI  0x004DA5A0 ----  [HIGH] */
int __cdecl OptimizeVertices_T2T2C4V3_ATI(_DWORD *a1, int a2, int a3)
{
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;

  v3 = 32 * a1[9];
  v4 = ri_Hunk_AllocateTempMemory(v3);
  v5 = 0;
  if ( (int)a1[9] > 0 )
  {
    v6 = 0;
    v7 = v4 + 8;
    do
    {
      *(_DWORD *)(v7 - 8) = *(_DWORD *)(a1[13] + 8 * v5);
      *(_DWORD *)(v7 - 4) = *(_DWORD *)(a1[13] + 8 * v5 + 4);
      *(_DWORD *)v7 = *(_DWORD *)(a1[14] + 8 * v5);
      *(_DWORD *)(v7 + 4) = *(_DWORD *)(a1[14] + 8 * v5 + 4);
      *(_BYTE *)(v7 + 8) = *(_BYTE *)(a1[15] + 4 * v5);
      *(_BYTE *)(v7 + 9) = *(_BYTE *)(a1[15] + 4 * v5 + 1);
      *(_BYTE *)(v7 + 10) = *(_BYTE *)(a1[15] + 4 * v5 + 2);
      *(_BYTE *)(v7 + 11) = *(_BYTE *)(a1[15] + 4 * v5 + 3);
      *(_DWORD *)(v7 + 12) = *(_DWORD *)(v6 + a1[16]);
      *(_DWORD *)(v7 + 16) = *(_DWORD *)(v6 + a1[16] + 4);
      *(_DWORD *)(v7 + 20) = *(_DWORD *)(v6 + a1[16] + 8);
      ++v5;
      v6 += 12;
      v7 += 32;
    }
    while ( v5 < a1[9] );
  }
  qglUpdateObjectBufferATI(a2, a3, v3, v4, 34658);
  return ri_Hunk_FreeTempMemory(v4);
}

/* ---- OptimizeVertices_T2T2C4V3_ARB  0x004DA660 ----  [HIGH] */
int __cdecl OptimizeVertices_T2T2C4V3_ARB(_DWORD *a1, int a2, int a3)
{
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;

  v3 = 32 * a1[9];
  v4 = ri_Hunk_AllocateTempMemory(v3);
  v5 = 0;
  if ( (int)a1[9] > 0 )
  {
    v6 = 0;
    v7 = v4 + 8;
    do
    {
      *(_DWORD *)(v7 - 8) = *(_DWORD *)(a1[13] + 8 * v5);
      *(_DWORD *)(v7 - 4) = *(_DWORD *)(a1[13] + 8 * v5 + 4);
      *(_DWORD *)v7 = *(_DWORD *)(a1[14] + 8 * v5);
      *(_DWORD *)(v7 + 4) = *(_DWORD *)(a1[14] + 8 * v5 + 4);
      *(_BYTE *)(v7 + 8) = *(_BYTE *)(a1[15] + 4 * v5);
      *(_BYTE *)(v7 + 9) = *(_BYTE *)(a1[15] + 4 * v5 + 1);
      *(_BYTE *)(v7 + 10) = *(_BYTE *)(a1[15] + 4 * v5 + 2);
      *(_BYTE *)(v7 + 11) = *(_BYTE *)(a1[15] + 4 * v5 + 3);
      *(_DWORD *)(v7 + 12) = *(_DWORD *)(v6 + a1[16]);
      *(_DWORD *)(v7 + 16) = *(_DWORD *)(v6 + a1[16] + 4);
      *(_DWORD *)(v7 + 20) = *(_DWORD *)(v6 + a1[16] + 8);
      ++v5;
      v6 += 12;
      v7 += 32;
    }
    while ( v5 < a1[9] );
  }
  qglBindBufferARB(34962, a2);
  qglBufferSubDataARB(34962, a3, v3, v4);
  qglBindBufferARB(34962, 0);
  return ri_Hunk_FreeTempMemory(v4);
}

/* ---- R_RefreshOptimizedWorldSurfaces_ARB  0x004DA740 ----  [HIGH] */
int R_RefreshOptimizedWorldSurfaces_ARB()
{
  int result;
  int v1;
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  unsigned __int16 *v7;
  int v8;
  int i;

  result = tr_world;
  v1 = 0;
  for ( i = 0; i < *(_DWORD *)(result + 156); ++i )
  {
    v2 = *(_DWORD *)(result + 160);
    v3 = *(_DWORD *)(v2 + v1 + 8);
    v4 = v2 + v1;
    if ( *(_DWORD *)v3 == 26 )
    {
      v5 = *(_DWORD *)(v3 + 68);
      v6 = *(unsigned __int16 *)(v3 + 72);
      if ( v5 > 1 )
      {
        v7 = (unsigned __int16 *)(v3 + 74);
        v8 = v5 - 1;
        do
        {
          if ( v6 > *v7 )
            v6 = *v7;
          ++v7;
          --v8;
        }
        while ( v8 );
      }
      OptimizeVertices_T2T2C4V3_ARB((_DWORD *)v3, *(_DWORD *)(*(_DWORD *)(v4 + 4) + 396), 32 * v6);
      result = tr_world;
    }
    v1 += 12;
  }
  return result;
}

/* ---- R_IncrementalRefreshOptimizedWorldSurfaces_ARB  0x004DA7D0 ----  [HIGH] */
void R_IncrementalRefreshOptimizedWorldSurfaces_ARB()
{
  int v0;
  int v1;
  int v2;
  int v3;
  int v4;
  unsigned __int16 *v5;
  int v6;

  if ( tr_world )
  {
    v0 = tr_worldRefreshSurfaceIndex;
    v1 = tr_worldRefreshSurfaceIndex;
    while ( 1 )
    {
      tr_worldRefreshSurfaceIndex = ++v0;
      if ( v0 == *(_DWORD *)(tr_world + 156) )
      {
        v0 = 0;
        tr_worldRefreshSurfaceIndex = 0;
      }
      if ( v0 == v1 )
        break;
      v2 = *(_DWORD *)(*(_DWORD *)(tr_world + 160) + 12 * v0 + 8);
      if ( *(_DWORD *)v2 == 26 )
      {
        v3 = *(_DWORD *)(v2 + 68);
        v4 = *(unsigned __int16 *)(v2 + 72);
        if ( v3 > 1 )
        {
          v5 = (unsigned __int16 *)(v2 + 74);
          v6 = v3 - 1;
          do
          {
            if ( v4 > *v5 )
              v4 = *v5;
            ++v5;
            --v6;
          }
          while ( v6 );
        }
        OptimizeVertices_T2T2C4V3_ARB(
          (_DWORD *)v2,
          *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(tr_world + 160) + 12 * v0 + 4) + 396),
          32 * v4);
        return;
      }
    }
  }
}

/* ---- AdjustColors  0x004DA860 ----  VERIFIED */
unsigned int __cdecl AdjustColors(int a1, double a2, int a3)
{
  unsigned int result;

  tess_numVertexes = *(_DWORD *)(a1 + 36);
  qmemcpy(tess_vertexColors, *(const void **)(a1 + 60), 4 * *(_DWORD *)(a1 + 36));
  RB_ComputeColors(*(_DWORD *)(*(_DWORD *)(a3 + 4) + 340));
  result = 4 * *(_DWORD *)(a1 + 36);
  qmemcpy(*(void **)(a1 + 60), tess_stageVertexColors, result);
  tess_numVertexes = 0;
  return result;
}

/* ---- CanOptimizeShader  0x004DA8D0 ----  VERIFIED */
int __cdecl CanOptimizeShader(int a1)
{
  int v1;
  int v2;
  int v3;
  char **i;
  char *v5;

  if ( r_optimizeWorld->integer )
  {
    if ( r_optimize->integer )
    {
      if ( (*(_BYTE *)(a1 + 76) & 1) == 0 && (*(_DWORD *)(a1 + 80) & 0x3FF5F900) == 0 )
      {
        v1 = *(_DWORD *)(a1 + 336);
        if ( v1 )
        {
          if ( *(_DWORD *)(a1 + 88) != 1065353216 )
          {
            v2 = 0;
            if ( v1 <= 0 )
              return 1;
            v3 = a1 + 340;
            for ( i = (char **)(a1 + 340); ; ++i )
            {
              v5 = *i;
              if ( **i >= 0
                && (!*((_DWORD *)v5 + 37)
                 || !*((_DWORD *)v5 + 87)
                 || *((_DWORD *)v5 + 409) != *(_DWORD *)(*(_DWORD *)v3 + 1636)
                 || *((_DWORD *)v5 + 415) != *(_DWORD *)(*(_DWORD *)v3 + 1660)) )
              {
                break;
              }
              if ( ++v2 >= v1 )
                return 1;
            }
          }
        }
      }
    }
  }
  return 0;
}


/* ---- BeginShaderSurfaces  0x004DA980 ----  [HIGH] */
int __cdecl BeginShaderSurfaces(int a1, int *a2, _DWORD *a3)
{
  int result;
  int v5;
  int v6;
  unsigned int v7;
  unsigned int v8;
  int v9;

  v6 = 0;

  if ( a1 >= 0x10000 )
    return ri_Error(1, "\x15" "surface with shader %s has more than 65,536 vertices\n", a3);
  v5 = CanOptimizeShader((int)a3);
  *a2 = v5;
  a2[1] = 32 * a1;
  if ( v5 )
  {
    a3[98] = 25;
    if ( glConfig_ARBVertexBufferObject )
    {
      result = R_CreateBufferARB(v6, 34962, a2[1], 0, 35044);
      a3[99] = result;
      if ( result )
      {
        a3[98] = 26;
        a2[3] = storageClass;
      }
    }
    else if ( glConfig_NVVertexArrayRange )
    {
      a3[98] = 28;
      v7 = (a2[1] + 31) & 0xFFFFFFE0;
      if ( (int)(tr_staticVertexMemoryPrimaryUsed + v7) >= tr_staticVertexMemoryPrimaryLimit )
      {
        if ( (int)(tr_staticVertexMemorySecondaryUsed + v7) >= tr_staticVertexMemorySecondaryLimit )
        {
          a3[99] = Hunk_AllocAlignInternal(v7, 32);
          result = 3;
          a2[3] = 3;
        }
        else
        {
          a3[99] = tr_staticVertexMemorySecondary + tr_staticVertexMemorySecondaryUsed;
          tr_staticVertexMemorySecondaryUsed += v7;
          result = 2;
          a2[3] = 2;
        }
      }
      else
      {
        a3[99] = tr_staticVertexMemoryPrimary + tr_staticVertexMemoryPrimaryUsed;
        tr_staticVertexMemoryPrimaryUsed += v7;
        result = 1;
        a2[3] = 1;
      }
    }
    else
    {
      result = glConfig_ATIVertexArrayObject;
      if ( glConfig_ATIVertexArrayObject )
      {
        v8 = (a2[1] + 31) & 0xFFFFFFE0;
        if ( (int)(tr_staticVertexMemorySecondaryUsed + v8) > tr_staticVertexMemorySecondaryLimit )
        {
          result = 0;
        }
        else
        {
          a3[100] = tr_staticVertexMemorySecondaryUsed;
          tr_staticVertexMemorySecondaryUsed += v8;
          result = 2;
        }
        a2[3] = result;
        if ( result == 2 )
        {
          a3[98] = 27;
          if ( a2[3] == 2 )
          {
            result = tr_staticVertexMemorySecondary;
            a3[99] = tr_staticVertexMemorySecondary;
          }
          else
          {
            a3[99] = tr_staticVertexMemoryPrimary;
          }
          a2[3] = 3;
        }
      }
    }
    if ( a3[98] == 25 )
    {
      v9 = a2[1];
      a2[3] = 3;
      result = ri_Hunk_Alloc(v9);
      a3[99] = result;
      a2[4] = (int)a3;
      a2[2] = 0;
      return result;
    }
  }
  else
  {
    a3[98] = 24;
    result = storageClass;
    a2[3] = storageClass;
  }
  a2[4] = (int)a3;
  a2[2] = 0;
  return result;
}

/* ---- BuildOptimizedSurface  0x004DAB70 ----  [HIGH] */
int __cdecl BuildOptimizedSurface(
        double a1,
        int a2,
        int a3,
        float *a4,
        int a5,
        int a6,
        int a7,
        _WORD *a8)
{
  int v9;
  _DWORD *v10;
  int v11;
  char *v12;
  char *v13;
  int v14;
  char *v15;
  int v16;
  __int16 *v17;
  __int16 v18;
  int v20;
  __int16 *v21;

  v20 = CanOptimizeShader(*(_DWORD *)(a2 + 4));
  v9 = 2 * a7 + 72;
  v10 = (_DWORD *)ri_Hunk_Alloc(v9 + 68 * a5);
  v10[10] = (char *)v10 + v9;
  v11 = (int)&v10[3 * a5] + v9;
  v10[11] = v11;
  v10[12] = 12 * a5 + v11;
  v12 = (char *)&v10[9 * a5] + v9;
  v10[16] = v12;
  v13 = &v12[12 * a5];
  v14 = a2;
  v10[13] = v13;
  v15 = &v13[8 * a5];
  v10[14] = v15;
  v10[15] = &v15[8 * a5];
  *v10 = *(_DWORD *)(*(_DWORD *)(a2 + 4) + 392);
  v10[1] = *(_DWORD *)(a3 + 12);
  v10[9] = a5;
  v10[17] = a7;
  v10[5] = 1216348160;
  v10[4] = 1216348160;
  v10[3] = 1216348160;
  v10[8] = -931135488;
  v10[7] = -931135488;
  v10[6] = -931135488;
  if ( *a8 )
    ri_Error(1, "\x15" "First index is not 0 in triangle soup surface");
  v16 = 0;
  if ( a7 > 0 )
  {
    v17 = (__int16 *)(v10 + 18);
    v21 = (__int16 *)(v10 + 18);
    do
    {
      if ( *v17 >= a5 )
      {
        ri_Error(1, "\x15" "Bad index in triangle soup surface");
        v17 = v21;
      }
      v18 = *(_WORD *)(a3 + 8) + a8[v16++];
      *v17++ = v18;
      v21 = v17;
    }
    while ( v16 < a7 );
  }
  LittleVertices_T2T2C4V3(a6, (int)v10, a5, a4, *(_DWORD *)(a3 + 8));
  if ( v20 )
  {
    if ( *(_DWORD *)(*(_DWORD *)(a2 + 4) + 336) )
    {
      AdjustColors((int)v10, a1, a2);
      v14 = a2;
    }
  }
  switch ( *v10 )
  {
    case 0x19:
    case 0x1C:
      OptimizeVertices_T2T2C4V3_GenericOrNV(
        v10,
        *(_DWORD *)(*(_DWORD *)(v14 + 4) + 396) + 32 * *(_DWORD *)(a3 + 8));
      break;
    case 0x1A:
      OptimizeVertices_T2T2C4V3_ARB(v10, *(_DWORD *)(*(_DWORD *)(v14 + 4) + 396), 32 * *(_DWORD *)(a3 + 8));
      break;
    case 0x1B:
      OptimizeVertices_T2T2C4V3_ATI(
        v10,
        *(_DWORD *)(*(_DWORD *)(v14 + 4) + 396),
        *(_DWORD *)(*(_DWORD *)(v14 + 4) + 400) + 32 * *(_DWORD *)(a3 + 8));
      break;
    default:
      break;
  }
  *((float *)v10 + 3) = *((float *)v10 + 3) - *(float *)(*(_DWORD *)(v14 + 4) + 332);
  *((float *)v10 + 4) = *((float *)v10 + 4) - *(float *)(*(_DWORD *)(v14 + 4) + 332);
  *((float *)v10 + 5) = *((float *)v10 + 5) - *(float *)(*(_DWORD *)(v14 + 4) + 332);
  *((float *)v10 + 6) = *(float *)(*(_DWORD *)(v14 + 4) + 332) + *((float *)v10 + 6);
  *((float *)v10 + 7) = *(float *)(*(_DWORD *)(v14 + 4) + 332) + *((float *)v10 + 7);
  *((float *)v10 + 8) = *(float *)(*(_DWORD *)(v14 + 4) + 332) + *((float *)v10 + 8);
  if ( *(_DWORD *)a3 )
    *(_DWORD *)(a3 + 8) += a5;
  *(_DWORD *)(v14 + 8) = v10;
  return 1;
}

/* ---- ParseTriangleSoup  0x004DADF0 ----  [HIGH] */
int __cdecl ParseTriangleSoup(
        int a1,
        int a2,
        int a3,
        double a4,
        int a5,
        int a6,
        int a7)
{
  int v7;
  __int16 v8;
  cvar_t *v9;
  float *v10;

  v7 = *(_DWORD *)(a2 + 16);
  v8 = *(_WORD *)(a1 + 2);
  v9 = r_singleShader;
  *(_DWORD *)(a3 + 4) = v7;
  if ( v9->integer )
  {
    if ( (*(_BYTE *)(v7 + 76) & 8) == 0 )
      *(_DWORD *)(a3 + 4) = tr_defaultShader;
  }
  if ( v8 < 0 )
    v10 = 0;
  else
    v10 = (float *)(a5 + 20 * v8);
  return BuildOptimizedSurface(
           a4,
           a3,
           a2,
           v10,
           *(__int16 *)(a1 + 8),
           a6 + 44 * *(_DWORD *)(a1 + 4),
           *(__int16 *)(a1 + 10),
           (_WORD *)(a7 + 2 * *(_DWORD *)(a1 + 12)));
}

/* ---- R_LoadSurfaces  0x004DAE60 ----  [CONFIRMED] */
int __cdecl R_LoadSurfaces(int a1, int a2, _DWORD *a3, double a4, int a5)
{
  int v8;
  int v9;
  unsigned int v10;
  void *v11;
  int v12;
  int v13;
  int v14;
  int v15;
  __int16 *v16;
  __int16 v17;
  char *Shader;
  char *v19;
  int v20;
  int v21;
  __int16 *v22;
  int v23;
  __int16 v24;
  cvar_t *v25;
  float *v26;
  __int16 v28;
  __int16 v29;
  int v30;
  int v31;
  _DWORD *v32;
  char *v33;
  int v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  int v40;
  int v41[5]; // [esp+48h] [ebp-30h] BYREF -- ONE record in retail; v42 == v41[4]
  struct tagMSG Msg; // [esp+5Ch] [ebp-1Ch] BYREF

  v34 = bspImageBase + *(_DWORD *)(a2 + 4);
  if ( (*(_BYTE *)a2 & 0xF) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v40 = bspImageBase + a3[1];
  v8 = *(_DWORD *)a2 >> 4;
  v30 = v8;
  if ( *a3 % 0x2Cu )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v39 = bspImageBase + *(_DWORD *)(a1 + 4);
  if ( (*(_BYTE *)a1 & 1) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v9 = v8;
  s_worldData_numsurfaces = v8;
  if ( !v8 )
  {
    ri_Error(1, "\x15" "LoadMap: no surfaces in %s", &s_worldData);
    v9 = s_worldData_numsurfaces;
  }
  v10 = 4 * v8;
  s_worldData_surfaces = ri_Hunk_Alloc(12 * v9);
  v38 = s_worldData_surfaces;
  v11 = (void *)ri_Hunk_AllocateTempMemory(v10);
  memset(v11, 0, v10);
  v12 = 0;
  v32 = v11;
  v33 = 0;
  v29 = 0;
  if ( s_worldData_numShaders > 0 )
  {
    v31 = 0;
    do
    {
      v28 = -1;
      if ( tr_lightmapCount > -1 )
      {
        v35 = -1;
        do
        {
          ++v12;
          v13 = 0;
          v14 = 0;
          v15 = 0;
          v37 = v12;
          if ( v30 > 0 )
          {
            v16 = (__int16 *)(v34 + 8);
            do
            {
              if ( *(v16 - 4) == v29 )
              {
                v17 = *(v16 - 3);
                if ( v17 >= 0 )
                  v17 = *(_WORD *)(a5 + 20 * v17);
                if ( v17 == v28 )
                {
                  v13 += *v16;
                  ++v14;
                  v32[v15] = v12;
                }
              }
              ++v15;
              v16 += 8;
            }
            while ( v15 < v30 );
            if ( v13 )
            {
              if ( v29 < 0 || v31 >= s_worldData_numShaders )
                ri_Error(1, "\x15" "ShaderForShaderNum: bad num %i", v31);
              Shader = R_FindShader((char *)(s_worldData_shaders + 72 * v31), v35, 1, (const char *)8);
              v19 = tr_defaultShader;
              if ( (Shader[76] & 1) == 0 )
                v19 = Shader;
              BeginShaderSurfaces(v13, v41, v19);
              if ( *((void (**)())v19 + 93) == RB_StageIteratorSky )
              {
                if ( v28 != -1 )
                  ri_Error(1, "\x15" "sky shader '%s' has a lightmap\n", v19);
                if ( v33 )
                  ri_Error(1, "\x15" "more than one sky shader: at least '%s' and '%s'\n", v19, v33);
                v33 = v19;
                s_worldData_skySurfaces = ri_Hunk_Alloc(4 * v14);
              }
              v20 = v38;
              v21 = 0;
              v36 = 0;
              v22 = (__int16 *)(v34 + 8);
              do
              {
                if ( v32[v21] == v37 )
                {
                  v23 = v41[4];
                  v24 = *(v22 - 3);
                  v25 = r_singleShader;
                  *(_DWORD *)(v20 + 4) = v41[4];
                  if ( v25->integer )
                  {
                    if ( (*(_BYTE *)(v23 + 76) & 8) == 0 )
                      *(_DWORD *)(v20 + 4) = tr_defaultShader;
                  }
                  if ( v24 < 0 )
                    v26 = 0;
                  else
                    v26 = (float *)(a5 + 20 * v24);
                  BuildOptimizedSurface(
                    a4,
                    v20,
                    (int)v41,
                    v26,
                    *v22,
                    v40 + 44 * *((_DWORD *)v22 - 1),
                    v22[1],
                    (_WORD *)(v39 + 2 * *((_DWORD *)v22 + 1)));
                  if ( *((void (**)())v19 + 93) == RB_StageIteratorSky )
                    *(_DWORD *)(s_worldData_skySurfaces + 4 * s_worldData_skySurfaceCount++) = v20;
                }
                v21 = v36 + 1;
                v22 += 8;
                v20 += 12;
                ++v36;
              }
              while ( v36 < v30 );
              v12 = v37;
            }
          }
          v35 = ++v28;
        }
        while ( v28 < tr_lightmapCount );
      }
      if ( PeekMessageA(&Msg, 0, 0x218u, 0x218u, 0) )
      {
        if ( GetMessageA(&Msg, 0, 0x218u, 0x218u) > 0 )
        {
          TranslateMessage(&Msg);
          DispatchMessageA(&Msg);
        }
      }
      SetThreadExecutionState(2u);
      v31 = ++v29;
    }
    while ( v29 < s_worldData_numShaders );
    v11 = v32;
  }
  return ri_Hunk_FreeTempMemory(v11);
}

/* ---- R_LoadSubmodels  0x004DB240 ----  [HIGH] */
int __cdecl R_LoadSubmodels(_DWORD *a1, const char *a2)
{
  int v3;
  int result;
  int v5;
  int v6;
  int v7;
  double v8;
  bool v9; // cc
  const char *v10;
  int size;
  signed int v12;

  v3 = bspImageBase + a1[1];
  if ( *a1 % 0x30u )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v12 = *a1 / 0x30u;
  result = ri_Hunk_Alloc(32 * v12);
  v5 = result;
  s_worldData_bmodels = result;
  size = 0;
  if ( v12 > 0 )
  {
    v10 = a2;
    do
    {
      if ( tr_numModels == 2048 )
      {
        v6 = 0;
      }
      else
      {
        v7 = ri_Hunk_Alloc(96);
        *(_DWORD *)(v7 + 68) = tr_numModels;
        tr_models[tr_numModels++] = v7;
        v6 = v7;
      }
      *(_DWORD *)(v6 + 64) = 1;
      *(_DWORD *)(v6 + 76) = v5;
      Com_sprintf((char *)v6, 64, "*%d", size);
      *(_DWORD *)v5 = *(_DWORD *)v3;
      *(float *)(v5 + 12) = *(float *)(v3 + 12);
      *(float *)(v5 + 4) = *(float *)(v3 + 4);
      v8 = *(float *)(v3 + 16);
      v3 += 48;
      *(float *)(v5 + 16) = v8;
      v5 += 32;
      *(float *)(v5 - 24) = *(float *)(v3 - 40);
      *(float *)(v5 - 12) = *(float *)(v3 - 28);
      *(_DWORD *)(v5 - 8) = s_worldData_surfaces + 12 * *(_DWORD *)(v3 - 24);
      result = size + 1;
      v9 = size + 1 < v12;
      *(_DWORD *)(v5 - 4) = *(_DWORD *)(v3 - 20);
      ++size;
    }
    while ( v9 );
  }
  return result;
}

/* ---- R_SetParentAndCell  0x004DB370 ----  VERIFIED */
int __cdecl R_SetParentAndCell(_DWORD *a1, int a2)
{
  int result;
  int v3;
  int v4;

  result = a2;
  a1[1] = a2;
  if ( *a1 == -1 )
  {
    R_SetParentAndCell((_DWORD *)a1[4], (int)a1);
    R_SetParentAndCell((_DWORD *)a1[5], (int)a1);
    v3 = a1[4];
    v4 = a1[5];
    a1[2] = -2;
    result = *(_DWORD *)(v3 + 8);
    if ( result == *(_DWORD *)(v4 + 8) )
      a1[2] = result;
  }
  return result;
}

/* ---- R_LoadNodesAndLeafs  0x004DB3C0 ----  [HIGH] */
int __cdecl R_LoadNodesAndLeafs(_DWORD *a1, _DWORD *a2)
{
  _DWORD *v3;
  signed int v4;
  int v5;
  _DWORD *v6;
  signed int v7;
  int v8;
  int v9;
  int result;
  int *v11;
  int *v12;
  signed int v13;
  int v14;
  int v15;
  int v16;
  bool v17; // zf
  int v18;
  int v19;
  signed int v20;

  v3 = (_DWORD *)(bspImageBase + a1[1]);
  if ( *a1 % 0x24u || *a2 % 0x24u )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v4 = *a1 / 0x24u;
  v5 = *a2 / 0x24u + v4;
  v20 = *a2 / 0x24u;
  v6 = (_DWORD *)ri_Hunk_Alloc(28 * v5);
  s_worldData_nodes = (int)v6;
  s_worldData_numnodes = v5;
  s_worldData_numDecisionNodes = v4;
  if ( v4 > 0 )
  {
    v7 = v4;
    do
    {
      v6[3] = ri_CM_GetPlaneNum(*v3);
      *v6 = -1;
      v8 = v3[1];
      if ( v8 < 0 )
        v6[4] = s_worldData_nodes + 28 * (v4 - v8 - 1);
      else
        v6[4] = s_worldData_nodes + 28 * v8;
      v9 = v3[2];
      if ( v9 < 0 )
        v6[5] = s_worldData_nodes + 28 * (v4 - v9 - 1);
      else
        v6[5] = s_worldData_nodes + 28 * v9;
      v3 += 9;
      v6 += 7;
      --v7;
    }
    while ( v7 );
  }
  result = v20;
  v11 = (int *)(bspImageBase + a2[1]);
  if ( v20 > 0 )
  {
    v12 = v6 + 5;
    v13 = v20;
    do
    {
      *(v12 - 3) = v11[6];
      v14 = *v11;
      *(v12 - 2) = *v11;
      if ( v14 >= s_worldData_numClusters )
        s_worldData_numClusters = v14 + 1;
      v15 = v11[7];
      *v12 = v15;
      result = v11[8];
      v12[1] = result;
      *(v12 - 1) = 0;
      if ( result )
      {
        if ( *(__int16 *)(s_worldData_lightIndexes + 2 * v15) < 0 )
        {
          --result;
          *v12 = v15 + 1;
          v12[1] = result;
          *(v12 - 1) = 1;
        }
      }
      if ( v12[1] > 15 )
        result = ri_Error(1, "\x15" "R_LoadNodesAndLeafs: too many lights in leaf. The map needs to be recompiled.");
      v11 += 9;
      v12 += 7;
      --v13;
    }
    while ( v13 );
  }
  v16 = s_worldData_nodes;
  v17 = *(_DWORD *)s_worldData_nodes == -1;
  *(_DWORD *)(s_worldData_nodes + 4) = 0;
  if ( v17 )
  {
    R_SetParentAndCell(*(_DWORD **)(v16 + 16), v16);
    R_SetParentAndCell(*(_DWORD **)(v16 + 20), v16);
    v18 = *(_DWORD *)(v16 + 16);
    v19 = *(_DWORD *)(v16 + 20);
    *(_DWORD *)(v16 + 8) = -2;
    result = *(_DWORD *)(v18 + 8);
    if ( result == *(_DWORD *)(v19 + 8) )
      *(_DWORD *)(v16 + 8) = result;
  }
  return result;
}

/* ---- R_LoadShaders  0x004DB5A0 ----  VERIFIED */
char *__cdecl R_LoadShaders(_DWORD *a1)
{
  const void *v2;
  signed int v3;
  char *result;

  v2 = (const void *)(bspImageBase + a1[1]);
  if ( *a1 % 0x48u )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *a1 / 0x48u;
  result = (char *)ri_Hunk_Alloc(72 * v3);
  s_worldData_shaders = (int)result;
  s_worldData_numShaders = v3;
  qmemcpy(result, v2, 4 * ((unsigned int)(72 * v3) >> 2));
  if ( v3 > 0 )
  {
    result += 68;
    do
    {
      result += 72;
      --v3;
    }
    while ( v3 );
  }
  return result;
}

/* ---- R_LoadLights  0x004DB620 ----  [HIGH] */
int __cdecl R_LoadLights(_DWORD *a1)
{
  int v2;
  signed int v3;
  int result;
  int v5;
  double v6;
  double v7;
  bool v11; // zf
  int v12;
  double v13;
  double v14;
  double v15;
  double v16;
  double v17;
  double v18;
  double v19;
  signed int v20;
  float v21;
  float v22;

  v2 = bspImageBase + a1[1];
  if ( *a1 % 0x48u )
    ri_Error(1, "\x15" "R_LoadLights: funny lump size in %s", &s_worldData);
  v3 = *a1 / 0x48u;
  result = ri_Hunk_Alloc(136 * v3);
  v5 = result;
  s_worldData_sunLight = 0;
  s_worldData_lights = result;
  s_worldData_lightCount = v3;
  if ( v3 > 0 )
  {
    v20 = v3;
    do
    {
      *(_DWORD *)v5 = *(_DWORD *)v2;
      v6 = *(float *)(v2 + 4) * tr_identityLight;
      v21 = *(float *)(v2 + 8) * tr_identityLight;
      v22 = *(float *)(v2 + 12) * tr_identityLight;
      v7 = 0.114 * v22 + 0.58700001 * v21 + 0.29899999 * v6;
      *(float *)(v5 + 16) = v7;
      if ( v7 != 0.0 )
        v7 = 1.0 / v7;
      v11 = *(_DWORD *)v5 == 1;
      *(float *)(v5 + 4) = v7 * v6;
      *(float *)(v5 + 8) = v21 * v7;
      *(float *)(v5 + 12) = v7 * v22;
      if ( v11 )
      {
        *(_DWORD *)(v5 + 20) = 0;
        *(_DWORD *)(v5 + 24) = 0;
        *(_DWORD *)(v5 + 28) = 0;
        *(_DWORD *)(v5 + 32) = 0;
        *(float *)(v5 + 36) = s_worldData_sunDiffuseColorR;
        *(float *)(v5 + 40) = s_worldData_sunDiffuseColorG;
        *(float *)(v5 + 44) = s_worldData_sunDiffuseColorB;
        *(_DWORD *)(v5 + 48) = s_worldData_sunDiffuseColorA;
        *(float *)(v5 + 16) = 0.114 * *(float *)(v5 + 44)
                            + 0.29899999 * *(float *)(v5 + 36)
                            + 0.58700001 * *(float *)(v5 + 40);
        s_worldData_sunLight = v5;
        s_worldData_entitySunLightIntensity = s_worldData_entityAmbientScaleB * 0.114 + s_worldData_entityAmbientScaleG * 0.58700001 + s_worldData_entityAmbientScaleR * 0.29899999;
      }
      else
      {
        *(_DWORD *)(v5 + 32) = 1065353216;
        *(_DWORD *)(v5 + 48) = 1065353216;
        *(float *)(v5 + 20) = 0.1 * v6;
        *(float *)(v5 + 24) = v21 * 0.1;
        *(float *)(v5 + 28) = v22 * 0.1;
        *(float *)(v5 + 36) = v6 * 0.80000001;
        *(float *)(v5 + 40) = v21 * 0.80000001;
        *(float *)(v5 + 44) = v22 * 0.80000001;
      }
      v12 = *(_DWORD *)v5 - 1;
      *(_DWORD *)(v5 + 52) = 0;
      *(_DWORD *)(v5 + 56) = 0;
      *(_DWORD *)(v5 + 60) = 0;
      *(_DWORD *)(v5 + 64) = 1065353216;
      *(_DWORD *)(v5 + 96) = 0;
      *(_DWORD *)(v5 + 100) = 0;
      *(_DWORD *)(v5 + 104) = 0;
      *(_DWORD *)(v5 + 112) = 1127481344;
      *(_DWORD *)(v5 + 108) = 0;
      switch ( v12 )
      {
        case 0:
          *(float *)(v5 + 68) = *(float *)(v2 + 28);
          *(float *)(v5 + 72) = *(float *)(v2 + 32);
          v13 = *(float *)(v2 + 36);
          *(_DWORD *)(v5 + 80) = 0;
          *(float *)(v5 + 76) = v13;
          *(_DWORD *)(v5 + 96) = 1065353216;
          break;
        case 1:
          *(float *)(v5 + 68) = *(float *)(v2 + 16);
          *(float *)(v5 + 72) = *(float *)(v2 + 20);
          v14 = *(float *)(v2 + 24);
          *(_DWORD *)(v5 + 80) = 1065353216;
          *(float *)(v5 + 76) = v14;
          *(_DWORD *)(v5 + 104) = 1065353216;
          break;
        case 2:
          *(float *)(v5 + 68) = *(float *)(v2 + 16);
          *(float *)(v5 + 72) = *(float *)(v2 + 20);
          v15 = *(float *)(v2 + 24);
          *(_DWORD *)(v5 + 80) = 1065353216;
          *(float *)(v5 + 76) = v15;
          *(float *)(v5 + 100) = *(float *)(v2 + 40);
          break;
        case 3:
          *(float *)(v5 + 68) = *(float *)(v2 + 16);
          *(float *)(v5 + 72) = *(float *)(v2 + 20);
          v16 = *(float *)(v2 + 24);
          *(_DWORD *)(v5 + 80) = 1065353216;
          *(float *)(v5 + 76) = v16;
          *(float *)(v5 + 104) = *(float *)(v2 + 40);
          *(float *)(v5 + 96) = *(float *)(v2 + 44);
          break;
        case 4:
          *(float *)(v5 + 68) = *(float *)(v2 + 16);
          *(float *)(v5 + 72) = *(float *)(v2 + 20);
          v17 = *(float *)(v2 + 24);
          *(_DWORD *)(v5 + 80) = 1065353216;
          *(_DWORD *)(v5 + 104) = 1065353216;
          *(float *)(v5 + 76) = v17;
          *(float *)(v5 + 84) = -*(float *)(v2 + 28);
          *(float *)(v5 + 88) = -*(float *)(v2 + 32);
          *(float *)(v5 + 92) = -*(float *)(v2 + 36);
          *(float *)(v5 + 112) = acos(*(float *)(v2 + 40)) * 57.29577791868205;
          v18 = (double)*(int *)(v2 + 44);
          goto LABEL_17;
        case 6:
          *(float *)(v5 + 68) = *(float *)(v2 + 16);
          *(float *)(v5 + 72) = *(float *)(v2 + 20);
          v19 = *(float *)(v2 + 24);
          *(_DWORD *)(v5 + 80) = 1065353216;
          *(float *)(v5 + 76) = v19;
          *(float *)(v5 + 104) = *(float *)(v2 + 40);
          *(float *)(v5 + 96) = *(float *)(v2 + 44);
          *(float *)(v5 + 84) = -*(float *)(v2 + 28);
          *(float *)(v5 + 88) = -*(float *)(v2 + 32);
          *(float *)(v5 + 92) = -*(float *)(v2 + 36);
          *(float *)(v5 + 112) = acos(*(float *)(v2 + 48)) * 57.29577791868205;
          v18 = (double)*(int *)(v2 + 52);
LABEL_17:
          *(float *)(v5 + 108) = v18;
          break;
        default:
          break;
      }
      v2 += 72;
      v5 += 136;
      result = --v20;
    }
    while ( v20 );
  }
  return result;
}

/* ---- R_LoadLightIndexes  0x004DB980 ----  VERIFIED */
_WORD *__cdecl R_LoadLightIndexes(int a1)
{
  int v2;
  int v3;
  _WORD *result;
  int v5;

  v2 = bspImageBase + *(_DWORD *)(a1 + 4);
  if ( (*(_BYTE *)a1 & 1) != 0 )
    ri_Error(1, "\x15" "R_LoadLightIndexes: funny lump size in %s", &s_worldData);
  v3 = *(_DWORD *)a1 >> 1;
  result = (_WORD *)ri_Hunk_Alloc(2 * v3);
  s_worldData_lightIndexes = (int)result;
  s_worldData_lightIndexCount = v3;
  if ( v3 > 0 )
  {
    v5 = v2 - (_DWORD)result;
    do
    {
      *result = *(_WORD *)((char *)result + v5);
      ++result;
      --v3;
    }
    while ( v3 );
  }
  return result;
}

/* ---- R_LoadLightVisCache  0x004DB9E0 ----  VERIFIED */
int __cdecl R_LoadLightVisCache(int *a1)
{
  int v1;
  int result;
  int v3;

  v1 = a1[1];
  result = *a1;
  v3 = bspImageBase + v1;
  if ( result > 0 )
  {
    result = R_InitLightVisCacheFromBuffer(v3, result);
    if ( !result )
      return ri_Error(1, "\x15" "R_LoadLightVisCache: funny lump size in %s", &s_worldData);
  }
  return result;
}

/* ---- R_ValueForKey  0x004DBA20 ----  VERIFIED */
int __cdecl R_ValueForKey(int a1, char *String2, int a3)
{
  int v3;
  const char *i;

  v3 = 1;
  if ( a1 <= 1 )
    return 0;
  for ( i = (const char *)(a3 + 4096); _stricmp(i, String2); i += 4096 )
  {
    if ( ++v3 >= a1 )
      return 0;
  }
  return (v3 << 12) + a3 + 2048;
}

/* ---- R_FloatForKey  0x004DBA80 ----  VERIFIED */
double __cdecl R_FloatForKey(char *String2, int a2, int a3, float a4)
{
  const char *v4;

  v4 = (const char *)R_ValueForKey(a3, String2, a2);
  if ( v4 )
    return atof(v4);
  else
    return a4;
}

/* ---- R_VectorForKey  0x004DBAA0 ----  VERIFIED */
int __cdecl R_VectorForKey(char *String2, int a2, int a3, int a4, const char *a5)
{
  int v5;
  const char *v6;

  v5 = 1;
  v6 = (const char *)R_ValueForKey(a3, String2, a2);
  if ( !v6 )
  {
    v6 = a5;
    v5 = 0;
  }
  sscanf(v6, "%f %f %f", a4, a4 + 4, a4 + 8);
  return v5;
}

/* ---- R_LoadMiscModel  0x004DBAE0 ----  [HIGH] */
void __cdecl R_LoadMiscModel(int a1, int a2)
{
  int v3;
  const char *v4;
  char *v5;
  const char *v6;
  double v7;
  const char *v8;
  int v9;
  const char *v10;
  const char *v11;
  double v12;
  const char *v13;
  float v15[3]; // [esp+24h] [ebp-30h] BYREF -- angles
  float v18[3]; // [esp+30h] [ebp-24h] BYREF -- origin
  float v21[3]; // [esp+3Ch] [ebp-18h] BYREF -- modelscale_vec
  float v24[3]; // [esp+48h] [ebp-Ch]  BYREF -- lightingPrecalc

  v3 = 1;
  v4 = (const char *)R_ValueForKey(a1, "origin", a2);
  if ( !v4 )
  {
    v3 = 0;
    v4 = "0 0 0";
  }
  sscanf(v4, "%f %f %f", &v18[0], &v18[1], &v18[2]);
  if ( !v3 )
    ri_Error(1, "\x15" "R_LoadMiscModel: no origin specified\n");
  v5 = (char *)R_ValueForKey(a1, "model", a2);
  if ( !v5 )
    ri_Error(1, "\x15" "R_LoadMiscModel: no model specified in misc_model at (%.0f %.0f %.0f)\n", v18[0], v18[1], v18[2]);
  if ( _strnicmp(v5, "xmodel/shadow_", 0xEu) )
  {
    v6 = (const char *)R_ValueForKey(a1, "angle", a2);
    if ( !v6 || (v7 = atof(v6), (v7 == 0.0) | __UNORDERED__(v7, 0.0)) )
    {
      v8 = (const char *)R_ValueForKey(a1, "angles", a2);
      if ( !v8 )
        v8 = "0 0 0";
      sscanf(v8, "%f %f %f", &v15[0], &v15[1], &v15[2]);
    }
    else
    {
      v15[1] = v7;
      v15[0] = 0;
      v15[2] = 0;
    }
    v9 = 1;
    v10 = (const char *)R_ValueForKey(a1, "modelscale_vec", a2);
    if ( !v10 )
    {
      v9 = 0;
      v10 = "1 1 1";
    }
    sscanf(v10, "%f %f %f", &v21[0], &v21[1], &v21[2]);
    if ( !v9 )
    {
      v11 = (const char *)R_ValueForKey(a1, "modelscale", a2);
      if ( v11 )
        v12 = atof(v11);
      else
        v12 = 1.0;
      v21[0] = v12;
      v21[1] = v12;
      v21[2] = v12;
    }
    v13 = (const char *)R_ValueForKey(a1, "lightingPrecalc", a2);
    if ( !v13 )
      v13 = "1 1 1";
    sscanf(v13, "%f %f %f", &v24[0], &v24[1], &v24[2]);
    R_CreateStaticModel(v21, v5, (int)&s_worldData, v18, v15, v24);
  }
}

/* ---- R_LoadCorona  0x004DBCD0 ----  VERIFIED */
unsigned __int64 __cdecl R_LoadCorona(int a1, int a2)
{
  char *v3;
  int v4;
  const char *v5;
  const char *v6;
  double v7;
  const char *v8;
  double v9;
  const char *v10;
  double v11;
  const char *v12;
  double v13;
  double v14;
  double v15;
  unsigned __int64 result; // rax
  float v17;
  float v18; // [esp+14h] [ebp-Ch] BYREF
  float v19; // [esp+18h] [ebp-8h] BYREF
  float v20; // [esp+1Ch] [ebp-4h] BYREF

  if ( s_worldData_coronaCount >= 512 )
  {
    v3 = va("\x15" "MAX_MAP_CORONAS(%i) exceeded", 512);
    ri_Error(1, v3);
  }
  v4 = s_worldData_coronas + 32 * s_worldData_coronaCount++;
  *(_DWORD *)v4 = R_FindShader("flareShader", -1, 1, (const char *)4);
  v5 = (const char *)R_ValueForKey(a1, "origin", a2);
  if ( !v5 )
    v5 = "0 0 0";
  sscanf(v5, "%f %f %f", v4 + 4, v4 + 8, v4 + 12);
  v6 = (const char *)R_ValueForKey(a1, "scale", a2);
  if ( v6 )
  {
    v7 = atof(v6);
    v17 = v7;
    if ( v7 <= 0.0 )
      ri_Error(1, "\x15" "corona scale must be > 0");
  }
  else
  {
    v17 = 1.0;
  }
  *(float *)(v4 + 16) = v17 * 25.5;
  v8 = (const char *)R_ValueForKey(a1, "zcutoff", a2);
  if ( v8 )
    v9 = atof(v8);
  else
    v9 = -0.15000001;
  *(float *)(v4 + 20) = v9;
  v10 = (const char *)R_ValueForKey(a1, "zfadeout", a2);
  if ( v10 )
    v11 = atof(v10);
  else
    v11 = -0.25;
  *(float *)(v4 + 24) = v11;
  v12 = (const char *)R_ValueForKey(a1, "dl_color", a2);
  if ( !v12 )
    v12 = "1 1 1";
  sscanf(v12, "%f %f %f", &v18, &v19, &v20);
  v13 = v18 * 255.0;
  if ( v13 >= 0.0 && (v13 > 255.0) | __UNORDERED__(255.0, v13) )
  {
    v13 = 255.0;
  }
  else if ( v13 < 0.0 )
  {
    v13 = 0.0;
  }
  *(_BYTE *)(v4 + 28) = (unsigned __int64)v13;
  v14 = v19 * 255.0;
  if ( v14 >= 0.0 && (v14 > 255.0) | __UNORDERED__(255.0, v14) )
  {
    v14 = 255.0;
  }
  else if ( v14 < 0.0 )
  {
    v14 = 0.0;
  }
  *(_BYTE *)(v4 + 29) = (unsigned __int64)v14;
  v15 = v20 * 255.0;
  if ( v15 >= 0.0 && (v15 > 255.0) | __UNORDERED__(255.0, v15) )
  {
    result = (unsigned __int64)255.0;
    *(_BYTE *)(v4 + 30) = (unsigned __int64)255.0;
    *(_BYTE *)(v4 + 31) = -1;
  }
  else
  {
    if ( v15 < 0.0 )
      v15 = 0.0;
    result = (unsigned __int64)v15;
    *(_BYTE *)(v4 + 30) = (unsigned __int64)v15;
    *(_BYTE *)(v4 + 31) = -1;
  }
  return result;
}

/* ---- R_LoadEntities  0x004DBF50 ----  VERIFIED */
char __cdecl R_LoadEntities(_DWORD *a1)
{
  parseInfo_t *v1;
  int ungetLineSave;
  char result;
  int v4;
  parseInfo_t *v5;
  int v6;
  char *v7;
  parseInfo_t *v8;
  int v9;
  char *v10;
  double v11;
  double v12;
  double v13;
  double v14;
  double v15;
  double v16;
  parseInfo_t *v17;
  int v18;
  parseInfo_t *v19;
  int v20;
  char *v21;
  parseInfo_t *v22;
  int v23;
  int v24;
  char *v25;
  char v26;
  parseInfo_t *v27;
  int v28;
  int v29;
  char *data_p; // [esp+10h] [ebp-45048h] BYREF
  int v34;
  float v35[3]; // [ebp-45040h] BYREF -- sundiffusecolor
  float v38[3]; // [ebp-45034h] BYREF -- suncolor
  float v41;  /* diffuseFraction */
  float v42[3]; // [ebp-45024h] BYREF -- _color (ambient colour)
  float v45;
  float v46[3]; // [esp+44h] [ebp-45014h] BYREF -- ONE vec3_t in retail
  char Destination[2048]; // [esp+50h] [ebp-45008h] BYREF
  char String[2048]; // [esp+850h] [ebp-44808h] BYREF
  int v51[4096]; // [esp+1050h] [ebp-44008h] BYREF
  char v52[64 * 4096]; // [esp+5050h] [ebp-40008h] BYREF -- ONE object in retail
  char *String1 = v52 + 2048;  /* interior alias, the value column */
  unsigned int v54;
  unsigned int retaddr;

  v54 = retaddr ^ _security_cookie;
  v29 = *a1 + 1;
  data_p = (char *)(bspImageBase + a1[1]);
  entityParseBufferHead = ri_Hunk_Alloc(v29);
  strcpy((char *)entityParseBufferHead, data_p);
  v1 = parseInfo;
  entityParseCursor = (char *)entityParseBufferHead;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    ungetLineSave = v1->ungetLineSave;
    data_p = v1->ungetTokenSave;
    v1->currentLine = ungetLineSave;
  }
  result = *Com_ParseExt(&data_p, qtrue);
  if ( result == 123 )
  {
    v4 = 0;
    *(float *)&v34 = 0.0;
    v45 = 1.0;
    v41 = 0.5;
    v42[2] = 0.0;
    v42[1] = 0.0;
    v42[0] = 0.0;
    v38[2] = 0.0;
    v38[1] = 0.0;
    v38[0] = 0.0;
    v35[2] = 0.0;
    v35[1] = 0.0;
    v35[0] = 0.0;
    while ( 1 )
    {
      v5 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        v6 = v5->ungetLineSave;
        data_p = v5->ungetTokenSave;
        v5->currentLine = v6;
      }
      v7 = Com_ParseExt(&data_p, qtrue);
      if ( !*v7 )
        break;
      if ( *v7 == 125 )
        break;
      strncpy(Destination, v7, 0x7FFu);
      v8 = parseInfo;
      Destination[2047] = 0;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        v9 = v8->ungetLineSave;
        data_p = v8->ungetTokenSave;
        v8->currentLine = v9;
      }
      v10 = Com_ParseExt(&data_p, qtrue);
      if ( !*v10 || *v10 == 125 )
        break;
      strncpy(String, v10, 0x7FFu);
      String[2047] = 0;
      if ( Q_stricmpn(Destination, "ambient", 99999) )
      {
        if ( Q_stricmpn(Destination, "_color", 99999) )
        {
          if ( Q_stricmpn(Destination, "diffuseFraction", 99999) )
          {
            if ( Q_stricmpn(Destination, "suncolor", 99999) )
            {
              if ( Q_stricmpn(Destination, "sundiffusecolor", 99999) )
              {
                if ( Q_stricmpn(Destination, "sunlight", 99999) )
                {
                  if ( !Q_stricmpn(Destination, "sundirection", 99999) )
                  {
                    sscanf(String, "%f %f %f", &v46[0], &v46[1], &v46[2]);
                    AngleVectors(v46, (float *)&tr_sunDirection, 0, 0);
                  }
                }
                else
                {
                  v45 = atof(String);
                }
              }
              else
              {
                sscanf(String, "%f %f %f", &v35[0], &v35[1], &v35[2]);
                NormalizeColorWhite_m(v35, v35);
                v4 = 1;
              }
            }
            else
            {
              sscanf(String, "%f %f %f", &v38[0], &v38[1], &v38[2]);
              NormalizeColorWhite_m(v38, v38);
            }
          }
          else
          {
            v41 = atof(String);
          }
        }
        else
        {
          sscanf(String, "%f %f %f", &v42[0], &v42[1], &v42[2]);
        }
      }
      else
      {
        v11 = atof(String);
        *(float *)&v34 = v11;
        if ( v11 > 2.0 )
        {
          ri_Printf(
            0,
            "^3WARNING: ambient too big, assuming it uses the old 0-255 scale instead of the proper 0-1 scale (value = '%s')\n",
            String);
          *(float *)&v34 = *(float *)&v34 * 0.015686275;
        }
      }
    }
    s_worldData_entityAmbientBaseA = 1065353216;
    if ( !((*(float *)&v34 == 0.0) | __UNORDERED__(*(float *)&v34, 0.0)) )
    {
      v12 = NormalizeColorWhite_m(v42, v42);
      if ( !((v12 == 0.0) | __UNORDERED__(v12, 0.0)) )
      {
        v13 = tr_identityLight * *(float *)&v34;
        s_worldData_entityAmbientBaseR = v42[0] * v13;
        s_worldData_entityAmbientBaseG = v42[1] * v13;
        s_worldData_entityAmbientBaseB = v13 * v42[2];
        glLightModelfv(0xB53u, &s_worldData_entityAmbientBaseR);
      }
    }
    if ( !v4 )
    {
      v35[0] = v38[0];
      v35[1] = v38[1];
      v35[2] = v38[2];
    }
    s_worldData_sunDiffuseColorA = 1065353216;
    s_worldData_entityAmbientScaleA = 1065353216;
    s_worldData_coronas = (int)v51;
    v14 = (v45 - *(float *)&v34) * tr_identityLight;
    s_worldData_coronaCount = 0;
    v15 = (1.0 - v41) * v14;
    s_worldData_sunDiffuseColorR = v38[0] * v15;
    s_worldData_sunDiffuseColorG = v38[1] * v15;
    s_worldData_sunDiffuseColorB = v15 * v38[2];
    v16 = v14 * v41;
    s_worldData_entityAmbientScaleR = v35[0] * v16;
    s_worldData_entityAmbientScaleG = v35[1] * v16;
    s_worldData_entityAmbientScaleB = v16 * v35[2];
    while ( 1 )
    {
      v17 = parseInfo;
      if ( parseInfo->ungetReady )
      {
        parseInfo->ungetReady = qfalse;
        v18 = v17->ungetLineSave;
        data_p = v17->ungetTokenSave;
        v17->currentLine = v18;
      }
      if ( *Com_ParseExt(&data_p, qtrue) != 123 )
        break;
      v52[0] = 0;
      v34 = 1;
      while ( 1 )
      {
        v19 = parseInfo;
        if ( parseInfo->ungetReady )
        {
          parseInfo->ungetReady = qfalse;
          v20 = v19->ungetLineSave;
          data_p = v19->ungetTokenSave;
          v19->currentLine = v20;
        }
        v21 = Com_ParseExt(&data_p, qtrue);
        if ( !*v21 || *v21 == 125 )
          break;
        if ( !strcmp(v21, "classname") )
        {
          strcpy(v52, v21);
          v22 = parseInfo;
          if ( parseInfo->ungetReady )
          {
            parseInfo->ungetReady = qfalse;
            v23 = v22->ungetLineSave;
            data_p = v22->ungetTokenSave;
            v22->currentLine = v23;
          }
          strcpy(String1, Com_ParseExt(&data_p, qtrue));
        }
        else
        {
          v24 = v34;
          if ( v34 == 64 )
            ri_Error(1, "\x15" "R_LoadEntities: MAX_SPAWN_VARS (%i) reached\n", 64);
          v25 = v21;
          do
          {
            v26 = *v25;
            v52[(v24 << 12) - (_DWORD)v21 + (_DWORD)v25] = *v25;
            ++v25;
          }
          while ( v26 );
          v27 = parseInfo;
          if ( parseInfo->ungetReady )
          {
            parseInfo->ungetReady = qfalse;
            v28 = v27->ungetLineSave;
            data_p = v27->ungetTokenSave;
            v27->currentLine = v28;
          }
          strcpy(&String1[4096 * v24], Com_ParseExt(&data_p, qtrue));
          v34 = v24 + 1;
        }
      }
      if ( !v52[0] )
        ri_Error(1, "\x15" "R_LoadEntities: entity without a classname\n");
      if ( !_stricmp(String1, "misc_model") )
      {
        R_LoadMiscModel(v34, (int)v52);
      }
      else if ( !_stricmp(String1, "corona") )
      {
        R_LoadCorona(v34, (int)v52);
      }
    }
    result = s_worldData_coronaCount;
    if ( s_worldData_coronaCount )
    {
      s_worldData_coronas = ri_Hunk_Alloc(32 * s_worldData_coronaCount);
      Com_Memcpy(s_worldData_coronas, v51, 32 * s_worldData_coronaCount);
      return result;
    }
    else
    {
      s_worldData_coronas = 0;
    }
  }
  return result;
}

/* ---- R_GetEntityToken  0x004DC6D0 ----  VERIFIED */
int __cdecl R_GetEntityToken(char *Destination, int a2)
{
  parseInfo_t *v2;
  char *v3;

  v2 = parseInfo;
  if ( parseInfo->ungetReady )
  {
    parseInfo->ungetReady = qfalse;
    entityParseCursor = v2->ungetTokenSave;
    v2->currentLine = v2->ungetLineSave;
  }
  v3 = Com_ParseExt(&entityParseCursor, qtrue);
  strncpy(Destination, v3, a2 - 1);
  Destination[a2 - 1] = 0;
  if ( entityParseCursor && *v3 )
    return 1;
  entityParseCursor = (char *)entityParseBufferHead;
  return 0;
}

/* ---- R_FinishLoadingAABBTrees_r  0x004DC760 ----  [HIGH] */
int __cdecl R_FinishLoadingAABBTrees_r(int a1, int a2)
{
  float *v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v9;
  bool v10; // cc
  int v11;
  int v12;
  int i;

  *(_DWORD *)(a1 + 8) = 1216348160;
  *(_DWORD *)(a1 + 4) = 1216348160;
  *(_DWORD *)a1 = 1216348160;
  v3 = (float *)(a1 + 12);
  *(_DWORD *)(a1 + 20) = -931135488;
  *(_DWORD *)(a1 + 16) = -931135488;
  *(_DWORD *)(a1 + 12) = -931135488;
  v4 = *(_DWORD *)(a1 + 36);
  v5 = 0;
  if ( v4 )
  {
    v9 = v4 + a2;
    *(_DWORD *)(a1 + 32) = s_worldData_aabbTrees + 40 * a2;
    v12 = 0;
    if ( v4 > 0 )
    {
      for ( i = 0; ; v5 = i )
      {
        v9 = R_FinishLoadingAABBTrees_r(v5 + *(_DWORD *)(a1 + 32), v9);
        ExpandBounds((float *)(*(_DWORD *)(a1 + 32) + v5), (float *)a1, (float *)(*(_DWORD *)(a1 + 32) + v5 + 12), v3);
        v10 = ++v12 < *(_DWORD *)(a1 + 36);
        i += 40;
        if ( !v10 )
          break;
      }
    }
    return v9;
  }
  else
  {
    v6 = 0;
    if ( *(int *)(a1 + 28) > 0 )
    {
      v11 = 0;
      do
      {
        v7 = *(_DWORD *)(v11 + *(_DWORD *)(a1 + 24) + 8);
        ExpandBounds((float *)(v7 + 12), (float *)a1, (float *)(v7 + 24), v3);
        ++v6;
        v11 += 12;
      }
      while ( v6 < *(_DWORD *)(a1 + 28) );
    }
    return a2;
  }
}

/* ---- R_LoadAABBTrees  0x004DC850 ----  VERIFIED */
int __cdecl R_LoadAABBTrees(_DWORD *a1)
{
  int v2;
  signed int v3;
  int v4;
  _DWORD *v5;
  _DWORD *v6;
  signed int v7;
  int result;

  v2 = bspImageBase + a1[1];
  if ( *a1 % 0xCu )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *a1 / 0xCu;
  v4 = ri_Hunk_Alloc(40 * v3);
  s_worldData_aabbTrees = v4;
  s_worldData_aabbTreeCount = v3;
  if ( v3 > 0 )
  {
    v5 = (_DWORD *)(v2 + 8);
    v6 = (_DWORD *)(v4 + 28);
    v7 = v3;
    do
    {
      *(v6 - 1) = s_worldData_surfaces + 12 * *(v5 - 2);
      *v6 = *(v5 - 1);
      v6[2] = *v5;
      v5 += 3;
      v6 += 10;
      --v7;
    }
    while ( v7 );
  }
  for ( result = 0; result < v3; result = R_FinishLoadingAABBTrees_r(s_worldData_aabbTrees + 40 * result, result + 1) )
    ;
  return result;
}

/* ---- R_LoadCells  0x004DC910 ----  VERIFIED */
int __cdecl R_LoadCells(_DWORD *a1)
{
  int v2;
  signed int v3;
  int result;
  int v5;
  double v6;

  v2 = bspImageBase + a1[1];
  if ( *a1 % 0x34u )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *a1 / 0x34u;
  result = ri_Hunk_Alloc(v3 << 6);
  s_worldData_cells = result;
  s_worldData_cellCount = v3;
  if ( v3 > 0 )
  {
    v5 = result + 8;
    result = v2 + 8;
    do
    {
      v6 = *(float *)(result - 8);
      result += 52;
      *(float *)(v5 - 8) = v6;
      v5 += 64;
      --v3;
      *(float *)(v5 - 68) = *(float *)(result - 56);
      *(_DWORD *)(v5 - 64) = *(_DWORD *)(result - 52);
      *(float *)(v5 - 60) = *(float *)(result - 48);
      *(float *)(v5 - 56) = *(float *)(result - 44);
      *(float *)(v5 - 52) = *(float *)(result - 40);
      *(_DWORD *)(v5 - 48) = s_worldData_aabbTrees + 40 * *(_DWORD *)(result - 36);
      *(_DWORD *)(v5 - 44) = s_worldData_portals + 36 * *(_DWORD *)(result - 32);
      *(_DWORD *)(v5 - 40) = *(_DWORD *)(result - 28);
      *(_DWORD *)(v5 - 36) = s_worldData_cullGroupIndexes + 4 * *(_DWORD *)(result - 24);
      *(_DWORD *)(v5 - 32) = *(_DWORD *)(result - 20);
      *(_DWORD *)(v5 - 28) = s_worldData_occluderIndexes + 4 * *(_DWORD *)(result - 16);
      *(_DWORD *)(v5 - 24) = *(_DWORD *)(result - 12);
    }
    while ( v3 );
  }
  return result;
}

/* ---- R_LoadPortalVerts  0x004DCA00 ----  VERIFIED */
int __cdecl R_LoadPortalVerts(_DWORD *a1)
{
  int v2;
  signed int v3;
  int result;
  int v5;
  float *v6;
  int v7;
  double v8;

  v2 = bspImageBase + a1[1];
  if ( *a1 % 0xCu )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *a1 / 0xCu;
  result = ri_Hunk_Alloc(12 * v3);
  s_worldData_portalVerts = result;
  if ( v3 > 0 )
  {
    v5 = v2;
    v6 = (float *)(result + 4);
    v7 = v2 - result;
    do
    {
      result = *(_DWORD *)v5;
      *(v6 - 1) = *(float *)v5;
      *v6 = *(float *)((char *)v6 + v7);
      v8 = *(float *)(v5 + 8);
      v5 += 12;
      v6[1] = v8;
      v6 += 3;
      --v3;
    }
    while ( v3 );
  }
  return result;
}

/* ---- R_LoadPortals  0x004DCA80 ----  VERIFIED */
int __cdecl R_LoadPortals(int a1)
{
  int v2;
  int v3;
  int v4;
  _DWORD *v5;
  int v6;
  _DWORD *v7;
  int result;
  int v9;
  int v10;

  v2 = bspImageBase + *(_DWORD *)(a1 + 4);
  if ( (*(_BYTE *)a1 & 0xF) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *(_DWORD *)a1 >> 4;
  v4 = ri_Hunk_Alloc(36 * v3);
  s_worldData_portals = v4;
  s_worldData_portalCount = v3;
  if ( v3 > 0 )
  {
    v5 = (_DWORD *)(v2 + 8);
    v6 = v4 + 4;
    do
    {
      v7 = (_DWORD *)ri_CM_GetPlaneNum(*(v5 - 2));
      *(_DWORD *)(v6 - 4) = *v7;
      *(_DWORD *)v6 = v7[1];
      *(_DWORD *)(v6 + 4) = v7[2];
      *(_DWORD *)(v6 + 8) = v7[3];
      LOBYTE(v7) = *(int *)(v6 - 4) <= 0;
      *(float *)(v6 + 8) = *(float *)(v6 + 8) - 0.001;
      *(_BYTE *)(v6 + 12) = ((_BYTE)v7 - 1) & 0xC;
      *(_BYTE *)(v6 + 13) = *(int *)v6 <= 0 ? 4 : 16;
      *(_BYTE *)(v6 + 14) = *(int *)(v6 + 4) <= 0 ? 8 : 20;
      *(_DWORD *)(v6 + 16) = s_worldData_cells + (*(v5 - 1) << 6);
      *(_DWORD *)(v6 + 20) = s_worldData_portalVerts + 12 * *v5;
      *(_DWORD *)(v6 + 24) = v5[1];
      *(_DWORD *)(v6 + 28) = 0;
      v5 += 4;
      v6 += 36;
      --v3;
    }
    while ( v3 );
  }
  result = s_worldData_cellCount;
  v9 = 0;
  if ( s_worldData_cellCount > 0 )
  {
    v10 = 0;
    do
    {
      *(_DWORD *)(v10 + s_worldData_cells + 28) = s_worldData_portals + 36 * (*(_DWORD *)(v10 + s_worldData_cells + 28) / 36);
      result = s_worldData_cellCount;
      ++v9;
      v10 += 64;
    }
    while ( v9 < s_worldData_cellCount );
  }
  return result;
}

/* ---- R_LoadCullGroups  0x004DCBD0 ----  [HIGH] */
int __cdecl R_LoadCullGroups(int a1)
{
  int v2;
  int v3;
  int result;
  int v5;
  int v6;
  int v7;
  int v8;
  double v9;

  v2 = bspImageBase + *(_DWORD *)(a1 + 4);
  if ( (*(_BYTE *)a1 & 0x1F) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *(_DWORD *)a1 >> 5;
  result = ri_Hunk_Alloc(36 * v3);
  s_worldData_cullGroups = result;
  s_worldData_cullGroupCount = v3;
  if ( v3 > 0 )
  {
    v5 = result + 28;
    v6 = result + 4;
    v7 = v2 + 28;
    result = v2 + 4;
    v8 = v3;
    do
    {
      v9 = *(float *)(result - 4);
      result += 32;
      *(float *)(v6 - 4) = v9;
      v6 += 36;
      *(float *)(v6 - 28) = *(float *)(result - 24);
      *(_DWORD *)(v6 - 36) = *(_DWORD *)(result - 32);
      *(float *)(v6 - 24) = *(float *)(result - 20);
      v7 += 32;
      *(float *)(v6 - 32) = *(float *)(result - 28);
      v5 += 36;
      --v8;
      *(float *)(v6 - 20) = *(float *)(result - 16);
      *(_DWORD *)(v5 - 40) = s_worldData_surfaces + 12 * *(_DWORD *)(v7 - 36);
      *(_DWORD *)(v5 - 36) = *(_DWORD *)(v7 - 32);
    }
    while ( v8 );
  }
  return result;
}

/* ---- R_LoadCullGroupIndexes  0x004DCC90 ----  VERIFIED */
_DWORD *__cdecl R_LoadCullGroupIndexes(int a1)
{
  int v2;
  int v3;
  _DWORD *result;
  int v5;

  v2 = bspImageBase + *(_DWORD *)(a1 + 4);
  if ( (*(_BYTE *)a1 & 3) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *(_DWORD *)a1 >> 2;
  result = (_DWORD *)ri_Hunk_Alloc(4 * v3);
  s_worldData_cullGroupIndexes = (int)result;
  s_worldData_cullGroupIndexCount = v3;
  if ( v3 > 0 )
  {
    v5 = v2 - (_DWORD)result;
    do
    {
      *result = s_worldData_cullGroups + 36 * *(_DWORD *)((char *)result + v5);
      ++result;
      --v3;
    }
    while ( v3 );
  }
  return result;
}

/* ---- R_LoadOccluders  0x004DCD00 ----  [HIGH] */
int __cdecl R_LoadOccluders(int a1, _DWORD *a2, int a3)
{
  int v5;
  signed int v6;
  int v7;
  int result;
  int *v9;
  int v10;
  int v11;
  int v12;
  _DWORD *v13;
  _DWORD *v14;
  int v15;
  int v16;
  int v17;
  double v18;
  int v19;
  int v20;
  int v21;
  unsigned __int8 *v22;
  signed int v23;
  int v24;
  int v25;
  int v26;
  int v27;
  int v28;

  v5 = bspImageBase + a2[1];
  if ( *a2 % 0x14u )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v6 = *a2 / 0x14u;
  v7 = ri_Hunk_Alloc(36 * v6);
  v25 = bspImageBase + *(_DWORD *)(a1 + 4);
  if ( (*(_BYTE *)a1 & 3) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v24 = ri_Hunk_Alloc(20 * (*(_DWORD *)a1 >> 2));
  v27 = bspImageBase + *(_DWORD *)(a3 + 4);
  if ( (*(_BYTE *)a3 & 3) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  result = ri_Hunk_Alloc(16 * (*(_DWORD *)a3 >> 2));
  v26 = result;
  s_worldData_occluders = v7;
  s_worldData_occluderCount = v6;
  if ( v6 > 0 )
  {
    result = v5 + 12;
    v28 = v5 + 12;
    v9 = (int *)(v7 + 4);
    v23 = v6;
    do
    {
      *v9 = *(__int16 *)(result - 8);
      v10 = *(_DWORD *)(result - 12);
      *(v9 - 1) = v24 + 20 * v10;
      v11 = 0;
      if ( *v9 > 0 )
      {
        v12 = 0;
        v13 = (_DWORD *)(v25 + 4 * v10);
        do
        {
          v14 = (_DWORD *)ri_CM_GetPlaneNum(*v13);
          *(_DWORD *)(v12 + *(v9 - 1)) = *v14;
          *(_DWORD *)(v12 + *(v9 - 1) + 4) = v14[1];
          *(_DWORD *)(v12 + *(v9 - 1) + 8) = v14[2];
          *(_DWORD *)(v12 + *(v9 - 1) + 12) = v14[3];
          v15 = *(v9 - 1);
          v16 = *(_DWORD *)(v15 + v12);
          v17 = *(_DWORD *)(v15 + v12 + 4);
          v18 = *(float *)(v15 + v12 + 12) - 0.001;
          v19 = v12 + v15;
          *(float *)(v19 + 12) = v18;
          *(_BYTE *)(v19 + 16) = v16 <= 0 ? 0 : 0xC;
          LOBYTE(v16) = *(int *)(v19 + 8) <= 0 ? 8 : 20;
          *(_BYTE *)(v19 + 17) = v17 <= 0 ? 4 : 16;
          *(_BYTE *)(v19 + 18) = v16;
          ++v11;
          ++v13;
          v12 += 20;
        }
        while ( v11 < *v9 );
        result = v28;
      }
      v9[4] = s_worldData_portalVerts + 12 * *(_DWORD *)result;
      v9[3] = *(__int16 *)(result + 4);
      v9[1] = *(__int16 *)(result - 6);
      v9[2] = v26 + 16 * *(_DWORD *)(result - 4);
      v20 = 0;
      if ( v9[1] > 0 )
      {
        v21 = 0;
        v22 = (unsigned __int8 *)(v27 + 2);
        do
        {
          *(_DWORD *)(v21 + v9[2]) = *(v9 - 1) + 20 * *(v22 - 2);
          *(_DWORD *)(v9[2] + v21 + 4) = *(v9 - 1) + 20 * *(v22 - 1);
          *(_DWORD *)(v9[2] + v21 + 8) = v9[4] + 12 * *v22;
          *(_DWORD *)(v9[2] + v21 + 12) = v9[4] + 12 * v22[1];
          ++v20;
          v22 += 4;
          v21 += 16;
        }
        while ( v20 < v9[1] );
        result = v28;
      }
      v9[6] = 0;
      v9[7] = 0;
      result += 20;
      v9 += 9;
      v28 = result;
      --v23;
    }
    while ( v23 );
  }
  return result;
}

/* ---- R_LoadOccluderIndexes  0x004DCF90 ----  VERIFIED */
int __cdecl R_LoadOccluderIndexes(int a1)
{
  int v2;
  int v3;
  int result;
  int v5;

  v2 = bspImageBase + *(_DWORD *)(a1 + 4);
  if ( (*(_BYTE *)a1 & 1) != 0 )
    ri_Error(1, "\x15" "LoadMap: funny lump size in %s", &s_worldData);
  v3 = *(_DWORD *)a1 >> 1;
  result = ri_Hunk_Alloc(4 * v3);
  v5 = 0;
  s_worldData_occluderIndexes = result;
  for ( s_worldData_occluderIndexCount = v3; v5 < v3; ++v5 )
    *(_DWORD *)(result + 4 * v5) = s_worldData_occluders + 36 * *(__int16 *)(v2 + 2 * v5);
  return result;
}

/* ---- RE_LoadWorldMap  0x004DD000 ----  VERIFIED */
int __cdecl RE_LoadWorldMap(char *Source, unsigned int *a2)
{
  int v3;
  char v4;
  char *v5;
  const char *i;
  char v7;
  char *j;
  int v9;
  void *v10;
  int v11;
  char *v12;
  unsigned int k;
  double v14;
  int v15;
  int v16;
  int v17;
  void *buffer; // [esp+10h] [ebp-A08h] BYREF
  int v20;
  _BYTE v21[2560]; // [esp+18h] [ebp-A00h] BYREF

  dword_16DCA54 = 0;
  if ( tr_worldMapLoaded )
    ri_Error(1, "\x15" "ERROR: attempted to redundantly load world map\n");
  R_InitStaticModelCache(0);
  dword_16C57D0 = 1055286886;
  dword_16C57D4 = 1050253722;
  dword_16C57D8 = 1063675494;
  Com_Memset(&rendererSunState, 0, 0x9Cu);
  tr_sunName = 0;
  dword_16C4B70 = 0;
  dword_16C4BB0 = 0;
  dword_16C4BF0 = 0;
  dword_16C4C70 = 0;
  dword_16C4CF0 = 0;
  dword_16C4C30 = 0;
  VectorNormalize((float *)&tr_sunDirection);
  tr_worldMapLoaded = 1;
  v3 = ri_FS_ReadFile(Source, &buffer);
  if ( !buffer )
    ri_Error(1, "\x15" "RE_LoadWorldMap: %s not found", Source);
  if ( a2 )
    *a2 = Com_BlockChecksum(buffer, v3);
  tr_world = 0;
  memset(&s_worldData, 0, 0x154u);
  strncpy(&s_worldData, Source, 0x3Fu);
  v4 = s_worldData;
  v5 = &s_worldData;
  byte_11A3127 = 0;
  for ( i = &s_worldData; v4; ++v5 )
  {
    if ( v4 == 47 )
      i = v5 + 1;
    v4 = v5[1];
  }
  strncpy(&s_worldData_baseName, i, 0x3Fu);
  v7 = s_worldData_baseName;
  byte_11A3167 = 0;
  for ( j = &s_worldData_baseName; v7; v7 = *++j )
  {
    if ( v7 == 46 )
      break;
    *j = v7;
  }
  *j = 0;
  rendererSkyBox = (int)&s_worldData_skyVertexStorage;
  v9 = ri_Hunk_Alloc(0);
  v10 = buffer;
  v20 = v9;
  v11 = *((_DWORD *)buffer + 1);
  bspImageBase = (int)buffer;
  if ( v11 != 59 )
  {
    v12 = va("EXE_ERR_WRONG_MAP_VERSION_NUM\x15%s\x15(%i \x14" "EXE_ERR_SHOULD_BE\x15 %i)", Source, v11, 59);
    ri_Error(1, v12);
  }
  for ( k = 0; k < 0x44; ++k )
    ;
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadShaders((_DWORD *)v10 + 2);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadLightmaps((int *)v10 + 4, 0, (int)v10 + 56, (int)v21);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  v14 = ri_Printf(0, "Loading surfaces...\n");
  R_LoadSurfaces((int)v10 + 72, (int)v10 + 56, (_DWORD *)v10 + 16, v14, (int)v21);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  ri_Printf(0, "Loading cull groups...\n");
  R_LoadCullGroups((int)v10 + 80);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadCullGroupIndexes((int)v10 + 88);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  ri_Printf(0, "Loading visibility info...\n");
  R_LoadPortalVerts((_DWORD *)v10 + 24);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadOccluders((int)v10 + 112, (_DWORD *)v10 + 26, (int)v10 + 120);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadOccluderIndexes((int)v10 + 128);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadAABBTrees((_DWORD *)v10 + 34);
  R_LoadCells((_DWORD *)v10 + 36);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadPortals((int)v10 + 152);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadLightIndexes((int)v10 + 160);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadNodesAndLeafs((_DWORD *)v10 + 42, (_DWORD *)v10 + 44);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  ri_Printf(0, "Loading models and entities...\n");
  R_LoadSubmodels((_DWORD *)v10 + 56, (const char *)v10 + 56);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  R_LoadEntities((_DWORD *)v10 + 60);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  ri_Printf(0, "Loading lights...\n");
  R_LoadLights((_DWORD *)v10 + 62);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  v15 = *((_DWORD *)v10 + 67);
  v16 = *((_DWORD *)v10 + 66);
  v17 = bspImageBase + v15;
  if ( v16 > 0 && !R_InitLightVisCacheFromBuffer(v17, v16) )
    ri_Error(1, "\x15" "R_LoadLightVisCache: funny lump size in %s", &s_worldData);
  ri_Cmd_ExecuteText(0, "updatescreen\n");
  s_worldData_dataSize = ri_Hunk_Alloc(0) - v20;
  tr_world = (int)&s_worldData;
  if ( r_vc_compile->integer )
    R_PrecalcLightVisCache((int)a2);
  R_InitLightVisHistory();
  if ( tr_sunName )
    R_LoadSunThroughCvars(&tr_sunName);
  return ri_FS_FreeFile(buffer);
}
