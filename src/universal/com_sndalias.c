/*
 * universal/com_sndalias.c
 *
 * Retail range 0x00432800-0x004353B0, 35 functions.
 *
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"

extern cvar_t   *fs_homepath;
extern cvar_t   *fs_basepath;
extern char     fs_gamedir[MAX_OSPATH];

extern char     **FS_ListFilteredFiles( const char *path, const char *extension,
										char *filter, int *numfiles );
extern int      FS_FOpenFileRead_Internal( const char *qpath, fileHandle_t *file,
										   qboolean uniqueFILE, int filter_flag );
extern qboolean FS_FileExists( const char *file );
extern void     FS_BuildOSPath3( const char *base, const char *game,
								 const char *qpath, char *fs_path,
								 int allowOverflow );

extern char     *Com_Parse( char **data_p );
extern char     *Com_ParseOnLine( char **data_p );
extern void     Com_SkipRestOfLine( int unused, char **data_p );
extern void     Com_SetCSV( qboolean enabled );
extern void     Com_UngetToken( void );

extern void     Hunk_SetLowTempMark( void );
extern void     Hunk_ClearToLowTempMark( void );

extern void     Sys_OutOfMemoryPrep( void );

extern void     *MSS_LoadSoundFile( const char *name );
extern void     MSS_StopSounds( int flag );

typedef enum {
	SND_COL_UNKNOWN     = 0,
	SND_COL_NAME        = 1,
	SND_COL_SEQUENCE    = 2,
	SND_COL_FILE        = 3,
	SND_COL_SUBTITLE    = 4,
	SND_COL_VOL_MIN     = 5,
	SND_COL_VOL_MAX     = 6,
	SND_COL_PITCH_MIN   = 7,
	SND_COL_PITCH_MAX   = 8,
	SND_COL_DIST_MIN    = 9,
	SND_COL_DIST_MAX    = 10,
	SND_COL_CHANNEL     = 11,
	SND_COL_TYPE        = 12,
	SND_COL_LOOP        = 13,
	SND_COL_PROBABILITY = 14,
	SND_COL_LOADSPEC    = 15,
	SND_COL_MASTERSLAVE = 16,

	SND_COL_COUNT       = 17
} sndAliasColumn_t;

typedef enum {
	SND_TYPE_UNKNOWN  = 0,
	SND_TYPE_LOADED   = 1,
	SND_TYPE_STREAMED = 2
} sndAliasType_t;

typedef enum {
	SND_CHANNEL_AUTO       = 0,
	SND_CHANNEL_MENU       = 1,
	SND_CHANNEL_WEAPON     = 2,
	SND_CHANNEL_VOICE      = 3,
	SND_CHANNEL_ITEM       = 4,
	SND_CHANNEL_BODY       = 5,
	SND_CHANNEL_LOCAL      = 6,
	SND_CHANNEL_MUSIC      = 7,
	SND_CHANNEL_ANNOUNCER  = 8,
	SND_CHANNEL_SHELLSHOCK = 9,

	SND_CHANNEL_COUNT      = 10
} sndAliasChannel_t;

typedef struct snd_alias_s {
	const char          *name;              /* +0x00 shared with the whole sequence group */
	const char          *file;              /* +0x04 shared with every alias naming the same wav */
	const char          *subtitle;          /* +0x08 NULL when the CSV cell was empty */
	void                *soundData;         /* +0x0C MSS_LoadSoundFile result, loaded aliases only */
	int                 pickSequence;       /* +0x10 anti-repeat counter, bumped by Com_PickSoundAlias */
	float               volMin;             /* +0x14 */
	float               volMax;             /* +0x18 */
	float               pitchMin;           /* +0x1C */
	float               pitchMax;           /* +0x20 */
	float               distMin;            /* +0x24 */
	float               distMax;            /* +0x28 */
	int                 channel;            /* +0x2C sndAliasChannel_t */
	int                 type;               /* +0x30 sndAliasType_t */
	byte                loop;               /* +0x34 */
	byte                isMaster;           /* +0x35 */
	byte                isSlave;            /* +0x36 */
	byte                streamExists;       /* +0x37 streamed aliases only */
	float               slavePercent;       /* +0x38 */
	float               probability;        /* +0x3C selection weight */
	struct snd_alias_s  *hashNext;          /* +0x40 */
} snd_alias_t;

typedef struct snd_aliasBuild_s {
	char                    sourceFile[64]; /* +0x0000 the CSV this row came from */
	char                    name[64];       /* +0x0040 */
	char                    subtitle[4096]; /* +0x0080 */
	int                     sequence;       /* +0x1080 */
	char                    file[64];       /* +0x1084 */
	const char              *compactFile;   /* +0x10C4 where file ended up in the string block */
	float                   volMin;         /* +0x10C8 */
	float                   volMax;         /* +0x10CC */
	float                   pitchMin;       /* +0x10D0 */
	float                   pitchMax;       /* +0x10D4 */
	float                   distMin;        /* +0x10D8 */
	float                   distMax;        /* +0x10DC */
	int                     channel;        /* +0x10E0 */
	int                     type;           /* +0x10E4 */
	byte                    loop;           /* +0x10E8 */
	byte                    isMaster;       /* +0x10E9 */
	byte                    isSlave;        /* +0x10EA */
	byte                    pad_10EB;       /* +0x10EB */
	float                   slavePercent;   /* +0x10EC */
	float                   probability;    /* +0x10F0 */
	int                     loadspec;       /* +0x10F4 zero means "not in this locale" */
	struct snd_aliasBuild_s *dupFileNode;   /* +0x10F8 earlier node naming the same wav */
	struct snd_aliasBuild_s *next;          /* +0x10FC */
} snd_aliasBuild_t;

#include "com_sndalias.h"

#define SND_ALIAS_MAX_COLUMNS       256
#define SND_ALIAS_HASH_MULTIPLIER   31337
#define SND_ALIAS_LOADSPEC_MAX      65535

#define SUBTITLE_FILE               "soundaliases/subtitle.st"
#define SUBTITLE_PREFIX             "SUBTITLE_"
#define SUBTITLE_PREFIX_LEN         9

static const char *snd_aliasColumnNames[SND_COL_COUNT] = {
	NULL,        "name",      "sequence",    "file",
	"subtitle",  "vol_min",   "vol_max",     "pitch_min",
	"pitch_max", "dist_min",  "dist_max",    "channel",
	"type",      "loop",      "probability", "loadspec",
	"masterslave"
};

/* 0x0057ACEC */
static const char *snd_aliasChannelNames[SND_CHANNEL_COUNT] = {
	"auto", "menu",  "weapon",    "voice", "item",
	"body", "local", "music",     "announcer", "shellshock"
};

const char          *snd_currentFile;                                   /* 0x008931D0 */
byte                snd_aliasSetLoaded[SND_ALIAS_LOADED_SLOTS];         /* 0x008931D4 */
snd_alias_t         *snd_aliasHashTable[SND_ALIAS_LOCALE_COUNT][SND_ALIAS_HASH_SIZE];
																		/* 0x008931D8 */
snd_alias_t         *snd_aliasTable[SND_ALIAS_LOCALE_COUNT];            /* 0x00893DD8 */
int                 snd_aliasTableCount[SND_ALIAS_LOCALE_COUNT];        /* 0x00893DE4 */
snd_aliasBuild_t    *snd_aliasBuildHead;                                /* 0x00893DF0 */
char                snd_aliasLocalizedName[64];                         /* 0x00893DF4 */
int                 snd_aliasTableChecksum[SND_ALIAS_LOCALE_COUNT];     /* 0x00893E34 */
char                snd_subtitleReference[1024];                        /* 0x00893E40 */

