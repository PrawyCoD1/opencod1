/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include <malloc.h>
#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_shaderregistry.h"
#include "tr_gl_types.h"
#include "tr_orientation.h"   /* backEnd.or 0x016D8DA8 -- one object, not 16 shards */
#include "tr_tess.h"

extern char xmodel_defaultCollision[];

extern int GL_Bind();
extern int GL_ClientState();
extern int GL_Cull();
extern int GL_State();
extern int RB_CheckOverflow();
extern int RB_ChooseSurfaceCountColor();
extern int RB_EndMultitexture();
extern int RB_GetAnimatedImage();
extern void __cdecl RB_SetIteratorFog( void );
extern int RB_SetupMultitexture();
extern int R_AddDrawSurf();
extern int R_AddScaledDebugString();
extern int R_SetupEntityLighting();
extern int XSurfaceGetTris();
extern int DObjBackfillParentPartBits();
extern int DObjBuildPartCollisionTable();
extern int DObjGetNumSurfaces();
extern int DObjGetSurfaces();
extern int __cdecl XModelGetLodForDist( const void *model, float distance );

extern double __cdecl R_GetLodDist( float *entityOrigin );

#define backEnd_modelMatrix   ((GLfloat *)(void *)&flt_16D8DE4)

/* ---- R_CullModel  0x0050DB00 ----  VERIFIED */
int __cdecl R_CullModel(int a1, int a2)
{
  if ( ((cvar_t *)r_drawXModels)->integer )
    return *(_DWORD *)(a2 + 692);
  else
    return 2;
}

/* ---- R_AddEntityDrawSurf  0x0050DB20 ----  VERIFIED */
cvar_t *__cdecl R_AddEntityDrawSurf(int a1, _DWORD *a2, int a3, int a4)
{
  _DWORD *v4;
  int v5;
  int v6;
  int *v7;
  cvar_t *result;

  v4 = (_DWORD *)(tr_refdef_entitySurfaces + 16 * tr_refdef_entitySurfaceCount);
  v5 = 0;
  v6 = storageClass;
  if ( *(_WORD *)(a1 + 6) == 0xFFFF )
  {
    LOBYTE(v5) = sys_hasSSE != 0;
    *v4 = v5 + 9;
    goto LABEL_11;
  }
  LOBYTE(v5) = sys_hasSSE != 0;
  *v4 = v5 + 4;
  if ( (a2[20] & 0x3FF7FC00) == 0 )
  {
    if ( *(_DWORD *)(a1 + 40) )
    {
      *v4 = 6;
      v6 = storageClass;
      goto LABEL_11;
    }
    if ( *(_DWORD *)(a1 + 48) )
    {
      *v4 = 8;
      v7 = *(int **)(a1 + 48);
    }
    else
    {
      if ( !*(_DWORD *)(a1 + 44) )
        goto LABEL_11;
      *v4 = 7;
      v7 = *(int **)(a1 + 44);
    }
    v6 = *v7;
  }
LABEL_11:
  v4[2] = a1;
  v4[1] = a3;
  v4[3] = a4;
  if ( tr_refdef_numDrawSurfs < 0x10000 )
  {
    *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs) = tr_shiftedEntityNumber | (((v6 << 12) | a2[18]) << 18) | 4;
    *(_DWORD *)(tr_refdef_drawSurfs + 8 * tr_refdef_numDrawSurfs++ + 4) = v4;
  }
  result = cg_shadows;
  ++tr_refdef_entitySurfaceCount;
  if ( cg_shadows->integer == 2 && a2[22] == 1084227584 )
  {
    R_AddDrawSurf(v6, tr_stencilShadowShader, (int)v4, 0, 0, 0);
    return (cvar_t *)++tr_refdef_entitySurfaceCount;
  }
  return result;
}

/* ---- R_DObjGetSurfIndex  0x0050DC40 ----  VERIFIED */
int __cdecl R_DObjGetSurfIndex(int a1, int a2)
{
  int result;

  LOWORD(a2) = *(_WORD *)(a2 + 2 * a1 + 56);
  result = (__int16)a2;
  if ( (__int16)a2 < 0 )
    return (__int16)ri_CG_GetGameModel(-a2);
  return result;
}

/* ---- RB_TransformRigidVertex  0x0050DC60 ----  VERIFIED */
float *__cdecl RB_TransformRigidVertex(
        float *result,
        float *a2,
        float *a3,
        float *a4,
        float *a5)
{
  *a4 = a2[1] * result[4] + *a2 * *result + result[8] * a2[2] + result[12];
  a4[1] = a2[1] * result[5] + result[9] * a2[2] + result[1] * *a2 + result[13];
  a4[2] = a2[1] * result[6] + result[10] * a2[2] + result[2] * *a2 + result[14];
  *a5 = a3[1] * result[4] + *a3 * *result + a3[2] * result[8];
  a5[1] = result[1] * *a3 + result[9] * a3[2] + a3[1] * result[5];
  a5[2] = result[2] * *a3 + result[10] * a3[2] + result[6] * a3[1];
  return result;
}

/* ---- RB_AccumulateWeightedPoint  0x0050DD00 ----  VERIFIED */
float *__cdecl RB_AccumulateWeightedPoint(float *result, float *a2, float *a3, float a4)
{
  *a2 = (result[4] * a3[1] + result[8] * a3[2] + *a3 * *result + result[12]) * a4 + *a2;
  a2[1] = (result[5] * a3[1] + result[9] * a3[2] + result[1] * *a3 + result[13]) * a4 + a2[1];
  a2[2] = (result[6] * a3[1] + result[10] * a3[2] + result[2] * *a3 + result[14]) * a4 + a2[2];
  return result;
}

static const float colorWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

static const int s_boneBoxEdges[12][6] =
{
	{ 0,0,0,  1,0,0 },
	{ 0,0,0,  0,1,0 },
	{ 1,1,0,  1,0,0 },
	{ 1,1,0,  0,1,0 },
	{ 0,0,1,  1,0,1 },
	{ 0,0,1,  0,1,1 },
	{ 1,1,1,  1,0,1 },
	{ 1,1,1,  0,1,1 },
	{ 0,0,0,  0,0,1 },
	{ 1,0,0,  1,0,1 },
	{ 0,1,0,  0,1,1 },
	{ 1,1,0,  1,1,1 }
};

