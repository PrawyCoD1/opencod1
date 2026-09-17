/*
 * universal/com_memory_core.c
 *
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

#include <time.h>

#define HUNK_LOW_MARK   ( *(int *) hunk_lowMark )
#define HUNK_LOW_PERM   ( *(int *) hunk_temp_permanent )
#define HUNK_LOW_TEMP   ( *(int *) hunk_temp_temp )
#define HUNK_HIGH_MARK  ( *(int *) hunk_highMark )
#define HUNK_HIGH_PERM  ( *(int *) hunk_highUsed )
#define HUNK_HIGH_TEMP  ( *(int *) hunk_highTemp )
#define HUNK_TOTAL      ( *(int *) hunk_totalSize )
#define ZONE_TOTAL      ( *(int *) hunk_totalZoneSize )

extern void Com_Meminfo_f( void );
extern void Sys_OutOfMemoryPrep( void );

static int s_hunkTotal;                 /* 0x0089319C */
static int s_zoneTotal;                 /* 0x008931A0 */
static int s_hunkHighMark;              /* 0x008931A4 */
static int s_hunkHighPermanent;         /* 0x008931AC */
static int s_hunkHighTemp;              /* 0x008931B0 */
static int s_hunkLowMark;               /* 0x008931BC */
static int s_hunkLowPermanent;          /* 0x008931C4 */
static int s_hunkLowTemp;               /* 0x008931C8 */

void Com_BindHunkGlobals( void ) {
	if ( hunk_totalSize ) {
		return;
	}
	hunk_totalSize        = (int) &s_hunkTotal;
	hunk_totalZoneSize        = (int) &s_zoneTotal;
	hunk_highMark        = (int) &s_hunkHighMark;
	hunk_highUsed        = (int) &s_hunkHighPermanent;
	hunk_highTemp        = (int) &s_hunkHighTemp;
	hunk_lowMark        = (int) &s_hunkLowMark;
	hunk_temp_permanent = (int) &s_hunkLowPermanent;
	hunk_temp_temp      = (int) &s_hunkLowTemp;
}

/* ---- Com_InitHunkMemory  0x00431E00 ---- */
void Com_InitHunkMemory( void ) {
	cvar_t      *cv;
	int minMegs;
	const char  *fmt;
	void        *buf;

	Com_BindHunkGlobals();

	if ( FS_LoadStack() ) {
		Com_Error( ERR_FATAL,
				   "\x15Hunk initialization failed. File system load stack not zero" );
	}

	cv = Cvar_Get( "com_hunkMegs", "128", CVAR_ARCHIVE | CVAR_LATCH );

	if ( com_dedicated && com_dedicated->integer ) {
		minMegs = 1;
		fmt = "Minimum com_hunkMegs for a dedicated server is %i, allocating %i megs.\n";
	} else {
		minMegs = 80;
		fmt = "Minimum com_hunkMegs is %i, allocating %i megs.\n";
	}

	if ( cv->integer >= minMegs ) {
		HUNK_TOTAL = cv->integer << 20;
	} else {
		HUNK_TOTAL = minMegs << 20;
		Com_Printf( fmt, minMegs, HUNK_TOTAL / ( 1024 * 1024 ) );
	}

	buf = malloc( HUNK_TOTAL + 31 );
	if ( !buf ) {
		Sys_OutOfMemoryPrep();
		Sys_Error( "Com_InitHunkMemory: failed to allocate %i bytes", HUNK_TOTAL + 31 );
		return;
	}
	s_hunkData = (void *) ( ( (unsigned int) buf + 31 ) & 0xFFFFFFE0 );

	CL_ShutdownCGame();
	CL_ShutdownUI();
	SV_ShutdownGameProgs();
	CIN_CloseAllVideos();
	Hunk_ClearToStart();
	VM_Clear();

	Cmd_AddCommand( "meminfo", Com_Meminfo_f );
}

/* ---- Z_MallocInternal  0x00431900 ---- */
void *Z_MallocInternal( int size ) {
	void *p;

	p = malloc( size );
	if ( !p ) {
		Sys_Error( "Z_MallocInternal: failed to allocate %i bytes", size );
		return NULL;
	}
	Com_Memset( p, 0, size );
	return p;
}

