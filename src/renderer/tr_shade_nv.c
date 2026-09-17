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

#define rb_nvBufferBase             backEnd_dynamicBuffer_storage   /* 0x016D93EC */
#define rb_nvBufferSize             backEnd_dynamicBuffer_capacity   /* 0x016D93F0 */
#define rb_nvBufferCursor           backEnd_dynamicBuffer_currentOffset   /* 0x016D93F4 */
#define rb_nvFenceHead              backEnd_dynamicBuffer_allocationSequence   /* 0x016D9400 */
#define rb_nvFenceTail              backEnd_dynamicBuffer_reclaimSequence   /* 0x016D9404 */
#define rb_nvBytesFree              backEnd_dynamicBuffer_freeBytes   /* 0x016D9408 */
#define rb_nvFenceRingOffset        backEnd_dynamicBuffer_allocations   /* 0x016D940C, stride 8 */
#define rb_nvFenceRingSize          dword_16D9410   /* 0x016D9410, stride 8 */

#define backEnd_currentEntity       backEnd_currentEntity   /* 0x016D8E5C */
#define backEnd_currentLight        backEnd_currentDlight   /* 0x016D8E60 */
#define backEnd_currentLightScale   backEnd_currentLightScale     /* 0x016D8E64 */
#define backEnd_num_dlights         backEnd_refdef_num_dlights   /* 0x016D8B1C */
#define backEnd_dlights             backEnd_refdef_dlights   /* 0x016D8B24 */


#define R_MAX_SHADER_STAGES         8
#define R_DLIGHT_STRIDE             0x88
#define SHADER_STAGE_PER_LIGHT      0x80
#define SHADER_LIGHTING_PER_ENTITY  0x80

void RB_FinishFenceNV( void );

/* ---- RB_FinishFenceNV  0x0050BB20 ----  VERIFIED */
void RB_FinishFenceNV( void )
{
  int slot;

  slot = rb_nvFenceTail & 0x1FF;
  if ( rb_nvFenceRingOffset[2 * slot] >= 0 )
    qglFinishFenceNV( rb_nvFenceTail );
  rb_nvBytesFree += rb_nvFenceRingSize[2 * slot];
  ++rb_nvFenceTail;
}

/* ---- RB_SelectStorageNV  0x0050BB80 ----  VERIFIED */
void __cdecl RB_SelectStorageNV(int a1)
{
  if ( glState_currentStorageMode == 3 )
  {
    glEnableClientState(0x851Du);
  }
  else if ( a1 == 3 )
  {
    if ( glConfig_NVVertexArrayRange == 1 )
      glDisableClientState(0x851Du);
    else
      glDisableClientState(0x8533u);
LABEL_8:
    glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
    glNormalPointer(0x1406u, 0, tess_stageNormals);
    glTexCoordPointer(2, 0x1406u, 0, tess_activeTexCoords);
    glColorPointer(4, 0x1401u, 0, tess_stageVertexColors);
    return;
  }
  if ( a1 == 1 )
  {
    qglVertexArrayRangeNV(tr_staticVertexMemoryPrimaryLimit, (const GLvoid *)tr_staticVertexMemoryPrimary);
    return;
  }
  if ( a1 != 2 )
    goto LABEL_8;
  qglVertexArrayRangeNV(tr_staticVertexMemorySecondaryLimit, (const GLvoid *)tr_staticVertexMemorySecondary);
}

/* ---- RB_GetBuffersNV  0x0050BC40 ----  VERIFIED */
int __cdecl RB_GetBuffersNV(int a1)
{
  signed int v1;
  int v2;
  int v3;
  int v4;
  int v5;
  bool v6; // zf

  v1 = (a1 + 31) & 0xFFFFFFE0;
  v2 = ((_WORD)backEnd_dynamicBuffer_allocationSequence++ + 1) & 0x1FF;
  if ( backEnd_dynamicBuffer_allocationSequence == backEnd_dynamicBuffer_reclaimSequence + 512 )
    RB_FinishFenceNV();
  v3 = backEnd_dynamicBuffer_currentOffset;
  if ( backEnd_dynamicBuffer_currentOffset + v1 > backEnd_dynamicBuffer_capacity )
  {
    v4 = backEnd_dynamicBuffer_capacity - backEnd_dynamicBuffer_currentOffset;
    if ( backEnd_dynamicBuffer_capacity != backEnd_dynamicBuffer_currentOffset )
    {
      dword_16D9410[2 * v2] = v4;
      backEnd_dynamicBuffer_allocations[2 * v2] = -1;
      backEnd_dynamicBuffer_freeBytes -= v4;
      v5 = ((_WORD)backEnd_dynamicBuffer_allocationSequence + 1) & 0x1FF;
      v6 = ++backEnd_dynamicBuffer_allocationSequence == backEnd_dynamicBuffer_reclaimSequence + 512;
      v2 = v5;
      if ( v6 )
        RB_FinishFenceNV();
    }
    v3 = 0;
    backEnd_dynamicBuffer_currentOffset = 0;
  }
  if ( v1 > backEnd_dynamicBuffer_freeBytes )
  {
    do
      RB_FinishFenceNV();
    while ( v1 > backEnd_dynamicBuffer_freeBytes );
    v3 = backEnd_dynamicBuffer_currentOffset;
  }
  dword_16D9410[2 * v2] = v1;
  backEnd_dynamicBuffer_allocations[2 * v2] = v3;
  backEnd_dynamicBuffer_freeBytes -= v1;
  backEnd_dynamicBuffer_currentOffset += v1;
  return backEnd_dynamicBuffer_storage + backEnd_dynamicBuffer_allocations[2 * v2];
}

