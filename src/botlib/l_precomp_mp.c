/*
 * Retail range 0x0043D7B0-0x00442290, 80 functions.
 *
 * @fidelity: likely
 * @fidelity-default: unreviewed
 *
 * Hand-written parts: the SPLIT-RECORD sites (see the note below);
 * PC_ExpandBuiltinDefine 0x0043E4B0; PC_EvaluateTokens 0x0043FC10 and its two
 * call sites; and the two calls in PC_ReadToken to PC_ReadDirective 0x00440FE0
 * and PC_ReadDollarDirective 0x00441350, which live in botlib/l_precomp_pc.c
 * along with the two .data tables they walk.
 *
 * PC_ExpectTokenType (0x00441740) is a stub, unreachable in retail (no xrefs):
 * CoD MP ships no BotLibSetup and no bots.
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "l_script.h"

/*
 * ===========================================================================
 * THE SPLIT TOKEN RECORD -- read this before touching any local named
 * ArgList, Buffer or Source in this file.
 * ===========================================================================
 *
 * Retail builds a `token_t token;` on the stack in eleven functions here.  It
 * is 1072 bytes and it always sits at [ebp-434h], running down to the security
 * cookie at [ebp-4].  Callees read and write the full record: PC_ReadLine and
 * PC_ReadSourceToken fill 1072 bytes; PC_CopyToken memcpys 0x430 bytes out.
 *
 * Each such site declares the real `token_t tok;`, points the original char*
 * name at tok.string so the body text does not have to change, and roots each
 * loose scalar on its field (+1024 type, +1028 subtype, +1048 whitespace_p
 * and so on).  `tok.string` and `&tok` are the same address, so calls that
 * want the whole record are correct unchanged.  Sites are marked SPLIT-RECORD.
 *
 * Layout is in l_script.h.
 */

/* Functions this unit calls that live elsewhere.  Declared unprototyped and
 * int-returning: C89 makes that compatible with any definition.  Anything a
 * real header already declares is excluded -- StripDoubleQuotes
 * (__fastcall, string@<edx>), LoadScriptFile (one stack argument),
 * LoadScriptMemory (retail lifts length into EAX; source order is
 * ( ptr, length, name )) and PS_ReadToken (retail ( token@<eax>,
 * script@<ecx> ); source order is ( script, token )) all come from
 * l_script.h.  An unprototyped `extern int f();` binds a __fastcall body to
 * the wrong decorated name and switches argument checking off. */
/* Log_Write: 0x0043D650 is RTCW's Log_Write, not Log_WriteTimeStamped (which
 * emits a botlibglobals.time stamp and is absent from this binary);
 * PC_PrintDefineHashTable is RTCW l_precomp.c:522 verbatim and uses Log_Write
 * for all three prints. */
extern void QDECL Log_Write( char *fmt, ... );
extern int SEH_GetLocalizedString_m();
extern void Sys_OutOfMemoryPrep( void );

/* PC_CheckTokenString (0x00441A20) is defined later in this file but called
 * earlier, from PC_ReadDefineParms.  The spelling matches the definition. */
int __cdecl PC_CheckTokenString( _DWORD *source, const char *string );

/* MessageBoxA is implicitly declared in this unit (four sites, all on the
 * Sys_OutOfMemoryPrep -> exit(-1) path).  It resolves to the cdecl shim in
 * win32/win_import_aliases.c, which forwards to the real __stdcall import.
 * Do not declare the __stdcall prototype here; that would bind to
 * __imp__MessageBoxA@16 and bypass the shim. */

/* ---- SourceError  0x0043D7B0 ---- */
// SourceError | CONFIRMED | RTCW l_precomp.c SourceError.  DIVERGENCE: CoD prefixes the message with the "^1" red colour code; RTCW emits it plain via botimport.Print(PRT_ERROR,...)
void SourceError(int a1, char *Format, ...)
{
  char Buffer[1024]; // [esp+0h] [ebp-404h] BYREF
  unsigned int v3;
  unsigned int retaddr;
  va_list ArgList; // [esp+410h] [ebp+Ch] BYREF

  va_start(ArgList, Format);
  v3 = retaddr ^ _security_cookie;
  vsprintf(Buffer, Format, ArgList);
  Com_Printf(
    "^1Error: file %s, line %d: %s\n",
    *(const char **)(a1 + 524),
    *(_DWORD *)(*(_DWORD *)(a1 + 524) + 288),
    Buffer);
}

/* ---- SourceWarning  0x0043D820 ---- */
// SourceWarning | HIGH | RTCW SourceWarning (same format, no colour prefix).  Distinguished from SourceError purely by the absence of the ^1 colour prefix.
void SourceWarning(int a1, char *Format, ...)
{
  char Buffer[1024]; // [esp+0h] [ebp-404h] BYREF
  unsigned int v3;
  unsigned int retaddr;
  va_list ArgList; // [esp+410h] [ebp+Ch] BYREF

  va_start(ArgList, Format);
  v3 = retaddr ^ _security_cookie;
  vsprintf(Buffer, Format, ArgList);
  Com_Printf("file %s, line %d: %s\n", *(const char **)(a1 + 524), *(_DWORD *)(*(_DWORD *)(a1 + 524) + 288), Buffer);
}

/* ---- PC_PushIndent  0x0043D890 ---- */
// PC_PushIndent | CONFIRMED | RTCW l_precomp.c:189 -- mallocs 0x14 node w/ 0x12345678 heap magic, links onto source[135] indent stack, tracks skip in [136]
int *__cdecl PC_PushIndent(_DWORD *a1, int a2, int a3)
{
  int *result;
  BOOL v4;
  int v5;
  int v6;

  result = Z_MallocInternal(0x14u);
  if ( result )
    *result++ = 305419896;
  *result = a2;
  result[2] = a1[131];
  v4 = a3 != 0;
  result[1] = v4;
  v5 = v4 + a1[136];
  v6 = a1[135];
  a1[136] = v5;
  result[3] = v6;
  a1[135] = result;
  return result;
}

/* ---- PC_PopIndent  0x0043D8F0 ---- */
// PC_PopIndent | CONFIRMED | RTCW l_precomp.c:206 -- pops source[135], writes back type/skip, frees node if heap magic present
void __cdecl PC_PopIndent(_DWORD *a1, _DWORD *a2, _DWORD *a3)
{
  _DWORD *v4;
  int v5;
  int v6;
  _DWORD *v7;

  *a1 = 0;
  *a2 = 0;
  v4 = (_DWORD *)a3[135];
  if ( v4 )
  {
    if ( v4[2] == a3[131] )
    {
      *a1 = *v4;
      *a2 = v4[1];
      v5 = a3[136];
      a3[135] = *(_DWORD *)(a3[135] + 12);
      v6 = v4[1];
      v7 = v4 - 1;
      a3[136] = v5 - v6;
      if ( *v7 == 305419896 )
        free(v7);
    }
  }
}

/* ---- PC_PushScript  0x0043D960 ---- */
// PC_PushScript | CONFIRMED | RTCW l_precomp.c
/* Q_stricmpn at 0x0043D977 is __usercall and its three arguments all arrive
   in registers:

       mov eax, 1869Fh      ; n  = 99999
       mov ecx, edi         ; s2 = the script being pushed
       mov edx, esi         ; s1 = the script already on the stack
       call Q_stricmpn

   Both scripts are script_t*, whose filename[] is at +0, so passing the struct
   pointers is passing the filenames.  99999 is retail's "compare it all"
   length; it is not a buffer bound and is reproduced as-is. */
void __cdecl PC_PushScript(int a1, int a2)
{
  int v2;
  int v3;

  v2 = *(_DWORD *)(a2 + 524);
  v3 = v2;
  if ( v2 )
  {
    while ( !a1 || Q_stricmpn((const char *)v3, (const char *)a1, 99999) )
    {
      v3 = *(_DWORD *)(v3 + 1384);
      if ( !v3 )
        goto LABEL_5;
    }
    SourceError(a2, "%s recursively included", (const char *)a1);
  }
  else
  {
LABEL_5:
    *(_DWORD *)(a1 + 1384) = v2;
    *(_DWORD *)(a2 + 524) = a1;
  }
}

/* ---- PC_InitTokenHeap  0x0043D9C0 ----   VERBATIM
 *
 * RTCW's PC_InitTokenHeap (l_precomp.c:254), whose entire body is commented
 * out in the reference -- the USE_TOKEN_HEAP free-list initialiser -- so the
 * compiler emitted a bare `retn`.  It sits where RTCW puts it, between
 * PC_PushScript (0x0043D960) and PC_CopyToken (0x0043D9D0), and the Mac 1.3
 * xSYM listing names `PC_InitTokenHeap` in botlib/l_precomp_mp.c
 * (docs/mac13_symbols.tsv:3309).  Zero xrefs in the Windows binary -- retail
 * never calls it, which is also true of RTCW once the heap is disabled.
 *
 * VERIFIED 0x0043D9C0.  The whole function is one byte, `C3`. */
void PC_InitTokenHeap( void )
{
}

/* ---- PC_CopyToken  0x0043D9D0 ---- */
// PC_CopyToken | CONFIRMED | RTCW l_precomp.c PC_CopyToken -- token alloc 0x434 (magic 0x12345678 + 0x430 payload), qmemcpy 0x430, bumps numtokens.
void *__cdecl PC_CopyToken(const void *token)
{
  int *v1;
  void *result;
  int v3;

  v1 = Z_MallocInternal(0x434u);
  if ( !v1 || (*v1 = 305419896, (result = v1 + 1) == 0) )
    Com_Error(ERR_FATAL, "EXE_ERR_OUT_OF_MEMORY");
  qmemcpy(result, token, 0x430u);
  v3 = numtokens + 1;
  *((_DWORD *)result + 266) = 0;
  numtokens = v3;
  return result;
}

/* ---- PC_FreeToken  0x0043DA30 ---- */
// PC_FreeToken | CONFIRMED | FreeMemory(token) then --numtokens.
void __cdecl PC_FreeToken(int a1)
{
  int v1;
  void *v2;

  v1 = *(_DWORD *)(a1 - 4);
  v2 = (void *)(a1 - 4);
  if ( v1 == 305419896 )
    free(v2);
  --numtokens;
}

/* ---- PC_ReadSourceToken  0x0043DA50 ---- */
// PC_ReadSourceToken | CONFIRMED | RTCW l_precomp.c
int __cdecl PC_ReadSourceToken(_DWORD *a1, int a2)
{
  _DWORD *v3;
  int v4;
  _DWORD *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  void *v10;
  int v11;
  int v12;
  _DWORD *v13;

  if ( *(_DWORD *)(a2 + 528) )
  {
LABEL_18:
    qmemcpy(a1, *(const void **)(a2 + 528), 0x430u);
    v11 = *(_DWORD *)(a2 + 528);
    v12 = *(_DWORD *)(v11 + 1064);
    v13 = (_DWORD *)(v11 - 4);
    *(_DWORD *)(a2 + 528) = v12;
    if ( *v13 == 305419896 )
      free(v13);
    --numtokens;
  }
  else
  {
    /* PS_ReadToken( script, token ) -- source order; retail's register order
       is ( token@<eax>, script@<ecx> ). */
    while ( !PS_ReadToken( (script_t *)*(_DWORD *)(a2 + 524), (token_t *)a1 ) )
    {
      if ( *(_DWORD *)(*(_DWORD *)(a2 + 524) + 264) >= *(_DWORD *)(*(_DWORD *)(a2 + 524) + 268) )
      {
        while ( *(_DWORD *)(a2 + 540) )
        {
          if ( *(_DWORD *)(*(_DWORD *)(a2 + 540) + 8) != *(_DWORD *)(a2 + 524) )
            break;
          SourceWarning(a2, "missing #endif");
          v3 = *(_DWORD **)(a2 + 540);
          if ( v3 )
          {
            if ( v3[2] == *(_DWORD *)(a2 + 524) )
            {
              *(_DWORD *)(a2 + 540) = v3[3];
              v4 = *(_DWORD *)(a2 + 544) - v3[1];
              v5 = v3 - 1;
              *(_DWORD *)(a2 + 544) = v4;
              if ( *v5 == 305419896 )
                free(v5);
            }
          }
        }
      }
      v6 = *(_DWORD *)(a2 + 524);
      v7 = *(_DWORD *)(v6 + 1384);
      if ( !v7 )
        return 0;
      *(_DWORD *)(a2 + 524) = v7;
      v8 = *(_DWORD *)(v6 + 308);
      if ( v8 )
      {
        v9 = *(_DWORD *)(v8 - 4);
        v10 = (void *)(v8 - 4);
        if ( v9 == 305419896 )
          free(v10);
      }
      if ( *(_DWORD *)(v6 - 4) == 305419896 )
        free((void *)(v6 - 4));
      if ( *(_DWORD *)(a2 + 528) )
        goto LABEL_18;
    }
  }
  return 1;
}

/* ---- PC_UnreadSourceToken  0x0043DBC0 ---- */
// PC_UnreadSourceToken | CONFIRMED | RTCW l_precomp.c PC_UnreadSourceToken.  Wraps PC_CopyToken then prepends to source->tokens at +528.  Retail: source in ESI, token in EAX.
int __cdecl PC_UnreadSourceToken(const void *a1, int a2)
{
  _DWORD *v2;

  v2 = PC_CopyToken(a1);
  v2[266] = *(_DWORD *)(a2 + 528);
  *(_DWORD *)(a2 + 528) = v2;
  return 1;
}

