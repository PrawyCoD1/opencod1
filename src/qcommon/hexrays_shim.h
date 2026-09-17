/*
 * Hex-Rays pseudocode compatibility layer.
 *
 * Raw translation units (everything carrying the "@fidelity: raw" banner) use
 * the vocabulary of IDA's defs.h.  This header supplies that vocabulary for
 * MSVC 6 without dragging in the real defs.h, which assumes a newer compiler.
 *
 * The macro semantics here match defs.h exactly, INCLUDING the parts that look
 * wrong:
 *
 *   HIBYTE(x) is byte 1, not the top byte of a dword.  Hex-Rays only emits
 *   HIBYTE for 16-bit values and uses BYTE3 for the top of a 32-bit one, so
 *   +1 is correct for the code that was generated against it.  Do not
 *   "fix" this to +3.
 *
 * WHAT THIS HEADER CANNOT FIX -- a raw unit's __usercall/__userpurge functions
 * are spelled __cdecl, so its ABI is NOT the retail ABI; anything that has to
 * interoperate with retail code, a retail DLL, or a hand-written asm thunk
 * must be reconstructed properly first.  Register-to-parameter order in a raw
 * prototype is unreliable, and an untyped access such as *(_DWORD *)(p + 8)
 * has a real offset but a guessed width.
 */

#ifndef __HEXRAYS_SHIM_H__
#define __HEXRAYS_SHIM_H__

typedef unsigned char       _BYTE;
typedef unsigned short      _WORD;
typedef unsigned int        _DWORD;
typedef unsigned __int64    _QWORD;
typedef int                 _UNKNOWN;

typedef signed char         _SBYTE;
typedef short               _SWORD;
typedef int                 _SDWORD;
typedef __int64             _SQWORD;

#ifndef BOOL
typedef int                 BOOL;
#endif

typedef unsigned int        uint32;
typedef unsigned short      uint16;
typedef unsigned char       uint8;

/* ---- sub-register access, defs.h semantics ---- */

#define LOBYTE(x)   (*((_BYTE  *)&(x)))
#define LOWORD(x)   (*((_WORD  *)&(x)))
#define LODWORD(x)  (*((_DWORD *)&(x)))
#define HIBYTE(x)   (*((_BYTE  *)&(x) + 1))
#define HIWORD(x)   (*((_WORD  *)&(x) + 1))
#define HIDWORD(x)  (*((_DWORD *)&(x) + 1))

#define BYTEn(x, n) (*((_BYTE  *)&(x) + n))
#define WORDn(x, n) (*((_WORD  *)&(x) + n))
#define BYTE1(x)    BYTEn(x, 1)
#define BYTE2(x)    BYTEn(x, 2)
#define BYTE3(x)    BYTEn(x, 3)
/* Byte 4 is the low byte of the HIGH dword of a 64-bit value; Hex-Rays emits
 * it when a __int64 is being taken apart.  One use, cl_cgame_mp.c:1620. */
#define BYTE4(x)    BYTEn(x, 4)
#define WORD1(x)    WORDn(x, 1)
#define WORD2(x)    WORDn(x, 2)

#define SLOBYTE(x)  (*((_SBYTE  *)&(x)))
#define SLOWORD(x)  (*((_SWORD  *)&(x)))
#define SLODWORD(x) (*((_SDWORD *)&(x)))
#define SHIBYTE(x)  (*((_SBYTE  *)&(x) + 1))
#define SHIWORD(x)  (*((_SWORD  *)&(x) + 1))
#define SHIDWORD(x) (*((_SDWORD *)&(x) + 1))
#define SBYTEn(x,n) (*((_SBYTE  *)&(x) + n))
#define SBYTE1(x)   SBYTEn(x, 1)
#define SBYTE2(x)   SBYTEn(x, 2)
#define SBYTE3(x)   SBYTEn(x, 3)

/* ---- helpers Hex-Rays emits by name ---- */

#define qmemcpy     memcpy

