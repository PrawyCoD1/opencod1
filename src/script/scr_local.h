/*
 * Private interface for the CoD1 GSC script VM (script/scr_*).
 *
 * Reconstructed from Call of Duty 1.1 (Windows, CoDMP.exe).  opencoduo
 * (coduo-server-master/dedicated-engine/src/scripting) is the same VM one
 * engine revision later.
 *
 * Nothing in the retail script/*.cpp units is actually C++ apart from the
 * script error exception; the whole tree compiles as C.  The retail symbols
 * carry Aspyr's GNU C++ mangling ("MT_GetScore__Fi") because the names came out
 * of the Mac build's symbol table -- the mangling encodes the real argument
 * types, which is why it is quoted in comments below.
 *
 * @fidelity: likely
 */

#ifndef __SCR_LOCAL_H__
#define __SCR_LOCAL_H__

#include "../qcommon/qcommon.h"

/*
 * ---------------------------------------------------------------------------
 * Script memory tree (scr_memorytree.cpp, retail 0x0046DB80-0x0046E580)
 * ---------------------------------------------------------------------------
 *
 * A buddy allocator over a fixed 512 KiB arena of 8-byte blocks.  Every script
 * allocation is identified by a 16-bit block index, which is what makes a
 * script string or variable handle fit in a word.
 *
 * Free blocks of each power-of-two bucket live in a treap: ordered by block
 * index (searched by the implicit centre/span bisection of [0,65536)) and
 * heap-ordered by MT_GetScore, which prefers blocks that keep the arena
 * coalescible.
 *
 * Retail arena base: 0x008E6318.  Retail also caches &blocks[0] in a global at
 * 0x008E6208 (GetRefString_var); the array is indexed directly here.
 */

#define SCR_MT_BLOCK_COUNT      0x10000     /* 65536 blocks */
#define SCR_MT_BLOCK_SIZE       8
#define SCR_MT_MAX_BUCKET       16
#define SCR_MT_BUCKET_COUNT     (SCR_MT_MAX_BUCKET + 1)
#define SCR_MT_LOOKUP_COUNT     256
#define SCR_MT_MAX_ALLOC_SIZE   0x10000

typedef union scrMemBlock_u {
	struct {
		unsigned short  left;
		unsigned short  right;
		unsigned int    payload;
	} node;
	unsigned char       bytes[SCR_MT_BLOCK_SIZE];
} scrMemBlock_t;

extern scrMemBlock_t    scrMemTree_blocks[SCR_MT_BLOCK_COUNT];
extern unsigned short   scrMemTree_freeRoots[SCR_MT_BUCKET_COUNT];
extern int              scrMemTree_allocBuckets;    /* retail 0x008E6310 */
extern int              scrMemTree_allocInstances;  /* retail 0x008E60D8 */

void            MT_Init( void );
unsigned short  MT_AllocIndex( int size, int type );
void            MT_FreeIndex( unsigned short blockIndex, unsigned int size );
void           *MT_Alloc( int size, int type );
void            MT_Free( void *p, unsigned int size );
qboolean        MT_Realloc( int oldSize, int newSize );
void            MT_DumpTree( void );
int             Scr_GetStringUsage( void );

/*
 * ---------------------------------------------------------------------------
 * Script string list (scr_stringlist.cpp, retail 0x0046F7C0-0x004704B0)
 * ---------------------------------------------------------------------------
 *
 * Every string the VM has ever seen is interned exactly once in the memory
 * tree and referred to by its 16-bit block index ("string handle").  A 16384
 * entry open-addressed table with an intrusive free list maps hash -> handle.
 *
 * Slot 0 is not a hash bucket: slot0.linkAndFlags is the head of the free-slot
 * list and slot0.stringHandle its tail.  A slot's two flag bits mean:
 *
 *   0x8000  occupied, and this slot is the head of its hash chain
 *   0x4000  occupied, chained off some other slot's hash
 *   0x0000  free -- linkAndFlags/stringHandle are the free list's next/prev
 *
 * Retail table base: 0x00966458.  Retail arena entry layout at handle h is
 * blocks[h]: refCount(u16) sizeByte(u8) flags(u8) then the NUL-terminated text.
 * "flags" is the per-usage ownership mask that SL_ShutdownSystem sweeps.
 */

