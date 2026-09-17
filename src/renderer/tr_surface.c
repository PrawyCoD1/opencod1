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
extern int GL_State();
void PerpendicularVector( float *dst, const float *src );
extern int RB_BeginImmediateMode();
extern int RB_glVertex3f();
extern int R_FogOn();
void RotatePointAroundVector( float *dst, const float *dir, const float *point, float degrees );

extern void MakeNormalVectors( const float *forward, float *right, float *up );

extern void RB_BeginSurface( void *shader, int vertexComponentCount );
extern void RB_EndSurface( void );

#define SHADER_MAX_VERTEXES          8192      /* cmp 2000h  @0x0050F96D */
#define SHADER_MAX_INDEXES           49152     /* cmp 0C000h @0x0050F981 */

#define tess_vertexComponentCount    tess_vertexComponentCount
#define tess_shader                  tess_shader
#define tess_entity                  tess_entity
#define tess_numIndexes              tess_numIndexes
#define tess_numVertexes             tess_numVertexes
#define backEnd_currentEntity        backEnd_currentEntity

/* ---- RB_CheckOverflow  0x0050F960 ----  VERIFIED */
void __cdecl RB_CheckOverflow( int verts, int indexes )
{
  if ( tess_numVertexes + verts < SHADER_MAX_VERTEXES
    && tess_numIndexes + indexes < SHADER_MAX_INDEXES )
    return;

  RB_EndSurface();

  if ( verts > SHADER_MAX_VERTEXES )
    ri_Error( 1, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, SHADER_MAX_VERTEXES );
  if ( indexes > SHADER_MAX_INDEXES )
    ri_Error( 1, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, SHADER_MAX_INDEXES );

  backEnd_currentEntity = tess_entity;
  RB_BeginSurface( (void *)tess_shader, tess_vertexComponentCount );
}

/* ---- RB_CheckOverflow_Optimized  0x0050F9F0 ----  VERIFIED */
void __cdecl RB_CheckOverflow_Optimized( void )
{
  RB_EndSurface();
  backEnd_currentEntity = tess_entity;
  RB_BeginSurface( (void *)tess_shader, tess_vertexComponentCount );
}

/* ---- RB_AddQuadStampFadingCornersExt  0x0050FA20 ----  VERIFIED */
int __cdecl RB_AddQuadStampFadingCornersExt( float *origin, float *left, float *up,
                                             int *color, float s1, float t1,
                                             float s2, float t2 )
{
  int    numIndexes;
  int    numVertexes;
  int    ndx;
  float *xyz;
  float *normal;
  float  nx, ny, nz;
  int    packedColor;
  int    fadedColor;

  numIndexes  = tess_numIndexes;
  numVertexes = tess_numVertexes;

  if ( numVertexes + 5 >= SHADER_MAX_VERTEXES
    || ( numIndexes + 12 >= SHADER_MAX_INDEXES
      && ( numVertexes + 5 >= SHADER_MAX_VERTEXES
        || numIndexes + 12 >= SHADER_MAX_INDEXES ) ) )
  {
    RB_EndSurface();                                     /* 0x0050FA56 */
    backEnd_currentEntity = tess_entity;                 /* 0x0050FA6D */
    RB_BeginSurface( (void *)tess_shader, tess_vertexComponentCount );
    numIndexes  = tess_numIndexes;                       /* 0x0050FA77 */
    numVertexes = tess_numVertexes;
  }

  ndx = (unsigned short)numVertexes;      /* 0x0050FA86 movzx eax, dx */

  /* Four triangles fanning around vertex 4.  0x0050FA96-0x0050FB71. */
  tess_indexes[numIndexes     ] = (unsigned short)( numVertexes     );
  tess_indexes[numIndexes +  1] = (unsigned short)( numVertexes + 1 );
  tess_indexes[numIndexes +  2] = (unsigned short)( numVertexes + 4 );
  tess_indexes[numIndexes +  3] = (unsigned short)( numVertexes + 1 );
  tess_indexes[numIndexes +  4] = (unsigned short)( numVertexes + 2 );
  tess_indexes[numIndexes +  5] = (unsigned short)( numVertexes + 4 );
  tess_indexes[numIndexes +  6] = (unsigned short)( numVertexes + 2 );
  tess_indexes[numIndexes +  7] = (unsigned short)( numVertexes + 3 );
  tess_indexes[numIndexes +  8] = (unsigned short)( numVertexes + 4 );
  tess_indexes[numIndexes +  9] = (unsigned short)( numVertexes + 3 );
  tess_indexes[numIndexes + 10] = (unsigned short)( numVertexes     );
  tess_indexes[numIndexes + 11] = (unsigned short)( numVertexes + 4 );

  /* 0x0050FA8F/0x0050FB79: base = tess.xyz + 4*(vcc * (u16)numVertexes). */
  xyz = &tess_xyz[ tess_vertexComponentCount * ndx ];

  xyz[ 0] = left[0]   + up[0]     + origin[0];
  xyz[ 1] = origin[1] + up[1]     + left[1];
  xyz[ 2] = origin[2] + up[2]     + left[2];

  xyz[ 3] = origin[0] - left[0]   + up[0];
  xyz[ 4] = origin[1] - left[1]   + up[1];
  xyz[ 5] = origin[2] - left[2]   + up[2];

  xyz[ 6] = origin[0] - left[0]   - up[0];
  xyz[ 7] = origin[1] - left[1]   - up[1];
  xyz[ 8] = origin[2] - left[2]   - up[2];

  xyz[ 9] = left[0]   + origin[0] - up[0];
  xyz[10] = origin[1] + left[1]   - up[1];
  xyz[11] = origin[2] + left[2]   - up[2];

  xyz[12] = origin[0];
  xyz[13] = origin[1];
  xyz[14] = origin[2];

  nx = 0.0f - backEnd_viewParms_axis00;                /* 0x0050FC2C */
  ny = 0.0f - backEnd_viewParms_axis01;
  nz = 0.0f - backEnd_viewParms_axis02;

  normal = &tess_stageNormals[ 3 * ndx ]; /* 0x0050FC5E/61: 12*ndx bytes */
  normal[ 0] = nx;  normal[ 1] = ny;  normal[ 2] = nz;
  normal[ 3] = nx;  normal[ 4] = ny;  normal[ 5] = nz;
  normal[ 6] = nx;  normal[ 7] = ny;  normal[ 8] = nz;
  normal[ 9] = nx;  normal[10] = ny;  normal[11] = nz;
  normal[12] = nx;  normal[13] = ny;  normal[14] = nz;

  tess_texCoords0[2 * ndx    ] = s1;   tess_texCoords1[2 * ndx    ] = s1;
  tess_texCoords0[2 * ndx + 1] = t1;   tess_texCoords1[2 * ndx + 1] = t1;
  tess_texCoords0[2 * ndx + 2] = s2;   tess_texCoords1[2 * ndx + 2] = s2;
  tess_texCoords0[2 * ndx + 3] = t1;   tess_texCoords1[2 * ndx + 3] = t1;
  tess_texCoords0[2 * ndx + 4] = s2;   tess_texCoords1[2 * ndx + 4] = s2;
  tess_texCoords0[2 * ndx + 5] = t2;   tess_texCoords1[2 * ndx + 5] = t2;
  tess_texCoords0[2 * ndx + 6] = s1;   tess_texCoords1[2 * ndx + 6] = s1;
  tess_texCoords0[2 * ndx + 7] = t2;   tess_texCoords1[2 * ndx + 7] = t2;
  tess_texCoords0[2 * ndx + 8] = ( s1 + s2 ) * 0.5f;
  tess_texCoords1[2 * ndx + 8] = ( s1 + s2 ) * 0.5f;
  tess_texCoords0[2 * ndx + 9] = ( t1 + t2 ) * 0.5f;
  tess_texCoords1[2 * ndx + 9] = ( t1 + t2 ) * 0.5f;

  packedColor = *color;
  ((int *)tess_vertexColors)[ndx + 4] = packedColor;

  fadedColor = *color;
  ((unsigned char *)&fadedColor)[3] = 0;
  ((int *)tess_vertexColors)[ndx    ] = fadedColor;
  ((int *)tess_vertexColors)[ndx + 1] = fadedColor;
  ((int *)tess_vertexColors)[ndx + 2] = fadedColor;
  ((int *)tess_vertexColors)[ndx + 3] = fadedColor;

  tess_numVertexes += 5;                  /* 0x0050FDFC */
  tess_numIndexes  += 12;                 /* 0x0050FDFF */
  return tess_numIndexes;
}
#if 0
int __cdecl RB_AddQuadStampFadingCornersExt(
        void *a1,
        float *a2,
        float *a3,
        float *a4,
        int *a5,
        int a6,
        int a7,
        int a8,
        int a9)
{
  int v9;
  __int16 v10;
  int v11;
  int v12;
  float *v13;
  double v14;
  double v15;
  int v16;
  double v17;
  double v18;
  int result;
  __int16 v20;
  int v21;
  int v22;

  v9 = tess_numIndexes;
  if ( tess_numVertexes + 5 >= 0x2000
    || (a1 = (void *)(tess_numIndexes + 12), tess_numIndexes + 12 >= 49152)
    && (tess_numVertexes + 5 >= 0x2000 || tess_numIndexes + 12 >= 49152) )
  {
    RB_EndSurface(a1);
    backEnd_currentEntity = tess_entity;
    RB_BeginSurface(tess_vertexComponentCount);
    v9 = tess_numIndexes;
  }
  v10 = tess_numVertexes;
  v11 = (unsigned __int16)tess_numVertexes;
  v20 = tess_numVertexes;
  v12 = tess_vertexComponentCount * (unsigned __int16)tess_numVertexes;
  tess_indexes[v9] = tess_numVertexes;
  word_17A7F62[tess_numIndexes] = v10 + 1;
  word_17A7F64[tess_numIndexes] = v20 + 4;
  word_17A7F66[tess_numIndexes] = v20 + 1;
  word_17A7F68[tess_numIndexes] = v20 + 2;
  word_17A7F6A[tess_numIndexes] = v20 + 4;
  word_17A7F6C[tess_numIndexes] = v20 + 2;
  word_17A7F6E[tess_numIndexes] = v20 + 3;
  word_17A7F70[tess_numIndexes] = v20 + 4;
  word_17A7F72[tess_numIndexes] = v20 + 3;
  word_17A7F74[tess_numIndexes] = v20;
  word_17A7F76[tess_numIndexes] = v20 + 4;
  v13 = (float *)(4 * v12 + ((int)(char *)tess_xyz + 24));
  *(v13 - 6) = *a2 + *a3 + *a4;
  *(v13 - 5) = a4[1] + a3[1] + a2[1];
  *(v13 - 4) = a4[2] + a3[2] + a2[2];
  *(v13 - 3) = *a4 - *a2 + *a3;
  *(v13 - 2) = a4[1] - a2[1] + a3[1];
  *(v13 - 1) = a4[2] - a2[2] + a3[2];
  *v13 = *a4 - *a2 - *a3;
  v13 += 7;
  *(v13 - 6) = a4[1] - a2[1] - a3[1];
  *(v13 - 5) = a4[2] - a2[2] - a3[2];
  *(v13 - 4) = *a2 + *a4 - *a3;
  *(v13 - 3) = a4[1] + a2[1] - a3[1];
  *(v13 - 2) = a4[2] + a2[2] - a3[2];
  *(v13 - 1) = *a4;
  v14 = 0.0 - backEnd_viewParms_axis00;
  *v13 = a4[1];
  v15 = 0.0 - backEnd_viewParms_axis01;
  v13[1] = a4[2];
  v16 = 12 * v11;
  *(float *)&v21 = 0.0 - backEnd_viewParms_axis02;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 48)) = v14;
  dword_1827F98[v16 / 4u] = v21;
  dword_1827F8C[v16 / 4u] = v21;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 36)) = v14;
  dword_1827F80[v16 / 4u] = v21;
  dword_1827F74[v16 / 4u] = v21;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 24)) = v14;
  dword_1827F68[v16 / 4u] = *(float *)&v21;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 12)) = v14;
  *(float *)(v16 + ((int)(char *)tess_stageNormals)) = v14;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 52)) = v15;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 40)) = v15;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 28)) = v15;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 16)) = v15;
  *(float *)(v16 + ((int)(char *)tess_stageNormals + 4)) = v15;
  tess_texCoords1[2 * v11] = *(float *)&a6;
  tess_texCoords0[2 * v11] = a6;
  flt_17FFF64[2 * v11] = *(float *)&a7;
  dword_17DFF64[2 * v11] = a7;
  flt_17FFF68[2 * v11] = *(float *)&a8;
  dword_17DFF68[2 * v11] = a8;
  flt_17FFF6C[2 * v11] = *(float *)&a7;
  dword_17DFF6C[2 * v11] = a7;
  flt_17FFF70[2 * v11] = *(float *)&a8;
  dword_17DFF70[2 * v11] = a8;
  flt_17FFF74[2 * v11] = *(float *)&a9;
  dword_17DFF74[2 * v11] = a9;
  flt_17FFF78[2 * v11] = *(float *)&a6;
  dword_17DFF78[2 * v11] = a6;
  flt_17FFF7C[2 * v11] = *(float *)&a9;
  dword_17DFF7C[2 * v11] = a9;
  v17 = (*(float *)&a6 + *(float *)&a8) * 0.5;
  flt_17FFF80[2 * v11] = v17;
  flt_17DFF80[2 * v11] = v17;
  v18 = (*(float *)&a7 + *(float *)&a9) * 0.5;
  flt_17FFF84[2 * v11] = v18;
  flt_17DFF84[2 * v11] = v18;
  dword_181FF70[v11] = *a5;
  v22 = *a5;
  BYTE3(v22) = 0;
  dword_181FF6C[v11] = v22;
  dword_181FF68[v11] = v22;
  dword_181FF64[v11] = v22;
  tess_vertexColors[v11] = v22;
  result = tess_numIndexes + 12;
  tess_numVertexes += 5;
  tess_numIndexes += 12;
  return result;
}
#endif

