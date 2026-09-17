/*
 * botlib -- the lexicographical parser.
 *
 * Retail range 0x004422C0-0x00443D28, 38 functions.
 *
 * @fidelity: likely
 * @fidelity-default: likely
 *
 * Functions carrying a `VERIFIED <addr>` line in their banner have been diffed
 * against the disassembly instruction by instruction; between them they cover
 * every function the retail binary reaches.  Everything else is dead in retail
 * (zero xrefs, listed below) and inherits the unit default.
 *
 * ===========================================================================
 * RTCW CORRESPONDENCE -- THIS UNIT IS RTCW's botlib/l_script.c
 * ===========================================================================
 *
 * Every function here has an RTCW ancestor and all but two are VERBATIM:
 * `RTCW-MP/src/botlib/l_script.c` compiled statement for statement, with the
 * usual compiler transforms (loop rotation, CSE of the duplicated
 * `if (lastp)` tail, cross-TU helpers inlined).
 *
 * The two that diverge are ScriptError/ScriptWarning, which print through
 * Com_Printf instead of botimport.Print and gain CoD's "^1" colour code on the
 * error path.  Everything else is unchanged id code.
 *
 * ---------------------------------------------------------------------------
 * ARGUMENT ORDER: SOURCE ORDER, NOT REGISTER ORDER
 * ---------------------------------------------------------------------------
 *
 * Almost every function in this unit is `__usercall` in retail -- the compiler
 * lifted one to four leading source parameters into registers.  Each register
 * binding is recorded on the function.  Every one is source order with a
 * prefix of the arguments lifted into registers, except these, whose register
 * binding is permuted with respect to the source:
 *
 *   PS_CreatePunctuationTable  punctuations@eax, script on stack  -- SWAPPED
 *   SetScriptPunctuations      p@edi, script@esi                  -- SWAPPED
 *   PS_ReadToken               token@eax, script@ecx              -- SWAPPED
 *   PS_ExpectAnyToken          token@eax, script@ecx              -- SWAPPED
 *   PS_SkipUntilString         string@edi, script on stack        -- SWAPPED
 *   SetScriptFlags             flags@eax, script@ecx              -- SWAPPED
 *   ScriptSkipTo               value@edi, script@esi              -- SWAPPED
 *
 * All are declared in RTCW SOURCE order here, and every call site uses that
 * order (the four outside this unit are all in botlib/: l_precomp_mp.c
 * PS_ReadToken and LoadScriptMemory x2, l_precomp_pc.c
 * PS_CreatePunctuationTable).  The bindings above are what to restore for
 * byte-for-byte work.
 *
 * ---------------------------------------------------------------------------
 * PS_ReadToken CARRIES PS_ReadPrimitive INLINED
 * ---------------------------------------------------------------------------
 *
 * 0x00442FB0 is RTCW's PS_ReadToken with PS_ReadPrimitive INLINED into its
 * SCFL_PRIMITIVE branch: it tests script->tokenavailable and copies out of
 * script->token, which only PS_ReadToken does; its SCFL_PRIMITIVE arm contains
 * PS_ReadPrimitive's whole body rather than a call; and every one of its ten
 * callers is a function that calls PS_ReadToken in RTCW (PC_ReadSourceToken,
 * PS_Expect*, PS_Check*, PS_SkipUntilString, ReadSigned*).  There is no
 * separate PS_ReadPrimitive in the binary.
 *
 * ---------------------------------------------------------------------------
 * THREE RETAIL BUGS THAT MUST STAY -- ALL THREE ARE RTCW's, NOT CoD's
 * ---------------------------------------------------------------------------
 *
 * 1. PS_ReadNumber's hexadecimal scanner accepts only 'A', not 'A'-'F'
 *    (0x00442B92: `cmp cl,41h` / `jl` / `jg`).  RTCW-MP's
 *    src/botlib/l_script.c line 660 reads
 *
 *        ( c >= 'A' && c <= 'A' ) )
 *
 *    verbatim.  CoD inherited it from Q3 through RTCW untouched -- "0xFF"
 *    must keep scanning as "0x" plus a name token "FF".  NumberValue's
 *    converter does accept 'A'-'F', so its upper arm is unreachable through
 *    the normal path; that asymmetry is RTCW's too.
 *
 * 2. PS_ReadPunctuation indexes the 256-entry punctuation hash with a SIGNED
 *    char (`movsx ecx, byte ptr [eax]` at 0x00442E96), so any byte >= 0x80
 *    reads off the front of the table.  RTCW writes
 *    `script->punctuationtable[(unsigned int)*script->script_p]`, and a cast of
 *    a signed char to unsigned int is sign-extension -- identical code.
 *    PS_CreatePunctuationTable indexes the same way when BUILDING the table
 *    (`movsx eax, byte ptr [ebp+0]` at 0x00442380), so the two agree and the
 *    table is self-consistent; only a high-bit byte in the INPUT is affected.
 *
 * 3. PS_ReadEscapeCharacter's hexadecimal escape accepts 'A'-'Z' and 'a'-'z'
 *    rather than 'A'-'F'/'a'-'f', so "\xZZ" parses.  RTCW lines 409-412.
 *
 * A fourth, in this unit's callee: the `#include <...>` strncat overflow at
 * 0x0043ECCE lives in l_precomp_mp.c and is likewise Q3's and RTCW's.
 *
 * ---------------------------------------------------------------------------
 * WHAT IS DEAD IN THE RETAIL BINARY
 * ---------------------------------------------------------------------------
 *
 * LIVE:
 * PS_CreatePunctuationTable, ScriptError, ScriptWarning, PS_ReadWhiteSpace,
 * PS_ReadEscapeCharacter, PS_ReadString, PS_ReadName, NumberValue,
 * PS_ReadNumber, PS_ReadPunctuation, PS_ReadToken, LoadScriptMemory,
 * StripDoubleQuotes (plus LoadScriptFile, in l_precomp_pc.c).
 *
 * DEAD -- zero xrefs, kept by the linker only: PunctuationFromNum,
 * SetScriptPunctuations (LoadScriptFile and LoadScriptMemory inline it),
 * PS_ReadLiteral (PS_ReadToken routes literals through PS_ReadString, exactly
 * as RTCW does with PS_ReadLiteral commented out at its call site),
 * PS_ExpectTokenString, PS_ExpectTokenType, PS_ExpectAnyToken,
 * PS_CheckTokenString, PS_CheckTokenType, PS_SkipUntilString,
 * PS_UnreadLastToken, PS_UnreadToken, PS_NextWhiteSpaceChar, ReadSignedFloat,
 * ReadSignedInt, SetScriptFlags, GetScriptFlags, ResetScript, EndOfScript,
 * NumLinesCrossed, ScriptSkipTo, FreeScript, PS_SetBaseFolder.
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "l_script.h"

/* universal/q_parse.c 0x00449AB0. */
extern int Com_Compress( char *data_p );

/*
 * Retail inlines botlib's GetMemory/GetClearedMemory/FreeMemory at every site
 * in this unit: allocate four bytes more than asked, stamp 0x12345678 into the
 * first dword, return the payload; free only if the magic matches.  The bodies
 * at 0x0043D6B0 are never called from here, so calling them would emit a call
 * retail does not have.  Same choice, same reason, as PC_GetMemory in
 * l_precomp_pc.c.
 */
#define Z_MAGIC  0x12345678

static void *PS_GetMemory( int size ) {
	int *block;

	block = (int *) Z_MallocInternal( size + 4 );
	if ( !block ) {
		return NULL;
	}
	*block = Z_MAGIC;
	return block + 1;
}

static void PS_FreeMemory( void *ptr ) {
	int *block;

	block = (int *) ptr - 1;
	if ( *block == Z_MAGIC ) {
		free( block );
	}
}

/*
 * The module-static base folder, 0x01646560, 64 bytes.  Retail compiled the
 * same helper into both botlib translation units, so PS_SetBaseFolder
 * (0x00443D10) and PC_SetBaseFolder (0x00442270) are byte-for-byte identical
 * and write the SAME buffer.  RTCW declares `char basefolder[MAX_QPATH]`;
 * CoD's is 64 bytes, which is MAX_QPATH.
 */
