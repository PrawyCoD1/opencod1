/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_gl_types.h"
#include "tr_orientation.h"   /* backEnd.or 0x016D8DA8 -- one object, not 16 shards */
#include "tr_tess.h"

extern int GL_Bind();
extern int GL_State();
extern int RB_BeginImmediateMode();
extern int RB_glVertex3f();
extern int R_FogOn();

/* ---- R_AddEdgeDef  0x00512990 ----  [HIGH] */
int __cdecl R_AddEdgeDef(int a1, int a2, int a3)
{
  int v3;
  int result;

  v3 = dword_13FECE0[a1];
  if ( v3 != 32 )
  {
    result = 2 * (v3 + 32 * a1);
    edgeDefs[result] = a2;
    dword_11FECE4[result] = a3;
    dword_13FECE0[a1] = v3 + 1;
  }
  return result * 4;
}

/* ---- R_RenderShadowEdges  0x005129D0 ----  VERIFIED */
void R_RenderShadowEdges()
{
  int v0;
  GLsizei v1;
  int *v2;
  int v3;
  int v4;
  int *v5;
  int v6;
  double v7;
  double v8;
  double v9;
  char *v10;
  int v11;
  int v12;
  double v13;
  double v14;
  double v15;
  char *v16;
  int v17;
  double v18;
  double v19;
  double v20;
  char *v21;
  int v22;
  int v23;
  double v24;
  double v25;
  double v26;
  char *v27;
  int v28;
  int v29;
  int v30;
  int *v31;
  int hit[2];

  RB_BeginImmediateMode();
  v0 = 0;
  if ( tess_numVertexes > 0 )
  {
    v1 = rbDebug_immediateVertexCount;
    v2 = edgeDefs;
    v31 = edgeDefs;
    do
    {
      if ( dword_13FECE0[v0] > 0 )
      {
        v30 = dword_13FECE0[v0];
        do
        {
          if ( v2[1] )
          {
            v3 = *v2;
            v4 = dword_13FECE0[*v2];
            hit[0] = 0;
            hit[1] = 0;
            if ( v4 <= 0 )
              goto LABEL_12;
            v5 = &dword_11FECE4[64 * v3];
            v6 = v4;
            do
            {
              if ( *(v5 - 1) == v0 )
                ++hit[*v5];
              v5 += 2;
              --v6;
            }
            while ( v6 );
            if ( !hit[1] )
            {
LABEL_12:
              v7 = *(float *)&dword_17BFF68[v0 * tess_vertexComponentCount];
              v8 = flt_17BFF64[v0 * tess_vertexComponentCount];
              v9 = *(float *)&tess_xyz[v0 * tess_vertexComponentCount];
              mode = 5;
              if ( v1 < rbDebug_immediateVertexCapacity )
              {
                v10 = (char *)rbDebug_immediateVertices + 32 * v1;
                v10[28] = rbDebug_immediateColorR;
                v10[29] = rbDebug_immediateColorG;
                v10[30] = rbDebug_immediateColorB;
                v10[31] = rbDebug_immediateColorA;
                *((_DWORD *)v10 + 3) = rbDebug_immediateTexCoordS;
                v11 = rbDebug_immediateTexCoordT;
                *(float *)v10 = v9;
                *((_DWORD *)v10 + 4) = v11;
                *((float *)v10 + 1) = v8;
                *((float *)v10 + 2) = v7;
                v1 = ++rbDebug_immediateVertexCount;
              }
              v12 = tess_vertexComponentCount * (v0 + tess_numVertexes);
              v13 = *(float *)&dword_17BFF68[v12];
              v14 = flt_17BFF64[v12];
              v15 = *(float *)&tess_xyz[v12];
              if ( v1 < rbDebug_immediateVertexCapacity )
              {
                v16 = (char *)rbDebug_immediateVertices + 32 * v1;
                v16[28] = rbDebug_immediateColorR;
                v16[29] = rbDebug_immediateColorG;
                v16[30] = rbDebug_immediateColorB;
                v16[31] = rbDebug_immediateColorA;
                *((_DWORD *)v16 + 3) = rbDebug_immediateTexCoordS;
                v17 = rbDebug_immediateTexCoordT;
                *(float *)v16 = v15;
                *((_DWORD *)v16 + 4) = v17;
                *((float *)v16 + 1) = v14;
                *((float *)v16 + 2) = v13;
                v1 = ++rbDebug_immediateVertexCount;
              }
              v18 = *(float *)&dword_17BFF68[v3 * tess_vertexComponentCount];
              v19 = flt_17BFF64[v3 * tess_vertexComponentCount];
              v20 = *(float *)&tess_xyz[v3 * tess_vertexComponentCount];
              if ( v1 < rbDebug_immediateVertexCapacity )
              {
                v21 = (char *)rbDebug_immediateVertices + 32 * v1;
                v21[28] = rbDebug_immediateColorR;
                v21[29] = rbDebug_immediateColorG;
                v21[30] = rbDebug_immediateColorB;
                v21[31] = rbDebug_immediateColorA;
                *((_DWORD *)v21 + 3) = rbDebug_immediateTexCoordS;
                v22 = rbDebug_immediateTexCoordT;
                *(float *)v21 = v20;
                *((_DWORD *)v21 + 4) = v22;
                *((float *)v21 + 1) = v19;
                *((float *)v21 + 2) = v18;
                v1 = ++rbDebug_immediateVertexCount;
              }
              v23 = tess_vertexComponentCount * (tess_numVertexes + v3);
              v24 = *(float *)&dword_17BFF68[v23];
              v25 = flt_17BFF64[v23];
              v26 = *(float *)&tess_xyz[v23];
              if ( v1 < rbDebug_immediateVertexCapacity )
              {
                v27 = (char *)rbDebug_immediateVertices + 32 * v1;
                v27[28] = rbDebug_immediateColorR;
                v27[29] = rbDebug_immediateColorG;
                v27[30] = rbDebug_immediateColorB;
                v27[31] = rbDebug_immediateColorA;
                *((_DWORD *)v27 + 3) = rbDebug_immediateTexCoordS;
                v28 = rbDebug_immediateTexCoordT;
                *(float *)v27 = v26;
                *((_DWORD *)v27 + 4) = v28;
                *((float *)v27 + 1) = v25;
                *((float *)v27 + 2) = v24;
                v1 = ++rbDebug_immediateVertexCount;
              }
              glDrawArrays(mode, 0, v1);
              v1 = 0;
              rbDebug_immediateVertexCount = 0;
              mode = 0;
            }
          }
          v2 += 2;
          --v30;
        }
        while ( v30 );
      }
      ++v0;
      v2 = v31 + 64;
      v31 += 64;
    }
    while ( v0 < tess_numVertexes );
  }
  rbDebug_immediateModeActive = 0;
  if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
  {
    if ( r_fog->integer )
    {
      v29 = dword_16C4BB0;
      if ( (backEnd_refdef_rdflags & 8) == 0 )
        v29 = glfogNum;
      if ( v29 )
      {
        glEnable(0xB60u);
        glState_glStateBits |= 0x200000u;
      }
    }
  }
  glVertexPointer(3, 0x1406u, 0, tess_xyz);
}

