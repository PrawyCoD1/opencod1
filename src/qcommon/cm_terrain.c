/*
 * qcommon/cm_terrain.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_terrain.c
 *
 * Retail range 0x00420000-0x00421BE0, 9 functions.
 *
 * @fidelity: likely
 */

#include "../qcommon/cm_patch_local.h"
#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

extern void *Hunk_AllocAlignInternal( int size, int align );
extern void PerpendicularVector( vec3_t dst, const vec3_t src );
qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b,
						  const vec3_t c );

typedef struct cmTerrainSphere_t
{
	qboolean use;                   /* +0x0F4 */
	float radius;                   /* +0x0F8 */
	float halfheight;               /* +0x0FC */
	vec3_t offset;                  /* +0x100 */
} cmTerrainSphere_t;

typedef struct cmTerrainTrace_t
{
	vec3_t start;                   /* +0x000 */
	vec3_t end;                     /* +0x00C */
	vec3_t delta;                   /* +0x018 */
	float deltaLengthSquared;       /* +0x024 */
	vec3_t mins;                    /* +0x028 */
	vec3_t maxs;                    /* +0x034 */
	vec3_t offsets[8];              /* +0x040 */
	byte _pad_A0[0x0C0 - 0x0A0];
	qboolean isPoint;               /* +0x0C0 */
	trace_t trace;                  /* +0x0C4 */
	cmTerrainSphere_t sphere;       /* +0x0F4 */
	vec3_t sphereExtents;           /* +0x10C */
} cmTerrainTrace_t;
COD1_ASSERT_SIZE( cmTerrainTrace_t, 280 );

#define CM_TW( p )  ( ( cmTerrainTrace_t * ) (void *) (p) )

typedef struct terrainPlane_t
{
	vec3_t normal;                  /* +0x00 */
	float dist;                     /* +0x0C */
} terrainPlane_t;
COD1_ASSERT_SIZE( terrainPlane_t, 16 );

typedef struct terrainVertexSphere_t
{
	int checkcount;                 /* +0x00 */
	vec3_t origin;                  /* +0x04 */
} terrainVertexSphere_t;
COD1_ASSERT_SIZE( terrainVertexSphere_t, 16 );

typedef struct terrainEdgeCylinder_t
{
	int checkcount;                 /* +0x00 */
	vec3_t origin;                  /* +0x04 */
	vec3_t radialAxes[2];           /* +0x10 */
	vec3_t axis;                    /* +0x28 */
	float length;                   /* +0x34 */
} terrainEdgeCylinder_t;
COD1_ASSERT_SIZE( terrainEdgeCylinder_t, 56 );

typedef struct terrainFacet_t
{
	terrainPlane_t surfacePlane;                    /* +0x00 */
	terrainPlane_t edgePlanes[2];                   /* +0x10 */
	terrainVertexSphere_t   *vertexSpheres[3];      /* +0x30 */
	terrainEdgeCylinder_t   *edgeCylinders[3];      /* +0x3C */
} terrainFacet_t;
COD1_ASSERT_SIZE( terrainFacet_t, 72 );

typedef struct terrainCollide_t
{
	unsigned short facetCount;      /* +0x00 */
	byte capsuleOffsetSign;         /* +0x02 */
	byte unk03;                     /* +0x03 */
} terrainCollide_t;
COD1_ASSERT_SIZE( terrainCollide_t, 4 );

#define CM_TC( p )          ( ( const terrainCollide_t * ) (const void *) (p) )
#define CM_TC_FACETS( p )   ( ( const terrainFacet_t * ) (const void *) \
							  ( ( const char * ) (p) + 4 ) )
#define CM_TC_FACETS_W( p ) ( ( terrainFacet_t * ) (void *) \
							  ( ( char * ) (p) + 4 ) )

#define TERRAIN_CLIP_EPSILON    0.125f
#define TERRAIN_BARY_MIN        ( -0.001f )
#define TERRAIN_BARY_MAX        1.001f
/* 0x00568FA4 -- the sphere walker backs every accepted fraction off by this. */
#define TERRAIN_HIT_EPSILON     0.00001f

#define CM_CHECKCOUNT           cm_checkcount

/* ---- FindEdge  0x00420000 ---- VERIFIED */
int __cdecl FindEdge(int *edges, int numEdges, __int16 p1, __int16 p2)
{
  int result;
  __int16 v5;

  result = 0;
  if ( numEdges <= 0 )
    return -1;
  while ( 1 )
  {
    v5 = edges[result];
    if ( v5 == p2 && HIWORD(edges[result]) == p1 )
      break;
    if ( v5 == p1 && HIWORD(edges[result]) == p2 )
      break;
    if ( ++result >= numEdges )
      return -1;
  }
  return result;
}

