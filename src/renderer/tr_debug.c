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

extern int GL_Bind();
extern int GL_ClientState();
extern int GL_State();
void RB_BeginImmediateMode( void );
char *__cdecl RB_glVertex3f( int xbits, int ybits, int zbits );
int __cdecl RB_AddQuadStampExt( float *origin, float *left, float *up, int *color,
                                float s1, float t1, float s2, float t2 );
void __cdecl RB_BeginSurface( void *shader, int vertexComponentCount );
extern int RB_EndMultitexture();
void __cdecl RB_EndSurface( void );
extern int RB_SelectStorageNV();
void __cdecl RE_RegisterFont( int name, int size, int fontInfo, const char *usage );
extern int R_FogOn();
int __cdecl R_AddDebugString( int *pos, int *color, int scaleBits, char *text );

/* ---- RB_DrawDebugPolys  0x004DFD00 ----  VERIFIED */
void RB_DrawDebugPolys()
{
  int v0;
  int v1;
  int v2;
  GLenum v3;

  if ( rbDebug_polygonCount )
  {
    if ( glState_currentStorageMode != 3 )
    {
      if ( glConfig_NVVertexArrayRange )
      {
        if ( glConfig_NVVertexArrayRange == 1 )
          v3 = 34077;
        else
          v3 = 34099;
        glDisableClientState(v3);
        glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
        glNormalPointer(0x1406u, 0, tess_stageNormals);
        glTexCoordPointer(2, 0x1406u, 0, tess_activeTexCoords);
        glColorPointer(4, 0x1401u, 0, tess_stageVertexColors);
      }
      else if ( glConfig_ATIVertexArrayObject )
      {
        glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
        glNormalPointer(0x1406u, 12, tess_stageNormals);
        glTexCoordPointer(2, 0x1406u, 0, tess_activeTexCoords);
      }
      glState_currentStorageMode = 3;
    }
    GL_Bind((GLenum *)tr_whiteImage);
    GL_ClientState(1024);
    glVertexPointer(3, 0x1406u, 0, pointer);
    glLoadMatrixf(tr_viewParms_world_modelMatrix);
    glMatrixMode(0x1701u);
    glLoadMatrixf(tr_viewParms_projectionMatrix);
    glMatrixMode(0x1700u);
    if ( (glState_glStateBits & 0x200000) != 0 )
    {
      glDisable(0xB60u);
      glState_glStateBits &= ~0x200000u;
    }
    if ( qglLockArraysEXT )
      qglLockArraysEXT(0, rbDebug_polygonVertexCount);
    if ( glState_faceCulling != 2 )
    {
      glDisable(0xB44u);
      glState_faceCulling = 2;
    }
    v0 = 0;
    if ( rbDebug_polygonCount > 0 )
    {
      v1 = 0;
      do
      {
        GL_State(357);
        qglColor4fv((char *)rbDebug_polygons + v1);
        glDrawArrays(9u, *(_DWORD *)((char *)rbDebug_polygons + v1 + 16), *(_DWORD *)((char *)rbDebug_polygons + v1 + 20));
        GL_State(4352);
        glDepthRange(0.0, 0.0);
        qglColor3fv((char *)rbDebug_polygons + v1);
        glDrawArrays(9u, *(_DWORD *)((char *)rbDebug_polygons + v1 + 16), *(_DWORD *)((char *)rbDebug_polygons + v1 + 20));
        glDepthRange(0.0, 1.0);
        ++v0;
        v1 += 24;
      }
      while ( v0 < rbDebug_polygonCount );
    }
    rbDebug_polygonCount = 0;
    rbDebug_polygonVertexCount = 0;
    if ( qglUnlockArraysEXT )
      qglUnlockArraysEXT();
    if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
    {
      if ( r_fog->integer )
      {
        v2 = dword_16C4BB0;
        if ( (backEnd_refdef_rdflags & 8) == 0 )
          v2 = glfogNum;
        if ( v2 )
        {
          glEnable(0xB60u);
          glState_glStateBits |= 0x200000u;
        }
      }
    }
  }
}

/* ---- RB_DrawDebugLines  0x004DFFB0 ----  VERIFIED */
void __cdecl RB_DrawDebugLines(int a1, int a2)
{
  int v2;
  int v3;
  double v4;
  double v5;
  double v6;
  int v7;

  v2 = a1;
  if ( a1 )
  {
    GL_Bind((GLenum *)tr_whiteImage);
    RB_BeginImmediateMode();
    glLoadMatrixf(tr_viewParms_world_modelMatrix);
    glMatrixMode(0x1701u);
    glLoadMatrixf(tr_viewParms_projectionMatrix);
    glMatrixMode(0x1700u);
    if ( v2 > 0 )
    {
      v3 = a2 + 32;
      do
      {
        GL_State(*(_DWORD *)(v3 + 8) != 0 ? 256 : 65792);
        mode = 1;
        v4 = *(float *)(v3 + 4);
        v5 = *(float *)v3;
        v6 = *(float *)(v3 - 4) * 255.0;
        rbDebug_immediateColorR = (unsigned __int64)(*(float *)(v3 - 8) * 255.0);
        rbDebug_immediateColorG = (unsigned __int64)v6;
        rbDebug_immediateColorB = (unsigned __int64)(v5 * 255.0);
        rbDebug_immediateColorA = (unsigned __int64)(v4 * 255.0);
        RB_glVertex3f(*(_DWORD *)(v3 - 32), *(_DWORD *)(v3 - 28), *(_DWORD *)(v3 - 24));
        RB_glVertex3f(*(_DWORD *)(v3 - 20), *(_DWORD *)(v3 - 16), *(_DWORD *)(v3 - 12));
        glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
        v3 += 44;
        --v2;
        rbDebug_immediateVertexCount = 0;
        mode = 0;
      }
      while ( v2 );
    }
    rbDebug_immediateModeActive = 0;
    if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
    {
      if ( r_fog->integer )
      {
        v7 = dword_16C4BB0;
        if ( (backEnd_refdef_rdflags & 8) == 0 )
          v7 = glfogNum;
        if ( v7 )
        {
          glEnable(0xB60u);
          glState_glStateBits |= 0x200000u;
        }
      }
    }
    glVertexPointer(3, 0x1406u, 0, tess_xyz);
  }
}

