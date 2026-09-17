/*
 * stringed/stringed_hooks.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/stringed/stringed_hooks.cpp
 *
 * Retail range 0x004A99B0-0x004AA35F, 15 functions.
 *
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"

#include <string.h>

extern cvar_t   *cl_language;                    /* 0x0161734C */
extern cvar_t   *cl_languagesavailable;          /* 0x015CE830 */
extern cvar_t   *cl_languagetranslate;           /* 0x01432954 */
extern cvar_t   *cl_languagewarnings;            /* 0x01617350 */
extern cvar_t   *cl_languagewarningsaserrors;    /* 0x01432844 */

extern cvar_t   *fs_ignoreLocalized;

extern int      g_currentAsian;

#define SE_GETSTRING    SE_GetString
extern char *SE_GetString( const char *key, int wantReference );
extern int   Clear__16CStringEdPackageFi( void *package, int bKeepFileInfo );
extern int   SE_GetNumLanguages__Fv( void );
extern char *SE_LoadAllLanguageFiles_m( const char *languageName, int force );

extern int  TheStringPackage;                   /* 0x01407450 */

typedef struct seh_searchpath_s {
	struct seh_searchpath_s *next;
	void                    *pack;
	void                    *dir;
	int                      localized;
	int                      language;
} seh_searchpath_t;

extern seh_searchpath_t *fs_searchpaths;

int SEH_StringEd_SetLanguageStrings( int language );

#define NUM_LANGUAGES       14

#define LANGUAGE_FIRST_ASIAN    8
#define LANGUAGE_LAST_ASIAN     12

static const char *seh_languageNames[NUM_LANGUAGES] = {
	"english",   "french",     "german",   "italian",
	"spanish",   "british",    "russian",  "polish",
	"korean",    "taiwanese",  "japanese", "chinese",
	"thai",      "leet"
};

static int seh_languagePresent[NUM_LANGUAGES];

#define SEH_MARK_LOC_ON     0x14
#define SEH_MARK_LOC_OFF    0x15
#define SEH_MARK_NO_INSERT  0x16

#define LOCMSG_NOERR        1

#define ERR_LOCALIZATION    7

#define MAX_LOCALIZED_LEN   1024

static int  iCurrString;
static char szStrings[2][MAX_LOCALIZED_LEN];

static char szSafeTranslate[MAX_LOCALIZED_LEN];

/* ---- SEH_UpdateCurrentLanguage  0x004A99B0 ---- */
int SEH_UpdateCurrentLanguage( void ) {
	int language;

	cl_language = Cvar_Get( "cl_language", "0", CVAR_ARCHIVE | CVAR_LATCH );

	language = cl_language->integer;
	g_currentAsian = ( language >= LANGUAGE_FIRST_ASIAN && language <= LANGUAGE_LAST_ASIAN );

	return language;
}

/* ---- SEH_GetCurrentLanguage  0x004A99F0 ---- */
int SEH_GetCurrentLanguage( void ) {
	return cl_language->integer;
}

/* ---- SEH_GetLanguageName  0x004A9A00 ---- */
const char *SEH_GetLanguageName( int language ) {
	if ( (unsigned int)language >= NUM_LANGUAGES ) {
		return seh_languageNames[0];
	}
	return seh_languageNames[language];
}

/* ---- SEH_GetLanguageIndexForName  0x004A9A20 ---- */
qboolean SEH_GetLanguageIndexForName( const char *name, int *languageOut ) {
	int i;

	for ( i = 0 ; i < NUM_LANGUAGES ; i++ ) {
		if ( name && seh_languageNames[i] && !Q_stricmpn( name, seh_languageNames[i], 99999 ) ) {
			*languageOut = i;
			return qtrue;
		}
	}

	*languageOut = 0;
	return qfalse;
}

/* ---- SEH_InitLanguage  0x004A9A60 ---- */
int SEH_InitLanguage( void ) {
	cl_language                 = Cvar_Get( "cl_language", "0", CVAR_ARCHIVE | CVAR_LATCH );
	cl_languagesavailable       = Cvar_Get( "cl_languagesavailable", "0", 0 );
	cl_languagetranslate        = Cvar_Get( "cl_languagetranslate", "1", CVAR_LATCH );
	cl_languagewarnings         = Cvar_Get( "cl_languagewarnings", "0", 0 );
	cl_languagewarningsaserrors = Cvar_Get( "cl_languagewarningsaserrors", "0", 0 );

	return SEH_UpdateCurrentLanguage();
}

/* SEH_LanguageHasAssets  0x004A9B65 */
static qboolean SEH_LanguageHasAssets( int language ) {
	seh_searchpath_t *search;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( search->localized && search->language == language ) {
			return qtrue;
		}
	}
	return qfalse;
}

