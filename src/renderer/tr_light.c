/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_gl_types.h"
#include "tr_tess.h"


#define cg_shadows                 ((cvar_t *)(cg_shadows))
#define r_diffuseSunQuality        ((cvar_t *)(r_diffuseSunQuality))
#define r_diffuseSunSteps          ((cvar_t *)(r_diffuseSunSteps))
#define r_entFullbright            ((cvar_t *)(r_entFullbright))
#define r_entLightCutoff           ((cvar_t *)(r_entLightCutoff))
#define r_maxEntLights             ((cvar_t *)(r_maxEntLights))
#define r_minEntLightIntensity     ((cvar_t *)(r_minEntLightIntensity))
#define r_showLeafLights           ((cvar_t *)(r_showLeafLights))
#define r_vc_compile               ((cvar_t *)(r_vc_compile))
#define r_vc_makelog               ((cvar_t *)(r_vc_makelog))
#define r_vc_showlog               ((cvar_t *)(r_vc_showlog))


int  CM_SightTrace( int oldHitNum, const float *start, const float *end, const float *mins, const float *maxs, clipHandle_t model, const float *origin, int brushmask, qboolean capsule, const void *sphere );
void CM_Trace( trace_t *results, const float *start, const float *end, const float *mins, const float *maxs, clipHandle_t model, const float *origin, int brushmask, qboolean capsule, const void *sphere );
extern float DistanceSquared( const float *p1, const float *p2 );
extern int MatrixInverse();
void memset32( unsigned int *dest, unsigned int constant, unsigned int count );
int  R_SortedHistoryEntry( unsigned short a1, short a2, short a3, int a4 );
extern int R_AddDebugBox();
extern int R_AddDebugLine();
extern int R_AddDebugString();
int R_CullPointAndRadius( float *pt, float radius );
extern int R_PointInLeaf();


void VectorNormalizeFast( float *v );

/* ---- FastFloor  0x004B4CB0 ----  VERIFIED */
int __cdecl FastFloor(float a1)
{
  return (int)floor( (double)a1 );
}

/* ---- R_ClearLightVisCache  0x004B4CE0 ----  VERIFIED */
int R_ClearLightVisCache()
{
  int result;

  result = 0;
  memset(&tr_lightVisCache, 0, 0x200000u);
  lightVisCache_maxAssociativity = 0;
  lightVisCache_entriesUsed = 0;
  lightVisCache_entriesFlushed = 0;
  lightVisCache_entriesFilledAtRuntime = 0;
  return result;
}

/* ---- R_SetHwLightGlobals  0x004B4D10 ----  VERIFIED */
int R_SetHwLightGlobals()
{
  int v0;
  int v1;
  int v2;
  int result;

  tr_diffuseSunQuality = r_diffuseSunQuality->integer;
  v0 = tr_diffuseSunQuality;
  if ( tr_diffuseSunQuality >= 0 )
  {
    if ( tr_diffuseSunQuality <= 2 )
      goto LABEL_6;
    v0 = 2;
  }
  else
  {
    v0 = 0;
  }
  tr_diffuseSunQuality = v0;
LABEL_6:
  tr_diffuseSunSteps = r_diffuseSunSteps->integer;
  v1 = tr_diffuseSunSteps;
  if ( tr_diffuseSunSteps >= 1 )
  {
    if ( tr_diffuseSunSteps <= 5 )
      goto LABEL_11;
    v1 = 5;
  }
  else
  {
    v1 = 1;
  }
  tr_diffuseSunSteps = v1;
LABEL_11:
  v2 = v0 + 1;
  tr_diffuseSunSampleScale = 0.5 / (double)(v1 * v1);
  tr_maxEntityLights = r_maxEntLights->integer;
  result = tr_maxEntityLights;
  if ( tr_maxEntityLights < v2 )
  {
    result = v2;
    tr_maxEntityLights = v2;
  }
  if ( result > glConfig_maxLights )
    tr_maxEntityLights = glConfig_maxLights;
  if ( r_diffuseSunSteps->modified )
  {
    result = 0;
    r_diffuseSunSteps->modified = qfalse;
    memset(&tr_lightVisCache, 0, 0x200000u);
    lightVisCache_maxAssociativity = 0;
    lightVisCache_entriesUsed = 0;
    lightVisCache_entriesFlushed = 0;
    lightVisCache_entriesFilledAtRuntime = 0;
  }
  return result;
}


/* ---- reverse_bits  0x004B4DE0 ----  VERIFIED */
int __cdecl reverse_bits(int a1)
{
  int v1;
  int v3;

  v1 = 0;
  if ( reverseBitsTableNeedsInitialization )
  {
    reverseBitsTableNeedsInitialization = 0;
    do
    {
      reverseBitsTable[v1] = 0;
      if ( (v1 & 1) != 0 )
        reverseBitsTable[v1] = 0x80;
      if ( (v1 & 2) != 0 )
        reverseBitsTable[v1] |= 0x40u;
      if ( (v1 & 4) != 0 )
        reverseBitsTable[v1] |= 0x20u;
      if ( (v1 & 8) != 0 )
        reverseBitsTable[v1] |= 0x10u;
      if ( (v1 & 0x10) != 0 )
        reverseBitsTable[v1] |= 8u;
      if ( (v1 & 0x20) != 0 )
        reverseBitsTable[v1] |= 4u;
      if ( (v1 & 0x40) != 0 )
        reverseBitsTable[v1] |= 2u;
      if ( (char)v1 < 0 )
        reverseBitsTable[v1] |= 1u;
      ++v1;
    }
    while ( v1 < 256 );
  }
  LOBYTE(v3) = reverseBitsTable[(unsigned __int8)a1];
  BYTE1(v3) = reverseBitsTable[BYTE1(a1)];
  BYTE2(v3) = reverseBitsTable[BYTE2(a1)];
  BYTE3(v3) = reverseBitsTable[BYTE3(a1)];
  return v3;
}

/* ---- R_SkyTracePassed  0x004B4EB0 ----  VERIFIED */
BOOL __cdecl R_SkyTracePassed(int a1)
{
  if ( *(_BYTE *)(a1 + 47) )
    return (*(_DWORD *)(a1 + 32) >> 11) & 1;
  return *(_DWORD *)a1 == 1065353216 || (*(_BYTE *)(a1 + 28) & 4) != 0;
}

/* ---- CM_IsBigStaticModel  0x004B4EE0 ----  VERIFIED */
qboolean __fastcall CM_IsBigStaticModel(const vec3_t maxs, const vec3_t mins)
{
  double v2;
  double v3;
  double v4;
  qboolean result;

  v2 = maxs[2] - mins[2];
  result = qfalse;
  if ( !((v2 < 64.0) | __UNORDERED__(v2, 64.0)) )
  {
    v3 = maxs[1] - mins[1];
    if ( !((v3 < 32.0) | __UNORDERED__(v3, 32.0)) )
    {
      v4 = *maxs - *mins;
      if ( !((v4 < 32.0) | __UNORDERED__(v4, 32.0)) )
        return 1;
    }
  }
  return result;
}

/* ---- R_LightCacheSkyTrace  0x004B4F20 ----  VERIFIED */
char __cdecl R_LightCacheSkyTrace(const float *a1, const float *a2)
{
  trace_t trace; /* [esp+0h] [ebp-30h] BYREF */

  trace.fraction = 1.0f;
  CM_Trace( &trace, a2, a1, vec3_origin, vec3_origin, 0, vec3_origin, 0x2001, qfalse, NULL );
  if ( trace.startsolid )
  {
    if ( ((trace.contents >> 11) & 1) == 0 )
      return 0;
  }
  else if ( !((trace.fraction == 1.0) | __UNORDERED__(trace.fraction, 1.0)) && (trace.surfaceFlags & 4) == 0 )
  {
    return 0;
  }
  return 2;
}

/* ---- R_LightCacheTrace  0x004B4F90 ----  VERIFIED */
BOOL __cdecl R_LightCacheTrace(const float *a1, const float *a2)
{
  trace_t trace; /* [esp+0h] [ebp-30h] BYREF */

  trace.fraction = 1.0f;
  CM_Trace( &trace, a2, a1, vec3_origin, vec3_origin, 0, vec3_origin, 0x2001, qfalse, NULL );
  return !trace.startsolid && !((trace.fraction < 1.0) | __UNORDERED__(trace.fraction, 1.0));
}

/* ---- R_SampleDiffuseSunVisibility  0x004B4FF0 ----  VERIFIED */
void __cdecl R_SampleDiffuseSunVisibility(int a1, int a2, float *a3, int a4)
{
  bool v4; // zf
  char v5;
  double v6;
  double v7;
  float v9;
  float maxs;
  float maxs_4;
  float maxs_8;
  clipHandle_t model[4]; // [esp+10h] [ebp-4Ch] BYREF; [3] is retail var_40
  float start[3]; // [esp+20h] [ebp-3Ch] BYREF
  trace_t v16; // [esp+2Ch] [ebp-30h] BYREF

  if ( *(_DWORD *)(a1 + 16) )
  {
    maxs_8 = 32768.0;
    if ( tr_diffuseSunSteps > 3 )
      maxs_8 = 16384.0;
    maxs_4 = (double)(tr_diffuseSunSteps - 1) * (maxs_8 * 0.5);
    start[2] = a3[2] + 0.1;
    *(float *)&model[3] = a3[2] + 32768.0;
    *(float *)model = -maxs_4;
    maxs = *(float *)model;
    if ( maxs_4 >= (double)*(float *)model )
    {
      (void)a2;
      while ( 1 )
      {
        v9 = -maxs_4;
        start[1] = maxs * 0.00000061035155 + a3[1];
        *(float *)&model[2] = maxs + a3[1];
        if ( *(float *)model <= (double)maxs_4 )
          break;
LABEL_12:
        v7 = maxs + maxs_8;
        maxs = v7;
        if ( v7 > maxs_4 )
          return;
      }
      while ( 1 )
      {
        start[0] = v9 * 0.00000061035155 + *a3;
        *(float *)&model[1] = v9 + *a3;
        v16.fraction = 1.0;
        CM_Trace( &v16, start, (const float *)&model[1], vec3_origin, vec3_origin,
                  0, vec3_origin, 0x2001, qfalse, NULL );
        if ( v16.startsolid )
        {
          v4 = ((v16.contents >> 11) & 1) == 0;
        }
        else
        {
          if ( (v16.fraction == 1.0) | __UNORDERED__(v16.fraction, 1.0) )
          {
LABEL_10:
            v5 = *(_BYTE *)(a4 + 5);
            *(_WORD *)(a4 + 6) |= 0x8000u;
            *(_BYTE *)(a4 + 5) = v5 + 2;
            goto LABEL_11;
          }
          v4 = (v16.surfaceFlags & 4) == 0;
        }
        if ( !v4 )
          goto LABEL_10;
LABEL_11:
        v6 = v9 + maxs_8;
        v9 = v6;
        if ( v6 > maxs_4 )
          goto LABEL_12;
      }
    }
  }
}

