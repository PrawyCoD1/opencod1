/*
 * g_utils_mp.c -- misc utility functions for the multiplayer game module.
 *
 * Half of this unit is RTCW's game/g_utils.c, carried across unchanged in
 * shape: the configstring index helpers, G_Find/G_PickTarget, vtos,
 * G_SetMovedir, the gentity_t lifecycle (G_InitGentity / G_Spawn /
 * G_FreeEntity / G_TempEntity), G_KillBox, the event helpers, G_SetOrigin and
 * infront.  The other half is CoD's own: the DObj model/tag attachment system
 * (G_EntAttach .. G_DObjGetWorldTagMatrix) that replaced RTCW's single
 * s.modelindex, and the const-string helpers that go with the script VM.
 *
 * Function order is binary order (0x2003A800 .. 0x2003C7C0).
 *
 * @fidelity: likely
 */

#include "g_local.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Declarations that belong in a shared header. */

/* Configstring layout.  Each base is the previous base plus its count. */
#define CS_TAGS                     108
#define CS_TAGS_COUNT               32
#define CS_MODELS                   268
#define CS_MODELS_COUNT             256
#define CS_SOUNDS                   524
#define CS_SOUNDS_COUNT             256
#define CS_EFFECTS                  780
#define CS_EFFECTS_COUNT            64
#define CS_SHELLSHOCKS              1100
#define CS_SHELLSHOCKS_COUNT        16
#define CS_LOCALIZED_STRINGS        1244
#define CS_LOCALIZED_STRINGS_COUNT  256
#define CS_SHADERS                  1500
#define CS_SHADERS_COUNT            128

/* entityState_t.eType.  0..11 are UO's values unchanged; 1.1 has no vehicle
 * types, so its event base is 12 rather than 16 (G_TempEntity 0x2003C317). */
#define ET_GENERAL                  0
#define ET_SCRIPTMOVER              8
#define ET_TURRET                   11
#define ET_EVENTS                   12

/* trajectory_t.trType. */
#define TR_STATIONARY               0
#define TR_INTERPOLATE              1

/* The sound-alias event, 0xAC (G_PlaySoundAlias 0x2003C5CB). */
#define EV_SOUND_ALIAS              172

/* The only content bit this unit names (G_KillBox 0x2003C3FB). */
#define CONTENTS_BODY               0x2000000

/* G_PickTarget's fixed choice array. */
#define MAXCHOICES                  32

#define FOFS( x ) ( (int)&( ( (gentity_t *)0 )->x ) )

/* universal/q_math.c.  The three-operand forms compose left-to-right:
 * out = in0 applied inside in1. */
void        AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void        AxisToAngles( vec3_t axis[3], vec3_t angles );
float       vectoyaw( const vec3_t vec );
void        MatrixMultiply( vec3_t in0[3], vec3_t in1[3], vec3_t out[3] );
void        MatrixTranspose( vec3_t in[3], vec3_t out[3] );
void        MatrixMultiply43( const void *in0, const void *in1, void *out );
void        MatrixInverseOrthogonal34( void *out, const void *in );
/* Argument order is ( matrix, out, in ) -- destination in the MIDDLE
   (universal/com_math.c 0x0042F950, server_mp/sv_game_mp.c 0x00456470). */
void        MatrixTransformVector43( const void *mat, vec3_t out, const vec3_t in );
void        DObjSkelMatrixMultiply43( const void *skelMat, const void *in1, void *out );
/* OUT IS THE SECOND PARAMETER, not the third: universal/com_math_raw.c:109
   writes through a2 and reads a1 and a3.  Spelled exactly as cg_local.h
   spells it. */
void        DObjSkel2MatrixMultiply43( const float *mat43, float *out,
                                       const float *boneMatrix );
/* QuatMultiply is declared in g_local.h: destination is the MIDDLE argument. */

/* universal/q_shared.c. */
int         Q_stricmp( const char *s1, const char *s2 );
int         Com_sprintf( char *dest, int size, const char *fmt, ... );
char        *va( const char *format, ... );

/* forward: g_main_mp.c */
void        G_Printf( const char *fmt, ... );
void        G_Error( const char *fmt, ... );

/* forward: g_combat_mp.c */
void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, const vec3_t dir,
			   const vec3_t point, int damage, int dflags, int mod, int hitLoc );

/* forward: g_client_mp.c */
void G_ClientStopUsingTurret( gentity_t *ent );

/* forward: g_scr_main_mp.c */
void Scr_FreeEntityConstStrings( gentity_t *ent );

/* The XModel behind every model configstring index, filled in by G_ModelIndex
 * and read back by G_GetModel (0x2008BF40). */
static void *s_modelIndexXModels[CS_MODELS_COUNT];

