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

extern int GL_Cull();
extern int GL_State();
int __fastcall RB_BuildDlightArrays( unsigned char *a1, float *a2, float *a3, int a4 );
void __cdecl RB_ComputeColors( int stage );
extern int RB_ComputeTexCoords();
extern int RB_DeformTessGeometry();
void __cdecl RB_SetIteratorFog( void );
extern int RB_SetupMultitexture();

#define tess_numVertexes            tess_numVertexes   /* 0x0197FFA8 */
#define tess_numIndexes             tess_numIndexes   /* 0x0197FFA4 */
#define tess_vertexComponentCount   tess_vertexComponentCount   /* 0x0197FF80 */
#define tess_shader                 tess_shader   /* 0x0197FF90 */
#define tess_dlightBits             tess_dlightBits   /* 0x0197FFA0 */
#define tess_activeStageCount       tess_activeStageCount   /* 0x019BFFBC */
#define tess_activeStages           tess_activeStages   /* 0x019BFFC4 */
#define tess_indexes_base           tess_indexes    /* 0x017A7F60 = tess base */

#define rb_clientStateBits          glState_clientStateBits   /* 0x016C3954 */
#define tr_dlightShader             tr_dlightShader   /* 0x016C4E14 */
#define rb_arbBuffer                backEnd_dynamicBuffer_storage   /* 0x016D93EC */
#define rb_arbBufferSize            backEnd_dynamicBuffer_capacity   /* 0x016D93F0 */
#define rb_arbBufferCursor          backEnd_dynamicBuffer_currentOffset   /* 0x016D93F4 */
#define rb_arbInterleave            tr_vboInterleaved   /* 0x016D8888 */

#define backEnd_currentEntity       backEnd_currentEntity   /* 0x016D8E5C */
#define backEnd_currentLight        backEnd_currentDlight   /* 0x016D8E60 */
#define backEnd_currentLightScale   backEnd_currentLightScale     /* 0x016D8E64 */
#define backEnd_num_dlights         backEnd_refdef_num_dlights   /* 0x016D8B1C */
#define backEnd_dlights             backEnd_refdef_dlights   /* 0x016D8B24 */

#define R_MAX_SHADER_STAGES         8
#define R_DLIGHT_STRIDE             0x88
#define SHADER_STAGE_PER_LIGHT      0x80
#define SHADER_LIGHTING_PER_ENTITY  0x80

#define RB_ARB_MAX_UPLOADS          11

void __cdecl RB_SingleStageGenericARB(GLubyte *a3, GLsizei count, GLvoid *indices);
void __cdecl RB_SingleStageGenericARB2(_DWORD *a2, GLsizei count, GLvoid *indices);

