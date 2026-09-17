/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include <malloc.h>
#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_shaderregistry.h"
#include "tr_gl_types.h"
#include "tr_tess.h"

extern char xmodel_defaultCollision[];

extern int Hunk_AllocAlignInternal();
extern int Hunk_AllocXModelPrecache();
extern int Hunk_AllocXModelPrecacheMesh();
extern int RB_ExecuteRenderCommands();
extern int RE_ClearFlares();
extern int R_FindShader();
extern int R_FinishLoadingStaticModels();
extern int R_Init();
extern int R_LoadDelayedImages();
extern int R_MergeShadersForImageSheets();
extern int R_SetupTextureCoordinateRemap();
extern _DWORD *__cdecl R_SyncRenderThread( void );
typedef void *( *XModelAllocFn )( int size );
void *XModelPrecache( const char *name, int loadSurfaces, XModelAllocFn alloc, XModelAllocFn allocMesh );
extern int qsort_m();
extern int DObjGetNumSurfaces();
extern int DObjGetSurfaces();
extern int XSurfaceGetVerts();
extern int XSurfaceRefresh_ARB();

/* ---- R_GetModelByHandle  0x005029B0 ----  VERIFIED */
int __cdecl R_GetModelByHandle(int a1)
{
  return tr_models[a1];
}

/* ---- RE_GetXModelByHandle  0x005029C0 ----  VERIFIED */
int __cdecl RE_GetXModelByHandle(int a1)
{
  return *(_DWORD *)(tr_models[a1] + 84);
}

/* ---- R_GetXModelBounds  0x005029D0 ----  VERIFIED */
float *__cdecl R_GetXModelBounds(float *a1, int a2, float *a3, float *a4)
{
  int v4;
  int v5;
  _WORD *v7;
  float *result;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  float *v14;
  double v15;
  double v16;
  double v17;
  _WORD *v18;
  float v19[3 * 8192]; // [esp+8h] [ebp-18020h] BYREF -- retail's positionsOut
  int v21[4]; // [esp+18008h] [ebp-20h] BYREF -- partBits, 128 bits
  _WORD *v22;  /* the alloca'd surfaceRefs */
  int v23;  /* surfaceCount */
  int v24; // [esp+18020h] [ebp-8h] BYREF     -- lodIndex
  int v25;

  v4 = *(unsigned __int8 *)(a2 + 22) - 1;
  if ( v4 < 0 )
  {
LABEL_5:
    *a3 = 3.4028235e38;
    a3[1] = 3.4028235e38;
    a3[2] = 3.4028235e38;
    *a4 = -3.4028235e38;
    a4[1] = -3.4028235e38;
    a4[2] = -3.4028235e38;
    v24 = 0;
    v23 = DObjGetNumSurfaces(a2, (int)&v24);
    v18 = (_WORD *)_alloca((4 * v23 + 3) & ~3);
    v7 = v18;
    v22 = v18;
    DObjGetSurfaces(v21, a2, v18, (int)&v24);
    result = (float *)v23;
    v9 = 0;
    v25 = 0;
    if ( v23 > 0 )
    {
      while ( 1 )
      {
        v10 = (__int16)v7[2 * v9];
        v11 = v25;
        v12 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(a2 + 4 * v10 + 24) + 4)
                                                            + 20 * (*(&v24 + v10) + 1))
                                                + 4)
                                    + 8)
                        + 4 * (__int16)v7[2 * v25 + 1]);
        v13 = *(__int16 *)(v12 + 2);
        XSurfaceGetVerts(0, v19, v12, (*(unsigned __int8 *)(v10 + a2 + 80) << 6) + *(_DWORD *)(a2 + 4) + 48, 0);
        if ( v13 > 0 )
        {
          v14 = &v19[2];       /* ebp-0x18018 == v19 + 8 */
          v25 = v13;
          do
          {
            v15 = a1[6] * *v14 + a1[3] * *(v14 - 1) + *a1 * *(v14 - 2);
            if ( (v15 < *a3) | __UNORDERED__(v15, *a3) )
              *a3 = v15;
            if ( v15 > *a4 )
              *a4 = v15;
            v16 = a1[7] * *v14 + a1[4] * *(v14 - 1) + a1[1] * *(v14 - 2);
            if ( (v16 < a3[1]) | __UNORDERED__(v16, a3[1]) )
              a3[1] = v16;
            if ( v16 > a4[1] )
              a4[1] = v16;
            v17 = a1[8] * *v14 + a1[5] * *(v14 - 1) + a1[2] * *(v14 - 2);
            if ( (v17 < a3[2]) | __UNORDERED__(v17, a3[2]) )
              a3[2] = v17;
            if ( v17 > a4[2] )
              a4[2] = v17;
            v14 += 3;
            --v25;
          }
          while ( v25 );
        }
        result = (float *)v23;
        v9 = v11 + 1;
        v25 = v9;
        if ( v9 >= v23 )
          break;
        v7 = v22;
      }
    }
  }
  else
  {
    v5 = a2 + 4 * v4 + 24;
    while ( (void *)*(_DWORD *)(*(_DWORD *)v5 + 4) != (void *)xmodel_defaultCollision )
    {
      --v4;
      v5 -= 4;
      if ( v4 < 0 )
        goto LABEL_5;
    }
    *a3 = -3.4028235e38;
    a3[1] = -3.4028235e38;
    a3[2] = -3.4028235e38;
    result = a4;
    *a4 = 3.4028235e38;
    a4[1] = 3.4028235e38;
    a4[2] = 3.4028235e38;
  }
  return result;
}