/*
================
G_FindConfigstringIndex
================
*/
int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create,
							 const char *fieldname ) {
	int i;
	const char  *s;

	if ( !name || !name[0] ) {
		return 0;
	}

	for ( i = 1 ; i < max ; i++ ) {
		s = trap_GetConfigstringConst( start + i );
		if ( !s[0] ) {
			break;
		}
		if ( !_stricmp( s, name ) ) {
			return i;
		}
	}

	if ( !create ) {
		if ( fieldname ) {
			Scr_Error( va( "%s \"%s\" not precached", fieldname, name ) );
		}
		return 0;
	}

	if ( i == max ) {
		G_Error( "G_FindConfigstringIndex: overflow" );
	}

	trap_SetConfigstring( start + i, name );

	return i;
}

int G_LocalizedStringIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_LOCALIZED_STRINGS, CS_LOCALIZED_STRINGS_COUNT,
									level.spawning, "localized string" );
}

int G_ShaderIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_SHADERS, CS_SHADERS_COUNT,
									level.spawning, "shader" );
}

/*
================
G_ModelIndex

Not a G_FindConfigstringIndex wrapper: it caches the XModel alongside the
configstring, and a model that was not precached is an error but still gets
registered.
================
*/
int G_ModelIndex( const char *name ) {
	int i;
	const char  *s;

	if ( !name[0] ) {
		return 0;
	}

	for ( i = 1 ; i < CS_MODELS_COUNT ; i++ ) {
		s = trap_GetConfigstringConst( CS_MODELS + i );
		if ( !s[0] ) {
			break;
		}
		if ( !_stricmp( s, name ) ) {
			return i;
		}
	}

	if ( !level.spawning ) {
		Scr_Error( va( "model '%s' not precached", name ) );
	}

	if ( i == CS_MODELS_COUNT ) {
		G_Error( "G_ModelIndex: overflow" );
	}

	s_modelIndexXModels[i] = trap_XModelGet( name );
	trap_SetConfigstring( CS_MODELS + i, name );

	return i;
}

static void *G_GetModel( int modelIndex ) {
	return s_modelIndexXModels[modelIndex];
}

const char *G_ModelName( int modelIndex ) {
	return trap_GetConfigstringConst( CS_MODELS + modelIndex );
}

int G_TagIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_TAGS, CS_TAGS_COUNT, qtrue, NULL );
}

int G_EffectIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_EFFECTS, CS_EFFECTS_COUNT,
									level.spawning, "effect" );
}

int G_ShellShockIndex( const char *name ) {
	return G_FindConfigstringIndex( name, CS_SHELLSHOCKS, CS_SHELLSHOCKS_COUNT, qtrue, NULL );
}

byte G_SoundAliasIndex( const char *name ) {
	return (byte)G_FindConfigstringIndex( name, CS_SOUNDS, CS_SOUNDS_COUNT, qtrue, NULL );
}

unsigned short G_GetGameId( gentity_t *ent ) {
	return Scr_GetEntityId( ent->s.number, 0 );
}

/*
================
G_DObjUpdate

Rebuild the entity's DObj from its model and its attach slots.  Clients keep
their DObj in sync from ClientEndFrame instead, so they are left alone.
================
*/
void G_DObjUpdate( gentity_t *ent ) {
	DObjModel models[MAX_ATTACHED_MODELS + 1];
	unsigned short gameId;
	int modelCount;
	int i;

	if ( ent->client ) {
		return;
	}

	G_SafeDObjFree( ent );

	if ( !ent->model ) {
		G_UpdateTagInfoOfChildren( ent, qfalse );
		return;
	}

	gameId = G_GetGameId( ent );

	models[0].model = G_GetModel( ent->model );
	models[0].tagName = NULL;
	models[0].boneName = -(short)ent->model;
	modelCount = 1;

	switch ( ent->s.eType ) {
	case ET_GENERAL:
	case ET_SCRIPTMOVER:
	case ET_TURRET:
		ent->s.index = ent->model;
		break;
	}

	for ( i = 0 ; i < MAX_ATTACHED_MODELS ; i++ ) {
		if ( !ent->attachModelIndex[i] ) {
			continue;
		}
		models[modelCount].model = G_GetModel( ent->attachModelIndex[i] );
		models[modelCount].tagName = SL_ConvertToString( ent->attachTagName[i] );
		models[modelCount].boneName = -(short)ent->attachModelIndex[i];
		modelCount++;
	}

	trap_DObjCreate( models, (unsigned short)modelCount, NULL, ent->s.number, gameId );

	G_UpdateTagInfoOfChildren( ent, qtrue );
}

void G_SetModel( gentity_t *ent, const char *modelName ) {
	if ( !modelName[0] ) {
		ent->model = 0;
		return;
	}

	ent->model = (byte)G_ModelIndex( modelName );
}

qboolean G_EntAttach( gentity_t *ent, const char *modelName, const char *tagName ) {
	int i;

	i = 0;
	while ( ent->attachModelIndex[i] ) {
		if ( ++i >= MAX_ATTACHED_MODELS ) {
			return qfalse;
		}
	}

	ent->attachModelIndex[i] = (byte)G_ModelIndex( modelName );
	ent->attachTagName[i] = SL_GetLowercaseString( tagName, 0 );

	G_DObjUpdate( ent );

	return qtrue;
}

