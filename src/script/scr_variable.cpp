/*
 * script/scr_variable.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_variable.cpp
 *
 * Retail range 0x00470530-0x00475810, 141 functions.
 *
 * @fidelity: likely
 */

#include "scr_local.h"
#include "../qcommon/hexrays_shim.h"

#define FindNextSibling     cod1_globals_FindNextSibling_data
#define FindObject          cod1_globals_FindObject_data
#define dword_976468        cod1_globals_scalar_decl_976468
#include "../qcommon/cod1_globals.h"
#undef FindNextSibling
#undef FindObject
#undef dword_976468

extern void  Com_Meminfo_f( void );
extern char **FS_ListFilteredFiles( const char *path, const char *extension,
									const char *filter, int *numfiles );
extern void *Hunk_AllocLowAlignInternal( int size, int align );
extern void *Hunk_AllocateTempMemoryHighInternal( int size );
extern int   Hunk_ClearTempMemoryHigh( void );
extern int   Hunk_CommitTempMemory( void );
extern void  Scr_PrintPrevCodePos( const char *codePos, int offset );
extern void  qsort_m( void *base, unsigned int count, unsigned int width,
					  int ( __cdecl *compare )( const void *, const void * ) );

extern int Hunk_ReallocateTempMemory();

extern unsigned short scrVarPub_gameId;

extern unsigned short scrVarPub_pauseArrayId;

extern unsigned short scrVarPub_levelId;
extern unsigned short scrVarPub_animId;

extern unsigned int   scrVarPub_time;

typedef struct VariableStackBuffer_s {
	const char     *pos;            /* +0x00 */
	const char     *codePos;        /* +0x04 */
	unsigned short  size;           /* +0x08  entry count */
	unsigned short  objectId;       /* +0x0A */
	/* +0x0C  `size` entries, stride 5: unsigned char type; VariableUnion u; */
} VariableStackBuffer;

#define SCR_STACKENTRY_SIZE     5
#define SCR_STACKBUFFER_HEADER  12

void             WriteString( unsigned short id );
unsigned short   ReadId( void );
void             AddSaveObject( unsigned short id );
void             DoLoadEntryInternal( VariableValue *value );
void             DoSaveEntryInternal( unsigned char type, VariableUnion u );

void            AddSaveStack( const struct VariableStackBuffer_s *buf );
float           Scr_GetObjectUsage( unsigned short id );
float           Scr_GetEndonUsage( unsigned short id );
float           Scr_GetThreadUsage( const struct VariableStackBuffer_s *buf, float *endonUsage );

Variable                scrVarIndirections[VARIABLELIST_SIZE];

/* retail 0x009B6478 -- 65536 * 12 bytes */
VariableValueInternal   scrVarNodes[VARIABLELIST_SIZE];

unsigned short          scrVarPub_tempVariable;

unsigned short          scrVarPub_classId;
unsigned short          scrVarPub_entId;

scrClassMapEntry_t     *scrVarPub_classMap;
unsigned int            scrVarPub_classNum;

int                     scrVarPub_error_index;

char                   *scrVarPub_fieldBuffer;

/* retail 0x005757C8 */
const char * const var_typename[VAR_TYPE_COUNT] = {
	"undefined",
	"string",
	"localized string",
	"vector",
	"float",
	"int",
	"codepos",
	"object",
	"key/value",
	"function",
	"stack",
	"animation",
	"thread",
	"entity",
	"struct",
	"array",
	"dead thread",
	"dead entity",
	"dead object"
};

#define VAR_REFCOUNT( n )   ( (n)->u.halfword[0] )
#define VAR_OBJECTID( n )   ( (n)->u.halfword[0] )
#define VAR_PREVFREE( n )   ( (n)->u.halfword[0] )
#define VAR_KVENTNUM( n )   ( (n)->u.halfword[0] )
#define VAR_SELF( n )       ( (n)->u.halfword[1] )
#define VAR_ENTNUM( n )     ( (n)->u.halfword[1] )
#define VAR_ARRAYSIZE( n )  ( (n)->u.halfword[1] )
#define VAR_KVOFFSET( n )   ( (n)->u.halfword[1] )

#define VAR_NODEINDEX( n )  ( (unsigned short)( (n) - scrVarNodes ) )
#define VAR_HANDLEINDEX( s )( (unsigned short)( (s) - scrVarIndirections ) )

#define VAR_TYPE( n )       ( (int)( (n)->status & VAR_STATUS_TYPE_MASK ) )
#define VAR_STAT( n )       ( (n)->status & VAR_STATUS_STAT_MASK )
#define VAR_NAME( n )       ( (n)->status >> VAR_STATUS_NAME_SHIFT )

#define VAR_PACKED_THREAD   ( VAR_STAT_EXTERNAL | VAR_THREAD )
#define VAR_PACKED_ENTITY   ( VAR_STAT_EXTERNAL | VAR_ENTITY )
#define VAR_PACKED_STRUCT   ( VAR_STAT_EXTERNAL | VAR_STRUCT )
#define VAR_PACKED_ARRAY    ( VAR_STAT_EXTERNAL | VAR_ARRAY )

#define SCR_VECTOR_ALLOC_SIZE   ( 2 + 3 * 4 )

#define SCR_STRING_MT_TYPE_TEMP     14
#define SCR_STRING_MT_TYPE_FIELD    15      /* Scr_AddClassField, 0x00472781 */

typedef struct scrThreadInfo_s {
	const char *callStack[32];  /* +0x00  innermost first */
	int         callStackCount; /* +0x80 */
	float       varUsage;       /* +0x84 */
	float       endonUsage;     /* +0x88 */
} scrThreadInfo_t;

/* ---- ThreadInfoCompare  0x00470530 ---- VERIFIED */
int __cdecl ThreadInfoCompare( const void *a, const void *b ) {
	const scrThreadInfo_t *ta = (const scrThreadInfo_t *)a;
	const scrThreadInfo_t *tb = (const scrThreadInfo_t *)b;
	int i;

	for ( i = 0; i < ta->callStackCount; i++ ) {
		if ( i >= tb->callStackCount ) {
			break;
		}
		if ( ta->callStack[i] != tb->callStack[i] ) {
			return (int)( ta->callStack[i] - tb->callStack[i] );
		}
	}
	return ta->callStackCount - tb->callStackCount;
}

