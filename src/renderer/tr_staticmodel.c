/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include <malloc.h>
#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_gl_types.h"
#include "tr_tess.h"
#include "tr_smcache.h"

/* Native 1.1 DObj model-list entry, including the model index at +8. */
typedef struct {
  void *model;
  const char *tagName;
  short modelIndex;
  short padding;
} staticDObjModel_t;
typedef char staticDObjModel_size_check[sizeof(staticDObjModel_t) == 12 ? 1 : -1];

extern char xmodel_defaultCollision[];

extern void AnglesToAxis( const float *angles, float *axis );
extern int BoxOnPlaneSide();
extern int DObjCalcAnim();
extern int DObjCalcSkel();
extern int DObjCreate();
extern int GL_Bind();
extern int GL_ClientState();
extern int GL_Cull();
extern int GL_State();
extern int Hunk_AllocAlignInternal();
extern int MatrixInverse();
extern int RB_ChooseSurfaceCountColor();
extern int RB_GetAnimatedImage();
extern void __cdecl RB_SetIteratorFog( void );
extern int RB_SetupMultitexture();
extern int RB_SetupMultitextureATI();
extern int RE_RegisterModel();
extern int R_AddScaledDebugString();
extern int R_CacheStaticModelSurface();
extern char *__cdecl R_CacheableStaticModelShader( int shader );
extern int R_CreateBufferARB();
extern int R_FindShader();
extern double __cdecl R_GetLodDist( float *entityOrigin );
extern int R_GetStaticLightContributions();
extern float *__cdecl R_GetXModelBounds( float *transform, int obj, float *mins, float *maxs );
extern int R_RemapTextureCoordinatesForSheet();
extern int count;
extern int R_SetupStaticModelLighting();
extern _DWORD *__cdecl R_SyncRenderThread( void );
extern float VectorMax( const float *v );
extern void __cdecl DObjBindEvalStorage( void *storage, void *dobj );
extern int DObjGetNumSurfaces();
extern int DObjGetSurfaces();
extern int XSurfaceGetVerts();
extern int __cdecl XModelGetLodForDist( const void *model, float distance );

/* ---- R_AddModelToCell_0  0x005038E0 ----  [HIGH] */
_DWORD *__cdecl R_AddModelToCell_0(int a1, int a2, int a3)
{
  int v3;
  _DWORD *result;

  v3 = *(_DWORD *)(a2 + 292) + (a1 << 6);
  result = *(_DWORD **)(v3 + 56);
  if ( !result || *result != a3 )
  {
    result = (_DWORD *)ri_Hunk_Alloc(8);
    *result = a3;
    result[1] = *(_DWORD *)(v3 + 56);
    *(_DWORD *)(v3 + 56) = result;
  }
  return result;
}

/* ---- R_FilterModelIntoCells_r_0  0x00503920 ----  VERIFIED */
void __cdecl R_FilterModelIntoCells_r_0(int a1, _DWORD *a2, int a3, int *a4, _DWORD *a5)
{
  _DWORD *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  _DWORD *v10;
  int v11;
  _DWORD *v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18[3]; // [esp+10h] [ebp-18h] BYREF
  _DWORD v19[3]; // [esp+1Ch] [ebp-Ch] BYREF

  v5 = a2;
  if ( a2[2] != -2 )
  {
LABEL_7:
    v7 = v5[2];
    if ( v7 >= 0 )
    {
      v8 = *(_DWORD *)(a1 + 292);
      v9 = v7 << 6;
      v10 = *(_DWORD **)(v9 + v8 + 56);
      v11 = v8 + v9;
      if ( !v10 || *v10 != a3 )
      {
        v12 = (_DWORD *)ri_Hunk_Alloc(8);
        *v12 = a3;
        v12[1] = *(_DWORD *)(v11 + 56);
        *(_DWORD *)(v11 + 56) = v12;
      }
    }
    return;
  }
  while ( 1 )
  {
    v6 = BoxOnPlaneSide((int)a4, (int)a5, v5[3]);
    if ( v6 != 3 )
    {
      v5 = (_DWORD *)v5[v6 + 3];
      goto LABEL_6;
    }
    if ( *(_BYTE *)(v5[3] + 16) < 3u )
      break;
    R_FilterModelIntoCells_r_0(a1, (_DWORD *)v5[4], a3, a4, a5);
    v5 = (_DWORD *)v5[5];
LABEL_6:
    if ( v5[2] != -2 )
      goto LABEL_7;
  }
  v13 = *a4;
  v14 = a4[2];
  v18[1] = a4[1];
  v15 = v5[3];
  v18[0] = v13;
  v18[2] = v14;
  v18[*(unsigned __int8 *)(v15 + 16)] = *(_DWORD *)(v15 + 12);
  v16 = a5[1];
  v19[0] = *a5;
  v17 = a5[2];
  v19[1] = v16;
  v19[2] = v17;
  v19[*(unsigned __int8 *)(v15 + 16)] = *(_DWORD *)(v15 + 12);
  if ( (*(float *)&v18[*(unsigned __int8 *)(v15 + 16)] < (double)*(float *)&a5[*(unsigned __int8 *)(v15 + 16)])
     | __UNORDERED__(*(float *)&v18[*(unsigned __int8 *)(v15 + 16)], *(float *)&a5[*(unsigned __int8 *)(v15 + 16)]) )
  {
    R_FilterModelIntoCells_r_0(a1, (_DWORD *)v5[4], a3, v18, a5);
  }
  R_FilterModelIntoCells_r_0(a1, (_DWORD *)v5[5], a3, a4, v19);
}

/* ---- R_OptimizeSModelSurfGeneric  0x00503A60 ----  VERIFIED */
int __cdecl R_OptimizeSModelSurfGeneric(int a1)
{
  int result;
  int v2;
  bool v3; // zf
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;

  result = *(_DWORD *)(*(_DWORD *)(a1 + 8) + 80);
  if ( (result & 0x3FF5FD00) != 0 )
  {
    if ( (result & 0x3FF7FC00) == 0 )
    {
      v8 = 32 * *(unsigned __int16 *)(a1 + 18);
      *(_DWORD *)(a1 + 4) = 3;
      *(_DWORD *)a1 = 20;
      result = ri_Hunk_Alloc(v8);
      v5 = 0;
      v3 = *(_WORD *)(a1 + 18) == 0;
      *(_DWORD *)(a1 + 36) = result;
      if ( !v3 )
      {
        result = 0;
        v6 = 0;
        do
        {
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36)) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v5);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 4) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v5 + 4);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 8) = *(_DWORD *)(v6 + *(_DWORD *)(a1 + 24));
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 12) = *(_DWORD *)(*(_DWORD *)(a1 + 24) + v6 + 4);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 16) = *(_DWORD *)(*(_DWORD *)(a1 + 24) + v6 + 8);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 20) = *(_DWORD *)(v6 + *(_DWORD *)(a1 + 28));
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 24) = *(_DWORD *)(v6 + *(_DWORD *)(a1 + 28) + 4);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 28) = *(_DWORD *)(v6 + *(_DWORD *)(a1 + 28) + 8);
          ++v5;
          v6 += 12;
          result += 32;
        }
        while ( v5 < *(unsigned __int16 *)(a1 + 18) );
      }
    }
  }
  else
  {
    v7 = 20 * *(unsigned __int16 *)(a1 + 18);
    *(_DWORD *)(a1 + 4) = 3;
    *(_DWORD *)a1 = 16;
    result = ri_Hunk_Alloc(v7);
    v2 = 0;
    v3 = *(_WORD *)(a1 + 18) == 0;
    *(_DWORD *)(a1 + 36) = result;
    if ( !v3 )
    {
      result = 0;
      v4 = 0;
      do
      {
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36)) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v2);
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 4) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v2 + 4);
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 8) = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 28));
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 12) = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 28) + 4);
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 16) = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 28) + 8);
        ++v2;
        v4 += 12;
        result += 20;
      }
      while ( v2 < *(unsigned __int16 *)(a1 + 18) );
    }
  }
  return result;
}

/* ---- R_OptimizeSModelSurfNV  0x00503BC0 ----  VERIFIED */
int __cdecl R_OptimizeSModelSurfNV(int a1)
{
  int result;
  int v2;
  unsigned int v3;
  int v4;
  bool v5; // zf
  int v6;
  int v7;
  unsigned int v8;
  int v9;
  int v10;

  result = *(_DWORD *)(*(_DWORD *)(a1 + 8) + 80);
  if ( (result & 0x3FF5FD00) != 0 )
  {
    if ( (result & 0x3FF7FC00) == 0 )
    {
      v7 = 32 * *(unsigned __int16 *)(a1 + 18) + 31;
      *(_DWORD *)a1 = 23;
      v8 = v7 & 0xFFFFFFE0;
      if ( (int)(tr_staticVertexMemoryPrimaryUsed + v8) >= tr_staticVertexMemoryPrimaryLimit )
      {
        if ( (int)(tr_staticVertexMemorySecondaryUsed + v8) >= tr_staticVertexMemorySecondaryLimit )
        {
          *(_DWORD *)(a1 + 36) = Hunk_AllocAlignInternal(v8, 32);
          result = 3;
        }
        else
        {
          *(_DWORD *)(a1 + 36) = tr_staticVertexMemorySecondaryUsed + tr_staticVertexMemorySecondary;
          tr_staticVertexMemorySecondaryUsed += v8;
          result = 2;
        }
      }
      else
      {
        *(_DWORD *)(a1 + 36) = tr_staticVertexMemoryPrimaryUsed + tr_staticVertexMemoryPrimary;
        tr_staticVertexMemoryPrimaryUsed += v8;
        result = 1;
      }
      v9 = 0;
      v5 = *(_WORD *)(a1 + 18) == 0;
      *(_DWORD *)(a1 + 4) = result;
      if ( !v5 )
      {
        result = 0;
        v10 = 0;
        do
        {
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36)) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v9);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 4) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v9 + 4);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 8) = *(_DWORD *)(v10 + *(_DWORD *)(a1 + 24));
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 12) = *(_DWORD *)(*(_DWORD *)(a1 + 24) + v10 + 4);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 16) = *(_DWORD *)(*(_DWORD *)(a1 + 24) + v10 + 8);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 20) = *(_DWORD *)(v10 + *(_DWORD *)(a1 + 28));
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 24) = *(_DWORD *)(v10 + *(_DWORD *)(a1 + 28) + 4);
          *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 28) = *(_DWORD *)(v10 + *(_DWORD *)(a1 + 28) + 8);
          ++v9;
          v10 += 12;
          result += 32;
        }
        while ( v9 < *(unsigned __int16 *)(a1 + 18) );
      }
    }
  }
  else
  {
    v2 = 5 * *(unsigned __int16 *)(a1 + 18);
    *(_DWORD *)a1 = 19;
    v3 = (4 * v2 + 31) & 0xFFFFFFE0;
    if ( (int)(tr_staticVertexMemoryPrimaryUsed + v3) >= tr_staticVertexMemoryPrimaryLimit )
    {
      if ( (int)(tr_staticVertexMemorySecondaryUsed + v3) >= tr_staticVertexMemorySecondaryLimit )
      {
        *(_DWORD *)(a1 + 36) = Hunk_AllocAlignInternal(v3, 32);
        result = 3;
      }
      else
      {
        *(_DWORD *)(a1 + 36) = tr_staticVertexMemorySecondaryUsed + tr_staticVertexMemorySecondary;
        tr_staticVertexMemorySecondaryUsed += v3;
        result = 2;
      }
    }
    else
    {
      *(_DWORD *)(a1 + 36) = tr_staticVertexMemoryPrimaryUsed + tr_staticVertexMemoryPrimary;
      tr_staticVertexMemoryPrimaryUsed += v3;
      result = 1;
    }
    v4 = 0;
    v5 = *(_WORD *)(a1 + 18) == 0;
    *(_DWORD *)(a1 + 4) = result;
    if ( !v5 )
    {
      result = 0;
      v6 = 0;
      do
      {
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36)) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v4);
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 4) = *(_DWORD *)(*(_DWORD *)(a1 + 20) + 8 * v4 + 4);
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 8) = *(_DWORD *)(v6 + *(_DWORD *)(a1 + 28));
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 12) = *(_DWORD *)(v6 + *(_DWORD *)(a1 + 28) + 4);
        *(_DWORD *)(result + *(_DWORD *)(a1 + 36) + 16) = *(_DWORD *)(v6 + *(_DWORD *)(a1 + 28) + 8);
        ++v4;
        v6 += 12;
        result += 20;
      }
      while ( v4 < *(unsigned __int16 *)(a1 + 18) );
    }
  }
  return result;
}

