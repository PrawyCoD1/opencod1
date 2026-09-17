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
#include "tr_dpvsplane.h"


#define outsideMapEnts             ((cvar_t *)(outsideMapEnts))
#define r_cullBModels              ((cvar_t *)(r_cullBModels))
#define r_cullXModels              ((cvar_t *)(r_cullXModels))
#define r_drawBModels              ((cvar_t *)(r_drawBModels))
#define r_drawSModels              ((cvar_t *)(r_drawSModels))
#define r_drawXModels              ((cvar_t *)(r_drawXModels))
#define r_drawentities             ((cvar_t *)(r_drawentities))
#define r_drawworld                ((cvar_t *)(r_drawworld))
#define r_lockpvs                  ((cvar_t *)(r_lockpvs))
#define r_portalbevels             ((cvar_t *)(r_portalbevels))
#define r_showCullBModels          ((cvar_t *)(r_showCullBModels))
#define r_showCullSModels          ((cvar_t *)(r_showCullSModels))
#define r_showCullXModels          ((cvar_t *)(r_showCullXModels))
#define r_showaabbtrees            ((cvar_t *)(r_showaabbtrees))
#define r_showportals              ((cvar_t *)(r_showportals))
#define r_singlecell               ((cvar_t *)(r_singlecell))
#define r_zfar                     ((cvar_t *)(r_zfar))


void AddPointToBounds( const float *point, float *mins, float *maxs );
extern int BoxOnPlaneSide();
extern int ExpandBounds();
extern int RE_AddRefEntityToScene();
extern int R_AddDebugBox();
extern int R_AddDebugLine();
extern int R_AddDebugPolygon();
extern int R_AddDrawSurf();
void VectorNormalizeFast( float *v );
extern int MatrixInverse44();


typedef struct {
    float         normal[3];   /* +0x00 */
    float         dist;        /* +0x0C */
    unsigned char sides[3];    /* +0x10  0-or-12, 4-or-16, 8-or-20 */
    unsigned char pad;         /* +0x13 */
} tr_dpvsPlane_t;

typedef struct {
    float         normal[3];   /* +0x00 */
    float         dist;        /* +0x0C */
    unsigned char type;        /* +0x10 */
    unsigned char signbits;    /* +0x11 */
    unsigned char pad[2];      /* +0x12 */
} tr_cplane_t;

#define tr_frustum            ((const tr_cplane_t *)(const void *)tr_viewParms_frustum)
#define tr_dpvsFrustumPlanes  ((tr_dpvsPlane_t *)(void *)dpvs_frustumPlanes)
#define TR_FRUSTUM_PLANES     4


/* ---- R_SetPlaneSidesDPVS  0x004E1620 ----  VERIFIED */
int __cdecl R_SetPlaneSidesDPVS(int result)
{
  int v1;
  char v2;
  char v3;

  v1 = *(_DWORD *)(result + 4);
  v2 = (*(int *)result <= 0) - 1;
  *(float *)(result + 12) = *(float *)(result + 12) - 0.001;
  *(_BYTE *)(result + 16) = v2 & 0xC;
  v3 = *(int *)(result + 8) <= 0 ? 0 : 0xC;
  *(_BYTE *)(result + 17) = v1 <= 0 ? 4 : 16;
  *(_BYTE *)(result + 18) = v3 + 8;
  return result;
}

/* ---- RE_GetFarPlaneDist  0x004E1670 ----  VERIFIED */
double RE_GetFarPlaneDist()
{
  double result;

  result = r_zfar->value;
  if ( (result == 0.0) | __UNORDERED__(result, 0.0) )
  {
    if ( glfogNum > 0 )
    {
      if ( dword_16C4C70 )
      {
        if ( dword_16C4C40 == 9729 )
          result = flt_16C4C64;
      }
    }
  }
  if ( (result < dpvs_cullDist) | __UNORDERED__(result, dpvs_cullDist) )
    return dpvs_cullDist;
  return result;
}


/* ---- R_CullDlightsForBox  0x004E16D0 ----  VERIFIED */
int __cdecl R_CullDlightsForBox(int a1, float *a2, float *a3)
{
  int v3;
  int v4;
  int v5;
  float *v6;

  v3 = a1;
  v4 = 0;
  v5 = 1;
  if ( a1 )
  {
    v6 = (float *)(tr_refdef_dlights + 116);
    do
    {
      if ( (v5 & v3) != 0 )
      {
        v3 &= ~v5;
        if ( *v6 + v6[1] > *a3
          && v6[1] - *v6 < *a2
          && *v6 + v6[2] > a3[1]
          && v6[2] - *v6 < a2[1]
          && *v6 + v6[3] > a3[2]
          && v6[3] - *v6 < a2[2] )
        {
          v4 |= v5;
        }
      }
      v5 *= 2;
      v6 += 34;
    }
    while ( v3 );
  }
  return v4;
}

/* ---- R_AddWorldSurfaceNoCull  0x004E1760 ----  VERIFIED */
int __cdecl R_AddWorldSurfaceNoCull(BOOL a1, _DWORD *a2)
{
  int v2;
  int v3;

  v2 = a2[2];
  *a2 = tr_viewCount;
  if ( *(int *)v2 < 24 )
  {
    if ( a1 )
      a1 = 0;
    return R_AddDrawSurf(storageClass, a2[1], v2, a1, 0, 0);
  }
  else
  {
    if ( a1 )
    {
      v3 = R_CullDlightsForBox(a1, (float *)(v2 + 24), (float *)(v2 + 12));
      *(_DWORD *)(v2 + 8) = v3;
      a1 = v3 != 0;
    }
    return R_AddDrawSurf(*(_DWORD *)(v2 + 4), a2[1], a2[2], a1, 0, 0);
  }
}

/* ---- R_AllocClipPlanes  0x004E17D0 ----  VERIFIED */
int __cdecl R_AllocClipPlanes(int a1)
{
  return ri_Hunk_AllocateTempMemory(20 * a1);
}

/* ---- R_FreeClipPlanes  0x004E17F0 ----  VERIFIED */
int __cdecl R_FreeClipPlanes(int a1)
{
  return ri_Hunk_FreeTempMemory(a1);
}

/* ---- R_FrustumClipPlanes  0x004E1800 ----  VERIFIED */
int R_FrustumClipPlanes()
{
  int i;

  for ( i = 0; i < TR_FRUSTUM_PLANES; ++i )
  {
    tr_dpvsPlane_t       *dst = &tr_dpvsFrustumPlanes[i];
    const tr_cplane_t    *src = &tr_frustum[i];

    *(int *)&dst->normal[0] = *(const int *)&src->normal[0];
    *(int *)&dst->normal[1] = *(const int *)&src->normal[1];
    *(int *)&dst->normal[2] = *(const int *)&src->normal[2];
    *(int *)&dst->dist      = *(const int *)&src->dist;

    dst->dist = dst->dist - 0.001f;

    dst->sides[0] = (unsigned char)(*(const int *)&dst->normal[0] <= 0 ?  0 : 12);
    dst->sides[1] = (unsigned char)(*(const int *)&dst->normal[1] <= 0 ?  4 : 16);
    dst->sides[2] = (unsigned char)(*(const int *)&dst->normal[2] <= 0 ?  8 : 20);
  }
  return TR_FRUSTUM_PLANES * 20;
}

/* ---- R_SetupTransformMatrix  0x004E18A0 ----  VERIFIED */
int R_SetupTransformMatrix()
{
  int i;
  float *v1;
  float *v2;
  double v3;
  int result;
  float v5[16]; // [esp+0h] [ebp-40h] BYREF -- retail's single 64-byte matrix

  for ( i = 0; i < 16; i += 4 )
  {
    v1 = &tr_viewParms_world_modelMatrix[2];
    v2 = &v5[i / 4u];
    do
    {
      v3 = v1[1];
      v1 += 4;
      v2 += 4;
      *(v2 - 4) = v3 * tr_viewParms_projectionMatrix[i / 4u + 12]
                + *(v1 - 6) * tr_viewParms_projectionMatrix[i / 4u]
                + *(v1 - 5) * tr_viewParms_projectionMatrix[i / 4u + 4]
                + tr_viewParms_projectionMatrix[i / 4u + 8] * *(v1 - 4);
    }
    while ( (int)v1 < (int)&flt_16C5450 );
  }
  MatrixInverse44(dpvs_transformMatrixInverse, v5);
  dpvs_transformMatrixTransposed = LODWORD(v5[0]);
  dword_11CC19C = LODWORD(v5[4]);
  dword_11CC1A0 = LODWORD(v5[8]);
  dword_11CC1A4 = LODWORD(v5[12]);
  dword_11CC1A8 = LODWORD(v5[1]);
  dword_11CC1AC = LODWORD(v5[5]);
  dword_11CC1B0 = LODWORD(v5[9]);
  dword_11CC1B4 = LODWORD(v5[13]);
  dword_11CC1B8 = LODWORD(v5[2]);
  dword_11CC1BC = LODWORD(v5[6]);
  dword_11CC1C0 = LODWORD(v5[10]);
  dword_11CC1C4 = LODWORD(v5[14]);
  dword_11CC1C8 = LODWORD(v5[3]);
  result = LODWORD(v5[15]); /* retail 0x4E1995 reads var_4, i.e. m[15] */
  dword_11CC1CC = LODWORD(v5[7]);
  dword_11CC1D0 = LODWORD(v5[11]);
  dword_11CC1D4 = LODWORD(v5[15]); /* 0x4E19A5 */
  return result;
}

