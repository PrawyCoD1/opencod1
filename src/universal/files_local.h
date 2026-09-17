/*
 * files_local.h -- filesystem-private types for Call of Duty 1.1.
 *
 * Shared by universal/com_files.c (which owns fs_searchpaths, the fsh[] handle
 * array and the pak loader) and qcommon/files.c (the pure-server and
 * pak-reference side).  Nothing outside those two units should include this.
 *
 *   fileHandleData_t   288 bytes (the handle array stride is 72 dwords):
 *                          +0x00  handleFiles.file   fsh base 0x01635A00
 *                          +0x08  handleSync         fsh_handleSync
 *                          +0x18  zipFile            fsh_zipFile
 *                          +0x1C  streamed           dword_1635A1C
 *                          +0x20  name[256]          fsh_name
 *                      RTCW's fileHandleData_t exactly.
 *
 *   pack_t             800 bytes.  hashSize at +788 and hashTable at +792
 *                      (FS_FileIsInPAK).
 *
 *   fileInPack_t       24 bytes.  FS_FileIsInPAK walks the hash chain with
 *                      *(node + 20), so `next` is at +0x14: the six-field
 *                      layout, not RTCW's 12-byte three-field one.
 *
 *   searchpath_t       20 bytes.  The `localized` and `language` fields are
 *                      unconfirmed for 1.1 (RTCW has neither); if handle or
 *                      search-path behaviour misbehaves at runtime, suspect
 *                      this struct first.
  *
 * @fidelity-default: verified
 */

#ifndef __FILES_LOCAL_H__
#define __FILES_LOCAL_H__

#include "../qcommon/cod1_types.h"

#include <stdio.h>

#define MAX_OSPATH          256
#define MAX_ZPATH           256
#define MAX_SEARCH_PATHS    4096
#define MAX_FILEHASH_SIZE   1024
#define MAX_FOUND_FILES     0x1000
#define MAX_FILE_HANDLES    64

/* Two rotating scratch buffers for PakFileLanguage(); the CODUO_ names come
 * from opencoduo. */
#define CODUO_FS_LANGUAGE_NAME_BUFFER_COUNT 2
#define CODUO_FS_LANGUAGE_NAME_BUFFER_SIZE  64

typedef void *unzFile;

/*
 * minizip's unz_file_info, the struct Unz_GetCurrentFileInfo fills.
 *
 * unzOpenCurrentFile (0x004A94A0) reads compression_method at unz_s+52, crc
 * at +60, compressed_size at +64 and uncompressed_size at +68; cur_file_info
 * begins at unz_s+40, so those are +12, +20, +24, +28 here.  Total 80 bytes,
 * ending at the cur_file_info_internal dword at unz_s+120.
 */
typedef struct {
	unsigned int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year;
} unz_tm_unz;

typedef struct {
	unsigned int version;              /* +0x00 */
	unsigned int version_needed;       /* +0x04 */
	unsigned int flag;                 /* +0x08 */
	unsigned int compression_method;   /* +0x0C */
	unsigned int dosDate;              /* +0x10 */
	unsigned int crc;                  /* +0x14 */
	unsigned int compressed_size;      /* +0x18 */
	unsigned int uncompressed_size;    /* +0x1C */
	unsigned int size_filename;        /* +0x20 */
	unsigned int size_file_extra;      /* +0x24 */
	unsigned int size_file_comment;    /* +0x28 */
	unsigned int disk_num_start;       /* +0x2C */
	unsigned int internal_fa;          /* +0x30 */
	unsigned int external_fa;          /* +0x34 */
	unz_tm_unz   tmu_date;             /* +0x38 */
} unz_file_info;
COD1_ASSERT_SIZE( unz_file_info, 80 );

typedef union qfile_gu {
	FILE        *o;
	unzFile z;
} qfile_gut;

typedef struct qfile_us {
	qfile_gut file;
	qboolean unique;
} qfile_ut;

