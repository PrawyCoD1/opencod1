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
void __cdecl RB_BeginSurface( void *shader, int vertexComponentCount );
extern void __cdecl RB_CalcSunBlind( float *a1, float *a2, float a3 );
void __cdecl RB_EndSurface( void );
extern int RB_SelectStorageATI();
extern int RB_SelectStorageNV();
unsigned __int64 __cdecl RB_glColor4f( float r, float g, float b, float a );
char *__cdecl RB_glVertex3f( int xbits, int ybits, int zbits );
extern int R_FogOn();
extern int R_TransformClipToWindow();
extern int R_TransformHomogenousModelToClip();
 double __cdecl R_UpdateOverTime( float current, float target,
                                  int fadeInRate, int fadeOutRate,
                                  int elapsedMsec );
void VectorNormalizeFast( float *v );


#define FLARE_BYTES   0x2600

/* ---- RE_ClearFlares  0x004D8C50 ----  VERIFIED */
int *RE_ClearFlares()
{
  int *v0;
  int *result;

  memset(&unk_16DA420, 0, FLARE_BYTES);
  v0 = 0;
  dword_16DA40C = 0;
  flt_14072FC = 0.0;
  rendererSunState_currentBlindFraction = 0.0;
  rendererSunState_currentGlareFraction = 0.0;
  rendererSunState_lastUpdateTime = 0;
  result = (int *)&unk_16DA420;
  do
  {
    *result = (int)v0;
    v0 = result;
    result += 19;
  }
  while ( (int)result < (int)&unk_16DA420[FLARE_BYTES] );
  dword_16DCA20 = (int)v0;
  return result;
}

