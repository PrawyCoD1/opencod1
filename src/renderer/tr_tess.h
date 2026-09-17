
#ifndef TR_TESS_H
#define TR_TESS_H

extern unsigned short tess_indexes[49152];
extern float tess_xyz[32768];
extern float tess_texCoords0[32768];
extern float tess_texCoords1[32768];
extern unsigned char tess_vertexColors[32768];
extern float tess_stageNormals[24576];
extern float tess_stageBitangents[24576];
extern float tess_stageTangents[24576];
extern unsigned char tess_stageVertexColors[32768];


#undef  word_17A7F5E
#define word_17A7F5E    ((__int16 *)((char *)tess_indexes - 2))   /* 0x017A7F5E = tess_indexes-2 */
#undef  word_17A7F60
#define word_17A7F60    ((__int16 *)((char *)tess_indexes + 0))   /* 0x017A7F60 = tess_indexes+0 */
#undef  word_17A7F62
#define word_17A7F62    ((__int16 *)((char *)tess_indexes + 2))   /* 0x017A7F62 = tess_indexes+2 */
#undef  word_17A7F64
#define word_17A7F64    ((__int16 *)((char *)tess_indexes + 4))   /* 0x017A7F64 = tess_indexes+4 */
#undef  word_17A7F66
#define word_17A7F66    ((__int16 *)((char *)tess_indexes + 6))   /* 0x017A7F66 = tess_indexes+6 */
#undef  word_17A7F68
#define word_17A7F68    ((__int16 *)((char *)tess_indexes + 8))   /* 0x017A7F68 = tess_indexes+8 */
#undef  word_17A7F6A
#define word_17A7F6A    ((__int16 *)((char *)tess_indexes + 10))   /* 0x017A7F6A = tess_indexes+10 */
#undef  word_17A7F6C
#define word_17A7F6C    ((__int16 *)((char *)tess_indexes + 12))   /* 0x017A7F6C = tess_indexes+12 */
#undef  word_17A7F6E
#define word_17A7F6E    ((__int16 *)((char *)tess_indexes + 14))   /* 0x017A7F6E = tess_indexes+14 */
#undef  word_17A7F70
#define word_17A7F70    ((__int16 *)((char *)tess_indexes + 16))   /* 0x017A7F70 = tess_indexes+16 */
#undef  word_17A7F72
#define word_17A7F72    ((__int16 *)((char *)tess_indexes + 18))   /* 0x017A7F72 = tess_indexes+18 */
#undef  word_17A7F74
#define word_17A7F74    ((__int16 *)((char *)tess_indexes + 20))   /* 0x017A7F74 = tess_indexes+20 */
#undef  word_17A7F76
#define word_17A7F76    ((__int16 *)((char *)tess_indexes + 22))   /* 0x017A7F76 = tess_indexes+22 */
#undef  word_17BFF5E
#define word_17BFF5E    (*(unsigned short *)((char *)tess_indexes + 98302))   /* 0x017BFF5E = tess_indexes+98302 */

#undef  dword_17BFF60
#define dword_17BFF60   ((int *)((char *)tess_xyz + 0))   /* 0x017BFF60 = tess_xyz+0 */
#undef  flt_17BFF64
#define flt_17BFF64     ((float *)((char *)tess_xyz + 4))   /* 0x017BFF64 = tess_xyz+4 */
#undef  dword_17BFF68
#define dword_17BFF68   ((int *)((char *)tess_xyz + 8))   /* 0x017BFF68 = tess_xyz+8 */
#undef  flt_17BFF6C
#define flt_17BFF6C     (*(float *)((char *)tess_xyz + 12))   /* 0x017BFF6C = tess_xyz+12 */
#undef  flt_17BFF70
#define flt_17BFF70     (*(float *)((char *)tess_xyz + 16))   /* 0x017BFF70 = tess_xyz+16 */
#undef  dword_17BFF74
#define dword_17BFF74   (*(int *)((char *)tess_xyz + 20))   /* 0x017BFF74 = tess_xyz+20 */
#undef  flt_17BFF78
#define flt_17BFF78     (*(float *)((char *)tess_xyz + 24))   /* 0x017BFF78 = tess_xyz+24 */
#undef  flt_17BFF7C
#define flt_17BFF7C     (*(float *)((char *)tess_xyz + 28))   /* 0x017BFF7C = tess_xyz+28 */
#undef  dword_17BFF80
#define dword_17BFF80   (*(int *)((char *)tess_xyz + 32))   /* 0x017BFF80 = tess_xyz+32 */
#undef  dword_17BFF84
#define dword_17BFF84   (*(int *)((char *)tess_xyz + 36))   /* 0x017BFF84 = tess_xyz+36 */
#undef  flt_17BFF88
#define flt_17BFF88     (*(float *)((char *)tess_xyz + 40))   /* 0x017BFF88 = tess_xyz+40 */
#undef  dword_17BFF8C
#define dword_17BFF8C   (*(int *)((char *)tess_xyz + 44))   /* 0x017BFF8C = tess_xyz+44 */