/* ---- RB_AddQuadStampExt  0x0050FE20 ----  VERIFIED */
int __cdecl RB_AddQuadStampExt( float *origin, float *left, float *up, int *color,
                                float s1, float t1, float s2, float t2 )
{
  int    numIndexes;
  int    numVertexes;
  int    ndx;
  float *xyz;
  float *normal;
  float  nx, ny, nz;
  int    packedColor;

  numIndexes  = tess_numIndexes;
  numVertexes = tess_numVertexes;

  if ( numVertexes + 4 >= SHADER_MAX_VERTEXES
    || ( numIndexes + 6 >= SHADER_MAX_INDEXES
      && ( numVertexes + 4 >= SHADER_MAX_VERTEXES
        || numIndexes + 6 >= SHADER_MAX_INDEXES ) ) )
  {
    RB_EndSurface();                                     /* 0x0050FE57 */
    backEnd_currentEntity = tess_entity;                 /* 0x0050FE68 */
    RB_BeginSurface( (void *)tess_shader, tess_vertexComponentCount );
    numIndexes  = tess_numIndexes;                       /* 0x0050FE7A */
    numVertexes = tess_numVertexes;                      /* 0x0050FE80 */
  }

  tess_indexes[numIndexes    ] = (unsigned short)( numVertexes     );
  tess_indexes[numIndexes + 1] = (unsigned short)( numVertexes + 1 );
  tess_indexes[numIndexes + 2] = (unsigned short)( numVertexes + 3 );
  tess_indexes[numIndexes + 3] = (unsigned short)( numVertexes + 3 );
  tess_indexes[numIndexes + 4] = (unsigned short)( numVertexes + 1 );
  tess_indexes[numIndexes + 5] = (unsigned short)( numVertexes + 2 );

  /* 0x0050FEF4: base = 0x017BFF60 + 4 * (vcc * numVertexes). */
  xyz = &tess_xyz[ tess_vertexComponentCount * numVertexes ];

  xyz[ 0] = left[0] + up[0]     + origin[0];
  xyz[ 1] = left[1] + origin[1] + up[1];
  xyz[ 2] = left[2] + origin[2] + up[2];

  xyz[ 3] = origin[0] - left[0] + up[0];
  xyz[ 4] = origin[1] - left[1] + up[1];
  xyz[ 5] = origin[2] - left[2] + up[2];

  xyz[ 6] = origin[0] - left[0] - up[0];
  xyz[ 7] = origin[1] - left[1] - up[1];
  xyz[ 8] = origin[2] - left[2] - up[2];

  xyz[ 9] = left[0] + origin[0] - up[0];
  xyz[10] = left[1] + origin[1] - up[1];
  xyz[11] = left[2] + origin[2] - up[2];

  /* 0x0050FF9C-0x0050FFC6. */
  nx = 0.0f - backEnd_viewParms_axis00;
  ny = 0.0f - backEnd_viewParms_axis01;
  nz = 0.0f - backEnd_viewParms_axis02;

  ndx    = (unsigned short)numVertexes;          /* 0x0050FFB4 movzx eax,di */
  normal = &tess_stageNormals[ 3 * ndx ];        /* 0x0050FFBD/C6: 12*ndx bytes */

  normal[ 0] = nx;  normal[ 1] = ny;  normal[ 2] = nz;
  normal[ 3] = nx;  normal[ 4] = ny;  normal[ 5] = nz;
  normal[ 6] = nx;  normal[ 7] = ny;  normal[ 8] = nz;
  normal[ 9] = nx;  normal[10] = ny;  normal[11] = nz;

  tess_texCoords0[2 * ndx    ] = s1;   tess_texCoords1[2 * ndx    ] = s1;
  tess_texCoords0[2 * ndx + 1] = t1;   tess_texCoords1[2 * ndx + 1] = t1;
  tess_texCoords0[2 * ndx + 2] = s2;   tess_texCoords1[2 * ndx + 2] = s2;
  tess_texCoords0[2 * ndx + 3] = t1;   tess_texCoords1[2 * ndx + 3] = t1;
  tess_texCoords0[2 * ndx + 4] = s2;   tess_texCoords1[2 * ndx + 4] = s2;
  tess_texCoords0[2 * ndx + 5] = t2;   tess_texCoords1[2 * ndx + 5] = t2;
  tess_texCoords0[2 * ndx + 6] = s1;   tess_texCoords1[2 * ndx + 6] = s1;
  tess_texCoords0[2 * ndx + 7] = t2;   tess_texCoords1[2 * ndx + 7] = t2;

  /* 0x005100D7: one dword load, four dword stores. */
  packedColor = *color;
  ((int *)tess_vertexColors)[ndx    ] = packedColor;
  ((int *)tess_vertexColors)[ndx + 1] = packedColor;
  ((int *)tess_vertexColors)[ndx + 2] = packedColor;
  ((int *)tess_vertexColors)[ndx + 3] = packedColor;

  tess_numVertexes += 4;                          /* 0x00510100 */
  tess_numIndexes  += 6;                          /* 0x00510103 */
  return tess_numIndexes;
}
#if 0
int __cdecl RB_AddQuadStampExt(float *a1, float *a2, float *a3, int *a4, int a5, int a6, int a7, int a8)
{
  int v8;
  int v9;
  GLint v10;
  float *v11;
  double v12;
  double v13;
  int v14;
  int v15;
  int result;
  int v17;

  v8 = tess_numIndexes;
  v9 = tess_numVertexes;
  if ( tess_numVertexes + 4 >= 0x2000
    || tess_numIndexes + 6 >= 49152 && (tess_numVertexes + 4 >= 0x2000 || tess_numIndexes + 6 >= 49152) )
  {
    RB_EndSurface((void *)tess_numIndexes);
    backEnd_currentEntity = tess_entity;
    RB_BeginSurface(tess_vertexComponentCount);
    v8 = tess_numIndexes;
    v9 = tess_numVertexes;
  }
  v10 = tess_vertexComponentCount;
  tess_indexes[v8] = v9;
  word_17A7F62[tess_numIndexes] = v9 + 1;
  word_17A7F64[tess_numIndexes] = v9 + 3;
  word_17A7F66[tess_numIndexes] = v9 + 3;
  word_17A7F68[tess_numIndexes] = v9 + 1;
  word_17A7F6A[tess_numIndexes] = v9 + 2;
  v11 = (float *)(4 * v9 * v10 + ((int)(char *)tess_xyz + 28));
  *(v11 - 7) = *a2 + *a3 + *a1;
  v11 += 3;
  *(v11 - 9) = a2[1] + a1[1] + a3[1];
  *(v11 - 8) = a2[2] + a1[2] + a3[2];
  *(v11 - 7) = *a1 - *a2 + *a3;
  *(v11 - 6) = a1[1] - a2[1] + a3[1];
  *(v11 - 5) = a1[2] - a2[2] + a3[2];
  *(v11 - 4) = *a1 - *a2 - *a3;
  *(v11 - 3) = a1[1] - a2[1] - a3[1];
  *(v11 - 2) = a1[2] - a2[2] - a3[2];
  *(v11 - 1) = *a2 + *a1 - *a3;
  *v11 = a2[1] + a1[1] - a3[1];
  v11[1] = a2[2] + a1[2] - a3[2];
  v12 = 0.0 - backEnd_viewParms_axis00;
  v13 = 0.0 - backEnd_viewParms_axis01;
  v14 = 12 * (unsigned __int16)v9;
  *(float *)&v17 = 0.0 - backEnd_viewParms_axis02;
  dword_1827F68[v14 / 4u] = *(float *)&v17;
  *(float *)(v14 + ((int)(char *)tess_stageNormals + 36)) = v12;
  dword_1827F8C[v14 / 4u] = v17;
  dword_1827F80[v14 / 4u] = v17;
  *(float *)(v14 + ((int)(char *)tess_stageNormals + 24)) = v12;
  dword_1827F74[v14 / 4u] = v17;
  *(float *)(v14 + ((int)(char *)tess_stageNormals + 12)) = v12;
  *(float *)(v14 + ((int)(char *)tess_stageNormals)) = v12;
  *(float *)(v14 + ((int)(char *)tess_stageNormals + 40)) = v13;
  *(float *)(v14 + ((int)(char *)tess_stageNormals + 28)) = v13;
  *(float *)(v14 + ((int)(char *)tess_stageNormals + 16)) = v13;
  *(float *)(v14 + ((int)(char *)tess_stageNormals + 4)) = v13;
  tess_texCoords1[2 * (unsigned __int16)v9] = *(float *)&a5;
  tess_texCoords0[2 * (unsigned __int16)v9] = a5;
  flt_17FFF64[2 * (unsigned __int16)v9] = *(float *)&a6;
  dword_17DFF64[2 * (unsigned __int16)v9] = a6;
  flt_17FFF68[2 * (unsigned __int16)v9] = *(float *)&a7;
  dword_17DFF68[2 * (unsigned __int16)v9] = a7;
  flt_17FFF6C[2 * (unsigned __int16)v9] = *(float *)&a6;
  dword_17DFF6C[2 * (unsigned __int16)v9] = a6;
  flt_17FFF70[2 * (unsigned __int16)v9] = *(float *)&a7;
  dword_17DFF70[2 * (unsigned __int16)v9] = a7;
  flt_17FFF74[2 * (unsigned __int16)v9] = *(float *)&a8;
  dword_17DFF74[2 * (unsigned __int16)v9] = a8;
  flt_17FFF78[2 * (unsigned __int16)v9] = *(float *)&a5;
  dword_17DFF78[2 * (unsigned __int16)v9] = a5;
  flt_17FFF7C[2 * (unsigned __int16)v9] = *(float *)&a8;
  dword_17DFF7C[2 * (unsigned __int16)v9] = a8;
  v15 = *a4;
  dword_181FF6C[(unsigned __int16)v9] = *a4;
  dword_181FF68[(unsigned __int16)v9] = v15;
  dword_181FF64[(unsigned __int16)v9] = v15;
  tess_vertexColors[(unsigned __int16)v9] = v15;
  result = tess_numIndexes + 6;
  tess_numVertexes += 4;
  tess_numIndexes += 6;
  return result;
}
#endif