/* ---- R_OptimizeSModelSurfATI  0x00503DD0 ----  VERIFIED */
int __cdecl R_OptimizeSModelSurfATI(int a1)
{
  int v1;
  unsigned int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  unsigned int v8;
  int v9;
  int v10;
  int v11;
  _DWORD *v12;
  _DWORD *v13;
  int v14;
  _DWORD *v15;
  int v16;
  _DWORD *v17;
  _DWORD *v18;
  _DWORD *v19;
  _DWORD *v20;
  int v21;
  int v23;
  int v24;
  int v25;
  int v26;
  _BYTE v27[262144]; // [esp+14h] [ebp-40000h] BYREF

  v1 = *(_DWORD *)(*(_DWORD *)(a1 + 8) + 80);
  v2 = (2 * *(unsigned __int16 *)(a1 + 16) + 31) & 0xFFFFFFE0;
  if ( (v1 & 0x3FF5FD00) != 0 )
  {
    if ( (v1 & 0x3FF7FC00) != 0 )
      return 0;
    v5 = *(unsigned __int16 *)(a1 + 18);
    *(_DWORD *)a1 = 22;
    v4 = 32 * v5;
  }
  else
  {
    v3 = *(unsigned __int16 *)(a1 + 18);
    *(_DWORD *)a1 = 18;
    v4 = 20 * v3;
  }
  v25 = v4;
  v6 = v4;
  if ( glConfig_ATIElementArray )
    v6 = v4 + v2;
  v7 = tr_staticVertexMemoryPrimaryUsed;
  v8 = (v6 + 31) & 0xFFFFFFE0;
  if ( (int)(tr_staticVertexMemoryPrimaryUsed + v8) > tr_staticVertexMemoryPrimaryLimit )
  {
    v7 = tr_staticVertexMemorySecondaryUsed;
    v10 = tr_staticVertexMemorySecondaryUsed + v8;
    if ( v10 > tr_staticVertexMemorySecondaryLimit )
    {
      v7 = v4;
      v9 = 0;
    }
    else
    {
      tr_staticVertexMemorySecondaryUsed = v10;
      v9 = 2;
    }
  }
  else
  {
    tr_staticVertexMemoryPrimaryUsed += v8;
    v9 = 1;
  }
  *(_DWORD *)(a1 + 4) = v9;
  if ( !v9 )
    return 0;
  if ( v9 == 1 )
    *(_DWORD *)(a1 + 36) = tr_staticVertexMemoryPrimary;
  else
    *(_DWORD *)(a1 + 36) = tr_staticVertexMemorySecondary;
  *(_DWORD *)(a1 + 4) = 3;
  if ( glConfig_ATIElementArray )
  {
    v11 = *(_DWORD *)(a1 + 32);
    *(_DWORD *)(a1 + 40) = v7 + v2;
    v24 = 2 * *(unsigned __int16 *)(a1 + 16);
    v23 = *(_DWORD *)(a1 + 36);
    *(_DWORD *)(a1 + 44) = v7;
    qglUpdateObjectBufferATI(v23, v7, v24, v11, 34658);
  }
  else
  {
    *(_DWORD *)(a1 + 44) = 0;
    *(_DWORD *)(a1 + 40) = v7;
  }
  if ( *(_DWORD *)a1 == 18 )
  {
    if ( *(_WORD *)(a1 + 18) )
    {
      v12 = *(_DWORD **)(a1 + 28);
      v13 = *(_DWORD **)(a1 + 20);
      v14 = *(unsigned __int16 *)(a1 + 18);
      v15 = (_DWORD *)(v27 + 4);
      do
      {
        *(v15 - 1) = *v13;
        *v15 = v13[1];
        v15[1] = *v12;
        v15[2] = v12[1];
        v15[3] = v12[2];
        v13 += 2;
        v12 += 3;
        v15 += 5;
        --v14;
      }
      while ( v14 );
    }
  }
  else if ( *(_WORD *)(a1 + 18) )
  {
    v16 = *(_DWORD *)(a1 + 24);
    v17 = *(_DWORD **)(a1 + 28);
    v18 = *(_DWORD **)(a1 + 20);
    v19 = (_DWORD *)(v16 + 8);
    v20 = (_DWORD *)(v27 + 4);
    v21 = v16 - (_DWORD)v17;
    v26 = *(unsigned __int16 *)(a1 + 18);
    do
    {
      *(v20 - 1) = *v18;
      *v20 = v18[1];
      v20[1] = *(_DWORD *)((char *)v17 + v21);
      v20[2] = *(v19 - 1);
      v20[3] = *v19;
      v20[4] = *v17;
      v20[5] = v17[1];
      v20[6] = v17[2];
      v18 += 2;
      v19 += 3;
      v17 += 3;
      v20 += 8;
      --v26;
    }
    while ( v26 );
    v4 = v25;
  }
  qglUpdateObjectBufferATI(*(_DWORD *)(a1 + 36), *(_DWORD *)(a1 + 40), v4, v27, 34658);
  return 1;
}

/* ---- R_OptimizeSModelSurfARB  0x00503FD0 ----  VERIFIED */
int __cdecl R_OptimizeSModelSurfARB(int a1)
{
  int v1;
  unsigned int v2;
  int v3;
  int v4;
  int BufferARB;
  _DWORD *v6;
  _DWORD *v7;
  int v8;
  _DWORD *v9;
  int v10;
  _DWORD *v11;
  _DWORD *v12;
  _DWORD *v13;
  _DWORD *v14;
  int v15;
  int v16;
  int v18;
  int v19;
  _BYTE v20[262144]; // [esp+14h] [ebp-40000h] BYREF

  v1 = *(_DWORD *)(*(_DWORD *)(a1 + 8) + 80);
  v2 = (2 * *(unsigned __int16 *)(a1 + 16) + 31) & 0xFFFFFFE0;
  if ( (v1 & 0x3FF5FD00) != 0 )
  {
    if ( (v1 & 0x3FF7FC00) != 0 )
      return 0;
    v4 = *(unsigned __int16 *)(a1 + 18);
    *(_DWORD *)a1 = 21;
    v3 = 32 * v4;
  }
  else
  {
    v1 = *(unsigned __int16 *)(a1 + 18);
    *(_DWORD *)a1 = 17;
    v3 = 20 * v1;
  }
  v18 = v3;
  BufferARB = R_CreateBufferARB(v1, 34963, v2, *(_DWORD *)(a1 + 32), 35044);
  v6 = (_DWORD *)(a1 + 40);
  *(_DWORD *)(a1 + 40) = BufferARB;
  if ( !BufferARB )
    return 0;
  if ( *(_DWORD *)a1 == 17 )
  {
    if ( *(_WORD *)(a1 + 18) )
    {
      v6 = *(_DWORD **)(a1 + 28);
      v7 = *(_DWORD **)(a1 + 20);
      v8 = *(unsigned __int16 *)(a1 + 18);
      v9 = (_DWORD *)(v20 + 4);
      do
      {
        *(v9 - 1) = *v7;
        *v9 = v7[1];
        v9[1] = *v6;
        v9[2] = v6[1];
        v9[3] = v6[2];
        v7 += 2;
        v6 += 3;
        v9 += 5;
        --v8;
      }
      while ( v8 );
    }
  }
  else
  {
    if ( *(_WORD *)(a1 + 18) )
    {
      v10 = *(_DWORD *)(a1 + 24);
      v11 = *(_DWORD **)(a1 + 28);
      v12 = *(_DWORD **)(a1 + 20);
      v13 = (_DWORD *)(v10 + 8);
      v14 = (_DWORD *)(v20 + 4);
      v15 = v10 - (_DWORD)v11;
      v19 = *(unsigned __int16 *)(a1 + 18);
      do
      {
        *(v14 - 1) = *v12;
        *v14 = v12[1];
        v14[1] = *(_DWORD *)((char *)v11 + v15);
        v14[2] = *(v13 - 1);
        v14[3] = *v13;
        v14[4] = *v11;
        v14[5] = v11[1];
        v14[6] = v11[2];
        v12 += 2;
        v13 += 3;
        v11 += 3;
        v14 += 8;
        --v19;
      }
      while ( v19 );
      v3 = v18;
    }
    v6 = v20;
  }
  v16 = R_CreateBufferARB((int)v6, 34962, v3, (int)v20, 35044);
  *(_DWORD *)(a1 + 36) = v16;
  if ( !v16 )
  {
    qglDeleteBuffersARB(1, a1 + 40);
    return 0;
  }
  return 1;
}

/* ---- R_SetupDObjToStaticModel  0x00504150 ----  VERIFIED */
int __cdecl R_SetupDObjToStaticModel(int a1, const char *a2)
{
  int v2;
  int v3;
  int result;
  int v5;
  char *v6;  /* retail's cursor; NOT const, it is the copy source AND */
  int v7;
  char v8;

  if ( r_optimize->integer && r_optimizeSModels->integer )
  {
    v2 = *(_DWORD *)(a1 + 24);
    v3 = *(__int16 *)(*(_DWORD *)(v2 + 4) + 100);
    result = ri_Hunk_Alloc(strlen(a2) + 4 * v3 + 13);
    v5 = result + 4 * v3 + 12;
    *(_DWORD *)result = v5;
    v6 = (char *)a2;
    v7 = v5 - (int)a2;
    do
    {
      v8 = *v6;
      v6[v7] = *v6;
      ++v6;
    }
    while ( v8 );
    *(_DWORD *)(result + 4) = v3;
    *(_DWORD *)(result + 8) = v2;
  }
  else
  {
    result = ri_Hunk_Alloc(strlen(a2) + 13);
    *(_DWORD *)result = result + 12;
    strcpy((char *)(result + 12), a2);
    *(_DWORD *)(result + 4) = 0;
    *(_DWORD *)(result + 8) = 0;
  }
  return result;
}