/* ---- RB_ShadowTessEnd  0x00512D60 ----  [HIGH] */
void RB_ShadowTessEnd()
{
  int v0;
  int v1;
  float *v2;
  float *i;
  int v4;
  __int16 *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  float v20;
  __int16 *j;
  float v22;
  int v23;
  float v24;
  float v25;
  float v26;
  float v27;
  float v28;
  float v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;
  float v35;
  float v36;
  float v37;

  v0 = tess_numVertexes;
  if ( tess_numVertexes < 4096 && glConfig_stencilBits >= 4 )
  {
    v32 = *(float *)(backEnd_currentEntity + 164);
    v33 = *(float *)(backEnd_currentEntity + 168);
    v1 = 0;
    v34 = *(float *)(backEnd_currentEntity + 172);
    v2 = (float *)tess_xyz;
    for ( i = (float *)(4 * tess_numVertexes * tess_vertexComponentCount + ((int)(char *)tess_xyz)); v1 < tess_numVertexes; ++v1 )
    {
      v20 = v32 * 512.0;
      *i = *v2 - v20;
      v22 = v33 * 512.0;
      i[1] = v2[1] - v22;
      v24 = v34 * 512.0;
      i[2] = v2[2] - v24;
      v2 += tess_vertexComponentCount;
      i += tess_vertexComponentCount;
      v0 = tess_numVertexes;
    }
    memset(dword_13FECE0, 0, 4 * v0);
    v4 = 0;
    v23 = tess_numIndexes / 3;
    if ( tess_numIndexes / 3 > 0 )
    {
      v5 = word_17A7F62;
      for ( j = word_17A7F62; ; v5 = j )
      {
        v6 = (unsigned __int16)*v5;
        v7 = (unsigned __int16)*(v5 - 1);
        v8 = (unsigned __int16)v5[1];
        v29 = *(float *)&tess_xyz[v6 * tess_vertexComponentCount] - *(float *)&tess_xyz[v7 * tess_vertexComponentCount];
        v30 = flt_17BFF64[v6 * tess_vertexComponentCount] - flt_17BFF64[v7 * tess_vertexComponentCount];
        v31 = *(float *)&dword_17BFF68[v6 * tess_vertexComponentCount] - *(float *)&dword_17BFF68[v7 * tess_vertexComponentCount];
        v26 = *(float *)&tess_xyz[v8 * tess_vertexComponentCount] - *(float *)&tess_xyz[v7 * tess_vertexComponentCount];
        v27 = flt_17BFF64[v8 * tess_vertexComponentCount] - flt_17BFF64[v7 * tess_vertexComponentCount];
        v28 = *(float *)&dword_17BFF68[v8 * tess_vertexComponentCount] - *(float *)&dword_17BFF68[v7 * tess_vertexComponentCount];
        v35 = v28 * v30 - v27 * v31;
        v36 = v26 * v31 - v28 * v29;
        v37 = v27 * v29 - v26 * v30;
        v25 = v37 * v34 + v36 * v33 + v35 * v32;
        facing[v4] = v25 > 0.0;
        v9 = dword_13FECE0[v7];
        if ( v9 != 32 )
        {
          v10 = facing[v4];
          v11 = 2 * (v9 + 32 * v7);
          edgeDefs[v11] = v6;
          dword_11FECE4[v11] = v10;
          dword_13FECE0[v7] = v9 + 1;
        }
        v12 = dword_13FECE0[v6];
        if ( v12 != 32 )
        {
          v13 = facing[v4];
          v14 = 2 * (v12 + 32 * v6);
          edgeDefs[v14] = v8;
          dword_11FECE4[v14] = v13;
          dword_13FECE0[v6] = v12 + 1;
        }
        v15 = dword_13FECE0[v8];
        if ( v15 != 32 )
        {
          v16 = facing[v4];
          v17 = 2 * (v15 + 32 * v8);
          edgeDefs[v17] = v7;
          dword_11FECE4[v17] = v16;
          dword_13FECE0[v8] = v15 + 1;
        }
        ++v4;
        j += 3;
        if ( v4 >= v23 )
          break;
      }
    }
    GL_Bind((GLenum *)tr_whiteImage);
    GL_State(18);
    qglColor3f(0.2f, 0.2f, 0.2f);
    qglColorMask(0, 0, 0, 0);
    glEnable(0xB90u);
    glStencilFunc(0x207u, 1, 0xFFu);
    if ( glState_faceCulling != 1 )
    {
      if ( glState_faceCulling == 2 )
        glEnable(0xB44u);
      if ( backEnd_viewParms_isMirror )
        v18 = 1028;
      else
        v18 = 1029;
      qglCullFace(v18);
      glState_faceCulling = 1;
    }
    glStencilOp(0x1E00u, 0x1E00u, 0x1E02u);
    R_RenderShadowEdges();
    if ( glState_faceCulling )
    {
      if ( glState_faceCulling == 2 )
        glEnable(0xB44u);
      if ( backEnd_viewParms_isMirror )
        v19 = 1029;
      else
        v19 = 1028;
      qglCullFace(v19);
      glState_faceCulling = 0;
    }
    glStencilOp(0x1E00u, 0x1E00u, 0x1E03u);
    R_RenderShadowEdges();
    qglColorMask(1, 1, 1, 1);
  }
}