/* ---- R_SampleLightVisibility  0x004B5190 ----  VERIFIED */
int __cdecl R_SampleLightVisibility(float *a1, int a2, int a3, int a4, int a5)
{
  _DWORD *v7;
  int v8;
  int i;
  float *v10;
  const float *v11;
  int result;
  int v13;
  float model[3]; // [esp+14h] [ebp-18h] BYREF
  float v17[3]; // [esp+20h] [ebp-Ch] BYREF
  int v18;

  *(_WORD *)(a2 + 6) = 0;
  *(_BYTE *)(a2 + 5) = 0;
  v7 = R_PointInLeaf(a1);
  v8 = (int)v7;
  v13 = (int)v7;
  if ( (*(_BYTE *)v7 & 1) != 0 )
    goto LABEL_4;
  if ( (int)v7[3] < 0 )
    goto LABEL_4;
  v18 = 0;
  model[0] = *(float *)a5 - *a1;
  model[1] = *(float *)(a5 + 4) - a1[1];
  model[2] = *(float *)(a5 + 8) - a1[2];
  VectorNormalizeFast(model);
  model[0] = model[0] * 0.0099999998 + *a1;
  model[1] = model[1] * 0.0099999998 + a1[1];
  model[2] = model[2] * 0.0099999998 + a1[2];
  if ( CM_SightTrace( 0, (const float *)a5, (const float *)model,
                      vec3_origin, vec3_origin, 0, vec3_origin,
                      0x2001, qfalse, NULL ) )
  {
LABEL_4:
    v18 = 1;
  }
  else
  {
    for ( i = 0; i < a3; ++i )
    {
      v10 = *(float **)(a4 + 4 * i);
      if ( (v10[20] == 0.0) | __UNORDERED__(v10[20], 0.0) )
      {
        model[0] = v10[17] * 0.1 + *a1;
        model[1] = v10[18] * 0.1 + a1[1];
        model[2] = v10[19] * 0.1 + a1[2];
        v17[0] = v10[17] * 32768.0 + *a1;
        v17[1] = v10[18] * 32768.0 + a1[1];
        v17[2] = v10[19] * 32768.0 + a1[2];
        if ( R_LightCacheSkyTrace(v17, (const float *)model) == 2 )
          *(_WORD *)(a2 + 6) |= 1 << i;
      }
      else
      {
        v11 = v10 + 17;
        model[0] = v10[17] - *a1;
        model[1] = v10[18] - a1[1];
        model[2] = v10[19] - a1[2];
        VectorNormalizeFast(model);
        model[0] = model[0] * 0.1 + *a1;
        model[1] = model[1] * 0.1 + a1[1];
        model[2] = model[2] * 0.1 + a1[2];
        if ( R_LightCacheTrace(v11, (const float *)model) )
          *(_WORD *)(a2 + 6) |= 1 << i;
      }
    }
    R_SampleDiffuseSunVisibility(v13, a5, a1, a2);
    v8 = v13;
  }
  result = v8;
  *(_BYTE *)(a2 + 4) = (v18 != 0) + 1;
  return result;
}

/* ---- R_LightVisHash  0x004B53D0 ----  VERIFIED */
int __cdecl R_LightVisHash(__int16 a1, __int16 a2, int a3, int a4, int *a5, int *a6)
{
  int result;

  *a5 = a1 & 0xFFF | (((a3 << 10) | a2 & 0x3FF) << 12);
  result = (3137 * (_WORD)a3 - 3133 * a2 + (unsigned __int16)reverse_bits(a4)) & 0x1FFF;
  *a6 = result;
  return result;
}

/* ---- R_GetCachedVisibility  0x004B5420 ----  VERIFIED */
int __cdecl R_GetCachedVisibility(
        __int16 a1,
        int a2,
        int a3,
        int a4,
        int a5,
        int a6,
        _DWORD *a7,
        _BYTE *a8)
{
  int v10;
  int v11;
  char *v12;
  int v13;
  char *Src;
  int v16;
  float v17[3]; // [esp+18h] [ebp-Ch] BYREF
  int v18;

  v10 = a1 & 0xFFF | (((a2 << 10) | a3 & 0x3FF) << 12);
  v11 = ((3137 * (_WORD)a2 - 3133 * (_WORD)a3 + (unsigned __int16)reverse_bits(a4)) & 0x1FFF) << 8;
  v12 = (char *)&tr_lightVisCache + v11;
  v16 = v11;
  v13 = 32;
  Src = v12;
  do
  {
    if ( !v12[4] )
      break;
    if ( *(_DWORD *)v12 == v10 )
    {
      *a8 = v12[5];
      goto LABEL_14;
    }
    v12 += 8;
    --v13;
  }
  while ( v13 );
  v18 = v13;
  if ( lightVisHistory )
  {
    if ( lightVisHistoryCount < 0x400000 )      /* 0x004B54B1: cmp ecx, 400000h */
    {
      *((_DWORD *)lightVisHistory + 6 * lightVisHistoryCount) = a3;
      *((_DWORD *)lightVisHistory + 6 * lightVisHistoryCount + 1) = a2;
      *((_DWORD *)lightVisHistory + 6 * lightVisHistoryCount + 2) = a4;
      *((_DWORD *)lightVisHistory + 6 * lightVisHistoryCount + 3) = *a7;
      *((_DWORD *)lightVisHistory + 6 * lightVisHistoryCount + 4) = a7[1];
      *((_DWORD *)lightVisHistory + 6 * lightVisHistoryCount + 5) = a7[2];
      R_SortedHistoryEntry(a4, a3, a2, 1);
      ++lightVisHistoryCount;
      v13 = v18;
    }
  }
  ++lightVisCache_entriesFilledAtRuntime;
  if ( v13 )
  {
    ++lightVisCache_entriesUsed;
    if ( lightVisCache_maxAssociativity < 33 - v13 )
      lightVisCache_maxAssociativity = 33 - v13;
  }
  else
  {
    memcpy((char *)&tr_lightVisCache + 8 + v16, Src, 0xF8u);
    v12 = Src;
    ++lightVisCache_entriesFlushed;
  }
  v17[0] = (float)(32 * (a3 - 4096));
  v17[1] = (float)(32 * (a2 - 4096));
  v17[2] = (float)((a4 - 2048) << 6);
  *(_DWORD *)v12 = v10;
  R_SampleLightVisibility(v17, (int)v12, a5, a6, (int)a7);
  *a8 = v12[5];
LABEL_14:
  if ( v12[4] == 1 )
    return *((unsigned __int16 *)v12 + 3);
  else
    return -1;
}

/* ---- R_TransformDlights  0x004B5650 ----  [HIGH] */
void __cdecl R_TransformDlights(int a1, float *a2, float *a3)
{
  int v3;
  int v4;
  double v5;
  double v6;
  double v7;
  double v8; // rt0
  double v9;
  double v10;
  double v11;
  float *v12;
  double v13;
  double v14;
  double v15;
  double v16; // rt2
  double v17;
  double v18;
  double v19;
  float *v20;
  double v21;
  double v22;
  double v23;
  double v24; // rt0
  double v25;
  double v26;
  double v27;
  float *v28;
  double v29;
  double v30;
  double v31;
  double v32; // rt2
  double v33;
  double v34;
  double v35;
  float *v36;
  int v37;
  double v38;
  double v39;
  double v40;
  double v41; // rt0
  double v42;
  double v43;
  double v44;

  v3 = 0;
  if ( a1 >= 4 )
  {
    v4 = 3;
    do
    {
      if ( (a2[20] == 0.0) | __UNORDERED__(a2[20], 0.0) )
      {
        a2[30] = a2[18] * a3[4] + a3[3] * a2[17] + a2[19] * a3[5];
        a2[31] = a2[18] * a3[7] + a2[19] * a3[8] + a3[6] * a2[17];
        v11 = a3[11] * a2[19] + a2[18] * a3[10];
        v10 = a2[17];
      }
      else
      {
        v5 = a2[17] - *a3;
        v6 = a2[18] - a3[1];
        v7 = a2[19] - a3[2];
        a2[30] = v5 * a3[3] + v7 * a3[5] + v6 * a3[4];
        a2[31] = v5 * a3[6] + v7 * a3[8] + v6 * a3[7];
        v8 = v7 * a3[11];
        v9 = v6 * a3[10];
        v10 = v5;
        v11 = v8 + v9;
      }
      v12 = a2 + 34;
      *(v12 - 2) = v11 + v10 * a3[9];
      if ( (v12[20] == 0.0) | __UNORDERED__(v12[20], 0.0) )
      {
        v12[30] = v12[18] * a3[4] + a3[3] * v12[17] + v12[19] * a3[5];
        v12[31] = v12[18] * a3[7] + v12[19] * a3[8] + a3[6] * v12[17];
        v19 = a3[11] * v12[19] + v12[18] * a3[10];
        v18 = v12[17];
      }
      else
      {
        v13 = v12[17] - *a3;
        v14 = v12[18] - a3[1];
        v15 = v12[19] - a3[2];
        v12[30] = v13 * a3[3] + v15 * a3[5] + v14 * a3[4];
        v12[31] = v13 * a3[6] + v15 * a3[8] + v14 * a3[7];
        v16 = v15 * a3[11];
        v17 = v14 * a3[10];
        v18 = v13;
        v19 = v16 + v17;
      }
      v20 = v12 + 34;
      *(v20 - 2) = v19 + v18 * a3[9];
      if ( (v20[20] == 0.0) | __UNORDERED__(v20[20], 0.0) )
      {
        v20[30] = v20[18] * a3[4] + a3[3] * v20[17] + v20[19] * a3[5];
        v20[31] = v20[18] * a3[7] + v20[19] * a3[8] + a3[6] * v20[17];
        v27 = a3[11] * v20[19] + v20[18] * a3[10];
        v26 = v20[17];
      }
      else
      {
        v21 = v20[17] - *a3;
        v22 = v20[18] - a3[1];
        v23 = v20[19] - a3[2];
        v20[30] = v21 * a3[3] + v23 * a3[5] + v22 * a3[4];
        v20[31] = v21 * a3[6] + v23 * a3[8] + v22 * a3[7];
        v24 = v23 * a3[11];
        v25 = v22 * a3[10];
        v26 = v21;
        v27 = v24 + v25;
      }
      v28 = v20 + 34;
      *(v28 - 2) = v27 + v26 * a3[9];
      if ( (v28[20] == 0.0) | __UNORDERED__(v28[20], 0.0) )
      {
        v28[30] = v28[18] * a3[4] + a3[3] * v28[17] + v28[19] * a3[5];
        v28[31] = v28[18] * a3[7] + v28[19] * a3[8] + a3[6] * v28[17];
        v35 = a3[11] * v28[19] + v28[18] * a3[10];
        v34 = v28[17];
      }
      else
      {
        v29 = v28[17] - *a3;
        v30 = v28[18] - a3[1];
        v31 = v28[19] - a3[2];
        v28[30] = v29 * a3[3] + v31 * a3[5] + v30 * a3[4];
        v28[31] = v29 * a3[6] + v31 * a3[8] + v30 * a3[7];
        v32 = v31 * a3[11];
        v33 = v30 * a3[10];
        v34 = v29;
        v35 = v32 + v33;
      }
      v4 += 4;
      a2 = v28 + 34;
      v3 += 4;
      *(a2 - 2) = v35 + v34 * a3[9];
    }
    while ( v4 < a1 );
  }
  if ( v3 < a1 )
  {
    v36 = a2 + 30;
    v37 = a1 - v3;
    do
    {
      if ( (*(v36 - 10) == 0.0) | __UNORDERED__(*(v36 - 10), 0.0) )
      {
        *v36 = *(v36 - 11) * a3[5] + *(v36 - 12) * a3[4] + a3[3] * *(v36 - 13);
        v36[1] = a3[6] * *(v36 - 13) + a3[8] * *(v36 - 11) + a3[7] * *(v36 - 12);
        v44 = a3[11] * *(v36 - 11) + a3[10] * *(v36 - 12);
        v43 = *(v36 - 13);
      }
      else
      {
        v38 = *(v36 - 13) - *a3;
        v39 = *(v36 - 12) - a3[1];
        v40 = *(v36 - 11) - a3[2];
        *v36 = v38 * a3[3] + v40 * a3[5] + v39 * a3[4];
        v36[1] = v38 * a3[6] + v40 * a3[8] + v39 * a3[7];
        v41 = v40 * a3[11];
        v42 = v39 * a3[10];
        v43 = v38;
        v44 = v41 + v42;
      }
      v36 += 34;
      --v37;
      *(v36 - 32) = v44 + v43 * a3[9];
    }
    while ( v37 );
  }
}