/* ---- RB_DrawDebugStrings  0x004E0130 ----  [HIGH] */
void __cdecl RB_DrawDebugStrings(const char *a1, const char *a2, int a3)
{
  void *v4;
  int v5;
  int v6;
  double v7;
  bool v8; // c0
  bool v9; // c2
  double v10;
  bool v11; // c0
  bool v12; // c2
  double v13;
  bool v14; // c0
  bool v15; // c2
  double v16;
  unsigned __int64 v17; // rax
  double v18;
  unsigned __int8 *v19;
  int v20;
  int v21;
  char *v22;
  double v23;
  double v24;
  char v25;
  int v26; // [esp+4Ch] [ebp-68h] BYREF
  int v27; // [esp+50h] [ebp-64h] BYREF
  int v28;
  const char *v29;
  int v30;
  float v31[3]; // [esp+60h] [ebp-54h] BYREF
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
  float v44;
  float v45;
  int v46[3]; // [esp+9Ch] [ebp-18h] BYREF
  int v47[3]; // [esp+A8h] [ebp-Ch] BYREF

  v26 = -16777216;
  if ( a1 )
  {
    if ( !rbDebug_font )
    {
      rbDebug_font = Z_MallocInternal(0x5048u);
      RE_RegisterFont((int)"fonts/normalFont", 16, (int)rbDebug_font, (const char *)1);
    }
    glLoadMatrixf(tr_viewParms_world_modelMatrix);
    glMatrixMode(0x1701u);
    glLoadMatrixf(tr_viewParms_projectionMatrix);
    glMatrixMode(0x1700u);
    glDepthRange(0.0, 0.0);
    v5 = -1;
    if ( (int)a1 > 0 )
    {
      v6 = a3 + 16;
      v29 = a1;
      do
      {
        if ( (*(float *)(v6 - 4) > 1.0) | __UNORDERED__(1.0, *(float *)(v6 - 4)) || *(float *)(v6 - 4) >= 0.0 )
        {
          if ( (*(float *)(v6 - 4) > 1.0) | __UNORDERED__(1.0, *(float *)(v6 - 4)) )
            v7 = 1.0;
          else
            v7 = *(float *)(v6 - 4);
        }
        else
        {
          v7 = 0.0;
        }
        v8 = *(float *)v6 > 1.0;
        v9 = __UNORDERED__(1.0, *(float *)v6);
        LOBYTE(v27) = (unsigned __int64)(v7 * 255.0);
        if ( v8 || v9 || *(float *)v6 >= 0.0 )
        {
          if ( (*(float *)v6 > 1.0) | __UNORDERED__(1.0, *(float *)v6) )
            v10 = 1.0;
          else
            v10 = *(float *)v6;
        }
        else
        {
          v10 = 0.0;
        }
        v11 = *(float *)(v6 + 4) > 1.0;
        v12 = __UNORDERED__(1.0, *(float *)(v6 + 4));
        BYTE1(v27) = (unsigned __int64)(v10 * 255.0);
        if ( v11 || v12 || *(float *)(v6 + 4) >= 0.0 )
        {
          if ( (*(float *)(v6 + 4) > 1.0) | __UNORDERED__(1.0, *(float *)(v6 + 4)) )
            v13 = 1.0;
          else
            v13 = *(float *)(v6 + 4);
        }
        else
        {
          v13 = 0.0;
        }
        v14 = *(float *)(v6 + 8) > 1.0;
        v15 = __UNORDERED__(1.0, *(float *)(v6 + 8));
        BYTE2(v27) = (unsigned __int64)(v13 * 255.0);
        if ( v14 || v15 || *(float *)(v6 + 8) >= 0.0 )
        {
          if ( (*(float *)(v6 + 8) > 1.0) | __UNORDERED__(1.0, *(float *)(v6 + 8)) )
            v16 = 1.0;
          else
            v16 = *(float *)(v6 + 8);
        }
        else
        {
          v16 = 0.0;
        }
        v17 = (unsigned __int64)(v16 * 255.0);
        v18 = *(float *)(v6 + 12) * 0.5;
        v4 = *(void **)(v6 - 12);
        HIDWORD(v17) = *(_DWORD *)(v6 - 8);
        BYTE3(v27) = v17;
        v40 = *(float *)(v6 - 16);
        v34 = backEnd_refdef_viewaxis10 * v18;
        v41 = *(float *)&v4;
        LOBYTE(v4) = *(_BYTE *)(v6 + 16);
        v42 = *((float *)&v17 + 1);
        v35 = backEnd_refdef_viewaxis11 * v18;
        v36 = backEnd_refdef_viewaxis12 * v18;
        v37 = backEnd_refdef_viewaxis20 * v18;
        v38 = backEnd_refdef_viewaxis21 * v18;
        v39 = v18 * backEnd_refdef_viewaxis22;
        v45 = v39 + v36;
        v43 = (v37 + v34) * -2.0;
        v44 = (v38 + v35) * -2.0;
        v45 = v45 * -2.0;
        if ( (_BYTE)v4 )
        {
          v19 = (unsigned __int8 *)(v6 + 16);
          do
          {
            v20 = 80 * *v19;
            v21 = *(_DWORD *)((char *)rbDebug_font + v20 + 44);
            v22 = (char *)rbDebug_font + v20;
            if ( v5 != v21 )
            {
              v28 = v21;
              backEnd_currentEntity = (int)&backEnd_debugEntity;
              v30 = storageClass;
              if ( storageClass != glState_currentStorageMode )
              {
                if ( glConfig_NVVertexArrayRange )
                {
                  RB_SelectStorageNV(storageClass);
                }
                else if ( glConfig_ATIVertexArrayObject )
                {
                  if ( storageClass == 3 )
                  {
                    glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
                    glNormalPointer(0x1406u, 12, tess_stageNormals);
                    glTexCoordPointer(2, 0x1406u, 0, tess_activeTexCoords);
                  }
                }
                glState_currentStorageMode = v30;
              }
              RB_BeginSurface((void *)tr_shaders[v28], 3);   /* 0x004E0492: mov ecx,tr_shaders[eax*4], eax = var_60 = v28 / push 3 */
              v5 = v28;
            }
            *(float *)v47 = (double)*((int *)v22 + 5) * v34;
            *(float *)&v47[1] = (double)*((int *)v22 + 5) * v35;
            *(float *)&v47[2] = (double)*((int *)v22 + 5) * v36;
            *(float *)v46 = (double)*((int *)v22 + 6) * v37;
            *(float *)&v46[1] = (double)*((int *)v22 + 6) * v38;
            *(float *)&v46[2] = (double)*((int *)v22 + 6) * v39;
            v31[0] = v40 - (double)*((int *)v22 + 5) * v34;
            v31[1] = v41 - (double)*((int *)v22 + 5) * v35;
            v31[2] = v42 - (double)*((int *)v22 + 5) * v36;
            v31[0] = (*((float *)v22 + 2) + *((float *)v22 + 2) - (double)*((int *)v22 + 6)) * v37
                           + v31[0];
            v31[1] = (*((float *)v22 + 2) + *((float *)v22 + 2) - (double)*((int *)v22 + 6)) * v38
                           + v31[1];
            v23 = (*((float *)v22 + 2) + *((float *)v22 + 2) - (double)*((int *)v22 + 6)) * v39 + v31[2];
            v31[0] = v31[0] + v43;
            v31[1] = v31[1] + v44;
            v31[2] = v23 + v45;
            RB_AddQuadStampExt(
              v31,
              (float *)v47,
              (float *)v46,
              &v26,
              *((float *)v22 + 7),
              *((float *)v22 + 8),
              *((float *)v22 + 9),
              *((float *)v22 + 10));
            v31[0] = v31[0] - v43;
            v31[1] = v31[1] - v44;
            v31[2] = v31[2] - v45;
            RB_AddQuadStampExt(
              v31,
              (float *)v47,
              (float *)v46,
              &v27,
              *((float *)v22 + 7),
              *((float *)v22 + 8),
              *((float *)v22 + 9),
              *((float *)v22 + 10));
            v24 = *((float *)v22 + 4) * -2.0;
            v25 = *++v19;
            v40 = v34 * v24 + v40;
            v41 = v35 * v24 + v41;
            v42 = v24 * v36 + v42;
          }
          while ( v25 );
        }
        v6 += 128;
        --v29;
      }
      while ( v29 );
    }
    RB_EndSurface();
    glDepthRange(0.0, 1.0);
  }
}

