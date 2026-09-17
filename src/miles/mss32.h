/*
 * mss32.h -- Miles Sound System (mss32.dll) binding for the CoD1 client.
 *
 * NOT machine output.  Hand-written, and every declaration in it is checked
 * against the retail binary rather than against a copy of Miles' own mss.h.
 *
 * WHERE THIS COMES FROM.  Retail CoDMP.exe links mss32.dll through its IMPORT
 * TABLE -- there is no LoadLibrary/GetProcAddress anywhere in the Miles layer.
 * `DUMPBIN /IMPORTS "CoDMP.exe"` lists 74 entries under mss32.dll, and every
 * one of them is decorated:
 *
 *     _AIL_startup@0   _AIL_set_3D_position@16   _AIL_process_digital_audio@24
 *
 * That decoration is the whole reason this header exists.  `_name@N` is the
 * MSVC spelling of __stdcall, and N is the argument list in BYTES.  So the
 * retail import table states the calling convention AND the exact argument
 * count of all 74 entry points, and both are load-bearing:
 *
 *   - Miles is __stdcall, so the CALLEE pops the arguments.  Calling it as
 *     cdecl leaves the stack unbalanced by N bytes on every single call.
 *   - Getting an argument count wrong unbalances it by the difference.
 *
 * Both of those are caught at LINK time here rather than at run time, because
 * the decorated name is what the linker resolves: declare AIL_set_3D_position
 * with three arguments and the compiler emits a reference to
 * _AIL_set_3D_position@12, which is not in mss32.lib, and the build stops with
 * LNK2019.  The import library is generated from that same 74-entry list
 * (opencod1/lib/mss32.def, built by opencod1/lib/gen_mss32.py), so this header
 * and the retail import table cannot drift apart without the link failing.
 *
 * FLOATS.  Miles takes F32 by value for every volume, pan, distance and 3D
 * coordinate.  The recovered bodies pass those as integer BIT PATTERNS,
 * because Hex-Rays renders a float argument as LODWORD(x) or as the raw
 * constant -- 1065353216 is 1.0f and 1056964608 is 0.5f.  Those had to be
 * converted to real floats at every call site when this header was introduced:
 * an int passed to an F32 parameter converts by VALUE, so 1056964608 would
 * have become 1.05696461e9f instead of 0.5f.  The compiler cannot warn about
 * that, so any new AIL call must be checked by hand.
 *
 * HANDLES are declared S32 rather than the opaque pointers real mss.h uses.
 * The recovered tree keeps every Miles handle in an int global; the values are
 * 4-byte opaque tokens either way, so this is ABI-identical and it keeps the
 * raw call sites free of pointer/int conversion warnings.
 */
#ifndef __MSS32_H__
#define __MSS32_H__

/* Pull in the import library.  build.py's LIBS list is shared with every other
 * area, so the dependency is declared here instead: the directive rides in
 * miles' object files, and the linker resolves the bare name against the
 * library search path -- src/miles (this file's own directory), added by
 * build.py's link step and gen_sln.py's AdditionalLibraryDirectories.  A
 * dedicated-server link never sees this directive -- build.py excludes miles/
 * -- so the server keeps its "no mss32.dll needed" property. */
#pragma comment(lib, "mss32.lib")

typedef signed int      S32;
typedef unsigned int    U32;
typedef float           F32;

typedef S32 HDIGDRIVER;      /* digital driver, 0 when not open  */
typedef S32 HSAMPLE;         /* 2D sample handle                 */
typedef S32 H3DSAMPLE;       /* 3D sample handle                 */
typedef S32 HSTREAM;         /* streaming sample handle          */
typedef S32 HPROVIDER;       /* 3D provider                      */

/* AIL_sample_status / AIL_3D_sample_status / AIL_stream_status.
 * The recovered bodies all compare against the literal 2, which is DONE --
 * "this channel is free".  Named here so new code does not have to. */
#define SMP_FREE        1
#define SMP_DONE        2
#define SMP_PLAYING     4
#define SMP_STOPPED     8
#define SMP_PLAYINGBUT  16

/* AIL_set_file_callbacks.  These are __stdcall too -- snd_miles.c's four
 * callbacks are already declared that way, which is independent confirmation:
 * the recovered MSS_FileReadCallback carries __stdcall out of the IDB. */
typedef U32  (__stdcall *AIL_FILE_OPEN_CB )( const char *filename, U32 *file_handle );
typedef void (__stdcall *AIL_FILE_CLOSE_CB)( U32 file_handle );
typedef S32  (__stdcall *AIL_FILE_SEEK_CB )( U32 file_handle, S32 offset, U32 type );
typedef U32  (__stdcall *AIL_FILE_READ_CB )( U32 file_handle, void *buffer, U32 bytes );