static void  Com_LoadedSoundList( int source );
static void  Com_StreamedSoundList( int source );
int   Com_LoadSoundAliasFile( const char *csvPath, const char *sourceName,
									 int count, int source );
void  Com_MakeSoundAliasesPermanent( int count, int source );
static snd_aliasBuild_t *Com_SortTempSoundAliases_r( snd_aliasBuild_t *head, int *count );

/* ---- Com_LoadedSoundList  0x00432800 ---- */
static void Com_LoadedSoundList( int source ) {
	snd_alias_t *table;
	int i, j;
	int total;
	int size;

	if ( !snd_aliasSetLoaded[source] ) {
		return;
	}

	table = snd_aliasTable[source];
	total = 0;

	for ( i = 0 ; i < snd_aliasTableCount[source] ; i++ ) {
		if ( table[i].type != SND_TYPE_LOADED ) {
			continue;
		}
		for ( j = 0 ; j < i ; j++ ) {
			if ( table[j].type == SND_TYPE_LOADED && table[j].file == table[i].file ) {
				break;
			}
		}
		if ( j < i ) {
			continue;
		}

		if ( table[i].soundData ) {
			size = *( (int *) table[i].soundData + 2 ) + 36;
			total += size;
			Com_Printf( "%-64s %7.1f KB\n", table[i].file, (double) size / 1024.0 );
		} else {
			Com_Printf( "%-64s FAILED TO LOAD\n", table[i].file );
		}
	}

	Com_Printf( "\ntotal usage %7.3f MB\n", (double) total / ( 1024.0 * 1024.0 ) );
}

static void Com_StreamedSoundList( int source ) {
	snd_alias_t *table;
	int i, j;

	if ( !snd_aliasSetLoaded[source] ) {
		return;
	}

	table = snd_aliasTable[source];

	for ( i = 0 ; i < snd_aliasTableCount[source] ; i++ ) {
		if ( table[i].type != SND_TYPE_STREAMED ) {
			continue;
		}
		for ( j = 0 ; j < i ; j++ ) {
			if ( table[j].type == SND_TYPE_STREAMED && table[j].file == table[i].file ) {
				break;
			}
		}
		if ( j < i ) {
			continue;
		}

		if ( table[i].streamExists ) {
			Com_Printf( "%-64s\n", table[i].file );
		} else {
			Com_Printf( "%-64s FILE NOT FOUND\n", table[i].file );
		}
	}
}

/* ---- Com_SoundList_f  0x00432980 ---- */
void Com_SoundList_f( void ) {
	Com_Printf( "\n________________________________________\ncurrently streamed menu sounds:\n" );
	Com_StreamedSoundList( SND_LOCALE_MENU );
	Com_Printf( "\n________________________________________\ncurrently streamed in-game sounds:\n" );
	Com_StreamedSoundList( SND_LOCALE_INGAME );
	Com_Printf( "________________________________________\ncurrently loaded menu sounds:\n" );
	Com_LoadedSoundList( SND_LOCALE_MENU );
	Com_Printf( "\n________________________________________\ncurrently loaded in-game sounds:\n" );
	Com_LoadedSoundList( SND_LOCALE_INGAME );
	Com_Printf( "\n" );
}

/* ---- Com_HashAliasName  0x004329E0 ---- VERIFIED */
int Com_HashAliasName( const char *name ) {
	unsigned int hash;

	hash = 0;
	while ( *name ) {
		hash = (unsigned int) tolower( *(const signed char *) name )
			   + hash * SND_ALIAS_HASH_MULTIPLIER;
		name++;
	}
	return (int) ( hash & ( SND_ALIAS_HASH_SIZE - 1 ) );
}

/* ---- Com_IsValidAliasName  0x00432A20 ---- VERIFIED */
qboolean Com_IsValidAliasName( const char *name ) {
	signed char c;

	c = (signed char) *name;
	if ( c <= 31 || ( !isalpha( c ) && c != '_' ) ) {
		return qfalse;
	}

	for ( ;; ) {
		name++;
		c = (signed char) *name;
		if ( !c ) {
			return qtrue;
		}
		if ( c <= 31 || ( !isalnum( c ) && c != '_' ) ) {
			return qfalse;
		}
	}
}

/* ---- Com_FindSoundAlias  0x00432A80 ---- VERIFIED */
snd_alias_t *Com_FindSoundAlias( const char *name, int source ) {
	snd_alias_t *alias;

	if ( !name ) {
		return NULL;
	}

	for ( alias = snd_aliasHashTable[source][Com_HashAliasName( name )] ;
		  alias ;
		  alias = alias->hashNext ) {
		if ( alias->name && !Q_stricmp( name, alias->name ) ) {
			return alias;
		}
	}
	return NULL;
}

/* ---- Com_LoadSoundAliasDefaults  0x00432AD0 ---- VERIFIED */
static void Com_LoadSoundAliasDefaults( snd_aliasBuild_t *node, qboolean defaultLoadspec ) {
	strcpy( node->sourceFile, snd_currentFile );
	node->name[0]      = '\0';
	node->subtitle[0]  = '\0';
	node->sequence     = 0;
	node->file[0]      = '\0';
	node->volMin       = 1.0f;
	node->volMax       = 1.0f;
	node->pitchMin     = 1.0f;
	node->pitchMax     = 1.0f;
	node->distMin      = 120.0f;
	node->distMax      = 0.0f;
	node->channel      = SND_CHANNEL_AUTO;
	node->type         = SND_TYPE_LOADED;
	node->loop         = 0;
	node->isMaster     = 0;
	node->isSlave      = 0;
	node->slavePercent = 1.0f;
	node->probability  = 1.0f;
	node->loadspec     = ( defaultLoadspec != qfalse );
	node->next         = NULL;
}

/* ---- Com_SoundAliasChannelForName  0x00432B80 ---- VERIFIED */
static int Com_SoundAliasChannelForName( const char *name ) {
	char buffer[16384];
	int length;
	int i;

	for ( i = 0 ; i < SND_CHANNEL_COUNT ; i++ ) {
		if ( name && snd_aliasChannelNames[i]
			 && !Q_stricmp( name, snd_aliasChannelNames[i] ) ) {
			return i;
		}
	}

	length = 0;
	for ( i = 0 ; i < SND_CHANNEL_COUNT ; i++ ) {
		length += sprintf( buffer + length, "%s", snd_aliasChannelNames[i] );
		if ( i < SND_CHANNEL_ANNOUNCER ) {
			length += sprintf( buffer + length, ", " );
		} else if ( i == SND_CHANNEL_ANNOUNCER ) {
			length += sprintf( buffer + length, " or " );
		}
	}

	Com_Error( ERR_DROP,
			   "\x15Sound alias file %s: Unknown sound channel '%s'; should be %s\n",
			   snd_currentFile, name, buffer );
	return SND_CHANNEL_AUTO;
}

/* ---- Com_SoundAliasTypeForName  0x00432C70 ---- VERIFIED */
static int Com_SoundAliasTypeForName( const char *name ) {
	if ( name ) {
		if ( !Q_stricmp( name, "streamed" ) ) {
			return SND_TYPE_STREAMED;
		}
		if ( !Q_stricmp( name, "loaded" ) ) {
			return SND_TYPE_LOADED;
		}
	}

	Com_Error( ERR_DROP,
			   "\x15Sound alias file %s: Unknown sound type '%s'; should be streamed or loaded\n",
			   snd_currentFile, name );
	return SND_TYPE_UNKNOWN;
}

