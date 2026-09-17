/*
 * bg_animation.c -- the flexible animation script system, shared by the game
 * module and cgame.
 *
 * Call of Duty 1.1 multiplayer (game_mp_x86.dll, 0x20001000 .. 0x20004F4E).
 * RTCW's game/bg_animation.c is the direct ancestor and most of the text below
 * is that original, adjusted where CoD changed it:
 *
 *   - there is one player model, so RTCW's animModelInfo_t was folded into
 *     animScriptData_t and the per-client model table is gone;
 *   - the animations are XAnims out of a script anim tree rather than frame
 *     ranges out of an .animcfg, so BG_AnimParseAnimConfig is replaced by
 *     BG_FinalizePlayerAnims;
 *   - the condition list is cut from RTCW's 18 to 9, the movetypes retuned to
 *     18 and the events to 14;
 *   - BG_ParseCommands understands three new per-animation keywords
 *     (duration, blendtime, turretanim) and derives the movetype bit set, the
 *     strafe flags and the firing/death flags as it parses.
 *
 * The XAnim/DObj player layer (BG_SetNewAnimation, BG_RunLerpFrameRate,
 * BG_PlayerAngles, BG_Player_DoControllers, BG_UpdatePlayerDObj and friends)
 * has no RTCW ancestor at all; the rest still tracks RTCW's bg_animation.c /
 * cg_players.c.
 *
 * The animation_t flag bits are still nameless in bg_public.h, so they are
 * spelled numerically here with the meaning in a comment:
 *   0x001  animation came from the runtime anim list, not from the tree
 *   0x002  ladder animation (climbup/climbdown with a movespeed)
 *   0x004  turret animation
 *   0x008  firing animation
 *   0x010  strafe-left animation
 *   0x020  strafe-right animation
 *   0x040  death animation
 *   0x080  looped
 *   0x100  no animation behind this index
 * and 0x200 is RTCW's ANIM_TOGGLEBIT in ps->legsAnim / ps->torsoAnim.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "g_public.h"
#include "bg_public.h"

/* game_mp has no header for the qcommon / q_math / q_parse half of the
 * module, so those prototypes are declared here. */
extern parseInfo_t *parseInfo;

void QDECL Com_Error( int level, const char *fmt, ... );
void Com_BeginParseSession( const char *name );
void Com_EndParseSession( void );
char *Com_Parse( char **data_p );
char *Com_ParseExt( char **data_p, qboolean allowLineBreaks );
/*
 * PARSE THROUGH THE WRAPPERS, NOT Com_ParseExt.  Com_ParseExt is the raw
 * tokenizer and does not touch parseInfo->ungetReady; Com_Parse and
 * Com_ParseOnLine consume the pushed-back token first.  Calling the raw one
 * after a Com_UngetToken leaves ungetReady set, so the next unget in the same
 * loop dies on "UngetToken called twice".  Retail calls the wrappers; the
 * compiler inlined them, with the ungetReady restore open-coded ahead of the
 * raw call (at 0x300021BA it is hoisted well above the call it belongs to).
 */
char *Com_ParseOnLine( char **data_p );
void Com_UngetToken( void );
void QDECL Com_Printf( const char *fmt, ... );
int Q_stricmp( const char *s1, const char *s2 );
int Q_strncmp( const char *s1, const char *s2, int n );
void Q_strcat( char *dest, int size, const char *src );
float AngleMod( float a );
float AngleSubtract( float a1, float a2 );
float AngleNormalize180( float angle );
vec_t Distance( const vec3_t p1, const vec3_t p2 );

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void trap_FS_Read( void *buffer, int len, fileHandle_t f );
void trap_FS_FCloseFile( fileHandle_t f );
int trap_XAnimIsPrimitive( scr_anim_t anim );
int trap_XAnimIsLooped( scr_anim_t anim );
const char *trap_XAnimGetAnimName( scr_anim_t anim );
int trap_XAnimGetLength( void *tree, unsigned short animIndex );
int trap_XAnimGetAnimTreeSize( void *tree );
void trap_XAnimGetRelDelta( scr_anim_t anim, float *deltaRot, float *deltaMove,
							float startTime, float endTime );
float trap_XAnimGetTime( void *tree, unsigned short animIndex );
float trap_XAnimGetWeight( void *tree, unsigned short animIndex );
void trap_XAnimSetAnimRate( void *tree, unsigned short animIndex, float rate );
/*
 * Two traps that are NOT the same call in the two builds.
 *
 * XModelGet does not exist on the client at all: cgame's BG_GetXModel
 * (0x300046D0) is `trap_R_RegisterModel( name, 7 )` and then trap 50 on the
 * handle it returns -- two calls, not one renamed one.
 *
 * DObjCreate takes FOUR arguments on the client, not five: the cgame wrapper
 * at 0x30031BA0 issues syscall( 164, models, modelCount, tree, handle ) and the
 * engine supplies the scrNotifyId itself.  BG_DObjCreate (0x300046F0) is
 * byte-identical to that wrapper.  This is deliberately NOT hidden behind a
 * forwarder that pads a fifth zero -- the call site differs, so the conditional
 * is on the call.
 *
 * qhandle_t is cg_public.h's and a bg unit cannot see that header, so
 * trap_R_RegisterModel is declared with its underlying int here.
 */
#ifdef CGAMEDLL
/* cg_syscalls.c */
int  trap_R_RegisterModel( const char *name, int arg2 );
int  trap_R_GetXModelByHandle( int hModel );
void trap_DObjCreate( void *models, unsigned short modelCount, void *tree, int handle );
#else
void *trap_XModelGet( const char *name );
void trap_DObjCreate( void *models, unsigned short modelCount, void *tree,
					  int handle, unsigned short scrNotifyId );
#endif
void trap_SafeDObjFree( int handle, unsigned int releaseTree );
#ifndef CGAMEDLL
qboolean trap_DObjExists( gentity_t *ent );
void *trap_XAnimCreateTree( void *anims );  /* inferred */
#endif

void trap_XAnimClearGoalWeight( void *tree, unsigned short animIndex, float goalTime );
void trap_XAnimSetTime( void *tree, unsigned short animIndex, float time );
void trap_XAnimSetCompleteGoalWeight( void *tree, unsigned short animIndex, float goalWeight,
									  float goalTime, float rate, unsigned short notifyIndex,
									  int notifyFlags );
void trap_XAnimSetCompleteGoalWeightKnobAll( void *tree, unsigned short animIndex,
											 unsigned short childIndex, float goalWeight,
											 float goalTime, float rate,
											 unsigned short notifyIndex, int notifyFlags );

/*
 * ---- the one genuine client/server split in this file --------------------
 *
 * The two DObj-writing functions below (BG_Player_DoControllers and
 * BG_UpdatePlayerDObj) do not compile to the same thing in the two DLLs, and
 * the difference is not inlining.  The game build takes the gentity_t and
 * resolves the DObj itself; the cgame build is handed the DObj already
 * resolved by its caller (CG_TransitionSnapshot does Com_GetClientDObj, trap
 * 162) and only null-tests it.  Everything downstream then goes through the
 * CG_ helpers instead of the G_ ones, and through the cgame trap numbers.
 *
 *   BG_Player_DoControllers   game 0x200046B0  cgame 0x30004710
 *   BG_UpdatePlayerDObj       game 0x20004CF0  cgame 0x30004EB0
 *
 * Q3 and RTCW split their bg_ files exactly this way; RTCW's spelling of the
 * macro is CGAMEDLL, so that is the spelling here.  The game build is the
 * default.
 */
#ifdef CGAMEDLL

/* cg_ent.c */
qboolean CG_DObjSetLocalTag( int obj, int *partBits, const char *tagName,
							 const vec3_t origin, const vec3_t angles );
qboolean CG_DObjSetControlTagAngles( int obj, int *partBits, const char *tagName,
									 const vec3_t angles );

#define BG_DOBJ_PARM                int obj
#define BG_DObjSetLocalTag          CG_DObjSetLocalTag
#define BG_DObjSetControlTagAngles  CG_DObjSetControlTagAngles
#define BG_DOBJ                     obj

#else

/* g_utils_mp.c; declared here because the bg units do not include g_local.h */
qboolean G_DObjSetLocalTag( gentity_t *ent, int *partBits, const char *tagName,
							const vec3_t origin, const vec3_t angles );
qboolean G_DObjSetControlTagAngles( gentity_t *ent, int *partBits, const char *tagName,
									const vec3_t angles );

#define BG_DOBJ_PARM                gentity_t *ent
#define BG_DObjSetLocalTag          G_DObjSetLocalTag
#define BG_DObjSetControlTagAngles  G_DObjSetControlTagAngles
#define BG_DOBJ                     ent

#endif

/* Installed by the script far-hook table (g_scr_main_mp.c).  g_local.h carries
   the canonical block, but a bg unit does not include it, so these three stay
   here; they are byte-identical to the canonical declarations. */
extern int ( *Scr_GetAnimsIndex )( int anims );
extern void ( *Scr_FindAnim )( const char *treeName, const char *animName, scr_anim_t *anim );
extern void *( *Scr_FindAnimTree )( const char *treeName );

/* 0x200A1A70 / 74 / 78, written by G_InitGame and G_RunFrame. */
int bgs_time;                                   /* 0x200A1A70 */
int bgs_animTime;                               /* 0x200A1A74 */
int bgs_frametime;                              /* 0x200A1A78 */

/* The client registers the same cvar under its own prefix; the two reads
   BG_SetNewAnimation (0x30003B0B, == 1) and BG_RunLerpFrameRate (0x30003DAF,
   == 2) make are the two this file emits.  Conditional on the symbol only. */
#ifdef CGAMEDLL
#define g_debuganim     cg_debuganim
#endif
extern vmCvar_t g_debuganim;                    /* 0x20237280, cgame 0x30298480 */
extern vmCvar_t bg_swingSpeed;                  /* 0x20234580 */

/*
 * 0x200A1C80.  The parse-side functions reach the script data through
 * globalScriptData, but the run-time ones address the object by name -- e.g.
 * BG_SetNewAnimation's `animations[i]` at 0x20003733 and its numAnimations
 * test at 0x200036EB are absolute, which a load through the pointer could
 * never be.  g_main_mp.c declares the same address as `bgs_animState`, a
 * 0xAC8EC byte blob, because G_InitGame clears animScriptData_t + the four
 * resolved anims + bg_clientinfo[64] with one rep stosd (0x20025EB6): the
 * retail name for all of it is `bgs`.
 */
animScriptData_t bgs_animScriptData;

/* entityState_t.eFlags bits this file tests.  0x2|0x4 and 0x800 have no
 * recovered name; they are spelled numerically at their use sites. */
#define EF_DEAD                     0x00000001
#define EF_CROUCHING                0x00000020
#define EF_PRONE                    0x00000040
#define EF_AIMING_DOWN_SIGHT        0x00000200
#define EF_FIRING                   0x00000400
#define EF_TURRET_ACTIVE_PRONE      0x00004000
#define EF_TURRET_ACTIVE_DUCK       0x00008000
#define EF_TURRET_ACTIVE_MASK       ( EF_TURRET_ACTIVE_PRONE | EF_TURRET_ACTIVE_DUCK )

#define BUTTON_ATTACK               0x1

#define ANIM_TOGGLEBIT              0x200

/* the pair of movetype bits every "is this player on a ladder" test uses */
#define ANIM_MT_CLIMBING_MASK       ( ( 1 << ANIM_MT_CLIMBUP ) | ( 1 << ANIM_MT_CLIMBDOWN ) )

/* q_shared.h has neither; BG_PlayerAngles open-codes the pair (0x200040BE,
 * 0x200040F1) to snap the view yaw to the wire's precision */
#define ANGLE2SHORT( x )            ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )            ( (float)( x ) * ( 360.0f / 65536.0f ) )

/* animation_t.flags, still nameless in bg_public.h (see the file header) */
#define ANIMFL_LADDER               0x002
#define ANIMFL_STRAFELEFT           0x010
#define ANIMFL_STRAFERIGHT          0x020
#define ANIMFL_DEATH                0x040

/*
 * One entry of the runtime animation list BG_AnimParseAnimScript is handed.
 * 72 bytes.  The list is live on the first parse: GScr_LoadScripts passes a
 * 36864-byte stack buffer, which is exactly 512 of these, and the re-parse
 * from BG_FinalizePlayerAnims passes NULL.  The type is file-private -- the
 * callers outside this file only forward the buffer, which is why bg_public.h
 * spells the parameter void *.
 */
typedef struct animScriptAnim_t
{
	scr_anim_t anim;
	int hash;
	char name[64];
} animScriptAnim_t;
COD1_ASSERT_SIZE( animScriptAnim_t, 72 );

void BG_UpdateConditionValue( int client, int condition, int value, qboolean checkConversion );

