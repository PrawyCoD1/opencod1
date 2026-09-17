/*
 * qcommon/files.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/files.c
 *
 * Retail range 0x0043A730-0x0043C7.
 *
 * @fidelity: verified
 */

#include "qcommon.h"
#include "../universal/files_local.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---- FS_ExtractFromPakFile  0x0043ACA0 ---- VERIFIED */
int FS_ExtractFromPakFile( const char *osPath, int unused, const char *qpath ) {
	void        *fsData;
	int fsLen;
	FILE        *f;
	int diskLen;
	byte        *diskData;
	int i;
	qboolean different;
	fileHandle_t out;

	fsLen = FS_ReadFile( qpath, &fsData );
	if ( fsLen == -1 ) {
		return 0;
	}

	different = qtrue;

	f = fopen( osPath, "rb" );
	if ( f ) {
		fseek( f, 0, SEEK_END );
		diskLen = ftell( f );
		fseek( f, 0, SEEK_SET );

		if ( diskLen > 0 ) {
			diskData = Z_MallocInternal( diskLen );
			fread( diskData, 1, diskLen, f );

			if ( diskLen == fsLen ) {
				for ( i = 0 ; i < diskLen ; i++ ) {
					if ( diskData[i] != ( (byte *)fsData )[i] ) {
						break;
					}
				}
				if ( i == diskLen ) {
					different = qfalse;
				}
			}

			free( diskData );
		}
		fclose( f );
	}

	if ( different ) {
		out = FS_FOpenFileWrite( qpath );
		if ( !out ) {
			Com_Printf( "Failed to open %s\n", qpath );
			return 0;
		}
		FS_Write( fsData, fsLen, out );
		FS_FCloseFile( out );
	}

	FS_FreeFile( fsData );
	return 1;
}

/* ---- FS_Read2  0x0043ADD0 ---- VERIFIED */
int FS_Read2( void *buffer, int len, fileHandle_t f ) {
	if ( !f ) {
		return 0;
	}
	if ( fsh[f].streamed ) {
		int r;
		fsh[f].streamed = qfalse;
		r = FS_Read( buffer, len, f );
		fsh[f].streamed = qtrue;
		return r;
	} else {
		return FS_Read( buffer, len, f );
	}
}

/* ---- FS_RemoveCommands  0x0043BA50 ---- VERIFIED */
void FS_RemoveCommands( void ) {
	Cmd_RemoveCommand( "path" );
	Cmd_RemoveCommand( "dir" );
	Cmd_RemoveCommand( "fdir" );
	Cmd_RemoveCommand( "touchFile" );
}

/* ---- FS_AddCommands  0x0043BA80 ---- VERIFIED */
void FS_AddCommands( void ) {
	Cmd_AddCommand( "path", FS_Path_f );
	Cmd_AddCommand( "fullpath", FS_FullPath_f );
	Cmd_AddCommand( "dir", FS_Dir_f );
	Cmd_AddCommand( "fdir", FS_NewDir_f );
	Cmd_AddCommand( "touchFile", FS_TouchFile_f );
}

extern char **FS_ListFilteredFiles( const char *path, const char *extension, char *filter, int *numfiles );
extern void FS_SortFileList( char **filelist, int numfiles );
extern void FS_ConvertPath( char *s );
extern int  FS_FOpenFileRead_Internal( const char *qpath, fileHandle_t *file, qboolean uniqueFILE, int filter_flag );

/* ---- FS_Dir_f  0x0043B4A0 ---- VERIFIED */
void FS_Dir_f( void ) {
	char	*path;
	char	*extension;
	char	**dirnames;
	int		ndirs;
	int		i;

	if ( Cmd_Argc() < 2 || Cmd_Argc() > 3 ) {
		Com_Printf( "usage: dir <directory> [extension]\n" );
		return;
	}

	if ( Cmd_Argc() == 2 ) {
		path = Cmd_Argv( 1 );
		extension = "";
	} else {
		path = Cmd_Argv( 1 );
		extension = Cmd_Argv( 2 );
	}

	Com_Printf( "Directory of %s %s\n", path, extension );
	Com_Printf( "---------------\n" );

	dirnames = FS_ListFilteredFiles( path, extension, NULL, &ndirs );

	for ( i = 0 ; i < ndirs ; i++ ) {
		Com_Printf( "%s\n", dirnames[i] );
	}

	FS_FreeFileList( dirnames );
}

