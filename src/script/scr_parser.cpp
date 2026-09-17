/*
 * script/scr_parser.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_parser.cpp
 *
 * Retail range 0x0046E590-0x0046F0F0, 15 functions.
 *
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

#define scrCompilePub_scriptsAvailable  scrVarPub_developer    /* 0x008E60D0 */
#define scrCompileGlob_codegenMode      developer_script_int /* 0x008E5EE8 */
#define scrCompileGlob_lastOpcodePos    scrCompileGlob_lastOpcodePos    /* 0x00966430 */
#define scrCompilePub_developerBufPos   developerCodeStart    /* 0x008E5EE4 */
#define scrCompileGlob_codeRelocStart   scrCompileGlob_codeRelocStart    /* 0x008E5F00 */
#define scrVarPub_programBuffer         scrVarPub_programBuffer   /* 0x01407380 */
#define scrCompilePub_programLen        scrCompilePub_programLen   /* 0x01407388 */
#define scrParserPub_sourceBuf          scrCompilePub_sourceBuf   /* 0x01407370 */
#define scrParserPub_scriptfilename     scrCompilePub_scriptfilename   /* 0x01407374 */

#define SCR_CODEGEN_INTERN          0
#define SCR_CODEGEN_RELOCATE        1
#define SCR_CODEGEN_DISCARD         2

#define scrVmPub_terminal_error     scrVmPub_terminal_error     /* 0x00A7A5CC */
#define scrVmGlob_inparamcount      scrVmGlob_funcCount    /* 0x00A7A5D8 */
#define scrVmGlob_callStack         scrVmGlob_callStack    /* 0x00A7A510 */

extern void *Hunk_AllocateTempMemoryHighInternal( int size );
extern char *SEH_GetLocalizedString_m( const char *name );
extern void  Sys_OutOfMemoryPrep( void );

__declspec( dllimport ) int __stdcall MessageBoxA( void *wnd, const char *text,
												   const char *caption,
												   unsigned int type );
#define SCR_MB_ICONHAND     0x10u

#define SCR_SOURCEPOS_TABLE_COUNT       2
#define SCR_SOURCEPOS_INITIAL_CAPACITY  0x10000
#define SCR_SOURCEFILE_INITIAL_CAPACITY 0x10
#define SCR_SAVEDSOURCE_NONE            (-1)

typedef struct scrSourcePosRecord_s {
	const char     *codePos;            /* +0x00 */
	unsigned int    sourcePosIndex;     /* +0x04  first slot in the pool */
	unsigned int    unk_08;             /* +0x08 */
	unsigned int    unk_0C;             /* +0x0C */
} scrSourcePosRecord_t;

typedef struct scrSourceFile_s {
	const char     *codeStart;          /* +0x00  low hunk mark when it compiled */
	const char     *developerCodeStart; /* +0x04  developer buffer mark, ditto */
	char           *name;               /* +0x08  Z_Malloc'd, name then source */
	char           *source;             /* +0x0C  points inside ->name */
	int             sourceLen;          /* +0x10  -1 means "file was missing" */
} scrSourceFile_t;

typedef struct scrSavedSource_s {
	char           *source;             /* +0x00 */
	int             sourceLen;          /* +0x04 */
} scrSavedSource_t;

/* retail 0x00966440 / 0x00966444 -- [0] normal code, [1] relocated code */
static scrSourcePosRecord_t *scrParser_posTable[SCR_SOURCEPOS_TABLE_COUNT];
/* retail 0x00966434 / 0x00966438 */
static unsigned int          scrParser_posCapacity[SCR_SOURCEPOS_TABLE_COUNT];
/* retail 0x00966448 / 0x0096644C */
static unsigned int          scrParser_posCount[SCR_SOURCEPOS_TABLE_COUNT];

/* retail 0x0140736C -- the flat pool every record indexes into */
static unsigned int         *scrParser_sourcePosPool;
/* retail 0x00966424 / 0x00966420 */
static unsigned int          scrParser_poolCapacity;
static unsigned int          scrParser_poolCount;

