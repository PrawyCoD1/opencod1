/*
 * zlib_rdata.c -- zlib 1.1.4 data objects that cod1_globals.c gets wrong.
 *
 * @fidelity: verified   (storage and initialisers only -- no code here)
 *
 * This unit deliberately does NOT include ../qcommon/cod1_globals.h.  The
 * definitions below and that header's declarations of the same names have
 * different types and must never meet inside one translation unit; the linker
 * matches them by NAME alone, which is the whole mechanism.  Same arrangement,
 * and same reason, as EffectsCore/fx_rdata.c and client_mp/cl_refstorage.c:
 * cod1_globals.c's objects are C tentative definitions (COMMON), and an
 * INITIALISED definition anywhere in the link is a strong definition that the
 * COMMON binds to.  Nothing else has to change.
 *
 * zlibVersion (0x004A9950) pins the bundled library at 1.1.4, so every layout
 * below is read off the public 1.1.4 sources -- zutil.c and trees.c -- and then
 * checked against the image.  No middleware internals are decompiled here.
 *
 * ==========================================================================
 * static_l_desc / static_d_desc / static_bl_desc  @ 0x00571BC8, BDC, BF0
 * ==========================================================================
 *
 * trees.c:
 *
 *     typedef struct static_tree_desc_s {
 *         const ct_data *static_tree;
 *         const intf    *extra_bits;
 *         int            extra_base;
 *         int            elems;
 *         int            max_length;
 *     } static_tree_desc;
 *
 * -- five fields, 20 bytes, and retail has three of them back to back:
 *
 *     0x00571BC8  005407C0 00540670 00000101 0000011E 0000000F   static_l_desc
 *     0x00571BDC  00540C40 005406E8 00000000 0000001E 0000000F   static_d_desc
 *     0x00571BF0  00000000 00540760 00000000 00000013 00000007   static_bl_desc
 *
 * which is trees.c verbatim: {static_ltree, extra_lbits, LITERALS+1 (257),
 * L_CODES (286), MAX_BITS (15)}, {static_dtree, extra_dbits, 0, D_CODES (30),
 * MAX_BITS}, {NULL, extra_blbits, 0, BL_CODES (19), MAX_BL_BITS (7)}.  The
 * three run 0x00571BC8..0x00571C03 and inflate_mask follows at 0x00571C08.
 *
 * cod1_globals.h had `extern int off_571BC8;` and `extern int off_571BDC;` --
 * FOUR BYTES each, zero -- for 20-byte initialised structs.  _tr_init
 * (_tr_init, zlib/inflate.c) stores their addresses into the deflate state
 * at +2840 and +2852, and every subsequent read of .extra_bits, .extra_base,
 * .elems and .max_length then lands on whatever cod1_globals.c placed next.
 * unk_571BF0 was reserved at the right SIZE (24 bytes) but zero-filled, so its
 * three constant fields were wrong too.
 *
 * DO NOT TRUST THE SIZE COMMENTS: the header said 64 and 44 bytes.  Both are
 * distance to the end of the region (0x00571C08), not the stride, which is 20.
 *
 * STILL OUTSTANDING, and NOT this unit's fault family -- the tables these
 * descriptors point at are declared at adequate size but are zero-filled,
 * because cod1_globals.c reserves them without initialisers.  Retail holds
 * trees.h's precomputed data, all of it in .rdata:
 *
 *     0x00540670  extra_lbits[29]     116 bytes   (dword_540670, decl [654])
 *     0x005406E8  extra_dbits[30]     120 bytes   (dword_5406E8, decl [624])
 *     0x00540760  extra_blbits[19]     76 bytes   (unnamed)
 *     0x005407AC  bl_order[19]         19 bytes   (byte_5407AC)
 *     0x005407C0  static_ltree[288]  1152 bytes   (unk_5407C0,  decl [2280])
 *     0x00540C40  static_dtree[30]    120 bytes   (unk_540C40,  decl [1128])
 *     0x00540CB8  _dist_code[512]     512 bytes   (byte_540CB8)
 *     0x00540EB8  _length_code[256]   256 bytes
 *     0x00540FB8  base_length[29]     116 bytes   (dword_540FB8)
 *     0x00541030  base_dist[30]       120 bytes   (dword_541030)
 *
 * ending exactly at the "inflate 1.1.4 Copyright" literal (0x005410A8), which
 * is what pins the chain.  That is a ZERO-CONTENT defect, not a wrong-size one,
 * and it only bites deflate -- `compress` (0x004A2CC0) has no xrefs in retail,
 * so nothing in the game ever reaches it.  Left for a separate pass rather than
 * mixed into this one.
 *
 * The extra_bits pointers below therefore aim at the existing (still zeroed)
 * symbols, exactly as retail does, so that fixing those tables needs no change
 * here.  extra_blbits has no symbol in our tree because retail's 0x00540760 has
 * no xref for IDA to name; it is `local` in trees.c, so it is file-static here,
 * with the contents the image holds.
 *
 * ==========================================================================
 * z_errmsg  @ 0x00571BA0
 * ==========================================================================
 *
 * zutil.c's `const char *z_errmsg[10]`, 40 bytes, ending exactly where
 * static_l_desc begins:
 *
 *     0x00571BA0 -> 0x00559218 "need dictionary"       Z_NEED_DICT      2
 *     0x00571BA4 -> 0x0055920C "stream end"            Z_STREAM_END     1
 *     0x00571BA8 -> 0x00559228 ""                      Z_OK             0
 *     0x00571BAC -> 0x00559200 "file error"            Z_ERRNO         -1
 *     0x00571BB0 -> 0x005591F0 "stream error"          Z_STREAM_ERROR  -2
 *     0x00571BB4 -> 0x005591E4 "data error"            Z_DATA_ERROR    -3
 *     0x00571BB8 -> 0x005591D0 "insufficient memory"   Z_MEM_ERROR     -4
 *     0x00571BBC -> 0x005591C0 "buffer error"          Z_BUF_ERROR     -5
 *     0x00571BC0 -> 0x005591A8 "incompatible version"  Z_VERSION_ERROR -6
 *     0x00571BC4 -> 0x00559228 ""
 *
 * THE NAME IS NOT AT THE BASE.  IDA's `z_errmsg` label sits at 0x00571BA8,
 * which is &z_errmsg[2]: ERR_MSG(err) is z_errmsg[Z_NEED_DICT - err] and the
 * compiler folded the constant 2 into the base address, so the only xref in the
 * image points eight bytes in.  cod1_globals.h inherited that address AND the
 * four-byte int, which is why zError indexed backwards off a scalar.  Defining
 * the object here under the name the source uses puts the base back where zlib
 * expects it, and zlib/unzip.c's zError is spelled as zutil.c spells it.
 * off_571BA0, off_571BB0, off_571BB8 and off_571BBC are IDA labels on interior
 * elements of this same array, not separate objects; nothing references them.
 */