/* ---- R_DlightBmodel  0x004B59F0 ----  [HIGH] */
int __cdecl R_DlightBmodel(int a1)
{
  int v2;
  int v3;
  int v4;
  float *v5;
  float *v6;
  __int64 v7; // rax
  int v8;
  int *v9;
  int v11;

  R_TransformDlights(tr_refdef_num_dlights, (float *)tr_refdef_dlights, &tr_or);
  v2 = 0;
  v11 = 0;
  if ( tr_refdef_num_dlights > 0 )
  {
    v3 = tr_refdef_dlights + 124;
    do
    {
      v4 = 0;
      v5 = (float *)v3;
      v6 = (float *)(a1 + 16);
      while ( *(v5 - 1) - *(v6 - 1) <= *(float *)(v3 - 8) && *(v6 - 4) - *(v5 - 1) <= *(float *)(v3 - 8) )
      {
        if ( *v5 - *v6 > *(float *)(v3 - 8) || *(v6 - 3) - *v5 > *(float *)(v3 - 8) )
        {
          ++v4;
          break;
        }
        if ( v5[1] - v6[1] > *(float *)(v3 - 8) || *(v6 - 2) - v5[1] > *(float *)(v3 - 8) )
        {
          v4 += 2;
          break;
        }
        v4 += 3;
        v6 += 3;
        v5 += 3;
        if ( v4 >= 3 )
          goto LABEL_15;
      }
      if ( v4 < 3 )
        goto LABEL_16;
LABEL_15:
      v11 |= 1 << v2;
LABEL_16:
      ++v2;
      v3 += 136;
    }
    while ( v2 < tr_refdef_num_dlights );
  }
  *(_DWORD *)(tr_currentEntity + 156) = v11;
  v7 = *(unsigned int *)(a1 + 28);
  if ( (int)v7 > 0 )
  {
    v8 = 0;
    do
    {
      v9 = *(int **)(v8 + *(_DWORD *)(a1 + 24) + 8);
      if ( *v9 >= 24 )
        v9[2] = v11;
      LODWORD(v7) = *(_DWORD *)(a1 + 28);
      ++HIDWORD(v7);
      v8 += 12;
    }
    while ( SHIDWORD(v7) < (int)v7 );
  }
  return v7;
}

/* ---- R_ShowLeafLights  0x004B5B20 ----  VERIFIED */
void __cdecl R_ShowLeafLights(float *a1, int a2)
{
  int integer;
  double v3;
  _DWORD *v4;
  int v5;
  int v6;
  float *v7;
  int v8;
  _DWORD *v9;
  float **v10;
  bool v11; // zf
  float *v12;
  _DWORD *v13;
  float *v14;
  double v15;
  double v16;
  float v17;
  float v18;
  _DWORD *v19;
  float v20;
  int v21;
  float v22;
  float v23;
  float v24[3]; // [esp+18h] [ebp-48h] BYREF -- ONE vec3_t
  float v27[3]; // [esp+24h] [ebp-3Ch] BYREF -- ONE vec3_t
  float v30[3]; // [esp+34h] [ebp-2Ch] BYREF -- never written

  if ( tr_world )
  {
    if ( *(_DWORD *)(tr_world + 152) )
    {
      integer = r_showLeafLights->integer;
      if ( integer != 3
        || (v24[0] = *a1 - tr_refdef_vieworgX,
            v24[1] = a1[1] - tr_refdef_vieworgY,
            v24[2] = a1[2] - tr_refdef_vieworgZ,
            VectorNormalizeFast(v24),
            v3 = *(float *)&tr_refdef_viewaxis02 * v24[2] + *(float *)&tr_refdef_viewaxis01 * v24[1] + *(float *)&tr_refdef_viewaxis00 * v24[0],
            !((v3 < 0.94999999) | __UNORDERED__(v3, 0.94999999))) )
      {
        if ( integer != 4 || DistanceSquared(&tr_refdef_vieworgX, a1) <= 9216.0 )
        {
          v4 = R_PointInLeaf(a1);
          v19 = v4;
          if ( v4 )
          {
            v21 = 0;
            if ( (int)v4[6] > 0 )
            {
              do
              {
                v5 = 136 * *(__int16 *)(*(_DWORD *)(tr_world + 284) + 2 * (v21 + v4[5]));
                v6 = *(_DWORD *)(a2 + 204);
                v7 = (float *)(*(_DWORD *)(tr_world + 272) + v5);
                v8 = 0;
                v9 = dword_541830;
                if ( v6 > 0 )
                {
                  v10 = (float **)(a2 + 208);
                  while ( *v10 != v7 )
                  {
                    ++v8;
                    v10 += 2;
                    if ( v8 >= v6 )
                      goto LABEL_15;
                  }
                  v9 = dword_541840;
                }
LABEL_15:
                if ( (v7[20] == 0.0) | __UNORDERED__(v7[20], 0.0) )
                {
                  v11 = v9 == dword_541840;
                  v9 = dword_541870;
                  if ( !v11 )
                    v9 = dword_541860;
                  v12 = v27;
                  v27[0] = v7[17] * 1024.0 + *a1;
                  v27[1] = v7[18] * 1024.0 + a1[1];
                  v27[2] = v7[19] * 1024.0 + a1[2];
                }
                else
                {
                  v12 = v7 + 17;
                }
                R_AddDebugLine(a1, v12, v9);
                ++v21;
                v4 = v19;
              }
              while ( v21 < v19[6] );
            }
            if ( r_showLeafLights->integer >= 4 )
            {
              if ( v4[4] )
              {
                v22 = 32768.0;
                if ( tr_diffuseSunSteps > 3 )
                  v22 = 16384.0;
                v20 = (double)(tr_diffuseSunSteps - 1) * (v22 * 0.5);
                v24[2] = a1[2] + 0.1;
                v27[2] = a1[2] + 32768.0;
                v23 = -v20;
                v18 = v23;
                if ( v20 >= (double)v23 )
                {
                  do
                  {
                    v17 = -v20;
                    v24[1] = v18 * 0.00000061035155 + a1[1];
                    v27[1] = v18 + a1[1];
                    if ( v23 <= (double)v20 )
                    {
                      do
                      {
                        v24[0] = v17 * 0.00000061035155 + *a1;
                        v27[0] = v17 + *a1;
                        if ( R_LightCacheSkyTrace(v27, v24) )
                        {
                          v13 = clippedLeafDebugColor;
                          v14 = v27;
                        }
                        else
                        {
                          v13 = staticModelDebugColor;
                          v14 = v30;
                        }
                        R_AddDebugLine(v24, v14, v13);
                        v15 = v17 + v22;
                        v17 = v15;
                      }
                      while ( v15 <= v20 );
                    }
                    v16 = v18 + v22;
                    v18 = v16;
                  }
                  while ( v16 <= v20 );
                }
              }
            }
          }
        }
      }
    }
  }
}


/* ---- R_MaxLightIntensity  0x004B5E30 ----  VERIFIED */
double __cdecl R_MaxLightIntensity(float *a1, float *a2)
{
  double result;
  long double v3;
  double v4;
  double v5;
  double v6;
  double v7;
  long double v8;

  result = a1[4];
  if ( tr_overbrightBits )
    result = result * (double)(1 << tr_overbrightBits);
  if ( !((a1[20] == 0.0) | __UNORDERED__(a1[20], 0.0)) )
  {
    v3 = a1[24];
    if ( (a1[25] == 0.0) | __UNORDERED__(a1[25], 0.0) && (a1[26] == 0.0) | __UNORDERED__(a1[26], 0.0) )
      return result / v3;
    v4 = a1[19] - a2[2];
    v5 = a1[18] - a2[1];
    v6 = v4 * v4 + v5 * v5;
    v7 = a1[17] - *a2;
    v8 = v6 + v7 * v7;
    v3 = v3 + v8 * a1[26];
    if ( (a1[25] == 0.0) | __UNORDERED__(a1[25], 0.0) )
      return result / v3;
    else
      return result / (v3 + sqrt(v8) * a1[25]);
  }
  return result;
}