/* ---- RB_AddFlare  0x004D8CA0 ----  VERIFIED */
void __cdecl RB_AddFlare(int a1, float *a2)
{
  float *v2;
  float v4;
  float v5;
  double v6;
  double v7;
  int v8;
  double v9;
  int v10;
  double v11;
  double v12;
  float eye[4];  /* [ebp-30h] BYREF -- retail vec4_t */
  float src[4];  /* [ebp-20h] BYREF -- retail vec4_t */
  float clip[4]; /* [ebp-10h] BYREF -- retail vec4_t */
  float v22;  /* retail reuses the arg_0 slot as scratch */

  v2 = (float *)dword_16DA40C;
  ++backEnd_pc_flareAddCount;
  if ( dword_16DA40C )
  {
    do
    {
      if ( *((_DWORD *)v2 + 18) == *(_DWORD *)a1
        && *((_DWORD *)v2 + 3) == backEnd_viewParms_frameSceneNum
        && *((_DWORD *)v2 + 2) == backEnd_wireframeOverride )
      {
        break;
      }
      v2 = *(float **)v2;
    }
    while ( v2 );
  }
  v4 = *(float *)(a1 + 12);
  v5 = *(float *)(a1 + 16);
  src[0] = *(float *)(a1 + 8);
  src[3] = *(float *)(a1 + 20);
  src[1] = v4;
  src[2] = v5;
  if ( !((src[3] == 0.0) | __UNORDERED__(src[3], 0.0)) )
  {
    v6 = -src[3];
    eye[0] = backEnd_viewParms_originX[0] * v6 + src[0];
    eye[1] = backEnd_viewParms_originY * v6 + src[1];
    eye[2] = v6 * backEnd_viewParms_originZ + src[2];
    VectorNormalizeFast(eye);
    v7 = src[3] * -12.0;                 /* flt_569084 = 0xC1400000 */
    src[0] = eye[0] * v7 + src[0];
    src[1] = eye[1] * v7 + src[1];
    src[2] = v7 * eye[2] + src[2];
  }
  R_TransformHomogenousModelToClip(eye, flt_16D8DE4, backEnd_viewParms_projectionMatrix, clip, src);
  if ( v2 || *(_DWORD *)(a1 + 44) )
  {
LABEL_14:
    R_TransformClipToWindow(clip, (int)backEnd_viewParms_originX, eye, src);
    if ( !v2 )
    {
      v2 = (float *)dword_16DCA20;
      if ( !dword_16DCA20 )
        return;
      v9 = (double)*(int *)(a1 + 44);
      v22 = v9;
      if ( (v9 + src[0] < 0.0) | __UNORDERED__(v9 + src[0], 0.0)
        || (double)backEnd_viewParms_viewportWidth <= src[0] - v22
        || (v22 + src[1] < 0.0) | __UNORDERED__(v22 + src[1], 0.0)
        || (double)backEnd_viewParms_viewportHeight <= src[1] - v22 )
      {
        return;
      }
      v10 = *(_DWORD *)dword_16DCA20;
      *(_DWORD *)dword_16DCA20 = dword_16DA40C;
      dword_16DA40C = (int)v2;
      *((_DWORD *)v2 + 3) = backEnd_viewParms_frameSceneNum;
      dword_16DCA20 = v10;
      *((_DWORD *)v2 + 2) = backEnd_wireframeOverride;
      *((_DWORD *)v2 + 1) = -1;
      v2[18] = *(float *)a1;
      v2[9] = 0.0;
      *((_DWORD *)v2 + 8) = backEnd_refdef_time - 10;
    }
    v2[6] = *(float *)(a1 + 4);
    v2[7] = *(float *)(a1 + 56);
    v2[4] = *(float *)(a1 + 48);
    v2[5] = *(float *)(a1 + 52);
    *((_DWORD *)v2 + 1) = backEnd_viewParms_frameCount;
    v2[13] = *(float *)(a1 + 24);
    v2[14] = *(float *)(a1 + 28);
    v2[15] = *(float *)(a1 + 32);
    v2[16] = *(float *)(a1 + 36);
    v2[16] = *(float *)(a1 + 40);
    v2[17] = *(float *)(a1 + 44);
    if ( a2 )
    {
      eye[0] = backEnd_viewParms_originX[0] - *(float *)(a1 + 8);
      eye[1] = backEnd_viewParms_originY - *(float *)(a1 + 12);
      eye[2] = backEnd_viewParms_originZ - *(float *)(a1 + 16);
      VectorNormalizeFast(eye);
      v11 = eye[2] * a2[2] + eye[1] * a2[1] + eye[0] * *a2;
      v2[13] = v11 * v2[13];
      v2[14] = v11 * v2[14];
      v2[15] = v11 * v2[15];
    }
    v12 = src[1];
    *((_DWORD *)v2 + 10) = (int)src[0] + backEnd_viewParms_viewportX;
    *((_DWORD *)v2 + 11) = (int)v12 + backEnd_viewParms_viewportY;
    if ( (*(float *)(a1 + 20) == 0.0) | __UNORDERED__(*(float *)(a1 + 20), 0.0) )
      v2[12] = 1.0;
    else
      v2[12] = (clip[2] / clip[3] + 1.0) * 0.5;
    return;
  }
  v8 = 0;
  while ( clip[v8] < (double)clip[3] && -clip[3] < clip[v8] )
  {
    if ( ++v8 >= 3 )
      goto LABEL_14;
  }
}

/* ---- RB_AddDlightFlares  0x004D8FD0 ----  VERIFIED */
cvar_t *RB_AddDlightFlares()
{
  cvar_t *result;
  float value;
  double v2;
  double v3;
  int v4;
  _DWORD *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  float v12;
  float v13;
  _DWORD v14[15]; // [esp+20h] [ebp-3Ch] BYREF

  result = r_flares;
  if ( r_flares->integer >= 2 )
  {
    value = r_flareSize->value;
    v14[1] = tr_flareShader;
    v2 = r_flareFadeIn->value;
    v14[0] = 1024;
    v14[9] = 1065353216;
    *(float *)&v14[10] = value;
    v14[11] = 0;
    v12 = v2 * 1000.0;
    v3 = r_flareFadeOut->value * 1000.0;
    v14[12] = (int)(v12 + 9.313225746154785e-10);
    v13 = v3;
    result = (cvar_t *)backEnd_refdef_dlights;
    v14[13] = (int)(v13 + 9.313225746154785e-10);
    v4 = 0;
    v14[14] = 1;
    if ( backEnd_refdef_num_dlights > 0 )
    {
      v5 = (_DWORD *)(backEnd_refdef_dlights + 72);
      do
      {
        v6 = *(v5 - 1);
        v7 = v5[1];
        v14[3] = *v5;
        v8 = *(v5 - 17);
        v14[2] = v6;
        v9 = v5[2];
        v14[6] = v8;
        v14[4] = v7;
        v10 = *(v5 - 16);
        v14[5] = v9;
        v11 = *(v5 - 15);
        v14[7] = v10;
        v14[8] = v11;
        RB_AddFlare((int)v14, 0);
        ++v14[0];
        result = (cvar_t *)backEnd_refdef_num_dlights;
        ++v4;
        v5 += 34;
      }
      while ( v4 < backEnd_refdef_num_dlights );
    }
  }
  return result;
}

