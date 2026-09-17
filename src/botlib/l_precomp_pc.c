/*
 * botlib -- the precompiler's public handle API and the file loader beneath it.
 *
 * these in two translation units (l_precomp_mp.c and l_script_mp.c); they are
 * together here because they are one path.
 *
 * @fidelity: verified
 *
 * This is the menu system's front door.  ui_mp_x86.dll's first act after
 * UI_SET_ACTIVE_MENU is to ask for `ui_mp/menus.txt` through UI trap 0x4A.
 *
 * ---------------------------------------------------------------------------
 * ARGUMENT ORDER
 * ---------------------------------------------------------------------------
 *
 * All five entry points are __usercall.  CL_UISystemCalls
 * (0x00418B23-0x00418B72) and CL_CgameSystemCalls (0x00403298-0x004032DB)
 * tail-call them, so the register loads are naked:
 *
 *   trap 0x49 / cg 94  mov edx,[ebp+4]                       -> PC_AddGlobalDefine  (define@edx)
 *   trap 0x4A / cg 95  mov eax,[ebp+4]; mov [esp+4],eax      -> PC_LoadSourceHandle (filename on STACK, one arg)
 *   trap 0x4B / cg 96  mov edi,[ebp+4]                       -> PC_FreeSourceHandle (handle@edi)
 *   trap 0x4C          mov ecx,[ebp+4]; mov edi,[ebp+8]; push ecx
 *                                                            -> PC_ReadTokenHandle  (pc_token@edi, handle on stack)
 *   trap 0x4D          mov edi,[ebp+0Ch]; mov edx,[ebp+8]; mov esi,[ebp+4]
 *                                                            -> PC_SourceFileAndLine(line@edi, filename@edx, handle@esi)
 *
 * Each is corroborated by the callee: PC_FreeSourceHandle opens with
 * `cmp edi, 1` (0x00442100), PC_SourceFileAndLine with `cmp esi, 1` and writes
 * the name through EDX (0x00442238) and the line through EDI (0x00442256),
 * PC_ReadTokenHandle reads its handle from [esp+438h] and stores the token
 * through EDI (0x004421BF).  All five are given their Q3/RTCW SOURCE order
 * here and both dispatchers call them that way:
 *
 *   PC_LoadSourceHandle ( args[1] )
 *   PC_FreeSourceHandle ( args[1] )
 *   PC_ReadTokenHandle  ( args[1], args[2] )
 *   PC_SourceFileAndLine( args[1], args[2], args[3] )
 *
 * PC_LoadSourceHandle: the `push esi` / `push edi` pair at 0x004420BC are
 * ordinary callee-saved register spills, popped at 0x004420E1, not arguments.
 * Com_sprintf is __usercall(dest@edi, size@esi, fmt, ...) (0x0044AC60: it
 * forwards EDI and ESI straight into _vsnprintf's Buffer and BufferCount), so
 * the ONE pushed argument there is the format string, the empty string at
 * 0x00559228.  That whole call is an inlined PS_SetBaseFolder("") -- RTCW's
 * l_precomp.c has the same line in the same place.
 *
 * ---------------------------------------------------------------------------
 * RETAIL BUGS DELIBERATELY PRESERVED
 * ---------------------------------------------------------------------------
 *
 *  - LoadScriptFile builds its path in a 64-BYTE buffer with Com_sprintf, so a
 *    qpath longer than 63 characters is silently truncated.  Q3 and RTCW both
 *    use MAX_QPATH (64) here as well.
 *  - LoadScriptFile then `strcpy`s the untruncated filename into
 *    script->filename[260] with no bound.  This is reachable from a client with
 *    an #include in a menu file; see the WARNING in the header comment there.
 *  - PC_CheckOpenSourceHandles dereferences source->scriptstack unguarded.
 *
 * Do not "fix" these without recording the divergence.
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "l_script.h"

/* Defined in l_precomp_mp.c; all three are __cdecl there. */
extern int PC_ReadToken( void *source, token_t *token );            /* l_precomp_mp.c 0x00441490 */
extern void FreeSource( void *source );                             /* l_precomp_mp.c 0x00441EA0 */
extern void PC_AddGlobalDefinesToSource( void *source );            /* l_precomp_mp.c 0x0043F8E0 */
/* PS_CreatePunctuationTable is declared in l_script.h in RTCW SOURCE order,
 * ( script, punctuations ); retail's register order is punctuations@<eax>
 * with the script pushed. */
