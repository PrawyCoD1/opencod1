/*
 * qcommon/hangwatch.c
 *
 * @fidelity: n/a
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define HW_DEFAULT_TIMEOUT_MSEC   15000
#define HW_POLL_MSEC                250
#define HW_REPEAT_MSEC             5000
#define HW_FULL_RECORDS               3
#define HW_MAX_RECORDS              200
#define HW_RETS                      16
#define HW_STACK_WORDS             1024

static volatile LONG   hw_progress;
static volatile LONG   hw_installed;
static HANDLE          hw_mainThread;
static DWORD           hw_timeoutMsec = HW_DEFAULT_TIMEOUT_MSEC;
static DWORD           hw_exeBase;
static DWORD           hw_exeEnd;
static int             hw_records;
static char            hw_note[256];

static HANDLE          hw_file = INVALID_HANDLE_VALUE;

/* HW_Out  0x0054A85A */

static void HW_Out( const char *s ) {
	DWORD  n;
	DWORD  len = (DWORD) strlen( s );
	HANDLE err;

	if ( hw_file == INVALID_HANDLE_VALUE ) {
		hw_file = CreateFileA( "hang_mp.txt", GENERIC_WRITE,
		                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hw_file != INVALID_HANDLE_VALUE ) {
			SetFilePointer( hw_file, 0, NULL, FILE_END );
		}
	}
	if ( hw_file != INVALID_HANDLE_VALUE ) {
		WriteFile( hw_file, s, len, &n, NULL );
		FlushFileBuffers( hw_file );
	}

	err = GetStdHandle( STD_ERROR_HANDLE );
	if ( err && err != INVALID_HANDLE_VALUE ) {
		WriteFile( err, s, len, &n, NULL );
	}
}

static void HW_Printf( const char *fmt, ... ) {
	char    buf[1024];
	va_list ap;

	va_start( ap, fmt );
	wvsprintfA( buf, fmt, ap );
	va_end( ap );
	HW_Out( buf );
}

static const char *HW_Own( DWORD addr, char *out ) {
	if ( addr >= hw_exeBase && addr < hw_exeEnd ) {
		wsprintfA( out, "exe+0x%lX", addr - hw_exeBase );
	} else {
		out[0] = 0;
	}
	return out;
}

static void HW_Foreign( DWORD addr ) {
	MEMORY_BASIC_INFORMATION mbi;
	char  path[MAX_PATH];
	char *base;

	if ( !addr || ( addr >= hw_exeBase && addr < hw_exeEnd ) ) {
		return;
	}
	if ( !VirtualQuery( (void *) addr, &mbi, sizeof( mbi ) ) ) {
		return;
	}
	if ( mbi.State != MEM_COMMIT || !mbi.AllocationBase ) {
		return;
	}
	if ( !GetModuleFileNameA( (HMODULE) mbi.AllocationBase, path, sizeof( path ) ) ) {
		return;
	}
	base = strrchr( path, '\\' );
	base = base ? base + 1 : path;
	HW_Printf( "    0x%08lX  %s+0x%lX\n", addr, base,
	           addr - (DWORD) mbi.AllocationBase );
}

typedef struct {
	CONTEXT ctx;
	DWORD   rets[HW_RETS];
	int     numRets;
	DWORD   stack[HW_STACK_WORDS];
	int     numStack;
	int     ok;
} hwSnapshot_t;

static hwSnapshot_t hw_snap;
static DWORD        hw_lastEip;

static void HW_Capture( hwSnapshot_t *s ) {
	DWORD *frame;
	DWORD *sp;
	int    i;

	s->ok = 0;
	s->numRets = 0;
	s->numStack = 0;

	if ( SuspendThread( hw_mainThread ) == (DWORD) -1 ) {
		return;
	}

	s->ctx.ContextFlags = CONTEXT_FULL;
	if ( GetThreadContext( hw_mainThread, &s->ctx ) ) {
		s->ok = 1;

		frame = (DWORD *) s->ctx.Ebp;
		for ( i = 0; i < HW_RETS; i++ ) {
			if ( IsBadReadPtr( frame, 8 ) ) {
				break;
			}
			s->rets[s->numRets++] = frame[1];
			frame = (DWORD *) frame[0];
		}

		sp = (DWORD *) ( s->ctx.Esp & ~3UL );
		for ( i = 0; i < HW_STACK_WORDS; i++ ) {
			if ( IsBadReadPtr( sp + i, 4 ) ) {
				break;
			}
			s->stack[s->numStack++] = sp[i];
		}
	}

	ResumeThread( hw_mainThread );
}