/* ---- R_XModelDebugBoxes  0x0050DD70 ----  VERIFIED */
float *__cdecl R_XModelDebugBoxes(int a1, int a2)
{
  int v2;
  int v3;
  int v4;
  int *parts; /* retail's alloca block -- ESP at 0x0050DD9B, var_C */
  float *result;
  int v7;
  int v8;
  _DWORD *v9;
  int v10;
  _DWORD *v11;
  double v12;
  double v13;
  bool v14; // zf
  bool v15; // cc
  float color[4]; // [esp+Ch] [ebp-5Ch] BYREF
  float start[3]; // [esp+1Ch] [ebp-4Ch] BYREF
  float end[3];   // [esp+28h] [ebp-40h] BYREF
  float v23;
  float v24;
  float v25;
  float v26;
  float v27;
  float v28;
  int v29;
  _DWORD *v30;
  int v31;
  int v32;
  int *v33;
  float *v34;
  int v35;

  v2 = a1;
  v3 = *(_DWORD *)(a1 + 144);
  v4 = *(unsigned __int8 *)(v3 + 23);
  v29 = v4;
  parts = (int *)alloca(4 * v4);
  v33 = parts;
  DObjBuildPartCollisionTable(parts, v3);
  result = (float *)((*(unsigned __int8 *)(*(_DWORD *)(v2 + 144) + 80) << 6)
                   + *(_DWORD *)(*(_DWORD *)(v2 + 144) + 4)
                   + 48);
  v7 = 0;
  color[0] = 1.0f;   /* 0x50DDC6 mov [ebp+color], 3F800000h */
  color[1] = 1.0f;
  color[2] = 1.0f;
  color[3] = 0.0f;
  v35 = 0;
  if ( v4 > 0 )
  {
    result += 4;
    v34 = result;
    do
    {
      if ( ((1 << (v7 & 7)) & *(char *)((v7 >> 3) + a2)) != 0 )
      {
        v8 = *v33;
        v31 = *v33;
        v9 = (_DWORD *)s_boneBoxEdges;
        v32 = 12;
        while ( 1 )
        {
          v26 = *(float *)(v8 + 12 * *v9);
          v27 = *(float *)(v8 + 12 * v9[1] + 4);
          v28 = *(float *)(v8 + 12 * v9[2] + 8);
          v10 = v9[3];
          v11 = v9 + 3;
          v23 = v28 * result[4] + v27 * *result + v26 * *(result - 4) + result[8];
          v24 = v28 * result[5] + v27 * result[1] + v26 * *(result - 3) + result[9];
          v25 = v28 * result[6] + v27 * result[2] + v26 * *(result - 2) + result[10];
          start[0] = v25 * *(float *)(v2 + 52) + v24 * *(float *)(v2 + 40) + v23 * *(float *)(v2 + 28);
          start[1] = v25 * *(float *)(v2 + 56) + v24 * *(float *)(v2 + 44) + v23 * *(float *)(v2 + 32);
          start[2] = v25 * *(float *)(v2 + 60) + v24 * *(float *)(v2 + 48) + v23 * *(float *)(v2 + 36);
          start[0] = start[0] + *(float *)(v2 + 68);
          start[1] = start[1] + *(float *)(v2 + 72);
          start[2] = start[2] + *(float *)(v2 + 76);
          v26 = *(float *)(v8 + 12 * v10);
          v12 = v26 * *(result - 4);
          v27 = *(float *)(v8 + 12 * v11[1] + 4);
          v28 = *(float *)(v8 + 12 * v11[2] + 8);
          v13 = v28 * result[4];
          v30 = v11 + 3;
          v23 = v12 + v13 + v27 * *result + result[8];
          v24 = v26 * *(result - 3) + v28 * result[5] + v27 * result[1] + result[9];
          v25 = v26 * *(result - 2) + v28 * result[6] + v27 * result[2] + result[10];
          end[0] = v23 * *(float *)(v2 + 28) + v25 * *(float *)(v2 + 52) + v24 * *(float *)(v2 + 40);
          end[1] = v23 * *(float *)(v2 + 32) + v25 * *(float *)(v2 + 56) + v24 * *(float *)(v2 + 44);
          end[2] = v23 * *(float *)(v2 + 36) + v25 * *(float *)(v2 + 60) + v24 * *(float *)(v2 + 48);
          end[0] = end[0] + *(float *)(v2 + 68);
          end[1] = end[1] + *(float *)(v2 + 72);
          end[2] = end[2] + *(float *)(v2 + 76);
          CL_AddDebugLine(start, end, color, 0, 0, 0);
          v2 = a1;
          v14 = v32-- == 1;
          result = v34;
          if ( v14 )
            break;
          v9 = v30;
          v8 = v31;
        }
        v4 = v29;
      }
      v7 = v35 + 1;
      result += 16;
      v15 = ++v35 < v4;
      v34 = result;
      ++v33;
    }
    while ( v15 );
  }
  return result;
}

