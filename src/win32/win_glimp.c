/*
 * @fidelity: verified
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"

#include "../renderer/qgl.h"
#include "win_wgl.h"

#define RCVAR( n )  ( (cvar_t *)( n ) )

#define GLW_PRINT_ALL       0
#define GLW_PRINT_WARNING   2

#define GL_VENDOR               0x1F00      /* push 1F00h @ 0x0050AC96 */
#define GL_RENDERER             0x1F01
#define GL_VERSION              0x1F02
#define GL_EXTENSIONS           0x1F03
#define GL_MAX_TEXTURE_SIZE     0x0D33      /* push 3379 @ 0x005080E1 */

#define glConfig_displayFrequency   glConfig_displayFrequency   /* +0x90 */
#define glConfig_isFullscreen       glConfig_isFullscreen   /* +0x94 */
#define glConfig_stereoEnabled      glConfig_stereoEnabled   /* +0x98 */

/* 0x016C3AEC, glConfig +0x8C. Absent from cod1_globals.h, so owned here. */
float   glConfig_windowAspect;

extern int      g_wv_hInstance;
extern char    *va( const char *format, ... );
extern char    *SEH_GetLocalizedString_m( const char *reference );

#define QGL_GetString( e )      qglGetString( e )
#define QGL_GetIntegerv( e, p ) qglGetIntegerv( ( e ), ( p ) )

/* GLW_SetMode return codes, from the caller's `dec eax` chain at 0x0050A59F. */
#define RSERR_OK            0
#define RSERR_INVALID_FULLSCREEN 1
#define RSERR_INVALID_MODE  2

#define WINDOW_CLASS_NAME   "Call of Duty Multiplayer"

extern void WG_CheckHardwareGamma( void );
extern void WG_RestoreGamma( void );

int      glw_allowSoftwareGL;       /* 0x019BFFC8 r_allowSoftwareGL cvar_t* */
WNDPROC  glw_wndproc;               /* 0x019BFFE0 see GLimp_Init */
HDC      glw_hDC;                   /* 0x019BFFE4 */
HGLRC    glw_hGLRC;                 /* 0x019BFFE8 */
HMODULE  glw_hinstOpenGL;           /* 0x019BFFEC set by QCL_Init */
int      glw_pixelFormatSet;        /* 0x019BFFF0 */
int      glw_desktopBitsPixel;      /* 0x019BFFF4 */
int      glw_desktopWidth;          /* 0x019BFFF8 */
int      glw_desktopHeight;         /* 0x019BFFFC */
int      glw_cdsFullscreen;         /* 0x019C0000 */

static PIXELFORMATDESCRIPTOR    glw_pfd;    /* 0x011EECB8, 40 bytes */
static int                      s_classRegistered;  /* 0x014072E8 */

/* 0x016C3AC0, glConfig +0x60. Set by GLW_InitExtensions for GL_NV_fence. Absent from cod1_globals.h, so owned here. */
int     glConfig_NVFenceAvailable;

extern qboolean QCL_Init( const char *dllname );
extern void     QGL_Shutdown( void );

#define glConfig_maxAnisotropy          glConfig_maxAnisotropy   /* +0x30 */
#define glConfig_textureCubeMap         glConfig_textureCubeMap   /* +0x3C */
#define glConfig_textureEnvCombine      glConfig_textureEnvCombine   /* +0x40 */
#define glConfig_textureEnvDot3         glConfig_textureEnvDot3   /* +0x44 */
#define glConfig_ARBVertexBufferObject  glConfig_ARBVertexBufferObject   /* +0x48 */
#define glConfig_ARBVertexProgram       glConfig_ARBVertexProgram   /* +0x4C */
#define glConfig_rescaleNormal          glConfig_rescaleNormal   /* +0x50 */
#define glConfig_NVVertexArrayRange     glConfig_NVVertexArrayRange   /* +0x5C */
#define glConfig_NVRegisterCombiners    glConfig_NVRegisterCombiners   /* +0x64 */
#define glConfig_NVTextureShader        glConfig_NVTextureShader   /* +0x68 */
#define glConfig_ATIVertexArrayObject   glConfig_ATIVertexArrayObject   /* +0x78 */
#define glConfig_ATIElementArray        glConfig_ATIElementArray   /* +0x7C */
#define glConfig_ATIFragmentShader      glConfig_ATIFragmentShader   /* +0x80 */
#define glConfig_maxActiveTextures      Value           /* +0x18, 0x016C3A78; `Value` is the spelling the renderer reads in GL_SetDefaultState */

#define GL_MAX_TEXTURE_UNITS_ARB        0x84E2
#define GL_TEXTURE0_ARB                 0x84C0
#define GL_MAX_GENERAL_COMBINERS_NV     0x854D
#define GL_FOG_DISTANCE_MODE_NV         0x855A
#define GL_EYE_PLANE_ABSOLUTE_NV        0x855C

#define GLW_BIND( slot, name )  ( *(PROC *)&( slot ) = qwglGetProcAddress( name ) )

/* ---- GLW_HaveGLVersion  0x00508B80 ---- */
static qboolean GLW_HaveGLVersion( int major, int minor ) {
	char	version[1024];
	int		haveMajor, haveMinor;
	char	*dot;

	strncpy( version, (const char *)glConfig_version_string, sizeof( version ) );
	version[sizeof( version ) - 1] = '\0';

	haveMajor = atol( version );
	dot = strchr( version, '.' );
	haveMinor = dot ? atol( dot + 1 ) : 0;

	return ( haveMajor > major || ( haveMajor == major && haveMinor >= minor ) );
}

/* ---- GLW_HaveExtension  0x00508C30 ---- */
static qboolean GLW_HaveExtension( const char *ext, const char *extensions ) {
	size_t	len;
	char	*p;

	if ( !extensions ) {
		return qfalse;
	}
	len = strlen( ext );
	p = strstr( (char *)extensions, ext );
	while ( p ) {
		if ( ( p == extensions || isspace( (unsigned char)*( p - 1 ) ) ) &&
			 ( p[len] == '\0' || isspace( (unsigned char)p[len] ) ) ) {
			return qtrue;
		}
		p = strstr( p + 1, ext );
	}
	return qfalse;
}

/* ---- GLW_MissingFeature  0x00508CD0 ---- */
static void GLW_MissingFeature( void ) {
	ri_Printf( GLW_PRINT_ALL, "\nGL_VENDOR: %s\n", glConfig_vendor_string );
	ri_Printf( GLW_PRINT_ALL, "GL_RENDERER: %s\n", glConfig_renderer_string );
	ri_Printf( GLW_PRINT_ALL, "GL_VERSION: %s\n", glConfig_version_string );
	ri_Printf( GLW_PRINT_ALL, "GL_EXTENSIONS: %s\n", glConfig_extensions_string );
	ri_Error( 0, va( "%s", "EXE_ERR_VIDEOCARD_MISSING_FEATURE" ) );
}

