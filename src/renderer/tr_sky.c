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

extern int GL_Bind();
extern int GL_ClientState();
extern int GL_State();
extern int Hunk_AllocAlignInternal();
void __cdecl RB_BeginSurface( void *shader, int vertexComponentCount );
extern int RB_EndMultitexture();
void __cdecl RB_EndSurface( void );
extern int RB_SelectStorageATI();
extern int RB_SelectStorageNV();
extern void __cdecl RB_UpdateSunFlare( int a1, int a2, int a3, float a4, int a5, int a6 );
extern int R_CreateBufferARB();
extern int R_FindShader();
extern int R_FogOn();
 double __cdecl R_UpdateOverTime( float current, float target,
                                  int fadeInRate, int fadeOutRate,
                                  int elapsedMsec );

static const char * const r_sunCvarNames[20] =
{
	"r_sunsprite_shader",       /* 0x005713C8 */
	"r_sunsprite_size",
	"r_sunflare_shader",
	"r_sunflare_min_size",
	"r_sunflare_min_angle",
	"r_sunflare_max_size",
	"r_sunflare_max_angle",
	"r_sunflare_max_alpha",
	"r_sunflare_fadein",
	"r_sunflare_fadeout",
	"r_sunblind_min_angle",
	"r_sunblind_max_angle",
	"r_sunblind_max_darken",
	"r_sunblind_fadein",
	"r_sunblind_fadeout",
	"r_sunglare_min_angle",
	"r_sunglare_max_angle",
	"r_sunglare_max_lighten",
	"r_sunglare_fadein",
	"r_sunglare_fadeout"
};

/* 0x00571418.  "degres" is retail's spelling in six of these; kept. */
static const char * const r_sunCvarHelp[20] =
{
	"name for static sprite; can be any shader",
	"diameter in pixels at 640x480 and 80 fov",
	"name for flare effect; can be any shader",
	"smallest size of flare effect in pixels at 640x480",
	"angle from sun in degrees outside which effect is 0",
	"largest size of flare effect in pixels at 640x480",
	"angle from sun in degrees inside which effect is max",
	"0-1 vertex color and alpha of sun at max effect",
	"time in seconds to fade alpha from 0% to 100%",
	"time in seconds to fade alpha from 100% to 0%",
	"angle from sun in degres outside which blinding is 0",
	"angle from sun in degres inside which blinding is max",
	"0-1 fraction for how black the world is at max blind",
	"time in seconds to fade blind from 0% to 100%",
	"time in seconds to fade blind from 100% to 0%",
	"angle from sun in degres outside which glare is 0",
	"angle from sun in degres inside which glare is max",
	"0-1 fraction for how white the world is at max glare",
	"time in seconds to fade glare from 0% to 100%",
	"time in seconds to fade glare from 100% to 0%"
};


/* 0x005714B0.  AddSkyPolygon's world-vector -> face-ST axis map. */
static const int vec_to_st[6][3] =
{
	{ -2,  3,  1 },
	{  2,  3, -1 },
	{  1,  3,  2 },
	{ -1,  3, -2 },
	{ -2, -1,  3 },
	{ -2,  1, -3 }
};

/* 0x005714F8.  MakeSkyVec's face-ST -> world-vector axis map. */
static const int st_to_vec[6][3] =
{
	{  3, -1,  2 },
	{ -3,  1,  2 },
	{  1,  3,  2 },
	{ -1, -3,  2 },
	{ -2, -1,  3 },
	{  2, -1, -3 }
};

static const int sky_texorder[6] = { 0, 2, 1, 3, 4, 5 };

/* ---- AddSkyPolygon  0x004E61D0 ----  VERIFIED */
void __cdecl AddSkyPolygon(float *a1, int a2)
{
  double v2;
  long double v3;
  float *v4;
  int v5;
  int v6;
  int v7;
  double v8;
  int v9;
  int v10;
  int v11;
  double v12;
  int v13;
  double v14;
  int v15;
  double v16;
  float v17;
  float v18;
  float v19;
  float v20;
  float v21;
  float v22;
  float v23;
  float v24;
  float v25;
  float v26;
  float v27;

  v2 = 0.0;
  v3 = 0.0;
  v4 = a1;
  v20 = 0.0;
  v5 = a2;
  v6 = 0;
  v22 = 0.0;
  if ( a2 >= 4 )
  {
    v7 = 3;
    do
    {
      v7 += 4;
      v8 = v2 + *a1;
      v6 += 4;
      a1 += 12;
      v23 = v22 + *(a1 - 10);
      v18 = v8 + *(a1 - 9);
      v19 = v18 + *(a1 - 6);
      v21 = v20 + *(a1 - 11) + *(a1 - 8) + *(a1 - 5);
      v24 = v23 + *(a1 - 7) + *(a1 - 4);
      v2 = v19 + *(a1 - 3);
      v3 = v21 + *(a1 - 2);
      v20 = v3;
      v22 = v24 + *(a1 - 1);
    }
    while ( v7 < v5 );
  }
  if ( v6 < v5 )
  {
    v9 = v5 - v6;
    do
    {
      a1 += 3;
      --v9;
      v2 = v2 + *(a1 - 3);
      v3 = v3 + *(a1 - 2);
      v22 = v22 + *(a1 - 1);
    }
    while ( v9 );
    v20 = v3;
  }
  v25 = fabs(v2);
  v26 = fabs(v3);
  v27 = fabs(v22);
  if ( v25 <= (double)v26 || v25 <= (double)v27 )
  {
    if ( v26 <= (double)v27 || v26 <= (double)v25 )
    {
      v10 = 5;
      if ( !((v22 < 0.0) | __UNORDERED__(v22, 0.0)) )
        v10 = 4;
    }
    else if ( (v20 < 0.0) | __UNORDERED__(v20, 0.0) )
    {
      v10 = 3;
    }
    else
    {
      v10 = 2;
    }
  }
  else
  {
    v10 = ((v2 < 0.0) | __UNORDERED__(v2, 0.0)) != 0;
  }
  if ( v5 > 0 )
  {
    v11 = vec_to_st[v10][2];
    do
    {
      if ( v11 <= 0 )
        v12 = -v4[-v11 - 1];
      else
        v12 = v4[v11 - 1];
      if ( !((v12 < 0.001) | __UNORDERED__(v12, 0.001)) )
      {
        v13 = vec_to_st[v10][0];
        if ( v13 >= 0 )
          v14 = v4[v13 - 1] / v12;
        else
          v14 = -1.0 / v12 * v4[-v13 - 1];
        v15 = vec_to_st[v10][1];
        v17 = v14;
        if ( v15 >= 0 )
          v16 = v4[v15 - 1] / v12;
        else
          v16 = -1.0 / v12 * v4[-v15 - 1];
        if ( (v17 < (double)sky_mins[v10]) | __UNORDERED__(v17, sky_mins[v10]) )
          sky_mins[v10] = v17;
        if ( (v16 < flt_11DAE20[v10]) | __UNORDERED__(v16, flt_11DAE20[v10]) )
          flt_11DAE20[v10] = v16;
        if ( v17 > (double)sky_maxs[v10] )
          sky_maxs[v10] = v17;
        if ( v16 > flt_11DAE50[v10] )
          flt_11DAE50[v10] = v16;
      }
      v4 += 3;
      --v5;
    }
    while ( v5 );
  }
}

