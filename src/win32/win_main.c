/*
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

extern void         NET_Config( qboolean enableNetworking );
extern unsigned int Sys_DetectVideoRamMegs_m( void );
extern int          Sys_HasSSE_m( void );

extern void *CreateFileA( const char *name, unsigned access, unsigned share,
                          void *sec, unsigned disp, unsigned flags, void *tmpl );
extern int   ReadFile( void *h, void *buf, unsigned n, unsigned *got, void *ov );
extern int   WriteFile( void *h, const void *buf, unsigned n, unsigned *put, void *ov );
extern int   CloseHandle( void *h );
extern int   DeleteFileA( const char *p );
extern int   GetDriveTypeA( const char *root );
extern int   GetModuleFileNameA( void *m, char *buf, unsigned n );
extern int   FreeLibrary( void *m );
extern int   IsBadReadPtr( const void *p, unsigned n );

extern qboolean SEH_GetLanguageIndexForName( const char *name, int *languageOut );

/* Com_Parse (universal/q_parse.c, 0x00449F80) comes from qcommon.h. */

#define SEH_LOCALIZATION_SIZE   4096
static char seh_localizationBuffer[SEH_LOCALIZATION_SIZE];

/* ---- SEH_LoadLocalizationFile_m  0x00462350 ----  VERIFIED */
int SEH_LoadLocalizationFile_m( void )
{
	FILE *f;
	long  len;
	size_t got;
	char *nl;
	int   language = 0;

	seh_localizationBase = 0;
	seh_localizationTable = 0;

	f = fopen( "localization.txt", "r" );
	if ( !f ) {
		return 0;
	}

	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, 0, SEEK_SET );

	if ( len < 0 ) {
		len = 0;
	}
	if ( len > SEH_LOCALIZATION_SIZE - 1 ) {
		len = SEH_LOCALIZATION_SIZE - 1;
	}

	got = fread( seh_localizationBuffer, 1, (size_t) len, f );
	fclose( f );

	if ( !got ) {
		return 0;
	}

	seh_localizationBuffer[got] = 0;
	seh_localizationBase = (int) seh_localizationBuffer;

	if ( !seh_localizationBuffer[0] ) {
		return 0;
	}

	nl = strchr( seh_localizationBuffer, '\n' );
	if ( !nl ) {
		return 0;
	}
	*nl = 0;
	seh_localizationTable = (int) ( nl + 1 );

	SEH_GetLanguageIndexForName( seh_localizationBuffer, &language );
	return language;
}

/* ---- SEH_ShutdownLocalization_m  0x00462430 ----  VERIFIED */
int SEH_ShutdownLocalization_m()
{
  int result;

  result = 0;
  seh_localizationBase = 0;
  seh_localizationTable = 0;
  return result;
}

/* ---- SEH_LocalizeIdentity_m  0x00462440 ----  VERIFIED */
char *__cdecl SEH_LocalizeIdentity_m(const char *a1)
{
  return va("%s", a1);
}

/* ---- SEH_GetLocalizedString_m  0x00462450 ----  VERIFIED */
char *__cdecl SEH_GetLocalizedString_m( const char *key )
{
	char *data_p;
	char *token;
	int   matched;

	if ( !key ) {
		key = "";
	}
	if ( !seh_localizationTable ) {
		return va( "%s", key );
	}

	Com_BeginParseSession( "localization" );
	data_p = (char *) seh_localizationTable;

	while ( 1 ) {
		token = Com_Parse( &data_p );
		if ( !*token ) {
			break;
		}
		matched = ( strcmp( token, key ) == 0 );

		token = Com_Parse( &data_p );
		if ( !*token ) {
			break;
		}
		if ( matched ) {
			Com_EndParseSession();
			return va( "%s", token );
		}
	}

	Com_EndParseSession();
	return va( "%s", key );
}

/* ---- Sys_GetInfo  0x004625A0 ----  VERIFIED */
void __cdecl Sys_GetInfo( int *out )
{
	if ( !out ) {
		return;
	}
	*(double *) out = sys_cpuMHzValue;
	out[2] = sys_sysMBValue;
	out[3] = sys_vidMBValue;
}