/* ---- sub_502BD0  0x00502BD0 ----  VERIFIED */
void __cdecl sub_502BD0(void *this)
{
  Hunk_AllocAlignInternal((unsigned int)this, 32);
}

/* ---- sub_502BE0  0x00502BE0 ----  VERIFIED */
void __cdecl sub_502BE0(void *this)
{
  Hunk_AllocAlignInternal((unsigned int)this, 32);
}

/* ---- R_LoadXModel  0x00502BF0 ----  VERIFIED */
int __cdecl R_LoadXModel(_DWORD *a1, const char *a2, const char *a3)
{
  int *v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  _DWORD *v9;
  _DWORD *v10;
  char *v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  unsigned __int16 v17;
  const char *v18;
  char *Shader;
  int v20;
  bool v21; // zf
  int result;
  _DWORD *v23;
  int *v24;
  int v25;
  char *v26;
  int v27;
  _DWORD *v28;
  char v29[68]; // [esp+28h] [ebp-48h] BYREF
  unsigned int v30;
  unsigned int retaddr;

  v30 = retaddr ^ _security_cookie;
  if ( tr_modelsFinishedLoading )
  {
    if ( r_errorOnConflicts->integer )
    {
      if ( !tr_ignorePrecacheErrorCount )
        ri_Error(1, "\x15" "model '%s' not precached\n", a2);
    }
  }
  else
  {
    tr_delayedImageGroup = ++tr_delayedImageGroupSequence;
  }
  v3 = (int *)XModelPrecache(
         a2,
         2,
         (XModelAllocFn)Hunk_AllocXModelPrecache,
         (XModelAllocFn)Hunk_AllocXModelPrecacheMesh);
  v4 = v3[1];
  v5 = *(__int16 *)(v4 + 100);
  v6 = 0;
  v24 = v3;
  if ( v5 > 0 )
  {
    v7 = v4 + 20;
    v8 = v5;
    do
    {
      v6 += *(__int16 *)(*(_DWORD *)(*(_DWORD *)v7 + 4) + 4);
      v7 += 20;
      --v8;
    }
    while ( v8 );
  }
  v9 = Hunk_AllocAlignInternal(2 * (v6 + 2 * v5), 32);
  v10 = v9;
  v28 = v9;
  v11 = (char *)&v9[v5];
  if ( v5 > 0 )
  {
    v12 = 0;
    v23 = v9;
    v25 = v5;
    while ( 1 )
    {
      v13 = *(_DWORD *)(*(_DWORD *)(v3[1] + v12 + 20) + 4);
      v14 = *(__int16 *)(v13 + 4);
      v15 = *(_DWORD *)(v13 + 8);
      *v23 = v11;
      v16 = 0;
      v27 = v15;
      v26 = &v11[2 * v14];
      if ( v14 > 0 )
      {
        while ( 1 )
        {
          strcpy(v29, "skins/");
          v17 = *(_WORD *)(*(_DWORD *)(v24[1] + v12 + 16) + 2 * v16);
          v18 = v17 ? (const char *)(GetRefString_var + 8 * v17 + 4) : "DEFAULT";
          strcpy(&v29[6], v18);
          if ( tr_delayedImageGroup )
          {
            tr_delayedImageGroupTriCount = *(__int16 *)(*(_DWORD *)(v15 + 4 * v16) + 4);
            tr_delayedImageGroupTileMode = **(unsigned __int8 **)(v15 + 4 * v16);
          }
          Shader = R_FindShader(v29, -1, 1, a3);
          if ( (*((_DWORD *)Shader + 19) & 0x1000) != 0 )
          {
            ri_Printf(2, "WARNING: model '%s' not precached, bad texturing will result on some surfaces\n", a2);
            Shader = (char *)*((_DWORD *)Shader + 96);
          }
          if ( (Shader[76] & 1) != 0 )
            LOWORD(v20) = 0;
          else
            v20 = *((_DWORD *)Shader + 17);
          *(_WORD *)(*v23 + 2 * v16++) = v20;
          if ( v16 >= v14 )
            break;
          v15 = v27;
        }
        v10 = v28;
      }
      v3 = v24;
      v12 += 20;
      v21 = v25 == 1;
      ++v23;
      --v25;
      if ( v21 )
        break;
      v11 = v26;
    }
  }
  a1[21] = v3;
  a1[16] = 2;
  a1[20] = v10;
  result = 0;
  tr_delayedImageGroup = 0;
  tr_delayedImageGroupTriCount = 0;
  return result;
}

