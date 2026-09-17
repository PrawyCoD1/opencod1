/*
 * script/scr_vm.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_vm.cpp
 *
 * Retail range 0x00475850-0x0047E360, 82 functions.
 *
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"

#define FindNextSibling     cod1_globals_bogus_decl_FindNextSibling
#define FindObject          cod1_globals_bogus_decl_FindObject
#define scrVmGlob_callStack        cod1_globals_scalar_decl_A7A510
#define scrVmGlob_devCallStack        cod1_globals_scalar_decl_A7A488
#define scrVmGlob_devOpcode         cod1_globals_scalar_decl_A7A5AC
#include "../qcommon/cod1_globals.h"
#undef FindNextSibling
#undef FindObject
#undef scrVmGlob_callStack
#undef scrVmGlob_devCallStack
#undef scrVmGlob_devOpcode

#include "scr_local.h"

#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define SCR_VM_STACK_COUNT      2048    /* 0x00A76480, 8 bytes each */
#define SCR_VM_MAX_CALL_DEPTH   32

#define SCR_TIME_MASK           0x00FFFFFFu
#define SCR_MAX_WAIT_SECONDS    16777
#define SCR_WAIT_SCALE          1000.0f
#define SCR_SHIFT_MASK          0x1F
#define SCR_FLOAT_EQ_EPSILON    0.000001f

#define SCR_STRING_MT_TYPE      14
#define SCR_STRING_MT_TYPE_ADD  6

#define SCR_NAME_NOTIFY_LIST    0x20000u
#define SCR_NAME_STACK          0x20001u

#define SCR_STACK_ENTRY_SIZE    5
#define SCR_STACK_HEADER_SIZE   12
#define SCR_STACK_BUFFER_SIZE( n )  ( SCR_STACK_ENTRY_SIZE * (n) + SCR_STACK_HEADER_SIZE )
#define SCR_STACK_MT_TYPE           1

typedef struct VariableStackBuffer_s {
	unsigned int    time;           /* +0x00 resume key, or 0 for a notify wait */
	const char     *codePos;        /* +0x04 where to resume */
	unsigned short  valueCount;     /* +0x08 packed entries that follow */
	unsigned short  thread;         /* +0x0A innermost thread handle */
	unsigned char   values[1];      /* +0x0C count * (byte type, dword payload) */
} VariableStackBuffer;

/* retail 0x00A76478 / 0x00A7A5A0 / 0x00A76480 -- see VM_Init */
VariableValue           scrVmPub_stack[SCR_VM_STACK_COUNT];
VariableValue          *scrVmPub_top;
VariableValue          *scrVmPub_maxstack;

int                     scrVmPub_outparamcount;
int                     scrVmPub_inparamcount;

#define scrVmGlob_funcCount         scrVmGlob_funcCount        /* 0x00A7A5D8 */
#define scrVmPub_terminal_error     scrVmPub_terminal_error         /* 0x00A7A5CC */

/* 0x00A7A5A4 / the dialog-message slot beside it */
const char             *scrVarPub_error_message;
const char             *scrVmGlob_dialog_error_message;

int                     scrVmGlob_leftType;
int                     scrVmGlob_rightType;

int                     scrVmGlob_loopTick;

/* 0x00A7A510 / 0x00A7A488 / 0x00A7A5AC -- the three per-depth stacks. */
const char             *scrVmGlob_callStack[SCR_VM_MAX_CALL_DEPTH];
const char             *scrVmGlob_devCallStack[SCR_VM_MAX_CALL_DEPTH];
unsigned char           scrVmGlob_devOpcode[SCR_VM_MAX_CALL_DEPTH];

#define scrVmGlob_callStack     scrVmGlob_callStack
#define scrVmGlob_devCallStack  scrVmGlob_devCallStack
#define scrVmGlob_devOpcode     scrVmGlob_devOpcode

unsigned short          scrVarPub_pauseArrayId;
unsigned short          scrVarPub_levelId;
unsigned short          scrVarPub_gameId;
unsigned short          scrVarPub_animId;

unsigned int            scrVarPub_time;

#define SCR_PROGRAM_BASE        ( (const char *)scrVarPub_programBuffer )
#define SCR_DEV_PATCH_TABLE     ( (unsigned char **)scrCompileGlob_devPatchTable )
#define SCR_DEVELOPER           scrVarPub_developer

#define SCR_SET_ENTITY_FIELD    scrImport_SetObjectField
#define SCR_GET_ENTITY_FIELD    scrImport_GetObjectField

extern int              scr_loading;
extern int              scrVarPub_error_index;
extern int              __rdtsc( void );

extern int  Scr_GetAnims( int slot );
extern int  Scr_GetAnimTreeCount( void );

extern int  RuntimeError();
extern int  Scr_PrintPrevCodePos();
extern unsigned short   Scr_EvalArrayIndex_m( unsigned short id, VariableValue *index );
extern unsigned short   EvalArrayRef( unsigned short id, VariableValue *index );
extern void             ClearArray( unsigned short id, VariableValue *index );
extern void             GetEmptyArray( VariableValue *value );

static unsigned short   VM_Execute( VariableValue *stackTop, const char *codePos,
									unsigned short thread, unsigned short currentObject,
									VariableValue *stackBase );

static jmp_buf         *scrVmErrorFrame;

/* ---- Scr_Error  0x0047DF40 ---- */
void Scr_Error( const char *error ) {
	scrVarPub_error_message = error;

	if ( scrVmGlob_funcCount != 0 && scrVmErrorFrame != NULL ) {
		longjmp( *scrVmErrorFrame, 1 );
	}
	Com_Error( ERR_DROP, "\x15%s", error );
}

void Scr_RaiseError( void ) {
	if ( scrVmGlob_funcCount != 0 && scrVmErrorFrame != NULL ) {
		longjmp( *scrVmErrorFrame, 1 );
	}
	Com_Error( ERR_DROP, "\x15%s", scrVarPub_error_message );
}

/* ---- Scr_Error__Fv  0x00475980 ---- */
int Scr_Error__Fv( void ) { Scr_RaiseError(); return 0; }

/* ---- Scr_ErrorWithDialogMessage  0x0047DF80 ---- */
void Scr_ErrorWithDialogMessage( const char *error, const char *dialogError ) {
	scrVmGlob_dialog_error_message = dialogError;
	Scr_Error( error );
}

/* ---- Scr_TerminalError  0x0047DFD0 ---- */
void Scr_TerminalError( const char *error ) {
	Scr_DumpScriptThreads();
	scrVmPub_terminal_error = 1;
	Scr_Error( error );
}

/* ---- Scr_ParamError  0x0047E020 ---- */
void Scr_ParamError( unsigned int index, const char *error ) {
	scrVarPub_error_index = (int)index + 1;
	Scr_Error( error );
}

/* ---- Scr_ObjectError  0x0047E070 ---- */
void Scr_ObjectError( const char *error ) {
	scrVarPub_error_index = -1;
	Scr_Error( error );
}

#define VAR_NODE( id )          ( &scrVarNodes[ (id) ] )
#define VAR_TYPEOF( id )        ( (int)( scrVarNodes[ (id) ].status & VAR_STATUS_TYPE_MASK ) )
#define VAR_NAMEOF( id )        ( scrVarNodes[ (id) ].status >> VAR_STATUS_NAME_SHIFT )

/* AddRefToValue__FP13VariableValue  0x00475AEE  VERIFIED */
void AddRefToValue__FP13VariableValue( VariableValue *value ) {
	AddRefToValue( value->type, value->u );
}

void RemoveRefToValue__FP13VariableValue( VariableValue *value ) {
	RemoveRefToValue( value->type, value->u );
}

static void VM_ReleaseValue( VariableValue *v ) {
	RemoveRefToValue__FP13VariableValue( v );
	v->type = VAR_UNDEFINED;
}

static VariableValue *VM_ReleaseToMarker( VariableValue *top ) {
	while ( top->type != VAR_CODEPOS ) {
		RemoveRefToValue( top->type, top->u );
		top--;
	}
	return top;
}

/* ---- IsZero  0x004759D0 ---- */
qboolean IsZero( const char *s ) {
	if ( *s <= ' ' ) {
		while ( *s ) {
			s++;
			if ( *s > ' ' ) {
				break;
			}
		}
		if ( *s == '\0' ) {
			return qfalse;
		}
	}
	if ( *s == '-' || *s == '+' ) {
		s++;
	}
	if ( *s == '0' ) {
		return qtrue;
	}
	if ( *s != '.' ) {
		return qfalse;
	}
	return s[1] == '0' ? qtrue : qfalse;
}

int IsZero__FPCc( const char *s ) { return IsZero( s ); }

/* ---- Scr_GetNumScriptThreads  0x004759C0 ---- VERIFIED */
int Scr_GetNumScriptThreads( void ) {
	return 0;
}

/* ---- CastBool  0x00475A10 ---- */
qboolean CastBool( VariableValue *value ) {
	unsigned short  handle;
	int             n;

	if ( value->type == VAR_FLOAT ) {
		value->type = VAR_INTEGER;
		value->u.intValue = ( value->u.floatValue != 0.0f ) ? 1 : 0;
		return qtrue;
	}

	if ( value->type == VAR_STRING ) {
		handle = value->u.stringValue;
		n = atol( SL_ConvertToString( handle ) );
		value->u.intValue = ( n != 0 ) ? 1 : 0;

		if ( n != 0 || IsZero( SL_ConvertToString( handle ) ) ) {
			value->type = VAR_INTEGER;
			SL_RemoveRefToString( handle );
			return qtrue;
		}

		scrVarPub_error_message =
			va( "cannot cast \"%s\" to bool", SL_ConvertToString( handle ) );
		SL_RemoveRefToString( handle );
		value->type = VAR_UNDEFINED;
		return qfalse;
	}

	scrVarPub_error_message =
		va( "cannot cast %s to bool", var_typename[value->type] );
	VM_ReleaseValue( value );
	return qfalse;
}

/* ---- CastInt  0x00475B00 ---- */
qboolean CastInt( VariableValue *value ) {
	unsigned short  handle;
	int             n;

	switch ( value->type ) {
	case VAR_INTEGER:
		return qtrue;

	case VAR_FLOAT:
		value->type = VAR_INTEGER;
		value->u.intValue = (int)value->u.floatValue;
		return qtrue;

	case VAR_STRING:
		handle = value->u.stringValue;
		n = atol( SL_ConvertToString( handle ) );
		value->u.intValue = n;

		if ( n == 0 && !IsZero( SL_ConvertToString( handle ) ) ) {
			scrVarPub_error_message =
				va( "cannot cast \"%s\" to int", SL_ConvertToString( handle ) );
			SL_RemoveRefToString( handle );
			value->type = VAR_UNDEFINED;
			return qfalse;
		}
		value->type = VAR_INTEGER;
		SL_RemoveRefToString( handle );
		return qtrue;

	default:
		break;
	}

	scrVarPub_error_message =
		va( "cannot cast %s to int", var_typename[value->type] );
	VM_ReleaseValue( value );
	return qfalse;
}

/* ---- CastFloat  0x00475BD0 ---- */
qboolean CastFloat( VariableValue *value ) {
	unsigned short  handle;
	double          f;

	switch ( value->type ) {
	case VAR_FLOAT:
		return qtrue;

	case VAR_INTEGER:
		value->type = VAR_FLOAT;
		value->u.floatValue = (float)value->u.intValue;
		return qtrue;

	case VAR_STRING:
		handle = value->u.stringValue;
		f = atof( SL_ConvertToString( handle ) );
		value->u.floatValue = (float)f;

		if ( f == 0.0 && !IsZero( SL_ConvertToString( handle ) ) ) {
			scrVarPub_error_message =
				va( "cannot cast \"%s\" to float", SL_ConvertToString( handle ) );
			SL_RemoveRefToString( handle );
			value->type = VAR_UNDEFINED;
			return qfalse;
		}
		value->type = VAR_FLOAT;
		SL_RemoveRefToString( handle );
		return qtrue;

	default:
		break;
	}

	scrVarPub_error_message =
		va( "cannot cast %s to float", var_typename[value->type] );
	VM_ReleaseValue( value );
	return qfalse;
}

/* ---- CastString  0x00475CB0 ---- */
qboolean CastString( VariableValue *value ) {
	const float *v;

	switch ( value->type ) {
	case VAR_STRING:
		return qtrue;

	case VAR_INTEGER:
		value->u.stringValue = SL_GetStringForInt( value->u.intValue );
		value->type = VAR_STRING;
		return qtrue;

	case VAR_FLOAT:
		value->u.stringValue = SL_GetStringForFloat( value->u.floatValue );
		value->type = VAR_STRING;
		return qtrue;

	case VAR_VECTOR:
		v = value->u.vectorValue;
		value->u.stringValue = SL_GetStringForVector( v );
		value->type = VAR_STRING;
		RemoveRefToVector( v );
		return qtrue;

	default:
		break;
	}

	scrVarPub_error_message =
		va( "cannot cast %s to string", var_typename[value->type] );
	VM_ReleaseValue( value );
	return qfalse;
}

/* ---- CastIString  0x00475D50 ---- */
qboolean CastIString( VariableValue *value ) {
	if ( value->type == VAR_ISTRING ) {
		return qtrue;
	}
	scrVarPub_error_message =
		va( "cannot cast %s to istring", var_typename[value->type] );
	VM_ReleaseValue( value );
	return qfalse;
}

/* ---- CastVector  0x00475D90 ---- */
qboolean CastVector( VariableValue *value ) {
	if ( value->type == VAR_VECTOR ) {
		return qtrue;
	}
	scrVarPub_error_message =
		va( "cannot cast %s to vector", var_typename[value->type] );
	VM_ReleaseValue( value );
	return qfalse;
}

