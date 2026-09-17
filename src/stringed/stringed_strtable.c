/*
 * stringed/stringed_strtable.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/stringed/stringed_interface.cpp
 *
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"

#include <string.h>
#include <stdlib.h>

extern const char *SEH_GetLanguageName( int language );

#define SE_NUM_LANGUAGES    14
#define SE_HASH_SIZE        1024
#define SE_MAX_KEY          128
#define SE_MAX_VALUE        1024
#define SE_MAX_LINE         2048

typedef struct seString_s {
	struct seString_s   *next;
	char                *key;
	char                *value;
	char                *reference;
} seString_t;

static seString_t   *se_hash[SE_HASH_SIZE];
static int          se_numStrings;

static int          se_useReference;

static char         se_activeKeyword[32];
static int          se_activeIsEnglish;

static char         se_filePrefix[SE_MAX_KEY];

/* SE_HashKey  0x004AAC80 */
static unsigned int SE_HashKey( const char *key ) {
	unsigned int h;

	h = 2166136261u;
	while ( *key ) {
		h ^= (unsigned char)*key++;
		h *= 16777619u;
	}
	return h & ( SE_HASH_SIZE - 1 );
}

static seString_t *SE_Find( const char *key ) {
	seString_t *e;

	for ( e = se_hash[SE_HashKey( key )] ; e ; e = e->next ) {
		if ( !strcmp( e->key, key ) ) {
			return e;
		}
	}
	return NULL;
}

static char *SE_CopyString( const char *s ) {
	char    *out;
	int     len;

	len = strlen( s );
	out = (char *)malloc( len + 1 );
	if ( !out ) {
		return NULL;
	}
	memcpy( out, s, len + 1 );
	return out;
}

static void SE_ClearStore( void ) {
	int         i;
	seString_t  *e, *next;

	for ( i = 0 ; i < SE_HASH_SIZE ; i++ ) {
		for ( e = se_hash[i] ; e ; e = next ) {
			next = e->next;
			if ( e->key ) {
				free( e->key );
			}
			if ( e->value ) {
				free( e->value );
			}
			if ( e->reference && e->reference != e->value ) {
				free( e->reference );
			}
			free( e );
		}
		se_hash[i] = NULL;
	}
	se_numStrings = 0;
}

static void SE_SetEntry( const char *key, const char *text, int isActive, int isEnglish ) {
	seString_t      *e;
	unsigned int    slot;

	e = SE_Find( key );
	if ( !e ) {
		e = (seString_t *)malloc( sizeof( *e ) );
		if ( !e ) {
			return;
		}
		e->key = SE_CopyString( key );
		if ( !e->key ) {
			free( e );
			return;
		}
		e->value = NULL;
		e->reference = NULL;
		slot = SE_HashKey( key );
		e->next = se_hash[slot];
		se_hash[slot] = e;
		se_numStrings++;
	}

	if ( isEnglish ) {
		if ( e->reference && e->reference != e->value ) {
			free( e->reference );
		}
		e->reference = SE_CopyString( text );
		if ( !e->value ) {
			e->value = SE_CopyString( text );
		}
	}

	if ( isActive && !isEnglish ) {
		if ( !strcmp( text, "#same" ) ) {
			if ( e->reference ) {
				if ( e->value ) {
					free( e->value );
				}
				e->value = SE_CopyString( e->reference );
			}
			return;
		}
		if ( e->value ) {
			free( e->value );
		}
		e->value = SE_CopyString( text );
	}
}

static void SE_UnescapeValue( char *dst, const char *src, int dstSize ) {
	int i;

	i = 0;
	while ( *src && i < dstSize - 1 ) {
		if ( *src == '\\' && src[1] ) {
			src++;
			switch ( *src ) {
			case 'n':   dst[i++] = '\n'; break;
			case 't':   dst[i++] = '\t'; break;
			case 'r':   dst[i++] = '\r'; break;
			case '"':   dst[i++] = '"';  break;
			case '\\':  dst[i++] = '\\'; break;
			default:    dst[i++] = *src; break;
			}
			src++;
			continue;
		}
		dst[i++] = *src++;
	}
	dst[i] = '\0';
}

