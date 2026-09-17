/*
 * Machine-translated from Call of Duty 1.1 (Windows, CoDMP.exe).
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/zlib/inflate.c
 *
 * Retail range 0x004A45C0-0x004A8730, 44 functions: 44 translated, 0 stubbed.
 *
 * @fidelity: likely
 * @fidelity-default: unreviewed
 *
 * `@fidelity-default: unreviewed` ADDED: this unit's per-function confidence
 * is not uniform (see SECOND PASS below) and the file-level `likely` was
 * being inherited by every un-banded function, including the trees.c/deflate
 * half that is explicitly UNVERIFIED and dead.  `_tr_flush_block` (0x004A7EF0,
 * deflate-side, unreachable -- see the dead-code note below) was flagged by
 * uninit_scan.py as "claims likely, does not compile clean" purely because
 * of this inherited default; the C4700 there is real (an un-banded raw body,
 * left alone deliberately, see the dead-code note) but the confidence claim
 * was spurious.  The 22 inflate-side functions are unaffected: each carries
 * its own inline `VERIFIED` band, which overrides this default.
 *
 * TAKEN OUT OF THE GENERATOR DELIBERATELY.  Most of this unit is still raw
 * machine output, but the inflate side of it has been reconstructed by hand and
 * regenerating the file would silently put the server back to reading zeroes
 * out of every pak.  What was done, and must survive:
 *
 *   - inflate_blocks (0x004A46C0), inflate_codes (0x004A5300) and inflate_fast
 *     (0x004A5A10) were AUTO-STUBBED `return 0;`.  That is the entire deflate
 *     decoder, so inflate() returned Z_OK having produced nothing and every
 *     FS_ReadFile out of a pak came back zero-filled.  Their recovered bodies
 *     are now live.
 *   - the six zlib constant tables they need (inflate_mask, inflate_border,
 *     cplens, cplext, cpdist, cpdext) are defined below with real types.  The
 *     global generator saw them as `int inflate_border;` / `unsigned char
 *     zlib_cplens;` -- one scalar each -- which is both why the three bodies
 *     would not compile and why huft_build would have been fed all-zero
 *     length/distance bases even if they had.
 *   - huft_build's two split stack arrays were restored; see the note in it.
 *   - inflate_trees_fixed was rewritten: the recovered form stores through an
 *     uninitialised pointer (a lost ECX argument) and hands back the zeroed
 *     placeholders for the two 4KB fixed-Huffman tables.
 *
 * Verified by decompressing 1,040 entries across pak0 and pak5 and comparing
 * byte-for-byte against Python's zipfile -- all three deflate block types
 * (stored, fixed, dynamic), sizes from 10 bytes to 6.1 MB.
 *
 * ---------------------------------------------------------------------------
 * SECOND PASS -- VERIFICATION.  Every inflate-side function now carries a
 * `VERIFIED <addr>` note saying what was actually checked.  Three kinds of
 * evidence, and it is worth knowing which is which:
 *
 *  1. BEHAVIOURAL, and now much wider than the run above: all NINE stock
 *     retail paks, 14,493 non-empty entries, read through this file plus
 *     zlib/unzip.c and zlib/unz_compat.c in the exact call sequence
 *     universal/com_files.c uses, every one matching its central-directory
 *     CRC.  Sizes 1 byte to 45 MB.  This is the first pass in which the CRC
 *     comparison inside unzCloseCurrentFile has actually run -- crc32
 *     (0x004A2CE0) was an AUTO-STUB returning 0 until today -- and it now
 *     passes on every entry.
 *
 *  2. MECHANICAL TABLE DIFFS, because this file is mostly constant tables and
 *     eyeballing them is not verification:
 *       - the six tables below (inflate_mask, inflate_border, cplens, cplext,
 *         cpdist, cpdext) were read back out of retail .data and compared
 *         element by element against upstream zlib 1.1.4.  All six are
 *         identical in all three places: retail == upstream == this file.
 *       - the FIXED-HUFFMAN TREES that inf_build_fixed constructs were dumped
 *         and compared against retail's precompiled fixed_tl (0x00571C58,
 *         4096 bytes) and fixed_td (0x00572C58, 256 bytes).  SHA-1 identical
 *         over all 4,352 bytes, bl/bd = 9/5 matching 0x00571C4C/0x00571C50.
 *         That verifies huft_build itself, not just the tables.
 *
 *  3. LINE-BY-LINE against upstream zlib 1.1.4 for the small functions and
 *     for inflate_flush.
 *
 * ONE REAL DEFECT FIXED: huft_build read an uninitialised local (`v33 = v65`).
 * See the note on v65 there -- it was a faithful rendering of upstream's
 * uninitialised `inflate_huft r`, pinned to 0, no behaviour change (the fixed
 * trees are byte-identical before and after).  uninit_scan.py now reports one
 * remaining hit in this file, _tr_flush_block, which is deflate-side.
 *
 * WHAT IS *NOT* VERIFIED: everything from 0x004A6D40 on.  That block is not
 * inflate at all -- it is zlib's trees.c (_tr_init, _tr_stored_block,
 * _tr_align, _tr_flush_block, _tr_tally and their statics), the COMPRESSION
 * side, emitted into this unit by the generator.  It is dead in the retail
 * binary: its only entry is compress() at 0x004A2CC0, which nothing calls.
 * Left alone deliberately -- see the note on _tr_init below.
 *
 * WHICH IS WHY THE MARKER ABOVE STILL SAYS `likely` AND NOT `verified`.  It is
 * per-unit, and this unit is two units: the inflate half (0x004A45C0 through
 * 0x004A6C00, 22 functions, all carrying a VERIFIED note) and the trees.c half
 * (0x004A6D40 through 0x004A8730, unchecked, five of them still AUTO-STUBBED).
 * Read the marker as the weaker of the two, not as a verdict on the first.
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
 * ==========================================================================
 * EVERY REMAINING STUB IN THIS UNIT IS DEAD CODE, 2026-08-19.  Verified, not
 * assumed.  Do not "fix" them.
 * ==========================================================================
 *
 * CoDMP.exe links all of zlib but only ever INFLATES -- it reads .iwd/.pk3
 * archives and never writes one.  The whole deflate/trees half is therefore
 * unreachable, and the five `sub_4A*` stubs plus inflateSync are all in it.
 *
 * The call graph terminates, which is the proof:
 *
 *     sub_4A7980, sub_4A79F0, sub_4A8170  <- _tr_flush_block
 *     sub_4A8730                          <- _tr_stored_block
 *     _tr_flush_block   <- deflate_stored, deflate_slow, deflate_fast
 *     _tr_stored_block  <- deflate, _tr_flush_block
 *     deflate_stored    <- deflateParams, deflate
 *     deflate           <- compress2  <- compress  <- NOTHING
 *
 * and deflate_slow, deflate_fast and deflateParams have no callers at all.
 *
 * CHECKED FOR THE INLINING TRAP TOO, because "zero xrefs" does NOT mean dead
 * in this tree -- six EffectsCore functions with no xrefs turned out to be
 * live and inlined.  A scan of every loaded segment for little-endian pointer
 * literals finds none to compress, compress2, deflate, deflateEnd,
 * deflateInit_, deflateInit2_, deflateParams, _tr_tally or inflateSync.
 * deflate_fast and deflate_slow DO appear, at 0x0054136C..0x005413A8 -- but
 * that is zlib's own `configuration_table`, one entry per compression level,
 * and the only thing that walks it is deflate(), which nothing calls.  A
 * pointer in a table a dead function reads does not make its target live.
 *
 * _tr_tally has neither xrefs nor pointer literals: it is inlined into
 * _tr_flush_block, which is itself dead.
 *
 * inflateSync is a public zlib entry point that this image never calls.
 *
 * zlib is third-party under CLAUDE.md 4 in any case; if a future build ever
 * needs the deflate half, PORT IT FROM THE ZLIB RELEASE rather than
 * decompiling it -- the source is public and permissive, and that is the same
 * rule that applies to the RTCW/Q3 code elsewhere in this tree.
 */


/* Functions this unit calls that live elsewhere.  Declared
 * unprototyped and int-returning: C89 makes that compatible with any
 * definition, and each unit is compiled on its own, so this cannot
 * conflict with the real signature in the defining unit.  Anything a
 * real header already declares is excluded. */
/* REAL prototypes, not `extern int f();`.  An empty parameter list switches
 * argument checking off entirely and has produced nine faults in this tree.
 * All three are only ever address-taken here (stored into z_stream.zalloc /
 * .zfree and into inflate_blocks_state.checkfn), but the declarations are
 * still written out so a future call site is checked.
 *
 *   sub_4A2AF0 = adler32 (0x004A2AF0, universal/memorytree.cpp), called
 *                through checkfn as (check, buf, len) -- see inflate_flush.
 *   zcalloc/zcfree are defined in zlib/unzip.c; the signatures below are
 *                that file's, exactly. */
extern int  __cdecl sub_4A2AF0( int adler, unsigned char *buf, unsigned int len );
extern int *__cdecl zcalloc( int opaque, int items, int size );
extern void __cdecl zcfree( int opaque, void *block );

/* Defined below at its address-order place, declared here because both of its
 * call sites (_tr_init and sub_4A6E20) come first in the file.  Without this
 * they would be implicit declarations, which decorate as plain `_sub_4A6DB0`
 * and would have kept binding to the link stub -- the silent shadow. */
short *__cdecl sub_4A6DB0( int unused, int s );

/*
 * ---------------------------------------------------------------------------
 * zlib 1.1.4 constant tables.
 *
 * These live in retail .data and the recovered code referenced them by
 * address: inflate_mask at 0x00571C08, border at 0x005412D0, cplens/cplext at
 * 0x005410D8/0x00541158, cpdist/cpdext at 0x005411D8/0x00541250.  The global
 * generator has no way to know a `inflate_border` is nineteen elements wide, so
 * it emitted each as a single zeroed scalar -- which is why the three block
 * decoders below could not even compile (`subscript requires array or pointer
 * type`) and why huft_build, had they compiled, would have been handed all-zero
 * length/distance bases.
 *
 * Every value here was read back out of the loaded image (idc.get_wide_dword
 * over each range) and matches zlib 1.1.4's inftrees.c / infutil.c exactly.
 * ---------------------------------------------------------------------------
 */

/* infutil.c: inflate_mask[n] = low n bits set.  Retail 0x00571C08. */
static const unsigned int inflate_mask[17] = {
	0x00000000u, 0x00000001u, 0x00000003u, 0x00000007u,
	0x0000000fu, 0x0000001fu, 0x0000003fu, 0x0000007fu,
	0x000000ffu, 0x000001ffu, 0x000003ffu, 0x000007ffu,
	0x00000fffu, 0x00001fffu, 0x00003fffu, 0x00007fffu,
	0x0000ffffu
};

/* infblock.c: order of the bit length code lengths.  Retail 0x005412D0. */
static const unsigned int inflate_border[19] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

/* inftrees.c: copy lengths and their extra bits.  Retail 0x005410D8/0x00541158.
 * The trailing 0,112,112 in cplext are zlib's "invalid code" markers. */
static const unsigned int cplens[31] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};
static const unsigned int cplext[31] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 112, 112
};

/* inftrees.c: copy offsets and their extra bits.  Retail 0x005411D8/0x00541250. */
static const unsigned int cpdist[30] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49,
	65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073,
	4097, 6145, 8193, 12289, 16385, 24577
};
static const unsigned int cpdext[30] = {
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4,
	5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
	11, 11, 12, 12, 13, 13
};

/*
 * The fixed-Huffman trees.
 *
 * Retail keeps them as precompiled data -- fixed_bl/fixed_bd at 0x00571C4C and
 * 0x00571C50 (9 and 5), fixed_tl at 0x00571C58 (512 inflate_hufts, 0x1000
 * bytes) and fixed_td at 0x00572C58 -- so its inflate_trees_fixed just hands
 * the four values back.  Those two tables are 4KB of initialised data the
 * global generator emitted as one zero byte each.
 *
 * Rather than paste 544 recovered table entries, this builds them once with
 * huft_build, which is exactly how zlib generated inffixed.h in the first
 * place (its BUILDFIXED path).  The result is identical; the first entries were
 * spot-checked against the image ({96,7,256} {0,8,80} {0,8,16} {84,8,115} for
 * fixed_tl, {80,5,1} {87,5,257} {83,5,17} for fixed_td) and match.
 *
 * DIVERGENCE FROM RETAIL: built at first use instead of loaded from .data.
 */
/* The fixed trees need exactly 544 hufts (zlib's FIXEDH, and the reason retail's
 * fixed_tl is 0x1000 bytes = 512 entries and fixed_td 32).  The pool is sized at
 * MANY instead, because huft_build's own overflow guard is the hard-coded
 * `if (*hn + (1 << j) > 0x5A0)` -- 1440 -- rather than anything derived from the
 * pool it was handed, so a smaller buffer would not be checked against. */
#define INF_MANY 1440

static int          inf_fixed_built = 0;
static unsigned int inf_fixed_bl;
static unsigned int inf_fixed_bd;
static void        *inf_fixed_tl;
static void        *inf_fixed_td;
static unsigned int inf_fixed_pool[2 * INF_MANY];   /* inflate_huft is 8 bytes */

/* Recovered argument order is (m, b, n, s, d, e, t, hp, hn, v) -- NOT zlib's
 * (b, n, s, d, e, t, m, hp, hn, v).  Pinned by both call sites in
 * inflate_trees_bits and inflate_trees_dynamic below; see the note there. */
extern int __cdecl huft_build(
        unsigned int *m, _DWORD *b, unsigned int n, unsigned int s,
        int d, int e, _DWORD *t, int hp, unsigned int *hn, unsigned int *v);

static void inf_build_fixed( void )
{
	unsigned int c[288];
	unsigned int v[288];
	unsigned int hn;
	int k;

	if ( inf_fixed_built ) {
		return;
	}

	hn = 0;

	/* literal/length tree: 144x8, 112x9, 24x7, 8x8 */
	for ( k = 0; k < 144; k++ ) { c[k] = 8; }
	for ( ; k < 256; k++ )      { c[k] = 9; }
	for ( ; k < 280; k++ )      { c[k] = 7; }
	for ( ; k < 288; k++ )      { c[k] = 8; }
	inf_fixed_bl = 9;
	huft_build( &inf_fixed_bl, (_DWORD *)c, 288u, 257u,
				(int)cplens, (int)cplext, (_DWORD *)&inf_fixed_tl,
				(int)inf_fixed_pool, &hn, v );

	/* distance tree: 30x5 */
	for ( k = 0; k < 30; k++ ) { c[k] = 5; }
	inf_fixed_bd = 5;
	huft_build( &inf_fixed_bd, (_DWORD *)c, 30u, 0u,
				(int)cpdist, (int)cpdext, (_DWORD *)&inf_fixed_td,
				(int)inf_fixed_pool, &hn, v );

	inf_fixed_built = 1;
}

/* ---- inflate_blocks_reset  0x004A45C0 ---- */
// inflate_blocks_reset -- CONFIRMED 92. zlib 1.1.4 infblock.c. Frees the codes/trees for modes 4/5/6 then resets bitb/bitk/read/write. Callers: inflate_blocks_new, inflateReset, inflate.
/*
 * VERIFIED 0x004A45C0 against zlib 1.1.4 infblock.c inflate_blocks_reset.
 * Every offset checked against inflate_blocks_state: mode 0, sub.trees.blens
 * 12, sub.decode.codes 4, last 24, bitk 28, bitb 32, hufts 36, window 40, end
 * 44, read 48, write 52, checkfn 56, check 60 -- 64 bytes, which is exactly
 * the ZALLOC(1,64) in inflate_blocks_new. Modes 4/5/6 are BTREE/DTREE/CODES
 * in zlib's enum, so the two frees are upstream's. ARGUMENT ORDER IS REGISTER
 * ORDER: (c, z, s), not zlib's (s, z, c). All three call sites
 * (inflate_blocks_new, inflateReset, inflate) pass it that way; do not
 * restore zlib's order without fixing them in the same change.
 */
int (__cdecl *__cdecl inflate_blocks_reset(
        _DWORD *a1,
        int a2,
        _DWORD *a3))(_DWORD, _DWORD, _DWORD)
{
  int v3; // eax
  int (__cdecl *result)(_DWORD, _DWORD, _DWORD); // eax

  if ( a1 )
    *a1 = a3[15];
  if ( *a3 == 4 || *a3 == 5 )
    (*(void (__cdecl **)(_DWORD, _DWORD))(a2 + 36))(*(_DWORD *)(a2 + 40), a3[3]);
  if ( *a3 == 6 )
    (*(void (__cdecl **)(_DWORD, _DWORD))(a2 + 36))(*(_DWORD *)(a2 + 40), a3[1]);
  v3 = a3[10];
  a3[13] = v3;
  a3[12] = v3;
  result = (int (__cdecl *)(_DWORD, _DWORD, _DWORD))a3[14];
  *a3 = 0;
  a3[7] = 0;
  a3[8] = 0;
  if ( result )
  {
    result = (int (__cdecl *)(_DWORD, _DWORD, _DWORD))result(0, 0, 0);
    a3[15] = result;
    *(_DWORD *)(a2 + 48) = result;
  }
  return result;
}

/* ---- inflate_blocks_new  0x004A4630 ---- */
// inflate_blocks_new -- CONFIRMED 95. ZALLOC(1, sizeof(inflate_blocks_state)=64) then ZALLOC(sizeof(inflate_huft)=8, MANY=1440) for the huft pool. The 8x1440 pair is a unique-constant anchor for zlib 1.1.x.
/*
 * VERIFIED 0x004A4630 against upstream. The three allocations are ZALLOC(1,
 * 64) for the state, ZALLOC(8, 1440) for the huft pool --
 * sizeof(inflate_huft) x MANY, a unique-constant anchor for zlib 1.1.x -- and
 * ZALLOC(1, 1<<wbits) for the window. Exercised on all 14,493 stock-pak
 * reads.
 */
_DWORD *__cdecl inflate_blocks_new(int a1, int a2, int a3)
{
  _DWORD *v4; // esi
  int v5; // eax
  int v7; // eax

  v4 = (_DWORD *)(*(int (__cdecl **)(_DWORD, int, int))(a1 + 32))(*(_DWORD *)(a1 + 40), 1, 64);
  if ( !v4 )
    return 0;
  v5 = (*(int (__cdecl **)(_DWORD, int, int))(a1 + 32))(*(_DWORD *)(a1 + 40), 8, 1440);
  v4[9] = v5;
  if ( !v5 )
  {
    (*(void (__cdecl **)(_DWORD, _DWORD *))(a1 + 36))(*(_DWORD *)(a1 + 40), v4);
    return 0;
  }
  v7 = (*(int (__cdecl **)(_DWORD, int, int))(a1 + 32))(*(_DWORD *)(a1 + 40), 1, a2);
  v4[10] = v7;
  if ( !v7 )
  {
    (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), v4[9]);
    (*(void (__cdecl **)(_DWORD, _DWORD *))(a1 + 36))(*(_DWORD *)(a1 + 40), v4);
    return 0;
  }
  v4[11] = a2 + v7;
  v4[14] = a3;
  *v4 = 0;
  inflate_blocks_reset(0, a1, v4);
  return v4;
}

/* ---- inflate_blocks  0x004A46C0 ---- */
/*
 * VERIFIED 0x004A46C0 BEHAVIOURALLY, which for the block-type dispatcher is
 * the strongest test available: 14,493 stock-pak entries decompressed and
 * CRC-checked, covering all three deflate block types (stored, fixed,
 * dynamic) at sizes from 1 byte to 45 MB. A wrong constant in any state would
 * corrupt one of them.
 */
/*
 * PROMOTED from AUTO-STUBBED.  The recovered body was sound; it failed to
 * compile only because it subscripts two constant tables (`inflate_border` =
 * border, `inflate_mask` = inflate_mask) that the global generator had emitted
 * as scalar ints.  With those given real types at the top of this file the body
 * builds unchanged.  Only two things were touched:
 *   - the two table names, and
 *   - the fixed-block case, which read the precompiled fixed trees straight out
 *     of .data (retail inlines inflate_trees_fixed here).
 *
 * Argument order (a1 = s, a2 = z, a3 = r) matches zlib and is pinned by the
 * call in inflate():  inflate_blocks(state->blocks, z, r).
 *
 * While it returned 0 this function was the whole bug: inflate() treats Z_OK
 * from it as "no progress but no error", so inflate returned Z_OK having
 * produced nothing and every pak read handed back a zero-filled buffer.
 */
