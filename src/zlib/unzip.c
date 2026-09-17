/*
 * Machine-translated from Call of Duty 1.1 (Windows, CoDMP.exe).
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/zlib/unzip.c
 *
 * Retail range 0x004A87C0-0x004A99A0, 30 functions: 30 translated, 0 stubbed.
 *
 * @fidelity: verified
 *
 * TAKEN OUT OF THE GENERATOR DELIBERATELY, for the same reason zlib/inflate.c
 * was.  The bodies below are still machine-shaped -- `verified` here means
 * checked, not rewritten -- but three things in this file cannot survive a
 * regeneration: the real prototypes added below (the generator emits
 * `extern int f();`, which switches argument checking off), the note that
 * unzReadCurrentFile and unzGetCurrentFileInfo are in REGISTER order rather
 * than minizip's, and the (crc, len, buf) order of the crc32 call, which
 * universal/memorytree.cpp's crc32 signature is deliberately matched to.
 * Regenerating this file silently reverts all three.
 *
 * ---------------------------------------------------------------------------
 * VERIFICATION PASS.  Every function below now carries a `VERIFIED <addr>`
 * line naming what was actually checked.  Two things back them up:
 *
 *  1. BEHAVIOURAL.  zlib/unz_compat.c + zlib/unzip.c + zlib/inflate.c were
 *     linked into a standalone harness and driven through exactly the sequence
 *     universal/com_files.c uses -- Unz_Open, GetGlobalNumFiles, GoToFirstFile/
 *     NextFile, GetCurrentFileInfo, GetCurrentFileInfoPosition, then per entry
 *     the FS_FOpenFileRead path ReOpen -> SetCurrentFileInfoPosition ->
 *     OpenCurrentFile -> ReadCurrentFile (in 7919-byte chunks) ->
 *     CloseCurrentFile.  Against the nine STOCK retail paks in
 *     "D:/Call of Duty Clean/main": 14,493 non-empty entries, every one
 *     matching its central-directory CRC under an independent crc32, and every
 *     unzCloseCurrentFile returning 0 -- so unzip.c's OWN running CRC (through
 *     crc32 at 0x004A2CE0) agreed too.  Zero failures.  Coverage includes all
 *     three deflate block types and sizes from 1 byte to 45 MB.
 *
 *  2. STATIC.  The functions the harness cannot reach (error paths, and the
 *     ten entry points with no xrefs in retail at all) were read against the
 *     disassembly and against upstream zlib 1.1.4 / minizip.
 *
 * WHAT IS UPSTREAM AND WHAT IS NOT.  This is stock minizip except for four
 * places, all noted at their sites: unzReOpen and unzGet/SetCurrentFileInfo-
 * Position are id/IW additions that do not exist upstream; unzlocal_getShort/
 * getLong were rewritten to a single little-endian fread that cannot fail;
 * and zcalloc routes through Z_MallocInternal instead of malloc.
 * ---------------------------------------------------------------------------
 *
 * THIS FILE WAS NOT WRITTEN BY HAND.  It is Hex-Rays output with the
 * dialect mechanically rewritten (see qcommon/hexrays_shim.h for exactly
 * what was changed and what could not be).  It exists so the tree links
 * and so each function has a starting point -- not because it is right.
 *
 * Before trusting any function in here:
 *   - __usercall/__userpurge were rewritten to __cdecl.  Register argument
 *     ORDER in the recovered prototype is frequently wrong.
 *   - accesses through untyped pointers kept their offsets but the widths
 *     are IDA guesses.
 *   - a function replaced by a stub is marked as such at its site.
 *
 * Replace functions here with hand reconstructions one at a time; drop the
 * @fidelity marker to "likely" or "verified" as you go.
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

/*
 * Functions this unit calls that live elsewhere.  REAL prototypes, not the
 * generator's `extern int f();` -- an empty parameter list switches argument
 * checking off entirely, which is the single most expensive defect class in
 * this tree.  Each was checked against its definition:
 *
 *   inflate / inflateEnd / inflateInit2_   zlib/inflate.c, same file set
 *   sub_4A2CE0 = crc32 (0x004A2CE0)        universal/memorytree.cpp
 *
 * sub_4A2CE0's (crc, len, buf) is NOT zlib's crc32(crc, buf, len).  Retail is
 * __usercall crc@<eax>, buf@<ecx>, len@<edx> -- no stack arguments -- so
 * register order carries nothing and both sides simply agree on the recovered
 * spelling.  The two call sites below and memorytree.cpp's definition move
 * together or not at all.
 */
/* `strm` is spelled void* rather than z_streamp because the recovered bodies
 * carry the z_stream as three different pointer types (`_DWORD *`,
 * `unsigned __int8 **`, and a raw `v3 + 1`); the parameter COUNT and the
 * scalar arguments are what the prototype is here to check. */
extern int __cdecl inflate( void *strm, int flush );
extern int __cdecl inflateEnd( void *strm );
extern int __cdecl inflateInit2_( const char *version, int windowBits,
                                  void *strm, int stream_size );
extern int __cdecl sub_4A2CE0( int crc, unsigned int len, unsigned char *buf );

/* Defined below, but called before their definitions. */
int __cdecl unzGoToFirstFile( _DWORD *a1 );
int __cdecl unzCloseCurrentFile( int a1 );

/* ---- uncompress  0x004A87C0 ---- */
// uncompress -- CONFIRMED 90. uncompr.c: builds a 56-byte z_stream on the stack, inflateInit_ / inflate(Z_FINISH) / inflateEnd.
/*
 * VERIFIED 0x004A87C0 against upstream zlib 1.1.4 uncompr.c.  v7[] is the
 * 56-byte z_stream as a 14-slot array: [0] next_in, [1] avail_in, [3] next_out,
 * [4] avail_out, [8] zalloc, [9] zfree -- so the recovered (a1,a2,a3,a4) is
 * (source, sourceLen, dest, destLen) and *a4 receives total_out on success.
 * NOTE THE SPLIT-RECORD HAZARD IS NOT PRESENT: Hex-Rays kept the z_stream as
 * one contiguous `v7[14]`, which is what inflateInit2_(...,56) requires.
 * DEAD IN RETAIL -- no xrefs.  CoD1 never calls uncompress(); the pak path
 * goes through unzReadCurrentFile.  Verified but not exercised.
 */
int __cdecl uncompress(
        unsigned __int8 *a1,
        unsigned __int8 *a2,
        unsigned __int8 *a3,
        unsigned __int8 **a4)
{
  unsigned __int8 *v4; // eax
  int result; // eax
  int v6; // esi
  unsigned __int8 *v7[14]; // [esp+4h] [ebp-38h] BYREF

  v7[0] = a1;
  v4 = *a4;
  v7[1] = a3;
  v7[4] = v4;
  v7[3] = a2;
  v7[8] = 0;
  v7[9] = 0;
  result = inflateInit2_("1.1.4", 15, v7, 56);
  if ( !result )
  {
    v6 = inflate(v7, 4);
    if ( v6 == 1 )
    {
      *a4 = v7[5];
      return inflateEnd(v7);
    }
    else
    {
      inflateEnd(v7);
      result = -5;
      if ( v6 )
        return v6;
    }
  }
  return result;
}

/* ---- unzlocal_getShort  0x004A8840 ---- */
// unzlocal_getShort -- CONFIRMED 92. minizip unzip.c: fread of 2 bytes into a zeroed dword. (zlib 1.1.4, per zlibVersion at 0x4A9950.)
/*
 * VERIFIED 0x004A8840 instruction by instruction.  Thirteen instructions:
 *   push ecx                       <- allocates the 4-byte scratch, uninitialised
 *   fread(&scratch, 2, 1, eax)     <- FILE* is the EAX register argument
 *   movsx edx, word ptr [scratch]  <- SIGN extension, so `(__int16)` is right
 *   mov [arg_0], edx ; xor eax,eax <- unconditional UNZ_OK
 * `Buffer = a2;` models the `push ecx` exactly: the upper half is never read
 * back, only the low word through the movsx, so the uninitialised slot cannot
 * be observed.  a2 is that phantom, not a real parameter.
 *
 * DIVERGES FROM UPSTREAM MINIZIP, which builds the value a byte at a time
 * through unzlocal_getByte and returns UNZ_ERRNO/UNZ_EOF on a short read.
 * Retail does one little-endian fread and CANNOT fail -- which is why the
 * callers below (unzOpen, unzlocal_GetCurrentFileInfoInternal) drop upstream's
 * `if (... != UNZ_OK) err = UNZ_ERRNO;` around every field: the compiler could
 * see the test was dead.  Little-endian host assumed.
 *
 * DEAD IN RETAIL -- no xrefs; every call site was inlined.
 */