/* ---- R_MergeLights  0x004B5ED0 ----  VERIFIED */
void __cdecl R_MergeLights(int a1, float *a2, int a3, _DWORD *a4)
{
  int v4;
  _DWORD *v5;
  double v6;
  BOOL v7;
  double v8;
  int i;
  int v10;
  float *v11;
  float *v12;
  int v13;
  float v14;
  float *v15;
  double v16;
  double v17;
  int v18;
  _DWORD *v19;
  int v20;
  float *v21;
  int v22;
  double v23;
  double v24;
  double v25;
  double v26;
  int v27;
  double v28;
  float v29;
  float v30;
  int v31;
  float *v32;
  float v33[3]; // [esp+Ch] [ebp-6Ch] BYREF -- ONE vec3_t in retail
  _BYTE v36[96]; // [esp+18h] [ebp-60h] BYREF -- 8 x vec3_t, one per merged light

  v4 = *(int *)(a1 + 204);
  if ( v4 > tr_maxEntityLights )
  {
    v5 = a4;
    v6 = *(float *)(a3 + 4 * *a4);
    v7 = ((v6 == 1.0e19) | __UNORDERED__(v6, 1.0e19)) != 0;
    v8 = *(float *)(a3 + 4 * a4[v7]) * r_entLightCutoff->value;
    for ( i = v7 + 1; i < tr_maxEntityLights - 1; ++i )
    {
      v30 = v8;
      if ( *(float *)(a3 + 4 * a4[i]) <= (double)v30 )
        break;
    }
    v33[2] = 0.0;
    v33[1] = 0.0;
    v33[0] = 0.0;
    v10 = i;
    if ( i < v4 )
    {
      v11 = (float *)&v36[12 * i];
      v31 = a1 + 8 * i + 208;
      do
      {
        v12 = *(float **)v31;
        if ( (*(float *)(*(_DWORD *)v31 + 80) == 0.0) | __UNORDERED__(*(float *)(*(_DWORD *)v31 + 80), 0.0) )
        {
          *v11 = v12[17];
          v11[1] = v12[18];
          v13 = *((_DWORD *)v12 + 19);
          v14 = *v12;
          *((_DWORD *)v11 + 2) = v13;
          if ( LODWORD(v14) != 1 && LODWORD(v14) != 8 )
            goto LABEL_13;
          v15 = (float *)(a3 + 4 * v5[v10]);
          *v15 = *v15 - (double)(1 << tr_overbrightBits) * *(float *)(tr_world + 264) * *(float *)(a1 + 272);
        }
        else
        {
          *v11 = v12[17] - *a2;
          v11[1] = v12[18] - a2[1];
          v11[2] = v12[19] - a2[2];
          VectorNormalize(v11);
        }
        v5 = a4;
LABEL_13:
        v16 = *(float *)(a3 + 4 * v5[v10]);
        v17 = v16 * *v11;
        v31 += 8;
        v18 = *(_DWORD *)(a1 + 204);
        ++v10;
        v11 += 3;
        v33[0] = v17 + v33[0];
        v33[1] = v16 * *(v11 - 2) + v33[1];
        v33[2] = v16 * *(v11 - 1) + v33[2];
      }
      while ( v10 < v18 );
    }
    VectorNormalize(v33);
    v19 = (_DWORD *)(a1 + 276);
    *(_DWORD *)(a1 + 304) = 0;
    *(_DWORD *)(a1 + 300) = 0;
    *(_DWORD *)(a1 + 296) = 0;
    *(_DWORD *)(a1 + 320) = 0;
    *(_DWORD *)(a1 + 316) = 0;
    *(_DWORD *)(a1 + 312) = 0;
    *(_DWORD *)(a1 + 336) = 0;
    *(_DWORD *)(a1 + 332) = 0;
    *(_DWORD *)(a1 + 328) = 0;
    v20 = i;
    if ( i < *(_DWORD *)(a1 + 204) )
    {
      v21 = (float *)&v36[12 * i + 4];
      v32 = v21;
      v22 = a1 + 8 * i + 208;
      do
      {
        v23 = v33[0] * *(v21 - 1) + v33[2] * v21[1] + v33[1] * *v21;
        if ( v23 > 0.0 )
        {
          if ( **(_DWORD **)v22 != 1 && *v19 != 8 )
            v23 = v23 * 0.80000001;
          v25 = tr_identityLight * *(float *)(a3 + 4 * a4[v20]) * v23;
          *(float *)(a1 + 312) = v25 * *(float *)(*(_DWORD *)v22 + 4) + *(float *)(a1 + 312);
          *(float *)(a1 + 316) = v25 * *(float *)(*(_DWORD *)v22 + 8) + *(float *)(a1 + 316);
          *(float *)(a1 + 320) = v25 * *(float *)(*(_DWORD *)v22 + 12) + *(float *)(a1 + 320);
          v24 = (1.0 - v23) * 0.5;
        }
        else
        {
          v24 = 0.5;
        }
        v26 = tr_identityLight * *(float *)(a3 + 4 * a4[v20]);
        v27 = *(_DWORD *)v22;
        ++v20;
        v22 += 8;
        v28 = v24 * v26;
        *(float *)(a1 + 296) = v28 * *(float *)(v27 + 4) + *(float *)(a1 + 296);
        *(float *)(a1 + 300) = v28 * *(float *)(*(_DWORD *)(v22 - 8) + 8) + *(float *)(a1 + 300);
        v21 = v32 + 3;
        v32 += 3;
        *(float *)(a1 + 304) = v28 * *(float *)(*(_DWORD *)(v22 - 8) + 12) + *(float *)(a1 + 304);
      }
      while ( v20 < *(_DWORD *)(a1 + 204) );
    }
    *(float *)(a1 + 344) = v33[0];
    *(float *)(a1 + 348) = v33[1];
    v29 = v33[2];
    *(_DWORD *)(a1 + 376) = 0;
    *(_DWORD *)(a1 + 380) = 0;
    *(_DWORD *)(a1 + 308) = 1065353216;
    *(_DWORD *)(a1 + 324) = 1065353216;
    *(_DWORD *)(a1 + 340) = 1065353216;
    *(float *)(a1 + 352) = v29;
    *(_DWORD *)(a1 + 372) = 1065353216;
    *(_DWORD *)(a1 + 388) = 1127481344;
    *(_DWORD *)(a1 + 204) = i + 1;
    *(_DWORD *)(a1 + 8 * i + 208) = v19;
    *(_DWORD *)(a1 + 8 * i + 212) = 1065353216;
  }
}