static qboolean Com_SoundAliasLoop( const char *value ) {
	if ( value ) {
		if ( !Q_stricmp( value, "looping" ) ) {
			return qtrue;
		}
		if ( !Q_stricmp( value, "nonlooping" ) ) {
			return qfalse;
		}
	}

	Com_Error( ERR_DROP,
			   "\x15Sound alias file %s: Unknown sound looping type '%s'; "
			   "should be looping or nonlooping\n",
			   snd_currentFile, value );
	return qfalse;
}

/* ---- Com_SoundAliasLoadSpec  0x00432D20 ---- VERIFIED */
static int Com_SoundAliasLoadSpec( const char *sourceName, const char *value ) {
	char spec[SND_ALIAS_LOADSPEC_MAX + 1];
	unsigned int sourceLen;
	char *cursor;
	char *match;

	sourceLen = strlen( sourceName );
	spec[SND_ALIAS_LOADSPEC_MAX] = '\0';
	strncpy( spec, value, SND_ALIAS_LOADSPEC_MAX + 1 );
	if ( spec[SND_ALIAS_LOADSPEC_MAX] ) {
		Com_Error( ERR_DROP,
				   "\x15Sound alias file %s: loadspec is > %i characters\n",
				   snd_currentFile, SND_ALIAS_LOADSPEC_MAX );
	}

	Q_strlwr( spec );
	cursor = spec;

	if ( spec[0] == '!' ) {
		do {
			cursor++;
			if ( *cursor > ' ' ) {
				break;
			}
		} while ( *cursor );

		if ( !strcmp( cursor, sourceName ) ) {
			return 0;
		}
		return strcmp( cursor, "all_mp" ) != 0;
	}

	match = strstr( spec, sourceName );
	while ( match ) {
		if ( ( match == spec || *( match - 1 ) <= ' ' ) && match[sourceLen] <= ' ' ) {
			return 1;
		}
		cursor = match + 1;
		match = strstr( cursor, sourceName );
	}

	return strcmp( cursor, "all_mp" ) == 0;
}

/* ---- Com_SoundAliasMasterSlave  0x00432E70 ---- VERIFIED */
static void Com_SoundAliasMasterSlave( const char *value, snd_aliasBuild_t *node ) {
	if ( !Q_stricmp( value, "master" ) ) {
		node->isMaster = 1;
		node->isSlave  = 0;
	} else {
		node->isMaster     = 0;
		node->isSlave      = 1;
		node->slavePercent = (float) atof( value );
	}
}

/* ---- Com_LoadSoundAliasField  0x00432EB0 ---- VERIFIED */
static void Com_LoadSoundAliasField( byte *seen, const char *value, int column,
									 const char *sourceName, snd_aliasBuild_t *node ) {
	float f;
	const char *p;

	if ( column == SND_COL_UNKNOWN ) {
		return;
	}

	if ( seen[column] ) {
		Com_Error( ERR_DROP,
				   "\x15Sound alias file %s: Duplicate entries for the '%s' column\n",
				   snd_currentFile, snd_aliasColumnNames[column] );
	}
	seen[column] = 1;

	switch ( column ) {
	case SND_COL_NAME:
		if ( strlen( value ) >= sizeof( node->name ) - 1 ) {
			Com_Error( ERR_DROP,
					   "\x15Sound alias file %s: Alias name '%s' is longer than %i characters\n",
					   snd_currentFile, value, sizeof( node->name ) - 1 );
		}
		if ( Com_IsValidAliasName( value ) == qfalse ) {
			Com_Error( ERR_DROP,
					   "\x15Sound alias file %s: Alias name '%s' is invalid\n",
					   snd_currentFile, value );
		}
		strcpy( node->name, value );
		break;

	case SND_COL_SEQUENCE:
		node->sequence = atoi( value );
		break;

	case SND_COL_FILE:
		if ( strlen( value ) >= sizeof( node->file ) - 1 ) {
			Com_Error( ERR_DROP,
					   "\x15Sound alias file %s: Sound file '%s' is longer than %i characters\n",
					   snd_currentFile, value, sizeof( node->file ) - 1 );
		}
		strcpy( node->file, value );
		break;

	case SND_COL_SUBTITLE:
		if ( strlen( value ) >= sizeof( node->subtitle ) - 1 ) {
			Com_Error( ERR_DROP,
					   "\x15Sound alias file %s: Subtitle '%s' is longer than %i characters\n",
					   snd_currentFile, value, sizeof( node->subtitle ) - 1 );
		}
		for ( p = value ; *p ; p++ ) {
			if ( *(const signed char *) p < 0 ) {
				Com_Error( ERR_DROP,
						   "\x15Sound alias file %s: Subtitle '%s' has invalid character "
						   "'%c' ascii %i\n",
						   snd_currentFile, value, *(const signed char *) p,
						   *(const unsigned char *) p );
			}
		}
		strcpy( node->subtitle, value );
		break;

	case SND_COL_VOL_MIN:
		f = (float) atof( value );
		node->volMin = f;
		if ( !seen[SND_COL_VOL_MAX] ) {
			node->volMax = f;
		}
		break;

	case SND_COL_VOL_MAX:
		node->volMax = (float) atof( value );
		break;

	case SND_COL_PITCH_MIN:
		f = (float) atof( value );
		node->pitchMin = f;
		if ( !seen[SND_COL_PITCH_MAX] ) {
			node->pitchMax = f;
		}
		break;

	case SND_COL_PITCH_MAX:
		node->pitchMax = (float) atof( value );
		break;

	case SND_COL_DIST_MIN:
		node->distMin = (float) atof( value );
		break;

	case SND_COL_DIST_MAX:
		node->distMax = (float) atof( value );
		break;

	case SND_COL_CHANNEL:
		node->channel = Com_SoundAliasChannelForName( value );
		break;

	case SND_COL_TYPE:
		node->type = Com_SoundAliasTypeForName( value );
		break;

	case SND_COL_LOOP:
		node->loop = (byte) Com_SoundAliasLoop( value );
		break;

	case SND_COL_PROBABILITY:
		node->probability = (float) atof( value );
		break;

	case SND_COL_LOADSPEC:
		node->loadspec = Com_SoundAliasLoadSpec( sourceName, value );
		break;

	case SND_COL_MASTERSLAVE:
		Com_SoundAliasMasterSlave( value, node );
		break;

	default:
		break;
	}
}

/* ---- Com_FinishBuildingSoundAlias  0x00433190 ---- VERIFIED */
static void Com_FinishBuildingSoundAlias( snd_aliasBuild_t *node ) {
	float tmp;

	if ( node->pitchMin > node->pitchMax ) {
		tmp = node->pitchMin;
		node->pitchMin = node->pitchMax;
		node->pitchMax = tmp;
	}
	if ( node->pitchMin <= 0.0f ) {
		Com_Error( ERR_DROP, "\x15sound alias '%s' has pitch_min %g <= 0\n",
				   node->name, node->pitchMin );
	}

	if ( node->volMin > node->volMax ) {
		tmp = node->volMin;
		node->volMin = node->volMax;
		node->volMax = tmp;
	}
	if ( !( node->volMin >= 0.0f ) ) {
		Com_Error( ERR_DROP, "\x15sound alias '%s' has vol_min < 0\n",
				   node->name, node->volMin );
	}

	if ( !( node->distMax != 0.0f ) ) {
		node->distMax = node->distMin * 5.0f;
	}
	if ( !( node->distMax >= node->distMin ) ) {
		Com_Error( ERR_DROP, "\x15sound alias '%s' has dist_min %g <= dist_max %g\n",
				   node->name, node->distMin, node->distMax );
	}
	if ( node->distMin <= 0.0f ) {
		Com_Error( ERR_DROP, "\x15sound alias '%s' has dist_min <= 0\n",
				   node->name, node->distMin );
	}
}