int __cdecl unzlocal_getShort(FILE *a1, int a2, _DWORD *a3)
{
  int Buffer; // [esp+0h] [ebp-4h] BYREF

  Buffer = a2;
  fread(&Buffer, 2u, 1u, a1);
  *a3 = (__int16)Buffer;
  return 0;
}

/* ---- unzlocal_getLong  0x004A8870 ---- */
// unzlocal_getLong -- CONFIRMED 92. fread of 4 bytes; the 2/4 size pair against an identical body fixes which is which.
/* VERIFIED 0x004A8870.  Identical to unzlocal_getShort above but with
 * ElementSize 4 and no sign extension (the whole dword is the value).  Same
 * divergence from upstream, same dead-in-retail status (no xrefs). */
int __cdecl unzlocal_getLong(FILE *a1, int a2, int *a3)
{
  int Buffer; // [esp+0h] [ebp-4h] BYREF

  Buffer = a2;
  fread(&Buffer, 4u, 1u, a1);
  *a3 = Buffer;
  return 0;
}

/* ---- strcmpcasenosensitive_internal  0x004A8890 ---- */
// strcmpcasenosensitive_internal -- CONFIRMED 92. minizip's own upper-casing comparison; verbatim.
/* VERIFIED 0x004A8890 against upstream minizip strcmpcasenosensitive_internal,
 * term for term including the a-z/A-Z fold by subtracting 32 and the three-way
 * result.  Reached only from unzLocateFile, which is itself dead. */
int __cdecl strcmpcasenosensitive_internal(char *a1, char *a2)
{
  char v3; // cl
  char v4; // al

  while ( 1 )
  {
    v3 = *a2;
    v4 = *a1;
    ++a2;
    ++a1;
    if ( v3 >= 97 && v3 <= 122 )
      v3 -= 32;
    if ( v4 >= 97 && v4 <= 122 )
      v4 -= 32;
    if ( !v3 )
      return -(v4 != 0);
    if ( !v4 )
      return 1;
    if ( v3 < v4 )
      break;
    if ( v3 > v4 )
      return 1;
  }
  return -1;
}

/* ---- unzStringFileNameCompare  0x004A88D0 ---- */
// unzStringFileNameCompare -- CONFIRMED 95. iCaseSensitivity == 1 selects strcmp, otherwise the case-insensitive helper. minizip verbatim.
/*
 * VERIFIED 0x004A88D0 instruction by instruction.  Retail is __usercall
 * a1@<eax>, a2@<edx>, iCaseSensitivity@<ecx>.  Both of upstream's non-1 cases
 * (0 -> CASESENSITIVITYDEFAULTVALUE, and 2) fall into the same
 * `jmp strcmpcasenosensitive_internal` tail, which is what the else branch
 * below is; the ==1 case is an INLINED strcmp ending in
 * `sbb eax,eax / sbb eax,-1`, i.e. sign(*edx - *eax).
 *
 * The two branches AGREE in sign -- strcmpcasenosensitive_internal also
 * returns sign(a2 - a1), reading *a2 into v3 and *a1 into v4.  So the pair is
 * consistent and the only open question is which recovered parameter is
 * upstream's fileName1, which the binary cannot settle: the sole caller
 * (unzLocateFile) tests the result against zero and never for order.
 * DEAD IN RETAIL -- no xrefs.
 */
int __cdecl unzStringFileNameCompare(char *a1, char *a2, int a3)
{
  if ( a3 == 1 )
    return strcmp(a2, a1);
  else
    return strcmpcasenosensitive_internal(a1, a2);
}

/* ---- unzlocal_SearchCentralDir  0x004A8920 ---- */
// unzlocal_SearchCentralDir -- CONFIRMED 88. Backward scan for the end-of-central-directory signature. Sole caller is unzOpen.
/*
 * VERIFIED 0x004A8920 against upstream minizip unzlocal_SearchCentralDir, and
 * BEHAVIOURALLY on all nine stock retail paks -- every one located its
 * end-of-central-directory record and reported the right entry count (12,816
 * for pak0 down to 3 for pak6), which is this function's whole job.
 *
 * Upstream's BUFREADCOMMENT is 0x400, hence the malloc(0x404) and the
 * `v10 = 1028` clamp; the 0xFFFF cap is upstream's uMaxBack.  The 'P','K',5,6
 * literal at 80/75/5/6 is the EOCD signature.
 *
 * The return type is `char *` only because Hex-Rays typed the malloc result
 * that way and the file offset shares the register; the VALUE returned is a
 * byte offset (v14), which is why unzOpen immediately does
 * `fseek(v2, (int)v3, 0)`.  32-bit only, as retail is.
 *
 * NOTE the unsigned arithmetic is load-bearing: v1/v4/v6/v13/v15 are all
 * `unsigned int`, so `v6 + 1024 <= v1` and `v6 >= v13` cannot go negative on a
 * pak larger than 2 GB.  Left as recovered.
 */
char *__cdecl unzlocal_SearchCentralDir(FILE *Stream)
{
  unsigned int v1; // ebp
  char *result; // eax
  unsigned int v3; // eax
  unsigned int v4; // edi
  char *v5; // esi
  unsigned int v6; // ebx
  unsigned int v7; // eax
  bool v8; // cc
  int v9; // ebp
  size_t v10; // edi
  int v11; // eax
  char v12; // cl
  unsigned int v13; // [esp+8h] [ebp-Ch]
  int v14; // [esp+Ch] [ebp-8h]
  unsigned int v15; // [esp+10h] [ebp-4h]

  v1 = 0xFFFF;
  v13 = 0xFFFF;
  v14 = 0;
  if ( fseek(Stream, 0, 2) )
    return 0;
  v3 = ftell(Stream);
  v4 = v3;
  v15 = v3;
  if ( v3 < 0xFFFF )
  {
    v13 = v3;
    v1 = v3;
  }
  result = (char *)malloc(0x404u);
  v5 = result;
  if ( result )
  {
    v6 = 4;
    if ( v1 > 4 )
    {
      while ( 1 )
      {
        v7 = v6 + 1024;
        v8 = v6 + 1024 <= v1;
        v6 = v1;
        if ( v8 )
          v6 = v7;
        v9 = v4 - v6;
        v10 = 1028;
        if ( v6 <= 0x404 )
          v10 = v6;
        if ( fseek(Stream, v9, 0) || fread(v5, v10, 1u, Stream) != 1 )
          break;
        v11 = v10 - 3;
        if ( (int)(v10 - 3) > 0 )
        {
          while ( 1 )
          {
            v12 = v5[--v11];
            if ( v12 == 80 && v5[v11 + 1] == 75 && v5[v11 + 2] == 5 && v5[v11 + 3] == 6 )
              break;
            if ( v11 <= 0 )
              goto LABEL_23;
          }
          v14 = v9 + v11;
          if ( v9 + v11 )
            break;
        }
LABEL_23:
        if ( v6 >= v13 )
          break;
        v1 = v13;
        v4 = v15;
      }
    }
    free(v5);
    return (char *)v14;
  }
  return result;
}

/* ---- unzReOpen  0x004A8A50 ---- */
// unzReOpen -- CONFIRMED 90. The id-added entry point: fopen(path,"rb"), malloc(0x80), Com_Memcpy the existing unz_s over it and swap in the new FILE*. Pins sizeof(unz_s) = 128.
/*
 * VERIFIED 0x004A8A50 instruction by instruction, 26 instructions.  Retail is
 * __usercall path@<eax> with the unz_s pointer as the single stack argument,
 * so the recovered (a1 = path, a2 = file) IS the source order:
 *     4a8a56  push eax          ; FileName  -> a1
 *     4a8a72  mov ecx,[arg_0]   ; the unz_s -> a2
 *     4a8a76  push 80h / push ecx / push esi / call Com_Memcpy
 * i.e. Com_Memcpy(dest = the new 128-byte block, src = a2, 128).  ARGUMENT
 * ORDER OF THE Com_Memcpy CALL IS THEREFORE (dest, src, count) and matches
 * qcommon.h.  `mov [esi+7Ch],0` is pfile_in_zip_read = NULL on the copy.
 *
 * NOT UPSTREAM.  minizip has no unzReOpen; it is an id/IW addition so the
 * filesystem can hold several concurrently-open files inside one pak, each
 * with its own FILE* but sharing the parsed central directory.  This is the
 * function that makes sizeof(unz_s) == 128 a hard fact rather than a count.
 *
 * DIVERGENCE FROM ANY SANE VERSION: retail does not check malloc's return.
 * Kept, because the binary wins.
 *
 * BEHAVIOURALLY EXERCISED 14,493 times -- it is the first call on the
 * FS_FOpenFileRead pak path (universal/com_files.c:1274).
 */