/* ---- R_PortalClipPlanesInternal  0x004E19B0 ----  VERIFIED */
int __cdecl R_PortalClipPlanesInternal(_DWORD *a1, int a2, int a3)
{
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  float *v8;
  float *v9;
  int v10;
  float v11;
  float v12;
  int v13;
  float *v14;
  int v15;
  int v16;
  int v17;
  float *v18;
  int v19;
  double v20;
  int v21;
  int v22;
  int v23;
  int v24;
  int v25;
  float *v26;
  int v27;
  int v28;
  bool v29; // zf
  bool v30; // sf
  float v31;
  int v32;
  int v33;
  int v34;
  float *v35;
  double v36;
  float v37;
  float v38;
  float v39;
  double v40;
  float v41;
  float v42;
  float v43;
  double v44;
  float v45;
  float v46;
  float v47;
  double v48;
  float v49;
  float v50;
  float v51;
  float *v52;
  int v53;
  int v54;
  double v55;
  float v56;
  float v57;
  float v58;
  int result;
  float v60;
  float v61;
  float v62;
  float v63;
  float v64;
  float v65;
  float v66;
  float v67;
  float v68;
  float v69;
  float v70;
  float v71;
  float v72;
  float v73;
  float v74;
  float v75;
  unsigned int v76;
  float v77;
  float v78;
  int v79;
  float v80;
  float v81;
  float v82;
  float v83;
  float v84;
  float v85;
  float v86;
  float v87;
  float v88;
  float v89;
  float v90;
  float v91;
  float v92;
  float v93;
  float v94;
  float v95;
  float v96;
  float v97;
  int v98;
  float v99;
  float v100;
  float v101;
  float v102;
  float v103;
  float portalPts[15];      /* [ebp-6048h]  4 portal corners + closing copy */
  float relPts[3075];       /* [ebp-600Ch]  winding points - r_viewOrigin   */
  float edgeNormals[3072];  /* [ebp-3000h]  one plane normal per edge       */

  v3 = a2;
  if ( r_portalbevels->value <= 0.0 )
  {
    v98 = ri_Hunk_AllocateTempMemory(20 * a2);
  }
  else
  {
    *(float *)&v4 = 1.0;
    *(float *)&v5 = -1.0;
    v6 = 0;
    v98 = ri_Hunk_AllocateTempMemory(4 * (5 * a2 + 20));
    v77 = 1.0;
    v82 = 1.0;
    v90 = -1.0;
    v93 = -1.0;
    if ( a2 >= 4 )
    {
      v7 = 3;
      v8 = (float *)(a3 + 4);
      do
      {
        v60 = *(float *)&dword_11CC1CC * *v8
            + *(float *)&dword_11CC1D0 * v8[1]
            + *(float *)&dword_11CC1C8 * *(v8 - 1)
            + *(float *)&dword_11CC1D4;
        v61 = 1.0 / v60;
        v84 = (*(float *)&dword_11CC19C * *v8
             + *(float *)&dword_11CC1A0 * v8[1]
             + *(float *)&dpvs_transformMatrixTransposed * *(v8 - 1)
             + *(float *)&dword_11CC1A4)
            * v61;
        v62 = (*(float *)&dword_11CC1AC * *v8
             + *(float *)&dword_11CC1B0 * v8[1]
             + *(float *)&dword_11CC1A8 * *(v8 - 1)
             + *(float *)&dword_11CC1B4)
            * v61;
        if ( v77 > (double)v84 )
          v77 = v84;
        if ( (v90 < (double)v84) | __UNORDERED__(v90, v84) )
          v90 = v84;
        if ( v82 > (double)v62 )
          v82 = v62;
        if ( (v93 < (double)v62) | __UNORDERED__(v93, v62) )
        {
          *(float *)&v5 = v62;
          v93 = v62;
        }
        v63 = *(float *)&dword_11CC1D0 * v8[4]
            + *(float *)&dword_11CC1CC * v8[3]
            + *(float *)&dword_11CC1C8 * v8[2]
            + *(float *)&dword_11CC1D4;
        v64 = 1.0 / v63;
        v85 = (*(float *)&dword_11CC1A0 * v8[4]
             + *(float *)&dword_11CC19C * v8[3]
             + *(float *)&dpvs_transformMatrixTransposed * v8[2]
             + *(float *)&dword_11CC1A4)
            * v64;
        v65 = (*(float *)&dword_11CC1B0 * v8[4]
             + *(float *)&dword_11CC1AC * v8[3]
             + *(float *)&dword_11CC1A8 * v8[2]
             + *(float *)&dword_11CC1B4)
            * v64;
        if ( v77 > (double)v85 )
          v77 = v85;
        if ( (v90 < (double)v85) | __UNORDERED__(v90, v85) )
          v90 = v85;
        if ( v82 > (double)v65 )
          v82 = v65;
        if ( (v93 < (double)v65) | __UNORDERED__(v93, v65) )
        {
          *(float *)&v5 = v65;
          v93 = v65;
        }
        v66 = *(float *)&dword_11CC1D0 * v8[7]
            + *(float *)&dword_11CC1CC * v8[6]
            + *(float *)&dword_11CC1C8 * v8[5]
            + *(float *)&dword_11CC1D4;
        v67 = 1.0 / v66;
        v86 = (*(float *)&dword_11CC1A0 * v8[7]
             + *(float *)&dword_11CC19C * v8[6]
             + *(float *)&dpvs_transformMatrixTransposed * v8[5]
             + *(float *)&dword_11CC1A4)
            * v67;
        v68 = (*(float *)&dword_11CC1B0 * v8[7]
             + *(float *)&dword_11CC1AC * v8[6]
             + *(float *)&dword_11CC1A8 * v8[5]
             + *(float *)&dword_11CC1B4)
            * v67;
        if ( v77 > (double)v86 )
          v77 = v86;
        if ( (v90 < (double)v86) | __UNORDERED__(v90, v86) )
          v90 = v86;
        if ( v82 > (double)v68 )
          v82 = v68;
        if ( (v93 < (double)v68) | __UNORDERED__(v93, v68) )
        {
          *(float *)&v5 = v68;
          v93 = v68;
        }
        v69 = *(float *)&dword_11CC1D0 * v8[10]
            + *(float *)&dword_11CC1CC * v8[9]
            + *(float *)&dword_11CC1C8 * v8[8]
            + *(float *)&dword_11CC1D4;
        v70 = 1.0 / v69;
        v87 = (*(float *)&dword_11CC1A0 * v8[10]
             + *(float *)&dword_11CC19C * v8[9]
             + *(float *)&dpvs_transformMatrixTransposed * v8[8]
             + *(float *)&dword_11CC1A4)
            * v70;
        v71 = (*(float *)&dword_11CC1B0 * v8[10]
             + *(float *)&dword_11CC1AC * v8[9]
             + *(float *)&dword_11CC1A8 * v8[8]
             + *(float *)&dword_11CC1B4)
            * v70;
        if ( v77 > (double)v87 )
          v77 = v87;
        if ( (v90 < (double)v87) | __UNORDERED__(v90, v87) )
          v90 = v87;
        if ( v82 > (double)v71 )
          v82 = v71;
        if ( (v93 < (double)v71) | __UNORDERED__(v93, v71) )
        {
          *(float *)&v5 = v71;
          v93 = v71;
        }
        v7 += 4;
        v6 += 4;
        v8 += 12;
      }
      while ( v7 < a2 );
      *(float *)&v4 = v82;
    }
    if ( v6 < a2 )
    {
      v9 = (float *)(a3 + 12 * v6 + 4);
      v10 = a2 - v6;
      do
      {
        v72 = *(float *)&dword_11CC1D0 * v9[1]
            + *(float *)&dword_11CC1C8 * *(v9 - 1)
            + *(float *)&dword_11CC1CC * *v9
            + *(float *)&dword_11CC1D4;
        v73 = 1.0 / v72;
        v88 = (*(float *)&dword_11CC1A0 * v9[1]
             + *(float *)&dpvs_transformMatrixTransposed * *(v9 - 1)
             + *(float *)&dword_11CC19C * *v9
             + *(float *)&dword_11CC1A4)
            * v73;
        v74 = (*(float *)&dword_11CC1B0 * v9[1]
             + *(float *)&dword_11CC1A8 * *(v9 - 1)
             + *(float *)&dword_11CC1AC * *v9
             + *(float *)&dword_11CC1B4)
            * v73;
        if ( v77 > (double)v88 )
          v77 = v88;
        if ( (v90 < (double)v88) | __UNORDERED__(v90, v88) )
          v90 = v88;
        if ( v82 > (double)v74 )
        {
          *(float *)&v4 = v74;
          v82 = v74;
        }
        if ( (v93 < (double)v74) | __UNORDERED__(v93, v74) )
        {
          *(float *)&v5 = v74;
          v93 = v74;
        }
        v9 += 3;
        --v10;
      }
      while ( v10 );
    }
    v11 = v77;
    v12 = v90;
    v13 = 0;
    v14 = &portalPts[1];
    do
    {
      v78 = v11;
      if ( v13 >= 2 )
        v78 = v12;
      if ( v13 == 1 || (v83 = *(float *)&v5, v13 == 2) )
        v83 = *(float *)&v4;
      ++v13;
      v14 += 3;
      v91 = dpvs_transformMatrixInverse[4] * v83 + dpvs_transformMatrixInverse[0] * v78 + dpvs_transformMatrixInverse[12];
      v94 = dpvs_transformMatrixInverse[5] * v83 + dpvs_transformMatrixInverse[1] * v78 + dpvs_transformMatrixInverse[13];
      v95 = dpvs_transformMatrixInverse[6] * v83 + dpvs_transformMatrixInverse[2] * v78 + dpvs_transformMatrixInverse[14];
      v96 = dpvs_transformMatrixInverse[7] * v83 + dpvs_transformMatrixInverse[3] * v78 + dpvs_transformMatrixInverse[15];
      v75 = 1.0 / v96;
      *(v14 - 4) = v91 * v75;
      *(v14 - 3) = v94 * v75;
      *(v14 - 2) = v95 * v75;
    }
    while ( v13 < 4 );
    portalPts[12] = portalPts[0];
    portalPts[13] = portalPts[1];
    portalPts[14] = portalPts[2];
  }
  v15 = 0;
  v16 = 0;
  v79 = 0;
  if ( a2 > 0 )
  {
    v17 = a3 + 8;
    v18 = &relPts[1];
    v19 = a2;
    v16 = a2;
    do
    {
      v18 += 3;
      v20 = *(float *)(v17 - 8) - *(float *)&dpvs_viewOrigin;
      v17 += 12;
      --v19;
      *(v18 - 4) = v20;
      *(v18 - 3) = *(float *)(v17 - 16) - *(float *)&dword_11CC104;
      *(float *)((char *)relPts + (v17 - a3 - 12)) = *(float *)(v17 - 12) - *(float *)&dword_11CC108;
    }
    while ( v19 );
  }
  v21 = 12 * v16;
  *(float *)((char *)relPts + v21) = relPts[0];
  *(float *)((char *)&relPts[1] + v21) = relPts[1];
  *(float *)((char *)&relPts[2] + v21) = relPts[2];
  if ( a2 > 0 )
  {
    v22 = 0;
    v23 = a2;
    do
    {
      edgeNormals[v22]     = relPts[4 + v22] * relPts[2 + v22]
                           - relPts[5 + v22] * relPts[1 + v22];
      edgeNormals[v22 + 1] = relPts[5 + v22] * relPts[v22]
                           - relPts[2 + v22] * relPts[3 + v22];
      edgeNormals[v22 + 2] = relPts[1 + v22] * relPts[3 + v22]
                           - relPts[4 + v22] * relPts[v22];
      VectorNormalizeFast(&edgeNormals[v22]);
      v22 += 3;
      --v23;
    }
    while ( v23 );
  }
  if ( r_portalbevels->value > 0.0 )
  {
    v24 = v98 + 4;
    v76 = 0;
    relPts[0] = portalPts[0] - *(float *)&dpvs_viewOrigin;
    relPts[12] = relPts[0];
    relPts[1] = portalPts[1] - *(float *)&dword_11CC104;
    relPts[13] = relPts[1];
    relPts[2] = portalPts[2] - *(float *)&dword_11CC108;
    relPts[14] = relPts[2];
    relPts[3] = portalPts[3] - *(float *)&dpvs_viewOrigin;
    relPts[4] = portalPts[4] - *(float *)&dword_11CC104;
    relPts[5] = portalPts[5] - *(float *)&dword_11CC108;
    relPts[6] = portalPts[6] - *(float *)&dpvs_viewOrigin;
    relPts[7] = portalPts[7] - *(float *)&dword_11CC104;
    relPts[8] = portalPts[8] - *(float *)&dword_11CC108;
    relPts[9] = portalPts[9] - *(float *)&dpvs_viewOrigin;
    relPts[10] = portalPts[10] - *(float *)&dword_11CC104;
    relPts[11] = portalPts[11] - *(float *)&dword_11CC108;
    do
    {
      *(float *)(v24 - 4) = relPts[v76 / 4 + 4] * relPts[v76 / 4 + 2]
                          - relPts[v76 / 4 + 1] * relPts[v76 / 4 + 5];
      *(float *)v24 = relPts[v76 / 4] * relPts[v76 / 4 + 5]
                    - relPts[v76 / 4 + 3] * relPts[v76 / 4 + 2];
      *(float *)(v24 + 4) = relPts[v76 / 4 + 1] * relPts[v76 / 4 + 3]
                          - relPts[v76 / 4] * relPts[v76 / 4 + 4];
      VectorNormalizeFast((float *)(v24 - 4));
      v25 = 0;
      if ( v3 <= 0 )
      {
LABEL_70:
        if ( r_showportals->integer )
        {
          R_AddDebugLine((float *)((char *)portalPts + v76), &portalPts[v76 / 4 + 3], acceptedBevelDebugColor);
          v3 = a2;
          v15 = v79;
        }
        *(float *)(v24 + 8) = *(float *)&dword_11CC104 * *(float *)v24
                            + *(float *)&dword_11CC108 * *(float *)(v24 + 4)
                            + *(float *)&dpvs_viewOrigin * *(float *)(v24 - 4);
        v27 = *(_DWORD *)(v24 - 4);
        v28 = *(_DWORD *)(v24 + 4);
        v29 = v27 == 0;
        v30 = v27 < 0;
        v31 = *(float *)v24;
        *(float *)(v24 + 8) = *(float *)(v24 + 8) - 0.001;
        ++v15;
        *(_BYTE *)(v24 + 12) = v30 || v29 ? 0 : 0xC;
        *(_BYTE *)(v24 + 13) = SLODWORD(v31) <= 0 ? 4 : 16;
        *(_BYTE *)(v24 + 14) = v28 <= 0 ? 8 : 20;
        v79 = v15;
        v24 += 20;
      }
      else
      {
        v26 = &edgeNormals[1];
        while ( *(v26 - 1) * *(float *)(v24 - 4) + v26[1] * *(float *)(v24 + 4) + *v26 * *(float *)v24 <= r_portalbevels->value )
        {
          ++v25;
          v26 += 3;
          if ( v25 >= v3 )
            goto LABEL_70;
        }
        if ( (r_showportals->integer & 2) != 0 )
        {
          R_AddDebugLine((float *)((char *)portalPts + v76), &portalPts[v76 / 4 + 3], rejectedBevelDebugColor);
          v3 = a2;
          v15 = v79;
        }
      }
      v76 += 12;
    }
    while ( (int)v76 < 48 );
  }
  v32 = 0;
  if ( v3 >= 4 )
  {
    v33 = 3;
    v34 = v98 + 20 * v15;
    v35 = &edgeNormals[1];
    do
    {
      v36 = *v35 * *v35 + v35[1] * v35[1] + *(v35 - 1) * *(v35 - 1);
      if ( !((v36 == 0.0) | __UNORDERED__(v36, 0.0)) )
      {
        v37 = *(v35 - 1);
        *(float *)v34 = v37;
        v89 = v37;
        v38 = *v35;
        *(float *)(v34 + 4) = *v35;
        v100 = v38;
        v39 = v35[1];
        *(float *)(v34 + 8) = v39;
        *(float *)(v34 + 12) = v39 * *(float *)&dword_11CC108
                             + v89 * *(float *)&dpvs_viewOrigin
                             + v100 * *(float *)&dword_11CC104;
        LOBYTE(v39) = (*(int *)v34 <= 0) - 1;
        *(float *)(v34 + 12) = *(float *)(v34 + 12) - 0.001;
        *(_BYTE *)(v34 + 16) = LOBYTE(v39) & 0xC;
        *(_BYTE *)(v34 + 17) = *(int *)(v34 + 4) <= 0 ? 4 : 16;
        *(_BYTE *)(v34 + 18) = *(int *)(v34 + 8) <= 0 ? 8 : 20;
        ++v15;
        v34 += 20;
      }
      v40 = v35[4] * v35[4] + v35[3] * v35[3] + v35[2] * v35[2];
      if ( !((v40 == 0.0) | __UNORDERED__(v40, 0.0)) )
      {
        v41 = v35[2];
        *(float *)v34 = v41;
        v101 = v41;
        v42 = v35[3];
        *(float *)(v34 + 4) = v42;
        v99 = v42;
        v43 = v35[4];
        *(float *)(v34 + 8) = v43;
        *(float *)(v34 + 12) = v43 * *(float *)&dword_11CC108
                             + v99 * *(float *)&dword_11CC104
                             + v101 * *(float *)&dpvs_viewOrigin;
        LOBYTE(v43) = (*(int *)v34 <= 0) - 1;
        *(float *)(v34 + 12) = *(float *)(v34 + 12) - 0.001;
        *(_BYTE *)(v34 + 16) = LOBYTE(v43) & 0xC;
        *(_BYTE *)(v34 + 17) = *(int *)(v34 + 4) <= 0 ? 4 : 16;
        *(_BYTE *)(v34 + 18) = *(int *)(v34 + 8) <= 0 ? 8 : 20;
        ++v15;
        v34 += 20;
      }
      v44 = v35[7] * v35[7] + v35[6] * v35[6] + v35[5] * v35[5];
      if ( !((v44 == 0.0) | __UNORDERED__(v44, 0.0)) )
      {
        v45 = v35[5];
        *(float *)v34 = v45;
        v102 = v45;
        v46 = v35[6];
        *(float *)(v34 + 4) = v46;
        v103 = v46;
        v47 = v35[7];
        *(float *)(v34 + 8) = v47;
        *(float *)(v34 + 12) = v47 * *(float *)&dword_11CC108
                             + v103 * *(float *)&dword_11CC104
                             + v102 * *(float *)&dpvs_viewOrigin;
        LOBYTE(v47) = (*(int *)v34 <= 0) - 1;
        *(float *)(v34 + 12) = *(float *)(v34 + 12) - 0.001;
        *(_BYTE *)(v34 + 16) = LOBYTE(v47) & 0xC;
        *(_BYTE *)(v34 + 17) = *(int *)(v34 + 4) <= 0 ? 4 : 16;
        *(_BYTE *)(v34 + 18) = *(int *)(v34 + 8) <= 0 ? 8 : 20;
        ++v15;
        v34 += 20;
      }
      v48 = v35[10] * v35[10] + v35[9] * v35[9] + v35[8] * v35[8];
      if ( !((v48 == 0.0) | __UNORDERED__(v48, 0.0)) )
      {
        v49 = v35[8];
        *(float *)v34 = v49;
        v80 = v49;
        v50 = v35[9];
        *(float *)(v34 + 4) = v50;
        v92 = v50;
        v51 = v35[10];
        *(float *)(v34 + 8) = v51;
        *(float *)(v34 + 12) = v51 * *(float *)&dword_11CC108
                             + v92 * *(float *)&dword_11CC104
                             + v80 * *(float *)&dpvs_viewOrigin;
        LOBYTE(v51) = (*(int *)v34 <= 0) - 1;
        *(float *)(v34 + 12) = *(float *)(v34 + 12) - 0.001;
        *(_BYTE *)(v34 + 16) = LOBYTE(v51) & 0xC;
        *(_BYTE *)(v34 + 17) = *(int *)(v34 + 4) <= 0 ? 4 : 16;
        *(_BYTE *)(v34 + 18) = *(int *)(v34 + 8) <= 0 ? 8 : 20;
        ++v15;
        v34 += 20;
      }
      v33 += 4;
      v32 += 4;
      v35 += 12;
    }
    while ( v33 < v3 );
  }
  if ( v32 >= v3 )
  {
    result = v98;
    *a1 = v15;
  }
  else
  {
    v52 = &edgeNormals[3 * v32 + 1];
    v53 = v98 + 20 * v15 + 4;
    v54 = v3 - v32;
    do
    {
      v55 = *v52 * *v52 + v52[1] * v52[1] + *(v52 - 1) * *(v52 - 1);
      if ( !((v55 == 0.0) | __UNORDERED__(v55, 0.0)) )
      {
        v56 = *(v52 - 1);
        *(float *)(v53 - 4) = v56;
        v97 = v56;
        v57 = *v52;
        *(float *)v53 = *v52;
        v81 = v57;
        v58 = v52[1];
        *(float *)(v53 + 4) = v58;
        *(float *)(v53 + 8) = v81 * *(float *)&dword_11CC104
                            + *(float *)&dword_11CC108 * v58
                            + v97 * *(float *)&dpvs_viewOrigin;
        LOBYTE(v58) = (*(int *)(v53 - 4) <= 0) - 1;
        *(float *)(v53 + 8) = *(float *)(v53 + 8) - 0.001;
        *(_BYTE *)(v53 + 12) = LOBYTE(v58) & 0xC;
        *(_BYTE *)(v53 + 13) = *(int *)v53 <= 0 ? 4 : 16;
        *(_BYTE *)(v53 + 14) = *(int *)(v53 + 4) <= 0 ? 8 : 20;
        ++v15;
        v53 += 20;
      }
      v52 += 3;
      --v54;
    }
    while ( v54 );
    *a1 = v15;
    return v98;
  }
  return result;
}