/* ---- R_XModelDebugAxes  0x0050E040 ----  VERIFIED */
int __cdecl R_XModelDebugAxes(int a1, int a2)
{
  float *ent;
  float *result;
  int v3;
  int v4;
  int v5;
  int v6;
  float *v7;
  float *v8;
  double v9;
  double v10;
  int v11;
  float *i;
  int v13;
  float v14;
  float v15;
  float v16;
  float v17;
  float v18;
  float v19;
  float start[3]; // [esp+28h] [ebp-4Ch] BYREF
  float end[3];   // [esp+34h] [ebp-40h] BYREF
  float color[5]; // [esp+40h] [ebp-34h] BYREF
  float v27[8];   // [esp+54h] [ebp-20h] BYREF

  ent = (float *)a1;
  color[4] = 6.0f;                      /* 0x50E049 */
  v27[3] = 6.0f;                        /* 0x50E04D */
  v27[7] = 6.0f;                        /* 0x50E051 */
  result = ent;
  v3 = *(_DWORD *)&ent[36];
  v4 = *(unsigned __int8 *)(v3 + 23);
  v5 = (*(unsigned __int8 *)(v3 + 80) << 6) + *(_DWORD *)(v3 + 4) + 48;
  v6 = 0;
  memset(v27, 0, 12);
  memset(&v27[4], 0, 12);
  v13 = 0;
  if ( v4 )
  {
    v7 = (float *)(v5 + 16);            /* +0x10: axis[0] is v7[-4..-2] */
    do
    {
      if ( ((1 << (v6 & 7)) & *(char *)((v6 >> 3) + a2)) != 0 )
      {
        v8 = v27;
        v11 = 0;
        for ( i = v27; ; v8 = i )
        {
          v9 = 0.0f * v7[0];
          v10 = 0.0f * v7[4];
          memset(color, 0, 16);
          color[v11] = 1.0f;
          v14 = v9 + v10 + 0.0f * *(v7 - 4) + v7[8];
          v16 = 0.0f * v7[1] + 0.0f * v7[5] + 0.0f * *(v7 - 3) + v7[9];
          v18 = 0.0f * v7[2] + 0.0f * v7[6] + 0.0f * *(v7 - 2) + v7[10];
          start[0] = v18 * result[13] + v16 * result[10] + v14 * result[7];
          start[1] = v18 * result[14] + v16 * result[11] + v14 * result[8];
          start[2] = v18 * result[15] + v16 * result[12] + v14 * result[9];
          start[0] = start[0] + result[17];
          start[1] = start[1] + result[18];
          start[2] = start[2] + result[19];
          v15 = v7[0] * v8[0] + v8[1] * v7[4] + *(v8 - 1) * *(v7 - 4) + v7[8];
          v17 = v7[1] * v8[0] + *(v8 - 1) * *(v7 - 3) + v8[1] * v7[5] + v7[9];
          v19 = *(v8 - 1) * *(v7 - 2) + v7[2] * v8[0] + v8[1] * v7[6] + v7[10];
          end[0] = v19 * result[13] + v17 * result[10] + v15 * result[7];
          end[1] = v15 * result[8] + v19 * result[14] + v17 * result[11];
          end[2] = v15 * result[9] + v19 * result[15] + v17 * result[12];
          end[0] = end[0] + result[17];
          end[1] = end[1] + result[18];
          end[2] = end[2] + result[19];
          CL_AddDebugLine(start, end, color, 0, 0, 0);
          i += 3;
          ++v11;
          result = ent;
          if ( v11 >= 3 )
            break;
        }
        v6 = v13;
      }
      ++v6;
      v7 += 16;
      v13 = v6;
    }
    while ( v6 < v4 );
  }
  return (int)result;
}

/* ---- R_XModelDebug  0x0050E310 ----  VERIFIED */
void __cdecl R_XModelDebug(int a1, int a2)
{
  int v2;

  v2 = 0;
  if ( !_stricmp(r_xdebug->string, "boxes") || !_stricmp(r_xdebug->string, "both") )
  {
    v2 = 1;
    R_XModelDebugBoxes(a1, a2);
  }
  if ( !_stricmp(r_xdebug->string, "axes") || !_stricmp(r_xdebug->string, "both") )
  {
    R_XModelDebugAxes(a1, a2);
  }
  else if ( !v2 )
  {
    Cvar_Set2("r_xdebug", &empty_string, qtrue);
    Com_Printf("boxes - show bounding boxes\n");
    Com_Printf("axes - show axes\n");
    Com_Printf("both - show bounding boxes and axes\n");
  }
}

