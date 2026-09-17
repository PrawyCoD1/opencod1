/*
 * ui_gameinfo_mp.c -- arena (.arena) info loading.
 *
 * ui_mp_x86.dll (CoD 1.1, imagebase 0x40000000), unit
 * ui/ui_gameinfo.c: 0x40007170 .. 0x40007695, three functions.  RTCW's
 * ui/ui_gameinfo.c is the ancestor; CoD keeps only its arena half (the bot
 * loaders and lookups are gone) and changes three things:
 *
 *   - .arena files come from "mp/", not "scripts/", and the path is built with
 *     one sprintf( "%s/%s" ) (0x40007475) rather than strcpy/strcat.
 *   - The per-map type is the "gametype" key: a whitespace list of gametype
 *     script names parsed inside a Com_BeginParseSession and matched against
 *     uiInfo.gameTypes[] by name, each hit setting bit j of typeBits
 *     (0x400075C1 .. 0x4000763F).  No key at all gives typeBits -1, where
 *     RTCW defaulted to ffa.  RTCW's Timelimit/AxisRespawnTime/
 *     AlliedRespawnTime keys are gone.
 *   - UI_ParseInfos reads keys with Com_Parse and values with Com_ParseOnLine
 *     (CoD's names for COM_ParseExt( .., qtrue/qfalse )) and substitutes the
 *     literal "<NULL>" rather than strcpy'ing it over the token (0x40007216).
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdio.h>

#include "ui_local.h"


//
// arena and bot info
//

static int ui_numArenas;                    /* 0x40050D58 */
static char     *ui_arenaInfos[MAX_ARENAS]; /* 0x40050C58 */

/*
===============
UI_ParseInfos  0x40007170
===============
*/
int UI_ParseInfos( char *buf, int max, char *infos[] ) {
	char    *token;
	int count;
	char key[MAX_TOKEN_CHARS];
	char info[MAX_INFO_STRING];

	count = 0;

	while ( 1 ) {
		token = Com_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) {
			token = Com_Parse( &buf );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = Com_ParseOnLine( &buf );
			if ( !token[0] ) {
				token = "<NULL>";
			}
			Info_SetValueForKey( info, key, token );
		}
		//NOTE: extra space for arena number
		infos[count] = UI_Alloc( strlen( info ) + strlen( "\\num\\" ) + strlen( va( "%d", MAX_ARENAS ) ) + 1 );
		if ( infos[count] ) {
			strcpy( infos[count], info );
			count++;
		}
	}
	return count;
}

/*
===============
UI_LoadArenasFromFile  0x400072F0
===============
*/
static void UI_LoadArenasFromFile( char *filename ) {
	int len;
	fileHandle_t f;
	char buf[MAX_ARENAS_TEXT];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_ARENAS_TEXT ) {
		trap_Print( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ui_numArenas += UI_ParseInfos( buf, MAX_ARENAS - ui_numArenas, &ui_arenaInfos[ui_numArenas] );
}

/*
===============
UI_LoadArenas  0x40007400
===============
*/
void UI_LoadArenas( void ) {
	int numdirs;
	char filename[128];
	char dirlist[1024];
	char*       dirptr;
	int i, n, j;
	int dirlen;
	char        *type, *token, *p;

	ui_numArenas = 0;
	uiInfo.mapCount = 0;

	// get all arenas from .arena files
	numdirs = trap_FS_GetFileList( "mp", ".arena", dirlist, 1024 );
	dirptr  = dirlist;
	for ( i = 0; i < numdirs; i++, dirptr += dirlen + 1 ) {
		dirlen = strlen( dirptr );
		sprintf( filename, "%s/%s", "mp", dirptr );
		UI_LoadArenasFromFile( filename );
	}
	if ( UI_OutOfMemory() ) {
		trap_Print( S_COLOR_YELLOW "WARNING: not anough memory in pool to load all arenas\n" );
	}

	for ( n = 0; n < ui_numArenas; n++ ) {
		// determine type

		uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
		uiInfo.mapList[uiInfo.mapCount].mapLoadName = String_Alloc( Info_ValueForKey( ui_arenaInfos[n], "map" ) );
		uiInfo.mapList[uiInfo.mapCount].mapName = String_Alloc( Info_ValueForKey( ui_arenaInfos[n], "longname" ) );
		uiInfo.mapList[uiInfo.mapCount].levelShot = -1;
		uiInfo.mapList[uiInfo.mapCount].imageName = String_Alloc( va( "levelshots/%s", uiInfo.mapList[uiInfo.mapCount].mapLoadName ) );

		type = Info_ValueForKey( ui_arenaInfos[n], "gametype" );
		if ( type && *type ) {
			uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

			Com_BeginParseSession( va( ".arena files : %s", uiInfo.mapList[uiInfo.mapCount].mapLoadName ) );
			p = type;
			while ( 1 ) {
				token = Com_Parse( &p );
				if ( !token || !token[0] ) {
					break;
				}
				for ( j = 0; j < uiInfo.numGameTypes; j++ ) {
					if ( !Q_stricmp( uiInfo.gameTypes[j].gameType, token ) ) {
						uiInfo.mapList[uiInfo.mapCount].typeBits |= ( 1 << j );
					}
				}
			}
			Com_EndParseSession();
		} else {
			uiInfo.mapList[uiInfo.mapCount].typeBits = -1;
		}

		uiInfo.mapCount++;
		if ( uiInfo.mapCount >= MAX_MAPS ) {
			break;
		}
	}
}