/* ---- R_CullOccluderByPlane  0x004E2810 ----  VERIFIED */
int __cdecl R_CullOccluderByPlane(int a1, float *a2)
{
  int v2;
  int v3;
  float *i;

  v2 = *(_DWORD *)(a1 + 16);
  v3 = 0;
  if ( v2 <= 0 )
    return 1;
  for ( i = *(float **)(a1 + 20); i[1] * a2[1] + i[2] * a2[2] + *i * *a2 <= a2[3]; i += 3 )
  {
    if ( ++v3 >= v2 )
      return 1;
  }
  return 0;
}

/* ---- R_AddCellOccluders  0x004E2860 ----  [HIGH] */
void __cdecl R_AddCellOccluders(int a1, float *a2, int a3)
{
  int v3;
  int v4;
  _DWORD *v5;
  int v6;
  float *v7;
  int j;
  int v9;
  float *v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  unsigned __int8 v17;
  int v18;
  float *v19;
  double v20;
  float *v21;
  double v22;
  float *v23;
  int v24;
  int v25;
  int v26;
  int k;
  int i;
  float v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;

  v3 = 0;
  v4 = a1;
  dpvs_occluderCount = 0;
  dpvs_activePlaneCount = 0;
  for ( i = 0; v3 < *(_DWORD *)(v4 + 48); i = v3 )
  {
    v5 = *(_DWORD **)(*(_DWORD *)(v4 + 44) + 4 * v3);
    v6 = 0;
    if ( (int)v5[4] > 0 )
    {
      v7 = (float *)v5[5];
      while ( v7[1] * *(float *)(dpvs_activeNearPlane + 4)
            + v7[2] * *(float *)(dpvs_activeNearPlane + 8)
            + *(float *)dpvs_activeNearPlane * *v7 <= *(float *)(dpvs_activeNearPlane + 12) )
      {
        ++v6;
        v7 += 3;
        if ( v6 >= v5[4] )
          goto LABEL_28;
      }
      for ( j = a3; j; a2 += 5 )
      {
        v9 = 0;
        v10 = (float *)v5[5];
        while ( v10[2] * a2[2] + v10[1] * a2[1] + *a2 * *v10 <= a2[3] )
        {
          ++v9;
          v10 += 3;
          if ( v9 >= v5[4] )
            goto LABEL_27;
        }
        a3 = --j;
      }
      if ( dpvs_occluderCount == 1024 )
        ri_Error(1, "\x15" "Too many active occluder brushes");
      *(_DWORD *)(dpvs_occluders + 4 * dpvs_occluderCount++) = v5;
      v5[6] = 0x7FFFFFFF;
      v5[7] = dpvs_activePlaneCount;
      v5[8] = dpvs_activePlanes + 20 * dpvs_activePlaneCount;
      v11 = 0;
      for ( k = 0; k < v5[1]; ++k )
      {
        *(_BYTE *)(*v5 + v11 + 19) = 0;
        v12 = *v5 + v11;
        if ( *(float *)&dword_11CC108 * *(float *)(v12 + 8)
           + *(float *)&dword_11CC104 * *(float *)(v12 + 4)
           + *(float *)&dpvs_viewOrigin * *(float *)v12 > *(float *)(v12 + 12) )
        {
          *(_BYTE *)(v12 + 19) = 1;
          v13 = dpvs_activePlaneCount;
          if ( dpvs_activePlaneCount == 6144 )
          {
            ri_Error(1, "\x15" "Too many active occluder faces");
            v13 = dpvs_activePlaneCount;
          }
          qmemcpy((void *)(dpvs_activePlanes + 20 * v13), (const void *)(v11 + *v5), 0x14u);
          ++dpvs_activePlaneCount;
        }
        v11 += 20;
      }
      v14 = 0;
      if ( (int)v5[2] > 0 )
      {
        v15 = 0;
        do
        {
          v16 = v5[3];
          v17 = *(_BYTE *)(*(_DWORD *)(v16 + v15) + 19);
          if ( v17 != *(_BYTE *)(*(_DWORD *)(v16 + v15 + 4) + 19) )
          {
            v18 = v17;
            v19 = *(float **)(v16 + 4 * ((v17 == 0) + 4 * v14) + 8);
            v29 = *v19 - *(float *)&dpvs_viewOrigin;
            v30 = v19[1] - *(float *)&dword_11CC104;
            v20 = v19[2];
            v21 = *(float **)(v16 + 4 * (v18 + 4 * v14) + 8);
            v31 = v20 - *(float *)&dword_11CC108;
            v32 = *v21 - *(float *)&dpvs_viewOrigin;
            v33 = v21[1] - *(float *)&dword_11CC104;
            v22 = v21[2];
            v23 = (float *)(dpvs_activePlanes + 20 * dpvs_activePlaneCount);
            v34 = v22 - *(float *)&dword_11CC108;
            *v23 = v33 * v31 - v34 * v30;
            v23[1] = v34 * v29 - v32 * v31;
            v23[2] = v32 * v30 - v33 * v29;
            VectorNormalizeFast((float *)(dpvs_activePlanes + 20 * dpvs_activePlaneCount));
            *(float *)(dpvs_activePlanes + 20 * dpvs_activePlaneCount + 12) = *(float *)&dword_11CC108
                                                                * *(float *)(dpvs_activePlanes + 20 * dpvs_activePlaneCount + 8)
                                                                + *(float *)&dword_11CC104
                                                                * *(float *)(dpvs_activePlanes + 20 * dpvs_activePlaneCount + 4)
                                                                + *(float *)&dpvs_viewOrigin
                                                                * *(float *)(dpvs_activePlanes + 20 * dpvs_activePlaneCount);
            v24 = dpvs_activePlanes + 20 * dpvs_activePlaneCount;
            v25 = *(_DWORD *)(v24 + 4);
            *(_BYTE *)(v24 + 16) = *(int *)v24 <= 0 ? 0 : 0xC;
            v26 = *(_DWORD *)(v24 + 8);
            *(float *)(v24 + 12) = *(float *)(v24 + 12) - 0.001;
            *(_BYTE *)(v24 + 17) = v25 <= 0 ? 4 : 16;
            *(_BYTE *)(v24 + 18) = v26 <= 0 ? 8 : 20;
            ++dpvs_activePlaneCount;
          }
          ++v14;
          v15 += 16;
        }
        while ( v14 < v5[2] );
      }
      v5[7] = dpvs_activePlaneCount - v5[7];
LABEL_27:
      v4 = a1;
      v3 = i;
    }
LABEL_28:
    ++v3;
  }
}

/* ---- R_CellForCamera  0x004E2BE0 ----  VERIFIED */
int R_CellForCamera()
{
  _DWORD *v0;

  if ( !tr_world )
    ri_Error(1, "\x15" "R_CellForCamera: bad model");
  v0 = *(_DWORD **)(tr_world + 152);
  while ( *v0 == -1 )
  {
    if ( *(float *)&dword_11CC108 * *(float *)(v0[3] + 8)
       + *(float *)&dword_11CC104 * *(float *)(v0[3] + 4)
       + *(float *)&dpvs_viewOrigin * *(float *)v0[3]
       - *(float *)(v0[3] + 12) <= 0.0 )
      v0 = (_DWORD *)v0[5];
    else
      v0 = (_DWORD *)v0[4];
  }
  return v0[2];
}

/* ---- R_PortalBehindAnyPlane  0x004E2C60 ----  VERIFIED */
int __cdecl R_PortalBehindAnyPlane(int a1, float *a2, int a3)
{
  int v3;
  float *v5;
  int i;

  v3 = a3;
  if ( a3 )
  {
    while ( 2 )
    {
      v5 = *(float **)(a1 + 24);
      for ( i = *(_DWORD *)(a1 + 28); ; --i )
      {
        if ( !i )
          return 1;
        if ( v5[2] * a2[2] + v5[1] * a2[1] + *v5 * *a2 > a2[3] )
          break;
        v5 += 3;
      }
      --v3;
      a2 += 5;
      if ( v3 )
        continue;
      break;
    }
  }
  return 0;
}

/* ---- R_PortalBehindAllPlanes  0x004E2CC0 ----  VERIFIED */
int __cdecl R_PortalBehindAllPlanes(int a1, float *a2, int a3)
{
  int v3;
  int v5;
  float *v6;

  v3 = a3;
  if ( !a3 )
    return 1;
  while ( 1 )
  {
    v5 = *(_DWORD *)(a1 + 28);
    v6 = *(float **)(a1 + 24);
    if ( v5 )
      break;
LABEL_5:
    --v3;
    a2 += 5;
    if ( !v3 )
      return 1;
  }
  while ( v6[2] * a2[2] + v6[1] * a2[1] + *v6 * *a2 <= a2[3] )
  {
    --v5;
    v6 += 3;
    if ( !v5 )
      goto LABEL_5;
  }
  return 0;
}