/* ---- R_GetStaticLightContributions  0x004B6210 ----  VERIFIED */
int __cdecl R_GetStaticLightContributions(float *a1, char *a2, float *a3, char *a4)
{
  float *v4;
  _DWORD *v5;
  int v6;
  int v7;
  int v9;
  int v10;
  char *v11;
  double v12;
  double v13;
  int v14;
  double v15;
  int v16;
  int integer;
  double v18;
  double v19;
  int v20;
  int v21;
  int v22;
  int v23;
  int v24;
  int v25;
  int CachedVisibility;
  int v27;
  double v28;
  double v29;
  double v30;
  char v31;
  char *v32;
  int v33;
  cvar_t *v34;
  float *v35;
  float *v36;
  _DWORD *v37;
  int v38;
  int v39;
  char *v40;
  int v41;
  float *v42;
  double v43;
  double v44;
  double v45;
  double v46;
  float *v47;
  int v48;
  double v49;
  unsigned __int8 v50; // [esp+13h] [ebp-79h] BYREF
  float v51;
  int v52;
  float v53;
  int v54;
  int v55;
  float v56;
  _DWORD *v57;
  int v58;
  int v59;
  int v60;
  double v61;
  float v62[3]; // [esp+44h] v62/v63/v64                       BYREF
  float v65[3]; // [esp+50h] [ebp-3Ch] BYREF
  float v66[3]; // [esp+5Ch] v66/v67/v68                       BYREF
  float v69[3]; // [esp+68h] [ebp-24h] BYREF
  float v70[6]; // [esp+74h] v70..v75, the trilinear weights

  *a3 = 0.0;
  if ( !*(_DWORD *)(tr_world + 152) )
    return 0;
  v4 = a1;
  v5 = R_PointInLeaf(a1);
  v6 = v5[3];
  v57 = v5;
  if ( v6 < 0 )
  {
    v7 = *(_DWORD *)(tr_world + 276);
    if ( v7 )
    {
      *(_DWORD *)a4 = v7;
      *(_DWORD *)a2 = 1065353216;
      *a3 = 1.0;
      return 1;
    }
    return 0;
  }
  v9 = v5[6];
  if ( !v9 && !v5[4] )
    return 0;
  v10 = 0;
  v52 = 0;
  if ( v9 > 0 )
  {
    v11 = a2;
    do
    {
      *(_DWORD *)&v11[a4 - a2] = *(_DWORD *)(tr_world + 272)
                               + 136 * *(__int16 *)(*(_DWORD *)(tr_world + 284) + 2 * (v10 + v5[5]));
      *(_DWORD *)v11 = 0;
      ++v10;
      v11 += 4;
    }
    while ( v10 < v5[6] );
    v4 = a1;
    v52 = v10;
  }
  v12 = *v4;
  v61 = 0.4999999990686774;
  *(float *)&v60 = v12 - -131072.0;
  v59 = (int)(*(float *)&v60 - 0.4999999990686774);
  v13 = *v4 - -131072.0;
  v14 = v59 >> 5;
  v58 = v59 >> 5;
  v61 = 0.4999999990686774;
  v70[1] = v13 * 0.03125 - (double)(v59 >> 5);
  v70[0] = 1.0 - v70[1];
  *(float *)&v60 = v4[1] - -131072.0;
  v59 = (int)(*(float *)&v60 - 0.4999999990686774);
  v15 = v4[1] - -131072.0;
  v16 = v59 >> 5;
  v54 = v59 >> 5;
  v61 = 0.4999999990686774;
  v70[3] = v15 * 0.03125 - (double)(v59 >> 5);
  v70[2] = 1.0 - v70[3];
  *(float *)&v60 = v4[2] - -131072.0;
  v59 = (int)(*(float *)&v60 - 0.4999999990686774);
  integer = r_showLeafLights->integer;
  v18 = (v4[2] - -131072.0) * 0.015625;
  v55 = v59 >> 6;
  v70[5] = v18 - (double)(v59 >> 6);
  v70[4] = 1.0 - v70[5];
  if ( integer != 2 )
  {
    if ( integer == 3 )
    {
      v62[0] = *v4 - tr_refdef_vieworgX;
      v62[1] = v4[1] - tr_refdef_vieworgY;
      v62[2] = v4[2] - tr_refdef_vieworgZ;
      VectorNormalizeFast(v62);
      v19 = *(float *)&tr_refdef_viewaxis02 * v62[2] + *(float *)&tr_refdef_viewaxis01 * v62[1] + *(float *)&tr_refdef_viewaxis00 * v62[0];
      if ( (v19 < 0.94999999) | __UNORDERED__(v19, 0.94999999) )
        goto LABEL_18;
    }
    else if ( integer >= 0 )
    {
      goto LABEL_18;
    }
  }
  v62[0] = (float)(32 * (v14 - 4096));
  v60 = (((int)(*(float *)&v60 - 0.4999999990686774) >> 6) - 2048) << 6;
  v62[1] = (float)(32 * (v16 - 4096));
  v62[2] = (float)v60;
  v66[0] = v62[0] + 32.0;
  v66[1] = v62[1] + 32.0;
  v66[2] = v62[2] + 64.0;
  R_AddDebugBox(dword_5418F0, v66, v62);
LABEL_18:
  v20 = 0;
  v51 = 0.0;
  v53 = 0.0;
  while ( 1 )
  {
    v21 = (v20 >> 2) & 1;
    v22 = v20 >> 1;
    v23 = v20 & 1;
    v24 = v22 & 1;
    v25 = v23 + v14;
    CachedVisibility = R_GetCachedVisibility(v57[3], v24 + v54, v25, v21 + v55, v52, (int)a4, a1, &v50);
    v27 = r_showLeafLights->integer;
    LODWORD(v61) = CachedVisibility;
    if ( v27 < 0 )
    {
      v62[0] = (double)(32 * v25) - 131072.0;
      v60 = 16 * ((LOBYTE(v53) & 2) + 2 * v54);
      v28 = (double)v60 - 131072.0;
      v60 = 16 * ((LOBYTE(v53) & 4) + 4 * v55);
      v62[1] = v28;
      v62[2] = (double)v60 - 131072.0;
      v66[0] = v62[0] - 1.0;
      v66[1] = v62[1] - 1.0;
      v66[2] = v62[2] - 1.0;
      v65[0] = v62[0] + 1.0;
      v65[1] = v62[1] + 1.0;
      v65[2] = v62[2] + 1.0;
    }
    if ( CachedVisibility >= 0 )
    {
      v29 = v70[2 + v24] * v70[4 + v21];
      v60 = v50;
      v30 = v29 * v70[0 + v23];
      v56 = v30;
      v51 = v30 + v51;
      *a3 = (double)v50 * v56 * tr_diffuseSunSampleScale + *a3;
      if ( CachedVisibility )
      {
        v31 = 0;
        *(float *)&v60 = 0.0;
        if ( v52 > 0 )
        {
          v32 = a4;
          v33 = a2 - a4;
          v59 = a2 - a4;
          while ( 1 )
          {
            if ( ((1 << v31) & CachedVisibility) != 0 )
            {
              v34 = r_showLeafLights;
              *(float *)&v32[v33] = v56 + *(float *)&v32[v33];
              if ( v34->integer < 0 )
              {
                v35 = *(float **)v32;
                if ( (*(float *)(*(_DWORD *)v32 + 80) == 0.0) | __UNORDERED__(*(float *)(*(_DWORD *)v32 + 80), 0.0) )
                {
                  v37 = dword_541870;
                  v36 = v69;
                  v69[0] = v35[17] * 32768.0 + v62[0];
                  v69[1] = v35[18] * 32768.0 + v62[1];
                  v69[2] = v35[19] * 32768.0 + v62[2];
                }
                else
                {
                  v36 = v35 + 17;
                  v37 = dword_541840;
                }
                R_AddDebugLine(v62, v36, v37);
                v33 = v59;
              }
            }
            v31 = v60 + 1;
            v32 += 4;
            if ( ++v60 >= v52 )
              break;
            CachedVisibility = LODWORD(v61);
          }
        }
      }
      else if ( r_showLeafLights->integer < 0 )
      {
        R_AddDebugBox(dword_541860, v65, v66);
      }
    }
    else if ( v27 < 0 )
    {
      R_AddDebugBox(dword_541830, v65, v66);
    }
    ++LODWORD(v53);
    if ( SLODWORD(v53) >= 8 )
      break;
    v20 = LODWORD(v53);
    v14 = v58;
  }
  if ( (v51 < 0.98000002) | __UNORDERED__(v51, 0.98000002) )
  {
    if ( (v51 == 0.0) | __UNORDERED__(v51, 0.0) )
    {
      if ( (int)v57[3] < 0 )
      {
        if ( v52 > 0 )
          memset32(a2, 1065353216, v52);
        *a3 = 1.0;
      }
    }
    else
    {
      v53 = 1.0 / v51;
      if ( !((*a3 == 0.0) | __UNORDERED__(*a3, 0.0)) )
        *a3 = v53 * *a3;
      v38 = v52;
      v39 = 0;
      if ( v52 >= 4 )
      {
        v40 = a4 + 12;
        v41 = 3;
        v42 = (float *)(a2 + 4);
        LODWORD(v61) = a4 - a2;
        do
        {
          if ( !((*(v42 - 1) == 0.0) | __UNORDERED__(*(v42 - 1), 0.0)) )
          {
            if ( *((_DWORD *)v40 - 3) == *(_DWORD *)(tr_world + 276) )
              v43 = 1.0 - v51 + *(v42 - 1);
            else
              v43 = v53 * *(v42 - 1);
            *(v42 - 1) = v43;
          }
          if ( !((*v42 == 0.0) | __UNORDERED__(*v42, 0.0)) )
          {
            if ( *(_DWORD *)((char *)v42 + LODWORD(v61)) == *(_DWORD *)(tr_world + 276) )
              v44 = 1.0 - v51 + *v42;
            else
              v44 = v53 * *v42;
            *v42 = v44;
          }
          if ( !((v42[1] == 0.0) | __UNORDERED__(v42[1], 0.0)) )
          {
            if ( *((_DWORD *)v40 - 1) == *(_DWORD *)(tr_world + 276) )
              v45 = 1.0 - v51 + v42[1];
            else
              v45 = v53 * v42[1];
            v42[1] = v45;
          }
          if ( !((v42[2] == 0.0) | __UNORDERED__(v42[2], 0.0)) )
          {
            if ( *(_DWORD *)v40 == *(_DWORD *)(tr_world + 276) )
              v46 = 1.0 - v51 + v42[2];
            else
              v46 = v53 * v42[2];
            v42[2] = v46;
          }
          v38 = v52;
          v41 += 4;
          v39 += 4;
          v42 += 4;
          v40 += 16;
        }
        while ( v41 < v52 );
      }
      if ( v39 < v38 )
      {
        v47 = (float *)&a2[4 * v39];
        v48 = v52 - v39;
        do
        {
          if ( !((*v47 == 0.0) | __UNORDERED__(*v47, 0.0)) )
          {
            if ( *(_DWORD *)((char *)v47 + (a4 - a2)) == *(_DWORD *)(tr_world + 276) )
              v49 = 1.0 - v51 + *v47;
            else
              v49 = v53 * *v47;
            *v47 = v49;
          }
          ++v47;
          --v48;
        }
        while ( v48 );
      }
    }
  }
  if ( v57[4] )
  {
    if ( (*a3 < 0.25) | __UNORDERED__(*a3, 0.25) )
      *a3 = 0.25;
  }
  return v52;
}