qboolean G_EntDetach( gentity_t *ent, const char *modelName, const char *tagName ) {
	unsigned short tagId;
	int i;

	tagId = SL_FindLowercaseString( tagName );
	if ( !tagId ) {
		return qfalse;
	}

	i = 0;
	while ( ent->attachTagName[i] != tagId
			|| _stricmp( G_ModelName( ent->attachModelIndex[i] ), modelName ) ) {
		if ( ++i >= MAX_ATTACHED_MODELS ) {
			return qfalse;
		}
	}

	ent->attachModelIndex[i] = 0;
	Scr_SetString( &ent->attachTagName[i], 0 );

	for ( ; i < MAX_ATTACHED_MODELS - 1 ; i++ ) {
		ent->attachModelIndex[i] = ent->attachModelIndex[i + 1];
		ent->attachTagName[i] = ent->attachTagName[i + 1];
	}

	ent->attachModelIndex[i] = 0;
	ent->attachTagName[i] = 0;

	G_DObjUpdate( ent );

	return qtrue;
}

void G_EntDetachAll( gentity_t *ent ) {
	int i;

	for ( i = 0 ; i < MAX_ATTACHED_MODELS ; i++ ) {
		ent->attachModelIndex[i] = 0;
		Scr_SetString( &ent->attachTagName[i], 0 );
	}

	G_DObjUpdate( ent );
}

/*
================
G_EntLinkToInternal

Build the link record.  Refuses a link that would close a cycle, and a tagged
link whose tag is not a bone of the parent's DObj.
================
*/
static qboolean G_EntLinkToInternal( gentity_t *child, gentity_t *parent, const char *tagName ) {
	int parentTagIndex;
	gentity_t           *ancestor;
	entityLinkInfo_t    *linkInfo;

	G_EntUnlink( child );

	if ( tagName[0] ) {
		if ( !trap_DObjExists( parent ) ) {
			return qfalse;
		}
		parentTagIndex = trap_DObjGetBoneIndex( parent, tagName );
		if ( parentTagIndex < 0 ) {
			return qfalse;
		}
	} else {
		parentTagIndex = -1;
	}

	for ( ancestor = parent ; ; ancestor = ancestor->linkInfo->parent ) {
		if ( ancestor == child ) {
			return qfalse;
		}
		if ( !ancestor->linkInfo ) {
			break;
		}
	}

	linkInfo = MT_Alloc( sizeof( entityLinkInfo_t ), 16 );
	linkInfo->parent = parent;
	linkInfo->tagName = tagName[0] ? SL_GetLowercaseString( tagName, 0 ) : 0;
	linkInfo->nextChild = parent->firstChild;
	linkInfo->parentTagIndex = parentTagIndex;
	memset( &linkInfo->relAxis, 0, sizeof( linkInfo->relAxis ) );
	parent->firstChild = child;
	child->linkInfo = linkInfo;
	memset( &linkInfo->parentRelAxis, 0, sizeof( linkInfo->parentRelAxis ) );

	return qtrue;
}

qboolean G_EntLinkTo( gentity_t *child, gentity_t *parent, const char *tagName ) {
	if ( !G_EntLinkToInternal( child, parent, tagName ) ) {
		return qfalse;
	}

	G_CalcTagAxis( child, qfalse );

	return qtrue;
}

qboolean G_EntLinkToWithOffset( gentity_t *child, gentity_t *parent, const char *tagName,
								const vec3_t originOffset, const vec3_t anglesOffset ) {
	entityLinkInfo_t    *linkInfo;

	if ( !G_EntLinkToInternal( child, parent, tagName ) ) {
		return qfalse;
	}

	linkInfo = child->linkInfo;
	AnglesToAxis( anglesOffset, linkInfo->relAxis.axis );
	VectorCopy( originOffset, linkInfo->relAxis.origin );

	return qtrue;
}

/*
================
G_EntUnlink

Freeze the entity where the link left it, then drop it out of its parent's
child list.
================
*/
void G_EntUnlink( gentity_t *ent ) {
	entityLinkInfo_t    *linkInfo;
	gentity_t           *prev;
	gentity_t           *child;

	linkInfo = ent->linkInfo;
	if ( !linkInfo ) {
		return;
	}

	G_SetOrigin( ent, ent->r.currentOrigin );
	G_SetAngle( ent, ent->r.currentAngles );

	prev = NULL;
	for ( child = linkInfo->parent->firstChild ; child != ent ;
		  child = child->linkInfo->nextChild ) {
		prev = child;
	}

	if ( !prev ) {
		linkInfo->parent->firstChild = linkInfo->nextChild;
	} else {
		prev->linkInfo->nextChild = linkInfo->nextChild;
	}

	ent->linkInfo = NULL;
	Scr_SetString( &linkInfo->tagName, 0 );
	MT_Free( linkInfo, sizeof( entityLinkInfo_t ) );
}

qboolean G_EntIsLinkedTo( gentity_t *child, gentity_t *parent ) {
	entityLinkInfo_t    *linkInfo;

	linkInfo = child->linkInfo;

	return (qboolean)( linkInfo && linkInfo->parent == parent );
}