/* ---- R_ChopPortalWinding  0x004E2D20 ----  VERIFIED */
float *__cdecl R_ChopPortalWinding(const float *plane, int a1, int *a2, float *a3)
{
  const float *v4;
  int v5;
  int v6;
  int v7;
  float *v8;
  double v9;
  double v10;
  float *result;
  int v15;
  float *v16;
  int v17;
  int v18;
  float *v19;
  int v20;
  int v21;
  double v22;
  float *v23;
  _DWORD v24[1025];
  float v25[1025];

  v4 = plane;
  v5 = 0;
  v6 = 0;
  v7 = 0;
  if ( *a2 <= 0 )
    goto LABEL_9;
  v8 = (float *)(a1 + 4);
  do
  {
    v9 = v8[1];
    v24[v7] = 2;
    v10 = v9 * v4[2] + *(v8 - 1) * *v4 + v4[1] * *v8 - v4[3];
    v25[v7] = v10;
    if ( v10 < -0.001 )
    {
      v24[v7] = 1;
      ++v6;
    }
    else if ( v10 > 0.001 )
    {
      v24[v7] = 0;
      ++v5;
    }
    ++v7;
    v8 += 3;
  }
  while ( v7 < *a2 );
  if ( !v5 )
  {
LABEL_9:
    *a2 = 0;
    return 0;
  }
  if ( !v6 )
    return (float *)a1;
  v15 = v24[0];
  v25[v7] = v25[0];
  v16 = a3;
  v17 = 0;
  v18 = 0;
  v24[v7] = v15;
  v19 = (float *)(a1 + 8);
  while ( 1 )
  {
    v20 = v24[v18];
    if ( v20 != 2 )
      break;
    *v16 = *(v19 - 2);
    v16[1] = *(v19 - 1);
    v16[2] = *v19;
    ++v17;
    v16 += 3;
LABEL_20:
    ++v18;
    v19 += 3;
    if ( v18 >= *a2 )
      goto LABEL_21;
  }
  if ( !v20 )
  {
    *v16 = *(v19 - 2);
    v16[1] = *(v19 - 1);
    v16[2] = *v19;
    ++v17;
    v16 += 3;
  }
  v21 = v24[v18 + 1];
  if ( v21 == 2 )
    goto LABEL_20;
  if ( v21 == v20 )
    goto LABEL_20;
  ++v17;
  v22 = v25[v18] / (v25[v18] - v25[v18 + 1]);
  v16 += 3;
  v23 = (float *)(a1 + 12 * ((v18 + 1) % *a2));
  *(v16 - 3) = (*v23 - *(v19 - 2)) * v22 + *(v19 - 2);
  *(v16 - 2) = (v23[1] - *(v19 - 1)) * v22 + *(v19 - 1);
  *(v16 - 1) = (v23[2] - *v19) * v22 + *v19;
  if ( v17 != 1024 )
    goto LABEL_20;
LABEL_21:
  result = a3;
  *a2 = v17;
  return result;
}

/* ---- R_PortalClipPlanes  0x004E2EE0 ----  VERIFIED */
char *__cdecl R_PortalClipPlanes(int *a1, int a2, int a3, int a4, char *a5)
{
  float *v7;
  double v8;
  char *result;
  char *v10;
  int v11;
  char v12[24576]; // [esp+14h] [ebp-6000h] BYREF

  *a1 = *(_DWORD *)(a3 + 28);
  /* 0x004E2EFF: mov ecx, [esp+arg_4] -- the plane is a4 */
  v7 = R_ChopPortalWinding((const float *)a4, *(_DWORD *)(a3 + 24), a1, (float *)v12);
  if ( !*a1 )
    return 0;
  v8 = *(float *)(a3 + 12)
     - (*(float *)&dword_11CC108 * *(float *)(a3 + 8)
      + *(float *)&dword_11CC104 * *(float *)(a3 + 4)
      + *(float *)&dpvs_viewOrigin * *(float *)a3);
  if ( (v8 < 1.0) | __UNORDERED__(v8, 1.0) )
  {
    *a1 = a2;
    result = (char *)ri_Hunk_AllocateTempMemory(20 * a2);
    qmemcpy(result, a5, 20 * a2);
    return result;
  }
  if ( dpvs_activeFarPlane )
  {
    /* 0x004E2F87: mov ecx, dpvs_activeFarPlane -- the plane is the global itself */
    v7 = R_ChopPortalWinding(
           (const float *)dpvs_activeFarPlane,
           (int)v7,
           a1,
           (float *)&v12[12288 * (v7 == (float *)v12)]);
    if ( !*a1 )
      return 0;
  }
  v11 = 0;
  if ( a2 > 0 )
  {
    v10 = a5;
    do
    {
      /* 0x004E2FEC: mov ecx, ebx -- v10 is the clip plane */
      v7 = R_ChopPortalWinding(
             (const float *)v10,
             (int)v7,
             a1,
             (float *)&v12[12288 * (v7 == (float *)v12)]);
      if ( !*a1 )
        return 0;
      v10 += 20;
    }
    while ( ++v11 < a2 );
  }
  if ( r_showportals->integer )
    R_AddDebugPolygon(dword_540568, *a1, v7);
  return (char *)R_PortalClipPlanesInternal(a1, *a1, (int)v7);
}

/* ---- R_AddTrianglesSurface  0x004E3070 ----  [HIGH] */
void __cdecl R_AddTrianglesSurface(int a1, _DWORD *a2, BOOL a3, int a4, int a5)
{
  int v5;
  float *v6;
  int v7;
  int v8;
  double v9;
  double v10;
  double v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  double v20;
  int v21;

  if ( *a2 != tr_viewCount )
  {
    v5 = a2[2];
    v21 = v5;
    v6 = (float *)(v5 + 12);
    if ( a5 >= dpvs_cullPlaneLimit || (v7 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, a1 <= 0) )
    {
LABEL_7:
      v10 = *(float *)((char *)v6 + *(unsigned __int8 *)(dpvs_activeNearPlane + 18)) * *(float *)(dpvs_activeNearPlane + 8)
          + *(float *)((char *)v6 + *(unsigned __int8 *)(dpvs_activeNearPlane + 17)) * *(float *)(dpvs_activeNearPlane + 4)
          + *(float *)((char *)v6 + *(unsigned __int8 *)(dpvs_activeNearPlane + 16)) * *(float *)dpvs_activeNearPlane;
      if ( !((v10 < *(float *)(dpvs_activeNearPlane + 12)) | __UNORDERED__(v10, *(float *)(dpvs_activeNearPlane + 12))) )
      {
        if ( !dpvs_activeFarPlane
          || (v11 = *(float *)((char *)v6 + *(unsigned __int8 *)(dpvs_activeFarPlane + 18)) * *(float *)(dpvs_activeFarPlane + 8)
                  + *(float *)((char *)v6 + *(unsigned __int8 *)(dpvs_activeFarPlane + 17)) * *(float *)(dpvs_activeFarPlane + 4)
                  + *(float *)((char *)v6 + *(unsigned __int8 *)(dpvs_activeFarPlane + 16)) * *(float *)dpvs_activeFarPlane,
              !((v11 < *(float *)(dpvs_activeFarPlane + 12)) | __UNORDERED__(v11, *(float *)(dpvs_activeFarPlane + 12)))) )
        {
          v12 = 0;
          if ( dpvs_occluderCount <= 0 )
          {
LABEL_18:
            if ( (r_showaabbtrees->integer & 2) != 0 )
              R_AddDebugBox(dword_541840, (float *)(v5 + 24), v6);
            R_AddWorldSurfaceNoCull(a3, a2);
          }
          else
          {
            while ( 1 )
            {
              v13 = *(_DWORD *)(dpvs_occluders + 4 * v12);
              if ( a5 < *(_DWORD *)(v13 + 24) )
                break;
LABEL_16:
              if ( ++v12 >= dpvs_occluderCount )
              {
                v5 = v21;
                goto LABEL_18;
              }
            }
            *(_DWORD *)(v13 + 24) = 0x7FFFFFFF;
            v14 = *(_DWORD *)(dpvs_occluders + 4 * v12);
            v15 = 0;
            v16 = 0;
            while ( v15 < *(_DWORD *)(v14 + 28) )
            {
              v17 = *(_DWORD *)(v14 + 32);
              v18 = *(unsigned __int8 *)(v17 + v16 + 18);
              v19 = v16 + v17;
              v20 = *(float *)((char *)v6 + v18) * *(float *)(v19 + 8)
                  + *(float *)((char *)v6 + *(unsigned __int8 *)(v19 + 17)) * *(float *)(v19 + 4)
                  + *(float *)((char *)v6 + *(unsigned __int8 *)(v19 + 16)) * *(float *)v19;
              if ( !((v20 < *(float *)(v19 + 12)) | __UNORDERED__(v20, *(float *)(v19 + 12))) )
                goto LABEL_16;
              ++v15;
              v16 += 20;
            }
          }
        }
      }
    }
    else
    {
      v8 = a4 + 8;
      while ( 1 )
      {
        v9 = *(float *)((char *)v6 + *(unsigned __int8 *)(v8 + 8)) * *(float *)(v8 - 8)
           + *(float *)((char *)v6 + *(unsigned __int8 *)(v8 + 9)) * *(float *)(v8 - 4)
           + *(float *)((char *)v6 + *(unsigned __int8 *)(v8 + 10)) * *(float *)v8;
        if ( (v9 < *(float *)(v8 + 4)) | __UNORDERED__(v9, *(float *)(v8 + 4)) )
          break;
        ++v7;
        v8 += 20;
        if ( v7 >= a1 )
          goto LABEL_7;
      }
    }
  }
}


/* ---- R_AddAABBTreeSurfaces_r  0x004E3210 ----  VERIFIED */
void __cdecl R_AddAABBTreeSurfaces_r(int a1, int a2, int a3, int a4, int a5)
{
  int v5;
  int v6;
  int v7;
  int v8;
  double v9;
  double v10;
  double v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  double v20;
  int v21;
  int v22;
  int v23;
  _DWORD *v24;
  int v25;
  int v26;
  int v27;
  int v28;
  int v29;
  int v30;
  int v31;
  int v32;
  int v33;
  int v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  int v40;

  v5 = a5;
  v6 = a3;
  if ( a5 >= dpvs_cullPlaneLimit || (v7 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, a3 <= 0) )
  {
LABEL_6:
    v10 = *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 18) + a1) * *(float *)(dpvs_activeNearPlane + 8)
        + *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 17) + a1) * *(float *)(dpvs_activeNearPlane + 4)
        + *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 16) + a1) * *(float *)dpvs_activeNearPlane;
    if ( (v10 < *(float *)(dpvs_activeNearPlane + 12)) | __UNORDERED__(v10, *(float *)(dpvs_activeNearPlane + 12)) )
      return;
    if ( dpvs_activeFarPlane )
    {
      v11 = *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 18) + a1) * *(float *)(dpvs_activeFarPlane + 8)
          + *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 17) + a1) * *(float *)(dpvs_activeFarPlane + 4)
          + *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 16) + a1) * *(float *)dpvs_activeFarPlane;
      if ( (v11 < *(float *)(dpvs_activeFarPlane + 12)) | __UNORDERED__(v11, *(float *)(dpvs_activeFarPlane + 12)) )
        return;
    }
    v12 = 0;
    if ( dpvs_occluderCount > 0 )
    {
      while ( 1 )
      {
        v13 = *(_DWORD *)(dpvs_occluders + 4 * v12);
        if ( v5 < *(_DWORD *)(v13 + 24) )
          break;
LABEL_16:
        if ( ++v12 >= dpvs_occluderCount )
          goto LABEL_17;
      }
      *(_DWORD *)(v13 + 24) = 0x7FFFFFFF;
      v14 = *(_DWORD *)(dpvs_occluders + 4 * v12);
      v15 = 0;
      v16 = 0;
      while ( v15 < *(_DWORD *)(v14 + 28) )
      {
        v17 = *(_DWORD *)(v14 + 32);
        v18 = *(unsigned __int8 *)(v17 + v16 + 18);
        v19 = v16 + v17;
        v20 = *(float *)(a1 + v18) * *(float *)(v19 + 8)
            + *(float *)(a1 + *(unsigned __int8 *)(v19 + 17)) * *(float *)(v19 + 4)
            + *(float *)(a1 + *(unsigned __int8 *)(v19 + 16)) * *(float *)v19;
        if ( !((v20 < *(float *)(v19 + 12)) | __UNORDERED__(v20, *(float *)(v19 + 12))) )
        {
          v5 = a5;
          v6 = a3;
          goto LABEL_16;
        }
        ++v15;
        v16 += 20;
      }
      return;
    }