/* ---- Sys_HeadlessNotice  no-address ---- */
static void Sys_HeadlessNotice( const char *titleKey, const char *bodyKey )
{
	fprintf( stderr, "\n---- %s ----\n%s\n\n",
	         SEH_GetLocalizedString_m( titleKey ),
	         SEH_GetLocalizedString_m( bodyKey ) );
	fflush( stderr );
}

/* ---- Sys_UpdateForConfigChange  0x004625D0 ----  VERIFIED */
BOOL Sys_UpdateForConfigChange( void )
{
	Sys_HeadlessNotice( "WIN_CONFIGURE_UPDATED_TITLE", "WIN_CONFIGURE_UPDATED_BODY" );
	return 0;
}

/* ---- Sys_ConfigureChecksumChanged  0x00462600 ----  VERIFIED */
BOOL __cdecl Sys_ConfigureChecksumChanged( int checksum )
{
	cvar_t *sum;
	int     archived;

	sum = Cvar_Get( "sys_configSum", "0", CVAR_ARCHIVE | CVAR_ROM );
	archived = sum->integer;

	if ( archived && archived != checksum ) {
		Sys_HeadlessNotice( "WIN_CONFIGURE_UPDATED_TITLE", "WIN_CONFIGURE_UPDATED_BODY" );
	}

	if ( !archived || archived != checksum ) {
		Cvar_Set2( "sys_configSum", va( "%i", checksum ), qtrue );
	}

	return 0;
}

/* ---- Sys_RegisterInfoCvars  0x00462690 ----  VERIFIED */
cvar_t *Sys_RegisterInfoCvars( void )
{
	cvar_t *result;

	sys_cpuMHz = (int) Cvar_Get( "sys_cpuMHz", "0", CVAR_ARCHIVE | CVAR_ROM );
	sys_sysMB  = (int) Cvar_Get( "sys_sysMB",  "0", CVAR_ARCHIVE | CVAR_ROM );
	result     = Cvar_Get( "sys_vidMB", "0", CVAR_ARCHIVE | CVAR_ROM );
	sys_vidMB  = (int) result;
	return result;
}

/* ---- Sys_DetectHardware_m  0x004626E0 ----  VERIFIED */
int Sys_DetectHardware_m( void )
{
	sys_cpuMHzValue   = (double) Sys_GetCPUSpeed();
	sys_sysMBValue = Sys_GetSystemRam();
	sys_vidMBValue = Sys_DetectVideoRamMegs_m();

	sys_hasSSE = Sys_HasSSE_m();
	return sys_hasSSE;
}

/* ---- Sys_IsLowMemory_m  0x00462920 ----  VERIFIED */
BOOL Sys_IsLowMemory_m()
{
  return sys_sysMBValue <= 100663296;
}

/* ---- Sys_WriteCrashMarker_m  0x00462940 ----  VERIFIED */
int Sys_WriteCrashMarker_m( void )
{
	void *h;

	h = CreateFileA( "crash", 0x40000000u, 0, 0, 1u, 2u, 0 );
	if ( h == (void *) -1 ) {
		return 0;
	}
	CloseHandle( h );
	return 1;
}