typedef struct {
	qfile_ut handleFiles;
	qboolean handleSync;
	int baseOffset;
	int fileSize;
	int zipFilePos;
	qboolean zipFile;
	qboolean streamed;
	char name[MAX_ZPATH];
} fileHandleData_t;
COD1_ASSERT_SIZE( fileHandleData_t, 288 );

typedef struct fileInPack_s {
	int zipPosition;                    /* +0x00  file info position in the zip */
	char                    *basename;  /* +0x04 */
	void                    *payload;   /* +0x08 */
	void ( *clearCallback )( void * );  /* +0x0C */
	char                    *name;      /* +0x10  full name of the file */
	struct fileInPack_s     *next;      /* +0x14  next file in the hash chain */
} fileInPack_t;
COD1_ASSERT_SIZE( fileInPack_t, 24 );

typedef struct {
	char pakFilename[MAX_OSPATH];       /* +0x000  c:\quake3\baseq3\pak0.pk3 */
	char pakBasename[MAX_OSPATH];       /* +0x100  pak0 */
	char pakGamename[MAX_OSPATH];       /* +0x200  baseq3 */
	unzFile handle;                     /* +0x300  handle to zip file */
	int checksum;                       /* +0x304 */
	int pure_checksum;                  /* +0x308 */
	int numfiles;                       /* +0x30C  number of files in pk3 */
	int referenced;                     /* +0x310 */
	int hashSize;                       /* +0x314  hash table size (power of 2) */
	fileInPack_t            **hashTable; /* +0x318 */
	fileInPack_t            *buildBuffer; /* +0x31C */
} pack_t;
COD1_ASSERT_SIZE( pack_t, 800 );

typedef struct {
	char path[MAX_OSPATH];              /* c:\quake3 */
	char gamedir[MAX_OSPATH];           /* baseq3 */
} directory_t;
COD1_ASSERT_SIZE( directory_t, 512 );

typedef struct searchpath_s {
	struct searchpath_s *next;
	pack_t              *pack;          /* only one of pack / dir will be non-NULL */
	directory_t         *dir;
	qboolean localized;                 /* ASSUMED -- see header comment */
	int language;                       /* ASSUMED -- see header comment */
} searchpath_t;
COD1_ASSERT_SIZE( searchpath_t, 20 );

extern searchpath_t     *fs_searchpaths;
extern fileHandleData_t fsh[MAX_FILE_HANDLES];

/*
 * The cached-asset record FS_GetDataForFile returns.
 *
 * Both lookup paths in FS_GetDataForFile (0x0042C900) return a pointer to the
 * SAME three-field shape, and FS_ClearDataForFiles (0x0042CA60) hands that
 * same pointer to the clear callback:
 *
 *   pak path   returns &fileInPack->basename, i.e. fileInPack_t + 4, whose next
 *              three words are basename / payload / clearCallback (0x0042CA53,
 *              and 0x0042CA8x hands `j + 1` to the callback);
 *   dir path   returns the fs_dirFile record itself, whose first three words
 *              are name / data / clearCallback (0x0042CA58, and the callback
 *              receives `n`).
 *
 * So the two record types are laid out so their tails coincide.  Only these
 * three fields are common; `next` sits at +0x14 in fileInPack_t and at +0x0C in
 * the dir record, and no caller of FS_GetDataForFile reads it.
 */
typedef struct fileData_s {
	char    *name;                          /* +0x00 */
	void    *data;                          /* +0x04 */
	void  ( *clearCallback )( struct fileData_s * );  /* +0x08 */
} fileData_t;

/*
 * The per-directory cached-file list -- retail's second FS_GetDataForFile
 * lookup, head at 0x0140742C.
 *
 * Offsets (FS_GetDataForFile 0x0042C9E0-0x0042CA33, FS_ClearDataForFiles
 * 0x0042CAD1, FS_ShutdownFileLists 0x0042CBD0): the node base is compared with
 * _stricmp against the `dir` argument, so the name is a char array at +0;
 * hashSize at +0x104; hashTable at +0x108; a second allocation freed at
 * shutdown at +0x10C; next at +0x110.
 *
 * The builder is FS_AddNonPackFileDirectory_Internal (0x0042C560); it reaches
 * the list through the live head at 0x01407424 while every reader goes through
 * the published head at 0x0140742C.  It does `strncpy(list, dir, 255)` with
 * `list[255] = 0`, then stores the file count at +0x100 and the hash size at
 * +0x104.
 *
 * The builder is currently a no-op (see com_files.c), so only the first
 * lookup (the pak hash) can hit.
 */