/* ---- GLW_InitVertexProgramExtension  0x00508D40 ---- */
static void GLW_InitVertexProgramExtension( void ) {
	GLW_BIND( qglVertexAttrib1sARB,   "glVertexAttrib1sARB" );
	GLW_BIND( qglVertexAttrib1fARB,   "glVertexAttrib1fARB" );
	GLW_BIND( qglVertexAttrib1dARB,   "glVertexAttrib1dARB" );
	GLW_BIND( qglVertexAttrib2sARB,   "glVertexAttrib2sARB" );
	GLW_BIND( qglVertexAttrib2fARB,   "glVertexAttrib2fARB" );
	GLW_BIND( qglVertexAttrib2dARB,   "glVertexAttrib2dARB" );
	GLW_BIND( qglVertexAttrib3sARB,   "glVertexAttrib3sARB" );
	GLW_BIND( qglVertexAttrib3fARB,   "glVertexAttrib3fARB" );
	GLW_BIND( qglVertexAttrib3dARB,   "glVertexAttrib3dARB" );
	GLW_BIND( qglVertexAttrib4sARB,   "glVertexAttrib4sARB" );
	GLW_BIND( qglVertexAttrib4fARB,   "glVertexAttrib4fARB" );
	GLW_BIND( qglVertexAttrib4dARB,   "glVertexAttrib4dARB" );
	GLW_BIND( qglVertexAttrib4NubARB, "glVertexAttrib4NubARB" );
	GLW_BIND( qglVertexAttrib1svARB,  "glVertexAttrib1svARB" );
	GLW_BIND( qglVertexAttrib1fvARB,  "glVertexAttrib1fvARB" );
	GLW_BIND( qglVertexAttrib1dvARB,  "glVertexAttrib1dvARB" );
	GLW_BIND( qglVertexAttrib2svARB,  "glVertexAttrib2svARB" );
	GLW_BIND( qglVertexAttrib2fvARB,  "glVertexAttrib2fvARB" );
	GLW_BIND( qglVertexAttrib2dvARB,  "glVertexAttrib2dvARB" );
	GLW_BIND( qglVertexAttrib3svARB,  "glVertexAttrib3svARB" );
	GLW_BIND( qglVertexAttrib3fvARB,  "glVertexAttrib3fvARB" );
	GLW_BIND( qglVertexAttrib3dvARB,  "glVertexAttrib3dvARB" );
	GLW_BIND( qglVertexAttrib4bvARB,  "glVertexAttrib4bvARB" );
	GLW_BIND( qglVertexAttrib4svARB,  "glVertexAttrib4svARB" );
	GLW_BIND( qglVertexAttrib4ivARB,  "glVertexAttrib4ivARB" );
	GLW_BIND( qglVertexAttrib4ubvARB, "glVertexAttrib4ubvARB" );
	GLW_BIND( qglVertexAttrib4usvARB, "glVertexAttrib4usvARB" );
	GLW_BIND( qglVertexAttrib4uivARB, "glVertexAttrib4uivARB" );
	GLW_BIND( qglVertexAttrib4fvARB,  "glVertexAttrib4fvARB" );
	GLW_BIND( qglVertexAttrib4dvARB,  "glVertexAttrib4dvARB" );
	GLW_BIND( qglVertexAttrib4NbvARB,  "glVertexAttrib4NbvARB" );
	GLW_BIND( qglVertexAttrib4NsvARB,  "glVertexAttrib4NsvARB" );
	GLW_BIND( qglVertexAttrib4NivARB,  "glVertexAttrib4NivARB" );
	GLW_BIND( qglVertexAttrib4NubvARB, "glVertexAttrib4NubvARB" );
	GLW_BIND( qglVertexAttrib4NusvARB, "glVertexAttrib4NusvARB" );
	GLW_BIND( qglVertexAttrib4NuivARB, "glVertexAttrib4NuivARB" );
	GLW_BIND( qglVertexAttribPointerARB, "glVertexAttribPointerARB" );
	GLW_BIND( qglEnableVertexAttribArrayARB,  "glEnableVertexAttribArrayARB" );
	GLW_BIND( qglDisableVertexAttribArrayARB, "glDisableVertexAttribArrayARB" );
	GLW_BIND( qglProgramStringARB,    "glProgramStringARB" );
	GLW_BIND( qglBindProgramARB,      "glBindProgramARB" );
	GLW_BIND( qglDeleteProgramsARB,   "glDeleteProgramsARB" );
	GLW_BIND( qglGenProgramsARB,      "glGenProgramsARB" );
	GLW_BIND( qglProgramEnvParameter4fARB,   "glProgramEnvParameter4fARB" );
	GLW_BIND( qglProgramEnvParameter4dARB,   "glProgramEnvParameter4dARB" );
	GLW_BIND( qglProgramEnvParameter4fvARB,  "glProgramEnvParameter4fvARB" );
	GLW_BIND( qglProgramEnvParameter4dvARB,  "glProgramEnvParameter4dvARB" );
	GLW_BIND( qglProgramLocalParameter4fARB,  "glProgramLocalParameter4fARB" );
	GLW_BIND( qglProgramLocalParameter4dARB,  "glProgramLocalParameter4dARB" );
	GLW_BIND( qglProgramLocalParameter4fvARB, "glProgramLocalParameter4fvARB" );
	GLW_BIND( qglProgramLocalParameter4dvARB, "glProgramLocalParameter4dvARB" );
	GLW_BIND( qglGetProgramEnvParameterfvARB, "glGetProgramEnvParameterfvARB" );
	GLW_BIND( qglGetProgramEnvParameterdvARB, "glGetProgramEnvParameterdvARB" );
	GLW_BIND( qglGetProgramLocalParameterfvARB, "glGetProgramLocalParameterfvARB" );
	GLW_BIND( qglGetProgramLocalParameterdvARB, "glGetProgramLocalParameterdvARB" );
	GLW_BIND( qglGetProgramivARB,     "glGetProgramivARB" );
	GLW_BIND( qglGetProgramStringARB, "glGetProgramStringARB" );
	GLW_BIND( qglGetVertexAttribdvARB, "glGetVertexAttribdvARB" );
	GLW_BIND( qglGetVertexAttribfvARB, "glGetVertexAttribfvARB" );
	GLW_BIND( qglGetVertexAttribivARB, "glGetVertexAttribivARB" );
	GLW_BIND( qglGetVertexAttribPointervARB, "glGetVertexAttribPointervARB" );
	GLW_BIND( qglIsProgramARB,        "glIsProgramARB" );
}