/* ---- cand_Sys_VerifyFunctionIntegrity  0x00462970 ----  VERIFIED */
int __fastcall cand_Sys_VerifyFunctionIntegrity(int a1, int a2)
{
  int v2;
  int v3;
  int i;
  int v5;
  char v6;
  int v7;
  int v8;
  char v9;
  char v11;
  _BYTE v12[31];

  memset(v12, 144, 2);
  v11 = -61;
  memset(&v12[2], 0, 29);
  if ( *(_BYTE *)a2 == 0xE9 )
    v2 = *(_DWORD *)(a2 + 1) + a2 + 5;
  else
    v2 = a2;
  if ( *(_BYTE *)a1 == 0xE9 )
    v3 = *(_DWORD *)(a1 + 1) + a1 + 5;
  else
    v3 = a1;
  for ( i = 0; i < 1024; ++i )
  {
    v5 = 0;
    v6 = v11;
    do
    {
      if ( *(_BYTE *)(i + v2 + v5) != v6 )
        break;
      v6 = v12[v5++];
    }
    while ( v6 );
    if ( !v12[v5 - 1] )
      break;
  }
  v7 = i + 2;
  v8 = 0;
  if ( v7 <= 0 )
    return 1;
  while ( 1 )
  {
    v9 = *(_BYTE *)(v8 + v2);
    if ( v9 != -24 || v8 + *(_DWORD *)(v8 + v2 + 1) + v2 + 5 != v8 + *(_DWORD *)(v8 + v3 + 1) + v3 + 5 )
      break;
    v8 += 4;
LABEL_19:
    if ( ++v8 >= v7 )
      return 1;
  }
  if ( v9 == *(_BYTE *)(v8 + v3) )
    goto LABEL_19;
  return 0;
}

/* ---- cand_Sys_ScanForInlineHook  0x00462A70 ----  VERIFIED */
unsigned int __fastcall cand_Sys_ScanForInlineHook(int a1, _BYTE *a2)
{
  _BYTE *v2;
  int i;
  int v4;
  char v5;
  char v7;
  _BYTE v8[31];

  memset(v8, 144, 2);
  v7 = -61;
  memset(&v8[2], 0, 29);
  if ( *a2 == 0xE9 )
    v2 = &a2[*(_DWORD *)(a2 + 1) + 5];
  else
    v2 = a2;
  for ( i = 0; i < 1024; ++i )
  {
    v4 = 0;
    v5 = v7;
    do
    {
      if ( v2[i + v4] != v5 )
        break;
      v5 = v8[v4++];
    }
    while ( v5 );
    if ( !v8[v4 - 1] )
      break;
  }
  return Com_BlockChecksum(v2, i + 2);
}

/* ---- cand_Sys_GetIntegrityState  0x00462B10 ----  VERIFIED */
int cand_Sys_GetIntegrityState()
{
  return dword_8E3C48;
}

extern void     CL_ShutdownRef( void );
extern unsigned __stdcall timeEndPeriod( unsigned uPeriod );

/* ---- Sys_OutOfMemoryPrep  0x00462B20 ----  VERIFIED */
void Sys_OutOfMemoryPrep( void )
{
	/* retail LTCG-inlines Sys_DestroySplashWindow (0x00465E10) and
	   CL_ShutdownRef (0x00411500) here; the eax left behind by the
	   inlined re.Shutdown test is not a return value */
	Sys_DestroySplashWindow();
	timeEndPeriod( 1 );
	IN_Shutdown();
	CL_ShutdownCGame();
	CL_ShutdownUI();
	CL_ShutdownRef();
}

/* ---- Sys_OutOfMemoryError  0x00462BA0 ----  VERIFIED */
void Sys_OutOfMemoryError( void )
{
	Sys_OutOfMemoryPrep();
	Sys_HeadlessNotice( "WIN_OUT_OF_MEM_TITLE", "WIN_OUT_OF_MEM_BODY" );
	exit( -1 );
}

/* ---- Sys_UnableToLoadDllError_m  0x00462BE0 ----  VERIFIED */
void  Sys_UnableToLoadDllError_m()
{
  char *LocalizedString_m;

  LocalizedString_m = SEH_GetLocalizedString_m("WIN_UNABLE_LOAD_DLL_BODY");
  Com_Error(ERR_FATAL, "%s\n", LocalizedString_m);
}

/* ---- sub_462BF7  0x00462BF7 ----  VERIFIED */
void sub_462BF7()
{
  ;
}

/* ---- Sys_ChecksumModuleImage_m  0x00462C00 ----  VERIFIED */
unsigned int __cdecl Sys_ChecksumModuleImage_m(int a1)
{
  _DWORD *v1;

  v1 = (_DWORD *)(a1 + *(_DWORD *)(a1 + 60));
  if ( IsBadReadPtr(v1, 0xF8u) || *v1 != 17744 )
    return 0;
  else
    return Com_BlockChecksum((void *)(a1 + v1[65]), v1[66]);
}

