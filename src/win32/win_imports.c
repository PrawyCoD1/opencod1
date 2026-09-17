#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#include <mmsystem.h>

/* ---- cod1_ClipCursor  no-address ---- */
int cod1_ClipCursor( const void *r )        { return ClipCursor( (const RECT *) r ); }
/* ---- cod1_ShowCursor  no-address ---- */
int cod1_ShowCursor( int show )             { return ShowCursor( show ); }
/* ---- cod1_ReleaseCapture  no-address ---- */
int cod1_ReleaseCapture( void )             { return ReleaseCapture(); }

/* ---- cod1_GetSystemMetrics  no-address ---- */
int cod1_GetSystemMetrics( int index )      { return GetSystemMetrics( index ); }
/* ---- cod1_GetWindowRect  no-address ---- */
int cod1_GetWindowRect( void *h, void *r )  { return GetWindowRect( (HWND) h, (RECT *) r ); }
/* ---- cod1_GetCursorPos  no-address ---- */
int cod1_GetCursorPos( void *p )            { return GetCursorPos( (POINT *) p ); }
/* ---- cod1_SetCursorPos  no-address ---- */
int cod1_SetCursorPos( int x, int y )       { return SetCursorPos( x, y ); }
/* ---- cod1_SetCapture  no-address ---- */
int cod1_SetCapture( void *h )              { return (int) SetCapture( (HWND) h ); }
/* ---- cod1_MessageBoxA  no-address ---- */
int cod1_MessageBoxA( void *h, const char *t, const char *c, unsigned f ) {
	return MessageBoxA( (HWND) h, t, c, f );
}
/* ---- cod1_DeleteFileA  no-address ---- */
int cod1_DeleteFileA( const char *p )       { return DeleteFileA( p ); }
/* ---- cod1_FreeLibrary  no-address ---- */
int cod1_FreeLibrary( void *m )             { return FreeLibrary( (HMODULE) m ); }
/* ---- cod1_IsBadReadPtr  no-address ---- */
int cod1_IsBadReadPtr( const void *p, unsigned n ) { return IsBadReadPtr( p, n ); }
/* ---- cod1_WSAStartup  no-address ---- */
int cod1_WSAStartup( unsigned short v, void *d )   { return WSAStartup( v, (LPWSADATA) d ); }
/* ---- cod1_WSACleanup  no-address ---- */
int cod1_WSACleanup( void )                 { return WSACleanup(); }
/* ---- cod1_WSAGetLastError  no-address ---- */
int cod1_WSAGetLastError( void )            { return WSAGetLastError(); }
/* ---- cod1_timeGetTime  no-address ---- */
int cod1_timeGetTime( void )                { return (int) timeGetTime(); }
/* ---- cod1_midiInClose  no-address ---- */
int cod1_midiInClose( void *h )             { return midiInClose( (HMIDIIN) h ); }

/* ---- cod1_CreateFileA  no-address ---- */
void *cod1_CreateFileA( const char *name, unsigned access, unsigned share,
                        void *sec, unsigned disp, unsigned flags, void *tmpl ) {
	return (void *) CreateFileA( name, access, share,
	                             (LPSECURITY_ATTRIBUTES) sec, disp, flags,
	                             (HANDLE) tmpl );
}
/* ---- cod1_ReadFile  no-address ---- */
int cod1_ReadFile( void *h, void *buf, unsigned n, unsigned *got, void *ov ) {
	return ReadFile( (HANDLE) h, buf, n, (LPDWORD) got, (LPOVERLAPPED) ov );
}
/* ---- cod1_WriteFile  no-address ---- */
int cod1_WriteFile( void *h, const void *buf, unsigned n, unsigned *put, void *ov ) {
	return WriteFile( (HANDLE) h, buf, n, (LPDWORD) put, (LPOVERLAPPED) ov );
}
/* ---- cod1_CloseHandle  no-address ---- */
int cod1_CloseHandle( void *h )             { return CloseHandle( (HANDLE) h ); }
/* ---- cod1_GetDriveTypeA  no-address ---- */
int cod1_GetDriveTypeA( const char *root )  { return GetDriveTypeA( root ); }
/* ---- cod1_GetModuleFileNameA  no-address ---- */
int cod1_GetModuleFileNameA( void *m, char *buf, unsigned n ) {
	return GetModuleFileNameA( (HMODULE) m, buf, n );
}
/* ---- cod1_Sleep  no-address ---- */
void cod1_Sleep( unsigned ms )              { Sleep( ms ); }