/* ---- RB_SingleStageGenericARB  0x0050C9D0 ----  VERIFIED */
void __cdecl RB_SingleStageGenericARB(GLubyte *a3, GLsizei count, GLvoid *indices)
{
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int *v10;
  int v11;
  GLsizei v12;
  int v13;
  int *v14;
  int v15;
  int i;
  _DWORD *v18;
  _DWORD *v19;
  int v20;
  int v21;
  _DWORD *v22;
  float *v23;
  int *v24;
  int v25;
  int *v26;
  int v27;
  _DWORD *v28;
  int *v29;
  int v30;
  int *v31;
  int v32;
  _DWORD *v33;
  int *v34;
  int v35;
  int *v36;
  int v37;
  int *v38;
  _DWORD *v39;
  int *v40;
  int v41;
  int *v42;
  int v43;
  _DWORD *v44;
  _DWORD *v45;
  int *v46;
  int v47;
  int v48;
  int v49;
  _DWORD *v50;
  int *v51;
  int *v52;
  int v53;
  int v54;
  int v55;
  int v56;
  int v57;
  int v58;
  int v59;
  int v60;
  int v61;
  int v62;
  int v63;
  int v64;
  int v65;
  int *v66;
  int v67;
  int v68;
  int v69;
  int v70;
  int v80;
  float *v81;
  int v82;
  int v83;
  int v84;
  _DWORD *v85;
  GLsizei v86;
  int v103;
  int v104;
  int v105;
  int v106;
  int v107;
  int v108;
  int v109;
  int v110;
  int v111;
  int v112;
  int v113;
  _DWORD *texCoordSrc[8];   /* [esp+40h] [ebp-C8h] */
  int     texCoordComps[8]; /* [esp+7Ch] [ebp-8Ch] */
  int     texCoordOfs[8];   /* [esp+9Ch] [ebp-6Ch] */
  _DWORD v114[8]; // [esp+E8h] [ebp-20h] BYREF -- 8 dword stores, not char[32]

  v5 = 0;
  v6 = 0;
  v82 = 0;
  RB_ComputeTexCoords((int)a3);   /* takes ONE argument, shader@EAX -- 0x00500400 */
  v7 = Value;
  v8 = 0;
  if ( Value > 0 )
  {
    v9 = *(_DWORD *)a3;
    v10 = (int *)(a3 + 144);
    do
    {
      if ( ((256 << v8) & v9) != 0 )
      {
        v11 = *v10;
        texCoordOfs[v8] = v5;
        texCoordComps[v8] = v11;
        v5 += 4 * v11;
        texCoordSrc[v8] = (_DWORD *)(&tess_activeTexCoords)[v8];
      }
      ++v8;
      v10 += 50;
    }
    while ( v8 < v7 );
  }
  if ( (*(_DWORD *)a3 & 0x10000) != 0 )
  {
    v6 = v5;
    v5 += 4;
    RB_ComputeColors((int)a3);   /* ONE argument, the stage -- 0x0050CA51 */
  }
  if ( (*(_DWORD *)a3 & 0x20000) != 0 )
  {
    v82 = v5;
    v5 += 12;
  }
  v12 = v5 + 4 * tess_vertexComponentCount;
  v13 = v12 * tess_numVertexes;
  v84 = v5;
  v86 = v12;
  v14 = tess_xyz;
  v80 = 0;
  v106 = v12 * tess_numVertexes;
  if ( backEnd_dynamicBuffer_storage )
  {
    qglBindBufferARB(34962, backEnd_dynamicBuffer_storage);
    if ( backEnd_dynamicBuffer_currentOffset + v13 <= backEnd_dynamicBuffer_capacity )
      v80 = backEnd_dynamicBuffer_currentOffset;
  }
  else
  {
    qglBindBufferARB(34962, ++backEnd_dynamicBuffer_frameSerial);
  }
  v15 = Value;
  for ( i = 0; i < v15; ++i )
    v114[i] = v80 + texCoordOfs[i];
  RB_SetupMultitexture(a3, (int)v114, v12);
  if ( (*(_DWORD *)a3 & 0x10000) != 0 )
  {
    if ( (glState_clientStateBits & 0x100) == 0 )
    {
      glEnableClientState(0x8076u);
      glState_clientStateBits |= 0x100u;
    }
    glColorPointer(4, 0x1401u, v12, (const GLvoid *)(v6 + v80));
  }
  else
  {
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( *((_DWORD *)a3 + 409) == 11 )
      glColor4ubv((const GLubyte *)(backEnd_currentEntity + 108));
    else
      glColor4ubv(a3 + 1664);
  }
  if ( (*(_DWORD *)a3 & 0x20000) != 0 )
  {
    if ( (glState_clientStateBits & 0x200) == 0 )
    {
      glEnableClientState(0x8075u);
      glState_clientStateBits |= 0x200u;
    }
    glNormalPointer(0x1406u, v12, (const GLvoid *)(v82 + v80));
  }
  else if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  if ( (glState_clientStateBits & 0x400) == 0 )
  {
    glEnableClientState(0x8074u);
    glState_clientStateBits |= 0x400u;
  }
  glVertexPointer(tess_vertexComponentCount, 0x1406u, v12, (const GLvoid *)(v84 + v80));
  if ( backEnd_dynamicBuffer_storage
    || (v18 = 0, v85 = 0, qglBufferDataARB(34962, v13, 0, 35040), (v19 = (_DWORD *)qglMapBufferARB(34962, 35001)) == 0) )
  {
    v19 = (_DWORD *)ri_Hunk_AllocateTempMemory(v13);
    v18 = v19;
    v85 = v19;
  }
  v20 = *(_DWORD *)a3 & 0x3FF00;
  if ( v20 == 131328 && texCoordComps[0] == 2 && tess_vertexComponentCount == 3 )
  {
    v21 = 0;
    if ( tess_numVertexes > 0 )
    {
      v22 = texCoordSrc[0];
      v23 = flt_1827F64;
      do
      {
        *v19 = *v22;
        v19[1] = v22[1];
        v24 = v19 + 1;
        v24[1] = *((_DWORD *)v23 - 1);
        ++v24;
        v24[1] = *(_DWORD *)v23;
        ++v24;
        v24[1] = *((_DWORD *)v23 + 1);
        v24 += 2;
        *v24 = *v14;
        v25 = v14[1];
        v26 = v14 + 1;
        *++v24 = v25;
        *++v24 = v26[1];
        v22 += 2;
        v19 = v24 + 1;
        v14 = v26 + 2;
        ++v21;
        v23 += 3;
      }
      while ( v21 < tess_numVertexes );
    }
  }
  else if ( v20 == 65792 && texCoordComps[0] == 2 && tess_vertexComponentCount == 3 )
  {
    v27 = 0;
    if ( tess_numVertexes > 0 )
    {
      v28 = texCoordSrc[0];
      do
      {
        *v19 = *v28;
        v19[1] = v28[1];
        v29 = v19 + 1;
        v29[1] = dword_186FF60[v27];   /* +0 shard: retail 0x0050CD7A is `mov edi, tess_stageVertexColors[edx*4]`, a DWORD load; the bare name is unsigned char[] and read one byte */
        v29 += 2;
        *v29 = *v14;
        v30 = v14[1];
        v31 = v14 + 1;
        *++v29 = v30;
        *++v29 = v31[1];
        v28 += 2;
        v19 = v29 + 1;
        v14 = v31 + 2;
        ++v27;
      }
      while ( v27 < tess_numVertexes );
    }
  }
  else if ( v20 == 256 && texCoordComps[0] == 2 && tess_vertexComponentCount == 3 )
  {
    v32 = 0;
    if ( tess_numVertexes > 0 )
    {
      v33 = texCoordSrc[0];
      do
      {
        *v19 = *v33;
        v19[1] = v33[1];
        v34 = v19 + 2;
        *v34 = *v14;
        v35 = v14[1];
        v36 = v14 + 1;
        *++v34 = v35;
        *++v34 = v36[1];
        v33 += 2;
        v19 = v34 + 1;
        v14 = v36 + 2;
        ++v32;
      }
      while ( v32 < tess_numVertexes );
    }
  }
  else if ( v20 == 66304 && texCoordComps[0] == 2 && texCoordComps[1] == 2 && tess_vertexComponentCount == 3 )
  {
    v37 = 0;
    if ( tess_numVertexes > 0 )
    {
      v38 = texCoordSrc[1];
      v39 = texCoordSrc[0];
      do
      {
        *v19 = *v39;
        v19[1] = v39[1];
        v40 = v19 + 1;
        v40[1] = *v38;
        ++v40;
        v40[1] = v38[1];
        ++v40;
        v40[1] = dword_186FF60[v37];   /* +0 shard; retail 0x0050CE94 */
        v40 += 2;
        *v40 = *v14;
        v41 = v14[1];
        v42 = v14 + 1;
        *++v40 = v41;
        *++v40 = v42[1];
        v39 += 2;
        v38 += 2;
        v19 = v40 + 1;
        v14 = v42 + 2;
        ++v37;
      }
      while ( v37 < tess_numVertexes );
    }
  }
  else
  {
    v83 = 0;
    if ( tess_numVertexes > 0 )
    {
      v43 = texCoordOfs[1];
      v112 = texCoordOfs[0] - texCoordOfs[1];
      v108 = texCoordOfs[2] - texCoordOfs[1];
      v110 = texCoordOfs[3] - texCoordOfs[1];
      v103 = texCoordOfs[4] - texCoordOfs[1];
      v105 = texCoordOfs[5] - texCoordOfs[1];
      v113 = texCoordOfs[6] - texCoordOfs[1];
      v107 = texCoordOfs[7] - texCoordOfs[1];
      v109 = v82 - texCoordOfs[1];
      v104 = v84 - texCoordOfs[1];
      v44 = texCoordSrc[7];
      v45 = (_DWORD *)((char *)v19 + texCoordOfs[1]);
      v81 = flt_1827F64;
      v111 = v6 - texCoordOfs[1];
      do
      {
        v46 = texCoordSrc[1];
        if ( (a3[1] & 1) != 0 )
        {
          *(_DWORD *)((char *)v45 + v112) = *texCoordSrc[0];
          v47 = texCoordSrc[0][1];
          ++texCoordSrc[0];
          v48 = texCoordOfs[0];
          *(_DWORD *)((char *)v19 + texCoordOfs[0] + 4) = v47;
          ++texCoordSrc[0];
          if ( texCoordComps[0] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v48 + 8) = *texCoordSrc[0]++;
            if ( texCoordComps[0] == 4 )
              *(_DWORD *)((char *)v19 + v48 + 12) = *texCoordSrc[0]++;
          }
          v46 = texCoordSrc[1];
        }
        if ( (a3[1] & 2) != 0 )
        {
          v49 = *v46;
          v50 = v46 + 1;
          *v45 = v49;
          *(_DWORD *)((char *)v19 + v43 + 4) = *v50;
          v51 = v50 + 1;
          texCoordSrc[1] = v51;
          if ( texCoordComps[1] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v43 + 8) = *v51;
            v52 = v51 + 1;
            texCoordSrc[1] = v52;
            if ( texCoordComps[1] == 4 )
            {
              *(_DWORD *)((char *)v19 + v43 + 12) = *v52;
              texCoordSrc[1] = v52 + 1;
            }
          }
        }
        if ( (*(_DWORD *)a3 & 0x400) != 0 )
        {
          *(_DWORD *)((char *)v45 + v108) = *texCoordSrc[2];
          v53 = texCoordSrc[2][1];
          ++texCoordSrc[2];
          v54 = texCoordOfs[2];
          *(_DWORD *)((char *)v19 + texCoordOfs[2] + 4) = v53;
          ++texCoordSrc[2];
          if ( texCoordComps[2] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v54 + 8) = *texCoordSrc[2]++;
            if ( texCoordComps[2] == 4 )
              *(_DWORD *)((char *)v19 + v54 + 12) = *texCoordSrc[2]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x800) != 0 )
        {
          *(_DWORD *)((char *)v45 + v110) = *texCoordSrc[3];
          v55 = texCoordSrc[3][1];
          ++texCoordSrc[3];
          v56 = texCoordOfs[3];
          *(_DWORD *)((char *)v19 + texCoordOfs[3] + 4) = v55;
          ++texCoordSrc[3];
          if ( texCoordComps[3] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v56 + 8) = *texCoordSrc[3]++;
            if ( texCoordComps[3] == 4 )
              *(_DWORD *)((char *)v19 + v56 + 12) = *texCoordSrc[3]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x1000) != 0 )
        {
          *(_DWORD *)((char *)v45 + v103) = *texCoordSrc[4];
          v57 = texCoordSrc[4][1];
          ++texCoordSrc[4];
          v58 = texCoordOfs[4];
          *(_DWORD *)((char *)v19 + texCoordOfs[4] + 4) = v57;
          ++texCoordSrc[4];
          if ( texCoordComps[4] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v58 + 8) = *texCoordSrc[4]++;
            if ( texCoordComps[4] == 4 )
              *(_DWORD *)((char *)v19 + v58 + 12) = *texCoordSrc[4]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x2000) != 0 )
        {
          *(_DWORD *)((char *)v45 + v105) = *texCoordSrc[5];
          v59 = texCoordSrc[5][1];
          ++texCoordSrc[5];
          v60 = texCoordOfs[5];
          *(_DWORD *)((char *)v19 + texCoordOfs[5] + 4) = v59;
          ++texCoordSrc[5];
          if ( texCoordComps[5] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v60 + 8) = *texCoordSrc[5]++;
            if ( texCoordComps[5] == 4 )
              *(_DWORD *)((char *)v19 + v60 + 12) = *texCoordSrc[5]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x4000) != 0 )
        {
          *(_DWORD *)((char *)v45 + v113) = *texCoordSrc[6];
          v61 = texCoordSrc[6][1];
          ++texCoordSrc[6];
          v62 = texCoordOfs[6];
          *(_DWORD *)((char *)v19 + texCoordOfs[6] + 4) = v61;
          ++texCoordSrc[6];
          if ( texCoordComps[6] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v62 + 8) = *texCoordSrc[6]++;
            if ( texCoordComps[6] == 4 )
              *(_DWORD *)((char *)v19 + v62 + 12) = *texCoordSrc[6]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x8000) != 0 )
        {
          *(_DWORD *)((char *)v45 + v107) = *v44;
          v63 = texCoordOfs[7];
          *(_DWORD *)((char *)v19 + texCoordOfs[7] + 4) = v44[1];
          v44 += 2;
          if ( texCoordComps[7] >= 3 )
          {
            *(_DWORD *)((char *)v19 + v63 + 8) = *v44++;
            if ( texCoordComps[7] == 4 )
              *(_DWORD *)((char *)v19 + v63 + 12) = *v44++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x10000) != 0 )
          *(_DWORD *)((char *)v45 + v111) = dword_186FF60[v83];   /* +0 shard; retail 0x0050D2C3 */
        if ( (*(_DWORD *)a3 & 0x20000) != 0 )
        {
          *(_DWORD *)((char *)v45 + v109) = *((_DWORD *)v81 - 1);
          v64 = v82;
          *(float *)((char *)v19 + v82 + 4) = *v81;
          *(_DWORD *)((char *)v19 + v64 + 8) = *((_DWORD *)v81 + 1);
        }
        v65 = *v14;
        v66 = v14 + 1;
        *(_DWORD *)((char *)v45 + v104) = v65;
        v67 = *v66;
        v68 = v84;
        ++v66;
        *(_DWORD *)((char *)v19 + v84 + 4) = v67;
        *(_DWORD *)((char *)v19 + v68 + 8) = *v66;
        v14 = v66 + 1;
        if ( tess_vertexComponentCount == 4 )
          *(_DWORD *)((char *)v19 + v68 + 12) = *v14++;
        v19 = (_DWORD *)((char *)v19 + v86);
        v45 = (_DWORD *)((char *)v45 + v86);
        v81 += 3;
        ++v83;
      }
      while ( v83 < tess_numVertexes );
      v18 = v85;
    }
  }
  GL_State(*((_DWORD *)a3 + 417));
  if ( backEnd_dynamicBuffer_storage )
  {
    v69 = v106;
    v70 = v80;
    qglBufferSubDataARB(34962, v80, v106, v18);
    backEnd_dynamicBuffer_currentOffset = (v69 + v70 + 31) & 0xFFFFFFE0;
LABEL_114:
    ri_Hunk_FreeTempMemory(v18);
LABEL_115:
    ++backEnd_pc_drawCallCount;
    backEnd_pc_drawnIndexCount += count;
    glDrawElements(4u, count, 0x1403u, indices);
    qglBindBufferARB(34962, 0);
    return;
  }
  if ( v18 )
  {
    qglBufferDataARB(34962, v86 * tess_numVertexes, v18, 35040);
    goto LABEL_114;
  }
  if ( (unsigned __int8)qglUnmapBufferARB(34962) )
    goto LABEL_115;
  qglBindBufferARB(34962, 0);
}