/* ---- R_AddXModelSurfaces  0x0050E3E0 ----  VERIFIED */
void __cdecl R_AddXModelSurfaces(int a1)
{
  int integer;
  int v2;
  int v3;
  int v5;
  int v6;
  int v7;
  int v8;
  int v10;
  int *v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  bool v18; // zf
  int v19;
  int v20;
  int v21;
  int v22;
  char *v23;
  int v24;
  int v25;
  int v26;
  int v27;
  int v28;
  int v29;
  int v30;
  int v31;
  int v32;
  bool v33; // zf
  int v34;
  int v35;
  int v36;
  int v37;
  bool v38; // zf
  int v39;
  int v40;
  int *v41;
  char *v42;
  char *v43;
  int *v44;  /* surface list */
  int v45[4]; // [esp+Ch] [ebp-40h] BYREF
  int v46;
  int v47;
  float LodDist;
  int *v49;
  int v50;
  int v51;
  int v52;
  int *v53;
  int *v54;
  int i;
  int v56;

  if ( (*(_BYTE *)(a1 + 4) & 2) == 0 || (v50 = 1, tr_viewParms_isPortal) )
    v50 = 0;
  integer = r_drawXModels->integer;
  v2 = *(_DWORD *)(a1 + 144);
  v56 = v2;
  if ( integer && *(_DWORD *)(a1 + 692) != 2 )
  {
    v3 = *(unsigned __int8 *)(v2 + 22);
    v54 = (int *)alloca((4 * v3 + 7) & 0xFFFFFFF8);
    v5 = 0;
    LodDist = R_GetLodDist((float *)a1);
    if ( v3 > 0 )
    {
      v6 = v2 + 24;
      do
      {
        v7 = XModelGetLodForDist(*(int *)v6, LodDist);
        v54[v5++] = v7;
        v6 += 4;
      }
      while ( v5 < v3 );
      v2 = v56;
    }
    v8 = DObjGetNumSurfaces(v2, (int)v54);
    v52 = v8;
    if ( v8 + tr_refdef_entitySurfaceCount <= 4096 )
    {
      v44 = (int *)alloca((4 * v8 + 7) & 0xFFFFFFF8);
      v10 = v56;
      v11 = v44;
      v49 = v44;
      DObjGetSurfaces(v45, v56, v44, (int)v54);
      if ( *(_DWORD *)(a1 + 148) )
      {
        DObjBackfillParentPartBits(v10, (int)v45);
        ri_CG_DObjCalcPose(*(_DWORD *)(a1 + 148), v10, v45);
      }
      v12 = *(unsigned __int8 *)(v10 + 22) - 1;
      if ( v12 < 0 )
      {
LABEL_19:
        HIWORD(v14) = HIWORD(v52);
        v15 = 0;
        LodDist = 0.0;
        v51 = 0;
        v53 = 0;
        for ( i = 0; v15 < v52; i = v15 )
        {
          v16 = SLOWORD(v44[v15]);
          LOWORD(v14) = *(_WORD *)(v56 + 2 * v16 + 56);
          v17 = (__int16)v14;
          v18 = (__int16)v14 == 0;
          if ( (__int16)v14 < 0 )
          {
            v17 = (__int16)ri_CG_GetGameModel(-v14);
            v18 = v17 == 0;
          }
          if ( !v18 )
          {
            v19 = *(_DWORD *)(tr_models[v17] + 80);
            v20 = SHIWORD(v44[v15]);
            v21 = *(_DWORD *)(v19 + 4 * v54[v16]);
            v22 = *(unsigned __int16 *)(v21 + 2 * v20);
            if ( v22 < tr_numShaders[0] )
            {
              v23 = (char *)tr_shaders[v22];
            }
            else
            {
              ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", *(unsigned __int16 *)(v21 + 2 * v20));
              v23 = tr_defaultShader;
            }
            if ( (v23[84] & 0x18) != 0 )
              LODWORD(LodDist) = 1;
            if ( !v50 )
            {
              v24 = SLOWORD(v44[v15]);
              v25 = *(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v56 + 4 * v24 + 24) + 4)
                                                                  + 20 * (v54[v24] + 1))
                                                      + 4)
                                          + 8)
                              + 4 * SHIWORD(v44[v15]));
              R_AddEntityDrawSurf(v25, v23, v56, v24);
              v26 = *(__int16 *)(v25 + 4);
              v27 = *(__int16 *)(v25 + 2);
              v15 = i;
              v51 += v26;
              v53 = (int *)((char *)v53 + v27);
            }
          }
          HIWORD(v14) = HIWORD(v52);
          ++v15;
        }
        v28 = r_showtricounts->integer;
        if ( !v28 || (*(_BYTE *)(a1 + 4) & 0x18) != 0 )
        {
          if ( r_showsurfcounts->integer )
          {
            if ( (*(_BYTE *)(a1 + 4) & 0x18) == 0 )
            {
              v29 = v52;
              v30 = 1;
              v51 = 1;
              i = 1;
              if ( v52 > 1 )
              {
                v53 = &v44[1];
                do
                {
                  v31 = *(__int16 *)v53;
                  v47 = v31;
                  LOWORD(v31) = *(_WORD *)(v56 + 2 * v31 + 56);
                  v32 = (__int16)v31;
                  v33 = (__int16)v31 == 0;
                  if ( (__int16)v31 < 0 )
                  {
                    v32 = (__int16)ri_CG_GetGameModel(-v31);
                    v33 = v32 == 0;
                  }
                  if ( !v33 )
                  {
                    v34 = tr_models[v32];
                    v35 = 0;
                    v46 = *(_DWORD *)(v34 + 80);
                    if ( v30 > 0 )
                    {
                      do
                      {
                        v36 = SLOWORD(v11[v35]);
                        LOWORD(v34) = *(_WORD *)(v56 + 2 * v36 + 56);
                        v37 = (__int16)v34;
                        v38 = (__int16)v34 == 0;
                        if ( (__int16)v34 < 0 )
                        {
                          v37 = (__int16)ri_CG_GetGameModel(-v34);
                          v38 = v37 == 0;
                        }
                        if ( !v38 )
                        {
                          v39 = *(_DWORD *)(*(_DWORD *)(tr_models[v37] + 80) + 4 * v54[v36]);
                          v40 = SHIWORD(v11[v35]);
                          v11 = v49;
                          if ( *(_WORD *)(*(_DWORD *)(v46 + 4 * v54[v47]) + 2 * *((__int16 *)v53 + 1)) == *(_WORD *)(v39 + 2 * v40) )
                            break;
                        }
                        HIWORD(v34) = HIWORD(i);
                        ++v35;
                      }
                      while ( v35 < i );
                      v30 = i;
                    }
                    if ( v30 == v35 )
                      ++v51;
                  }
                  v29 = v52;
                  i = ++v30;
                  ++v53;
                }
                while ( v30 < v52 );
              }
              v43 = va("%i/%i", v51, v29);
              R_AddScaledDebugString(a1 + 68, (unsigned int *)colorWhite, v43);
            }
          }
        }
        else
        {
          if ( v28 == 2 )
            v41 = v53;
          else
            v41 = (int *)v51;
          v42 = va("%i", v41);
          R_AddScaledDebugString(a1 + 68, (unsigned int *)colorWhite, v42);
        }
        if ( *r_xdebug->string )
          R_XModelDebug(a1, (int)v45);
        if ( (!v50 || cg_shadows->integer > 1) && LodDist != 0.0 )
          R_SetupEntityLighting(a1, (int)&tr_refdef_x);
      }
      else
      {
        v13 = v10 + 4 * v12 + 24;
        while ( (void *)*(_DWORD *)(*(_DWORD *)v13 + 4) != (void *)xmodel_defaultCollision )
        {
          --v12;
          v13 -= 4;
          if ( v12 < 0 )
            goto LABEL_19;
        }
        if ( com_developer->integer )
        {
          R_XModelDebugBoxes(a1, (int)v45);
          R_XModelDebugAxes(a1, (int)v45);
        }
      }
    }
    else if ( com_developer->integer )
    {
      ri_Printf(0, "WARNING: MAX_ENTSURFS exceeded\n");
    }
  }
}

