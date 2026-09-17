/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_records.h"
#include "tr_gl_types.h"
#include "tr_tess.h"

extern int GL_Cull();
extern int GL_State();
int __fastcall RB_BuildDlightArrays( unsigned char *a1, float *a2, float *a3, int a4 );
extern int RB_ComputeColors();
extern int RB_ComputeTexCoords();
extern int RB_DeformTessGeometry();
extern int RB_SetIteratorFog();
extern int RB_SetupMultitextureATI();

#define tess_numVertexes            tess_numVertexes   /* 0x0197FFA8 */
#define tess_numIndexes             tess_numIndexes   /* 0x0197FFA4 */
#define tess_vertexComponentCount   tess_vertexComponentCount   /* 0x0197FF80 */
#define tess_shader                 tess_shader   /* 0x0197FF90 */
#define tess_dlightBits             tess_dlightBits   /* 0x0197FFA0 */
#define tess_activeTexCoords        (&tess_activeTexCoords) /* 0x01877F60, 8 ptrs */
#define tess_activeStageCount       tess_activeStageCount   /* 0x019BFFBC */
#define tess_activeStages           tess_activeStages   /* 0x019BFFC4 */
#define tess_indexes_base           tess_indexes    /* 0x017A7F60 = tess base */

#define glConfig_maxActiveTextures  Value           /* 0x016C3A78 */
#define rb_clientStateBits          glState_clientStateBits   /* 0x016C3954 */
#define tr_dlightShader             tr_dlightShader   /* 0x016C4E14 */

#define rb_atiObjectBuffer          backEnd_dynamicBuffer_storage   /* 0x016D93EC */
#define rb_atiBufferSize            backEnd_dynamicBuffer_capacity   /* 0x016D93F0 */
#define rb_atiBufferCursor          backEnd_dynamicBuffer_currentOffset   /* 0x016D93F4 */

#define backEnd_currentEntity       backEnd_currentEntity   /* 0x016D8E5C */
#define backEnd_currentLight        backEnd_currentDlight   /* 0x016D8E60 */
#define backEnd_currentLightScale   backEnd_currentLightScale     /* 0x016D8E64 */
#define backEnd_num_dlights         backEnd_refdef_num_dlights   /* 0x016D8B1C */
#define backEnd_dlights             backEnd_refdef_dlights   /* 0x016D8B24 */

#define R_MAX_SHADER_STAGES         8
#define R_DLIGHT_STRIDE             0x88
#define SHADER_STAGE_PER_LIGHT      0x80
#define SHADER_LIGHTING_PER_ENTITY  0x80

/* ---- RB_PickBufferOffsetATI  0x0050B540 ----  VERIFIED */
int __cdecl RB_PickBufferOffsetATI( int size, int capacity, int *cursor )
{
	int offset;

	offset  = ( *cursor + size > capacity ) ? 0 : *cursor;
	*cursor = offset + size;
	return offset;
}