/* ---- SEH_UpdateLanguageInfo  0x004A9B10 ---- */
void SEH_UpdateLanguageInfo( void ) {
	int i;
	int numLanguages;
	int result;

	SEH_UpdateCurrentLanguage();

	numLanguages = 0;
	for ( i = 0 ; i < NUM_LANGUAGES ; i++ ) {
		if ( SEH_LanguageHasAssets( i ) ) {
			seh_languagePresent[i] = 1;
			numLanguages++;
		} else {
			seh_languagePresent[i] = 0;
		}
	}

	if ( numLanguages < 1 ) {
		Com_Printf( "^1ERROR: No languages available because no localized assets were found\n" );
	}

	Cvar_Set2( "cl_languagesavailable", va( "%i", numLanguages ), qtrue );

	result = SEH_StringEd_SetLanguageStrings( cl_language->integer );
	if ( result ) {
		return;
	}

	for ( i = 0 ; i < NUM_LANGUAGES ; i++ ) {
		if ( !seh_languagePresent[i] ) {
			continue;
		}

		Cvar_Set2( "cl_language", va( "%i", i ), qtrue );
		SEH_UpdateCurrentLanguage();

		result = SEH_StringEd_SetLanguageStrings( i );
		if ( result ) {
			break;
		}
	}

	if ( i == NUM_LANGUAGES ) {
		Cvar_Set2( "cl_language", "0", qtrue );
		SEH_UpdateCurrentLanguage();
	}
}

/* ---- SEH_VerifyLanguageSelection  0x004A9C90 ---- */
int SEH_VerifyLanguageSelection( int language ) {
	int i;

	if ( (unsigned int)language >= NUM_LANGUAGES ) {
		language = 0;
	}

	if ( seh_languagePresent[language] ) {
		return language;
	}

	for ( i = 0 ; i < NUM_LANGUAGES ; i++ ) {
		if ( seh_languagePresent[( i + language ) % NUM_LANGUAGES] ) {
			return ( i + language ) % NUM_LANGUAGES;
		}
	}

	return 0;
}

/* ---- SEH_StringEd_SetLanguageStrings  0x004A9D00 ---- */
int SEH_StringEd_SetLanguageStrings( int language ) {
	const char  *name;
	char        *err;
	int         warn;

	Clear__16CStringEdPackageFi( (void *)&TheStringPackage, 1 );

	warn = ( fs_ignoreLocalized && !fs_ignoreLocalized->integer
			 && cl_languagewarnings && cl_languagewarnings->integer );

	if ( !SE_GetNumLanguages__Fv() ) {
		if ( warn ) {
			name = SEH_GetLanguageName( language );
			if ( cl_languagewarningsaserrors && cl_languagewarningsaserrors->integer ) {
				Com_Error( ERR_LOCALIZATION,
						   "No language string information available for %s", name );
			}
			Com_Printf( "^3WARNING: No language string information available for %s\n", name );
		}
		return qfalse;
	}

	err = SE_LoadAllLanguageFiles_m( SEH_GetLanguageName( language ), 1 );
	if ( !err ) {
		return qtrue;
	}

	if ( warn ) {
		name = SEH_GetLanguageName( language );
		if ( cl_languagewarningsaserrors && cl_languagewarningsaserrors->integer ) {
			Com_Error( ERR_LOCALIZATION,
					   "Could not load localization strings for %s: %s", name, err );
		}
		Com_Printf( "^3WARNING: Could not load localization strings for %s: %s\n", name, err );
	}
	return qfalse;
}

/* ---- SEH_StringEd_GetString  0x004A9E20 ---- */
char *SEH_StringEd_GetString( const char *key ) {
	if ( cl_languagetranslate && cl_languagetranslate->integer && key[0] && key[1] ) {
		return SE_GETSTRING( key, 0 );
	}
	return (char *)key;
}

/* ---- SEH_SafeTranslateString  0x004A9E50 ---- */
char *SEH_SafeTranslateString( const char *key ) {
	char *translated;

	translated = (char *)key;
	if ( cl_languagetranslate && cl_languagetranslate->integer && key[0] && key[1] ) {
		translated = SE_GETSTRING( key, 0 );
	}

	if ( translated ) {
		return translated;
	}

	if ( cl_languagewarnings && cl_languagewarnings->integer ) {
		if ( cl_languagewarningsaserrors && cl_languagewarningsaserrors->integer ) {
			Com_Error( ERR_LOCALIZATION, "Could not translate exe string \"%s\"", key );
		}
		Com_Printf( "^3WARNING: Could not translate exe string \"%s\"\n", key );
		Com_sprintf( szSafeTranslate, sizeof( szSafeTranslate ),
					 "^1UNLOCALIZED(^7%s^1)^7", key );
	} else {
		Q_strncpyz( szSafeTranslate, key, sizeof( szSafeTranslate ) );
	}

	return szSafeTranslate;
}