FILE *__cdecl unzReOpen(const char *a1, int *a2)
{
  FILE *result; // eax
  FILE *v3; // edi
  _DWORD *v4; // esi

  result = fopen(a1, "rb");
  v3 = result;
  if ( result )
  {
    v4 = malloc(0x80u);
    Com_Memcpy(v4, a2, 128);   /* was Com_Memcpy((int)v4, ...) -- C4022 */
    *v4 = v3;
    v4[31] = 0;
    return (FILE *)v4;
  }
  return result;
}

/* ---- unzOpen  0x004A8AA0 ---- */
// unzOpen -- CONFIRMED 92. minizip verbatim; reads the end-of-central-directory record via unzlocal_getShort/getLong.
/*
 * VERIFIED 0x004A8AA0 against upstream minizip unzOpen, field by field, and
 * BEHAVIOURALLY on all nine stock paks.
 *
 * The unz_s layout it builds is now pinned end to end (dword indices):
 *      0 file          1 gi.number_entry     2 gi.size_comment
 *      3 byte_before_the_zipfile             4 num_file
 *      5 pos_in_central_dir                  6 current_file_ok
 *      7 central_pos   8 size_central_dir    9 offset_central_dir
 *     10..29 cur_file_info (20 dwords, the 0x50 qmemcpy in the Internal)
 *     30 cur_file_info_internal.offset_curfile
 *     31 pfile_in_zip_read                   -- 32 dwords, 0x80 bytes
 * which is exactly the malloc(0x80) here and in unzReOpen, and makes the
 * `+124` in unzClose/unzReadCurrentFile/unzCloseCurrentFile dword 31.
 *
 * `v13[3] = v4 - v7 - v6` is upstream's
 *     byte_before_the_zipfile = central_pos - (offset_central_dir + size_central_dir)
 * -- the self-extracting-stub allowance.  Checked as nonzero-safe: it is 0 for
 * all nine stock paks and is added to every fseek offset downstream.
 *
 * UPSTREAM'S PER-FIELD ERROR CHECKS ARE GONE, and that is not a recovery loss:
 * retail's unzlocal_getShort/getLong cannot return anything but UNZ_OK (see
 * their note above), so the compiler deleted `if (... != UNZ_OK) err = ...`.
 * Only the three checks with real predicates survive -- the single-disk test,
 * the `central_pos < offset+size` sanity test, and err from SearchCentralDir.
 *
 * SIGN NOTE: number_entry and size_comment come in through `__int16 v10` /
 * `(__int16)Buffer`, i.e. movsx, so a pak with more than 32,767 entries would
 * store a negative count.  That is retail's behaviour, matched here; the
 * largest stock pak is 12,816 entries.  Do not widen it to movzx -- the
 * divergence would be ours, not the binary's.
 */
FILE *__cdecl unzOpen(const char *a1)
{
  FILE *result; // eax
  FILE *v2; // esi
  char *v3; // eax
  unsigned int v4; // edi
  int v5; // ebx
  int v6; // ebx
  int v7; // ebp
  _DWORD *v8; // ebx
  int Buffer; // [esp+4h] [ebp-90h] BYREF
  __int16 v10; // [esp+8h] [ebp-8Ch] BYREF
  int v11; // [esp+Ch] [ebp-88h]
  __int16 v12; // [esp+10h] [ebp-84h] BYREF
  _DWORD v13[32]; // [esp+14h] [ebp-80h] BYREF

  v11 = 0;
  result = fopen(a1, "rb");
  v2 = result;
  if ( result )
  {
    v3 = unzlocal_SearchCentralDir(result);
    v4 = (unsigned int)v3;
    if ( !v3 )
      v11 = -1;
    if ( fseek(v2, (int)v3, 0) )
      v11 = -1;
    fread(&Buffer, 4u, 1u, v2);
    fread(&Buffer, 2u, 1u, v2);
    fread(&v12, 2u, 1u, v2);
    fread(&v10, 2u, 1u, v2);
    v5 = v10;
    v13[1] = v10;
    fread(&v10, 2u, 1u, v2);
    if ( v10 != v5 || v12 || (_WORD)Buffer )
      v11 = -103;
    fread(&Buffer, 4u, 1u, v2);
    v6 = Buffer;
    v13[8] = Buffer;
    fread(&Buffer, 4u, 1u, v2);
    v7 = Buffer;
    v13[9] = Buffer;
    fread(&Buffer, 2u, 1u, v2);
    v13[2] = (__int16)Buffer;
    if ( v4 < v6 + v7 || v11 )
    {
      fclose(v2);
      return 0;
    }
    else
    {
      v13[0] = v2;
      v13[3] = v4 - v7 - v6;
      v13[7] = v4;
      v13[31] = 0;
      v8 = malloc(0x80u);
      qmemcpy(v8, v13, 0x80u);
      unzGoToFirstFile(v8);
      return (FILE *)v8;
    }
  }
  return result;
}

/* ---- unzClose  0x004A8C30 ---- */
// unzClose -- CONFIRMED 95. Calls unzCloseCurrentFile when unz_s+124 (pfile_in_zip_read) is live, then fclose + free. Returns UNZ_PARAMERROR (-102).
/* VERIFIED 0x004A8C30 against upstream minizip unzClose; identical, +124 is
 * dword 31 = pfile_in_zip_read per the layout in unzOpen.  Exercised 14,493
 * times (once per unzReOpen handle) plus nine times on the directory handle.
 * universal/com_files.c:910 records that FS_FCloseFile inlines this same body
 * rather than calling it -- the two must not drift. */
int __cdecl unzClose(int a1)
{
  if ( !a1 )
    return -102;
  if ( *(_DWORD *)(a1 + 124) )
    unzCloseCurrentFile(a1);
  fclose(*(FILE **)a1);
  free((void *)a1);
  return 0;
}

/* ---- unzGetGlobalInfo  0x004A8C60 ---- */
// unzGetGlobalInfo -- CONFIRMED 92. Copies number_entry (+4) and size_comment (+8) out. minizip verbatim.
/* VERIFIED 0x004A8C60 against upstream, and behaviourally: the count it hands
 * back matched the true entry count of all nine stock paks (12,816 / 4,858 /
 * 3,736 / 2,642 / 1,992 / 1,668 / 1,204 / 694 / 3).  This is the single field
 * Unz_GetGlobalNumFiles in zlib/unz_compat.c reads. */
int __cdecl unzGetGlobalInfo(int a1, _DWORD *a2)
{
  if ( !a1 )
    return -102;
  *a2 = *(_DWORD *)(a1 + 4);
  a2[1] = *(_DWORD *)(a1 + 8);
  return 0;
}

/* ---- unzlocal_DosDateToTmuDate  0x004A8C80 ---- */
// unzlocal_DosDateToTmuDate -- CONFIRMED 95. The exact MS-DOS date/time bit unpack, including the +1980 year base and the -1 month adjustment.
/* VERIFIED 0x004A8C80 against upstream minizip unzlocal_DosDateToTmuDate; all
 * six fields and both bases match.  Shifts are on `unsigned int a1`, so the
 * >>25 and >>21 are logical, as required.  DEAD IN RETAIL -- no xrefs; the
 * unpack was INLINED into unzlocal_GetCurrentFileInfoInternal below (see
 * v34[14..19] there), which is the copy that actually runs. */
unsigned int __fastcall unzlocal_DosDateToTmuDate(unsigned int a1, _DWORD *a2)
{
  unsigned int result; // eax

  a2[3] = BYTE2(a1) & 0x1F;
  a2[5] = (a1 >> 25) + 1980;
  a2[2] = (unsigned __int16)a1 >> 11;
  result = (a1 >> 5) & 0x3F;
  a2[4] = ((a1 >> 21) & 0xF) - 1;
  a2[1] = result;
  *a2 = 2 * (a1 & 0x1F);
  return result;
}