#define MAX_ANIM_DEFINES 16

/* this is used globally within this file to reduce redundant params */
static animScriptData_t *globalScriptData;      /* 0x200A06A4 */
static animScriptAnim_t *globalAnims;           /* 0x20088CC8 */
static int *globalNumAnims;                     /* 0x200865B0 */

static char *globalFilename = "mp/playeranim.script";   /* 0x2006CBE8 */
static qboolean animScriptLoaded;                       /* 0x200A06A8 */
static char globalScriptText[100000];                   /* 0x2006DF10 */

/* these are used globally during script parsing */
static int numDefines[NUM_ANIM_CONDITIONS];                             /* 0x2006DA6C */
static char defineStrings[10000];                                       /* 0x200865B8 */
static int defineStringsOffset;                                         /* 0x200865B4 */
static animStringItem_t defineStr[NUM_ANIM_CONDITIONS][MAX_ANIM_DEFINES];   /* 0x2006D5E8 */
static int defineBits[NUM_ANIM_CONDITIONS][MAX_ANIM_DEFINES][2];            /* 0x2006DA90 */

static scriptAnimMoveTypes_t parseMovetype;     /* 0x2006DA68 */
static int parseEvent;                          /* 0x2006D5E0 */

static animStringItem_t weaponStrings[MAX_WEAPONS];     /* 0x200A1A80 */
static qboolean weaponStringsInited;                    /* 0x200A06AC */

/*
 * The player anim tree BG_FindAnimTrees resolves.  Not static: cg_animtree_mp.c
 * still reads it for its inline copy of the seeding loops (retail 0x30005182).
 */
void *bgs_animTree;                             /* 0x2013D358 */

/* The four script animations BG_FindAnims resolves out of the player tree.
   Not static: they sit inside the 0x200A1C80..0x2014E56C run G_InitGame
   clears with one rep stosd (0x20025EB6), so g_main_mp.c writes them. */
scr_anim_t bgs_rootAnim;                        /* 0x2013D35C */
scr_anim_t bgs_torsoAnim;                       /* 0x2013D360 */
scr_anim_t bgs_legsAnim;                        /* 0x2013D364 */
scr_anim_t bgs_turningAnim;                     /* 0x2013D368 */

/* The per-client animation records; bg_public.h declares them, this unit is
   the definition.  Last member of the same cleared run. */
clientInfo_t bg_clientinfo[MAX_CLIENTS];        /* 0x2013D36C */

static animStringItem_t animStateStr[] =
{
	{"RELAXED", -1},
	{"QUERY", -1},
	{"ALERT", -1},
	{"COMBAT", -1},

	{NULL, -1},
};

static animStringItem_t animMoveTypesStr[] =
{
	{"** UNUSED **", -1},
	{"IDLE", -1},
	{"IDLECR", -1},
	{"IDLEPRONE", -1},
	{"WALK", -1},
	{"WALKBK", -1},
	{"WALKCR", -1},
	{"WALKCRBK", -1},
	{"WALKPRONE", -1},
	{"WALKPRONEBK", -1},
	{"RUN", -1},
	{"RUNBK", -1},
	{"RUNCR", -1},
	{"RUNCRBK", -1},
	{"TURNRIGHT", -1},
	{"TURNLEFT", -1},
	{"CLIMBUP", -1},
	{"CLIMBDOWN", -1},

	{NULL, -1},
};

static animStringItem_t animEventTypesStr[] =
{
	{"PAIN", -1},
	{"DEATH", -1},
	{"FIREWEAPON", -1},
	{"JUMP", -1},
	{"JUMPBK", -1},
	{"LAND", -1},
	{"DROPWEAPON", -1},
	{"RAISEWEAPON", -1},
	{"CLIMBMOUNT", -1},
	{"CLIMBDISMOUNT", -1},
	{"RELOAD", -1},
	{"CROUCH_TO_PRONE", -1},
	{"PRONE_TO_CROUCH", -1},
	{"MELEEATTACK", -1},

	{NULL, -1},
};

static animStringItem_t animBodyPartsStr[] =
{
	{"** UNUSED **", -1},
	{"LEGS", -1},
	{"TORSO", -1},
	{"BOTH", -1},

	{NULL, -1},
};

/*------------------------------------------------------------*/
/* conditions */

/* 0x2006CD50.  RTCW's ENEMY_POSITION values; CoD dropped the condition but the
 * table is still compiled in, unreferenced. */
static animStringItem_t animConditionPositionsStr[] =
{
	{"** UNUSED **", -1},
	{"BEHIND", -1},
	{"INFRONT", -1},
	{"RIGHT", -1},
	{"LEFT", -1},

	{NULL, -1},
};

static animStringItem_t animConditionMountedStr[] =
{
	{"** UNUSED **", -1},
	{"MG42", -1},

	{NULL, -1},
};

/* the weapon classes, spelled the way the animation script spells them; this
 * is bg_animation.c's own table, not bg_weapon.c's bg_weaponClassNames. */
static animStringItem_t animConditionWeaponClassStr[] =
{
	{"RIFLE", -1},
	{"MG", -1},
	{"SMG", -1},
	{"PISTOL", -1},
	{"GRENADE", -1},
	{"ROCKETLAUNCHER", -1},
	{"TURRET", -1},
	{"NON-PLAYER", -1},

	{NULL, -1},
};

static animStringItem_t animConditionWeaponPositionStr[] =
{
	{"HIP", -1},
	{"ADS", -1},

	{NULL, -1},
};

static animStringItem_t animConditionStrafingStr[] =
{
	{"NOT", -1},
	{"LEFT", -1},
	{"RIGHT", -1},

	{NULL, -1},
};

typedef struct animConditionTable_t
{
	animScriptConditionTypes_t type;
	animStringItem_t            *values;
} animConditionTable_t;

static animStringItem_t animConditionsStr[] =
{
	{"WEAPONS", -1},
	{"WEAPONCLASS", -1},
	{"MOUNTED", -1},
	{"MOVETYPE", -1},
	{"UNDERHAND", -1},
	{"CROUCHING", -1},
	{"FIRING", -1},
	{"WEAPON_POSITION", -1},
	{"STRAFING", -1},

	{NULL, -1},
};

static animConditionTable_t animConditionsTable[NUM_ANIM_CONDITIONS] =
{
	{ANIM_CONDTYPE_BITFLAGS,        weaponStrings},
	{ANIM_CONDTYPE_BITFLAGS,        animConditionWeaponClassStr},
	{ANIM_CONDTYPE_VALUE,           animConditionMountedStr},
	{ANIM_CONDTYPE_BITFLAGS,        animMoveTypesStr},
	{ANIM_CONDTYPE_VALUE,           NULL},
	{ANIM_CONDTYPE_VALUE,           NULL},
	{ANIM_CONDTYPE_VALUE,           NULL},
	{ANIM_CONDTYPE_VALUE,           animConditionWeaponPositionStr},
	{ANIM_CONDTYPE_VALUE,           animConditionStrafingStr},
};

/*------------------------------------------------------------*/

/*
================
BG_StringHashValue

  return a hash value for the given string
================
*/
static int BG_StringHashValue( const char *fname ) {
	int i;
	int hash;
	char letter;

	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		letter = tolower( fname[i] );
		hash += (int)( letter ) * ( i + 119 );
		i++;
	}
	if ( hash == -1 ) {
		hash = 0;   // never return -1
	}
	return hash;
}

/*
=================
BG_AnimParseError
=================
*/
void QDECL BG_AnimParseError( const char *msg, ... ) {
	va_list argptr;
	char text[1024];

	va_start( argptr, msg );
	vsprintf( text, msg, argptr );
	va_end( argptr );

	if ( globalFilename ) {
		Com_Error( ERR_DROP, "\x15" "%s: (%s, line %i)", text, globalFilename, parseInfo->currentLine + 1 );
	} else {
		Com_Error( ERR_DROP, "\x15" "%s", text );
	}
}

/*
=================
BG_AnimationIndexForString

  With a runtime animation list attached -- the first parse, from
  GScr_LoadScripts -- the name is registered into it on demand; without one,
  which is the BG_FinalizePlayerAnims re-parse, the script data's own animation
  table is the only source and an unknown name is a parse error.
=================
*/
int BG_AnimationIndexForString( char *string ) {
	int i, hash, num;
	animation_t *anim;

	hash = BG_StringHashValue( string );

	if ( globalAnims ) {
		num = *globalNumAnims;
		for ( i = 0; i < num; i++ ) {
			if ( hash == globalAnims[i].hash && !Q_stricmp( string, globalAnims[i].name ) ) {
				return i;
			}
		}
		Scr_FindAnim( "multiplayer", string, &globalAnims[num].anim );
		strcpy( globalAnims[num].name, string );
		globalAnims[num].hash = hash;
		return ( *globalNumAnims )++;
	}

	for ( i = 0, anim = globalScriptData->animations; i < globalScriptData->numAnimations; i++, anim++ ) {
		if ( hash == anim->hash && !Q_stricmp( string, anim->name ) ) {
			return i;
		}
	}

	BG_AnimParseError( "BG_AnimationIndexForString: unknown player animation '%s'", string );
	return -1;      // shutup compiler
}

/*
=================
BG_AnimationForString
=================
*/
animation_t *BG_AnimationForString( char *string ) {
	int i, hash;
	animation_t *anim;

	hash = BG_StringHashValue( string );

	for ( i = 0, anim = globalScriptData->animations; i < globalScriptData->numAnimations; i++, anim++ ) {
		if ( hash == anim->hash && !Q_stricmp( string, anim->name ) ) {
			return anim;
		}
	}

	Com_Error( ERR_DROP, "\x15" "BG_AnimationForString: unknown player animation '%s'", string );
	return NULL;    // shutup compiler
}

/*
=================
BG_IndexForString

  errors out if no match found
=================
*/
int BG_IndexForString( char *token, animStringItem_t *strings, qboolean allowFail ) {
	int i, hash;
	animStringItem_t *strav;

	hash = BG_StringHashValue( token );

	for ( i = 0, strav = strings; strav->string; strav++, i++ ) {
		if ( strav->hash == -1 ) {
			strav->hash = BG_StringHashValue( strav->string );
		}
		if ( hash == strav->hash && !Q_stricmp( token, strav->string ) ) {
			return i;
		}
	}

	if ( !allowFail ) {
		BG_AnimParseError( "BG_IndexForString: unknown token '%s'", token );
	}

	return -1;
}

/*
===============
BG_CopyStringIntoBuffer
===============
*/
char *BG_CopyStringIntoBuffer( const char *string, char *buffer, unsigned int bufSize, int *offset ) {
	char *pch;

	// check for overloaded buffer
	if ( *offset + strlen( string ) + 1 >= bufSize ) {
		BG_AnimParseError( "BG_CopyStringIntoBuffer: out of buffer space" );
	}

	pch = &buffer[*offset];

	// safe to do a strcpy since we've already checked for overrun
	strcpy( pch, string );

	// move the offset along
	*offset += strlen( string ) + 1;

	return pch;
}

/*
============
BG_InitWeaponStrings

  Builds the list of weapon names from the parsed weapon files.  Slot 0 is the
  "none" entry the script uses for an unarmed player.
============
*/
void BG_InitWeaponStrings( void ) {
	int i;

	memset( weaponStrings, 0, sizeof( weaponStrings ) );

	weaponStrings[0].string = "none";
	weaponStrings[0].hash = BG_StringHashValue( weaponStrings[0].string );

	for ( i = 1; i <= bg_numWeapons; i++ ) {
		weaponStrings[i].string = bg_weaponInfo[i]->szInternalName;
		weaponStrings[i].hash = BG_StringHashValue( weaponStrings[i].string );
	}

	weaponStringsInited = qtrue;
}

/*
=================
BG_LoadAnimForAnimIndex

  Finds the runtime animation list entry that carries this animation index.
=================
*/
animScriptAnim_t *BG_LoadAnimForAnimIndex( unsigned int animIndex ) {
	int i;

	if ( animIndex >= (unsigned int)globalScriptData->numAnimations ) {
		Com_Error( ERR_DROP, "\x15" "Player animation index %i out of 0 to %i range",
				   animIndex, globalScriptData->numAnimations );
	}

	for ( i = 0; i < *globalNumAnims; i++ ) {
		if ( animIndex == globalAnims[i].anim.index ) {
			return &globalAnims[i];
		}
	}

	return NULL;
}