/* ---- CM_GenerateTerrainCollide  0x00420030 ---- VERIFIED */
void *__cdecl CM_GenerateTerrainCollide(int numIndexes, const short *indexes,
                                        int numVerts, const vec3_t *pointArray,
                                        vec3_t *bounds)
{
  const int width    = numIndexes;
  const int height   = (int)indexes;
  const int points   = numVerts;
  const int contents = (int)pointArray;

  int v4;
  __int16 v5;
  unsigned __int16 v6;
  bool v7;
  int v8;
  __int16 v9;
  __int16 v10;
  double v11;
  float *v12;
  float *v13;
  float *v14;
  double v15;
  int v16;
  float *v17;
  double v18;
  __int16 v19;
  float v20;
  __int16 v21;
  _WORD *v22;
  int v23;
  int i;
  int v25;
  bool v26;
  _WORD *v27;
  int v28;
  _WORD *v29;
  __int16 v30;
  _WORD *v31;
  float *v32;
  unsigned __int16 v33;
  unsigned __int16 v34;
  float *v35;
  float *v36;
  double v37;
  double v38;
  double v39;
  double v40;
  __int16 v41;
  double v42;
  char *v43;
  double v44;
  char *v45;
  __int16 v46;
  __int16 v47;
  char *v48;
  __int16 v49;
  __int16 v50;
  char *v51;
  int v52;
  __int16 v53;
  char *v54;
  __int16 v55;
  int v56;
  __int16 v57;
  char *v58;
  int v59;
  __int16 v60;
  char *v61;
  unsigned __int16 *v62;
  __int16 *v63;
  bool v64;
  char v65;
  float *v66;
  char *v67;
  int v68;
  float *v69;
  __int16 v70;
  char *v71;
  __int16 v72;
  char *v73;
  __int16 v74;
  char *v75;
  __int16 v76;
  char *v77;
  float *v78;
  __int16 v79;
  char *v80;
  int v81;
  float *v82;
  int v83;
  int v84;
  int v85;
  int v86;
  float *v87;
  float *v88;
  float *v89;
  double v90;
  double v91;
  __int16 *v93;
  float v94;
  float v95;
  int v96;
  float v97;
  float v98;
  float v99;
  float v100;
  char v101;
  char v102;
  int v103;
  int v104;
  int v105;
  int v106;
  float vec3a[3];
  float v110;
  float vec3b[3];
  int v114;
  float v115;
  int v116;
  float v117;
  __int16 *v118;
  float v119;
  char *v120;
  char *v121;
  float v122;
  float v123;
  float v124;
  float v125;
  float v126;
  float v127;
  float v128;
  float v129;
  float v130;
  float v131;
  float v132;
  float v133;
  float v134;
  float v135;
  _WORD vertMap[0x10000];
  _WORD edgeVerts[0x10000][2];
  unsigned int v137;
  unsigned int retaddr;
  float *v139 = &bounds[0][0];

  v137 = retaddr ^ _security_cookie;
  LODWORD(v110) = width / 3;
  if ( width % 3 )
    Com_Error(ERR_DROP, "\x15" "CM_GenerateTerrainCollide: numIndexes % 3 != 0, corrupt bsp?");
  if ( (unsigned int)points >= 0x10000 )
    Com_Error(ERR_DROP, "\x15" "CM_GenerateTerrainCollide: too many vertices");
  v103 = 0;
  v104 = 0;
  if ( width > 0 )
  {
    v4 = height;
    do
    {
      v114 = 2;
      v93 = (__int16 *)v4;
      v96 = 3;
      do
      {
        v5 = *v93;
        v6 = *(_WORD *)(height + 2 * (v104 + (v114 - 1) % 3));
        v7 = *v93 == (__int16)v6;
        LOWORD(v118) = *(_WORD *)(height + 2 * (v104 + v114 % 3));
        if ( v7 )
          Com_Error(ERR_DROP, "\x15" "CM_GenerateTerrainCollide: degenerate triangle, corrupt bsp?");
        v8 = 0;
        if ( v103 <= 0 )
          goto LABEL_18;
        while ( 1 )
        {
          v9 = edgeVerts[v8][0];
          if ( v9 == v6 && edgeVerts[v8][1] == v5 )
            break;
          if ( v9 == v5 && edgeVerts[v8][1] == v6 )
            break;
          if ( ++v8 >= v103 )
            goto LABEL_18;
        }
        if ( v8 >= 0 )
        {
          v11 = *(float *)(contents + 12 * v5);
          v12 = (float *)(contents + 12 * v5);
          v13 = (float *)(contents + 12 * (__int16)v118);
          v14 = (float *)(contents + 12 * (__int16)v6);
          v127 = v11 - *v13;
          v15 = v12[1];
          LODWORD(v115) = &vertMap[v8];
          v16 = (__int16)*(_WORD *)LODWORD(v115);
          v128 = v15 - v13[1];
          v17 = (float *)(contents + 12 * v16);
          v129 = v12[2] - v13[2];
          vec3b[0] = *v14 - *v13;
          vec3b[1] = v14[1] - v13[1];
          vec3b[2] = v14[2] - v13[2];
          v133 = *v17 - *v13;
          v134 = v17[1] - v13[1];
          v135 = v17[2] - v13[2];
          vec3a[0] = vec3b[1] * v129 - vec3b[2] * v128;
          vec3a[1] = vec3b[2] * v127 - vec3b[0] * v129;
          vec3a[2] = vec3b[0] * v128 - vec3b[1] * v127;
          *(float *)&v116 = vec3a[2] * v135 + vec3a[1] * v134 + vec3a[0] * v133;
          if ( *(float *)&v116 > 0.0 )
          {
            v124 = *v17 - *v12;
            v125 = v17[1] - v12[1];
            v126 = v17[2] - v12[2];
            v122 = v126 * v126 + v125 * v125 + v124 * v124;
            v130 = *v17 - *v14;
            v131 = v17[1] - v14[1];
            v132 = v17[2] - v14[2];
            v123 = v132 * v132 + v131 * v131 + v130 * v130;
            if ( v122 <= (double)v123 )
              v117 = v123;
            else
              v117 = v122;
            v18 = vec3a[2] * vec3a[2] + vec3a[1] * vec3a[1] + vec3a[0] * vec3a[0];
            if ( (v18 < v117 * 0.0000000064000001) | __UNORDERED__(v18, v117 * 0.0000000064000001) )
            {
              edgeVerts[v8][0] = edgeVerts[v103 - 1][0];
              v19 = edgeVerts[v103 - 1][1];
              v20 = v115;
              v21 = vertMap[--v103];
              edgeVerts[v8][1] = v19;
              *(_WORD *)LODWORD(v20) = v21;
            }
          }
        }
        else
        {
LABEL_18:
          if ( (unsigned int)v103 >= 0x10000 )
            Com_Error(ERR_DROP, "\x15" "CM_GenerateTerrainCollide: too many edges");
          v10 = (__int16)v118;
          edgeVerts[v103][0] = v5;
          edgeVerts[v103][1] = v6;
          vertMap[v103++] = v10;
        }
        v22 = v93 + 1;
        v7 = v96 == 1;
        ++v93;
        ++v114;
        --v96;
      }
      while ( !v7 );
      v4 = (int)v22;
      v104 += 3;
    }
    while ( v104 < width );
  }
  memset(vertMap, 0xFF, sizeof(vertMap));
  v23 = 0;
  for ( i = 0; i < v103; ++i )
  {
    v25 = (__int16)edgeVerts[i][0];
    v26 = (__int16)vertMap[v25] < 0;
    v27 = &vertMap[v25];
    if ( v26 )
      *v27 = v23++;
    v28 = (__int16)edgeVerts[i][1];
    v26 = (__int16)vertMap[v28] < 0;
    v29 = &vertMap[v28];
    if ( v26 )
      *v29 = v23++;
  }
  v30 = LOWORD(v110);
  v31 = Hunk_AllocAlignInternal(72 * LODWORD(v110) + 4, 32);
  v114 = (int)v31;
  v120 = (char *)Hunk_AllocAlignInternal(16 * v23, 32);
  v121 = (char *)Hunk_AllocAlignInternal(56 * v103, 32);
  v102 = 0;
  v101 = 0;
  *v31 = LOWORD(v110);
  v105 = 0;
  if ( !v30 )
    goto LABEL_81;
  v32 = (float *)(v31 + 6);
  v118 = (__int16 *)(height + 4);
  do
  {
    v33 = *(v118 - 1);
    v34 = *(v118 - 2);
    LODWORD(v122) = *v118;
    v116 = v33;
    v35 = (float *)(contents + 12 * (__int16)v33);
    LODWORD(v117) = v34;
    v36 = (float *)(contents + 12 * (__int16)v34);
    LODWORD(v115) = contents + 12 * SLOWORD(v122);
    PlaneFromPoints( v32 - 2, v36, v35, (float *)LODWORD(v115) );
    if ( (*v32 < -0.001) | __UNORDERED__(*v32, -0.001) )
    {
      v102 = 1;
    }
    else if ( *v32 > 0.001 )
    {
      v101 = 1;
    }
    vec3b[0] = *v35 - *v36;
    vec3b[1] = v35[1] - v36[1];
    vec3b[2] = v35[2] - v36[2];
    v110 = VectorNormalize(vec3b);
    vec3a[0] = *(float *)LODWORD(v115) - *v36;
    vec3a[1] = *(float *)(LODWORD(v115) + 4) - v36[1];
    vec3a[2] = *(float *)(LODWORD(v115) + 8) - v36[2];
    v115 = VectorNormalize(vec3a);
    v97 = vec3a[2] * vec3b[2] + vec3a[1] * vec3b[1] + vec3a[0] * vec3b[0];
    v94 = 1.0 / (1.0 - v97 * v97);
    vec3b[0] = v94 * vec3b[0];
    vec3b[1] = v94 * vec3b[1];
    vec3b[2] = v94 * vec3b[2];
    vec3a[0] = v94 * vec3a[0];
    vec3a[1] = v94 * vec3a[1];
    vec3a[2] = v94 * vec3a[2];
    v95 = -v97;
    v98 = 1.0 / v110;
    v110 = (v95 * vec3a[0] + vec3b[0]) * v98;
    v37 = v95 * vec3a[1];
    v32[2] = v110;
    v119 = (v37 + vec3b[1]) * v98;
    v38 = v95 * vec3a[2];
    v32[3] = v119;
    v99 = (v38 + vec3b[2]) * v98;
    v39 = v119;
    v32[4] = v99;
    v32[5] = v39 * v36[1] + v99 * v36[2] + v110 * *v36;
    v100 = 1.0 / v115;
    v115 = (v95 * vec3b[0] + vec3a[0]) * v100;
    v40 = v95 * vec3b[1];
    v32[6] = v115;
    v41 = vertMap[SLOWORD(v117)];
    v110 = (v40 + vec3a[1]) * v100;
    v42 = v95 * vec3b[2];
    v32[7] = v110;
    v43 = v120;
    v119 = (v42 + vec3a[2]) * v100;
    v44 = v110;
    v32[8] = v119;
    v32[9] = v44 * v36[1] + v119 * v36[2] + v115 * *v36;
    if ( v41 < 0 )
      v45 = 0;
    else
      v45 = &v43[16 * v41];
    v46 = v116;
    *((_DWORD *)v32 + 10) = v45;
    v47 = vertMap[v46];
    if ( v47 < 0 )
      v48 = 0;
    else
      v48 = &v43[16 * v47];
    v49 = LOWORD(v122);
    *((_DWORD *)v32 + 11) = v48;
    v50 = vertMap[v49];
    if ( v50 < 0 )
      v51 = 0;
    else
      v51 = &v43[16 * v50];
    *((_DWORD *)v32 + 12) = v51;
    v52 = 0;
    if ( v103 <= 0 )
      goto LABEL_56;
    while ( 1 )
    {
      v53 = edgeVerts[v52][0];
      if ( v53 == v49 && edgeVerts[v52][1] == v46 )
        break;
      if ( v53 == v46 && edgeVerts[v52][1] == v49 )
        break;
      if ( ++v52 >= v103 )
        goto LABEL_56;
    }
    if ( v52 < 0 )
LABEL_56:
      v54 = 0;
    else
      v54 = &v121[56 * v52];
    v55 = LOWORD(v117);
    *((_DWORD *)v32 + 13) = v54;
    v56 = 0;
    if ( v103 <= 0 )
      goto LABEL_63;
    while ( 1 )
    {
      v57 = edgeVerts[v56][0];
      if ( v57 == v55 && edgeVerts[v56][1] == v49 )
        break;
      if ( v57 == v49 && edgeVerts[v56][1] == v55 )
        break;
      if ( ++v56 >= v103 )
        goto LABEL_63;
    }
    if ( v56 < 0 )
LABEL_63:
      v58 = 0;
    else
      v58 = &v121[56 * v56];
    *((_DWORD *)v32 + 14) = v58;
    v59 = 0;
    if ( v103 <= 0 )
      goto LABEL_70;
    while ( 1 )
    {
      v60 = edgeVerts[v59][0];
      if ( v60 == (_WORD)v116 && edgeVerts[v59][1] == v55 )
        break;
      if ( v60 == v55 && edgeVerts[v59][1] == (_WORD)v116 )
        break;
      if ( ++v59 >= v103 )
        goto LABEL_70;
    }
    if ( v59 < 0 )
LABEL_70:
      v61 = 0;
    else
      v61 = &v121[56 * v59];
    v62 = (unsigned __int16 *)v114;
    v63 = v118;
    *((_DWORD *)v32 + 15) = v61;
    v32 += 18;
    v64 = ++v105 < *v62;
    v118 = v63 + 3;
  }
  while ( v64 );
  if ( v102 && !v101 )
    v65 = 1;
  else
LABEL_81:
    v65 = 0;
  v66 = (float *)points;
  v67 = v120;
  *(_BYTE *)(v114 + 2) = v65;
  v139[2] = 262144.0;
  v139[1] = 262144.0;
  *v139 = 262144.0;
  v68 = 0;
  v139[5] = -262144.0;
  v139[4] = -262144.0;
  v139[3] = -262144.0;
  if ( (int)points >= 4 )
  {
    v116 = 3;
    v69 = (float *)(contents + 20);
    do
    {
      if ( (*(v69 - 5) < (double)*v139) | __UNORDERED__(*(v69 - 5), *v139) )
        *v139 = *(v69 - 5);
      if ( *(v69 - 5) > (double)v139[3] )
        v139[3] = *(v69 - 5);
      if ( (*(v69 - 4) < (double)v139[1]) | __UNORDERED__(*(v69 - 4), v139[1]) )
        v139[1] = *(v69 - 4);
      if ( *(v69 - 4) > (double)v139[4] )
        v139[4] = *(v69 - 4);
      if ( (*(v69 - 3) < (double)v139[2]) | __UNORDERED__(*(v69 - 3), v139[2]) )
        v139[2] = *(v69 - 3);
      if ( *(v69 - 3) > (double)v139[5] )
        v139[5] = *(v69 - 3);
      v70 = vertMap[v68];
      if ( v70 >= 0 )
      {
        v71 = &v67[16 * v70];
        *(_DWORD *)v71 = 0;
        *((float *)v71 + 1) = *(v69 - 5);
        *((float *)v71 + 2) = *(v69 - 4);
        *((float *)v71 + 3) = *(v69 - 3);
      }
      if ( (*(v69 - 2) < (double)*v139) | __UNORDERED__(*(v69 - 2), *v139) )
        *v139 = *(v69 - 2);
      if ( *(v69 - 2) > (double)v139[3] )
        v139[3] = *(v69 - 2);
      if ( (*(v69 - 1) < (double)v139[1]) | __UNORDERED__(*(v69 - 1), v139[1]) )
        v139[1] = *(v69 - 1);
      if ( *(v69 - 1) > (double)v139[4] )
        v139[4] = *(v69 - 1);
      if ( (*v69 < (double)v139[2]) | __UNORDERED__(*v69, v139[2]) )
        v139[2] = *v69;
      if ( *v69 > (double)v139[5] )
        v139[5] = *v69;
      v72 = vertMap[v68 + 1];
      if ( v72 >= 0 )
      {
        v73 = &v67[16 * v72];
        *(_DWORD *)v73 = 0;
        *((float *)v73 + 1) = *(v69 - 2);
        *((float *)v73 + 2) = *(v69 - 1);
        *((float *)v73 + 3) = *v69;
      }
      if ( (v69[1] < (double)*v139) | __UNORDERED__(v69[1], *v139) )
        *v139 = v69[1];
      if ( v69[1] > (double)v139[3] )
        v139[3] = v69[1];
      if ( (v69[2] < (double)v139[1]) | __UNORDERED__(v69[2], v139[1]) )
        v139[1] = v69[2];
      if ( v69[2] > (double)v139[4] )
        v139[4] = v69[2];
      if ( (v69[3] < (double)v139[2]) | __UNORDERED__(v69[3], v139[2]) )
        v139[2] = v69[3];
      if ( v69[3] > (double)v139[5] )
        v139[5] = v69[3];
      v74 = vertMap[v68 + 2];
      if ( v74 >= 0 )
      {
        v75 = &v67[16 * v74];
        *(_DWORD *)v75 = 0;
        *((float *)v75 + 1) = v69[1];
        *((float *)v75 + 2) = v69[2];
        *((float *)v75 + 3) = v69[3];
      }
      if ( (v69[4] < (double)*v139) | __UNORDERED__(v69[4], *v139) )
        *v139 = v69[4];
      if ( v69[4] > (double)v139[3] )
        v139[3] = v69[4];
      if ( (v69[5] < (double)v139[1]) | __UNORDERED__(v69[5], v139[1]) )
        v139[1] = v69[5];
      if ( v69[5] > (double)v139[4] )
        v139[4] = v69[5];
      if ( (v69[6] < (double)v139[2]) | __UNORDERED__(v69[6], v139[2]) )
        v139[2] = v69[6];
      if ( v69[6] > (double)v139[5] )
        v139[5] = v69[6];
      v76 = vertMap[v68 + 3];
      if ( v76 >= 0 )
      {
        v77 = &v67[16 * v76];
        *(_DWORD *)v77 = 0;
        *((float *)v77 + 1) = v69[4];
        *((float *)v77 + 2) = v69[5];
        *((float *)v77 + 3) = v69[6];
      }
      v66 = (float *)points;
      v68 += 4;
      v69 += 12;
      v116 += 4;
    }
    while ( v116 < (int)points );
  }
  if ( v68 < (int)v66 )
  {
    v78 = (float *)(contents + 12 * v68 + 8);
    do
    {
      if ( (*(v78 - 2) < (double)*v139) | __UNORDERED__(*(v78 - 2), *v139) )
        *v139 = *(v78 - 2);
      if ( *(v78 - 2) > (double)v139[3] )
        v139[3] = *(v78 - 2);
      if ( (*(v78 - 1) < (double)v139[1]) | __UNORDERED__(*(v78 - 1), v139[1]) )
        v139[1] = *(v78 - 1);
      if ( *(v78 - 1) > (double)v139[4] )
        v139[4] = *(v78 - 1);
      if ( (*v78 < (double)v139[2]) | __UNORDERED__(*v78, v139[2]) )
        v139[2] = *v78;
      if ( *v78 > (double)v139[5] )
        v139[5] = *v78;
      v79 = vertMap[v68];
      if ( v79 >= 0 )
      {
        v80 = &v67[16 * v79];
        *(_DWORD *)v80 = 0;
        *((float *)v80 + 1) = *(v78 - 2);
        *((float *)v80 + 2) = *(v78 - 1);
        *((float *)v80 + 3) = *v78;
      }
      ++v68;
      v78 += 3;
    }
    while ( v68 < (int)points );
  }
  v81 = 0;
  v106 = 0;
  if ( v103 > 0 )
  {
    v82 = (float *)(v121 + 24);
    while ( 1 )
    {
      v83 = (__int16)edgeVerts[v81][0];
      v84 = (__int16)edgeVerts[v81][1];
      *(v82 - 6) = 0.0;
      v83 *= 3;
      v85 = *(_DWORD *)(contents + 4 * v83);
      v86 = contents + 4 * v83;
      *((_DWORD *)v82 - 5) = v85;
      *(v82 - 4) = *(float *)(v86 + 4);
      *(v82 - 3) = *(float *)(v86 + 8);
      v87 = (float *)(contents + 12 * v84);
      v88 = v82 + 4;
      v82[4] = *v87 - *(float *)v86;
      v82[5] = v87[1] - *(float *)(v86 + 4);
      v82[6] = v87[2] - *(float *)(v86 + 8);
      v89 = v82 - 2;
      v82[7] = VectorNormalize(v82 + 4);
      PerpendicularVector(v82 - 2, v82 + 4);
      v90 = v82[5] * *v82;
      v91 = v82[6] * *(v82 - 1);
      v82 += 14;
      v64 = ++v106 < v103;
      *(v82 - 13) = v90 - v91;
      *(v82 - 12) = *(v82 - 8) * *v89 - *(v82 - 14) * *v88;
      *(v82 - 11) = *(v82 - 15) * *v88 - *(v82 - 9) * *v89;
      if ( !v64 )
        break;
      v81 = v106;
    }
  }
  return (void *)v114;
}

