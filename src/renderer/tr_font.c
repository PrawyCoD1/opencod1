/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_gl_types.h"
#include "tr_tess.h"
#include "tr_font.h"

extern int RE_RegisterShaderNoMip();
extern int RE_SetColor();
extern int RE_StretchPic();
extern int R_FindShader();
extern int __cdecl R_IssueRenderCommands( int runPerformanceCounters );

unsigned int __fastcall Japanese_CollapseShiftJISCode( int uiCode );
unsigned int __fastcall Korean_CollapseKSC5601HangulCode( int uiCode );
int          __fastcall Taiwanese_CollapseBig5Code( int uiCode );


static fontInfo_t s_registeredFonts[8];

glyphInfo_t g_asianGlyph;

#define dword_11CBCA4  g_asianGlyph.top
#define flt_11CBCA8    g_asianGlyph.vertBearing
#define dword_11CBCAC  g_asianGlyph.horzBearing
#define flt_11CBCB0    g_asianGlyph.xSkip
#define dword_11CBCB4  g_asianGlyph.imageWidth
#define dword_11CBCB8  g_asianGlyph.imageHeight
#define flt_11CBCBC    g_asianGlyph.s
#define flt_11CBCC0    g_asianGlyph.t
#define flt_11CBCC4    g_asianGlyph.s2
#define flt_11CBCC8    g_asianGlyph.t2
#define dword_11CBCCC  g_asianGlyph.glyph

int    __cdecl R_LoadAsianFont( const char *shaderPath );
double __cdecl R_GetAsianScale( int font, float scale );
double __cdecl R_GetGlyphHorizAdvance( unsigned int c, int font );
int   *__cdecl R_GetCharacterGlyph( unsigned int c, int font );
int __fastcall R_DrawStrlen( int ecxUnused, _BYTE *text );
void   __cdecl RE_Text_PaintWithCursor( int a1, int a2, int a3, int a4, float *a5, const char *a6, int a7, char a8, int a9, int a10, int a11 );

/* ---- R_LoadAsianFont  0x004DE850 ----  VERIFIED */
int __cdecl R_LoadAsianFont(const char *a1)
{
  int result;
  int v2;
  const char *v3;
  int v4;
  int v5;
  char *Shader;
  char v7[64]; // [esp+8h] [ebp-44h] BYREF
  unsigned int v8;
  unsigned int retaddr;

  v8 = retaddr ^ _security_cookie;
  result = g_currentAsian;
  v2 = 0;
  if ( !g_currentAsian )
  {
    dword_14072F8 = 0;
    return result;
  }
  result = (int)cl_language;
  if ( !dword_14072F8 || -1 != cl_language->integer )
  {
    if ( cl_language->integer == 8 )
    {
      g_asianFontPageDimension = 32;
      v3 = "kor";
    }
    else
    {
      if ( cl_language->integer == 9 )
      {
        g_asianFontPageDimension = 64;
        v4 = 4;
        v3 = "tai";
        goto LABEL_13;
      }
      result = cl_language->integer - 10;
      if ( cl_language->integer != 10 )
      {
        dword_14072F8 = 0;
        return result;
      }
      g_asianFontPageDimension = 64;
      v3 = "jap";
    }
    v4 = 3;
LABEL_13:
    g_asianFontPageCount = v4;
    do
    {
      Com_sprintf(v7, sizeof(v7), "font/%s_%d_1024_%d.tga", v3, 1024 / g_asianFontPageDimension, v2);
      if ( tr_registered )
      {
        if ( strlen(v7) < 0x40 )
        {
          Shader = R_FindShader(v7, -4, 0, a1);
          if ( (Shader[76] & 1) != 0 )
            v5 = 0;
          else
            v5 = *((_DWORD *)Shader + 17);
        }
        else
        {
          Com_Printf("Shader name exceeds MAX_QPATH\n");
          v5 = 0;
        }
      }
      else
      {
        v5 = 0;
      }
      g_asianFontPageHandles[v2++] = v5;
    }
    while ( v2 < g_asianFontPageCount );
    result = 1024 / g_asianFontPageDimension;
    rendererAsianFontLastPageHalfHeight = 1;
    dword_11CBCAC = 0;
    dword_14072F8 = 1;
    g_asianGlyph.height = 1024 / g_asianFontPageDimension;
    dword_11CBCA4 = 1024 / g_asianFontPageDimension;
    dword_11CBCB4 = 1024 / g_asianFontPageDimension;
    flt_11CBCA8 = (float)(1024 / g_asianFontPageDimension);
    dword_11CBCB8 = 1024 / g_asianFontPageDimension;
    flt_11CBCB0 = flt_11CBCA8;
  }
  return result;
}

