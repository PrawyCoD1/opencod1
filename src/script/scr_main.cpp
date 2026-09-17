/*
 * script/scr_main.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_main.cpp
 *
 * Retail range 0x0046D380-0x0046DBB0, 12 functions.
 *
 * @fidelity: likely
 */

#include "scr_local.h"

#define FindNextSibling     cod1_globals_FindNextSibling_data
#define FindObject          cod1_globals_FindObject_data
#include "../qcommon/cod1_globals.h"
#undef FindNextSibling
#undef FindObject

extern void FS_ClearDataForFiles( void *lowEnd, void *highEnd );
extern int Hunk_AllocAlignInternal();
extern int Hunk_AllocLowAlignInternal();
extern int InitOpcodeLookup();
extern int ShutdownOpcodeLookup();
extern int Scr_AddSourceBuffer();
extern int Scr_InitDeveloperOpcodes__Fv();
extern int Scr_InsertDeveloperOpcodes__Fv();
extern int Scr_LoadAnimTreeAtIndex__FiPFi_Pv();
extern void ScriptCompile( int parseTree, unsigned short currentFunctionRoot );
extern int ScriptParse();
extern int XModelClearData();

#define HUNK_HIGH_PERM      ( *(int *)hunk_highUsed )
#define HUNK_HIGH_TEMP      ( *(int *)hunk_highTemp )
#define HUNK_TOTAL          ( *(int *)hunk_totalSize )
#define HUNK_LOW_PERM       ( *(int *)hunk_temp_permanent )

#define SCR_HANDLE( g )     ( *(unsigned short *)&(g) )

/* 0x008E60BC: canonical count in the low word, the two loading flags in bytes 2 and 3 */
#define SCR_CANONICAL_COUNT ( *(unsigned short *)&dword_8E60BC )
#define SCR_LOADING_SCRIPTS ( *( (unsigned char *)&dword_8E60BC + 2 ) )
#define SCR_LOADING_ANIMS   ( *( (unsigned char *)&dword_8E60BC + 3 ) )

#define SCR_SCRIPTSPOS      SCR_HANDLE( scrCompilePub_scriptsPos )
#define SCR_LOADEDSCRIPTS   SCR_HANDLE( scrCompilePub_loadedScripts )
#define SCR_ANIMTREES       SCR_HANDLE( scrAnimPub_treeRoot )

#define SCR_CANONICAL_MAP_BYTES     0x20000
#define SCR_FILENAME_BUFFER_SIZE    64

void Scr_BeginLoadAnimTrees( void );
void Scr_EndLoadAnimTrees( void );
void Scr_EndLoadScripts( void );

/* ---- Scr_GetFunctionHandle  0x0046D380 ---- */
int Scr_GetFunctionHandle( const char *filename, const char *funcname ) {
	unsigned short  canonical;
	unsigned short  fileId;
	unsigned short  nameId;
	unsigned short  funcId;
	unsigned int    codePos;

	canonical = Scr_CreateCanonicalFilename( filename );
	fileId = FindVariable( SCR_SCRIPTSPOS, canonical );
	SL_RemoveRefToString( canonical );
	if ( !fileId ) {
		return 0;
	}

	nameId = SL_FindLowercaseString( funcname );
	if ( !nameId ) {
		return 0;
	}

	funcId = FindVariable( FindObject( fileId ), nameId );
	if ( !funcId ) {
		return 0;
	}

	codePos = GetVariableValueAddress( funcId )->u.uintValue;
	if ( codePos < (unsigned int)scrVarPub_programBuffer
	  || codePos >= (unsigned int)scrVarPub_programBuffer + (unsigned int)scrCompilePub_programLen ) {
		return 0;
	}
	return (int)( codePos - (unsigned int)scrVarPub_programBuffer );
}

/* ---- SL_BeginLoadScripts  0x0046D460 ---- VERIFIED */
void SL_BeginLoadScripts( void ) {
	hunk_highTempMark = HUNK_HIGH_PERM;

	scrString_canonicalMap = (int)Hunk_AllocAlignInternal( SCR_CANONICAL_MAP_BYTES, 32 );
	scrString_canonicalMap = (unsigned short *)scrString_canonicalMap;

	SCR_CANONICAL_COUNT = 0;
	scrString_canonicalCount = 0;
}