/* ---- RB_AddQuadStamp  0x00510120 ----  VERIFIED */
int __cdecl RB_AddQuadStamp(float *a1, float *a2, int *a3, float *a4)
{
  return RB_AddQuadStampExt(a4, a2, a1, a3, 0.0f, 0.0f, 1.0f, 1.0f);
}

/* ---- RB_SurfaceSplash  0x00510140 ----  VERIFIED */
int RB_SurfaceSplash()
{
  double v0;
  float v2[3]; /* [ebp-18h] BYREF */
  int v5[3]; // [esp+10h] [ebp-Ch] BYREF

  v0 = *(float *)(backEnd_currentEntity + 124);
  v2[1] = 0.0;
  v2[0] = -v0;
  v2[2] = 0.0;
  *(float *)&v5[1] = v0;
  v5[0] = 0;
  v5[2] = 0;
  if ( backEnd_viewParms_isMirror )
  {
    v2[1] = 0.0;
    v2[2] = 0.0;
    v2[0] = 0.0 - v2[0];
  }
  return RB_AddQuadStampExt(
           (float *)(backEnd_currentEntity + 68),
           v2,
           (float *)v5,
           (int *)(backEnd_currentEntity + 108),
           0.0f,
           0.0f,
           1.0f,
           1.0f);
}

/* ---- RB_SurfaceSprite  0x005101D0 ----  [CONFIRMED] */
int RB_SurfaceSprite()
{
  int v0;
  double v1;
  double v2;
  double v3;
  double v4;
  double v5;
  double v6;
  float v8;
  float v9;
  float v10;
  float v11; // [esp+8h] [ebp-2Ch] BYREF
  float v12[3]; // [esp+Ch] [ebp-28h] BYREF
  float v13;
  float v14[3]; /* [ebp-18h] BYREF */
  float v17[3]; /* [ebp-0Ch] BYREF */

  v0 = backEnd_currentEntity;
  v8 = *(float *)(backEnd_currentEntity + 124);
  v10 = *(float *)(backEnd_currentEntity + 100);
  if ( (*(float *)(backEnd_currentEntity + 128) == 0.0) | __UNORDERED__(*(float *)(backEnd_currentEntity + 128), 0.0) )
  {
    v14[0] = backEnd_viewParms_axis10 * v8;
    v14[1] = backEnd_viewParms_axis11 * v8;
    v14[2] = backEnd_viewParms_axis12 * v8;
    v17[0] = backEnd_viewParms_axis20 * v10;
    v17[1] = backEnd_viewParms_axis21 * v10;
    v1 = backEnd_viewParms_axis22 * v10;
  }
  else
  {
    v2 = *(float *)(backEnd_currentEntity + 128) * 0.017453292;
    LODWORD(v12[2]) = &v11;
    LODWORD(v12[1]) = v12;
    v13 = v2;
    v11 = cos(v13);
    v12[0] = sin(v13);
    v0 = backEnd_currentEntity;
    v3 = v11 * v8;
    v14[1] = v3 * backEnd_viewParms_axis11;
    v14[2] = v3 * backEnd_viewParms_axis12;
    v4 = -(v12[0] * v8);
    v9 = v4;
    v14[0] = v4 * backEnd_viewParms_axis20 + backEnd_viewParms_axis10 * v3;
    v14[1] = v9 * backEnd_viewParms_axis21 + v14[1];
    v14[2] = v9 * backEnd_viewParms_axis22 + v14[2];
    v5 = v11 * v10;
    v17[1] = v5 * backEnd_viewParms_axis21;
    v17[2] = v5 * backEnd_viewParms_axis22;
    v6 = v12[0] * v10;
    v12[0] = v6;
    v17[0] = v6 * backEnd_viewParms_axis10 + backEnd_viewParms_axis20 * v5;
    v17[1] = v12[0] * backEnd_viewParms_axis11 + v17[1];
    v1 = v12[0] * backEnd_viewParms_axis12 + v17[2];
  }
  v17[2] = v1;
  if ( backEnd_viewParms_isMirror )
  {
    v14[0] = 0.0 - v14[0];
    v14[1] = 0.0 - v14[1];
    v14[2] = 0.0 - v14[2];
  }
  return RB_AddQuadStampExt(
           (float *)(v0 + 68),
           v14,
           v17,
           (int *)(v0 + 108),
           0.0f,
           0.0f,
           1.0f,
           1.0f);
}

/* ---- RB_SurfaceOrientedQuad  0x005103C0 ----  VERIFIED */
int RB_SurfaceOrientedQuad( void )
{
  int    e;
  float  width, height;
  float  right[3], up[3];
  float  s, c;
  float  rotRight[3];
  int    i;

  e      = backEnd_currentEntity;
  width  = *(float *)( e + 124 );
  height = *(float *)( e + 100 );

  MakeNormalVectors( (const float *)( e + 28 ), right, up );

  if ( *(float *)( e + 128 ) == 0.0f )
  {
    for ( i = 0; i < 3; i++ )
    {
      right[i] *= width;
      up[i]    *= height;
    }
  }
  else
  {
    /* fsincos at 0x00510470: cos -> var_44, sin -> var_40. */
    float ang = *(float *)( e + 128 ) * 0.017453292f;
    c = (float)cos( ang );
    s = (float)sin( ang );

    for ( i = 0; i < 3; i++ )
      rotRight[i] = ( right[i] * c - up[i] * s ) * width;
    for ( i = 0; i < 3; i++ )
      up[i] = ( right[i] * s + up[i] * c ) * height;
    for ( i = 0; i < 3; i++ )
      right[i] = rotRight[i];
  }

  if ( backEnd_viewParms_isMirror )
  {
    right[0] = -right[0];
    right[1] = -right[1];
    right[2] = -right[2];
  }

  return RB_AddQuadStampExt( (float *)( e + 68 ), right, up, (int *)( e + 108 ),
                             0.0f, 0.0f, 1.0f, 1.0f );
}