extern int Com_Compress( char *data_p );                            /* universal/q_parse.c 0x00449AB0 */

/* The module-static base folder.  0x01646560, 64 bytes, written by
 * PC_SetBaseFolder / PS_SetBaseFolder and read here and in LoadScriptFile. */
#define pc_basefolder  ( (char *) pc_baseFolder )

/* Retail's Z_Malloc wrapper is inlined at every site: allocate four bytes more
 * than asked, stamp 0x12345678 into the first dword, hand back the payload.
 * FreeSource and PC_FreeDefine check that magic before calling free(). */
#define Z_MAGIC  0x12345678

static void *PC_GetMemory( int size ) {
	int *block;

	block = (int *) Z_MallocInternal( size + 4 );
	if ( !block ) {
		return NULL;
	}
	*block = Z_MAGIC;
	return block + 1;
}

/*
========================
default_punctuations       0x0057A820

The 52-entry punctuation table in .data, 12-byte records (char *p; int n;
punctuation_s *next): the string pointer, then the subtype, then a NULL
`next`.  Subtypes run 1..52 with no gaps and the strings are Q3/RTCW's
default_punctuations in Q3's exact order, so the P_* numbering carries over
unchanged.  A 53rd entry { NULL, 0, NULL } terminates the walk in
PS_CreatePunctuationTable and PunctuationFromNum.  The table sits 64 bytes
into the unnamed 88-byte blob that starts at 0x0057A7E0.

`next` is written by PS_CreatePunctuationTable, so the array is NOT const.

VERIFIED 0x0057A820.  All 53 entries match the binary, including `"\\"` at
subtype 50.
========================
*/
punctuation_t default_punctuations[] = {
	/* three characters */
	{ ">>=",  1, NULL },        /* P_RSHIFT_ASSIGN   */
	{ "<<=",  2, NULL },        /* P_LSHIFT_ASSIGN   */
	{ "...",  3, NULL },        /* P_PARMS           */
	/* two characters */
	{ "##",   4, NULL },        /* P_PRECOMPMERGE    */
	{ "&&",   5, NULL },        /* P_LOGIC_AND       */
	{ "||",   6, NULL },        /* P_LOGIC_OR        */
	{ ">=",   7, NULL },        /* P_LOGIC_GEQ       */
	{ "<=",   8, NULL },        /* P_LOGIC_LEQ       */
	{ "==",   9, NULL },        /* P_LOGIC_EQ        */
	{ "!=",  10, NULL },        /* P_LOGIC_UNEQ      */
	{ "*=",  11, NULL },        /* P_MUL_ASSIGN      */
	{ "/=",  12, NULL },        /* P_DIV_ASSIGN      */
	{ "%=",  13, NULL },        /* P_MOD_ASSIGN      */
	{ "+=",  14, NULL },        /* P_ADD_ASSIGN      */
	{ "-=",  15, NULL },        /* P_SUB_ASSIGN      */
	{ "++",  16, NULL },        /* P_INC             */
	{ "--",  17, NULL },        /* P_DEC             */
	{ "&=",  18, NULL },        /* P_BIN_AND_ASSIGN  */
	{ "|=",  19, NULL },        /* P_BIN_OR_ASSIGN   */
	{ "^=",  20, NULL },        /* P_BIN_XOR_ASSIGN  */
	{ ">>",  21, NULL },        /* P_RSHIFT          */
	{ "<<",  22, NULL },        /* P_LSHIFT          */
	{ "->",  23, NULL },        /* P_POINTERREF      */
	{ "::",  24, NULL },        /* P_CPP1            */
	{ ".*",  25, NULL },        /* P_CPP2            */
	/* one character */
	{ "*",   26, NULL },        /* P_MUL             */
	{ "/",   27, NULL },        /* P_DIV             */
	{ "%",   28, NULL },        /* P_MOD             */
	{ "+",   29, NULL },        /* P_ADD             */
	{ "-",   30, NULL },        /* P_SUB             */
	{ "=",   31, NULL },        /* P_ASSIGN          */
	{ "&",   32, NULL },        /* P_BIN_AND         */
	{ "|",   33, NULL },        /* P_BIN_OR          */
	{ "^",   34, NULL },        /* P_BIN_XOR         */
	{ "~",   35, NULL },        /* P_BIN_NOT         */
	{ "!",   36, NULL },        /* P_LOGIC_NOT       */
	{ ">",   37, NULL },        /* P_LOGIC_GREATER   */
	{ "<",   38, NULL },        /* P_LOGIC_LESS      */
	{ ".",   39, NULL },        /* P_REF             */
	{ ",",   40, NULL },        /* P_COMMA           */
	{ ";",   41, NULL },        /* P_SEMICOLON       */
	{ ":",   42, NULL },        /* P_COLON           */
	{ "?",   43, NULL },        /* P_QUESTIONMARK    */
	{ "(",   44, NULL },        /* P_PARENTHESESOPEN */
	{ ")",   45, NULL },        /* P_PARENTHESESCLOSE*/
	{ "{",   46, NULL },        /* P_BRACEOPEN       */
	{ "}",   47, NULL },        /* P_BRACECLOSE      */
	{ "[",   48, NULL },        /* P_SQBRACKETOPEN   */
	{ "]",   49, NULL },        /* P_SQBRACKETCLOSE  */
	{ "\\",  50, NULL },        /* P_BACKSLASH       */
	{ "#",   51, NULL },        /* P_PRECOMP         */
	{ "$",   52, NULL },        /* P_DOLLAR          */
	{ NULL,   0, NULL }
};

