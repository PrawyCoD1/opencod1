/*
 * script/scr_compiler.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_compiler.cpp
 *
 * Retail range 0x00468820-0x0046CD20, 127 functions.
 *
 * @fidelity: verified
 */

#include "scr_local.h"
#include "../qcommon/hexrays_shim.h"

#define FindNextSibling     cod1_globals_FindNextSibling_data
#define FindObject          cod1_globals_FindObject_data
#include "../qcommon/cod1_globals.h"
#undef FindNextSibling
#undef FindObject

#include <malloc.h>

typedef unsigned int sval_t;

typedef struct sval_cell_s {
	sval_t                  value;
	struct sval_cell_s     *next;
} sval_cell_t;

typedef struct sval_list_s {
	sval_cell_t            *first;
	sval_cell_t            *last;
} sval_list_t;

typedef struct sval_entry_s {
	sval_t                  node;
	unsigned int            sourcePos;
} sval_entry_t;

typedef struct sval_stmtlist_s {
	sval_t                  kind;
	sval_cell_t            *head;
} sval_stmtlist_t;

#define SVAL_NODE( v )      ( (sval_t *)(v) )
#define SVAL_LIST( v )      ( (sval_list_t *)(v) )
#define SVAL_ENTRY( v )     ( (sval_entry_t *)(v) )
#define SVAL_BLOCKLIST( v ) ( *(sval_stmtlist_t **)(v) )

enum {
	SVAL_ASSIGNMENT_STATEMENT       = 0x02,
	SVAL_STRING_REF                 = 0x03,
	SVAL_PRIMITIVE_EXPRESSION       = 0x04,
	SVAL_INTEGER_LITERAL            = 0x05,
	SVAL_FLOAT_LITERAL              = 0x06,
	SVAL_NEG_INTEGER_LITERAL        = 0x07,
	SVAL_NEG_FLOAT_LITERAL          = 0x08,
	SVAL_STRING                     = 0x09,
	SVAL_ISTRING                    = 0x0A,
	SVAL_ARRAY_REF                  = 0x0B,
	SVAL_FIELD_REF                  = 0x0C,
	SVAL_REFERENCE_EXPRESSION       = 0x0D,
	SVAL_SCRIPT_FUNCTION_NAME       = 0x0E,
	SVAL_CALL_VALUE                 = 0x0F,
	SVAL_FUNCTION_REF               = 0x10,
	SVAL_SCRIPT_FUNCTION_REF        = 0x11,
	SVAL_FUNCTION_POINTER_CALL      = 0x12,
	SVAL_FUNCTION_CALL_VALUE        = 0x13,
	SVAL_METHOD_CALL_VALUE          = 0x14,
	SVAL_CALL_STATEMENT             = 0x15,
	SVAL_FUNCTION_CALL              = 0x16,
	SVAL_RETURN_VALUE_STATEMENT     = 0x17,
	SVAL_RETURN_STATEMENT           = 0x18,
	SVAL_WAIT_STATEMENT             = 0x19,
	SVAL_METHOD_CALL                = 0x1A,
	SVAL_UNDEFINED                  = 0x1B,
	SVAL_SELF                       = 0x1C,
	SVAL_LEVEL                      = 0x1D,
	SVAL_GAME                       = 0x1E,
	SVAL_ANIM                       = 0x1F,
	SVAL_IF_STATEMENT               = 0x20,
	SVAL_IF_ELSE_STATEMENT          = 0x21,
	SVAL_WHILE_STATEMENT            = 0x22,
	SVAL_DO_WHILE_STATEMENT         = 0x23,
	SVAL_FOR_STATEMENT              = 0x24,
	SVAL_INC_STATEMENT              = 0x25,
	SVAL_DEC_STATEMENT              = 0x26,
	SVAL_REF_ASSIGNMENT_STATEMENT   = 0x27,
	SVAL_STATEMENT_BLOCK            = 0x28,
	SVAL_DEVELOPER_BLOCK            = 0x29,
	SVAL_EXPRESSION_LIST            = 0x2A,
	SVAL_BOOL_OR                    = 0x2B,
	SVAL_BOOL_AND                   = 0x2C,
	SVAL_BINARY_OPERATOR            = 0x2D,
	SVAL_CAST_BOOL                  = 0x2E,
	SVAL_CAST_INT                   = 0x2F,
	SVAL_CAST_FLOAT                 = 0x30,
	SVAL_CAST_STRING                = 0x31,
	SVAL_BOOL_NOT                   = 0x32,
	SVAL_BOOL_COMPLEMENT            = 0x33,
	SVAL_SIZE                       = 0x34,
	SVAL_WAITTILL_STATEMENT         = 0x36,
	SVAL_WAITTILLMATCH_STATEMENT    = 0x37,
	SVAL_NOTIFY_STATEMENT           = 0x38,
	SVAL_ENDON_STATEMENT            = 0x39,
	SVAL_SWITCH_STATEMENT           = 0x3A,
	SVAL_CASE_STATEMENT             = 0x3B,
	SVAL_DEFAULT_STATEMENT          = 0x3C,
	SVAL_BREAK_STATEMENT            = 0x3D,
	SVAL_CONTINUE_STATEMENT         = 0x3E,
	SVAL_FOR_CONDITION              = 0x3F,
	SVAL_EMPTY_ARRAY                = 0x40,
	SVAL_ANIM_STRING                = 0x41,
	SVAL_FUNCTION_DEFINITION        = 0x42,
	SVAL_DEVELOPER_FUNCTION_DEF     = 0x43,
	SVAL_USING_ANIMTREE             = 0x44,
	SVAL_LITERAL_ZERO               = 0x45,
	SVAL_LITERAL_ONE                = 0x46,
	SVAL_ANIMTREE                   = 0x47
};

enum {
	OP_End                      = 0x00,
	OP_Return                   = 0x01,
	OP_GetUndefined             = 0x02,
	OP_GetInteger               = 0x03,
	OP_GetFloat                 = 0x04,
	OP_GetString                = 0x05,
	OP_GetIString               = 0x06,
	OP_GetSelfObject            = 0x07,
	OP_GetLevelObject           = 0x08,
	OP_GetAnimObject            = 0x09,
	OP_GetSelf                  = 0x0A,
	OP_GetLevel                 = 0x0B,
	OP_GetGame                  = 0x0C,
	OP_GetAnim                  = 0x0D,
	OP_GetAnimation             = 0x0E,
	OP_GetGameRef               = 0x0F,
	OP_GetFunction              = 0x10,
	OP_EvalLocalVariable        = 0x11,
	OP_EvalLocalVariableRef     = 0x12,
	OP_ClearLocalVariable       = 0x13,
	OP_EvalArray                = 0x14,
	OP_EvalArrayRef             = 0x15,
	OP_EvalArrayRefNoCreate     = 0x16,
	OP_ClearArray               = 0x17,
	OP_EmptyArray               = 0x18,
	OP_EvalFieldVariable        = 0x19,
	OP_EvalFieldVariableRef     = 0x1A,
	OP_ClearFieldVariable       = 0x1B,
	OP_DiscardRef               = 0x1C,
	OP_SafeSetVariableField     = 0x1D,
	OP_SafeSetWaittillField     = 0x1E,
	OP_EndWaittill              = 0x1F,
	OP_CheckClearParams         = 0x20,
	OP_SetVariableField         = 0x21,
	OP_CallBuiltin              = 0x22,
	OP_CallBuiltinMethod        = 0x23,
	OP_Wait                     = 0x24,
	OP_PreScriptCall            = 0x25,
	OP_ScriptFunctionCall       = 0x26,
	OP_ScriptFunctionCallPtr    = 0x27,
	OP_ScriptMethodCall         = 0x28,
	OP_ScriptMethodCallPtr      = 0x29,
	OP_ScriptThreadCall         = 0x2A,
	OP_ScriptThreadCallPtr      = 0x2B,
	OP_ScriptMethodThreadCall   = 0x2C,
	OP_ScriptMethodThreadCallPtr= 0x2D,
	OP_DecTop                   = 0x2E,
	OP_CastFieldObject          = 0x2F,
	OP_CastBool                 = 0x30,
	OP_CastInt                  = 0x31,
	OP_CastFloat                = 0x32,
	OP_CastString               = 0x33,
	OP_BoolNot                  = 0x34,
	OP_BoolComplement           = 0x35,
	OP_JumpOnFalse              = 0x36,
	OP_JumpOnTrueBack           = 0x37,
	OP_JumpOnFalseExpr          = 0x38,
	OP_JumpOnTrueExpr           = 0x39,
	OP_Jump                     = 0x3A,
	OP_JumpBack                 = 0x3B,
	OP_Inc                      = 0x3C,
	OP_Dec                      = 0x3D,
	OP_Size                     = 0x4E,
	OP_WaitTillMatch            = 0x4F,
	OP_WaitTill                 = 0x50,
	OP_Notify                   = 0x51,
	OP_EndOn                    = 0x52,
	OP_VoidCodepos              = 0x53,
	OP_Switch                   = 0x54,
	OP_EndSwitch                = 0x55,
	OP_Vector                   = 0x56,
	OP_NOP                      = 0x57,
	OP_DevBlockBegin            = 0x58,
	OP_DevBlockDeferred         = 0x59
};

#define LOCALDEPTH_NONE         0
#define LOCALDEPTH_BUILTIN      1
#define LOCALDEPTH_THREAD       2
#define LOCALDEPTH_CALL         3

#define SCR_CODEGEN_INTERN      0
#define SCR_CODEGEN_RELOCATE    1
#define SCR_CODEGEN_DISCARD     2

#define SCR_STRING_USAGE_CASE       1
#define SCR_STRING_USAGE_FUNCTION   2
#define SCR_STRING_USAGE_CANONICAL  2

#define SCR_FUNC_REFCOUNT       0
#define SCR_FUNC_DEFINITION     1
#define SCR_FUNC_FIRST_REF      2

#define SCR_DEV_OP_BUF_SIZE     0x100000
#define SCR_DEV_PATCH_CAPACITY  0x10000
#define SCR_BUILTIN_PARAM_LIMIT 0x100
#define SCR_LOCAL_SLOT_WIDTH    0x20
#define SCR_OPERAND_STACK_LIMIT 0x800

#define SCR_CASE_TEST_BIAS      0x007E0000u
#define SCR_CASE_TEST_LIMIT     0x00FE0000u
#define SCR_CASE_ENCODE_BIAS    0x00800000u
#define SCR_CASE_ENCODE_MASK    0x00FFFFFFu

#define scrCompileGlob_animTreeValue    scrCompileGlob_animTreeValue    /* 0x008E5ED8 */
#define scrCompilePub_developerBufPos   developerCodeStart    /* 0x008E5EE4 */
#define scrCompileGlob_codegenMode      developer_script_int /* 0x008E5EE8 */
#define scrCompileGlob_currentFuncRoot  scrCompileGlob_currentFuncRoot    /* 0x008E5EEC (low word) */
#define scrCompileGlob_loadCursor       scrCompileGlob_loadCursor    /* 0x008E5EF4 */
#define scrCompileGlob_codePos          scrCompileGlob_codePos    /* 0x008E5EFC */
#define scrCompileGlob_codeRelocStart   scrCompileGlob_codeRelocStart    /* 0x008E5F00 */
#define scrCompileGlob_lastOpcodePos    scrCompileGlob_lastOpcodePos    /* 0x00966430 */
#define scrCompilePub_checksum          scrCompilePub_checksum   /* 0x01407364 */
#define scrVarPub_programBuffer         scrVarPub_programBuffer   /* 0x01407380 */
#define scrCompilePub_programLen        scrCompilePub_programLen   /* 0x01407388 */
#define scrCompileGlob_devOpBuffer      scrCompileGlob_devOpBuffer   /* 0x0140738C */
#define scrCompileGlob_devPatchMax      scrCompileGlob_devPatchMax   /* 0x01407390 */
#define scrCompileGlob_devPatchTable    scrCompileGlob_devPatchTable   /* 0x01407394 */
#define scrCompileGlob_stringFixups     scrCompileGlob_stringFixups   /* 0x01407398 */
#define scrCompileGlob_breakAllowed     scrCompileGlob_breakAllowed    /* 0x0140739C */
#define scrCompileGlob_breakAllowedDev  scrCompileGlob_breakAllowedDev    /* 0x0140739D */
#define scrCompileGlob_breakList        scrCompileGlob_breakList   /* 0x014073A0 */
#define scrCompileGlob_contAllowed      scrCompileGlob_contAllowed    /* 0x014073A4 */
#define scrCompileGlob_contAllowedDev   scrCompileGlob_contAllowedDev    /* 0x014073A5 */
#define scrCompileGlob_contList         scrCompileGlob_contList   /* 0x014073A8 */
#define scrCompileGlob_caseAllowed      scrCompileGlob_caseAllowed    /* 0x014073AC */
#define scrCompileGlob_caseAllowedDev   scrCompileGlob_caseAllowedDev    /* 0x014073AD */
#define scrCompileGlob_caseList         scrCompileGlob_caseList   /* 0x014073B0 */
#define scrAnimPub_currentAnimTree      dword_8E60B0    /* 0x008E60B0 (low word) */
#define scrCompilePub_loadedScripts     scrCompilePub_loadedScripts    /* 0x008E60B4 (low word) */
#define scrCompilePub_scriptCount       scrCompilePub_scriptCount    /* 0x008E60B8 */
#define scrCompilePub_developerScript   scrVarPub_developerScript    /* 0x008E60C4 */
#define scrCompilePub_scriptsPos        scrCompilePub_scriptsPos    /* 0x008E60CC (low word) */
#define scrCompilePub_scriptsAvailable  scrVarPub_developer    /* 0x008E60D0 */

#define SCR_HANDLE( g )     ( *(unsigned short *)&(g) )

#define scrCompileGlob_bDeferredCheck   ( *( (unsigned char *)&scrAnimPub_currentTreeRoot + 2 ) )
#define scrCompileGlob_bOwnsStrings     ( *( (unsigned char *)&scrAnimPub_currentTreeRoot + 3 ) )