/* ---- RB_SetFenceNV  0x0050BD40 ----  VERIFIED */
void RB_SetFenceNV( void )
{
  qglSetFenceNV( rb_nvFenceHead, 0x84F2  );
}

/* ---- RB_SingleStageGenericNV  0x0050BD60 ----  VERIFIED */
void __cdecl RB_SingleStageGenericNV(GLubyte *a3, GLsizei count, GLvoid *indices)
{
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int *v10;
  int v11;
  GLsizei v12;
  int *v13;
  int BuffersNV;
  int v15;
  int v16;
  int i;
  int v18;
  int v19;
  _DWORD *v20;
  float *v21;
  int *v22;
  int v23;
  int *v24;
  int v25;
  _DWORD *v26;
  int *v27;
  int v28;
  int *v29;
  int v30;
  _DWORD *v31;
  int *v32;
  int v33;
  int *v34;
  int v35;
  int *v36;
  _DWORD *v37;
  int *v38;
  int v39;
  int *v40;
  int v41;
  _DWORD *v42;
  _DWORD *v43;
  int *v44;
  int v45;
  int v46;
  int v47;
  _DWORD *v48;
  int *v49;
  int *v50;
  int v51;
  int v52;
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
  int *v64;
  int v65;
  int v66;
  int v67;
  float *v77;
  int v78;
  int v79;
  int v80;
  int v97;
  int v98;
  int v99;
  GLsizei v100;
  int v101;
  int v102;
  int v103;
  int v104;
  int v105;
  int v106;
  int v107;
  _DWORD *texCoordSrc[8];   /* [esp+20h] [ebp-BCh] */
  int     texCoordComps[8]; /* [esp+50h] [ebp-8Ch] */
  int     texCoordOfs[8];   /* [esp+70h] [ebp-6Ch] */
  _DWORD v108[8]; // [esp+BCh] [ebp-20h] BYREF

  v5 = 0;
  v6 = 0;
  v78 = 0;
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
    RB_ComputeColors((int)a3);   /* ONE argument, the stage -- 0x0050BDE1 */
  }
  if ( (*(_DWORD *)a3 & 0x20000) != 0 )
  {
    v78 = v5;
    v5 += 12;
  }
  v12 = v5 + 4 * tess_vertexComponentCount;
  v80 = v5;
  v100 = v12;
  v13 = tess_xyz;
  BuffersNV = RB_GetBuffersNV(v12 * tess_numVertexes);
  v15 = Value;
  v16 = BuffersNV;
  for ( i = 0; i < v15; ++i )
    v108[i] = v16 + texCoordOfs[i];
  RB_SetupMultitexture(a3, (int)v108, v12);
  if ( (*(_DWORD *)a3 & 0x10000) != 0 )
  {
    if ( (glState_clientStateBits & 0x100) == 0 )
    {
      glEnableClientState(0x8076u);
      glState_clientStateBits |= 0x100u;
    }
    glColorPointer(4, 0x1401u, v12, (const GLvoid *)(v16 + v6));
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
    glNormalPointer(0x1406u, v12, (const GLvoid *)(v16 + v78));
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
  glVertexPointer(tess_vertexComponentCount, 0x1406u, v12, (const GLvoid *)(v16 + v80));
  v18 = *(_DWORD *)a3 & 0x3FF00;
  if ( v18 == 131328 && texCoordComps[0] == 2 && tess_vertexComponentCount == 3 )
  {
    v19 = 0;
    if ( tess_numVertexes > 0 )
    {
      v20 = texCoordSrc[0];
      v21 = flt_1827F64;
      do
      {
        *(_DWORD *)v16 = *v20;
        *(_DWORD *)(v16 + 4) = v20[1];
        v22 = (int *)(v16 + 4);
        v22[1] = *((_DWORD *)v21 - 1);
        ++v22;
        v22[1] = *(_DWORD *)v21;
        ++v22;
        v22[1] = *((_DWORD *)v21 + 1);
        v22 += 2;
        *v22 = *v13;
        v23 = v13[1];
        v24 = v13 + 1;
        *++v22 = v23;
        *++v22 = v24[1];
        v20 += 2;
        v16 = (int)(v22 + 1);
        v13 = v24 + 2;
        ++v19;
        v21 += 3;
      }
      while ( v19 < tess_numVertexes );
    }
  }
  else if ( v18 == 65792 && texCoordComps[0] == 2 && tess_vertexComponentCount == 3 )
  {
    v25 = 0;
    if ( tess_numVertexes > 0 )
    {
      v26 = texCoordSrc[0];
      do
      {
        *(_DWORD *)v16 = *v26;
        *(_DWORD *)(v16 + 4) = v26[1];
        v27 = (int *)(v16 + 4);
        v27[1] = dword_186FF60[v25];   /* +0 shard: retail 0x0050C06A is `mov edx, tess_stageVertexColors[ecx*4]`, a DWORD load */
        v27 += 2;
        *v27 = *v13;
        v28 = v13[1];
        v29 = v13 + 1;
        *++v27 = v28;
        *++v27 = v29[1];
        v26 += 2;
        v16 = (int)(v27 + 1);
        v13 = v29 + 2;
        ++v25;
      }
      while ( v25 < tess_numVertexes );
    }
  }
  else if ( v18 == 256 && texCoordComps[0] == 2 && tess_vertexComponentCount == 3 )
  {
    v30 = 0;
    if ( tess_numVertexes > 0 )
    {
      v31 = texCoordSrc[0];
      do
      {
        *(_DWORD *)v16 = *v31;
        *(_DWORD *)(v16 + 4) = v31[1];
        v32 = (int *)(v16 + 8);
        *v32 = *v13;
        v33 = v13[1];
        v34 = v13 + 1;
        *++v32 = v33;
        *++v32 = v34[1];
        v31 += 2;
        v16 = (int)(v32 + 1);
        v13 = v34 + 2;
        ++v30;
      }
      while ( v30 < tess_numVertexes );
    }
  }
  else if ( v18 == 66304 && texCoordComps[0] == 2 && texCoordComps[1] == 2 && tess_vertexComponentCount == 3 )
  {
    v35 = 0;
    if ( tess_numVertexes > 0 )
    {
      v36 = texCoordSrc[1];
      v37 = texCoordSrc[0];
      do
      {
        *(_DWORD *)v16 = *v37;
        *(_DWORD *)(v16 + 4) = v37[1];
        v38 = (int *)(v16 + 4);
        v38[1] = *v36;
        ++v38;
        v38[1] = v36[1];
        ++v38;
        v38[1] = dword_186FF60[v35];   /* +0 shard; retail 0x0050C17E */
        v38 += 2;
        *v38 = *v13;
        v39 = v13[1];
        v40 = v13 + 1;
        *++v38 = v39;
        *++v38 = v40[1];
        v37 += 2;
        v36 += 2;
        v16 = (int)(v38 + 1);
        v13 = v40 + 2;
        ++v35;
      }
      while ( v35 < tess_numVertexes );
    }
  }
  else
  {
    v79 = 0;
    if ( tess_numVertexes > 0 )
    {
      v41 = texCoordOfs[1];
      v107 = texCoordOfs[0] - texCoordOfs[1];
      v102 = texCoordOfs[2] - texCoordOfs[1];
      v104 = texCoordOfs[3] - texCoordOfs[1];
      v106 = texCoordOfs[4] - texCoordOfs[1];
      v99 = texCoordOfs[5] - texCoordOfs[1];
      v97 = texCoordOfs[6] - texCoordOfs[1];
      v101 = texCoordOfs[7] - texCoordOfs[1];
      v103 = v78 - texCoordOfs[1];
      v98 = v80 - texCoordOfs[1];
      v42 = texCoordSrc[7];
      v43 = (_DWORD *)(texCoordOfs[1] + v16);
      v77 = flt_1827F64;
      v105 = v6 - texCoordOfs[1];
      do
      {
        v44 = texCoordSrc[1];
        if ( (a3[1] & 1) != 0 )
        {
          *(_DWORD *)((char *)v43 + v107) = *texCoordSrc[0];
          v45 = texCoordSrc[0][1];
          ++texCoordSrc[0];
          v46 = texCoordOfs[0];
          *(_DWORD *)(texCoordOfs[0] + v16 + 4) = v45;
          ++texCoordSrc[0];
          if ( texCoordComps[0] >= 3 )
          {
            *(_DWORD *)(v46 + v16 + 8) = *texCoordSrc[0]++;
            if ( texCoordComps[0] == 4 )
              *(_DWORD *)(v46 + v16 + 12) = *texCoordSrc[0]++;
          }
          v44 = texCoordSrc[1];
        }
        if ( (a3[1] & 2) != 0 )
        {
          v47 = *v44;
          v48 = v44 + 1;
          *v43 = v47;
          *(_DWORD *)(v41 + v16 + 4) = *v48;
          v49 = v48 + 1;
          texCoordSrc[1] = v49;
          if ( texCoordComps[1] >= 3 )
          {
            *(_DWORD *)(v41 + v16 + 8) = *v49;
            v50 = v49 + 1;
            texCoordSrc[1] = v50;
            if ( texCoordComps[1] == 4 )
            {
              *(_DWORD *)(v41 + v16 + 12) = *v50;
              texCoordSrc[1] = v50 + 1;
            }
          }
        }
        if ( (*(_DWORD *)a3 & 0x400) != 0 )
        {
          *(_DWORD *)((char *)v43 + v102) = *texCoordSrc[2];
          v51 = texCoordSrc[2][1];
          ++texCoordSrc[2];
          v52 = texCoordOfs[2];
          *(_DWORD *)(texCoordOfs[2] + v16 + 4) = v51;
          ++texCoordSrc[2];
          if ( texCoordComps[2] >= 3 )
          {
            *(_DWORD *)(v52 + v16 + 8) = *texCoordSrc[2]++;
            if ( texCoordComps[2] == 4 )
              *(_DWORD *)(v52 + v16 + 12) = *texCoordSrc[2]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x800) != 0 )
        {
          *(_DWORD *)((char *)v43 + v104) = *texCoordSrc[3];
          v53 = texCoordSrc[3][1];
          ++texCoordSrc[3];
          v54 = texCoordOfs[3];
          *(_DWORD *)(texCoordOfs[3] + v16 + 4) = v53;
          ++texCoordSrc[3];
          if ( texCoordComps[3] >= 3 )
          {
            *(_DWORD *)(v54 + v16 + 8) = *texCoordSrc[3]++;
            if ( texCoordComps[3] == 4 )
              *(_DWORD *)(v54 + v16 + 12) = *texCoordSrc[3]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x1000) != 0 )
        {
          *(_DWORD *)((char *)v43 + v106) = *texCoordSrc[4];
          v55 = texCoordSrc[4][1];
          ++texCoordSrc[4];
          v56 = texCoordOfs[4];
          *(_DWORD *)(texCoordOfs[4] + v16 + 4) = v55;
          ++texCoordSrc[4];
          if ( texCoordComps[4] >= 3 )
          {
            *(_DWORD *)(v56 + v16 + 8) = *texCoordSrc[4]++;
            if ( texCoordComps[4] == 4 )
              *(_DWORD *)(v56 + v16 + 12) = *texCoordSrc[4]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x2000) != 0 )
        {
          *(_DWORD *)((char *)v43 + v99) = *texCoordSrc[5];
          v57 = texCoordSrc[5][1];
          ++texCoordSrc[5];
          v58 = texCoordOfs[5];
          *(_DWORD *)(texCoordOfs[5] + v16 + 4) = v57;
          ++texCoordSrc[5];
          if ( texCoordComps[5] >= 3 )
          {
            *(_DWORD *)(v58 + v16 + 8) = *texCoordSrc[5]++;
            if ( texCoordComps[5] == 4 )
              *(_DWORD *)(v58 + v16 + 12) = *texCoordSrc[5]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x4000) != 0 )
        {
          *(_DWORD *)((char *)v43 + v97) = *texCoordSrc[6];
          v59 = texCoordSrc[6][1];
          ++texCoordSrc[6];
          v60 = texCoordOfs[6];
          *(_DWORD *)(texCoordOfs[6] + v16 + 4) = v59;
          ++texCoordSrc[6];
          if ( texCoordComps[6] >= 3 )
          {
            *(_DWORD *)(v60 + v16 + 8) = *texCoordSrc[6]++;
            if ( texCoordComps[6] == 4 )
              *(_DWORD *)(v60 + v16 + 12) = *texCoordSrc[6]++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x8000) != 0 )
        {
          *(_DWORD *)((char *)v43 + v101) = *v42;
          v61 = texCoordOfs[7];
          *(_DWORD *)(texCoordOfs[7] + v16 + 4) = v42[1];
          v42 += 2;
          if ( texCoordComps[7] >= 3 )
          {
            *(_DWORD *)(v61 + v16 + 8) = *v42++;
            if ( texCoordComps[7] == 4 )
              *(_DWORD *)(v61 + v16 + 12) = *v42++;
          }
        }
        if ( (*(_DWORD *)a3 & 0x10000) != 0 )
          *(_DWORD *)((char *)v43 + v105) = dword_186FF60[v79];   /* +0 shard; retail 0x0050C5A0 */
        if ( (*(_DWORD *)a3 & 0x20000) != 0 )
        {
          *(_DWORD *)((char *)v43 + v103) = *((_DWORD *)v77 - 1);
          v62 = v78;
          *(float *)(v16 + v78 + 4) = *v77;
          *(float *)(v16 + v62 + 8) = v77[1];
        }
        v63 = *v13;
        v64 = v13 + 1;
        *(_DWORD *)((char *)v43 + v98) = v63;
        v65 = *v64;
        v66 = v80;
        ++v64;
        *(_DWORD *)(v80 + v16 + 4) = v65;
        *(_DWORD *)(v66 + v16 + 8) = *v64;
        v13 = v64 + 1;
        if ( tess_vertexComponentCount == 4 )
          *(_DWORD *)(v16 + v66 + 12) = *v13++;
        v16 += v100;
        v43 = (_DWORD *)((char *)v43 + v100);
        v77 += 3;
        ++v79;
      }
      while ( v79 < tess_numVertexes );
    }
  }
  if ( (v16 & 0x1F) != 0 )
  {
    v67 = (32 - (v16 & 0x1F)) >> 2;
    do
    {
      *(_DWORD *)v16 = 0;
      v16 += 4;
      --v67;
    }
    while ( v67 );
  }
  GL_State(*((_DWORD *)a3 + 417));
  backEnd_pc_drawnIndexCount += count;
  ++backEnd_pc_drawCallCount;
  glDrawElements(4u, count, 0x1403u, indices);
  qglSetFenceNV( rb_nvFenceHead, 0x84F2  );
}