/* ---- CM_TracePointThroughTerrainCollide  0x00420D50 ---- VERIFIED */
void __cdecl CM_TracePointThroughTerrainCollide(traceWork_t *twOpaque, void *tc)
{
	cmTerrainTrace_t *tw = CM_TW( twOpaque );
	const terrainFacet_t *facets = CM_TC_FACETS( tc );
	int numFacets = ( int ) CM_TC( tc )->facetCount;
	int i;

	for ( i = 0; i < numFacets; i++ ) {
		const terrainFacet_t *facet = &facets[i];
		const terrainPlane_t *surf = &facet->surfacePlane;
		float d1, d2, enterFrac, hitFrac, e0, e1;
		vec3_t hit;

		d2 = ( tw->end[0] * surf->normal[0] + tw->end[1] * surf->normal[1] )
			 + tw->end[2] * surf->normal[2] - surf->dist;
		if ( d2 >= 0.0f ) {
			continue;
		}

		d1 = ( tw->start[0] * surf->normal[0] + tw->start[1] * surf->normal[1] )
			 + tw->start[2] * surf->normal[2] - surf->dist;
		if ( 0.0f >= d1 ) {
			continue;
		}

		enterFrac = ( d1 - TERRAIN_CLIP_EPSILON ) / ( d1 - d2 );
		if ( enterFrac >= tw->trace.fraction ) {
			continue;
		}

		hitFrac = d1 / ( d1 - d2 );
		hit[0] = tw->start[0] + tw->delta[0] * hitFrac;
		hit[1] = tw->start[1] + tw->delta[1] * hitFrac;
		hit[2] = tw->start[2] + tw->delta[2] * hitFrac;

		e0 = ( hit[0] * facet->edgePlanes[0].normal[0]
			   + hit[1] * facet->edgePlanes[0].normal[1] )
			 + hit[2] * facet->edgePlanes[0].normal[2]
			 - facet->edgePlanes[0].dist;
		if ( TERRAIN_BARY_MIN > e0 || e0 > TERRAIN_BARY_MAX ) {
			continue;
		}

		e1 = ( hit[0] * facet->edgePlanes[1].normal[0]
			   + hit[1] * facet->edgePlanes[1].normal[1] )
			 + hit[2] * facet->edgePlanes[1].normal[2]
			 - facet->edgePlanes[1].dist;
		if ( TERRAIN_BARY_MIN > e1 || e0 + e1 > TERRAIN_BARY_MAX ) {
			continue;
		}

		tw->trace.fraction = enterFrac;
		VectorCopy( surf->normal, tw->trace.normal );
	}
}