/* ---- RB_AddCoronaFlares  0x004D9100 ----  VERIFIED */
int RB_AddCoronaFlares()
{
  int result;
  double value;
  double v2;
  int v3;
  int v4;
  bool v5; // zf
  int v6;
  int v7;
  double v8;
  double v9;
  int v10;
  float v11;
  float v12;
  int v13; // [esp+20h] [ebp-3Ch] BYREF
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;
  int v22;
  float v23;
  int v24;
  int v25;
  int v26;
  int i;

  result = r_flares->integer;
  if ( result == 1 || result == 3 )
  {
    result = tr_world;
    if ( tr_world )
    {
      value = r_flareFadeIn->value;
      v18 = 1065353216;
      v22 = 1065353216;
      v24 = 0;
      v11 = value * 1000.0;
      v2 = r_flareFadeOut->value * 1000.0;
      v25 = (int)(v11 + 9.313225746154785e-10);
      v12 = v2;
      result = backEnd_refdef_coronaCount;
      v3 = backEnd_refdef_coronas;
      v4 = 0;
      v26 = (int)(v12 + 9.313225746154785e-10);
      for ( i = 1; v4 < backEnd_refdef_coronaCount; v3 += 48 )
      {
        v5 = (*(_BYTE *)(v3 + 44) & 2) == 0;
        v13 = *(_DWORD *)(v3 + 40);
        if ( v5 )
          v14 = tr_flareShader;
        else
          v14 = tr_spotLightShader;
        v6 = *(_DWORD *)(v3 + 4);
        v7 = *(_DWORD *)(v3 + 8);
        v15 = *(_DWORD *)v3;
        v19 = *(_DWORD *)(v3 + 12);
        v8 = r_flareSize->value;
        v16 = v6;
        v9 = v8 * *(float *)(v3 + 36);
        v20 = *(_DWORD *)(v3 + 16);
        v17 = v7;
        v10 = *(_DWORD *)(v3 + 20);
        v23 = v9;
        v21 = v10;
        RB_AddFlare((int)&v13, 0);
        result = backEnd_refdef_coronaCount;
        ++v4;
      }
    }
  }
  return result;
}

/* ---- RB_UpdateSunFlare  0x004D9250 ----  VERIFIED */
void __cdecl RB_UpdateSunFlare(int a1, int a2, int a3, float a4, int a5, int a6)
{
  _DWORD *v6;
  int v7;
  int v8;
  _DWORD v10[15]; // [esp+8h] [ebp-3Ch] BYREF
  float v11;

  v10[1] = a1;
  v6 = *(_DWORD **)(tr_world + 276);
  v7 = v6[18];
  v10[2] = v6[17];
  v8 = v6[19];
  v10[3] = v7;
  v10[4] = v8;
  v11 = (double)backEnd_viewParms_viewportWidth * a4 * 0.0015625;
  v10[6] = a2;
  v10[0] = -1;
  v10[5] = 0;
  v10[7] = a2;
  v10[8] = a2;
  v10[9] = 1065353216;
  v10[10] = a3;
  v10[11] = (int)(v11 + 9.313225746154785e-10);
  v10[12] = a5;
  v10[13] = a6;
  v10[14] = 1;
  RB_AddFlare((int)v10, 0);
}