/* ---- FS_NewDir_f  0x0043B580 ---- VERIFIED */
void FS_NewDir_f( void ) {
	char	*filter;
	char	**dirnames;
	int		ndirs;
	int		i;

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "usage: fdir <filter>\n" );
		Com_Printf( "example: fdir *q3dm*.bsp\n" );
		return;
	}

	filter = Cmd_Argv( 1 );

	Com_Printf( "---------------\n" );

	dirnames = FS_ListFilteredFiles( "", "", filter, &ndirs );

	FS_SortFileList( dirnames, ndirs );

	for ( i = 0 ; i < ndirs ; i++ ) {
		FS_ConvertPath( dirnames[i] );
		Com_Printf( "%s\n", dirnames[i] );
	}

	Com_Printf( "%d files listed\n", ndirs );

	FS_FreeFileList( dirnames );
}

/* ---- FS_TouchFile_f  0x0043B660 ---- VERIFIED */
void FS_TouchFile_f( void ) {
	fileHandle_t	f;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: touchFile <file>\n" );
		return;
	}

	fs_loadingMode = 1;

	FS_FOpenFileRead_Internal( Cmd_Argv( 1 ), &f, qfalse, 0 );
	if ( f ) {
		FS_FCloseFile( f );
	}
}


/* ==========================================================================
 * Merged from files_raw.c (retail linked it as a separate translation unit).
 * ========================================================================== */

void Cmd_TokenizeString2( const char *text_in, int max_tokens );

int MSS_StopSounds( int flags );

static char fs_referencedPakChecksums[BIG_INFO_STRING];     /* 0x008AFC48 */
static char fs_referencedPakNames[BIG_INFO_STRING];         /* 0x008B1C48 */
static char fs_referencedPakPureChecksums[BIG_INFO_STRING]; /* 0x008B3C48 */
static char fs_loadedPakPureChecksums[BIG_INFO_STRING];     /* 0x008B5C48 */
static char fs_loadedPakNames[BIG_INFO_STRING];             /* 0x008B7C48 */
static char fs_loadedPakChecksums[BIG_INFO_STRING];         /* 0x008B9C48 */
static char fs_buf[1024];                                   /* 0x008AF848 */

static int fs_numServerReferencedPaks;
static int fs_serverReferencedPaks[MAX_SEARCH_PATHS];
static char *fs_serverReferencedPakNames[MAX_SEARCH_PATHS];

static int fs_checksumFeed;
static int fs_fakeChkSum;

extern char *fs_serverPakNames[];

char *FS_ShiftStr( const char *string, int shift );
int FS_FileIsInPAK( const char *filename, int *pChecksum );
int FS_ExtractFromPakFile_m( char *ospath, int game, char *qpath );
int Sys_CountFileList( char **list );
char **Sys_ConcatenateFileLists( char **list0, char **list1, char **list2 );
qboolean FS_idPak( const char *pak, const char *base );
qboolean FS_ComparePaks( char *neededpaks, int len, qboolean dlstring );
void FS_SetRestrictions( void );
const char *FS_LoadedPakChecksums( void );
const char *FS_LoadedPakNames( void );
const char *FS_LoadedPakPureChecksums( void );
const char *FS_ReferencedPakChecksums( void );
const char *FS_ReferencedPakNames( void );
const char *FS_ReferencedPakPureChecksums( void );

#define NUM_ID_PAKS     9

/* ---- FS_SV_FileExists  0x0043A730 ---- VERIFIED */
qboolean FS_SV_FileExists( const char *file ) {
	char testpath[MAX_OSPATH];
	FILE *f;

	FS_BuildOSPath3( fs_homepath->string, file, "", testpath, 0 );
	testpath[strlen( testpath ) - 1] = '\0';

	f = fopen( testpath, "rb" );
	if ( f ) {
		fclose( f );
		return qtrue;
	}
	return qfalse;
}

/* ---- FS_SV_FOpenFileWrite  0x0043A7E0 ---- VERIFIED */
fileHandle_t FS_SV_FOpenFileWrite( const char *filename ) {
	char ospath[MAX_OSPATH];
	fileHandle_t f;

	FS_BuildOSPath3( fs_homepath->string, filename, "", ospath, 0 );
	ospath[strlen( ospath ) - 1] = '\0';

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	if ( fs_debug->integer ) {
		Com_Printf( "FS_SV_FOpenFileWrite: %s\n", ospath );
	}

	if ( FS_CreatePath( ospath ) ) {
		return 0;
	}

	Com_DPrintf( "writing to: %s\n", ospath );
	fsh[f].handleFiles.file.o = fopen( ospath, "wb" );

	strncpy( fsh[f].name, filename, sizeof( fsh[f].name ) - 1 );
	fsh[f].name[sizeof( fsh[f].name ) - 1] = '\0';

	fsh[f].handleSync = qfalse;

	if ( !fsh[f].handleFiles.file.o ) {
		f = 0;
	}
	return f;
}