static const float sky_clip[6][3] = {
	{  1.0f,  1.0f,  0.0f },
	{  1.0f, -1.0f,  0.0f },
	{  0.0f, -1.0f,  1.0f },
	{  0.0f,  1.0f,  1.0f },
	{  1.0f,  0.0f,  1.0f },
	{ -1.0f,  0.0f,  1.0f }
};

/* ---- ClipSkyPolygon  0x004E6450 ----  VERIFIED */
void __cdecl ClipSkyPolygon(int a1, int a2, int a3)
{
  int v3;
  int v4;
  int v5;
  float *v6;
  float *v7;
  double v8;
  double v9;
  _DWORD *v10;
  int v11;
  int v12;
  _DWORD *v13;
  char *v14;
  _DWORD *v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  double v21;
  double v22;
  int v23;
  double v24;
  double v25;
  int v26;
  int v27;
  float *v28;
  float *v29;
  float *v30;
  int v31;
  _DWORD v32[64];
  float v33[64];
  float v34[64][3]; // [esp+220h] [ebp-600h] BYREF
  _DWORD v37[192]; // [esp+520h] [ebp-300h] BYREF -- the newv[1] half

  if ( a1 > 62 )
    ri_Error(1, "\x15" "ClipSkyPolygon: MAX_CLIP_VERTS");
  v3 = a3;
  if ( a3 == 6 )
  {
    AddSkyPolygon((float *)a2, a1);
    return;
  }
  v4 = 0;
  v5 = 0;
  v6 = (float *)sky_clip[a3];   /* retail 0x00571468 */
  v26 = 0;
  if ( a1 > 0 )
  {
    v7 = (float *)(a2 + 4);
    while ( 1 )
    {
      v8 = v7[1] * v6[2] + *(v7 - 1) * *v6 + v6[1] * *v7;
      if ( v8 <= 0.1 )
      {
        if ( (v8 < -0.1) | __UNORDERED__(v8, -0.1) )
        {
          v26 = 1;
          v32[v4] = 1;
        }
        else
        {
          v32[v4] = 2;
        }
      }
      else
      {
        v5 = 1;
        v32[v4] = 0;
      }
      v33[v4++] = v8;
      v7 += 3;
      if ( v4 >= a1 )
        break;
      v6 = (float *)sky_clip[a3];   /* retail 0x00571468 */
    }
    if ( v5 && v26 )
    {
      v9 = v33[0];
      v32[v4] = v32[0];
      v33[v4] = v9;
      v10 = (_DWORD *)(a2 + 12 * v4);
      *v10 = *(_DWORD *)a2;
      v10[1] = *(_DWORD *)(a2 + 4);
      v10[2] = *(_DWORD *)(a2 + 8);
      v28 = &v34[0][2];
      v11 = 0;
      v29 = &v34[0][1];
      v31 = 0;
      v27 = 0;
      v12 = a2 + 4;
      v13 = (_DWORD *)&v34[0][2];
      v14 = (char *)&v34[0][1];
      v15 = (_DWORD *)&v34[0][0];
      v30 = &v34[0][0];
      while ( 1 )
      {
        v16 = v32[v27];
        if ( !v16 )
        {
          *v15 = *(_DWORD *)(v12 - 4);
          *(_DWORD *)v14 = *(_DWORD *)v12;
          *v13 = *(_DWORD *)(v12 + 4);
          ++v31;
          v30 += 3;
          v29 += 3;
          v15 += 3;
          v14 += 12;
          v13 += 3;
          v28 += 3;
          goto LABEL_23;
        }
        v17 = v16 - 1;
        if ( !v17 )
          goto LABEL_21;
        if ( v17 == 1 )
        {
          *v15 = *(_DWORD *)(v12 - 4);
          *(_DWORD *)v14 = *(_DWORD *)v12;
          *v13 = *(_DWORD *)(v12 + 4);
          ++v31;
          v30 += 3;
          v29 += 3;
          v15 += 3;
          v14 += 12;
          v13 += 3;
          v28 += 3;
LABEL_21:
          v18 = 3 * v11;
          v37[v18] = *(_DWORD *)(v12 - 4);
          v37[v18 + 1] = *(_DWORD *)v12;
          v37[v18 + 2] = *(_DWORD *)(v12 + 4);
          ++v11;
        }
LABEL_23:
        v19 = v32[v27];
        if ( v19 != 2 )
        {
          v20 = v32[v27 + 1];
          if ( v20 != 2 && v20 != v19 )
          {
            v15 += 3;
            v14 += 12;
            v21 = v33[v27] / (v33[v27] - v33[v27 + 1]);
            v13 += 3;
            v22 = (*(float *)(v12 + 8) - *(float *)(v12 - 4)) * v21 + *(float *)(v12 - 4);
            *v30 = v22;
            v23 = 3 * v11;
            *(float *)&v37[v23] = v22;
            v24 = (*(float *)(v12 + 12) - *(float *)v12) * v21 + *(float *)v12;
            *v29 = v24;
            *(float *)&v37[v23 + 1] = v24;
            v28 += 3;
            v25 = v21 * (*(float *)(v12 + 16) - *(float *)(v12 + 4)) + *(float *)(v12 + 4);
            *(v28 - 3) = v25;
            *(float *)&v37[v23 + 2] = v25;
            ++v31;
            v30 += 3;
            v29 += 3;
            ++v11;
          }
        }
        v12 += 12;
        if ( ++v27 >= a1 )
        {
          ClipSkyPolygon(v31, (int)v34, a3 + 1);
          ClipSkyPolygon(v11, (int)v37, a3 + 1);
          return;
        }
      }
    }
    v3 = a3;
  }
  ClipSkyPolygon(a1, a2, v3 + 1);
}

/* ---- ClearSkyBox  0x004E67A0 ----  [HIGH] */
int ClearSkyBox()
{
  int result;

  for ( result = 0; result < 6; ++result )
  {
    flt_11DAE20[result] = 9999.0;
    sky_mins[result] = 9999.0;
    flt_11DAE50[result] = -9999.0;
    sky_maxs[result] = -9999.0;
  }
  return result * 4;
}

/* ---- RB_ClipSkyPolygons  0x004E67E0 ----  VERIFIED */
int __cdecl RB_ClipSkyPolygons(int a1)
{
  int result;
  int i;
  int v3;
  int v4;
  double v5;
  int v6;
  double v7;
  int v8;
  float v9[15]; // [esp+4h] [ebp-3Ch] BYREF -- Q3 vec3_t p[5]

  ClearSkyBox();
  result = tess_numIndexes;
  for ( i = 0; i < result; i += 3 )
  {
    v3 = tess_vertexComponentCount;
    v4 = v3 * (unsigned __int16)tess_indexes[i];
    v9[0] = tess_xyz[v4] - backEnd_viewParms_originX[0];
    v9[1] = tess_xyz[v4 + 1] - backEnd_viewParms_originY;
    v5 = tess_xyz[v4 + 2];
    v6 = v3 * (unsigned __int16)tess_indexes[i + 1];
    v9[2] = v5 - backEnd_viewParms_originZ;
    v9[3] = tess_xyz[v6] - backEnd_viewParms_originX[0];
    v9[4] = tess_xyz[v6 + 1] - backEnd_viewParms_originY;
    v7 = tess_xyz[v6 + 2];
    v8 = v3 * (unsigned __int16)tess_indexes[i + 2];
    v9[5] = v7 - backEnd_viewParms_originZ;
    v9[6] = tess_xyz[v8] - backEnd_viewParms_originX[0];
    v9[7] = tess_xyz[v8 + 1] - backEnd_viewParms_originY;
    v9[8] = tess_xyz[v8 + 2] - backEnd_viewParms_originZ;
    ClipSkyPolygon(3, (int)v9, 0);
    result = tess_numIndexes;
  }
  return result;
}