/* ---- SL_EndLoadScripts  0x0046D490 ---- VERIFIED */
void SL_EndLoadScripts( void ) {
	HUNK_HIGH_TEMP = hunk_highTempMark;
	HUNK_HIGH_PERM = hunk_highTempMark;

	XModelClearData( (char *)s_hunkData + HUNK_TOTAL - hunk_highTempMark,
					 (char *)s_hunkData + HUNK_LOW_PERM );
	FS_ClearDataForFiles( (char *)s_hunkData + HUNK_LOW_PERM,
						  (char *)s_hunkData + HUNK_TOTAL - hunk_highTempMark );
}

/* ---- SL_TransferToCanonicalString  0x0046D4D0 ---- VERIFIED */
unsigned short SL_TransferToCanonicalString( unsigned short handle ) {
	scrStringEntry_t   *entry = GetRefString( handle );
	unsigned short      id;

	if ( entry->flags & SCR_STRING_USER_CANONICAL ) {
		entry->refCount--;
	} else {
		entry->flags |= SCR_STRING_USER_CANONICAL;
	}

	id = scrString_canonicalMap[handle];
	if ( !id ) {
		id = (unsigned short)( SCR_CANONICAL_COUNT + 1 );
		SCR_CANONICAL_COUNT = id;
		scrString_canonicalCount = id;
		scrString_canonicalMap[handle] = id;
	}
	return id;
}

/* ---- SL_FindCanonicalString  0x0046D520 ---- */
unsigned short SL_FindCanonicalString( const char *text ) {
	return scrString_canonicalMap[ SL_FindStringOfLen( text, (unsigned int)strlen( text ) + 1 ) ];
}

/* ---- Scr_BeginLoadScripts  0x0046D550 ---- */
void Scr_BeginLoadScripts( void ) {
	SCR_LOADING_SCRIPTS = 1;

	InitOpcodeLookup();
	Scr_InitDeveloperOpcodes__Fv();

	SCR_SCRIPTSPOS    = Scr_AllocArray();
	SCR_LOADEDSCRIPTS = Scr_AllocArray();

	scrVarPub_programBuffer = (int)Hunk_AllocLowAlignInternal( 0, 32 );
	scrCompilePub_programLen = 0;
	dword_1407384 = 0;

	SL_BeginLoadScripts();
	dword_976468 = 0;

	Scr_BeginLoadAnimTrees();
}

/* ---- Scr_BeginLoadAnimTrees  0x0046D5F0 ---- */
void Scr_BeginLoadAnimTrees( void ) {
	SCR_LOADING_ANIMS = 1;

	SCR_ANIMTREES = Scr_AllocArray();
	SCR_HANDLE( scrAnimPub_currentTreeRoot ) = 0;

}

/* ---- Scr_LoadScript  0x0046D690 ---- */
qboolean Scr_LoadScript( const char *filename ) {
	unsigned short  canonical;
	unsigned short  fileId;
	int             sourceBuf;
	int             savedSourcePos;
	const char     *savedFilename;
	void           *sourceMem;
	char            name[SCR_FILENAME_BUFFER_SIZE];
	int             parseTree;

	canonical = Scr_CreateCanonicalFilename( filename );

	if ( FindVariable( SCR_LOADEDSCRIPTS, canonical ) ) {
		SL_RemoveRefToString( canonical );
		return FindVariable( SCR_SCRIPTSPOS, canonical ) != 0 ? qtrue : qfalse;
	}

	GetVariableIndexInternal( SCR_LOADEDSCRIPTS, canonical );
	SL_RemoveRefToString( canonical );

	currentPos = 0;

	strncpy( name, canonical ? SL_ConvertToString( canonical ) : NULL,
			 SCR_FILENAME_BUFFER_SIZE );
	strncat( name, ".gsc", SCR_FILENAME_BUFFER_SIZE );

	savedSourcePos = scrCompilePub_sourceBuf;
	sourceMem = (void *)Hunk_AllocLowAlignInternal( 0, 32 );

	sourceBuf = (int)Scr_AddSourceBuffer( name, (int)sourceMem, developerCodeStart );
	if ( !sourceBuf ) {
		return qfalse;
	}

	savedFilename = (const char *)scrCompilePub_scriptfilename;
	SCR_HANDLE( dword_8E60B0 ) = 0;
	scrCompilePub_scriptCount = 0;
	scrCompilePub_scriptfilename = (int)name;

	ScriptParse( sourceBuf, &parseTree );

	fileId = GetVariable( SCR_SCRIPTSPOS, canonical );
	ScriptCompile( parseTree, GetObject( fileId ) );

	scrCompilePub_scriptfilename = (int)savedFilename;
	scrCompilePub_sourceBuf = savedSourcePos;
	return qtrue;
}

