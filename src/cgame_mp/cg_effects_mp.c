/*
 * cg_effects.c -- the impact-effect table loader.
 *
 * Call of Duty 1.1 multiplayer client game (cgame_mp_x86.dll, imagebase
 * 0x30000000).  RTCW's cgame/cg_effects.c has nothing in common with this file:
 * CoD moved every particle/smoke/spark local entity into the engine-side FX
 * system (traps 222..235), so all that is left on the cgame side is the startup
 * pass that reads fx/*.csv and turns "effect type x surface type" into the
 * handle tables cg_event.c and cg_weapons.c index.
 *
 * The CSV is three columns: effect type name, surface type name, effect file.
 * An empty third column is an explicit "no effect here" and is recorded by
 * poking a 1 into byte 1 of the (empty) name slot -- that marker is the only
 * thing that stops CG_RegisterImpactEffectsOfType reporting the row missing
 * (0x3001A6ED / 0x3001A515).
 *
 * Functions are in address order, which is the original file order.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdlib.h>

#include "cg_local.h"

/*
 * 23 surface types: every one of the six footstep event blocks in
 * bg_misc.c's eventnames[] is 23 wide, and CG_RegisterImpactEffectsOfType
 * walks exactly 23 entries (0x3001A50A).
 */
#define SURF_TYPE_NUM               23

/* Nine effect types, the nine names CG_RegisterImpactEffects builds on its
   stack at 0x3001A81B..0x3001A85B. */
#define NUM_IMPACT_EFFECT_TYPES     9

/* The third column is strcpy'd into a 64-byte slot and rejected at 63 chars
   (0x3001A6CB / 0x3001A6DD). */
#define MAX_EFFECT_FILENAME         64

/* trap_FS_GetFileList's buffer and the pointer table qsort sorts; both are
   CG_RegisterImpactEffects locals (0x10000 and 4096 entries, 0x3001A7FB /
   0x3001A8D3). */
#define MAX_EFFECT_FILE_TEXT        0x10000
#define MAX_EFFECT_FILES            4096

/* Owned by other units. */
void        CG_Printf( const char *msg, ... );                  /* cg_main.c 0x300206F0 */
void        CG_Error( const char *msg, ... );                   /* cg_main.c 0x30020750 */

/*
==============
CG_RegisterImpactEffectsOfType     0x3001A500

Registers one effect type's 23 surface entries and returns how many rows the
CSVs never supplied.
==============
*/
static int CG_RegisterImpactEffectsOfType( char ( *fileNames )[MAX_EFFECT_FILENAME],
										   const char *effectType, int *effects ) {
	int missing;
	int i;

	missing = 0;

	for ( i = 0 ; i < SURF_TYPE_NUM ; i++ ) {
		if ( fileNames[i][0] ) {
			effects[i] = trap_syscall_0xDE( fileNames[i] );
		} else {
			/* byte 1 is CG_ParseImpactEffects' "the file said blank on purpose"
			   marker; without it the row was simply never listed. */
			if ( !fileNames[i][1] ) {
				CG_Printf( "no entry for effect type '%s' on surface type '%s'\n",
						   effectType, trap_SurfaceTypeToName( i ) );
				missing++;
			}
			effects[i] = 0;
		}
	}

	return missing;
}

/*
==============
CG_ParseImpactEffects     0x3001A570

Returns NULL on success, or a va()'d error message for CG_Error.
==============
*/
static const char *CG_ParseImpactEffects( const char *filename, char *data_p,
										  int numEffectTypes, const char **effectTypes,
										  char ( *fileNames )[SURF_TYPE_NUM][MAX_EFFECT_FILENAME] ) {
	char    *token;
	char    *name;
	int     i;
	int     surfaceType;

	while ( 1 ) {
		token = Com_Parse( &data_p );
		if ( !data_p ) {
			return NULL;
		}

		if ( !token[0] || token[0] == '#' ) {
			Com_SkipRestOfLine( 0, &data_p );
			continue;
		}

		for ( i = 0 ; i < numEffectTypes ; i++ ) {
			if ( !_stricmp( effectTypes[i], token ) ) {
				break;
			}
		}
		if ( i == numEffectTypes ) {
			return va( "unknown effect type '%s' in first column of file '%s'", token, filename );
		}

		token = Com_ParseOnLine( &data_p );
		if ( !token[0] ) {
			return va( "missing surface type in second column of file '%s'", filename );
		}

		surfaceType = trap_SurfaceTypeFromName( token );
		if ( surfaceType < 0 ) {
			return va( "unknown surface type '%s' in second column of file '%s'", token, filename );
		}

		token = Com_ParseOnLine( &data_p );

		name = fileNames[i][surfaceType];
		name[MAX_EFFECT_FILENAME - 1] = 0;
		strcpy( name, token );
		if ( name[MAX_EFFECT_FILENAME - 1] ) {
			return va( "effect filename '%s' in third column of file '%s' is longer than %i characters",
					   token, filename, MAX_EFFECT_FILENAME - 1 );
		}
		if ( !token[0] ) {
			name[1] = 1;
		}

		Com_SkipRestOfLine( 0, &data_p );
	}
}

