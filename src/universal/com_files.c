/*
 * universal/com_files.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/com_files.c
 *
 * Retail range 0x00428C80-0x0042D5C0.
 *
 * @fidelity-default: likely
 */

#include "../qcommon/qcommon.h"
#include "files_local.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <io.h>
#include <direct.h>
#include <windows.h>

#define BASEGAME            "main"
#define DEMOGAME            "demomain"
#define MAX_ZIPPED_FILES    16384

searchpath_t        *fs_searchpaths;
fileHandleData_t fsh[MAX_FILE_HANDLES];

extern int fs_dirFileLists;
extern int fs_savedSearchpaths;
extern int fs_savedDirFileLists;

#define fs_dirFileLists       ( *(fs_dirFileList_t **)&fs_dirFileLists )
#define fs_savedSearchpaths   ( *(searchpath_t **)&fs_savedSearchpaths )
#define fs_savedDirFileLists  ( *(fs_dirFileList_t **)&fs_savedDirFileLists )

cvar_t  *fs_debug;
const char *g_languages[14] = {
	"english",  "french",  "german",  "italian",
	"spanish",  "british", "russian", "polish",
	"korean",   "taiwanese", "japanese", "chinese",
	"thai",     "leet"
};
#define NUM_LANGUAGES  14

cvar_t  *fs_homepath;
cvar_t  *fs_basepath;
cvar_t  *fs_basegame;
cvar_t  *fs_cdpath;
cvar_t  *fs_gamedirvar;
cvar_t  *fs_restrict;
cvar_t  *fs_ignoreLocalized;

static int fs_loadStack;
static int fs_readCount;
static int fs_loadCount;
static int fs_packFiles;

int fs_checksumFeed;                    /* 0x0162D768 */
int fs_fakeChkSum;                      /* 0x01631788 */

int fs_numServerPaks;                   /* 0x0162D764 */
int fs_serverPaks[MAX_SEARCH_PATHS];    /* 0x016318C0 */
char        *fs_serverPakNames[MAX_SEARCH_PATHS];   /* 0x01625640 */

int fs_numServerReferencedPaks;
int fs_serverReferencedPaks[MAX_SEARCH_PATHS];
char        *fs_serverReferencedPakNames[MAX_SEARCH_PATHS];

char fs_gamedir[MAX_OSPATH];

static char lastValidBase[MAX_OSPATH];
static char lastValidGame[MAX_OSPATH];

int  SEH_InitLanguage( void );                                  /* 0x004A9A60 */
int  Clear__16CStringEdPackageFi( void *package, int keepFileInfo ); /* 0x004AAAA0 */
extern int TheStringPackage;                                    /* 0x01407450 */

int  FS_SetRestrictions( void );                                /* 0x0043BAD0 */

int  Sys_DirectoryHasContents_m( const char *path );

unzFile Unz_Open( const char *path );
int      Unz_Close( unzFile file );
unzFile Unz_ReOpen( const char *path, unzFile file );
int      Unz_GetGlobalNumFiles( unzFile file, int *numFiles );
int      Unz_GoToFirstFile( unzFile file );
int      Unz_GoToNextFile( unzFile file );
int      Unz_GetCurrentFileInfo( unzFile file, void *pfile_info,
								 char *szFileName, unsigned fileNameBufferSize,
								 void *extraField, unsigned extraFieldBufferSize,
								 char *szComment, unsigned commentBufferSize );
unsigned long Unz_GetCurrentFileInfoPosition( unzFile file );
int      Unz_SetCurrentFileInfoPosition( unzFile file, unsigned long pos );
int      Unz_GetCurrentFileUncompressedSize( unzFile file );
int      Unz_OpenCurrentFile( unzFile file );
int      Unz_ReadCurrentFile( unzFile file, void *buf, unsigned len );
int      Unz_CloseCurrentFile( unzFile file );

int      unztell( unzFile file );

/* ---- FS_Initialized  0x00428C80 ---- VERIFIED */
qboolean FS_Initialized( void ) {
	return ( fs_searchpaths != NULL );
}

/* ---- FS_CheckFileSystemStarted  0x00428C90 ---- VERIFIED */
void FS_CheckFileSystemStarted( void ) {
}

/* ---- FS_PakIsPure  0x00428CA0 ---- VERIFIED */
qboolean FS_PakIsPure( pack_t *pack ) {
	int i;

	if ( fs_numServerPaks ) {
		for ( i = 0 ; i < fs_numServerPaks ; i++ ) {
			if ( pack->checksum == fs_serverPaks[i] ) {
				return qtrue;
			}
		}
		return qfalse;
	}
	return qtrue;
}

/* ---- FS_LoadStack  0x00428CD0 ---- VERIFIED */
int FS_LoadStack( void ) {
	return fs_loadStack;
}

/* ---- FS_UseSearchPath  0x00428CE0 ---- VERIFIED */
qboolean FS_UseSearchPath( searchpath_t *sp ) {
	extern cvar_t *cl_language;

	if ( !sp->localized ) {
		return qtrue;
	}
	if ( fs_ignoreLocalized && fs_ignoreLocalized->integer ) {
		return qfalse;
	}
	return (qboolean) ( sp->language == cl_language->integer );
}

/* ---- FS_LanguageHasAssets  0x00428D20 ---- VERIFIED */
qboolean FS_LanguageHasAssets( int language ) {
	searchpath_t *sp;

	for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
		if ( sp->localized && sp->language == language ) {
			return qtrue;
		}
	}
	return qfalse;
}

/* ---- FS_HashFileName  0x00428D50 ---- VERIFIED */
long FS_HashFileName( const char *fname, int hashSize ) {
	int i;
	long hash;
	char letter;

	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		letter = tolower( fname[i] );
		if ( letter == '.' ) {
			break;
		}
		if ( letter == '\\' ) {
			letter = '/';
		}
		hash += (long)( letter ) * ( i + 119 );
		i++;
	}
	hash = hash ^ ( ( hash ^ ( hash >> 10 ) ) >> 10 );
	hash &= ( hashSize - 1 );
	return hash;
}

/* ---- FS_HandleForFile  0x00428DB0 ---- VERIFIED */
fileHandle_t FS_HandleForFileStreamed( int streamed ) {
	int i;
	int start, count;

	if ( streamed ) {
		start = 51;
		count = 13;
	} else {
		start = 1;
		count = 50;
	}

	for ( i = 0 ; i < count ; i++ ) {
		if ( fsh[start + i].handleFiles.file.o == NULL ) {
			return start + i;
		}
	}

	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		Com_Printf( "FILE %2i: '%s'\n", i, fsh[i].name );
	}
	Com_Error( ERR_DROP, "\x15" "FS_HandleForFile: none free" );
	return -1;
}

fileHandle_t FS_HandleForFile( void ) {
	return FS_HandleForFileStreamed( 0 );
}

/* ---- FS_FileForHandle  0x00428E40 ---- VERIFIED */
FILE *FS_FileForHandle( fileHandle_t f ) {
	return fsh[f].handleFiles.file.o;
}

/* ---- FS_ForceFlush  0x00428E50 ---- VERIFIED */
void FS_ForceFlush( fileHandle_t f ) {
	FILE *file;

	file = FS_FileForHandle( f );
	setvbuf( file, NULL, _IONBF, 0 );
}

/* ---- FS_filelength  0x00428E70 ---- VERIFIED */
int FS_filelength( fileHandle_t f ) {
	FILE *h;
	int pos;
	int end;

	if ( fsh[f].zipFile == qtrue ) {
		return Unz_GetCurrentFileUncompressedSize( fsh[f].handleFiles.file.z );
	}

	h = FS_FileForHandle( f );
	pos = ftell( h );
	fseek( h, 0, SEEK_END );
	end = ftell( h );
	fseek( h, pos, SEEK_SET );

	return end;
}

/* ---- FS_ReplaceSeparators  0x00428EC0 ---- VERIFIED */
void FS_ReplaceSeparators( char *path ) {
	char *s;

	for ( s = path ; *s ; s++ ) {
		if ( *s == '/' || *s == '\\' ) {
			*s = '\\';
		}
	}
}

/* ---- FS_BuildOSPath3  0x00428EE0 ---- VERIFIED */
void FS_BuildOSPath3( const char *base, const char *game, const char *qpath,
					  char *fs_path, int allowOverflow ) {
	int baseLen, gameLen, qLen;
	char *s;

	if ( !game || !*game ) {
		game = fs_gamedir;
	}

	baseLen = strlen( base );
	gameLen = strlen( game );
	qLen = strlen( qpath );

	if ( baseLen + gameLen + qLen + 2 < MAX_OSPATH ) {
		Com_Memcpy( fs_path, base, baseLen );
		s = &fs_path[baseLen];
		*s = '/';
		Com_Memcpy( &fs_path[baseLen + 1], game, gameLen );
		fs_path[baseLen + gameLen + 1] = '/';
		Com_Memcpy( &fs_path[baseLen + gameLen + 2], qpath, qLen + 1 );
		FS_ReplaceSeparators( s );
	} else {
		if ( !allowOverflow ) {
			Com_Error( ERR_FATAL, "\x15" "FS_BuildOSPath: os path length exceeded\n" );
		}
		*fs_path = 0;
	}
}

/* FS_BuildOSPath_Internal  0x00428EE0 */
void FS_BuildOSPath_Internal( const char *base, const char *qpath, char *fs_path, int allowOverflow ) {
	int baseLen, qLen;
	char *s;

	if ( !base || !*base ) {
		base = fs_homepath->string;
	}

	baseLen = strlen( base );
	qLen = strlen( qpath );

	if ( baseLen + qLen + 2 < MAX_OSPATH ) {
		Com_Memcpy( fs_path, base, baseLen );
		s = &fs_path[baseLen];
		*s = '/';
		Com_Memcpy( &fs_path[baseLen + 1], qpath, qLen + 1 );
		FS_ReplaceSeparators( s );
	} else {
		if ( !allowOverflow ) {
			Com_Error( ERR_FATAL, "\x15" "FS_BuildOSPath: os path length exceeded\n" );
		}
		*fs_path = 0;
	}
}