/* ---- compare_xsurface_remap  0x00502E20 ----  VERIFIED */
int __cdecl compare_xsurface_remap(_DWORD *a1, _DWORD *a2)
{
  int result;

  result = a1[1] - a2[1];
  if ( !result )
    return (*a1 - *a2) / 408;
  return result;
}

/* ---- R_FixupXModelTexCoords  0x00502E50 ----  [HIGH] */
int R_FixupXModelTexCoords()
{
  int result;
  int v1;
  signed int v2;
  int v3;
  int v4;
  float v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  signed int v11;
  int *v12;
  int v13;
  int ***v14;
  int **v15;
  __int16 *v16;
  int v17;
  void *v18;
  int v19;
  int v20;
  int **v21;
  int *v22;
  int v23;
  BOOL v24;
  int v25;
  double v26;
  bool v27; // cc
  int v28;
  int v29;
  int v30;
  int v31;
  int v32;
  int v33;
  int v34;
  _WORD *v35;
  int v36;
  int v37;
  signed int v38;
  _DWORD *v39;
  int v40;
  int ***v41;
  int v42;
  int j; // [esp+18h] [ebp-C0020h] BYREF
  int i;
  int v45;
  BOOL v46; // [esp+24h] [ebp-C0014h] BYREF
  float v47[2]; // [esp+28h] [ebp-C0010h] BYREF
  float v48[2]; // [esp+30h] [ebp-C0008h] BYREF
  _DWORD remaps[65536][3]; // [esp+38h] [ebp-C0000h] BYREF

  result = tr_numModels;
  v1 = 1;
  v2 = 0;
  v45 = 0;
  j = 1;
  if ( tr_numModels > 1 )
  {
    do
    {
      v3 = tr_models[v1];
      v4 = *(_DWORD *)(v3 + 84);
      v5 = 0.0;
      v46 = v3;
      if ( v4 )
      {
        v45 = *(__int16 *)(*(_DWORD *)(v4 + 4) + 100);
        v47[0] = 0.0;
        if ( v45 > 0 )
        {
          v40 = 0;
          do
          {
            v6 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v3 + 84) + 4) + v40 + 20) + 4);
            v7 = *(__int16 *)(v6 + 4);
            v8 = *(_DWORD *)(v6 + 8);
            v9 = 0;
            for ( i = v7; v9 < v7; ++v9 )
            {
              if ( *(_WORD *)(*(_DWORD *)(*(_DWORD *)(v3 + 80) + 4 * LODWORD(v5)) + 2 * v9) )
              {
                v10 = tr_shaders[*(unsigned __int16 *)(*(_DWORD *)(*(_DWORD *)(v3 + 80) + 4 * LODWORD(v5)) + 2 * v9)];
                if ( (*(_DWORD *)(v10 + 76) & 0x1000) == 0 )
                  v10 = 0;
                if ( v2 == 0x10000 )
                  ri_Error(1, "\x15" "More than %i xmodel surfaces need to be remapped\n"
                              "You may need to set r_optimizeTextures temporarily to 0\n"
                              "Eventually you want to use fewer unique model surfaces\n", 0x10000);
                v11 = 0;
                if ( v2 <= 0 )
                {
LABEL_16:
                  v13 = 3 * v2;
                  remaps[v2][0] = v10;
                  remaps[v2][1] = *(_DWORD *)(v8 + 4 * v9);
                  remaps[v2][2] = 0;
                  ++v2;
                }
                else
                {
                  v12 = &remaps[0][0];
                  while ( v12[1] != *(_DWORD *)(v8 + 4 * v9) || *v12 != v10 )
                  {
                    ++v11;
                    v12 += 3;
                    if ( v11 >= v2 )
                      goto LABEL_16;
                  }
                }
                v3 = v46;
                v5 = v47[0];
                v7 = i;
              }
            }
            ++LODWORD(v5);
            v47[0] = v5;
            v40 += 20;
          }
          while ( SLODWORD(v5) < v45 );
          v1 = j;
        }
      }
      result = tr_numModels;
      j = ++v1;
    }
    while ( v1 < tr_numModels );
    v45 = v2;
    if ( v2 )
    {
      qsort_m((unsigned int)remaps, v2, 0xCu, (int (__cdecl *)(unsigned int, _BYTE *))compare_xsurface_remap);
      i = 0;
      if ( v2 > 0 )
      {
        v14 = (int ***)&remaps[0][1];
        v41 = (int ***)&remaps[0][1];
        do
        {
          v15 = *v14;
          if ( v14[3] == *v14 )
          {
            v16 = (__int16 *)Hunk_AllocAlignInternal(0x34u, 32);
            qmemcpy(v16, v15, 0x34u);
            v17 = v16[1];
            *((_DWORD *)v16 + 11) = 0;
            *((_DWORD *)v16 + 12) = 0;
            v18 = Hunk_AllocAlignInternal(8 * v17, 32);
            v19 = 8 * v16[1];
            *((_DWORD *)v16 + 9) = v18;
            Com_Memcpy((int)v18, v15[9], v19);
            v2 = v45;
            v41[1] = (int **)v16;
            v14 = v41;
          }
          else
          {
            v14[1] = *v14;
          }
          v20 = (int)*(v14 - 1);
          if ( v20 )
          {
            R_SetupTextureCoordinateRemap(v20, v47, &v46, v48, &j);
            v21 = v14[1];
            v22 = v21[9];
            v23 = *((__int16 *)v21 + 1);
            if ( v23 )
            {
              v24 = v46;
              v25 = j;
              do
              {
                v26 = *(float *)&v22[v24];
                v22 += 2;
                --v23;
                *((float *)v22 - 2) = *(float *)&v22[v25 - 2] * v47[0] + v48[0];
                *((float *)v22 - 1) = v47[1] * v26 + v48[1];
              }
              while ( v23 );
            }
          }
          v14 += 3;
          v27 = ++i < v2;
          v41 = v14;
        }
        while ( v27 );
      }
      result = tr_numModels;
      v28 = 1;
      for ( j = 1; v28 < tr_numModels; j = v28 )
      {
        v29 = tr_models[v28];
        v30 = *(_DWORD *)(v29 + 84);
        v46 = v29;
        if ( v30 )
        {
          v31 = *(__int16 *)(*(_DWORD *)(v30 + 4) + 100);
          v45 = v31;
          v47[0] = 0.0;
          if ( v31 > 0 )
          {
            v42 = 0;
            do
            {
              v32 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v29 + 84) + 4) + v42 + 20) + 4);
              v33 = *(_DWORD *)(v32 + 8);
              v34 = 0;
              i = *(__int16 *)(v32 + 4);
              if ( i > 0 )
              {
                do
                {
                  v35 = (_WORD *)(*(_DWORD *)(*(_DWORD *)(v29 + 80) + 4 * LODWORD(v47[0])) + 2 * v34);
                  v36 = (unsigned __int16)*v35;
                  if ( *v35 )
                  {
                    v37 = tr_shaders[v36];
                    if ( (*(_DWORD *)(v37 + 76) & 0x1000) != 0 )
                      v36 = *(_DWORD *)(*(_DWORD *)(v37 + 384) + 68);
                    else
                      v37 = 0;
                    *v35 = v36;
                    v38 = 0;
                    if ( v2 > 0 )
                    {
                      v39 = &remaps[0][1];
                      while ( *(v39 - 1) != v37 || *v39 != *(_DWORD *)(v33 + 4 * v34) )
                      {
                        ++v38;
                        v39 += 3;
                        if ( v38 >= v2 )
                          goto LABEL_49;
                      }
                      *(_DWORD *)(v33 + 4 * v34) = remaps[v38][2];
                    }
                  }
LABEL_49:
                  v29 = v46;
                  ++v34;
                }
                while ( v34 < i );
                v31 = v45;
              }
              v27 = ++LODWORD(v47[0]) < v31;
              v42 += 20;
            }
            while ( v27 );
            v28 = j;
          }
        }
        result = tr_numModels;
        ++v28;
      }
    }
  }
  return result;
}

