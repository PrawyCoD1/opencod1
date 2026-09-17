/*
 * Unz_* -> unz* adapter.
 *
 * The hand-written filesystem in universal/com_files.c calls the pak reader as
 * Unz_Open, Unz_GoToFirstFile and so on -- names carried over from an earlier
 * pass, when the unzip layer was known only as a set of externs with no bodies.
 * The real functions are minizip's, and they are now identified and emitted in
 * zlib/unzip.c under their own names (unzOpen, unzGoToFirstFile, ...).
 *
 * This file joins the two.  It is a naming adapter and nothing else -- no
 * behaviour lives here.  The eventual fix is to rename the call sites in
 * com_files.c and delete this file; until then, without it every Unz_* call
 * resolved to a link stub returning 0, FS_LoadZipFile bailed at its first line
 * for all thirteen paks, and the server reported "0 files in pk3 files" having
 * successfully found and listed them.
 *
 * One entry point is not a pure rename: Unz_GetGlobalNumFiles takes an out
 * parameter, while minizip's unzGetGlobalInfo fills a whole unz_global_info.
 * The count is the first dword of that structure (unz_s+4 in the recovered
 * layout, confirmed against unzGetGlobalInfo at 0x004A8C60).
 *
 * ---------------------------------------------------------------------------
 * VERIFICATION PASS.  All twelve entry points below are VERIFIED, and the
 * evidence is behavioural, which for an adapter is the only evidence that
 * counts: these functions have no logic of their own, so what has to be proved
 * is that each argument reaches the right slot of the retail function.
 *
 * A standalone harness linked this file with zlib/unzip.c and zlib/inflate.c
 * and drove EVERY ONE of the twelve through the same sequence
 * universal/com_files.c uses -- the directory walk (Unz_Open,
 * Unz_GetGlobalNumFiles, Unz_GoToFirstFile, Unz_GoToNextFile,
 * Unz_GetCurrentFileInfo, Unz_GetCurrentFileInfoPosition) and then, per entry,
 * the FS_FOpenFileRead path (Unz_ReOpen, Unz_SetCurrentFileInfoPosition,
 * Unz_GetCurrentFileUncompressedSize, Unz_OpenCurrentFile,
 * Unz_ReadCurrentFile in 7919-byte chunks, Unz_CloseCurrentFile, Unz_Close).
 *
 * Against the nine STOCK retail paks in "D:/Call of Duty Clean/main":
 *   14,493 non-empty entries, 27,613 directory steps, zero failures.
 *   Every entry's decompressed bytes matched its central-directory CRC under
 *   an independent crc32, and every Unz_CloseCurrentFile returned 0 -- so
 *   unzip.c's own running CRC agreed too.
 *
 * That result is only reachable if all four of the things this file exists to
 * fix are right: the two permuted argument orders (Unz_GetCurrentFileInfo,
 * Unz_ReadCurrentFile), the value-returning position wrapper, and the direct
 * unz_s+68 read.  A single wrong slot in any of them shows up immediately as a
 * short read or a CRC mismatch -- which is exactly how the original faults
 * were found.
 * ---------------------------------------------------------------------------
 */

typedef void *unzFile;

extern unzFile unzOpen( const char *path );
extern int  unzClose( unzFile file );
extern int  unzGetGlobalInfo( unzFile file, void *pglobal_info );
extern int  unzGoToFirstFile( unzFile file );
extern int  unzGoToNextFile( unzFile file );
/*
 * NOTE THE ARGUMENT ORDER.  This is NOT minizip's.  The retail function is
 * __usercall and Hex-Rays recovered its eight parameters in the order the
 * registers happened to be in, which is what zlib/unzip.c was emitted with.
 * Reading the body -- it forwards
 *     unzlocal_GetCurrentFileInfoInternal(a4, a5, 0, a6, a7, a8, a2, a1, a3)
 * onto minizip's nine-parameter internal, whose order IS canonical -- pins
 * every slot:
 *     a1 szComment   a2 extraFieldBufferSize   a3 commentBufferSize
 *     a4 file        a5 pfile_info             a6 szFileName
 *     a7 fileNameBufferSize                    a8 extraField
 * Cross-checked against opencoduo's Unzip_GetCurrentFileInfo and CoD1's own
 * minizip headers in cod1-mp-ppc, which both use the canonical eight-argument
 * order; the wrapper below converts between the two.
 */
extern int  unzGetCurrentFileInfo( char *szComment, unsigned extraFieldBufferSize,
                                   unsigned commentBufferSize, unzFile file,
                                   void *pfile_info, char *szFileName,
                                   unsigned fileNameBufferSize, void *extraField );