/* ---- FS_BuildOSPath  0x00428FE0 ---- VERIFIED */
char *FS_BuildOSPath( const char *base, const char *game, const char *qpath ) {
	static char ospath[MAX_OSPATH];
	FS_BuildOSPath3( base, game, qpath, ospath, 0 );
	return ospath;
}

/* ---- FS_CreatePath  0x00429000 ---- VERIFIED */
qboolean FS_CreatePath( char *OSPath ) {
	char *ofs;

	if ( strstr( OSPath, ".." ) || strstr( OSPath, "::" ) ) {
		Com_Printf( "WARNING: refusing to create relative path \"%s\"\n", OSPath );
		return qtrue;
	}

	for ( ofs = OSPath + 1 ; *ofs ; ofs++ ) {
		if ( *ofs == '\\' ) {
			*ofs = 0;
			_mkdir( OSPath );
			*ofs = '\\';
		}
	}
	return qfalse;
}

/* ---- FS_CopyFile  0x00429070 ---- VERIFIED */
qboolean FS_CopyFile( char *fromOSPath, char *toOSPath ) {
	FILE *f;
	int len;
	byte *buf;

	if ( strstr( fromOSPath, "journal.dat" ) || strstr( fromOSPath, "journaldata.dat" ) ) {
		return qfalse;
	}

	f = fopen( fromOSPath, "rb" );
	if ( !f ) {
		return qfalse;
	}
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, 0, SEEK_SET );

	buf = malloc( len );
	if ( fread( buf, 1, len, f ) != (size_t)len ) {
		Com_Error( ERR_FATAL, "\x15" "Short read in FS_Copyfiles()\n" );
	}
	fclose( f );

	if ( FS_CreatePath( toOSPath ) ) {
		free( buf );
		return qfalse;
	}

	f = fopen( toOSPath, "wb" );
	if ( !f ) {
		free( buf );
		return qfalse;
	}
	if ( fwrite( buf, 1, len, f ) != (size_t)len ) {
		Com_Error( ERR_FATAL, "\x15" "Short write in FS_Copyfiles()\n" );
	}
	fclose( f );
	free( buf );
	return qtrue;
}

/* ---- FS_CopyFiles  0x00429170 ---- VERIFIED */
void FS_CopyFiles( const char *fromDir, const char *toDir, const char *ext ) {
	char fromOSPath[MAX_OSPATH];
	char toOSPath[MAX_OSPATH];
	FILE *f;
	int len;
	byte *buf;

	FS_BuildOSPath_Internal( fs_homepath->string, fromDir, fromOSPath, 0 );
	FS_BuildOSPath_Internal( fs_homepath->string, toDir, toOSPath, 0 );

	if ( strstr( fromOSPath, "journal.dat" ) || strstr( fromOSPath, "journaldata.dat" ) ) {
		Com_Printf( "Ignoring journal files\n" );
		return;
	}

	f = fopen( fromOSPath, "rb" );
	if ( !f ) {
		return;
	}
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, 0, SEEK_SET );
	buf = malloc( len );
	if ( fread( buf, 1, len, f ) != (size_t)len ) {
		Com_Error( ERR_FATAL, "\x15" "Short read in FS_Copyfiles()\n" );
	}
	fclose( f );

	if ( FS_CreatePath( toOSPath ) == qfalse ) {
		f = fopen( toOSPath, "wb" );
		if ( f ) {
			if ( fwrite( buf, 1, len, f ) != (size_t)len ) {
				Com_Error( ERR_FATAL, "\x15" "Short write in FS_Copyfiles()\n" );
			}
			fclose( f );
		}
	}
	free( buf );
}

/* ---- FS_Remove  0x00429300 ---- VERIFIED */
void FS_Remove( const char *osPath ) {
	remove( osPath );
}

/* ---- FS_FileExists  0x00429310 ---- VERIFIED */
qboolean FS_FileExists( const char *file ) {
	FILE *f;
	char testpath[MAX_OSPATH];

	FS_BuildOSPath_Internal( fs_homepath->string, file, testpath, 0 );

	f = fopen( testpath, "rb" );
	if ( f ) {
		fclose( f );
		return qtrue;
	}
	return qfalse;
}

/* ---- FS_Rename  0x004293B0 ---- VERIFIED */
void FS_Rename( const char *from, const char *to ) {
	char fromOSPath[MAX_OSPATH];
	char toOSPath[MAX_OSPATH];

	FS_BuildOSPath_Internal( fs_homepath->string, from, fromOSPath, 0 );
	FS_BuildOSPath_Internal( fs_homepath->string, to, toOSPath, 0 );

	if ( fs_debug->integer ) {
		Com_Printf( "FS_Rename: %s --> %s\n", fromOSPath, toOSPath );
	}

	if ( rename( fromOSPath, toOSPath ) ) {
		remove( toOSPath );
		if ( rename( fromOSPath, toOSPath ) ) {
			FS_CopyFile( fromOSPath, toOSPath );
			remove( fromOSPath );
		}
	}
}

/* ---- FS_FCloseFile  0x004294B0 ---- VERIFIED */
void FS_FCloseFile( fileHandle_t f ) {
	if ( fsh[f].zipFile == qtrue ) {
		Unz_CloseCurrentFile( fsh[f].handleFiles.file.z );
		if ( fsh[f].handleFiles.unique ) {
			Unz_Close( fsh[f].handleFiles.file.z );
		}
		Com_Memset( &fsh[f], 0, sizeof( fsh[f] ) );
		return;
	}

	if ( fsh[f].handleFiles.file.o ) {
		fclose( fsh[f].handleFiles.file.o );
	}
	Com_Memset( &fsh[f], 0, sizeof( fsh[f] ) );
}

/* ---- FS_FOpenFileWrite  0x00429550 ---- VERIFIED */
fileHandle_t FS_FOpenFileWrite( const char *filename ) {
	fileHandle_t f;
	char ospath[MAX_OSPATH];

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	FS_BuildOSPath3( fs_homepath->string, fs_gamedir, filename, ospath, 0 );

	if ( fs_debug->integer ) {
		Com_Printf( "FS_FOpenFileWrite: %s\n", ospath );
	}

	if ( FS_CreatePath( ospath ) ) {
		return 0;
	}

	fsh[f].handleFiles.file.o = fopen( ospath, "wb" );
	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	fsh[f].handleSync = qfalse;
	if ( !fsh[f].handleFiles.file.o ) {
		return 0;
	}
	return f;
}

/* ---- FS_FOpenTextFileWrite  0x00429640 ---- VERIFIED */
fileHandle_t FS_FOpenTextFileWrite( const char *filename ) {
	fileHandle_t f;
	char ospath[MAX_OSPATH];

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	FS_BuildOSPath3( fs_homepath->string, fs_gamedir, filename, ospath, 0 );

	if ( fs_debug->integer ) {
		Com_Printf( "FS_FOpenFileWrite: %s\n", ospath );
	}

	if ( FS_CreatePath( ospath ) ) {
		return 0;
	}

	fsh[f].handleFiles.file.o = fopen( ospath, "wt" );
	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	fsh[f].handleSync = qfalse;
	if ( !fsh[f].handleFiles.file.o ) {
		return 0;
	}
	return f;
}

/* ---- FS_FOpenFileAppend  0x00429730 ---- VERIFIED */
fileHandle_t FS_FOpenFileAppend( const char *filename ) {
	fileHandle_t f;
	char ospath[MAX_OSPATH];

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	FS_BuildOSPath3( fs_homepath->string, fs_gamedir, filename, ospath, 0 );

	if ( fs_debug->integer ) {
		Com_Printf( "FS_FOpenFileAppend: %s\n", ospath );
	}

	if ( FS_CreatePath( ospath ) ) {
		return 0;
	}

	fsh[f].handleFiles.file.o = fopen( ospath, "ab" );
	fsh[f].handleSync = qfalse;
	if ( !fsh[f].handleFiles.file.o ) {
		return 0;
	}
	return f;
}

/* ---- FS_FilenameCompare  0x00429820 ---- VERIFIED */
qboolean FS_FilenameCompare( const char *s1, const char *s2 ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( c1 >= 'a' && c1 <= 'z' ) {
			c1 -= ( 'a' - 'A' );
		}
		if ( c2 >= 'a' && c2 <= 'z' ) {
			c2 -= ( 'a' - 'A' );
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}

		if ( c1 != c2 ) {
			return -1;
		}
	} while ( c1 );

	return 0;
}

/* ---- FS_FileCompare  0x00429880 ---- VERIFIED */
qboolean FS_FileCompare( const char *s1, const char *s2 ) {
	FILE *f1, *f2;
	int len1, len2, pos, i;
	byte *buf1, *buf2;

	f1 = fopen( s2, "rb" );
	if ( !f1 ) {
		Com_Error( ERR_FATAL, "\x15" "FS_FileCompare: %s does not exist\n", s2 );
	}
	f2 = fopen( s1, "rb" );
	if ( !f2 ) {
		fclose( f1 );
		return qfalse;
	}

	pos = ftell( f1 );
	fseek( f1, 0, SEEK_END );
	len1 = ftell( f1 );
	fseek( f1, pos, SEEK_SET );

	pos = ftell( f2 );
	fseek( f2, 0, SEEK_END );
	len2 = ftell( f2 );
	fseek( f2, pos, SEEK_SET );

	if ( len1 != len2 ) {
		fclose( f1 );
		fclose( f2 );
		return qfalse;
	}

	buf1 = Z_MallocInternal( len1 );
	if ( fread( buf1, 1, len1, f1 ) != (size_t)len1 ) {
		Com_Error( ERR_FATAL, "\x15" "Short read in FS_FileCompare()\n" );
	}
	fclose( f1 );

	buf2 = Z_MallocInternal( len2 );
	if ( fread( buf2, 1, len2, f2 ) != (size_t)len2 ) {
		Com_Error( ERR_FATAL, "\x15" "Short read in FS_FileCompare()\n" );
	}
	fclose( f2 );

	for ( i = 0 ; i < len1 ; i++ ) {
		if ( buf1[i] != buf2[i] ) {
			free( buf1 );
			free( buf2 );
			return qfalse;
		}
	}

	free( buf1 );
	free( buf2 );
	return qtrue;
}