/* ---- R_PickFinalLights  0x004B69F0 ----  VERIFIED */
void __cdecl R_PickFinalLights(int a1, int a2, float *a3, int a4, int a5, float a6, int a7, int a8)
{
  int v8;
  int v9;
  int v10;
  _DWORD *v11;
  float *v12;
  double v13;
  double v14;
  int v15;
  int v16;
  double v17;
  double v18;
  double v19;
  int v20;
  int v21;
  int v22;
  float *v23;
  char *v24;
  double v25;
  int v27;
  int v28;
  int v29;
  int i;
  int v31;
  int v32;
  _DWORD *v33;
  int v34;
  int v35;
  int v36;
  int v37;
  _DWORD *v38;
  int v39;
  cvar_t *v40;
  int v41;
  int v42;
  int v43;
  float v44;
  float v45;
  float v46;
  float v47;
  float v48;
  float v49;
  _DWORD v50[8]; // [esp+2Ch] [ebp-E4h] BYREF
  float v51[49]; // [esp+4Ch] [ebp-C4h] BYREF

  v8 = a2;
  *(_BYTE *)(a1 + 160) = 0;
  if ( a2 && (v42 = 0, *(int *)(a2 + 352) > 0) )
  {
    v9 = a5;
    v10 = a4;
    v41 = 0;
    v11 = (_DWORD *)(a5 + 4 * a4);
    while ( 1 )
    {
      v12 = (float *)(v41 + *(_DWORD *)(v8 + 356));
      v47 = *a3 - v12[17];
      v48 = a3[1] - v12[18];
      v44 = v12[29];
      v49 = a3[2] - v12[19];
      v13 = v49 * v49 + v48 * v48 + v47 * v47;
      if ( !((v44 * v44 * 4.0 < v13) | __UNORDERED__(v44 * v44 * 4.0, v13)) )
      {
        *(_DWORD *)((char *)v11 + a7 - a5) = v12;
        *v11 = 1065353216;
        ++v10;
        ++v11;
        *(_BYTE *)(a1 + 160) = 1;
      }
      v41 += 136;
      if ( ++v42 >= *(_DWORD *)(a2 + 352) )
        break;
      v8 = a2;
    }
    a4 = v10;
  }
  else
  {
    v10 = a4;
    v9 = a5;
  }
  if ( !((a6 == 0.0) | __UNORDERED__(a6, 0.0))
    && !((*(float *)(tr_world + 264) == 0.0) | __UNORDERED__(*(float *)(tr_world + 264), 0.0)) )
  {
    if ( tr_diffuseSunQuality )
    {
      if ( tr_diffuseSunQuality == 1 || v10 > tr_maxEntityLights )
      {
        *(_DWORD *)(a1 + 412) = 8;
        *(_DWORD *)(a1 + 416) = *(_DWORD *)(tr_world + 248);
        *(_DWORD *)(a1 + 420) = *(_DWORD *)(tr_world + 252);
        *(_DWORD *)(a1 + 424) = *(_DWORD *)(tr_world + 256);
        v16 = a1 + 412;
        *(_DWORD *)(a1 + 428) = *(_DWORD *)(tr_world + 260);
        *(_DWORD *)(a1 + 428) = *(_DWORD *)(tr_world + 264);
        *(float *)(a1 + 432) = *(float *)(tr_world + 248) * 0.5;
        *(float *)(a1 + 436) = *(float *)(tr_world + 252) * 0.5;
        *(float *)(a1 + 440) = *(float *)(tr_world + 256) * 0.5;
        *(float *)(a1 + 444) = *(float *)(tr_world + 260) * 0.5;
        *(float *)(a1 + 448) = *(float *)(tr_world + 248) * 0.5;
        *(float *)(a1 + 452) = *(float *)(tr_world + 252) * 0.5;
        *(float *)(a1 + 456) = *(float *)(tr_world + 256) * 0.5;
        v19 = *(float *)(tr_world + 260);
        *(_DWORD *)(a1 + 464) = 0;
        *(_DWORD *)(a1 + 468) = 0;
        *(_DWORD *)(a1 + 472) = 0;
        *(_DWORD *)(a1 + 476) = 0;
        *(float *)(a1 + 460) = v19 * 0.5;
        *(_DWORD *)(a1 + 480) = 0;
        *(_DWORD *)(a1 + 484) = 0;
        *(_DWORD *)(a1 + 488) = 1065353216;
        *(_DWORD *)(a1 + 492) = 0;
        *(_DWORD *)(a1 + 524) = 1127481344;
        *(float *)(v9 + 4 * v10) = a6;
      }
      else
      {
        *(_DWORD *)(a1 + 412) = 8;
        *(_DWORD *)(a1 + 416) = *(_DWORD *)(tr_world + 248);
        *(_DWORD *)(a1 + 420) = *(_DWORD *)(tr_world + 252);
        *(_DWORD *)(a1 + 424) = *(_DWORD *)(tr_world + 256);
        *(_DWORD *)(a1 + 428) = *(_DWORD *)(tr_world + 260);
        *(_DWORD *)(a1 + 428) = *(_DWORD *)(tr_world + 264);
        ++v10;
        *(float *)(a1 + 432) = *(float *)(tr_world + 248) * 0.75;
        *(float *)(a1 + 436) = *(float *)(tr_world + 252) * 0.75;
        *(float *)(a1 + 440) = *(float *)(tr_world + 256) * 0.75;
        *(float *)(a1 + 444) = *(float *)(tr_world + 260) * 0.75;
        *(float *)(a1 + 448) = *(float *)(tr_world + 248) * 0.25;
        *(float *)(a1 + 452) = *(float *)(tr_world + 252) * 0.25;
        *(float *)(a1 + 456) = *(float *)(tr_world + 256) * 0.25;
        v14 = *(float *)(tr_world + 260) * 0.25;
        *(_DWORD *)(a1 + 464) = 0;
        *(_DWORD *)(a1 + 468) = 0;
        *(_DWORD *)(a1 + 472) = 0;
        *(float *)(a1 + 460) = v14;
        *(_DWORD *)(a1 + 476) = 0;
        *(_DWORD *)(a1 + 480) = 0;
        *(_DWORD *)(a1 + 484) = 0;
        *(_DWORD *)(a1 + 488) = 1065353216;
        *(_DWORD *)(a1 + 492) = 0;
        *(_DWORD *)(a1 + 524) = 1127481344;
        *(float *)(v9 + 4 * v10 - 4) = a6;
        *(_DWORD *)(a7 + 4 * v10 - 4) = a1 + 412;
        *(_DWORD *)(a1 + 552) = *(_DWORD *)(a1 + 416);
        *(_DWORD *)(a1 + 556) = *(_DWORD *)(a1 + 420);
        v15 = *(_DWORD *)(a1 + 424);
        *(_DWORD *)(a1 + 548) = 8;
        *(_DWORD *)(a1 + 560) = v15;
        v16 = a1 + 548;
        v17 = *(float *)(tr_world + 264) * -0.25;
        *(_DWORD *)(a1 + 568) = 0;
        *(_DWORD *)(a1 + 572) = 0;
        *(_DWORD *)(a1 + 576) = 0;
        *(float *)(a1 + 564) = v17;
        *(_DWORD *)(a1 + 580) = 0;
        *(float *)(a1 + 584) = *(float *)(tr_world + 248) * -0.25;
        *(float *)(a1 + 588) = *(float *)(tr_world + 252) * -0.25;
        *(float *)(a1 + 592) = *(float *)(tr_world + 256) * -0.25;
        v18 = *(float *)(tr_world + 260);
        *(_DWORD *)(a1 + 600) = 0;
        *(_DWORD *)(a1 + 604) = 0;
        *(_DWORD *)(a1 + 608) = 0;
        *(_DWORD *)(a1 + 612) = 0;
        *(float *)(a1 + 596) = v18 * -0.25;
        *(_DWORD *)(a1 + 616) = 0;
        *(_DWORD *)(a1 + 620) = 0;
        *(_DWORD *)(a1 + 624) = -1082130432;
        *(_DWORD *)(a1 + 628) = 0;
        *(_DWORD *)(a1 + 660) = 1127481344;
        *(float *)(v9 + 4 * v10) = a6;
      }
      *(_DWORD *)(a7 + 4 * v10++) = v16;
      a4 = v10;
      a6 = 0.0;
    }
  }
  v20 = 0;
  v43 = 0;
  if ( v10 > 0 )
  {
    v21 = a5 - a7;
    v22 = a7 - (_DWORD)v51;
    v23 = v51;
    while ( 1 )
    {
      v24 = (char *)v23 + v22;
      if ( (*(float *)((char *)v23 + v22 + v21) == 0.0) | __UNORDERED__(*(float *)((char *)v23 + v22 + v21), 0.0) )
        goto LABEL_49;
      if ( *(float *)(*(_DWORD *)v24 + 16) < 0.0 )
      {
        *v23 = 1.0e19;
      }
      else
      {
        v25 = R_MaxLightIntensity((float *)*(_DWORD *)v24, a3);
        v27 = **(_DWORD **)v24;
        v45 = v25 * *(float *)((char *)v23 - (char *)v51 + a5);
        *v23 = v45;
        if ( v27 == 1 || v27 == 8 )
        {
          v46 = (float)(1 << tr_overbrightBits);
          *v23 = v46 * *(float *)(tr_world + 264) * a6 + v45;
        }
        if ( (*v23 < (double)r_minEntLightIntensity->value) | __UNORDERED__(*v23, r_minEntLightIntensity->value) )
          goto LABEL_49;
      }
      v28 = 0;
      if ( v20 < 4 )
      {
LABEL_36:
        while ( v28 < v20 )
        {
          if ( v51[v50[v28]] <= (double)*v23 )
            break;
          ++v28;
        }
      }
      else
      {
        v29 = 3;
        while ( v51[v50[v28]] > (double)*v23 )
        {
          if ( v51[v50[v28 + 1]] <= (double)*v23 )
          {
            ++v28;
            break;
          }
          if ( v51[v50[v28 + 2]] <= (double)*v23 )
          {
            v28 += 2;
            break;
          }
          if ( v51[v50[v28 + 3]] <= (double)*v23 )
          {
            v28 += 3;
            break;
          }
          v29 += 4;
          v28 += 4;
          if ( v29 >= v20 )
            goto LABEL_36;
        }
      }
      if ( v28 < 8 )
      {
        if ( v20 == 8 )
          v20 = 7;
        for ( i = v20; i > v28; --i )
          v50[i] = v50[i - 1];
        v50[v28] = v43;
        ++v20;
      }
LABEL_49:
      ++v23;
      if ( ++v43 >= a4 )
        break;
      v22 = a7 - (_DWORD)v51;
      v21 = a5 - a7;
    }
  }
  v31 = 0;
  *(_DWORD *)(a1 + 204) = v20;
  if ( v20 >= 4 )
  {
    v32 = 3;
    v33 = (_DWORD *)(a1 + 208);
    do
    {
      v34 = 4 * v50[v31];
      v33[1] = *(_DWORD *)(v34 + a5);
      *v33 = *(_DWORD *)(v34 + a7);
      v35 = 4 * v50[v31 + 1];
      v33[3] = *(_DWORD *)(v35 + a5);
      v33[2] = *(_DWORD *)(v35 + a7);
      v36 = 4 * v50[v31 + 2];
      v33[5] = *(_DWORD *)(v36 + a5);
      v33[4] = *(_DWORD *)(v36 + a7);
      v37 = 4 * v50[v31 + 3];
      v33[7] = *(_DWORD *)(v37 + a5);
      v33[6] = *(_DWORD *)(v37 + a7);
      v32 += 4;
      v31 += 4;
      v33 += 8;
    }
    while ( v32 < v20 );
  }
  if ( v31 < v20 )
  {
    v38 = (_DWORD *)(a1 + 8 * v31 + 208);
    do
    {
      v39 = 4 * v50[v31];
      v38[1] = *(_DWORD *)(v39 + a5);
      *v38 = *(_DWORD *)(v39 + a7);
      ++v31;
      v38 += 2;
    }
    while ( v31 < v20 );
  }
  v40 = r_showLeafLights;
  *(float *)(a1 + 272) = a6;
  if ( v40->integer > 0 )
    R_ShowLeafLights(a3, a1);
  if ( a8 )
    R_MergeLights(a1, a3, (int)v51, v50);
}


/* ---- R_PickLights  0x004B7290 ----  VERIFIED */
void __cdecl R_PickLights(int a1, float *a2, int a3, int a4)
{
  int StaticLightContributions;
  float v6; // [esp+8h] [ebp-194h] BYREF
  char v7[200]; // [esp+Ch] [ebp-190h] BYREF
  char v8[200]; // [esp+D4h] [ebp-C8h] BYREF

  if ( tr_world && *(_DWORD *)(tr_world + 152) && !r_entFullbright->integer )
  {
    StaticLightContributions = R_GetStaticLightContributions(a2, v8, &v6, v7);
    R_PickFinalLights(a1, a3, a2, StaticLightContributions, (int)v8, v6, (int)v7, a4);
  }
  else
  {
    *(_DWORD *)(a1 + 204) = 0;
  }
}


/* ---- R_SetupEntityLighting  0x004B7320 ----  VERIFIED */
char __cdecl R_SetupEntityLighting(int a1, int a2)
{
  int integer;
  char v3;
  float v4;
  float v5;
  float v6;
  float v8[3]; // [esp+8h] [ebp-30h] BYREF
  float v9[9]; // [esp+14h] [ebp-24h] BYREF

  LOBYTE(integer) = *(_BYTE *)(a1 + 161);
  if ( !(_BYTE)integer )
  {
    v3 = *(_BYTE *)(a1 + 4);
    *(_BYTE *)(a1 + 161) = 1;
    if ( v3 >= 0 )
    {
      v4 = *(float *)(a1 + 68);
      v5 = *(float *)(a1 + 72);
      v6 = *(float *)(a1 + 76);
    }
    else
    {
      v4 = *(float *)(a1 + 12);
      v5 = *(float *)(a1 + 16);
      v6 = *(float *)(a1 + 20);
    }
    v8[0] = v4;
    v8[1] = v5;
    v8[2] = v6;
    R_PickLights(a1, v8, a2, 1);
    integer = cg_shadows->integer;
    if ( integer == 2 )
    {
      LOBYTE(integer) = (unsigned __int8)MatrixInverse((float *)(a1 + 28), v9);
      *(float *)(a1 + 164) = *(float *)&tr_sunDirection * v9[0];
      *(float *)(a1 + 168) = *(float *)&tr_sunDirection * v9[1];
      *(float *)(a1 + 172) = *(float *)&tr_sunDirection * v9[2];
      *(float *)(a1 + 164) = *(float *)&dword_16C57D4 * v9[3] + *(float *)(a1 + 164);
      *(float *)(a1 + 168) = *(float *)&dword_16C57D4 * v9[4] + *(float *)(a1 + 168);
      *(float *)(a1 + 172) = *(float *)&dword_16C57D4 * v9[5] + *(float *)(a1 + 172);
      *(float *)(a1 + 164) = *(float *)&dword_16C57D8 * v9[6] + *(float *)(a1 + 164);
      *(float *)(a1 + 168) = *(float *)&dword_16C57D8 * v9[7] + *(float *)(a1 + 168);
      *(float *)(a1 + 172) = *(float *)&dword_16C57D8 * v9[8] + *(float *)(a1 + 172);
    }
  }
  return integer;
}


