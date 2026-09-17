/*
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include <io.h>
#include <direct.h>

extern long DirectDrawCreateEx( GUID *lpGuid, void **lplpDD, void *iid,
                                void *pUnkOuter );
extern long DirectDrawEnumerateExA( void *lpCallback, void *lpContext,
                                    unsigned long dwFlags );

void __cdecl CPUID( int function, unsigned int *regs );
int  IsPentium( void );

extern void *GetModuleHandleA( const char *lpModuleName );
extern void *CreateWindowExA( unsigned long dwExStyle, const char *lpClassName,
                              const char *lpWindowName, unsigned long dwStyle,
                              int X, int Y, int nWidth, int nHeight,
                              void *hWndParent, void *hMenu, void *hInstance,
                              void *lpParam );
extern int   DestroyWindow( void *hWnd );

/* ---- Sys_FistpRound  no-address ---- */
static int Sys_FistpRound( double x ) {
	int i;

	__asm {
		fld     x
		fistp   i
	}

	return i;
}

/* ---- Sys_Mkdir  0x004604E0 ----  VERIFIED */
int __cdecl Sys_Mkdir(const char *a1)
{
  return _mkdir(a1);
}

/* ---- Sys_Cwd  0x004604F0 ----  VERIFIED */
char *Sys_Cwd()
{
  getcwd(sys_cwd, 255);
  sys_cwdLastByte = 0;
  return sys_cwd;
}

/* ---- Sys_DefaultCDPath  0x00460510 ----  VERIFIED */
char *Sys_DefaultCDPath()
{
  return &empty_string;
}

/* ---- Sys_DefaultBasePath  0x00460520 ----  VERIFIED */
char *Sys_DefaultBasePath()
{
  getcwd(sys_cwd, 255);
  sys_cwdLastByte = 0;
  return sys_cwd;
}

/* ---- Sys_DefaultHomePath  0x00460540 ----  VERIFIED */
char *Sys_DefaultHomePath( void ) {
	return 0;
}

/* ---- Sys_DefaultInstallPath  0x00460550 ----  VERIFIED */
char *Sys_DefaultInstallPath()
{
  getcwd(sys_cwd, 255);
  sys_cwdLastByte = 0;
  return sys_cwd;
}

/* ---- Sys_DirectoryHasContents_m  0x004609C0 ---- */
int Sys_DirectoryHasContents_m( const char *path )
{
	struct _finddata_t findinfo;
	long findhandle;
	char search[256];

	Com_sprintf( search, sizeof( search ), "%s\\*", path );

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		return 0;
	}

	while ( findinfo.attrib & _A_SUBDIR ) {
		if ( Q_stricmpn( ".", findinfo.name, 99999 )
		  && Q_stricmpn( "..", findinfo.name, 99999 )
		  && Q_stricmpn( "CVS", findinfo.name, 99999 ) ) {
			break;
		}
		if ( _findnext( findhandle, &findinfo ) == -1 ) {
			return 0;
		}
	}
	return 1;
}

/* ---- nullsub_19  0x00460AB0 ----  VERIFIED */
void nullsub_19() { ; }
/* ---- nullsub_20  0x00460AC0 ---- */
void nullsub_20() { ; }
/* ---- nullsub_21  0x00460AD0 ---- */
void nullsub_21() { ; }
/* ---- nullsub_22  0x00460AE0 ---- */
void nullsub_22() { ; }

/* ---- FS_FReadCallback_m  0x00460AF0 ----  VERIFIED */
int __cdecl FS_FReadCallback_m(fileHandle_t f, void *buffer, int a3, int a4)
{
  return FS_Read(buffer, a4 * a3, f);
}

/* ---- FS_FSeekCallback_m  0x00460B10 ----  VERIFIED */
int __fastcall FS_FSeekCallback_m(int offset, fileHandle_t f, int origin)
{
  return FS_Seek(f, offset, origin);
}

