extern int cod1_ClipCursor( const void *r );
extern int cod1_ShowCursor( int show );
extern int cod1_ReleaseCapture( void );
extern int cod1_GetSystemMetrics( int index );
extern int cod1_GetWindowRect( void *h, void *r );
extern int cod1_GetCursorPos( void *p );
extern int cod1_SetCursorPos( int x, int y );
extern int cod1_SetCapture( void *h );
extern int cod1_MessageBoxA( void *h, const char *t, const char *c, unsigned f );
extern int cod1_DeleteFileA( const char *p );
extern int cod1_FreeLibrary( void *m );
extern int cod1_IsBadReadPtr( const void *p, unsigned n );
extern int cod1_WSAStartup( unsigned short v, void *d );
extern int cod1_WSACleanup( void );
extern int cod1_WSAGetLastError( void );
extern int cod1_timeGetTime( void );
extern int cod1_midiInClose( void *h );

extern void *cod1_CreateFileA( const char *name, unsigned access, unsigned share,
                               void *sec, unsigned disp, unsigned flags, void *tmpl );
extern int  cod1_ReadFile( void *h, void *buf, unsigned n, unsigned *got, void *ov );
extern int  cod1_WriteFile( void *h, const void *buf, unsigned n, unsigned *put, void *ov );
extern int  cod1_CloseHandle( void *h );
extern int  cod1_GetDriveTypeA( const char *root );
extern int  cod1_GetModuleFileNameA( void *m, char *buf, unsigned n );
extern void cod1_Sleep( unsigned ms );

/* ---- ClipCursor  no-address ---- */
int ClipCursor( const void *r )        { return cod1_ClipCursor( r ); }
/* ---- ShowCursor  no-address ---- */
int ShowCursor( int show )             { return cod1_ShowCursor( show ); }
/* ---- ReleaseCapture  no-address ---- */
int ReleaseCapture( void )             { return cod1_ReleaseCapture(); }
/* ---- GetSystemMetrics  no-address ---- */
int GetSystemMetrics( int index )      { return cod1_GetSystemMetrics( index ); }
/* ---- GetWindowRect  no-address ---- */
int GetWindowRect( void *h, void *r )  { return cod1_GetWindowRect( h, r ); }
/* ---- GetCursorPos  no-address ---- */
int GetCursorPos( void *p )            { return cod1_GetCursorPos( p ); }
/* ---- SetCursorPos  no-address ---- */
int SetCursorPos( int x, int y )       { return cod1_SetCursorPos( x, y ); }
/* ---- SetCapture  no-address ---- */
int SetCapture( void *h )              { return cod1_SetCapture( h ); }
/* ---- MessageBoxA  no-address ---- */
int MessageBoxA( void *h, const char *t, const char *c, unsigned f ) {
	return cod1_MessageBoxA( h, t, c, f );
}
/* ---- DeleteFileA  no-address ---- */
int DeleteFileA( const char *p )       { return cod1_DeleteFileA( p ); }
/* ---- FreeLibrary  no-address ---- */
int FreeLibrary( void *m )             { return cod1_FreeLibrary( m ); }
/* ---- IsBadReadPtr  no-address ---- */
int IsBadReadPtr( const void *p, unsigned n ) { return cod1_IsBadReadPtr( p, n ); }
/* ---- WSAStartup  no-address ---- */
int WSAStartup( unsigned short v, void *d )   { return cod1_WSAStartup( v, d ); }
/* ---- WSACleanup  no-address ---- */
int WSACleanup( void )                 { return cod1_WSACleanup(); }
/* ---- WSAGetLastError  no-address ---- */
int WSAGetLastError( void )            { return cod1_WSAGetLastError(); }
/* ---- timeGetTime  no-address ---- */
int timeGetTime( void )                { return cod1_timeGetTime(); }
/* ---- midiInClose  no-address ---- */
int midiInClose( void *h )             { return cod1_midiInClose( h ); }

/* ---- CreateFileA  no-address ---- */
void *CreateFileA( const char *name, unsigned access, unsigned share,
                   void *sec, unsigned disp, unsigned flags, void *tmpl ) {
	return cod1_CreateFileA( name, access, share, sec, disp, flags, tmpl );
}
/* ---- ReadFile  no-address ---- */
int ReadFile( void *h, void *buf, unsigned n, unsigned *got, void *ov ) {
	return cod1_ReadFile( h, buf, n, got, ov );
}
/* ---- WriteFile  no-address ---- */
int WriteFile( void *h, const void *buf, unsigned n, unsigned *put, void *ov ) {
	return cod1_WriteFile( h, buf, n, put, ov );
}
/* ---- CloseHandle  no-address ---- */
int CloseHandle( void *h )             { return cod1_CloseHandle( h ); }
/* ---- GetDriveTypeA  no-address ---- */
int GetDriveTypeA( const char *root )  { return cod1_GetDriveTypeA( root ); }
/* ---- GetModuleFileNameA  no-address ---- */
int GetModuleFileNameA( void *m, char *buf, unsigned n ) {
	return cod1_GetModuleFileNameA( m, buf, n );
}
/* ---- Sleep  no-address ---- */
void Sleep( unsigned ms )              { cod1_Sleep( ms ); }