#define basefolder  ( (char *) pc_baseFolder )

/*
===============================================================================
PS_CreatePunctuationTable          0x004422C0    VERBATIM

RTCW l_script.c:181.  Statement for statement, including the sort that keeps
longer punctuations first.  The compiler shared the `if (lastp) lastp->next =
newp; else table[c] = newp;` tail between RTCW's two copies of it (both arms
reach 0x0044236A), which is the only structural difference.

RETAIL: __usercall( punctuations@<eax>, script on stack ).  SWAPPED with
respect to RTCW's (script, punctuations); source order here.

GetMemory( 256 * sizeof(punctuation_t *) ) is inlined: `push 404h` /
Z_MallocInternal / stamp / +4 at 0x004422D5.  1024 payload + 4 header.

VERIFIED 0x004422C0.  punctuationtable is +0x134 (308); the table clear is
`mov ecx,100h / rep stosd`, 256 dwords; the two inlined strlens at 0x442337 and
0x442347 feed `cmp esi,eax / jb`, i.e. strlen(p->p) < strlen(newp->p), the
longer-first sort; the entry cursor advances `add ebx,0Ch`, the 12-byte
punctuation_t stride.

The table is INDEXED with a sign-extended char when it is built -- 0x442317
and 0x442380 are both `movsx ecx, byte ptr [ebp+0]` -- exactly as
PS_ReadPunctuation does when it reads, so the table is self-consistent and the
retail bug only bites on a high-bit byte in the INPUT.  `(unsigned int)
newp->p[0]` below compiles to the same `movsx` because the cast applies to an
already-signed char.
===============================================================================
*/
void PS_CreatePunctuationTable( script_t *script, punctuation_t *punctuations ) {
	int i;
	punctuation_t *p, *lastp, *newp;

	/* get memory for the table */
	if ( !script->punctuationtable ) {
		script->punctuationtable = (punctuation_t **)
			PS_GetMemory( 256 * sizeof( punctuation_t * ) );
	}
	memset( script->punctuationtable, 0, 256 * sizeof( punctuation_t * ) );

	/* add the punctuations in the list to the punctuation table */
	for ( i = 0; punctuations[i].p; i++ ) {
		newp = &punctuations[i];
		lastp = NULL;
		/* sort this table entry on length, longer punctuations first */
		for ( p = script->punctuationtable[(unsigned int) newp->p[0]]; p; p = p->next ) {
			if ( strlen( p->p ) < strlen( newp->p ) ) {
				newp->next = p;
				if ( lastp ) {
					lastp->next = newp;
				} else {
					script->punctuationtable[(unsigned int) newp->p[0]] = newp;
				}
				break;
			}
			lastp = p;
		}
		if ( !p ) {
			newp->next = NULL;
			if ( lastp ) {
				lastp->next = newp;
			} else {
				script->punctuationtable[(unsigned int) newp->p[0]] = newp;
			}
		}
	}
}

/*
===============================================================================
PunctuationFromNum                 0x004423B0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:222.  The 12-byte stride of the walk (`add eax, 0Ch` at
0x004423CC) is what pins sizeof(punctuation_t).  The "unkown punctuation"
spelling is id's and is in the binary.

RETAIL: __usercall( script@<eax>, num@<edi> ).  Source order.
===============================================================================
*/
char *PunctuationFromNum( script_t *script, int num ) {
	int i;

	for ( i = 0; script->punctuations[i].p; i++ ) {
		if ( script->punctuations[i].n == num ) {
			return script->punctuations[i].p;
		}
	}
	return "unkown punctuation";
}

/*
===============================================================================
ScriptError                        0x004423F0    DIVERGED
ScriptWarning                      0x00442460    DIVERGED

RTCW l_script.c:239 / :266.  The bodies are identical -- SCFL_NOERRORS /
SCFL_NOWARNINGS guard, `char text[1024]`, vsprintf, one print -- but the print
is CoD's.  RTCW routes through the botlib import table:

    botimport.Print( PRT_ERROR,   "file %s, line %d: %s\n", ... )
    botimport.Print( PRT_WARNING, "file %s, line %d: %s\n", ... )

CoD1 has no botlib import table and calls Com_Printf directly, prefixing the
error with the "^1" red colour code.  The WARNING format is RTCW's unchanged --
so a CoD warning prints with no "warning:" marker at all, which is RTCW's
behaviour once PRT_WARNING is dropped.  The same substitution was made in
l_precomp_mp.c's SourceError/SourceWarning.

RETAIL: __usercall( script@<esi>, str on stack, ... ).  Source order.

VERIFIED 0x004423F0 and 0x00442460.  `sub esp,404h` is the 1024-byte buffer
plus the /GS cookie slot; the guards are `test byte ptr [esi+12Ch],1` and
`...,2`, so SCFL_NOERRORS is 1 and SCFL_NOWARNINGS is 2 at flags +0x12C;
script->line is +0x120.  0x00561BE8 is "^1Error: file %s, line %d: %s\n" and
0x00561BD0 is "file %s, line %d: %s\n".

The first argument pushed for the format is `esi`, the SCRIPT pointer, where
the format wants script->filename; filename is at offset 0, so the two are the
same address.
===============================================================================
*/
void QDECL ScriptError( script_t *script, char *str, ... ) {
	char text[1024];
	va_list ap;

	if ( script->flags & SCFL_NOERRORS ) {
		return;
	}

	va_start( ap, str );
	vsprintf( text, str, ap );
	va_end( ap );

	Com_Printf( "^1Error: file %s, line %d: %s\n", script->filename, script->line, text );
}

/* VERIFIED 0x00442460.  `test byte ptr [esi+12Ch],2`; format at 0x00561BD0. */
void QDECL ScriptWarning( script_t *script, char *str, ... ) {
	char text[1024];
	va_list ap;

	if ( script->flags & SCFL_NOWARNINGS ) {
		return;
	}

	va_start( ap, str );
	vsprintf( text, str, ap );
	va_end( ap );

	Com_Printf( "file %s, line %d: %s\n", script->filename, script->line, text );
}

/*
===============================================================================
SetScriptPunctuations              0x004424D0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:293 with PUNCTABLE defined.  Dead because both callers --
LoadScriptFile (0x00443A60) and LoadScriptMemory (0x00443BF0) -- inlined it.

RETAIL: __usercall( p@<edi>, script@<esi> ).  SWAPPED; source order here.
===============================================================================
*/
void SetScriptPunctuations( script_t *script, punctuation_t *p ) {
	if ( p ) {
		PS_CreatePunctuationTable( script, p );
		script->punctuations = p;
	} else {
		PS_CreatePunctuationTable( script, default_punctuations );
		script->punctuations = default_punctuations;
	}
}