/* ---- Sys_MonkeyCheck_m  0x00462C40 ----  VERIFIED */
int Sys_MonkeyCheck_m() { return 0; }
#if 0
// Sys_MonkeyCheck_m -- MEDIUM 60. Formats "%s monkey %d %d \"%s\"" and reads a file in binary; 'monkey' is CoD's own name for this integrity/anti-cheat handshake.
FILE *Sys_MonkeyCheck_m()
{
  int v0;
  int v1;
  HANDLE FileA;
  DWORD CurrentProcessId;
  HANDLE v4;
  void *v5;
  FILE *result;
  FILE *v7;
  signed int v8;
  int *v9;
  signed int v10;
  char v11;
  int v12;
  char v13;
  FILE *v14;
  const char *v15;
  const char *v16;
  const char *v17;
  const char *v18;
  int v19;
  int v20;
  struct _PROCESS_INFORMATION ProcessInformation; // [esp+10h] [ebp-460h] BYREF
  struct _STARTUPINFOA StartupInfo; // [esp+20h] [ebp-450h] BYREF
  CHAR Buffer[260]; // [esp+64h] [ebp-40Ch] BYREF
  CHAR Filename[260]; // [esp+168h] [ebp-308h] BYREF
  CHAR CommandLine[512]; // [esp+26Ch] [ebp-204h] BYREF
  unsigned int v26;
  unsigned int retaddr;

  v0 = dword_57C498;
  v1 = *(_DWORD *)(dword_57C498 + 4);
  v26 = retaddr ^ _security_cookie;
  if ( !v1 || Q_stricmpn(v15, v17, v19) )
  {
    GetModuleFileNameA(0, Filename, 0x104u);
    GetTempPathA(0x104u, Buffer);
    GetTempFileNameA(Buffer, PrefixString, 0, Buffer);
    CopyFileA(Filename, Buffer, 0);
    FileA = CreateFileA(Buffer, 0, 1u, 0, 3u, 0x4000000u, 0);
    CurrentProcessId = GetCurrentProcessId();
    v4 = OpenProcess(0x100000u, 1, CurrentProcessId);
    wsprintfA(CommandLine, "%s monkey %d %d \"%s\"", Buffer, dword_8E3708, v4, Filename);
    memset(&StartupInfo, 0, sizeof(StartupInfo));
    StartupInfo.cb = 68;
    CreateProcessA(0, CommandLine, 0, 0, 1, 0, 0, 0, &StartupInfo, &ProcessInformation);
    CloseHandle(v4);
    CloseHandle(FileA);
    goto LABEL_14;
  }
  dword_8E3708 = j__atol(*(const char **)(v0 + 8));
  v5 = (void *)j__atol(*(const char **)(dword_57C498 + 12));
  WaitForSingleObject(v5, 0xFFFFFFFF);
  CloseHandle(v5);
  result = fopen(*(const char **)(dword_57C498 + 16), "rb");
  v7 = result;
  if ( result )
  {
    fseek(result, 0, 2);
    v8 = ftell(v7);
    fseek(v7, 0, 0);
    v9 = Z_MallocInternal(v8);
    result = (FILE *)fread(v9, v8, 1u, v7);
    if ( result == (FILE *)1 )
    {
      fclose(v7);
      result = (FILE *)strlen(off_575978);
      v10 = 0;
      if ( v8 > 0 )
      {
        while ( 1 )
        {
          result = (FILE *)Q_strncmp(v16, v18, v20);
          if ( !result )
            break;
          if ( ++v10 >= v8 )
            return result;
        }
        if ( v10 < v8 )
        {
          v11 = BYTE2(dword_8E3708);
          v12 = dword_57C498;
          *((_BYTE *)v9 + v10) = HIBYTE(dword_8E3708);
          *((_BYTE *)v9 + v10 + 1) = v11;
          v13 = dword_8E3708;
          *((_BYTE *)v9 + v10 + 2) = BYTE1(dword_8E3708);
          *((_BYTE *)v9 + v10 + 3) = v13;
          *((_BYTE *)v9 + v10 + 9) = 0;
          *((_BYTE *)v9 + v10 + 8) = 0;
          *((_BYTE *)v9 + v10 + 7) = 0;
          *((_BYTE *)v9 + v10 + 6) = 0;
          *((_BYTE *)v9 + v10 + 5) = 0;
          *((_BYTE *)v9 + v10 + 4) = 0;
          result = fopen(*(const char **)(v12 + 16), "wb");
          v14 = result;
          if ( result )
          {
            result = (FILE *)fwrite(v9, v8, 1u, result);
            if ( result == (FILE *)1 )
            {
              fclose(v14);
              free(v9);
LABEL_14:
              exit(0);
            }
          }
        }
      }
    }
  }
  return result;
}
#endif

