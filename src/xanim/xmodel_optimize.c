/*
 * Reconstructed from Call of Duty 1.1 (Windows, CoDMP.exe).
 * Original translation unit:
 * /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/xanim/xmodel_optimize.c
 * Retail range 0x0048C580-0x0048CB2F, 7 functions.
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "../renderer/qgl.h"

void *Hunk_AllocAlignInternal( int size, int align );

extern int R_CreateBufferARB();

/* ---- XSurfaceRefresh_ARB  0x0048C580 ----  VERIFIED */
int __cdecl XSurfaceRefresh_ARB(int a1, char a2)
{
  int result;
  _DWORD *v3;
  _DWORD *v4;
  unsigned int v5;
  int v6;
  _DWORD *v7;
  int v8;
  _BYTE v9[0x40000]; /* retail frame slot [ebp-40000h] */

  result = *(_DWORD *)(a1 + 40);
  v7 = (_DWORD *)result;
  if ( result )
  {
    result = *(__int16 *)(a1 + 4);
    v3 = *(_DWORD **)(a1 + 32);
    v4 = *(_DWORD **)(a1 + 36);
    v5 = (6 * result + 31) & 0xFFFFFFE0;
    v6 = 32 * *(__int16 *)(a1 + 2);
    if ( *(__int16 *)(a1 + 2) > 0 )
    {
      result = (int)(v9 + 4);
      v8 = *(__int16 *)(a1 + 2);
      do
      {
        *(_DWORD *)(result - 4) = *v4;
        *(_DWORD *)result = v4[1];
        *(_DWORD *)(result + 4) = *v3;
        *(_DWORD *)(result + 8) = v3[1];
        *(_DWORD *)(result + 12) = v3[2];
        *(_DWORD *)(result + 16) = v3[3];
        *(_DWORD *)(result + 20) = v3[4];
        *(_DWORD *)(result + 24) = v3[5];
        v3 += 6;
        v4 += 2;
        result += 32;
        --v8;
      }
      while ( v8 );
    }
    if ( (a2 & 2) != 0 )
    {
      qglBindBufferARB(34963, v7[1]);
      qglBufferDataARB(34963, v5, 0, 35044);
      qglBufferDataARB(34963, v5, *(_DWORD *)(a1 + 28), 35044);
      qglBindBufferARB(34963, 0);
    }
    if ( (a2 & 1) != 0 )
    {
      qglBindBufferARB(34962, *v7);
      qglBufferDataARB(34962, v6, 0, 35044);
      qglBufferDataARB(34962, v6, v9, 35044);
      qglBindBufferARB(34962, 0);
      return result;
    }
  }
  return result;
}

/* ---- XModelSurfsRefresh_ARB  0x0048C6C0 ----  VERIFIED */
int __cdecl XModelSurfsRefresh_ARB(int result, char a2)
{
  int v2;
  int v3;
  int i;

  v2 = *(__int16 *)(result + 4);
  v3 = *(_DWORD *)(result + 8);
  for ( i = 0; i < v2; ++i )
    result = XSurfaceRefresh_ARB(*(_DWORD *)(v3 + 4 * i), a2);
  return result;
}

/* ---- R_OptimizeRigidXSurfaceARB  0x0048C6F0 ----  VERIFIED */
int *__cdecl R_OptimizeRigidXSurfaceARB(int a1, int (__cdecl *a2)(int))
{
  int *result;
  _DWORD *v3;
  _DWORD *v4;
  unsigned int v5;
  int v6;
  char *v7;
  int BufferARB;
  int v9; /* retail frame slot [ebp-40004h] */
  _BYTE v10[0x40000]; /* retail frame slot [ebp-40000h] */

  result = *(int **)(a1 + 40);
  if ( !result )
  {
    v3 = *(_DWORD **)(a1 + 32);
    v4 = *(_DWORD **)(a1 + 36);
    v5 = (6 * *(__int16 *)(a1 + 4) + 31) & 0xFFFFFFE0;
    v6 = 32 * *(__int16 *)(a1 + 2);
    if ( *(__int16 *)(a1 + 2) > 0 )
    {
      v7 = (char *)v10 + 4;   /* retail 0x0048C72D lea eax,[esp+40010h+var_3FFFC] */
      v9 = *(__int16 *)(a1 + 2);
      do
      {
        *((_DWORD *)v7 - 1) = *v4;
        *(_DWORD *)v7 = v4[1];
        *((_DWORD *)v7 + 1) = *v3;
        *((_DWORD *)v7 + 2) = v3[1];
        *((_DWORD *)v7 + 3) = v3[2];
        *((_DWORD *)v7 + 4) = v3[3];
        *((_DWORD *)v7 + 5) = v3[4];
        *((_DWORD *)v7 + 6) = v3[5];
        v3 += 6;
        v4 += 2;
        v7 += 32;
        --v9;
      }
      while ( v9 );
    }
    result = (int *)R_CreateBufferARB((int)v3, 34963, v5, *(_DWORD *)(a1 + 28), 35044);
    v9 = (int)result;
    if ( result )
    {
      BufferARB = R_CreateBufferARB((int)v10, 34962, v6, (int)v10, 35044);
      if ( BufferARB )
      {
        result = (int *)a2(8);
        result[1] = v9;
        *result = BufferARB;
        *(_DWORD *)(a1 + 40) = result;
      }
      else
      {
        qglDeleteBuffersARB(1, &v9);
        return result;
      }
    }
  }
  return result;
}

