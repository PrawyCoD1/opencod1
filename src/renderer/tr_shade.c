/*
 * @fidelity: likely
 * @fidelity-default: reviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_gl_types.h"
#include "tr_tess.h"

extern int GL_Bind();
extern int __cdecl GL_CheckErrors( const char *location );
extern int GL_ClientState();
extern int GL_Cull();
extern int GL_State();
extern int GL_TexEnv();
extern int GlobalPositionToLocal();
extern int RB_BeginImmediateMode();
void __cdecl RB_CalcAlphaFromEntity( unsigned char *colors );
void __cdecl RB_CalcAlphaFromOneMinusEntity( unsigned char *colors );
void __fastcall RB_CalcColorFromEntity( unsigned char *colors );
int __fastcall RB_CalcColorFromOneMinusEntity( void *colors );
extern int RB_CalcCubeMapBumpmapFrameTexCoords();
extern int RB_CalcCubeMapDot3ReflectTexCoords();
int __fastcall RB_CalcCubeMapEyeToVertexTexCoords( int a1 );
extern int RB_CalcCubeMapLightHalfAngleTexCoords();
int __fastcall RB_CalcCubeMapLightVectorTexCoords( int a1 );
float * __fastcall RB_CalcCubeMapReflectionTexCoords( int a1, int a2 );
extern int RB_CalcCubeMapSunHalfAngleTexCoords();
extern int RB_CalcCubeMapTbnTexCoords();
int __fastcall RB_CalcCubeMapVertexToEyeTexCoords( int a1 );
int __cdecl RB_CalcDiffuseColor( void *colors );
extern int RB_CalcEnvironmentTexCoords();
extern float *__cdecl RB_CalcRotateTexCoords( float a1, float *a2 );
void __cdecl RB_CalcScaleTexCoords( float *st, const float *scale );
void __cdecl RB_CalcScrollTexCoords( const float *scrollSpeed, float *st );
int __cdecl RB_CalcSpecularAlpha( int colors );
extern int RB_CalcStretchTexCoords();
void __cdecl RB_CalcSwapTexCoords( float *st );
extern int RB_CalcTangentSpace();
extern int RB_CalcTransformTexCoords();
void __cdecl RB_CalcTurbulentTexCoords( float *st, const float *wf );
int __cdecl RB_CalcWaveAlpha( void *wave, void *colors );
int __cdecl RB_CalcWaveColor( void *colors, void *wave );
int __cdecl RB_DeformTessGeometry( void );
extern int RB_SelectStorageNV();
extern int RB_ShadowTessEnd();
extern int RB_StageIteratorSky();
extern int RB_UploadWaterTexture();
extern int RB_glVertex3f();
extern int R_Fog();
void VectorNormalizeFast( float *v );
void VectorRotate( float *in, float *matrix, float *out );

void __cdecl RB_DisableTMU( int tmu );
void __cdecl RB_BeginSurface( void *shader, int vertexComponentCount );
void __cdecl RB_EndSurface( void );

/* ---- RB_ChooseSurfaceCountColor  0x004FDC10 ----  VERIFIED */
void __cdecl RB_ChooseSurfaceCountColor(int a1, _BYTE *a2)
{
  unsigned int v3;
  double v4;
  double v5;
  float v6;
  float v7;
  float v8;
  float v9[3]; // [ebp-0xc] BYREF  -- retail vec3_t

  if ( r_showtris->integer <= 6 )
  {
    if ( a1 > 30 )
    {
      if ( a1 > 150 )
      {
        if ( a1 > 450 )
        {
          if ( a1 > 1200 )
          {
            if ( a1 <= 3000 )
            {
              *a2 = tr_identityLightByte >> 2;
              a2[1] = tr_identityLightByte >> 2;
              a2[2] = tr_identityLightByte;
            }
            else
            {
              *a2 = 0;
              a2[1] = tr_identityLightByte;
              a2[2] = 0;
            }
            a2[3] = -1;
          }
          else
          {
            *a2 = tr_identityLightByte;
            a2[1] = tr_identityLightByte;
            a2[2] = 0;
            a2[3] = -1;
          }
        }
        else
        {
          *a2 = tr_identityLightByte;
          a2[1] = 0;
          a2[2] = 0;
          a2[3] = -1;
        }
      }
      else
      {
        *a2 = tr_identityLightByte;
        a2[1] = tr_identityLightByte >> 1;
        a2[2] = tr_identityLightByte >> 1;
        a2[3] = -1;
      }
    }
    else
    {
      *a2 = tr_identityLightByte;
      a2[1] = tr_identityLightByte;
      a2[2] = tr_identityLightByte;
      a2[3] = -1;
    }
  }
  else
  {
    v3 = rand();
    srand(31337 * (a1 + *(_DWORD *)(tess_shader + 72)));
    v9[0] = (double)rand() * 0.000030517578;
    v9[1] = (double)rand() * 0.000030517578;
    v9[2] = (double)rand() * 0.000030517578;
    VectorNormalize(v9);
    v6 = v9[0] * 255.0;
    v4 = v9[1] * 255.0;
    *a2 = (int)(v6 + 9.313225746154785e-10);
    v7 = v4;
    v5 = v9[2] * 255.0;
    a2[1] = (int)(v7 + 9.313225746154785e-10);
    v8 = v5;
    a2[2] = (int)(v8 + 9.313225746154785e-10);
    srand(v3);
    a2[3] = -1;
  }
}

/* ---- RB_SetSurfaceCountColor  0x004FDE30 ----  VERIFIED */
void __cdecl RB_SetSurfaceCountColor(int a1)
{
  GLubyte v[4]; // [esp+8h] [ebp-4h] BYREF

  RB_ChooseSurfaceCountColor(a1, v);
  glColor4ubv(v);
}

/* ---- RB_MakeNormalVectors  0x004FDE50 ----  VERIFIED */
int __cdecl RB_MakeNormalVectors(float *a1, float *a2, float *a3)
{
  int result;
  double v4;
  double v5;
  double v6;
  int v7;
  double v8;
  double v9;
  double v10;
  float v11;
  float v12;
  float v13;

  if ( *a2 <= -1.0 )
  {
    v13 = 1.0 / (a2[2] + 1.0);
    v8 = *a2 * *a2 * v13;
    v9 = a2[1] * *a2 * v13;
    v10 = a2[1] * a2[1] * v13;
    *a3 = -v9;
    a3[1] = 1.0 - v10;
    a3[2] = -a2[1];
    *a1 = v8 - 1.0;
    a1[1] = v9;
    result = *(int *)a2;
    a1[2] = *a2;
  }
  else
  {
    result = *((int *)a2 + 1);
    v11 = 1.0 / (*a2 + 1.0);
    v4 = *(float *)&result * *(float *)&result * v11;
    v5 = a2[2] * *(float *)&result * v11;
    v6 = a2[2];
    *a3 = *(float *)&result;
    v12 = v6 * v6 * v11;
    a3[1] = v4 - 1.0;
    a3[2] = v5;
    v7 = *((_DWORD *)a2 + 2);
    a1[1] = v5;
    *(_DWORD *)a1 = v7;
    a1[2] = v12 - 1.0;
  }
  return result;
}

/* ---- R_DrawStripElements  0x004FDF30 ----  VERIFIED */
int __cdecl R_DrawStripElements(
        unsigned __int16 *a1,
        void (__stdcall *a2)(int),
        signed int a4)
{
  int result;
  int v6;
  int v7;
  int v8;
  unsigned __int16 *v9;
  int v10;
  int v11;
  int v13;

  result = a4;
  ++c_begins;
  if ( a4 > 0 )
  {
    qglBegin(5);
    a2(*a1);
    a2(a1[1]);
    a2(a1[2]);
    v6 = *a1;
    v13 = a1[1];
    v7 = 0;
    c_vertexes += 3;
    v8 = a1[2];
    if ( a4 > 3 )
    {
      v9 = a1 + 5;
      a4 = (a4 - 4) / 3u + 1;
      while ( !v7 )
      {
        if ( *(v9 - 2) != v8 || *(v9 - 1) != v13 )
          goto LABEL_11;
        a2(*v9);
        v10 = c_vertexes + 1;
        v7 = 1;
LABEL_13:
        v6 = *(v9 - 2);
        v8 = *v9;
        c_vertexes = v10;
        v13 = *(v9 - 1);
        v9 += 3;
        if ( !--a4 )
          qglEnd();
          return result;
      }
      if ( v8 == *(v9 - 1) && v6 == *(v9 - 2) )
      {
        a2(*v9);
        v10 = c_vertexes + 1;
      }
      else
      {
LABEL_11:
        qglEnd();
        qglBegin(5);
        v11 = *(v9 - 2);
        ++c_begins;
        a2(v11);
        a2(*(v9 - 1));
        a2(*v9);
        v10 = c_vertexes + 3;
      }
      v7 = 0;
      goto LABEL_13;
    }
    qglEnd();
    return result;
  }
  return result;
}

/* ---- R_DrawElements  0x004FE070 ----  VERIFIED */
void __cdecl R_DrawElements(GLvoid *indexes, GLsizei numIndexes)
{
  int integer;

  integer = r_primitives->integer;
  if ( integer )
  {
    if ( integer != 2 )
    {
      if ( integer != 1 )
        return;
      goto LABEL_7;
    }
LABEL_5:
    backEnd_pc_drawnIndexCount += numIndexes;
    ++backEnd_pc_drawCallCount;
    glDrawElements(4u, numIndexes, 0x1403u, indexes);
    return;
  }
  if ( qglLockArraysEXT )
    goto LABEL_5;
LABEL_7:
  R_DrawStripElements((unsigned __int16 *)indexes, (void (__stdcall *)(int))qglArrayElement, numIndexes);
}

/* ---- RB_GetAnimatedImage  0x004FE0E0 ----  VERIFIED */
int __cdecl RB_GetAnimatedImage(int a1, int a2, int a3)
{
  int v4;
  int v5;
  float v6;

  if ( *(_BYTE *)(a2 + 197) )
  {
    ri_CIN_RunCinematic(*(_DWORD *)(a2 + 192), a1);
    ri_CIN_UploadCinematic(*(_DWORD *)(a2 + 192));
    return tr_scratchImages[*(_DWORD *)(a2 + 192)];
  }
  if ( *(_DWORD *)(a2 + 136) )
  {
    RB_UploadWaterTexture(*(_DWORD *)(a2 + 136), tr_refdef_time, a3);
    return *(_DWORD *)a2;
  }
  if ( *(int *)(a2 + 128) <= 1 )
    return *(_DWORD *)a2;
  v6 = tess_shaderTime * *(float *)(a2 + 132) * 1024.0;
  ftol_tempSpill = (int)v6;
  v4 = (int)v6 >> 10;
  if ( v4 < 0 )
    v4 = 0;
  if ( *(_BYTE *)(a2 + 198) )
  {
    v5 = *(_DWORD *)(a2 + 128);
    if ( v4 >= v5 )
      return *(_DWORD *)(a2 + 4 * (v5 - 1));
  }
  else
  {
    v4 %= *(_DWORD *)(a2 + 128);
  }
  return *(_DWORD *)(a2 + 4 * v4);
}

/* ---- RB_BindAnimatedImage  0x004FE1A0 ----  VERIFIED */
void __cdecl RB_BindAnimatedImage(void *this)
{
  GLenum *AnimatedImage;

  AnimatedImage = (GLenum *)RB_GetAnimatedImage((int)this, (int)this, glState_currentTmu);

  GL_Bind(AnimatedImage);
}