extern unsigned __stdcall GetCurrentDirectoryA( unsigned nBufferLength, char *lpBuffer );
extern unsigned __stdcall GetLastError( void );
extern unsigned __stdcall FormatMessageA( unsigned dwFlags, const void *lpSource,
                                          unsigned dwMessageId, unsigned dwLanguageId,
                                          char *lpBuffer, unsigned nSize, void *Arguments );
extern int      __stdcall CreateProcessA( const char *lpApplicationName, char *lpCommandLine,
                                          void *lpProcessAttributes, void *lpThreadAttributes,
                                          int bInheritHandles, unsigned dwCreationFlags,
                                          void *lpEnvironment, const char *lpCurrentDirectory,
                                          void *lpStartupInfo, void *lpProcessInformation );

typedef struct {
	unsigned  cb;
	char     *lpReserved;
	char     *lpDesktop;
	char     *lpTitle;
	unsigned  dwX;
	unsigned  dwY;
	unsigned  dwXSize;
	unsigned  dwYSize;
	unsigned  dwXCountChars;
	unsigned  dwYCountChars;
	unsigned  dwFillAttribute;
	unsigned  dwFlags;
	unsigned short wShowWindow;
	unsigned short cbReserved2;
	unsigned char *lpReserved2;
	void     *hStdInput;
	void     *hStdOutput;
	void     *hStdError;
} winStartupInfo_t;

typedef struct {
	void     *hProcess;
	void     *hThread;
	unsigned  dwProcessId;
	unsigned  dwThreadId;
} winProcessInfo_t;

/* ---- Sys_StartProcess  0x00462EF0 ----  VERIFIED */
void Sys_StartProcess( const char *cmd, int doexit )
{
	winProcessInfo_t ProcessInformation;
	winStartupInfo_t StartupInfo;
	char            *msgbuf;
	char             Buffer[260];
	unsigned         lastError;

	memset( &StartupInfo, 0, sizeof( StartupInfo ) );
	StartupInfo.cb = sizeof( StartupInfo );

	GetCurrentDirectoryA( 0x104, Buffer );

	if ( !CreateProcessA( NULL, va( "\"%s\\%s\"", Buffer, cmd ),
	                      NULL, NULL, 0, 0, NULL, NULL,
	                      &StartupInfo, &ProcessInformation ) )
	{
		lastError = GetLastError();
		FormatMessageA( 0x1300, NULL, lastError, 0x400, (char *) &msgbuf, 0, NULL );
		Com_Error( ERR_DROP, va( "EXE_ERR_COULDNT_START_PROCESS\x15" "'%s\\%s'\n%s\n%08x",
		                         Buffer, cmd, msgbuf, lastError ) );
	}

	if ( doexit ) {
		Cbuf_AddText( "quit\n" );
	}
}

extern void * __stdcall ShellExecuteA( void *hwnd, const char *op, const char *file,
                                       const char *params, const char *dir, int nShowCmd );
extern void * __stdcall GetForegroundWindow( void );
extern int    __stdcall ShowWindow( void *hWnd, int nCmdShow );