/* ---- CM_TraceSphereThroughTerrainCollide  0x00420EC0 ---- */
void __cdecl CM_TraceSphereThroughTerrainCollide( traceWork_t *twOpaque,
												 void *tcVoid )
{
	cmTerrainTrace_t *tw = CM_TW( twOpaque );
	const terrainCollide_t *tc = CM_TC( tcVoid );
	terrainFacet_t *facets = CM_TC_FACETS_W( tcVoid );
	int numFacets = ( int ) tc->facetCount;
	float capOffset = tw->sphere.halfheight - tw->sphere.radius;
	float capDelta, radius, negRadius;
	int i;

	if ( tc->capsuleOffsetSign ) {
		capDelta = capOffset * -2.0f;
		tw->start[2] += capOffset;
		tw->end[2] += capOffset;
	} else {
		capDelta = capOffset + capOffset;
		tw->start[2] -= capOffset;
		tw->end[2] -= capOffset;
	}

	radius = tw->sphere.radius + TERRAIN_CLIP_EPSILON;
	negRadius = -radius;

	for ( i = 0; i < numFacets; i++ ) {
		terrainFacet_t *facet = &facets[i];
		const terrainPlane_t *surf = &facet->surfacePlane;
		float d1, d2, delta;

		d2 = ( tw->end[0] * surf->normal[0] + tw->end[1] * surf->normal[1] )
			 + tw->end[2] * surf->normal[2] - surf->dist;
		if ( d2 >= radius ) {
			continue;
		}

		d1 = ( tw->start[0] * surf->normal[0] + tw->start[1] * surf->normal[1] )
			 + tw->start[2] * surf->normal[2] - surf->dist;
		delta = d1 - d2;
		if ( 0.0f >= delta ) {
			continue;
		}

		if ( !( d1 > negRadius ) ) {
			float capDist = capDelta * surf->normal[2] + d1;
			float invNz, off, baseE0, baseE1, e0, e1;

			if ( !( capDist > negRadius ) ) {
				continue;
			}

			invNz = 1.0f / surf->normal[2];
			off = ( negRadius - d1 ) * invNz;

			baseE0 = ( tw->start[0] * facet->edgePlanes[0].normal[0]
					   + tw->start[1] * facet->edgePlanes[0].normal[1] )
					 + tw->start[2] * facet->edgePlanes[0].normal[2]
					 - facet->edgePlanes[0].dist;
			baseE1 = ( tw->start[0] * facet->edgePlanes[1].normal[0]
					   + tw->start[1] * facet->edgePlanes[1].normal[1] )
					 + tw->start[2] * facet->edgePlanes[1].normal[2]
					 - facet->edgePlanes[1].dist;

			e0 = off * facet->edgePlanes[0].normal[2] + baseE0;
			if ( e0 >= 0.0f ) {
				e1 = off * facet->edgePlanes[1].normal[2] + baseE1;
				if ( e1 >= 0.0f && e0 + e1 <= 1.0f ) {
					goto startSolid;
				}
			}

			if ( capDist < radius ) {
				off = capDelta;
			} else {
				off = ( radius - d1 ) * invNz;
			}

			e0 = off * facet->edgePlanes[0].normal[2] + baseE0;
			if ( !( e0 < 0.0f ) ) {
				e1 = off * facet->edgePlanes[1].normal[2] + baseE1;
				if ( !( e1 < 0.0f ) && e0 + e1 <= 1.0f ) {
					goto startSolid;
				}
			}
			continue;

startSolid:
			VectorCopy( surf->normal, tw->trace.normal );
			tw->trace.fraction = 0.0f;
			tw->trace.startsolid = 1;
			return;
		} else {
			float hitFrac, e0, e1;
			vec3_t hit;
			unsigned int outside;
			int e;

			if ( !( d1 >= radius ) ) {
				hitFrac = 0.0f;
				VectorCopy( tw->start, hit );
			} else {
				hitFrac = ( d1 - radius ) / delta;
				if ( hitFrac > tw->trace.fraction ) {
					continue;
				}
				hit[0] = tw->start[0] + tw->delta[0] * hitFrac;
				hit[1] = tw->start[1] + tw->delta[1] * hitFrac;
				hit[2] = tw->start[2] + tw->delta[2] * hitFrac;
			}

			e0 = ( hit[0] * facet->edgePlanes[0].normal[0]
				   + hit[1] * facet->edgePlanes[0].normal[1] )
				 + hit[2] * facet->edgePlanes[0].normal[2]
				 - facet->edgePlanes[0].dist;
			e1 = ( hit[0] * facet->edgePlanes[1].normal[0]
				   + hit[1] * facet->edgePlanes[1].normal[1] )
				 + hit[2] * facet->edgePlanes[1].normal[2]
				 - facet->edgePlanes[1].dist;

			outside = 0;
			if ( e0 + e1 > 1.0f ) {
				outside |= 1;
			}
			if ( e0 < 0.0f ) {
				outside |= 2;
			}
			if ( e1 < 0.0f ) {
				outside |= 4;
			}

			if ( !outside ) {
				VectorCopy( surf->normal, tw->trace.normal );
				if ( hitFrac <= TERRAIN_HIT_EPSILON ) {
					tw->trace.fraction = 0.0f;
					tw->trace.startsolid = 1;
					return;
				}
				tw->trace.fraction = hitFrac - TERRAIN_HIT_EPSILON;
				continue;
			}

			for ( e = 0; e < 3; e++ ) {
				if ( ( outside >> e ) & 1 ) {
					terrainEdgeCylinder_t *cyl = facet->edgeCylinders[e];
					vec3_t d;
					float radial0, radial1, axial, radiusDelta;

					if ( !cyl || cyl->checkcount == CM_CHECKCOUNT ) {
						continue;
					}
					cyl->checkcount = CM_CHECKCOUNT;

					VectorSubtract( tw->start, cyl->origin, d );
					radial0 = ( d[0] * cyl->radialAxes[0][0]
								+ d[1] * cyl->radialAxes[0][1] )
							  + d[2] * cyl->radialAxes[0][2];
					radial1 = ( d[0] * cyl->radialAxes[1][0]
								+ d[1] * cyl->radialAxes[1][1] )
							  + d[2] * cyl->radialAxes[1][2];
					axial = ( d[0] * cyl->axis[0] + d[1] * cyl->axis[1] )
							+ d[2] * cyl->axis[2];
					radiusDelta = ( radial0 * radial0 + radial1 * radial1 )
								  - radius * radius;

					if ( radiusDelta > 0.0f ) {
						float vel0, vel1, vdot, vlenSq, disc, frac;

						vel0 = ( tw->delta[0] * cyl->radialAxes[0][0]
								 + tw->delta[1] * cyl->radialAxes[0][1] )
							   + tw->delta[2] * cyl->radialAxes[0][2];
						vel1 = ( tw->delta[0] * cyl->radialAxes[1][0]
								 + tw->delta[1] * cyl->radialAxes[1][1] )
							   + tw->delta[2] * cyl->radialAxes[1][2];
						vdot = vel0 * radial0 + vel1 * radial1;
						if ( !( vdot < 0.0f ) ) {
							continue;
						}

						vlenSq = vel0 * vel0 + vel1 * vel1;
						disc = vdot * vdot - vlenSq * radiusDelta;
						if ( !( disc > 0.0f ) ) {
							continue;
						}

						frac = ( ( float ) -sqrt( disc ) - vdot ) / vlenSq;
						if ( !( frac < tw->trace.fraction ) ) {
							continue;
						}

						axial += frac * ( ( tw->delta[0] * cyl->axis[0]
											+ tw->delta[1] * cyl->axis[1] )
										  + tw->delta[2] * cyl->axis[2] );
						if ( axial < 0.0f || axial > cyl->length ) {
							continue;
						}

						{
							float inv = 1.0f / radius;
							float n0 = ( vel0 * frac + radial0 ) * inv;
							float n1 = ( vel1 * frac + radial1 ) * inv;

							tw->trace.normal[0] = cyl->radialAxes[0][0] * n0;
							tw->trace.normal[1] = cyl->radialAxes[0][1] * n0;
							tw->trace.normal[2] = cyl->radialAxes[0][2] * n0;
							tw->trace.normal[0] += cyl->radialAxes[1][0] * n1;
							tw->trace.normal[1] += cyl->radialAxes[1][1] * n1;
							tw->trace.normal[2] += cyl->radialAxes[1][2] * n1;
						}

						if ( tw->trace.fraction <= TERRAIN_HIT_EPSILON ) {
							tw->trace.fraction = 0.0f;
							tw->trace.startsolid = 1;
							return;
						}
						tw->trace.fraction = frac - TERRAIN_HIT_EPSILON;
					} else if ( !( axial < 0.0f ) && axial <= cyl->length ) {
						VectorCopy( surf->normal, tw->trace.normal );
						tw->trace.fraction = 0.0f;
						tw->trace.startsolid = 1;
						return;
					}
				} else {
					terrainVertexSphere_t *vert = facet->vertexSpheres[e];
					vec3_t d;
					float radiusDelta, vdot, disc, frac, inv;

					if ( !vert || vert->checkcount == CM_CHECKCOUNT ) {
						continue;
					}
					vert->checkcount = CM_CHECKCOUNT;

					VectorSubtract( tw->start, vert->origin, d );
					radiusDelta = ( d[0] * d[0] + d[1] * d[1] + d[2] * d[2] )
								  - radius * radius;
					if ( radiusDelta <= 0.0f ) {
						VectorCopy( surf->normal, tw->trace.normal );
						tw->trace.fraction = 0.0f;
						tw->trace.startsolid = 1;
						return;
					}

					vdot = ( tw->delta[0] * d[0] + tw->delta[1] * d[1] )
						   + tw->delta[2] * d[2];
					if ( !( vdot < 0.0f ) ) {
						continue;
					}

					disc = vdot * vdot
						   - tw->deltaLengthSquared * radiusDelta;
					if ( disc < 0.0f ) {
						continue;
					}

					frac = ( ( float ) -sqrt( disc ) - vdot )
						   / tw->deltaLengthSquared;
					if ( !( frac < tw->trace.fraction ) ) {
						continue;
					}

					inv = 1.0f / radius;
					tw->trace.normal[0] = ( tw->delta[0] * frac + d[0] ) * inv;
					tw->trace.normal[1] = ( tw->delta[1] * frac + d[1] ) * inv;
					tw->trace.normal[2] = ( tw->delta[2] * frac + d[2] ) * inv;

					if ( tw->trace.fraction <= TERRAIN_HIT_EPSILON ) {
						tw->trace.fraction = 0.0f;
						tw->trace.startsolid = 1;
						return;
					}
					tw->trace.fraction = frac - TERRAIN_HIT_EPSILON;
				}
			}
		}
	}
}
#if 0
void __cdecl CM_TraceSphereThroughTerrainCollide(traceWork_t *tw, void *tc)
{
  int v2;
  double v3;
  int v5;
  double v6;
  int v7;
  int v8;
  float *v9;
  double v10;
  double v11;
  double v12;
  double v13;
  double v14;
  double v15;
  _DWORD *v16;
  double v17;
  double v18;
  double v19;
  double v20;
  double v21;
  double v22;
  double v23;
  double v24;
  int i;
  int v26;
  float v27;
  double v28;
  double v29;
  double v30;
  double v31;
  _DWORD *v32;
  double v33;
  double v34;
  long double v35;
  long double v36;
  double v37;
  double v38;
  double v39;
  float v40;
  double v41;
  double v42;
  double v43;
  double v44;
  double v45;
  double v46;
  float v47;
  float v48;
  float v49;
  float v50;
  float v51;
  float v52;
  float v53;
  float v54;
  float v55;
  int v56;
  float v57;
  float v58;
  float v59;
  float v60;
  int v61;
  float v62;
  float v63;
  float v64;
  float v65;
  float v66;
  float v67;
  float v68;
  float v69;
  float v70;
  float v71;
  float v72;
  float twa;

  v3 = *(float *)(v2 + 252) - *(float *)(v2 + 248);
  if ( BYTE2(tw->start[0]) )
  {
    v59 = -2.0 * v3;
    *(float *)(v2 + 8) = v3 + *(float *)(v2 + 8);
    *(float *)(v2 + 20) = v3 + *(float *)(v2 + 20);
  }
  else
  {
    v59 = v3 + v3;
    *(float *)(v2 + 8) = *(float *)(v2 + 8) - v3;
    *(float *)(v2 + 20) = *(float *)(v2 + 20) - v3;
  }
  v5 = 0;
  v6 = *(float *)(v2 + 248) + 0.125;
  v7 = 0;
  v56 = 0;
  twa = v6;
  v58 = -v6;
  if ( LOWORD(tw->start[0]) )
  {
    v8 = 13;
    v9 = &tw->delta[1];
    while ( 1 )
    {
      v10 = *(v9 - 4) * *(float *)(v2 + 20)
          + *(v9 - 5) * *(float *)(v2 + 16)
          + *(v9 - 6) * *(float *)(v2 + 12)
          - *(v9 - 3);
      if ( v10 < twa )
      {
        v11 = *(v9 - 4) * *(float *)(v2 + 8) + *(v9 - 5) * *(float *)(v2 + 4) + *(v9 - 6) * *(float *)v2 - *(v9 - 3);
        v52 = v11;
        v62 = v11 - v10;
        if ( v62 > 0.0 )
        {
          if ( v52 > (double)v58 )
          {
            if ( (v52 < (double)twa) | __UNORDERED__(v52, twa) )
            {
              v20 = 0.0;
              v21 = *(float *)v2;
              v22 = *(float *)(v2 + 4);
              v23 = *(float *)(v2 + 8);
            }
            else
            {
              v20 = (v52 - twa) / v62;
              if ( v20 > *(float *)(v2 + 196) )
                goto LABEL_57;
              v21 = v20 * *(float *)(v2 + 24) + *(float *)v2;
              v22 = v20 * *(float *)(v2 + 28) + *(float *)(v2 + 4);
              v23 = v20 * *(float *)(v2 + 32) + *(float *)(v2 + 8);
            }
            v49 = v22 * *(v9 - 1) + v21 * *(v9 - 2) + v23 * *v9 - v9[1];
            v24 = v23 * v9[4] + v22 * v9[3] + v21 * v9[2] - v9[5];
            if ( v49 + v24 > 1.0 )
              v5 = 1;
            v61 = (4 * (((v24 < 0.0) | __UNORDERED__(v24, 0.0)) != 0))
                | (2 * (((v49 < 0.0) | __UNORDERED__(v49, 0.0)) != 0))
                | v5;
            if ( v61 )
            {
              for ( i = 0; i < 3; ++i )
              {
                v26 = v8 + i;
                if ( ((1 << i) & v61) != 0 )
                {
                  v27 = tw->end[v26];
                  if ( v27 != 0.0 && *(_DWORD *)LODWORD(v27) != cm_checkcount )
                  {
                    *(_DWORD *)LODWORD(v27) = cm_checkcount;
                    v28 = *(float *)v2 - *(float *)(LODWORD(v27) + 4);
                    v29 = *(float *)(v2 + 4) - *(float *)(LODWORD(v27) + 8);
                    v30 = *(float *)(v2 + 8) - *(float *)(LODWORD(v27) + 12);
                    v69 = v28 * *(float *)(LODWORD(v27) + 16)
                        + v30 * *(float *)(LODWORD(v27) + 24)
                        + v29 * *(float *)(LODWORD(v27) + 20);
                    v70 = v28 * *(float *)(LODWORD(v27) + 28)
                        + v30 * *(float *)(LODWORD(v27) + 36)
                        + v29 * *(float *)(LODWORD(v27) + 32);
                    v71 = v30 * *(float *)(LODWORD(v27) + 48)
                        + v29 * *(float *)(LODWORD(v27) + 44)
                        + v28 * *(float *)(LODWORD(v27) + 40);
                    v31 = v70 * v70 + v69 * v69 - twa * twa;
                    v53 = v31;
                    if ( v31 > 0.0 )
                    {
                      v64 = *(float *)(LODWORD(v27) + 16) * *(float *)(v2 + 24)
                          + *(float *)(v2 + 28) * *(float *)(LODWORD(v27) + 20)
                          + *(float *)(LODWORD(v27) + 24) * *(float *)(v2 + 32);
                      v66 = *(float *)(v2 + 28) * *(float *)(LODWORD(v27) + 32)
                          + *(float *)(v2 + 24) * *(float *)(LODWORD(v27) + 28)
                          + *(float *)(LODWORD(v27) + 36) * *(float *)(v2 + 32);
                      v33 = v70 * v66 + v69 * v64;
                      v50 = v33;
                      if ( v33 < 0.0 )
                      {
                        v34 = v66 * v66 + v64 * v64;
                        v35 = v50 * v50 - v34 * v53;
                        if ( v35 > 0.0 )
                        {
                          v36 = (-sqrt(v35) - v50) / v34;
                          v54 = v36;
                          if ( (v36 < *(float *)(v2 + 196)) | __UNORDERED__(v36, *(float *)(v2 + 196)) )
                          {
                            v37 = (*(float *)(LODWORD(v27) + 48) * *(float *)(v2 + 32)
                                 + *(float *)(v2 + 24) * *(float *)(LODWORD(v27) + 40)
                                 + *(float *)(LODWORD(v27) + 44) * *(float *)(v2 + 28))
                                * v54
                                + v71;
                            if ( !((v37 < 0.0) | __UNORDERED__(v37, 0.0)) && v37 <= *(float *)(LODWORD(v27) + 52) )
                            {
                              v38 = 1.0 / twa;
                              v39 = (v64 * v54 + v69) * v38;
                              v72 = (v66 * v54 + v70) * v38;
                              *(float *)(v2 + 212) = v39 * *(float *)(LODWORD(v27) + 16);
                              *(float *)(v2 + 216) = v39 * *(float *)(LODWORD(v27) + 20);
                              *(float *)(v2 + 220) = v39 * *(float *)(LODWORD(v27) + 24);
                              *(float *)(v2 + 212) = v72 * *(float *)(LODWORD(v27) + 28) + *(float *)(v2 + 212);
                              *(float *)(v2 + 216) = v72 * *(float *)(LODWORD(v27) + 32) + *(float *)(v2 + 216);
                              *(float *)(v2 + 220) = v72 * *(float *)(LODWORD(v27) + 36) + *(float *)(v2 + 220);
                              if ( *(float *)(v2 + 196) <= 0.0000099999997 )
                                goto LABEL_39;
                              *(float *)(v2 + 196) = v54 - 0.0000099999997;
                            }
                          }
                        }
                      }
                    }
                    else if ( !((v71 < 0.0) | __UNORDERED__(v71, 0.0)) && v71 <= (double)*(float *)(LODWORD(v27) + 52) )
                    {
LABEL_38:
                      v32 = (_DWORD *)&tw->start[18 * v56];
                      *(_DWORD *)(v2 + 212) = v32[1];
                      *(_DWORD *)(v2 + 216) = v32[2];
                      *(_DWORD *)(v2 + 220) = v32[3];
LABEL_39:
                      *(_DWORD *)(v2 + 196) = 0;
                      *(_BYTE *)(v2 + 243) = 1;
                      return;
                    }
                  }
                }
                else
                {
                  v40 = tw->start[v26];
                  if ( v40 != 0.0 && *(_DWORD *)LODWORD(v40) != cm_checkcount )
                  {
                    *(_DWORD *)LODWORD(v40) = cm_checkcount;
                    v65 = *(float *)v2 - *(float *)(LODWORD(v40) + 4);
                    v67 = *(float *)(v2 + 4) - *(float *)(LODWORD(v40) + 8);
                    v41 = *(float *)(v2 + 8) - *(float *)(LODWORD(v40) + 12);
                    v68 = v41;
                    v42 = v41 * v68 + v67 * v67 + v65 * v65 - twa * twa;
                    v55 = v42;
                    if ( v42 <= 0.0 )
                      goto LABEL_38;
                    v43 = v67 * *(float *)(v2 + 28) + v65 * *(float *)(v2 + 24) + v68 * *(float *)(v2 + 32);
                    v51 = v43;
                    if ( v43 < 0.0 )
                    {
                      v44 = v51 * v51 - *(float *)(v2 + 36) * v55;
                      if ( !((v44 < 0.0) | __UNORDERED__(v44, 0.0)) )
                      {
                        v45 = (-sqrt(v44) - v51) / *(float *)(v2 + 36);
                        if ( (v45 < *(float *)(v2 + 196)) | __UNORDERED__(v45, *(float *)(v2 + 196)) )
                        {
                          v46 = 1.0 / twa;
                          *(float *)(v2 + 212) = (v45 * *(float *)(v2 + 24) + v65) * v46;
                          *(float *)(v2 + 216) = (v45 * *(float *)(v2 + 28) + v67) * v46;
                          *(float *)(v2 + 220) = (v45 * *(float *)(v2 + 32) + v68) * v46;
                          if ( *(float *)(v2 + 196) <= 0.0000099999997 )
                            goto LABEL_60;
                          *(float *)(v2 + 196) = v45 - 0.0000099999997;
                        }
                      }
                    }
                  }
                }
              }
              v7 = v56;
            }
            else
            {
              *(float *)(v2 + 212) = *(v9 - 6);
              *(float *)(v2 + 216) = *(v9 - 5);
              *(float *)(v2 + 220) = *(v9 - 4);
              if ( v20 <= 0.0000099999997 )
              {
LABEL_60:
                *(_DWORD *)(v2 + 196) = 0;
                *(_BYTE *)(v2 + 243) = 1;
                return;
              }
              *(float *)(v2 + 196) = v20 - 0.0000099999997;
            }
          }
          else
          {
            v12 = v59 * *(v9 - 4) + v52;
            if ( v12 > v58 )
            {
              v60 = 1.0 / *(v9 - 4);
              v13 = (v58 - v52) * v60;
              v57 = *(v9 - 1) * *(float *)(v2 + 4) + *(v9 - 2) * *(float *)v2 + *v9 * *(float *)(v2 + 8) - v9[1];
              v63 = v9[3] * *(float *)(v2 + 4) + v9[2] * *(float *)v2 + v9[4] * *(float *)(v2 + 8) - v9[5];
              v14 = v13 * *v9 + v57;
              if ( v14 >= 0.0 )
              {
                v15 = v13 * v9[4] + v63;
                if ( v15 >= 0.0 )
                {
                  v47 = v14;
                  if ( v47 + v15 <= 1.0 )
                    goto LABEL_14;
                }
              }
              v17 = (v12 < twa) | (unsigned __int8)__UNORDERED__(v12, twa) ? v59 : (twa - v52) * v60;
              v18 = v17 * *v9 + v57;
              if ( !((v18 < 0.0) | __UNORDERED__(v18, 0.0)) )
              {
                v19 = v17 * v9[4] + v63;
                if ( !((v19 < 0.0) | __UNORDERED__(v19, 0.0)) )
                {
                  v48 = v18;
                  if ( v48 + v19 <= 1.0 )
                  {
LABEL_14:
                    v16 = (_DWORD *)&tw->start[18 * v7];
                    *(_DWORD *)(v2 + 212) = v16[1];
                    *(_DWORD *)(v2 + 216) = v16[2];
                    *(_DWORD *)(v2 + 220) = v16[3];
                    *(_DWORD *)(v2 + 196) = 0;
                    *(_BYTE *)(v2 + 243) = 1;
                    return;
                  }
                }
              }
            }
          }
        }
      }
LABEL_57:
      ++v7;
      v9 += 18;
      v8 += 18;
      v56 = v7;
      if ( v7 >= LOWORD(tw->start[0]) )
        return;
      v5 = 0;
    }
  }
}
#endif

