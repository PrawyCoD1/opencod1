/*
 * @fidelity: verified   (sizes only -- there is no code here)
 */

unsigned short tess_indexes[49152];
float tess_xyz[32768];
float tess_texCoords0[32768];
float tess_texCoords1[32768];
unsigned char tess_vertexColors[32768];
float tess_stageNormals[24576];
float tess_stageBitangents[24576];
float tess_stageTangents[24576];
unsigned char tess_stageVertexColors[32768];

unsigned char tess_optimizedIndexes[262144];

void *tess_activeTexCoords[8];
unsigned char tess_generatedTexCoords[0x100000];  /* 1048576 bytes -- tess.generatedTexCoords, 8 units x 0x20000 */


float tr_sunLight[3];       /*     12 bytes -- 0x016C57C4, one vec3_t */
float tr_sunDirection[3];   /*     12 bytes -- 0x016C57D0, one vec3_t */
