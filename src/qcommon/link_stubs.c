/*
 * Definitions for whatever the link is still missing.
 *
 * Every symbol gets a definition so the build produces an executable:
 *
 *   a retail function        ->  int name() { return 0; }
 *   retail data              ->  a byte array of the item's real size
 *   anything else            ->  int name;
 *
 * This file includes NO project headers, deliberately.  Many of these symbols are
 * unresolved precisely BECAUSE a header declares them with a prototype the
 * recovered code contradicts.  C linkage only matches on the name, so defining
 * them in a header-free translation unit resolves the reference without ever
 * letting the two declarations meet.
 *
 * A stubbed function returns 0 and does nothing.  The binary links and starts;
 * any code path that depends on a stub will behave wrongly.  Win32 imports are
 * NOT stubbed here -- see win32/win_imports.c.
 */

/* ---- functions: 71 ---- */

/* libjpeg compress side -- real bodies in jpeg-6/, no stubs:
 *   0x00516A00  jpeg_create_compress   (jcapimin.c)
 *   0x00516B10  jpeg_finish_compress   (jcapimin.c)
 *   0x00517140  jpeg_set_quality       (jcparam.c)
 *   0x00517280  jpeg_set_defaults      (jcparam.c)
 * Retail links jcphuff.c: sub_520560, called from jinit_compress_master's
 * progressive_mode arm, allocates a 108-byte struct and installs a
 * jinit_phuff_encoder-shaped start_pass, and the two "Invalid progressive
 * parameters" strings at 0x543498/0x5434D0 match jcmaster.c validate_script.
 * jpeg_start_compress/jpeg_write_scanlines (jcapistd.c) are in
 * renderer/tr_image.c. */
int CreateWindowExA() { return 0; }   /* 0x005250A6 */
int DestroyWindow() { return 0; }   /* 0x005250B8 */
/* NOT `return 0`.  This returns an HRESULT, where 0 is S_OK: a zero stub tells
 * Sys_GetDDrawVidMem_m the call succeeded, its `if (hr < 0) return 0` guard
 * passes, and it dereferences an interface pointer the call never filled in.
 * A negative value is the only honest stub for an API whose failure is
 * signalled by a negative return.  E_FAIL.  The same applies to any other
 * HRESULT or "returns zero on failure" import stubbed here.
 */
int DirectDrawCreateEx() { return (int) 0x80004005; }   /* 0x00524FBC */
int GetModuleHandleA() { return 0; }   /* 0x00524FF2 */
int stl_map_insert_m() { return 0; }   /* 0x004AD170 */
int stl_tree_erase_all_m() { return 0; }   /* 0x004AD2E0 */
int stl_map44_iter_prev_m() { return 0; }   /* 0x004AE5B0 */
int stl_map_iter_prev_m() { return 0; }   /* 0x004AEBF0 */
int stl_map44_buynode_m() { return 0; }   /* 0x004AF100 */
int stl_map_iter_next104_m() { return 0; }   /* 0x004AF3B0 */
int stl_map44_iter_next_m() { return 0; }   /* 0x004AF420 */
/* The CRT's _findfirst and _findnext, deliberately NOT renamed.  0x005278D8 is
 * FindFirstFileA + a GetLastError -> errno map (2/3 -> ENOENT, 8 -> ENOMEM,
 * else EINVAL) filling attrib/time_create/time_access/time_write/size/name at
 * +0,+4,+8,+12,+16,+20 -- the _finddata_t layout -- and returning the handle;
 * 0x005279BB is the identical body over FindNextFileA, returning 0.
 *
 * win32/win_listfiles.c and win32/win_shared.c call the real CRT
 * _findfirst/_findnext through <io.h>, so a cdecl `_findfirst` defined here
 * would either be an LNK2005 against the CRT or silently win and make every
 * FS_ListFiles and Sys_ListFiles return an empty directory.  Nothing references
 * either sub_ name. */
int sub_5278D8() { return 0; }   /* 0x005278D8 */
int sub_5279BB() { return 0; }   /* 0x005279BB */