int __cdecl inflate_blocks(_DWORD *a1, unsigned __int8 **a2, int a3)
{
  unsigned int v4; // esi
  unsigned __int8 **v5; // edi
  unsigned int v6; // eax
  char *v7; // ecx
  int v8; // eax
  int v9; // ecx
  unsigned int v10; // ebx
  unsigned __int8 *v11; // eax
  int v12; // edx
  unsigned int v13; // esi
  int v14; // ecx
  int v15; // eax
  int v16; // edx
  int v17; // ecx
  char *v18; // edx
  unsigned int v19; // eax
  unsigned int v20; // ecx
  int v21; // eax
  int v22; // eax
  char *v23; // ecx
  unsigned int v24; // edx
  char *v25; // ebx
  unsigned int v26; // eax
  unsigned int v27; // eax
  unsigned int v28; // ecx
  int v29; // edx
  unsigned int v30; // eax
  unsigned int v31; // ecx
  unsigned int v32; // eax
  int v33; // eax
  int v34; // edx
  unsigned int v35; // eax
  unsigned int v36; // ecx
  int v37; // eax
  int v38; // ebx
  unsigned int v39; // ecx
  int v40; // edx
  int v41; // edx
  unsigned int v42; // ecx
  unsigned int v43; // edx
  int v44; // edx
  unsigned int v45; // ebx
  unsigned int v46; // eax
  int v47; // ecx
  int v48; // eax
  unsigned int v49; // eax
  int v50; // eax
  int v51; // ebx
  int v52; // eax
  char v53; // dl
  int v54; // ecx
  int v55; // edx
  unsigned __int8 *v56; // ebx
  unsigned __int8 *v57; // edx
  int v58; // eax
  unsigned int v59; // eax
  char *v60; // ecx
  int v61; // eax
  unsigned __int8 *v62; // edx
  unsigned __int8 *v63; // ecx
  unsigned __int8 *v65; // esi
  unsigned __int8 *v66; // ebx
  unsigned __int8 *v67; // ebx
  int v68; // edx
  unsigned __int8 *v69; // esi
  unsigned __int8 *v70; // ebx
  int v71; // edx
  unsigned __int8 *v72; // ebx
  unsigned __int8 *v73; // esi
  unsigned __int8 *v74; // edx
  unsigned __int8 *v75; // ebx
  int v76; // edx
  unsigned __int8 *v77; // esi
  unsigned __int8 *v78; // ebx
  _DWORD *v79; // [esp-18h] [ebp-50h]
  _DWORD *v80; // [esp-10h] [ebp-48h]
  int v81; // [esp-4h] [ebp-3Ch]
  int v82; // [esp-4h] [ebp-3Ch]
  int v83; // [esp-4h] [ebp-3Ch]
  unsigned __int8 *v84; // [esp+10h] [ebp-28h]
  unsigned __int8 *v85; // [esp+14h] [ebp-24h]
  char *v86; // [esp+18h] [ebp-20h]
  unsigned int v87; // [esp+1Ch] [ebp-1Ch]
  int v88; // [esp+1Ch] [ebp-1Ch]
  int v89; // [esp+20h] [ebp-18h] BYREF
  unsigned int v90; // [esp+24h] [ebp-14h] BYREF
  unsigned int v91; // [esp+28h] [ebp-10h]
  unsigned int v92; // [esp+2Ch] [ebp-Ch]
  int v93; // [esp+30h] [ebp-8h] BYREF
  int v94; // [esp+34h] [ebp-4h] BYREF
  unsigned int v95; // [esp+3Ch] [ebp+4h]
  int v96; // [esp+44h] [ebp+Ch]

  v4 = a1[7];
  v5 = a2;
  v85 = *a2;
  v6 = a1[12];
  v84 = a2[1];
  v7 = (char *)a1[13];
  v95 = a1[8];
  v89 = v4;
  v86 = v7;
  if ( (unsigned int)v7 >= v6 )
    v8 = a1[11] - (_DWORD)v7;
  else
    v8 = v6 - (_DWORD)v7 - 1;
  v9 = *a1;
  v87 = v8;
  while ( 2 )
  {
    v10 = v95;
    v11 = v85;
    switch ( v9 )
    {
      case 0:
        if ( v4 >= 3 )
          goto LABEL_9;
        do
        {
          if ( !v84 )
            goto LABEL_109;
          --v84;
          v12 = *v11 << v4;
          v4 += 8;
          a3 = 0;
          v89 = v4;
          v10 |= v12;
          ++v11;
          v95 = v10;
          v85 = v11;
        }
        while ( v4 < 3 );
LABEL_9:
        a1[6] = v10 & 1;
        switch ( (unsigned __int8)(v10 & 7) >> 1 )
        {
          case 0:
            v13 = v4 - 3;
            v14 = v13 & 7;
            v89 = v13 - v14;
            *a1 = 1;
            v95 = v10 >> 3 >> v14;
            goto LABEL_93;
          case 1:
            /* Fixed-Huffman block.  Retail inlines inflate_trees_fixed here and
             * reads the precompiled trees straight out of .data (fixed_bl at
             * 0x00571C4C = 9, fixed_bd at 0x00571C50 = 5, fixed_tl 0x00571C58,
             * fixed_td 0x00572C58).  Those two tables are 4KB of initialised
             * data that the global generator flattened to a single zero byte, so
             * they are rebuilt once instead -- see inf_build_fixed above.
             *
             * inflate_codes_new's recovered order is (z, bl, bd, tl, td), NOT
             * zlib's (bl, bd, tl, td, z): this call site is what pins it. */
            inf_build_fixed();
            v15 = inflate_codes_new((int)v5, (char)inf_fixed_bl, (char)inf_fixed_bd,
                                    (int)inf_fixed_tl, (int)inf_fixed_td);
            a1[1] = v15;
            if ( !v15 )
            {
              v83 = -4;
              goto LABEL_95;
            }
            v95 = v10 >> 3;
            v89 = v4 - 3;
            *a1 = 6;
            break;
          case 2:
            v95 = v10 >> 3;
            v89 = v4 - 3;
            *a1 = 3;
            goto LABEL_93;
          case 3:
            *a1 = 9;
            v5[6] = "invalid block type";
            a1[8] = v95 >> 3;
            a1[7] = v4 - 3;
            v5[1] = v84;
            v83 = -3;
            goto LABEL_96;
          default:
            goto LABEL_93;
        }
        goto LABEL_93;
      case 1:
        if ( v4 >= 0x20 )
          goto LABEL_17;
        do
        {
          if ( !v84 )
            goto LABEL_109;
          --v84;
          v16 = *v11 << v4;
          v4 += 8;
          a3 = 0;
          v10 |= v16;
          ++v11;
          v95 = v10;
          v85 = v11;
        }
        while ( v4 < 0x20 );
LABEL_17:
        if ( ~v10 >> 16 != (unsigned __int16)v10 )
        {
          *a1 = 9;
          v5[6] = "invalid stored block lengths";
          v83 = -3;
          goto LABEL_95;
        }
        a1[1] = (unsigned __int16)v10;
        v17 = a1[1];
        v89 = 0;
        v95 = 0;
        if ( v17 )
          *a1 = 2;
        else
          *a1 = a1[6] != 0 ? 7 : 0;
        goto LABEL_93;
      case 2:
        if ( !v84 )
          goto LABEL_109;
        if ( !v87 )
        {
          v18 = (char *)a1[11];
          if ( v86 != v18
            || (v19 = a1[12], v20 = a1[10], v20 == v19)
            || ((v86 = (char *)a1[10], v20 >= v19) ? (v21 = (int)&v18[-v20]) : (v21 = v19 - v20 - 1), (v87 = v21) == 0) )
          {
            a1[13] = v86;
            v22 = inflate_flush(a1, v5, a3);
            v23 = (char *)a1[13];
            v24 = a1[12];
            v96 = v22;
            v86 = v23;
            v87 = (unsigned int)v23 >= v24 ? a1[11] - (_DWORD)v23 : v24 - (_DWORD)v23 - 1;
            v25 = (char *)a1[11];
            if ( v23 == v25 )
            {
              v26 = a1[10];
              if ( v24 != v26 )
              {
                v23 = (char *)a1[10];
                v86 = v23;
                if ( v26 >= v24 )
                  v87 = (unsigned int)&v25[-v26];
                else
                  v87 = v24 - v26 - 1;
              }
            }
            if ( !v87 )
            {
              a1[8] = v95;
              a1[7] = v4;
              v65 = *v5;
              v66 = v5[2];
              v5[1] = v84;
              *v5 = v85;
              v5[2] = &v66[v85 - v65];
              a1[13] = v23;
              return inflate_flush(a1, v5, v96);
            }
          }
        }
        v27 = a1[1];
        a3 = 0;
        if ( v27 > (unsigned int)v84 )
          v27 = (unsigned int)v84;
        if ( v27 > v87 )
          v27 = v87;
        qmemcpy(v86, v85, v27);
        v86 += v27;
        v28 = a1[1] - v27;
        v85 += v27;
        v84 -= v27;
        v87 -= v27;
        a1[1] = v28;
        if ( !v28 )
          *a1 = a1[6] != 0 ? 7 : 0;
        goto LABEL_93;
      case 3:
        if ( v4 >= 0xE )
          goto LABEL_47;
        do
        {
          if ( !v84 )
            goto LABEL_110;
          --v84;
          v29 = *v11 << v4;
          v4 += 8;
          a3 = 0;
          v10 |= v29;
          ++v11;
          v95 = v10;
          v85 = v11;
        }
        while ( v4 < 0xE );
LABEL_47:
        v30 = v10 & 0x3FFF;
        v31 = v10 & 0x1F;
        a1[1] = v30;
        if ( v31 > 0x1D || (v32 = (v30 >> 5) & 0x1F, v32 > 0x1D) )
        {
          *a1 = 9;
          v5[6] = "too many length or distance symbols";
LABEL_105:
          a1[8] = v95;
          a1[7] = v4;
          v67 = v5[2];
          v5[1] = v84;
          v68 = v85 - *v5;
          *v5 = v85;
          v5[2] = &v67[v68];
          a1[13] = v86;
          return inflate_flush(a1, v5, -3);
        }
        v33 = ((int (__cdecl *)(unsigned __int8 *, unsigned int, int))v5[8])(v5[10], v32 + v31 + 258, 4);
        a1[3] = v33;
        if ( !v33 )
        {
          a1[8] = v95;
          a1[7] = v4;
          v5[1] = v84;
          v83 = -4;
          goto LABEL_96;
        }
        v11 = v85;
        v10 >>= 14;
        v95 = v10;
        v4 -= 14;
        a1[2] = 0;
        *a1 = 4;
LABEL_51:
        if ( a1[2] < (unsigned int)((a1[1] >> 10) + 4) )
        {
          while ( v4 >= 3 )
          {
LABEL_57:
            *(_DWORD *)(a1[3] + 4 * inflate_border[a1[2]]) = v10 & 7;
            v35 = a1[2] + 1;
            a1[2] = v35;
            v10 >>= 3;
            v4 -= 3;
            v95 = v10;
            if ( v35 >= (a1[1] >> 10) + 4 )
              goto LABEL_58;
            v11 = v85;
          }
          while ( v84 )
          {
            --v84;
            v34 = *v11 << v4;
            v4 += 8;
            a3 = 0;
            v10 |= v34;
            ++v11;
            v95 = v10;
            v85 = v11;
            if ( v4 >= 3 )
              goto LABEL_57;
          }
LABEL_110:
          a1[8] = v95;
          a1[7] = v4;
          v70 = v5[2];
          v71 = v85 - *v5;
          *v5 = v85;
          v5[2] = &v70[v71];
          v5[1] = 0;
          a1[13] = v86;
          return inflate_flush(a1, v5, a3);
        }
LABEL_58:
        if ( a1[2] < 0x13u )
        {
          do
          {
            *(_DWORD *)(a1[3] + 4 * inflate_border[a1[2]]) = 0;
            v36 = a1[2] + 1;
            a1[2] = v36;
          }
          while ( v36 < 0x13 );
        }
        v81 = a1[9];
        v80 = (_DWORD *)a1[3];
        a1[4] = 7;
        v37 = inflate_trees_bits((int)v5, v80, a1 + 4, a1 + 5, v81);
        v38 = v37;
        if ( v37 )
        {
          if ( v37 == -3 )
          {
            ((void (__cdecl *)(unsigned __int8 *, _DWORD))v5[9])(v5[10], a1[3]);
            *a1 = 9;
          }
          a1[8] = v95;
          a1[7] = v4;
          v69 = *v5;
          v5[1] = v84;
          v5[2] += v85 - v69;
          *v5 = v85;
          v83 = v38;
          goto LABEL_98;
        }
        v10 = v95;
        a1[2] = 0;
        v11 = v85;
        *a1 = 5;
LABEL_62:
        if ( a1[2] < ((a1[1] >> 5) & 0x1F) + (a1[1] & 0x1Fu) + 258 )
        {
          while ( 1 )
          {
            v39 = a1[4];
            if ( v4 < v39 )
              break;
LABEL_68:
            v41 = a1[5] + 8 * (v10 & inflate_mask[v39]);
            v42 = *(unsigned __int8 *)(v41 + 1);
            v43 = *(_DWORD *)(v41 + 4);
            v90 = v42;
            v92 = v43;
            if ( v43 >= 0x10 )
            {
              if ( v43 == 18 )
                v88 = 7;
              else
                v88 = v43 - 14;
              v89 = 8 * (v43 == 18) + 3;
              v91 = v88 + v90;
              if ( v4 < v88 + v90 )
              {
                while ( v84 )
                {
                  --v84;
                  v44 = *v11 << v4;
                  v4 += 8;
                  a3 = 0;
                  v10 |= v44;
                  ++v11;
                  v95 = v10;
                  v85 = v11;
                  if ( v4 >= v91 )
                    goto LABEL_76;
                }
                goto LABEL_110;
              }
LABEL_76:
              v45 = v10 >> v90;
              v46 = a1[1];
              v89 += v45 & inflate_mask[v88];
              v10 = v45 >> v88;
              v4 -= v90 + v88;
              v47 = a1[2];
              v95 = v10;
              if ( v47 + v89 > ((v46 >> 5) & 0x1F) + (v46 & 0x1F) + 258 )
                goto LABEL_111;
              if ( v92 == 16 )
              {
                if ( !v47 )
                {
LABEL_111:
                  ((void (__cdecl *)(unsigned __int8 *, _DWORD))v5[9])(v5[10], a1[3]);
                  *a1 = 9;
                  v5[6] = "invalid bit length repeat";
                  a1[8] = v10;
                  a1[7] = v4;
                  v72 = v5[2];
                  v5[1] = v84;
                  v5[2] = &v72[v85 - *v5];
                  *v5 = v85;
                  a1[13] = v86;
                  return inflate_flush(a1, v5, -3);
                }
                v48 = *(_DWORD *)(a1[3] + 4 * v47 - 4);
              }
              else
              {
                v48 = 0;
              }
              do
              {
                *(_DWORD *)(a1[3] + 4 * v47++) = v48;
                --v89;
              }
              while ( v89 );
              a1[2] = v47;
            }
            else
            {
              v10 >>= v42;
              v4 -= v42;
              *(_DWORD *)(a1[3] + 4 * a1[2]) = v43;
              v95 = v10;
              ++a1[2];
            }
            if ( a1[2] >= ((a1[1] >> 5) & 0x1F) + (a1[1] & 0x1Fu) + 258 )
              goto LABEL_84;
            v11 = v85;
          }
          while ( v84 )
          {
            --v84;
            v40 = *v11 << v4;
            v39 = a1[4];
            v4 += 8;
            a3 = 0;
            v10 |= v40;
            ++v11;
            v95 = v10;
            v85 = v11;
            if ( v4 >= v39 )
              goto LABEL_68;
          }
LABEL_109:
          a1[8] = v95;
          a1[7] = v4;
          v62 = *v5;
          v63 = v85;
          *v5 = v85;
          v5[1] = 0;
          v83 = a3;
          goto LABEL_97;
        }
LABEL_84:
        v49 = a1[1];
        v82 = a1[9];
        v79 = (_DWORD *)a1[3];
        a1[5] = 0;
        v90 = 9;
        v89 = 6;
        v50 = inflate_trees_dynamic(
                (int)v5,
                (v49 & 0x1F) + 257,
                ((v49 >> 5) & 0x1F) + 1,
                v79,
                &v90,
                (unsigned int *)&v89,
                &v93,
                &v94,
                v82);
        v51 = v50;
        if ( v50 )
        {
          if ( v50 == -3 )
          {
            ((void (__cdecl *)(unsigned __int8 *, _DWORD))v5[9])(v5[10], a1[3]);
            *a1 = 9;
          }
          a1[8] = v95;
          a1[7] = v4;
          v73 = *v5;
          v74 = v5[2];
          v5[1] = v84;
          v5[2] = &v74[v85 - v73];
          *v5 = v85;
          v83 = v51;
          goto LABEL_98;
        }
        v52 = ((int (__cdecl *)(unsigned __int8 *, int, int))v5[8])(v5[10], 1, 28);
        if ( !v52 )
        {
          a1[8] = v95;
          a1[7] = v4;
          v75 = v5[2];
          v5[1] = v84;
          v76 = v85 - *v5;
          *v5 = v85;
          v5[2] = &v75[v76];
          a1[13] = v86;
          return inflate_flush(a1, v5, -4);
        }
        v53 = v89;
        *(_BYTE *)(v52 + 16) = v90;
        v54 = v93;
        *(_BYTE *)(v52 + 17) = v53;
        v55 = v94;
        *(_DWORD *)v52 = 0;
        *(_DWORD *)(v52 + 20) = v54;
        *(_DWORD *)(v52 + 24) = v55;
        a1[1] = v52;
        ((void (__cdecl *)(unsigned __int8 *, _DWORD))v5[9])(v5[10], a1[3]);
        v10 = v95;
        *a1 = 6;
LABEL_87:
        a1[8] = v10;
        a1[7] = v4;
        v56 = v5[2];
        v5[1] = v84;
        v57 = *v5;
        *v5 = v85;
        v5[2] = &v56[v85 - v57];
        a1[13] = v86;
        v58 = inflate_codes(a1, (int)v5, a3);
        if ( v58 != 1 )
          return inflate_flush(a1, v5, v58);
        a3 = 0;
        ((void (__cdecl *)(unsigned __int8 *, _DWORD))v5[9])(v5[10], a1[1]);
        v4 = a1[7];
        v85 = *v5;
        v59 = a1[12];
        v84 = v5[1];
        v60 = (char *)a1[13];
        v95 = a1[8];
        v89 = v4;
        v86 = v60;
        if ( (unsigned int)v60 >= v59 )
          v61 = a1[11] - (_DWORD)v60;
        else
          v61 = v59 - (_DWORD)v60 - 1;
        v87 = v61;
        if ( !a1[6] )
        {
          *a1 = 0;
LABEL_93:
          v9 = *a1;
          v5 = a2;
          v4 = v89;
          if ( *a1 > 9u )
          {
LABEL_94:
            v83 = -2;
LABEL_95:
            a1[8] = v95;
            a1[7] = v4;
            v5[1] = v84;
            goto LABEL_96;
          }
          continue;
        }
        *a1 = 7;
LABEL_117:
        a1[13] = v86;
        v58 = inflate_flush(a1, v5, a3);
        v86 = (char *)a1[13];
        if ( (char *)a1[12] == v86 )
        {
          *a1 = 8;
LABEL_121:
          a1[8] = v95;
          a1[7] = v4;
          v5[1] = v84;
          v83 = 1;
LABEL_96:
          v62 = *v5;
          v63 = v85;
          *v5 = v85;
LABEL_97:
          v5[2] += v63 - v62;
LABEL_98:
          a1[13] = v86;
          return inflate_flush(a1, v5, v83);
        }
        a1[8] = v95;
        a1[7] = v4;
        v77 = *v5;
        v78 = v5[2];
        v5[1] = v84;
        *v5 = v85;
        v5[2] = &v78[v85 - v77];
        a1[13] = v86;
        return inflate_flush(a1, v5, v58);
      case 4:
        goto LABEL_51;
      case 5:
        goto LABEL_62;
      case 6:
        goto LABEL_87;
      case 7:
        goto LABEL_117;
      case 8:
        goto LABEL_121;
      case 9:
        goto LABEL_105;
      default:
        goto LABEL_94;
    }
  }
}

/* ---- inflate_blocks_free  0x004A5200 ---- */
// inflate_blocks_free -- CONFIRMED 92. Same mode-keyed free as inflate_blocks_reset, without the reset. Sole caller is inflateEnd.
/*
 * VERIFIED 0x004A5200 against upstream inflate_blocks_free -- the same
 * mode-keyed frees as inflate_blocks_reset plus ZFREE of window, hufts and
 * the state itself. Runs once per pak file close, 14,493 times in the test
 * run with no double-free.
 */
int __cdecl inflate_blocks_free(int a1, _DWORD *a2)
{
  int v2; // eax
  int (__cdecl *v3)(_DWORD, _DWORD, _DWORD); // eax
  int v4; // eax

  if ( *a2 == 4 || *a2 == 5 )
    (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), a2[3]);
  if ( *a2 == 6 )
    (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), a2[1]);
  v2 = a2[10];
  a2[13] = v2;
  a2[12] = v2;
  v3 = (int (__cdecl *)(_DWORD, _DWORD, _DWORD))a2[14];
  *a2 = 0;
  a2[7] = 0;
  a2[8] = 0;
  if ( v3 )
  {
    v4 = v3(0, 0, 0);
    a2[15] = v4;
    *(_DWORD *)(a1 + 48) = v4;
  }
  (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), a2[10]);
  (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), a2[9]);
  (*(void (__cdecl **)(_DWORD, _DWORD *))(a1 + 36))(*(_DWORD *)(a1 + 40), a2);
  return 0;
}

/* ---- inflate_set_dictionary  0x004A5280 ---- */
// inflate_set_dictionary -- CONFIRMED 90. memcpy into the window, then read = write = window + n.
/*
 * VERIFIED 0x004A5280 against upstream: zmemcpy into the window then read =
 * write = window + n. Reachable only from inflateSetDictionary, which has no
 * xrefs in retail -- it never runs on a pak, since raw-deflate streams carry
 * no dictionary.
 */
int __cdecl inflate_set_dictionary(int result, unsigned int a2, const void *a3)
{
  int v3; // ecx

  qmemcpy(*(void **)(result + 40), a3, a2);
  v3 = a2 + *(_DWORD *)(result + 40);
  *(_DWORD *)(result + 52) = v3;
  *(_DWORD *)(result + 48) = v3;
  return result;
}

/* ---- inflate_blocks_sync_point  0x004A52B0 ---- */
// inflate_blocks_sync_point -- CONFIRMED 88. s->mode == LENS (1).
/*
 * VERIFIED 0x004A52B0. `s->mode == LENS` and LENS is 1 in zlib's enum.
 * Reachable only from inflateSyncPoint, which is dead in retail.
 */
BOOL __cdecl inflate_blocks_sync_point(_DWORD *this)
{
  return *this == 1;
}

/* ---- inflate_codes_new  0x004A52C0 ---- */
// inflate_codes_new -- CONFIRMED 92. ZALLOC(1, 28) then stores lbits/dbits (bytes at +16/+17) and the two trees. Pins sizeof(inflate_codes_state) = 28.
/*
 * VERIFIED 0x004A52C0 against upstream. ZALLOC(1, 28) pins
 * sizeof(inflate_codes_state) = 28; lbits and dbits are single BYTES at
 * +16/+17, which is what upstream's `Byte lbits; Byte dbits;` compiles to,
 * and ltree/dtree are dwords at +20/+24.
 */