/* ---- R_FinishDObjToStaticModel  0x00504200 ----  VERIFIED */
int __cdecl R_FinishDObjToStaticModel(_DWORD *a1, int a2)
{
  int v2;
  int result;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v10;
  int *v11;
  int v12;
  int v13;
  int v14;
  unsigned __int16 v15;
  const char *v16;
  char *v17;
  char *Shader;
  int v19;
  int v20;
  int v21;
  int v22;
  int v23;
  int v24;
  int v25;
  int *v26;
  unsigned __int16 *v27;
  int v28;
  char *v29;
  int v30;
  int v31;
  unsigned __int16 v32;
  int v33;
  int v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  int v40;
  int v41;
  int v42;
  int v43;
  int v44;
  int v45;
  char *v46;
  int v47;
  int v48;
  int v49;
  int v50;
  int v51;
  int v52;
  int v53;
  int v54;
  int v55;
  const void *v56;
  void *v57;
  int v58;
  int v59;
  int *v60;
  int v61;
  int v62;
  int v63;
  int **v64;
  _WORD *v65;
  _DWORD v66[1024]; // [esp+10h] [ebp-5058h] BYREF
  int v67[4096]; // [esp+1010h] [ebp-4058h] BYREF
  int v68[4]; // [esp+5010h] [ebp-58h] BYREF
  int v69;
  int v70;
  int *v71;
  int v72;
  char *v73;
  int v74;
  int **v75;
  char *v76;
  int v77;
  int v78;
  int v79; // [esp+5048h] [ebp-20h] BYREF
  int v80;
  int v81;
  _DWORD *v82;
  int v83;
  int *v84;
  int *v85;
  int v86;

  v2 = a2;
  result = *(__int16 *)(*(_DWORD *)(*(_DWORD *)(a2 + 24) + 4) + 100);
  v74 = result;
  v77 = 0;
  if ( result > 0 )
  {
    v75 = (int **)(a1 + 3);
    while ( 1 )
    {
      v79 = v77;
      v4 = 0;
      v5 = *(unsigned __int8 *)(v2 + 22) - 1;
      v78 = 0;
      if ( v5 >= 0 )
      {
        v6 = v2 + 4 * v5 + 24;
        do
        {
          v7 = *(&v79 + v5);
          if ( v7 >= 0 )
          {
            v8 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)v6 + 4) + 4 * (5 * v7 + 5));
            if ( v8 )
              v4 += *(__int16 *)(*(_DWORD *)(v8 + 4) + 4);
          }
          --v5;
          v6 -= 4;
        }
        while ( v5 >= 0 );
        v78 = v4;
        if ( v4 > 1024 )
          ri_Error(1, "\x15" "model '%s' has %i surfaces, which is more than %i\n", *a1, v4, 1024);
      }
      v65 = (_WORD *)alloca((4 * v4 + 7) & 0xFFFFFFF8);
      v85 = (int *)v65;
      DObjGetSurfaces(v68, v2, v65, (int)&v79);
      v10 = 0;
      v83 = 0;
      if ( v78 > 0 )
      {
        v11 = v85;
        v76 = (char *)((char *)v66 - (char *)v85);
        v82 = (_DWORD *)((char *)v67 - (char *)v85);
        v84 = 0;
        v86 = v78;
        do
        {
          v12 = *(__int16 *)v11;
          v13 = *((__int16 *)v11 + 1);
          v14 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v2 + 4 * v12 + 24) + 4)
                                                              + 20 * (*(&v79 + v12) + 1))
                                                  + 4)
                                      + 8)
                          + 4 * v13);
          v15 = *(_WORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v2 + 4 * v12 + 24) + 4) + 20 * *(&v79 + v12) + 16)
                         + 2 * v13);
          if ( v15 )
            v16 = (const char *)(GetRefString_var + 8 * v15 + 4);
          else
            v16 = "DEFAULT";
          v17 = va("skins/%s", v16);
          Shader = R_FindShader(v17, -1, 1, (const char *)7);
          v19 = *((_DWORD *)Shader + 19);
          *(int *)((char *)v11 + (_DWORD)v76) = (int)Shader;
          v20 = (int)v84;
          *(int *)((char *)&v67[3072] + (_DWORD)v84) = (int)Shader;
          if ( (v19 & 0x1000) != 0 )
            *(int *)((char *)&v67[3072] + v20) = *((_DWORD *)Shader + 96);
          v21 = *(__int16 *)(v14 + 4);
          v22 = *(__int16 *)(v14 + 2);
          v10 = v83;
          v81 = 3 * v21;
          v23 = 0;
          v80 = v22;
          if ( v20 <= 0 )
          {
LABEL_21:
            v24 = v81;
            *(int *)((char *)v11 + (_DWORD)v82) = v83;
            v25 = v80;
            ++v10;
            *(int *)((char *)&v67[2048] + v20) = v24;
            *(int *)((char *)&v67[1024] + v20) = v25;
            v83 = v10;
            v84 = (int *)(v20 + 4);
          }
          else
          {
            while ( v67[v23 + 3072] != *(int *)((char *)&v67[3072] + v20) )
            {
              if ( ++v23 >= v83 )
                goto LABEL_21;
            }
            v41 = v81;
            *(int *)((char *)v11 + (_DWORD)v82) = v23;
            v42 = v41 + v67[v23 + 2048];
            v43 = v67[v23 + 1024];
            v67[v23 + 2048] = v42;
            v67[v23 + 1024] = v80 + v43;
          }
          ++v11;
          --v86;
        }
        while ( v86 );
      }
      v26 = (int *)ri_Hunk_Alloc(48 * v10 + 4);
      v84 = v26;
      *v26 = v10;
      v86 = 0;
      if ( v10 > 0 )
      {
        v27 = (unsigned __int16 *)v26 + 11;
        do
        {
          v28 = v67[v86 + 3072];
          *(_DWORD *)(v27 - 9) = 11;
          *(_DWORD *)(v27 - 7) = storageClass;
          *(_DWORD *)(v27 - 5) = v28;
          v29 = R_CacheableStaticModelShader(v28);
          v30 = v86;
          v31 = v67[v86 + 2048];
          v32 = v67[v86 + 1024];
          *(_DWORD *)(v27 - 3) = v29;
          LOWORD(v29) = v67[v30 + 2048];
          *(v27 - 1) = (unsigned __int16)v29;
          *v27 = v32;
          if ( (unsigned __int16)v29 != v31 || v32 != v67[v30 + 1024] )
            ri_Error(1, "\x15" "model %s surface %s has more than 65,535 vertices"
              " or more than 21,845 triangles", *a1, v66[v30]);
          v33 = *v27;
          v34 = 8 * v33;
          v33 *= 12;
          v35 = ri_Hunk_Alloc(v34 + v33 + v33 + 2 * *(v27 - 1));
          *(_DWORD *)(v27 + 1) = v35;
          v36 = v35 + v33;
          *(_DWORD *)(v27 + 3) = v35 + v34;
          *(_DWORD *)(v27 + 7) = v34 + v33 + v33 + v35;
          v37 = v86;
          v67[v86 + 2048] = 0;
          v67[v37 + 1024] = 0;
          v38 = v83;
          *(_DWORD *)(v27 + 5) = v34 + v36;
          v27 += 24;
          v86 = v37 + 1;
        }
        while ( v37 + 1 < v38 );
        v2 = a2;
        v26 = v84;
        v10 = v38;
      }
      v39 = 0;
      v86 = 0;
      if ( v78 > 0 )
      {
        v76 = (char *)((char *)v66 - (char *)v85);
        v40 = (char *)v85 - (char *)v67;
        v69 = (char *)v85 - (char *)v67;
        while ( 1 )
        {
          v44 = v40 + 4 * v39;
          v45 = *(__int16 *)((char *)v67 + v44);
          v46 = (char *)v67 + v44;
          v72 = (*(unsigned __int8 *)(v45 + v2 + 80) << 6) + *(_DWORD *)(v2 + 4) + 48;
          v47 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v2 + 4 * v45 + 24) + 4) + 20 * (*(&v79 + v45) + 1)) + 4);
          v48 = *((__int16 *)v46 + 1);
          v73 = v46;
          v49 = *(_DWORD *)(*(_DWORD *)(v47 + 8) + 4 * v48);
          v50 = v67[v39];
          v51 = v67[v50 + 1024];
          v52 = (int)&v84[12 * v50 + 1];
          v53 = v67[v50 + 2048];
          v85 = &v67[v50 + 2048];
          v54 = *(__int16 *)(v49 + 4);
          v71 = &v67[v50 + 1024];
          v81 = 3 * v54;
          v80 = *(__int16 *)(v49 + 2);
          v55 = 2 * v53;
          v56 = *(const void **)(v49 + 28);
          v82 = (_DWORD *)v52;
          v57 = (void *)(v55 + *(_DWORD *)(v52 + 32));
          v70 = v55;
          qmemcpy(v57, v56, 6 * v54);
          XSurfaceGetVerts((char *)(8 * v51 + v82[5]), (float *)(12 * v51 + v82[7]), v49, v72, 12 * v51 + v82[6]);
          if ( v51 )
          {
            v58 = v81;
            if ( v81 > 0 )
            {
              v59 = v70;
              do
              {
                *(_WORD *)(v82[8] + v59) += v51;
                v59 += 2;
                --v58;
              }
              while ( v58 );
            }
          }
          if ( (*(_DWORD *)(*(_DWORD *)&v76[(_DWORD)v73] + 76) & 0x1000) != 0 )
            R_RemapTextureCoordinatesForSheet(v80, 8 * v51 + v82[5], *(_DWORD *)&v76[(_DWORD)v73]);
          v60 = v71;
          *v85 += v81;
          v61 = v86;
          *v60 = v80 + v51;
          v2 = a2;
          v86 = v61 + 1;
          if ( v61 + 1 >= v78 )
            break;
          v40 = v69;
          v39 = v86;
        }
        v10 = v83;
        v26 = v84;
      }
      if ( v10 > 0 )
        break;
LABEL_54:
      v64 = v75;
      *v75 = v26;
      result = v77 + 1;
      v75 = v64 + 1;
      if ( ++v77 >= v74 )
        return result;
    }
    v62 = (int)(v26 + 1);
    v85 = (int *)v10;
    while ( 1 )
    {
      if ( glConfig_ARBVertexBufferObject )
      {
        v63 = R_OptimizeSModelSurfARB(v62);
      }
      else
      {
        if ( glConfig_NVVertexArrayRange )
        {
          R_OptimizeSModelSurfNV(v62);
          goto LABEL_52;
        }
        if ( !glConfig_ATIVertexArrayObject )
          goto LABEL_51;
        v63 = R_OptimizeSModelSurfATI(v62);
      }
      if ( !v63 )
LABEL_51:
        R_OptimizeSModelSurfGeneric(v62);
LABEL_52:
      v62 += 48;
      v85 = (int *)((char *)v85 - 1);
      if ( !v85 )
      {
        v26 = v84;
        goto LABEL_54;
      }
    }
  }
  return result;
}