/* ---- RB_SetModelMatrixForRigidSurface  0x0050E830 ----  VERIFIED */
void __cdecl RB_SetModelMatrixForRigidSurface(_DWORD *a1)
{
  float *v1;
  double v2;
  double v3;
  double v4;
  double v5;
  double v6;
  double v7;
  double v8;
  double v9;
  const GLfloat *mm;   /* backEnd.orientation.modelMatrix, 0x016D8DE4 */
  GLfloat m[16]; // [esp+0h] [ebp-40h] BYREF

  v1 = (float *)(*(__int16 *)(a1[2] + 6) + (*(unsigned __int8 *)(a1[3] + a1[1] + 80) << 6) + *(_DWORD *)(a1[1] + 4) + 48);
  mm = backEnd_modelMatrix;
  m[0] = mm[0] * *v1 + mm[8] * v1[2] + mm[4] * v1[1];
  m[1] = mm[1] * *v1 + mm[9] * v1[2] + mm[5] * v1[1];
  v2 = mm[2] * *v1 + mm[10] * v1[2];
  v3 = mm[6] * v1[1];
  m[3] = 0.0;
  m[2] = v2 + v3;
  m[4] = mm[0] * v1[4] + mm[4] * v1[5] + mm[8] * v1[6];
  m[5] = mm[1] * v1[4] + mm[5] * v1[5] + mm[9] * v1[6];
  v4 = mm[2] * v1[4] + mm[6] * v1[5];
  v5 = mm[10] * v1[6];
  m[7] = 0.0;
  m[6] = v4 + v5;
  m[8] = mm[0] * v1[8] + mm[4] * v1[9] + mm[8] * v1[10];
  m[9] = mm[1] * v1[8] + mm[5] * v1[9] + mm[9] * v1[10];
  v6 = mm[2] * v1[8] + mm[6] * v1[9];
  v7 = mm[10] * v1[10];
  m[11] = 0.0;
  m[10] = v6 + v7;
  m[12] = mm[0] * v1[12] + mm[4] * v1[13] + mm[8] * v1[14] + mm[12];
  m[13] = mm[1] * v1[12] + mm[5] * v1[13] + mm[9] * v1[14] + mm[13];
  v8 = mm[2] * v1[12] + mm[6] * v1[13];
  v9 = mm[10] * v1[14];
  m[15] = 1.0;
  m[14] = v8 + v9 + mm[14];
  glLoadMatrixf(m);
}

/* ---- RB_SurfaceXModelRigid  0x0050EA40 ----  VERIFIED */
int __cdecl RB_SurfaceXModelRigid(_DWORD *a1)
{
  int v1;
  int v2;
  int v3;
  unsigned __int16 v4;
  int v5;
  int v6;
  float *v7;
  int result;
  bool v9; // zf
  float *v10;
  float *v10n; /* retail's esi = v10 + 0x68000 == &tess.normal[3*i] */
  int v11;
  float *v12;
  int v13;

  v1 = a1[2];
  v2 = *(__int16 *)(v1 + 2);
  v3 = 3 * *(__int16 *)(v1 + 4);
  v11 = a1[1];
  RB_CheckOverflow(v2, v3);
  v4 = tess_numVertexes;
  tess_numVertexes += v2;
  XSurfaceGetTris(2 * tess_numIndexes + ((int)(char *)tess_indexes), v1, v4);
  v5 = v3 + tess_numIndexes;
  v6 = v4;
  tess_numIndexes = v5;
  v7 = *(float **)(v1 + 32);
  v10 = (float *)(12 * v6 + ((int)(char *)tess_xyz));
  v10n = (float *)(12 * v6 + ((int)(char *)tess_stageNormals));
  Com_Memcpy(8 * v6 + ((int)(char *)tess_texCoords0), *(int **)(v1 + 36), 8 * v2);
  result = *(_DWORD *)(v11 + 4);
  v12 = (float *)(*(__int16 *)(v1 + 6) + (*(unsigned __int8 *)(a1[3] + v11 + 80) << 6) + result + 48);
  if ( v2 )
  {
    v13 = v2;
    do
    {
      RB_TransformRigidVertex(v12, v7 + 3, v7, v10, v10n);
      v7 += 6;
      result = v13 - 1;
      v9 = v13 == 1;
      v10 += 3;
      v10n += 3;
      --v13;
    }
    while ( !v9 );
  }
  return result;
}

/* ---- RB_SurfaceXModelRigidSSE  0x0050EB40 ----  VERIFIED */
void __cdecl RB_SurfaceXModelRigidSSE(_DWORD *a1)
{
  int v1;               /* esi -- the XSurface */
  int v2;               /* edi -- vertexCount */
  int v3;
  unsigned __int16 v4;  /* the tess base vertex, MOVZX'd at 0x50EB9B */
  const float *v5;
  const float *m;
  float *v10;
  float *v10n;
  int v13;
  int c;

  v1 = a1[2];
  v2 = *(__int16 *)(v1 + 2);
  v3 = 3 * *(__int16 *)(v1 + 4);
  RB_CheckOverflow(v2, v3);
  v4 = tess_numVertexes;
  tess_numVertexes += v2;
  XSurfaceGetTris(2 * tess_numIndexes + ((int)(char *)tess_indexes), v1, v4);
  tess_numIndexes += v3;
  v10  = (float *)(12 * v4 + ((int)(char *)tess_xyz));
  v10n = (float *)(12 * v4 + ((int)(char *)tess_stageNormals));
  v5 = *(const float **)(v1 + 32);
  Com_Memcpy((void *)(8 * v4 + ((int)(char *)tess_texCoords0)), *(int **)(v1 + 36), 8 * v2);
  m = (const float *)(*(__int16 *)(v1 + 6)
                      + (*(unsigned __int8 *)(a1[3] + a1[1] + 80) << 6)
                      + *(_DWORD *)(a1[1] + 4)
                      + 48);
  v13 = v2;
  do
  {
    for ( c = 0; c < 3; ++c )
    {
      v10n[c] = (v5[0] * m[c] + v5[1] * m[c + 4]) + v5[2] * m[c + 8];
      v10[c]  = ((v5[3] * m[c] + v5[4] * m[c + 4]) + v5[5] * m[c + 8]) + m[c + 12];
    }
    v5 += 6;
    v10 += 3;
    v10n += 3;
    --v13;
  }
  while ( v13 );
}