int __cdecl inflate_codes_new(int a1, char a2, char a3, int a4, int a5)
{
  int result; // eax

  result = (*(int (__cdecl **)(_DWORD, int, int))(a1 + 32))(*(_DWORD *)(a1 + 40), 1, 28);
  if ( result )
  {
    *(_BYTE *)(result + 16) = a2;
    *(_BYTE *)(result + 17) = a3;
    *(_DWORD *)result = 0;
    *(_DWORD *)(result + 20) = a4;
    *(_DWORD *)(result + 24) = a5;
  }
  return result;
}

/* ---- inflate_codes  0x004A5300 ---- */
// inflate_codes -- CONFIRMED 92. The literal/length/distance state machine; owns "invalid literal/length code" and "invalid distance code".
/*
 * VERIFIED 0x004A5300 BEHAVIOURALLY over 14,493 stock-pak entries. This is
 * the slow-path literal/length/distance state machine; it runs whenever
 * inflate_fast declines, i.e. near the end of every output buffer, so every
 * one of those reads covers it.
 */
/*
 * PROMOTED from AUTO-STUBBED; see the note on inflate_blocks.  The body is the
 * recovered one, with `inflate_mask[...]` renamed to inflate_mask[...] -- that
 * subscript was the only reason it would not compile.
 *
 * Argument order (a1 = s, a2 = z, a3 = r) matches zlib; pinned by the call in
 * inflate_blocks' CODES case and by inflate_flush(a1, a2, ...) further down,
 * whose own order (s, z, r) is fixed by its body (a1[12]=s->read, a2[3]=z->next_out).
 */
int __cdecl inflate_codes(_DWORD *a1, int a2, int a3)
{
  unsigned int v4; // edx
  _DWORD *v5; // ecx
  unsigned __int8 *v6; // ebp
  unsigned int v7; // eax
  _BYTE *v8; // edx
  unsigned int v9; // edi
  unsigned int v10; // eax
  int v11; // ecx
  unsigned __int8 *v12; // edi
  int v13; // eax
  unsigned int v14; // ecx
  int v15; // ecx
  unsigned int v16; // eax
  int v17; // eax
  int v18; // ecx
  int v19; // eax
  unsigned int v20; // eax
  int v21; // eax
  int v22; // ecx
  _DWORD *v23; // eax
  unsigned int v24; // eax
  int v25; // eax
  int v26; // ecx
  unsigned int v27; // eax
  int v28; // eax
  int v29; // ecx
  unsigned int v30; // ecx
  unsigned int v31; // eax
  _BYTE *v32; // ecx
  _DWORD *v33; // ecx
  unsigned int v34; // eax
  unsigned int v35; // ecx
  int v36; // eax
  unsigned int v37; // eax
  unsigned int v38; // ecx
  bool v39; // zf
  unsigned int v40; // eax
  unsigned int v41; // ecx
  int v42; // eax
  unsigned int v43; // eax
  unsigned int v44; // ecx
  unsigned __int8 *v45; // edi
  int v47; // eax
  unsigned __int8 *v48; // edi
  int v49; // eax
  unsigned __int8 *v50; // edi
  unsigned __int8 *v51; // edi
  int v52; // [esp-4h] [ebp-2Ch]
  unsigned int v53; // [esp+Ch] [ebp-1Ch]
  _DWORD *v54; // [esp+10h] [ebp-18h]
  unsigned int v55; // [esp+14h] [ebp-14h]
  unsigned int v56; // [esp+18h] [ebp-10h]
  unsigned __int8 *v57; // [esp+1Ch] [ebp-Ch]
  unsigned int v58; // [esp+1Ch] [ebp-Ch]
  unsigned int v59; // [esp+1Ch] [ebp-Ch]
  unsigned int v60; // [esp+1Ch] [ebp-Ch]
  _BYTE *v61; // [esp+1Ch] [ebp-Ch]
  _BYTE *v62; // [esp+1Ch] [ebp-Ch]
  _BYTE *v63; // [esp+20h] [ebp-8h]
  unsigned int i; // [esp+24h] [ebp-4h]
  unsigned int v65; // [esp+24h] [ebp-4h]
  unsigned int v66; // [esp+24h] [ebp-4h]
  int v67; // [esp+2Ch] [ebp+4h]

  v4 = a1[8];
  v5 = (_DWORD *)a1[1];
  v6 = *(unsigned __int8 **)a2;
  v55 = *(_DWORD *)(a2 + 4);
  v7 = a1[12];
  v53 = v4;
  v8 = (_BYTE *)a1[13];
  v9 = a1[7];
  v54 = v5;
  if ( (unsigned int)v8 >= v7 )
    v10 = a1[11] - (_DWORD)v8;
  else
    v10 = v7 - (_DWORD)v8 - 1;
  v11 = *v5;
  v56 = v10;
  while ( 2 )
  {
    switch ( v11 )
    {
      case 0:
        if ( v10 >= 0x102 && v55 >= 0xA )
        {
          a1[8] = v53;
          a1[7] = v9;
          v12 = *(unsigned __int8 **)a2;
          *(_DWORD *)(a2 + 4) = v55;
          *(_DWORD *)(a2 + 8) += v6 - v12;
          *(_DWORD *)a2 = v6;
          a1[13] = v8;
          v13 = inflate_fast(
                  *((unsigned __int8 *)v54 + 16),
                  *((unsigned __int8 *)v54 + 17),
                  v54[5],
                  v54[6],
                  a1,
                  (unsigned __int8 **)a2);
          v6 = *(unsigned __int8 **)a2;
          v9 = a1[7];
          v55 = *(_DWORD *)(a2 + 4);
          v14 = a1[12];
          v53 = a1[8];
          v8 = (_BYTE *)a1[13];
          a3 = v13;
          v15 = (unsigned int)v8 >= v14 ? a1[11] - (_DWORD)v8 : v14 - (_DWORD)v8 - 1;
          v56 = v15;
          if ( v13 )
          {
            *v54 = 2 * (v13 != 1) + 7;
            v10 = v15;
            goto LABEL_84;
          }
        }
        v54[3] = *((unsigned __int8 *)v54 + 16);
        v54[2] = v54[5];
        *v54 = 1;
        goto LABEL_14;
      case 1:
LABEL_14:
        v16 = v54[3];
        if ( v9 >= v16 )
          goto LABEL_17;
        do
        {
          if ( !v55 )
            goto LABEL_90;
          --v55;
          v17 = *v6 << v9;
          v9 += 8;
          a3 = 0;
          v18 = v17 | v53;
          v16 = v54[3];
          ++v6;
          v53 = v18;
        }
        while ( v9 < v16 );
LABEL_17:
        v57 = (unsigned __int8 *)(v54[2] + 8 * (v53 & inflate_mask[v16]));
        v53 >>= v57[1];
        v9 -= v57[1];
        v19 = *v57;
        if ( *v57 )
        {
          if ( (v19 & 0x10) != 0 )
          {
            v54[2] = v19 & 0xF;
            v54[1] = *((_DWORD *)v57 + 1);
            v10 = v56;
            *v54 = 2;
          }
          else if ( (v19 & 0x40) != 0 )
          {
            if ( (v19 & 0x20) == 0 )
            {
              *v54 = 9;
              *(_DWORD *)(a2 + 24) = "invalid literal/length code";
              v52 = -3;
              goto LABEL_86;
            }
            *v54 = 7;
            v10 = v56;
          }
          else
          {
LABEL_35:
            v54[3] = v19;
            v54[2] = &v57[8 * *((_DWORD *)v57 + 1)];
            v10 = v56;
          }
        }
        else
        {
          v54[2] = *((_DWORD *)v57 + 1);
          *v54 = 6;
          v10 = v56;
        }
        goto LABEL_84;
      case 2:
        v20 = v54[2];
        v58 = v20;
        if ( v9 >= v20 )
          goto LABEL_27;
        do
        {
          if ( !v55 )
            goto LABEL_90;
          --v55;
          v21 = *v6 << v9;
          v9 += 8;
          a3 = 0;
          v22 = v21 | v53;
          v20 = v54[2];
          ++v6;
          v53 = v22;
        }
        while ( v9 < v58 );
LABEL_27:
        v54[1] += v53 & inflate_mask[v20];
        v53 >>= v58;
        v9 -= v58;
        v23 = v54;
        v54[3] = *((unsigned __int8 *)v54 + 17);
        v54[2] = v54[6];
        *v54 = 3;
LABEL_29:
        v24 = v23[3];
        v59 = v24;
        if ( v9 < v24 )
        {
          while ( v55 )
          {
            --v55;
            v25 = *v6 << v9;
            v9 += 8;
            a3 = 0;
            v26 = v25 | v53;
            v24 = v59;
            ++v6;
            v53 = v26;
            if ( v9 >= v59 )
              goto LABEL_32;
          }
LABEL_90:
          a1[8] = v53;
          a1[7] = v9;
          v47 = (int)&v6[*(_DWORD *)(a2 + 8) - *(_DWORD *)a2];
          *(_DWORD *)(a2 + 4) = 0;
          *(_DWORD *)(a2 + 8) = v47;
          *(_DWORD *)a2 = v6;
          a1[13] = v8;
          return inflate_flush(a1, (_DWORD *)a2, a3);
        }
LABEL_32:
        v57 = (unsigned __int8 *)(v54[2] + 8 * (v53 & inflate_mask[v24]));
        v53 >>= v57[1];
        v19 = *v57;
        v9 -= v57[1];
        if ( (v19 & 0x10) != 0 )
        {
          v54[2] = v19 & 0xF;
          v54[3] = *((_DWORD *)v57 + 1);
          v10 = v56;
          *v54 = 4;
          goto LABEL_84;
        }
        if ( (v19 & 0x40) != 0 )
        {
          *v54 = 9;
          *(_DWORD *)(a2 + 24) = "invalid distance code";
          v52 = -3;
          goto LABEL_86;
        }
        goto LABEL_35;
      case 3:
        v23 = v54;
        goto LABEL_29;
      case 4:
        v27 = v54[2];
        v60 = v27;
        if ( v9 >= v27 )
          goto LABEL_39;
        do
        {
          if ( !v55 )
            goto LABEL_90;
          --v55;
          v28 = *v6 << v9;
          v9 += 8;
          a3 = 0;
          v29 = v28 | v53;
          v27 = v54[2];
          ++v6;
          v53 = v29;
        }
        while ( v9 < v60 );
LABEL_39:
        v54[3] += v53 & inflate_mask[v27];
        v53 >>= v60;
        v9 -= v60;
        *v54 = 5;
LABEL_40:
        v30 = a1[10];
        v63 = &v8[-v54[3]];
        if ( (unsigned int)v63 < v30 )
        {
          v31 = a1[11] - v30;
          v32 = &v8[-v54[3]];
          for ( i = v31; ; v31 = i )
          {
            v32 += v31;
            if ( (unsigned int)v32 >= a1[10] )
              break;
          }
          v63 = v32;
        }
        v33 = v54;
        v10 = v56;
        if ( !v54[1] )
        {
LABEL_83:
          *v33 = 0;
LABEL_84:
          v11 = *v54;
          if ( *v54 > 9u )
          {
LABEL_85:
            v52 = -2;
LABEL_86:
            a1[8] = v53;
            a1[7] = v9;
            v45 = *(unsigned __int8 **)a2;
            *(_DWORD *)(a2 + 4) = v55;
            *(_DWORD *)(a2 + 8) += v6 - v45;
            goto LABEL_87;
          }
          continue;
        }
        while ( 1 )
        {
          if ( !v10 )
          {
            if ( v8 != (_BYTE *)a1[11]
              || (v34 = a1[12], v35 = a1[10], v34 == v35)
              || ((v8 = (_BYTE *)a1[10], v35 >= v34) ? (v10 = a1[11] - v35) : (v10 = v34 - v35 - 1), !v10) )
            {
              a1[13] = v8;
              v36 = inflate_flush(a1, (_DWORD *)a2, a3);
              v8 = (_BYTE *)a1[13];
              v67 = v36;
              v37 = a1[12];
              v65 = v37;
              v10 = (unsigned int)v8 >= v37 ? a1[11] - (_DWORD)v8 : v37 - (_DWORD)v8 - 1;
              v61 = (_BYTE *)a1[11];
              if ( v8 == v61 )
              {
                v38 = a1[10];
                if ( v65 != v38 )
                {
                  v8 = (_BYTE *)a1[10];
                  if ( v38 >= v65 )
                    v10 = (unsigned int)&v61[-v38];
                  else
                    v10 = v65 - v38 - 1;
                }
              }
              if ( !v10 )
                break;
            }
          }
          *v8++ = *v63;
          --v10;
          v39 = v63 + 1 == (_BYTE *)a1[11];
          a3 = 0;
          ++v63;
          v56 = v10;
          if ( v39 )
            v63 = (_BYTE *)a1[10];
          v33 = v54;
          if ( !--v54[1] )
            goto LABEL_83;
        }
LABEL_91:
        a1[8] = v53;
        a1[7] = v9;
        v48 = *(unsigned __int8 **)a2;
        *(_DWORD *)(a2 + 4) = v55;
        *(_DWORD *)(a2 + 8) += v6 - v48;
        v52 = v67;
LABEL_87:
        *(_DWORD *)a2 = v6;
        a1[13] = v8;
        return inflate_flush(a1, (_DWORD *)a2, v52);
      case 5:
        goto LABEL_40;
      case 6:
        if ( !v10 )
        {
          if ( v8 != (_BYTE *)a1[11]
            || (v40 = a1[12], v41 = a1[10], v40 == v41)
            || ((v8 = (_BYTE *)a1[10], v41 >= v40) ? (v10 = a1[11] - v41) : (v10 = v40 - v41 - 1), !v10) )
          {
            a1[13] = v8;
            v42 = inflate_flush(a1, (_DWORD *)a2, a3);
            v8 = (_BYTE *)a1[13];
            v67 = v42;
            v43 = a1[12];
            v66 = v43;
            v10 = (unsigned int)v8 >= v43 ? a1[11] - (_DWORD)v8 : v43 - (_DWORD)v8 - 1;
            v62 = (_BYTE *)a1[11];
            if ( v8 == v62 )
            {
              v44 = a1[10];
              if ( v66 != v44 )
              {
                v8 = (_BYTE *)a1[10];
                if ( v44 >= v66 )
                  v10 = (unsigned int)&v62[-v44];
                else
                  v10 = v66 - v44 - 1;
              }
            }
            if ( !v10 )
              goto LABEL_91;
          }
        }
        *v8 = *((_BYTE *)v54 + 8);
        v33 = v54;
        ++v8;
        --v10;
        a3 = 0;
        v56 = v10;
        goto LABEL_83;
      case 7:
        if ( v9 > 7 )
        {
          v9 -= 8;
          ++v55;
          --v6;
        }
        a1[13] = v8;
        v49 = inflate_flush(a1, (_DWORD *)a2, a3);
        v8 = (_BYTE *)a1[13];
        if ( (_BYTE *)a1[12] != v8 )
        {
          a1[8] = v53;
          a1[7] = v9;
          v50 = *(unsigned __int8 **)a2;
          *(_DWORD *)(a2 + 4) = v55;
          *(_DWORD *)(a2 + 8) += v6 - v50;
          v52 = v49;
          goto LABEL_87;
        }
        *v54 = 8;
LABEL_97:
        v52 = 1;
        goto LABEL_86;
      case 8:
        goto LABEL_97;
      case 9:
        a1[8] = v53;
        a1[7] = v9;
        v51 = *(unsigned __int8 **)a2;
        *(_DWORD *)(a2 + 4) = v55;
        *(_DWORD *)(a2 + 8) += v6 - v51;
        v52 = -3;
        goto LABEL_87;
      default:
        goto LABEL_85;
    }
  }
}

/* ---- inflate_codes_free  0x004A5A00 ---- */
// inflate_codes_free -- CONFIRMED 90. Single ZFREE.
/*
 * VERIFIED 0x004A5A00. A single ZFREE, upstream verbatim.
 */
int __cdecl inflate_codes_free(int a1, int a2)
{
  return (*(int (__cdecl **)(_DWORD, int))(a1 + 36))(*(_DWORD *)(a1 + 40), a2);
}

/* ---- inflate_fast  0x004A5A10 ---- */
// inflate_fast -- CONFIRMED 90. inffast.c: the unrolled inner loop, sharing the same two error strings as inflate_codes.
/*
 * VERIFIED 0x004A5A10 BEHAVIOURALLY over 14,493 stock-pak entries -- it is
 * the path that handles the bulk of every file over a few hundred bytes.
 * Verified indirectly at the table level too: the fixed-Huffman trees it
 * walks are byte-identical to retail's, see inflate_trees_fixed below.
 */
/*
 * PROMOTED from AUTO-STUBBED; see the note on inflate_blocks.  Recovered body,
 * with `inflate_mask[...]` renamed to inflate_mask[...].
 *
 * Argument order (bl, bd, tl, td, s, z) matches zlib and is pinned by the call
 * in inflate_codes.
 */
int __cdecl inflate_fast(int a1, int a2, int a3, int a4, _DWORD *a5, unsigned __int8 **a6)
{
  unsigned __int8 *v6; // ebp
  unsigned int v7; // ecx
  unsigned int v8; // edx
  _BYTE *v9; // edi
  unsigned int v10; // eax
  int v11; // esi
  int v12; // ebx
  int v13; // esi
  int v14; // ecx
  int v15; // ecx
  int v16; // ebx
  unsigned int v17; // eax
  unsigned int v18; // edx
  int v19; // esi
  int v20; // ebx
  int v21; // esi
  int v22; // ecx
  int v23; // ecx
  unsigned int i; // ebx
  int v25; // ebp
  int v26; // ebp
  unsigned int v27; // ecx
  _BYTE *v28; // esi
  unsigned int v29; // ebx
  int v30; // ebp
  int v31; // ecx
  _BYTE *v32; // esi
  _BYTE *v33; // edi
  _BYTE *v34; // esi
  unsigned int v35; // ecx
  _BYTE *v36; // edi
  _BYTE *v37; // esi
  int v38; // ecx
  unsigned int v39; // ecx
  unsigned __int8 *v40; // edx
  unsigned __int8 *v41; // ebp
  unsigned __int8 *v42; // eax
  unsigned __int8 **v44; // ecx
  unsigned int v45; // ebx
  bool v46; // zf
  unsigned int v47; // ecx
  unsigned __int8 *v48; // edx
  unsigned __int8 *v49; // ebp
  unsigned __int8 *v50; // eax
  unsigned int v51; // esi
  unsigned __int8 *v52; // ebp
  unsigned __int8 *v53; // eax
  int v54; // edx
  unsigned int v55; // [esp+10h] [ebp-14h]
  unsigned int v56; // [esp+14h] [ebp-10h]
  unsigned __int8 *v57; // [esp+18h] [ebp-Ch]
  int v58; // [esp+1Ch] [ebp-8h]
  int v59; // [esp+28h] [ebp+4h]
  int v60; // [esp+28h] [ebp+4h]
  int v61; // [esp+2Ch] [ebp+8h]

  v6 = *a6;
  v7 = a5[12];
  v8 = a5[8];
  v9 = (_BYTE *)a5[13];
  v55 = (unsigned int)a6[1];
  v10 = a5[7];
  v57 = *a6;
  if ( (unsigned int)v9 >= v7 )
    v56 = a5[11] - (_DWORD)v9;
  else
    v56 = v7 - (_DWORD)v9 - 1;
  v58 = inflate_mask[a1];
  v61 = inflate_mask[a2];
  while ( 1 )
  {
    if ( v10 < 0x14 )
    {
      do
      {
        --v55;
        v11 = *v6 << v10;
        v10 += 8;
        v8 |= v11;
        ++v6;
      }
      while ( v10 < 0x14 );
      v57 = v6;
    }
    v12 = *(unsigned __int8 *)(a3 + 8 * (v8 & v58));
    v13 = a3 + 8 * (v8 & v58);
    v14 = *(unsigned __int8 *)(v13 + 1);
    v8 >>= v14;
    if ( !v12 )
    {
LABEL_36:
      v10 -= v14;
      *v9++ = *(_BYTE *)(v13 + 4);
      --v56;
      goto LABEL_37;
    }
    v10 -= *(unsigned __int8 *)(v13 + 1);
    if ( (v12 & 0x10) == 0 )
      break;
LABEL_13:
    v16 = v12 & 0xF;
    v17 = v10 - v16;
    v59 = *(_DWORD *)(v13 + 4) + (v8 & inflate_mask[v16]);
    v18 = v8 >> v16;
    if ( v17 < 0xF )
    {
      do
      {
        --v55;
        v19 = *v6 << v17;
        v17 += 8;
        v18 |= v19;
        ++v6;
      }
      while ( v17 < 0xF );
      v57 = v6;
    }
    v20 = *(unsigned __int8 *)(a4 + 8 * (v18 & v61));
    v21 = a4 + 8 * (v18 & v61);
    v8 = v18 >> *(_BYTE *)(v21 + 1);
    v10 = v17 - *(unsigned __int8 *)(v21 + 1);
    if ( (v20 & 0x10) == 0 )
    {
      while ( (v20 & 0x40) == 0 )
      {
        v22 = *(_DWORD *)(v21 + 4) + (v8 & inflate_mask[v20]);
        v20 = *(unsigned __int8 *)(v21 + 8 * v22);
        v21 += 8 * v22;
        v23 = *(unsigned __int8 *)(v21 + 1);
        v8 >>= v23;
        v10 -= v23;
        if ( (v20 & 0x10) != 0 )
          goto LABEL_19;
      }
      v44 = a6;
      v45 = v55;
      a6[6] = "invalid distance code";
      goto LABEL_48;
    }
LABEL_19:
    for ( i = v20 & 0xF; v10 < i; ++v57 )
    {
      --v55;
      v25 = *v6 << v10;
      v10 += 8;
      v8 |= v25;
      v6 = v57 + 1;
    }
    v26 = *(_DWORD *)(v21 + 4) + (v8 & inflate_mask[i]);
    v8 >>= i;
    v27 = v59;
    v56 -= v59;
    v28 = &v9[-v26];
    v10 -= i;
    v29 = a5[10];
    if ( (unsigned int)&v9[-v26] >= v29 )
    {
      *v9 = *v28;
      v36 = v9 + 1;
      *v36 = v28[1];
      v9 = v36 + 1;
      v37 = v28 + 2;
      v38 = v59 - 2;
      do
      {
        *v9++ = *v37++;
        --v38;
      }
      while ( v38 );
      v6 = v57;
    }
    else
    {
      v60 = a5[11];
      do
        v28 += v60 - v29;
      while ( (unsigned int)v28 < v29 );
      v30 = v60 - (_DWORD)v28;
      if ( v27 <= v60 - (int)v28 )
      {
        *v9 = *v28;
        v33 = v9 + 1;
        *v33 = v28[1];
        v9 = v33 + 1;
        v34 = v28 + 2;
        v35 = v27 - 2;
        do
        {
          *v9++ = *v34++;
          --v35;
        }
        while ( v35 );
        v6 = v57;
      }
      else
      {
        v31 = v27 - v30;
        do
        {
          *v9++ = *v28++;
          --v30;
        }
        while ( v30 );
        v32 = (_BYTE *)a5[10];
        do
        {
          *v9++ = *v32++;
          --v31;
        }
        while ( v31 );
        v6 = v57;
      }
    }
LABEL_37:
    if ( v56 < 0x102 || v55 < 0xA )
    {
      v39 = (unsigned int)&a6[1][-v55];
      if ( v10 >> 3 < v39 )
        v39 = v10 >> 3;
      a5[8] = v8;
      a5[7] = v10 - 8 * v39;
      v40 = *a6;
      v41 = &v6[-v39];
      v42 = a6[2];
      a6[1] = (unsigned __int8 *)(v55 + v39);
      a6[2] = &v42[v41 - v40];
      *a6 = v41;
      a5[13] = v9;
      return 0;
    }
  }
  while ( (v12 & 0x40) == 0 )
  {
    v15 = *(_DWORD *)(v13 + 4) + (v8 & inflate_mask[v12]);
    v12 = *(unsigned __int8 *)(v13 + 8 * v15);
    v13 += 8 * v15;
    v14 = *(unsigned __int8 *)(v13 + 1);
    v8 >>= v14;
    if ( !v12 )
      goto LABEL_36;
    v10 -= *(unsigned __int8 *)(v13 + 1);
    if ( (v12 & 0x10) != 0 )
      goto LABEL_13;
  }
  v46 = (v12 & 0x20) == 0;
  v45 = v55;
  if ( v46 )
  {
    v44 = a6;
    a6[6] = "invalid literal/length code";
LABEL_48:
    v51 = v10 >> 3;
    if ( v10 >> 3 >= (unsigned int)&v44[1][-v45] )
      v51 = (unsigned int)&v44[1][-v45];
    a5[8] = v8;
    a5[7] = v10 - 8 * v51;
    v52 = &v6[-v51];
    v53 = v44[2];
    v44[1] = (unsigned __int8 *)(v55 + v51);
    v54 = v52 - *v44;
    *v44 = v52;
    v44[2] = &v53[v54];
    a5[13] = v9;
    return -3;
  }
  v47 = (unsigned int)&a6[1][-v55];
  if ( v10 >> 3 < v47 )
    v47 = v10 >> 3;
  a5[8] = v8;
  a5[7] = v10 - 8 * v47;
  v48 = *a6;
  v49 = &v6[-v47];
  v50 = a6[2];
  a6[1] = (unsigned __int8 *)(v55 + v47);
  a6[2] = &v50[v49 - v48];
  *a6 = v49;
  a5[13] = v9;
  return 1;
}