static int SE_ExtractQuoted( const char *rest, char *out, int outSize ) {
	const char  *last;
	char        raw[SE_MAX_LINE];
	int         len;

	if ( *rest != '"' ) {
		return 0;
	}
	last = strrchr( rest, '"' );
	if ( !last || last == rest ) {
		return 0;
	}

	len = last - ( rest + 1 );
	if ( len < 0 ) {
		return 0;
	}
	if ( len > (int)sizeof( raw ) - 1 ) {
		len = sizeof( raw ) - 1;
	}
	memcpy( raw, rest + 1, len );
	raw[len] = '\0';

	SE_UnescapeValue( out, raw, outSize );
	return 1;
}

/* SE_ParseBuffer  0x004AB470 */
static int SE_ParseBuffer( const char *text, int len, const char *qpath ) {
	char        line[SE_MAX_LINE];
	char        key[SE_MAX_KEY];
	char        value[SE_MAX_VALUE];
	char        reference[SE_MAX_KEY];
	const char  *p, *end;
	char        *w;
	int         n, i;
	int         sawEndMarker;
	int         loaded;

	p = text;
	end = text + len;
	reference[0] = '\0';
	sawEndMarker = 0;
	loaded = 0;

	while ( p < end ) {
		n = 0;
		while ( p < end && *p != '\n' ) {
			if ( n < (int)sizeof( line ) - 1 ) {
				line[n++] = *p;
			}
			p++;
		}
		if ( p < end ) {
			p++;
		}
		line[n] = '\0';

		while ( n > 0 && ( line[n - 1] == '\r' || line[n - 1] == ' ' || line[n - 1] == '\t' ) ) {
			line[--n] = '\0';
		}

		w = line;
		while ( *w == ' ' || *w == '\t' ) {
			w++;
		}
		if ( !*w || ( w[0] == '/' && w[1] == '/' ) ) {
			continue;
		}

		i = 0;
		while ( w[i] && w[i] != ' ' && w[i] != '\t' && i < (int)sizeof( key ) - 1 ) {
			key[i] = w[i];
			i++;
		}
		key[i] = '\0';
		w += i;
		while ( *w == ' ' || *w == '\t' ) {
			w++;
		}

		if ( !strcmp( key, "ENDMARKER" ) ) {
			sawEndMarker = 1;
			continue;
		}
		if ( !strcmp( key, "REFERENCE" ) ) {
			Q_strncpyz( reference, w, sizeof( reference ) );
			continue;
		}
		if ( !strcmp( key, "VERSION" ) || !strcmp( key, "CONFIG" )
			 || !strcmp( key, "FILENOTES" ) || !strcmp( key, "NOTES" )
			 || !strcmp( key, "FLAGS" ) ) {
			continue;
		}
		if ( strncmp( key, "LANG_", 5 ) ) {
			continue;
		}

		if ( !reference[0] ) {
			continue;
		}
		{
			int isEnglish = !strcmp( key, "LANG_ENGLISH" );
			int isActive  = !strcmp( key, se_activeKeyword );
			char composed[SE_MAX_KEY];

			if ( !isEnglish && !isActive ) {
				continue;
			}
			if ( !SE_ExtractQuoted( w, value, sizeof( value ) ) ) {
				continue;
			}
			Com_sprintf( composed, sizeof( composed ), "%s_%s", se_filePrefix, reference );
			SE_SetEntry( composed, value, isActive, isEnglish );
			loaded++;
		}
	}

	if ( !sawEndMarker ) {
		Com_DPrintf( "^3WARNING: %s has no ENDMARKER; it may be truncated\n", qpath );
	}
	return loaded;
}

/* SE_SetFilePrefix  0x004AAC80 */
static void SE_SetFilePrefix( const char *qpath ) {
	const char  *base, *dot;
	int         i, n;

	base = strrchr( qpath, '/' );
	if ( !base ) {
		base = strrchr( qpath, '\\' );
	}
	base = base ? base + 1 : qpath;

	dot = strrchr( base, '.' );
	n = dot ? (int)( dot - base ) : (int)strlen( base );
	if ( n > (int)sizeof( se_filePrefix ) - 1 ) {
		n = sizeof( se_filePrefix ) - 1;
	}

	for ( i = 0 ; i < n ; i++ ) {
		char c = base[i];
		if ( c >= 'a' && c <= 'z' ) {
			c = (char)( c - 'a' + 'A' );
		}
		se_filePrefix[i] = c;
	}
	se_filePrefix[n] = '\0';
}