static void HW_WriteRecord( hwSnapshot_t *s, const char *why, int stalledMsec, int full ) {
	SYSTEMTIME st;
	char       where[64];
	int        i;

	if ( !s->ok ) {
		HW_Printf( "\n=== hang === capture FAILED (%s)\n", why );
		return;
	}

	if ( !full ) {
		HW_Printf( "  ... still stalled %ld s, eip 0x%08lX %s  ret %08lX %08lX %08lX\n",
		           (long) ( stalledMsec / 1000 ), s->ctx.Eip,
		           HW_Own( s->ctx.Eip, where ),
		           s->numRets > 0 ? s->rets[0] : 0,
		           s->numRets > 1 ? s->rets[1] : 0,
		           s->numRets > 2 ? s->rets[2] : 0 );
		return;
	}

	GetLocalTime( &st );
	HW_Printf( "\n=== hang === %04d-%02d-%02d %02d:%02d:%02d\n",
	           st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );
	HW_Printf( "  pid    %lu\n", GetCurrentProcessId() );
	HW_Printf( "  reason %s, %ld s with no frame progress (frame %ld)\n",
	           why, (long) ( stalledMsec / 1000 ), (long) hw_progress );
	if ( hw_note[0] ) {
		HW_Printf( "  last   %s\n", hw_note );
	}
	HW_Printf( "  eip    0x%08lX  %s\n", s->ctx.Eip, HW_Own( s->ctx.Eip, where ) );
	HW_Printf( "  eax=%08lX ebx=%08lX ecx=%08lX edx=%08lX\n",
	           s->ctx.Eax, s->ctx.Ebx, s->ctx.Ecx, s->ctx.Edx );
	HW_Printf( "  esi=%08lX edi=%08lX ebp=%08lX esp=%08lX\n",
	           s->ctx.Esi, s->ctx.Edi, s->ctx.Ebp, s->ctx.Esp );

	HW_Printf( "  ebp chain (approximate):\n" );
	for ( i = 0; i < s->numRets; i++ ) {
		HW_Printf( "    ret 0x%08lX  %s\n", s->rets[i], HW_Own( s->rets[i], where ) );
	}

	HW_Printf( "  eip burst (16 samples, 5 ms apart):\n   " );
	for ( i = 0; i < 16; i++ ) {
		DWORD eip = 0;
		if ( SuspendThread( hw_mainThread ) != (DWORD) -1 ) {
			CONTEXT c;
			c.ContextFlags = CONTEXT_CONTROL;
			if ( GetThreadContext( hw_mainThread, &c ) ) {
				eip = c.Eip;
			}
			ResumeThread( hw_mainThread );
		}
		HW_Printf( " 0x%08lX", eip );
		Sleep( 5 );
	}
	HW_Printf( "\n" );

	HW_Printf( "  stack scan (approximate, may include stale frames):\n" );
	{
		int hits = 0;
		for ( i = 0; i < s->numStack && hits < 32; i++ ) {
			DWORD v = s->stack[i];
			if ( v > hw_exeBase && v < hw_exeEnd ) {
				HW_Printf( "    [esp+%04X] 0x%08lX  %s\n",
				           (unsigned int) ( i * 4 ), v, HW_Own( v, where ) );
				hits++;
			}
		}
	}

	HW_Printf( "  foreign modules seen on the stack:\n" );
	{
		DWORD seen[16];
		int   nseen = 0;
		int   j;
		for ( i = -1; i < s->numRets && nseen < 16; i++ ) {
			DWORD v = ( i < 0 ) ? s->ctx.Eip : s->rets[i];
			if ( !v || ( v >= hw_exeBase && v < hw_exeEnd ) ) {
				continue;
			}
			for ( j = 0; j < nseen; j++ ) {
				if ( ( seen[j] & ~0xFFFFUL ) == ( v & ~0xFFFFUL ) ) {
					break;
				}
			}
			if ( j < nseen ) {
				continue;
			}
			seen[nseen++] = v;
			HW_Foreign( v );
		}
	}
	HW_Printf( "=== end hang ===\n" );
}