/*
===============================================================================
PS_ReadWhiteSpace                  0x00442500    VERBATIM

RTCW l_script.c:311.  Skips whitespace and both comment forms, counting lines.
The compiler rotated the leading `while (*script_p <= ' ')` so the first test
is peeled (0x00442509 `cmp byte ptr [eax],20h / jg 442536`, entered before the
loop body at 0x442510) and merged the two `continue` paths into one tail at
0x0044259C.  Behaviourally identical.

RETAIL: __usercall( script@<ecx> ).

VERIFIED 0x00442500.  script_p is +0x108 and line is +0x120 throughout.
`mov bl,2Fh` at 0x442501 hoists '/' into BL for both comment tests.

The merged tail is 0x4425A3 (`inc eax / store / cmp [eax],0 / jz return 0 /
jnz outer`).  The `//` arm JUMPS to it after its line++, so it gets exactly
ONE script_p++ and one NUL check; the block-comment arm does its own increment
and check at 0x442597 and then FALLS INTO the tail for a second pair -- one
increment for `//` and two for the block form, spelled out longhand below.

Both exits are explicit: 0x4425B3 `xor eax,eax` for every NUL-terminated path,
0x4425B7 `mov eax,1` for "stopped on a real character".
===============================================================================
*/
int PS_ReadWhiteSpace( script_t *script ) {
	while ( 1 ) {
		/* skip white space */
		while ( *script->script_p <= ' ' ) {
			if ( !*script->script_p ) {
				return 0;
			}
			if ( *script->script_p == '\n' ) {
				script->line++;
			}
			script->script_p++;
		}
		/* skip comments */
		if ( *script->script_p == '/' ) {
			/* comments // */
			if ( *( script->script_p + 1 ) == '/' ) {
				script->script_p++;
				do {
					script->script_p++;
					if ( !*script->script_p ) {
						return 0;
					}
				} while ( *script->script_p != '\n' );
				script->line++;
				script->script_p++;
				if ( !*script->script_p ) {
					return 0;
				}
				continue;
			}
			/* comments <slash-star star-slash> */
			else if ( *( script->script_p + 1 ) == '*' ) {
				script->script_p++;
				do {
					script->script_p++;
					if ( !*script->script_p ) {
						return 0;
					}
					if ( *script->script_p == '\n' ) {
						script->line++;
					}
				} while ( !( *script->script_p == '*' && *( script->script_p + 1 ) == '/' ) );
				script->script_p++;
				if ( !*script->script_p ) {
					return 0;
				}
				script->script_p++;
				if ( !*script->script_p ) {
					return 0;
				}
				continue;
			}
		}
		break;
	}
	return 1;
}

/*
===============================================================================
PS_ReadEscapeCharacter             0x004425C0    VERBATIM

RTCW l_script.c:382.  The switch is a jump table at 0x00442784 (data, not a
function).

Retail's `\x` arm accepts 'A'-'Z' and 'a'-'z' rather than 'A'-'F'/'a'-'f'.
That is RTCW's, lines 409-412, not CoD's.  Both value clamps and both
ScriptWarning texts match.

RETAIL: __usercall( script@<eax>, ch on stack ).  Source order.

VERIFIED 0x004425C0.  The twelve switch arms and their values:

    92 '\'->0x5C   110 'n'->0x0A   114 'r'->0x0D   116 't'->0x09
   118 'v'->0x0B    98 'b'->0x08   102 'f'->0x0C    97 'a'->0x07
    39 '''->0x27    34 '"'->0x22    63 '?'->0x3F   120 'x'->hex arm

0x442699 is `cmp eax,5Ah` ('Z', not 'F') and 0x44269E is `sub eax,37h`, i.e.
c - 'A' + 10; the lower-case arm is `cmp eax,7Ah` ('z') and `sub eax,57h`.  So
"\xZZ" parses, to (35*16+35)&0xFF.  RTCW lines 409-412 are the same.

The decimal default arm is `lea ecx,[ecx+ecx*4] / lea ecx,[eax+ecx*2-30h]` at
0x4426F5 -- val*10 + c-'0' strength-reduced.  The ScriptError for a non-digit
does NOT return: retail falls straight into the decimal loop afterwards, so an
unknown escape warns and then yields 0.

Spelling only: retail SHARES one `script_p--` / clamp-to-0xFF / ScriptWarning
tail at 0x442713 between the hex and decimal arms, where RTCW's source has two
identical copies; the two copies are kept below because that is the source
shape.  The simple escape arms jump past that tail directly to 0x44273A, so
they get the trailing script_p++ WITHOUT the preceding script_p--.
===============================================================================
*/
int PS_ReadEscapeCharacter( script_t *script, char *ch ) {
	int c, val, i;

	/* step over the leading backslash */
	script->script_p++;
	/* determine the escape character */
	switch ( *script->script_p ) {
	case '\\': c = '\\'; break;
	case 'n': c = '\n'; break;
	case 'r': c = '\r'; break;
	case 't': c = '\t'; break;
	case 'v': c = '\v'; break;
	case 'b': c = '\b'; break;
	case 'f': c = '\f'; break;
	case 'a': c = '\a'; break;
	case '\'': c = '\''; break;
	case '\"': c = '\"'; break;
	case '\?': c = '\?'; break;
	case 'x':
	{
		script->script_p++;
		for ( i = 0, val = 0; ; i++, script->script_p++ ) {
			c = *script->script_p;
			if ( c >= '0' && c <= '9' ) {
				c = c - '0';
			} else if ( c >= 'A' && c <= 'Z' ) {       /* RTCW's, not 'A'-'F' */
				c = c - 'A' + 10;
			} else if ( c >= 'a' && c <= 'z' ) {
				c = c - 'a' + 10;
			} else {
				break;
			}
			val = ( val << 4 ) + c;
		}
		script->script_p--;
		if ( val > 0xFF ) {
			ScriptWarning( script, "too large value in escape character" );
			val = 0xFF;
		}
		c = val;
		break;
	}
	default:     /* NOTE: decimal ASCII code, NOT octal */
	{
		if ( *script->script_p < '0' || *script->script_p > '9' ) {
			ScriptError( script, "unknown escape char" );
		}
		for ( i = 0, val = 0; ; i++, script->script_p++ ) {
			c = *script->script_p;
			if ( c >= '0' && c <= '9' ) {
				c = c - '0';
			} else {
				break;
			}
			val = val * 10 + c;
		}
		script->script_p--;
		if ( val > 0xFF ) {
			ScriptWarning( script, "too large value in escape character" );
			val = 0xFF;
		}
		c = val;
		break;
	}
	}
	/* step over the escape character or the last digit of the number */
	script->script_p++;
	/* store the escape character */
	*ch = c;
	/* succesfully read escape character */
	return 1;
}

/*
===============================================================================
PS_ReadString                      0x004427E0    VERBATIM   (VERIFIED)

RTCW l_script.c:463.  Reads a C-like string; quotes included; two adjacent
strings concatenate.  Against the disassembly:

  ARGUMENT ORDER   __usercall( script@<eax>, token, quote ).  `mov edx,
                   [esp+arg_4]` / `mov ebp, [esp+8+arg_0]` / `mov esi, eax`.
                   Source order, nothing permuted.
  BOUND            0x004428CE is `cmp edi, 3FEh` / `jl` -- signed, and `len`
                   only ever increments from 1, so it cannot arrive negative.
                   Worst case writes index 1022 then the terminator at 1023,
                   inside MAX_TOKEN.  This is RTCW's MAX_TOKEN - 2.
  SIGNEDNESS       `movsx edx, al` at 0x00442848 before comparing with the
                   quote character, so a high-bit byte compares negative and
                   can never close the string.  Falls through to the ordinary
                   copy path, which is the safe direction.  RTCW's `char`
                   comparison compiles to the same thing.

The compiler rotated the MAX_TOKEN test to the bottom of the loop; `len` is 1
on entry so the first iteration cannot need it.
===============================================================================
*/
int PS_ReadString( script_t *script, token_t *token, int quote ) {
	int len, tmpline;
	char *tmpscript_p;

	if ( quote == '\"' ) {
		token->type = TT_STRING;
	} else {
		token->type = TT_LITERAL;
	}

	len = 0;
	/* leading quote */
	token->string[len++] = *script->script_p++;

	while ( 1 ) {
		/* minus 2 because trailing double quote and zero have to be appended */
		if ( len >= MAX_TOKEN - 2 ) {
			ScriptError( script, "string longer than MAX_TOKEN = %d", MAX_TOKEN );
			return 0;
		}
		/* if there is an escape character and escapes are allowed */
		if ( *script->script_p == '\\' && !( script->flags & SCFL_NOSTRINGESCAPECHARS ) ) {
			if ( !PS_ReadEscapeCharacter( script, &token->string[len] ) ) {
				token->string[len] = 0;
				return 0;
			}
			len++;
		}
		/* if a trailing quote */
		else if ( *script->script_p == quote ) {
			/* step over the double quote */
			script->script_p++;
			/* if white spaces in a string are not allowed */
			if ( script->flags & SCFL_NOSTRINGWHITESPACES ) {
				break;
			}
			tmpscript_p = script->script_p;
			tmpline = script->line;
			/* read unusefull stuff between possible two following strings */
			if ( !PS_ReadWhiteSpace( script ) ) {
				script->script_p = tmpscript_p;
				script->line = tmpline;
				break;
			}
			/* if there's no leading double qoute */
			if ( *script->script_p != quote ) {
				script->script_p = tmpscript_p;
				script->line = tmpline;
				break;
			}
			/* step over the new leading double quote */
			script->script_p++;
		} else {
			if ( *script->script_p == '\0' ) {
				token->string[len] = 0;
				ScriptError( script, "missing trailing quote" );
				return 0;
			}
			if ( *script->script_p == '\n' ) {
				token->string[len] = 0;
				ScriptError( script, "newline inside string %s", token->string );
				return 0;
			}
			token->string[len++] = *script->script_p++;
		}
	}
	/* trailing quote */
	token->string[len++] = quote;
	/* end string with a zero */
	token->string[len] = '\0';
	/* the sub type is the length of the string */
	token->subtype = len;
	return 1;
}