/* ---- nullsub_23  0x00460B30 ---- */
void nullsub_23() { ; }
/* ---- nullsub_24  0x00460B40 ---- */
void nullsub_24() { ; }

/* ---- RoundFloatToInt_m  0x00460B50 ----  VERIFIED */
int __cdecl RoundFloatToInt_m(float a1)
{
  return Sys_FistpRound( (double)a1 + 0.4999999990686774 );
}

/* ---- Sys_GetDDrawVidMem_m  0x00460E70 ----  VERIFIED */
int __cdecl Sys_GetDDrawVidMem_m(GUID *lpGuid)
{
  int v2;
  LPVOID lpDD;
  int v4;
  _BYTE v5[4];
  _DWORD v6[4];

  if ( DirectDrawCreateEx(lpGuid, &lpDD, &iid, 0) < 0 )
    return 0;
  memset(&v6[1], 0, 12);
  v6[0] = 0x4000;
  v2 = (*(int (__stdcall **)(LPVOID, _DWORD *, int *, _BYTE *))(*(_DWORD *)lpDD + 92))(lpDD, v6, &v4, v5);
  (*(void (__stdcall **)(LPVOID))(*(_DWORD *)lpDD + 8))(lpDD);
  return v2 < 0 ? 0 : v4;
}

/* ---- Sys_DDrawEnumVidMemCallback_m  0x00460EF0 ----  VERIFIED */
BOOL __stdcall Sys_DDrawEnumVidMemCallback_m(GUID *lpGuid, LPSTR a2, LPSTR a3, unsigned int *a4, HMONITOR a5)
{
  unsigned int DDrawVidMem_m;

  if ( !a5 )
  {
    DDrawVidMem_m = Sys_GetDDrawVidMem_m(lpGuid);
    if ( *a4 < DDrawVidMem_m )
      *a4 = DDrawVidMem_m;
  }
  return 1;
}

/* ---- Sys_QueryDDrawVidMem_m  0x00460F20 ----  VERIFIED */
unsigned int __cdecl Sys_QueryDDrawVidMem_m(GUID *lpGuid)
{
  HMODULE ModuleHandleA;
  HWND Window;
  LPVOID v5;
  int v6;
  LPVOID lpDD;
  int v8;
  int v9;
  _DWORD v10[4];

  if ( DirectDrawCreateEx(lpGuid, &lpDD, &iid, 0) < 0 )
    return 0;
  ModuleHandleA = (HMODULE)GetModuleHandleA(0);
  Window = (HWND)CreateWindowExA(0, "static", "dummy", 0, 0, 0, 1, 1, 0, 0, ModuleHandleA, 0);
  v5 = lpDD;
  if ( !Window )
    goto LABEL_6;
  if ( (*(int (__stdcall **)(LPVOID, HWND, int))(*(_DWORD *)lpDD + 80))(lpDD, Window, 17) < 0 )
  {
    DestroyWindow(Window);
    v5 = lpDD;
LABEL_6:
    (*(void (__stdcall **)(LPVOID))(*(_DWORD *)v5 + 8))(v5);
    return 0;
  }
  memset(&v10[1], 0, 12);
  v10[0] = 0x4000;
  v6 = (*(int (__stdcall **)(LPVOID, _DWORD *, int *, int *))(*(_DWORD *)lpDD + 92))(lpDD, v10, &v8, &v9);
  DestroyWindow(Window);
  (*(void (__stdcall **)(LPVOID))(*(_DWORD *)lpDD + 8))(lpDD);
  return v6 < 0 ? 0 : v8;
}

/* ---- Sys_DDrawEnumQueryCallback_m  0x00461000 ----  VERIFIED */
BOOL __stdcall Sys_DDrawEnumQueryCallback_m(GUID *lpGuid, LPSTR a2, LPSTR a3, unsigned int *a4, HMONITOR a5)
{
  unsigned int DDrawVidMem_m;

  DDrawVidMem_m = Sys_QueryDDrawVidMem_m(lpGuid);
  if ( *a4 < DDrawVidMem_m )
    *a4 = DDrawVidMem_m;
  return 1;
}