/* ---- FS_ShiftedStrStr  0x004299F0 ---- VERIFIED */
char *FS_ShiftedStrStr( const char *string, const char *substring, int shift ) {
	char buf[MAX_OSPATH];
	int i;

	for ( i = 0 ; substring[i] ; i++ ) {
		buf[i] = substring[i] + shift;
	}
	buf[i] = '\0';
	return strstr( string, buf );
}

/* ---- FS_FOpenFileRead_Internal  0x00429A70 ---- */
int FS_FOpenFileRead_Internal( const char *qpath, fileHandle_t *file, qboolean uniqueFILE, int filter_flag ) {
	extern cvar_t *cl_language;
	extern int com_fullyInitialized;
	searchpath_t *search;
	pack_t *pak;
	fileInPack_t *pakFile;
	directory_t *dir;
	long hash;
	char netpath[MAX_OSPATH];
	int len;

	hash = 0;

	if ( !file ) {
		return -1;
	}

	while ( *qpath == '/' || *qpath == '\\' ) {
		qpath++;
	}

	if ( strstr( qpath, ".." ) || strstr( qpath, "::" )
		 || ( com_fullyInitialized && strstr( qpath, "codkey" ) ) ) {
		*file = 0;
		return -1;
	}

	*file = FS_HandleForFileStreamed( filter_flag );
	fsh[*file].handleFiles.unique = uniqueFILE;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !FS_UseSearchPath( search ) ) {
			continue;
		}

		if ( search->pack ) {
			pak = search->pack;
			hash = FS_HashFileName( qpath, pak->hashSize );

			if ( pak->hashTable[hash] && FS_PakIsPure( pak ) ) {
				pakFile = pak->hashTable[hash];
				do {
					if ( !FS_FilenameCompare( pakFile->name, qpath ) ) {
						len = strlen( qpath );

						if ( !pak->referenced
							 && Q_stricmp( ".shader", &qpath[len - 7] )
							 && Q_stricmp( ".txt", &qpath[len - 4] )
							 && Q_stricmp( ".cfg", &qpath[len - 4] )
							 && Q_stricmp( ".config", &qpath[len - 7] )
							 && !strstr( qpath, "levelshots" )
							 && Q_stricmp( ".bot", &qpath[len - 4] )
							 && Q_stricmp( ".arena", &qpath[len - 6] )
							 && Q_stricmp( ".menu", &qpath[len - 5] ) ) {
							pak->referenced |= 0x01;
						}

						if ( !( pak->referenced & 0x02000000 )
							 && FS_ShiftedStrStr( qpath, "wgmgskesve~><4jrr", -6 ) ) {
							pak->referenced |= 0x02000000;
						}
						if ( !( pak->referenced & 0x00020000 )
							 && FS_ShiftedStrStr( qpath, "eicogaoraz:80fnn", -2 ) ) {
							pak->referenced |= 0x00020000;
						}
						if ( !( pak->referenced & 0x00000200 )
							 && FS_ShiftedStrStr( qpath, "zndrud}=;3iqq", -5 ) ) {
							pak->referenced |= 0x00000200;
						}

						if ( uniqueFILE ) {
							fsh[*file].handleFiles.file.z = Unz_ReOpen( pak->pakFilename, pak->handle );
							if ( !fsh[*file].handleFiles.file.z ) {
								if ( !filter_flag ) {
									Com_Error( ERR_FATAL, "\x15" "Couldn't reopen %s", pak->pakFilename );
								}
								FS_FCloseFile( *file );
								*file = 0;
								return -1;
							}
						} else {
							fsh[*file].handleFiles.file.z = pak->handle;
						}

						Q_strncpyz( fsh[*file].name, qpath, sizeof( fsh[*file].name ) );
						fsh[*file].zipFile = qtrue;

						Unz_SetCurrentFileInfoPosition( fsh[*file].handleFiles.file.z, pakFile->zipPosition );
						Unz_OpenCurrentFile( fsh[*file].handleFiles.file.z );
						fsh[*file].zipFilePos = pakFile->zipPosition;

						if ( fs_debug->integer && !filter_flag ) {
							Com_Printf( "FS_FOpenFileRead: %s (found in '%s')\n", qpath, pak->pakFilename );
						}
						return Unz_GetCurrentFileUncompressedSize( fsh[*file].handleFiles.file.z );
					}
					pakFile = pakFile->next;
				} while ( pakFile );
			}
		} else if ( search->dir ) {
			dir = search->dir;
			len = strlen( qpath );

			if ( ( !fs_ignoreLocalized->integer && !fs_numServerPaks )
				 || !Q_stricmp( ".cfg", &qpath[len - 4] )
				 || !Q_stricmp( ".menu", &qpath[len - 5] )
				 || !Q_stricmp( ".svg", &qpath[len - 4] )
				 || !Q_stricmp( ".game", &qpath[len - 5] )
				 || !Q_stricmp( ".dm_PROTOCOL_VERSION", &qpath[len - 20] )
				 || !Q_stricmp( ".dat", &qpath[len - 4] ) ) {
				FS_BuildOSPath3( dir->path, dir->gamedir, qpath, netpath, filter_flag );
				fsh[*file].handleFiles.file.o = fopen( netpath, "rb" );
				if ( fsh[*file].handleFiles.file.o ) {
					Q_strncpyz( fsh[*file].name, qpath, sizeof( fsh[*file].name ) );
					fsh[*file].zipFile = qfalse;
					if ( fs_debug->integer && !filter_flag ) {
						Com_Printf( "FS_FOpenFileRead: %s (found in '%s/%s')\n",
									qpath, dir->path, dir->gamedir );
					}
					return FS_filelength( *file );
				}
			}
		}
	}

	if ( fs_debug->integer && !filter_flag ) {
		Com_Printf( "Can't find %s\n", qpath );
	}
	*file = 0;
	return -1;
}

/* ---- FS_FOpenFileReadStream  0x0042A200 ---- */
int FS_FOpenFileReadStream( const char *qpath, fileHandle_t *file ) {
	return FS_FOpenFileRead_Internal( qpath, file, qfalse, 1 );
}

/* ---- FS_FOpenFileRead  0x0042A210 ---- */
int FS_FOpenFileRead( const char *qpath, fileHandle_t *file, qboolean uniqueFILE ) {
	extern int fs_loadingMode;
	fs_loadingMode = 1;
	return FS_FOpenFileRead_Internal( qpath, file, uniqueFILE, 0 );
}

/* ---- FS_TouchFile  0x0042A230 ---- VERIFIED */
void FS_TouchFile( const char *qpath ) {
	extern int fs_loadingMode;
	fileHandle_t f;

	fs_loadingMode = 1;
	FS_FOpenFileRead_Internal( qpath, &f, qfalse, 0 );
	if ( f ) {
		FS_FCloseFile( f );
	}
}

/* ---- FS_ShortOSFilePath  0x0042A270 ---- VERIFIED */
char *FS_ShortOSFilePath( const char *fullpath ) {
	extern cvar_t *cl_language;
	searchpath_t *search;
	directory_t *dir;
	FILE *f;
	char netpath[MAX_OSPATH];

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !FS_UseSearchPath( search ) ) {
			continue;
		}
		dir = search->dir;
		if ( dir ) {
			FS_BuildOSPath3( dir->path, dir->gamedir, fullpath, netpath, 0 );
			f = fopen( netpath, "rb" );
			if ( f ) {
				fclose( f );
				return va( "%s/%s", dir->gamedir, fullpath );
			}
		}
	}
	return NULL;
}

/* ---- FS_Delete  0x0042A330 ---- VERIFIED */
int FS_Delete( const char *filename ) {
	char osPath[MAX_OSPATH];
	char c;

	if ( !filename || !filename[0] ) {
		return 0;
	}

	if ( strncmp( filename, "save", 4 ) ) {
		return 0;
	}
	c = filename[4];
	if ( c != '/' && c != '\\' ) {
		return 0;
	}

	FS_BuildOSPath_Internal( fs_homepath->string, filename, osPath, 0 );
	if ( remove( osPath ) != -1 ) {
		return 1;
	}
	return 0;
}

/* ---- FS_MakeReadOnly  0x0042A3F0 ---- VERIFIED */
qboolean FS_MakeReadOnly( const char *filename, qboolean readOnly ) {
	char osPath[MAX_OSPATH];
	DWORD attr, newAttr;

	FS_BuildOSPath_Internal( fs_homepath->string, filename, osPath, 0 );
	attr = GetFileAttributesA( osPath );

	if ( readOnly ) {
		newAttr = attr | FILE_ATTRIBUTE_READONLY;
	} else {
		newAttr = attr & ~FILE_ATTRIBUTE_READONLY;
	}

	if ( newAttr == attr ) {
		return qfalse;
	}
	SetFileAttributesA( osPath, newAttr );
	return qtrue;
}

/* ---- FS_Read  0x0042A490 ---- VERIFIED */
int FS_Read( void *buffer, int len, fileHandle_t f ) {
	int block, remaining;
	int read;
	byte *buf;
	int tries;

	if ( !f ) {
		return 0;
	}

	if ( fsh[f].zipFile ) {
		return Unz_ReadCurrentFile( fsh[f].handleFiles.file.z, buffer, len );
	}

	buf = (byte *)buffer;
	remaining = len;
	tries = 0;
	if ( !len ) {
		return len;
	}

	while ( 1 ) {
		block = remaining;
		read = fread( buf, 1, block, fsh[f].handleFiles.file.o );
		if ( read == 0 ) {
			if ( tries ) {
				return len - remaining;
			}
			tries = 1;
		} else if ( read == -1 ) {
			if ( f < 51 || f >= MAX_FILE_HANDLES ) {
				Com_Error( ERR_FATAL, "\x15" "FS_Read: -1 bytes read" );
			}
			return -1;
		}

		remaining -= read;
		buf += read;
		if ( !remaining ) {
			return len;
		}
	}
}

