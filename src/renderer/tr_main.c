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


#define r_drawentities             ((cvar_t *)(r_drawentities))
#define r_fastsky                  ((cvar_t *)(r_fastsky))
#define r_fog                      ((cvar_t *)(r_fog))
#define r_highLodDist              ((cvar_t *)(r_highLodDist))
#define r_lodViewDist              ((cvar_t *)(r_lodViewDist))
#define r_lowLodDist               ((cvar_t *)(r_lowLodDist))
#define r_mediumLodDist            ((cvar_t *)(r_mediumLodDist))
#define r_nocull                   ((cvar_t *)(r_nocull))
#define r_noportals                ((cvar_t *)(r_noportals))
#define r_portalOnly               ((cvar_t *)(r_portalOnly))
#define r_speeds                   ((cvar_t *)(r_speeds))
#define r_vc_makelog               ((cvar_t *)(r_vc_makelog))
#define r_zfar                     ((cvar_t *)(r_zfar))
#define r_znear                    ((cvar_t *)(r_znear))
#define r_znear_depthhack          ((cvar_t *)(r_znear_depthhack))


void PerpendicularVector( float *dst, const float *src );
extern int PlaneFromPoints();
extern int RB_BeginSurface();
extern int RB_SelectStorageATI();
extern int RB_SelectStorageNV();
extern int RB_SurfaceBad();
extern int RB_SurfaceEntity();
extern int RB_SurfaceOptimized();
extern int RB_SurfacePolychain();
extern int RB_SurfaceStaticModel();
extern int RB_SurfaceStaticModelATI();
extern int RB_SurfaceStaticModelCached();
extern int RB_SurfaceStaticModelT2N3V3_ARB();
extern int RB_SurfaceStaticModelT2N3V3_Generic();
extern int RB_SurfaceStaticModelT2N3V3_NV();
extern int RB_SurfaceStaticModelT2V3_ARB();
extern int RB_SurfaceStaticModelT2V3_Generic();
extern int RB_SurfaceStaticModelT2V3_NV();
extern int RB_SurfaceTriangles();
extern int RB_SurfaceXModelRigid();
extern int RB_SurfaceXModelRigidARB();
extern int RB_SurfaceXModelRigidATI();
extern int RB_SurfaceXModelRigidNV();
extern int RB_SurfaceXModelRigidSSE();
extern int RB_SurfaceXModelWeight();
extern int RB_SurfaceXModelWeightSSE();
extern int R_AddBrushModelSurfaces();
extern int R_AddDrawSurfCmd();
extern int R_AddPolygonSurfaces();
extern int R_AddStaticModelSurfaces();
extern int R_AddWorldSurfacesDPVS();
extern int R_AddXModelSurfaces();
extern int R_SetPlaneSidesDPVS();
_DWORD *__cdecl R_RenderView( _DWORD *a1 );
extern int R_ShowLightVisCachePoints();
extern float         xmodel_testLodDist;             /* 0x00A9CE10 */
extern unsigned char xmodel_testLodsEnabled;         /* 0x00A9CE14 */
extern float         xmodel_testLodDistances[15];    /* 0x00A9CE18 */
void RotatePointAroundVector( float *dst, const float *dir, const float *point, float degrees );
extern int nullsub_2();

void R_FogOn(void);

int R_SetFogColor(void);
int __cdecl R_CullPointAndRadius(float *pt, float radius);

typedef void (__cdecl *rb_surfaceFunc_t)( void *surf );
#define rb_surfaceTable   ((rb_surfaceFunc_t *)(void *)funcs_4D4A37)

typedef struct {
    float         normal[3];   /* +0x00 */
    float         dist;        /* +0x0C */
    unsigned char type;        /* +0x10  always 3 (non-axial) */
    unsigned char signbits;    /* +0x11 */
    unsigned char pad[2];      /* +0x12 */
} tr_cplane_t;

#define tr_frustum          ((tr_cplane_t *)(void *)tr_viewParms_frustum)
#define TR_FRUSTUM_PLANES   4

/* ---- R_Fog  0x004D28E0 ----  VERIFIED */
void __cdecl R_Fog(int a1)
{
  double value;
  float v2;

  if ( r_fog->integer && *(_DWORD *)(a1 + 48) )
  {
    if ( (*(float *)(a1 + 44) == 0.0) | __UNORDERED__(*(float *)(a1 + 44), 0.0) )
      *(_DWORD *)(a1 + 44) = 1065353216;
    if ( !*(_DWORD *)(a1 + 4) )
      *(_DWORD *)(a1 + 4) = 4352;
    if ( !*(_DWORD *)a1 )
      *(_DWORD *)a1 = 9729;
    R_FogOn();
    if ( *(_DWORD *)a1 != glState_fogMode )
    {
      qglFogi(2917, *(_DWORD *)a1);
      glState_fogMode = *(_DWORD *)a1;
    }
    glState_fogColor = *(float *)(a1 + 16);
    flt_16C3964 = *(float *)(a1 + 20);
    flt_16C3968 = *(float *)(a1 + 24);
    dword_16C396C = *(_DWORD *)(a1 + 28);
    R_SetFogColor();
    if ( !((*(float *)(a1 + 44) == glState_fogDensity) | __UNORDERED__(*(float *)(a1 + 44), glState_fogDensity)) )
    {
      qglFogf(2914, *(float *)(a1 + 44));
      glState_fogDensity = *(float *)(a1 + 44);
    }
    if ( glState_fogHint != *(_DWORD *)(a1 + 4) )
    {
      qglHint(3156, *(_DWORD *)(a1 + 4));
      glState_fogHint = *(_DWORD *)(a1 + 4);
    }
    if ( !((glState_fogStart == *(float *)(a1 + 32)) | __UNORDERED__(glState_fogStart, *(float *)(a1 + 32))) )
    {
      qglFogf(2915, *(float *)(a1 + 32));
      glState_fogStart = *(float *)(a1 + 32);
    }
    if ( (r_zfar->value == 0.0) | __UNORDERED__(r_zfar->value, 0.0) )
      value = *(float *)(a1 + 36);
    else
      value = r_zfar->value;
    if ( !((value == glState_fogEnd) | __UNORDERED__(value, glState_fogEnd)) )
    {
      v2 = value;
      qglFogf(2916, v2);
      glState_fogEnd = value;
    }
  }
  else if ( (glState_glStateBits & 0x200000) != 0 )
  {
    glDisable(0xB60u);
    glState_glStateBits &= ~0x200000u;
  }
}


/* ---- R_FogOn  0x004D2A80 ----  VERIFIED */
void R_FogOn()
{
  int v0;

  if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
  {
    if ( r_fog->integer )
    {
      v0 = dword_16C4BB0;
      if ( (backEnd_refdef_rdflags & 8) == 0 )
        v0 = glfogNum;
      if ( v0 )
      {
        glEnable(0xB60u);
        glState_glStateBits |= 0x200000u;
      }
    }
  }
}


/* ---- R_SetFogColor  0x004D2AD0 ----  VERIFIED */
int R_SetFogColor()
{
  int result;
  float fogColor[4]; // [esp+0h] [ebp-10h] BYREF

  result = glfogNum;
  if ( glfogNum )
  {
    if ( (glState_glStateBits & 0xF0) == 0x20 )
    {
      fogColor[2] = 0.0;
      fogColor[1] = 0.0;
      fogColor[0] = 0.0;
    }
    else
    {
      fogColor[0] = tr_identityLight * glState_fogColor;
      fogColor[1] = flt_16C3964 * tr_identityLight;
      fogColor[2] = flt_16C3968 * tr_identityLight;
    }
    *(int *)&fogColor[3] = dword_16C396C;
    qglFogfv(2918, fogColor);
    return result;
  }
  return result;
}

/* ---- R_SetFog  0x004D2B50 ----  [HIGH] */
void __cdecl R_SetFog(int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
  int v7;
  int v8;
  int *v9;

  if ( a1 == 8 )
  {
    if ( a2 == 3 )
    {
      if ( dword_16C4C70 )
        qmemcpy(&dword_16C4C80, &dword_16C4C40, 0x40u);
      memset(&unk_16C4BC0, 0, 0x40u);
      memset(&dword_16C4CC0, 0, 0x40u);
      glfogNum = 0;
    }
    else
    {
      v8 = a2 << 6;
      if ( dword_16C4B30[16 * a2] == 1 )
      {
        glfogNum = a2;
        v9 = &dword_16C4C40;
        if ( !dword_16C4C70 )
          v9 = (int *)((char *)tr_fogTable + v8);
        qmemcpy(&dword_16C4C80, v9, 0x40u);
        qmemcpy(&dword_16C4CC0, (char *)tr_fogTable + v8, 0x40u);
        if ( a3 )
        {
          dword_16C4CC8 = tr_refdef_time;
          dword_16C4CCC = a3 + tr_refdef_time;
        }
        else
        {
          dword_16C4CFC = 1;
          dword_16C4C7C = 1;
          dword_16C4CC8 = 0;
          dword_16C4CCC = 0;
        }
      }
    }
  }
  else if ( a2 || a3 )
  {
    v7 = a1 << 6;
    *(int *)((char *)&dword_16C4B10 + v7) = a4;
    *(float *)((char *)&dword_16C4B10 + v7 + 0x10) = (float)a2;
    *(int *)((char *)&dword_16C4B14 + v7) = a5;
    *(int *)((char *)&dword_16C4B18 + v7) = a6;
    *(float *)((char *)&dword_16C4B10 + v7 + 0x14) = (float)a3;
    *(int *)((char *)&dword_16C4B1C + v7) = 1065353216;
    *(int *)((char *)&dword_16C4B38 + v7) = 0;
    *(int *)((char *)&dword_16C4B34 + v7) = 1;
    if ( *(float *)&a7 < 1.0 )
    {
      tr_fogTable[16 * a1] = 2048;
      dword_16C4B2C[16 * a1] = a7;
    }
    else
    {
      tr_fogTable[16 * a1] = 9729;
      dword_16C4B2C[16 * a1] = 1065353216;
    }
    dword_16C4B30[16 * a1] = 1;
    dword_16C4B04[16 * a1] = 4352;
  }
  else
  {
    dword_16C4B30[16 * a1] = 0;
  }
}