/* ---- RB_ShadowFinish  0x00513130 ----  VERIFIED */
void RB_ShadowFinish()
{
  if ( cg_shadows->integer == 2 && glConfig_stencilBits >= 4 )
  {
    glEnable(0xB90u);
    glStencilFunc(0x205u, 0, 0xFFu);
    glDisable(0x3000u);
    if ( glState_faceCulling != 2 )
    {
      glDisable(0xB44u);
      glState_faceCulling = 2;
    }
    GL_Bind((GLenum *)tr_whiteImage);
    qglLoadIdentity();
    RB_BeginImmediateMode();
    rbDebug_immediateColorR = -103;
    rbDebug_immediateColorG = -103;
    rbDebug_immediateColorB = -103;
    rbDebug_immediateColorA = -1;
    GL_State(275);
    mode = 7;
    RB_glVertex3f(-1027080192, 1120403456, -1054867456);
    RB_glVertex3f(1120403456, 1120403456, -1054867456);
    RB_glVertex3f(1120403456, -1027080192, -1054867456);
    RB_glVertex3f(-1027080192, -1027080192, -1054867456);
    glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
    rbDebug_immediateVertexCount = 0;
    mode = 0;
    glDisable(0xB90u);
    rbDebug_immediateModeActive = 0;
    R_FogOn();
    glVertexPointer(3, 0x1406u, 0, tess_xyz);
  }
}