/*
========================
LoadScriptFile            0x00443A60

One stack argument, the qpath.  `lea edi,[esp+58h+Source]` and `mov esi,40h`
at 0x00443A89 are Com_sprintf's DEST and SIZE arriving in registers, not
arguments of this function.

The allocation is one block: sizeof(script_t) + length + 1, plus four bytes for
the heap magic.  `lea edi,[ebx+571h]` at 0x00443AF3 is that 1393, which is what
pins sizeof(script_t) at 1392.  The text follows the record, so script->buffer
is script + 1392 and the NUL lands at buffer[length].

WARNING -- REACHABLE STACK SMASH.  `strcpy(script->filename, filename)` at
0x00443B40 is unbounded into a 260-byte field, and this is now reachable from a
CLIENT with a hostile menu file: PC_Directive_include hands whatever the
`#include` names straight through.  It is retail behaviour (Q3 and RTCW have
the same line) and it is left alone, but it is a genuine remote-ish surface
on the client that it was not on a dedicated server.

VERIFIED 0x00443A60.

  * the frame has exactly one argument slot (`fmt` at +0x4C) above
    `__return_address` at +0x48;
  * `lea edi,[ebx+571h]` then `lea ecx,[edi+4]` is Z_MallocInternal(length +
    1393 + 4), i.e. PC_GetMemory( sizeof(script_t) + length + 1 ) with
    sizeof(script_t) == 1392;
  * the guard is `test ebp,ebp` on the HANDLE, not on the length, so a
    zero-length file that opens is still loaded -- `if ( !f )`, not
    `if ( !length )`;
  * the double zeroing is real: `rep stosd`/`rep stosb` for length+1393 bytes
    at 0x443B16, then `mov ecx,15Ch / rep stosd` for the 1392-byte header again
    at 0x443B2C;
  * field stores: +0x104 buffer, +0x108 script_p, +0x10C end_p, +0x110
    lastscript_p, +0x11C length, +0x120 line, +0x124 lastline, +0x128
    tokenavailable, +0x130 punctuations;
  * `flags` (+0x12C) is never stored, because the memset already cleared it;
  * `mov [esi+11Ch], eax` after Com_Compress overwrites length and leaves end_p
    pointing at the uncompressed end.

The strcpy at 0x443B40 is the inlined byte-copy loop, and `mov [eax+ebx], cl`
immediately after it reuses the loop's terminating NUL to write
buffer[length] -- so the unbounded copy and the NUL store are one idiom, which
is why the overflow is not visible as a separate call.
========================
*/
script_t *LoadScriptFile( const char *filename ) {
	char         path[64];
	fileHandle_t f;
	int          length;
	script_t    *script;

	/* Inlined: retail tests strlen(basefolder) with an inline scan at
	 * 0x00443A78 and picks the format string from it. */
	if ( strlen( pc_basefolder ) ) {
		Com_sprintf( path, sizeof( path ), "%s/%s", pc_basefolder, filename );
	} else {
		Com_sprintf( path, sizeof( path ), "%s", filename );
	}

	length = FS_FOpenFileByMode( path, &f, FS_READ );
	if ( !f ) {
		return NULL;
	}

	script = (script_t *) PC_GetMemory( sizeof( script_t ) + length + 1 );
	if ( !script ) {
		FS_FCloseFile( f );
		return NULL;
	}
	/* Retail zeroes the whole block and then zeroes the header a second time
	 * (0x00443B16 and 0x00443B2A).  The second pass is redundant. */
	memset( script, 0, sizeof( script_t ) + length + 1 );

	strcpy( script->filename, filename );          /* unbounded -- see above */
	script->buffer = (char *) script + sizeof( script_t );
	script->buffer[length]  = '\0';
	script->script_p        = script->buffer;
	script->lastscript_p    = script->buffer;
	script->end_p           = script->buffer + length;
	script->line            = 1;
	script->lastline        = 1;
	script->length          = length;
	script->tokenavailable  = 0;

	/* SetScriptPunctuations( script, NULL ) -- inlined by retail */
	PS_CreatePunctuationTable( script, default_punctuations );
	script->punctuations = default_punctuations;

	FS_Read( script->buffer, length, f );
	FS_FCloseFile( f );

	/* Com_Compress strips comments and runs of whitespace in place and returns
	 * the new length; retail stores it back over script->length at 0x00443BD4,
	 * so end_p is deliberately left pointing at the UNCOMPRESSED end.  That is
	 * retail's behaviour; the text only ever shrinks, so end_p stays a valid
	 * upper bound. */
	script->length = Com_Compress( script->buffer );

	return script;
}

