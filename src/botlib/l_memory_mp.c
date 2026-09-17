/*
 *
 * Retail range 0x0043D6B0-0x0043D7A0, 7 functions.
 *
 * PrintUsedMemorySize and PrintMemoryLabels are at 0x0043D790 and 0x0043D7A0,
 * immediately after FreeMemory, and are empty: RTCW's release (non-MEMDEBUG)
 * half of l_memory.c defines both with empty bodies.  MemoryByteSize and
 * DumpMemory are absent -- those two are inside #ifdef MEMDEBUG.  The
 * reference's release build defines four functions after GetHunkMemory, in the
 * order GetClearedHunkMemory, FreeMemory, PrintUsedMemorySize,
 * PrintMemoryLabels, and CoD1 has four at 0x0043D730/770/790/7A0 in that order.
 *
 * The label argument is absent from the allocators: it is the MEMDEBUG half
 * that carries it.
 *
 * The two header magics are unmodified from the reference:
 *
 *     MEM_ID   0x12345678      Z_Malloc-backed blocks
 *     HUNK_ID  0x87654321      Hunk_AllocAlign-backed blocks
 *
 * Each allocation reserves a leading dword for the magic and returns the
 * pointer past it, so FreeMemory can assert the block came from GetMemory and
 * not from the hunk (freeing a hunk block would corrupt the hunk).
 *
 * NOTE: retail routes GetMemory through Z_MallocInternal and GetHunkMemory
 * through Hunk_AllocAlignInternal with a 32-byte alignment.  RTCW's botlib uses
 * plain malloc for the former; the alignment argument is a CoD addition.
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"
#include "l_memory.h"

#define MEM_ID      0x12345678l
#define HUNK_ID     0x87654321l

/* universal/com_memory_core.c 0x00432170.  Retail takes both arguments in
 * registers, align@<eax> and size@<ecx>, so the registers look transposed
 * relative to the source order ( size, align ) used here and at every hunk
 * allocator call site.  Do not "fix" this. */
extern void *Hunk_AllocAlignInternal( int size, int align );

/*
=================
GetMemory                  0x0043D6B0
=================
*/
void *GetMemory( unsigned long size ) {
	void *ptr;
	unsigned long *memid;

	ptr = Z_MallocInternal( size + sizeof( unsigned long ) );
	if ( !ptr ) {
		return NULL;
	}
	memid = (unsigned long *) ptr;
	*memid = MEM_ID;

	return (void *) ( (char *) ptr + sizeof( unsigned long ) );
}

/*
=================
GetClearedMemory           0x0043D6D0
=================
*/
void *GetClearedMemory( unsigned long size ) {
	void *ptr;

	ptr = GetMemory( size );
	memset( ptr, 0, size );
	return ptr;
}

/*
=================
GetHunkMemory              0x0043D710

DIVERGENCE from RTCW: the alignment argument (32) is CoD's.
=================
*/
void *GetHunkMemory( unsigned long size ) {
	void *ptr;
	unsigned long *memid;

	ptr = (void *) Hunk_AllocAlignInternal( size + sizeof( unsigned long ), 32 );
	if ( !ptr ) {
		return NULL;
	}
	memid = (unsigned long *) ptr;
	*memid = HUNK_ID;

	return (void *) ( (char *) ptr + sizeof( unsigned long ) );
}

/*
=================
GetClearedHunkMemory       0x0043D730
=================
*/
void *GetClearedHunkMemory( unsigned long size ) {
	void *ptr;

	ptr = GetHunkMemory( size );
	memset( ptr, 0, size );
	return ptr;
}

/*
=================
FreeMemory                 0x0043D770

Silently ignores hunk blocks: the magic test fails and nothing is freed.  That
is retail behaviour, not an omission -- hunk memory is reclaimed wholesale by
Hunk_Clear, never individually.
=================
*/
void FreeMemory( void *ptr ) {
	unsigned long *memid;

	memid = (unsigned long *) ( (char *) ptr - sizeof( unsigned long ) );

	if ( *memid == MEM_ID ) {
		free( memid );
	}
}

/*
=================
PrintUsedMemorySize        0x0043D790     HIGH
PrintMemoryLabels          0x0043D7A0     HIGH

RTCW l_memory.c:435 and :443 -- the non-MEMDEBUG definitions, both of which are
EMPTY in the reference.  Retail is one `retn` apiece and neither has an xref.

Identified by position: RTCW's release half of this unit ends with exactly
four functions -- GetClearedHunkMemory, FreeMemory, PrintUsedMemorySize,
PrintMemoryLabels -- and CoD1's tail is exactly four, at 0x0043D730 /
0x0043D770 / 0x0043D790 / 0x0043D7A0, with the first two pinned by HUNK_ID
and MEM_ID.

HIGH rather than higher: the pair is certain, but which address is which rests
on the compiler emitting in source order; both bodies are empty, so nothing
separates them.
=================
*/
void PrintUsedMemorySize( void ) {
}

/* Band: HIGH -- see the note on 0x0043D790 above. */
void PrintMemoryLabels( void ) {
}