/* ---- RB_SurfacePolychain  0x005105B0 ----  VERIFIED */
int __cdecl RB_SurfacePolychain(int a1)
{
  int v1;
  int v3;
  int v4;
  _DWORD *v5;
  _DWORD *v6;
  _DWORD *v7;
  _DWORD *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  float v15;
  float dx, dy, dz;
  int result;
  int v18;

  v1 = tess_numVertexes;
  v3 = *(_DWORD *)(a1 + 8);
  if ( v3 + tess_numVertexes >= 0x2000 || tess_numIndexes + 2 * (v3 - 2) + v3 - 2 >= 49152 )
  {
    RB_CheckOverflow(v3, 3 * v3 - 6);
    v1 = tess_numVertexes;
  }
  v4 = 0;
  v5 = (_DWORD *)(4 * v1 * tess_vertexComponentCount + ((int)(char *)tess_xyz));
  v6 = (_DWORD *)(8 * v1 + ((int)(char *)tess_texCoords0));
  v7 = (_DWORD *)(8 * v1 + ((int)(char *)tess_texCoords1));
  v8 = (_DWORD *)(4 * v1 + ((int)(char *)tess_vertexColors));
  v18 = 0;
  if ( *(int *)(a1 + 8) > 0 )
  {
    do
    {
      *v5 = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12));
      v5[1] = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12) + 4);
      v5[2] = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12) + 8);
      *v6 = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12) + 12);
      v6[1] = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12) + 16);
      *v7 = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12) + 20);
      v7[1] = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12) + 24);
      *v8 = *(_DWORD *)(v4 + *(_DWORD *)(a1 + 12) + 28);
      v5 += tess_vertexComponentCount;
      v6 += 2;
      v7 += 2;
      ++v8;
      v4 += 32;
      ++v18;
    }
    while ( v18 < *(_DWORD *)(a1 + 8) );
    v1 = tess_numVertexes;
  }
  if ( tr_refdef_num_dlights )
  {
    if ( *(char *)(tess_shader + 84) < 0 )
    {
      v9 = 0;
      if ( tr_refdef_num_dlights > 0 )
      {
        v10 = tr_refdef_dlights + 120;
        do
        {
          if ( ((1 << v9) & tess_dlightBits) == 0 )
          {
            v11 = *(_DWORD *)(a1 + 8);
            v12 = 0;
            v15 = *(float *)(v10 - 4) * *(float *)(v10 - 4);
            if ( v11 > 0 )
            {
              v13 = *(_DWORD *)(a1 + 12);
              while ( 1 )
              {
                dx = *(float *)v13 - *(float *)v10;
                dy = *(float *)(v13 + 4) - *(float *)(v10 + 4);
                dz = *(float *)(v13 + 8) - *(float *)(v10 + 8);
                if ( dx * dx + dy * dy + dz * dz < v15 )
                  break;
                ++v12;
                v13 += 32;
                if ( v12 >= v11 )
                  goto LABEL_18;
              }
              tess_dlightBits |= 1 << v9;
            }
          }
LABEL_18:
          ++v9;
          v10 += 136;
        }
        while ( v9 < tr_refdef_num_dlights );
        v1 = tess_numVertexes;
      }
    }
  }
  result = 0;
  if ( *(int *)(a1 + 8) - 2 > 0 )
  {
    while ( 1 )
    {
      tess_indexes[tess_numIndexes] = v1;
      word_17A7F62[tess_numIndexes] = result + tess_numVertexes + 1;
      word_17A7F64[tess_numIndexes] = result + tess_numVertexes + 2;
      tess_numIndexes += 3;
      if ( ++result >= *(int *)(a1 + 8) - 2 )
        break;
      LOWORD(v1) = tess_numVertexes;
    }
    result = *(_DWORD *)(a1 + 8) + tess_numVertexes;
    tess_numVertexes = result;
  }
  else
  {
    tess_numVertexes = *(_DWORD *)(a1 + 8) + v1;
  }
  return result;
}

#define NUM_BEAM_SEGS 6

/* ---- RB_SurfaceBeam  0x00510800 ----  VERIFIED */
void RB_SurfaceBeam()
{
  int i;
  float perpvec[3];
  float direction[3], normalized_direction[3];
  float start_points[NUM_BEAM_SEGS][3], end_points[NUM_BEAM_SEGS][3];

  normalized_direction[0] = direction[0] =
      *(float *)(backEnd_currentEntity + 84) - *(float *)(backEnd_currentEntity + 68);
  normalized_direction[1] = direction[1] =
      *(float *)(backEnd_currentEntity + 88) - *(float *)(backEnd_currentEntity + 72);
  normalized_direction[2] = direction[2] =
      *(float *)(backEnd_currentEntity + 92) - *(float *)(backEnd_currentEntity + 76);

  if ( VectorNormalize( normalized_direction ) == 0.0 )   /* 0x00510867 */
    return;

  PerpendicularVector( perpvec, normalized_direction );

  VectorScale( perpvec, 4, perpvec );

  for ( i = 0; i < NUM_BEAM_SEGS; i++ )
  {
    RotatePointAroundVector( start_points[i], normalized_direction, perpvec,
                             (360.0 / NUM_BEAM_SEGS) * i );
    VectorAdd( start_points[i], direction, end_points[i] );
  }

  GL_Bind( (GLenum *)tr_whiteImage );
  GL_State( 34 );
  RB_BeginImmediateMode();

  rbDebug_immediateColorR = -1;
  rbDebug_immediateColorG = 0;
  rbDebug_immediateColorB = 0;
  rbDebug_immediateColorA = -1;
  mode = 5;

  for ( i = 0; i <= NUM_BEAM_SEGS; i++ )
  {
    RB_glVertex3f( *(int *)&start_points[i % NUM_BEAM_SEGS][0],
                   *(int *)&start_points[i % NUM_BEAM_SEGS][1],
                   *(int *)&start_points[i % NUM_BEAM_SEGS][2] );
    RB_glVertex3f( *(int *)&end_points[i % NUM_BEAM_SEGS][0],
                   *(int *)&end_points[i % NUM_BEAM_SEGS][1],
                   *(int *)&end_points[i % NUM_BEAM_SEGS][2] );
  }

  glDrawArrays( mode, 0, rbDebug_immediateVertexCount );                 /* 0x00510F8B */
  rbDebug_immediateVertexCount = 0;
  mode = 0;

  /* RB_EndImmediateMode, open-coded (0x00510F91-0x00510FF9). */
  rbDebug_immediateModeActive = 0;
  R_FogOn();
  glVertexPointer( 3, 0x1406u, 0, tess_xyz );
}

/* ---- DoLine  0x00511010 ----  VERIFIED */
int __cdecl DoLine( float *right, float *start, float *end, float spanWidth )
{
  int    numIndexes;
  int    numVertexes;
  float  negSpan;
  int    color;
  float *xyz;
  float *st;
  int   *colors;
  int    vcc;

  numVertexes = tess_numVertexes;

  if ( numVertexes + 4 >= SHADER_MAX_VERTEXES
    || ( tess_numIndexes + 6 >= SHADER_MAX_INDEXES
      && ( numVertexes + 4 >= SHADER_MAX_VERTEXES
        || tess_numIndexes + 6 >= SHADER_MAX_INDEXES ) ) )
  {
    RB_EndSurface();                                     /* 0x0051104B */
    backEnd_currentEntity = tess_entity;                 /* 0x00511062 */
    RB_BeginSurface( (void *)tess_shader, tess_vertexComponentCount );
    numVertexes = tess_numVertexes;                      /* 0x0051106C */
  }

  negSpan = -spanWidth;                                  /* 0x0051107B */
  color   = *(int *)( backEnd_currentEntity + 108 );     /* entity + 0x6C */

  vcc    = tess_vertexComponentCount;
  xyz    = &tess_xyz[ vcc * numVertexes ];               /* 0x00511095 */
  st     = &tess_texCoords0[ 2 * numVertexes ];          /* 0x0051109C */
  colors = &((int *)tess_vertexColors)[ numVertexes ];   /* 0x005110B0 */

  xyz[0] = spanWidth * right[0] + start[0];
  xyz[1] = spanWidth * right[1] + start[1];
  xyz[2] = spanWidth * right[2] + start[2];
  st[0]  = 0.0f;   st[1] = 0.0f;
  colors[0] = color;

  xyz += tess_vertexComponentCount;                      /* 0x005110F8 */
  xyz[0] = negSpan * right[0] + start[0];
  xyz[1] = negSpan * right[1] + start[1];
  xyz[2] = negSpan * right[2] + start[2];
  st[2]  = 1.0f;   st[3] = 0.0f;
  colors[1] = color;

  xyz += tess_vertexComponentCount;                      /* 0x0051113B */
  xyz[0] = spanWidth * right[0] + end[0];
  xyz[1] = spanWidth * right[1] + end[1];
  xyz[2] = spanWidth * right[2] + end[2];
  st[4]  = 0.0f;   st[5] = 1.0f;
  colors[2] = color;

  xyz += tess_vertexComponentCount;                      /* 0x00511176 */
  xyz[0] = negSpan * right[0] + end[0];
  xyz[1] = negSpan * right[1] + end[1];
  xyz[2] = negSpan * right[2] + end[2];
  st[6]  = 1.0f;   st[7] = 1.0f;
  colors[3] = color;

  numIndexes = tess_numIndexes;
  tess_indexes[numIndexes    ] = (unsigned short)( numVertexes     );
  tess_indexes[numIndexes + 1] = (unsigned short)( numVertexes + 1 );
  tess_indexes[numIndexes + 2] = (unsigned short)( numVertexes + 2 );
  tess_indexes[numIndexes + 3] = (unsigned short)( numVertexes + 2 );
  tess_indexes[numIndexes + 4] = (unsigned short)( numVertexes + 1 );
  tess_indexes[numIndexes + 5] = (unsigned short)( numVertexes + 3 );

  tess_numIndexes  += 6;                                 /* 0x00511237 */
  tess_numVertexes += 4;                                 /* 0x00511233 */
  return tess_numVertexes;                               /* EAX = nv + 4 */
}
#if 0
int __cdecl DoLine(float *a1, float *a2, float *a3, float a4)
{
  int v5;
  double v7;
  int v8;
  float *v9;
  _DWORD *v10;
  _DWORD *v11;
  GLint v12;
  float *v13;
  GLint v14;
  float *v15;
  GLint v16;
  float *v17;
  int result;
  int v19;

  v5 = tess_numVertexes;
  if ( tess_numVertexes + 4 >= 0x2000
    || tess_numIndexes + 6 >= 49152 && (tess_numVertexes + 4 >= 0x2000 || tess_numIndexes + 6 >= 49152) )
  {
    RB_EndSurface((void *)tess_numVertexes);
    backEnd_currentEntity = tess_entity;
    RB_BeginSurface(tess_vertexComponentCount);
    v5 = tess_numVertexes;
  }
  v7 = -a4;
  v8 = *(_DWORD *)(backEnd_currentEntity + 108);
  v9 = (float *)(4 * v5 * tess_vertexComponentCount + ((int)(char *)tess_xyz));
  v10 = (_DWORD *)(8 * v5 + ((int)(char *)tess_texCoords0 + 4));
  *v9 = a4 * *a1 + *a2;
  v19 = v8;
  v11 = (_DWORD *)(4 * v5 + ((int)(char *)tess_vertexColors + 8));
  v9[1] = a4 * a1[1] + a2[1];
  v10 += 5;
  v9[2] = a4 * a1[2] + a2[2];
  v12 = tess_vertexComponentCount;
  *(v10 - 6) = 0;
  *(v10 - 5) = 0;
  v13 = &v9[v12];
  *(v11 - 2) = v19;
  *v13 = v7 * *a1 + *a2;
  v13[1] = v7 * a1[1] + a2[1];
  v13[2] = v7 * a1[2] + a2[2];
  v14 = tess_vertexComponentCount;
  *(v10 - 4) = 1065353216;
  *(v10 - 3) = 0;
  *(v11 - 1) = v19;
  v15 = &v13[v14];
  *v15 = a4 * *a1 + *a3;
  v15[1] = a4 * a1[1] + a3[1];
  v15[2] = a4 * a1[2] + a3[2];
  v16 = tess_vertexComponentCount;
  *(v10 - 2) = 0;
  *(v10 - 1) = 1065353216;
  *v11 = v19;
  v17 = &v15[v16];
  *v17 = v7 * *a1 + *a3;
  v17[1] = v7 * a1[1] + a3[1];
  v17[2] = v7 * a1[2] + a3[2];
  *v10 = 1065353216;
  v10[1] = 1065353216;
  v11[1] = v19;
  LOWORD(v10) = tess_numVertexes;
  tess_indexes[tess_numIndexes++] = tess_numVertexes;
  tess_indexes[tess_numIndexes++] = (_WORD)v10 + 1;
  tess_indexes[tess_numIndexes++] = (_WORD)v10 + 2;
  tess_indexes[tess_numIndexes++] = (_WORD)v10 + 2;
  tess_indexes[tess_numIndexes++] = (_WORD)v10 + 1;
  tess_indexes[tess_numIndexes] = (_WORD)v10 + 3;
  result = tess_numVertexes + 4;
  ++tess_numIndexes;
  tess_numVertexes += 4;
  return result;
}
#endif