/*
========================
LoadSourceFile            0x00441D20

One stack argument: `mov eax,[esp+4]` at entry and `mov ecx,[esp+10h+Source]`
at 0x00441D6D read the SAME slot, once for LoadScriptFile and once for the
strncpy.

0x65C at 0x00441D3E is the source_t allocation -- 4 bytes of heap magic plus
1624 -- and 0x1004 at 0x00441D7D is the define hash, 1024 slots plus its own
magic.  filename is strncpy'd with 0x104 = 260, which is what sizes the field.
========================
*/
source_t *LoadSourceFile( const char *filename ) {
	source_t *source;
	script_t *script;

	script = LoadScriptFile( filename );
	if ( !script ) {
		return NULL;
	}
	script->next = NULL;

	source = (source_t *) PC_GetMemory( sizeof( source_t ) );
	if ( !source ) {
		return NULL;
	}
	memset( source, 0, sizeof( source_t ) );

	/* strncpy, not strcpy: retail bounds this one at 260 and does NOT
	 * guarantee the terminator, exactly as RTCW.  The memset above is what
	 * makes a 260-character name terminate. */
	strncpy( source->filename, filename, 260 );
	source->scriptstack = script;
	source->tokens      = NULL;
	source->defines     = NULL;
	source->indentstack = NULL;
	source->skip        = 0;

	source->definehash = (void **) PC_GetMemory( 1024 * sizeof( void * ) );
	if ( source->definehash ) {
		memset( source->definehash, 0, 1024 * sizeof( void * ) );
	}

	PC_AddGlobalDefinesToSource( source );
	return source;
}

/*
========================
The open-source-handle table      0x01646440, 64 entries

file-local in retail as it is in RTCW: the only five functions that touch
0x01646440 are the five below, so this is a private table in everything but
storage.  It stays on the shared symbol rather than becoming a `static` here
so that all five agree on one array (0x01646444 is &sourceFiles[1], the scan
base).  cod1_globals.h types it `int[64]`; the cast to `source_t *` is
confined to this one macro.

Retail's scan at 0x00442020 is the compiler's 9-way unrolling of the plain
RTCW loop; it can read up to sourceFiles[71] before the `cmp ebx,40h` bound
check rejects the index, which is harmless -- the over-read is discarded.
========================
*/
#define pc_sourceFiles  ( (source_t **) sourceFiles )