/* ---- FS_Write  0x0042A560 ---- VERIFIED */
int FS_Write( const void *buffer, int len, fileHandle_t f ) {
	int block, remaining;
	int written;
	byte *buf;
	int tries;
	FILE *file;

	if ( !f ) {
		return 0;
	}

	file = fsh[f].handleFiles.file.o;
	buf = (byte *)buffer;
	remaining = len;
	tries = 0;
	while ( remaining ) {
		block = remaining;
		written = fwrite( buf, 1, block, file );
		if ( written == 0 ) {
			if ( tries ) {
				Com_Printf( "FS_Write: 0 bytes written\n" );
				return 0;
			}
			tries = 1;
		} else if ( written == -1 ) {
			Com_Printf( "FS_Write: -1 bytes written\n" );
			return 0;
		}
		remaining -= written;
		buf += written;
	}

	if ( fsh[f].handleSync ) {
		fflush( file );
	}
	return len;
}

/* ---- FS_Printf  0x0042A610 ---- VERIFIED */
void QDECL FS_Printf( fileHandle_t f, const char *fmt, ... ) {
	va_list argptr;
	char msg[4096];

	va_start( argptr, fmt );
	vsprintf( msg, fmt, argptr );
	va_end( argptr );

	FS_Write( msg, strlen( msg ), f );
}

/* FS_ZipDecompressedPos  0x004A95C0 */
static int FS_ZipDecompressedPos( unzFile z ) {
	unsigned long *s;
	unsigned long readInfo;

	if ( !z ) {
		return -102;
	}
	s = (unsigned long *) z;
	readInfo = s[31];
	if ( !readInfo ) {
		return -102;
	}
	return (int) *(unsigned long *)( readInfo + 24 );
}

/* ---- FS_Seek  0x0042A690 ---- VERIFIED */
int FS_Seek( fileHandle_t f, int offset, int origin ) {
	byte buffer[65536];
	int remaining, read;
	int pos;
	int fseek_origin;

	if ( fsh[f].streamed ) {
		fsh[f].streamed = qfalse;
		FS_Seek( f, offset, origin );
		fsh[f].streamed = qtrue;
	}

	if ( fsh[f].zipFile == qtrue ) {
		if ( offset == 0 && origin == FS_SEEK_SET ) {
			Unz_SetCurrentFileInfoPosition( fsh[f].handleFiles.file.z, fsh[f].zipFilePos );
			return Unz_OpenCurrentFile( fsh[f].handleFiles.file.z );
		}
		if ( offset == 0 && origin == FS_SEEK_CUR ) {
			return 0;
		}

		pos = FS_ZipDecompressedPos( fsh[f].handleFiles.file.z );

		switch ( origin ) {
		case FS_SEEK_CUR:
			if ( offset < 0 ) {
				Unz_SetCurrentFileInfoPosition( fsh[f].handleFiles.file.z, fsh[f].zipFilePos );
				Unz_OpenCurrentFile( fsh[f].handleFiles.file.z );
				remaining = offset + pos;
			} else {
				remaining = offset;
			}
			break;
		case FS_SEEK_END:
			if ( offset + FS_filelength( f ) >= pos ) {
				remaining = offset - pos + FS_filelength( f );
			} else {
				Unz_SetCurrentFileInfoPosition( fsh[f].handleFiles.file.z, fsh[f].zipFilePos );
				Unz_OpenCurrentFile( fsh[f].handleFiles.file.z );
				remaining = offset + FS_filelength( f );
			}
			break;
		case FS_SEEK_SET:
			if ( offset >= pos ) {
				remaining = offset - pos;
			} else {
				Unz_SetCurrentFileInfoPosition( fsh[f].handleFiles.file.z, fsh[f].zipFilePos );
				Unz_OpenCurrentFile( fsh[f].handleFiles.file.z );
				remaining = offset;
			}
			break;
		default:
			return -1;
		}

		while ( remaining ) {
			if ( remaining >= (int)sizeof( buffer ) ) {
				read = FS_Read( buffer, sizeof( buffer ), f );
				remaining -= sizeof( buffer );
			} else {
				read = FS_Read( buffer, remaining, f );
				remaining = 0;
			}
			if ( !read ) {
				return -1;
			}
		}
		return 0;
	}

	switch ( origin ) {
	case FS_SEEK_CUR:
		fseek_origin = SEEK_CUR;
		break;
	case FS_SEEK_END:
		fseek_origin = SEEK_END;
		break;
	case FS_SEEK_SET:
		fseek_origin = SEEK_SET;
		break;
	default:
		return 0;
	}
	return fseek( fsh[f].handleFiles.file.o, offset, fseek_origin );
}

/* ---- FS_ReadFile  0x0042A8B0 ---- VERIFIED */
int FS_ReadFile( const char *qpath, void **buffer ) {
	extern int fs_loadingMode;
	extern cvar_t *com_journal;
	extern fileHandle_t com_journalFile;
	fileHandle_t h;
	byte *buf;
	int len;
	qboolean isConfig;

	if ( !qpath || !qpath[0] ) {
		Com_Error( ERR_FATAL, "\x15" "FS_ReadFile with empty name\n" );
	}

	buf = NULL;

	isConfig = ( strstr( qpath, ".cfg" ) != NULL );

	if ( isConfig && com_journal && com_journal->integer == 2 ) {
		Com_DPrintf( "Loading %s from journal file.\n", qpath );
		if ( FS_Read( &len, sizeof( len ), com_journalFile ) != sizeof( len ) ) {
			if ( buffer ) {
				*buffer = NULL;
			}
			return -1;
		}
		if ( !len ) {
			if ( buffer ) {
				*buffer = NULL;
				return -1;
			}
			return 1;
		}
		if ( buffer ) {
			buf = Hunk_AllocateTempMemoryInternal( len + 1 );
			*buffer = buf;
			if ( FS_Read( buf, len, com_journalFile ) != len ) {
				Com_Error( ERR_FATAL, "EXE_ERR_JOURNAL_FILE_READ" );
			}
			fs_loadStack++;
			buf[len] = 0;
		}
		return len;
	}

	fs_loadingMode = 1;
	len = FS_FOpenFileRead_Internal( qpath, &h, qfalse, 0 );
	if ( !h ) {
		if ( buffer ) {
			*buffer = NULL;
		}
		if ( isConfig && com_journal && com_journal->integer == 1 ) {
			Com_DPrintf( "Writing zero for %s to journal file.\n", qpath );
			len = 0;
			FS_Write( &len, sizeof( len ), com_journalFile );
			fflush( fsh[com_journalFile].handleFiles.file.o );
		}
		return -1;
	}

	if ( !buffer ) {
		if ( isConfig && com_journal && com_journal->integer == 1 ) {
			Com_DPrintf( "Writing len for %s to journal file.\n", qpath );
			FS_Write( &len, sizeof( len ), com_journalFile );
			FS_Flush( com_journalFile );
		}
		FS_FCloseFile( h );
		return len;
	}

	fs_loadStack++;
	buf = Hunk_AllocateTempMemoryInternal( len + 1 );
	*buffer = buf;
	FS_Read( buf, len, h );
	buf[len] = 0;
	FS_FCloseFile( h );

	if ( isConfig && com_journal && com_journal->integer == 1 ) {
		Com_DPrintf( "Writing %s to journal file.\n", qpath );
		FS_Write( &len, sizeof( len ), com_journalFile );
		FS_Write( buf, len, com_journalFile );
		FS_Flush( com_journalFile );
	}
	return len;
}

/* ---- FS_ResetFiles  0x0042AB40 ---- VERIFIED */
void FS_ResetFiles( void ) {
	fs_loadStack = 0;
}

/* ---- FS_FreeFile  0x0042AB50 ---- VERIFIED */
void FS_FreeFile( void *buffer ) {
	extern void *s_hunkData;
	extern void Hunk_FreeTempMemoryInternal( void *buf );

	if ( !buffer ) {
		Com_Error( ERR_FATAL, "\x15" "FS_FreeFile( NULL )" );
	}
	fs_loadStack--;

	if ( s_hunkData ) {
		Hunk_FreeTempMemoryInternal( buffer );
	} else {
		free( buffer );
	}
}

/* ---- FS_WriteFile  0x0042ABC0 ---- VERIFIED */
void FS_WriteFile( const char *qpath, const void *buffer, int size ) {
	fileHandle_t f;

	if ( !qpath || !buffer ) {
		Com_Error( ERR_FATAL, "\x15" "FS_WriteFile: NULL parameter" );
	}

	f = FS_FOpenFileWrite( qpath );
	if ( !f ) {
		Com_Printf( "Failed to open %s\n", qpath );
		return;
	}

	FS_Write( buffer, size, f );
	FS_FCloseFile( f );
}