/* ---- RB_SingleStageGenericATI  0x0050B560 ----  VERIFIED */
void __cdecl RB_SingleStageGenericATI( _DWORD *stage, GLsizei numIndexes, GLvoid *indexes )
{
	int   capacity;       /* ebx -- rb_atiBufferSize, cached at entry */
	int   cursor;         /* esi -- running offset, published at the end */
	int   i;
	int   size;
	int   offset;
	int   colorOffset;
	int   normalOffset;
	int   vertexOffset;
	_DWORD *bundle;
	int   texCoordOffsets[8];   /* [esp+2Ch] var_20, the 8 dwords of `sub esp,20h` */

	capacity = rb_atiBufferSize;
	cursor   = rb_atiBufferCursor;

	qglEnableClientState( 0x8768u  );

	RB_ComputeTexCoords( (int)stage );

	if ( glConfig_maxActiveTextures > 0 )
	{
		bundle = stage + 36;               /* stage + 0x90 */
		for ( i = 0; i < glConfig_maxActiveTextures; i++, bundle += 50  )
		{
			texCoordOffsets[i] = 0;
			if ( ( *stage & ( 256 << i ) ) != 0 )
			{
				size   = 4 * tess_numVertexes * *bundle;
				offset = ( size + cursor > capacity ) ? 0 : cursor;
				cursor = offset + size;
				texCoordOffsets[i] = offset;
				qglUpdateObjectBufferATI( rb_atiObjectBuffer, offset, size,
				                          (const GLvoid *)tess_activeTexCoords[i],
				                          0x8762  );
			}
		}
	}

	RB_SetupMultitextureATI( stage, rb_atiObjectBuffer, (int)texCoordOffsets, 0 );

	if ( ( *stage & 0x10000 ) != 0 )
	{
		size        = 4 * tess_numVertexes;
		colorOffset = ( size + cursor > capacity ) ? 0 : cursor;
		cursor      = colorOffset + size;

		RB_ComputeColors( (int)stage );

		if ( ( rb_clientStateBits & 0x100 ) == 0 )
		{
			qglEnableClientState( 0x8076u  );
			rb_clientStateBits |= 0x100u;
		}
		qglUpdateObjectBufferATI( rb_atiObjectBuffer, colorOffset, size,
		                          tess_stageVertexColors, 0x8762 );
		qglArrayObjectATI( 0x8076 , 4, 0x1401 ,
		                   0, rb_atiObjectBuffer, colorOffset );
	}
	else
	{
		if ( ( rb_clientStateBits & 0x100 ) != 0 )
		{
			qglDisableClientState( 0x8076u );
			rb_clientStateBits &= ~0x100u;
		}
		if ( stage[409] /* +0x664 rgbGen */ == 11 )
			qglColor4ubv( (const GLubyte *)( backEnd_currentEntity + 108 ) );
		else
			qglColor4ubv( (const GLubyte *)stage + 1664 /* +0x680 constantColor */ );
	}

	if ( ( *stage & 0x20000 ) != 0 )
	{
		size         = 12 * tess_numVertexes;
		normalOffset = ( size + cursor > capacity ) ? 0 : cursor;
		cursor       = normalOffset + size;
		qglUpdateObjectBufferATI( rb_atiObjectBuffer, normalOffset, size,
		                          tess_stageNormals, 0x8762 );
		if ( ( rb_clientStateBits & 0x200 ) == 0 )
		{
			qglEnableClientState( 0x8075u  );
			rb_clientStateBits |= 0x200u;
		}
		qglArrayObjectATI( 0x8075, 3, 0x1406 , 0,
		                   rb_atiObjectBuffer, normalOffset );
	}
	else if ( ( rb_clientStateBits & 0x200 ) != 0 )
	{
		qglDisableClientState( 0x8075u );
		rb_clientStateBits &= ~0x200u;
	}

	size         = 4 * tess_numVertexes * tess_vertexComponentCount;
	vertexOffset = ( size + cursor > capacity ) ? 0 : cursor;
	cursor       = vertexOffset + size;
	qglUpdateObjectBufferATI( rb_atiObjectBuffer, vertexOffset, size,
	                          tess_xyz, 0x8762 );
	if ( ( rb_clientStateBits & 0x400 ) == 0 )
	{
		qglEnableClientState( 0x8074u  );
		rb_clientStateBits |= 0x400u;
	}
	qglArrayObjectATI( 0x8074, tess_vertexComponentCount, 0x1406, 0,
	                   rb_atiObjectBuffer, vertexOffset );

	GL_State( stage[417] /* +0x684 stateBits */ );

	rb_atiBufferCursor = cursor;

	qglDrawElements( 4u , numIndexes,
	                 0x1403u , indexes );
	qglDisableClientState( 0x8768u );
}

/* ---- RB_IterateStagesGenericATI  0x0050B840 ----  VERIFIED */
void __cdecl RB_IterateStagesGenericATI( void )
{
	int    i;
	char  *stage;

	for ( i = 0; i < 4 * R_MAX_SHADER_STAGES; i += 4 )
	{
		stage = *(char **)( i + tess_activeStages );
		if ( !stage )
			break;

		if ( *stage >= 0 )
		{
			RB_SingleStageGenericATI( (_DWORD *)stage, tess_numIndexes,
			                          (GLvoid *)tess_indexes_base );
			if ( r_lightmap->integer )
			{
				if ( *(unsigned char *)( stage + 200 ) )
					break;
				if ( *(unsigned char *)( stage + 400 ) )
					break;
			}
		}
	}
}

