#ifndef TR_DPVSPLANE_H
#define TR_DPVSPLANE_H

typedef struct {
	float         normal[3];      /* +0x00 */
	float         distance;       /* +0x0c */
	unsigned char sideOffsets[3]; /* +0x10 */
	unsigned char cameraSide;     /* +0x13  never written by CoD1 */
} dpvsPlane_t;

extern dpvsPlane_t dpvs_nearPlaneRec;   /* retail 0x011CC15C */
extern dpvsPlane_t dpvs_farPlaneRec;    /* retail 0x011CC170 */

#undef  dpvs_nearPlane
#define dpvs_nearPlane   (*(int *)&dpvs_nearPlaneRec.normal[0])      /* +0x00 */
#undef  dword_11CC160
#define dword_11CC160    (*(int *)&dpvs_nearPlaneRec.normal[1])      /* +0x04 */
#undef  dword_11CC164
#define dword_11CC164    (*(int *)&dpvs_nearPlaneRec.normal[2])      /* +0x08 */
#undef  flt_11CC168
#define flt_11CC168      dpvs_nearPlaneRec.distance                  /* +0x0c */
#undef  byte_11CC16C
#define byte_11CC16C     dpvs_nearPlaneRec.sideOffsets[0]            /* +0x10 */
#undef  byte_11CC16D
#define byte_11CC16D     dpvs_nearPlaneRec.sideOffsets[1]            /* +0x11 */
#undef  byte_11CC16E
#define byte_11CC16E     dpvs_nearPlaneRec.sideOffsets[2]            /* +0x12 */

#undef  dpvs_farPlane
#define dpvs_farPlane    dpvs_farPlaneRec.normal[0]                  /* +0x00 */
#undef  flt_11CC174
#define flt_11CC174      dpvs_farPlaneRec.normal[1]                  /* +0x04 */
#undef  flt_11CC178
#define flt_11CC178      dpvs_farPlaneRec.normal[2]                  /* +0x08 */
#undef  flt_11CC17C
#define flt_11CC17C      dpvs_farPlaneRec.distance                   /* +0x0c */
#undef  byte_11CC180
#define byte_11CC180     dpvs_farPlaneRec.sideOffsets[0]             /* +0x10 */
#undef  byte_11CC181
#define byte_11CC181     dpvs_farPlaneRec.sideOffsets[1]             /* +0x11 */
#undef  byte_11CC182
#define byte_11CC182     dpvs_farPlaneRec.sideOffsets[2]             /* +0x12 */

#endif