/* ---- data: 28 ---- */
unsigned char PADDING[88];   /* 0x0057A7E0 */
unsigned char cl_cdkey[33];   /* 0x0057AC78 */
unsigned char cl_cdkeychecksum[5];   /* 0x0057AC9C */
int cl_language;   /* 0x0161734C */
/* cmd_argc 0x008930F0 / cmd_argv 0x00890BF0 live in qcommon/cmd.c; retail reads
 * them directly because /GL inlined Cmd_Argc/Cmd_Argv. */
unsigned char commandLine[1023];   /* 0x008E3710 */
int fs_loadingMode;   /* 0x01631780 */
int fs_numServerPaks;   /* 0x0162D764 */
int fs_serverPakNames;   /* 0x01625640 */
int fs_serverPaks;   /* 0x016318C0 */
int g_languages;   /* 0x00571B30 */
int g_splashWnd;   /* 0x008E5268 */
int g_wv_hWnd;   /* 0x016C35E8 */
/* lastTokenPos 0x008CCCD4 / prevTokenPos 0x008CCCD0 are static in
 * universal/q_parse.c; retail open-codes Com_UngetToken at two sites in
 * renderer/tr_shader.c, reading both directly (the tree calls
 * Com_UngetToken() there). */
int msgInit;   /* 0x014073F0 */
int parseInfo;   /* 0x0057A1B0 */
int s_hunkData;   /* 0x0140741C */
unsigned char sv[398572];   /* 0x016515A0 */
int sys_timeBase;   /* 0x016C35C0 */
int sys_timeBaseInit;   /* 0x014073B8 */
int time_backend;   /* 0x0163B3D0 */
int time_frontend;   /* 0x0163B3CC */
int time_game;   /* 0x0163A238 */

/* ---- no retail symbol: 90 ---- */
/*
 * Names with no retail symbol: invented by hand-written code, or CRT
 * internals.  A name that is ever CALLED must be defined as code; defining a
 * called name as data means the call jumps into the data section.
 */
