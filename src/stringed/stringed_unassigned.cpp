/*
 * stringed/stringed_unassigned.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/stringed/stringed_unassigned.cpp
 *
 * Retail range 0x004A99B0-0x004B08C0, 284 functions.
 *
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

extern int FS_ListFilteredFiles();
extern int j__atol();
extern int j__free();
extern int strtok_m();
extern int sub_492500();
extern int sub_497890();
extern int sub_4979A0();
extern int sub_497C10();
extern int sub_498100();
extern int sub_498410();
extern int sub_4984B0();
extern int sub_4989B0();
extern int sub_498F10();
extern int sub_499550();
extern int sub_499AB0();
extern int sub_53C250();
extern int sub_53C280();

extern int Clear__16CStringEdPackageFi( void *package, int bKeepFileInfo );

/* ---- SE_Init__Fv  0x004A9CD0 ---- VERIFIED */
int SE_Init__Fv(void)
{
  return Clear__16CStringEdPackageFi(&TheStringPackage, 0);
}

/* ---- SE_ShutDown__Fv  0x004A9CE0 ---- VERIFIED */
int SE_ShutDown__Fv(void)
{
  return Clear__16CStringEdPackageFi(&TheStringPackage, 0);
}

/* ---- SE_NewLanguage__Fv  0x004A9CF0 ---- VERIFIED */
int SE_NewLanguage__Fv(void)
{
  return Clear__16CStringEdPackageFi(&TheStringPackage, 1);
}

/* ---- Korean_ValidKSC5601HangulCode  0x004AA360 ---- VERIFIED */
BOOL __cdecl Korean_ValidKSC5601HangulCode(unsigned __int8 a1, unsigned __int8 a2)
{
  return a1 >= 0xB0u && a1 <= 0xC8u && a2 > 0xA0u && a2 != 0xFF;
}

/* ---- Korean_ValidKSC5601HangulCodeWord  0x004AA380 ---- VERIFIED */
BOOL __fastcall Korean_ValidKSC5601HangulCodeWord(__int16 a1)
{
  return HIBYTE(a1) >= 0xB0u && HIBYTE(a1) <= 0xC8u && (unsigned __int8)a1 > 0xA0u && (_BYTE)a1 != 0xFF;
}

/* ---- Korean_CollapseKSC5601HangulCode  0x004AA3A0 ---- VERIFIED */
unsigned int __fastcall Korean_CollapseKSC5601HangulCode(int a1)
{
  if ( BYTE1(a1) < 0xB0u || BYTE1(a1) > 0xC8u || (unsigned __int8)a1 <= 0xA0u || (_BYTE)a1 == 255 )
    return 0;
  else
    return (unsigned __int8)(a1 + 96) + 96 * ((unsigned int)(a1 - 45216) >> 8);
}

/* ---- Taiwanese_ValidBig5Code  0x004AA3E0 ---- VERIFIED */
bool __fastcall Taiwanese_ValidBig5Code(__int16 a1)
{
  return (HIBYTE(a1) >= 0xA1u && HIBYTE(a1) <= 0xC6u || HIBYTE(a1) >= 0xC9u && HIBYTE(a1) <= 0xF9u)
      && ((unsigned __int8)a1 >= 0x40u && (unsigned __int8)a1 <= 0x7Eu
       || (unsigned __int8)a1 >= 0xA1u && (_BYTE)a1 != 0xFF);
}

/* ---- Taiwanese_IsTrailingPunctuation  0x004AA410 ---- VERIFIED */
bool __cdecl Taiwanese_IsTrailingPunctuation(unsigned int a1)
{
  return a1 >= 0xA140 && a1 < 0xA154;
}

/* ---- Taiwanese_CollapseBig5Code  0x004AA430 ---- VERIFIED */
int __fastcall Taiwanese_CollapseBig5Code(int uiCode)
{
  unsigned int v1;

  if ( (BYTE1(uiCode) < 0xA1u || BYTE1(uiCode) > 0xC6u) && (BYTE1(uiCode) < 0xC9u || BYTE1(uiCode) > 0xF9u)
    || ((unsigned __int8)uiCode < 0x40u || (unsigned __int8)uiCode > 0x7Eu)
    && ((unsigned __int8)uiCode < 0xA1u || (_BYTE)uiCode == 0xFF) )
  {
    return 0;
  }
  v1 = uiCode - 41280;
  if ( (unsigned __int8)v1 >= 0x60u )
    v1 -= 32;
  return (unsigned __int8)v1 + 160 * (v1 >> 8);
}

/* ---- Japanese_ValidShiftJISCode  0x004AA490 ---- VERIFIED */
bool __cdecl Japanese_ValidShiftJISCode(unsigned __int8 _iHi, char _iLo)
{
  return (_iHi >= 0x81u && _iHi <= 0x9Fu || _iHi >= 0xE0u && _iHi <= 0xEFu)
      && ((unsigned __int8)_iLo >= 0x40u && (unsigned __int8)_iLo <= 0x7Eu || _iLo <= (char)0xFFFFFFFC);
}

/* ---- Japanese_ValidShiftJISCodeWord  0x004AA4C0 ---- VERIFIED */
bool __fastcall Japanese_ValidShiftJISCodeWord(__int16 a1)
{
  return (HIBYTE(a1) >= 0x81u && HIBYTE(a1) <= 0x9Fu || HIBYTE(a1) >= 0xE0u && HIBYTE(a1) <= 0xEFu)
      && ((unsigned __int8)a1 >= 0x40u && (unsigned __int8)a1 <= 0x7Eu || (char)a1 <= -4);
}

/* ---- Japanese_IsTrailingPunctuation  0x004AA4F0 ---- VERIFIED */
int Japanese_IsTrailingPunctuation(unsigned int uiCode)
{
  return uiCode >= 0x8140 && uiCode < 0x8152;
}

/* ---- Japanese_CollapseShiftJISCode  0x004AA510 ---- VERIFIED */
unsigned int __fastcall Japanese_CollapseShiftJISCode(int a1)
{
  unsigned int v1;

  if ( (BYTE1(a1) < 0x81u || BYTE1(a1) > 0x9Fu) && (BYTE1(a1) < 0xE0u || BYTE1(a1) > 0xEFu)
    || ((unsigned __int8)a1 < 0x40u || (unsigned __int8)a1 > 0x7Eu) && (char)a1 > -4 )
  {
    return 0;
  }
  v1 = a1 - 33088;
  if ( (unsigned __int8)v1 >= 0x40u )
    --v1;
  if ( (v1 & 0xFF00) >= 0x5F00 )
    v1 -= 0x4000;
  return (unsigned __int8)v1 + 188 * (v1 >> 8);
}

/* ---- SEH_ReadCharFromString  0x004AA580 ---- VERIFIED */
int __cdecl SEH_ReadCharFromString(unsigned __int8 **a1, _DWORD *a2)
{
  unsigned __int8 *v2;
  unsigned __int8 v3;
  int v5;
  int result;
  unsigned int v7;
  unsigned __int8 v8;
  unsigned __int8 v9;

  v2 = *a1;
  if ( !g_currentAsian )
    goto LABEL_19;
  if ( cl_language->integer != 8 )
  {
    if ( cl_language->integer != 9 )
    {
      if ( cl_language->integer == 10 )
      {
        v3 = *v2;
        if ( Japanese_ValidShiftJISCode(*v2, v2[1]) )
        {
          v5 = v2[1] + (v3 << 8);
          *a1 = v2 + 2;
          if ( a2 )
            *a2 = Japanese_IsTrailingPunctuation(v5);
          return v5;
        }
      }
      goto LABEL_19;
    }
    v7 = (*v2 << 8) + v2[1];
    if ( Taiwanese_ValidBig5Code(v7) )
    {
      *a1 = v2 + 2;
      if ( a2 )
        *a2 = Taiwanese_IsTrailingPunctuation(v7);
      return v7;
    }
LABEL_19:
    result = *v2;
    *a1 = v2 + 1;
    if ( a2 )
      *a2 = result == 33 || result == 63 || result == 44 || result == 46 || result == 59 || result == 58;
    return result;
  }
  v8 = *v2;
  v9 = v2[1];
  if ( *v2 < 0xB0u || v8 > 0xC8u || v9 <= 0xA0u || v9 == 0xFF )
    goto LABEL_19;
  result = v9 + (v8 << 8);
  *a1 = v2 + 2;
  if ( a2 )
    *a2 = 0;
  return result;
}

/* ---- Language_IsAsian  0x004AA690 ---- VERIFIED */
int Language_IsAsian()
{
  return g_currentAsian;
}

/* ---- Language_UsesSpaceWordBreak_m  0x004AA6A0 ---- VERIFIED */
BOOL Language_UsesSpaceWordBreak_m()
{
  int integer;

  integer = cl_language->integer;
  return integer < 9 || integer > 10;
}

/* ---- SEH_PrintStrlen  0x004AA6C0 ---- VERIFIED */
int __cdecl SEH_PrintStrlen(char *a1)
{
  int result;
  unsigned __int8 v3;
  char v4;
  int v5;
  unsigned __int8 v6;
  unsigned __int8 v7;
  char v8;

  result = 0;
  if ( a1 && *a1 )
  {
    do
    {
      if ( g_currentAsian )
      {
        switch ( cl_language->integer )
        {
          case 8:
            v6 = *a1;
            v7 = a1[1];
            if ( (unsigned __int8)*a1 >= 0xB0u && v6 <= 0xC8u && v7 > 0xA0u && v7 != 0xFF )
            {
              v5 = v7 + (v6 << 8);
              a1 += 2;
              goto LABEL_30;
            }
            break;
          case 9:
            v5 = (unsigned __int8)a1[1] + ((unsigned __int8)*a1 << 8);
            if ( (BYTE1(v5) >= 0xA1u && BYTE1(v5) <= 0xC6u || BYTE1(v5) >= 0xC9u && BYTE1(v5) <= 0xF9u)
              && ((unsigned __int8)v5 >= 0x40u && (unsigned __int8)v5 <= 0x7Eu
               || (unsigned __int8)v5 >= 0xA1u && (_BYTE)v5 != 0xFF) )
            {
              a1 += 2;
              goto LABEL_30;
            }
            break;
          case 0xA:
            if ( (v3 = *a1, v4 = a1[1], (unsigned __int8)*a1 >= 0x81u) && v3 <= 0x9Fu || v3 >= 0xE0u && v3 <= 0xEFu )
            {
              if ( (unsigned __int8)v4 >= 0x40u && (unsigned __int8)v4 <= 0x7Eu || v4 <= -4 )
              {
                v5 = (unsigned __int8)v4 + (v3 << 8);
                a1 += 2;
                goto LABEL_30;
              }
            }
            break;
        }
      }
      v5 = (unsigned __int8)*a1++;
LABEL_30:
      if ( v5 != 94 )
      {
        if ( v5 == 10 || v5 == 13 )
          continue;
LABEL_38:
        ++result;
        continue;
      }
      if ( !a1 )
        goto LABEL_38;
      v8 = *a1;
      if ( *a1 == 94 || v8 < 48 || v8 > 55 )
        goto LABEL_38;
      ++a1;
    }
    while ( *a1 );
  }
  return result;
}

/* ---- stl_string_find_ch_m  0x004AA7F0 ---- MEDIUM */
void *__cdecl stl_string_find_ch_m(size_t a1, char *a2, void *Buf)
{
  return memchr(Buf, *a2, a1);
}

/* ---- CStringEdString_Construct  0x004AA810 ---- HIGH */
int __cdecl CStringEdString_Construct(int result)
{
  *(_DWORD *)(result + 24) = 15;
  *(_DWORD *)(result + 20) = 0;
  *(_BYTE *)(result + 4) = 0;
  *(_DWORD *)(result + 52) = 15;
  *(_DWORD *)(result + 48) = 0;
  *(_BYTE *)(result + 32) = 0;
  *(_DWORD *)(result + 56) = 0;
  return result;
}

/* ---- CStringEdPackage_Construct  AUTO-STUBBED ---- HIGH */
int __stdcall CStringEdPackage_Construct(int a1)
{ return 0; }
#if 0
int __stdcall CStringEdPackage_Construct(int a1)
{
  _DWORD *v1;
  _DWORD *v2;

  *(_DWORD *)(a1 + 28) = 15;
  *(_DWORD *)(a1 + 24) = 0;
  *(_BYTE *)(a1 + 8) = 0;
  *(_DWORD *)(a1 + 56) = 15;
  *(_DWORD *)(a1 + 52) = 0;
  *(_BYTE *)(a1 + 36) = 0;
  *(_DWORD *)(a1 + 84) = 15;
  *(_DWORD *)(a1 + 80) = 0;
  *(_BYTE *)(a1 + 64) = 0;
  *(_DWORD *)(a1 + 112) = 15;
  *(_DWORD *)(a1 + 108) = 0;
  *(_BYTE *)(a1 + 92) = 0;
  *(_BYTE *)(a1 + 120) = a1;
  v1 = stl_map_buynode_str_m(a1 + 120);
  *(_DWORD *)(a1 + 124) = v1;
  *((_BYTE *)v1 + 101) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 124) + 4) = *(_DWORD *)(a1 + 124);
  **(_DWORD **)(a1 + 124) = *(_DWORD *)(a1 + 124);
  *(_DWORD *)(*(_DWORD *)(a1 + 124) + 8) = *(_DWORD *)(a1 + 124);
  *(_DWORD *)(a1 + 128) = 0;
  *(_DWORD *)(a1 + 140) = 0;
  *(_DWORD *)(a1 + 144) = 0;
  *(_DWORD *)(a1 + 148) = 0;
  *(_BYTE *)(a1 + 152) = a1;
  v2 = sub_499550(a1 + 152);
  *(_DWORD *)(a1 + 156) = v2;
  *((_BYTE *)v2 + 45) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 156) + 4) = *(_DWORD *)(a1 + 156);
  **(_DWORD **)(a1 + 156) = *(_DWORD *)(a1 + 156);
  *(_DWORD *)(*(_DWORD *)(a1 + 156) + 8) = *(_DWORD *)(a1 + 156);
  *(_DWORD *)(a1 + 160) = 0;
  CStringEdPackage::Clear((_DWORD *)a1, 0);
  return a1;
}
#endif

/* ---- CStringEdPackage_Destruct  AUTO-STUBBED ---- HIGH */
void __stdcall CStringEdPackage_Destruct(int a1)
{ return 0; }
#if 0
void __stdcall CStringEdPackage_Destruct(int a1)
{
  int v1;
  int *v2;
  int *v3;
  int *v4;
  int *v5;
  int *v6; // [esp+10h] [ebp-10h] BYREF
  int v7;

  v7 = 6;
  CStringEdPackage::Clear((_DWORD *)a1, 0);
  v4 = *(int **)(a1 + 156);
  v2 = (int *)*v4;
  LOBYTE(v7) = 5;
  sub_4989B0(a1 + 152, &v6, v2, v4);
  j__free(*(void **)(a1 + 156));
  *(_DWORD *)(a1 + 156) = 0;
  *(_DWORD *)(a1 + 160) = 0;
  v1 = *(_DWORD *)(a1 + 140);
  if ( v1 )
  {
    stl_vector_destroy_range_impl_m(v1, *(_DWORD *)(a1 + 144));
    j__free(*(void **)(a1 + 140));
  }
  *(_DWORD *)(a1 + 140) = 0;
  *(_DWORD *)(a1 + 144) = 0;
  *(_DWORD *)(a1 + 148) = 0;
  v5 = *(int **)(a1 + 124);
  v3 = (int *)*v5;
  LOBYTE(v7) = 3;
  stl_map_erase_range_m(a1 + 120, &v6, v3, v5);
  j__free(*(void **)(a1 + 124));
  *(_DWORD *)(a1 + 124) = 0;
  *(_DWORD *)(a1 + 128) = 0;
  if ( *(_DWORD *)(a1 + 112) >= 0x10u )
    j__free(*(void **)(a1 + 92));
  *(_DWORD *)(a1 + 112) = 15;
  *(_DWORD *)(a1 + 108) = 0;
  *(_BYTE *)(a1 + 92) = 0;
  if ( *(_DWORD *)(a1 + 84) >= 0x10u )
    j__free(*(void **)(a1 + 64));
  *(_DWORD *)(a1 + 84) = 15;
  *(_DWORD *)(a1 + 80) = 0;
  *(_BYTE *)(a1 + 64) = 0;
  if ( *(_DWORD *)(a1 + 56) >= 0x10u )
    j__free(*(void **)(a1 + 36));
  *(_DWORD *)(a1 + 56) = 15;
  *(_DWORD *)(a1 + 52) = 0;
  *(_BYTE *)(a1 + 36) = 0;
  if ( *(_DWORD *)(a1 + 28) >= 0x10u )
    j__free(*(void **)(a1 + 8));
  *(_DWORD *)(a1 + 28) = 15;
  *(_DWORD *)(a1 + 24) = 0;
  *(_BYTE *)(a1 + 8) = 0;
}
#endif

/* ---- CStringEdPackage_HasEndMarker_m  0x004AAA60 ---- VERIFIED */
int __cdecl CStringEdPackage_HasEndMarker_m(int a1)
{
  return *(_DWORD *)a1;
}

static char se_szDirNameBuf[64];        /* retail 0x0140CAA8 */
static char se_szNoExtBuf[64];          /* retail 0x0140CA68 */
static char se_szBaseNameBuf[64];       /* retail 0x0140CA28 */

/* ---- SE_StripFilename_m  0x004AAB80 ---- VERIFIED */
char *__cdecl SE_StripFilename_m(char *path, int unused)
{
  char *slash;
  char *backslash;

  (void)unused;

  Q_strncpyz(se_szDirNameBuf, path, sizeof(se_szDirNameBuf));

  backslash = strrchr(se_szDirNameBuf, '\\');
  slash     = strrchr(se_szDirNameBuf, '/');
  if ( backslash > slash )
    slash = backslash;
  if ( slash )
    *slash = '\0';

  return se_szDirNameBuf;
}

/* ---- SE_StripExtension_m  0x004AABD0 ---- VERIFIED */
char *__cdecl SE_StripExtension_m(char *path, int unused)
{
  char *dot;
  char *backslash;
  char *slash;

  (void)unused;

  Q_strncpyz(se_szNoExtBuf, path, sizeof(se_szNoExtBuf));

  dot       = strrchr(se_szNoExtBuf, '.');
  backslash = strrchr(se_szNoExtBuf, '\\');
  slash     = strrchr(se_szNoExtBuf, '/');
  if ( dot && (!backslash || dot > backslash) && (!slash || dot > slash) )
    *dot = '\0';

  return se_szNoExtBuf;
}

/* ---- SE_SkipPath_m  0x004AAC30 ---- VERIFIED */
char *__fastcall SE_SkipPath_m(int unused_ecx, char *path, int unused_stack)
{
  const char *base;
  const char *p;

  (void)unused_ecx;
  (void)unused_stack;

  base = path;
  for ( p = path ; *p ; p++ )
  {
    if ( *p == '/' || *p == '\\' )
      base = p + 1;
  }

  Q_strncpyz(se_szBaseNameBuf, base, sizeof(se_szBaseNameBuf));
  return se_szBaseNameBuf;
}

/* ---- SE_GetParentDirName_m  0x004AAC70 ---- VERIFIED */
char *__cdecl SE_GetParentDirName_m(char *path, int unused)
{
  char *dir;

  dir = SE_StripFilename_m(path, unused);
  /* SE_SkipPath_m's first (ecx) parameter is dead in its body -- 0 is exact. */
  return SE_SkipPath_m(0, dir, unused);
}

/* ---- SE_SetPackageFileInfo_m  0x004AAC80 ---- VERIFIED */
BOOL __cdecl SE_SetPackageFileInfo_m(char *a1, _DWORD *a2, int a3)
{
  char *v4;
  char *v6;
  char *v8;
  const char *v9;
  BOOL result;
  char Destination[64];
  unsigned int v13;
  unsigned int retaddr;

  v13 = retaddr ^ _security_cookie;
  v4 = SE_StripExtension_m(a1, (int)a2);
  strcpy(Destination, SE_SkipPath_m(0, v4, (int)a2));
  strupr(Destination);
  sub_498410((int)(a2 + 15), Destination, (void **)strlen(Destination));
  v6 = SE_StripFilename_m(a1, (int)a2);
  v8 = SE_SkipPath_m(0, v6, (int)a2);
  sub_498410((int)(a2 + 22), v8, (void **)strlen(v8));
  if ( a2[28] < 0x10u )
    v9 = (const char *)(a2 + 23);
  else
    v9 = (const char *)a2[23];
  result = _stricmp(v9, "english") == 0;
  a2[29] = result;
  a2[33] = a3;
  return result;
}

/* ---- SE_MatchKeyword_m  0x004AAD50 ---- VERIFIED */
int __cdecl SE_MatchKeyword_m(const char *keyword, const char **pp, int unused)
{
  const char *p;

  (void)unused;

  if ( _strnicmp(keyword, *pp, strlen(keyword)) )
    return 0;

  p = *pp + strlen(keyword);
  while ( *p == ' ' || *p == '\t' )
    p++;
  *pp = p;
  return 1;
}

/* ---- SE_UnescapeNewlines_m  AUTO-STUBBED ---- HIGH */
int SE_UnescapeNewlines_m() { return 0; }
#if 0
void **__cdecl SE_UnescapeNewlines_m(char *a1, int a2)
{
  unsigned int i;
  void **v4;
  void **result;

  if ( (dword_140CB64 & 1) == 0 )
  {
    dword_140CB64 |= 1u;
    dword_140CB60 = 15;
    dword_140CB5C = 0;
    LOBYTE(dword_140CB4C) = 0;
    atexit(sub_53C250);
  }
  sub_498410((int)&dword_140CB48, a1, (void **)strlen(a1));
  for ( i = stl_string_find_m(0, &dword_140CB48, "\\n", strlen("\\n"));
        i != -1;
        i = stl_string_find_m(0, &dword_140CB48, "\\n", strlen("\\n")) )
  {
    v4 = (void **)dword_140CB4C;
    if ( (unsigned int)dword_140CB60 < 0x10 )
      v4 = &dword_140CB4C;
    *((_BYTE *)v4 + i) = 10;
    sub_4984B0((std::_String_base *)&dword_140CB48, i + 1, 1u);
  }
  result = (void **)dword_140CB4C;
  if ( (unsigned int)dword_140CB60 < 0x10 )
    return &dword_140CB4C;
  return result;
}
#endif