/* ---- GLW_InitExtensions  0x00509260 ---- */
static void GLW_InitExtensions( void ) {
	const char	*ext = (const char *)glConfig_extensions_string;
	const char	*msg;
	int			combiners;
	int			maxTextures;

	glConfig_maxAnisotropy         = 0;
	glConfig_textureEnvAddAvailable = 0;
	glConfig_textureCubeMap        = 0;
	glConfig_textureEnvCombine     = 0;
	glConfig_textureEnvDot3        = 0;
	glConfig_ARBVertexBufferObject = 0;
	glConfig_ARBVertexProgram      = 0;
	glConfig_rescaleNormal         = 0;
	glConfig_NVFogAvailable        = 0;
	glConfig_NVVertexArrayRange    = 0;
	glConfig_NVFenceAvailable      = 0;
	glConfig_NVRegisterCombiners   = 0;
	glConfig_NVTextureShader       = 0;
	glConfig_ATIVertexArrayObject  = 0;
	glConfig_ATIElementArray       = 0;
	glConfig_ATIFragmentShader     = 0;

	ri_Printf( GLW_PRINT_ALL, "Initializing OpenGL extensions\n" );

	if ( GLW_HaveGLVersion( 1, 3 ) ) {
		GLW_BIND( qglCompressedTexImage3DARB,    "glCompressedTexImage3D" );
		GLW_BIND( qglCompressedTexImage2DARB,    "glCompressedTexImage2D" );
		GLW_BIND( qglCompressedTexImage1DARB,    "glCompressedTexImage1D" );
		GLW_BIND( qglCompressedTexSubImage3DARB, "glCompressedTexSubImage3D" );
		GLW_BIND( qglCompressedTexSubImage2DARB, "glCompressedTexSubImage2D" );
		GLW_BIND( qglCompressedTexSubImage1D,    "glCompressedTexSubImage1D" );
		GLW_BIND( qglGetCompressedTexImage,      "glGetCompressedTexImage" );
	} else {
		if ( !GLW_HaveExtension( "GL_ARB_texture_compression", ext ) ) {
			GLW_MissingFeature();
		}
		GLW_BIND( qglCompressedTexImage3DARB,    "glCompressedTexImage3DARB" );
		GLW_BIND( qglCompressedTexImage2DARB,    "glCompressedTexImage2DARB" );
		GLW_BIND( qglCompressedTexImage1DARB,    "glCompressedTexImage1DARB" );
		GLW_BIND( qglCompressedTexSubImage3DARB, "glCompressedTexSubImage3DARB" );
		GLW_BIND( qglCompressedTexSubImage2DARB, "glCompressedTexSubImage2DARB" );
		GLW_BIND( qglCompressedTexSubImage1D,    "glCompressedTexSubImage1DARB" );
		GLW_BIND( qglGetCompressedTexImage,      "glGetCompressedTexImageARB" );
	}
	if ( !GLW_HaveExtension( "GL_EXT_texture_compression_s3tc", ext ) ) {
		GLW_MissingFeature();
	}

	qglDrawRangeElementsEXT = NULL;
	if ( GLW_HaveGLVersion( 1, 2 ) ) {
		if ( r_ext_draw_range_elements->integer ) {
			GLW_BIND( qglDrawRangeElementsEXT, "glDrawRangeElements" );
			msg = "...using OpenGL 1.2 draw element range\n";
		} else {
			msg = "...ignoring OpenGL 1.2 draw element range\n";
		}
	} else if ( GLW_HaveExtension( "GL_EXT_draw_range_elements", ext ) ) {
		if ( r_ext_draw_range_elements->integer ) {
			GLW_BIND( qglDrawRangeElementsEXT, "glDrawRangeElementsEXT" );
			msg = "...using GL_EXT_draw_range_elements\n";
		} else {
			msg = "...ignoring GL_EXT_draw_range_elements\n";
		}
	} else {
		msg = "...GL_EXT_draw_range_elements not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_textureEnvAddAvailable = 0;
	if ( GLW_HaveGLVersion( 1, 3 ) ) {
		if ( r_arb_texture_env_add->integer ) {
			glConfig_textureEnvAddAvailable = 1;
			msg = "...using OpenGL 1.3 texture add environment mode\n";
		} else {
			msg = "...ignoring OpenGL 1.3 texture add environment mode\n";
		}
	} else if ( GLW_HaveExtension( "GL_EXT_texture_env_add", ext ) ) {
		if ( r_arb_texture_env_add->integer ) {
			glConfig_textureEnvAddAvailable = 1;
			msg = "...using GL_EXT_texture_env_add\n";
		} else {
			msg = "...ignoring GL_EXT_texture_env_add\n";
		}
	} else {
		msg = "...GL_EXT_texture_env_add not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_textureEnvCombine = 0;
	if ( GLW_HaveGLVersion( 1, 3 ) ) {
		if ( r_arb_texture_env_combine->integer ) {
			glConfig_textureEnvCombine = 1;
			msg = "...using OpenGL 1.3 texture combine environment mode\n";
		} else {
			msg = "...ignoring OpenGL 1.3 texture combine environment mode\n";
		}
	} else if ( GLW_HaveExtension( "GL_ARB_texture_env_combine", ext ) ) {
		if ( r_arb_texture_env_combine->integer ) {
			glConfig_textureEnvCombine = 1;
			msg = "...using GL_ARB_texture_env_combine\n";
		} else {
			msg = "...ignoring GL_ARB_texture_env_combine\n";
		}
	} else {
		msg = "...GL_ARB_texture_env_combine not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_textureEnvDot3 = 0;
	if ( GLW_HaveGLVersion( 1, 3 ) ) {
		if ( r_arb_texture_env_dot3->integer ) {
			glConfig_textureEnvDot3 = 1;
			msg = "...using OpenGL 1.3 texture dot3 environment mode\n";
		} else {
			msg = "...ignoring OpenGL 1.3 texture dot3 environment mode\n";
		}
	} else if ( GLW_HaveExtension( "GL_ARB_texture_env_dot3", ext ) ) {
		if ( r_arb_texture_env_dot3->integer && glConfig_textureEnvCombine ) {
			glConfig_textureEnvDot3 = 1;
			msg = "...using GL_ARB_texture_env_dot3\n";
		} else {
			msg = "...ignoring GL_ARB_texture_env_dot3\n";
		}
	} else {
		msg = "...GL_ARB_texture_env_dot3 not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_textureCubeMap = 0;
	if ( GLW_HaveGLVersion( 1, 3 ) ) {
		if ( r_arb_texture_cube_map->integer ) {
			glConfig_textureCubeMap = 1;
			msg = "...using OpenGL 1.3 cube map textures\n";
		} else {
			msg = "...ignoring OpenGL 1.3 cube map textures\n";
		}
	} else if ( GLW_HaveExtension( "GL_ARB_texture_cube_map", ext ) ) {
		if ( r_arb_texture_cube_map->integer ) {
			glConfig_textureCubeMap = 1;
			msg = "...using GL_ARB_texture_cube_map\n";
		} else {
			msg = "...ignoring GL_ARB_texture_cube_map\n";
		}
	} else {
		msg = "...GL_ARB_texture_cube_map not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_ARBVertexProgram = 0;
	if ( GLW_HaveExtension( "GL_ARB_vertex_program", ext ) ) {
		if ( r_arb_vertex_program->integer ) {
			glConfig_ARBVertexProgram = 1;
			GLW_InitVertexProgramExtension();
			msg = "...using GL_ARB_vertex_program\n";
		} else {
			msg = "...ignoring GL_ARB_vertex_program\n";
		}
	} else {
		msg = "...GL_ARB_vertex_program not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_NVTextureShader = 0;
	if ( GLW_HaveExtension( "GL_NV_texture_shader", ext ) ) {
		if ( r_nv_texture_shader->integer ) {
			glConfig_NVTextureShader = 1;
			msg = "...using GL_NV_texture_shader\n";
		} else {
			msg = "...ignoring GL_NV_texture_shader\n";
		}
	} else {
		msg = "...GL_NV_texture_shader not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_NVRegisterCombiners = 0;
	if ( !GLW_HaveExtension( "GL_NV_register_combiners", ext ) ) {
		ri_Printf( GLW_PRINT_ALL, "...GL_NV_register_combiners not found\n" );
	} else if ( !r_nv_register_combiners->integer ) {
		ri_Printf( GLW_PRINT_ALL, "...ignoring GL_NV_register_combiners\n" );
	} else {
		glConfig_NVRegisterCombiners = 1;
		ri_Printf( GLW_PRINT_ALL, "...using GL_NV_register_combiners\n" );
		GLW_BIND( qglCombinerParameterfvNV, "glCombinerParameterfvNV" );
		GLW_BIND( qglCombinerParameterfNV,  "glCombinerParameterfNV" );
		GLW_BIND( qglCombinerParameterivNV, "glCombinerParameterivNV" );
		GLW_BIND( qglCombinerParameteriNV,  "glCombinerParameteriNV" );
		GLW_BIND( qglCombinerInputNV,       "glCombinerInputNV" );
		GLW_BIND( qglCombinerOutputNV,      "glCombinerOutputNV" );
		GLW_BIND( qglFinalCombinerInputNV,  "glFinalCombinerInputNV" );
		GLW_BIND( qglGetCombinerInputParameterfvNV,  "glGetCombinerInputParameterfvNV" );
		GLW_BIND( qglGetCombinerInputParameterivNV,  "glGetCombinerInputParameterivNV" );
		GLW_BIND( qglGetCombinerOutputParameterfvNV, "glGetCombinerOutputParameterfvNV" );
		GLW_BIND( qglGetCombinerOutputParameterivNV, "glGetCombinerOutputParameterivNV" );
		GLW_BIND( qglGetFinalCombinerInputParameterfvNV, "glGetFinalCombinerInputParameterfvNV" );
		GLW_BIND( qglGetFinalCombinerInputParameterivNV, "glGetFinalCombinerInputParameterivNV" );

		if ( !GLW_HaveExtension( "GL_NV_register_combiners2", ext ) ) {
			ri_Printf( GLW_PRINT_ALL, "...GL_NV_register_combiners2 not found\n" );
		} else {
			combiners = 0;
			qglGetIntegerv( GL_MAX_GENERAL_COMBINERS_NV, &combiners );
			if ( combiners < 8 ) {
				ri_Printf( GLW_PRINT_ALL,
					"...ignoring GL_NV_register_combiners2 because "
					"GL_MAX_GENERAL_COMBINERS_NV is %i < 8\n", combiners );
			} else if ( r_nv_register_combiners->integer < 2 ) {
				ri_Printf( GLW_PRINT_ALL, "...ignoring GL_NV_register_combiners2\n" );
			} else {
				glConfig_NVRegisterCombiners = 2;
				ri_Printf( GLW_PRINT_ALL, "...using GL_NV_register_combiners2\n" );
				GLW_BIND( qglCombinerStageParameterfvNV, "glCombinerStageParameterfvNV" );
				GLW_BIND( qglGetCombinerStageParameterfvNV, "glGetCombinerStageParameterfvNV" );
			}
		}
	}

	GLW_BIND( qwglSwapIntervalEXT, "wglSwapIntervalEXT" );
	if ( qwglSwapIntervalEXT ) {
		ri_Printf( GLW_PRINT_ALL, "...using WGL_EXT_swap_control\n" );
		r_swapInterval->modified = qtrue;
	} else {
		ri_Printf( GLW_PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
	}

	if ( GLW_HaveGLVersion( 1, 3 ) ) {
		GLW_BIND( qglActiveTextureARB,       "glActiveTexture" );
		GLW_BIND( qglClientActiveTextureARB, "glClientActiveTexture" );
		if ( !qglActiveTextureARB || !qglClientActiveTextureARB ) {
			ri_Error( 0, va( "%s", "EXE_ERR_MULTITEX_INIT_FAIL" ) );
		}
		qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &glConfig_maxActiveTextures );
		if ( glConfig_maxActiveTextures <= 1 ) {
			ri_Error( 0, va( "%s", "EXE_ERR_MULTITEX_BAD_MAX" ) );
		}
		maxTextures = glConfig_maxActiveTextures;
		if ( maxTextures > 8 ) {
			maxTextures = 8;
			glConfig_maxActiveTextures = 8;
		}
		if ( r_maxActiveTextures->integer >= 2 &&
			 maxTextures > r_maxActiveTextures->integer ) {
			maxTextures = r_maxActiveTextures->integer;
			glConfig_maxActiveTextures = maxTextures;
		}
		/* Retail bug reproduced: the format has no conversion, yet 0x00509BE1 still pushes maxTextures. */
		ri_Printf( GLW_PRINT_ALL, "...using OpenGL 1.3 multitexture\n",
				   maxTextures );
	} else if ( GLW_HaveExtension( "GL_ARB_multitexture", ext ) ) {
		GLW_BIND( qglActiveTextureARB,       "glActiveTextureARB" );
		GLW_BIND( qglClientActiveTextureARB, "glClientActiveTextureARB" );
		if ( !qglActiveTextureARB || !qglClientActiveTextureARB ) {
			ri_Error( 0, va( "%s", "EXE_ERR_ARB_MULTITEX_INIT_FAILED" ) );
		}
		qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &glConfig_maxActiveTextures );
		if ( glConfig_maxActiveTextures <= 1 ) {
			ri_Error( 0, va( "%s", "EXE_ERR_ARB_MULTITEX_BAD_MAX" ) );
		}
		maxTextures = glConfig_maxActiveTextures;
		if ( maxTextures > 8 ) {
			maxTextures = 8;
			glConfig_maxActiveTextures = 8;
		}
		if ( r_maxActiveTextures->integer >= 2 &&
			 maxTextures > r_maxActiveTextures->integer ) {
			maxTextures = r_maxActiveTextures->integer;
			glConfig_maxActiveTextures = maxTextures;
		}
		ri_Printf( GLW_PRINT_ALL,
			"...using GL_ARB_multitexture with %i max textures\n", maxTextures );
	} else {
		GLW_MissingFeature();
	}

	if ( GLW_HaveExtension( "GL_EXT_compiled_vertex_array", ext ) ) {
		if ( r_ext_compiled_vertex_array->integer ) {
			ri_Printf( GLW_PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			GLW_BIND( qglLockArraysEXT,   "glLockArraysEXT" );
			GLW_BIND( qglUnlockArraysEXT, "glUnlockArraysEXT" );
		} else {
			ri_Printf( GLW_PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	} else {
		ri_Printf( GLW_PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

	glConfig_rescaleNormal = 0;
	if ( GLW_HaveGLVersion( 1, 2 ) ) {
		if ( r_ext_rescale_normal->integer ) {
			glConfig_rescaleNormal = 1;
			ri_Printf( GLW_PRINT_ALL, "...using OpenGL 1.2 normal rescaling\n" );
		} else {
			ri_Printf( GLW_PRINT_ALL, "...ignoring OpenGL 1.2 normal rescaling\n" );
		}
	} else if ( GLW_HaveExtension( "GL_EXT_rescale_normal", ext ) ) {
		if ( r_ext_rescale_normal->integer ) {
			ri_Printf( GLW_PRINT_ALL, "...using GL_EXT_rescale_normal\n" );
			glConfig_rescaleNormal = 1;
		} else {
			ri_Printf( GLW_PRINT_ALL, "...ignoring GL_EXT_rescale_normal\n" );
		}
	} else {
		ri_Printf( GLW_PRINT_ALL, "...GL_EXT_rescale_normal not found\n" );
	}

	if ( GLW_HaveExtension( "GL_ATI_pn_triangles", ext ) ) {
		if ( r_ati_pntriangles->integer ) {
			ri_Printf( GLW_PRINT_ALL, "...using GL_ATI_pn_triangles\n" );
			GLW_BIND( qglPNTrianglesiATI, "glPNTrianglesiATI" );
			GLW_BIND( qglPNTrianglesfATI, "glPNTrianglesfATI" );
		} else {
			ri_Printf( GLW_PRINT_ALL, "...ignoring GL_ATI_pn_triangles\n" );
		}
	} else {
		ri_Printf( GLW_PRINT_ALL, "...GL_ATI_pn_triangles not found\n" );
		ri_Cvar_Set( "r_ati_pntriangles", "0" );
	}

	if ( GLW_HaveExtension( "GL_ARB_vertex_buffer_object", ext ) ) {
		if ( r_arb_vertex_buffer_object->integer ) {
			GLW_BIND( qglBindBufferARB,     "glBindBufferARB" );
			GLW_BIND( qglDeleteBuffersARB,  "glDeleteBuffersARB" );
			GLW_BIND( qglGenBuffersARB,     "glGenBuffersARB" );
			GLW_BIND( qglIsBufferARB,       "glIsBufferARB" );
			GLW_BIND( qglBufferDataARB,     "glBufferDataARB" );
			GLW_BIND( qglBufferSubDataARB,  "glBufferSubDataARB" );
			GLW_BIND( qglGetBufferSubDataARB, "glGetBufferSubDataARB" );
			GLW_BIND( qglMapBufferARB,      "glMapBufferARB" );
			GLW_BIND( qglUnmapBufferARB,    "glUnmapBufferARB" );
			GLW_BIND( qglGetBufferParameterivARB, "glGetBufferParameterivARB" );
			GLW_BIND( qglGetBufferPointervARB,    "glGetBufferPointervARB" );
			glConfig_ARBVertexBufferObject = 1;
			msg = "...using GL_ARB_vertex_buffer_object\n";
		} else {
			msg = "...ignoring GL_ARB_vertex_buffer_object\n";
		}
	} else {
		msg = "...GL_ARB_vertex_buffer_object not found\n";
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	if ( GLW_HaveExtension( "GL_NV_fog_distance", ext ) ) {
		if ( r_nv_fog_dist->integer ) {
			glConfig_NVFogAvailable = 1;
			ri_Printf( GLW_PRINT_ALL, "...using GL_NV_fog_distance\n" );
		} else {
			ri_Printf( GLW_PRINT_ALL, "...ignoring GL_NV_fog_distance\n" );
			qglFogi( GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV );
		}
		ri_Cvar_Set( "r_nv_fog_available", "1" );
	} else {
		ri_Printf( GLW_PRINT_ALL, "...GL_NV_fog_distance not found\n" );
		ri_Cvar_Set( "r_nv_fog_dist", "0" );
		ri_Cvar_Set( "r_nv_fog_available", "0" );
	}

	msg = "...GL_NV_vertex_array_range not found\n";
	if ( GLW_HaveExtension( "GL_NV_vertex_array_range", ext ) ||
		 GLW_HaveExtension( "GL_NV_vertex_array_range2", ext ) ) {
		if ( !r_nv_vertex_array_range->integer || glConfig_ARBVertexBufferObject ) {
			msg = "...ignoring GL_NV_vertex_array_range";
		} else {
			GLW_BIND( qglFlushVertexArrayRangeNV, "glFlushVertexArrayRangeNV" );
			GLW_BIND( qglVertexArrayRangeNV,      "glVertexArrayRangeNV" );
			GLW_BIND( qwglAllocateMemoryNV,       "wglAllocateMemoryNV" );
			GLW_BIND( qwglFreeMemoryNV,           "wglFreeMemoryNV" );
			glConfig_NVVertexArrayRange = 1;
			ri_Printf( GLW_PRINT_ALL, "...using GL_NV_vertex_array_range\n" );

			if ( GLW_HaveExtension( "GL_NV_vertex_array_range2", ext ) ) {
				if ( r_nv_vertex_array_range->integer == 2 ) {
					glConfig_NVVertexArrayRange = 2;
					msg = "...using GL_NV_vertex_array_range2\n";
				} else {
					msg = "...ignoring GL_NV_vertex_array_range2\n";
				}
			} else {
				msg = "...GL_NV_vertex_array_range2 not found\n";
			}
		}
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	msg = "...GL_NV_fence not found\n";
	if ( GLW_HaveExtension( "GL_NV_fence", ext ) ) {
		if ( !r_nv_fence->integer || glConfig_ARBVertexBufferObject ) {
			msg = "...ignoring GL_NV_fence\n";
		} else {
			GLW_BIND( qglDeleteFencesNV, "glDeleteFencesNV" );
			GLW_BIND( qglGenFencesNV,    "glGenFencesNV" );
			GLW_BIND( qglIsFenceNV,      "glIsFenceNV" );
			GLW_BIND( qglTestFenceNV,    "glTestFenceNV" );
			GLW_BIND( qglGetFenceivNV,   "glGetFenceivNV" );
			GLW_BIND( qglFinishFenceNV,  "glFinishFenceNV" );
			GLW_BIND( qglSetFenceNV,     "glSetFenceNV" );
			glConfig_NVFenceAvailable = 1;
			msg = "...using GL_NV_fence\n";
		}
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	msg = "...GL_ATI_vertex_array_object not found\n";
	if ( GLW_HaveExtension( "GL_ATI_vertex_array_object", ext ) ) {
		if ( !r_ati_vertex_array_object->integer ||
			 glConfig_ARBVertexBufferObject || glConfig_NVVertexArrayRange ) {
			msg = "...ignoring GL_ATI_vertex_array_object\n";
		} else {
			GLW_BIND( qglNewObjectBufferATI,    "glNewObjectBufferATI" );
			GLW_BIND( qglIsObjectBufferATI,     "glIsObjectBufferATI" );
			GLW_BIND( qglUpdateObjectBufferATI, "glUpdateObjectBufferATI" );
			GLW_BIND( qglGetObjectBufferfvATI,  "glGetObjectBufferfvATI" );
			GLW_BIND( qglGetObjectBufferivATI,  "glGetObjectBufferivATI" );
			GLW_BIND( qglFreeObjectBufferATI,   "glFreeObjectBufferATI" );
			GLW_BIND( qglArrayObjectATI,        "glArrayObjectATI" );
			GLW_BIND( qglGetArrayObjectfvATI,   "glGetArrayObjectfvATI" );
			GLW_BIND( qglGetArrayObjectivATI,   "glGetArrayObjectivATI" );
			GLW_BIND( qglVariantArrayObjectATI, "glVariantArrayObjectATI" );
			GLW_BIND( qglGetVariantArrayObjectfvATI, "glGetVariantArrayObjectfvATI" );
			GLW_BIND( qglGetVariantArrayObjectivATI, "glGetVariantArrayObjectivATI" );
			glConfig_ATIVertexArrayObject = 1;
			ri_Printf( GLW_PRINT_ALL, "...using GL_ATI_vertex_array_object\n" );

			if ( GLW_HaveExtension( "GL_ATI_element_array", ext ) ) {
				if ( r_ati_element_array->integer ) {
					GLW_BIND( qglElementPointerATI,       "glElementPointerATI" );
					GLW_BIND( qglDrawElementArrayATI,     "glDrawElementArrayATI" );
					GLW_BIND( qglDrawRangeElementArrayATI, "glDrawRangeElementArrayATI" );
					glConfig_ATIElementArray = 1;
					msg = "...using GL_ATI_element_array\n";
				} else {
					msg = "...ignoring GL_ATI_element_array\n";
				}
			} else {
				msg = "...GL_ATI_element_array not found\n";
			}
		}
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	glConfig_ATIFragmentShader = 0;
	msg = "...GL_ATI_fragment_shader not found\n";
	if ( GLW_HaveExtension( "GL_ATI_fragment_shader", ext ) ) {
		if ( r_ati_fragment_shader->integer ) {
			GLW_BIND( qglGenFragmentShadersATI,   "glGenFragmentShadersATI" );
			GLW_BIND( qglBindFragmentShaderATI,   "glBindFragmentShaderATI" );
			GLW_BIND( qglDeleteFragmentShaderATI, "glDeleteFragmentShaderATI" );
			GLW_BIND( qglBeginFragmentShaderATI,  "glBeginFragmentShaderATI" );
			GLW_BIND( qglEndFragmentShaderATI,    "glEndFragmentShaderATI" );
			GLW_BIND( qglPassTexCoordATI,         "glPassTexCoordATI" );
			GLW_BIND( qglSampleMapATI,            "glSampleMapATI" );
			GLW_BIND( qglColorFragmentOp1ATI,     "glColorFragmentOp1ATI" );
			GLW_BIND( qglColorFragmentOp2ATI,     "glColorFragmentOp2ATI" );
			GLW_BIND( qglColorFragmentOp3ATI,     "glColorFragmentOp3ATI" );
			GLW_BIND( qglAlphaFragmentOp1ATI,     "glAlphaFragmentOp1ATI" );
			GLW_BIND( qglAlphaFragmentOp2ATI,     "glAlphaFragmentOp2ATI" );
			GLW_BIND( qglAlphaFragmentOp3ATI,     "glAlphaFragmentOp3ATI" );
			GLW_BIND( qglSetFragmentShaderConstantATI, "glSetFragmentShaderConstantATI" );
			glConfig_ATIFragmentShader = 1;
			msg = "...using GL_ATI_fragment_shader\n";
		} else {
			msg = "...ignoring GL_ATI_fragment_shader\n";
		}
	}
	ri_Printf( GLW_PRINT_ALL, msg );

	/* Both branches disable anisotropy: retail 0x0050A427 and 0x0050A44E each Cvar_Set it to "0". */
	if ( GLW_HaveExtension( "GL_EXT_texture_filter_anisotropic", ext ) ) {
		glConfig_maxAnisotropy = 0;
		ri_Printf( GLW_PRINT_ALL,
			"...ignoring GL_EXT_texture_filter_anisotropic\n" );
	}
	ri_Cvar_Set( "r_ext_texture_filter_anisotropic", "0" );
}

/* ---- GLW_CheckOSVersion  0x0050A470 ---- */
static qboolean GLW_CheckOSVersion( void ) {
	return qtrue;
}

/* ---- PrintCDSError  0x00508580 ---- */
static void PrintCDSError( int value ) {
	const char	*s;

	switch ( value ) {
	case DISP_CHANGE_RESTART:		s = "restart required";	break;
	case DISP_CHANGE_BADPARAM:		s = "bad param";		break;
	case DISP_CHANGE_BADFLAGS:		s = "bad flags";		break;
	case DISP_CHANGE_FAILED:		s = "failed";			break;
	case DISP_CHANGE_BADMODE:		s = "bad mode";			break;
	case DISP_CHANGE_NOTUPDATED:	s = "not updated";		break;
	default:						s = "unknown";			break;
	}
	ri_Printf( GLW_PRINT_ALL, "%s\n", s );
}

extern int R_GetModeInfo( int mode, int *height, float *windowAspect, int *width );

/* ---- GLW_CreatePFD  0x00507DD0 ----  VERIFIED */
static void GLW_CreatePFD( PIXELFORMATDESCRIPTOR *pPFD, int colorbits,
						   int depthbits, int stencilbits, qboolean stereo ) {
	PIXELFORMATDESCRIPTOR	src;

	memset( &src, 0, sizeof( src ) );

	src.nSize		= sizeof( PIXELFORMATDESCRIPTOR );
	src.nVersion	= 1;
	src.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	src.iPixelType	= PFD_TYPE_RGBA;
	src.cColorBits	= (BYTE)colorbits;
	src.cDepthBits	= (BYTE)depthbits;
	src.cStencilBits = (BYTE)stencilbits;
	src.iLayerType	= PFD_MAIN_PLANE;

	if ( stereo ) {
		ri_Printf( GLW_PRINT_ALL, "...attempting to use stereo\n" );
		src.dwFlags |= PFD_STEREO;
		glConfig_stereoEnabled = qtrue;
	} else {
		glConfig_stereoEnabled = qfalse;
	}

	*pPFD = src;
}

#define MAX_PFDS 1024

/* ---- GLW_ChoosePFD  0x00507AA0 ----  VERIFIED */
static int GLW_ChoosePFD( HDC hDC, PIXELFORMATDESCRIPTOR *pPFD ) {
	PIXELFORMATDESCRIPTOR	pfds[MAX_PFDS + 1];
	int		maxPFD;
	int		i;
	int		bestMatch;

	ri_Printf( GLW_PRINT_ALL, "...GLW_ChoosePFD( %d, %d, %d )\n",
		(int)pPFD->cColorBits, (int)pPFD->cDepthBits, (int)pPFD->cStencilBits );

	bestMatch = 0;

	maxPFD = DescribePixelFormat( hDC, 1, sizeof( PIXELFORMATDESCRIPTOR ), &pfds[0] );
	if ( maxPFD > MAX_PFDS ) {
		ri_Printf( GLW_PRINT_WARNING, "...numPFDs > MAX_PFDS (%d > %d)\n",
			maxPFD, MAX_PFDS );
		maxPFD = MAX_PFDS;
	}
	ri_Printf( GLW_PRINT_ALL, "...%d PFDs found\n", maxPFD - 1 );

	for ( i = 1; i <= maxPFD; i++ ) {
		DescribePixelFormat( hDC, i, sizeof( PIXELFORMATDESCRIPTOR ), &pfds[i] );
	}

	for ( i = 1; i <= maxPFD; i++ ) {
		PIXELFORMATDESCRIPTOR	*p = &pfds[i];

		if ( ( p->dwFlags & PFD_GENERIC_FORMAT ) != 0 &&
			 !RCVAR( glw_allowSoftwareGL )->integer ) {
			if ( RCVAR( r_verbose )->integer ) {
				ri_Printf( GLW_PRINT_ALL,
					"...PFD %d rejected, software acceleration\n", i );
			}
			continue;
		}

		if ( RCVAR( r_verbose )->integer ) {
			ri_Printf( GLW_PRINT_ALL,
				"ChoosePFD: format %i: %i color %i depth %i stencil\n",
				i, (int)p->cColorBits, (int)p->cDepthBits, (int)p->cStencilBits );
		}

		if ( p->iPixelType != PFD_TYPE_RGBA ) {
			if ( RCVAR( r_verbose )->integer ) {
				ri_Printf( GLW_PRINT_ALL, "...PFD %d rejected, not RGBA\n", i );
			}
			continue;
		}

		if ( ( p->dwFlags & pPFD->dwFlags ) != pPFD->dwFlags ) {
			if ( RCVAR( r_verbose )->integer ) {
				ri_Printf( GLW_PRINT_ALL,
					"...PFD %d rejected, improper flags (%x instead of %x)\n",
					i, p->dwFlags, pPFD->dwFlags );
			}
			continue;
		}

		if ( p->cDepthBits < 15 ) {
			if ( RCVAR( r_verbose )->integer ) {
				ri_Printf( GLW_PRINT_ALL,
					"...PFD %d rejected, insufficient depth bits (%d instead of %d)\n",
					i, (int)p->cDepthBits, (int)pPFD->cDepthBits );
			}
			continue;
		}

		if ( p->cStencilBits < 4 && pPFD->cStencilBits != 0 ) {
			if ( RCVAR( r_verbose )->integer ) {
				ri_Printf( GLW_PRINT_ALL,
					"...PFD %d rejected, insufficient stencil bits (%d instead of %d)\n",
					i, (int)p->cStencilBits, (int)pPFD->cStencilBits );
			}
			continue;
		}

		if ( !bestMatch ) {
			bestMatch = i;
			continue;
		}

		if ( ( pPFD->dwFlags & PFD_DOUBLEBUFFER ) != 0 &&
			 ( ( p->dwFlags & PFD_DOUBLEBUFFER ) !=
			   ( pfds[bestMatch].dwFlags & PFD_DOUBLEBUFFER ) ) ) {
			bestMatch = i;
			continue;
		}

		if ( pfds[bestMatch].cColorBits != pPFD->cColorBits ) {
			if ( p->cColorBits == pPFD->cColorBits ||
				 p->cColorBits > pfds[bestMatch].cColorBits ) {
				bestMatch = i;
				continue;
			}
		}
		if ( pfds[bestMatch].cDepthBits != pPFD->cDepthBits ) {
			if ( p->cDepthBits == pPFD->cDepthBits ||
				 p->cDepthBits > pfds[bestMatch].cDepthBits ) {
				bestMatch = i;
				continue;
			}
		}
		if ( pfds[bestMatch].cStencilBits != pPFD->cStencilBits ) {
			if ( p->cStencilBits == pPFD->cStencilBits ||
				 ( p->cStencilBits > pfds[bestMatch].cStencilBits &&
				   pPFD->cStencilBits != 0 ) ) {
				bestMatch = i;
				continue;
			}
		}
	}

	if ( !bestMatch ) {
		return 0;
	}

	if ( ( pfds[bestMatch].dwFlags & PFD_GENERIC_FORMAT ) != 0 ) {
		if ( !RCVAR( glw_allowSoftwareGL )->integer ) {
			ri_Printf( GLW_PRINT_ALL, "...no hardware acceleration found\n" );
			return 0;
		}
		ri_Printf( GLW_PRINT_ALL, "...using software emulation\n" );
	} else if ( ( pfds[bestMatch].dwFlags & PFD_GENERIC_ACCELERATED ) != 0 ) {
		ri_Printf( GLW_PRINT_ALL, "...MCD acceleration found\n" );
	} else {
		ri_Printf( GLW_PRINT_ALL, "...hardware acceleration found\n" );
	}

	*pPFD = pfds[bestMatch];
	return bestMatch;
}

/* ---- GLW_GetString  0x00507EA0 ----  VERIFIED */
static qboolean GLW_GetString( void ) {
	if ( !QGL_GetString( GL_VENDOR ) ) {
		ri_Printf( GLW_PRINT_ALL, "glGetString(GL_VENDOR) returned NULL\n" );
		return qfalse;
	}
	if ( !QGL_GetString( GL_RENDERER ) ) {
		ri_Printf( GLW_PRINT_ALL, "glGetString(GL_RENDERER) returned NULL\n" );
		return qfalse;
	}
	if ( !QGL_GetString( GL_VERSION ) ) {
		ri_Printf( GLW_PRINT_ALL, "glGetString(GL_VERSION) returned NULL\n" );
		return qfalse;
	}
	if ( !QGL_GetString( GL_EXTENSIONS ) ) {
		ri_Printf( GLW_PRINT_ALL, "glGetString(GL_EXTENSIONS) returned NULL\n" );
		return qfalse;
	}
	return qtrue;
}

/* ---- GLW_MakeContext  0x00507F30 ----  VERIFIED */
static int GLW_MakeContext( PIXELFORMATDESCRIPTOR *pPFD ) {
	int		pixelformat;

	if ( !glw_pixelFormatSet ) {
		pixelformat = GLW_ChoosePFD( glw_hDC, pPFD );
		if ( !pixelformat ) {
			ri_Printf( GLW_PRINT_ALL, "...GLW_ChoosePFD failed\n" );
			return 1;
		}
		ri_Printf( GLW_PRINT_ALL, "...PIXELFORMAT %d selected\n", pixelformat );

		DescribePixelFormat( glw_hDC, pixelformat,
			sizeof( PIXELFORMATDESCRIPTOR ), pPFD );

		if ( !SetPixelFormat( glw_hDC, pixelformat, pPFD ) ) {
			ri_Printf( GLW_PRINT_ALL, "...SetPixelFormat failed\n" );
			return 1;
		}
		glw_pixelFormatSet = qtrue;
	}

	if ( glw_hGLRC ) {
		return 0;
	}

	ri_Printf( GLW_PRINT_ALL, "...creating GL context: " );
	glw_hGLRC = qwglCreateContext( glw_hDC );
	if ( !glw_hGLRC ) {
		ri_Printf( GLW_PRINT_ALL, "failed\n" );
		return 2;
	}
	ri_Printf( GLW_PRINT_ALL, "succeeded\n" );

	ri_Printf( GLW_PRINT_ALL, "...making context current: " );
	if ( !qwglMakeCurrent( glw_hDC, glw_hGLRC ) ) {
		qwglDeleteContext( glw_hGLRC );
		glw_hGLRC = NULL;
		ri_Printf( GLW_PRINT_ALL, "failed\n" );
		return 2;
	}
	ri_Printf( GLW_PRINT_ALL, "succeeded\n" );

	if ( GLW_GetString() ) {
		return 0;
	}

	qwglMakeCurrent( NULL, NULL );
	qwglDeleteContext( glw_hGLRC );
	glw_hGLRC = NULL;
	return 2;
}

/* ---- GLW_InitDriver  0x005080A0 ----  VERIFIED */
static qboolean GLW_InitDriver( int colorbits ) {
	int		tpixelFormat;
	int		depthbits, stencilbits;
	int		maxTextureSize;

	ri_Printf( GLW_PRINT_ALL, "Initializing OpenGL driver\n" );

	if ( !glw_hDC ) {
		ri_Printf( GLW_PRINT_ALL, "...getting DC: " );
		glw_hDC = GetDC( (HWND)g_wv_hWnd );
		if ( !glw_hDC ) {
			ri_Printf( GLW_PRINT_ALL, "failed\n" );
			return qfalse;
		}
		ri_Printf( GLW_PRINT_ALL, "succeeded\n" );
	}

	if ( !colorbits ) {
		colorbits = glw_desktopBitsPixel;
	}

	depthbits = RCVAR( r_depthbits )->integer;
	if ( !depthbits ) {
		depthbits = ( colorbits > 16 ) ? 24 : 16;
	}
	stencilbits = RCVAR( r_stencilbits )->integer;
	if ( depthbits < 24 ) {
		stencilbits = 0;
	}

	if ( !glw_pixelFormatSet ) {
		GLW_CreatePFD( &glw_pfd, colorbits, depthbits, stencilbits, qfalse );
		tpixelFormat = GLW_MakeContext( &glw_pfd );
		if ( tpixelFormat ) {
			if ( tpixelFormat == 2 ) {
				ri_Printf( GLW_PRINT_WARNING, "...failed hard\n" );
				return qfalse;
			}
			if ( RCVAR( r_colorbits )->integer == glw_desktopBitsPixel &&
				 stencilbits == 0 ) {
				goto fail;
			}
			if ( colorbits > glw_desktopBitsPixel ) {
				colorbits = glw_desktopBitsPixel;
			}
			GLW_CreatePFD( &glw_pfd, colorbits, depthbits, 0, qfalse );
			if ( GLW_MakeContext( &glw_pfd ) ) {
				goto fail;
			}
		}
	}

	QGL_GetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize );
	if ( dwStyle > maxTextureSize || dwExStyle > maxTextureSize ) {
		if ( glw_hGLRC ) {
			qwglMakeCurrent( NULL, NULL );
			qwglDeleteContext( glw_hGLRC );
			glw_hGLRC = NULL;
		}
		if ( glw_hDC ) {
			ReleaseDC( (HWND)g_wv_hWnd, glw_hDC );
			glw_hDC = NULL;
		}
		glw_pixelFormatSet = qfalse;
		ri_Printf( GLW_PRINT_ALL,
			"video card cannot set mode %ix%i because GL_MAX_TEXTURE_SIZE is only %i\n",
			dwStyle, dwExStyle, maxTextureSize );
		return qfalse;
	}

	glConfig_colorBits   = glw_pfd.cColorBits;
	glConfig_depthBits   = glw_pfd.cDepthBits;
	glConfig_stencilBits = glw_pfd.cStencilBits;
	return qtrue;

fail:
	if ( glw_hDC ) {
		ReleaseDC( (HWND)g_wv_hWnd, glw_hDC );
		glw_hDC = NULL;
	}
	ri_Printf( GLW_PRINT_ALL, "...failed to find an appropriate PIXELFORMAT\n" );
	return qfalse;
}

/* ---- GLW_CreateWindow  0x005082E0 ----  VERIFIED */
static qboolean GLW_CreateWindow( int width, int height, int colorbits,
								  qboolean cdsFullscreen ) {
	RECT		r;
	WNDCLASS	wc;
	DWORD		stylebits, exstyle;
	int			x, y, w, h;

	if ( !s_classRegistered ) {
		memset( &wc, 0, sizeof( wc ) );

		wc.style			= 0;
		wc.lpfnWndProc		= glw_wndproc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= (HINSTANCE)g_wv_hInstance;
		wc.hIcon			= LoadIcon( (HINSTANCE)g_wv_hInstance,
										MAKEINTRESOURCE( 1 ) );
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= CreateSolidBrush( RGB( 0, 0, 0 ) );
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= WINDOW_CLASS_NAME;

		if ( !RegisterClass( &wc ) ) {
			ri_Error( 0, "EXE_ERR_COULDNT_REGISTER_WINDOW" );
		}
		s_classRegistered = qtrue;
		ri_Printf( GLW_PRINT_ALL, "...registered window class\n" );
	}

	r.left	 = 0;
	r.top	 = 0;
	r.right	 = width;
	r.bottom = height;

	if ( cdsFullscreen ) {
		exstyle		= WS_EX_LEFT;
		stylebits	= WS_POPUP | WS_VISIBLE | WS_SYSMENU;
	} else {
		exstyle		= 0;
		stylebits	= WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE |
					  WS_SYSMENU | WS_MINIMIZEBOX;
		AdjustWindowRect( &r, 0x10C80000, FALSE );
	}

	w = r.right - r.left;
	h = r.bottom - r.top;

	if ( cdsFullscreen ) {
		x = 0;
		y = 0;
	} else {
		x = RCVAR( ri_Cvar_Get( "vid_xpos", "", 0 ) )->integer;
		y = RCVAR( ri_Cvar_Get( "vid_ypos", "", 0 ) )->integer;

		if ( x < 0 ) { x = 0; }
		if ( y < 0 ) { y = 0; }

		if ( w < glw_desktopWidth && h < glw_desktopHeight ) {
			if ( x + w > glw_desktopWidth )  { x = glw_desktopWidth  - w; }
			if ( y + h > glw_desktopHeight ) { y = glw_desktopHeight - h; }
		}
	}

	if ( g_wv_hWnd ) {
		ri_Printf( GLW_PRINT_ALL,
			"...window already present, CreateWindowEx skipped\n" );
		MoveWindow( (HWND)g_wv_hWnd, x, y, w, h, FALSE );
		ri_Printf( GLW_PRINT_ALL, "...moved window to %d,%d (%dx%d)\n",
			x, y, w, h );
	} else {
		g_wv_hWnd = (int)CreateWindowEx( exstyle, WINDOW_CLASS_NAME,
			WINDOW_CLASS_NAME, stylebits, x, y, w, h,
			NULL, NULL, (HINSTANCE)g_wv_hInstance, NULL );

		if ( !g_wv_hWnd ) {
			ri_Error( 0, "EXE_ERR_COULDNT_CREATE_WINDOW" );
		}
		ShowWindow( (HWND)g_wv_hWnd, SW_SHOW );
		UpdateWindow( (HWND)g_wv_hWnd );
		ri_Printf( GLW_PRINT_ALL, "...created window@%d,%d (%dx%d)\n",
			x, y, w, h );
	}

	{
		RECT cr;

		if ( GetClientRect( (HWND)g_wv_hWnd, &cr ) ) {
			int cw = cr.right  - cr.left;
			int ch = cr.bottom - cr.top;

			if ( cw > 0 && ch > 0 && ( cw != width || ch != height ) ) {
				w += width  - cw;
				h += height - ch;
				SetWindowPos( (HWND)g_wv_hWnd, NULL, 0, 0, w, h,
					SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

				GetClientRect( (HWND)g_wv_hWnd, &cr );
				ri_Printf( GLW_PRINT_ALL,
					"...client was %dx%d; window resized to %dx%d, client now %dx%d\n",
					cw, ch, w, h,
					cr.right - cr.left, cr.bottom - cr.top );
			}
		}
	}

	if ( !GLW_InitDriver( colorbits ) ) {
		ShowWindow( (HWND)g_wv_hWnd, SW_HIDE );
		DestroyWindow( (HWND)g_wv_hWnd );
		g_wv_hWnd = 0;
		return qfalse;
	}

	SetForegroundWindow( (HWND)g_wv_hWnd );
	SetFocus( (HWND)g_wv_hWnd );
	if ( g_splashWnd ) {
		ShowWindow( (HWND)g_splashWnd, SW_HIDE );
	}
	return qtrue;
}

/* ---- GLW_SetMode  0x00508630 ----  VERIFIED */
static int GLW_SetMode( int mode, int colorbits, qboolean cdsFullscreen ) {
	HDC			hDC;
	DEVMODE		dm;
	int			cdsRet;

	ri_Printf( GLW_PRINT_ALL, "...setting mode %d:", mode );

	if ( !R_GetModeInfo( mode, &dwExStyle, &glConfig_windowAspect, &dwStyle ) ) {
		ri_Printf( GLW_PRINT_ALL, " invalid mode\n" );
		return RSERR_INVALID_MODE;
	}
	ri_Printf( GLW_PRINT_ALL, " %d %d %s\n", dwStyle, dwExStyle,
		cdsFullscreen
			? ( RCVAR( r_displayRefresh )->integer
					? va( "FS (%i Hz)", RCVAR( r_displayRefresh )->integer )
					: "FS" )
			: "W" );

	hDC = GetDC( GetDesktopWindow() );
	glw_desktopBitsPixel = GetDeviceCaps( hDC, BITSPIXEL );
	glw_desktopWidth     = GetDeviceCaps( hDC, HORZRES );
	glw_desktopHeight    = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow(), hDC );

	if ( glw_desktopBitsPixel < 15 || glw_desktopBitsPixel == 24 ) {
		if ( !colorbits || ( !cdsFullscreen && colorbits < 15 ) ) {
		} else {
			const char *title = (const char *)SEH_GetLocalizedString_m(
									"WIN_COLORDEPTH_WARN_TITLE" );
			const char *body  = (const char *)SEH_GetLocalizedString_m(
									"WIN_COLORDEPTH_WARN_BODY" );
			if ( MessageBox( NULL, body, title,
					MB_YESNO | MB_ICONEXCLAMATION ) != IDYES ) {
				return RSERR_INVALID_MODE;
			}
		}
	}

	if ( !cdsFullscreen ) {
		if ( glw_cdsFullscreen ) {
			ChangeDisplaySettings( NULL, 0 );
		}
		glw_cdsFullscreen = qfalse;
		if ( !GLW_CreateWindow( dwStyle, dwExStyle, colorbits, qfalse ) ) {
			return RSERR_INVALID_MODE;
		}
		goto record;
	}

	memset( &dm, 0, sizeof( dm ) );
	dm.dmSize		= sizeof( dm );
	dm.dmPelsWidth	= dwStyle;
	dm.dmPelsHeight	= dwExStyle;
	dm.dmFields		= DM_PELSWIDTH | DM_PELSHEIGHT;

	if ( RCVAR( r_displayRefresh )->integer ) {
		dm.dmDisplayFrequency = RCVAR( r_displayRefresh )->integer;
		dm.dmFields |= DM_DISPLAYFREQUENCY;
	}
	if ( colorbits ) {
		dm.dmBitsPerPel = colorbits;
		dm.dmFields |= DM_BITSPERPEL;
		ri_Printf( GLW_PRINT_ALL, "...using colorbits of %d\n", colorbits );
	} else {
		ri_Printf( GLW_PRINT_ALL, "...using desktop display depth of %d\n",
			glw_desktopBitsPixel );
	}

	if ( glw_cdsFullscreen ) {
		ri_Printf( GLW_PRINT_ALL,
			"...already fullscreen, avoiding redundant CDS\n" );
		if ( !GLW_CreateWindow( dwStyle, dwExStyle, colorbits, qtrue ) ) {
			ri_Printf( GLW_PRINT_ALL, "...restoring display settings\n" );
			ChangeDisplaySettings( NULL, 0 );
			return RSERR_INVALID_MODE;
		}
		goto record;
	}

	ri_Printf( GLW_PRINT_ALL, "...calling CDS: " );
	cdsRet = ChangeDisplaySettings( &dm, CDS_FULLSCREEN );
	if ( cdsRet == DISP_CHANGE_SUCCESSFUL ) {
		ri_Printf( GLW_PRINT_ALL, "ok\n" );
		if ( !GLW_CreateWindow( dwStyle, dwExStyle, colorbits, qtrue ) ) {
			ri_Printf( GLW_PRINT_ALL, "...restoring display settings\n" );
			ChangeDisplaySettings( NULL, 0 );
			return RSERR_INVALID_MODE;
		}
		glw_cdsFullscreen = qtrue;
		goto record;
	}

	ri_Printf( GLW_PRINT_ALL, "failed, " );
	PrintCDSError( cdsRet );
	ri_Printf( GLW_PRINT_ALL, "...trying next higher resolution:" );
	{
		DEVMODE		devmode;
		int			modeNum = 0;

		memset( &devmode, 0, sizeof( devmode ) );
		devmode.dmSize = sizeof( devmode );

		if ( EnumDisplaySettings( NULL, 0, &devmode ) ) {
			while ( devmode.dmPelsWidth  < (DWORD)dwStyle ||
					devmode.dmPelsHeight < (DWORD)dwExStyle ||
					devmode.dmBitsPerPel < 15 ) {
				if ( !EnumDisplaySettings( NULL, ++modeNum, &devmode ) ) {
					goto cdsfail;
				}
			}
			cdsRet = ChangeDisplaySettings( &devmode, CDS_FULLSCREEN );
			if ( cdsRet == DISP_CHANGE_SUCCESSFUL ) {
				ri_Printf( GLW_PRINT_ALL, " ok\n" );
				if ( !GLW_CreateWindow( dwStyle, dwExStyle, colorbits, qtrue ) ) {
					ri_Printf( GLW_PRINT_ALL, "...restoring display settings\n" );
					ChangeDisplaySettings( NULL, 0 );
					return RSERR_INVALID_MODE;
				}
				glw_cdsFullscreen = qtrue;
				goto record;
			}
		}
	}

cdsfail:
	ri_Printf( GLW_PRINT_ALL, " failed, " );
	PrintCDSError( cdsRet );
	ri_Printf( GLW_PRINT_ALL, "...restoring display settings\n" );
	ChangeDisplaySettings( NULL, 0 );
	glw_cdsFullscreen = qfalse;
	glConfig_isFullscreen = qfalse;

	if ( !GLW_CreateWindow( dwStyle, dwExStyle, colorbits, qfalse ) ) {
		return RSERR_INVALID_MODE;
	}
	if ( g_wv_hWnd ) {
		if ( glw_hGLRC ) {
			qwglMakeCurrent( NULL, NULL );
			qwglDeleteContext( glw_hGLRC );
			glw_hGLRC = NULL;
		}
		ShowWindow( (HWND)g_wv_hWnd, SW_HIDE );
		DestroyWindow( (HWND)g_wv_hWnd );
		g_wv_hWnd = 0;
		glw_pixelFormatSet = qfalse;
	}
	return RSERR_INVALID_FULLSCREEN;

record:
	{
		DEVMODE		cur;
		memset( &cur, 0, sizeof( cur ) );
		cur.dmSize = sizeof( cur );
		if ( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &cur ) ) {
			glConfig_displayFrequency = cur.dmDisplayFrequency;
		}
	}
	glConfig_isFullscreen = cdsFullscreen;
	return RSERR_OK;
}

/* ---- GLW_StartDriverAndSetMode  0x0050A510 ----  VERIFIED */
static qboolean GLW_StartDriverAndSetMode( const char *drivername ) {
	char	buffer[1024];
	int		err;
	char	*p;

	strncpy( buffer, drivername, sizeof( buffer ) - 1 );
	buffer[sizeof( buffer ) - 1] = '\0';
	for ( p = buffer; *p; p++ ) {
		*p = (char)tolower( (unsigned char)*p );
	}

	if ( !QCL_Init( buffer ) ) {
		QGL_Shutdown();
		return qfalse;
	}

	err = GLW_SetMode( RCVAR( r_mode )->integer,
					   RCVAR( r_colorbits )->integer,
					   RCVAR( r_fullscreen )->integer );
	switch ( err ) {
	case RSERR_INVALID_FULLSCREEN:
		ri_Printf( GLW_PRINT_ALL,
			"...WARNING: fullscreen unavailable in this mode\n" );
		QGL_Shutdown();
		return qfalse;
	case RSERR_INVALID_MODE:
		ri_Printf( GLW_PRINT_ALL,
			"...WARNING: could not set the given mode (%d)\n",
			RCVAR( r_mode )->integer );
		QGL_Shutdown();
		return qfalse;
	default:
		break;
	}
	return qtrue;
}

/* ---- GLimp_Init  0x0050ABF0 ----  VERIFIED */
void GLimp_Init( void ) {
	cvar_t	*lastValidRenderer;
	cvar_t	*cv;

	lastValidRenderer = (cvar_t *)ri_Cvar_Get( "r_lastValidRenderer",
											   "(uninitialized)", 1 );

	ri_Printf( GLW_PRINT_ALL, "Initializing OpenGL subsystem\n" );

	if ( !GLW_CheckOSVersion() ) {
		ri_Error( 0, "EXE_ERR_BAD_WINDOWS_VER" );
	}

	cv = (cvar_t *)ri_Cvar_Get( "win_hinstance", "", 0 );
	sscanf( cv->string, "%i", (int *)&g_wv_hInstance );

	cv = (cvar_t *)ri_Cvar_Get( "win_wndproc", "", 0 );
	sscanf( cv->string, "%i", (int *)&glw_wndproc );

	glw_allowSoftwareGL = (int)ri_Cvar_Get( "r_allowSoftwareGL", "0", 0x20 );

	if ( !GLW_StartDriverAndSetMode( "opengl32" ) ) {
		ri_Error( 0, "EXE_ERR_COULDNT_LOAD_OPENGL" );
		return;
	}

	glConfig_vendor_string     = (int)QGL_GetString( GL_VENDOR );
	glConfig_renderer_string   = (int)QGL_GetString( GL_RENDERER );
	glConfig_version_string    = (char *)QGL_GetString( GL_VERSION );
	glConfig_extensions_string = (int)QGL_GetString( GL_EXTENSIONS );

	qwglGetExtensionsStringEXT = (const char * (WINAPI *)( void ))
		qwglGetProcAddress( "wglGetExtensionsStringEXT" );
	if ( qwglGetExtensionsStringEXT ) {
		glConfig_wextensions_string = (int)qwglGetExtensionsStringEXT();
	}

	if ( lastValidRenderer->string && glConfig_renderer_string &&
		 Q_stricmpn( (const char *)glConfig_renderer_string,
					 lastValidRenderer->string, 99999 ) ) {
		ri_Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST" );
	}
	ri_Cvar_Set( "r_lastValidRenderer", (const char *)glConfig_renderer_string );

	GLW_InitExtensions();
	WG_CheckHardwareGamma();
}

/* ---- GLimp_EndFrame  0x0050A610 ----  VERIFIED */
void GLimp_EndFrame( void ) {
	if ( RCVAR( r_swapInterval )->modified ) {
		RCVAR( r_swapInterval )->modified = qfalse;
		if ( !glConfig_stereoEnabled && qwglSwapIntervalEXT ) {
			qwglSwapIntervalEXT( RCVAR( r_swapInterval )->integer );
		}
	}

	if ( !RCVAR( r_drawBuffer )->string ||
		 Q_stricmpn( "GL_FRONT", RCVAR( r_drawBuffer )->string, 99999 ) ) {
		if ( glConfig_NVVertexArrayRange ) {
			qglFlushVertexArrayRangeNV();
		}
		SwapBuffers( glw_hDC );
	}
}

/* ---- GLW_ClearGlConfig  no-address ---- */
static void GLW_ClearGlConfig( void ) {
	glConfig_renderer_string                = 0;    /* +0x00 */
	glConfig_vendor_string                  = 0;    /* +0x04 */
	glConfig_version_string                 = 0;    /* +0x08 */
	glConfig_extensions_string              = 0;    /* +0x0C */
	glConfig_wextensions_string             = 0;    /* +0x10 */
	glConfig_maxTextureSize                 = 0;    /* +0x14 */
	Value                                   = 0;    /* +0x18 maxActiveTextures */
	glConfig_maxLights                      = 0;    /* +0x1C */
	glConfig_colorBits                      = 0;    /* +0x20 */
	glConfig_depthBits                      = 0;    /* +0x24 */
	glConfig_stencilBits                    = 0;    /* +0x28 */
	glConfig_deviceSupportsGamma            = 0;    /* +0x2C */
	glConfig_maxAnisotropy                  = 0;    /* +0x30 */
	glConfig_maxTextureFilterAnisotropy     = 0;    /* +0x34 */
	glConfig_textureEnvAddAvailable         = 0;    /* +0x38 */
	glConfig_textureCubeMap                 = 0;    /* +0x3C */
	glConfig_textureEnvCombine              = 0;    /* +0x40 */
	glConfig_textureEnvDot3                 = 0;    /* +0x44 */
	glConfig_ARBVertexBufferObject          = 0;    /* +0x48 */
	glConfig_ARBVertexProgram               = 0;    /* +0x4C */
	glConfig_rescaleNormal                  = 0;    /* +0x50 */
	glConfig_NVFogAvailable                 = 0;    /* +0x54 */
	glConfig_NVFogMode                      = 0;    /* +0x58 */
	glConfig_NVVertexArrayRange             = 0;    /* +0x5C */
	glConfig_NVFenceAvailable               = 0;    /* +0x60 owned here */
	glConfig_NVRegisterCombiners            = 0;    /* +0x64 */
	glConfig_NVTextureShader                = 0;    /* +0x68 */
	glConfig_maxPNTrianglesTessellationLevel = 0;   /* +0x6C */
	glConfig_pnTrianglesNormalMode          = 0;    /* +0x70 */
	glConfig_pnTrianglesPointMode           = 0;    /* +0x74 */
	glConfig_ATIVertexArrayObject           = 0;    /* +0x78 */
	glConfig_ATIElementArray                = 0;    /* +0x7C */
	glConfig_ATIFragmentShader              = 0;    /* +0x80 */
	dwStyle                                 = 0;    /* +0x84 vidWidth */
	dwExStyle                               = 0;    /* +0x88 vidHeight */
	glConfig_windowAspect                   = 0.0f; /* +0x8C owned here */
	glConfig_displayFrequency               = 0;    /* +0x90 */
	glConfig_isFullscreen                   = 0;    /* +0x94 */
	glConfig_stereoEnabled                  = 0;    /* +0x98 */
}

/* ---- GLW_ClearGlState  no-address ---- */
static void GLW_ClearGlState( void ) {
	glState_currentTmu              = 0;                    /* +0x00 */
	glState_currentClientTmu        = 0;                    /* +0x04 */
	memset( glState_currentTextures, 0,
			sizeof( glState_currentTextures ) );            /* +0x08, 16 dwords */
	dword_16C38CC                   = 0;                    /* +0x0C alias */
	memset( glState_texEnv, 0, sizeof( glState_texEnv ) );  /* +0x28, 8 dwords */
	memset( cap, 0, sizeof( cap ) );                        /* +0x48, 8 dwords */
	memset( glState_textureShaderEnabled, 0,
			sizeof( glState_textureShaderEnabled ) );       /* +0x68, 9 dwords */
	glState_finishCalled            = 0;                    /* +0x88 alias */
	glState_faceCulling             = 0;                    /* +0x8C */
	glState_glStateBits             = 0;                    /* +0x90 */
	glState_clientStateBits         = 0;                    /* +0x94 */
	glState_fogMode                 = 0;                    /* +0x98 */
	glState_fogHint                 = 0;                    /* +0x9C */
	glState_fogColor                = 0.0f;                 /* +0xA0 */
	flt_16C3964                     = 0.0f;                 /* +0xA4 */
	flt_16C3968                     = 0.0f;                 /* +0xA8 */
	dword_16C396C                   = 0;                    /* +0xAC */
	glState_fogStart                = 0.0f;                 /* +0xB0 */
	glState_fogEnd                  = 0.0f;                 /* +0xB4 */
	glState_fogDensity              = 0.0f;                 /* +0xB8 */
	glState_currentStorageMode      = 0;                    /* +0xBC */
	glState_numEnabledLights        = 0;                    /* +0xC0 */
	glState_currentLightingEntity   = 0;                    /* +0xC4 */
	glState_currentLightingFlags    = 0;                    /* +0xC8 */
	glState_normalizeTarget         = 0;                    /* +0xCC */
	glState_boundFragmentShader     = 0;                    /* +0xD0 */
	glState_boundVertexProgram      = 0;                    /* +0xD4 */
}

/* ---- GLimp_Shutdown  0x0050AD50 ---- */
void GLimp_Shutdown( void ) {
	static const char  *success[2] = { "failed", "success" };
	int					retVal;

	if ( !qwglMakeCurrent ) {
		return;
	}

	ri_Printf( GLW_PRINT_ALL, "Shutting down OpenGL subsystem\n" );

	WG_RestoreGamma();

	if ( qwglMakeCurrent ) {
		retVal = qwglMakeCurrent( NULL, NULL ) != 0;
		ri_Printf( GLW_PRINT_ALL, "...wglMakeCurrent( NULL, NULL ): %s\n",
			success[retVal] );
	}

	if ( glw_hGLRC ) {
		retVal = qwglDeleteContext( glw_hGLRC ) != 0;
		ri_Printf( GLW_PRINT_ALL, "...deleting GL context: %s\n",
			success[retVal] );
		glw_hGLRC = NULL;
	}

	if ( glw_hDC ) {
		retVal = ReleaseDC( (HWND)g_wv_hWnd, glw_hDC ) != 0;
		ri_Printf( GLW_PRINT_ALL, "...releasing DC: %s\n", success[retVal] );
		glw_hDC = NULL;
	}

	if ( g_wv_hWnd ) {
		ri_Printf( GLW_PRINT_ALL, "...destroying window\n" );
		ShowWindow( (HWND)g_wv_hWnd, SW_HIDE );
		DestroyWindow( (HWND)g_wv_hWnd );
		g_wv_hWnd = 0;
		glw_pixelFormatSet = qfalse;

		if ( !com_dedicated->integer && g_splashWnd ) {
			ShowWindow( (HWND)g_splashWnd, SW_SHOW );
			UpdateWindow( (HWND)g_splashWnd );
		}
	}

	/* 0x019C0004, the QGL log file, spelled `Stream` as at the renderer's logging sites. */
	if ( Stream ) {
		fclose( Stream );
		Stream = NULL;
	}

	if ( glw_cdsFullscreen ) {
		ri_Printf( GLW_PRINT_ALL, "...resetting display\n" );
		ChangeDisplaySettings( NULL, 0 );
		glw_cdsFullscreen = qfalse;
	}

	QGL_Shutdown();

	GLW_ClearGlConfig();
	GLW_ClearGlState();
}