/* ---- Scr_DumpScriptThreads  0x00470580 ---- VERIFIED */
void Scr_DumpScriptThreads( void ) {
	scrThreadInfo_t            *threadInfo;
	scrThreadInfo_t            *info;
	VariableValueInternal      *node;
	const VariableStackBuffer  *buf;
	const char                 *entry;
	const char                 *callStack[32];
	int                         callStackCount;
	int                         numThreads;
	int                         count;
	int                         t;
	int                         j;
	unsigned int                i;
	unsigned short              classRoot;
	unsigned short              child;
	float                       varUsage;
	float                       endonUsage;

	threadInfo = (scrThreadInfo_t *)malloc( sizeof( scrThreadInfo_t ) * VARIABLELIST_SIZE );
	if ( !threadInfo ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( threadInfo, 0, sizeof( scrThreadInfo_t ) * VARIABLELIST_SIZE );

	numThreads = 0;
	info = threadInfo;
	for ( i = 1; i < VARIABLELIST_SIZE; i++ ) {
		node = &scrVarNodes[i];
		if ( VAR_STAT( node ) == VAR_STAT_FREE || VAR_TYPE( node ) != VAR_STACK ) {
			continue;
		}
		buf = (const VariableStackBuffer *)node->u.pointerValue;
		numThreads++;

		callStackCount = 0;
		entry = (const char *)buf + SCR_STACKBUFFER_HEADER;
		for ( j = buf->size; j != 0; j-- ) {
			if ( *entry == VAR_CODEPOS ) {
				callStack[callStackCount++] = ( (const VariableUnion *)( entry + 1 ) )->codePosValue;
			}
			entry += SCR_STACKENTRY_SIZE;
		}
		callStack[callStackCount++] = buf->codePos;

		info->varUsage       = Scr_GetThreadUsage( buf, &info->endonUsage );
		info->callStackCount = callStackCount;
		for ( j = 0; j < info->callStackCount; j++ ) {
			info->callStack[j] = callStack[callStackCount - 1 - j];
		}
		info++;
	}

	qsort_m( threadInfo, numThreads, sizeof( scrThreadInfo_t ), ThreadInfoCompare );

	Com_Printf( "********************************\n" );
	t = 0;
	while ( t < numThreads ) {
		info       = &threadInfo[t];
		varUsage   = 0;
		endonUsage = 0;
		count      = 0;
		do {
			count++;
			varUsage   += threadInfo[t].varUsage;
			endonUsage += threadInfo[t].endonUsage;
			t++;
		} while ( t < numThreads && !ThreadInfoCompare( info, &threadInfo[t] ) );

		Com_Printf( "count: %d, var usage: %d, endon usage: %d\n",
					count, (int)varUsage, (int)endonUsage );
		Scr_PrintPrevCodePos( info->callStack[0], 0 );
		for ( j = 1; j < info->callStackCount; j++ ) {
			Com_Printf( "called from:\n" );
			Scr_PrintPrevCodePos( info->callStack[j], 0 );
		}
	}
	free( threadInfo );

	Com_Printf( "********************************\n" );
	for ( i = 0; i < scrVarPub_classNum; i++ ) {
		if ( !FindVariable( scrVarPub_entId, i ) ) {
			continue;
		}
		varUsage = 0;
		count    = 0;
		classRoot = VAR_OBJECTID( &scrVarNodes[
			scrVarIndirections[ FindVariableIndexInternal( scrVarPub_entId, i ) ].id ] );
		for ( child = scrVarIndirections[ scrVarNodes[classRoot].nextSibling ].id;
		      VAR_TYPE( &scrVarNodes[child] ) < VAR_THREAD && child != 0;
		      child = scrVarIndirections[ scrVarNodes[child].nextSibling ].id ) {
			count++;
			varUsage += Scr_GetObjectUsage( VAR_OBJECTID( &scrVarNodes[child] ) );
		}
		Com_Printf( "ent type '%s'... count: %d, var usage: %d\n",
					scrVarPub_classMap[i].name, count, (int)varUsage );
	}
	Com_Printf( "********************************\n" );
}

/* ---- Scr_DumpScriptVariables  0x004708F0 ---- VERIFIED */
void Scr_DumpScriptVariables( void ) {
}

/* ---- InitVariables  0x00470900 ---- VERIFIED */
void InitVariables( void ) {
	unsigned int    i;
	unsigned short  prev = 0;

	for ( i = 1; i < VARIABLELIST_SIZE; i++ ) {
		scrVarNodes[i].status = VAR_UNDEFINED;
		scrVarNodes[prev].hashNext = (unsigned short)i;
		VAR_PREVFREE( &scrVarNodes[i] ) = prev;
		scrVarIndirections[i].id = (unsigned short)i;
		prev = (unsigned short)i;
	}

	scrVarNodes[0].status = VAR_UNDEFINED;
	scrVarNodes[prev].hashNext = 0;
	scrVarIndirections[0].id = 0;
	VAR_PREVFREE( &scrVarNodes[0] ) = prev;
}

/* ---- Var_Init  0x00470970 ---- VERIFIED */
void Var_Init( void ) {
	SL_Init();
	InitVariables();
	scrVarPub_classId = AllocObject();
	scrVarPub_entId   = AllocObject();
}

/* ---- Scr_GetNumScriptVars  0x004709E0 ---- VERIFIED */
int Scr_GetNumScriptVars( void ) {
	return 0;
}

/* ---- AllocVariable  0x004713F0 ---- VERIFIED */
VariableValueInternal *AllocVariable( void ) {
	unsigned short          freeHandle;
	VariableValueInternal  *node;

	freeHandle = scrVarNodes[0].hashNext;
	if ( !freeHandle ) {
		Scr_TerminalError( "exceeded maximum number of script variables" );
	}

	node = &scrVarNodes[ scrVarIndirections[freeHandle].id ];

	scrVarNodes[0].hashNext = node->hashNext;
	VAR_PREVFREE( &scrVarNodes[ scrVarIndirections[ scrVarNodes[0].hashNext ].id ] ) = 0;

	node->hashNext    = freeHandle;
	node->nextSibling = freeHandle;
	scrVarIndirections[freeHandle].prevSibling = freeHandle;

	return node;
}

/* ---- FreeVariable  0x00471460 ---- VERIFIED */
void FreeVariable( unsigned short id ) {
	VariableValueInternal  *node = &scrVarNodes[id];
	unsigned short          handle = node->hashNext;
	unsigned short          next   = node->nextSibling;
	unsigned short          prev   = scrVarIndirections[handle].prevSibling;

	scrVarIndirections[next].prevSibling = prev;
	scrVarNodes[ scrVarIndirections[prev].id ].nextSibling = next;

	node->status &= ~VAR_STATUS_STAT_MASK;
	node->hashNext = scrVarNodes[0].hashNext;
	VAR_PREVFREE( node ) = 0;
	VAR_PREVFREE( &scrVarNodes[ scrVarIndirections[ scrVarNodes[0].hashNext ].id ] ) = handle;
	scrVarNodes[0].hashNext = handle;
}

/* ---- AllocValue  0x004714E0 ---- VERIFIED */
unsigned short AllocValue( void ) {
	VariableValueInternal *node = AllocVariable();

	node->status = VAR_STAT_EXTERNAL;
	return VAR_NODEINDEX( node );
}

/* ---- AllocObject  0x00471510 ---- VERIFIED */
unsigned short AllocObject( void ) {
	VariableValueInternal *node = AllocVariable();

	node->status = VAR_PACKED_STRUCT;
	VAR_REFCOUNT( node ) = 0;
	return VAR_NODEINDEX( node );
}

/* ---- Scr_AllocArray  0x00471580 ---- VERIFIED */
unsigned short Scr_AllocArray( void ) {
	VariableValueInternal *node = AllocVariable();

	node->status = VAR_PACKED_ARRAY;
	VAR_REFCOUNT( node )  = 0;
	VAR_ARRAYSIZE( node ) = 0;
	return VAR_NODEINDEX( node );
}

/* ---- AllocEntity  0x00471540 ---- VERIFIED */
unsigned short AllocEntity( int classnum, unsigned short entnum ) {
	VariableValueInternal *node = AllocVariable();

	node->status = ( (unsigned int)classnum << VAR_STATUS_NAME_SHIFT ) | VAR_PACKED_ENTITY;
	VAR_REFCOUNT( node ) = 0;
	VAR_ENTNUM( node )   = entnum;
	return VAR_NODEINDEX( node );
}

/* ---- AllocThread  0x004715C0 ---- VERIFIED */
unsigned short AllocThread( unsigned short self ) {
	VariableValueInternal *node = AllocVariable();

	node->status = VAR_PACKED_THREAD;
	VAR_REFCOUNT( node ) = 0;
	VAR_SELF( node )     = self;
	return VAR_NODEINDEX( node );
}

/* ---- AddRefToObject  0x004716A0 ---- VERIFIED */
void AddRefToObject( unsigned short id ) {
	VAR_REFCOUNT( &scrVarNodes[id] )++;
}

/* ---- RemoveRefToObject  0x004716B0 ---- VERIFIED */
void RemoveRefToObject( unsigned short id ) {
	if ( VAR_REFCOUNT( &scrVarNodes[id] ) == 0 ) {
		ClearObject( id );
		FreeVariable( id );
		return;
	}
	VAR_REFCOUNT( &scrVarNodes[id] )--;
}

/* ---- AllocVector  0x00471700 ---- VERIFIED */
float *AllocVector( void ) {
	unsigned short *refCount = (unsigned short *)MT_Alloc( SCR_VECTOR_ALLOC_SIZE, 2 );

	*refCount = 0;
	return (float *)( refCount + 1 );
}

/* ---- AllocVector  0x00471720 ---- VERIFIED */
VariableUnion AllocVectorValue( const float *v ) {
	VariableUnion   u;
	float          *copy = AllocVector();

	copy[0] = v[0];
	copy[1] = v[1];
	copy[2] = v[2];
	u.vectorValue = copy;
	return u;
}

/* ---- AddRefToVector  0x00471750 ---- VERIFIED */
void AddRefToVector( const float *v ) {
	( (unsigned short *)v )[-1]++;
}

/* ---- RemoveRefToVector  0x00471770 ---- VERIFIED */
void RemoveRefToVector( const float *v ) {
	unsigned short *refCount = (unsigned short *)v - 1;

	if ( *refCount == 0 ) {
		MT_Free( refCount, SCR_VECTOR_ALLOC_SIZE );
		return;
	}
	( *refCount )--;
}

/* ---- AddRefToValue  0x004717B0 ---- VERIFIED */
void AddRefToValue( int type, VariableUnion u ) {
	if ( type == VAR_VECTOR ) {
		AddRefToVector( u.vectorValue );
		return;
	}
	if ( type < VAR_FLOAT ) {
		if ( type > VAR_UNDEFINED ) {
			SL_AddRefToString( u.stringValue );
		}
		return;
	}
	if ( type == VAR_OBJECT ) {
		AddRefToObject( u.halfword[0] );
	}
}

/* ---- RemoveRefToValue  0x00471820 ---- VERIFIED */
void RemoveRefToValue( int type, VariableUnion u ) {
	if ( type == VAR_VECTOR ) {
		RemoveRefToVector( u.vectorValue );
		return;
	}
	if ( type < VAR_FLOAT ) {
		if ( type > VAR_UNDEFINED ) {
			SL_RemoveRefToString( u.stringValue );
		}
		return;
	}
	if ( type == VAR_OBJECT ) {
		RemoveRefToObject( u.halfword[0] );
	}
}

/* ---- FreeValue  0x00471610 ---- VERIFIED */
void FreeValueInternal( VariableValueInternal *node ) {
	RemoveRefToValue( VAR_TYPE( node ), node->u );
	FreeVariable( VAR_NODEINDEX( node ) );
}

/* ---- FreeValue  0x00471680 ---- VERIFIED */
void FreeValue( unsigned short id ) {
	FreeValueInternal( &scrVarNodes[id] );
}

/* ---- IsValidArrayIndex  0x00471870 ---- VERIFIED */
qboolean IsValidArrayIndex( unsigned int index ) {
	return ( index + 0x7E0000u ) < 0xFE0000u ? qtrue : qfalse;
}

/* ---- GetInternalVariableIndex  0x00471880 ---- VERIFIED */
unsigned int GetInternalVariableIndex( unsigned int index ) {
	return ( index - VAR_NAME_INDEX_BIAS ) & VAR_NAME_INDEX_MASK;
}

static unsigned short Var_HashBucket( unsigned short parentId, unsigned int name ) {
	return (unsigned short)( ( name + 31u * (unsigned int)parentId ) % 0xFFFFu + 1u );
}

/* ---- FindVariableIndexInternal  0x00470A30 ---- VERIFIED */
unsigned short FindVariableIndexInternal( unsigned short parentId, unsigned int name ) {
	unsigned short          bucket = Var_HashBucket( parentId, name );
	Variable               *bucketSlot = &scrVarIndirections[bucket];
	VariableValueInternal  *node = &scrVarNodes[ bucketSlot->id ];
	unsigned short          link;
	Variable               *slot;

	if ( VAR_STAT( node ) != VAR_STAT_HEAD ) {
		return 0;
	}
	if ( VAR_NAME( node ) == name ) {
		return bucket;
	}

	link = node->hashNext;
	slot = &scrVarIndirections[link];
	while ( slot != bucketSlot ) {
		node = &scrVarNodes[ slot->id ];
		if ( VAR_NAME( node ) == name ) {
			return link;
		}
		link = node->hashNext;
		slot = &scrVarIndirections[link];
	}
	return 0;
}

/* ---- GetVariableIndexInternal  0x00470AE0 ---- VERIFIED */
unsigned short GetVariableIndexInternal( unsigned short parentId, unsigned int name ) {
	unsigned short          bucket = Var_HashBucket( parentId, name );
	Variable               *bucketSlot = &scrVarIndirections[bucket];
	VariableValueInternal  *node = &scrVarNodes[ bucketSlot->id ];
	VariableValueInternal  *parentNode;
	unsigned short          childHandle = bucket;
	unsigned short          firstChild;
	qboolean                becomesHead = qtrue;
	unsigned int            stat = VAR_STAT( node );

	if ( stat == VAR_STAT_HEAD ) {
		unsigned short          link;
		Variable               *slot;
		VariableValueInternal  *chained;

		if ( VAR_NAME( node ) == name ) {
			return bucket;
		}

		link = node->hashNext;
		slot = &scrVarIndirections[link];
		while ( slot != bucketSlot ) {
			VariableValueInternal *walk = &scrVarNodes[ slot->id ];

			if ( VAR_NAME( walk ) == name ) {
				return link;
			}
			link = walk->hashNext;
			slot = &scrVarIndirections[link];
		}

		chained = AllocVariable();
		childHandle = chained->hashNext;
		chained->status   = VAR_STAT_CHAINED;
		chained->hashNext = scrVarNodes[ bucketSlot->id ].hashNext;
		scrVarNodes[ bucketSlot->id ].hashNext = childHandle;
		bucketSlot = &scrVarIndirections[childHandle];
		node = chained;
		becomesHead = qfalse;
	} else if ( stat == VAR_STAT_FREE ) {
		unsigned short prevFree = VAR_PREVFREE( node );
		unsigned short nextFree = node->hashNext;

		scrVarNodes[ scrVarIndirections[prevFree].id ].hashNext = nextFree;
		VAR_PREVFREE( &scrVarNodes[ scrVarIndirections[nextFree].id ] ) = prevFree;
	} else {
		VariableValueInternal  *newNode = AllocVariable();
		unsigned short          newHandle  = newNode->hashNext;
		unsigned short          newIndex   = VAR_NODEINDEX( newNode );
		unsigned short          oldIndex   = bucketSlot->id;
		unsigned short          oldPrev    = bucketSlot->prevSibling;
		unsigned short          oldNext    = node->nextSibling;

		scrVarNodes[ scrVarIndirections[oldPrev].id ].nextSibling = newHandle;
		scrVarIndirections[oldNext].prevSibling = newHandle;

		if ( stat == VAR_STAT_CHAINED ) {
			unsigned short walk = scrVarIndirections[ node->hashNext ].id;

			while ( scrVarNodes[walk].hashNext != bucket ) {
				walk = scrVarIndirections[ scrVarNodes[walk].hashNext ].id;
			}
			scrVarNodes[walk].hashNext = newHandle;
		} else {
			node->hashNext = newHandle;
		}

		scrVarIndirections[newHandle].prevSibling = bucketSlot->prevSibling;
		scrVarIndirections[newHandle].id = oldIndex;
		bucketSlot->id = newIndex;
		node = newNode;
	}

	if ( becomesHead ) {
		node->status   = VAR_STAT_HEAD;
		node->hashNext = childHandle;
	}

	parentNode = &scrVarNodes[parentId];
	firstChild = parentNode->nextSibling;
	node->nextSibling = firstChild;
	scrVarIndirections[firstChild].prevSibling = childHandle;
	bucketSlot->prevSibling = parentNode->hashNext;
	parentNode->nextSibling = childHandle;

	node->status  = (unsigned char)node->status;
	node->status |= name << VAR_STATUS_NAME_SHIFT;

	if ( VAR_TYPE( parentNode ) == VAR_ARRAY ) {
		VAR_ARRAYSIZE( parentNode )++;
		if ( name < VAR_NAME_OBJECT_BASE ) {
			SL_AddRefToString( (unsigned short)name );
		} else if ( name < VAR_NAME_OBJECT_LIMIT ) {
			AddRefToObject( (unsigned short)name );
		}
	}

	return childHandle;
}

/* ---- MakeVariableExternal  0x00470E00 ---- VERIFIED */
void MakeVariableExternal( Variable *slot, VariableValueInternal *parentNode ) {
	unsigned short          handle = VAR_HANDLEINDEX( slot );
	VariableValueInternal  *node = &scrVarNodes[ slot->id ];

	if ( VAR_TYPE( parentNode ) == VAR_ARRAY ) {
		unsigned int name;

		VAR_ARRAYSIZE( parentNode )--;
		name = VAR_NAME( node );
		if ( name < VAR_NAME_OBJECT_BASE ) {
			SL_RemoveRefToString( (unsigned short)name );
		} else if ( name < VAR_NAME_OBJECT_LIMIT ) {
			RemoveRefToObject( (unsigned short)name );
		}
	}

	if ( VAR_STAT( node ) == VAR_STAT_HEAD ) {
		unsigned short  nextHandle = node->hashNext;
		Variable       *nextSlot = &scrVarIndirections[nextHandle];

		if ( nextSlot != slot ) {
			VariableValueInternal  *nextNode = &scrVarNodes[ nextSlot->id ];
			unsigned short          removedPrev = slot->prevSibling;
			unsigned short          removedNext = node->nextSibling;
			unsigned short          promotedPrev = nextSlot->prevSibling;
			Variable                tmp;

			nextNode->status &= ~VAR_STATUS_STAT_MASK;
			nextNode->status |= VAR_STAT_HEAD;

			scrVarIndirections[ nextNode->nextSibling ].prevSibling = handle;
			scrVarNodes[ scrVarIndirections[promotedPrev].id ].nextSibling = handle;
			scrVarIndirections[removedNext].prevSibling = nextHandle;
			scrVarNodes[ scrVarIndirections[removedPrev].id ].nextSibling = nextHandle;

			tmp = *slot;
			*slot = *nextSlot;
			*nextSlot = tmp;

			node->status  |= VAR_STAT_EXTERNAL;
			node->hashNext = nextHandle;
			return;
		}
	} else {
		Variable               *prevSlot = slot;
		Variable               *walkSlot = slot;
		VariableValueInternal  *walkNode = node;

		do {
			prevSlot = walkSlot;
			walkSlot = &scrVarIndirections[ walkNode->hashNext ];
			walkNode = &scrVarNodes[ walkSlot->id ];
		} while ( walkSlot != slot );

		scrVarNodes[ prevSlot->id ].hashNext = node->hashNext;
	}

	node->status  |= VAR_STAT_EXTERNAL;
	node->hashNext = handle;
}

/* ---- FindArrayVariableIndex  0x00471890 ---- VERIFIED */
unsigned short FindArrayVariableIndex( unsigned short parentId, unsigned int index ) {
	return FindVariableIndexInternal( parentId, GetInternalVariableIndex( index ) );
}

/* ---- FindArrayVariable  0x004718B0 ---- VERIFIED */
unsigned short FindArrayVariable( unsigned short parentId, int index ) {
	return scrVarIndirections[ FindArrayVariableIndex( parentId, (unsigned int)index ) ].id;
}

/* ---- FindVariable  0x004718E0 ---- VERIFIED */
unsigned short FindVariable( unsigned short parentId, unsigned int name ) {
	return scrVarIndirections[ FindVariableIndexInternal( parentId, name ) ].id;
}

/* ---- FindObjectVariable  0x00471900 ---- VERIFIED */
unsigned short FindObjectVariable( unsigned short parentId, unsigned short childId ) {
	return scrVarIndirections[
		FindVariableIndexInternal( parentId, (unsigned int)childId + VAR_NAME_OBJECT_BASE ) ].id;
}

/* ---- GetArrayVariableIndex  0x00471930 ---- VERIFIED */
unsigned short GetArrayVariableIndex( unsigned short parentId, unsigned int index ) {
	return GetVariableIndexInternal( parentId, GetInternalVariableIndex( index ) );
}

/* ---- GetArrayVariable  0x00471B00 ---- VERIFIED */
unsigned short GetArrayVariable( unsigned short parentId, int index ) {
	return scrVarIndirections[ GetArrayVariableIndex( parentId, (unsigned int)index ) ].id;
}

/* ---- GetVariable  0x00471B40 ---- VERIFIED */
unsigned short GetVariable( unsigned short parentId, unsigned int name ) {
	return scrVarIndirections[ GetVariableIndexInternal( parentId, name ) ].id;
}

/* ---- GetObjectVariable  0x00471B60 ---- VERIFIED */
unsigned short GetObjectVariable( unsigned short parentId, unsigned short childId ) {
	return scrVarIndirections[
		GetVariableIndexInternal( parentId, (unsigned int)childId + VAR_NAME_OBJECT_BASE ) ].id;
}

/* ---- GetVariableField  0x00471950 ---- VERIFIED */
unsigned short GetVariableField( unsigned short parentId, unsigned short name ) {
	unsigned short          handle;
	VariableValueInternal  *parentNode;
	VariableValueInternal  *tempNode;
	unsigned short          fieldId;
	int                     parentType;

	handle = FindVariableIndexInternal( parentId, name );
	if ( handle ) {
		return scrVarIndirections[handle].id;
	}

	parentNode = &scrVarNodes[parentId];
	parentType = VAR_TYPE( parentNode );

	if ( parentType >= VAR_DEADTHREAD ) {
		ClearVariableValue( scrVarPub_tempVariable );
		return scrVarPub_tempVariable;
	}

	if ( parentType != VAR_ENTITY ) {
		return scrVarIndirections[ GetVariableIndexInternal( parentId, name ) ].id;
	}

	fieldId = FindArrayVariable( scrVarPub_classMap[ VAR_NAME( parentNode ) ].id, name );
	if ( !fieldId ) {
		return scrVarIndirections[ GetVariableIndexInternal( parentId, name ) ].id;
	}

	ClearVariableValue( scrVarPub_tempVariable );
	tempNode = &scrVarNodes[ scrVarPub_tempVariable ];
	tempNode->status = ( parentNode->status & ~0xFFu ) | tempNode->status | VAR_KEYVALUE;
	VAR_KVENTNUM( tempNode ) = VAR_ENTNUM( parentNode );
	VAR_KVOFFSET( tempNode ) = VAR_ARRAYSIZE( &scrVarNodes[fieldId] );

	return scrVarPub_tempVariable;
}

/* ---- ClearVariableField  0x00471A70 ---- VERIFIED */
void ClearVariableField( unsigned short parentId, unsigned short name ) {
	VariableValueInternal *parentNode;

	if ( FindVariableIndexInternal( parentId, name ) ) {
		RemoveVariable( parentId, name );
		return;
	}

	parentNode = &scrVarNodes[parentId];
	if ( VAR_TYPE( parentNode ) == VAR_ENTITY ) {
		if ( FindArrayVariable( scrVarPub_classMap[ VAR_NAME( parentNode ) ].id, name ) ) {
			Scr_Error( "cannot set entity builtin key/value to undefined" );
		}
	}
}

/* ---- RemoveVariable  0x00471B80 ---- VERIFIED */
void RemoveVariable( unsigned short parentId, unsigned int name ) {
	unsigned short handle = FindVariableIndexInternal( parentId, name );
	unsigned short childId = scrVarIndirections[handle].id;

	MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[parentId] );
	FreeValueInternal( &scrVarNodes[childId] );
}