/* ---- Hunk_AllocAlignInternal  0x00432170 ---- */
void *Hunk_AllocAlignInternal( int size, int align ) {
	int mark;
	void *block;

	if ( !s_hunkData ) {
		Com_Error( ERR_FATAL, "\x15" "Hunk_AllocAlign: Hunk memory system not initialized" );
	}

	mark = ( align - 1 + size + HUNK_HIGH_PERM ) & ~( align - 1 );
	block = (void *) ( (char *) s_hunkData + HUNK_TOTAL - mark );

	HUNK_HIGH_PERM = mark;
	HUNK_HIGH_TEMP = mark;

	if ( mark + HUNK_LOW_TEMP > HUNK_TOTAL ) {
		Com_Meminfo_f();
		Com_Error( ERR_DROP, "\x15" "Hunk_AllocAlign failed on %i", size );
	}

	com_hunkMeminfoRunningTotal = mark + HUNK_LOW_PERM;

	Com_Memset( block, 0, size );
	return block;
}

/* ---- Hunk_AllocLowAlignInternal  0x00432280 ---- */
void *Hunk_AllocLowAlignInternal( int size, int align ) {
	int base, mark;
	void *block;

	base = ( align - 1 + HUNK_LOW_PERM ) & ~( align - 1 );
	block = (void *) ( (char *) s_hunkData + base );
	mark = size + base;

	HUNK_LOW_PERM = mark;
	HUNK_LOW_TEMP = mark;

	if ( mark + HUNK_HIGH_TEMP > HUNK_TOTAL ) {
		Com_Meminfo_f();
		Com_Error( ERR_DROP, "\x15" "Hunk_AllocLowAlign failed on %i", size );
	}

	com_hunkMeminfoRunningTotal = mark + HUNK_HIGH_PERM;

	Com_Memset( block, 0, size );
	return block;
}