/*
===============================================================================
PS_ReadName                        0x00442940    VERBATIM   (VERIFIED)

RTCW l_script.c:546.  Checked against the disassembly:

  ARGUMENT ORDER   __usercall( script@<eax>, token@<edx> ).  Source order.
  BOUND            0x00442963 `cmp ecx, 400h` / `jge` -- signed, counter starts
                   at 0 and only increments.  Writes at the pre-increment index
                   and terminates at the post-increment one, both <= 1023.
  SIGNEDNESS       all four class tests are signed byte compares, so any byte
                   with the high bit set fails every range and ends the name.
                   A UTF-8 or Latin-1 file breaks names at the first non-ASCII
                   byte rather than running on.  RTCW's `char c` -- identical.
===============================================================================
*/
int PS_ReadName( script_t *script, token_t *token ) {
	int len = 0;
	char c;

	token->type = TT_NAME;
	do {
		token->string[len++] = *script->script_p++;
		if ( len >= MAX_TOKEN ) {
			ScriptError( script, "name longer than MAX_TOKEN = %d", MAX_TOKEN );
			return 0;
		}
		c = *script->script_p;
	} while ( ( c >= 'a' && c <= 'z' ) ||
			  ( c >= 'A' && c <= 'Z' ) ||
			  ( c >= '0' && c <= '9' ) ||
			  c == '_' );
	token->string[len] = '\0';
	/* the sub type is the length of the name */
	token->subtype = len;
	return 1;
}

/*
===============================================================================
NumberValue                        0x004429C0    VERBATIM   (VERIFIED)

RTCW l_script.c:574.

RETAIL: __usercall( string@<eax>, subtype@<edx>, intvalue@<esi>,
floatvalue@<edi> ) -- RTCW's source order lifted whole into registers, no
stack arguments at all.  The first six instructions:

    4429C1  xor ecx, ecx          dotfound = 0
    4429C3  test dh, 8            subtype  @edx   (0x0800 = TT_FLOAT)
    4429C6  mov [esi], ecx        intvalue @esi   -- one dword zeroed
    4429C8  mov [edi], ecx        floatvalue @edi -- two dwords zeroed, so it
    4429CA  mov [edi+4], ecx                         is a 64-bit double
    4429CF  mov dl, [eax]         string   @eax

From the listing:
  - the unsigned-to-double idiom at 442A06 (`jge` past `fadd dbl_568F10`, 2^32)
    confirms dotfound is UNSIGNED, as RTCW declares it;
  - dbl_568F20 is 10.0, and `lea ecx,[ecx+ecx*4]` + `shl ecx,1` at 442A10 is
    `dotfound *= 10`;
  - decimal accumulate `lea edx,[edx+edx*4]` + `lea edx,[ecx+edx*2-30h]`;
  - hex is `shl ecx,4` with three digit classes, a-f, A-F and everything -'0';
  - octal `lea edx,[ecx+edx*8-30h]`, binary `lea edx,[ecx+edx*2-30h]`;
  - the `movsx` on every digit is retail's, so a high-bit byte sign-extends.

SPELLING: retail converts the float result with `__ftol2`, a SIGNED truncation,
and stores it into the unsigned intvalue slot.  RTCW spells that
`(unsigned long)*floatvalue`.  Written signed-then-unsigned below so the bit
pattern matches retail at and above 2^31.

RTCW declares this void and its one caller discards EAX; void here.  RTCW's
parameter is `long double *floatvalue`, which MSVC makes a 64-bit double --
token_t's field is 8 bytes, so `double` is the honest spelling.
===============================================================================
*/
void NumberValue( char *string, int subtype, unsigned long *intvalue,
				  double *floatvalue ) {
	unsigned long dotfound = 0;

	*intvalue = 0;
	*floatvalue = 0;

	/* floating point number */
	if ( subtype & TT_FLOAT ) {
		while ( *string ) {
			if ( *string == '.' ) {
				if ( dotfound ) {
					return;                      /* 4429DC: two dots, give up */
				}
				dotfound = 10;
				string++;
			}
			if ( dotfound ) {
				*floatvalue = *floatvalue + (double) ( *string - '0' ) /
							  (double) dotfound;
				dotfound *= 10;
			} else {
				*floatvalue = *floatvalue * 10.0 + (double) ( *string - '0' );
			}
			string++;
		}
		*intvalue = (unsigned long) (long) *floatvalue;   /* __ftol2 */
	} else if ( subtype & TT_DECIMAL ) {
		while ( *string ) {
			*intvalue = *intvalue * 10 + ( *string++ - '0' );
		}
		*floatvalue = (double) *intvalue;
	} else if ( subtype & TT_HEX ) {
		/* step over the leading 0x or 0X */
		string += 2;
		while ( *string ) {
			*intvalue <<= 4;
			if ( *string >= 'a' && *string <= 'f' ) {
				*intvalue += *string - 'a' + 10;
			} else if ( *string >= 'A' && *string <= 'F' ) {
				*intvalue += *string - 'A' + 10;
			} else {
				*intvalue += *string - '0';
			}
			string++;
		}
		*floatvalue = (double) *intvalue;
	} else if ( subtype & TT_OCTAL ) {
		/* step over the first zero */
		string += 1;
		while ( *string ) {
			*intvalue = ( *intvalue << 3 ) + ( *string++ - '0' );
		}
		*floatvalue = (double) *intvalue;
	} else if ( subtype & TT_BINARY ) {
		/* step over the leading 0b or 0B */
		string += 2;
		while ( *string ) {
			*intvalue = ( *intvalue << 1 ) + ( *string++ - '0' );
		}
		*floatvalue = (double) *intvalue;
	}
}