/* ---- Com_AddBuildSoundAlias  0x004332F0 ---- */
static snd_aliasBuild_t *Com_AddBuildSoundAlias( const snd_aliasBuild_t *node ) {
	snd_aliasBuild_t *clone;

	clone = (snd_aliasBuild_t *) Hunk_AllocateTempMemoryInternal( sizeof( *clone ) );
	memcpy( clone, node, sizeof( *clone ) );
	clone->next = snd_aliasBuildHead;
	snd_aliasBuildHead = clone;
	return clone;
}

/* ---- Com_AddSoundAlias  0x00433320 ---- VERIFIED */
static void Com_AddSoundAlias( const snd_aliasBuild_t *node, snd_alias_t *alias,
							   const char *name, const char *file,
							   const char *subtitle, int source ) {
	int bucket;

	alias->name         = name;
	alias->file         = file;
	alias->subtitle     = subtitle;
	alias->soundData    = NULL;
	alias->pickSequence = 0;
	alias->volMin       = node->volMin;
	alias->volMax       = node->volMax;
	alias->pitchMin     = node->pitchMin;
	alias->pitchMax     = node->pitchMax;
	alias->distMin      = node->distMin;
	alias->distMax      = node->distMax;
	alias->channel      = node->channel;
	alias->type         = node->type;
	alias->loop         = node->loop;
	alias->isMaster     = node->isMaster;
	alias->isSlave      = node->isSlave;
	alias->slavePercent = node->slavePercent;
	alias->probability  = node->probability;

	bucket = Com_HashAliasName( name );
	alias->hashNext = snd_aliasHashTable[source][bucket];
	snd_aliasHashTable[source][bucket] = alias;
}

/* ---- Com_LoadSoundAliasFile  0x004333D0 ---- VERIFIED */
int Com_LoadSoundAliasFile( const char *csvPath, const char *sourceName,
								   int count, int source ) {
	void                *buffer;
	char                *data_p;
	char                *token;
	int                 columnMap[SND_ALIAS_MAX_COLUMNS];
	byte                seen[SND_COL_COUNT];
	snd_aliasBuild_t    node;
	int                 columnCount;
	int                 column;
	int                 i;
	qboolean            hasName, hasFile;

	if ( FS_ReadFile( csvPath, &buffer ) < 0 ) {
		return count;
	}

	Com_BeginParseSession( csvPath );
	Com_SetCSV( qtrue );
	data_p = (char *) buffer;
	columnCount = 0;

	for ( ;; ) {
		token = Com_Parse( &data_p );
		if ( !data_p ) {
			break;
		}

		if ( !token[0] || token[0] == '#' ) {
			Com_SkipRestOfLine( 0, &data_p );
			continue;
		}

		if ( columnCount == 0 ) {
			hasName = qfalse;
			hasFile = qfalse;

			for ( ;; ) {
				columnMap[columnCount] = SND_COL_UNKNOWN;
				for ( column = SND_COL_NAME ; column < SND_COL_COUNT ; column++ ) {
					if ( snd_aliasColumnNames[column] && token
						 && !Q_stricmp( snd_aliasColumnNames[column], token ) ) {
						columnMap[columnCount] = column;
						if ( column == SND_COL_NAME ) {
							hasName = qtrue;
						} else if ( column == SND_COL_FILE ) {
							hasFile = qtrue;
						}
						break;
					}
				}

				columnCount++;
				if ( columnCount == SND_ALIAS_MAX_COLUMNS || !data_p || *data_p == '\n' ) {
					break;
				}
				token = Com_ParseOnLine( &data_p );
			}

			if ( !hasName || !hasFile ) {
				Com_Error( ERR_DROP,
						   "\x15Sound alias file %s: missing 'name' and/or 'file' columns\n",
						   snd_currentFile );
			}

			Com_SkipRestOfLine( 0, &data_p );
			continue;
		}

		memset( seen, 0, sizeof( seen ) );
		Com_LoadSoundAliasDefaults( &node, source );

		i = 0;
		for ( ;; ) {
			if ( token[0] ) {
				Com_LoadSoundAliasField( seen, token, columnMap[i], sourceName, &node );
			}
			if ( ++i == columnCount ) {
				break;
			}
			token = Com_ParseOnLine( &data_p );
		}

		if ( !seen[SND_COL_NAME] || !seen[SND_COL_FILE] ) {
			Com_Error( ERR_DROP,
					   "\x15Sound alias file %s: alias entry missing name and/or file\n",
					   snd_currentFile );
		}

		if ( node.loadspec ) {
			Com_FinishBuildingSoundAlias( &node );
			Com_AddBuildSoundAlias( &node );
			count++;
		}

		Com_SkipRestOfLine( 0, &data_p );
	}

	Com_EndParseSession();
	return count;
}

/* ---- Com_SortTempSoundAliases_r  0x00433750 ---- VERIFIED */
static snd_aliasBuild_t *Com_SortTempSoundAliases_r( snd_aliasBuild_t *head, int *count ) {
	int                 leftCount, rightCount;
	int                 i;
	snd_aliasBuild_t    *rightHead;
	snd_aliasBuild_t    *left, *right;
	snd_aliasBuild_t    *merged, *tail, *pick;
	int                 cmp, fileCmp;

	if ( *count == 1 ) {
		head->next = NULL;
		return head;
	}

	leftCount  = *count / 2;
	rightCount = *count - leftCount;

	rightHead = head;
	for ( i = leftCount ; i > 0 ; i-- ) {
		rightHead = rightHead->next;
	}

	left  = Com_SortTempSoundAliases_r( head, &leftCount );
	right = Com_SortTempSoundAliases_r( rightHead, &rightCount );

	*count = 0;
	merged = NULL;
	tail   = NULL;

	while ( leftCount && rightCount ) {
		cmp = Q_stricmp( left->name, right->name );
		if ( cmp == 0 ) {
			cmp = left->sequence - right->sequence;
			if ( cmp == 0 ) {
				fileCmp = Q_stricmp( left->sourceFile, right->sourceFile );
				if ( fileCmp == 0 ) {
					Com_Error( ERR_DROP,
							   "\x15sound alias file %s: duplicate alias '%s'\n",
							   left->sourceFile, left->name );
					cmp = 0;
				} else if ( fileCmp < 0 ) {
					left = left->next;
					leftCount--;
					continue;
				} else {
					right = right->next;
					rightCount--;
					continue;
				}
			}
		}

		if ( cmp < 0 ) {
			pick = left;
			left = left->next;
			leftCount--;
		} else {
			pick = right;
			right = right->next;
			rightCount--;
		}

		if ( !merged ) {
			merged = pick;
		} else {
			tail->next = pick;
		}
		tail = pick;
		( *count )++;
	}

	if ( leftCount == 0 ) {
		if ( !merged ) {
			merged = right;
		} else {
			tail->next = right;
		}
		*count += rightCount;
	} else {
		if ( !merged ) {
			merged = left;
		} else {
			tail->next = left;
		}
		*count += leftCount;
	}

	return merged;
}