/* ---- MakeSkyVec  0x004E68E0 ----  [HIGH] */
void __cdecl MakeSkyVec(int a1, float *a2, float *a3, float a4, float a5)
{
  int v5;
  int v6;
  int v7;
  int v8;
  double v9;
  _DWORD v11[3];
  float v12;

  *(float *)v11 = a4;
  v5 = a1;
  *(float *)&v11[1] = a5;
  v6 = st_to_vec[v5][0];
  v11[2] = 1065353216;
  if ( v6 >= 0 )
    *(_DWORD *)a2 = v11[v6 - 1];
  else
    *a2 = -*(float *)&v11[-v6 - 1];
  v7 = st_to_vec[v5][1];
  if ( v7 >= 0 )
    *((_DWORD *)a2 + 1) = v11[v7 - 1];
  else
    a2[1] = -*(float *)&v11[-v7 - 1];
  v8 = st_to_vec[v5][2];
  if ( v8 >= 0 )
    *((_DWORD *)a2 + 2) = v11[v8 - 1];
  else
    a2[2] = -*(float *)&v11[-v8 - 1];
  a2[3] = 0.0;
  v9 = (a4 + 1.0) * 0.5;
  v12 = (a5 + 1.0) * 0.5;
  if ( (v9 < sky_min) | __UNORDERED__(v9, sky_min) )
  {
    v9 = sky_min;
  }
  else if ( v9 > sky_max )
  {
    v9 = sky_max;
  }
  if ( (v12 < (double)sky_min) | __UNORDERED__(v12, sky_min) )
  {
    v12 = sky_min;
  }
  else if ( v12 > (double)sky_max )
  {
    v12 = sky_max;
  }
  if ( a3 )
  {
    *a3 = v9;
    a3[1] = 1.0 - v12;
  }
}

/* ---- DrawSkyBox  0x004E6A30 ----  VERIFIED */
int __cdecl DrawSkyBox(int a1, int a2)
{
  int v2;
  int v3;
  int result;
  GLsizei v5;
  bool v6; // c0
  double v7;
  int v8;
  int v9;
  int v10;
  unsigned __int64 v11; // rax
  int v12;
  int v13;
  int v14;
  int v15;
  __int16 v16;
  int v17;
  GLenum *v18;
  int v19;
  float v20;
  int v21;
  int v22;
  float *v23;
  int v24;
  int v25;
  _WORD indices[256]; // [esp+9Ch] [ebp-204h] BYREF
  unsigned int v27;
  unsigned int retaddr;

  v27 = retaddr ^ _security_cookie;
  RB_EndMultitexture();
  v2 = *(_DWORD *)rendererSkyBox;
  if ( *(_DWORD *)rendererSkyBox != glState_currentStorageMode )
  {
    if ( glConfig_NVVertexArrayRange )
    {
      RB_SelectStorageNV(v2);
    }
    else if ( glConfig_ATIVertexArrayObject )
    {
      RB_SelectStorageATI(*(_DWORD *)rendererSkyBox);
    }
    glState_currentStorageMode = v2;
  }
  GL_State(256);
  v3 = 0;
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
  GL_ClientState(1025);
  qglColor3f(tr_identityLight, tr_identityLight, tr_identityLight);
  if ( a2 )
  {
    qglBlendFunc(770, 771);
    glEnable(0xBE2u);
    if ( glState_texEnv[glState_currentTmu] != 8448 )
    {
      glState_texEnv[glState_currentTmu] = 8448;
      glTexEnvi(0x2300u, 0x2200u, 8448);
    }
  }
  result = rendererSkyBox;
  v25 = 0;
  switch ( *(_DWORD *)(rendererSkyBox + 4) )
  {
    case 0:
      glTexCoordPointer(2, 0x1406u, 24, *(const GLvoid **)(rendererSkyBox + 8));
      glVertexPointer(4, 0x1406u, 24, (const GLvoid *)(*(_DWORD *)(rendererSkyBox + 8) + 8));
      if ( qglLockArraysEXT )
      {
        qglLockArraysEXT(0, 486);
        v25 = 1;
      }
      goto LABEL_24;
    case 1:
      qglBindBufferARB(34962, *(_DWORD *)(rendererSkyBox + 8));
      glTexCoordPointer(2, 0x1406u, 24, 0);
      glVertexPointer(4, 0x1406u, 24, (const GLvoid *)8);
      goto LABEL_24;
    case 2:
      qglArrayObjectATI(32888, 2, 5126, 24, *(_DWORD *)(rendererSkyBox + 8), *(_DWORD *)(rendererSkyBox + 12));
      qglArrayObjectATI(32884, 4, 5126, 24, *(_DWORD *)(rendererSkyBox + 8), *(_DWORD *)(rendererSkyBox + 12) + 8);
      goto LABEL_24;
    case 3:
      glTexCoordPointer(2, 0x1406u, 24, *(const GLvoid **)(rendererSkyBox + 8));
      glVertexPointer(4, 0x1406u, 24, (const GLvoid *)(*(_DWORD *)(rendererSkyBox + 8) + 8));
LABEL_24:
      v5 = 0;
      v22 = 0;
      do
      {
        sky_mins[v3] = floor(sky_mins[v3] * 4.0) * 0.25;
        v23 = &sky_maxs[v3];
        flt_11DAE20[v3] = floor(flt_11DAE20[v3] * 4.0) * 0.25;
        sky_maxs[v3] = ceil(sky_maxs[v3] * 4.0) * 0.25;
        v20 = ceil(flt_11DAE50[v3] * 4.0) * 0.25;
        v6 = sky_mins[v3] < (double)sky_maxs[v3];
        flt_11DAE50[v3] = v20;
        if ( v6 && flt_11DAE20[v3] < (double)v20 )
        {
          v7 = flt_11DAE20[v3] * 4.0;
          v8 = (unsigned __int64)(sky_mins[v3] * 4.0);
          v9 = (unsigned __int64)v7;
          v10 = (unsigned __int64)(*v23 * 4.0);
          v11 = (unsigned __int64)(v20 * 4.0);
          if ( v8 >= -4 )
          {
            if ( v8 > 4 )
              v8 = 4;
          }
          else
          {
            v8 = -4;
          }
          if ( v9 >= -4 )
          {
            if ( v9 > 4 )
              v9 = 4;
          }
          else
          {
            v9 = -4;
          }
          if ( v10 >= -4 )
          {
            if ( v10 > 4 )
              v10 = 4;
          }
          else
          {
            v10 = -4;
          }
          if ( (int)v11 >= -4 )
          {
            if ( (int)v11 > 4 )
              LODWORD(v11) = 4;
          }
          else
          {
            LODWORD(v11) = -4;
          }
          v12 = v11 + 4;
          v13 = v9 + 4;
          v24 = v12;
          if ( v13 < v12 )
          {
            v21 = v8 + 4;
            v14 = v10 + 4;
            do
            {
              v15 = v21;
              if ( v21 < v14 )
              {
                v16 = 9 * (v13 + 8 * v22 + v22);
                do
                {
                  indices[v5] = v16 + v15 + 1;
                  indices[v5 + 1] = v16 + v15;
                  indices[v5 + 2] = v16 + v15 + 9;
                  indices[v5 + 3] = v16 + v15 + 10;
                  v5 += 4;
                  ++v15;
                }
                while ( v15 < v14 );
                v12 = v24;
              }
              ++v13;
            }
            while ( v13 < v12 );
          }
          v17 = sky_texorder[v22];
          if ( a2 )
            v18 = *(GLenum **)(a1 + 4 * v17 + 124);
          else
            v18 = *(GLenum **)(a1 + 4 * v17 + 100);
          GL_Bind(v18);
          backEnd_pc_drawnIndexCount += v5;
          ++backEnd_pc_drawCallCount;
          glDrawElements(7u, v5, 0x1403u, indices);
          if ( r_showtris->integer )
          {
            glPolygonMode(0x408u, 0x1B01u);
            glDisable(0xDE1u);
            qglColor3f(1.0f, 1.0f, 1.0f);
            backEnd_pc_drawnIndexCount += v5;
            ++backEnd_pc_drawCallCount;
            glDrawElements(7u, v5, 0x1403u, indices);
            glEnable(0xDE1u);
            glPolygonMode(0x408u, 0x1B02u);
          }
          v3 = v22;
          backEnd_pc_indexCount += v5;
          backEnd_pc_vertexCount += 486;
          v5 = 0;
        }
        v22 = ++v3;
      }
      while ( v3 < 6 );
      if ( v25 )
        qglUnlockArraysEXT();
      if ( a2 )
        glDisable(0xBE2u);
      result = rendererSkyBox;
      if ( *(_DWORD *)(rendererSkyBox + 4) == 1 )
        qglBindBufferARB(34962, 0);
      break;
    default:
      return result;
  }
  return result;
}