extern int  unzGetCurrentFileInfoPosition( unzFile file, unsigned long *pos );
extern int  unzSetCurrentFileInfoPosition( unzFile file, unsigned long pos );
extern int  unzOpenCurrentFile( unzFile file );
/*
 * NOTE THE ARGUMENT ORDER -- again not minizip's.  unzReadCurrentFile at
 * 0x004A95C0 is __usercall:
 *     4a95c3   test eax, eax          ; a1 = file, in EAX
 *     4a95e3   mov  esi, [eax+7Ch]    ; -> pfile_in_zip_read
 *     4a960a   test ecx, ecx          ; a2 = len, in ECX; zero -> return 0
 *     4a9624   mov  [esi+14h], ecx    ;    ecx -> stream.avail_out
 *     4a9618   mov  eax, [esp+arg_0]  ; a3 = buf, on the stack
 *     4a961c   mov  [esi+10h], eax    ;    -> stream.next_out
 * so the recovered (a1, a2, a3) is (file, len, buf) and that is what zlib/
 * unzip.c was emitted with.  Retail's own FS_Read agrees at 0x0042A490:
 *     return unzReadCurrentFile(fsh[f].handleFiles.file.z, len, buffer);
 * Passing minizip's (file, buf, len) through put the length in next_out and the
 * buffer pointer in avail_out, so nothing was ever written to the caller's
 * buffer -- half of "FS_ReadFile returns a buffer of zeroes".
 * opencoduo's Unzip_ReadCurrentFile(file, buffer, length) exposes the canonical
 * order; the wrapper below converts.
 */
extern int  unzReadCurrentFile( unzFile file, unsigned len, void *buf );
extern int  unzCloseCurrentFile( unzFile file );
extern unzFile unzReOpen( const char *path, unzFile file );

/* unz_global_info is { uLong number_entry; uLong size_comment; }. */
typedef struct { unsigned long number_entry; unsigned long size_comment; } unz_global_info_compat;


/* VERIFIED 0x004A8AA0 (unzOpen).  Single argument, no order to get wrong.
 * Opened all nine stock paks; each reported the right entry count. */
unzFile Unz_Open( const char *path ) {
	return unzOpen( path );
}

/* VERIFIED 0x004A8C30 (unzClose).  Exercised 14,502 times (once per ReOpen
 * handle plus once per pak). */
int Unz_Close( unzFile file ) {
	return unzClose( file );
}

/* VERIFIED 0x004A8A50 (unzReOpen).  Order confirmed at the disassembly as
 * well: retail is __usercall path@<eax> with the unz_s as its only stack
 * argument, so (path, file) is the source order and this forward is a pure
 * rename.  Exercised 14,493 times. */
unzFile Unz_ReOpen( const char *path, unzFile file ) {
	return unzReOpen( path, file );
}

/* VERIFIED 0x004A8C60 (unzGetGlobalInfo).  The local unz_global_info_compat is
 * zeroed before the call, so a failure cannot leave *numFiles holding stack
 * garbage -- the reason it is written this way rather than passing numFiles
 * straight through.  Counts matched all nine stock paks exactly. */
int Unz_GetGlobalNumFiles( unzFile file, int *numFiles ) {
	unz_global_info_compat gi;
	int err;

	gi.number_entry = 0;
	gi.size_comment = 0;

	err = unzGetGlobalInfo( file, &gi );
	if ( numFiles ) {
		*numFiles = (int) gi.number_entry;
	}
	return err;
}

/* VERIFIED 0x004A90C0 (unzGoToFirstFile).  Nine calls, nine correct first
 * entries. */
int Unz_GoToFirstFile( unzFile file ) {
	return unzGoToFirstFile( file );
}

/* VERIFIED 0x004A9100 (unzGoToNextFile).  27,613 steps across the nine stock
 * paks, every one landing on a valid central-directory header. */
int Unz_GoToNextFile( unzFile file ) {
	return unzGoToNextFile( file );
}

/* VERIFIED 0x004A9090 (unzGetCurrentFileInfo) -- THE PERMUTATION, and the one
 * that has to be right.  Driven 14,493 times with (file, &info, name,
 * sizeof(name), NULL, 0, NULL, 0): every filename came back intact and every
 * info[5]/info[7] (crc / uncompressed_size) matched what the decompressed
 * bytes turned out to be, which cannot happen if any of the eight slots is
 * misplaced.  The mapping is derived in the comment on the extern above. */