#define SW_MAXIMIZE_    3
#define SW_RESTORE_     9

/* ---- Sys_OpenURL  0x00462FE0 ----  VERIFIED */
void Sys_OpenURL( const char *url, int doexit )
{
	void *wnd;

	if ( !ShellExecuteA( NULL, "open", url, NULL, NULL, SW_RESTORE_ ) ) {
		Com_Error( ERR_DROP, va( "EXE_ERR_COULDNT_OPEN_URL\x15" "%s", url ) );
	}

	wnd = GetForegroundWindow();
	if ( wnd ) {
		ShowWindow( wnd, SW_MAXIMIZE_ );
	}

	if ( doexit ) {
		Cbuf_AddText( "quit\n" );
	}
}

/* ---- Sys_BeginProfiling  0x00463040 ----  VERIFIED */
void Sys_BeginProfiling()
{
  ;
}

/* ---- Sys_Print  0x004631B0 ----  VERIFIED */
void __cdecl Sys_Print( const char *msg )
{
	Conbuf_AppendText( msg );
}

/* ---- Sys_RunSetup_m  0x004631C0 ----  VERIFIED */
int Sys_RunSetup_m( void )
{
	FILE *f;
	char  rootPath[4];
	char  path[256];
	int   drive;

	for ( drive = 'c'; drive <= 'z'; drive++ ) {
		rootPath[0] = (char) drive;
		rootPath[1] = ':';
		rootPath[2] = '\\';
		rootPath[3] = 0;

		if ( GetDriveTypeA( rootPath ) != 5 ) {
			continue;
		}

		Com_sprintf( (char *) sys_cdRootPath, sizeof( sys_cdRootPath ), "%s%s", rootPath, "" );
		Com_sprintf( path, sizeof( path ), "%s\\%s", (char *) sys_cdRootPath, "setup\\setup.exe" );
		f = fopen( path, "r" );
		if ( f ) {
			fclose( f );
			return 1;
		}

		Com_sprintf( (char *) sys_cdRootPath, sizeof( sys_cdRootPath ), "%s%s", rootPath, "bin\\x86\\glibc-2.1" );
		Com_sprintf( path, sizeof( path ), "%s\\%s", (char *) sys_cdRootPath, "setup\\setup" );
		f = fopen( path, "r" );
		if ( f ) {
			fclose( f );
			return 1;
		}
	}

	return 0;
}

/* ---- Sys_CheckCD  0x004632F0 ----  VERIFIED */
int Sys_CheckCD()
{
  return 1;
}

extern int      __stdcall OpenClipboard( void *hWndNewOwner );
extern void *   __stdcall GetClipboardData( unsigned uFormat );
extern int      __stdcall CloseClipboard( void );
extern void *   __stdcall GlobalLock( void *hMem );
extern unsigned __stdcall GlobalSize( void *hMem );
extern int      __stdcall GlobalUnlock( void *hMem );

/* ---- Sys_GetClipboardData  0x00463300 ----  VERIFIED */
char *Sys_GetClipboardData( void )
{
	char   *data = NULL;
	char   *cliptext;
	void   *hClipboardData;

	if ( OpenClipboard( NULL ) != 0 ) {
		if ( ( hClipboardData = GetClipboardData( 1  ) ) != 0 ) {
			if ( ( cliptext = (char *) GlobalLock( hClipboardData ) ) != 0 ) {
				unsigned size;

				data = (char *) Z_MallocInternal( GlobalSize( hClipboardData ) + 1 );
				size = GlobalSize( hClipboardData );
				strncpy( data, cliptext, size - 1 );
				data[ size - 1 ] = 0;
				GlobalUnlock( hClipboardData );

				strtok( data, "\n\r\b" );
			}
		}
		CloseClipboard();
	}
	return data;
}

/* ---- Sys_UnloadDll  0x00463380 ----  VERIFIED */
HMODULE __cdecl Sys_UnloadDll(HMODULE dllHandle)
{
  if ( dllHandle )
  {
    dllHandle = (HMODULE)FreeLibrary(dllHandle);
    if ( !dllHandle )
      Com_Error(ERR_FATAL, "\x15Sys_UnloadDll FreeLibrary failed");
  }
  return dllHandle;
}