/* ---- RB_AddPlumeStrings  0x004E0660 ----  VERIFIED */
int RB_AddPlumeStrings()
{
  int result;
  int v1;
  _DWORD *v2;
  int v3;
  char *v4;
  double v5;
  long double v6;
  char *v7;
  int v8;
  int i;
  float v10[3]; // [esp+10h] [ebp-Ch] BYREF

  result = rbDebug_plumeCount;
  v1 = 0;
  for ( i = 0; i < rbDebug_plumeCount; result = rbDebug_plumeCount )
  {
    v2 = (char *)rbDebug_plumes + v1;
    v3 = tr_refdef_time - *(_DWORD *)((char *)rbDebug_plumes + v1 + 32);
    if ( v3 < 0 || v3 > v2[9] )
    {
      rbDebug_plumeCount = result - 1;
      qmemcpy(v2, (char *)rbDebug_plumes + 40 * result - 40, 0x28u);
    }
    else
    {
      v2[6] = 1065353216;
      v4 = (char *)rbDebug_plumes;
      if ( 2 * v3 > *(_DWORD *)((char *)rbDebug_plumes + v1 + 36) )
      {
        *(float *)((char *)rbDebug_plumes + v1 + 24) = 2.0
                                                    - ((double)v3 + (double)v3)
                                                    / (double)*(int *)((char *)rbDebug_plumes + v1 + 36);
        v4 = (char *)rbDebug_plumes;
      }
      v5 = (double)v3;
      v8 = *(_DWORD *)&v4[v1 + 28];
      v6 = sin(0.012566371 * v5 + (double)i) * 4.0;
      v10[0] = tr_refdef_viewaxis10 * v6 + *(float *)&v4[v1];
      v10[1] = tr_refdef_viewaxis11 * v6 + *(float *)&v4[v1 + 4];
      v10[2] = tr_refdef_viewaxis12 * v6 + *(float *)&v4[v1 + 8] + v5 * 0.064000003;
      v7 = va("%i", v8);
      R_AddDebugString(v10, (char *)rbDebug_plumes + v1 + 12, 1056964608, v7);
      ++i;
      v1 += 40;
    }
  }
  return result;
}

/* ---- RB_DrawDebug  0x004E07A0 ----  VERIFIED */
void __cdecl RB_DrawDebug(const char *a1)
{
  RB_AddPlumeStrings();
  RB_DrawDebugPolys();
  RB_DrawDebugLines(rbDebug_lineCount, (int)rbDebug_lines);
  RB_DrawDebugLines(rbDebug_locatedLineCount, rbDebug_locatedLines);
  rbDebug_lineCount = 0;
  RB_DrawDebugStrings((const char *)rbDebug_stringCount, a1, (int)rbDebug_strings);
  RB_DrawDebugStrings((const char *)rbDebug_locatedStringCount, a1, rbDebug_locatedStrings);
  rbDebug_stringCount = 0;
}

/* ---- R_AddDebugPolygon  0x004E0810 ----  VERIFIED */
int __cdecl R_AddDebugPolygon(_DWORD *a1, int a2, const void *a3)
{
  int result;

  result = a2 + rbDebug_polygonVertexCount;
  if ( a2 + rbDebug_polygonVertexCount <= rbDebug_polygonVertexCapacity )
  {
    result = rbDebug_polygonCount;
    if ( rbDebug_polygonCount + 1 <= rbDebug_polygonCapacity )
    {
      if ( !rbDebug_polygons || !pointer )
      {
        rbDebug_polygons = Z_MallocInternal(24 * rbDebug_polygonCapacity);
        pointer = Z_MallocInternal(12 * rbDebug_polygonVertexCapacity);
        result = rbDebug_polygonCount;
      }
      *((_DWORD *)rbDebug_polygons + 6 * result + 4) = rbDebug_polygonVertexCount;
      *((_DWORD *)rbDebug_polygons + 6 * rbDebug_polygonCount + 5) = a2;
      *((_DWORD *)rbDebug_polygons + 6 * rbDebug_polygonCount) = *a1;
      *((_DWORD *)rbDebug_polygons + 6 * rbDebug_polygonCount + 1) = a1[1];
      *((_DWORD *)rbDebug_polygons + 6 * rbDebug_polygonCount + 2) = a1[2];
      *((_DWORD *)rbDebug_polygons + 6 * rbDebug_polygonCount + 3) = a1[3];
      result = (int)pointer;
      ++rbDebug_polygonCount;
      qmemcpy((char *)pointer + 12 * rbDebug_polygonVertexCount, a3, 4 * ((unsigned int)(12 * a2) >> 2));
      rbDebug_polygonVertexCount += a2;
    }
  }
  return result;
}