/*
===============================================================================
PS_ReadNumber                      0x00442B20    VERBATIM   (VERIFIED)

RTCW l_script.c:642, with BINARYNUMBERS and NUMBERVALUE both defined -- both
are `#define`d unconditionally in RTCW's l_script.h, so the 0b arm and the
NumberValue call being present is agreement, not a CoD addition.

RETAIL: __usercall( script@<eax>, token@<edx> ).  Source order.  The bounds
are sound: the hex and binary loops stop at MAX_TOKEN and the decimal loop at
MAX_TOKEN - 1, each testing after the write and before the next, so the highest
index ever written, terminator included, is 1023.

DO NOT "FIX" THE HEX DIGIT TEST -- IT IS RTCW's BUG, NOT CoD's.

Retail at 0x00442B92 is

    cmp cl, 41h    ; 'A'
    jl  loc_442BCB ; below 'A' -- reject
    jg  loc_442BCB ; above 'A' -- reject

so both directions leave the loop and only equality falls through: the accepted
hexadecimal alphabet is 0-9, a-f and 'A'.  RTCW-MP src/botlib/l_script.c line
660 is

    ( c >= 'A' && c <= 'A' ) )

which is the same test and the same bug, inherited from Q3.  The binary and
the reference AGREE.

Visible effect, retail's: an uppercase hex literal terminates early, so "0xFF"
scans as the number "0x" followed by a separate name token "FF".
===============================================================================
*/
int PS_ReadNumber( script_t *script, token_t *token ) {
	int len = 0, i;
	int octal, dot;
	char c;

	token->type = TT_NUMBER;
	/* check for a hexadecimal number */
	if ( *script->script_p == '0' &&
		 ( *( script->script_p + 1 ) == 'x' ||
		   *( script->script_p + 1 ) == 'X' ) ) {
		token->string[len++] = *script->script_p++;
		token->string[len++] = *script->script_p++;
		c = *script->script_p;
		/* hexadecimal -- 'A' only on the upper arm, see above */
		while ( ( c >= '0' && c <= '9' ) ||
				( c >= 'a' && c <= 'f' ) ||
				( c >= 'A' && c <= 'A' ) ) {
			token->string[len++] = *script->script_p++;
			if ( len >= MAX_TOKEN ) {
				ScriptError( script, "hexadecimal number longer than MAX_TOKEN = %d", MAX_TOKEN );
				return 0;
			}
			c = *script->script_p;
		}
		token->subtype |= TT_HEX;
	}
	/* check for a binary number */
	else if ( *script->script_p == '0' &&
			  ( *( script->script_p + 1 ) == 'b' ||
				*( script->script_p + 1 ) == 'B' ) ) {
		token->string[len++] = *script->script_p++;
		token->string[len++] = *script->script_p++;
		c = *script->script_p;
		while ( c == '0' || c == '1' ) {
			token->string[len++] = *script->script_p++;
			if ( len >= MAX_TOKEN ) {
				ScriptError( script, "binary number longer than MAX_TOKEN = %d", MAX_TOKEN );
				return 0;
			}
			c = *script->script_p;
		}
		token->subtype |= TT_BINARY;
	} else {  /* decimal or octal integer or floating point number */
		octal = qfalse;
		dot = qfalse;
		if ( *script->script_p == '0' ) {
			octal = qtrue;
		}
		while ( 1 ) {
			c = *script->script_p;
			if ( c == '.' ) {
				dot = qtrue;
			} else if ( c == '8' || c == '9' ) {
				octal = qfalse;
			} else if ( c < '0' || c > '9' ) {
				break;
			}
			token->string[len++] = *script->script_p++;
			if ( len >= MAX_TOKEN - 1 ) {
				ScriptError( script, "number longer than MAX_TOKEN = %d", MAX_TOKEN );
				return 0;
			}
		}
		if ( octal ) {
			token->subtype |= TT_OCTAL;
		} else {
			token->subtype |= TT_DECIMAL;
		}
		if ( dot ) {
			token->subtype |= TT_FLOAT;
		}
	}

	for ( i = 0; i < 2; i++ ) {
		c = *script->script_p;
		/* check for a LONG number */
		if ( ( c == 'l' || c == 'L' ) && !( token->subtype & TT_LONG ) ) {
			script->script_p++;
			token->subtype |= TT_LONG;
		}
		/* check for an UNSIGNED number */
		else if ( ( c == 'u' || c == 'U' ) &&
				  !( token->subtype & ( TT_UNSIGNED | TT_FLOAT ) ) ) {
			script->script_p++;
			token->subtype |= TT_UNSIGNED;
		}
	}
	token->string[len] = '\0';

	NumberValue( token->string, token->subtype, &token->intvalue, &token->floatvalue );

	if ( !( token->subtype & TT_FLOAT ) ) {
		token->subtype |= TT_INTEGER;
	}
	return 1;
}

/*
===============================================================================
PS_ReadLiteral                     0x00442DB0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:753.  Dead for the same reason it is dead in RTCW: PS_ReadToken
routes literals through PS_ReadString, with the PS_ReadLiteral call left in the
source as a comment (RTCW l_script.c:897).  CoD's compiler kept the body.

RETAIL: __usercall( script@<eax>, token@<edi> ).  Source order.
===============================================================================
*/
int PS_ReadLiteral( script_t *script, token_t *token ) {
	token->type = TT_LITERAL;
	/* first quote */
	token->string[0] = *script->script_p++;
	/* check for end of file */
	if ( !*script->script_p ) {
		ScriptError( script, "end of file before trailing \'" );
		return 0;
	}
	/* if it is an escape character */
	if ( *script->script_p == '\\' ) {
		if ( !PS_ReadEscapeCharacter( script, &token->string[1] ) ) {
			return 0;
		}
	} else {
		token->string[1] = *script->script_p++;
	}
	/* check for trailing quote */
	if ( *script->script_p != '\'' ) {
		ScriptWarning( script, "too many characters in literal, ignored" );
		while ( *script->script_p &&
				*script->script_p != '\'' &&
				*script->script_p != '\n' ) {
			script->script_p++;
		}
		if ( *script->script_p == '\'' ) {
			script->script_p++;
		}
	}
	/* store the trailing quote */
	token->string[2] = *script->script_p++;
	/* store trailing zero to end the string */
	token->string[3] = '\0';
	/* the sub type is the integer literal value */
	token->subtype = token->string[1];
	return 1;
}

/*
===============================================================================
PS_ReadPunctuation                 0x00442E90    VERBATIM   (VERIFIED)

RTCW l_script.c:800 with PUNCTABLE defined.  Against the disassembly:

  ARGUMENT ORDER   __usercall( script@<edi>, token on stack ).  Source order.
  END GUARD        0x00442ED1 `cmp ecx, edx` / `ja` -- UNSIGNED, comparing
                   script_p + strlen(punctuation) against end_p.  Unsigned is
                   right for a pointer comparison; this is the one place in the
                   tokenizer where signedness would have mattered and it is
                   already correct.
  strncpy          bounded by MAX_TOKEN, source is a table entry of at most
                   three characters, so the NUL is always written.

The table index is a SIGNED char (`movsx ecx, byte ptr [eax]` at 0x00442E96),
so a byte >= 0x80 indexes off the front of the 256-entry table and follows
whatever dword is there as a list head.  RTCW casts to `(unsigned int)`, which
for a signed char IS sign-extension, so this is not a CoD divergence -- the two
compile to the same instruction.  In practice the table is a 1024-byte
Z_Malloc block whose preceding bytes are heap header, which is why it has never
been seen to fault.  Recorded, not changed.
===============================================================================
*/
int PS_ReadPunctuation( script_t *script, token_t *token ) {
	int len;
	char *p;
	punctuation_t *punc;

	for ( punc = script->punctuationtable[(unsigned int)*script->script_p]; punc; punc = punc->next ) {
		p = punc->p;
		len = strlen( p );
		/* if the script contains at least as much characters as the punctuation */
		if ( script->script_p + len <= script->end_p ) {
			/* if the script contains the punctuation */
			if ( !strncmp( script->script_p, p, len ) ) {
				strncpy( token->string, p, MAX_TOKEN );
				script->script_p += len;
				token->type = TT_PUNCTUATION;
				/* sub type is the number of the punctuation */
				token->subtype = punc->n;
				return 1;
			}
		}
	}
	return 0;
}