/*
================
G_UpdateTagInfo

The parent's DObj was rebuilt, so the cached bone index is stale.  A link whose
tag no longer exists is dropped.
================
*/
void G_UpdateTagInfo( gentity_t *ent, qboolean updateBoneIndex ) {
	entityLinkInfo_t    *linkInfo;

	linkInfo = ent->linkInfo;

	if ( !linkInfo->tagName ) {
		linkInfo->parentTagIndex = -1;
		return;
	}

	if ( updateBoneIndex ) {
		linkInfo->parentTagIndex = trap_DObjGetBoneIndex( linkInfo->parent,
														  SL_ConvertToString( linkInfo->tagName ) );
		if ( linkInfo->parentTagIndex >= 0 ) {
			return;
		}
	}

	G_EntUnlink( ent );
}

void G_UpdateTagInfoOfChildren( gentity_t *parent, qboolean updateBoneIndex ) {
	gentity_t   *child;
	gentity_t   *nextChild;

	for ( child = parent->firstChild ; child ; child = nextChild ) {
		nextChild = child->linkInfo->nextChild;
		G_UpdateTagInfo( child, updateBoneIndex );
	}
}

/*
================
G_CalcTagParentAxis

The world transform of whatever the child hangs off: the parent itself for an
untagged link, otherwise the parent's tagged bone.
================
*/
void G_CalcTagParentAxis( gentity_t *child, matrix43_t *outAxis ) {
	entityLinkInfo_t    *linkInfo;
	gentity_t           *parent;
	matrix43_t parentAxis;

	linkInfo = child->linkInfo;
	parent = linkInfo->parent;

	if ( linkInfo->parentTagIndex < 0 ) {
		AnglesToAxis( parent->r.currentAngles, outAxis->axis );
		VectorCopy( parent->r.currentOrigin, outAxis->origin );
		return;
	}

	AnglesToAxis( parent->r.currentAngles, parentAxis.axis );
	VectorCopy( parent->r.currentOrigin, parentAxis.origin );

	G_DObjCalcBone( parent, linkInfo->parentTagIndex );
	DObjSkelMatrixMultiply43( (float *)trap_DObjGetMatrixArray( parent )
							  + linkInfo->parentTagIndex * 16,
							  &parentAxis, outAxis );
}

void G_CalcTagParentRelAxis( gentity_t *child, matrix43_t *outAxis ) {
	entityLinkInfo_t    *linkInfo;
	matrix43_t parentAxis;

	linkInfo = child->linkInfo;
	G_CalcTagParentAxis( child, &parentAxis );
	MatrixMultiply43( &linkInfo->parentRelAxis, &parentAxis, outAxis );
}

/*
================
G_CalcTagAxis

Record where the entity currently sits relative to its link parent.  With
useLinkedAngles the origin is left alone and only the rotation is captured.
================
*/
void G_CalcTagAxis( gentity_t *ent, int useLinkedAngles ) {
	entityLinkInfo_t    *linkInfo;
	matrix43_t inverseParentAxis;
	matrix43_t parentAxis;
	matrix43_t localAxis;

	G_CalcTagParentAxis( ent, &parentAxis );
	AnglesToAxis( ent->r.currentAngles, localAxis.axis );
	linkInfo = ent->linkInfo;

	if ( useLinkedAngles ) {
		MatrixTranspose( parentAxis.axis, inverseParentAxis.axis );
		MatrixMultiply( localAxis.axis, inverseParentAxis.axis, linkInfo->relAxis.axis );
	} else {
		MatrixInverseOrthogonal34( &inverseParentAxis, &parentAxis );
		VectorCopy( ent->r.currentOrigin, localAxis.origin );
		MatrixMultiply43( &localAxis, &inverseParentAxis, &linkInfo->relAxis );
	}
}

/*
================
G_SetFixedLink

Drive the entity from its stored relative transform.  Mode 0 takes the whole
transform, 1 takes the origin plus the yaw, 2 takes the origin only.
================
*/
void G_SetFixedLink( gentity_t *ent, int mode ) {
	entityLinkInfo_t    *linkInfo;
	matrix43_t parentAxis;
	matrix43_t linkedAxis;

	G_CalcTagParentAxis( ent, &parentAxis );
	linkInfo = ent->linkInfo;

	if ( mode == 1 ) {
		MatrixMultiply43( &linkInfo->relAxis, &parentAxis, &linkedAxis );
		VectorCopy( linkedAxis.origin, ent->r.currentOrigin );
		ent->r.currentAngles[1] = vectoyaw( linkedAxis.axis[0] );
	} else if ( mode < 2 ) {
		if ( mode == 0 ) {
			MatrixMultiply43( &linkInfo->relAxis, &parentAxis, &linkedAxis );
			VectorCopy( linkedAxis.origin, ent->r.currentOrigin );
			AxisToAngles( linkedAxis.axis, ent->r.currentAngles );
		}
	} else if ( mode == 2 ) {
		MatrixTransformVector43( &parentAxis, linkedAxis.origin, linkInfo->relAxis.origin );
		VectorCopy( linkedAxis.origin, ent->r.currentOrigin );
	}
}