#define SCR_STRING_HASH_SLOT_COUNT      16384
#define SCR_STRING_HASH_SLOT_MASK       0x3FFF
#define SCR_STRING_HASH_LINK_MASK       0x3FFF
#define SCR_STRING_HASH_FLAGS_MASK      0xC000
#define SCR_STRING_HASH_OCCUPIED        0x8000
#define SCR_STRING_HASH_CHAINED         0x4000
#define SCR_STRING_HASH_EMPTY           0x0000
#define SCR_STRING_HASH_FIRST_SLOT      1
#define SCR_STRING_HASH_MULTIPLIER      31
#define SCR_STRING_HASH_SHORT_LIMIT     0x100
#define SCR_STRING_ENTRY_HEADER_SIZE    4
#define SCR_STRING_FORMAT_BUFFER_SIZE   128
#define SCR_STRING_CANONICAL_LIMIT      0x400

/* usage masks -- the "user" argument of SL_GetString and friends */
#define SCR_STRING_USER_SCRIPT          1
#define SCR_STRING_USER_CANONICAL       2

typedef struct scrStringEntry_s {
	unsigned short  refCount;
	unsigned char   sizeByte;
	unsigned char   flags;
	char            text[1];        /* runs on into the following blocks */
} scrStringEntry_t;

typedef struct scrStringHashSlot_s {
	unsigned short  linkAndFlags;
	unsigned short  stringHandle;
} scrStringHashSlot_t;

extern scrStringHashSlot_t  scrStringHashSlots[SCR_STRING_HASH_SLOT_COUNT];
extern scrStringHashSlot_t *scrString_freedHashSlot;     /* retail 0x00966450 */
extern unsigned short      *scrString_canonicalMap;
extern unsigned short       scrString_canonicalCount;

scrStringEntry_t *GetRefString( unsigned short handle );

void            SL_Init( void );
const char     *SL_ConvertToString( unsigned short handle );
unsigned short  SL_ConvertFromString( const char *text );
unsigned short  SL_FindStringOfLen( const char *text, unsigned int size );
unsigned short  SL_FindString( const char *text );
unsigned short  SL_FindLowercaseString( const char *text );
unsigned short  SL_GetStringOfLen( const char *text, unsigned char user,
								   unsigned int size, int type );
unsigned short  SL_GetString_( const char *text, unsigned char user, int type );
unsigned short  SL_GetString( const char *text, unsigned char user );
unsigned short  SL_GetLowercaseStringOfLen( const char *text, unsigned char user,
											unsigned int size, int type );
unsigned short  SL_GetLowercaseString_( const char *text, unsigned char user,
										int type );
unsigned short  SL_GetLowercaseString( const char *text, unsigned char user );
void            SL_AddRefToString( unsigned short handle );
void            SL_RemoveRefToString( unsigned short handle );
void            SL_RemoveRefToStringOfLen( unsigned short handle,
										   unsigned int size );
void            SL_TransferRefToString( unsigned short handle,
										unsigned char usage );
void            SL_FreeString( unsigned short handle, const char *text,
							   unsigned int size );
void            SL_ShutdownSystem( unsigned char usage );
void            Scr_SetString( unsigned short *slot, unsigned short value );
unsigned short  Scr_AllocString( const char *text );
unsigned short  SL_GetStringForFloat( float value );
unsigned short  SL_GetStringForInt( int value );
unsigned short  SL_GetStringForVector( const float *v );
void            CreateCanonicalFilename( char *dest, const char *source,
										 int maxLength );
unsigned short  Scr_CreateCanonicalFilename( const char *filename );

/*
 * ---------------------------------------------------------------------------
 * Script temp memory (scr_tempmemory.cpp, retail 0x00470470-0x004704B0)
 * ---------------------------------------------------------------------------
 * A bump allocator on top of the hunk's temp-memory region.  The compiler uses
 * it for the parse tree, which is thrown away wholesale after each GSC file.
 */

/* the cursor lives in qcommon/cod1_globals.c as "currentPos" (retail
 * 0x01407368); scr_main.cpp writes it directly.  See scr_tempmemory.cpp. */

void            TempMemoryReset( void );
void           *TempMalloc( int len );
void            TempMemorySetPos( void *pos );

