/*
 * qcommon/crt_thunks.c
 *
 * @fidelity: verified
 */

#include <stdlib.h>

/* ---- j__atol  0x00525445 ---- */
long j__atol( const char *string ) {
	return atol( string );
}

/* ---- j__free  0x00528435 ---- */
void j__free( void *block ) {
	free( block );
}

/* ---- qsort_m  0x00526580 ---- */
void qsort_m( void *base, unsigned int count, unsigned int width,
			  int ( __cdecl *compare )( const void *, const void * ) ) {
	qsort( base, count, width, compare );
}

/* The CRT _putenv (_lock(7) is _ENV_LOCK; the guarded call forwards to one
 * internal worker).  Named putenv_m, not _putenv, on the qsort_m precedent
 * above -- the undecorated CRT spelling would collide with the <stdlib.h>
 * import this very line calls.  CL_Setenv_f is the only caller, in retail
 * (0x0040F55D) and here. */
/* ---- putenv_m  0x005370C6 ---- */
int putenv_m( const char *envstring ) {
	return _putenv( envstring );
}

#if !defined( _MSC_VER ) || _MSC_VER < 1400 /* the __rdtsc intrinsic arrived with VC8; VC7 (1300) lacks it */
__declspec( naked ) int __rdtsc( void ) {
	__asm {
		push    edx
		_emit   0x0F
		_emit   0x31
		pop     edx
		ret
	}
}
#endif