/* ---- inflateReset  0x004A5DB0 ---- */
// inflateReset -- CONFIRMED 92. Clears total_in/total_out/msg and calls inflate_blocks_reset.
/*
 * VERIFIED 0x004A5DB0 against upstream. internal_state is 24 bytes -- mode 0,
 * sub 4..11, nowrap 12, wbits 16, blocks 20 -- matching the ZALLOC(1,24) in
 * inflateInit2_; `nowrap ? 7 : 0` is BLOCKS vs METHOD in zlib's enum, and
 * raw-deflate pak streams always take the 7.
 */
int __cdecl inflateReset(_DWORD *a1)
{
  _DWORD *v2; // eax

  if ( !a1 )
    return -2;
  v2 = (_DWORD *)a1[7];
  if ( !v2 )
    return -2;
  a1[5] = 0;
  a1[2] = 0;
  a1[6] = 0;
  *v2 = v2[3] != 0 ? 7 : 0;
  inflate_blocks_reset(0, (int)a1, *(_DWORD **)(a1[7] + 20));
  return 0;
}

/* ---- inflateEnd  0x004A5DF0 ---- */
// inflateEnd -- CONFIRMED 95. inflate_blocks_free then ZFREE of the state. Called by unzCloseCurrentFile and uncompress.
/*
 * VERIFIED 0x004A5DF0 against upstream. Called from unzCloseCurrentFile once
 * per pak file, 14,493 times in the test run.
 */
int __cdecl inflateEnd(_DWORD *a1)
{
  int v2; // eax
  _DWORD *v3; // esi

  if ( !a1 )
    return -2;
  v2 = a1[7];
  if ( !v2 || !a1[9] )
    return -2;
  v3 = *(_DWORD **)(v2 + 20);
  if ( v3 )
    inflate_blocks_free((int)a1, v3);
  ((void (__cdecl *)(_DWORD, _DWORD))a1[9])(a1[10], a1[7]);
  a1[7] = 0;
  return 0;
}

/* ---- inflateInit2_  0x004A5E40 ---- */
// inflateInit2_ -- CONFIRMED 95. Validates version[0] == '1' (49) and stream_size == 56. The 56 pins sizeof(z_stream) for this build.
/*
 * VERIFIED 0x004A5E40 against upstream. The two guards are version[0] == 49
 * ('1') and stream_size == 56, which pins sizeof(z_stream) for this build.
 * Negative windowBits sets nowrap and suppresses the adler32 checkfn, which
 * is how the pak path runs -- and is why the CRC in unzCloseCurrentFile is
 * the only integrity check a pak read gets.
 */
int __cdecl inflateInit2_(_BYTE *a1, int a2, _DWORD *a3, int a4)
{
  bool v5; // zf
  int v6; // eax

  if ( !a1 || *a1 != 49 || a4 != 56 )
    return -6;
  if ( !a3 )
    return -2;
  v5 = a3[8] == 0;
  a3[6] = 0;
  if ( v5 )
  {
    a3[8] = zcalloc;
    a3[10] = 0;
  }
  if ( !a3[9] )
    a3[9] = zcfree;
  v6 = ((int (__cdecl *)(_DWORD, int, int))a3[8])(a3[10], 1, 24);
  a3[7] = v6;
  if ( !v6 )
    return -4;
  *(_DWORD *)(v6 + 20) = 0;
  *(_DWORD *)(a3[7] + 12) = 0;
  if ( a2 < 0 )
  {
    a2 = -a2;
    *(_DWORD *)(a3[7] + 12) = 1;
  }
  if ( a2 < 8 || a2 > 15 )
  {
    inflateEnd(a3);
    return -2;
  }
  *(_DWORD *)(a3[7] + 16) = a2;
  *(_DWORD *)(a3[7] + 20) = inflate_blocks_new(
                              (int)a3,
                              1 << a2,
                              *(_DWORD *)(a3[7] + 12) != 0 ? 0 : (unsigned int)sub_4A2AF0);
  if ( !*(_DWORD *)(a3[7] + 20) )
  {
    inflateEnd(a3);
    return -4;
  }
  inflateReset(a3);
  return 0;
}

/* ---- inflateInit_  0x004A5F30 ---- */
// inflateInit_ -- CONFIRMED 92. inflateInit2_(strm, DEF_WBITS=15, version, stream_size).
/*
 * VERIFIED 0x004A5F30. inflateInit2_(strm, DEF_WBITS = 15, version,
 * stream_size); the argument order is confirmed by matching the forwarded
 * slots against the callee above.
 */
int __cdecl inflateInit_(int a1, _DWORD *a2, _BYTE *a3)
{
  return inflateInit2_(a3, 15, a2, a1);
}

/* ---- inflate  0x004A5F50 ---- */
// inflate -- CONFIRMED 95. The inflate.c driver; owns "unknown compression method", "invalid window size", "incorrect header check".
/*
 * VERIFIED 0x004A5F50 against upstream inflate.c. Exercised in both
 * configurations: raw-deflate on 14,493 pak entries (Z_SYNC_FLUSH from
 * unzReadCurrentFile), and zlib-wrapped through uncompress, which is dead in
 * retail.
 */
int __cdecl inflate(unsigned __int8 **a1, int a2)
{
  unsigned __int8 *v3; // eax
  unsigned int v4; // eax
  int v5; // ebx
  unsigned int v6; // ebp
  unsigned __int8 *v7; // eax
  unsigned __int8 *v8; // ecx
  unsigned __int8 *v9; // edx
  unsigned __int8 *v10; // eax
  unsigned __int8 *v11; // eax
  unsigned __int8 *v12; // eax
  unsigned __int8 *v13; // esi
  unsigned __int8 *v14; // eax
  int v15; // ecx
  unsigned __int8 *v16; // esi
  int v17; // eax
  unsigned __int8 *v18; // eax
  unsigned __int8 *v19; // eax
  unsigned __int8 *v20; // edx
  unsigned __int8 *v21; // ecx
  unsigned __int8 *v22; // eax
  unsigned __int8 *v23; // ecx
  unsigned __int8 *v24; // eax
  unsigned __int8 *v25; // edx
  unsigned __int8 *v26; // edx
  unsigned __int8 *v27; // eax
  unsigned __int8 *v28; // ecx
  unsigned __int8 *v29; // eax
  unsigned __int8 *v30; // edx
  unsigned __int8 *v31; // ecx
  unsigned __int8 *v32; // eax
  unsigned __int8 *v33; // eax
  unsigned __int8 *v35; // eax
  unsigned __int8 *v36; // ecx
  unsigned __int8 *v37; // edx
  unsigned __int8 *v38; // edx
  unsigned __int8 *v39; // eax
  unsigned __int8 *v40; // ecx
  unsigned __int8 *v41; // eax
  unsigned __int8 *v42; // ecx
  unsigned __int8 *v43; // eax
  unsigned __int8 *v44; // eax
  unsigned __int8 *v45; // edx
  unsigned __int8 *v46; // ecx
  unsigned __int8 *v47; // eax
  unsigned __int8 *v48; // eax
  unsigned __int8 *v49; // eax

  if ( !a1 )
    return -2;
  v3 = a1[7];
  if ( !v3 || !*a1 )
    return -2;
  v4 = *(_DWORD *)v3;
  v5 = -5;
  v6 = a2 != 4 ? 0 : 0xFFFFFFFB;
  while ( 2 )
  {
    switch ( v4 )
    {
      case 0u:
        v7 = a1[1];
        if ( !v7 )
          return v5;
        v8 = a1[7];
        ++a1[2];
        v9 = *a1;
        a1[1] = v7 - 1;
        *((_DWORD *)v8 + 1) = *v9;
        v10 = a1[7];
        LOBYTE(v9) = *((_DWORD *)v10 + 1) & 0xF;
        v5 = v6;
        ++*a1;
        if ( (_BYTE)v9 != 8 )
        {
          *(_DWORD *)v10 = 13;
          v11 = a1[7];
          a1[6] = "unknown compression method";
          *((_DWORD *)v11 + 1) = 5;
LABEL_35:
          v4 = *(_DWORD *)a1[7];
          if ( v4 > 0xD )
            return -2;
          continue;
        }
        if ( (unsigned int)((*((_DWORD *)v10 + 1) >> 4) + 8) > *((_DWORD *)v10 + 4) )
        {
          *(_DWORD *)v10 = 13;
          a1[6] = "invalid window size";
LABEL_34:
          *((_DWORD *)a1[7] + 1) = 5;
          goto LABEL_35;
        }
        *(_DWORD *)v10 = 1;
LABEL_12:
        v12 = a1[1];
        if ( !v12 )
          return v5;
        v13 = a1[2];
        a1[1] = v12 - 1;
        v14 = *a1 + 1;
        a1[2] = v13 + 1;
        v15 = *(v14 - 1);
        v16 = a1[7];
        *a1 = v14;
        v5 = v6;
        if ( (v15 + (*((_DWORD *)v16 + 1) << 8)) % 0x1Fu )
        {
          *(_DWORD *)v16 = 13;
          a1[6] = "incorrect header check";
          goto LABEL_34;
        }
        if ( (v15 & 0x20) == 0 )
        {
          *(_DWORD *)v16 = 7;
          goto LABEL_35;
        }
        *(_DWORD *)a1[7] = 2;
LABEL_39:
        v35 = a1[1];
        if ( !v35 )
          return v5;
        v36 = a1[7];
        ++a1[2];
        v37 = *a1;
        a1[1] = v35 - 1;
        *((_DWORD *)v36 + 2) = *v37 << 24;
        v38 = a1[7];
        ++*a1;
        v5 = v6;
        *(_DWORD *)v38 = 3;
LABEL_41:
        v39 = a1[1];
        if ( !v39 )
          return v5;
        v40 = *a1;
        a1[1] = v39 - 1;
        ++a1[2];
        *((_DWORD *)a1[7] + 2) += *v40 << 16;
        ++*a1;
        v5 = v6;
        *(_DWORD *)a1[7] = 4;
LABEL_43:
        v41 = a1[1];
        if ( !v41 )
          return v5;
        v42 = *a1;
        a1[1] = v41 - 1;
        v43 = a1[7];
        v5 = v6;
        ++a1[2];
        *((_DWORD *)v43 + 2) += *v42 << 8;
        ++*a1;
        *(_DWORD *)a1[7] = 5;
LABEL_45:
        v44 = a1[1];
        if ( !v44 )
          return v5;
        v45 = a1[2];
        v46 = *a1;
        a1[1] = v44 - 1;
        v47 = a1[7];
        a1[2] = v45 + 1;
        *((_DWORD *)v47 + 2) += *v46;
        ++*a1;
        v48 = a1[7];
        a1[12] = (unsigned __int8 *)*((_DWORD *)v48 + 2);
        *(_DWORD *)v48 = 6;
        return 2;
      case 1u:
        goto LABEL_12;
      case 2u:
        goto LABEL_39;
      case 3u:
        goto LABEL_41;
      case 4u:
        goto LABEL_43;
      case 5u:
        goto LABEL_45;
      case 6u:
        *(_DWORD *)a1[7] = 13;
        v49 = a1[7];
        a1[6] = "need dictionary";
        *((_DWORD *)v49 + 1) = 0;
        return -2;
      case 7u:
        v17 = inflate_blocks(*((_DWORD **)a1[7] + 5), a1, v5);
        v5 = v17;
        if ( v17 == -3 )
        {
          *(_DWORD *)a1[7] = 13;
          *((_DWORD *)a1[7] + 1) = 0;
          goto LABEL_35;
        }
        if ( !v17 )
          v5 = v6;
        if ( v5 != 1 )
          return v5;
        v5 = v6;
        inflate_blocks_reset((_DWORD *)a1[7] + 1, (int)a1, *((_DWORD **)a1[7] + 5));
        v18 = a1[7];
        if ( *((_DWORD *)v18 + 3) )
        {
          *(_DWORD *)v18 = 12;
          goto LABEL_35;
        }
        *(_DWORD *)v18 = 8;
LABEL_25:
        v19 = a1[1];
        if ( !v19 )
          return v5;
        v20 = a1[2];
        v21 = *a1;
        a1[1] = v19 - 1;
        v22 = a1[7];
        a1[2] = v20 + 1;
        *((_DWORD *)v22 + 2) = *v21 << 24;
        v23 = a1[7];
        ++*a1;
        v5 = v6;
        *(_DWORD *)v23 = 9;
LABEL_27:
        v24 = a1[1];
        if ( !v24 )
          return v5;
        v25 = *a1;
        a1[1] = v24 - 1;
        ++a1[2];
        *((_DWORD *)a1[7] + 2) += *v25 << 16;
        v26 = a1[7];
        ++*a1;
        v5 = v6;
        *(_DWORD *)v26 = 10;
LABEL_29:
        v27 = a1[1];
        if ( !v27 )
          return v5;
        v28 = *a1;
        a1[1] = v27 - 1;
        ++a1[2];
        *((_DWORD *)a1[7] + 2) += *v28 << 8;
        ++*a1;
        v5 = v6;
        *(_DWORD *)a1[7] = 11;
LABEL_31:
        v29 = a1[1];
        if ( !v29 )
          return v5;
        v30 = a1[2];
        v31 = *a1;
        a1[1] = v29 - 1;
        v32 = a1[7];
        a1[2] = v30 + 1;
        *((_DWORD *)v32 + 2) += *v31;
        v33 = a1[7];
        ++*a1;
        v5 = v6;
        if ( *((_DWORD *)v33 + 1) == *((_DWORD *)v33 + 2) )
        {
          *(_DWORD *)a1[7] = 12;
          return 1;
        }
        *(_DWORD *)v33 = 13;
        a1[6] = "incorrect data check";
        goto LABEL_34;
      case 8u:
        goto LABEL_25;
      case 9u:
        goto LABEL_27;
      case 0xAu:
        goto LABEL_29;
      case 0xBu:
        goto LABEL_31;
      case 0xCu:
        return 1;
      case 0xDu:
        return -3;
      default:
        return -2;
    }
  }
}

/* ---- inflateSetDictionary  0x004A6340 ---- */
// inflateSetDictionary -- CONFIRMED 90. Public wrapper over inflate_set_dictionary; checks state mode == DICT0 (6).
/*
 * VERIFIED 0x004A6340 against upstream. DEAD IN RETAIL -- no xrefs. Verified,
 * not exercised.
 */
int __cdecl inflateSetDictionary(unsigned int a1, int a2, unsigned __int8 *a3)
{
  unsigned int v4; // ebp
  _DWORD *v5; // ebx
  unsigned int v7; // eax

  v4 = a1;
  if ( !a2 )
    return -2;
  v5 = *(_DWORD **)(a2 + 28);
  if ( !v5 || *v5 != 6 )
    return -2;
  if ( sub_4A2AF0(1u, a3, a1) != *(_DWORD *)(a2 + 48) )
    return -3;
  *(_DWORD *)(a2 + 48) = 1;
  v7 = 1 << v5[4];
  if ( a1 >= v7 )
  {
    v4 = v7 - 1;
    a3 += a1 - (v7 - 1);
  }
  inflate_set_dictionary(v5[5], v4, a3);
  **(_DWORD **)(a2 + 28) = 7;
  return 0;
}

/* ---- inflateSync  AUTO-STUBBED ----
 * The body did not compile.  Everything recovered for it is kept in the
 * #if 0 block below -- that is the starting point for reconstructing it
 * by hand.  Until then the signature is real but the body does nothing.
 */
int inflateSync() { return 0; }
#if 0
// inflateSync -- CONFIRMED 88. Scans for the 00 00 FF FF sync pattern.
int __cdecl inflateSync(_DWORD *a1)
{
  _DWORD *v1; // eax
  int v2; // ebp
  int v4; // ebx
  unsigned int v5; // eax
  _BYTE *v6; // ecx
  int v7; // edi
  int v8; // ebx
  _DWORD *v9; // eax

  if ( !a1 )
    return -2;
  v1 = (_DWORD *)a1[7];
  if ( !v1 )
    return -2;
  if ( *v1 != 13 )
  {
    *v1 = 13;
    *(_DWORD *)(a1[7] + 4) = 0;
  }
  v2 = a1[1];
  if ( !v2 )
    return -5;
  v4 = a1[7];
  v5 = *(_DWORD *)(v4 + 4);
  v6 = (_BYTE *)*a1;
  do
  {
    if ( v5 >= 4 )
      break;
    if ( *v6 == byte_5412C8[v5] )
      ++v5;
    else
      v5 = *v6 ? 0 : 4 - v5;
    ++v6;
    --v2;
  }
  while ( v2 );
  a1[2] += &v6[-*a1];
  *a1 = v6;
  a1[1] = v2;
  *(_DWORD *)(v4 + 4) = v5;
  if ( v5 != 4 )
    return -3;
  v7 = a1[2];
  v8 = a1[5];
  inflateReset(a1);
  v9 = (_DWORD *)a1[7];
  a1[2] = v7;
  a1[5] = v8;
  *v9 = 7;
  return 0;
}
#endif

/* ---- inflateSyncPoint  0x004A6490 ---- */
// inflateSyncPoint -- CONFIRMED 90. Forwards to inflate_blocks_sync_point.
/*
 * VERIFIED 0x004A6490. One-line forward to inflate_blocks_sync_point. DEAD IN
 * RETAIL -- no xrefs.
 */
int __cdecl inflateSyncPoint(int a1)
{
  int v1; // eax
  _DWORD *v2; // eax

  if ( a1 && (v1 = *(_DWORD *)(a1 + 28)) != 0 && (v2 = *(_DWORD **)(v1 + 20)) != 0 )
    return *v2 == 1;
  else
    return -2;
}

/* ---- huft_build  0x004A64C0 ---- */
// huft_build -- CONFIRMED 92. inftrees.c: the Huffman table builder shared by inflate_trees_bits and inflate_trees_dynamic.
/*
 * VERIFIED 0x004A64C0 MECHANICALLY, and this is the strongest single result
 * in the file. Its output for the fixed literal/length and distance trees was
 * dumped and compared against retail's own precompiled tables at 0x00571C58
 * (fixed_tl, 4096 bytes) and 0x00572C58 (fixed_td, 256 bytes): SHA-1
 * identical over all 4,352 bytes, with bl/bd coming back 9/5 to match
 * 0x00571C4C/0x00571C50. Those tables ARE huft_build output, so that verifies
 * the whole builder -- including the restored huft_c/huft_x arrays and the
 * hard-coded 0x5A0 (1440) overflow guard -- without reading a single
 * instruction. The dynamic path is covered behaviourally by 14,493 stock-pak
 * entries. ARGUMENT ORDER IS REGISTER ORDER (m, b, n, s, d, e, t, hp, hn, v),
 * not zlib's (b, n, s, d, e, t, m, hp, hn, v).
 */