/* ---- RE_SaveFogState  0x004D2CF0 ----  VERIFIED */
int __cdecl RE_SaveFogState(_DWORD *a1, unsigned int a2)
{
  if ( a2 < 0x244 )
    ri_Error(1, "couldn't save fog settings (%i bytes available, %i bytes needed)\n", a2, 580);
  qmemcpy(a1, tr_fogTable, 0x240u);
  a1[144] = glfogNum;
  return 580;
}

/* ---- RE_RestoreFogState  0x004D2D40 ----  VERIFIED */
int __cdecl RE_RestoreFogState(_DWORD *a1, unsigned int a2)
{
  if ( a2 < 0x244 )
    ri_Error(1, "couldn't restore fog settings (savegame is probably corrupt or an old version)\n");
  qmemcpy(tr_fogTable, a1, 0x240u);
  glfogNum = a1[144];
  return 580;
}

/* ---- R_UpdateOverTime  0x004D2D90 ----  VERIFIED */
double __cdecl R_UpdateOverTime(float a1, float a2, int a3, int a4, int a5)
{
  double v5;
  double v7;

  if ( (a1 < (double)a2) | __UNORDERED__(a1, a2) )
  {
    if ( a3 <= 0 )
      return a2;
    v5 = (double)a5 / (double)a3 + a1;
    a1 = v5;
    if ( v5 > a2 )
      return a2;
  }
  else if ( a1 > (double)a2 )
  {
    if ( a4 <= 0 )
      return a2;
    v7 = a1 - (double)a5 / (double)a4;
    a1 = v7;
    if ( (v7 < a2) | __UNORDERED__(v7, a2) )
      return a2;
  }
  return a1;
}