/* ---- R_AddDebugLine  0x004E0940 ----  VERIFIED */
int __cdecl R_AddDebugLine(_DWORD *a1, _DWORD *a2, _DWORD *a3)
{
  int result;
  int *v4;

  result = rbDebug_lineCount;
  if ( rbDebug_lineCount + 1 <= rbDebug_lineCapacity )
  {
    v4 = (int *)rbDebug_lines;
    if ( !rbDebug_lines )
    {
      v4 = Z_MallocInternal(44 * rbDebug_lineCapacity);
      result = rbDebug_lineCount;
      rbDebug_lines = v4;
    }
    result = (int)&v4[11 * result];
    *(_DWORD *)result = *a1;
    *(_DWORD *)(result + 4) = a1[1];
    *(_DWORD *)(result + 8) = a1[2];
    *(_DWORD *)(result + 12) = *a2;
    *(_DWORD *)(result + 16) = a2[1];
    *(_DWORD *)(result + 20) = a2[2];
    *(_DWORD *)(result + 24) = *a3;
    *(_DWORD *)(result + 28) = a3[1];
    *(_DWORD *)(result + 32) = a3[2];
    *(_DWORD *)(result + 36) = a3[3];
    *(_DWORD *)(result + 40) = 0;
    ++rbDebug_lineCount;
  }
  return result;
}

static const int r_debugBoxEdges[24] =
{
	0,1,  0,2,  0,4,  1,3,  1,5,  2,3,
	2,6,  3,7,  4,5,  4,6,  5,7,  6,7
};

/* ---- R_AddDebugBox  0x004E09C0 ----  VERIFIED */
int __cdecl R_AddDebugBox(_DWORD *a1, float *a2, float *a3)
{
  int v5;
  float *v6;
  double v7;
  double v8;
  double v9;
  unsigned int i;
  int result;
  float v12[8][3]; // [esp+10h] [ebp-60h] BYREF

  v5 = 0;
  v6 = &v12[0][1];
  do
  {
    if ( (v5 & 1) != 0 )
      v7 = *a2;
    else
      v7 = *a3;
    *(v6 - 1) = v7;
    if ( (v5 & 2) != 0 )
      v8 = a2[1];
    else
      v8 = a3[1];
    *v6 = v8;
    if ( (v5 & 4) != 0 )
      v9 = a2[2];
    else
      v9 = a3[2];
    v6[1] = v9;
    ++v5;
    v6 += 3;
  }
  while ( v5 < 8 );
  for ( i = 0; i < 24; i += 2 )
    result = R_AddDebugLine((_DWORD *)v12[r_debugBoxEdges[i]],
                            (_DWORD *)v12[r_debugBoxEdges[i + 1]], a1);
  return result;
}

/* ---- R_AddDebugString  0x004E0A40 ----  VERIFIED */
int __cdecl R_AddDebugString(int *a1, int *a2, int a3, char *Source)
{
  int v4;
  int result;
  int *v6;
  int *v7;

  v4 = rbDebug_stringCount;
  result = rbDebug_stringCount + 1;
  if ( rbDebug_stringCount + 1 <= rbDebug_stringCapacity )
  {
    v6 = (int *)rbDebug_strings;
    if ( !rbDebug_strings )
    {
      v6 = Z_MallocInternal(rbDebug_stringCapacity << 7);
      v4 = rbDebug_stringCount;
      rbDebug_strings = v6;
    }
    v7 = &v6[32 * v4];
    *v7 = *a1;
    v7[1] = a1[1];
    v7[2] = a1[2];
    v7[3] = *a2;
    v7[4] = a2[1];
    v7[5] = a2[2];
    v7[6] = a2[3];
    v7[7] = a3;
    strncpy((char *)v7 + 32, Source, 0x5Fu);
    *((_BYTE *)v7 + 127) = 0;
    return ++rbDebug_stringCount;
  }
  return result;
}

/* ---- R_AddScaledDebugString  0x004E0AD0 ----  VERIFIED */
int __cdecl R_AddScaledDebugString(int a1, int *a2, char *Source)
{
  double v4;
  float v6;
  float v7[3]; // [esp+Ch] [ebp-Ch] BYREF -- retail vec3_t

  v7[0] = *(float *)a1 - tr_viewParms_originX;
  v7[1] = *(float *)(a1 + 4) - tr_viewParms_originY;
  v7[2] = *(float *)(a1 + 8) - tr_viewParms_originZ;
  v4 = VectorNormalize(v7);
  v6 = (*(float *)&tr_viewParms_axis02 * v7[2] + *(float *)&tr_viewParms_axis01 * v7[1] + *(float *)&tr_viewParms_axis00 * v7[0] - 0.995) * v4;
  if ( (v6 < 1.0) | __UNORDERED__(v6, 1.0) )
    v6 = 1.0;
  return R_AddDebugString((int *)a1, a2, SLODWORD(v6), Source);
}

/* ---- R_InitDebug  0x004E0B70 ----  VERIFIED */
int R_InitDebug()
{
  int result;

  rbDebug_polygonVertexCapacity = 0;  rbDebug_polygonVertexCount = 0;  rbDebug_polygonCapacity = 0;  rbDebug_polygonCount = 0;
  rbDebug_polygons = 0;  rbDebug_stringCapacity = 0;  rbDebug_stringCount = 0;  rbDebug_strings = 0;
  rbDebug_font = 0;  rbDebug_locatedStringCount = 0;  rbDebug_locatedStrings = 0;  rbDebug_lineCapacity = 0;
  rbDebug_lineCount = 0;  rbDebug_lines = 0;  rbDebug_locatedLineCount = 0;  rbDebug_locatedLines = 0;
  rbDebug_plumeCount = 0;  rbDebug_plumeCapacity = 0;  rbDebug_plumes = 0;  rbDebug_immediateModeActive = 0;
  mode = 0;           rbDebug_immediateLineWidth = 0;  rbDebug_immediateTexCoordS = 0;  rbDebug_immediateTexCoordT = 0;
  rbDebug_immediateColorR = 0;   rbDebug_immediateColorG = 0;   rbDebug_immediateColorB = 0;   rbDebug_immediateColorA = 0;
  rbDebug_immediateVertexCount = 0;  rbDebug_immediateVertexCapacity = 0;  rbDebug_immediateVertices = 0;
  result = 4097;
  rbDebug_polygonVertexCapacity = 4096;
  rbDebug_stringCapacity = 4096;
  rbDebug_plumeCapacity = 4096;
  rbDebug_polygonCapacity = 512;
  rbDebug_lineCapacity = 0x4000;
  mode = 0;
  rbDebug_immediateLineWidth = 0;
  rbDebug_immediateTexCoordS = 0;
  rbDebug_immediateTexCoordT = 0;
  rbDebug_immediateColorR = 1;
  rbDebug_immediateColorG = 1;
  rbDebug_immediateColorB = 1;
  rbDebug_immediateColorA = 1;
  rbDebug_immediateVertexCapacity = 0x2000;
  return result;
}