/* ---- Com_MakeSoundAliasesPermanent  0x00433900 ---- VERIFIED */
void Com_MakeSoundAliasesPermanent( int count, int source ) {
	snd_aliasBuild_t    *node, *scan;
	snd_alias_t         *table;
	char                *stringBase, *stringCursor;
	const char          *currentName;
	const char          *file;
	const char          *subtitle;
	int                 stringBytes, sharedBytes;
	int                 recordIndex;
	int                 byteIndex;
	unsigned int        checksum;
	size_t              length;
	size_t              blockSize;

	snd_aliasBuildHead = Com_SortTempSoundAliases_r( snd_aliasBuildHead, &count );

	stringBytes = 0;
	sharedBytes = 0;
	currentName = NULL;

	for ( node = snd_aliasBuildHead ; node ; node = node->next ) {
		length = strlen( node->name ) + 1;
		if ( !currentName || Q_stricmp( currentName, node->name ) ) {
			stringBytes += (int) length;
			currentName = node->name;
		} else {
			sharedBytes += (int) length;
		}

		if ( node->subtitle[0] ) {
			stringBytes += (int) strlen( node->subtitle ) + 1;
		}

		length = strlen( node->file ) + 1;
		node->dupFileNode = NULL;
		for ( scan = snd_aliasBuildHead ; scan != node ; scan = scan->next ) {
			if ( !Q_stricmp( scan->file, node->file ) ) {
				sharedBytes += (int) length;
				node->dupFileNode = scan;
				break;
			}
		}
		if ( !node->dupFileNode ) {
			stringBytes += (int) length;
		}
	}

	Com_DPrintf( "Sound alias strings use %.1f KB; %.1f KB saved by string sharing\n",
				 (double) stringBytes / 1024.0, (double) sharedBytes / 1024.0 );

	blockSize = (size_t) count * sizeof( snd_alias_t ) + (size_t) stringBytes;
	table = (snd_alias_t *) malloc( blockSize );
	if ( !table ) {
		Sys_OutOfMemoryPrep();
		Sys_Error( "Com_MakeSoundAliasesPermanent: failed to allocate %i bytes",
				   (int) blockSize );
		return;
	}
	Com_Memset( table, 0, blockSize );

	stringBase   = (char *) &table[count];
	stringCursor = stringBase;
	currentName  = NULL;
	recordIndex  = 0;

	for ( node = snd_aliasBuildHead ; node ; node = node->next ) {
		if ( !currentName || Q_stricmp( currentName, node->name ) ) {
			currentName = stringCursor;
			strcpy( stringCursor, node->name );
			stringCursor += strlen( stringCursor ) + 1;
		}

		if ( !node->subtitle[0] ) {
			subtitle = NULL;
		} else {
			subtitle = stringCursor;
			strcpy( stringCursor, node->subtitle );
			stringCursor += strlen( stringCursor ) + 1;
		}

		if ( !node->dupFileNode ) {
			file = stringCursor;
			strcpy( stringCursor, node->file );
			stringCursor += strlen( stringCursor ) + 1;
		} else {
			file = node->dupFileNode->compactFile;
		}
		node->compactFile = file;

		Com_AddSoundAlias( node, &table[recordIndex], currentName, file, subtitle, source );
		recordIndex++;
	}

	snd_aliasTable[source]      = table;
	snd_aliasTableCount[source] = count;

	checksum = (unsigned int) count * (unsigned int) stringBytes;
	snd_aliasTableChecksum[source] = (int) checksum;
	for ( byteIndex = 0 ; byteIndex < stringBytes ; byteIndex++ ) {
		checksum  = (unsigned int) snd_aliasTableChecksum[source] * SND_ALIAS_HASH_MULTIPLIER;
		checksum += (unsigned int) (signed char) stringBase[byteIndex];
		snd_aliasTableChecksum[source] = (int) checksum;
	}
}

#ifndef DEDICATED
/* ---- Com_LoadSoundAliasSounds  0x00433C20 ---- */
int Com_LoadSoundAliasSounds( int source ) {
	extern cvar_t *mss_errorOnMissing;

	snd_alias_t     *table;
	snd_alias_t     *alias;
	fileHandle_t    f;
	const char      *path;
	int             i, j;
	int             missing;
	qboolean        warned;

	missing = 0;
	table   = snd_aliasTable[source];

	for ( i = 0 ; i < snd_aliasTableCount[source] ; i++ ) {
		alias  = &table[i];
		warned = qfalse;

		if ( alias->type == SND_TYPE_LOADED ) {
			for ( j = 0 ; j < i ; j++ ) {
				if ( table[j].file != alias->file ) {
					continue;
				}
				if ( table[j].type == SND_TYPE_LOADED ) {
					alias->soundData = table[j].soundData;
					goto nextAlias;
				}
				if ( !warned && Q_stricmp( alias->file, "temp.wav" ) ) {
					warned = qtrue;
					Com_Printf( "WARNING: sound file '%s' used as streamed in alias '%s' "
								"and loaded in alias '%s'\n",
								alias->file, table[j].name, alias->name );
				}
			}

			alias->soundData = MSS_LoadSoundFile( alias->file );
			if ( alias->soundData ) {
				goto nextAlias;
			}
			Com_Printf( "WARNING: loaded sound file 'sound/%s' couldn't be read\n",
						alias->file );
			missing++;
		} else {
			for ( j = 0 ; j < i ; j++ ) {
				if ( table[j].file != alias->file ) {
					continue;
				}
				if ( table[j].type == SND_TYPE_STREAMED ) {
					alias->streamExists = table[j].streamExists;
					goto nextAlias;
				}
				if ( !warned && Q_stricmp( alias->file, "temp.wav" ) ) {
					warned = qtrue;
					Com_Printf( "WARNING: sound file '%s' used as streamed in alias '%s' "
								"and loaded in alias '%s'\n",
								alias->file, alias->name, table[j].name );
				}
			}

			path = va( "sound/%s", alias->file );
			fs_loadingMode = 1;
			f = 0;
			FS_FOpenFileRead_Internal( path, &f, qfalse, 0 );
			if ( f ) {
				FS_FCloseFile( f );
				alias->streamExists = 1;
			} else {
				alias->streamExists = 0;
			}

			if ( alias->streamExists ) {
				goto nextAlias;
			}
			Com_Printf( "WARNING: streamed sound 'sound/%s' not found\n", alias->file );
			missing++;
		}

nextAlias:
		;
	}

	if ( missing && mss_errorOnMissing && mss_errorOnMissing->integer ) {
		Com_Error( source != 0 ? ERR_DROP : ERR_FATAL,
				   va( "\x15%i sound file(s) are missing or in a bad format\n", missing ) );
	}

	return missing;
}

/* ---- Com_UnloadSoundAliasSounds  0x00433E20 ---- VERIFIED */
void Com_UnloadSoundAliasSounds( int source ) {
	snd_alias_t *table;
	int i;

	MSS_StopSounds( 0 );

	table = snd_aliasTable[source];
	for ( i = 0 ; i < snd_aliasTableCount[source] ; i++ ) {
		if ( table[i].type == SND_TYPE_LOADED ) {
			table[i].soundData = NULL;
		}
	}
}
#endif

