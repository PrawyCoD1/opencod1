/*
 * @fidelity: verified
 */
#ifndef TR_ORIENTATION_H
#define TR_ORIENTATION_H

#include "tr_records.h"

#define flt_16D8DA8   backEnd_or.origin[0]        /* +0x00 */
#define flt_16D8DAC   backEnd_or.origin[1]        /* +0x04 */
#define flt_16D8DB0   backEnd_or.origin[2]        /* +0x08 */
#define flt_16D8DB4   backEnd_or.axis[0][0]       /* +0x0C */
#define flt_16D8DB8   backEnd_or.axis[0][1]       /* +0x10 */
#define flt_16D8DBC   backEnd_or.axis[0][2]       /* +0x14 */
#define flt_16D8DC0   backEnd_or.axis[1][0]       /* +0x18 */
#define flt_16D8DC4   backEnd_or.axis[1][1]       /* +0x1C */
#define flt_16D8DC8   backEnd_or.axis[1][2]       /* +0x20 */
#define flt_16D8DCC   backEnd_or.axis[2][0]       /* +0x24 */
#define flt_16D8DD0   backEnd_or.axis[2][1]       /* +0x28 */
#define flt_16D8DD4   backEnd_or.axis[2][2]       /* +0x2C */
#define flt_16D8DD8   backEnd_or.viewOrigin[0]    /* +0x30 */
#define flt_16D8DDC   backEnd_or.viewOrigin[1]    /* +0x34 */
#define flt_16D8DE0   backEnd_or.viewOrigin[2]    /* +0x38 */
#define flt_16D8DE4   backEnd_or.modelMatrix      /* +0x3C, 16 floats */

#endif