/* ---- R_ShutdownDebug  0x004E0C00 ----  VERIFIED */
void R_ShutdownDebug()
{
  if ( rbDebug_polygons )
  {
    free(rbDebug_polygons);
    rbDebug_polygons = 0;
  }
  if ( pointer )
  {
    free(pointer);
    pointer = 0;
  }
  if ( rbDebug_strings )
  {
    free(rbDebug_strings);
    rbDebug_strings = 0;
  }
  if ( rbDebug_lines )
  {
    free(rbDebug_lines);
    rbDebug_lines = 0;
  }
  if ( rbDebug_immediateVertices )
  {
    free(rbDebug_immediateVertices);
    rbDebug_immediateVertices = 0;
  }
  if ( rbDebug_plumes )
  {
    free(rbDebug_plumes);
    rbDebug_plumes = 0;
  }
  if ( rbDebug_font )
  {
    free(rbDebug_font);
    rbDebug_font = 0;
  }
}

/* ---- RE_LocateDebugStrings  0x004E0CB0 ----  VERIFIED */
int __cdecl RE_LocateDebugStrings(int a1, int a2)
{
  int result;

  result = a1;
  rbDebug_locatedStrings = a1;
  rbDebug_locatedStringCount = a2;
  return result;
}

/* ---- RE_LocateDebugLines  0x004E0CD0 ----  VERIFIED */
int __cdecl RE_LocateDebugLines(int a1, int a2)
{
  int result;

  result = a1;
  rbDebug_locatedLines = a1;
  rbDebug_locatedLineCount = a2;
  return result;
}

/* ---- RE_AddPlume  0x004E0CF0 ----  VERIFIED */
int *__cdecl RE_AddPlume(int *a1, int a2, _DWORD *a3, int a4)
{
  int *result;
  int v5;

  result = (int *)rbDebug_plumes;
  if ( rbDebug_plumes )
  {
    v5 = rbDebug_plumeCount;
  }
  else
  {
    result = Z_MallocInternal(40 * rbDebug_plumeCapacity);
    v5 = 0;
    rbDebug_plumes = result;
    rbDebug_plumeCount = 0;
  }
  if ( v5 != rbDebug_plumeCapacity )
  {
    result[10 * v5] = *a1;
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount + 1) = a1[1];
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount + 2) = a1[2];
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount + 3) = *a3;
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount + 4) = a3[1];
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount + 5) = a3[2];
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount + 7) = a2;
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount + 8) = tr_refdef_time;
    result = (int *)rbDebug_plumes;
    *((_DWORD *)rbDebug_plumes + 10 * rbDebug_plumeCount++ + 9) = a4;
  }
  return result;
}

/* ---- RB_BeginImmediateMode  0x004E0E00 ----  VERIFIED */
void RB_BeginImmediateMode()
{
  GLenum v0;

  if ( !rbDebug_immediateVertices )
    rbDebug_immediateVertices = Z_MallocInternal(32 * rbDebug_immediateVertexCapacity);
  rbDebug_immediateColorR = -1;
  rbDebug_immediateColorG = -1;
  rbDebug_immediateColorB = -1;
  rbDebug_immediateColorA = -1;
  mode = 0;
  rbDebug_immediateLineWidth = 0;
  rbDebug_immediateTexCoordS = 0;
  rbDebug_immediateTexCoordT = 0;
  if ( glState_currentStorageMode != 3 )
  {
    if ( glConfig_NVVertexArrayRange )
    {
      if ( glConfig_NVVertexArrayRange == 1 )
        v0 = 34077;
      else
        v0 = 34099;
      glDisableClientState(v0);
      glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
      glNormalPointer(0x1406u, 0, tess_stageNormals);
      glTexCoordPointer(2, 0x1406u, 0, tess_activeTexCoords);
      glColorPointer(4, 0x1401u, 0, tess_stageVertexColors);
    }
    else if ( glConfig_ATIVertexArrayObject )
    {
      glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
      glNormalPointer(0x1406u, 12, tess_stageNormals);
      glTexCoordPointer(2, 0x1406u, 0, tess_activeTexCoords);
    }
    glState_currentStorageMode = 3;
  }
  RB_EndMultitexture();
  GL_ClientState(1281);
  if ( (glState_glStateBits & 0x200000) != 0 )
  {
    glDisable(0xB60u);
    glState_glStateBits &= ~0x200000u;
  }
  glTexCoordPointer(2, 0x1406u, 32, (char *)rbDebug_immediateVertices + 12);
  glColorPointer(4, 0x1401u, 32, (char *)rbDebug_immediateVertices + 28);
  qglNormal3f(0.0f, 0.0f, 1.0f);
  glVertexPointer(3, 0x1406u, 32, rbDebug_immediateVertices);
  rbDebug_immediateModeActive = 1;
}

/* ---- RB_EndImmediateMode  0x004E0FD0 ----  VERIFIED */
void RB_EndImmediateMode()
{
  rbDebug_immediateModeActive = 0;
  R_FogOn();
  glVertexPointer(3, 0x1406u, 0, tess_xyz);
}

/* ---- RB_glBegin  0x004E1000 ----  VERIFIED */
GLenum __cdecl RB_glBegin(GLenum result)
{
  mode = result;
  return result;
}

/* ---- RB_glEnd  0x004E1010 ----  VERIFIED */
void RB_glEnd()
{
  glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
  rbDebug_immediateVertexCount = 0;
  mode = 0;
}

/* ---- RB_glVertex2i  0x004E1040 ----  VERIFIED */
char *__cdecl RB_glVertex2i(int a1, int a2)
{
  int v3;
  int v4;

  *(float *)&v4 = (float)a2;
  *(float *)&v3 = (float)a1;
  return RB_glVertex3f(v3, v4, 0);
}