/* ---- SE_StripComment_m  0x004AAEA0 ---- VERIFIED */
char *__cdecl SE_StripComment_m(char *line, int unused)
{
  char    *comment;
  char    *segment;
  int      quotes;
  int      running;
  int      i;
  int      n;

  (void)unused;

  segment = line;
  quotes  = 0;

  comment = strstr(line, "//");
  if ( !comment )
    return NULL;

  for ( ;; )
  {
    running = quotes;
    for ( i = 0 ; i < (int)( comment - segment ) ; i++ )
    {
      if ( segment[i] == '"' )
        running++;
    }
    if ( ( running & 1 ) == 0 )
      break;

    segment = comment + 1;
    quotes  = running;
    comment = strstr( comment + 1, "//" );
    if ( !comment )
      return NULL;
  }

  *comment = '\0';

  if ( *segment )
  {
    n = strlen( segment );
    for ( i = n - 1 ; i >= 0 ; i-- )
    {
      if ( !isspace( (unsigned char)segment[i] ) )
        break;
      segment[i] = '\0';
    }
  }

  return comment;
}

/* ---- SE_GetNextLine_m  0x004AAF30 ---- VERIFIED */
int __cdecl SE_GetNextLine_m(const char **pp, char *out, int unused)
{
  const char *cursor;
  const char *nl;
  size_t      len;
  int         i;

  cursor = *pp;
  if ( !*cursor )
    return 0;

  nl = strchr(cursor, '\n');
  if ( nl )
  {
    len = nl - cursor;
    memcpy(out, cursor, len);
    out[len] = '\0';

    cursor += len;
    while ( *cursor && strchr("\r\n", *cursor) )
      cursor++;
    *pp = cursor;
  }
  else
  {
    strcpy(out, cursor);
    *pp = cursor + strlen(cursor);
  }

  if ( *out )
  {
    for ( i = (int)strlen(out) - 1 ; i >= 0 ; i-- )
    {
      if ( !isspace( (unsigned char)out[i] ) )
        break;
      out[i] = '\0';
    }
    SE_StripComment_m(out, unused);            /* 0x004AAEA0 */
  }
  return 1;
}

/* ---- SE_UnquoteAndTrim_m  AUTO-STUBBED ---- HIGH */
int SE_UnquoteAndTrim_m() { return 0; }
#if 0
void **__cdecl SE_UnquoteAndTrim_m(_DWORD *a1, int a2)
{
  unsigned int v3;
  void **result;
  const char *v5;
  unsigned int v6;
  void **v7;
  const char *v8;
  unsigned int v9;
  void **v10;
  const char *v11;
  unsigned int v12;
  void **v13;

  if ( (dword_140CB44 & 1) == 0 )
  {
    dword_140CB44 |= 1u;
    dword_140CB40 = 15;
    dword_140CB3C = 0;
    LOBYTE(dword_140CB2C) = 0;
    atexit(sub_53C280);
  }
  sub_498410((int)&unk_140CB28, &empty_string, (void **)strlen(&empty_string));
  while ( *(_BYTE *)a1 == 32 || *(_BYTE *)a1 == 9 )
    a1 = (_DWORD *)((char *)a1 + 1);
  if ( *(_BYTE *)a1 == 34 )
    a1 = (_DWORD *)((char *)a1 + 1);
  sub_498410((int)&unk_140CB28, a1, (void **)strlen((const char *)a1));
  if ( !*(_BYTE *)a1 )
    goto LABEL_31;
  while ( 1 )
  {
    v3 = dword_140CB40;
    result = (void **)dword_140CB2C;
    v5 = (const char *)dword_140CB2C;
    if ( (unsigned int)dword_140CB40 < 0x10 )
      v5 = (const char *)&dword_140CB2C;
    v6 = strlen(v5);
    v7 = (void **)dword_140CB2C;
    if ( (unsigned int)dword_140CB40 < 0x10 )
      v7 = &dword_140CB2C;
    if ( *((_BYTE *)v7 + v6 - 1) != 32 )
    {
      v8 = (const char *)dword_140CB2C;
      if ( (unsigned int)dword_140CB40 < 0x10 )
        v8 = (const char *)&dword_140CB2C;
      v9 = strlen(v8);
      v10 = (void **)dword_140CB2C;
      if ( (unsigned int)dword_140CB40 < 0x10 )
        v10 = &dword_140CB2C;
      if ( *((_BYTE *)v10 + v9 - 1) != 9 )
        break;
    }
    if ( (unsigned int)dword_140CB40 < 0x10 )
      result = &dword_140CB2C;
    sub_4984B0((std::_String_base *)&unk_140CB28, strlen((const char *)result) - 1, 1u);
  }
  v11 = (const char *)dword_140CB2C;
  if ( (unsigned int)dword_140CB40 < 0x10 )
    v11 = (const char *)&dword_140CB2C;
  v12 = strlen(v11);
  v13 = (void **)dword_140CB2C;
  if ( (unsigned int)dword_140CB40 < 0x10 )
    v13 = &dword_140CB2C;
  if ( *((_BYTE *)v13 + v12 - 1) == 34 )
  {
    if ( (unsigned int)dword_140CB40 < 0x10 )
      result = &dword_140CB2C;
    sub_4984B0((std::_String_base *)&unk_140CB28, strlen((const char *)result) - 1, 1u);
LABEL_31:
    result = (void **)dword_140CB2C;
    v3 = dword_140CB40;
  }
  if ( v3 < 0x10 )
    return &dword_140CB2C;
  return result;
}
#endif

/* ---- SE_FindConfigValue_m  AUTO-STUBBED ---- HIGH */
int __fastcall SE_FindConfigValue_m(int a1, char *a2, int a3)
{ return 0; }
#if 0
int __fastcall SE_FindConfigValue_m(int a1, char *a2, int a3)
{
  int v3;
  int v5; // [esp+10h] [ebp-28h] BYREF
  void *Block;
  int v7;
  unsigned int v8;
  int v9;

  v3 = a3;
  v8 = 15;
  v7 = 0;
  LOBYTE(Block) = 0;
  sub_498410((int)&v5, a2, (void **)strlen(a2));
  v9 = 0;
  sub_4979A0((int **)&a3, v3 + 152, (std::_String_base *)&v5);
  if ( v8 >= 0x10 )
    j__free(Block);
  if ( a3 == *(_DWORD *)(v3 + 156) )
    return 0;
  else
    return *(_DWORD *)(a3 + 40);
}
#endif

/* ---- SE_ParseVersion_m  AUTO-STUBBED ---- */
int SE_ParseVersion_m() { return 0; }
#if 0
// SE_ParseVersion_m -- MEDIUM 55. VERSION handler; called only by SE_ParseLine_m.
int __cdecl SE_ParseVersion_m(char *this, _DWORD *a2, int a3)
{
  struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
  _DWORD *v4;
  int v6;
  int v7;
  int v8;
  const char *v9;
  char *v10;
  int result;
  void **v12; // [esp+0h] [ebp-44h] BYREF
  void *Block;
  int v14;
  unsigned int v15;
  int v16; // [esp+1Ch] [ebp-28h] BYREF
  void *v17;
  int v18;
  unsigned int v19;
  struct _EXCEPTION_REGISTRATION_RECORD *v20;
  void *v21;
  int v22;

  v22 = -1;
  ExceptionList = NtCurrentTeb()->NtTib.ExceptionList;
  v21 = &loc_53BB88;
  v20 = ExceptionList;
  v4 = a2;
  v6 = SE_FindConfigValue_m((int)this, this, (int)a2);
  if ( !v6 )
  {
    v15 = 15;
    v14 = 0;
    LOBYTE(Block) = 0;
    sub_498410((int)&v12, this, (void **)strlen(this));
    v22 = 0;
    stl_vector_push_back_m((int)(a2 + 34), &v12);
    v22 = -1;
    if ( v15 >= 0x10 )
      j__free(Block);
    v7 = a2[35];
    if ( v7 )
      v8 = (a2[36] - v7) / 28;
    else
      LOBYTE(v8) = 0;
    v6 = 1 << (v8 - 1);
    v15 = 15;
    v14 = 0;
    LOBYTE(Block) = 0;
    sub_498410((int)&v12, this, (void **)strlen(this));
    v22 = 1;
    *(_DWORD *)sub_497890((int)(a2 + 38), &v12) = v6;
    v22 = -1;
    if ( v15 >= 0x10 )
      j__free(Block);
    v4 = a2;
  }
  if ( v4[21] < 0x10u )
    v9 = (const char *)(v4 + 16);
  else
    v9 = (const char *)v4[16];
  v10 = va("%s_%s", v9, (const char *)a3);
  v19 = 15;
  v18 = 0;
  LOBYTE(v17) = 0;
  sub_498410((int)&v16, v10, (void **)strlen(v10));
  v22 = 2;
  stl_map_find_m((int **)&a3, (int)(a2 + 30), (std::_String_base *)&v16);
  if ( v19 >= 0x10 )
    j__free(v17);
  result = a3;
  if ( a3 != a2[31] )
    *(_DWORD *)(a3 + 96) |= v6;
  return result;
}
#endif

/* ---- SE_ParseLine_m  AUTO-STUBBED ---- */
int SE_ParseLine_m() { return 0; }
#if 0
// SE_ParseLine_m -- MEDIUM 60. Dispatches the VERSION / CONFIG / FILENOTES / REFERENCE keywords; "Unexpected version number %d, expecting %d!".
const char *__cdecl SE_ParseLine_m(_DWORD *a1, char *Source)
{
  void **v2;
  int v3;
  char *v5;
  _BYTE *v6;
  char *i;
  char *v8;
  char v9;
  const char *v10;
  const char *j;
  size_t v12;
  char *v13;
  char *v14;
  int v15;
  BOOL v16;
  const char *v17;
  const char *v18;
  char *v20;
  const char *v21;
  char Destination; // [esp+10h] [ebp-408h] BYREF
  _BYTE v23[1020]; // [esp+11h] [ebp-407h] BYREF
  __int16 v24;
  char v25;
  unsigned int v26;
  unsigned int retaddr;

  v26 = retaddr ^ _security_cookie;
  v20 = 0;
  if ( Source )
  {
    if ( SE_MatchKeyword_m("VERSION", (const char **)&Source, (int)a1) )
    {
      v2 = SE_UnquoteAndTrim_m(Source, (int)a1);
      v3 = j__atol((const char *)v2);
      if ( v3 != 1 )
        return va("Unexpected version number %d, expecting %d!\n", v3, 1);
    }
    else if ( !SE_MatchKeyword_m("CONFIG", (const char **)&Source, (int)a1)
           && !SE_MatchKeyword_m("FILENOTES", (const char **)&Source, (int)a1)
           && !SE_MatchKeyword_m("NOTES", (const char **)&Source, (int)a1) )
    {
      if ( SE_MatchKeyword_m("REFERENCE", (const char **)&Source, (int)a1) )
      {
        v5 = (char *)SE_UnquoteAndTrim_m(Source, (int)a1);
        SE_ParseConfig_m((int)a1, v5);
      }
      else if ( SE_MatchKeyword_m("FLAGS", (const char **)&Source, (int)a1) )
      {
        if ( a1[7] < 0x10u )
          v6 = a1 + 2;
        else
          v6 = (_BYTE *)a1[2];
        if ( *v6 )
        {
          Destination = 0;
          memset(v23, 0, sizeof(v23));
          v24 = 0;
          v25 = 0;
          strncpy(&Destination, Source, 0x3FFu);
          for ( i = strtok_m(&Destination, " \t"); i; i = strtok_m(0, " \t") )
          {
            strupr(i);
            SE_ParseVersion_m(i, a1, (int)v6);
          }
        }
        else
        {
          return "Error parsing file: Unexpected \"FLAGS\"\n";
        }
      }
      else if ( SE_MatchKeyword_m("ENDMARKER", (const char **)&Source, (int)a1) )
      {
        *a1 = 1;
      }
      else
      {
        v8 = Source;
        if ( _strnicmp("LANG_", Source, 5u) )
          return va("Unknown keyword at linestart: \"%s\"\n", v8);
        if ( a1[7] < 0x10u )
          v21 = (const char *)(a1 + 2);
        else
          v21 = (const char *)a1[2];
        if ( *v21 )
        {
          v9 = v8[5];
          v10 = v8 + 5;
          for ( j = v10; v9; v9 = *++j )
          {
            if ( v9 == 32 )
              break;
            if ( v9 == 9 )
              break;
          }
          Destination = 0;
          memset(v23, 0, sizeof(v23));
          v24 = 0;
          v12 = j - v10;
          v25 = 0;
          if ( v12 > 0x3FF )
            v12 = 1023;
          strncpy(&Destination, v10, v12);
          v13 = (char *)SE_UnquoteAndTrim_m(&v10[strlen(&Destination)], (int)a1);
          v14 = (char *)SE_UnescapeNewlines_m(v13, (int)a1);
          if ( a1[29] )
          {
            SE_ParseLanguageString_m(v21, (int)a1, v14, 0);
          }
          else
          {
            v15 = _stricmp(&Destination, "english");
            v16 = v15 == 0;
            if ( !v15
              || (a1[28] < 0x10u ? (v17 = (const char *)(a1 + 23)) : (v17 = (const char *)a1[23]),
                  !_stricmp(v17, &Destination)
               || (a1[28] < 0x10u ? (v18 = (const char *)(a1 + 23)) : (v18 = (const char *)a1[23]),
                   (v20 = va("Language \"%s\" found when expecting \"%s\"!\n", &Destination, v18)) == 0)) )
            {
              SE_ParseLanguageString_m(v21, (int)a1, v14, v16);
            }
          }
        }
        else
        {
          return "Error parsing file: Unexpected \"LANG_\"\n";
        }
      }
    }
  }
  return v20;
}
#endif

/* ---- CStringEdPackage_GetFileName_m  0x004AB780 ---- MEDIUM */
int __cdecl CStringEdPackage_GetFileName_m(int a1)
{
  if ( *(_DWORD *)(a1 + 28) < 0x10u )
    return a1 + 8;
  else
    return *(_DWORD *)(a1 + 8);
}

/* ---- SE_ParseConfig_m  AUTO-STUBBED ---- */
void ****__stdcall SE_ParseConfig_m(int a1, char *a2)
{ return 0; }
#if 0
// SE_ParseConfig_m -- MEDIUM 55. CONFIG handler; builds the "%s_%s" language-qualified key.
void ****__stdcall SE_ParseConfig_m(int a1, char *a2)
{
  int v2;
  const char *v3;
  char *v4;
  bool v5; // cf
  const char *v6;
  char *v7;
  int v8;
  int *v10; // [esp+Ch] [ebp-88h] BYREF
  void *v11; // [esp+10h] [ebp-84h] BYREF
  void *v12;
  int v13;
  unsigned int v14;
  int v15; // [esp+2Ch] [ebp-68h] BYREF
  void *Block;
  int v17;
  unsigned int v18;
  void ***v19; // [esp+48h] [ebp-4Ch] BYREF
  void *v20;
  int v21;
  unsigned int v22;
  void ***v23; // [esp+64h] [ebp-30h] BYREF
  void *v24;
  int v25;
  unsigned int v26;
  int v27;
  unsigned int v28;
  int v29;
  unsigned int retaddr;

  v2 = a1;
  v28 = retaddr ^ _security_cookie;
  if ( *(_DWORD *)(a1 + 84) < 0x10u )
    v3 = (const char *)(a1 + 64);
  else
    v3 = *(const char **)(a1 + 64);
  v18 = 15;
  v17 = 0;
  LOBYTE(Block) = 0;
  v4 = va("%s_%s", v3, a2);
  sub_498410((int)&v15, v4, (void **)strlen(v4));
  v29 = 0;
  stl_map_find_m(&v10, a1 + 120, (std::_String_base *)&v15);
  v29 = -1;
  if ( v18 >= 0x10 )
    j__free(Block);
  if ( v10 == *(int **)(a1 + 124) )
  {
    v22 = 15;
    v21 = 0;
    LOBYTE(v20) = 0;
    v26 = 15;
    v25 = 0;
    LOBYTE(v24) = 0;
    v27 = 0;
    v5 = *(_DWORD *)(a1 + 84) < 0x10u;
    v29 = 1;
    if ( v5 )
      v6 = (const char *)(a1 + 64);
    else
      v6 = *(const char **)(a1 + 64);
    v7 = va("%s_%s", v6, a2);
    v14 = 15;
    v13 = 0;
    LOBYTE(v12) = 0;
    sub_498410((int)&v11, v7, (void **)strlen(v7));
    LOBYTE(v29) = 2;
    v8 = stl_map_index_m(a1 + 120, &v11);
    sub_497C10((void ****)v8, &v19, 0, (void **)0xFFFFFFFF);
    sub_497C10((void ****)(v8 + 28), &v23, 0, (void **)0xFFFFFFFF);
    *(_DWORD *)(v8 + 56) = v27;
    if ( v14 >= 0x10 )
      j__free(v12);
    v14 = 15;
    v13 = 0;
    LOBYTE(v12) = 0;
    v29 = -1;
    if ( v26 >= 0x10 )
      j__free(v24);
    v26 = 15;
    v25 = 0;
    LOBYTE(v24) = 0;
    if ( v22 >= 0x10 )
      j__free(v20);
    v2 = a1;
    v22 = 15;
    v21 = 0;
    LOBYTE(v20) = 0;
  }
  return sub_498410(v2 + 4, a2, (void **)strlen(a2));
}
#endif

/* ---- CStringEdString_Copy  0x004ABA40 ---- HIGH */
int __cdecl CStringEdString_Copy(int a1, int a2)
{
  sub_497C10((void ****)a2, (void ****)a1, 0, (void **)0xFFFFFFFF);
  sub_497C10((void ****)(a2 + 28), (void ****)(a1 + 28), 0, (void **)0xFFFFFFFF);
  *(_DWORD *)(a2 + 56) = *(_DWORD *)(a1 + 56);
  return a2;
}

/* ---- SE_ParseLanguageString_m  AUTO-STUBBED ---- */
int SE_ParseLanguageString_m() { return 0; }
#if 0
// SE_ParseLanguageString_m -- MEDIUM 58. Per-language string handler; "#same" aliases the entry to the reference language.
void ****__cdecl SE_ParseLanguageString_m(const char *this, int a2, char *String1, int a4)
{
  struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
  int v5;
  const char *v6;
  char *v7;
  void ****result;
  int v9;
  void ****v10;
  char *v11;
  int v12;
  int v13;
  _DWORD *v14;
  char *v15;
  std::_String_base *v16;
  int v17; // [esp+0h] [ebp-28h] BYREF
  void *Block;
  int v19;
  unsigned int v20;
  struct _EXCEPTION_REGISTRATION_RECORD *v21;
  void *v22;
  int v23;

  v23 = -1;
  ExceptionList = NtCurrentTeb()->NtTib.ExceptionList;
  v22 = &loc_53B9F8;
  v21 = ExceptionList;
  v5 = a2;
  if ( *(_DWORD *)(a2 + 84) < 0x10u )
    v6 = (const char *)(a2 + 64);
  else
    v6 = *(const char **)(a2 + 64);
  v20 = 15;
  v19 = 0;
  LOBYTE(Block) = 0;
  v7 = va("%s_%s", v6, this);
  sub_498410((int)&v17, v7, (void **)strlen(v7));
  v23 = 0;
  stl_map_find_m((int **)&a2, v5 + 120, (std::_String_base *)&v17);
  v23 = -1;
  if ( v20 >= 0x10 )
    j__free(Block);
  result = (void ****)a2;
  v9 = *(_DWORD *)(v5 + 124);
  v20 = 15;
  v19 = 0;
  LOBYTE(Block) = 0;
  if ( a2 != v9 )
  {
    v10 = (void ****)(a2 + 40);
    if ( a4 || *(_DWORD *)(v5 + 116) )
    {
      v15 = String1;
      sub_498410((int)v10, String1, (void **)strlen(String1));
      if ( *(_DWORD *)(v5 + 132) )
      {
        v16 = (std::_String_base *)(v10 + 7);
        sub_498410((int)v16, "[", (void **)strlen("["));
        stl_string_append_m(strlen(v15), v16, v15);
        stl_string_append_m(strlen("]"), v16, "]");
      }
      return sub_498410(v5 + 32, v15, (void **)strlen(v15));
    }
    else
    {
      v11 = String1;
      if ( !_stricmp(String1, "#same") )
      {
        result = sub_497C10(v10, (void ****)(v5 + 32), 0, (void **)0xFFFFFFFF);
        if ( *(_DWORD *)(v5 + 132) )
        {
          v14 = v10 + 7;
          stl_string_assign_cstr_m(v13, "[", (int)v14);
          stl_string_append_cstr_m("#same", v14);
          return (void ****)stl_string_append_cstr_m("]", v14);
        }
      }
      else
      {
        return (void ****)stl_string_assign_cstr_m(v12, v11, (int)v10);
      }
    }
  }
  return result;
}
#endif

/* ---- SE_LoadLanguageFile_m  AUTO-STUBBED ---- */
int SE_LoadLanguageFile_m() { return 0; }
#if 0
// SE_LoadLanguageFile_m -- MEDIUM 62. FS_ReadFile then a line loop over a 16 KB buffer until the ENDMARKER token; errors are "Truncated file, failed to find \"%s\" at file end!" and "Unable to load \"%s\"!". The SE_/SEH_ prefix is anchored by the neighbouring mangled symbols SE_GetNumLanguages__Fv and SEH_StringEd_SetLanguageStrings; only the suffix is inferred.
char *__cdecl SE_LoadLanguageFile_m(int a1, int a2)
{
  const char *v2;
  char *v3;
  const char *v4;
  void *v5;
  void *Block; // [esp+Ch] [ebp-400Ch] BYREF
  char Source[16384]; // [esp+14h] [ebp-4004h] BYREF
  unsigned int v10;
  unsigned int retaddr;

  v10 = retaddr ^ _security_cookie;
  v3 = (char *)v2;
  v4 = 0;
  if ( FS_ReadFile(v2, &Block) > 0 && (v5 = Block) != 0 )
  {
    SE_SetPackageFileInfo_m(v3, TheStringPackage, a1);
    do
    {
      if ( !SE_GetNextLine_m((const char **)&Block, Source, (int)TheStringPackage) )
        break;
      if ( strlen(Source) )
        v4 = SE_ParseLine_m(TheStringPackage, Source);
    }
    while ( !v4 );
    FS_FreeFile(v5);
    if ( !v4 && !TheStringPackage[0] )
      return va("Truncated file, failed to find \"%s\" at file end!", "ENDMARKER");
  }
  else if ( !a2 )
  {
    return va("Unable to load \"%s\"!", v3);
  }
  return (char *)v4;
}
#endif