typedef struct fs_dirFile_s {
	char    *name;                          /* +0x00 */
	void    *data;                          /* +0x04 */
	void  ( *clearCallback )( struct fileData_s * );  /* +0x08 */
	struct fs_dirFile_s *next;              /* +0x0C  hash chain */
} fs_dirFile_t;

typedef struct fs_dirFileList_s {
	char    dir[256];                       /* +0x000 */
	int     numFiles;                       /* +0x100 */
	int     hashSize;                       /* +0x104 */
	fs_dirFile_t **hashTable;               /* +0x108 */
	void    *buildBuffer;                   /* +0x10C  freed by FS_ShutdownFileLists */
	struct fs_dirFileList_s *next;          /* +0x110 */
} fs_dirFileList_t;

/*
 * 0x0042C900.  THREE arguments -- the third arrives in EBX.  Formats
 * "%s/%s%s" (dir, name, ext) for the pak lookup and "%s%s" (name, ext) for the
 * per-directory lookup.
 */
fileData_t *FS_GetDataForFile( const char *dir, const char *name, const char *ext );

/*
 * Shared between universal/com_files.c (which defines them) and
 * qcommon/files.c.  fs_numServerPaks and fs_serverPaks are also declared by
 * qcommon.h; the spellings agree.
 */
extern int      fs_checksumFeed;
extern int      fs_fakeChkSum;

extern int      fs_numServerPaks;
extern int      fs_serverPaks[MAX_SEARCH_PATHS];
extern char     *fs_serverPakNames[MAX_SEARCH_PATHS];

extern int      fs_numServerReferencedPaks;
extern int      fs_serverReferencedPaks[MAX_SEARCH_PATHS];
extern char     *fs_serverReferencedPakNames[MAX_SEARCH_PATHS];

/* Not static: FS_SetRestrictions (qcommon, 0x0043BAD0) calls it for the
 * fs_restrict demo restart, FS_Shutdown(qfalse) + FS_Startup("demomain"). */
void FS_Startup( const char *gameName );

extern cvar_t   *fs_debug;
extern cvar_t   *fs_homepath;
extern cvar_t   *fs_basepath;
extern cvar_t   *fs_basegame;
extern cvar_t   *fs_cdpath;
extern cvar_t   *fs_gamedirvar;
extern cvar_t   *fs_restrict;

long FS_HashFileName( const char *fname, int hashSize );

/* 0x00428DB0 -- the range selector is a parameter (EAX), not a global.
 * `streamed` nonzero draws from handles 51..63, zero from 1..50. */
fileHandle_t FS_HandleForFileStreamed( int streamed );
fileHandle_t FS_HandleForFile( void );          /* == FS_HandleForFileStreamed(0) */

FILE *FS_FileForHandle( fileHandle_t f );
qboolean FS_PakIsPure( pack_t *pack );
qboolean FS_FilenameCompare( const char *s1, const char *s2 );

/* The retail join, 0x00428EE0: base '/' game '/' qpath.  See com_files.c for
 * the register mapping.  FS_BuildOSPath_Internal below is the two-component
 * form the other callers use; it is a deliberate divergence. */
void FS_BuildOSPath3( const char *base, const char *game, const char *qpath,
					  char *fs_path, int allowOverflow );
void FS_BuildOSPath_Internal( const char *base, const char *qpath, char *out, int flags );
qboolean FS_CreatePath( char *OSPath );
qboolean FS_CopyFile( char *fromOSPath, char *toOSPath );
int FS_filelength( fileHandle_t f );

#endif  /* __FILES_LOCAL_H__ */