/* ---- R_CullLocalBox  0x004D2E00 ----  VERIFIED */
int __cdecl R_CullLocalBox(int a1)
{
  int v3;
  float *v4;
  double v5;
  double v6;
  double v7;
  double v8;
  double v9;
  int v10;
  float *v11;
  int v12;
  int v13;
  int v14;
  float *v15;
  double v16;
  double v17;
  double v18;
  double v19;
  double v20;
  double v21;
  double v22;
  double v23;
  float v24[8];  /* retail dists[8]           [esp+4h]  */
  float v25[24]; /* retail transformed[8][3]  [esp+24h] */

  if ( r_nocull->integer )
    return 1;
  v3 = 0;
  v4 = v25 + 1;
  do
  {
    v5 = *(float *)(a1 + 12 * (v3 & 1));
    v6 = *(float *)(a1 + 12 * ((v3 >> 1) & 1) + 4);
    v7 = *(float *)(a1 + 12 * ((v3 >> 2) & 1) + 8);
    v8 = tr_or_axis20;
    *(v4 - 1) = tr_or;
    v9 = tr_or_axis10;
    *v4 = tr_or_originY;
    v4[1] = tr_or_originZ;
    ++v3;
    v4 += 3;
    *(v4 - 4) = v8 * v7 + v9 * v6 + *(float *)&tr_or_axis00 * v5 + *(v4 - 4);
    *(v4 - 3) = tr_or_axis21 * v7 + *(float *)&tr_or_axis11 * v6 + tr_or_axis01 * v5 + *(v4 - 3);
    *(v4 - 2) = *(float *)&tr_or_axis22 * v7 + tr_or_axis12 * v6 + tr_or_axis02 * v5 + *(v4 - 2);
  }
  while ( v3 < 8 );
  v10 = 0;
  v11 = &tr_frustum[0].normal[1];
  while ( 2 )
  {
    v12 = 0;
    v13 = 0;
    v14 = 0;
    v15 = v25 + 1;
    do
    {
      v16 = v15[1] * v11[1] + *v15 * *v11 + *(v15 - 1) * *(v11 - 1);
      v24[v14] = v16;
      if ( v16 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v17 = v15[4] * v11[1] + v15[3] * *v11 + v15[2] * *(v11 - 1);
      v24[v14 + 1] = v17;
      if ( v17 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v18 = v15[7] * v11[1] + v15[6] * *v11 + v15[5] * *(v11 - 1);
      v24[v14 + 2] = v18;
      if ( v18 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v19 = v15[10] * v11[1] + v15[9] * *v11 + v15[8] * *(v11 - 1);
      v24[v14 + 3] = v19;
      if ( v19 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v20 = v15[13] * v11[1] + v15[12] * *v11 + v15[11] * *(v11 - 1);
      v24[v14 + 4] = v20;
      if ( v20 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v21 = v15[16] * v11[1] + v15[15] * *v11 + v15[14] * *(v11 - 1);
      v24[v14 + 5] = v21;
      if ( v21 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v22 = v15[19] * v11[1] + v15[18] * *v11 + v15[17] * *(v11 - 1);
      v24[v14 + 6] = v22;
      if ( v22 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v23 = v15[22] * v11[1] + v15[21] * *v11 + v15[20] * *(v11 - 1);
      v24[v14 + 7] = v23;
      if ( v23 <= v11[2] )
      {
        v12 = 1;
      }
      else
      {
        v13 = 1;
        if ( v12 )
          goto LABEL_41;
      }
      v14 += 8;
      v15 += 24;
    }
    while ( v14 < 8 );
    if ( !v13 )
      return 2;
LABEL_41:
    v11 += 5;
    v10 |= v12;
    if ( (const char *)v11 < (const char *)tr_frustum + TR_FRUSTUM_PLANES * 20 )
      continue;
    return v10 != 0;
  }
}


float *__cdecl R_LocalPointToWorld(float *result, float *a2);

/* ---- R_CullLocalPointAndRadius  0x004D3100 ----  VERIFIED */
int __cdecl R_CullLocalPointAndRadius(float *a1, float a2)
{
  float v3[3]; // [esp+0h] [ebp-Ch] BYREF

  R_LocalPointToWorld(a1, v3);
  return R_CullPointAndRadius(v3, a2);
}

/* ---- R_CullPointAndRadius  0x004D3120 ----  VERIFIED */
int __cdecl R_CullPointAndRadius(float *a1, float a2)
{
  int v2;
  float *v4;
  double v5;
  float v6;

  v2 = 0;
  if ( r_nocull->integer )
    return 1;
  v4 = &tr_frustum[0].normal[1];
  while ( 1 )
  {
    v5 = *(v4 - 1) * *a1 + v4[1] * a1[2] + *v4 * a1[1] - v4[2];
    v6 = -a2;
    if ( (v5 < v6) | __UNORDERED__(v5, v6) )
      break;
    if ( v5 <= a2 )
      v2 = 1;
    v4 += 5;
    if ( (const char *)v4 >= (const char *)tr_frustum + TR_FRUSTUM_PLANES * 20 )
      return v2 != 0;
  }
  return 2;
}


/* ---- R_LocalNormalToWorld  0x004D31A0 ----  VERIFIED */
float *__cdecl R_LocalNormalToWorld(float *result, float *a2)
{
  *a2 = *(float *)&tr_or_axis00 * *result + tr_or_axis20 * result[2] + tr_or_axis10 * result[1];
  a2[1] = tr_or_axis01 * *result + tr_or_axis21 * result[2] + *(float *)&tr_or_axis11 * result[1];
  a2[2] = tr_or_axis02 * *result + *(float *)&tr_or_axis22 * result[2] + tr_or_axis12 * result[1];
  return result;
}

/* ---- R_LocalPointToWorld  0x004D3210 ----  VERIFIED */
float *__cdecl R_LocalPointToWorld(float *result, float *a2)
{
  *a2 = *(float *)&tr_or_axis00 * *result + tr_or_axis20 * result[2] + tr_or_axis10 * result[1] + tr_or;
  a2[1] = tr_or_axis01 * *result + tr_or_axis21 * result[2] + *(float *)&tr_or_axis11 * result[1] + tr_or_originY;
  a2[2] = tr_or_axis02 * *result + *(float *)&tr_or_axis22 * result[2] + tr_or_axis12 * result[1] + tr_or_originZ;
  return result;
}

/* ---- R_WorldToLocal  0x004D3290 ----  VERIFIED */
float *__cdecl R_WorldToLocal(float *result, float *a2)
{
  *a2 = *(float *)&tr_or_axis00 * *result + tr_or_axis02 * result[2] + tr_or_axis01 * result[1];
  a2[1] = tr_or_axis10 * *result + tr_or_axis12 * result[2] + *(float *)&tr_or_axis11 * result[1];
  a2[2] = tr_or_axis20 * *result + *(float *)&tr_or_axis22 * result[2] + tr_or_axis21 * result[1];
  return result;
}

/* ---- R_TransformModelToClip  0x004D3300 ----  VERIFIED */
float *__cdecl R_TransformModelToClip(
        float *result,
        float *a2,
        float *a3,
        float *a4,
        float *a5)
{
  *result = a2[4] * a5[1] + a2[8] * a5[2] + *a5 * *a2 + a2[12];
  result[1] = a2[5] * a5[1] + a2[1] * *a5 + a2[9] * a5[2] + a2[13];
  result[2] = a2[6] * a5[1] + a2[2] * *a5 + a2[10] * a5[2] + a2[14];
  result[3] = a2[7] * a5[1] + a2[3] * *a5 + a2[11] * a5[2] + a2[15];
  *a4 = a3[12] * result[3] + a3[8] * result[2] + a3[4] * result[1] + *a3 * *result;
  a4[1] = a3[13] * result[3] + a3[1] * *result + a3[9] * result[2] + a3[5] * result[1];
  a4[2] = a3[14] * result[3] + a3[2] * *result + a3[10] * result[2] + a3[6] * result[1];
  a4[3] = a3[15] * result[3] + a3[3] * *result + a3[11] * result[2] + a3[7] * result[1];
  return result;
}

/* ---- R_TransformHomogenousModelToClip  0x004D33F0 ----  VERIFIED */
float *__cdecl R_TransformHomogenousModelToClip(
        float *result,
        float *a2,
        float *a3,
        float *a4,
        float *a5)
{
  *result = a2[8] * a5[2] + a2[4] * a5[1] + a2[12] * a5[3] + *a5 * *a2;
  result[1] = a2[9] * a5[2] + a2[5] * a5[1] + a2[1] * *a5 + a2[13] * a5[3];
  result[2] = a2[10] * a5[2] + a2[6] * a5[1] + a2[2] * *a5 + a2[14] * a5[3];
  result[3] = a2[11] * a5[2] + a2[7] * a5[1] + a2[3] * *a5 + a2[15] * a5[3];
  *a4 = a3[8] * result[2] + a3[4] * result[1] + a3[12] * result[3] + *a3 * *result;
  a4[1] = a3[9] * result[2] + a3[5] * result[1] + a3[1] * *result + a3[13] * result[3];
  a4[2] = a3[10] * result[2] + a3[6] * result[1] + a3[2] * *result + a3[14] * result[3];
  a4[3] = a3[11] * result[2] + a3[7] * result[1] + a3[3] * *result + a3[15] * result[3];
  return result;
}

/* ---- R_TransformClipToWindow  0x004D34F0 ----  VERIFIED */
void __cdecl R_TransformClipToWindow(float *a1, int a2, float *a3, float *a4)
{
  double v4;

  *a3 = *a1 / a1[3];
  a3[1] = a1[1] / a1[3];
  a3[2] = (a1[2] + a1[3]) / (a1[3] + a1[3]);
  *a4 = (*a3 + 1.0) * (double)*(int *)(a2 + 304) * 0.5;
  a4[1] = (double)*(int *)(a2 + 308) * (a3[1] + 1.0) * 0.5;
  v4 = *a4;
  a4[2] = a3[2];
  *a4 = floor(v4 + 0.5);
  a4[1] = floor(a4[1] + 0.5);
}

/* ---- myGlMultMatrix  0x004D3580 ----  VERIFIED */
int __cdecl myGlMultMatrix(int a, float *b, int out)
{
  int result;
  int v4;
  int v5;
  const int    a1 = a;      /* EAX -- left-hand matrix, row-indexed  */
  const int    a2 = out;    /* EDX -- destination                    */
  float *const a3 = b;      /* ECX -- right-hand matrix, column-strided */

  result = a1 + 8;
  v4 = a2 + 8;
  v5 = 4;
  do
  {
    result += 16;
    v4 += 16;
    --v5;
    *(float *)(v4 - 24) = a3[12] * *(float *)(result - 12)
                        + a3[8] * *(float *)(result - 16)
                        + a3[4] * *(float *)(result - 20)
                        + *(float *)(result - 24) * *a3;
    *(float *)(v4 - 20) = a3[13] * *(float *)(result - 12)
                        + a3[9] * *(float *)(result - 16)
                        + a3[5] * *(float *)(result - 20)
                        + a3[1] * *(float *)(result - 24);
    *(float *)(v4 - 16) = a3[14] * *(float *)(result - 12)
                        + a3[10] * *(float *)(result - 16)
                        + a3[6] * *(float *)(result - 20)
                        + a3[2] * *(float *)(result - 24);
    *(float *)(v4 - 12) = a3[15] * *(float *)(result - 12)
                        + a3[11] * *(float *)(result - 16)
                        + a3[7] * *(float *)(result - 20)
                        + a3[3] * *(float *)(result - 24);
  }
  while ( v5 );
  return result;
}

/* ---- R_RotateForModelEntity  0x004D3630 ----  VERIFIED */
void __cdecl R_RotateForModelEntity( float *a1, float *a2, int a3 )
{
  float   viewDelta[3];
  float   axisLength[3];
  float   diff;
  unsigned int absBits;
  float  *outAxis;
  float  *entAxis;
  int     i;
  _DWORD  m[16];      /* [ebp-40h] BYREF -- the composed 4x4 */

  a1[0]  = *(float *)( a3 + 68 );
  a1[1]  = *(float *)( a3 + 72 );
  a1[2]  = *(float *)( a3 + 76 );
  a1[3]  = *(float *)( a3 + 28 );
  a1[4]  = *(float *)( a3 + 32 );
  a1[5]  = *(float *)( a3 + 36 );
  a1[6]  = *(float *)( a3 + 40 );
  a1[7]  = *(float *)( a3 + 44 );
  a1[8]  = *(float *)( a3 + 48 );
  a1[9]  = *(float *)( a3 + 52 );
  a1[10] = *(float *)( a3 + 56 );
  a1[11] = *(float *)( a3 + 60 );

  m[0]  = *((_DWORD *)a1 + 3);
  m[1]  = *((_DWORD *)a1 + 4);
  m[2]  = *((_DWORD *)a1 + 5);
  m[3]  = 0;
  m[4]  = *((_DWORD *)a1 + 6);
  m[5]  = *((_DWORD *)a1 + 7);
  m[6]  = *((_DWORD *)a1 + 8);
  m[7]  = 0;
  m[8]  = *((_DWORD *)a1 + 9);
  m[9]  = *((_DWORD *)a1 + 10);
  m[10] = *((_DWORD *)a1 + 11);
  m[11] = 0;
  m[12] = *((_DWORD *)a1 + 0);
  m[13] = *((_DWORD *)a1 + 1);
  m[14] = *((_DWORD *)a1 + 2);
  m[15] = 1065353216;                      /* 0x3F800000 = 1.0f */

  myGlMultMatrix( (int)m, a2 + 46, (int)( a1 + 15 ) );

  viewDelta[0] = a2[0] - a1[0];
  viewDelta[1] = a2[1] - a1[1];
  viewDelta[2] = a2[2] - a1[2];
  a1[12] = viewDelta[0] * a1[3]  + viewDelta[2] * a1[5]  + viewDelta[1] * a1[4];
  a1[13] = viewDelta[1] * a1[7]  + viewDelta[0] * a1[6]  + viewDelta[2] * a1[8];
  a1[14] = viewDelta[2] * a1[11] + viewDelta[1] * a1[10] + viewDelta[0] * a1[9];

  *(_DWORD *)( a3 + 684 ) = 0;             /* 0x004D3758 */

  if ( *(float *)( a3 + 64 ) != 0.0f )
  {
    outAxis = a1 + 12;
    entAxis = (float *)( a3 + 32 );
    for ( i = 0; i < 3; i++ )
    {
      axisLength[i] = (float)sqrt( entAxis[1] * entAxis[1]
                                 + entAxis[0] * entAxis[0]
                                 + entAxis[-1] * entAxis[-1] );
      if ( axisLength[i] != 0.0f )
        *outAxis = *outAxis / axisLength[i];
      entAxis += 3;
      ++outAxis;
    }

    *(_DWORD *)( a3 + 684 ) = 2977;

    if ( glConfig_rescaleNormal )
    {
      diff = axisLength[2] - axisLength[0];
      absBits = *(unsigned int *)&diff & 0x7FFFFFFF;
      if ( *(float *)&absBits < 0.001 )
      {
        diff = axisLength[2] - axisLength[1];
        absBits = *(unsigned int *)&diff & 0x7FFFFFFF;
        if ( *(float *)&absBits < 0.001 )
          *(_DWORD *)( a3 + 684 ) = 32826;
      }
    }
  }
}
#if 0
void __cdecl R_RotateForModelEntity(float *a1, float *a2, int a3)
{
  int v4;
  int v5;
  double v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  double v16;
  double v17;
  double v18;
  double v19;
  float *v20;
  int v21;
  float *v22;
  long double v23;
  unsigned __int8 v25; // c2
  unsigned __int8 v26; // c3
  float v27;
  float v28;
  float v29[2];
  float v30;
  _DWORD v31[16]; // [esp+18h] [ebp-40h] BYREF

  *a1 = *(float *)(a3 + 68);
  a1[1] = *(float *)(a3 + 72);
  a1[2] = *(float *)(a3 + 76);
  a1[3] = *(float *)(a3 + 28);
  a1[4] = *(float *)(a3 + 32);
  a1[5] = *(float *)(a3 + 36);
  a1[6] = *(float *)(a3 + 40);
  a1[7] = *(float *)(a3 + 44);
  a1[8] = *(float *)(a3 + 48);
  v4 = *((_DWORD *)a1 + 3);
  a1[9] = *(float *)(a3 + 52);
  v5 = *((_DWORD *)a1 + 6);
  a1[10] = *(float *)(a3 + 56);
  v6 = *(float *)(a3 + 60);
  v7 = *((_DWORD *)a1 + 9);
  a1[11] = *(float *)(a3 + 60);
  v31[0] = v4;
  *(float *)&v31[10] = v6;
  v8 = *(_DWORD *)a1;
  v31[4] = v5;
  v9 = *((_DWORD *)a1 + 4);
  v31[8] = v7;
  v10 = *((_DWORD *)a1 + 7);
  v31[12] = v8;
  v11 = *((_DWORD *)a1 + 10);
  v31[1] = v9;
  v12 = *((_DWORD *)a1 + 1);
  v31[5] = v10;
  v13 = *((_DWORD *)a1 + 5);
  v31[9] = v11;
  v14 = *((_DWORD *)a1 + 8);
  v31[13] = v12;
  v15 = *((_DWORD *)a1 + 2);
  v31[2] = v13;
  v31[6] = v14;
  v31[14] = v15;
  v31[3] = 0;
  v31[7] = 0;
  v31[11] = 0;
  v31[15] = 1065353216;
  /* myGlMultMatrix( a, b, out ) */
  myGlMultMatrix((int)v31, a2 + 46, (int)(a1 + 15));
  v16 = *a2 - *a1;
  v17 = a2[1] - a1[1];
  v18 = a2[2] - a1[2];
  a1[12] = v16 * a1[3] + v18 * a1[5] + v17 * a1[4];
  a1[13] = v17 * a1[7] + v16 * a1[6] + v18 * a1[8];
  a1[14] = v18 * a1[11] + v17 * a1[10] + v16 * a1[9];
  v19 = *(float *)(a3 + 64);
  *(_DWORD *)(a3 + 684) = 0;
  if ( !((v19 == 0.0) | __UNORDERED__(v19, 0.0)) )
  {
    v20 = a1 + 12;
    v21 = 0;
    v22 = (float *)(a3 + 32);
    do
    {
      v23 = sqrt(v22[1] * v22[1] + *v22 * *v22 + *(v22 - 1) * *(v22 - 1));
      v29[v21] = v23;
      if ( !(v26 | v25) )
        *v20 = *v20 / v23;
      ++v21;
      v22 += 3;
      ++v20;
    }
    while ( v21 < 3 );
    *(_DWORD *)(a3 + 684) = 2977;
    if ( glConfig_rescaleNormal )
    {
      v27 = v30 - v29[0];
      if ( (COERCE_FLOAT(LODWORD(v27) & 0x7FFFFFFF) < 0.001)
         | __UNORDERED__(COERCE_FLOAT(LODWORD(v27) & 0x7FFFFFFF), 0.001) )
      {
        v28 = v30 - v29[1];
        if ( (COERCE_FLOAT(LODWORD(v28) & 0x7FFFFFFF) < 0.001)
           | __UNORDERED__(COERCE_FLOAT(LODWORD(v28) & 0x7FFFFFFF), 0.001) )
        {
          *(_DWORD *)(a3 + 684) = 32826;
        }
      }
    }
  }
}
#endif

/* ---- R_RotateForEntity  0x004D3850 ----  VERIFIED */
void __cdecl R_RotateForEntity(float *a1, int *a2, float *a3)
{
  int v4;

  v4 = *a2;
  if ( *a2 == 1 || v4 == 2 || !v4 )
    R_RotateForModelEntity(a1, a3, (int)a2);
  else
    qmemcpy(a1, a3 + 31, 0x7Cu);
}

/* ---- R_RotateForViewer  0x004D3880 ----  [HIGH] */
int R_RotateForViewer()
{
  int result;
  _DWORD v1[16]; // [esp+8h] [ebp-40h] BYREF

  memset(&tr_or, 0, 0x7Cu);
  tr_or_viewOriginZ = LODWORD(tr_viewParms_originZ);
  tr_or_viewOriginY = LODWORD(tr_viewParms_originY);
  v1[8] = tr_viewParms_axis02;
  v1[4] = tr_viewParms_axis01;
  tr_or_viewOriginX = LODWORD(tr_viewParms_originX);
  v1[0] = tr_viewParms_axis00;
  v1[9] = tr_viewParms_axis12;
  v1[5] = tr_viewParms_axis11;
  v1[1] = tr_viewParms_axis10;
  v1[10] = tr_viewParms_axis22;
  v1[6] = tr_viewParms_axis21;
  v1[2] = tr_viewParms_axis20;
  *(float *)&v1[12] = -(*(float *)&tr_viewParms_axis02 * tr_viewParms_originZ
                      + *(float *)&tr_viewParms_axis01 * tr_viewParms_originY
                      + *(float *)&tr_viewParms_axis00 * tr_viewParms_originX);
  tr_or_axis00 = 1065353216;
  tr_or_axis11 = 1065353216;
  tr_or_axis22 = 1065353216;
  v1[3] = 0;
  v1[7] = 0;
  v1[11] = 0;
  v1[15] = 1065353216;
  *(float *)&v1[13] = -(*(float *)&tr_viewParms_axis12 * tr_viewParms_originZ
                      + *(float *)&tr_viewParms_axis11 * tr_viewParms_originY
                      + *(float *)&tr_viewParms_axis10 * tr_viewParms_originX);
  *(float *)&v1[14] = -(*(float *)&tr_viewParms_axis22 * tr_viewParms_originZ
                      + *(float *)&tr_viewParms_axis21 * tr_viewParms_originY
                      + *(float *)&tr_viewParms_axis20 * tr_viewParms_originX);
  result = myGlMultMatrix((int)v1, s_flipMatrix, (int)tr_or_modelMatrix);
  qmemcpy(&tr_viewParms_world, &tr_or, 0x7Cu);
  return result;
}

/* ---- R_SetFrameFog  0x004D39E0 ----  VERIFIED */
void R_SetFrameFog()
{
  bool v0; // zf
  double v1;
  double v2;
  double v3;
  double v4;
  int v5;

  if ( r_speeds->integer == 5 && !dword_16C4CF0 )
  {
    ri_Printf(0, "no fog - calc zFar: %0.1f\n", tr_viewParms_zFar);
    return;
  }
  if ( !dword_16C4CCC || dword_16C4CCC < tr_refdef_time )
  {
    qmemcpy(&dword_16C4C40, &dword_16C4CC0, 0x40u);
    dword_16C4C7C = 0;
    goto LABEL_21;
  }
  if ( dword_16C4C80 == 2048 )
  {
    v0 = dword_16C4CC0 == 9729;
  }
  else
  {
    if ( dword_16C4C80 != 9729 )
    {
LABEL_12:
      v5 = dword_16C4CCC - dword_16C4CC8;
      if ( dword_16C4CCC - dword_16C4CC8 <= 0 )
        v5 = 1;
      v1 = (double)(tr_refdef_time - dword_16C4CC8) / (double)v5;
      if ( v1 > 1.0 )
        v1 = 1.0;
      dword_16C4C40 = dword_16C4CC0;
      dword_16C4C70 = 1;
      flt_16C4C60 = (flt_16C4CE0 - flt_16C4CA0) * v1 + flt_16C4CA0;
      flt_16C4C64 = (flt_16C4CE4 - flt_16C4CA4) * v1 + flt_16C4CA4;
      flt_16C4C6C = (flt_16C4CEC - flt_16C4CAC) * v1 + flt_16C4CAC;
      flt_16C4C50 = (flt_16C4CD0 - flt_16C4C90) * v1 + flt_16C4C90;
      flt_16C4C54 = (flt_16C4CD4 - flt_16C4C94) * v1 + flt_16C4C94;
      flt_16C4C58 = (flt_16C4CD8 - flt_16C4C98) * v1 + flt_16C4C98;
      if ( dword_16C4CF8 || (dword_16C4C78 = 0, dword_16C4CB8) )
        dword_16C4C78 = 1;
      dword_16C4C7C = 1;
      goto LABEL_21;
    }
    v0 = dword_16C4CC0 == 2048;
  }
  if ( !v0 )
    goto LABEL_12;
  qmemcpy(&dword_16C4C40, &dword_16C4CC0, 0x40u);
  dword_16C4CCC = 0;
  dword_16C4C7C = 1;
LABEL_21:
  if ( dword_16C4C40 == 9729 )
  {
    if ( (flt_16C4C64 < (double)tr_viewParms_zFar) | __UNORDERED__(flt_16C4C64, tr_viewParms_zFar) )
      tr_viewParms_zFar = flt_16C4C64;
  }
  if ( r_speeds->integer == 5 )
  {
    v4 = flt_16C4C64;
    v3 = tr_viewParms_zFar;
    v2 = flt_16C4C6C;
    if ( dword_16C4C40 == 9729 )
      ri_Printf(0, "farclip fog - den: %0.1f  calc zFar: %0.1f  fog zfar: %0.1f\n", v2, v3, v4);
    else
      ri_Printf(0, "density fog - den: %0.6f  calc zFar: %0.1f  fog zFar: %0.1f\n", v2, v3, v4);
  }
}


/* ---- SetFarClip  0x004D3C40 ----  VERIFIED */
void SetFarClip()
{
  if ( (tr_refdef_rdflags & 1) != 0 )
  {
    tr_viewParms_zFar = 2048.0;
  }
  else if ( (r_zfar->value == 0.0) | __UNORDERED__(r_zfar->value, 0.0) )
  {
    tr_viewParms_zFar = 524288.0;
    R_SetFrameFog();
  }
  else
  {
    tr_viewParms_zFar = r_zfar->value;
    R_SetFrameFog();
    if ( r_speeds->integer == 5 )
      ri_Printf(0, "r_zfar value forcing farclip at: %f\n", tr_viewParms_zFar);
  }
}


/* ---- R_SetupProjection  0x004D3CB0 ----  VERIFIED */
cvar_t *R_SetupProjection()
{
  cvar_t *result;
  double value;
  long double v2;
  long double v3;
  long double v4;
  double v5;
  double v6;
  float v7;

  SetFarClip();
  result = r_znear;
  value = r_znear->value;
  tr_viewParms_projectionMatrix[4] = 0.0;
  tr_viewParms_projectionMatrix[12] = 0.0;
  tr_viewParms_projectionMatrix[1] = 0.0;
  tr_viewParms_projectionMatrix[13] = 0.0;
  v2 = tan(tr_refdef_fov_y * 0.0087266462);
  tr_viewParms_projectionMatrix[2] = 0.0;
  tr_viewParms_projectionMatrix[6] = 0.0;
  tr_viewParms_projectionMatrix[10] = -1.0;
  tr_viewParms_projectionMatrix[3] = 0.0;
  tr_viewParms_projectionMatrix[7] = 0.0;
  tr_viewParms_projectionMatrix[11] = -1.0;
  tr_viewParms_projectionMatrix[15] = 0.0;
  v3 = tan(tr_refdef_fov_x * 0.0087266462);
  v7 = v2 - -v2;
  v4 = 1.0 / (v3 - -v3);
  tr_viewParms_projectionMatrix[0] = v4 + v4;
  tr_viewParms_projectionMatrix[8] = (v3 - v3) * v4;
  v5 = 1.0 / v7;
  tr_viewParms_projectionMatrix[5] = v5 + v5;
  tr_viewParms_projectionMatrix[9] = (v2 - v2) * v5;
  tr_viewParms_projectionMatrix[14] = value * -2.0;
  v6 = r_znear_depthhack->value * -2.0;
  qmemcpy(tr_viewParms_depthHackProjectionMatrix, tr_viewParms_projectionMatrix, 0x40u);
  tr_viewParms_depthHackProjectionMatrix[14] = v6;
  return result;
}


/* ---- R_SetupFrustum  0x004D3DE0 ----  VERIFIED */
void R_SetupFrustum()
{
  float ang, s, c, t;
  int   i;

  ang = (float)(tr_viewParms_fovX / 180.0 * 1.5707964);
  c = (float)cos(ang);
  s = (float)sin(ang);

  t = (float)(*(float *)&tr_viewParms_axis00 * s);
  tr_frustum[0].normal[0] = t;
  tr_frustum[1].normal[0] = t;
  t = (float)(*(float *)&tr_viewParms_axis01 * s);
  tr_frustum[0].normal[1] = t;
  tr_frustum[1].normal[1] = t;
  t = (float)(*(float *)&tr_viewParms_axis02 * s);
  tr_frustum[0].normal[2] = t;          /* 0x4D3E79, overwritten five lines down */
  tr_frustum[1].normal[2] = t;

  tr_frustum[0].normal[0] = (float)(*(float *)&tr_viewParms_axis10 * c + tr_frustum[0].normal[0]);
  tr_frustum[0].normal[1] = (float)(*(float *)&tr_viewParms_axis11 * c + tr_frustum[0].normal[1]);
  tr_frustum[0].normal[2] = (float)(*(float *)&tr_viewParms_axis12 * c + t);

  t = -c;
  tr_frustum[1].normal[0] = (float)(t * *(float *)&tr_viewParms_axis10 + tr_frustum[1].normal[0]);
  tr_frustum[1].normal[1] = (float)(t * *(float *)&tr_viewParms_axis11 + tr_frustum[1].normal[1]);
  tr_frustum[1].normal[2] = (float)(t * *(float *)&tr_viewParms_axis12 + tr_frustum[1].normal[2]);

  ang = (float)(tr_viewParms_fovY / 180.0 * 1.5707964);
  c = (float)cos(ang);
  s = (float)sin(ang);

  t = (float)(*(float *)&tr_viewParms_axis00 * s);
  tr_frustum[2].normal[0] = t;
  tr_frustum[3].normal[0] = t;          /* retail keeps this in EAX (0x4D3F9E) */
  t = (float)(*(float *)&tr_viewParms_axis01 * s);
  tr_frustum[2].normal[1] = t;
  tr_frustum[3].normal[1] = t;          /* ECX (0x4D3FA9) */
  t = (float)(*(float *)&tr_viewParms_axis02 * s);
  tr_frustum[2].normal[2] = t;
  tr_frustum[3].normal[2] = t;          /* EDX (0x4D3FB3) */

  tr_frustum[2].normal[0] = (float)(*(float *)&tr_viewParms_axis20 * c + tr_frustum[2].normal[0]);
  tr_frustum[2].normal[1] = (float)(*(float *)&tr_viewParms_axis21 * c + tr_frustum[2].normal[1]);
  tr_frustum[2].normal[2] = (float)(*(float *)&tr_viewParms_axis22 * c + t);

  t = -c;
  tr_frustum[3].normal[0] = (float)(t * *(float *)&tr_viewParms_axis20 + tr_frustum[3].normal[0]);
  tr_frustum[3].normal[1] = (float)(t * *(float *)&tr_viewParms_axis21 + tr_frustum[3].normal[1]);
  tr_frustum[3].normal[2] = (float)(t * *(float *)&tr_viewParms_axis22 + tr_frustum[3].normal[2]);

  for ( i = 0; i < TR_FRUSTUM_PLANES; ++i )
  {
    tr_cplane_t  *p = &tr_frustum[i];
    unsigned char signbits;

    p->type = 3;
    p->dist = (float)( tr_viewParms_originZ * p->normal[2]
                     + tr_viewParms_originY * p->normal[1]
                     + tr_viewParms_originX * p->normal[0] );

    signbits = 0;
    if ( p->normal[0] < 0.0f ) signbits  = 1;
    if ( p->normal[1] < 0.0f ) signbits |= 2;
    if ( p->normal[2] < 0.0f ) signbits |= 4;
    p->signbits = signbits;
  }
}

/* ---- R_MirrorPoint  0x004D40A0 ----  [HIGH] */
float *__cdecl R_MirrorPoint(float *result, float *a2, float *a3, float *a4)
{
  double v4;
  double v5;
  double v6;
  double v7;
  double v8;
  double v9;
  float v10;
  float v11;
  float v12;
  float v13;
  float v14;
  float v15;
  float v16;
  float v17;

  v4 = *a4 - *a3;
  v5 = a4[1] - a3[1];
  v6 = a4[2] - a3[2];
  v7 = v6 * a3[5] + v5 * a3[4] + v4 * a3[3];
  v12 = v7 * result[4];
  v15 = v7 * result[5];
  v8 = v6 * a3[8] + v5 * a3[7] + v4 * a3[6];
  v10 = v8;
  v11 = v8 * result[6] + v7 * result[3];
  v13 = v10 * result[7] + v12;
  v16 = v10 * result[8] + v15;
  v9 = v6 * a3[11] + v5 * a3[10] + v4 * a3[9];
  v14 = v9 * result[10] + v13;
  v17 = v9 * result[11] + v16;
  *a2 = v9 * result[9] + v11 + *result;
  a2[1] = v14 + result[1];
  a2[2] = v17 + result[2];
  return result;
}

/* ---- R_MirrorVector  0x004D4170 ----  VERIFIED */
float *__cdecl R_MirrorVector(float *result, float *a2, float *a3, float *a4)
{
  double v4;
  double v5;
  double v6;

  result[2] = 0.0;
  result[1] = 0.0;
  *result = 0.0;
  v4 = a2[5] * a4[2] + a2[4] * a4[1] + a2[3] * *a4;
  *result = v4 * a3[3];
  result[1] = v4 * a3[4];
  result[2] = v4 * a3[5];
  v5 = a2[8] * a4[2] + a2[7] * a4[1] + a2[6] * *a4;
  *result = v5 * a3[6] + *result;
  result[1] = v5 * a3[7] + result[1];
  result[2] = v5 * a3[8] + result[2];
  v6 = a2[11] * a4[2] + a2[10] * a4[1] + a2[9] * *a4;
  *result = v6 * a3[9] + *result;
  result[1] = v6 * a3[10] + result[1];
  result[2] = v6 * a3[11] + result[2];
  return result;
}

/* ---- R_PlaneForSurface  0x004D4220 ----  VERIFIED */
int __cdecl R_PlaneForSurface(int a1, _DWORD *a2)
{
  int result;
  float plane[4]; // [esp+4h] [ebp-10h] BYREF -- ONE vec4_t in retail

  if ( a1 )
  {
    result = *(_DWORD *)a1;
    if ( *(int *)a1 < 24 )
    {
      if ( result == 2 )
      {
        PlaneFromPoints(
          plane,
          *(float **)(a1 + 12),
          (float *)(*(_DWORD *)(a1 + 12) + 32),
          (float *)(*(_DWORD *)(a1 + 12) + 64));
        ((float *)a2)[0] = plane[0];
        ((float *)a2)[1] = plane[1];
        ((float *)a2)[2] = plane[2];
        ((float *)a2)[3] = plane[3];
      }
      else
      {
        *a2 = 0;
        a2[1] = 0;
        a2[2] = 0;
        a2[3] = 0;
        a2[4] = 0;
        *a2 = 1065353216;
      }
    }
    else
    {
      PlaneFromPoints(
        plane,
        *(float **)(a1 + 64),
        (float *)(*(_DWORD *)(a1 + 64) + 12 * (*(unsigned __int16 *)(a1 + 74) - *(unsigned __int16 *)(a1 + 72))),
        (float *)(*(_DWORD *)(a1 + 64) + 12 * (*(unsigned __int16 *)(a1 + 76) - *(unsigned __int16 *)(a1 + 72))));
      ((float *)a2)[0] = plane[0];
      ((float *)a2)[1] = plane[1];
      ((float *)a2)[2] = plane[2];
      ((float *)a2)[3] = plane[3];
      result = *(int *)&plane[1];
    }
  }
  else
  {
    result = 0;
    *a2 = 0;
    a2[1] = 0;
    a2[2] = 0;
    a2[3] = 0;
    a2[4] = 0;
    *a2 = 1065353216;
  }
  return result;
}

/* ---- R_GetPortalOrientations  0x004D42F0 ----  VERIFIED */
int __cdecl R_GetPortalOrientations(int entityNum, int a1, int a2, int a3, _DWORD *a4, _DWORD *a5)
{
  int *v7;
  int v8;
  int v9;
  int v10;
  float *v11;
  int v12;
  int i;
  double v15;
  double v16;
  int *v17;
  float *v18;
  double v19;
  int v20;
  int v21;
  double v22;
  double v23;
  int v24;
  double v25;
  double v26;
  float v27;
  int v28;
  int v29;
  int v30;
  int v31[5]; // [esp+14h] [ebp-2Ch] BYREF
  float v32[6]; // [esp+28h] [ebp-18h] BYREF  (v32[0..3] = plane, v32[5] = cookie)
  float v34;
  float v35;
  float v36;
  float v37;
  float v38;
  float v39;
  float v40;
  float v41;
  float v42;
  float v43;
  int v44;

  R_PlaneForSurface(*(_DWORD *)(a1 + 4), (_DWORD *)v32);
  if ( entityNum == 1022 )
  {
    qmemcpy(v31, v32, sizeof(v31));
  }
  else
  {
    tr_currentEntityNumber = entityNum;
    v7 = (int *)(tr_refdef_entities + 696 * entityNum);
    tr_currentEntity = (int)v7;
    v8 = *v7;
    if ( *v7 == 1 || v8 == 2 || !v8 )
      R_RotateForModelEntity(&tr_or, &tr_viewParms_originX, (int)v7);
    else
      qmemcpy(&tr_or, &tr_viewParms_world, 0x7Cu);
    R_LocalNormalToWorld(v32, (float *)v31);
    *(float *)&v31[3] = tr_or_originZ * *(float *)&v31[2]
                      + tr_or_originY * *(float *)&v31[1]
                      + tr_or * *(float *)v31
                      + v32[3];
    v32[3] = tr_or_originZ * v32[2] + tr_or_originY * v32[1] + tr_or * v32[0] + v32[3];
  }
  v9 = v31[0];
  v10 = v31[2];
  v11 = (float *)(a2 + 12);
  *(_DWORD *)(a2 + 16) = v31[1];
  *(_DWORD *)(a2 + 12) = v9;
  *(_DWORD *)(a2 + 20) = v10;
  PerpendicularVector((float *)(a2 + 24), (float *)(a2 + 12));
  v12 = 0;
  *(float *)(a2 + 36) = *(float *)(a2 + 16) * *(float *)(a2 + 32) - *(float *)(a2 + 28) * *(float *)(a2 + 20);
  *(float *)(a2 + 40) = *(float *)(a2 + 20) * *(float *)(a2 + 24) - *(float *)(a2 + 12) * *(float *)(a2 + 32);
  *(float *)(a2 + 44) = *(float *)(a2 + 28) * *(float *)(a2 + 12) - *(float *)(a2 + 16) * *(float *)(a2 + 24);
  if ( tr_refdef_num_entities <= 0 )
    return 0;
  for ( i = tr_refdef_entities + 72; ; i += 696 )
  {
    if ( *(_DWORD *)(i - 72) == 11 )
    {
      v34 = v32[2] * *(float *)(i + 4) + v32[0] * *(float *)(i - 4) + v32[1] * *(float *)i - v32[3];
      if ( v34 <= 64.0 && !((v34 < -64.0) | __UNORDERED__(v34, -64.0)) )
        break;
    }
    if ( ++v12 >= tr_refdef_num_entities )
      return 0;
  }
  *a4 = *(_DWORD *)(i + 12);
  a4[1] = *(_DWORD *)(i + 16);
  a4[2] = *(_DWORD *)(i + 20);
  if ( (*(float *)(i + 12) == *(float *)(i - 4)) | __UNORDERED__(*(float *)(i + 12), *(float *)(i - 4))
    && (*(float *)(i + 16) == *(float *)i) | __UNORDERED__(*(float *)(i + 16), *(float *)i)
    && (*(float *)(i + 20) == *(float *)(i + 4)) | __UNORDERED__(*(float *)(i + 20), *(float *)(i + 4)) )
  {
    v35 = *(float *)&v31[3] * *(float *)v31;
    v15 = *(float *)&v31[3] * *(float *)&v31[1];
    *(float *)a2 = v35;
    *(float *)(a2 + 4) = v15;
    *(float *)(a2 + 8) = *(float *)&v31[3] * *(float *)&v31[2];
    *(float *)a3 = v35;
    *(_DWORD *)(a3 + 4) = *(_DWORD *)(a2 + 4);
    *(_DWORD *)(a3 + 8) = *(_DWORD *)(a2 + 8);
    *(float *)(a3 + 12) = 0.0 - *v11;
    *(float *)(a3 + 16) = 0.0 - *(float *)(a2 + 16);
    *(float *)(a3 + 20) = 0.0 - *(float *)(a2 + 20);
    *(_DWORD *)(a3 + 24) = *(_DWORD *)(a2 + 24);
    *(_DWORD *)(a3 + 28) = *(_DWORD *)(a2 + 28);
    *(_DWORD *)(a3 + 32) = *(_DWORD *)(a2 + 32);
    *(_DWORD *)(a3 + 36) = *(_DWORD *)(a2 + 36);
    *(_DWORD *)(a3 + 40) = *(_DWORD *)(a2 + 40);
    *(_DWORD *)(a3 + 44) = *(_DWORD *)(a2 + 44);
    *a5 = 1;
    return 1;
  }
  v36 = *(float *)&v31[2] * *(float *)(i + 4)
      + *(float *)&v31[1] * *(float *)i
      + *(float *)v31 * *(float *)(i - 4)
      - *(float *)&v31[3];
  v37 = -v36;
  v16 = v37 * *v11;
  v17 = (int *)(a3 + 24);
  *(float *)a2 = v16 + *(float *)(i - 4);
  *(float *)(a2 + 4) = v37 * *(float *)(a2 + 16) + *(float *)i;
  *(float *)(a2 + 8) = v37 * *(float *)(a2 + 20) + *(float *)(i + 4);
  *(_DWORD *)a3 = *(_DWORD *)(i + 12);
  *(_DWORD *)(a3 + 4) = *(_DWORD *)(i + 16);
  *(_DWORD *)(a3 + 8) = *(_DWORD *)(i + 20);
  *(_DWORD *)(a3 + 12) = *(_DWORD *)(i - 44);
  *(_DWORD *)(a3 + 16) = *(_DWORD *)(i - 40);
  *(_DWORD *)(a3 + 20) = *(_DWORD *)(i - 36);
  *(_DWORD *)(a3 + 24) = *(_DWORD *)(i - 32);
  v18 = (float *)(a3 + 12);
  *(_DWORD *)(a3 + 28) = *(_DWORD *)(i - 28);
  *(_DWORD *)(a3 + 32) = *(_DWORD *)(i - 24);
  *(_DWORD *)(a3 + 36) = *(_DWORD *)(i - 20);
  *(_DWORD *)(a3 + 40) = *(_DWORD *)(i - 16);
  *(_DWORD *)(a3 + 44) = *(_DWORD *)(i - 12);
  *(float *)(a3 + 12) = 0.0 - *(float *)(a3 + 12);
  *(float *)(a3 + 16) = 0.0 - *(float *)(a3 + 16);
  *(float *)(a3 + 20) = 0.0 - *(float *)(a3 + 20);
  *(float *)(a3 + 24) = 0.0 - *(float *)(a3 + 24);
  *(float *)(a3 + 28) = 0.0 - *(float *)(a3 + 28);
  *(float *)&v44 = 0.0 - *(float *)(a3 + 32);
  *(float *)(a3 + 32) = *(float *)&v44;
  if ( *(_DWORD *)(i + 24) )
  {
    v19 = (double)tr_refdef_time;
    v30 = *(_DWORD *)(i + 8);
    if ( !v30 )
    {
      v41 = v19;
      v42 = sin(v41 * 0.003);
      v24 = *(_DWORD *)(a3 + 28);
      v31[0] = *v17;
      v25 = v42 * 4.0 + *(float *)(i + 56);
      v31[1] = v24;
      v31[2] = v44;
      v43 = v25;
      RotatePointAroundVector((float *)(a3 + 24), (float *)(a3 + 12), (float *)v31, v43);
      *(float *)(a3 + 36) = *(float *)(a3 + 32) * *(float *)(a3 + 16) - *(float *)(a3 + 20) * *(float *)(a3 + 28);
      *(float *)(a3 + 40) = *(float *)(a3 + 20) * *(float *)(a3 + 24) - *(float *)(a3 + 12) * *(float *)(a3 + 32);
      v23 = *v18 * *(float *)(a3 + 28);
      v26 = *(float *)v17 * *(float *)(a3 + 16);
LABEL_27:
      *(float *)(a3 + 44) = v23 - v26;
      goto LABEL_28;
    }
    v38 = v19;
    v20 = *v17;
    v21 = *(_DWORD *)(a3 + 28);
    v22 = v38 / 1000.0;
    v31[2] = v44;
    v31[0] = v20;
    v31[1] = v21;
    v39 = (float)v30;
    v40 = v22 * v39;
    RotatePointAroundVector((float *)(a3 + 24), (float *)(a3 + 12), (float *)v31, v40);
    *(float *)(a3 + 36) = *(float *)(a3 + 32) * *(float *)(a3 + 16) - *(float *)(a3 + 28) * *(float *)(a3 + 20);
    *(float *)(a3 + 40) = *(float *)v17 * *(float *)(a3 + 20) - *(float *)(a3 + 32) * *v18;
    v23 = *v18 * *(float *)(a3 + 28);
LABEL_26:
    v26 = *(float *)(a3 + 16) * *(float *)v17;
    goto LABEL_27;
  }
  if ( !((*(float *)(i + 56) == 0.0) | __UNORDERED__(*(float *)(i + 56), 0.0)) )
  {
    v27 = *(float *)(i + 56);
    v28 = *(_DWORD *)(a3 + 28);
    v29 = *v17;
    v31[2] = v44;
    v31[1] = v28;
    v31[0] = v29;
    RotatePointAroundVector((float *)(a3 + 24), (float *)(a3 + 12), (float *)v31, v27);
    *(float *)(a3 + 36) = *(float *)(a3 + 16) * *(float *)(a3 + 32) - *(float *)(a3 + 28) * *(float *)(a3 + 20);
    *(float *)(a3 + 40) = *(float *)(a3 + 20) * *(float *)v17 - *v18 * *(float *)(a3 + 32);
    v23 = *(float *)(a3 + 28) * *v18;
    goto LABEL_26;
  }
LABEL_28:
  *a5 = 0;
  return 1;
}

/* ---- IsMirror  0x004D4820 ----  VERIFIED */
BOOL __cdecl IsMirror(int entityNum, int a1)
{
  int *v3;
  int v4;
  int v5;
  float *i;
  double v7;
  float plane[4]; // [esp+8h] [ebp-18h] BYREF -- ONE vec4_t in retail

  R_PlaneForSurface(*(_DWORD *)(a1 + 4), (_DWORD *)plane);
  if ( entityNum != 1022 )
  {
    tr_currentEntityNumber = entityNum;
    v3 = (int *)(tr_refdef_entities + 696 * entityNum);
    tr_currentEntity = (int)v3;
    v4 = *v3;
    if ( *v3 == 1 || v4 == 2 || !v4 )
      R_RotateForModelEntity(&tr_or, &tr_viewParms_originX, (int)v3);
    else
      qmemcpy(&tr_or, &tr_viewParms_world, 0x7Cu);
    plane[3] = tr_or_originZ * plane[2] + tr_or_originY * plane[1] + tr_or * plane[0] + plane[3];
  }
  v5 = 0;
  if ( tr_refdef_num_entities <= 0 )
    return 0;
  for ( i = (float *)(tr_refdef_entities + 72); ; i += 174 )
  {
    if ( *((_DWORD *)i - 18) == 11 )
    {
      v7 = plane[2] * i[1] + plane[0] * *(i - 1) + plane[1] * *i - plane[3];
      if ( v7 <= 64.0 && !((v7 < -64.0) | __UNORDERED__(v7, -64.0)) )
        break;
    }
    if ( ++v5 >= tr_refdef_num_entities )
      return 0;
  }
  return (i[3] == *(i - 1)) | __UNORDERED__(i[3], *(i - 1))
      && (i[4] == *i) | __UNORDERED__(i[4], *i)
      && (i[5] == i[1]) | __UNORDERED__(i[5], i[1]);
}

/* ---- SurfIsOffscreen  0x004D4980 ----  VERIFIED */
int __cdecl SurfIsOffscreen( int drawSurf, float (*clipDest)[4] )
{
  float shortest;
  unsigned int pointAnd;
  unsigned int pointFlags;
  int    sort;
  int    entityNum;
  int    shader;
  int    storageClass;
  int    numTriangles;
  int    numVertexes;
  int    vcc;
  int    i, j;
  int    index;
  float  normal[3];
  float  len;
  float *xyz;
  float  clip[4];
  float  eye[4];

  shortest = 100000000.0f;                     /* 0x4CBEBC20 @0x004D498A */
  pointAnd = (unsigned int)~0;

  R_RotateForViewer();

  /* R_DecomposeSort, inlined (0x004D499F-0x004D49C3). */
  sort         = *(int *)drawSurf;
  entityNum    = ( sort >> 8 ) & 0x3FF;
  shader       = dword_16CF84C[ ( (unsigned int)sort >> 18 ) & 0xFFF ];
  storageClass = (unsigned int)sort >> 30;
  if ( ( sort & 2 ) != 0 )
    entityNum = 1022;

  if ( entityNum == 1022 )
    backEnd_currentEntity = (int)&tr_worldEntity;
  else
    backEnd_currentEntity = backEnd_refdef_entities + 696 * entityNum;

  if ( storageClass != glState_currentStorageMode )         /* 0x004D49DD, CoD addition */
  {
    if ( glConfig_NVVertexArrayRange )
      RB_SelectStorageNV( storageClass );
    else if ( glConfig_ATIVertexArrayObject )
      RB_SelectStorageATI( storageClass );
    glState_currentStorageMode = storageClass;
  }

  RB_BeginSurface( (void *)shader, 3 );
  rb_surfaceTable[ **(_DWORD **)( drawSurf + 4 ) ]( (void *)*(_DWORD *)( drawSurf + 4 ) );

  numVertexes = tess_numVertexes;
  xyz = tess_xyz;
  if ( numVertexes <= 0 )
    return 1;

  for ( i = 0; i < numVertexes; i++ )
  {
    pointFlags = 0;

    R_TransformModelToClip( eye, tr_or_modelMatrix, tr_viewParms_projectionMatrix, clip, xyz );

    for ( j = 0; j < 3; j++ )
    {
      if ( clip[j] >= clip[3] )
        pointFlags |= ( 1 << ( j * 2 ) );
      else if ( clip[j] <= -clip[3] )
        pointFlags |= ( 1 << ( j * 2 + 1 ) );
    }
    pointAnd &= pointFlags;

    xyz += tess_vertexComponentCount;          /* 0x004D4AF5, reloaded */
  }

  if ( pointAnd )
    return 1;

  vcc          = tess_vertexComponentCount;
  numTriangles = tess_numIndexes / 3;

  for ( i = 0; i < tess_numIndexes; i += 3 )
  {
    index = tess_indexes[i];

    normal[0] = tess_xyz[ vcc * index     ] - tr_viewParms_originX;
    normal[1] = tess_xyz[ vcc * index + 1 ] - tr_viewParms_originY;
    normal[2] = tess_xyz[ vcc * index + 2 ] - tr_viewParms_originZ;

    len = normal[2] * normal[2] + normal[1] * normal[1] + normal[0] * normal[0];
    if ( len < shortest )
      shortest = len;

    if ( normal[2] * tess_stageNormals[ 3 * index + 2 ]
       + normal[1] * tess_stageNormals[ 3 * index + 1 ]
       + normal[0] * tess_stageNormals[ 3 * index     ] >= 0.0 )
      numTriangles--;
  }

  if ( !numTriangles )
    return 1;

  if ( IsMirror( entityNum, drawSurf ) )
    return 0;

  if ( shortest > *(float *)( tess_shader + 164 ) * *(float *)( tess_shader + 164 ) )
    return 1;

  return 0;
}

/* ---- R_MirrorViewBySurface  0x004D4C30 ----  VERIFIED */
int __cdecl R_MirrorViewBySurface( int drawSurf, int entityNum )
{
  float surface[12];
  float camera[12];
  float newParms[152];
  float oldParms[152];
  float clipDest[128][4];

  if ( tr_viewParms_isPortal )
  {
    ri_Printf( 1, "WARNING: recursive mirror/portal found\n" );
    return 0;
  }

  if ( r_noportals->integer )
    return 0;
  if ( r_fastsky->integer )
    return 0;

  if ( SurfIsOffscreen( drawSurf, clipDest ) )
    return 0;

  qmemcpy( oldParms, &tr_viewParms_originX, 0x260u );       /* 0x004D4CAB */
  qmemcpy( newParms, &tr_viewParms_originX, 0x260u );       /* 0x004D4CBE */
  *(int *)&newParms[65] = 1;

  if ( !R_GetPortalOrientations( entityNum, drawSurf, (int)surface, (int)camera,
                                 (_DWORD *)&newParms[62], (_DWORD *)&newParms[66] ) )
    return 0;

  R_MirrorPoint( camera, newParms, surface, oldParms );

  newParms[69] = 0.0 - camera[3];
  newParms[70] = 0.0 - camera[4];
  newParms[71] = 0.0 - camera[5];
  newParms[72] = camera[2] * newParms[71]
               + camera[1] * newParms[70]
               + camera[0] * newParms[69];

  R_SetPlaneSidesDPVS( &newParms[69] );

  R_MirrorVector( &newParms[3], surface, camera, &oldParms[3] );
  R_MirrorVector( &newParms[6], surface, camera, &oldParms[6] );
  R_MirrorVector( &newParms[9], surface, camera, &oldParms[9] );

  R_RenderView( (_DWORD *)newParms );

  qmemcpy( &tr_viewParms_originX, oldParms, 0x260u );
  return 1;
}

/* ---- shortsort  0x004D4E20 ----  VERIFIED */
int *__cdecl shortsort(int *result, int *a2)
{
  int *v2;
  int *i;
  int v4;
  int v5;
  int v6;

  for ( ; result > a2; result -= 2 )
  {
    v2 = a2 + 2;
    for ( i = a2; v2 <= result; v2 += 2 )
    {
      if ( *v2 > (unsigned int)*i )
        i = v2;
    }
    v4 = *i;
    *i = *result;
    v5 = result[1];
    *result = v4;
    v6 = i[1];
    i[1] = v5;
    result[1] = v6;
  }
  return result;
}

/* ---- qsortFast  0x004D4E70 ----  [HIGH] */
int *__cdecl qsortFast(int *result, int *a2, unsigned int a3)
{
  int *v3;
  int v4;
  int *v5;
  unsigned int v6;
  unsigned int v7;
  int v8;
  int *v9;
  int v10;
  int v11;
  int *v12;
  unsigned int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  _DWORD v21[60];

  v3 = a2;
  if ( (unsigned int)result >= 2 && a3 )
  {
    v4 = 0;
    v20 = 0;
    v5 = (int *)((char *)a2 + a3 * ((_DWORD)result - 1));
    while ( 1 )
    {
      v6 = ((char *)v5 - (char *)v3) / a3 + 1;
      if ( v6 <= 8 )
      {
        result = shortsort(v5, v3);
        goto LABEL_6;
      }
      v7 = a3 * (v6 >> 1);
      v8 = *(int *)((char *)v3 + v7);
      v9 = (int *)((char *)v3 + v7);
      *v9 = *v3;
      v10 = v3[1];
      *v3 = v8;
      v11 = v9[1];
      v9[1] = v10;
      v3[1] = v11;
      v12 = v3;
      for ( result = (int *)((char *)v5 + a3); ; result[1] = v16 )
      {
        do
          v12 = (int *)((char *)v12 + a3);
        while ( v12 <= v5 && *v12 <= (unsigned int)*v3 );
        do
          result = (int *)((char *)result - a3);
        while ( result > v3 && *result >= (unsigned int)*v3 );
        v13 = *result;
        if ( result < v12 )
          break;
        v14 = *v12;
        *v12 = v13;
        v15 = result[1];
        *result = v14;
        v16 = v12[1];
        v12[1] = v15;
      }
      v17 = *v3;
      *v3 = v13;
      v18 = result[1];
      *result = v17;
      v19 = v3[1];
      v3[1] = v18;
      result[1] = v19;
      v4 = v20;
      if ( (char *)result - (char *)v3 - 1 < (char *)v5 - (char *)v12 )
      {
        if ( v12 < v5 )
        {
          v21[v20] = v12;
          v21[v20 + 30] = v5;
          v4 = ++v20;
        }
        if ( (int *)((char *)v3 + a3) >= result )
        {
LABEL_6:
          v20 = --v4;
          if ( v4 < 0 )
            return result;
          v3 = (int *)v21[v4];
          v5 = (int *)v21[v4 + 30];
        }
        else
        {
          v5 = (int *)((char *)result - a3);
        }
      }
      else
      {
        if ( (int *)((char *)v3 + a3) < result )
        {
          result = (int *)((char *)result - a3);
          v21[v20] = v3;
          v21[v20 + 30] = result;
          v4 = ++v20;
        }
        if ( v12 >= v5 )
          goto LABEL_6;
        v3 = v12;
      }
    }
  }
  return result;
}

/* ---- R_AddDrawSurf  0x004D4FB0 ----  VERIFIED */
int __cdecl R_AddDrawSurf(int result, int a2, int a3, int a4, int a5, int a6)
{
  if ( tr_refdef_numDrawSurfs < 0x10000 )
  {
    *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs) = a4
                                                   | tr_shiftedEntityNumber
                                                   | (2
                                                    * (a6 | (2 * (a5 | (((result << 12) | *(_DWORD *)(a2 + 72)) << 16)))));
    result = a3;
    *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs++ + 4) = a3;
  }
  return result;
}

/* ---- R_DecomposeSort  0x004D5010 ----  VERIFIED */
unsigned int __cdecl R_DecomposeSort(
        unsigned int result,
        unsigned int *a2,
        unsigned int *a3,
        _DWORD *a4,
        unsigned int *a5,
        unsigned int *a6)
{
  *a3 = result >> 30;
  *a2 = (result >> 8) & 0x3FF;
  *a4 = dword_16CF84C[(result >> 18) & 0xFFF];
  *a5 = result & 1;
  *a6 = (result >> 2) & 1;
  if ( (result & 2) != 0 )
    *a2 = 1022;
  return result;
}

/* ---- R_SortDrawSurfs  0x004D5070 ----  [HIGH] */
_DWORD *__cdecl R_SortDrawSurfs(int *a1, int a2)
{
  int v2;
  _DWORD *result;
  int *v4;
  int *v5;
  int v6;
  int v7;
  int entityNum;
  int v8;

  v2 = a2;
  if ( a2 < 1 )
    return R_AddDrawSurfCmd(a2, (int)a1);
  if ( a2 > 0x10000 )
    v2 = 0x10000;
  v4 = a1;
  qsortFast((int *)v2, a1, 8u);
  v8 = 0;
  if ( v2 > 0 )
  {
    v5 = a1;
    do
    {
      v6 = ((unsigned int)*v5 >> 18) & 0xFFF;
      v7 = dword_16CF84C[v6];
      entityNum = ((unsigned int)*v5 >> 8) & 0x3FF;
      if ( (*v5 & 2) != 0 )
        entityNum = 1022;
      if ( *(float *)(v7 + 88) > 1.0 )
        break;
      if ( (*(float *)(v7 + 88) == 0.0) | __UNORDERED__(*(float *)(v7 + 88), 0.0) )
        ri_Error(1, "\x15" "Shader '%s'with sort == SS_BAD", dword_16CF84C[v6]);
      if ( *(_DWORD *)(v7 + 88) == 1065353216 )
      {
        if ( R_MirrorViewBySurface((int)v5, entityNum) )
        {
          result = (_DWORD *)r_portalOnly->integer;
          if ( result )
            return result;
        }
      }
      v5 += 2;
      ++v8;
    }
    while ( v8 < v2 );
    v4 = a1;
  }
  return R_AddDrawSurfCmd(v2, (int)v4);
}

/* ---- R_AddEntitySurfaces  0x004D5170 ----  [HIGH] */
cvar_t *R_AddEntitySurfaces()
{
  cvar_t *result;
  int v1;
  int v2;
  int v3;
  char *v4;

  result = r_drawentities;
  if ( r_drawentities->integer )
  {
    result = 0;
    for ( tr_currentEntityNumber = 0; tr_currentEntityNumber < tr_refdef_num_entities; ++tr_currentEntityNumber )
    {
      v1 = tr_refdef_entities + 696 * (_DWORD)result;
      tr_currentEntity = v1;
      *(_DWORD *)(v1 + 156) = 0;
      tr_shiftedEntityNumber = tr_currentEntityNumber << 8;
      v2 = *(_DWORD *)(v1 + 4);
      if ( (v2 & 4) == 0 || !tr_viewParms_isPortal )
      {
        switch ( *(_DWORD *)v1 )
        {
          case 0:
            R_AddBrushModelSurfaces((cvar_t *)v1);
            break;
          case 1:
            R_AddXModelSurfaces(v1);
            break;
          case 2:
            R_AddStaticModelSurfaces(v1);
            break;
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
          case 0xA:
          case 0xC:
          case 0xD:
          case 0xE:
          case 0xF:
            if ( (v2 & 2) == 0 || tr_viewParms_isPortal )
            {
              v3 = *(_DWORD *)(v1 + 104);
              if ( v3 >= 0 && v3 < tr_numShaders[0] )
              {
                v4 = (char *)tr_shaders[v3];
              }
              else
              {
                ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", v3);
                v4 = tr_defaultShader;
              }
              R_AddDrawSurf(storageClass, (int)v4, (int)&entitySurface, 0, 0, 0);
            }
            break;
          case 0xB:
            break;
          default:
            ri_Error(1, "\x15" "R_AddEntitySurfaces: Bad reType");
            break;
        }
      }
      result = (cvar_t *)(tr_currentEntityNumber + 1);
    }
  }
  return result;
}

void __cdecl R_GenerateDrawSurfs( void );

/* ---- sub_4D52A8  0x004D52A8 ----  VERIFIED */
cvar_t *__cdecl sub_4D52A8(char a1, _BYTE *a2)
{
  *a2 += a1;
  R_GenerateDrawSurfs();
  return 0;
}

/* ---- R_GenerateDrawSurfs  0x004D52C0 ----  VERIFIED */
void __cdecl R_GenerateDrawSurfs( void )
{
  R_SetupProjection();
  if ( tr_world )
  {
    if ( *(_DWORD *)( tr_world + 292 ) )
      R_AddWorldSurfacesDPVS();
  }
  R_AddPolygonSurfaces();
  R_AddEntitySurfaces();
  if ( r_vc_makelog->integer )
    R_ShowLightVisCachePoints();
}
#if 0
cvar_t *R_GenerateDrawSurfs()
{
  cvar_t *result;

  R_SetupProjection();
  if ( tr_world )
  {
    if ( *(_DWORD *)(tr_world + 292) )
      R_AddWorldSurfacesDPVS();
  }
  R_AddPolygonSurfaces();
  R_AddEntitySurfaces();
  result = r_vc_makelog;
  if ( r_vc_makelog->integer )
    return (cvar_t *)R_ShowLightVisCachePoints();
  return result;
}
#endif

/* ---- R_RenderView  0x004D5310 ----  [HIGH] */
_DWORD *__cdecl R_RenderView(_DWORD *a1)
{
  _DWORD *result;
  int v2;
  double value;
  double v4;
  double v5;

  result = (_DWORD *)a1[76];
  if ( (int)result > 0 )
  {
    result = (_DWORD *)a1[77];
    if ( (int)result > 0 )
    {
      ++tr_viewCount;
      qmemcpy(&tr_viewParms_originX, a1, 0x260u);
      v2 = tr_refdef_numDrawSurfs;
      tr_viewParms_frameSceneNum = tr_frameSceneNum;
      ++tr_viewCount;
      tr_viewParms_frameCount = tr_frameCount;
      value = r_highLodDist->value;
      xmodel_testLodsEnabled = value >= 0.0;
      if ( (value < 0.0) | __UNORDERED__(value, 0.0) )
        value = 0.0;
      xmodel_testLodDistances[0] = value;
      v4 = r_mediumLodDist->value;
      if ( (v4 < 0.0) | __UNORDERED__(v4, 0.0) )
        v4 = 0.0;
      xmodel_testLodDistances[5] = v4;
      v5 = r_lowLodDist->value;
      if ( (v5 < 0.0) | __UNORDERED__(v5, 0.0) )
        v5 = 0.0;
      xmodel_testLodDistances[10] = v5;
      if ( r_lodViewDist->value <= 0.0 )
        xmodel_testLodDist = 0.0;
      else
        xmodel_testLodDist = r_lodViewDist->value;
      R_RotateForViewer();
      R_SetupFrustum();
      R_GenerateDrawSurfs();
      return R_SortDrawSurfs((int *)(tr_refdef_drawSurfs + 8 * v2), tr_refdef_numDrawSurfs - v2);
    }
  }
  return result;
}

float s_flipMatrix[74] = {
     0.0f,  0.0f, -1.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,  0.0f,
     0.0f,  1.0f,  0.0f,  0.0f,
     0.0f,  0.0f,  0.0f,  1.0f,
};

unsigned char entitySurface[232] = { 3, 0, 0, 0 };