/* Sys_LoadDll (0x004633A0) is defined in win_platform.c. */

/* Sys_QueEvent (0x00463580) is defined in win_platform.c. */

typedef struct {
	void     *hwnd;
	unsigned  message;
	unsigned  wParam;
	unsigned  lParam;
	unsigned  time;
	long      pt_x;
	long      pt_y;
} winMsg_t;

extern int      __stdcall PeekMessageA( winMsg_t *lpMsg, void *hWnd, unsigned min,
                                        unsigned max, unsigned wRemoveMsg );
extern int      __stdcall GetMessageA( winMsg_t *lpMsg, void *hWnd, unsigned min,
                                       unsigned max );
extern int      __stdcall TranslateMessage( const winMsg_t *lpMsg );
extern long     __stdcall DispatchMessageA( const winMsg_t *lpMsg );
extern unsigned __stdcall SetThreadExecutionState( unsigned esFlags );

/* ---- Sys_LoadingKeepAlive  0x00463640 ----  VERIFIED */
unsigned Sys_LoadingKeepAlive( void )
{
	winMsg_t msg;

	if ( PeekMessageA( &msg, NULL, 0x218, 0x218, 0  ) ) {
		if ( GetMessageA( &msg, NULL, 0x218, 0x218 ) > 0 ) {
			TranslateMessage( &msg );
			DispatchMessageA( &msg );
		}
	}
	return SetThreadExecutionState( 2  );
}

/* ---- Sys_In_Restart_f  0x004638A0 ----  VERIFIED */
void Sys_In_Restart_f( void )
{
	IN_Shutdown();
	IN_Init();
}

/* ---- Sys_Net_Restart_f  0x004638B0 ----  VERIFIED */
void Sys_Net_Restart_f(void)
{
  NET_Config(networkingEnabled);
}

/* Sys_Init (0x004638C0) is defined in win_platform.c. */

extern unsigned __stdcall GetCurrentProcessId( void );
extern void *   __stdcall OpenProcess( unsigned access, int inherit, unsigned pid );
extern void *   __stdcall CreateToolhelp32Snapshot( unsigned flags, unsigned pid );
extern int      __stdcall Module32First( void *snapshot, void *me );
extern int      __stdcall Module32Next( void *snapshot, void *me );

typedef struct sys_moduleEntry32_s {
	unsigned long   dwSize;
	unsigned long   th32ModuleID;
	unsigned long   th32ProcessID;
	unsigned long   GlblcntUsage;
	unsigned long   ProccntUsage;
	unsigned char  *modBaseAddr;
	unsigned long   modBaseSize;
	void           *hModule;
	char            szModule[256];
	char            szExePath[260];
} sys_moduleEntry32_t;

typedef char sys_moduleEntry32_size_check[ sizeof( sys_moduleEntry32_t ) == 548 ? 1 : -1 ];

/* ---- Sys_FindModuleInProcess_m  0x00463DC0 ----  VERIFIED */
int __cdecl Sys_FindModuleInProcess_m( unsigned int pid )
{
	sys_moduleEntry32_t me;
	void *process;
	void *snapshot;
	char  ourPath[260];
	const char *ourName;
	char *p;
	unsigned int n;
	int   found = 0;

	process = OpenProcess( 0x1F0FFFu, 0, pid );
	if ( !process ) {
		return 0;
	}
	CloseHandle( process );

	snapshot = CreateToolhelp32Snapshot( 8u, pid );
	if ( snapshot == (void *) -1 ) {
		return 0;
	}

	n = (unsigned int) GetModuleFileNameA( 0, ourPath, sizeof( ourPath ) );
	if ( n == 0 || n >= sizeof( ourPath ) ) {
		CloseHandle( snapshot );
		return 0;
	}
	ourPath[sizeof( ourPath ) - 1] = 0;

	ourName = ourPath;
	for ( p = ourPath; *p; p++ ) {
		if ( *p == '\\' || *p == '/' || *p == ':' ) {
			ourName = p + 1;
		}
	}

	me.dwSize = sizeof( me );
	if ( Module32First( snapshot, &me ) ) {
		do {
			if ( !Q_stricmp( me.szModule, ourName ) ) {
				found = 1;
				break;
			}
			me.dwSize = sizeof( me );
		} while ( Module32Next( snapshot, &me ) );
	}

	CloseHandle( snapshot );
	return found;
}

