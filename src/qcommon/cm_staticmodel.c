/*
 * qcommon/cm_staticmodel.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cm_staticmodel.c
 *
 * Retail range 0x0041F760-0x0041FFF0, 5 functions.
 *
 * @fidelity: likely
 */

#include "cm_local.h"

typedef struct cmStaticModel_t
{
	void                    *model;         /* +0x00  XModel *  */
	vec3_t origin;                          /* +0x04 */
	float inverseAxis[9];                   /* +0x10 */
	struct cmStaticModel_t  *next;          /* +0x34 */
	vec3_t linkMins;                        /* +0x38 */
	vec3_t linkMaxs;                        /* +0x44 */
	qboolean isBig;                         /* +0x50 */
} cmStaticModel_t;
COD1_ASSERT_SIZE( cmStaticModel_t, 84 );

extern int cm_numStaticModels;                   /* 0x016174E0 */
extern cmStaticModel_t *cm_staticModels;      /* 0x016174E4 */

#define cm_numStaticModels  cm_numStaticModels
#define cm_staticModels     cm_staticModels

/* universal/com_math.c -- both settled there, both __usercall in retail. */
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right,
				   vec3_t up );
void MatrixInverse( const float in[9], float out[9] );

void CM_LinkStaticModel( void *link );

typedef void *( *XModelAllocFn )( int size );
void *XModelPrecache( const char *name, int loadSurfaces, XModelAllocFn alloc,
					  XModelAllocFn allocMesh );

int XModelGetStaticBounds( void *model, float *mins, const float *axis, float *maxs );

/* ---- CM_Hunk_AllocXModel  0x0041F760 ---- VERIFIED */
void *CM_Hunk_AllocXModel( int size ) {
	return Hunk_AllocAlignInternal( size, 32 );
}

/* ---- CM_Hunk_AllocXModelMesh  0x0041F770 ---- VERIFIED */
void *CM_Hunk_AllocXModelMesh( int size ) {
	return Hunk_AllocAlignInternal( size, 32 );
}

/* CM_StaticModelContents  0x0041F935 */
static int CM_StaticModelContents( void *model ) {
	return *( int * ) ( *( byte ** ) ( ( byte * ) model + 4 ) + 72 );
}

/* ---- CM_CreateStaticModel  0x0041F780 ---- VERIFIED */
void CM_CreateStaticModel( const char *name, cmStaticModel_t *sm,
						   const vec3_t origin, const vec3_t angles,
						   const vec3_t scale ) {
	float axis[9];
	vec3_t right;
	int i;

	if ( !name || !name[0] ) {
		Com_Error( ERR_DROP, "\x15" "Invalid static model name\n" );
	}
	if ( scale[0] == 0.0f ) {
		Com_Error( ERR_DROP, "\x15" "Static model [%s] has x scale of 0.0\n",
				   name );
	}
	if ( scale[1] == 0.0f ) {
		Com_Error( ERR_DROP, "\x15" "Static model [%s] has y scale of 0.0\n",
				   name );
	}
	if ( scale[2] == 0.0f ) {
		Com_Error( ERR_DROP, "\x15" "Static model [%s] has z scale of 0.0\n",
				   name );
	}

	sm->model = XModelPrecache( name, 1, CM_Hunk_AllocXModel,
								CM_Hunk_AllocXModelMesh );

	sm->origin[0] = origin[0];
	sm->origin[1] = origin[1];
	sm->origin[2] = origin[2];

	AngleVectors( angles, &axis[0], right, &axis[6] );

	axis[3] = 0.0f - right[0];
	axis[4] = 0.0f - right[1];
	axis[5] = 0.0f - right[2];

	for ( i = 0; i < 3; i++ ) {
		axis[0 + i] *= scale[0];
		axis[3 + i] *= scale[1];
		axis[6 + i] *= scale[2];
	}

	MatrixInverse( axis, sm->inverseAxis );

	if ( !XModelGetStaticBounds( sm->model, sm->linkMins, axis, sm->linkMaxs ) ) {
		return;
	}

	sm->linkMins[0] += origin[0];
	sm->linkMins[1] += origin[1];
	sm->linkMins[2] += origin[2];
	sm->linkMaxs[0] += origin[0];
	sm->linkMaxs[1] += origin[1];
	sm->linkMaxs[2] += origin[2];

	if ( CM_StaticModelContents( sm->model ) ) {
		CM_LinkStaticModel( sm );
	}
}

static char *CM_ParseEntityToken( char **data_p ) {
	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	return Com_ParseExt( data_p, qtrue );
}