/* ---- R_AllocModel  0x005033C0 ----  VERIFIED */
int R_AllocModel()
{
  int result;

  if ( tr_numModels == 2048 )
    return 0;
  result = ri_Hunk_Alloc(96);
  *(_DWORD *)(result + 68) = tr_numModels;
  tr_models[tr_numModels++] = result;
  return result;
}

/* ---- RE_RegisterModel  0x00503400 ----  VERIFIED */
int __cdecl RE_RegisterModel(char *Source, const char *a2)
{
  const char *v3;
  int v4;
  int v5;
  int v6;
  char v7;

  if ( !Source || !*Source )
    return 0;
  if ( strlen(Source) >= 0x40 )
  {
    Com_Printf("Model name exceeds MAX_QPATH\n");
    return 0;
  }
  v3 = (const char *)tr_numModels;
  v4 = 1;
  if ( tr_numModels <= 1 )
  {
LABEL_9:
    if ( v3 == (const char *)2048 )
    {
      ri_Printf(2, "RE_RegisterModel: R_AllocModel() failed for '%s'\n", Source);
      return 0;
    }
    v6 = ri_Hunk_Alloc(96);
    *(_DWORD *)(v6 + 68) = tr_numModels;
    tr_models[tr_numModels++] = v6;
    strncpy((char *)v6, Source, 0x3Fu);
    *(_BYTE *)(v6 + 63) = 0;
    R_SyncRenderThread();
    *(_DWORD *)(v6 + 92) = 0;
    if ( !_strnicmp(Source, "xmodel", 6u) )
    {
      v7 = Source[6];
      if ( v7 == 47 || v7 == 92 )
      {
        R_LoadXModel((_DWORD *)v6, Source + 7, a2);
        return *(_DWORD *)(v6 + 68);
      }
    }
    *(_DWORD *)(v6 + 64) = 0;
    return 0;
  }
  while ( 1 )
  {
    v5 = tr_models[v4];
    if ( v5 )
    {
      if ( !Q_stricmpn((const char *)v5, Source, 99999) )
        return *(_DWORD *)(v5 + 64) != 0 ? v4 : 0;
    }
    if ( ++v4 >= (int)v3 )
      goto LABEL_9;
  }
}