/* ---- FS_SV_FOpenFileRead  0x0043A900 ---- VERIFIED */
int FS_SV_FOpenFileRead( const char *filename, fileHandle_t *fp ) {
	char ospath[MAX_OSPATH];
	fileHandle_t f;

	f = FS_HandleForFile();
	fsh[f].zipFile = qfalse;

	strncpy( fsh[f].name, filename, sizeof( fsh[f].name ) - 1 );
	fsh[f].name[sizeof( fsh[f].name ) - 1] = '\0';

	FS_BuildOSPath3( fs_homepath->string, filename, "", ospath, 0 );
	ospath[strlen( ospath ) - 1] = '\0';

	if ( fs_debug->integer ) {
		Com_Printf( "FS_SV_FOpenFileRead (fs_homepath): %s\n", ospath );
	}

	fsh[f].handleFiles.file.o = fopen( ospath, "rb" );
	fsh[f].handleSync = qfalse;

	if ( !fsh[f].handleFiles.file.o ) {
		if ( !fs_homepath->string || !fs_basepath->string
			 || Q_stricmpn( fs_homepath->string, fs_basepath->string, 99999 ) ) {
			FS_BuildOSPath3( fs_basepath->string, filename, "", ospath, 0 );
			ospath[strlen( ospath ) - 1] = '\0';

			if ( fs_debug->integer ) {
				Com_Printf( "FS_SV_FOpenFileRead (fs_basepath): %s\n", ospath );
			}

			fsh[f].handleFiles.file.o = fopen( ospath, "rb" );
			fsh[f].handleSync = qfalse;

			if ( !fsh[f].handleFiles.file.o ) {
				f = 0;
			}
		}
	}

	if ( !fsh[f].handleFiles.file.o ) {
		FS_BuildOSPath3( fs_cdpath->string, filename, "", ospath, 0 );
		ospath[strlen( ospath ) - 1] = '\0';

		if ( fs_debug->integer ) {
			Com_Printf( "FS_SV_FOpenFileRead (fs_cdpath) : %s\n", ospath );
		}

		fsh[f].handleFiles.file.o = fopen( ospath, "rb" );
		fsh[f].handleSync = qfalse;

		if ( !fsh[f].handleFiles.file.o ) {
			f = 0;
		}
	}

	*fp = f;
	if ( f ) {
		return FS_filelength( f );
	}
	return 0;
}

/* ---- FS_SV_Rename  0x0043AB30 ---- VERIFIED */
void FS_SV_Rename( const char *from, const char *to ) {
	char from_ospath[MAX_OSPATH];
	char to_ospath[MAX_OSPATH];

	FS_BuildOSPath3( fs_homepath->string, from, "", from_ospath, 0 );
	FS_BuildOSPath3( fs_homepath->string, to, "", to_ospath, 0 );
	from_ospath[strlen( from_ospath ) - 1] = '\0';
	to_ospath[strlen( to_ospath ) - 1] = '\0';

	if ( fs_debug->integer ) {
		Com_Printf( "FS_SV_Rename: %s --> %s\n", from_ospath, to_ospath );
	}

	if ( rename( from_ospath, to_ospath ) ) {
		FS_CopyFile( from_ospath, to_ospath );
		remove( from_ospath );
	}
}

/* ---- FS_ShiftStr  0x0043AC40 ---- VERIFIED */
char *FS_ShiftStr( const char *string, int shift ) {
	int i, l;

	l = strlen( string );
	for ( i = 0 ; i < l ; i++ ) {
		fs_buf[i] = (char) ( string[i] + shift );
	}
	fs_buf[i] = '\0';

	return fs_buf;
}

