/*
 * script/scr_opcode.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_opcode.cpp
 *
 * Retail range 0x0046D2E0-0x0046D374, 4 functions.
 *
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"

#include <ctype.h>

/* ---- Scr_IsInOpcodeMemory  0x0046D2E0 ---- */
qboolean Scr_IsInOpcodeMemory( const char *pos ) {
	return ( (unsigned int)pos >= (unsigned int)scrVarPub_programBuffer
		  && (unsigned int)pos <  (unsigned int)scrVarPub_programBuffer + (unsigned int)scrCompilePub_programLen )
		? qtrue : qfalse;
}

/* ---- Scr_IsInOpcodeBuffer_m  0x0046D300 ---- MEDIUM */
qboolean Scr_IsInOpcodeBuffer_m( const char *pos ) {
	return ( (unsigned int)pos >= (unsigned int)scrCompileGlob_devOpBuffer
		  && (unsigned int)pos <  (unsigned int)developerCodeStart )
		? qtrue : qfalse;
}

/* ---- Scr_IsInDeveloperOpcodeMemory  0x0046D320 ---- */
qboolean Scr_IsInDeveloperOpcodeMemory( const char *pos ) {
	return ( (unsigned int)pos >= (unsigned int)scrVarPub_programBuffer
		  && (unsigned int)pos <  (unsigned int)dword_1407384 )
		? qtrue : qfalse;
}

/* ---- Scr_IsIdentifier  0x0046D340 ---- */
qboolean Scr_IsIdentifier( const char *name ) {
	const signed char *p = (const signed char *)name;

	while ( *p ) {
		if ( !isalnum( *p ) && *p != '_' ) {
			return qfalse;
		}
		p++;
	}
	return qtrue;
}

qboolean Scr_IsIdentifier__FPCc( const char *name ) {
	return Scr_IsIdentifier( name );
}