/* retail 0x0096643C / 0x00966428 -- run of positions sharing one opcode */
static const char           *scrParser_lastCodePos;
static unsigned int          scrParser_lastCodePosCount;

/* retail 0x01407378 / 0x0096641C / 0x0096642C */
static scrSourceFile_t      *scrParser_sourceFiles;
static unsigned int          scrParser_sourceFileCount;
static unsigned int          scrParser_sourceFileCapacity;

/* retail 0x0140737C / 0x00575814 -- only non-NULL while loading a savegame */
static scrSavedSource_t     *scrParser_savedSource;
static int                   scrParser_savedSourceCount = SCR_SAVEDSOURCE_NONE;

typedef void ( *scrSourceIO_t )( void *buffer, int len );

/* Scr_OutOfMemory  0x00575814 */
static void Scr_OutOfMemory( void ) {
	char   *title;
	char   *body;

	Sys_OutOfMemoryPrep();
	title = SEH_GetLocalizedString_m( "WIN_OUT_OF_MEM_TITLE" );
	body  = SEH_GetLocalizedString_m( "WIN_OUT_OF_MEM_BODY" );
	MessageBoxA( 0, body, title, SCR_MB_ICONHAND );
	exit( -1 );
}

static void *Scr_Malloc( unsigned int size ) {
	void   *p;

	p = malloc( size );
	if ( !p ) {
		Scr_OutOfMemory();
	}
	Com_Memset( p, 0, size );
	return p;
}

/* ---- InitOpcodeLookup  0x0046E590 ---- */
void InitOpcodeLookup( void ) {
	int     i;

	if ( !scrCompilePub_scriptsAvailable ) {
		return;
	}

	for ( i = 0; i < SCR_SOURCEPOS_TABLE_COUNT; i++ ) {
		scrParser_posCapacity[i] = SCR_SOURCEPOS_INITIAL_CAPACITY;
		scrParser_posCount[i]    = 0;
		scrParser_posTable[i]    = (scrSourcePosRecord_t *)
			Scr_Malloc( scrParser_posCapacity[i] * sizeof( scrSourcePosRecord_t ) );
	}

	scrParser_poolCapacity = SCR_SOURCEPOS_INITIAL_CAPACITY;
	scrParser_poolCount    = 0;
	scrParser_sourcePosPool = (unsigned int *)
		Scr_Malloc( scrParser_poolCapacity * sizeof( unsigned int ) );

	scrParser_lastCodePos      = NULL;
	scrParser_lastCodePosCount = 0;

	scrParser_sourceFileCapacity = SCR_SOURCEFILE_INITIAL_CAPACITY;
	scrParser_sourceFileCount    = 0;
	scrParser_sourceFiles = (scrSourceFile_t *)
		Scr_Malloc( scrParser_sourceFileCapacity * sizeof( scrSourceFile_t ) );
}

/* ---- ShutdownOpcodeLookup  0x0046E720 ---- */
void ShutdownOpcodeLookup( void ) {
	int     i;

	for ( i = 0; i < SCR_SOURCEPOS_TABLE_COUNT; i++ ) {
		if ( scrParser_posTable[i] ) {
			free( scrParser_posTable[i] );
			scrParser_posTable[i] = NULL;
		}
	}

	if ( scrParser_sourcePosPool ) {
		free( scrParser_sourcePosPool );
		scrParser_sourcePosPool = NULL;
	}

	if ( scrParser_sourceFiles ) {
		for ( i = 0; i < (int)scrParser_sourceFileCount; i++ ) {
			free( scrParser_sourceFiles[i].name );
		}
		free( scrParser_sourceFiles );
		scrParser_sourceFiles = NULL;
	}

	if ( scrParser_savedSource ) {
		for ( i = 0; i < scrParser_savedSourceCount; i++ ) {
			if ( scrParser_savedSource[i].source ) {
				free( scrParser_savedSource[i].source );
			}
		}
		free( scrParser_savedSource );
		scrParser_savedSourceCount = SCR_SAVEDSOURCE_NONE;
		scrParser_savedSource      = NULL;
	}
}