/* ---- R_SetupStaticModelLighting  0x004B7450 ----  VERIFIED */
char __cdecl R_SetupStaticModelLighting(int a1, int a2)
{
  char result;
  int v4;
  float v5[9]; // [esp+8h] [ebp-24h] BYREF

  if ( r_showLeafLights->integer < 0 )
    return R_SetupEntityLighting(a2, a1);
  result = *(_BYTE *)(a2 + 161);
  if ( !result )
  {
    v4 = *(_DWORD *)(a2 + 688);
    *(_BYTE *)(a2 + 161) = 1;
    R_PickFinalLights(a2, a1, (float *)(a2 + 12), *(_DWORD *)(v4 + 184), v4 + 192, *(float *)(v4 + 188), v4 + 388, 1);
    result = (char)cg_shadows;
    if ( cg_shadows->integer == 2 )
    {
      result = (unsigned __int8)MatrixInverse((float *)(a2 + 28), v5);
      *(float *)(a2 + 164) = *(float *)&tr_sunDirection * v5[0];
      *(float *)(a2 + 168) = *(float *)&tr_sunDirection * v5[1];
      *(float *)(a2 + 172) = *(float *)&tr_sunDirection * v5[2];
      *(float *)(a2 + 164) = *(float *)&dword_16C57D4 * v5[3] + *(float *)(a2 + 164);
      *(float *)(a2 + 168) = *(float *)&dword_16C57D4 * v5[4] + *(float *)(a2 + 168);
      *(float *)(a2 + 172) = *(float *)&dword_16C57D4 * v5[5] + *(float *)(a2 + 172);
      *(float *)(a2 + 164) = *(float *)&dword_16C57D8 * v5[6] + *(float *)(a2 + 164);
      *(float *)(a2 + 168) = *(float *)&dword_16C57D8 * v5[7] + *(float *)(a2 + 168);
      *(float *)(a2 + 172) = *(float *)&dword_16C57D8 * v5[8] + *(float *)(a2 + 172);
    }
  }
  return result;
}


/* ---- R_VC_stats_f  0x004B7590 ----  VERIFIED */
void R_VC_stats_f()
{
  ri_Printf(0, "light visibility cache performance:\n");
  ri_Printf(0, "%i entries used (%.1f%%)\n", lightVisCache_entriesUsed, (double)lightVisCache_entriesUsed * 0.00038146973);
  ri_Printf(0, "%i max associativity\n", lightVisCache_maxAssociativity);
  ri_Printf(0, "%i entries flushed\n", lightVisCache_entriesFlushed);
  ri_Printf(0, "%i entries filled in at runtime instead of read from disk\n", lightVisCache_entriesFilledAtRuntime);
}

/* ---- R_LightVisHistoryFilename  0x004B7600 ----  VERIFIED */
int __cdecl R_LightVisHistoryFilename(char *a1)
{
  int v1;
  char v3;
  char *i;
  char *v5;
  int result;

  v1 = tr_world;
  v3 = *(_BYTE *)tr_world;
  for ( i = a1; v3; ++v1 )
  {
    if ( v3 == 46 )
      break;
    *i = v3;
    v3 = *(_BYTE *)(v1 + 1);
    ++i;
  }
  *i = 0;
  if ( strlen(a1) + 6 >= 0x40 )
    ri_Error(1, "\x15" "light vis cache log filename '%s.vclog' is too long\n", a1);
  v5 = &a1[strlen(a1)];
  result = 1818457646;
  strcpy(v5, ".vclog");
  return result;
}

/* ---- R_InitLightVisHistory  0x004B7680 ----  VERIFIED */
cvar_t *R_InitLightVisHistory()
{
  cvar_t *result;
  unsigned int v1;
  int v2;
  _DWORD *v3;
  int v4;
  int v5;
  int v6;
  int v7;
  __int16 *v8;
  const void *v9; // [esp+4h] [ebp-8Ch] BYREF
  char v10; // [esp+Bh] [ebp-85h] BYREF
  unsigned int v11;
  _DWORD v12[15]; // [esp+10h] [ebp-80h] BYREF
  char v13[64]; // [esp+4Ch] [ebp-44h] BYREF
  unsigned int v14;
  unsigned int retaddr;

  v14 = retaddr ^ _security_cookie;
  result = r_vc_makelog;
  lightVisHistoryCount = 0;
  sortedLightVisHistoryCount = 0;
  if ( r_vc_makelog->integer )
  {
    lightVisHistory = Z_MallocInternal(0x6000000u);
    sortedLightVisHistory = Z_MallocInternal(0x2000000u);
    result = (cvar_t *)r_vc_makelog->integer;
    if ( result == (cvar_t *)2 )
    {
      R_LightVisHistoryFilename(v13);
      v1 = ri_FS_ReadFile(v13, &v9);
      if ( !(v1 % 0x18) )
      {
        if ( v1 > 0x6000000 )
          v1 = 100663296;
        qmemcpy(lightVisHistory, v9, v1);
        if ( (int)(v1 / 0x18) > 0 )
        {
          v2 = 0;
          v11 = v1 / 0x18;
          do
          {
            v3 = R_PointInLeaf((float *)((char *)v9 + v2 + 12));
            v4 = v3[3];
            if ( v4 >= 0 )
            {
              v5 = v3[6];
              v6 = 0;
              if ( v5 > 0 )
              {
                v7 = *(_DWORD *)(tr_world + 272);
                v8 = (__int16 *)(*(_DWORD *)(tr_world + 284) + 2 * v3[5]);
                do
                  v12[v6++] = v7 + 136 * *v8++;
                while ( v6 < v5 );
              }
              R_GetCachedVisibility(
                v4,
                *(_DWORD *)((char *)v9 + v2 + 4),
                *(_DWORD *)((char *)v9 + v2),
                *(_DWORD *)((char *)v9 + v2 + 8),
                v5,
                (int)v12,
                (char *)v9 + v2 + 12,
                &v10);
            }
            v2 += 24;
            --v11;
          }
          while ( v11 );
        }
      }
      return (cvar_t *)ri_FS_FreeFile(v9);
    }
  }
  else
  {
    lightVisHistory = 0;
    sortedLightVisHistory = 0;
  }
  return result;
}


/* ---- R_SaveLightVisHistory  0x004B7840 ----  VERIFIED */
void R_SaveLightVisHistory()
{
  char v0[64]; // [esp+0h] [ebp-44h] BYREF
  unsigned int v1;
  unsigned int retaddr;

  v1 = retaddr ^ _security_cookie;
  if ( lightVisHistory )
  {
    if ( tr_world )
    {
      R_LightVisHistoryFilename(v0);
      ri_FS_WriteFile(v0, lightVisHistory, 24 * lightVisHistoryCount);
      free(lightVisHistory);
      free(sortedLightVisHistory);
      lightVisHistory = 0;
      sortedLightVisHistory = 0;
      lightVisHistoryCount = 0;
      sortedLightVisHistoryCount = 0;
    }
  }
}

/* ---- R_PrecalcLightVisCachePoint  0x004B78D0 ----  [HIGH] */
int __cdecl R_PrecalcLightVisCachePoint(int a1, int a2, int a3, float *a4, int a5)
{
  _DWORD *v5;
  int result;
  int v7;
  int v8;
  int v9;
  __int16 *v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  char v16;
  int v17;
  char v18;
  int v19;
  _BYTE v20[8]; // [esp+Ch] [ebp-50h] BYREF
#define v21 (*(char *)&v20[4])       /* [ebp-4Ch] */
#define v22 (*(char *)&v20[5])       /* [ebp-4Bh] */
#define v23 (*(__int16 *)&v20[6])    /* [ebp-4Ah] */
  float v24[3]; // [esp+14h] [ebp-48h] BYREF
  _DWORD v25[15]; // [esp+20h] [ebp-3Ch] BYREF

  v5 = R_PointInLeaf(a4);
  result = v5[3];
  if ( result >= 0 )
  {
    v7 = v5[6];
    v8 = 0;
    if ( v7 > 0 )
    {
      v9 = *(_DWORD *)(tr_world + 272);
      v10 = (__int16 *)(*(_DWORD *)(tr_world + 284) + 2 * v5[5]);
      do
        v25[v8++] = v9 + 136 * *v10++;
      while ( v8 < v7 );
    }
    v11 = result & 0xFFF | (((a2 << 10) | a1 & 0x3FF) << 12);
    v12 = (3137 * (_WORD)a2 - 3133 * (_WORD)a1 + (unsigned __int16)reverse_bits(a3)) & 0x1FFF;
    v13 = 0;
    result = a5 + 384 * v12;
    while ( *(_BYTE *)(result + 4) )
    {
      if ( *(_DWORD *)result != v11 )
      {
        ++v13;
        result += 12;
        if ( v13 < 32 )
          continue;
      }
      return result;
    }
    v24[0] = (float)(32 * (a1 - 4096));
    v24[1] = (float)(32 * (a2 - 4096));
    tr_diffuseSunSteps = 1;
    v14 = v5[6];
    v24[2] = (float)((a3 - 2048) << 6);
    v15 = R_SampleLightVisibility(v24, (int)v20, v14, (int)v25, (int)a4);
    v16 = v21;
    v19 = v15;
    v17 = a5 + 12 * (v13 + 32 * v12);
    LOWORD(v15) = v23;
    v18 = v22;
    *(_DWORD *)v17 = v11;
    *(_BYTE *)(v17 + 4) = v16;
    *(_BYTE *)(v17 + 5) = v18;
    *(_WORD *)(v17 + 10) = v15;
    result = v5[4];
    if ( result )
    {
      for ( result = ++tr_diffuseSunSteps; tr_diffuseSunSteps <= 5; ++tr_diffuseSunSteps )
      {
        if ( v21 != 2 )
        {
          v22 = 0;
          R_SampleDiffuseSunVisibility(v19, v17, v24, (int)v20);
        }
        *(_BYTE *)(tr_diffuseSunSteps + v17 + 4) = v22;
        result = tr_diffuseSunSteps + 1;
      }
    }
  }
  return result;
}
#undef v21
#undef v22
#undef v23