int AIL_3D_provider_attribute() { return 0; }   /* called somewhere; must be code */
int AIL_WAV_info() { return 0; }   /* called somewhere; must be code */
int AIL_allocate_sample_handle() { return 0; }   /* called somewhere; must be code */
int AIL_close_stream() { return 0; }   /* called somewhere; must be code */
int AIL_end_sample() { return 0; }   /* called somewhere; must be code */
int AIL_enumerate_3D_providers() { return 0; }   /* called somewhere; must be code */
int AIL_init_sample() { return 0; }   /* called somewhere; must be code */
int AIL_last_error() { return 0; }   /* called somewhere; must be code */
int AIL_minimum_sample_buffer_size() { return 0; }   /* called somewhere; must be code */
int AIL_open_3D_provider() { return 0; }   /* called somewhere; must be code */
int AIL_process_digital_audio() { return 0; }   /* called somewhere; must be code */
int AIL_release_sample_handle() { return 0; }   /* called somewhere; must be code */
int AIL_sample_playback_rate() { return 0; }   /* called somewhere; must be code */
int AIL_sample_position() { return 0; }   /* called somewhere; must be code */
int AIL_sample_status() { return 0; }   /* called somewhere; must be code */
int AIL_sample_volume_pan() { return 0; }   /* called somewhere; must be code */
int AIL_set_3D_distance_factor() { return 0; }   /* called somewhere; must be code */
int AIL_set_3D_position() { return 0; }   /* called somewhere; must be code */
int AIL_set_3D_room_type() { return 0; }   /* called somewhere; must be code */
int AIL_set_DirectSound_HWND() { return 0; }   /* called somewhere; must be code */
int AIL_set_digital_master_room_type() { return 0; }   /* called somewhere; must be code */
int AIL_set_sample_playback_rate() { return 0; }   /* called somewhere; must be code */
int AIL_set_sample_type() { return 0; }   /* called somewhere; must be code */
int AIL_set_sample_volume_pan() { return 0; }   /* called somewhere; must be code */
int AIL_size_processed_digital_audio() { return 0; }   /* called somewhere; must be code */
int AIL_stream_playback_rate() { return 0; }   /* called somewhere; must be code */
int AIL_stream_status() { return 0; }   /* called somewhere; must be code */
int AIL_stream_volume_pan() { return 0; }   /* called somewhere; must be code */
int COM_DefaultExtension() { return 0; }   /* called somewhere; must be code */
int Com_ResetParseSession() { return 0; }   /* called somewhere; must be code */
int CompileTransferRefToString() { return 0; }   /* called somewhere; must be code */
int EmitArrayPrimitiveExpressionRef() { return 0; }   /* called somewhere; must be code */
int EmitCall() { return 0; }   /* called somewhere; must be code */
int EmitCanonicalString() { return 0; }   /* called somewhere; must be code */
int EmitExpression() { return 0; }   /* called somewhere; must be code */
int EmitFormalParameterListRefInternal() { return 0; }   /* called somewhere; must be code */
int EmitFormalWaittillParameterListRefInternal() { return 0; }   /* called somewhere; must be code */
int EmitFunction() { return 0; }   /* called somewhere; must be code */
int EmitMethod() { return 0; }   /* called somewhere; must be code */
int EmitOpcode() { return 0; }   /* called somewhere; must be code */
int EmitPrimitiveExpressionFieldObject() { return 0; }   /* called somewhere; must be code */
int EmitStatement() { return 0; }   /* called somewhere; must be code */
int EmitThread() { return 0; }   /* called somewhere; must be code */
int EmitVariableExpressionRef() { return 0; }   /* called somewhere; must be code */
int GetCurrentProcessId() { return 0; }   /* called somewhere; must be code */
int GetPrevSourcePos() { return 0; }   /* called somewhere; must be code */
int SEH_GetLanguageString() { return 0; }   /* called somewhere; must be code */
int Scr_InitDeveloperOpcodes() { return 0; }   /* called somewhere; must be code */
int Scr_TransferToDeveloperBuffer() { return 0; }   /* called somewhere; must be code */
int SpecifyThread() { return 0; }   /* called somewhere; must be code */
int Sys_PlatformExit() { return 0; }   /* called somewhere; must be code */
int UnmatchingTypesError() { return 0; }   /* called somewhere; must be code */
/* `return 0` IS CORRECT here; this is not a stub.  1,256 of the 1,411 x87
 * compare sequences in the binary map exactly onto plain C comparison
 * semantics (already false on NaN), so a real unordered test would INVERT
 * them; `test ah,44h` never pairs with jz/jnz anywhere in the image.  The
 * mask/branch table is in qcommon/hexrays_shim.h, which defines this as
 * `( 0 )`; this definition covers a unit that misses that header.  The 155
 * NaN-divergent sites need `!(a <= b)` / `!(a >= b)` written at the site. */
int __UNORDERED__() { return 0; }
/* No `__rdtsc` stub: qcommon/crt_thunks.c has the real body, `push edx / rdtsc
 * / pop edx / ret`, matching retail's own out-of-line COMDAT at 0x00475840.
 * A constant 0 disables scr_vm.cpp's OP_JumpBack infinite-loop watchdog
 * (`if ( (int)( __rdtsc() - scrVmGlob_loopTick ) >= 0 ) continue;`). */
/* No `alloca` stub: a unit that calls `alloca` without <malloc.h> gets C89's
 * implicit `extern int alloca()` and would bind here, so the allocation would
 * silently never happen.  Add the include instead. */
int cand_RotatePointByAngles;
int com_clientDObjHandles;
int com_dobjAllocBits;
int com_dobjLastIndex;
int com_dobjPool;
int com_serverDObjHandles;
int g_unk_A9CC58;
int g_wv_hInstance;
int g_wv_isMinimized;
int SE_LookupString_m() { return 0; }   /* called somewhere; must be code */
int stl_map_buynode_str_m() { return 0; }   /* called somewhere; must be code */

/* ---- hand-added ------------------------------------------------------
 *
 *   CM_AreaEntities_r        cm_world.c defines it `static` (which the
 *                            disassembly supports), but server_mp/sv_world_mp.c
 *                            still calls it as an extern.  One of the two is
 *                            wrong; the call site is the suspect.
 *   DirectDrawEnumerateExA   ddraw, only reached by video-memory detection; a
 *                            dedicated server does not need it.
 */
int CM_AreaEntities_r() { return 0; }
int DirectDrawEnumerateExA() { return (int) 0x80004005; }   /* HRESULT: E_FAIL */