/* ---- RB_IterateStagesGenericNV  0x0050C6E0 ----  VERIFIED */
void __cdecl RB_IterateStagesGenericNV( void )
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
      RB_SingleStageGenericNV( stage, tess_numIndexes, (GLvoid *)tess_indexes_base );
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

/* ---- ProjectDlightTextureNV  0x0050C740 ----  VERIFIED */
void __cdecl ProjectDlightTextureNV( void )
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
  if ( !rb_nvBufferSize )
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
        RB_SingleStageGenericNV( dlightStage, hitIndexCount, hitIndexes );
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
          RB_SingleStageGenericNV( stage, hitIndexCount, hitIndexes );
      }
    }
  }
}

/* ---- RB_StageIteratorGenericNV  0x0050C870 ----  VERIFIED */
void __cdecl RB_StageIteratorGenericNV( int portalPass )
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
    logText = va( "--- RB_StageIteratorGenericNV( %s ) ---\n",
                  (const char *)tess_shader );
    if ( Stream )
      fprintf( Stream, "%s", (int)logText );
  }

  RB_DeformTessGeometry();
  RB_SetIteratorFog();
  GL_Cull( *(_DWORD *)( tess_shader + 168  ) );

  if ( !portalPass )
    RB_IterateStagesGenericNV();

  if ( tess_dlightBits )
  {
    if ( *(float *)( tess_shader + 88  ) <= 5.0
      || ( *(_BYTE *)( tess_shader + 84 ) & SHADER_LIGHTING_PER_ENTITY ) != 0 )
    {
      if ( ( *(_DWORD *)( tess_shader + 92  ) & 0x20004 ) == 0 )
      {
        ProjectDlightTextureNV();
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
          RB_SingleStageGenericNV( stage, tess_numIndexes, (GLvoid *)tess_indexes_base );
          entity = backEnd_currentEntity;
        }
      }
    }
    lightCount = *(_DWORD *)( entity + 204 );
  }

  backEnd_currentLight = 0;
}