/* ---- FS_LoadZipFile  0x0042AC20 ---- */
pack_t *FS_LoadZipFile( char *zipfile, const char *basename ) {
	pack_t *pack;
	unzFile uf;
	int numFiles;
	int i, len;
	int hashSize;
	fileInPack_t *buildBuffer;
	fileInPack_t *curFile;
	char *namePtr;
	char filename[MAX_ZPATH];
	int *fs_headerLongs;
	int headerLen;
	long hash;

	uf = Unz_Open( zipfile );
	if ( !uf ) {
		return NULL;
	}
	Unz_GetGlobalNumFiles( uf, &numFiles );

	fs_packFiles += numFiles;

	Unz_GoToFirstFile( uf );
	len = 0;
	for ( i = 0 ; i < numFiles ; i++ ) {
		if ( Unz_GetCurrentFileInfo( uf, NULL, filename, sizeof( filename ), NULL, 0, NULL, 0 ) != 0 ) {
			break;
		}
		len += strlen( filename ) + 1;
		Unz_GoToNextFile( uf );
	}

	buildBuffer = malloc( ( numFiles * sizeof( fileInPack_t ) ) + len );
	if ( !buildBuffer ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( buildBuffer, 0, ( numFiles * sizeof( fileInPack_t ) ) + len );
	namePtr = ( (char *)buildBuffer ) + numFiles * sizeof( fileInPack_t );

	fs_headerLongs = malloc( numFiles * sizeof( int ) );
	if ( !fs_headerLongs ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( fs_headerLongs, 0, numFiles * sizeof( int ) );

	for ( hashSize = 1 ; hashSize <= MAX_FILEHASH_SIZE ; hashSize <<= 1 ) {
		if ( hashSize > numFiles ) {
			break;
		}
	}

	pack = malloc( sizeof( pack_t ) + hashSize * sizeof( fileInPack_t * ) );
	if ( !pack ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( pack, 0, sizeof( pack_t ) + hashSize * sizeof( fileInPack_t * ) );
	pack->hashTable = (fileInPack_t **)( pack + 1 );
	pack->hashSize = hashSize;
	for ( i = 0 ; i < hashSize ; i++ ) {
		pack->hashTable[i] = NULL;
	}

	Q_strncpyz( pack->pakFilename, zipfile, sizeof( pack->pakFilename ) );
	Q_strncpyz( pack->pakBasename, basename, sizeof( pack->pakBasename ) );

	if ( strlen( pack->pakBasename ) > 4
		 && !Q_stricmp( &pack->pakBasename[strlen( pack->pakBasename ) - 4], ".pk3" ) ) {
		pack->pakBasename[strlen( pack->pakBasename ) - 4] = 0;
	}

	pack->handle = uf;
	pack->numfiles = numFiles;
	Unz_GoToFirstFile( uf );

	headerLen = 0;
	curFile = buildBuffer;
	for ( i = 0 ; i < numFiles ; i++ ) {
		unz_file_info info;

		if ( Unz_GetCurrentFileInfo( uf, &info, filename, sizeof( filename ), NULL, 0, NULL, 0 ) != 0 ) {
			break;
		}

		if ( info.uncompressed_size ) {
			fs_headerLongs[headerLen++] = (int) info.crc;
		}

		curFile->zipPosition = Unz_GetCurrentFileInfoPosition( uf );

		{
			char *c;
			for ( c = filename ; *c ; c++ ) {
				*c = tolower( *c );
			}
		}

		hash = FS_HashFileName( filename, pack->hashSize );
		curFile->name = namePtr;
		strcpy( curFile->name, filename );
		namePtr += strlen( filename ) + 1;

		curFile->basename = curFile->name;
		{
			int k;
			for ( k = strlen( curFile->name ) - 1 ; k >= 0 ; k-- ) {
				if ( curFile->name[k] == '/' || curFile->name[k] == '\\' ) {
					curFile->basename = &curFile->name[k + 1];
					break;
				}
			}
		}

		curFile->next = pack->hashTable[hash];
		pack->hashTable[hash] = curFile;

		Unz_GoToNextFile( uf );
		curFile++;
	}

	pack->checksum = Com_BlockChecksum( fs_headerLongs, 4 * headerLen );
	pack->pure_checksum = Com_BlockChecksumKey( fs_headerLongs, 4 * headerLen, fs_checksumFeed );
	free( fs_headerLongs );

	pack->buildBuffer = buildBuffer;
	return pack;
}

/* ---- FS_ReturnPath  0x0042B1A0 ---- VERIFIED */
int FS_ReturnPath( const char *zname, char *zpath, int *depth ) {
	int len, at, newdep;

	newdep = 0;
	zpath[0] = 0;
	len = 0;
	at = 0;

	while ( zname[at] != 0 ) {
		if ( zname[at] == '/' || zname[at] == '\\' ) {
			len = at;
			newdep++;
		}
		at++;
	}
	strcpy( zpath, zname );
	zpath[len] = 0;
	if ( len + 1 == at ) {
		newdep--;
	}
	*depth = newdep;

	return len;
}

/* ---- FS_AddFileToList  0x0042B200 ---- VERIFIED */
static int FS_AddFileToList( char *name, char *list[MAX_FOUND_FILES], int nfiles ) {
	int i;

	if ( nfiles == MAX_FOUND_FILES - 1 ) {
		return nfiles;
	}
	for ( i = 0 ; i < nfiles ; i++ ) {
		if ( !name || !list[i] ) {
			continue;           /* retail skips the compare, 0x0042B220 */
		}
		if ( !Q_stricmp( name, list[i] ) ) {
			return nfiles;
		}
	}
	list[nfiles] = malloc( strlen( name ) + 1 );
	if ( !list[nfiles] ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( list[nfiles], 0, strlen( name ) + 1 );
	strcpy( list[nfiles], name );
	nfiles++;

	return nfiles;
}

/* ---- FS_ListFilteredFiles  0x0042B2A0 ---- VERIFIED */
char **FS_ListFilteredFiles( const char *path, const char *extension, char *filter, int *numfiles ) {
	extern cvar_t *cl_language;
	int nfiles;
	char        **listCopy;
	char        *list[MAX_FOUND_FILES];
	searchpath_t *search;
	int i;
	int pathLength;
	int extensionLength;
	int length, pathDepth, temp;
	qboolean wantSubs;
	pack_t      *pak;
	fileInPack_t *buildBuffer;
	char zpath[MAX_ZPATH];
	char dirName[64];

	if ( !path ) {
		*numfiles = 0;
		return NULL;
	}
	if ( !extension ) {
		extension = "";
	}
	if ( *path == '/' || *path == '\\' ) {
		path++;
	}

	wantSubs = (qboolean) ( Q_stricmpn( extension, "/", 99999 ) == 0 );

	pathLength = strlen( path );
	if ( pathLength > 0 && ( path[pathLength - 1] == '\\' || path[pathLength - 1] == '/' ) ) {
		pathLength--;
	}
	extensionLength = strlen( extension );
	nfiles = 0;
	FS_ReturnPath( path, zpath, &pathDepth );
	if ( *path ) {
		pathDepth++;
	}

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !FS_UseSearchPath( search ) ) {
			continue;
		}

		if ( search->pack ) {
			pak = search->pack;
			if ( fs_numServerPaks && !FS_PakIsPure( pak ) ) {
				continue;
			}
			buildBuffer = pak->buildBuffer;
			for ( i = 0 ; i < pak->numfiles ; i++ ) {
				char    *name;
				int zpathLen, depth;

				name = buildBuffer[i].name;

				if ( filter ) {
					if ( !Com_FilterPath( filter, name, qfalse ) ) {
						continue;
					}
					nfiles = FS_AddFileToList( name, list, nfiles );
				} else {
					zpathLen = FS_ReturnPath( name, zpath, &depth );

					if ( depth != pathDepth || pathLength > zpathLen ) {
						continue;
					}
					if ( pathLength && name[pathLength] != '/' ) {
						continue;
					}
					if ( Q_stricmpn( name, path, pathLength ) ) {
						continue;
					}

					length = strlen( name );
					if ( length < extensionLength ) {
						continue;
					}
					if ( Q_stricmp( extension, &name[length - extensionLength] ) ) {
						continue;
					}
					temp = pathLength;
					if ( pathLength ) {
						temp++;
					}
					if ( wantSubs ) {
						Q_strncpyz( dirName, &name[temp], sizeof( dirName ) );
						if ( dirName[0] ) {
							dirName[strlen( dirName ) - 1] = 0;
						}
						nfiles = FS_AddFileToList( dirName, list, nfiles );
					} else {
						nfiles = FS_AddFileToList( &name[temp], list, nfiles );
					}
				}
			}
		} else if ( search->dir ) {
			char    *netpath;
			int numSysFiles;
			char    **sysFiles;
			char    *name;

			if ( ( ( fs_restrict && fs_restrict->integer ) || fs_numServerPaks )
				 && Q_stricmpn( extension, "svg", 99999 ) ) {
				continue;
			}

			netpath = FS_BuildOSPath( search->dir->path, search->dir->gamedir, path );
			sysFiles = Sys_ListFiles( netpath, extension, filter, &numSysFiles, wantSubs );
			for ( i = 0 ; i < numSysFiles ; i++ ) {
				name = sysFiles[i];
				nfiles = FS_AddFileToList( name, list, nfiles );
			}
			Sys_FreeFileList( sysFiles );
		}
	}

	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = Z_MallocInternal( ( nfiles + 1 ) * sizeof( *listCopy ) );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

/* ---- FS_ListFiles  0x0042B6F0 ---- VERIFIED */
char **FS_ListFiles( const char *path, const char *extension, int *numfiles ) {
	return FS_ListFilteredFiles( path, extension, NULL, numfiles );
}

/* ---- FS_FreeFileList  0x0042B710 ---- VERIFIED */
void FS_FreeFileList( char **list ) {
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		free( list[i] );
	}
	free( list );
}

/* ---- FS_GetFileList  0x0042B750 ---- VERIFIED */
int FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize ) {
	int nFiles, i, nTotal, nLen;
	char        **pFiles;

	*listbuf = 0;
	nFiles = 0;
	nTotal = 0;

	if ( path && Q_stricmp( path, "$modlist" ) == 0 ) {
		return FS_GetModList( listbuf, bufsize );
	}

	pFiles = FS_ListFilteredFiles( path, extension, NULL, &nFiles );

	for ( i = 0 ; i < nFiles ; i++ ) {
		nLen = strlen( pFiles[i] ) + 1;
		if ( nTotal + nLen + 1 < bufsize ) {
			strcpy( listbuf, pFiles[i] );
			listbuf += nLen;
			nTotal += nLen;
		} else {
			nFiles = i;
			break;
		}
	}

	FS_FreeFileList( pFiles );

	return nFiles;
}

/* ---- FS_ConvertPath  0x0042B840 ---- VERIFIED */
void FS_ConvertPath( char *s ) {
	while ( *s ) {
		if ( *s == '\\' || *s == ':' ) {
			*s = '/';
		}
		s++;
	}
}

/* ---- FS_PathCmp  0x0042B860 ---- VERIFIED */
int FS_PathCmp( const char *s1, const char *s2 ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( c1 >= 'a' && c1 <= 'z' ) {
			c1 -= ( 'a' - 'A' );
		}
		if ( c2 >= 'a' && c2 <= 'z' ) {
			c2 -= ( 'a' - 'A' );
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}

		if ( c1 < c2 ) {
			return -1;
		}
		if ( c1 > c2 ) {
			return 1;
		}
	} while ( c1 );

	return 0;
}