/* ---- RemoveObjectVariable  0x00471BD0 ---- VERIFIED */
void RemoveObjectVariable( unsigned short parentId, unsigned short childId ) {
	RemoveVariable( parentId, (unsigned int)childId + VAR_NAME_OBJECT_BASE );
}

/* ---- SafeRemoveArrayVariable  0x00471C20 ---- VERIFIED */
void SafeRemoveArrayVariable( unsigned short parentId, unsigned int index ) {
	SafeRemoveVariable( parentId, GetInternalVariableIndex( index ) );
}

/* ---- RemoveArrayVariable  0x00471C80 ---- VERIFIED */
void RemoveArrayVariable( unsigned short parentId, unsigned int index ) {
	RemoveVariable( parentId, GetInternalVariableIndex( index ) );
}

/* ---- SafeRemoveVariable  0x00471CD0 ---- VERIFIED */
void SafeRemoveVariable( unsigned short parentId, unsigned int name ) {
	unsigned short handle = FindVariableIndexInternal( parentId, name );
	unsigned short childId;

	if ( !handle ) {
		return;
	}
	childId = scrVarIndirections[handle].id;
	MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[parentId] );
	FreeValueInternal( &scrVarNodes[childId] );
}

/* ---- ClearObjectInternal  0x00470F70 ---- VERIFIED */
void ClearObjectInternal( unsigned short id ) {
	Variable       *slot;
	unsigned short  childId;

	slot = &scrVarIndirections[ scrVarNodes[id].nextSibling ];
	childId = slot->id;
	while ( childId != id ) {
		MakeVariableExternal( slot, &scrVarNodes[id] );
		slot = &scrVarIndirections[ scrVarNodes[childId].nextSibling ];
		childId = slot->id;
	}

	childId = scrVarIndirections[ scrVarNodes[id].nextSibling ].id;
	while ( childId != id ) {
		unsigned short next = scrVarIndirections[ scrVarNodes[childId].nextSibling ].id;

		FreeValueInternal( &scrVarNodes[childId] );
		childId = next;
	}
}

/* ---- ClearObject  0x00471010 ---- VERIFIED */
void ClearObject( unsigned short id ) {
	AddRefToObject( id );
	ClearObjectInternal( id );
	RemoveRefToObject( id );
}

/* ---- GetThreadNotifyName  0x00471040 ---- VERIFIED */
unsigned short GetThreadNotifyName( unsigned short id ) {
	return (unsigned short)VAR_NAME( &scrVarNodes[id] );
}

/* ---- SetThreadNotifyName  0x00471060 ---- VERIFIED */
void SetThreadNotifyName( unsigned short id, unsigned short name ) {
	SL_AddRefToString( name );
	scrVarNodes[id].status |= (unsigned int)name << VAR_STATUS_NAME_SHIFT;
}

/* ---- ClearThreadNotifyName  0x00471090 ---- VERIFIED */
void ClearThreadNotifyName( unsigned short id ) {
	SL_RemoveRefToString( GetThreadNotifyName( id ) );
	scrVarNodes[id].status = (unsigned char)scrVarNodes[id].status;
}

/* ---- KillThread  0x00471100 ---- VERIFIED */
void KillThread( unsigned short id ) {
	unsigned short endonId;
	unsigned short self;
	unsigned short pauseSlot;
	unsigned short bucket;

	AddRefToObject( id );
	ClearObjectInternal( id );

	endonId = VAR_SELF( &scrVarNodes[id] );
	RemoveRefToObject( endonId );

	self = VAR_SELF( &scrVarNodes[id] );

	pauseSlot = FindObjectVariable( scrVarPub_pauseArrayId, id );
	if ( pauseSlot ) {
		bucket = VAR_OBJECTID( &scrVarNodes[pauseSlot] );

		for ( ;; ) {
			unsigned short          entry;
			unsigned short          waiter;
			unsigned short          handle;
			VariableValueInternal  *entryNode;

			entry = FindNextSibling( bucket );
			if ( !entry ) {
				break;
			}
			waiter = (unsigned short)VAR_NAME( &scrVarNodes[entry] );

			handle = FindVariableIndexInternal( bucket,
					(unsigned int)waiter + VAR_NAME_OBJECT_BASE );
			VM_CancelNotify__FUsUs( waiter,
					VAR_OBJECTID( &scrVarNodes[ scrVarIndirections[handle].id ] ) );

			scrVarNodes[waiter].status =
				( scrVarNodes[waiter].status & ~VAR_STATUS_TYPE_MASK ) | VAR_DEADTHREAD;

			handle = FindVariableIndexInternal( bucket,
					(unsigned int)waiter + VAR_NAME_OBJECT_BASE );
			entryNode = &scrVarNodes[ scrVarIndirections[handle].id ];
			MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[bucket] );
			FreeValueInternal( entryNode );

			RemoveRefToObject( id );
		}

		RemoveObjectVariable( scrVarPub_pauseArrayId, id );
	}

	if ( GetThreadNotifyName( id ) ) {
		pauseSlot = FindObjectVariable( scrVarPub_pauseArrayId, self );
		bucket    = VAR_OBJECTID( &scrVarNodes[pauseSlot] );

		VM_CancelNotify__FUsUs( id,
			VAR_OBJECTID( &scrVarNodes[ FindObjectVariable( bucket, id ) ] ) );

		bucket = VAR_OBJECTID( &scrVarNodes[
			FindObjectVariable( scrVarPub_pauseArrayId, self ) ] );
		RemoveObjectVariable( bucket, id );

		if ( VAR_ARRAYSIZE( &scrVarNodes[bucket] ) == 0 ) {
			RemoveObjectVariable( scrVarPub_pauseArrayId, self );
		}
	}

	scrVarNodes[id].status = ( scrVarNodes[id].status & ~VAR_STATUS_TYPE_MASK ) | VAR_DEADTHREAD;
	RemoveRefToObject( id );
}

/* ---- SetVariableValue  0x00471E50 ---- VERIFIED */
void SetVariableValue( unsigned short id, VariableValue *value ) {
	VariableValueInternal *node = &scrVarNodes[id];

	RemoveRefToValue( VAR_TYPE( node ), node->u );
	node->status &= ~VAR_STATUS_TYPE_MASK;
	node->status |= (unsigned int)value->type;
	node->u = value->u;
}

/* ---- SetNewVariableValue  0x00471ED0 ---- VERIFIED */
void SetNewVariableValue( unsigned short id, VariableValue *value ) {
	VariableValueInternal *node = &scrVarNodes[id];

	node->status |= (unsigned int)value->type;
	node->u = value->u;
}

/* ---- GetVariableValueAddress  0x00471EF0 ---- VERIFIED */
VariableValue *GetVariableValueAddress( unsigned short id ) {
	return (VariableValue *)&scrVarNodes[id];
}

/* ---- ClearVariableValue  0x00471F00 ---- VERIFIED */
void ClearVariableValue( unsigned short id ) {
	VariableValueInternal *node = &scrVarNodes[id];

	RemoveRefToValue( VAR_TYPE( node ), node->u );
	node->status &= VAR_STATUS_STAT_MASK;
}

/* ---- SetVariableFieldValue  0x00471F80 ---- VERIFIED */
void SetVariableFieldValue( unsigned short id, VariableValue *value ) {
	VariableValueInternal *node = &scrVarNodes[id];

	if ( VAR_TYPE( node ) == VAR_KEYVALUE ) {
		SetEntityFieldValue__FUiiiP13VariableValue( VAR_NAME( node ),
			VAR_KVENTNUM( node ), VAR_KVOFFSET( node ), value );
		return;
	}
	SetVariableValue( id, value );
}

/* ---- GetVariableValue  0x00472020 ---- VERIFIED */
void GetVariableValue( unsigned short id, VariableValue *value ) {
	value->type = VAR_TYPE( &scrVarNodes[id] );
	value->u = scrVarNodes[id].u;
}

/* ---- GetVariableFieldValue  0x00472040 ---- VERIFIED */
void GetVariableFieldValue( unsigned short id, VariableValue *value ) {
	VariableValueInternal *node = &scrVarNodes[id];

	if ( VAR_TYPE( node ) == VAR_KEYVALUE ) {
		GetEntityFieldValue__FUiiiP13VariableValue( VAR_NAME( node ),
			VAR_KVENTNUM( node ), VAR_KVOFFSET( node ), value );

		if ( value->type == VAR_OBJECT
		  && GetVarType( value->u.halfword[0] ) == VAR_ARRAY ) {
			unsigned short source = value->u.halfword[0];

			RemoveRefToObject( source );
			value->u.halfword[0] = Scr_AllocArray();
			CopyArray( source, value->u.halfword[0] );
		}
		return;
	}

	GetVariableValue( id, value );
	AddRefToValue( value->type, value->u );
}

/* ---- GetSizeValue  0x00472140 ---- VERIFIED */
void GetSizeValue( VariableValue *value ) {
	if ( value->type == VAR_OBJECT ) {
		unsigned short object = value->u.halfword[0];

		value->type = VAR_INTEGER;
		if ( GetVarType( object ) == VAR_ARRAY ) {
			value->u.intValue = VAR_ARRAYSIZE( &scrVarNodes[object] );
		} else {
			value->u.intValue = 1;
		}
		RemoveRefToObject( object );
		return;
	}

	if ( value->type == VAR_STRING ) {
		unsigned short  handle = value->u.stringValue;
		const char     *text = SL_ConvertToString( handle );

		value->type = VAR_INTEGER;
		value->u.intValue = (int)strlen( text );
		SL_RemoveRefToString( handle );
		return;
	}

	Scr_Error( va( "size cannot be applied to %s", var_typename[value->type] ) );
}

/* ---- GetArraySize  0x004721F0 ---- VERIFIED */
unsigned short GetArraySize( unsigned short id ) {
	return VAR_ARRAYSIZE( &scrVarNodes[id] );
}

/* ---- FindNextSibling  0x00472200 ---- VERIFIED */
unsigned short FindNextSibling( unsigned short id ) {
	unsigned short next = scrVarIndirections[ scrVarNodes[id].nextSibling ].id;

	return VAR_TYPE( &scrVarNodes[next] ) < VAR_THREAD ? next : 0;
}

/* ---- GetVariableName  0x00472230 ---- VERIFIED */
unsigned int GetVariableName( unsigned short id ) {
	return VAR_NAME( &scrVarNodes[id] );
}

