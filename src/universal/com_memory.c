/*
 * universal/com_memory.c
 *
 * Retail range 0x00431430-0x004327C0.
 *
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

extern void FS_ClearDataForFiles( void *lowEnd, void *highEnd );
extern int  XModelClearData();
extern int  timeGetTime( void );

extern void *fs_searchpaths;
extern void *Hunk_AllocAlignInternal( int size, int align );
extern void *Hunk_AllocLowAlignInternal( int size, int align );

#define HUNK_TOTAL          ( *(int *) hunk_totalSize )   /* 0x0089319C */
#define ZONE_TOTAL          ( *(int *) hunk_totalZoneSize )   /* 0x008931A0 */
#define HUNK_HIGH_MARK      ( *(int *) hunk_highMark )   /* 0x008931A4 */
#define HUNK_HIGH_TEMPMARK  ( hunk_highTempMark )            /* 0x008931A8, plain */
#define HUNK_HIGH_PERM      ( *(int *) hunk_highUsed )   /* 0x008931AC */
#define HUNK_HIGH_TEMP      ( *(int *) hunk_highTemp )   /* 0x008931B0 */
#define HUNK_LOW_MARK       ( *(int *) hunk_lowMark )   /* 0x008931BC */
#define HUNK_LOW_TEMPMARK   ( hunk_lowTempMark )            /* 0x008931C0, plain */
#define HUNK_LOW_PERM       ( *(int *) hunk_temp_permanent )  /* 0x008931C4 */
#define HUNK_LOW_TEMP       ( *(int *) hunk_temp_temp )       /* 0x008931C8 */

/* ---- Com_SRand_m  0x00431430 ---- VERIFIED */
int __cdecl Com_SRand_m( int seed ) {
	com_randSeed_m = seed;
	return seed;
}

/* ---- Com_RandFloatRange_m  0x00431440 ---- VERIFIED */
double __cdecl Com_RandFloatRange_m( float lo, float hi ) {
	float r;

	com_randSeed_m = 214013 * com_randSeed_m + 2531011;
	r = (float) ( (unsigned int) com_randSeed_m >> 17 );
	return (float) ( ( hi - lo ) * r / 32768.0 + lo );
}

/* ---- Com_RandIntRange_m  0x00431490 ---- VERIFIED */
int __fastcall Com_RandIntRange_m( int a1, int a2 ) {
	com_randSeed_m = 214013 * com_randSeed_m + 2531011;
	return a2 + ( (int) ( ( a1 - a2 ) * ( (unsigned int) com_randSeed_m >> 17 ) ) >> 15 );
}

/* ---- Z_FreeInternal  0x004318F0 ---- VERIFIED */
void __cdecl Z_FreeInternal( void *block ) {
	free( block );
}

/* CopyStringInternal  0x00431960  VERIFIED */
char *__cdecl CopyStringInternal( const char *in ) {
	char *out;

	out = (char *) Z_MallocInternal( strlen( in ) + 1 );
	strcpy( out, in );
	return out;
}

/* ---- Com_Meminfo_f  0x00431990 ---- VERIFIED */
void Com_Meminfo_f( void ) {
	Com_Printf( "%8i bytes total hunk\n", HUNK_TOTAL );
	Com_Printf( "%8i bytes total zone\n", ZONE_TOTAL );
	Com_Printf( "\n" );

	Com_Printf( "%8i low mark\n", HUNK_LOW_MARK );
	Com_Printf( "%8i low permanent\n", HUNK_LOW_PERM );
	if ( HUNK_LOW_TEMP != HUNK_LOW_PERM ) {
		Com_Printf( "%8i low temp\n", HUNK_LOW_TEMP );
	}
	Com_Printf( "\n" );

	Com_Printf( "%8i high mark\n", HUNK_HIGH_MARK );
	Com_Printf( "%8i high permanent\n", HUNK_HIGH_PERM );
	if ( HUNK_HIGH_TEMP != HUNK_HIGH_PERM ) {
		Com_Printf( "%8i high temp\n", HUNK_HIGH_TEMP );
	}
	Com_Printf( "\n" );

	Com_Printf( "%8i total hunk in use\n", HUNK_LOW_PERM + HUNK_HIGH_PERM );
	Com_Printf( "\n" );
}