/* ---- R_GetAsianCode  0x004DEA00 ----  VERIFIED */
int __cdecl R_GetAsianCode(int this)
{
  int result;

  result = 0;
  if ( dword_14072F8 )
  {
    switch ( cl_language->integer )
    {
      case 8:
        return Korean_CollapseKSC5601HangulCode(this);
      case 9:
        return Taiwanese_CollapseBig5Code(this);
      case 0xA:
        return Japanese_CollapseShiftJISCode(this);
    }
  }
  return result;
}

/* ---- R_GetCharacterGlyph  0x004DEA30 ----  VERIFIED */
int *__cdecl R_GetCharacterGlyph(unsigned int a1, int a2)
{
  signed int v3;
  signed int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  double v10;
  double v11;
  double v12;
  int integer;

  if ( !dword_14072F8 )
    return (int *)(a2 + 80 * (a1 & 0xFF));
  integer = cl_language->integer;
  switch ( integer )
  {
    case 8:
      v3 = Korean_CollapseKSC5601HangulCode(a1);
      break;
    case 9:
      v3 = Taiwanese_CollapseBig5Code(a1);
      break;
    case 10:
      v3 = Japanese_CollapseShiftJISCode(a1);
      break;
    default:
      return (int *)(a2 + 80 * (a1 & 0xFF));
  }
  v4 = v3;
  if ( v3 )
  {
    v5 = g_asianFontPageDimension * g_asianFontPageDimension;
    v6 = v3 / (g_asianFontPageDimension * g_asianFontPageDimension);
    if ( v6 > g_asianFontPageCount )
      v6 = 0;
    v7 = (v3 - v6 * v5) / g_asianFontPageDimension;
    v8 = (v4 - v6 * v5) % g_asianFontPageDimension;
    if ( v6 == g_asianFontPageCount - 1 && rendererAsianFontLastPageHalfHeight )
      v9 = g_asianFontPageDimension / 2;
    else
      v9 = g_asianFontPageDimension;
    if ( integer == 9 )
    {
      flt_11CBCBC = (double)(v8 * (1024 / g_asianFontPageDimension) + 1) * 0.0009765625;
      flt_11CBCC0 = (double)(v7 * (1024 / v9) + 1) * 0.0009765625;
      flt_11CBCC4 = (double)(1024 / g_asianFontPageDimension * (v8 + 1)) * 0.0009765625;
      v12 = (double)(1024 / v9 * (v7 + 1));
    }
    else
    {
      if ( integer != 10 )
      {
        v10 = (double)g_asianFontPageDimension;
        flt_11CBCBC = (double)v8 / v10;
        v11 = (double)v9;
        flt_11CBCC0 = (double)v7 / v11;
        flt_11CBCC4 = (double)(v8 + 1) / v10;
        flt_11CBCC8 = (double)(v7 + 1) / v11;
LABEL_21:
        dword_11CBCCC = g_asianFontPageHandles[v6];
        return (int *)&g_asianGlyph;
      }
      flt_11CBCBC = (double)(v8 * (1024 / g_asianFontPageDimension)) * 0.0009765625;
      flt_11CBCC0 = (double)(v7 * (1024 / v9)) * 0.0009765625;
      flt_11CBCC4 = (double)(1024 / g_asianFontPageDimension * (v8 + 1) - 1) * 0.0009765625;
      v12 = (double)(1024 / v9 * (v7 + 1) - 1);
    }
    flt_11CBCC8 = v12 * 0.0009765625;
    goto LABEL_21;
  }
  return (int *)(a2 + 80 * (a1 & 0xFF));
}