/*
===============================================================================
PS_ReadToken                       0x00442FB0    DIVERGED (inlining only)

RTCW l_script.c:862 -- PS_ReadToken, with RTCW's PS_ReadPrimitive (l_script.c:838)
INLINED into the SCFL_PRIMITIVE arm; see the unit banner.

Everything else is verbatim: the tokenavailable fast path, the lastscript_p /
lastline save, the memset of the whole 1072-byte token, the whitespace bracket
recorded on both script and token, and the dispatch order -- string, literal,
number, primitive, name, punctuation.

RETAIL: __usercall( token@<eax>, script@<ecx> ).  SWAPPED with respect to
RTCW's (script, token); source order here (the one call site outside this
unit is l_precomp_mp.c PC_ReadSourceToken, 0x0043DA6A).

VERIFIED 0x00442FB0.  `mov ebx,eax` / `mov esi,ecx` open it.  Both the
fast-path copy and the clear are `mov ecx,10Ch / rep movsd|stosd`, 268 dwords
== 1072 bytes; `add esi,138h` is script->token at +312.  Fields written:
+0x418 whitespace_p (1048), +0x41C endwhitespace_p (1052), +0x420 line (1056),
+0x424 linescrossed (1060), computed as script->line - script->lastline.

Dispatch: `cmp al,22h` then `cmp al,27h` BOTH fall into one
`call PS_ReadString` with the quote character pushed (0x443061 pushes 0x22,
0x443069 pushes 0x27, both reaching 0x44306B), so the literal goes through
PS_ReadString and RTCW's commented-out PS_ReadLiteral call is commented out
here too.  Digits and the `.`-followed-by-digit case follow at 0x443082.
===============================================================================
*/
int PS_ReadToken( script_t *script, token_t *token ) {
	int len;

	/* if there is a token available (from UnreadToken) */
	if ( script->tokenavailable ) {
		script->tokenavailable = 0;
		memcpy( token, &script->token, sizeof( token_t ) );
		return 1;
	}
	/* save script pointer */
	script->lastscript_p = script->script_p;
	/* save line counter */
	script->lastline = script->line;
	/* clear the token stuff */
	memset( token, 0, sizeof( token_t ) );
	/* start of the white space */
	script->whitespace_p = script->script_p;
	token->whitespace_p = script->script_p;
	/* read unusefull stuff */
	if ( !PS_ReadWhiteSpace( script ) ) {
		return 0;
	}
	/* end of the white space */
	script->endwhitespace_p = script->script_p;
	token->endwhitespace_p = script->script_p;
	/* line the token is on */
	token->line = script->line;
	/* number of lines crossed before token */
	token->linescrossed = script->line - script->lastline;

	/* if there is a leading double quote */
	if ( *script->script_p == '\"' ) {
		if ( !PS_ReadString( script, token, '\"' ) ) {
			return 0;
		}
	}
	/* if an literal */
	else if ( *script->script_p == '\'' ) {
		/* RTCW: PS_ReadLiteral is commented out at this site and the literal
		 * goes through PS_ReadString.  CoD does the same. */
		if ( !PS_ReadString( script, token, '\'' ) ) {
			return 0;
		}
	}
	/* if there is a number */
	else if ( ( *script->script_p >= '0' && *script->script_p <= '9' ) ||
			  ( *script->script_p == '.' &&
				( *( script->script_p + 1 ) >= '0' && *( script->script_p + 1 ) <= '9' ) ) ) {
		if ( !PS_ReadNumber( script, token ) ) {
			return 0;
		}
	}
	/* if this is a primitive script -- RTCW calls PS_ReadPrimitive here; the
	 * retail compiler inlined it, so the whole body is spelled out. */
	else if ( script->flags & SCFL_PRIMITIVE ) {
		len = 0;
		while ( *script->script_p > ' ' && *script->script_p != ';' ) {
			if ( len >= MAX_TOKEN ) {
				ScriptError( script, "primitive token longer than MAX_TOKEN = %d", MAX_TOKEN );
				return 0;
			}
			token->string[len++] = *script->script_p++;
		}
		token->string[len] = 0;
		/* copy the token into the script structure */
		memcpy( &script->token, token, sizeof( token_t ) );
		/* primitive reading successfull */
		return 1;
	}
	/* if there is a name */
	else if ( ( *script->script_p >= 'a' && *script->script_p <= 'z' ) ||
			  ( *script->script_p >= 'A' && *script->script_p <= 'Z' ) ||
			  *script->script_p == '_' ) {
		if ( !PS_ReadName( script, token ) ) {
			return 0;
		}
	}
	/* check for punctuations */
	else if ( !PS_ReadPunctuation( script, token ) ) {
		ScriptError( script, "can't read token" );
		return 0;
	}
	/* copy the token into the script structure */
	memcpy( &script->token, token, sizeof( token_t ) );
	/* succesfully read a token */
	return 1;
}

/*
===============================================================================
PS_ExpectTokenString                0x00443120    VERBATIM   (dead: no xrefs)

RTCW l_script.c:938.  `mov esi, ecx` at 0x00443146 loads the script.

RETAIL: __usercall( script@<ecx>, string on stack ).  Source order.
===============================================================================
*/
int PS_ExpectTokenString( script_t *script, char *string ) {
	token_t token;

	if ( !PS_ReadToken( script, &token ) ) {
		ScriptError( script, "couldn't find expected %s", string );
		return 0;
	}

	if ( strcmp( token.string, string ) ) {
		ScriptError( script, "expected %s, found %s", string, token.string );
		return 0;
	}
	return 1;
}

/*
===============================================================================
PS_ExpectTokenType                  0x004431E0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:958.  Dead in the retail binary: its only two callers,
ReadSignedFloat and ReadSignedInt, are themselves dead.

RETAIL: __usercall( script@<edx>, type@<ecx>, subtype@<ebx>, token on stack ).
Source order, first three lifted into registers.  EBX is read and never
written, which is why the prologue does not save it.

THE PUNCTUATION ERROR PASSES A STRUCT BY VALUE, AND THAT IS RTCW's LINE.
RTCW writes

    ScriptError( script, "expected %s, found %s",
                 script->punctuations[subtype], token->string );

where `script->punctuations[subtype]` is a punctuation_t BY VALUE.  MSVC pushes
all twelve bytes, so the format's first %s gets `.p`, its second %s gets `.n` --
an int rendered as a string -- and `token->string` is a third vararg nothing
consumes.  The disassembly at 0x004432D5 pushes exactly that.  Written as
RTCW writes it, so it compiles to the same push sequence.  Do not "fix" it
into `.p`: that changes the instruction stream and the bug is id's.

The `str` buffer is MAX_TOKEN and is built by strcpy/strcat with no bound, and
in the type-mismatch arm it is left UNINITIALISED when `type` is none of the
five known values.  RTCW has both.
===============================================================================
*/
int PS_ExpectTokenType( script_t *script, int type, int subtype, token_t *token ) {
	char str[MAX_TOKEN];

	if ( !PS_ReadToken( script, token ) ) {
		ScriptError( script, "couldn't read expected token" );
		return 0;
	}

	if ( token->type != type ) {
		if ( type == TT_STRING ) {
			strcpy( str, "string" );
		}
		if ( type == TT_LITERAL ) {
			strcpy( str, "literal" );
		}
		if ( type == TT_NUMBER ) {
			strcpy( str, "number" );
		}
		if ( type == TT_NAME ) {
			strcpy( str, "name" );
		}
		if ( type == TT_PUNCTUATION ) {
			strcpy( str, "punctuation" );
		}
		ScriptError( script, "expected a %s, found %s", str, token->string );
		return 0;
	}
	if ( token->type == TT_NUMBER ) {
		if ( ( token->subtype & subtype ) != subtype ) {
			if ( subtype & TT_DECIMAL ) {
				strcpy( str, "decimal" );
			}
			if ( subtype & TT_HEX ) {
				strcpy( str, "hex" );
			}
			if ( subtype & TT_OCTAL ) {
				strcpy( str, "octal" );
			}
			if ( subtype & TT_BINARY ) {
				strcpy( str, "binary" );
			}
			if ( subtype & TT_LONG ) {
				strcat( str, " long" );
			}
			if ( subtype & TT_UNSIGNED ) {
				strcat( str, " unsigned" );
			}
			if ( subtype & TT_FLOAT ) {
				strcat( str, " float" );
			}
			if ( subtype & TT_INTEGER ) {
				strcat( str, " integer" );
			}
			ScriptError( script, "expected %s, found %s", str, token->string );
			return 0;
		}
	} else if ( token->type == TT_PUNCTUATION ) {
		if ( subtype < 0 ) {
			ScriptError( script, "BUG: wrong punctuation subtype" );
			return 0;
		}
		if ( token->subtype != subtype ) {
			/* struct passed by value -- see the note above.  id's line. */
			ScriptError( script, "expected %s, found %s",
						 script->punctuations[subtype], token->string );
			return 0;
		}
	}
	return 1;
}