/* ---- Com_LoadSoundAliases  0x00433E90 ---- VERIFIED */
void Com_LoadSoundAliases( const char *name, int source ) {
	char    normalized[64];
	char    **files;
	char    **p;
	int     numFiles;
	int     i;
	int     count;
	size_t  length;

	count = 0;

	if ( source == SND_LOCALE_INGAME && !Q_stricmp( snd_aliasLocalizedName, name ) ) {
		snd_aliasTable[SND_LOCALE_INGAME]      = snd_aliasTable[SND_LOCALE_LOCALIZED];
		snd_aliasTableCount[SND_LOCALE_INGAME] = snd_aliasTableCount[SND_LOCALE_LOCALIZED];
		Com_Memcpy( snd_aliasHashTable[SND_LOCALE_INGAME],
					snd_aliasHashTable[SND_LOCALE_LOCALIZED],
					SND_ALIAS_HASH_SLICE_BYTES );
	} else {
		if ( !Q_stricmpn( name, "maps/", 5 ) ) {
			strcpy( normalized, name + 5 );
			length = strlen( normalized );
			if ( length >= 4 && !Q_stricmp( normalized + length - 4, ".bsp" ) ) {
				normalized[length - 4] = '\0';
			}
		} else {
			strcpy( normalized, name );
		}
		Q_strlwr( normalized );

		files = FS_ListFilteredFiles( "soundaliases", "csv", NULL, &numFiles );
		if ( !numFiles ) {
			Com_Printf( "WARNING: can't find any sound alias files (soundaliases/*.csv)\n" );
			return;
		}

		Hunk_SetLowTempMark();
		for ( i = 0 ; i < numFiles ; i++ ) {
			snd_currentFile = files[i];
			count = Com_LoadSoundAliasFile( va( "soundaliases/%s", files[i] ),
											normalized, count, source );
		}
		if ( count ) {
			Com_MakeSoundAliasesPermanent( count, source );
		}
		Hunk_ClearToLowTempMark();

		if ( files ) {
			for ( p = files ; *p ; p++ ) {
				free( *p );
			}
			free( files );
		}

		if ( source == SND_LOCALE_LOCALIZED ) {
			strcpy( snd_aliasLocalizedName, name );
		}
	}

	if ( source == SND_LOCALE_MENU || source == SND_LOCALE_INGAME ) {
		if ( !snd_aliasSetLoaded[SND_LOCALE_INGAME] && !snd_aliasSetLoaded[SND_LOCALE_MENU] ) {
			Cmd_AddCommand( "snd_list", Com_SoundList_f );
		}
	}

	snd_aliasSetLoaded[source] = 1;

#ifndef DEDICATED
	if ( source == SND_LOCALE_MENU || source == SND_LOCALE_INGAME ) {
		Com_LoadSoundAliasSounds( source );
	}
#endif
}

/* ---- Com_UnloadSoundAliases  0x004340B0 ---- VERIFIED */
void Com_UnloadSoundAliases( int source ) {
	if ( !snd_aliasSetLoaded[source] ) {
		return;
	}

	if ( source == SND_LOCALE_LOCALIZED ) {
		snd_aliasLocalizedName[0] = '\0';
	}
#ifndef DEDICATED
	else {
		Com_UnloadSoundAliasSounds( source );
	}
#endif

	if ( snd_aliasTable[source] ) {
		if ( source != SND_LOCALE_INGAME || !snd_aliasTable[SND_LOCALE_LOCALIZED] ) {
			free( snd_aliasTable[source] );
		}
		snd_aliasTable[source]      = NULL;
		snd_aliasTableCount[source] = 0;
		memset( snd_aliasHashTable[source], 0, SND_ALIAS_HASH_SLICE_BYTES );
	}

	snd_aliasSetLoaded[source] = 0;

	if ( ( source == SND_LOCALE_MENU || source == SND_LOCALE_INGAME )
		 && !snd_aliasSetLoaded[SND_LOCALE_INGAME]
		 && !snd_aliasSetLoaded[SND_LOCALE_MENU] ) {
		Cmd_RemoveCommand( "snd_list" );
	}
}

/* ---- Com_SoundAliasString  0x00434150 ---- VERIFIED */
const char *Com_SoundAliasString( const char *name, int source ) {
	snd_alias_t *alias;

	alias = Com_FindSoundAlias( name, source );
	if ( !alias ) {
		return NULL;
	}
	return alias->name;
}

/* ---- Com_PickSoundAlias  0x00434160 ---- VERIFIED */
snd_alias_t *Com_PickSoundAlias( const char *name, int source ) {
	snd_alias_t *first;
	snd_alias_t *selected;
	snd_alias_t *cursor;
	snd_alias_t *tableBase;
	int         matches;
	int         remaining;
	int         maxPickSequence;
	float       totalWeight;

	first = Com_FindSoundAlias( name, source );
	if ( !first ) {
		return NULL;
	}

	selected        = first;
	totalWeight     = first->probability;
	matches         = 1;
	maxPickSequence = first->pickSequence;
	tableBase       = snd_aliasTable[source];

	if ( first != tableBase ) {
		cursor = first;
		do {
			cursor--;
			if ( cursor->name != first->name ) {
				break;
			}

			matches++;
			totalWeight += cursor->probability;
			if ( cursor->probability * 32768.0 > (double) rand() * totalWeight ) {
				selected = cursor;
			}
			if ( maxPickSequence < cursor->pickSequence ) {
				maxPickSequence = cursor->pickSequence;
			}
		} while ( cursor != tableBase );

		if ( matches > 2 && maxPickSequence == selected->pickSequence ) {
			totalWeight = 0.0f;
			cursor = first;
			for ( remaining = matches ; remaining ; remaining-- ) {
				if ( maxPickSequence != cursor->pickSequence ) {
					totalWeight += cursor->probability;
					if ( cursor->probability * 32768.0 > (double) rand() * totalWeight ) {
						selected = cursor;
					}
				}
				cursor--;
			}
		}
	}

	selected->pickSequence = maxPickSequence + 1;
	return selected;
}

/* ---- Com_GetSoundAlias  0x00434260 ---- VERIFIED */
snd_alias_t *Com_GetSoundAlias( int source, int index ) {
	if ( index <= 0 || index > snd_aliasTableCount[source] ) {
		return NULL;
	}
	return &snd_aliasTable[source][index - 1];
}

/* Com_SoundAliasIndex  0x00434280  VERIFIED */
int Com_SoundAliasIndex( const snd_alias_t *alias, int source ) {
	int index;

	index = (int) ( alias - snd_aliasTable[source] ) + 1;
	if ( index <= 0 || index > snd_aliasTableCount[source] ) {
		return 0;
	}
	return index;
}

/* Com_SoundAliasChecksum  0x00893E34  VERIFIED */
int Com_SoundAliasChecksum( int source ) {
	return snd_aliasTableChecksum[source];
}

/* ---- Com_StringEdReferenceExists  0x004342C0 ---- */
qboolean Com_StringEdReferenceExists( const char *reference ) {
	void        *buffer;
	char        *data_p;
	char        *token;
	qboolean    found;

	if ( Q_strncmp( reference, SUBTITLE_PREFIX, SUBTITLE_PREFIX_LEN ) ) {
		return qfalse;
	}

	if ( FS_ReadFile( SUBTITLE_FILE, &buffer ) < 0 ) {
		Com_Printf( "WARNING: Could not read local copy of StringEd file %s\n",
					SUBTITLE_FILE );
		return qfalse;
	}

	Com_BeginParseSession( SUBTITLE_FILE );
	data_p = (char *) buffer;
	found  = qfalse;

	for ( ;; ) {
		token = Com_Parse( &data_p );
		if ( !data_p ) {
			break;
		}
		if ( !strcmp( token, "REFERENCE" ) ) {
			token = Com_ParseOnLine( &data_p );
			if ( token && !Q_stricmp( reference + SUBTITLE_PREFIX_LEN, token ) ) {
				found = qtrue;
				break;
			}
		}
		Com_SkipRestOfLine( 0, &data_p );
	}

	Com_EndParseSession();
	FS_FreeFile( buffer );
	return found;
}