typedef struct {
	const char *renderer;
	int         megs;
} sys_vidMemEntry_t;

static const sys_vidMemEntry_t sys_vidMemTable[] = {
	{ "Radeon 7200",              64 },
	{ "Radeon 7500",              64 },
	{ "Radeon 8500",              64 },
	{ "Radeon 9000",              64 },
	{ "Radeon 9100",              64 },
	{ "Radeon 9200",              64 },
	{ "Radeon 9500",              64 },
	{ "Radeon 9600",             128 },
	{ "Radeon 9700",             128 },
	{ "Radeon 9800",             128 },
	{ "Radeon",                   32 },
	{ "GeForce 256",              32 },
	{ "GeForce2 GTS",             32 },
	{ "GeForce2 MX",              32 },
	{ "GeForce2",                 32 },
	{ "GeForce3",                 64 },
	{ "GeForce4 420 Go",          64 },
	{ "GeForce4 4200 Go",         64 },
	{ "GeForce4 440 Go",          64 },
	{ "GeForce4 460 Go",          64 },
	{ "GeForce4 MX 420",          64 },
	{ "GeForce4 MX 440",          64 },
	{ "GeForce4 MX 460",          64 },
	{ "GeForce4 MX",              64 },
	{ "GeForce4 Ti 4200",         64 },
	{ "GeForce4 Ti 4400",        128 },
	{ "GeForce4 Ti 4600",        128 },
	{ "GeForce4 Ti 4800",        128 },
	{ "GeForce4",                 64 },
	{ "GeForce FX 5200",          64 },
	{ "GeForce FX 5600",         128 },
	{ "GeForce FX 5800 Ultra",   128 },
	{ "GeForce FX 5800",         128 },
	{ "GeForce FX 5900 Ultra",   128 },
	{ "GeForce FX 5900",         128 },
	{ "Quadro FX 1000",          128 },
	{ "Quadro FX",               128 },
	{ "Quadro4 500",              64 },
	{ "Quadro4 700",              64 },
	{ "Quadro4 900",              64 },
	{ "Quadro4",                  64 },
	{ "Quadro2 Pro",              32 },
	{ "Quadro2 MXR",              32 },
	{ "Quadro2",                  32 },
	{ "Quadro DCC",               32 },
	{ "Quadro",                   32 },
	{ "Matrox ICD for Parhelia", 128 }
};

typedef char sys_vidMemTable_size_check
	[ ( sizeof( sys_vidMemTable ) / sizeof( sys_vidMemTable[0] ) ) == 47 ? 1 : -1 ];

/* ---- Sys_VideoRamFromRenderer  0x00461030 ----  VERIFIED */
int __cdecl Sys_VideoRamFromRenderer( const char *renderer )
{
	int i;

	if ( !renderer ) {
		return 0;
	}

	for ( i = 0; i < 47; i++ ) {
		if ( !_strnicmp( sys_vidMemTable[i].renderer, renderer,
		                 strlen( sys_vidMemTable[i].renderer ) ) ) {
			return sys_vidMemTable[i].megs << 20;
		}
	}

	return 0;
}