/* ---- RB_SurfaceLine  0x00511250 ----  [HIGH] */
int RB_SurfaceLine()
{
  int v0;
  float v1;
  float v2;
  float v4;
  float v5;
  float v6;
  float v7;
  float v8;
  float v9;
  float v10[3]; // [esp+20h] [ebp-24h] BYREF
  float v11[3]; // [esp+2Ch] [ebp-18h] BYREF
  float v14[3]; // [esp+38h] [ebp-Ch] BYREF

  v0 = backEnd_currentEntity;
  v1 = *(float *)(backEnd_currentEntity + 88);
  v2 = *(float *)(backEnd_currentEntity + 92);
  v10[0] = *(float *)(backEnd_currentEntity + 84);
  v11[0] = *(float *)(backEnd_currentEntity + 68);
  v10[1] = v1;
  v11[1] = *(float *)(backEnd_currentEntity + 72);
  v7 = v11[0] - backEnd_viewParms_originX[0];
  v10[2] = v2;
  v11[2] = *(float *)(backEnd_currentEntity + 76);
  v8 = v11[1] - backEnd_viewParms_originY;
  v9 = v11[2] - backEnd_viewParms_originZ;
  v4 = v10[0] - backEnd_viewParms_originX[0];
  v5 = v1 - backEnd_viewParms_originY;
  v6 = v2 - backEnd_viewParms_originZ;
  v14[0] = v6 * v8 - v5 * v9;
  v14[1] = v4 * v9 - v6 * v7;
  v14[2] = v5 * v7 - v4 * v8;
  VectorNormalize(v14);
  return DoLine(v14, v11, v10, *(float *)(v0 + 124));
}

#define CYLINDER_MIN_SEGS   8      /* cmp eax, 8    @0x00511403 */
#define CYLINDER_MAX_SEGS  32      /* cmp eax, 20h  @0x00511416 */

/* ---- RB_SurfaceCylinder  0x00511350 ----  VERIFIED */
void RB_SurfaceCylinder()
{
  int    i;
  int    segs;
  int    numVertexes;
  int    numIndexes;
  int    color;
  int    vcc;
  float  lodDist;
  float  angleStep;
  float  texStep;
  float  texCoord;
  float  angle;
  float  sa, ca;
  float  radiusA, radiusB;
  float  forward[3], right[3], up[3];
  float  start[CYLINDER_MAX_SEGS + 1][3];
  float  end[CYLINDER_MAX_SEGS + 1][3];
  float *xyz;
  float *st;
  int   *colors;

  forward[0] = ( *(float *)( backEnd_currentEntity + 68 )
               + *(float *)( backEnd_currentEntity + 84 ) ) * 0.5f - backEnd_viewParms_originX[0];
  forward[1] = ( *(float *)( backEnd_currentEntity + 88 )
               + *(float *)( backEnd_currentEntity + 72 ) ) * 0.5f - backEnd_viewParms_originY;
  forward[2] = ( *(float *)( backEnd_currentEntity + 92 )
               + *(float *)( backEnd_currentEntity + 76 ) ) * 0.5f - backEnd_viewParms_originZ;

  lodDist = VectorNormalize( forward );                  /* 0x005113BA */

  segs = (int)( ( 1.0 - lodDist * ( backEnd_viewParms_fovX * 0.011111111 ) * 0.0009765625 ) * 32.0
                + 9.313225746154785e-10 );
  if ( segs < CYLINDER_MIN_SEGS )
    segs = CYLINDER_MIN_SEGS;
  else if ( segs > CYLINDER_MAX_SEGS )
    segs = CYLINDER_MAX_SEGS;

  if ( tess_numVertexes + 2 * segs + 2 >= SHADER_MAX_VERTEXES   /* 0x00511431 */
    || tess_numIndexes + 6 * segs >= SHADER_MAX_INDEXES )       /* 0x00511445 */
    RB_CheckOverflow( 2 * segs + 2, 6 * segs );

  forward[0] = *(float *)( backEnd_currentEntity + 68 ) - *(float *)( backEnd_currentEntity + 84 );
  forward[1] = *(float *)( backEnd_currentEntity + 72 ) - *(float *)( backEnd_currentEntity + 88 );
  forward[2] = *(float *)( backEnd_currentEntity + 76 ) - *(float *)( backEnd_currentEntity + 92 );
  VectorNormalize( forward );

  MakeNormalVectors( forward, right, up );

  texStep   = 1.0f / (float)segs;                        /* 0x005114A6 */
  angleStep = texStep * 6.2831855f;                      /* 0x005114B4 */

  radiusA = *(float *)( backEnd_currentEntity + 100 );   /* [ebp+64h] */
  radiusB = *(float *)( backEnd_currentEntity + 124 );   /* [ebp+7Ch] */

  for ( i = 0; i < segs; i++ )
  {
    angle = (float)i * angleStep;
    ca = (float)cos( angle );                            /* fsincos, 0x005114F2 */
    sa = (float)sin( angle );

    start[i][0] = up[0] * ( ca * radiusA ) + right[0] * ( sa * radiusA )
                + *(float *)( backEnd_currentEntity + 68 );
    start[i][1] = up[1] * ( ca * radiusA ) + right[1] * ( sa * radiusA )
                + *(float *)( backEnd_currentEntity + 72 );
    start[i][2] = up[2] * ( ca * radiusA ) + right[2] * ( sa * radiusA )
                + *(float *)( backEnd_currentEntity + 76 );

    end[i][0] = up[0] * ( ca * radiusB ) + right[0] * ( sa * radiusB )
              + *(float *)( backEnd_currentEntity + 84 );
    end[i][1] = up[1] * ( ca * radiusB ) + right[1] * ( sa * radiusB )
              + *(float *)( backEnd_currentEntity + 88 );
    end[i][2] = up[2] * ( ca * radiusB ) + right[2] * ( sa * radiusB )
              + *(float *)( backEnd_currentEntity + 92 );
  }

  /* Close the ring: element [segs] repeats element [0].  0x005115B1 */
  start[segs][0] = start[0][0];
  start[segs][1] = start[0][1];
  start[segs][2] = start[0][2];
  end[segs][0]   = end[0][0];
  end[segs][1]   = end[0][1];
  end[segs][2]   = end[0][2];

  for ( i = 0; i < segs; i++ )                           /* 0x00511610 */
  {
    numVertexes = tess_numVertexes;

    numIndexes = tess_numIndexes;
    tess_indexes[numIndexes    ] = (unsigned short)( numVertexes + 2 * i     );
    tess_indexes[numIndexes + 1] = (unsigned short)( numVertexes + 2 * i + 1 );
    tess_indexes[numIndexes + 2] = (unsigned short)( numVertexes + 2 * i + 3 );
    tess_numIndexes = numIndexes + 3;

    numIndexes = tess_numIndexes;
    tess_indexes[numIndexes    ] = (unsigned short)( numVertexes + 2 * i + 3 );
    tess_indexes[numIndexes + 1] = (unsigned short)( numVertexes + 2 * i + 2 );
    tess_indexes[numIndexes + 2] = (unsigned short)( numVertexes + 2 * i     );
    tess_numIndexes = numIndexes + 3;
  }

  numVertexes = tess_numVertexes;                        /* 0x005116BD */
  vcc         = tess_vertexComponentCount;
  color       = *(int *)( backEnd_currentEntity + 108 ); /* entity + 0x6C */

  xyz    = &tess_xyz[ vcc * numVertexes ];
  st     = &tess_texCoords0[ 2 * numVertexes ];
  colors = &((int *)tess_vertexColors)[ numVertexes ];

  for ( i = 0; i <= segs; i++ )
  {
    texCoord = (float)i * texStep;

    xyz[0] = start[i][0];
    xyz[1] = start[i][1];
    xyz[2] = start[i][2];
    st[0]  = texCoord;
    st[1]  = 1.0f;
    colors[0] = color;
    xyz += tess_vertexComponentCount;

    xyz[0] = end[i][0];
    xyz[1] = end[i][1];
    xyz[2] = end[i][2];
    st[2]  = texCoord;
    st[3]  = 0.0f;
    colors[1] = color;
    xyz += tess_vertexComponentCount;

    st     += 4;
    colors += 2;
  }

  tess_numVertexes = numVertexes + 2 * ( segs + 1 );     /* 0x005119C0 */
}

