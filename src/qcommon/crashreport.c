/*
 * qcommon/crashreport.c
 *
 * @fidelity: verified
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>
#include <string.h>

/* Reporting must not enter the CRT allocator: the original exception may
 * already be inside a damaged heap, or on a driver thread without CRT state. */
static HANDLE crash_file = INVALID_HANDLE_VALUE;
static volatile LONG crash_reporting;

static void CR_Printf( const char *fmt, ... ) {
	char text[2048];
	DWORD written;
	int length;
	va_list ap;
	va_start( ap, fmt );
	/* All formats are internal, with bounded module paths and integer values.
	 * Win32 wvsprintf has a 1024-character output limit. */
	length = wvsprintfA( text, fmt, ap );
	va_end( ap );
	if ( length > 0 && crash_file != INVALID_HANDLE_VALUE ) {
		WriteFile( crash_file, text, (DWORD)length, &written, NULL );
	}
}

static void CR_Where( char *out, int outSize, unsigned long addr ) {
	MEMORY_BASIC_INFORMATION mbi;
	char path[MAX_PATH];
	char *base;

	out[0] = 0;
	if ( !addr || !VirtualQuery( (void *) addr, &mbi, sizeof( mbi ) ) ) {
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
	wsprintfA( out, "%s+0x%lX", base,
	           addr - (unsigned long) mbi.AllocationBase );
	out[outSize - 1] = 0;
}

static const char *ExceptionName( DWORD code ) {
	switch ( code ) {
	case EXCEPTION_ACCESS_VIOLATION:      return "ACCESS_VIOLATION";
	case EXCEPTION_STACK_OVERFLOW:        return "STACK_OVERFLOW";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:    return "INT_DIVIDE_BY_ZERO";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:    return "FLT_DIVIDE_BY_ZERO";
	case EXCEPTION_ILLEGAL_INSTRUCTION:   return "ILLEGAL_INSTRUCTION";
	case EXCEPTION_PRIV_INSTRUCTION:      return "PRIV_INSTRUCTION";
	case EXCEPTION_IN_PAGE_ERROR:         return "IN_PAGE_ERROR";
	default:                              return "?";
	}
}

static LONG WINAPI Cod1_CrashFilter( EXCEPTION_POINTERS *ep ) {
	EXCEPTION_RECORD *er = ep->ExceptionRecord;
	CONTEXT *cx = ep->ContextRecord;
	unsigned int *frame;
	char where[MAX_PATH + 32];
	SYSTEMTIME now;
	int i;

	/* A second fault must reach Windows rather than recurse into reporting. */
	if ( InterlockedCompareExchange( &crash_reporting, 1, 0 ) ) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	crash_file = CreateFileA( "crash_mp.txt", FILE_APPEND_DATA,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL );
	GetLocalTime( &now );
	CR_Printf( "\n=== crash === %04u-%02u-%02u %02u:%02u:%02u\n",
		now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond );
	CR_Printf( "  pid    %lu\n", (unsigned long) GetCurrentProcessId() );
	CR_Printf( "  code   0x%08lX  %s\n",
	           (unsigned long) er->ExceptionCode, ExceptionName( er->ExceptionCode ) );
	CR_Where( where, sizeof( where ), (unsigned long) cx->Eip );
	CR_Printf( "  eip    0x%08lX  %s\n", (unsigned long) cx->Eip, where );

	if ( er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2 ) {
		CR_Printf( "  %s 0x%08lX\n",
		           er->ExceptionInformation[0] ? "writing" : "reading",
		           (unsigned long) er->ExceptionInformation[1] );
	}

	CR_Printf( "  eax=%08lX ebx=%08lX ecx=%08lX edx=%08lX\n",
	           (unsigned long) cx->Eax, (unsigned long) cx->Ebx,
	           (unsigned long) cx->Ecx, (unsigned long) cx->Edx );
	CR_Printf( "  esi=%08lX edi=%08lX ebp=%08lX esp=%08lX\n",
	           (unsigned long) cx->Esi, (unsigned long) cx->Edi,
	           (unsigned long) cx->Ebp, (unsigned long) cx->Esp );

	{
		extern const char *Cod1_HangWatchdogLastNote( void );
		const char *note = Cod1_HangWatchdogLastNote();
		if ( note ) {
			char boundedNote[512];
			lstrcpynA( boundedNote, note, sizeof( boundedNote ) );
			CR_Printf( "  last   %s\n", boundedNote );
		}
	}

	CR_Printf( "  ebp chain (approximate):\n" );
	frame = (unsigned int *) cx->Ebp;
	for ( i = 0; i < 12; i++ ) {
		if ( IsBadReadPtr( frame, 8 ) ) {
			break;
		}
		CR_Where( where, sizeof( where ), frame[1] );
		CR_Printf( "    ret 0x%08X  %s\n", frame[1], where );
		frame = (unsigned int *) frame[0];
	}

	CR_Printf( "  stack scan (approximate, may include stale frames):\n" );
	{
		unsigned int *sp = (unsigned int *) ( cx->Esp & ~3UL );
		int hits = 0;
		for ( i = 0; i < 1024 && hits < 32; i++, sp++ ) {
			unsigned int v;
			if ( IsBadReadPtr( sp, 4 ) ) {
				break;
			}
			v = *sp;
			if ( ( v >= 0x00401000 && v < 0x00580000 ) ||
			     ( v >= 0x10000000 && v < 0x70000000 ) ) {
				if ( !IsBadReadPtr( (void *) ( v - 5 ), 5 ) ) {
					CR_Where( where, sizeof( where ), v );
					CR_Printf( "    [esp+%04X] 0x%08X  %s\n",
					           (unsigned int) ( (char *) sp - (char *) cx->Esp ),
					           v, where );
					hits++;
				}
			}
		}
	}

	CR_Printf( "=== end crash ===\n" );
	if ( crash_file != INVALID_HANDLE_VALUE ) {
		FlushFileBuffers( crash_file );
		CloseHandle( crash_file );
		crash_file = INVALID_HANDLE_VALUE;
	}

	/* Preserve the original exception for Windows Error Reporting and dumps. */
	return EXCEPTION_CONTINUE_SEARCH;
}

void Cod1_InstallCrashReporter( void ) {
	HMODULE k32 = GetModuleHandleA( "kernel32.dll" );
	BOOL ( WINAPI *pSetThreadStackGuarantee )( ULONG * ) =
		(BOOL ( WINAPI * )( ULONG * )) GetProcAddress( k32, "SetThreadStackGuarantee" );
	ULONG guarantee = 64 * 1024;

	if ( pSetThreadStackGuarantee ) {
		pSetThreadStackGuarantee( &guarantee );
	}
	/* Do not report first-chance exceptions. Drivers and libraries may handle
	 * those themselves; our reporter runs only after their handlers decline. */
	SetUnhandledExceptionFilter( Cod1_CrashFilter );
}