/*
========================
PC_LoadSourceHandle       0x00442010

Returns a handle in [1,64), or 0.  Slot 0 is never used so that 0 can mean
failure.
========================
*/
int PC_LoadSourceHandle( const char *filename ) {
	source_t *source;
	int       i;

	for ( i = 1; i < MAX_SOURCEFILES; i++ ) {
		if ( !pc_sourceFiles[i] ) {
			break;
		}
	}
	if ( i >= MAX_SOURCEFILES ) {
		return 0;
	}

	/* PS_SetBaseFolder("") inlined -- Com_sprintf(basefolder, 64, "") at
	 * 0x004420CD, whose format string is the empty string at 0x00559228.
	 * Without it a base folder left over from a previous load would be
	 * prepended to this filename by LoadScriptFile. */
	Com_sprintf( pc_basefolder, 64, "" );

	source = LoadSourceFile( filename );
	if ( !source ) {
		return 0;
	}

	pc_sourceFiles[i] = source;
	return i;
}

/*
========================
PC_FreeSourceHandle       0x00442100
========================
*/
int PC_FreeSourceHandle( int handle ) {
	if ( handle < 1 || handle >= MAX_SOURCEFILES ) {
		return 0;
	}
	if ( !pc_sourceFiles[handle] ) {
		return 0;
	}

	FreeSource( pc_sourceFiles[handle] );
	pc_sourceFiles[handle] = NULL;
	return 1;
}

/*
========================
PC_ReadTokenHandle        0x00442140

Reads one token and copies the pc_token_t subset out.  The 64-bit floatvalue is
narrowed to 32 bits by the `fstp dword ptr [edi+0Ch]` at 0x004421BF.

The stack token_t is 1072 bytes at [esp+0]..[esp+42Fh] with the cookie at
[esp+430h]; PC_ReadToken fills all 1072 bytes of it.

DIVERGENCE (deliberate): the record is zeroed before the call.  Retail does
not, and PC_ReadToken's early-failure paths can leave it untouched, which makes
the strcpy below read an uninitialised 1024-byte stack buffer with no
guaranteed NUL.  Retail got away with it; there is no behavioural difference on
the success path and the failure path stops depending on stack residue.
========================
*/
int PC_ReadTokenHandle( int handle, pc_token_t *pc_token ) {
	token_t token;
	int     ret;

	if ( handle < 1 || handle >= MAX_SOURCEFILES ) {
		return 0;
	}
	if ( !pc_sourceFiles[handle] ) {
		return 0;
	}

	memset( &token, 0, sizeof( token ) );        /* see DIVERGENCE above */
	ret = PC_ReadToken( pc_sourceFiles[handle], &token );

	strcpy( pc_token->string, token.string );
	pc_token->type       = token.type;
	pc_token->subtype    = token.subtype;
	pc_token->intvalue   = (int) token.intvalue;
	pc_token->floatvalue = (float) token.floatvalue;

	if ( pc_token->type == TT_STRING ) {
		/* __fastcall(unused@ecx, string@edx); see l_script.h. */
		StripDoubleQuotes( 0, pc_token->string );
	}
	return ret;
}

/*
========================
PC_SourceFileAndLine      0x00442220

`strcpy`, unbounded, into whatever buffer the VM handed over -- retail, and Q3.
========================
*/
int PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	source_t *source;

	if ( handle < 1 || handle >= MAX_SOURCEFILES ) {
		return 0;
	}
	source = pc_sourceFiles[handle];
	if ( !source ) {
		return 0;
	}

	strcpy( filename, source->filename );
	if ( source->scriptstack ) {
		*line = source->scriptstack->line;
	} else {
		*line = 0;
	}
	return 1;
}

/*
========================
PC_CheckOpenSourceHandles 0x00442290

Retail reads `*(char **)(source + 524)` and prints it with %s.  That is
source->scriptstack reinterpreted as a string, which is correct only because
script_t::filename sits at offset 0 -- i.e. it is
`pc_sourceFiles[i]->scriptstack->filename`, spelled the way RTCW spells it.
Unguarded against a NULL scriptstack, as retail is.
========================
*/
void PC_CheckOpenSourceHandles( void ) {
	int i;

	for ( i = 1; i < MAX_SOURCEFILES; i++ ) {
		if ( pc_sourceFiles[i] ) {
			Com_Printf( "^1Error: file %s still open in precompiler\n",
			            pc_sourceFiles[i]->scriptstack->filename );
		}
	}
}