/* ---- CastPointer  0x00475DD0 ---- */
qboolean CastPointer( VariableValue *value ) {
	if ( value->type == VAR_OBJECT ) {
		return qtrue;
	}
	scrVarPub_error_message =
		va( "cannot cast %s to object", var_typename[value->type] );
	VM_ReleaseValue( value );
	return qfalse;
}

/* ---- CastVector2  0x00475E10 ---- */
qboolean CastVector2( VariableValue *values ) {
	float   v[3];
	float  *out;
	int     i;

	for ( i = 2; i >= 0; i-- ) {
		if ( !CastFloat( &values[i] ) ) {
			scrVarPub_error_index = i;
			Scr_RaiseError();
			return qfalse;
		}
		v[2 - i] = values[i].u.floatValue;
	}

	out = AllocVector();
	out[0] = v[0];
	out[1] = v[1];
	out[2] = v[2];

	values[0].type = VAR_VECTOR;
	values[0].u.vectorValue = out;
	return qtrue;
}

/* ---- ClearVector  0x00475ED0 ---- */
void ClearVector( VariableValue *values ) {
	int i;

	for ( i = 2; i >= 0; i-- ) {
		RemoveRefToValue( values[i].type, values[i].u );
	}
	values[0].type = VAR_UNDEFINED;
}

/* ---- UnmatchingTypesError  0x00475FB0 ---- */
void VM_UnmatchingTypesError( VariableValue *left, VariableValue *right ) {
	scrVarPub_error_message = va( "pair has unmatching types '%s' and '%s'",
								  var_typename[scrVmGlob_leftType],
								  var_typename[scrVmGlob_rightType] );
	VM_ReleaseValue( right );
	VM_ReleaseValue( left );
}

/* ---- CastWeakerPair  0x00476090 ---- */
qboolean CastWeakerPair( VariableValue *left, VariableValue *right ) {
	const float *v;

	scrVmGlob_leftType  = left->type;
	scrVmGlob_rightType = right->type;

	if ( left->type == right->type ) {
		return qtrue;
	}

	if ( left->type < right->type ) {
		if ( left->type == VAR_STRING ) {
			if ( right->type == VAR_VECTOR ) {
				v = right->u.vectorValue;
				right->u.stringValue = SL_GetStringForVector( v );
				right->type = VAR_STRING;
				RemoveRefToVector( v );
				return qtrue;
			}
			if ( right->type == VAR_FLOAT ) {
				right->u.stringValue = SL_GetStringForFloat( right->u.floatValue );
				right->type = VAR_STRING;
				return qtrue;
			}
			if ( right->type == VAR_INTEGER ) {
				right->u.stringValue = SL_GetStringForInt( right->u.intValue );
				right->type = VAR_STRING;
				return qtrue;
			}
		} else if ( left->type == VAR_FLOAT && right->type == VAR_INTEGER ) {
			right->u.floatValue = (float)right->u.intValue;
			right->type = VAR_FLOAT;
			return qtrue;
		}
	} else {
		if ( right->type == VAR_STRING ) {
			if ( left->type == VAR_VECTOR ) {
				v = left->u.vectorValue;
				left->u.stringValue = SL_GetStringForVector( v );
				left->type = VAR_STRING;
				RemoveRefToVector( v );
				return qtrue;
			}
			if ( left->type == VAR_FLOAT ) {
				left->u.stringValue = SL_GetStringForFloat( left->u.floatValue );
				left->type = VAR_STRING;
				return qtrue;
			}
			if ( left->type == VAR_INTEGER ) {
				left->u.stringValue = SL_GetStringForInt( left->u.intValue );
				left->type = VAR_STRING;
				return qtrue;
			}
		} else if ( right->type == VAR_FLOAT && left->type == VAR_INTEGER ) {
			left->u.floatValue = (float)left->u.intValue;
			left->type = VAR_FLOAT;
			return qtrue;
		}
	}

	VM_UnmatchingTypesError( left, right );
	return qfalse;
}

/* CastWeakerPairArray  0x004761E0 */
qboolean CastWeakerPairArray( VariableValue *values ) {
	return CastWeakerPair( &values[0], &values[1] );
}

/* ---- CheckEquality  0x004761E0 ---- */
qboolean CheckEquality( VariableValue *left, VariableValue *right ) {
	unsigned short  ls, rs;
	const float    *lv, *rv;
	float           d;

	if ( !CastWeakerPair( left, right ) ) {
		return qfalse;
	}

	switch ( left->type ) {
	case VAR_UNDEFINED:
		left->type = VAR_INTEGER;
		left->u.intValue = 1;
		return qtrue;

	case VAR_STRING:
	case VAR_ISTRING:
		ls = left->u.stringValue;
		rs = right->u.stringValue;
		left->type = VAR_INTEGER;
		SL_RemoveRefToString( ls );
		SL_RemoveRefToString( rs );
		left->u.intValue = ( ls == rs ) ? 1 : 0;
		return qtrue;

	case VAR_VECTOR:
		lv = left->u.vectorValue;
		rv = right->u.vectorValue;
		left->type = VAR_INTEGER;
		left->u.intValue = ( lv[0] == rv[0] && lv[1] == rv[1] && lv[2] == rv[2] ) ? 1 : 0;
		RemoveRefToVector( lv );
		RemoveRefToVector( rv );
		return qtrue;

	case VAR_FLOAT:
		d = left->u.floatValue - right->u.floatValue;
		left->type = VAR_INTEGER;
		left->u.intValue = ( fabs( (double)d ) < (double)SCR_FLOAT_EQ_EPSILON ) ? 1 : 0;
		return qtrue;

	case VAR_INTEGER:
		left->u.intValue = ( left->u.intValue == right->u.intValue ) ? 1 : 0;
		return qtrue;

	case VAR_OBJECT:
		ls = left->u.halfword[0];
		rs = right->u.halfword[0];
		left->type = VAR_INTEGER;
		RemoveRefToObject( ls );
		RemoveRefToObject( rs );
		left->u.intValue = ( ls == rs ) ? 1 : 0;
		return qtrue;

	case VAR_ANIMATION:
		left->type = VAR_INTEGER;
		left->u.intValue = ( left->u.uintValue == right->u.uintValue ) ? 1 : 0;
		return qtrue;

	default:
		break;
	}

	VM_UnmatchingTypesError( left, right );
	return qfalse;
}

#define SCR_CONCAT_MAX  0x4000

/* ---- VM_ConcatenateStrings  0x0047E360 ---- */
unsigned short VM_ConcatenateStrings( VariableValue *values ) {
	static char     buffer[SCR_CONCAT_MAX];
	const char     *a;
	const char     *b;
	unsigned int    la, lb;

	a = SL_ConvertToString( values[0].u.stringValue );
	b = SL_ConvertToString( values[1].u.stringValue );
	if ( a == NULL ) {
		a = "";
	}
	if ( b == NULL ) {
		b = "";
	}

	la = strlen( a );
	lb = strlen( b );
	if ( la + lb + 1 > SCR_CONCAT_MAX ) {
		Scr_Error( "string concatenation too long" );
		return 0;
	}

	memcpy( buffer, a, la );
	memcpy( buffer + la, b, lb + 1 );
	return SL_GetStringOfLen( buffer, 0, la + lb + 1, SCR_STRING_MT_TYPE );
}

/* ---- IncInParam  0x0047C470 ---- */
int IncInParam( void ) {
	while ( scrVmPub_outparamcount != 0 ) {
		RemoveRefToValue( scrVmPub_top->type, scrVmPub_top->u );
		scrVmPub_top--;
		scrVmPub_outparamcount--;
	}

	scrVmPub_top++;
	scrVmPub_inparamcount++;

	if ( scrVmPub_top > scrVmPub_maxstack ) {
		Com_Error( ERR_DROP, "\x15Internal script stack overflow" );
	}
	return 0;
}

int IncInParam__Fv( void ) { return IncInParam(); }

static void VM_ReleaseInParams( void ) {
	while ( scrVmPub_outparamcount != 0 ) {
		RemoveRefToValue( scrVmPub_top->type, scrVmPub_top->u );
		scrVmPub_top--;
		scrVmPub_outparamcount--;
	}
}

static VariableValue *Scr_Param( unsigned int index ) {
	if ( index >= (unsigned int)scrVmPub_outparamcount ) {
		Scr_Error( va( "parameter %d does not exist", index + 1 ) );
		return scrVmPub_top;
	}
	return scrVmPub_top - index;
}

/* ---- Scr_GetNumParam  0x0047DB30 ---- */
int Scr_GetNumParam( void ) {
	return scrVmPub_outparamcount;
}

/* ---- Scr_GetType  0x0047DA20 ---- */
int Scr_GetType( unsigned int index ) {
	return Scr_Param( index )->type;
}

/* ---- Scr_GetPointerType  0x0047DA90 ---- */
int Scr_GetPointerType( unsigned int index ) {
	VariableValue *v = Scr_Param( index );

	if ( v->type != VAR_OBJECT ) {
		Scr_Error( va( "parameter %d is not a pointer", index + 1 ) );
		return VAR_UNDEFINED;
	}
	return VAR_TYPEOF( v->u.halfword[0] );
}

/* ---- Scr_GetBool  0x0047D050 ---- */
int Scr_GetBool( unsigned int index ) {
	VariableValue *v = Scr_Param( index );

	if ( v->type == VAR_INTEGER ) {
		return v->u.intValue != 0;
	}
	if ( !CastBool( v ) ) {
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
	}
	return v->u.intValue;
}

/* ---- Scr_GetInt  0x0047D100 ---- */
int Scr_GetInt( unsigned int index ) {
	VariableValue *v = Scr_Param( index );

	if ( !CastInt( v ) ) {
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
	}
	return v->u.intValue;
}

/* ---- Scr_GetFloat  0x0047D3F0 ---- */
float Scr_GetFloat( unsigned int index ) {
	VariableValue *v = Scr_Param( index );

	if ( !CastFloat( v ) ) {
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
	}
	return v->u.floatValue;
}

/* ---- Scr_GetConstString  0x0047D490 ---- */
unsigned short Scr_GetConstString( unsigned int index ) {
	VariableValue *v = Scr_Param( index );

	if ( !CastString( v ) ) {
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
	}
	return v->u.stringValue;
}

/* ---- Scr_GetString  0x0047D530 ---- */
const char *Scr_GetString( unsigned int index ) {
	unsigned short handle = Scr_GetConstString( index );

	return handle != 0 ? GetRefString( handle )->text : NULL;
}

/* ---- Scr_GetConstIString  0x0047D6B0 ---- */
unsigned short Scr_GetConstIString( unsigned int index ) {
	VariableValue *v = Scr_Param( index );

	if ( v->type != VAR_ISTRING ) {
		scrVarPub_error_message =
			va( "cannot cast %s to istring", var_typename[v->type] );
		VM_ReleaseValue( v );
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
	}
	return v->u.stringValue;
}

/* ---- Scr_GetIString  0x0047D760 ---- */
const char *Scr_GetIString( unsigned int index ) {
	unsigned short handle = Scr_GetConstIString( index );

	return handle != 0 ? GetRefString( handle )->text : NULL;
}

/* ---- Scr_GetDebugString  0x0047D560 ---- */
const char *Scr_GetDebugString( unsigned int index ) {
	VariableValue  *v = Scr_Param( index );
	int             type = v->type;
	const char     *typeName;
	unsigned int    anim;
	void           *tree;

	typeName = ( type == VAR_OBJECT )
		? var_typename[ VAR_TYPEOF( v->u.halfword[0] ) ]
		: var_typename[ type ];

	if ( CastString( v ) ) {
		return v->u.stringValue != 0 ? GetRefString( v->u.stringValue )->text : NULL;
	}

	scrVarPub_error_message = NULL;
	scrVmGlob_dialog_error_message = NULL;
	scrVarPub_error_index = 0;

	if ( type == VAR_ANIMATION ) {
		anim = v->u.uintValue;
		tree = (void *)Scr_GetAnims( (int)( anim >> 16 ) );
		if ( tree == NULL ) {
			return "<bad animtree>";
		}
		if ( *(unsigned short *)( (char *)tree + 8 * ( anim & 0xFFFF ) + 8 ) != 0 ) {
			return "<non-leaf anim>";
		}
		return **(char ***)( (char *)tree + 8 * ( anim & 0xFFFF ) + 12 );
	}
	if ( type == VAR_ISTRING ) {
		return SL_ConvertToString( v->u.stringValue );
	}
	return typeName;
}

/* ---- Scr_GetAnim  0x0047D190 ---- */
unsigned int Scr_GetAnim( unsigned int index, const char **tree ) {
	VariableValue  *v = Scr_Param( index );
	unsigned int    anim;
	const char    **owner;
	const char     *animName;

	if ( v->type != VAR_ANIMATION ) {
		scrVarPub_error_message =
			va( "cannot cast %s to anim", var_typename[v->type] );
		VM_ReleaseValue( v );
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
		return 0;
	}

	anim = v->u.uintValue;
	if ( tree == NULL ) {
		return anim;
	}

	owner = (const char **)Scr_GetAnims( (int)( anim >> 16 ) );
	if ( owner == *(const char ***)tree ) {
		return anim;
	}

	if ( owner == NULL ) {
		animName = "<bad animtree>";
	} else if ( *(unsigned short *)&owner[ 2 * ( anim & 0xFFFF ) + 2 ] != 0 ) {
		animName = "<non-leaf anim>";
	} else {
		animName = *(const char **)owner[ 2 * ( anim & 0xFFFF ) + 3 ];
	}

	scrVarPub_error_message = va(
		"anim '%s' in animtree '%s' does not belong to the entity's animtree '%s'",
		animName, owner != NULL ? owner[0] : "?", *tree );
	VM_ReleaseValue( v );
	scrVarPub_error_index = (int)index + 1;
	Scr_RaiseError();
	return 0;
}