/* ---- RB_SurfaceXModelRigidARB  0x0050EC90 ----  [HIGH] */
int __cdecl RB_SurfaceXModelRigidARB(_DWORD *v)
{
  _DWORD *v1;
  char *v2;
  int v3;
  _DWORD *v4;
  void *v5;
  int v6;
  int v7;
  _DWORD *v8;
  int integer;
  _DWORD *v10;
  int result;
  GLsizei v12;
  GLsizei v13;
  _DWORD v14[4]; // [esp+4Ch] [ebp-10h] BYREF

  v1 = v;
  if ( r_logFile->integer )
  {
    v2 = va(
           "--- RB_SurfaceXModelRigidARB( %s ), model: %s ---\n",
           (const char *)tess_shader,
           **(const char ***)(v[1] + 4 * v[3] + 24));
    if ( Stream )
      fprintf(Stream, "%s", (int)v2);
  }
  RB_SetModelMatrixForRigidSurface(v1);
  v3 = v1[2];
  v4 = *(_DWORD **)(v3 + 40);
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1537);
  qglBindBufferARB(34962, *v4);
  qglBindBufferARB(34963, v4[1]);
  glNormalPointer(0x1406u, 32, (const GLvoid *)8);
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)0x14);
  v6 = *(_DWORD *)(tess_shader + 336);
  v7 = 0;
  v14[0] = 0;
  if ( v6 > 0 )
  {
    do
    {
      v8 = *(_DWORD **)(tess_activeStages + 4 * v7);
      GL_State(v8[417]);
      RB_SetupMultitexture(v8, (int)v14, 32);
      v12 = 3 * *(__int16 *)(v3 + 4);
      backEnd_pc_drawnIndexCount += v12;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, v12, 0x1403u, 0);
      ++v7;
    }
    while ( v7 < *(_DWORD *)(tess_shader + 336) );
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    v10 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v10[417]);
    RB_SetupMultitexture(v10, (int)v14, 32);
    if ( r_showtris->integer < 5 )
    {
      qglColor3f(tr_identityLight, 0.0f, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(3 * *(__int16 *)(v3 + 4), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v13 = 3 * *(__int16 *)(v3 + 4);
    backEnd_pc_drawnIndexCount += v13;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v13, 0x1403u, 0);
    glDepthRange(0.0, 1.0);
  }
  qglBindBufferARB(34963, 0);
  qglBindBufferARB(34962, 0);
  glLoadMatrixf(backEnd_modelMatrix);   /* 0x016D8DE4 -- see the macro at the top */
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(__int16 *)(v3 + 4) + 2 * *(__int16 *)(v3 + 4);
  result = *(__int16 *)(v3 + 2);
  backEnd_pc_vertexCount += result;
  return result;
}

/* ---- RB_SurfaceXModelRigidATI  0x0050EF20 ----  [HIGH] */
int __cdecl RB_SurfaceXModelRigidATI(_DWORD *v)
{
  _DWORD *v1;
  int v2;
  char *v3;
  int v4;
  _DWORD *v5;
  void *v6;
  int v7;
  bool v8; // cc
  int v9;
  GLenum *AnimatedImage;
  int v11;
  int integer;
  int v13;
  GLenum *v14;
  int v15;
  int v16;
  GLenum *v17;
  int v18;
  GLenum *v19;
  int result;
  GLsizei v21;
  GLsizei v22;
  GLsizei v23;
  int v24;
  const GLvoid *v25;
  const GLvoid *v26;
  const GLvoid *v27;

  v1 = v;
  v2 = 0;
  if ( r_logFile->integer )
  {
    v3 = va(
           "--- RB_SurfaceXModelRigidATI( %s ), model: %s ---\n",
           (const char *)tess_shader,
           **(const char ***)(v[1] + 4 * v[3] + 24));
    if ( Stream )
      fprintf(Stream, "%s", (int)v3);
  }
  RB_EndMultitexture();
  RB_SetModelMatrixForRigidSurface(v1);
  v4 = v1[2];
  v5 = *(_DWORD **)(v4 + 44);
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1537);
  qglArrayObjectATI(32888, 2, 5126, 32, v5[1], v5[2]);
  qglArrayObjectATI(32885, 3, 5126, 32, v5[1], v5[2] + 8);
  qglArrayObjectATI(32884, 3, 5126, 32, v5[1], v5[2] + 20);
  if ( glConfig_ATIElementArray )
  {
    glEnableClientState(0x8768u);
    v7 = tess_shader;
    v8 = *(int *)(tess_shader + 336) <= 0;
    v = 0;
    if ( !v8 )
    {
      v9 = 340;
      do
      {
        GL_State(*(_DWORD *)(*(_DWORD *)(v7 + v9) + 1668));
        AnimatedImage = (GLenum *)RB_GetAnimatedImage(tess_shader, *(_DWORD *)(tess_shader + v9) + 4, glState_currentTmu);
        GL_Bind(AnimatedImage);
        qglArrayObjectATI(34664, 1, 5123, 0, v5[1], v5[3]);
        v24 = 3 * *(__int16 *)(v4 + 4);
        backEnd_pc_drawnIndexCount += v24;
        ++backEnd_pc_drawCallCount;
        glDrawElementArrayATI(4, v24);
        v7 = tess_shader;
        v11 = *(_DWORD *)(tess_shader + 336);
        v9 += 4;
        v = (_DWORD *)((char *)v + 1);
      }
      while ( (int)v < v11 );
    }
    integer = r_showtris->integer;
    if ( integer )
    {
      if ( (integer & 1) != 0 )
        glDepthRange(0.0, 0.1000000014901161);
      GL_State(*(_DWORD *)(*(_DWORD *)(tr_showTrisShader + 340) + 1668));
      v14 = (GLenum *)RB_GetAnimatedImage(0, *(_DWORD *)(tr_showTrisShader + 340) + 4, glState_currentTmu);
      GL_Bind(v14);
      if ( r_showtris->integer < 5 )
      {
        qglColor3f(tr_identityLight, 0.0f, tr_identityLight);
      }
      else
      {
        RB_ChooseSurfaceCountColor(3 * *(__int16 *)(v4 + 4), &v);
        glColor4ubv((const GLubyte *)&v);
      }
      v25 = *(const GLvoid **)(v4 + 28);
      v21 = 3 * *(__int16 *)(v4 + 4);
      backEnd_pc_drawnIndexCount += v21;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, v21, 0x1403u, v25);
      glDepthRange(0.0, 1.0);
    }
    glDisableClientState(0x8768u);
  }
  else
  {
    v15 = tess_shader;
    if ( *(int *)(tess_shader + 336) > 0 )
    {
      v16 = 340;
      do
      {
        GL_State(*(_DWORD *)(*(_DWORD *)(v16 + v15) + 1668));
        v17 = (GLenum *)RB_GetAnimatedImage(tess_shader, *(_DWORD *)(v16 + tess_shader) + 4, glState_currentTmu);
        GL_Bind(v17);
        v26 = *(const GLvoid **)(v4 + 28);
        v22 = 3 * *(__int16 *)(v4 + 4);
        backEnd_pc_drawnIndexCount += v22;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, v22, 0x1403u, v26);
        v15 = tess_shader;
        ++v2;
        v16 += 4;
      }
      while ( v2 < *(_DWORD *)(tess_shader + 336) );
    }
    v18 = r_showtris->integer;
    if ( v18 )
    {
      if ( (v18 & 1) != 0 )
        glDepthRange(0.0, 0.1000000014901161);
      GL_State(*(_DWORD *)(*(_DWORD *)(tr_showTrisShader + 340) + 1668));
      v19 = (GLenum *)RB_GetAnimatedImage(glState_currentTmu, *(_DWORD *)(tr_showTrisShader + 340) + 4, glState_currentTmu);
      GL_Bind(v19);
      if ( r_showtris->integer < 5 )
      {
        qglColor3f(tr_identityLight, 0.0f, tr_identityLight);
      }
      else
      {
        RB_ChooseSurfaceCountColor(3 * *(__int16 *)(v4 + 4), &v);
        glColor4ubv((const GLubyte *)&v);
      }
      v27 = *(const GLvoid **)(v4 + 28);
      v23 = 3 * *(__int16 *)(v4 + 4);
      backEnd_pc_drawnIndexCount += v23;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, v23, 0x1403u, v27);
      glDepthRange(0.0, 1.0);
    }
  }
  glLoadMatrixf(backEnd_modelMatrix);   /* 0x016D8DE4 -- see the macro at the top */
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(__int16 *)(v4 + 4) + 2 * *(__int16 *)(v4 + 4);
  result = *(__int16 *)(v4 + 2) + backEnd_pc_vertexCount;
  backEnd_pc_vertexCount = result;
  return result;
}