#define __ROL4__(x, n)  ( ( (_DWORD)(x) << ( (n) & 31 ) ) | ( (_DWORD)(x) >> ( 32 - ( (n) & 31 ) ) ) )
#define __ROR4__(x, n)  ( ( (_DWORD)(x) >> ( (n) & 31 ) ) | ( (_DWORD)(x) << ( 32 - ( (n) & 31 ) ) ) )
#define __ROL2__(x, n)  ( ( (_WORD)(x)  << ( (n) & 15 ) ) | ( (_WORD)(x)  >> ( 16 - ( (n) & 15 ) ) ) )
#define __ROR2__(x, n)  ( ( (_WORD)(x)  >> ( (n) & 15 ) ) | ( (_WORD)(x)  << ( 16 - ( (n) & 15 ) ) ) )
#define __ROL1__(x, n)  ( ( (_BYTE)(x)  << ( (n) & 7  ) ) | ( (_BYTE)(x)  >> ( 8  - ( (n) & 7  ) ) ) )
#define __ROR1__(x, n)  ( ( (_BYTE)(x)  >> ( (n) & 7  ) ) | ( (_BYTE)(x)  << ( 8  - ( (n) & 7  ) ) ) )

#define __PAIR64__(hi, lo)  ( ( (unsigned __int64)(hi) << 32 ) | (_DWORD)(lo) )

/* Carry/overflow helpers.  These appear when the decompiler could not fold a
 * flag test back into the expression that produced it. */
#define __CFADD__(x, y)     ( (_DWORD)( (_DWORD)(x) + (_DWORD)(y) ) < (_DWORD)(x) )
#define __CFSUB__(x, y)     ( (_DWORD)(x) < (_DWORD)(y) )
#define __OFADD__(x, y)     ( ( ( (x) ^ ( (x) + (y) ) ) & ( (y) ^ ( (x) + (y) ) ) ) < 0 )
#define __OFSUB__(x, y)     ( ( ( (x) ^ (y) ) & ( (x) ^ ( (x) - (y) ) ) ) < 0 )
#define __SETP__(x, y)      ( 0 )   /* parity: never load-bearing in this binary */

/* __UNORDERED__  --  ZERO IS THE CORRECT ANSWER.  This is not a stub.  Do not
 * re-implement it as a real NaN test without reading the table below.
 *
 * Hex-Rays emits `__UNORDERED__` beside a comparison because MSVC's
 * `fcom / fnstsw ax / test ah, <mask> / j<cc>` folds the relational answer and
 * the unordered answer into one branch.  Whether that is expressible in plain C
 * depends entirely on the mask/branch PAIR, because C already has its own NaN
 * rules: `==`, `<`, `>`, `<=`, `>=` are all FALSE when either operand is NaN,
 * and `!=` is TRUE.
 *
 * Flags after FCOM:  greater C3C2C0=000, less=001, equal=100, unordered=111
 * (C0 -> ah bit 0x01, C2 -> 0x04, C3 -> 0x40).  So:
 *
 *   mask  branch  set of cases taken        plain C equivalent       exact?
 *   ----  ------  ------------------------  -----------------------  ------
 *   44h   jnp     {equal}                   a == b                    YES
 *   44h   jp      {gt, lt, unordered}       a != b                    YES
 *   5     jnp     {less}                    a < b                     YES
 *   5     jp      {gt, eq, unordered}       !(a < b)                  YES
 *   41h   jz      {greater}                 a > b                     YES
 *   41h   jnz     {lt, eq, unordered}       !(a > b)                  YES
 *   1     jz      {gt, eq}                  a >= b                    YES
 *   41h   jp      {greater, unordered}      !(a <= b)                 NO
 *   1     jnz     {less, unordered}         !(a >= b)                 NO
 *
 * Over the whole of .text: 1,411 of these sequences, of which only 155 are the
 * two divergent rows (41h+jp 70, 1+jnz 85).  `test ah,44h` is followed by
 * `jz`/`jnz` -- the only pairing that would mean "equal OR unordered" and so
 * justify `(a == b) | __UNORDERED__(a,b)` -- zero times in this binary.  Every
 * one of the 336 `44h`+`jp` and 202 `44h`+`jnp` sites is plain C `!=` / `==`,
 * NaN included.
 *
 * So for the ~1,256 non-divergent sites, `(a REL b) | 0` reduces to exactly the
 * retail behaviour, and a REAL NaN test INVERTS them.  Example: Con_DrawConsole
 * 0x0040A1F3 is `fld con_displayFrac / fcomp 0.0 / fnstsw ax / test ah,44h /
 * jnp locret`, so retail returns only on ORDERED equal and a NaN falls through
 * and DRAWS; `if ( (con_displayFrac == 0.0) | __UNORDERED__(...) ) return;` is
 * correct with 0, and with a real NaN test it returns instead of drawing.
 *
 * THE TWO DIVERGENT ROWS ARE REAL and are NOT fixed by this macro either way:
 * they need the comparison rewritten at the site as `!(a <= b)` or
 * `!(a >= b)` (as RE_SetFarPlaneDist does).  A macro cannot distinguish them,
 * because by the time it is expanded the branch polarity is gone.  They show
 * up as the `<`- and `>`-shaped sites; the `==`-shaped ones are never
 * divergent. */