/* ---- RE_FinishLoadingModels  0x00503540 ----  VERIFIED */
cvar_t *RE_FinishLoadingModels()
{
  void *v0;
  cvar_t *result;
  cvar_t *v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  _WORD *v10;
  bool v11; // zf
  int v12;
  int v13;
  unsigned __int16 *v14;
  int v15;
  int v16;
  int v17;
  bool v18; // cc
  int v19;
  int v20;
  int v21;
  int i;
  int v23;
  _DWORD v24[1024]; // [esp+14h] [ebp-1000h] BYREF

  R_LoadDelayedImages();
  tr_modelsFinishedLoading = 1;
  R_MergeShadersForImageSheets();
  R_FinishLoadingStaticModels();
  R_FixupXModelTexCoords();
  v0 = alloca(4116);
  result = r_optimize;
  if ( r_optimize->integer )
  {
    v2 = r_optimizeXModels;
    if ( r_optimizeXModels->integer > 0 )
    {
      result = (cvar_t *)tr_numModels;
      v3 = 1;
      for ( i = 1; v3 < tr_numModels; i = v3 )
      {
        v4 = tr_models[v3];
        v5 = *(_DWORD *)(v4 + 84);
        v21 = v4;
        if ( v5 )
        {
          v23 = *(__int16 *)(*(_DWORD *)(v5 + 4) + 100);
          v19 = 0;
          if ( v23 > 0 )
          {
            v20 = 0;
            do
            {
              v6 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v4 + 84) + 4) + v20 + 20) + 4);
              v7 = *(__int16 *)(v6 + 4);
              v8 = *(_DWORD *)(v6 + 8);
              v9 = 0;
              if ( v7 > 0 )
              {
                v10 = *(_WORD **)(*(_DWORD *)(v4 + 80) + 4 * v19);
                do
                {
                  v11 = *v10 == 0;
                  v24[v9] = 0;
                  if ( !v11 && *(__int16 *)(*(_DWORD *)((char *)&v24[v9] + v8 - (_DWORD)v24) + 2) >= v2->integer )
                    v24[v9] = 1;
                  ++v9;
                  ++v10;
                }
                while ( v9 < v7 );
                v4 = v21;
              }
              v12 = 0;
              if ( v7 > 0 )
              {
                v13 = v8 - (_DWORD)v24;
                do
                {
                  v14 = *(unsigned __int16 **)(*(_DWORD *)(v4 + 80) + 4 * v19);
                  v15 = v14[v12];
                  if ( v14[v12] )
                  {
                    if ( v24[v12] )
                    {
LABEL_22:
                      v17 = *(_DWORD *)((char *)&v24[v12] + v13);
                      if ( *(_WORD *)(v17 + 6) != 0xFFFF )
                      {
                        if ( glConfig_ARBVertexBufferObject )
                        {
                          R_OptimizeRigidXSurfaceARB(v17, (int (__cdecl *)(int))Hunk_AllocXModelPrecacheMesh);
                        }
                        else if ( glConfig_NVVertexArrayRange )
                        {
                          R_OptimizeRigidXSurfaceNV(v17, (int (__cdecl *)(int))Hunk_AllocXModelPrecacheMesh);
                        }
                        else if ( glConfig_ATIVertexArrayObject )
                        {
                          R_OptimizeRigidXSurfaceATI(v7, v17, v13, (int (__stdcall *)(int, int, int))Hunk_AllocXModelPrecacheMesh);
                        }
                      }
                    }
                    else
                    {
                      v16 = 0;
                      while ( v16 == v12 || *v14 != v15 )
                      {
                        ++v16;
                        ++v14;
                        if ( v16 >= v7 )
                          goto LABEL_22;
                      }
                    }
                    v4 = v21;
                  }
                  ++v12;
                }
                while ( v12 < v7 );
              }
              v2 = r_optimizeXModels;
              v18 = ++v19 < v23;
              v20 += 20;
            }
            while ( v18 );
            v3 = i;
          }
        }
        result = (cvar_t *)tr_numModels;
        ++v3;
      }
      xmodel_animCheck = 1;
    }
  }
  return result;
}