/* ---- GetVariableKeyObject  0x004709F0 ---- VERIFIED */
unsigned short GetVariableKeyObject( unsigned short id ) {
	return (unsigned short)VAR_NAME( &scrVarNodes[id] );
}

/* ---- GetEntityType  0x00470A10 ---- VERIFIED */
unsigned int GetEntityType( unsigned short id ) {
	return VAR_NAME( &scrVarNodes[id] );
}

/* ---- GetSelf  0x00471600 ---- VERIFIED */
unsigned short GetSelf( unsigned short id ) {
	return VAR_SELF( &scrVarNodes[id] );
}

/* ---- GetEntnum  0x00473580 ---- VERIFIED */
unsigned short GetEntnum( unsigned short id ) {
	return VAR_ENTNUM( &scrVarNodes[id] );
}

/* ---- GetObject  0x00472250 ---- VERIFIED */
unsigned short GetObject( unsigned short id ) {
	VariableValueInternal *node = &scrVarNodes[id];

	if ( VAR_TYPE( node ) == VAR_UNDEFINED ) {
		node->status |= VAR_OBJECT;
		VAR_OBJECTID( node ) = AllocObject();
	}
	return VAR_OBJECTID( node );
}

/* ---- GetArray  0x004722A0 ---- VERIFIED */
unsigned short GetArray( unsigned short id ) {
	VariableValueInternal *node = &scrVarNodes[id];

	if ( VAR_TYPE( node ) == VAR_UNDEFINED ) {
		node->status |= VAR_OBJECT;
		VAR_OBJECTID( node ) = Scr_AllocArray();
	}
	return VAR_OBJECTID( node );
}

/* ---- FindObject  0x00472300 ---- VERIFIED */
unsigned short FindObject( unsigned short id ) {
	return VAR_OBJECTID( &scrVarNodes[id] );
}

/* ---- IsFieldObject  0x00472310 ---- VERIFIED */
qboolean IsFieldObject( unsigned short id ) {
	return VAR_TYPE( &scrVarNodes[id] ) < VAR_ARRAY ? qtrue : qfalse;
}

/* ---- Scr_IsThreadAlive  0x00472330 ---- VERIFIED */
qboolean Scr_IsThreadAlive( unsigned short id ) {
	return VAR_TYPE( &scrVarNodes[id] ) == VAR_THREAD ? qtrue : qfalse;
}

/* ---- GetVarType  0x00472420 ---- VERIFIED */
int GetVarType( unsigned short id ) {
	return VAR_TYPE( &scrVarNodes[id] );
}

/* ---- CastFieldObject  0x00472350 ---- VERIFIED */
unsigned short CastFieldObject( VariableValue *value ) {
	unsigned short  object;
	VariableValue   objectValue;

	if ( value->type != VAR_OBJECT ) {
		Scr_Error( va( "%s is not an object", var_typename[value->type] ) );
		return 0;
	}

	object = value->u.halfword[0];
	if ( !IsFieldObject( object ) ) {
		Scr_Error( va( "%s is not an object", var_typename[ GetVarType( object ) ] ) );
		return 0;
	}

	objectValue.u.halfword[0] = object;
	objectValue.u.halfword[1] = 0;
	objectValue.type = VAR_OBJECT;
	SetVariableValue( scrVarPub_tempVariable, &objectValue );
	return object;
}

/* ---- CopyArray  0x00471D20 ---- VERIFIED */
void CopyArray( unsigned short sourceId, unsigned short destId ) {
	unsigned short childId = scrVarIndirections[ scrVarNodes[sourceId].nextSibling ].id;

	while ( childId != sourceId ) {
		VariableValueInternal  *childNode = &scrVarNodes[childId];
		int                     childType = GetVarType( childId );
		unsigned short          destChild = GetVariable( destId, GetVariableName( childId ) );
		VariableValueInternal  *destNode = &scrVarNodes[destChild];

		destNode->status |= (unsigned int)childType;

		if ( childType == VAR_OBJECT ) {
			unsigned short childObject = VAR_OBJECTID( childNode );

			if ( GetVarType( childObject ) == VAR_ARRAY ) {
				VAR_OBJECTID( destNode ) = Scr_AllocArray();
				CopyArray( childObject, VAR_OBJECTID( destNode ) );
			} else {
				VAR_OBJECTID( destNode ) = childObject;
				AddRefToObject( childObject );
			}
		} else {
			destNode->u = childNode->u;
			AddRefToValue( childType, childNode->u );
		}

		childId = scrVarIndirections[ scrVarNodes[childId].nextSibling ].id;
	}
}

/* ---- EvalArray  0x00472970 ---- VERIFIED */
void EvalArray( VariableValue *index, VariableValue *container ) {
	unsigned short  arrayId;
	unsigned short  handle;
	unsigned short  childId;

	if ( container->type == VAR_STRING ) {
		unsigned short  stringHandle;
		const char     *text;
		int             at;
		char            single[2];

		if ( index->type != VAR_INTEGER ) {
			Scr_Error( va( "%s is not a string index", var_typename[index->type] ) );
		}
		at = index->u.intValue;
		stringHandle = container->u.stringValue;
		text = SL_ConvertToString( stringHandle );
		if ( at < 0 || (unsigned int)at >= strlen( text ) ) {
			Scr_Error( va( "string index %d out of range", at ) );
		}
		single[0] = text[at];
		single[1] = '\0';
		index->type = VAR_STRING;
		index->u.stringValue = SL_GetStringOfLen( single, 0, 2, SCR_STRING_MT_TYPE_TEMP );
		SL_RemoveRefToString( stringHandle );
		return;
	}

	if ( container->type == VAR_VECTOR ) {
		if ( index->type != VAR_INTEGER ) {
			Scr_Error( va( "%s is not a vector index", var_typename[index->type] ) );
		}
		if ( index->u.uintValue >= 3 ) {
			Scr_Error( va( "vector index %d out of range", index->u.intValue ) );
		}
		{
			const float *v = container->u.vectorValue;
			unsigned int component = index->u.uintValue;

			index->u.floatValue = v[component];
			index->type = VAR_FLOAT;
			RemoveRefToVector( v );
		}
		return;
	}

	if ( container->type != VAR_OBJECT ) {
		scrVarPub_error_index = 1;
		Scr_Error( va( "%s is not an array, string, or vector",
			var_typename[container->type] ) );
		return;
	}

	arrayId = container->u.halfword[0];
	if ( GetVarType( arrayId ) != VAR_ARRAY ) {
		scrVarPub_error_index = 1;
		Scr_Error( va( "%s is not an array", var_typename[ GetVarType( arrayId ) ] ) );
	}

	if ( index->type == VAR_STRING ) {
		unsigned short stringHandle = index->u.stringValue;

		handle = FindVariableIndexInternal( arrayId, stringHandle );
		SL_RemoveRefToString( stringHandle );
	} else if ( index->type == VAR_INTEGER ) {
		if ( !IsValidArrayIndex( index->u.uintValue ) ) {
			Scr_Error( va( "array index %d out of range", index->u.intValue ) );
		}
		handle = FindArrayVariableIndex( arrayId, index->u.uintValue );
	} else {
		Scr_Error( va( "%s is not an array index", var_typename[index->type] ) );
		return;
	}

	childId = scrVarIndirections[handle].id;
	if ( !childId ) {
		RemoveRefToObject( arrayId );
		index->type = VAR_UNDEFINED;
		return;
	}

	GetVariableValue( childId, index );
	AddRefToValue( index->type, index->u );
	RemoveRefToObject( arrayId );
}

/* Var_ResolveArrayObject -- the KEYVALUE unwrapping the three array functions
   share.  Retail (0x00472C2D, 0x00472F1C, 0x0047329D) hands GetEntityFieldValue
   the VM stack slot ABOVE the index, index + 1, as the scratch value: it sets
   scrVmPub_top = index and reads the result from index[1].  A C local here is
   wrong, not merely different: IncInParam refuses any top above scrVmPub_maxstack
   (0x0047C58D), and a C-stack local sits above the .bss stack whenever the loader
   puts the thread stack above the image -- "Internal script stack overflow" on
   the first self.field[i] a script evaluates. */
static int Var_ResolveArrayObject( VariableValueInternal *node, unsigned short *arrayId,
								   VariableValue *index ) {
	int type = VAR_TYPE( node );

	*arrayId = VAR_OBJECTID( node );

	while ( type == VAR_KEYVALUE ) {
		GetEntityFieldValue__FUiiiP13VariableValue( VAR_NAME( node ),
			VAR_KVENTNUM( node ), VAR_KVOFFSET( node ), index + 1 );
		*arrayId = index[1].u.halfword[0];
		type = index[1].type;
		RemoveRefToValue( type, index[1].u );
	}
	return type;
}

/* ---- Scr_EvalArrayIndex_m  0x00472BE0 ---- VERIFIED */
unsigned short Scr_EvalArrayIndex_m( unsigned short id, VariableValue *index ) {
	unsigned short  arrayId;
	unsigned short  handle;
	int             type;

	type = Var_ResolveArrayObject( &scrVarNodes[id], &arrayId, index );
	if ( type != VAR_OBJECT ) {
		scrVarPub_error_index = 1;
		Scr_Error( va( "%s is not an array", var_typename[type] ) );
		return 0;
	}

	if ( GetVarType( arrayId ) != VAR_ARRAY ) {
		scrVarPub_error_index = 1;
		Scr_Error( va( "%s is not an array", var_typename[ GetVarType( arrayId ) ] ) );
	}

	if ( index->type == VAR_INTEGER ) {
		if ( !IsValidArrayIndex( index->u.uintValue ) ) {
			Scr_Error( va( "array index %d out of range", index->u.intValue ) );
		}
		handle = FindArrayVariableIndex( arrayId, index->u.uintValue );
		if ( !handle ) {
			Scr_Error( "array index does not exist" );
		}
		return scrVarIndirections[handle].id;
	}

	if ( index->type != VAR_STRING ) {
		Scr_Error( va( "%s is not an array index", var_typename[index->type] ) );
		return 0;
	}

	handle = FindVariableIndexInternal( arrayId, index->u.stringValue );
	if ( !handle ) {
		Scr_Error( "array index does not exist" );
	}
	SL_RemoveRefToString( index->u.stringValue );
	return scrVarIndirections[handle].id;
}

/* ---- EvalArrayRef  0x00472EC0 ---- VERIFIED */
unsigned short EvalArrayRef( unsigned short id, VariableValue *index ) {
	VariableValueInternal  *node = &scrVarNodes[id];
	unsigned short          arrayId;
	unsigned short          handle;
	int                     type;

	type = Var_ResolveArrayObject( node, &arrayId, index );

	if ( type == VAR_UNDEFINED ) {
		arrayId = Scr_AllocArray();
		node->status |= VAR_OBJECT;
		VAR_OBJECTID( node ) = arrayId;
	} else if ( type != VAR_OBJECT ) {
		scrVarPub_error_index = 1;
		if ( type == VAR_STRING ) {
			Scr_Error( "string characters cannot be individually changed" );
		} else if ( type == VAR_VECTOR ) {
			Scr_Error( "vector components cannot be individually changed" );
		} else {
			Scr_Error( va( "%s is not an array", var_typename[type] ) );
		}
		return 0;
	} else {
		if ( GetVarType( arrayId ) != VAR_ARRAY ) {
			scrVarPub_error_index = 1;
			Scr_Error( va( "%s is not an array", var_typename[ GetVarType( arrayId ) ] ) );
		}

		if ( VAR_REFCOUNT( &scrVarNodes[arrayId] ) ) {
			unsigned short source = arrayId;

			RemoveRefToObject( source );
			arrayId = Scr_AllocArray();
			CopyArray( source, arrayId );
			VAR_OBJECTID( node ) = arrayId;
		}
	}

	if ( index->type == VAR_INTEGER ) {
		if ( !IsValidArrayIndex( index->u.uintValue ) ) {
			Scr_Error( va( "array index %d out of range", index->u.intValue ) );
		}
		return scrVarIndirections[ GetArrayVariableIndex( arrayId, index->u.uintValue ) ].id;
	}

	if ( index->type != VAR_STRING ) {
		Scr_Error( va( "%s is not an array index", var_typename[index->type] ) );
		return 0;
	}

	handle = GetVariableIndexInternal( arrayId, index->u.stringValue );
	SL_RemoveRefToString( index->u.stringValue );
	return scrVarIndirections[handle].id;
}

/* ---- ClearArray  0x00473250 ---- VERIFIED */
void ClearArray( unsigned short id, VariableValue *index ) {
	unsigned short  arrayId;
	int             type;

	type = Var_ResolveArrayObject( &scrVarNodes[id], &arrayId, index );
	if ( type != VAR_OBJECT ) {
		scrVarPub_error_index = 1;
		Scr_Error( va( "%s is not an array", var_typename[type] ) );
		return;
	}

	if ( GetVarType( arrayId ) != VAR_ARRAY ) {
		scrVarPub_error_index = 1;
		Scr_Error( va( "%s is not an array", var_typename[ GetVarType( arrayId ) ] ) );
	}

	if ( index->type == VAR_INTEGER ) {
		if ( !IsValidArrayIndex( index->u.uintValue ) ) {
			Scr_Error( va( "array index %d out of range", index->u.intValue ) );
		}
		SafeRemoveArrayVariable( arrayId, index->u.uintValue );
		return;
	}

	if ( index->type != VAR_STRING ) {
		Scr_Error( va( "%s is not an array index", var_typename[index->type] ) );
		return;
	}

	SafeRemoveVariable( arrayId, index->u.stringValue );
	SL_RemoveRefToString( index->u.stringValue );
}

/* ---- GetEmptyArray  0x004734F0 ---- VERIFIED */
void GetEmptyArray( VariableValue *value ) {
	value->type = VAR_OBJECT;
	value->u.halfword[0] = Scr_AllocArray();
}