/* ---- FS_ExtractFromPakFile_m  0x0043ACA0 ---- VERIFIED */
int FS_ExtractFromPakFile_m( char *ospath, int game, char *qpath ) {
	void *fsData;
	int fsLen;
	FILE *f;
	int diskLen;
	byte *diskData;
	int i;
	qboolean different;
	fileHandle_t out;

	(void) game;

	different = qtrue;

	fsLen = FS_ReadFile( qpath, &fsData );
	if ( fsLen == -1 ) {
		return 0;
	}

	f = fopen( ospath, "rb" );
	if ( f ) {
		fseek( f, 0, SEEK_END );
		diskLen = ftell( f );
		fseek( f, 0, SEEK_SET );

		if ( diskLen > 0 ) {
			diskData = Z_MallocInternal( diskLen );
			fread( diskData, 1, diskLen, f );

			if ( diskLen == fsLen ) {
				for ( i = 0 ; i < diskLen ; i++ ) {
					if ( diskData[i] != ( (byte *) fsData )[i] ) {
						break;
					}
				}
				if ( i == diskLen ) {
					different = qfalse;
				}
			}
			free( diskData );
		}
		fclose( f );
	}

	if ( different ) {
		out = FS_FOpenFileWrite( qpath );
		if ( !out ) {
			Com_Printf( "Failed to open %s\n", qpath );
			return 0;
		}
		FS_Write( fsData, fsLen, out );
		FS_FCloseFile( out );
	}

	FS_FreeFile( fsData );
	return 1;
}

/* ---- FS_FileIsInPAK  0x0043AE20 ---- VERIFIED */
int FS_FileIsInPAK( const char *filename, int *pChecksum ) {
	searchpath_t *search;
	pack_t *pak;
	fileInPack_t *pakFile;
	long hash = 0;

	if ( !filename ) {
		Com_Error( ERR_FATAL, "\x15" "FS_FOpenFileRead: NULL 'filename' parameter passed\n" );
	}

	if ( filename[0] == '/' || filename[0] == '\\' ) {
		filename++;
	}

	if ( strstr( filename, ".." ) || strstr( filename, "::" ) ) {
		return -1;
	}

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( search->pack ) {
			hash = FS_HashFileName( filename, search->pack->hashSize );
		}

		if ( search->pack ) {
			pak = search->pack;

			if ( pak->hashTable[hash] && FS_PakIsPure( pak ) ) {
				pakFile = pak->hashTable[hash];
				do {
					if ( !FS_FilenameCompare( pakFile->name, filename ) ) {
						if ( pChecksum ) {
							*pChecksum = pak->pure_checksum;
						}
						return 1;
					}
					pakFile = pakFile->next;
				} while ( pakFile );
			}
		}
	}

	return -1;
}

/* ---- Sys_CountFileList  0x0043AF10 ---- VERIFIED */
int Sys_CountFileList( char **list ) {
	int i = 0;

	if ( list ) {
		while ( *list ) {
			list++;
			i++;
		}
	}
	return i;
}

/* ---- Sys_ConcatenateFileLists  0x0043AF30 ---- VERIFIED */
char **Sys_ConcatenateFileLists( char **list0, char **list1, char **list2 ) {
	int totalLength;
	char **cat;
	char **dst;
	char **src;

	totalLength = 0;
	totalLength += Sys_CountFileList( list0 );
	totalLength += Sys_CountFileList( list1 );
	totalLength += Sys_CountFileList( list2 );

	dst = cat = (char **) Z_MallocInternal( ( totalLength + 1 ) * sizeof( char * ) );

	if ( list0 ) {
		for ( src = list0 ; *src ; src++, dst++ ) {
			*dst = *src;
		}
	}
	if ( list1 ) {
		for ( src = list1 ; *src ; src++, dst++ ) {
			*dst = *src;
		}
	}
	if ( list2 ) {
		for ( src = list2 ; *src ; src++, dst++ ) {
			*dst = *src;
		}
	}

	*dst = NULL;

	if ( list0 ) {
		free( list0 );
	}
	if ( list1 ) {
		free( list1 );
	}
	if ( list2 ) {
		free( list2 );
	}

	return cat;
}

static void FS_FreeSysFileList( char **list ) {
	char **p;

	if ( !list ) {
		return;
	}
	for ( p = list ; *p ; p++ ) {
		free( *p );
	}
	free( list );
}