/* ---- RB_TestFlare  0x004D9310 ----  VERIFIED */
void __cdecl RB_TestFlare(int a1)
{
  int v1;
  int v2;
  int v3;
  int v4;
  GLint v5;
  GLint v6;
  GLsizei v7;
  GLsizei v8;
  int i;
  int v10;
  int v12;
  float v13;
  int v14;
  float pixels[4225]; // [esp+10h] [ebp-4204h] BYREF

  ++*(int *)&backEnd_pc_flareTests;
  v1 = *(_DWORD *)(a1 + 68);
  v2 = *(_DWORD *)(a1 + 40);
  if ( v2 + v1 < 0 || v2 - v1 >= backEnd_viewParms_viewportWidth || (v3 = *(_DWORD *)(a1 + 44), v3 + v1 < 0) || v3 - v1 >= backEnd_viewParms_viewportHeight )
  {
    v13 = 1.0;
    if ( (*(_BYTE *)(a1 + 28) & 1) == 0 )
      v13 = 0.0;
  }
  else
  {
    glState_finishCalled = 0;
    v4 = 2 * *(_DWORD *)(a1 + 68) + 1;
    if ( v4 <= 65 )
    {
      if ( v4 < 1 )
        v4 = 1;
    }
    else
    {
      v4 = 65;
    }
    v5 = *(_DWORD *)(a1 + 40) - v4 / 2;
    v6 = *(_DWORD *)(a1 + 44) - v4 / 2;
    v7 = v4;
    v8 = v4;
    v14 = v4 * v4;
    if ( v5 >= 0 )
    {
      if ( v4 > backEnd_viewParms_viewportWidth - v5 )
        v7 = backEnd_viewParms_viewportWidth - v5;
    }
    else
    {
      v7 = v5 + v4;
      v5 = 0;
    }
    if ( v6 >= 0 )
    {
      if ( v4 > backEnd_viewParms_viewportHeight - v6 )
        v8 = backEnd_viewParms_viewportHeight - v6;
    }
    else
    {
      v8 = v6 + v4;
      v6 = 0;
    }
    v12 = 0;
    if ( v7 > 0 && v8 > 0 )
    {
      glReadPixels(v5, v6, v7, v8, 0x1902u, 0x1406u, pixels);
      for ( i = 0; i < v7 * v8; v12 += v10 )
        v10 = *(float *)(a1 + 48) <= (double)pixels[i++];
    }
    v13 = (double)v12 / (double)v14;
  }
  if ( *(_DWORD *)(a1 + 72) == -1 )
    flt_14072FC = v13;
  *(float *)(a1 + 36) = R_UpdateOverTime(
                          *(float *)(a1 + 36),
                          v13,
                          *(_DWORD *)(a1 + 16),
                          *(_DWORD *)(a1 + 20),
                          backEnd_refdef_time - *(_DWORD *)(a1 + 32));
  *(_DWORD *)(a1 + 32) = backEnd_refdef_time;
}