static DWORD WINAPI HW_Thread( LPVOID unused ) {
	LONG  lastProgress = -1;
	DWORD lastMoveTick = GetTickCount();
	DWORD lastSnapTick = 0;
	int   inStall = 0;
	int   stallRecords = 0;
	int   stallEipMoved = 0;

	(void) unused;

	for ( ;; ) {
		DWORD now;

		Sleep( HW_POLL_MSEC );
		now = GetTickCount();

		if ( hw_progress != lastProgress ) {
			if ( inStall ) {
				HW_Printf( "  RECOVERED after %ld s -- %s\n",
				           (long) ( ( now - lastMoveTick ) / 1000 ),
				           stallEipMoved
				             ? "the eip moved between snapshots, so that was a slow "
				               "section, not a hang"
				             : "the eip stayed put or circled one small region, so the main "
				               "thread really was stuck; whatever held it has now let go" );
				inStall = 0;
				stallRecords = 0;
				stallEipMoved = 0;
			}
			lastProgress = hw_progress;
			lastMoveTick = now;
			continue;
		}

		if ( hw_records >= HW_MAX_RECORDS ) {
			continue;
		}
		if ( now - lastMoveTick < hw_timeoutMsec ) {
			continue;
		}
		if ( inStall && now - lastSnapTick < HW_REPEAT_MSEC ) {
			continue;
		}

		lastSnapTick = now;
		HW_Capture( &hw_snap );
		hw_records++;

		if ( !inStall ) {
			inStall = 1;
			stallRecords = 0;
			stallEipMoved = 0;
		}
		HW_WriteRecord( &hw_snap, hw_progress == 0 ? "no frame has completed yet"
		                                           : "main thread stalled",
		                (int) ( now - lastMoveTick ),
		                stallRecords < HW_FULL_RECORDS );
		if ( hw_snap.ok ) {
			if ( stallRecords > 0 ) {
				DWORD d = hw_snap.ctx.Eip > hw_lastEip
				          ? hw_snap.ctx.Eip - hw_lastEip
				          : hw_lastEip - hw_snap.ctx.Eip;
				if ( hw_snap.ctx.Eip == hw_lastEip ) {
					HW_Printf( "  >>> eip unchanged across snapshots -- BLOCKED on something "
					           "that never completes <<<\n" );
				} else if ( d < 0x1000 ) {
					HW_Printf( "  >>> eip moved %ld bytes but stayed in one region -- this is a "
					           "SPIN; the burst above is the loop body <<<\n", (long) d );
				} else {
					stallEipMoved = 1;
				}
			}
			hw_lastEip = hw_snap.ctx.Eip;
		}
		stallRecords++;

		if ( hw_records == HW_MAX_RECORDS ) {
			HW_Printf( "  record cap reached; the watchdog stops writing\n" );
		}
	}
}

void Cod1_HangWatchdogTick( void ) {
	hw_progress++;
}

/* Cod1_HangWatchdogLastNote -- the crash reporter prints this too, so a
   diagnostic Com_Printf is not lost when the process faults instead of
   stalling.  hang_mp.txt only ever saw it because the watchdog fired. */
const char *Cod1_HangWatchdogLastNote( void ) {
	return hw_note[0] ? hw_note : 0;
}

void Cod1_HangWatchdogNote( const char *msg ) {
	int i;

	if ( !hw_installed || !msg ) {
		return;
	}
	for ( i = 0; i < (int) sizeof( hw_note ) - 1 && msg[i]; i++ ) {
		hw_note[i] = ( msg[i] == '\n' || msg[i] == '\r' ) ? ' ' : msg[i];
	}
	hw_note[i] = 0;
}

void Cod1_InstallHangWatchdog( void ) {
	IMAGE_DOS_HEADER *dos;
	IMAGE_NT_HEADERS *nt;
	const char       *e;
	HANDLE            thread;
	DWORD             id;
	SYSTEMTIME        st;

	if ( hw_installed ) {
		return;
	}

	e = getenv( "COD1_HANGWATCH" );
	if ( e ) {
		int secs = atoi( e );
		if ( secs <= 0 ) {
			return;
		}
		hw_timeoutMsec = (DWORD) secs * 1000;
	}

	if ( !DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
	                       GetCurrentProcess(), &hw_mainThread,
	                       0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
		return;
	}

	hw_exeBase = (DWORD) GetModuleHandleA( NULL );
	dos = (IMAGE_DOS_HEADER *) hw_exeBase;
	nt  = (IMAGE_NT_HEADERS *) ( hw_exeBase + dos->e_lfanew );
	hw_exeEnd = hw_exeBase + nt->OptionalHeader.SizeOfImage;

	thread = CreateThread( NULL, 0, HW_Thread, NULL, 0, &id );
	if ( !thread ) {
		CloseHandle( hw_mainThread );
		hw_mainThread = NULL;
		return;
	}
	CloseHandle( thread );
	hw_installed = 1;

	GetLocalTime( &st );
	HW_Printf( "\n=== hangwatch armed === %04d-%02d-%02d %02d:%02d:%02d  pid %lu  "
	           "timeout %ld s  image 0x%08lX..0x%08lX\n",
	           st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
	           GetCurrentProcessId(), (long) ( hw_timeoutMsec / 1000 ),
	           hw_exeBase, hw_exeEnd );
}