/* ---- RB_glVertex2f  0x004E1060 ----  VERIFIED */
char *__cdecl RB_glVertex2f(int a1, int a2)
{
  return RB_glVertex3f(a1, a2, 0);
}

/* ---- RB_glVertex2fv  0x004E1080 ----  VERIFIED */
char *__cdecl RB_glVertex2fv(int *a1)
{
  return RB_glVertex3f(*a1, a1[1], 0);
}

/* ---- RB_glVertex3f  0x004E10A0 ----  VERIFIED */
char *__cdecl RB_glVertex3f( int xbits, int ybits, int zbits )
{
	char *result;

	result = (char *)rbDebug_immediateVertexCount;
	if ( rbDebug_immediateVertexCount < rbDebug_immediateVertexCapacity )
	{
		result = (char *)rbDebug_immediateVertices + 32 * rbDebug_immediateVertexCount;
		result[28] = rbDebug_immediateColorR;
		result[29] = rbDebug_immediateColorG;
		result[30] = rbDebug_immediateColorB;
		result[31] = rbDebug_immediateColorA;
		*((_DWORD *)result + 3) = rbDebug_immediateTexCoordS;
		*((_DWORD *)result + 4) = rbDebug_immediateTexCoordT;
		*(_DWORD *)result       = xbits;
		*((_DWORD *)result + 1) = ybits;
		*((_DWORD *)result + 2) = zbits;
		++rbDebug_immediateVertexCount;
	}
	return result;
}

/* ---- RB_glVertex3fv  0x004E1110 ----  VERIFIED */
char *__cdecl RB_glVertex3fv(int *a1)
{
  return RB_glVertex3f(*a1, a1[1], a1[2]);
}

/* ---- RB_glTexCoord2f  0x004E1130 ----  VERIFIED */
int __cdecl RB_glTexCoord2f(int a1, int a2)
{
  int result;

  result = a1;
  rbDebug_immediateTexCoordS = a1;
  rbDebug_immediateTexCoordT = a2;
  return result;
}

/* ---- RB_glTexCoord2fv  0x004E1150 ----  VERIFIED */
int *__cdecl RB_glTexCoord2fv(int *result)
{
  rbDebug_immediateTexCoordS = *result;
  rbDebug_immediateTexCoordT = result[1];
  return result;
}

/* ---- RB_glColor3f  0x004E1170 ----  VERIFIED */
unsigned __int64 __cdecl RB_glColor3f(float a1, float a2, float a3)
{
  unsigned __int64 result; // rax

  rbDebug_immediateColorR = (unsigned __int64)(a1 * 255.0);
  rbDebug_immediateColorG = (unsigned __int64)(a2 * 255.0);
  result = (unsigned __int64)(a3 * 255.0);
  rbDebug_immediateColorB = result;
  rbDebug_immediateColorA = -1;
  return result;
}

/* ---- RB_glColor4f  0x004E11C0 ----  VERIFIED */
unsigned __int64 __cdecl RB_glColor4f(float a1, float a2, float a3, float a4)
{
  unsigned __int64 result; // rax

  rbDebug_immediateColorR = (unsigned __int64)(a1 * 255.0);
  rbDebug_immediateColorG = (unsigned __int64)(a2 * 255.0);
  rbDebug_immediateColorB = (unsigned __int64)(a3 * 255.0);
  result = (unsigned __int64)(a4 * 255.0);
  rbDebug_immediateColorA = result;
  return result;
}

/* ---- RB_glColor3fv  0x004E1220 ----  VERIFIED */
unsigned __int64 __cdecl RB_glColor3fv(float *a1)
{
  double v1;
  double v2;
  unsigned __int64 result; // rax

  v1 = a1[2];
  v2 = a1[1];
  rbDebug_immediateColorR = (unsigned __int64)(*a1 * 255.0);
  rbDebug_immediateColorG = (unsigned __int64)(v2 * 255.0);
  result = (unsigned __int64)(v1 * 255.0);
  rbDebug_immediateColorB = result;
  rbDebug_immediateColorA = -1;
  return result;
}

/* ---- RB_glColor4fv  0x004E1260 ----  VERIFIED */
unsigned __int64 __cdecl RB_glColor4fv(float *a1)
{
  double v1;
  double v2;
  double v3;
  unsigned __int64 result; // rax

  v1 = a1[3];
  v2 = a1[2];
  v3 = a1[1];
  rbDebug_immediateColorR = (unsigned __int64)(*a1 * 255.0);
  rbDebug_immediateColorG = (unsigned __int64)(v3 * 255.0);
  rbDebug_immediateColorB = (unsigned __int64)(v2 * 255.0);
  result = (unsigned __int64)(v1 * 255.0);
  rbDebug_immediateColorA = result;
  return result;
}

/* ---- RB_glLineWidth  0x004E12B0 ----  VERIFIED */
void __cdecl RB_glLineWidth(int a1)
{
  if ( !((*(float *)&rbDebug_immediateLineWidth == *(float *)&a1) | __UNORDERED__(*(float *)&rbDebug_immediateLineWidth, *(float *)&a1)) )
  {
    rbDebug_immediateLineWidth = a1;
    qglLineWidth(*(float *)&a1);
  }
}

/* ---- R_BoxBehindPlane  0x004E12E0 ----  VERIFIED */
BOOL __cdecl R_BoxBehindPlane(int a1, int a2)
{
  double v2;

  v2 = *(float *)(*(unsigned __int8 *)(a1 + 18) + a2) * *(float *)(a1 + 8)
     + *(float *)(*(unsigned __int8 *)(a1 + 17) + a2) * *(float *)(a1 + 4)
     + *(float *)(*(unsigned __int8 *)(a1 + 16) + a2) * *(float *)a1;
  return ((v2 < *(float *)(a1 + 12)) | __UNORDERED__(v2, *(float *)(a1 + 12))) != 0;
}

/* ---- R_BoxInFrontOfPlane  0x004E1320 ----  VERIFIED */
BOOL __cdecl R_BoxInFrontOfPlane(int a1, int a2)
{
  return *(float *)(a2 - *(unsigned __int8 *)(a1 + 18) + 28) * *(float *)(a1 + 8)
       + *(float *)(a2 - *(unsigned __int8 *)(a1 + 17) + 20) * *(float *)(a1 + 4)
       + *(float *)(a2 - *(unsigned __int8 *)(a1 + 16) + 12) * *(float *)a1 > *(float *)(a1 + 12);
}