/* ---- AddOpcodePos  0x0046E810 ---- */
void AddOpcodePos( unsigned int sourcePos ) {
	unsigned int            tableIndex;
	unsigned int            oldCapacity;
	scrSourcePosRecord_t   *newTable;
	scrSourcePosRecord_t   *record;
	unsigned int           *newPool;

	if ( !scrCompilePub_scriptsAvailable ) {
		return;
	}
	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_DISCARD ) {
		return;
	}

	tableIndex = ( scrCompileGlob_codegenMode == SCR_CODEGEN_RELOCATE ) ? 1u : 0u;

	if ( scrParser_posCount[tableIndex] >= scrParser_posCapacity[tableIndex] ) {
		oldCapacity = scrParser_posCapacity[tableIndex];
		scrParser_posCapacity[tableIndex] = oldCapacity * 2;
		newTable = (scrSourcePosRecord_t *)Z_MallocInternal(
			(int)( scrParser_posCapacity[tableIndex] * sizeof( scrSourcePosRecord_t ) ) );
		Com_Memset( newTable, 0,
					scrParser_posCapacity[tableIndex] * sizeof( scrSourcePosRecord_t ) );
		Com_Memcpy( newTable, scrParser_posTable[tableIndex],
					oldCapacity * sizeof( scrSourcePosRecord_t ) );
		free( scrParser_posTable[tableIndex] );
		scrParser_posTable[tableIndex] = newTable;
	}

	if ( scrParser_poolCount >= scrParser_poolCapacity ) {
		oldCapacity = scrParser_poolCapacity;
		scrParser_poolCapacity = oldCapacity * 2;
		newPool = (unsigned int *)Z_MallocInternal(
			(int)( scrParser_poolCapacity * sizeof( unsigned int ) ) );
		Com_Memcpy( newPool, scrParser_sourcePosPool,
					oldCapacity * sizeof( unsigned int ) );
		free( scrParser_sourcePosPool );
		scrParser_sourcePosPool = newPool;
	}

	if ( (const char *)scrCompileGlob_lastOpcodePos == scrParser_lastCodePos ) {
		scrParser_posCount[tableIndex]--;
	} else {
		scrParser_lastCodePosCount = 0;
		scrParser_lastCodePos = (const char *)scrCompileGlob_lastOpcodePos;
		scrParser_posTable[tableIndex][ scrParser_posCount[tableIndex] ].sourcePosIndex =
			scrParser_poolCount;
	}

	record = &scrParser_posTable[tableIndex][ scrParser_posCount[tableIndex] ];
	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_RELOCATE ) {
		record->codePos = (const char *)scrCompileGlob_lastOpcodePos
						+ ( (int)scrCompilePub_developerBufPos - scrCompileGlob_codeRelocStart );
	} else {
		record->codePos = (const char *)scrCompileGlob_lastOpcodePos;
	}

	scrParser_sourcePosPool[ record->sourcePosIndex + scrParser_lastCodePosCount ] = sourcePos;

	scrParser_posCount[tableIndex]++;
	scrParser_lastCodePosCount++;
	scrParser_poolCount++;
}

/* ---- GetPrevSourcePos  0x0046E9B0 ---- */
static unsigned int GetPrevSourcePos( const char *codePos, int offset ) {
	unsigned int    tableIndex;
	int             low;
	int             high;
	int             middle;

	tableIndex = ( (unsigned int)codePos >= (unsigned int)scrVarPub_programBuffer
				&& (unsigned int)codePos <  (unsigned int)scrVarPub_programBuffer
										  + (unsigned int)scrCompilePub_programLen ) ? 0u : 1u;

	low  = 0;
	high = (int)scrParser_posCount[tableIndex] - 1;

	while ( low <= high ) {
		middle = ( low + high ) / 2;
		if ( scrParser_posTable[tableIndex][middle].codePos < codePos ) {
			low = middle + 1;
			if ( low == (int)scrParser_posCount[tableIndex]
			  || codePos <= scrParser_posTable[tableIndex][low].codePos ) {
				return scrParser_sourcePosPool[
					scrParser_posTable[tableIndex][middle].sourcePosIndex + offset ];
			}
		} else {
			high = middle - 1;
		}
	}

	return 0;
}