/* ---- R_GetGlyphHorizAdvance  0x004DEC50 ----  VERIFIED */
double __cdecl R_GetGlyphHorizAdvance(unsigned int a1, int a2)
{
  unsigned int v3;
  double v5;

  if ( !dword_14072F8 )
    goto LABEL_10;
  switch ( cl_language->integer )
  {
    case 8:
      v3 = Korean_CollapseKSC5601HangulCode(a1);
      break;
    case 9:
      v3 = Taiwanese_CollapseBig5Code(a1);
      break;
    case 0xA:
      v3 = Japanese_CollapseShiftJISCode(a1);
      break;
    default:
      goto LABEL_10;
  }
  if ( v3 )
    return flt_11CBCB0;
LABEL_10:
  v5 = *(float *)(80 * (a1 & 0xFF) + a2 + 16);
  if ( (v5 == 0.0) | __UNORDERED__(v5, 0.0) )
    return *(float *)(a2 + 3696);
  else
    return *(float *)(80 * (a1 & 0xFF) + a2 + 16);
}

/* ---- R_GetAsianScale  0x004DECD0 ----  VERIFIED */
double __cdecl R_GetAsianScale(int font, float a1)
{
  int v1 = font;
  float v3;

  v3 = 48.0 / *(float *)(v1 + 20480) / (double)g_asianGlyph.height;
  if ( a1 > 0.25 )
    v3 = ((a1 - 0.25) * 0.40000001 + 0.25) / a1 * v3;
  a1 = (double)dword_11CBCB8 * v3 * a1 * *(float *)(v1 + 20480);
  ri_SCR_AdjustFrom640(0, 0, 0, &a1);
  if ( (a1 < 16.0) | __UNORDERED__(a1, 16.0) )
    return 16.0 / a1 * v3;
  else
    return v3;
}

/* ---- R_GetAsianGlyphHeight  0x004DED70 ----  VERIFIED */
double R_GetAsianGlyphHeight()
{
  return (double)g_asianGlyph.height;
}

/* ---- readInt  0x004DED80 ----  VERIFIED */
int readInt()
{
  int result;

  result = *(unsigned __int8 *)(s_fontDataOffset + s_fontData)
         + ((*(unsigned __int8 *)(s_fontDataOffset + s_fontData + 1)
           + ((*(unsigned __int8 *)(s_fontDataOffset + s_fontData + 2)
             + (*(unsigned __int8 *)(s_fontDataOffset + s_fontData + 3) << 8)) << 8)) << 8);
  s_fontDataOffset += 4;
  return result;
}

/* ---- readFloat  0x004DEDC0 ----  VERIFIED */
double readFloat()
{
  double result;

  result = *(float *)(s_fontDataOffset + s_fontData);
  s_fontDataOffset += 4;
  return result;
}