/*
================
G_GeneralLink

Per-frame follow for a linked entity: recompute the world transform and push it
out through the trajectories so the clients interpolate it.
================
*/
void G_GeneralLink( gentity_t *ent ) {
	matrix43_t parentAxis;
	matrix43_t linkedAxis;

	G_CalcTagParentAxis( ent, &parentAxis );
	MatrixMultiply43( &ent->linkInfo->relAxis, &parentAxis, &linkedAxis );

	VectorCopy( linkedAxis.origin, ent->r.currentOrigin );
	AxisToAngles( linkedAxis.axis, ent->r.currentAngles );

	G_SetOrigin( ent, ent->r.currentOrigin );
	G_SetAngle( ent, ent->r.currentAngles );

	ent->s.pos.trType = TR_INTERPOLATE;
	ent->s.apos.trType = TR_INTERPOLATE;

	trap_LinkEntity( ent );
}

void Think_GeneralLink( gentity_t *ent ) {
	ent->nextthink = level.time + 50;

	if ( ent->linkInfo ) {
		G_GeneralLink( ent );
	}
}

void G_SafeDObjFree( gentity_t *ent ) {
	trap_SafeDObjFree( ent->s.number, qfalse );
}

int G_DObjUpdateServerTime( gentity_t *ent, int stopOnNotetrack ) {
	return trap_DObjUpdateServerTime( ent, level.frameTime * 0.001f, stopOnNotetrack );
}

/*
================
G_DObjSetLocalTagInternal

Overwrite one bone of the pose with an explicit origin/angles.  The three
angle-to-quaternion helpers are inlined here by the retail build.
================
*/
/* Not static: turret_think (g_misc_mp.c) calls it.  0x2003B790. */
void G_DObjSetLocalTagInternal( gentity_t *ent, const vec3_t origin, const vec3_t angles,
								int boneIndex ) {
	DObjAnimMat *rotTrans;
	vec4_t yawQuat;
	vec4_t pitchQuat;
	vec4_t rollQuat;
	vec4_t pitchYawQuat;
	float halfAngle;

	rotTrans = &( (DObjAnimMat *)trap_DObjGetRotTransArray( ent ) )[boneIndex];

	halfAngle = angles[1] * ( (float)M_PI / 360.0f );
	yawQuat[0] = 0.0f;
	yawQuat[1] = 0.0f;
	yawQuat[2] = (float)sin( halfAngle );
	yawQuat[3] = (float)cos( halfAngle );

	halfAngle = angles[0] * ( (float)M_PI / 360.0f );
	pitchQuat[0] = 0.0f;
	pitchQuat[1] = (float)sin( halfAngle );
	pitchQuat[2] = 0.0f;
	pitchQuat[3] = (float)cos( halfAngle );

	halfAngle = angles[2] * ( (float)M_PI / 360.0f );
	rollQuat[0] = (float)sin( halfAngle );
	rollQuat[1] = 0.0f;
	rollQuat[2] = 0.0f;
	rollQuat[3] = (float)cos( halfAngle );

	/* 0x2003B887: eax=yawQuat, edx=pitchYawQuat (out), ecx=pitchQuat.
	   0x2003B896: eax=pitchYawQuat, edx=rotTrans (out), ecx=rollQuat. */
	QuatMultiply( yawQuat, pitchYawQuat, pitchQuat );
	QuatMultiply( pitchYawQuat, rotTrans->quat, rollQuat );

	rotTrans->accumulatedWeight = 0.0f;
	VectorCopy( origin, rotTrans->translation );
}

qboolean G_DObjSetLocalTag( gentity_t *ent, int *partBits, const char *tagName,
							const vec3_t origin, const vec3_t angles ) {
	int boneIndex;

	boneIndex = trap_DObjGetBoneIndex( ent, tagName );
	if ( boneIndex < 0 ) {
		return qfalse;
	}

	if ( !trap_DObjSetRotTransIndex( ent, partBits, boneIndex ) ) {
		return qfalse;
	}

	G_DObjSetLocalTagInternal( ent, origin, angles, boneIndex );

	return qtrue;
}

qboolean G_DObjSetControlTagAngles( gentity_t *ent, int *partBits, const char *tagName,
									const vec3_t angles ) {
	int boneIndex;

	boneIndex = trap_DObjGetBoneIndex( ent, tagName );
	if ( boneIndex < 0 ) {
		return qfalse;
	}

	if ( !trap_DObjSetControlRotTransIndex( ent, partBits, boneIndex ) ) {
		return qfalse;
	}

	G_DObjSetLocalTagInternal( ent, vec3_origin, angles, boneIndex );

	return qtrue;
}

void G_DObjCalcPose( gentity_t *ent ) {
	int partBits[4];

	memset( partBits, 255, sizeof( partBits ) );

	if ( trap_DObjCreateSkelForBones( ent, partBits ) ) {
		return;
	}

	trap_DObjCalcAnim( ent, partBits );
	if ( ent->controller ) {
		ent->controller( ent, (unsigned int *)partBits );
	}
	trap_DObjCalcSkel( ent, partBits );
}