/* ---- SEH_GetLocalizedTokenReference  0x004A9F80 ---- */
static qboolean SEH_GetLocalizedTokenReference( int flag, const char *type,
												const char *token, char *out ) {
	char *translated;

	if ( cl_languagetranslate && cl_languagetranslate->integer && token[0] && token[1] ) {
		translated = SE_GETSTRING( token, 0 );
	} else {
		translated = (char *)token;
	}

	if ( !translated ) {
		if ( cl_languagewarnings && cl_languagewarnings->integer ) {
			if ( cl_languagewarningsaserrors && cl_languagewarningsaserrors->integer
				 && flag != LOCMSG_NOERR ) {
				Com_Error( ERR_LOCALIZATION,
						   "Could not translate part of %s: \"%s\"", type, token );
			}
			Com_Printf( "^3WARNING: Could not translate part of %s: \"%s\"\n", type, token );
			translated = va( "^1UNLOCALIZED(^7%s^1)^7", token );
		} else {
			translated = va( "%s", token );
		}

		if ( flag == LOCMSG_NOERR ) {
			return qfalse;
		}
	}

	if ( translated != out ) {
		strcpy( out, translated );
	}
	return qtrue;
}

/* ---- SEH_LocalizeTextMessage  0x004AA040 ---- */
char *SEH_LocalizeTextMessage( const char *text, const char *type, int flag ) {
	char        szTokenBuf[MAX_LOCALIZED_LEN];
	char        szInsertBuf[MAX_LOCALIZED_LEN];
	char        *out;
	const char  *in;
	const char  *tokStart;
	int         outLen;
	int         tokLen;
	int         insertLevel;
	int         tokenHasInsert;
	int         locOn;
	int         insertEnabled;
	int         locSkipped;
	int         at;
	int         i;

	iCurrString = ( iCurrString + 1 ) % 2;
	out = szStrings[iCurrString];
	memset( out, 0, MAX_LOCALIZED_LEN );

	szInsertBuf[0] = '\0';

	in             = text;
	tokStart       = text;
	outLen         = 0;
	tokLen         = 0;
	insertLevel    = 0;
	tokenHasInsert = 0;
	locOn          = 1;
	insertEnabled  = 1;
	locSkipped     = 0;

	while ( *tokStart ) {
		while ( *in && *in != SEH_MARK_LOC_ON && *in != SEH_MARK_LOC_OFF
				&& *in != SEH_MARK_NO_INSERT ) {
			in++;
		}

		if ( in > tokStart ) {
			tokLen = in - tokStart;
			strncpy( szTokenBuf, tokStart, tokLen );
			szTokenBuf[tokLen] = '\0';

			if ( locOn ) {
				if ( !SEH_GetLocalizedTokenReference( flag, type, szTokenBuf, szTokenBuf ) ) {
					return NULL;
				}
				tokLen = strlen( szTokenBuf );
			}

			if ( tokLen + outLen >= MAX_LOCALIZED_LEN ) {
				if ( cl_languagewarnings && cl_languagewarnings->integer
					 && cl_languagewarningsaserrors && cl_languagewarningsaserrors->integer
					 && flag != LOCMSG_NOERR ) {
					Com_Error( ERR_DROP, "%s too long when translated: \"%s\"", type, text );
				}
				Com_Printf( "%s too long when translated: \"%s\"\n", type, text );
			}

			for ( i = 0 ; i < tokLen - 1 ; i++ ) {
				if ( szTokenBuf[i] != '%' || szTokenBuf[i + 1] != 's' ) {
					continue;
				}
				if ( insertEnabled && !tokenHasInsert ) {
					insertLevel++;
					tokenHasInsert = 1;
				} else {
					szTokenBuf[i] = SEH_MARK_NO_INSERT;
					locSkipped = 1;
				}
			}

			if ( insertLevel == tokenHasInsert ) {
				strcpy( &out[outLen], szTokenBuf );
			} else {
				at = 0;
				if ( outLen - 1 > 0 ) {
					for ( at = 0 ; at < outLen - 1 ; at++ ) {
						if ( out[at] == '%' && out[at + 1] == 's' ) {
							strcpy( szInsertBuf, &out[at + 2] );
							out[at] = '\0';
							break;
						}
					}
				}
				strcpy( &out[at], szTokenBuf );
				strcpy( &out[at + tokLen], szInsertBuf );
				outLen -= 2;
				insertLevel--;
			}

			outLen += tokLen;
			tokenHasInsert = 0;
		}

		insertEnabled = 1;
		if ( *in == SEH_MARK_LOC_ON ) {
			locOn = 1;
			in++;
		} else if ( *in == SEH_MARK_LOC_OFF ) {
			locOn = 0;
			in++;
		}
		if ( *in == SEH_MARK_NO_INSERT ) {
			in++;
			insertEnabled = 0;
		}

		tokStart = in;
	}

	if ( locSkipped ) {
		for ( i = 0 ; i < outLen ; i++ ) {
			if ( out[i] == SEH_MARK_NO_INSERT ) {
				out[i] = '%';
			}
		}
	}

	return out;
}