/* ---- RE_RegisterFont  0x004DEE00 ----  VERIFIED */
void __cdecl RE_RegisterFont(int a1, int size, int a5, const char *a6)
{
  int v6;
  int v7;
  int v8;
  char *v9;
  int v10;
  int v11;
  unsigned __int8 *v12;
  _DWORD *v13;
  int v14;
  unsigned __int8 *v15;
  char v16;
  char v17;
  int v18;
  int v19;
  int v20;
  int v21;
  int v22;
  int v23;
  char v24;
  int v25;
  char *v26;
  int v27;
  int v28;
  char *Shader;
  int v30;
  int v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  int v40;
  int v41;
  int v42;
  int v43; // [esp+24h] [ebp-408h] BYREF
  char Source[1024]; // [esp+28h] [ebp-404h] BYREF
  unsigned int v45;
  unsigned int retaddr;

  v6 = size;
  v45 = retaddr ^ _security_cookie;
  if ( size <= 0 )
    v6 = 12;
  if ( tr_registered )
    R_IssueRenderCommands(0);       /* `push 0` -- ONE argument */
  R_LoadAsianFont(a6);
  if ( dword_14072F4 < 8 )
  {
    Com_sprintf(Source, sizeof(Source), "fonts/fontImage_%i.dat", v6);
    v7 = dword_14072F4;
    v8 = 0;
    if ( dword_14072F4 <= 0 )
    {
LABEL_12:
      if ( ri_FS_ReadFile(Source, 0) == 20552 )
      {
        ri_FS_ReadFile(Source, &v43);
        v10 = v43;
        s_fontData = v43;

        {
          const unsigned char *cur = (const unsigned char *)v10;
          int glyphIndex;

          for ( glyphIndex = 0; glyphIndex < 256; glyphIndex++ )
          {
            _DWORD *g = (_DWORD *)( a5 + 80 * glyphIndex );
            int field;

            for ( field = 0; field < 12; field++ )
            {
              g[field] = cur[0] | ( cur[1] << 8 ) | ( cur[2] << 16 ) | ( cur[3] << 24 );
              cur += 4;
            }
            qmemcpy( g + 12, cur, 0x20u );
            cur += 32;
          }

          *(_DWORD *)(a5 + 20480) = cur[0] | ( cur[1] << 8 ) | ( cur[2] << 16 ) | ( cur[3] << 24 );
          cur += 4;
          *(_DWORD *)(a5 + 20484) = cur[0] | ( cur[1] << 8 ) | ( cur[2] << 16 ) | ( cur[3] << 24 );
          cur += 4;
          s_fontDataOffset = (int)cur - v10;
          qmemcpy((void *)(a5 + 20488), cur, 0x40u);
          strncpy((char *)(a5 + 20488), Source, 0x3Fu);
          *(_BYTE *)(a5 + 20551) = 0;
        }
        v26 = (char *)(a5 + 48);
        v27 = 255;
        do
        {
          if ( tr_registered )
          {
            if ( strlen(v26) < 0x40 )
            {
              Shader = R_FindShader(v26, -4, 0, a6);
              if ( (Shader[76] & 1) != 0 )
                v28 = 0;
              else
                v28 = *((_DWORD *)Shader + 17);
            }
            else
            {
              Com_Printf("Shader name exceeds MAX_QPATH\n");
              v28 = 0;
            }
          }
          else
          {
            v28 = 0;
          }
          *((_DWORD *)v26 - 1) = v28;
          v26 += 80;
          --v27;
        }
        while ( v27 );
        v30 = dword_14072F4;
        qmemcpy(&s_registeredFonts[dword_14072F4], (const void *)a5, 0x5048u);
        dword_14072F4 = v30 + 1;
        ri_FS_FreeFile(v43);
      }
      else
      {
        ri_Printf(0, "RE_RegisterFont: Unable to load font data %s\n", Source);
      }
    }
    else
    {
      v9 = s_registeredFonts[0].name;               /* retail 0x011A8A50 */
      while ( !v9 || Q_stricmpn(Source, v9, 99999) )
      {
        ++v8;
        v9 += 20552;
        if ( v8 >= v7 )
          goto LABEL_12;
      }
      qmemcpy((void *)a5, &s_registeredFonts[v8], 0x5048u);
    }
  }
  else
  {
    ri_Printf(0, "RE_RegisterFont: Too many fonts registered already.\n");
  }
}

/* ---- R_InitFreeType  0x004DF2E0 ----  VERIFIED */
int R_InitFreeType()
{
  int result;

  result = 0;
  dword_14072F4 = 0;
  dword_14072F8 = 0;
  return result;
}

/* ---- R_DoneFreeType  0x004DF2F0 ----  VERIFIED */
int R_DoneFreeType()
{
  int result;

  result = 0;
  dword_14072F4 = 0;
  dword_14072F8 = 0;
  return result;
}

/* ---- R_GetFontInfo  0x004DF300 ----  VERIFIED */
int __cdecl R_GetFontInfo(int ecxArg, int a1)
{
  return ri_GetFontInfo(ecxArg, a1);
}