/* ---- FillCloudySkySide  0x004E6F80 ----  VERIFIED */
int __cdecl FillCloudySkySide(int a1, _WORD *a2, _WORD *a3, int a4, int a5)
{
  int v5;
  _DWORD *v6;
  _WORD *v7;
  __int16 v8;
  int v9;
  unsigned __int16 v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int result;
  int v16;
  int v17;
  int v18;
  int v19;
  __int16 v20;
  float v21;
  int v22;
  int v23;
  float v24;
  int v25;
  unsigned __int16 v26;
  int v27;
  int v28;

  HIWORD(v5) = HIWORD(tess_numVertexes);
  v6 = a3;
  v7 = a2;
  v27 = tess_numVertexes;
  LOWORD(v5) = a2[2];
  v8 = a3[2] - v5;
  v22 = v5 + 4;
  v9 = (unsigned __int16)(v5 + 4);
  v10 = v8 + 1;
  v11 = (unsigned __int16)(*a3 - *a2) + 1;
  v26 = v10;
  if ( v9 <= *((_DWORD *)a3 + 1) + 4 )
  {
    while ( 1 )
    {
      LOWORD(a1) = *v7 + 4;
      v12 = (unsigned __int16)a1;
      if ( (unsigned __int16)a1 <= *v6 + 4 )
      {
        v13 = a4;
        v25 = 9 * (v9 + 8 * a4 + a4);
        v24 = (double)(v9 - 4) * 0.25;
        while ( 1 )
        {
          v21 = (double)(v12 - 4) * 0.25;
          MakeSkyVec(v13, (float *)(4 * tess_numVertexes * tess_vertexComponentCount + ((int)(char *)tess_xyz)), 0, v21, v24);
          v14 = 2 * (v25 + v12);
          dword_17DFF60[2 * tess_numVertexes] = s_cloudTexCoords[v14];
          dword_17DFF64[2 * tess_numVertexes++] = dword_11DAE6C[v14];
          if ( tess_numVertexes >= 0x2000 )
            ri_Error(1, "\x15" "SHADER_MAX_VERTEXES hit in FillCloudySkySide()\n");
          v6 = a3;
          v12 = (unsigned __int16)++a1;
          if ( (unsigned __int16)a1 > *(_DWORD *)a3 + 4 )
            break;
          v13 = a4;
        }
        v10 = v26;
      }
      v9 = (unsigned __int16)++v22;
      if ( (unsigned __int16)v22 > v6[1] + 4 )
        break;
      v7 = a2;
    }
  }
  result = a5;
  if ( a5 )
  {
    result = v10 - 1;
    v23 = 0;
    v28 = result;
    if ( result > 0 )
    {
      v16 = (unsigned __int16)v11 - 1;
      v17 = v27 + 1;
      do
      {
        v18 = 0;
        if ( v16 > 0 )
        {
          v19 = v17;
          do
          {
            tess_indexes[tess_numIndexes++] = v19 - 1;
            v20 = v11 + v19 - 1;
            tess_indexes[tess_numIndexes++] = v20;
            tess_indexes[tess_numIndexes++] = v19;
            tess_indexes[tess_numIndexes++] = v20;
            tess_indexes[tess_numIndexes++] = v11 + v19;
            tess_indexes[tess_numIndexes] = v19;
            ++v18;
            ++tess_numIndexes;
            ++v19;
          }
          while ( (unsigned __int16)v18 < v16 );
          result = v28;
        }
        v17 += v11;
        ++v23;
      }
      while ( (unsigned __int16)v23 < result );
    }
  }
  return result;
}

/* ---- FillCloudBox  0x004E71D0 ----  VERIFIED */
void __cdecl FillCloudBox(int a1, int a2)
{
  int i;
  double v3;
  int v4;
  double v5;
  int v6;
  double v7;
  int v8;
  double v9;
  float v10;
  float v11;
  float v12;
  float v13;
  /* FillCloudySkySide reads two contiguous 32-bit bounds per axis pair.
   * Separate decompiler locals are not guaranteed to be adjacent on the stack. */
  int maxs[2];
  int mins[2];

  for ( i = 0; i < 6; ++i )
  {
    if ( i != 5 )
    {
      sky_mins[i] = floor(sky_mins[i] * 4.0) * 0.25;
      flt_11DAE20[i] = floor(flt_11DAE20[i] * 4.0) * 0.25;
      sky_maxs[i] = ceil(sky_maxs[i] * 4.0) * 0.25;
      v3 = ceil(flt_11DAE50[i] * 4.0) * 0.25;
      flt_11DAE50[i] = v3;
      if ( sky_mins[i] < (double)sky_maxs[i] && v3 > flt_11DAE20[i] )
      {
        v10 = sky_mins[i] * 4.0;
        ftol_tempSpill = (int)v10;
        v4 = (int)v10;
        v5 = flt_11DAE20[i] * 4.0;
        mins[0] = v4;
        v11 = v5;
        ftol_tempSpill = (int)v11;
        v6 = (int)v11;
        v7 = sky_maxs[i] * 4.0;
        mins[1] = v6;
        v12 = v7;
        ftol_tempSpill = (int)v12;
        v8 = (int)v12;
        v9 = flt_11DAE50[i] * 4.0;
        maxs[0] = v8;
        v13 = v9;
        ftol_tempSpill = (int)v13;
        maxs[1] = (int)v13;
        if ( v4 >= -4 )
        {
          if ( v4 > 4 )
            mins[0] = 4;
        }
        else
        {
          mins[0] = -4;
        }
        if ( ((double)mins[1] < -4.0) | __UNORDERED__((double)mins[1], -4.0) )
        {
          mins[1] = -4;
        }
        else if ( v6 > 4 )
        {
          mins[1] = 4;
        }
        if ( v8 >= -4 )
        {
          if ( v8 > 4 )
            maxs[0] = 4;
        }
        else
        {
          maxs[0] = -4;
        }
        if ( ((double)maxs[1] < -4.0) | __UNORDERED__((double)maxs[1], -4.0) )
        {
          maxs[1] = -4;
        }
        else if ( (int)v13 > 4 )
        {
          maxs[1] = 4;
        }
        FillCloudySkySide(0, (_WORD *)mins, (_WORD *)maxs, i, a2 == 0);
      }
    }
  }
}