/* ---- RB_PickBufferOffsetARB  0x0050D440 ----  VERIFIED */
int __cdecl RB_PickBufferOffsetARB(int a1, int a2, int *a3)
{
  int result;

  result = *a3 + a1 > a2 ? 0 : *a3;
  *a3 = result + a1;
  return result;
}

/* ---- RB_SingleStageGenericARB2  0x0050D460 ----  VERIFIED */
void __cdecl RB_SingleStageGenericARB2(_DWORD *a2, GLsizei count, GLvoid *indices)
{
  int v4;
  int v5;
  GLubyte *v6;
  int v7;
  int v9;
  _DWORD *v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  GLint v16;
  int v17;
  BOOL v18;
  GLint v19;
  bool v20; // zf
  int v21;
  GLint v22;
  int v23;
  int v24;
  int i;
  int v27;
  int v28;
  GLint         uploadOffset[RB_ARB_MAX_UPLOADS];  /* [esp+28h] [ebp-A4h] */
  const GLvoid *uploadSource[RB_ARB_MAX_UPLOADS];  /* [esp+54h] [ebp-78h] */
  GLsizei       uploadSize[RB_ARB_MAX_UPLOADS];    /* [esp+80h] [ebp-4Ch] */
  _DWORD v30[8]; // [esp+ACh] [ebp-20h] BYREF

  v4 = backEnd_dynamicBuffer_storage;
  if ( backEnd_dynamicBuffer_storage )
  {
    v5 = backEnd_dynamicBuffer_currentOffset;
    v27 = backEnd_dynamicBuffer_capacity;
  }
  else
  {
    v5 = 0;
    v4 = backEnd_dynamicBuffer_frameSerial + 1;
    v27 = 0x7FFFFFFF;
    ++backEnd_dynamicBuffer_frameSerial;
  }
  qglBindBufferARB(34962, v4);
  v6 = (GLubyte *)a2;
  v7 = 0;
  RB_ComputeTexCoords((int)a2);   /* takes ONE argument, shader@EAX -- 0x00500400 */
  v9 = 0;
  if ( Value > 0 )
  {
    v28 = *a2;
    v10 = a2 + 36;
    do
    {
      v30[v9] = 0;
      if ( ((256 << v9) & v28) != 0 )
      {
        v11 = 4 * tess_numVertexes * *v10;
        uploadSize[v7] = v11;
        v12 = v11 + v5 > v27 ? 0 : v5;
        v6 = (GLubyte *)a2;
        v5 = v12 + v11;
        v13 = (int)*(&tess_activeTexCoords + v9);
        uploadOffset[v7] = v12;
        uploadSource[v7] = (const GLvoid *)v13;
        v30[v9] = v12;
        ++v7;
      }
      ++v9;
      v10 += 50;
    }
    while ( v9 < Value );
  }
  RB_SetupMultitexture(v6, (int)v30, 0);
  if ( (*(_DWORD *)v6 & 0x10000) != 0 )
  {
    v14 = 4 * tess_numVertexes;
    v15 = 4 * tess_numVertexes + v5;
    uploadSize[v7] = 4 * tess_numVertexes;
    uploadSource[v7] = (const GLvoid *)tess_stageVertexColors;
    v16 = v15 > v27 ? 0 : v5;
    v5 = v16 + v14;
    uploadOffset[v7] = v16;
    RB_ComputeColors((int)a2);
    if ( (glState_clientStateBits & 0x100) == 0 )
    {
      glEnableClientState(0x8076u);
      glState_clientStateBits |= 0x100u;
    }
    glColorPointer(4, 0x1401u, 0, (const GLvoid *)v16);
    v6 = (GLubyte *)a2;
    ++v7;
  }
  else
  {
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( *((_DWORD *)v6 + 409) == 11 )
      glColor4ubv((const GLubyte *)(backEnd_currentEntity + 108));
    else
      glColor4ubv(v6 + 1664);
  }
  if ( (*(_DWORD *)v6 & 0x20000) != 0 )
  {
    v17 = 12 * tess_numVertexes;
    v18 = 12 * tess_numVertexes + v5 > v27;
    uploadSize[v7] = 12 * tess_numVertexes;
    uploadSource[v7] = (const GLvoid *)tess_stageNormals;
    v19 = v18 ? 0 : v5;
    v5 = v19 + v17;
    v20 = (glState_clientStateBits & 0x200) == 0;
    uploadOffset[v7] = v19;
    if ( v20 )
    {
      glEnableClientState(0x8075u);
      glState_clientStateBits |= 0x200u;
    }
    glNormalPointer(0x1406u, 0, (const GLvoid *)v19);
    v6 = (GLubyte *)a2;
    ++v7;
  }
  else if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  v21 = 4 * tess_numVertexes * tess_vertexComponentCount;
  uploadSize[v7] = v21;
  uploadSource[v7] = (const GLvoid *)tess_xyz;
  v22 = v21 + v5 > v27 ? 0 : v5;
  v23 = v22 + v21;
  v20 = (glState_clientStateBits & 0x400) == 0;
  uploadOffset[v7] = v22;
  if ( v20 )
  {
    glEnableClientState(0x8074u);
    glState_clientStateBits |= 0x400u;
  }
  glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, (const GLvoid *)v22);
  v24 = v7 + 1;   /* `inc esi` 0x0050D707 -- the position array is upload v7 */
  if ( backEnd_dynamicBuffer_storage )
    backEnd_dynamicBuffer_currentOffset = v23;
  else
    qglBufferDataARB(34962, v23, 0, 35040);
  for ( i = 0; i < v24; ++i )
    qglBufferSubDataARB(34962, uploadOffset[i], uploadSize[i], uploadSource[i]);
  GL_State(*((_DWORD *)v6 + 417));
  backEnd_pc_drawnIndexCount += count;
  ++backEnd_pc_drawCallCount;
  glDrawElements(4u, count, 0x1403u, indices);
  qglBindBufferARB(34962, 0);
}