/* ---- FS_GetModList  0x0043B030 ---- VERIFIED */
int FS_GetModList( char *listbuf, int bufsize ) {
	int nMods, i, j, nTotal, nLen, nPaks, nPotential, nDescLen;
	char **pFiles = NULL;
	char **pPaks = NULL;
	char *name;
	char descPath[MAX_OSPATH];
	char fs_path[MAX_OSPATH];
	fileHandle_t descHandle;
	int dummy;
	char **pFiles0 = NULL;
	char **pFiles1 = NULL;
	char **pFiles2 = NULL;
	qboolean bDrop = qfalse;

	*listbuf = '\0';
	nMods = nPotential = nTotal = 0;

	pFiles0 = Sys_ListFiles( fs_homepath->string, NULL, NULL, &dummy, qtrue );
	pFiles1 = Sys_ListFiles( fs_basepath->string, NULL, NULL, &dummy, qtrue );
	if ( fs_cdpath->string && strlen( fs_cdpath->string ) ) {
		pFiles2 = Sys_ListFiles( fs_cdpath->string, NULL, NULL, &dummy, qtrue );
	}

	pFiles = Sys_ConcatenateFileLists( pFiles0, pFiles1, pFiles2 );
	nPotential = Sys_CountFileList( pFiles );

	for ( i = 0 ; i < nPotential ; i++ ) {
		name = pFiles[i];

		if ( i != 0 ) {
			bDrop = qfalse;
			for ( j = 0 ; j < i ; j++ ) {
				if ( pFiles[j] && name && !Q_stricmpn( pFiles[j], name, 99999 ) ) {
					bDrop = qtrue;
					break;
				}
			}
			if ( bDrop ) {
				continue;
			}
		} else if ( bDrop ) {
			continue;
		}

		if ( name && !Q_stricmpn( name, "main", 99999 ) ) {
			continue;
		}
		if ( !Q_stricmpn( name, ".", 1 ) ) {
			continue;
		}

		FS_BuildOSPath3( fs_basepath->string, name, "", fs_path, 0 );
		nPaks = 0;
		pPaks = Sys_ListFiles( fs_path, ".pk3", NULL, &nPaks, qfalse );
		FS_FreeSysFileList( pPaks );

		if ( nPaks <= 0 ) {
			FS_BuildOSPath3( fs_cdpath->string, name, "", fs_path, 0 );
			nPaks = 0;
			pPaks = Sys_ListFiles( fs_path, ".pk3", NULL, &nPaks, qfalse );
			FS_FreeSysFileList( pPaks );
		}
		if ( nPaks <= 0 ) {
			FS_BuildOSPath3( fs_homepath->string, name, "", fs_path, 0 );
			nPaks = 0;
			pPaks = Sys_ListFiles( fs_path, ".pk3", NULL, &nPaks, qfalse );
			FS_FreeSysFileList( pPaks );
		}
		if ( nPaks <= 0 ) {
			continue;
		}

		nLen = strlen( name ) + 1;

		descPath[0] = '\0';
		strcpy( descPath, name );
		strcat( descPath, "/description.txt" );

		nDescLen = FS_SV_FOpenFileRead( descPath, &descHandle );
		if ( nDescLen > 0 && descHandle ) {
			FILE *file = (FILE *) fsh[descHandle].handleFiles.file.o;

			Com_Memset( descPath, 0, sizeof( descPath ) );
			nDescLen = fread( descPath, 1, 48, file );
			if ( nDescLen >= 0 ) {
				descPath[nDescLen] = '\0';
			}
			FS_FCloseFile( descHandle );
		} else {
			strcpy( descPath, name );
		}
		nDescLen = strlen( descPath ) + 1;

		if ( nTotal + nLen + nDescLen + 2 >= bufsize ) {
			break;
		}

		strcpy( listbuf, name );
		strcpy( listbuf + nLen, descPath );
		listbuf += nLen + nDescLen;
		nTotal += nLen + nDescLen;
		nMods++;
	}

	FS_FreeSysFileList( pFiles );

	return nMods;
}

/* ---- FS_idPak  0x0043B6B0 ---- VERIFIED */
qboolean FS_idPak( const char *pak, const char *base ) {
	int i;
	char *p;
	char teststr[64];

	if ( !FS_FilenameCompare( pak, va( "%s/mp_bin", base ) ) ) {
		return qtrue;
	}

	for ( i = 0 ; i < NUM_ID_PAKS ; i++ ) {
		if ( !FS_FilenameCompare( pak, va( "%s/pak%d", base, i ) ) ) {
			return qtrue;
		}
		if ( !FS_FilenameCompare( pak, va( "%s/mp_pak%d", base, i ) ) ) {
			return qtrue;
		}
		if ( !FS_FilenameCompare( pak, va( "%s/sp_pak%d", base, i ) ) ) {
			return qtrue;
		}
	}

	p = strstr( pak, "localized_" );
	if ( !p ) {
		return qfalse;
	}

	strcpy( teststr, pak );
	teststr[( p - pak ) + 10] = '\0';
	if ( FS_FilenameCompare( teststr, va( "%s/localized_", base ) ) ) {
		return qfalse;
	}

	strcpy( teststr, p + 10 );
	for ( i = 0 ; teststr[i] ; i++ ) {
		teststr[i] = (char) tolower( teststr[i] );
	}

	for ( i = 0 ; i < NUM_ID_PAKS ; i++ ) {
		if ( strstr( teststr, va( "_pak%d", i ) ) ) {
			return qtrue;
		}
	}

	return qfalse;
}