/* ---- RB_BuildCloudData  0x004E73F0 ----  VERIFIED */
void __cdecl RB_BuildCloudData(int a1)
{
  double v1;
  int i;

  tess_numIndexes = 0;
  tess_numVertexes = 0;
  tess_vertexComponentCount = 4;
  v1 = *(float *)(tess_shader + 96);
  sky_min = 0.00390625;
  sky_max = 0.99609375;
  if ( !((v1 == 0.0) | __UNORDERED__(v1, 0.0)) )
  {
    for ( i = 0; i < 8; ++i )
    {
      if ( !*(_DWORD *)(tess_activeStages + 4 * i) )
        break;
      FillCloudBox(tess_shader, i);
    }
  }
}

/* ---- R_BuildSkyBox  0x004E7470 ----  VERIFIED */
int __cdecl R_BuildSkyBox(const void *a1)
{
  int v2;
  int v3;
  int result;
  int *v5;
  int v6;
  _DWORD *v7;
  void *v8;
  int v9;
  int v10;
  int v11;

  if ( glConfig_ARBVertexBufferObject )
  {
    v2 = rendererSkyBox;
    v3 = storageClass;
    *(_DWORD *)rendererSkyBox = storageClass;
    *(_DWORD *)(v2 + 4) = 1;
    result = R_CreateBufferARB(v3, 34962, 11664, (int)a1, 35044);
    v5 = (int *)rendererSkyBox;
    *(_DWORD *)(rendererSkyBox + 8) = result;
    if ( !result )
    {
LABEL_3:
      *v5 = 3;
      v5[1] = 0;
      result = ri_Hunk_Alloc(11664);
      *(_DWORD *)(rendererSkyBox + 8) = result;
      qmemcpy((void *)result, a1, 0x2D90u);
    }
  }
  else if ( glConfig_NVVertexArrayRange )
  {
    v6 = rendererSkyBox;
    v7 = (_DWORD *)(rendererSkyBox + 8);
    if ( tr_staticVertexMemoryPrimaryUsed + 11680 >= tr_staticVertexMemoryPrimaryLimit )
    {
      if ( tr_staticVertexMemorySecondaryUsed + 11680 >= tr_staticVertexMemorySecondaryLimit )
      {
        v8 = Hunk_AllocAlignInternal(0x2DA0u, 32);
        v6 = rendererSkyBox;
        *v7 = v8;
        result = 3;
        *(_DWORD *)v6 = 3;
      }
      else
      {
        *v7 = tr_staticVertexMemorySecondary + tr_staticVertexMemorySecondaryUsed;
        tr_staticVertexMemorySecondaryUsed += 11680;
        result = 2;
        *(_DWORD *)v6 = 2;
      }
      *(_DWORD *)(v6 + 4) = 3;
      qmemcpy(*(void **)(v6 + 8), a1, 0x2D90u);
    }
    else
    {
      *v7 = tr_staticVertexMemoryPrimary + tr_staticVertexMemoryPrimaryUsed;
      tr_staticVertexMemoryPrimaryUsed += 11680;
      result = 1;
      *(_DWORD *)v6 = 1;
      *(_DWORD *)(v6 + 4) = 3;
      qmemcpy(*(void **)(v6 + 8), a1, 0x2D90u);
    }
  }
  else
  {
    v5 = (int *)rendererSkyBox;
    if ( !glConfig_ATIVertexArrayObject )
      goto LABEL_3;
    if ( tr_staticVertexMemoryPrimaryUsed + 11680 > tr_staticVertexMemoryPrimaryLimit )
    {
      if ( tr_staticVertexMemorySecondaryUsed + 11680 > tr_staticVertexMemorySecondaryLimit )
      {
        v9 = 0;
      }
      else
      {
        *(_DWORD *)(rendererSkyBox + 12) = tr_staticVertexMemorySecondaryUsed;
        tr_staticVertexMemorySecondaryUsed += 11680;
        v9 = 2;
      }
    }
    else
    {
      *(_DWORD *)(rendererSkyBox + 12) = tr_staticVertexMemoryPrimaryUsed;
      tr_staticVertexMemoryPrimaryUsed += 11680;
      v9 = 1;
    }
    *v5 = v9;
    v5[1] = 2;
    if ( *v5 == 1 )
    {
      *v5 = 3;
      v11 = tr_staticVertexMemoryPrimary;
      v5[2] = tr_staticVertexMemoryPrimary;
      qglUpdateObjectBufferATI(v11, v5[3], 11664, a1, 34658);
      return result;
    }
    else
    {
      if ( *v5 != 2 )
        goto LABEL_3;
      *v5 = 3;
      v10 = tr_staticVertexMemorySecondary;
      v5[2] = tr_staticVertexMemorySecondary;
      qglUpdateObjectBufferATI(v10, v5[3], 11664, a1, 34658);
      return result;
    }
  }
  return result;
}

/* ---- R_SetSkyBox  0x004E7690 ----  [HIGH] */
int __cdecl R_SetSkyBox(int result)
{
  rendererSkyBox = result;
  return result;
}

#define SKY_TEXP_BYTES  (6 * 9 * 9 * 4)