/* ---- PC_ReadDefineParms  0x0043DBF0 ---- */
// PC_ReadDefineParms | CONFIRMED | RTCW l_precomp.c:377
int __cdecl PC_ReadDefineParms(int a1, const char **a2, _DWORD *a3, int ArgList)
{
  int v4;
  const char **v6;
  const char *v7;
  int v8;
  int v9;
  _DWORD *v10;
  int v11;
  _DWORD *v12;
  int *v13;
  int *v14;
  void *v15;
  int v16;
  char *v17;
  char *LocalizedString_m;
  int v19;
  _DWORD *v20;
  int v21;
  int v22;
  int v23;
  _DWORD token[269]; // [esp+1Ch] [ebp-434h] BYREF
  unsigned int retaddr;

  v4 = a1;
  token[268] = retaddr ^ _security_cookie;
  if ( PC_ReadSourceToken(token, a1) )
  {
    v6 = a2;
    v7 = a2[3];
    v8 = ArgList;
    if ( (int)v7 <= ArgList )
    {
      v9 = 0;
      if ( (int)v7 > 0 )
      {
        do
          a3[v9++] = 0;
        while ( v9 < (int)a2[3] );
      }
      if ( !strcmp((const char *)token, "(") )
      {
        v11 = 0;
        v22 = 0;
        v21 = 0;
        v19 = 0;
        v20 = a3;
        while ( 2 )
        {
          if ( v11 >= v8 )
          {
            SourceError(v4, "define %s with too many parms", *v6);
            return 0;
          }
          else if ( v11 >= (int)v6[3] )
          {
            SourceWarning(v4, "define %s has too many parms", *v6);
            return 0;
          }
          else
          {
            v12 = 0;
            *v20 = 0;
            v23 = 1;
            while ( 1 )
            {
              if ( !PC_ReadSourceToken(token, a1) )
              {
                SourceError(a1, "define %s incomplete", *a2);
                return 0;
              }
              if ( !strcmp((const char *)token, ",") && v19 <= 0 )
                break;
              v23 = 0;
              if ( !strcmp((const char *)token, "(") )
              {
                ++v19;
              }
              else
              {
                if ( !strcmp((const char *)token, ")") && --v19 <= 0 )
                {
                  if ( !a3[(_DWORD)a2[3] - 1] )
                    SourceWarning(a1, "too few define parms");
                  v22 = 1;
                  goto LABEL_34;
                }
                if ( v21 < (int)a2[3] )
                {
                  v13 = (int *)malloc(0x434u);
                  v14 = v13;
                  if ( !v13 )
                  {
                    Sys_OutOfMemoryPrep();
                    LocalizedString_m = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_TITLE");
                    v17 = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_BODY");
                    MessageBoxA(0, v17, LocalizedString_m, 0x10u);
                    exit(-1);
                  }
                  Com_Memset(v13, 0, 0x434u);
                  v15 = v14 + 1;
                  *v14 = 305419896;
                  if ( v14 == (int *)-4 )
                    Com_Error(ERR_FATAL, "EXE_ERR_OUT_OF_MEMORY");
                  qmemcpy(v15, token, 0x430u);
                  v16 = numtokens + 1;
                  v14[267] = 0;
                  numtokens = v16;
                  v14[267] = 0;
                  if ( v12 )
                    v12[266] = v15;
                  else
                    *v20 = v15;
                  v12 = v14 + 1;
                }
              }
            }
            if ( v23 )
              SourceWarning(a1, "too many comma's");
LABEL_34:
            v6 = a2;
            v11 = ++v21;
            ++v20;
            if ( !v22 )
            {
              v4 = a1;
              v8 = ArgList;
              continue;
            }
            return 1;
          }
        }
      }
      else
      {
        v10 = PC_CopyToken(token);
        v10[266] = *(_DWORD *)(a1 + 528);
        *(_DWORD *)(a1 + 528) = v10;
        SourceError(a1, "define %s missing parms", *a2);
        return 0;
      }
    }
    else
    {
      SourceError(a1, "define with more than %d parameters", ArgList);
      return 0;
    }
  }
  else
  {
    SourceError(a1, "define %s missing parms", *a2);
    return 0;
  }
}

/* ---- PC_StringizeTokens  0x0043DF70 ---- */
// PC_StringizeTokens | CONFIRMED | Builds a quoted string by walking the token list via +1064 (token->next) and strncat-ing each token->string, capped at 1024 (MAX_TOKEN).
int __cdecl PC_StringizeTokens(int a1, char *Destination)
{
  *((_DWORD *)Destination + 256) = 1;
  *((_DWORD *)Destination + 262) = 0;
  *((_DWORD *)Destination + 263) = 0;
  *Destination = 0;
  for ( *(_WORD *)&Destination[strlen(Destination)] = 34; a1; a1 = *(_DWORD *)(a1 + 1064) )
    strncat(Destination, (const char *)a1, 1024 - strlen(Destination));
  /* Closing quote: the two-byte literal at 0x005682F8, a double quote and a
     NUL. */
  strncat(Destination, "\"", 1024 - strlen(Destination));
  return 1;
}

/* ---- PC_MergeTokens  0x0043E020 ---- */
// PC_MergeTokens | CONFIRMED | Concatenates two tokens when the type pair permits (4 = TT_NAME / 3 = TT_NUMBER in this build).
int __cdecl PC_MergeTokens(int a1, int a2)
{
  int v3;
  int v4;
  unsigned int v5;
  void *v6;

  v3 = *(_DWORD *)(a2 + 1024);
  if ( v3 == 4 )
  {
    v4 = *(_DWORD *)(a1 + 1024);
    if ( v4 == 4 || v4 == 3 )
    {
      v5 = strlen((const char *)a1) + 1;
      v6 = (void *)(a2 + strlen((const char *)a2));
LABEL_8:
      qmemcpy(v6, (const void *)a1, v5);
      return 1;
    }
  }
  if ( v3 == 1 && *(_DWORD *)(a1 + 1024) == 1 )
  {
    *(_BYTE *)(strlen((const char *)a2) + a2 - 1) = 0;
    v5 = strlen((const char *)++a1) + 1;
    v6 = (void *)(a2 + strlen((const char *)a2));
    goto LABEL_8;
  }
  return 0;
}

/* ---- PC_PrintDefineHashTable  0x0043E0C0 ---- */
// PC_PrintDefineHashTable | CONFIRMED | Iterates 1024 buckets (DEFINEHASHSIZE) printing "%4d:" then each define name; hash chain link is define->hashnext at +28.
void __cdecl PC_PrintDefineHashTable(int a1)
{
  int i;
  _DWORD *j;

  for ( i = 0; i < 1024; ++i )
  {
    Log_Write("%4d:", i);
    for ( j = *(_DWORD **)(a1 + 4 * i); j; j = (_DWORD *)j[7] )
      Log_Write((char *)off_56610C, *j);
    Log_Write("\n");
  }
}

/* ---- PC_NameHash  0x0043E110 ---- */
// PC_NameHash | HIGH | RTCW l_precomp.c define-name hash -- (h ^ ((h^(h>>10))>>10)) & 0x3FF, base 119 multiplier
int __cdecl PC_NameHash(char *a1)
{
  char v1;
  int v2;
  int v3;
  int v4;

  v1 = *a1;
  v2 = 0;
  if ( *a1 )
  {
    v3 = 119 - (_DWORD)a1;
    do
    {
      v4 = v1 * (_DWORD)&a1[v3];
      v1 = a1[1];
      v2 += v4;
      ++a1;
    }
    while ( v1 );
  }
  return ((unsigned __int16)v2 ^ (unsigned __int16)((v2 ^ (v2 >> 10)) >> 10)) & 0x3FF;
}

/* ---- PC_AddDefineToHash  0x0043E150 ---- */
// PC_AddDefineToHash | CONFIRMED | define->hashnext (+28) = bucket; bucket = define.
int __cdecl PC_AddDefineToHash(int a1, int a2)
{
  int result;

  result = PC_NameHash(*(char **)a2);
  *(_DWORD *)(a2 + 28) = *(_DWORD *)(a1 + 4 * result);
  *(_DWORD *)(a1 + 4 * result) = a2;
  return result;
}

/* ---- PC_FindHashedDefine  0x0043E170 ---- */
// PC_FindHashedDefine | CONFIRMED | RTCW l_precomp.c:576 -- hashes name, walks bucket via define->hashnext(+28), strcmp on define->name
int __cdecl PC_FindHashedDefine(int a1, char *a2)
{
  int v2;

  v2 = *(_DWORD *)(a1 + 4 * PC_NameHash(a2));
  if ( !v2 )
    return 0;
  while ( strcmp(*(const char **)v2, a2) )
  {
    v2 = *(_DWORD *)(v2 + 28);
    if ( !v2 )
      return 0;
  }
  return v2;
}

/* ---- PC_FindDefine  0x0043E1E0 ---- */
// PC_FindDefine | CONFIRMED | Linear strcmp walk over the non-hashed define list, link at +24 (define->next).
int __cdecl PC_FindDefine(int a1, const char *a2)
{
  int v2;

  v2 = a1;
  if ( !a1 )
    return 0;
  while ( strcmp(*(const char **)v2, a2) )
  {
    v2 = *(_DWORD *)(v2 + 24);
    if ( !v2 )
      return 0;
  }
  return v2;
}

/* ---- PC_FindDefineParm  0x0043E240 ---- */
// PC_FindDefineParm | CONFIRMED | RTCW l_precomp.c:614 -- walks define->parms(+16) via +1064 stride, returns ordinal or -1
int __cdecl PC_FindDefineParm(const char *a1, int a2)
{
  int v2;
  int v3;

  v2 = *(_DWORD *)(a2 + 16);
  v3 = 0;
  if ( !v2 )
    return -1;
  while ( strcmp((const char *)v2, a1) )
  {
    v2 = *(_DWORD *)(v2 + 1064);
    ++v3;
    if ( !v2 )
      return -1;
  }
  return v3;
}

/* ---- PC_FreeDefine  0x0043E2A0 ---- */
// PC_FreeDefine | CONFIRMED | RTCW l_precomp.c:634 -- frees parm token chain (+1064 stride) then define, decrementing numtokens
void __cdecl PC_FreeDefine(int a1)
{
  int v1;
  int v2;
  int v3;
  void *v4;
  int v5;
  int v6;
  int v7;
  void *v8;

  v1 = *(_DWORD *)(a1 + 16);
  if ( v1 )
  {
    do
    {
      v2 = *(_DWORD *)(v1 + 1064);
      v3 = *(_DWORD *)(v1 - 4);
      v4 = (void *)(v1 - 4);
      if ( v3 == 305419896 )
        free(v4);
      --numtokens;
      v1 = v2;
    }
    while ( v2 );
  }
  v5 = *(_DWORD *)(a1 + 20);
  if ( v5 )
  {
    do
    {
      v6 = *(_DWORD *)(v5 + 1064);
      v7 = *(_DWORD *)(v5 - 4);
      v8 = (void *)(v5 - 4);
      if ( v7 == 305419896 )
        free(v8);
      --numtokens;
      v5 = v6;
    }
    while ( v6 );
  }
  if ( *(_DWORD *)(a1 - 4) == 305419896 )
    free((void *)(a1 - 4));
}

/* ---- PC_AddBuiltinDefines  0x0043E340 ---- */
// PC_AddBuiltinDefines | CONFIRMED | RTCW l_precomp.c:668
int __cdecl PC_AddBuiltinDefines(int a1)
{
  const char *v1;
  const char **v2;
  unsigned int v3; // kr00_4
  int *v4;
  int *v5;
  _DWORD *v6;
  const char *v7;
  char *v8;
  int v9;
  char v10;
  int v11;
  int v12;
  int v13;
  int result;
  char *v15;
  char *LocalizedString_m;
  const char **v17;
  _DWORD v18[10]; // [esp+14h] [ebp-28h] BYREF

  v1 = "__LINE__";
  v2 = (const char **)v18;
  v18[0] = "__LINE__";
  v18[1] = 1;
  v18[2] = "__FILE__";
  v18[3] = 2;
  v18[4] = "__DATE__";
  v18[5] = 3;
  v18[6] = "__TIME__";
  v18[7] = 4;
  v18[8] = 0;
  v18[9] = 0;
  v17 = (const char **)v18;
  do
  {
    v3 = strlen(v1);
    v4 = (int *)malloc(v3 + 37);
    v5 = v4;
    if ( !v4 )
    {
      Sys_OutOfMemoryPrep();
      LocalizedString_m = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_TITLE");
      v15 = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_BODY");
      MessageBoxA(0, v15, LocalizedString_m, 0x10u);
      exit(-1);
    }
    Com_Memset(v4, 0, v3 + 37);
    v6 = v5 + 1;
    *v5 = 305419896;
    memset(v5 + 1, 0, 0x20u);
    v5[1] = (int)(v5 + 9);
    strcpy((char *)v5 + 36, v1);
    v7 = v2[1];
    v8 = (char *)v5[1];
    v6[1] |= 1u;
    v6[2] = v7;
    v9 = *(_DWORD *)(a1 + 536);
    v10 = *v8;
    v11 = 0;
    if ( *v8 )
    {
      v12 = 119 - (_DWORD)v8;
      do
      {
        v13 = v10 * (_DWORD)&v8[v12];
        v10 = v8[1];
        v11 += v13;
        ++v8;
      }
      while ( v10 );
      v2 = v17;
    }
    result = ((unsigned __int16)v11 ^ (unsigned __int16)((v11 ^ (v11 >> 10)) >> 10)) & 0x3FF;
    v2 += 2;
    v6[7] = *(_DWORD *)(v9 + 4 * result);
    *(_DWORD *)(v9 + 4 * result) = v6;
    v1 = *v2;
    v17 = v2;
  }
  while ( *v2 );
  return result;
}

/* ---- PC_ExpandBuiltinDefine  0x0043E4B0 ---- */
// PC_ExpandBuiltinDefine | CONFIRMED | RTCW l_precomp.c PC_ExpandBuiltinDefine -- switch on define->builtin (+8) producing __LINE__/__FILE__/__DATE__/__TIME__ token text; the __DATE__ and __TIME__ arms call ctime
/*
 * Q3 source order ( source, deftoken, define, firsttoken, lasttoken ):
 * `PC_CopyToken(deftoken)` (RTCW copies deftoken here) and
 * `strcpy(token->string, source->scriptstack->filename)` -- +524 is
 * source->scriptstack and script_t::filename is at offset 0.
 *
 * REACHABILITY.  Dead in retail: the switch only runs when define->builtin is
 * non-zero, and the only thing that sets it is PC_AddBuiltinDefines
 * (0x0043E340), which has NO XREFS in the binary.  CoD never registers
 * __LINE__, __FILE__, __DATE__ or __TIME__, so every define reaching
 * PC_ExpandDefine has builtin == 0 and this function is never entered.  The
 * default arm returns 1 with both out-parameters NULL, which is success.
 *
 * RETAIL DEFECT PRESERVED, and it is a live one if anybody ever calls
 * PC_AddBuiltinDefines: the __DATE__ and __TIME__ arms `free()` the pointer
 * returned by ctime, which is a static CRT buffer.  Q3 and RTCW have the
 * identical line.  Retail's own ctime wrapper is 0x00527746 (localtime then
 * asctime), so the pointer really is the CRT's, and freeing it is heap
 * corruption.  Do not "fix" it without recording the divergence, and do not
 * make PC_AddBuiltinDefines live without dealing with it first.
 *
 * The __LINE__ arm sets subtype 4104 = TT_DECIMAL|TT_INTEGER, exactly as
 * RTCW-MP src/botlib/l_precomp.c:714 (0x1008); no divergence.
 *
 * This is the 5-argument, PC_CopyToken(deftoken) form that RTCW-MP's botlib
 * carries, NOT the older 4-argument form in RTCW-MP's src/extractfuncs/ copy,
 * which builds the token on the stack from source->token.  CoD1 forked the
 * same late-Q3 snapshot RTCW did.
 */
/* <time.h> is not in this unit's include set.  These two declarations are
 * MSVC 6's exactly: `time_t` is `long` there, which is also what retail's
 * `__time32_t` is. */
extern long   time( long *timer );
extern char * ctime( const long *timer );