/* ---- FS_ComparePaks  0x0043B830 ---- VERIFIED */
qboolean FS_ComparePaks( char *neededpaks, int len, qboolean dlstring ) {
	searchpath_t *sp;
	qboolean havepak;
	char dest[MAX_OSPATH];
	int i;

	if ( !fs_numServerReferencedPaks ) {
		return qfalse;
	}

	*neededpaks = '\0';

	for ( i = 0 ; i < fs_numServerReferencedPaks ; i++ ) {
		if ( FS_idPak( fs_serverReferencedPakNames[i], "main" ) ) {
			continue;
		}

		havepak = qfalse;
		for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
			if ( sp->pack && sp->pack->checksum == fs_serverReferencedPaks[i] ) {
				havepak = qtrue;
				break;
			}
		}
		if ( havepak ) {
			continue;
		}

		if ( !fs_serverReferencedPakNames[i] || !*fs_serverReferencedPakNames[i] ) {
			continue;
		}

		if ( dlstring ) {
			Q_strcat( neededpaks, len, "@" );
			Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
			Q_strcat( neededpaks, len, ".pk3" );
			Q_strcat( neededpaks, len, "@" );

			if ( FS_SV_FileExists( va( "%s.pk3", fs_serverReferencedPakNames[i] ) ) ) {
				Com_sprintf( dest, sizeof( dest ), "%s.%08x.pk3",
							 fs_serverReferencedPakNames[i], fs_serverReferencedPaks[i] );
				Q_strcat( neededpaks, len, dest );
			} else {
				Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
				Q_strcat( neededpaks, len, ".pk3" );
			}
		} else {
			Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
			Q_strcat( neededpaks, len, ".pk3" );
			if ( FS_SV_FileExists( va( "%s.pk3", fs_serverReferencedPakNames[i] ) ) ) {
				Q_strcat( neededpaks, len, " (local file exists with wrong checksum)" );
			}
			Q_strcat( neededpaks, len, "\n" );
		}
	}

	if ( *neededpaks ) {
		Com_Printf( "Need paks: %s\n", neededpaks );
		return qtrue;
	}

	return qfalse;
}

/* ---- FS_SetRestrictions  0x0043BAD0 ---- VERIFIED */
void FS_SetRestrictions( void ) {
	searchpath_t *sp;
	int checksum;

	if ( !fs_restrict->integer ) {
		return;
	}

	Cvar_Set2( "fs_restrict", "1", qtrue );
	Com_Printf( "\nRunning in restricted demo mode.\n\n" );

	Com_Printf( "FS_SetRestrictions: demomain restart skipped -- FS_Startup is static in universal/com_files.c\n" );

	for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
		if ( sp->localized && ( fs_ignoreLocalized->integer
								|| sp->language != cl_language->integer ) ) {
			continue;
		}
		if ( sp->pack ) {
			checksum = sp->pack->checksum;
			if ( ( checksum ^ 0x2261994 ) != (int) 0xB3D38C61 ) {
				Com_Error( ERR_FATAL, "Corrupted pak0.pk3: %u", checksum );
			}
		}
	}
}

/* ---- FS_LoadedPakChecksums  0x0043BB80 ---- VERIFIED */
const char *FS_LoadedPakChecksums( void ) {
	searchpath_t *search;

	fs_loadedPakChecksums[0] = '\0';

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		if ( search->localized ) {
			continue;
		}
		Q_strcat( fs_loadedPakChecksums, sizeof( fs_loadedPakChecksums ),
				  va( "%i ", search->pack->checksum ) );
	}

	return fs_loadedPakChecksums;
}

/* ---- FS_LoadedPakNames  0x0043BC20 ---- VERIFIED */
const char *FS_LoadedPakNames( void ) {
	searchpath_t *search;

	fs_loadedPakNames[0] = '\0';

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		if ( search->localized ) {
			continue;
		}
		if ( fs_loadedPakNames[0] ) {
			Q_strcat( fs_loadedPakNames, sizeof( fs_loadedPakNames ), " " );
		}
		Q_strcat( fs_loadedPakNames, sizeof( fs_loadedPakNames ),
				  search->pack->pakBasename );
	}

	return fs_loadedPakNames;
}