/* ---- DoRailCore  0x005119F0 ----  [HIGH] */
int __cdecl DoRailCore(float *a1, float *a2, float *a3, float a4, float a5)
{
  double v5;
  double v6;
  int v7;
  double v9;
  int v10;
  int v11;
  int v12;
  int v14;
  _DWORD *v15;
  GLint v16;
  float *v17;
  GLint v18;
  float *v19;
  float *v20;
  int result;
  int v22;

  v5 = a4 * 0.00390625;
  v6 = -a5;
  v7 = *(_DWORD *)(backEnd_currentEntity + 108);
  v9 = a5 * *a3 + *a2;
  v10 = tess_numVertexes;
  v11 = tess_numVertexes * tess_vertexComponentCount;
  *(float *)&tess_xyz[v11] = v9;
  v12 = 4 * v11 + ((int)(char *)tess_xyz);
  *(float *)(v12 + 4) = a5 * a3[1] + a2[1];
  v14 = 8 * v10 + ((int)(char *)tess_texCoords0 + 4);
  v22 = v7;
  v15 = (_DWORD *)(4 * v10 + ((int)(char *)tess_vertexColors + 4));
  v14 += 16;
  *(float *)(v12 + 8) = a5 * a3[2] + a2[2];
  v16 = tess_vertexComponentCount;
  *(_DWORD *)(v14 - 20) = 0;
  *(_DWORD *)(v14 - 16) = 0;
  v17 = (float *)(v12 + 4 * v16);
  *(v15++ - 1) = v22;
  v14 += 4;
  *v17 = v6 * *a3 + *a2;
  v17[1] = v6 * a3[1] + a2[1];
  v17[2] = v6 * a3[2] + a2[2];
  v18 = tess_vertexComponentCount;
  *(_DWORD *)(v14 - 16) = 0;
  *(_DWORD *)(v14 - 12) = 1065353216;
  *(v15 - 1) = v22;
  v19 = &v17[v18];
  *v19 = a5 * *a3 + *a1;
  v19[1] = a5 * a3[1] + a1[1];
  v19[2] = a5 * a3[2] + a1[2];
  v20 = &v19[tess_vertexComponentCount];
  *(float *)(v14 - 8) = v5;
  *(_DWORD *)(v14 - 4) = 0;
  *v15 = v22;
  *v20 = v6 * *a3 + *a1;
  v20[1] = v6 * a3[1] + a1[1];
  v20[2] = v6 * a3[2] + a1[2];
  *(float *)v14 = v5;
  *(_DWORD *)(v14 + 4) = 1065353216;
  v15[1] = v22;
  LOWORD(v14) = tess_numVertexes;
  tess_indexes[tess_numIndexes++] = tess_numVertexes;
  tess_indexes[tess_numIndexes++] = v14 + 1;
  tess_indexes[tess_numIndexes++] = v14 + 2;
  tess_indexes[tess_numIndexes++] = v14 + 2;
  tess_indexes[tess_numIndexes++] = v14 + 1;
  tess_indexes[tess_numIndexes] = v14 + 3;
  result = tess_numVertexes + 4;
  ++tess_numIndexes;
  tess_numVertexes += 4;
  return result;
}

/* ---- DoRailDiscs  0x00511BE0 ----  VERIFIED */
int __cdecl DoRailDiscs( float *up, float *right, float *fwdStep,
                         int numSegs, float *start )
{
  int    railWidth;
  float  scale;
  int    angle;
  float  rad, s, c;
  float  x, y, z;
  float  discPoints[12];
  float *p;
  int    color;
  int    i, seg;
  int    numVertexes, numIndexes;
  float *xyz;
  float *st;
  int   *colors;
  int    tc;

  railWidth = r_railWidth->integer;              /* 0x00511BEB */
  if ( numSegs > 1 )
    --numSegs;                                   /* 0x00511C03 */
  if ( !numSegs )
    return numSegs;                              /* 0x00511C0A -> epilogue */

  scale = (float)railWidth;                      /* fild, 0x00511C10 */

  p = discPoints;
  for ( angle = 45; angle < 405; angle += 90 )
  {
    rad = (float)( (double)angle * 3.1415927 * 0.0055555557 );
    c   = (float)cos( rad );                     /* fsincos, 0x00511C5A */
    s   = (float)sin( rad );

    x = ( c * right[0] + s * up[0] ) * scale * 0.25f;
    y = ( s * up[1] + c * right[1] ) * scale * 0.25f;
    z = ( s * up[2] + c * right[2] ) * scale * 0.25f;

    p[0] = x + start[0];
    p[1] = y + start[1];
    p[2] = z + start[2];

    if ( numSegs > 1 )                           /* 0x00511CE0 */
    {
      p[0] = ( x + start[0] ) + fwdStep[0];
      p[1] = ( y + start[1] ) + fwdStep[1];
      p[2] = ( z + start[2] ) + fwdStep[2];
    }
    p += 3;
  }

  color = *(int *)( backEnd_currentEntity + 108 );   /* entity + 0x6C */
  if ( numSegs <= 0 )
    return numSegs;                              /* 0x00511D2A */

  seg = numSegs;
  do
  {
    numVertexes = tess_numVertexes;
    numIndexes  = tess_numIndexes;

    if ( numVertexes + 4 >= SHADER_MAX_VERTEXES
      || ( numIndexes + 6 >= SHADER_MAX_INDEXES
        && ( numVertexes + 4 >= SHADER_MAX_VERTEXES
          || numIndexes + 6 >= SHADER_MAX_INDEXES ) ) )
    {
      RB_EndSurface();                           /* 0x00511D69 */
      backEnd_currentEntity = tess_entity;
      RB_BeginSurface( (void *)tess_shader, tess_vertexComponentCount );
      numVertexes = tess_numVertexes;            /* 0x00511D8A */
    }

    xyz    = &tess_xyz[ tess_vertexComponentCount * numVertexes ];
    st     = &tess_texCoords0[ 2 * numVertexes ];
    colors = &((int *)tess_vertexColors)[ numVertexes ];

    p = discPoints;
    for ( i = 0; i < 4; i++ )
    {
      xyz[0] = p[0];
      xyz[1] = p[1];
      xyz[2] = p[2];
      xyz += tess_vertexComponentCount;          /* 0x00511DCC */

      st[0] = (float)( i < 2 );                  /* setl, 0x00511DD4 */
      tc = 1;
      if ( i == 0 || i == 3 )
        tc = 0;
      st[1] = (float)tc;
      st += 2;

      *colors++ = color;

      p[0] = fwdStep[0] + p[0];                  /* 0x00511E12: fwd first */
      p[1] = p[1] + fwdStep[1];
      p[2] = p[2] + fwdStep[2];
      p += 3;
    }

    numIndexes = tess_numIndexes;
    tess_indexes[numIndexes    ] = (unsigned short)( numVertexes     );
    tess_indexes[numIndexes + 1] = (unsigned short)( numVertexes + 1 );
    tess_indexes[numIndexes + 2] = (unsigned short)( numVertexes + 3 );
    tess_indexes[numIndexes + 3] = (unsigned short)( numVertexes + 3 );
    tess_indexes[numIndexes + 4] = (unsigned short)( numVertexes + 1 );
    tess_indexes[numIndexes + 5] = (unsigned short)( numVertexes + 2 );

    tess_numIndexes  += 6;
    tess_numVertexes += 4;
  }
  while ( --seg );

  return tess_numVertexes;                       /* EAX at 0x00511EEB */
}
#if 0
int __cdecl DoRailDiscs(float *a1, float *a2, float *a3, int a4, float *a5)
{
  int result;
  float *v7;
  long double v8;
  float *v9;
  double v10;
  double v11;
  double v12;
  int v13;
  int v14;
  int v15;
  _DWORD *v16;
  int v17;
  float *v18;
  int v19;
  float v20;
  double v21;
  bool v22; // zf
  float v23; // [esp+Ch] [ebp-60h] BYREF
  float v24; // [esp+10h] [ebp-5Ch] BYREF
  float v25;
  int integer;
  float *v27;
  float *v28;
  float v29;
  float v30;
  float v31;
  _BYTE v32[48]; // [esp+3Ch] [ebp-30h] BYREF
  int v33;

  result = a4;
  integer = r_railWidth->integer;
  if ( a4 > 1 )
    result = --a4;
  if ( result )
  {
    v28 = &v24;
    v25 = (float)integer;
    v27 = &v23;
    v7 = (float *)v32;
    for ( integer = 45; integer < 405; integer += 90 )
    {
      v29 = (double)integer * 3.1415927 * 0.0055555557;
      v8 = sin(v29);
      v9 = v27;
      *v28 = cos(v29);
      *v9 = v8;
      v10 = (v24 * *a2 + v23 * *a1) * v25 * 0.25;
      v11 = (v23 * a1[1] + v24 * a2[1]) * v25 * 0.25;
      v31 = (v23 * a1[2] + v24 * a2[2]) * v25 * 0.25;
      v29 = v10 + *a5;
      *(v7 - 1) = v29;
      v12 = v11 + a5[1];
      *v7 = v12;
      v30 = v31 + a5[2];
      v7[1] = v30;
      if ( a4 > 1 )
      {
        *(v7 - 1) = v29 + *a3;
        *v7 = v12 + a3[1];
        v7[1] = v30 + a3[2];
      }
      v7 += 3;
    }
    result = a4;
    v30 = *(float *)(backEnd_currentEntity + 108);
    if ( a4 > 0 )
    {
      v13 = tess_numIndexes;
      integer = a4;
      result = tess_numVertexes;
      do
      {
        if ( result + 4 >= 0x2000 || v13 + 6 >= 49152 && (result + 4 >= 0x2000 || (v13 += 6, v13 >= 49152)) )
        {
          RB_EndSurface((void *)v13);
          backEnd_currentEntity = tess_entity;
          RB_BeginSurface(tess_vertexComponentCount);
          result = tess_numVertexes;
        }
        v14 = 8 * result + ((int)(char *)tess_texCoords0);
        v15 = 4 * result + ((int)(char *)tess_vertexColors);
        v16 = (_DWORD *)(4 * result * tess_vertexComponentCount + ((int)(char *)tess_xyz));
        v17 = 0;
        v18 = (float *)v32;
        do
        {
          *v16 = *((_DWORD *)v18 - 1);
          v16[1] = *(_DWORD *)v18;
          v16[2] = *((_DWORD *)v18 + 1);
          v16 += tess_vertexComponentCount;
          v19 = v14 + 4;
          *(float *)(v19 - 4) = (float)(v17 < 2);
          if ( !v17 || (v33 = 1, v17 == 3) )
            v33 = 0;
          v20 = v30;
          v14 = v19 + 4;
          v15 += 4;
          *(float *)(v14 - 4) = (float)v33;
          *(float *)(v15 - 4) = v20;
          ++v17;
          v21 = *a3 + *(v18 - 1);
          v18 += 3;
          *(v18 - 4) = v21;
          *(v18 - 3) = *(v18 - 3) + a3[1];
          *(v18 - 2) = *(v18 - 2) + a3[2];
        }
        while ( v17 < 4 );
        tess_indexes[tess_numIndexes++] = tess_numVertexes;
        tess_indexes[tess_numIndexes++] = tess_numVertexes + 1;
        tess_indexes[tess_numIndexes++] = tess_numVertexes + 3;
        tess_indexes[tess_numIndexes++] = tess_numVertexes + 3;
        tess_indexes[tess_numIndexes++] = tess_numVertexes + 1;
        tess_indexes[tess_numIndexes] = tess_numVertexes + 2;
        v13 = tess_numIndexes + 1;
        result = tess_numVertexes + 4;
        v22 = integer == 1;
        ++tess_numIndexes;
        tess_numVertexes += 4;
        --integer;
      }
      while ( !v22 );
    }
  }
  return result;
}
#endif