int __cdecl PC_ExpandBuiltinDefine(int source, token_t *deftoken, int define,
                                   token_t **firsttoken, token_t **lasttoken)
{
  token_t *token;
  char    *curtime;
  long     t;

  token = (token_t *)PC_CopyToken(deftoken);
  switch ( *(int *)(define + 8) )        /* define->builtin */
  {
    case 1:                              /* BUILTIN_LINE */
      sprintf(token->string, "%d", deftoken->line);
      token->intvalue   = deftoken->line;
      token->type       = TT_NUMBER;
      token->floatvalue = (double)deftoken->line;
      token->subtype    = TT_DECIMAL | TT_INTEGER;   /* 4104, as RTCW */
      *firsttoken = token;
      *lasttoken  = token;
      return 1;

    case 2:                              /* BUILTIN_FILE */
      strcpy(token->string, *(const char **)(source + 524));  /* scriptstack->filename */
      token->type    = TT_NAME;
      token->subtype = strlen(token->string);
      *firsttoken = token;
      *lasttoken  = token;
      return 1;

    case 3:                              /* BUILTIN_DATE */
      t = time(0);
      curtime = ctime(&t);
      strcpy(token->string, "\"");
      strncat(token->string, curtime + 4, 7);
      strncat(token->string + 7, curtime + 20, 4);
      strcat(token->string, "\"");
      free(curtime);                     /* retail defect -- see above */
      token->type    = TT_NAME;
      token->subtype = strlen(token->string);
      *firsttoken = token;
      *lasttoken  = token;
      return 1;

    case 4:                              /* BUILTIN_TIME */
      t = time(0);
      curtime = ctime(&t);
      strcpy(token->string, "\"");
      strncat(token->string, curtime + 11, 8);
      strcat(token->string, "\"");
      free(curtime);                     /* retail defect -- see above */
      token->type    = TT_NAME;
      token->subtype = strlen(token->string);
      *firsttoken = token;
      *lasttoken  = token;
      return 1;

    default:                             /* BUILTIN_STDC and anything else */
      *firsttoken = 0;
      *lasttoken  = 0;
      return 1;
  }
}

/* ---- PC_ExpandDefine  0x0043E6A0 ---- */
// PC_ExpandDefine | CONFIRMED | RTCW l_precomp.c:844
int __cdecl PC_ExpandDefine(int a1, _DWORD *a2, int a3, char **a4, char **a5)
{
  int v5;
  int result;
  int v7;
  int v8;
  int v9;
  _DWORD *i;
  int *v11;
  int *v12;
  void *v13;
  int v14;
  int v15;
  int DefineParm;
  _DWORD *v17;
  int v18;
  int v19;
  int v20;
  int v21;
  void *v22;
  int v23;
  char *v24;
  int v25;
  int v26;
  int v27;
  int v28;
  void *v29;
  char *LocalizedString_m;
  char *v31;
  char *v32;
  _DWORD *v33;
  _DWORD v34[128]; // [esp+Ch] [ebp-634h] BYREF
  char v35[1072]; // [esp+20Ch] [ebp-434h] BYREF
  unsigned int v36;
  unsigned int retaddr;

  v5 = a3;
  v36 = retaddr ^ _security_cookie;
  if ( *(_DWORD *)(a3 + 8) )
    /* Q3 source order (source, deftoken, define, firsttoken, lasttoken). */
    return PC_ExpandBuiltinDefine(a1, (token_t *)a2, a3, (token_t **)a4, (token_t **)a5);
  if ( *(_DWORD *)(a3 + 12) )
  {
    result = PC_ReadDefineParms(a1, (const char **)a3, v34, 128);
    if ( !result )
      return result;
  }
  v7 = *(_DWORD *)(a3 + 20);
  v8 = 0;
  v33 = 0;
  v32 = 0;
  if ( !v7 )
    goto LABEL_31;
  do
  {
    if ( *(_DWORD *)(v7 + 1024) == 4 && (v9 = PC_FindDefineParm((const char *)v7, v5), v9 >= 0) )
    {
      for ( i = (_DWORD *)v34[v9]; i; v32 = (char *)(v12 + 1) )
      {
        v11 = (int *)malloc(0x434u);
        v12 = v11;
        if ( !v11 )
        {
          Sys_OutOfMemoryPrep();
          LocalizedString_m = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_TITLE");
          v24 = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_BODY");
          MessageBoxA(0, v24, LocalizedString_m, 0x10u);
          exit(-1);
        }
        Com_Memset(v11, 0, 0x434u);
        v13 = v12 + 1;
        *v12 = 305419896;
        if ( v12 == (int *)-4 )
          Com_Error((errorParm_t)v13, "EXE_ERR_OUT_OF_MEMORY");
        qmemcpy(v13, i, 0x430u);
        v14 = numtokens + 1;
        v12[267] = 0;
        numtokens = v14;
        v12[267] = 0;
        if ( v32 )
          *((_DWORD *)v32 + 266) = v13;
        else
          v33 = v12 + 1;
        i = (_DWORD *)i[266];
      }
    }
    else
    {
      if ( *(_BYTE *)v7 != 35 || *(_BYTE *)(v7 + 1) )
      {
        v31 = (char *)v7;
      }
      else
      {
        v15 = *(_DWORD *)(v7 + 1064);
        if ( !v15 || (DefineParm = PC_FindDefineParm(*(const char **)(v7 + 1064), a3), DefineParm < 0) )
        {
          SourceWarning(a1, "stringizing operator without define parameter");
          goto LABEL_29;
        }
        v7 = v15;
        if ( !PC_StringizeTokens(v34[DefineParm], v35) )
        {
          SourceError(a1, "can't stringize tokens");
          return 0;
        }
        v31 = v35;
      }
      v17 = PC_CopyToken(v31);
      v17[266] = 0;
      if ( v32 )
        *((_DWORD *)v32 + 266) = v17;
      else
        v33 = v17;
      v32 = (char *)v17;
    }
LABEL_29:
    v7 = *(_DWORD *)(v7 + 1064);
    v5 = a3;
  }
  while ( v7 );
  v8 = (int)v33;
LABEL_31:
  if ( v8 )
  {
    while ( 1 )
    {
      v18 = *(_DWORD *)(v8 + 1064);
      if ( v18 && *(_BYTE *)v18 == 35 && *(_BYTE *)(v18 + 1) == 35 && (v19 = *(_DWORD *)(v18 + 1064)) != 0 )
      {
        if ( !PC_MergeTokens(*(_DWORD *)(v18 + 1064), v8) )
        {
          SourceError(a1, "can't merge %s with %s", (const char *)v8, (const char *)v19);
          return 0;
        }
        v20 = *(_DWORD *)(v8 + 1064);
        v21 = *(_DWORD *)(v20 - 4);
        v22 = (void *)(v20 - 4);
        if ( v21 == 305419896 )
          free(v22);
        v23 = numtokens;
        *(_DWORD *)(v8 + 1064) = *(_DWORD *)(v19 + 1064);
        numtokens = v23 - 1;
        if ( (char *)v19 == v32 )
          v32 = (char *)v8;
        if ( *(_DWORD *)(v19 - 4) == 305419896 )
          free((void *)(v19 - 4));
        --numtokens;
      }
      else
      {
        v8 = *(_DWORD *)(v8 + 1064);
      }
      if ( !v8 )
      {
        v5 = a3;
        v8 = (int)v33;
        break;
      }
    }
  }
  *a4 = (char *)v8;
  *a5 = v32;
  v25 = 0;
  if ( *(int *)(v5 + 12) > 0 )
  {
    do
    {
      v26 = v34[v25];
      if ( v26 )
      {
        do
        {
          v27 = *(_DWORD *)(v26 + 1064);
          v28 = *(_DWORD *)(v26 - 4);
          v29 = (void *)(v26 - 4);
          if ( v28 == 305419896 )
            free(v29);
          --numtokens;
          v26 = v27;
        }
        while ( v27 );
      }
      ++v25;
    }
    while ( v25 < *(_DWORD *)(a3 + 12) );
  }
  return 1;
}

/* ---- PC_ExpandDefineIntoSource  0x0043EA90 ---- */
// PC_ExpandDefineIntoSource | HIGH | RTCW l_precomp.c:906 -- calls PC_ExpandDefine then splices firsttoken/lasttoken onto source[528] token stack
int __cdecl PC_ExpandDefineIntoSource(int a1, int a2, _DWORD *a3)
{
  char *v3;
  char *v5; // [esp+0h] [ebp-8h] BYREF
  char *v6; // [esp+4h] [ebp-4h] BYREF

  if ( !PC_ExpandDefine(a2, a3, a1, &v5, &v6) )
    return 0;
  v3 = v5;
  if ( !v5 || !v6 )
    return 0;
  *((_DWORD *)v6 + 266) = *(_DWORD *)(a2 + 528);
  *(_DWORD *)(a2 + 528) = v3;
  return 1;
}

/* ---- PC_ConvertPath  0x0043EAF0 ---- */
// PC_ConvertPath | CONFIRMED | Collapses doubled '\\' and '//' separators in place.  l_precomp.c verbatim.
_BYTE *__cdecl PC_ConvertPath(_BYTE *a1)
{
  _BYTE *v1;
  char v2;
  char *v3;
  char v4;
  _BYTE *result;

  v1 = a1;
  while ( *v1 )
  {
    if ( (*v1 == 92 || *v1 == 47) && ((v2 = v1[1], v3 = v1 + 1, v2 == 92) || v2 == 47) )
    {
      do
      {
        v4 = *v3;
        *(v3 - 1) = *v3;
        ++v3;
      }
      while ( v4 );
    }
    else
    {
      ++v1;
    }
  }
  result = a1;
  if ( *a1 )
  {
    do
    {
      if ( *result == 47 || *result == 92 )
        *result = 92;
    }
    while ( *++result );
  }
  return result;
}

/* ---- PC_Directive_include  0x0043EB50 ----
 *
 * RETAIL STACK OVERFLOW, REPRODUCED DELIBERATELY.  Read before "fixing" it,
 * and before deciding this parser is safe on untrusted input.
 *
 * The `#include <a><b><c>` form accumulates the bracketed tokens into a
 * 260-byte stack buffer.  Retail's loop is
 *
 *     0043ECCE  push 104h        ; Count = 260
 *     0043ECD3  lea  edx, [Source]
 *     0043ECDF  lea  eax, [Destination]   ; char[260], [ebp-538h]
 *     0043ECE0  call _strncat
 *
 * and strncat's count bounds the SOURCE, not the room left in the
 * destination.  Each iteration appends up to 260 more bytes onto a buffer
 * that is 260 bytes total, and the loop runs once per token until a '>' or a
 * line break.  Destination sits below the token record and the cookie, so a
 * long enough `#include <...>` walks straight up the frame.  Q3 and RTCW have
 * the identical line; CoD inherited it unchanged.
 *
 * Left as retail because the binary is the spec here and because nothing on a
 * dedicated server can reach it -- the only callers of the whole precompiler
 * are the two client VM syscall dispatchers.  It is reachable in a CLIENT that
 * loads a .menu file from a downloaded IWD.  If this unit is ever linked
 * somewhere that untrusted input can reach it, this is the first thing to fix:
 * the bound wants to be the remaining space in Destination.
 */
// PC_Directive_include | CONFIRMED | RTCW l_precomp.c:969
int __cdecl PC_Directive_include(int a1)
{
  int v2;
  _DWORD *ScriptFile;
  char *v4;
  char v5;
  unsigned int v6;
  char *v7;
  char *v9;
  unsigned int v10;
  char *v11;
  char *v12;
  char v13;
  unsigned int v14; // [esp+4h] [ebp-53Ch] BYREF
  char Destination[260]; // [esp+8h] [ebp-538h] BYREF
  /* SPLIT-RECORD: one token_t at [ebp-434h]; PC_ReadSourceToken fills all
     1072 bytes of it.  type is +1024, linescrossed +1060. */
  token_t tok;                          /* [esp+10Ch] [ebp-434h] BYREF */
  char *Source = tok.string;
  unsigned int v19;
  unsigned int retaddr;

  v19 = retaddr ^ _security_cookie;
  if ( *(int *)(a1 + 544) > 0 )
    return 1;
  if ( !PC_ReadSourceToken(Source, a1) || tok.linescrossed > 0 )
  {
    SourceError(a1, "#include without file name");
    return 0;
  }
  if ( tok.type == TT_STRING )
  {
    /* StripDoubleQuotes is __fastcall(unused@ecx, string@edx). */
    StripDoubleQuotes(0, Source);
    PC_ConvertPath(Source);
    /* One argument, the filename; see botlib/l_precomp_pc.c. */
    ScriptFile = LoadScriptFile(Source);
    if ( ScriptFile )
      goto LABEL_31;
    v4 = (char *)(a1 + 260);
    do
    {
      v5 = *v4;
      Destination[(_DWORD)v4 - 260 - a1] = *v4;
      ++v4;
    }
    while ( v5 );
    v6 = strlen(Source) + 1;
    v7 = (char *)&v14 + 3;
    while ( *++v7 )
      ;
    qmemcpy(v7, Source, 4 * (v6 >> 2));
    v9 = &v7[4 * (v6 >> 2)];
    v10 = v6 & 3;
    qmemcpy(v9, &Source[4 * (v6 >> 2)], v10);
    v11 = &v9[v10];
LABEL_29:
    /* One argument -- Destination. */
    ScriptFile = LoadScriptFile(Destination);
    if ( !ScriptFile )
    {
      SourceError(a1, "file %s not found", Destination);
      return 0;
    }
LABEL_31:
    PC_PushScript((int)ScriptFile, a1);
    return 1;
  }
  if ( tok.type != TT_PUNCTUATION || Source[0] != '<' )
  {
    SourceError(a1, "#include without file name");
    return 0;
  }
  v12 = (char *)(a1 + 260);
  do
  {
    v13 = *v12;
    Destination[(_DWORD)v12 - 260 - a1] = *v12;
    ++v12;
  }
  while ( v13 );
  if ( PC_ReadSourceToken(Source, a1) )
  {
    while ( tok.linescrossed <= 0 )
    {
      if ( tok.type == TT_PUNCTUATION && Source[0] == '>' )
        goto LABEL_26;
      /* RETAIL OVERFLOW -- see the banner above this function.
         Destination is char[260]; the bound is the SOURCE length, not the
         remaining room, so this loop appends without limit. */
      strncat(Destination, Source, 0x104u);
      if ( !PC_ReadSourceToken(Source, a1) )
        goto LABEL_24;
    }
    PC_UnreadSourceToken(Source, a1);
  }
LABEL_24:
  if ( Source[0] != 62 )
    SourceWarning(a1, "#include missing trailing >");
LABEL_26:
  v14 = strlen(Destination);
  if ( v14 )
  {
    v11 = Destination;
    PC_ConvertPath(Destination);
    goto LABEL_29;
  }
  SourceError(a1, "#include without file name between < >");
  return 0;
}

/* ---- PC_ReadLine  0x0043EDB0 ---- */
// PC_ReadLine | HIGH | RTCW l_precomp.c:1043 -- reads tokens until line changes; backslash continues; unreads via PC_CopyToken onto source[528]
int __cdecl PC_ReadLine(int a1, _DWORD *token)
{
  int v3;
  int result;
  _DWORD *v5;

  v3 = 0;
  while ( PC_ReadSourceToken(token, a1) )
  {
    if ( token[265] > v3 )
    {
      v5 = PC_CopyToken(token);
      v5[266] = *(_DWORD *)(a1 + 528);
      *(_DWORD *)(a1 + 528) = v5;
      return 0;
    }
    result = 1;
    if ( strcmp((const char *)token, "\\") )
      return result;
    v3 = 1;
  }
  return 0;
}