/* ---- RE_Text_Width  0x004DF310 ----  VERIFIED */
int __cdecl RE_Text_Width(char *a1, int a2, float a3, float a4, int a5)
{
  int v6;
  cvar_t *v7;
  unsigned __int8 v8;
  char v9;
  unsigned int v10;
  int v11;
  char v12;
  unsigned int v13;
  unsigned __int8 v14;
  unsigned __int8 v15;
  char v16;
  double v17;
  double GlyphHorizAdvance;
  int v20;
  float v21;
  float AsianScale;
  float v23;
  float v24;

  v6 = ri_GetFontInfo(a2, LODWORD(a3));
  if ( (a4 == 0.0) | __UNORDERED__(a4, 0.0) )
    v23 = a3 * *(float *)(v6 + 20480);
  else
    v23 = a4;
  AsianScale = 1.0;
  if ( g_currentAsian )
    AsianScale = R_GetAsianScale(v6, a3);   /* `mov ecx, edi` at 0x4DF36A */
  v24 = 0.0;
  v21 = 0.0;
  if ( a1 )
  {
    if ( a5 <= 0 )
      a5 = 0x7FFFFFFF;
    v20 = 0;
    if ( *a1 )
    {
      v7 = cl_language;
      while ( 1 )
      {
        if ( v20 >= a5 )
          return (int)(v21 * v23);
        if ( !g_currentAsian )
          break;
        switch ( v7->integer )
        {
          case 8:
            v14 = *a1;
            v15 = a1[1];
            if ( (unsigned __int8)*a1 < 0xB0u || v14 > 0xC8u || v15 <= 0xA0u || v15 == 0xFF )
              goto LABEL_38;
            v10 = v15 + (v14 << 8);
            a1 += 2;
            break;
          case 9:
            v11 = (unsigned __int8)a1[1];
            v12 = a1[1];
            v13 = (unsigned int)(v11 + ((unsigned __int8)*a1 << 8)) >> 8;
            if ( ((unsigned __int8)v13 < 0xA1u || (unsigned __int8)v13 > 0xC6u)
              && ((unsigned __int8)v13 < 0xC9u || (unsigned __int8)v13 > 0xF9u)
              || ((unsigned __int8)v12 < 0x40u || (unsigned __int8)v12 > 0x7Eu)
              && ((unsigned __int8)v12 < 0xA1u || v12 == -1) )
            {
              goto LABEL_38;
            }
            v10 = v11 + ((unsigned __int8)*a1 << 8);
            a1 += 2;
            break;
          case 0xA:
            v8 = *a1;
            v9 = a1[1];
            if ( ((unsigned __int8)*a1 < 0x81u || v8 > 0x9Fu) && (v8 < 0xE0u || v8 > 0xEFu) )
              goto LABEL_38;
            if ( ((unsigned __int8)v9 < 0x40u || (unsigned __int8)v9 > 0x7Eu) && v9 > -4 )
              goto LABEL_38;
            v10 = (unsigned __int8)v9 + (v8 << 8);
            a1 += 2;
            break;
          default:
            goto LABEL_38;
        }
LABEL_39:
        if ( v10 == 10 )
        {
          v24 = 0.0;
        }
        else if ( v10 == 94 && a1 && (v16 = *a1, *a1 != 94) && v16 >= 48 && v16 <= 55 )
        {
          ++a1;
        }
        else
        {
          if ( (a4 == 0.0) | __UNORDERED__(a4, 0.0) )
          {
            GlyphHorizAdvance = R_GetGlyphHorizAdvance(v10, v6);
            if ( v10 > 0xFF )
              GlyphHorizAdvance = GlyphHorizAdvance * AsianScale;
            v17 = GlyphHorizAdvance + v24;
          }
          else
          {
            v17 = v24 + 1.0;
          }
          v24 = v17;
          if ( v24 > (double)v21 )
            v21 = v17;
          ++v20;
        }
        if ( !*a1 )
          return (int)(v21 * v23);
      }
LABEL_38:
      v10 = (unsigned __int8)*a1++;
      goto LABEL_39;
    }
  }
  return (int)(v21 * v23);
}

/* ---- RE_Text_Height  0x004DF540 ----  VERIFIED */
int __cdecl RE_Text_Height(int a1, float a2)
{
  return (int)(a2 * *(float *)(ri_GetFontInfo(a1, LODWORD(a2)) + 20484));
}

/* ---- RE_Text_Paint  0x004DF570 ----  VERIFIED */
void __cdecl RE_Text_Paint(int a1, int a2, int a3, int a4, float *a5, const char *a6, int a7, int a8, int a9)
{
  RE_Text_PaintWithCursor(a1, a2, a3, a4, a5, a6, -1, 0, a7, a8, a9);
}