/* ---- Scr_GetAnimTree  0x0047D2C0 ---- */
void *Scr_GetAnimTree( unsigned int index ) {
	VariableValue  *v = Scr_Param( index );
	void           *tree;

	if ( v->type != VAR_INTEGER ) {
		scrVarPub_error_message =
			va( "cannot cast %s to animtree", var_typename[v->type] );
		VM_ReleaseValue( v );
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
		return NULL;
	}

	tree = (void *)Scr_GetAnims( v->u.intValue );
	if ( v->u.uintValue > (unsigned int)Scr_GetAnimTreeCount() || tree == NULL ) {
		scrVarPub_error_message = "bad anim tree";
		VM_ReleaseValue( v );
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
		return NULL;
	}
	return tree;
}

/* ---- Scr_GetVector  0x0047D790 ---- */
void Scr_GetVector( unsigned int index, float *out ) {
	VariableValue *v = Scr_Param( index );

	if ( v->type != VAR_VECTOR ) {
		scrVarPub_error_message =
			va( "cannot cast %s to vector", var_typename[v->type] );
		VM_ReleaseValue( v );
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
		return;
	}
	out[0] = v->u.vectorValue[0];
	out[1] = v->u.vectorValue[1];
	out[2] = v->u.vectorValue[2];
}

/* ---- Scr_GetFunc  0x0047D860 ---- */
int Scr_GetFunc( unsigned int index ) {
	VariableValue *v = Scr_Param( index );

	if ( v->type != VAR_FUNCTION ) {
		scrVarPub_error_message = "not a function";
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
		return 0;
	}
	return (int)( v->u.codePosValue - SCR_PROGRAM_BASE );
}

/* ---- Scr_GetEntityNum  0x0047D8F0 ---- */
int Scr_GetEntityNum( unsigned int index, unsigned int *classnum ) {
	VariableValue  *v = Scr_Param( index );
	unsigned short  id;

	if ( v->type != VAR_OBJECT ) {
		scrVarPub_error_message =
			va( "cannot cast %s to object", var_typename[v->type] );
		VM_ReleaseValue( v );
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
		return 0;
	}

	id = v->u.halfword[0];
	if ( VAR_TYPEOF( id ) != VAR_ENTITY ) {
		scrVarPub_error_message =
			va( "cannot cast %s to entity", var_typename[ VAR_TYPEOF( id ) ] );
		scrVarPub_error_index = (int)index + 1;
		Scr_RaiseError();
		return 0;
	}

	*classnum = VAR_NAMEOF( id );
	return GetEntnum( id );
}

/* ---- Scr_AddBool  0x0047DB40 ---- */
void Scr_AddBool( qboolean value ) {
	IncInParam();
	scrVmPub_top->type = VAR_INTEGER;
	scrVmPub_top->u.intValue = value != qfalse;
}

/* ---- Scr_AddInt  0x0047DB60 ---- */
void Scr_AddInt( int value ) {
	IncInParam();
	scrVmPub_top->type = VAR_INTEGER;
	scrVmPub_top->u.intValue = value;
}

/* ---- Scr_AddFloat  0x0047DB80 ---- */
void Scr_AddFloat( float value ) {
	IncInParam();
	scrVmPub_top->type = VAR_FLOAT;
	scrVmPub_top->u.floatValue = value;
}

/* ---- Scr_AddAnim  0x0047DBA0 ---- */
void Scr_AddAnim( unsigned int value ) {
	IncInParam();
	scrVmPub_top->type = VAR_ANIMATION;
	scrVmPub_top->u.uintValue = value;
}

/* ---- Scr_AddUndefined  0x0047DBC0 ---- */
void Scr_AddUndefined( void ) {
	IncInParam();
	scrVmPub_top->type = VAR_UNDEFINED;
}

/* ---- Scr_AddObject  0x0047DBE0 ---- */
void Scr_AddObject( unsigned short id ) {
	IncInParam();
	scrVmPub_top->type = VAR_OBJECT;
	scrVmPub_top->u.halfword[0] = id;
	AddRefToObject( id );
}

/* ---- Scr_AddEntityNum  0x0047DC10 ---- */
void Scr_AddEntityNum( int entnum, int classnum ) {
	unsigned short id = Scr_GetEntityId( entnum, classnum );

	IncInParam();
	scrVmPub_top->type = VAR_OBJECT;
	scrVmPub_top->u.halfword[0] = id;
	AddRefToObject( id );
}

/* ---- Scr_AddStruct  0x0047DC50 ---- */
void Scr_AddStruct( void ) {
	VariableValueInternal  *node = AllocVariable();
	unsigned short          id;

	node->status = ( VAR_STAT_EXTERNAL | VAR_STRUCT );
	node->u.halfword[0] = 0;
	id = (unsigned short)( node - scrVarNodes );

	IncInParam();
	scrVmPub_top->type = VAR_OBJECT;
	scrVmPub_top->u.halfword[0] = id;
	AddRefToObject( id );
	RemoveRefToObject( id );
}

/* ---- Scr_AddString  0x0047DCE0 ---- */
void Scr_AddString( const char *text ) {
	IncInParam();
	scrVmPub_top->type = VAR_STRING;
	scrVmPub_top->u.stringValue =
		SL_GetStringOfLen( text, 0, strlen( text ) + 1, SCR_STRING_MT_TYPE_ADD );
}

/* ---- Scr_AddIString  0x0047DD30 ---- */
void Scr_AddIString( const char *text ) {
	IncInParam();
	scrVmPub_top->type = VAR_ISTRING;
	scrVmPub_top->u.stringValue =
		SL_GetStringOfLen( text, 0, strlen( text ) + 1, SCR_STRING_MT_TYPE_ADD );
}

/* ---- Scr_AddConstString  0x0047DD80 ---- */
void Scr_AddConstString( unsigned short handle ) {
	IncInParam();
	scrVmPub_top->type = VAR_STRING;
	scrVmPub_top->u.stringValue = handle;
	SL_AddRefToString( handle );
}

/* ---- Scr_AddVector  0x0047DDB0 ---- */
void Scr_AddVector( const float *v ) {
	float *out;

	IncInParam();
	out = AllocVector();
	out[0] = v[0];
	out[1] = v[1];
	out[2] = v[2];
	scrVmPub_top->type = VAR_VECTOR;
	scrVmPub_top->u.vectorValue = out;
}

/* ---- Scr_MakeArray  0x0047DE00 ---- */
void Scr_MakeArray( void ) {
	IncInParam();
	scrVmPub_top->type = VAR_OBJECT;
	scrVmPub_top->u.halfword[0] = Scr_AllocArray();
}

/* ---- Scr_AddArray  0x0047DE50 ---- */
void Scr_AddArray( void ) {
	unsigned short  arrayId;
	unsigned short  child;

	scrVmPub_top--;
	scrVmPub_inparamcount--;

	arrayId = scrVmPub_top->u.halfword[0];
	child = GetVariable( arrayId,
						 GetInternalVariableIndex( GetArraySize( arrayId ) ) );

	scrVarNodes[child].status |= (unsigned int)scrVmPub_top[1].type;
	scrVarNodes[child].u = scrVmPub_top[1].u;
}

/* ---- Scr_AddArrayStringIndexed  0x0047DED0 ---- */
void Scr_AddArrayStringIndexed( unsigned short name ) {
	unsigned short  arrayId;
	unsigned short  child;

	scrVmPub_top--;
	scrVmPub_inparamcount--;

	arrayId = scrVmPub_top->u.halfword[0];
	child = GetVariable( arrayId, name );

	scrVarNodes[child].status |= (unsigned int)scrVmPub_top[1].type;
	scrVarNodes[child].u = scrVmPub_top[1].u;
}

/* ---- SetEntityFieldValue  0x0047E0C0 ---- VERIFIED */
/* ---- GetEntityFieldValue  0x0047E1E0 ---- */
void SetEntityFieldValue__FUiiiP13VariableValue( unsigned int classnum, int entnum,
												 int offset, VariableValue *value ) {
	scrVmPub_top = value;
	scrVmPub_outparamcount = 1;

	SCR_SET_ENTITY_FIELD( classnum, entnum, offset );

	VM_ReleaseInParams();
}

void GetEntityFieldValue__FUiiiP13VariableValue( unsigned int classnum, int entnum,
												 int offset, VariableValue *value ) {
	value->type = VAR_UNDEFINED;
	scrVmPub_top = value - 1;

	SCR_GET_ENTITY_FIELD( classnum, entnum, offset );

	scrVmPub_inparamcount = 0;
}

/* ---- Scr_SetDynamicEntityField  0x0047E210 ---- VERIFIED */
void Scr_SetDynamicEntityField( int entnum, int classnum, unsigned short name ) {
	unsigned short entityId = Scr_GetEntityId( entnum, classnum );

	SetVariableFieldValue( GetVariableField( entityId, name ), scrVmPub_top );
	scrVmPub_top--;
	scrVmPub_inparamcount = 0;
}

/* ---- VM_ArchiveStack  0x0047AEC0 ---- */
void VM_ArchiveStack( int count, const char *codePos, unsigned short thread,
					  unsigned short currentObject, VariableValue *stackBase,
					  unsigned int time ) {
	unsigned short          blockIndex;
	VariableStackBuffer    *buf;
	unsigned char          *out;
	VariableValue          *src;
	unsigned short          child;
	int                     depth;
	int                     i;

	blockIndex = MT_AllocIndex( SCR_STACK_BUFFER_SIZE( count ), SCR_STACK_MT_TYPE );
	buf = (VariableStackBuffer *)&scrMemTree_blocks[blockIndex];

	AddRefToObject( currentObject );

	buf->time       = time;
	buf->codePos    = codePos;
	buf->valueCount = (unsigned short)count;
	buf->thread     = thread;

	depth = scrVmGlob_funcCount - 1;

	out = buf->values;
	src = stackBase;
	for ( i = 0; i < count; i++ ) {
		src++;
		*out++ = (unsigned char)src->type;
		memcpy( out, &src->u, 4 );
		out += 4;
		if ( src->type == VAR_CODEPOS ) {
			depth--;
		}
	}
	scrVmGlob_funcCount = depth;

	child = GetVariable( currentObject, SCR_NAME_STACK );
	scrVarNodes[child].status |= (unsigned int)VAR_STACK;
	scrVarNodes[child].u.pointerValue = buf;
}

void VM_ArchiveStack__FiPCcUsUsP13VariableValueUi( int count, const char *codePos,
		unsigned short thread, unsigned short currentObject,
		VariableValue *stackBase, unsigned int time ) {
	VM_ArchiveStack( count, codePos, thread, currentObject, stackBase, time );
}

/* ---- VM_TerminateStackInternal  0x0047AF80 ---- */
void VM_TerminateStackInternal( unsigned short id, const VariableStackBuffer *buf ) {
	unsigned short          handle;
	const unsigned char    *p;
	int                     count;
	int                     archived;
	unsigned short          thread;
	VariableValueInternal  *doomed;
	VariableUnion           u;
	int                     type;

	handle = FindVariableIndexInternal( id, SCR_NAME_STACK );
	doomed = &scrVarNodes[ scrVarIndirections[handle].id ];
	MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[id] );
	FreeValueInternal( doomed );
	RemoveRefToObject( id );

	archived = buf->valueCount;
	count    = archived;
	thread   = buf->thread;
	p        = buf->values + SCR_STACK_ENTRY_SIZE * archived;

	KillThread( thread );
	RemoveRefToObject( thread );

	while ( count > 0 ) {
		p -= 4;
		memcpy( &u, p, 4 );
		p -= 1;
		type = *p;
		count--;

		if ( type == VAR_CODEPOS ) {
			p -= 4;
			memcpy( &u, p, 4 );
			p -= 1;
			count--;
			KillThread( u.halfword[0] );
			RemoveRefToObject( u.halfword[0] );
		} else {
			RemoveRefToValue( type, u );
		}
	}

	MT_FreeIndex( (unsigned short)( ( (const scrMemBlock_t *)buf ) - scrMemTree_blocks ),
				  SCR_STACK_BUFFER_SIZE( archived ) );
}

void VM_TerminateStackInternal__FUsPC19VariableStackBuffer( unsigned short id,
		const VariableStackBuffer *buf ) {
	VM_TerminateStackInternal( id, buf );
}

/* ---- VM_TerminateStack  0x0047B1A0 ---- */
void VM_TerminateStack( unsigned short id ) {
	unsigned short child = FindVariable( id, SCR_NAME_STACK );

	if ( child != 0 ) {
		VM_TerminateStackInternal( id, (const VariableStackBuffer *)
								   scrVarNodes[child].u.pointerValue );
		return;
	}
	KillThread( id );
	RemoveRefToObject( id );
}

void VM_TerminateStack__FUs( unsigned short id ) { VM_TerminateStack( id ); }

/* ---- VM_CancelNotify  0x0047B1F0 ---- */
void VM_CancelNotify( unsigned short startLocalId, unsigned short notifyListId ) {
	unsigned short  name;
	unsigned short  waitRoot;
	unsigned short  bucket;

	name = GetThreadNotifyName( startLocalId );
	ClearThreadNotifyName( startLocalId );

	waitRoot = scrVarNodes[ FindVariable( notifyListId, SCR_NAME_NOTIFY_LIST ) ].u.halfword[0];
	bucket   = scrVarNodes[ FindVariable( waitRoot, name ) ].u.halfword[0];

	RemoveObjectVariable( bucket, startLocalId );

	if ( GetArraySize( bucket ) == 0 ) {
		RemoveVariable( waitRoot, name );
		if ( GetArraySize( waitRoot ) == 0 ) {
			RemoveVariable( notifyListId, SCR_NAME_NOTIFY_LIST );
		}
	}
}

void VM_CancelNotify__FUsUs( unsigned short startLocalId, unsigned short notifyListId ) {
	VM_CancelNotify( startLocalId, notifyListId );
}