/* ---- Sys_DetectVideoRamMegs_m  0x004614F0 ----  VERIFIED */
unsigned int Sys_DetectVideoRamMegs_m( void )
{
	unsigned int bytes;
	unsigned int megs;
	unsigned int pow2;

	bytes = Sys_GetDDrawVidMem_m( 0 );

	if ( !bytes ) {
		DirectDrawEnumerateExA( (void *) Sys_DDrawEnumVidMemCallback_m, &bytes, 0 );
	}
	if ( !bytes ) {
		bytes = Sys_QueryDDrawVidMem_m( NULL );
	}
	if ( !bytes ) {
		DirectDrawEnumerateExA( (void *) Sys_DDrawEnumQueryCallback_m, &bytes, 0 );
	}
	if ( !bytes ) {
		bytes = Sys_GetVideoRam();
	}
	if ( !bytes ) {
		return 0;
	}

	megs = ( ( bytes - 1 ) >> 20 ) + 1;

	for ( pow2 = 1; pow2 < megs; pow2 *= 2 ) {
		;
	}
	if ( pow2 - megs > 0x20 ) {
		return pow2 >> 1;
	}
	return pow2;
}

/* ---- Sys_HasSSE_m  0x00461590 ----  VERIFIED */
int Sys_HasSSE_m( void )
{
	unsigned int regs[4];

	CPUID( 1, regs );
	return ( regs[3] & 0x02000000u ) != 0;
}

/* ---- fastftol  0x00465A00 ----  VERIFIED */
int __cdecl fastftol(float a1)
{
  tmp = Sys_FistpRound( (double)a1 );
  return tmp;
}

/* ---- Sys_SnapVector  0x00465A10 ----  VERIFIED */
float *__cdecl Sys_SnapVector(float *v)
{
	v[0] = (float) Sys_FistpRound( (double) v[0] );
	v[1] = (float) Sys_FistpRound( (double) v[1] );
	v[2] = (float) Sys_FistpRound( (double) v[2] );
	return &v[2];
}

/* ---- CPUID  0x00465A60 ----  VERIFIED */
void __cdecl CPUID( int function, unsigned int *regs )
{
	unsigned int ra, rb, rc, rd;

	__asm {
		push ebx
		mov  eax, function
		_emit 0x0F
		_emit 0xA2
		mov  ra, eax
		mov  rc, ecx
		mov  rd, edx
		mov  eax, ebx
		pop  ebx
		mov  rb, eax
	}

	regs[0] = ra;
	regs[1] = rb;
	regs[2] = rc;
	regs[3] = rd;
}

/* ---- IsPentium  0x00465AB0 ----  VERIFIED */
int IsPentium( void )
{
	int hasCPUID;

	__asm {
		pushfd
		pop  eax
		mov  ecx, eax
		xor  eax, 200000h
		push eax
		popfd
		pushfd
		pop  eax
		xor  eax, ecx
		shr  eax, 21
		and  eax, 1
		mov  hasCPUID, eax
		push ecx
		popfd
	}

	return hasCPUID;
}

/* ---- Is3DNOW  0x00465AF0 ----  VERIFIED */
BOOL Is3DNOW( void )
{
	unsigned int vendor[4];
	unsigned int regs[4];

	CPUID( 0, vendor );

	CPUID( 0x80000000, regs );
	if ( regs[0] < 0x80000000u ) {
		return 0;
	}

	CPUID( 0x80000001, regs );
	return ( regs[3] & 0x80000000u ) != 0;
}

/* ---- IsKNI  0x00465B70 ----  VERIFIED */
BOOL IsKNI( void )
{
	unsigned int regs[4];

	CPUID( 1, regs );
	return ( regs[3] & 0x02000000u ) != 0;
}

/* ---- IsMMX  0x00465BA0 ----  VERIFIED */
BOOL IsMMX( void )
{
	unsigned int regs[4];

	CPUID( 1, regs );
	return ( regs[3] & 0x00800000u ) != 0;
}

/* ---- Sys_GetProcessorId  0x00465BD0 ----  VERIFIED */
int Sys_GetProcessorId( void )
{
	if ( !IsPentium() ) {
		return 32;
	}
	if ( !IsMMX() ) {
		return 33;
	}
	if ( Is3DNOW() ) {
		return 48;
	}
	if ( IsKNI() ) {
		return 35;
	}
	return 34;
}

/* Sys_GetCurrentUser (0x00465C20) is defined in win_platform.c. */