/* ---- R_Text_GetConsoleString  0x004DF5B0 ----  VERIFIED */
int __cdecl R_Text_GetConsoleString(_DWORD *a1, float *a2, int a3, int *a4, _DWORD *a5)
{
  int v6;
  int v7;
  int v8;
  unsigned __int16 v9;
  int v10;
  int v11;
  char v12;
  int v14;
  int i;

  *a1 = s_consoleTextBuffer;
  if ( *a4 > 1023 )
    *a4 = 1023;
  s_consoleTextBuffer[0] = 0;
  v6 = 0;
  v7 = -1;
  v8 = 0;
  v14 = 7;
  for ( i = 0; v8 < *a4; ++v8 )
  {
    v9 = *(_WORD *)(a3 + 2 * v8);
    v10 = HIBYTE(v9);
    if ( v10 == v14 )
      goto LABEL_19;
    switch ( v10 )
    {
      case 13:
      case 16:
      case 17:
      case 18:
        *a5 = a3 + 2 * v8;
        v7 = -1;
        i = 1;
        goto LABEL_27;
      case 10:
        if ( a2 )
          *a2 = (double)(unsigned __int8)*(_WORD *)(a3 + 2 * v8) * 0.0039215689;
        break;
      case 11:
        if ( a2 )
          a2[1] = (double)(unsigned __int8)*(_WORD *)(a3 + 2 * v8) * 0.0039215689;
        break;
      case 12:
        if ( a2 )
          a2[2] = (double)(unsigned __int8)*(_WORD *)(a3 + 2 * v8) * 0.0039215689;
        break;
      default:
        s_consoleTextBuffer[v6] = 94;
        v14 = HIBYTE(v9);
        v11 = v6 + 1;
        s_consoleTextBuffer[v11] = HIBYTE(v9) + 48;
        v6 = v11 + 1;
LABEL_19:
        v12 = *(_BYTE *)(a3 + 2 * v8);
        s_consoleTextBuffer[v6] = v12;
        if ( v12 == 32 )
        {
          if ( v7 == -1 )
            v7 = v6;
        }
        else
        {
          v7 = -1;
        }
        ++v6;
        continue;
    }
  }
LABEL_27:
  s_consoleTextBuffer[v6] = 0;
  if ( v7 >= 0 )
    s_consoleTextBuffer[v7] = 0;
  if ( v8 == *a4 )
    *a5 = 0;
  else
    *a5 = a3 + 2 * v8;
  *a4 -= v8;
  return i;
}

/* ---- R_Text_GetConsoleIcon  0x004DF750 ----  VERIFIED */
int __cdecl R_Text_GetConsoleIcon(
        _DWORD *a1,
        int a2,
        int *a3,
        float a4,
        float *a5,
        float *a6,
        int *a7,
        float *a8)
{
  double v8;
  int result;
  int v10;
  int v11;
  int v12;
  float *v13;
  double v14;
  char *v15;
  __int16 v16;
  char v17[64]; // [esp+0h] [ebp-44h] BYREF
  unsigned int v18;
  unsigned int retaddr;

  v8 = a4 * 48.0;
  v18 = retaddr ^ _security_cookie;
  *a5 = v8;
  *a6 = v8;
  *a7 = 0;
  memset(v17, 0, sizeof(v17));
  result = *a3;
  v10 = 0;
  if ( *a3 > 0 )
  {
    while ( 2 )
    {
      v11 = HIBYTE(*(unsigned __int16 *)(a2 + 2 * v10));
      v12 = (unsigned __int8)*(_WORD *)(a2 + 2 * v10);
      switch ( v11 )
      {
        case 13:
          v13 = a8;
          if ( !a8 )
            goto LABEL_13;
          v14 = (double)v12 * 0.0039215689;
          goto LABEL_12;
        case 14:
          if ( a8 )
            a8[1] = (double)v12 * 0.0039215689;
          goto LABEL_13;
        case 15:
          if ( a8 )
            a8[2] = (double)v12 * 0.0039215689;
          goto LABEL_13;
        case 16:
          v13 = a5;
          goto LABEL_11;
        case 17:
          v13 = a6;
LABEL_11:
          v14 = (double)v12 * 0.03125 * *v13;
LABEL_12:
          *v13 = v14;
LABEL_13:
          *a1 += 2;
          goto LABEL_14;
        case 18:
          if ( v11 == 18 )
          {
            v15 = v17;
            do
            {
              if ( !v12 )
                break;
              if ( v10 >= *a3 - 1 )
                break;
              *v15++ = v12;
              *a1 += 2;
              v16 = *(_WORD *)(a2 + 2 * v10++ + 2);
              v12 = (unsigned __int8)v16;
            }
            while ( (v16 & 0xFF00) == 0x1200 );
          }
          *a7 = RE_RegisterShaderNoMip(v17, (const char *)5);
          result = *a3 - v10;
          *a3 = result;
          return result;
        default:
LABEL_14:
          result = *a3;
          if ( ++v10 >= *a3 )
            return result;
          continue;
      }
    }
  }
  return result;
}