/* ---- R_CullBoxDPVS  0x004E1360 ----  VERIFIED */
int __cdecl R_CullBoxDPVS(int a1, int a2, int a3, int a4)
{
  int v4;
  int v5;
  double v6;
  double v7;
  double v8;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  double v18;

  if ( a4 < dpvs_cullPlaneLimit )
  {
    v4 = 0;
    dpvs_cullPlaneLimit = 0x7FFFFFFF;
    if ( a1 > 0 )
    {
      v5 = a3 + 8;
      do
      {
        v6 = *(float *)(*(unsigned __int8 *)(v5 + 8) + a2) * *(float *)(v5 - 8)
           + *(float *)(*(unsigned __int8 *)(v5 + 9) + a2) * *(float *)(v5 - 4)
           + *(float *)(*(unsigned __int8 *)(v5 + 10) + a2) * *(float *)v5;
        if ( (v6 < *(float *)(v5 + 4)) | __UNORDERED__(v6, *(float *)(v5 + 4)) )
          return 1;
        ++v4;
        v5 += 20;
      }
      while ( v4 < a1 );
    }
  }
  v7 = *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 18) + a2) * *(float *)(dpvs_activeNearPlane + 8)
     + *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 17) + a2) * *(float *)(dpvs_activeNearPlane + 4)
     + *(float *)(*(unsigned __int8 *)(dpvs_activeNearPlane + 16) + a2) * *(float *)dpvs_activeNearPlane;
  if ( (v7 < *(float *)(dpvs_activeNearPlane + 12)) | __UNORDERED__(v7, *(float *)(dpvs_activeNearPlane + 12)) )
    return 1;
  if ( dpvs_activeFarPlane )
  {
    v8 = *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 18) + a2) * *(float *)(dpvs_activeFarPlane + 8)
       + *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 17) + a2) * *(float *)(dpvs_activeFarPlane + 4)
       + *(float *)(*(unsigned __int8 *)(dpvs_activeFarPlane + 16) + a2) * *(float *)dpvs_activeFarPlane;
    if ( (v8 < *(float *)(dpvs_activeFarPlane + 12)) | __UNORDERED__(v8, *(float *)(dpvs_activeFarPlane + 12)) )
      return 1;
  }
  v10 = 0;
  if ( dpvs_occluderCount <= 0 )
    return 0;
  while ( 1 )
  {
    v11 = *(_DWORD *)(dpvs_occluders + 4 * v10);
    if ( a4 < *(_DWORD *)(v11 + 24) )
      break;
LABEL_16:
    if ( ++v10 >= dpvs_occluderCount )
      return 0;
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
      goto LABEL_16;
    ++v13;
    v14 += 20;
  }
  return 1;
}

/* ---- R_CullBoxDPVSStrict  0x004E14C0 ----  VERIFIED */
int __cdecl R_CullBoxDPVSStrict(int a1, int a2, int a3, int a4)
{
  int v4;
  int v5;
  int v6;
  int v7;
  _DWORD *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v15;

  v4 = a4;
  if ( a4 < dpvs_cullPlaneLimit )
  {
    v5 = 0;
    if ( a1 > 0 )
    {
      v6 = a2 + 8;
      while ( *(float *)(a3 - *(unsigned __int8 *)(v6 + 9) + 20) * *(float *)(v6 - 4)
            + *(float *)(a3 - *(unsigned __int8 *)(v6 + 8) + 12) * *(float *)(v6 - 8)
            + *(float *)(a3 - *(unsigned __int8 *)(v6 + 10) + 28) * *(float *)v6 > *(float *)(v6 + 4) )
      {
        ++v5;
        v6 += 20;
        if ( v5 >= a1 )
          goto LABEL_6;
      }
      return 1;
    }
LABEL_6:
    dpvs_cullPlaneLimit = a4 + 1;
  }
  v7 = 0;
  v15 = 0;
  if ( dpvs_occluderCount <= 0 )
    return 0;
  while ( 1 )
  {
    v8 = *(_DWORD **)(dpvs_occluders + 4 * v7);
    if ( v4 < v8[6] )
      break;
LABEL_14:
    v15 = ++v7;
    if ( v7 >= dpvs_occluderCount )
      return 0;
  }
  v9 = 0;
  v10 = 0;
  while ( v9 < v8[7] )
  {
    v11 = v8[8];
    v12 = *(unsigned __int8 *)(v11 + v10 + 18);
    v13 = v10 + v11;
    if ( *(float *)(a3 - v12 + 28) * *(float *)(v13 + 8)
       + *(float *)(a3 - *(unsigned __int8 *)(v13 + 17) + 20) * *(float *)(v13 + 4)
       + *(float *)(a3 - *(unsigned __int8 *)(v13 + 16) + 12) * *(float *)v13 > *(float *)(v13 + 12) )
    {
      v4 = a4;
      v8[6] = a4 + 1;
      v7 = v15;
      goto LABEL_14;
    }
    ++v9;
    v10 += 20;
  }
  return 1;
}

/* ---- R_SetPlaneSidesDPVS_dup  0x004E15D0 ----  VERIFIED */
int __cdecl R_SetPlaneSidesDPVS_dup(int result)
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