/* ---- SetEmptyArray  0x00473530 ---- VERIFIED */
void SetEmptyArray( unsigned short id ) {
	VariableValueInternal *node = &scrVarNodes[id];

	node->status |= VAR_OBJECT;
	VAR_OBJECTID( node ) = Scr_AllocArray();
}

/* ---- Scr_SetClassMap  0x00472500 ---- VERIFIED */
void Scr_SetClassMap( scrClassMapEntry_t *classMap, unsigned int classCount ) {
	unsigned int i;

	scrVarPub_classMap = classMap;
	scrVarPub_classNum = classCount;

	for ( i = 0; i < classCount; i++ ) {
		VariableValueInternal  *node;
		unsigned short          child;

		child = scrVarIndirections[ GetVariableIndexInternal( scrVarPub_entId, i ) ].id;
		node  = &scrVarNodes[child];
		if ( VAR_TYPE( node ) == VAR_UNDEFINED ) {
			node->status |= VAR_OBJECT;
			VAR_OBJECTID( node ) = Scr_AllocArray();
		}

		child = scrVarIndirections[ GetVariableIndexInternal( scrVarPub_classId, i ) ].id;
		node  = &scrVarNodes[child];
		if ( VAR_TYPE( node ) == VAR_UNDEFINED ) {
			node->status |= VAR_OBJECT;
			VAR_OBJECTID( node ) = Scr_AllocArray();
		}

		classMap[i].id = VAR_OBJECTID( node );
	}
}

/* ---- Scr_RemoveClassMap  0x00472620 ---- VERIFIED */
void Scr_RemoveClassMap( void ) {
	unsigned int i;

	if ( !scr_inited ) {
		return;
	}

	for ( i = 0; i < scrVarPub_classNum; i++ ) {
		unsigned short handle;

		handle = FindVariableIndexInternal( scrVarPub_classId, i );
		if ( handle ) {
			unsigned short childId = scrVarIndirections[handle].id;

			MakeVariableExternal( &scrVarIndirections[handle],
				&scrVarNodes[ scrVarPub_classId ] );
			FreeValueInternal( &scrVarNodes[childId] );
		}

		handle = FindVariableIndexInternal( scrVarPub_entId, i );
		if ( handle ) {
			unsigned short childId = scrVarIndirections[handle].id;

			MakeVariableExternal( &scrVarIndirections[handle],
				&scrVarNodes[ scrVarPub_entId ] );
			FreeValueInternal( &scrVarNodes[childId] );
		}
	}
}

/* ---- Scr_AddClassField  0x004726F0 ---- VERIFIED */
void Scr_AddClassField( unsigned short classId, const char *name, unsigned short offset ) {
	unsigned short          canonical;
	unsigned short          handle;
	unsigned short          childId;
	VariableValueInternal  *node;

	canonical = scrString_canonicalMap[ SL_FindStringOfLen( name, strlen( name ) + 1 ) ];
	if ( canonical ) {
		childId = scrVarIndirections[
			GetVariableIndexInternal( classId, GetInternalVariableIndex( canonical ) ) ].id;
		node = &scrVarNodes[childId];
		node->status = ( node->status & ~VAR_STATUS_TYPE_MASK ) | VAR_INTEGER;
		VAR_KVOFFSET( node ) = offset;
	}

	handle = SL_GetStringOfLen( name, 0, strlen( name ) + 1, SCR_STRING_MT_TYPE_FIELD );
	childId = scrVarIndirections[ GetVariableIndexInternal( classId, handle ) ].id;
	SL_RemoveRefToString( handle );

	node = &scrVarNodes[childId];
	VAR_KVOFFSET( node ) = offset;
	node->status = ( node->status & ~VAR_STATUS_TYPE_MASK ) | VAR_INTEGER;
}

/* ---- Scr_GetOffset  0x00472800 ---- VERIFIED */
int Scr_GetOffset( unsigned short classId, const char *name ) {
	unsigned short handle = SL_ConvertFromString( name );
	unsigned short childId = FindVariable( classId, handle );

	if ( !childId ) {
		return -1;
	}
	return VAR_KVOFFSET( &scrVarNodes[childId] );
}

/* ---- FindEntityId  0x00472850 ---- VERIFIED */
unsigned short FindEntityId( int entnum, int classnum ) {
	unsigned short classRoot;
	unsigned short childId;

	classRoot = VAR_OBJECTID( &scrVarNodes[
		scrVarIndirections[ FindVariableIndexInternal( scrVarPub_entId, classnum ) ].id ] );

	childId = FindArrayVariable( classRoot, entnum );
	if ( !childId ) {
		return 0;
	}
	return VAR_OBJECTID( &scrVarNodes[childId] );
}

/* ---- Scr_GetEntityId  0x004728B0 ---- VERIFIED */
unsigned short Scr_GetEntityId( int entnum, int classnum ) {
	unsigned short          classRoot;
	unsigned short          childId;
	VariableValueInternal  *node;

	classRoot = VAR_OBJECTID( &scrVarNodes[
		scrVarIndirections[ FindVariableIndexInternal( scrVarPub_entId, classnum ) ].id ] );

	childId = GetArrayVariable( classRoot, entnum );
	node = &scrVarNodes[childId];

	if ( VAR_TYPE( node ) != VAR_UNDEFINED ) {
		return VAR_OBJECTID( node );
	}

	VAR_OBJECTID( node ) = AllocEntity( classnum, (unsigned short)entnum );
	node->status |= VAR_OBJECT;
	return VAR_OBJECTID( node );
}

/* ---- Scr_FreeEntityNum  0x00472440 ---- VERIFIED */
void Scr_FreeEntityNum( int entnum, int classnum ) {
	unsigned short classRoot;
	unsigned short childId;
	unsigned short entityId;

	if ( !scr_inited ) {
		return;
	}

	classRoot = VAR_OBJECTID( &scrVarNodes[
		scrVarIndirections[ FindVariableIndexInternal( scrVarPub_entId, classnum ) ].id ] );

	childId = FindArrayVariable( classRoot, entnum );
	if ( !childId ) {
		return;
	}

	entityId = VAR_OBJECTID( &scrVarNodes[childId] );
	scrVarNodes[entityId].status =
		( scrVarNodes[entityId].status & ~VAR_STATUS_TYPE_MASK ) | VAR_DEADENTITY;

	AddRefToObject( entityId );
	VM_TerminateNotifyList__FUs( entityId );
	ClearObjectInternal( entityId );
	RemoveRefToObject( entityId );

	RemoveArrayVariable( classRoot, entnum );
}

/* ---- CopyEntity  0x00473590 ---- VERIFIED */
void CopyEntity( unsigned short sourceId, unsigned short destId ) {
	unsigned short childId = FindNextSibling( sourceId );

	while ( childId ) {
		VariableValueInternal *childNode = &scrVarNodes[childId];

		if ( VAR_NAME( childNode ) != VAR_NAME_OBJECT_LIMIT ) {
			unsigned short          destChild;
			VariableValueInternal  *destNode;
			unsigned int            status;

			destChild = scrVarIndirections[
				GetVariableIndexInternal( destId, VAR_NAME( childNode ) ) ].id;
			destNode = &scrVarNodes[destChild];

			status = childNode->status & ~VAR_STATUS_STAT_MASK;
			destNode->status |= status;
			destNode->u = childNode->u;
			AddRefToValue( (int)( status & VAR_STATUS_TYPE_MASK ), childNode->u );
		}

		childId = FindNextSibling( childId );
	}
}

/* ---- Scr_CopyEntityNum  0x00473660 ---- VERIFIED */
void Scr_CopyEntityNum( int fromEntnum, int toEntnum, int classnum ) {
	unsigned short sourceId = FindEntityId( fromEntnum, classnum );

	if ( !sourceId ) {
		return;
	}
	if ( !FindNextSibling( sourceId ) ) {
		return;
	}
	CopyEntity( sourceId, Scr_GetEntityId( toEntnum, classnum ) );
}

void Var_Init__Fv( void ) { Var_Init(); }
void InitVariables__Fv( void ) { InitVariables(); }

unsigned short FindVariableIndexInternal__FUsUi( unsigned short p, unsigned int n ) {
	return FindVariableIndexInternal( p, n );
}
unsigned short GetVariableIndexInternal__FUsUi( unsigned short p, unsigned int n ) {
	return GetVariableIndexInternal( p, n );
}
void MakeVariableExternal__FP8VariableP21VariableValueInternal(
		Variable *slot, VariableValueInternal *parent ) {
	MakeVariableExternal( slot, parent );
}
void ClearObjectInternal__FUs( unsigned short id ) { ClearObjectInternal( id ); }
void ClearObject__FUs( unsigned short id ) { ClearObject( id ); }
void ClearThreadNotifyName__FUs( unsigned short id ) { ClearThreadNotifyName( id ); }
void SetThreadNotifyName__FUsUs( unsigned short id, unsigned short name ) {
	SetThreadNotifyName( id, name );
}
void KillThread__FUs( unsigned short id ) { KillThread( id ); }
VariableValueInternal *AllocVariable__Fv( void ) { return AllocVariable(); }
unsigned short AllocValue__Fv( void ) { return AllocValue(); }
unsigned short AllocObject__Fv( void ) { return AllocObject(); }
unsigned short AllocThread__FUs( unsigned short self ) { return AllocThread( self ); }
unsigned short AllocEntity__FiUs( int c, unsigned short e ) { return AllocEntity( c, e ); }
void FreeVariable__FUs( unsigned short id ) { FreeVariable( id ); }
void FreeValue__FP21VariableValueInternal( VariableValueInternal *n ) { FreeValueInternal( n ); }
void FreeValue__FUs( unsigned short id ) { FreeValue( id ); }
void AddRefToObject__FUs( unsigned short id ) { AddRefToObject( id ); }
void RemoveRefToObject__FUs( unsigned short id ) { RemoveRefToObject( id ); }
void AddRefToVector__FPCf( const float *v ) { AddRefToVector( v ); }
void RemoveRefToVector__FPCf( const float *v ) { RemoveRefToVector( v ); }
void AddRefToValue__Fi13VariableUnion( int t, VariableUnion u ) { AddRefToValue( t, u ); }
void RemoveRefToValue__Fi13VariableUnion( int t, VariableUnion u ) { RemoveRefToValue( t, u ); }
unsigned short FindVariable__FUsUi( unsigned short p, unsigned int n ) {
	return FindVariable( p, n );
}
unsigned short GetVariable__FUsUi( unsigned short p, unsigned int n ) {
	return GetVariable( p, n );
}
unsigned short FindObjectVariable__FUsUs( unsigned short p, unsigned short c ) {
	return FindObjectVariable( p, c );
}
unsigned short GetObjectVariable__FUsUs( unsigned short p, unsigned short c ) {
	return GetObjectVariable( p, c );
}
unsigned short FindArrayVariable__FUsi( unsigned short p, int i ) {
	return FindArrayVariable( p, i );
}
unsigned short GetArrayVariable__FUsi( unsigned short p, int i ) {
	return GetArrayVariable( p, i );
}
unsigned short GetArrayVariable__FUsUi( unsigned short p, unsigned int i ) {
	return GetArrayVariable( p, (int)i );
}
unsigned short GetVariableField__FUsUs( unsigned short p, unsigned short n ) {
	return GetVariableField( p, n );
}
void ClearVariableField__FUsUs( unsigned short p, unsigned short n ) {
	ClearVariableField( p, n );
}
void RemoveVariable__FUsUi( unsigned short p, unsigned int n ) { RemoveVariable( p, n ); }
void SafeRemoveVariable__FUsUi( unsigned short p, unsigned int n ) { SafeRemoveVariable( p, n ); }
void RemoveArrayVariable__FUsUi( unsigned short p, unsigned int n ) {
	RemoveArrayVariable( p, n );
}
void SafeRemoveArrayVariable__FUsUi( unsigned short p, unsigned int n ) {
	SafeRemoveArrayVariable( p, n );
}
void RemoveObjectVariable__FUsUs( unsigned short p, unsigned short c ) {
	RemoveObjectVariable( p, c );
}
void SetVariableValue__FUsP13VariableValue( unsigned short id, VariableValue *v ) {
	SetVariableValue( id, v );
}
void SetNewVariableValue__FUsP13VariableValue( unsigned short id, VariableValue *v ) {
	SetNewVariableValue( id, v );
}
void SetVariableFieldValue__FUsP13VariableValue( unsigned short id, VariableValue *v ) {
	SetVariableFieldValue( id, v );
}
void GetVariableValue__FUsP13VariableValue( unsigned short id, VariableValue *v ) {
	GetVariableValue( id, v );
}
void GetVariableFieldValue__FUsP13VariableValue( unsigned short id, VariableValue *v ) {
	GetVariableFieldValue( id, v );
}
void ClearVariableValue__FUs( unsigned short id ) { ClearVariableValue( id ); }
VariableValue *GetVariableValueAddress__FUs( unsigned short id ) {
	return GetVariableValueAddress( id );
}
void GetSizeValue__FP13VariableValue( VariableValue *v ) { GetSizeValue( v ); }
unsigned short GetArraySize__FUs( unsigned short id ) { return GetArraySize( id ); }
unsigned short FindNextSibling__FUs( unsigned short id ) { return FindNextSibling( id ); }
unsigned int GetVariableName__FUs( unsigned short id ) { return GetVariableName( id ); }
unsigned short GetVariableKeyObject__FUs( unsigned short id ) {
	return GetVariableKeyObject( id );
}
unsigned int GetEntityType__FUs( unsigned short id ) { return GetEntityType( id ); }
unsigned short GetThreadNotifyName__FUs( unsigned short id ) {
	return GetThreadNotifyName( id );
}
unsigned short GetSelf__FUs( unsigned short id ) { return GetSelf( id ); }
unsigned short GetEntnum__FUs( unsigned short id ) { return GetEntnum( id ); }
unsigned short GetObject__FUs( unsigned short id ) { return GetObject( id ); }
unsigned short GetArray__FUs( unsigned short id ) { return GetArray( id ); }
unsigned short FindObject__FUs( unsigned short id ) { return FindObject( id ); }
qboolean IsFieldObject__FUs( unsigned short id ) { return IsFieldObject( id ); }
int GetVarType__FUs( unsigned short id ) { return GetVarType( id ); }
qboolean IsValidArrayIndex__FUi( unsigned int i ) { return IsValidArrayIndex( i ); }
unsigned int GetInternalVariableIndex__FUi( unsigned int i ) {
	return GetInternalVariableIndex( i );
}
unsigned short FindArrayVariableIndex__FUsUi( unsigned short p, unsigned int i ) {
	return FindArrayVariableIndex( p, i );
}
unsigned short GetArrayVariableIndex__FUsUi( unsigned short p, unsigned int i ) {
	return GetArrayVariableIndex( p, i );
}
unsigned short CastFieldObject__FP13VariableValue( VariableValue *v ) {
	return CastFieldObject( v );
}
void CopyArray__FUsUs( unsigned short s, unsigned short d ) { CopyArray( s, d ); }
void CopyEntity__FUsUs( unsigned short s, unsigned short d ) { CopyEntity( s, d ); }
void EvalArray__FP13VariableValueP13VariableValue( VariableValue *i, VariableValue *c ) {
	EvalArray( i, c );
}
unsigned short EvalArrayRef__FUsP13VariableValue( unsigned short id, VariableValue *i ) {
	return EvalArrayRef( id, i );
}
void ClearArray__FUsP13VariableValue( unsigned short id, VariableValue *i ) {
	ClearArray( id, i );
}
void GetEmptyArray__FP13VariableValue( VariableValue *v ) { GetEmptyArray( v ); }
void SetEmptyArray__FUs( unsigned short id ) { SetEmptyArray( id ); }
float *AllocVector__Fv( void ) { return AllocVector(); }
VariableUnion AllocVector__FPCf( const float *v ) { return AllocVectorValue( v ); }
unsigned short FindEntityId__Fii( int e, int c ) { return FindEntityId( e, c ); }