static int SE_LoadOneFile( const char *qpath ) {
	void    *buffer;
	int     len;
	int     loaded;

	buffer = NULL;
	len = FS_ReadFile( qpath, &buffer );
	if ( len <= 0 || !buffer ) {
		if ( buffer ) {
			FS_FreeFile( buffer );
		}
		return 0;
	}

	SE_SetFilePrefix( qpath );
	loaded = SE_ParseBuffer( (const char *)buffer, len, qpath );

	FS_FreeFile( buffer );
	return loaded;
}

/* ---- SE_GetString  0x004ABF00 ---- */
char *SE_GetString( const char *key, int wantReference ) {
	seString_t *e;

	if ( !key || !key[0] ) {
		return NULL;
	}

	e = SE_Find( key );
	if ( !e ) {
		return NULL;
	}

	if ( wantReference && se_useReference && e->reference ) {
		return e->reference;
	}
	return e->value;
}

char *sub_4ABF00( const char *key, int wantReference ) {
	return SE_GetString( key, wantReference );
}

/* Clear__16CStringEdPackageFi  0x004AAAA0 */
int Clear__16CStringEdPackageFi( void *package, int bKeepFileInfo ) {
	(void)package;
	(void)bKeepFileInfo;

	SE_ClearStore();
	return 0;
}

/* ---- SE_GetNumLanguages  0x004AC1B0 ---- */
int SE_GetNumLanguages__Fv( void ) {
	char        path[MAX_QPATH];
	char        **files;
	int         numFiles;
	int         i;
	int         count;

	count = 0;
	for ( i = 0 ; i < SE_NUM_LANGUAGES ; i++ ) {
		Com_sprintf( path, sizeof( path ), "localizedstrings/%s", SEH_GetLanguageName( i ) );

		numFiles = 0;
		files = FS_ListFiles( path, ".str", &numFiles );
		if ( numFiles > 0 ) {
			count++;
		}
		if ( files ) {
			FS_FreeFileList( files );
		}
	}
	return count;
}

/* ---- SE_LoadAllLanguageFiles  0x004AC7B0 ---- */
char *SE_LoadAllLanguageFiles_m( const char *languageName, int force ) {
	static char errorText[256];
	char        path[MAX_QPATH];
	char        qpath[MAX_QPATH];
	char        **files;
	int         numFiles;
	int         i, n;
	int         loaded;
	int         totalStrings;

	if ( !languageName || !languageName[0] ) {
		languageName = "english";
	}

	Com_sprintf( se_activeKeyword, sizeof( se_activeKeyword ), "LANG_%s", languageName );
	for ( i = 0 ; se_activeKeyword[i] ; i++ ) {
		if ( se_activeKeyword[i] >= 'a' && se_activeKeyword[i] <= 'z' ) {
			se_activeKeyword[i] = (char)( se_activeKeyword[i] - 'a' + 'A' );
		}
	}
	se_activeIsEnglish = !Q_stricmp( languageName, "english" );
	se_useReference = force;

	SE_ClearStore();

	Com_sprintf( path, sizeof( path ), "localizedstrings/%s", languageName );

	numFiles = 0;
	files = FS_ListFiles( path, ".str", &numFiles );
	if ( !files || numFiles <= 0 ) {
		if ( files ) {
			FS_FreeFileList( files );
		}
		Com_sprintf( errorText, sizeof( errorText ),
					 "no .str files found under %s", path );
		return errorText;
	}

	totalStrings = 0;
	n = 0;
	for ( i = 0 ; i < numFiles ; i++ ) {
		Com_sprintf( qpath, sizeof( qpath ), "%s/%s", path, files[i] );
		loaded = SE_LoadOneFile( qpath );
		if ( loaded > 0 ) {
			totalStrings += loaded;
			n++;
		}
	}
	FS_FreeFileList( files );

	if ( !se_numStrings ) {
		Com_sprintf( errorText, sizeof( errorText ),
					 "%i .str files under %s but no strings in them", numFiles, path );
		return errorText;
	}

	Com_Printf( "Localization: %i strings for \"%s\" from %i of %i .str files\n",
				se_numStrings, languageName, n, numFiles );

	return NULL;
}