/*
===============================================================================
PS_ExpectAnyToken                   0x004434B0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1034.  Retail neither loads EAX nor ECX before calling
PS_ReadToken -- it forwards its own two register arguments untouched -- so the
binding is the same as PS_ReadToken's.

RETAIL: __usercall( token@<eax>, script@<ecx> ).  SWAPPED; source order here.
===============================================================================
*/
int PS_ExpectAnyToken( script_t *script, token_t *token ) {
	if ( !PS_ReadToken( script, token ) ) {
		ScriptError( script, "couldn't read expected token" );
		return 0;
	} else {
		return 1;
	}
}

/*
===============================================================================
PS_CheckTokenString                 0x004434E0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1050.  RETAIL: __usercall( script@<edi>, string on stack ).
===============================================================================
*/
int PS_CheckTokenString( script_t *script, char *string ) {
	token_t tok;

	if ( !PS_ReadToken( script, &tok ) ) {
		return 0;
	}
	/* if the token is available */
	if ( !strcmp( tok.string, string ) ) {
		return 1;
	}
	/* token not available */
	script->script_p = script->lastscript_p;
	return 0;
}

/*
===============================================================================
PS_CheckTokenType                   0x00443590    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1070.  RETAIL: __usercall( script@<edx>, type on stack arg_0,
subtype@<ecx>, token on stack arg_4 ) -- source order with the first and third
arguments lifted, which leaves type and token on the stack in source order.
===============================================================================
*/
int PS_CheckTokenType( script_t *script, int type, int subtype, token_t *token ) {
	token_t tok;

	if ( !PS_ReadToken( script, &tok ) ) {
		return 0;
	}
	/* if the type matches */
	if ( tok.type == type && ( tok.subtype & subtype ) == subtype ) {
		memcpy( token, &tok, sizeof( token_t ) );
		return 1;
	}
	/* token is not available */
	script->script_p = script->lastscript_p;
	return 0;
}

/*
===============================================================================
PS_SkipUntilString                  0x00443620    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1092.  RETAIL: __usercall( string@<edi>, script on stack ).
SWAPPED; source order here.
===============================================================================
*/
int PS_SkipUntilString( script_t *script, char *string ) {
	token_t token;

	while ( PS_ReadToken( script, &token ) ) {
		if ( !strcmp( token.string, string ) ) {
			return 1;
		}
	}
	return 0;
}

/*
===============================================================================
PS_UnreadLastToken                  0x004436C0    VERBATIM   (dead: no xrefs)
PS_UnreadToken                      0x004436D0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1109 / :1118.  RETAIL: script@<eax>, and for the second
token@<edx>.  Source order.  The 0x430-byte `rep movsd` at 0x004436DF is one of
the three sites that pin sizeof(token_t) at 1072.
===============================================================================
*/
void PS_UnreadLastToken( script_t *script ) {
	script->tokenavailable = 1;
}

void PS_UnreadToken( script_t *script, token_t *token ) {
	memcpy( &script->token, token, sizeof( token_t ) );
	script->tokenavailable = 1;
}

/*
===============================================================================
PS_NextWhiteSpaceChar               0x004436F0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1129.  RETAIL: __usercall( script@<edx> ).  ECX is untouched
(a one-register __usercall, which MSVC would spell as __fastcall with a dead
first parameter); written with the one real parameter here.
===============================================================================
*/
char PS_NextWhiteSpaceChar( script_t *script ) {
	if ( script->whitespace_p != script->endwhitespace_p ) {
		return *script->whitespace_p++;
	} else {
		return 0;
	}
}

/*
===============================================================================
StripDoubleQuotes                   0x00443710    VERBATIM   (VERIFIED)
StripSingleQuotes                   0x00443760    VERBATIM   (VERIFIED)

RTCW l_script.c:1144 / :1158.

CONVENTION -- MUST STAY __fastcall.  The retail bodies read the string out of
EDX and never touch ECX; the `lea ecx, [ecx+0]` at 0x0044371D and 0x0044376D is
a three-byte NOP, not a use.  So this is __usercall(string@<edx>), which MSVC
spells as __fastcall with a dead first parameter.  A __fastcall definition
decorates as @StripDoubleQuotes@8; an `extern int StripDoubleQuotes();` would
emit a call to the cdecl spelling _StripDoubleQuotes, which nothing defines, so
both callers take the prototype from l_script.h.

CONSTNESS.  Retail writes through the parameter -- `mov [esi+eax], cl` at
0x00443722 and `mov [eax+edx-1], cl` at 0x0044374E -- so it is `char *`, not
`const char *`.  RETURN VALUE: EAX holds strlen measured AFTER the opening quote
is shifted out and BEFORE the closing one is overwritten.  Neither caller uses
it and RTCW declares these void; kept as unsigned int to match the binary.

DIVERGENCE (deliberate, one instruction).  Retail tests the last character with
`cmp byte ptr [eax+edx-1], 22h` using the measured length unguarded, so an empty
string reads -- and can then write a NUL to -- the byte before the buffer.
RTCW has the identical unguarded `string[strlen(string) - 1]`.  Unreachable
today: both callers gate on token.type == TT_STRING and PS_ReadString always
deposits at least the opening quote.  The `len > 0` guard below therefore cannot
change behaviour on any input retail could produce, and it removes a one-byte
underflow from a parser that eats untrusted files.  Remove it if bit-exactness
ever matters more than that.
===============================================================================
*/
unsigned int __fastcall StripDoubleQuotes( int unused, char *string ) {
	char *p;
	unsigned int len;

	(void)unused;

	if ( string[0] == '\"' ) {
		/* shift the whole string down one byte, NUL included.  RTCW spells
		 * this `strcpy(string, string + 1)`, an overlapping strcpy; retail
		 * emits the byte loop, which is what MSVC's intrinsic becomes. */
		for ( p = string + 1; ; p++ ) {
			p[-1] = p[0];
			if ( !p[0] ) {
				break;
			}
		}
	}

	len = strlen( string );
	if ( len > 0 && string[len - 1] == '\"' ) {
		string[len - 1] = 0;
	}
	return len;
}

/* VERIFIED 0x00443760.  StripDoubleQuotes with 0x27 for 0x22 throughout:
 * `cmp byte ptr [edx],27h` opens it, so string@<edx>; the `lea ecx,[ecx+0]` at
 * 0x0044376D is the same three-byte NOP and ECX is again untouched; the
 * last-character test is the same unguarded `cmp byte ptr [eax+edx-1],27h`.
 *
 * Retail recomputes strlen a THIRD time at 0x00443790 before the terminating
 * store, where `len` is reused below; the string cannot change between the
 * two. */
unsigned int __fastcall StripSingleQuotes( int unused, char *string ) {
	char *p;
	unsigned int len;

	(void)unused;

	if ( string[0] == '\'' ) {
		for ( p = string + 1; ; p++ ) {
			p[-1] = p[0];
			if ( !p[0] ) {
				break;
			}
		}
	}

	len = strlen( string );
	if ( len > 0 && string[len - 1] == '\'' ) {
		string[len - 1] = 0;
	}
	return len;
}

/*
===============================================================================
ReadSignedFloat                     0x004437B0    VERBATIM   (dead: no xrefs)
ReadSignedInt                       0x00443880    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1172 / :1192.  Both call PS_ExpectAnyToken, which the compiler
INLINED -- retail opens with PS_ReadToken followed by
`ScriptError(script, "couldn't read expected token")`, which is that function's
whole body.

ReadSignedFloat returns a double.  Retail ends with

    4437B0  ...                              sign = 1.0  (a 64-bit double)
    44385A  fld  [esp+444h+var_24]           token.floatvalue  (token + 1040)
    443868  fmul [esp+444h+var_43C]          * sign

and the negative arm at 0x00443825 builds { eax, 0BFF00000h } = -1.0.  So it
returns a double, exactly as RTCW's `long double ReadSignedFloat`.

ReadSignedInt's `token.subtype == TT_FLOAT` -- an equality test where a mask
test is meant -- is RTCW's, line 1201.

RETAIL: both are plain __cdecl with the script as the one stack argument.  The
only two functions in this unit that are not __usercall.
===============================================================================
*/
double ReadSignedFloat( script_t *script ) {
	token_t token;
	double sign = 1;

	PS_ExpectAnyToken( script, &token );
	if ( !strcmp( token.string, "-" ) ) {
		sign = -1;
		PS_ExpectTokenType( script, TT_NUMBER, 0, &token );
	} else if ( token.type != TT_NUMBER ) {
		ScriptError( script, "expected float value, found %s\n", token.string );
	}
	return sign * token.floatvalue;
}