/* ---- ProjectDlightTextureATI  0x0050B8A0 ----  VERIFIED */
void __cdecl ProjectDlightTextureATI( void )
{
	int     i;
	int     dlightOffset;
	int     stageIndex;
	char   *stage;
	_DWORD *dlightStage;
	GLsizei hitIndexCount;
	unsigned short hitIndexes[49152];   /* 0x18004 bytes incl. 4 pad */

	if ( !backEnd_num_dlights )
		return;

	dlightOffset = 0;
	for ( i = 0; i < backEnd_num_dlights; i++, dlightOffset += R_DLIGHT_STRIDE )
	{
		if ( ( ( 1 << i ) & tess_dlightBits ) == 0 )
			continue;

		hitIndexCount = RB_BuildDlightArrays( (unsigned char *)tess_vertexColors,
		                                      (float *)tess_texCoords1,
		                                      (float *)( backEnd_dlights + dlightOffset ),
		                                      (int)hitIndexes );
		if ( !hitIndexCount )
			continue;

		if ( *(char *)( tess_shader + 84 ) >= 0 )
		{
			dlightStage = *(_DWORD **)( tr_dlightShader + 340  );
			if ( dlightStage )
				RB_SingleStageGenericATI( dlightStage, hitIndexCount, hitIndexes );
		}
		else
		{
			backEnd_currentLight = backEnd_dlights + dlightOffset;
			for ( stageIndex = 0; stageIndex < 4 * R_MAX_SHADER_STAGES; stageIndex += 4 )
			{
				stage = *(char **)( stageIndex + tess_activeStages );
				if ( !stage )
					break;
				if ( *stage < 0 )
					RB_SingleStageGenericATI( (_DWORD *)stage, hitIndexCount, hitIndexes );
			}
		}
	}
}

/* ---- RB_StageIteratorGenericATI  0x0050B9C0 ----  VERIFIED */
void __cdecl RB_StageIteratorGenericATI( int portalPass )
{
	char   *logText;
	int     entity;
	int     lightCount;
	int     lightIndex;
	int     stageIndex;
	_DWORD *stage;

	if ( tess_activeStageCount <= 0 )
		return;
	if ( portalPass && !tess_dlightBits )
		return;

	if ( r_logFile->integer )
	{
		logText = va( "--- RB_StageIteratorGenericATI( %s ) ---\n",
		              (const char *)tess_shader );
		if ( Stream )
			fprintf( Stream, "%s", (int)logText );
	}

	RB_DeformTessGeometry();
	RB_SetIteratorFog();
	GL_Cull( *(_DWORD *)( tess_shader + 168  ) );

	if ( !portalPass )
		RB_IterateStagesGenericATI();

	if ( tess_dlightBits )
	{
		if ( *(float *)( tess_shader + 88  ) <= 5.0
		  || ( *(unsigned char *)( tess_shader + 84 ) & SHADER_LIGHTING_PER_ENTITY ) != 0 )
		{
			if ( ( *(_DWORD *)( tess_shader + 92  ) & 0x20004 ) == 0 )
			{
				ProjectDlightTextureATI();
				return;
			}
		}
	}

	if ( ( *(unsigned char *)( tess_shader + 84 ) & SHADER_LIGHTING_PER_ENTITY ) == 0 )
		return;

	entity = backEnd_currentEntity;
	if ( !entity )
		return;
	lightCount = *(_DWORD *)( entity + 204  );
	if ( !lightCount )
		return;

	for ( lightIndex = 0; lightIndex < lightCount; lightIndex++ )
	{
		backEnd_currentLight = *(_DWORD *)( entity + 8 * lightIndex + 208  );
		if ( *(_DWORD *)backEnd_currentLight != 8  )
		{
			backEnd_currentLightScale =
			    *(float *)( entity + 8 * lightIndex + 212  );

			for ( stageIndex = 0; stageIndex < 4 * R_MAX_SHADER_STAGES; stageIndex += 4 )
			{
				stage = *(_DWORD **)( stageIndex + tess_activeStages );
				if ( !stage )
					break;
				if ( ( *(unsigned char *)stage & SHADER_STAGE_PER_LIGHT ) != 0 )
				{
					RB_SingleStageGenericATI( stage, tess_numIndexes,
					                          (GLvoid *)tess_indexes_base );
					entity = backEnd_currentEntity;
				}
			}
		}
		lightCount = *(_DWORD *)( entity + 204 );
	}

	backEnd_currentLight = 0;
}