/* ---- RB_SurfaceXModelRigidNV  0x0050F380 ----  [HIGH] */
int __cdecl RB_SurfaceXModelRigidNV(_DWORD *v)
{
  _DWORD *v1;
  char *v2;
  int v3;
  int v4;
  void *v5;
  int v6;
  int v7;
  _DWORD *v8;
  int integer;
  _DWORD *v10;
  int result;
  GLsizei v12;
  GLsizei v13;
  const GLvoid *v14;
  const GLvoid *v15;
  _DWORD v16[4]; // [esp+3Ch] [ebp-10h] BYREF

  v1 = v;
  if ( r_logFile->integer )
  {
    v2 = va(
           "--- RB_SurfaceXModelRigidNV( %s ), model: %s ---\n",
           (const char *)tess_shader,
           **(const char ***)(v[1] + 4 * v[3] + 24));
    if ( Stream )
      fprintf(Stream, "%s", (int)v2);
  }
  RB_SetModelMatrixForRigidSurface(v1);
  v3 = v1[2];
  v4 = *(_DWORD *)(v3 + 48);
  RB_SetIteratorFog();
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  GL_ClientState(1537);
  glNormalPointer(0x1406u, 32, (const GLvoid *)(*(_DWORD *)(v4 + 4) + 8));
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)(*(_DWORD *)(v4 + 4) + 20));
  v6 = *(_DWORD *)(tess_shader + 336);
  v7 = 0;
  v16[0] = *(_DWORD *)(v4 + 4);
  if ( v6 > 0 )
  {
    do
    {
      v8 = *(_DWORD **)(tess_activeStages + 4 * v7);
      GL_State(v8[417]);
      RB_SetupMultitexture(v8, (int)v16, 32);
      v14 = *(const GLvoid **)(v3 + 28);
      v12 = 3 * *(__int16 *)(v3 + 4);
      backEnd_pc_drawnIndexCount += v12;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, v12, 0x1403u, v14);
      ++v7;
    }
    while ( v7 < *(_DWORD *)(tess_shader + 336) );
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    v10 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v10[417]);
    RB_SetupMultitexture(v10, (int)v16, 32);
    if ( r_showtris->integer < 5 )
    {
      qglColor3f(tr_identityLight, 0.0f, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(3 * *(__int16 *)(v3 + 4), &v);
      glColor4ubv((const GLubyte *)&v);
    }
    v15 = *(const GLvoid **)(v3 + 28);
    v13 = 3 * *(__int16 *)(v3 + 4);
    backEnd_pc_drawnIndexCount += v13;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, v13, 0x1403u, v15);
    glDepthRange(0.0, 1.0);
  }
  glLoadMatrixf(backEnd_modelMatrix);   /* 0x016D8DE4 -- see the macro at the top */
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
  backEnd_pc_indexCount += *(__int16 *)(v3 + 4) + 2 * *(__int16 *)(v3 + 4);
  result = *(__int16 *)(v3 + 2) + backEnd_pc_vertexCount;
  backEnd_pc_vertexCount = result;
  return result;
}