/* ---- RE_SetIgnorePrecacheErrors  0x00503570 ----  VERIFIED */
int __cdecl RE_SetIgnorePrecacheErrors(int a1)
{
  if ( a1 )
    return ++tr_ignorePrecacheErrorCount;
  else
    return --tr_ignorePrecacheErrorCount;
}

/* ---- RE_GetIgnorePrecacheErrors  0x00503590 ----  VERIFIED */
BOOL RE_GetIgnorePrecacheErrors()
{
  return tr_ignorePrecacheErrorCount != 0;
}

/* ---- RE_BeginRegistration  0x005035A0 ----  [HIGH] */
unsigned int __cdecl RE_BeginRegistration(const char *a1, int *a2)
{
  int v2;
  cvar_t *v3;
  int v4;
  int v5;
  unsigned int result;
  _DWORD *v7;
  char *v8;

  R_Init(a1);
  v2 = backEndData;
  qmemcpy(a2, &glConfig_renderer_string, 0xA0u);
  if ( tr_registered )
  {
    *(_DWORD *)(v2 + 1636096 + *(_DWORD *)(v2 + 1898240)) = 0;
    v3 = r_skipBackEnd;
    *(_DWORD *)(v2 + 1898240) = 0;
    if ( !v3->integer )
    {
      RB_ExecuteRenderCommands((_DWORD *)(v2 + 1636096), a1);
      v2 = backEndData;              /* `mov edx, backEndData` 0x5035ED */
    }
  }
  v4 = v2;
  tr_viewCluster = -1;
  RE_ClearFlares();
  r_firstSceneDlight = r_numdlights;
  r_firstSceneEntity = r_numentities;
  tr_registered = 1;
  v5 = *(_DWORD *)(v4 + 1898240);
  r_firstSceneCorona = r_numcoronas;
  result = v5 + 40;
  r_firstScenePoly = r_numpolys;
  if ( result <= 0x3FFF8 )
  {
    v7 = (_DWORD *)(result + v4 + 1636056);
    *(_DWORD *)(v4 + 1898240) = result;
    if ( result + v4 != -1636056 )
    {
      *v7 = 2;
      if ( tr_numShaders[0] > 0 )
      {
        v8 = (char *)tr_shaders[0];
      }
      else
      {
        ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", 0);
        v8 = tr_defaultShader;
      }
      v7[1] = v8;
      result = 1065353216;
      v7[2] = 0;
      v7[3] = 0;
      v7[4] = 0;
      v7[5] = 0;
      v7[6] = 0;
      v7[7] = 0;
      v7[8] = 1065353216;
      v7[9] = 1065353216;
    }
  }
  return result;
}