#define SCR_SAVE_IDMAP_SIZE     ( VARIABLELIST_SIZE * (int)sizeof( unsigned short ) )

#define scrSave_idToSaveId      ( (unsigned short *)scrSave_idToSaveId )
#define scrSave_saveIdToId      ( (unsigned short *)scrSave_saveIdToId )
#define scrSave_numSaveObjects  ( *(unsigned short *)&scrSave_numSaveObjects )
#define scrSave_readPos         ( (char *)scrSave_readPos )

/* ---- WriteByte__FUc  0x004736D0 ---- VERIFIED */
_BYTE *__cdecl WriteByte(char a1)
{
  int v1;
  _BYTE *result;

  v1 = currentPos + 1;
  result = (_BYTE *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 1));
  currentPos = v1;
  *result = a1;
  return result;
}

/* ---- ReadByte__Fv  0x00473700 ---- VERIFIED */
char ReadByte()
{
  return *(_BYTE *)scrSave_readPos++;
}

/* ---- WriteShort__FUs  0x00473710 ---- VERIFIED */
_WORD *__cdecl WriteShort(__int16 a1)
{
  int v1;
  _WORD *result;

  v1 = currentPos + 2;
  result = (_WORD *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 2));
  currentPos = v1;
  *result = a1;
  return result;
}

/* ---- ReadShort__Fv  0x00473740 ---- VERIFIED */
__int16 ReadShort()
{
  __int16 result;

  result = *(_WORD *)scrSave_readPos;
  scrSave_readPos += 2;
  return result;
}

/* ---- WriteString  0x00473760 ---- VERIFIED */
void WriteString( unsigned short id ) {
	const char *text = SL_ConvertToString( id );
	int         len  = (int)strlen( text );
	char       *dest;

	dest = (char *)( currentPos + Hunk_ReallocateTempMemory( currentPos + len + 1 ) );
	currentPos += len + 1;

	do {
		*dest++ = *text;
	} while ( *text++ );
}

void WriteString__FUs( unsigned short id ) { WriteString( id ); }

/* ---- SafeWriteString__FUs  0x004737C0 ---- VERIFIED */
void SafeWriteString( unsigned short id ) {
	char *present;

	present = (char *)( currentPos + Hunk_ReallocateTempMemory( currentPos + 1 ) );
	currentPos += 1;

	if ( id ) {
		*present = 1;
		WriteString( id );
		return;
	}
	*present = 0;
}

/* ---- ReadString__Fv  0x004737F0 ---- VERIFIED */
__int16 ReadString()
{
  unsigned int v0;
  __int16 result;

  v0 = strlen((const char *)scrSave_readPos);
  result = SL_GetStringOfLen((char *)scrSave_readPos, 0, v0 + 1, SCR_STRING_MT_TYPE_TEMP);
  scrSave_readPos += v0 + 1;
  return result;
}

/* ---- SafeReadString__Fv  0x00473830 ---- VERIFIED */
__int16 SafeReadString()
{
  char *v0;
  unsigned int v2;
  __int16 result;

  v0 = (char *)(scrSave_readPos + 1);
  if ( *(_BYTE *)scrSave_readPos++ == 0 )
    return 0;
  v2 = strlen(v0);
  result = SL_GetStringOfLen(v0, 0, v2 + 1, SCR_STRING_MT_TYPE_TEMP);
  scrSave_readPos += v2 + 1;
  return result;
}

/* ---- WriteFloat__Ff  0x00473880 ---- VERIFIED */
_DWORD *__cdecl WriteFloat(int a1)
{
  int v1;
  _DWORD *result;

  v1 = currentPos + 4;
  result = (_DWORD *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 4));
  currentPos = v1;
  *result = a1;
  return result;
}

/* ---- ReadFloat__Fv  0x004738B0 ---- VERIFIED */
double ReadFloat()
{
  double result;

  result = *(float *)scrSave_readPos;
  scrSave_readPos += 4;
  return result;
}

/* ---- WriteVector__FPCf  0x004738C0 ---- VERIFIED */
int *__cdecl WriteVector(int *a1)
{
  int v1;
  int v2;
  int v3;
  int *result;
  int v5;
  int v6;

  v1 = currentPos + 4;
  v5 = *a1;
  *(_DWORD *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 4)) = v5;
  v2 = a1[1];
  currentPos = v1;
  v1 += 4;
  *(_DWORD *)(currentPos + Hunk_ReallocateTempMemory(v1)) = v2;
  v3 = a1[2];
  currentPos = v1;
  v1 += 4;
  v6 = v3;
  result = (int *)(currentPos + Hunk_ReallocateTempMemory(v1));
  currentPos = v1;
  *result = v6;
  return result;
}

/* ---- ReadVector__Fv  0x00473930 ---- VERIFIED */
_DWORD *ReadVector()
{
  _DWORD *result;
  int v1;
  int v2;
  int v3;

  v1 = *(_DWORD *)scrSave_readPos;
  v2 = *(_DWORD *)(scrSave_readPos + 4);
  v3 = *(_DWORD *)(scrSave_readPos + 8);
  scrSave_readPos += 12;
  result = (_DWORD *)((char *)MT_Alloc(SCR_VECTOR_ALLOC_SIZE, 2) + 2);
  *result = v1;
  *((_WORD *)result - 1) = 0;
  result[1] = v2;
  result[2] = v3;
  return result;
}

/* ---- WriteInt__Fi  0x00473990 ---- VERIFIED */
_DWORD *__cdecl WriteInt(int a1)
{
  int v1;
  _DWORD *result;

  v1 = currentPos + 4;
  result = (_DWORD *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 4));
  currentPos = v1;
  *result = a1;
  return result;
}

/* ---- ReadInt__Fv  0x004739C0 ---- VERIFIED */
int ReadInt()
{
  int result;

  result = *(_DWORD *)scrSave_readPos;
  scrSave_readPos += 4;
  return result;
}

/* ---- WriteCodepos__FPCc  0x004739E0 ---- VERIFIED */
int *__cdecl WriteCodepos(int a1)
{
  int v1;
  int v2;
  int *result;

  if ( a1 )
    v1 = a1 - scrVarPub_programBuffer;
  else
    v1 = -1;
  v2 = currentPos + 4;
  result = (int *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 4));
  *result = v1;
  currentPos = v2;
  return result;
}

/* ---- ReadCodepos__Fv  0x00473A20 ---- VERIFIED */
int ReadCodepos()
{
  int v0;
  bool v1;

  v0 = *(_DWORD *)scrSave_readPos;
  v1 = *(int *)scrSave_readPos < 0;
  scrSave_readPos += 4;
  if ( v1 )
    return 0;
  else
    return scrVarPub_programBuffer + v0;
}

/* ---- WriteId__FUs  0x00473A50 ---- VERIFIED */
_WORD *__cdecl WriteId(unsigned __int16 a1)
{
  __int16 v1;
  int v2;
  _WORD *result;

  v1 = *((_WORD *)scrSave_idToSaveId + a1);
  v2 = currentPos + 2;
  result = (_WORD *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 2));
  *result = v1;
  currentPos = v2;
  return result;
}

/* ---- ReadId  0x00473A80 ---- VERIFIED */
unsigned short ReadId( void ) {
	unsigned short saveId = *(unsigned short *)scrSave_readPos;
	unsigned short id;

	scrSave_readPos += 2;
	id = scrSave_saveIdToId[saveId];
	AddRefToObject( id );
	return id;
}

unsigned short ReadId__Fv( void ) { return ReadId(); }

/* ---- WriteBuffer__FPci  0x00473AB0 ---- VERIFIED */
void WriteBuffer( int len, const void *buf ) {
	char *dest;

	dest = (char *)( currentPos + Hunk_ReallocateTempMemory( currentPos + len ) );
	currentPos += len;
	Com_Memcpy( dest, buf, len );
}

/* ---- ReadBuffer  0x00473AE0 ---- VERIFIED */
void ReadBuffer( char *buf, int len ) {
	Com_Memcpy( buf, scrSave_readPos, len );
	scrSave_readPos += len;
}

void ReadBuffer__FPci( char *buf, int len ) { ReadBuffer( buf, len ); }

/* ---- Scr_ConvertThreadToSave  0x00473B00 ---- VERIFIED */
__int16 __cdecl Scr_ConvertThreadToSave(unsigned __int16 a1)
{
  if ( !a1 )
    return 0;
  AddSaveObject(a1);
  return *((_WORD *)scrSave_idToSaveId + a1);
}

/* ---- Scr_ConvertThreadFromLoad  0x00473B30 ---- VERIFIED */
unsigned short Scr_ConvertThreadFromLoad( unsigned short saveId ) {
	unsigned short id;

	if ( !saveId ) {
		return 0;
	}
	id = scrSave_saveIdToId[saveId];
	AddRefToObject( id );
	return id;
}

/* ---- WriteStack__FPC19VariableStackBuffer  0x00473B60 ---- VERIFIED */
void WriteStack( const VariableStackBuffer *buf ) {
	unsigned short  count;
	unsigned short  saveId;
	int             codePos;
	const char     *entry;
	char           *p;
	int             i;

	count = buf->size;
	p = (char *)( currentPos + Hunk_ReallocateTempMemory( currentPos + 2 ) );
	currentPos += 2;
	*(unsigned short *)p = count;

	p = (char *)( currentPos + Hunk_ReallocateTempMemory( currentPos + 4 ) );
	currentPos += 4;
	*(const char **)p = buf->pos;

	codePos = buf->codePos ? (int)buf->codePos - scrVarPub_programBuffer : -1;
	p = (char *)( currentPos + Hunk_ReallocateTempMemory( currentPos + 4 ) );
	saveId = scrSave_idToSaveId[ buf->objectId ];
	*(int *)p = codePos;
	currentPos += 4;

	p = (char *)( currentPos + Hunk_ReallocateTempMemory( currentPos + 2 ) );
	currentPos += 2;
	*(unsigned short *)p = saveId;

	entry = (const char *)buf + SCR_STACKBUFFER_HEADER;
	for ( i = count; i != 0; i-- ) {
		unsigned char   type = (unsigned char)entry[0];
		VariableUnion   u    = *(const VariableUnion *)( entry + 1 );

		entry += SCR_STACKENTRY_SIZE;
		DoSaveEntryInternal( type, u );
	}
}

/* ---- ReadStack  0x00473C30 ---- VERIFIED */
VariableStackBuffer *ReadStack( void ) {
	VariableStackBuffer *buf;
	unsigned short       count;
	unsigned short       saveId;
	int                  codePos;
	char                *entry;
	int                  i;

	count = *(unsigned short *)scrSave_readPos;
	scrSave_readPos += 2;

	buf = (VariableStackBuffer *)MT_Alloc(
			SCR_STACKENTRY_SIZE * count + SCR_STACKBUFFER_HEADER, 1 );
	buf->size = count;

	buf->pos = *(const char **)scrSave_readPos;
	scrSave_readPos += 4;

	codePos = *(const int *)scrSave_readPos;
	scrSave_readPos += 4;
	buf->codePos = codePos < 0 ? NULL : (const char *)( scrVarPub_programBuffer + codePos );

	saveId = *(unsigned short *)scrSave_readPos;
	scrSave_readPos += 2;
	buf->objectId = scrSave_saveIdToId[saveId];
	AddRefToObject( buf->objectId );

	entry = (char *)buf + SCR_STACKBUFFER_HEADER;
	for ( i = count; i != 0; i-- ) {
		VariableValue value;

		DoLoadEntryInternal( &value );
		entry[0] = (char)value.type;
		*(VariableUnion *)( entry + 1 ) = value.u;
		entry += SCR_STACKENTRY_SIZE;
	}
	return buf;
}