signed long int ReadSignedInt( script_t *script ) {
	token_t token;
	signed long int sign = 1;

	PS_ExpectAnyToken( script, &token );
	if ( !strcmp( token.string, "-" ) ) {
		sign = -1;
		PS_ExpectTokenType( script, TT_NUMBER, TT_INTEGER, &token );
	} else if ( token.type != TT_NUMBER || token.subtype == TT_FLOAT ) {
		ScriptError( script, "expected integer value, found %s\n", token.string );
	}
	return sign * token.intvalue;
}

/*
===============================================================================
SetScriptFlags                      0x00443960    VERBATIM   (dead: no xrefs)
GetScriptFlags                      0x00443970    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1212 / :1221.  RETAIL: flags@<eax>, script@<ecx> -- SWAPPED;
and script@<eax>.  Source order here.
===============================================================================
*/
void SetScriptFlags( script_t *script, int flags ) {
	script->flags = flags;
}

int GetScriptFlags( script_t *script ) {
	return script->flags;
}

/*
===============================================================================
ResetScript                         0x00443980    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1230.  Every one of the nine stores lines up field for field.
RETAIL: __usercall( script@<edx> ).
===============================================================================
*/
void ResetScript( script_t *script ) {
	/* pointer in script buffer */
	script->script_p = script->buffer;
	/* pointer in script buffer before reading token */
	script->lastscript_p = script->buffer;
	/* begin of white space */
	script->whitespace_p = NULL;
	/* end of white space */
	script->endwhitespace_p = NULL;
	/* set if there's a token available in script->token */
	script->tokenavailable = 0;
	script->line = 1;
	script->lastline = 1;
	/* clear the saved token */
	memset( &script->token, 0, sizeof( token_t ) );
}

/*
===============================================================================
EndOfScript                         0x004439D0    VERBATIM   (dead: no xrefs)
NumLinesCrossed                     0x004439E0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1254 / :1263.  RETAIL: script@<eax>, script@<ecx>.
The `sbb eax,eax / inc eax` at 0x004439DC is the compiler's boolean, and it is
an UNSIGNED pointer comparison, which is right.
===============================================================================
*/
int EndOfScript( script_t *script ) {
	return script->script_p >= script->end_p;
}

int NumLinesCrossed( script_t *script ) {
	return script->line - script->lastline;
}

/*
===============================================================================
ScriptSkipTo                        0x004439F0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1272.  The compiler peeled the first PS_ReadWhiteSpace call out
of the do/while.  RETAIL: __usercall( value@<edi>, script@<esi> ).  SWAPPED;
source order here.
===============================================================================
*/
int ScriptSkipTo( script_t *script, char *value ) {
	int len;
	char firstchar;

	firstchar = *value;
	len = strlen( value );
	do {
		if ( !PS_ReadWhiteSpace( script ) ) {
			return 0;
		}
		if ( *script->script_p == firstchar ) {
			if ( !strncmp( script->script_p, value, len ) ) {
				return 1;
			}
		}
		script->script_p++;
	} while ( 1 );
}

/* LoadScriptFile (0x00443A60) is in botlib/l_precomp_pc.c, beside
 * LoadSourceFile and the PC_* handle API.  One stack argument.  RTCW
 * l_script.c:1316, VERBATIM apart from botimport.FS_* becoming direct FS_*
 * calls. */

/*
===============================================================================
LoadScriptMemory                    0x00443BF0    VERBATIM

RTCW l_script.c:1388, with GetClearedMemory and SetScriptPunctuations both
inlined.  Retail zeroes the whole block (length + 1393) and then zeroes the
1392-byte header a second time -- RTCW's GetClearedMemory followed by RTCW's
own `memset(script, 0, sizeof(script_t))`, the redundant pair intact.

`lea edi,[esi+571h]` at 0x00443BFA is sizeof(script_t) + length + 1 with
sizeof(script_t) = 1392, which is one of the two sites that pin it.

`strcpy(script->filename, name)` is unbounded into a 260-byte field.  RTCW's
filename is 1024 and the line is the same; CoD shrank the field, so the
overflow arrives sooner.  Both callers pass a literal or a define name.

RETAIL: __usercall( length@<eax>, ptr on stack arg_0, name on stack arg_4 ) --
RTCW's (ptr, length, name) with `length` lifted out of the middle.  Source order
here.

VERIFIED 0x00443BF0.  It is LoadScriptFile's body with the file read replaced
by a memcpy: identical allocation (`lea edi,[esi+571h]` / `lea eax,[edi+4]`),
identical double zeroing, identical inlined strcpy, and the same nine field
stores at +0x104/108/10C/110/11C/120/124/128/130.

TWO REAL DIFFERENCES FROM LoadScriptFile:
there is NO Com_Compress call here -- the text is taken as given -- and
script->length is therefore never rewritten, so end_p and length stay
consistent with each other rather than deliberately diverging.
===============================================================================
*/
script_t *LoadScriptMemory( char *ptr, int length, char *name ) {
	void *buffer;
	script_t *script;

	buffer = PS_GetMemory( sizeof( script_t ) + length + 1 );
	memset( buffer, 0, sizeof( script_t ) + length + 1 );
	script = (script_t *) buffer;
	memset( script, 0, sizeof( script_t ) );
	strcpy( script->filename, name );
	script->buffer = (char *) buffer + sizeof( script_t );
	script->buffer[length] = 0;
	script->length = length;
	/* pointer in script buffer */
	script->script_p = script->buffer;
	/* pointer in script buffer before reading token */
	script->lastscript_p = script->buffer;
	/* pointer to end of script buffer */
	script->end_p = &script->buffer[length];
	/* set if there's a token available in script->token */
	script->tokenavailable = 0;
	script->line = 1;
	script->lastline = 1;
	/* SetScriptPunctuations( script, NULL ) -- inlined by retail */
	PS_CreatePunctuationTable( script, default_punctuations );
	script->punctuations = default_punctuations;

	memcpy( script->buffer, ptr, length );

	return script;
}

/*
===============================================================================
FreeScript                          0x00443CD0    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1423 with PUNCTABLE defined and FreeMemory inlined at both
sites.  Dead in retail because FreeSource inlines it, the same way
LoadScriptFile inlines SetScriptPunctuations.

RETAIL: __usercall( script@<esi> ).
===============================================================================
*/
void FreeScript( script_t *script ) {
	if ( script->punctuationtable ) {
		PS_FreeMemory( script->punctuationtable );
	}
	PS_FreeMemory( script );
}

/*
===============================================================================
PS_SetBaseFolder                    0x00443D10    VERBATIM   (dead: no xrefs)

RTCW l_script.c:1437, non-BSPC arm.  Byte-for-byte identical to
PC_SetBaseFolder at 0x00442270, down to the same 64-byte buffer at
pc_baseFolder -- the retail build compiled the same helper into both botlib
translation units and both write the same storage.

RETAIL: __usercall( path@<eax> ).  Com_sprintf's dest and size arrive in EDI and
ESI.

RTCW passes `path` as the FORMAT string, so a '%' in the path is a format
directive.  Retail does the same (`push eax` is the only pushed argument at
0x00443D12); left alone under the binary-wins rule.
===============================================================================
*/
void PS_SetBaseFolder( char *path ) {
	Com_sprintf( basefolder, 64, path );
}