/*
 * ---------------------------------------------------------------------------
 * Script variable system (scr_variable.cpp, retail 0x00470530-0x00475810)
 * ---------------------------------------------------------------------------
 *
 * Two parallel fixed arrays of 65536 entries, and everything in the VM is a
 * 16-bit index into them.
 *
 *   scrVarIndirections[]   retail 0x00976478,  4 bytes per entry
 *   scrVarNodes[]          retail 0x009B6478, 12 bytes per entry
 *
 * The 4-byte entry is the HANDLE and the 12-byte entry is the NODE.  Almost
 * every public entry point deals in node indices; the hash table and the
 * sibling lists deal in handle indices.  Handle h names node
 * scrVarIndirections[h].id, and the mapping is only the identity until
 * MakeVariableExternal swaps a pair to unhook a variable from its bucket
 * without moving the node its value lives in.
 *
 *   node.status  bits 0-4   variable type (scrVarType_t)
 *                bits 5-6   hash occupancy, see VAR_STAT_* below
 *                bits 8-31  the variable's NAME, which is one of
 *                             0x00000-0x0FFFF  a string handle
 *                             0x10000-0x1FFFF  0x10000 + an object node index
 *                             anything else    an array subscript, folded by
 *                                              GetInternalVariableIndex
 *                           For an ENTITY node it is the class number instead.
 *
 * The word pair at node +0x00/+0x02 is packed: what each half means depends
 * on the node's type, and the free list overlays both of them.  See the VAR_*
 * accessors in scr_variable.cpp, which name every use at its offset.
 *
 * Node 0 is not a variable.  It is the free list sentinel: node0.hashNext is
 * the head and node0.u.halfword[0] the tail, and a zero handle is what every
 * Find* entry point returns for "not present".
 *
 * The 0x60 occupancy field is what makes a node's +0x08 link unambiguous:
 *
 *   VAR_STAT_FREE      0x00  on the free list; +0x08 next free, +0x00 prev free
 *   VAR_STAT_CHAINED   0x20  a collision cell interposed in a bucket's chain
 *   VAR_STAT_HEAD      0x40  occupies its own hash bucket; +0x08 chains on
 *   VAR_STAT_EXTERNAL  0x60  in no bucket at all; +0x08 is its own handle
 *
 * Identical to opencoduo: the same 12-byte node, the same 0x1f/0x60/<<8
 * packing, the same 19 types and the same hash (name + 31 * parent) % 0xffff
 * + 1.  The CoD1 typename table at 0x005757C8 has all 19 entries in this order.
 */

#define VARIABLELIST_SIZE       0x10000      /* nodes and handles alike */

typedef enum {
	VAR_UNDEFINED       = 0,
	VAR_STRING          = 1,
	VAR_ISTRING         = 2,     /* "localized string" */
	VAR_VECTOR          = 3,
	VAR_FLOAT           = 4,
	VAR_INTEGER         = 5,
	VAR_CODEPOS         = 6,
	VAR_OBJECT          = 7,
	VAR_KEYVALUE        = 8,     /* "key/value": a builtin entity field */
	VAR_FUNCTION        = 9,
	VAR_STACK           = 10,
	VAR_ANIMATION       = 11,
	/* 12 and up are object nodes rather than values */
	VAR_THREAD          = 12,
	VAR_ENTITY          = 13,
	VAR_STRUCT          = 14,
	VAR_ARRAY           = 15,
	VAR_DEADTHREAD      = 16,
	VAR_DEADENTITY      = 17,
	VAR_DEADOBJECT      = 18,

	VAR_TYPE_COUNT      = 19
} scrVarType_t;

#define VAR_STATUS_TYPE_MASK    0x0000001Fu
#define VAR_STATUS_STAT_MASK    0x00000060u
#define VAR_STATUS_NAME_SHIFT   8

#define VAR_STAT_FREE           0x00000000u
#define VAR_STAT_CHAINED        0x00000020u
#define VAR_STAT_HEAD           0x00000040u
#define VAR_STAT_EXTERNAL       0x00000060u

/* name-space boundaries, see the node.status comment above */
#define VAR_NAME_OBJECT_BASE    0x00010000u
#define VAR_NAME_OBJECT_LIMIT   0x00020000u
#define VAR_NAME_INDEX_BIAS     0x00800000u
#define VAR_NAME_INDEX_MASK     0x00FFFFFFu

typedef union VariableUnion_u {
	int                 intValue;
	unsigned int        uintValue;
	float               floatValue;
	unsigned short      stringValue;
	const float        *vectorValue;
	const char         *codePosValue;
	void               *pointerValue;
	/* [0] is at +0x00 and [1] at +0x02; scr_variable.cpp names every use */
	unsigned short      halfword[2];
} VariableUnion;

