
#ifndef TR_FONT_H
#define TR_FONT_H

typedef struct {
    int   height;               /* +0x00 */
    int   top;                  /* +0x04 */
    float vertBearing;          /* +0x08 */
    float horzBearing;          /* +0x0C */
    float xSkip;                /* +0x10 */
    int   imageWidth;           /* +0x14 */
    int   imageHeight;          /* +0x18 */
    float s;                    /* +0x1C */
    float t;                    /* +0x20 */
    float s2;                   /* +0x24 */
    float t2;                   /* +0x28 */
    int   glyph;                /* +0x2C  qhandle_t */
    char  shaderName[32];       /* +0x30 */
} glyphInfo_t;

typedef struct {
    glyphInfo_t glyphs[256];    /* +0x0000 */
    float       glyphScale;     /* +0x5000 */
    float       fontHeight;     /* +0x5004 */
    char        name[64];       /* +0x5008 */
} fontInfo_t;                   /* 0x5048 */

extern glyphInfo_t g_asianGlyph;

#endif
