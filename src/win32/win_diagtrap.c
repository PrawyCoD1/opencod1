#include <windows.h>
#include <stdio.h>

static void          *diag_watchAddr;
static unsigned long  diag_watchPage;
static unsigned long  diag_ignoreFrom;
static int            diag_logCount;
static int            diag_nulledSeen;
static unsigned long  diag_poisonValue;
static unsigned long  diag_lastEip;
static unsigned long  diag_lastTarget;
static int            diag_valueMode;
static unsigned long  diag_lastRets[4];

#define DIAG_LOG_CAP 400

static FILE *diag_file;

/* ---- DiagOut  no-address ---- */
static FILE *DiagOut( void )
{
	if ( !diag_file ) {
		diag_file = fopen( "diagwatch.txt", "w" );
		if ( !diag_file ) {
			return stderr;
		}
	}
	return diag_file;
}

typedef LONG ( WINAPI *PVECTORED_HANDLER )( struct _EXCEPTION_POINTERS * );
typedef PVOID ( WINAPI *PFN_AddVEH )( ULONG, PVECTORED_HANDLER );

/* ---- Diag_Handler  no-address ---- */
static LONG WINAPI Diag_Handler( struct _EXCEPTION_POINTERS *ep )
{
	DWORD code = ep->ExceptionRecord->ExceptionCode;
	DWORD prot;

	if ( code == EXCEPTION_SINGLE_STEP ) {
		VirtualProtect( (void *)diag_watchPage, 1, PAGE_READONLY, &prot );
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	if ( code == EXCEPTION_ACCESS_VIOLATION
	     && ep->ExceptionRecord->NumberParameters >= 2
	     && ep->ExceptionRecord->ExceptionInformation[0] == 1  ) {
		DWORD tgt = (DWORD)ep->ExceptionRecord->ExceptionInformation[1];
		if ( ( tgt & ~0xFFFUL ) == diag_watchPage ) {
			if ( diag_valueMode ) {
				DWORD  prot4;
				DWORD *bp = (DWORD *)ep->ContextRecord->Ebp;
				int    i;
				diag_lastEip    = ep->ContextRecord->Eip;
				diag_lastTarget = tgt;
				for ( i = 0; i < 4; i++ ) {
					if ( !bp || IsBadReadPtr( bp, 8 ) ) {
						diag_lastRets[i] = 0;
						continue;
					}
					diag_lastRets[i] = bp[1];
					bp = (DWORD *)bp[0];
				}
				VirtualProtect( (void *)diag_watchPage, 1, PAGE_READWRITE, &prot4 );
				ep->ContextRecord->EFlags |= 0x100;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
			if ( diag_poisonValue ) {
				DWORD prot3;
				diag_lastEip    = ep->ContextRecord->Eip;
				diag_lastTarget = tgt;
				VirtualProtect( (void *)diag_watchPage, 1, PAGE_READWRITE, &prot3 );
				ep->ContextRecord->EFlags |= 0x100;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
			if ( diag_ignoreFrom && tgt >= diag_ignoreFrom ) {
				DWORD prot2;
				VirtualProtect( (void *)diag_watchPage, 1, PAGE_READWRITE, &prot2 );
				ep->ContextRecord->EFlags |= 0x100;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
			{
			static DWORD seenEip[64];
			static int   seenN[64];
			int          s, known = 0;
			for ( s = 0; s < 64 && seenEip[s]; s++ ) {
				if ( seenEip[s] == ep->ContextRecord->Eip ) {
					known = ( ++seenN[s] > 3 );
					break;
				}
			}
			if ( s < 64 && !seenEip[s] ) {
				seenEip[s] = ep->ContextRecord->Eip;
				seenN[s] = 1;
			}
			if ( !known && diag_logCount < DIAG_LOG_CAP ) {
				DWORD *bp = (DWORD *)ep->ContextRecord->Ebp;
				int    i;
				diag_logCount++;
				fprintf( DiagOut(), "DIAG-WRITE #%03d eip=%08lx -> %08lx",
				         diag_logCount, ep->ContextRecord->Eip, tgt );
				for ( i = 0; i < 4 && bp
				      && !IsBadReadPtr( bp, 8 ); i++ ) {
					fprintf( DiagOut(), " ret=%08lx", bp[1] );
					bp = (DWORD *)bp[0];
				}
				fprintf( DiagOut(), "\n" );
			} else if ( diag_logCount == DIAG_LOG_CAP ) {
				diag_logCount++;
				fprintf( DiagOut(), "DIAG-WRITE log cap reached; still watching the sentinel\n" );
			}
			VirtualProtect( (void *)diag_watchPage, 1, PAGE_READWRITE, &prot );
			ep->ContextRecord->EFlags |= 0x100;
			fflush( DiagOut() );
			return EXCEPTION_CONTINUE_EXECUTION;
			}
		}
	}
	if ( !diag_nulledSeen && diag_watchAddr && *(DWORD *)diag_watchAddr == 0 ) {
		diag_nulledSeen = 1;
		fprintf( DiagOut(), ">>> SENTINEL r_lightmap IS NOW NULL -- the write above did it <<<\n" );
		fflush( DiagOut() );
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

/* ---- Diag_HandlerOuter  no-address ---- */
static LONG WINAPI Diag_HandlerOuter( struct _EXCEPTION_POINTERS *ep )
{
	LONG r = Diag_Handler( ep );
	if ( ep->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP
	     && diag_valueMode && diag_lastTarget ) {
		if ( diag_lastTarget < (DWORD)diag_watchAddr + 4
		     && diag_lastTarget + 4 > (DWORD)diag_watchAddr ) {
			static DWORD vEip[32];
			static int   vN[32];
			int          s, suppressed = 0;
			for ( s = 0; s < 32 && vEip[s]; s++ ) {
				if ( vEip[s] == diag_lastEip ) {
					suppressed = ( ++vN[s] > 6 );
					break;
				}
			}
			if ( s < 32 && !vEip[s] ) {
				vEip[s] = diag_lastEip;
				vN[s] = 1;
			}
			if ( !suppressed ) {
				fprintf( DiagOut(),
				         "DIAG-HIT eip=%08lx tgt=%08lx val=%08lx ret=%08lx,%08lx,%08lx,%08lx\n",
				         diag_lastEip, diag_lastTarget,
				         *(DWORD *)diag_lastTarget,
				         diag_lastRets[0], diag_lastRets[1],
				         diag_lastRets[2], diag_lastRets[3] );
				fflush( DiagOut() );
			}
		}
		diag_lastTarget = 0;
	}
	if ( ep->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP
	     && !diag_nulledSeen && diag_watchAddr ) {
		if ( diag_poisonValue && diag_lastTarget
		     && *(DWORD *)diag_lastTarget == diag_poisonValue ) {
			diag_nulledSeen = 1;
			fprintf( DiagOut(), ">>> POISON %08lx WRITTEN: eip=%08lx target=%08lx <<<\n",
			         diag_poisonValue, diag_lastEip, diag_lastTarget );
			fflush( DiagOut() );
		} else if ( !diag_poisonValue
		            && *(DWORD *)diag_watchAddr == 0 ) {
			diag_nulledSeen = 1;
			fprintf( DiagOut(), ">>> SENTINEL r_lightmap NULLED BY DIAG-WRITE #%03d (see line above) <<<\n",
			         diag_logCount );
			fflush( DiagOut() );
		}
	}
	return r;
}

void Diag_ArmValueWatch( void *addr );

/* ---- Diag_SetPoisonValue  no-address ---- */
void Diag_SetPoisonValue( unsigned long v )
{
	diag_poisonValue = v;
}

/* ---- Diag_SetIgnoreFrom  no-address ---- */
void Diag_SetIgnoreFrom( void *addr )
{
	diag_ignoreFrom = (DWORD)addr;
}

/* ---- Diag_ArmWriteWatch  no-address ---- */
void Diag_ArmWriteWatch( void *addr )
{
	HMODULE    k32 = GetModuleHandleA( "kernel32.dll" );
	PFN_AddVEH addVeh;
	DWORD      prot;

	addVeh = (PFN_AddVEH)GetProcAddress( k32, "AddVectoredExceptionHandler" );
	if ( !addVeh ) {
		fprintf( DiagOut(), "DIAG-WATCH: AddVectoredExceptionHandler unavailable\n" );
		return;
	}
	diag_watchAddr = addr;
	diag_watchPage = (DWORD)addr & ~0xFFFUL;
	addVeh( 1, Diag_HandlerOuter );
	VirtualProtect( addr, 1, PAGE_READONLY, &prot );
	fprintf( DiagOut(), "DIAG-WATCH armed: sentinel &r_lightmap=%08lx page=%08lx (old prot %lu)\n",
	         (DWORD)addr, diag_watchPage, prot );
	fflush( DiagOut() );
}

/* ---- Diag_ArmValueWatch  no-address ---- */
void Diag_ArmValueWatch( void *addr )
{
	diag_valueMode = 1;
	diag_nulledSeen = 1;
	Diag_ArmWriteWatch( addr );
}
