
#ifndef TR_QGL_H
#define TR_QGL_H

#include "tr_gl_types.h"

#ifndef QGLAPI
#define QGLAPI  __stdcall
#endif

#ifdef QGL_OWNER
#define QGLDECL
#else
#define QGLDECL extern
#endif

#define qglActiveTextureARB   qgls_ActiveTextureARB   /* 0x016C3F34 */
#define qglGetString          qgls_GetString          /* 0x016C3ED4 */
#define qglLockArraysEXT      qgls_LockArraysEXT      /* 0x016C40FC */

typedef void ( QGLAPI *QGLPROC )( void );


QGLDECL void ( QGLAPI * qglAccum )( GLenum op, GLfloat value );
QGLDECL void ( QGLAPI * qglAlphaFunc )( GLenum func, GLclampf ref );
QGLDECL GLboolean ( QGLAPI * qglAreTexturesResident )( GLsizei n, const GLuint *textures, GLboolean *residences );
QGLDECL void ( QGLAPI * qglArrayElement )( GLint i );
QGLDECL void ( QGLAPI * qglBegin )( GLenum mode );
QGLDECL void ( QGLAPI * qglBindTexture )( GLenum target, GLuint texture );
QGLDECL void ( QGLAPI * qglBitmap )( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap );
QGLDECL void ( QGLAPI * qglBlendFunc )( GLenum sfactor, GLenum dfactor );
QGLDECL void ( QGLAPI * qglCallList )( GLuint list );
QGLDECL void ( QGLAPI * qglCallLists )( GLsizei n, GLenum type, const GLvoid *lists );
QGLDECL void ( QGLAPI * qglClear )( GLbitfield mask );
QGLDECL void ( QGLAPI * qglClearAccum )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
QGLDECL void ( QGLAPI * qglClearColor )( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
QGLDECL void ( QGLAPI * qglClearDepth )( GLclampd depth );
QGLDECL void ( QGLAPI * qglClearIndex )( GLfloat c );
QGLDECL void ( QGLAPI * qglClearStencil )( GLint s );
QGLDECL void ( QGLAPI * qglClipPlane )( GLenum plane, const GLdouble *equation );
QGLDECL void ( QGLAPI * qglColor3b )( GLbyte red, GLbyte green, GLbyte blue );
QGLDECL void ( QGLAPI * qglColor3bv )( const GLbyte *v );
QGLDECL void ( QGLAPI * qglColor3d )( GLdouble red, GLdouble green, GLdouble blue );
QGLDECL void ( QGLAPI * qglColor3dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglColor3f )( GLfloat red, GLfloat green, GLfloat blue );
QGLDECL void ( QGLAPI * qglColor3fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglColor3i )( GLint red, GLint green, GLint blue );
QGLDECL void ( QGLAPI * qglColor3iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglColor3s )( GLshort red, GLshort green, GLshort blue );
QGLDECL void ( QGLAPI * qglColor3sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglColor3ub )( GLubyte red, GLubyte green, GLubyte blue );
QGLDECL void ( QGLAPI * qglColor3ubv )( const GLubyte *v );
QGLDECL void ( QGLAPI * qglColor3ui )( GLuint red, GLuint green, GLuint blue );
QGLDECL void ( QGLAPI * qglColor3uiv )( const GLuint *v );
QGLDECL void ( QGLAPI * qglColor3us )( GLushort red, GLushort green, GLushort blue );
QGLDECL void ( QGLAPI * qglColor3usv )( const GLushort *v );
QGLDECL void ( QGLAPI * qglColor4b )( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha );
QGLDECL void ( QGLAPI * qglColor4bv )( const GLbyte *v );
QGLDECL void ( QGLAPI * qglColor4d )( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha );
QGLDECL void ( QGLAPI * qglColor4dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglColor4f )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
QGLDECL void ( QGLAPI * qglColor4fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglColor4i )( GLint red, GLint green, GLint blue, GLint alpha );
QGLDECL void ( QGLAPI * qglColor4iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglColor4s )( GLshort red, GLshort green, GLshort blue, GLshort alpha );
QGLDECL void ( QGLAPI * qglColor4sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglColor4ub )( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );
QGLDECL void ( QGLAPI * qglColor4ubv )( const GLubyte *v );
QGLDECL void ( QGLAPI * qglColor4ui )( GLuint red, GLuint green, GLuint blue, GLuint alpha );
QGLDECL void ( QGLAPI * qglColor4uiv )( const GLuint *v );
QGLDECL void ( QGLAPI * qglColor4us )( GLushort red, GLushort green, GLushort blue, GLushort alpha );
QGLDECL void ( QGLAPI * qglColor4usv )( const GLushort *v );
QGLDECL void ( QGLAPI * qglColorMask )( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );
QGLDECL void ( QGLAPI * qglColorMaterial )( GLenum face, GLenum mode );
QGLDECL void ( QGLAPI * qglColorPointer )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
QGLDECL void ( QGLAPI * qglCopyPixels )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type );
QGLDECL void ( QGLAPI * qglCopyTexImage1D )( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border );
QGLDECL void ( QGLAPI * qglCopyTexImage2D )( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border );
QGLDECL void ( QGLAPI * qglCopyTexSubImage1D )( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width );
QGLDECL void ( QGLAPI * qglCopyTexSubImage2D )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height );
QGLDECL void ( QGLAPI * qglCullFace )( GLenum mode );
QGLDECL void ( QGLAPI * qglDeleteLists )( GLuint list, GLsizei range );
QGLDECL void ( QGLAPI * qglDeleteTextures )( GLsizei n, const GLuint *textures );
QGLDECL void ( QGLAPI * qglDepthFunc )( GLenum func );
QGLDECL void ( QGLAPI * qglDepthMask )( GLboolean flag );
QGLDECL void ( QGLAPI * qglDepthRange )( GLclampd zNear, GLclampd zFar );
QGLDECL void ( QGLAPI * qglDisable )( GLenum cap );
QGLDECL void ( QGLAPI * qglDisableClientState )( GLenum array );
QGLDECL void ( QGLAPI * qglDrawArrays )( GLenum mode, GLint first, GLsizei count );
QGLDECL void ( QGLAPI * qglDrawBuffer )( GLenum mode );
QGLDECL void ( QGLAPI * qglDrawElements )( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
QGLDECL void ( QGLAPI * qglDrawPixels )( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
QGLDECL void ( QGLAPI * qglEdgeFlag )( GLboolean flag );
QGLDECL void ( QGLAPI * qglEdgeFlagPointer )( GLsizei stride, const GLvoid *pointer );
QGLDECL void ( QGLAPI * qglEdgeFlagv )( const GLboolean *flag );
QGLDECL void ( QGLAPI * qglEnable )( GLenum cap );
QGLDECL void ( QGLAPI * qglEnableClientState )( GLenum array );
QGLDECL void ( QGLAPI * qglEnd )( void );
QGLDECL void ( QGLAPI * qglEndList )( void );
QGLDECL void ( QGLAPI * qglEvalCoord1d )( GLdouble u );
QGLDECL void ( QGLAPI * qglEvalCoord1dv )( const GLdouble *u );
QGLDECL void ( QGLAPI * qglEvalCoord1f )( GLfloat u );
QGLDECL void ( QGLAPI * qglEvalCoord1fv )( const GLfloat *u );
QGLDECL void ( QGLAPI * qglEvalCoord2d )( GLdouble u, GLdouble v );
QGLDECL void ( QGLAPI * qglEvalCoord2dv )( const GLdouble *u );
QGLDECL void ( QGLAPI * qglEvalCoord2f )( GLfloat u, GLfloat v );
QGLDECL void ( QGLAPI * qglEvalCoord2fv )( const GLfloat *u );
QGLDECL void ( QGLAPI * qglEvalMesh1 )( GLenum mode, GLint i1, GLint i2 );
QGLDECL void ( QGLAPI * qglEvalMesh2 )( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 );
QGLDECL void ( QGLAPI * qglEvalPoint1 )( GLint i );
QGLDECL void ( QGLAPI * qglEvalPoint2 )( GLint i, GLint j );
QGLDECL void ( QGLAPI * qglFeedbackBuffer )( GLsizei size, GLenum type, GLfloat *buffer );
QGLDECL void ( QGLAPI * qglFinish )( void );
QGLDECL void ( QGLAPI * qglFlush )( void );
QGLDECL void ( QGLAPI * qglFogf )( GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglFogfv )( GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglFogi )( GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglFogiv )( GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglFrontFace )( GLenum mode );
QGLDECL void ( QGLAPI * qglFrustum )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
QGLDECL GLuint ( QGLAPI * qglGenLists )( GLsizei range );
QGLDECL void ( QGLAPI * qglGenTextures )( GLsizei n, GLuint *textures );
QGLDECL void ( QGLAPI * qglGetBooleanv )( GLenum pname, GLboolean *params );
QGLDECL void ( QGLAPI * qglGetClipPlane )( GLenum plane, GLdouble *equation );
QGLDECL void ( QGLAPI * qglGetDoublev )( GLenum pname, GLdouble *params );
QGLDECL GLenum ( QGLAPI * qglGetError )( void );
QGLDECL void ( QGLAPI * qglGetFloatv )( GLenum pname, GLfloat *params );
QGLDECL void ( QGLAPI * qglGetIntegerv )( GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglGetLightfv )( GLenum light, GLenum pname, GLfloat *params );
QGLDECL void ( QGLAPI * qglGetLightiv )( GLenum light, GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglGetMapdv )( GLenum target, GLenum query, GLdouble *v );
QGLDECL void ( QGLAPI * qglGetMapfv )( GLenum target, GLenum query, GLfloat *v );
QGLDECL void ( QGLAPI * qglGetMapiv )( GLenum target, GLenum query, GLint *v );
QGLDECL void ( QGLAPI * qglGetMaterialfv )( GLenum face, GLenum pname, GLfloat *params );
QGLDECL void ( QGLAPI * qglGetMaterialiv )( GLenum face, GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglGetPixelMapfv )( GLenum map, GLfloat *values );
QGLDECL void ( QGLAPI * qglGetPixelMapuiv )( GLenum map, GLuint *values );
QGLDECL void ( QGLAPI * qglGetPixelMapusv )( GLenum map, GLushort *values );
QGLDECL void ( QGLAPI * qglGetPointerv )( GLenum pname, GLvoid* *params );
QGLDECL void ( QGLAPI * qglGetPolygonStipple )( GLubyte *mask );
QGLDECL const GLubyte *( QGLAPI * qglGetString )( GLenum name );
QGLDECL void ( QGLAPI * qglGetTexEnvfv )( GLenum target, GLenum pname, GLfloat *params );
QGLDECL void ( QGLAPI * qglGetTexEnviv )( GLenum target, GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglGetTexGendv )( GLenum coord, GLenum pname, GLdouble *params );
QGLDECL void ( QGLAPI * qglGetTexGenfv )( GLenum coord, GLenum pname, GLfloat *params );
QGLDECL void ( QGLAPI * qglGetTexGeniv )( GLenum coord, GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglGetTexImage )( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels );
QGLDECL void ( QGLAPI * qglGetTexLevelParameterfv )( GLenum target, GLint level, GLenum pname, GLfloat *params );
QGLDECL void ( QGLAPI * qglGetTexLevelParameteriv )( GLenum target, GLint level, GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglGetTexParameterfv )( GLenum target, GLenum pname, GLfloat *params );
QGLDECL void ( QGLAPI * qglGetTexParameteriv )( GLenum target, GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglHint )( GLenum target, GLenum mode );
QGLDECL void ( QGLAPI * qglIndexMask )( GLuint mask );
QGLDECL void ( QGLAPI * qglIndexPointer )( GLenum type, GLsizei stride, const GLvoid *pointer );
QGLDECL void ( QGLAPI * qglIndexd )( GLdouble c );
QGLDECL void ( QGLAPI * qglIndexdv )( const GLdouble *c );
QGLDECL void ( QGLAPI * qglIndexf )( GLfloat c );
QGLDECL void ( QGLAPI * qglIndexfv )( const GLfloat *c );
QGLDECL void ( QGLAPI * qglIndexi )( GLint c );
QGLDECL void ( QGLAPI * qglIndexiv )( const GLint *c );
QGLDECL void ( QGLAPI * qglIndexs )( GLshort c );
QGLDECL void ( QGLAPI * qglIndexsv )( const GLshort *c );
QGLDECL void ( QGLAPI * qglIndexub )( GLubyte c );
QGLDECL void ( QGLAPI * qglIndexubv )( const GLubyte *c );
QGLDECL void ( QGLAPI * qglInitNames )( void );
QGLDECL void ( QGLAPI * qglInterleavedArrays )( GLenum format, GLsizei stride, const GLvoid *pointer );
QGLDECL GLboolean ( QGLAPI * qglIsEnabled )( GLenum cap );
QGLDECL GLboolean ( QGLAPI * qglIsList )( GLuint list );
QGLDECL GLboolean ( QGLAPI * qglIsTexture )( GLuint texture );
QGLDECL void ( QGLAPI * qglLightModelf )( GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglLightModelfv )( GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglLightModeli )( GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglLightModeliv )( GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglLightf )( GLenum light, GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglLightfv )( GLenum light, GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglLighti )( GLenum light, GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglLightiv )( GLenum light, GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglLineStipple )( GLint factor, GLushort pattern );
QGLDECL void ( QGLAPI * qglLineWidth )( GLfloat width );
QGLDECL void ( QGLAPI * qglListBase )( GLuint base );
QGLDECL void ( QGLAPI * qglLoadIdentity )( void );
QGLDECL void ( QGLAPI * qglLoadMatrixd )( const GLdouble *m );
QGLDECL void ( QGLAPI * qglLoadMatrixf )( const GLfloat *m );
QGLDECL void ( QGLAPI * qglLoadName )( GLuint name );
QGLDECL void ( QGLAPI * qglLogicOp )( GLenum opcode );
QGLDECL void ( QGLAPI * qglMap1d )( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points );
QGLDECL void ( QGLAPI * qglMap1f )( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points );
QGLDECL void ( QGLAPI * qglMap2d )( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points );
QGLDECL void ( QGLAPI * qglMap2f )( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points );
QGLDECL void ( QGLAPI * qglMapGrid1d )( GLint un, GLdouble u1, GLdouble u2 );
QGLDECL void ( QGLAPI * qglMapGrid1f )( GLint un, GLfloat u1, GLfloat u2 );
QGLDECL void ( QGLAPI * qglMapGrid2d )( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 );
QGLDECL void ( QGLAPI * qglMapGrid2f )( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 );
QGLDECL void ( QGLAPI * qglMaterialf )( GLenum face, GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglMaterialfv )( GLenum face, GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglMateriali )( GLenum face, GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglMaterialiv )( GLenum face, GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglMatrixMode )( GLenum mode );
QGLDECL void ( QGLAPI * qglMultMatrixd )( const GLdouble *m );
QGLDECL void ( QGLAPI * qglMultMatrixf )( const GLfloat *m );
QGLDECL void ( QGLAPI * qglNewList )( GLuint list, GLenum mode );
QGLDECL void ( QGLAPI * qglNormal3b )( GLbyte nx, GLbyte ny, GLbyte nz );
QGLDECL void ( QGLAPI * qglNormal3bv )( const GLbyte *v );
QGLDECL void ( QGLAPI * qglNormal3d )( GLdouble nx, GLdouble ny, GLdouble nz );
QGLDECL void ( QGLAPI * qglNormal3dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglNormal3f )( GLfloat nx, GLfloat ny, GLfloat nz );
QGLDECL void ( QGLAPI * qglNormal3fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglNormal3i )( GLint nx, GLint ny, GLint nz );
QGLDECL void ( QGLAPI * qglNormal3iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglNormal3s )( GLshort nx, GLshort ny, GLshort nz );
QGLDECL void ( QGLAPI * qglNormal3sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglNormalPointer )( GLenum type, GLsizei stride, const GLvoid *pointer );
QGLDECL void ( QGLAPI * qglOrtho )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
QGLDECL void ( QGLAPI * qglPassThrough )( GLfloat token );
QGLDECL void ( QGLAPI * qglPixelMapfv )( GLenum map, GLsizei mapsize, const GLfloat *values );
QGLDECL void ( QGLAPI * qglPixelMapuiv )( GLenum map, GLsizei mapsize, const GLuint *values );
QGLDECL void ( QGLAPI * qglPixelMapusv )( GLenum map, GLsizei mapsize, const GLushort *values );
QGLDECL void ( QGLAPI * qglPixelStoref )( GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglPixelStorei )( GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglPixelTransferf )( GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglPixelTransferi )( GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglPixelZoom )( GLfloat xfactor, GLfloat yfactor );
QGLDECL void ( QGLAPI * qglPointSize )( GLfloat size );
QGLDECL void ( QGLAPI * qglPolygonMode )( GLenum face, GLenum mode );
QGLDECL void ( QGLAPI * qglPolygonOffset )( GLfloat factor, GLfloat units );
QGLDECL void ( QGLAPI * qglPolygonStipple )( const GLubyte *mask );
QGLDECL void ( QGLAPI * qglPopAttrib )( void );
QGLDECL void ( QGLAPI * qglPopClientAttrib )( void );
QGLDECL void ( QGLAPI * qglPopMatrix )( void );
QGLDECL void ( QGLAPI * qglPopName )( void );
QGLDECL void ( QGLAPI * qglPrioritizeTextures )( GLsizei n, const GLuint *textures, const GLclampf *priorities );
QGLDECL void ( QGLAPI * qglPushAttrib )( GLbitfield mask );
QGLDECL void ( QGLAPI * qglPushClientAttrib )( GLbitfield mask );
QGLDECL void ( QGLAPI * qglPushMatrix )( void );
QGLDECL void ( QGLAPI * qglPushName )( GLuint name );
QGLDECL void ( QGLAPI * qglRasterPos2d )( GLdouble x, GLdouble y );
QGLDECL void ( QGLAPI * qglRasterPos2dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglRasterPos2f )( GLfloat x, GLfloat y );
QGLDECL void ( QGLAPI * qglRasterPos2fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglRasterPos2i )( GLint x, GLint y );
QGLDECL void ( QGLAPI * qglRasterPos2iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglRasterPos2s )( GLshort x, GLshort y );
QGLDECL void ( QGLAPI * qglRasterPos2sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglRasterPos3d )( GLdouble x, GLdouble y, GLdouble z );
QGLDECL void ( QGLAPI * qglRasterPos3dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglRasterPos3f )( GLfloat x, GLfloat y, GLfloat z );
QGLDECL void ( QGLAPI * qglRasterPos3fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglRasterPos3i )( GLint x, GLint y, GLint z );
QGLDECL void ( QGLAPI * qglRasterPos3iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglRasterPos3s )( GLshort x, GLshort y, GLshort z );
QGLDECL void ( QGLAPI * qglRasterPos3sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglRasterPos4d )( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
QGLDECL void ( QGLAPI * qglRasterPos4dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglRasterPos4f )( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
QGLDECL void ( QGLAPI * qglRasterPos4fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglRasterPos4i )( GLint x, GLint y, GLint z, GLint w );
QGLDECL void ( QGLAPI * qglRasterPos4iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglRasterPos4s )( GLshort x, GLshort y, GLshort z, GLshort w );
QGLDECL void ( QGLAPI * qglRasterPos4sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglReadBuffer )( GLenum mode );
QGLDECL void ( QGLAPI * qglReadPixels )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );
QGLDECL void ( QGLAPI * qglRectd )( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 );
QGLDECL void ( QGLAPI * qglRectdv )( const GLdouble *v1, const GLdouble *v2 );
QGLDECL void ( QGLAPI * qglRectf )( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );
QGLDECL void ( QGLAPI * qglRectfv )( const GLfloat *v1, const GLfloat *v2 );
QGLDECL void ( QGLAPI * qglRecti )( GLint x1, GLint y1, GLint x2, GLint y2 );
QGLDECL void ( QGLAPI * qglRectiv )( const GLint *v1, const GLint *v2 );
QGLDECL void ( QGLAPI * qglRects )( GLshort x1, GLshort y1, GLshort x2, GLshort y2 );
QGLDECL void ( QGLAPI * qglRectsv )( const GLshort *v1, const GLshort *v2 );
QGLDECL GLint ( QGLAPI * qglRenderMode )( GLenum mode );
QGLDECL void ( QGLAPI * qglRotated )( GLdouble angle, GLdouble x, GLdouble y, GLdouble z );
QGLDECL void ( QGLAPI * qglRotatef )( GLfloat angle, GLfloat x, GLfloat y, GLfloat z );
QGLDECL void ( QGLAPI * qglScaled )( GLdouble x, GLdouble y, GLdouble z );
QGLDECL void ( QGLAPI * qglScalef )( GLfloat x, GLfloat y, GLfloat z );
QGLDECL void ( QGLAPI * qglScissor )( GLint x, GLint y, GLsizei width, GLsizei height );
QGLDECL void ( QGLAPI * qglSelectBuffer )( GLsizei size, GLuint *buffer );
QGLDECL void ( QGLAPI * qglShadeModel )( GLenum mode );
QGLDECL void ( QGLAPI * qglStencilFunc )( GLenum func, GLint ref, GLuint mask );
QGLDECL void ( QGLAPI * qglStencilMask )( GLuint mask );
QGLDECL void ( QGLAPI * qglStencilOp )( GLenum fail, GLenum zfail, GLenum zpass );
QGLDECL void ( QGLAPI * qglTexCoord1d )( GLdouble s );
QGLDECL void ( QGLAPI * qglTexCoord1dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglTexCoord1f )( GLfloat s );
QGLDECL void ( QGLAPI * qglTexCoord1fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglTexCoord1i )( GLint s );
QGLDECL void ( QGLAPI * qglTexCoord1iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglTexCoord1s )( GLshort s );
QGLDECL void ( QGLAPI * qglTexCoord1sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglTexCoord2d )( GLdouble s, GLdouble t );
QGLDECL void ( QGLAPI * qglTexCoord2dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglTexCoord2f )( GLfloat s, GLfloat t );
QGLDECL void ( QGLAPI * qglTexCoord2fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglTexCoord2i )( GLint s, GLint t );
QGLDECL void ( QGLAPI * qglTexCoord2iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglTexCoord2s )( GLshort s, GLshort t );
QGLDECL void ( QGLAPI * qglTexCoord2sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglTexCoord3d )( GLdouble s, GLdouble t, GLdouble r );
QGLDECL void ( QGLAPI * qglTexCoord3dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglTexCoord3f )( GLfloat s, GLfloat t, GLfloat r );
QGLDECL void ( QGLAPI * qglTexCoord3fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglTexCoord3i )( GLint s, GLint t, GLint r );
QGLDECL void ( QGLAPI * qglTexCoord3iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglTexCoord3s )( GLshort s, GLshort t, GLshort r );
QGLDECL void ( QGLAPI * qglTexCoord3sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglTexCoord4d )( GLdouble s, GLdouble t, GLdouble r, GLdouble q );
QGLDECL void ( QGLAPI * qglTexCoord4dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglTexCoord4f )( GLfloat s, GLfloat t, GLfloat r, GLfloat q );
QGLDECL void ( QGLAPI * qglTexCoord4fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglTexCoord4i )( GLint s, GLint t, GLint r, GLint q );
QGLDECL void ( QGLAPI * qglTexCoord4iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglTexCoord4s )( GLshort s, GLshort t, GLshort r, GLshort q );
QGLDECL void ( QGLAPI * qglTexCoord4sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglTexCoordPointer )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
QGLDECL void ( QGLAPI * qglTexEnvf )( GLenum target, GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglTexEnvfv )( GLenum target, GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglTexEnvi )( GLenum target, GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglTexEnviv )( GLenum target, GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglTexGend )( GLenum coord, GLenum pname, GLdouble param );
QGLDECL void ( QGLAPI * qglTexGendv )( GLenum coord, GLenum pname, const GLdouble *params );
QGLDECL void ( QGLAPI * qglTexGenf )( GLenum coord, GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglTexGenfv )( GLenum coord, GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglTexGeni )( GLenum coord, GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglTexGeniv )( GLenum coord, GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglTexImage1D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
QGLDECL void ( QGLAPI * qglTexImage2D )( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
QGLDECL void ( QGLAPI * qglTexParameterf )( GLenum target, GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglTexParameterfv )( GLenum target, GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglTexParameteri )( GLenum target, GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglTexParameteriv )( GLenum target, GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglTexSubImage1D )( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels );
QGLDECL void ( QGLAPI * qglTexSubImage2D )( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
QGLDECL void ( QGLAPI * qglTranslated )( GLdouble x, GLdouble y, GLdouble z );
QGLDECL void ( QGLAPI * qglTranslatef )( GLfloat x, GLfloat y, GLfloat z );
QGLDECL void ( QGLAPI * qglVertex2d )( GLdouble x, GLdouble y );
QGLDECL void ( QGLAPI * qglVertex2dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglVertex2f )( GLfloat x, GLfloat y );
QGLDECL void ( QGLAPI * qglVertex2fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglVertex2i )( GLint x, GLint y );
QGLDECL void ( QGLAPI * qglVertex2iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglVertex2s )( GLshort x, GLshort y );
QGLDECL void ( QGLAPI * qglVertex2sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglVertex3d )( GLdouble x, GLdouble y, GLdouble z );
QGLDECL void ( QGLAPI * qglVertex3dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglVertex3f )( GLfloat x, GLfloat y, GLfloat z );
QGLDECL void ( QGLAPI * qglVertex3fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglVertex3i )( GLint x, GLint y, GLint z );
QGLDECL void ( QGLAPI * qglVertex3iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglVertex3s )( GLshort x, GLshort y, GLshort z );
QGLDECL void ( QGLAPI * qglVertex3sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglVertex4d )( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
QGLDECL void ( QGLAPI * qglVertex4dv )( const GLdouble *v );
QGLDECL void ( QGLAPI * qglVertex4f )( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
QGLDECL void ( QGLAPI * qglVertex4fv )( const GLfloat *v );
QGLDECL void ( QGLAPI * qglVertex4i )( GLint x, GLint y, GLint z, GLint w );
QGLDECL void ( QGLAPI * qglVertex4iv )( const GLint *v );
QGLDECL void ( QGLAPI * qglVertex4s )( GLshort x, GLshort y, GLshort z, GLshort w );
QGLDECL void ( QGLAPI * qglVertex4sv )( const GLshort *v );
QGLDECL void ( QGLAPI * qglVertexPointer )( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
QGLDECL void ( QGLAPI * qglViewport )( GLint x, GLint y, GLsizei width, GLsizei height );


QGLDECL void ( QGLAPI * qglActiveTextureARB )( GLenum texture );
QGLDECL void ( QGLAPI * qglAlphaFragmentOp1ATI )( GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod );
QGLDECL void ( QGLAPI * qglAlphaFragmentOp2ATI )( GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod );
QGLDECL void ( QGLAPI * qglAlphaFragmentOp3ATI )( GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod );
QGLDECL void ( QGLAPI * qglArrayObjectATI )( GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset );
QGLDECL void ( QGLAPI * qglBeginFragmentShaderATI )( void );
QGLDECL void ( QGLAPI * qglBindBufferARB )( GLenum target, GLuint buffer );
QGLDECL void ( QGLAPI * qglBindFragmentShaderATI )( GLuint id );
QGLDECL void ( QGLAPI * qglBindProgramARB )( GLenum target, GLuint program );
QGLDECL void ( QGLAPI * qglBufferDataARB )( GLenum target, GLsizei size, const GLvoid *data, GLenum usage );
QGLDECL void ( QGLAPI * qglBufferSubDataARB )( GLenum target, GLint offset, GLsizei size, const GLvoid *data );
QGLDECL void ( QGLAPI * qglClientActiveTextureARB )( GLenum texture );
QGLDECL void ( QGLAPI * qglColorFragmentOp1ATI )( GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod );
QGLDECL void ( QGLAPI * qglColorFragmentOp2ATI )( GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod );
QGLDECL void ( QGLAPI * qglColorFragmentOp3ATI )( GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod );
QGLDECL void ( QGLAPI * qglCombinerInputNV )( GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage );
QGLDECL void ( QGLAPI * qglCombinerOutputNV )( GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum );
QGLDECL void ( QGLAPI * qglCombinerParameterfNV )( GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglCombinerParameterfvNV )( GLenum pname, const GLfloat *params );
QGLDECL void ( QGLAPI * qglCombinerParameteriNV )( GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglCombinerParameterivNV )( GLenum pname, const GLint *params );
QGLDECL void ( QGLAPI * qglCombinerStageParameterfvNV )( GLenum stage, GLenum pname, const GLfloat *params );
QGLDECL QGLPROC qglCompressedTexImage1DARB;
QGLDECL void ( QGLAPI * qglCompressedTexImage2DARB )( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data );
QGLDECL QGLPROC qglCompressedTexImage3DARB;
QGLDECL QGLPROC qglCompressedTexSubImage1D;
QGLDECL QGLPROC qglCompressedTexSubImage2DARB;
QGLDECL QGLPROC qglCompressedTexSubImage3DARB;
QGLDECL void ( QGLAPI * qglDeleteBuffersARB )( GLsizei n, const GLuint *buffers );
QGLDECL void ( QGLAPI * qglDeleteFencesNV )( GLsizei n, const GLuint *fences );
QGLDECL void ( QGLAPI * qglDeleteFragmentShaderATI )( GLuint id );
QGLDECL void ( QGLAPI * qglDeleteProgramsARB )( GLsizei n, const GLuint *programs );
QGLDECL void ( QGLAPI * qglDisableVertexAttribArrayARB )( GLuint index );
QGLDECL void ( QGLAPI * qglDrawElementArrayATI )( GLenum mode, GLsizei count );
QGLDECL void ( QGLAPI * qglDrawRangeElementArrayATI )( GLenum mode, GLuint start, GLuint end, GLsizei count );
QGLDECL void ( QGLAPI * qglDrawRangeElementsEXT )( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices );
QGLDECL void ( QGLAPI * qglElementPointerATI )( GLenum type, const GLvoid *pointer );
QGLDECL void ( QGLAPI * qglEnableVertexAttribArrayARB )( GLuint index );
QGLDECL void ( QGLAPI * qglEndFragmentShaderATI )( void );
QGLDECL void ( QGLAPI * qglFinalCombinerInputNV )( GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage );
QGLDECL void ( QGLAPI * qglFinishFenceNV )( GLuint fence );
QGLDECL void ( QGLAPI * qglFlushVertexArrayRangeNV )( void );
QGLDECL void ( QGLAPI * qglFreeObjectBufferATI )( GLuint buffer );
QGLDECL void ( QGLAPI * qglGenBuffersARB )( GLsizei n, GLuint *buffers );
QGLDECL void ( QGLAPI * qglGenFencesNV )( GLsizei n, GLuint *fences );
QGLDECL GLuint ( QGLAPI * qglGenFragmentShadersATI )( GLuint range );
QGLDECL void ( QGLAPI * qglGenProgramsARB )( GLsizei n, GLuint *programs );
QGLDECL QGLPROC qglGetArrayObjectfvATI;
QGLDECL QGLPROC qglGetArrayObjectivATI;
QGLDECL void ( QGLAPI * qglGetBufferParameterivARB )( GLenum target, GLenum pname, GLint *params );
QGLDECL void ( QGLAPI * qglGetBufferPointervARB )( GLenum target, GLenum pname, GLvoid **params );
QGLDECL void ( QGLAPI * qglGetBufferSubDataARB )( GLenum target, GLint offset, GLsizei size, GLvoid *data );
QGLDECL QGLPROC qglGetCombinerInputParameterfvNV;
QGLDECL QGLPROC qglGetCombinerInputParameterivNV;
QGLDECL QGLPROC qglGetCombinerOutputParameterfvNV;
QGLDECL QGLPROC qglGetCombinerOutputParameterivNV;
QGLDECL QGLPROC qglGetCombinerStageParameterfvNV;
QGLDECL QGLPROC qglGetCompressedTexImage;
QGLDECL void ( QGLAPI * qglGetFenceivNV )( GLuint fence, GLenum pname, GLint *params );
QGLDECL QGLPROC qglGetFinalCombinerInputParameterfvNV;
QGLDECL QGLPROC qglGetFinalCombinerInputParameterivNV;
QGLDECL QGLPROC qglGetObjectBufferfvATI;
QGLDECL QGLPROC qglGetObjectBufferivATI;
QGLDECL QGLPROC qglGetProgramEnvParameterdvARB;
QGLDECL QGLPROC qglGetProgramEnvParameterfvARB;
QGLDECL QGLPROC qglGetProgramLocalParameterdvARB;
QGLDECL QGLPROC qglGetProgramLocalParameterfvARB;
QGLDECL QGLPROC qglGetProgramStringARB;
QGLDECL void ( QGLAPI * qglGetProgramivARB )( GLenum target, GLenum pname, GLint *params );
QGLDECL QGLPROC qglGetVariantArrayObjectfvATI;
QGLDECL QGLPROC qglGetVariantArrayObjectivATI;
QGLDECL QGLPROC qglGetVertexAttribPointervARB;
QGLDECL QGLPROC qglGetVertexAttribdvARB;
QGLDECL QGLPROC qglGetVertexAttribfvARB;
QGLDECL QGLPROC qglGetVertexAttribivARB;
QGLDECL GLboolean ( QGLAPI * qglIsBufferARB )( GLuint buffer );
QGLDECL GLboolean ( QGLAPI * qglIsFenceNV )( GLuint fence );
QGLDECL GLboolean ( QGLAPI * qglIsObjectBufferATI )( GLuint buffer );
QGLDECL GLboolean ( QGLAPI * qglIsProgramARB )( GLuint program );
QGLDECL void ( QGLAPI * qglLockArraysEXT )( GLint first, GLsizei count );
QGLDECL GLvoid *( QGLAPI * qglMapBufferARB )( GLenum target, GLenum access );
QGLDECL GLuint ( QGLAPI * qglNewObjectBufferATI )( GLsizei size, const GLvoid *pointer, GLenum usage );
QGLDECL void ( QGLAPI * qglPNTrianglesfATI )( GLenum pname, GLfloat param );
QGLDECL void ( QGLAPI * qglPNTrianglesiATI )( GLenum pname, GLint param );
QGLDECL void ( QGLAPI * qglPassTexCoordATI )( GLuint dst, GLuint coord, GLenum swizzle );
QGLDECL QGLPROC qglProgramEnvParameter4dARB;
QGLDECL QGLPROC qglProgramEnvParameter4dvARB;
QGLDECL void ( QGLAPI * qglProgramEnvParameter4fARB )( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w );
QGLDECL void ( QGLAPI * qglProgramEnvParameter4fvARB )( GLenum target, GLuint index, const GLfloat *params );
QGLDECL QGLPROC qglProgramLocalParameter4dARB;
QGLDECL QGLPROC qglProgramLocalParameter4dvARB;
QGLDECL void ( QGLAPI * qglProgramLocalParameter4fARB )( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w );
QGLDECL void ( QGLAPI * qglProgramLocalParameter4fvARB )( GLenum target, GLuint index, const GLfloat *params );
QGLDECL void ( QGLAPI * qglProgramStringARB )( GLenum target, GLenum format, GLsizei len, const GLvoid *string );
QGLDECL void ( QGLAPI * qglSampleMapATI )( GLuint dst, GLuint interp, GLenum swizzle );
QGLDECL void ( QGLAPI * qglSetFenceNV )( GLuint fence, GLenum condition );
QGLDECL void ( QGLAPI * qglSetFragmentShaderConstantATI )( GLuint dst, const GLfloat *value );
QGLDECL GLboolean ( QGLAPI * qglTestFenceNV )( GLuint fence );
QGLDECL void ( QGLAPI * qglUnlockArraysEXT )( void );
QGLDECL GLboolean ( QGLAPI * qglUnmapBufferARB )( GLenum target );
QGLDECL void ( QGLAPI * qglUpdateObjectBufferATI )( GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve );
QGLDECL QGLPROC qglVariantArrayObjectATI;
QGLDECL void ( QGLAPI * qglVertexArrayRangeNV )( GLsizei length, const GLvoid *pointer );
QGLDECL QGLPROC qglVertexAttrib1dARB;
QGLDECL QGLPROC qglVertexAttrib1dvARB;
QGLDECL QGLPROC qglVertexAttrib1fARB;
QGLDECL QGLPROC qglVertexAttrib1fvARB;
QGLDECL QGLPROC qglVertexAttrib1sARB;
QGLDECL QGLPROC qglVertexAttrib1svARB;
QGLDECL QGLPROC qglVertexAttrib2dARB;
QGLDECL QGLPROC qglVertexAttrib2dvARB;
QGLDECL QGLPROC qglVertexAttrib2fARB;
QGLDECL QGLPROC qglVertexAttrib2fvARB;
QGLDECL QGLPROC qglVertexAttrib2sARB;
QGLDECL QGLPROC qglVertexAttrib2svARB;
QGLDECL QGLPROC qglVertexAttrib3dARB;
QGLDECL QGLPROC qglVertexAttrib3dvARB;
QGLDECL QGLPROC qglVertexAttrib3fARB;
QGLDECL QGLPROC qglVertexAttrib3fvARB;
QGLDECL QGLPROC qglVertexAttrib3sARB;
QGLDECL QGLPROC qglVertexAttrib3svARB;
QGLDECL QGLPROC qglVertexAttrib4NbvARB;
QGLDECL QGLPROC qglVertexAttrib4NivARB;
QGLDECL QGLPROC qglVertexAttrib4NsvARB;
QGLDECL QGLPROC qglVertexAttrib4NubARB;
QGLDECL QGLPROC qglVertexAttrib4NubvARB;
QGLDECL QGLPROC qglVertexAttrib4NuivARB;
QGLDECL QGLPROC qglVertexAttrib4NusvARB;
QGLDECL QGLPROC qglVertexAttrib4bvARB;
QGLDECL QGLPROC qglVertexAttrib4dARB;
QGLDECL QGLPROC qglVertexAttrib4dvARB;
QGLDECL QGLPROC qglVertexAttrib4fARB;
QGLDECL QGLPROC qglVertexAttrib4fvARB;
QGLDECL QGLPROC qglVertexAttrib4ivARB;
QGLDECL QGLPROC qglVertexAttrib4sARB;
QGLDECL QGLPROC qglVertexAttrib4svARB;
QGLDECL QGLPROC qglVertexAttrib4ubvARB;
QGLDECL QGLPROC qglVertexAttrib4uivARB;
QGLDECL QGLPROC qglVertexAttrib4usvARB;
QGLDECL void ( QGLAPI * qglVertexAttribPointerARB )( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer );


#define glAlphaFunc                    qglAlphaFunc                       /* 0x016C4528 */
#define glBindFragmentShaderATI        qglBindFragmentShaderATI           /* 0x016C4334 */
#define glBindProgramARB               qglBindProgramARB                  /* 0x016C3FC4 */
#define glBindTexture                  qglBindTexture                     /* 0x016C439C */
#define glClear                        qglClear                           /* 0x016C46A4 */
#define glClearColor                   qglClearColor                      /* 0x016C4838 */
#define glColor4ubv                    qglColor4ubv                       /* 0x016C41D0 */
#define glColorPointer                 qglColorPointer                    /* 0x016C474C */
#define glCombinerInputNV              qglCombinerInputNV                 /* 0x016C48A0 */
#define glCombinerOutputNV             qglCombinerOutputNV                /* 0x016C3B30 */
#define glCombinerParameterfvNV        qglCombinerParameterfvNV           /* 0x016C3D78 */
#define glCombinerParameteriNV         qglCombinerParameteriNV            /* 0x016C45C0 */
#define glCombinerStageParameterfvNV   qglCombinerStageParameterfvNV      /* 0x016C3C74 */
#define glCopyTexSubImage2D            qglCopyTexSubImage2D               /* 0x016C4AF8 */
#define glDepthFunc                    qglDepthFunc                       /* 0x016C3B64 */
#define glDepthRange                   qglDepthRange                      /* 0x016C46BC */
#define glDisable                      qglDisable                         /* 0x016C3D00 */
#define glDisableClientState           qglDisableClientState              /* 0x016C3C7C */
#define glDrawArrays                   qglDrawArrays                      /* 0x016C433C */
#define glDrawBuffer                   qglDrawBuffer                      /* 0x016C4598 */
#define glDrawElementArrayATI          qglDrawElementArrayATI             /* 0x016C3E00 */
#define glDrawElements                 qglDrawElements                    /* 0x016C445C */
#define glEnable                       qglEnable                          /* 0x016C4288 */
#define glEnableClientState            qglEnableClientState               /* 0x016C3C58 */
#define glFinalCombinerInputNV         qglFinalCombinerInputNV            /* 0x016C3DF0 */
#define qglGetError_0                  qglGetError                        /* 0x016C3B5C */
#define glLightModelfv                 qglLightModelfv                    /* 0x016C3F00 */
#define glLoadMatrixf                  qglLoadMatrixf                     /* 0x016C4020 */
#define glMatrixMode                   qglMatrixMode                      /* 0x016C408C */
#define glNormalPointer                qglNormalPointer                   /* 0x016C435C */
#define lConfig_textureCompression     qglPNTrianglesiATI                 /* 0x016C3BE0 */
#define glPolygonMode                  qglPolygonMode                     /* 0x016C4094 */
#define glProgramLocalParameter4fARB   qglProgramLocalParameter4fARB      /* 0x016C4044 */
#define glProgramLocalParameter4fvARB  qglProgramLocalParameter4fvARB     /* 0x016C3DC4 */
#define glReadPixels                   qglReadPixels                      /* 0x016C4234 */
#define glScissor                      qglScissor                         /* 0x016C4540 */
#define glStencilFunc                  qglStencilFunc                     /* 0x016C419C */
#define glStencilOp                    qglStencilOp                       /* 0x016C4748 */
#define glTexCoordPointer              qglTexCoordPointer                 /* 0x016C3CC0 */
#define glTexEnvf                      qglTexEnvf                         /* 0x016C3CD8 */
#define glTexEnvfv                     qglTexEnvfv                        /* 0x016C3D30 */
#define glTexEnvi                      qglTexEnvi                         /* 0x016C447C */
#define glTexImage2D                   qglTexImage2D                      /* 0x016C3FE4 */
#define glTexParameteri                qglTexParameteri                   /* 0x016C49DC */
#define glTexSubImage2D                qglTexSubImage2D                   /* 0x016C416C */
#define glVertexPointer                qglVertexPointer                   /* 0x016C4810 */
#define glViewport                     qglViewport                        /* 0x016C47A0 */
#endif