LABEL_17:
    if ( a4 )
      a4 = R_CullDlightsForBox(a4, (float *)(a1 + 12), (float *)a1);
    if ( v5 < dpvs_cullPlaneLimit )
    {
      v21 = 0;
      if ( v6 > 0 )
      {
        v22 = a2 + 8;
        while ( *(float *)(a1 - *(unsigned __int8 *)(v22 + 9) + 20) * *(float *)(v22 - 4)
              + *(float *)(a1 - *(unsigned __int8 *)(v22 + 8) + 12) * *(float *)(v22 - 8)
              + *(float *)(a1 - *(unsigned __int8 *)(v22 + 10) + 28) * *(float *)v22 > *(float *)(v22 + 4) )
        {
          ++v21;
          v22 += 20;
          if ( v21 >= v6 )
            goto LABEL_24;
        }
LABEL_41:
        v33 = *(_DWORD *)(a1 + 36);
        if ( v33 )
        {
          v34 = 0;
          if ( v33 > 0 )
          {
            v35 = v5 + 1;
            v36 = 0;
            do
            {
              R_AddAABBTreeSurfaces_r(v36 + *(_DWORD *)(a1 + 32), a2, a3, a4, v35);
              ++v34;
              v36 += 40;
            }
            while ( v34 < *(_DWORD *)(a1 + 36) );
          }
        }
        else
        {
          if ( r_showaabbtrees->integer )
            R_AddDebugBox(clippedLeafDebugColor, (float *)(a1 + 12), (float *)a1);
          v37 = 0;
          if ( *(int *)(a1 + 28) > 0 )
          {
            v38 = v5 + 1;
            v39 = 0;
            do
            {
              R_AddTrianglesSurface(a3, (_DWORD *)(v39 + *(_DWORD *)(a1 + 24)), a4, a2, v38);
              ++v37;
              v39 += 12;
            }
            while ( v37 < *(_DWORD *)(a1 + 28) );
          }
        }
        return;
      }
LABEL_24:
      dpvs_cullPlaneLimit = v5 + 1;
    }
    v23 = 0;
    v40 = 0;
    if ( dpvs_occluderCount <= 0 )
    {
LABEL_33:
      if ( r_showaabbtrees->integer )
        R_AddDebugBox(dword_541870, (float *)(a1 + 12), (float *)a1);
      v30 = 0;
      if ( *(int *)(a1 + 28) > 0 )
      {
        v31 = 0;
        do
        {
          v32 = *(_DWORD *)(a1 + 24);
          if ( *(_DWORD *)(v31 + v32) != tr_viewCount )
            R_AddWorldSurfaceNoCull(a4, (_DWORD *)(v31 + v32));
          ++v30;
          v31 += 12;
        }
        while ( v30 < *(_DWORD *)(a1 + 28) );
      }
      return;
    }
    while ( 1 )
    {
      v24 = *(_DWORD **)(dpvs_occluders + 4 * v23);
      if ( v5 < v24[6] )
        break;
LABEL_32:
      v40 = ++v23;
      if ( v23 >= dpvs_occluderCount )
        goto LABEL_33;
    }
    v25 = 0;
    v26 = 0;
    while ( v25 < v24[7] )
    {
      v27 = v24[8];
      v28 = *(unsigned __int8 *)(v27 + v26 + 18);
      v29 = v26 + v27;
      v5 = a5;
      if ( *(float *)(a1 - v28 + 28) * *(float *)(v29 + 8)
         + *(float *)(a1 - *(unsigned __int8 *)(v29 + 17) + 20) * *(float *)(v29 + 4)
         + *(float *)(a1 - *(unsigned __int8 *)(v29 + 16) + 12) * *(float *)v29 > *(float *)(v29 + 12) )
      {
        v24[6] = a5 + 1;
        v23 = v40;
        goto LABEL_32;
      }
      ++v25;
      v26 += 20;
    }
    goto LABEL_41;
  }
  v8 = a2 + 8;
  while ( 1 )
  {
    v9 = *(float *)(*(unsigned __int8 *)(v8 + 8) + a1) * *(float *)(v8 - 8)
       + *(float *)(*(unsigned __int8 *)(v8 + 9) + a1) * *(float *)(v8 - 4)
       + *(float *)(*(unsigned __int8 *)(v8 + 10) + a1) * *(float *)v8;
    if ( (v9 < *(float *)(v8 + 4)) | __UNORDERED__(v9, *(float *)(v8 + 4)) )
      break;
    ++v7;
    v8 += 20;
    if ( v7 >= a3 )
      goto LABEL_6;
  }
}


/* ---- R_CullModels  0x004E3560 ----  [HIGH] */
void __cdecl R_CullModels(int a1, int a2, int a3)
{
  _DWORD *v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  double v9;
  double v10;
  double v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  double v20;

  v3 = *(_DWORD **)(a1 + 60);
  v4 = a3;
  if ( v3 )
  {
    v5 = dpvs_activeNearPlane;
    v6 = dpvs_activeFarPlane;
    do
    {
      if ( *(_DWORD *)(*v3 + 692) )
      {
        if ( dpvs_cullPlaneLimit <= 0 || (v7 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, v4 <= 0) )
        {
LABEL_9:
          v10 = *(float *)((char *)v3 + *(unsigned __int8 *)(v5 + 18) + 4) * *(float *)(v5 + 8)
              + *(float *)((char *)v3 + *(unsigned __int8 *)(v5 + 17) + 4) * *(float *)(v5 + 4)
              + *(float *)((char *)v3 + *(unsigned __int8 *)(v5 + 16) + 4) * *(float *)v5;
          if ( !((v10 < *(float *)(v5 + 12)) | __UNORDERED__(v10, *(float *)(v5 + 12))) )
          {
            if ( !v6
              || (v11 = *(float *)((char *)v3 + *(unsigned __int8 *)(v6 + 18) + 4) * *(float *)(v6 + 8)
                      + *(float *)((char *)v3 + *(unsigned __int8 *)(v6 + 17) + 4) * *(float *)(v6 + 4)
                      + *(float *)((char *)v3 + *(unsigned __int8 *)(v6 + 16) + 4) * *(float *)v6,
                  !((v11 < *(float *)(v6 + 12)) | __UNORDERED__(v11, *(float *)(v6 + 12)))) )
            {
              v12 = 0;
              if ( dpvs_occluderCount <= 0 )
              {
LABEL_20:
                *(_DWORD *)(*v3 + 692) = 0;
              }
              else
              {
                while ( 1 )
                {
                  v13 = *(_DWORD *)(dpvs_occluders + 4 * v12);
                  if ( *(int *)(v13 + 24) > 0 )
                    break;
LABEL_18:
                  if ( ++v12 >= dpvs_occluderCount )
                  {
                    v4 = a3;
                    goto LABEL_20;
                  }
                }
                *(_DWORD *)(v13 + 24) = 0x7FFFFFFF;
                v14 = *(_DWORD *)(dpvs_occluders + 4 * v12);
                v15 = 0;
                v16 = 0;
                while ( v15 < *(_DWORD *)(v14 + 28) )
                {
                  v17 = *(_DWORD *)(v14 + 32);
                  v18 = *(unsigned __int8 *)(v17 + v16 + 18);
                  v19 = v16 + v17;
                  v20 = *(float *)((char *)v3 + v18 + 4) * *(float *)(v19 + 8)
                      + *(float *)((char *)v3 + *(unsigned __int8 *)(v19 + 17) + 4) * *(float *)(v19 + 4)
                      + *(float *)((char *)v3 + *(unsigned __int8 *)(v19 + 16) + 4) * *(float *)v19;
                  if ( !((v20 < *(float *)(v19 + 12)) | __UNORDERED__(v20, *(float *)(v19 + 12))) )
                    goto LABEL_18;
                  ++v15;
                  v16 += 20;
                }
                v4 = a3;
              }
              v6 = dpvs_activeFarPlane;
              v5 = dpvs_activeNearPlane;
            }
          }
        }
        else
        {
          v8 = a2 + 8;
          while ( 1 )
          {
            v9 = *(float *)((char *)v3 + *(unsigned __int8 *)(v8 + 8) + 4) * *(float *)(v8 - 8)
               + *(float *)((char *)v3 + *(unsigned __int8 *)(v8 + 9) + 4) * *(float *)(v8 - 4)
               + *(float *)((char *)v3 + *(unsigned __int8 *)(v8 + 10) + 4) * *(float *)v8;
            if ( (v9 < *(float *)(v8 + 4)) | __UNORDERED__(v9, *(float *)(v8 + 4)) )
              break;
            ++v7;
            v8 += 20;
            if ( v7 >= v4 )
              goto LABEL_9;
          }
        }
      }
      v3 = (_DWORD *)v3[7];
    }
    while ( v3 );
  }
}

/* ---- R_XModelWorldBounds  0x004E3700 ----  [HIGH] */
int __cdecl R_XModelWorldBounds(float *a1, int a2, float *a3)
{
  double v3;
  double v4;
  long double v5;
  int v6;
  double v7;
  double v8;
  int result;
  float v10;
  float v11;
  float v12;
  float v13;
  float v14;
  float v15;

  v3 = *(float *)(a2 + 28) * *(float *)(a2 + 28)
     + *(float *)(a2 + 32) * *(float *)(a2 + 32)
     + *(float *)(a2 + 36) * *(float *)(a2 + 36);
  v10 = v3;
  v4 = v3;
  v12 = *(float *)(a2 + 40) * *(float *)(a2 + 40)
      + *(float *)(a2 + 44) * *(float *)(a2 + 44)
      + *(float *)(a2 + 48) * *(float *)(a2 + 48);
  v14 = *(float *)(a2 + 52) * *(float *)(a2 + 52)
      + *(float *)(a2 + 56) * *(float *)(a2 + 56)
      + *(float *)(a2 + 60) * *(float *)(a2 + 60);
  if ( (v10 < (double)v12) | __UNORDERED__(v10, v12) )
    v4 = v12;
  if ( (v4 < v14) | __UNORDERED__(v4, v14) )
    v4 = v14;
  v5 = sqrt(v4);
  v6 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(a2 + 144) + 24) + 4);
  v15 = *(float *)(v6 + 84);
  v11 = *(float *)(v6 + 88);
  v13 = *(float *)(v6 + 92);
  v7 = *(float *)(v6 + 76);
  v8 = *(float *)(v6 + 80);
  result = *(int *)(v6 + 96);
  *a3 = v7 * v5 + *(float *)(a2 + 68);
  a3[1] = v8 * v5 + *(float *)(a2 + 72);
  a3[2] = v15 * v5 + *(float *)(a2 + 76);
  *a1 = v11 * v5 + *(float *)(a2 + 68);
  a1[1] = v13 * v5 + *(float *)(a2 + 72);
  a1[2] = *(float *)&result * v5 + *(float *)(a2 + 76);
  return result;
}

/* ---- R_BModelWorldBounds  0x004E3810 ----  [HIGH] */
void __cdecl R_BModelWorldBounds(int a1, int a2, float *a3)
{
  float *v3;
  int v5;
  int v6;
  double v7;
  float v8;
  float v9;
  double v10;
  float v11[3]; // [esp+Ch] [ebp-18h] BYREF  -- retail vec3_t
  float v14;
  float v15;
  float v16;

  v3 = a3;
  v5 = *(_DWORD *)(tr_models[*(_DWORD *)(a2 + 8)] + 76);
  a3[2] = 262144.0;
  a3[1] = 262144.0;
  *a3 = 262144.0;
  *(_DWORD *)(a1 + 8) = -931135488;
  *(_DWORD *)(a1 + 4) = -931135488;
  *(_DWORD *)a1 = -931135488;
  v6 = 0;
  while ( 1 )
  {
    v14 = *(float *)(v5 + 12 * (v6 & 1));
    v7 = v14 * *(float *)(a2 + 28);
    v15 = *(float *)(v5 + 12 * ((v6 >> 1) & 1) + 4);
    v8 = *(float *)(v5 + 12 * ((v6 >> 2) & 1) + 8);
    v11[0] = *(float *)(a2 + 68);
    v9 = *(float *)(a2 + 76);
    v16 = v8;
    v11[0] = v7 + v11[0];
    v11[1] = *(float *)(a2 + 72);
    v10 = v14 * *(float *)(a2 + 32);
    v11[2] = v9;
    v11[1] = v10 + v11[1];
    v11[2] = v14 * *(float *)(a2 + 36) + v9;
    v11[0] = v15 * *(float *)(a2 + 40) + v11[0];
    v11[1] = v15 * *(float *)(a2 + 44) + v11[1];
    v11[2] = v15 * *(float *)(a2 + 48) + v11[2];
    v11[0] = v8 * *(float *)(a2 + 52) + v11[0];
    v11[1] = v8 * *(float *)(a2 + 56) + v11[1];
    v11[2] = v8 * *(float *)(a2 + 60) + v11[2];
    AddPointToBounds(v11, v3, (float *)a1);
    if ( ++v6 >= 8 )
      break;
    v3 = a3;
  }
}

/* ---- R_AddModelToCell  0x004E3940 ----  VERIFIED */
void __cdecl R_AddModelToCell(int a1, int a2, int a3, int a4)
{
  int v5;
  _DWORD *v6;

  if ( dpvs_cellEntityLinkCount == 4096 )
  {
    ri_Printf(1, "^1Max xmodel refs (%i) exceeded\n", 4096);
  }
  else
  {
    v5 = *(_DWORD *)(a3 + 60);
    if ( v5 )
    {
      while ( *(_DWORD *)v5 != a4 )
      {
        v5 = *(_DWORD *)(v5 + 28);
        if ( !v5 )
          goto LABEL_6;
      }
      ExpandBounds((float *)a2, (float *)(v5 + 4), (float *)a1, (float *)(v5 + 16));
    }
    else
    {
LABEL_6:
      v6 = (_DWORD *)(dpvs_cellEntityLinks + 32 * dpvs_cellEntityLinkCount++);
      *v6 = a4;
      v6[1] = *(_DWORD *)a2;
      v6[2] = *(_DWORD *)(a2 + 4);
      v6[3] = *(_DWORD *)(a2 + 8);
      v6[4] = *(_DWORD *)a1;
      v6[5] = *(_DWORD *)(a1 + 4);
      v6[6] = *(_DWORD *)(a1 + 8);
      v6[7] = *(_DWORD *)(a3 + 60);
      *(_DWORD *)(a3 + 60) = v6;
    }
  }
}