#undef  dword_17DFF60
#define dword_17DFF60   ((int *)((char *)tess_texCoords0 + 0))   /* 0x017DFF60 = tess_texCoords0+0 */
#undef  dword_17DFF64
#define dword_17DFF64   ((int *)((char *)tess_texCoords0 + 4))   /* 0x017DFF64 = tess_texCoords0+4 */
#undef  dword_17DFF68
#define dword_17DFF68   ((int *)((char *)tess_texCoords0 + 8))   /* 0x017DFF68 = tess_texCoords0+8 */
#undef  dword_17DFF6C
#define dword_17DFF6C   ((int *)((char *)tess_texCoords0 + 12))   /* 0x017DFF6C = tess_texCoords0+12 */
#undef  dword_17DFF70
#define dword_17DFF70   ((int *)((char *)tess_texCoords0 + 16))   /* 0x017DFF70 = tess_texCoords0+16 */
#undef  dword_17DFF74
#define dword_17DFF74   ((int *)((char *)tess_texCoords0 + 20))   /* 0x017DFF74 = tess_texCoords0+20 */
#undef  dword_17DFF78
#define dword_17DFF78   ((int *)((char *)tess_texCoords0 + 24))   /* 0x017DFF78 = tess_texCoords0+24 */
#undef  dword_17DFF7C
#define dword_17DFF7C   ((int *)((char *)tess_texCoords0 + 28))   /* 0x017DFF7C = tess_texCoords0+28 */
#undef  flt_17DFF80
#define flt_17DFF80     ((float *)((char *)tess_texCoords0 + 32))   /* 0x017DFF80 = tess_texCoords0+32 */
#undef  flt_17DFF84
#define flt_17DFF84     ((float *)((char *)tess_texCoords0 + 36))   /* 0x017DFF84 = tess_texCoords0+36 */

#undef  flt_17FFF60
#define flt_17FFF60     ((float *)((char *)tess_texCoords1 + 0))   /* 0x017FFF60 = tess_texCoords1+0 */
#undef  flt_17FFF64
#define flt_17FFF64     ((float *)((char *)tess_texCoords1 + 4))   /* 0x017FFF64 = tess_texCoords1+4 */
#undef  flt_17FFF68
#define flt_17FFF68     ((float *)((char *)tess_texCoords1 + 8))   /* 0x017FFF68 = tess_texCoords1+8 */
#undef  flt_17FFF6C
#define flt_17FFF6C     ((float *)((char *)tess_texCoords1 + 12))   /* 0x017FFF6C = tess_texCoords1+12 */
#undef  flt_17FFF70
#define flt_17FFF70     ((float *)((char *)tess_texCoords1 + 16))   /* 0x017FFF70 = tess_texCoords1+16 */
#undef  flt_17FFF74
#define flt_17FFF74     ((float *)((char *)tess_texCoords1 + 20))   /* 0x017FFF74 = tess_texCoords1+20 */
#undef  flt_17FFF78
#define flt_17FFF78     ((float *)((char *)tess_texCoords1 + 24))   /* 0x017FFF78 = tess_texCoords1+24 */
#undef  flt_17FFF7C
#define flt_17FFF7C     ((float *)((char *)tess_texCoords1 + 28))   /* 0x017FFF7C = tess_texCoords1+28 */
#undef  flt_17FFF80
#define flt_17FFF80     ((float *)((char *)tess_texCoords1 + 32))   /* 0x017FFF80 = tess_texCoords1+32 */
#undef  flt_17FFF84
#define flt_17FFF84     ((float *)((char *)tess_texCoords1 + 36))   /* 0x017FFF84 = tess_texCoords1+36 */

