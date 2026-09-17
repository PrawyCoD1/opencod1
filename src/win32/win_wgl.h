#ifndef WIN_WGL_H
#define WIN_WGL_H

#ifdef QGL_OWNER
#define QWGLDECL
#else
#define QWGLDECL extern
#endif

QWGLDECL BOOL  ( WINAPI * qwglCopyContext )( HGLRC, HGLRC, UINT );              /* 0x016C3DF8 */
QWGLDECL HGLRC ( WINAPI * qwglCreateContext )( HDC );                           /* 0x016C49A4 */
QWGLDECL HGLRC ( WINAPI * qwglCreateLayerContext )( HDC, int );                 /* 0x016C3EAC */
QWGLDECL BOOL  ( WINAPI * qwglDeleteContext )( HGLRC );                         /* 0x016C4648 */
QWGLDECL HGLRC ( WINAPI * qwglGetCurrentContext )( VOID );                      /* 0x016C3F6C */
QWGLDECL HDC   ( WINAPI * qwglGetCurrentDC )( VOID );                           /* 0x016C44F0 */
QWGLDECL PROC  ( WINAPI * qwglGetProcAddress )( LPCSTR );                       /* 0x016C40E0 */
QWGLDECL BOOL  ( WINAPI * qwglMakeCurrent )( HDC, HGLRC );                      /* 0x016C4A4C */
QWGLDECL BOOL  ( WINAPI * qwglShareLists )( HGLRC, HGLRC );                     /* 0x016C3D28 */
QWGLDECL BOOL  ( WINAPI * qwglUseFontBitmaps )( HDC, DWORD, DWORD, DWORD );     /* 0x016C3B9C */
QWGLDECL BOOL  ( WINAPI * qwglUseFontOutlines )( HDC, DWORD, DWORD, DWORD,
					FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT );                   /* 0x016C3C90 */
QWGLDECL BOOL  ( WINAPI * qwglDescribeLayerPlane )( HDC, int, int, UINT,
					LPLAYERPLANEDESCRIPTOR );                                   /* 0x016C45EC */
QWGLDECL int   ( WINAPI * qwglSetLayerPaletteEntries )( HDC, int, int, int,
					CONST COLORREF * );                                         /* 0x016C4778 */
QWGLDECL int   ( WINAPI * qwglGetLayerPaletteEntries )( HDC, int, int, int,
					COLORREF * );                                               /* 0x016C4564 */
QWGLDECL BOOL  ( WINAPI * qwglRealizeLayerPalette )( HDC, int, BOOL );          /* 0x016C4798 */
QWGLDECL BOOL  ( WINAPI * qwglSwapLayerBuffers )( HDC, UINT );                  /* 0x016C3D24 */

QWGLDECL void *( WINAPI * qwglAllocateMemoryNV )( int, float, float, float );   /* 0x016C457C */
QWGLDECL void  ( WINAPI * qwglFreeMemoryNV )( void * );                         /* 0x016C4500 */
QWGLDECL BOOL  ( WINAPI * qwglSwapIntervalEXT )( int );                         /* 0x016C47E0 */

/* Not a dispatch slot. GLimp_Init fetches this one directly through qwglGetProcAddress and retail keeps it in its own global at 0x016C45A0. */
QWGLDECL const char *( WINAPI * qwglGetExtensionsStringEXT )( void );           /* 0x016C45A0 */

#endif