/* ---- R_DrawStrlen  0x004DF8E0 ----  VERIFIED */
int __fastcall R_DrawStrlen(int a1, _BYTE *a2)
{
  int result;
  char v3;

  result = 0;
  if ( *a2 )
  {
    do
    {
      if ( *a2 == 94 && (v3 = a2[1]) != 0 && v3 != 94 && v3 >= 48 && v3 <= 55 )
        a2 += 2;
      else
        ++result;
    }
    while ( *++a2 );
  }
  return result;
}

/* ---- RE_Text_ConsoleWidth  0x004DF920 ----  VERIFIED */
int __cdecl RE_Text_ConsoleWidth(int a1, int a2, float a3, float a4, int a5)
{
  int v5;
  float v6;
  float v7;
  int ConsoleString;
  double v10;
  float v12;
  int v13; // [esp+4h] [ebp-10h] BYREF
  int v14; // [esp+8h] [ebp-Ch] BYREF
  int v15; // [esp+Ch] [ebp-8h] BYREF
  int v16; // [esp+10h] [ebp-4h] BYREF

  v5 = a1;
  v12 = 0.0;
  if ( a1 )
  {
    v6 = a4;
    v7 = a3;
    do
    {
      ConsoleString = R_Text_GetConsoleString(&v13, 0, v5, &a5, &a1);
      if ( v13 )
      {
        if ( *(_BYTE *)v13 )
        {
          if ( (a4 == 0.0) | __UNORDERED__(a4, 0.0) )
          {
            v14 = RE_Text_Width((char *)v13, a2, v7, v6, a5);
            v10 = (double)v14;
          }
          else
          {
            v14 = R_DrawStrlen(0, (_BYTE *)v13);  /* ecx is dead in the callee */
            v10 = (double)v14 * a4;
          }
          v12 = v10 + v12;
        }
      }
      if ( ConsoleString )
      {
        R_Text_GetConsoleIcon(&a1, a1, &a5, v7, (float *)&v15, (float *)&v16, &v14, 0);
        v12 = *(float *)&v15 + v12;
      }
      v5 = a1;
    }
    while ( a1 );
  }
  return (int)v12;
}

/* ---- R_Text_PaintConsoleIcon  0x004DFA10 ----  VERIFIED */
unsigned int __cdecl R_Text_PaintConsoleIcon(int a1, int a2, int a3, int a4, int a5)
{
  ri_SCR_AdjustFrom640(&a1, &a2, &a3, &a4);
  return RE_StretchPic(a1, a2, a3, a4, 0, 0, 0x3F800000u, 1065353216, a5);
}