/* ---- Com_TouchMemory  0x00431A80 ---- VERIFIED */
static int Com_TouchMilliseconds( void ) {
	if ( !sys_timeBaseInit ) {
		sys_timeBase = timeGetTime();
		sys_timeBaseInit = 1;
	}
	return timeGetTime() - sys_timeBase;
}

void Com_TouchMemory( void ) {
	int         *hunkWords;
	int         sum;
	int         start, stop;
	int         startTime;

	startTime = Com_TouchMilliseconds();
	sum = 0;
	hunkWords = (int *) s_hunkData;

	stop = HUNK_LOW_PERM >> 2;
	for ( start = 0 ; start < stop ; start += 64 ) {
		sum += hunkWords[start];
	}

	start = ( HUNK_TOTAL - HUNK_HIGH_PERM ) >> 2;
	stop = HUNK_HIGH_PERM >> 2;
	for ( ; start < stop ; start += 64 ) {
		sum += hunkWords[start];
	}

	Com_Printf( "Com_TouchMemory: %i msec. Using sum: %d\n",
				Com_TouchMilliseconds() - startTime, sum );
}

/* ---- Com_InitZoneMemory  0x00431B30 ---- VERIFIED */
void Com_InitZoneMemory( void ) {
}

typedef struct hunkblock_s {
	int                 size;
	int                 printed;
	struct hunkblock_s  *next;
	int                 unused_0C;
	const char          *sourceFile;
	int                 sourceLine;
} hunkblock_t;

/* ---- Hunk_Log  0x00431B40 ---- */
void Hunk_Log( void ) {
	hunkblock_t *block;
	char        buf[4096];
	int         size;
	int         numBlocks;

	if ( !hunk_logfile || !fs_searchpaths ) {
		return;
	}

	size = 0;
	numBlocks = 0;

	Com_sprintf( buf, sizeof( buf ),
				 "\r\n================\r\nHunk log\r\n================\r\n" );
	FS_Write( buf, strlen( buf ), hunk_logfile );

	for ( block = (hunkblock_t *) hunkblocks ; block ; block = block->next ) {
		size += block->size;
		numBlocks++;
	}

	Com_sprintf( buf, sizeof( buf ), "%d Hunk memory\r\n", size );
	FS_Write( buf, strlen( buf ), hunk_logfile );

	Com_sprintf( buf, sizeof( buf ), "%d hunk blocks\r\n", numBlocks );
	FS_Write( buf, strlen( buf ), hunk_logfile );
}

void Hunk_SmallLog( void ) {
	hunkblock_t *block;
	hunkblock_t *other;
	char        buf[4096];
	int         size;
	int         numBlocks;

	if ( !hunk_logfile || !fs_searchpaths ) {
		return;
	}

	for ( block = (hunkblock_t *) hunkblocks ; block ; block = block->next ) {
		block->printed = 0;
	}

	size = 0;
	numBlocks = 0;

	Com_sprintf( buf, sizeof( buf ),
				 "\r\n================\r\nHunk Small log\r\n================\r\n" );
	FS_Write( buf, strlen( buf ), hunk_logfile );

	for ( block = (hunkblock_t *) hunkblocks ; block ; block = block->next ) {
		if ( block->printed ) {
			continue;
		}
		for ( other = block->next ; other ; other = other->next ) {
			if ( block->sourceLine == other->sourceLine
				 && block->sourceFile && other->sourceFile
				 && !Q_stricmp( block->sourceFile, other->sourceFile ) ) {
				size += other->size;
				other->printed = 1;
			}
		}
		size += block->size;
		numBlocks++;
	}

	Com_sprintf( buf, sizeof( buf ), "%d Hunk memory\r\n", size );
	FS_Write( buf, strlen( buf ), hunk_logfile );

	Com_sprintf( buf, sizeof( buf ), "%d hunk blocks\r\n", numBlocks );
	FS_Write( buf, strlen( buf ), hunk_logfile );
}