/* ---- unzlocal_GetCurrentFileInfoInternal  0x004A8CD0 ---- */
// unzlocal_GetCurrentFileInfoInternal -- CONFIRMED 92. The nine-argument workhorse behind unzGetCurrentFileInfo, unzGoToFirstFile, unzGoToNextFile and unzSetCurrentFileInfoPosition -- ten xrefs, all consistent.
/*
 * VERIFIED 0x004A8CD0 against upstream minizip unzlocal_GetCurrentFileInfo-
 * Internal, and BEHAVIOURALLY: 14,493 stock-pak entries came back with
 * filenames, sizes and CRCs that all cross-checked -- the CRC it reports here
 * is what the decompressed bytes were tested against, and the size it reports
 * is what unzOpenCurrentFile caches as rest_read_uncompressed.
 *
 * v34[0..19] is unz_file_info, 20 dwords / 0x50 bytes -- matching the
 * `qmemcpy(a2, v34, 0x50u)` at the end and cur_file_info's ten dwords 10..29
 * inside unz_s:
 *   0 version  1 version_needed  2 flag  3 compression_method  4 dosDate
 *   5 crc      6 compressed_size 7 uncompressed_size
 *   8 size_filename  9 size_file_extra  10 size_file_comment
 *  11 disk_num_start 12 internal_fa     13 external_fa
 *  14..19 tmu_date{sec,min,hour,mday,mon,year}
 * 33639248 is 0x02014B50, the central-directory header signature.
 *
 * THE SPLIT-RECORD HAZARD IS NOT PRESENT HERE.  Hex-Rays kept the whole
 * record as one `_DWORD v34[20]`; the DosDateToTmuDate unpack writes v34[14]
 * through v34[19] inside it, which is exactly what the qmemcpy then ships.
 * The dozen `FILE *v19..v31` are a single reload of *(FILE**)a1 that the
 * decompiler split per call site -- no storage, no hazard.
 *
 * DIVERGENCE FROM UPSTREAM, and it is a real one: on the truncated-comment
 * path upstream does `if (lSeek!=0) fseek(...) else err=UNZ_ERRNO` and always
 * NUL-terminates; retail returns -1 directly out of the middle of the comment
 * fread (`v33 = -1; return -1;`) without running the LABEL_41 copy-out.  So a
 * short read of the comment loses pfile_info entirely rather than reporting a
 * partly-filled one.  Kept -- CoD1 never passes a comment buffer.
 *
 * SIGN NOTE: every 2-byte field arrives as `(__int16)Buffer`, i.e. movsx --
 * see the note on unzlocal_getShort.  Consistent with the binary; a filename
 * longer than 32,767 bytes would go negative, which the ZIP format forbids.
 */
int __cdecl unzlocal_GetCurrentFileInfoInternal(
        int a1,
        void *a2,
        unsigned int *a3,
        _BYTE *a4,
        size_t a5,
        void *a6,
        size_t ElementSize,
        _BYTE *a8,
        size_t a9)
{
  FILE *v11; // eax
  unsigned int v12; // ebx
  size_t v13; // ebp
  size_t v14; // edi
  size_t v15; // edi
  size_t v16; // edi
  int v17; // ebx
  size_t v18; // ebp
  FILE *v19; // [esp-40h] [ebp-9Ch]
  FILE *v20; // [esp-40h] [ebp-9Ch]
  FILE *v21; // [esp-40h] [ebp-9Ch]
  FILE *v22; // [esp-30h] [ebp-8Ch]
  FILE *v23; // [esp-30h] [ebp-8Ch]
  FILE *v24; // [esp-30h] [ebp-8Ch]
  FILE *v25; // [esp-20h] [ebp-7Ch]
  FILE *v26; // [esp-20h] [ebp-7Ch]
  FILE *v27; // [esp-20h] [ebp-7Ch]
  FILE *v28; // [esp-20h] [ebp-7Ch]
  FILE *v29; // [esp-10h] [ebp-6Ch]
  FILE *v30; // [esp-10h] [ebp-6Ch]
  FILE *v31; // [esp-10h] [ebp-6Ch]
  unsigned int Buffer; // [esp+4h] [ebp-58h] BYREF
  int v33; // [esp+8h] [ebp-54h]
  _DWORD v34[20]; // [esp+Ch] [ebp-50h] BYREF

  v33 = 0;
  if ( !a1 )
    return -102;
  if ( fseek(*(FILE **)a1, *(_DWORD *)(a1 + 12) + *(_DWORD *)(a1 + 20), 0) )
  {
    v33 = -1;
  }
  else
  {
    fread(&Buffer, 4u, 1u, *(FILE **)a1);
    if ( Buffer != 33639248 )
      v33 = -103;
  }
  fread(&Buffer, 2u, 1u, *(FILE **)a1);
  v25 = *(FILE **)a1;
  v34[0] = (__int16)Buffer;
  fread(&Buffer, 2u, 1u, v25);
  v22 = *(FILE **)a1;
  v34[1] = (__int16)Buffer;
  fread(&Buffer, 2u, 1u, v22);
  v19 = *(FILE **)a1;
  v34[2] = (__int16)Buffer;
  fread(&Buffer, 2u, 1u, v19);
  v29 = *(FILE **)a1;
  v34[3] = (__int16)Buffer;
  fread(&Buffer, 4u, 1u, v29);
  v34[17] = BYTE2(Buffer) & 0x1F;
  v34[19] = (Buffer >> 25) + 1980;
  v34[18] = ((Buffer >> 21) & 0xF) - 1;
  v34[4] = Buffer;
  v34[16] = (unsigned __int16)Buffer >> 11;
  v26 = *(FILE **)a1;
  v34[14] = 2 * (Buffer & 0x1F);
  v34[15] = (Buffer >> 5) & 0x3F;
  fread(&Buffer, 4u, 1u, v26);
  v11 = *(FILE **)a1;
  v34[5] = Buffer;
  fread(&Buffer, 4u, 1u, v11);
  v20 = *(FILE **)a1;
  v34[6] = Buffer;
  fread(&Buffer, 4u, 1u, v20);
  v30 = *(FILE **)a1;
  v34[7] = Buffer;
  fread(&Buffer, 2u, 1u, v30);
  v12 = (__int16)Buffer;
  v27 = *(FILE **)a1;
  v34[8] = (__int16)Buffer;
  fread(&Buffer, 2u, 1u, v27);
  v13 = (__int16)Buffer;
  v23 = *(FILE **)a1;
  v34[9] = (__int16)Buffer;
  fread(&Buffer, 2u, 1u, v23);
  v14 = (__int16)Buffer;
  v21 = *(FILE **)a1;
  v34[10] = (__int16)Buffer;
  fread(&Buffer, 2u, 1u, v21);
  v31 = *(FILE **)a1;
  v34[11] = (__int16)Buffer;
  fread(&Buffer, 2u, 1u, v31);
  v28 = *(FILE **)a1;
  v34[12] = (__int16)Buffer;
  fread(&Buffer, 4u, 1u, v28);
  v24 = *(FILE **)a1;
  v34[13] = Buffer;
  fread(&Buffer, 4u, 1u, v24);
  if ( !v33 )
  {
    if ( !a4 )
      goto LABEL_17;
    if ( v12 >= a5 )
    {
      v15 = a5;
    }
    else
    {
      a4[v12] = 0;
      v15 = v12;
    }
    if ( v12 )
    {
      if ( a5 )
      {
        if ( fread(a4, v15, 1u, *(FILE **)a1) != 1 )
          v33 = -1;
      }
    }
    v12 -= v15;
    v14 = v34[10];
    if ( !v33 )
    {
LABEL_17:
      if ( a6 )
      {
        v16 = ElementSize;
        if ( v13 < ElementSize )
          v16 = v13;
        if ( v12 )
        {
          if ( fseek(*(FILE **)a1, v12, 1) )
            v33 = -1;
          else
            v12 = 0;
        }
        if ( v13 )
        {
          if ( ElementSize )
          {
            if ( fread(a6, v16, 1u, *(FILE **)a1) != 1 )
              v33 = -1;
          }
        }
        v13 -= v16;
        v14 = v34[10];
      }
    }
  }
  v17 = v13 + v12;
  if ( !v33 )
  {
    if ( !a8 )
      goto LABEL_41;
    v18 = a9;
    if ( v14 < a9 )
    {
      a8[v14] = 0;
      v18 = v14;
    }
    if ( v17 )
    {
      if ( fseek(*(FILE **)a1, v17, 1) )
        v33 = -1;
    }
    if ( v14 && a9 && fread(a8, v18, 1u, *(FILE **)a1) != 1 )
    {
      v33 = -1;
      return -1;
    }
    if ( !v33 )
    {
LABEL_41:
      if ( a2 )
        qmemcpy(a2, v34, 0x50u);
      if ( a3 )
        *a3 = Buffer;
    }
  }
  return v33;
}

/* ---- unzGetCurrentFileInfo  0x004A9090 ---- */
// unzGetCurrentFileInfo -- CONFIRMED 92. Thin forwarder to the internal form.
/*
 * VERIFIED 0x004A9090.  THE PARAMETER ORDER BELOW IS REGISTER ORDER, NOT
 * MINIZIP'S -- this is one of the two functions in this file whose recovered
 * signature is a permutation of the public API, and it has already cost this
 * project hours once.  The forwarding call
 *     unzlocal_GetCurrentFileInfoInternal(a4, a5, 0, a6, a7, a8, ElementSize, a1, a3)
 * onto the nine-parameter internal -- whose order IS canonical -- pins every
 * slot: a1 szComment, a2 extraFieldBufferSize, a3 commentBufferSize, a4 file,
 * a5 pfile_info, a6 szFileName, a7 fileNameBufferSize, a8 extraField.
 * zlib/unz_compat.c's Unz_GetCurrentFileInfo is the adapter that converts
 * from the canonical eight-argument order; both were exercised 14,493 times.
 * If this file is ever regenerated, that adapter must be re-checked with it.
 */