/* ---- PC_WhiteSpaceBeforeToken  0x0043EE20 ---- */
// PC_WhiteSpaceBeforeToken | CONFIRMED | token->endwhitespace_p (+1052) - token->whitespace_p (+1048) > 0.
BOOL __cdecl PC_WhiteSpaceBeforeToken(int a1)
{
  return *(_DWORD *)(a1 + 1052) - *(_DWORD *)(a1 + 1048) > 0;
}

/* ---- PC_ClearTokenWhiteSpace  0x0043EE40 ---- */
// PC_ClearTokenWhiteSpace | CONFIRMED | Zeroes whitespace_p (+1048), endwhitespace_p (+1052) and linescrossed (+1060).  token_t: string[1024], type 1024, subtype 1028, intvalue 1032, floatvalue 1040 (8-aligned), whitespace_p 1048, endwhitespace_p 1052, line 1056, linescrossed 1060, next 1064 (see l_script.h).
_DWORD *__cdecl PC_ClearTokenWhiteSpace(_DWORD *result)
{
  result[262] = 0;
  result[263] = 0;
  result[265] = 0;
  return result;
}

/* ---- PC_Directive_undef  0x0043EE60 ---- */
// PC_Directive_undef | CONFIRMED | RTCW
int __cdecl PC_Directive_undef(_DWORD *a1)
{
  _DWORD *v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD *v7;
  /* SPLIT-RECORD: one token_t at [ebp-434h].  PC_ReadLine writes 1072 bytes
     into it and PC_CopyToken reads 1072 back out. */
  token_t tok;                          /* [esp+8h] [ebp-434h] BYREF */
  char *ArgList = tok.string;
  unsigned int v10;
  unsigned int retaddr;

  v10 = retaddr ^ _security_cookie;
  if ( (int)a1[136] <= 0 )
  {
    if ( !PC_ReadLine((int)a1, ArgList) )
    {
      SourceError((int)a1, "undef without name");
      return 0;
    }
    if ( tok.type != TT_NAME )
    {
      v2 = PC_CopyToken(ArgList);
      v2[266] = a1[132];
      a1[132] = v2;
      SourceError((int)a1, "expected name, found %s", ArgList);
      return 0;
    }
    v3 = PC_NameHash(ArgList);
    v4 = a1[134];
    v5 = *(_DWORD *)(v4 + 4 * v3);
    v6 = 0;
    v7 = (_DWORD *)(v4 + 4 * v3);
    if ( v5 )
    {
      while ( strcmp(*(const char **)v5, ArgList) )
      {
        v6 = v5;
        v5 = *(_DWORD *)(v5 + 28);
        if ( !v5 )
          return 1;
      }
      if ( (*(_BYTE *)(v5 + 4) & 1) != 0 )
      {
        SourceWarning((int)a1, "can't undef %s", ArgList);
      }
      else
      {
        if ( v6 )
          *(_DWORD *)(v6 + 28) = *(_DWORD *)(v5 + 28);
        else
          *v7 = *(_DWORD *)(v5 + 28);
        PC_FreeDefine(v5);
      }
    }
  }
  return 1;
}

/* ---- PC_Directive_define  0x0043EFE0 ---- */
// PC_Directive_define | CONFIRMED | RTCW l_precomp.c:1161
int __cdecl PC_Directive_define(_DWORD *a1)
{
  _DWORD *v1;
  _DWORD *v3;
  int HashedDefine;
  int *v5;
  char *v6;
  int v7;
  int v8;
  _DWORD *v9;
  _DWORD *v10;
  _DWORD *v11;
  const char *v12;
  const char *v13;
  /* SPLIT-RECORD: one token_t at [ebp-434h]; type (+1024), whitespace_p
     (+1048) and endwhitespace_p (+1052) are read off it. */
  token_t tok;                          /* [esp+8h] [ebp-434h] BYREF */
  char *ArgList = tok.string;
  unsigned int v18;
  unsigned int retaddr;

  v1 = a1;
  v18 = retaddr ^ _security_cookie;
  if ( (int)a1[136] > 0 )
    return 1;
  if ( !PC_ReadLine((int)a1, ArgList) )
  {
    SourceError((int)a1, "#define without name");
    return 0;
  }
  if ( tok.type != TT_NAME )
  {
    v3 = PC_CopyToken(ArgList);
    v3[266] = a1[132];
    a1[132] = v3;
    SourceError((int)a1, "expected name after #define, found %s", ArgList);
    return 0;
  }
  HashedDefine = PC_FindHashedDefine(a1[134], ArgList);
  if ( HashedDefine )
  {
    if ( (*(_BYTE *)(HashedDefine + 4) & 1) != 0 )
    {
      SourceError((int)a1, "can't redefine %s", ArgList);
      return 0;
    }
    SourceWarning((int)a1, "redefinition of %s", ArgList);
    PC_UnreadSourceToken(ArgList, (int)a1);
    if ( !PC_Directive_undef(a1) )
      return 0;
  }
  v5 = Z_MallocInternal(strlen(ArgList) + 37);
  if ( v5 )
  {
    *v5 = 305419896;
    v6 = (char *)(v5 + 1);
  }
  else
  {
    v6 = 0;
  }
  memset(v6, 0, 0x20u);
  *(_DWORD *)v6 = v6 + 32;
  strcpy(v6 + 32, ArgList);
  v7 = a1[134];
  v8 = PC_NameHash(*(char **)v6);
  *((_DWORD *)v6 + 7) = *(_DWORD *)(v7 + 4 * v8);
  *(_DWORD *)(v7 + 4 * v8) = v6;
  if ( !PC_ReadLine((int)a1, ArgList) )
    return 1;
  /* no whitespace between the macro name and the '(' -- i.e. it takes
     parameters.  Retail compares the two pointers by subtraction. */
  if ( tok.endwhitespace_p - tok.whitespace_p <= 0 && !strcmp(ArgList, "(") )
  {
    v9 = a1;
    v10 = 0;
    if ( !PC_CheckTokenString(a1, ")") )
    {
      if ( !PC_ReadLine((int)a1, ArgList) )
      {
LABEL_29:
        SourceError((int)v9, "expected define parameter");
        return 0;
      }
      while ( 1 )
      {
        if ( tok.type != TT_NAME )
        {
          SourceError((int)v9, "invalid define parameter");
          return 0;
        }
        if ( PC_FindDefineParm(ArgList, (int)v6) >= 0 )
        {
          SourceError((int)v9, "two the same define parameters");
          return 0;
        }
        v11 = PC_CopyToken(ArgList);
        v11[262] = 0;
        v11[263] = 0;
        v11[265] = 0;
        v11[266] = 0;
        if ( v10 )
          v10[266] = v11;
        else
          *((_DWORD *)v6 + 4) = v11;
        v10 = v11;
        ++*((_DWORD *)v6 + 3);
        if ( !PC_ReadLine((int)v9, ArgList) )
        {
          SourceError((int)v9, "define parameters not terminated");
          return 0;
        }
        if ( !strcmp(ArgList, ")") )
          break;
        if ( strcmp(ArgList, ",") )
        {
          SourceError((int)a1, "define not terminated");
          return 0;
        }
        v9 = a1;
        if ( !PC_ReadLine((int)a1, ArgList) )
          goto LABEL_29;
      }
      v9 = a1;
    }
    if ( !PC_ReadLine((int)v9, ArgList) )
      return 1;
    v1 = a1;
  }
  v13 = 0;
  do
  {
    v12 = (const char *)PC_CopyToken(ArgList);
    if ( *((_DWORD *)v12 + 256) == 4 )
    {
      if ( !strcmp(v12, *(const char **)v6) )
      {
        SourceError((int)a1, "recursive define (removed recursion)");
        v1 = a1;
        continue;
      }
      v1 = a1;
    }
    *((_DWORD *)v12 + 262) = 0;
    *((_DWORD *)v12 + 263) = 0;
    *((_DWORD *)v12 + 265) = 0;
    *((_DWORD *)v12 + 266) = 0;
    if ( v13 )
      *((_DWORD *)v13 + 266) = v12;
    else
      *((_DWORD *)v6 + 5) = v12;
    v13 = v12;
  }
  while ( PC_ReadLine((int)v1, ArgList) );
  if ( v13 && (!strcmp(*((const char **)v6 + 5), "##") || !strcmp(v13, "##")) )
  {
    SourceError((int)v1, "define with misplaced ##");
    return 0;
  }
  return 1;
}

/* ---- PC_DefineFromString  0x0043F480 ---- */
// PC_DefineFromString | CONFIRMED | RTCW l_precomp.c PC_DefineFromString
int __fastcall PC_DefineFromString(int a1, const char *a2)
{
  _DWORD *ScriptMemory;
  int *v3;
  void *v4;
  int v5;
  int v6;
  int i;
  _DWORD *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  void *v13;
  char Destination[1628]; // [esp+10h] [ebp-660h] BYREF
  unsigned int v16;
  unsigned int retaddr;

  v16 = retaddr ^ _security_cookie;
  /* source order ( ptr, length, name ) -- retail lifts length into EAX */
  ScriptMemory = (_DWORD *)LoadScriptMemory((char *)a2, strlen(a2), "*extern");
  memset(Destination, 0, 0x658u);
  strncpy(Destination, "*extern", 0x104u);
  *(_DWORD *)&Destination[524] = ScriptMemory;
  v3 = Z_MallocInternal(0x1004u);
  if ( v3 )
  {
    *v3 = 305419896;
    v4 = v3 + 1;
  }
  else
  {
    v4 = 0;
  }
  memset(v4, 0, 0x1000u);
  *(_DWORD *)&Destination[536] = v4;
  v5 = PC_Directive_define(Destination);
  v6 = *(_DWORD *)&Destination[528];
  for ( i = *(_DWORD *)&Destination[528]; v6; i = v6 )
  {
    v6 = *(_DWORD *)(v6 + 1064);
    v8 = (_DWORD *)(i - 4);
    *(_DWORD *)&Destination[528] = v6;
    if ( *v8 == 305419896 )
    {
      free(v8);
      v6 = *(_DWORD *)&Destination[528];
    }
    --numtokens;
  }
  v9 = 0;
  v10 = 0;
  while ( !*(_DWORD *)(*(_DWORD *)&Destination[536] + 4 * v10) )
  {
    if ( ++v10 >= 1024 )
      goto LABEL_13;
  }
  v9 = *(_DWORD *)(*(_DWORD *)&Destination[536] + 4 * v10);
LABEL_13:
  if ( *(_DWORD *)(*(_DWORD *)&Destination[536] - 4) == 305419896 )
    free((void *)(*(_DWORD *)&Destination[536] - 4));
  v11 = ScriptMemory[77];
  if ( v11 )
  {
    v12 = *(_DWORD *)(v11 - 4);
    v13 = (void *)(v11 - 4);
    if ( v12 == 305419896 )
      free(v13);
  }
  if ( *(ScriptMemory - 1) == 305419896 )
    free(ScriptMemory - 1);
  if ( v5 > 0 )
    return v9;
  if ( *(_DWORD *)&Destination[532] )
    PC_FreeDefine(v9);
  return 0;
}

/* ---- PC_AddDefineToSourceFromString  0x0043F620 ---- */
// PC_AddDefineToSourceFromString | CONFIRMED | PC_DefineFromString then inlined PC_AddDefineToHash against source->definehash at +536.
int __fastcall PC_AddDefineToSourceFromString(int a1, const char *a2, int a3)
{
  int result;
  int v4;
  int v5;
  int v6;

  result = PC_DefineFromString(a1, a2);
  v4 = result;
  if ( result )
  {
    v5 = *(_DWORD *)(a3 + 536);
    v6 = PC_NameHash(*(char **)result);
    *(_DWORD *)(v4 + 28) = *(_DWORD *)(v5 + 4 * v6);
    *(_DWORD *)(v5 + 4 * v6) = v4;
    return 1;
  }
  return result;
}

/* ---- PC_AddGlobalDefine  0x0043F660 ---- */
// PC_AddGlobalDefine | CONFIRMED | Pushes onto the globaldefines list (link at +24).
int __fastcall PC_AddGlobalDefine(int a1, const char *a2)
{
  int result;

  result = PC_DefineFromString(a1, a2);
  if ( result )
  {
    *(_DWORD *)(result + 24) = globaldefines;
    globaldefines = (void *)result;
    return 1;
  }
  return result;
}

/* ---- PC_RemoveGlobalDefine  0x0043F680 ---- */
// PC_RemoveGlobalDefine | CONFIRMED | PC_FindDefine(globaldefines, name) then PC_FreeDefine.
int __cdecl PC_RemoveGlobalDefine(const char *a1)
{
  int Define;

  Define = PC_FindDefine((int)globaldefines, a1);
  if ( !Define )
    return 0;
  PC_FreeDefine(Define);
  return 1;
}

/* ---- PC_RemoveAllGlobalDefines  0x0043F6B0 ---- */
// PC_RemoveAllGlobalDefines | CONFIRMED | Drains globaldefines through PC_FreeDefine.
void PC_RemoveAllGlobalDefines()
{
  _DWORD *i;

  for ( i = globaldefines; globaldefines; i = globaldefines )
  {
    globaldefines = (void *)i[6];
    PC_FreeDefine((int)i);
  }
}