void G_DObjCalcBone( gentity_t *ent, int boneIndex ) {
	int partBits[4];

	if ( trap_DObjCreateSkelForBone( ent, boneIndex ) ) {
		return;
	}

	trap_DObjGetHierarchyBits( ent, boneIndex, partBits );
	trap_DObjCalcAnim( ent, partBits );
	if ( ent->controller ) {
		ent->controller( ent, (unsigned int *)partBits );
	}
	trap_DObjCalcSkel( ent, partBits );
}

float *G_DObjGetLocalTagMatrix( gentity_t *ent, const char *tagName ) {
	int boneIndex;

	boneIndex = trap_DObjGetBoneIndex( ent, tagName );
	if ( boneIndex < 0 ) {
		return NULL;
	}

	G_DObjCalcBone( ent, boneIndex );

	return (float *)trap_DObjGetMatrixArray( ent ) + boneIndex * 16;
}

qboolean G_DObjGetWorldTagMatrix( gentity_t *ent, const char *tagName, void *outMatrix ) {
	const float *localMatrix;
	matrix43_t entityAxis;

	localMatrix = G_DObjGetLocalTagMatrix( ent, tagName );
	if ( !localMatrix ) {
		return qfalse;
	}

	AnglesToAxis( ent->r.currentAngles, entityAxis.axis );
	VectorCopy( ent->r.currentOrigin, entityAxis.origin );
	/* ( entityAxis, outMatrix, localMatrix ) -- all three slots, not just the
	   out.  0x2003BB36 calls it with eax = &entityAxis, edx = the outMatrix
	   argument and ecx = localMatrix, and the register mapping is eax = mat43,
	   edx = out, ecx = boneMatrix (cg_weapons_mp.c:57, from CG_AddPlayerWeapon
	   0x30037137). */
	DObjSkel2MatrixMultiply43( (const float *)&entityAxis, (float *)outMatrix,
							   localMatrix );

	return qtrue;
}

/*
=============
G_Find

Searches all active entities for the next one that holds the const string id
`match` in the field at `fieldofs`.

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.
=============
*/
gentity_t *G_Find( gentity_t *from, int fieldofs, unsigned short match ) {
	unsigned short s;

	if ( !from ) {
		from = g_entities;
	} else {
		from++;
	}

	for ( ; from < &g_entities[level.num_entities] ; from++ ) {
		if ( !from->inuse ) {
			continue;
		}
		s = *( unsigned short * )( (byte *)from + fieldofs );
		if ( !s ) {
			continue;
		}
		if ( s == match ) {
			return from;
		}
	}

	return NULL;
}

gentity_t *G_FindStr( gentity_t *from, int fieldofs, const char *match ) {
	char    *s;

	if ( !from ) {
		from = g_entities;
	} else {
		from++;
	}

	for ( ; from < &g_entities[level.num_entities] ; from++ ) {
		if ( !from->inuse ) {
			continue;
		}
		s = *( char ** )( (byte *)from + fieldofs );
		if ( !s ) {
			continue;
		}
		if ( !match ) {
			continue;
		}
		if ( !Q_stricmp( s, match ) ) {
			return from;
		}
	}

	return NULL;
}

/*
=============
G_PickTarget

Selects a random entity from among the targets
=============
*/
gentity_t *G_PickTarget( unsigned short targetname ) {
	gentity_t   *ent = NULL;
	int num_choices = 0;
	gentity_t   *choice[MAXCHOICES];

	if ( !targetname ) {
		return NULL;
	}

	while ( 1 ) {
		ent = G_Find( ent, FOFS( targetname ), targetname );
		if ( !ent ) {
			break;
		}
		choice[num_choices++] = ent;
		if ( num_choices == MAXCHOICES ) {
			break;
		}
	}

	if ( !num_choices ) {
		G_Printf( "G_PickTarget: target %s not found\n", SL_ConvertToString( targetname ) );
		return NULL;
	}

	return choice[rand() % num_choices];
}

char *vtos( const vec3_t v ) {
	static int index;
	static char str[8][32];
	char    *s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = ( index + 1 ) & 7;

	Com_sprintf( s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2] );

	return s;
}

char *vtosf( const vec3_t v ) {
	static int index;
	static char str[8][64];
	char    *s;

	s = str[index];
	index = ( index + 1 ) & 7;

	Com_sprintf( s, 64, "(%f %f %f)", v[0], v[1], v[2] );

	return s;
}

/*
================
G_SetMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
================
*/
void G_SetMovedir( vec3_t angles, vec3_t movedir ) {
	static vec3_t VEC_UP        = { 0, -1, 0 };
	static vec3_t MOVEDIR_UP    = { 0, 0, 1 };
	static vec3_t VEC_DOWN      = { 0, -2, 0 };
	static vec3_t MOVEDIR_DOWN  = { 0, 0, -1 };

	if ( angles[0] == VEC_UP[0] && angles[1] == VEC_UP[1] && angles[2] == VEC_UP[2] ) {
		VectorCopy( MOVEDIR_UP, movedir );
	} else if ( angles[0] == VEC_DOWN[0] && angles[1] == VEC_DOWN[1] && angles[2] == VEC_DOWN[2] ) {
		VectorCopy( MOVEDIR_DOWN, movedir );
	} else {
		AngleVectors( angles, movedir, NULL, NULL );
	}

	VectorClear( angles );
}

