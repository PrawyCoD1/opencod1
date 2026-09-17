/*
 * botlib memory allocation -- reconstructed from CoD 1.1 (CoDMP.exe).
 *
 * Retail bodies at 0x0043D6B0-0x0043D78F; see l_memory_mp.c.
 *
 * Only the five entry points the retail linker kept are declared.  RTCW's
 * MemoryByteSize / PrintUsedMemorySize / PrintMemoryLabels / DumpMemory are
 * absent from the binary and are deliberately not declared here.
 */

#ifndef __L_MEMORY_H__
#define __L_MEMORY_H__

void *GetMemory( unsigned long size );
void *GetClearedMemory( unsigned long size );
void *GetHunkMemory( unsigned long size );
void *GetClearedHunkMemory( unsigned long size );
void FreeMemory( void *ptr );

#endif /* __L_MEMORY_H__ */