/*
=================
BG_SetupAnimNoteTypes

  Marks every animation the RELOAD event script plays.  +0x58 of animation_t is
  still unknown_ in bg_public.h; the two stores are at 0x200014C1/0x200014D2.
=================
*/
void BG_SetupAnimNoteTypes( animScriptData_t *scriptData ) {
	int i, j;
	animScript_t *script;
	animScriptItem_t *item;
	animScriptCommand_t *command;

	for ( i = 0; i < scriptData->numAnimations; i++ ) {
		scriptData->animations[i].noteType = 0;
	}

	script = &scriptData->scriptEvents[ANIM_ET_RELOAD];
	for ( i = 0; i < script->numItems; i++ ) {
		item = script->items[i];
		for ( j = 0; j < item->numCommands; j++ ) {
			command = &item->commands[j];
			if ( command->bodyPart[0] ) {
				scriptData->animations[command->animIndex[0]].noteType = 1;
			}
			if ( command->bodyPart[1] ) {
				scriptData->animations[command->animIndex[1]].noteType = 1;
			}
		}
	}
}

/*
=================
BG_FinalizePlayerAnims

  CoD's replacement for RTCW's BG_AnimParseAnimConfig: the animation table is
  built out of the script anim tree instead of an .animcfg, then the script
  itself is parsed on top of it.
=================
*/
void BG_FinalizePlayerAnims( void ) {
	animScriptData_t *scriptData;
	animScriptAnim_t *listAnim;
	animation_t *anim;
	scr_anim_t treeAnim;
	int tree;
	int numAnims;
	int i;
	int len;
	float deltaRot[2];      /* the retail frame reserves exactly two floats here */
	vec3_t deltaPos;

	scriptData = globalScriptData;
	tree = scriptData->animTreeIndex;
	treeAnim.anims = (unsigned short)Scr_GetAnimsIndex( tree );

	numAnims = trap_XAnimGetAnimTreeSize( (void *)tree );
	globalScriptData->numAnimations = numAnims;

	scriptData->animations[0].flags |= 0x101;
	strncpy( scriptData->animations[0].name, "root", 63 );
	scriptData->animations[0].name[63] = 0;
	scriptData->animations[0].hash = 0;

	for ( i = 1, anim = &scriptData->animations[1]; i < numAnims; i++, anim++ ) {
		listAnim = BG_LoadAnimForAnimIndex( i );
		if ( !listAnim ) {
			anim->flags |= 0x100;
			strncpy( anim->name, "unused", 63 );
			anim->name[63] = 0;
			anim->hash = 0;
			continue;
		}

		treeAnim.index = (unsigned short)i;

		if ( !trap_XAnimIsPrimitive( treeAnim ) ) {
			anim->flags |= 0x001;
			strncpy( anim->name, listAnim->name, 63 );
			anim->name[63] = 0;
			anim->hash = listAnim->hash;
			if ( !anim->blendTime ) {
				anim->blendTime = -1;
			}
			anim->duration = 0;
			anim->moveSpeed = 0;
			continue;
		}

		strncpy( anim->name, trap_XAnimGetAnimName( treeAnim ), 63 );
		anim->name[63] = 0;
		anim->hash = BG_StringHashValue( anim->name );
		if ( !anim->blendTime ) {
			anim->blendTime = -1;
		}

		anim->duration = trap_XAnimGetLength( (void *)tree, (unsigned short)i );
		if ( anim->duration ) {
			trap_XAnimGetRelDelta( treeAnim, deltaRot, deltaPos, 0.0f, 1.0f );
			len = (int)VectorLength( deltaPos );
			if ( len ) {
				anim->moveSpeed = 1000 * len / anim->duration;
			} else {
				anim->moveSpeed = 0;
			}
		} else {
			anim->moveSpeed = 0;
		}

		if ( anim->duration < 500 ) {
			anim->duration = 500;
		}

		if ( trap_XAnimIsLooped( treeAnim ) ) {
			anim->flags |= 0x080;
		}
	}

	BG_AnimParseAnimScript( globalScriptData, NULL, NULL );
	BG_SetupAnimNoteTypes( globalScriptData );
}

/*
=================
BG_ParseConditionBits

  convert the string into a single int containing bit flags, stopping at a ','
  or end of line
=================
*/
void BG_ParseConditionBits( char **text_pp, animStringItem_t *stringTable, int condIndex, int result[2] ) {
	qboolean endFlag = qfalse;
	int indexFound;
	int tempBits[2];
	char currentString[64];
	qboolean minus = qfalse;
	char *token;

	currentString[0] = '\0';
	memset( result, 0, sizeof( result ) );      /* RTCW quirk: clears result[0] only */
	memset( tempBits, 0, sizeof( tempBits ) );

	while ( !endFlag ) {

		token = Com_ParseOnLine( text_pp );
		if ( !token || !token[0] ) {
			Com_UngetToken();   // go back to the previous token
			endFlag = qtrue;    // done parsing indexes
			if ( !strlen( currentString ) ) {
				break;
			}
		}

		if ( !Q_stricmp( token, "," ) ) {
			endFlag = qtrue;    // end of indexes
		}

		if ( !Q_stricmp( token, "none" ) ) { // first bit is always the "unused" bit
			result[0] |= 1;
			continue;
		}

		if ( !Q_stricmp( token, "none," ) ) {    // first bit is always the "unused" bit
			result[0] |= 1;
			endFlag = qtrue;    // end of indexes
			continue;
		}

		if ( !Q_stricmp( token, "NOT" ) ) {
			token = "MINUS"; // NOT is equivalent to MINUS
		}

		if ( !endFlag && Q_stricmp( token, "AND" ) && Q_stricmp( token, "MINUS" ) ) { // must be an index
			// check for a comma (end of indexes)
			if ( token[strlen( token ) - 1] == ',' ) {
				endFlag = qtrue;
				token[strlen( token ) - 1] = '\0';
			}
			// append this to the currentString
			if ( strlen( currentString ) ) {
				Q_strcat( currentString, sizeof( currentString ), " " );
			}
			Q_strcat( currentString, sizeof( currentString ), token );
		}

		if ( !Q_stricmp( token, "AND" ) || !Q_stricmp( token, "MINUS" ) || endFlag ) {
			// process the currentString
			if ( !strlen( currentString ) ) {
				if ( endFlag ) {
					BG_AnimParseError( "BG_ParseConditionBits: unexpected end of condition" );
				} else {
					// check for minus indexes to follow
					if ( !Q_stricmp( token, "MINUS" ) ) {
						minus = qtrue;
						continue;
					}
					BG_AnimParseError( "BG_ParseConditionBits: unexpected '%s'", token );
				}
			}
			if ( !Q_stricmp( currentString, "all" ) ) {
				tempBits[0] = ~0x0;
				tempBits[1] = ~0x0;
			} else {
				// first check this string with our defines
				indexFound = BG_IndexForString( currentString, defineStr[condIndex], qtrue );
				if ( indexFound >= 0 ) {
					// we have precalculated the bitflags for the defines
					tempBits[0] = defineBits[condIndex][indexFound][0];
					tempBits[1] = defineBits[condIndex][indexFound][1];
				} else {
					// convert the string into an index
					indexFound = BG_IndexForString( currentString, stringTable, qfalse );
					// convert the index into a bitflag
					tempBits[indexFound >> 5] |= 1 << ( indexFound & 31 );
				}
			}
			// perform operation
			if ( minus ) {    // subtract
				result[0] &= ~tempBits[0];
				result[1] &= ~tempBits[1];
			} else {        // add
				result[0] |= tempBits[0];
				result[1] |= tempBits[1];
			}
			// clear the currentString
			currentString[0] = '\0';
			// check for minus indexes to follow
			if ( !Q_stricmp( token, "MINUS" ) ) {
				minus = qtrue;
			}
		}

	}
}

/*
=================
BG_ParseConditions

  returns qtrue if everything went ok, error drops otherwise
=================
*/
qboolean BG_ParseConditions( char **text_pp, animScriptItem_t *scriptItem ) {
	int conditionIndex, conditionValue[2];
	char    *token;

	conditionValue[0] = 0;
	conditionValue[1] = 0;

	while ( 1 ) {

		token = Com_ParseOnLine( text_pp );
		if ( !token || !token[0] ) {
			break;
		}

		// special case, "default" has no conditions
		if ( !Q_stricmp( token, "default" ) ) {
			return qtrue;
		}

		conditionIndex = BG_IndexForString( token, animConditionsStr, qfalse );

		switch ( animConditionsTable[conditionIndex].type ) {
		case ANIM_CONDTYPE_BITFLAGS:
			BG_ParseConditionBits( text_pp, animConditionsTable[conditionIndex].values, conditionIndex, conditionValue );
			break;
		case ANIM_CONDTYPE_VALUE:
			if ( animConditionsTable[conditionIndex].values ) {
				token = Com_ParseOnLine( text_pp );
				if ( !token || !token[0] ) {
					BG_AnimParseError( "BG_ParseConditions: expected condition value, found end of line" );
				}
				// check for a comma (condition divider)
				if ( token[strlen( token ) - 1] == ',' ) {
					token[strlen( token ) - 1] = '\0';
				}
				conditionValue[0] = BG_IndexForString( token, animConditionsTable[conditionIndex].values, qfalse );
			} else {
				conditionValue[0] = 1;  // not used, just check for a positive condition
			}
			break;
		default:
			break;
		}

		// now append this condition to the item
		scriptItem->conditions[scriptItem->numConditions].index = conditionIndex;
		scriptItem->conditions[scriptItem->numConditions].value[0] = conditionValue[0];
		scriptItem->conditions[scriptItem->numConditions].value[1] = conditionValue[1];
		scriptItem->numConditions++;
	}

	if ( scriptItem->numConditions == 0 ) {
		BG_AnimParseError( "BG_ParseConditions: no conditions found" );
	}

	return qtrue;
}

/*
=================
BG_ParseCommands

  As well as filling in the script item, this is where an animation picks up
  everything the client has to be able to derive from it later: the movetype
  bit set, the ladder/strafe/firing/death flags and the per-instance blendtime.
  All of that is skipped when a runtime animation list is attached.
=================
*/
void BG_ParseCommands( char **input, animScriptItem_t *scriptItem, animScriptData_t *scriptData ) {
	char    *token;
	animScriptCommand_t *command = NULL;
	animation_t *anim;
	int partIndex = 0;
	int i;

	while ( 1 ) {

		// parse the body part
		token = ( partIndex < 1 ) ? Com_Parse( input ) : Com_ParseOnLine( input );
		if ( !token || !token[0] ) {
			break;
		}
		if ( !Q_stricmp( token, "}" ) ) {
			// unget the bracket and get out of here
			*input -= strlen( token );
			break;
		}

		// new command?
		if ( partIndex == 0 ) {
			// have we exceeded the maximum number of commands?
			if ( scriptItem->numCommands >= MAX_ANIMSCRIPT_ANIMCOMMANDS ) {
				BG_AnimParseError( "BG_ParseCommands: exceeded maximum number of animations (%i)", MAX_ANIMSCRIPT_ANIMCOMMANDS );
			}
			command = &scriptItem->commands[scriptItem->numCommands++];
			memset( command, 0, sizeof( command ) );    /* RTCW quirk: clears 4 bytes */
		}

		command->bodyPart[partIndex] = BG_IndexForString( token, animBodyPartsStr, qtrue );
		if ( command->bodyPart[partIndex] > 0 ) {
			// parse the animation
			token = Com_ParseOnLine( input );
			if ( !token || !token[0] ) {
				BG_AnimParseError( "BG_ParseCommands: expected animation" );
			}
			command->animIndex[partIndex] = BG_AnimationIndexForString( token );
			anim = &scriptData->animations[command->animIndex[partIndex]];
			command->animDuration[partIndex] = (short)anim->duration;

			if ( !globalAnims ) {
				// if this is a locomotion, set the movetype of the animation so we can
				// reverse engineer the movetype from the animation, on the client
				if ( parseMovetype != ANIM_MT_UNUSED && command->bodyPart[partIndex] != ANIM_BP_TORSO ) {
					anim->movetype |= 1 << parseMovetype;

					if ( parseMovetype == ANIM_MT_CLIMBUP || parseMovetype == ANIM_MT_CLIMBDOWN ) {
						if ( scriptData->animations[command->animIndex[partIndex]].moveSpeed ) {
							scriptData->animations[command->animIndex[partIndex]].flags |= 0x002;
						}
					}

					for ( i = 0; i < scriptItem->numConditions; i++ ) {
						if ( scriptItem->conditions[i].index != ANIM_COND_STRAFING ) {
							continue;
						}
						if ( scriptItem->conditions[i].value[0] == STRAFING_LEFT ) {
							scriptData->animations[command->animIndex[partIndex]].flags |= 0x010;
						} else if ( scriptItem->conditions[i].value[0] == STRAFING_RIGHT ) {
							scriptData->animations[command->animIndex[partIndex]].flags |= 0x020;
						}
						break;
					}
				}
				// if this is a fireweapon event, then this is a firing animation
				if ( parseEvent == ANIM_ET_FIREWEAPON ) {
					scriptData->animations[command->animIndex[partIndex]].flags |= 0x008;
					scriptData->animations[command->animIndex[partIndex]].blendTime = 30;
				} else if ( parseEvent == ANIM_ET_DEATH ) {
					scriptData->animations[command->animIndex[partIndex]].moveSpeed = 0;
					scriptData->animations[command->animIndex[partIndex]].flags |= 0x040;
				}
			}

			// check for parameters attached to this animation instance
			while ( 1 ) {
				token = Com_ParseOnLine( input );
				if ( !token || !token[0] ) {
					Com_UngetToken();
					break;
				}

				if ( !Q_stricmp( token, "duration" ) ) {
					token = Com_ParseOnLine( input );
					if ( !token || !token[0] ) {
						BG_AnimParseError( "BG_ParseCommands: expected duration value" );
					}
					command->animDuration[partIndex] = (short)atoi( token );
					continue;
				}

				if ( !Q_stricmp( token, "turretanim" ) ) {
					if ( !globalAnims ) {
						scriptData->animations[command->animIndex[partIndex]].flags |= 0x004;
					}
					if ( command->bodyPart[partIndex] != ANIM_BP_BOTH ) {
						BG_AnimParseError( "BG_ParseCommands: Turret animations can only be played on the 'both' body part" );
					}
					continue;
				}

				if ( !Q_stricmp( token, "blendtime" ) ) {
					token = Com_ParseOnLine( input );
					if ( !token || !token[0] ) {
						BG_AnimParseError( "BG_ParseCommands: expected blendtime value" );
					}
					if ( !globalAnims ) {
						scriptData->animations[command->animIndex[partIndex]].blendTime = atoi( token );
					}
					continue;
				}

				// unget the token
				Com_UngetToken();
				break;
			}

			if ( command->bodyPart[partIndex] != ANIM_BP_BOTH && partIndex++ < 1 ) {
				continue;   // allow parsing of another bodypart
			}
		} else {
			// unget the token
			*input -= strlen( token );
		}

		// parse optional parameters (sounds, etc)
		while ( 1 ) {
			/* Com_ParseOnLine, and it MUST be: the loop above ungets at end of
			   line, and Com_ParseExt would not restore -- the data pointer is
			   already past the newline, so the raw read returns the next line's
			   `}` and this loop calls it an unknown parameter. */
			token = Com_ParseOnLine( input );
			if ( !token || !token[0] ) {
				break;
			}

			if ( !Q_stricmp( token, "sound" ) ) {

				token = Com_ParseOnLine( input );
				if ( !token || !token[0] ) {
					BG_AnimParseError( "BG_ParseCommands: expected sound" );
				}
				// NOTE: only sound scripts are supported at this stage
				if ( strstr( token, ".wav" ) ) {
					BG_AnimParseError( "BG_ParseCommands: wav files not supported, only sound scripts" );
				}
				command->soundIndex = globalScriptData->soundIndex( token );

			} else {
				// unknown??
				BG_AnimParseError( "BG_ParseCommands: unknown parameter '%s'", token );
			}
		}

		partIndex = 0;
	}
}