/* ---- Scr_EndLoadScripts  0x0046D830 ---- */
void Scr_EndLoadScripts( void ) {
	unsigned short root;

	SCR_LOADING_SCRIPTS = 0;
	SL_EndLoadScripts();

	root = SCR_LOADEDSCRIPTS;
	ClearObject( root );
	RemoveRefToObject( root );
	SCR_LOADEDSCRIPTS = 0;

	root = SCR_SCRIPTSPOS;
	ClearObject( root );
	RemoveRefToObject( root );
	SCR_SCRIPTSPOS = 0;

	Scr_InsertDeveloperOpcodes__Fv();
	SL_ShutdownSystem( SCR_STRING_USER_CANONICAL );
}

extern int  Scr_GetAnimTreeCount( void );
extern void Scr_LoadAnimTreeAtIndex( int treeIndex, void *(__cdecl *allocFn)( int ) );

/* ---- Scr_PrecacheAnimTrees  0x0046D990 ---- HIGH */
void Scr_PrecacheAnimTrees( void *allocFn ) {
	int i;

	for ( i = 1; i <= Scr_GetAnimTreeCount(); i++ ) {
		Scr_LoadAnimTreeAtIndex( i, (void *(__cdecl *)( int ))allocFn );
	}
}

/* ---- Scr_EndLoadAnimTrees  0x0046D9D0 ---- */
void Scr_EndLoadAnimTrees( void ) {
	unsigned short root;

	SCR_LOADING_ANIMS = 0;

	root = SCR_ANIMTREES;
	ClearObject( root );
	RemoveRefToObject( root );
	SCR_ANIMTREES = 0;

	if ( SCR_HANDLE( scrAnimPub_currentTreeRoot ) ) {
		RemoveRefToObject( SCR_HANDLE( scrAnimPub_currentTreeRoot ) );
	}

	SL_ShutdownSystem( SCR_STRING_USER_CANONICAL );

	dword_1407384 = (int)Hunk_AllocLowAlignInternal( 0, 32 );
}

/* ---- Scr_FreeScripts  0x0046DAF0 ---- */
void Scr_FreeScripts( void ) {
	if ( SCR_LOADING_SCRIPTS ) {
		Scr_EndLoadScripts();
	}
	if ( SCR_LOADING_ANIMS ) {
		Scr_EndLoadAnimTrees();
	}

	scrVarPub_programBuffer = 0;
	scrCompilePub_programLen = 0;
	dword_1407384 = 0;
	scrCompilePub_checksum = 0;
	SCR_LOADING_SCRIPTS = 0;

	SL_ShutdownSystem( SCR_STRING_USER_SCRIPT );
	ShutdownOpcodeLookup();

	if ( scrVarPub_developerScript ) {
		if ( scrCompileGlob_devOpBuffer ) {
			free( (void *)scrCompileGlob_devOpBuffer );
			scrCompileGlob_devOpBuffer = 0;
		}
		if ( scrCompileGlob_devPatchTable ) {
			free( (void *)scrCompileGlob_devPatchTable );
			scrCompileGlob_devPatchTable = 0;
		}
	}
}

void SL_BeginLoadScripts__Fv( void ) { SL_BeginLoadScripts(); }
void SL_EndLoadScripts__Fv( void ) { SL_EndLoadScripts(); }
unsigned short SL_TransferToCanonicalString__FUs( unsigned short h ) {
	return SL_TransferToCanonicalString( h );
}
unsigned short SL_FindCanonicalString__FPCc( const char *text ) {
	return SL_FindCanonicalString( text );
}
