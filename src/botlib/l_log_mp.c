/*
 * botlib -- the log file.
 *
 * Retail range 0x0043D520-0x0043D690, 7 functions.
 *
 * @fidelity: verified
 * @fidelity-default: verified
 *
 * ===========================================================================
 * RTCW CORRESPONDENCE -- THIS UNIT IS RTCW's botlib/l_log.c
 * ===========================================================================
 *
 * All seven functions are VERBATIM apart from the print substitution below.
 * RTCW ships eight; CoD drops Log_WriteTimeStamped, which is the only one that
 * needs `botlibglobals.time`, and CoD has no botlibglobals.
 *
 *   0x0043D520  Log_AlwaysOpen  (l_log.c:64)   -- the opener that owns the fopen
 *   0x0043D5C0  Log_Open        (l_log.c:88)   -- the LibVarValue("log","0") wrapper
 *   0x0043D650  Log_Write       (l_log.c:130)  -- `vfprintf; fflush` only
 *
 * PC_PrintDefineHashTable (0x0043E0C0) is RTCW l_precomp.c:522 verbatim and
 * uses Log_Write for all three of its prints; Log_WriteTimeStamped emits a
 * time stamp first and is not in this binary.
 *
 * DIVERGENCE (the only one): RTCW routes every message through
 * `botimport.Print( PRT_MESSAGE | PRT_ERROR, ... )`.  CoD has no botlib import
 * table and calls Com_Printf directly, prefixing the error paths with the "^1"
 * red colour code.  Identical substitution to ScriptError/SourceError.
 *
 * logfile_t is RTCW's.  `logfile_filename` is at 0x008BBC48 and `logfile_fp`
 * at 0x008BC048 -- exactly 1024 bytes apart, so retail's storage is one
 * `static logfile_t logfile` with RTCW's layout
 *   { char filename[MAX_LOGFILENAMESIZE]; FILE *fp; int numwrites; }
 * and MAX_LOGFILENAMESIZE unchanged at 1024.  `numwrites` is never touched
 * here because the only function that increments it is the one CoD dropped.
 *
 * WHOLLY DEAD IN THE RETAIL BINARY.  Log_Open has no xrefs, so nothing below it
 * runs: CoD MP ships no bots and never calls BotLibSetup.  Log_Write is the one
 * exception -- PC_PrintDefineHashTable calls it -- and that function has no
 * xrefs either.
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "l_libvar.h"

#define MAX_LOGFILENAMESIZE     1024

/*
 * Retail's storage is one record at 0x008BBC48; see the banner.  `logfile_fp`
 * is declared in cod1_globals.h, so only the filename half is declared here.
 * File-local because RTCW's `logfile` is `static`.
 */
static char logfile_filename[MAX_LOGFILENAMESIZE];

/*
===============================================================================
Log_AlwaysOpen                     0x0043D520    VERBATIM

RTCW l_log.c:64.  Retail inlines strlen as a byte scan at 0x0043D52B-0x43D537
and folds the null and empty-string tests into one `jz` to 0x0043D5A7.  The
already-open arm prints logfile_filename (0x008BBC48); the fopen-failed arm
prints the argument.  strncpy count is 0x400.

RETAIL: __usercall( filename@<esi> ).

VERIFIED 0x0043D520.
===============================================================================
*/
void Log_AlwaysOpen( char *filename ) {
	if ( !filename || !strlen( filename ) ) {
		Com_Printf( "openlog <filename>\n" );
		return;
	}
	if ( logfile_fp ) {
		Com_Printf( "^1Error: log file %s is already opened\n", logfile_filename );
		return;
	}
	logfile_fp = fopen( filename, "wb" );
	if ( !logfile_fp ) {
		Com_Printf( "^1Error: can't open the log file %s\n", filename );
		return;
	}
	strncpy( logfile_filename, filename, MAX_LOGFILENAMESIZE );
	Com_Printf( "Opened log %s\n", logfile_filename );
}