/*
=================
BG_AnimParseAnimScript

  Parse the animation script for this model, converting it into run-time
  structures
=================
*/

typedef enum
{
	PARSEMODE_DEFINES,
	PARSEMODE_ANIMATION,
	PARSEMODE_CANNED_ANIMATIONS,
	PARSEMODE_STATECHANGES,
	PARSEMODE_EVENTS
} animScriptParseMode_t;

static animStringItem_t animParseModesStr[] =
{
	{"defines", -1},
	{"animations", -1},
	{"canned_animations", -1},
	{"statechanges", -1},
	{"events", -1},

	{NULL, -1},
};

void BG_AnimParseAnimScript( animScriptData_t *scriptData, void *anims, int *numAnims ) {
	#define MAX_INDENT_LEVELS   3

	char    *text_p, *token;
	animScriptParseMode_t parseMode;
	animScript_t        *currentScript;
	animScriptItem_t tempScriptItem;
	animScriptItem_t *currentScriptItem = NULL;
	int indexes[MAX_INDENT_LEVELS], indentLevel, oldState, newParseMode;
	int i, defineType;
	fileHandle_t f;
	int len;

	// the script is read once and kept for the life of the module
	if ( !animScriptLoaded ) {
		len = trap_FS_FOpenFile( globalFilename, &f, FS_READ );
		if ( len <= 0 ) {
			Com_Error( ERR_DROP, "\x15" "Couldn't load player animation script %s\n", globalFilename );
		}
		if ( (unsigned int)len >= sizeof( globalScriptText ) - 1 ) {
			Com_Error( ERR_DROP, "\x15" "Couldn't load player animation script %s\n", globalFilename );
		}
		trap_FS_Read( globalScriptText, len, f );
		globalScriptText[len] = 0;
		trap_FS_FCloseFile( f );
		animScriptLoaded = qtrue;
	}

	// the scriptData passed into here must be the one this binary is using
	globalScriptData = scriptData;
	globalAnims = anims;
	globalNumAnims = numAnims;

	// start at the defines
	parseMode = PARSEMODE_DEFINES;

	if ( !weaponStringsInited ) {
		BG_InitWeaponStrings();
	}

	// init the global defines
	memset( defineStr, 0, sizeof( defineStr ) );
	memset( defineStrings, 0, sizeof( defineStrings ) );
	memset( numDefines, 0, sizeof( numDefines ) );
	defineStringsOffset = 0;

	for ( i = 0; i < MAX_INDENT_LEVELS; i++ )
		indexes[i] = -1;
	indentLevel = 0;
	currentScript = NULL;

	text_p = globalScriptText;
	Com_BeginParseSession( "BG_AnimParseAnimScript" );

	while ( 1 ) {

		token = Com_Parse( &text_p );
		if ( !token || !token[0] ) {
			if ( indentLevel ) {
				BG_AnimParseError( "BG_AnimParseAnimScript: unexpected end of file: %s" );
			}
			break;
		}

		// check for a new section
		newParseMode = BG_IndexForString( token, animParseModesStr, qtrue );
		if ( newParseMode >= 0 ) {
			if ( indentLevel ) {
				BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );
			}

			parseMode = newParseMode;
			parseMovetype = ANIM_MT_UNUSED;
			parseEvent = -1;
			continue;
		}

		switch ( parseMode ) {

		case PARSEMODE_DEFINES:

			if ( !Q_stricmp( token, "set" ) ) {

				// read in the define type
				token = Com_ParseOnLine( &text_p );
				if ( !token || !token[0] ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: expected condition type string" );
				}
				defineType = BG_IndexForString( token, animConditionsStr, qfalse );
				if ( animConditionsTable[defineType].type != ANIM_CONDTYPE_BITFLAGS ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: can not make a define of type '%s'", token );
				}

				// read in the define
				token = Com_ParseOnLine( &text_p );
				if ( !token || !token[0] ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: expected condition define string" );
				}

				// copy the define to the strings list
				defineStr[defineType][numDefines[defineType]].string =
					BG_CopyStringIntoBuffer( token, defineStrings, sizeof( defineStrings ), &defineStringsOffset );
				defineStr[defineType][numDefines[defineType]].hash =
					BG_StringHashValue( defineStr[defineType][numDefines[defineType]].string );

				// expecting an =
				token = Com_ParseOnLine( &text_p );
				if ( !token ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: expected '=', found end of line" );
				}
				if ( Q_stricmp( token, "=" ) ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: expected '=', found '%s'", token );
				}

				// parse the bits
				BG_ParseConditionBits( &text_p, animConditionsTable[defineType].values, defineType,
									   defineBits[defineType][numDefines[defineType]] );
				numDefines[defineType]++;
			}

			break;

		case PARSEMODE_ANIMATION:
		case PARSEMODE_CANNED_ANIMATIONS:

			if ( !Q_stricmp( token, "{" ) ) {

				// about to increment indent level, check that we have enough information to do this
				if ( indentLevel >= MAX_INDENT_LEVELS ) { // too many indentations
					BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );
				}
				if ( indexes[indentLevel] < 0 ) {     // we havent found out what this new group is yet
					BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );
				}
				indentLevel++;

			} else if ( !Q_stricmp( token, "}" ) ) {

				// reduce the indentLevel
				indentLevel--;
				if ( indentLevel < 0 ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );
				}
				if ( indentLevel == 1 ) {
					currentScript = NULL;
				}
				// make sure we read a new index before next indent
				indexes[indentLevel] = -1;

			} else if ( indentLevel == 0 && indexes[indentLevel] < 0 ) {

				if ( Q_stricmp( token, "state" ) ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: expected 'state'" );
				}

				// read in the state type
				token = Com_ParseOnLine( &text_p );
				if ( !token ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: expected state type" );
				}
				indexes[indentLevel] = BG_IndexForString( token, animStateStr, qfalse );

				// check for the open bracket
				token = Com_Parse( &text_p );
				if ( !token || Q_stricmp( token, "{" ) ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: expected '{'" );
				}
				indentLevel++;

			} else if ( indentLevel == 1 && indexes[indentLevel] < 0 ) {

				// we are expecting a movement type
				indexes[indentLevel] = BG_IndexForString( token, animMoveTypesStr, qfalse );
				if ( parseMode == PARSEMODE_ANIMATION ) {
					currentScript = &scriptData->scriptAnims[indexes[0]][indexes[1]];
					parseMovetype = indexes[1];
				} else if ( parseMode == PARSEMODE_CANNED_ANIMATIONS ) {
					currentScript = &scriptData->scriptCannedAnims[indexes[0]][indexes[1]];
				}
				memset( currentScript, 0, sizeof( *currentScript ) );

			} else if ( indentLevel == 2 && indexes[indentLevel] < 0 ) {

				// we are expecting a condition specifier
				// move the text_p backwards so we can read in the last token again
				text_p -= strlen( token );
				// sanity check that
				if ( Q_strncmp( text_p, token, strlen( token ) ) ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: internal error" );
				}
				memset( &tempScriptItem, 0, sizeof( tempScriptItem ) );
				indexes[indentLevel] = BG_ParseConditions( &text_p, &tempScriptItem );
				// do we have enough room in this script for another item?
				if ( currentScript->numItems >= MAX_ANIMSCRIPT_ITEMS ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: exceeded maximum items per script (%i)", MAX_ANIMSCRIPT_ITEMS );
				}
				// are there enough items left in the global list?
				if ( scriptData->numScriptItems >= MAX_ANIMSCRIPT_ITEMS_PER_MODEL ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: exceeded maximum global items (%i)", MAX_ANIMSCRIPT_ITEMS_PER_MODEL );
				}
				// it was parsed ok, so grab an item from the global list to use
				currentScript->items[currentScript->numItems] = &scriptData->scriptItems[scriptData->numScriptItems++];
				currentScriptItem = currentScript->items[currentScript->numItems];
				currentScript->numItems++;
				// copy the data across from the temp script item
				*currentScriptItem = tempScriptItem;

			} else if ( indentLevel == 3 ) {

				// we are reading the commands, so parse this line as if it were a command
				text_p -= strlen( token );
				if ( Q_strncmp( text_p, token, strlen( token ) ) ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: internal error" );
				}
				BG_ParseCommands( &text_p, currentScriptItem, scriptData );

			} else {

				// huh ??
				BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );

			}

			break;

		case PARSEMODE_STATECHANGES:
		case PARSEMODE_EVENTS:

			if ( !Q_stricmp( token, "{" ) ) {

				if ( indentLevel >= MAX_INDENT_LEVELS ) { // too many indentations
					BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );
				}
				if ( indexes[indentLevel] < 0 ) {     // we havent found out what this new group is yet
					BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );
				}
				indentLevel++;

			} else if ( !Q_stricmp( token, "}" ) ) {

				indentLevel--;
				if ( indentLevel < 0 ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );
				}
				if ( indentLevel == 0 ) {
					currentScript = NULL;
				}
				indexes[indentLevel] = -1;

			} else if ( indentLevel == 0 && indexes[indentLevel] < 0 ) {

				if ( parseMode == PARSEMODE_STATECHANGES ) {

					if ( Q_stricmp( token, "statechange" ) ) {
						BG_AnimParseError( "BG_AnimParseAnimScript: expected 'statechange', got '%s'", token );
					}

					// read in the old state type
					token = Com_ParseOnLine( &text_p );
					if ( !token ) {
						BG_AnimParseError( "BG_AnimParseAnimScript: expected <state type>" );
					}
					oldState = BG_IndexForString( token, animStateStr, qfalse );

					// read in the new state type
					token = Com_ParseOnLine( &text_p );
					if ( !token ) {
						BG_AnimParseError( "BG_AnimParseAnimScript: expected <state type>" );
					}
					indexes[indentLevel] = BG_IndexForString( token, animStateStr, qfalse );

					currentScript = &scriptData->scriptStateChange[oldState][indexes[indentLevel]];

					// check for the open bracket
					token = Com_Parse( &text_p );
					if ( !token || Q_stricmp( token, "{" ) ) {
						BG_AnimParseError( "BG_AnimParseAnimScript: expected '{'" );
					}
					indentLevel++;

				} else {

					// read in the event type
					indexes[indentLevel] = BG_IndexForString( token, animEventTypesStr, qfalse );
					currentScript = &scriptData->scriptEvents[indexes[0]];

					parseEvent = indexes[indentLevel];

				}

				memset( currentScript, 0, sizeof( *currentScript ) );

			} else if ( indentLevel == 1 && indexes[indentLevel] < 0 ) {

				// we are expecting a condition specifier
				text_p -= strlen( token );
				if ( Q_strncmp( text_p, token, strlen( token ) ) ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: internal error" );
				}
				memset( &tempScriptItem, 0, sizeof( tempScriptItem ) );
				indexes[indentLevel] = BG_ParseConditions( &text_p, &tempScriptItem );
				if ( currentScript->numItems >= MAX_ANIMSCRIPT_ITEMS ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: exceeded maximum items per script (%i)", MAX_ANIMSCRIPT_ITEMS );
				}
				if ( scriptData->numScriptItems >= MAX_ANIMSCRIPT_ITEMS_PER_MODEL ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: exceeded maximum global items (%i)", MAX_ANIMSCRIPT_ITEMS_PER_MODEL );
				}
				currentScript->items[currentScript->numItems] = &scriptData->scriptItems[scriptData->numScriptItems++];
				currentScriptItem = currentScript->items[currentScript->numItems];
				currentScript->numItems++;
				*currentScriptItem = tempScriptItem;

			} else if ( indentLevel == 2 ) {

				text_p -= strlen( token );
				if ( Q_strncmp( text_p, token, strlen( token ) ) ) {
					BG_AnimParseError( "BG_AnimParseAnimScript: internal error" );
				}
				BG_ParseCommands( &text_p, currentScriptItem, scriptData );

			} else {

				// huh ??
				BG_AnimParseError( "BG_AnimParseAnimScript: unexpected '%s'", token );

			}

			break;

		default:
			break;
		}

	}

	globalFilename = NULL;

	Com_EndParseSession();
}