/* ---- CM_PositionTestSphereWithTerrainCollide  0x004216B0 ---- */
qboolean __cdecl CM_PositionTestSphereWithTerrainCollide(traceWork_t *twOpaque,
														 void *tcVoid)
{
	cmTerrainTrace_t *tw = CM_TW( twOpaque );
	const terrainCollide_t *tc = CM_TC( tcVoid );
	const terrainFacet_t *facets = CM_TC_FACETS( tcVoid );
	int numFacets = ( int ) tc->facetCount;
	float radius = tw->sphere.radius;
	float negRadius = -radius;
	float capOffset = tw->sphere.halfheight - radius;
	float capDelta;
	int i;

	if ( tc->capsuleOffsetSign ) {
		capDelta = capOffset * -2.0f;
		tw->start[2] += capOffset;
		tw->end[2] += capOffset;
	} else {
		capDelta = capOffset + capOffset;
		tw->start[2] -= capOffset;
		tw->end[2] -= capOffset;
	}

	for ( i = 0; i < numFacets; i++ ) {
		const terrainFacet_t *facet = &facets[i];
		const terrainPlane_t *surf = &facet->surfacePlane;
		float surfDist;

		surfDist = ( tw->start[0] * surf->normal[0]
					 + tw->start[1] * surf->normal[1] )
				   + tw->start[2] * surf->normal[2] - surf->dist;

		if ( surfDist >= radius ) {
			continue;
		}

		if ( surfDist <= negRadius ) {
			float capDist = surfDist + capDelta * surf->normal[2];
			float invNormalZ, enterOffset, leaveOffset;
			float baseE0, baseE1, e0, e1;

			if ( negRadius >= capDist ) {
				continue;
			}

			invNormalZ = 1.0f / surf->normal[2];
			enterOffset = ( negRadius - surfDist ) * invNormalZ;

			baseE0 = ( tw->start[0] * facet->edgePlanes[0].normal[0]
					   + tw->start[1] * facet->edgePlanes[0].normal[1] )
					 + tw->start[2] * facet->edgePlanes[0].normal[2]
					 - facet->edgePlanes[0].dist;
			baseE1 = ( tw->start[0] * facet->edgePlanes[1].normal[0]
					   + tw->start[1] * facet->edgePlanes[1].normal[1] )
					 + tw->start[2] * facet->edgePlanes[1].normal[2]
					 - facet->edgePlanes[1].dist;

			e0 = baseE0 + enterOffset * facet->edgePlanes[0].normal[2];
			if ( e0 >= 0.0f ) {
				e1 = baseE1 + enterOffset * facet->edgePlanes[1].normal[2];
				if ( e1 >= 0.0f && e0 + e1 <= 1.0f ) {
					return qtrue;
				}
			}

			if ( radius > capDist ) {
				leaveOffset = capDelta;
			} else {
				leaveOffset = ( radius - surfDist ) * invNormalZ;
			}

			e0 = baseE0 + leaveOffset * facet->edgePlanes[0].normal[2];
			if ( !( 0.0f > e0 ) ) {
				e1 = baseE1 + leaveOffset * facet->edgePlanes[1].normal[2];
				if ( !( 0.0f > e1 ) && !( e0 + e1 > 1.0f ) ) {
					return qtrue;
				}
			}
		} else {
			vec3_t proj;
			float e0, e1;
			unsigned int outside;
			int e;

			proj[0] = tw->start[0] - surfDist * surf->normal[0];
			proj[1] = tw->start[1] - surfDist * surf->normal[1];
			proj[2] = tw->start[2] - surfDist * surf->normal[2];

			e0 = ( proj[0] * facet->edgePlanes[0].normal[0]
				   + proj[1] * facet->edgePlanes[0].normal[1] )
				 + proj[2] * facet->edgePlanes[0].normal[2]
				 - facet->edgePlanes[0].dist;
			e1 = ( proj[0] * facet->edgePlanes[1].normal[0]
				   + proj[1] * facet->edgePlanes[1].normal[1] )
				 + proj[2] * facet->edgePlanes[1].normal[2]
				 - facet->edgePlanes[1].dist;

			outside = 0;
			if ( e0 + e1 > 1.0f ) {
				outside |= 1;
			}
			if ( e0 < 0.0f ) {
				outside |= 2;
			}
			if ( e1 < 0.0f ) {
				outside |= 4;
			}

			if ( !outside ) {
				return qtrue;
			}

			for ( e = 0; e < 3; e++ ) {
				vec3_t d;
				float distSq;

				if ( ( outside >> e ) & 1 ) {
					const terrainEdgeCylinder_t *cyl = facet->edgeCylinders[e];
					float along;

					if ( !cyl ) {
						continue;
					}

					VectorSubtract( tw->start, cyl->origin, d );
					along = ( d[0] * cyl->axis[0] + d[1] * cyl->axis[1] )
							+ d[2] * cyl->axis[2];
					if ( along < 0.0f || along > cyl->length ) {
						continue;
					}

					d[0] -= along * cyl->axis[0];
					d[1] -= along * cyl->axis[1];
					d[2] -= along * cyl->axis[2];
				} else {
					const terrainVertexSphere_t *vert = facet->vertexSpheres[e];

					if ( !vert ) {
						continue;
					}

					VectorSubtract( tw->start, vert->origin, d );
				}

				distSq = ( d[0] * d[0] + d[1] * d[1] ) + d[2] * d[2];
				if ( distSq < radius * radius ) {
					return qtrue;
				}
			}
		}
	}

	return qfalse;
}
#if 0
qboolean __cdecl CM_PositionTestSphereWithTerrainCollide(traceWork_t *tw, void *tc)
{
  float *v2;
  double v3;
  int v5;
  float *i;
  double v7;
  double v8;
  double v9;
  double v10;
  double v11;
  double v13;
  double v14;
  double v15;
  double v16;
  double v17;
  double v18;
  double v19;
  int v20;
  int v21;
  float v22;
  double v23;
  unsigned __int8 v25; // c0
  unsigned __int8 v26; // c3
  float v28;
  float v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;
  float v35;
  int v36;
  int v37;
  float v38;
  float v39;
  float v40;
  float v41;
  float v42;
  float twa;
  float twb;
  float twc;

  v3 = v2[63] - v2[62];
  if ( BYTE2(tw->start[0]) )
  {
    v32 = -2.0 * v3;
    v2[2] = v3 + v2[2];
    v2[5] = v3 + v2[5];
  }
  else
  {
    v32 = v3 + v3;
    v2[2] = v2[2] - v3;
    v2[5] = v2[5] - v3;
  }
  v30 = v2[62];
  v31 = -v30;
  v37 = 0;
  if ( !LOWORD(tw->start[0]) )
    return 0;
  v5 = 13;
  for ( i = tw->end; ; i += 18 )
  {
    v7 = *i * v2[2] + v2[1] * *(i - 1) + *v2 * *(i - 2) - i[1];
    twa = v7;
    if ( v7 >= v30 )
      goto LABEL_31;
    if ( twa > (double)v31 )
      break;
    v8 = v32 * *i + twa;
    if ( v8 > v31 )
    {
      v33 = 1.0 / *i;
      v9 = (v31 - twa) * v33;
      v34 = i[3] * v2[1] + i[2] * *v2 + i[4] * v2[2] - i[5];
      v35 = i[7] * v2[1] + i[6] * *v2 + i[8] * v2[2] - i[9];
      v10 = v9 * i[4] + v34;
      if ( v10 >= 0.0 )
      {
        v11 = v9 * i[8] + v35;
        if ( v11 >= 0.0 )
        {
          v28 = v10;
          if ( v28 + v11 <= 1.0 )
            return 1;
        }
      }
      v13 = (v8 < v30) | (unsigned __int8)__UNORDERED__(v8, v30) ? v32 : (v30 - twa) * v33;
      v14 = v13 * i[4] + v34;
      if ( !((v14 < 0.0) | __UNORDERED__(v14, 0.0)) )
      {
        v15 = v13 * i[8] + v35;
        if ( !((v15 < 0.0) | __UNORDERED__(v15, 0.0)) )
        {
          v29 = v14;
          if ( v29 + v15 <= 1.0 )
            return 1;
        }
      }
    }
LABEL_31:
    v5 += 18;
    if ( ++v37 >= LOWORD(tw->start[0]) )
      return 0;
  }
  v16 = -twa;
  v38 = v16 * *(i - 2) + *v2;
  v39 = v16 * *(i - 1) + v2[1];
  v17 = v16 * *i + v2[2];
  v40 = v17;
  v18 = v17 * i[4] + v39 * i[3] + v38 * i[2] - i[5];
  v19 = v40 * i[8] + v39 * i[7] + v38 * i[6] - i[9];
  twb = v19;
  v20 = (4 * (((twb < 0.0) | __UNORDERED__(twb, 0.0)) != 0))
      | (2 * (((v18 < 0.0) | __UNORDERED__(v18, 0.0)) != 0))
      | (v19 + v18 > 1.0);
  v36 = v20;
  if ( v20 )
  {
    v21 = 0;
    while ( 1 )
    {
      if ( ((1 << v21) & v20) != 0 )
      {
        v22 = tw->end[v5 + v21];
        if ( v22 == 0.0 )
          goto LABEL_30;
        v41 = *v2 - *(float *)(LODWORD(v22) + 4);
        v42 = v2[1] - *(float *)(LODWORD(v22) + 8);
        v23 = (v2[2] - *(float *)(LODWORD(v22) + 12)) * *(float *)(LODWORD(v22) + 48)
            + v42 * *(float *)(LODWORD(v22) + 44)
            + v41 * *(float *)(LODWORD(v22) + 40);
        if ( v23 < 0.0 )
          goto LABEL_30;
        twc = v23;
        if ( twc > (double)*(float *)(LODWORD(v22) + 52) )
          goto LABEL_30;
      }
      else if ( !LODWORD(tw->start[v5 + v21]) )
      {
        goto LABEL_30;
      }
      if ( !(v25 | v26) )
        return 1;
LABEL_30:
      if ( ++v21 >= 3 )
        goto LABEL_31;
      v20 = v36;
    }
  }
  return 1;
}
#endif