int __cdecl huft_build(
        unsigned int *a1,
        _DWORD *a2,
        unsigned int a3,
        unsigned int a4,
        int a5,
        int a6,
        _DWORD *a7,
        int a8,
        unsigned int *a9,
        unsigned int *a10)
{
  unsigned int v10; // ebp
  _DWORD *v13; // eax
  unsigned int v14; // edx
  unsigned int v16; // eax
  unsigned int v17; // ecx
  unsigned int v18; // esi
  int v19; // edx
  int v20; // edx
  int *v21; // edi
  int v22; // edx
  int v23; // ecx
  unsigned int v24; // esi
  int v25; // edi
  int *v26; // esi
  unsigned int v27; // edi
  int v28; // ecx
  int v29; // esi
  signed int v30; // ecx
  unsigned int v31; // edi
  int v32; // ebx
  unsigned int v33; // esi
  unsigned int j; // ecx
  unsigned int v35; // edx
  unsigned int v36; // ecx
  unsigned int v37; // eax
  int *v38; // ebp
  unsigned int v39; // eax
  unsigned int v40; // edi
  unsigned int v41; // eax
  unsigned int v42; // edx
  int v43; // ebp
  unsigned int v44; // eax
  int v45; // ecx
  unsigned int v46; // esi
  int v47; // edx
  unsigned int v48; // eax
  _DWORD *v49; // ecx
  unsigned int i; // eax
  int v51; // eax
  int v52; // ecx
  unsigned int v53; // [esp+10h] [ebp-FCh]
  signed int v54; // [esp+14h] [ebp-F8h]
  int v55; // [esp+18h] [ebp-F4h]
  int v56; // [esp+1Ch] [ebp-F0h]
  int *v57; // [esp+20h] [ebp-ECh]
  unsigned int *v58; // [esp+20h] [ebp-ECh]
  int *v59; // [esp+24h] [ebp-E8h]
  signed int v60; // [esp+28h] [ebp-E4h]
  unsigned int v61; // [esp+2Ch] [ebp-E0h]
  unsigned int v62; // [esp+30h] [ebp-DCh]
  int v63; // [esp+30h] [ebp-DCh]
  int v64; // [esp+34h] [ebp-D8h]
  /* v65 was the recovered name for zlib's `r.base` BEFORE its first
   * assignment.  Nothing ever writes it -- uninit_scan.py flags it -- because
   * upstream's `inflate_huft r;` is declared uninitialised and the
   * out-of-values path (`r.exop = 128 + 64`) deliberately leaves base alone.
   * Faithful, but a genuine uninitialised read, so it is pinned to 0 below.
   * Cannot change behaviour: exop 192 means "invalid code", and both
   * inflate_codes and inflate_fast test `e & 64` and abort without ever
   * looking at base. */
  signed int v66; // [esp+3Ch] [ebp-D0h]
  char v67; // [esp+40h] [ebp-CCh]
  unsigned int v68; // [esp+44h] [ebp-C8h]
  _DWORD *v69; // [esp+48h] [ebp-C4h]
  int v70; // [esp+4Ch] [ebp-C0h]
  /*
   * Hex-Rays split two contiguous stack arrays into scalars plus short arrays
   * and then indexed straight across the split -- `*(&v71 + i)`, `v75[i]`,
   * `(char *)&v72 + 4 * i`, `*(&v76 + i)`, `v78[i]`.  That is only correct if
   * the compiler lays the locals out in declaration order at the recovered
   * offsets, and VC6 does not: the code-length counts came back out of the
   * wrong slots and every dynamic block was rejected with "oversubscribed
   * dynamic bit lengths tree".  Restored to zlib's own two arrays.
   *
   * The mapping is fixed by the recovered stack offsets:
   *     huft_c[0..15] = v71 v72 v73 v74 v75[0..10] v76   (zlib c[BMAX+1],
   *                     ebp-BCh..ebp-80h, 64 bytes = 16 uInts)
   *     huft_x[0..15] = v77 v78[0..14]                   (zlib x[BMAX+1],
   *                     ebp-7Ch..ebp-40h)
   * so every old `v78[k]` reads `huft_x[k + 1]`, and `*(&v76 + h)` -- one int
   * below huft_x -- is `huft_x[h - 1]`.
   *
   * v79 is zlib's u[BMAX] table stack and was already self-contained.
   */
  int huft_c[16];
  int huft_x[16];
  _DWORD v79[15]; // [esp+D0h] [ebp-3Ch] BYREF
  int v80; // [esp+110h] [ebp+4h]

  v10 = 0;
  memset(huft_c, 0, sizeof(huft_c));
  v13 = a2;
  v14 = a3;
  do
  {
    ++huft_c[*v13++];
    --v14;
  }
  while ( v14 );
  if ( huft_c[0] == a3 )
  {
    *a7 = 0;
    *a1 = 0;
    return 0;
  }
  v16 = *a1;
  v53 = *a1;
  v17 = 1;
  while ( !huft_c[v17] )
  {
    if ( huft_c[v17 + 1] )
    {
      ++v17;
      break;
    }
    if ( huft_c[v17 + 2] )
    {
      v17 += 2;
      break;
    }
    if ( huft_c[v17 + 3] )
    {
      v17 += 3;
      break;
    }
    if ( huft_c[v17 + 4] )
    {
      v17 += 4;
      break;
    }
    v17 += 5;
    if ( v17 > 0xF )
      break;
  }
  v54 = v17;
  if ( v16 < v17 )
  {
    v53 = v17;
    v16 = v17;
  }
  v18 = 15;
  do
  {
    if ( huft_c[v18] )
      break;
    --v18;
  }
  while ( v18 );
  v60 = v18;
  if ( v16 > v18 )
  {
    v53 = v18;
    v16 = v18;
  }
  v19 = 1 << v17;
  for ( *a1 = v16; v17 < v18; v19 = 2 * v20 )
  {
    v20 = v19 - huft_c[v17];
    if ( v20 < 0 )
      return -3;
    ++v17;
  }
  v21 = &huft_c[v18];
  v62 = v18;
  v22 = v19 - *v21;
  v70 = v22;
  if ( v22 < 0 )
    return -3;
  *v21 += v22;
  v23 = 0;
  v24 = v18 - 1;
  huft_x[1] = 0;
  if ( v24 )
  {
    v25 = 0;
    do
    {
      v23 += huft_c[v25 + 1];
      ++v25;
      --v24;
      huft_x[v25 + 1] = v23;
    }
    while ( v24 );
  }
  v26 = a2;
  v27 = 0;
  do
  {
    v28 = *v26++;
    v57 = v26;
    if ( v28 )
    {
      v29 = huft_x[v28];
      a10[v29] = v27;
      huft_x[v28] = v29 + 1;
      v26 = v57;
    }
    ++v27;
  }
  while ( v27 < a3 );
  v80 = huft_x[v62];
  v58 = a10;
  v30 = v54;
  v31 = 0;
  v32 = -v16;
  v61 = 0;
  huft_x[0] = 0;
  v55 = -1;
  v79[0] = 0;
  v63 = 0;
  if ( v54 <= v60 )
  {
    v33 = 0;              /* was `v33 = v65;` -- see the note on v65 above */
    v67 = v54 - 1;
    v59 = &huft_c[v54];
    while ( 1 )
    {
      v56 = *v59;
      if ( *v59 )
        break;
LABEL_72:
      ++v59;
      ++v30;
      ++v67;
      v54 = v30;
      if ( v30 > v60 )
      {
        v22 = v70;
        goto LABEL_74;
      }
    }
    while ( 1 )
    {
      --v56;
      if ( v54 > (int)(v32 + v16) )
        break;
LABEL_55:
      if ( v58 < &a10[v80] )
      {
        v33 = *v58;
        if ( *v58 >= a4 )
        {
          v46 = 4 * (v33 - a4);
          LOBYTE(v64) = *(_BYTE *)(v46 + a6) + 80;
          v33 = *(_DWORD *)(v46 + a5);
        }
        else
        {
          LOBYTE(v64) = v33 < 0x100 ? 0 : 0x60;
        }
        ++v58;
      }
      else
      {
        LOBYTE(v64) = -64;
      }
      v47 = 1 << (v54 - v32);
      v48 = v10 >> v32;
      if ( v10 >> v32 < v31 )
      {
        v49 = (_DWORD *)(v63 + 8 * v48);
        do
        {
          BYTE1(v64) = v54 - v32;
          *v49 = v64;
          v49[1] = v33;
          v48 += v47;
          v49 += 2 * v47;
        }
        while ( v48 < v31 );
        v10 = v61;
      }
      for ( i = 1 << v67; (i & v10) != 0; i >>= 1 )
        v10 ^= i;
      v10 ^= i;
      v51 = v55;
      v61 = v10;
      if ( (v10 & ((1 << v32) - 1)) != (unsigned int)huft_x[v55] )
      {
        do
        {
          v32 -= v53;
          v52 = huft_x[v51 - 1];
          --v51;
        }
        while ( (v10 & ((1 << v32) - 1)) != v52 );
        v55 = v51;
      }
      v16 = v53;
      if ( !v56 )
      {
        v30 = v54;
        goto LABEL_72;
      }
    }
    v66 = v32 + v16;
    for ( j = v32 - v16; ; j = v68 )
    {
      ++v55;
      v32 += v16;
      v35 = v60 - v32;
      v68 = v16 + j;
      v66 += v16;
      if ( v60 - v32 > v16 )
        v35 = v16;
      v36 = v54 - v32;
      v37 = 1 << (v54 - v32);
      if ( v37 > v56 + 1 )
      {
        v38 = v59;
        v39 = -1 - v56 + v37;
        if ( v36 < v35 && ++v36 < v35 )
        {
          do
          {
            v40 = v38[1];
            ++v38;
            v41 = 2 * v39;
            if ( v41 <= v40 )
              break;
            v39 = v41 - v40;
            ++v36;
          }
          while ( v36 < v35 );
        }
      }
      v31 = 1 << v36;
      v42 = *a9 + (1 << v36);
      if ( v42 > 0x5A0 )
        break;
      v43 = a8 + 8 * *a9;
      v69 = &v79[v55];
      *v69 = v43;
      *a9 = v42;
      v63 = v43;
      if ( v55 )
      {
        huft_x[v55] = v61;
        LOBYTE(v64) = v36;
        v44 = v61 >> v68;
        v45 = *(v69 - 1);
        BYTE1(v64) = v53;
        v33 = ((v43 - v45) >> 3) - (v61 >> v68);
        *(_DWORD *)(v45 + 8 * v44) = v64;
        *(_DWORD *)(v45 + 8 * v44 + 4) = v33;
      }
      else
      {
        *a7 = v43;
      }
      if ( v54 <= v66 )
      {
        v10 = v61;
        goto LABEL_55;
      }
      v16 = v53;
    }
    return -3;
  }
LABEL_74:
  if ( !v22 || v60 == 1 )
    return 0;
  return -5;
}

/* ---- inflate_trees_bits  0x004A69B0 ---- */
// inflate_trees_bits -- CONFIRMED 95. Owns "oversubscribed dynamic bit lengths tree" / "incomplete dynamic bit lengths tree".
/*
 * VERIFIED 0x004A69B0 against upstream inflate_trees_bits, including the
 * 19/19 code counts and both error strings. The &v12 that vecsplit_scan.py
 * flags is huft_build's 4-byte `hn` out-parameter, not a split record.
 */
int __cdecl inflate_trees_bits(int a1, _DWORD *a2, unsigned int *a3, _DWORD *a4, int a5)
{
  unsigned int *v5; // eax
  unsigned int *v6; // esi
  int v8; // eax
  int v9; // edi
  int v10; // [esp-Ch] [ebp-18h]
  int v11; // [esp-Ch] [ebp-18h]
  unsigned int v12; // [esp+8h] [ebp-4h] BYREF

  v10 = *(_DWORD *)(a1 + 40);
  v12 = 0;
  v5 = (unsigned int *)(*(int (__cdecl **)(int, int, int))(a1 + 32))(v10, 19, 4);
  v6 = v5;
  if ( !v5 )
    return -4;
  v8 = huft_build(a3, a2, 0x13u, 0x13u, 0, 0, a4, a5, &v12, v5);
  v9 = v8;
  if ( v8 == -3 )
  {
    v11 = *(_DWORD *)(a1 + 40);
    *(_DWORD *)(a1 + 24) = "oversubscribed dynamic bit lengths tree";
    (*(void (__cdecl **)(int, unsigned int *))(a1 + 36))(v11, v6);
    return -3;
  }
  else
  {
    if ( v8 == -5 || !*a3 )
    {
      *(_DWORD *)(a1 + 24) = "incomplete dynamic bit lengths tree";
      v9 = -3;
    }
    (*(void (__cdecl **)(_DWORD, unsigned int *))(a1 + 36))(*(_DWORD *)(a1 + 40), v6);
    return v9;
  }
}

/* ---- inflate_trees_dynamic  0x004A6A50 ---- */
/*
 * VERIFIED 0x004A6A50 against upstream inflate_trees_dynamic -- both
 * huft_build calls, the 0x101 (257) literal base, and all five error strings.
 * `&a4[a2]` is the distance-code half of the shared blens array, upstream's
 * `c + nl`. Exercised on every dynamic block in the stock paks.
 */
int __cdecl inflate_trees_dynamic(
        int a1,
        unsigned int a2,
        unsigned int a3,
        _DWORD *a4,
        unsigned int *a5,
        unsigned int *a6,
        _DWORD *a7,
        _DWORD *a8,
        int a9)
{
  unsigned int *v9; // eax
  unsigned int *v10; // esi
  int v12; // eax
  int v13; // edi
  int v14; // eax
  int v15; // [esp-Ch] [ebp-18h]
  int v16; // [esp-Ch] [ebp-18h]
  int v17; // [esp-Ch] [ebp-18h]
  int v18; // [esp-Ch] [ebp-18h]
  unsigned int v19; // [esp+8h] [ebp-4h] BYREF

  v15 = *(_DWORD *)(a1 + 40);
  v19 = 0;
  v9 = (unsigned int *)(*(int (__cdecl **)(int, int, int))(a1 + 32))(v15, 288, 4);
  v10 = v9;
  if ( !v9 )
    return -4;
  v12 = huft_build(a5, a4, a2, 0x101u, (int)cplens, (int)cplext, a7, a9, &v19, v9);
  v13 = v12;
  if ( v12 )
  {
    if ( v12 == -3 )
    {
      v18 = *(_DWORD *)(a1 + 40);
      *(_DWORD *)(a1 + 24) = "oversubscribed literal/length tree";
      (*(void (__cdecl **)(int, unsigned int *))(a1 + 36))(v18, v10);
      return -3;
    }
    if ( v12 == -4 )
      goto LABEL_20;
LABEL_19:
    *(_DWORD *)(a1 + 24) = "incomplete literal/length tree";
    v13 = -3;
    goto LABEL_20;
  }
  if ( !*a5 )
    goto LABEL_19;
  v14 = huft_build(a6, &a4[a2], a3, 0, (int)cpdist, (int)cpdext, a8, a9, &v19, v10);
  v13 = v14;
  if ( v14 )
  {
    switch ( v14 )
    {
      case -3:
        v16 = *(_DWORD *)(a1 + 40);
        *(_DWORD *)(a1 + 24) = "oversubscribed distance tree";
        (*(void (__cdecl **)(int, unsigned int *))(a1 + 36))(v16, v10);
        return -3;
      case -5:
        v17 = *(_DWORD *)(a1 + 40);
        *(_DWORD *)(a1 + 24) = "incomplete distance tree";
        (*(void (__cdecl **)(int, unsigned int *))(a1 + 36))(v17, v10);
        return -3;
      case -4:
        goto LABEL_20;
    }
  }
  else if ( *a6 || a2 <= 0x101 )
  {
    (*(void (__cdecl **)(_DWORD, unsigned int *))(a1 + 36))(*(_DWORD *)(a1 + 40), v10);
    return 0;
  }
  *(_DWORD *)(a1 + 24) = "empty distance tree with lengths";
  v13 = -3;
LABEL_20:
  (*(void (__cdecl **)(_DWORD, unsigned int *))(a1 + 36))(*(_DWORD *)(a1 + 40), v10);
  return v13;
}

/* ---- inflate_trees_fixed  0x004A6BD0 ---- */
// inflate_trees_fixed -- CONFIRMED 88. Hands back the two static tables at 0x571C58 and 0x572C58 with fixed bit counts -- no huft_build call, which is what distinguishes it.
/*
 * VERIFIED 0x004A6BD0 BY OUTPUT EQUIVALENCE rather than by shape, because the
 * rewrite deliberately diverges: retail loads precompiled tables, this builds
 * them once at first use. The tables it produces were dumped and are SHA-1
 * identical to retail's fixed_tl at 0x00571C58 and fixed_td at 0x00572C58,
 * all 4,352 bytes, with bl/bd = 9/5. The divergence is in method only.
 */
/*
 * The recovered body was unusable twice over.  It is __usercall with `bl` in
 * ECX, which the rewrite to __cdecl dropped, so the emitted code stored through
 * an uninitialised `v3` (VC6 warns C4700 on it); and the two tables it hands
 * back are the zeroed placeholders.  Rewritten with zlib's own signature and
 * backed by inf_build_fixed.
 *
 * Nothing else in the tree calls this -- retail inlines it into inflate_blocks'
 * fixed-block case -- so the argument order is free; zlib's is used.
 */
int __cdecl inflate_trees_fixed( unsigned int *bl, unsigned int *bd,
                                 _DWORD *tl, _DWORD *td )
{
  inf_build_fixed();
  *bl = inf_fixed_bl;
  *bd = inf_fixed_bd;
  *tl = (_DWORD)inf_fixed_tl;
  *td = (_DWORD)inf_fixed_td;
  return 0;
}

/* ---- inflate_flush  0x004A6C00 ---- */
// inflate_flush -- CONFIRMED 92. infutil.c: copies as much of the window as will fit into next_out, updating check and total_out. Callers: inflate_blocks, inflate_codes.
/*
 * VERIFIED 0x004A6C00 line by line against upstream infutil.c inflate_flush,
 * including the window-wrap half. The apparent v19/v6 and v20/v14 duplicates
 * are Hex-Rays keeping two names for one value -- v19 is assigned v6 and then
 * both are set to avail_out on the clamp, so they are always equal, and the
 * pointer advance uses the clamped count as upstream does. Field map:
 * s->window 40, end 44, read 48, write 52, checkfn 56, check 60; z->next_out
 * 12, avail_out 16, total_out 20, adler 48.
 */
int __cdecl inflate_flush(_DWORD *a1, _DWORD *a2, int a3)
{
  char *v3; // ebp
  unsigned int v4; // esi
  unsigned int v5; // eax
  unsigned int v6; // esi
  unsigned int v7; // edx
  int (__cdecl *v8)(_DWORD, char *, unsigned int); // eax
  int v9; // eax
  char *v10; // ebp
  char *v11; // eax
  char *v12; // ebp
  unsigned int v13; // eax
  unsigned int v14; // esi
  unsigned int v15; // edx
  int (__cdecl *v16)(_DWORD, char *, unsigned int); // eax
  int v17; // eax
  int result; // eax
  unsigned int v19; // [esp+Ch] [ebp-8h]
  unsigned int v20; // [esp+Ch] [ebp-8h]
  char *v21; // [esp+10h] [ebp-4h]
  char *v22; // [esp+10h] [ebp-4h]

  v3 = (char *)a1[12];
  v4 = a1[13];
  v21 = (char *)a2[3];
  if ( (unsigned int)v3 > v4 )
    v4 = a1[11];
  v5 = a2[4];
  v6 = v4 - (_DWORD)v3;
  v19 = v6;
  if ( v6 > v5 )
  {
    v19 = a2[4];
    v6 = v19;
  }
  if ( v6 )
  {
    if ( a3 == -5 )
      a3 = 0;
  }
  v7 = v6 + a2[5];
  a2[4] = v5 - v6;
  a2[5] = v7;
  v8 = (int (__cdecl *)(_DWORD, char *, unsigned int))a1[14];
  if ( v8 )
  {
    v9 = v8(a1[15], v3, v6);
    a1[15] = v9;
    a2[12] = v9;
  }
  qmemcpy(v21, v3, v6);
  v10 = &v3[v19];
  v11 = (char *)a1[11];
  v22 = &v21[v19];
  if ( v10 == v11 )
  {
    v12 = (char *)a1[10];
    if ( (char *)a1[13] == v11 )
      a1[13] = v12;
    v13 = a2[4];
    v14 = a1[13] - (_DWORD)v12;
    v20 = v14;
    if ( v14 > v13 )
    {
      v20 = a2[4];
      v14 = v20;
    }
    if ( v14 )
    {
      if ( a3 == -5 )
        a3 = 0;
    }
    v15 = v14 + a2[5];
    a2[4] = v13 - v14;
    a2[5] = v15;
    v16 = (int (__cdecl *)(_DWORD, char *, unsigned int))a1[14];
    if ( v16 )
    {
      v17 = v16(a1[15], v12, v14);
      a1[15] = v17;
      a2[12] = v17;
    }
    qmemcpy(v22, v12, v14);
    v22 += v20;
    v10 = &v12[v20];
  }
  a2[3] = v22;
  result = a3;
  a1[12] = v10;
  return result;
}

/* ---- nullsub_68  0x004A6D40 ---- */
void nullsub_68()
{
  ;
}