/* ---- RB_RenderFlare  0x004D94B0 ----  VERIFIED */
void __cdecl RB_RenderFlare(int a1)
{
  double v1;
  int v2;
  int v3;
  int v4;
  cvar_t *v5;
  float *v6;
  _DWORD *v7;
  _DWORD *v8;
  int *v9;
  float v10;
  int v11;
  float v12;
  float v13;

  ++HIDWORD(backEnd_pc_flareTests);
  v12 = tr_identityLight * *(float *)(a1 + 56);
  v13 = tr_identityLight * *(float *)(a1 + 60);
  LOBYTE(v11) = (unsigned __int64)(tr_identityLight * *(float *)(a1 + 52) * 255.0);
  BYTE1(v11) = (unsigned __int64)(v12 * 255.0);
  BYTE2(v11) = (unsigned __int64)(v13 * 255.0);
  v1 = *(float *)(a1 + 64) * 0.0015625 * (double)backEnd_viewParms_viewportWidth;
  BYTE3(v11) = (unsigned __int64)(*(float *)(a1 + 36) * 255.0);
  backEnd_currentEntity = (int)&tr_worldEntity;
  v2 = storageClass;
  v10 = v1;
  if ( storageClass != glState_currentStorageMode )
  {
    if ( glConfig_NVVertexArrayRange )
    {
      RB_SelectStorageNV(storageClass);
    }
    else if ( glConfig_ATIVertexArrayObject )
    {
      RB_SelectStorageATI(storageClass);
    }
    glState_currentStorageMode = v2;
  }
  RB_BeginSurface(*(void **)(a1 + 24), 3);   /* 0x004D9588: mov ecx,[edi+18h] / push 3 */
  v3 = tess_numVertexes;
  v4 = tess_numVertexes * tess_vertexComponentCount;
  *(float *)&tess_xyz[v4] = (double)*(int *)(a1 + 40) - v10;
  v5 = r_znear;
  v6 = (float *)(4 * v4 + ((int)(char *)tess_xyz + 8));
  v7 = (_DWORD *)(8 * v3 + ((int)(char *)tess_texCoords0));
  *(v6 - 1) = (double)*(int *)(a1 + 44) - v10;
  *v6 = v5->value;
  *v7 = 0;
  v7[1] = 0;
  dword_181FF60[v3] = v11;
  v6 += 3;
  *(v6 - 2) = (double)*(int *)(a1 + 40) - v10;
  v6 += 3;
  v8 = (_DWORD *)(8 * v3 + ((int)(char *)tess_texCoords0 + 20));
  v9 = (int *)(4 * v3 + ((int)(char *)tess_vertexColors + 8));
  *(v6 - 4) = (double)*(int *)(a1 + 44) + v10;
  *(v6 - 3) = v5->value;
  *(v8 - 3) = 0;
  *(v8 - 2) = 1065353216;
  *(v9 - 1) = v11;
  v6 += 2;
  ++v8;
  *(v6 - 4) = (double)*(int *)(a1 + 40) + v10;
  *(v6 - 3) = (double)*(int *)(a1 + 44) + v10;
  *(v6 - 2) = v5->value;
  *(v8 - 2) = 1065353216;
  *(v8 - 1) = 1065353216;
  *v9 = v11;
  *(v6 - 1) = (double)*(int *)(a1 + 40) + v10;
  *v6 = (double)*(int *)(a1 + 44) - v10;
  v6[1] = v5->value;
  *v8 = 1065353216;
  v8[1] = 0;
  v9[1] = v11;
  tess_indexes[tess_numIndexes++] = tess_numVertexes;
  tess_indexes[tess_numIndexes++] = tess_numVertexes + 1;
  tess_indexes[tess_numIndexes++] = tess_numVertexes + 2;
  tess_indexes[tess_numIndexes++] = tess_numVertexes;
  tess_indexes[tess_numIndexes++] = tess_numVertexes + 2;
  tess_indexes[tess_numIndexes++] = tess_numVertexes + 3;
  tess_numVertexes += 4;
  RB_EndSurface();
}