/* ---- RB_IterateStagesGenericARB  0x0050D7B0 ----  VERIFIED */
void __cdecl RB_IterateStagesGenericARB( void )
{
  int      i;
  GLubyte *stage;

  for ( i = 0; i < 4 * R_MAX_SHADER_STAGES; i += 4 )
  {
    stage = *(GLubyte **)( i + tess_activeStages );
    if ( !stage )
      break;

    if ( (char)*stage >= 0 )
    {
      if ( rb_arbInterleave )
        RB_SingleStageGenericARB( stage, tess_numIndexes, (GLvoid *)tess_indexes_base );
      else
        RB_SingleStageGenericARB2( (_DWORD *)stage, tess_numIndexes,
                                   (GLvoid *)tess_indexes_base );

      if ( r_lightmap->integer )
      {
        if ( stage[200] )
          break;
        if ( stage[400] )
          break;
      }
    }
  }
}

/* ---- ProjectDlightTextureARB  0x0050D820 ----  VERIFIED */
void __cdecl ProjectDlightTextureARB( void )
{
  int      i;
  int      dlightOffset;
  int      stageIndex;
  GLsizei  hitIndexCount;
  GLubyte *stage;
  GLubyte *dlightStage;
  unsigned short hitIndexes[49152];   /* 0x18004 bytes incl. 4 pad */

  if ( !backEnd_num_dlights )
    return;

  dlightOffset = 0;
  for ( i = 0; i < backEnd_num_dlights; i++, dlightOffset += R_DLIGHT_STRIDE )
  {
    if ( ( ( 1 << i ) & tess_dlightBits ) == 0 )
      continue;

    hitIndexCount = RB_BuildDlightArrays( (unsigned char *)tess_vertexColors,
                                          (float *)tess_texCoords1,
                                          (float *)( backEnd_dlights + dlightOffset ),
                                          (int)hitIndexes );
    if ( !hitIndexCount )
      continue;

    if ( *(char *)( tess_shader + 84 ) >= 0 )
    {
      dlightStage = *(GLubyte **)( tr_dlightShader + 340  );
      if ( dlightStage )
      {
        if ( rb_arbInterleave )
          RB_SingleStageGenericARB( dlightStage, hitIndexCount, hitIndexes );
        else
          RB_SingleStageGenericARB2( (_DWORD *)dlightStage, hitIndexCount, hitIndexes );
      }
    }
    else
    {
      backEnd_currentLight = backEnd_dlights + dlightOffset;
      for ( stageIndex = 0; stageIndex < 4 * R_MAX_SHADER_STAGES; stageIndex += 4 )
      {
        stage = *(GLubyte **)( stageIndex + tess_activeStages );
        if ( !stage )
          break;
        if ( (char)*stage < 0 )
        {
          if ( rb_arbInterleave )
            RB_SingleStageGenericARB( stage, hitIndexCount, hitIndexes );
          else
            RB_SingleStageGenericARB2( (_DWORD *)stage, hitIndexCount, hitIndexes );
        }
      }
    }
  }
}