#define AIL_FILE_SEEK_BEGIN     0
#define AIL_FILE_SEEK_CURRENT   1
#define AIL_FILE_SEEK_END       2

/* AILSOUNDINFO, the record AIL_WAV_info fills in and that
 * AIL_size_processed_digital_audio / AIL_process_digital_audio consume.
 * 36 bytes: pinned by MSS_LoadSoundFile's `qmemcpy(v5, v15, 0x24u)` at
 * 0x0044FC10, which copies the whole record into the head of its hunk block,
 * and by that block's `v5[8] = v5 + 9` -- field 8 is initial_ptr.
 * Declared for hand-written code; the AIL entry points below take void *
 * so the raw bodies can keep passing their recovered int[] scratch record. */
typedef struct {
    S32          format;
    const void  *data_ptr;
    U32          data_len;
    U32          rate;
    S32          bits;
    S32          channels;
    U32          samples;
    U32          block_size;
    const void  *initial_ptr;
} AILSOUNDINFO;


/* ---- startup, shutdown, preferences ---- */
S32           __stdcall AIL_startup( void );
void          __stdcall AIL_shutdown( void );
const char *  __stdcall AIL_last_error( void );
S32           __stdcall AIL_set_preference( U32 number, S32 value );
void          __stdcall AIL_set_redist_directory( const char *dir );
void          __stdcall AIL_set_file_callbacks( AIL_FILE_OPEN_CB opencb, AIL_FILE_CLOSE_CB closecb, AIL_FILE_SEEK_CB seekcb, AIL_FILE_READ_CB readcb );
HDIGDRIVER    __stdcall AIL_open_digital_driver( U32 frequency, S32 bits, S32 channels, U32 flags );
void          __stdcall AIL_set_DirectSound_HWND( HDIGDRIVER dig, void *hwnd );
/* S32, NOT F32, and this one is worth spelling out.  Published Miles headers
 * declare AIL_digital_CPU_percent as returning F32, but retail MSS_Update does
 *
 *     call AIL_digital_CPU_percent / mov mss_cpuPercent, eax / cmp eax, 2
 *
 * -- it reads the result out of EAX and never touches the FPU.  If the DLL
 * really returned in ST0 that would leave a value on the x87 stack every
 * frame and overflow it after eight, turning the whole mix into NaNs; retail's
 * sound works, so this build of Miles returns an integer percent.  The
 * decoration is @4 either way, so the link cannot catch a mistake here. */
S32           __stdcall AIL_digital_CPU_percent( HDIGDRIVER dig );

/* ---- 2D samples ---- */
HSAMPLE       __stdcall AIL_allocate_sample_handle( HDIGDRIVER dig );
void          __stdcall AIL_release_sample_handle( HSAMPLE S );
void          __stdcall AIL_init_sample( HSAMPLE S );
void          __stdcall AIL_set_sample_type( HSAMPLE S, S32 format, U32 flags );
void          __stdcall AIL_set_sample_address( HSAMPLE S, const void *start, U32 len );
void          __stdcall AIL_set_sample_adpcm_block_size( HSAMPLE S, U32 blocksize );
void          __stdcall AIL_set_sample_playback_rate( HSAMPLE S, S32 playback_rate );
S32           __stdcall AIL_sample_playback_rate( HSAMPLE S );
void          __stdcall AIL_set_sample_volume_pan( HSAMPLE S, F32 volume, F32 pan );
void          __stdcall AIL_sample_volume_pan( HSAMPLE S, F32 *volume, F32 *pan );
void          __stdcall AIL_set_sample_loop_count( HSAMPLE S, S32 loops );
void          __stdcall AIL_set_sample_reverb_levels( HSAMPLE S, F32 dry_level, F32 wet_level );
void          __stdcall AIL_set_sample_ms_position( HSAMPLE S, S32 milliseconds );
void          __stdcall AIL_sample_ms_position( HSAMPLE S, S32 *total_ms, S32 *current_ms );
U32           __stdcall AIL_sample_position( HSAMPLE S );
void          __stdcall AIL_start_sample( HSAMPLE S );
void          __stdcall AIL_stop_sample( HSAMPLE S );
void          __stdcall AIL_resume_sample( HSAMPLE S );
void          __stdcall AIL_end_sample( HSAMPLE S );
S32           __stdcall AIL_sample_status( HSAMPLE S );
S32           __stdcall AIL_sample_buffer_ready( HSAMPLE S );
void          __stdcall AIL_load_sample_buffer( HSAMPLE S, U32 buff_num, const void *buffer, U32 len );
S32           __stdcall AIL_minimum_sample_buffer_size( HDIGDRIVER dig, S32 playback_rate, S32 format );