/* ---- RB_RenderFlares  0x004D9770 ----  VERIFIED */
void RB_RenderFlares()
{
  int v0;
  int v1;  /* "some flare in this view is visible" */
  int *v2;  /* trailing pointer for the unlink */
  int v3;
  int i;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  float v14; // [esp+60h] [ebp-Ch] BYREF -- RB_CalcSunBlind out, via ESI
  float v15; // [esp+64h] [ebp-8h] BYREF -- RB_CalcSunBlind out, via EDI

  if ( r_flares->integer )
  {
    RB_AddDlightFlares();
    RB_AddCoronaFlares();
    v0 = dword_16DA40C;
    v1 = 0;
    flt_14072FC = 0.0;
    v2 = &dword_16DA40C;
    if ( dword_16DA40C )
    {
      do
      {
        if ( *(_DWORD *)(v0 + 4) < backEnd_viewParms_frameCount )
        {
          if ( *(float *)(v0 + 36) <= 0.0 )
          {
            v3 = dword_16DCA20;
            *v2 = *(_DWORD *)v0;
            *(_DWORD *)v0 = v3;
            dword_16DCA20 = v0;
            goto LABEL_12;
          }
          *(_DWORD *)(v0 + 28) = 0;
          *(_DWORD *)(v0 + 48) = 0x40000000;
        }
        if ( *(_DWORD *)(v0 + 12) == backEnd_viewParms_frameSceneNum && *(_DWORD *)(v0 + 8) == backEnd_wireframeOverride )
        {
          RB_TestFlare(v0);
          if ( (*(float *)(v0 + 36) == 0.0) | __UNORDERED__(*(float *)(v0 + 36), 0.0) )
          {
            v5 = dword_16DCA20;
            *v2 = *(_DWORD *)v0;
            *(_DWORD *)v0 = v5;
            dword_16DCA20 = v0;
            goto LABEL_12;
          }
          v1 = 1;
        }
        v2 = (int *)v0;
LABEL_12:
        v0 = *v2;
      }
      while ( *v2 );
    }
    RB_CalcSunBlind(&v15, &v14, flt_14072FC);
    if ( v1 || v14 > 0.0 || v15 > 0.0 )
    {
      if ( backEnd_wireframeOverride )
        glDisable(0x3000u);
      qglPushMatrix();
      qglLoadIdentity();
      glMatrixMode(0x1701u);
      qglPushMatrix();
      qglLoadIdentity();
      qglOrtho(
        (double)backEnd_viewParms_viewportX,
        (double)(backEnd_viewParms_viewportX + backEnd_viewParms_viewportWidth),
        (double)backEnd_viewParms_viewportY,
        (double)(backEnd_viewParms_viewportY + backEnd_viewParms_viewportHeight),
        -99999.0,
        99999.0);
      if ( !((v14 == 0.0) | __UNORDERED__(v14, 0.0)) || !((v15 == 0.0) | __UNORDERED__(v15, 0.0)) )
      {
        RB_BeginImmediateMode();
        GL_Bind((GLenum *)tr_whiteImage);
        GL_State(65634);                       /* 0x10062 */
        if ( glState_texEnv[glState_currentTmu] != 8448 )
        {
          glState_texEnv[glState_currentTmu] = 8448;
          glTexEnvi(0x2300u, 0x2200u, 8448);
        }
        RB_glColor4f(v15, v15, v15, v14);
        *(float *)&v10 = (float)backEnd_viewParms_viewportY;
        mode = 7;
        *(float *)&v6 = (float)backEnd_viewParms_viewportX;
        RB_glVertex3f(v6, v10, 0);
        *(float *)&v11 = (float)(backEnd_viewParms_viewportY + backEnd_viewParms_viewportHeight);
        *(float *)&v7 = (float)backEnd_viewParms_viewportX;
        RB_glVertex3f(v7, v11, 0);
        *(float *)&v12 = (float)(backEnd_viewParms_viewportY + backEnd_viewParms_viewportHeight);
        *(float *)&v8 = (float)(backEnd_viewParms_viewportX + backEnd_viewParms_viewportWidth);
        RB_glVertex3f(v8, v12, 0);
        *(float *)&v13 = (float)backEnd_viewParms_viewportY;
        *(float *)&v9 = (float)(backEnd_viewParms_viewportX + backEnd_viewParms_viewportWidth);
        RB_glVertex3f(v9, v13, 0);
        glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
        rbDebug_immediateVertexCount = 0;
        mode = 0;
        rbDebug_immediateModeActive = 0;
        R_FogOn();
        glVertexPointer(3, 0x1406u, 0, tess_xyz);
      }
      if ( v1 )
      {
        for ( i = dword_16DA40C; i; i = *(_DWORD *)i )
        {
          if ( *(_DWORD *)(i + 12) == backEnd_viewParms_frameSceneNum
            && *(_DWORD *)(i + 8) == backEnd_wireframeOverride
            && !((*(float *)(i + 36) == 0.0) | __UNORDERED__(*(float *)(i + 36), 0.0)) )
          {
            RB_RenderFlare(i);
          }
        }
      }
      qglPopMatrix();
      glMatrixMode(0x1700u);
      qglPopMatrix();
    }
  }
}