/* ---- VM_Terminate  0x0047B2A0 ---- */
void VM_Terminate( unsigned short id ) {
	unsigned short          stackChild;
	VariableStackBuffer    *buf;
	unsigned short          self;
	unsigned short          pauseSlot;
	unsigned short          pauseBucket;
	unsigned short          waitEntry;
	unsigned short          timeBucket;
	unsigned int            time;

	stackChild = FindVariable( id, SCR_NAME_STACK );
	if ( stackChild == 0 ) {
		return;
	}
	buf  = (VariableStackBuffer *)scrVarNodes[stackChild].u.pointerValue;
	self = GetSelf( id );

	pauseSlot = FindVariable( scrVarPub_pauseArrayId,
							  (unsigned int)self + VAR_NAME_OBJECT_BASE );
	if ( pauseSlot != 0 ) {
		pauseBucket = scrVarNodes[pauseSlot].u.halfword[0];
		waitEntry = FindVariable( pauseBucket, (unsigned int)id + VAR_NAME_OBJECT_BASE );
	} else {
		pauseBucket = 0;
		waitEntry = 0;
	}

	if ( waitEntry != 0 ) {
		VM_CancelNotify( id, scrVarNodes[waitEntry].u.halfword[0] );
		AddRefToObject( id );
		RemoveObjectVariable( pauseBucket, id );
		if ( GetArraySize( pauseBucket ) == 0 ) {
			RemoveObjectVariable( scrVarPub_pauseArrayId, self );
		}
	} else {
		time = buf->time;
		timeBucket = scrVarNodes[ FindVariable( scr_timeArrayId, time ) ].u.halfword[0];
		AddRefToObject( id );
		RemoveObjectVariable( timeBucket, id );
		if ( GetArraySize( timeBucket ) == 0 && time != scrVarPub_time ) {
			RemoveVariable( scr_timeArrayId, time );
		}
	}

	VM_TerminateStackInternal( id, buf );
}

void VM_Terminate__FUs( unsigned short id ) { VM_Terminate( id ); }

/* ---- VM_TerminateNotifyList  0x0047BDC0 ---- */
void VM_TerminateNotifyList( unsigned short id ) {
	unsigned short  root;
	unsigned short  waitRoot;
	unsigned short  bucketEntry;
	unsigned short  bucket;
	unsigned short  threadEntry;
	unsigned short  thread;
	unsigned short  stackChild;

	for ( ;; ) {
		root = FindVariable( id, SCR_NAME_NOTIFY_LIST );
		if ( root == 0 ) {
			return;
		}
		waitRoot = scrVarNodes[root].u.halfword[0];

		bucketEntry = FindNextSibling( waitRoot );
		if ( bucketEntry == 0 ) {
			return;
		}
		bucket = scrVarNodes[bucketEntry].u.halfword[0];

		threadEntry = FindNextSibling( bucket );
		if ( threadEntry == 0 ) {
			return;
		}
		thread = VAR_NAMEOF( threadEntry ) - VAR_NAME_OBJECT_BASE;

		AddRefToObject( thread );
		stackChild = FindVariable( thread, SCR_NAME_STACK );
		if ( stackChild != 0 ) {
			VM_TerminateStackInternal( thread, (const VariableStackBuffer *)
									   scrVarNodes[stackChild].u.pointerValue );
		} else {
			KillThread( thread );
			RemoveRefToObject( thread );
		}
	}
}

void VM_TerminateNotifyList__FUs( unsigned short id ) { VM_TerminateNotifyList( id ); }

/* ---- VM_TerminateTime  0x0047BF10 ---- */
void VM_TerminateTime( unsigned short bucketId ) {
	unsigned short          entry;
	unsigned short          thread;
	unsigned short          handle;
	unsigned short          stackChild;
	VariableValueInternal  *doomed;

	AddRefToObject( bucketId );

	for ( ;; ) {
		entry = FindNextSibling( bucketId );
		if ( entry == 0 ) {
			break;
		}
		thread = VAR_NAMEOF( entry ) - VAR_NAME_OBJECT_BASE;
		AddRefToObject( thread );

		handle = FindVariableIndexInternal( bucketId,
						(unsigned int)thread + VAR_NAME_OBJECT_BASE );
		doomed = &scrVarNodes[ scrVarIndirections[handle].id ];
		MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[bucketId] );
		FreeValueInternal( doomed );

		stackChild = FindVariable( thread, SCR_NAME_STACK );
		if ( stackChild != 0 ) {
			VM_TerminateStackInternal( thread, (const VariableStackBuffer *)
									   scrVarNodes[stackChild].u.pointerValue );
		} else {
			KillThread( thread );
			RemoveRefToObject( thread );
		}
	}

	RemoveRefToObject( bucketId );
}

void VM_TerminateTime__FUs( unsigned short id ) { VM_TerminateTime( id ); }

/* ---- VM_Resume  0x0047C090 ---- */
void VM_Resume( unsigned short bucketId ) {
	VariableValue          *stackTop;
	VariableStackBuffer    *buf;
	unsigned short          entry;
	unsigned short          thread;
	unsigned short          innerThread;
	unsigned short          handle;
	unsigned short          stackChild;
	unsigned short          result;
	VariableValueInternal  *doomed;
	const unsigned char    *p;
	const char             *codePos;
	int                     depth;
	int                     i;
	int                     count;

	scrVmGlob_loopTick = __rdtsc();
	AddRefToObject( bucketId );

	for ( ;; ) {
		entry = FindNextSibling( bucketId );
		if ( entry == 0 ) {
			break;
		}
		thread = VAR_NAMEOF( entry ) - VAR_NAME_OBJECT_BASE;

		handle = FindVariableIndexInternal( bucketId,
						(unsigned int)thread + VAR_NAME_OBJECT_BASE );
		doomed = &scrVarNodes[ scrVarIndirections[handle].id ];
		MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[bucketId] );
		FreeValueInternal( doomed );

		stackChild = FindVariable( thread, SCR_NAME_STACK );
		buf = (VariableStackBuffer *)scrVarNodes[stackChild].u.pointerValue;
		handle = FindVariableIndexInternal( thread, SCR_NAME_STACK );
		doomed = &scrVarNodes[ scrVarIndirections[handle].id ];
		MakeVariableExternal( &scrVarIndirections[handle], &scrVarNodes[thread] );
		FreeValueInternal( doomed );

		innerThread = buf->thread;
		codePos     = buf->codePos;
		count       = buf->valueCount;

		stackTop = scrVmPub_stack;
		depth    = scrVmGlob_funcCount;
		p        = buf->values;
		for ( i = 0; i < count; i++ ) {
			stackTop++;
			stackTop->type = *p++;
			memcpy( &stackTop->u, p, 4 );
			p += 4;
			if ( stackTop->type == VAR_CODEPOS ) {
				scrVmGlob_callStack[depth++] = (const char *)stackTop->u.codePosValue;
			}
		}
		scrVmGlob_callStack[depth] = codePos;
		scrVmGlob_funcCount = depth + 1;

		MT_FreeIndex( (unsigned short)( ( (scrMemBlock_t *)buf ) - scrMemTree_blocks ),
					  SCR_STACK_BUFFER_SIZE( count ) );

		if ( !IsFieldObject( GetSelf( innerThread ) ) ) {
			for ( ;; ) {
				KillThread( innerThread );
				RemoveRefToObject( innerThread );

				while ( stackTop->type != VAR_CODEPOS ) {
					RemoveRefToValue( stackTop->type, stackTop->u );
					stackTop--;
				}
				scrVmGlob_funcCount--;
				if ( stackTop == scrVmPub_stack ) {
					break;
				}
				stackTop--;
				innerThread = stackTop->u.halfword[0];
				stackTop--;
			}
		} else {
			result = VM_Execute( stackTop, codePos, innerThread,
								 thread, scrVmPub_stack );
			RemoveRefToObject( result );
			RemoveRefToValue( scrVmPub_stack[1].type, scrVmPub_stack[1].u );
		}
	}

	RemoveRefToObject( bucketId );
	ClearVariableValue( scrVarPub_tempVariable );
}

void VM_Resume__FUs( unsigned short id ) { VM_Resume( id ); }

/* ---- VM_Notify  0x0047B430 ---- VERIFIED */
void VM_Notify( unsigned short objectId, unsigned short name, VariableValue *params ) {
	unsigned short          root;
	unsigned short          waitRoot;
	unsigned short          bucketEntry;
	unsigned short          bucket;
	unsigned short          entry;
	unsigned short          thread;
	unsigned short          self;
	unsigned short          pauseBucket;
	unsigned short          stackChild;
	VariableStackBuffer    *buf;
	VariableValue          *p;
	VariableValue           archived;
	VariableValue           supplied;
	const unsigned char    *pat;
	unsigned char          *out;
	int                     patCount;
	int                     matched;
	int                     extra;
	int                     oldCount;
	int                     newCount;
	int                     i;

	root = FindVariable( objectId, SCR_NAME_NOTIFY_LIST );
	if ( root == 0 ) {
		return;
	}
	waitRoot = scrVarNodes[root].u.halfword[0];

	bucketEntry = FindVariable( waitRoot, name );
	if ( bucketEntry == 0 ) {
		return;
	}
	bucket = scrVarNodes[bucketEntry].u.halfword[0];
	AddRefToObject( bucket );

	entry = FindNextSibling( bucket );
	while ( entry != 0 ) {
		thread = VAR_NAMEOF( entry ) - VAR_NAME_OBJECT_BASE;
		self   = GetSelf( thread );
		pauseBucket = scrVarNodes[ FindObjectVariable( scrVarPub_pauseArrayId, self ) ]
						.u.halfword[0];

		stackChild = FindVariable( thread, SCR_NAME_STACK );
		if ( stackChild == 0 ) {
			ClearThreadNotifyName( thread );
			AddRefToObject( thread );
			RemoveObjectVariable( bucket, thread );
			if ( GetArraySize( bucket ) == 0 ) {
				RemoveVariable( waitRoot, name );
				if ( GetArraySize( waitRoot ) == 0 ) {
					RemoveVariable( objectId, SCR_NAME_NOTIFY_LIST );
				}
			}
			RemoveObjectVariable( pauseBucket, thread );
			if ( GetArraySize( pauseBucket ) == 0 ) {
				RemoveObjectVariable( scrVarPub_pauseArrayId, self );
			}
			AddRefToObject( self );
			KillThread( thread );
			RemoveRefToObject( thread );
			VM_Terminate( self );
			RemoveRefToObject( self );

			entry = FindNextSibling( bucket );
			continue;
		}

		buf = (VariableStackBuffer *)scrVarNodes[stackChild].u.pointerValue;
		matched = 1;
		extra   = 0;

		if ( buf->codePos[-1] == 79  ) {
			patCount = buf->codePos[0];
			pat = buf->values + SCR_STACK_ENTRY_SIZE * ( buf->valueCount - patCount );
			p = params;

			for ( i = 0; i < patCount; i++ ) {
				if ( p->type == VAR_CODEPOS ) {
					matched = 0;
					break;
				}
				archived.type = *pat++;
				if ( archived.type == VAR_CODEPOS ) {
					break;
				}
				memcpy( &archived.u, pat, 4 );
				pat += 4;
				AddRefToValue( archived.type, archived.u );

				supplied = *p;
				AddRefToValue( supplied.type, supplied.u );

				if ( !CastWeakerPair( &supplied, &archived ) ) {
					RuntimeError( (char *)scrVmGlob_dialog_error_message,
								  (unsigned int)buf->codePos, 0,
								  scrVarPub_error_message );
					scrVarPub_error_message = NULL;
					scrVmGlob_dialog_error_message = NULL;
					matched = 0;
					break;
				}
				if ( supplied.u.intValue == 0 ) {
					matched = 0;
					break;
				}
				p -= 1;
			}
			if ( matched ) {
				buf->codePos++;
			}
			extra = 0;
		} else {
			extra = ( params[0].type != VAR_CODEPOS );
		}

		if ( !matched ) {
			entry = FindNextSibling( entry );
			continue;
		}

		ClearThreadNotifyName( thread );
		RemoveObjectVariable( bucket, thread );
		if ( GetArraySize( bucket ) == 0 ) {
			RemoveVariable( waitRoot, name );
			if ( GetArraySize( waitRoot ) == 0 ) {
				RemoveVariable( objectId, SCR_NAME_NOTIFY_LIST );
			}
		}

		buf->time = scrVarPub_time;
		GetObjectVariable( GetArray( GetVariable( scr_timeArrayId, scrVarPub_time ) ),
						   thread );

		RemoveObjectVariable( pauseBucket, thread );
		if ( GetArraySize( pauseBucket ) == 0 ) {
			RemoveObjectVariable( scrVarPub_pauseArrayId, self );
		}

		if ( extra ) {
			oldCount = buf->valueCount;
			newCount = oldCount;
			p = params;
			while ( p->type != VAR_CODEPOS ) {
				newCount++;
				p -= 1;
			}

			if ( !MT_Realloc( SCR_STACK_BUFFER_SIZE( oldCount ),
							  SCR_STACK_BUFFER_SIZE( newCount ) ) ) {
				VariableStackBuffer *grown =
					(VariableStackBuffer *)MT_Alloc( SCR_STACK_BUFFER_SIZE( newCount ),
									 SCR_STACK_MT_TYPE );

				grown->time       = buf->time;
				grown->codePos    = buf->codePos;
				grown->thread     = buf->thread;
				Com_Memcpy( grown->values, buf->values,
							SCR_STACK_ENTRY_SIZE * oldCount );
				MT_Free( buf, SCR_STACK_BUFFER_SIZE( oldCount ) );
				buf = grown;
				scrVarNodes[stackChild].u.pointerValue = buf;
			}
			buf->valueCount = (unsigned short)newCount;

			out = buf->values + SCR_STACK_ENTRY_SIZE * oldCount;
			for ( i = oldCount; i < newCount; i++ ) {
				p += 1;
				AddRefToValue( p->type, p->u );
				*out++ = (unsigned char)p->type;
				memcpy( out, &p->u, 4 );
				out += 4;
			}
		}

		entry = FindNextSibling( bucket );
	}

	RemoveRefToObject( bucket );
}