/* ---- CM_LoadStaticModels  0x0041F950 ---- VERIFIED */
void CM_LoadStaticModels( void ) {
	char *data_p;
	const char *token;
	char key[64];
	char value[64];
	char modelName[64];
	vec3_t origin, angles, scale;
	qboolean isStaticModel;
	int cursor;

	data_p = cm_entityString;
	cm_numStaticModels = 0;
	cm_staticModels = NULL;

	while ( *CM_ParseEntityToken( &data_p ) == '{' ) {
		isStaticModel = qfalse;

		for ( ;; ) {
			token = CM_ParseEntityToken( &data_p );
			if ( !token[0] || token[0] == '}' ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = CM_ParseEntityToken( &data_p );
			if ( !token[0] ) {
				break;
			}
			Q_strncpyz( value, token, sizeof( value ) );

			if ( !Q_stricmp( key, "classname" ) &&
				 !Q_stricmp( value, "misc_model" ) ) {
				isStaticModel = qtrue;
			}
		}

		if ( isStaticModel ) {
			cm_numStaticModels++;
		}
	}

	if ( !cm_numStaticModels ) {
		return;
	}

	cm_staticModels = ( cmStaticModel_t * )
		Hunk_AllocAlignInternal( 84 * cm_numStaticModels, 32 );

	data_p = cm_entityString;
	cursor = 0;

	while ( *CM_ParseEntityToken( &data_p ) == '{' ) {
		Com_Memset( modelName, 0, sizeof( modelName ) );
		VectorClear( origin );
		VectorClear( angles );
		scale[0] = 1.0f;
		scale[1] = 1.0f;
		scale[2] = 1.0f;
		isStaticModel = qfalse;

		for ( ;; ) {
			token = CM_ParseEntityToken( &data_p );
			if ( !token[0] || token[0] == '}' ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = CM_ParseEntityToken( &data_p );
			if ( !token[0] ) {
				break;
			}
			Q_strncpyz( value, token, sizeof( value ) );

			if ( !Q_stricmp( key, "classname" ) ) {
				if ( !Q_stricmp( value, "misc_model" ) ) {
					isStaticModel = qtrue;
				}
			} else if ( !Q_stricmp( key, "model" ) ) {
				Q_strncpyz( modelName, value, sizeof( modelName ) );
			} else if ( !Q_stricmp( key, "origin" ) ) {
				sscanf( value, "%f %f %f",
						&origin[0], &origin[1], &origin[2] );
			} else if ( !Q_stricmp( key, "angles" ) ) {
				sscanf( value, "%f %f %f",
						&angles[0], &angles[1], &angles[2] );
			} else if ( !Q_stricmp( key, "modelscale_vec" ) ) {
				sscanf( value, "%f %f %f",
						&scale[0], &scale[1], &scale[2] );
			} else if ( !Q_stricmp( key, "modelscale" ) ) {
				scale[2] = ( float ) atof( value );
				scale[1] = scale[2];
				scale[0] = scale[2];
			}
		}

		if ( isStaticModel ) {
			CM_CreateStaticModel( modelName + 7,
								  ( cmStaticModel_t * )
								  ( ( byte * ) cm_staticModels + cursor ),
								  origin, angles, scale );
			cursor += 84;
		}
	}
}

#include <malloc.h>

int XModelNumBones( const void *model );
void XModelGetBasePose( void *model, void *basePose );

int XModelTraceLine( void *model, trace_t *work, const float *basePose,
				const vec3_t localStart, const vec3_t localEnd,
				int contentmask );

vec_t VectorNormalize( vec3_t v );

/* ---- CM_TraceStaticModel  0x0041FE00 ---- VERIFIED */
void CM_TraceStaticModel( cmStaticModel_t *sm, trace_t *results,
						  const vec3_t start, const vec3_t end,
						  int contentmask ) {
	trace_t work;
	vec3_t localStart, localEnd;
	vec3_t d, n;
	void *basePose;
	int numBones;
	int hit;
	int i;

	numBones = XModelNumBones( sm->model );
	basePose = _alloca( numBones << 6 );
	XModelGetBasePose( sm->model, basePose );

	for ( i = 0; i < 3; i++ ) {
		d[i] = start[i] - sm->origin[i];
	}
	localStart[0] = d[0] * sm->inverseAxis[0] + d[1] * sm->inverseAxis[3]
					+ d[2] * sm->inverseAxis[6];
	localStart[1] = d[0] * sm->inverseAxis[1] + d[1] * sm->inverseAxis[4]
					+ d[2] * sm->inverseAxis[7];
	localStart[2] = d[0] * sm->inverseAxis[2] + d[1] * sm->inverseAxis[5]
					+ d[2] * sm->inverseAxis[8];

	for ( i = 0; i < 3; i++ ) {
		d[i] = end[i] - sm->origin[i];
	}
	localEnd[0] = d[0] * sm->inverseAxis[0] + d[1] * sm->inverseAxis[3]
				  + d[2] * sm->inverseAxis[6];
	localEnd[1] = d[0] * sm->inverseAxis[1] + d[1] * sm->inverseAxis[4]
				  + d[2] * sm->inverseAxis[7];
	localEnd[2] = d[0] * sm->inverseAxis[2] + d[1] * sm->inverseAxis[5]
				  + d[2] * sm->inverseAxis[8];

	work.fraction = results->fraction;

	hit = XModelTraceLine( sm->model, &work, (const float *)basePose,
					  localStart, localEnd, contentmask );
	if ( hit < 0 ) {
		return;
	}

	work.entityNum = ENTITYNUM_WORLD;               /* WORD 0x3FE at work+0x28 */

	for ( i = 0; i < 3; i++ ) {
		work.endpos[i] = start[i] + work.fraction * ( end[i] - start[i] );
	}

	for ( i = 0; i < 3; i++ ) {
		n[i] = work.normal[0] * sm->inverseAxis[3 * i]
			   + work.normal[1] * sm->inverseAxis[3 * i + 1]
			   + work.normal[2] * sm->inverseAxis[3 * i + 2];
	}
	VectorNormalize( n );
	work.normal[0] = n[0];
	work.normal[1] = n[1];
	work.normal[2] = n[2];

	Com_Memcpy( results, &work, sizeof( work ) );   /* rep movsd, ECX = 0x0C */
}