//------------------------------------------------------------------------
//
// run-time gameplay functions, these are called during gameplay, so they must be
// cpu efficient.
//

/*
===============
BG_EvaluateConditions

  returns qfalse if the set of conditions fails, qtrue otherwise
===============
*/
qboolean BG_EvaluateConditions( clientInfo_t *ci, animScriptItem_t *scriptItem ) {
	int i;
	animScriptCondition_t *cond;

	for ( i = 0, cond = scriptItem->conditions; i < scriptItem->numConditions; i++, cond++ )
	{
		switch ( animConditionsTable[cond->index].type ) {
		case ANIM_CONDTYPE_BITFLAGS:
			if ( !( ci->conditions[cond->index][0] & cond->value[0] ) &&
				 !( ci->conditions[cond->index][1] & cond->value[1] ) ) {
				return qfalse;
			}
			break;
		case ANIM_CONDTYPE_VALUE:
			if ( !( ci->conditions[cond->index][0] == cond->value[0] ) ) {
				return qfalse;
			}
			break;
		default:
			break;
		}
	}

	// all conditions must have passed
	return qtrue;
}

/*
===============
BG_FirstValidItem

  scroll through the script items, returning the first script found to pass all
  conditions

  returns NULL if no match found
===============
*/
animScriptItem_t *BG_FirstValidItem( int client, animScript_t *script ) {
	animScriptItem_t **ppScriptItem;
	int i;

	for ( i = 0, ppScriptItem = script->items; i < script->numItems; i++, ppScriptItem++ )
	{
		if ( BG_EvaluateConditions( &bg_clientinfo[client], *ppScriptItem ) ) {
			return *ppScriptItem;
		}
	}

	return NULL;
}

/*
===============
BG_PlayAnim
===============
*/
int BG_PlayAnim( playerState_t *ps, int animNum, animBodyPart_t bodyPart, int forceDuration,
				 qboolean setTimer, qboolean isContinue, qboolean force ) {
	int duration;
	qboolean wasSet = qfalse;

	if ( forceDuration ) {
		duration = forceDuration;
	} else {
		duration = globalScriptData->animations[animNum].duration + 50;   // account for lerping between anims
	}

	switch ( bodyPart ) {
	case ANIM_BP_BOTH:
	case ANIM_BP_LEGS:

		if ( ps->legsTimer < 50 || force ) {
			if ( !isContinue || ( ps->legsAnim & ~0x200 ) != animNum ) {
				wasSet = qtrue;
				ps->legsAnim = ( ( ps->legsAnim & 0x200 ) ^ 0x200 ) | animNum;
				if ( setTimer ) {
					ps->legsTimer = duration;
				}
			} else if ( setTimer && ( globalScriptData->animations[animNum].flags & 0x080 ) ) {
				ps->legsTimer = duration;
			}
		}

		if ( bodyPart == ANIM_BP_LEGS ) {
			break;
		}
		animNum = 0;    /* the retail code falls through with the index cleared */

	case ANIM_BP_TORSO:

		if ( ps->torsoTimer < 50 || force ) {
			if ( !isContinue || ( ps->torsoAnim & ~0x200 ) != animNum ) {
				ps->torsoAnim = ( ( ps->torsoAnim & 0x200 ) ^ 0x200 ) | animNum;
				if ( setTimer ) {
					ps->torsoTimer = duration;
				}
			} else if ( setTimer && ( globalScriptData->animations[animNum].flags & 0x080 ) ) {
				ps->torsoTimer = duration;
			}
		}

		break;
	default:
		break;
	}

	if ( !wasSet ) {
		return -1;
	}

	return duration;
}

/*
===============
BG_PlayAnimName
===============
*/
int BG_PlayAnimName( playerState_t *ps, char *animName, animBodyPart_t bodyPart,
					 qboolean setTimer, qboolean isContinue, qboolean force ) {
	return BG_PlayAnim( ps, BG_AnimationIndexForString( animName ), bodyPart, 0, setTimer, isContinue, force );
}

/*
===============
BG_ExecuteCommand

  returns the duration of the animation, -1 if no anim was set
===============
*/
int BG_ExecuteCommand( playerState_t *ps, animScriptCommand_t *scriptCommand,
					   qboolean setTimer, qboolean isContinue, qboolean force ) {
	int duration = -1;
	qboolean playedLegsAnim = qfalse;

	if ( scriptCommand->bodyPart[0] ) {
		duration = scriptCommand->animDuration[0] + 50;
		if ( scriptCommand->bodyPart[0] == ANIM_BP_LEGS || scriptCommand->bodyPart[0] == ANIM_BP_BOTH ) {
			playedLegsAnim = ( BG_PlayAnim( ps, scriptCommand->animIndex[0], scriptCommand->bodyPart[0],
											duration, setTimer, isContinue, force ) > -1 );
		} else {
			BG_PlayAnim( ps, scriptCommand->animIndex[0], scriptCommand->bodyPart[0],
						 duration, setTimer, isContinue, force );
		}
	}
	if ( scriptCommand->bodyPart[1] ) {
		duration = scriptCommand->animDuration[0] + 50;
		/* the retail code tests bodyPart[0] here, not [1] */
		if ( scriptCommand->bodyPart[0] == ANIM_BP_LEGS || scriptCommand->bodyPart[0] == ANIM_BP_BOTH ) {
			playedLegsAnim = ( BG_PlayAnim( ps, scriptCommand->animIndex[1], scriptCommand->bodyPart[1],
											duration, setTimer, isContinue, force ) > -1 );
		} else {
			BG_PlayAnim( ps, scriptCommand->animIndex[1], scriptCommand->bodyPart[1],
						 duration, setTimer, isContinue, force );
		}
	}

	if ( scriptCommand->soundIndex ) {
		/* Two arguments, clientNum first -- game 0x20002FF0 and cgame
		   0x30003000 emit the identical `push soundIndex; push clientNum;
		   call [+0x9B6D4]; add esp,8`.  The cast goes away once
		   animScriptCommand_t.soundIndex is corrected to const char *. */
		globalScriptData->playSound( ps->clientNum, (const char *)scriptCommand->soundIndex );
	}

	if ( !playedLegsAnim ) {
		return -1;
	}

	return duration;
}

/*
================
BG_AnimScriptAnimation

  runs the normal locomotive animations

  returns 1 if an animation was set, -1 if no animation was found, 0 otherwise
================
*/
int BG_AnimScriptAnimation( playerState_t *ps, aistateEnum_t state, scriptAnimMoveTypes_t movetype,
							qboolean isContinue ) {
	animScript_t        *script = NULL;
	animScriptItem_t    *scriptItem = NULL;
	animScriptCommand_t *scriptCommand;

	if ( ps->pm_type >= PM_DEAD ) {
		return -1;
	}

	// try finding a match in all states below the given state
	while ( !scriptItem && state >= 0 ) {
		script = &globalScriptData->scriptAnims[state][movetype];
		if ( !script->numItems ) {
			state--;
			continue;
		}
		// find the first script item, that passes all the conditions for this event
		scriptItem = BG_FirstValidItem( ps->clientNum, script );
		if ( !scriptItem ) {
			state--;
			continue;
		}
	}

	if ( !scriptItem || !scriptItem->numCommands ) {
		return -1;
	}

	// save this as our current movetype
	BG_UpdateConditionValue( ps->clientNum, ANIM_COND_MOVETYPE, movetype, qtrue );

	// pick the correct animation for this character (animations must be constant for
	// each character, otherwise they'll constantly change)
	scriptCommand = &scriptItem->commands[ps->clientNum % scriptItem->numCommands];

	// run it
	return ( BG_ExecuteCommand( ps, scriptCommand, qfalse, isContinue, qfalse ) != -1 );
}

/*
================
BG_AnimScriptStateChange

  returns the duration in milliseconds that this model should be paused.  -1 if
  no anim found
================
*/
int BG_AnimScriptStateChange( playerState_t *ps, aistateEnum_t newState, aistateEnum_t oldState ) {
	animScript_t        *script;
	animScriptItem_t    *scriptItem;
	animScriptCommand_t *scriptCommand;

	if ( ps->pm_type >= PM_DEAD ) {
		return -1;
	}

	script = &globalScriptData->scriptStateChange[oldState][newState];
	if ( !script->numItems ) {
		return -1;
	}

	// find the first script item, that passes all the conditions for this event
	scriptItem = BG_FirstValidItem( ps->clientNum, script );
	if ( !scriptItem || !scriptItem->numCommands ) {
		return -1;
	}

	// pick a random command
	scriptCommand = &scriptItem->commands[rand() % scriptItem->numCommands];

	// run it
	return BG_ExecuteCommand( ps, scriptCommand, qtrue, qfalse, qfalse );
}

/*
================
BG_AnimScriptEvent

  returns the duration in milliseconds that this model should be paused.  -1 if
  no event found
================
*/
int BG_AnimScriptEvent( playerState_t *ps, scriptAnimEventTypes_t event, qboolean isContinue,
						qboolean force ) {
	animScript_t        *script;
	animScriptItem_t    *scriptItem;
	animScriptCommand_t *scriptCommand;

	if ( event != ANIM_ET_DEATH && ps->pm_type >= PM_DEAD ) {
		return -1;
	}

	script = &globalScriptData->scriptEvents[event];
	if ( !script->numItems ) {
		return -1;
	}

	// find the first script item, that passes all the conditions for this event
	scriptItem = BG_FirstValidItem( ps->clientNum, script );
	if ( !scriptItem || !scriptItem->numCommands ) {
		return -1;
	}

	// pick a random command
	scriptCommand = &scriptItem->commands[rand() % scriptItem->numCommands];

	// run it
	return BG_ExecuteCommand( ps, scriptCommand, qtrue, isContinue, force );
}

/*
===============
BG_GetAnimString
===============
*/
char *BG_GetAnimString( int anim ) {
	if ( (unsigned int)anim >= (unsigned int)globalScriptData->numAnimations ) {
		BG_AnimParseError( "BG_GetAnimString: anim index is out of range" );
	}

	return globalScriptData->animations[anim].name;
}

/*
==============
BG_UpdateConditionValue
==============
*/
void BG_UpdateConditionValue( int client, int condition, int value, qboolean checkConversion ) {
	if ( checkConversion ) {
		// we may need to convert to bitflags
		if ( animConditionsTable[condition].type == ANIM_CONDTYPE_BITFLAGS ) {
			// we want the explicit value passed in, and COM_BitSet ORs on top of
			// whatever is there, so clear it first
			bg_clientinfo[client].conditions[condition][0] = 0;
			bg_clientinfo[client].conditions[condition][1] = 0;

			bg_clientinfo[client].conditions[condition][value >> 5] |= 1 << ( value & 31 );
			return;
		}
	}

	bg_clientinfo[client].conditions[condition][0] = value;
}