void VM_Notify__FUsUsP13VariableValue( unsigned short objectId, unsigned short name,
									   VariableValue *params ) {
	VM_Notify( objectId, name, params );
}

/* ---- Scr_NotifyId  0x0047B890 ---- */
void Scr_NotifyId( unsigned short id, unsigned short name, unsigned int paramCount ) {
	VariableValue  *base;
	int             savedType;
	int             savedIn;

	VM_ReleaseInParams();

	base    = scrVmPub_top - paramCount;
	savedIn = scrVmPub_inparamcount - (int)paramCount;

	savedType  = base->type;
	base->type = VAR_CODEPOS;
	scrVmPub_inparamcount = 0;

	VM_Notify( id, name, scrVmPub_top );

	base->type = savedType;
	while ( scrVmPub_top != base ) {
		RemoveRefToValue( scrVmPub_top->type, scrVmPub_top->u );
		scrVmPub_top--;
	}
	scrVmPub_inparamcount = savedIn;
}

/* ---- Scr_NotifyNum  0x0047BB20 ---- */
void Scr_NotifyNum( int entnum, int classnum, unsigned short name,
					unsigned int paramCount ) {
	VariableValue  *base;
	unsigned short  id;
	int             savedType;
	int             savedIn;

	VM_ReleaseInParams();

	base    = scrVmPub_top - paramCount;
	savedIn = scrVmPub_inparamcount - (int)paramCount;

	id = FindEntityId( entnum, classnum );
	if ( id != 0 ) {
		savedType  = base->type;
		base->type = VAR_CODEPOS;
		scrVmPub_inparamcount = 0;
		VM_Notify( id, name, scrVmPub_top );
		base->type = savedType;
	}

	while ( scrVmPub_top != base ) {
		RemoveRefToValue( scrVmPub_top->type, scrVmPub_top->u );
		scrVmPub_top--;
	}
	scrVmPub_inparamcount = savedIn;
}

/* ---- VM_SetTime  0x0047CB90 ---- */
void VM_SetTime( void ) {
	unsigned short child;

	if ( scr_timeArrayId == 0 ) {
		return;
	}
	child = FindVariable( scr_timeArrayId, scrVarPub_time );
	if ( child == 0 ) {
		return;
	}
	VM_Resume( scrVarNodes[child].u.halfword[0] );
	SafeRemoveVariable( scr_timeArrayId, scrVarPub_time );
}

/* ---- Scr_RunCurrentThreads  0x0047E340 ---- */
void Scr_RunCurrentThreads( void ) {
	VM_SetTime();
}

/* ---- Scr_SetTime  0x0047E250 ---- */
void Scr_SetTime( int time ) {
	unsigned int    target = (unsigned int)time & SCR_TIME_MASK;
	unsigned short  child;

	if ( (int)( target - scrVarPub_time ) <= 0 ) {
		scrVarPub_time = target;
		return;
	}

	do {
		if ( scr_timeArrayId != 0 ) {
			child = FindVariable( scr_timeArrayId, scrVarPub_time );
			if ( child != 0 ) {
				VM_Resume( scrVarNodes[child].u.halfword[0] );
				SafeRemoveVariable( scr_timeArrayId, scrVarPub_time );
			}
		}
		scrVarPub_time = ( scrVarPub_time + 1 ) & SCR_TIME_MASK;
	} while ( scrVarPub_time != target );
}

/* ---- Scr_ResetTimeout  0x0047E350 ---- */
void Scr_ResetTimeout( void ) {
	scrVmGlob_loopTick = __rdtsc();
}

typedef enum {
	OP_End                          = 0,
	OP_Return                       = 1,
	OP_GetUndefined                 = 2,
	OP_GetInteger                   = 3,
	OP_GetFloat                     = 4,
	OP_GetString                    = 5,
	OP_GetIString                   = 6,
	OP_GetSelfObject                = 7,
	OP_GetLevelObject               = 8,
	OP_GetAnimObject                = 9,
	OP_GetSelf                      = 10,
	OP_GetLevel                     = 11,
	OP_GetGame                      = 12,
	OP_GetAnim                      = 13,
	OP_GetAnimation                 = 14,
	OP_GetGameRef                   = 15,
	OP_GetFunction                  = 16,
	OP_GetLocal                     = 17,
	OP_SetLocalRef                  = 18,
	OP_ClearLocal                   = 19,
	OP_EvalArray                    = 20,
	OP_EvalArrayRef                 = 21,
	OP_EvalArrayRefNoCreate         = 22,
	OP_ClearArray                   = 23,
	OP_EmptyArray                   = 24,
	OP_GetField                     = 25,
	OP_SetFieldRef                  = 26,
	OP_ClearField                   = 27,
	OP_StoreTemp                    = 28,
	OP_SetLocal                     = 29,
	OP_SetLocalAndClear             = 30,
	OP_ClearParams                  = 31,
	OP_CheckClearParams             = 32,
	OP_StoreRef                     = 33,
	OP_CallBuiltin                  = 34,
	OP_CallBuiltinMethod            = 35,
	OP_Wait                         = 36,
	OP_ThreadMarker                 = 37,
	OP_CallFunction                 = 38,
	OP_CallPointer                  = 39,
	OP_MethodCallFunction           = 40,
	OP_MethodCallPointer            = 41,
	OP_ThreadCallFunction           = 42,
	OP_ThreadCallPointer            = 43,
	OP_MethodThreadCallFunction     = 44,
	OP_MethodThreadCallPointer      = 45,
	OP_DecTop                       = 46,
	OP_CastFieldObject              = 47,
	OP_CastBool                     = 48,
	OP_CastInt                      = 49,
	OP_CastFloat                    = 50,
	OP_CastString                   = 51,
	OP_BoolNot                      = 52,
	OP_BitNot                       = 53,
	OP_JumpOnFalse                  = 54,
	OP_JumpOnTrueBack               = 55,
	OP_JumpOnFalseExpr              = 56,
	OP_JumpOnTrueExpr               = 57,
	OP_Jump                         = 58,
	OP_JumpBack                     = 59,
	OP_Inc                          = 60,
	OP_Dec                          = 61,
	OP_BitOr                        = 62,
	OP_BitXor                       = 63,
	OP_BitAnd                       = 64,
	OP_Equality                     = 65,
	OP_Inequality                   = 66,
	OP_Less                         = 67,
	OP_Greater                      = 68,
	OP_LessEqual                    = 69,
	OP_GreaterEqual                 = 70,
	OP_ShiftLeft                    = 71,
	OP_ShiftRight                   = 72,
	OP_Plus                         = 73,
	OP_Minus                        = 74,
	OP_Multiply                     = 75,
	OP_Divide                       = 76,
	OP_Modulus                      = 77,
	OP_SizeOf                       = 78,
	OP_WaitTillMatch                = 79,
	OP_WaitTill                     = 80,
	OP_Notify                       = 81,
	OP_EndOn                        = 82,
	OP_VoidCodePos                  = 83,
	OP_Switch                       = 84,
	OP_EndSwitch                    = 85,
	OP_Vector                       = 86,
	OP_NOP                          = 87,
	OP_DevBlockBegin                = 88,
	OP_DevBlockEnd                  = 89
} scrOpcode_t;

typedef void (*scrBuiltinFunc_t)( void );
typedef void (*scrBuiltinMethod_t)( int entnum );

#define SCR_OPERAND_CODEPOS     4
#define SCR_OPERAND_CALL        8
#define SCR_OPERAND_POINTERCALL 4
#define SCR_SWITCH_ENTRY        8

static int VM_ReadI32( const char *p ) {
	int v;
	memcpy( &v, p, 4 );
	return v;
}

static unsigned int VM_ReadU32( const char *p ) {
	unsigned int v;
	memcpy( &v, p, 4 );
	return v;
}

static unsigned short VM_ReadU16( const char *p ) {
	unsigned short v;
	memcpy( &v, p, 2 );
	return v;
}

static short VM_ReadI16( const char *p ) {
	short v;
	memcpy( &v, p, 2 );
	return v;
}

static unsigned short VM_RequireObject( VariableValue *value, int paramIndex ) {
	unsigned short id;

	if ( value->type != VAR_OBJECT ) {
		scrVarPub_error_index = paramIndex;
		Scr_Error( va( "%s is not an object", var_typename[value->type] ) );
		return 0;
	}
	id = value->u.halfword[0];
	if ( !IsFieldObject( id ) ) {
		scrVarPub_error_index = paramIndex;
		Scr_Error( va( "%s is not an object", var_typename[ VAR_TYPEOF( id ) ] ) );
		return 0;
	}
	return id;
}

static void VM_CheckCallDepth( void ) {
	if ( scrVmGlob_funcCount >= SCR_VM_MAX_CALL_DEPTH ) {
		scrVarPub_error_index = 1;
		Scr_Error( "script stack overflow (too many embedded function calls)" );
	}
}