/* ---- FS_LoadedPakPureChecksums  0x0043BD20 ---- VERIFIED */
const char *FS_LoadedPakPureChecksums( void ) {
	searchpath_t *search;

	fs_loadedPakPureChecksums[0] = '\0';

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		if ( search->localized ) {
			continue;
		}
		Q_strcat( fs_loadedPakPureChecksums, sizeof( fs_loadedPakPureChecksums ),
				  va( "%i ", search->pack->pure_checksum ) );
	}

	return fs_loadedPakPureChecksums;
}

/* ---- FS_ReferencedPakChecksums  0x0043BDC0 ---- VERIFIED */
const char *FS_ReferencedPakChecksums( void ) {
	searchpath_t *search;

	fs_referencedPakChecksums[0] = '\0';

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		if (
			 Q_stricmpn( search->pack->pakGamename, "main", 4 ) || 1 ) {
			Q_strcat( fs_referencedPakChecksums, sizeof( fs_referencedPakChecksums ),
					  va( "%i ", search->pack->checksum ) );
		}
	}

	return fs_referencedPakChecksums;
}

/* ---- FS_ReferencedPakNames  0x0043BE90 ---- VERIFIED */
const char *FS_ReferencedPakNames( void ) {
	searchpath_t *search;

	fs_referencedPakNames[0] = '\0';

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		if ( Q_stricmpn( search->pack->pakGamename, "main", 4 ) || 1 ) {
			if ( fs_referencedPakNames[0] ) {
				Q_strcat( fs_referencedPakNames, sizeof( fs_referencedPakNames ), " " );
			}
			Q_strcat( fs_referencedPakNames, sizeof( fs_referencedPakNames ),
					  search->pack->pakGamename );
			Q_strcat( fs_referencedPakNames, sizeof( fs_referencedPakNames ), "/" );
			Q_strcat( fs_referencedPakNames, sizeof( fs_referencedPakNames ),
					  search->pack->pakBasename );
		}
	}

	return fs_referencedPakNames;
}

/* ---- FS_ReferencedPakPureChecksums  0x0043C050 ---- VERIFIED */
const char *FS_ReferencedPakPureChecksums( void ) {
	searchpath_t *search;
	int nFlags, numPaks, checksum;
	size_t l;

	fs_referencedPakPureChecksums[0] = '\0';

	checksum = fs_checksumFeed;
	numPaks = 0;

	for ( nFlags = 2 ; nFlags >= 0 ; nFlags-- ) {
		if ( nFlags == 0 ) {
			l = strlen( fs_referencedPakPureChecksums );
			fs_referencedPakPureChecksums[l + 1] = '\0';
			l = strlen( fs_referencedPakPureChecksums );
			fs_referencedPakPureChecksums[l + 2] = '\0';
			fs_referencedPakPureChecksums[strlen( fs_referencedPakPureChecksums )] = '@';
			fs_referencedPakPureChecksums[strlen( fs_referencedPakPureChecksums )] = ' ';
		}

		for ( search = fs_searchpaths ; search ; search = search->next ) {
			if ( !search->pack || search->localized ) {
				continue;
			}
			if ( !( (const byte *) &search->pack->referenced )[nFlags] ) {
				continue;
			}

			Q_strcat( fs_referencedPakPureChecksums,
					  sizeof( fs_referencedPakPureChecksums ),
					  va( "%i ", search->pack->pure_checksum ) );

			if ( nFlags == 2 || nFlags == 1 ) {
				break;
			}
			checksum ^= search->pack->pure_checksum;
			numPaks++;
		}

		if ( fs_fakeChkSum ) {
			Q_strcat( fs_referencedPakPureChecksums,
					  sizeof( fs_referencedPakPureChecksums ),
					  va( "%i ", fs_fakeChkSum ) );
		}
	}

	checksum ^= numPaks;
	Q_strcat( fs_referencedPakPureChecksums, sizeof( fs_referencedPakPureChecksums ),
			  va( "%i ", checksum ) );

	return fs_referencedPakPureChecksums;
}