/* ---- _tr_init  0x004A6D50  VERIFIED ---- */
/*
 * THIS IS zlib trees.c `_tr_init`.  It WAS shadowed by a link stub; FIXED
 * 2026-08-15 by the qcommon/universal owner, who took zlib on for this.
 *
 * The shadow, as it stood:
 *     body   inflate.obj      @_tr_init@8   (__fastcall)
 *     stub   link_stubs.obj   _sub_4A6D50     (__cdecl)   <-- won the link
 *     caller universal/memorytree.cpp  `extern int _tr_init();`
 *                                      _tr_init(0, (int)v23)
 * Different decoration, so no LNK2005 and no warning: the body below had never
 * run, and the Huffman tree state was never initialised.
 *
 * THE FIX IS THE ONE THIS FILE ALREADY USES FOR sub_4A6DB0, twenty lines down:
 * declare the definition __cdecl so it decorates as plain `_sub_4A6D50` and
 * therefore collides with -- rather than hiding behind -- the stub, and delete
 * the stub in the same change.  qcommon/link_stubs.c's
 * `int _tr_init() { return 0; }` is removed.  No call site changes: every
 * caller already spells it __cdecl, which is exactly why the shadow existed.
 *
 * Proved at the symbol level, not by reading the source, because the source
 * reads correctly either way -- `dumpbin /SYMBOLS` now shows `_sub_4A6D50` as
 * SECT4 External (a definition) in inflate.obj and UNDEF in memorytree.obj,
 * where before inflate.obj carried only `@_tr_init@8` and nothing resolved
 * `_sub_4A6D50` but the stub.
 *
 * THE RECOVERED SIGNATURE IS STILL WRONG, and is kept deliberately.  Retail
 * 0x004A6D50 is fourteen instructions and touches ONE register argument:
 * everything is `[edx + ...]`, and ECX is written as scratch at 0x004A6D6A
 * before it is ever read.  So retail is __usercall s@<edx>, one argument, and
 * Hex-Rays' `(int a1, int a2)` has a phantom a1 -- which is why the call site
 * passes a literal 0 for it.  The IDB now carries `__usercall(s@<edx>)`.  The
 * tree keeps the two-parameter spelling so the caller needs no edit and the
 * dummy is harmless: `a1` is not read below.  Same disposition as sub_4A6DB0
 * and as Com_SkipRestOfLine in universal/q_parse.c.
 *
 * SEVERITY IS LOW AND THAT IS NOT A GUESS: this is the deflate side.  Its only
 * caller is deflateInit2_ (0x004A2E20), reached only from compress2 (0x004A2C20),
 * reached only from compress (0x004A2CC0), which has no xrefs in the retail
 * binary at all.  Nothing in a dedicated server compresses anything.  Unshadowing
 * it removes a latent wrong-code path rather than fixing a live one.
 *
 * It also tail-calls sub_4A6DB0 = trees.c `init_block`, which is real below and
 * no longer stubbed, so the deflate init path is now whole.
 */
_WORD *__cdecl _tr_init(int unused, int s)
{
  *(_DWORD *)(s + 2832) = s + 140;
  *(_DWORD *)(s + 2856) = s + 2676;
  *(_DWORD *)(s + 2840) = &off_571BC8;
  *(_DWORD *)(s + 2844) = s + 2432;
  *(_DWORD *)(s + 2852) = &off_571BDC;
  *(_DWORD *)(s + 2864) = &unk_571BF0;
  *(_WORD *)(s + 5808) = 0;
  *(_DWORD *)(s + 5812) = 0;
  *(_DWORD *)(s + 5804) = 8;
  return sub_4A6DB0(s + 2432, s);
}

/* ---- sub_4A6DB0  0x004A6DB0  VERIFIED ----
 *
 * WAS STUB-ONLY.  Written 2026-08-14 from the coverage-gap audit; the only
 * definition in the tree was `int sub_4A6DB0() { return 0; }` in
 * qcommon/link_stubs.c.  THAT LINE MUST NOW BE DELETED -- this definition is
 * __cdecl, deliberately, so the two decorate identically (`_sub_4A6DB0`) and
 * the collision is a loud LNK2005 rather than the silent shadow that has kept
 * _tr_init next door from ever running.
 *
 * THIS IS zlib 1.1.4 trees.c `init_block`, and the identification is exact
 * rather than by analogy -- the three loop counts are zlib's three code-count
 * constants and nothing else has that triple:
 *
 *     0x004A6DB7  mov ecx, 11Eh   286 = L_CODES    over s->dyn_ltree
 *     0x004A6DCF  mov ecx, 1Eh     30 = D_CODES    over s->dyn_dtree
 *     0x004A6DE3  mov ecx, 13h     19 = BL_CODES   over s->bl_tree
 *
 * and the strides confirm the ct_data layout: each loop stores a WORD (the
 * .Freq half) and advances by 4 (sizeof ct_data).  The base offsets fall out
 * of deflate_state exactly -- dyn_ltree is ct_data[HEAP_SIZE] = [573], so
 * 0x8C + 573*4 = 0x980 is dyn_dtree, and 0x980 + 61*4 = 0xA74 is bl_tree.
 *
 *     s + 0x008C  dyn_ltree[573]
 *     s + 0x0980  dyn_dtree[61]
 *     s + 0x0A74  bl_tree[39]
 *     s + 0x048C  dyn_ltree[END_BLOCK].Freq   0x8C + 256*4, set to 1
 *     s + 0x1698  last_lit    = 0
 *     s + 0x16A0  opt_len     = 0
 *     s + 0x16A4  static_len  = 0
 *     s + 0x16A8  matches     = 0
 *
 * all four of the last group written as DWORDS (`mov [edx+...], esi` with esi
 * zeroed at 0x004A6DBE), and only the END_BLOCK store is a word.
 *
 * ARGUMENT COUNT: ONE, in EDX.  Every access is `[edx + ...]`; ECX is loaded
 * with a literal at 0x004A6DB7 before it is ever read, so Hex-Rays' first
 * parameter is a phantom -- the same shape _tr_init above has, and the
 * reason both call sites in this file pass a junk value first.  The parameter
 * is kept in the signature so those two emitted call sites stay correct;
 * `unused` is the honest name for it.
 *
 * RETURN VALUE: zlib's init_block is void, but retail leaves EAX at
 * s + 0xA74 + 19*4 = s + 0xAC0, the address one past the end of bl_tree,
 * because that is where the third loop's cursor stopped.  sub_4A6E20 below
 * assigns that value and can return it, so it is reproduced rather than
 * dropped -- it is junk in both, but it is the SAME junk.
 *
 * SEVERITY: low, and for the reason the _tr_init note above already
 * establishes -- this is the deflate side, whose only route in is
 * compress() (0x004A2CC0), which has no xrefs in the retail binary at all.
 * Nothing a dedicated server does compresses anything.  Written because the
 * pair was the last thing keeping trees.c from being reconstructable, not
 * because anything was failing.
 */
short *__cdecl sub_4A6DB0( int unused, int s )
{
	short  *p;
	int     n;

	(void)unused;

	p = (short *)( s + 0x008C );            /* dyn_ltree */
	for ( n = 286 ; n ; n-- ) {             /* L_CODES */
		*p = 0;
		p = (short *)( (char *)p + 4 );
	}

	p = (short *)( s + 0x0980 );            /* dyn_dtree */
	for ( n = 30 ; n ; n-- ) {              /* D_CODES */
		*p = 0;
		p = (short *)( (char *)p + 4 );
	}

	p = (short *)( s + 0x0A74 );            /* bl_tree */
	for ( n = 19 ; n ; n-- ) {              /* BL_CODES */
		*p = 0;
		p = (short *)( (char *)p + 4 );
	}

	*(int *)( s + 0x16A4 ) = 0;             /* static_len */
	*(int *)( s + 0x16A0 ) = 0;             /* opt_len */
	*(int *)( s + 0x16A8 ) = 0;             /* matches */
	*(int *)( s + 0x1698 ) = 0;             /* last_lit */

	*(short *)( s + 0x048C ) = 1;           /* dyn_ltree[END_BLOCK].Freq */

	return p;
}

/* ---- sub_4A6E20  0x004A6E20 ---- */
int __cdecl sub_4A6E20(int result, int a2, int a3)
{
  int v3; // edx
  int v4; // ebp
  int v5; // ecx
  bool v6; // cc
  int v7; // esi
  int v8; // ebp
  unsigned __int16 v9; // dx
  unsigned __int16 v10; // bx
  int v11; // esi
  unsigned __int16 v12; // dx
  unsigned __int16 v13; // bx
  int v14; // edx
  int v15; // [esp+Ch] [ebp-4h]

  v3 = *(_DWORD *)(result + 5192);
  v4 = *(_DWORD *)(result + 4 * a3 + 2900);
  v5 = 2 * a3;
  v6 = 2 * a3 < v3;
  v15 = v4;
  if ( 2 * a3 > v3 )
  {
    *(_DWORD *)(result + 4 * a3 + 2900) = v4;
  }
  else
  {
    while ( 1 )
    {
      if ( v6 )
      {
        v7 = *(_DWORD *)(result + 4 * v5 + 2904);
        v8 = *(_DWORD *)(result + 4 * v5 + 2900);
        v9 = *(_WORD *)(a2 + 4 * v7);
        v10 = *(_WORD *)(a2 + 4 * v8);
        if ( v9 < v10 || v9 == v10 && *(_BYTE *)(v7 + result + 5200) <= *(_BYTE *)(result + v8 + 5200) )
          ++v5;
        v4 = v15;
      }
      v11 = *(_DWORD *)(result + 4 * v5 + 2900);
      v12 = *(_WORD *)(a2 + 4 * v4);
      v13 = *(_WORD *)(a2 + 4 * v11);
      if ( v12 < v13 )
      {
LABEL_12:
        *(_DWORD *)(result + 4 * a3 + 2900) = v4;
        return result;
      }
      if ( v12 == v13 && *(_BYTE *)(result + v4 + 5200) <= *(_BYTE *)(v11 + result + 5200) )
        break;
      *(_DWORD *)(result + 4 * a3 + 2900) = v11;
      v14 = *(_DWORD *)(result + 5192);
      a3 = v5;
      v5 *= 2;
      v6 = v5 < v14;
      if ( v5 > v14 )
        goto LABEL_12;
    }
    *(_DWORD *)(result + 4 * a3 + 2900) = v4;
  }
  return result;
}

/* ---- sub_4A6EF0  0x004A6EF0 ---- */
int __cdecl sub_4A6EF0(int *a1, _DWORD *a2)
{
  int v2; // ecx
  int v3; // ebx
  int *v4; // eax
  int v5; // ebp
  int v6; // esi
  int result; // eax
  int v8; // ecx
  int v9; // ecx
  int v10; // eax
  int v11; // esi
  int v12; // edi
  bool v13; // zf
  int v14; // edi
  _WORD *v15; // esi
  int v16; // eax
  _WORD *i; // ecx
  int v18; // ecx
  int v19; // esi
  _DWORD *v20; // ebp
  int v21; // edi
  int v22; // [esp+10h] [ebp-20h]
  int *v23; // [esp+14h] [ebp-1Ch]
  int v24; // [esp+18h] [ebp-18h]
  int v25; // [esp+18h] [ebp-18h]
  int v26; // [esp+1Ch] [ebp-14h]
  int v27; // [esp+20h] [ebp-10h]
  _WORD *v28; // [esp+20h] [ebp-10h]
  int v29; // [esp+24h] [ebp-Ch]
  int v30; // [esp+28h] [ebp-8h]
  int v31; // [esp+2Ch] [ebp-4h]

  v2 = a1[1];
  v3 = *a1;
  v4 = (int *)a1[2];
  v29 = v2;
  v31 = v4[1];
  v5 = *v4;
  v30 = v4[2];
  v6 = v4[4];
  memset(a2 + 717, 0, 0x20u);
  result = a2[1299];
  *(_WORD *)(v3 + 4 * a2[result + 725] + 2) = 0;
  v8 = a2[1299] + 1;
  v24 = v6;
  v22 = 0;
  if ( v8 < 573 )
  {
    v23 = &a2[v8 + 725];
    v27 = 573 - v8;
    v26 = 573;
    do
    {
      v9 = *v23;
      v10 = *(unsigned __int16 *)(v3 + 4 * *(unsigned __int16 *)(v3 + 4 * *v23 + 2) + 2) + 1;
      if ( v10 > v6 )
      {
        v10 = v6;
        ++v22;
      }
      *(_WORD *)(v3 + 4 * v9 + 2) = v10;
      if ( v9 <= v29 )
      {
        ++*((_WORD *)a2 + v10 + 1434);
        v11 = 0;
        if ( v9 >= v30 )
          v11 = *(_DWORD *)(v31 + 4 * (v9 - v30));
        v12 = *(unsigned __int16 *)(v3 + 4 * v9);
        a2[1448] += v12 * (v11 + v10);
        if ( v5 )
          a2[1449] += v12 * (v11 + *(unsigned __int16 *)(v5 + 4 * v9 + 2));
        v6 = v24;
      }
      v13 = v27 == 1;
      ++v23;
      --v27;
    }
    while ( !v13 );
    result = v22;
    if ( v22 )
    {
      v14 = v6 - 1;
      v15 = (_WORD *)a2 + v6 + 1434;
      do
      {
        v16 = v14;
        for ( i = (_WORD *)a2 + v14 + 1434; !*i; --v16 )
          --i;
        --*((_WORD *)a2 + v16 + 1434);
        *((_WORD *)a2 + v16 + 1435) += 2;
        --*v15;
        result = v22 - 2;
        v22 = result;
      }
      while ( result > 0 );
      v18 = v24;
      if ( v24 )
      {
        v28 = v15;
        do
        {
          v19 = (unsigned __int16)*v15;
          v25 = v19;
          if ( v19 )
          {
            v20 = &a2[v26 + 725];
            do
            {
              result = *--v20;
              --v26;
              if ( result <= v29 )
              {
                v21 = *(unsigned __int16 *)(v3 + 4 * result + 2);
                if ( v21 != v18 )
                {
                  a2[1448] += *(unsigned __int16 *)(v3 + 4 * result) * (v18 - v21);
                  *(_WORD *)(v3 + 4 * result + 2) = v18;
                }
                result = v25 - 1;
                v25 = result;
                v19 = result;
              }
            }
            while ( v19 );
          }
          --v18;
          v15 = --v28;
        }
        while ( v18 );
      }
    }
  }
  return result;
}

/* ---- sub_4A7100  0x004A7100 ---- */
int __cdecl sub_4A7100(int a1, int a2, int a3)
{
  __int16 v3; // cx
  int result; // eax
  int v5; // edx
  int i; // esi
  int v7; // edx
  unsigned __int16 v8; // ax
  unsigned int v9; // ecx
  unsigned int v10; // eax
  int v11; // ebp
  __int16 v12; // [esp+4h] [ebp-24h]
  char v13; // [esp+6h] [ebp-22h] BYREF
  unsigned int v14; // [esp+24h] [ebp-4h]
  unsigned int retaddr; // [esp+28h] [ebp+0h]

  v14 = retaddr ^ _security_cookie;
  v3 = 0;
  result = 1;
  v5 = a1 - (_DWORD)&v13;
  do
  {
    v3 = 2 * (v3 + *(__int16 *)((char *)&v12 + 2 * result + v5));
    *(&v12 + result++) = v3;
  }
  while ( result <= 15 );
  for ( i = 0; i <= a2; ++i )
  {
    v7 = *(unsigned __int16 *)(a3 + 4 * i + 2);
    if ( *(_WORD *)(a3 + 4 * i + 2) )
    {
      v8 = *(&v12 + v7);
      v9 = v8;
      *(&v12 + v7) = v8 + 1;
      v10 = 0;
      do
      {
        v11 = v9 & 1;
        v9 >>= 1;
        v10 = 2 * (v11 | v10);
        --v7;
      }
      while ( v7 > 0 );
      result = v10 >> 1;
      *(_WORD *)(a3 + 4 * i) = result;
    }
  }
  return result;
}

/* ---- sub_4A7190  0x004A7190 ---- */
int __cdecl sub_4A7190(_DWORD *a1, int *a2)
{
  int v2; // edi
  int *v3; // eax
  int v4; // ecx
  int v5; // ebx
  int v6; // ebp
  int v7; // eax
  int v8; // ecx
  int v9; // eax
  int v10; // ecx
  int i; // ebx
  int v12; // ebp
  int v13; // eax
  int v14; // edx
  int v15; // ebx
  int v16; // eax
  int v17; // edx
  int v18; // ecx
  unsigned __int8 v19; // cl
  int v20; // edx
  int v21; // eax
  int v23; // [esp+Ch] [ebp-8h]
  int v24; // [esp+10h] [ebp-4h]

  v2 = *a2;
  v3 = (int *)a2[2];
  v4 = v3[3];
  v5 = *v3;
  v6 = -1;
  v7 = 0;
  v24 = v4;
  v23 = -1;
  a1[1298] = 0;
  for ( a1[1299] = 573; v7 < v24; ++v7 )
  {
    if ( *(_WORD *)(v2 + 4 * v7) )
    {
      v8 = a1[1298] + 1;
      a1[1298] = v8;
      a1[v8 + 725] = v7;
      v23 = v7;
      *((_BYTE *)a1 + v7 + 5200) = 0;
      v6 = v7;
    }
    else
    {
      *(_WORD *)(v2 + 4 * v7 + 2) = 0;
    }
  }
  if ( (int)a1[1298] < 2 )
  {
    do
    {
      if ( v6 >= 2 )
        v9 = 0;
      else
        v9 = ++v6;
      v10 = a1[1298] + 1;
      a1[1298] = v10;
      a1[v10 + 725] = v9;
      *(_WORD *)(v2 + 4 * v9) = 1;
      *((_BYTE *)a1 + v9 + 5200) = 0;
      --a1[1448];
      if ( v5 )
        a1[1449] -= *(unsigned __int16 *)(v5 + 4 * v9 + 2);
    }
    while ( (int)a1[1298] < 2 );
    v23 = v6;
  }
  a2[1] = v6;
  for ( i = a1[1298] / 2; i >= 1; --i )
    sub_4A6E20((int)a1, v2, i);
  v12 = v24;
  do
  {
    v13 = a1[1298];
    v14 = a1[v13 + 725];
    v15 = a1[726];
    a1[1298] = v13 - 1;
    a1[726] = v14;
    sub_4A6E20((int)a1, v2, 1);
    v16 = a1[726];
    v17 = a1[1299] - 1;
    a1[1299] = v17;
    a1[v17 + 725] = v15;
    v18 = a1[1299] - 1;
    a1[1299] = v18;
    a1[v18 + 725] = v16;
    *(_WORD *)(v2 + 4 * v12) = *(_WORD *)(v2 + 4 * v15) + *(_WORD *)(v2 + 4 * v16);
    v19 = *((_BYTE *)a1 + v16 + 5200);
    if ( *((_BYTE *)a1 + v15 + 5200) >= v19 )
      v19 = *((_BYTE *)a1 + v15 + 5200);
    *((_BYTE *)a1 + v12 + 5200) = v19 + 1;
    *(_WORD *)(v2 + 4 * v16 + 2) = v12;
    *(_WORD *)(v2 + 4 * v15 + 2) = v12;
    a1[726] = v12++;
    sub_4A6E20((int)a1, v2, 1);
  }
  while ( (int)a1[1298] >= 2 );
  v20 = a1[726];
  v21 = a1[1299] - 1;
  a1[1299] = v21;
  a1[v21 + 725] = v20;
  sub_4A6EF0(a2, a1);
  return sub_4A7100((int)(a1 + 717), v23, v2);
}

/* ---- sub_4A7390  0x004A7390 ---- */
unsigned __int16 *__cdecl sub_4A7390(unsigned __int16 *result, int a2, _WORD *a3)
{
  int v3; // edi
  int v4; // ebp
  int v5; // esi
  int v7; // ecx
  int v8; // edx
  int v9; // ebx
  int v10; // eax
  unsigned __int16 *v11; // [esp+10h] [ebp-4h]

  v3 = result[1];
  v4 = -1;
  v5 = 0;
  v7 = 7;
  v8 = 4;
  if ( !result[1] )
  {
    v7 = 138;
    v8 = 3;
  }
  result[2 * a2 + 3] = -1;
  if ( a2 >= 0 )
  {
    v11 = result + 3;
    v9 = a2 + 1;
    do
    {
      v10 = v3;
      v3 = *v11;
      if ( ++v5 >= v7 || v10 != v3 )
      {
        if ( v5 >= v8 )
        {
          if ( v10 )
          {
            if ( v10 != v4 )
              ++a3[2 * v10 + 1338];
            ++a3[1370];
          }
          else if ( v5 > 10 )
          {
            ++a3[1374];
          }
          else
          {
            ++a3[1372];
          }
        }
        else
        {
          a3[2 * v10 + 1338] += v5;
        }
        v5 = 0;
        v4 = v10;
        if ( v3 )
        {
          if ( v10 == v3 )
          {
            v7 = 6;
            v8 = 3;
          }
          else
          {
            v7 = 7;
            v8 = 4;
          }
        }
        else
        {
          v7 = 138;
          v8 = 3;
        }
      }
      result = v11 + 2;
      --v9;
      v11 += 2;
    }
    while ( v9 );
  }
  return result;
}