/* ---- RB_EnableTMU  0x004FE1C0 ----  VERIFIED */
_DWORD *__cdecl RB_EnableTMU(int a1, cvar_t *a2, int a3)
{
  _DWORD *AnimatedImage;
  GLenum v5;
  GLenum v6;
  _DWORD *result;
  int v8;
  GLuint v9;
  int param;

  param = *(_DWORD *)(a1 + 144);
  if ( *(_BYTE *)(a1 + 196) )
  {
    a2 = r_lightmap;
    if ( r_lightmap->integer )
      param = 7681;
  }
  AnimatedImage = (_DWORD *)RB_GetAnimatedImage((int)a2, a1, a3);
  v5 = AnimatedImage[21];
  if ( cap[a3] != v5 || AnimatedImage[22] != glState_currentTextures[a3] || param != glState_texEnv[a3] )
  {
    if ( a3 != glState_currentTmu )
    {
      qglActiveTextureARB(a3 + 33984);
      glState_currentTmu = a3;
    }
    v6 = cap[a3];
    if ( v6 != v5 )
    {
      if ( v6 )
        glDisable(cap[a3]);
      glEnable(v5);
      cap[a3] = v5;
    }
    if ( AnimatedImage[22] != glState_currentTextures[a3] )
    {
      v9 = AnimatedImage[22];
      AnimatedImage[23] = tr_frameCount;
      glBindTexture(v5, v9);
      glState_currentTextures[a3] = AnimatedImage[22];
    }
    if ( param != glState_texEnv[a3] )
    {
      glTexEnvi(0x2300u, 0x2200u, param);
      glState_texEnv[a3] = param;
    }
  }
  if ( param == 34160 )
  {
    if ( a3 != glState_currentTmu )
    {
      qglActiveTextureARB(a3 + 33984);
      glState_currentTmu = a3;
    }
    glTexEnvfv(0x2300u, 0x2201u, *(const GLfloat **)(a1 + 148));
    glTexEnvi(0x2300u, 0x8571u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 16));
    glTexEnvf(0x2300u, 0x8573u, *(GLfloat *)(*(_DWORD *)(a1 + 148) + 44));
    glTexEnvi(0x2300u, 0x8580u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 20));
    glTexEnvi(0x2300u, 0x8581u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 24));
    glTexEnvi(0x2300u, 0x8582u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 28));
    glTexEnvi(0x2300u, 0x8590u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 32));
    glTexEnvi(0x2300u, 0x8591u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 36));
    glTexEnvi(0x2300u, 0x8592u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 40));
    glTexEnvi(0x2300u, 0x8572u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 48));
    glTexEnvf(0x2300u, 0xD1Cu, *(GLfloat *)(*(_DWORD *)(a1 + 148) + 76));
    glTexEnvi(0x2300u, 0x8588u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 52));
    glTexEnvi(0x2300u, 0x8589u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 56));
    glTexEnvi(0x2300u, 0x858Au, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 60));
    glTexEnvi(0x2300u, 0x8598u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 64));
    glTexEnvi(0x2300u, 0x8599u, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 68));
    glTexEnvi(0x2300u, 0x859Au, *(_DWORD *)(*(_DWORD *)(a1 + 148) + 72));
  }
  result = *(_DWORD **)(a1 + 152);
  if ( result )
  {
    if ( *result || (result = (_DWORD *)glState_textureShaderEnabled[a3]) != 0 )
    {
      if ( a3 != glState_currentTmu )
      {
        qglActiveTextureARB(a3 + 33984);
        glState_currentTmu = a3;
      }
      glTexEnvi(0x86DEu, 0x86DFu, **(_DWORD **)(a1 + 152));
      v8 = *(_DWORD *)(a1 + 152);
      switch ( *(_DWORD *)v8 )
      {
        case 0x86E7:
          glTexEnvfv(0x86DEu, 0x86E0u, (const GLfloat *)(v8 + 12));
          break;
        case 0x86E9:
        case 0x86EA:
          glTexEnvi(0x86DEu, 0x86E4u, *(_DWORD *)(v8 + 4));
          break;
        case 0x86EC:
        case 0x86EE:
        case 0x86F0:
        case 0x86F1:
        case 0x86F2:
          goto LABEL_30;
        case 0x86F3:
          glTexEnvfv(0x86DEu, 0x86E5u, (const GLfloat *)(v8 + 12));
LABEL_30:
          glTexEnvi(0x86DEu, 0x86E4u, *(_DWORD *)(*(_DWORD *)(a1 + 152) + 4));
          glTexEnvi(0x86DEu, 0x86D9u, *(_DWORD *)(*(_DWORD *)(a1 + 152) + 8));
          break;
        default:
          break;
      }
      result = **(_DWORD ***)(a1 + 152);
      glState_textureShaderEnabled[a3] = result != 0;
    }
  }
  return result;
}

/* ---- RB_DisableTMU  0x004FE5C0 ----  VERIFIED */
void __cdecl RB_DisableTMU( int tmu )
{
  if ( !cap[tmu] )
    return;

  if ( tmu != glState_currentTmu )
  {
    qglActiveTextureARB( tmu + 0x84C0 );
    glState_currentTmu = tmu;
  }

  glDisable( cap[tmu] );
  cap[tmu] = 0;

  if ( glState_textureShaderEnabled[tmu] )
  {
    glTexEnvi( 0x86DE, 0x86DF, 0 );
    glState_textureShaderEnabled[tmu] = 0;
  }
}

/* ---- RB_SetupTmu  0x004FE630 ----  VERIFIED */
void __cdecl RB_SetupTmu(int a1, int a2)
{
  int v3;
  cvar_t *v4;

  v3 = a2 + 200 * a1;
  v4 = *(cvar_t **)(v3 + 148);
  if ( v4 )
    RB_EnableTMU(v3 + 4, v4, a1);
  else
    RB_DisableTMU(a1);
}

/* ---- RB_EnableClientTmu  0x004FE660 ----  VERIFIED */
void __cdecl RB_EnableClientTmu(int a1, int a2, GLvoid *pointer, GLsizei stride)
{
  char v4;

  v4 = glState_currentClientTmu;
  if ( glState_currentClientTmu != a1 )
  {
    qglClientActiveTextureARB(a1 + 33984);
    v4 = a1;
    glState_currentClientTmu = a1;
  }
  if ( (glState_clientStateBits & (1 << v4)) == 0 )
  {
    glEnableClientState(0x8078u);
    glState_clientStateBits |= 1 << glState_currentClientTmu;
  }
  glTexCoordPointer(*(_DWORD *)(a2 + 140), 0x1406u, stride, pointer);
}

/* ---- RB_DisableClientTmu  0x004FE6D0 ----  VERIFIED */
int __cdecl RB_DisableClientTmu(int a1)
{
  int result;

  result = glState_clientStateBits;
  if ( (glState_clientStateBits & (1 << a1)) != 0 )
  {
    if ( glState_currentClientTmu != a1 )
    {
      qglClientActiveTextureARB(a1 + 33984);
      glState_currentClientTmu = a1;
    }
    glDisableClientState(0x8078u);
    result = ~(1 << a1) & glState_clientStateBits;
    glState_clientStateBits = result;
  }
  return result;
}

/* ---- RB_SetupClientTmu  0x004FE720 ----  VERIFIED */
void __cdecl RB_SetupClientTmu(int a1, GLsizei a2, int a3, int a4)
{
  int v5;

  v5 = a3 + 200 * a1;
  if ( *(_DWORD *)(v5 + 148) )
    RB_EnableClientTmu(a1, v5 + 4, *(GLvoid **)(a4 + 4 * a1), a2);
  else
    RB_DisableClientTmu(a1);
}

/* ---- RB_EnableClientTmuATI  0x004FE760 ----  VERIFIED */
int __cdecl RB_EnableClientTmuATI(int a1, int a2, int a3, int a4, int a5)
{
  char v5;

  v5 = glState_currentClientTmu;
  if ( glState_currentClientTmu != a1 )
  {
    qglClientActiveTextureARB(a1 + 33984);
    v5 = a1;
    glState_currentClientTmu = a1;
  }
  if ( (glState_clientStateBits & (1 << v5)) == 0 )
  {
    glEnableClientState(0x8078u);
    glState_clientStateBits |= 1 << glState_currentClientTmu;
  }
  qglArrayObjectATI(32888, *(_DWORD *)(a2 + 140), 5126, a5, a3, a4);
  return 0;
}

/* ---- RB_SetupClientTmuATI  0x004FE7E0 ----  VERIFIED */
int __cdecl RB_SetupClientTmuATI(int a1, int a2, int a3, int a4, int a5)
{
  int v6;

  v6 = a3 + 200 * a1;
  if ( *(_DWORD *)(v6 + 148) )
    return RB_EnableClientTmuATI(a1, v6 + 4, a4, *(_DWORD *)(a5 + 4 * a1), a2);
  else
    return RB_DisableClientTmu(a1);
}

/* ---- RB_SetupRegisterCombiners  0x004FE820 ----  VERIFIED */
int __cdecl RB_SetupRegisterCombiners(int a1)
{
  const char *v1;
  int v2;
  int v3;
  _DWORD *v4;
  _DWORD *v5;
  bool v6; // zf
  int v7;
  int v8;
  _DWORD *v9;
  int v11;
  int v12;
  int v13;

  v1 = (const char *)a1;
  glCombinerParameterfvNV(34090, a1);
  glCombinerParameterfvNV(34091, a1 + 16);
  glCombinerParameteriNV(34126, *(_DWORD *)(a1 + 1528));
  if ( glConfig_NVRegisterCombiners >= 2 )
  {
    if ( *(_DWORD *)(a1 + 1532) )
      glEnable(0x8535u);
    else
      glDisable(0x8535u);
  }
  if ( *(int *)(a1 + 1528) > 0 )
  {
    v12 = 34128;
    v2 = a1 + 136;
    do
    {
      v11 = 34083;
      v4 = (_DWORD *)v2;
      v5 = (_DWORD *)(v2 - 72 + 4);
      v13 = 4;
      do
      {
        glCombinerInputNV(v12, 6407, v11, *(v5 - 1), *v5, v5[1]);
        glCombinerInputNV(v12, 6406, v11, *v4, v5[18], v4[2]);
        v5 += 3;
        v4 += 3;
        v6 = v13 == 1;
        ++v11;
        --v13;
      }
      while ( !v6 );
      v7 = v12;
      v3 = v2 - 72;
      glCombinerOutputNV(
        v12,
        6407,
        *(_DWORD *)(v3 + 48),
        *(_DWORD *)(v3 + 52),
        *(_DWORD *)(v3 + 56),
        *(_DWORD *)(v3 + 60),
        *(_DWORD *)(v3 + 64),
        *(unsigned __int8 *)(v3 + 68),
        *(unsigned __int8 *)(v3 + 69),
        *(unsigned __int8 *)(v3 + 70));
      glCombinerOutputNV(
        v12,
        6406,
        *(_DWORD *)(v2 + 48),
        *(_DWORD *)(v2 + 52),
        *(_DWORD *)(v2 + 56),
        *(_DWORD *)(v2 + 60),
        *(_DWORD *)(v2 + 64),
        *(unsigned __int8 *)(v2 + 68),
        *(unsigned __int8 *)(v2 + 69),
        *(unsigned __int8 *)(v2 + 70));
      if ( *(_DWORD *)(a1 + 1532) )
      {
        glCombinerStageParameterfvNV(v12, 34090, v2 - 104);
        glCombinerStageParameterfvNV(v12, 34091, v2 - 88);
      }
      v2 += 176;
      ++v12;
    }
    while ( v7 - 34127 < *(_DWORD *)(a1 + 1528) );
    v1 = (const char *)a1;
  }
  v8 = 0;
  v9 = v1 + 1444;
  do
  {
    glFinalCombinerInputNV(v8 + 34083, *(v9 - 1), *v9, v9[1]);
    ++v8;
    v9 += 3;
  }
  while ( v8 < 7 );
  return GL_CheckErrors("RB_SetupRegisterCombiners");
}

/* ---- RB_SetupVertexProgram  0x004FEA20 ----  VERIFIED */
int __cdecl RB_SetupVertexProgram(int a1)
{
  int result;
  float v2[4];

  if ( a1 != glState_boundVertexProgram )
  {
    glBindProgramARB(34336, *(_DWORD *)(a1 + 64));
    glState_boundVertexProgram = a1;
  }
  GlobalPositionToLocal(v2, backEnd_viewParms_originX);
  v2[3] = 1.0;
  glProgramLocalParameter4fvARB(34336, 0, v2);
  result = backEnd_currentDlight;
  if ( backEnd_currentDlight )
  {
    GlobalPositionToLocal(v2, (float *)(backEnd_currentDlight + 68));
    v2[3] = 1.0;
    glProgramLocalParameter4fvARB(34336, 1, v2);
    v2[0] = backEnd_currentLightScale * *(float *)(backEnd_currentDlight + 36);
    v2[1] = backEnd_currentLightScale * *(float *)(backEnd_currentDlight + 40);
    v2[2] = backEnd_currentLightScale * *(float *)(backEnd_currentDlight + 44);
    v2[3] = backEnd_currentLightScale * *(float *)(backEnd_currentDlight + 48);
    glProgramLocalParameter4fvARB(34336, 2, backEnd_currentDlight + 36);
    glProgramLocalParameter4fARB(
             34336,
             3,
             *(float *)(backEnd_currentDlight + 96),
             *(float *)(backEnd_currentDlight + 100),
             *(float *)(backEnd_currentDlight + 104),
             1.0f);
    return result;
  }
  return result;
}

/* ---- GL_BindFragmentShaderATI  0x004FEB10 ----  VERIFIED */
void __cdecl GL_BindFragmentShaderATI(int a1)
{
  if ( a1 != glState_boundFragmentShader )
  {
    glBindFragmentShaderATI(a1);
    glState_boundFragmentShader = a1;
  }
}

/* ---- RB_SetupMultitexture  0x004FEB30 ----  VERIFIED */
int __cdecl RB_SetupMultitexture(_DWORD *a1, int a2, GLsizei a3)
{
  int v4;
  _DWORD *v5;
  cvar_t *v6;
  cvar_t *v7;
  int v8;
  _DWORD *v9;
  int result;
  int v11;
  int v12;
  int v13;
  int v14;

  v4 = a1[50 * glState_currentClientTmu + 37];
  v5 = &a1[50 * glState_currentClientTmu];
  v14 = glState_currentClientTmu;
  if ( v4 )
    RB_EnableClientTmu(glState_currentClientTmu, (int)(v5 + 1), *(GLvoid **)(a2 + 4 * glState_currentClientTmu), a3);
  else
    RB_DisableClientTmu(glState_currentClientTmu);
  v6 = (cvar_t *)a1[50 * glState_currentTmu + 37];
  v13 = glState_currentTmu;
  if ( v6 )
    RB_EnableTMU((int)&a1[50 * glState_currentTmu + 1], v6, glState_currentTmu);
  else
    RB_DisableTMU(glState_currentTmu);
  v8 = 0;
  if ( Value > 0 )
  {
    v9 = a1 + 1;
    do
    {
      if ( v8 != v14 )
      {
        if ( v9[36] )
        {
          RB_EnableClientTmu(v8, (int)v9, *(GLvoid **)(a2 + 4 * v8), a3);
        }
        else
        {
          v7 = (cvar_t *)v8;
          if ( ((1 << v8) & glState_clientStateBits) != 0 )
          {
            if ( glState_currentClientTmu != v8 )
            {
              qglClientActiveTextureARB(v8 + 33984);
              glState_currentClientTmu = v8;
            }
            glDisableClientState(0x8078u);
            glState_clientStateBits &= ~(1 << v8);
          }
        }
      }
      if ( v8 != v13 )
      {
        if ( v9[36] )
          RB_EnableTMU((int)v9, v7, v8);
        else
          RB_DisableTMU(v8);
      }
      ++v8;
      v9 += 50;
    }
    while ( v8 < Value );
  }
  result = a1[402];
  if ( result )
    result = RB_SetupRegisterCombiners(a1[402]);
  v11 = a1[403];
  if ( v11 )
    result = RB_SetupVertexProgram(v11);
  v12 = a1[401];
  if ( v12 != glState_boundFragmentShader )
  {
    glBindFragmentShaderATI(a1[401]);
    glState_boundFragmentShader = v12;
  }
  return result;
}