/* ---- FS_SortFileList  0x0042B8C0 ---- VERIFIED */
void FS_SortFileList( char **filelist, int numfiles ) {
	char **sortedlist;
	int i, j;

	sortedlist = Z_MallocInternal( ( numfiles + 1 ) * sizeof( *sortedlist ) );
	sortedlist[0] = NULL;

	for ( i = 0 ; i < numfiles ; i++ ) {
		for ( j = 0 ; j < i ; j++ ) {
			if ( FS_PathCmp( filelist[i], sortedlist[j] ) < 0 ) {
				break;
			}
		}
		{
			int k;
			for ( k = i ; k > j ; k-- ) {
				sortedlist[k] = sortedlist[k - 1];
			}
		}
		sortedlist[j] = filelist[i];
	}

	Com_Memcpy( filelist, sortedlist, numfiles * sizeof( *filelist ) );
	free( sortedlist );
}

#define FS_LANGUAGE_NAME( idx )  \
	( g_languages[ (unsigned int)( idx ) > NUM_LANGUAGES - 1 ? 0 : (unsigned int)( idx ) ] )

/* ---- FS_DisplayPath  0x0042B960 ---- VERIFIED */
void FS_DisplayPath( qboolean showAll ) {
	extern cvar_t *cl_language;
	extern const char *g_languages[];
	searchpath_t *s;
	int i;

	Com_Printf( "Current language: %s\n", FS_LANGUAGE_NAME( cl_language->integer ) );
	if ( fs_ignoreLocalized->integer ) {
		Com_Printf( "    localized assets are being ignored\n" );
	}

	Com_Printf( "Current search path:\n" );
	for ( s = fs_searchpaths ; s ; s = s->next ) {
		if ( showAll && !FS_UseSearchPath( s ) ) {
			continue;
		}

		if ( s->pack ) {
			Com_Printf( "%s (%i files)\n", s->pack->pakFilename, s->pack->numfiles );
			if ( s->localized ) {
				Com_Printf( "    localized assets pak file for %s\n", FS_LANGUAGE_NAME( s->language ) );
			}
			if ( fs_numServerPaks ) {
				if ( !FS_PakIsPure( s->pack ) ) {
					Com_Printf( "    not on the pure list\n" );
				} else {
					Com_Printf( "    on the pure list\n" );
				}
			}
		} else {
			Com_Printf( "%s/%s\n", s->dir->path, s->dir->gamedir );
			if ( s->localized ) {
				Com_Printf( "    localized assets game folder for %s\n", FS_LANGUAGE_NAME( s->language ) );
			}
		}
	}

	Com_Printf( "\nFile Handles:\n" );
	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		if ( fsh[i].handleFiles.file.o ) {
			Com_Printf( "handle %i: %s\n", i, fsh[i].name );
		}
	}
}

/* ---- FS_FullPath_f  0x0042BB20 ---- VERIFIED */
void FS_FullPath_f( void ) {
	FS_DisplayPath( qfalse );
}

/* ---- FS_Path_f  0x0042BB30 ---- VERIFIED */
void FS_Path_f( void ) {
	FS_DisplayPath( qtrue );
}

/* ---- PakFileLanguage  0x0042BB40 ---- VERIFIED */
char *PakFileLanguage( const char *pakName ) {
	static char buffers[CODUO_FS_LANGUAGE_NAME_BUFFER_COUNT][CODUO_FS_LANGUAGE_NAME_BUFFER_SIZE];
	static int index;
	char *out;
	int i;

	index ^= 1;
	out = buffers[index];

	if ( strlen( pakName ) >= 10 ) {
		Com_Memset( out, 0, CODUO_FS_LANGUAGE_NAME_BUFFER_SIZE );
		for ( i = 10 ; i < CODUO_FS_LANGUAGE_NAME_BUFFER_SIZE ; i++ ) {
			if ( !pakName[i] ) {
				break;
			}
			if ( !isalpha( pakName[i] ) ) {
				break;
			}
			out[i - 10] = pakName[i];
		}
	} else {
		out[0] = 0;
	}
	return out;
}

/* ---- paksort  0x0042BBE0 ---- VERIFIED */
int paksort( const void *a, const void *b ) {
	char        *aa, *bb;
	char        *langA, *langB;

	aa = *(char **)a;
	bb = *(char **)b;

	if ( Q_strncmp( aa, "          ", 10 ) ) {
		return FS_PathCmp( aa, bb );
	}
	if ( Q_strncmp( bb, "          ", 10 ) ) {
		return FS_PathCmp( aa, bb );
	}

	langA = PakFileLanguage( aa );
	langB = PakFileLanguage( bb );

	if ( langA && !Q_stricmp( langA, "english" ) ) {
		if ( langB && !Q_stricmp( langB, "english" ) ) {
			return FS_PathCmp( aa, bb );
		}
		return -1;
	}

	if ( langB && !Q_stricmp( langB, "english" ) ) {
		return 1;
	}

	return FS_PathCmp( aa, bb );
}

/* ---- FS_AddSearchPath  0x0042BCF0 ---- VERIFIED */
void FS_AddSearchPath( searchpath_t *sp ) {
	searchpath_t *s;

	if ( sp->localized && fs_searchpaths ) {
		for ( s = fs_searchpaths ; s->next ; s = s->next ) {
			if ( s->next->localized ) {
				break;
			}
		}
		sp->next = s->next;
		s->next = sp;
	} else {
		sp->next = fs_searchpaths;
		fs_searchpaths = sp;
	}
}

/* ---- FS_AddPakFilesForGameDirectory  0x0042BD30 ---- */
void FS_AddPakFilesForGameDirectory( const char *path, const char *dir ) {
	extern const char *g_languages[];
	char pakfile[MAX_OSPATH];
	char        **pakfiles;
	int numfiles;
	int i, j;
	searchpath_t *search;
	pack_t      *pak;

	FS_BuildOSPath_Internal( path, dir, pakfile, 0 );

	pakfiles = Sys_ListFiles( pakfile, ".pk3", NULL, &numfiles, qfalse );

	Com_Printf( "  scanning \"%s\" -> %i pk3\n", pakfile, numfiles );

	if ( numfiles > MAX_SEARCH_PATHS ) {
		Com_Printf( "WARNING: Exceeded max number of pak files in %s/%s (%i/%i)\n",
					path, dir, numfiles, MAX_SEARCH_PATHS );
		numfiles = MAX_SEARCH_PATHS;
	}

	qsort( pakfiles, numfiles, sizeof( char * ), paksort );

	for ( i = 0 ; i < numfiles ; i++ ) {
		int localized;
		int language;
		char        *lang;

		localized = 0;
		language = 0;

		if ( !Q_stricmpn( pakfiles[i], "localized_", 10 ) ) {
			localized = 1;
			lang = PakFileLanguage( pakfiles[i] );
			if ( lang[0] ) {
				for ( j = 0 ; j < NUM_LANGUAGES ; j++ ) {
					if ( g_languages[j] && !Q_stricmp( lang, g_languages[j] ) ) {
						language = j;
						break;
					}
				}
				if ( j == NUM_LANGUAGES ) {
					Com_Printf( "WARNING: Localized assets pak file %s/%s/%s has invalid name "
								"(bad language name specified). Proper naming convention is: "
								"localized_[language]_pak#.pk3\n", path, dir, pakfiles[i] );
				}
			} else {
				Com_Printf( "WARNING: Localized assets pak file %s/%s/%s has invalid name "
							"(no language specified). Proper naming convention is: "
							"localized_[language]_pak#.pk3\n", path, dir, pakfiles[i] );
			}
		}

		FS_BuildOSPath3( path, dir, pakfiles[i], pakfile, 0 );
		pak = FS_LoadZipFile( pakfile, pakfiles[i] );
		if ( !pak ) {
			continue;
		}
		Q_strncpyz( pak->pakGamename, dir, sizeof( pak->pakGamename ) );

		search = malloc( sizeof( searchpath_t ) );
		if ( !search ) {
			Sys_OutOfMemoryError();
		}
		Com_Memset( search, 0, sizeof( searchpath_t ) );
		search->pack = pak;
		search->localized = localized;
		search->language = language;
		FS_AddSearchPath( search );
	}

	Sys_FreeFileList( pakfiles );
}