/* ---- Hunk_MemoryRemaining  0x00431F10 ---- VERIFIED */
int Hunk_MemoryRemaining( void ) {
	return HUNK_TOTAL - HUNK_HIGH_TEMP - HUNK_LOW_TEMP;
}

/* ---- Hunk_RetouchMemory  0x00431F30 ---- VERIFIED */
void Hunk_RetouchMemory( void ) {
	XModelClearData( (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM,
					 (char *) s_hunkData + HUNK_LOW_PERM );
	FS_ClearDataForFiles( (char *) s_hunkData + HUNK_LOW_PERM,
						  (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM );
}

/* ---- Hunk_SetHighMark  0x00431F70 ---- VERIFIED */
int Hunk_SetHighMark( void ) {
	HUNK_HIGH_MARK = HUNK_HIGH_PERM;
	return HUNK_HIGH_PERM;
}

/* ---- Hunk_SetHighTempMark  0x00431F80 ---- VERIFIED */
int Hunk_SetHighTempMark( void ) {
	HUNK_HIGH_TEMPMARK = HUNK_HIGH_PERM;
	return HUNK_HIGH_PERM;
}

/* ---- Hunk_CheckHighMark  0x00431F90 ---- VERIFIED */
qboolean __cdecl Hunk_CheckHighMark( const void *ptr ) {
	return (qboolean) ( (const char *) ptr
						>= (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_MARK );
}

/* ---- Hunk_ClearToHighMark  0x00431FB0 ---- VERIFIED */
void Hunk_ClearToHighMark( void ) {
	HUNK_HIGH_TEMP = HUNK_HIGH_MARK;
	HUNK_HIGH_PERM = HUNK_HIGH_MARK;
	Hunk_RetouchMemory();
}

/* ---- Hunk_ClearToHighTempMark  0x00431FF0 ---- VERIFIED */
void Hunk_ClearToHighTempMark( void ) {
	HUNK_HIGH_TEMP = HUNK_HIGH_TEMPMARK;
	HUNK_HIGH_PERM = HUNK_HIGH_TEMPMARK;
	Hunk_RetouchMemory();
}

/* ---- Hunk_HighMarkIsSet  0x00432030 ---- VERIFIED */
qboolean Hunk_HighMarkIsSet( void ) {
	return (qboolean) ( HUNK_HIGH_MARK != 0 );
}

/* ---- Hunk_SetLowMark  0x00432040 ---- VERIFIED */
int Hunk_SetLowMark( void ) {
	HUNK_LOW_MARK = HUNK_LOW_PERM;
	return HUNK_LOW_PERM;
}

/* ---- Hunk_ClearToLowMark  0x00432050 ---- VERIFIED */
void Hunk_ClearToLowMark( void ) {
	HUNK_LOW_TEMP = HUNK_LOW_MARK;
	HUNK_LOW_PERM = HUNK_LOW_MARK;
	XModelClearData( (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM,
					 (char *) s_hunkData + HUNK_LOW_MARK );
	FS_ClearDataForFiles( (char *) s_hunkData + HUNK_LOW_MARK,
						  (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM );
}

/* ---- Hunk_Clear  0x00432140 ---- VERIFIED */
void Hunk_Clear( void ) {
	CL_ShutdownCGame();
	CL_ShutdownUI();
	SV_ShutdownGameProgs();
	CIN_CloseAllVideos();
	Hunk_ClearToStart();
	VM_Clear();
}

/* ---- Hunk_AllocInternal  0x00432160 ---- VERIFIED */
void *__cdecl Hunk_AllocInternal( int size ) {
	return Hunk_AllocAlignInternal( size, 32 );
}

/* ---- Hunk_ClearTempMemoryHigh  0x00432260 ---- VERIFIED */
int Hunk_ClearTempMemoryHigh( void ) {
	HUNK_HIGH_TEMP = HUNK_HIGH_PERM;
	return HUNK_HIGH_PERM;
}

/* ---- Hunk_AllocLowInternal  0x00432270 ---- VERIFIED */
void *__cdecl Hunk_AllocLowInternal( int size ) {
	return Hunk_AllocLowAlignInternal( size, 32 );
}

/* ---- Hunk_CommitTempMemory  0x00432300 ---- VERIFIED */
int Hunk_CommitTempMemory( void ) {
	HUNK_LOW_PERM = HUNK_LOW_TEMP;
	return HUNK_LOW_TEMP;
}

/* ---- Hunk_ClearToLowTempMark  0x004323F0 ---- VERIFIED */
int Hunk_ClearToLowTempMark( void ) {
	HUNK_LOW_TEMP = HUNK_LOW_TEMPMARK;
	return HUNK_LOW_TEMPMARK;
}

/* ---- Hunk_SetLowTempMark  0x00432400 ---- VERIFIED */
int Hunk_SetLowTempMark( void ) {
	HUNK_LOW_TEMPMARK = HUNK_LOW_TEMP;
	return HUNK_LOW_TEMP;
}

/* ---- Hunk_FreeTempMemory  0x00432410 ---- VERIFIED */
void Hunk_FreeTempMemory( void *buf ) {
	int delta;

	if ( !s_hunkData ) {
		free( buf );
		return;
	}

	if ( *( (int *) buf - 4 ) != (int) 0x89537892 ) {
		Com_Error( ERR_FATAL, "\x15Hunk_FreeTempMemory: bad magic" );
	}
	delta = *( (int *) buf - 3 );
	*( (int *) buf - 4 ) = (int) 0x89537893;
	HUNK_LOW_TEMP -= delta;
}

/* ---- Hunk_ClearTempMemory  0x00432450 ---- VERIFIED */
void Hunk_ClearTempMemory( void ) {
	if ( s_hunkData ) {
		HUNK_LOW_TEMP = HUNK_LOW_PERM;
	}
}

/* ---- Hunk_ConvertTempToPermLowInternal  0x00432470 ---- VERIFIED */
int Hunk_ConvertTempToPermLowInternal( void ) {
	int old;

	old = HUNK_LOW_PERM;
	HUNK_LOW_PERM = HUNK_LOW_TEMP;
	return old;
}

/* ---- Hunk_SetLowUsedInternal  0x00432490 ---- VERIFIED */
int __cdecl Hunk_SetLowUsedInternal( int lowUsed ) {
	HUNK_LOW_PERM = lowUsed;
	return lowUsed;
}

/* ---- nullsub_12  0x004324A0 ---- VERIFIED */
void nullsub_12( void ) {
}

/* ---- memset32  0x004324B0 ---- VERIFIED */
void memset32( unsigned int *dest, unsigned int constant, unsigned int count ) {
	unsigned int i;

	for ( i = 0 ; i < count ; i++ ) {
		dest[i] = constant;
	}
}

/* ---- Com_Memcmp_m  0x00432720 ---- VERIFIED */
int __cdecl Com_Memcmp_m( const void *a, const void *b, unsigned int n ) {
	return memcmp( a, b, n ) == 0;
}

/* ---- Com_TouchMemoryBlock_m  0x004327C0 ---- VERIFIED */
int __cdecl Com_TouchMemoryBlock_m( int selector, const char *block, int size ) {
	unsigned int i;
	int          n;
	volatile int sink;

	if ( !selector || ( selector -= 2 ) == 0 ) {
		n = size;
		if ( n > 4096 ) {
			n = 4096;
		}
		for ( i = (unsigned int) ( n + 31 ) >> 5 ; i ; --i ) {
			sink = *block;
			block += 32;
		}
		(void) sink;
	}
	return selector;
}