/* ---- VM_Execute  0x00476370 ---- */
static unsigned short VM_Execute( VariableValue *stackTopIn, const char *codePosIn,
								  unsigned short threadIn, unsigned short currentObject,
								  VariableValue *stackBase ) {
	VariableValue * volatile stackTop = stackTopIn;
	const char * volatile   codePos  = codePosIn;
	volatile unsigned short thread   = threadIn;
	volatile unsigned short fieldRef = 0;
	volatile int            devDepth = 0;
	volatile int            opcode   = OP_NOP;

	jmp_buf                 frame;
	jmp_buf * volatile      savedFrame = scrVmErrorFrame;

	VariableValue          *top;
	VariableValue          *left;
	VariableValue           value;
	const char             *target;
	const char             *table;
	unsigned short          child;
	unsigned short          object;
	unsigned short          parent;
	unsigned short          notifyName;
	unsigned short          nested;
	unsigned int            u;
	int                     argCount;
	int                     i;
	int                     n;
	float                   f;

	scrVmErrorFrame = &frame;

	for ( ;; ) {
		if ( setjmp( frame ) != 0 ) {
			top = (VariableValue *)stackTop;

			switch ( opcode ) {
			case OP_EvalArray:
				RemoveRefToValue( top->type, top->u );
				RemoveRefToValue( top[1].type, top[1].u );
				top->type = VAR_UNDEFINED;
				break;

			case OP_EvalArrayRef:
			case OP_EvalArrayRefNoCreate:
				ClearVariableValue( scrVarPub_tempVariable );
				fieldRef = scrVarPub_tempVariable;
			case OP_ClearArray:
				if ( scrVarPub_error_index < 0 ) {
					scrVarPub_error_index = 1;
				}
				RemoveRefToValue( top->type, top->u );
				top--;
				break;

			case OP_GetField:
			case OP_ClearField:
				scrVarPub_error_index = 0;
				codePos += 2;
				break;

			case OP_SetLocal:
			case OP_SetLocalAndClear:
				VM_ReleaseInParams();
				scrVarPub_error_index = 0;
				top--;
				codePos += 2;
				break;

			case OP_StoreRef:
				VM_ReleaseInParams();
				scrVarPub_error_index = 0;
				top--;
				break;

			case OP_CheckClearParams:
				top = VM_ReleaseToMarker( top );
				break;

			case OP_CallBuiltin:
				if ( scrVarPub_error_index > 0 ) {
					scrVarPub_error_index =
						scrVmPub_outparamcount - scrVarPub_error_index + 1;
				}
				codePos += 1 + sizeof( scrBuiltinFunc_t );
				VM_ReleaseInParams();
				top = scrVmPub_top + 1;
				top->type = VAR_UNDEFINED;
				break;

			case OP_CallBuiltinMethod:
				if ( scrVarPub_error_index < 0 ) {
					scrVarPub_error_index = 1;
				} else if ( scrVarPub_error_index > 0 ) {
					scrVarPub_error_index =
						scrVmPub_outparamcount - scrVarPub_error_index + 2;
				}
				codePos += 1 + sizeof( scrBuiltinMethod_t );
				VM_ReleaseInParams();
				top = scrVmPub_top + 1;
				top->type = VAR_UNDEFINED;
				break;

			case OP_Wait:
				scrVarPub_error_index = 1;
				top--;
				break;

			case OP_CallFunction:
			case OP_MethodCallFunction:
				argCount = VM_ReadI32( codePos + SCR_OPERAND_CODEPOS );
				for ( i = 0; i < argCount; i++ ) {
					RemoveRefToValue( top->type, top->u );
					top--;
				}
				if ( opcode == OP_MethodCallFunction ) {
					RemoveRefToValue( top->type, top->u );
					top--;
				}
				top->type = VAR_UNDEFINED;
				codePos += SCR_OPERAND_CALL;
				break;

			case OP_CallPointer:
			case OP_MethodCallPointer:
			case OP_ThreadCallPointer:
			case OP_MethodThreadCallPointer:
				argCount = VM_ReadI32( codePos ) + 1;
				for ( i = 0; i < argCount; i++ ) {
					RemoveRefToValue( top->type, top->u );
					top--;
				}
				top++;
				top->type = VAR_UNDEFINED;
				codePos += SCR_OPERAND_POINTERCALL;
				break;

			case OP_MethodThreadCallFunction:
				argCount = VM_ReadI32( codePos + SCR_OPERAND_CODEPOS ) + 1;
				for ( i = 0; i < argCount; i++ ) {
					RemoveRefToValue( top->type, top->u );
					top--;
				}
				top++;
				top->type = VAR_UNDEFINED;
				codePos += SCR_OPERAND_CALL;
				break;

			case OP_CastFieldObject:
				RemoveRefToValue( top->type, top->u );
				ClearVariableValue( scrVarPub_tempVariable );
				fieldRef = GetObject( scrVarPub_tempVariable );
				top--;
				break;

			case OP_BitNot:
			case OP_SizeOf:
				RemoveRefToValue( top->type, top->u );
				top->type = VAR_UNDEFINED;
				break;

			case OP_JumpOnFalse:
			case OP_JumpOnTrueBack:
			case OP_JumpOnFalseExpr:
			case OP_JumpOnTrueExpr:
				top--;
				codePos += 4;
				break;

			case OP_Inc:
			case OP_Dec:
				scrVarPub_error_index = 0;
				break;

			case OP_WaitTillMatch:
				codePos += 1;
			case OP_WaitTill:
			case OP_EndOn:
				RemoveRefToValue( top->type, top->u );
				RemoveRefToValue( top[-1].type, top[-1].u );
				top -= 2;
				break;

			case OP_Notify:
				top = VM_ReleaseToMarker( top );
				top--;
				break;

			case OP_Switch:
				RemoveRefToValue( top->type, top->u );
				top--;
				break;

			case OP_Vector:
				ClearVector( top - 2 );
				top -= 2;
				break;

			default:
				break;
			}

			stackTop = top;

			RuntimeError( (char *)scrVmGlob_dialog_error_message,
						  (unsigned int)codePos, scrVarPub_error_index,
						  scrVarPub_error_message );
			scrVarPub_error_message = NULL;
			scrVmGlob_dialog_error_message = NULL;
			scrVarPub_error_index = 0;
			continue;
		}

		opcode = (unsigned char)*codePos++;

	dispatch:
		top = (VariableValue *)stackTop;

		switch ( opcode ) {

		case OP_End:
		case OP_Return:
			if ( opcode == OP_Return ) {
				value = *top;
				top--;
			} else {
				value.type = VAR_UNDEFINED;
				value.u.intValue = 0;
			}
			KillThread( thread );

			for ( ;; ) {
				top = VM_ReleaseToMarker( top );
				scrVmGlob_funcCount--;

				if ( top == stackBase ) {
					stackBase[1] = value;
					stackTop = top;
					scrVmErrorFrame = savedFrame;
					return thread;
				}

				RemoveRefToObject( thread );
				codePos = top->u.codePosValue;
				top--;
				thread = top->u.halfword[0];
				*top = value;

				if ( IsFieldObject( GetSelf( thread ) ) ) {
					break;
				}
				KillThread( thread );
				value.type = VAR_UNDEFINED;
				value.u.intValue = 0;
			}
			stackTop = top;
			break;

		case OP_GetUndefined:
			top++;
			top->type = VAR_UNDEFINED;
			top->u.intValue = 0;
			stackTop = top;
			break;

		case OP_GetInteger:
			top++;
			top->type = VAR_INTEGER;
			top->u.uintValue = VM_ReadU32( codePos );
			codePos += 4;
			stackTop = top;
			break;

		case OP_GetFloat:
			top++;
			top->type = VAR_FLOAT;
			top->u.uintValue = VM_ReadU32( codePos );
			codePos += 4;
			stackTop = top;
			break;

		case OP_GetString:
		case OP_GetIString:
			top++;
			top->type = ( opcode == OP_GetString ) ? VAR_STRING : VAR_ISTRING;
			top->u.stringValue = VM_ReadU16( codePos );
			codePos += 2;
			SL_AddRefToString( top->u.stringValue );
			stackTop = top;
			break;

		case OP_GetAnimation:
			top++;
			top->type = VAR_ANIMATION;
			top->u.uintValue = VM_ReadU32( codePos );
			codePos += 4;
			stackTop = top;
			break;

		case OP_GetFunction:
			top++;
			top->type = VAR_FUNCTION;
			top->u.codePosValue = (const char *)VM_ReadU32( codePos );
			codePos += 4;
			stackTop = top;
			break;

		case OP_GetSelfObject:
			fieldRef = GetSelf( thread );
			break;

		case OP_GetLevelObject:
			fieldRef = scrVarPub_levelId;
			break;

		case OP_GetAnimObject:
			fieldRef = scrVarPub_animId;
			break;

		case OP_GetGameRef:
			fieldRef = scrVarPub_gameId;
			break;

		case OP_GetSelf:
			parent = GetSelf( thread );
			AddRefToObject( parent );
			top++;
			top->type = VAR_OBJECT;
			top->u.halfword[0] = parent;
			stackTop = top;
			break;

		case OP_GetLevel:
			AddRefToObject( scrVarPub_levelId );
			top++;
			top->type = VAR_OBJECT;
			top->u.halfword[0] = scrVarPub_levelId;
			stackTop = top;
			break;

		case OP_GetAnim:
			AddRefToObject( scrVarPub_animId );
			top++;
			top->type = VAR_OBJECT;
			top->u.halfword[0] = scrVarPub_animId;
			stackTop = top;
			break;

		case OP_GetGame:
			top++;
			GetVariableValue( scrVarPub_gameId, top );
			AddRefToValue( top->type, top->u );
			stackTop = top;
			break;

		case OP_GetLocal:
			child = GetVariable( thread, VM_ReadU16( codePos ) );
			codePos += 2;
			top++;
			GetVariableValue( child, top );
			AddRefToValue( top->type, top->u );
			stackTop = top;
			break;

		case OP_SetLocalRef:
			fieldRef = GetVariable( thread, VM_ReadU16( codePos ) );
			codePos += 2;
			break;

		case OP_ClearLocal:
			SafeRemoveVariable( thread, VM_ReadU16( codePos ) );
			codePos += 2;
			break;

		case OP_SetLocal:
		case OP_SetLocalAndClear:
			if ( top->type == VAR_CODEPOS ) {
				if ( opcode == OP_SetLocalAndClear ) {
					SafeRemoveVariable( thread, VM_ReadU16( codePos ) );
				}
			} else {
				child = GetVariable( thread, VM_ReadU16( codePos ) );
				SetVariableFieldValue( child, top );
				top--;
				stackTop = top;
			}
			codePos += 2;
			break;

		case OP_EvalArray:
			top--;
			EvalArray( top, top + 1 );
			stackTop = top;
			break;

		case OP_EvalArrayRef:
			fieldRef = EvalArrayRef( fieldRef, top );
			top--;
			stackTop = top;
			break;

		case OP_EvalArrayRefNoCreate:
			fieldRef = Scr_EvalArrayIndex_m( fieldRef, top );
			top--;
			stackTop = top;
			break;

		case OP_ClearArray:
			ClearArray( fieldRef, top );
			top--;
			stackTop = top;
			break;

		case OP_EmptyArray:
			top++;
			GetEmptyArray( top );
			stackTop = top;
			break;

		case OP_GetField:
			child = GetVariableField( fieldRef, VM_ReadU16( codePos ) );
			codePos += 2;
			top++;
			GetVariableFieldValue( child, top );
			stackTop = top;
			break;

		case OP_SetFieldRef:
			fieldRef = GetVariableField( fieldRef, VM_ReadU16( codePos ) );
			codePos += 2;
			break;

		case OP_ClearField:
			ClearVariableField( fieldRef, VM_ReadU16( codePos ) );
			codePos += 2;
			break;

		case OP_StoreTemp:
			SetVariableValue( scrVarPub_tempVariable, top );
			fieldRef = scrVarPub_tempVariable;
			top--;
			stackTop = top;
			break;

		case OP_StoreRef:
			SetVariableFieldValue( fieldRef, top );
			top--;
			stackTop = top;
			break;

		case OP_ClearParams:
			stackTop = VM_ReleaseToMarker( top );
			break;

		case OP_CheckClearParams:
			if ( top->type != VAR_CODEPOS ) {
				Scr_Error( "function called with too many parameters" );
			}
			break;

		case OP_VoidCodePos:
			top++;
			top->type = VAR_CODEPOS;
			top->u.intValue = 0;
			stackTop = top;
			break;

		case OP_CallBuiltin: {
			scrBuiltinFunc_t fn;

			scrVmPub_outparamcount = (unsigned char)*codePos;
			scrVmPub_top = top;
			memcpy( &fn, codePos + 1, sizeof( fn ) );
			codePos += 1 + sizeof( fn );

			fn();

			VM_ReleaseInParams();
			top = scrVmPub_top;
			if ( scrVmPub_inparamcount == 0 ) {
				top++;
				top->type = VAR_UNDEFINED;
			} else {
				scrVmPub_inparamcount = 0;
			}
			stackTop = top;
			break;
		}

		case OP_CallBuiltinMethod: {
			scrBuiltinMethod_t  method;
			int                 entnum;

			scrVmPub_outparamcount = (unsigned char)*codePos;
			scrVmPub_top = top - 1;
			memcpy( &method, codePos + 1, sizeof( method ) );

			if ( top->type != VAR_OBJECT ) {
				int badType = top->type;
				RemoveRefToValue( top->type, top->u );
				scrVarPub_error_index = -1;
				Scr_Error( va( "%s is not an entity", var_typename[badType] ) );
			}
			object = top->u.halfword[0];
			if ( VAR_TYPEOF( object ) != VAR_ENTITY ) {
				int badType = VAR_TYPEOF( object );
				RemoveRefToObject( object );
				scrVarPub_error_index = -1;
				Scr_Error( va( "%s is not an entity", var_typename[badType] ) );
			}

			entnum = GetEntnum( object );
			RemoveRefToObject( object );
			codePos += 1 + sizeof( method );

			method( entnum );

			VM_ReleaseInParams();
			top = scrVmPub_top;
			if ( scrVmPub_inparamcount == 0 ) {
				top++;
				top->type = VAR_UNDEFINED;
			} else {
				scrVmPub_inparamcount = 0;
			}
			stackTop = top;
			break;
		}

		case OP_CallFunction:
		case OP_MethodCallFunction:
		case OP_CallPointer:
		case OP_MethodCallPointer: {
			const char *ret;

			RemoveRefToObject( thread );

			if ( opcode == OP_CallFunction || opcode == OP_MethodCallFunction ) {
				target   = (const char *)VM_ReadU32( codePos );
				argCount = VM_ReadI32( codePos + SCR_OPERAND_CODEPOS );
				ret      = codePos + SCR_OPERAND_CALL;
			} else {
				if ( top->type != VAR_FUNCTION ) {
					int badType = top->type;
					if ( opcode == OP_MethodCallPointer ) {
						RemoveRefToValue( top->type, top->u );
						top--;
						stackTop = top;
					}
					Scr_Error( va( "%s is not a function pointer",
								   var_typename[badType] ) );
				}
				argCount = VM_ReadI32( codePos );
				target   = top->u.codePosValue;
				ret      = codePos + SCR_OPERAND_POINTERCALL;
				top--;
			}

			if ( opcode == OP_MethodCallFunction || opcode == OP_MethodCallPointer ) {
				parent = VM_RequireObject( top,
							opcode == OP_MethodCallFunction ? 1 : 2 );
				top--;
			} else {
				parent = GetSelf( thread );
				AddRefToObject( parent );
			}

			VM_CheckCallDepth();
			scrVmGlob_callStack[ scrVmGlob_funcCount++ ] = codePos;

			thread = AllocThread( parent );

			left = top - argCount;
			left->u.codePosValue = ret;
			left->type = VAR_CODEPOS;

			codePos  = target;
			stackTop = top;
			break;
		}

		case OP_ThreadCallFunction:
		case OP_ThreadCallPointer:
		case OP_MethodThreadCallFunction:
		case OP_MethodThreadCallPointer: {
			VariableValue  *marker;
			int             savedType;

			if ( opcode == OP_ThreadCallFunction ||
				 opcode == OP_MethodThreadCallFunction ) {
				target   = (const char *)VM_ReadU32( codePos );
				argCount = VM_ReadI32( codePos + SCR_OPERAND_CODEPOS );
			} else {
				if ( top->type != VAR_FUNCTION ) {
					int badType = top->type;
					if ( opcode == OP_MethodThreadCallPointer ) {
						RemoveRefToValue( top->type, top->u );
						top--;
						stackTop = top;
					}
					Scr_Error( va( "%s is not a function pointer",
								   var_typename[badType] ) );
				}
				argCount = VM_ReadI32( codePos );
				target   = top->u.codePosValue;
				top--;
			}

			if ( opcode == OP_MethodThreadCallFunction ||
				 opcode == OP_MethodThreadCallPointer ) {
				parent = VM_RequireObject( top, 2 );
				top--;
			} else {
				parent = GetSelf( thread );
				AddRefToObject( parent );
			}

			VM_CheckCallDepth();
			scrVmGlob_callStack[ scrVmGlob_funcCount++ ] = codePos;

			marker = top - argCount;
			savedType = marker->type;
			marker->type = VAR_CODEPOS;

			nested = AllocThread( parent );
			nested = VM_Execute( top, target, nested, nested, marker );
			RemoveRefToObject( nested );

			marker->type = savedType;
			stackTop = marker + 1;

			codePos += ( opcode == OP_ThreadCallFunction ||
						 opcode == OP_MethodThreadCallFunction )
					 ? SCR_OPERAND_CALL : SCR_OPERAND_POINTERCALL;
			break;
		}

		case OP_DecTop:
			RemoveRefToValue( top->type, top->u );
			top--;
			stackTop = top;
			break;

		case OP_CastFieldObject:
			fieldRef = CastFieldObject( top );
			top--;
			stackTop = top;
			break;

		case OP_CastBool:
			if ( top->type == VAR_INTEGER ) {
				top->u.intValue = top->u.intValue != 0;
			} else if ( !CastBool( top ) ) {
				Scr_RaiseError();
			}
			break;

		case OP_CastInt:
			if ( !CastInt( top ) ) {
				Scr_RaiseError();
			}
			break;

		case OP_CastFloat:
			if ( !CastFloat( top ) ) {
				Scr_RaiseError();
			}
			break;

		case OP_CastString:
			if ( !CastString( top ) ) {
				Scr_RaiseError();
			}
			break;

		case OP_BoolNot:
			if ( top->type != VAR_INTEGER && !CastBool( top ) ) {
				Scr_RaiseError();
			}
			top->u.intValue = ( top->u.intValue == 0 );
			break;

		case OP_BitNot:
			if ( top->type != VAR_INTEGER ) {
				Scr_Error( va( "~ cannot be applied to \"%s\"",
							   var_typename[top->type] ) );
			}
			top->u.uintValue = ~top->u.uintValue;
			break;

		case OP_JumpOnFalse:
			if ( top->type != VAR_INTEGER && !CastBool( top ) ) {
				Scr_RaiseError();
			}
			if ( top->u.intValue == 0 ) {
				codePos += VM_ReadI32( codePos );
			}
			codePos += 4;
			top--;
			stackTop = top;
			break;

		case OP_JumpOnFalseExpr:
			if ( top->type != VAR_INTEGER && !CastBool( top ) ) {
				Scr_RaiseError();
			}
			if ( top->u.intValue != 0 ) {
				top--;
				stackTop = top;
				codePos += 4;
			} else {
				codePos += VM_ReadI32( codePos ) + 4;
			}
			break;

		case OP_JumpOnTrueExpr:
			if ( top->type != VAR_INTEGER && !CastBool( top ) ) {
				Scr_RaiseError();
			}
			if ( top->u.intValue != 0 ) {
				codePos += VM_ReadI32( codePos ) + 4;
			} else {
				top--;
				stackTop = top;
				codePos += 4;
			}
			break;

		case OP_JumpOnTrueBack:
			if ( top->type != VAR_INTEGER && !CastBool( top ) ) {
				scrVarPub_error_index = 1;
				Scr_RaiseError();
			}
			if ( top->u.intValue == 0 ) {
				top--;
				stackTop = top;
				codePos += 4;
				break;
			}
			top--;
			stackTop = top;

		case OP_JumpBack:
			/* retail 0x00478688: the budget is the sign bit of a 32-bit rdtsc difference -- nothing is added at any arming site; fires at 2^31 cycles, scaling inversely with clock rate. */
			if ( (int)( __rdtsc() - scrVmGlob_loopTick ) >= 0 ) {
				codePos += VM_ReadI32( codePos );
				break;
			}
			if ( scr_loading ) {
				Com_Printf( "WARNING: potential infinite loop in script.\n" );
				Scr_PrintPrevCodePos( codePos, 0 );
				codePos += VM_ReadI32( codePos );
				scrVmGlob_loopTick = __rdtsc();
				break;
			}
			if ( SCR_DEVELOPER ) {
				Scr_TerminalError( "potential infinite loop in script" );
			}
			Com_Printf( "ERROR: potential infinite loop in script - killing thread.\n" );
			Scr_PrintPrevCodePos( codePos, 0 );
			scrVmGlob_loopTick = __rdtsc();

			value.type = VAR_UNDEFINED;
			value.u.intValue = 0;
			for ( ;; ) {
				top = VM_ReleaseToMarker( top );
				scrVmGlob_funcCount--;
				if ( top == stackBase ) {
					stackBase[1] = value;
					stackTop = top;
					scrVmErrorFrame = savedFrame;
					return thread;
				}
				RemoveRefToObject( thread );
				top--;
				thread = top->u.halfword[0];
				top->type = VAR_UNDEFINED;
			}

		case OP_Jump:
			codePos += VM_ReadI32( codePos );
			break;

		case OP_Inc:
		case OP_Dec:
			top++;
			GetVariableFieldValue( fieldRef, top );
			stackTop = top;
			if ( top->type != VAR_INTEGER ) {
				Scr_Error( va( opcode == OP_Inc
					? "++ must be applied to an int (applied to %s)"
					: "-- must be applied to an int (applied to %s)",
					var_typename[top->type] ) );
			}
			top->u.intValue += ( opcode == OP_Inc ) ? 1 : -1;
			break;

		case OP_BitOr:
		case OP_BitXor:
		case OP_BitAnd:
		case OP_ShiftLeft:
		case OP_ShiftRight:
			top--;
			stackTop = top;
			left = top;
			if ( !CastWeakerPair( left, left + 1 ) || left->type != VAR_INTEGER ) {
				if ( scrVarPub_error_message == NULL ) {
					VM_UnmatchingTypesError( left, left + 1 );
				}
				Scr_RaiseError();
			}
			switch ( opcode ) {
			case OP_BitOr:
				left->u.uintValue |= left[1].u.uintValue;
				break;
			case OP_BitXor:
				left->u.uintValue ^= left[1].u.uintValue;
				break;
			case OP_BitAnd:
				left->u.uintValue &= left[1].u.uintValue;
				break;
			case OP_ShiftLeft:
				left->u.uintValue <<= ( left[1].u.uintValue & SCR_SHIFT_MASK );
				break;
			default:
				left->u.intValue >>= ( left[1].u.uintValue & SCR_SHIFT_MASK );
				break;
			}
			break;

		case OP_Equality:
			top--;
			stackTop = top;
			if ( !CheckEquality( top, top + 1 ) ) {
				Scr_RaiseError();
			}
			break;

		case OP_Inequality:
			top--;
			stackTop = top;
			left = top;
			if ( !CastWeakerPair( left, left + 1 ) ) {
				Scr_RaiseError();
			}
			switch ( left->type ) {
			case VAR_UNDEFINED:
				left->type = VAR_INTEGER;
				left->u.intValue = 0;
				break;
			case VAR_STRING:
			case VAR_ISTRING: {
				unsigned short a = left->u.stringValue;
				unsigned short b = left[1].u.stringValue;
				left->type = VAR_INTEGER;
				SL_RemoveRefToString( a );
				SL_RemoveRefToString( b );
				left->u.intValue = ( a != b );
				break;
			}
			case VAR_VECTOR: {
				const float *a = left->u.vectorValue;
				const float *b = left[1].u.vectorValue;
				int ne = ( a[0] != b[0] || a[1] != b[1] || a[2] != b[2] );
				left->type = VAR_INTEGER;
				RemoveRefToVector( a );
				RemoveRefToVector( b );
				left->u.intValue = ne;
				break;
			}
			case VAR_FLOAT:
				f = left->u.floatValue - left[1].u.floatValue;
				left->type = VAR_INTEGER;
				left->u.intValue =
					( fabs( (double)f ) >= (double)SCR_FLOAT_EQ_EPSILON );
				break;
			case VAR_INTEGER:
				left->u.intValue = ( left->u.uintValue != left[1].u.uintValue );
				break;
			case VAR_OBJECT: {
				unsigned short a = left->u.halfword[0];
				unsigned short b = left[1].u.halfword[0];
				left->type = VAR_INTEGER;
				RemoveRefToObject( a );
				RemoveRefToObject( b );
				left->u.intValue = ( a != b );
				break;
			}
			case VAR_ANIMATION:
				left->type = VAR_INTEGER;
				left->u.intValue = ( left->u.uintValue != left[1].u.uintValue );
				break;
			default:
				VM_UnmatchingTypesError( left, left + 1 );
				Scr_RaiseError();
				break;
			}
			break;

		case OP_Less:
		case OP_Greater:
		case OP_LessEqual:
		case OP_GreaterEqual:
			top--;
			stackTop = top;
			left = top;
			if ( !CastWeakerPair( left, left + 1 ) ) {
				Scr_RaiseError();
			}
			if ( left->type == VAR_FLOAT ) {
				float a = left->u.floatValue;
				float b = left[1].u.floatValue;
				n = ( opcode == OP_Less )         ? ( a <  b )
				  : ( opcode == OP_Greater )      ? ( b <  a )
				  : ( opcode == OP_LessEqual )    ? ( a <= b )
				  :                                 ( b <= a );
			} else if ( left->type == VAR_INTEGER ) {
				int a = left->u.intValue;
				int b = left[1].u.intValue;
				n = ( opcode == OP_Less )         ? ( a <  b )
				  : ( opcode == OP_Greater )      ? ( b <  a )
				  : ( opcode == OP_LessEqual )    ? ( a <= b )
				  :                                 ( b <= a );
			} else {
				VM_UnmatchingTypesError( left, left + 1 );
				Scr_RaiseError();
				n = 0;
			}
			left->type = VAR_INTEGER;
			left->u.intValue = n;
			break;

		case OP_Plus:
		case OP_Minus:
		case OP_Multiply:
		case OP_Divide:
		case OP_Modulus:
			top--;
			stackTop = top;
			left = top;
			if ( !CastWeakerPair( left, left + 1 ) ) {
				Scr_RaiseError();
			}

			if ( left->type == VAR_INTEGER ) {
				int a = left->u.intValue;
				int b = left[1].u.intValue;

				if ( ( opcode == OP_Divide || opcode == OP_Modulus ) && b == 0 ) {
					left->u.intValue = 0;
					Scr_Error( "divide by 0" );
				}
				switch ( opcode ) {
				case OP_Plus:     left->u.intValue = a + b; break;
				case OP_Minus:    left->u.intValue = a - b; break;
				case OP_Multiply: left->u.intValue = a * b; break;
				case OP_Divide:   left->u.intValue = a / b; break;
				default:          left->u.intValue = a % b; break;
				}
			} else if ( left->type == VAR_FLOAT ) {
				float a = left->u.floatValue;
				float b = left[1].u.floatValue;

				switch ( opcode ) {
				case OP_Plus:     left->u.floatValue = a + b; break;
				case OP_Minus:    left->u.floatValue = a - b; break;
				case OP_Multiply: left->u.floatValue = a * b; break;
				case OP_Divide:
					if ( b == 0.0f ) {
						left->u.floatValue = 0.0f;
						Scr_Error( "divide by 0" );
					}
					left->u.floatValue = a / b;
					break;
				default:
					VM_UnmatchingTypesError( left, left + 1 );
					Scr_RaiseError();
					break;
				}
			} else if ( left->type == VAR_VECTOR &&
						( opcode == OP_Plus || opcode == OP_Minus ) ) {
				const float *a = left->u.vectorValue;
				const float *b = left[1].u.vectorValue;
				float *out = AllocVector();

				for ( i = 0; i < 3; i++ ) {
					out[i] = ( opcode == OP_Plus ) ? a[i] + b[i] : a[i] - b[i];
				}
				RemoveRefToVector( a );
				RemoveRefToVector( b );
				left->u.vectorValue = out;
			} else if ( opcode == OP_Plus && left->type == VAR_STRING ) {
				unsigned short a = left->u.stringValue;
				unsigned short b = left[1].u.stringValue;

				left->u.stringValue = VM_ConcatenateStrings( left );
				SL_RemoveRefToString( a );
				SL_RemoveRefToString( b );
			} else {
				VM_UnmatchingTypesError( left, left + 1 );
				Scr_RaiseError();
			}
			break;

		case OP_SizeOf:
			GetSizeValue( top );
			break;

		case OP_Wait:
			if ( devDepth != 0 ) {
				Scr_Error( "wait not allowed in /# ... #/ comment "
						   "(call as a thread to fix)" );
			}
			if ( !CastFloat( top ) ) {
				Scr_RaiseError();
			}
			f = top->u.floatValue;
			if ( f < 0.0f ) {
				Scr_Error( va( "negative wait of %g is not allowed", f ) );
			}
			if ( !( f < (float)SCR_MAX_WAIT_SECONDS ) ) {
				Scr_Error( va( "wait of %.0f seconds is too long", f ) );
			}

			n = (int)( f * SCR_WAIT_SCALE + 0.5f );
			u = ( scrVarPub_time + (unsigned int)n ) & SCR_TIME_MASK;

			GetObjectVariable( GetArray( GetVariable( scr_timeArrayId, u ) ),
							   currentObject );
			VM_ArchiveStack( (int)( top - stackBase - 1 ), codePos,
							 thread, currentObject, stackBase, u );
			stackBase[1].type = VAR_UNDEFINED;
			scrVmErrorFrame = savedFrame;
			return currentObject;

		case OP_ThreadMarker:
			AddRefToObject( thread );
			top++;
			top->type = VAR_OBJECT;
			top->u.halfword[0] = thread;
			top++;
			top->type = VAR_UNDEFINED;
			stackTop = top;
			break;

		case OP_WaitTillMatch:
		case OP_WaitTill: {
			unsigned short waitRoot;
			unsigned short bucket;
			unsigned short pauseBucket;

			if ( devDepth != 0 ) {
				Scr_Error( "waittill not allowed in /# ... #/ comment "
						   "(call as a thread to fix)" );
			}
			object = VM_RequireObject( top, 2 );
			if ( top[-1].type != VAR_STRING ) {
				scrVarPub_error_index = 1;
				Scr_Error( "first parameter of waittill must evaluate to a string" );
			}
			notifyName = top[-1].u.stringValue;

			waitRoot = GetArray( GetVariable( object, SCR_NAME_NOTIFY_LIST ) );
			bucket   = GetArray( GetVariable( waitRoot, notifyName ) );
			GetObjectVariable( bucket, currentObject );

			value.type = VAR_OBJECT;
			value.u.halfword[0] = object;
			parent      = GetSelf( currentObject );
			pauseBucket = GetArray( GetObjectVariable( scrVarPub_pauseArrayId, parent ) );
			SetNewVariableValue( GetObjectVariable( pauseBucket, currentObject ), &value );

			SetThreadNotifyName( currentObject, notifyName );
			SL_RemoveRefToString( notifyName );

			VM_ArchiveStack( (int)( top - stackBase - 2 ), codePos,
							 thread, currentObject, stackBase, 0 );
			stackBase[1].type = VAR_UNDEFINED;
			scrVmErrorFrame = savedFrame;
			return currentObject;
		}

		case OP_Notify:
			object = VM_RequireObject( top, 2 );
			if ( top[-1].type != VAR_STRING ) {
				scrVarPub_error_index = 1;
				Scr_Error( "first parameter of notify must evaluate to a string" );
			}
			notifyName = top[-1].u.stringValue;

			VM_Notify( object, notifyName, top - 2 );

			top = VM_ReleaseToMarker( top );
			top--;
			stackTop = top;
			break;

		case OP_EndOn: {
			unsigned short waitRoot;
			unsigned short bucket;
			unsigned short pauseBucket;
			unsigned short endonThread;

			object = VM_RequireObject( top, 1 );
			if ( top[-1].type != VAR_STRING ) {
				Scr_Error( "first parameter of notify must evaluate to a string" );
			}
			notifyName = top[-1].u.stringValue;

			AddRefToObject( currentObject );
			endonThread = AllocThread( currentObject );

			waitRoot = GetArray( GetVariable( object, SCR_NAME_NOTIFY_LIST ) );
			bucket   = GetArray( GetVariable( waitRoot, notifyName ) );
			GetObjectVariable( bucket, endonThread );
			RemoveRefToObject( endonThread );

			value.type = VAR_OBJECT;
			value.u.halfword[0] = object;
			pauseBucket = GetArray( GetObjectVariable( scrVarPub_pauseArrayId,
													   currentObject ) );
			SetNewVariableValue( GetObjectVariable( pauseBucket, endonThread ), &value );

			SetThreadNotifyName( endonThread, notifyName );
			SL_RemoveRefToString( notifyName );

			top -= 2;
			stackTop = top;
			break;
		}

		case OP_Switch: {
			int             caseCount;
			unsigned int    key;

			table = codePos + VM_ReadI16( codePos );
			caseCount = VM_ReadI16( table - 2 );
			codePos = table;

			if ( top->type == VAR_STRING ) {
				key = top->u.stringValue;
				SL_RemoveRefToString( top->u.stringValue );
			} else if ( top->type == VAR_INTEGER ) {
				if ( !IsValidArrayIndex( top->u.uintValue ) ) {
					Scr_Error( va( "switch index %d out of range", top->u.intValue ) );
				}
				key = GetInternalVariableIndex( top->u.uintValue );
			} else {
				Scr_Error( va( "cannot switch on %s", var_typename[top->type] ) );
				key = 0;
			}
			top--;
			stackTop = top;

			while ( caseCount != 0 ) {
				unsigned int caseValue = VM_ReadU32( codePos );

				if ( caseValue == key || ( caseCount == 1 && caseValue == 0 ) ) {
					codePos = (const char *)VM_ReadU32( codePos + 4 );
					break;
				}
				codePos += SCR_SWITCH_ENTRY;
				caseCount--;
			}
			break;
		}

		case OP_EndSwitch:
			codePos += 2 + VM_ReadI16( codePos ) * SCR_SWITCH_ENTRY;
			break;

		case OP_Vector:
			top -= 2;
			stackTop = top;
			CastVector2( top );
			break;

		case OP_NOP:
			break;

		case OP_DevBlockBegin: {
			int offset = (int)( codePos - SCR_PROGRAM_BASE ) - 1;
			unsigned char *patch = SCR_DEV_PATCH_TABLE[offset];

			scrVmGlob_devCallStack[ scrVmGlob_funcCount ] = codePos;
			scrVmGlob_devOpcode[ scrVmGlob_funcCount ] = patch[-1];
			devDepth++;
			codePos = (const char *)patch;
			break;
		}

		case OP_DevBlockEnd:
			codePos = scrVmGlob_devCallStack[ scrVmGlob_funcCount ];
			opcode  = scrVmGlob_devOpcode[ scrVmGlob_funcCount ];
			scrVmGlob_devCallStack[ scrVmGlob_funcCount ] = NULL;
			devDepth--;
			goto dispatch;

		default:
			Scr_Error( va( "bad opcode %d", opcode ) );
			break;
		}
	}
}