/*
==============
BG_GetConditionValue
==============
*/
int BG_GetConditionValue( clientInfo_t *ci, int condition, qboolean checkConversion ) {
	int value, i;

	value = ci->conditions[condition][0];

	if ( checkConversion ) {
		// we may need to convert to a value
		if ( animConditionsTable[condition].type == ANIM_CONDTYPE_BITFLAGS ) {
			for ( i = 0; i < 8 * sizeof( ci->conditions[0] ); i++ ) {
				if ( ci->conditions[condition][i >> 5] & ( 1 << ( i & 31 ) ) ) {
					return i;
				}
			}
			// nothing found
			return 0;
		}
	}

	return value;
}

/*
================
BG_GetAnimScriptEvent

  returns the animation index the event would play, -1 if no event found
================
*/
int BG_GetAnimScriptEvent( playerState_t *ps, scriptAnimEventTypes_t event ) {
	animScript_t        *script;
	animScriptItem_t    *scriptItem;
	animScriptCommand_t *scriptCommand;

	if ( event != ANIM_ET_DEATH && ps->pm_type >= PM_DEAD ) {
		return -1;
	}

	script = &globalScriptData->scriptEvents[event];
	if ( !script->numItems ) {
		return -1;
	}

	scriptItem = BG_FirstValidItem( ps->clientNum, script );
	if ( !scriptItem || !scriptItem->numCommands ) {
		return -1;
	}

	scriptCommand = &scriptItem->commands[rand() % scriptItem->numCommands];

	return scriptCommand->animIndex[0];
}

/*
===============
BG_GetAnimationForIndex

  returns the animation_t for the given index
===============
*/
animation_t *BG_GetAnimationForIndex( int index ) {
	if ( (unsigned int)index >= (unsigned int)globalScriptData->numAnimations ) {
		Com_Error( ERR_DROP, "\x15" "BG_GetAnimationForIndex: index out of bounds" );
	}

	return &globalScriptData->animations[index];
}

/*
=================
BG_AnimUpdatePlayerStateConditions

  Refreshes the conditions the prediction path owns.  CoD dropped RTCW's
  EF_CROUCHING bookkeeping from here (BG_AnimPlayerConditions does it from the
  entity state instead) and added the weapon class and the ADS position.
=================
*/
void BG_AnimUpdatePlayerStateConditions( pmove_t *pmove ) {
	playerState_t *ps = pmove->ps;

	// WEAPON
	BG_UpdateConditionValue( ps->clientNum, ANIM_COND_WEAPONS, ps->weapon, qtrue );

	// WEAPONCLASS
	BG_UpdateConditionValue( ps->clientNum, ANIM_COND_WEAPONCLASS,
							 bg_weaponInfo[ps->weapon]->weaponClass, qtrue );

	// WEAPON_POSITION
	if ( ps->eFlags & EF_AIMING_DOWN_SIGHT ) {
		BG_UpdateConditionValue( ps->clientNum, ANIM_COND_WEAPON_POSITION, WEAPON_POSITION_ADS, qtrue );
	} else {
		BG_UpdateConditionValue( ps->clientNum, ANIM_COND_WEAPON_POSITION, WEAPON_POSITION_HIP, qtrue );
	}

	// MOUNTED
	if ( ps->eFlags & EF_TURRET_ACTIVE_MASK ) {
		BG_UpdateConditionValue( ps->clientNum, ANIM_COND_MOUNTED, MOUNTED_MG42, qtrue );
	} else {
		BG_UpdateConditionValue( ps->clientNum, ANIM_COND_MOUNTED, MOUNTED_UNUSED, qtrue );
	}

	// UNDERHAND
	BG_UpdateConditionValue( ps->clientNum, ANIM_COND_UNDERHAND, ps->viewangles[0] > 0, qtrue );

	// FIRING
	if ( pmove->cmd.buttons & BUTTON_ATTACK ) {
		BG_UpdateConditionValue( ps->clientNum, ANIM_COND_FIRING, qtrue, qtrue );
	} else {
		BG_UpdateConditionValue( ps->clientNum, ANIM_COND_FIRING, qfalse, qtrue );
	}
}

/*
===============
BG_IsCrouchingAnim

  IDLECR, WALKCR and WALKCRBK; RUNCR is deliberately not in the set.
===============
*/
qboolean BG_IsCrouchingAnim( int anim ) {
	animation_t *animation;

	animation = BG_GetAnimationForIndex( anim & ~0x200 );

	return ( animation->movetype
			 & ( ( 1 << ANIM_MT_IDLECR ) | ( 1 << ANIM_MT_WALKCR ) | ( 1 << ANIM_MT_WALKCRBK ) ) ) != 0;
}

/*
===============
BG_IsProneAnim
===============
*/
qboolean BG_IsProneAnim( int anim ) {
	animation_t *animation;

	animation = BG_GetAnimationForIndex( anim & ~0x200 );

	return ( animation->movetype
			 & ( ( 1 << ANIM_MT_IDLEPRONE ) | ( 1 << ANIM_MT_WALKPRONE ) | ( 1 << ANIM_MT_WALKPRONEBK ) ) ) != 0;
}

/*
==================
BG_SetNewAnimation

  Switches one slot to a new animation: picks the blend time, hands the anim
  tree the goal weights, and phases looping locomotion per client so a row of
  players does not march in lockstep.

  `corpse` is entityState_t.eFlags bit 0x800, which BG_RunLerpFrameRate shifts
  out (0x20003C0E); it only changes how a death animation is started.
==================
*/
void BG_SetNewAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, qboolean corpse ) {
	int oldAnimation;
	animation_t *oldAnim;
	animation_t *anim;
	qboolean isLegs;
	int index;
	int blendTime;
	scr_anim_t sa;
	void *tree;
	float startTime;

	oldAnimation = lf->animationNumber;
	oldAnim = lf->animation;
	isLegs = ( lf == &ci->legs );
	startTime = 0;

	lf->animationNumber = newAnimation;
	index = newAnimation & ~ANIM_TOGGLEBIT;
	if ( (unsigned int)index >= (unsigned int)bgs_animScriptData.numAnimations ) {
		Com_Error( ERR_DROP, "\x15" "Player animation index out of range (%i): %i",
				   bgs_animScriptData.numAnimations, index );
	}

	tree = ci->animTree;
	sa.anims = (unsigned short)Scr_GetAnimsIndex( bgs_animScriptData.animTreeIndex );

	if ( index ) {
		anim = &bgs_animScriptData.animations[index];
		lf->animation = anim;
		lf->animationTime = anim->blendTime;
	} else {
		anim = NULL;
		lf->animation = NULL;
		lf->animationTime = 200;
	}

	if ( !oldAnim && isLegs ) {
		// nothing was playing on the legs, so start the new one cold
		lf->animationTime = 0;
	} else if ( !anim || lf->animationTime <= 0 ) {
		if ( anim && isLegs
			 && ( BG_IsCrouchingAnim( index ) != BG_IsCrouchingAnim( oldAnimation )
				  || BG_IsProneAnim( index ) != BG_IsProneAnim( oldAnimation ) ) ) {
			// the stance changed under the legs
			if ( anim->moveSpeed
				 || ( anim->movetype & ( ( 1 << ANIM_MT_TURNRIGHT ) | ( 1 << ANIM_MT_TURNLEFT ) ) ) ) {
				blendTime = 200;
			} else {
				blendTime = 250;
			}
		} else if ( anim && anim->moveSpeed ) {
			blendTime = 120;
		} else if ( oldAnim && oldAnim->moveSpeed ) {
			blendTime = 250;
		} else {
			blendTime = 170;
		}
		if ( lf->animationTime < blendTime ) {
			lf->animationTime = blendTime;
		}
	}

	if ( anim && anim->moveSpeed ) {
		sa.index = (unsigned short)index;
		if ( trap_XAnimIsLooped( sa ) ) {
			sa.index = (unsigned short)( oldAnimation & ~ANIM_TOGGLEBIT );
			if ( oldAnim && oldAnim->moveSpeed && trap_XAnimIsLooped( sa ) ) {
				// carry the phase of the loop we are leaving
				startTime = trap_XAnimGetTime( tree, sa.index );
			} else {
				startTime = (float)( ( bgs_time / 100 + ci->clientNum * 360 ) % 1000 ) * 0.001f;
			}
		}
	}

	if ( oldAnim ) {
		trap_XAnimClearGoalWeight( tree, (unsigned short)( oldAnimation & ~ANIM_TOGGLEBIT ),
								   lf->animationTime * 0.001f );
	}

	if ( index ) {
		if ( !isLegs ) {
			// the weapon hangs off the torso, so its DObj has to be rebuilt
			ci->gunHandLeft = 0;
			ci->dobjNeedsUpdate = 1;
		}

		sa.index = (unsigned short)index;

		if ( anim->flags & ANIMFL_DEATH ) {
			if ( trap_XAnimIsLooped( sa ) ) {
				Com_Error( ERR_DROP, "death animation '%s' is looping", anim->name );
			}
			if ( corpse ) {
				trap_XAnimSetCompleteGoalWeight( tree, sa.index, 1.0f,
												 lf->animationTime * 0.001f, 1.0f, 0, 0 );
			} else {
				trap_XAnimSetCompleteGoalWeightKnobAll( tree, sa.index, bgs_rootAnim.index,
														1.0f, 0, 1.0f, 0, 0 );
				trap_XAnimSetTime( tree, sa.index, 1.0f );
			}
		} else {
			trap_XAnimSetCompleteGoalWeight( tree, sa.index, 1.0f, lf->animationTime * 0.001f,
											 1.0f, (unsigned short)anim->noteType, 1 );
			if ( anim->moveSpeed ) {
				trap_XAnimSetTime( tree, sa.index, startTime );
			}
		}

		if ( !isLegs ) {
			// hand the torso the whole body and leave the legs tree just alive
			trap_XAnimSetCompleteGoalWeight( tree, bgs_animScriptData.torsoAnim.index, 1.0f,
											 lf->animationTime * 0.001f, 1.0f,
											 (unsigned short)anim->noteType, 0 );
			trap_XAnimSetCompleteGoalWeight( tree, bgs_animScriptData.legsAnim.index, 0.01f,
											 lf->animationTime * 0.001f, 1.0f,
											 (unsigned short)anim->noteType, 0 );
		}
	} else if ( !isLegs ) {
		// no torso animation: give the legs tree the body back
		trap_XAnimSetCompleteGoalWeight( tree, bgs_animScriptData.torsoAnim.index, 0.0f,
										 lf->animationTime * 0.001f, 1.0f, 0, 0 );
		trap_XAnimSetCompleteGoalWeight( tree, bgs_animScriptData.legsAnim.index, 1.0f,
										 lf->animationTime * 0.001f, 1.0f, 0, 0 );
	}

	if ( g_debuganim.integer == 1 ) {
		Com_Printf( "Anim-%s: %i, %s\n", isLegs ? "legs " : "torso", index,
					bgs_animScriptData.animations[index].name );
	}
}