/*
===============================================================================
Log_Open                           0x0043D5C0    VERBATIM   (dead: no xrefs)

RTCW l_log.c:88.  Retail spells the guard as LibVar("log","0")->value rather
than LibVarValue("log","0") -- 0x0043D5CB calls LibVar and 0x0043D5D0 is
`fld dword ptr [eax+10h]`, libvar_t.value at +16, compared against 0.0.  Written
that way here so it compiles to the same instructions; RTCW's spelling would
emit a call to LibVarValue, which retail does not have.

RETAIL: __usercall( var_name@<eax>, value@<esi> ) for the inlined LibVar call;
this function's own filename arrives on the stack.

VERIFIED 0x0043D5C0.  The "log" string is at 0x00561C44 and the 0.0 constant
at 0x00568E64.

NaN: the compare is `fcomp 0.0 / fnstsw ax / test ah,44h / jnp`.  0x44 masks
C3 (equal) and C2 (unordered); equal gives 0x40, PF=0, `jnp` taken, call
skipped; unordered gives 0x44, PF=1, not taken, so it CALLS; greater or less
gives 0x00, PF=1, so it CALLS.  Retail opens the log for any value that is not
exactly 0.0, NaN included.  C's `!value` is false for NaN, so the plain
`if ( !...->value ) return;` takes the same arm in all three cases.
===============================================================================
*/
void Log_Open( char *filename ) {
	if ( !LibVar( "log", "0" )->value ) {
		return;
	}
	Log_AlwaysOpen( filename );
}

/*
===============================================================================
Log_Close                          0x0043D5F0    VERBATIM   (dead: no xrefs)

RTCW l_log.c:106.  The compiler hoisted the shared `push offset
logfile_filename` above the branch at 0x0043D604, which is the only structural
difference.

VERIFIED 0x0043D5F0.  The success arm clears logfile_fp between the push of
the format and the call (0x43D61E sits between 0x43D619 and 0x43D628); that is
scheduling, not order of effects -- Com_Printf's arguments are already pushed.
===============================================================================
*/
void Log_Close( void ) {
	if ( !logfile_fp ) {
		return;
	}
	if ( fclose( logfile_fp ) ) {
		Com_Printf( "^1Error: can't close log file %s\n", logfile_filename );
		return;
	}
	logfile_fp = NULL;
	Com_Printf( "Closed log %s\n", logfile_filename );
}

/*
===============================================================================
Log_Shutdown                       0x0043D640    VERBATIM   (dead: no xrefs)

RTCW l_log.c:124.  Retail tail-calls (`jmp Log_Close` at 0x0043D649).

VERIFIED 0x0043D640.
===============================================================================
*/
void Log_Shutdown( void ) {
	if ( logfile_fp ) {
		Log_Close();
	}
}

/*
===============================================================================
Log_Write                          0x0043D650    VERBATIM

RTCW l_log.c:130, including the `fprintf(logfile.fp, "\r\n")` that RTCW leaves
commented out -- retail has no such call either.

Its one caller, PC_PrintDefineHashTable, has no xrefs, so this is dead too.

VERIFIED 0x0043D650.  Retail re-loads logfile_fp from memory for the fflush at
0x43D669 rather than reusing EAX; `fflush( logfile_fp )` rather than a cached
pointer reproduces that.
===============================================================================
*/
void QDECL Log_Write( char *fmt, ... ) {
	va_list ap;

	if ( !logfile_fp ) {
		return;
	}
	va_start( ap, fmt );
	vfprintf( logfile_fp, fmt, ap );
	va_end( ap );
	fflush( logfile_fp );
}

/*
===============================================================================
Log_FilePointer                    0x0043D680    VERBATIM   (dead: no xrefs)
Log_Flush                          0x0043D690    VERBATIM   (dead: no xrefs)

RTCW l_log.c:159 / :168.

VERIFIED 0x0043D680 and 0x0043D690:

  0043D680  a1 48 c0 8b 00              mov  eax, logfile_fp
            c3                          retn
  0043D690  a1 48 c0 8b 00              mov  eax, logfile_fp
            85 c0                       test eax, eax
            74 07                       jz   short +7
            50                          push eax
            e8 8a 93 0e 00              call _fflush
            59                          pop  ecx
            c3                          retn

0x90 padding follows each; neither falls into a tail.
===============================================================================
*/
FILE *Log_FilePointer( void ) {
	return logfile_fp;
}

/* VERIFIED 0x0043D690.  See the byte listing above. */
void Log_Flush( void ) {
	if ( logfile_fp ) {
		fflush( logfile_fp );
	}
}