unsigned short VM_Execute__FP13VariableValuePCcUsUsP13VariableValue(
		VariableValue *stackTop, const char *codePos, unsigned short thread,
		unsigned short currentObject, VariableValue *stackBase ) {
	return VM_Execute( stackTop, codePos, thread, currentObject, stackBase );
}

/* ---- VM_execute  0x0047C5D0 ---- */
unsigned short VM_execute( unsigned short self, const char *codePos, int paramCount ) {
	VariableValue  *base;
	unsigned short  thread;
	int             savedIn;
	int             savedType;
	int             i;

	VM_ReleaseInParams();

	base    = scrVmPub_top - paramCount;
	savedIn = scrVmPub_inparamcount - paramCount;

	AddRefToObject( self );
	thread = AllocThread( self );

	if ( scrVmGlob_funcCount >= SCR_VM_MAX_CALL_DEPTH ) {
		KillThread( thread );
		scrVmPub_inparamcount = savedIn + 1;
		for ( i = 0; i < savedIn; i++ ) {
			RemoveRefToValue( scrVmPub_top->type, scrVmPub_top->u );
			scrVmPub_top--;
		}
		scrVmPub_top++;
		scrVmPub_top->type = VAR_UNDEFINED;
		RuntimeError( NULL, (unsigned int)codePos, 0,
					  "script stack overflow (too many embedded function calls)" );
		return thread;
	}

	scrVmGlob_callStack[ scrVmGlob_funcCount++ ] = codePos;

	savedType  = base->type;
	base->type = VAR_CODEPOS;
	scrVmPub_inparamcount = 0;

	thread = VM_Execute( scrVmPub_top, codePos, thread, thread, base );

	base->type = savedType;
	scrVmPub_top = base + 1;
	scrVmPub_inparamcount = savedIn + 1;
	ClearVariableValue( scrVarPub_tempVariable );
	return thread;
}