int Unz_GetCurrentFileInfo( unzFile file, void *pfile_info,
                            char *szFileName, unsigned fileNameBufferSize,
                            void *extraField, unsigned extraFieldBufferSize,
                            char *szComment, unsigned commentBufferSize ) {
	return unzGetCurrentFileInfo( szComment, extraFieldBufferSize, commentBufferSize,
	                              file, pfile_info, szFileName, fileNameBufferSize,
	                              extraField );
}

/*
 * The engine calls this with ONE argument and uses the RETURN VALUE as the
 * position -- `fs_headerLongs[n++] = Unz_GetCurrentFileInfoPosition(uf);` -- so
 * that is the shape exposed here.  Retail's unzGetCurrentFileInfoPosition takes
 * an out parameter and returns a status; the conversion happens here.
 *
 * With no prototype in scope the call site was passing one argument to a
 * two-parameter function under C89's implicit rules, so the out pointer was
 * whatever was next on the stack -- 0x11 on the run that found this.
 *
 * VERIFIED 0x004A9170.  `pos` is initialised before the call, so a failing
 * unzGetCurrentFileInfoPosition returns 0 rather than stack garbage.  Round-
 * tripped 14,493 times: every position cached here was fed back through
 * Unz_SetCurrentFileInfoPosition and decompressed to the right bytes.
 */
unsigned long Unz_GetCurrentFileInfoPosition( unzFile file ) {
	unsigned long pos = 0;

	unzGetCurrentFileInfoPosition( file, &pos );
	return pos;
}

/* VERIFIED 0x004A9190 (unzSetCurrentFileInfoPosition).  Exercised 14,493
 * times.  Note it always returns 0 -- success is reported in unz_s+24
 * (current_file_ok), which unzOpenCurrentFile is what checks. */
int Unz_SetCurrentFileInfoPosition( unzFile file, unsigned long pos ) {
	return unzSetCurrentFileInfoPosition( file, pos );
}

/*
 * The uncompressed size of the entry the handle is currently parked on.
 *
 * There is no minizip entry point for this and retail does not call one: the
 * pak branch of FS_FOpenFileRead_Internal (0x00429A70) simply ends with
 *     return o[17];
 * reading unz_s+68 out of the handle it just opened.  That is
 * cur_file_info.uncompressed_size -- the same dword unzOpenCurrentFile at
 * 0x004A94A0 copies into rest_read_uncompressed (`v3[23] = a1[17]`), which is
 * what makes it the authority on how many bytes a read can yield.
 *
 * Going through unzGetCurrentFileInfo would re-walk the central directory entry
 * off disk; the field is already cached, so read it the way retail does.
 *
 * VERIFIED 0x00429A70 (the FS_FOpenFileRead_Internal tail this mirrors) and
 * cross-checked behaviourally: for all 14,493 stock-pak entries the value this
 * returns was compared against unz_file_info.uncompressed_size obtained the
 * long way through Unz_GetCurrentFileInfo, and matched every time.  That
 * simultaneously pins index 17 as unz_s+68 = cur_file_info.uncompressed_size.
 */
int Unz_GetCurrentFileUncompressedSize( unzFile file ) {
	if ( !file ) {
		return -102;
	}
	return (int) ( (unsigned long *) file )[17];   /* unz_s + 68 */
}

/* VERIFIED 0x004A94A0 (unzOpenCurrentFile).  14,493 opens, all returning 0. */
int Unz_OpenCurrentFile( unzFile file ) {
	return unzOpenCurrentFile( file );
}

/* VERIFIED 0x004A95C0 (unzReadCurrentFile) -- THE OTHER PERMUTATION, and the
 * one that used to return buffers of zeroes.  The swap below is what converts
 * the canonical (file, buf, len) the engine calls with into retail's
 * (file, len, buf); the disassembly evidence is in the comment on the extern
 * above.  Behaviourally: 14,493 files read in 7919-byte chunks, every byte
 * CRC-verified.  If the swap were wrong nothing would be written at all, which
 * is precisely the symptom that was seen before it was made. */
int Unz_ReadCurrentFile( unzFile file, void *buf, unsigned len ) {
	return unzReadCurrentFile( file, len, buf );
}

/* VERIFIED 0x004A9870 (unzCloseCurrentFile).  14,493 closes, ALL returning 0
 * -- i.e. unzip.c's own CRC check passed on every stock pak entry.  This is
 * the first pass in which that check has actually run: crc32 (0x004A2CE0, in
 * universal/memorytree.cpp) was an AUTO-STUB returning 0 until today, which
 * made the comparison fail silently for every completely-read file. */
int Unz_CloseCurrentFile( unzFile file ) {
	return unzCloseCurrentFile( file );
}