/* ---- AddSaveStack__FPC19VariableStackBuffer  0x00473CF0 ---- VERIFIED */
void AddSaveStack( const VariableStackBuffer *buf ) {
	const char *entry;
	int         i;

	AddSaveObject( buf->objectId );

	entry = (const char *)buf + SCR_STACKBUFFER_HEADER;
	for ( i = buf->size; i != 0; i-- ) {
		unsigned char   type = (unsigned char)entry[0];
		VariableUnion   u    = *(const VariableUnion *)( entry + 1 );

		entry += SCR_STACKENTRY_SIZE;
		if ( type == VAR_OBJECT ) {
			AddSaveObject( u.halfword[0] );
		} else if ( type == VAR_STACK ) {
			AddSaveStack( (const VariableStackBuffer *)u.pointerValue );
		}
	}
}

/* ---- AddSaveEntry__FUc13VariableUnion  0x00473D40 ---- VERIFIED */
void AddSaveEntry( unsigned char type, VariableUnion u ) {
	if ( type == VAR_OBJECT ) {
		AddSaveObject( u.halfword[0] );
		return;
	}
	if ( type == VAR_STACK ) {
		AddSaveStack( (const VariableStackBuffer *)u.pointerValue );
	}
}

/* ---- AddSaveEntry  0x00473D60 ---- VERIFIED */
void AddSaveEntry__FP21VariableValueInternal( VariableValueInternal *node ) {
	AddSaveEntry( (unsigned char)VAR_TYPE( node ), node->u );
}

/* ---- Scr_GetEntryUsage  0x00473D90 ---- VERIFIED */
float Scr_GetEntryUsage( unsigned char type, VariableUnion u ) {
	VariableValueInternal *node;

	if ( type != VAR_OBJECT ) {
		return 0.0f;
	}
	node = &scrVarNodes[ u.halfword[0] ];
	if ( VAR_TYPE( node ) != VAR_ARRAY ) {
		return 0.0f;
	}
	return Scr_GetObjectUsage( u.halfword[0] ) / ( (float)VAR_REFCOUNT( node ) + 1.0f );
}

float Scr_GetEntryUsage__FUc13VariableUnion( unsigned char type, VariableUnion u ) {
	return Scr_GetEntryUsage( type, u );
}

/* ---- Scr_GetEntryUsage  0x00473DE0 ---- VERIFIED */
float Scr_GetEntryUsage__FP21VariableValueInternal( VariableValueInternal *node ) {
	return Scr_GetEntryUsage( (unsigned char)VAR_TYPE( node ), node->u ) + 1.0f;
}

/* ---- DoSaveEntryInternal  0x00473E00 ---- VERIFIED */
void DoSaveEntryInternal( unsigned char type, VariableUnion u ) {
	WriteByte( (char)type );

	switch ( type & VAR_STATUS_TYPE_MASK ) {
	case VAR_STRING:
	case VAR_ISTRING:
		WriteString( u.stringValue );
		break;
	case VAR_VECTOR:
		WriteVector( (int *)u.vectorValue );
		break;
	case VAR_FLOAT:
		WriteFloat( u.intValue );
		break;
	case VAR_INTEGER:
	case VAR_ANIMATION:
		WriteInt( u.intValue );
		break;
	case VAR_CODEPOS:
	case VAR_FUNCTION:
		WriteCodepos( (int)u.codePosValue );
		break;
	case VAR_OBJECT:
		WriteId( u.halfword[0] );
		break;
	case VAR_STACK:
		WriteStack( (const VariableStackBuffer *)u.pointerValue );
		break;
	default:
		break;
	}
}

/* ---- DoSaveEntry__FP13VariableValueUib  0x00473ED0 ---- VERIFIED */
void DoSaveEntry( VariableValue *value, unsigned int name, char bIntern ) {
	DoSaveEntryInternal( (unsigned char)value->type, value->u );

	if ( bIntern && name < VAR_NAME_OBJECT_LIMIT ) {
		if ( name >= VAR_NAME_OBJECT_BASE ) {
			WriteByte( 3 );
			WriteId( (unsigned short)name );
		} else {
			WriteByte( 2 );
			WriteString( (unsigned short)name );
		}
		return;
	}

	if ( name >= VAR_NAME_OBJECT_BASE ) {
		WriteByte( 1 );
		WriteInt( (int)name );
	} else {
		WriteByte( 0 );
		WriteShort( (short)name );
	}
}

/* ---- DoLoadEntryInternal  0x00473F80 ---- VERIFIED */
void DoLoadEntryInternal( VariableValue *value ) {
	unsigned char type = (unsigned char)*scrSave_readPos;

	value->type = type;
	scrSave_readPos += 1;

	switch ( type & VAR_STATUS_TYPE_MASK ) {
	case VAR_STRING:
	case VAR_ISTRING:
		value->u.stringValue = (unsigned short)ReadString();
		break;
	case VAR_VECTOR:
		value->u.vectorValue = (const float *)ReadVector();
		break;
	case VAR_FLOAT:
		value->u.floatValue = *(const float *)scrSave_readPos;
		scrSave_readPos += 4;
		break;
	case VAR_INTEGER:
	case VAR_ANIMATION:
		value->u.intValue = *(const int *)scrSave_readPos;
		scrSave_readPos += 4;
		break;
	case VAR_CODEPOS:
	case VAR_FUNCTION:
		value->u.codePosValue = (const char *)ReadCodepos();
		break;
	case VAR_OBJECT:
		value->u.halfword[0] = ReadId();
		break;
	case VAR_STACK:
		value->u.pointerValue = ReadStack();
		break;
	default:
		break;
	}
}

void DoLoadEntryInternal__FP13VariableValue( VariableValue *value ) {
	DoLoadEntryInternal( value );
}

/* ---- DoLoadEntry__FP13VariableValue  0x00474020 ---- VERIFIED */
unsigned int DoLoadEntry( VariableValue *value ) {
	unsigned char   tag;
	unsigned int    name;

	DoLoadEntryInternal( value );

	tag = (unsigned char)*scrSave_readPos;
	scrSave_readPos += 1;

	switch ( tag ) {
	case 0:
		name = *(unsigned short *)scrSave_readPos;
		scrSave_readPos += 2;
		break;
	case 1:
		name = *(unsigned int *)scrSave_readPos;
		scrSave_readPos += 4;
		break;
	case 2:
		name = (unsigned short)ReadString();
		break;
	case 3:
		name = (unsigned short)ReadId() + VAR_NAME_OBJECT_BASE;   /* movzx, 0x00474071 */
		break;
	default:
		name = 0;
		break;
	}
	return name;
}

unsigned int DoLoadEntry__FP13VariableValue( VariableValue *value ) {
	return DoLoadEntry( value );
}

/* ---- AddSaveObject  0x00474090 ---- VERIFIED */
void AddSaveObject( unsigned short id ) {
	while ( scrSave_idToSaveId[id] == 0 ) {
		VariableValueInternal  *node;
		int                     parentType;
		qboolean                bArray;
		unsigned short          child;

		scrSave_numSaveObjects++;
		scrSave_idToSaveId[id] = scrSave_numSaveObjects;
		scrSave_saveIdToId[ scrSave_numSaveObjects ] = id;

		node       = &scrVarNodes[id];
		parentType = VAR_TYPE( node );
		bArray     = parentType == VAR_ARRAY ? qtrue : qfalse;

		for ( child = scrVarIndirections[ node->nextSibling ].id;
		      VAR_TYPE( &scrVarNodes[child] ) < VAR_THREAD && child != 0;
		      child = scrVarIndirections[ scrVarNodes[child].nextSibling ].id ) {
			VariableValueInternal *entry = &scrVarNodes[child];

			if ( bArray ) {
				unsigned int name = VAR_NAME( entry );

				if ( name >= VAR_NAME_OBJECT_BASE && name < VAR_NAME_OBJECT_LIMIT ) {
					AddSaveObject( (unsigned short)name );
				}
			}

			if ( VAR_TYPE( entry ) == VAR_OBJECT ) {
				AddSaveObject( VAR_OBJECTID( entry ) );
			} else if ( VAR_TYPE( entry ) == VAR_STACK ) {
				AddSaveStack( (const VariableStackBuffer *)entry->u.pointerValue );
			}
		}

		if ( parentType != VAR_THREAD ) {
			return;
		}
		id = VAR_SELF( node );
	}
}

void AddSaveObject__FUs( unsigned short id ) { AddSaveObject( id ); }

/* ---- Scr_GetEndonUsage  0x004741D0 ---- VERIFIED */
float Scr_GetEndonUsage( unsigned short id ) {
	unsigned short handle;
	unsigned short node;

	handle = FindVariableIndexInternal( scrVarPub_pauseArrayId,
										id + VAR_NAME_OBJECT_BASE );
	node = scrVarIndirections[handle].id;
	if ( !node ) {
		return 0.0f;
	}
	return Scr_GetObjectUsage( VAR_OBJECTID( &scrVarNodes[node] ) );
}

float Scr_GetEndonUsage__FUs( unsigned short id ) { return Scr_GetEndonUsage( id ); }

/* ---- Scr_GetObjectUsage  0x00474220 ---- VERIFIED */
float Scr_GetObjectUsage( unsigned short id ) {
	float           usage = 1.0f;
	unsigned short  child;

	for ( child = scrVarIndirections[ scrVarNodes[id].nextSibling ].id;
	      VAR_TYPE( &scrVarNodes[child] ) < VAR_THREAD && child != 0;
	      child = scrVarIndirections[ scrVarNodes[child].nextSibling ].id ) {
		usage += Scr_GetEntryUsage__FP21VariableValueInternal( &scrVarNodes[child] );
	}
	return usage;
}

float Scr_GetObjectUsage__FUs( unsigned short id ) { return Scr_GetObjectUsage( id ); }

/* ---- Scr_GetThreadUsage  0x00474310 ---- VERIFIED */
float Scr_GetThreadUsage( const VariableStackBuffer *buf, float *endonUsage ) {
	float           usage;
	const char     *entry;
	int             i;

	usage       = Scr_GetObjectUsage( buf->objectId );
	*endonUsage = Scr_GetEndonUsage( buf->objectId );

	entry = (const char *)buf + SCR_STACKBUFFER_HEADER
			+ SCR_STACKENTRY_SIZE * buf->size;

	for ( i = buf->size; i != 0; ) {
		unsigned char   type;
		VariableUnion   u;

		u = *(const VariableUnion *)( entry - 4 );
		entry -= SCR_STACKENTRY_SIZE;
		type  = (unsigned char)entry[0];
		i--;

		if ( type == VAR_CODEPOS ) {
			VariableUnion frame;

			frame  = *(const VariableUnion *)( entry - 4 );
			entry -= SCR_STACKENTRY_SIZE;
			i--;

			usage       += Scr_GetObjectUsage( frame.halfword[0] );
			*endonUsage += Scr_GetEndonUsage( frame.halfword[0] );
			continue;
		}

		usage += Scr_GetEntryUsage( type, u );
	}
	return usage;
}

float Scr_GetThreadUsage__FPC19VariableStackBufferPf( const VariableStackBuffer *buf,
													  float *endonUsage ) {
	return Scr_GetThreadUsage( buf, endonUsage );
}

/* ---- DoSaveObjectInfo  0x00474470 ---- VERIFIED */
void DoSaveObjectInfo( unsigned short id ) {
	VariableValueInternal  *node = &scrVarNodes[id];
	unsigned char           status;
	int                     type;
	char                    bArray;
	unsigned short          count;
	unsigned short          child;

	status = (unsigned char)( node->status & 0x9F );
	WriteByte( (char)status );

	type = status & VAR_STATUS_TYPE_MASK;
	if ( type == VAR_THREAD ) {
		WriteId( VAR_SELF( node ) );
		SafeWriteString( (unsigned short)VAR_NAME( node ) );
	} else if ( type == VAR_ENTITY || type == VAR_DEADENTITY ) {
		WriteShort( (short)VAR_ENTNUM( node ) );
		WriteShort( (short)VAR_NAME( node ) );
	}

	bArray = (char)( VAR_TYPE( node ) == VAR_ARRAY );

	count = 0;
	for ( child = scrVarIndirections[ node->nextSibling ].id;
	      VAR_TYPE( &scrVarNodes[child] ) < VAR_THREAD && child != 0;
	      child = scrVarIndirections[ scrVarNodes[child].nextSibling ].id ) {
		count++;
	}
	WriteShort( (short)count );

	for ( child = scrVarIndirections[ node->nextSibling ].id;
	      VAR_TYPE( &scrVarNodes[child] ) < VAR_THREAD && child != 0;
	      child = scrVarIndirections[ scrVarNodes[child].nextSibling ].id ) {
		VariableValueInternal  *entry = &scrVarNodes[child];
		VariableValue           value;

		value.u    = entry->u;
		value.type = (int)( entry->status & 0x9F );
		DoSaveEntry( &value, VAR_NAME( entry ), bArray );
	}
}

void DoSaveObjectInfo__FUs( unsigned short id ) { DoSaveObjectInfo( id ); }

