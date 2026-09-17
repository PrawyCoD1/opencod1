#include "../qcommon/qcommon.h"
#include <io.h>
#include <string.h>

#define MAX_FOUND_FILES     0x1000

/* ---- Sys_ListFilteredFiles_r  0x00460570 ----  VERIFIED */
static void Sys_ListFilteredFiles_r( const char *basedir, char *subdirs,
                                     char *filter, char **list, int *numfiles ) {
	char search[MAX_OSPATH], filename[MAX_OSPATH];
	struct _finddata_t findinfo;
	long findhandle;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	/* strlen here, a NULL test at 0x00460675 there -- retail is inconsistent in exactly this way. */
	if ( strlen( subdirs ) ) {
		Com_sprintf( search, sizeof( search ), "%s\\%s\\*", basedir, subdirs );
	} else {
		Com_sprintf( search, sizeof( search ), "%s\\*", basedir );
	}

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		return;
	}

	do {
		if ( ( findinfo.attrib & _A_SUBDIR )
		     && ( !Q_stricmp( findinfo.name, "." )
		       || !Q_stricmp( findinfo.name, ".." )
		       || !Q_stricmp( findinfo.name, "CVS" ) ) ) {
			continue;
		}

		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}

		if ( subdirs ) {
			Com_sprintf( filename, sizeof( filename ), "%s\\%s", subdirs, findinfo.name );
		} else {
			Com_sprintf( filename, sizeof( filename ), "%s", findinfo.name );
		}

		if ( !Com_FilterPath( filter, filename, qfalse ) ) {
			continue;
		}
		list[*numfiles] = CopyStringInternal( filename );
		( *numfiles )++;
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	_findclose( findhandle );
}

/* ---- Sys_ListFiles  0x00460770 ---- */
char **Sys_ListFiles( const char *directory, const char *extension,
                      const char *filter, int *numfiles, int wantsubs ) {
	char search[MAX_OSPATH];
	int nfiles;
	char **listCopy;
	char *list[MAX_FOUND_FILES];
	struct _finddata_t findinfo;
	long findhandle;
	int flag;
	int i;

	if ( filter ) {
		nfiles = 0;
		Sys_ListFilteredFiles_r( directory, "", (char *) filter, list, &nfiles );

		list[nfiles] = NULL;
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

	if ( !extension ) {
		extension = "";
	}

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	Com_sprintf( search, sizeof( search ), "%s\\*%s", directory, extension );

	nfiles = 0;

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		*numfiles = 0;
		return NULL;
	}

	do {
		if ( ( !wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR ) ) || ( wantsubs && findinfo.attrib & _A_SUBDIR ) ) {
			if ( nfiles == MAX_FOUND_FILES - 1 ) {
				break;
			}
			list[nfiles] = CopyStringInternal( findinfo.name );
			nfiles++;
		}
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	list[nfiles] = NULL;
	_findclose( findhandle );

	if ( !nfiles ) {
		*numfiles = 0;
		return NULL;
	}

	listCopy = Z_MallocInternal( ( nfiles + 1 ) * sizeof( *listCopy ) );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	*numfiles = nfiles;
	return listCopy;
}

/* ---- Sys_FreeFileList  0x00460990 ---- */
void Sys_FreeFileList( char **list ) {
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_FreeInternal( list[i] );
	}

	Z_FreeInternal( list );
}