typedef struct ct_data_s {
	unsigned short fc;      /* freq / code  */
	unsigned short dl;      /* dad  / len   */
} ct_data;

typedef struct static_tree_desc_s {
	const ct_data *static_tree;
	const int     *extra_bits;
	int            extra_base;
	int            elems;
	int            max_length;
} static_tree_desc;

/* The generated names for static_ltree, static_dtree, extra_lbits and
 * extra_dbits.  Restated locally, at the types cod1_globals.h gives them,
 * because this unit cannot include that header. */
extern unsigned char unk_5407C0[];      /* 0x005407C0  static_ltree  */
extern unsigned char unk_540C40[];      /* 0x00540C40  static_dtree  */
extern int           dword_540670[];    /* 0x00540670  extra_lbits   */
extern int           dword_5406E8[];    /* 0x005406E8  extra_dbits   */

/* 0x00540760 -- unnamed in the IDB, `local` in trees.c. */
static const int extra_blbits[19] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 7 };

/* 0x00571BC8  static_l_desc */
static_tree_desc off_571BC8 = {
	(const ct_data *)unk_5407C0, dword_540670, 257, 286, 15
};

/* 0x00571BDC  static_d_desc */
static_tree_desc off_571BDC = {
	(const ct_data *)unk_540C40, dword_5406E8, 0, 30, 15
};

/* 0x00571BF0  static_bl_desc */
static_tree_desc unk_571BF0 = {
	(const ct_data *)0, extra_blbits, 0, 19, 7
};

/* 0x00571BA0  z_errmsg[10] */
const char *z_errmsg[10] = {
	"need dictionary",
	"stream end",
	"",
	"file error",
	"stream error",
	"data error",
	"insufficient memory",
	"buffer error",
	"incompatible version",
	""
};