/*
==============
compare_impact_files     0x3001A7C0
==============
*/
static int compare_impact_files( const void *a, const void *b ) {
	return _stricmp( *(const char **)a, *(const char **)b );
}

/*
==============
CG_RegisterImpactEffects     0x3001A7E0
==============
*/
void CG_RegisterImpactEffects( void ) {
	/* Both tables are built on the stack every call (0x3001A81B / 0x3001A863).
	   The nine destinations are cgs.media's nine 23-entry handle tables, which
	   cg_local.h carries as one hole. */
	const char  *effectTypes[NUM_IMPACT_EFFECT_TYPES] = {
		"bullet_small_normal",
		"bullet_small_reflect",
		"bullet_large_normal",
		"bullet_large_reflect",
		"grenade_bounce",
		"grenade_explode",
		"rocket_explode",
		"molotov_explode_normal",
		"molotov_explode_reflect"
	};
	int         *effects[NUM_IMPACT_EFFECT_TYPES] = {
		(int *)&cgs.media.unknown_0x70C[0 * SURF_TYPE_NUM * 4],  /* cgs+0xA0F0 */
		(int *)&cgs.media.unknown_0x70C[1 * SURF_TYPE_NUM * 4],  /* cgs+0xA14C */
		(int *)&cgs.media.unknown_0x70C[2 * SURF_TYPE_NUM * 4],
		(int *)&cgs.media.unknown_0x70C[3 * SURF_TYPE_NUM * 4],
		(int *)&cgs.media.unknown_0x70C[4 * SURF_TYPE_NUM * 4],
		(int *)&cgs.media.unknown_0x70C[5 * SURF_TYPE_NUM * 4],
		(int *)&cgs.media.unknown_0x70C[6 * SURF_TYPE_NUM * 4],
		(int *)&cgs.media.unknown_0x70C[7 * SURF_TYPE_NUM * 4],
		(int *)&cgs.media.unknown_0x70C[8 * SURF_TYPE_NUM * 4]   /* cgs+0xA3D0 */
	};
	char        fileNames[NUM_IMPACT_EFFECT_TYPES][SURF_TYPE_NUM][MAX_EFFECT_FILENAME];
	char        *files[MAX_EFFECT_FILES];
	char        fileText[MAX_EFFECT_FILE_TEXT];
	const char  *filename;
	const char  *error;
	char        *p;
	char        *buffer;
	fileHandle_t f;
	int         numFiles;
	int         len;
	int         missing;
	int         i;

	numFiles = trap_FS_GetFileList( "fx", "csv", fileText, sizeof( fileText ) );
	if ( !numFiles ) {
		CG_Error( "No CSV files in the fx directory to identify impact effects\n" );
	} else if ( numFiles > MAX_EFFECT_FILES ) {
		numFiles = MAX_EFFECT_FILES;
	}

	p = fileText;
	for ( i = 0 ; i < numFiles ; i++ ) {
		files[i] = p;
		p += strlen( p ) + 1;
	}

	qsort( files, numFiles, sizeof( files[0] ), compare_impact_files );

	memset( fileNames, 0, sizeof( fileNames ) );

	for ( i = 0 ; i < numFiles ; i++ ) {
		filename = va( "fx/%s", files[i] );

		len = trap_FS_FOpenFile( filename, &f, FS_READ );
		if ( len < 0 ) {
			continue;
		}

		buffer = (char *)trap_Z_MallocInternal( len + 1 );
		trap_FS_Read( buffer, len, f );
		trap_FS_FCloseFile( f );
		buffer[len] = 0;

		Com_BeginParseSession( filename );
		Com_SetCSV( qtrue );
		error = CG_ParseImpactEffects( filename, buffer, NUM_IMPACT_EFFECT_TYPES,
									   effectTypes, fileNames );
		Com_EndParseSession();
		trap_Z_FreeInternal( buffer );

		if ( error ) {
			CG_Error( error );
		}
	}

	missing = 0;
	for ( i = 0 ; i < NUM_IMPACT_EFFECT_TYPES ; i++ ) {
		missing += CG_RegisterImpactEffectsOfType( fileNames[i], effectTypes[i], effects[i] );
	}

	if ( missing ) {
		CG_Error( "%i missing entries in effect CSV files (see console for details)", missing );
	}
}