/* ---- R_PrecalcLightVisCache  0x004B7AB0 ----  VERIFIED */
int __cdecl R_PrecalcLightVisCache(int a1)
{
  int v1;
  signed int v2;
  int v3;
  int v4;
  int v5;
  int v7; // [esp+0h] [ebp-48h] BYREF
  char v8[64]; // [esp+4h] [ebp-44h] BYREF
  unsigned int v9;
  unsigned int retaddr;

  v9 = retaddr ^ _security_cookie;
  if ( !*(_DWORD *)(tr_world + 280) )
    ri_Error(1, "\x15" "cannot precalculate light vis cache; no lights compiled into map '%s'\n", tr_world);
  R_LightVisHistoryFilename(v8);
  v1 = ri_FS_ReadFile(v8, &v7);
  if ( v1 <= 0 )
    ri_Error(1, "\x15" "light vis cache file '%s' is missing or empty\n", v8);
  v2 = v1 / 0x18u;
  if ( v1 % 0x18u )
    ri_Error(1, "\x15" "light vis cache has funny size\n", v8);
  v3 = tr_diffuseSunSteps;
  v4 = ri_Malloc(3145728);
  if ( v2 > 0 )
  {
    v5 = 0;
    do
    {
      R_PrecalcLightVisCachePoint(
        *(_DWORD *)(v5 + v7),
        *(_DWORD *)(v5 + v7 + 4),
        *(_DWORD *)(v5 + v7 + 8),
        (float *)(v5 + v7 + 12),
        v4);
      v5 += 24;
      --v2;
    }
    while ( v2 );
  }
  tr_diffuseSunSteps = v3;
  ri_FS_FreeFile(v7);
  ri_CM_SaveLump(32, v4, 3145728, a1);
  ri_Free(v4);
  if ( r_vc_compile->integer == 2 )
    ri_Cmd_ExecuteText(0, "quit");
  return ri_Cvar_Set("r_vc_compile", "0");
}


/* ---- R_InitLightVisCacheFromBuffer  0x004B7C00 ----  VERIFIED */
int __cdecl R_InitLightVisCacheFromBuffer(int a1, int a2)
{
  int v2;
  int v3;
  int v4;
  _DWORD *v5;
  _BYTE *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  _BYTE *v13;
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

  v2 = 0;
  if ( !r_vc_makelog->integer && !r_vc_compile->integer )
  {
    if ( a1 )
    {
      if ( a2 == 3145728 )
      {
        v14 = -12 - tr_diffuseSunSteps;
        v15 = -6 - tr_diffuseSunSteps;
        v16 = -4 - tr_diffuseSunSteps;
        v17 = 6 - tr_diffuseSunSteps;
        v18 = 8 - tr_diffuseSunSteps;
        v19 = 12 - tr_diffuseSunSteps;
        v20 = 18 - tr_diffuseSunSteps;
        v21 = 20 - tr_diffuseSunSteps;
        v22 = 24 - tr_diffuseSunSteps;
        v23 = 30 - tr_diffuseSunSteps;
        v3 = 0;
        v13 = (_BYTE *)(a1 + 16);
        lightVisCache_entriesUsed = 0;
        lightVisCache_maxAssociativity = 0;
        lightVisCache_entriesFlushed = 0;
        lightVisCache_entriesFilledAtRuntime = 0;
        v4 = 2;
        v5 = (_DWORD *)((char *)&tr_lightVisCache + 4);
        v6 = (_BYTE *)(tr_diffuseSunSteps + a1 + 16);
        v7 = -16 - tr_diffuseSunSteps;
        do
        {
          *(v5 - 1) = *(_DWORD *)&v6[v7];
          *(_BYTE *)v5 = v6[v14];
          *((_BYTE *)v5 + 1) = *(v6 - 12);
          *((_WORD *)v5 + 1) = *(_WORD *)&v6[v15];
          if ( *(_BYTE *)v5 )
          {
            v8 = ((_BYTE)v4 - 2) & 0x1F;
            ++v3;
            if ( v2 <= v8 )
              v2 = v8 + 1;
          }
          v5[1] = *(_DWORD *)&v6[v16];
          *((_BYTE *)v5 + 8) = *v13;
          *((_BYTE *)v5 + 9) = *v6;
          *((_WORD *)v5 + 5) = *(_WORD *)&v6[v17];
          if ( *((_BYTE *)v5 + 8) )
          {
            v9 = ((_BYTE)v4 - 1) & 0x1F;
            ++v3;
            if ( v2 <= v9 )
              v2 = v9 + 1;
          }
          v5[3] = *(_DWORD *)&v6[v18];
          *((_BYTE *)v5 + 16) = v6[v19];
          *((_BYTE *)v5 + 17) = v6[12];
          *((_WORD *)v5 + 9) = *(_WORD *)&v6[v20];
          if ( *((_BYTE *)v5 + 16) )
          {
            v10 = v4 & 0x1F;
            ++v3;
            if ( v2 <= v10 )
              v2 = v10 + 1;
          }
          v5[5] = *(_DWORD *)&v6[v21];
          *((_BYTE *)v5 + 24) = v6[v22];
          *((_BYTE *)v5 + 25) = v6[24];
          *((_WORD *)v5 + 13) = *(_WORD *)&v6[v23];
          if ( *((_BYTE *)v5 + 24) )
          {
            v11 = ((_BYTE)v4 + 1) & 0x1F;
            ++v3;
            if ( v2 <= v11 )
              v2 = v11 + 1;
          }
          v4 += 4;
          v13 += 48;
          v6 += 48;
          v5 += 8;
        }
        while ( v4 - 2 < 0x40000 );
        lightVisCache_entriesUsed = v3;
        lightVisCache_maxAssociativity = v2;
      }
    }
  }
  return 1;
}


/* ---- R_AddSortedHistoryEntry  0x004B7E10 ----  VERIFIED */
int __cdecl R_AddSortedHistoryEntry(_DWORD *a1, int a2)
{
  if ( sortedLightVisHistoryCount >= 0x400000 )       /* 0x004B7E15: cmp eax, 400000h */
    return 0;
  memcpy((char *)sortedLightVisHistory + 8 * a2 + 8, (char *)sortedLightVisHistory + 8 * a2, 8 * (sortedLightVisHistoryCount - a2));
  *((_DWORD *)sortedLightVisHistory + 2 * a2) = *a1;
  *((_DWORD *)sortedLightVisHistory + 2 * a2 + 1) = a1[1];
  ++sortedLightVisHistoryCount;
  return 1;
}

/* ---- R_SortedHistoryEntry  0x004B7E70 ----  VERIFIED */
int __cdecl R_SortedHistoryEntry(unsigned __int16 a1, __int16 a2, __int16 a3, int a4)
{
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v11; // [esp+10h] [ebp-8h] BYREF
  unsigned __int16 v12;

  HIWORD(v11) = a3;
  v5 = sortedLightVisHistoryCount - 1;
  v6 = 0;
  LOWORD(v11) = a2;
  v12 = a1;
  if ( sortedLightVisHistoryCount - 1 < 0 )
  {
LABEL_8:
    if ( a4 && R_AddSortedHistoryEntry(&v11, v6) )
      return v6;
    else
      return -1;
  }
  else
  {
    while ( 1 )
    {
      v7 = (v6 + v5) >> 1;
      v8 = v11 - *((_DWORD *)sortedLightVisHistory + 2 * v7);
      if ( v11 == *((_DWORD *)sortedLightVisHistory + 2 * v7) )
      {
        v9 = *((unsigned __int16 *)sortedLightVisHistory + 4 * v7 + 2);
        v8 = a1 - v9;
        if ( a1 == v9 )
          break;
      }
      if ( v8 >= 0 )
        v6 = v7 + 1;
      else
        v5 = v7 - 1;
      if ( v6 > v5 )
        goto LABEL_8;
    }
    if ( a4 == 2 )
      R_AddSortedHistoryEntry(&v11, v7);
    return v7;
  }
}

/* ---- R_ShowLightVisCachePoints  0x004B7F20 ----  VERIFIED */
int R_ShowLightVisCachePoints()
{
  int result;
  int integer;
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int *v10;
  int v11;
  int v12;
  int v13;
  int v14;
  float v15;
  float v16;
  float v17;
  int v18;
  int v19;
  int v20;
  int v21;
  int v22;
  int v23;
  float v24[3]; // [esp+48h] [ebp-Ch] BYREF -- ONE vec3_t

  result = (int)lightVisHistory;
  if ( lightVisHistory )
  {
    result = (int)r_vc_showlog;
    integer = r_vc_showlog->integer;
    v22 = integer;
    if ( integer > 0 )
    {
      v15 = tr_viewParms_originX - -131072.0;
      v2 = (int)(v15 + 9.313225746154785e-10) >> 5;
      v21 = v2;
      v16 = tr_viewParms_originY - -131072.0;
      v3 = (int)(v16 + 9.313225746154785e-10) >> 5;
      v20 = v3;
      v17 = tr_viewParms_originZ - -131072.0;
      v24[0] = (float)(32 * (v2 - 4096));
      v4 = (int)(v17 - 0.4999999990686774) >> 6;
      v24[1] = (float)(32 * (v3 - 4096));
      v5 = -1;
      v23 = v4;
      v12 = -1;
      v24[2] = (float)((v4 - 2048) << 6);
      do
      {
        result = v5 + v4;
        v13 = v5 + v4;
        if ( v5 + v4 >= 0 )
        {
          result <<= 6;
          if ( result <= 0x80000 )
          {
            v6 = -integer;
            v19 = -integer;
            if ( -integer <= integer )
            {
              v7 = v3 - integer;
              result = 2 * integer + 1;
              v14 = v3 - integer;
              v18 = result;
              do
              {
                if ( v7 >= 0 && 32 * v7 <= 0x80000 && v6 <= integer )
                {
                  v8 = v6 + v2;
                  v11 = 32 * (v6 + v2 - 4096);
                  v9 = integer - v6 + 1;
                  do
                  {
                    if ( v8 >= 0 && 32 * v8 <= 0x80000 && R_SortedHistoryEntry(v13, v8, v7, 0) < 0 )
                    {
                      v24[0] = (float)v11;
                      v24[1] = (float)(32 * (v7 - 4096));
                      v24[2] = (float)((v13 - 2048) << 6);
                      if ( !R_CullPointAndRadius(v24, 0.0f) )
                      {
                        if ( !v12 || (v10 = dword_541870, v12 == 1) )
                          v10 = dword_541840;
                        R_AddDebugString((int *)v24, v10, 1065353216, ".");
                        v7 = v14;
                        v6 = v19;
                      }
                    }
                    ++v8;
                    --v9;
                    v11 += 32;
                  }
                  while ( v9 );
                  v3 = v20;
                  v2 = v21;
                  integer = v22;
                  result = v18;
                }
                ++v7;
                --result;
                v14 = v7;
                v18 = result;
              }
              while ( result );
              v5 = v12;
              v4 = v23;
            }
          }
        }
        v12 = ++v5;
      }
      while ( v5 <= 1 );
    }
  }
  return result;
}