/* ---- R_ModelInit  0x005036B0 ----  VERIFIED */
int R_ModelInit()
{
  int result;

  tr_numModels = 0;
  result = ri_Hunk_Alloc(96);
  *(_DWORD *)(result + 68) = tr_numModels;
  tr_models[tr_numModels++] = result;
  *(_DWORD *)(result + 64) = 0;
  return result;
}

/* ---- R_ModelBounds  0x005036F0 ----  VERIFIED */
_DWORD *__cdecl R_ModelBounds(int a1, _DWORD *a2, _DWORD *a3)
{
  int v3;
  _DWORD *v4;
  _DWORD *result;

  v3 = tr_models[a1];
  v4 = *(_DWORD **)(v3 + 76);
  if ( v4 )
  {
    *a2 = *v4;
    a2[1] = *(_DWORD *)(*(_DWORD *)(v3 + 76) + 4);
    a2[2] = *(_DWORD *)(*(_DWORD *)(v3 + 76) + 8);
    *a3 = *(_DWORD *)(*(_DWORD *)(v3 + 76) + 12);
    a3[1] = *(_DWORD *)(*(_DWORD *)(v3 + 76) + 16);
    result = *(_DWORD **)(v3 + 76);
    a3[2] = result[5];
  }
  else
  {
    a2[2] = 0;
    a2[1] = 0;
    *a2 = 0;
    result = a3;
    a3[2] = 0;
    a3[1] = 0;
    *a3 = 0;
  }
  return result;
}