/* ---- PC_CopyDefine  0x0043F6E0 ---- */
// PC_CopyDefine | HIGH | RTCW l_precomp.c PC_CopyDefine -- allocates via Z_Malloc(strlen(name)+37), copies flags/builtin/numparms/name, then walks parms and tokens lists into new define.  Divergence: CoD1 copies tokens list first (source[+20]) then parms list (source[+16]); RTCW does parms then tokens.  Field layout unchanged.
void *__cdecl PC_CopyDefine(void *source, const void *define)
{
  _DWORD *v2;
  int *v3;
  char *v4;
  _DWORD *v5;
  int *v6;
  int *v7;
  void *v8;
  int v9;
  _DWORD *v10;
  int *v11;
  int *v12;
  void *v13;
  int v14;
  char *v16;
  char *LocalizedString_m;
  _DWORD *v18;
  _DWORD *v19;

  v2 = define;
  v3 = Z_MallocInternal(strlen(*(const char **)define) + 37);
  if ( v3 )
  {
    *v3 = 305419896;
    v4 = (char *)(v3 + 1);
  }
  else
  {
    v4 = 0;
  }
  *(_DWORD *)v4 = v4 + 32;
  strcpy(v4 + 32, *(const char **)define);
  *((_DWORD *)v4 + 1) = *((_DWORD *)define + 1);
  *((_DWORD *)v4 + 2) = *((_DWORD *)define + 2);
  *((_DWORD *)v4 + 3) = *((_DWORD *)define + 3);
  *((_DWORD *)v4 + 6) = 0;
  *((_DWORD *)v4 + 7) = 0;
  *((_DWORD *)v4 + 5) = 0;
  v5 = (_DWORD *)*((_DWORD *)define + 5);
  v18 = 0;
  if ( v5 )
  {
    while ( 1 )
    {
      v6 = (int *)malloc(0x434u);
      v7 = v6;
      if ( !v6 )
        break;
      Com_Memset(v6, 0, 0x434u);
      v8 = v7 + 1;
      *v7 = 305419896;
      if ( v7 == (int *)-4 )
        Com_Error((errorParm_t)v8, "EXE_ERR_OUT_OF_MEMORY");
      qmemcpy(v8, v5, 0x430u);
      v2 = define;
      v9 = numtokens + 1;
      v7[267] = 0;
      numtokens = v9;
      v7[267] = 0;
      if ( v18 )
        v18[266] = v8;
      else
        *((_DWORD *)v4 + 5) = v8;
      v5 = (_DWORD *)v5[266];
      v18 = v7 + 1;
      if ( !v5 )
        goto LABEL_12;
    }
LABEL_21:
    Sys_OutOfMemoryPrep();
    LocalizedString_m = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_TITLE");
    v16 = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_BODY");
    MessageBoxA(0, v16, LocalizedString_m, 0x10u);
    exit(-1);
  }
LABEL_12:
  *((_DWORD *)v4 + 4) = 0;
  v10 = (_DWORD *)v2[4];
  v19 = 0;
  if ( v10 )
  {
    while ( 1 )
    {
      v11 = (int *)malloc(0x434u);
      v12 = v11;
      if ( !v11 )
        break;
      Com_Memset(v11, 0, 0x434u);
      v13 = v12 + 1;
      *v12 = 305419896;
      if ( v12 == (int *)-4 )
        Com_Error((errorParm_t)v13, "EXE_ERR_OUT_OF_MEMORY");
      qmemcpy(v13, v10, 0x430u);
      v14 = numtokens + 1;
      v12[267] = 0;
      numtokens = v14;
      v12[267] = 0;
      if ( v19 )
        v19[266] = v13;
      else
        *((_DWORD *)v4 + 4) = v13;
      v10 = (_DWORD *)v10[266];
      v19 = v12 + 1;
      if ( !v10 )
        return v4;
    }
    goto LABEL_21;
  }
  return v4;
}

/* ---- PC_AddGlobalDefinesToSource  0x0043F8E0 ---- */
// PC_AddGlobalDefinesToSource | CONFIRMED | RTCW l_precomp.c PC_AddGlobalDefinesToSource -- iterates globaldefines list (via ->next at +24), copies each via PC_CopyDefine, computes name hash (poly 119, mask 0x3FF), inserts into source->definehash at +536.
void __cdecl PC_AddGlobalDefinesToSource(void *source)
{
  _DWORD *v1;
  char **v2;
  char *v3;
  char v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;

  v1 = globaldefines;
  if ( globaldefines )
  {
    do
    {
      v2 = (char **)PC_CopyDefine(source, v1);
      v3 = *v2;
      v4 = **v2;
      v5 = *((_DWORD *)source + 134);
      v6 = 0;
      if ( v4 )
      {
        v7 = 119 - (_DWORD)v3;
        do
        {
          v8 = v4 * (_DWORD)&v3[v7];
          v4 = v3[1];
          v6 += v8;
          ++v3;
        }
        while ( v4 );
        v5 = *((_DWORD *)source + 134);
      }
      v9 = ((unsigned __int16)v6 ^ (unsigned __int16)((v6 ^ (v6 >> 10)) >> 10)) & 0x3FF;
      v2[7] = *(char **)(v5 + 4 * v9);
      *(_DWORD *)(v5 + 4 * v9) = v2;
      v1 = (_DWORD *)v1[6];
    }
    while ( v1 );
  }
}

/* ---- PC_Directive_if_def  0x0043F960 ---- */
// PC_Directive_if_def | CONFIRMED | RTCW
int __cdecl PC_Directive_if_def(_DWORD *a1, int a2)
{
  _DWORD *v4;
  int HashedDefine;
  /* SPLIT-RECORD: one token_t at [ebp-434h] that PC_ReadLine fills. */
  token_t tok;                          /* [esp+4h] [ebp-434h] BYREF */
  char *ArgList = tok.string;
  unsigned int v8;
  unsigned int retaddr;

  v8 = retaddr ^ _security_cookie;
  if ( PC_ReadLine((int)a1, ArgList) )
  {
    if ( tok.type == TT_NAME )
    {
      HashedDefine = PC_FindHashedDefine(a1[134], ArgList);
      PC_PushIndent(a1, a2, (HashedDefine == 0) == (a2 == 8));
      return 1;
    }
    else
    {
      v4 = PC_CopyToken(ArgList);
      v4[266] = a1[132];
      a1[132] = v4;
      SourceError((int)a1, "expected name after #ifdef, found %s", ArgList);
      return 0;
    }
  }
  else
  {
    SourceError((int)a1, "#ifdef without name");
    return 0;
  }
}

/* ---- PC_Directive_ifdef  0x0043FA60 ---- */
// PC_Directive_ifdef | CONFIRMED | PC_Directive_if_def(source, 8).  8 = INDENT_IFDEF.
int __cdecl PC_Directive_ifdef(_DWORD *a1)
{
  return PC_Directive_if_def(a1, 8);
}

/* ---- PC_Directive_ifndef  0x0043FA80 ---- */
// PC_Directive_ifndef | CONFIRMED | PC_Directive_if_def(source, 16).  16 = INDENT_IFNDEF.
int __cdecl PC_Directive_ifndef(_DWORD *a1)
{
  return PC_Directive_if_def(a1, 16);
}

/* ---- PC_Directive_else  0x0043FAA0 ---- */
// PC_Directive_else | CONFIRMED | RTCW
int __cdecl PC_Directive_else(_DWORD *a1)
{
  int v2; // [esp+4h] [ebp-8h] BYREF
  int v3; // [esp+8h] [ebp-4h] BYREF

  PC_PopIndent(&v2, &v3, a1);
  if ( v2 )
  {
    if ( v2 == 2 )
    {
      SourceError((int)a1, "#else after #else");
      return 0;
    }
    else
    {
      PC_PushIndent(a1, 2, v3 == 0);
      return 1;
    }
  }
  else
  {
    SourceError((int)a1, "misplaced #else");
    return 0;
  }
}

/* ---- PC_Directive_endif  0x0043FB10 ---- */
// PC_Directive_endif | CONFIRMED | RTCW
int __cdecl PC_Directive_endif(_DWORD *a1)
{
  int v2; // [esp+4h] [ebp-8h] BYREF
  int v3; // [esp+8h] [ebp-4h] BYREF

  PC_PopIndent(&v2, &v3, a1);
  if ( v2 )
    return 1;
  SourceError((int)a1, "misplaced #endif");
  return 0;
}

/* ---- PC_OperatorPriority  0x0043FB50 ---- */
// PC_OperatorPriority | CONFIRMED | Dense switch mapping punctuation subtype to C operator precedence; feeds PC_EvaluateTokens.
int __cdecl PC_OperatorPriority(int a1)
{
  int result;

  switch ( a1 )
  {
    case 5:
      result = 7;
      break;
    case 6:
      result = 6;
      break;
    case 7:
    case 8:
    case 37:
    case 38:
      result = 12;
      break;
    case 9:
    case 10:
      result = 11;
      break;
    case 21:
    case 22:
      result = 13;
      break;
    case 26:
    case 27:
    case 28:
      result = 15;
      break;
    case 29:
    case 30:
      result = 14;
      break;
    case 32:
      result = 10;
      break;
    case 33:
      result = 8;
      break;
    case 34:
      result = 9;
      break;
    case 35:
    case 36:
      result = 16;
      break;
    case 42:
    case 43:
      result = 5;
      break;
    default:
      result = 0;
      break;
  }
  return result;
}

/* ---- PC_EvaluateTokens  0x0043FC10 ---- */
// PC_EvaluateTokens | CONFIRMED | RTCW l_precomp.c:1784
/* Retail is __usercall(tokens@ecx, source, intvalue, floatvalue, integer) --
 * `mov ebx, ecx` at 0x0043FC2A, then `mov ecx, [ebp+arg_0]` and
 * `mov eax, [ebp+arg_4]` for the two stack arguments the prologue touches.
 * tokens is Q3/RTCW's parameter, in Q3's own order; both callers
 * (PC_Evaluate, PC_DollarEvaluate) pass the firsttoken list head they free
 * immediately afterwards. */