typedef struct VariableValue_s {
	VariableUnion       u;              /* +0x00 */
	int                 type;           /* +0x04 */
} VariableValue;

typedef struct VariableValueInternal_s {
	VariableUnion       u;              /* +0x00 */
	unsigned int        status;         /* +0x04 */
	unsigned short      hashNext;       /* +0x08 */
	unsigned short      nextSibling;    /* +0x0A */
} VariableValueInternal;

typedef struct Variable_s {
	unsigned short      id;             /* +0x00  node this handle names */
	unsigned short      prevSibling;    /* +0x02 */
} Variable;

/* the table Scr_SetClassMap is handed by the game module: 8 bytes per entry,
 * a field-map object handle at +0x00 and a name pointer at +0x04. */
typedef struct scrClassMapEntry_s {
	unsigned short      id;
	unsigned short      pad;
	const char         *name;
} scrClassMapEntry_t;

extern Variable                 scrVarIndirections[VARIABLELIST_SIZE];
extern VariableValueInternal    scrVarNodes[VARIABLELIST_SIZE];

/* retail 0x01407358 low word (byte 2 of the same dword is scr_inited in
 * scr_vm_state.c).  The scratch node every entity key/value read and
 * CastFieldObject writes through. */
extern unsigned short           scrVarPub_tempVariable;

extern const char * const       var_typename[VAR_TYPE_COUNT];  /* retail 0x005757C8 */

/* owned by scr_vm_state.c */
extern qboolean                 scr_inited;
extern unsigned short           scr_timeArrayId;

/* pool and lifetime */
void            InitVariables( void );
void            Var_Init( void );
int             Scr_GetNumScriptVars( void );

/* allocation */
VariableValueInternal *AllocVariable( void );
unsigned short  AllocValue( void );
unsigned short  AllocObject( void );
unsigned short  Scr_AllocArray( void );
unsigned short  AllocThread( unsigned short self );
unsigned short  AllocEntity( int classnum, unsigned short entnum );
void            FreeVariable( unsigned short nodeIndex );

/* refcounting */
void            AddRefToObject( unsigned short id );
void            RemoveRefToObject( unsigned short id );
void            AddRefToValue( int type, VariableUnion u );
void            RemoveRefToValue( int type, VariableUnion u );
/* The VariableValue* overloads of the two above -- separate functions at
 * 0x004704D0 / 0x004704E0, not aliases.  Defined in scr_vm.cpp (per the Mac
 * symbol table).  Mangled spellings because C cannot overload. */
void            AddRefToValue__FP13VariableValue( VariableValue *value );
void            RemoveRefToValue__FP13VariableValue( VariableValue *value );
float          *AllocVector( void );
VariableUnion   AllocVectorValue( const float *v );
void            AddRefToVector( const float *v );
void            RemoveRefToVector( const float *v );

/* lookup */
unsigned short  FindVariableIndexInternal( unsigned short parentId, unsigned int name );
unsigned short  GetVariableIndexInternal( unsigned short parentId, unsigned int name );
unsigned short  FindVariable( unsigned short parentId, unsigned int name );
unsigned short  GetVariable( unsigned short parentId, unsigned int name );
unsigned short  FindObjectVariable( unsigned short parentId, unsigned short childId );
unsigned short  GetObjectVariable( unsigned short parentId, unsigned short childId );
unsigned short  FindArrayVariable( unsigned short parentId, int index );
unsigned short  GetArrayVariable( unsigned short parentId, int index );
unsigned short  FindArrayVariableIndex( unsigned short parentId, unsigned int index );
unsigned short  GetArrayVariableIndex( unsigned short parentId, unsigned int index );
unsigned short  GetVariableField( unsigned short parentId, unsigned short name );
void            ClearVariableField( unsigned short parentId, unsigned short name );
qboolean        IsValidArrayIndex( unsigned int index );
unsigned int    GetInternalVariableIndex( unsigned int index );