/* ---- FS_AddGameDirectory  0x0042C250 ---- VERIFIED */
void FS_AddGameDirectory( const char *path, const char *gameName, qboolean localized, int language ) {
	searchpath_t *sp;
	searchpath_t *search;
	directory_t *d;
	char folder[64];
	char fs_path[MAX_OSPATH];
	int len;

	if ( localized ) {
		Com_sprintf( folder, sizeof( folder ), "%s_%s",
					 gameName, g_languages[( language > NUM_LANGUAGES - 1 ) ? 0 : language] );
	} else {
		Q_strncpyz( folder, gameName, sizeof( folder ) );
	}

	for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
		if ( sp->dir && !Q_stricmp( sp->dir->path, path ) && !Q_stricmp( sp->dir->gamedir, folder ) ) {
			if ( sp->localized != localized ) {
				Com_Printf( "WARNING: game folder %s/%s added as both localized & non-localized. "
							"Using folder as %s\n",
							path, folder, sp->localized ? "localized" : "non-localized" );
			}
			if ( sp->localized && sp->language != language ) {
				Com_Printf( "WARNING: game golder %s/%s re-added as localized folder with "
							"different language\n", path, folder );
			}
			return;
		}
	}

	if ( localized ) {
		FS_BuildOSPath3( path, folder, "", fs_path, 0 );
		len = strlen( fs_path );
		if ( len > 0 ) {
			fs_path[len - 1] = 0;
		}
		if ( !Sys_DirectoryHasContents_m( fs_path ) ) {
			return;
		}
	} else {
		Q_strncpyz( fs_gamedir, folder, sizeof( fs_gamedir ) );
	}

	search = malloc( sizeof( searchpath_t ) );
	if ( !search ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( search, 0, sizeof( searchpath_t ) );

	d = malloc( sizeof( directory_t ) );
	if ( !d ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( d, 0, sizeof( directory_t ) );
	Q_strncpyz( d->path, path, sizeof( d->path ) );
	Q_strncpyz( d->gamedir, folder, sizeof( d->gamedir ) );

	search->dir = d;
	search->localized = localized;
	search->language = language;

	FS_AddSearchPath( search );

	FS_AddPakFilesForGameDirectory( path, folder );
}

/* ---- FS_AddLocalizedGameDirectory  0x0042C530 ---- VERIFIED */
void FS_AddLocalizedGameDirectory( const char *path, const char *gameName ) {
	int i;

	for ( i = NUM_LANGUAGES - 1 ; i >= 0 ; i-- ) {
		FS_AddGameDirectory( path, gameName, qtrue, i );
	}
	FS_AddGameDirectory( path, gameName, qfalse, 0 );
}

/* ---- FS_AddNonPackFileDirectory_Internal  0x0042C560 ---- VERIFIED */
void FS_AddNonPackFileDirectory_Internal( const char *path, const char *gamedir,
										  const char *dir, const char *ext ) {
	(void)path;
	(void)gamedir;
	(void)dir;
	(void)ext;
}

/* ---- FS_AddNonPackFileDirectory  0x0042C8C0 ---- VERIFIED */
void FS_AddNonPackFileDirectory( const char *dir, const char *ext ) {
	searchpath_t *search;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( search->pack ) {
			continue;
		}
		FS_AddNonPackFileDirectory_Internal( search->dir->path, search->dir->gamedir,
											 dir, ext );
	}
}

/* ---- FS_GetDataForFile  0x0042C900 ---- VERIFIED */
fileData_t *FS_GetDataForFile( const char *dir, const char *name, const char *ext ) {

	char              filename[1024];   /* retail's frame: Buffer[1024] at ebp-0x404 */
	searchpath_t     *search;
	pack_t           *pak;
	fileInPack_t     *pakFile;
	fs_dirFileList_t *list;
	fs_dirFile_t     *entry;
	long              hash;

	sprintf( filename, "%s/%s%s", dir, name, ext );

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !FS_UseSearchPath( search ) ) {
			continue;
		}
		pak = search->pack;
		if ( !pak ) {
			continue;
		}

		hash = FS_HashFileName( filename, pak->hashSize );
		for ( pakFile = pak->hashTable[hash] ; pakFile ; pakFile = pakFile->next ) {
			if ( !FS_FilenameCompare( filename, pakFile->name ) ) {
				/* 0x0042CA53: lea eax, [esi+4] */
				return (fileData_t *)&pakFile->basename;
			}
		}
	}

	sprintf( filename, "%s%s", name, ext );

	for ( list = fs_savedDirFileLists ; list ; list = list->next ) {
		if ( Q_stricmp( list->dir, dir ) ) {
			continue;
		}

		hash = FS_HashFileName( filename, list->hashSize );
		for ( entry = list->hashTable[hash] ; entry ; entry = entry->next ) {
			if ( !FS_FilenameCompare( filename, entry->name ) ) {
				return (fileData_t *)entry;
			}
		}
	}

	return NULL;
}

/* ---- FS_ClearDataForFiles  0x0042CA60 ---- VERIFIED */
void FS_ClearDataForFiles( void *lowEnd, void *highEnd ) {
	searchpath_t     *search;
	pack_t           *pak;
	fileInPack_t     *pakFile;
	fs_dirFileList_t *list;
	fs_dirFile_t     *entry;
	fileData_t       *fd;
	int i;

	for ( search = fs_savedSearchpaths ; search ; search = search->next ) {
		pak = search->pack;
		if ( !pak ) {
			continue;
		}
		for ( i = 0 ; i < pak->hashSize ; i++ ) {
			for ( pakFile = pak->hashTable[i] ; pakFile ; pakFile = pakFile->next ) {
				if ( (unsigned char *)pakFile->payload < (unsigned char *)lowEnd ||
					 (unsigned char *)pakFile->payload >= (unsigned char *)highEnd ) {
					continue;
				}
				/* 0x0042CA53's shape: the callback sees fileInPack_t + 4 */
				fd = (fileData_t *)&pakFile->basename;
				if ( pakFile->clearCallback ) {
					pakFile->clearCallback( fd );
					pakFile->clearCallback = NULL;
				}
				pakFile->payload = NULL;
			}
		}
	}

	for ( list = fs_savedDirFileLists ; list ; list = list->next ) {
		for ( i = 0 ; i < list->hashSize ; i++ ) {
			for ( entry = list->hashTable[i] ; entry ; entry = entry->next ) {
				if ( (unsigned char *)entry->data < (unsigned char *)lowEnd ||
					 (unsigned char *)entry->data >= (unsigned char *)highEnd ) {
					continue;
				}
				if ( entry->clearCallback ) {
					entry->clearCallback( (fileData_t *)entry );
					entry->clearCallback = NULL;
				}
				entry->data = NULL;
			}
		}
	}
}

/* ---- FS_ShutdownSearchPaths  0x0042CB50 ---- VERIFIED */
static void FS_ShutdownSearchPaths( searchpath_t *paths ) {
	searchpath_t *p, *next;

	for ( p = paths ; p ; p = next ) {
		next = p->next;

		if ( p->pack ) {
			Unz_Close( p->pack->handle );
			free( p->pack->buildBuffer );
			free( p->pack );
		}
		if ( p->dir ) {
			free( p->dir );
		}
		free( p );
	}
}

/* ---- FS_ShutdownFileLists  0x0042CBD0 ---- VERIFIED */
static void FS_ShutdownFileLists( fs_dirFileList_t *lists ) {
	fs_dirFileList_t *n, *next;

	for ( n = lists ; n ; n = next ) {
		next = n->next;
		free( n->buildBuffer );
		free( n );
	}
}

/* ---- FS_ClearMemory  0x0042CC10 ---- VERIFIED */
void FS_ClearMemory( void ) {
	if ( fs_savedSearchpaths != fs_searchpaths ) {
		FS_ShutdownSearchPaths( fs_savedSearchpaths );
		fs_savedSearchpaths = fs_searchpaths;
	}
	if ( fs_savedDirFileLists != fs_dirFileLists ) {
		FS_ShutdownFileLists( fs_savedDirFileLists );
		fs_savedDirFileLists = fs_dirFileLists;
	}
}

/* ---- FS_ShutdownServerPakNames  0x0042CC50 ---- VERIFIED */
void FS_ShutdownServerPakNames( void ) {
	int i;

	for ( i = 0 ; i < fs_numServerPaks ; i++ ) {
		if ( fs_serverPakNames[i] ) {
			free( fs_serverPakNames[i] );
		}
		fs_serverPakNames[i] = NULL;
	}
	fs_numServerPaks = 0;
}

/* ---- FS_ShutdownServerReferencedPaks  0x0042CC90 ---- VERIFIED */
void FS_ShutdownServerReferencedPaks( void ) {
	int i;

	for ( i = 0 ; i < fs_numServerReferencedPaks ; i++ ) {
		if ( fs_serverReferencedPakNames[i] ) {
			free( fs_serverReferencedPakNames[i] );
		}
		fs_serverReferencedPakNames[i] = NULL;
	}
	fs_numServerReferencedPaks = 0;
}

/* ---- FS_Shutdown  0x0042CCD0 ---- VERIFIED */
void FS_Shutdown( qboolean closemfp ) {
	extern void MSS_StopSounds( int flags );
	extern int Clear__16CStringEdPackageFi( void *package, int bKeepFileInfo );
	extern int TheStringPackage;
	int i;

#ifndef DEDICATED
	MSS_StopSounds( 8 );
#endif
	Clear__16CStringEdPackageFi( &TheStringPackage, 0 );

	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		if ( fsh[i].fileSize ) {
			FS_FCloseFile( i );
		}
	}

	if ( closemfp ) {
		FS_ShutdownSearchPaths( fs_searchpaths );
		FS_ShutdownFileLists( fs_dirFileLists );
		fs_savedSearchpaths = NULL;
		fs_savedDirFileLists = NULL;
	} else {
		if ( fs_savedSearchpaths != fs_searchpaths ) {
			FS_ShutdownSearchPaths( fs_searchpaths );
		}
		if ( fs_savedDirFileLists != fs_dirFileLists ) {
			FS_ShutdownFileLists( fs_dirFileLists );
		}
	}

	fs_searchpaths  = NULL;
	fs_dirFileLists = NULL;
	fs_fakeChkSum   = 0;

	Cmd_RemoveCommand( "path" );
	Cmd_RemoveCommand( "fullpath" );
	Cmd_RemoveCommand( "dir" );
	Cmd_RemoveCommand( "fdir" );
	Cmd_RemoveCommand( "touchFile" );
}