/* ---- RB_SetupMultitextureATI  0x004FECB0 ----  VERIFIED */
int __cdecl RB_SetupMultitextureATI(_DWORD *a1, int a2, int a3, int a4)
{
  _DWORD *v4;
  cvar_t *v5;
  cvar_t *v6;
  int v7;
  _DWORD *v8;
  int result;
  int v10;
  int v11;
  int v12;
  int v13;

  v4 = a1;
  v12 = glState_currentClientTmu;
  if ( a1[50 * glState_currentClientTmu + 37] )
    RB_EnableClientTmuATI(glState_currentClientTmu, (int)&a1[50 * glState_currentClientTmu + 1], a2, *(_DWORD *)(a3 + 4 * glState_currentClientTmu), a4);
  else
    RB_DisableClientTmu(glState_currentClientTmu);
  v5 = (cvar_t *)a1[50 * glState_currentTmu + 37];
  v13 = glState_currentTmu;
  if ( v5 )
    RB_EnableTMU((int)&a1[50 * glState_currentTmu + 1], v5, glState_currentTmu);
  else
    RB_DisableTMU(glState_currentTmu);
  v7 = 0;
  if ( Value > 0 )
  {
    v8 = a1 + 1;
    do
    {
      if ( v7 != v12 )
      {
        if ( v8[36] )
        {
          RB_EnableClientTmuATI(v7, (int)v8, a2, *(_DWORD *)(a3 + 4 * v7), a4);
        }
        else
        {
          v6 = (cvar_t *)v7;
          if ( ((1 << v7) & glState_clientStateBits) != 0 )
          {
            if ( glState_currentClientTmu != v7 )
            {
              qglClientActiveTextureARB(v7 + 33984);
              glState_currentClientTmu = v7;
            }
            glDisableClientState(0x8078u);
            glState_clientStateBits &= ~(1 << v7);
          }
        }
      }
      if ( v7 != v13 )
      {
        if ( v8[36] )
          RB_EnableTMU((int)v8, v6, v7);
        else
          RB_DisableTMU(v7);
      }
      ++v7;
      v8 += 50;
    }
    while ( v7 < Value );
    v4 = a1;
  }
  result = v4[402];
  if ( result )
    result = RB_SetupRegisterCombiners(v4[402]);
  v10 = v4[403];
  if ( v10 )
    result = RB_SetupVertexProgram(v10);
  v11 = v4[401];
  if ( v11 != glState_boundFragmentShader )
  {
    glBindFragmentShaderATI(v4[401]);
    glState_boundFragmentShader = v11;
  }
  return result;
}

/* ---- RB_EndMultitexture  0x004FEE40 ----  VERIFIED */
void RB_EndMultitexture()
{
  int i;
  int v1;

  for ( i = 1; i < Value; ++i )
  {
    if ( (glState_clientStateBits & (1 << i)) != 0 )
    {
      if ( glState_currentClientTmu != i )
      {
        qglClientActiveTextureARB(i + 33984);
        glState_currentClientTmu = i;
      }
      glDisableClientState(0x8078u);
      glState_clientStateBits &= ~(1 << i);
    }
    RB_DisableTMU(i);
  }
  v1 = glState_currentTmu;
  if ( glState_currentTmu )
  {
    qglActiveTextureARB(33984);
    v1 = 0;
    glState_currentTmu = 0;
  }
  if ( glState_currentClientTmu )
  {
    qglClientActiveTextureARB(33984);
    v1 = glState_currentTmu;
    glState_currentClientTmu = 0;
  }
  if ( glState_texEnv[v1] != 8448 )
  {
    glState_texEnv[v1] = 8448;
    glTexEnvi(0x2300u, 0x2200u, 8448);
  }
}

/* ---- RB_SetupStage  0x004FEF20 ----  VERIFIED */
int __cdecl RB_SetupStage(_DWORD *a1, int a2, GLsizei a3)
{
  GL_State(a1[417]);
  return RB_SetupMultitexture(a1, a2, a3);
}

/* ---- RB_SetupStageATI  0x004FEF40 ----  VERIFIED */
int __cdecl RB_SetupStageATI(_DWORD *a1, int a2, int a3, int a4)
{
  GL_State(a1[417]);
  return RB_SetupMultitextureATI(a1, a2, a3, a4);
}