/*
==================
BG_RunLerpFrameRate

  Drives one animation slot's playback rate from how far the entity actually
  moved since the last frame, so a walk cycle does not slide.
==================
*/
void BG_RunLerpFrameRate( clientInfo_t *ci, lerpFrame_t *lf, entityState_t *es, int newAnimation ) {
	qboolean ladder;
	void *tree;
	int index;
	animation_t *animation;
	int moveSpeed;
	float dist;
	float speed;
	float maxScale;
	scr_anim_t sa;

	ladder = ( lf->animation && ( lf->animation->flags & ANIMFL_LADDER ) );

	tree = ci->animTree;
	sa.anims = (unsigned short)Scr_GetAnimsIndex( bgs_animScriptData.animTreeIndex );

	if ( newAnimation != lf->animationNumber
		 || ( !lf->animation && ( newAnimation & ~ANIM_TOGGLEBIT ) ) ) {
		BG_SetNewAnimation( ci, lf, newAnimation, ( es->eFlags >> 11 ) & 1 );
	}

	index = newAnimation & ~ANIM_TOGGLEBIT;
	if ( !index ) {
		return;
	}

	animation = lf->animation;
	moveSpeed = animation->moveSpeed;

	if ( !moveSpeed || !lf->oldFrameSnapshotTime ) {
		lf->animSpeedScale = 1.0f;
		lf->oldFrameSnapshotTime = bgs_animTime;
		VectorCopy( es->pos.trBase, lf->oldFramePos );
	} else if ( bgs_animTime != lf->oldFrameSnapshotTime ) {
		if ( ladder ) {
			dist = (float)fabs( lf->oldFramePos[2] - es->pos.trBase[2] );
		} else {
			dist = Distance( lf->oldFramePos, es->pos.trBase );
		}
		speed = dist / ( (float)( bgs_animTime - lf->oldFrameSnapshotTime ) * 0.001f );
		lf->animSpeedScale = speed / (float)moveSpeed;

		lf->oldFrameSnapshotTime = bgs_animTime;
		VectorCopy( es->pos.trBase, lf->oldFramePos );

		if ( lf->animSpeedScale < 0.1f ) {
			if ( lf->animSpeedScale < 0.01f && ladder ) {
				lf->animSpeedScale = 0.0f;
			} else {
				lf->animSpeedScale = 0.1f;
			}
		} else if ( lf->animSpeedScale > 2.0f ) {
			if ( animation->flags & ANIMFL_LADDER ) {
				if ( lf->animSpeedScale > 4.0f ) {
					lf->animSpeedScale = 4.0f;
				}
			} else if ( animation->moveSpeed > 150 ) {
				lf->animSpeedScale = 2.0f;
			} else if ( animation->moveSpeed < 20 ) {
				if ( lf->animSpeedScale > 3.0f ) {
					lf->animSpeedScale = 3.0f;
				}
			} else {
				// 3.0 at 20 units/sec falling to 2.0 at 150
				maxScale = 3.0f - (float)( animation->moveSpeed - 20 ) * 0.0076923077f;
				if ( maxScale < lf->animSpeedScale ) {
					lf->animSpeedScale = maxScale;
				}
			}
		}

		if ( g_debuganim.integer == 2 ) {
			Com_Printf( "MoveSpeed: %s, %i, %4.4f : %1.4f\n",
						bgs_animScriptData.animations[index].name,
						bgs_animScriptData.animations[index].moveSpeed,
						speed, lf->animSpeedScale );
		}
	}

	if ( lf->animationNumber ) {
		trap_XAnimSetAnimRate( tree, (unsigned short)( lf->animationNumber & ~ANIM_TOGGLEBIT ),
							   lf->animSpeedScale );
	}
}

/*
==================
BG_PlayerAnimation_VerifyAnim

  The anim tree drops an animation once its goal weight has run out; forget it
  here so the next BG_SetNewAnimation restarts it instead of blending from a
  frame that is no longer playing.
==================
*/
void BG_PlayerAnimation_VerifyAnim( lerpFrame_t *lf, void *tree ) {
	scr_anim_t sa;

	sa.anims = (unsigned short)Scr_GetAnimsIndex( bgs_animScriptData.animTreeIndex );

	if ( lf->animationNumber ) {
		sa.index = (unsigned short)( lf->animationNumber & ~ANIM_TOGGLEBIT );
		if ( trap_XAnimGetWeight( tree, sa.index ) == 0.0f ) {
			lf->animationNumber = 0;
			lf->animation = NULL;
			lf->animationTime = 150;
		}
	}
}

/*
==================
BG_SwingAngles
==================
*/
void BG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					 float speed, float *angle, qboolean *swinging ) {
	float swing;
	float move;
	float scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing <= swingTolerance && swing >= -swingTolerance ) {
			return;
		}
		*swinging = qtrue;
	}

	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	scale *= 0.05f;
	if ( scale < 0.5 ) {
		scale = 0.5;
	}

	// swing towards the destination angle
	if ( swing < 0 ) {
		move = -( bgs_frametime * scale * speed );
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
	} else {
		move = bgs_frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
	}
	*angle = AngleMod( *angle + move );

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - ( clampTolerance - 1 ) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + ( clampTolerance - 1 ) );
	}
}

/*
==================
BG_PlayerAngles

  Drifts the legs and torso towards the view.  CoD1 kept RTCW's structure but
  drives the legs from the entity's lean offset instead of its movementDir, and
  the swing speed comes from a cvar rather than a literal.
==================
*/
void BG_PlayerAngles( clientInfo_t *ci, entityState_t *es ) {
	float yaw;
	float legsYaw;
	float dest;
	float clampTolerance;
	animation_t *animation;

	yaw = SHORT2ANGLE( ANGLE2SHORT( ci->viewYaw ) );

	if ( ( es->eFlags & EF_TURRET_ACTIVE_MASK )
		 || ( ci->conditions[ANIM_COND_MOVETYPE][0] & ANIM_MT_CLIMBING_MASK )
		 || !( es->eFlags & 0x6 ) ) {
		// always centred
		ci->legs.yawing = qtrue;
		ci->torso.yawing = qtrue;
		ci->torso.pitching = qtrue;
	} else if ( BG_GetConditionValue( ci, ANIM_COND_FIRING, qtrue ) ) {
		ci->torso.yawing = qtrue;
		ci->torso.pitching = qtrue;
	}

	// --------- yaw -------------
	legsYaw = yaw + ci->leanAmount;

	if ( es->eFlags & EF_DEAD ) {
		legsYaw = yaw;
		dest = yaw;
		clampTolerance = 90;
	} else if ( ci->conditions[ANIM_COND_MOVETYPE][0] & ANIM_MT_CLIMBING_MASK ) {
		dest = legsYaw;
		clampTolerance = 0;
	} else if ( es->eFlags & EF_PRONE ) {
		dest = yaw;
		clampTolerance = 90;
	} else if ( es->eFlags & EF_FIRING ) {
		dest = yaw;
		clampTolerance = 45;
	} else if ( es->eFlags & EF_AIMING_DOWN_SIGHT ) {
		dest = yaw;
		clampTolerance = 90;
	} else {
		dest = ci->leanAmount * 0.3f + yaw;
		clampTolerance = 90;
	}
	BG_SwingAngles( dest, 0, clampTolerance, bg_swingSpeed.value,
					&ci->torso.yawAngle, &ci->torso.yawing );

	if ( es->eFlags & EF_DEAD ) {
		BG_SwingAngles( legsYaw, 0, 150, bg_swingSpeed.value,
						&ci->legs.yawAngle, &ci->legs.yawing );
	} else if ( es->eFlags & EF_PRONE ) {
		// prone legs are pinned; the body already points where it lies
		ci->legs.yawing = qfalse;
		ci->legs.yawAngle = legsYaw;
	} else {
		animation = &bgs_animScriptData.animations[es->legsAnim & ~ANIM_TOGGLEBIT];
		if ( animation->flags & ( ANIMFL_STRAFELEFT | ANIMFL_STRAFERIGHT ) ) {
			// a strafe animation already carries the offset in the model
			ci->legs.yawing = qfalse;
			BG_SwingAngles( yaw, 0, 150, bg_swingSpeed.value,
							&ci->legs.yawAngle, &ci->legs.yawing );
		} else if ( ci->legs.yawing ) {
			BG_SwingAngles( legsYaw, 0, 150, bg_swingSpeed.value,
							&ci->legs.yawAngle, &ci->legs.yawing );
		} else {
			BG_SwingAngles( legsYaw, 40, 150, bg_swingSpeed.value,
							&ci->legs.yawAngle, &ci->legs.yawing );
		}
	}

	if ( es->eFlags & EF_TURRET_ACTIVE_MASK ) {
		ci->torso.yawAngle = yaw;
		ci->legs.yawAngle = yaw;
	} else if ( ci->conditions[ANIM_COND_MOVETYPE][0] & ANIM_MT_CLIMBING_MASK ) {
		ci->torso.yawAngle = legsYaw;
		ci->legs.yawAngle = legsYaw;
	}

	// --------- pitch -------------
	if ( ( es->eFlags & EF_DEAD )
		 || ( es->eFlags & EF_TURRET_ACTIVE_MASK )
		 || ( ci->conditions[ANIM_COND_MOVETYPE][0] & ANIM_MT_CLIMBING_MASK ) ) {
		dest = 0;
	} else if ( ci->viewPitch > 180 ) {
		// only show a fraction of the pitch angle in the torso
		dest = ( ci->viewPitch - 360 ) * 0.6f;
	} else {
		dest = ci->viewPitch * 0.6f;
	}
	BG_SwingAngles( dest, 0, 45, 0.15f, &ci->torso.pitchAngle, &ci->torso.pitching );
}

/*
=================
BG_AnimPlayerConditions

  The server-side half of BG_AnimUpdatePlayerStateConditions: the same nine
  conditions rebuilt from the entity state once the player state has been
  snapshotted, plus the two the animation itself carries.
=================
*/
void BG_AnimPlayerConditions( entityState_t *es, clientInfo_t *ci ) {
	animation_t *animation;

	BG_UpdateConditionValue( es->clientNum, ANIM_COND_WEAPONS, es->weapon, qtrue );

	BG_UpdateConditionValue( es->clientNum, ANIM_COND_WEAPONCLASS,
							 bg_weaponInfo[es->weapon]->weaponClass, qtrue );

	if ( es->eFlags & EF_AIMING_DOWN_SIGHT ) {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_WEAPON_POSITION, WEAPON_POSITION_ADS, qtrue );
	} else {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_WEAPON_POSITION, WEAPON_POSITION_HIP, qtrue );
	}

	if ( es->eFlags & EF_TURRET_ACTIVE_MASK ) {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_MOUNTED, MOUNTED_MG42, qtrue );
	} else {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_MOUNTED, MOUNTED_UNUSED, qtrue );
	}

	BG_UpdateConditionValue( es->clientNum, ANIM_COND_UNDERHAND, ci->viewPitch > 0, qtrue );

	if ( es->eFlags & EF_CROUCHING ) {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_CROUCHING, qtrue, qtrue );
	} else {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_CROUCHING, qfalse, qtrue );
	}

	if ( es->eFlags & EF_FIRING ) {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_FIRING, qtrue, qtrue );
	} else {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_FIRING, qfalse, qtrue );
	}

	// reverse engineer the movetype and the strafe direction out of the
	// animation BG_ParseCommands tagged them onto
	animation = &bgs_animScriptData.animations[es->legsAnim & ~ANIM_TOGGLEBIT];

	if ( animation->movetype ) {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_MOVETYPE, animation->movetype, qfalse );
	}

	if ( animation->flags & ANIMFL_STRAFELEFT ) {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_STRAFING, STRAFING_LEFT, qtrue );
	} else if ( animation->flags & ANIMFL_STRAFERIGHT ) {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_STRAFING, STRAFING_RIGHT, qtrue );
	} else {
		BG_UpdateConditionValue( es->clientNum, ANIM_COND_STRAFING, STRAFING_NOT, qtrue );
	}
}

/*
==================
BG_GetSurfIndex

  Surfaces are a client-side concept; the game module has none.
==================
*/
short BG_GetSurfIndex( void ) {
	return 0;
}

/*
==================
BG_GetXModel
==================
*/
void *BG_GetXModel( const char *name ) {
#ifdef CGAMEDLL
	/* 0x300046D0: trap_R_RegisterModel then trap 50 on the handle.  LTCG
	   inlined the trap 50 wrapper, which is why the retail body issues the
	   syscall itself. */
	return (void *)trap_R_GetXModelByHandle( trap_R_RegisterModel( name, 7 ) );
#else
	return trap_XModelGet( name );
#endif
}

/*
==================
BG_DObjCreate
==================
*/
void BG_DObjCreate( DObjModel *models, unsigned short numModels, void *tree, int handle ) {
#ifdef CGAMEDLL
	trap_DObjCreate( models, numModels, tree, handle );      /* 0x300046F0 */
#else
	trap_DObjCreate( models, numModels, tree, handle, 0 );
#endif
}