/* ---- RE_Text_ConsolePaint  0x004DFA60 ----  VERIFIED */
int __cdecl RE_Text_ConsolePaint(float a1, int a2, int a3, int a4, int *a5, int a6, int a7, int a8, int a9)
{
  int v9;
  int v10;
  int v11;
  int result;
  int ConsoleString;
  char *v14;
  int v15;
  float v16;
  int v17;
  int v18; // [esp+3Ch] [ebp-30h] BYREF
  int v19; // [esp+40h] [ebp-2Ch] BYREF
  int v20; // [esp+44h] [ebp-28h] BYREF
  int v21; // [esp+48h] [ebp-24h] BYREF
  int v22; // [esp+4Ch] [ebp-20h] BYREF
  int v23; // [esp+50h] [ebp-1Ch] BYREF
  int v24; // [esp+54h] [ebp-18h] BYREF
  int v25; // [esp+58h] [ebp-14h] BYREF
  int v26[4]; // [esp+5Ch] [ebp-10h] BYREF
  float v27;

  v9 = a5[1];
  v26[0] = *a5;
  v10 = a5[2];
  v26[1] = v9;
  v11 = a5[3];
  result = a6;
  v26[2] = v10;
  v26[3] = v11;
  v27 = 0.0;
  while ( a6 )
  {
    ConsoleString = R_Text_GetConsoleString(&v24, (float *)v26, result, &a8, &a6);
    v14 = (char *)v24;
    v15 = ConsoleString;
    if ( !v24 || !*(_BYTE *)v24 )
      goto LABEL_6;
    v16 = *(float *)&a7;
    *(float *)&v17 = v27 + a1;
    RE_Text_PaintWithCursor(v17, a2, a3, a4, (float *)v26, (const char *)v24, -1, 0, a7, 0, a9);
    if ( v15 )
    {
      v22 = RE_Text_Width(v14, a3, *(float *)&a4, v16, a8);
      v27 = (double)v22 + v27;
LABEL_6:
      if ( v15 )
      {
        R_Text_GetConsoleIcon(&a6, a6, &a8, *(float *)&a4, (float *)&v23, (float *)&v18, &v25, (float *)v26);
        RE_SetColor((float *)v26);
        v19 = v18;
        v20 = v23;
        *(float *)&v21 = *(float *)&a2 - (*(float *)&a4 * 38.400002 + *(float *)&v18) * 0.5;
        *(float *)&v22 = v27 + a1;
        ri_SCR_AdjustFrom640(&v22, &v21, &v20, &v19);
        RE_StretchPic(v22, v21, v20, v19, 0, 0, 0x3F800000u, 1065353216, v25);
        v27 = *(float *)&v23 + v27;
      }
    }
    result = a6;
  }
  return result;
}

extern unsigned int __fastcall R_GetCommandBuffer( int ecxUnused, int bytes );

/* ---- RE_Text_PaintWithCursor  0x004DFC10 ----  VERIFIED */
void __cdecl RE_Text_PaintWithCursor(
        int a1,
        int a2,
        int a3,
        int a4,
        float *a5,
        const char *a6,
        int a7,
        char a8,
        int a9,
        int a10,
        int a11)
{
  signed int v12;
  unsigned int CommandBuffer;
  unsigned int v14;

  if ( a6 )
  {
    v12 = strlen(a6);
    if ( a10 > 0 && v12 > a10 )
      v12 = a10;

    CommandBuffer = R_GetCommandBuffer(0, (v12 + 41) & 0xFFFFFFFC);
    v14 = CommandBuffer;
    if ( CommandBuffer )
    {
      *(_DWORD *)(CommandBuffer + 4) = a1;
      *(_DWORD *)(CommandBuffer + 8) = a2;
      *(_DWORD *)(CommandBuffer + 12) = a3;
      *(_DWORD *)(CommandBuffer + 16) = a4;
      *(_DWORD *)CommandBuffer = 6;
      *(_BYTE *)(CommandBuffer + 20) = (unsigned char)(int)(*a5 * 255.0);
      *(_BYTE *)(CommandBuffer + 21) = (unsigned char)(int)(a5[1] * 255.0);
      *(_BYTE *)(CommandBuffer + 22) = (unsigned char)(int)(a5[2] * 255.0);
      *(_DWORD *)(v14 + 24) = a9;
      *(_BYTE *)(v14 + 36) = a8;
      *(_DWORD *)(v14 + 28) = a11;
      *(_BYTE *)(v14 + 23) = (unsigned char)(int)(a5[3] * 255.0);
      *(_DWORD *)(v14 + 32) = a7;
      qmemcpy((void *)(v14 + 37), a6, v12);
      *(_BYTE *)(v14 + v12 + 37) = 0;
    }
  }
}