static void VM_ArmTimeout( void ) {
	if ( scrVmGlob_funcCount == 0 ) {
		scrVmGlob_loopTick = __rdtsc();
	}
}

static void VM_DiscardResult( void ) {
	RemoveRefToValue( scrVmPub_top->type, scrVmPub_top->u );
	scrVmPub_top->type = VAR_UNDEFINED;
	scrVmPub_top--;
	scrVmPub_inparamcount--;
}

/* ---- Scr_ExecThread  0x0047C8E0 ---- */
unsigned short Scr_ExecThread( int posOffset, int paramCount ) {
	unsigned short thread;

	VM_ArmTimeout();
	thread = VM_execute( scrVarPub_levelId, SCR_PROGRAM_BASE + posOffset, paramCount );
	VM_DiscardResult();
	return thread;
}

/* ---- Scr_ExecEntThreadNum  0x0047C990 ---- */
unsigned short Scr_ExecEntThreadNum( int entnum, int classnum, int posOffset,
									 int paramCount ) {
	unsigned short thread;
	unsigned short entityId;

	VM_ArmTimeout();
	entityId = Scr_GetEntityId( entnum, classnum );
	thread = VM_execute( entityId, SCR_PROGRAM_BASE + posOffset, paramCount );
	VM_DiscardResult();
	return thread;
}

/* ---- Scr_AddExecThread  0x0047CA40 ---- */
void Scr_AddExecThread( int posOffset, int paramCount ) {
	unsigned short thread;

	VM_ArmTimeout();
	thread = VM_execute( scrVarPub_levelId, SCR_PROGRAM_BASE + posOffset, paramCount );
	RemoveRefToObject( thread );
}

/* ---- Scr_AddExecEntThreadNum  0x0047CAC0 ---- */
void Scr_AddExecEntThreadNum( int entnum, int classnum, int posOffset, int paramCount ) {
	unsigned short thread;
	unsigned short entityId;

	VM_ArmTimeout();
	entityId = Scr_GetEntityId( entnum, classnum );
	thread = VM_execute( entityId, SCR_PROGRAM_BASE + posOffset, paramCount );
	RemoveRefToObject( thread );
}

/* ---- Scr_FreeThread  0x0047CB40 ---- */
void Scr_FreeThread( unsigned short thread ) {
	RemoveRefToObject( thread );
}

/* ---- Scr_InitSystem  0x0047CBF0 ---- */
void Scr_InitSystem( int unused, int time ) {
	VariableValueInternal *node;

	node = AllocVariable();
	node->status = ( VAR_STAT_EXTERNAL | VAR_STRUCT );
	node->u.halfword[0] = 0;
	scr_timeArrayId = (unsigned short)( node - scrVarNodes );

	node = AllocVariable();
	node->status = ( VAR_STAT_EXTERNAL | VAR_ARRAY );
	node->u.halfword[0] = 0;
	node->u.halfword[1] = 0;
	scrVarPub_pauseArrayId = (unsigned short)( node - scrVarNodes );

	node = AllocVariable();
	node->status = ( VAR_STAT_EXTERNAL | VAR_STRUCT );
	node->u.halfword[0] = 0;
	scrVarPub_levelId = (unsigned short)( node - scrVarNodes );

	node = AllocVariable();
	node->status = ( VAR_STAT_EXTERNAL | VAR_STRUCT );
	node->u.halfword[0] = 0;
	scrVarPub_animId = (unsigned short)( node - scrVarNodes );

	scrVarPub_time = (unsigned int)time & SCR_TIME_MASK;
}

/* ---- Scr_ShutdownSystem  0x0047CCD0 ---- */
void Scr_ShutdownSystem( void ) {
	unsigned short entry;
	unsigned short next;
	unsigned short bucket;
	unsigned short thread;

	entry = FindNextSibling( scr_timeArrayId );
	while ( entry != 0 ) {
		next = FindNextSibling( entry );
		VM_TerminateTime( scrVarNodes[entry].u.halfword[0] );
		entry = next;
	}

	for ( ;; ) {
		entry = FindNextSibling( scrVarPub_pauseArrayId );
		if ( entry == 0 ) {
			break;
		}
		bucket = scrVarNodes[entry].u.halfword[0];
		entry = FindNextSibling( bucket );
		if ( entry == 0 ) {
			break;
		}
		thread = scrVarNodes[entry].u.halfword[0];

		AddRefToObject( thread );
		VM_TerminateNotifyList( thread );
		RemoveRefToObject( thread );
	}

	AddRefToObject( scrVarPub_levelId );
	ClearObjectInternal( scrVarPub_levelId );
	RemoveRefToObject( scrVarPub_levelId );
	RemoveRefToObject( scrVarPub_levelId );
	scrVarPub_levelId = 0;

	AddRefToObject( scrVarPub_animId );
	ClearObjectInternal( scrVarPub_animId );
	RemoveRefToObject( scrVarPub_animId );
	RemoveRefToObject( scrVarPub_animId );
	scrVarPub_animId = 0;

	AddRefToObject( scr_timeArrayId );
	ClearObjectInternal( scr_timeArrayId );
	RemoveRefToObject( scr_timeArrayId );
	RemoveRefToObject( scr_timeArrayId );
	scr_timeArrayId = 0;

	RemoveRefToObject( scrVarPub_pauseArrayId );
	scrVarPub_pauseArrayId = 0;
}

/* ---- VM_Init  0x00475850 ---- */
void VM_Init__Fv( void ) {
	scrVmPub_maxstack = &scrVmPub_stack[SCR_VM_STACK_COUNT - 1];
	scrVmPub_top      = &scrVmPub_stack[0];

	scrVmGlob_funcCount = 0;
	scrVmErrorFrame = NULL;
	scrVarPub_error_message = NULL;
	scrVmGlob_dialog_error_message = NULL;
	scrVarPub_error_index = 0;
	scrVmPub_terminal_error = 0;
	scrVmPub_outparamcount = 0;
	scrVmPub_inparamcount = 0;

	scrVarPub_tempVariable = AllocValue();

	scr_timeArrayId        = 0;
	scrVarPub_pauseArrayId = 0;
	scrVarPub_levelId      = 0;
	scrVarPub_gameId       = 0;
	scrVarPub_animId       = 0;
	scr_loading            = 0;

	scrVmPub_stack[0].type = VAR_CODEPOS;
}

int CastBool__FP13VariableValue( VariableValue *v )   { return CastBool( v ); }
int CastInt__FP13VariableValue( VariableValue *v )    { return CastInt( v ); }
int CastFloat__FP13VariableValue( VariableValue *v )  { return CastFloat( v ); }
int CastString__FP13VariableValue( VariableValue *v ) { return CastString( v ); }
int CastIString__FP13VariableValue( VariableValue *v ){ return CastIString( v ); }
int CastVector__FP13VariableValue( VariableValue *v ) { return CastVector( v ); }
int CastPointer__FP13VariableValue( VariableValue *v ){ return CastPointer( v ); }
int CastVector2__FP13VariableValue( VariableValue *v ){ return CastVector2( v ); }
int ClearVector__FP13VariableValue( VariableValue *v ){ ClearVector( v ); return 0; }

int UnmatchingTypesError__FP13VariableValueP13VariableValue(
		VariableValue *left, VariableValue *right ) {
	VM_UnmatchingTypesError( left, right );
	return 0;
}

/* CastWeakerPair__FP13VariableValueP13VariableValue  0x004761E0 */
int CastWeakerPair__FP13VariableValueP13VariableValue(
		VariableValue *right, VariableValue *left ) {
	return CastWeakerPair( left, right );
}
int CastWeakerPair__FP13VariableValue( VariableValue *values ) {
	return CastWeakerPairArray( values );
}
int CheckEquality__FP13VariableValue( VariableValue *values ) {
	return CheckEquality( &values[0], &values[1] );
}
int VM_ConcatenateStrings__FP13VariableValue( VariableValue *values ) {
	return VM_ConcatenateStrings( values );
}