int     scrCompileGlob_cumulOffset;     /* retail 0x008E5EF0 */
int     scrCompileGlob_maxOffset;       /* retail 0x008E5EF8 */
int     scrCompileGlob_maxCallOffset;   /* retail 0x008E5EE0 */

typedef struct scrCodeOffsetPatch_s {
	char                           *codePos;
	struct scrCodeOffsetPatch_s    *next;
} scrCodeOffsetPatch_t;

typedef struct scrCaseRecord_s {
	unsigned int                    value;
	unsigned int                    codePos;
	unsigned int                    sourcePos;
	struct scrCaseRecord_s         *next;
} scrCaseRecord_t;

typedef struct scrCaseTableEntry_s {
	unsigned int                    value;
	unsigned int                    codePos;
} scrCaseTableEntry_t;

typedef struct scrStringFixup_s {
	unsigned short                 *codePos;
	struct scrStringFixup_s        *next;
} scrStringFixup_t;

typedef struct scrScriptLoad_s {
	unsigned short                  filename;
	unsigned short                  pad;
	unsigned int                    sourcePos;
} scrScriptLoad_t;

extern void         *Hunk_AllocateTempMemoryHighInternal( int size );
extern int           Hunk_CommitTempMemory( void );
extern int           Hunk_ClearTempMemoryHigh( void );

extern void          AddOpcodePos( unsigned int sourcePos );
extern void QDECL    CompileError( unsigned int sourcePos, const char *format, ... );
extern void QDECL    CompileError2( const char *codePos, const char *format, ... );

extern unsigned short SL_TransferToCanonicalString( unsigned short handle );
extern qboolean      Scr_LoadScript( const char *filename );

extern qboolean      Scr_IsIdentifier( const char *name );

extern void         *Scr_GetFunction( const char **name, int *developerOnly );
extern void         *Scr_GetMethod( const char **name, int *developerOnly );

extern void          Scr_EmitAnimationInternal( char *animRef, unsigned short animName,
												unsigned short tree, unsigned int sourcePos );
extern unsigned short Scr_UsingTreeInternal( const char *name, int *animTreeValue );

static void  EmitExpression( sval_t node );
static void  EmitPrimitiveExpression( sval_t node );
static void  EmitPrimitiveExpressionFieldObject( sval_t node, unsigned int sourcePos );
static void  EmitExpressionFieldObject( sval_t node, unsigned int sourcePos );
static void  EmitVariableExpression( sval_t node );
static void  EmitVariableExpressionRef( sval_t node, qboolean bCreate );
static void  EmitArrayExpressionRef( sval_t node, qboolean bCreate, unsigned int sourcePos );
static void  EmitArrayPrimitiveExpressionRef( sval_t node, qboolean bCreate,
											  unsigned int sourcePos );
static void  EmitStatement( sval_t node );
static void  EmitStatementList( sval_t block );
static void  EmitCall( sval_t callNode, sval_list_t *args, qboolean bDropTop );
static void  EmitMethod( sval_t objectNode, sval_t callNode, sval_list_t *args,
						 unsigned int methodSourcePos, qboolean bDropTop );
static void  EmitFunction( sval_t node, unsigned int sourcePos );
static int   EmitExpressionList( sval_list_t *list );
static void  AddExpressionListOpcodePos( sval_list_t *list );
static void  Scr_TransferToDeveloperBuffer( void );

static void  Scr_TransferStatementListToDeveloperBuffer( void );
static void  EmitDeveloperStatementList( sval_t block, unsigned int sourcePos );

void         Scr_InitDeveloperOpcodes__Fv( void );
void         Scr_InsertDeveloperOpcodes__Fv( void );
void         Scr_ShutdownDeveloperOpcodes__Fv( void );

/* ---- CompileRemoveRefToString  0x00468820 ---- */
static void CompileRemoveRefToString( unsigned short string ) {
	if ( !scrCompileGlob_bOwnsStrings ) {
		SL_RemoveRefToString( string );
	}
}

/* ---- EmitCanonicalString  0x00468870 ---- */
static void EmitCanonicalString( unsigned short string ) {
	unsigned short     *out;
	scrStringFixup_t   *fixup;

	out = (unsigned short *)TempMalloc( 2 );
	scrCompileGlob_codePos = (int)out;

	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_DISCARD ) {
		CompileRemoveRefToString( string );
		return;
	}

	if ( scrCompileGlob_bOwnsStrings ) {
		SL_AddRefToString( string );
	}

	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_INTERN ) {
		*out = SL_TransferToCanonicalString( string );
		return;
	}

	*out = string;
	fixup = (scrStringFixup_t *)Z_MallocInternal( (int)sizeof( scrStringFixup_t ) );
	fixup->codePos = (unsigned short *)( (char *)out
					+ ( (int)scrCompilePub_developerBufPos - scrCompileGlob_codeRelocStart ) );
	*fixup->codePos = 0;
	fixup->next = (scrStringFixup_t *)scrCompileGlob_stringFixups;
	scrCompileGlob_stringFixups = (int)fixup;
}

/* ---- CompileTransferRefToString  0x00468910 ---- */
static void CompileTransferRefToString( unsigned short string, unsigned char usage ) {
	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_DISCARD ) {
		CompileRemoveRefToString( string );
		return;
	}

	if ( scrCompileGlob_bOwnsStrings ) {
		SL_AddRefToString( string );
	}
	SL_TransferRefToString( string, usage );
}

/* ---- EmitOpcode  0x00468950 ---- */
static void EmitOpcode( unsigned char opcode, int stackDelta, int localDepthMode ) {
	unsigned char  *out;

	if ( scrCompileGlob_bDeferredCheck && scrCompileGlob_codegenMode == SCR_CODEGEN_INTERN ) {
		scrCompileGlob_bDeferredCheck = 0;
		Scr_TransferStatementListToDeveloperBuffer();
	}

	scrCompileGlob_cumulOffset += stackDelta;
	if ( scrCompileGlob_maxOffset < scrCompileGlob_cumulOffset ) {
		scrCompileGlob_maxOffset = scrCompileGlob_cumulOffset;
	}

	if ( localDepthMode && scrCompileGlob_maxCallOffset < scrCompileGlob_cumulOffset ) {
		scrCompileGlob_maxCallOffset = scrCompileGlob_cumulOffset;
		if ( localDepthMode == LOCALDEPTH_CALL ) {
			scrCompileGlob_maxCallOffset = scrCompileGlob_cumulOffset + 1;
		}
	}

	out = (unsigned char *)TempMalloc( 1 );
	*out = opcode;
	scrCompileGlob_codePos       = (int)out;
	scrCompileGlob_lastOpcodePos = (int)out;
	scrCompilePub_checksum = scrCompilePub_checksum * 31 + opcode;
}

/* ---- EmitNOP  0x004689F0 ---- */
static void EmitNOP( void ) {
	EmitOpcode( OP_NOP, 0, LOCALDEPTH_NONE );
}

/* ---- EmitInteger  0x00468A00 ---- */
static void EmitInteger( int value ) {
	int    *out = (int *)TempMalloc( 4 );

	scrCompileGlob_codePos = (int)out;
	*out = value;
}

/* ---- EmitShort  0x00468A30 ---- */
static void EmitShort( short value ) {
	short  *out = (short *)TempMalloc( 2 );

	scrCompileGlob_codePos = (int)out;
	*out = value;
}

/* ---- EmitByte  0x00468A60 ---- */
static void EmitByte( unsigned char value ) {
	unsigned char  *out = (unsigned char *)TempMalloc( 1 );

	scrCompileGlob_codePos = (int)out;
	*out = value;
}

/* ---- EmitFloat  0x00468A90 ---- */
static void EmitFloat( float value ) {
	float  *out = (float *)TempMalloc( 4 );

	scrCompileGlob_codePos = (int)out;
	*out = value;
}

/* ---- EmitString  0x00468AC0 ---- */
static void EmitString( unsigned short value ) {
	unsigned short *out = (unsigned short *)TempMalloc( 2 );

	scrCompileGlob_codePos = (int)out;
	*out = value;
}

/* ---- EmitBlankPos  0x00468AF0 ---- */
static void EmitBlankPos( void ) {
	int    *out = (int *)TempMalloc( 4 );

	scrCompileGlob_codePos = (int)out;
	*out = 0;
}

/* ---- EmitCodepos  0x00468B20 ---- */
static void EmitCodepos( const char *codePos ) {
	const char **out = (const char **)TempMalloc( 4 );

	scrCompileGlob_codePos = (int)out;
	*out = codePos;
}

/* ---- EmitBuiltinFunction  0x00468B50 ---- */
static void EmitBuiltinFunction( void *function ) {
	void  **out = (void **)TempMalloc( 4 );

	scrCompileGlob_codePos = (int)out;
	*out = function;
}

/* ---- EmitBuiltinMethod  0x00468B80 ---- */
static void EmitBuiltinMethod( void *method ) {
	void  **out = (void **)TempMalloc( 4 );

	scrCompileGlob_codePos = (int)out;
	*out = method;
}

/* ---- EmitGetFloat  0x00468C00 ---- */
static void EmitGetFloat( float value ) {
	EmitOpcode( OP_GetFloat, 1, LOCALDEPTH_NONE );
	EmitFloat( value );
}

static void EmitGetInteger( int value ) {
	EmitOpcode( OP_GetInteger, 1, LOCALDEPTH_NONE );
	EmitInteger( value );
}

static void EmitFalse( void ) {
	EmitGetInteger( 0 );
}

static void EmitTrue( void ) {
	EmitGetInteger( 1 );
}

static void EmitIncTop( void ) {
	EmitOpcode( OP_GetUndefined, 1, LOCALDEPTH_NONE );
}

static void EmitSelf( void )  { EmitOpcode( OP_GetSelf,  1, LOCALDEPTH_NONE ); }
static void EmitLevel( void ) { EmitOpcode( OP_GetLevel, 1, LOCALDEPTH_NONE ); }
static void EmitGame( void )  { EmitOpcode( OP_GetGame,  1, LOCALDEPTH_NONE ); }
static void EmitAnim( void )  { EmitOpcode( OP_GetAnim,  1, LOCALDEPTH_NONE ); }

static void EmitEmptyArray( void ) {
	EmitOpcode( OP_EmptyArray, 1, LOCALDEPTH_NONE );
}

static void EmitAnimTree( unsigned int sourcePos ) {
	if ( !scrCompileGlob_animTreeValue ) {
		CompileError( sourcePos, "#using_animtree was not specified" );
	}
	EmitOpcode( OP_GetInteger, 1, LOCALDEPTH_NONE );
	EmitInteger( scrCompileGlob_animTreeValue );
}