/* ---- R_NeedsBoundsAdjustment  0x00504750 ----  VERIFIED */
int __cdecl R_NeedsBoundsAdjustment(int a1)
{
  int v1;
  int *v2;
  int v3;
  int v4;
  _DWORD *i;
  int v7;
  int v8;
  int v9;
  int v11;
  unsigned __int16 v12;
  const char *v13;
  char *v14;
  int *v15;
  int v16[4]; // [esp+Ch] [ebp-18h] BYREF
  _DWORD v17[2]; // [esp+1Ch] [ebp-8h] BYREF

  v1 = *(_DWORD *)(a1 + 152);
  if ( !v1 )
  {
    v7 = *(_DWORD *)(a1 + 144);
    if ( v7 )
    {
      v8 = *(_DWORD *)(a1 + 144);
      v17[0] = 0;
      v9 = DObjGetNumSurfaces(v8, (int)v17);
      /* retail 0x005047CE, sized off DObjGetNumSurfaces' count */
      v15 = (int *)alloca((4 * v9 + 7) & 0xFFFFFFF8);
      DObjGetSurfaces(v16, v7, v15, (int)v17);
      v11 = 0;
      if ( v9 > 0 )
      {
        while ( 1 )
        {
          v12 = *(_WORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(a1 + 144) + 4 * SLOWORD(v15[v11]) + 24) + 4)
                                     + 20 * v17[SLOWORD(v15[v11])]
                                     + 16)
                         + 2 * SHIWORD(v15[v11]));
          v13 = v12 ? (const char *)(GetRefString_var + 8 * v12 + 4) : "DEFAULT";
          v14 = va("skins/%s", v13);
          if ( (*((_DWORD *)R_FindShader(v14, -1, 1, (const char *)7) + 20) & 0x20000000) != 0 )
            break;
          if ( ++v11 >= v9 )
            return 0;
        }
        return 1;
      }
    }
    return 0;
  }
  v2 = *(int **)(v1 + 12);
  v3 = *v2;
  v4 = 0;
  if ( *v2 <= 0 )
    return 0;
  for ( i = v2 + 3; (*(_DWORD *)(*i + 80) & 0x20000000) == 0; i += 12 )
  {
    if ( ++v4 >= v3 )
      return 0;
  }
  return 1;
}

/* ---- R_AdjustBoundsForAutosprite  0x00504880 ----  VERIFIED */
int __cdecl R_AdjustBoundsForAutosprite(int a1, float *a2, float *a3)
{
  int result;
  double v4;
  double v5;
  double v6;
  double v7;
  double v8;
  double v9;
  long double v10;
  float v11;
  float v12;

  result = R_NeedsBoundsAdjustment(a1);
  if ( result )
  {
    v12 = a3[2] + a2[2];
    v11 = (*a2 + *a3) * 0.5;
    v4 = (a3[1] + a2[1]) * 0.5;
    v5 = v12 * 0.5;
    v6 = a3[2] - a2[2];
    v7 = a3[1] - a2[1];
    v8 = v6 * v6 + v7 * v7;
    v9 = *a3 - *a2;
    v10 = sqrt(v8 + v9 * v9) * 0.55000001 + 2.0;
    *a2 = v11 - v10;
    a2[1] = v4 - v10;
    a2[2] = v5 - v10;
    *a3 = v11 + v10;
    a3[1] = v10 + v4;
    a3[2] = v10 + v5;
  }
  return result;
}

/* ---- R_RegisterStaticModel  0x00504930 ----  VERIFIED */
_DWORD *__cdecl R_RegisterStaticModel(const char *a1, int a2)
{
  int v3;
  int i;
  _DWORD *v5;
  int v7;
  const char *v8;
  const char *v9;
  int v10;

  v3 = tr_registeredStaticModelCount;
  for ( i = 0; i < v3; ++i )
  {
    v5 = (_DWORD *)tr_registeredStaticModels[i];
    if ( *v5 && a1 )
    {
      if ( !Q_stricmpn(*(const char **)v5, a1, 99999) )
        return v5[2] != 0 ? v5 : 0;
      v3 = tr_registeredStaticModelCount;
    }
  }
  if ( v3 == 2048 )
  {
    ri_Printf(2, "R_RegisterStaticModel failed for '%s' -- more than %i unique static models\n", a1, 2048);
    return 0;
  }
  else
  {
    if ( tr_modelsFinishedLoading )
    {
      if ( r_errorOnConflicts->integer )
      {
        if ( !tr_ignorePrecacheErrorCount )
          ri_Error(1, "\x15" "model '%s' not precached\n", a1);
      }
    }
    else
    {
      tr_delayedImageGroup = ++tr_delayedImageGroupSequence;
    }
    v7 = R_SetupDObjToStaticModel(a2, a1);
    tr_delayedImageGroup = 0;
    tr_delayedImageGroupTriCount = 0;
    tr_registeredStaticModels[tr_registeredStaticModelCount++] = v7;
    return *(_DWORD *)(v7 + 8) != 0 ? (_DWORD *)v7 : 0;
  }
}

/* ---- R_CreateStaticModel  0x00504A20 ----  VERIFIED */
void __fastcall R_CreateStaticModel(int a1, char *a2, int a3, _DWORD *a4, float *a5, _DWORD *a6)
{
  char v8;
  int v9;
  char *v10;
  char v11;
  void *v12;
  int v13;
  _BYTE v14[88]; // [esp+Ch] [ebp-78h] BYREF -- DObj, 0x58 bytes
  int v15[4]; // [esp+64h] [ebp-20h] BYREF -- partBits
  staticDObjModel_t model = {0};
  __int16 v17;

  if ( !_strnicmp(a2, "xmodel", 6u) && ((v8 = a2[6], v8 == 47) || v8 == 92) )
  {
    if ( strlen(a2) < 0x39 )
    {
      R_SyncRenderThread();
      v9 = ri_Malloc(160);
      v10 = a2 + 7;
      do
      {
        v11 = *v10;
        v10[v9 - (_DWORD)(a2 + 7)] = *v10;
        ++v10;
      }
      while ( v11 );
      v17 = RE_RegisterModel(a2, (const char *)7);
      model.model = *(void **)(((int *)tr_models)[v17] + 84);
      model.modelIndex = v17;
      DObjCreate(0, (int)v14, (int)&model, 1u, 0);
      v12 = alloca(96 * v14[23] + 64);
      DObjBindEvalStorage((void *)( ( (unsigned int)v12 + 15 ) & ~15u ), v14);
      Com_Memset(v15, 255, 0x10u);
      DObjCalcAnim((int)v14, v15);
      DObjCalcSkel(v15, (int)v14);
      *(_DWORD *)(v9 + 64) = *a4;
      *(_DWORD *)(v9 + 68) = a4[1];
      *(_DWORD *)(v9 + 72) = a4[2];
      AnglesToAxis(a5, (float *)(v9 + 76));
      *(_DWORD *)(v9 + 112) = 0;
      if ( *(_DWORD *)a1 != 1065353216 || *(_DWORD *)(a1 + 4) != 1065353216 || *(_DWORD *)(a1 + 8) != 1065353216 )
      {
        *(float *)(v9 + 76) = *(float *)a1 * *(float *)(v9 + 76);
        *(float *)(v9 + 80) = *(float *)a1 * *(float *)(v9 + 80);
        *(float *)(v9 + 84) = *(float *)a1 * *(float *)(v9 + 84);
        *(float *)(v9 + 88) = *(float *)(a1 + 4) * *(float *)(v9 + 88);
        *(float *)(v9 + 92) = *(float *)(a1 + 4) * *(float *)(v9 + 92);
        *(float *)(v9 + 96) = *(float *)(a1 + 4) * *(float *)(v9 + 96);
        *(float *)(v9 + 100) = *(float *)(v9 + 100) * *(float *)(a1 + 8);
        *(float *)(v9 + 104) = *(float *)(v9 + 104) * *(float *)(a1 + 8);
        *(float *)(v9 + 108) = *(float *)(v9 + 108) * *(float *)(a1 + 8);
        *(float *)(v9 + 112) = VectorMax((float *)a1);
      }
      R_GetXModelBounds((float *)(v9 + 76), (int)v14, (float *)(v9 + 132), (float *)(v9 + 144));
      *(float *)(v9 + 132) = *(float *)(v9 + 64) + *(float *)(v9 + 132);
      *(float *)(v9 + 136) = *(float *)(v9 + 68) + *(float *)(v9 + 136);
      *(float *)(v9 + 140) = *(float *)(v9 + 140) + *(float *)(v9 + 72);
      *(float *)(v9 + 144) = *(float *)(v9 + 144) + *(float *)(v9 + 64);
      *(float *)(v9 + 148) = *(float *)(v9 + 68) + *(float *)(v9 + 148);
      *(float *)(v9 + 152) = *(float *)(v9 + 152) + *(float *)(v9 + 72);
      *(_DWORD *)(v9 + 116) = *a6;
      *(_DWORD *)(v9 + 120) = a6[1];
      v13 = (int)model.model;
      *(_DWORD *)(v9 + 124) = a6[2];
      *(_DWORD *)(v9 + 128) = 0;
      if ( (void *)*(_DWORD *)(v13 + 4) != (void *)xmodel_defaultCollision )
        *(_DWORD *)(v9 + 128) = R_RegisterStaticModel((const char *)v9, (int)v14);
      *(_DWORD *)(v9 + 156) = dword_14072EC;
      dword_14072EC = v9;
    }
    else
    {
      Com_Printf("Model '%s' has a name longer than %i characters\n", a2, 57);
    }
  }
  else
  {
    Com_Printf("Model '%s' is not an xmodel\n", a2);
  }
}

