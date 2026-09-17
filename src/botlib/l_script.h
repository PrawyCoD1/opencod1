/*
 * botlib tokenizer records -- reconstructed from CoD 1.1 (CoDMP.exe).
 *
 * Retail bodies at 0x004422C0-0x00443D28; see l_script_mp.c.
 *
 * token_t is 1072 contiguous bytes; callees memcpy 0x430 bytes in and out of
 * it.  Size:
 *   qmemcpy( script + 312, token, 0x430 )  PS_ReadToken       0x0044310F
 *   qmemcpy( script + 312, token, 0x430 )  PS_UnreadToken     0x004436D0
 *   memset ( token, 0, 0x430 )             PS_ReadToken       0x00442FE9
 *   Z_MallocInternal( 0x434 )              PC_CopyToken       0x0043D9D0
 *     (0x434 = 4-byte heap magic + 0x430 payload)
 * and every function that builds one on the stack places it at [ebp-434h]
 * running to the security cookie at [ebp-4]: 0x434 - 0x4 = 0x430.
 *
 * Field offsets, with the retail access that fixes each:
 *   +1024 type            cmp [esp+43Ch+var_34], 4      PC_Directive_undef 0x0043EECC
 *   +1028 subtype         mov [ebp+404h], edi           PS_ReadString      0x004428A5
 *   +1032 intvalue        NumberValue(..., token+1032)  PS_ReadNumber      0x00442D89
 *   +1040 floatvalue      fld [esp+444h+var_24]         ReadSignedFloat    0x0044385A
 *                         -- 8 bytes.  Q3 declares this `long double`, which
 *                         MSVC makes a 64-bit double; the 4-byte hole at
 *                         +1036 is its alignment padding and is real.
 *   +1048 whitespace_p    a1[262] = script->script_p    PS_ReadToken       0x00442FF8
 *   +1052 endwhitespace_p a1[263] = script->script_p    PS_ReadToken       0x0044300D
 *   +1056 line            a1[264] = script->line        PS_ReadToken       0x00443016
 *   +1060 linescrossed    a1[265] = line - lastline     PS_ReadToken       0x00443022
 *   +1064 next            mov [eax+428h], edx           PC_Directive_undef 0x0043EEE6
 *
 * The layout is Q3/RTCW's token_t unchanged.  No CoD divergence found.
 *
 * script_t is 1392 bytes: LoadScriptFile at
 * 0x00443AF3 allocates `filelength + 0x571 + 4`, i.e. sizeof(script_t) = 1392
 * with the file text and its NUL following in the same block.  1392 also falls
 * out of the field offsets: token at +312 plus 1072 is 1384, `next` at +1384,
 * padded to the 8-byte alignment the embedded double forces.
 *
 * filename[260] is derived by subtraction: `buffer` is pinned at +260 only by
 * `script_p` being pinned at +264.  RTCW declares filename[1024]; CoD shrank it.
 *
 * @fidelity: verified
 */

#ifndef __L_SCRIPT_H__
#define __L_SCRIPT_H__

#define MAX_TOKEN 1024

/* script->flags, and the retail tests that fix each bit:
 *   0x01  `test byte ptr [esi+12Ch], 1`   ScriptError            0x00442409
 *   0x02  `test byte ptr [esi+12Ch], 2`   ScriptWarning          0x00442479
 *   0x04  `test byte ptr [eax+12Ch], 4`   PS_ReadString          0x00442879
 *   0x08  `test byte ptr [eax+12Ch], 8`   PS_ReadString          0x00442816
 *   0x10  `test byte ptr [esi+12Ch], 10h` PS_ReadToken           0x00443056
 * All five agree with RTCW's l_script.h numbering.  SCFL_NOBINARYNUMBERS and
 * SCFL_NONUMBERVALUES are declared there too but no retail function tests
 * them -- RTCW does not test them either. */
#define SCFL_NOERRORS               0x0001
#define SCFL_NOWARNINGS             0x0002
#define SCFL_NOSTRINGWHITESPACES    0x0004
#define SCFL_NOSTRINGESCAPECHARS    0x0008
#define SCFL_PRIMITIVE              0x0010
#define SCFL_NOBINARYNUMBERS        0x0020
#define SCFL_NONUMBERVALUES         0x0040

/* token->type */
#define TT_STRING           1
#define TT_LITERAL          2
#define TT_NUMBER           3
#define TT_NAME             4
#define TT_PUNCTUATION      5

/* token->subtype, for type == TT_NUMBER.  PS_ReadNumber (0x00442B20) ors in
 * 0x100 at 0x00442B7B, then 0x400, 0x200, 8, 0x800, 0x2000, 0x4000 and 0x1000;
 * NumberValue (0x004429C0) tests `dh,8 / dl,8 / dh,1 / dh,2 / dh,4`.  They
 * agree with Q3/RTCW exactly; no CoD divergence in the numbering. */