/* ---- R_RefreshXModels_ARB  0x00503760 ----  [HIGH] */
int __cdecl R_RefreshXModels_ARB(char a1)
{
  int result;
  int *v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  bool v9; // zf
  bool v10; // cc
  int v11;
  int v12;
  int v13;
  int *v14;

  result = tr_numModels;
  v13 = 0;
  if ( tr_numModels > 0 )
  {
    v2 = tr_models;
    v14 = tr_models;
    do
    {
      v3 = *(_DWORD *)(*v2 + 84);
      if ( v3 )
      {
        v4 = *(__int16 *)(*(_DWORD *)(v3 + 4) + 100);
        if ( v4 > 0 )
        {
          v11 = 0;
          v12 = v4;
          do
          {
            v5 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*v2 + 84) + 4) + v11 + 20) + 4);
            v6 = *(__int16 *)(v5 + 4);
            v7 = *(_DWORD *)(v5 + 8);
            v8 = 0;
            if ( v6 > 0 )
            {
              do
                XSurfaceRefresh_ARB(*(_DWORD *)(v7 + 4 * v8++), a1);
              while ( v8 < v6 );
              v2 = v14;
            }
            v9 = v12 == 1;
            v11 += 20;
            --v12;
          }
          while ( !v9 );
        }
      }
      result = v13 + 1;
      ++v2;
      v10 = ++v13 < tr_numModels;
      v14 = v2;
    }
    while ( v10 );
  }
  return result;
}

/* ---- R_IncrementalRefreshXModels_ARB  0x00503820 ----  [HIGH] */
int __cdecl R_IncrementalRefreshXModels_ARB(char a1)
{
  int v1;
  int result;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;

  v1 = tr_xmodelRefreshLodIndex;
  result = tr_xmodelRefreshModelIndex;
  v3 = tr_xmodelRefreshSurfaceIndex;
  do
  {
LABEL_2:
    v4 = *(_DWORD *)(tr_models[result] + 84);
    if ( !v4 )
      break;
    v5 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v4 + 4) + 4 * (5 * v1 + 5)) + 4);
    v6 = *(_DWORD *)(v5 + 8);
    v7 = *(__int16 *)(v5 + 4);
    v8 = v3 + 1;
    tr_xmodelRefreshSurfaceIndex = v8;
    if ( v8 < v7 )
      return XSurfaceRefresh_ARB(*(_DWORD *)(v6 + 4 * v8), a1);
    v3 = 0;
    tr_xmodelRefreshSurfaceIndex = 0;
    v9 = *(__int16 *)(*(_DWORD *)(*(_DWORD *)(tr_models[result] + 84) + 4) + 100);
    tr_xmodelRefreshLodIndex = ++v1;
  }
  while ( v1 < v9 );
  v1 = 0;
  tr_xmodelRefreshLodIndex = 0;
  v10 = result;
  while ( 1 )
  {
    tr_xmodelRefreshModelIndex = ++result;
    if ( result >= tr_numModels )
    {
      result = 0;
      tr_xmodelRefreshModelIndex = 0;
    }
    if ( result == v10 )
      return result;
    if ( *(_DWORD *)(tr_models[result] + 84) )
      goto LABEL_2;
  }
}