/* ---- R_AddStaticModelToWorld  0x00504CC0 ----  VERIFIED */
int __cdecl R_AddStaticModelToWorld(int a1)
{
  int v2;
  int v3;
  int v4;
  int v5;
  _DWORD **v6;
  int v7;
  char *v8;
  int v9;
  _DWORD *v10;
  int v11;
  int *v12;
  float v13;
  bool v14; // c0
  bool v15; // c3
  float v16;
  bool v17; // c0
  bool v18; // c3
  float v19;
  bool v20; // c0
  bool v21; // c3
  int result;
  staticDObjModel_t model = {0};
  __int16 v24;
  int v25[4]; // [esp+20h] [ebp-10h] BYREF
  int v26;
  float v27;
  float v28;
  float v29;
  float v30;
  float v31;
  float v32;

  v2 = *(_DWORD *)(a1 + 128);
  if ( v2 )
  {
    v3 = *(_DWORD *)(v2 + 4);
    v4 = 0;
    v5 = 2;
    v26 = 0;
    if ( v3 > 0 )
    {
      v6 = (_DWORD **)(v2 + 12);
      do
      {
        v4 += **v6++;
        --v3;
      }
      while ( v3 );
    }
  }
  else
  {
    v5 = 1;
    v7 = ri_Hunk_Alloc(88);
    v8 = va("xmodel/%s", (const char *)a1);
    v24 = RE_RegisterModel(v8, (const char *)7);
    model.model = *(void **)(tr_models[v24] + 84);
    model.modelIndex = v24;
    DObjCreate(0, v7, (int)&model, 1u, 0);
    v26 = v7;
    v9 = ri_Hunk_Alloc(96 * *(unsigned __int8 *)(v7 + 23) + 48);
    *(_DWORD *)(v7 + 4) = v9;
    v10 = (_DWORD *)(v9 + 32);
    v11 = 4;
    do
    {
      *(v10 - 8) = 0;
      *(v10 - 4) = 0;
      *v10++ = 0;
      --v11;
    }
    while ( v11 );
    Com_Memset(v25, 255, 0x10u);
    DObjCalcAnim(v7, v25);
    DObjCalcSkel(v25, v7);
    v4 = 0;
  }
  v12 = (int *)ri_Hunk_Alloc(4 * v4 + 584);
  memset(v12, 0, 0x9Cu);
  *v12 = v5;
  v12[36] = v26;
  v12[1] = 128;
  v12[38] = *(_DWORD *)(a1 + 128);
  v12[17] = *(_DWORD *)(a1 + 64);
  v12[18] = *(_DWORD *)(a1 + 68);
  v12[19] = *(_DWORD *)(a1 + 72);
  v12[7] = *(_DWORD *)(a1 + 76);
  v12[8] = *(_DWORD *)(a1 + 80);
  v12[9] = *(_DWORD *)(a1 + 84);
  v12[10] = *(_DWORD *)(a1 + 88);
  v12[11] = *(_DWORD *)(a1 + 92);
  v12[12] = *(_DWORD *)(a1 + 96);
  v12[13] = *(_DWORD *)(a1 + 100);
  v12[14] = *(_DWORD *)(a1 + 104);
  v12[15] = *(_DWORD *)(a1 + 108);
  v12[16] = *(_DWORD *)(a1 + 112);
  R_AdjustBoundsForAutosprite((int)v12, (float *)(a1 + 132), (float *)(a1 + 144));
  *((float *)v12 + 3) = *(float *)(a1 + 132) + *(float *)(a1 + 144);
  *((float *)v12 + 4) = *(float *)(a1 + 148) + *(float *)(a1 + 136);
  *((float *)v12 + 5) = *(float *)(a1 + 140) + *(float *)(a1 + 152);
  *((float *)v12 + 3) = *((float *)v12 + 3) * 0.5;
  *((float *)v12 + 4) = *((float *)v12 + 4) * 0.5;
  *((float *)v12 + 5) = *((float *)v12 + 5) * 0.5;
  v27 = tr_identityLight * *(float *)(a1 + 116) * 255.0;
  v13 = v27;
  if ( v27 >= 0.0 && (v27 > 255.0) | __UNORDERED__(255.0, v27) )
  {
    v28 = 255.0;
  }
  else
  {
    v14 = v27 > 0.0;
    v15 = 0.0 == v27;
    v28 = 0.0;
    if ( v14 || v15 )
      v28 = v13;
  }
  *((_BYTE *)v12 + 108) = (unsigned __int64)v28;
  v29 = tr_identityLight * *(float *)(a1 + 120) * 255.0;
  v16 = v29;
  if ( v29 >= 0.0 && (v29 > 255.0) | __UNORDERED__(255.0, v29) )
  {
    v30 = 255.0;
  }
  else
  {
    v17 = v29 > 0.0;
    v18 = 0.0 == v29;
    v30 = 0.0;
    if ( v17 || v18 )
      v30 = v16;
  }
  *((_BYTE *)v12 + 109) = (unsigned __int64)v30;
  v31 = tr_identityLight * *(float *)(a1 + 124) * 255.0;
  v19 = v31;
  if ( v31 >= 0.0 && (v31 > 255.0) | __UNORDERED__(255.0, v31) )
  {
    v32 = 255.0;
  }
  else
  {
    v20 = v31 > 0.0;
    v21 = 0.0 == v31;
    v32 = 0.0;
    if ( v20 || v21 )
      v32 = v19;
  }
  *((_BYTE *)v12 + 110) = (unsigned __int64)v32;
  *((_BYTE *)v12 + 111) = -1;
  v12[39] = *(_DWORD *)(a1 + 132);
  v12[40] = *(_DWORD *)(a1 + 136);
  v12[41] = *(_DWORD *)(a1 + 140);
  v12[42] = *(_DWORD *)(a1 + 144);
  v12[43] = *(_DWORD *)(a1 + 148);
  v12[44] = *(_DWORD *)(a1 + 152);
  v12[45] = 0;
  R_FilterModelIntoCells_r_0(tr_world, *(_DWORD **)(tr_world + 152), (int)v12, v12 + 39, v12 + 42);
  result = R_GetStaticLightContributions((float *)v12 + 3, (char *)v12 + 192, (float *)v12 + 47, (char *)v12 + 388);
  v12[46] = result;
  return result;
}

/* ---- R_FinishLoadingStaticModels  0x005050A0 ----  VERIFIED */
int R_FinishLoadingStaticModels()
{
  int result;
  int v1;
  int v2;
  char *v3;
  __int16 v4;
  int v5;
  int v6;
  _DWORD *v7;
  int v8;
  int v9;
  int v10;
  int i;
  staticDObjModel_t model = {0};
  int v14[4]; // [esp+20h] [ebp-70h] BYREF
  _BYTE v15[0x58]; // [esp+30h] [ebp-60h] BYREF -- sizeof(DObj)
  unsigned int v18;
  unsigned int retaddr;

  v18 = retaddr ^ _security_cookie;
  result = tr_registeredStaticModelCount;
  v1 = 0;
  for ( i = 0; v1 < tr_registeredStaticModelCount; i = v1 )
  {
    v2 = tr_registeredStaticModels[v1];
    if ( !*(_DWORD *)(v2 + 12) )
    {
      v3 = va("xmodel/%s", *(const char **)v2);
      v4 = RE_RegisterModel(v3, (const char *)7);
      v5 = tr_models[v4];
      model.modelIndex = v4;
      model.model = *(void **)(v5 + 84);
      model.tagName = NULL;
      DObjCreate(0, (int)v15, (int)&model, 1u, 0);
      v6 = ri_Hunk_AllocateTempMemory(96 * (unsigned __int8)v15[0x17] + 48);
      *(int *)&v15[0x04] = v6;
      v7 = (_DWORD *)(v6 + 32);
      v8 = 4;
      do
      {
        *(v7 - 8) = 0;
        *(v7 - 4) = 0;
        *v7++ = 0;
        --v8;
      }
      while ( v8 );
      Com_Memset(v14, 255, 0x10u);
      DObjCalcAnim((int)v15, v14);
      DObjCalcSkel(v14, (int)v15);
      R_FinishDObjToStaticModel((_DWORD *)tr_registeredStaticModels[i], (int)v15);
      ri_Hunk_FreeTempMemory(v6);
      v1 = i;
    }
    result = tr_registeredStaticModelCount;
    ++v1;
  }
  v9 = dword_14072EC;
  if ( dword_14072EC )
  {
    do
    {
      v10 = *(_DWORD *)(v9 + 156);
      R_AddStaticModelToWorld(v9);
      result = ri_Free(v9);
      v9 = v10;
    }
    while ( v10 );
  }
  dword_14072EC = 0;
  return result;
}

/* ---- R_ShutdownStaticModels  0x00505220 ----  VERIFIED */
int R_ShutdownStaticModels()
{
  int result;
  int v1;

  result = dword_14072EC;
  if ( dword_14072EC )
  {
    do
    {
      v1 = *(_DWORD *)(result + 156);
      ri_Free(result);
      result = v1;
    }
    while ( v1 );
  }
  dword_14072EC = 0;
  return result;
}

/* ---- R_AddStaticModelSurfaces  0x00505260 ----  VERIFIED */
char __cdecl R_AddStaticModelSurfaces(int a1)
{
  int v2;
  int v3;
  int v4;
  _DWORD **v5;
  int v6;
  _DWORD *v7;
  int v8;
  unsigned __int16 *v9;
  int v10;
  unsigned int v11;
  _DWORD *v12;
  _DWORD *v13;
  int v14;
  int v15;
  char *v16;
  float LodDist;
  int v19;
  int v20;
  int v21;
  int v22;
  _DWORD *v23;
  int v24;
  float v25[9]; // [esp+2Ch] [ebp-24h] BYREF

  v2 = *(_DWORD *)(a1 + 152);
  LodDist = R_GetLodDist((float *)a1);
  v3 = XModelGetLodForDist(*(int *)(v2 + 8), LodDist);
  if ( v3 >= 0 )
  {
    v4 = 0;
    v22 = 0;
    if ( v3 > 0 )
    {
      v5 = (_DWORD **)(v2 + 12);
      v6 = v3;
      do
      {
        v4 += **v5++;
        --v6;
      }
      while ( v6 );
      v22 = v4;
    }
    v7 = *(_DWORD **)(v2 + 4 * v3 + 12);
    v21 = 0;
    v24 = 0;
    v20 = 0;
    v23 = v7;
    v19 = 0;
    if ( (int)*v7 > 0 )
    {
      v8 = 4 * v4 + 584;
      v9 = (unsigned __int16 *)(v7 + 4);
      do
      {
        if ( !*(_DWORD *)(*(_DWORD *)(a1 + 688) + v8) )
        {
          if ( *(_DWORD *)v9 )
          {
            if ( !v21 )
            {
              MatrixInverse((float *)(a1 + 28), v25);
              R_SetupStaticModelLighting((int)&tr_refdef_x, a1);
              v21 = 1;
            }
            if ( !*(_BYTE *)(a1 + 160) )
              *(_DWORD *)(*(_DWORD *)(a1 + 688) + v8) = R_CacheStaticModelSurface(
                                                          *(float **)(a1 + 688),
                                                          (int)(v9 - 6),
                                                          v22 + v19,
                                                          a1,
                                                          v25);
          }
        }
        v10 = *(_DWORD *)(*(_DWORD *)(a1 + 688) + v8);
        if ( v10 )
        {
          v11 = (int)((unsigned __int64)(2411209711i64 * (v10 - (int)&r_smcCache)) >> 32) >> 8;
          v12 = (_DWORD *)((char *)&r_smcCache + 456 * (v11 >> 31) + 456 * v11);
          v13 = (_DWORD *)v12[1];
          v12[2] = tr_frameCount;
          *v13 = *v12;
          *(_DWORD *)(*v12 + 4) = v12[1];
          *v12 = &dword_11DA658;
          v12[1] = dword_11DA65C;
          dword_11DA65C = (int)v12;
          *(_DWORD *)v12[1] = v12;
          if ( (*(_BYTE *)(*((_DWORD *)v9 - 1) + 84) & 0x18) != 0
            && (R_SetupStaticModelLighting((int)&tr_refdef_x, a1), *(_BYTE *)(a1 + 160)) )
          {
            if ( tr_refdef_numDrawSurfs < 0x10000 )
            {
              *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs) = tr_shiftedEntityNumber
                                                             | ((*(_DWORD *)(*((_DWORD *)v9 - 1) + 72)
                                                               | (*((_DWORD *)v9 - 2) << 12)) << 18);
              *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs++ + 4) = v9 - 6;
            }
          }
          else
          {
            v14 = *(_DWORD *)(*(_DWORD *)(a1 + 688) + v8);
            if ( tr_refdef_numDrawSurfs < 0x10000 )
            {
              *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs) = tr_shiftedEntityNumber
                                                             | (((tr_cachedStaticModelStorageSource << 12) | *(_DWORD *)(*(_DWORD *)v9 + 72)) << 18);
              v7 = v23;
              *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs++ + 4) = v14;
            }
          }
        }
        else
        {
          if ( tr_refdef_numDrawSurfs < 0x10000 )
          {
            *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs) = tr_shiftedEntityNumber
                                                           | (((*((_DWORD *)v9 - 2) << 12)
                                                             | *(_DWORD *)(*((_DWORD *)v9 - 1) + 72)) << 18);
            *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs++ + 4) = v9 - 6;
          }
          if ( (*(_BYTE *)(*((_DWORD *)v9 - 1) + 84) & 0x18) != 0 )
            v24 = 1;
        }
        v20 += v9[2];
        v8 += 4;
        v9 += 24;
        ++v19;
      }
      while ( v19 < *v7 );
    }
    if ( !r_showtricounts->integer || (*(_BYTE *)(a1 + 4) & 8) != 0 )
    {
      if ( !r_showsurfcounts->integer || (*(_BYTE *)(a1 + 4) & 8) != 0 )
        goto LABEL_33;
      v15 = *v7;
    }
    else
    {
      v15 = v20 / 3;
    }
    v16 = va("%i", v15);
    R_AddScaledDebugString(a1 + 68, modelBoundsDebugColor, v16);
LABEL_33:
    LOBYTE(v3) = v24;
    if ( v24 )
      LOBYTE(v3) = R_SetupStaticModelLighting((int)&tr_refdef_x, a1);
  }
  return v3;
}