#define TT_DECIMAL     0x0008
#define TT_HEX         0x0100
#define TT_OCTAL       0x0200
#define TT_BINARY      0x0400
#define TT_FLOAT       0x0800
#define TT_INTEGER     0x1000
#define TT_LONG        0x2000
#define TT_UNSIGNED    0x4000

typedef struct token_s {
	char string[MAX_TOKEN];         /* +0    */
	int type;                       /* +1024 */
	int subtype;                    /* +1028 */
	unsigned long intvalue;         /* +1032 */
	                                /* +1036 -- alignment hole, real */
	double floatvalue;              /* +1040 -- Q3 spells this `long double` */
	char *whitespace_p;             /* +1048 */
	char *endwhitespace_p;          /* +1052 */
	int line;                       /* +1056 */
	int linescrossed;               /* +1060 */
	struct token_s *next;           /* +1064 */
} token_t;                          /* 1072 bytes */

COD1_ASSERT_SIZE( token_t, 1072 );

typedef struct punctuation_s {
	char *p;                        /* +0  */
	int n;                          /* +4  */
	struct punctuation_s *next;     /* +8  */
} punctuation_t;                    /* 12 bytes -- the stride of the
                                       PunctuationFromNum walk */

/* The default punctuation table.  Retail keeps it in .data at 0x0057A820,
 * 64 bytes into the unnamed 88-byte blob at 0x0057A7E0.  Defined in
 * l_precomp_pc.c. */
extern struct punctuation_s default_punctuations[];

typedef struct script_s {
	char filename[260];             /* +0    */
	char *buffer;                   /* +260  */
	char *script_p;                 /* +264  */
	char *end_p;                    /* +268  */
	char *lastscript_p;             /* +272  */
	char *whitespace_p;             /* +276  */
	char *endwhitespace_p;          /* +280  */
	int length;                     /* +284  */
	int line;                       /* +288  */
	int lastline;                   /* +292  */
	int tokenavailable;             /* +296  */
	int flags;                      /* +300  */
	punctuation_t *punctuations;    /* +304  */
	punctuation_t **punctuationtable;/* +308 */
	token_t token;                  /* +312  */
	struct script_s *next;          /* +1384 */
} script_t;                         /* 1392 bytes */

COD1_ASSERT_SIZE( script_t, 1392 );

/*
 * source_t -- the precompiler's per-file state.  1624 bytes, pinned by
 * `push 65Ch` at LoadSourceFile 0x00441D3E (0x65C = 4-byte heap magic + 1624).
 * Field offsets read off LoadSourceFile's stores and PC_SetPunctuations:
 *
 *   +520 punctuations   mov [a2+208h], eax          PC_SetPunctuations 0x00441D10
 *   +524 scriptstack    mov [esi+20Ch], ebx         LoadSourceFile     0x00441D82
 *   +528 tokens         mov [esi+210h], ebp
 *   +532 defines        mov [esi+214h], ebp
 *   +536 definehash     mov [esi+218h], ebp   (0x1000-byte table, 1024 slots)
 *   +540 indentstack    mov [esi+21Ch], ebp
 *   +544 skip           mov [esi+220h], ebp
 *
 * filename and includepath are 260 each rather than RTCW's 1024, which is what
 * puts punctuations at 520; the same 260 that l_script.h's script_t uses, and
 * the same 0x104 LoadSourceFile passes to strncpy at 0x00441D71.  The embedded
 * token_t forces 8-byte alignment, so `skip` at +544 is followed by four bytes
 * of padding and the record ends 1624 bytes in.
 */
typedef struct source_s {
	char filename[260];             /* +0    */
	char includepath[260];          /* +260  */
	struct punctuation_s *punctuations; /* +520 */
	script_t *scriptstack;          /* +524  */
	token_t *tokens;                /* +528  */
	void *defines;                  /* +532  */
	void **definehash;              /* +536  */
	void *indentstack;              /* +540  */
	int skip;                       /* +544  */
	int _pad548;                    /* +548 -- alignment for the token below */
	token_t token;                  /* +552  */
} source_t;                         /* 1624 bytes */

COD1_ASSERT_SIZE( source_t, 1624 );

/*
 * pc_token_t -- the cut-down record handed across the VM boundary.  Read off
 * PC_ReadTokenHandle's stores through EDI at 0x004421BF-0x004421D1:
 *   +0  type        +4  subtype        +8  intvalue
 *   +12 floatvalue  -- `fstp dword ptr [edi+0Ch]`, so the token_t's 64-bit
 *                      floatvalue is NARROWED to 32 bits here
 *   +16 string      -- plain strcpy, unbounded, exactly as Q3
 */