/*
 * ===========================================================================
 * THE PRECOMPILER DIRECTIVE TABLES
 * ===========================================================================
 *
 * directives[]       0x0057AAA0   14 entries + NULL terminator
 * dollardirectives[] 0x0057AB40    2 entries + NULL terminator
 *
 * 8-byte records: name pointer, handler address, then the NULL pair that
 * terminates the walk.  The order is retail's, and is also Q3/RTCW's,
 * unchanged.
 *
 * Retail keeps both tables in l_precomp_mp.c's translation unit as statics;
 * they are here with the directive dispatchers.
 */

typedef struct directive_s {
	const char *name;
	int ( *func )( void *source );
} directive_t;

/* The 16 handlers, all in l_precomp_mp.c, all __cdecl with the source_t as
 * their one stack argument (`push ebp / call [table] / add esp,4` at
 * 0x004410EF).  Declared with a uniform prototype so the table needs no casts;
 * l_precomp_mp.c spells the same parameter `int` or `_DWORD *` from function
 * to function, the same 4-byte cdecl argument either way. */
extern int PC_Directive_if             ( void *source );   /* 0x00440BE0 */
extern int PC_Directive_ifdef          ( void *source );   /* 0x0043FA60 */
extern int PC_Directive_ifndef         ( void *source );   /* 0x0043FA80 */
extern int PC_Directive_elif           ( void *source );   /* 0x00440B70 */
extern int PC_Directive_else           ( void *source );   /* 0x0043FAA0 */
extern int PC_Directive_endif          ( void *source );   /* 0x0043FB10 */
extern int PC_Directive_include        ( void *source );   /* 0x0043EB50 */
extern int PC_Directive_define         ( void *source );   /* 0x0043EFE0 */
extern int PC_Directive_undef          ( void *source );   /* 0x0043EE60 */
extern int PC_Directive_line           ( void *source );   /* 0x00440C20 */
extern int PC_Directive_error          ( void *source );   /* 0x00440C40 */
extern int PC_Directive_pragma         ( void *source );   /* 0x00440CA0 */
extern int PC_Directive_eval           ( void *source );   /* 0x00440DC0 */
extern int PC_Directive_evalfloat      ( void *source );   /* 0x00440ED0 */
extern int PC_DollarDirective_evalint  ( void *source );   /* 0x00441100 */
extern int PC_DollarDirective_evalfloat( void *source );   /* 0x00441220 */

/* Also in l_precomp_mp.c, both __cdecl there.  Note the order -- the TOKEN
 * is first.  Retail passes it in EAX with the source in EBX and no push at
 * all (0x00441002-0x00441008), so neither is a stack argument in the
 * original; the order below is the one the definition and every call site in
 * l_precomp_mp.c agree on. */
extern int  PC_ReadSourceToken  ( token_t *token, void *source );  /* 0x0043DA50 */
extern int  PC_UnreadSourceToken( token_t *token, void *source );  /* 0x0043DBC0 */
extern void SourceError         ( void *source, char *fmt, ... );  /* 0x0043D7B0 */

/* VERIFIED 0x0057AAA0 (directives) and 0x0057AB40 (dollardirectives).  The
 * two tables are the only .data references PC_ReadDirective and
 * PC_ReadDollarDirective make apart from __security_cookie.  All fourteen
 * names and handler addresses match, in this order, terminated by a
 * { NULL, NULL } pair; dollardirectives is two entries plus its terminator. */