/* ---- R_FilterModelIntoCells_r  0x004E39D0 ----  VERIFIED */
int __cdecl R_FilterModelIntoCells_r(_DWORD *a1, int a2, _DWORD *a3, _DWORD *a4)
{
  _DWORD *v4;
  int v5;
  int v6;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  _DWORD v16[3]; // [esp+10h] [ebp-18h] BYREF
  _DWORD v17[3]; // [esp+1Ch] [ebp-Ch] BYREF

  v4 = a1;
  if ( a1[2] == -2 )
  {
    while ( 1 )
    {
      v5 = BoxOnPlaneSide((int)a3, (int)a4, v4[3]);
      if ( v5 == 3 )
        break;
      v4 = (_DWORD *)v4[v5 + 3];
      if ( v4[2] != -2 )
        goto LABEL_4;
    }
    v8 = v4[3];
    if ( *(_BYTE *)(v8 + 16) >= 3u )
    {
      v15 = R_FilterModelIntoCells_r((_DWORD *)v4[4], a2, a3, a4);
      v13 = R_FilterModelIntoCells_r((_DWORD *)v4[5], a2, a3, a4);
    }
    else
    {
      v9 = a3[1];
      v16[0] = *a3;
      v10 = a3[2];
      v16[1] = v9;
      v16[2] = v10;
      v16[*(unsigned __int8 *)(v8 + 16)] = *(_DWORD *)(v8 + 12);
      v11 = a4[1];
      v17[0] = *a4;
      v12 = a4[2];
      v17[1] = v11;
      v17[2] = v12;
      v17[*(unsigned __int8 *)(v8 + 16)] = *(_DWORD *)(v8 + 12);
      v13 = R_FilterModelIntoCells_r((_DWORD *)v4[5], a2, a3, v17);
      v14 = *(unsigned __int8 *)(v4[3] + 16);
      v15 = v13;
      if ( !((*(float *)&v16[v14] < (double)*(float *)&a4[v14]) | __UNORDERED__(*(float *)&v16[v14], *(float *)&a4[v14])) )
        return v15;
      v15 = R_FilterModelIntoCells_r((_DWORD *)v4[4], a2, v16, a4);
    }
    if ( v15 == v13 )
      return v15;
    return -2;
  }
  else
  {
LABEL_4:
    v6 = v4[2];
    if ( v6 >= 0 )
      R_AddModelToCell((int)a4, (int)a3, *(_DWORD *)(tr_world + 292) + (v6 << 6), a2);
    return v4[2];
  }
}

/* ---- R_XModelIsHuge  0x004E3B20 ----  VERIFIED */
BOOL __fastcall R_XModelIsHuge(int a1, int a2)
{
  int dxBig;
  int dyBig;

  dxBig = *(float *)a1 - *(float *)a2 > 1536.0;
  dyBig = *(float *)(a1 + 4) - *(float *)(a2 + 4) > 1536.0;

  if ( dxBig && dyBig )
    return 1;
  if ( !dxBig && !dyBig )
    return 0;
  return *(float *)(a1 + 8) - *(float *)(a2 + 8) > 1536.0;
}

/* ---- R_FilterModelsIntoCells  0x004E3B70 ----  VERIFIED */
int __cdecl R_FilterModelsIntoCells(int a1, int a2)
{
  int v2;
  int v3;
  int result;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  double v11;
  double v12;
  double v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;
  double v22;
  int v23_xBig;
  int v25_yBig;
  cvar_t *v28;
  bool v29; // zf
  int v30;
  int v31;
  double v32;
  double v33;
  double v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  int v40;
  int v41;
  double v42;
  bool v43; // cc
  int v44;
  int v45;
  int v46;
  int v47;
  float v48[6]; /* retail: mins[3] [esp+18h] then maxs[3] [esp+24h] */
  float *const v49 = v48 + 3;

  v2 = 0;
  if ( !r_cullBModels->integer || (v46 = 1, !r_drawBModels->integer) )
    v46 = 0;
  if ( !r_cullXModels->integer || (v45 = 1, !r_drawXModels->integer) )
    v45 = 0;
  v3 = tr_world;
  result = 0;
  if ( *(int *)(tr_world + 288) > 0 )
  {
    v5 = 0;
    do
    {
      *(_DWORD *)(*(_DWORD *)(v3 + 292) + v5 + 60) = 0;
      v3 = tr_world;
      ++result;
      v5 += 64;
    }
    while ( result < *(_DWORD *)(tr_world + 288) );
  }
  dpvs_cellEntityLinkCount = 0;
  v47 = 0;
  if ( tr_refdef_num_entities > 0 )
  {
    v6 = 0;
    v44 = 0;
    while ( 1 )
    {
      *(_DWORD *)(v6 + tr_refdef_entities + 692) = 0;
      v7 = *(_DWORD *)(v6 + tr_refdef_entities);
      if ( v7 )
      {
        if ( v7 == 1 && v45 )
        {
          *(_DWORD *)(v6 + tr_refdef_entities + 692) = 2;
          v8 = tr_refdef_entities;
          R_XModelWorldBounds(v49, v6 + tr_refdef_entities, v48);
          if ( dpvs_cullPlaneLimit <= 0 || (v9 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, a2 <= 0) )
          {
LABEL_21:
            v12 = *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeNearPlane + 18)) * *(float *)(dpvs_activeNearPlane + 8)
                + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeNearPlane + 17)) * *(float *)(dpvs_activeNearPlane + 4)
                + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeNearPlane + 16)) * *(float *)dpvs_activeNearPlane;
            if ( !((v12 < *(float *)(dpvs_activeNearPlane + 12)) | __UNORDERED__(v12, *(float *)(dpvs_activeNearPlane + 12))) )
            {
              if ( !dpvs_activeFarPlane
                || (v13 = *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeFarPlane + 18))
                        * *(float *)(dpvs_activeFarPlane + 8)
                        + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeFarPlane + 17))
                        * *(float *)(dpvs_activeFarPlane + 4)
                        + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeFarPlane + 16)) * *(float *)dpvs_activeFarPlane,
                    !((v13 < *(float *)(dpvs_activeFarPlane + 12)) | __UNORDERED__(v13, *(float *)(dpvs_activeFarPlane + 12)))) )
              {
                v14 = 0;
                if ( dpvs_occluderCount > 0 )
                {
                  while ( 1 )
                  {
                    v15 = *(_DWORD *)(dpvs_occluders + 4 * v14);
                    if ( *(int *)(v15 + 24) > 0 )
                      break;
LABEL_30:
                    if ( ++v14 >= dpvs_occluderCount )
                    {
                      v6 = v44;
                      v8 = tr_refdef_entities;
                      goto LABEL_32;
                    }
                  }
                  *(_DWORD *)(v15 + 24) = 0x7FFFFFFF;
                  v16 = *(_DWORD *)(dpvs_occluders + 4 * v14);
                  v17 = 0;
                  v18 = 0;
                  while ( v17 < *(_DWORD *)(v16 + 28) )
                  {
                    v19 = *(_DWORD *)(v16 + 32);
                    v20 = *(unsigned __int8 *)(v19 + v18 + 18);
                    v21 = v18 + v19;
                    v22 = *(float *)((char *)v48 + v20) * *(float *)(v21 + 8)
                        + *(float *)((char *)v48 + *(unsigned __int8 *)(v21 + 17)) * *(float *)(v21 + 4)
                        + *(float *)((char *)v48 + *(unsigned __int8 *)(v21 + 16)) * *(float *)v21;
                    if ( !((v22 < *(float *)(v21 + 12)) | __UNORDERED__(v22, *(float *)(v21 + 12))) )
                      goto LABEL_30;
                    ++v17;
                    v18 += 20;
                  }
LABEL_63:
                  v6 = v44;
                  goto LABEL_64;
                }
LABEL_32:
                v23_xBig = v49[0] - v48[0] > 1536.0;
                v25_yBig = v49[1] - v48[1] > 1536.0;
                if ( !v23_xBig )
                {
                  if ( v25_yBig )
                    goto LABEL_36;
                }
                else
                {
                  if ( v25_yBig )
                    goto LABEL_37;
LABEL_36:
                  if ( v49[2] - v48[2] > 1536.0 )
                  {
LABEL_37:
                    v28 = r_showCullXModels;
                    *(_DWORD *)(v6 + v8 + 692) = 0;
                    v29 = v28->integer == 1;
LABEL_61:
                    if ( v29 )
                      R_AddDebugBox(modelBoundsDebugColor, v49, v48);
                    goto LABEL_64;
                  }
                }
                if ( R_FilterModelIntoCells_r(*(_DWORD **)(tr_world + 152), v6 + v8, v48, v49) == -1 )
                  *(_DWORD *)(v6 + tr_refdef_entities + 692) = 0;
                v29 = r_showCullXModels->integer == 1;
                goto LABEL_61;
              }
            }
          }
          else
          {
            v10 = a1 + 8;
            while ( 1 )
            {
              v11 = *(float *)((char *)v48 + *(unsigned __int8 *)(v10 + 8)) * *(float *)(v10 - 8)
                  + *(float *)((char *)v48 + *(unsigned __int8 *)(v10 + 9)) * *(float *)(v10 - 4)
                  + *(float *)((char *)v48 + *(unsigned __int8 *)(v10 + 10)) * *(float *)v10;
              if ( (v11 < *(float *)(v10 + 4)) | __UNORDERED__(v11, *(float *)(v10 + 4)) )
                break;
              ++v9;
              v10 += 20;
              if ( v9 >= a2 )
                goto LABEL_21;
            }
          }
        }
      }
      else
      {
        if ( !v46 )
          goto LABEL_64;
        *(_DWORD *)(v6 + tr_refdef_entities + 692) = 2;
        R_BModelWorldBounds((int)v49, v6 + tr_refdef_entities, v48);
        if ( dpvs_cullPlaneLimit <= 0 || (v30 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, a2 <= 0) )
        {
LABEL_47:
          v33 = *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeNearPlane + 18)) * *(float *)(dpvs_activeNearPlane + 8)
              + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeNearPlane + 17)) * *(float *)(dpvs_activeNearPlane + 4)
              + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeNearPlane + 16)) * *(float *)dpvs_activeNearPlane;
          if ( !((v33 < *(float *)(dpvs_activeNearPlane + 12)) | __UNORDERED__(v33, *(float *)(dpvs_activeNearPlane + 12))) )
          {
            if ( !dpvs_activeFarPlane
              || (v34 = *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeFarPlane + 18))
                      * *(float *)(dpvs_activeFarPlane + 8)
                      + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeFarPlane + 17))
                      * *(float *)(dpvs_activeFarPlane + 4)
                      + *(float *)((char *)v48 + *(unsigned __int8 *)(dpvs_activeFarPlane + 16)) * *(float *)dpvs_activeFarPlane,
                  !((v34 < *(float *)(dpvs_activeFarPlane + 12)) | __UNORDERED__(v34, *(float *)(dpvs_activeFarPlane + 12)))) )
            {
              v35 = 0;
              if ( dpvs_occluderCount > 0 )
              {
                while ( 1 )
                {
                  v36 = *(_DWORD *)(dpvs_occluders + 4 * v35);
                  if ( *(int *)(v36 + 24) > 0 )
                    break;
LABEL_56:
                  ++v35;
                  v2 = 0;
                  if ( v35 >= dpvs_occluderCount )
                  {
                    v6 = v44;
                    goto LABEL_58;
                  }
                }
                *(_DWORD *)(v36 + 24) = 0x7FFFFFFF;
                v37 = *(_DWORD *)(dpvs_occluders + 4 * v35);
                v38 = 0;
                while ( v2 < *(_DWORD *)(v37 + 28) )
                {
                  v39 = *(_DWORD *)(v37 + 32);
                  v40 = *(unsigned __int8 *)(v39 + v38 + 18);
                  v41 = v38 + v39;
                  v42 = *(float *)((char *)v48 + v40) * *(float *)(v41 + 8)
                      + *(float *)((char *)v48 + *(unsigned __int8 *)(v41 + 17)) * *(float *)(v41 + 4)
                      + *(float *)((char *)v48 + *(unsigned __int8 *)(v41 + 16)) * *(float *)v41;
                  if ( !((v42 < *(float *)(v41 + 12)) | __UNORDERED__(v42, *(float *)(v41 + 12))) )
                    goto LABEL_56;
                  ++v2;
                  v38 += 20;
                }
                goto LABEL_63;
              }
LABEL_58:
              if ( R_FilterModelIntoCells_r(*(_DWORD **)(tr_world + 152), v6 + tr_refdef_entities, v48, v49) == -1 )
                *(_DWORD *)(v6 + tr_refdef_entities + 692) = 0;
              v29 = r_showCullBModels->integer == 1;
              goto LABEL_61;
            }
          }
        }
        else
        {
          v31 = a1 + 8;
          while ( 1 )
          {
            v32 = *(float *)((char *)v48 + *(unsigned __int8 *)(v31 + 8)) * *(float *)(v31 - 8)
                + *(float *)((char *)v48 + *(unsigned __int8 *)(v31 + 9)) * *(float *)(v31 - 4)
                + *(float *)((char *)v48 + *(unsigned __int8 *)(v31 + 10)) * *(float *)v31;
            if ( (v32 < *(float *)(v31 + 4)) | __UNORDERED__(v32, *(float *)(v31 + 4)) )
              break;
            ++v30;
            v31 += 20;
            if ( v30 >= a2 )
              goto LABEL_47;
          }
        }
      }
LABEL_64:
      result = v47 + 1;
      v6 += 696;
      v43 = ++v47 < tr_refdef_num_entities;
      v44 = v6;
      if ( !v43 )
        return result;
      v2 = 0;
    }
  }
  return result;
}


/* ---- R_AddCellSurfaces  0x004E4080 ----  VERIFIED */
void __cdecl R_AddCellSurfaces(int a1, int a2, int a3, int a4)
{
  _DWORD *v4;
  int v5;
  int v6;
  int v7;
  int v8;

  dpvs_cullPlaneLimit = 0x7FFFFFFF;
  v4 = *(_DWORD **)(a1 + 24);
  v5 = v4[9];
  v6 = 0;
  if ( v5 )
  {
    if ( v5 > 0 )
    {
      v7 = 0;
      do
      {
        R_AddAABBTreeSurfaces_r(v7 + v4[8], a2, a3, a4, 0);
        v4 = *(_DWORD **)(a1 + 24);
        ++v6;
        v7 += 40;
      }
      while ( v6 < v4[9] );
    }
  }
  else if ( (int)v4[7] > 0 )
  {
    v8 = 0;
    do
    {
      R_AddTrianglesSurface(a3, (_DWORD *)(v8 + v4[6]), a4, a2, 0);
      v4 = *(_DWORD **)(a1 + 24);
      ++v6;
      v8 += 12;
    }
    while ( v6 < v4[7] );
  }
  if ( r_drawentities->integer )
    R_CullModels(a1, a2, a3);
}