/* ---- sub_4A7470  0x004A7470 ---- */
int __cdecl sub_4A7470(int result, int a2, int a3)
{
  int v4; // esi
  int v5; // ebx
  int v6; // ecx
  int v7; // edx
  int v8; // ebp
  int v9; // edi
  int v10; // ecx
  unsigned __int16 v11; // si
  int v12; // edx
  int v13; // ecx
  char v14; // bl
  int v15; // edx
  int v16; // edx
  int v17; // ecx
  int v18; // edi
  int v19; // ecx
  unsigned __int16 v20; // si
  int v21; // edx
  int v22; // ecx
  char v23; // bl
  int v24; // edx
  int v25; // edx
  int v26; // ecx
  int v27; // edi
  int v28; // ecx
  unsigned __int16 v29; // si
  int v30; // edx
  int v31; // ecx
  char v32; // bl
  int v33; // edx
  int v34; // edx
  int v35; // ecx
  int v36; // esi
  int v37; // edx
  int v38; // ecx
  char v39; // bl
  int v40; // edx
  int v41; // edx
  int v42; // ecx
  int v43; // ecx
  int v44; // edi
  unsigned __int16 v45; // si
  int v46; // edx
  int v47; // ecx
  char v48; // bl
  int v49; // edx
  int v50; // edx
  int v51; // ecx
  int v52; // esi
  int v53; // edx
  int v54; // ecx
  char v55; // bl
  int v56; // edx
  int v57; // edx
  int v58; // edi
  unsigned __int16 v59; // si
  int v60; // edx
  int v61; // ecx
  char v62; // bl
  int v63; // edx
  int v64; // edx
  int v65; // ecx
  int v66; // esi
  int v67; // edx
  int v68; // ecx
  char v69; // bl
  int v70; // edx
  int v71; // edx
  bool v72; // zf
  int v73; // [esp+10h] [ebp-Ch]
  unsigned __int16 *v74; // [esp+14h] [ebp-8h]
  int v75; // [esp+18h] [ebp-4h]
  int v76; // [esp+20h] [ebp+4h]

  v73 = *(unsigned __int16 *)(a2 + 2);
  v4 = 0;
  v5 = -1;
  v6 = 7;
  v7 = 4;
  if ( !v73 )
  {
    v6 = 138;
    v7 = 3;
  }
  if ( a3 >= 0 )
  {
    v74 = (unsigned __int16 *)(a2 + 6);
    v75 = a3 + 1;
    do
    {
      v8 = v73;
      ++v4;
      v73 = *v74;
      v76 = v4;
      if ( v4 < v6 && v8 == *v74 )
        goto LABEL_44;
      if ( v4 < v7 )
      {
        do
        {
          v9 = *(unsigned __int16 *)(result + 4 * v8 + 2678);
          v10 = *(_DWORD *)(result + 5812);
          if ( v10 <= 16 - v9 )
          {
            *(_WORD *)(result + 5808) |= *(_WORD *)(result + 4 * v8 + 2676) << v10;
            v17 = v9 + v10;
          }
          else
          {
            v11 = *(_WORD *)(result + 4 * v8 + 2676);
            v12 = v11 << v10;
            v13 = *(_DWORD *)(result + 8);
            *(_WORD *)(result + 5808) |= v12;
            *(_BYTE *)(v13 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
            v14 = *(_BYTE *)(result + 5809);
            v15 = *(_DWORD *)(result + 20) + 1;
            *(_DWORD *)(result + 20) = v15;
            *(_BYTE *)(v15 + *(_DWORD *)(result + 8)) = v14;
            v16 = *(_DWORD *)(result + 5812);
            ++*(_DWORD *)(result + 20);
            v17 = v16 + v9 - 16;
            *(_WORD *)(result + 5808) = v11 >> (16 - v16);
            v4 = v76;
          }
          --v4;
          *(_DWORD *)(result + 5812) = v17;
          v76 = v4;
        }
        while ( v4 );
        goto LABEL_39;
      }
      if ( v8 )
      {
        if ( v8 != v5 )
        {
          v18 = *(unsigned __int16 *)(result + 4 * v8 + 2678);
          v19 = *(_DWORD *)(result + 5812);
          if ( v19 <= 16 - v18 )
          {
            *(_WORD *)(result + 5808) |= *(_WORD *)(result + 4 * v8 + 2676) << v19;
            v26 = v18 + v19;
          }
          else
          {
            v20 = *(_WORD *)(result + 4 * v8 + 2676);
            v21 = v20 << v19;
            v22 = *(_DWORD *)(result + 8);
            *(_WORD *)(result + 5808) |= v21;
            *(_BYTE *)(v22 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
            v23 = *(_BYTE *)(result + 5809);
            v24 = *(_DWORD *)(result + 20) + 1;
            *(_DWORD *)(result + 20) = v24;
            *(_BYTE *)(v24 + *(_DWORD *)(result + 8)) = v23;
            v25 = *(_DWORD *)(result + 5812);
            ++*(_DWORD *)(result + 20);
            v26 = v25 + v18 - 16;
            *(_WORD *)(result + 5808) = v20 >> (16 - v25);
            v4 = v76;
          }
          --v4;
          *(_DWORD *)(result + 5812) = v26;
          v76 = v4;
        }
        v27 = *(unsigned __int16 *)(result + 2742);
        v28 = *(_DWORD *)(result + 5812);
        if ( v28 <= 16 - v27 )
        {
          *(_WORD *)(result + 5808) |= *(_WORD *)(result + 2740) << v28;
          v35 = v27 + v28;
        }
        else
        {
          v29 = *(_WORD *)(result + 2740);
          v30 = v29 << v28;
          v31 = *(_DWORD *)(result + 8);
          *(_WORD *)(result + 5808) |= v30;
          *(_BYTE *)(v31 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
          v32 = *(_BYTE *)(result + 5809);
          v33 = *(_DWORD *)(result + 20) + 1;
          *(_DWORD *)(result + 20) = v33;
          *(_BYTE *)(v33 + *(_DWORD *)(result + 8)) = v32;
          v34 = *(_DWORD *)(result + 5812);
          ++*(_DWORD *)(result + 20);
          v35 = v34 + v27 - 16;
          *(_WORD *)(result + 5808) = v29 >> (16 - v34);
          v4 = v76;
        }
        v36 = v4 - 3;
        *(_DWORD *)(result + 5812) = v35;
        if ( v35 > 14 )
        {
          v37 = v36 << v35;
          v38 = *(_DWORD *)(result + 8);
          *(_WORD *)(result + 5808) |= v37;
          *(_BYTE *)(v38 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
          v39 = *(_BYTE *)(result + 5809);
          v40 = *(_DWORD *)(result + 20) + 1;
          *(_DWORD *)(result + 20) = v40;
          *(_BYTE *)(v40 + *(_DWORD *)(result + 8)) = v39;
          v41 = *(_DWORD *)(result + 5812);
          ++*(_DWORD *)(result + 20);
          *(_DWORD *)(result + 5812) = v41 - 14;
          *(_WORD *)(result + 5808) = (unsigned __int16)v36 >> (16 - v41);
          goto LABEL_39;
        }
        *(_WORD *)(result + 5808) |= v36 << v35;
        v42 = v35 + 2;
      }
      else
      {
        v43 = *(_DWORD *)(result + 5812);
        if ( v4 > 10 )
        {
          v58 = *(unsigned __int16 *)(result + 2750);
          if ( v43 <= 16 - v58 )
          {
            *(_WORD *)(result + 5808) |= *(_WORD *)(result + 2748) << v43;
            v65 = v58 + v43;
          }
          else
          {
            v59 = *(_WORD *)(result + 2748);
            v60 = v59 << v43;
            v61 = *(_DWORD *)(result + 8);
            *(_WORD *)(result + 5808) |= v60;
            *(_BYTE *)(v61 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
            v62 = *(_BYTE *)(result + 5809);
            v63 = *(_DWORD *)(result + 20) + 1;
            *(_DWORD *)(result + 20) = v63;
            *(_BYTE *)(v63 + *(_DWORD *)(result + 8)) = v62;
            v64 = *(_DWORD *)(result + 5812);
            ++*(_DWORD *)(result + 20);
            v65 = v64 + v58 - 16;
            *(_WORD *)(result + 5808) = v59 >> (16 - v64);
            v4 = v76;
          }
          v66 = v4 - 11;
          *(_DWORD *)(result + 5812) = v65;
          if ( v65 > 9 )
          {
            v67 = v66 << v65;
            v68 = *(_DWORD *)(result + 8);
            *(_WORD *)(result + 5808) |= v67;
            *(_BYTE *)(v68 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
            v69 = *(_BYTE *)(result + 5809);
            v70 = *(_DWORD *)(result + 20) + 1;
            *(_DWORD *)(result + 20) = v70;
            *(_BYTE *)(v70 + *(_DWORD *)(result + 8)) = v69;
            v71 = *(_DWORD *)(result + 5812);
            ++*(_DWORD *)(result + 20);
            *(_DWORD *)(result + 5812) = v71 - 9;
            *(_WORD *)(result + 5808) = (unsigned __int16)v66 >> (16 - v71);
            goto LABEL_39;
          }
          *(_WORD *)(result + 5808) |= v66 << v65;
          v42 = v65 + 7;
        }
        else
        {
          v44 = *(unsigned __int16 *)(result + 2746);
          if ( v43 <= 16 - v44 )
          {
            *(_WORD *)(result + 5808) |= *(_WORD *)(result + 2744) << v43;
            v51 = v44 + v43;
          }
          else
          {
            v45 = *(_WORD *)(result + 2744);
            v46 = v45 << v43;
            v47 = *(_DWORD *)(result + 8);
            *(_WORD *)(result + 5808) |= v46;
            *(_BYTE *)(v47 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
            v48 = *(_BYTE *)(result + 5809);
            v49 = *(_DWORD *)(result + 20) + 1;
            *(_DWORD *)(result + 20) = v49;
            *(_BYTE *)(v49 + *(_DWORD *)(result + 8)) = v48;
            v50 = *(_DWORD *)(result + 5812);
            ++*(_DWORD *)(result + 20);
            v51 = v50 + v44 - 16;
            *(_WORD *)(result + 5808) = v45 >> (16 - v50);
            v4 = v76;
          }
          v52 = v4 - 3;
          *(_DWORD *)(result + 5812) = v51;
          if ( v51 > 13 )
          {
            v53 = v52 << v51;
            v54 = *(_DWORD *)(result + 8);
            *(_WORD *)(result + 5808) |= v53;
            *(_BYTE *)(v54 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
            v55 = *(_BYTE *)(result + 5809);
            v56 = *(_DWORD *)(result + 20) + 1;
            *(_DWORD *)(result + 20) = v56;
            *(_BYTE *)(v56 + *(_DWORD *)(result + 8)) = v55;
            v57 = *(_DWORD *)(result + 5812);
            ++*(_DWORD *)(result + 20);
            *(_DWORD *)(result + 5812) = v57 - 13;
            *(_WORD *)(result + 5808) = (unsigned __int16)v52 >> (16 - v57);
            goto LABEL_39;
          }
          *(_WORD *)(result + 5808) |= v52 << v51;
          v42 = v51 + 3;
        }
      }
      *(_DWORD *)(result + 5812) = v42;
LABEL_39:
      v4 = 0;
      v5 = v8;
      if ( v73 )
      {
        if ( v8 == v73 )
        {
          v6 = 6;
          v7 = 3;
        }
        else
        {
          v6 = 7;
          v7 = 4;
        }
      }
      else
      {
        v6 = 138;
        v7 = 3;
      }
LABEL_44:
      v72 = v75 == 1;
      v74 += 2;
      --v75;
    }
    while ( !v72 );
  }
  return result;
}

/* ---- sub_4A7980  AUTO-STUBBED ----
 * The body did not compile.  Everything recovered for it is kept in the
 * #if 0 block below -- that is the starting point for reconstructing it
 * by hand.  Until then the signature is real but the body does nothing.
 */
int sub_4A7980() { return 0; }
#if 0
int __cdecl sub_4A7980(int a1)
{
  int result; // eax

  sub_4A7390((unsigned __int16 *)(a1 + 140), *(_DWORD *)(a1 + 2836), (_WORD *)a1);
  sub_4A7390((unsigned __int16 *)(a1 + 2432), *(_DWORD *)(a1 + 2848), (_WORD *)a1);
  sub_4A7190((_DWORD *)a1, (int *)(a1 + 2856));
  for ( result = 18; result >= 3; --result )
  {
    if ( *(_WORD *)(a1 + 4 * (unsigned __int8)byte_5407AC[result] + 2678) )
      break;
  }
  *(_DWORD *)(a1 + 5792) += 3 * result + 17;
  return result;
}
#endif

/* ---- sub_4A79F0  AUTO-STUBBED ----
 * The body did not compile.  Everything recovered for it is kept in the
 * #if 0 block below -- that is the starting point for reconstructing it
 * by hand.  Until then the signature is real but the body does nothing.
 */
int sub_4A79F0() { return 0; }
#if 0
int __cdecl sub_4A79F0(int a1, int a2, int a3, int a4)
{
  int v4; // ecx
  int v5; // ebx
  int v7; // edx
  int v8; // ecx
  char v9; // bl
  int v10; // edx
  int v11; // edx
  int v12; // ecx
  unsigned __int16 v13; // si
  int v14; // edx
  int v15; // ecx
  char v16; // bl
  int v17; // edx
  int v18; // edx
  int v19; // ecx
  int v20; // edx
  int v21; // ecx
  char v22; // bl
  int v23; // edx
  int v24; // edx
  int v25; // edi
  int v26; // ecx
  int v27; // edx
  unsigned __int16 v28; // si
  int v29; // edx
  int v30; // ecx
  char v31; // bl
  int v32; // edx
  int v33; // edx
  int v34; // eax
  int v36; // [esp+1Ch] [ebp+Ch]
  int v37; // [esp+1Ch] [ebp+Ch]

  v4 = *(_DWORD *)(a1 + 5812);
  v5 = a3;
  if ( v4 <= 11 )
  {
    *(_WORD *)(a1 + 5808) |= (a2 - 257) << v4;
    *(_DWORD *)(a1 + 5812) = v4 + 5;
  }
  else
  {
    v7 = (a2 - 257) << v4;
    v8 = *(_DWORD *)(a1 + 20);
    *(_WORD *)(a1 + 5808) |= v7;
    *(_BYTE *)(v8 + *(_DWORD *)(a1 + 8)) = *(_BYTE *)(a1 + 5808);
    v9 = *(_BYTE *)(a1 + 5809);
    v10 = *(_DWORD *)(a1 + 20) + 1;
    *(_DWORD *)(a1 + 20) = v10;
    *(_BYTE *)(v10 + *(_DWORD *)(a1 + 8)) = v9;
    v11 = *(_DWORD *)(a1 + 5812);
    v5 = a3;
    ++*(_DWORD *)(a1 + 20);
    *(_DWORD *)(a1 + 5812) = v11 - 11;
    *(_WORD *)(a1 + 5808) = (unsigned __int16)(a2 - 257) >> (16 - v11);
  }
  v12 = *(_DWORD *)(a1 + 5812);
  if ( v12 <= 11 )
  {
    *(_WORD *)(a1 + 5808) |= (v5 - 1) << v12;
    *(_DWORD *)(a1 + 5812) = v12 + 5;
  }
  else
  {
    v13 = v5 - 1;
    v14 = (v5 - 1) << v12;
    v15 = *(_DWORD *)(a1 + 20);
    v36 = v5;
    *(_WORD *)(a1 + 5808) |= v14;
    *(_BYTE *)(v15 + *(_DWORD *)(a1 + 8)) = *(_BYTE *)(a1 + 5808);
    v16 = *(_BYTE *)(a1 + 5809);
    v17 = *(_DWORD *)(a1 + 20) + 1;
    *(_DWORD *)(a1 + 20) = v17;
    *(_BYTE *)(v17 + *(_DWORD *)(a1 + 8)) = v16;
    v18 = *(_DWORD *)(a1 + 5812);
    v5 = v36;
    ++*(_DWORD *)(a1 + 20);
    *(_DWORD *)(a1 + 5812) = v18 - 11;
    *(_WORD *)(a1 + 5808) = v13 >> (16 - v18);
  }
  v19 = *(_DWORD *)(a1 + 5812);
  if ( v19 <= 12 )
  {
    *(_WORD *)(a1 + 5808) |= (a4 - 4) << v19;
    *(_DWORD *)(a1 + 5812) = v19 + 4;
  }
  else
  {
    v20 = (a4 - 4) << v19;
    v21 = *(_DWORD *)(a1 + 20);
    v37 = v5;
    *(_WORD *)(a1 + 5808) |= v20;
    *(_BYTE *)(v21 + *(_DWORD *)(a1 + 8)) = *(_BYTE *)(a1 + 5808);
    v22 = *(_BYTE *)(a1 + 5809);
    v23 = *(_DWORD *)(a1 + 20) + 1;
    *(_DWORD *)(a1 + 20) = v23;
    *(_BYTE *)(v23 + *(_DWORD *)(a1 + 8)) = v22;
    v24 = *(_DWORD *)(a1 + 5812);
    v5 = v37;
    ++*(_DWORD *)(a1 + 20);
    *(_DWORD *)(a1 + 5812) = v24 - 12;
    *(_WORD *)(a1 + 5808) = (unsigned __int16)(a4 - 4) >> (16 - v24);
  }
  v25 = 0;
  if ( a4 > 0 )
  {
    do
    {
      v26 = *(_DWORD *)(a1 + 5812);
      v27 = (unsigned __int8)byte_5407AC[v25];
      if ( v26 <= 13 )
      {
        *(_WORD *)(a1 + 5808) |= *(_WORD *)(a1 + 4 * v27 + 2678) << v26;
        *(_DWORD *)(a1 + 5812) = v26 + 3;
      }
      else
      {
        v28 = *(_WORD *)(a1 + 4 * v27 + 2678);
        v29 = v28 << v26;
        v30 = *(_DWORD *)(a1 + 20);
        *(_WORD *)(a1 + 5808) |= v29;
        *(_BYTE *)(v30 + *(_DWORD *)(a1 + 8)) = *(_BYTE *)(a1 + 5808);
        v31 = *(_BYTE *)(a1 + 5809);
        v32 = *(_DWORD *)(a1 + 20) + 1;
        *(_DWORD *)(a1 + 20) = v32;
        *(_BYTE *)(v32 + *(_DWORD *)(a1 + 8)) = v31;
        v33 = *(_DWORD *)(a1 + 5812);
        ++*(_DWORD *)(a1 + 20);
        *(_DWORD *)(a1 + 5812) = v33 - 13;
        *(_WORD *)(a1 + 5808) = v28 >> (16 - v33);
      }
      ++v25;
    }
    while ( v25 < a4 );
    v5 = a3;
  }
  v34 = sub_4A7470(a1, a1 + 140, a2 - 1);
  return sub_4A7470(v34, v34 + 2432, v5 - 1);
}
#endif

/* ---- _tr_stored_block  0x004A7C60 ---- */
_DWORD *__cdecl _tr_stored_block(int a1, int a2, _BYTE *a3, int a4)
{
  int v5; // ecx
  int v6; // edx
  int v7; // ecx
  char v8; // bl
  int v9; // edx
  int v10; // ecx
  int v11; // edx

  v5 = *(_DWORD *)(a1 + 5812);
  if ( v5 <= 13 )
  {
    *(_DWORD *)(a1 + 5812) = v5 + 3;
    *(_WORD *)(a1 + 5808) |= a2 << v5;
  }
  else
  {
    v6 = a2 << v5;
    v7 = *(_DWORD *)(a1 + 8);
    *(_WORD *)(a1 + 5808) |= v6;
    *(_BYTE *)(v7 + *(_DWORD *)(a1 + 20)) = *(_BYTE *)(a1 + 5808);
    v8 = *(_BYTE *)(a1 + 5809);
    v9 = *(_DWORD *)(a1 + 8);
    v10 = *(_DWORD *)(a1 + 20) + 1;
    *(_DWORD *)(a1 + 20) = v10;
    *(_BYTE *)(v10 + v9) = v8;
    v11 = *(_DWORD *)(a1 + 5812);
    ++*(_DWORD *)(a1 + 20);
    *(_DWORD *)(a1 + 5812) = v11 - 13;
    *(_WORD *)(a1 + 5808) = (unsigned __int16)a2 >> (16 - v11);
  }
  return sub_4A8730(a1, a3, a4, 1);
}

/* ---- _tr_align  0x004A7D00 ---- */
int __cdecl _tr_align(int a1)
{
  int v1; // ecx
  char v2; // bl
  int v3; // edx
  int v4; // ecx
  int v5; // edx
  int v6; // ecx
  char v7; // bl
  int v8; // edx
  int v9; // ecx
  int v10; // edx
  int result; // eax
  int v12; // ecx
  char v13; // bl
  int v14; // edx
  int v15; // ecx
  int v16; // edx
  int v17; // ecx
  char v18; // bl
  int v19; // edx
  int v20; // ecx
  int v21; // edx

  v1 = *(_DWORD *)(a1 + 5812);
  *(_WORD *)(a1 + 5808) |= 2 << v1;
  if ( v1 <= 13 )
  {
    *(_DWORD *)(a1 + 5812) = v1 + 3;
  }
  else
  {
    *(_BYTE *)(*(_DWORD *)(a1 + 8) + *(_DWORD *)(a1 + 20)) = *(_BYTE *)(a1 + 5808);
    v2 = *(_BYTE *)(a1 + 5809);
    v3 = *(_DWORD *)(a1 + 8);
    v4 = *(_DWORD *)(a1 + 20) + 1;
    *(_DWORD *)(a1 + 20) = v4;
    *(_BYTE *)(v4 + v3) = v2;
    v5 = *(_DWORD *)(a1 + 5812);
    ++*(_DWORD *)(a1 + 20);
    *(_WORD *)(a1 + 5808) = 2u >> (16 - v5);
    *(_DWORD *)(a1 + 5812) = v5 - 13;
  }
  v6 = *(_DWORD *)(a1 + 5812);
  *(_WORD *)(a1 + 5808) = *(_WORD *)(a1 + 5808);
  if ( v6 <= 9 )
  {
    *(_DWORD *)(a1 + 5812) = v6 + 7;
  }
  else
  {
    *(_BYTE *)(*(_DWORD *)(a1 + 8) + *(_DWORD *)(a1 + 20)) = *(_BYTE *)(a1 + 5808);
    v7 = *(_BYTE *)(a1 + 5809);
    v8 = *(_DWORD *)(a1 + 8);
    v9 = *(_DWORD *)(a1 + 20) + 1;
    *(_DWORD *)(a1 + 20) = v9;
    *(_BYTE *)(v9 + v8) = v7;
    v10 = *(_DWORD *)(a1 + 5812);
    ++*(_DWORD *)(a1 + 20);
    *(_WORD *)(a1 + 5808) = 0;
    *(_DWORD *)(a1 + 5812) = v10 - 9;
  }
  result = sub_4A8640(a1);
  v12 = *(_DWORD *)(result + 5812);
  if ( *(_DWORD *)(result + 5804) - v12 + 11 < 9 )
  {
    *(_WORD *)(result + 5808) |= 2 << v12;
    if ( v12 <= 13 )
    {
      *(_DWORD *)(result + 5812) = v12 + 3;
    }
    else
    {
      *(_BYTE *)(*(_DWORD *)(result + 8) + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
      v13 = *(_BYTE *)(result + 5809);
      v14 = *(_DWORD *)(result + 8);
      v15 = *(_DWORD *)(result + 20) + 1;
      *(_DWORD *)(result + 20) = v15;
      *(_BYTE *)(v15 + v14) = v13;
      v16 = *(_DWORD *)(result + 5812);
      ++*(_DWORD *)(result + 20);
      *(_WORD *)(result + 5808) = 2u >> (16 - v16);
      *(_DWORD *)(result + 5812) = v16 - 13;
    }
    v17 = *(_DWORD *)(result + 5812);
    *(_WORD *)(result + 5808) = *(_WORD *)(result + 5808);
    if ( v17 > 9 )
    {
      *(_BYTE *)(*(_DWORD *)(result + 8) + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
      v18 = *(_BYTE *)(result + 5809);
      v19 = *(_DWORD *)(result + 8);
      v20 = *(_DWORD *)(result + 20) + 1;
      *(_DWORD *)(result + 20) = v20;
      *(_BYTE *)(v20 + v19) = v18;
      v21 = *(_DWORD *)(result + 5812) - 9;
      ++*(_DWORD *)(result + 20);
      *(_WORD *)(result + 5808) = 0;
      *(_DWORD *)(result + 5812) = v21;
      result = sub_4A8640(result);
      *(_DWORD *)(result + 5804) = 7;
      return result;
    }
    *(_DWORD *)(result + 5812) = v17 + 7;
    result = sub_4A8640(result);
  }
  *(_DWORD *)(result + 5804) = 7;
  return result;
}

/* ---- _tr_flush_block  0x004A7EF0 ---- */
_WORD *__cdecl _tr_flush_block(int a1, int a2, int a3, _BYTE *a4)
{
  int v6; // ebp
  int v7; // eax
  unsigned int v8; // edx
  unsigned int v9; // ecx
  int v10; // ecx
  bool v11; // zf
  int v12; // ecx
  int v13; // eax
  int v14; // edx
  int v15; // ecx
  char v16; // bl
  int v17; // edx
  int v18; // edx
  int v19; // edx
  int v20; // eax
  int v21; // ecx
  int v22; // eax
  int v23; // ebx
  int v24; // ebx
  _WORD *result; // eax

  v6 = 0;
  if ( *(int *)(a2 + 124) <= 0 )
  {
    v9 = a1 + 5;
LABEL_7:
    v8 = v9;
    goto LABEL_8;
  }
  if ( *(_BYTE *)(a2 + 28) == 2 )
    sub_4A8590(a2);
  sub_4A7190((_DWORD *)a2, (int *)(a2 + 2832));
  sub_4A7190((_DWORD *)a2, (int *)(a2 + 2844));
  v7 = sub_4A7980(a2);
  v8 = (unsigned int)(*(_DWORD *)(a2 + 5792) + 10) >> 3;
  v9 = (unsigned int)(*(_DWORD *)(a2 + 5796) + 10) >> 3;
  v6 = v7;
  if ( v9 <= v8 )
    goto LABEL_7;
LABEL_8:
  if ( a1 + 4 <= v8 && a4 )
  {
    _tr_stored_block(a2, a3, a4, a1);
  }
  else
  {
    v11 = v9 == v8;
    v12 = *(_DWORD *)(a2 + 5812);
    if ( v11 )
    {
      v13 = a3 + 2;
      if ( v12 <= 13 )
      {
        *(_WORD *)(a2 + 5808) |= v13 << v12;
        *(_DWORD *)(a2 + 5812) = v12 + 3;
      }
      else
      {
        v14 = v13 << v12;
        v15 = *(_DWORD *)(a2 + 8);
        *(_WORD *)(a2 + 5808) |= v14;
        *(_BYTE *)(v15 + *(_DWORD *)(a2 + 20)) = *(_BYTE *)(a2 + 5808);
        v16 = *(_BYTE *)(a2 + 5809);
        v17 = *(_DWORD *)(a2 + 20) + 1;
        *(_DWORD *)(a2 + 20) = v17;
        *(_BYTE *)(v17 + *(_DWORD *)(a2 + 8)) = v16;
        v18 = *(_DWORD *)(a2 + 5812);
        ++*(_DWORD *)(a2 + 20);
        *(_DWORD *)(a2 + 5812) = v18 - 13;
        *(_WORD *)(a2 + 5808) = (unsigned __int16)v13 >> (16 - v18);
      }
      sub_4A8170(a2, (int)&unk_5407C0, (int)&unk_540C40);
    }
    else
    {
      v19 = a3 + 4;
      if ( v12 <= 13 )
      {
        *(_WORD *)(a2 + 5808) |= v19 << v12;
        *(_DWORD *)(a2 + 5812) = v12 + 3;
      }
      else
      {
        v20 = v19 << v12;
        v21 = *(_DWORD *)(a2 + 8);
        *(_WORD *)(a2 + 5808) |= v20;
        *(_BYTE *)(v21 + *(_DWORD *)(a2 + 20)) = *(_BYTE *)(a2 + 5808);
        v22 = *(_DWORD *)(a2 + 8);
        v23 = *(_DWORD *)(a2 + 20) + 1;
        *(_DWORD *)(a2 + 20) = v23;
        *(_BYTE *)(v23 + v22) = *(_BYTE *)(a2 + 5809);
        v24 = *(_DWORD *)(a2 + 5812);
        ++*(_DWORD *)(a2 + 20);
        *(_DWORD *)(a2 + 5812) = v24 - 13;
        *(_WORD *)(a2 + 5808) = (unsigned __int16)v19 >> (16 - v24);
      }
      sub_4A79F0(a2, *(_DWORD *)(a2 + 2836) + 1, *(_DWORD *)(a2 + 2848) + 1, v6 + 1);
      sub_4A8170(a2, a2 + 140, a2 + 2432);
    }
  }
  result = sub_4A6DB0(v10, a2);
  if ( a3 )
    return (_WORD *)sub_4A86C0(a2);
  return result;
}

/* ---- _tr_tally  AUTO-STUBBED ----
 * The body did not compile.  Everything recovered for it is kept in the
 * #if 0 block below -- that is the starting point for reconstructing it
 * by hand.  Until then the signature is real but the body does nothing.
 */
int _tr_tally() { return 0; }
#if 0
BOOL __cdecl _tr_tally(_DWORD *a1, int a2, int a3)
{
  unsigned int v3; // ecx
  int v4; // ecx

  *(_WORD *)(a1[1447] + 2 * a1[1446]) = a3;
  *(_BYTE *)(a1[1444] + a1[1446]++) = a2;
  if ( a3 )
  {
    ++a1[1450];
    v3 = a3 - 1;
    ++LOWORD(a1[(unsigned __int8)byte_540EB8[a2] + 292]);
    if ( v3 >= 0x100 )
      v4 = (unsigned __int8)byte_540DB8[v3 >> 7];
    else
      v4 = (unsigned __int8)byte_540CB8[v3];
    ++LOWORD(a1[v4 + 608]);
  }
  else
  {
    ++LOWORD(a1[a2 + 35]);
  }
  return a1[1446] == a1[1445] - 1;
}
#endif

/* ---- sub_4A8170  AUTO-STUBBED ----
 * The body did not compile.  Everything recovered for it is kept in the
 * #if 0 block below -- that is the starting point for reconstructing it
 * by hand.  Until then the signature is real but the body does nothing.
 */
int sub_4A8170() { return 0; }
#if 0
int __cdecl sub_4A8170(int result, int a2, int a3)
{
  int v3; // ebx
  unsigned int v4; // ecx
  int v5; // ebp
  int v6; // esi
  int v7; // ecx
  int v8; // edi
  int v9; // esi
  int v10; // edx
  int v11; // ecx
  char v12; // bl
  int v13; // edx
  int v14; // edx
  int v15; // ecx
  int v16; // edx
  int v17; // edi
  int v18; // edx
  int v19; // ecx
  char v20; // bl
  int v21; // edx
  int v22; // edx
  int v23; // edi
  int v24; // esi
  int v25; // ecx
  int v26; // edx
  int v27; // ecx
  char v28; // bl
  int v29; // edx
  int v30; // edx
  int v31; // ecx
  unsigned int v32; // ebp
  int v33; // edi
  int v34; // edx
  int v35; // ecx
  unsigned __int16 v36; // si
  int v37; // edx
  int v38; // ecx
  char v39; // bl
  int v40; // edx
  int v41; // edx
  int v42; // esi
  unsigned int v43; // ebp
  int v44; // ecx
  unsigned int v45; // edx
  int v46; // ecx
  char v47; // bl
  int v48; // edx
  int v49; // edx
  int v50; // edi
  int v51; // ecx
  unsigned __int16 v52; // si
  int v53; // edx
  int v54; // ecx
  int v55; // ecx
  int v56; // edx
  int v57; // edx
  int v58; // [esp+10h] [ebp-Ch]
  int v59; // [esp+10h] [ebp-Ch]
  int v60; // [esp+14h] [ebp-8h]
  int v61; // [esp+14h] [ebp-8h]
  int v62; // [esp+14h] [ebp-8h]
  unsigned int v63; // [esp+18h] [ebp-4h]

  v3 = a2;
  v4 = 0;
  if ( *(_DWORD *)(result + 5784) )
  {
    do
    {
      v5 = *(unsigned __int16 *)(*(_DWORD *)(result + 5788) + 2 * v4);
      v6 = *(unsigned __int8 *)(v4 + *(_DWORD *)(result + 5776));
      v63 = v4 + 1;
      v7 = *(_DWORD *)(result + 5812);
      if ( v5 )
      {
        v16 = (unsigned __int8)byte_540EB8[v6];
        v59 = *(unsigned __int16 *)(v3 + 4 * v16 + 1030);
        v60 = v16;
        if ( v7 <= 16 - v59 )
        {
          *(_WORD *)(result + 5808) |= *(_WORD *)(a2 + 4 * v16 + 1028) << v7;
          *(_DWORD *)(result + 5812) = v59 + v7;
        }
        else
        {
          v17 = *(unsigned __int16 *)(a2 + 4 * v16 + 1028);
          v18 = v17 << v7;
          v19 = *(_DWORD *)(result + 8);
          *(_WORD *)(result + 5808) |= v18;
          *(_BYTE *)(v19 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
          v20 = *(_BYTE *)(result + 5809);
          v21 = *(_DWORD *)(result + 20) + 1;
          *(_DWORD *)(result + 20) = v21;
          *(_BYTE *)(v21 + *(_DWORD *)(result + 8)) = v20;
          v22 = *(_DWORD *)(result + 5812);
          ++*(_DWORD *)(result + 20);
          LOWORD(v17) = (unsigned __int16)v17 >> (16 - v22);
          *(_DWORD *)(result + 5812) = v22 + v59 - 16;
          v16 = v60;
          *(_WORD *)(result + 5808) = v17;
        }
        v23 = dword_540670[v16];
        v3 = a2;
        if ( v23 )
        {
          v24 = v6 - dword_540FB8[v16];
          v25 = *(_DWORD *)(result + 5812);
          if ( v25 <= 16 - v23 )
          {
            *(_WORD *)(result + 5808) |= v24 << v25;
            v31 = v23 + v25;
          }
          else
          {
            v26 = v24 << v25;
            v27 = *(_DWORD *)(result + 8);
            *(_WORD *)(result + 5808) |= v26;
            *(_BYTE *)(v27 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
            v28 = *(_BYTE *)(result + 5809);
            v29 = *(_DWORD *)(result + 20) + 1;
            *(_DWORD *)(result + 20) = v29;
            *(_BYTE *)(v29 + *(_DWORD *)(result + 8)) = v28;
            v30 = *(_DWORD *)(result + 5812);
            v3 = a2;
            ++*(_DWORD *)(result + 20);
            v31 = v30 + v23 - 16;
            *(_WORD *)(result + 5808) = (unsigned __int16)v24 >> (16 - v30);
          }
          *(_DWORD *)(result + 5812) = v31;
        }
        v32 = v5 - 1;
        if ( v32 >= 0x100 )
          v33 = (unsigned __int8)byte_540DB8[v32 >> 7];
        else
          v33 = (unsigned __int8)byte_540CB8[v32];
        v34 = *(unsigned __int16 *)(a3 + 4 * v33 + 2);
        v35 = *(_DWORD *)(result + 5812);
        v61 = v34;
        if ( v35 <= 16 - v34 )
        {
          *(_WORD *)(result + 5808) |= *(_WORD *)(a3 + 4 * v33) << v35;
          *(_DWORD *)(result + 5812) = v34 + v35;
        }
        else
        {
          v36 = *(_WORD *)(a3 + 4 * v33);
          v37 = v36 << v35;
          v38 = *(_DWORD *)(result + 8);
          *(_WORD *)(result + 5808) |= v37;
          *(_BYTE *)(v38 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
          v39 = *(_BYTE *)(result + 5809);
          v40 = *(_DWORD *)(result + 20) + 1;
          *(_DWORD *)(result + 20) = v40;
          *(_BYTE *)(v40 + *(_DWORD *)(result + 8)) = v39;
          v41 = *(_DWORD *)(result + 5812);
          v3 = a2;
          ++*(_DWORD *)(result + 20);
          *(_DWORD *)(result + 5812) = v41 + v61 - 16;
          *(_WORD *)(result + 5808) = v36 >> (16 - v41);
        }
        v42 = dword_5406E8[v33];
        if ( !v42 )
          goto LABEL_25;
        v43 = v32 - dword_541030[v33];
        v44 = *(_DWORD *)(result + 5812);
        if ( v44 <= 16 - v42 )
        {
          *(_WORD *)(result + 5808) |= v43 << v44;
          v15 = v42 + v44;
        }
        else
        {
          v45 = v43 << v44;
          v46 = *(_DWORD *)(result + 8);
          v62 = v3;
          *(_WORD *)(result + 5808) |= v45;
          *(_BYTE *)(v46 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
          v47 = *(_BYTE *)(result + 5809);
          v48 = *(_DWORD *)(result + 20) + 1;
          *(_DWORD *)(result + 20) = v48;
          *(_BYTE *)(v48 + *(_DWORD *)(result + 8)) = v47;
          v49 = *(_DWORD *)(result + 5812);
          v3 = v62;
          ++*(_DWORD *)(result + 20);
          v15 = v49 + v42 - 16;
          *(_WORD *)(result + 5808) = (unsigned __int16)v43 >> (16 - v49);
        }
      }
      else
      {
        v8 = *(unsigned __int16 *)(v3 + 4 * v6 + 2);
        if ( v7 <= 16 - v8 )
        {
          *(_WORD *)(result + 5808) |= *(_WORD *)(v3 + 4 * v6) << v7;
          v15 = v8 + v7;
        }
        else
        {
          v9 = *(unsigned __int16 *)(v3 + 4 * v6);
          v10 = v9 << v7;
          v11 = *(_DWORD *)(result + 8);
          v58 = v3;
          *(_WORD *)(result + 5808) |= v10;
          *(_BYTE *)(v11 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
          v12 = *(_BYTE *)(result + 5809);
          v13 = *(_DWORD *)(result + 20) + 1;
          *(_DWORD *)(result + 20) = v13;
          *(_BYTE *)(v13 + *(_DWORD *)(result + 8)) = v12;
          v14 = *(_DWORD *)(result + 5812);
          v3 = v58;
          ++*(_DWORD *)(result + 20);
          v15 = v14 + v8 - 16;
          *(_WORD *)(result + 5808) = (unsigned __int16)v9 >> (16 - v14);
        }
      }
      *(_DWORD *)(result + 5812) = v15;
LABEL_25:
      v4 = v63;
    }
    while ( v63 < *(_DWORD *)(result + 5784) );
  }
  v50 = *(unsigned __int16 *)(v3 + 1026);
  v51 = *(_DWORD *)(result + 5812);
  if ( v51 <= 16 - v50 )
  {
    *(_WORD *)(result + 5808) |= *(_WORD *)(v3 + 1024) << v51;
    *(_DWORD *)(result + 5812) = v50 + v51;
  }
  else
  {
    v52 = *(_WORD *)(v3 + 1024);
    v53 = v52 << v51;
    v54 = *(_DWORD *)(result + 8);
    *(_WORD *)(result + 5808) |= v53;
    *(_BYTE *)(v54 + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
    v55 = *(_DWORD *)(result + 8);
    v56 = *(_DWORD *)(result + 20) + 1;
    *(_DWORD *)(result + 20) = v56;
    *(_BYTE *)(v55 + v56) = *(_BYTE *)(result + 5809);
    v57 = *(_DWORD *)(result + 5812);
    ++*(_DWORD *)(result + 20);
    *(_DWORD *)(result + 5812) = v57 + v50 - 16;
    *(_WORD *)(result + 5808) = v52 >> (16 - v57);
  }
  *(_DWORD *)(result + 5804) = *(unsigned __int16 *)(v3 + 1026);
  return result;
}
#endif

/* ---- sub_4A8590  0x004A8590 ---- */
bool __cdecl sub_4A8590(int this)
{
  unsigned int v1; // esi
  unsigned int v2; // eax
  unsigned __int16 *v3; // edx
  int v4; // edi
  unsigned __int16 *v5; // edx
  int v6; // edi
  bool result; // al

  v1 = 0;
  v2 = *(unsigned __int16 *)(this + 140)
     + *(unsigned __int16 *)(this + 144)
     + *(unsigned __int16 *)(this + 148)
     + *(unsigned __int16 *)(this + 152)
     + *(unsigned __int16 *)(this + 156)
     + *(unsigned __int16 *)(this + 160)
     + *(unsigned __int16 *)(this + 164);
  v3 = (unsigned __int16 *)(this + 168);
  v4 = 121;
  do
  {
    v1 += *v3;
    v3 += 2;
    --v4;
  }
  while ( v4 );
  v5 = (unsigned __int16 *)(this + 652);
  v6 = 128;
  do
  {
    v2 += *v5;
    v5 += 2;
    --v6;
  }
  while ( v6 );
  result = v1 >> 2 >= v2;
  *(_BYTE *)(this + 28) = result;
  return result;
}

/* ---- sub_4A8620  0x004A8620 ---- */
unsigned int __fastcall sub_4A8620(unsigned int a1, int a2)
{
  unsigned int v2; // eax
  unsigned int v3; // esi

  v2 = 0;
  do
  {
    v3 = a1 & 1;
    a1 >>= 1;
    v2 = 2 * (v3 | v2);
    --a2;
  }
  while ( a2 > 0 );
  return v2 >> 1;
}

/* ---- sub_4A8640  0x004A8640 ---- */
int __cdecl sub_4A8640(int result)
{
  int v1; // ecx
  char v2; // bl
  int v3; // edx
  int v4; // edx
  int v5; // ecx

  v1 = *(_DWORD *)(result + 5812);
  if ( v1 == 16 )
  {
    *(_BYTE *)(*(_DWORD *)(result + 8) + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
    v2 = *(_BYTE *)(result + 5809);
    v3 = *(_DWORD *)(result + 20) + 1;
    *(_DWORD *)(result + 20) = v3;
    *(_BYTE *)(v3 + *(_DWORD *)(result + 8)) = v2;
    ++*(_DWORD *)(result + 20);
    *(_WORD *)(result + 5808) = 0;
    *(_DWORD *)(result + 5812) = 0;
  }
  else if ( v1 >= 8 )
  {
    *(_BYTE *)(*(_DWORD *)(result + 8) + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
    v4 = *(_DWORD *)(result + 20);
    *(_WORD *)(result + 5808) = *(unsigned __int8 *)(result + 5809);
    v5 = *(_DWORD *)(result + 5812);
    *(_DWORD *)(result + 20) = v4 + 1;
    *(_DWORD *)(result + 5812) = v5 - 8;
  }
  return result;
}

/* ---- sub_4A86C0  0x004A86C0 ---- */
int __cdecl sub_4A86C0(int result)
{
  int v1; // edx
  char v2; // bl
  int v3; // edx

  v1 = *(_DWORD *)(result + 5812);
  if ( v1 <= 8 )
  {
    if ( v1 > 0 )
      *(_BYTE *)(*(_DWORD *)(result + 8) + (*(_DWORD *)(result + 20))++) = *(_BYTE *)(result + 5808);
    *(_WORD *)(result + 5808) = 0;
    *(_DWORD *)(result + 5812) = 0;
  }
  else
  {
    *(_BYTE *)(*(_DWORD *)(result + 8) + *(_DWORD *)(result + 20)) = *(_BYTE *)(result + 5808);
    v2 = *(_BYTE *)(result + 5809);
    v3 = *(_DWORD *)(result + 20) + 1;
    *(_DWORD *)(result + 20) = v3;
    *(_BYTE *)(v3 + *(_DWORD *)(result + 8)) = v2;
    ++*(_DWORD *)(result + 20);
    *(_WORD *)(result + 5808) = 0;
    *(_DWORD *)(result + 5812) = 0;
  }
  return result;
}

/* ---- sub_4A8730  AUTO-STUBBED ----
 * The body did not compile.  Everything recovered for it is kept in the
 * #if 0 block below -- that is the starting point for reconstructing it
 * by hand.  Until then the signature is real but the body does nothing.
 */
int sub_4A8730() { return 0; }
#if 0
_DWORD *__cdecl sub_4A8730(int a1, _BYTE *a2, int a3, int a4)
{
  _DWORD *result; // eax
  int v7; // edx
  int v8; // ecx
  int v9; // edx
  int v10; // esi
  int v11; // edx
  int v12; // esi
  int v13; // [esp+10h] [ebp+4h]

  result = (_DWORD *)sub_4A86C0(a1);
  result[1451] = 8;
  if ( a4 )
  {
    *(_BYTE *)(result[5] + result[2]) = a3;
    v7 = result[2];
    v8 = result[5] + 1;
    result[5] = v8;
    *(_BYTE *)(v8 + v7) = BYTE1(a3);
    v9 = result[2];
    v10 = result[5] + 1;
    result[5] = v10;
    *(_BYTE *)(v10 + v9) = ~(_BYTE)a3;
    v11 = result[2];
    v12 = result[5] + 1;
    result[5] = v12;
    *(_BYTE *)(v12 + v11) = (unsigned __int16)~(_WORD)a3 >> 8;
    ++result[5];
  }
  if ( a3 )
  {
    do
    {
      v13 = a3;
      *(_BYTE *)(result[5] + result[2]) = *a2++;
      --a3;
      ++result[5];
    }
    while ( v13 != 1 );
  }
  return result;
}
#endif