/* ---- RB_ProjectionShadowDeform  0x00513290 ----  VERIFIED */
int RB_ProjectionShadowDeform()
{
  float *v0;
  double v1;
  double v2;
  double v3;
  double v4;
  double v5;
  int result;
  double v7;
  float v8;
  float v9;
  float v10;
  float v11;
  float v12;
  float v13;
  float i;
  float v15;
  float v16;

  v10 = flt_16D8DBC;
  v12 = flt_16D8DD4;
  v11 = flt_16D8DC8;
  v13 = *(float *)(backEnd_currentEntity + 172);
  v9 = flt_16D8DB0 - *(float *)(backEnd_currentEntity + 24);
  v0 = (float *)tess_xyz;
  v1 = *(float *)(backEnd_currentEntity + 164);
  v2 = *(float *)(backEnd_currentEntity + 168);
  v3 = v13 * flt_16D8DD4 + v2 * flt_16D8DC8 + v1 * flt_16D8DBC;
  if ( (v3 < 0.5) | __UNORDERED__(v3, 0.5) )
  {
    v4 = 0.5 - v3;
    v1 = v1 + flt_16D8DBC * v4;
    v2 = v2 + flt_16D8DC8 * v4;
    v5 = v4 * flt_16D8DD4 + v13;
    v13 = v5;
    v3 = v5 * flt_16D8DD4 + v2 * flt_16D8DC8 + v1 * flt_16D8DBC;
  }
  result = 0;
  v8 = 1.0 / v3;
  for ( i = v8 * v1; result < tess_numVertexes; v0 += tess_vertexComponentCount )
  {
    ++result;
    v7 = v12 * v0[2] + v11 * v0[1] + v10 * *v0 + v9;
    *v0 = *v0 - i * v7;
    v15 = v8 * v2;
    v0[1] = v0[1] - v15 * v7;
    v16 = v8 * v13;
    v0[2] = v0[2] - v7 * v16;
  }
  return result;
}