/* ---- Scr_HasSourceFiles  0x0046EA40 ---- */
int Scr_HasSourceFiles( void ) {
	return scrCompilePub_scriptsAvailable;
}

/* ---- Scr_SaveSource  0x0046EA50 ---- */
void Scr_SaveSource( scrSourceIO_t write ) {
	unsigned int    i;

	write( &scrParser_sourceFileCount, 4 );

	for ( i = 0; i < scrParser_sourceFileCount; i++ ) {
		write( &scrParser_sourceFiles[i].sourceLen, 4 );
		if ( scrParser_sourceFiles[i].sourceLen > 0 ) {
			write( scrParser_sourceFiles[i].source, scrParser_sourceFiles[i].sourceLen );
		}
	}
}

/* ---- Scr_LoadSource  0x0046EAC0 ---- */
void Scr_LoadSource( scrSourceIO_t read ) {
	int     i;

	read( &scrParser_savedSourceCount, 4 );
	scrParser_savedSource = (scrSavedSource_t *)Z_MallocInternal(
		scrParser_savedSourceCount * (int)sizeof( scrSavedSource_t ) );

	for ( i = scrParser_savedSourceCount - 1; i >= 0; i-- ) {
		read( &scrParser_savedSource[i].sourceLen, 4 );

		if ( scrParser_savedSource[i].sourceLen <= 0 ) {
			scrParser_savedSource[i].source = NULL;
			continue;
		}

		scrParser_savedSource[i].source =
			(char *)Scr_Malloc( (unsigned int)scrParser_savedSource[i].sourceLen );
		read( scrParser_savedSource[i].source, scrParser_savedSource[i].sourceLen );
	}
}

/* ---- Scr_SkipSource  0x0046EB90 ---- */
void Scr_SkipSource( scrSourceIO_t read ) {
	int     count;
	int     i;
	int     sourceLen;

	read( &count, 4 );
	for ( i = count - 1; i >= 0; i-- ) {
		read( &sourceLen, 4 );
		if ( sourceLen > 0 ) {
			read( NULL, sourceLen );
		}
	}
}

/* ---- Scr_GetNewSourceBuffer  0x0046EBE0 ---- */
static scrSourceFile_t *Scr_GetNewSourceBuffer( void ) {
	scrSourceFile_t    *newFiles;

	if ( scrParser_sourceFileCount >= scrParser_sourceFileCapacity ) {
		scrParser_sourceFileCapacity *= 2;
		newFiles = (scrSourceFile_t *)Z_MallocInternal(
			(int)( scrParser_sourceFileCapacity * sizeof( scrSourceFile_t ) ) );
		Com_Memcpy( newFiles, scrParser_sourceFiles,
					scrParser_sourceFileCount * sizeof( scrSourceFile_t ) );
		free( scrParser_sourceFiles );
		scrParser_sourceFiles = newFiles;
	}

	return &scrParser_sourceFiles[ scrParser_sourceFileCount++ ];
}