/* ---- SE_NextSemicolonToken_m  AUTO-STUBBED ---- HIGH */
int SE_NextSemicolonToken_m() { return 0; }
#if 0
char *__cdecl SE_NextSemicolonToken_m(char *a1)
{
  unsigned int v1;
  char *v2;
  const char *v3;
  char *v5;

  v1 = *((_DWORD *)a1 + 6);
  v2 = a1 + 4;
  if ( v1 < 0x10 )
    v3 = a1 + 4;
  else
    v3 = *(const char **)v2;
  if ( !strlen(v3) )
    return 0;
  if ( v1 >= 0x10 )
    v2 = *(char **)v2;
  strncpy(se_szTokenBuf, v2, 0x3Fu);
  byte_140CB27 = 0;
  v5 = strchr(se_szTokenBuf, 59);
  if ( v5 )
  {
    *v5 = 0;
    sub_4984B0((std::_String_base *)a1, 0, v5 - se_szTokenBuf + 1);
  }
  else
  {
    sub_4984B0((std::_String_base *)a1, 0, 0xFFFFFFFF);
  }
  return se_szTokenBuf;
}
#endif

/* ---- SE_LoadStringEditFile_m  0x004ABDF0 ---- HIGH */
char *__cdecl SE_LoadStringEditFile_m(char *Source, int a2)
{
  char *LanguageFile_m;
  char *v4;
  char Destination[64];
  unsigned int v7;
  unsigned int retaddr;

  v7 = retaddr ^ _security_cookie;
  LanguageFile_m = SE_LoadLanguageFile_m(a2, 0);
  if ( LanguageFile_m )
    return LanguageFile_m;
  strncpy(Destination, Source, 0x3Fu);
  Destination[63] = 0;
  v4 = strrchr(Destination, 46);
  if ( !v4 || strlen(v4) != 4 )
    return LanguageFile_m;
  strcpy(v4, ".ste");
  return SE_LoadLanguageFile_m(a2, 1);
}

/* ---- SE_GetStringForLanguage_m  0x004ABEA0 ---- MEDIUM */
int *__cdecl SE_GetStringForLanguage_m(const char *prefix, const char *a1, int a2)
{
  char Buffer[256];
  unsigned int v5;
  unsigned int retaddr;

  v5 = retaddr ^ _security_cookie;
  sprintf(Buffer, "%s_%s", prefix, a1);
  return sub_4ABF00(Buffer, a2);
}

/* ---- SE_LookupStringForLanguage_m  0x004AC010 ---- MEDIUM */
int __cdecl SE_LookupStringForLanguage_m(const char *prefix, const char *a1)
{
  char Buffer[256];
  unsigned int v5;
  unsigned int retaddr;

  v5 = retaddr ^ _security_cookie;
  sprintf(Buffer, "%s_%s", prefix, a1);
  return SE_LookupString_m(0, Buffer);
}

/* ---- SE_LookupString_m  AUTO-STUBBED ---- HIGH  NO DEFINITION EMITTED */
/* ---- SE_GetNumLanguageDirs_m  0x004AC120 ---- */
int SE_GetNumLanguageDirs_m()
{
  int result;

  result = seLanguageDirVectorFirst;
  if ( seLanguageDirVectorFirst )
    return (seLanguageDirVectorLast - seLanguageDirVectorFirst) / 28;
  return result;
}

/* ---- SE_GetLanguageDirName_m  0x004AC150 ---- HIGH */
char *__cdecl SE_GetLanguageDirName_m(unsigned int a1)
{
  unsigned int v1;

  if ( !seLanguageDirVectorFirst || a1 >= (seLanguageDirVectorLast - seLanguageDirVectorFirst) / 28 )
    return &empty_string;
  v1 = seLanguageDirVectorFirst + 28 * a1;
  if ( *(_DWORD *)(v1 + 24) < 0x10u )
    return (char *)(v1 + 4);
  else
    return *(char **)(v1 + 4);
}

/* ---- SE_GetConfigValue_m  0x004AC1A0 ---- HIGH */
int __fastcall SE_GetConfigValue_m(int a1, char *a2)
{
  return SE_FindConfigValue_m(a1, a2, (int)TheStringPackage);
}

/* ---- SE_GetStrFileName_m  0x004AC6C0 ---- HIGH */
char *__cdecl SE_GetStrFileName_m(unsigned int a1)
{
  char *v1;

  if ( !seStrFileVectorFirst || a1 >= (seStrFileVectorLast - (int)seStrFileVectorFirst) / 28 )
    return &empty_string;
  v1 = (char *)seStrFileVectorFirst + 28 * a1;
  if ( *((_DWORD *)v1 + 6) < 0x10u )
    return v1 + 4;
  else
    return (char *)*((_DWORD *)v1 + 1);
}

/* ---- SE_GetLanguageFilePath_m  0x004AC710 ---- VERIFIED */
char *__cdecl SE_GetLanguageFilePath_m(unsigned int a1)
{
  char *v1;
  const char *v2;

  if ( !seStrFileVectorFirst || a1 >= (seStrFileVectorLast - (int)seStrFileVectorFirst) / 28 )
    return &empty_string;
  v1 = (char *)seStrFileVectorFirst + 28 * a1;
  if ( *((_DWORD *)v1 + 6) < 0x10u )
    v2 = v1 + 4;
  else
    v2 = (const char *)*((_DWORD *)v1 + 1);
  return va("%s/%s", "localizedstrings", v2);
}

/* ---- SE_NewLanguage_m  0x004AC780 ---- VERIFIED */
int SE_NewLanguage_m(void)
{
  return Clear__16CStringEdPackageFi(&TheStringPackage, 1);
}

/* ---- SE_Init_m  0x004AC790 ---- VERIFIED */
int SE_Init_m(void)
{
  return Clear__16CStringEdPackageFi(&TheStringPackage, 0);
}

/* ---- SE_ShutDown_m  0x004AC7A0 ---- VERIFIED */
int SE_ShutDown_m(void)
{
  return Clear__16CStringEdPackageFi(&TheStringPackage, 0);
}

/* ---- stl_string_assign_cstr_m  0x004AC930 ---- MEDIUM */
void ****__fastcall stl_string_assign_cstr_m(int a1, char *a2, int a3)
{
  return sub_498410(a3, a2, (void **)strlen(a2));
}

/* ---- stl_string_append_cstr_m  AUTO-STUBBED ---- MEDIUM */
int stl_string_append_cstr_m() { return 0; }
#if 0
std::_String_base *__cdecl stl_string_append_cstr_m(char *this, std::_String_base *a2)
{
  return stl_string_append_m(strlen(this), a2, this);
}
#endif

/* ---- stl_string_at_m  0x004AC970 ---- MEDIUM */
int __cdecl stl_string_at_m(int a1, int a2)
{
  if ( *(_DWORD *)(a1 + 24) < 0x10u )
    return a1 + a2 + 4;
  else
    return a2 + *(_DWORD *)(a1 + 4);
}

/* ---- stl_string_find_cstr_m  0x004AC990 ---- MEDIUM */
unsigned int __fastcall stl_string_find_cstr_m(int a1, char *a2, _DWORD *a3, unsigned int a4)
{
  return stl_string_find_m(a4, a3, a2, strlen(a2));
}

/* ---- stl_map_construct_m  0x004AC9C0 ---- MEDIUM */
int __cdecl stl_map_construct_m(int a1)
{
  _DWORD *v1;

  *(_BYTE *)a1 = 0;
  v1 = stl_map_buynode_str_m(a1);
  *(_DWORD *)(a1 + 4) = v1;
  *((_BYTE *)v1 + 101) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  **(_DWORD **)(a1 + 4) = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(a1 + 8) = 0;
  return a1;
}

/* ---- stl_map_index_m  0x004AC9F0 ---- HIGH */
int __stdcall stl_map_index_m(int a1, void ***a2)
{
  _DWORD *v2;
  int v3;
  _BYTE v5[4];
  void *v6;
  int v7;
  unsigned int v8;
  void *Block;
  int v10;
  unsigned int v11;
  int v12;
  _BYTE v13[8];
  _BYTE v14[88];
  int v15;

  v8 = 15;
  v7 = 0;
  LOBYTE(v6) = 0;
  v11 = 15;
  v10 = 0;
  LOBYTE(Block) = 0;
  v12 = 0;
  v15 = 0;
  v2 = (_DWORD *)stl_map_pair_construct_m((int)v5, (int)v14, a2);
  LOBYTE(v15) = 1;
  v3 = *(_DWORD *)stl_map_insert_m(a1, (int)v13, v2);
  stl_map_pair_destroy_m((int)v14);
  if ( v11 >= 0x10 )
    j__free(Block);
  v11 = 15;
  v10 = 0;
  LOBYTE(Block) = 0;
  if ( v8 >= 0x10 )
    j__free(v6);
  return v3 + 40;
}

/* ---- stl_map_tidy_m  0x004ACAE0 ---- HIGH */
int __cdecl stl_map_tidy_m(int a1)
{
  int result;
  int *v3;

  stl_map_erase_range_m(a1, &v3, **(int ***)(a1 + 4), *(int **)(a1 + 4));
  j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- stl_map_end_m  0x004ACB10 ---- MEDIUM */
_DWORD *__cdecl stl_map_end_m(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 4);
  return result;
}

/* ---- stl_map_clear_m  0x004ACB20 ---- HIGH */
_DWORD *__cdecl stl_map_clear_m(int a1)
{
  _DWORD *result;

  stl_tree_erase_all_m(*(void ***)(*(_DWORD *)(a1 + 4) + 4));
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  result = *(_DWORD **)(a1 + 4);
  *(_DWORD *)(a1 + 8) = 0;
  *result = result;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
  return result;
}

/* ---- stl_map_find_m  AUTO-STUBBED ---- HIGH */
int stl_map_find_m() { return 0; }
#if 0
int **__cdecl stl_map_find_m(int **a1, int a2, std::_String_base *a3)
{
  int *v3;
  unsigned int v4;
  const char *v5;
  int *v7;

  v3 = stl_map_lower_bound_m(a2, (int)a3);
  v7 = v3;
  if ( v3 == *(int **)(a2 + 4)
    || ((v4 = v3[8], (unsigned int)v3[9] < 0x10) ? (v5 = (const char *)(v3 + 4)) : (v5 = (const char *)v3[4]),
        sub_499AB0(*((_DWORD *)a3 + 5), a3, 0, v5, v4) < 0) )
  {
    *a1 = *(int **)(a2 + 4);
    return a1;
  }
  else
  {
    *a1 = v7;
    return a1;
  }
}
#endif

/* ---- sub_4ACBB0  0x004ACBB0 ---- MEDIUM */
BOOL __cdecl sub_4ACBB0(_DWORD *a1, _DWORD *a2)
{
  return *a2 == *a1;
}

/* ---- sub_4ACBC0  0x004ACBC0 ---- MEDIUM */
BOOL __cdecl sub_4ACBC0(_DWORD *a1, _DWORD *a2)
{
  return *a1 != *a2;
}

/* ---- sub_4ACBD0  0x004ACBD0 ---- MEDIUM */
int __cdecl sub_4ACBD0(_DWORD *a1)
{
  return *a1 + 12;
}

/* ---- stl_vector_construct_m  0x004ACBE0 ---- HIGH */
_DWORD *__cdecl stl_vector_construct_m(_DWORD *result)
{
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;
  return result;
}

/* ---- stl_vector_begin_m  0x004ACC30 ---- MEDIUM */
_DWORD *__cdecl stl_vector_begin_m(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 4);
  return result;
}

/* ---- stl_vector_size_m  0x004ACC40 ---- HIGH */
int __cdecl stl_vector_size_m(_DWORD *this)
{
  int result;

  result = this[1];
  if ( result )
    return (this[2] - result) / 28;
  return result;
}

/* ---- stl_vector_at_m  0x004ACC70 ---- MEDIUM */
int __cdecl stl_vector_at_m(int a1, int a2)
{
  return *(_DWORD *)(a2 + 4) + 28 * a1;
}

/* ---- stl_vector_push_back_m  0x004ACC80 ---- HIGH */
void ***__cdecl stl_vector_push_back_m(int a1, void *a2)
{
  int v3;
  unsigned int v4;
  int v5;
  void ***result;

  v3 = *(_DWORD *)(a1 + 4);
  if ( v3 )
    v4 = (*(_DWORD *)(a1 + 8) - v3) / 28;
  else
    v4 = 0;
  if ( !v3 || v4 >= (*(_DWORD *)(a1 + 12) - v3) / 28 )
    return (void ***)stl_vector_insert_at_m(a1, &a2, *(void ****)(a1 + 8), a2);
  v5 = *(_DWORD *)(a1 + 8);
  result = stl_vector_ufill_n_impl_m(v5, (void ***)a2);
  *(_DWORD *)(a1 + 8) = v5 + 28;
  return result;
}

/* ---- stl_vector_insert_at_m  AUTO-STUBBED ---- */
int stl_vector_insert_at_m() { return 0; }
#if 0
_DWORD *__cdecl stl_vector_insert_at_m(int a1, _DWORD *a2, void ***a3, void *a4)
{
  int v4;
  int v5;

  v4 = *(_DWORD *)(a1 + 4);
  if ( v4 && (*(_DWORD *)(a1 + 8) - v4) / 28 )
    v5 = ((int)a3 - v4) / 28;
  else
    v5 = 0;
  stl_vector_insert_n_m(a1, a3, (char *)1, (unsigned int)a4);
  *a2 = *(_DWORD *)(a1 + 4) + 28 * v5;
  return a2;
}
#endif

/* ---- stl_vector_tidy_m  0x004ACD80 ---- HIGH */
void __cdecl stl_vector_tidy_m(int *a1)
{
  int v1;

  v1 = a1[1];
  if ( v1 )
  {
    stl_vector_destroy_range_impl_m(v1, a1[2]);
    j__free((void *)a1[1]);
  }
  a1[1] = 0;
  a1[2] = 0;
  a1[3] = 0;
}

/* ---- stl_map44_construct_alloccopy_m  0x004ACDC0 ---- HIGH */
int __cdecl stl_map44_construct_alloccopy_m(int a1)
{
  _DWORD *v1;

  *(_BYTE *)a1 = 0;
  v1 = stl_map44_buynode_m(a1);
  *(_DWORD *)(a1 + 4) = v1;
  *((_BYTE *)v1 + 41) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  **(_DWORD **)(a1 + 4) = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(a1 + 8) = 0;
  return a1;
}

/* ---- stl_map44_tidy_m  0x004ACDF0 ---- HIGH */
int __cdecl stl_map44_tidy_m(int a1)
{
  int result;
  int *v3;

  stl_map44_erase_range_m(a1, &v3, **(int ***)(a1 + 4), *(int **)(a1 + 4));
  j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- sub_4ACE20  AUTO-STUBBED ---- */
int __stdcall sub_4ACE20(int a1, int a2, void ***a3)
{ return 0; }
#if 0
int __stdcall sub_4ACE20(int a1, int a2, void ***a3)
{
  int v3;
  void **v4;
  char v5;
  void ***v6;
  char v7;
  void ***v8;
  void **v9;
  unsigned int v10;
  const char *v11;
  void **v12;
  int v13;
  const char *v14;
  int v15;
  bool v16; // sf
  int v17;
  void ***v18;
  void **v19;
  int result;
  const char *v21;
  void **v22;
  char v23;
  void ***v24;

  v3 = a1;
  v4 = *(void ***)(*(_DWORD *)(a1 + 4) + 4);
  v5 = *((_BYTE *)v4 + 41);
  v6 = *(void ****)(a1 + 4);
  v7 = 1;
  v8 = a3;
  v23 = 1;
  if ( !v5 )
  {
    v9 = a3[5];
    do
    {
      v10 = (unsigned int)v4[8];
      v24 = (void ***)v4;
      if ( (unsigned int)v4[9] < 0x10 )
        v11 = (const char *)(v4 + 4);
      else
        v11 = (const char *)v4[4];
      v12 = v9;
      if ( !v9 )
        goto LABEL_13;
      v13 = (int)v9;
      if ( (unsigned int)v9 >= v10 )
        v13 = (int)v4[8];
      v14 = (unsigned int)v8[6] < 0x10 ? (const char *)(v8 + 1) : (const char *)v8[1];
      v15 = memcmp(v14, v11, v13);
      v16 = v15 < 0;
      v9 = a3[5];
      v8 = a3;
      if ( !v15 )
      {
LABEL_13:
        if ( (unsigned int)v12 >= v10 )
          v17 = v12 != (void **)v10;
        else
          v17 = -1;
        v16 = v17 < 0;
      }
      v7 = v16;
      v23 = v16;
      if ( v16 )
        v4 = (void **)*v4;
      else
        v4 = (void **)v4[2];
      v5 = *((_BYTE *)v4 + 41);
    }
    while ( !v5 );
    v3 = a1;
    v6 = v24;
  }
  v18 = v6;
  a3 = v6;
  if ( v7 )
  {
    if ( v6 == **(void *****)(v3 + 4) )
    {
      v19 = *stl_map44_insert_rebalance_m(v6, v3, (void ***)&a3, 1, (void **)v8);
      result = a2;
      *(_DWORD *)a2 = v19;
      *(_BYTE *)(a2 + 4) = 1;
      return result;
    }
    stl_map44_iter_prev_m(v5, (int **)&a3);
    v18 = a3;
  }
  if ( (unsigned int)v8[6] < 0x10 )
    v21 = (const char *)(v8 + 1);
  else
    v21 = (const char *)v8[1];
  if ( sub_499AB0((unsigned int)v18[8], (std::_String_base *)(v18 + 3), 0, v21, (unsigned int)v8[5]) >= 0 )
  {
    result = a2;
    *(_DWORD *)a2 = v18;
    *(_BYTE *)(a2 + 4) = 0;
  }
  else
  {
    v22 = *stl_map44_insert_rebalance_m(v6, v3, (void ***)&a3, v23, (void **)v8);
    result = a2;
    *(_DWORD *)a2 = v22;
    *(_BYTE *)(a2 + 4) = 1;
  }
  return result;
}
#endif

/* ---- stl_map44_count_m  0x004ACF80 ---- HIGH */
int __cdecl stl_map44_count_m(_DWORD *a1, int a2)
{
  int *v3;
  int *v4;
  int v6;
  int *i;

  v3 = stl_map44_upper_bound_m(a2, a1);
  v4 = stl_map44_lower_bound_m(a2, (int)a1);
  v6 = 0;
  /* stl_map44_iter_next_m is the map iterator increment; its ecx parameter is dead. */
  for ( i = v4; i != v3; stl_map44_iter_next_m(0, &i) )
    ++v6;
  return v6;
}

/* ---- stl_map_pair_destroy_m  AUTO-STUBBED ---- */
int stl_map_pair_destroy_m() { return 0; }
#if 0
void __cdecl stl_map_pair_destroy_m(int a1)
{
  if ( *(_DWORD *)(a1 + 80) >= 0x10u )
    j__free(*(void **)(a1 + 60));
  *(_DWORD *)(a1 + 80) = 15;
  *(_DWORD *)(a1 + 76) = 0;
  *(_BYTE *)(a1 + 60) = 0;
  if ( *(_DWORD *)(a1 + 52) >= 0x10u )
    j__free(*(void **)(a1 + 32));
  *(_DWORD *)(a1 + 52) = 15;
  *(_DWORD *)(a1 + 48) = 0;
  *(_BYTE *)(a1 + 32) = 0;
  if ( *(_DWORD *)(a1 + 24) >= 0x10u )
    j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 20) = 0;
  *(_DWORD *)(a1 + 24) = 15;
  *(_BYTE *)(a1 + 4) = 0;
}
#endif

/* ---- stl_string_append_cstr2_m  AUTO-STUBBED ---- */
int stl_string_append_cstr2_m() { return 0; }
#if 0
std::_String_base *__cdecl stl_string_append_cstr2_m(char *this, std::_String_base *a2)
{
  return stl_string_append_m(strlen(this), a2, this);
}
#endif

/* ---- stl_string_find_m  0x004AD050 ---- HIGH */
unsigned int __cdecl stl_string_find_m(unsigned int result, _DWORD *a2, char *a3, unsigned int a4)
{
  unsigned int v4;
  unsigned int v5;
  unsigned int v6;
  size_t v7;
  _DWORD *v8;
  _BYTE *v9;
  char *v10;
  char *v11;

  v4 = a4;
  if ( a4 || result > a2[5] )
  {
    v5 = a2[5];
    if ( result < v5
      && (v6 = v5 - result, a4 <= v6)
      && ((v7 = 1 - a4 + v6, a2[6] < 0x10u) ? (v8 = a2 + 1) : (v8 = (_DWORD *)a2[1]),
          v9 = (char *)v8 + result,
          (v10 = (char *)memchr(v9, *a3, v7)) != 0) )
    {
      while ( memcmp(v10, a3, v4) )
      {
        v7 = v7 + v9 - v10 - 1;
        v9 = v10 + 1;
        v10 = (char *)memchr(v10 + 1, *a3, v7);
        if ( !v10 )
          return -1;
        v4 = a4;
      }
      v11 = (char *)(a2 + 1);
      if ( a2[6] >= 0x10u )
        v11 = (char *)a2[1];
      return v10 - v11;
    }
    else
    {
      return -1;
    }
  }
  return result;
}

/* ---- sub_4AD120  0x004AD120 ---- MEDIUM */
int __cdecl sub_4AD120(int a1)
{
  return a1 + 12;
}

/* ---- sub_4AD130  0x004AD130 ---- MEDIUM */
int __cdecl sub_4AD130(int a1)
{
  return a1 + 12;
}

/* ---- stl_map_construct_alloc_m  0x004AD140 ---- HIGH */
int __cdecl stl_map_construct_alloc_m(_BYTE *a1, int a2, int a3)
{
  _DWORD *v3;

  *(_BYTE *)a2 = *a1;
  v3 = stl_map_buynode_str_m(a2);
  *(_DWORD *)(a2 + 4) = v3;
  *((_BYTE *)v3 + 101) = 1;
  *(_DWORD *)(*(_DWORD *)(a2 + 4) + 4) = *(_DWORD *)(a2 + 4);
  **(_DWORD **)(a2 + 4) = *(_DWORD *)(a2 + 4);
  *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = *(_DWORD *)(a2 + 4);
  *(_DWORD *)(a2 + 8) = 0;
  return a2;
}

/* ---- stl_map_insert_m  AUTO-STUBBED ---- HIGH  NO DEFINITION EMITTED */
/* ---- stl_map_lower_bound_iter_m  0x004AD2D0 ---- */
int **__cdecl stl_map_lower_bound_iter_m(int a1, int a2, int **a3)
{
  *a3 = stl_map_lower_bound_m(a1, a2);
  return a3;
}