int __cdecl PC_EvaluateTokens(int a1, _DWORD *tokens, _DWORD *a2, _DWORD *a3, int a4)
{
  double *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  char *v10;
  int v11;
  _DWORD *v12;
  double *v13;
  double *v14;
  int v15;
  _DWORD *v16;
  int *v17;
  double *v18;
  int *v19;
  int v20;
  int v21;
  int v22;
  double v23;
  double v24;
  double v25;
  BOOL v26;
  double v27;
  BOOL v28;
  double v29;
  double v30;
  double v31;
  double v32;
  double v33;
  double v34;
  double v35;
  int v36;
  int v37;
  int v38;
  int v39;
  int v40;
  _DWORD *v41;
  _DWORD *i;
  double *j;
  int v45;
  char *v46;
  _DWORD *v47;
  int v48;
  int v49;
  int v50;
  double *v51;
  int v52;
  _DWORD *v53;
  int v54;
  int v55;
  int v56;
  double *v57;
  char *v58;
  int v59;
  double *v60;
  int v61;
  double v62;
  int v63;
  /* Retail's frame is `sub esp, 0D3Ch` at 0x0043FC16 and the two heaps tile
   * it exactly: [ebp-D00h]..[ebp-800h] is 0x500 = 64 x 20, RTCW's
   * `operator_t operator_heap[MAX_OPERATORS]`, and [ebp-800h]..[ebp-0] is
   * 0x800 = 64 x 32, `value_t value_heap[MAX_VALUES]`.  Live: the engine's
   * PC_* traps parse the UI .menu files, so any `#if` in one runs this. */
  char v64[64 * 20]; // [esp+48h] [ebp-D00h] BYREF  -- operator_heap
  char v65[64 * 32]; // [esp+548h] [ebp-800h] BYREF -- value_heap

  v5 = 0;
  v6 = 0;
  v7 = (int)tokens;
  v8 = a1;
  v61 = 0;
  v54 = 0;
  v50 = 0;
  v55 = 0;
  v63 = 0;
  v56 = 0;
  v47 = 0;
  v53 = 0;
  v51 = 0;
  v57 = 0;
  if ( a2 )
    *a2 = 0;
  if ( a3 )
  {
    *a3 = 0;
    a3[1] = 0;
  }
  if ( !v7 )
  {
LABEL_80:
    SourceError(a1, "trailing operator in #if/#elif");
    goto LABEL_83;
  }
  v60 = (double *)v65;
  v58 = v64;
  while ( 1 )
  {
    if ( *(_DWORD *)(v7 + 1024) == 3 )
    {
      if ( v6 )
        goto LABEL_77;
      if ( v56 >= 64 )
      {
        SourceError(v8, "out of value space\n");
        goto LABEL_83;
      }
      v14 = v60;
      v60 += 4;
      v15 = *(_DWORD *)(v7 + 1032);
      ++v56;
      if ( v55 )
      {
        *(_DWORD *)v14 = -v15;
        v14[1] = -*(double *)(v7 + 1040);
      }
      else
      {
        *(_DWORD *)v14 = v15;
        *((_DWORD *)v14 + 2) = *(_DWORD *)(v7 + 1040);
        *((_DWORD *)v14 + 3) = *(_DWORD *)(v7 + 1044);
      }
      *((_DWORD *)v14 + 4) = v54;
      *((_DWORD *)v14 + 6) = 0;
      *((_DWORD *)v14 + 5) = v5;
      if ( v5 )
        *((_DWORD *)v5 + 6) = v14;
      else
        v57 = v14;
      v51 = v14;
      v55 = 0;
      goto LABEL_64;
    }
    if ( *(_DWORD *)(v7 + 1024) == 4 )
    {
      if ( v6 || v55 )
      {
LABEL_77:
        SourceError(v8, "syntax error in #if/#elif");
        goto LABEL_83;
      }
      if ( strcmp((const char *)v7, "defined") )
      {
        SourceError(a1, "undefined name %s in #if/#elif", (const char *)v7);
        goto LABEL_83;
      }
      v7 = *(_DWORD *)(v7 + 1064);
      if ( !strcmp((const char *)v7, "(") )
      {
        v7 = *(_DWORD *)(v7 + 1064);
        v61 = 1;
      }
      if ( !v7 || *(_DWORD *)(v7 + 1024) != 4 )
      {
        SourceError(a1, "defined without name in #if/#elif");
        goto LABEL_83;
      }
      if ( v56 >= 64 )
      {
        SourceError(a1, "out of value space\n");
        goto LABEL_83;
      }
      v13 = v60;
      v60 += 4;
      ++v56;
      if ( PC_FindHashedDefine(*(_DWORD *)(a1 + 536), (char *)v7) )
      {
        *(_DWORD *)v13 = 1;
        *((_DWORD *)v13 + 2) = 0;
        *((_DWORD *)v13 + 3) = 1072693248;
      }
      else
      {
        *(_DWORD *)v13 = 0;
        *((_DWORD *)v13 + 2) = 0;
        *((_DWORD *)v13 + 3) = 0;
      }
      *((_DWORD *)v13 + 4) = v54;
      *((_DWORD *)v13 + 6) = 0;
      *((_DWORD *)v13 + 5) = v51;
      if ( v51 )
        *((_DWORD *)v51 + 6) = v13;
      else
        v57 = v13;
      v51 = v13;
      if ( v61 )
      {
        v7 = *(_DWORD *)(v7 + 1064);
        if ( !v7 || strcmp((const char *)v7, ")") )
        {
          SourceError(a1, "defined without ) in #if/#elif");
          goto LABEL_83;
        }
      }
      v61 = 0;
LABEL_64:
      v6 = 1;
      goto LABEL_65;
    }
    if ( *(_DWORD *)(v7 + 1024) != 5 )
    {
      SourceError(v8, "unknown %s in #if/#elif", (const char *)v7);
      goto LABEL_83;
    }
    if ( v55 )
    {
      SourceError(v8, "misplaced minus sign in #if/#elif");
      goto LABEL_83;
    }
    v9 = *(_DWORD *)(v7 + 1028);
    if ( v9 == 44 )
    {
      ++v54;
    }
    else if ( v9 == 45 )
    {
      if ( --v54 < 0 )
      {
        SourceError(v8, "too many ) in #if/#elsif");
        goto LABEL_83;
      }
    }
    else
    {
      if ( !a4 && (v9 == 35 || v9 == 28 || v9 == 21 || v9 == 22 || v9 == 32 || v9 == 33 || v9 == 34) )
      {
        SourceError(v8, "illigal operator %s on floating point operands\n", (const char *)v7);
        goto LABEL_83;
      }
      switch ( v9 )
      {
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 21:
        case 22:
        case 26:
        case 27:
        case 28:
        case 29:
        case 32:
        case 33:
        case 34:
        case 37:
        case 38:
        case 42:
        case 43:
          if ( v6 )
            goto LABEL_31;
          SourceError(v8, "operator %s after operator in #if/#elif", (const char *)v7);
          goto LABEL_83;
        case 16:
        case 17:
          SourceError(v8, "++ or -- used in #if/#elif");
          goto LABEL_31;
        case 30:
          if ( !v6 )
            v55 = 1;
          goto LABEL_31;
        case 35:
        case 36:
          if ( v6 )
          {
            SourceError(v8, "! or ~ after value in #if/#elif");
            goto LABEL_83;
          }
LABEL_31:
          if ( v55 )
            break;
          if ( v63 >= 64 )
          {
            SourceError(a1, "out of operator space\n");
            goto LABEL_83;
          }
          v10 = v58;
          v58 += 20;
          /* v10 and v12 are the same pointer -- the 20-byte operator_t just
             carved off the 64-entry heap at [ebp-D00h]. */
          v12 = (_DWORD *)v10;
          *(_DWORD *)v10 = *(_DWORD *)(v7 + 1028);
          ++v63;
          v11 = PC_OperatorPriority(*(_DWORD *)(v7 + 1028));
          v12[1] = v11;
          v6 = 0;
          v12[2] = v54;
          v12[4] = 0;
          v12[3] = v47;
          if ( v47 )
            v47[4] = v12;
          else
            v53 = v12;
          v47 = v12;
          break;
        default:
          SourceError(v8, "invalid operator %s in #if/#elif", (const char *)v7);
          goto LABEL_83;
      }
    }
LABEL_65:
    v7 = *(_DWORD *)(v7 + 1064);
    if ( !v7 )
      break;
    v8 = a1;
    v5 = v51;
  }
  if ( !v6 )
    goto LABEL_80;
  if ( v54 )
  {
    SourceError(a1, "too many ( in #if/#elif");
LABEL_83:
    v50 = 1;
  }
  v52 = 0;
  v59 = 0;
  v62 = 0.0;
  if ( v50 )
    goto LABEL_163;
  while ( 2 )
  {
    v16 = v53;
    if ( !v53 )
      goto LABEL_164;
    v17 = (int *)v53[4];
    v18 = v57;
    v19 = v53;
    if ( v17 )
    {
      while ( 1 )
      {
        v20 = v19[2];
        v21 = v17[2];
        if ( v20 > v21 || v20 == v21 && v19[1] >= v17[1] )
          goto LABEL_95;
        if ( *v19 != 36 && *v19 != 35 )
          v18 = (double *)*((_DWORD *)v18 + 6);
        if ( !v18 )
          break;
        v19 = v17;
        v17 = (int *)v17[4];
        if ( !v17 )
          goto LABEL_95;
      }
      SourceError(a1, "mising values in #if/#elif");
      v50 = 1;
      goto LABEL_164;
    }
LABEL_95:
    v22 = *((_DWORD *)v18 + 6);
    switch ( *v19 )
    {
      case 5:
        v26 = *(_DWORD *)v18 && *(_DWORD *)v22;
        v27 = v18[1];
        *(_DWORD *)v18 = v26;
        if ( (v27 == 0.0) | __UNORDERED__(v27, 0.0)
          || (v48 = 1, (*(double *)(v22 + 8) == 0.0) | __UNORDERED__(*(double *)(v22 + 8), 0.0)) )
        {
          v48 = 0;
        }
        v18[1] = (double)v48;
        goto LABEL_142;
      case 6:
        v28 = *(_DWORD *)v18 || *(_DWORD *)v22;
        v29 = v18[1];
        *(_DWORD *)v18 = v28;
        if ( !((v29 == 0.0) | __UNORDERED__(v29, 0.0))
          || (v49 = 0, !((*(double *)(v22 + 8) == 0.0) | __UNORDERED__(*(double *)(v22 + 8), 0.0))) )
        {
          v49 = 1;
        }
        v18[1] = (double)v49;
        goto LABEL_142;
      case 7:
        v30 = v18[1];
        *(_DWORD *)v18 = *(_DWORD *)v18 >= *(_DWORD *)v22;
        v18[1] = (double)(v30 >= *(double *)(v22 + 8));
        goto LABEL_142;
      case 8:
        v31 = v18[1];
        *(_DWORD *)v18 = *(_DWORD *)v18 <= *(_DWORD *)v22;
        v18[1] = (double)(v31 <= *(double *)(v22 + 8));
        goto LABEL_142;
      case 9:
        v32 = v18[1];
        *(_DWORD *)v18 = *(_DWORD *)v22 == *(_DWORD *)v18;
        v18[1] = (double)(((v32 == *(double *)(v22 + 8)) | __UNORDERED__(v32, *(double *)(v22 + 8))) != 0);
        goto LABEL_142;
      case 10:
        v33 = v18[1];
        *(_DWORD *)v18 = *(_DWORD *)v18 != *(_DWORD *)v22;
        v18[1] = (double)(((v33 == *(double *)(v22 + 8)) | __UNORDERED__(v33, *(double *)(v22 + 8))) == 0);
        goto LABEL_142;
      case 21:
        *(int *)v18 >>= *(_DWORD *)v22;
        goto LABEL_142;
      case 22:
        *(_DWORD *)v18 <<= *(_DWORD *)v22;
        goto LABEL_142;
      case 26:
        *(_DWORD *)v18 *= *(_DWORD *)v22;
        v18[1] = *(double *)(v22 + 8) * v18[1];
        goto LABEL_142;
      case 27:
        if ( !*(_DWORD *)v22 || (*(double *)(v22 + 8) == 0.0) | __UNORDERED__(*(double *)(v22 + 8), 0.0) )
        {
          v46 = "divide by zero in #if/#elif\n";
          goto LABEL_161;
        }
        v24 = v18[1];
        *(int *)v18 /= *(int *)v22;
        v18[1] = v24 / *(double *)(v22 + 8);
        goto LABEL_142;
      case 28:
        if ( !*(_DWORD *)v22 )
        {
          v46 = "divide by zero in #if/#elif\n";
          v45 = a1;
          goto LABEL_162;
        }
        *(int *)v18 %= *(_DWORD *)v22;
        goto LABEL_142;
      case 29:
        *(_DWORD *)v18 += *(_DWORD *)v22;
        v18[1] = *(double *)(v22 + 8) + v18[1];
        goto LABEL_142;
      case 30:
        v25 = v18[1];
        *(_DWORD *)v18 -= *(_DWORD *)v22;
        v18[1] = v25 - *(double *)(v22 + 8);
        goto LABEL_142;
      case 32:
        *(_DWORD *)v18 &= *(_DWORD *)v22;
        goto LABEL_142;
      case 33:
        *(_DWORD *)v18 |= *(_DWORD *)v22;
        goto LABEL_142;
      case 34:
        *(_DWORD *)v18 ^= *(_DWORD *)v22;
        goto LABEL_142;
      case 35:
        *(_DWORD *)v18 = ~*(_DWORD *)v18;
        goto LABEL_142;
      case 36:
        v23 = v18[1];
        *(_DWORD *)v18 = *(_DWORD *)v18 == 0;
        v18[1] = (double)(((v23 == 0.0) | __UNORDERED__(v23, 0.0)) != 0);
        goto LABEL_142;
      case 37:
        v34 = v18[1];
        *(_DWORD *)v18 = *(_DWORD *)v18 > *(_DWORD *)v22;
        v18[1] = (double)(v34 > *(double *)(v22 + 8));
        goto LABEL_142;
      case 38:
        v35 = v18[1];
        *(_DWORD *)v18 = *(_DWORD *)v18 < *(_DWORD *)v22;
        v18[1] = (double)(((v35 < *(double *)(v22 + 8)) | __UNORDERED__(v35, *(double *)(v22 + 8))) != 0);
        goto LABEL_142;
      case 42:
        if ( !v52 )
        {
          v46 = ": without ? in #if/#elif";
          v45 = a1;
          goto LABEL_162;
        }
        if ( a4 )
        {
          if ( !v59 )
          {
            *(_DWORD *)v18 = *(_DWORD *)v22;
            v52 = 0;
            goto LABEL_142;
          }
        }
        else if ( (v62 == 0.0) | __UNORDERED__(v62, 0.0) )
        {
          *((_DWORD *)v18 + 2) = *(_DWORD *)(v22 + 8);
          *((_DWORD *)v18 + 3) = *(_DWORD *)(v22 + 12);
        }
        v52 = 0;
        goto LABEL_142;
      case 43:
        if ( !v52 )
        {
          v59 = *(_DWORD *)v18;
          v62 = v18[1];
          v52 = 1;
LABEL_142:
          v36 = *v19;
          if ( *v19 != 36 && v36 != 35 )
          {
            if ( v36 != 43 )
              v18 = (double *)v22;
            v37 = *((_DWORD *)v18 + 5);
            if ( v37 )
              *(_DWORD *)(v37 + 24) = *((_DWORD *)v18 + 6);
            else
              v57 = (double *)*((_DWORD *)v18 + 6);
            v38 = *((_DWORD *)v18 + 6);
            if ( v38 )
              *(_DWORD *)(v38 + 20) = *((_DWORD *)v18 + 5);
          }
          v39 = v19[3];
          if ( v39 )
            *(_DWORD *)(v39 + 16) = v19[4];
          else
            v53 = (_DWORD *)v19[4];
          v40 = v19[4];
          if ( v40 )
            *(_DWORD *)(v40 + 12) = v19[3];
          continue;
        }
        v46 = "? after ? in #if/#elif";
LABEL_161:
        v45 = a1;
LABEL_162:
        SourceError(v45, v46);
        v50 = 1;
LABEL_163:
        v16 = v53;
LABEL_164:
        if ( v57 )
        {
          if ( a2 )
            *a2 = *(_DWORD *)v57;
          v41 = a3;
          if ( a3 )
          {
            *a3 = *((_DWORD *)v57 + 2);
            a3[1] = *((_DWORD *)v57 + 3);
          }
        }
        else
        {
          v41 = a3;
        }
        for ( i = v16; i; i = (_DWORD *)i[4] )
          ;
        for ( j = v57; j; j = (double *)*((_DWORD *)j + 6) )
          ;
        if ( !v50 )
          return 1;
        if ( a2 )
          *a2 = 0;
        if ( v41 )
        {
          *v41 = 0;
          v41[1] = 0;
        }
        return 0;
      default:
        goto LABEL_142;
    }
  }
}

/* 0x00440628-0x0044064F is NOT A FUNCTION: it is the JUMP TABLE for the
 * operator switch in PC_EvaluateTokens.  Its only cross-reference is a DATA
 * reference from 0x004400F5, inside that switch.  (0x00442784 in
 * l_script_mp.c is PS_ReadEscapeCharacter's jump table, likewise.) */

/* ---- PC_Evaluate  0x00440650 ---- */
// PC_Evaluate | CONFIRMED | RTCW l_precomp.c:2128
int __cdecl PC_Evaluate(int a1, _DWORD *a2, _DWORD *a3, int a4)
{
  int v4;
  _DWORD *v6;
  _DWORD *v7;
  int HashedDefine;
  int *v9;
  int *v10;
  int v11;
  int v12;
  int v13;
  int v14;
  void *v15;
  char *v16;
  char *LocalizedString_m;
  char v18[4];
  int v19;
  char ArgList[1072]; // [esp+10h] [ebp-434h] BYREF
  unsigned int v21;
  unsigned int retaddr;

  v21 = retaddr ^ _security_cookie;
  v19 = 0;
  if ( a2 )
    *a2 = 0;
  if ( a3 )
  {
    *a3 = 0;
    a3[1] = 0;
  }
  v4 = a1;
  if ( !PC_ReadLine(a1, ArgList) )
  {
    SourceError(a1, "no value after #if/#elif");
    return 0;
  }
  *(_DWORD *)v18 = 0;
  v6 = 0;
  do
  {
    if ( *(_DWORD *)&ArgList[1024] != 4 )
    {
      if ( *(_DWORD *)&ArgList[1024] != 3 && *(_DWORD *)&ArgList[1024] != 5 )
      {
        SourceError(v4, "can't evaluate %s", ArgList);
        return 0;
      }
      v9 = (int *)malloc(0x434u);
      v10 = v9;
      if ( !v9 )
      {
        Sys_OutOfMemoryPrep();
        LocalizedString_m = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_TITLE");
        v16 = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_BODY");
        MessageBoxA(0, v16, LocalizedString_m, 0x10u);
        exit(-1);
      }
      Com_Memset(v9, 0, 0x434u);
      v7 = v10 + 1;
      *v10 = 305419896;
      if ( v10 == (int *)-4 )
        Com_Error(ERR_FATAL, "EXE_ERR_OUT_OF_MEMORY");
      qmemcpy(v7, ArgList, 0x430u);
      v11 = numtokens + 1;
      v10[267] = 0;
      numtokens = v11;
      goto LABEL_13;
    }
    if ( v19 )
    {
      v19 = 0;
      v7 = PC_CopyToken(ArgList);
LABEL_13:
      v7[266] = 0;
      if ( v6 )
        v6[266] = v7;
      else
        *(_DWORD *)v18 = v7;
      v6 = v7;
      goto LABEL_26;
    }
    if ( !strcmp(ArgList, "defined") )
    {
      v19 = 1;
      v7 = PC_CopyToken(ArgList);
      goto LABEL_13;
    }
    HashedDefine = PC_FindHashedDefine(*(_DWORD *)(a1 + 536), ArgList);
    if ( !HashedDefine )
    {
      SourceError(a1, "can't evaluate %s, not defined", ArgList);
      return 0;
    }
    if ( !PC_ExpandDefineIntoSource(HashedDefine, a1, ArgList) )
      return 0;
LABEL_26:
    v4 = a1;
  }
  while ( PC_ReadLine(a1, ArgList) );
  /* tokens is the ECX argument in retail; it is the list this function frees below. */
  if ( !PC_EvaluateTokens(a1, *(_DWORD **)v18, a2, a3, a4) )
    return 0;
  v12 = *(_DWORD *)v18;
  if ( *(_DWORD *)v18 )
  {
    do
    {
      v13 = *(_DWORD *)(v12 + 1064);
      v14 = *(_DWORD *)(v12 - 4);
      v15 = (void *)(v12 - 4);
      if ( v14 == 305419896 )
        free(v15);
      --numtokens;
      v12 = v13;
    }
    while ( v13 );
  }
  return 1;
}

/* ---- PC_DollarEvaluate  0x00440900 ---- */
// PC_DollarEvaluate | CONFIRMED | RTCW l_precomp.c:2232
/* Retail: source@<ecx> (`mov ebx, ecx` at 0x00440926) and the three stack
 * arguments at arg_0/arg_4/arg_8.  Spelled as an explicit leading parameter. */