/* ---- Scr_AddSourceBuffer  0x0046EC50 ---- VERIFIED */
char *Scr_AddSourceBuffer( const char *filename, const char *codeStart,
						   const char *developerCodeStart ) {
	fileHandle_t        f;
	int                 sourceLen;
	char               *source;
	char               *tracked;
	char               *trackedSource;
	unsigned int        nameLen;
	int                 i;
	char                c;
	scrSourceFile_t    *record;

	if ( !scrParser_savedSource ) {
		sourceLen = FS_FOpenFileByMode( filename, &f, FS_READ );
		if ( sourceLen < 0 ) {
			goto missing;
		}

		source = (char *)Hunk_AllocateTempMemoryHighInternal( sourceLen + 1 );
		FS_Read( source, sourceLen, f );
		source[sourceLen] = '\0';
		FS_FCloseFile( f );
	} else {
		scrParser_savedSourceCount--;
		sourceLen = scrParser_savedSource[ scrParser_savedSourceCount ].sourceLen;
		if ( sourceLen < 0 ) {
			goto missing;
		}

		source = (char *)Hunk_AllocateTempMemoryHighInternal( sourceLen + 1 );
		for ( i = 0; i < sourceLen; i++ ) {
			c = scrParser_savedSource[ scrParser_savedSourceCount ].source[i];
			source[i] = (char)( ( c == '\0' ) ? '\n' : c );
		}
		source[sourceLen] = '\0';

		if ( scrParser_savedSource[ scrParser_savedSourceCount ].source ) {
			free( scrParser_savedSource[ scrParser_savedSourceCount ].source );
		}
	}

	if ( !scrParser_sourceFiles ) {
		scrParserPub_sourceBuf = 0;
		return source;
	}

	nameLen = strlen( filename );
	tracked = (char *)Z_MallocInternal( (int)( sourceLen + nameLen + 3 ) );
	strcpy( tracked, filename );
	trackedSource = tracked + nameLen + 1;

	for ( i = 0; i <= sourceLen; i++ ) {
		c = source[i];
		trackedSource[i] = (char)( ( c == '\n' ) ? '\0' : c );
	}

	record = Scr_GetNewSourceBuffer();
	record->codeStart          = codeStart;
	record->developerCodeStart = developerCodeStart;
	record->name               = tracked;
	record->source             = trackedSource;
	record->sourceLen          = sourceLen;

	scrParserPub_sourceBuf = (int)trackedSource;
	return source;

missing:
	if ( scrParser_sourceFiles ) {
		record = Scr_GetNewSourceBuffer();
		record->codeStart          = NULL;
		record->developerCodeStart = NULL;
		record->name               = NULL;
		record->source             = NULL;
		record->sourceLen          = -1;
	}
	return NULL;
}

#define SCR_MAX_PRINTED_LINE    1024

/* ---- PrintSourcePos  0x0046EDE0 ---- */
static void PrintSourcePos( unsigned int sourcePos, const char *source,
							const char *filename ) {
	const char     *lineStart;
	const char     *p;
	unsigned int    remaining;
	int             line;
	unsigned int    lineLen;
	unsigned int    column;
	unsigned int    i;
	char            buffer[SCR_MAX_PRINTED_LINE];
	char            caret[SCR_MAX_PRINTED_LINE];

	lineStart = source;
	line      = 1;
	p         = source;

	for ( remaining = sourcePos; remaining; remaining-- ) {
		if ( !*p ) {
			lineStart = p + 1;
			line++;
		}
		p++;
	}

	lineLen = strlen( lineStart );
	if ( lineLen > SCR_MAX_PRINTED_LINE - 1 ) {
		lineLen = SCR_MAX_PRINTED_LINE - 1;
	}
	for ( i = 0; i < lineLen; i++ ) {
		buffer[i] = (char)( ( lineStart[i] == '\t' ) ? ' ' : lineStart[i] );
	}
	buffer[lineLen] = '\0';

	Com_Printf( "(file '%s'%s, line %d)\n",
				filename,
				scrParser_savedSource ? " (savegame)" : "",
				line );
	Com_Printf( "%s\n", buffer );

	column = (unsigned int)( p - lineStart );
	if ( column > SCR_MAX_PRINTED_LINE - 2 ) {
		column = SCR_MAX_PRINTED_LINE - 2;
	}
	Com_Memset( caret, ' ', column );
	caret[column]     = '*';
	caret[column + 1] = '\0';
	Com_Printf( "%s\n", caret );
}