static directive_t directives[] = {
	{ "if",          PC_Directive_if        },   /* 0x00440BE0 */
	{ "ifdef",       PC_Directive_ifdef     },   /* 0x0043FA60 */
	{ "ifndef",      PC_Directive_ifndef    },   /* 0x0043FA80 */
	{ "elif",        PC_Directive_elif      },   /* 0x00440B70 */
	{ "else",        PC_Directive_else      },   /* 0x0043FAA0 */
	{ "endif",       PC_Directive_endif     },   /* 0x0043FB10 */
	{ "include",     PC_Directive_include   },   /* 0x0043EB50 */
	{ "define",      PC_Directive_define    },   /* 0x0043EFE0 */
	{ "undef",       PC_Directive_undef     },   /* 0x0043EE60 */
	{ "line",        PC_Directive_line      },   /* 0x00440C20 */
	{ "error",       PC_Directive_error     },   /* 0x00440C40 */
	{ "pragma",      PC_Directive_pragma    },   /* 0x00440CA0 */
	{ "eval",        PC_Directive_eval      },   /* 0x00440DC0 */
	{ "evalfloat",   PC_Directive_evalfloat },   /* 0x00440ED0 */
	{ NULL,          NULL                   }
};

/* DIVERGENCE FROM Q3/RTCW, retail's: Q3 registers several dollar directives;
 * CoD ships exactly two.  At 0x0057AB40 the third dword pair is already the
 * NULL terminator. */
static directive_t dollardirectives[] = {
	{ "evalint",     PC_DollarDirective_evalint   },   /* 0x00441100 */
	{ "evalfloat",   PC_DollarDirective_evalfloat },   /* 0x00441220 */
	{ NULL,          NULL                         }
};

/*
========================
PC_ReadDirective          0x00440FE0   VERIFIED 0x00440FE0

Called from PC_ReadToken (0x004414E3), which returns whatever this returns.

The token is read first and the "found # without name" arm is the FAILURE of
that read (0x0044100F); linescrossed is tested next at [ebp-10h], +0x424 into
the 1072-byte record, i.e. token.linescrossed (0x00441026); the type test is
[ebp-34h] = +0x400 = token.type against 4 = TT_NAME (0x0044105F); and the
handler call is `push ebp / call [0x0057AAA4+edi*8] / add esp,4` at
0x004410EF, a plain cdecl indirect with the source as its one argument.

DIVERGENCE (spelling only): retail inlines PC_UnreadSourceToken at 0x00441031
as PC_CopyToken followed by the two-store list prepend.  PC_UnreadSourceToken
is exactly those three instructions, so the call below is the same code.

Note what this does NOT do, and what the $ twin below does: on an unrecognised
`#name` the token is NOT pushed back.  That asymmetry is in the binary --
0x004410BD falls straight into SourceError -- and it is in RTCW too.
========================
*/
int PC_ReadDirective( void *source ) {
	token_t token;
	int     i;

	/* read the directive name */
	if ( !PC_ReadSourceToken( &token, source ) ) {
		SourceError( source, "found # without name" );
		return 0;
	}
	/* the directive name must be on the same line as the # */
	if ( token.linescrossed > 0 ) {
		PC_UnreadSourceToken( &token, source );
		SourceError( source, "found # at end of line" );
		return 0;
	}
	if ( token.type == TT_NAME ) {
		for ( i = 0; directives[i].name; i++ ) {
			if ( !strcmp( directives[i].name, token.string ) ) {
				return directives[i].func( source );
			}
		}
	}
	SourceError( source, "unknown precompiler directive %s", token.string );
	return 0;
}

/*
========================
PC_ReadDollarDirective    0x00441350   VERIFIED 0x00441350

Same shape as the # reader above, for $evalint and $evalfloat.

The one structural difference is real and is RTCW's as well: the unknown-name
arm DOES unread the token before erroring.  The listing does the
PC_CopyToken/prepend pair twice in this function -- once for the end-of-line
arm and once for the unknown arm -- where PC_ReadDirective does it once.  Kept.
========================
*/
int PC_ReadDollarDirective( void *source ) {
	token_t token;
	int     i;

	if ( !PC_ReadSourceToken( &token, source ) ) {
		SourceError( source, "found $ without name" );
		return 0;
	}
	if ( token.linescrossed > 0 ) {
		PC_UnreadSourceToken( &token, source );
		SourceError( source, "found $ at end of line" );
		return 0;
	}
	if ( token.type == TT_NAME ) {
		for ( i = 0; dollardirectives[i].name; i++ ) {
			if ( !strcmp( dollardirectives[i].name, token.string ) ) {
				return dollardirectives[i].func( source );
			}
		}
	}
	PC_UnreadSourceToken( &token, source );
	SourceError( source, "unknown precompiler directive %s", token.string );
	return 0;
}