/* ---- R_InitSkyTexCoords  0x004E76A0 ----  VERIFIED */
int __cdecl R_InitSkyTexCoords(float a1)
{
  int *v1;
  float *v2;
  float *v3;
  int v4;
  double v5;
  int v6;
  int v7;
  float *v8;
  bool v9; // cc
  float v11;
  float v12;
  float v13;
  float v14;
  float v15;
  float v16;
  float v17;
  float v18;
  float *v19;
  float v20;
  float v21;
  float v22;
  int v23;
  int v24;
  float v25;
  float v26;
  float v27[3]; // [esp+40h] [ebp-2DA8h] BYREF
  float v30;
  float v31;
  float v32;
  float v33[2916]; // [esp+58h] [ebp-2D90h] BYREF

  v26 = a1 * a1;
  sky_min = 0.0;
  v23 = 0;
  backEnd_viewParms_zFar = 1090519040;
  v32 = v26 + 16777216.0;
  sky_max = 1.0;
  v1 = dword_11DAE6C;
  v2 = (float *)&s_cloudTexP;
  v3 = &v33[2];
  do
  {
    v24 = 0;
    do
    {
      v4 = 0;
      v19 = v2;
      v12 = (float)(v24 - 4);
      v30 = v12 / 4.0;
      do
      {
        v13 = (float)(v4 - 4);
        v11 = v13 / 4.0;
        MakeSkyVec(v23, v3, v3 - 2, v11, v30);
        v22 = v3[2];
        v21 = v3[1];
        v20 = *v3;
        v14 = v22 * v22;
        v25 = v21 * v21;
        v17 = v20 * v20;
        v31 = sqrt((v17 + v25 + v14) * a1 * 8192.0 + (v17 + v25) * v26 + v32 * v14);
        v5 = v17 + v25 + v14;
        v18 = (v31 + v31 - v22 * 8192.0) * (1.0 / (v5 + v5));
        *v19 = v18;
        v27[0] = v18 * v20;
        v27[1] = v18 * v21;
        v27[2] = v18 * v22;
        v27[2] = v27[2] + 4096.0;
        VectorNormalize(v27);
        v15 = acos(v27[0]);
        v6 = 1078530011;
        if ( v15 <= 3.1415927 && !((v15 < -3.1415927) | __UNORDERED__(v15, -3.1415927)) )
          v6 = LODWORD(v15);
        v16 = acos(v27[1]);
        if ( v16 <= 3.1415927 )
        {
          v7 = 1078530011;
          if ( !((v16 < -3.1415927) | __UNORDERED__(v16, -3.1415927)) )
            v7 = LODWORD(v16);
        }
        else
        {
          v7 = 1078530011;
        }
        *(v1 - 1) = v6;
        *v1 = v7;
        ++v4;
        v8 = v19 + 1;
        v3 += 6;
        v1 += 2;
        ++v19;
      }
      while ( v4 <= 8 );
      v9 = ++v24 <= 8;
      v2 = v8;
    }
    while ( v9 );
    ++v23;
  }
  while ( v8 < (float *)&s_cloudTexP[SKY_TEXP_BYTES] );
  return R_BuildSkyBox(v33);
}

/* ---- R_FindSunSpriteShader  0x004E7930 ----  VERIFIED */
char *__cdecl R_FindSunSpriteShader(char *a1)
{
  char *result;
  int v2;
  char *v3;
  int v4;

  result = R_FindShader(a1, -3, 0, (const char *)4);
  v2 = 0;
  v3 = result + 340;
  do
  {
    v4 = *(_DWORD *)v3;
    if ( !*(_DWORD *)v3 )
      break;
    ++v2;
    v3 += 4;
    *(_DWORD *)(v4 + 1668) &= ~0x100u;
  }
  while ( v2 < 8 );
  return result;
}

/* ---- R_SetSunSpriteSize  0x004E7980 ----  VERIFIED */
int __cdecl R_SetSunSpriteSize(float a1)
{
  int result;
  float v2;
  float v3[3]; // [esp+8h] [ebp-30h] BYREF -- retail vec3_t
  float v6;
  float v7;
  float v8;
  float v9;
  float v10;
  float v11;
  float v12;
  float v13;
  float v14;

  v2 = a1 * 0.001311093;
  if ( *(float *)&dword_16C57D8 * *(float *)&dword_16C57D8 <= 0.99000001 )
  {
    v12 = *(float *)&dword_16C57D4;
    v13 = -*(float *)&tr_sunDirection;
  }
  else
  {
    v12 = 1.0;
    v13 = 0.0;
  }
  v3[0] = *(float *)&dword_16C57D4 * 0.0 - *(float *)&dword_16C57D8 * v13;
  v3[1] = *(float *)&dword_16C57D8 * v12 - *(float *)&tr_sunDirection * 0.0;
  v3[2] = *(float *)&tr_sunDirection * v13 - *(float *)&dword_16C57D4 * v12;
  VectorNormalize(v3);
  v3[0] = v3[0] * v2;
  v3[1] = v3[1] * v2;
  v3[2] = v3[2] * v2;
  v6 = *(float *)&dword_16C57D8 * v3[1] - *(float *)&dword_16C57D4 * v3[2];
  v7 = *(float *)&tr_sunDirection * v3[2] - *(float *)&dword_16C57D8 * v3[0];
  v8 = *(float *)&dword_16C57D4 * v3[0] - *(float *)&tr_sunDirection * v3[1];
  v9 = v6 + v3[0];
  v10 = v7 + v3[1];
  v11 = v8 + v3[2];
  v12 = v3[0] - v6;
  v13 = v3[1] - v7;
  v14 = v3[2] - v8;
  rendererSunState_spriteVert0X = *(float *)&tr_sunDirection + v9;
  rendererSunState_spriteVert0Y = *(float *)&dword_16C57D4 + v10;
  rendererSunState_spriteVert0Z = *(float *)&dword_16C57D8 + v11;
  rendererSunState_spriteVert1X = *(float *)&tr_sunDirection + v12;
  rendererSunState_spriteVert1Y = *(float *)&dword_16C57D4 + v13;
  rendererSunState_spriteVert0W = 0;
  rendererSunState_spriteVert1W = 0;
  rendererSunState_spriteVert2W = 0;
  rendererSunState_spriteVert1Z = *(float *)&dword_16C57D8 + v14;
  rendererSunState_spriteVert3W = 0;
  rendererSunState_spriteSize = a1;
  rendererSunState_spriteVert2X = *(float *)&tr_sunDirection - v9;
  rendererSunState_spriteVert2Y = *(float *)&dword_16C57D4 - v10;
  rendererSunState_spriteVert2Z = *(float *)&dword_16C57D8 - v11;
  rendererSunState_spriteVert3X = *(float *)&tr_sunDirection - v12;
  rendererSunState_spriteVert3Y = *(float *)&dword_16C57D4 - v13;
  rendererSunState_spriteVert3Z = *(float *)&dword_16C57D8 - v14;
  result = 0;   /* retail leaves EDI (zeroed at 0x004E799C) in the return slot */
  return result;
}