/* ---- RB_SurfaceXModelWeight  0x0050F5E0 ----  VERIFIED */
int __cdecl RB_SurfaceXModelWeight(_DWORD *a1)
{
  int v2;
  int v3;
  int v4;
  unsigned __int16 v5;
  int v6;
  float *v7;
  int v8;
  int result;
  int v10;
  int v11;
  int v13;
  int v14;
  int v15;
  int v16;
  double v17;
  double v18;
  float *v19;
  double v20;
  bool v21; // zf
  float *v7n; /* retail's esi = v7 + 0x68000 == &tess.normal[3*i] */
  int v22;
  int v23;
  int v24;
  unsigned __int16 v25;
  int v26;

  v2 = a1[2];
  v3 = *(__int16 *)(v2 + 2);
  v23 = a1[1];
  v4 = 3 * *(__int16 *)(v2 + 4);
  RB_CheckOverflow(v3, v4);
  v5 = tess_numVertexes;
  tess_numVertexes += v3;
  v25 = v5;
  XSurfaceGetTris(2 * tess_numIndexes + ((int)(char *)tess_indexes), v2, v5);
  v6 = v25;
  tess_numIndexes += v4;
  v26 = *(_DWORD *)(v2 + 32);
  v7 = (float *)(12 * v6 + ((int)(char *)tess_xyz));
  v7n = (float *)(12 * v6 + ((int)(char *)tess_stageNormals));
  Com_Memcpy(8 * v6 + ((int)(char *)tess_texCoords0), *(int **)(v2 + 36), 8 * v3);
  v8 = a1[3];
  result = *(_DWORD *)(v23 + 4);
  v10 = *(_DWORD *)(v2 + 24);
  v11 = (*(unsigned __int8 *)(v8 + v23 + 80) << 6) + result + 48;
  v22 = v11;
  if ( v3 )
  {
    v24 = v3;
    do
    {
      RB_TransformRigidVertex((float *)(v11 + *(_DWORD *)(v26 + 28)), (float *)(v26 + 16), (float *)v26, v7, v7n);
      v11 = v22;
      v13 = v26;
      v14 = *(_DWORD *)(v26 + 12);
      if ( v14 )
      {
        *v7 = *(float *)(v13 + 32) * *v7;
        v7[1] = *(float *)(v13 + 32) * v7[1];
        v7[2] = *(float *)(v13 + 32) * v7[2];
        do
        {
          v16 = *(_DWORD *)(v10 + 12);
          v17 = *(float *)(v10 + 16);
          v18 = *(float *)(v16 + v22 + 32);
          v19 = (float *)(v22 + v16);
          v20 = v18 * *(float *)(v10 + 8);
          --v14;
          v10 += 20;
          *v7 = (v20 + v19[4] * *(float *)(v10 - 16) + *(float *)(v10 - 20) * *v19 + v19[12]) * v17 + *v7;
          v7[1] = (v19[1] * *(float *)(v10 - 20)
                 + v19[9] * *(float *)(v10 - 12)
                 + v19[5] * *(float *)(v10 - 16)
                 + v19[13])
                * v17
                + v7[1];
          v7[2] = (v19[2] * *(float *)(v10 - 20)
                 + v19[10] * *(float *)(v10 - 12)
                 + v19[6] * *(float *)(v10 - 16)
                 + v19[14])
                * v17
                + v7[2];
        }
        while ( v14 );
        v15 = v13 + 36;
      }
      else
      {
        v15 = v13 + 32;
      }
      v7 += 3;
      v7n += 3;
      result = v24 - 1;
      v21 = v24 == 1;
      v26 = v15;
      --v24;
    }
    while ( !v21 );
  }
  return result;
}

/* ---- RB_SurfaceXModelWeightSSE  0x0050F780 ----  VERIFIED */
void __cdecl RB_SurfaceXModelWeightSSE(_DWORD *a1)
{
  int v1;               /* esi -- the XSurface */
  int v2;               /* edi -- vertexCount */
  int v3;
  unsigned __int16 v4;  /* the tess base vertex, MOVZX'd at 0x50F7DB */
  const char *vert;     /* edi -- the blend vertex stream, surf[+20] */
  const char *wpt;
  int mbase;            /* var_10 -- dobj->skelMats + (part << 6) + 0x30 */
  const float *m;
  const float *am;
  const float *p;
  const float *q;
  float *v10;
  float *v10n;
  float pos[3];
  float w;
  float aw;
  int n;
  int v13;
  int c;

  v1 = a1[2];
  v2 = *(__int16 *)(v1 + 2);
  v3 = 3 * *(__int16 *)(v1 + 4);
  RB_CheckOverflow(v2, v3);
  v4 = tess_numVertexes;
  tess_numVertexes += v2;
  XSurfaceGetTris(2 * tess_numIndexes + ((int)(char *)tess_indexes), v1, v4);
  tess_numIndexes += v3;
  v10  = (float *)(12 * v4 + ((int)(char *)tess_xyz));
  v10n = (float *)(12 * v4 + ((int)(char *)tess_stageNormals));
  vert = *(const char **)(v1 + 32);
  Com_Memcpy((void *)(8 * v4 + ((int)(char *)tess_texCoords0)), *(int **)(v1 + 36), 8 * v2);
  mbase = (*(unsigned __int8 *)(a1[3] + a1[1] + 80) << 6) + *(_DWORD *)(a1[1] + 4) + 48;
  wpt = *(const char **)(v1 + 24);
  v13 = v2;
  do
  {
    m = (const float *)(mbase + *(_DWORD *)(vert + 28));
    p = (const float *)(vert + 16);
    q = (const float *)vert;
    for ( c = 0; c < 3; ++c )
    {
      /* origin BEFORE the z product -- 0x50F8A9 precedes 0x50F8AF */
      pos[c]  = ((p[0] * m[c] + p[1] * m[c + 4]) + m[c + 12]) + p[2] * m[c + 8];
      v10n[c] = (q[0] * m[c] + q[1] * m[c + 4]) + q[2] * m[c + 8];
    }
    n = *(_DWORD *)(vert + 12);
    if ( n )
    {
      w = *(float *)(vert + 32);
      pos[0] = pos[0] * w;
      pos[1] = pos[1] * w;
      pos[2] = pos[2] * w;
      do
      {
        am = (const float *)(mbase + *(_DWORD *)(wpt + 12));
        q  = (const float *)wpt;
        aw = *(float *)(wpt + 16);
        for ( c = 0; c < 3; ++c )
          pos[c] = ((( q[0] * am[c] + q[1] * am[c + 4]) + q[2] * am[c + 8])
                    + am[c + 12]) * aw
                   + pos[c];
        wpt += 20;
        --n;
      }
      while ( n );
      vert += 4;
    }
    v10[0] = pos[0];
    v10[1] = pos[1];
    v10[2] = pos[2];
    vert += 32;
    v10 += 3;
    v10n += 3;
    --v13;
  }
  while ( v13 );
}