/* ---- CM_TraceSquareThroughTerrainCollide  0x00421AA0 ---- VERIFIED */
void __cdecl CM_TraceSquareThroughTerrainCollide(traceWork_t *twOpaque, void *tc)
{
	cmTerrainTrace_t *tw = CM_TW( twOpaque );
	float savedStartZ, savedEndZ, savedRadius;

	if ( !cm_boxOnTerrainWarned ) {
		cm_boxOnTerrainWarned = 1;
		Com_DPrintf( "^1Box collision on terrain currently being faked with capsule collision\n" );
	}

	savedStartZ = tw->start[2];
	savedEndZ = tw->end[2];
	savedRadius = tw->sphere.radius;

	tw->sphere.use = 1;
	tw->sphere.radius = tw->maxs[0];

	CM_TraceSphereThroughTerrainCollide( twOpaque, tc );

	tw->sphere.use = 0;
	tw->start[2] = savedStartZ;
	tw->end[2] = savedEndZ;
	tw->sphere.radius = savedRadius;
}

/* ---- CM_TraceThroughTerrainCollide  0x00421B20 ---- VERIFIED */
void __cdecl CM_TraceThroughTerrainCollide(traceWork_t *twOpaque, void *tc)
{
	cmTerrainTrace_t *tw = CM_TW( twOpaque );

	if ( tw->isPoint ) {
		CM_TracePointThroughTerrainCollide( twOpaque, tc );
		return;
	}

	if ( tw->sphere.use ) {
		float savedStartZ = tw->start[2];
		float savedEndZ = tw->end[2];

		CM_TraceSphereThroughTerrainCollide( twOpaque, tc );

		tw->start[2] = savedStartZ;
		tw->end[2] = savedEndZ;
		return;
	}

	CM_TraceSquareThroughTerrainCollide( twOpaque, tc );
}

/* ---- CM_SightTraceThroughTerrainCollide  0x00421B80 ---- VERIFIED */
qboolean __cdecl CM_SightTraceThroughTerrainCollide(traceWork_t *twOpaque, void *tc)
{
	cmTerrainTrace_t sightWork;

	sightWork = *CM_TW( twOpaque );
	sightWork.trace.fraction = 1.0f;

	CM_TraceThroughTerrainCollide( ( traceWork_t * ) (void *) &sightWork, tc );

	return sightWork.trace.fraction == 1.0f;
}

/* ---- CM_PositionTestInTerrainCollide  0x00421BE0 ---- VERIFIED */
qboolean __cdecl CM_PositionTestInTerrainCollide(traceWork_t *twOpaque, void *tc)
{
	cmTerrainTrace_t *tw = CM_TW( twOpaque );
	float savedStartZ, savedEndZ;
	qboolean result;

	if ( tw->isPoint ) {
		return qfalse;
	}

	savedStartZ = tw->start[2];
	savedEndZ = tw->end[2];

	result = CM_PositionTestSphereWithTerrainCollide( twOpaque, tc );

	tw->start[2] = savedStartZ;
	tw->end[2] = savedEndZ;

	return result;
}