/* ---- Com_GetSubtitleStringEdReference  0x00434470 ---- */
const char *Com_GetSubtitleStringEdReference( const char *englishText ) {
	void    *buffer;
	char    *data_p;
	char    *token;

	if ( FS_ReadFile( SUBTITLE_FILE, &buffer ) < 0 ) {
		Com_Printf( "WARNING: Could not read local copy of StringEd file %s\n",
					SUBTITLE_FILE );
		return NULL;
	}

	Com_BeginParseSession( SUBTITLE_FILE );
	data_p = (char *) buffer;

	while ( data_p ) {
		token = Com_Parse( &data_p );
		if ( !data_p ) {
			break;
		}

		if ( !strcmp( token, "REFERENCE" ) ) {
			token = Com_ParseOnLine( &data_p );
			strcpy( snd_subtitleReference, token );
			Com_SkipRestOfLine( 0, &data_p );

			do {
				token = Com_Parse( &data_p );
				if ( !data_p ) {
					Com_Error( ERR_DROP, "\x15StringEd file %s has bad syntax",
							   SUBTITLE_FILE );
				}
			} while ( strcmp( token, "LANG_ENGLISH" ) );

			token = Com_ParseOnLine( &data_p );
			if ( englishText && token && !Q_stricmp( englishText, token ) ) {
				Com_EndParseSession();
				FS_FreeFile( buffer );
				return snd_subtitleReference;
			}
		}

		Com_SkipRestOfLine( 0, &data_p );
	}

	Com_EndParseSession();
	FS_FreeFile( buffer );
	return NULL;
}

/* ---- Com_WriteStringEdReferenceToFile  0x004346F0 ---- */
static void Com_WriteStringEdReferenceToFile( const char *englishText,
											  const char *reference,
											  fileHandle_t f ) {
	FS_Write( "REFERENCE           ", strlen( "REFERENCE           " ), f );
	FS_Write( reference, strlen( reference ), f );
	FS_Write( "\r\nLANG_ENGLISH        \"", strlen( "\r\nLANG_ENGLISH        \"" ), f );
	FS_Write( englishText, strlen( englishText ), f );
	FS_Write( "\"\r\n\r\n", strlen( "\"\r\n\r\n" ), f );
}

/* ---- Com_SetStringEdReference  0x00434790 ---- (0x00434760 was mid-instruction) */
void Com_SetStringEdReference( const char *referenceTag, const char *englishText ) {
	void            *buffer;
	char            *data_p;
	char            *copyFrom;
	char            *token;
	const char      *reference;
	fileHandle_t    f;
	qboolean        replaced;
	char            tempPath[MAX_OSPATH];
	char            realPath[MAX_OSPATH];

	replaced  = qfalse;
	reference = referenceTag + SUBTITLE_PREFIX_LEN;

	f = FS_FOpenFileWrite( "soundaliases/temp.st" );
	if ( !f ) {
		Com_Printf( "WARNING: Could not open output file %s for writing\n",
					"soundaliases/temp.st" );
		return;
	}

	if ( FS_ReadFile( SUBTITLE_FILE, &buffer ) < 0 ) {
		Com_Printf( "WARNING: Could not read local copy of StringEd file %s\n",
					SUBTITLE_FILE );
		FS_FCloseFile( f );
		return;
	}

	Com_BeginParseSession( SUBTITLE_FILE );
	data_p   = (char *) buffer;
	copyFrom = (char *) buffer;

	for ( ;; ) {
		token = Com_Parse( &data_p );
		if ( !data_p ) {
			break;
		}

		if ( !strcmp( token, "ENDMARKER" ) ) {
			if ( copyFrom < data_p ) {
				FS_Write( copyFrom, (int) ( data_p - copyFrom ) - 11, f );
			}
			copyFrom = NULL;
			break;
		}

		if ( !strcmp( token, "REFERENCE" ) ) {
			token = Com_ParseOnLine( &data_p );
			if ( !strcmp( token, reference ) ) {
				if ( copyFrom < data_p ) {
					FS_Write( copyFrom, (int) ( data_p - copyFrom ), f );
				}
				Com_WriteStringEdReferenceToFile( englishText, reference, f );
				replaced = qtrue;

				for ( ;; ) {
					copyFrom = data_p;
					token = Com_Parse( &data_p );
					if ( !data_p ) {
						copyFrom = NULL;
						break;
					}
					if ( !strcmp( token, "REFERENCE" ) || !strcmp( token, "ENDMARKER" ) ) {
						Com_UngetToken();
						break;
					}
				}
				if ( !copyFrom ) {
					break;
				}
			}
		}

		Com_SkipRestOfLine( 0, &data_p );
	}

	if ( !replaced ) {
		Com_WriteStringEdReferenceToFile( englishText, reference, f );
	}

	Com_EndParseSession();
	FS_FreeFile( buffer );

	FS_Write( "\r\nENDMARKER\r\n\r\n\r\n", strlen( "\r\nENDMARKER\r\n\r\n\r\n" ), f );
	FS_FCloseFile( f );

	FS_BuildOSPath3( fs_basepath->string, fs_gamedir, "soundaliases/temp.st", tempPath, 0 );
	FS_BuildOSPath3( fs_basepath->string, fs_gamedir, SUBTITLE_FILE, realPath, 0 );
	FS_CopyFile( tempPath, realPath );
	remove( tempPath );
}