/*
==================
BG_Player_DoControllers

  Bends the spine, neck, head and pelvis so the model looks where the player
  looks and leans where the player leans.  Everything is written straight into
  the DObj pose -- there is no per-client controller array.

  Second argument: gentity_t * in the game build, the resolved DObj in cgame.
  See the CGAMEDLL block at the top of the file.

  Game 0x200046B0.  TODO: the cgame build (0x30004710) carries an extra
  client-only block, gated on dword_301E0D0C == 4/5/6, that folds
  sin( dword_300F0310 * K + es-><+0x90> ) into the back_low/back_mid/back_up,
  neck, head and pelvis angles (0x30004985, 0x30004AE7, 0x30004D27 and the
  __CIsin calls around them).  That block is not present here.
==================
*/
void BG_Player_DoControllers( clientInfo_t *ci, BG_DOBJ_PARM, entityState_t *es, int *partBits ) {
	vec3_t angles;
	vec3_t origin;
	vec3_t waistAngles;
	float torsoYawAngle;
	float pitch;
	float torsoPitch;
	float torsoYaw;
	float pitchDelta;
	float yawDelta;
	float lean;
	float leanBase;
	float leanSide;
	float leanRoll;

	if ( es->eFlags & EF_TURRET_ACTIVE_MASK ) {
		return;
	}

	angles[0] = 0;
	angles[1] = ci->legs.yawAngle;
	angles[2] = 0;
	torsoYawAngle = ci->torso.yawAngle;

	if ( ci->conditions[ANIM_COND_MOVETYPE][0] & ANIM_MT_CLIMBING_MASK ) {
		pitch = 0;
	} else if ( !( es->eFlags & EF_PRONE ) ) {
		pitch = ci->torso.pitchAngle;
	} else {
		pitch = AngleNormalize180( ci->torso.pitchAngle );
		if ( pitch > 0 ) {
			pitch = pitch * 0.5f;
		} else {
			pitch = pitch * 0.25f;
		}
	}

	torsoPitch = AngleSubtract( ci->viewPitch, pitch );
	torsoYaw = AngleSubtract( ci->viewYaw, torsoYawAngle );
	pitchDelta = AngleSubtract( pitch, 0 );
	yawDelta = AngleSubtract( torsoYawAngle, angles[1] );

	origin[0] = 0;
	origin[1] = 0;
	origin[2] = es->fTorsoHeight;

	lean = ( 2.0f - (float)fabs( ci->leanFraction ) ) * ci->leanFraction;

	if ( lean == 0.0f ) {
		leanSide = 0;
		leanRoll = 0;
	} else {
		leanBase = 50.0f * lean * 0.925f;

		leanSide = leanBase;
		if ( es->eFlags & EF_CROUCHING ) {
			leanSide = leanSide * ( lean > 0 ? 1.5f : 1.8f );
		}

		if ( es->eFlags & EF_PRONE ) {
			origin[1] = lean * -2.5f;
		} else if ( es->eFlags & EF_CROUCHING ) {
			origin[1] = lean * -1.5f;
		} else {
			origin[1] = -lean;
		}

		leanRoll = leanBase;
		if ( es->eFlags & EF_CROUCHING ) {
			leanRoll = leanRoll * ( lean > 0 ? 1.5f : 1.8f );
		} else if ( es->eFlags & EF_PRONE ) {
			leanRoll = leanRoll * 0.5f;
		}
	}

	if ( !( es->eFlags & EF_DEAD ) ) {
		angles[1] = AngleSubtract( angles[1], ci->viewYaw );
	}
	if ( !( es->eFlags & EF_PRONE ) ) {
		angles[2] = 50.0f * lean * 0.075f;
	} else {
		angles[0] = es->fTorsoPitch;
	}
	BG_DObjSetLocalTag( BG_DOBJ, partBits, "tag_origin", origin, angles );

	if ( es->eFlags & EF_PRONE ) {

		angles[0] = 0;
		angles[1] = leanSide * -1.2f;
		angles[2] = leanSide * 0.3f;
		if ( es->fTorsoPitch != 0 || es->fWaistPitch != 0 ) {
			angles[0] = AngleSubtract( es->fTorsoPitch, es->fWaistPitch );
		}
		BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "back_low", angles );

		angles[0] = 0;
		angles[1] = yawDelta * 0.1f - leanSide * 0.2f;
		angles[2] = leanSide * 0.2f;
		BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "back_mid", angles );

		angles[0] = pitchDelta;
		angles[1] = yawDelta * 0.8f + leanSide;
		angles[2] = leanSide * -0.2f;
		BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "back_up", angles );

	} else {

		angles[0] = pitchDelta * 0.2f;
		angles[1] = yawDelta * 0.4f;
		angles[2] = leanSide * 0.5f;
		if ( es->fTorsoPitch != 0 || es->fWaistPitch != 0 ) {
			angles[0] += AngleSubtract( es->fTorsoPitch, es->fWaistPitch );
		}
		BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "back_low", angles );

		angles[0] = pitchDelta * 0.3f;
		angles[1] = yawDelta * 0.4f;
		angles[2] = leanSide * 0.5f;
		BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "back_mid", angles );

		angles[0] = pitchDelta * 0.5f;
		angles[1] = yawDelta * 0.2f;
		angles[2] = leanSide * -0.6f;
		BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "back_up", angles );
	}

	angles[0] = torsoPitch * 0.3f;
	angles[1] = torsoYaw * 0.3f;
	angles[2] = 0;
	BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "neck", angles );

	angles[0] = torsoPitch * 0.7f;
	angles[1] = torsoYaw * 0.7f;
	angles[2] = leanRoll * -0.3f;
	BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "head", angles );

	/* the retail frame folds these three onto the torsoPitch/torsoYaw/leanRoll
	 * slots, which are dead by here */
	waistAngles[0] = 0;
	waistAngles[1] = 0;
	waistAngles[2] = 0;
	if ( es->fWaistPitch != 0 || es->fTorsoPitch != 0 ) {
		waistAngles[0] = AngleSubtract( es->fWaistPitch, es->fTorsoPitch );
	}
	BG_DObjSetControlTagAngles( BG_DOBJ, partBits, "pelvis", waistAngles );
}

/*
==================
BG_PlayerAnimation

  The middle parameter is passed but never read: the entity state carries
  everything this needs.  It goes through BG_DOBJ_PARM like its two siblings
  so the cgame build spells it `int obj` and the game build `gentity_t *ent`.

  ORDER IS ( ci, obj/ent, es ).  Retail 0x30004E40 is
  __usercall( ci@<ebx>, obj, es ): ci+0x37C and ci+0x3AC (clientInfo_t.legs
  and .torso) go to BG_PlayerAnimation_VerifyAnim.
==================
*/
void BG_PlayerAnimation( clientInfo_t *ci, BG_DOBJ_PARM, entityState_t *es ) {
	BG_PlayerAngles( ci, es );
	BG_AnimPlayerConditions( es, ci );

	BG_PlayerAnimation_VerifyAnim( &ci->legs, ci->animTree );
	BG_PlayerAnimation_VerifyAnim( &ci->torso, ci->animTree );

	BG_RunLerpFrameRate( ci, &ci->legs, es, es->legsAnim );
	BG_RunLerpFrameRate( ci, &ci->torso, es, es->torsoAnim );
}

/*
==================
BG_UpdatePlayerDObj

  Rebuilds the player's DObj out of the model and attach names ClientEndFrame
  copied into the clientInfo_t, but only when the weapon changed or the torso
  animation asked for it.

  First argument: gentity_t * in the game build, which resolves the DObj here.
  The cgame build is handed the DObj already resolved -- CG_TransitionSnapshot
  calls Com_GetClientDObj( es->clientNum ) (trap 162) and this only null-tests
  the result.  See the CGAMEDLL block at the top of the file.

  Game 0x20004CF0, cgame 0x30004EB0.

  The client registers every model name through trap_R_RegisterModel before
  asking for the XModel, Com_Errors when that fails, and keeps the shader
  handle in DObjModel.boneName rather than 0 (0x30004F24..0x30004F66 for the
  base model, 0x30004F85..0x30004FC5 for each attachment).  BG_GetXModel then
  registers the name a second time (LTCG inlined that one-line wrapper).

  0x30004FDA..0x30005039: a non-zero weapon appends cg_weapons[weapon].worldModel
  at "tag_weapon_right" / "tag_weapon_left" (chosen on ci->gunHandLeft).  A bg
  unit cannot include cg_local.h (g_public.h and cg_public.h each define
  entityState_t), so the record is reached by its retail stride and offset
  through CG_WEAPON_WORLDMODEL below.
==================
*/
#ifdef CGAMEDLL
extern char cg_weapons[];       /* cgWeaponInfo_t[64] at 0x301A6940, 0x198 bytes each */
#define CG_WEAPON_WORLDMODEL( w )   ( *(int *)( cg_weapons + ( w ) * 0x198 + 0xB0 ) )   /* +0xB0 worldModel */
#endif
void BG_UpdatePlayerDObj( BG_DOBJ_PARM, entityState_t *es, clientInfo_t *ci ) {
	DObjModel models[MAX_ATTACHED_MODELS + 1];
	qboolean dobjExists;
	int weapon;
	int modelCount;
	int i;

#ifdef CGAMEDLL
	dobjExists = ( obj != 0 );
#else
	dobjExists = trap_DObjExists( ent );
#endif

	weapon = es->weapon;
	if ( es->eFlags & EF_TURRET_ACTIVE_MASK ) {
		// on a turret the player holds nothing
		weapon = 0;
	}

	if ( !ci->infoValid || !ci->modelName[0] ) {
		trap_SafeDObjFree( es->number, qtrue );
		return;
	}

	if ( dobjExists ) {
		if ( ci->dobjWeapon == weapon && !ci->dobjNeedsUpdate ) {
			return;
		}
		trap_SafeDObjFree( es->number, qfalse );
	}

#ifdef CGAMEDLL
	models[0].boneName = (short)trap_R_RegisterModel( ci->modelName, 7 );
	if ( !models[0].boneName ) {
		Com_Error( ERR_DROP, "\x15" "Could not load model '%s'", ci->modelName );
	}
#else
	models[0].boneName = 0;
#endif
	models[0].model = BG_GetXModel( ci->modelName );
	models[0].tagName = NULL;
	modelCount = 1;

	for ( i = 0 ; i < MAX_ATTACHED_MODELS ; i++ ) {
		if ( !ci->attachModelNames[i][0] ) {
			continue;
		}
#ifdef CGAMEDLL
		models[modelCount].boneName = (short)trap_R_RegisterModel( ci->attachModelNames[i], 7 );
		if ( !models[modelCount].boneName ) {
			Com_Error( ERR_DROP, "\x15" "Could not load model '%s'", ci->attachModelNames[i] );
		}
#else
		models[modelCount].boneName = 0;
#endif
		models[modelCount].model = BG_GetXModel( ci->attachModelNames[i] );
		models[modelCount].tagName = ci->attachTagNames[i];
		modelCount++;
	}

#ifdef CGAMEDLL
	/* 0x30004FDA..0x30005039: the held weapon's world model rides on the hand
	   tag.  cg_weapons is reached by its retail shape (imul 0x198, +0xB0)
	   because cg_local.h cannot be included here (see the header comment). */
	if ( weapon && CG_WEAPON_WORLDMODEL( weapon ) ) {
		models[modelCount].model = (void *)trap_R_GetXModelByHandle( CG_WEAPON_WORLDMODEL( weapon ) );
		models[modelCount].boneName = (short)CG_WEAPON_WORLDMODEL( weapon );
		models[modelCount].tagName = ci->gunHandLeft ? "tag_weapon_left" : "tag_weapon_right";
		modelCount++;
	}
#endif

	BG_DObjCreate( models, (unsigned short)modelCount, ci->animTree, es->number );

	ci->dobjWeapon = weapon;
	ci->dobjNeedsUpdate = 0;
}

/*
==================
BG_FindAnims
==================
*/
void BG_FindAnims( void ) {
	Scr_FindAnim( "multiplayer", "root", &bgs_rootAnim );
	Scr_FindAnim( "multiplayer", "torso", &bgs_torsoAnim );
	Scr_FindAnim( "multiplayer", "legs", &bgs_legsAnim );
	Scr_FindAnim( "multiplayer", "turning", &bgs_turningAnim );
}

/*
==================
BG_FindAnimTree
==================
*/
void *BG_FindAnimTree( const char *treeName, qboolean errorIfMissing ) {
	void *tree;

	tree = Scr_FindAnimTree( treeName );
	if ( !tree && errorIfMissing ) {
		Com_Error( ERR_DROP, "\x15" "Could not find animation tree '%s'", treeName );
	}

	return tree;
}

/*
==================
BG_FindAnimTrees

  Publishes the player anim tree and the three script animations the run-time
  code blends against.  The retail build inlines BG_FindAnimTree here.
==================
*/
void BG_FindAnimTrees( void ) {
	void *tree;

	tree = BG_FindAnimTree( "multiplayer", qtrue );

	bgs_animTree = tree;
	bgs_animScriptData.animTreeIndex = (int)tree;

	bgs_animScriptData.torsoAnim = bgs_torsoAnim;
	bgs_animScriptData.legsAnim = bgs_legsAnim;
	bgs_animScriptData.turningAnim = bgs_turningAnim;
}

#ifndef CGAMEDLL
/*
==================
BG_CreateClientAnimTrees

  0x20004F20 -- GScr_LoadScripts ends in a jmp to it (0x2002F16F).  The name
  is inferred.  cgame's twin at 0x30005180 (the tail of
  CGScr_LoadAnimTrees) goes on to seed cgs.corpseinfo[MAX_CORPSES] the same
  way; that half is cg_animtree_mp.c's BG_CreateClientAnimTrees, because a bg
  unit cannot reach cgs, so this build is game-only.
  bgs_animTree is read once, before the loop (0x20004F22 mov edi), and the
  same edi is pushed for all 64 calls.
==================
*/
void BG_CreateClientAnimTrees( void ) {
	void *tree;
	int i;

	tree = bgs_animTree;

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		bg_clientinfo[i].animTree = trap_XAnimCreateTree( tree );
	}
}
#endif