int __cdecl PC_DollarEvaluate(int source, _DWORD *a1, _DWORD *a2, int a3)
{
  int v3 = source;
  int v4;
  _DWORD *v6;
  _DWORD *v7;
  int HashedDefine;
  int v9;
  int v10;
  int v11;
  int v12;
  void *v13;
  int v14;
  char v15[4];
  int v16;
  /* SPLIT-RECORD: one token_t at [ebp-434h] that PC_ReadSourceToken and
     PC_ExpandDefineIntoSource both fill completely. */
  token_t tok;                          /* [esp+14h] [ebp-434h] BYREF */
  char *ArgList = tok.string;
  unsigned int v19;
  unsigned int retaddr;

  v19 = retaddr ^ _security_cookie;
  v4 = v3;
  v16 = 0;
  if ( a1 )
    *a1 = 0;
  if ( a2 )
  {
    *a2 = 0;
    a2[1] = 0;
  }
  if ( !PC_ReadSourceToken(ArgList, v3) )
  {
    SourceError(v4, "no leading ( after $evalint/$evalfloat");
    return 0;
  }
  if ( !PC_ReadSourceToken(ArgList, v4) )
  {
    SourceError(v4, "nothing to evaluate");
    return 0;
  }
  v14 = 1;
  *(_DWORD *)v15 = 0;
  v6 = 0;
  do
  {
    if ( tok.type != TT_NAME )
    {
      if ( tok.type != TT_NUMBER && tok.type != TT_PUNCTUATION )
      {
        SourceError(v4, "can't evaluate %s", ArgList);
        return 0;
      }
      if ( ArgList[0] == '(' )
      {
        v9 = v14 + 1;
      }
      else
      {
        if ( ArgList[0] != ')' )
          goto LABEL_29;
        v9 = v14 - 1;
      }
      v14 = v9;
LABEL_29:
      if ( v14 <= 0 )
        break;
      goto LABEL_13;
    }
    if ( v16 )
    {
      v16 = 0;
LABEL_13:
      v7 = PC_CopyToken(ArgList);
      v7[266] = 0;
      if ( v6 )
        v6[266] = v7;
      else
        *(_DWORD *)v15 = v7;
LABEL_32:
      v6 = v7;
      continue;
    }
    if ( !strcmp(ArgList, "defined") )
    {
      v16 = 1;
      v7 = PC_CopyToken(ArgList);
      v7[266] = 0;
      if ( v6 )
        v6[266] = v7;
      else
        *(_DWORD *)v15 = v7;
      goto LABEL_32;
    }
    HashedDefine = PC_FindHashedDefine(*(_DWORD *)(v4 + 536), ArgList);
    if ( !HashedDefine )
    {
      SourceError(v4, "can't evaluate %s, not defined", ArgList);
      return 0;
    }
    if ( !PC_ExpandDefineIntoSource(HashedDefine, v4, ArgList) )
      return 0;
  }
  while ( PC_ReadSourceToken(ArgList, v4) );
  /* tokens is the ECX argument in retail; it is the list this function frees below. */
  if ( !PC_EvaluateTokens(v4, *(_DWORD **)v15, a1, a2, a3) )
    return 0;
  v10 = *(_DWORD *)v15;
  if ( *(_DWORD *)v15 )
  {
    do
    {
      v11 = *(_DWORD *)(v10 + 1064);
      v12 = *(_DWORD *)(v10 - 4);
      v13 = (void *)(v10 - 4);
      if ( v12 == 305419896 )
        free(v13);
      --numtokens;
      v10 = v11;
    }
    while ( v11 );
  }
  return 1;
}

/* ---- PC_Directive_elif  0x00440B70 ---- */
// PC_Directive_elif | CONFIRMED | RTCW
int __cdecl PC_Directive_elif(_DWORD *a1)
{
  int v2; // [esp+4h] [ebp-8h] BYREF
  int v3; // [esp+8h] [ebp-4h] BYREF

  PC_PopIndent(&v2, &v3, a1);
  if ( !v2 || v2 == 2 )
  {
    SourceError((int)a1, "misplaced #elif");
  }
  else if ( PC_Evaluate((int)a1, &v2, 0, 1) )
  {
    PC_PushIndent(a1, 4, v2 == 0);
    return 1;
  }
  return 0;
}

/* ---- PC_Directive_if  0x00440BE0 ---- */
// PC_Directive_if | CONFIRMED | PC_Evaluate(source, &value, NULL, qtrue) then PC_PushIndent(source, INDENT_IF=1, value == 0).
int __cdecl PC_Directive_if(_DWORD *a1)
{
  int result;
  int v2; // [esp+4h] [ebp-4h] BYREF

  result = PC_Evaluate((int)a1, &v2, 0, 1);
  if ( result )
  {
    PC_PushIndent(a1, 1, v2 == 0);
    return 1;
  }
  return result;
}

/* ---- PC_Directive_line  0x00440C20 ---- */
// PC_Directive_line | CONFIRMED | RTCW
int __cdecl PC_Directive_line(int a1)
{
  SourceError(a1, "#line directive not supported");
  return 0;
}

/* ---- PC_Directive_error  0x00440C40 ---- */
// PC_Directive_error | CONFIRMED | RTCW
int __cdecl PC_Directive_error(int a1)
{
  char ArgList[1072]; // [esp+4h] [ebp-434h] BYREF
  unsigned int v3;
  unsigned int retaddr;

  v3 = retaddr ^ _security_cookie;
  ArgList[0] = 0;
  PC_ReadSourceToken(ArgList, a1);
  SourceError(a1, "#error directive: %s", ArgList);
  return 0;
}

/* ---- PC_Directive_pragma  0x00440CA0 ---- */
// PC_Directive_pragma | CONFIRMED | RTCW
int __cdecl PC_Directive_pragma(int a1)
{
  _DWORD v2[269]; // [esp+4h] [ebp-434h] BYREF
  unsigned int retaddr;

  v2[268] = retaddr ^ _security_cookie;
  SourceWarning(a1, "#pragma directive not supported");
  while ( PC_ReadLine(a1, v2) )
    ;
  return 1;
}

/* ---- PC_UnreadSignToken  0x00440D10 ---- */
// PC_UnreadSignToken | CONFIRMED | Builds a '-' punctuation token on a stack token_t and unreads it.
_DWORD *__cdecl PC_UnreadSignToken(int a1)
{
  int v1;
  int v2;
  _DWORD *result;
  __int16 token[512]; // [esp+0h] [ebp-434h] BYREF
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  unsigned int v11;
  unsigned int retaddr;

  v11 = retaddr ^ _security_cookie;
  v1 = *(_DWORD *)(a1 + 524);
  v2 = *(_DWORD *)(v1 + 288);
  v7 = *(_DWORD *)(v1 + 264);
  v8 = v7;
  v9 = v2;
  v10 = 0;
  token[0] = 45;
  v5 = 5;
  v6 = 30;
  result = PC_CopyToken(token);
  result[266] = *(_DWORD *)(a1 + 528);
  *(_DWORD *)(a1 + 528) = result;
  return result;
}

/* ---- PC_Directive_eval  0x00440DC0 ---- */
// PC_Directive_eval | CONFIRMED | Evaluates and pushes the integer result back as a "%d" token.
int __cdecl PC_Directive_eval(int a1)
{
  int result;
  int v2;
  int v3;
  _DWORD *v4;
  int v5; // [esp+4h] [ebp-438h] BYREF
  /* SPLIT-RECORD: one token_t built at [ebp-434h] before handing it to
     PC_CopyToken, which memcpys all 1072 bytes of it onto the heap. */
  token_t tok;                          /* [esp+8h] [ebp-434h] BYREF */
  char *Buffer = tok.string;
  unsigned int v13;
  unsigned int retaddr;

  v13 = retaddr ^ _security_cookie;
  result = PC_Evaluate(a1, &v5, 0, 1);
  if ( result )
  {
    v2 = *(_DWORD *)(a1 + 524);
    tok.line = *(_DWORD *)(v2 + 288);
    tok.whitespace_p = *(char **)(v2 + 264);
    v3 = v5;
    tok.endwhitespace_p = *(char **)(v2 + 264);
    tok.linescrossed = 0;
    sprintf(Buffer, "%d", abs32(v5));
    tok.type = TT_NUMBER;
    tok.subtype = 12296;
    v4 = PC_CopyToken(Buffer);
    v4[266] = *(_DWORD *)(a1 + 528);
    *(_DWORD *)(a1 + 528) = v4;
    if ( v3 < 0 )
      PC_UnreadSignToken(a1);
    return 1;
  }
  return result;
}

/* ---- PC_Directive_evalfloat  0x00440ED0 ---- */
// PC_Directive_evalfloat | CONFIRMED | RTCW l_precomp.c:2466; reached from PC_ReadDirective 0x00440FE0
int __cdecl PC_Directive_evalfloat(int a1)
{
  int result;
  int v2;
  _DWORD *v3;
  bool v4; // c0
  bool v5; // c2
  double v6; // [esp+Ch] [ebp-43Ch] BYREF
  /* SPLIT-RECORD -- see PC_Directive_eval. */
  token_t tok;                          /* [esp+14h] [ebp-434h] BYREF */
  char *Buffer = tok.string;
  unsigned int v14;
  unsigned int retaddr;

  v14 = retaddr ^ _security_cookie;
  result = PC_Evaluate(a1, 0, &v6, 0);
  if ( result )
  {
    v2 = *(_DWORD *)(a1 + 524);
    tok.line = *(_DWORD *)(v2 + 288);
    tok.whitespace_p = *(char **)(v2 + 264);
    tok.endwhitespace_p = *(char **)(v2 + 264);
    tok.linescrossed = 0;
    sprintf(Buffer, "%1.2f", (double)fabs(v6));
    tok.type = TT_NUMBER;
    tok.subtype = 10248;
    v3 = PC_CopyToken(Buffer);
    v4 = v6 < 0.0;
    v5 = __UNORDERED__(v6, 0.0);
    v3[266] = *(_DWORD *)(a1 + 528);
    *(_DWORD *)(a1 + 528) = v3;
    if ( v4 || v5 )
      PC_UnreadSignToken(a1);
    return 1;
  }
  return result;
}

/* PC_ReadDirective (0x00440FE0) is in botlib/l_precomp_pc.c, together with
 * the 14-entry `directives[]` table at 0x0057AAA0.  Declared in l_script.h. */

/* ---- PC_DollarDirective_evalint  0x00441100 ---- */
// PC_DollarDirective_evalint | CONFIRMED | $evalint twin of PC_Directive_eval; pairs with PC_DollarDirective_evalfloat at 0x441220.
int __cdecl PC_DollarDirective_evalint(int a1)
{
  int result;
  int v2;
  int v3;
  _DWORD *v4;
  int v5; // [esp+4h] [ebp-438h] BYREF
  /* SPLIT-RECORD -- see PC_Directive_eval.  Writes type, subtype, intvalue,
     floatvalue (the 8-byte double at +1040), whitespace_p, endwhitespace_p,
     line and linescrossed. */
  token_t tok;                          /* [esp+8h] [ebp-434h] BYREF */
  char *Buffer = tok.string;
  unsigned int v15;
  unsigned int retaddr;

  v15 = retaddr ^ _security_cookie;
  result = PC_DollarEvaluate(a1, &v5, 0, 1);   /* a1 = source, was passed in ECX */
  if ( result )
  {
    v2 = *(_DWORD *)(a1 + 524);
    tok.line = *(_DWORD *)(v2 + 288);
    tok.whitespace_p = *(char **)(v2 + 264);
    v3 = v5;
    tok.endwhitespace_p = *(char **)(v2 + 264);
    tok.linescrossed = 0;
    sprintf(Buffer, "%d", abs32(v5));
    tok.type = TT_NUMBER;
    tok.floatvalue = (double)v5;
    tok.subtype = 12296;
    tok.intvalue = (unsigned long)v3;
    v4 = PC_CopyToken(Buffer);
    v4[266] = *(_DWORD *)(a1 + 528);
    *(_DWORD *)(a1 + 528) = v4;
    if ( v3 < 0 )
      PC_UnreadSignToken(a1);
    return 1;
  }
  return result;
}

/* ---- PC_DollarDirective_evalfloat  0x00441220 ---- */
// PC_DollarDirective_evalfloat | CONFIRMED | RTCW l_precomp.c:2576; reached from PC_ReadDollarDirective 0x00441350
int __cdecl PC_DollarDirective_evalfloat(int a1)
{
  int result;
  int v2;
  _DWORD *v3;
  bool v4; // c0
  bool v5; // c2
  int v6[2]; // [esp+Ch] [ebp-43Ch] BYREF
  /* SPLIT-RECORD -- see PC_Directive_eval. */
  token_t tok;                          /* [esp+14h] [ebp-434h] BYREF */
  char *Buffer = tok.string;
  unsigned int v16;
  unsigned int retaddr;

  v16 = retaddr ^ _security_cookie;
  result = PC_DollarEvaluate(a1, 0, v6, 0);    /* a1 = source, was passed in ECX */
  if ( result )
  {
    v2 = *(_DWORD *)(a1 + 524);
    tok.line = *(_DWORD *)(v2 + 288);
    tok.whitespace_p = *(char **)(v2 + 264);
    tok.endwhitespace_p = *(char **)(v2 + 264);
    tok.linescrossed = 0;
    sprintf(Buffer, "%1.2f", (double)fabs(*(long double *)v6));
    tok.type = TT_NUMBER;
    tok.subtype = 10248;
    tok.floatvalue = *(double *)v6;
    tok.intvalue = (unsigned long)*(double *)v6;
    v3 = PC_CopyToken(Buffer);
    v4 = *(double *)v6 < 0.0;
    v5 = __UNORDERED__(*(double *)v6, 0.0);
    v3[266] = *(_DWORD *)(a1 + 528);
    *(_DWORD *)(a1 + 528) = v3;
    if ( v4 || v5 )
      PC_UnreadSignToken(a1);
    return 1;
  }
  return result;
}

/* PC_ReadDollarDirective (0x00441350) is in botlib/l_precomp_pc.c, with the
 * 2-entry `dollardirectives[]` table at 0x0057AB40.  Declared in l_script.h. */

/* ---- PC_ReadToken  0x00441490 ---- */
// PC_ReadToken | CONFIRMED | RTCW
int __cdecl PC_ReadToken(_DWORD *a1, _DWORD *a2)
{
  int v2;
  int Directive;
  _DWORD *v4;
  int HashedDefine;
  _DWORD token[269]; // [esp+10h] [ebp-434h] BYREF
  unsigned int retaddr;

  token[268] = retaddr ^ _security_cookie;
  if ( !PC_ReadSourceToken(a2, (int)a1) )
    return 0;
  while ( 1 )
  {
    v2 = a2[256];
    if ( v2 == 5 )
    {
      if ( *(_BYTE *)a2 == 35 )
      {
        Directive = PC_ReadDirective(a1);
        goto LABEL_17;
      }
      if ( *(_BYTE *)a2 == 36 )
      {
        Directive = PC_ReadDollarDirective(a1);
        goto LABEL_17;
      }
    }
    if ( v2 == 1 && PC_ReadToken(a1, token) )
    {
      if ( token[256] == 1 )
      {
        *((_BYTE *)a2 + strlen((const char *)a2) - 1) = 0;
        if ( strlen((const char *)token + 1) + strlen((const char *)a2) + 1 >= 0x400 )
        {
          SourceError((int)a1, "string longer than MAX_TOKEN %d\n", 1024);
          return 0;
        }
        strcat((char *)a2, (const char *)token + 1);
      }
      else
      {
        v4 = PC_CopyToken(token);
        v4[266] = a1[132];
        a1[132] = v4;
      }
    }
    if ( !a1[136] )
      break;
LABEL_18:
    if ( !PC_ReadSourceToken(a2, (int)a1) )
      return 0;
  }
  if ( a2[256] == 4 )
  {
    HashedDefine = PC_FindHashedDefine(a1[134], (char *)a2);
    if ( HashedDefine )
    {
      Directive = PC_ExpandDefineIntoSource(HashedDefine, (int)a1, a2);
LABEL_17:
      if ( !Directive )
        return 0;
      goto LABEL_18;
    }
  }
  qmemcpy(a1 + 138, a2, 0x430u);
  return 1;
}

