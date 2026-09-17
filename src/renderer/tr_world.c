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

extern int R_AddDrawSurf();
extern int R_AddWorldSurfaceNoCull();
extern int R_DlightBmodel();
extern int R_RotateForModelEntity();
extern int R_CullLocalBox();

/* ---- R_DlightTris  0x0050AF60 ----  VERIFIED */
int __cdecl R_DlightTris(float *a1, int a2)
{
  int v2;
  float *v3;
  int v4;
  int v5;
  int v6;
  float *v7;
  float *v8;
  int v9;
  float *v10;
  float *v11;
  int v13;
  int v14;

  v2 = tr_refdef_num_dlights;
  v3 = (float *)tr_refdef_dlights;
  v4 = 0;
  v13 = 0;
  v14 = 0;
  if ( tr_refdef_num_dlights >= 4 )
  {
    v5 = 2;
    do
    {
      if ( ((1 << v4) & a2) != 0
        && v3[30] + v3[29] > a1[3]
        && v3[30] - v3[29] < a1[6]
        && v3[31] + v3[29] > a1[4]
        && v3[31] - v3[29] < a1[7]
        && v3[29] + v3[32] > a1[5]
        && v3[32] - v3[29] < a1[8] )
      {
        v13 |= 1 << v4;
      }
      v6 = 1 << (v5 - 1);
      v7 = v3 + 34;
      if ( (v6 & a2) != 0
        && v7[30] + v7[29] > a1[3]
        && v7[30] - v7[29] < a1[6]
        && v7[31] + v7[29] > a1[4]
        && v7[31] - v7[29] < a1[7]
        && v7[29] + v7[32] > a1[5]
        && v7[32] - v7[29] < a1[8] )
      {
        v13 |= v6;
      }
      v8 = v7 + 34;
      if ( ((1 << v5) & a2) != 0
        && v8[30] + v8[29] > a1[3]
        && v8[30] - v8[29] < a1[6]
        && v8[31] + v8[29] > a1[4]
        && v8[31] - v8[29] < a1[7]
        && v8[29] + v8[32] > a1[5]
        && v8[32] - v8[29] < a1[8] )
      {
        v13 |= 1 << v5;
      }
      v9 = 1 << (v5 + 1);
      v10 = v8 + 34;
      if ( (v9 & a2) != 0
        && v10[30] + v10[29] > a1[3]
        && v10[30] - v10[29] < a1[6]
        && v10[31] + v10[29] > a1[4]
        && v10[31] - v10[29] < a1[7]
        && v10[29] + v10[32] > a1[5]
        && v10[32] - v10[29] < a1[8] )
      {
        v13 |= v9;
      }
      v2 = tr_refdef_num_dlights;
      v5 += 4;
      v4 = v14 + 4;
      v3 = v10 + 34;
      v14 += 4;
    }
    while ( v5 + 1 < tr_refdef_num_dlights );
  }
  if ( v4 < v2 )
  {
    v11 = v3 + 29;
    do
    {
      if ( ((1 << v4) & a2) != 0
        && v11[1] + *v11 > a1[3]
        && v11[1] - *v11 < a1[6]
        && *v11 + v11[2] > a1[4]
        && v11[2] - *v11 < a1[7]
        && *v11 + v11[3] > a1[5]
        && v11[3] - *v11 < a1[8] )
      {
        v13 |= 1 << v4;
      }
      LOBYTE(v4) = v14 + 1;
      v11 += 34;
      ++v14;
    }
    while ( v14 < tr_refdef_num_dlights );
  }
  return v13;
}

/* ---- R_DlightSurface  0x0050B240 ----  VERIFIED */
int __cdecl R_DlightSurface(int a1, int a2)
{
  int v2;

  v2 = *(_DWORD *)(a1 + 8);
  if ( *(int *)v2 < 24 )
    return 0;
  *(_DWORD *)(v2 + 8) = R_DlightTris((float *)v2, a2);
  if ( a2 )
    ++*(int *)&qword_16C5808;
  return a2;
}

/* ---- R_AddWorldSurface  0x0050B270 ----  VERIFIED */
int __cdecl R_AddWorldSurface(BOOL a1, _DWORD *a2)
{
  int result;
  int v4;
  int *v5;
  int v6;

  result = tr_viewCount;
  if ( *a2 != tr_viewCount )
  {
    *a2 = tr_viewCount;
    v4 = a2[2];
    if ( r_nocull->integer || *(int *)v4 < 24 || (result = R_CullLocalBox(v4 + 12) == 2) == 0 )
    {
      if ( a1 )
      {
        if ( *(int *)v4 < 24 )
        {
          a1 = 0;
        }
        else
        {
          *(_DWORD *)(v4 + 8) = R_DlightTris((float *)v4, a1);
          /* `inc dword ptr qword_16C5808` at 0x0050B2C7; see R_DlightSurface. */
          ++*(int *)&qword_16C5808;
        }
        a1 = a1;
      }
      v5 = (int *)a2[2];
      v6 = a2[1];
      if ( *v5 < 24 )
        return R_AddDrawSurf(storageClass, v6, (int)v5, a1, 0, 0);
      else
        return R_AddDrawSurf(v5[1], v6, (int)v5, a1, 0, 0);
    }
  }
  return result;
}

/* ---- R_AddBrushModelSurfaces  0x0050B310 ----  VERIFIED */
int __cdecl R_AddBrushModelSurfaces(int ent)
{
  int bmodel;
  int i;
  int surfOffset;
  int numSurfaces;

  numSurfaces = ent;
  if ( *(int *)(ent + 0x2B4) != 2 )
  {
    numSurfaces = (int)r_drawBModels;
    if ( r_drawBModels->integer )
    {
      R_RotateForModelEntity(&tr_or, &tr_viewParms_originX, ent);
      bmodel = *(_DWORD *)(tr_models[*(int *)(ent + 8)] + 76);
      R_DlightBmodel(bmodel);
      numSurfaces = *(int *)(bmodel + 28);
      i = 0;
      if ( numSurfaces > 0 )
      {
        surfOffset = 0;
        do
        {
          R_AddWorldSurfaceNoCull(*(_DWORD *)(tr_currentEntity + 156),
                                  (_DWORD *)(surfOffset + *(_DWORD *)(bmodel + 24)));
          numSurfaces = *(int *)(bmodel + 28);
          ++i;
          surfOffset += 12;
        }
        while ( i < numSurfaces );
      }
    }
  }
  return numSurfaces;
}

/* ---- R_PointInLeaf  0x0050B390 ----  VERIFIED */
_DWORD *__cdecl R_PointInLeaf(float *a1)
{
  _DWORD *v1;
  float *v2;

  if ( !tr_world )
    ri_Error(1, "\x15" "R_PointInLeaf: bad model");
  v1 = *(_DWORD **)(tr_world + 152);
  while ( *v1 == -1 )
  {
    v2 = (float *)v1[3];
    if ( v2[2] * a1[2] + v2[1] * a1[1] + *v2 * *a1 - v2[3] <= 0.0 )
      v1 = (_DWORD *)v1[5];
    else
      v1 = (_DWORD *)v1[4];
  }
  return v1;
}