/* ---- Scr_PrintPrevCodePos  0x0046EEE0 ---- */
void Scr_PrintPrevCodePos( const char *codePos, int offset ) {
	int                 isDeveloper;
	int                 i;
	const char         *start;
	unsigned int        sourcePos;

	if ( !scrCompilePub_scriptsAvailable ) {
		Com_Printf( "@ %d\n", (int)( codePos - (const char *)scrVarPub_programBuffer ) );
		return;
	}

	isDeveloper = !( (unsigned int)codePos >= (unsigned int)scrVarPub_programBuffer
				  && (unsigned int)codePos <  (unsigned int)scrVarPub_programBuffer
											+ (unsigned int)scrCompilePub_programLen );

	for ( i = (int)scrParser_sourceFileCount - 1; i > 0; i-- ) {
		start = isDeveloper ? scrParser_sourceFiles[i].developerCodeStart
							: scrParser_sourceFiles[i].codeStart;
		if ( start && start < codePos ) {
			break;
		}
	}
	if ( i < 0 ) {
		i = 0;
	}

	sourcePos = GetPrevSourcePos( codePos, offset );
	PrintSourcePos( sourcePos, scrParser_sourceFiles[i].source,
					scrParser_sourceFiles[i].name );
}

/* ---- CompileError  0x0046EF80 ---- */
void QDECL CompileError( unsigned int sourcePos, const char *format, ... ) {
	char        buffer[1024];
	va_list     argptr;

	va_start( argptr, format );
	vsprintf( buffer, format, argptr );
	va_end( argptr );

	Com_Printf( "\n" );
	Com_Printf( "******* script compile error *******\n" );
	if ( scrCompilePub_scriptsAvailable ) {
		Com_Printf( "%s: ", buffer );
		PrintSourcePos( sourcePos, (const char *)scrParserPub_sourceBuf,
						(const char *)scrParserPub_scriptfilename );
	} else {
		Com_Printf( "%s\n", buffer );
	}
	Com_Printf( "************************************\n" );
	Com_Error( ERR_DROP, "\x15script compile error\n(see console for details)" );
}

/* ---- CompileError2  0x0046F050 ---- */
void QDECL CompileError2( const char *codePos, const char *format, ... ) {
	char        buffer[1024];
	va_list     argptr;

	va_start( argptr, format );
	vsprintf( buffer, format, argptr );
	va_end( argptr );

	Com_Printf( "\n" );
	Com_Printf( "******* script compile error *******\n" );
	Com_Printf( "%s: ", buffer );
	Scr_PrintPrevCodePos( codePos, 0 );
	Com_Printf( "************************************\n" );
	Com_Error( ERR_DROP, "\x15script compile error\n(see console for details)" );
}

/* ---- RuntimeError  0x0046F0F0 ---- */
int RuntimeError( const char *dialogMessage, const char *codePos, int offset,
				  const char *message ) {
	int             i;
	const char     *tail;
	const char     *separator;

	if ( !scrCompilePub_scriptsAvailable && !scrVmPub_terminal_error ) {
		return scrVmPub_terminal_error;
	}

	Com_Printf( "\n" );
	Com_Printf( "******* script runtime error *******\n" );
	Com_Printf( "%s: ", message );
	Scr_PrintPrevCodePos( codePos, offset );

	for ( i = scrVmGlob_inparamcount - 1; i >= 0; i-- ) {
		Com_Printf( "called from:\n" );
		Scr_PrintPrevCodePos( ( (const char **)&scrVmGlob_callStack )[i], 0 );
	}
	Com_Printf( "************************************\n" );

	if ( dialogMessage ) {
		tail      = dialogMessage;
		separator = "\n";
	} else {
		tail      = "";
		separator = "";
	}

	Com_Error( scrVmPub_terminal_error ? ERR_DROP : ERR_SERVERDISCONNECT,
			   "\x15script runtime error\n(see console for details)%s%s",
			   separator, tail );
	return 0;
}