/* ---- PC_ExpectTokenString  0x00441650 ---- */
// PC_ExpectTokenString | CONFIRMED | RTCW
int __cdecl PC_ExpectTokenString(const char *a1, _DWORD *a2)
{
  _DWORD v3[269]; // [esp+4h] [ebp-434h] BYREF
  unsigned int retaddr;

  v3[268] = retaddr ^ _security_cookie;
  if ( PC_ReadToken(a2, v3) )
  {
    if ( !strcmp((const char *)v3, a1) )
    {
      return 1;
    }
    else
    {
      SourceError((int)a2, "expected %s, found %s", a1, (const char *)v3);
      return 0;
    }
  }
  else
  {
    SourceError((int)a2, "couldn't find expected %s", a1);
    return 0;
  }
}

/* ---- PC_ExpectTokenType  AUTO-STUBBED ----
 * Unreachable in the RETAIL binary, not merely on a dedicated server: no
 * xrefs at all (0x00441740).  An unverified body is kept in the #if 0 block
 * below.
 */
int PC_ExpectTokenType() { return 0; }
#if 0
// PC_ExpectTokenType | CONFIRMED 95 | anchors: string:"couldn't read expected token"; tierC:RTCW l_precomp.c:2797 | module: botlib/l_precomp_mp.c
int __cdecl PC_ExpectTokenType(int a1, int a2, _DWORD *a3, char *a4)
{
  int v5;
  char *v6;
  char *v8;
  char *v10;
  char *v12;
  char v14; // [esp+7h] [ebp-405h] BYREF
  char ArgList[8]; // [esp+8h] [ebp-404h] BYREF
  void *v16;
  unsigned int v17;
  unsigned int retaddr;

  v17 = retaddr ^ _security_cookie;
  if ( !PC_ReadToken(a3, a4) )
  {
    SourceError((int)a3, "couldn't read expected token");
    return 0;
  }
  v5 = *((_DWORD *)a4 + 256);
  if ( v5 != a2 )
  {
    ArgList[0] = 0;
    switch ( a2 )
    {
      case 1:
        strcpy(ArgList, "string");
        break;
      case 2:
        *(_DWORD *)ArgList = 1702127980;
        *(_DWORD *)&ArgList[4] = &unk_6C6172;
        break;
      case 3:
        strcpy(ArgList, "number");
        break;
      case 4:
        strcpy(ArgList, "name");
        break;
      case 5:
        qmemcpy(ArgList, "punctuat", sizeof(ArgList));
        v16 = &unk_6E6F69;
        break;
    }
    SourceError((int)a3, "expected a %s, found %s", ArgList, a4);
    return 0;
  }
  if ( v5 != 3 )
  {
    if ( v5 == 5 && *((_DWORD *)a4 + 257) != a1 )
    {
      SourceError((int)a3, "found %s", a4);
      return 0;
    }
    return 1;
  }
  if ( (a1 & *((_DWORD *)a4 + 257)) == a1 )
    return 1;
  if ( (a1 & 8) != 0 )
  {
    *(_DWORD *)ArgList = 1768121700;
    *(_DWORD *)&ArgList[4] = &unk_6C616D;
  }
  if ( (a1 & 0x100) != 0 )
    *(_DWORD *)ArgList = &unk_786568;
  if ( (a1 & 0x200) != 0 )
    strcpy(ArgList, "octal");
  if ( (a1 & 0x400) != 0 )
    strcpy(ArgList, "binary");
  if ( (a1 & 0x2000) != 0 )
  {
    v6 = &v14;
    while ( *++v6 )
      ;
    strcpy(v6, " long");
  }
  if ( (a1 & 0x4000) != 0 )
  {
    v8 = &v14;
    while ( *++v8 )
      ;
    strcpy(v8, " unsigned");
  }
  if ( (a1 & 0x800) != 0 )
  {
    v10 = &v14;
    while ( *++v10 )
      ;
    strcpy(v10, " float");
  }
  if ( (a1 & 0x1000) != 0 )
  {
    v12 = &v14;
    while ( *++v12 )
      ;
    strcpy(v12, " integer");
  }
  SourceError((int)a3, "expected %s, found %s", ArgList, a4);
  return 0;
}
#endif

/* ---- PC_ExpectAnyToken  0x004419F0 ---- */
// PC_ExpectAnyToken | HIGH | RTCW PC_ExpectAnyToken; 37-byte thin wrapper sharing its error string with PC_ExpectTokenType.
int __cdecl PC_ExpectAnyToken(_DWORD *a1, _DWORD *a2)
{
  if ( PC_ReadToken(a2, a1) )
    return 1;
  SourceError((int)a2, "couldn't read expected token");
  return 0;
}

/* ---- PC_CheckTokenString  0x00441A20 ---- */
// PC_CheckTokenString | CONFIRMED | Reads a token, returns 1 on match, otherwise pushes a copy back onto source->tokens (+528).
int __cdecl PC_CheckTokenString(_DWORD *a1, const char *a2)
{
  _DWORD *v3;
  _DWORD token[269]; // [esp+0h] [ebp-434h] BYREF
  unsigned int retaddr;

  token[268] = retaddr ^ _security_cookie;
  if ( PC_ReadToken(a1, token) )
  {
    if ( !strcmp((const char *)token, a2) )
      return 1;
    v3 = PC_CopyToken(token);
    v3[266] = a1[132];
    a1[132] = v3;
  }
  return 0;
}

/* ---- PC_CheckTokenType  0x00441AF0 ---- */
// PC_CheckTokenType | CONFIRMED | Same shape as PC_CheckTokenString but matches token->type (+1024) and masks token->subtype (+1028); copies the whole 0x430-byte token out on success.
int __fastcall PC_CheckTokenType(int a1, _DWORD *a2, int a3, void *a4)
{
  _DWORD *v7;
  _DWORD token[269]; // [esp+8h] [ebp-434h] BYREF
  unsigned int retaddr;

  token[268] = retaddr ^ _security_cookie;
  if ( PC_ReadToken(a2, token) )
  {
    if ( token[256] == a3 && (a1 & token[257]) == a1 )
    {
      qmemcpy(a4, token, 0x430u);
      return 1;
    }
    v7 = PC_CopyToken(token);
    v7[266] = a2[132];
    a2[132] = v7;
  }
  return 0;
}

/* ---- PC_SkipUntilString  0x00441BA0 ---- */
// PC_SkipUntilString | CONFIRMED | Reads tokens until strcmp matches or the source runs dry.
int __cdecl PC_SkipUntilString(const char *a1, _DWORD *a2)
{
  _DWORD v3[269]; // [esp+Ch] [ebp-434h] BYREF
  unsigned int retaddr;

  v3[268] = retaddr ^ _security_cookie;
  if ( !PC_ReadToken(a2, v3) )
    return 0;
  while ( strcmp((const char *)v3, a1) )
  {
    if ( !PC_ReadToken(a2, v3) )
      return 0;
  }
  return 1;
}

/* ---- PC_UnreadLastToken  0x00441C50 ---- */
// PC_UnreadLastToken | CONFIRMED | PC_CopyToken(&source->token at +552) pushed onto source->tokens (+528).
_DWORD *__cdecl PC_UnreadLastToken(int a1)
{
  _DWORD *result;

  result = PC_CopyToken((const void *)(a1 + 552));
  result[266] = *(_DWORD *)(a1 + 528);
  *(_DWORD *)(a1 + 528) = result;
  return result;
}

/* ---- PC_UnreadToken  0x00441C80 ---- */
// PC_UnreadToken | CONFIRMED | Same as above for an arbitrary token.
_DWORD *__cdecl PC_UnreadToken(const void *a1, int a2)
{
  _DWORD *result;

  result = PC_CopyToken(a1);
  result[266] = *(_DWORD *)(a2 + 528);
  *(_DWORD *)(a2 + 528) = result;
  return result;
}

/* ---- PC_SetIncludePath  0x00441CA0 ---- */
// PC_SetIncludePath | CONFIRMED | strncpy into source->includepath (+260, MAX_PATH 0x104) and appends a separator when one is missing.
char __cdecl PC_SetIncludePath(const char *a1, int a2)
{
  unsigned int v2;
  char *v3;

  strncpy((char *)(a2 + 260), a1, 0x104u);
  v2 = strlen((const char *)(a2 + 260));
  if ( *(_BYTE *)(v2 + a2 + 259) != 92 )
  {
    v2 = strlen((const char *)(a2 + 260));
    if ( *(_BYTE *)(v2 + a2 + 259) != 47 )
    {
      v3 = (char *)(a2 + 259);
      do
        LOBYTE(v2) = *++v3;
      while ( (_BYTE)v2 );
      strcpy(v3, "\\");
    }
  }
  return v2;
}

/* ---- PC_SetPunctuations  0x00441D10 ---- */
// PC_SetPunctuations | CONFIRMED | Single store into source->punctuations (+520).
int __cdecl PC_SetPunctuations(int result, int a2)
{
  *(_DWORD *)(a2 + 520) = result;
  return result;
}

/* LoadSourceFile (0x00441D20) is in botlib/l_precomp_pc.c.  One argument. */

/* ---- LoadSourceMemory  0x00441DE0 ---- */
// LoadSourceMemory | CONFIRMED | LoadScriptMemory twin of LoadSourceFile.
/* RTCW l_precomp.c:2952, VERBATIM.  DEAD in retail -- zero xrefs.
 *
 * Retail is __usercall( ptr@<ecx>, length on stack arg_0, name on stack
 * arg_4 ) -- RTCW's ( ptr, length, name ) with only the first argument lifted
 * into a register.  Source order here. */
_DWORD *__cdecl LoadSourceMemory(char *ptr, int length, char *Source)
{
  _DWORD *ScriptMemory;
  _DWORD *v4;
  void *v5;
  int *v7;
  _DWORD *v8;
  int *v9;

  ScriptMemory = (_DWORD *)LoadScriptMemory(ptr, length, Source);
  v4 = ScriptMemory;
  v5 = 0;
  if ( !ScriptMemory )
    return 0;
  ScriptMemory[346] = 0;
  v7 = Z_MallocInternal(0x65Cu);
  if ( v7 )
  {
    *v7 = 305419896;
    v8 = v7 + 1;
  }
  else
  {
    v8 = 0;
  }
  memset(v8, 0, 0x658u);
  strncpy((char *)v8, Source, 0x104u);
  v8[131] = v4;
  v8[132] = 0;
  v8[133] = 0;
  v8[135] = 0;
  v8[136] = 0;
  v9 = Z_MallocInternal(0x1004u);
  if ( v9 )
  {
    *v9 = 305419896;
    v5 = v9 + 1;
  }
  memset(v5, 0, 0x1000u);
  v8[134] = v5;
  PC_AddGlobalDefinesToSource(v8);
  return v8;
}

/* ---- FreeSource  0x00441EA0 ---- */
// FreeSource | CONFIRMED | Frees scripts, tokens, defines and indents; called by PC_FreeSourceHandle.
void __cdecl FreeSource(_DWORD *a1)
{
  int v1;
  int v2;
  int v3;
  void *v4;
  int v5;
  int v6;
  _DWORD *v7;
  int v8;
  int i;
  int v10;
  int v11;
  int v12;
  int v13;
  _DWORD *v14;
  int v15;
  int v16;
  void *v17;

  while ( a1[131] )
  {
    v1 = a1[131];
    a1[131] = *(_DWORD *)(v1 + 1384);
    v2 = *(_DWORD *)(v1 + 308);
    if ( v2 )
    {
      v3 = *(_DWORD *)(v2 - 4);
      v4 = (void *)(v2 - 4);
      if ( v3 == 305419896 )
        free(v4);
    }
    if ( *(_DWORD *)(v1 - 4) == 305419896 )
      free((void *)(v1 - 4));
  }
  if ( a1[132] )
  {
    do
    {
      v5 = a1[132];
      v6 = *(_DWORD *)(v5 + 1064);
      v7 = (_DWORD *)(v5 - 4);
      a1[132] = v6;
      if ( *v7 == 305419896 )
        free(v7);
      v8 = a1[132];
      --numtokens;
    }
    while ( v8 );
  }
  for ( i = 0; i < 4096; i += 4 )
  {
    while ( *(_DWORD *)(i + a1[134]) )
    {
      v10 = a1[134];
      v11 = *(_DWORD *)(v10 + i);
      *(_DWORD *)(i + v10) = *(_DWORD *)(v11 + 28);
      PC_FreeDefine(v11);
    }
  }
  while ( a1[135] )
  {
    v12 = a1[135];
    v13 = *(_DWORD *)(v12 + 12);
    v14 = (_DWORD *)(v12 - 4);
    a1[135] = v13;
    if ( *v14 == 305419896 )
      free(v14);
  }
  v15 = a1[134];
  if ( v15 )
  {
    v16 = *(_DWORD *)(v15 - 4);
    v17 = (void *)(v15 - 4);
    if ( v16 == 305419896 )
      free(v17);
  }
  if ( *(a1 - 1) == 305419896 )
    free(a1 - 1);
}

/* PC_LoadSourceHandle (0x00442010), PC_FreeSourceHandle (0x00442100),
 * PC_ReadTokenHandle (0x00442140) and PC_SourceFileAndLine (0x00442220)
 * are in botlib/l_precomp_pc.c, together with the 64-entry sourceFiles table
 * they share and PC_CheckOpenSourceHandles.  All four are __usercall; see
 * the header comment in that file for the register bindings. */

/* ---- PC_SetBaseFolder  0x00442270 ---- */
// PC_SetBaseFolder | CONFIRMED | Com_sprintf into the module-static base folder buffer.
/* Com_sprintf is __usercall: destination in EDI, size in ESI, so only the
   format string is pushed.  Retail at 0x00442270 is

       mov  esi, 40h                     ; size  = 64
       mov  edi, offset pc_baseFolder     ; dest  = the base folder buffer
       push eax                          ; fmt   = the caller's path

   RtCW passes the path as the format string here too, which is sloppy but is
   what the binary does; a path containing a '%' would be misread by both. */
int __cdecl PC_SetBaseFolder(char *a1)
{
  Com_sprintf((char *)pc_baseFolder, 64, a1);
  return 1;
}

/* PC_CheckOpenSourceHandles (0x00442290) is in botlib/l_precomp_pc.c; it
 * walks the same private sourceFiles table the four handle functions do. */