typedef struct pc_token_s {
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKEN];
} pc_token_t;

#define MAX_SOURCEFILES 64

/* l_precomp_mp.c 0x0043F660 -- __fastcall, with the define string arriving in
 * EDX and ECX dead, the same shape as StripDoubleQuotes.  Both syscall
 * dispatchers reach it with a bare `mov edx,[ebp+4]` and no push
 * (0x00418B23, 0x00403298).  The convention has to be in the prototype: a
 * plain `extern int PC_AddGlobalDefine();` emits a call to the undecorated
 * cdecl name, which the real body does not answer. */
int __fastcall PC_AddGlobalDefine( int unused, const char *string );

/* l_precomp_pc.c -- the menu/script loader front end.  All three of these are
 * __usercall in retail and are given SOURCE order here; see that file. */
script_t *LoadScriptFile( const char *filename );
/* Also l_precomp_pc.c.  Plain __cdecl, one stack argument --
 * `mov ebp, [esp+43Ch+arg_0]` at 0x00440FF4. */
int PC_ReadDirective( void *source );        /* 0x00440FE0 */
int PC_ReadDollarDirective( void *source );  /* 0x00441350 */
source_t *LoadSourceFile( const char *filename );
int PC_LoadSourceHandle( const char *filename );
int PC_FreeSourceHandle( int handle );
int PC_ReadTokenHandle( int handle, pc_token_t *pc_token );
int PC_SourceFileAndLine( int handle, char *filename, int *line );
void PC_CheckOpenSourceHandles( void );

/*
 * l_script_mp.c -- the tokenizer.  Every one of these is RTCW's
 * botlib/l_script.c and is declared here in RTCW SOURCE order; see the banner
 * in l_script_mp.c for the retail register binding of each.
 *
 * PS_ReadToken (0x00442FB0) is RTCW's PS_ReadToken with PS_ReadPrimitive
 * inlined into its SCFL_PRIMITIVE arm; there is no separate PS_ReadPrimitive
 * in the binary.
 */
void    PS_CreatePunctuationTable( script_t *script, punctuation_t *punctuations );
char   *PunctuationFromNum( script_t *script, int num );
void QDECL ScriptError( script_t *script, char *str, ... );
void QDECL ScriptWarning( script_t *script, char *str, ... );
void    SetScriptPunctuations( script_t *script, punctuation_t *p );
int     PS_ReadWhiteSpace( script_t *script );
int     PS_ReadEscapeCharacter( script_t *script, char *ch );
int     PS_ReadString( script_t *script, token_t *token, int quote );
int     PS_ReadName( script_t *script, token_t *token );
void    NumberValue( char *string, int subtype, unsigned long *intvalue, double *floatvalue );
int     PS_ReadNumber( script_t *script, token_t *token );
int     PS_ReadLiteral( script_t *script, token_t *token );
int     PS_ReadPunctuation( script_t *script, token_t *token );
int     PS_ReadToken( script_t *script, token_t *token );
int     PS_ExpectTokenString( script_t *script, char *string );
int     PS_ExpectTokenType( script_t *script, int type, int subtype, token_t *token );
int     PS_ExpectAnyToken( script_t *script, token_t *token );
int     PS_CheckTokenString( script_t *script, char *string );
int     PS_CheckTokenType( script_t *script, int type, int subtype, token_t *token );
int     PS_SkipUntilString( script_t *script, char *string );
void    PS_UnreadLastToken( script_t *script );
void    PS_UnreadToken( script_t *script, token_t *token );
char    PS_NextWhiteSpaceChar( script_t *script );
double  ReadSignedFloat( script_t *script );
signed long int ReadSignedInt( script_t *script );
void    SetScriptFlags( script_t *script, int flags );
int     GetScriptFlags( script_t *script );
void    ResetScript( script_t *script );
int     EndOfScript( script_t *script );
int     NumLinesCrossed( script_t *script );
int     ScriptSkipTo( script_t *script, char *value );
script_t *LoadScriptMemory( char *ptr, int length, char *name );
void    FreeScript( script_t *script );
void    PS_SetBaseFolder( char *path );

/* StripDoubleQuotes / StripSingleQuotes are __fastcall in retail with the
 * string arriving in EDX and ECX dead -- see the note on their definitions in
 * l_script_mp.c.  The prototype MUST carry the convention: a plain
 * `extern int StripDoubleQuotes();` emits a call to the undecorated cdecl
 * _StripDoubleQuotes, which the real body does not answer. */
unsigned int __fastcall StripDoubleQuotes( int unused, char *string );
unsigned int __fastcall StripSingleQuotes( int unused, char *string );

#endif /* __L_SCRIPT_H__ */