/* ---- Hunk_AllocateTempMemoryHighInternal  0x00432210 ---- */
void *Hunk_AllocateTempMemoryHighInternal( int size ) {
	HUNK_HIGH_TEMP = ( HUNK_HIGH_TEMP + size + 15 ) & ~15;

	if ( HUNK_HIGH_TEMP + HUNK_LOW_TEMP > HUNK_TOTAL ) {
		Com_Meminfo_f();
		Com_Error( ERR_DROP, "\x15" "Hunk_AllocateTempMemoryHigh: failed on %i", size );
	}

	return (void *) ( (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_TEMP );
}

typedef enum
{
	PRE_READ,                                   // prefetch assuming that buffer is used for reading only
	PRE_WRITE,                                  // prefetch assuming that buffer is used for writing only
	PRE_READ_WRITE                              // prefetch assuming that buffer is used for both reading and writing
} e_prefetch;

void Com_Prefetch( const void *s, const unsigned int bytes, e_prefetch type );

/* RTCW _copyDWord; 0x004324B0, com_memory.c */
void memset32( unsigned int *dest, unsigned int constant, unsigned int count );

/* ---- Com_Memcpy  0x00432540 ---- VERIFIED */
// optimized memory copy routine that handles all alignment
// cases and block sizes efficiently
void Com_Memcpy( void* dest, const void* src, size_t count ) {
	Com_Prefetch( src, count, PRE_READ );       /* inlined in retail */
	__asm
	{
		push edi
		push esi
		mov ecx,count
		cmp ecx,0                           // count = 0 check (just to be on the safe side)
		je outta
		mov edx,dest
		mov ebx,src
		cmp ecx,32                          // padding only?
		jl padding

		mov edi,ecx
		and     edi,~31                 // edi = count&~31
		sub edi,32

		align 16
loopMisAligned:
		mov eax,[ebx + edi + 0 + 0 * 8]
		mov esi,[ebx + edi + 4 + 0 * 8]
		mov     [edx + edi + 0 + 0 * 8],eax
		mov     [edx + edi + 4 + 0 * 8],esi
		mov eax,[ebx + edi + 0 + 1 * 8]
		mov esi,[ebx + edi + 4 + 1 * 8]
		mov     [edx + edi + 0 + 1 * 8],eax
		mov     [edx + edi + 4 + 1 * 8],esi
		mov eax,[ebx + edi + 0 + 2 * 8]
		mov esi,[ebx + edi + 4 + 2 * 8]
		mov     [edx + edi + 0 + 2 * 8],eax
		mov     [edx + edi + 4 + 2 * 8],esi
		mov eax,[ebx + edi + 0 + 3 * 8]
		mov esi,[ebx + edi + 4 + 3 * 8]
		mov     [edx + edi + 0 + 3 * 8],eax
		mov     [edx + edi + 4 + 3 * 8],esi
		sub edi,32
		jge loopMisAligned

		mov edi,ecx
		and     edi,~31
		add ebx,edi                     // increase src pointer
		add edx,edi                     // increase dst pointer
		and     ecx,31                  // new count
		jz outta                        // if count = 0, get outta here

padding:
		cmp ecx,16
		jl skip16
		mov eax,dword ptr [ebx]
		mov dword ptr [edx],eax
		mov eax,dword ptr [ebx + 4]
		mov dword ptr [edx + 4],eax
		mov eax,dword ptr [ebx + 8]
		mov dword ptr [edx + 8],eax
		mov eax,dword ptr [ebx + 12]
		mov dword ptr [edx + 12],eax
		sub ecx,16
		add ebx,16
		add edx,16
skip16:
		cmp ecx,8
		jl skip8
		mov eax,dword ptr [ebx]
		mov dword ptr [edx],eax
		mov eax,dword ptr [ebx + 4]
		sub ecx,8
		mov dword ptr [edx + 4],eax
		add ebx,8
		add edx,8
skip8:
		cmp ecx,4
		jl skip4
		mov eax,dword ptr [ebx]     // here 4-7 bytes
		add ebx,4
		sub ecx,4
		mov dword ptr [edx],eax
		add edx,4
skip4:                          // 0-3 remaining bytes
		cmp ecx,2
		jl skip2
		mov ax,word ptr [ebx]       // two bytes
		cmp ecx,3                   // less than 3?
		mov word ptr [edx],ax
		jl outta
		mov al,byte ptr [ebx + 2]   // last byte
		mov byte ptr [edx + 2],al
		jmp outta
skip2:
		cmp ecx,1
		jl outta
		mov al,byte ptr [ebx]
		mov byte ptr [edx],al
outta:
		pop esi
		pop edi
	}
}

/* ---- Com_Memset  0x00432670 ---- VERIFIED */
void Com_Memset( void* dest, int val, size_t count ) {
	unsigned int fillval;

	if ( count < 8 ) {
		__asm
		{
			mov edx,dest
			mov eax, val
			mov ah,al
			mov ebx,eax
			and     ebx, 0xffff
			shl eax,16
			add eax,ebx                 // eax now contains pattern
			mov ecx,count
			cmp ecx,4
			jl skip4
			mov     [edx],eax           // copy first dword
			add edx,4
			sub ecx,4
skip4:  cmp ecx,2
			jl skip2
			mov word ptr [edx],ax       // copy 2 bytes
			add edx,2
			sub ecx,2
skip2:  cmp ecx,0
			je skip1
			mov byte ptr [edx],al       // copy single byte
skip1:
		}
		return;
	}

	fillval = val;

	fillval = fillval | ( fillval << 8 );
	fillval = fillval | ( fillval << 16 );        // fill dword with 8-bit pattern

	memset32( (unsigned int*)( dest ),fillval, count / 4 );

	__asm                                   // padding of 0-3 bytes
	{
		mov ecx,count
		mov eax,ecx
		and     ecx,3
		jz skipA
		and     eax,~3
		mov ebx,dest
		add ebx,eax
		mov eax,fillval
		cmp ecx,2
		jl skipB
		mov word ptr [ebx],ax
		cmp ecx,2
		je skipA
		mov byte ptr [ebx + 2],al
		jmp skipA
skipB:
		cmp ecx,0
		je skipA
		mov byte ptr [ebx],al
skipA:
	}
}

/* ---- Com_Prefetch  0x004327C0 ---- VERIFIED */
/* Alias Com_TouchMemoryBlock_m; type arrives in eax (LTCG).
   The PRE_READ instance is inlined into Com_Memcpy in retail. */
void Com_Prefetch( const void *s, const unsigned int bytes, e_prefetch type ) {
	// write buffer prefetching is performed only if
	// the processor benefits from it. Read and read/write
	// prefetching is always performed.

	switch ( type )
	{
	case PRE_WRITE: break;
	case PRE_READ:
	case PRE_READ_WRITE:

		__asm
		{
			mov ebx,s
			mov ecx,bytes
			cmp ecx,4096                    // clamp to 4kB
			jle skipClamp
			mov ecx,4096
skipClamp:
			add ecx,0x1f
			shr ecx,5                       // number of cache lines
			jz skip
			jmp loopie

			align 16
loopie: test byte ptr [ebx],al
			add ebx,32
			dec ecx
			jnz loopie
skip:
		}

		break;
	}
}

/* ---- Hunk_AllocateTempMemoryInternal  0x00432310 ---- VERIFIED */
void *Hunk_AllocateTempMemoryInternal( int size ) {
	int *header;
	int headerOffset;
	int previousLowTemp;

	if ( !s_hunkData ) {
		return Z_MallocInternal( size );
	}

	size += 16;

	previousLowTemp = HUNK_LOW_TEMP;
	HUNK_LOW_TEMP = ( HUNK_LOW_TEMP + 15 ) & ~15;
	headerOffset = HUNK_LOW_TEMP;
	HUNK_LOW_TEMP += size;

	if ( HUNK_HIGH_TEMP + HUNK_LOW_TEMP > HUNK_TOTAL ) {
		Com_Meminfo_f();
		Com_Error( ERR_DROP, "\x15" "Hunk_AllocateTempMemory: failed on %i, needs %i",
				   size, HUNK_HIGH_TEMP + HUNK_LOW_TEMP - HUNK_TOTAL );
	}

	header = (int *) ( (char *) s_hunkData + headerOffset );
	header[0] = 0x89537892;
	header[1] = HUNK_LOW_TEMP - previousLowTemp;

	return (char *) header + 16;
}

extern void Hunk_FreeTempMemory( void *buf );
extern void Hunk_ClearTempMemory( void );
extern int Hunk_ClearTempMemoryHigh( void );

/* Hunk_FreeTempMemoryInternal  0x00432410  VERIFIED */
void Hunk_FreeTempMemoryInternal( void *buf ) {
	Hunk_FreeTempMemory( buf );
}

void Hunk_ResetTempMark( void ) {
	Hunk_ClearTempMemory();
	Hunk_ClearTempMemoryHigh();
}

/* ---- Hunk_ReallocateTempMemory  0x004323A0 ---- */
void *Hunk_ReallocateTempMemory( int size ) {
	HUNK_LOW_TEMP = HUNK_LOW_PERM + size;

	if ( HUNK_LOW_PERM + size + HUNK_HIGH_TEMP > HUNK_TOTAL ) {
		Com_Meminfo_f();
		Com_Error( ERR_DROP, "\x15Hunk_ReallocateTempMemory: failed on %i", size );
	}

	return (void *) ( (char *) s_hunkData + HUNK_LOW_PERM );
}

/* ---- Hunk_ClearToStart  0x00432090 ---- */
void Hunk_ClearToStart( void ) {
	extern void XModelClearData( void *high, void *low );
	extern void FS_ClearDataForFiles( void *lowEnd, void *highEnd );
	extern void FS_ClearMemory( void );

	if ( !s_hunkData ) {
		return;
	}

	HUNK_LOW_MARK = 0;
	hunk_lowTempMark = 0;
	HUNK_LOW_PERM = 0;
	HUNK_LOW_TEMP = 0;
	HUNK_HIGH_MARK = 0;
	hunk_highTempMark = 0;
	HUNK_HIGH_PERM = 0;
	HUNK_HIGH_TEMP = 0;
	com_hunkMeminfoRunningTotal = 0;

	Com_Printf( "Hunk_Clear: reset the hunk ok\n" );

	XModelClearData( (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM,
					 (char *) s_hunkData + HUNK_LOW_PERM );
	FS_ClearDataForFiles( (char *) s_hunkData + HUNK_LOW_PERM,
						  (char *) s_hunkData + HUNK_TOTAL - HUNK_HIGH_PERM );

	FS_ClearMemory();
}

/* ---- Com_StringContains  0x004314C0 ---- */
const char *Com_StringContains( const char *str1, const char *str2, int casesensitive ) {
	int len, i, j;

	len = strlen( str1 ) - strlen( str2 );
	for ( i = 0; i <= len; i++, str1++ ) {
		for ( j = 0; str2[j]; j++ ) {
			if ( casesensitive ) {
				if ( str1[j] != str2[j] ) {
					break;
				}
			} else {
				if ( toupper( str1[j] ) != toupper( str2[j] ) ) {
					break;
				}
			}
		}
		if ( !str2[j] ) {
			return str1;
		}
	}
	return NULL;
}

/* ---- Com_Filter  0x00431580 ---- */
qboolean Com_Filter( const char *filter, const char *name, int casesensitive ) {
	char buf[MAX_TOKEN_CHARS];
	const char  *ptr;
	int i, found;

	while ( *filter ) {
		if ( *filter == '*' ) {
			filter++;
			for ( i = 0; *filter; i++ ) {
				if ( *filter == '*' || *filter == '?' ) {
					break;
				}
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if ( strlen( buf ) ) {
				ptr = Com_StringContains( name, buf, casesensitive );
				if ( !ptr ) {
					return qfalse;
				}
				name = ptr + strlen( buf );
			}
		} else if ( *filter == '?' ) {
			filter++;
			name++;
		} else if ( *filter == '[' && *( filter + 1 ) == '[' ) {
			filter++;
		} else if ( *filter == '[' ) {
			filter++;
			found = qfalse;
			while ( *filter && !found ) {
				if ( *filter == ']' && *( filter + 1 ) != ']' ) {
					break;
				}
				if ( *( filter + 1 ) == '-' && *( filter + 2 )
					 && ( *( filter + 2 ) != ']' || *( filter + 3 ) == ']' ) ) {
					if ( casesensitive ) {
						if ( *name >= *filter && *name <= *( filter + 2 ) ) {
							found = qtrue;
						}
					} else {
						if ( toupper( *name ) >= toupper( *filter )
							 && toupper( *name ) <= toupper( *( filter + 2 ) ) ) {
							found = qtrue;
						}
					}
					filter += 3;
				} else {
					if ( casesensitive ) {
						if ( *filter == *name ) {
							found = qtrue;
						}
					} else {
						if ( toupper( *filter ) == toupper( *name ) ) {
							found = qtrue;
						}
					}
					filter++;
				}
			}
			if ( !found ) {
				return qfalse;
			}
			while ( *filter ) {
				if ( *filter == ']' && *( filter + 1 ) != ']' ) {
					break;
				}
				filter++;
			}
			filter++;
			name++;
		} else {
			if ( casesensitive ) {
				if ( *filter != *name ) {
					return qfalse;
				}
			} else {
				if ( toupper( *filter ) != toupper( *name ) ) {
					return qfalse;
				}
			}
			filter++;
			name++;
		}
	}
	return qtrue;
}

/* ---- Com_FilterPath  0x00431790 ---- */
qboolean Com_FilterPath( const char *filter, const char *name, int casesensitive ) {
	int i;
	char new_filter[MAX_QPATH];
	char new_name[MAX_QPATH];

	for ( i = 0; i < MAX_QPATH - 1 && filter[i]; i++ ) {
		if ( filter[i] == '\\' || filter[i] == ':' ) {
			new_filter[i] = '/';
		} else {
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';

	for ( i = 0; i < MAX_QPATH - 1 && name[i]; i++ ) {
		if ( name[i] == '\\' || name[i] == ':' ) {
			new_name[i] = '/';
		} else {
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';

	return Com_Filter( new_filter, new_name, casesensitive );
}

/* ---- Com_HashKey  0x00431850 ---- */
int Com_HashKey( const char *string, int maxlen ) {
	int hash, i;

	hash = 0;
	for ( i = 0; i < maxlen && string[i] != '\0'; i++ ) {
		hash += (signed char) string[i] * ( 119 + i );
	}
	hash = ( hash ^ ( hash >> 10 ) ^ ( hash >> 20 ) );
	return hash;
}

#ifndef QTIME_T_DEFINED
#define QTIME_T_DEFINED
typedef struct qtime_s {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
} qtime_t;
#endif

/* ---- Com_RealTime  0x00431890 ---- */
int Com_RealTime( qtime_t *qtime ) {
	time_t t;
	struct tm   *tms;

	t = time( NULL );
	if ( !qtime ) {
		return (int) t;
	}

	tms = localtime( &t );
	if ( tms ) {
		qtime->tm_sec = tms->tm_sec;
		qtime->tm_min = tms->tm_min;
		qtime->tm_hour = tms->tm_hour;
		qtime->tm_mday = tms->tm_mday;
		qtime->tm_mon = tms->tm_mon;
		qtime->tm_year = tms->tm_year;
		qtime->tm_wday = tms->tm_wday;
		qtime->tm_yday = tms->tm_yday;
		qtime->tm_isdst = tms->tm_isdst;
	}
	return (int) t;
}