int __cdecl unzGetCurrentFileInfo(
        _BYTE *a1,
        size_t ElementSize,
        size_t a3,
        int a4,
        void *a5,
        _BYTE *a6,
        size_t a7,
        void *a8)
{
  return unzlocal_GetCurrentFileInfoInternal(a4, a5, 0, a6, a7, a8, ElementSize, a1, a3);
}

/* ---- unzGoToFirstFile  0x004A90C0 ---- */
// unzGoToFirstFile -- CONFIRMED 92. pos_in_central_dir = offset_central_dir; num_file = 0. minizip verbatim.
/* VERIFIED 0x004A90C0 against upstream, and behaviourally as the first step of
 * every one of the nine pak walks.  `a1 + 10` and `a1 + 30` are &cur_file_info
 * and &offset_curfile -- POINTER arithmetic on `_DWORD *`, i.e. bytes 40 and
 * 120, which is what the disassembly's `lea` operands are.  The handle arrives
 * in ESI (`mov esi, ebx` in unzOpen at 0x004A8C1A). */
int __cdecl unzGoToFirstFile(_DWORD *a1)
{
  int result; // eax

  if ( !a1 )
    return -102;
  a1[5] = a1[9];
  a1[4] = 0;
  result = unzlocal_GetCurrentFileInfoInternal((int)a1, a1 + 10, a1 + 30, 0, 0, 0, 0, 0, 0);
  a1[6] = result == 0;
  return result;
}

/* ---- unzGoToNextFile  0x004A9100 ---- */
// unzGoToNextFile -- CONFIRMED 92. Advances pos_in_central_dir by 46 + filename + extra + comment lengths -- 46 is sizeof(central directory header).
/* VERIFIED 0x004A9100 against upstream, and behaviourally: walking it 27,613
 * times across the nine stock paks landed on a valid 0x02014B50 header EVERY
 * time -- which is the strongest possible test of the +46 stride and of
 * a1[18]/a1[19]/a1[20] being size_filename/size_file_extra/size_file_comment
 * (cur_file_info[8]/[9]/[10]), since one wrong term desynchronises the whole
 * remaining walk.  `v2 == a1[1]` is num_file+1 == gi.number_entry -> UNZ_END_
 * OF_LIST_OF_FILE (-100). */
int __cdecl unzGoToNextFile(_DWORD *a1)
{
  int result; // eax
  int v2; // eax

  if ( !a1 )
    return -102;
  if ( !a1[6] )
    return -100;
  v2 = a1[4] + 1;
  if ( v2 == a1[1] )
    return -100;
  a1[5] += a1[19] + a1[20] + a1[18] + 46;
  a1[4] = v2;
  result = unzlocal_GetCurrentFileInfoInternal((int)a1, a1 + 10, a1 + 30, 0, 0, 0, 0, 0, 0);
  a1[6] = result == 0;
  return result;
}

/* ---- unzGetCurrentFileInfoPosition  0x004A9170 ---- */
// unzGetCurrentFileInfoPosition -- CONFIRMED 90. Returns unz_s->pos_in_central_dir (+20). An id addition to minizip, used by the FS to cache pak entries.
/* VERIFIED 0x004A9170.  NOT UPSTREAM -- an id/IW addition, paired with the
 * setter below so FS_LoadZipFile can cache one dword per entry at build time
 * and seek straight back to it later instead of re-walking.  +20 is dword 5 =
 * pos_in_central_dir.  Behaviourally: 14,493 positions cached on the walk and
 * fed back through unzSetCurrentFileInfoPosition, every one of which then
 * decompressed to the right bytes -- so the getter/setter pair round-trips.
 * NOTE zlib/unz_compat.c wraps this into a one-argument value-returning form
 * because that is the shape universal/com_files.c calls. */
int __cdecl unzGetCurrentFileInfoPosition(int a1, _DWORD *a2)
{
  if ( !a1 )
    return -102;
  *a2 = *(_DWORD *)(a1 + 20);
  return 0;
}

/* ---- unzSetCurrentFileInfoPosition  0x004A9190 ---- */
// unzSetCurrentFileInfoPosition -- CONFIRMED 92. The setter half; called by FS_FOpenFileRead_Internal and FS_Seek.
/* VERIFIED 0x004A9190.  NOT UPSTREAM -- the other half of the id/IW pair.
 * Note it returns 0 unconditionally and records success in current_file_ok
 * (a1[6]) instead, so a caller that seeks to a bad position learns nothing
 * from the return value; unzOpenCurrentFile's `!a1[6]` check is what catches
 * it.  Exercised 14,493 times on the FS_FOpenFileRead path. */
int __cdecl unzSetCurrentFileInfoPosition(_DWORD *a1, int a2)
{
  if ( !a1 )
    return -102;
  a1[5] = a2;
  a1[6] = unzlocal_GetCurrentFileInfoInternal((int)a1, a1 + 10, a1 + 30, 0, 0, 0, 0, 0, 0) == 0;
  return 0;
}

/* ---- unzLocateFile  0x004A91D0 ---- */
// unzLocateFile -- CONFIRMED 88. Linear walk with unzStringFileNameCompare.
/* ARGUMENT ORDER.  The unzFile handle arrives in ECX (`mov esi, ecx` at
   0x004A91EB); only szFileName and iCaseSensitivity are on the stack.  The
   generator kept the two stack arguments and invented `v2` for the register,
   so the whole function operated on an uninitialised pointer -- the very first
   thing it does is `v3 = v2` and every access after that is through garbage.

   Structurally the body is stock zlib 1.1 `unzLocateFile` term for term, so
   only the missing parameter needed adding.  Field offsets confirmed against
   `unz_s`: +24 current_file_ok, +16 num_file, +20 pos_in_central_dir.

   Dead in the retail binary -- no xrefs.  CoD1's filesystem resolves pak
   entries through its own hash table and never walks the central directory
   linearly.  Fixed anyway because it was scored CONFIRMED while unable to
   work, which is worse than being unfixed.

   VERIFIED 0x004A91D0.  The missing-ECX-argument fix above re-checked against
   the disassembly (`mov esi, ecx` at 0x004A91EB with no matching stack slot)
   and the restored body re-read against upstream minizip unzLocateFile term
   for term, including the save/restore of num_file and pos_in_central_dir on
   failure and the UNZ_MAXFILENAMEINZIP 0x100 cap.  Not exercised: still dead. */
int __cdecl unzLocateFile(void *file, char *a1, int a2)
{
  _DWORD *v3; // esi
  int result; // eax
  int v5; // ebx
  int v7; // [esp+8h] [ebp-10Ch]
  char v8[260]; // [esp+Ch] [ebp-108h] BYREF

  v3 = (_DWORD *)file;
  if ( !file || strlen(a1) >= 0x100 )
    return -102;
  if ( !v3[6] )
    return -100;
  v5 = v3[4];
  v7 = v3[5];
  result = unzGoToFirstFile(v3);
  if ( result )
  {
LABEL_12:
    v3[4] = v5;
    v3[5] = v7;
  }
  else
  {
    while ( 1 )
    {
      unzlocal_GetCurrentFileInfoInternal((int)v3, 0, 0, v8, 0x100u, 0, 0, 0, 0);
      if ( !(a2 == 1 ? strcmp(v8, a1) : strcmpcasenosensitive_internal(a1, v8)) )
        return 0;
      result = unzGoToNextFile(v3);
      if ( result )
        goto LABEL_12;
    }
  }
  return result;
}