/* ---- R_SetSunFromCvars  0x004E7BE0 ----  [HIGH] */
int R_SetSunFromCvars()
{
  char *string;
  char *v1;
  int result;
  float v3;
  float v4;
  float v5;
  float v6;
  float v7;
  float v8;

  rendererSunState = 0;
  string = r_sunsprite_shader->string;
  if ( *string )
    rendererSunState = (int)R_FindSunSpriteShader(string);
  R_SetSunSpriteSize(r_sunsprite_size->value);
  rendererSunState_flareShader = 0;
  v1 = r_sunflare_shader->string;
  if ( *v1 )
    rendererSunState_flareShader = (int)R_FindShader(v1, -3, 0, (const char *)4);
  rendererSunState_flareMinHalfSize = r_sunflare_min_size->value * 0.5;
  rendererSunState_flareMinCosAngle = cos(r_sunflare_min_angle->value * 3.1415927 * 0.0055555557);
  rendererSunState_flareMaxHalfSize = r_sunflare_max_size->value * 0.5;
  rendererSunState_flareMaxCosAngle = cos(r_sunflare_max_angle->value * 3.1415927 * 0.0055555557);
  rendererSunState_flareMaxAlpha = LODWORD(r_sunflare_max_alpha->value);
  v3 = r_sunflare_fadein->value * 1000.0;
  rendererSunState_flareFadeInMsec = (int)(v3 + 9.313225746154785e-10);
  v4 = r_sunflare_fadeout->value * 1000.0;
  rendererSunState_flareFadeOutMsec = (int)(v4 + 9.313225746154785e-10);
  rendererSunState_blindMinCosAngle = cos(r_sunblind_min_angle->value * 3.1415927 * 0.0055555557);
  rendererSunState_blindMaxCosAngle = cos(r_sunblind_max_angle->value * 3.1415927 * 0.0055555557);
  rendererSunState_blindMaxDarken = LODWORD(r_sunblind_max_darken->value);
  v5 = r_sunblind_fadein->value * 1000.0;
  rendererSunState_blindFadeInMsec = (int)(v5 + 9.313225746154785e-10);
  v6 = r_sunblind_fadeout->value * 1000.0;
  rendererSunState_blindFadeOutMsec = (int)(v6 + 9.313225746154785e-10);
  rendererSunState_glareMinCosAngle = cos(r_sunglare_min_angle->value * 3.1415927 * 0.0055555557);
  rendererSunState_glareMaxCosAngle = cos(r_sunglare_max_angle->value * 3.1415927 * 0.0055555557);
  rendererSunState_glareMaxLighten = LODWORD(r_sunglare_max_lighten->value);
  v7 = r_sunglare_fadein->value * 1000.0;
  result = (int)(v7 + 9.313225746154785e-10);
  rendererSunState_glareFadeInMsec = result;
  v8 = r_sunglare_fadeout->value * 1000.0;
  rendererSunState_glareFadeOutMsec = (int)(v8 + 9.313225746154785e-10);
  return result;
}

/* ---- R_SaveSunFromCvars  0x004E7EA0 ----  VERIFIED */
int __cdecl R_SaveSunFromCvars(const char *a1)
{
  int result;
  unsigned int v2; // kr00_4
  char *v3;
  char v4[65536]; // [esp+0h] [ebp-10004h] BYREF
  unsigned int v5;
  unsigned int retaddr;

  v5 = retaddr ^ _security_cookie;
  result = ri_SaveCvarsToBuffer(r_sunCvarNames, 20, v4, 0x10000);   /* 0x004E7EC9: `push offset off_5713C8`, the ADDRESS */
  if ( result )
  {
    v2 = strlen(v4);
    v3 = va("scripts/%s.sun", a1);
    return ri_FS_WriteFile(v3, v4, v2);
  }
  return result;
}

/* ---- R_LoadSunThroughCvars  0x004E7F30 ----  VERIFIED */
void __cdecl R_LoadSunThroughCvars(const char *a1)
{
  char *v1;
  int v2; // [esp+4h] [ebp-4h] BYREF

  v1 = va("scripts/%s.sun", a1);
  if ( ri_FS_ReadFile(v1, &v2) >= 0 )
  {
    if ( ri_LoadCvarsFromBuffer(r_sunCvarNames, 20, v2, v1) )
      R_SetSunFromCvars();
    ri_FS_FreeFile(v2);
  }
  else
  {
    ri_Printf(0, "^3WARNING: couldn't load sun file '%s'\n", v1);
  }
}

/* ---- R_SaveSun_f  0x004E7FA0 ----  [HIGH] */
void R_SaveSun_f()
{
  const char *v0;

  if ( ri_Cmd_Argc() == 2 )
  {
    v0 = (const char *)ri_Cmd_Argv(1);
    R_SaveSunFromCvars(v0);
  }
  else
  {
    ri_Printf(0, "USAGE: r_savesun <sunname>\n  sunname must not have an extension\n");
  }
}

/* ---- R_LoadSun_f  0x004E7FD0 ----  [HIGH] */
void R_LoadSun_f()
{
  const char *v0;

  if ( ri_Cmd_Argc() == 2 )
  {
    if ( sv_cheats->integer )
    {
      v0 = (const char *)ri_Cmd_Argv(1);
      R_LoadSunThroughCvars(v0);
    }
    else
    {
      ri_Printf(0, "You must have cheats enabled to use r_loadsun\n");
    }
  }
  else
  {
    ri_Printf(0, "USAGE: r_loadsun <sunname>\n  sunname must not have an extension\n");
  }
}

/* ---- R_SunHelp_f  0x004E8020 ----  VERIFIED */
void R_SunHelp_f()
{
	int i;

	ri_Printf(0, "\n=== SUN COMMANDS ===\n");
	ri_Printf(0, "r_loadsun <sunname> -- loads sun from 'scripts/<sunname>.sun'\n");
	ri_Printf(0, "r_savesun <sunname> -- saves sun as 'scripts/<sunname>.sun'\n");
	ri_Printf(0, "\n=== SUN CVARS ===\n");
	ri_Printf(0, "(must have r_suntest set to 1 to tweak these values)\n");
	for ( i = 0; i < 20; ++i )
	{
		if ( i > 0 )
		{
			if ( _strnicmp(r_sunCvarNames[i - 1], r_sunCvarNames[i], 8u) )
				ri_Printf(0, "\n");
		}
		ri_Printf(0, "^2%-22s^7 %s\n", r_sunCvarNames[i], r_sunCvarHelp[i]);
	}
}

/* ---- RB_DrawSunSprite  0x004E80C0 ----  VERIFIED */
void RB_DrawSunSprite()
{
  int v0;

  if ( rendererSunState )
  {
    v0 = storageClass;
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
      glState_currentStorageMode = v0;
    }
    RB_BeginSurface((void *)rendererSunState, 4);   /* 0x004E810E: mov ecx,rendererSunState / push 4 */
    tess_stageIterator = tr_stageIteratorFunc;
    qmemcpy(tess_xyz, &rendererSunState_spriteVert0X, 0x40u);
    dword_17DFF60[0] = 1065353216;
    dword_17DFF64[0] = 1065353216;
    dword_17DFF68[0] = 0;
    dword_17DFF6C[0] = 1065353216;
    dword_17DFF70[0] = 0;
    dword_17DFF74[0] = 0;
    dword_17DFF78[0] = 1065353216;
    dword_17DFF7C[0] = 0;
    tess_numVertexes = 4;
    tess_indexes[0] = 0;
    word_17A7F62[0] = 1;
    word_17A7F64[0] = 2;
    word_17A7F66[0] = 0;
    word_17A7F68[0] = 2;
    word_17A7F6A[0] = 3;
    tess_numIndexes = 6;
    RB_EndSurface();
  }
}

/* ---- RB_CalcSunFlare  0x004E81E0 ----  [HIGH] */
void __cdecl RB_CalcSunFlare(float a1)
{
  double v1;
  int v2;
  int v3;

  if ( rendererSunState_flareShader )
  {
    if ( a1 > (double)rendererSunState_flareMinCosAngle )
    {
      if ( a1 < (double)rendererSunState_flareMaxCosAngle )
        v1 = (a1 - rendererSunState_flareMinCosAngle) / (rendererSunState_flareMaxCosAngle - rendererSunState_flareMinCosAngle);
      else
        v1 = 1.0;
      *(float *)&v3 = rendererSunState_flareMaxHalfSize * v1 + rendererSunState_flareMinHalfSize;
      *(float *)&v2 = *(float *)&rendererSunState_flareMaxAlpha * v1;
      RB_UpdateSunFlare(rendererSunState_flareShader, v2, v3, rendererSunState_spriteSize, rendererSunState_flareFadeInMsec, rendererSunState_flareFadeOutMsec);
    }
  }
}