/* ---- R_OptimizeRigidXSurfaceNV  0x0048C7F0 ----  VERIFIED */
int __cdecl R_OptimizeRigidXSurfaceNV(int a1, int (__cdecl *a2)(int))
{
  int v2;
  int result;
  _DWORD *v4;
  unsigned int v5;
  _DWORD *v6;
  _DWORD *v7;
  int v8;

  v2 = a1;
  result = *(_DWORD *)(a1 + 48);
  if ( !result )
  {
    v4 = (_DWORD *)a2(8);
    v5 = (32 * *(__int16 *)(a1 + 2) + 31) & 0xFFFFFFE0;
    if ( (int)(tr_staticVertexMemoryPrimaryUsed + v5) >= tr_staticVertexMemoryPrimaryLimit )
    {
      if ( (int)(tr_staticVertexMemorySecondaryUsed + v5) >= tr_staticVertexMemorySecondaryLimit )
      {
        v4[1] = (int)Hunk_AllocAlignInternal(v5, 32);
        result = 3;
      }
      else
      {
        v4[1] = tr_staticVertexMemorySecondaryUsed + tr_staticVertexMemorySecondary;
        tr_staticVertexMemorySecondaryUsed += v5;
        result = 2;
      }
    }
    else
    {
      v4[1] = tr_staticVertexMemoryPrimaryUsed + tr_staticVertexMemoryPrimary;
      tr_staticVertexMemoryPrimaryUsed += v5;
      result = 1;
    }
    *v4 = result;
    v6 = *(_DWORD **)(a1 + 32);
    v7 = *(_DWORD **)(a1 + 36);
    v8 = 0;
    if ( *(__int16 *)(a1 + 2) > 0 )
    {
      result = 0;
      do
      {
        *(_DWORD *)(result + v4[1]) = *v7;
        *(_DWORD *)(result + v4[1] + 4) = v7[1];
        *(_DWORD *)(result + v4[1] + 8) = *v6;
        *(_DWORD *)(result + v4[1] + 12) = v6[1];
        *(_DWORD *)(result + v4[1] + 16) = v6[2];
        *(_DWORD *)(result + v4[1] + 20) = v6[3];
        *(_DWORD *)(result + v4[1] + 24) = v6[4];
        *(_DWORD *)(result + v4[1] + 28) = v6[5];
        v2 = a1;
        v6 += 6;
        v7 += 2;
        ++v8;
        result += 32;
      }
      while ( v8 < *(__int16 *)(a1 + 2) );
    }
    *(_DWORD *)(v2 + 48) = v4;
  }
  return result;
}