/* ---- Com_ProcessSoundAliasFileLocalization  0x00434B50 ---- */
static void Com_ProcessSoundAliasFileLocalization( const char *filename ) {
	void                *buffer;
	FILE                *probe;
	char                *data_p;
	char                *lineStart;
	char                *token;
	fileHandle_t        f;
	int                 columnMap[SND_ALIAS_MAX_COLUMNS];
	byte                seen[SND_COL_COUNT];
	snd_aliasBuild_t    node;
	char                cells[SND_COL_COUNT][1024];
	char                tag[1024];
	const char          *existing;
	const char          *cell;
	const char          *fmt;
	char                *formatted;
	char                osPath[MAX_OSPATH];
	char                tempPath[MAX_OSPATH];
	char                realPath[MAX_OSPATH];
	int                 columnCount;
	int                 column;
	int                 i;
	int                 localized;
	qboolean            hasName, hasFile;

	FS_BuildOSPath3( fs_basepath->string, fs_gamedir, filename, osPath, 0 );
	Com_Printf( "Processing sound alias file %s..\n", osPath );

	probe = fopen( osPath, "r+" );
	if ( !probe ) {
		Com_Printf( "WARNING: Can not write to sound alias file %s\n", osPath );
		return;
	}
	fclose( probe );

	if ( FS_ReadFile( filename, &buffer ) < 0 ) {
		Com_Printf( "WARNING: Could not read sound alias file %s\n", filename );
		return;
	}

	f = FS_FOpenFileWrite( "soundaliases/temp.csv" );
	if ( !f ) {
		Com_Printf( "WARNING: Could not open output file %s for writing\n",
					"soundaliases/temp.csv" );
		return;
	}

	Com_BeginParseSession( filename );
	Com_SetCSV( qtrue );
	data_p      = (char *) buffer;
	columnCount = 0;
	localized   = 0;

	while ( data_p ) {
		while ( *data_p == '\r' ) {
			data_p++;
		}
		if ( *data_p == '\n' ) {
			data_p++;
			FS_Write( "\r\n", 2, f );
		}

		lineStart = data_p;
		token = Com_Parse( &data_p );
		if ( !data_p ) {
			break;
		}

		if ( !token[0] || token[0] == '#' ) {
			Com_SkipRestOfLine( 0, &data_p );
			if ( *lineStart == '\n' ) {
				FS_Write( "\r", 1, f );
			}
			FS_Write( lineStart, (int) ( data_p - lineStart ), f );
			continue;
		}

		if ( columnCount == 0 ) {
			hasName = qfalse;
			hasFile = qfalse;

			for ( ;; ) {
				columnMap[columnCount] = SND_COL_UNKNOWN;
				for ( column = SND_COL_NAME ; column < SND_COL_COUNT ; column++ ) {
					if ( snd_aliasColumnNames[column] && token
						 && !Q_stricmp( snd_aliasColumnNames[column], token ) ) {
						columnMap[columnCount] = column;
						if ( column == SND_COL_NAME ) {
							hasName = qtrue;
						} else if ( column == SND_COL_FILE ) {
							hasFile = qtrue;
						}
						break;
					}
				}

				columnCount++;
				if ( columnCount == SND_ALIAS_MAX_COLUMNS || !data_p || *data_p == '\n' ) {
					break;
				}
				token = Com_ParseOnLine( &data_p );
			}

			if ( !hasName || !hasFile ) {
				Com_Error( ERR_DROP,
						   "\x15Sound alias file %s: missing 'name' and/or 'file' columns\n",
						   snd_currentFile );
			}

			Com_SkipRestOfLine( 0, &data_p );
			if ( *lineStart == '\n' ) {
				FS_Write( "\r", 1, f );
			}
			FS_Write( lineStart, (int) ( data_p - lineStart ), f );
			continue;
		}

		memset( seen, 0, sizeof( seen ) );
		memset( cells, 0, sizeof( cells ) );
		Com_LoadSoundAliasDefaults( &node, qfalse );

		for ( i = 0 ; i < columnCount ; i++ ) {
			column = columnMap[i];
			strcpy( cells[column], token );
			if ( token[0] ) {
				Com_LoadSoundAliasField( seen, token, column, "", &node );
			}
			if ( i + 1 == columnCount ) {
				break;
			}
			token = Com_ParseOnLine( &data_p );
		}

		if ( !seen[SND_COL_NAME] || !seen[SND_COL_FILE] ) {
			Com_Error( ERR_DROP,
					   "\x15Sound alias file %s: alias entry missing name and/or file\n",
					   snd_currentFile );
		}

		if ( !seen[SND_COL_SUBTITLE]
			 || Com_StringEdReferenceExists( cells[SND_COL_SUBTITLE] ) ) {
			Com_SkipRestOfLine( 0, &data_p );
			FS_Write( lineStart, (int) ( data_p - lineStart ), f );
			continue;
		}

		for ( i = 0 ; i < columnCount ; i++ ) {
			column = columnMap[i];

			if ( !column || !seen[column] ) {
				if ( i != columnCount - 1 ) {
					FS_Write( ",", 1, f );
				}
				continue;
			}

			if ( column == SND_COL_SUBTITLE ) {
				existing = Com_GetSubtitleStringEdReference( cells[SND_COL_SUBTITLE] );
				if ( existing ) {
					Com_sprintf( tag, sizeof( tag ), "%s%s", SUBTITLE_PREFIX, existing );
					Q_strupr( tag );
				} else {
					if ( seen[SND_COL_SEQUENCE] ) {
						Com_sprintf( tag, sizeof( tag ), "%s%s_%s", SUBTITLE_PREFIX,
									 cells[SND_COL_NAME], cells[SND_COL_SEQUENCE] );
					} else {
						Com_sprintf( tag, sizeof( tag ), "%s%s", SUBTITLE_PREFIX,
									 cells[SND_COL_NAME] );
					}
					Q_strupr( tag );
					Com_SetStringEdReference( tag, cells[SND_COL_SUBTITLE] );
					localized++;
				}
				FS_Write( tag, strlen( tag ), f );
				continue;
			}

			cell = cells[column];
			if ( strchr( cell, ',' ) || strchr( cell, ' ' )
				 || strchr( cell, '\n' ) || strchr( cell, '\r' ) ) {
				fmt = ( i == columnCount - 1 ) ? "\"%s\"" : "\"%s\",";
			} else {
				fmt = ( i == columnCount - 1 ) ? "%s" : "%s,";
			}
			formatted = va( (char *) fmt, cell );
			FS_Write( formatted, strlen( formatted ), f );
		}

		FS_Write( "\r\n", 2, f );
		Com_SkipRestOfLine( 0, &data_p );
	}

	Com_EndParseSession();
	FS_FCloseFile( f );

	FS_BuildOSPath3( fs_basepath->string, fs_gamedir, "soundaliases/temp.csv", tempPath, 0 );
	FS_BuildOSPath3( fs_basepath->string, fs_gamedir, filename, realPath, 0 );
	if ( localized ) {
		FS_CopyFile( tempPath, realPath );
	}
	remove( tempPath );
	Com_Printf( "Localized %i sound alias subtitles\n", localized );
}

/* ---- COM_WriteFinalStringEdFile  0x004352F0 ---- */
static void COM_WriteFinalStringEdFile( const char *src, const char *dst ) {
	FILE    *in, *out;
	size_t  length;
	void    *data;

	in = fopen( src, "rb" );
	if ( !in ) {
		return;
	}

	fseek( in, 0, SEEK_END );
	length = ftell( in );
	fseek( in, 0, SEEK_SET );

	data = malloc( length );
	if ( fread( data, 1, length, in ) != length ) {
		Com_Error( ERR_FATAL, "\x15Short read in COM_WriteFinalStringEdFile()\n" );
	}
	fclose( in );

	out = fopen( dst, "wb" );
	if ( !out ) {
		free( data );
		return;
	}

	if ( fwrite( data, 1, length, out ) != length ) {
		Com_Error( ERR_FATAL, "\x15Short write in COM_WriteFinalStringEdFile()\n" );
	}
	fclose( out );
	free( data );
}

/* ---- Com_WriteLocalizedSoundAliasFiles  0x004353B0 ---- */
void Com_WriteLocalizedSoundAliasFiles( void ) {
	char    masterPath[MAX_OSPATH];
	char    localPath[MAX_OSPATH];
	char    csvPath[MAX_OSPATH];
	char    **files;
	FILE    *probe;
	int     numFiles;
	int     i;

	FS_BuildOSPath3( fs_homepath->string,
					 "../source_data/string_resources/subtitle.st", "",
					 masterPath, 0 );
	masterPath[strlen( masterPath ) - 1] = '\0';

	probe = fopen( masterPath, "r+" );
	if ( !probe ) {
		Com_Printf( "WARNING: Can not write to StringEd file %s\n", masterPath );
		return;
	}
	fclose( probe );

	FS_BuildOSPath3( fs_basepath->string, fs_gamedir, SUBTITLE_FILE, localPath, 0 );
	FS_CopyFile( masterPath, localPath );

	if ( !FS_FileExists( SUBTITLE_FILE ) ) {
		Com_Printf( "WARNING: Could not make local copy of StringEd file %s\n",
					SUBTITLE_FILE );
		return;
	}

	Com_Printf( "Localizing sound alias subtitle text...\n" );
	Com_Printf( "Writing to StringEd file %s\n", masterPath );

	files = FS_ListFilteredFiles( "soundaliases", "csv", NULL, &numFiles );
	if ( !numFiles ) {
		Com_Printf( "WARNING: can't find any sound alias files (soundaliases/*.csv)\n" );
		return;
	}

	for ( i = 0 ; i < numFiles ; i++ ) {
		Hunk_SetLowTempMark();
		Com_sprintf( csvPath, sizeof( csvPath ), "soundaliases/%s", files[i] );
		Com_ProcessSoundAliasFileLocalization( csvPath );
		Hunk_ClearToLowTempMark();
	}

	FS_FreeFileList( files );
	COM_WriteFinalStringEdFile( localPath, masterPath );
	remove( localPath );
	Com_Printf( "done\n" );
}