/* ---- RB_CalcSunBlind  0x004E8270 ----  [HIGH] */
void __cdecl RB_CalcSunBlind(float *a1, float *a2, float a3)
{
  int v3;
  double v4;
  double updated;
  double v6;
  double v7;
  float v8;
  float v9;
  float v10;

  if ( rendererSunState_lastUpdateTime )
    v3 = backEnd_refdef_time - rendererSunState_lastUpdateTime;
  else
    v3 = 10;
  rendererSunState_lastUpdateTime = backEnd_refdef_time;
  v10 = backEnd_viewParms_axis02 * *(float *)&dword_16C57D8
      + backEnd_viewParms_axis01 * *(float *)&dword_16C57D4
      + backEnd_viewParms_axis00 * *(float *)&tr_sunDirection;
  if ( *(float *)&rendererSunState_blindMaxDarken > 0.0 )
  {
    if ( v10 > (double)rendererSunState_blindMinCosAngle )
    {
      if ( v10 < (double)rendererSunState_blindMaxCosAngle )
        v4 = (v10 - rendererSunState_blindMinCosAngle) / (rendererSunState_blindMaxCosAngle - rendererSunState_blindMinCosAngle);
      else
        v4 = 1.0;
    }
    else
    {
      v4 = 0.0;
    }
    v8 = v4 * a3;
    updated = R_UpdateOverTime(rendererSunState_currentBlindFraction, v8, rendererSunState_blindFadeInMsec, rendererSunState_blindFadeOutMsec, v3);
    rendererSunState_currentBlindFraction = updated;
    *a2 = updated * *(float *)&rendererSunState_blindMaxDarken;
  }
  else
  {
    *a2 = 0.0;
  }
  if ( *(float *)&rendererSunState_glareMaxLighten > 0.0 )
  {
    if ( v10 > (double)rendererSunState_glareMinCosAngle )
    {
      if ( v10 < (double)rendererSunState_glareMaxCosAngle )
        v6 = (v10 - rendererSunState_glareMinCosAngle) / (rendererSunState_glareMaxCosAngle - rendererSunState_glareMinCosAngle);
      else
        v6 = 1.0;
    }
    else
    {
      v6 = 0.0;
    }
    v9 = v6 * a3;
    v7 = R_UpdateOverTime(rendererSunState_currentGlareFraction, v9, rendererSunState_glareFadeInMsec, rendererSunState_glareFadeOutMsec, v3);
    rendererSunState_currentGlareFraction = v7;
    *a1 = v7 * *(float *)&rendererSunState_glareMaxLighten;
  }
  else
  {
    *a1 = 0.0;
  }
}

/* ---- RB_AddSunEffects  0x004E83F0 ----  [HIGH] */
void RB_AddSunEffects()
{
  double v0;
  float v1;

  if ( *(_DWORD *)(tr_world + 276) )
  {
    if ( r_suntest->integer )
      R_SetSunFromCvars();
    v0 = backEnd_viewParms_axis02 * *(float *)&dword_16C57D8
       + backEnd_viewParms_axis01 * *(float *)&dword_16C57D4
       + backEnd_viewParms_axis00 * *(float *)&tr_sunDirection;
    if ( v0 > 0.0 )
    {
      RB_DrawSunSprite();
      v1 = v0;
      RB_CalcSunFlare(v1);
    }
  }
}

/* ---- RB_DrawSun  0x004E8460 ----  [HIGH] */
void RB_DrawSun()
{
  if ( r_drawSun->integer )
  {
    RB_AddSunEffects();
    RB_BeginSurface((void *)tess_shader, 3);   /* 0x004E847C: push 3 / mov ecx,esi; esi = tess_shader */
  }
}

/* ---- R_ClearSun  0x004E8490 ----  [HIGH] */
void R_ClearSun()
{
  rendererSunState_currentBlindFraction = 0.0;
  rendererSunState_currentGlareFraction = 0.0;
  rendererSunState_lastUpdateTime = 0;
}

/* ---- R_FlushSun  0x004E84B0 ----  [HIGH] */
int R_FlushSun()
{
  Com_Memset(&rendererSunState, 0, 0x9Cu);
  return 0;
}

/* ---- RB_StageIteratorSky  0x004E84D0 ----  VERIFIED */
void RB_StageIteratorSky()
{
  int v0;
  int v1;
  int v2;
  GLclampd v3;
  GLclampd v4;

  if ( r_fastsky->integer )
  {
    if ( r_drawSun->integer )
    {
      RB_AddSunEffects();
      RB_BeginSurface((void *)tess_shader, 3);   /* 0x004E84FE: push 3 / mov ecx,esi; esi = tess_shader */
    }
    return;
  }
  if ( !dword_16DCA54 || (backEnd_refdef_rdflags & 8) != 0 )
  {
    if ( backEnd_viewParms_glFogRegistered )
    {
      v0 = backEnd_viewParms_glFogDrawSky;
    }
    else
    {
      if ( glfogNum <= 0 )
      {
LABEL_11:
        if ( Stream )
          fprintf(Stream, "%s", (int)"--- RB_StageIteratorSky ---\n");
        backEnd_refdef_rdflags |= 0x40u;
        RB_ClipSkyPolygons((int)tess_indexes);
        if ( r_showsky->integer )
        {
          v4 = 0.0;
          HIDWORD(v3) = 0;
        }
        else
        {
          v4 = 1.0;
          HIDWORD(v3) = 1072693248;
        }
        LODWORD(v3) = 0;
        glDepthRange(v3, v4);
        if ( (glState_glStateBits & 0x200000) != 0 )
        {
          glDisable(0xB60u);
          glState_glStateBits &= ~0x200000u;
        }
        v1 = *(_DWORD *)(tess_shader + 100);
        if ( v1 )
        {
          if ( v1 != tr_defaultImage )
            DrawSkyBox(tess_shader, 0);
        }
        tess_numIndexes = 0;
        if ( r_drawSun->integer )
        {
          RB_AddSunEffects();
          RB_BeginSurface((void *)tess_shader, 3);   /* 0x004E860B: push 3 / mov ecx,esi; esi = ecx = tess_shader, 0x004E85E1 */
        }
        RB_BuildCloudData((int)tess_indexes);
        if ( tess_numIndexes )
          tr_stageIteratorFunc(0);
        v2 = *(_DWORD *)(tess_shader + 124);
        if ( v2 )
        {
          if ( v2 != tr_defaultImage )
            DrawSkyBox(tess_shader, 1);
        }
        R_FogOn();
        glDepthRange(0.0, 1.0);
        backEnd_refdef_rdflags &= ~0x40u;
        backEnd_skyRenderedThisView = 1;
        if ( Stream )
          fprintf(Stream, "%s", (int)"----------\n");
        return;
      }
      v0 = dword_16C4C74;
    }
    if ( !v0 )
      return;
    goto LABEL_11;
  }
}