#undef  dword_181FF60
#define dword_181FF60   ((int *)((char *)tess_vertexColors + 0))   /* 0x0181FF60 = tess_vertexColors+0 */
#undef  dword_181FF64
#define dword_181FF64   ((int *)((char *)tess_vertexColors + 4))   /* 0x0181FF64 = tess_vertexColors+4 */
#undef  dword_181FF68
#define dword_181FF68   ((int *)((char *)tess_vertexColors + 8))   /* 0x0181FF68 = tess_vertexColors+8 */
#undef  dword_181FF6C
#define dword_181FF6C   ((int *)((char *)tess_vertexColors + 12))   /* 0x0181FF6C = tess_vertexColors+12 */
#undef  dword_181FF70
#define dword_181FF70   ((int *)((char *)tess_vertexColors + 16))   /* 0x0181FF70 = tess_vertexColors+16 */

#undef  flt_1827F60
#define flt_1827F60     ((float *)((char *)tess_stageNormals + 0))   /* 0x01827F60 = tess_stageNormals+0 */
#undef  flt_1827F64
#define flt_1827F64     ((float *)((char *)tess_stageNormals + 4))   /* 0x01827F64 = tess_stageNormals+4 */
#undef  dword_1827F68
#define dword_1827F68   ((float *)((char *)tess_stageNormals + 8))   /* 0x01827F68 = tess_stageNormals+8 */
#undef  dword_1827F74
#define dword_1827F74   ((int *)((char *)tess_stageNormals + 20))   /* 0x01827F74 = tess_stageNormals+20 */
#undef  dword_1827F80
#define dword_1827F80   ((int *)((char *)tess_stageNormals + 32))   /* 0x01827F80 = tess_stageNormals+32 */
#undef  dword_1827F8C
#define dword_1827F8C   ((int *)((char *)tess_stageNormals + 44))   /* 0x01827F8C = tess_stageNormals+44 */
#undef  dword_1827F98
#define dword_1827F98   ((int *)((char *)tess_stageNormals + 56))   /* 0x01827F98 = tess_stageNormals+56 */

#undef  dword_183FF60
#define dword_183FF60   ((int *)((char *)tess_stageBitangents + 0))   /* 0x0183FF60 = tess_stageBitangents+0 */

#undef  dword_1857F60
#define dword_1857F60   ((int *)((char *)tess_stageTangents + 0))   /* 0x01857F60 = tess_stageTangents+0 */
#undef  unk_1857F64
#define unk_1857F64     (*(unsigned char *)((char *)tess_stageTangents + 4))   /* 0x01857F64 = tess_stageTangents+4 */
#undef  flt_1857F68
#define flt_1857F68     ((float *)((char *)tess_stageTangents + 8))   /* 0x01857F68 = tess_stageTangents+8 */
#undef  byte_186FF5E
#define byte_186FF5E    ((char *)((char *)tess_stageVertexColors - 2))   /* 0x0186FF5E = tess_stageVertexColors-2 */

#undef  dword_186FF60
#define dword_186FF60   ((int *)((char *)tess_stageVertexColors + 0))   /* 0x0186FF60 = tess_stageVertexColors+0 */


extern float tr_sunLight[3];       /* 0x016C57C4 */
extern float tr_sunDirection[3];   /* 0x016C57D0 */

#undef  flt_16C57C4
#define flt_16C57C4     (*(float *)((char *)tr_sunLight + 0))       /* 0x016C57C4 */
#undef  flt_16C57C8
#define flt_16C57C8     (*(float *)((char *)tr_sunLight + 4))       /* 0x016C57C8 */
#undef  flt_16C57CC
#define flt_16C57CC     (*(float *)((char *)tr_sunLight + 8))       /* 0x016C57CC */

#undef  dword_16C57D0
#define dword_16C57D0   (*(int *)((char *)tr_sunDirection + 0))     /* 0x016C57D0 */
#undef  dword_16C57D4
#define dword_16C57D4   (*(int *)((char *)tr_sunDirection + 4))     /* 0x016C57D4 */
#undef  dword_16C57D8
#define dword_16C57D8   (*(int *)((char *)tr_sunDirection + 8))     /* 0x016C57D8 */


#undef  flt_11DAE20
#define flt_11DAE20     ((float *)((char *)sky_mins + 24))          /* 0x011DAE20 = sky_mins[1] */
#undef  flt_11DAE50
#define flt_11DAE50     ((float *)((char *)sky_maxs + 24))          /* 0x011DAE50 = sky_maxs[1] */

#undef  dword_11DAE6C
#define dword_11DAE6C   ((int *)((char *)s_cloudTexCoords + 4))     /* 0x011DAE6C = s_cloudTexCoords+1 */


#undef  dword_11FECE4
#define dword_11FECE4   ((int *)((char *)edgeDefs + 4))             /* 0x011FECE4 = edgeDefs[0][0].facing */

#endif