/* ---- RB_SurfaceStaticModel  0x00505580 ----  VERIFIED */
int __cdecl RB_SurfaceStaticModel(int a1)
{
  char *v1;
  int v2;
  int result;

  if ( r_logFile->integer )
  {
    v1 = va("--- RB_SurfaceStaticModel( %s ) ---\n", (const char *)tess_shader);
    if ( Stream )
      fprintf(Stream, "%s", (int)v1);
  }
  Com_Memcpy(4 * tess_numVertexes * tess_vertexComponentCount + ((int)(char *)tess_xyz), *(int **)(a1 + 28), 12 * *(unsigned __int16 *)(a1 + 18));
  Com_Memcpy(8 * tess_numVertexes + ((int)(char *)tess_texCoords0), *(int **)(a1 + 20), 8 * *(unsigned __int16 *)(a1 + 18));
  if ( (*(_DWORD *)(tess_shader + 80) & 0x102) != 0 )
  {
    Com_Memcpy(12 * tess_numVertexes + ((int)(char *)tess_stageNormals), *(int **)(a1 + 24), 12 * *(unsigned __int16 *)(a1 + 18));
    tess_requiresVertexBasis = 1;
  }
  v2 = 0;
  if ( *(_WORD *)(a1 + 16) )
  {
    do
    {
      tess_indexes[v2 + tess_numIndexes] = tess_numVertexes + *(_WORD *)(*(_DWORD *)(a1 + 32) + 2 * v2);
      ++v2;
    }
    while ( v2 < *(unsigned __int16 *)(a1 + 16) );
  }
  tess_numIndexes += *(unsigned __int16 *)(a1 + 16);
  result = *(unsigned __int16 *)(a1 + 18) + tess_numVertexes;
  tess_numVertexes = result;
  return result;
}

/* ---- RB_SurfaceStaticModelCached  0x005056A0 ----  VERIFIED */
int __cdecl RB_SurfaceStaticModelCached(int a1)
{
  char *v1;
  int *v2;
  int *v3;
  int v4;
  int v5;
  int v6;
  int result;
  _DWORD *v8;
  _DWORD *v9;
  int v10;
  _DWORD *v11;
  _DWORD *v12;

  if ( r_logFile->integer )
  {
    v1 = va("--- RB_SurfaceStaticModelCached( %s ) ---\n", (const char *)tess_shader);
    if ( Stream )
      fprintf(Stream, "%s", (int)v1);
  }
  v2 = (int *)(a1 + 8);
  if ( tess_optimizedVertexEnd )
  {
    if ( tess_optimizedFirstVertex > *v2 )
      tess_optimizedFirstVertex = *v2;
    v3 = (int *)(a1 + 4);
    if ( tess_optimizedVertexEnd < *v2 + *(unsigned __int16 *)(*(_DWORD *)(a1 + 4) + 18) )
      tess_optimizedVertexEnd = *v2 + *(unsigned __int16 *)(*(_DWORD *)(a1 + 4) + 18);
  }
  else
  {
    tess_optimizedFirstVertex = *v2;
    v3 = (int *)(a1 + 4);
    tess_optimizedVertexEnd = *v2 + *(unsigned __int16 *)(*(_DWORD *)(a1 + 4) + 18);
  }
  v4 = *v3;
  v5 = *(unsigned __int16 *)(*v3 + 16);
  v6 = v5 / 6;
  result = *v2 | (*v2 << 16);
  v8 = *(_DWORD **)(v4 + 32);
  v9 = (_DWORD *)((char *)&tess_optimizedIndexes + 2 * count);
  count += v5;
  do
  {
    *v9 = result + *v8;
    v10 = v8[1];
    v11 = v8 + 1;
    v12 = v9 + 1;
    *v12++ = result + v10;
    *v12 = result + v11[1];
    v9 = v12 + 1;
    v8 = v11 + 2;
    --v6;
  }
  while ( v6 );
  return result;
}

/* ---- RB_SurfaceStaticModelT2V3_ARB  0x005057A0 ----  [HIGH] */
int __cdecl RB_SurfaceStaticModelT2V3_ARB(int a1)
{
  float v;      /* written before every read; retail spills it at
                 * [esp+2Ch+var_14] */
  void *v4;
  int v5;
  _DWORD *v6;
  int v7;
  GLsizei v8;
  int integer;
  int v10;
  GLenum *AnimatedImage;
  GLsizei v12;
  int result;
  _DWORD v14[5]; // [esp+2Ch] [ebp-14h] BYREF

  if ( Stream )
    fprintf(Stream, "%s", (int)"--- RB_SurfaceStaticModelT2V3_ARB ---\n");
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1025);
  qglBindBufferARB(34962, *(_DWORD *)(a1 + 36));
  qglBindBufferARB(34963, *(_DWORD *)(a1 + 40));
  glVertexPointer(3, 0x1406u, 20, (const GLvoid *)8);
  v5 = 0;
  for ( v14[0] = 0; v5 < tess_activeStageCount; ++v5 )
  {
    v6 = *(_DWORD **)(tess_activeStages + 4 * v5);
    GL_State(v6[417]);
    RB_SetupMultitexture(v6, (int)v14, 20);
    v7 = *(_DWORD *)(tess_activeStages + 4 * v5);
    if ( *(_DWORD *)(v7 + 1636) == 11 )
    {
      if ( r_entFullbright->integer )
        qglColor3f(tr_identityLight, tr_identityLight, tr_identityLight);
      else
        glColor4ubv((const GLubyte *)(backEnd_currentEntity + 108));
    }
    else
    {
      glColor4ubv((const GLubyte *)(v7 + 1664));
    }
    v8 = *(unsigned __int16 *)(a1 + 16);
    backEnd_pc_drawnIndexCount += v8;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v8, 0x1403u, 0);
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_State(*(_DWORD *)(*(_DWORD *)(tr_showTrisShader + 340) + 1668));
    AnimatedImage = (GLenum *)
      RB_GetAnimatedImage(0, *(_DWORD *)(tr_showTrisShader + 340) + 4, glState_currentTmu);
    GL_Bind(AnimatedImage);
    if ( r_showtris->integer < 5 )
    {
      v = tr_identityLight * 0.25;
      qglColor3f(v, v, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(a1 + 16), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v12 = *(unsigned __int16 *)(a1 + 16);
    backEnd_pc_drawnIndexCount += v12;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v12, 0x1403u, 0);
    glDepthRange(0.0, 1.0);
  }
  qglBindBufferARB(34962, 0);
  qglBindBufferARB(34963, 0);
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(unsigned __int16 *)(a1 + 16);
  result = *(unsigned __int16 *)(a1 + 18) + backEnd_pc_vertexCount;
  backEnd_pc_vertexCount = result;
  return result;
}

/* ---- RB_SurfaceStaticModelT2N3V3_ARB  0x00505A30 ----  [HIGH] */
int __cdecl RB_SurfaceStaticModelT2N3V3_ARB(int a1)
{
  float v;
  void *v4;
  int v5;
  _DWORD *v6;
  GLsizei v7;
  int integer;
  int v9;
  GLenum *AnimatedImage;
  GLsizei v11;
  int result;
  _DWORD v13[5]; // [esp+38h] [ebp-14h] BYREF

  if ( Stream )
    fprintf(Stream, "%s", (int)"--- RB_SurfaceStaticModelT2N3V3_ARB ---\n");
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1537);
  qglBindBufferARB(34962, *(_DWORD *)(a1 + 36));
  qglBindBufferARB(34963, *(_DWORD *)(a1 + 40));
  glNormalPointer(0x1406u, 32, (const GLvoid *)8);
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)0x14);
  v5 = 0;
  for ( v13[0] = 0; v5 < tess_activeStageCount; ++v5 )
  {
    v6 = *(_DWORD **)(tess_activeStages + 4 * v5);
    GL_State(v6[417]);
    RB_SetupMultitexture(v6, (int)v13, 32);
    v7 = *(unsigned __int16 *)(a1 + 16);
    backEnd_pc_drawnIndexCount += v7;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v7, 0x1403u, 0);
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_State(*(_DWORD *)(*(_DWORD *)(tr_showTrisShader + 340) + 1668));
    AnimatedImage = (GLenum *)
      RB_GetAnimatedImage(0, *(_DWORD *)(tr_showTrisShader + 340) + 4, glState_currentTmu);
    GL_Bind(AnimatedImage);
    if ( r_showtris->integer < 5 )
    {
      v = tr_identityLight * 0.25;
      qglColor3f(v, v, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(a1 + 16), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v11 = *(unsigned __int16 *)(a1 + 16);
    backEnd_pc_drawnIndexCount += v11;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v11, 0x1403u, 0);
    glDepthRange(0.0, 1.0);
  }
  qglBindBufferARB(34962, 0);
  qglBindBufferARB(34963, 0);
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(unsigned __int16 *)(a1 + 16);
  result = *(unsigned __int16 *)(a1 + 18) + backEnd_pc_vertexCount;
  backEnd_pc_vertexCount = result;
  return result;
}

