/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_gl_types.h"
#include "tr_tess.h"

#define R_MAX_VERTEX_PROGRAMS        128
#define R_VERTEXPROGRAM_STRIDE       68
#define R_VERTEXPROGRAM_NAMEOFF      0
#define R_VERTEXPROGRAM_GLNAMEOFF    64

#define rendererVertexPrograms       ((char *)rendererVertexPrograms)     /* 0x011DBE58 */
#define rendererVertexProgramCount   (r_numVertexPrograms[0])         /* 0x011DE058 */
#define rendererCurrentVertexProgram glState_boundVertexProgram              /* 0x016C3994 */
#define glConfig_vertexProgramAvail  glConfig_ARBVertexProgram              /* 0x016C3AAC */

#define ri_FS_ReadFile               ri_FS_ReadFile              /* 0x016D896C */
#define ri_FS_FreeFile               ri_FS_FreeFile              /* 0x016D8970 */

#define GL_VERTEX_PROGRAM_ARB          0x8620
#define GL_PROGRAM_FORMAT_ASCII_ARB    0x8875
#define GL_PROGRAM_ERROR_POSITION_ARB  0x864B
#define GL_PROGRAM_ERROR_STRING_ARB    0x8874

int R_DeleteVertexPrograms( void );

/* ---- R_LoadVertexProgram  0x004EA150 ----  VERIFIED */
char *__cdecl R_LoadVertexProgram( const char *name )
{
	const char *path;
	int   fileLength;
	char *program;
	int   errorString;
	void *fileBuffer;      /* [ebp-0Ch], ri.FS_ReadFile out-parameter */
	int   errorPosition;   /* [ebp-08h], qglGetIntegerv out-parameter */

	if ( rendererVertexProgramCount == R_MAX_VERTEX_PROGRAMS )
	{
		ri_Printf( 2, "WARNING: tried to load more than %i unique vertex programs in a single map\n",
		           R_MAX_VERTEX_PROGRAMS );
		return 0;
	}

	path = va( "scripts/%s.vp10", name );

	fileLength = ri_FS_ReadFile( path, &fileBuffer );
	if ( fileLength < 0 )
	{
		ri_Printf( 2, "WARNING: couldn't open vertex program '%s'\n", path );
		return 0;
	}

	program = rendererVertexPrograms + R_VERTEXPROGRAM_STRIDE * rendererVertexProgramCount;
	*(int *)( program + R_VERTEXPROGRAM_GLNAMEOFF ) = rendererVertexProgramCount + 1;

	strcpy( program + R_VERTEXPROGRAM_NAMEOFF, name );

	if ( glConfig_vertexProgramAvail )
	{
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB,
		                   *(int *)( program + R_VERTEXPROGRAM_GLNAMEOFF ) );
		qglProgramStringARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
		                     fileLength, fileBuffer );
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, 0 );

		qglGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errorPosition );
		if ( errorPosition >= 0 )
		{
			errorString = (int)qglGetString( GL_PROGRAM_ERROR_STRING_ARB );
			ri_Printf( 2, "WARNING: shader '%s': error in vertex program '%s' at char %i: %s\n",
			           path, errorPosition, errorString );
			qglDeleteProgramsARB( 1, (const GLuint *)( program + R_VERTEXPROGRAM_GLNAMEOFF ) );
			ri_FS_FreeFile( fileBuffer );
			return 0;
		}
	}

	ri_FS_FreeFile( fileBuffer );
	++rendererVertexProgramCount;
	return program;
}

/* ---- R_FindVertexProgram  0x004EA2B0 ----  VERIFIED */
char *__cdecl R_FindVertexProgram( const char *name )
{
	int   i;
	char *entry;

	entry = rendererVertexPrograms;
	for ( i = 0; i < rendererVertexProgramCount; i++, entry += R_VERTEXPROGRAM_STRIDE )
	{
		if ( !_stricmp( name, entry ) )
			return rendererVertexPrograms + R_VERTEXPROGRAM_STRIDE * i;
	}

	return R_LoadVertexProgram( name );
}

/* ---- R_InitVertexPrograms  0x004EA300 ----  VERIFIED */
int R_InitVertexPrograms( void )
{
	return R_DeleteVertexPrograms();
}

/* ---- R_DeleteVertexPrograms  0x004EA310 ----  VERIFIED */
int R_DeleteVertexPrograms( void )
{
	int   result;
	int   i;
	char *glName;

	result = glConfig_vertexProgramAvail;
	if ( glConfig_vertexProgramAvail )
	{
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, 0 );
		result = rendererVertexProgramCount;
		rendererCurrentVertexProgram = 0;

		glName = rendererVertexPrograms + R_VERTEXPROGRAM_GLNAMEOFF;
		for ( i = 0; i < rendererVertexProgramCount; i++, glName += R_VERTEXPROGRAM_STRIDE )
		{
			qglDeleteProgramsARB( 1, (const GLuint *)glName );
			result = rendererVertexProgramCount;
		}
	}

	rendererVertexProgramCount = 0;
	return result;
}