/* ---- DrawTris  0x004FEF70 ----  VERIFIED */
void __cdecl DrawTris(int a1)
{
  void *v1;
  int v2;
  int v3;
  int v4;

  if ( r_showtris->integer > 2 || (v1 = *(void **)(a1 + 1933372), v1 != &backEnd_entity2D) && v1 != &backEnd_debugEntity )
  {
    v2 = tess_activeStageCount;
    v3 = tess_shader;
    v4 = tess_activeStages;
    tess_shader = tr_showTrisShader;
    tess_activeStages = tr_showTrisShader + 340;
    tess_activeStageCount = *(_DWORD *)(tr_showTrisShader + 336);
    if ( (r_showtris->integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    tr_stageIteratorFunc(0);
    glDepthRange(0.0, 1.0);
    tess_activeStages = v4;
    tess_shader = v3;
    tess_activeStageCount = v2;
  }
}

/* ---- DrawNormals  0x004FF020 ----  VERIFIED */
void __cdecl DrawNormals(_DWORD *a1)
{
  int v1;
  int *v2;
  float *v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;

  v1 = 0;
  if ( a1[483337] )
  {
    RB_BeginImmediateMode();
    glDepthRange(0.0, 0.9990000128746033);
    GL_Bind((GLenum *)tr_whiteImage);
    rbDebug_immediateColorR = -1;
    rbDebug_immediateColorG = -1;
    rbDebug_immediateColorB = -1;
    rbDebug_immediateColorA = -1;
    if ( !a1[483339] || !a1[483338] )
      RB_CalcTangentSpace();
    if ( r_shownormals->integer == 1 )
      glDepthRange(0.0, 0.0);
    GL_State(4352);
    mode = 1;
    v2 = tess_xyz;
    if ( (int)a1[483346] > 0 )
    {
      v3 = (float *)(a1 + 180225);
      do
      {
        if ( a1[483338] )
        {
          if ( a1[483339] )
          {
            rbDebug_immediateColorR = -1;
            rbDebug_immediateColorG = 127;
            rbDebug_immediateColorB = 127;
            rbDebug_immediateColorA = -1;
            RB_glVertex3f(*v2, v2[1], v2[2]);
            *(float *)&v5 = *(v3 - 1) + *(v3 - 1) + *(float *)v2;
            *(float *)&v8 = *v3 + *v3 + *((float *)v2 + 1);
            *(float *)&v11 = v3[1] + v3[1] + *((float *)v2 + 2);
            RB_glVertex3f(v5, v8, v11);
            rbDebug_immediateColorR = 127;
            rbDebug_immediateColorG = -1;
            rbDebug_immediateColorB = 127;
            rbDebug_immediateColorA = -1;
            RB_glVertex3f(*v2, v2[1], v2[2]);
            *(float *)&v6 = *(v3 - 24577) + *(v3 - 24577) + *(float *)v2;
            *(float *)&v9 = *(v3 - 24576) + *(v3 - 24576) + *((float *)v2 + 1);
            *(float *)&v12 = *(v3 - 24575) + *(v3 - 24575) + *((float *)v2 + 2);
            RB_glVertex3f(v6, v9, v12);
            rbDebug_immediateColorR = 127;
            rbDebug_immediateColorG = 127;
            rbDebug_immediateColorB = -1;
            rbDebug_immediateColorA = -1;
          }
        }
        RB_glVertex3f(*v2, v2[1], v2[2]);
        *(float *)&v7 = *(v3 - 49153) + *(v3 - 49153) + *(float *)v2;
        *(float *)&v10 = *(v3 - 49152) + *(v3 - 49152) + *((float *)v2 + 1);
        *(float *)&v13 = *(v3 - 49151) + *(v3 - 49151) + *((float *)v2 + 2);
        RB_glVertex3f(v7, v10, v13);
        if ( (_BYTE)v1 == 0xFF )
        {
          glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
          rbDebug_immediateVertexCount = 0;
          mode = 1;
        }
        v2 += tess_vertexComponentCount;
        ++v1;
        v3 += 3;
      }
      while ( v1 < a1[483346] );
    }
    glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
    rbDebug_immediateVertexCount = 0;
    mode = 0;
    glDepthRange(0.0, 1.0);
    rbDebug_immediateModeActive = 0;
    if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
    {
      if ( r_fog->integer )
      {
        v4 = dword_16C4BB0;
        if ( (backEnd_refdef_rdflags & 8) == 0 )
          v4 = glfogNum;
        if ( v4 )
        {
          glEnable(0xB60u);
          glState_glStateBits |= 0x200000u;
        }
      }
    }
    glVertexPointer(3, 0x1406u, 0, tess_xyz);
  }
}

/* ---- DrawColoredNormals  0x004FF310 ----  VERIFIED */
void __cdecl DrawColoredNormals(int a1)
{
  int v1;
  int *v2;
  float *v3;
  double v4;
  double v5;
  int v6;
  int v7;
  int v8;
  int v9;

  RB_BeginImmediateMode();
  GL_Bind((GLenum *)tr_whiteImage);
  rbDebug_immediateColorR = -1;
  rbDebug_immediateColorG = -1;
  rbDebug_immediateColorB = -1;
  rbDebug_immediateColorA = -1;
  GL_State(4352);
  mode = 1;
  v1 = 0;
  v2 = tess_xyz;
  if ( *(int *)(a1 + 1933384) > 0 )
  {
    v3 = (float *)(a1 + 0x80000);
    do
    {
      v4 = (v3[6] + 1.0) * 0.5;
      v5 = (v3[3] + 1.0) * 0.5 * 255.0;
      rbDebug_immediateColorR = (unsigned __int64)((*v3 + 1.0) * 0.5 * 255.0);
      rbDebug_immediateColorG = (unsigned __int64)v5;
      rbDebug_immediateColorB = (unsigned __int64)(v4 * 255.0);
      rbDebug_immediateColorA = -1;
      RB_glVertex3f(*v2, v2[1], v2[2]);
      *(float *)&v7 = *v3 + *v3 + *(float *)v2;
      *(float *)&v8 = v3[1] + v3[1] + *((float *)v2 + 1);
      *(float *)&v9 = v3[2] + v3[2] + *((float *)v2 + 2);
      RB_glVertex3f(v7, v8, v9);
      v2 += tess_vertexComponentCount;
      ++v1;
      v3 += 3;
    }
    while ( v1 < *(_DWORD *)(a1 + 1933384) );
  }
  glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
  rbDebug_immediateVertexCount = 0;
  mode = 0;
  glDepthRange(0.0, 1.0);
  rbDebug_immediateModeActive = 0;
  if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
  {
    if ( r_fog->integer )
    {
      v6 = dword_16C4BB0;
      if ( (backEnd_refdef_rdflags & 8) == 0 )
        v6 = glfogNum;
      if ( v6 )
      {
        glEnable(0xB60u);
        glState_glStateBits |= 0x200000u;
      }
    }
  }
  glVertexPointer(3, 0x1406u, 0, tess_xyz);
}

/* ---- RB_SelectStorageATI  0x004FF4E0 ----  VERIFIED */
void __cdecl RB_SelectStorageATI(int a1)
{
  if ( a1 == 3 )
  {
    glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
    glNormalPointer(0x1406u, 12, tess_stageNormals);
    glTexCoordPointer(2, 0x1406u, 0, tess_activeTexCoords);
  }
}

/* ---- RB_SelectStorage  0x004FF530 ----  VERIFIED */
void __cdecl RB_SelectStorage(int a1)
{
  if ( a1 != glState_currentStorageMode )
  {
    if ( glConfig_NVVertexArrayRange )
    {
      RB_SelectStorageNV(a1);
      glState_currentStorageMode = a1;
    }
    else
    {
      if ( glConfig_ATIVertexArrayObject )
        RB_SelectStorageATI(a1);
      glState_currentStorageMode = a1;
    }
  }
}


int count;

/* ---- RB_BeginSurface  0x004FF570 ----  VERIFIED */
void __cdecl RB_BeginSurface( void *shader, int vertexComponentCount )
{
  int *shd = (int *)shader;

  tess_numIndexes = 0;
  tess_numVertexes = 0;
  tess_optimizedFirstVertex = 0;
  tess_optimizedVertexEnd = 0;
  tess_dlightBits = 0;

  tess_shader = (int)shd;
  tess_activeStages = (int)shd + 0x154;
  tess_activeStageCount = shd[0x150 / 4];
  tess_stageIterator = *(int (**)())&shd[0x174 / 4];
  tess_entity = backEnd_currentEntity;
  tess_requiresVertexBasis = ( shd[0x50 / 4] >> 1 ) & 1;

  tess_stageBitangentsValid = 0;
  tess_stageTangentsValid = 0;
  count         = 0;
  tess_renderedVertexCount = 0;
  tess_vertexComponentCount = vertexComponentCount;

  tess_shaderTime = backEnd_refdef_floatTime - *(float *)&shd[0x17C / 4];

  if ( *(float *)&shd[0x178 / 4] != 0.0f
    && tess_shaderTime >= *(float *)&shd[0x178 / 4] )
  {
    tess_shaderTime = *(float *)&shd[0x178 / 4];
  }
}

/* ---- DrawMultitextured  0x004FF640 ----  VERIFIED */
void __cdecl DrawMultitextured(_DWORD *a1, int a2, GLsizei a3, GLvoid *a4)
{
  GL_State(a1[417]);
  if ( backEnd_wireframeOverride )
    glPolygonMode(0x408u, 0x1B02u);
  RB_SetupMultitexture(a1, (int)&tess_activeTexCoords, 0);
  R_DrawElements(a4, a3);
}

/* ---- RB_BuildDlightArrays  0x004FF690 ----  VERIFIED */
int __fastcall RB_BuildDlightArrays(_BYTE *a1, float *a2, float *a3, int a4)
{
  const float  *dl = a3;
  _BYTE        *colors = a1;
  float        *texCoords = a2;
  unsigned short *hitIndexes;

  float  radius, invRadius;
  float  originX, originY, originZ;
  float  scaleR, scaleG, scaleB;
  float  dx, dy, dz;
  float  s, t, z, az, modulate;
  int    i, numIndexes;
  char   clip;
  unsigned short i0, i1, i2;
  _BYTE  clipBits[8192];              /* SHADER_MAX_VERTEXES, [esp+48h] */

  radius    = dl[29];                 /* +0x74 */
  originX   = dl[30];                 /* +0x78 */
  originY   = dl[31];                 /* +0x7C */
  originZ   = dl[32];                 /* +0x80 */
  invRadius = 1.0f / radius;
  scaleR    = dl[1] * 255.0f;
  scaleG    = dl[2] * 255.0f;
  scaleB    = dl[3] * 255.0f;

  if ( r_dlightQuality->integer && tess_requiresVertexBasis  )
  {
    if ( !tess_stageBitangentsValid || !tess_stageTangentsValid )
    {
      for ( i = 0; i < tess_numVertexes; i++ )
      {
        RB_MakeNormalVectors( &tess_stageBitangents[3 * i],
                              &tess_stageNormals[3 * i],
                              &tess_stageTangents[3 * i] );
      }
      tess_stageBitangentsValid = 1;
      tess_stageTangentsValid = 1;
    }
  }

  for ( i = 0; i < tess_numVertexes ; i++ )
  {
    const float *xyz = &tess_xyz[tess_vertexComponentCount * i];

    if ( r_dlightQuality->integer && tess_requiresVertexBasis )
    {
      const float *tangent   = &tess_stageTangents[3 * i];
      const float *bitangent = &tess_stageBitangents[3 * i];
      const float *normal    = &tess_stageNormals[3 * i];

      dx = originX - xyz[0];
      dy = originY - xyz[1];
      dz = originZ - xyz[2];

      s = dz * tangent[2]   + dx * tangent[0]   + dy * tangent[1];
      t = dz * bitangent[2] + dy * bitangent[1] + dx * bitangent[0];
      z = dz * normal[2]    + dy * normal[1]    + dx * normal[0];
    }
    else
    {
      s = originX - xyz[0];
      t = originY - xyz[1];
      z = originZ - xyz[2];
    }

    ++backEnd_pc_dlightVertexCount;

    clip = 0;
    s = s * invRadius + 0.5f;
    texCoords[0] = s;
    t = t * invRadius + 0.5f;
    texCoords[1] = t;

    if ( s < 0.0f )
      clip = 1;
    else if ( s > 1.0f )
      clip = 2;

    if ( t < 0.0f )
      clip |= 4;
    else if ( t > 1.0f )
      clip |= 8;

    if ( z <= radius )
    {
      if ( -radius <= z )
      {
        az = (float)fabs( z );
        if ( radius * 0.5f <= az )
        {
          modulate = ( radius - az ) * invRadius;
          modulate = modulate + modulate;
        }
        else
        {
          modulate = 1.0f;
        }
      }
      else
      {
        clip |= 0x20;
        modulate = 0.0f;
      }
    }
    else
    {
      clip |= 0x10;
      modulate = 0.0f;
    }

    clipBits[i] = clip;
    colors[0] = (_BYTE)(int)( modulate * scaleR );
    colors[1] = (_BYTE)(int)( modulate * scaleG );
    colors[2] = (_BYTE)(int)( modulate * scaleB );
    colors[3] = (_BYTE)0xFF;

    texCoords += 2;
    colors    += 4;
  }

  hitIndexes = (unsigned short *)a4;
  numIndexes = 0;
  for ( i = 0; i < tess_numIndexes ; i += 3 )
  {
    i2 = ((unsigned short *)tess_indexes)[i + 2];
    i1 = ((unsigned short *)tess_indexes)[i + 1];
    i0 = ((unsigned short *)tess_indexes)[i];
    if ( ( clipBits[i1] & clipBits[i2] & clipBits[i0] ) == 0 )
    {
      hitIndexes[0] = i0;
      hitIndexes[1] = i1;
      hitIndexes[2] = i2;
      numIndexes += 3;
      hitIndexes += 3;
    }
  }
  return numIndexes;
}
#if 0
int __fastcall RB_BuildDlightArrays(_BYTE *a1, float *a2, float *a3, int a4)
{
  int v6;
  float *v7;
  int v8;
  float *v9;
  float *v10;
  double v11;
  double v12;
  double v13;
  double v14;
  char v15;
  double v16;
  double v17;
  int result;
  int v19;
  unsigned __int16 *v20;
  unsigned __int16 v21;
  unsigned __int16 v22;
  unsigned __int16 v23;
  float v24;
  float v25;
  float v26;
  float v27;
  float v28;
  int v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;
  float v35;
  float v36;
  float v37;
  float v38;
  float v39;
  float v40;
  float v41;
  float v42;
  _BYTE v43[8192];
  unsigned int v44;
  unsigned int retaddr;

  v44 = retaddr ^ _security_cookie;
  v36 = a3[31];
  v31 = a3[29];
  v35 = a3[30];
  v37 = a3[32];
  v28 = 1.0 / v31;
  v40 = a3[1] * 255.0;
  v41 = a3[2] * 255.0;
  v42 = a3[3] * 255.0;
  if ( r_dlightQuality->integer )
  {
    if ( tess_requiresVertexBasis )
    {
      if ( !tess_stageBitangentsValid || !tess_stageTangentsValid )
      {
        v6 = 0;
        if ( tess_numVertexes > 0 )
        {
          v7 = (float *)tess_stageTangents;
          do
          {
            RB_MakeNormalVectors(v7 - 24576, v7 - 49152, v7);
            ++v6;
            v7 += 3;
          }
          while ( v6 < tess_numVertexes );
        }
        tess_stageBitangentsValid = 1;
        tess_stageTangentsValid = 1;
      }
    }
  }
  v8 = 0;
  v9 = (float *)tess_xyz;
  if ( tess_numVertexes > 0 )
  {
    v10 = (float *)&unk_1857F64;
    do
    {
      if ( r_dlightQuality->integer && tess_requiresVertexBasis )
      {
        v32 = v35 - *v9;
        v33 = v36 - v9[1];
        v11 = v37 - v9[2];
        v34 = v11;
        v12 = v11 * v10[1] + v32 * *(v10 - 1) + v33 * *v10;
        v13 = v34 * *(v10 - 24575) + v33 * *(v10 - 24576) + v32 * *(v10 - 24577);
        v14 = v34 * *(v10 - 49151) + v33 * *(v10 - 49152) + v32 * *(v10 - 49153);
      }
      else
      {
        v12 = v35 - *v9;
        v13 = v36 - v9[1];
        v14 = v37 - v9[2];
      }
      v38 = v14;
      ++backEnd_pc_dlightVertexCount;
      v15 = 0;
      v24 = v12 * v28 + 0.5;
      *a2 = v24;
      v16 = v13 * v28 + 0.5;
      a2[1] = v16;
      if ( (v24 < 0.0) | __UNORDERED__(v24, 0.0) )
      {
        v15 = 1;
      }
      else if ( v24 > 1.0 )
      {
        v15 = 2;
      }
      if ( (v16 < 0.0) | __UNORDERED__(v16, 0.0) )
      {
        v15 |= 4u;
      }
      else if ( v16 > 1.0 )
      {
        v15 |= 8u;
      }
      if ( v38 <= (double)v31 )
      {
        if ( -v31 <= v38 )
        {
          v39 = fabs(v14);
          if ( v31 * 0.5 <= v39 )
          {
            v17 = (v31 - v39) * v28;
            v30 = v17 + v17;
          }
          else
          {
            v30 = 1.0;
          }
        }
        else
        {
          v15 |= 0x20u;
          v30 = 0.0;
        }
      }
      else
      {
        v15 |= 0x10u;
        v30 = 0.0;
      }
      v43[v8] = v15;
      v25 = v30 * v40;
      *a1 = (int)v25;
      v26 = v30 * v41;
      a1[1] = (int)v26;
      v27 = v30 * v42;
      ftol_tempSpill = (int)v27;
      a1[2] = (int)v27;
      a1[3] = -1;
      ++v8;
      v10 += 3;
      a2 += 2;
      a1 += 4;
      v9 += tess_vertexComponentCount;
    }
    while ( v8 < tess_numVertexes );
  }
  result = 0;
  v19 = 0;
  v29 = 0;
  if ( tess_numIndexes > 0 )
  {
    v20 = (unsigned __int16 *)(a4 + 4);
    do
    {
      v21 = word_17A7F64[v19];
      v22 = word_17A7F62[v19];
      v23 = tess_indexes[v19];
      if ( ((unsigned __int8)(v43[v22] & v43[v21]) & v43[v23]) == 0 )
      {
        *(v20 - 2) = v23;
        *(v20 - 1) = v22;
        *v20 = v21;
        v29 += 3;
        v20 += 3;
      }
      v19 += 3;
    }
    while ( v19 < tess_numIndexes );
    return v29;
  }
  return result;
}
#endif

/* ---- RB_ComputeLocalViewOrigin  no-address ----  VERIFIED */
static void RB_ComputeLocalViewOrigin( float localViewOrigin[3] )
{
  float translated[3];

  if ( (void *)backEnd_currentEntity == (void *)tr_worldEntity )
  {
    localViewOrigin[0] = backEnd_viewParms_originX[0];
    localViewOrigin[1] = backEnd_viewParms_originY;
    localViewOrigin[2] = backEnd_viewParms_originZ;
  }
  else
  {
    translated[0] = backEnd_viewParms_originX[0] - *(float *)( backEnd_currentEntity + 68 );
    translated[1] = backEnd_viewParms_originY    - *(float *)( backEnd_currentEntity + 72 );
    translated[2] = backEnd_viewParms_originZ    - *(float *)( backEnd_currentEntity + 76 );
    VectorRotate( translated, (float *)( backEnd_currentEntity + 28 ), localViewOrigin );
  }
}

/* ---- RB_ComputeColors  0x004FFA60 ----  VERIFIED */
void __cdecl RB_ComputeColors( int stage )
{
  unsigned char (*colors)[4] =
      (unsigned char (*)[4])tess_stageVertexColors;
  const unsigned char (*vertexColors)[4] =
      (const unsigned char (*)[4])tess_vertexColors;
  int   i, c;
  int   rgbGen   = *(int *)( stage + 1636 );
  int   alphaGen = *(int *)( stage + 1660 );
  unsigned char fill;

  switch ( rgbGen )
  {
    case 2:
      for ( i = 0; i < tess_numVertexes; i++ )
        *(int *)colors[i] = -1;
      break;

    case 3:
      RB_CalcColorFromEntity( (unsigned char *)colors );
      break;

    case 4:
      RB_CalcColorFromOneMinusEntity( colors );
      break;

    case 5:
      memcpy( colors, vertexColors, 4 * tess_numVertexes );
      break;

    case 6:
      if ( !tr_overbrightBits )
      {
        memcpy( colors, vertexColors, 4 * tess_numVertexes );
      }
      else
      {
        for ( i = 0; i < tess_numVertexes; i++ )
        {
          colors[i][0] = (unsigned char)( vertexColors[i][0] >> tr_overbrightBits );
          colors[i][1] = (unsigned char)( vertexColors[i][1] >> tr_overbrightBits );
          colors[i][3] = vertexColors[i][3];
          colors[i][2] = (unsigned char)( vertexColors[i][2] >> tr_overbrightBits );
        }
      }
      break;

    case 7:
      if ( tr_identityLight == 1.0f )
      {
        for ( i = 0; i < tess_numVertexes; i++ )
        {
          colors[i][0] = (unsigned char)( 255 - vertexColors[i][0] );
          colors[i][1] = (unsigned char)( 255 - vertexColors[i][1] );
          colors[i][2] = (unsigned char)( 255 - vertexColors[i][2] );
        }
      }
      else
      {
        for ( i = 0; i < tess_numVertexes; i++ )
          for ( c = 0; c < 3; c++ )
            colors[i][c] = (unsigned char)(int)
                ( (float)( 255 - vertexColors[i][c] ) * tr_identityLight );
      }
      break;

    case 8:
      RB_CalcWaveColor( colors, (void *)( stage + 1616 ) );
      break;

    case 9:
    case 10:
      if ( *(int *)( tr_world + 280 ) )
      {
        for ( i = 0; i < tess_numVertexes; i++ )
          *(int *)colors[i] = -1;
      }
      else
      {
        RB_CalcDiffuseColor( colors );
      }
      break;

    case 12:
      for ( i = 0; i < tess_numVertexes; i++ )
        *(int *)colors[i] = *(int *)( stage + 1664 );
      break;

    case 13:
      if ( r_showtris->integer >= 5 )
        RB_ChooseSurfaceCountColor( tess_numIndexes, colors[0] );
      else
        *(int *)colors[0] = *(int *)( stage + 1664 );
      for ( i = 1; i < tess_numVertexes; i++ )
        *(int *)colors[i] = *(int *)colors[0];
      break;

    default:
      fill = (unsigned char)tr_identityLightByte;
      for ( i = 0; i < tess_numVertexes; i++ )
      {
        colors[i][0] = fill;
        colors[i][1] = fill;
        colors[i][2] = fill;
        colors[i][3] = fill;
      }
      break;
  }

  switch ( alphaGen )
  {
    case 0:
      if ( rgbGen == 2 )
        return;
      if ( rgbGen == 6 && tr_identityLight == 1.0f )
        return;
      for ( i = 0; i < tess_numVertexes; i++ )
        colors[i][3] = 0xFF;
      return;

    case 2:
      RB_CalcAlphaFromEntity( (unsigned char *)colors );
      return;

    case 3:
      RB_CalcAlphaFromOneMinusEntity( (unsigned char *)colors );
      return;

    case 4:
      if ( rgbGen != 6 )
      {
        for ( i = 0; i < tess_numVertexes; i++ )
          colors[i][3] = vertexColors[i][3];
      }
      return;

    case 5:
      for ( i = 0; i < tess_numVertexes; i++ )
        colors[i][3] = (unsigned char)( 255 - vertexColors[i][3] );
      return;

    case 6:
      RB_CalcSpecularAlpha( (int)colors );
      return;

    case 7:
      RB_CalcWaveAlpha( (void *)( stage + 1640 ), colors );
      return;

    case 8:
      for ( i = 0; i < tess_numVertexes; i++ )
      {
        const float *xyz = &tess_xyz[tess_vertexComponentCount * i];
        float dx = xyz[0] - backEnd_viewParms_originX[0];
        float dy = xyz[1] - backEnd_viewParms_originY;
        float dz = xyz[2] - backEnd_viewParms_originZ;
        float alpha = (float)
            ( sqrt( (double)( dz * dz + dy * dy ) + (double)( dx * dx ) )
              / *(float *)( tess_shader + 164 ) );

        if ( alpha < 0.0f )
          colors[i][3] = 0;
        else if ( alpha > 1.0f )
          colors[i][3] = 0xFF;
        else
          colors[i][3] = (unsigned char)(int)( alpha * 255.0f );
      }
      return;

    case 9:
      if ( rgbGen != 12 )
      {
        for ( i = 0; i < tess_numVertexes; i++ )
          colors[i][3] = *(unsigned char *)( stage + 1667 );
      }
      return;

    case 10:
    case 11:
    case 12:
    case 13:
    {
      float localViewOrigin[3];

      RB_ComputeLocalViewOrigin( localViewOrigin );

      for ( i = 0; i < tess_numVertexes; i++ )
      {
        const float *xyz    = &tess_xyz[tess_vertexComponentCount * i];
        const float *normal = &tess_stageNormals[3 * i];
        float  direction[3];
        float  alpha;
        int    bits;

        direction[0] = localViewOrigin[0] - xyz[0];
        direction[1] = localViewOrigin[1] - xyz[1];
        direction[2] = localViewOrigin[2] - xyz[2];
        VectorNormalizeFast( direction );

        if ( alphaGen == 12 )
        {
          alpha = direction[0] * normal[0] + direction[2] * normal[2]
                + direction[1] * normal[1];
        }
        else
        {
          alpha = direction[2] * normal[2] + direction[0] * normal[0]
                + direction[1] * normal[1];
          if ( alphaGen == 13 )
            alpha = -alpha;
        }

        memcpy( &bits, &alpha, 4 );
        if ( alphaGen == 12 )
          bits &= ( bits >> 31 );
        else
          bits &= ~( bits >> 31 );
        memcpy( &alpha, &bits, 4 );

        if ( alphaGen == 11 )
          alpha = 1.0f - alpha;
        else if ( alphaGen == 12 )
          alpha = alpha + 1.0f;

        colors[i][3] = (unsigned char)(int)( alpha * 255.0f + 0.5f );
      }
      return;
    }

    default:
      return;
  }
}
#if 0
double __cdecl RB_ComputeColors(int a1, double result, int a3)
{
  int v4;
  int mm;
  char v6;
  int i;
  char v8;
  int j;
  char v10;
  int k;
  unsigned __int64 v12; // rax
  int nn;
  int v14;
  int m;
  int kk;
  int n;
  int ii;
  int v19;
  float *jj;
  double v21;
  double v22;
  double v23;
  long double v24;
  unsigned __int64 v25; // rax
  int v26;
  float *v27;
  float *v28;
  int v29;
  float *v30;
  float *v31;
  int v32;
  float *v33;
  float *v34;
  int v35;
  float *v36;
  float *v37;
  float v38; // [esp+10h] [ebp-18h] BYREF
  float v39;
  float v40;
  float v41; // [esp+1Ch] [ebp-Ch] BYREF
  float v42;
  float v43;
  int v44;
  float v45;
  int v46;
  int v47;
  int v48;
  int v49;

  switch ( *(_DWORD *)(a3 + 1636) )
  {
    /* Retail biases the selector before the table -- 0x004FFA6F
     * `add eax, 0FFFFFFFEh` then `cmp eax, 0Bh / ja def_4FFA7D` -- so rgbGen
     * 2..13 index the jump table and rgbGen 0 (CGEN_BAD) and 1
     * (CGEN_IDENTITY_LIGHTING) both fall to the DEFAULT.  rgbGen
     * identityLighting is the common case for UI and 2D shaders.
     *
     * Retail's arm, 0x004FFCEC..0x004FFD08: replicate tr.identityLightByte
     * into all four lanes and `rep stosd` it over numVertexes dwords -- Q3's
     *     default: case CGEN_IDENTITY_LIGHTING:
     *         Com_Memset( tess.svars.colors, tr.identityLightByte,
     *                     tess.numVertexes * 4 );
     * The byte replication is the `mov dl,al / mov dh,dl / shl eax,10h /
     * mov ax,dx` sequence, i.e. 0x01010101 * the byte. */
    default:
      v4 = 0x01010101 * (unsigned __int8) tr_identityLightByte;
      goto LABEL_30;
    case 2:
      goto LABEL_3;
    case 3:
      RB_CalcColorFromEntity(tess_stageVertexColors);
      goto LABEL_31;
    case 4:
      RB_CalcColorFromOneMinusEntity(tess_stageVertexColors);
      goto LABEL_31;
    case 5:
      goto LABEL_9;
    case 6:
      v6 = tr_overbrightBits;
      if ( tr_overbrightBits )
      {
        for ( i = 0; i < tess_numVertexes; byte_186FF5E[4 * i] = v8 )
        {
          LOBYTE(tess_stageVertexColors[i]) = LOBYTE(tess_vertexColors[i]) >> v6;
          BYTE1(tess_stageVertexColors[i]) = BYTE1(tess_vertexColors[i]) >> v6;
          v8 = BYTE2(tess_vertexColors[i]) >> v6;
          BYTE3(tess_stageVertexColors[i]) = BYTE3(tess_vertexColors[i]);
          ++i;
        }
      }
      else
      {
LABEL_9:
        qmemcpy(tess_stageVertexColors, tess_vertexColors, 4 * tess_numVertexes);
      }
      goto LABEL_31;
    case 7:
      if ( (tr_identityLight == 1.0) | __UNORDERED__(tr_identityLight, 1.0) )
      {
        for ( j = 0; j < tess_numVertexes; ++j )
        {
          v10 = BYTE2(tess_vertexColors[j]);
          LOBYTE(tess_stageVertexColors[j]) = -1 - LOBYTE(tess_vertexColors[j]);
          BYTE1(tess_stageVertexColors[j]) = -1 - BYTE1(tess_vertexColors[j]);
          BYTE2(tess_stageVertexColors[j]) = -1 - v10;
        }
      }
      else
      {
        for ( k = 0; k < tess_numVertexes; ++k )
        {
          v12 = (unsigned __int64)((double)(255 - LOBYTE(tess_vertexColors[k])) * tr_identityLight);
          HIDWORD(v12) = BYTE1(tess_vertexColors[k]);
          LOBYTE(tess_stageVertexColors[k]) = v12;
          v44 = 255 - BYTE2(tess_vertexColors[k]);
          BYTE1(tess_stageVertexColors[k]) = (unsigned __int64)((double)(255 - HIDWORD(v12)) * tr_identityLight);
          BYTE2(tess_stageVertexColors[k]) = (unsigned __int64)((double)v44 * tr_identityLight);
        }
      }
      goto LABEL_31;
    case 8:
      RB_CalcWaveColor(tess_stageVertexColors, a3 + 1616, result);
      goto LABEL_31;
    case 9:
    case 0xA:
      if ( *(_DWORD *)(tr_world + 280) )
      {
LABEL_3:
        v4 = -1;
LABEL_30:
        memset32(tess_stageVertexColors, v4, tess_numVertexes);
      }
      else
      {
        RB_CalcDiffuseColor((int)tess_stageVertexColors);
      }
LABEL_31:
      switch ( *(_DWORD *)(a3 + 1660) )
      {
        case 0:
          v14 = *(_DWORD *)(a3 + 1636);
          if ( v14 != 2 && (v14 != 6 || !((tr_identityLight == 1.0) | __UNORDERED__(tr_identityLight, 1.0))) )
          {
            for ( m = 0; m < tess_numVertexes; ++m )
              BYTE3(tess_stageVertexColors[m]) = -1;
          }
          break;
        case 2:
          RB_CalcAlphaFromEntity((char *)tess_stageVertexColors);
          break;
        case 3:
          RB_CalcAlphaFromOneMinusEntity((char *)tess_stageVertexColors);
          break;
        case 4:
          if ( *(_DWORD *)(a3 + 1636) != 6 )
          {
            for ( n = 0; n < tess_numVertexes; ++n )
              BYTE3(tess_stageVertexColors[n]) = BYTE3(tess_vertexColors[n]);
          }
          break;
        case 5:
          for ( ii = 0; ii < tess_numVertexes; ++ii )
            BYTE3(tess_stageVertexColors[ii]) = -1 - BYTE3(tess_vertexColors[ii]);
          break;
        case 6:
          RB_CalcSpecularAlpha((int)tess_stageVertexColors);
          break;
        case 7:
          RB_CalcWaveAlpha(a3 + 1640, (int)tess_stageVertexColors);
          break;
        case 8:
          v19 = 0;
          for ( jj = (float *)tess_xyz; v19 < tess_numVertexes; ++v19 )
          {
            v21 = jj[1] - backEnd_viewParms_originY;
            v22 = jj[2] - backEnd_viewParms_originZ;
            v23 = *jj - backEnd_viewParms_originX[0];
            v24 = sqrt(v22 * v22 + v21 * v21 + v23 * v23) / *(float *)(tess_shader + 164);
            v45 = v24;
            if ( (v24 < 0.0) | __UNORDERED__(v24, 0.0) )
            {
              LOBYTE(v25) = 0;
            }
            else if ( v45 <= 1.0 )
            {
              v25 = (unsigned __int64)(v45 * 255.0);
            }
            else
            {
              LOBYTE(v25) = -1;
            }
            BYTE3(tess_stageVertexColors[v19]) = v25;
            jj += tess_vertexComponentCount;
          }
          break;
        case 9:
          if ( *(_DWORD *)(a3 + 1636) != 12 )
          {
            for ( kk = 0; kk < tess_numVertexes; ++kk )
              BYTE3(tess_stageVertexColors[kk]) = *(_BYTE *)(a3 + 1667);
          }
          break;
        case 0xA:
          if ( (_UNKNOWN *)backEnd_currentEntity == &tr_worldEntity )
          {
            v41 = backEnd_viewParms_originX[0];
            v42 = backEnd_viewParms_originY;
            v43 = backEnd_viewParms_originZ;
          }
          else
          {
            v38 = backEnd_viewParms_originX[0] - *(float *)(backEnd_currentEntity + 68);
            v39 = backEnd_viewParms_originY - *(float *)(backEnd_currentEntity + 72);
            v40 = backEnd_viewParms_originZ - *(float *)(backEnd_currentEntity + 76);
            /* VectorRotate(in, matrix, out): in = &v38 (the vector just
             * written above), matrix = currentEntity->axis (+28), out = &v41
             * (read by the loop below). */
            VectorRotate(&v38, (float *)(backEnd_currentEntity + 28), &v41);
          }
          v26 = 0;
          v27 = (float *)tess_xyz;
          if ( tess_numVertexes > 0 )
          {
            v28 = flt_1827F64;
            do
            {
              v38 = v41 - *v27;
              v39 = v42 - v27[1];
              v40 = v43 - v27[2];
              VectorNormalizeFast(&v38);
              *(float *)&v46 = v40 * v28[1] + v38 * *(v28 - 1) + v39 * *v28;
              BYTE3(tess_stageVertexColors[v26++]) = (unsigned __int64)(COERCE_FLOAT(v46 & ~(v46 >> 31)) * 255.0 + 0.5);
              v28 += 3;
              v27 += tess_vertexComponentCount;
            }
            while ( v26 < tess_numVertexes );
          }
          break;
        case 0xB:
          if ( (_UNKNOWN *)backEnd_currentEntity == &tr_worldEntity )
          {
            v41 = backEnd_viewParms_originX[0];
            v42 = backEnd_viewParms_originY;
            v43 = backEnd_viewParms_originZ;
          }
          else
          {
            v38 = backEnd_viewParms_originX[0] - *(float *)(backEnd_currentEntity + 68);
            v39 = backEnd_viewParms_originY - *(float *)(backEnd_currentEntity + 72);
            v40 = backEnd_viewParms_originZ - *(float *)(backEnd_currentEntity + 76);
            /* VectorRotate(in, matrix, out): in = &v38 (the vector just
             * written above), matrix = currentEntity->axis (+28), out = &v41
             * (read by the loop below). */
            VectorRotate(&v38, (float *)(backEnd_currentEntity + 28), &v41);
          }
          v29 = 0;
          v30 = (float *)tess_xyz;
          if ( tess_numVertexes > 0 )
          {
            v31 = flt_1827F64;
            do
            {
              v38 = v41 - *v30;
              v39 = v42 - v30[1];
              v40 = v43 - v30[2];
              VectorNormalizeFast(&v38);
              *(float *)&v47 = v40 * v31[1] + v38 * *(v31 - 1) + v39 * *v31;
              BYTE3(tess_stageVertexColors[v29++]) = (unsigned __int64)((1.0 - COERCE_FLOAT(v47 & ~(v47 >> 31))) * 255.0 + 0.5);
              v31 += 3;
              v30 += tess_vertexComponentCount;
            }
            while ( v29 < tess_numVertexes );
          }
          break;
        case 0xC:
          if ( (_UNKNOWN *)backEnd_currentEntity == &tr_worldEntity )
          {
            v41 = backEnd_viewParms_originX[0];
            v42 = backEnd_viewParms_originY;
            v43 = backEnd_viewParms_originZ;
          }
          else
          {
            v38 = backEnd_viewParms_originX[0] - *(float *)(backEnd_currentEntity + 68);
            v39 = backEnd_viewParms_originY - *(float *)(backEnd_currentEntity + 72);
            v40 = backEnd_viewParms_originZ - *(float *)(backEnd_currentEntity + 76);
            /* VectorRotate(in, matrix, out): in = &v38 (the vector just
             * written above), matrix = currentEntity->axis (+28), out = &v41
             * (read by the loop below). */
            VectorRotate(&v38, (float *)(backEnd_currentEntity + 28), &v41);
          }
          v32 = 0;
          v33 = (float *)tess_xyz;
          if ( tess_numVertexes > 0 )
          {
            v34 = flt_1827F64;
            do
            {
              v38 = v41 - *v33;
              v39 = v42 - v33[1];
              v40 = v43 - v33[2];
              VectorNormalizeFast(&v38);
              *(float *)&v48 = v38 * *(v34 - 1) + v40 * v34[1] + v39 * *v34;
              BYTE3(tess_stageVertexColors[v32++]) = (unsigned __int64)((COERCE_FLOAT(v48 & (v48 >> 31)) + 1.0) * 255.0 + 0.5);
              v34 += 3;
              v33 += tess_vertexComponentCount;
            }
            while ( v32 < tess_numVertexes );
          }
          break;
        case 0xD:
          if ( (_UNKNOWN *)backEnd_currentEntity == &tr_worldEntity )
          {
            v41 = backEnd_viewParms_originX[0];
            v42 = backEnd_viewParms_originY;
            v43 = backEnd_viewParms_originZ;
          }
          else
          {
            v38 = backEnd_viewParms_originX[0] - *(float *)(backEnd_currentEntity + 68);
            v39 = backEnd_viewParms_originY - *(float *)(backEnd_currentEntity + 72);
            v40 = backEnd_viewParms_originZ - *(float *)(backEnd_currentEntity + 76);
            /* VectorRotate(in, matrix, out): in = &v38 (the vector just
             * written above), matrix = currentEntity->axis (+28), out = &v41
             * (read by the loop below). */
            VectorRotate(&v38, (float *)(backEnd_currentEntity + 28), &v41);
          }
          v35 = 0;
          v36 = (float *)tess_xyz;
          if ( tess_numVertexes > 0 )
          {
            v37 = flt_1827F64;
            do
            {
              v38 = v41 - *v36;
              v39 = v42 - v36[1];
              v40 = v43 - v36[2];
              VectorNormalizeFast(&v38);
              *(float *)&v49 = -(v40 * v37[1] + v38 * *(v37 - 1) + v39 * *v37);
              BYTE3(tess_stageVertexColors[v35++]) = (unsigned __int64)(COERCE_FLOAT(v49 & ~(v49 >> 31)) * 255.0 + 0.5);
              v37 += 3;
              v36 += tess_vertexComponentCount;
            }
            while ( v35 < tess_numVertexes );
          }
          break;
        default:
          return result;
      }
      return result;
    case 0xC:
      for ( mm = 0; mm < tess_numVertexes; ++mm )
        tess_stageVertexColors[mm] = *(_DWORD *)(a3 + 1664);
      goto LABEL_31;
    case 0xD:
      if ( r_showtris->integer < 5 )
        tess_stageVertexColors[0] = *(_DWORD *)(a3 + 1664);
      else
        RB_ChooseSurfaceCountColor(tess_numIndexes, tess_stageVertexColors);
      for ( nn = 1; nn < tess_numVertexes; ++nn )
        tess_stageVertexColors[nn] = tess_stageVertexColors[0];
      goto LABEL_31;
    default:
      LOBYTE(a1) = tr_identityLightByte;
      BYTE1(a1) = tr_identityLightByte;
      v4 = a1 << 16;
      LOWORD(v4) = a1;
      goto LABEL_30;
  }
}
#endif

/* ---- RB_ComputeTexCoords  0x00500400 ----  VERIFIED */
int __cdecl RB_ComputeTexCoords(int a1)
{
  int a2;
  _DWORD *v2;
  int v3;
  GLvoid **v4;
  int result;
  float *i;
  int *v7;
  int *v8;
  int v9;
  int j;
  float *v11;
  float *v12;
  double v13;
  int v14;
  _DWORD *v15;
  int texUnit;

  texUnit = 0;
  v2 = &tess_generatedTexCoords;
  v3 = a1 + 172;
  v4 = &tess_activeTexCoords;
  v15 = &tess_generatedTexCoords;
  v14 = a1 + 172;
  while ( 2 )
  {
    *v4 = v2;
    result = *(_DWORD *)(v3 - 12);
    switch ( result )
    {
      case 0:
        return result;
      case 1:
        memset(v2, 0, 8 * tess_numVertexes);
        a2 = 0;
        goto LABEL_43;
      case 2:
        if ( setArraysOnce || *(_DWORD *)(v3 + 16) )
        {
          qmemcpy(v2, tess_texCoords1, 8 * tess_numVertexes);
          a2 = 0;
          v3 = v14;
        }
        else
        {
          *v4 = tess_texCoords1;
        }
        goto LABEL_43;
      case 3:
        if ( setArraysOnce || *(_DWORD *)(v3 + 16) )
        {
          qmemcpy(v2, tess_texCoords0, 8 * tess_numVertexes);
          a2 = 0;
          v3 = v14;
        }
        else
        {
          *v4 = tess_texCoords0;
        }
        goto LABEL_43;
      case 4:
        RB_CalcEnvironmentTexCoords((int)v2);
        goto LABEL_43;
      case 6:
        a2 = 0;
        for ( i = (float *)tess_xyz; a2 < tess_numVertexes; i += tess_vertexComponentCount )
        {
          ++a2;
          *((float *)*v4 + 2 * a2 - 2) = *(float *)(v3 - 4) * i[1] + *(float *)(v3 - 8) * *i + i[2] * *(float *)v3;
          *((float *)*v4 + 2 * a2 - 1) = *(float *)(v3 + 4) * *i
                                       + i[2] * *(float *)(v3 + 12)
                                       + *(float *)(v3 + 8) * i[1];
        }
        goto LABEL_43;
      case 7:
        if ( !*(_DWORD *)(v3 + 16) )
        {
          *v4 = tess_stageNormals;
          goto LABEL_43;
        }
        v7 = (int *)tess_stageNormals;
        v8 = v2;
        goto LABEL_19;
      case 8:
        if ( !tess_stageTangentsValid )
          RB_CalcTangentSpace();
        if ( *(_DWORD *)(v3 + 16) )
        {
          qmemcpy(*v4, tess_stageTangents, 12 * tess_numVertexes);
          a2 = 0;
          v3 = v14;
        }
        else
        {
          *v4 = tess_stageTangents;
        }
        goto LABEL_43;
      case 9:
        if ( !tess_stageBitangentsValid )
          RB_CalcTangentSpace();
        if ( *(_DWORD *)(v3 + 16) )
        {
          v8 = (int *)*v4;
          v7 = tess_stageBitangents;
LABEL_19:
          qmemcpy(v8, v7, 12 * tess_numVertexes);
          a2 = 0;
          v3 = v14;
        }
        else
        {
          *v4 = tess_stageBitangents;
        }
LABEL_43:
        result = *(_DWORD *)(v3 + 16);
        for ( j = 0; j < result; ++j )
        {
          v11 = (float *)(68 * j + *(_DWORD *)(v3 + 20));
          a2 = *(_DWORD *)v11;
          switch ( *(_DWORD *)v11 )
          {
            case 0:
              j = 4;
              break;
            case 1:
              RB_CalcTransformTexCoords(v11, (float *)*v4);
              break;
            case 2:
              RB_CalcTurbulentTexCoords((float *)*v4, v11 + 1);
              v2 = v15;
              v3 = v14;
              break;
            case 3:
              RB_CalcScrollTexCoords(v11 + 14, (float *)*v4);
              break;
            case 4:
              RB_CalcScaleTexCoords((float *)*v4, v11 + 12);
              break;
            case 5:
              RB_CalcStretchTexCoords((int)(v11 + 1), (float *)*v4);
              v3 = v14;
              break;
            case 6:
              RB_CalcRotateTexCoords(v11[16], (float *)*v4);
              break;
            case 7:
              RB_CalcScrollTexCoords((float *)(backEnd_currentEntity + 112), (float *)*v4);
              break;
            case 8:
              RB_CalcSwapTexCoords((float *)*v4);
              break;
            case 9:
              v12 = (float *)*v4;
              a2 = 3 * tess_numVertexes;
              if ( 3 * tess_numVertexes > 0 )
              {
                do
                {
                  v13 = *v12++;
                  --a2;
                  *(v12 - 1) = -v13;
                }
                while ( a2 );
              }
              break;
            case 0xA:
              RB_CalcCubeMapBumpmapFrameTexCoords((float *)*v4);
              break;
            default:
              ri_Error(1, "\x15" "ERROR: unknown texmod '%d' in shader '%s'\n", a2, tess_shader);
              break;
          }
          result = *(_DWORD *)(v3 + 16);
        }
        v2 += 0x8000;
        v3 += 200;
        ++v4;
        v15 = v2;
        v14 = v3;
        ++texUnit;
        if ( texUnit < 8 )
          continue;
        return result;
      case 10:
        RB_CalcCubeMapTbnTexCoords(v2, 0);
        goto LABEL_43;
      case 11:
        RB_CalcCubeMapTbnTexCoords(v2, 1);
        goto LABEL_43;
      case 12:
        RB_CalcCubeMapTbnTexCoords(v2, 2);
        goto LABEL_43;
      case 13:
        RB_CalcCubeMapVertexToEyeTexCoords((int)v2);
        goto LABEL_43;
      case 14:
        RB_CalcCubeMapEyeToVertexTexCoords((int)v2);
        goto LABEL_43;
      case 15:
        RB_CalcCubeMapReflectionTexCoords(a2, (int)v2);
        goto LABEL_43;
      case 16:
        RB_CalcCubeMapLightVectorTexCoords((int)v2);
        goto LABEL_43;
      case 17:
        RB_CalcCubeMapLightHalfAngleTexCoords((int)v2);
        goto LABEL_43;
      case 18:
        RB_CalcCubeMapSunHalfAngleTexCoords((int)v2);
        goto LABEL_43;
      case 19:
        v9 = 0;
        goto LABEL_42;
      case 20:
        v9 = 1;
        goto LABEL_42;
      case 21:
        v9 = 2;
LABEL_42:
        RB_CalcCubeMapDot3ReflectTexCoords(v9, (float *)v2);
        goto LABEL_43;
      default:
        goto LABEL_43;
    }
  }
}

/* ---- RB_SetIteratorFog  0x00500810 ----  VERIFIED */
void __cdecl RB_SetIteratorFog(void)
{
  if ( (backEnd_refdef_rdflags & 1) != 0 )
  {
LABEL_2:
    if ( (glState_glStateBits & 0x200000) != 0 )
    {
      glDisable(0xB60u);
      glState_glStateBits &= ~0x200000u;
    }
    return;
  }
  if ( (backEnd_refdef_rdflags & 0x40) != 0 )
  {
    if ( dword_16C4B70 )
    {
      R_Fog((int)&unk_16C4B40);
      return;
    }
    goto LABEL_2;
  }
  if ( dword_16DCA54 && (backEnd_refdef_rdflags & 8) != 0 )
  {
    if ( dword_16C4BB0 )
    {
      R_Fog((int)&unk_16C4B80);
      return;
    }
  }
  else if ( glfogNum > 0 )
  {
    R_Fog((int)&dword_16C4C40);
    return;
  }
  if ( (glState_glStateBits & 0x200000) != 0 )
  {
    glDisable(0xB60u);
    glState_glStateBits &= ~0x200000u;
  }
}

/* ---- RB_SingleStageGeneric  0x005008A0 ----  VERIFIED */
void __cdecl RB_SingleStageGeneric(_DWORD *stage, GLvoid *indexes, GLsizei numIndexes)
{
  cvar_t *v7;
  GLenum *AnimatedImage;
  int v9;
  int v10;

  _DWORD *a1 = stage;
  GLvoid *a3 = indexes;
  GLsizei a5 = numIndexes;

  RB_ComputeColors((int)a1);
  RB_ComputeTexCoords((int)a1);
  if ( !setArraysOnce )
  {
    if ( (glState_clientStateBits & 0x100) == 0 )
    {
      glEnableClientState(0x8076u);
      glState_clientStateBits |= 0x100u;
    }
    glColorPointer(4, 0x1401u, 0, tess_stageVertexColors);
  }
  if ( a1[51] )
  {
    GL_State(a1[417]);
    if ( backEnd_wireframeOverride )
      glPolygonMode(0x408u, 0x1B02u);
    RB_SetupMultitexture(a1, (int)&tess_activeTexCoords, 0);
    R_DrawElements(a3, a5);
  }
  else
  {
    if ( !setArraysOnce )
      glTexCoordPointer(a1[36], 0x1406u, 0, tess_activeTexCoords);
    if ( (*(_DWORD *)(*(_DWORD *)(tess_shader + 340) + 1668) & 0x100000) != 0
      && (v7 = r_debugEntLight, r_debugEntLight->integer) )
    {
      AnimatedImage = (GLenum *)tr_whiteImage;
    }
    else
    {
      AnimatedImage = (GLenum *)RB_GetAnimatedImage((int)v7, (int)(a1 + 1), glState_currentTmu);
    }
    GL_Bind(AnimatedImage);
    GL_State(a1[417]);
    if ( a1[402] )
      RB_SetupRegisterCombiners(a1[402]);
    v9 = a1[403];
    if ( v9 )
      RB_SetupVertexProgram(v9);
    v10 = a1[401];
    if ( v10 != glState_boundFragmentShader )
    {
      glBindFragmentShaderATI(v10);
      glState_boundFragmentShader = v10;
    }
    R_DrawElements(a3, a5);
  }
}

/* ---- ProjectDlightTexture  0x00500A10 ----  VERIFIED */
int __cdecl ProjectDlightTexture(void)
{
  int result;
  char v2;
  int v3;
  int v4;
  int v5;
  GLsizei v6;
  int i;
  char *v8;
  _DWORD *v9;
  int v10;
  _BYTE v11[98308]; // [esp+18h] [ebp-1800Ch] BYREF
  result = backEnd_refdef_num_dlights;
  if ( backEnd_refdef_num_dlights )
  {
    RB_EndMultitexture();
    result = backEnd_refdef_num_dlights;
    v2 = 0;
    v10 = 0;
    if ( backEnd_refdef_num_dlights > 0 )
    {
      v3 = 0;
      do
      {
        if ( ((1 << v2) & tess_dlightBits) != 0 )
        {
          v4 = backEnd_refdef_dlights + v3;
          v6 = RB_BuildDlightArrays(tess_vertexColors, tess_texCoords1, (float *)(backEnd_refdef_dlights + v3), (int)v11);
          if ( v6 )
          {
            if ( *(char *)(tess_shader + 84) >= 0 )
            {
              v9 = *(_DWORD **)(tr_dlightShader + 340);
              if ( v9 )
                RB_SingleStageGeneric(v9, v11, v6);
            }
            else
            {
              backEnd_currentDlight = v4;
              for ( i = 0; i < 32; i += 4 )
              {
                v8 = *(char **)(i + tess_activeStages);
                if ( !v8 )
                  break;
                if ( *v8 < 0 )
                  RB_SingleStageGeneric(v8, v11, v6);
              }
            }
          }
        }
        result = backEnd_refdef_num_dlights;
        v2 = v10 + 1;
        v3 += 136;
        ++v10;
      }
      while ( v10 < backEnd_refdef_num_dlights );
    }
  }
  return result;
}

/* ---- RB_IterateStagesGeneric  0x00500B30 ----  VERIFIED */
void __cdecl RB_IterateStagesGeneric( void )
{
  int   i;
  char *stage;

  RB_EndMultitexture();

  for ( i = 0; i < 32; i += 4 )
  {
    stage = *(char **)( i + tess_activeStages  );
    if ( !stage )
      break;

    if ( *stage >= 0 )
    {
      RB_SingleStageGeneric( (_DWORD *)stage, tess_indexes, tess_numIndexes );
      if ( r_lightmap->integer )
      {
        if ( *(unsigned char *)( stage + 200 ) )
          break;
        if ( *(unsigned char *)( stage + 400 ) )
          break;
      }
    }
  }
}

/* ---- RB_StageIteratorGeneric  0x00500BA0 ----  STATEMENT-DIFF CLEAN */
int (*__cdecl RB_StageIteratorGeneric(int a4))(void)
{
  int (*result)(void);
  int v5;
  char *v6;
  int v8;
  int v9;
  int v10;
  int v11;
  char *v12;
  int v13;

  result = (int (*)(void))tess_activeStageCount;
  if ( tess_activeStageCount > 0 && (!a4 || (result = (int (*)(void))tess_dlightBits) != 0) )
  {
    RB_DeformTessGeometry();
    v5 = (*(_DWORD *)(tess_shader + 80) >> 8) & 1;
    if ( r_logFile->integer )
    {
      v6 = va("--- RB_StageIteratorGeneric( %s ) ---\n", (const char *)tess_shader);
      if ( Stream )
        fprintf(Stream, "%s", (int)v6);
    }
    RB_EndMultitexture();
    RB_SetIteratorFog();                 /* takes no arguments -- 0x00500C1B */
    GL_Cull(*(_DWORD *)(tess_shader + 168));
    if ( tess_activeStageCount > 1 || *(_DWORD *)(*(_DWORD *)(tess_shader + 340) + 348) )
    {
      if ( !v5 )
      {
        GL_ClientState(1024);
LABEL_15:
        glVertexPointer(tess_vertexComponentCount, 0x1406u, 0, tess_xyz);
        if ( qglLockArraysEXT )
          qglLockArraysEXT(0, tess_numVertexes);
        if ( !setArraysOnce )
        {
          if ( v5 )
            GL_ClientState(1793);
          else
            GL_ClientState(1281);
        }
        if ( !a4 )
          RB_IterateStagesGeneric();
        if ( !tess_dlightBits || *(float *)(tess_shader + 88) > 5.0 || (*(_DWORD *)(tess_shader + 92) & 0x20004) != 0 )
        {
          if ( *(char *)(tess_shader + 84) < 0 )
          {
            v8 = backEnd_currentEntity;
            if ( backEnd_currentEntity )
            {
              v9 = *(_DWORD *)(backEnd_currentEntity + 204);
              if ( v9 )
              {
                v10 = 0;
                if ( v9 > 0 )
                {
                  do
                  {
                    backEnd_currentDlight = *(_DWORD *)(v8 + 8 * v10 + 208);
                    if ( *(_DWORD *)backEnd_currentDlight != 8 )
                    {
                      v11 = 0;
                      backEnd_currentLightScale = *(float *)(v8 + 8 * v10 + 212);
                      do
                      {
                        v12 = *(char **)(v11 + tess_activeStages);
                        if ( !v12 )
                          break;
                        if ( *v12 < 0 )
                        {
                          RB_SingleStageGeneric(v12, tess_indexes, tess_numIndexes);
                          v8 = backEnd_currentEntity;
                        }
                        v11 += 4;
                      }
                      while ( v11 < 32 );
                    }
                    ++v10;
                  }
                  while ( v10 < *(_DWORD *)(v8 + 204) );
                }
                backEnd_currentDlight = 0;
              }
            }
          }
        }
        else
        {
          ProjectDlightTexture();
        }
        result = (int)qglUnlockArraysEXT;
        if ( qglUnlockArraysEXT )
          qglUnlockArraysEXT();
        setArraysOnce = 0;
        return result;
      }
      GL_ClientState(1536);
    }
    else
    {
      setArraysOnce = 1;
      if ( v5 )
        v13 = 1793;
      else
        v13 = 1281;
      GL_ClientState(v13);
      glColorPointer(4, 0x1401u, 0, tess_stageVertexColors);
      glTexCoordPointer(*(_DWORD *)(*(_DWORD *)(tess_shader + 340) + 144), 0x1406u, 0, &tess_generatedTexCoords);
    }
    if ( v5 )
      glNormalPointer(0x1406u, 12, tess_stageNormals);
    goto LABEL_15;
  }
  return result;
}

/* ---- RB_EndSurface_OptimizedGeneric  0x00500E40 ----  VERIFIED */
void __cdecl RB_EndSurface_OptimizedGeneric( void )
{
  int i;
  char *v2;
  int integer;
  _DWORD *v4;
  GLubyte v[4]; // [esp+4Ch] [ebp-28h] BYREF
  _DWORD v7[9]; // [esp+50h] [ebp-24h] BYREF

  if ( r_lightmap->integer )
  {
    if ( glState_texEnv[glState_currentTmu] != 7681 )
    {
      glState_texEnv[glState_currentTmu] = 7681;
      glTexEnvi(0x2300u, 0x2200u, 7681);
    }
  }
  else
  {
    GL_TexEnv(*(_DWORD *)(*(_DWORD *)(tess_shader + 340) + 348));
  }
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  glColorPointer(4, 0x1401u, 32, (const GLvoid *)(*(_DWORD *)(tess_shader + 396) + 16));
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)(*(_DWORD *)(tess_shader + 396) + 20));
  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  v7[0] = *(_DWORD *)(tess_shader + 396);
  v7[1] = v7[0] + 8;
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v2 = *(char **)(tess_activeStages + 4 * i);
    if ( *v2 >= 0 )
    {
      GL_State(*((_DWORD *)v2 + 417));
      RB_SetupMultitexture(v2, (int)v7, 32);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_Cull(*(_DWORD *)(tr_showTrisShader + 168));
    v4 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v4[417]);
    RB_SetupMultitexture(v4, (int)v7, 32);
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( r_showtris->integer < 5 )
    {
      qglColor3f(0.0f, tr_identityLight, 0.0f);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
}

/* ---- RB_EndSurface_OptimizedARB  0x00501180 ----  VERIFIED */
void __cdecl RB_EndSurface_OptimizedARB( void )
{
  char *v0;
  int v2;
  char *v3;
  int integer;
  _DWORD *v5;
  GLubyte v[4]; // [esp+54h] [ebp-28h] BYREF
  _DWORD v8[9]; // [esp+58h] [ebp-24h] BYREF

  if ( r_logFile->integer )
  {
    v0 = va("--- RB_SurfaceOptimizedARB( %s ) ---\n", (const char *)tess_shader);
    if ( Stream )
      fprintf(Stream, "%s", (int)v0);
  }
  if ( r_lightmap->integer )
  {
    if ( glState_texEnv[glState_currentTmu] != 7681 )
    {
      glState_texEnv[glState_currentTmu] = 7681;
      glTexEnvi(0x2300u, 0x2200u, 7681);
    }
  }
  else
  {
    GL_TexEnv(*(_DWORD *)(*(_DWORD *)(tess_shader + 340) + 348));
  }
  qglBindBufferARB(34962, *(_DWORD *)(tess_shader + 396));
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  glColorPointer(4, 0x1401u, 32, (const GLvoid *)0x10);
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)0x14);
  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  v2 = 0;
  v8[0] = 0;
  for ( v8[1] = 8; v2 < tess_activeStageCount; ++v2 )
  {
    v3 = *(char **)(tess_activeStages + 4 * v2);
    if ( *v3 >= 0 )
    {
      GL_State(*((_DWORD *)v3 + 417));
      RB_SetupMultitexture(v3, (int)v8, 32);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_Cull(*(_DWORD *)(tr_showTrisShader + 168));
    v5 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v5[417]);
    RB_SetupMultitexture(v5, (int)v8, 32);
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( r_showtris->integer < 5 )
    {
      qglColor3f(0.0f, tr_identityLight, 0.0f);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
  qglBindBufferARB(34962, 0);
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
}

/* ---- RB_EndSurface_OptimizedATI  0x00501500 ----  VERIFIED */
void __cdecl RB_EndSurface_OptimizedATI( void )
{
  int v0;
  int v1;
  int i;
  char *v4;
  int integer;
  _DWORD *v6;
  GLubyte v[4]; // [esp+5Ch] [ebp-28h] BYREF
  _DWORD v9[9]; // [esp+60h] [ebp-24h] BYREF

  v0 = *(_DWORD *)(tess_shader + 396);
  v1 = *(_DWORD *)(tess_shader + 400);
  if ( r_lightmap->integer )
  {
    if ( glState_texEnv[glState_currentTmu] != 7681 )
    {
      glState_texEnv[glState_currentTmu] = 7681;
      glTexEnvi(0x2300u, 0x2200u, 7681);
    }
  }
  else
  {
    GL_TexEnv(*(_DWORD *)(*(_DWORD *)(tess_shader + 340) + 348));
  }
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  qglArrayObjectATI(32886, 4, 5121, 32, v0, v1 + 16);
  qglArrayObjectATI(32884, 3, 5126, 32, v0, v1 + 20);
  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  v9[0] = v1;
  v9[1] = v1 + 8;
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v4 = *(char **)(tess_activeStages + 4 * i);
    if ( *v4 >= 0 )
    {
      GL_State(*((_DWORD *)v4 + 417));
      RB_SetupMultitextureATI(v4, v0, (int)v9, 32);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_Cull(*(_DWORD *)(tr_showTrisShader + 168));
    v6 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v6[417]);
    RB_SetupMultitextureATI(v6, v0, (int)v9, 32);
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( r_showtris->integer < 5 )
    {
      qglColor3f(0.0f, tr_identityLight, 0.0f);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
}

/* ---- RB_EndSurface_OptimizedNV  0x00501830 ----  VERIFIED */
void __cdecl RB_EndSurface_OptimizedNV( void )
{
  char *v0;
  int i;
  char *v3;
  int integer;
  _DWORD *v5;
  GLubyte v[4]; // [esp+4Ch] [ebp-28h] BYREF
  _DWORD v8[9]; // [esp+50h] [ebp-24h] BYREF

  if ( r_logFile->integer )
  {
    v0 = va("--- RB_SurfaceOptimizedNV( %s ) ---\n", (const char *)tess_shader);
    if ( Stream )
      fprintf(Stream, "%s", (int)v0);
  }
  if ( r_lightmap->integer )
  {
    if ( glState_texEnv[glState_currentTmu] != 7681 )
    {
      glState_texEnv[glState_currentTmu] = 7681;
      glTexEnvi(0x2300u, 0x2200u, 7681);
    }
  }
  else
  {
    GL_TexEnv(*(_DWORD *)(*(_DWORD *)(tess_shader + 340) + 348));
  }
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  glColorPointer(4, 0x1401u, 32, (const GLvoid *)(*(_DWORD *)(tess_shader + 396) + 16));
  glVertexPointer(3, 0x1406u, 32, (const GLvoid *)(*(_DWORD *)(tess_shader + 396) + 20));
  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  v8[0] = *(_DWORD *)(tess_shader + 396);
  v8[1] = v8[0] + 8;
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v3 = *(char **)(tess_activeStages + 4 * i);
    if ( *v3 >= 0 )
    {
      GL_State(*((_DWORD *)v3 + 417));
      RB_SetupMultitexture(v3, (int)v8, 32);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_Cull(*(_DWORD *)(tr_showTrisShader + 168));
    v5 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v5[417]);
    RB_SetupMultitexture(v5, (int)v8, 32);
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( r_showtris->integer < 5 )
    {
      qglColor3f(0.0f, tr_identityLight, 0.0f);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
}

/* ---- RB_EndSurface_CachedStaticModelGeneric  0x00501BB0 ----  VERIFIED */
void __cdecl RB_EndSurface_CachedStaticModelGeneric( void )
{
  int i;
  char *v2;
  int integer;
  _DWORD *v4;
  float v6;
  GLubyte v[4]; // [esp+64h] [ebp-28h] BYREF
  _DWORD v8[9]; // [esp+68h] [ebp-24h] BYREF

  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  glColorPointer(4, 0x1401u, 24, (const GLvoid *)(tr_cachedStaticModelStorage + 8));
  glVertexPointer(3, 0x1406u, 24, (const GLvoid *)(tr_cachedStaticModelStorage + 12));
  v8[0] = tr_cachedStaticModelStorage;
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v2 = *(char **)(tess_activeStages + 4 * i);
    if ( *v2 >= 0 )
    {
      GL_State(*((_DWORD *)v2 + 417));
      RB_SetupMultitexture(v2, (int)v8, 24);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_Cull(*(_DWORD *)(tr_showTrisShader + 168));
    v4 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v4[417]);
    RB_SetupMultitexture(v4, (int)v8, 24);
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( r_showtris->integer < 5 )
    {
      v6 = tr_identityLight * 0.5;
      qglColor3f(v6, tr_identityLight, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
}

/* ---- RB_EndSurface_CachedStaticModelARB  0x00501E90 ----  VERIFIED */
void __cdecl RB_EndSurface_CachedStaticModelARB( void )
{
  int v1;
  char *v2;
  int integer;
  _DWORD *v4;
  float v6;
  GLubyte v[4]; // [esp+6Ch] [ebp-28h] BYREF
  _DWORD v8[9]; // [esp+70h] [ebp-24h] BYREF

  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  qglBindBufferARB(34962, tr_cachedStaticModelStorage);
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  glColorPointer(4, 0x1401u, 24, (const GLvoid *)8);
  glVertexPointer(3, 0x1406u, 24, (const GLvoid *)0xC);
  v1 = 0;
  for ( v8[0] = 0; v1 < tess_activeStageCount; ++v1 )
  {
    v2 = *(char **)(tess_activeStages + 4 * v1);
    if ( *v2 >= 0 )
    {
      GL_State(*((_DWORD *)v2 + 417));
      RB_SetupMultitexture(v2, (int)v8, 24);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_Cull(*(_DWORD *)(tr_showTrisShader + 168));
    v4 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v4[417]);
    RB_SetupMultitexture(v4, (int)v8, 24);
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( r_showtris->integer < 5 )
    {
      v6 = tr_identityLight * 0.5;
      qglColor3f(v6, tr_identityLight, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
  qglBindBufferARB(34962, 0);
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
}

/* ---- RB_EndSurface_CachedStaticModelATI  0x00502170 ----  VERIFIED */
void __cdecl RB_EndSurface_CachedStaticModelATI( void )
{
  int v0;
  int v1;
  int i;
  char *v4;
  int integer;
  _DWORD *v6;
  float v7;
  GLubyte v[4]; // [esp+70h] [ebp-28h] BYREF
  _DWORD v9[9]; // [esp+74h] [ebp-24h] BYREF

  v0 = tr_cachedStaticModelStorage;
  v1 = tr_cachedStaticModelStorageOffset;
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  qglArrayObjectATI(32886, 4, 5121, 24, v0, v1 + 8);
  qglArrayObjectATI(32884, 3, 5126, 24, v0, v1 + 12);
  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  v9[0] = v1;
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v4 = *(char **)(tess_activeStages + 4 * i);
    if ( *v4 >= 0 )
    {
      GL_State(*((_DWORD *)v4 + 417));
      RB_SetupMultitextureATI(v4, v0, (int)v9, 24);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    v6 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v6[417]);
    RB_SetupMultitextureATI(v6, v0, (int)v9, 24);
    if ( r_showtris->integer < 5 )
    {
      v7 = tr_identityLight * 0.5;
      qglColor3f(v7, tr_identityLight, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
}

/* ---- RB_EndSurface_CachedStaticModelNV  0x00502410 ----  VERIFIED */
void __cdecl RB_EndSurface_CachedStaticModelNV( void )
{
  int i;
  char *v2;
  int integer;
  _DWORD *v4;
  float v6;
  GLubyte v[4]; // [esp+64h] [ebp-28h] BYREF
  _DWORD v8[9]; // [esp+68h] [ebp-24h] BYREF

  RB_SetIteratorFog();   /* takes NO arguments -- 0x00500810 */
  GL_Cull(*(_DWORD *)(tess_shader + 168));
  if ( (glState_clientStateBits & 0x100) == 0 )
  {
    glEnableClientState(0x8076u);
    glState_clientStateBits |= 0x100u;
  }
  if ( (glState_clientStateBits & 0x200) != 0 )
  {
    glDisableClientState(0x8075u);
    glState_clientStateBits &= ~0x200u;
  }
  glColorPointer(4, 0x1401u, 24, (const GLvoid *)(tr_cachedStaticModelStorage + 8));
  glVertexPointer(3, 0x1406u, 24, (const GLvoid *)(tr_cachedStaticModelStorage + 12));
  v8[0] = tr_cachedStaticModelStorage;
  for ( i = 0; i < tess_activeStageCount; ++i )
  {
    v2 = *(char **)(tess_activeStages + 4 * i);
    if ( *v2 >= 0 )
    {
      GL_State(*((_DWORD *)v2 + 417));
      RB_SetupMultitexture(v2, (int)v8, 24);
      if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
      }
      else
      {
        backEnd_pc_drawnIndexCount += count;
        ++backEnd_pc_drawCallCount;
        glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
      }
    }
  }
  integer = r_showtris->integer;
  if ( integer )
  {
    if ( (integer & 1) != 0 )
      glDepthRange(0.0, 0.1000000014901161);
    GL_Cull(*(_DWORD *)(tr_showTrisShader + 168));
    v4 = *(_DWORD **)(tr_showTrisShader + 340);
    GL_State(v4[417]);
    RB_SetupMultitexture(v4, (int)v8, 24);
    if ( (glState_clientStateBits & 0x100) != 0 )
    {
      glDisableClientState(0x8076u);
      glState_clientStateBits &= ~0x100u;
    }
    if ( r_showtris->integer < 5 )
    {
      v6 = tr_identityLight * 0.5;
      qglColor3f(v6, tr_identityLight, tr_identityLight);
    }
    else
    {
      RB_ChooseSurfaceCountColor(count, v);
      glColor4ubv(v);
    }
    if ( qglDrawRangeElementsEXT && tess_optimizedVertexEnd )
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      qglDrawRangeElementsEXT(4, tess_optimizedFirstVertex, tess_optimizedVertexEnd, count, 5123, &tess_optimizedIndexes);
    }
    else
    {
      backEnd_pc_drawnIndexCount += count;
      ++backEnd_pc_drawCallCount;
      glDrawElements(4u, count, 0x1403u, &tess_optimizedIndexes);
    }
    glDepthRange(0.0, 1.0);
  }
  if ( Stream )
    fprintf(Stream, "%s", (int)"----------\n");
}

/* ---- RB_EndSurface_Optimized  0x005026F0 ----  VERIFIED */
void __cdecl RB_EndSurface_Optimized( void )
{
  switch ( *(int *)( tess_shader + 392 ) )
  {
    case 0xC:  RB_EndSurface_CachedStaticModelGeneric(); break;
    case 0xD:  RB_EndSurface_CachedStaticModelARB();     break;
    case 0xE:  RB_EndSurface_CachedStaticModelATI();     break;
    case 0xF:  RB_EndSurface_CachedStaticModelNV();      break;
    case 0x19: RB_EndSurface_OptimizedGeneric();         break;
    case 0x1A: RB_EndSurface_OptimizedARB();             break;
    case 0x1B: RB_EndSurface_OptimizedATI();             break;
    case 0x1C: RB_EndSurface_OptimizedNV();              break;
    default:   break;
  }
  backEnd_pc_indexCount += count;
  backEnd_pc_vertexCount += tess_renderedVertexCount;
  count         = 0;
  tess_renderedVertexCount = 0;
}
#if 0
int __cdecl RB_EndSurface_Optimized(void *this)
{
  int result;

  switch ( *(_DWORD *)(tess_shader + 392) )
  {
    case 0xC:
      RB_EndSurface_CachedStaticModelGeneric(this);
      break;
    case 0xD:
      RB_EndSurface_CachedStaticModelARB(this);
      break;
    case 0xE:
      RB_EndSurface_CachedStaticModelATI();
      break;
    case 0xF:
      RB_EndSurface_CachedStaticModelNV(this);
      break;
    case 0x19:
      RB_EndSurface_OptimizedGeneric();
      break;
    case 0x1A:
      RB_EndSurface_OptimizedARB();
      break;
    case 0x1B:
      RB_EndSurface_OptimizedATI();
      break;
    case 0x1C:
      RB_EndSurface_OptimizedNV();
      break;
    default:
      break;
  }
  backEnd_pc_indexCount += count;
  result = 0;
  backEnd_pc_vertexCount += tess_renderedVertexCount;
  count = 0;
  tess_renderedVertexCount = 0;
  return result;
}
#endif

/* ---- RB_EndSurface  0x005027C0 ----  VERIFIED */
void __cdecl RB_EndSurface( void )
{
  int   v1;
  char *string;

  v1 = 0;
  if ( count )
  {
    RB_EndSurface_Optimized();
    v1 = 1;
  }

  if ( !tess_numIndexes )
    return;

  if ( word_17BFF5E )
    ri_Error( 1, "\x15" "RB_EndSurface() - SHADER_MAX_INDEXES hit" );
  if ( *(float *)&tess_xyz[0x1FFF * tess_vertexComponentCount] != 0.0 )
    ri_Error( 1, "\x15" "RB_EndSurface() - SHADER_MAX_VERTEXES hit" );

  if ( tess_shader == tr_stencilShadowShader )
  {
    RB_ShadowTessEnd();
    tess_numIndexes = 0;
    return;
  }

  if ( r_debugSort->integer
    && (double)r_debugSort->integer < *(float *)(tess_shader + 88) )
  {
    tess_numIndexes = 0;
    return;
  }

  if ( dword_16DCA54 )
  {
    if ( (backEnd_refdef_rdflags & 8) != 0 )
    {
      if ( !dword_16DCA28 && (char *)tess_stageIterator != (char *)RB_StageIteratorSky )
      {
        tess_numIndexes = 0;
        return;
      }
    }
    else if ( (char *)tess_stageIterator == (char *)RB_StageIteratorSky )
    {
      tess_numIndexes = 0;
      return;
    }
  }

  ++backEnd_pc_shaderCount;
  if ( tess_activeStageCount )
  {
    backEnd_pc_vertexCount += tess_numVertexes;
    backEnd_pc_indexCount += tess_numIndexes;
  }

  tess_stageIterator( v1 );

  if ( r_showtris->integer )
  {
    if ( tess_numIndexes )
      DrawTris( (int)tess_indexes );
  }

  string = r_shownormals->string;
  if ( *string )
  {
    if ( !_stricmp( string, "white" ) )
    {
      DrawNormals( tess_indexes );
    }
    else if ( !_stricmp( r_shownormals->string, "color" ) )
    {
      DrawColoredNormals( (int)tess_indexes );
    }
    else
    {
      Cvar_Set2( "r_shownormals", &empty_string, qtrue );
      Com_Printf( "Proper Usage: r_shownormals white | color\n" );
    }
  }

  tess_numIndexes = 0;
  if ( Stream )
    fprintf( Stream, "%s", (int)"----------\n" );
}