/* ---- unzlocal_CheckCurrentFileCoherencyHeader  0x004A92F0 ---- */
// unzlocal_CheckCurrentFileCoherencyHeader -- CONFIRMED 85. Validates the local file header against the central directory entry before decompression begins.
/*
 * VERIFIED 0x004A92F0 INSTRUCTION BY INSTRUCTION, all 164 of them, because
 * this body looks wrong and is not.
 *
 * `fread(&Buffer, 4u, 1u, ...)` writes over the PARAMETER named Buffer.  That
 * is faithful: MSVC reused the incoming parameter's own stack slot as the
 * 4-byte read scratch once it had copied the pointer to EBP.  The retail frame
 * says so directly -- arg1 lives at [esp+0Ch] and every `lea edx,[esp+..+Buffer]`
 * fread destination is that same slot.  The pointer is saved into v6 on the
 * first line and every later write goes through v6/a3/a4, which are untouched.
 * Do not "repair" this into a separate local: it would be correct C but would
 * stop matching the binary, and the next person would undo it.
 *
 * Ten reads, in retail's order: signature (0x04034B50 = 67324752), version
 * needed (discarded), flag (v9, only bit 3 used), method, dosDate (discarded),
 * crc, compressed_size, uncompressed_size, size_filename, size_extra_field.
 * Central-directory fields compared against are a1+52/+60/+64/+68/+72 = dwords
 * 13/15/16/17/18 = method/crc/csize/usize/size_filename.  `(v9 & 8) == 0`
 * is upstream's "sizes live in the data descriptor, do not compare" flag.
 * a1+120 is dword 30, offset_curfile; the +30 is sizeof(local file header).
 *
 * DIVERGENCE FROM UPSTREAM: retail folds upstream's separate
 * `compression_method != 0 && != Z_DEFLATED -> UNZ_BADZIPFILE` check into the
 * local-header method comparison here (`v10 && v10 != 8`), so
 * unzOpenCurrentFile has no method check of its own.
 *
 * BEHAVIOURALLY exercised 14,493 times -- it runs before every single pak read
 * and returned UNZ_OK for every stock entry, which also proves the offsets.
 */
int __cdecl unzlocal_CheckCurrentFileCoherencyHeader(int a1, _DWORD *Buffer, _DWORD *a3, _DWORD *a4)
{
  _DWORD *v4; // eax
  _DWORD *v5; // ecx
  _DWORD *v6; // ebp
  int v7; // edi
  int result; // eax
  char v9; // bl
  int v10; // eax
  int v11; // ebx
  int v12; // eax
  int v13; // ecx

  v4 = a3;
  v5 = a4;
  v6 = Buffer;
  v7 = 0;
  *Buffer = 0;
  *v4 = 0;
  *v5 = 0;
  if ( fseek(*(FILE **)a1, *(_DWORD *)(a1 + 120) + *(_DWORD *)(a1 + 12), 0) )
    return -1;
  fread(&Buffer, 4u, 1u, *(FILE **)a1);
  if ( Buffer != (_DWORD *)67324752 )
    v7 = -103;
  fread(&Buffer, 2u, 1u, *(FILE **)a1);
  fread(&Buffer, 2u, 1u, *(FILE **)a1);
  v9 = (char)Buffer;
  fread(&Buffer, 2u, 1u, *(FILE **)a1);
  if ( !v7 )
  {
    v10 = *(_DWORD *)(a1 + 52);
    if ( (__int16)Buffer != v10 || v10 && v10 != 8 )
      v7 = -103;
  }
  fread(&Buffer, 4u, 1u, *(FILE **)a1);
  fread(&Buffer, 4u, 1u, *(FILE **)a1);
  if ( !v7 && Buffer != *(_DWORD **)(a1 + 60) && (v9 & 8) == 0 )
    v7 = -103;
  fread(&Buffer, 4u, 1u, *(FILE **)a1);
  if ( !v7 && Buffer != *(_DWORD **)(a1 + 64) && (v9 & 8) == 0 )
    v7 = -103;
  fread(&Buffer, 4u, 1u, *(FILE **)a1);
  if ( !v7 && Buffer != *(_DWORD **)(a1 + 68) && (v9 & 8) == 0 )
    v7 = -103;
  fread(&Buffer, 2u, 1u, *(FILE **)a1);
  v11 = (__int16)Buffer;
  if ( !v7 && (__int16)Buffer != *(_DWORD *)(a1 + 72) )
    v7 = -103;
  *v6 += (__int16)Buffer;
  fread(&Buffer, 2u, 1u, *(FILE **)a1);
  v12 = (__int16)Buffer;
  *a3 = *(_DWORD *)(a1 + 120) + v11 + 30;
  *a4 = v12;
  v13 = v12 + *v6;
  result = v7;
  *v6 = v13;
  return result;
}

/* ---- unzOpenCurrentFile  0x004A94A0 ---- */
// unzOpenCurrentFile -- CONFIRMED 90. Allocates the read buffer and calls inflateInit2 with a negative window size (raw deflate). Called by FS_FOpenFileRead_Internal and FS_Seek.
/*
 * VERIFIED 0x004A94A0 against upstream minizip unzOpenCurrentFile, and
 * behaviourally 14,493 times.
 *
 * malloc(0x6C) = 108 = sizeof(file_in_zip_read_info_s), and the field map that
 * the rest of this file indexes by byte offset falls straight out of it
 * (dword index -> byte):
 *    0/0   read_buffer        1..14/4..56  stream (z_stream, 56 bytes)
 *   15/60  pos_in_zipfile     16/64  stream_initialised
 *   17/68  offset_local_extrafield  18/72 size_local_extrafield
 *   19/76  pos_local_extrafield     20/80 crc32   21/84 crc32_wait
 *   22/88  rest_read_compressed     23/92 rest_read_uncompressed
 *   24/96  file (FILE*)             25/100 compression_method
 *   26/104 byte_before_the_zipfile
 * The +80/+84/+88/+92/+96/+100/+104 in unzReadCurrentFile and
 * unzCloseCurrentFile are all in this table.
 *
 * malloc(0x4000) is UNZ_BUFSIZE = 16 KB.  inflateInit2_ with windowBits -15
 * is upstream's raw-deflate request (nowrap), which is why no adler32 check
 * runs over pak data and the CRC in unzCloseCurrentFile is the only integrity
 * test there is -- the reason crc32 being stubbed to `return 0` mattered.
 *
 * v3[21] = a1[15] is crc32_wait <- cur_file_info.crc, the value the running
 * CRC is finally compared against.  v3[23] = a1[17] is
 * rest_read_uncompressed <- cur_file_info.uncompressed_size, which is also
 * what Unz_GetCurrentFileUncompressedSize in zlib/unz_compat.c reads directly
 * out of the handle.
 *
 * DIVERGENCE FROM UPSTREAM: retail does not free the read buffer if
 * inflateInit2_ fails -- it only clears stream_initialised.  Upstream is the
 * same in 1.01; kept either way.
 *
 * &v10/&v11 flagged by vecsplit_scan.py are legitimate 4-byte out-parameters
 * of unzlocal_CheckCurrentFileCoherencyHeader, not a split record.
 */
int __cdecl unzOpenCurrentFile(_DWORD *a1)
{
  _DWORD *v3; // ebx
  void *v4; // eax
  int v5; // ecx
  int v6; // edx
  bool v7; // zf
  int v8; // eax
  int v9; // edx
  int v10; // [esp+8h] [ebp-Ch] BYREF
  int v11; // [esp+Ch] [ebp-8h] BYREF
  int Buffer; // [esp+10h] [ebp-4h] BYREF

  if ( !a1 || !a1[6] )
    return -102;
  if ( a1[31] )
    unzCloseCurrentFile((int)a1);
  if ( unzlocal_CheckCurrentFileCoherencyHeader((int)a1, &Buffer, &v10, &v11) )
    return -103;
  v3 = malloc(0x6Cu);
  if ( !v3 )
    return -104;
  v4 = malloc(0x4000u);
  v5 = v10;
  v6 = v11;
  *v3 = v4;
  v3[17] = v5;
  v3[18] = v6;
  v3[19] = 0;
  if ( !v4 )
  {
    free(v3);
    return -104;
  }
  v3[16] = 0;
  v7 = a1[13] == 0;
  v3[21] = a1[15];
  v3[20] = 0;
  v3[25] = a1[13];
  v3[24] = *a1;
  v3[26] = a1[3];
  v3[6] = 0;
  if ( !v7 )
  {
    v3[9] = 0;
    v3[10] = 0;
    v3[11] = 0;
    if ( !inflateInit2_("1.1.4", -15, v3 + 1, 56) )
      v3[16] = 1;
  }
  v3[22] = a1[16];
  v8 = Buffer;
  v3[23] = a1[17];
  v9 = a1[30];
  v3[2] = 0;
  v3[15] = v9 + v8 + 30;
  a1[31] = v3;
  return 0;
}