/* ---- RB_StageIteratorGenericARB  0x0050D970 ----  VERIFIED */
void __cdecl RB_StageIteratorGenericARB( int portalPass )
{
  char    *logText;
  int      entity;
  int      lightCount;
  int      lightIndex;
  int      stageIndex;
  GLubyte *stage;

  if ( tess_activeStageCount <= 0 )
    return;
  if ( portalPass && !tess_dlightBits )
    return;

  if ( r_logFile->integer )
  {
    logText = va( "--- RB_StageIteratorGenericARB( %s ) ---\n",
                  (const char *)tess_shader );
    if ( Stream )
      fprintf( Stream, "%s", (int)logText );
  }

  RB_DeformTessGeometry();
  RB_SetIteratorFog();
  GL_Cull( *(_DWORD *)( tess_shader + 168  ) );

  if ( !portalPass )
    RB_IterateStagesGenericARB();

  if ( tess_dlightBits )
  {
    if ( *(float *)( tess_shader + 88  ) <= 5.0
      || ( *(_BYTE *)( tess_shader + 84 ) & SHADER_LIGHTING_PER_ENTITY ) != 0 )
    {
      if ( ( *(_DWORD *)( tess_shader + 92  ) & 0x20004 ) == 0 )
      {
        ProjectDlightTextureARB();
        return;
      }
    }
  }

  if ( ( *(_BYTE *)( tess_shader + 84 ) & SHADER_LIGHTING_PER_ENTITY ) == 0 )
    return;

  entity = backEnd_currentEntity;
  if ( !entity )
    return;
  lightCount = *(_DWORD *)( entity + 204  );
  if ( !lightCount )
    return;

  for ( lightIndex = 0; lightIndex < lightCount; lightIndex++ )
  {
    backEnd_currentLight = *(_DWORD *)( entity + 8 * lightIndex + 208  );
    if ( *(_DWORD *)backEnd_currentLight != 8  )
    {
      backEnd_currentLightScale = *(float *)( entity + 8 * lightIndex + 212  );

      for ( stageIndex = 0; stageIndex < 4 * R_MAX_SHADER_STAGES; stageIndex += 4 )
      {
        stage = *(GLubyte **)( stageIndex + tess_activeStages );
        if ( !stage )
          break;
        if ( ( *stage & SHADER_STAGE_PER_LIGHT ) != 0 )
        {
          if ( rb_arbInterleave )
            RB_SingleStageGenericARB( stage, tess_numIndexes,
                                      (GLvoid *)tess_indexes_base );
          else
            RB_SingleStageGenericARB2( (_DWORD *)stage, tess_numIndexes,
                                       (GLvoid *)tess_indexes_base );
          entity = backEnd_currentEntity;
        }
      }
    }
    lightCount = *(_DWORD *)( entity + 204 );
  }

  backEnd_currentLight = 0;
}