/* ---- R_AddCullGroupDPVS  0x004E4120 ----  VERIFIED */
void __cdecl R_AddCullGroupDPVS(int a1, int a2, int a3, BOOL a4)
{
  int v5;
  int v6;
  double v7;
  double v8;
  double v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  double v18;
  _DWORD *v19;
  int i;

  if ( dpvs_cullPlaneLimit <= 0 || (v5 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, a1 <= 0) )
  {
LABEL_6:
    v8 = *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 18) + a2) * *(float *)(dpvs_activeNearPlane + 8)
       + *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 17) + a2) * *(float *)(dpvs_activeNearPlane + 4)
       + *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 16) + a2) * *(float *)dpvs_activeNearPlane;
    if ( !((v8 < *(float *)(dpvs_activeNearPlane + 12)) | __UNORDERED__(v8, *(float *)(dpvs_activeNearPlane + 12))) )
    {
      if ( !dpvs_activeFarPlane
        || (v9 = *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 18) + a2) * *(float *)(dpvs_activeFarPlane + 8)
               + *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 17) + a2) * *(float *)(dpvs_activeFarPlane + 4)
               + *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 16) + a2) * *(float *)dpvs_activeFarPlane,
            !((v9 < *(float *)(dpvs_activeFarPlane + 12)) | __UNORDERED__(v9, *(float *)(dpvs_activeFarPlane + 12)))) )
      {
        v10 = 0;
        if ( dpvs_occluderCount <= 0 )
        {
LABEL_16:
          if ( (r_showportals->integer & 1) != 0 )
            R_AddDebugBox(cullGroupDebugColor, (float *)(a2 + 12), (float *)a2);
          v19 = *(_DWORD **)(a2 + 24);
          *(_DWORD *)(a2 + 32) = tr_viewCount;
          for ( i = *(_DWORD *)(a2 + 28); i; --i )
          {
            R_AddWorldSurfaceNoCull(a4, v19);
            v19 += 3;
          }
        }
        else
        {
          while ( 1 )
          {
            v11 = *(_DWORD *)(dpvs_occluders + 4 * v10);
            if ( *(int *)(v11 + 24) > 0 )
              break;
LABEL_15:
            if ( ++v10 >= dpvs_occluderCount )
              goto LABEL_16;
          }
          *(_DWORD *)(v11 + 24) = 0x7FFFFFFF;
          v12 = *(_DWORD *)(dpvs_occluders + 4 * v10);
          v13 = 0;
          v14 = 0;
          while ( v13 < *(_DWORD *)(v12 + 28) )
          {
            v15 = *(_DWORD *)(v12 + 32);
            v16 = *(unsigned __int8 *)(v15 + v14 + 18);
            v17 = v14 + v15;
            v18 = *(float *)(a2 + v16) * *(float *)(v17 + 8)
                + *(float *)(a2 + *(unsigned __int8 *)(v17 + 17)) * *(float *)(v17 + 4)
                + *(float *)(a2 + *(unsigned __int8 *)(v17 + 16)) * *(float *)v17;
            if ( !((v18 < *(float *)(v17 + 12)) | __UNORDERED__(v18, *(float *)(v17 + 12))) )
              goto LABEL_15;
            ++v13;
            v14 += 20;
          }
        }
      }
    }
  }
  else
  {
    v6 = a3 + 8;
    while ( 1 )
    {
      v7 = *(float *)(*(unsigned __int8 *)(v6 + 8) + a2) * *(float *)(v6 - 8)
         + *(float *)(*(unsigned __int8 *)(v6 + 9) + a2) * *(float *)(v6 - 4)
         + *(float *)(*(unsigned __int8 *)(v6 + 10) + a2) * *(float *)v6;
      if ( (v7 < *(float *)(v6 + 4)) | __UNORDERED__(v7, *(float *)(v6 + 4)) )
        break;
      ++v5;
      v6 += 20;
      if ( v5 >= a1 )
        goto LABEL_6;
    }
  }
}


/* ---- R_AddStaticModelDPVS  0x004E42B0 ----  VERIFIED */
void __cdecl R_AddStaticModelDPVS(int a1, int a2, int a3)
{
  int v3;
  float *v4;
  int v5;
  int v6;
  double v7;
  double v8;
  double v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  double v18;

  v3 = a3;
  v4 = (float *)(a3 + 156);
  if ( dpvs_cullPlaneLimit <= 0 || (v5 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, a1 <= 0) )
  {
LABEL_6:
    v8 = *(float *)((char *)v4 + *(unsigned __int8 *)(dpvs_activeNearPlane + 18)) * *(float *)(dpvs_activeNearPlane + 8)
       + *(float *)((char *)v4 + *(unsigned __int8 *)(dpvs_activeNearPlane + 17)) * *(float *)(dpvs_activeNearPlane + 4)
       + *(float *)((char *)v4 + *(unsigned __int8 *)(dpvs_activeNearPlane + 16)) * *(float *)dpvs_activeNearPlane;
    if ( !((v8 < *(float *)(dpvs_activeNearPlane + 12)) | __UNORDERED__(v8, *(float *)(dpvs_activeNearPlane + 12))) )
    {
      if ( !dpvs_activeFarPlane
        || (v9 = *(float *)((char *)v4 + *(unsigned __int8 *)(dpvs_activeFarPlane + 18)) * *(float *)(dpvs_activeFarPlane + 8)
               + *(float *)((char *)v4 + *(unsigned __int8 *)(dpvs_activeFarPlane + 17)) * *(float *)(dpvs_activeFarPlane + 4)
               + *(float *)((char *)v4 + *(unsigned __int8 *)(dpvs_activeFarPlane + 16)) * *(float *)dpvs_activeFarPlane,
            !((v9 < *(float *)(dpvs_activeFarPlane + 12)) | __UNORDERED__(v9, *(float *)(dpvs_activeFarPlane + 12)))) )
      {
        v10 = 0;
        if ( dpvs_occluderCount <= 0 )
        {
LABEL_17:
          if ( r_showCullSModels->integer )
            R_AddDebugBox(staticModelDebugColor, (float *)(v3 + 168), v4);
          *(_DWORD *)(v3 + 180) = tr_viewCount;
          RE_AddRefEntityToScene((int *)v3, v3);
        }
        else
        {
          while ( 1 )
          {
            v11 = *(_DWORD *)(dpvs_occluders + 4 * v10);
            if ( *(int *)(v11 + 24) > 0 )
              break;
LABEL_16:
            if ( ++v10 >= dpvs_occluderCount )
              goto LABEL_17;
          }
          *(_DWORD *)(v11 + 24) = 0x7FFFFFFF;
          v12 = *(_DWORD *)(dpvs_occluders + 4 * v10);
          v13 = 0;
          v14 = 0;
          while ( v13 < *(_DWORD *)(v12 + 28) )
          {
            v15 = *(_DWORD *)(v12 + 32);
            v16 = *(unsigned __int8 *)(v15 + v14 + 18);
            v17 = v14 + v15;
            v18 = *(float *)((char *)v4 + v16) * *(float *)(v17 + 8)
                + *(float *)((char *)v4 + *(unsigned __int8 *)(v17 + 17)) * *(float *)(v17 + 4)
                + *(float *)((char *)v4 + *(unsigned __int8 *)(v17 + 16)) * *(float *)v17;
            if ( !((v18 < *(float *)(v17 + 12)) | __UNORDERED__(v18, *(float *)(v17 + 12))) )
            {
              v3 = a3;
              goto LABEL_16;
            }
            ++v13;
            v14 += 20;
          }
        }
      }
    }
  }
  else
  {
    v6 = a2 + 8;
    while ( 1 )
    {
      v7 = *(float *)((char *)v4 + *(unsigned __int8 *)(v6 + 8)) * *(float *)(v6 - 8)
         + *(float *)((char *)v4 + *(unsigned __int8 *)(v6 + 9)) * *(float *)(v6 - 4)
         + *(float *)((char *)v4 + *(unsigned __int8 *)(v6 + 10)) * *(float *)v6;
      if ( (v7 < *(float *)(v6 + 4)) | __UNORDERED__(v7, *(float *)(v6 + 4)) )
        break;
      ++v5;
      v6 += 20;
      if ( v5 >= a1 )
        goto LABEL_6;
    }
  }
}


/* ---- R_AddCellCullGroups  0x004E4440 ----  VERIFIED */
void __cdecl R_AddCellCullGroups(int a1, BOOL a2, int a3, int a4)
{
  int v4;
  int *v5;
  int *i;
  int v7;

  v4 = a3;
  v5 = *(int **)(a3 + 36);
  if ( *(_DWORD *)(a3 + 40) )
  {
    v7 = *(_DWORD *)(a3 + 40);
    do
    {
      if ( *(_DWORD *)(*v5 + 32) != tr_viewCount )
        R_AddCullGroupDPVS(a1, *v5, a4, a2);
      ++v5;
      --v7;
    }
    while ( v7 );
    v4 = a3;
  }
  if ( r_drawSModels->integer )
  {
    for ( i = *(int **)(v4 + 56); i; i = (int *)i[1] )
    {
      if ( *(_DWORD *)(*i + 180) != tr_viewCount )
        R_AddStaticModelDPVS(a1, a4, *i);
    }
  }
}


/* ---- R_RecursivePortalWalk  0x004E44D0 ----  VERIFIED */
void __cdecl R_RecursivePortalWalk(int a1, int a2, float *a3, int a4, int a5)
{
  int v5;
  int v6;
  float *v7;
  int v8;
  int v9;
  int v10; // [esp+10h] [ebp-4h] BYREF
  int v11;


  R_AddCellOccluders(a1, a3, a4);
  R_AddCellSurfaces(a1, (int)a3, a4, a5);
  R_AddCellCullGroups(a4, a5, a1, (int)a3);
  v5 = *(_DWORD *)(a1 + 28);
  if ( *(_DWORD *)(a1 + 32) )
  {
    v11 = *(_DWORD *)(a1 + 32);
    do
    {
      if ( !*(_DWORD *)(v5 + 32)
        && *(float *)&dword_11CC108 * *(float *)(v5 + 8)
         + *(float *)&dword_11CC104 * *(float *)(v5 + 4)
         + *(float *)&dpvs_viewOrigin * *(float *)v5 <= *(float *)(v5 + 12)
        && !R_PortalBehindAnyPlane(v5, a3, a4) )
      {
        v6 = 0;
        if ( dpvs_occluderCount <= 0 )
        {
LABEL_9:
          v7 = (float *)R_PortalClipPlanes(&v10, a4, v5, a2, (char *)a3);
          if ( v10 )
          {
            v9 = v10;
            v8 = *(_DWORD *)(v5 + 20);
            *(_DWORD *)(v5 + 32) = 1;
            R_RecursivePortalWalk(v8, v5, v7, v9, a5);
            ri_Hunk_FreeTempMemory(v7);
            *(_DWORD *)(v5 + 32) = 0;
          }
        }
        else
        {
          while ( !R_PortalBehindAllPlanes(
                     v5,
                     *(float **)(*(_DWORD *)(dpvs_occluders + 4 * v6) + 32),
                     *(_DWORD *)(*(_DWORD *)(dpvs_occluders + 4 * v6) + 28)) )
          {
            if ( ++v6 >= dpvs_occluderCount )
              goto LABEL_9;
          }
        }
      }
      v5 += 36;
      --v11;
    }
    while ( v11 );
  }
}

/* ---- R_SetupDPVS  0x004E4610 ----  VERIFIED */
void R_SetupDPVS()
{
  double FarPlaneDist;
  double v1;

  if ( dpvs_initialSetup || !r_lockpvs->integer )
  {
    dpvs_initialSetup = 0;
    R_FrustumClipPlanes();
    dpvs_viewOrigin = LODWORD(tr_viewParms_originX);
    dpvs_nearPlane = tr_viewParms_axis00;
    dword_11CC108 = LODWORD(tr_viewParms_originZ);
    dword_11CC104 = LODWORD(tr_viewParms_originY);
    dword_11CC164 = tr_viewParms_axis02;
    dword_11CC160 = tr_viewParms_axis01;
    byte_11CC16C = tr_viewParms_axis00 <= 0 ? 0 : 0xC;
    byte_11CC16D = tr_viewParms_axis01 <= 0 ? 4 : 16;
    byte_11CC16E = tr_viewParms_axis02 <= 0 ? 8 : 20;
    dpvs_activeNearPlane = (int)&tr_viewParms_portalPlane;
    flt_11CC168 = *(float *)&tr_viewParms_axis02 * tr_viewParms_originZ
                + *(float *)&tr_viewParms_axis01 * tr_viewParms_originY
                + *(float *)&tr_viewParms_axis00 * tr_viewParms_originX
                - 0.1
                - 0.001;
    if ( !tr_viewParms_isMirror )
      dpvs_activeNearPlane = (int)&dpvs_nearPlane;
    FarPlaneDist = RE_GetFarPlaneDist();
    if ( FarPlaneDist <= 0.0 )
    {
      dpvs_activeFarPlane = 0;
    }
    else
    {
      dpvs_activeFarPlane = (int)&dpvs_farPlane;
      dpvs_farPlane = *(float *)&tr_viewParms_axis00 * -1.0;
      flt_11CC174 = *(float *)&tr_viewParms_axis01 * -1.0;
      v1 = *(float *)&tr_viewParms_axis02 * -1.0;
      flt_11CC178 = v1;
      byte_11CC180 = SLODWORD(dpvs_farPlane) <= 0 ? 0 : 0xC;
      byte_11CC181 = SLODWORD(flt_11CC174) <= 0 ? 4 : 16;
      byte_11CC182 = SLODWORD(flt_11CC178) <= 0 ? 8 : 20;
      flt_11CC17C = v1 * tr_viewParms_originZ + flt_11CC174 * tr_viewParms_originY + dpvs_farPlane * tr_viewParms_originX - FarPlaneDist - 0.001;
    }
    if ( r_portalbevels->value > 0.0 )
      R_SetupTransformMatrix();
  }
}