/* ---- DoLoadObjectInfo  0x004746C0 ---- VERIFIED */
void DoLoadObjectInfo( unsigned short id ) {
	VariableValueInternal  *node = &scrVarNodes[id];
	unsigned char           status;
	int                     type;
	qboolean                bArray;
	unsigned short          count;
	int                     i;

	status = (unsigned char)*scrSave_readPos;
	scrSave_readPos += 1;

	node->status = ( node->status & ~VAR_STATUS_TYPE_MASK ) | status;

	type = status & VAR_STATUS_TYPE_MASK;
	if ( type == VAR_THREAD ) {
		unsigned short saveId;
		unsigned short self;

		saveId = *(unsigned short *)scrSave_readPos;
		scrSave_readPos += 2;
		self = scrSave_saveIdToId[saveId];
		AddRefToObject( self );
		VAR_SELF( node ) = self;

		node->status |= (unsigned int)(unsigned short)SafeReadString()
						<< VAR_STATUS_NAME_SHIFT;
	} else if ( type == VAR_ENTITY || type == VAR_DEADENTITY ) {
		VAR_ENTNUM( node ) = *(unsigned short *)scrSave_readPos;
		scrSave_readPos += 2;
		node->status |= (unsigned int)*(unsigned short *)scrSave_readPos
						<< VAR_STATUS_NAME_SHIFT;
		scrSave_readPos += 2;
	} else if ( type == VAR_ARRAY ) {
		VAR_ARRAYSIZE( node ) = 0;
	}

	bArray = type == VAR_ARRAY ? qtrue : qfalse;

	count = *(unsigned short *)scrSave_readPos;
	scrSave_readPos += 2;

	for ( i = count; i != 0; i-- ) {
		VariableValue   value;
		unsigned int    name;
		unsigned short  child;

		name  = DoLoadEntry( &value );
		child = scrVarIndirections[ GetVariableIndexInternal( id, name ) ].id;

		if ( bArray ) {
			if ( name < VAR_NAME_OBJECT_BASE ) {
				SL_RemoveRefToString( (unsigned short)name );
			} else if ( name < VAR_NAME_OBJECT_LIMIT ) {
				RemoveRefToObject( (unsigned short)name );
			}
		}

		SetNewVariableValue( child, &value );
	}
}

void DoLoadObjectInfo__FUs( unsigned short id ) { DoLoadObjectInfo( id ); }

/* ---- Scr_SavePre  0x00474880 ---- VERIFIED */
void Scr_SavePre( void ) {
	unsigned int i;

	scrSave_idToSaveId = Hunk_AllocateTempMemoryHighInternal( SCR_SAVE_IDMAP_SIZE );
	scrSave_saveIdToId = Hunk_AllocateTempMemoryHighInternal( SCR_SAVE_IDMAP_SIZE );
	Com_Memset( scrSave_idToSaveId, 0, SCR_SAVE_IDMAP_SIZE );

	scrSave_numSaveObjects = 0;

	AddSaveObject( scrVarPub_levelId );
	AddSaveObject( scrVarPub_animId );
	AddSaveObject( scr_timeArrayId );
	AddSaveObject( scrVarPub_pauseArrayId );

	for ( i = 0; i < scrVarPub_classNum; i++ ) {
		AddSaveObject( VAR_OBJECTID( &scrVarNodes[
			scrVarIndirections[ FindVariableIndexInternal( scrVarPub_entId, i ) ].id ] ) );
		AddSaveObject( VAR_OBJECTID( &scrVarNodes[
			scrVarIndirections[ FindVariableIndexInternal( scrVarPub_classId, i ) ].id ] ) );
	}

	AddSaveEntry__FP21VariableValueInternal( &scrVarNodes[scrVarPub_gameId] );
}

/* ---- WriteGameEntry  0x00474A30 ---- VERIFIED */
void WriteGameEntry__Fv( void ) {
	VariableValueInternal *node = &scrVarNodes[scrVarPub_gameId];

	DoSaveEntryInternal( (unsigned char)( node->status & 0x9F ), node->u );
}

/* ---- ReadGameEntry  0x00474A60 ---- VERIFIED */
void ReadGameEntry__Fv( void ) {
	VariableValue value;

	scrVarPub_gameId = AllocValue();
	DoLoadEntryInternal( &value );
	SetNewVariableValue( scrVarPub_gameId, &value );
}

/* ---- Scr_SavePost  0x00474AC0 ---- VERIFIED */
void Scr_SavePost( void ) {
	unsigned int i;

	WriteInt( (int)scrVarPub_time );
	WriteShort( (short)scrSave_numSaveObjects );

	for ( i = 1; i <= scrSave_numSaveObjects; i++ ) {
		DoSaveObjectInfo( scrSave_saveIdToId[i] );
	}

	WriteGameEntry__Fv();

	WriteId( scrVarPub_levelId );
	WriteId( scrVarPub_animId );
	WriteId( scr_timeArrayId );
	WriteId( scrVarPub_pauseArrayId );

	for ( i = 0; i < scrVarPub_classNum; i++ ) {
		WriteId( VAR_OBJECTID( &scrVarNodes[
			scrVarIndirections[ FindVariableIndexInternal( scrVarPub_entId, i ) ].id ] ) );
	}
}

/* ---- Scr_SaveShutdown  0x00474DA0 ---- VERIFIED */
int Scr_SaveShutdown()
{
  int result;

  result = *(_DWORD *)hunk_highUsed;
  *(_DWORD *)hunk_highTemp = *(_DWORD *)hunk_highUsed;
  return result;
}

/* ---- Scr_LoadPre  0x00474DB0 ---- VERIFIED */
void Scr_LoadPre( void ) {
	unsigned int    i;
	VariableValue   value;

	scrSave_readPos = scrImport_LoadRead( 4 );
	scrSave_readPos = scrImport_LoadRead( *(int *)scrSave_readPos );

	scrVarPub_time = (unsigned int)*(int *)scrSave_readPos;
	scrSave_readPos += 4;
	scrSave_numSaveObjects = *(unsigned short *)scrSave_readPos;
	scrSave_readPos += 2;

	scrSave_idToSaveId = Hunk_AllocateTempMemoryInternal( SCR_SAVE_IDMAP_SIZE );
	scrSave_saveIdToId = Hunk_AllocateTempMemoryInternal(
						2 * (int)scrSave_numSaveObjects + 2 );

	for ( i = 1; i <= scrSave_numSaveObjects; i++ ) {
		unsigned short id = AllocObject();

		scrSave_saveIdToId[i]  = id;
		scrSave_idToSaveId[id] = (unsigned short)i;
	}

	for ( i = 1; i <= scrSave_numSaveObjects; i++ ) {
		DoLoadObjectInfo( scrSave_saveIdToId[i] );
	}

	ReadGameEntry__Fv();

	scrVarPub_levelId      = ReadId();
	scrVarPub_animId       = ReadId();
	scr_timeArrayId        = ReadId();
	scrVarPub_pauseArrayId = ReadId();

	value.type = VAR_OBJECT;
	for ( i = 0; i < scrVarPub_classNum; i++ ) {
		value.u.halfword[0] = ReadId();
		SetVariableValue(
			scrVarIndirections[ FindVariableIndexInternal( scrVarPub_entId, i ) ].id,
			&value );
	}
}

/* ---- Scr_LoadShutdown  0x004750B0 ---- VERIFIED */
void Scr_LoadShutdown( void ) {
	unsigned int i;

	for ( i = 1; i <= scrSave_numSaveObjects; i++ ) {
		RemoveRefToObject( scrSave_saveIdToId[i] );
	}

	Hunk_FreeTempMemoryInternal( scrSave_saveIdToId );
	Hunk_FreeTempMemoryInternal( scrSave_idToSaveId );
}

/* ---- Scr_FindField  0x004751B0 ---- VERIFIED */
short __cdecl Scr_FindField( const char *name, int *typeOut ) {
	const char *cursor = scrVarPub_fieldBuffer;

	while ( *cursor ) {
		unsigned int nameSize = strlen( cursor ) + 1;

		if ( !Q_stricmp( name, cursor ) ) {
			unsigned short offset;

			memcpy( &offset, cursor + nameSize, sizeof( offset ) );
			*typeOut = (int)(signed char)cursor[ nameSize + 2 ];
			return (short)offset;
		}
		cursor += nameSize + 3;
	}
	return 0;
}

/* ---- Scr_AddFieldsForFile  0x00475210 ---- VERIFIED */
static void Scr_AddFieldsForFile( const char *filename ) {
	fileHandle_t    f;
	int             length;
	char           *fileText;
	char           *parse;

	length = FS_FOpenFileByMode( filename, &f, FS_READ );
	if ( length < 0 ) {
		Com_Error( ERR_DROP, va( "\x15" "cannot find '%s'", filename ) );
	}

	fileText = (char *)Hunk_AllocateTempMemoryHighInternal( length + 1 );
	FS_Read( fileText, length, f );
	fileText[length] = '\0';
	FS_FCloseFile( f );

	parse = fileText;
	Com_BeginParseSession( "Scr_AddFields" );

	for ( ;; ) {
		char           *token;
		int             fieldType;
		unsigned int    nameSize;
		unsigned short  canonical;
		int             existingType;
		char           *record;

		token = Com_ParseExt( &parse, qtrue );
		if ( parse == NULL ) {
			break;
		}

		if ( !strcmp( token, "float" ) ) {
			fieldType = VAR_FLOAT;
		} else if ( !strcmp( token, "int" ) ) {
			fieldType = VAR_INTEGER;
		} else if ( !strcmp( token, "string" ) ) {
			fieldType = VAR_STRING;
		} else {
			Com_Error( ERR_DROP, va( "\x15" "unknown type '%s' in '%s'", token, filename ) );
			return;
		}

		token = Com_ParseExt( &parse, qtrue );
		if ( parse == NULL ) {
			Com_Error( ERR_DROP, va( "\x15" "missing field name in '%s'", filename ) );
			return;
		}

		nameSize = strlen( token ) + 1;
		{
			unsigned int i;

			for ( i = 0; i < nameSize; i++ ) {
				token[i] = (char)tolower( (unsigned char)token[i] );
			}
		}

		canonical = scrString_canonicalMap[ SL_FindStringOfLen( token, nameSize ) ];
		if ( !canonical ) {
			continue;
		}

		if ( Scr_FindField( token, &existingType ) ) {
			Com_Error( ERR_DROP, "\x15" "duplicate key '%s' in '%s'", token, filename );
		}

		record = (char *)TempMalloc( (int)nameSize + 3 );
		strcpy( record, token );
		memcpy( record + nameSize, &canonical, sizeof( canonical ) );
		record[ nameSize + 2 ] = (char)fieldType;
		record[ nameSize + 3 ] = '\0';
	}

	Com_EndParseSession();
	Hunk_ClearTempMemoryHigh();
}

int Scr_AddFieldsForFile__FPCc( const char *filename ) {
	Scr_AddFieldsForFile( filename );
	return 0;
}

/* ---- Scr_AddFields  0x004754F0 ---- VERIFIED */
void __cdecl Scr_AddFields( const char *path, const char *extension ) {
	char  **files;
	int     numfiles;
	int     i;
	char   *terminator;

	files = FS_ListFilteredFiles( path, extension, 0, &numfiles );

	TempMemoryReset();
	scrVarPub_fieldBuffer = (char *)Hunk_AllocLowAlignInternal( 0, 32 );
	scrVarPub_fieldBuffer[0] = '\0';

	for ( i = 0; i < numfiles; i++ ) {
		char qpath[64];

		Com_sprintf( qpath, sizeof( qpath ), "%s/%s", path, files[i] );
		Scr_AddFieldsForFile( qpath );
	}

	if ( files ) {
		FS_FreeFileList( files );
	}

	terminator  = (char *)TempMalloc( 1 );
	*terminator = '\0';
	Hunk_CommitTempMemory();
}

/* ---- Scr_FreeValue  0x00475610 ---- VERIFIED */
void __cdecl Scr_FreeValue( unsigned short id ) {
	RemoveRefToObject( id );
}

/* ---- Scr_MakeValuePrimitive  0x00475660 ---- VERIFIED */
int __cdecl Scr_MakeValuePrimitive( unsigned short arrayId ) {
	if ( VAR_TYPE( &scrVarNodes[arrayId] ) != VAR_ARRAY ) {
		return 0;
	}

	for ( ;; ) {
		unsigned short child = FindNextSibling( arrayId );

		while ( child ) {
			unsigned int    name = VAR_NAME( &scrVarNodes[child] );
			int             strip;

			switch ( VAR_TYPE( &scrVarNodes[child] ) ) {
			case VAR_CODEPOS:
			case VAR_FUNCTION:
			case VAR_STACK:
			case VAR_ANIMATION:
				strip = 1;
				break;
			case VAR_OBJECT:
				strip = !Scr_MakeValuePrimitive( VAR_OBJECTID( &scrVarNodes[child] ) );
				break;
			default:
				strip = 0;
				break;
			}

			if ( strip ) {
				unsigned short          handle = FindVariableIndexInternal( arrayId, name );
				VariableValueInternal  *doomed = &scrVarNodes[ scrVarIndirections[handle].id ];

				MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[arrayId] );
				FreeValueInternal( doomed );
				break;
			}
			child = FindNextSibling( child );
		}

		if ( !child ) {
			return 1;
		}
	}
}

int Scr_MakeValuePrimitive__FUs( unsigned short arrayId ) {
	return Scr_MakeValuePrimitive( arrayId );
}

/* ---- Scr_AllocGameVariable  0x00475790 ---- VERIFIED */
void __cdecl Scr_AllocGameVariable( void ) {
	if ( !scrVarPub_gameId ) {
		scrVarPub_gameId = AllocValue();
		SetEmptyArray( scrVarPub_gameId );
	}
}

/* ---- Scr_FreeGameVariable  0x004757D0 ---- VERIFIED */
void __cdecl Scr_FreeGameVariable( int bComplete ) {
	if ( !bComplete ) {
		Scr_MakeValuePrimitive( VAR_OBJECTID( &scrVarNodes[ scrVarPub_gameId ] ) );
		return;
	}
	FreeValueInternal( &scrVarNodes[ scrVarPub_gameId ] );
	scrVarPub_gameId = 0;
}

/* ---- Scr_GetChecksum  0x00475810 ---- VERIFIED */
_DWORD *__cdecl Scr_GetChecksum(_DWORD *a1)
{
  _DWORD *result;
  int v2;
  int v3;

  result = a1;
  v2 = scrCompilePub_programLen;
  *a1 = scrCompilePub_checksum;
  v3 = dword_1407384;
  a1[1] = v2;
  a1[2] = v3 - scrVarPub_programBuffer;
  return result;
}