/* ---- R_OptimizeRigidXSurfaceATI  0x0048C900 ----  VERIFIED */
int __cdecl R_OptimizeRigidXSurfaceATI(int a1, int a2, int a3, int (__stdcall *a4)(int, int, int))
{
  int result;
  unsigned int v5;
  int v6;
  unsigned int v7;
  _DWORD *v8;
  _DWORD *v9;
  _DWORD *v10;
  char *v11;
  int v13;
  int v14;
  int v15;
  _BYTE v16[0x3FFF4]; /* retail frame slot [ebp-3FFF4h] */

  result = *(_DWORD *)(a2 + 44);
  if ( !result )
  {
    if ( glConfig_ATIElementArray )
      v5 = (6 * *(__int16 *)(a2 + 4) + 31) & 0xFFFFFFE0;
    else
      v5 = 0;
    v6 = tr_staticVertexMemoryPrimaryUsed;
    v7 = (32 * *(__int16 *)(a2 + 2) + v5 + 31) & 0xFFFFFFE0;
    if ( (int)(tr_staticVertexMemoryPrimaryUsed + v7) > tr_staticVertexMemoryPrimaryLimit )
    {
      v6 = tr_staticVertexMemorySecondaryUsed;
      result = tr_staticVertexMemorySecondaryUsed + v7;
      if ( result > tr_staticVertexMemorySecondaryLimit )
        return result;
      tr_staticVertexMemorySecondaryUsed = result;
    }
    else
    {
      tr_staticVertexMemoryPrimaryUsed += v7;
    }
    v8 = (_DWORD *)a4(16, a3, a1);
    *v8 = 3;
    if ( v13 == 1 )
      v8[1] = tr_staticVertexMemoryPrimary;
    else
      v8[1] = tr_staticVertexMemorySecondary;
    v8[3] = v6;
    v8[2] = v5 + v6;
    v9 = *(_DWORD **)(a2 + 32);
    v10 = *(_DWORD **)(a2 + 36);
    if ( *(__int16 *)(a2 + 2) > 0 )
    {
      v11 = (char *)v16 + 4;
      v14 = *(__int16 *)(a2 + 2);
      do
      {
        *((_DWORD *)v11 - 1) = *v10;
        *(_DWORD *)v11 = v10[1];
        *((_DWORD *)v11 + 1) = *v9;
        *((_DWORD *)v11 + 2) = v9[1];
        *((_DWORD *)v11 + 3) = v9[2];
        *((_DWORD *)v11 + 4) = v9[3];
        *((_DWORD *)v11 + 5) = v9[4];
        *((_DWORD *)v11 + 6) = v9[5];
        v9 += 6;
        v10 += 2;
        v11 += 32;
        --v14;
      }
      while ( v14 );
    }
    if ( v5 )
      qglUpdateObjectBufferATI(v8[1], v8[3], v5, *(_DWORD *)(a2 + 28), 34658);
    qglUpdateObjectBufferATI(v8[1], v8[2], v15, v16, 34658);
    *(_DWORD *)(a2 + 44) = v8;
  }
  return result;
}

/* ---- XModelOptimize  0x0048CA70 ----  VERIFIED */
int *__cdecl XModelOptimize(int a1, int (__cdecl *a2)(int))
{
  int v2;
  int *result;
  int v4;
  int v5;
  int *i;

  v2 = *(__int16 *)(a1 + 4);
  result = *(int **)(a1 + 8);
  v4 = 0;
  for ( i = result; v4 < v2; ++v4 )
  {
    v5 = i[v4];
    if ( *(_WORD *)(v5 + 6) != 0xFFFF )
    {
      if ( glConfig_ARBVertexBufferObject )
      {
        result = R_OptimizeRigidXSurfaceARB(v5, a2);
      }
      else if ( glConfig_NVVertexArrayRange )
      {
        result = (int *)R_OptimizeRigidXSurfaceNV(v5, a2);
      }
      else
      {
        result = (int *)glConfig_ATIVertexArrayObject;
        if ( glConfig_ATIVertexArrayObject )
          result = (int *)R_OptimizeRigidXSurfaceATI(v2, v5, (int)a2, (int (__stdcall *)(int, int, int))a2);
      }
    }
  }
  return result;
}

/* ---- XSurfaceOptimize  0x0048CAE0 ----  VERIFIED */
int *__cdecl XSurfaceOptimize(int *result, int a2, int a3, int a4)
{
  if ( *(_WORD *)(a2 + 6) != 0xFFFF )
  {
    if ( glConfig_ARBVertexBufferObject )
    {
      return R_OptimizeRigidXSurfaceARB(a2, (int (__cdecl *)(int))result);
    }
    else if ( glConfig_NVVertexArrayRange )
    {
      return (int *)R_OptimizeRigidXSurfaceNV(a2, (int (__cdecl *)(int))result);
    }
    else if ( glConfig_ATIVertexArrayObject )
    {
      return (int *)R_OptimizeRigidXSurfaceATI(a3, a2, a4, (int (__stdcall *)(int, int, int))result);
    }
  }
  return result;
}