/* ---- RB_SurfaceStaticModelATI  0x00505C80 ----  VERIFIED */
int __cdecl RB_SurfaceStaticModelATI(int v)
{
  FILE *integer;
  char *v2;
  int v3;
  bool v4; // zf
  int v5;
  int v6;
  int i;
  int v8;
  _DWORD *v9;
  int v10;
  int v11;
  _DWORD *v12;
  int v13;
  int j;
  int v15;
  const GLubyte *v16;
  _DWORD *v17;
  int v18;
  GLsizei v19;
  int v20;
  _DWORD *v21;
  int v22;
  GLsizei v23;
  int result;
  float v25;
  float v26;
  float v27;
  float v28;
  int v29;
  int v30;
  int v31;
  const GLvoid *v32;
  const GLvoid *v33;
  int v34;
  int v35;
  _DWORD v36[8]; // [esp+84h] [ebp-20h] BYREF

  integer = (FILE *)r_logFile->integer;
  if ( integer )
  {
    v2 = va("--- RB_SurfaceStaticModelATI( %s ) ---\n", (const char *)tess_shader);
    integer = Stream;
    if ( Stream )
      fprintf(Stream, "%s", (int)v2);
  }
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1025);
  v3 = v;
  v4 = *(_DWORD *)v == 18;
  v5 = *(_DWORD *)(v + 40);
  v36[0] = v5;
  if ( v4 )
  {
    v35 = v5 + 8;
    v34 = *(_DWORD *)(v + 36);
    v = 20;
    v29 = 20;
  }
  else
  {
    v6 = *(_DWORD *)(v + 36);
    v = 32;
    qglArrayObjectATI(32885, 3, 5126, 32, v6, v5 + 8);
    v35 = *(_DWORD *)(v3 + 40) + 20;
    v34 = *(_DWORD *)(v3 + 36);
    v29 = 32;
  }
  qglArrayObjectATI(32884, 3, 5126, v29, v34, v35);
  if ( glConfig_ATIElementArray )
  {
    glEnableClientState(0x8768u);
    qglArrayObjectATI(34664, 1, 5123, 0, *(_DWORD *)(v3 + 36), *(_DWORD *)(v3 + 44));
    for ( i = 0; i < tess_activeStageCount; ++i )
    {
      if ( *(_DWORD *)v3 == 18 )
      {
        v8 = *(_DWORD *)(tess_activeStages + 4 * i);
        if ( *(_DWORD *)(v8 + 1636) == 11 )
        {
          if ( r_entFullbright->integer )
            qglColor3f(tr_identityLight, tr_identityLight, tr_identityLight);
          else
            glColor4ubv((const GLubyte *)(backEnd_currentEntity + 108));
        }
        else
        {
          glColor4ubv((const GLubyte *)(v8 + 1664));
        }
      }
      v9 = *(_DWORD **)(tess_activeStages + 4 * i);
      v10 = *(_DWORD *)(v3 + 36);
      GL_State(v9[417]);
      RB_SetupMultitextureATI(v9, v10, (int)v36, v);
      v30 = *(unsigned __int16 *)(v3 + 16);
      backEnd_pc_drawnIndexCount += v30;
      ++backEnd_pc_drawCallCount;
      glDrawElementArrayATI(4, v30);
    }
    v11 = r_showtris->integer;
    if ( v11 )
    {
      if ( (v11 & 1) != 0 )
        glDepthRange(0.0, 0.1000000014901161);
      v12 = *(_DWORD **)(tr_showTrisShader + 340);
      v13 = *(_DWORD *)(v3 + 36);
      GL_State(v12[417]);
      RB_SetupMultitextureATI(v12, v13, (int)v36, v);
      if ( r_showtris->integer < 5 )
      {
        v27 = tr_identityLight * 0.75;
        v25 = tr_identityLight * 0.5;
        qglColor3f(v25, v27, tr_identityLight);
      }
      else
      {
        RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(v3 + 16), &v);
        glColor4ubv((const GLubyte *)&v);
      }
      v31 = *(unsigned __int16 *)(v3 + 16);
      backEnd_pc_drawnIndexCount += v31;
      ++backEnd_pc_drawCallCount;
      glDrawElementArrayATI(4, v31);
      glDepthRange(0.0, 1.0);
    }
    glDisableClientState(0x8768u);
    goto LABEL_40;
  }
  for ( j = 0; j < tess_activeStageCount; ++j )
  {
    if ( *(_DWORD *)v3 == 18 )
    {
      v15 = *(_DWORD *)(tess_activeStages + 4 * j);
      if ( *(_DWORD *)(v15 + 1636) != 11 )
      {
        v16 = (const GLubyte *)(v15 + 1664);
        goto LABEL_31;
      }
      if ( !r_entFullbright->integer )
      {
        v16 = (const GLubyte *)(backEnd_currentEntity + 108);
LABEL_31:
        glColor4ubv(v16);
        goto LABEL_32;
      }
      qglColor3f(tr_identityLight, tr_identityLight, tr_identityLight);
    }
LABEL_32:
    v17 = *(_DWORD **)(tess_activeStages + 4 * j);
    v18 = *(_DWORD *)(v3 + 36);
    GL_State(v17[417]);
    RB_SetupMultitextureATI(v17, v18, (int)v36, v);
    v19 = *(unsigned __int16 *)(v3 + 16);
    v32 = *(const GLvoid **)(v3 + 32);
    backEnd_pc_drawnIndexCount += v19;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v19, 0x1403u, v32);
  }
  v20 = r_showtris->integer;
  if ( v20 )
  {
    if ( (v20 & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    v21 = *(_DWORD **)(tr_showTrisShader + 340);
    v22 = *(_DWORD *)(v3 + 36);
    GL_State(v21[417]);
    RB_SetupMultitextureATI(v21, v22, (int)v36, v);
    if ( r_showtris->integer < 5 )
    {
      v28 = tr_identityLight * 0.75;
      v26 = tr_identityLight * 0.5;
      qglColor3f(v26, v28, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(v3 + 16), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v23 = *(unsigned __int16 *)(v3 + 16);
    v33 = *(const GLvoid **)(v3 + 32);
    backEnd_pc_drawnIndexCount += v23;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v23, 0x1403u, v33);
    glDepthRange(0.0, 1.0);
  }
LABEL_40:
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(unsigned __int16 *)(v3 + 16);
  result = *(unsigned __int16 *)(v3 + 18);
  backEnd_pc_vertexCount += result;
  return result;
}

/* ---- RB_SurfaceStaticModelT2V3_NV  0x00506130 ----  VERIFIED */
int __cdecl RB_SurfaceStaticModelT2V3_NV(int v)
{
  void *v1;
  int v2;
  int v3;
  int v4;
  _DWORD *v5;
  GLsizei v6;
  int integer;
  _DWORD *v8;
  GLsizei v9;
  int result;
  float v11;
  float v12;
  const GLvoid *v13;
  const GLvoid *v14;
  _DWORD v15[8]; // [esp+4Ch] [ebp-20h] BYREF

  if ( Stream )
    fprintf(Stream, "%s", (int)"--- RB_SurfaceStaticModelT2V3_NV ---\n");
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1025);
  v2 = v;
  glVertexPointer(3, 0x1406u, 20, (const GLvoid *)(*(_DWORD *)(v + 36) + 8));
  v3 = 0;
  for ( v15[0] = *(_DWORD *)(v2 + 36); v3 < tess_activeStageCount; ++v3 )
  {
    v4 = *(_DWORD *)(tess_activeStages + 4 * v3);
    if ( *(_DWORD *)(v4 + 1636) == 11 )
    {
      if ( r_entFullbright->integer )
        qglColor3f(tr_identityLight, tr_identityLight, tr_identityLight);
      else
        glColor4ubv((const GLubyte *)(backEnd_currentEntity + 108));
    }
    else
    {
      glColor4ubv((const GLubyte *)(v4 + 1664));
    }
    v5 = *(_DWORD **)(tess_activeStages + 4 * v3);
    GL_State(v5[417]);
    RB_SetupMultitexture(v5, (int)v15, 20);
    v6 = *(unsigned __int16 *)(v2 + 16);
    v13 = *(const GLvoid **)(v2 + 32);
    backEnd_pc_drawnIndexCount += v6;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v6, 0x1403u, v13);
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    v8 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v8[417]);
    RB_SetupMultitexture(v8, (int)v15, 20);
    if ( r_showtris->integer < 5 )
    {
      v12 = tr_identityLight * 0.75;
      v11 = tr_identityLight * 0.5;
      qglColor3f(v11, v12, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(v2 + 16), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v9 = *(unsigned __int16 *)(v2 + 16);
    v14 = *(const GLvoid **)(v2 + 32);
    backEnd_pc_drawnIndexCount += v9;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v9, 0x1403u, v14);
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(unsigned __int16 *)(v2 + 16);
  result = *(unsigned __int16 *)(v2 + 18);
  backEnd_pc_vertexCount += result;
  return result;
}

/* ---- RB_SurfaceStaticModelT2N3V3_Generic  0x00506390 ----  VERIFIED */
int __cdecl RB_SurfaceStaticModelT2N3V3_Generic(int a1)
{
  float v;
  void *v1;
  int v2;  /* the surface pointer */
  int i;
  _DWORD *v4;
  int integer;
  GLenum *AnimatedImage;
  int result;
  GLsizei v8;
  GLsizei v9;
  const GLvoid *v10;
  const GLvoid *v11;
  _DWORD v12[8]; // [esp+1Ch] [ebp-20h] BYREF

  if ( Stream )
    fprintf(Stream, "%s", (int)"--- RB_SurfaceStaticModelT2N3V3_Generic ---\n");
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1537);
  v2 = a1;
  glNormalPointer(0x1406u, 32, (const GLvoid *)(*(_DWORD *)(a1 + 36) + 8));
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)(*(_DWORD *)(v2 + 36) + 20));
  v12[0] = *(_DWORD *)(v2 + 36);
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v4 = *(_DWORD **)(tess_activeStages + 4 * i);
    GL_State(v4[417]);
    RB_SetupMultitexture(v4, (int)v12, 32);
    v10 = *(const GLvoid **)(v2 + 32);
    v8 = *(unsigned __int16 *)(v2 + 16);
    backEnd_pc_drawnIndexCount += v8;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v8, 0x1403u, v10);
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_State(*(_DWORD *)(*(_DWORD *)(tr_showTrisShader + 340) + 1668));
    AnimatedImage = (GLenum *)RB_GetAnimatedImage(glState_currentTmu, *(_DWORD *)(tr_showTrisShader + 340) + 4, glState_currentTmu);
    GL_Bind(AnimatedImage);
    if ( r_showtris->integer < 5 )
    {
      v = tr_identityLight * 0.25;
      qglColor3f(v, v, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(v2 + 16), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v11 = *(const GLvoid **)(v2 + 32);
    v9 = *(unsigned __int16 *)(v2 + 16);
    backEnd_pc_drawnIndexCount += v9;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v9, 0x1403u, v11);
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(unsigned __int16 *)(v2 + 16);
  result = *(unsigned __int16 *)(v2 + 18);
  backEnd_pc_vertexCount += result;
  return result;
}

/* ---- RB_SurfaceStaticModelT2V3_Generic  0x005065C0 ----  VERIFIED */
int __cdecl RB_SurfaceStaticModelT2V3_Generic(int v)
{
  void *v1;
  int v2;
  int v3;
  int v4;
  _DWORD *v5;
  GLsizei v6;
  int integer;
  _DWORD *v8;
  GLsizei v9;
  int result;
  float v11;
  float v12;
  const GLvoid *v13;
  const GLvoid *v14;
  _DWORD v15[8]; // [esp+4Ch] [ebp-20h] BYREF

  if ( Stream )
    fprintf(Stream, "%s", (int)"--- RB_SurfaceStaticModelT2V3_Generic ---\n");
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1025);
  v2 = v;
  glVertexPointer(3, 0x1406u, 20, (const GLvoid *)(*(_DWORD *)(v + 36) + 8));
  v3 = 0;
  for ( v15[0] = *(_DWORD *)(v2 + 36); v3 < tess_activeStageCount; ++v3 )
  {
    v4 = *(_DWORD *)(tess_activeStages + 4 * v3);
    if ( *(_DWORD *)(v4 + 1636) == 11 )
    {
      if ( r_entFullbright->integer )
        qglColor3f(tr_identityLight, tr_identityLight, tr_identityLight);
      else
        glColor4ubv((const GLubyte *)(backEnd_currentEntity + 108));
    }
    else
    {
      glColor4ubv((const GLubyte *)(v4 + 1664));
    }
    v5 = *(_DWORD **)(tess_activeStages + 4 * v3);
    GL_State(v5[417]);
    RB_SetupMultitexture(v5, (int)v15, 20);
    v6 = *(unsigned __int16 *)(v2 + 16);
    v13 = *(const GLvoid **)(v2 + 32);
    backEnd_pc_drawnIndexCount += v6;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v6, 0x1403u, v13);
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    v8 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v8[417]);
    RB_SetupMultitexture(v8, (int)v15, 20);
    if ( r_showtris->integer < 5 )
    {
      v12 = tr_identityLight * 0.75;
      v11 = tr_identityLight * 0.5;
      qglColor3f(v11, v12, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(v2 + 16), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v9 = *(unsigned __int16 *)(v2 + 16);
    v14 = *(const GLvoid **)(v2 + 32);
    backEnd_pc_drawnIndexCount += v9;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v9, 0x1403u, v14);
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(unsigned __int16 *)(v2 + 16);
  result = *(unsigned __int16 *)(v2 + 18);
  backEnd_pc_vertexCount += result;
  return result;
}

