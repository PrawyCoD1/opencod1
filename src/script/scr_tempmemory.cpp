/*
 * script/scr_tempmemory.cpp
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_tempmemory.cpp
 *
 * Retail range 0x00470470-0x004704CB, 3 functions.
 *
 * @fidelity: verified
 */

#include "scr_local.h"

extern int currentPos;
#define scrTempMemory_currentPos currentPos

extern void *Hunk_ReallocateTempMemory( int size );

/* ---- TempMemoryReset  0x00470470 ---- VERIFIED */
void TempMemoryReset( void ) {
	scrTempMemory_currentPos = 0;
}

/* ---- TempMalloc  0x00470480 ---- VERIFIED */
void *TempMalloc( int len ) {
	int oldPos;

	oldPos = scrTempMemory_currentPos;
	scrTempMemory_currentPos += len;
	return (char *)Hunk_ReallocateTempMemory( scrTempMemory_currentPos ) + oldPos;
}

/* ---- TempMemorySetPos  0x004704A0 ---- VERIFIED */
void TempMemorySetPos( void *pos ) {
	char *end;

	end = (char *)TempMalloc( 0 );
	scrTempMemory_currentPos -= (int)( end - (char *)pos );
	Hunk_ReallocateTempMemory( scrTempMemory_currentPos );
}