void G_InitGentity( gentity_t *e ) {
	e->inuse = qtrue;
	Scr_SetString( &e->classname, scr_const.noclass );
	e->s.number = e - g_entities;
	e->r.ownerNum = ENTITYNUM_NONE;
	e->eventTime = 0;
	e->freeAfterEvent = qfalse;
}

/*
=================
G_Spawn

Either finds a free entity, or allocates a new one.

  The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
never be used by anything else.

Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
gentity_t *G_Spawn( void ) {
	gentity_t   *e;
	int i;

	if ( level.freeListHead ) {
		e = level.freeListHead;
		level.freeListHead = e->nextFree;
		if ( !level.freeListHead ) {
			level.freeListTail = NULL;
		}
		e->nextFree = NULL;
	} else {
		if ( level.num_entities == MAX_GENTITIES - 2 ) {
			for ( i = 0 ; i < level.num_entities ; i++ ) {
				G_Printf( "%4i: '%s', origin: %f %f %f\n", i,
						  g_entities[i].classname ? SL_ConvertToString( g_entities[i].classname ) : "",
						  g_entities[i].r.currentOrigin[0],
						  g_entities[i].r.currentOrigin[1],
						  g_entities[i].r.currentOrigin[2] );
			}
			G_Error( "G_Spawn: no free entities" );
		}

		e = &level.gentities[level.num_entities];
		level.num_entities++;
		trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
							 &level.clients[0].ps, sizeof( gclient_t ) );
	}

	G_InitGentity( e );

	return e;
}

/*
=================
G_SpawnPlayerClone

The corpse queue: BODY_QUEUE_SIZE slots immediately above the clients, reused
round-robin.  Bit 3 of eFlags is flipped on every reuse so the client sees the
slot as a different entity and does not interpolate the old corpse into the
new one.
=================
*/
gentity_t *G_SpawnPlayerClone( void ) {
	gentity_t   *e;
	int eFlags;

	e = &level.gentities[MAX_CLIENTS + level.playerCloneCursor];
	level.playerCloneCursor = ( level.playerCloneCursor + 1 ) % BODY_QUEUE_SIZE;

	eFlags = ~e->s.eFlags & 8;

	if ( e->inuse ) {
		G_FreeEntity( e );
	}

	G_InitGentity( e );
	e->s.eFlags = eFlags;

	return e;
}

/*
=================
G_FreeEntityRefs

Drop every pointer and entity number the rest of the game holds to ent, so
nothing follows it into the free list.
=================
*/
void G_FreeEntityRefs( gentity_t *ent ) {
	gentity_t   *e;
	int i;

	for ( i = 0 ; i < level.num_entities ; i++ ) {
		e = &g_entities[i];
		if ( !e->inuse ) {
			continue;
		}
		if ( e->parent == ent ) {
			e->parent = NULL;
		}
		if ( e->r.ownerNum == ent->s.number ) {
			e->r.ownerNum = ENTITYNUM_NONE;
			if ( e->s.eType == ET_TURRET ) {
				e->active = qfalse;
			}
		}
		if ( e->s.groundEntityNum == ent->s.number ) {
			e->s.groundEntityNum = ENTITYNUM_NONE;
		}
	}

	for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
		e = &g_entities[i];
		if ( !e->inuse ) {
			continue;
		}
		if ( e->client->lookatEnt == ent ) {
			e->client->lookatEnt = NULL;
		}
	}
}

/*
=================
G_FreeEntity

Marks the entity as free, and pushes it on the tail of the free list so the
oldest free slot is reused first.  The client and corpse slots are never queued.
=================
*/
void G_FreeEntity( gentity_t *ent ) {
	int spawnCount;

	G_EntUnlink( ent );
	while ( ent->firstChild ) {
		G_EntUnlink( ent->firstChild );
	}

	trap_UnlinkEntity( ent );       // unlink from world
	G_SafeDObjFree( ent );
	G_FreeEntityRefs( ent );

	if ( ent->turret ) {
		if ( g_entities[ent->r.ownerNum].client ) {
			G_ClientStopUsingTurret( ent );
		}
		ent->active = qfalse;
		*(int *)ent->turret = 0;
		ent->turret = NULL;
	}

	Scr_FreeEntityConstStrings( ent );
	Scr_FreeEntityNum( ent->s.number, 0 );

	spawnCount = ent->spawnCount;

	memset( ent, 0, sizeof( gentity_t ) );

	if ( ent - level.gentities >= MAX_CLIENTS + BODY_QUEUE_SIZE ) {
		if ( level.freeListTail ) {
			level.freeListTail->nextFree = ent;
		} else {
			level.freeListHead = ent;
		}
		level.freeListTail = ent;
		ent->nextFree = NULL;
	}

	ent->spawnCount = spawnCount + 1;
}