/* ---- RB_SurfaceRailRings  0x00511F10 ----  VERIFIED */
int RB_SurfaceRailRings()
{
  double v0;
  double v1;
  float v2;
  float v3;
  float v4;
  unsigned __int64 v5; // rax
  float v7;
  float v8[3]; // [ebp-0x30] BYREF  -- retail vec3_t
  float v11[3]; // [ebp-0x24] BYREF  -- retail vec3_t
  float v14[3]; // [esp+20h] [ebp-18h] BYREF
  float v15[3]; // [esp+2Ch] [ebp-Ch] BYREF

  v0 = *(float *)(backEnd_currentEntity + 68);
  v1 = *(float *)(backEnd_currentEntity + 72);
  v2 = *(float *)(backEnd_currentEntity + 88);
  v11[0] = *(float *)(backEnd_currentEntity + 84);
  v3 = *(float *)(backEnd_currentEntity + 92);
  v11[1] = v2;
  v4 = *(float *)(backEnd_currentEntity + 76);
  v8[0] = v0 - v11[0];
  v14[2] = v4;
  v11[2] = v3;
  v8[1] = v1 - v11[1];
  v8[2] = v4 - v3;
  v7 = VectorNormalize(v8);
  MakeNormalVectors(v8, v15, v14);
  v5 = (unsigned __int64)(v7 / r_railSegmentLength->value);
  if ( (int)v5 <= 0 )
    LODWORD(v5) = 1;
  v8[0] = v8[0] * r_railSegmentLength->value;
  v8[1] = v8[1] * r_railSegmentLength->value;
  v8[2] = v8[2] * r_railSegmentLength->value;
  return DoRailDiscs(v14, v15, v8, v5, v11);
}

/* ---- RB_SurfaceRailCore  0x00511FE0 ----  [HIGH] */
int RB_SurfaceRailCore()
{
  float v0;
  float v1;
  float v2;
  double v3;
  float v4;
  float v5;
  float v7;
  float v8[3]; // [ebp-0x3c] BYREF  -- retail vec3_t
  float v11[3]; // [ebp-0x30] BYREF  -- retail vec3_t
  float v14[3]; // [ebp-0x24] BYREF  -- retail vec3_t
  float v17[3]; // [ebp-0x18] BYREF  -- retail vec3_t
  float v20[3]; // [ebp-0xc] BYREF  -- retail vec3_t

  v0 = *(float *)(backEnd_currentEntity + 84);
  v17[1] = *(float *)(backEnd_currentEntity + 88);
  v1 = *(float *)(backEnd_currentEntity + 68);
  v17[0] = v0;
  v2 = *(float *)(backEnd_currentEntity + 92);
  v14[0] = v1;
  v3 = v1 - v17[0];
  v4 = *(float *)(backEnd_currentEntity + 76);
  v17[2] = v2;
  v5 = *(float *)(backEnd_currentEntity + 72);
  v20[0] = v3;
  v14[1] = v5;
  v14[2] = v4;
  v20[1] = v5 - v17[1];
  v20[2] = v4 - v17[2];
  v7 = VectorNormalize(v20);
  v11[0] = v17[0] - backEnd_viewParms_originX[0];
  v11[1] = v17[1] - backEnd_viewParms_originY;
  v11[2] = v17[2] - backEnd_viewParms_originZ;
  VectorNormalize(v11);
  v8[0] = v14[0] - backEnd_viewParms_originX[0];
  v8[1] = v14[1] - backEnd_viewParms_originY;
  v8[2] = v14[2] - backEnd_viewParms_originZ;
  VectorNormalize(v8);
  v20[0] = v8[2] * v11[1] - v8[1] * v11[2];
  v20[1] = v8[0] * v11[2] - v8[2] * v11[0];
  v20[2] = v8[1] * v11[0] - v8[0] * v11[1];
  VectorNormalize(v20);
  return DoRailCore(v14, v17, v20, v7, r_railCoreWidth->value);
}

/* ---- RB_SurfaceLightningBolt  0x00512130 ----  [HIGH] */
int RB_SurfaceLightningBolt()
{
  float v0;
  float v1;
  double v2;
  float v3;
  double v4;
  float v5;
  float v6;
  int v7;
  int result;
  float v9;
  float v10[3]; // [ebp-0x48] BYREF  -- retail vec3_t
  float v13[3]; // [ebp-0x3c] BYREF  -- retail vec3_t
  float v16[3]; // [ebp-0x30] BYREF  -- retail vec3_t
  float v19[3]; // [ebp-0x24] BYREF  -- retail vec3_t
  float v22[3]; // [ebp-0x18] BYREF  -- retail vec3_t
  float v25[3]; // [esp+40h] [ebp-Ch] BYREF

  v0 = *(float *)(backEnd_currentEntity + 84);
  v16[1] = *(float *)(backEnd_currentEntity + 88);
  v1 = *(float *)(backEnd_currentEntity + 68);
  v16[0] = v0;
  v2 = v0;
  v3 = *(float *)(backEnd_currentEntity + 92);
  v19[0] = v1;
  v4 = v2 - v1;
  v5 = *(float *)(backEnd_currentEntity + 76);
  v16[2] = v3;
  v6 = *(float *)(backEnd_currentEntity + 72);
  v25[0] = v4;
  v19[1] = v6;
  v19[2] = v5;
  v25[1] = v16[1] - v6;
  v25[2] = v16[2] - v5;
  v9 = VectorNormalize(v25);
  *(float *)v13 = v19[0] - backEnd_viewParms_originX[0];
  *(float *)&v13[1] = v19[1] - backEnd_viewParms_originY;
  v13[2] = v19[2] - backEnd_viewParms_originZ;
  VectorNormalize((float *)v13);
  v10[0] = v16[0] - backEnd_viewParms_originX[0];
  v10[1] = v16[1] - backEnd_viewParms_originY;
  v10[2] = v16[2] - backEnd_viewParms_originZ;
  VectorNormalize(v10);
  *(float *)v22 = v10[2] * *(float *)&v13[1] - v10[1] * v13[2];
  *(float *)&v22[1] = v10[0] * v13[2] - v10[2] * *(float *)v13;
  v22[2] = v10[1] * *(float *)v13 - v10[0] * *(float *)&v13[1];
  VectorNormalize((float *)v22);
  v7 = 4;
  do
  {
    DoRailCore(v16, v19, (float *)v22, v9, 8.0);
    RotatePointAroundVector((float *)v13, v25, (float *)v22, 45.0);
    result = *(int *)&v13[1];
    --v7;
    v22[0] = v13[0];
    v22[1] = v13[1];
    v22[2] = v13[2];
  }
  while ( v7 );
  return result;
}

/* ---- RB_SurfaceTriangles  0x005122B0 ----  [HIGH] */
int __cdecl RB_SurfaceTriangles(int a1)
{
  int v1;
  int v2;
  int v3;
  int v4;
  __int16 v5;
  _WORD *v6;
  int v7;
  int v8;
  int *v9;
  int *v10;
  int result;

  v1 = tess_numVertexes;
  v2 = *(_DWORD *)(a1 + 36);
  if ( v2 + tess_numVertexes >= 0x2000 || tess_numIndexes + *(_DWORD *)(a1 + 68) >= 49152 )
  {
    RB_CheckOverflow(v2, *(_DWORD *)(a1 + 68));
    v1 = tess_numVertexes;
  }
  tess_dlightBits |= *(_DWORD *)(a1 + 8);
  v3 = *(_DWORD *)(a1 + 68) - 1;
  v4 = 2 * tess_numIndexes + ((int)(char *)tess_indexes);
  v5 = v1;
  if ( v3 >= 0 )
  {
    v6 = (_WORD *)(v4 + 2 * v3);
    v7 = *(_DWORD *)(a1 + 68);
    do
    {
      *v6 = v5 + *(_WORD *)((char *)v6 + a1 + 72 - v4);
      --v6;
      --v7;
    }
    while ( v7 );
    v1 = tess_numVertexes;
  }
  tess_numIndexes += *(_DWORD *)(a1 + 68);
  v8 = *(_DWORD *)(a1 + 36);
  if ( (*(_BYTE *)(tess_shader + 80) & 2) != 0 || r_dlightQuality->integer && tess_dlightBits )
  {
    Com_Memcpy(12 * v1 + ((int)(char *)tess_stageNormals), *(int **)(a1 + 48), 12 * v8);
    tess_requiresVertexBasis = 1;
    if ( (*(_BYTE *)(tess_shader + 80) & 0xC) != 0 )
    {
      v9 = *(int **)(a1 + 40);
      if ( v9 )
      {
        Com_Memcpy(12 * tess_numVertexes + ((int)(char *)tess_stageTangents), v9, 12 * v8);
        tess_stageTangentsValid = 1;
      }
      v10 = *(int **)(a1 + 44);
      if ( v10 )
      {
        Com_Memcpy(12 * tess_numVertexes + ((int)(char *)tess_stageBitangents), v10, 12 * v8);
        tess_stageBitangentsValid = 1;
      }
    }
    v1 = tess_numVertexes;
  }
  Com_Memcpy(12 * v1 + ((int)(char *)tess_xyz), *(int **)(a1 + 64), 12 * v8);
  Com_Memcpy(8 * tess_numVertexes + ((int)(char *)tess_texCoords0), *(int **)(a1 + 52), 8 * v8);
  Com_Memcpy(8 * tess_numVertexes + ((int)(char *)tess_texCoords1), *(int **)(a1 + 56), 8 * v8);
  Com_Memcpy(4 * tess_numVertexes + ((int)(char *)tess_vertexColors), *(int **)(a1 + 60), 4 * v8);
  result = *(_DWORD *)(a1 + 36) + tess_numVertexes;
  tess_numVertexes = result;
  return result;
}