/* ---- RB_SurfaceStaticModelT2N3V3_NV  0x00506820 ----  VERIFIED */
int __cdecl RB_SurfaceStaticModelT2N3V3_NV(int a1)
{
  float v;
  void *v1;
  int v2;  /* the surface pointer */
  int i;
  _DWORD *v4;
  int integer;
  GLenum *AnimatedImage;
  int result;
  GLsizei v8;
  GLsizei v9;
  const GLvoid *v10;
  const GLvoid *v11;
  _DWORD v12[8]; // [esp+1Ch] [ebp-20h] BYREF

  if ( Stream )
    fprintf(Stream, "%s", (int)"--- RB_SurfaceStaticModelT2N3V3_NV ---\n");
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1537);
  v2 = a1;
  glNormalPointer(0x1406u, 32, (const GLvoid *)(*(_DWORD *)(a1 + 36) + 8));
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)(*(_DWORD *)(v2 + 36) + 20));
  v12[0] = *(_DWORD *)(v2 + 36);
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v4 = *(_DWORD **)(tess_activeStages + 4 * i);
    GL_State(v4[417]);
    RB_SetupMultitexture(v4, (int)v12, 32);
    v10 = *(const GLvoid **)(v2 + 32);
    v8 = *(unsigned __int16 *)(v2 + 16);
    backEnd_pc_drawnIndexCount += v8;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v8, 0x1403u, v10);
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_State(*(_DWORD *)(*(_DWORD *)(tr_showTrisShader + 340) + 1668));
    AnimatedImage = (GLenum *)RB_GetAnimatedImage(glState_currentTmu, *(_DWORD *)(tr_showTrisShader + 340) + 4, glState_currentTmu);
    GL_Bind(AnimatedImage);
    if ( r_showtris->integer < 5 )
    {
      v = tr_identityLight * 0.25;
      qglColor3f(v, v, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(*(unsigned __int16 *)(v2 + 16), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v11 = *(const GLvoid **)(v2 + 32);
    v9 = *(unsigned __int16 *)(v2 + 16);
    backEnd_pc_drawnIndexCount += v9;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v9, 0x1403u, v11);
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(unsigned __int16 *)(v2 + 16);
  result = *(unsigned __int16 *)(v2 + 18);
  backEnd_pc_vertexCount += result;
  return result;
}

/* ---- R_RefreshStaticModels_ARB  0x00506A50 ----  VERIFIED */
int __cdecl R_RefreshStaticModels_ARB(char a1)
{
  int result;
  int *v2;
  int *v3;
  _DWORD *v4;
  int v5;
  _DWORD *v6;
  _DWORD *v7;
  char *v8;
  int v9;
  _DWORD *v10;
  _DWORD *v11;
  _DWORD *v12;
  char *v13;
  int v14;
  bool v15; // cc
  int v16;
  char *v17;
  int v18;
  int v19;
  int *v20;
  int v21;
  int v22;
  int v23;
  int *v24;
  char v25[8192 * 20]; // [esp+2Ch] [ebp-67FF0h] BYREF -- T2V3 staging
  char v27[8192 * 32]; // [esp+2802Ch] [ebp-3FFF0h] BYREF -- T2N3V3 staging

  result = tr_registeredStaticModelCount;
  v19 = 0;
  if ( tr_registeredStaticModelCount > 0 )
  {
    v2 = tr_registeredStaticModels;
    v20 = tr_registeredStaticModels;
    do
    {
      v22 = 0;
      if ( *(int *)(*v2 + 4) > 0 )
      {
        v18 = 12;
        do
        {
          v3 = *(int **)(v18 + *v2);
          v24 = v3;
          v21 = 0;
          if ( *v3 > 0 )
          {
            v4 = v3 + 11;
            do
            {
              if ( *(v4 - 1) )
              {
                if ( *v4 )
                {
                  if ( (a1 & 2) != 0 )
                  {
                    qglBindBufferARB(34963, *v4);
                    qglBufferDataARB(34963, 2 * *((unsigned __int16 *)v4 - 12), *(v4 - 2), 35044);
                    qglBindBufferARB(34963, 0);
                  }
                  if ( (a1 & 1) != 0 )
                  {
                    qglBindBufferARB(34962, *(v4 - 1));
                    if ( *(v4 - 10) == 17 )
                    {
                      v5 = *((unsigned __int16 *)v4 - 11);
                      if ( *((_WORD *)v4 - 11) )
                      {
                        v6 = (_DWORD *)*(v4 - 3);
                        v7 = (_DWORD *)*(v4 - 5);
                        v8 = &v25[4];   /* ebp-0x67FEC */
                        do
                        {
                          *((_DWORD *)v8 - 1) = *v7;
                          *(_DWORD *)v8 = v7[1];
                          *((_DWORD *)v8 + 1) = *v6;
                          *((_DWORD *)v8 + 2) = v6[1];
                          *((_DWORD *)v8 + 3) = v6[2];
                          v7 += 2;
                          v6 += 3;
                          v8 += 20;
                          --v5;
                        }
                        while ( v5 );
                      }
                      v17 = v25;
                      v16 = 20 * *((unsigned __int16 *)v4 - 11);
                    }
                    else
                    {
                      if ( *((_WORD *)v4 - 11) )
                      {
                        v9 = *(v4 - 4);
                        v10 = (_DWORD *)*(v4 - 3);
                        v11 = (_DWORD *)*(v4 - 5);
                        v12 = (_DWORD *)(v9 + 8);
                        v13 = &v27[4];  /* ebp-0x3FFEC */
                        v14 = v9 - (_DWORD)v10;
                        v23 = *((unsigned __int16 *)v4 - 11);
                        do
                        {
                          *((_DWORD *)v13 - 1) = *v11;
                          *(_DWORD *)v13 = v11[1];
                          *((_DWORD *)v13 + 1) = *(_DWORD *)((char *)v10 + v14);
                          *((_DWORD *)v13 + 2) = *(v12 - 1);
                          *((_DWORD *)v13 + 3) = *v12;
                          *((_DWORD *)v13 + 4) = *v10;
                          *((_DWORD *)v13 + 5) = v10[1];
                          *((_DWORD *)v13 + 6) = v10[2];
                          v11 += 2;
                          v12 += 3;
                          v10 += 3;
                          v13 += 32;
                          --v23;
                        }
                        while ( v23 );
                        v3 = v24;
                      }
                      v17 = v27;
                      v16 = 32 * *((unsigned __int16 *)v4 - 11);
                    }
                    qglBufferDataARB(34962, v16, v17, 35044);
                    qglBindBufferARB(34962, 0);
                    v2 = v20;
                  }
                }
              }
              v4 += 12;
              ++v21;
            }
            while ( v21 < *v3 );
          }
          v15 = ++v22 < *(_DWORD *)(*v2 + 4);
          v18 += 4;
        }
        while ( v15 );
      }
      result = v19 + 1;
      ++v2;
      v15 = ++v19 < tr_registeredStaticModelCount;
      v20 = v2;
    }
    while ( v15 );
  }
  return result;
}

/* ---- R_IncrementalRefreshStaticModels_ARB  0x00506C80 ----  VERIFIED */
void __cdecl R_IncrementalRefreshStaticModels_ARB(char a1)
{
  int v1;
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD *v7;
  _DWORD *v8;
  _DWORD *v9;
  int v10;
  int v11;
  int v12;
  _DWORD *v13;
  _DWORD *v14;
  _DWORD *v15;
  _DWORD *v16;
  int v17;
  int v18;
  int v19;
  _BYTE v20[262144]; // [esp+18h] BYREF

  if ( tr_registeredStaticModelCount && !dword_14072EC )
  {
    v1 = tr_staticModelRefreshModelIndex;
    v2 = tr_staticModelRefreshLodIndex;
    v3 = tr_staticModelRefreshSurfaceIndex;
    v4 = tr_staticModelRefreshModelIndex;
    while ( 1 )
    {
      tr_staticModelRefreshSurfaceIndex = ++v3;
      if ( v3 >= **(_DWORD **)(tr_registeredStaticModels[v1] + 4 * v2 + 12) )
      {
        v3 = 0;
        ++v2;
        tr_staticModelRefreshSurfaceIndex = 0;
        tr_staticModelRefreshLodIndex = v2;
        if ( v2 >= *(_DWORD *)(tr_registeredStaticModels[v1] + 4) )
        {
          v2 = 0;
          ++v1;
          tr_staticModelRefreshLodIndex = 0;
          tr_staticModelRefreshModelIndex = v1;
          if ( v1 >= tr_registeredStaticModelCount )
          {
            v1 = 0;
            tr_staticModelRefreshModelIndex = 0;
          }
          if ( v1 == v4 )
            break;
        }
      }
      v5 = *(_DWORD *)(tr_registeredStaticModels[v1] + 4 * v2 + 12) + 48 * v3 + 4;
      if ( *(_DWORD *)(*(_DWORD *)(tr_registeredStaticModels[v1] + 4 * v2 + 12) + 48 * v3 + 40)
        && *(_DWORD *)(*(_DWORD *)(tr_registeredStaticModels[v1] + 4 * v2 + 12) + 48 * v3 + 44) )
      {
        if ( (a1 & 2) != 0 )
        {
          qglBindBufferARB(34963, *(_DWORD *)(*(_DWORD *)(tr_registeredStaticModels[v1] + 4 * v2 + 12) + 48 * v3 + 44));
          qglBufferDataARB(34963, 2 * *(unsigned __int16 *)(v5 + 16), 0, 35044);
          qglBufferDataARB(34963, 2 * *(unsigned __int16 *)(v5 + 16), *(_DWORD *)(v5 + 32), 35044);
          qglBindBufferARB(34963, 0);
        }
        if ( (a1 & 1) != 0 )
        {
          qglBindBufferARB(34962, *(_DWORD *)(v5 + 36));
          if ( *(_DWORD *)v5 == 17 )
          {
            v6 = *(unsigned __int16 *)(v5 + 18);
            if ( *(_WORD *)(v5 + 18) )
            {
              v7 = *(_DWORD **)(v5 + 28);
              v8 = *(_DWORD **)(v5 + 20);
              v9 = (_DWORD *)(v20 + 4);
              v10 = *(unsigned __int16 *)(v5 + 18);
              do
              {
                *(v9 - 1) = *v8;
                *v9 = v8[1];
                v9[1] = *v7;
                v9[2] = v7[1];
                v9[3] = v7[2];
                v8 += 2;
                v7 += 3;
                v9 += 5;
                --v10;
              }
              while ( v10 );
            }
            qglBufferDataARB(34962, 20 * v6, 0, 35044);
            v18 = 20 * *(unsigned __int16 *)(v5 + 18);
          }
          else
          {
            v11 = *(unsigned __int16 *)(v5 + 18);
            if ( *(_WORD *)(v5 + 18) )
            {
              v12 = *(_DWORD *)(v5 + 24);
              v13 = *(_DWORD **)(v5 + 28);
              v14 = *(_DWORD **)(v5 + 20);
              v15 = (_DWORD *)(v12 + 8);
              v16 = (_DWORD *)(v20 + 4);
              v17 = v12 - (_DWORD)v13;
              v19 = *(unsigned __int16 *)(v5 + 18);
              do
              {
                *(v16 - 1) = *v14;
                *v16 = v14[1];
                v16[1] = *(_DWORD *)((char *)v13 + v17);
                v16[2] = *(v15 - 1);
                v16[3] = *v15;
                v16[4] = *v13;
                v16[5] = v13[1];
                v16[6] = v13[2];
                v14 += 2;
                v15 += 3;
                v13 += 3;
                v16 += 8;
                --v19;
              }
              while ( v19 );
            }
            qglBufferDataARB(34962, 32 * v11, 0, 35044);
            v18 = 32 * *(unsigned __int16 *)(v5 + 18);
          }
          qglBufferDataARB(34962, v18, v20, 35044);
          qglBindBufferARB(34962, 0);
        }
        return;
      }
    }
  }
}