/*
=================
G_TempEntity

Spawns an event entity that will be auto-removed
The origin will be snapped to save net bandwidth, so care
must be taken if the origin is right on a surface (snap towards start vector
first)
=================
*/
gentity_t *G_TempEntity( const vec3_t origin, int event ) {
	gentity_t   *e;
	vec3_t snapped;

	e = G_Spawn();
	e->s.eType = ET_EVENTS + event;

	Scr_SetString( &e->classname, scr_const.tempEntity );
	e->eventTime = level.time;
	e->r.eventTime = level.time;
	e->freeAfterEvent = qtrue;

	VectorCopy( origin, snapped );
	snapped[0] = (float)(int)snapped[0];    // save network bandwidth
	snapped[1] = (float)(int)snapped[1];
	snapped[2] = (float)(int)snapped[2];
	G_SetOrigin( e, snapped );

	// find cluster for PVS
	trap_LinkEntity( e );

	return e;
}

/*
=================
G_KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
void G_KillBox( gentity_t *ent ) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t   *hit;
	vec3_t mins, maxs;

	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES, CONTENTS_BODY );

	for ( i = 0 ; i < num ; i++ ) {
		hit = &g_entities[touch[i]];
		if ( !hit->client ) {
			continue;
		}
		if ( !hit->r.linked ) {
			continue;
		}

		// nail it
		G_Damage( hit, ent, ent, NULL, NULL, 100000, DAMAGE_NO_TEAM_PROTECTION, MOD_TELEFRAG, 0 );
	}
}

/*
===============
G_AddPredictableEvent

Use for non-pmove events that would also be predicted on the
client side: jumppads and item pickups
Adds an event+parm and twiddles the event counter
===============
*/
void G_AddPredictableEvent( gentity_t *ent, byte event, byte eventParm ) {
	if ( !ent->client ) {
		return;
	}
	if ( !event ) {
		return;
	}

	ent->client->ps.events[ent->client->ps.eventSequence & 3] = event;
	ent->client->ps.eventParms[ent->client->ps.eventSequence & 3] = eventParm;
	ent->client->ps.eventSequence++;
}

/*
===============
G_AddEvent

Adds an event+parm and twiddles the event counter
===============
*/
void G_AddEvent( gentity_t *ent, int event, int eventParm ) {
	// clients need to add the event in playerState_t instead of entityState_t
	if ( ent->client ) {
		ent->client->ps.events[ent->client->ps.eventSequence & 3] = event;
		ent->client->ps.eventParms[ent->client->ps.eventSequence & 3] = eventParm;
		ent->client->ps.eventSequence++;
	} else {
		ent->s.events[ent->s.eventSequence & 3] = event;
		ent->s.eventParms[ent->s.eventSequence & 3] = eventParm;
		ent->s.eventSequence++;
	}

	ent->eventTime = level.time;
	ent->r.eventTime = level.time;
}

void G_PlaySoundAliasAtPoint( const vec3_t origin, byte alias ) {
	gentity_t   *te;

	if ( !alias ) {
		return;
	}

	te = G_TempEntity( origin, EV_SOUND_ALIAS );
	te->s.eventParm = alias;
}

void G_PlaySoundAlias( gentity_t *ent, byte alias ) {
	if ( !alias ) {
		return;
	}

	G_AddEvent( ent, EV_SOUND_ALIAS, alias );
}

/* Installed as the animation script's sound callback by G_InitGame
 * (0x20025EEE). */
void G_AnimScriptSound( int client, const char *name ) {
	byte alias;

	alias = G_SoundAliasIndex( name );
	if ( !alias ) {
		return;
	}

	G_AddEvent( &g_entities[client], EV_SOUND_ALIAS, alias );
}

void G_SetOrigin( gentity_t *ent, const vec3_t origin ) {
	VectorCopy( origin, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorClear( ent->s.pos.trDelta );

	VectorCopy( origin, ent->r.currentOrigin );
}

void G_SetAngle( gentity_t *ent, const vec3_t angles ) {
	VectorCopy( angles, ent->s.apos.trBase );
	ent->s.apos.trType = TR_STATIONARY;
	ent->s.apos.trTime = 0;
	ent->s.apos.trDuration = 0;
	VectorClear( ent->s.apos.trDelta );

	VectorCopy( angles, ent->r.currentAngles );
}

qboolean infront( gentity_t *self, gentity_t *other ) {
	vec3_t vec;
	float dot;
	vec3_t forward;

	AngleVectors( self->r.currentAngles, forward, NULL, NULL );
	VectorSubtract( other->r.currentOrigin, self->r.currentOrigin, vec );
	VectorNormalize( vec );
	dot = DotProduct( vec, forward );

	if ( dot > 0.0 ) {
		return qtrue;
	}

	return qfalse;
}

/* Debug polygons only work when running a local game; the multiplayer module
 * keeps the entry point and does nothing. */
int DebugLine( const vec3_t start, const vec3_t end, int color ) {
	return 0;
}

void G_SetConstString( unsigned short *dest, const char *string ) {
	Scr_SetString( dest, 0 );
	*dest = SL_GetString( string, 0 );
}