/* ---- Sys_BuildInstanceMutexName_m  0x00463ED0 ----  VERIFIED */
int Sys_BuildInstanceMutexName_m( void )
{
	char path[260];
	const char *base;
	char *dot;
	unsigned int n;

	FileName[0] = 0;

	n = (unsigned int) GetModuleFileNameA( 0, path, (unsigned) sizeof( path ) );
	if ( n == 0 || n >= sizeof( path ) ) {
		strcpy( (char *)FileName, "__cod1mp" );
		return (int) strlen( (char *)FileName );
	}
	path[sizeof( path ) - 1] = 0;

	base = path;
	for ( dot = path; *dot; dot++ ) {
		if ( *dot == '\\' || *dot == '/' || *dot == ':' ) {
			base = dot + 1;
		}
	}

	_snprintf( (char *)FileName, sizeof( FileName ), "__%s", base );
	FileName[sizeof( FileName ) - 1] = 0;
	dot = strchr( (char *)FileName, '.' );
	if ( dot ) {
		*dot = 0;
	}
	return (int) strlen( (char *)FileName );
}

/* ---- Sys_CheckCrashOrRerun  0x00463F60 ----  VERIFIED */
int Sys_CheckCrashOrRerun( void )
{
	void *h;
	unsigned int pid;
	unsigned int marker;
	unsigned int transferred;

	if ( !FileName[0] ) {
		return 1;
	}

	pid = GetCurrentProcessId();

	h = CreateFileA( (const char *) FileName, 0x80000000u, 0, 0, 3u, 2u, 0 );
	if ( h != (void *) -1 ) {
		marker = 0;
		transferred = 0;
		if ( ReadFile( h, &marker, 4u, &transferred, 0 ) && transferred == 4 ) {
			CloseHandle( h );

			if ( pid != marker && Sys_FindModuleInProcess_m( marker ) ) {
				fprintf( stderr,
				         "%s\n"
				         "Another copy of this server (pid %u) is already using this\n"
				         "installation.  Pass \"allowdupe\" on the command line to run a\n"
				         "second one anyway.\n",
				         SEH_GetLocalizedString_m( "WIN_ALREADY_RUNNING_BODY" ),
				         marker );
				return 0;
			}

			fprintf( stderr,
			         "%s\n"
			         "The previous run did not shut down cleanly.  Continuing without\n"
			         "safe mode; retail would have asked here.\n",
			         SEH_GetLocalizedString_m( "WIN_IMPROPER_QUIT_BODY" ) );
		} else {
			CloseHandle( h );
		}
	}

	h = CreateFileA( (const char *) FileName, 0x40000000u, 0, 0, 2u, 2u, 0 );
	if ( h != (void *) -1 ) {
		transferred = 0;
		if ( WriteFile( h, &pid, 4u, &transferred, 0 ) && transferred == 4 ) {
			CloseHandle( h );
			return 1;
		}
		CloseHandle( h );
	}

	fprintf( stderr, "%s\n", SEH_GetLocalizedString_m( "WIN_DISK_FULL_BODY" ) );
	fprintf( stderr, "Could not write the instance marker \"%s\".\n", (const char *) FileName );
	exit( -1 );
	return 0;
}

/* ---- Sys_DeleteInstanceMarker  0x004640A0 ----  VERIFIED */
BOOL Sys_DeleteInstanceMarker( void )
{
	if ( !FileName[0] ) {
		return 0;
	}
	return DeleteFileA( (const char *) FileName );
}