/* ---- R_AddCoronas  0x004E4800 ----  [HIGH] */
int __cdecl R_AddCoronas(int a1, int a2)
{
  int result;
  bool v3; // cc
  int v4;
  int v5;
  float *v6;
  double v7;
  int v8;
  int v9;
  double v10;
  double v11;
  double v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  double v21;
  int v22;
  int v23;
  char v24;
  double v25;
  double v26;
  unsigned __int64 v27; // rax
  int v28;
  int v29;
  float v30[3]; // [esp+8h] [ebp-CCh] BYREF -- ONE vec3_t in retail
  int v33;
  float v34[6];
  int v35;
  int v36[40]; // [esp+34h] [ebp-A0h] BYREF
  unsigned int retaddr;

  v36[39] = retaddr ^ _security_cookie;
  result = 0;
  memset(v36, 0, 0x9Cu);
  v3 = *(int *)(tr_world + 196) <= 0;
  v36[0] = 4;
  v33 = 0;
  if ( !v3 )
  {
    v4 = dpvs_activeFarPlane;
    v5 = dpvs_activeNearPlane;
    v29 = 0;
    do
    {
      v6 = (float *)(*(_DWORD *)(tr_world + 192) + v29);
      v30[0] = tr_refdef_vieworgX - v6[1];
      v30[1] = tr_refdef_vieworgY - v6[2];
      v30[2] = tr_refdef_vieworgZ - v6[3];
      VectorNormalize(v30);
      if ( v30[2] < (double)v6[5] )
      {
        v7 = v6[4] * 1.7320508;
        v34[0] = v6[1] - v7;
        v34[1] = v6[2] - v7;
        v34[2] = v6[3] - v7;
        v34[3] = v7 + v6[1];
        v34[4] = v7 + v6[2];
        v34[5] = v7 + v6[3];
        if ( dpvs_cullPlaneLimit <= 0 || (v8 = 0, dpvs_cullPlaneLimit = 0x7FFFFFFF, a2 <= 0) )
        {
LABEL_9:
          v11 = *(float *)((char *)v34 + *(unsigned __int8 *)(v5 + 18)) * *(float *)(v5 + 8)
              + *(float *)((char *)v34 + *(unsigned __int8 *)(v5 + 17)) * *(float *)(v5 + 4)
              + *(float *)((char *)v34 + *(unsigned __int8 *)(v5 + 16)) * *(float *)v5;
          if ( !((v11 < *(float *)(v5 + 12)) | __UNORDERED__(v11, *(float *)(v5 + 12))) )
          {
            if ( !v4
              || (v12 = *(float *)((char *)v34 + *(unsigned __int8 *)(v4 + 18)) * *(float *)(v4 + 8)
                      + *(float *)((char *)v34 + *(unsigned __int8 *)(v4 + 17)) * *(float *)(v4 + 4)
                      + *(float *)((char *)v34 + *(unsigned __int8 *)(v4 + 16)) * *(float *)v4,
                  !((v12 < *(float *)(v4 + 12)) | __UNORDERED__(v12, *(float *)(v4 + 12)))) )
            {
              v13 = 0;
              if ( dpvs_occluderCount <= 0 )
              {
LABEL_19:
                v22 = v29 + *(_DWORD *)(tr_world + 192);
                v23 = *(_DWORD *)(*(_DWORD *)v22 + 68);
                v24 = *(_BYTE *)(v22 + 28);
                *(float *)&v36[17] = v30[0] * 16.0 + *(float *)(v22 + 4);
                LOBYTE(v36[27]) = v24;
                BYTE2(v36[27]) = *(_BYTE *)(v22 + 30);
                v36[26] = v23;
                v25 = v30[1] * 16.0 + *(float *)(v22 + 8);
                BYTE1(v36[27]) = *(_BYTE *)(v22 + 29);
                *(float *)&v36[18] = v25;
                *(float *)&v36[19] = v30[2] * 16.0 + *(float *)(v22 + 12);
                v36[31] = *(int *)(v22 + 16);
                v36[25] = v36[31];
                if ( v30[2] <= (double)*(float *)(v22 + 24) )
                {
                  LOBYTE(v27) = *(_BYTE *)(v22 + 31);
                }
                else
                {
                  v26 = *(float *)(v22 + 20) - v30[2];
                  v35 = *(unsigned __int8 *)(v22 + 31);
                  v27 = (unsigned __int64)(v26 * (double)v35 / (*(float *)(v22 + 20) - *(float *)(v22 + 24)));
                }
                HIBYTE(v36[27]) = v27;
                RE_AddRefEntityToScene(v36, 0);
              }
              else
              {
                while ( 1 )
                {
                  v14 = *(_DWORD *)(dpvs_occluders + 4 * v13);
                  if ( *(int *)(v14 + 24) > 0 )
                    break;
LABEL_18:
                  if ( ++v13 >= dpvs_occluderCount )
                    goto LABEL_19;
                }
                *(_DWORD *)(v14 + 24) = 0x7FFFFFFF;
                v15 = *(_DWORD *)(dpvs_occluders + 4 * v13);
                v16 = 0;
                v17 = 0;
                while ( v16 < *(_DWORD *)(v15 + 28) )
                {
                  v18 = *(_DWORD *)(v15 + 32);
                  v19 = *(unsigned __int8 *)(v18 + v17 + 18);
                  v20 = v17 + v18;
                  v21 = *(float *)((char *)v34 + v19) * *(float *)(v20 + 8)
                      + *(float *)((char *)v34 + *(unsigned __int8 *)(v20 + 17)) * *(float *)(v20 + 4)
                      + *(float *)((char *)v34 + *(unsigned __int8 *)(v20 + 16)) * *(float *)v20;
                  if ( !((v21 < *(float *)(v20 + 12)) | __UNORDERED__(v21, *(float *)(v20 + 12))) )
                    goto LABEL_18;
                  ++v16;
                  v17 += 20;
                }
              }
              v4 = dpvs_activeFarPlane;
              v5 = dpvs_activeNearPlane;
            }
          }
        }
        else
        {
          v9 = a1 + 8;
          while ( 1 )
          {
            v10 = *(float *)((char *)v34 + *(unsigned __int8 *)(v9 + 8)) * *(float *)(v9 - 8)
                + *(float *)((char *)v34 + *(unsigned __int8 *)(v9 + 9)) * *(float *)(v9 - 4)
                + *(float *)((char *)v34 + *(unsigned __int8 *)(v9 + 10)) * *(float *)v9;
            if ( (v10 < *(float *)(v9 + 4)) | __UNORDERED__(v10, *(float *)(v9 + 4)) )
              break;
            ++v8;
            v9 += 20;
            if ( v8 >= a2 )
              goto LABEL_9;
          }
        }
      }
      v29 += 32;
      v28 = *(_DWORD *)(tr_world + 196);
      result = ++v33;
    }
    while ( v33 < v28 );
  }
  return result;
}

/* ---- R_AddSkySurfacesDPVS  0x004E4B70 ----  [HIGH] */
int R_AddSkySurfacesDPVS()
{
  int v0;
  int v1;
  int result;
  int i;
  _DWORD *v4;
  double v5;

  v0 = dpvs_activeFarPlane;
  if ( dpvs_activeFarPlane )
  {
    v1 = tr_world;
    dpvs_activeFarPlane = 0;
    result = *(_DWORD *)(tr_world + 164);
    for ( i = 0; i < result; ++i )
    {
      v4 = *(_DWORD **)(*(_DWORD *)(v1 + 168) + 4 * i);
      if ( *v4 != tr_viewCount )
      {
        v5 = *(float *)(*(unsigned __int8 *)(v0 + 18) + v4[2] + 12) * *(float *)(v0 + 8)
           + *(float *)(*(unsigned __int8 *)(v0 + 17) + v4[2] + 12) * *(float *)(v0 + 4)
           + *(float *)(*(unsigned __int8 *)(v0 + 16) + v4[2] + 12) * *(float *)v0;
        if ( (v5 < *(float *)(v0 + 12)) | __UNORDERED__(v5, *(float *)(v0 + 12)) )
        {
          R_AddTrianglesSurface(4, v4, 0, (int)&dpvs_frustumPlanes, 0);
          v1 = tr_world;
        }
      }
      result = *(_DWORD *)(v1 + 164);
    }
  }
  return result;
}

/* ---- R_AddWorldSurfacesDPVS  0x004E4C10 ----  VERIFIED */
cvar_t *R_AddWorldSurfacesDPVS()
{
  cvar_t *result;
  int v1;
  int v2;
  int v3;
  int v4;
  int v5;
  _DWORD *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  float *v13;
  int v14;
  _DWORD *v15;
  int v16;
  char v17[0x1000];  // [esp+8h] [ebp-3F000h] BYREF   -- occluders
  char v18[0x1E000]; // [esp+1008h] [ebp-3E000h] BYREF -- active planes
  char v19[0x20000]; // [esp+1F008h] [ebp-20000h] BYREF -- cell entity links

  result = r_drawworld;
  v1 = 0;
  if ( r_drawworld->integer )
  {
    if ( (tr_refdef_rdflags & 1) == 0 )
    {
      dpvs_occluders = (int)&v17;
      dpvs_activePlanes = (int)&v18;
      dpvs_cellEntityLinks = (int)&v19;
      dpvs_occluderCount = 0;
      tr_currentEntityNumber = 1022;
      tr_shiftedEntityNumber = 261632;
      v2 = (1 << tr_refdef_num_dlights) - 1;
      R_SetupDPVS();
      R_FilterModelsIntoCells((int)dpvs_frustumPlanes, 4);
      R_AddCoronas((int)dpvs_frustumPlanes, 4);
      v3 = R_CellForCamera();
      if ( v3 < 0 )
      {
        v6 = (_DWORD *)tr_world;
        v7 = *(_DWORD *)(tr_world + 288);
        if ( outsideMapEnts->integer )
        {
          v16 = 0;
          if ( v7 > 0 )
          {
            v8 = 0;
            do
            {
              R_AddCellSurfaces(v8 + v6[73], (int)dpvs_frustumPlanes, 4, v2);
              R_AddCellCullGroups(4, v2, v8 + *(_DWORD *)(tr_world + 292), (int)dpvs_frustumPlanes);
              v6 = (_DWORD *)tr_world;
              v8 += 64;
              ++v16;
            }
            while ( v16 < *(_DWORD *)(tr_world + 288) );
          }
        }
        else
        {
          if ( v7 > 0 )
          {
            v9 = 0;
            do
            {
              R_AddCellSurfaces(v9 + v6[73], (int)dpvs_frustumPlanes, 4, v2);
              v6 = (_DWORD *)tr_world;
              ++v1;
              v9 += 64;
            }
            while ( v1 < *(_DWORD *)(tr_world + 288) );
            v1 = 0;
          }
          v10 = 0;
          if ( (int)v6[81] > 0 )
          {
            do
            {
              R_AddCullGroupDPVS(4, v1 + v6[82], (int)dpvs_frustumPlanes, v2);
              v6 = (_DWORD *)tr_world;
              ++v10;
              v1 += 36;
            }
            while ( v10 < *(_DWORD *)(tr_world + 324) );
          }
        }
      }
      else
      {
        v4 = *(_DWORD *)(tr_world + 292) + (v3 << 6);
        v5 = v4;
        if ( r_singlecell->integer )
        {
          dpvs_activeFarPlane = 0;
          R_AddCellSurfaces(v4, (int)dpvs_frustumPlanes, 4, v2);
          R_AddCellCullGroups(4, v2, v5, (int)dpvs_frustumPlanes);
        }
        else
        {
          R_RecursivePortalWalk(v4, (int)&dpvs_nearPlane, (float *)dpvs_frustumPlanes, 4, v2);
        }
      }
      R_AddSkySurfacesDPVS();
      result = r_showCullXModels;
      if ( r_showCullXModels->integer == 2 || r_showCullBModels->integer == 2 )
      {
        result = (cvar_t *)tr_world;
        v11 = 0;
        if ( *(int *)(tr_world + 288) > 0 )
        {
          v12 = 0;
          do
          {
            v13 = *(float **)(*(int *)(tr_world + 292) + v12 + 60);
            if ( v13 )
            {
              do
              {
                v14 = **(_DWORD **)v13;
                if ( (v14 != 1 || r_showCullXModels->integer == 2) && (v14 || r_showCullBModels->integer == 2) )
                {
                  v15 = dword_541840;
                  if ( *(_DWORD *)(*(_DWORD *)v13 + 692) )
                    v15 = dword_541830;
                  R_AddDebugBox(v15, v13 + 4, v13 + 1);
                }
                v13 = (float *)*((_DWORD *)v13 + 7);
              }
              while ( v13 );
              result = (cvar_t *)tr_world;
            }
            ++v11;
            v12 += 64;
          }
          while ( v11 < *(int *)(tr_world + 288) );
        }
      }
    }
  }
  return result;
}


/* ---- RE_SetFarPlaneDist  0x004E4EB0 ----  VERIFIED */
void __cdecl RE_SetFarPlaneDist(float a1)
{
  if ( !(a1 <= 0.0) )
    dpvs_cullDist = a1;
  else
    dpvs_cullDist = 0.0;
}


int dpvs_initialSetup = 1;