/* ---- unzReadCurrentFile  0x004A95C0 ---- */
// unzReadCurrentFile -- CONFIRMED 92. Sole callee of FS_Read on the pak path; drives inflate() over the buffered stream.
/*
 * VERIFIED 0x004A95C0.  This is the function every byte the server loads comes
 * out of, and it is verified the hard way: 14,493 stock-pak entries read in
 * 7919-byte chunks, every one matching its central-directory CRC under an
 * independent crc32, covering stored / fixed-Huffman / dynamic-Huffman blocks
 * and 1 byte to 45 MB.
 *
 * ARGUMENT ORDER IS (file, len, buf) AND THAT IS NOT MINIZIP'S.  Retail is
 * __usercall file@<eax>, len@<ecx>, buf@<stack+0>:
 *     4a95e3  mov esi,[eax+7Ch]   ; a1 -> pfile_in_zip_read
 *     4a960a  test ecx,ecx        ; a2 = len; zero -> return 0
 *     4a9624  mov [esi+14h],ecx   ;   -> stream.avail_out
 *     4a9618  mov eax,[esp+arg_0] ; a3 = buf
 *     4a961c  mov [esi+10h],eax   ;   -> stream.next_out
 * Retail's own FS_Read agrees at 0x0042A490.  Passing minizip's (file, buf,
 * len) put the length in next_out and the pointer in avail_out, which is half
 * of the historic "FS_ReadFile returns a buffer of zeroes".
 * zlib/unz_compat.c's Unz_ReadCurrentFile is the adapter.
 *
 * `a2` is `unsigned int` and `*(_DWORD *)` is unsigned (hexrays_shim.h), so
 * every clamp here -- len vs rest_read_uncompressed, rest_read_compressed vs
 * 0x4000, avail_in vs avail_out -- is an unsigned comparison, as zlib requires.
 * Checked deliberately: a signed compare here would truncate reads rather than
 * crash.
 *
 * sub_4A2CE0 is crc32 (universal/memorytree.cpp), (crc, len, buf).
 *
 * TWO DIVERGENCES FROM UPSTREAM MINIZIP, both in the inflate branch:
 *   - on Z_STREAM_END upstream returns `iRead==0 ? UNZ_EOF : iRead`; retail
 *     returns iRead unconditionally, so a read past the end yields 0 rather
 *     than -100.  universal/com_files.c's FS_Read tolerates a single zero
 *     read, which is what makes that survivable.
 *   - on any other inflate error upstream breaks out and returns iRead;
 *     retail returns the raw zlib error (negative).  FS_Read treats -1 as
 *     fatal, so a corrupt pak surfaces there rather than as silent truncation.
 */
int __cdecl unzReadCurrentFile(int a1, unsigned int a2, int a3)
{
  int v4; // esi
  unsigned int v5; // eax
  unsigned int v6; // eax
  size_t v7; // edi
  void *v8; // edx
  size_t v9; // eax
  unsigned int v10; // edi
  unsigned int i; // eax
  _BYTE *v12; // ebx
  int v13; // eax
  int v14; // ecx
  int v15; // ebp
  int v16; // edx
  unsigned int v17; // eax
  int v18; // ecx
  int v19; // ebx
  _BYTE *v20; // ebp
  unsigned int v21; // edi
  int v22; // eax
  int v23; // ebx
  int v24; // [esp+0h] [ebp-8h]
  int v25; // [esp+4h] [ebp-4h]

  v24 = 0;
  if ( !a1 )
    return -102;
  v4 = *(_DWORD *)(a1 + 124);
  if ( !v4 )
    return -102;
  if ( !*(_DWORD *)v4 )
    return -100;
  if ( !a2 )
    return 0;
  *(_DWORD *)(v4 + 16) = a3;
  v5 = *(_DWORD *)(v4 + 92);
  *(_DWORD *)(v4 + 20) = a2;
  if ( a2 > v5 )
    *(_DWORD *)(v4 + 20) = v5;
  while ( *(_DWORD *)(v4 + 20) )
  {
    if ( !*(_DWORD *)(v4 + 8) )
    {
      v6 = *(_DWORD *)(v4 + 88);
      if ( v6 )
      {
        v7 = 0x4000;
        if ( v6 < 0x4000 )
          v7 = *(_DWORD *)(v4 + 88);
        if ( fseek(*(FILE **)(v4 + 96), *(_DWORD *)(v4 + 60) + *(_DWORD *)(v4 + 104), 0)
          || fread(*(void **)v4, v7, 1u, *(FILE **)(v4 + 96)) != 1 )
        {
          return -1;
        }
        v8 = *(void **)v4;
        v9 = *(_DWORD *)(v4 + 88) - v7;
        *(_DWORD *)(v4 + 60) += v7;
        *(_DWORD *)(v4 + 88) = v9;
        *(_DWORD *)(v4 + 4) = v8;
        *(_DWORD *)(v4 + 8) = v7;
      }
    }
    if ( *(_DWORD *)(v4 + 100) )
    {
      v19 = *(_DWORD *)(v4 + 24);
      v20 = *(_BYTE **)(v4 + 16);
      v25 = inflate((unsigned __int8 **)(v4 + 4), 2);
      v21 = *(_DWORD *)(v4 + 24) - v19;
      v22 = sub_4A2CE0(*(_DWORD *)(v4 + 80), v21, v20);
      v23 = *(_DWORD *)(v4 + 92);
      *(_DWORD *)(v4 + 80) = v22;
      *(_DWORD *)(v4 + 92) = v23 - v21;
      v24 += v21;
      if ( v25 == 1 )
        return v24;
      if ( v25 )
        return v25;
    }
    else
    {
      v10 = *(_DWORD *)(v4 + 8);
      if ( *(_DWORD *)(v4 + 20) < v10 )
        v10 = *(_DWORD *)(v4 + 20);
      for ( i = 0; i < v10; ++i )
        *(_BYTE *)(i + *(_DWORD *)(v4 + 16)) = *(_BYTE *)(i + *(_DWORD *)(v4 + 4));
      v12 = *(_BYTE **)(v4 + 16);
      v13 = sub_4A2CE0(*(_DWORD *)(v4 + 80), v10, v12);
      v14 = *(_DWORD *)(v4 + 92);
      v15 = *(_DWORD *)(v4 + 20);
      v16 = *(_DWORD *)(v4 + 4);
      *(_DWORD *)(v4 + 80) = v13;
      v17 = *(_DWORD *)(v4 + 8) - v10;
      *(_DWORD *)(v4 + 92) = v14 - v10;
      v18 = *(_DWORD *)(v4 + 24);
      *(_DWORD *)(v4 + 8) = v17;
      *(_DWORD *)(v4 + 20) = v15 - v10;
      *(_DWORD *)(v4 + 16) = &v12[v10];
      *(_DWORD *)(v4 + 4) = v10 + v16;
      *(_DWORD *)(v4 + 24) = v10 + v18;
      v24 += v10;
    }
  }
  return v24;
}

/* ---- unztell  0x004A97A0 ---- */
// unztell -- CONFIRMED 85. Returns stream.total_out. Position in the run after unzReadCurrentFile matches minizip.
/* VERIFIED 0x004A97A0.  +124 is pfile_in_zip_read, +24 inside it is
 * stream.total_out (z_stream offset 20 plus the 4-byte read_buffer that
 * precedes the stream in file_in_zip_read_info_s).  Upstream verbatim.
 * DEAD IN RETAIL -- no xrefs; FS_FTell uses its own bookkeeping. */
int __cdecl unztell(int a1)
{
  int v1; // eax

  if ( a1 && (v1 = *(_DWORD *)(a1 + 124)) != 0 )
    return *(_DWORD *)(v1 + 24);
  else
    return -102;
}

/* ---- unzeof  0x004A97C0 ---- */
// unzeof -- CONFIRMED 85. rest_read_uncompressed == 0.
/* VERIFIED 0x004A97C0.  +92 is rest_read_uncompressed, matching the field map
 * on unzOpenCurrentFile.  Upstream verbatim.  DEAD IN RETAIL -- no xrefs. */
int __cdecl unzeof(int a1)
{
  int v1; // eax

  if ( a1 && (v1 = *(_DWORD *)(a1 + 124)) != 0 )
    return *(_DWORD *)(v1 + 92) == 0;
  else
    return -102;
}

/* ---- unzGetLocalExtrafield  0x004A97E0 ---- */
// unzGetLocalExtrafield -- CONFIRMED 82. minizip verbatim.
/*
 * VERIFIED 0x004A97E0 instruction by instruction -- AND IT CONTAINS A RETAIL
 * BUFFER OVERFLOW, which is faithfully reproduced below.  DO NOT WIRE THIS UP.
 *
 * v3 is the caller's len clamped to the bytes remaining (upstream's
 * `read_now`); v7 is the UNCLAMPED remaining.  The fread reads v7 and the
 * function returns v3.  Retail agrees exactly:
 *     4a980d  sub edi, eax        ; edi = size_local_extrafield - pos_...
 *     4a981a  cmp ebx, edi / jbe / mov ebx, edi   ; ebx = min(len, edi)
 *     4a9851  push edi ; ElementSize  <-- reads EDI, the UNCLAMPED count
 *     4a9863  mov eax, ebx            <-- returns EBX, the clamped count
 * so any caller passing a buffer smaller than the entry's local extra field
 * gets that field written past the end of it.  Upstream minizip passes
 * `read_now` to both, i.e. this is an IW/Aspyr edit or a bad merge, not a
 * decompiler artifact.
 *
 * It is harmless today only because it is DEAD IN RETAIL -- no xrefs, and
 * nothing in universal/com_files.c reaches it.  Left matching the binary
 * because the binary wins; flagged this loudly because the day someone calls
 * it, it is a stack smash and it will not look like a zlib bug.
 */