/* ---- RB_DlightFallback  0x00512490 ----  [HIGH] */
int __cdecl RB_DlightFallback(int a1)
{
  int v1;
  int v2;
  _WORD *v3;
  __int16 v4;
  int i;
  int result;

  v1 = *(_DWORD *)(a1 + 36);
  v2 = tess_numVertexes;
  if ( v1 + tess_numVertexes >= 0x2000 || tess_numIndexes + *(_DWORD *)(a1 + 68) >= 49152 )
  {
    RB_CheckOverflow(v1, *(_DWORD *)(a1 + 68));
    v2 = tess_numVertexes;
  }
  if ( r_dlightQuality->integer )
  {
    Com_Memcpy(12 * v2 + ((int)(char *)tess_stageTangents), *(int **)(a1 + 40), 12 * *(_DWORD *)(a1 + 36));
    Com_Memcpy(12 * tess_numVertexes + ((int)(char *)tess_stageBitangents), *(int **)(a1 + 44), 12 * *(_DWORD *)(a1 + 36));
    Com_Memcpy(12 * tess_numVertexes + ((int)(char *)tess_stageNormals), *(int **)(a1 + 48), 12 * *(_DWORD *)(a1 + 36));
    tess_stageTangentsValid = 1;
    tess_stageBitangentsValid = 1;
    tess_requiresVertexBasis = 1;
    v2 = tess_numVertexes;
  }
  Com_Memcpy(8 * v2 + ((int)(char *)tess_texCoords0), *(int **)(a1 + 52), 8 * *(_DWORD *)(a1 + 36));
  Com_Memcpy(8 * tess_numVertexes + ((int)(char *)tess_texCoords1), *(int **)(a1 + 56), 8 * *(_DWORD *)(a1 + 36));
  Com_Memcpy(4 * tess_numVertexes + ((int)(char *)tess_vertexColors), *(int **)(a1 + 60), 4 * *(_DWORD *)(a1 + 36));
  Com_Memcpy(12 * tess_numVertexes + ((int)(char *)tess_xyz), *(int **)(a1 + 64), 12 * *(_DWORD *)(a1 + 36));
  v3 = (_WORD *)(a1 + 72);
  v4 = tess_numVertexes - *(_WORD *)(a1 + 72);
  for ( i = 0; i < *(_DWORD *)(a1 + 68); ++v3 )
  {
    tess_indexes[i + tess_numIndexes] = v4 + *v3;
    ++i;
  }
  tess_numVertexes += *(_DWORD *)(a1 + 36);
  tess_numIndexes += *(_DWORD *)(a1 + 68);
  result = *(_DWORD *)(a1 + 8);
  tess_dlightBits |= result;
  return result;
}

extern int count;   /* tess.renderedIndexCount, 0x019BFFB4 -- see tr_shade.c */

/* ---- RB_SurfaceOptimized  0x00512660 ----  VERIFIED */
int __cdecl RB_SurfaceOptimized( _DWORD *a1 )
{
  int firstVertex;
  int lastVertex;
  int result;

  if ( count + a1[17] >= 0x20000 )
  {
    RB_EndSurface();
    backEnd_currentEntity = tess_entity;
    RB_BeginSurface( (void *)tess_shader, tess_vertexComponentCount );
  }

  firstVertex = *((unsigned short *)a1 + 36);          /* u16 at +0x48 */

  if ( tess_optimizedVertexEnd )
  {
    if ( tess_optimizedFirstVertex > firstVertex )
      tess_optimizedFirstVertex = firstVertex;
    lastVertex = a1[9] + firstVertex;                  /* +0x24 + base */
    if ( tess_optimizedVertexEnd < lastVertex )
      tess_optimizedVertexEnd = lastVertex;
  }
  else
  {
    tess_optimizedFirstVertex = firstVertex;
    tess_optimizedVertexEnd = a1[9] + firstVertex;
  }

  /* 0x005126E8-0x005126FC.  Destination is &tess.optimizedIndexes[count]. */
  Com_Memcpy( &tess_optimizedIndexes + 2 * count, (unsigned char *)( a1 + 18 ), 2 * a1[17] );

  count         += a1[17];
  tess_renderedVertexCount += a1[9];

  result = backEnd_refdef_num_dlights;
  if ( backEnd_refdef_num_dlights )
  {
    result = a1[2];
    if ( result )
    {
      result = glConfig_NVVertexArrayRange;
      if ( !glConfig_NVVertexArrayRange || storageClass == glState_currentStorageMode )
        return RB_DlightFallback( (int)a1 );
    }
  }
  return result;
}
#if 0
int __cdecl RB_SurfaceOptimized(_DWORD *a1)
{
  void *v1;
  unsigned __int16 *v2;
  int v3;
  int result;

  if ( count + a1[17] >= 0x20000 )
  {
    RB_EndSurface(v1);
    backEnd_currentEntity = tess_entity;
    RB_BeginSurface(tess_vertexComponentCount);
  }
  v2 = (unsigned __int16 *)(a1 + 18);
  if ( tess_optimizedVertexEnd )
  {
    if ( tess_optimizedFirstVertex > *((unsigned __int16 *)a1 + 36) )
      tess_optimizedFirstVertex = *((unsigned __int16 *)a1 + 36);
    v3 = a1[9];
    if ( tess_optimizedVertexEnd < v3 + *v2 )
      tess_optimizedVertexEnd = v3 + *v2;
  }
  else
  {
    tess_optimizedFirstVertex = *((unsigned __int16 *)a1 + 36);
    tess_optimizedVertexEnd = a1[9] + *v2;
  }
  Com_Memcpy((int)( &tess_optimizedIndexes + 2 * count ), a1 + 18, 2 * a1[17]);
  count += a1[17];
  result = backEnd_refdef_num_dlights;
  tess_renderedVertexCount += a1[9];
  if ( backEnd_refdef_num_dlights )
  {
    result = a1[2];
    if ( result )
    {
      result = glConfig_NVVertexArrayRange;
      if ( !glConfig_NVVertexArrayRange || storageClass == glState_currentStorageMode )
        return RB_DlightFallback((int)a1);
    }
  }
  return result;
}
#endif

/* ---- RB_SurfaceAxis  0x00512760 ----  VERIFIED */
void RB_SurfaceAxis()
{
  RB_BeginImmediateMode();
  GL_Bind((GLenum *)tr_whiteImage);
  if ( !((*(float *)&rbDebug_immediateLineWidth == 3.0) | __UNORDERED__(*(float *)&rbDebug_immediateLineWidth, 3.0)) )
  {
    rbDebug_immediateLineWidth = 1077936128;
    qglLineWidth(3.0f);
  }
  mode = 1;
  rbDebug_immediateColorR = -1;
  rbDebug_immediateColorG = 0;
  rbDebug_immediateColorB = 0;
  rbDebug_immediateColorA = -1;
  RB_glVertex3f(0, 0, 0);
  RB_glVertex3f(1098907648, 0, 0);
  rbDebug_immediateColorR = 0;
  rbDebug_immediateColorG = -1;
  rbDebug_immediateColorB = 0;
  rbDebug_immediateColorA = -1;
  RB_glVertex3f(0, 0, 0);
  RB_glVertex3f(0, 1098907648, 0);
  rbDebug_immediateColorR = 0;
  rbDebug_immediateColorG = 0;
  rbDebug_immediateColorB = -1;
  rbDebug_immediateColorA = -1;
  RB_glVertex3f(0, 0, 0);
  RB_glVertex3f(0, 0, 1098907648);
  glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
  rbDebug_immediateVertexCount = 0;
  mode = 0;
  if ( !((*(float *)&rbDebug_immediateLineWidth == 1.0) | __UNORDERED__(*(float *)&rbDebug_immediateLineWidth, 1.0)) )
  {
    rbDebug_immediateLineWidth = 1065353216;
    qglLineWidth(1.0f);
  }
  rbDebug_immediateModeActive = 0;
  R_FogOn();
  glVertexPointer(3, 0x1406u, 0, tess_xyz);
}

/* ---- RB_SurfaceEntity  0x005128A0 ----  VERIFIED */
void RB_SurfaceEntity()
{
  switch ( *(_DWORD *)backEnd_currentEntity )
  {
    case 4:
      RB_SurfaceSprite();
      break;
    case 5:
      RB_SurfaceSplash();
      break;
    case 6:
      RB_SurfaceBeam();
      break;
    case 7:
      RB_SurfaceRailCore();
      break;
    case 9:
      RB_SurfaceRailRings();
      break;
    case 0xA:
      RB_SurfaceLightningBolt();
      break;
    case 0xC:
      RB_SurfaceOrientedQuad();
      break;
    case 0xD:
      RB_SurfaceLine();
      break;
    case 0xE:
      ri_Printf(0, "FXPORT RT_ELECTRICITY TDB\n");
      break;
    case 0xF:
      RB_SurfaceCylinder();
      break;
    default:
      RB_SurfaceAxis();
      break;
  }
}

/* ---- RB_SurfaceBad  0x00512960 ----  VERIFIED */
void RB_SurfaceBad()
{
  ri_Printf(0, "Bad surface tesselated.\n");
}

/* ---- RB_SurfaceSkip  0x00512980 ----  VERIFIED */
void RB_SurfaceSkip()
{
}