#define __UNORDERED__(a, b) ( 0 )

#ifndef __debugbreak
#define __debugbreak()      do { __asm { int 3 } } while ( 0 )
#endif

/* Intrinsics Hex-Rays names but MSVC 6 does not provide.  These are only ever
 * reached from machine-translated code; a hand reconstruction should express
 * the operation directly instead of calling one of these. */
#define __readeflags()      ( 0u )
#define __writeeflags(x)    ( (void)(x) )
#define COERCE_FLOAT(x)     ( *(float *)&(x) )
#define COERCE_DOUBLE(x)    ( *(double *)&(x) )
#define COERCE_UNSIGNED_INT(x)   ( *(unsigned int *)&(x) )
#define COERCE_UNSIGNED_INT64(x) ( *(unsigned __int64 *)&(x) )
#define __FYL2X__(x, y)     ( (y) * ( log( (double)(x) ) / log( 2.0 ) ) )
#define __FYL2XP1__(x, y)   ( (y) * ( log( 1.0 + (double)(x) ) / log( 2.0 ) ) )
#define abs32(x)            ( (x) < 0 ? -(x) : (x) )
#define is_mul_ok(a, b)     ( 1 )

/* MSVC 6 has no __noreturn. */
#ifndef __noreturn
#define __noreturn
#endif

/*
 * Minimal Win32 vocabulary.
 *
 * Hex-Rays types a great many recovered parameters with Windows handle types
 * because IDA knows the import signatures.
 *
 * These are opaque on purpose.  Nothing here should ever be dereferenced; a
 * unit that genuinely needs the real windows.h should be reconstructed by hand
 * and include it, the way win32/win_imports.c does.  windows.h is deliberately
 * NOT included here: it would collide with the shim's own _BYTE/_WORD/_DWORD
 * and with the undecorated forwarders in win32/win_import_aliases.c.
 */
#ifndef _WINDEF_
typedef void *              HANDLE;
typedef void *              HINSTANCE;
typedef void *              HMODULE;
typedef void *              HWND;
typedef void *              HDC;
typedef void *              HGLRC;
typedef void *              HMENU;
typedef void *              HICON;
typedef void *              HCURSOR;
typedef void *              HBRUSH;
typedef void *              HFONT;
typedef void *              HBITMAP;
typedef void *              HKEY;
typedef void *              HMIDIIN;
typedef void *              HMONITOR;
typedef void *              HGDIOBJ;
typedef void *              HGLOBAL;
typedef void *              LPVOID;
typedef char *              LPSTR;
typedef const char *        LPCSTR;
typedef char *              CHAR;
typedef unsigned int        UINT;
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef long                LONG;
typedef long                LPARAM;
typedef unsigned int        WPARAM;
typedef long                LRESULT;
typedef void *              GUID;
typedef void *              LPWSADATA;
typedef struct { long l, t, r, b; } RECT;
typedef struct { long x, y; } POINT;
typedef struct { unsigned long a[9]; } MODULEENTRY32;
typedef struct { unsigned long a[16]; } MSG;
#endif

/* Hex-Rays emits C++ `bool` for flag-valued temporaries even in C output. */
#ifndef __cplusplus
#define bool int
#define true 1
#define false 0
#endif

/* The retail build is /GS-instrumented, so almost every function with a local
 * buffer opens with `v = retaddr ^ _security_cookie`.  MSVC 6 predates /GS and
 * has no such symbol.  The XOR is dead code here -- nothing ever checks the
 * result. */
extern unsigned int _security_cookie;

#endif /* __HEXRAYS_SHIM_H__ */