/* ---- EmitSetVariableField  0x00468D20 ---- */
static void EmitSetVariableField( unsigned int sourcePos ) {
	EmitOpcode( OP_SetVariableField, -1, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitSafeSetVariableField  0x00468D40 ---- */
static void EmitSafeSetVariableField( unsigned short string, unsigned int sourcePos ) {
	EmitOpcode( OP_SafeSetVariableField, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
	EmitCanonicalString( string );
}

/* ---- EmitSafeSetWaittillVariableField  0x00468D70 ---- VERIFIED */
static void EmitSafeSetWaittillVariableField( unsigned short string,
											  unsigned int sourcePos ) {
	EmitOpcode( OP_SafeSetWaittillField, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
	EmitCanonicalString( string );
}

/* ---- EmitGetString  0x00468DA0 ---- */
static void EmitGetString( unsigned short string ) {
	EmitOpcode( OP_GetString, 1, LOCALDEPTH_NONE );
	EmitString( string );
	CompileTransferRefToString( string, SCR_STRING_USAGE_CASE );
}

/* ---- EmitGetIString  0x00468E20 ---- */
static void EmitGetIString( unsigned short string ) {
	EmitOpcode( OP_GetIString, 1, LOCALDEPTH_NONE );
	EmitString( string );
	CompileTransferRefToString( string, SCR_STRING_USAGE_CASE );
}

/* ---- EmitCastBool  0x00468EA0 ---- VERIFIED */
static void EmitCastBool( unsigned int sourcePos ) {
	EmitOpcode( OP_CastBool, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitCastInt  0x00468EC0 ---- VERIFIED */
static void EmitCastInt( unsigned int sourcePos ) {
	EmitOpcode( OP_CastInt, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitCastFloat  0x00468EE0 ---- VERIFIED */
static void EmitCastFloat( unsigned int sourcePos ) {
	EmitOpcode( OP_CastFloat, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitCastString  0x00468F00 ---- VERIFIED */
static void EmitCastString( unsigned int sourcePos ) {
	EmitOpcode( OP_CastString, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitBoolNot  0x00468F20 ---- VERIFIED */
static void EmitBoolNot( unsigned int sourcePos ) {
	EmitOpcode( OP_BoolNot, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitBoolComplement  0x00468F40 ---- VERIFIED */
static void EmitBoolComplement( unsigned int sourcePos ) {
	EmitOpcode( OP_BoolComplement, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitSize  0x00468F60 ---- */
static void EmitSize( sval_t node, unsigned int sourcePos ) {
	EmitPrimitiveExpression( node );
	EmitOpcode( OP_Size, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitSelfObject  0x00468FD0 ---- VERIFIED */
static void EmitSelfObject( void ) {
	EmitOpcode( OP_GetSelfObject, 0, LOCALDEPTH_NONE );
}

/* ---- EmitLevelObject  0x00468FE0 ---- VERIFIED */
static void EmitLevelObject( void ) {
	EmitOpcode( OP_GetLevelObject, 0, LOCALDEPTH_NONE );
}

/* ---- EmitAnimObject  0x00468FF0 ---- VERIFIED */
static void EmitAnimObject( void ) {
	EmitOpcode( OP_GetAnimObject, 0, LOCALDEPTH_NONE );
}

/* ---- EmitLocalVariable  0x00469000 ---- */
static void EmitLocalVariable( unsigned short string ) {
	EmitOpcode( OP_EvalLocalVariable, 1, LOCALDEPTH_NONE );
	EmitCanonicalString( string );
}

/* ---- EmitLocalVariableRef  0x00469020 ---- */
static void EmitLocalVariableRef( unsigned short string ) {
	EmitOpcode( OP_EvalLocalVariableRef, 0, LOCALDEPTH_NONE );
	EmitCanonicalString( string );
}

/* ---- EmitGameRef  0x00469040 ---- */
static void EmitGameRef( void ) {
	EmitOpcode( OP_GetGameRef, 0, LOCALDEPTH_NONE );
}

/* ---- EmitClearLocalVariable  0x00469050 ---- */
static void EmitClearLocalVariable( unsigned short string ) {
	EmitOpcode( OP_ClearLocalVariable, 0, LOCALDEPTH_NONE );
	EmitCanonicalString( string );
}

/* ---- EmitEvalArray  0x00469070 ---- */
static void EmitEvalArray( unsigned int arraySourcePos, unsigned int indexSourcePos ) {
	EmitOpcode( OP_EvalArray, -1, LOCALDEPTH_NONE );
	AddOpcodePos( indexSourcePos );
	AddOpcodePos( arraySourcePos );
}

/* ---- EmitEvalArrayRef  0x004690A0 ---- VERIFIED */
static void EmitEvalArrayRef( qboolean bCreate, unsigned int arraySourcePos,
							  unsigned int indexSourcePos ) {
	EmitOpcode( (unsigned char)( bCreate ? OP_EvalArrayRef : OP_EvalArrayRefNoCreate ),
				-1, LOCALDEPTH_NONE );
	AddOpcodePos( indexSourcePos );
	AddOpcodePos( arraySourcePos );
}

/* ---- EmitClearArray  0x004690D0 ---- */
static void EmitClearArray( unsigned int arraySourcePos, unsigned int indexSourcePos ) {
	EmitOpcode( OP_ClearArray, -1, LOCALDEPTH_NONE );
	AddOpcodePos( indexSourcePos );
	AddOpcodePos( arraySourcePos );
}

/* ---- EmitAnimation  0x00469110 ---- */
static void EmitAnimation( unsigned short string, unsigned int sourcePos ) {
	char   *animRef;

	EmitOpcode( OP_GetAnimation, 1, LOCALDEPTH_NONE );
	EmitInteger( -1 );
	animRef = (char *)scrCompileGlob_codePos;
	AddOpcodePos( sourcePos );

	if ( !SCR_HANDLE( scrAnimPub_currentAnimTree ) ) {
		CompileError( sourcePos, "#using_animtree was not specified" );
	}
	Scr_EmitAnimationInternal( animRef, string,
							   SCR_HANDLE( scrAnimPub_currentAnimTree ), sourcePos );
	CompileRemoveRefToString( string );
}

/* ---- EmitFieldVariable  0x004691D0 ---- */
static void EmitFieldVariable( sval_t objectNode, unsigned short string,
							   unsigned int sourcePos ) {
	EmitPrimitiveExpressionFieldObject( objectNode, sourcePos );
	EmitOpcode( OP_EvalFieldVariable, 1, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
	EmitCanonicalString( string );
}

/* ---- EmitFieldVariableRef  0x00469210 ---- */
static void EmitFieldVariableRef( sval_t objectNode, unsigned short string,
								  unsigned int sourcePos ) {
	EmitPrimitiveExpressionFieldObject( objectNode, sourcePos );
	EmitOpcode( OP_EvalFieldVariableRef, 0, LOCALDEPTH_NONE );
	EmitCanonicalString( string );
}

/* ---- EmitClearFieldVariable  0x00469240 ---- */
static void EmitClearFieldVariable( sval_t objectNode, unsigned short string,
									unsigned int objectSourcePos,
									unsigned int opcodeSourcePos ) {
	EmitPrimitiveExpressionFieldObject( objectNode, objectSourcePos );
	EmitOpcode( OP_ClearFieldVariable, 0, LOCALDEPTH_NONE );
	AddOpcodePos( opcodeSourcePos );
	EmitCanonicalString( string );
}

/* ---- EmitCallRef  0x00469280 ---- */
static void EmitCallRef( sval_t nameNode, sval_list_t *args ) {
	EmitCall( nameNode, args, qfalse );
	EmitOpcode( OP_DiscardRef, -1, LOCALDEPTH_NONE );
}

/* ---- EmitMethodRef  0x004692C0 ---- */
static void EmitMethodRef( sval_t objectNode, sval_t nameNode, sval_list_t *args,
						   unsigned int methodSourcePos ) {
	EmitMethod( objectNode, nameNode, args, methodSourcePos, qfalse );
	EmitOpcode( OP_DiscardRef, -1, LOCALDEPTH_NONE );
}

/* ---- EmitDecTop  0x004692F0 ---- */
static void EmitDecTop( void ) {
	EmitOpcode( OP_DecTop, -1, LOCALDEPTH_NONE );
}

/* ---- EmitCastFieldObject  0x00469300 ---- */
static void EmitCastFieldObject( unsigned int sourcePos ) {
	EmitOpcode( OP_CastFieldObject, -1, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitArrayVariable  0x00469320 ---- */
static void EmitArrayVariable( sval_t objectNode, sval_t indexNode,
							   unsigned int objectSourcePos,
							   unsigned int indexSourcePos ) {
	EmitExpression( indexNode );
	EmitPrimitiveExpression( objectNode );
	EmitEvalArray( objectSourcePos, indexSourcePos );
}

/* ---- EmitArrayVariableRef  0x00469360 ---- */
static void EmitArrayVariableRef( sval_t arrayNode, sval_t indexNode,
								  unsigned int arraySourcePos,
								  unsigned int indexSourcePos, qboolean bCreate ) {
	EmitExpression( indexNode );
	EmitArrayExpressionRef( arrayNode, bCreate, arraySourcePos );
	EmitEvalArrayRef( bCreate, arraySourcePos, indexSourcePos );
}

/* ---- EmitClearArrayVariable  0x004693B0 ---- */
static void EmitClearArrayVariable( sval_t arrayNode, sval_t indexNode,
									unsigned int arraySourcePos,
									unsigned int indexSourcePos ) {
	EmitExpression( indexNode );
	EmitArrayExpressionRef( arrayNode, qfalse, arraySourcePos );
	EmitClearArray( arraySourcePos, indexSourcePos );
}

/* ---- EmitVariableExpression  0x004693F0 ---- */
static void EmitVariableExpression( sval_t node ) {
	sval_t *n = SVAL_NODE( node );

	switch ( n[0] ) {
	case SVAL_STRING_REF:
		EmitLocalVariable( (unsigned short)n[1] );
		break;
	case SVAL_ARRAY_REF:
		EmitArrayVariable( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_FIELD_REF:
		EmitFieldVariable( n[1], (unsigned short)n[2], n[3] );
		break;
	default:
		break;
	}
}

/* ---- EmitExpressionList  0x00469490 ---- */
static int EmitExpressionList( sval_list_t *list ) {
	sval_cell_t    *item;
	int             count = 0;

	for ( item = list->first; item; item = item->next ) {
		EmitExpression( SVAL_ENTRY( item->value )->node );
		count++;
	}
	return count;
}

/* ---- GetSingleParameter  0x004694C0 ---- */
static sval_cell_t *GetSingleParameter( sval_list_t *list ) {
	sval_cell_t    *item = list->first;

	if ( item && item->next ) {
		return NULL;
	}
	return item;
}

/* ---- AddExpressionListOpcodePos  0x004694E0 ---- */
static void AddExpressionListOpcodePos( sval_list_t *list ) {
	sval_cell_t    *item;

	if ( !scrCompilePub_scriptsAvailable ) {
		return;
	}
	for ( item = list->first; item; item = item->next ) {
		AddOpcodePos( SVAL_ENTRY( item->value )->sourcePos );
	}
}

/* ---- AddFilePrecache  0x00469510 ---- */
static unsigned short AddFilePrecache( unsigned short filename, unsigned int sourcePos ) {
	scrScriptLoad_t    *cursor;

	SL_AddRefToString( filename );

	cursor = (scrScriptLoad_t *)scrCompileGlob_loadCursor;
	cursor->filename  = filename;
	cursor->sourcePos = sourcePos;
	scrCompileGlob_loadCursor = (int)( cursor + 1 );

	return GetObject( GetVariable( SCR_HANDLE( scrCompilePub_scriptsPos ), filename ) );
}

/* ---- EmitFunction  0x00469560 ---- */
static void EmitFunction( sval_t node, unsigned int sourcePos ) {
	sval_t             *n = SVAL_NODE( node );
	unsigned short      functionObject;
	unsigned short      functionName;
	unsigned short      functionHandle;
	unsigned short      scriptFilename;
	unsigned short      scriptFunctionName;
	unsigned short      canonical;
	unsigned short      loadedScript;
	unsigned short      scriptRoot;
	unsigned short      countSlot;
	unsigned short      referenceSlot;
	VariableValue       functionValue;
	VariableValue       countValue;
	VariableValue       referenceValue;

	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_DISCARD ) {
		CompileRemoveRefToString( (unsigned short)n[1] );
		if ( n[0] == SVAL_SCRIPT_FUNCTION_REF ) {
			CompileRemoveRefToString( (unsigned short)n[2] );
			scrCompilePub_scriptCount--;
		}
		return;
	}

	functionObject = 0;

	if ( n[0] == SVAL_FUNCTION_REF ) {
		functionName   = (unsigned short)n[1];
		functionHandle = FindVariable( SCR_HANDLE( scrCompileGlob_currentFuncRoot ),
									   functionName );
		CompileTransferRefToString( functionName, SCR_STRING_USAGE_FUNCTION );
		if ( !functionHandle ) {
			CompileError( sourcePos, "unknown function" );
			return;
		}
		functionObject = FindObject( functionHandle );
	} else if ( n[0] == SVAL_SCRIPT_FUNCTION_REF ) {
		scriptFilename     = (unsigned short)n[1];
		scriptFunctionName = (unsigned short)n[2];

		canonical = Scr_CreateCanonicalFilename( SL_ConvertToString( scriptFilename ) );
		CompileRemoveRefToString( scriptFilename );

		loadedScript = FindVariable( SCR_HANDLE( scrCompilePub_loadedScripts ), canonical );
		scriptRoot   = AddFilePrecache( canonical, sourcePos );
		CompileRemoveRefToString( canonical );

		if ( loadedScript ) {
			functionHandle = FindVariable( scriptRoot, scriptFunctionName );
			CompileTransferRefToString( scriptFunctionName, SCR_STRING_USAGE_FUNCTION );
			if ( !functionHandle ) {
				CompileError( sourcePos, "unknown function" );
				return;
			}
		} else {
			functionHandle = GetVariable( scriptRoot, scriptFunctionName );
			CompileTransferRefToString( scriptFunctionName, SCR_STRING_USAGE_FUNCTION );
		}

		GetVariableValue( functionHandle, &functionValue );
		if ( functionValue.type == VAR_CODEPOS ) {
			EmitCodepos( functionValue.u.codePosValue );
			return;
		}
		if ( functionValue.type == VAR_INTEGER ) {
			if ( scrCompileGlob_codegenMode != SCR_CODEGEN_RELOCATE ) {
				CompileError( sourcePos,
							  "normal script cannot reference /# ... #/ comment" );
			}
			EmitCodepos( (const char *)functionValue.u.intValue );
			return;
		}
		functionObject = GetObject( functionHandle );
	}

	EmitBlankPos();
	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_DISCARD ) {
		return;
	}

	countSlot = GetVariable( functionObject, SCR_FUNC_REFCOUNT );
	GetVariableValue( countSlot, &countValue );
	if ( countValue.type == VAR_UNDEFINED ) {
		countValue.u.intValue = 0;
		countValue.type       = VAR_INTEGER;
	}

	referenceSlot = GetVariable( functionObject,
								 (unsigned int)countValue.u.intValue + SCR_FUNC_FIRST_REF );
	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_RELOCATE ) {
		referenceValue.u.intValue = scrCompileGlob_codePos
			+ ( scrCompilePub_developerBufPos - scrCompileGlob_codeRelocStart );
		referenceValue.type = VAR_INTEGER;
	} else {
		referenceValue.u.intValue = scrCompileGlob_codePos;
		referenceValue.type = VAR_CODEPOS;
	}
	SetNewVariableValue( referenceSlot, &referenceValue );

	countValue.u.intValue++;
	SetVariableValue( countSlot, &countValue );
	AddOpcodePos( sourcePos );
}

/* ---- EmitGetFunction  0x00469830 ---- */
static void EmitGetFunction( sval_t nameNode, unsigned int sourcePos ) {
	EmitOpcode( OP_GetFunction, 1, LOCALDEPTH_NONE );
	EmitFunction( nameNode, sourcePos );
}

/* ---- EmitPostScriptFunction  0x00469850 ---- */
static void EmitPostScriptFunction( sval_t nameNode, int argCount,
									qboolean hasMethodContext, unsigned int sourcePos ) {
	if ( hasMethodContext ) {
		EmitOpcode( OP_ScriptMethodCall, -2 - argCount, LOCALDEPTH_CALL );
	} else {
		EmitOpcode( OP_ScriptFunctionCall, -1 - argCount, LOCALDEPTH_CALL );
	}
	EmitFunction( nameNode, sourcePos );
	EmitInteger( argCount );
}

/* ---- EmitPostScriptFunctionPointer  0x004698B0 ---- */
static void EmitPostScriptFunctionPointer( sval_t objectNode, int argCount,
										   qboolean hasMethodContext,
										   unsigned int callSourcePos,
										   unsigned int objectSourcePos ) {
	EmitExpression( objectNode );
	if ( hasMethodContext ) {
		EmitOpcode( OP_ScriptMethodCallPtr, -3 - argCount, LOCALDEPTH_CALL );
	} else {
		EmitOpcode( OP_ScriptFunctionCallPtr, -2 - argCount, LOCALDEPTH_CALL );
	}
	AddOpcodePos( objectSourcePos );
	AddOpcodePos( callSourcePos );
	EmitInteger( argCount );
}

/* ---- EmitPostScriptThread  0x00469920 ---- */
static void EmitPostScriptThread( sval_t nameNode, int argCount,
								  qboolean hasMethodContext, unsigned int sourcePos ) {
	if ( hasMethodContext ) {
		EmitOpcode( OP_ScriptMethodThreadCall, -argCount, LOCALDEPTH_THREAD );
	} else {
		EmitOpcode( OP_ScriptThreadCall, 1 - argCount, LOCALDEPTH_THREAD );
	}
	EmitFunction( nameNode, sourcePos );
	EmitInteger( argCount );
}

/* ---- EmitPostScriptThreadPointer  0x00469980 ---- */
static void EmitPostScriptThreadPointer( sval_t objectNode, int argCount,
										 qboolean hasMethodContext,
										 unsigned int sourcePos ) {
	EmitExpression( objectNode );
	if ( hasMethodContext ) {
		EmitOpcode( OP_ScriptMethodThreadCallPtr, -1 - argCount, LOCALDEPTH_THREAD );
	} else {
		EmitOpcode( OP_ScriptThreadCallPtr, -argCount, LOCALDEPTH_THREAD );
	}
	AddOpcodePos( sourcePos );
	EmitInteger( argCount );
}

/* ---- EmitPostScriptFunctionCall  0x004699F0 ---- */
static void EmitPostScriptFunctionCall( sval_t calleeNode, int argCount,
										qboolean hasMethodContext,
										unsigned int callSourcePos ) {
	sval_t *callee = SVAL_NODE( calleeNode );

	if ( callee[0] == SVAL_SCRIPT_FUNCTION_NAME ) {
		EmitPostScriptFunction( callee[1], argCount, hasMethodContext, callSourcePos );
	} else if ( callee[0] == SVAL_FUNCTION_POINTER_CALL ) {
		EmitPostScriptFunctionPointer( callee[1], argCount, hasMethodContext,
									   callSourcePos, callee[2] );
	}
}

/* ---- EmitPostScriptThreadCall  0x00469A30 ---- */
static void EmitPostScriptThreadCall( sval_t calleeNode, int argCount,
									  qboolean hasMethodContext,
									  unsigned int callSourcePos,
									  unsigned int methodSourcePos ) {
	sval_t *callee = SVAL_NODE( calleeNode );

	if ( callee[0] == SVAL_SCRIPT_FUNCTION_NAME ) {
		EmitPostScriptThread( callee[1], argCount, hasMethodContext, methodSourcePos );
	} else if ( callee[0] == SVAL_FUNCTION_POINTER_CALL ) {
		EmitPostScriptThreadPointer( callee[1], argCount, hasMethodContext, callee[2] );
	}
	AddOpcodePos( callSourcePos );
}

/* ---- EmitPreFunctionCall  0x00469A90 ---- */
static void EmitPreFunctionCall( sval_t callNode ) {
	if ( SVAL_NODE( callNode )[0] == SVAL_FUNCTION_CALL ) {
		EmitOpcode( OP_PreScriptCall, 2, LOCALDEPTH_NONE );
	}
}

/* ---- EmitPostFunctionCall  0x00469AB0 ---- */
static void EmitPostFunctionCall( sval_t callNode, int argCount,
								  qboolean hasMethodContext ) {
	sval_t *n = SVAL_NODE( callNode );

	if ( n[0] == SVAL_FUNCTION_CALL ) {
		EmitPostScriptFunctionCall( n[1], argCount, hasMethodContext, n[2] );
	} else if ( n[0] == SVAL_METHOD_CALL ) {
		EmitPostScriptThreadCall( n[1], argCount, hasMethodContext, n[2], n[3] );
	}
}

/* ---- GetBuiltin  0x00469B20 ---- */
static unsigned short GetBuiltin( sval_t callNode ) {
	sval_t         *n = SVAL_NODE( callNode );
	sval_t         *callee;
	sval_t         *nameNode;
	unsigned short  name;

	if ( n[0] != SVAL_FUNCTION_CALL ) {
		return 0;
	}
	callee = SVAL_NODE( n[1] );
	if ( callee[0] != SVAL_SCRIPT_FUNCTION_NAME ) {
		return 0;
	}
	nameNode = SVAL_NODE( callee[1] );
	if ( nameNode[0] != SVAL_FUNCTION_REF ) {
		return 0;
	}

	name = (unsigned short)nameNode[1];
	if ( FindVariable( SCR_HANDLE( scrCompileGlob_currentFuncRoot ), name ) ) {
		return 0;
	}
	return name;
}

static qboolean Scr_BeginDeveloperCommand( unsigned int sourcePos, qboolean bDropTop ) {
	if ( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		return qfalse;
	}

	if ( !bDropTop ) {
		CompileError( sourcePos, "developer command can only be used as a statement "
								 "if not in a /# ... #/ comment" );
	}

	if ( scrCompilePub_developerScript ) {
		if ( !scrCompileGlob_bDeferredCheck ) {
			EmitOpcode( OP_DevBlockBegin, 0, LOCALDEPTH_NONE );
			scrCompileGlob_codeRelocStart = scrCompileGlob_codePos;
		}
		scrCompileGlob_codegenMode = SCR_CODEGEN_RELOCATE;
	} else {
		scrCompileGlob_codegenMode = SCR_CODEGEN_DISCARD;
	}
	return qtrue;
}

static void Scr_EndDeveloperCommand( qboolean active, void *tempMark,
									 int savedChecksum ) {
	if ( !active ) {
		return;
	}

	scrCompileGlob_codegenMode = SCR_CODEGEN_INTERN;
	if ( scrCompilePub_developerScript ) {
		scrCompileGlob_bDeferredCheck = 1;
	} else {
		TempMemorySetPos( tempMark );
	}
	scrCompilePub_checksum = savedChecksum;
}

/* ---- EmitCall  0x00469B70 ---- */
static void EmitCall( sval_t callNode, sval_list_t *args, qboolean bDropTop ) {
	unsigned short  builtinName;
	const char     *builtinText;
	void           *function;
	int             developerOnly;
	qboolean        developerActive;
	int             savedChecksum;
	unsigned int    callSourcePos;
	void           *tempMark;
	int             argCount;

	builtinName = GetBuiltin( callNode );
	if ( !builtinName ) {
		EmitPreFunctionCall( callNode );
		argCount = EmitExpressionList( args );
		EmitPostFunctionCall( callNode, argCount, qfalse );
		AddExpressionListOpcodePos( args );
		if ( bDropTop ) {
			EmitDecTop();
		}
		return;
	}

	builtinText   = SL_ConvertToString( builtinName );
	developerOnly = 0;
	function      = Scr_GetFunction( &builtinText, &developerOnly );

	callSourcePos   = SVAL_NODE( callNode )[2];
	savedChecksum   = scrCompilePub_checksum;
	tempMark        = TempMalloc( 0 );
	developerActive = qfalse;

	if ( developerOnly ) {
		developerActive = Scr_BeginDeveloperCommand( callSourcePos, bDropTop );
	}

	argCount = EmitExpressionList( args );
	if ( argCount >= SCR_BUILTIN_PARAM_LIMIT ) {
		CompileRemoveRefToString( builtinName );
		CompileError( callSourcePos, "parameter count exceeds 256" );
	}
	if ( !function ) {
		CompileError( callSourcePos, "unknown (builtin) function '%s'", builtinText );
	}
	CompileRemoveRefToString( builtinName );

	EmitOpcode( OP_CallBuiltin, 1 - argCount, LOCALDEPTH_BUILTIN );
	EmitByte( (unsigned char)argCount );
	EmitBuiltinFunction( function );
	AddOpcodePos( callSourcePos );

	AddExpressionListOpcodePos( args );
	if ( bDropTop ) {
		EmitDecTop();
	}

	Scr_EndDeveloperCommand( developerActive, tempMark, savedChecksum );
}

/* ---- EmitMethod  0x00469E10 ---- */
static void EmitMethod( sval_t objectNode, sval_t callNode, sval_list_t *args,
						unsigned int methodSourcePos, qboolean bDropTop ) {
	unsigned short  builtinName;
	const char     *builtinText;
	void           *method;
	int             developerOnly;
	qboolean        developerActive;
	int             savedChecksum;
	unsigned int    callSourcePos;
	void           *tempMark;
	int             argCount;

	builtinName = GetBuiltin( callNode );
	if ( !builtinName ) {
		EmitPreFunctionCall( callNode );
		argCount = EmitExpressionList( args );
		EmitPrimitiveExpression( objectNode );
		EmitPostFunctionCall( callNode, argCount, qtrue );
		AddOpcodePos( methodSourcePos );
		AddExpressionListOpcodePos( args );
		if ( bDropTop ) {
			EmitDecTop();
		}
		return;
	}

	builtinText   = SL_ConvertToString( builtinName );
	developerOnly = 0;
	method        = Scr_GetMethod( &builtinText, &developerOnly );

	callSourcePos   = SVAL_NODE( callNode )[2];
	savedChecksum   = scrCompilePub_checksum;
	tempMark        = TempMalloc( 0 );
	developerActive = qfalse;

	if ( developerOnly ) {
		developerActive = Scr_BeginDeveloperCommand( callSourcePos, bDropTop );
	}

	argCount = EmitExpressionList( args );
	EmitPrimitiveExpression( objectNode );
	if ( argCount >= SCR_BUILTIN_PARAM_LIMIT ) {
		CompileRemoveRefToString( builtinName );
		CompileError( callSourcePos, "parameter count exceeds 256" );
	}
	if ( !method ) {
		CompileError( callSourcePos, "unknown (builtin) method '%s'", builtinText );
	}
	CompileRemoveRefToString( builtinName );

	EmitOpcode( OP_CallBuiltinMethod, -argCount, LOCALDEPTH_BUILTIN );
	EmitByte( (unsigned char)argCount );
	EmitBuiltinMethod( method );
	AddOpcodePos( callSourcePos );

	AddOpcodePos( methodSourcePos );
	AddExpressionListOpcodePos( args );
	if ( bDropTop ) {
		EmitDecTop();
	}

	Scr_EndDeveloperCommand( developerActive, tempMark, savedChecksum );
}

/* ---- AdjustFunctionAddresses  0x0046A0E0 ---- */
static void AdjustFunctionAddresses( void ) {
	unsigned short      functionSlot;
	unsigned short      functionObject;
	unsigned short      definitionSlot;
	unsigned short      referenceCountSlot;
	unsigned short      referenceSlot;
	VariableValue       definitionValue;
	VariableValue       referenceCountValue;
	VariableValue      *referenceValue;
	int                 index;

	for ( functionSlot = FindNextSibling( SCR_HANDLE( scrCompileGlob_currentFuncRoot ) );
		  functionSlot;
		  functionSlot = FindNextSibling( functionSlot ) ) {

		functionObject = FindObject( functionSlot );

		definitionValue.u.intValue = 0;
		definitionValue.type       = VAR_UNDEFINED;

		definitionSlot = FindVariable( functionObject, SCR_FUNC_DEFINITION );
		if ( definitionSlot ) {
			GetVariableValue( definitionSlot, &definitionValue );
		}

		referenceCountSlot = FindVariable( functionObject, SCR_FUNC_REFCOUNT );
		if ( referenceCountSlot ) {
			GetVariableValue( referenceCountSlot, &referenceCountValue );

			for ( index = 0; index < referenceCountValue.u.intValue; index++ ) {
				referenceSlot  = FindVariable( functionObject,
											   (unsigned int)( index + SCR_FUNC_FIRST_REF ) );
				referenceValue = GetVariableValueAddress( referenceSlot );

				if ( definitionValue.type == VAR_INTEGER
				  && GetVarType( referenceSlot ) == VAR_CODEPOS ) {
					CompileError2( referenceValue->u.codePosValue,
								   "normal script cannot reference /# ... #/ comment" );
				} else if ( !definitionSlot ) {
					CompileError2( referenceValue->u.codePosValue, "unknown function" );
				} else {
					*(unsigned int *)referenceValue->u.intValue =
						(unsigned int)definitionValue.u.intValue;
				}
			}
		}

		if ( definitionSlot ) {
			SetVariableValue( functionSlot, &definitionValue );
		}
	}
}

/* ---- SpecifyThreadPosition  0x0046A290 ---- */
static void SpecifyThreadPosition( unsigned short functionObject,
								   unsigned int sourcePos ) {
	unsigned short  definitionSlot;
	VariableValue   definitionValue;

	definitionSlot = GetVariable( functionObject, SCR_FUNC_DEFINITION );
	GetVariableValue( definitionSlot, &definitionValue );

	if ( definitionValue.type != VAR_UNDEFINED ) {
		CompileError( sourcePos, "function already defined" );
		return;
	}

	definitionValue.type       = VAR_CODEPOS;
	definitionValue.u.intValue = 0;
	SetNewVariableValue( definitionSlot, &definitionValue );
}

/* ---- SpecifyDeveloperThreadPosition  0x0046A2F0 ---- */
static void SpecifyDeveloperThreadPosition( unsigned short functionObject,
											unsigned int sourcePos ) {
	unsigned short  definitionSlot;
	VariableValue   definitionValue;

	if ( !scrCompilePub_developerScript ) {
		return;
	}

	definitionSlot = GetVariable( functionObject, SCR_FUNC_DEFINITION );
	GetVariableValue( definitionSlot, &definitionValue );

	if ( definitionValue.type != VAR_UNDEFINED ) {
		CompileError( sourcePos, "function already defined" );
		return;
	}

	definitionValue.type       = VAR_INTEGER;
	definitionValue.u.intValue = 0;
	SetNewVariableValue( definitionSlot, &definitionValue );
}

/* ---- SetThreadPosition  0x0046A360 ---- */
static void SetThreadPosition( unsigned short functionObject ) {
	unsigned short  definitionSlot;

	definitionSlot = FindVariable( functionObject, SCR_FUNC_DEFINITION );
	GetVariableValueAddress( definitionSlot )->u.pointerValue = TempMalloc( 0 );
}

/* ---- SetDeveloperThreadPosition  0x0046A3B0 ---- */
static void SetDeveloperThreadPosition( unsigned short functionObject ) {
	unsigned short  definitionSlot;

	definitionSlot = FindVariable( functionObject, SCR_FUNC_DEFINITION );
	GetVariableValueAddress( definitionSlot )->u.intValue =
		(int)TempMalloc( 0 )
		+ ( scrCompilePub_developerBufPos - scrCompileGlob_codeRelocStart );
}

/* ---- EmitCallExpression  0x0046A410 ---- */
static void EmitCallExpression( sval_t node, qboolean bDropTop ) {
	sval_t *n = SVAL_NODE( node );

	if ( n[0] == SVAL_FUNCTION_CALL_VALUE ) {
		EmitCall( n[1], SVAL_LIST( n[2] ), bDropTop );
	} else if ( n[0] == SVAL_METHOD_CALL_VALUE ) {
		EmitMethod( n[1], n[2], SVAL_LIST( n[3] ), n[4], bDropTop );
	}
}

/* ---- EmitCallExpressionRef  0x0046A450 ---- */
static void EmitCallExpressionRef( sval_t node ) {
	sval_t *n = SVAL_NODE( node );

	if ( n[0] == SVAL_FUNCTION_CALL_VALUE ) {
		EmitCallRef( n[1], SVAL_LIST( n[2] ) );
	} else if ( n[0] == SVAL_METHOD_CALL_VALUE ) {
		EmitMethodRef( n[1], n[2], SVAL_LIST( n[3] ), n[4] );
	}
}

/* ---- EmitCallExpressionFieldObject  0x0046A4B0 ---- */
static void EmitCallExpressionFieldObject( sval_t node ) {
	sval_t *n = SVAL_NODE( node );

	if ( n[0] == SVAL_FUNCTION_CALL_VALUE ) {
		EmitCall( n[1], SVAL_LIST( n[2] ), qfalse );
		EmitCastFieldObject( n[3] );
	} else if ( n[0] == SVAL_METHOD_CALL_VALUE ) {
		EmitMethod( n[1], n[2], SVAL_LIST( n[3] ), n[4], qfalse );
		EmitCastFieldObject( n[5] );
	}
}

/* ---- EmitPrimitiveExpressionList  0x0046A520 ---- */
static void EmitPrimitiveExpressionList( sval_list_t *list, unsigned int sourcePos ) {
	int     count;

	count = EmitExpressionList( list );
	if ( count == 1 ) {
		return;
	}
	if ( count != 3 ) {
		CompileError( sourcePos, "expression list must have 1 or 3 parameters" );
	}
	EmitOpcode( OP_Vector, -2, LOCALDEPTH_NONE );
	AddExpressionListOpcodePos( list );
}

/* ---- EmitArrayExpressionListRef  0x0046A580 ---- */
static void EmitArrayExpressionListRef( sval_list_t *list, qboolean bCreate,
										unsigned int sourcePos ) {
	sval_cell_t    *item;
	sval_entry_t   *entry;

	item = GetSingleParameter( list );
	if ( item ) {
		entry = SVAL_ENTRY( item->value );
		if ( SVAL_NODE( entry->node )[0] != SVAL_PRIMITIVE_EXPRESSION ) {
			CompileError( entry->sourcePos, "not an array, string, or vector" );
		}
		EmitArrayExpressionRef( SVAL_NODE( entry->node )[1], bCreate,
								SVAL_NODE( entry->node )[2] );
		return;
	}

	if ( EmitExpressionList( list ) != 3 ) {
		CompileError( sourcePos, "not an array, string, or vector" );
	}
	EmitOpcode( OP_Vector, -2, LOCALDEPTH_NONE );
	AddExpressionListOpcodePos( list );
	EmitOpcode( OP_DiscardRef, -1, LOCALDEPTH_NONE );
}

/* ---- EmitExpressionListFieldObject  0x0046A620 ---- */
static void EmitExpressionListFieldObject( sval_list_t *list, unsigned int sourcePos ) {
	sval_cell_t    *item;
	sval_entry_t   *entry;

	item = GetSingleParameter( list );
	if ( item ) {
		entry = SVAL_ENTRY( item->value );
		EmitExpressionFieldObject( entry->node, entry->sourcePos );
		return;
	}

	CompileError( sourcePos, "not an object" );
	EmitExpressionList( list );
}

/* ---- EmitPrimitiveExpression  0x0046A680 ---- */
static void EmitPrimitiveExpression( sval_t node ) {
	sval_t *n = SVAL_NODE( node );
	float   f;

	switch ( n[0] ) {
	case SVAL_INTEGER_LITERAL:
		EmitGetInteger( (int)n[1] );
		break;
	case SVAL_FLOAT_LITERAL:
		EmitGetFloat( *(float *)&n[1] );
		break;
	case SVAL_NEG_INTEGER_LITERAL:
		EmitGetInteger( -(int)n[1] );
		break;
	case SVAL_NEG_FLOAT_LITERAL:
		f = -*(float *)&n[1];
		EmitGetFloat( f );
		break;
	case SVAL_STRING:
		EmitGetString( (unsigned short)n[1] );
		break;
	case SVAL_ISTRING:
		EmitGetIString( (unsigned short)n[1] );
		break;
	case SVAL_REFERENCE_EXPRESSION:
		EmitVariableExpression( n[1] );
		break;
	case SVAL_SCRIPT_FUNCTION_NAME:
		EmitGetFunction( n[1], n[2] );
		break;
	case SVAL_CALL_VALUE:
		EmitCallExpression( n[1], qfalse );
		break;
	case SVAL_UNDEFINED:
		EmitIncTop();
		break;
	case SVAL_SELF:
		EmitSelf();
		break;
	case SVAL_LEVEL:
		EmitLevel();
		break;
	case SVAL_GAME:
		EmitGame();
		break;
	case SVAL_ANIM:
		EmitAnim();
		break;
	case SVAL_EXPRESSION_LIST:
		EmitPrimitiveExpressionList( SVAL_LIST( n[1] ), n[2] );
		break;
	case SVAL_SIZE:
		EmitSize( n[1], n[2] );
		break;
	case SVAL_EMPTY_ARRAY:
		EmitEmptyArray();
		break;
	case SVAL_ANIM_STRING:
		EmitAnimation( (unsigned short)n[1], n[2] );
		break;
	case SVAL_LITERAL_ZERO:
		EmitFalse();
		break;
	case SVAL_LITERAL_ONE:
		EmitTrue();
		break;
	case SVAL_ANIMTREE:
		EmitAnimTree( n[1] );
		break;
	default:
		break;
	}
}

/* ---- EmitBoolOrExpression  0x0046A830 ---- VERIFIED */
static void EmitBoolOrExpression( sval_t leftNode, unsigned int leftSourcePos,
								  sval_t rightNode, unsigned int rightSourcePos ) {
	char   *patch;
	char   *end;

	EmitExpression( leftNode );
	EmitOpcode( OP_JumpOnTrueExpr, -1, LOCALDEPTH_NONE );
	AddOpcodePos( leftSourcePos );
	EmitInteger( 0 );
	patch = (char *)scrCompileGlob_codePos;

	EmitExpression( rightNode );
	EmitCastBool( rightSourcePos );

	end = (char *)TempMalloc( 0 );
	*(int *)patch = (int)( end - patch ) - 4;
}

/* ---- EmitBoolAndExpression  0x0046A8D0 ---- VERIFIED */
static void EmitBoolAndExpression( sval_t leftNode, unsigned int leftSourcePos,
								   sval_t rightNode, unsigned int rightSourcePos ) {
	char   *patch;
	char   *end;

	EmitExpression( leftNode );
	EmitOpcode( OP_JumpOnFalseExpr, -1, LOCALDEPTH_NONE );
	AddOpcodePos( leftSourcePos );
	EmitInteger( 0 );
	patch = (char *)scrCompileGlob_codePos;

	EmitExpression( rightNode );
	EmitCastBool( rightSourcePos );

	end = (char *)TempMalloc( 0 );
	*(int *)patch = (int)( end - patch ) - 4;
}

/* ---- EmitBinaryOperatorExpression  0x0046A970 ---- */
static void EmitBinaryOperatorExpression( sval_t leftNode, sval_t rightNode,
										  unsigned char opcode,
										  unsigned int sourcePos ) {
	EmitExpression( leftNode );
	EmitExpression( rightNode );
	EmitOpcode( opcode, -1, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- EmitBinaryEqualsOperatorExpression  0x0046A9A0 ---- */
static void EmitBinaryEqualsOperatorExpression( sval_t refNode, sval_t valueNode,
												unsigned char opcode,
												unsigned int sourcePos ) {
	scrCompileGlob_bOwnsStrings = 1;
	EmitVariableExpression( refNode );
	scrCompileGlob_bOwnsStrings = 0;

	EmitExpression( valueNode );
	EmitOpcode( opcode, -1, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );

	EmitVariableExpressionRef( refNode, qtrue );
	EmitSetVariableField( sourcePos );
}

/* ---- EmitExpression  0x0046AA00 ---- */
static void EmitExpression( sval_t node ) {
	sval_t *n = SVAL_NODE( node );

	switch ( n[0] ) {
	case SVAL_PRIMITIVE_EXPRESSION:
		EmitPrimitiveExpression( n[1] );
		break;
	case SVAL_BOOL_OR:
		EmitBoolOrExpression( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_BOOL_AND:
		EmitBoolAndExpression( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_BINARY_OPERATOR:
		EmitBinaryOperatorExpression( n[1], n[2], (unsigned char)n[3], n[4] );
		break;
	case SVAL_CAST_BOOL:
		EmitExpression( n[1] );
		EmitCastBool( n[2] );
		break;
	case SVAL_CAST_INT:
		EmitExpression( n[1] );
		EmitCastInt( n[2] );
		break;
	case SVAL_CAST_FLOAT:
		EmitExpression( n[1] );
		EmitCastFloat( n[2] );
		break;
	case SVAL_CAST_STRING:
		EmitExpression( n[1] );
		EmitCastString( n[2] );
		break;
	case SVAL_BOOL_NOT:
		EmitExpression( n[1] );
		EmitBoolNot( n[2] );
		break;
	case SVAL_BOOL_COMPLEMENT:
		EmitExpression( n[1] );
		EmitBoolComplement( n[2] );
		break;
	default:
		break;
	}
}

/* ---- EmitVariableExpressionRef  0x0046ABB0 ---- */
static void EmitVariableExpressionRef( sval_t node, qboolean bCreate ) {
	sval_t *n = SVAL_NODE( node );

	switch ( n[0] ) {
	case SVAL_STRING_REF:
		EmitLocalVariableRef( (unsigned short)n[1] );
		break;
	case SVAL_ARRAY_REF:
		EmitArrayVariableRef( n[1], n[2], n[3], n[4], bCreate );
		break;
	case SVAL_FIELD_REF:
		EmitFieldVariableRef( n[1], (unsigned short)n[2], n[3] );
		break;
	default:
		break;
	}
}

/* ---- EmitArrayExpressionRef  0x0046AC20 ---- */
static void EmitArrayExpressionRef( sval_t node, qboolean bCreate,
									unsigned int sourcePos ) {
	sval_t *n = SVAL_NODE( node );

	switch ( n[0] ) {
	case SVAL_REFERENCE_EXPRESSION:
		EmitVariableExpressionRef( n[1], bCreate );
		break;
	case SVAL_CALL_VALUE:
		EmitCallExpressionRef( n[1] );
		break;
	case SVAL_GAME:
		EmitGameRef();
		break;
	case SVAL_EXPRESSION_LIST:
		EmitArrayExpressionListRef( SVAL_LIST( n[1] ), bCreate, sourcePos );
		break;
	default:
		CompileError( sourcePos, "not an array, string, or vector" );
		break;
	}
}

/* ---- EmitArrayPrimitiveExpressionRef  0x0046ACF0 ---- */
static void EmitArrayPrimitiveExpressionRef( sval_t node, qboolean bCreate,
											 unsigned int sourcePos ) {
	sval_t *n = SVAL_NODE( node );

	if ( n[0] != SVAL_PRIMITIVE_EXPRESSION ) {
		CompileError( sourcePos, "not an array, string, or vector" );
		EmitExpression( node );
		return;
	}
	EmitArrayExpressionRef( n[1], bCreate, n[2] );
}

/* ---- EmitPrimitiveExpressionFieldObject  0x0046AD30 ---- */
static void EmitPrimitiveExpressionFieldObject( sval_t node, unsigned int sourcePos ) {
	sval_t *n = SVAL_NODE( node );

	switch ( n[0] ) {
	case SVAL_REFERENCE_EXPRESSION:
		EmitVariableExpression( n[1] );
		EmitCastFieldObject( n[2] );
		return;
	case SVAL_CALL_VALUE:
		EmitCallExpressionFieldObject( n[1] );
		return;
	case SVAL_SELF:
		EmitSelfObject();
		return;
	case SVAL_LEVEL:
		EmitLevelObject();
		return;
	case SVAL_ANIM:
		EmitAnimObject();
		return;
	case SVAL_EXPRESSION_LIST:
		EmitExpressionListFieldObject( SVAL_LIST( n[1] ), sourcePos );
		return;
	default:
		break;
	}

	CompileError( sourcePos, "not an object" );
	EmitPrimitiveExpression( node );
}

/* ---- EmitExpressionFieldObject  0x0046AE20 ---- */
static void EmitExpressionFieldObject( sval_t node, unsigned int sourcePos ) {
	sval_t *n = SVAL_NODE( node );

	if ( n[0] == SVAL_PRIMITIVE_EXPRESSION ) {
		EmitPrimitiveExpressionFieldObject( n[1], n[2] );
		return;
	}

	CompileError( sourcePos, "not an object" );
	EmitExpression( node );
}

/* ---- ConnectBreakStatements  0x0046AE60 ---- */
static void ConnectBreakStatements( void ) {
	char                   *here;
	scrCodeOffsetPatch_t   *patch;

	here = (char *)TempMalloc( 0 );
	for ( patch = (scrCodeOffsetPatch_t *)scrCompileGlob_breakList;
		  patch; patch = patch->next ) {
		*(int *)patch->codePos = (int)( here - patch->codePos );
	}
}

/* ---- ConnectContinueStatements  0x0046AEA0 ---- */
static void ConnectContinueStatements( void ) {
	char                   *here;
	scrCodeOffsetPatch_t   *patch;

	EmitNOP();
	here = (char *)TempMalloc( 0 );
	for ( patch = (scrCodeOffsetPatch_t *)scrCompileGlob_contList;
		  patch; patch = patch->next ) {
		*(int *)patch->codePos = (int)( here - patch->codePos );
	}
}

/* ---- IsUndefinedPrimitiveExpression  0x0046AEF0 ---- */
static qboolean IsUndefinedPrimitiveExpression( sval_t node ) {
	return SVAL_NODE( node )[0] == SVAL_UNDEFINED ? qtrue : qfalse;
}

/* ---- IsUndefinedExpression  0x0046AF00 ---- */
static qboolean IsUndefinedExpression( sval_t node ) {
	sval_t *n = SVAL_NODE( node );

	if ( n[0] != SVAL_PRIMITIVE_EXPRESSION ) {
		return qfalse;
	}
	return IsUndefinedPrimitiveExpression( n[1] );
}

/* ---- EmitClearVariableExpression  0x0046AF20 ---- */
static void EmitClearVariableExpression( sval_t node ) {
	sval_t *n = SVAL_NODE( node );

	switch ( n[0] ) {
	case SVAL_STRING_REF:
		EmitClearLocalVariable( (unsigned short)n[1] );
		break;
	case SVAL_ARRAY_REF:
		EmitClearArrayVariable( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_FIELD_REF:
		EmitClearFieldVariable( n[1], (unsigned short)n[2], n[3], n[4] );
		break;
	default:
		break;
	}
}

/* ---- EmitAssignmentStatement  0x0046AFA0 ---- */
static void EmitAssignmentStatement( sval_t refNode, sval_t valueNode,
									 unsigned int sourcePos ) {
	if ( IsUndefinedExpression( valueNode ) ) {
		EmitClearVariableExpression( refNode );
		return;
	}

	EmitExpression( valueNode );
	EmitVariableExpressionRef( refNode, qtrue );
	EmitSetVariableField( sourcePos );
}

/* ---- EmitExpressionStatement  0x0046AFF0 ---- */
static void EmitExpressionStatement( sval_t node ) {
	EmitCallExpression( node, qtrue );
}

/* ---- EmitReturnStatement  0x0046B030 ---- */
static void EmitReturnStatement( sval_t node ) {
	EmitExpression( node );
	EmitOpcode( OP_Return, -1, LOCALDEPTH_NONE );
}

/* ---- EmitEndStatement  0x0046B050 ---- */
static void EmitEndStatement( void ) {
	EmitOpcode( OP_End, 0, LOCALDEPTH_NONE );
}

/* ---- EmitWaitStatement  0x0046B060 ---- */
static void EmitWaitStatement( sval_t timeNode, unsigned int timeSourcePos,
							   unsigned int opcodeSourcePos ) {
	if ( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( opcodeSourcePos, "wait not allowed in /# ... #/ comment" );
	}

	EmitExpression( timeNode );
	EmitOpcode( OP_Wait, -1, LOCALDEPTH_NONE );
	AddOpcodePos( opcodeSourcePos );
	AddOpcodePos( timeSourcePos );
}

static char *EmitForwardJump( unsigned char opcode, int stackDelta,
							  unsigned int sourcePos ) {
	EmitOpcode( opcode, stackDelta, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
	EmitInteger( 0 );
	return (char *)scrCompileGlob_codePos;
}

static void PatchForwardJump( char *patch ) {
	char   *here = (char *)TempMalloc( 0 );

	*(int *)patch = (int)( here - patch ) - 4;
}

/* ---- EmitIfStatement  0x0046B0B0 ---- */
static void EmitIfStatement( sval_t conditionNode, sval_t bodyNode,
							 unsigned int sourcePos ) {
	char   *falsePatch;

	EmitExpression( conditionNode );
	falsePatch = EmitForwardJump( OP_JumpOnFalse, -1, sourcePos );

	EmitStatement( bodyNode );
	EmitNOP();
	PatchForwardJump( falsePatch );
}

/* ---- EmitIfElseStatement  0x0046B140 ---- */
static void EmitIfElseStatement( sval_t conditionNode, sval_t thenNode,
								 sval_t elseNode, unsigned int sourcePos ) {
	char   *falsePatch;
	char   *endPatch;
	char   *here;

	EmitExpression( conditionNode );
	falsePatch = EmitForwardJump( OP_JumpOnFalse, -1, sourcePos );

	EmitStatement( thenNode );
	EmitOpcode( OP_Jump, 0, LOCALDEPTH_NONE );
	EmitInteger( 0 );
	endPatch = (char *)scrCompileGlob_codePos;

	PatchForwardJump( falsePatch );

	EmitStatement( elseNode );
	EmitNOP();

	here = (char *)TempMalloc( 0 );
	*(int *)endPatch = (int)( here - endPatch );
}

typedef struct scrLoopState_s {
	unsigned char           breakAllowed;
	unsigned char           breakAllowedDev;
	scrCodeOffsetPatch_t   *breakList;
	unsigned char           contAllowed;
	unsigned char           contAllowedDev;
	scrCodeOffsetPatch_t   *contList;
} scrLoopState_t;

static void Scr_SaveLoopState( scrLoopState_t *state ) {
	state->breakAllowed    = scrCompileGlob_breakAllowed;
	state->breakAllowedDev = scrCompileGlob_breakAllowedDev;
	state->breakList       = (scrCodeOffsetPatch_t *)scrCompileGlob_breakList;
	state->contAllowed     = scrCompileGlob_contAllowed;
	state->contAllowedDev  = scrCompileGlob_contAllowedDev;
	state->contList        = (scrCodeOffsetPatch_t *)scrCompileGlob_contList;
}

static void Scr_ClearLoopState( void ) {
	scrCompileGlob_breakAllowed    = 0;
	scrCompileGlob_breakAllowedDev = 0;
	scrCompileGlob_contAllowed     = 0;
	scrCompileGlob_contAllowedDev  = 0;
}

static void Scr_EnableLoopState( void ) {
	unsigned char   inDeveloper =
		(unsigned char)( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN );

	scrCompileGlob_breakAllowed    = 1;
	scrCompileGlob_breakAllowedDev = inDeveloper;
	scrCompileGlob_breakList       = 0;
	scrCompileGlob_contAllowed     = 1;
	scrCompileGlob_contAllowedDev  = inDeveloper;
	scrCompileGlob_contList        = 0;
}

static void Scr_RestoreLoopState( const scrLoopState_t *state ) {
	scrCompileGlob_breakAllowed    = state->breakAllowed;
	scrCompileGlob_breakAllowedDev = state->breakAllowedDev;
	scrCompileGlob_breakList       = (int)state->breakList;
	scrCompileGlob_contAllowed     = state->contAllowed;
	scrCompileGlob_contAllowedDev  = state->contAllowedDev;
	scrCompileGlob_contList        = (int)state->contList;
}

/* ---- EmitWhileStatement  0x0046B220 ---- */
static void EmitWhileStatement( sval_t conditionNode, sval_t bodyNode,
								unsigned int conditionSourcePos,
								unsigned int loopSourcePos ) {
	scrLoopState_t  state;
	char           *loopStart;
	char           *falsePatch;
	char           *jumpPos;

	EmitNOP();
	Scr_SaveLoopState( &state );
	Scr_ClearLoopState();

	loopStart = (char *)TempMalloc( 0 );
	EmitExpression( conditionNode );
	falsePatch = EmitForwardJump( OP_JumpOnFalse, -1, conditionSourcePos );

	Scr_EnableLoopState();
	EmitStatement( bodyNode );
	Scr_ClearLoopState();
	ConnectContinueStatements();

	EmitOpcode( OP_JumpBack, 0, LOCALDEPTH_NONE );
	AddOpcodePos( loopSourcePos );
	jumpPos = (char *)TempMalloc( 0 );
	EmitInteger( (int)( loopStart - jumpPos ) );

	PatchForwardJump( falsePatch );
	ConnectBreakStatements();
	Scr_RestoreLoopState( &state );
}

/* ---- EmitDoWhileStatement  0x0046B3F0 ---- */
static void EmitDoWhileStatement( sval_t bodyNode, sval_t conditionNode,
								  unsigned int conditionSourcePos,
								  unsigned int loopSourcePos ) {
	scrLoopState_t  state;
	char           *loopStart;
	char           *jumpPos;

	EmitNOP();
	Scr_SaveLoopState( &state );
	Scr_EnableLoopState();

	loopStart = (char *)TempMalloc( 0 );
	EmitStatement( bodyNode );
	Scr_ClearLoopState();
	ConnectContinueStatements();

	EmitExpression( conditionNode );
	EmitOpcode( OP_JumpOnTrueBack, -1, LOCALDEPTH_NONE );
	AddOpcodePos( loopSourcePos );
	AddOpcodePos( conditionSourcePos );
	jumpPos = (char *)TempMalloc( 0 );
	EmitInteger( (int)( loopStart - jumpPos ) );

	ConnectBreakStatements();
	Scr_RestoreLoopState( &state );
}

/* ---- EmitForStatement  0x0046B560 ---- */
static void EmitForStatement( sval_t initNode, sval_t conditionNode,
							  sval_t incrementNode, sval_t bodyNode,
							  unsigned int conditionSourcePos,
							  unsigned int loopSourcePos ) {
	scrLoopState_t  state;
	char           *loopStart;
	char           *falsePatch;
	char           *jumpPos;

	Scr_SaveLoopState( &state );
	Scr_ClearLoopState();

	EmitStatement( initNode );
	EmitNOP();
	loopStart = (char *)TempMalloc( 0 );

	falsePatch = NULL;
	if ( SVAL_NODE( conditionNode )[0] == SVAL_FOR_CONDITION ) {
		EmitExpression( SVAL_NODE( conditionNode )[1] );
		falsePatch = EmitForwardJump( OP_JumpOnFalse, -1, conditionSourcePos );
	}

	Scr_EnableLoopState();
	EmitStatement( bodyNode );
	Scr_ClearLoopState();
	ConnectContinueStatements();
	EmitStatement( incrementNode );

	EmitOpcode( OP_JumpBack, 0, LOCALDEPTH_NONE );
	AddOpcodePos( loopSourcePos );
	jumpPos = (char *)TempMalloc( 0 );
	EmitInteger( (int)( loopStart - jumpPos ) );

	if ( falsePatch ) {
		PatchForwardJump( falsePatch );
	}

	ConnectBreakStatements();
	Scr_RestoreLoopState( &state );
}

/* ---- EmitIncStatement  0x0046B760 ---- */
static void EmitIncStatement( sval_t refNode, unsigned int sourcePos ) {
	EmitVariableExpressionRef( refNode, qfalse );
	EmitOpcode( OP_Inc, 1, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
	EmitSetVariableField( sourcePos );
}

/* ---- EmitDecStatement  0x0046B7A0 ---- */
static void EmitDecStatement( sval_t refNode, unsigned int sourcePos ) {
	EmitVariableExpressionRef( refNode, qfalse );
	EmitOpcode( OP_Dec, 1, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
	EmitSetVariableField( sourcePos );
}

/* ---- EmitFormalParameterListRefInternal  0x0046B7E0 ---- */
static void EmitFormalParameterListRefInternal( sval_cell_t *item ) {
	for ( item = item->next; item; item = item->next ) {
		EmitSafeSetVariableField( (unsigned short)SVAL_ENTRY( item->value )->node,
								  SVAL_ENTRY( item->value )->sourcePos );
	}
}

/* ---- EmitFormalWaittillParameterListRefInternal  0x0046B8C0 ---- VERIFIED */
static void EmitFormalWaittillParameterListRefInternal( sval_cell_t *item ) {
	for ( item = item->next; item; item = item->next ) {
		EmitSafeSetWaittillVariableField(
			(unsigned short)SVAL_ENTRY( item->value )->node,
			SVAL_ENTRY( item->value )->sourcePos );
	}
}

/* ---- EmitWaittillStatement  0x0046B9A0 ---- VERIFIED */
static void EmitWaittillStatement( sval_t objectNode, sval_t listNode,
								   unsigned int objectSourcePos,
								   unsigned int opcodeSourcePos ) {
	sval_cell_t    *eventItem;

	if ( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( opcodeSourcePos, "waittill not allowed in developer script" );
	}

	eventItem = SVAL_LIST( listNode )->first->next;
	EmitExpression( SVAL_ENTRY( eventItem->value )->node );
	EmitPrimitiveExpression( objectNode );
	EmitOpcode( OP_WaitTill, -2, LOCALDEPTH_NONE );
	AddOpcodePos( opcodeSourcePos );
	AddOpcodePos( SVAL_ENTRY( eventItem->value )->sourcePos );
	AddOpcodePos( objectSourcePos );
	EmitFormalWaittillParameterListRefInternal( eventItem );
	EmitOpcode( OP_EndWaittill, 0, LOCALDEPTH_NONE );
}

/* ---- EmitWaittillmatchStatement  0x0046BA20 ---- */
static void EmitWaittillmatchStatement( sval_t objectNode, sval_t listNode,
										unsigned int objectSourcePos,
										unsigned int opcodeSourcePos ) {
	sval_cell_t    *eventItem;
	sval_cell_t    *item;
	int             matchCount = 0;

	if ( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( opcodeSourcePos, "waittillmatch not allowed in developer script" );
	}

	eventItem = SVAL_LIST( listNode )->first->next;

	for ( item = eventItem->next; item; item = item->next ) {
		EmitExpression( SVAL_ENTRY( item->value )->node );
		matchCount++;
	}

	EmitExpression( SVAL_ENTRY( eventItem->value )->node );
	EmitPrimitiveExpression( objectNode );
	EmitOpcode( OP_WaitTillMatch, -2 - matchCount, LOCALDEPTH_NONE );
	AddOpcodePos( opcodeSourcePos );
	AddOpcodePos( SVAL_ENTRY( eventItem->value )->sourcePos );
	AddOpcodePos( objectSourcePos );

	for ( item = eventItem->next; item; item = item->next ) {
		AddOpcodePos( SVAL_ENTRY( item->value )->sourcePos );
	}

	EmitByte( (unsigned char)matchCount );
	EmitOpcode( OP_EndWaittill, 0, LOCALDEPTH_NONE );
}

/* ---- EmitNotifyStatement  0x0046BB00 ---- */
static void EmitNotifyStatement( sval_t objectNode, sval_list_t *list,
								 unsigned int objectSourcePos,
								 unsigned int opcodeSourcePos ) {
	sval_cell_t    *item;
	sval_cell_t    *lastItem = NULL;
	int             argCount = 0;

	EmitOpcode( OP_VoidCodepos, 1, LOCALDEPTH_NONE );

	for ( item = list->first; item; item = item->next ) {
		lastItem = item;
		EmitExpression( SVAL_ENTRY( item->value )->node );
		argCount++;
	}

	EmitPrimitiveExpression( objectNode );
	EmitOpcode( OP_Notify, -2 - argCount, LOCALDEPTH_NONE );
	AddOpcodePos( opcodeSourcePos );
	AddOpcodePos( SVAL_ENTRY( lastItem->value )->sourcePos );
	AddOpcodePos( objectSourcePos );
}

/* ---- EmitEndOnStatement  0x0046BC20 ---- */
static void EmitEndOnStatement( sval_t objectNode, sval_t eventNode,
								unsigned int objectSourcePos,
								unsigned int opcodeSourcePos ) {
	EmitExpression( eventNode );
	EmitPrimitiveExpression( objectNode );
	EmitOpcode( OP_EndOn, -2, LOCALDEPTH_NONE );
	AddOpcodePos( opcodeSourcePos );
	AddOpcodePos( objectSourcePos );
}

/* ---- CompareCaseInfo  0x0046BC60 ---- */
static int __cdecl CompareCaseInfo( const void *a, const void *b ) {
	const scrCaseTableEntry_t  *left  = (const scrCaseTableEntry_t *)a;
	const scrCaseTableEntry_t  *right = (const scrCaseTableEntry_t *)b;

	if ( right->value < left->value ) {
		return -1;
	}
	return left->value < right->value;
}

/* ---- EmitCaseStatementInfo  0x0046BF50 ---- */
static void EmitCaseStatementInfo( unsigned int value, unsigned int sourcePos ) {
	scrCaseRecord_t    *record;

	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_DISCARD ) {
		return;
	}

	record = (scrCaseRecord_t *)
		Hunk_AllocateTempMemoryHighInternal( (int)sizeof( scrCaseRecord_t ) );
	record->value   = value;
	record->codePos = (unsigned int)TempMalloc( 0 );
	if ( scrCompileGlob_codegenMode == SCR_CODEGEN_RELOCATE ) {
		record->codePos += ( (int)scrCompilePub_developerBufPos - scrCompileGlob_codeRelocStart );
	}
	record->sourcePos = sourcePos;
	record->next      = (scrCaseRecord_t *)scrCompileGlob_caseList;
	scrCompileGlob_caseList = (int)record;
}

/* ---- EmitCaseStatement  0x0046BFD0 ---- */
static void EmitCaseStatement( sval_t valueNode, unsigned int sourcePos ) {
	sval_t         *n = SVAL_NODE( valueNode );
	unsigned int    caseValue;
	int             integerValue;

	if ( n[0] == SVAL_INTEGER_LITERAL ) {
		integerValue = (int)n[1];
		if ( (unsigned int)( integerValue + SCR_CASE_TEST_BIAS ) >= SCR_CASE_TEST_LIMIT ) {
			CompileError( sourcePos, va( "case index %d out of range", integerValue ) );
			return;
		}
		caseValue = ( (unsigned int)integerValue + SCR_CASE_ENCODE_BIAS )
				  & SCR_CASE_ENCODE_MASK;
	} else {
		if ( n[0] != SVAL_STRING ) {
			CompileError( sourcePos, "case expression must be an int or string" );
			return;
		}
		caseValue = (unsigned short)n[1];
		CompileTransferRefToString( (unsigned short)caseValue, SCR_STRING_USAGE_CASE );
	}

	if ( !scrCompileGlob_caseAllowed ) {
		CompileError( sourcePos, "illegal case statement" );
	} else if ( !scrCompileGlob_caseAllowedDev
			 && scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( sourcePos,
					  "cannot use /# ... #/ comments directly around a case statement" );
	}

	EmitCaseStatementInfo( caseValue, sourcePos );
}

static void EmitDefaultStatement( unsigned int sourcePos ) {
	if ( !scrCompileGlob_caseAllowed ) {
		CompileError( sourcePos, "illegal default statement" );
	}
	EmitCaseStatementInfo( 0, sourcePos );
}

typedef struct scrSwitchState_s {
	unsigned char           caseAllowed;
	unsigned char           caseAllowedDev;
	scrCaseRecord_t        *caseList;
	unsigned char           breakAllowed;
	unsigned char           breakAllowedDev;
	scrCodeOffsetPatch_t   *breakList;
} scrSwitchState_t;

/* ---- EmitSwitchStatement  0x0046BC80 ---- */
static void EmitSwitchStatement( sval_t valueNode, sval_t bodyNode,
								 unsigned int sourcePos ) {
	scrSwitchState_t        state;
	char                   *tablePatch;
	scrCaseTableEntry_t    *table;
	scrCaseRecord_t        *record;
	unsigned int            caseCount = 0;
	unsigned int            index;

	state.caseAllowed     = scrCompileGlob_caseAllowed;
	state.caseAllowedDev  = scrCompileGlob_caseAllowedDev;
	state.caseList        = (scrCaseRecord_t *)scrCompileGlob_caseList;
	state.breakAllowed    = scrCompileGlob_breakAllowed;
	state.breakAllowedDev = scrCompileGlob_breakAllowedDev;
	state.breakList       = (scrCodeOffsetPatch_t *)scrCompileGlob_breakList;

	scrCompileGlob_caseAllowed     = 0;
	scrCompileGlob_caseAllowedDev  = 0;
	scrCompileGlob_breakAllowed    = 0;
	scrCompileGlob_breakAllowedDev = 0;

	EmitExpression( valueNode );
	EmitOpcode( OP_Switch, -1, LOCALDEPTH_NONE );
	EmitInteger( 0 );
	tablePatch = (char *)scrCompileGlob_codePos;

	scrCompileGlob_caseAllowed     = 1;
	scrCompileGlob_caseAllowedDev  =
		(unsigned char)( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN );
	scrCompileGlob_caseList        = 0;
	scrCompileGlob_breakAllowed    = 1;
	scrCompileGlob_breakAllowedDev = scrCompileGlob_caseAllowedDev;
	scrCompileGlob_breakList       = 0;

	EmitStatement( bodyNode );

	scrCompileGlob_caseAllowed     = 0;
	scrCompileGlob_caseAllowedDev  = 0;
	scrCompileGlob_breakAllowed    = 0;
	scrCompileGlob_breakAllowedDev = 0;

	EmitOpcode( OP_EndSwitch, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
	EmitShort( 0 );

	table = (scrCaseTableEntry_t *)TempMalloc( 0 );
	*(int *)tablePatch = (int)( (char *)table - tablePatch );

	for ( record = (scrCaseRecord_t *)scrCompileGlob_caseList;
		  record; record = record->next ) {
		EmitInteger( (int)record->value );
		EmitInteger( (int)record->codePos );
		caseCount++;
	}

	*(unsigned short *)( (char *)table - 2 ) = (unsigned short)caseCount;
	qsort( table, caseCount, sizeof( scrCaseTableEntry_t ), CompareCaseInfo );

	for ( index = 1; index < caseCount; index++ ) {
		if ( table[index - 1].value != table[index].value ) {
			continue;
		}
		for ( record = (scrCaseRecord_t *)scrCompileGlob_caseList;
			  record; record = record->next ) {
			if ( record->value == table[index - 1].value ) {
				CompileError( record->sourcePos, "duplicate case expression" );
			}
		}
	}

	ConnectBreakStatements();

	scrCompileGlob_caseAllowed     = state.caseAllowed;
	scrCompileGlob_caseAllowedDev  = state.caseAllowedDev;
	scrCompileGlob_caseList        = (int)state.caseList;
	scrCompileGlob_breakAllowed    = state.breakAllowed;
	scrCompileGlob_breakAllowedDev = state.breakAllowedDev;
	scrCompileGlob_breakList       = (int)state.breakList;
}

static void EmitBreakStatement( unsigned int sourcePos ) {
	scrCodeOffsetPatch_t   *patch;

	if ( !scrCompileGlob_breakAllowed ) {
		CompileError( sourcePos, "illegal break statement" );
	} else if ( !scrCompileGlob_breakAllowedDev
			 && scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( sourcePos,
					  "cannot use /# ... #/ comments directly around a break statement" );
	}

	EmitOpcode( OP_Jump, 0, LOCALDEPTH_NONE );
	EmitInteger( 0 );

	patch = (scrCodeOffsetPatch_t *)
		Hunk_AllocateTempMemoryHighInternal( (int)sizeof( scrCodeOffsetPatch_t ) );
	patch->codePos = (char *)scrCompileGlob_codePos;
	patch->next    = (scrCodeOffsetPatch_t *)scrCompileGlob_breakList;
	scrCompileGlob_breakList = (int)patch;
}

static void EmitContinueStatement( unsigned int sourcePos ) {
	scrCodeOffsetPatch_t   *patch;

	if ( !scrCompileGlob_contAllowed ) {
		CompileError( sourcePos, "illegal continue statement" );
	} else if ( !scrCompileGlob_contAllowedDev
			 && scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( sourcePos,
					  "cannot use /# ... #/ comments directly around a continue statement" );
	}

	EmitOpcode( OP_Jump, 0, LOCALDEPTH_NONE );
	EmitInteger( 0 );

	patch = (scrCodeOffsetPatch_t *)
		Hunk_AllocateTempMemoryHighInternal( (int)sizeof( scrCodeOffsetPatch_t ) );
	patch->codePos = (char *)scrCompileGlob_codePos;
	patch->next    = (scrCodeOffsetPatch_t *)scrCompileGlob_contList;
	scrCompileGlob_contList = (int)patch;
}

/* ---- EmitStatement  0x0046C210 ---- */
static void EmitStatement( sval_t node ) {
	sval_t *n = SVAL_NODE( node );

	switch ( n[0] ) {
	case SVAL_ASSIGNMENT_STATEMENT:
		EmitAssignmentStatement( n[1], n[2], n[3] );
		break;
	case SVAL_CALL_STATEMENT:
		EmitExpressionStatement( n[1] );
		break;
	case SVAL_RETURN_VALUE_STATEMENT:
		EmitReturnStatement( n[1] );
		break;
	case SVAL_RETURN_STATEMENT:
		EmitEndStatement();
		break;
	case SVAL_WAIT_STATEMENT:
		EmitWaitStatement( n[1], n[2], n[3] );
		break;
	case SVAL_IF_STATEMENT:
		EmitIfStatement( n[1], n[2], n[3] );
		break;
	case SVAL_IF_ELSE_STATEMENT:
		EmitIfElseStatement( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_WHILE_STATEMENT:
		EmitWhileStatement( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_DO_WHILE_STATEMENT:
		EmitDoWhileStatement( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_FOR_STATEMENT:
		EmitForStatement( n[1], n[2], n[3], n[4], n[5], n[6] );
		break;
	case SVAL_INC_STATEMENT:
		EmitIncStatement( n[1], n[2] );
		break;
	case SVAL_DEC_STATEMENT:
		EmitDecStatement( n[1], n[2] );
		break;
	case SVAL_REF_ASSIGNMENT_STATEMENT:
		EmitBinaryEqualsOperatorExpression( n[1], n[2], (unsigned char)n[3], n[4] );
		break;
	case SVAL_STATEMENT_BLOCK:
		EmitStatementList( n[1] );
		break;
	case SVAL_DEVELOPER_BLOCK:
		EmitDeveloperStatementList( n[1], n[2] );
		break;
	case SVAL_WAITTILL_STATEMENT:
		EmitWaittillStatement( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_WAITTILLMATCH_STATEMENT:
		EmitWaittillmatchStatement( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_NOTIFY_STATEMENT:
		EmitNotifyStatement( n[1], SVAL_LIST( n[2] ), n[3], n[4] );
		break;
	case SVAL_ENDON_STATEMENT:
		EmitEndOnStatement( n[1], n[2], n[3], n[4] );
		break;
	case SVAL_SWITCH_STATEMENT:
		EmitSwitchStatement( n[1], n[2], n[3] );
		break;
	case SVAL_CASE_STATEMENT:
		EmitCaseStatement( n[1], n[2] );
		break;
	case SVAL_DEFAULT_STATEMENT:
		EmitDefaultStatement( n[1] );
		break;
	case SVAL_BREAK_STATEMENT:
		EmitBreakStatement( n[1] );
		break;
	case SVAL_CONTINUE_STATEMENT:
		EmitContinueStatement( n[1] );
		break;
	default:
		break;
	}
}

/* ---- EmitStatementList  0x0046C4E0 ---- */
static void EmitStatementList( sval_t block ) {
	sval_cell_t    *item;

	for ( item = SVAL_BLOCKLIST( block )->head; item; item = item->next ) {
		EmitStatement( item->value );
	}
}

/* ---- Scr_TransferToDeveloperBuffer  0x0046C510 ---- */
static void Scr_TransferToDeveloperBuffer( void ) {
	char   *tempEnd;
	int     byteCount;
	int     usedBytes;

	tempEnd   = (char *)TempMalloc( 0 );
	byteCount = (int)( tempEnd - (char *)scrCompileGlob_codeRelocStart );
	usedBytes = scrCompilePub_developerBufPos - scrCompileGlob_devOpBuffer;

	if ( usedBytes + byteCount > SCR_DEV_OP_BUF_SIZE ) {
		Com_Error( ERR_DROP,
				   "max developer script size exceeded - increase DEV_OP_BUF_SIZE" );
	}

	Com_Memcpy( (void *)scrCompilePub_developerBufPos,
				(const void *)scrCompileGlob_codeRelocStart, (size_t)byteCount );
	scrCompilePub_developerBufPos += byteCount;
}

/* ---- Scr_TransferStatementListToDeveloperBuffer  0x0046C580 ---- */
static void Scr_TransferStatementListToDeveloperBuffer( void ) {
	int         savedChecksum;
	int         codeOffset;
	int         oldCapacity;
	int         newCapacity;
	char      **newTable;

	savedChecksum = scrCompilePub_checksum;
	EmitOpcode( OP_DevBlockDeferred, 0, LOCALDEPTH_NONE );
	codeOffset = scrCompileGlob_codeRelocStart - scrVarPub_programBuffer;
	scrCompilePub_checksum = savedChecksum;

	if ( codeOffset >= scrCompileGlob_devPatchMax ) {
		oldCapacity = scrCompileGlob_devPatchMax;
		newCapacity = oldCapacity * 2;
		if ( newCapacity <= codeOffset ) {
			newCapacity = codeOffset * 2;
		}

		newTable = (char **)Z_MallocInternal( newCapacity * (int)sizeof( char * ) );
		Com_Memcpy( newTable, (const void *)scrCompileGlob_devPatchTable,
					(size_t)oldCapacity * sizeof( char * ) );
		Com_Memset( &newTable[oldCapacity], 0,
					(size_t)( newCapacity - oldCapacity ) * sizeof( char * ) );

		scrCompileGlob_devPatchMax = newCapacity;
		free( (void *)scrCompileGlob_devPatchTable );
		scrCompileGlob_devPatchTable = (int)newTable;
	}

	( (char **)scrCompileGlob_devPatchTable )[codeOffset] =
		(char *)scrCompilePub_developerBufPos;

	Scr_TransferToDeveloperBuffer();
	TempMemorySetPos( (void *)scrCompileGlob_codeRelocStart );
}

/* ---- EmitDeveloperStatementList  0x0046C660 ---- */
static void EmitDeveloperStatementList( sval_t block, unsigned int sourcePos ) {
	int     savedChecksum;
	void   *tempMark;

	if ( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( sourcePos, "cannot recurse /#" );
	}

	savedChecksum = scrCompilePub_checksum;

	if ( scrCompilePub_developerScript ) {
		if ( !scrCompileGlob_bDeferredCheck ) {
			EmitOpcode( OP_DevBlockBegin, 0, LOCALDEPTH_NONE );
			scrCompileGlob_codeRelocStart = scrCompileGlob_codePos;
		}
		scrCompileGlob_codegenMode = SCR_CODEGEN_RELOCATE;
		EmitStatementList( block );
		scrCompileGlob_bDeferredCheck = 1;
	} else {
		tempMark = TempMalloc( 0 );
		scrCompileGlob_codegenMode = SCR_CODEGEN_DISCARD;
		EmitStatementList( block );
		TempMemorySetPos( tempMark );
	}

	scrCompileGlob_codegenMode = SCR_CODEGEN_INTERN;
	scrCompilePub_checksum = savedChecksum;
}

/* ---- Scr_InitDeveloperOpcodes  0x0046C790 ---- */
void Scr_InitDeveloperOpcodes__Fv( void ) {
	if ( !scrCompilePub_developerScript ) {
		return;
	}

	scrCompileGlob_devOpBuffer   = (int)Z_MallocInternal( SCR_DEV_OP_BUF_SIZE );
	scrCompileGlob_devPatchMax   = SCR_DEV_PATCH_CAPACITY;
	scrCompileGlob_devPatchTable =
		(int)Z_MallocInternal( SCR_DEV_PATCH_CAPACITY * (int)sizeof( char * ) );
	Com_Memset( (void *)scrCompileGlob_devPatchTable, 0,
				SCR_DEV_PATCH_CAPACITY * sizeof( char * ) );

	scrCompilePub_developerBufPos = scrCompileGlob_devOpBuffer;
	scrCompileGlob_stringFixups   = 0;
}

/* ---- Scr_InsertDeveloperOpcodes  0x0046C800 ---- */
void Scr_InsertDeveloperOpcodes__Fv( void ) {
	int                 codeOffset;
	char               *patch;
	scrStringFixup_t   *fixup;
	unsigned short      string;

	if ( !scrCompilePub_developerScript ) {
		return;
	}

	for ( codeOffset = 0; codeOffset < scrCompileGlob_devPatchMax; codeOffset++ ) {
		patch = ( (char **)scrCompileGlob_devPatchTable )[codeOffset];
		if ( patch ) {
			*patch = ( (char *)scrVarPub_programBuffer )[codeOffset];
			( (char *)scrVarPub_programBuffer )[codeOffset] = (char)OP_DevBlockBegin;
		}
	}

	while ( scrCompileGlob_stringFixups ) {
		fixup  = (scrStringFixup_t *)scrCompileGlob_stringFixups;
		string = *fixup->codePos;
		if ( string ) {
			*fixup->codePos = SL_TransferToCanonicalString( string );
		}
		scrCompileGlob_stringFixups = (int)fixup->next;
		free( fixup );
	}
}

/* ---- Scr_ShutdownDeveloperOpcodes  0x0046C8C0 ---- */
void Scr_ShutdownDeveloperOpcodes__Fv( void ) {
	if ( !scrCompilePub_developerScript ) {
		return;
	}

	if ( scrCompileGlob_devOpBuffer ) {
		free( (void *)scrCompileGlob_devOpBuffer );
		scrCompileGlob_devOpBuffer = 0;
	}
	if ( scrCompileGlob_devPatchTable ) {
		free( (void *)scrCompileGlob_devPatchTable );
		scrCompileGlob_devPatchTable = 0;
	}
}

/* ---- EmitFormalParameterListRef  0x0046C910 ---- VERIFIED */
static void EmitFormalParameterListRef( sval_list_t *parameters,
										unsigned int sourcePos ) {
	EmitFormalParameterListRefInternal( parameters->first );
	EmitOpcode( OP_CheckClearParams, 0, LOCALDEPTH_NONE );
	AddOpcodePos( sourcePos );
}

/* ---- SpecifyThread  0x0046C940 ---- */
static void SpecifyThread( sval_t node ) {
	sval_t         *n = SVAL_NODE( node );
	unsigned short  functionObject;

	if ( n[0] == SVAL_FUNCTION_DEFINITION ) {
		functionObject = GetObject( GetVariable(
			SCR_HANDLE( scrCompileGlob_currentFuncRoot ), (unsigned short)n[1] ) );
		SpecifyThreadPosition( functionObject, n[4] );
	} else if ( n[0] == SVAL_DEVELOPER_FUNCTION_DEF ) {
		functionObject = GetObject( GetVariable(
			SCR_HANDLE( scrCompileGlob_currentFuncRoot ), (unsigned short)n[1] ) );
		SpecifyDeveloperThreadPosition( functionObject, n[4] );
	}
}

/* ---- EmitThreadInternal  0x0046C9C0 ---- */
static void EmitThreadInternal( sval_t node, unsigned int sourcePos ) {
	sval_t *n = SVAL_NODE( node );

	AddOpcodePos( sourcePos );

	scrCompileGlob_cumulOffset   = 0;
	scrCompileGlob_maxOffset     = 0;
	scrCompileGlob_maxCallOffset = 0;

	CompileTransferRefToString( (unsigned short)n[1], SCR_STRING_USAGE_FUNCTION );
	EmitFormalParameterListRef( SVAL_LIST( n[2] ), sourcePos );
	EmitStatementList( n[3] );
	EmitOpcode( OP_End, 0, LOCALDEPTH_NONE );

	if ( scrCompileGlob_maxOffset
	   + SCR_LOCAL_SLOT_WIDTH * scrCompileGlob_maxCallOffset
	   >= SCR_OPERAND_STACK_LIMIT ) {
		CompileError( sourcePos, "function exceeds operand stack size" );
	}
}

/* ---- EmitNormalThread  0x0046CAB0 ---- */
static void EmitNormalThread( sval_t node, unsigned int sourcePos ) {
	sval_t         *n = SVAL_NODE( node );
	unsigned short  functionObject;

	functionObject = FindObject( FindVariable(
		SCR_HANDLE( scrCompileGlob_currentFuncRoot ), (unsigned short)n[1] ) );
	SetThreadPosition( functionObject );
	EmitThreadInternal( node, sourcePos );
}

/* ---- EmitDeveloperThread  0x0046CB30 ---- */
static void EmitDeveloperThread( sval_t node, unsigned int sourcePos ) {
	sval_t         *n = SVAL_NODE( node );
	int             savedChecksum;
	unsigned short  functionObject;

	if ( scrCompileGlob_codegenMode != SCR_CODEGEN_INTERN ) {
		CompileError( sourcePos, "cannot recurse /#" );
	}

	scrCompileGlob_codeRelocStart = (int)TempMalloc( 0 );
	savedChecksum = scrCompilePub_checksum;

	if ( scrCompilePub_developerScript ) {
		scrCompileGlob_codegenMode = SCR_CODEGEN_RELOCATE;
		functionObject = FindObject( FindVariable(
			SCR_HANDLE( scrCompileGlob_currentFuncRoot ), (unsigned short)n[1] ) );
		SetDeveloperThreadPosition( functionObject );
		EmitThreadInternal( node, sourcePos );
		Scr_TransferToDeveloperBuffer();
	} else {
		scrCompileGlob_codegenMode = SCR_CODEGEN_DISCARD;
		EmitThreadInternal( node, sourcePos );
	}

	scrCompileGlob_codegenMode = SCR_CODEGEN_INTERN;
	TempMemorySetPos( (void *)scrCompileGlob_codeRelocStart );
	scrCompilePub_checksum = savedChecksum;
}

/* ---- EmitThread  0x0046CC30 ---- */
static void EmitThread( sval_t node, unsigned int sourcePos ) {
	sval_t         *n = SVAL_NODE( node );
	unsigned short  treeNameHandle;
	const char     *treeName;

	if ( n[0] == SVAL_FUNCTION_DEFINITION ) {
		EmitNormalThread( node, sourcePos );
		return;
	}
	if ( n[0] == SVAL_DEVELOPER_FUNCTION_DEF ) {
		EmitDeveloperThread( node, sourcePos );
		return;
	}
	if ( n[0] == SVAL_USING_ANIMTREE ) {
		treeNameHandle = (unsigned short)n[1];
		treeName = SL_ConvertToString( treeNameHandle );
		if ( !Scr_IsIdentifier( treeName ) ) {
			CompileError( n[2], "bad anim tree name" );
		}
		SCR_HANDLE( scrAnimPub_currentAnimTree ) =
			Scr_UsingTreeInternal( treeName, &scrCompileGlob_animTreeValue );
		CompileRemoveRefToString( treeNameHandle );
	}
}

/* ---- EmitThreadList  0x0046CCD0 ---- */
static void EmitThreadList( sval_t block ) {
	sval_cell_t    *item;

	for ( item = SVAL_BLOCKLIST( block )->head; item; item = item->next ) {
		SpecifyThread( SVAL_ENTRY( item->value )->node );
	}
	for ( item = SVAL_BLOCKLIST( block )->head; item; item = item->next ) {
		EmitThread( SVAL_ENTRY( item->value )->node,
					SVAL_ENTRY( item->value )->sourcePos );
	}
}

/* ---- ScriptCompile  0x0046CD20 ---- */
void ScriptCompile( sval_t parseTree, unsigned short currentFunctionRoot ) {
	scrScriptLoad_t    *pendingLoads;
	int                 pendingCount;
	int                 index;
	const char         *filename;

	SCR_HANDLE( scrCompileGlob_currentFuncRoot ) = currentFunctionRoot;

	scrCompileGlob_codePos        = (int)TempMalloc( 0 );
	scrCompileGlob_animTreeValue  = 0;
	scrCompileGlob_codegenMode    = SCR_CODEGEN_INTERN;
	scrCompileGlob_bDeferredCheck = 0;
	scrCompileGlob_bOwnsStrings   = 0;

	pendingCount = scrCompilePub_scriptCount;
	pendingLoads = pendingCount
		? (scrScriptLoad_t *)_alloca( pendingCount * sizeof( scrScriptLoad_t ) )
		: NULL;
	scrCompileGlob_loadCursor = (int)pendingLoads;

	EmitOpcode( OP_End, 0, LOCALDEPTH_NONE );
	EmitThreadList( SVAL_NODE( parseTree )[1] );

	scrCompilePub_programLen =
		(int)( (char *)TempMalloc( 0 ) - (char *)scrVarPub_programBuffer );

	AdjustFunctionAddresses();

	Hunk_CommitTempMemory();
	Hunk_ClearTempMemoryHigh();

	pendingCount = scrCompilePub_scriptCount;
	for ( index = 0; index < pendingCount; index++ ) {
		filename = SL_ConvertToString( pendingLoads[index].filename );
		if ( !Scr_LoadScript( filename ) ) {
			CompileError( pendingLoads[index].sourcePos,
						  "Could not find script '%s'",
						  SL_ConvertToString( pendingLoads[index].filename ) );
		}
		SL_RemoveRefToString( pendingLoads[index].filename );
	}
}