/* ---- FS_PureServerSetLoadedPaks  0x0043C290 ---- VERIFIED */
void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames ) {
	int i, j, c, d, len;
	int newPaks[MAX_SEARCH_PATHS];
	char *newPakNames[MAX_SEARCH_PATHS];
	const char *s;
	char *p;

	Cmd_TokenizeString2( pakSums, 0 );
	c = Cmd_Argc();
	if ( c > MAX_SEARCH_PATHS ) {
		c = MAX_SEARCH_PATHS;
	}
	for ( i = 0 ; i < c ; i++ ) {
		s = ( i < Cmd_Argc() ) ? Cmd_Argv( i ) : "";
		newPaks[i] = atoi( s );
	}

	Cmd_TokenizeString2( pakNames, 0 );
	d = Cmd_Argc();
	if ( d > MAX_SEARCH_PATHS ) {
		d = MAX_SEARCH_PATHS;
	}
	for ( i = 0 ; i < d ; i++ ) {
		s = ( i < Cmd_Argc() ) ? Cmd_Argv( i ) : "";
		len = strlen( s ) + 1;
		p = malloc( len );
		if ( !p ) {
			Sys_OutOfMemoryError();
		}
		Com_Memset( p, 0, len );
		strcpy( p, s );
		newPakNames[i] = p;
	}

	if ( c != d ) {
		Com_Error( ERR_DROP, "pak sum/name mismatch" );
	}

	if ( c == fs_numServerPaks && c > 0 ) {
		for ( i = 0 ; i < c ; i++ ) {
			for ( j = 0 ; j < fs_numServerPaks ; j++ ) {
				if ( newPaks[i] == fs_serverPaks[j]
					 && newPakNames[i] && fs_serverPakNames[j]
					 && !Q_stricmp( newPakNames[i], fs_serverPakNames[j] ) ) {
					break;
				}
			}
			if ( j >= fs_numServerPaks ) {
#ifndef DEDICATED
				MSS_StopSounds( 8 );
#endif

				for ( j = 0 ; j < fs_numServerPaks ; j++ ) {
					if ( fs_serverPakNames[j] ) {
						free( fs_serverPakNames[j] );
					}
					fs_serverPakNames[j] = NULL;
				}

				fs_numServerPaks = c;
				if ( c ) {
					Com_DPrintf( "Connected to a pure server.\n" );
					Com_Memcpy( fs_serverPaks, newPaks, 4 * fs_numServerPaks );
					Com_Memcpy( fs_serverPakNames, newPakNames, 4 * fs_numServerPaks );
				}
				return;
			}
		}
	}

	for ( j = 0 ; j < d ; j++ ) {
		free( newPakNames[j] );
	}
}

/* ---- FS_PureServerSetReferencedPaks  0x0043C530 ---- VERIFIED */
void FS_PureServerSetReferencedPaks( const char *pakSums, const char *pakNames ) {
	int i, c, d, len;
	const char *s;
	char *p;

	Cmd_TokenizeString2( pakSums, 0 );
	c = Cmd_Argc();
	if ( c > MAX_SEARCH_PATHS ) {
		c = MAX_SEARCH_PATHS;
	}

	for ( i = 0 ; i < fs_numServerReferencedPaks ; i++ ) {
		if ( fs_serverReferencedPakNames[i] ) {
			free( fs_serverReferencedPakNames[i] );
		}
		fs_serverReferencedPakNames[i] = NULL;
	}
	fs_numServerReferencedPaks = 0;

	for ( i = 0 ; i < c ; i++ ) {
		s = ( i < Cmd_Argc() ) ? Cmd_Argv( i ) : "";
		fs_serverReferencedPaks[i] = atoi( s );
	}

	if ( !pakNames || !*pakNames ) {
		if ( c ) {
			Com_Error( ERR_DROP, "pak sum/name mismatch" );
		}
		fs_numServerReferencedPaks = c;
		return;
	}

	Cmd_TokenizeString2( pakNames, 0 );
	d = Cmd_Argc();
	if ( d > MAX_SEARCH_PATHS ) {
		d = MAX_SEARCH_PATHS;
	}

	if ( c != d ) {
		Com_Error( ERR_DROP, "pak sum/name mismatch" );
	}

	if ( d <= 0 ) {
		fs_numServerReferencedPaks = c;
		return;
	}

	for ( i = 0 ; i < d ; i++ ) {
		s = ( i < Cmd_Argc() ) ? Cmd_Argv( i ) : "";
		len = strlen( s ) + 1;
		p = malloc( len );
		if ( !p ) {
			Sys_OutOfMemoryError();
		}
		Com_Memset( p, 0, len );
		strcpy( p, s );
		fs_serverReferencedPakNames[i] = p;
	}

	fs_numServerReferencedPaks = c;
}