/* ---- stl_tree_erase_all_m  AUTO-STUBBED ---- MEDIUM  NO DEFINITION EMITTED */
/* ---- sub_4AD320  0x004AD320 ---- */
int __cdecl sub_4AD320(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

/* ---- sub_4AD330  0x004AD330 ---- MEDIUM */
int __cdecl sub_4AD330(int a1)
{
  return *(_DWORD *)(a1 + 4) + 8;
}

/* ---- sub_4AD340  0x004AD340 ---- MEDIUM */
int __cdecl sub_4AD340(int a1)
{
  return *(_DWORD *)(a1 + 4) + 4;
}

/* ---- stl_map_tidy_dup_m  0x004AD350 ---- MEDIUM */
int __cdecl stl_map_tidy_dup_m(int a1)
{
  int result;
  int *v3;

  stl_map_erase_range_m(a1, &v3, **(int ***)(a1 + 4), *(int **)(a1 + 4));
  j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- nullsub_69  0x004AD380 ---- HIGH */
void nullsub_69()
{
  ;
}

/* ---- stl_map_pair_construct_m  0x004AD390 ---- HIGH */
int __cdecl stl_map_pair_construct_m(int a1, int a2, void ****a3)
{
  *(_DWORD *)(a2 + 24) = 15;
  *(_DWORD *)(a2 + 20) = 0;
  *(_BYTE *)(a2 + 4) = 0;
  sub_497C10((void ****)a2, a3, 0, (void **)0xFFFFFFFF);
  CStringEdString_CopyConstruct(a1, a2 + 28);
  return a2;
}

/* ---- nullsub_70  0x004AD400 ---- HIGH */
void nullsub_70()
{
  ;
}

/* ---- sub_4AD410  0x004AD410 ---- MEDIUM */
int __cdecl sub_4AD410(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_4AD420  0x004AD420 ---- MEDIUM */
_DWORD *__cdecl sub_4AD420(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- stl_vector_capacity_m  0x004AD430 ---- HIGH */
int __cdecl stl_vector_capacity_m(_DWORD *this)
{
  int result;

  result = this[1];
  if ( result )
    return (this[3] - result) / 28;
  return result;
}

/* ---- stl_vector_end_m  0x004AD460 ---- HIGH */
_DWORD *__cdecl stl_vector_end_m(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 8);
  return result;
}

/* ---- stl_vector_buy_m  AUTO-STUBBED ---- */
int stl_vector_buy_m() { return 0; }
#if 0
char __cdecl stl_vector_buy_m(unsigned int a1, _DWORD *a2)
{
  unsigned int v3;
  char *v4;

  a2[1] = 0;
  a2[2] = 0;
  a2[3] = 0;
  if ( !a1 )
    return 0;
  if ( a1 > 0x9249249 )
    stl_vector_xlen_m();
  v3 = 28 * a1;
  v4 = (char *)operator new(28 * a1);
  a2[1] = v4;
  a2[2] = v4;
  a2[3] = &v4[v3];
  return 1;
}
#endif

/* ---- stl_vector_tidy_dup_m  0x004AD4C0 ---- MEDIUM */
void __cdecl stl_vector_tidy_dup_m(int *a1)
{
  int v1;

  v1 = a1[1];
  if ( v1 )
  {
    stl_vector_destroy_range_impl_m(v1, a1[2]);
    j__free((void *)a1[1]);
  }
  a1[1] = 0;
  a1[2] = 0;
  a1[3] = 0;
}

/* ---- stl_vector_insert_n_m  AUTO-STUBBED ---- */
void __stdcall stl_vector_insert_n_m(int a1, void ****a2, char *a3, unsigned int a4)
{ return 0; }
#if 0
void __stdcall stl_vector_insert_n_m(int a1, void ****a2, char *a3, unsigned int a4)
{
  unsigned int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  unsigned int v9;
  char *v10;
  void ***v11;
  char *v12;
  void ***v13;
  int v14;
  int v15;
  int v16;
  unsigned int v17;
  int v18;
  unsigned int v19;
  int v20;
  void **v21;
  void **v22;
  unsigned int v23;
  int v24;
  int v25;
  int v26;
  void **v27;
  int v28;
  void ***v29; // [esp-Ch] [ebp-4Ch] BYREF
  int v30; // [esp-8h] [ebp-48h] BYREF
  void ***v31; // [esp-4h] [ebp-44h] BYREF
  int v32; // [esp+0h] [ebp-40h] BYREF
  void **v33; // [esp+Ch] [ebp-34h] BYREF
  void *Block;
  int v35;
  unsigned int v36;
  unsigned int v37;
  unsigned int v38;
  int *v39;
  int v40;
  unsigned int retaddr;

  v4 = (unsigned int)a3;
  v39 = &v32;
  v38 = retaddr ^ _security_cookie;
  v36 = 15;
  v35 = 0;
  LOBYTE(Block) = 0;
  sub_497C10((void ****)&v33, (void ****)a4, 0, (void **)0xFFFFFFFF);
  v40 = 0;
  v5 = *(_DWORD *)(a1 + 4);
  if ( v5 )
    a4 = (*(_DWORD *)(a1 + 12) - v5) / 28;
  else
    a4 = 0;
  if ( v4 )
  {
    if ( v5 )
      v6 = (*(_DWORD *)(a1 + 8) - v5) / 28;
    else
      v6 = 0;
    if ( 153391689 - v6 < v4 )
    {
      v31 = (void ***)a1;
      stl_vector_xlen_m();
    }
    if ( v5 )
      v7 = (*(_DWORD *)(a1 + 8) - v5) / 28;
    else
      v7 = 0;
    if ( a4 >= v4 + v7 )
    {
      v21 = *(void ***)(a1 + 8);
      v22 = (void **)a2;
      a4 = (char *)v21 - (char *)a2;
      v37 = (unsigned int)v21;
      v31 = (void ***)v21;
      if ( ((char *)v21 - (char *)a2) / 28 >= v4 )
      {
        v25 = 28 * v4;
        a2 = (void ****)&v21[v25 / 0xFFFFFFFC];
        v26 = stl_vector_ucopy_wrap2_m(a1, (int)v21, &v21[v25 / 0xFFFFFFFC], v21);
        v27 = (void **)a2;
        *(_DWORD *)(a1 + 8) = v26;
        a2 = &v29;
        stl_vector_ucopy_backward_wrap_m((void ***)&a2, v22, v27, (void **)v37);
        a2 = &v31;
        v31 = (void ***)&v22[v25 / 4u];
      }
      else
      {
        a4 = 28 * v4;
        v37 = (unsigned int)&v30;
        stl_vector_ucopy_wrap2_m(a1, (int)&a2[7 * v4], (void **)a2, v21);
        v31 = (void ***)a2;
        v30 = a1;
        v29 = &v33;
        v28 = *(_DWORD *)(a1 + 8);
        LOBYTE(v40) = 3;
        stl_vector_ufill_n_impl_m(v28, &v33);
        v23 = a4;
        v24 = a4 + *(_DWORD *)(a1 + 8);
        *(_DWORD *)(a1 + 8) = v24;
        a2 = &v31;
        v31 = (void ***)(v24 - v23);
        v40 = 0;
      }
      a2 = (void ****)&v30;
      stl_vector_ufill_range_m(&v33, (void ***)v22, v31);
    }
    else
    {
      if ( 153391689 - (a4 >> 1) >= a4 )
        a4 += a4 >> 1;
      else
        a4 = 0;
      if ( v5 )
        v8 = (*(_DWORD *)(a1 + 8) - v5) / 28;
      else
        v8 = 0;
      if ( a4 < v4 + v8 )
        a4 = v4 + stl_vector_size_m((_DWORD *)a1);
      v9 = 28 * a4;
      v37 = 28 * a4;
      v10 = (char *)operator new(28 * a4);
      v11 = *(void ****)(a1 + 4);
      a4 = (unsigned int)v10;
      a3 = v10;
      LOBYTE(v40) = 1;
      v12 = (char *)stl_vector_ucopy_m(v11, (void ***)a2, (int)v10);
      v31 = (void ***)a2;
      v30 = a1;
      a3 = v12;
      stl_vector_ufill_n_impl_m((int)v12, &v33);
      v13 = *(void ****)(a1 + 8);
      v31 = (void ***)a2;
      v30 = a1;
      a3 += 28 * v4;
      stl_vector_ucopy_m((void ***)a2, v13, (int)a3);
      v14 = *(_DWORD *)(a1 + 4);
      if ( v14 )
        v15 = (*(_DWORD *)(a1 + 8) - v14) / 28;
      else
        v15 = 0;
      v16 = *(_DWORD *)(a1 + 4);
      v17 = v15 + v4;
      if ( v16 )
      {
        v18 = *(_DWORD *)(a1 + 8);
        v31 = (void ***)a2;
        v30 = a1;
        stl_vector_destroy_range_impl_m(v16, v18);
        j__free(*(void **)(a1 + 4));
        v9 = v37;
      }
      v19 = a4;
      v20 = a4 + 28 * v17;
      *(_DWORD *)(a1 + 12) = a4 + v9;
      *(_DWORD *)(a1 + 8) = v20;
      *(_DWORD *)(a1 + 4) = v19;
    }
  }
  if ( v36 >= 0x10 )
    j__free(Block);
}
#endif

/* ---- stl_vector_ufill_n_m  0x004AD8B0 ---- HIGH */
int __cdecl stl_vector_ufill_n_m(void ***a1, int a2, int a3)
{
  stl_vector_ufill_n_impl_m(a2, a1);
  return a2 + 28 * a3;
}

/* ---- nullsub_71  0x004AD8D0 ---- HIGH */
void __stdcall nullsub_71(int a1)
{
  ;
}

/* ---- sub_4AD8E0  0x004AD8E0 ---- MEDIUM */
_DWORD *__cdecl sub_4AD8E0(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- sub_4AD8F0  0x004AD8F0 ---- MEDIUM */
_DWORD *__cdecl sub_4AD8F0(_DWORD *result, _DWORD *a2)
{
  *result = *a2;
  return result;
}

/* ---- sub_4AD900  0x004AD900 ---- MEDIUM */
int __cdecl sub_4AD900(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- stl_vector_iter_add_m  0x004AD910 ---- HIGH */
_DWORD *__cdecl stl_vector_iter_add_m(_DWORD *result, _DWORD *a2, int a3)
{
  *result = *a2 + 28 * a3;
  return result;
}

/* ---- stl_vector_iter_diff_m  0x004AD920 ---- HIGH */
int __cdecl stl_vector_iter_diff_m(_DWORD *a1, _DWORD *a2)
{
  return (*a1 - *a2) / 28;
}

/* ---- stl_map44_node_isnil_m  0x004AD940 ---- HIGH */
int __cdecl stl_map44_node_isnil_m(int a1)
{
  return a1 + 41;
}

/* ---- sub_4AD950  0x004AD950 ---- MEDIUM */
int __cdecl sub_4AD950(int a1)
{
  return a1 + 12;
}

/* ---- nullsub_72  0x004AD960 ---- HIGH */
void nullsub_72()
{
  ;
}

/* ---- sub_4AD970  0x004AD970 ---- MEDIUM */
int __cdecl sub_4AD970(int a1)
{
  return a1 + 8;
}

/* ---- stl_map44_construct_alloc_m  0x004AD980 ---- HIGH */
int __cdecl stl_map44_construct_alloc_m(_BYTE *a1, int a2, int a3)
{
  _DWORD *v3;

  *(_BYTE *)a2 = *a1;
  v3 = stl_map44_buynode_m(a2);
  *(_DWORD *)(a2 + 4) = v3;
  *((_BYTE *)v3 + 41) = 1;
  *(_DWORD *)(*(_DWORD *)(a2 + 4) + 4) = *(_DWORD *)(a2 + 4);
  **(_DWORD **)(a2 + 4) = *(_DWORD *)(a2 + 4);
  *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = *(_DWORD *)(a2 + 4);
  *(_DWORD *)(a2 + 8) = 0;
  return a2;
}

/* ---- stl_map44_begin_m  0x004AD9B0 ---- MEDIUM */
_DWORD *__cdecl stl_map44_begin_m(_DWORD *result, int a2)
{
  *result = **(_DWORD **)(a2 + 4);
  return result;
}

/* ---- stl_map44_equal_range_m  0x004AD9C0 ---- HIGH */
int **__cdecl stl_map44_equal_range_m(int a1, _DWORD *a2, int **a3)
{
  int *v3;

  v3 = stl_map44_upper_bound_m(a1, a2);
  *a3 = stl_map44_lower_bound_m(a1, (int)a2);
  a3[1] = v3;
  return a3;
}

/* ---- stl_map44_insert_rebalance_m  AUTO-STUBBED ---- */
int stl_map44_insert_rebalance_m() { return 0; }
#if 0
void ***__cdecl stl_map44_insert_rebalance_m(void ***a1, int a2, void ***a3, char a4, void ***a5)
{
  void ***v6;
  int v7;
  void ****v8;
  int v9;
  void ***v10;
  void ***v11;
  void **v12;
  void ***v13;
  void **v14;
  void **v15;
  void ***result;
  void **v17; // [esp+4h] [ebp-50h] BYREF
  char v18;
  int v19;
  int v20;
  _DWORD pExceptionObject[13]; // [esp+20h] [ebp-34h] BYREF
  void ***v22;

  if ( *(_DWORD *)(a2 + 8) >= 0x9249248u )
  {
    v20 = 15;
    v19 = 0;
    v18 = 0;
    sub_498410((int)&v17, "map/set<T> too long", (void **)strlen("map/set<T> too long"));
    pExceptionObject[12] = 0;
    sub_492500((int)pExceptionObject, &v17);
    pExceptionObject[0] = &std::length_error::`vftable';
    CxxThrowException_x_x_(pExceptionObject, (_ThrowInfo *)&_TI3_AVlength_error_std__);
  }
  v6 = stl_map44_buynode_init_m(a2, *(void ***)(a2 + 4), (void **)a1, *(void ***)(a2 + 4), a5, 0);
  v7 = *(_DWORD *)(a2 + 4);
  v22 = v6;
  ++*(_DWORD *)(a2 + 8);
  if ( a1 == (void ***)v7 )
  {
    *(_DWORD *)(v7 + 4) = v6;
    **(_DWORD **)(a2 + 4) = v6;
    *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = v6;
  }
  else if ( a4 )
  {
    *a1 = (void **)v6;
    v8 = *(void *****)(a2 + 4);
    if ( a1 == *v8 )
      *v8 = v6;
  }
  else
  {
    a1[2] = (void **)v6;
    v9 = *(_DWORD *)(a2 + 4);
    if ( a1 == *(void ****)(v9 + 8) )
      *(_DWORD *)(v9 + 8) = v6;
  }
  v10 = v6 + 1;
  v11 = v6;
  if ( !*((_BYTE *)v6[1] + 40) )
  {
    do
    {
      v12 = *v10;
      v13 = (void ***)(*v10)[1];
      v14 = *v13;
      if ( *v10 == *v13 )
      {
        v15 = v13[2];
        if ( *((_BYTE *)v15 + 40) )
        {
          if ( v11 == v12[2] )
          {
            v11 = (void ***)*v10;
            stl_map44_lrotate_m(v12, a2);
          }
          *((_BYTE *)v11[1] + 40) = 1;
          *((_BYTE *)v11[1][1] + 40) = 0;
          stl_map44_rrotate_m(v11[1][1], a2);
        }
        else
        {
          *((_BYTE *)v12 + 40) = 1;
          *((_BYTE *)v15 + 40) = 1;
          *((_BYTE *)(*v10)[1] + 40) = 0;
          v11 = (void ***)(*v10)[1];
        }
      }
      else if ( *((_BYTE *)v14 + 40) )
      {
        if ( v11 == *v12 )
        {
          v11 = (void ***)*v10;
          stl_map44_rrotate_m(v12, a2);
        }
        *((_BYTE *)v11[1] + 40) = 1;
        *((_BYTE *)v11[1][1] + 40) = 0;
        stl_map44_lrotate_m(v11[1][1], a2);
      }
      else
      {
        *((_BYTE *)v12 + 40) = 1;
        *((_BYTE *)v14 + 40) = 1;
        *((_BYTE *)(*v10)[1] + 40) = 0;
        v11 = (void ***)(*v10)[1];
      }
      v10 = v11 + 1;
    }
    while ( !*((_BYTE *)v11[1] + 40) );
    v6 = v22;
  }
  *(_BYTE *)(*(_DWORD *)(*(_DWORD *)(a2 + 4) + 4) + 40) = 1;
  result = a3;
  *a3 = (void **)v6;
  return result;
}
#endif

/* ---- sub_4ADBB0  0x004ADBB0 ---- MEDIUM */
int __cdecl sub_4ADBB0(int a1)
{
  return *(_DWORD *)(a1 + 4) + 4;
}

/* ---- stl_map44_tidy_dup_m  0x004ADBC0 ---- MEDIUM */
int __cdecl stl_map44_tidy_dup_m(int a1)
{
  int result;
  int *v3;

  stl_map44_erase_range_m(a1, &v3, **(int ***)(a1 + 4), *(int **)(a1 + 4));
  j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- nullsub_73  0x004ADBF0 ---- HIGH */
void nullsub_73()
{
  ;
}

/* ---- sub_4ADC00  0x004ADC00 ---- MEDIUM */
BOOL __cdecl sub_4ADC00(_DWORD *a1, _DWORD *a2)
{
  return *a2 == *a1;
}

/* ---- sub_4ADC10  0x004ADC10 ---- MEDIUM */
int __cdecl sub_4ADC10(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_4ADC20  0x004ADC20 ---- MEDIUM */
_DWORD *__cdecl sub_4ADC20(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- stl_map44_iter_prev_wrap_m  0x004ADC30 ---- HIGH */
int **__cdecl stl_map44_iter_prev_wrap_m(int a1, int **a2)
{
  stl_map44_iter_prev_m(a1, a2);
  return a2;
}

/* ---- stl_pair_iter_bool_m  0x004ADC40 ---- HIGH */
int __cdecl stl_pair_iter_bool_m(int result, _DWORD *a2, _BYTE *a3)
{
  *(_DWORD *)result = *a2;
  *(_BYTE *)(result + 4) = *a3;
  return result;
}

/* ---- CStringEdString_CopyConstruct  0x004ADC50 ---- HIGH */
int __cdecl CStringEdString_CopyConstruct(int a1, int a2)
{
  *(_DWORD *)(a2 + 24) = 15;
  *(_DWORD *)(a2 + 20) = 0;
  *(_BYTE *)(a2 + 4) = 0;
  sub_497C10((void ****)a2, (void ****)a1, 0, (void **)0xFFFFFFFF);
  *(_DWORD *)(a2 + 52) = 15;
  *(_DWORD *)(a2 + 48) = 0;
  *(_BYTE *)(a2 + 32) = 0;
  sub_497C10((void ****)(a2 + 28), (void ****)(a1 + 28), 0, (void **)0xFFFFFFFF);
  *(_DWORD *)(a2 + 56) = *(_DWORD *)(a1 + 56);
  return a2;
}

/* ---- stl_string_append_m  AUTO-STUBBED ---- */
int stl_string_append_m() { return 0; }
#if 0
std::_String_base *__cdecl stl_string_append_m(unsigned int a1, std::_String_base *a2, char *a3)
{
  unsigned int v4;
  char *v7;
  char *v8;
  std::_String_base *v9;
  unsigned int v11;
  unsigned int v12;
  bool v13; // zf
  char *v14;
  bool v15; // cf

  v4 = *((_DWORD *)a2 + 6);
  if ( v4 < 0x10 )
    v7 = (char *)a2 + 4;
  else
    v7 = (char *)*((_DWORD *)a2 + 1);
  if ( v7 <= a3 )
  {
    v8 = (char *)a2 + 4;
    v9 = v4 < 0x10 ? (std::_String_base *)((char *)a2 + 4) : *(std::_String_base **)v8;
    if ( a3 < (char *)v9 + *((_DWORD *)a2 + 5) )
    {
      if ( v4 >= 0x10 )
        v8 = *(char **)v8;
      return stl_string_replace_m(a1, a2, a2, a3 - v8);
    }
  }
  if ( -1 - *((_DWORD *)a2 + 5) <= a1 )
    std::_String_base::_Xlen_void_(a2);
  if ( !a1 )
    return a2;
  v11 = a1 + *((_DWORD *)a2 + 5);
  if ( v11 == -1 )
    std::_String_base::_Xlen_void_(a2);
  v12 = *((_DWORD *)a2 + 6);
  if ( v12 < v11 )
  {
    sub_498F10(a2, v11, *((_DWORD *)a2 + 5));
    v13 = v11 == 0;
    goto LABEL_19;
  }
  v13 = v11 == 0;
  if ( v11 )
  {
LABEL_19:
    if ( !v13 )
    {
      if ( *((_DWORD *)a2 + 6) < 0x10u )
        v14 = (char *)a2 + 4;
      else
        v14 = (char *)*((_DWORD *)a2 + 1);
      qmemcpy(&v14[*((_DWORD *)a2 + 5)], a3, a1);
      v15 = *((_DWORD *)a2 + 6) < 0x10u;
      *((_DWORD *)a2 + 5) = v11;
      if ( !v15 )
      {
        *(_BYTE *)(*((_DWORD *)a2 + 1) + v11) = 0;
        return a2;
      }
      *((_BYTE *)a2 + v11 + 4) = 0;
    }
    return a2;
  }
  *((_DWORD *)a2 + 5) = 0;
  if ( v12 < 0x10 )
    *((_BYTE *)a2 + 4) = 0;
  else
    **((_BYTE **)a2 + 1) = 0;
  return a2;
}
#endif

/* ---- stl_map_node_isnil_m  0x004ADDC0 ---- HIGH */
int __cdecl stl_map_node_isnil_m(int a1)
{
  return a1 + 101;
}

/* ---- nullsub_74  0x004ADDD0 ---- HIGH */
void nullsub_74()
{
  ;
}

/* ---- sub_4ADDE0  0x004ADDE0 ---- MEDIUM */
int __cdecl sub_4ADDE0(int a1)
{
  return a1 + 4;
}

/* ---- sub_4ADDF0  0x004ADDF0 ---- MEDIUM */
int __cdecl sub_4ADDF0(int a1)
{
  return a1 + 8;
}

/* ---- stl_map_begin_m  0x004ADE00 ---- MEDIUM */
_DWORD *__cdecl stl_map_begin_m(_DWORD *result, int a2)
{
  *result = **(_DWORD **)(a2 + 4);
  return result;
}

/* ---- stl_map_erase_range_m  AUTO-STUBBED ---- */
int stl_map_erase_range_m() { return 0; }
#if 0
int **__cdecl stl_map_erase_range_m(int a1, int **a2, int *a3, int *a4)
{
  int v4;
  int *v5;
  int *v6;
  int **v7;
  _DWORD *v8;
  int *v10;
  void *v11;

  v4 = *(_DWORD *)(a1 + 4);
  v5 = a3;
  v6 = a4;
  v7 = a2;
  if ( a3 == *(int **)v4 && a4 == (int *)v4 )
  {
    stl_tree_erase_all_m(*(void ***)(v4 + 4));
    *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
    v8 = *(_DWORD **)(a1 + 4);
    *(_DWORD *)(a1 + 8) = 0;
    *v8 = v8;
    *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
    *v7 = **(int ***)(a1 + 4);
    return v7;
  }
  else
  {
    if ( a3 != a4 )
    {
      do
      {
        v10 = v5;
        stl_map_iter_next104_m(v4, &a3);
        stl_map_erase_node_m(v11, a1, (int **)&a2, v10);
        v5 = a3;
      }
      while ( a3 != v6 );
    }
    *v7 = v5;
    return v7;
  }
}
#endif

/* ---- stl_map_construct_dup_m  0x004ADE90 ---- MEDIUM */
int __cdecl stl_map_construct_dup_m(int a1)
{
  _DWORD *v1;
  int result;

  v1 = stl_map_buynode_str_m(a1);
  *(_DWORD *)(a1 + 4) = v1;
  *((_BYTE *)v1 + 101) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  **(_DWORD **)(a1 + 4) = *(_DWORD *)(a1 + 4);
  result = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(result + 8) = result;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- stl_map_insert_rebalance_m  AUTO-STUBBED ---- */
int stl_map_insert_rebalance_m() { return 0; }
#if 0
_DWORD *__cdecl stl_map_insert_rebalance_m(_DWORD *a1, int a2, _DWORD *a3, char a4, void **a5)
{
  _DWORD *v6;
  _DWORD *v7;
  _DWORD *v8;
  int v9;
  _DWORD *v10;
  _DWORD *v11;
  int v12;
  int *v13;
  int v14;
  int v15;
  _DWORD *result;
  void **v17; // [esp+4h] [ebp-50h] BYREF
  char v18;
  int v19;
  int v20;
  _DWORD pExceptionObject[13]; // [esp+20h] [ebp-34h] BYREF
  _DWORD *v22;

  if ( *(_DWORD *)(a2 + 8) >= 0x2E8BA2Du )
  {
    v20 = 15;
    v19 = 0;
    v18 = 0;
    sub_498410((int)&v17, "map/set<T> too long", (void **)strlen("map/set<T> too long"));
    pExceptionObject[12] = 0;
    sub_492500((int)pExceptionObject, &v17);
    pExceptionObject[0] = &std::length_error::`vftable';
    CxxThrowException_x_x_(pExceptionObject, (_ThrowInfo *)&_TI3_AVlength_error_std__);
  }
  v6 = stl_map_buynode_init_m(a2, *(_DWORD *)(a2 + 4), (int)a1, *(_DWORD *)(a2 + 4), a5, 0);
  v7 = *(_DWORD **)(a2 + 4);
  v22 = v6;
  ++*(_DWORD *)(a2 + 8);
  if ( a1 == v7 )
  {
    v7[1] = v6;
    **(_DWORD **)(a2 + 4) = v6;
    *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = v6;
  }
  else if ( a4 )
  {
    *a1 = v6;
    v8 = *(_DWORD **)(a2 + 4);
    if ( a1 == (_DWORD *)*v8 )
      *v8 = v6;
  }
  else
  {
    a1[2] = v6;
    v9 = *(_DWORD *)(a2 + 4);
    if ( a1 == *(_DWORD **)(v9 + 8) )
      *(_DWORD *)(v9 + 8) = v6;
  }
  v10 = v6 + 1;
  v11 = v6;
  if ( !*(_BYTE *)(v6[1] + 100) )
  {
    do
    {
      v12 = *v10;
      v13 = *(int **)(*v10 + 4);
      v14 = *v13;
      if ( *v10 == *v13 )
      {
        v15 = v13[2];
        if ( *(_BYTE *)(v15 + 100) )
        {
          if ( v11 == *(_DWORD **)(v12 + 8) )
          {
            v11 = (_DWORD *)*v10;
            stl_map_lrotate_m((_DWORD *)v12, a2);
          }
          *(_BYTE *)(v11[1] + 100) = 1;
          *(_BYTE *)(*(_DWORD *)(v11[1] + 4) + 100) = 0;
          stl_map_rrotate_m(*(_DWORD **)(v11[1] + 4), a2);
        }
        else
        {
          *(_BYTE *)(v12 + 100) = 1;
          *(_BYTE *)(v15 + 100) = 1;
          *(_BYTE *)(*(_DWORD *)(*v10 + 4) + 100) = 0;
          v11 = *(_DWORD **)(*v10 + 4);
        }
      }
      else if ( *(_BYTE *)(v14 + 100) )
      {
        if ( v11 == *(_DWORD **)v12 )
        {
          v11 = (_DWORD *)*v10;
          stl_map_rrotate_m((_DWORD *)v12, a2);
        }
        *(_BYTE *)(v11[1] + 100) = 1;
        *(_BYTE *)(*(_DWORD *)(v11[1] + 4) + 100) = 0;
        stl_map_lrotate_m(*(_DWORD **)(v11[1] + 4), a2);
      }
      else
      {
        *(_BYTE *)(v12 + 100) = 1;
        *(_BYTE *)(v14 + 100) = 1;
        *(_BYTE *)(*(_DWORD *)(*v10 + 4) + 100) = 0;
        v11 = *(_DWORD **)(*v10 + 4);
      }
      v10 = v11 + 1;
    }
    while ( !*(_BYTE *)(v11[1] + 100) );
    v6 = v22;
  }
  *(_BYTE *)(*(_DWORD *)(*(_DWORD *)(a2 + 4) + 4) + 100) = 1;
  result = a3;
  *a3 = v6;
  return result;
}
#endif

/* ---- stl_map_lower_bound_m  AUTO-STUBBED ---- HIGH */
int stl_map_lower_bound_m() { return 0; }
#if 0
int *__cdecl stl_map_lower_bound_m(int a1, int a2)
{
  int *result;
  int v3;
  unsigned int v4;
  unsigned int v5;
  const char **v6;
  const char *v7;
  unsigned int v8;
  int v9;
  const char *v10;
  int v11;
  bool v12; // sf
  int *v13;
  const char **i;
  unsigned int v15;

  result = *(int **)(a1 + 4);
  v3 = result[1];
  v13 = result;
  if ( !*(_BYTE *)(v3 + 101) )
  {
    v4 = *(_DWORD *)(a2 + 24);
    v5 = *(_DWORD *)(a2 + 20);
    v6 = (const char **)(a2 + 4);
    v15 = v4;
    for ( i = v6; ; v6 = i )
    {
      if ( v15 < 0x10 )
        v7 = (const char *)v6;
      else
        v7 = *v6;
      v8 = *(_DWORD *)(v3 + 32);
      if ( !v8 )
        goto LABEL_14;
      v9 = *(_DWORD *)(v3 + 32);
      if ( v8 >= v5 )
        v9 = v5;
      v10 = *(_DWORD *)(v3 + 36) < 0x10u ? (const char *)(v3 + 16) : *(const char **)(v3 + 16);
      v11 = memcmp(v10, v7, v9);
      v12 = v11 < 0;
      if ( !v11 )
      {
LABEL_14:
        if ( v8 < v5 )
          goto LABEL_17;
        v12 = 0;
      }
      if ( v12 )
      {
LABEL_17:
        v3 = *(_DWORD *)(v3 + 8);
        goto LABEL_19;
      }
      v13 = (int *)v3;
      v3 = *(_DWORD *)v3;
LABEL_19:
      if ( *(_BYTE *)(v3 + 101) )
        return v13;
    }
  }
  return result;
}
#endif

/* ---- sub_4AE130  0x004AE130 ---- HIGH */
_BYTE *__cdecl sub_4AE130(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- nullsub_75  0x004AE140 ---- HIGH */
void nullsub_75()
{
  ;
}

/* ---- nullsub_76  0x004AE150 ---- HIGH */
void __stdcall nullsub_76(int a1)
{
  ;
}

/* ---- sub_4AE160  0x004AE160 ---- HIGH */
void __cdecl sub_4AE160(void *a1, int a2, int a3)
{
  j__free(a1);
}

/* ---- stl_map_node_destroy2_m  0x004AE170 ---- HIGH */
void __cdecl stl_map_node_destroy2_m(int a1, int a2)
{
  stl_map_node_destroy_m(a1);
}

/* ---- nullsub_77  0x004AE180 ---- HIGH */
void __stdcall nullsub_77(int a1, int a2)
{
  ;
}

/* ---- nullsub_78  0x004AE190 ---- HIGH */
void __stdcall nullsub_78(int a1)
{
  ;
}

/* ---- stl_alloc_str_allocate_m  AUTO-STUBBED ---- */
int stl_alloc_str_allocate_m() { return 0; }
#if 0
void *__cdecl stl_alloc_str_allocate_m(int a1, int a2, int a3)
{
  return operator new(28 * a1);
}
#endif

/* ---- sub_4AE1B0  0x004AE1B0 ---- HIGH */
void __cdecl sub_4AE1B0(void *a1, int a2, int a3)
{
  j__free(a1);
}

/* ---- sub_4AE1C0  0x004AE1C0 ---- MEDIUM */
_DWORD *__cdecl sub_4AE1C0(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- stl_map_iter_prev_wrap_m  0x004AE1D0 ---- HIGH */
int **__cdecl stl_map_iter_prev_wrap_m(int a1, int **a2)
{
  stl_map_iter_prev_m(a1, a2);
  return a2;
}

/* ---- stl_alloc_str_max_size_m  0x004AE1E0 ---- HIGH */
int __stdcall stl_alloc_str_max_size_m(int a1)
{
  return 153391689;
}

/* ---- stl_vector_destroy_range_m  0x004AE1F0 ---- HIGH */
void __cdecl stl_vector_destroy_range_m(int a1, int a2)
{
  stl_vector_destroy_range_impl_m(a2, a1);
}

/* ---- stl_vector_xlen_m  AUTO-STUBBED ---- */
int stl_vector_xlen_m() { return 0; }
#if 0
void  stl_vector_xlen_m()
{
  void **v0; // [esp+0h] [ebp-50h] BYREF
  char v1;
  int v2;
  int v3;
  _DWORD pExceptionObject[13]; // [esp+1Ch] [ebp-34h] BYREF

  v3 = 15;
  v2 = 0;
  v1 = 0;
  sub_498410((int)&v0, "vector<T> too long", (void **)strlen("vector<T> too long"));
  pExceptionObject[12] = 0;
  sub_492500((int)pExceptionObject, &v0);
  pExceptionObject[0] = &std::length_error::`vftable';
  CxxThrowException_x_x_(pExceptionObject, (_ThrowInfo *)&_TI3_AVlength_error_std__);
}
#endif

/* ---- sub_4AE290  0x004AE290 ---- MEDIUM */
int __cdecl sub_4AE290(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- stl_vector_iter_sub_m  0x004AE2A0 ---- HIGH */
_DWORD *__cdecl stl_vector_iter_sub_m(_DWORD *result, _DWORD *a2, int a3)
{
  *result = *a2 - 28 * a3;
  return result;
}

/* ---- stl_map_node_mapped_m  0x004AE2B0 ---- HIGH */
int __cdecl stl_map_node_mapped_m(int a1)
{
  return a1 + 40;
}

/* ---- sub_4AE2C0  0x004AE2C0 ---- MEDIUM */
int __cdecl sub_4AE2C0(int a1)
{
  return a1 + 4;
}

/* ---- sub_4AE2D0  0x004AE2D0 ---- MEDIUM */
int __cdecl sub_4AE2D0(int a1)
{
  return a1 + 12;
}

/* ---- sub_4AE2E0  0x004AE2E0 ---- MEDIUM */
_DWORD *__cdecl sub_4AE2E0(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 4);
  return result;
}

/* ---- stl_alloc_str_max_size_dup_m  0x004AE2F0 ---- HIGH */
int __stdcall stl_alloc_str_max_size_dup_m(int a1)
{
  return 153391689;
}

/* ---- stl_map44_erase_range_m  AUTO-STUBBED ---- */
int stl_map44_erase_range_m() { return 0; }
#if 0
int **__cdecl stl_map44_erase_range_m(int a1, int **a2, int *a3, int *a4)
{
  int v4;
  int *v5;
  int *v6;
  int **v7;
  _DWORD *v8;
  int *v10;
  void *v11;

  v4 = *(_DWORD *)(a1 + 4);
  v5 = a3;
  v6 = a4;
  v7 = a2;
  if ( a3 == *(int **)v4 && a4 == (int *)v4 )
  {
    stl_map44_erase_all_m(*(void ***)(v4 + 4));
    *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
    v8 = *(_DWORD **)(a1 + 4);
    *(_DWORD *)(a1 + 8) = 0;
    *v8 = v8;
    *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
    *v7 = **(int ***)(a1 + 4);
    return v7;
  }
  else
  {
    if ( a3 != a4 )
    {
      do
      {
        v10 = v5;
        stl_map44_iter_next_m(v4, &a3);
        stl_map44_erase_node_m(v11, a1, (int **)&a2, v10);
        v5 = a3;
      }
      while ( a3 != v6 );
    }
    *v7 = v5;
    return v7;
  }
}
#endif

/* ---- stl_map44_lower_bound_iter_m  0x004AE380 ---- HIGH */
int **__cdecl stl_map44_lower_bound_iter_m(int a1, int a2, int **a3)
{
  *a3 = stl_map44_lower_bound_m(a1, a2);
  return a3;
}

/* ---- stl_map44_upper_bound_iter_m  0x004AE390 ---- HIGH */
int **__cdecl stl_map44_upper_bound_iter_m(_DWORD *a1, int **a2, int a3)
{
  *a2 = stl_map44_upper_bound_m(a3, a1);
  return a2;
}

/* ---- stl_map44_construct_dup_m  0x004AE3B0 ---- MEDIUM */
int __cdecl stl_map44_construct_dup_m(int a1)
{
  _DWORD *v1;
  int result;

  v1 = stl_map44_buynode_m(a1);
  *(_DWORD *)(a1 + 4) = v1;
  *((_BYTE *)v1 + 41) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  **(_DWORD **)(a1 + 4) = *(_DWORD *)(a1 + 4);
  result = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(result + 8) = result;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- sub_4AE3E0  0x004AE3E0 ---- MEDIUM */
int __cdecl sub_4AE3E0(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

/* ---- stl_map44_lrotate_m  AUTO-STUBBED ---- */
int stl_map44_lrotate_m() { return 0; }
#if 0
_DWORD *__cdecl stl_map44_lrotate_m(_DWORD *this, int a2)
{
  _DWORD *result;
  int v3;
  _DWORD *v4;

  result = (_DWORD *)this[2];
  this[2] = *result;
  if ( !*(_BYTE *)(*result + 41) )
    *(_DWORD *)(*result + 4) = this;
  result[1] = this[1];
  v3 = *(_DWORD *)(a2 + 4);
  if ( this == *(_DWORD **)(v3 + 4) )
  {
    *(_DWORD *)(v3 + 4) = result;
    *result = this;
    this[1] = result;
  }
  else
  {
    v4 = (_DWORD *)this[1];
    if ( this == (_DWORD *)*v4 )
      *v4 = result;
    else
      v4[2] = result;
    *result = this;
    this[1] = result;
  }
  return result;
}
#endif

/* ---- sub_4AE450  0x004AE450 ---- MEDIUM */
int __cdecl sub_4AE450(int a1)
{
  return *(_DWORD *)(a1 + 4) + 8;
}

/* ---- stl_map44_rrotate_m  0x004AE460 ---- HIGH */
int __cdecl stl_map44_rrotate_m(_DWORD *this, int a2)
{
  int result;
  int v3;
  int v4;
  _DWORD *v5;

  result = *this;
  *this = *(_DWORD *)(*this + 8);
  v3 = *(_DWORD *)(result + 8);
  if ( !*(_BYTE *)(v3 + 41) )
    *(_DWORD *)(v3 + 4) = this;
  *(_DWORD *)(result + 4) = this[1];
  v4 = *(_DWORD *)(a2 + 4);
  if ( this == *(_DWORD **)(v4 + 4) )
  {
    *(_DWORD *)(v4 + 4) = result;
    *(_DWORD *)(result + 8) = this;
    this[1] = result;
  }
  else
  {
    v5 = (_DWORD *)this[1];
    if ( this == (_DWORD *)v5[2] )
      v5[2] = result;
    else
      *v5 = result;
    *(_DWORD *)(result + 8) = this;
    this[1] = result;
  }
  return result;
}

/* ---- stl_map44_buynode_init_m  AUTO-STUBBED ---- */
void ****__stdcall stl_map44_buynode_init_m(int a1, void ***a2, void ***a3, void ***a4, void ****a5, char a6)
{ return 0; }
#if 0
void ****__stdcall stl_map44_buynode_init_m(int a1, void ***a2, void ***a3, void ***a4, void ****a5, char a6)
{
  void ****v6;
  void ****v7;
  _DWORD v9[9]; // [esp+0h] [ebp-24h] BYREF

  v9[5] = v9;
  v6 = (void ****)operator new(0x2Cu);
  v7 = v6;
  v9[4] = v6;
  v9[8] = 1;
  v9[3] = v6;
  if ( v6 )
  {
    *v6 = a2;
    v6[1] = a3;
    v6[2] = a4;
    v6[9] = (void ***)15;
    v6[8] = 0;
    *((_BYTE *)v6 + 16) = 0;
    sub_497C10(v6 + 3, a5, 0, (void **)0xFFFFFFFF);
    *((_BYTE *)v7 + 40) = a6;
    *((_BYTE *)v7 + 41) = 0;
  }
  return v7;
}
#endif

/* ---- sub_4AE570  0x004AE570 ---- HIGH */
_BYTE *__cdecl sub_4AE570(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- sub_4AE580  0x004AE580 ---- HIGH */
void __cdecl sub_4AE580(void *a1, int a2, int a3)
{
  j__free(a1);
}

/* ---- nullsub_79  0x004AE590 ---- HIGH */
void __stdcall nullsub_79(int a1, int a2)
{
  ;
}

/* ---- sub_4AE5A0  0x004AE5A0 ---- MEDIUM */
_DWORD *__cdecl sub_4AE5A0(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- stl_map44_iter_prev_m  AUTO-STUBBED ---- MEDIUM  NO DEFINITION EMITTED */
/* ---- stl_pair_iter_bool_dup_m  0x004AE610 ---- */
int __cdecl stl_pair_iter_bool_dup_m(int result, _DWORD *a2, _BYTE *a3)
{
  *(_DWORD *)result = *a2;
  *(_BYTE *)(result + 4) = *a3;
  return result;
}

/* ---- stl_pair_iter_iter_m  0x004AE620 ---- HIGH */
_DWORD *__cdecl stl_pair_iter_iter_m(_DWORD *result, _DWORD *a2, _DWORD *a3)
{
  *result = *a2;
  result[1] = *a3;
  return result;
}

/* ---- stl_string_replace_m  AUTO-STUBBED ---- */
int stl_string_replace_m() { return 0; }
#if 0
std::_String_base *__cdecl stl_string_replace_m(
        unsigned int a1,
        _DWORD *a2,
        std::_String_base *a3,
        unsigned int a4)
{
  unsigned int v5;
  unsigned int v6;
  unsigned int v7;
  bool v8; // zf
  _DWORD *v9;
  char *v11;
  char *v12;
  bool v13; // cf

  v5 = a1;
  if ( a2[5] < a4 )
    std::_String_base::_Xran_void_(a3);
  if ( a2[5] - a4 < a1 )
    v5 = a2[5] - a4;
  if ( -1 - *((_DWORD *)a3 + 5) <= v5 )
    std::_String_base::_Xlen_void_(a3);
  if ( !v5 )
    return a3;
  v6 = v5 + *((_DWORD *)a3 + 5);
  if ( v6 == -1 )
    std::_String_base::_Xlen_void_(a3);
  v7 = *((_DWORD *)a3 + 6);
  if ( v7 < v6 )
  {
    sub_498F10(a3, v6, *((_DWORD *)a3 + 5));
    v8 = v6 == 0;
    goto LABEL_12;
  }
  v8 = v6 == 0;
  if ( v6 )
  {
LABEL_12:
    if ( !v8 )
    {
      if ( a2[6] < 0x10u )
        v9 = a2 + 1;
      else
        v9 = (_DWORD *)a2[1];
      v11 = (char *)a3 + 4;
      if ( *((_DWORD *)a3 + 6) < 0x10u )
        v12 = (char *)a3 + 4;
      else
        v12 = *(char **)v11;
      qmemcpy(&v12[*((_DWORD *)a3 + 5)], (char *)v9 + a4, v5);
      v13 = *((_DWORD *)a3 + 6) < 0x10u;
      *((_DWORD *)a3 + 5) = v6;
      if ( !v13 )
        v11 = *(char **)v11;
      v11[v6] = 0;
    }
    return a3;
  }
  *((_DWORD *)a3 + 5) = 0;
  if ( v7 < 0x10 )
    *((_BYTE *)a3 + 4) = 0;
  else
    **((_BYTE **)a3 + 1) = 0;
  return a3;
}
#endif

/* ---- stl_map_node_color_m  0x004AE720 ---- HIGH */
int __cdecl stl_map_node_color_m(int a1)
{
  return a1 + 100;
}

/* ---- stl_alloc_pair_max_size_m  0x004AE730 ---- HIGH */
int __stdcall stl_alloc_pair_max_size_m(int a1)
{
  return 48806446;
}

/* ---- stl_map_erase_node_m  AUTO-STUBBED ---- */
int stl_map_erase_node_m() { return 0; }
#if 0
int **__cdecl stl_map_erase_node_m(void *this, int a2, int **a3, int *a4)
{
  int *v4;
  int *v5;
  int v6;
  int *v7;
  int *v8;
  int v9;
  int v10;
  int *v11;
  int v12;
  int **v13;
  char v14;
  _BYTE *v15;
  bool v16; // zf
  int v17;
  int **result;
  int *Block;
  void **v20; // [esp+Ch] [ebp-50h] BYREF
  char v21;
  int v22;
  int v23;
  _DWORD pExceptionObject[13]; // [esp+28h] [ebp-34h] BYREF

  v4 = a4;
  if ( *((_BYTE *)a4 + 101) )
  {
    v23 = 15;
    v22 = 0;
    v21 = 0;
    sub_498410((int)&v20, "invalid map/set<T> iterator", (void **)strlen("invalid map/set<T> iterator"));
    pExceptionObject[12] = 0;
    sub_492500((int)pExceptionObject, &v20);
    pExceptionObject[0] = &std::out_of_range::`vftable';
    CxxThrowException_x_x_(pExceptionObject, (_ThrowInfo *)&_TI3_AVout_of_range_std__);
  }
  v5 = a4;
  Block = a4;
  stl_map_iter_next104_m((int)this, &a4);
  v6 = *v4;
  v7 = v4 + 2;
  if ( !*(_BYTE *)(*v4 + 101) )
  {
    if ( *(_BYTE *)(*v7 + 101) )
      goto LABEL_7;
    v4 = a4;
    v7 = a4 + 2;
  }
  v6 = *v7;
LABEL_7:
  if ( v4 == v5 )
  {
    v8 = (int *)v5[1];
    if ( !*(_BYTE *)(v6 + 101) )
      *(_DWORD *)(v6 + 4) = v8;
    v9 = a2;
    v10 = *(_DWORD *)(a2 + 4);
    if ( *(int **)(v10 + 4) == v5 )
    {
      *(_DWORD *)(v10 + 4) = v6;
    }
    else if ( (int *)*v8 == v5 )
    {
      *v8 = v6;
    }
    else
    {
      v8[2] = v6;
    }
    if ( **(int ***)(a2 + 4) == v5 )
    {
      if ( *(_BYTE *)(v6 + 101) )
        v11 = v8;
      else
        v11 = stl_map_subtree_min_m((_DWORD *)v6);
      **(_DWORD **)(a2 + 4) = v11;
    }
    if ( *(int **)(*(_DWORD *)(a2 + 4) + 8) == v5 )
    {
      if ( *(_BYTE *)(v6 + 101) )
        *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = v8;
      else
        *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = stl_map_subtree_max_m(v6);
    }
  }
  else
  {
    *(_DWORD *)(*v5 + 4) = v4;
    *v4 = *v5;
    if ( v4 == (int *)v5[2] )
    {
      v8 = v4;
    }
    else
    {
      v8 = (int *)v4[1];
      if ( !*(_BYTE *)(v6 + 101) )
        *(_DWORD *)(v6 + 4) = v8;
      *v8 = v6;
      *v7 = v5[2];
      *(_DWORD *)(v5[2] + 4) = v4;
    }
    v12 = *(_DWORD *)(a2 + 4);
    if ( *(int **)(v12 + 4) == v5 )
    {
      *(_DWORD *)(v12 + 4) = v4;
    }
    else
    {
      v13 = (int **)v5[1];
      if ( *v13 == v5 )
        *v13 = v4;
      else
        v13[2] = v4;
    }
    v4[1] = v5[1];
    v14 = *((_BYTE *)v4 + 100);
    *((_BYTE *)v4 + 100) = *((_BYTE *)v5 + 100);
    v9 = a2;
    *((_BYTE *)v5 + 100) = v14;
  }
  if ( *((_BYTE *)Block + 100) == 1 )
  {
    if ( v6 != *(_DWORD *)(*(_DWORD *)(v9 + 4) + 4) )
    {
      do
      {
        if ( *(_BYTE *)(v6 + 100) != 1 )
          break;
        v15 = (_BYTE *)*v8;
        if ( v6 == *v8 )
        {
          v15 = (_BYTE *)v8[2];
          if ( !v15[100] )
          {
            v15[100] = 1;
            *((_BYTE *)v8 + 100) = 0;
            stl_map_lrotate_m(v8, v9);
            v15 = (_BYTE *)v8[2];
          }
          if ( v15[101] )
            goto LABEL_53;
          if ( *(_BYTE *)(*(_DWORD *)v15 + 100) != 1 || *(_BYTE *)(*((_DWORD *)v15 + 2) + 100) != 1 )
          {
            if ( *(_BYTE *)(*((_DWORD *)v15 + 2) + 100) == 1 )
            {
              *(_BYTE *)(*(_DWORD *)v15 + 100) = 1;
              v15[100] = 0;
              stl_map_rrotate_m(v15, v9);
              v15 = (_BYTE *)v8[2];
            }
            v15[100] = *((_BYTE *)v8 + 100);
            *((_BYTE *)v8 + 100) = 1;
            *(_BYTE *)(*((_DWORD *)v15 + 2) + 100) = 1;
            stl_map_lrotate_m(v8, v9);
            break;
          }
        }
        else
        {
          if ( !v15[100] )
          {
            v15[100] = 1;
            *((_BYTE *)v8 + 100) = 0;
            stl_map_rrotate_m(v8, v9);
            v15 = (_BYTE *)*v8;
          }
          if ( v15[101] )
            goto LABEL_53;
          if ( *(_BYTE *)(*((_DWORD *)v15 + 2) + 100) != 1 || *(_BYTE *)(*(_DWORD *)v15 + 100) != 1 )
          {
            if ( *(_BYTE *)(*(_DWORD *)v15 + 100) == 1 )
            {
              *(_BYTE *)(*((_DWORD *)v15 + 2) + 100) = 1;
              v15[100] = 0;
              stl_map_lrotate_m(v15, v9);
              v15 = (_BYTE *)*v8;
            }
            v15[100] = *((_BYTE *)v8 + 100);
            *((_BYTE *)v8 + 100) = 1;
            *(_BYTE *)(*(_DWORD *)v15 + 100) = 1;
            stl_map_rrotate_m(v8, v9);
            break;
          }
        }
        v15[100] = 0;
LABEL_53:
        v6 = (int)v8;
        v16 = v8 == *(int **)(*(_DWORD *)(v9 + 4) + 4);
        v8 = (int *)v8[1];
      }
      while ( !v16 );
    }
    *(_BYTE *)(v6 + 100) = 1;
  }
  stl_map_pair_destroy_m((int)(Block + 3));
  j__free(Block);
  v17 = *(_DWORD *)(v9 + 8);
  if ( v17 )
    *(_DWORD *)(v9 + 8) = v17 - 1;
  result = a3;
  *a3 = a4;
  return result;
}
#endif

/* ---- stl_map_lrotate_m  AUTO-STUBBED ---- */
int stl_map_lrotate_m() { return 0; }
#if 0
_DWORD *__cdecl stl_map_lrotate_m(_DWORD *this, int a2)
{
  _DWORD *result;
  int v3;
  _DWORD *v4;

  result = (_DWORD *)this[2];
  this[2] = *result;
  if ( !*(_BYTE *)(*result + 101) )
    *(_DWORD *)(*result + 4) = this;
  result[1] = this[1];
  v3 = *(_DWORD *)(a2 + 4);
  if ( this == *(_DWORD **)(v3 + 4) )
  {
    *(_DWORD *)(v3 + 4) = result;
    *result = this;
    this[1] = result;
  }
  else
  {
    v4 = (_DWORD *)this[1];
    if ( this == (_DWORD *)*v4 )
      *v4 = result;
    else
      v4[2] = result;
    *result = this;
    this[1] = result;
  }
  return result;
}
#endif

/* ---- sub_4AEA80  0x004AEA80 ---- MEDIUM */
int __cdecl sub_4AEA80(int a1)
{
  return *(_DWORD *)(a1 + 4) + 4;
}

/* ---- stl_map_rrotate_m  0x004AEA90 ---- HIGH */
int __cdecl stl_map_rrotate_m(_DWORD *this, int a2)
{
  int result;
  int v3;
  int v4;
  _DWORD *v5;

  result = *this;
  *this = *(_DWORD *)(*this + 8);
  v3 = *(_DWORD *)(result + 8);
  if ( !*(_BYTE *)(v3 + 101) )
    *(_DWORD *)(v3 + 4) = this;
  *(_DWORD *)(result + 4) = this[1];
  v4 = *(_DWORD *)(a2 + 4);
  if ( this == *(_DWORD **)(v4 + 4) )
  {
    *(_DWORD *)(v4 + 4) = result;
    *(_DWORD *)(result + 8) = this;
    this[1] = result;
  }
  else
  {
    v5 = (_DWORD *)this[1];
    if ( this == (_DWORD *)v5[2] )
      v5[2] = result;
    else
      *v5 = result;
    *(_DWORD *)(result + 8) = this;
    this[1] = result;
  }
  return result;
}

/* ---- stl_map_buynode_str_m  AUTO-STUBBED ---- HIGH  NO DEFINITION EMITTED */
/* ---- stl_map_buynode_init_m  AUTO-STUBBED ---- NO DEFINITION EMITTED */
_DWORD *__stdcall stl_map_buynode_init_m(int a1, int a2, int a3, int a4, void ***a5, char a6)
{ return 0; }
#if 0
_DWORD *__stdcall stl_map_buynode_init_m(int a1, int a2, int a3, int a4, void ***a5, char a6)
{
  _DWORD *v6;
  _DWORD *v7;
  _DWORD v9[9]; // [esp+0h] [ebp-24h] BYREF

  v9[5] = v9;
  v6 = operator new(0x68u);
  v7 = v6;
  v9[4] = v6;
  v9[8] = 1;
  v9[3] = v6;
  if ( v6 )
  {
    *v6 = a2;
    v6[1] = a3;
    v6[2] = a4;
    stl_map_pair_copy_construct_m(a5, (int)(v6 + 3));
    *((_BYTE *)v7 + 100) = a6;
    *((_BYTE *)v7 + 101) = 0;
  }
  return v7;
}
#endif

/* ---- sub_4AEBD0  0x004AEBD0 ---- HIGH */
_BYTE *__cdecl sub_4AEBD0(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- stl_alloc_str_max_size_dup2_m  0x004AEBE0 ---- HIGH */
int __stdcall stl_alloc_str_max_size_dup2_m(int a1)
{
  return 153391689;
}

/* ---- stl_map_iter_prev_m  AUTO-STUBBED ---- HIGH  NO DEFINITION EMITTED */
/* ---- stl_map_iter_postinc_m  0x004AEC50 ---- */
int **__cdecl stl_map_iter_postinc_m(int **a1, int a2, int **a3, int a4)
{
  int *v4;

  v4 = *a1;
  stl_map_iter_next104_m(a2, a1);
  *a3 = v4;
  return a3;
}

/* ---- stl_map44_erase_node_m  AUTO-STUBBED ---- */
int stl_map44_erase_node_m() { return 0; }
#if 0
int **__cdecl stl_map44_erase_node_m(void *this, int a2, int **a3, int *a4)
{
  int *v4;
  int *v5;
  int v6;
  int *v7;
  int *v8;
  int v9;
  int v10;
  int *v11;
  int v12;
  int **v13;
  char v14;
  int *v15;
  _BYTE *v16;
  bool v17; // zf
  int v18;
  int **result;
  int *Block;
  void **v21; // [esp+Ch] [ebp-50h] BYREF
  char v22;
  int v23;
  int v24;
  _DWORD pExceptionObject[13]; // [esp+28h] [ebp-34h] BYREF

  v4 = a4;
  if ( *((_BYTE *)a4 + 41) )
  {
    v24 = 15;
    v23 = 0;
    v22 = 0;
    sub_498410((int)&v21, "invalid map/set<T> iterator", (void **)strlen("invalid map/set<T> iterator"));
    pExceptionObject[12] = 0;
    sub_492500((int)pExceptionObject, &v21);
    pExceptionObject[0] = &std::out_of_range::`vftable';
    CxxThrowException_x_x_(pExceptionObject, (_ThrowInfo *)&_TI3_AVout_of_range_std__);
  }
  v5 = a4;
  Block = a4;
  stl_map44_iter_next_m((int)this, &a4);
  v6 = *v4;
  v7 = v4 + 2;
  if ( !*(_BYTE *)(*v4 + 41) )
  {
    if ( *(_BYTE *)(*v7 + 41) )
      goto LABEL_7;
    v4 = a4;
    v7 = a4 + 2;
  }
  v6 = *v7;
LABEL_7:
  if ( v4 == v5 )
  {
    v8 = (int *)v5[1];
    if ( !*(_BYTE *)(v6 + 41) )
      *(_DWORD *)(v6 + 4) = v8;
    v9 = a2;
    v10 = *(_DWORD *)(a2 + 4);
    if ( *(int **)(v10 + 4) == v5 )
    {
      *(_DWORD *)(v10 + 4) = v6;
    }
    else if ( (int *)*v8 == v5 )
    {
      *v8 = v6;
    }
    else
    {
      v8[2] = v6;
    }
    if ( **(int ***)(a2 + 4) == v5 )
    {
      if ( *(_BYTE *)(v6 + 41) )
        v11 = v8;
      else
        v11 = stl_map44_subtree_min_m((_DWORD *)v6);
      **(_DWORD **)(a2 + 4) = v11;
    }
    if ( *(int **)(*(_DWORD *)(a2 + 4) + 8) == v5 )
    {
      if ( *(_BYTE *)(v6 + 41) )
        *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = v8;
      else
        *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = stl_map44_subtree_max_m(v6);
    }
  }
  else
  {
    *(_DWORD *)(*v5 + 4) = v4;
    *v4 = *v5;
    if ( v4 == (int *)v5[2] )
    {
      v8 = v4;
    }
    else
    {
      v8 = (int *)v4[1];
      if ( !*(_BYTE *)(v6 + 41) )
        *(_DWORD *)(v6 + 4) = v8;
      *v8 = v6;
      *v7 = v5[2];
      *(_DWORD *)(v5[2] + 4) = v4;
    }
    v12 = *(_DWORD *)(a2 + 4);
    if ( *(int **)(v12 + 4) == v5 )
    {
      *(_DWORD *)(v12 + 4) = v4;
    }
    else
    {
      v13 = (int **)v5[1];
      if ( *v13 == v5 )
        *v13 = v4;
      else
        v13[2] = v4;
    }
    v4[1] = v5[1];
    v14 = *((_BYTE *)v4 + 40);
    *((_BYTE *)v4 + 40) = *((_BYTE *)v5 + 40);
    v9 = a2;
    *((_BYTE *)v5 + 40) = v14;
  }
  v15 = Block;
  if ( *((_BYTE *)Block + 40) == 1 )
  {
    if ( v6 != *(_DWORD *)(*(_DWORD *)(v9 + 4) + 4) )
    {
      do
      {
        if ( *(_BYTE *)(v6 + 40) != 1 )
          break;
        v16 = (_BYTE *)*v8;
        if ( v6 == *v8 )
        {
          v16 = (_BYTE *)v8[2];
          if ( !v16[40] )
          {
            v16[40] = 1;
            *((_BYTE *)v8 + 40) = 0;
            stl_map44_lrotate_m(v8, v9);
            v16 = (_BYTE *)v8[2];
          }
          if ( v16[41] )
            goto LABEL_53;
          if ( *(_BYTE *)(*(_DWORD *)v16 + 40) != 1 || *(_BYTE *)(*((_DWORD *)v16 + 2) + 40) != 1 )
          {
            if ( *(_BYTE *)(*((_DWORD *)v16 + 2) + 40) == 1 )
            {
              *(_BYTE *)(*(_DWORD *)v16 + 40) = 1;
              v16[40] = 0;
              stl_map44_rrotate_m(v16, v9);
              v16 = (_BYTE *)v8[2];
            }
            v16[40] = *((_BYTE *)v8 + 40);
            *((_BYTE *)v8 + 40) = 1;
            *(_BYTE *)(*((_DWORD *)v16 + 2) + 40) = 1;
            stl_map44_lrotate_m(v8, v9);
            break;
          }
        }
        else
        {
          if ( !v16[40] )
          {
            v16[40] = 1;
            *((_BYTE *)v8 + 40) = 0;
            stl_map44_rrotate_m(v8, v9);
            v16 = (_BYTE *)*v8;
          }
          if ( v16[41] )
            goto LABEL_53;
          if ( *(_BYTE *)(*((_DWORD *)v16 + 2) + 40) != 1 || *(_BYTE *)(*(_DWORD *)v16 + 40) != 1 )
          {
            if ( *(_BYTE *)(*(_DWORD *)v16 + 40) == 1 )
            {
              *(_BYTE *)(*((_DWORD *)v16 + 2) + 40) = 1;
              v16[40] = 0;
              stl_map44_lrotate_m(v16, v9);
              v16 = (_BYTE *)*v8;
            }
            v16[40] = *((_BYTE *)v8 + 40);
            *((_BYTE *)v8 + 40) = 1;
            *(_BYTE *)(*(_DWORD *)v16 + 40) = 1;
            stl_map44_rrotate_m(v8, v9);
            break;
          }
        }
        v16[40] = 0;
LABEL_53:
        v6 = (int)v8;
        v17 = v8 == *(int **)(*(_DWORD *)(v9 + 4) + 4);
        v8 = (int *)v8[1];
      }
      while ( !v17 );
    }
    v15 = Block;
    *(_BYTE *)(v6 + 40) = 1;
  }
  if ( (unsigned int)v15[9] >= 0x10 )
    j__free((void *)v15[4]);
  Block[9] = 15;
  Block[8] = 0;
  *((_BYTE *)Block + 16) = 0;
  j__free(Block);
  v18 = *(_DWORD *)(v9 + 8);
  if ( v18 )
    *(_DWORD *)(v9 + 8) = v18 - 1;
  result = a3;
  *a3 = a4;
  return result;
}
#endif

/* ---- stl_map44_clear_m  AUTO-STUBBED ---- */
int stl_map44_clear_m() { return 0; }
#if 0
_DWORD *__cdecl stl_map44_clear_m(int a1)
{
  _DWORD *result;

  stl_map44_erase_all_m(*(void ***)(*(_DWORD *)(a1 + 4) + 4));
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  result = *(_DWORD **)(a1 + 4);
  *(_DWORD *)(a1 + 8) = 0;
  *result = result;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
  return result;
}
#endif

/* ---- stl_map44_lower_bound_m  AUTO-STUBBED ---- */
int stl_map44_lower_bound_m() { return 0; }
#if 0
int *__cdecl stl_map44_lower_bound_m(int a1, int a2)
{
  int *result;
  int v3;
  unsigned int v4;
  unsigned int v5;
  const char **v6;
  const char *v7;
  unsigned int v8;
  int v9;
  const char *v10;
  int v11;
  bool v12; // sf
  int *v13;
  const char **i;
  unsigned int v15;

  result = *(int **)(a1 + 4);
  v3 = result[1];
  v13 = result;
  if ( !*(_BYTE *)(v3 + 41) )
  {
    v4 = *(_DWORD *)(a2 + 24);
    v5 = *(_DWORD *)(a2 + 20);
    v6 = (const char **)(a2 + 4);
    v15 = v4;
    for ( i = v6; ; v6 = i )
    {
      if ( v15 < 0x10 )
        v7 = (const char *)v6;
      else
        v7 = *v6;
      v8 = *(_DWORD *)(v3 + 32);
      if ( !v8 )
        goto LABEL_14;
      v9 = *(_DWORD *)(v3 + 32);
      if ( v8 >= v5 )
        v9 = v5;
      v10 = *(_DWORD *)(v3 + 36) < 0x10u ? (const char *)(v3 + 16) : *(const char **)(v3 + 16);
      v11 = memcmp(v10, v7, v9);
      v12 = v11 < 0;
      if ( !v11 )
      {
LABEL_14:
        if ( v8 < v5 )
          goto LABEL_17;
        v12 = 0;
      }
      if ( v12 )
      {
LABEL_17:
        v3 = *(_DWORD *)(v3 + 8);
        goto LABEL_19;
      }
      v13 = (int *)v3;
      v3 = *(_DWORD *)v3;
LABEL_19:
      if ( *(_BYTE *)(v3 + 41) )
        return v13;
    }
  }
  return result;
}
#endif

/* ---- stl_map44_subtree_max_m  0x004AF030 ---- HIGH */
int __cdecl stl_map44_subtree_max_m(int result)
{
  int i;

  for ( i = *(_DWORD *)(result + 8); !*(_BYTE *)(i + 41); i = *(_DWORD *)(i + 8) )
    result = i;
  return result;
}

/* ---- stl_map44_upper_bound_m  AUTO-STUBBED ---- */
int stl_map44_upper_bound_m() { return 0; }
#if 0
int *__cdecl stl_map44_upper_bound_m(int a1, _DWORD *a2)
{
  int *result;
  int v3;
  unsigned int v4;
  unsigned int v5;
  const char *v6;
  unsigned int v7;
  int v8;
  const char *v9;
  int v10;
  bool v11; // sf
  int *v12;

  result = *(int **)(a1 + 4);
  v3 = result[1];
  v12 = result;
  if ( !*(_BYTE *)(v3 + 41) )
  {
    v4 = a2[5];
    while ( 1 )
    {
      v5 = *(_DWORD *)(v3 + 32);
      if ( *(_DWORD *)(v3 + 36) < 0x10u )
        v6 = (const char *)(v3 + 16);
      else
        v6 = *(const char **)(v3 + 16);
      v7 = v4;
      if ( !v4 )
        goto LABEL_13;
      v8 = v4;
      if ( v4 >= v5 )
        v8 = *(_DWORD *)(v3 + 32);
      v9 = a2[6] < 0x10u ? (const char *)(a2 + 1) : (const char *)a2[1];
      v10 = memcmp(v9, v6, v8);
      v11 = v10 < 0;
      v4 = a2[5];
      if ( !v10 )
      {
LABEL_13:
        if ( v7 < v5 )
          goto LABEL_16;
        v11 = 0;
      }
      if ( !v11 )
      {
        v3 = *(_DWORD *)(v3 + 8);
        goto LABEL_18;
      }
LABEL_16:
      v12 = (int *)v3;
      v3 = *(_DWORD *)v3;
LABEL_18:
      if ( *(_BYTE *)(v3 + 41) )
        return v12;
    }
  }
  return result;
}
#endif

/* ---- stl_map44_buynode_m  AUTO-STUBBED ---- HIGH  NO DEFINITION EMITTED */
/* ---- sub_4AF140  0x004AF140 ---- */
_BYTE *__cdecl sub_4AF140(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- stl_alloc_node44_allocate_m  AUTO-STUBBED ---- */
int stl_alloc_node44_allocate_m() { return 0; }
#if 0
void *__cdecl stl_alloc_node44_allocate_m(int a1, int a2, int a3)
{
  return operator new(44 * a1);
}
#endif

/* ---- stl_map44_node_init_m  0x004AF160 ---- HIGH */
int __cdecl stl_map44_node_init_m(int a1, int a2, int a3, int a4, void ****a5, char a6)
{
  *(_DWORD *)a4 = a1;
  *(_DWORD *)(a4 + 4) = a3;
  *(_DWORD *)(a4 + 8) = a2;
  *(_DWORD *)(a4 + 36) = 15;
  *(_DWORD *)(a4 + 32) = 0;
  *(_BYTE *)(a4 + 16) = 0;
  sub_497C10((void ****)(a4 + 12), a5, 0, (void **)0xFFFFFFFF);
  *(_BYTE *)(a4 + 40) = a6;
  *(_BYTE *)(a4 + 41) = 0;
  return a4;
}

/* ---- sub_4AF1A0  0x004AF1A0 ---- MEDIUM */
BOOL __cdecl sub_4AF1A0(_DWORD *a1, _DWORD *a2)
{
  return *a1 != *a2;
}

/* ---- stl_map44_iter_postinc_m  0x004AF1B0 ---- HIGH */
int **__cdecl stl_map44_iter_postinc_m(int **a1, int a2, int **a3, int a4)
{
  int *v4;

  v4 = *a1;
  stl_map44_iter_next_m(a2, a1);
  *a3 = v4;
  return a3;
}

/* ---- stl_map_subtree_max_m  0x004AF1C0 ---- HIGH */
int __cdecl stl_map_subtree_max_m(int result)
{
  int i;

  for ( i = *(_DWORD *)(result + 8); !*(_BYTE *)(i + 101); i = *(_DWORD *)(i + 8) )
    result = i;
  return result;
}

/* ---- stl_map_subtree_min_m  AUTO-STUBBED ---- */
int stl_map_subtree_min_m() { return 0; }
#if 0
_DWORD *__cdecl stl_map_subtree_min_m(_DWORD *result)
{
  int *v1;

  v1 = (int *)*result;
  if ( !*(_BYTE *)(*result + 101) )
  {
    do
    {
      result = v1;
      v1 = (int *)*v1;
    }
    while ( !*((_BYTE *)v1 + 101) );
  }
  return result;
}
#endif

/* ---- sub_4AF200  0x004AF200 ---- HIGH */
_BYTE *__cdecl sub_4AF200(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- stl_alloc_pair_max_size_dup_m  0x004AF210 ---- HIGH */
int __stdcall stl_alloc_pair_max_size_dup_m(int a1)
{
  return 48806446;
}

/* ---- stl_alloc_node104_allocate_m  AUTO-STUBBED ---- */
int stl_alloc_node104_allocate_m() { return 0; }
#if 0
void *__cdecl stl_alloc_node104_allocate_m(int a1, int a2, int a3)
{
  return operator new(104 * a1);
}
#endif

/* ---- stl_map_node_init_m  0x004AF230 ---- HIGH */
int __cdecl stl_map_node_init_m(int a1, int a2, int a3, int a4, void ***a5, char a6)
{
  *(_DWORD *)a4 = a1;
  *(_DWORD *)(a4 + 4) = a3;
  *(_DWORD *)(a4 + 8) = a2;
  stl_map_pair_copy_construct_m(a5, a4 + 12);
  *(_BYTE *)(a4 + 100) = a6;
  *(_BYTE *)(a4 + 101) = 0;
  return a4;
}

/* ---- sub_4AF260  0x004AF260 ---- MEDIUM */
_DWORD *__cdecl sub_4AF260(_DWORD *result, _DWORD *a2, int a3)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- stl_map_iter_next_wrap_m  0x004AF270 ---- HIGH */
int **__cdecl stl_map_iter_next_wrap_m(int a1, int **a2)
{
  stl_map_iter_next104_m(a1, a2);
  return a2;
}

/* ---- stl_map44_erase_all_m  AUTO-STUBBED ---- */
char __stdcall stl_map44_erase_all_m(void **Block)
{ return 0; }
#if 0
char __stdcall stl_map44_erase_all_m(void **Block)
{
  void **v1;
  char result;
  void **i;

  v1 = Block;
  result = *((_BYTE *)Block + 41);
  for ( i = Block; !result; v1 = i )
  {
    stl_map44_erase_all_m((void **)i[2]);
    i = (void **)*i;
    stl_map44_node_destroy_m((int)v1);
    j__free(v1);
    result = *((_BYTE *)i + 41);
  }
  return result;
}
#endif

/* ---- stl_map44_subtree_min_m  AUTO-STUBBED ---- */
int stl_map44_subtree_min_m() { return 0; }
#if 0
_DWORD *__cdecl stl_map44_subtree_min_m(_DWORD *result)
{
  int *v1;

  v1 = (int *)*result;
  if ( !*(_BYTE *)(*result + 41) )
  {
    do
    {
      result = v1;
      v1 = (int *)*v1;
    }
    while ( !*((_BYTE *)v1 + 41) );
  }
  return result;
}
#endif

/* ---- sub_4AF2E0  0x004AF2E0 ---- MEDIUM */
int __cdecl sub_4AF2E0(int a1)
{
  return *(_DWORD *)(a1 + 4) + 4;
}

/* ---- sub_4AF2F0  0x004AF2F0 ---- HIGH */
_BYTE *__cdecl sub_4AF2F0(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- stl_map44_node_destroy2_m  0x004AF300 ---- HIGH */
int __cdecl stl_map44_node_destroy2_m(int a1, int a2)
{
  return stl_map44_node_destroy_m(a1);
}

/* ---- sub_4AF310  0x004AF310 ---- MEDIUM */
_DWORD *__cdecl sub_4AF310(_DWORD *result, _DWORD *a2, int a3)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- stl_map44_iter_next_wrap_m  0x004AF320 ---- HIGH */
int **__cdecl stl_map44_iter_next_wrap_m(int a1, int **a2)
{
  stl_map44_iter_next_m(a1, a2);
  return a2;
}

/* ---- stl_map_pair_copy_construct_m  0x004AF330 ---- HIGH */
int __cdecl stl_map_pair_copy_construct_m(void ****this, int a2)
{
  *(_DWORD *)(a2 + 24) = 15;
  *(_DWORD *)(a2 + 20) = 0;
  *(_BYTE *)(a2 + 4) = 0;
  sub_497C10((void ****)a2, this, 0, (void **)0xFFFFFFFF);
  CStringEdString_CopyConstruct((int)(this + 7), a2 + 28);
  return a2;
}

/* ---- sub_4AF3A0  0x004AF3A0 ---- HIGH */
_BYTE *__cdecl sub_4AF3A0(_BYTE *result, char a2)
{
  *result = a2;
  return result;
}

/* ---- stl_map_iter_next104_m  AUTO-STUBBED ---- HIGH  NO DEFINITION EMITTED */
/* ---- sub_4AF410  0x004AF410 ---- */
_BYTE *__cdecl sub_4AF410(_BYTE *result, char a2)
{
  *result = a2;
  return result;
}

/* ---- stl_map44_iter_next_m  AUTO-STUBBED ---- HIGH  NO DEFINITION EMITTED */
/* ---- stl_map44_distance_m  0x004AF480 ---- */
int *__cdecl stl_map44_distance_m(int *a1, int *a2, int *a3)
{
  int *result;
  int *i;
  int v5;

  result = a2;
  for ( i = a3; a2 != i; result = stl_map44_iter_next_m(v5, &a2) )
  {
    v5 = *a1 + 1;
    *a1 = v5;
  }
  return result;
}

/* ---- stl_vector_ucopy_wrap2_m  AUTO-STUBBED ---- */
int __fastcall stl_vector_ucopy_wrap2_m(int a1, int a2, void ***a3, void ***a4)
{ return 0; }
#if 0
int __fastcall stl_vector_ucopy_wrap2_m(int a1, int a2, void ***a3, void ***a4)
{
  return stl_vector_ucopy_m(a3, a4, a2);
}
#endif

/* ---- stl_vector_ufill_range_m  AUTO-STUBBED ---- */
int stl_vector_ufill_range_m() { return 0; }
#if 0
void ****__cdecl stl_vector_ufill_range_m(void ****a1, void ****a2, void ****a3)
{
  void ****i;
  void ****result;

  for ( i = a2; i != a3; i += 7 )
    result = sub_497C10(i, a1, 0, (void **)0xFFFFFFFF);
  return result;
}
#endif

/* ---- stl_vector_ucopy_backward_wrap_m  AUTO-STUBBED ---- */
int stl_vector_ucopy_backward_wrap_m() { return 0; }
#if 0
void ****__cdecl stl_vector_ucopy_backward_wrap_m(void ****a1, void ***a2, void ***a3, void ***a4)
{
  stl_vector_ucopy_backward_m(a1, a2, a3, a4);
  return a1;
}
#endif

/* ---- stl_vector_ufill_wrap_m  0x004AF540 ---- HIGH */
void ***__cdecl stl_vector_ufill_wrap_m(void ***a1, int a2)
{
  return stl_vector_ufill_n_impl_m(a2, a1);
}

/* ---- stl_map_node_destroy1_m  0x004AF560 ---- HIGH */
void __cdecl stl_map_node_destroy1_m(int a1)
{
  stl_map_node_destroy_m(a1);
}

/* ---- nullsub_80  0x004AF570 ---- HIGH */
void nullsub_80()
{
  ;
}

/* ---- stl_alloc_str_allocate2_m  AUTO-STUBBED ---- */
int stl_alloc_str_allocate2_m() { return 0; }
#if 0
void *__cdecl stl_alloc_str_allocate2_m(int a1)
{
  return operator new(28 * a1);
}
#endif

/* ---- stl_vector_destroy_range_dup_m  0x004AF590 ---- MEDIUM */
void __cdecl stl_vector_destroy_range_dup_m(int a1, int a2)
{
  stl_vector_destroy_range_impl_m(a2, a1);
}

/* ---- nullsub_81  0x004AF5B0 ---- HIGH */
void nullsub_81()
{
  ;
}

/* ---- nullsub_82  0x004AF5C0 ---- HIGH */
void __stdcall nullsub_82(int a1)
{
  ;
}

/* ---- nullsub_83  0x004AF5D0 ---- HIGH */
void __stdcall nullsub_83(int a1)
{
  ;
}

/* ---- stl_alloc_node44_allocate2_m  AUTO-STUBBED ---- */
int stl_alloc_node44_allocate2_m() { return 0; }
#if 0
void *__cdecl stl_alloc_node44_allocate2_m(int a1)
{
  return operator new(44 * a1);
}
#endif

/* ---- nullsub_84  0x004AF5F0 ---- HIGH */
void __stdcall nullsub_84(int a1)
{
  ;
}

/* ---- stl_alloc_node104_allocate2_m  AUTO-STUBBED ---- */
int stl_alloc_node104_allocate2_m() { return 0; }
#if 0
void *__cdecl stl_alloc_node104_allocate2_m(int a1)
{
  return operator new(104 * a1);
}
#endif

/* ---- sub_4AF610  0x004AF610 ---- MEDIUM */
_DWORD *__cdecl sub_4AF610(_DWORD *result, _DWORD *a2)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- nullsub_85  0x004AF620 ---- HIGH */
void __stdcall nullsub_85(int a1)
{
  ;
}

/* ---- stl_map44_node_destroy1_m  0x004AF630 ---- HIGH */
int __cdecl stl_map44_node_destroy1_m(int a1)
{
  return stl_map44_node_destroy_m(a1);
}

/* ---- sub_4AF640  0x004AF640 ---- MEDIUM */
_DWORD *__cdecl sub_4AF640(_DWORD *result, _DWORD *a2)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- stl_map_node_scalar_delete_m  0x004AF650 ---- HIGH */
void *__cdecl stl_map_node_scalar_delete_m(void *a1, char a2)
{
  stl_map_node_destroy_m((int)a1);
  if ( (a2 & 1) != 0 )
    j__free(a1);
  return a1;
}

/* ---- stl_map44_node_scalar_delete_m  0x004AF670 ---- HIGH */
void *__cdecl stl_map44_node_scalar_delete_m(void *a1, char a2)
{
  stl_map44_node_destroy_m((int)a1);
  if ( (a2 & 1) != 0 )
    j__free(a1);
  return a1;
}

/* ---- stl_map_node_destroy_m  AUTO-STUBBED ---- */
int stl_map_node_destroy_m() { return 0; }
#if 0
void __cdecl stl_map_node_destroy_m(int a1)
{
  stl_map_pair_destroy_m(a1 + 12);
}
#endif

/* ---- stl_map44_node_destroy_m  0x004AF6A0 ---- HIGH */
int __cdecl stl_map44_node_destroy_m(int a1)
{
  int result;

  if ( *(_DWORD *)(a1 + 36) >= 0x10u )
    j__free(*(void **)(a1 + 16));
  result = 0;
  *(_DWORD *)(a1 + 36) = 15;
  *(_DWORD *)(a1 + 32) = 0;
  *(_BYTE *)(a1 + 16) = 0;
  return result;
}

/* ---- stl_vector_iter_preinc_m  0x004AF6D0 ---- HIGH */
_DWORD *__cdecl stl_vector_iter_preinc_m(_DWORD *result)
{
  *result += 28;
  return result;
}

/* ---- sub_4AF6E0  0x004AF6E0 ---- MEDIUM */
BOOL __cdecl sub_4AF6E0(_DWORD *a1, _DWORD *a2)
{
  return *a1 != *a2;
}

/* ---- sub_4AF6F0  0x004AF6F0 ---- MEDIUM */
BOOL __cdecl sub_4AF6F0(_DWORD *a1, _DWORD *a2)
{
  return *a2 == *a1;
}

/* ---- sub_4AF700  0x004AF700 ---- HIGH */
_BYTE *__cdecl sub_4AF700(_BYTE *result, int a2)
{
  *result = HIBYTE(a2);
  return result;
}

/* ---- stl_map44_distance_dup_m  0x004AF710 ---- MEDIUM */
int *__cdecl stl_map44_distance_dup_m(int *a1, int *a2, int *a3)
{
  int *result;
  int *i;
  int v5;

  result = a2;
  for ( i = a3; a2 != i; result = stl_map44_iter_next_m(v5, &a2) )
  {
    v5 = *a1 + 1;
    *a1 = v5;
  }
  return result;
}

/* ---- stl_vector_ucopy_wrap_m  0x004AF740 ---- HIGH */
int __cdecl stl_vector_ucopy_wrap_m(int a1, void ***a2, void ***a3)
{
  return stl_vector_ucopy_m(a2, a3, a1);
}

/* ---- sub_4AF770  0x004AF770 ---- HIGH */
char __fastcall sub_4AF770(int a1)
{
  return HIBYTE(a1);
}

/* ---- stl_vector_ucopy_backward_m  AUTO-STUBBED ---- */
int stl_vector_ucopy_backward_m() { return 0; }
#if 0
void *****__cdecl stl_vector_ucopy_backward_m(void *****a1, void ****a2, void ****a3, void ****a4)
{
  void ****v4;
  void ****v5;

  v4 = a3;
  if ( a2 == a3 )
  {
    *a1 = a4;
    return a1;
  }
  else
  {
    v5 = a4;
    do
    {
      v4 -= 7;
      v5 -= 7;
      sub_497C10(v5, v4, 0, (void **)0xFFFFFFFF);
    }
    while ( v4 != a2 );
    *a1 = v5;
    return a1;
  }
}
#endif

/* ---- sub_4AF7D0  0x004AF7D0 ---- HIGH */
char __fastcall sub_4AF7D0(int a1)
{
  return HIBYTE(a1);
}

/* ---- stl_vector_ufill_n_impl_m  AUTO-STUBBED ---- */
int stl_vector_ufill_n_impl_m() { return 0; }
#if 0
void ****__cdecl stl_vector_ufill_n_impl_m(int a1, void ****a2)
{
  int v2;
  int v3;
  int v4;
  void ****result;
  _DWORD v6[6]; // [esp+0h] [ebp-24h] BYREF
  int v7;

  v3 = a1;
  v6[5] = v6;
  v4 = v2;
  v6[4] = a1;
  v7 = 0;
  while ( v4 )
  {
    v6[3] = v3;
    LOBYTE(v7) = 1;
    if ( v3 )
    {
      *(_DWORD *)(v3 + 24) = 15;
      *(_DWORD *)(v3 + 20) = 0;
      *(_BYTE *)(v3 + 4) = 0;
      result = sub_497C10((void ****)v3, a2, 0, (void **)0xFFFFFFFF);
    }
    --v4;
    v3 += 28;
    LOBYTE(v7) = 0;
  }
  return result;
}
#endif

/* ---- stl_vector_destroy_range_impl_m  AUTO-STUBBED ---- */
int stl_vector_destroy_range_impl_m() { return 0; }
#if 0
void __cdecl stl_vector_destroy_range_impl_m(int a1, int a2)
{
  int i;

  for ( i = a1; i != a2; i += 28 )
  {
    if ( *(_DWORD *)(i + 24) >= 0x10u )
      j__free(*(void **)(i + 4));
    *(_DWORD *)(i + 24) = 15;
    *(_DWORD *)(i + 20) = 0;
    *(_BYTE *)(i + 4) = 0;
  }
}
#endif

/* ---- stl_string_construct_wrap_m  0x004AF8C0 ---- MEDIUM */
void ***__cdecl stl_string_construct_wrap_m(void ***a1, int a2, int a3)
{
  return stl_string_construct_copy_m(a2, a1);
}

/* ---- stl_string_tidy_m  0x004AF8D0 ---- HIGH */
int __cdecl stl_string_tidy_m(int a1, int a2)
{
  int result;

  if ( *(_DWORD *)(a1 + 24) >= 0x10u )
    j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 24) = 15;
  *(_DWORD *)(a1 + 20) = 0;
  *(_BYTE *)(a1 + 4) = 0;
  return result;
}

/* ---- stl_vector_iter_predec_m  0x004AF900 ---- HIGH */
_DWORD *__cdecl stl_vector_iter_predec_m(_DWORD *result)
{
  *result -= 28;
  return result;
}

/* ---- stl_map44_iter_next_wrap_dup_m  0x004AF910 ---- MEDIUM */
int **__cdecl stl_map44_iter_next_wrap_dup_m(int a1, int **a2)
{
  stl_map44_iter_next_m(a1, a2);
  return a2;
}

/* ---- sub_4AF920  0x004AF920 ---- HIGH */
char __fastcall sub_4AF920(int a1)
{
  return HIBYTE(a1);
}

/* ---- stl_vector_ucopy_m  0x004AF930 ---- HIGH */
int __cdecl stl_vector_ucopy_m(void ****a1, void ****a2, int a3)
{
  int v3;
  _DWORD v6[5];
  int v7;

  v3 = a3;
  v6[4] = v6;
  v6[3] = a3;
  v7 = 0;
  while ( a1 != a2 )
  {
    LOBYTE(v7) = 1;
    if ( v3 )
    {
      *(_DWORD *)(v3 + 24) = 15;
      *(_DWORD *)(v3 + 20) = 0;
      *(_BYTE *)(v3 + 4) = 0;
      sub_497C10((void ****)v3, a1, 0, (void **)0xFFFFFFFF);
    }
    v3 += 28;
    LOBYTE(v7) = 0;
    a1 += 7;
  }
  return v3;
}

/* ---- stl_string_construct_copy_m  AUTO-STUBBED ---- */
int stl_string_construct_copy_m() { return 0; }
#if 0
void ****__cdecl stl_string_construct_copy_m(int a1, void ****a2)
{
  void ****result;

  result = 0;
  if ( a1 )
  {
    *(_DWORD *)(a1 + 20) = 0;
    *(_DWORD *)(a1 + 24) = 15;
    *(_BYTE *)(a1 + 4) = 0;
    return sub_497C10((void ****)a1, a2, 0, (void **)0xFFFFFFFF);
  }
  return result;
}
#endif

/* ---- stl_string_tidy_dup_m  0x004AFA30 ---- MEDIUM */
int __cdecl stl_string_tidy_dup_m(int a1)
{
  int result;

  if ( *(_DWORD *)(a1 + 24) >= 0x10u )
    j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 24) = 15;
  *(_DWORD *)(a1 + 20) = 0;
  *(_BYTE *)(a1 + 4) = 0;
  return result;
}

/* ---- stl_string_scalar_delete_m  0x004AFA60 ---- HIGH */
int __cdecl stl_string_scalar_delete_m(int a1, char a2)
{
  if ( *(_DWORD *)(a1 + 24) >= 0x10u )
    j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 20) = 0;
  *(_DWORD *)(a1 + 24) = 15;
  *(_BYTE *)(a1 + 4) = 0;
  if ( (a2 & 1) != 0 )
    j__free((void *)a1);
  return a1;
}

/* ---- stl_char_traits_assign_m  0x004AFAA0 ---- HIGH */
char *__cdecl stl_char_traits_assign_m(char a1, char *a2, unsigned int a3, int a4)
{
  char v5;
  unsigned int v6;
  int v7;

  LOBYTE(a4) = a1;
  BYTE1(a4) = a1;
  v5 = a3;
  v6 = a3 >> 2;
  v7 = a4 << 16;
  LOWORD(v7) = a4;
  memset32(a2, v7, v6);
  memset(&a2[4 * v6], a4, v5 & 3);
  return a2;
}

/* ---- SE_LoadFile_m  0x004AFAD0 ---- VERIFIED */
void *__cdecl SE_LoadFile_m(char *qpath, int *lenOut)
{
  int   len;
  void *buffer;

  if ( lenOut )
    *lenOut = 0;

  buffer = NULL;
  len = FS_ReadFile(qpath, &buffer);
  if ( len <= 0 )
    return NULL;

  if ( lenOut )
    *lenOut = len;
  return buffer;
}

/* ---- SE_FreeFile_m  0x004AFB00 ---- VERIFIED */
void __cdecl SE_FreeFile_m(void *buffer)
{
  if ( buffer )
    FS_FreeFile(buffer);
}

/* ---- SE_EnumerateStrFiles_m  AUTO-STUBBED ---- */
int SE_EnumerateStrFiles_m() { return 0; }
#if 0
// SE_EnumerateStrFiles_m -- MEDIUM 58. FS_ListFiles-style enumeration filtered on ".str"; recursive. Caller SE_GetNumLanguages__Fv is a surviving mangled symbol, which anchors the SE_ prefix.
void __cdecl SE_EnumerateStrFiles_m(char *extension, char *path, std::_String_base *a3)
{
  char **v3;
  int v4;
  char v5;
  char **v6;
  int v7;
  void **v8;
  int v9;
  void *v10;
  void **v11;
  void *v12;
  void *v13;
  _DWORD *v14;
  int numfiles; // [esp+10h] [ebp-50h] BYREF
  int v16; // [esp+14h] [ebp-4Ch] BYREF
  void *Block;
  char Buffer[64]; // [esp+1Ch] [ebp-44h] BYREF
  unsigned int v19;
  unsigned int retaddr;

  v19 = retaddr ^ _security_cookie;
  v3 = FS_ListFilteredFiles(path, "/", 0, &numfiles);
  v4 = 0;
  for ( Block = v3; v4 < numfiles; ++v4 )
  {
    v5 = *v3[v4];
    if ( v5 )
    {
      if ( v5 != 46 )
      {
        sprintf(Buffer, "%s/%s", path, v3[v4]);
        SE_EnumerateStrFiles_m(extension, Buffer, a3);
      }
    }
  }
  v6 = FS_ListFilteredFiles(path, extension, 0, &v16);
  v7 = v16;
  v8 = (void **)v6;
  v9 = 0;
  for ( numfiles = (int)v6; v9 < v7; ++dword_CA26C4 )
  {
    sprintf(Buffer, "%s/%s", path, (const char *)v8[v9]);
    stl_string_append_m(strlen(Buffer), a3, Buffer);
    stl_string_append_fill_m(1u, v7, a3, 59);
    v8 = (void **)numfiles;
    ++v9;
  }
  if ( v8 )
  {
    v10 = *v8;
    if ( *v8 )
    {
      v11 = v8;
      do
      {
        free(v10);
        v10 = v11[1];
        ++v11;
      }
      while ( v10 );
      v8 = (void **)numfiles;
    }
    free(v8);
  }
  v12 = Block;
  if ( Block )
  {
    v13 = *(void **)Block;
    if ( *(_DWORD *)Block )
    {
      v14 = Block;
      do
      {
        free(v13);
        v13 = (void *)v14[1];
        ++v14;
      }
      while ( v13 );
    }
    free(v12);
  }
}
#endif

/* ---- SE_CountStrFiles_m  AUTO-STUBBED ---- */
int SE_CountStrFiles_m() { return 0; }
#if 0
// SE_CountStrFiles_m -- MEDIUM 55. Resets the counter, clears the output std::string and calls SE_EnumerateStrFiles_m with ".str".
int __cdecl SE_CountStrFiles_m(std::_String_base *a1, char *a2)
{
  dword_CA26C4 = 0;
  sub_498410((int)a1, &empty_string, (void **)strlen(&empty_string));
  SE_EnumerateStrFiles_m(".str", a2, a1);
  return dword_CA26C4;
}
#endif

/* ---- stl_string_append_char_m  AUTO-STUBBED ---- */
int stl_string_append_char_m() { return 0; }
#if 0
std::_String_base *__cdecl stl_string_append_char_m(char a1, int a2, std::_String_base *a3)
{
  return stl_string_append_fill_m(1u, a2, a3, a1);
}
#endif

/* ---- stl_string_append_fill_m  AUTO-STUBBED ---- */
int stl_string_append_fill_m() { return 0; }
#if 0
std::_String_base *__cdecl stl_string_append_fill_m(
        unsigned int a1,
        int a2,
        std::_String_base *a3,
        char a4)
{
  unsigned int v5;
  unsigned int v6;
  bool v7; // zf
  char *v8;
  unsigned int v10;
  char *v11;
  char v12;
  int v13;
  bool v14; // cf

  if ( -1 - *((_DWORD *)a3 + 5) <= a1 )
    std::_String_base::_Xlen_void_(a3);
  if ( !a1 )
    return a3;
  v5 = a1 + *((_DWORD *)a3 + 5);
  if ( v5 == -1 )
    std::_String_base::_Xlen_void_(a3);
  v6 = *((_DWORD *)a3 + 6);
  if ( v6 < v5 )
  {
    sub_498F10(a3, v5, *((_DWORD *)a3 + 5));
    v7 = v5 == 0;
    goto LABEL_8;
  }
  v7 = v5 == 0;
  if ( v5 )
  {
LABEL_8:
    if ( !v7 )
    {
      if ( *((_DWORD *)a3 + 6) < 0x10u )
        v8 = (char *)a3 + 4;
      else
        v8 = (char *)*((_DWORD *)a3 + 1);
      LOBYTE(a2) = a4;
      BYTE1(a2) = a4;
      v10 = a1;
      v11 = &v8[*((_DWORD *)a3 + 5)];
      v12 = v10;
      v10 >>= 2;
      v13 = a2 << 16;
      LOWORD(v13) = a2;
      memset32(v11, v13, v10);
      memset(&v11[4 * v10], a4, v12 & 3);
      v14 = *((_DWORD *)a3 + 6) < 0x10u;
      *((_DWORD *)a3 + 5) = v5;
      if ( !v14 )
      {
        *(_BYTE *)(*((_DWORD *)a3 + 1) + v5) = 0;
        return a3;
      }
      *((_BYTE *)a3 + v5 + 4) = 0;
    }
    return a3;
  }
  *((_DWORD *)a3 + 5) = 0;
  if ( v6 < 0x10 )
    *((_BYTE *)a3 + 4) = 0;
  else
    **((_BYTE **)a3 + 1) = 0;
  return a3;
}
#endif

/* ---- MD5Transform  0x004AFDB0 ---- VERIFIED */
int __cdecl MD5Transform(_DWORD *a1, _DWORD *a2)
{
  int v2;
  int v3;
  unsigned int v4;
  int v5;
  unsigned int v6;
  int v7;
  unsigned int v8;
  int v9;
  unsigned int v10;
  int v11;
  unsigned int v12;
  int v13;
  unsigned int v14;
  int v15;
  int v16;
  int v17;
  unsigned int v18;
  int v19;
  unsigned int v20;
  int v21;
  unsigned int v22;
  unsigned int v23;
  int v24;
  unsigned int v25;
  int v26;
  unsigned int v27;
  int v28;
  unsigned int v29;
  int v30;
  unsigned int v31;
  int v32;
  unsigned int v33;
  int v34;
  unsigned int v35;
  int v36;
  int v37;
  unsigned int v38;
  int v39;
  unsigned int v40;
  int v41;
  unsigned int v42;
  int v43;
  unsigned int v44;
  int v45;
  unsigned int v46;
  int v47;
  unsigned int v48;
  int v49;
  unsigned int v50;
  int v51;
  unsigned int v52;
  int v53;
  unsigned int v54;
  int v55;
  unsigned int v56;
  int v57;
  unsigned int v58;
  int v59;
  unsigned int v60;
  int v61;
  unsigned int v62;
  int v63;
  unsigned int v64;
  int v65;
  unsigned int v66;
  int v67;
  unsigned int v68;
  int v69;
  unsigned int v70;
  int v71;
  unsigned int v72;
  int v73;
  unsigned int v74;
  int v75;
  unsigned int v76;
  int v77;
  unsigned int v78;
  int v79;
  unsigned int v80;
  int v81;
  unsigned int v82;
  unsigned int v83;
  unsigned int v84;
  unsigned int v85;
  unsigned int v86;
  unsigned int v87;
  unsigned int v88;
  unsigned int v89;
  unsigned int v90;
  unsigned int v91;
  unsigned int v92;
  unsigned int v93;
  unsigned int v94;
  unsigned int v95;
  unsigned int v96;
  unsigned int v97;
  int v98;
  unsigned int v99;
  int v100;
  int v101;
  unsigned int v102;
  int v103;
  unsigned int v104;
  int v105;
  unsigned int v106;
  int v107;
  int v108;
  unsigned int v109;
  int v110;
  unsigned int v111;
  int v112;
  unsigned int v113;
  int v114;
  unsigned int v115;
  int v116;
  unsigned int v117;
  int result;
  unsigned int v119;
  int v120;
  int v121;
  unsigned int v122;
  int v123;
  int v124;
  int v125;
  int v126;
  int v127;
  int v128;
  int v129;
  int v130;
  int v131;
  int v132;
  int v133;
  int v134;
  int v135;
  int v136;
  int v137;
  int v138;
  int v139;
  int v140;
  int v141;
  int v142;

  v2 = a2[2];
  v3 = a2[1];
  v4 = *a1 + (v3 & v2 | a2[3] & ~v3) + *a2 - 680876936;
  v5 = v3 + ((v4 << 7) | (v4 >> 25));
  v134 = a1[1];
  v6 = a2[3] + v134 + (v5 & v3 | v2 & ~v5) - 389564586;
  v140 = a1[2];
  v7 = v5 + ((v6 << 12) | (v6 >> 20));
  v8 = v140 + (v5 & v7 | v3 & ~v7) + v2 + 606105819;
  v132 = a1[3];
  v9 = v7 + ((v8 << 17) | (v8 >> 15));
  v125 = v9
       + (((v132 + (v9 & v7 | v5 & (unsigned int)~v9) + a2[1] - 1044525330) >> 10)
        | ((v132 + (v9 & v7 | v5 & ~v9) + a2[1] - 1044525330) << 22));
  v138 = a1[4];
  v130 = a1[5];
  v10 = v5 + v138 + (v125 & v9 | v7 & ~v125) - 176418897;
  v11 = v125 + ((v10 << 7) | (v10 >> 25));
  v12 = v7 + v130 + (v11 & v125 | v9 & ~v11) + 1200080426;
  v13 = v11 + ((v12 << 12) | (v12 >> 20));
  v136 = a1[6];
  v14 = v9 + v136 + (v11 & v13 | v125 & ~v13) - 1473231341;
  v128 = a1[7];
  v15 = v13 + ((v14 << 17) | (v14 >> 15));
  v126 = v15
       + (((v128 + (v15 & v13 | v11 & (unsigned int)~v15) + v125 - 45705983) >> 10)
        | ((v128 + (v15 & v13 | v11 & ~v15) + v125 - 45705983) << 22));
  v135 = a1[8];
  v16 = v135 + (v126 & v15 | v13 & ~v126);
  v17 = v126 + (((v11 + v16 + 1770035416) << 7) | ((unsigned int)(v11 + v16 + 1770035416) >> 25));
  v141 = a1[9];
  v18 = v13 + v141 + (v17 & v126 | v15 & ~v17) - 1958414417;
  v19 = v17 + ((v18 << 12) | (v18 >> 20));
  v133 = a1[10];
  v20 = v15 + v133 + (v17 & v19 | v126 & ~v19) - 42063;
  v139 = a1[11];
  v21 = v19 + ((v20 << 17) | (v20 >> 15));
  v22 = v139 + (v21 & v19 | v17 & ~v21) + v126 - 1990404162;
  v131 = a1[12];
  v127 = v21 + ((v22 >> 10) | (v22 << 22));
  v23 = v17 + v131 + (v127 & v21 | v19 & ~v127) + 1804603682;
  v24 = v127 + ((v23 << 7) | (v23 >> 25));
  v137 = a1[13];
  v25 = v19 + v137 + (v24 & v127 | v21 & ~v24) - 40341101;
  v26 = v24 + ((v25 << 12) | (v25 >> 20));
  v129 = a1[14];
  v27 = v21 + v129 + (v24 & v26 | v127 & ~v26) - 1502002290;
  v142 = a1[15];
  v28 = v26 + ((v27 << 17) | (v27 >> 15));
  v29 = v127 + v142 + (v28 & v26 | v24 & ~v28) + 1236535329;
  v30 = v28 + ((v29 >> 10) | (v29 << 22));
  v31 = v24 + v134 + (v30 & v26 | v28 & ~v26) - 165796510;
  v32 = v30 + ((32 * v31) | (v31 >> 27));
  v33 = v26 + v136 + (v32 & v28 | v30 & ~v28) - 1069501632;
  v34 = v32 + ((v33 << 9) | (v33 >> 23));
  v35 = v28 + v139 + (v30 & v34 | v32 & ~v30) + 643717713;
  v36 = v34 + ((v35 << 14) | (v35 >> 18));
  v37 = v36
      + (((v30 + *a1 + (v32 & v36 | v34 & (unsigned int)~v32) - 373897302) >> 12)
       | ((v30 + *a1 + (v32 & v36 | v34 & ~v32) - 373897302) << 20));
  v38 = v32 + v130 + (v37 & v34 | v36 & ~v34) - 701558691;
  v39 = v37 + ((32 * v38) | (v38 >> 27));
  v40 = v34 + v133 + (v39 & v36 | v37 & ~v36) + 38016083;
  v41 = v39 + ((v40 << 9) | (v40 >> 23));
  v42 = v36 + v142 + (v37 & v41 | v39 & ~v37) - 660478335;
  v43 = v41 + ((v42 << 14) | (v42 >> 18));
  v44 = v37 + v138 + (v39 & v43 | v41 & ~v39) - 405537848;
  v45 = v43 + ((v44 >> 12) | (v44 << 20));
  v46 = v39 + v141 + (v45 & v41 | v43 & ~v41) + 568446438;
  v47 = v45 + ((32 * v46) | (v46 >> 27));
  v48 = v41 + v129 + (v47 & v43 | v45 & ~v43) - 1019803690;
  v49 = v47 + ((v48 << 9) | (v48 >> 23));
  v50 = v43 + v132 + (v45 & v49 | v47 & ~v45) - 187363961;
  v51 = v49 + ((v50 << 14) | (v50 >> 18));
  v52 = v45 + v135 + (v47 & v51 | v49 & ~v47) + 1163531501;
  v53 = v51 + ((v52 >> 12) | (v52 << 20));
  v54 = v47 + v137 + (v53 & v49 | v51 & ~v49) - 1444681467;
  v55 = v53 + ((32 * v54) | (v54 >> 27));
  v56 = v49 + v140 + (v55 & v51 | v53 & ~v51) - 51403784;
  v57 = v55 + ((v56 << 9) | (v56 >> 23));
  v58 = v51 + v128 + (v53 & v57 | v55 & ~v53) + 1735328473;
  v59 = v57 + ((v58 << 14) | (v58 >> 18));
  v60 = v53 + v131 + (v55 & v59 | v57 & ~v55) - 1926607734;
  v61 = v59 + ((v60 >> 12) | (v60 << 20));
  v62 = v55 + v130 + (v61 ^ v59 ^ v57) - 378558;
  v63 = v61 + ((16 * v62) | (v62 >> 28));
  v64 = v57 + v135 + (v63 ^ v61 ^ v59) - 2022574463;
  v65 = v63 + ((v64 << 11) | (v64 >> 21));
  v66 = v59 + v139 + (v63 ^ v61 ^ v65) + 1839030562;
  v67 = v65 + ((v66 << 16) | HIWORD(v66));
  v68 = v61 + v129 + (v63 ^ v67 ^ v65) - 35309556;
  v69 = v67 + ((v68 >> 9) | (v68 << 23));
  v70 = v63 + v134 + (v69 ^ v67 ^ v65) - 1530992060;
  v71 = v69 + ((16 * v70) | (v70 >> 28));
  v72 = v65 + v138 + (v71 ^ v69 ^ v67) + 1272893353;
  v73 = v71 + ((v72 << 11) | (v72 >> 21));
  v74 = v67 + v128 + (v71 ^ v69 ^ v73) - 155497632;
  v75 = v73 + ((v74 << 16) | HIWORD(v74));
  v76 = v69 + v133 + (v71 ^ v75 ^ v73) - 1094730640;
  v77 = v75 + ((v76 >> 9) | (v76 << 23));
  v78 = v71 + v137 + (v77 ^ v75 ^ v73) + 681279174;
  v79 = v77 + ((16 * v78) | (v78 >> 28));
  v80 = v73 + *a1 + (v79 ^ v77 ^ v75) - 358537222;
  v81 = v79 + ((v80 << 11) | (v80 >> 21));
  v82 = v75 + v132 + (v79 ^ v77 ^ v81) - 722521979;
  v83 = v81 + ((v82 << 16) | HIWORD(v82));
  v84 = v77 + v136 + (v79 ^ v83 ^ v81) + 76029189;
  v85 = v83 + ((v84 >> 9) | (v84 << 23));
  v86 = v79 + v141 + (v85 ^ v83 ^ v81) - 640364487;
  v87 = v85 + ((16 * v86) | (v86 >> 28));
  v88 = v87
      + (((v81 + v131 + (v87 ^ v85 ^ v83) - 421815835) << 11) | ((v81 + v131 + (v87 ^ v85 ^ v83) - 421815835) >> 21));
  v89 = v83 + v142 + (v87 ^ v85 ^ v88) + 530742520;
  v90 = v88 + ((v89 << 16) | HIWORD(v89));
  v91 = v85 + v140 + (v87 ^ v90 ^ v88) - 995338651;
  v92 = v90 + ((v91 >> 9) | (v91 << 23));
  v93 = v87 + *a1 + (v90 ^ (v92 | ~v88)) - 198630844;
  v94 = v92 + ((v93 << 6) | (v93 >> 26));
  v95 = v88 + v128 + (v92 ^ (v94 | ~v90)) + 1126891415;
  v96 = v94 + ((v95 << 10) | (v95 >> 22));
  v97 = v90 + v129 + (v94 ^ (v96 | ~v92)) - 1416354905;
  v98 = v96 + ((v97 << 15) | (v97 >> 17));
  v99 = v92 + v130 + (v96 ^ (v98 | ~v94)) - 57434055;
  v100 = v98 + ((v99 >> 11) | (v99 << 21));
  v101 = v100
       + (((v94 + v131 + (v98 ^ (v100 | ~v96)) + 1700485571) << 6)
        | ((v94 + v131 + (v98 ^ (v100 | ~v96)) + 1700485571) >> 26));
  v102 = v96 + v132 + (v100 ^ (v101 | ~v98)) - 1894986606;
  v103 = v101 + ((v102 << 10) | (v102 >> 22));
  v104 = v98 + v133 + (v101 ^ (v103 | ~v100)) - 1051523;
  v105 = v103 + ((v104 << 15) | (v104 >> 17));
  v106 = v100 + v134 + (v103 ^ (v105 | ~v101)) - 2054922799;
  v107 = v105 + ((v106 >> 11) | (v106 << 21));
  v108 = v107
       + (((v101 + v135 + (v105 ^ (v107 | ~v103)) + 1873313359) << 6)
        | ((v101 + v135 + (v105 ^ (v107 | (unsigned int)~v103)) + 1873313359) >> 26));
  v109 = v103 + v142 + (v107 ^ (v108 | ~v105)) - 30611744;
  v110 = v108 + ((v109 << 10) | (v109 >> 22));
  v111 = v105 + v136 + (v108 ^ (v110 | ~v107)) - 1560198380;
  v112 = v110 + ((v111 << 15) | (v111 >> 17));
  v113 = v107 + v137 + (v110 ^ (v112 | ~v108)) + 1309151649;
  v114 = v112 + ((v113 >> 11) | (v113 << 21));
  v115 = v108 + v138 + (v112 ^ (v114 | ~v110)) - 145523070;
  v116 = v114 + ((v115 << 6) | (v115 >> 26));
  v117 = v110 + v139 + (v114 ^ (v116 | ~v112)) - 1120210379;
  result = v116 + ((v117 << 10) | (v117 >> 22));
  v119 = v112 + v140 + (v116 ^ (result | ~v114)) + 718787259;
  v120 = result + ((v119 << 15) | (v119 >> 17));
  v121 = v116 + *a2;
  v122 = v114 + v141 + (result ^ (v120 | ~v116)) - 343485551;
  *a2 = v121;
  v123 = v120 + a2[1] + ((v122 >> 11) | (v122 << 21));
  a2[2] = v120 + v2;
  v124 = a2[3];
  a2[1] = v123;
  a2[3] = result + v124;
  return result;
}

/* ---- MD5Update  0x004B0750 ---- VERIFIED */
unsigned int __cdecl MD5Update(unsigned int a1, unsigned int *a2, _BYTE *a3)
{
  unsigned int v4;
  unsigned int result;
  unsigned int v6;
  unsigned int v7;
  unsigned __int16 *v8;
  int v9;
  int v10;
  int v11;
  _DWORD v12[16];

  v4 = *a2 + 8 * a1;
  result = (*a2 >> 3) & 0x3F;
  if ( v4 < *a2 )
    ++a2[1];
  *a2 = v4;
  a2[1] += a1 >> 29;
  if ( a1 )
  {
    v6 = a1;
    do
    {
      *((_BYTE *)a2 + result++ + 24) = *a3++;
      if ( result == 64 )
      {
        v7 = 0;
        v8 = (unsigned __int16 *)a2 + 13;
        do
        {
          v12[v7] = *((unsigned __int8 *)v8 - 2) | ((*((unsigned __int8 *)v8 - 1) | (*v8 << 8)) << 8);
          v12[v7 + 1] = *((unsigned __int8 *)v8 + 2) | ((*((unsigned __int8 *)v8 + 3) | (v8[2] << 8)) << 8);
          v12[v7 + 2] = *((unsigned __int8 *)v8 + 6) | ((*((unsigned __int8 *)v8 + 7) | (v8[4] << 8)) << 8);
          v12[v7 + 3] = *((unsigned __int8 *)v8 + 10) | ((*((unsigned __int8 *)v8 + 11) | (v8[6] << 8)) << 8);
          v12[v7 + 4] = *((unsigned __int8 *)v8 + 14) | ((*((unsigned __int8 *)v8 + 15) | (v8[8] << 8)) << 8);
          v9 = *((unsigned __int8 *)v8 + 23);
          v12[v7 + 5] = *((unsigned __int8 *)v8 + 18) | ((*((unsigned __int8 *)v8 + 19) | (v8[10] << 8)) << 8);
          v10 = *((unsigned __int8 *)v8 + 22) | ((v9 | (v8[12] << 8)) << 8);
          v11 = *((unsigned __int8 *)v8 + 27);
          v12[v7 + 6] = v10;
          v12[v7 + 7] = *((unsigned __int8 *)v8 + 26) | ((v11 | (v8[14] << 8)) << 8);
          v7 += 8;
          v8 += 16;
        }
        while ( v7 < 0x10 );
        MD5Transform(v12, a2 + 2);
        result = 0;
      }
      --v6;
    }
    while ( v6 );
  }
  return result;
}

/* ---- MD5Final  0x004B08C0 ---- VERIFIED */
char *__cdecl MD5Final(unsigned int *a1)
{
  unsigned int   in[16];
  unsigned char *buf;
  unsigned int   index;
  int            padLen;
  int            i;

  in[14] = a1[0];
  in[15] = a1[1];

  index  = (a1[0] >> 3) & 0x3F;
  padLen = ( index < 56 ? 56 : 120 ) - index;
  MD5Update(padLen, a1, MD5_PADDING);

  buf = (unsigned char *)a1 + 24;
  for ( i = 0 ; i < 14 ; i++ )
    in[i] = (unsigned int)buf[4 * i]
          | ((unsigned int)buf[4 * i + 1] << 8)
          | ((unsigned int)buf[4 * i + 2] << 16)
          | ((unsigned int)buf[4 * i + 3] << 24);

  MD5Transform((_DWORD *)in, (_DWORD *)(a1 + 2));

  memcpy((char *)a1 + 88, (char *)a1 + 8, 16);

  return (char *)a1 + 25;
}