/* ---- format conversion ---- */
S32           __stdcall AIL_WAV_info( const void *data, void *info );
S32           __stdcall AIL_size_processed_digital_audio( U32 dest_rate, S32 dest_format, S32 num_srcs, void *src );
S32           __stdcall AIL_process_digital_audio( void *dest_buffer, S32 dest_buffer_size, U32 dest_rate, S32 dest_format, S32 num_srcs, void *src );

/* ---- 3D providers and room effects ---- */
S32           __stdcall AIL_enumerate_3D_providers( S32 *next, HPROVIDER *dest, char **name );
S32           __stdcall AIL_open_3D_provider( HPROVIDER lib );
void          __stdcall AIL_close_3D_provider( HPROVIDER lib );
S32           __stdcall AIL_3D_provider_attribute( HPROVIDER lib, const char *name, void *val );
void          __stdcall AIL_set_3D_distance_factor( HPROVIDER lib, F32 factor );
void          __stdcall AIL_set_3D_room_type( HPROVIDER lib, S32 room_type );
void          __stdcall AIL_set_digital_master_room_type( HDIGDRIVER dig, S32 room_type );

/* ---- 3D samples ---- */
H3DSAMPLE     __stdcall AIL_allocate_3D_sample_handle( HPROVIDER lib );
S32           __stdcall AIL_set_3D_sample_info( H3DSAMPLE S, const void *info );
void          __stdcall AIL_set_3D_position( H3DSAMPLE S, F32 X, F32 Y, F32 Z );
void          __stdcall AIL_3D_position( H3DSAMPLE S, F32 *X, F32 *Y, F32 *Z );
void          __stdcall AIL_set_3D_sample_volume( H3DSAMPLE S, F32 volume );
F32           __stdcall AIL_3D_sample_volume( H3DSAMPLE S );
void          __stdcall AIL_set_3D_sample_distances( H3DSAMPLE S, F32 max_dist, F32 min_dist );
void          __stdcall AIL_set_3D_sample_effects_level( H3DSAMPLE S, F32 level );
void          __stdcall AIL_set_3D_sample_playback_rate( H3DSAMPLE S, S32 playback_rate );
S32           __stdcall AIL_3D_sample_playback_rate( H3DSAMPLE S );
void          __stdcall AIL_set_3D_sample_loop_count( H3DSAMPLE S, U32 loops );
void          __stdcall AIL_set_3D_sample_offset( H3DSAMPLE S, U32 offset );
U32           __stdcall AIL_3D_sample_offset( H3DSAMPLE S );
U32           __stdcall AIL_3D_sample_length( H3DSAMPLE S );
S32           __stdcall AIL_3D_sample_status( H3DSAMPLE S );
void          __stdcall AIL_start_3D_sample( H3DSAMPLE S );
void          __stdcall AIL_stop_3D_sample( H3DSAMPLE S );
void          __stdcall AIL_resume_3D_sample( H3DSAMPLE S );
void          __stdcall AIL_end_3D_sample( H3DSAMPLE S );

/* ---- streams ---- */
HSTREAM       __stdcall AIL_open_stream( HDIGDRIVER dig, const char *filename, S32 stream_mem );
void          __stdcall AIL_close_stream( HSTREAM S );
void          __stdcall AIL_start_stream( HSTREAM S );
void          __stdcall AIL_pause_stream( HSTREAM S, S32 onoff );
S32           __stdcall AIL_stream_status( HSTREAM S );
void          __stdcall AIL_set_stream_playback_rate( HSTREAM S, S32 playback_rate );
S32           __stdcall AIL_stream_playback_rate( HSTREAM S );
void          __stdcall AIL_set_stream_volume_pan( HSTREAM S, F32 volume, F32 pan );
void          __stdcall AIL_stream_volume_pan( HSTREAM S, F32 *volume, F32 *pan );
void          __stdcall AIL_set_stream_loop_count( HSTREAM S, S32 count );
void          __stdcall AIL_set_stream_ms_position( HSTREAM S, S32 milliseconds );
void          __stdcall AIL_stream_ms_position( HSTREAM S, S32 *total_ms, S32 *current_ms );
void          __stdcall AIL_set_stream_reverb_levels( HSTREAM S, F32 dry_level, F32 wet_level );

#endif /* __MSS32_H__ */