size_t __cdecl unzGetLocalExtrafield(int a1, size_t a2, void *Buffer)
{
  size_t v3; // ebx
  int v5; // esi
  int v6; // eax
  size_t v7; // edi

  v3 = a2;
  if ( !a1 )
    return -102;
  v5 = *(_DWORD *)(a1 + 124);
  if ( !v5 )
    return -102;
  v6 = *(_DWORD *)(v5 + 76);
  v7 = *(_DWORD *)(v5 + 72) - v6;
  if ( !Buffer )
    return *(_DWORD *)(v5 + 72) - v6;
  if ( a2 > v7 )
    v3 = *(_DWORD *)(v5 + 72) - v6;
  if ( !v3 )
    return 0;
  if ( fseek(*(FILE **)(v5 + 96), v6 + *(_DWORD *)(v5 + 68), 0) || fread(Buffer, v7, 1u, *(FILE **)(v5 + 96)) != 1 )
    return -1;
  return v3;
}

/* ---- unzCloseCurrentFile  0x004A9870 ---- */
// unzCloseCurrentFile -- CONFIRMED 92. inflateEnd + frees; called by unzClose, FS_FCloseFile and FS_ShutdownSearchPaths.
/*
 * VERIFIED 0x004A9870 against upstream, and behaviourally 14,493 times -- ALL
 * OF WHICH RETURNED 0.  That is the load-bearing result in this file: the test
 * `!rest_read_uncompressed && crc32 != crc32_wait -> UNZ_CRCERROR (-105)` on
 * the first line is the pak integrity check, it now actually runs (crc32 at
 * 0x004A2CE0 was an AUTO-STUB returning 0 until today, so this comparison was
 * 0 != expected for every completely-read file), and it passes on every entry
 * of every stock pak.
 *
 * +80 crc32, +84 crc32_wait, +92 rest_read_uncompressed, +64
 * stream_initialised -- the map on unzOpenCurrentFile.  `inflateEnd(v3 + 4)`
 * is &pfile->stream, i.e. byte offset 4, as `(_DWORD *)(v3 + 4)` on an int.
 *
 * NOTE the return value is the error, but universal/com_files.c:914 discards
 * it, so a CRC failure will NOT report itself -- it will surface as a corrupt
 * asset somewhere else.  Worth wiring to a Com_Printf one day; not changed
 * here because it is not this file's decision.
 */
int __cdecl unzCloseCurrentFile(int a1)
{
  int v1; // ebx
  int result; // eax
  int v3; // esi
  int v4; // eax

  v1 = 0;
  if ( !a1 )
    return -102;
  v3 = *(_DWORD *)(a1 + 124);
  if ( !v3 )
    return -102;
  if ( !*(_DWORD *)(v3 + 92) && *(_DWORD *)(v3 + 80) != *(_DWORD *)(v3 + 84) )
    v1 = -105;
  free(*(void **)v3);
  v4 = *(_DWORD *)(v3 + 64);
  *(_DWORD *)v3 = 0;
  if ( v4 )
    inflateEnd((_DWORD *)(v3 + 4));
  *(_DWORD *)(v3 + 64) = 0;
  free((void *)v3);
  result = v1;
  *(_DWORD *)(a1 + 124) = 0;
  return result;
}

/* ---- unzGetGlobalComment  0x004A98E0 ---- */
// unzGetGlobalComment -- CONFIRMED 82. Final function of minizip's unzip.c.
/* VERIFIED 0x004A98E0 against upstream minizip unzGetGlobalComment.  The
 * `*a2 = 0;` that happens before the `if (a2)` null check further down looks
 * like a defect and is upstream's own shape -- upstream writes `*szComment =
 * '\0'` inside `if (uReadThis > 0)` and only null-checks szComment for the
 * trailing terminator.  Left alone.  +8 is gi.size_comment, +28 is
 * central_pos, and 22 is sizeof(end of central directory record).
 * DEAD IN RETAIL -- no xrefs. */
size_t __cdecl unzGetGlobalComment(int a1, _BYTE *a2, size_t ElementSize)
{
  size_t v5; // eax
  size_t v6; // edi
  size_t v7; // esi

  if ( !a1 )
    return -102;
  v5 = *(_DWORD *)(a1 + 8);
  v6 = ElementSize;
  if ( ElementSize > v5 )
    v6 = v5;
  if ( fseek(*(FILE **)a1, *(_DWORD *)(a1 + 28) + 22, 0) )
    return -1;
  if ( v6 )
  {
    *a2 = 0;
    if ( fread(a2, v6, 1u, *(FILE **)a1) != 1 )
      return -1;
  }
  if ( a2 )
  {
    v7 = *(_DWORD *)(a1 + 8);
    if ( ElementSize > v7 )
      a2[v7] = 0;
  }
  return v6;
}

/* ---- zlibVersion  0x004A9950 ---- */
// zlibVersion -- CONFIRMED 96. Returns "1.1.4". Pins the bundled zlib at 1.1.4, so the whole inflate/inftrees/infblock block can be matched against that exact release.
/* VERIFIED 0x004A9950.  Two instructions, returning the "1.1.4" literal.  This
 * is the anchor the whole zlib verification rests on: it dates the bundled
 * library exactly, so inflate.c/infblock.c/infcodes.c/inffast.c/inftrees.c/
 * infutil.c and this unzip.c can be diffed against a specific public release
 * rather than guessed at.  DEAD IN RETAIL -- no xrefs. */
const char *zlibVersion()
{
  return "1.1.4";
}

/* ---- zError  0x004A9960 ---- */
// zError -- CONFIRMED 92. Indexes the z_errmsg table backwards from z_errmsg, i.e. z_errmsg[1-err]. zutil.c verbatim.
/*
 * VERIFIED 0x004A9960 against zutil.c's `return ERR_MSG(err);` =
 * z_errmsg[Z_NEED_DICT - err], with the compiler having folded the constant
 * into the base address -- IDA's `z_errmsg` label is at 0x00571BA8, which is
 * &z_errmsg[2], so retail's `&z_errmsg - err` is z_errmsg[2 - err].  DEAD IN
 * RETAIL -- no xrefs, and none in this tree either.
 *
 * FIXED.  The table now exists at its real base (0x00571BA0, ten `const char *`
 * -- zlib/zlib_rdata.c), so the folded constant is spelled out again rather
 * than being carried in the symbol's address.  Before that it indexed off the
 * FRONT of `extern int z_errmsg;` and returned neighbouring globals as strings.
 * The cast stays because cod1_globals.h is included here and still declares the
 * name an int; the storage behind it is the 40-byte table.
 */
const char *__cdecl zError(int a1)
{
  return ((const char **)&z_errmsg)[2 - a1];
}

/* ---- zcalloc  0x004A9980 ---- */
// zcalloc -- CONFIRMED 92. DIVERGENCE: stock zutil.c calls malloc; CoD routes zlib allocation through Z_MallocInternal so pak decompression shows up in the zone.
/*
 * VERIFIED 0x004A9980.  NOT UPSTREAM -- zutil.c's zcalloc calls malloc; this
 * routes through Z_MallocInternal so pak decompression is visible in the zone.
 * a1 is opaque and unused; the product a3*a2 is commutative, so the recovered
 * register order of items/size cannot be wrong in any way that matters.
 * Reached only as a function POINTER, installed by inflateInit2_ into
 * z_stream.zalloc and called as (opaque, items, size).  Exercised on every one
 * of the 14,493 reads -- it allocates the 32 KB inflate window and the 1,440-
 * entry huft pool each time a pak file is opened.
 */
int *__cdecl zcalloc(int a1, int a2, int a3)
{
  return Z_MallocInternal(a3 * a2);
}

/* ---- zcfree  0x004A99A0 ---- */
// zcfree -- CONFIRMED 92. Plain free().
/*
 * VERIFIED 0x004A99A0.  DIVERGENCE WORTH KNOWING, and it is retail's: zcalloc
 * above allocates through Z_MallocInternal but zcfree releases through the CRT
 * free().  If Z_MallocInternal is ever anything but a malloc wrapper, this
 * pair does not match.  It is what the binary does, so it stays; noted because
 * it is exactly the kind of asymmetry that reads as a typo later.
 * Reached only as the z_stream.zfree function pointer, (opaque, block).
 */
void __cdecl zcfree(int a1, void *Block)
{
  free(Block);
}