unsigned int dword_540568[51] =
{
	0x00000000, 0x3F800000, 0x3F800000, 0x3E800000,      /* 0x00540568  0      1      1      0.25  */
	0x00000000, 0x00000001, 0x00000000, 0x00000002,      /* 0x00540578  r_debugBoxEdges (cube edge pairs) */
	0x00000000, 0x00000004, 0x00000001, 0x00000003,      /* 0x00540588  r_debugBoxEdges (cube edge pairs) */
	0x00000001, 0x00000005, 0x00000002, 0x00000003,      /* 0x00540598  r_debugBoxEdges (cube edge pairs) */
	0x00000002, 0x00000006, 0x00000003, 0x00000007,      /* 0x005405A8  r_debugBoxEdges (cube edge pairs) */
	0x00000004, 0x00000005, 0x00000004, 0x00000006,      /* 0x005405B8  r_debugBoxEdges (cube edge pairs) */
	0x00000005, 0x00000007, 0x00000006, 0x00000007,      /* 0x005405C8  r_debugBoxEdges (cube edge pairs) */
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,      /* 0x005405D8  s_consoleColorTable (packed RGBA bytes) */
	0xFFFF0000, 0xFFFFFF00, 0xFFFF00FF, 0xFFFFFFFF,      /* 0x005405E8  s_consoleColorTable (packed RGBA bytes) */
	0x3E991687, 0x3F1645A2, 0x3DE978D5, 0x3F800000,      /* 0x005405F8  0.299  0.587  0.114  1     <- flt_5405F8 luminance */
	0x3F800000, 0x3F800000, 0x3F800000, 0x00000000,      /* 0x00540608  1      1      1      0 */
	0x00000000, 0x00000000, 0x3F800000, 0x00000000,      /* 0x00540618  0      0      1      0 */
	0x00000000, 0x00000000, 0x3F800000,                  /* 0x00540628  0      0      1 */
};
/* ---- dword_541830   0x00541830   retail .rdata, read with get_bytes ---- */
unsigned int dword_541830[9] =
{
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541830  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541840  0      1      0      1     */
	0x00000000,                                          /* 0x00541850  0     */
};
/* ---- dword_541840   0x00541840   retail .rdata, read with get_bytes ---- */
unsigned int dword_541840[5] =
{
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541840  0      1      0      1     */
	0x00000000,                                          /* 0x00541850  0     */
};
/* ---- dword_541860   0x00541860   retail .rdata, read with get_bytes ---- */
unsigned int dword_541860[92] =
{
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541860  0      0      1      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541870  1      1      0      1     */
	0x3F400000, 0x3F400000, 0x00000000, 0x3F800000,      /* 0x00541880  0.75   0.75   0      1     */
	0x3F000000, 0x3F000000, 0x00000000, 0x3F800000,      /* 0x00541890  0.5    0.5    0      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005418A0  1      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418B0  0      1      1      1     */
	0x00000000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x005418C0  0      0.75   0.75   1     */
	0x00000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x005418D0  0      0.5    0.5    1     */
	0x00000000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x005418E0  0      0.25   0.25   1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418F0  1      1      1      1     */
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- dword_541870   0x00541870   retail .rdata, read with get_bytes ---- */
unsigned int dword_541870[88] =
{
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541870  1      1      0      1     */
	0x3F400000, 0x3F400000, 0x00000000, 0x3F800000,      /* 0x00541880  0.75   0.75   0      1     */
	0x3F000000, 0x3F000000, 0x00000000, 0x3F800000,      /* 0x00541890  0.5    0.5    0      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005418A0  1      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418B0  0      1      1      1     */
	0x00000000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x005418C0  0      0.75   0.75   1     */
	0x00000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x005418D0  0      0.5    0.5    1     */
	0x00000000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x005418E0  0      0.25   0.25   1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418F0  1      1      1      1     */
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- cullGroupDebugColor   0x00541880   retail .rdata, read with get_bytes ---- */
unsigned int cullGroupDebugColor[84] =
{
	0x3F400000, 0x3F400000, 0x00000000, 0x3F800000,      /* 0x00541880  0.75   0.75   0      1     */
	0x3F000000, 0x3F000000, 0x00000000, 0x3F800000,      /* 0x00541890  0.5    0.5    0      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005418A0  1      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418B0  0      1      1      1     */
	0x00000000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x005418C0  0      0.75   0.75   1     */
	0x00000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x005418D0  0      0.5    0.5    1     */
	0x00000000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x005418E0  0      0.25   0.25   1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418F0  1      1      1      1     */
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- modelBoundsDebugColor   0x005418B0   retail .rdata, read with get_bytes ---- */
unsigned int modelBoundsDebugColor[72] =
{
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418B0  0      1      1      1     */
	0x00000000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x005418C0  0      0.75   0.75   1     */
	0x00000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x005418D0  0      0.5    0.5    1     */
	0x00000000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x005418E0  0      0.25   0.25   1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418F0  1      1      1      1     */
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- acceptedBevelDebugColor   0x005418C0   retail .rdata, read with get_bytes ---- */
unsigned int acceptedBevelDebugColor[68] =
{
	0x00000000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x005418C0  0      0.75   0.75   1     */
	0x00000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x005418D0  0      0.5    0.5    1     */
	0x00000000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x005418E0  0      0.25   0.25   1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418F0  1      1      1      1     */
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- rejectedBevelDebugColor   0x005418D0   retail .rdata, read with get_bytes ---- */
unsigned int rejectedBevelDebugColor[64] =
{
	0x00000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x005418D0  0      0.5    0.5    1     */
	0x00000000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x005418E0  0      0.25   0.25   1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418F0  1      1      1      1     */
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- dword_5418F0   0x005418F0   retail .rdata, read with get_bytes ---- */
unsigned int dword_5418F0[56] =
{
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005418F0  1      1      1      1     */
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- staticModelDebugColor   0x00541900   retail .rdata, read with get_bytes ---- */
unsigned int staticModelDebugColor[52] =
{
	0x3F400000, 0x3F400000, 0x3F400000, 0x3F800000,      /* 0x00541900  0.75   0.75   0.75   1     */
	0x3F000000, 0x3F000000, 0x3F000000, 0x3F800000,      /* 0x00541910  0.5    0.5    0.5    1     */
	0x3E800000, 0x3E800000, 0x3E800000, 0x3F800000,      /* 0x00541920  0.25   0.25   0.25   1     */
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};
/* ---- clippedLeafDebugColor   0x00541930   retail .rdata, read with get_bytes ---- */
unsigned int clippedLeafDebugColor[40] =
{
	0x3F800000, 0x3F333333, 0x00000000, 0x3F800000,      /* 0x00541930  1      0.7    0      1     */
	0x3F400000, 0x3F066666, 0x00000000, 0x3F800000,      /* 0x00541940  0.75   0.525  0      1     */
	0x00000000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541950  0      0      0      1     */
	0x3F800000, 0x00000000, 0x00000000, 0x3F800000,      /* 0x00541960  1      0      0      1     */
	0x00000000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541970  0      1      0      1     */
	0x3F800000, 0x3F800000, 0x00000000, 0x3F800000,      /* 0x00541980  1      1      0      1     */
	0x00000000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x00541990  0      0      1      1     */
	0x00000000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419A0  0      1      1      1     */
	0x3F800000, 0x00000000, 0x3F800000, 0x3F800000,      /* 0x005419B0  1      0      1      1     */
	0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000,      /* 0x005419C0  1      1      1      1     */
};

float flt_5405F8 = 0.299f;      /* 0x005405F8  0x3E991687 */
float flt_5405FC = 0.587f;      /* 0x005405FC  0x3F1645A2 */
float flt_540600 = 0.114f;      /* 0x00540600  0x3DE978D5 */