/* removal */
void            MakeVariableExternal( Variable *slot, VariableValueInternal *parent );
void            RemoveVariable( unsigned short parentId, unsigned int name );
void            SafeRemoveVariable( unsigned short parentId, unsigned int name );
void            RemoveArrayVariable( unsigned short parentId, unsigned int index );
void            SafeRemoveArrayVariable( unsigned short parentId, unsigned int index );
void            RemoveObjectVariable( unsigned short parentId, unsigned short childId );
void            ClearObject( unsigned short id );
void            ClearObjectInternal( unsigned short id );
void            FreeValue( unsigned short id );
void            FreeValueInternal( VariableValueInternal *node );

/* accessors */
int             GetVarType( unsigned short id );
unsigned int    GetVariableName( unsigned short id );
unsigned short  GetVariableKeyObject( unsigned short id );
unsigned int    GetEntityType( unsigned short id );
unsigned short  GetThreadNotifyName( unsigned short id );
void            SetThreadNotifyName( unsigned short id, unsigned short name );
void            ClearThreadNotifyName( unsigned short id );
unsigned short  GetSelf( unsigned short id );
unsigned short  GetEntnum( unsigned short id );
unsigned short  GetArraySize( unsigned short id );
unsigned short  FindNextSibling( unsigned short id );
unsigned short  GetObject( unsigned short id );
unsigned short  GetArray( unsigned short id );
unsigned short  FindObject( unsigned short id );
qboolean        IsFieldObject( unsigned short id );
qboolean        Scr_IsThreadAlive( unsigned short id );

VariableValue  *GetVariableValueAddress( unsigned short id );
void            GetVariableValue( unsigned short id, VariableValue *value );
void            GetVariableFieldValue( unsigned short id, VariableValue *value );
void            SetVariableValue( unsigned short id, VariableValue *value );
void            SetNewVariableValue( unsigned short id, VariableValue *value );
void            SetVariableFieldValue( unsigned short id, VariableValue *value );
void            ClearVariableValue( unsigned short id );
void            GetSizeValue( VariableValue *value );
unsigned short  CastFieldObject( VariableValue *value );
void            CopyArray( unsigned short sourceId, unsigned short destId );
void            EvalArray( VariableValue *index, VariableValue *container );
void            KillThread( unsigned short id );

/* entity field table (scr_variable.cpp, retail 0x004751B0-0x00475600).
 * One record is `char name[]; '\0'; u16 offset; s8 type`, packed with no
 * alignment and terminated by an empty name. */
extern char    *scrVarPub_fieldBuffer;          /* retail 0x00976468 */

short           Scr_FindField( const char *name, int *typeOut );
void            Scr_AddFields( const char *path, const char *extension );

/* the `game` array, and value lifetime for the game module */
extern unsigned short  scrVarPub_gameId;        /* retail 0x00A7A5DC, owned by scr_vm.cpp */

void            Scr_FreeValue( unsigned short id );
int             Scr_MakeValuePrimitive( unsigned short arrayId );
void            Scr_AllocGameVariable( void );
void            Scr_FreeGameVariable( int bComplete );

/* class map / entity ids */
void            Scr_SetClassMap( scrClassMapEntry_t *classMap, unsigned int classCount );
void            Scr_RemoveClassMap( void );
void            Scr_AddClassField( unsigned short classId, const char *name, unsigned short offset );
int             Scr_GetOffset( unsigned short classId, const char *name );
unsigned short  FindEntityId( int entnum, int classnum );
unsigned short  Scr_GetEntityId( int entnum, int classnum );
void            Scr_FreeEntityNum( int entnum, int classnum );

/*
 * ---------------------------------------------------------------------------
 * Provided elsewhere in script/
 * ---------------------------------------------------------------------------
 */
void            Scr_DumpScriptThreads( void );
void            Scr_Error( const char *msg );
void            Scr_TerminalError( const char *msg );

/*
 * Spelled with the Mac build's mangling, which is the symbol scr_vm.cpp
 * defines.  "__FUiiiP13VariableValue" gives the argument order:
 * (unsigned int classnum, int entnum, int offset, VariableValue *value), the
 * pointer last.  TODO: rename.
 */
void            GetEntityFieldValue__FUiiiP13VariableValue(
					unsigned int classnum, int entnum, int offset, VariableValue *value );
void            SetEntityFieldValue__FUiiiP13VariableValue(
					unsigned int classnum, int entnum, int offset, VariableValue *value );
void            VM_CancelNotify__FUsUs( unsigned short startLocalId,
										unsigned short notifyListId );
void            VM_TerminateNotifyList__FUs( unsigned short id );

#endif /* __SCR_LOCAL_H__ */