/* ---- FS_Startup  0x0042CDB0 ---- */
void FS_Startup( const char *gameName ) {
	Com_Printf( "----- FS_Startup -----\n" );

	fs_debug = Cvar_Get( "fs_debug", "0", 0 );
	fs_cdpath = Cvar_Get( "fs_cdpath", Sys_DefaultCDPath(), CVAR_INIT );
	fs_basepath = Cvar_Get( "fs_basepath", Sys_DefaultBasePath(), CVAR_INIT );
	fs_basegame = Cvar_Get( "fs_basegame", "", CVAR_INIT );
	{
		const char *homePath = Sys_DefaultHomePath();
		if ( !homePath || !homePath[0] ) {
			homePath = fs_basepath->string;
		}
		fs_homepath = Cvar_Get( "fs_homepath", (char *) homePath, CVAR_INIT );
	}
	fs_gamedirvar = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	fs_restrict = Cvar_Get( "fs_restrict", "", CVAR_INIT );

	fs_ignoreLocalized = Cvar_Get( "fs_ignoreLozalized", "0", CVAR_CHEAT | CVAR_LATCH );

	/* 0x0042CDDA -- registered here, unused by the filesystem itself. */
	Cvar_Get( "fs_copyfiles", "0", CVAR_INIT );

	Com_Printf( "fs_basepath: \"%s\"\n", fs_basepath->string );
	Com_Printf( "fs_homepath: \"%s\"\n", fs_homepath->string );
	Com_Printf( "fs_cdpath:   \"%s\"\n", fs_cdpath->string );
	Com_Printf( "fs_game:     \"%s\"\n", fs_gamedirvar->string );

	if ( fs_cdpath->string[0] ) {
		FS_AddLocalizedGameDirectory( fs_cdpath->string, gameName );
	}
	if ( fs_basepath->string[0] ) {
		FS_AddLocalizedGameDirectory( fs_basepath->string, gameName );
	}
	if ( fs_basepath->string[0] && fs_homepath->string[0]
		 && Q_stricmp( fs_homepath->string, fs_basepath->string ) ) {
		FS_AddLocalizedGameDirectory( fs_homepath->string, gameName );
	}

	if ( fs_basegame->string[0] && gameName && !Q_stricmp( gameName, BASEGAME )
		 && Q_stricmp( fs_basegame->string, gameName ) ) {
		if ( fs_cdpath->string[0] ) {
			FS_AddLocalizedGameDirectory( fs_cdpath->string, fs_basegame->string );
		}
		if ( fs_basepath->string[0] ) {
			FS_AddLocalizedGameDirectory( fs_basepath->string, fs_basegame->string );
		}
		if ( fs_homepath->string[0] && fs_basepath->string[0]
			 && Q_stricmp( fs_homepath->string, fs_basepath->string ) ) {
			FS_AddLocalizedGameDirectory( fs_homepath->string, fs_basegame->string );
		}
	}

	if ( fs_gamedirvar->string[0] && gameName && !Q_stricmp( gameName, BASEGAME )
		 && Q_stricmp( fs_gamedirvar->string, gameName ) ) {
		if ( fs_cdpath->string[0] ) {
			FS_AddLocalizedGameDirectory( fs_cdpath->string, fs_gamedirvar->string );
		}
		if ( fs_basepath->string[0] ) {
			FS_AddLocalizedGameDirectory( fs_basepath->string, fs_gamedirvar->string );
		}
		if ( fs_homepath->string[0] && fs_basepath->string[0]
			 && Q_stricmp( fs_homepath->string, fs_basepath->string ) ) {
			FS_AddLocalizedGameDirectory( fs_homepath->string, fs_gamedirvar->string );
		}
	}

	FS_AddNonPackFileDirectory( "xanim", "" );
	FS_AddNonPackFileDirectory( "xmodel", "" );
	FS_AddNonPackFileDirectory( "xmodelparts", "" );
	FS_AddNonPackFileDirectory( "xmodelsurfs", "" );
	FS_AddNonPackFileDirectory( "weapons", "" );
	FS_AddNonPackFileDirectory( "animtrees", ".atr" );

	Com_ReadCDKey( BASEGAME );
	if ( fs_gamedirvar->string[0] ) {
		Com_AppendCDKey( fs_gamedirvar->string );
	}

	FS_AddCommands();

	FS_DisplayPath( qtrue );

	fs_gamedirvar->modified = qfalse;

	Com_Printf( "----------------------\n" );
	Com_Printf( "%d files in pk3 files\n", fs_packFiles );
}

/* ---- FS_ClearPakReferences  0x0042D170 ---- VERIFIED */
void FS_ClearPakReferences( int flags ) {
	searchpath_t *search;
	unsigned char *ref;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		ref = (unsigned char *)&search->pack->referenced;

		ref[2] = 0;             /* +0x312 */
		ref[1] = 0;             /* +0x311 */

		if ( !flags ) {
			ref[0] = 0;         /* +0x310 */
			ref[3] = 0;         /* +0x313 */
		}
	}
}

/* ---- FS_InitFilesystem  0x0042D1C0 ---- VERIFIED */
void FS_InitFilesystem( void ) {
	Com_StartupVariable( "fs_cdpath" );
	Com_StartupVariable( "fs_basepath" );
	Com_StartupVariable( "fs_homepath" );
	Com_StartupVariable( "fs_game" );
	Com_StartupVariable( "fs_copyfiles" );
	Com_StartupVariable( "fs_restrict" );
	Com_StartupVariable( "fs_usewolf" );
	Com_StartupVariable( "cl_language" );

	SEH_InitLanguage();

	FS_Startup( BASEGAME );

	Clear__16CStringEdPackageFi( &TheStringPackage, 0 );
	SEH_UpdateLanguageInfo();
	FS_SetRestrictions();

	if ( FS_ReadFile( "default_mp.cfg", NULL ) <= 0 ) {
		Com_Error( ERR_FATAL, "Couldn't load %s.  Make sure Call of Duty is run from "
							  "the correct folder.", "default_mp.cfg" );
	}

	Q_strncpyz( lastValidBase, fs_basepath->string, sizeof( lastValidBase ) );
	Q_strncpyz( lastValidGame, fs_gamedirvar->string, sizeof( lastValidGame ) );
}

/* ---- FS_Restart  0x0042D2B0 ---- VERIFIED */
void FS_Restart( int checksumFeed ) {
	FS_Shutdown( qfalse );

	fs_checksumFeed = checksumFeed;
	FS_ClearPakReferences( 0 );

	FS_Startup( BASEGAME );

	Clear__16CStringEdPackageFi( &TheStringPackage, 0 );
	SEH_UpdateLanguageInfo();
	FS_SetRestrictions();

	if ( FS_ReadFile( "default_mp.cfg", NULL ) <= 0 ) {
		if ( lastValidBase[0] ) {
			FS_PureServerSetLoadedPaks( "", "" );
			Cvar_Set2( "fs_basepath", lastValidBase, qtrue );
			Cvar_Set2( "fs_gamedirvar", lastValidGame, qtrue );
			lastValidBase[0] = 0;
			lastValidGame[0] = 0;
			Cvar_Set2( "fs_restrict", "0", qtrue );
			FS_Restart( checksumFeed );
			Com_Error( ERR_DROP, "Invalid game folder\n" );
			return;
		}
		Com_Error( ERR_FATAL, "Couldn't load %s.  Make sure Call of Duty is run from "
							  "the correct folder.", "default_mp.cfg" );
	}

	if ( ( !fs_gamedirvar->string || Q_stricmp( lastValidGame, fs_gamedirvar->string ) )
		 && !Com_SafeMode() ) {
		Cbuf_AddText( va( "exec %s\n", "config_mp.cfg" ) );
	}

	Q_strncpyz( lastValidBase, fs_basepath->string, sizeof( lastValidBase ) );
	Q_strncpyz( lastValidGame, fs_gamedirvar->string, sizeof( lastValidGame ) );
}

/* ---- FS_ConditionalRestart  0x0042D410 ---- VERIFIED */
qboolean FS_ConditionalRestart( int checksumFeed ) {
	if ( com_sv_running->integer ) {
		return qfalse;
	}
	if ( !fs_gamedirvar->modified && checksumFeed == fs_checksumFeed ) {
		return qfalse;
	}
	FS_Restart( checksumFeed );
	return qtrue;
}

/* ---- FS_FOpenFileByMode  0x0042D450 ---- VERIFIED */
int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	int r;
	qboolean sync;

	sync = qfalse;

	switch ( mode ) {
	case FS_READ:
		r = FS_FOpenFileRead( qpath, f, qtrue );
		break;
	case FS_WRITE:
		*f = FS_FOpenFileWrite( qpath );
		r = 0;
		if ( *f == 0 ) {
			r = -1;
		}
		break;
	case FS_APPEND_SYNC:
		sync = qtrue;
	case FS_APPEND:
		*f = FS_FOpenFileAppend( qpath );
		r = 0;
		if ( *f == 0 ) {
			r = -1;
		}
		break;
	default:
		Com_Error( ERR_FATAL, "\x15" "FSH_FOpenFile: bad mode" );
		return -1;
	}

	if ( !f ) {
		return r;
	}

	if ( *f ) {
		if ( fsh[*f].zipFile == qtrue ) {
			fsh[*f].baseOffset = unztell( fsh[*f].handleFiles.file.z );
		} else {
			fsh[*f].baseOffset = ftell( fsh[*f].handleFiles.file.o );
		}
		fsh[*f].fileSize = r;
		fsh[*f].streamed = qfalse;
	}

	fsh[*f].handleSync = sync;

	return r;
}

/* ---- FS_FTell  0x0042D580 ---- VERIFIED */
int FS_FTell( fileHandle_t f ) {
	int pos;

	if ( fsh[f].zipFile == qtrue ) {
		pos = unztell( fsh[f].handleFiles.file.z );
	} else {
		pos = ftell( fsh[f].handleFiles.file.o );
	}
	return pos;
}

/* ---- FS_Flush  0x0042D5C0 ---- VERIFIED */
void FS_Flush( fileHandle_t f ) {
	fflush( fsh[f].handleFiles.file.o );
}
