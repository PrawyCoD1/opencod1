/*
 * g_scr_main_mp.c -- the multiplayer game module's half of the script system.
 *
 * Three things live here.  The script VM's exports arrive through Scr_FarHook
 * as a block of 102 function pointers (0x200A04F8) and the game's five
 * callbacks go back the other way (0x200A04E0); every Scr_* call anywhere in
 * this module is an indirect call through that block.  scr_const is the
 * module's table of interned script strings, filled once by GScr_LoadConsts.
 * And scr_functions[] / scr_methods[] are the builtin dispatch tables --
 * Scr_GetFunction and Scr_GetMethod are what the VM asks when a script names
 * something it does not know; almost every other function in the file is one
 * entry of those two tables.
 *
 * CoD's script VM has no RTCW ancestor, so none of this is a port.
 *
 * Function order is binary order (0x2002E330 .. 0x20035490), with the
 * far-hook block and Scr_FarHook (0x2003FB40, outside the unit's own range --
 * it belongs to the DLL side of script/scr_import.c) at the two ends.
 *
 * @fidelity: likely
 */

/* The unit calls strcpy / strcat / strncpy / strlwr the way the retail source
   did; the modern CRT only deprecates the names. */
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "g_local.h"

/* Declarations that belong in a shared header. */

#define ENTITYNUM_WORLD             ( MAX_GENTITIES - 2 )

/* Configstring layout this unit reaches. */
#define CS_MUSIC                    3       /* Scr_AmbientPlay also drives it */
#define CS_TEAMSCORE_AXIS           5
#define CS_TEAMSCORE_ALLIES         6
#define CS_SCORES                   20      /* the "winner" info string */
#define CS_STATUSICONS              21
#define CS_STATUSICONS_COUNT        8
#define CS_HEADICONS                29
#define CS_HEADICONS_COUNT          15
#define CS_MODELS                   268
#define CS_EFFECTS                  780
#define CS_FX                       844
#define CS_FX_COUNT                 256
#define CS_SHELLSHOCKS              1100
#define CS_SHELLSHOCKS_COUNT        16
#define CS_MENUS                    1180
#define CS_MENUS_COUNT              32
#define CS_HINTSTRINGS              1212
#define CS_HINTSTRINGS_COUNT        32

/* entityState_t.eType. */
#define ET_LOOP_FX                  10

/* entity_event_t; bg_misc.c's eventnames[] is the authority. */
#define EV_SOUND_ALIAS              172
#define EV_GRENADE_EXPLODE          178
#define EV_PLAY_FX                  191
#define EV_PLAY_FX_DIR              192
#define EV_PLAY_FX_ON_TAG           193
#define EV_EARTHQUAKE               195
#define EV_OBITUARY                 201

/* entityShared_t.svFlags. */
#define SVF_NOCLIENT                0x00000001
#define SVF_BROADCAST               0x00000008
#define SVF_OBJECTIVE               0x00000010
#define SVF_CAPSULE                 0x00000200

/* gentity_t.flags. */
#define FL_NO_GRENADE_TOUCH_DAMAGE  0x00008000      /* GScr_EnableGrenadeTouchDamage */
#define FL_NO_GRENADE_BOUNCE        0x00010000      /* GScr_DisableGrenadeBounce */

/* Content masks.  0x2802031 is bulletTrace's with characters, 0x802031
   without; 0x2810011 is the spawn-point capsule test's. */
#define MASK_BULLET_CHARACTERS      0x02802031
#define MASK_BULLET                 0x00802031
#define MASK_SPAWNPOINT             0x02810011
#define CONTENTS_BODY               0x02000000
/* the downward probe grenadeExplosionEffect uses to pick a surface type */
#define MASK_GRENADE_SURFACE        0x00000011

/* Com_Error levels: ERR_DROP is 1, the localization level is 7. */
#define ERR_LOCALIZATION            7

/*
 * The script VM's value discriminator.  Scr_GetType returns it for a value and
 * Scr_GetPointerType for the object a VAR_OBJECT refers to.  The engine's own
 * 19-entry type-name table fixes the domain.
 */
typedef enum scriptVarType_e {
	VAR_UNDEFINED           = 0,
	VAR_STRING              = 1,
	VAR_LOCALIZED_STRING    = 2,
	VAR_VECTOR              = 3,
	VAR_FLOAT               = 4,
	VAR_INTEGER             = 5,
	VAR_CODEPOS             = 6,
	VAR_OBJECT              = 7,
	VAR_KEY_VALUE           = 8,
	VAR_FUNCTION            = 9,
	VAR_STACK               = 10,
	VAR_ANIMATION           = 11,
	VAR_THREAD              = 12,
	VAR_ENTITY              = 13,
	VAR_STRUCT              = 14,
	VAR_ARRAY               = 15,
	VAR_DEAD_THREAD         = 16,
	VAR_DEAD_ENTITY         = 17,
	VAR_DEAD_OBJECT         = 18,

	VAR_COUNT               = 19
} scriptVarType_t;

/* level.exitRequest values. */
#define EXITREQUEST_NONE            0
#define EXITREQUEST_MAP_RESTART     1
#define EXITREQUEST_EXITLEVEL       2

/* level.objectives[].state. */
#define OBJST_EMPTY                 0
#define OBJST_ACTIVE                1
#define OBJST_INVISIBLE             2
#define OBJST_CURRENT               4

/* GScr_SetClientNameMode. */
#define CLIENTNAMEMODE_AUTO         0
#define CLIENTNAMEMODE_MANUAL       1

/* trap_SendConsoleCommand's exec_when. */
#define EXEC_APPEND                 2

/* forward: q_shared.c */
int         Q_stricmpn( const char *s1, const char *s2, int n );
int         Q_strncasecmp( const char *s1, const char *s2, int n );
int         Q_strcasecmp( const char *s1, const char *s2 );
char        *Info_ValueForKey( const char *s, const char *key );

/* q_shared.h carries these in the original tree. */
#ifndef M_PI
#define M_PI                        3.14159265358979323846f
#endif
#define PITCH                       0
#define YAW                         1
#define ROLL                        2

/* forward: q_math.c */
extern vec3_t vec3_origin;                      /* 0x20055D18 */
vec_t       VectorNormalize( vec3_t v );
void        vectoangles( const vec3_t value1, vec3_t angles );
void        AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );
byte        DirToByte( const vec3_t dir );
float       RotationToYaw( const float *quat );

/* forward: q_parse.c */
char        *Com_Parse( char **data_p );

/* forward: the CRT.  Scr_ParseGameTypeList lowercases the script name with
   the CRT's strlwr, not q_shared's Q_strlwr. */
char        *strlwr( char *string );

/* forward: bg_slidemove.c */
int         Q_ftol( float f );

/* forward: bg_animation.c / bg_misc.c */
void        BG_FindAnims( void );
void        BG_FindAnimTrees( void );
void        BG_FinalizePlayerAnims( void );
void        BG_CreateClientAnimTrees( void );
byte        BG_GetWeaponIndexForName( const char *name );
/* bg_animation.c defines the 0x200A1C80 object as bgs_animScriptData. */
extern animScriptData_t bgs_animScriptData;             /* 0x200A1C80 */

/* forward: g_hud_mp.c */
void        GScr_AddFieldsForHudElems( void );
void        GScr_NewHudElem( void );
void        GScr_NewClientHudElem( void );
void        GScr_NewTeamHudElem( void );

/*
 * A script builtin takes no arguments and reads its parameters off the VM
 * stack; a method is handed the object it was called on.  g_local.h's other
 * method tables spell that object `int entnum` rather than `unsigned int`,
 * hence the casts in Scr_GetMethod.
 */
typedef void ( *xfunction_t )( void );
typedef void ( *xmethod_t )( unsigned int entnum );

/* ScriptEnt_GetMethod (g_scr_ent_mp.c), HudElem_GetMethod (g_hud_mp.c) and
   Player_GetMethod (g_client_script_cmd_mp.c) are declared in g_local.h. */

/* forward: g_spawn_mp.c -- Scr_SetObjectField is the third of the five game
   callbacks Scr_FarHook publishes. */
void        Scr_SetObjectField( int classnum, int objectNum, int fieldnum );

/* forward: g_mover_mp.c.  GScr_SetCursorHint validates against the same table
   the "cursorhint" spawn key parses. */
extern char *predef_hintStrings[];                      /* 0x20069D98 */

/* forward: g_main_mp.c */
extern vmCvar_t g_gametype;                             /* 0x20236CE0 */
extern vmCvar_t g_no_script_spam;                       /* 0x20236AA0 */
extern vmCvar_t cl_languagewarnings;                    /* 0x20236740 */
extern vmCvar_t cl_languagewarningsaserrors;            /* 0x20235AE0 */

/*
 * ---------------------------------------------------------------------------
 * The script VM import block.
 *
 * Scr_FarHook memcpy's 0x198 bytes -- 102 pointers -- over the run of globals
 * starting at Scr_GetBool (0x200A04F8), in the order scr_import.c's
 * Scr_NearHook fills them.  Slots 54, 55, 75 and 78 are holes neither side
 * ever writes; the four names below keep the block's shape.
 *
 * Slots this module never calls are declared without a prototype: their
 * argument lists are not recovered and inventing one would be a guess.
 * ---------------------------------------------------------------------------
 */

/* The five game entry points Scr_FarHook hands back, immediately below the
   import block. */
static void ( *scr_gameExports[5] )( void );                                            /* 0x200A04E0 */

/*
 * THE 102 SLOTS BELOW MUST BE ONE CONTIGUOUS RUN, IN THIS ORDER.  Scr_FarHook
 * memcpy's 0x198 bytes over them starting at &Scr_GetBool, which is what the
 * engine's Scr_NearHook block expects and what retail has at 0x200A04F8.  As 102
 * separate objects that is undefined -- C promises nothing about where distinct
 * globals land relative to each other -- so a named bss segment pins it: MSVC
 * emits a segment's variables in declaration order and the linker keeps one
 * object file's contribution to a section contiguous.  The block is one object
 * in the retail image, so retail almost certainly had a struct here.
 */
#pragma bss_seg( ".scrimp" )
int         ( *Scr_GetBool )( unsigned int num );                                       /* 0x200A04F8 */
int         ( *Scr_GetInt )( unsigned int num );                                        /* 0x200A04FC */
scr_anim_t  ( *Scr_GetAnim )( unsigned int num, void *anims );                          /* 0x200A0500 */
void        ( *Scr_GetAnimTree )();                                                     /* 0x200A0504 */
float       ( *Scr_GetFloat )( unsigned int num );                                      /* 0x200A0508 */
const char  *( *Scr_GetString )( unsigned int num );                                    /* 0x200A050C */
unsigned short ( *Scr_GetConstString )( unsigned int num );                             /* 0x200A0510 */
const char  *( *Scr_GetDebugString )( unsigned int num );                               /* 0x200A0514 */
const char  *( *Scr_GetIString )( unsigned int num );                                   /* 0x200A0518 */
void        ( *Scr_GetConstIString )();                                                 /* 0x200A051C */
void        ( *Scr_GetVector )( unsigned int num, vec3_t out );                         /* 0x200A0520 */
void        ( *Scr_GetFunc )();                                                         /* 0x200A0524 */
int         ( *Scr_GetType )( unsigned int num );                                       /* 0x200A0528 */
int         ( *Scr_GetPointerType )( unsigned int num );                                /* 0x200A052C */
unsigned int ( *Scr_GetEntityNum )( unsigned int num, int *classnum );                  /* 0x200A0530 */
unsigned int ( *Scr_GetNumParam )( void );                                              /* 0x200A0534 */
void        ( *Scr_AddBool )( int value );                                              /* 0x200A0538 */
void        ( *Scr_AddInt )( int value );                                               /* 0x200A053C */
void        ( *Scr_AddFloat )( float value );                                           /* 0x200A0540 */
void        ( *Scr_AddAnim )();                                                         /* 0x200A0544 */
void        ( *Scr_AddUndefined )( void );                                              /* 0x200A0548 */
void        ( *Scr_AddEntityNum )( int entnum, int classnum );                          /* 0x200A054C */
void        ( *Scr_AddStruct )( void );                                                 /* 0x200A0550 */
void        ( *Scr_AddString )( const char *string );                                   /* 0x200A0554 */
void        ( *Scr_AddIString )();                                                      /* 0x200A0558 */
void        ( *Scr_AddConstString )( unsigned short id );                               /* 0x200A055C */
void        ( *Scr_AddVector )( const float *value );                                   /* 0x200A0560 */
void        ( *Scr_AddObject )( unsigned short id );                                    /* 0x200A0564 */
void        ( *Scr_AddArray )( void );                                                  /* 0x200A0568 */
void        ( *Scr_AddArrayStringIndexed )( unsigned short id );                        /* 0x200A056C */
void        ( *Scr_MakeArray )( void );                                                 /* 0x200A0570 */
void        ( *Scr_BeginLoadScripts )( void );                                          /* 0x200A0574 */
void        ( *Scr_BeginLoadAnimTrees )();                                              /* 0x200A0578 */
void        ( *Scr_EndLoadScripts )( void );                                            /* 0x200A057C */
void        ( *Scr_EndLoadAnimTrees )( void );                                          /* 0x200A0580 */
void        ( *Scr_PrecacheAnimTrees )( void *( *allocFn )( int size ) );               /* 0x200A0584 */
void        ( *Scr_FreeScripts )( int bComplete );                                      /* 0x200A0588 */
void        ( *Scr_FreeGameVariable )( int bComplete );                                 /* 0x200A058C */
void        ( *Scr_ShutdownSystem )( int bComplete );                                   /* 0x200A0590 */
int         ( *Scr_IsSystemActive )( int bComplete );                                   /* 0x200A0594 */
void        ( *Scr_AddExecThread )();                                                   /* 0x200A0598 */
void        ( *Scr_AddExecEntThreadNum )( int entnum, int classnum, int handle, unsigned int paramcount );      /* 0x200A059C */
unsigned short ( *Scr_ExecThread )( unsigned int handle, unsigned int paramcount );     /* 0x200A05A0 */
unsigned short ( *Scr_ExecEntThreadNum )( int entnum, int classnum, int handle, unsigned int paramcount );      /* 0x200A05A4 */
void        ( *Scr_IsThreadAlive )();                                                   /* 0x200A05A8 */
void        ( *Scr_Error )( const char *error );                                        /* 0x200A05AC */
void        ( *Scr_ErrorWithDialogMessage )();                                          /* 0x200A05B0 */
void        ( *Scr_ParamError )( unsigned int num, const char *error );                 /* 0x200A05B4 */
void        ( *Scr_ObjectError )( const char *error );                                  /* 0x200A05B8 */
void        ( *Scr_SetDynamicEntityField )( int entnum, int classnum, int fieldnum );   /* 0x200A05BC */
void        ( *Scr_FreeEntityNum )( int entnum, int classnum );                         /* 0x200A05C0 */
unsigned short ( *Scr_GetEntityId )( int entnum, int classnum );                        /* 0x200A05C4 */
void        ( *Scr_SetClassMap )( scr_classStruct_t *classMap, unsigned int count );    /* 0x200A05C8 */
void        ( *Scr_RemoveClassMap )( void );                                            /* 0x200A05CC */
void        ( *Scr_Unused36 )();                                                        /* 0x200A05D0 -- never written */
void        ( *Scr_Unused37 )();                                                        /* 0x200A05D4 -- never written */
void        ( *Scr_AddClassField )( unsigned short classnum, const char *name, unsigned short fieldnum );       /* 0x200A05D8 */
void        ( *Scr_AddFields )( const char *path, const char *extension );              /* 0x200A05DC */
unsigned short ( *Scr_FindField )( const char *name, int *type );                       /* 0x200A05E0 */
int         ( *Scr_GetOffset )( unsigned short classnum, const char *name );            /* 0x200A05E4 */
void        ( *Scr_CopyEntityNum )( int sourceEntnum, int destEntnum, int classnum );   /* 0x200A05E8 */
void        ( *Scr_Init )();                                                            /* 0x200A05EC */
void        ( *Scr_Shutdown )();                                                        /* 0x200A05F0 */
void        ( *Scr_Abort )();                                                           /* 0x200A05F4 */
void        ( *Scr_SetLoading )( int loading );                                         /* 0x200A05F8 */
void        ( *Scr_AllocGameVariable )( int classnum, int time );                       /* 0x200A05FC */
void        ( *Scr_InitSystem )( void );                                                /* 0x200A0600 */
void        ( *Scr_GetChecksum )();                                                     /* 0x200A0604 */
void        ( *Scr_HasSourceFiles )();                                                  /* 0x200A0608 */
void        ( *Scr_SaveSource )();                                                      /* 0x200A060C */
void        ( *Scr_LoadSource )();                                                      /* 0x200A0610 */
void        ( *Scr_SkipSource )();                                                      /* 0x200A0614 */
void        ( *Scr_SavePre )();                                                         /* 0x200A0618 */
void        ( *Scr_SavePost )();                                                        /* 0x200A061C */
void        ( *Scr_SaveShutdown )();                                                    /* 0x200A0620 */
void        ( *Scr_Unused4B )();                                                        /* 0x200A0624 -- never written */
void        ( *Scr_LoadPre )();                                                         /* 0x200A0628 */
void        ( *Scr_LoadShutdown )();                                                    /* 0x200A062C */
void        ( *Scr_Unused4E )();                                                        /* 0x200A0630 -- never written */
int         ( *Scr_LoadScript )( const char *scriptName );                              /* 0x200A0634 */
void        *( *Scr_FindAnimTree )( const char *treeName );                             /* 0x200A0638 */
void        ( *Scr_FindAnim )( const char *treeName, const char *animName, scr_anim_t *anim );  /* 0x200A063C */
int         ( *Scr_GetFunctionHandle )( const char *scriptName, const char *label );    /* 0x200A0640 */
void        ( *Scr_FreeThread )( unsigned short thread );                               /* 0x200A0644 */
void        ( *Scr_ConvertThreadToSave )();                                             /* 0x200A0648 */
void        ( *Scr_ConvertThreadFromLoad )();                                           /* 0x200A064C */
void        ( *Scr_SetString )( unsigned short *dest, unsigned short id );              /* 0x200A0650 */
unsigned short ( *Scr_AllocString )( const char *string, unsigned int user );           /* 0x200A0654 */
void        ( *Scr_NotifyNum )( int entnum, int classnum, unsigned short event, unsigned int paramcount );      /* 0x200A0658 */
void        ( *Scr_NotifyId )();                                                        /* 0x200A065C */
const char  *( *SL_ConvertToString )( unsigned short id );                              /* 0x200A0660 */
unsigned short ( *SL_GetString )( const char *string, unsigned int user );              /* 0x200A0664 */
unsigned short ( *SL_GetLowercaseString )( const char *string, unsigned int user );     /* 0x200A0668 */
unsigned short ( *SL_FindLowercaseString )( const char *string );                       /* 0x200A066C */
void        ( *Scr_CreateCanonicalFilename )();                                         /* 0x200A0670 */
void        ( *Scr_SetTime )( int time );                                               /* 0x200A0674 */
void        ( *Scr_RunCurrentThreads )( void );                                         /* 0x200A0678 */
void        ( *Scr_ResetTimeout )( void );                                              /* 0x200A067C */
int         ( *Scr_GetAnimsIndex )( int anims );                                        /* 0x200A0680 */
void        ( *Scr_GetAnims )();                                                        /* 0x200A0684 */
void        *( *MT_Alloc )( int size, int type );                                       /* 0x200A0688 */
void        ( *MT_Free )( void *p, int size );                                          /* 0x200A068C */
#pragma bss_seg()

/*
 * ---------------------------------------------------------------------------
 * g_scr_data -- the script handles and the game type list.
 * ---------------------------------------------------------------------------
 */

/* scr_data_t and its two helper types are in g_local.h: g_spawn_mp.c,
   g_hud_mp.c and g_client_mp.c all reach members of g_scr_data. */

scr_data_t g_scr_data;                          /* 0x202CD340 */

scr_const_t scr_const;                          /* 0x202CD500 */

/*
=============
GScr_AllocString
=============
*/
unsigned short GScr_AllocString( const char *string ) {
	return Scr_AllocString( string, 1 );
}

/*
=============
GScr_LoadConsts
=============
*/
void GScr_LoadConsts( void ) {
	scr_const.activate = Scr_AllocString( "active", 1 );
	scr_const.air_strike = Scr_AllocString( "air strike", 1 );
	scr_const.allies = Scr_AllocString( "allies", 1 );
	scr_const.animdone = Scr_AllocString( "animdone", 1 );
	scr_const.axis = Scr_AllocString( "axis", 1 );
	scr_const.bodyque = Scr_AllocString( "bodyque", 1 );
	scr_const.combat = Scr_AllocString( "combat", 1 );
	scr_const.connected = Scr_AllocString( "connected", 1 );
	scr_const.connecting = Scr_AllocString( "connecting", 1 );
	scr_const.count = Scr_AllocString( "count", 1 );
	scr_const.croutch = Scr_AllocString( "crouch", 1 );
	scr_const.crowbar = Scr_AllocString( "crowbar", 1 );
	scr_const.current = Scr_AllocString( "current", 1 );
	scr_const.damage = Scr_AllocString( "damage", 1 );
	scr_const.death = Scr_AllocString( "death", 1 );
	scr_const.disconnected = Scr_AllocString( "disconnected", 1 );
	scr_const.dlight = Scr_AllocString( "dlight", 1 );
	scr_const.done = Scr_AllocString( "done", 1 );
	scr_const.empty = Scr_AllocString( "empty", 1 );
	scr_const.enemy = Scr_AllocString( "enemy", 1 );
	scr_const.enemyhidden = Scr_AllocString( "enemyhidden", 1 );
	scr_const.enemyvisible = Scr_AllocString( "enemyvisible", 1 );
	scr_const.entity = Scr_AllocString( "entity", 1 );
	scr_const.failed = Scr_AllocString( "failed", 1 );
	scr_const.flamebarrel = Scr_AllocString( "flamebarrel", 1 );
	scr_const.fraction = Scr_AllocString( "fraction", 1 );
	scr_const.func_door = Scr_AllocString( "func_door", 1 );
	scr_const.func_door_rotating = Scr_AllocString( "func_door_rotating", 1 );
	scr_const.func_rotating = Scr_AllocString( "func_rotating", 1 );
	scr_const.func_tramcar = Scr_AllocString( "func_tramcar", 1 );
	scr_const.goal = Scr_AllocString( "goal", 1 );
	scr_const.grenade = Scr_AllocString( "grenade", 1 );
	scr_const.info_notnull = Scr_AllocString( "info_notnull", 1 );
	scr_const.invisible = Scr_AllocString( "invisible", 1 );
	scr_const.key1 = Scr_AllocString( "key1", 1 );
	scr_const.key2 = Scr_AllocString( "key2", 1 );
	scr_const.killanimscript = Scr_AllocString( "killanimscript", 1 );
	scr_const.left = Scr_AllocString( "left", 1 );
	scr_const.misc_flak = Scr_AllocString( "misc_flak", 1 );
	scr_const.misc_mg42 = Scr_AllocString( "misc_mg42", 1 );
	scr_const.misc_tagemitter = Scr_AllocString( "misc_tagemitter", 1 );
	scr_const.mortar = Scr_AllocString( "mortar", 1 );
	scr_const.movedone = Scr_AllocString( "movedone", 1 );
	scr_const.noclass = Scr_AllocString( "noclass", 1 );
	scr_const.noenemy = Scr_AllocString( "noenemy", 1 );
	scr_const.noncombat = Scr_AllocString( "noncombat", 1 );
	scr_const.normal = Scr_AllocString( "normal", 1 );
	scr_const.pisol = Scr_AllocString( "pistol", 1 );
	scr_const.place_waypoit = Scr_AllocString( "plane_waypoint", 1 );
	scr_const.player = Scr_AllocString( "player", 1 );
	scr_const.position = Scr_AllocString( "position", 1 );
	scr_const.primary = Scr_AllocString( "primary", 1 );
	scr_const.primaryb = Scr_AllocString( "primaryb", 1 );
	scr_const.prone = Scr_AllocString( "prone", 1 );
	scr_const.right = Scr_AllocString( "right", 1 );
	scr_const.rocket = Scr_AllocString( "rocket", 1 );
	scr_const.rotatedone = Scr_AllocString( "rotatedone", 1 );
	scr_const.script_brushmodel = Scr_AllocString( "script_brushmodel", 1 );
	scr_const.script_model = Scr_AllocString( "script_model", 1 );
	scr_const.script_origin = Scr_AllocString( "script_origin", 1 );
	scr_const.scriptcamera = Scr_AllocString( "scriptcamera", 1 );
	scr_const.spawned = Scr_AllocString( "spawned", 1 );
	scr_const.spectator = Scr_AllocString( "spectator", 1 );
	scr_const.stand = Scr_AllocString( "stand", 1 );
	scr_const.surfacetype = Scr_AllocString( "surfacetype", 1 );
	scr_const.tag_engine1 = Scr_AllocString( "tag_engine1", 1 );
	scr_const.tag_engine2 = Scr_AllocString( "tag_engine2", 1 );
	scr_const.target_location = Scr_AllocString( "target_location", 1 );
	scr_const.target_script_trigger = Scr_AllocString( "target_script_trigger", 1 );
	scr_const.tempEntity = Scr_AllocString( "tempEntity", 1 );
	scr_const.muzzleEntity = Scr_AllocString( "muzzleEntity", 1 );
	scr_const.smokegrenade = Scr_AllocString( "smokegrenade", 1 );
	scr_const.touch = Scr_AllocString( "touch", 1 );
	scr_const.trigger = Scr_AllocString( "trigger", 1 );
	scr_const.trigger_use = Scr_AllocString( "trigger_use", 1 );
	scr_const.trigger_damage = Scr_AllocString( "trigger_damage", 1 );
	scr_const.trigger_lookat = Scr_AllocString( "trigger_lookat", 1 );
	scr_const.truck_cam = Scr_AllocString( "truck_cam", 1 );
	scr_const.xmodel = Scr_AllocString( "xmodel/airborne", 1 );
	scr_const.wehrmacht = Scr_AllocString( "xmodel/wehrmacht", 1 );
	scr_const.worldspawn = Scr_AllocString( "worldspawn", 1 );
	scr_const.begin = Scr_AllocString( "begin", 1 );
	scr_const.dynamite = Scr_AllocString( "dynamite", 1 );
	scr_const.explosive_indicator = Scr_AllocString( "explosive_indicator", 1 );
	scr_const.flamechunk = Scr_AllocString( "flamechunk", 1 );
	scr_const.follow = Scr_AllocString( "follow", 1 );
	scr_const.free = Scr_AllocString( "free", 1 );
	scr_const.freed = Scr_AllocString( "freed", 1 );
	scr_const.func_leaky = Scr_AllocString( "func_leaky", 1 );
	scr_const.info_player_checkpoint = Scr_AllocString( "info_player_checkpoint", 1 );
	scr_const.initialize = Scr_AllocString( "initialize", 1 );
	scr_const.intermission = Scr_AllocString( "intermission", 1 );
	scr_const.item_stamina_brandy = Scr_AllocString( "item_stamina_brandy", 1 );
	scr_const.menuresponse = Scr_AllocString( "menuresponse", 1 );
	scr_const.misc_gunner_gun = Scr_AllocString( "misc_gunner_gun", 1 );
	scr_const.misc_gunner_ring = Scr_AllocString( "misc_gunner_ring", 1 );
	scr_const.mp_info_player_deathmatch = Scr_AllocString( "mp_info_player_deathmatch", 1 );
	scr_const.mp_info_player_intermission = Scr_AllocString( "mp_info_player_intermission", 1 );
	scr_const.mp_team_alliedplayer_respawn = Scr_AllocString( "mp_team_alliedplayer_respawn", 1 );
	scr_const.mp_team_alliedplayer_start = Scr_AllocString( "mp_team_alliedplayer_start", 1 );
	scr_const.mp_team_axisplayer_respawn = Scr_AllocString( "mp_team_axisplayer_respawn", 1 );
	scr_const.mp_team_axisplayer_start = Scr_AllocString( "mp_team_axisplayer_start", 1 );
	scr_const.nail = Scr_AllocString( "nail", 1 );
	scr_const.not = Scr_AllocString( "not", 1 );
	scr_const.playing = Scr_AllocString( "playing", 1 );
	scr_const.prox_mine = Scr_AllocString( "prox_mine", 1 );
	scr_const.reset = Scr_AllocString( "reset", 1 );
	scr_const.script_mover = Scr_AllocString( "script_mover", 1 );
	scr_const.script_multiplayer = Scr_AllocString( "script_multiplayer", 1 );
	scr_const.spear = Scr_AllocString( "spear", 1 );
	scr_const.tag_hand = Scr_AllocString( "tag_hand", 1 );
	scr_const.tag_rider = Scr_AllocString( "tag_rider", 1 );
	scr_const.tag_ring = Scr_AllocString( "tag_ring", 1 );
	scr_const.team_CTF_blueflag = Scr_AllocString( "team_CTF_blueflag", 1 );
	scr_const.team_CTF_redflag = Scr_AllocString( "team_CTF_redflag", 1 );
	scr_const.team_WOLF_checkpoint = Scr_AllocString( "team_WOLF_checkpoint", 1 );
	scr_const.team_WOLF_objective = Scr_AllocString( "team_WOLF_objective", 1 );
	scr_const.trigger_aidoor = Scr_AllocString( "trigger_aidoor", 1 );
	scr_const.trigger_flagonly = Scr_AllocString( "trigger_flagonly", 1 );
	scr_const.trigger_multiple = Scr_AllocString( "trigger_multiple", 1 );
	scr_const.trigger_objective_info = Scr_AllocString( "trigger_objective_info", 1 );
	scr_const.waiting_for_players = Scr_AllocString( "waiting_for_players", 1 );
	scr_const.WP = Scr_AllocString( "WP", 1 );
	scr_const.zombiespit = Scr_AllocString( "zombiespit", 1 );
	scr_const.none = Scr_AllocString( "none", 1 );
	scr_const.dead = Scr_AllocString( "dead", 1 );
	scr_const.auto_change = Scr_AllocString( "auto_change", 1 );
	scr_const.manual_change = Scr_AllocString( "manual_change", 1 );
}

/*
=============
Scr_LoadLevel
=============
*/
void Scr_LoadLevel( void ) {
	if ( g_scr_data.levelScriptMain ) {
		Scr_FreeThread( Scr_ExecThread( g_scr_data.levelScriptMain, 0 ) );
	}
}

/*
=============
GScr_LoadScriptAndLabel
=============
*/
int GScr_LoadScriptAndLabel( const char *scriptName, const char *label, qboolean mandatory ) {
	int handle;

	if ( !Scr_LoadScript( scriptName ) && mandatory ) {
		G_Error( "Could not find script '%s'", scriptName );
	}

	handle = Scr_GetFunctionHandle( scriptName, label );
	if ( !handle && mandatory ) {
		G_Error( "Could not find label '%s' in script '%s'", label, scriptName );
	}

	return handle;
}

/*
=============
GScr_LoadGameTypeScript
=============
*/
void GScr_LoadGameTypeScript( void ) {
	char scriptName[64];

	Com_sprintf( scriptName, sizeof( scriptName ), "maps/mp/gametypes/%s", g_gametype.string );

	g_scr_data.gameTypeMain = GScr_LoadScriptAndLabel( scriptName, "main", qtrue );
	g_scr_data.gameTypeStartup = GScr_LoadScriptAndLabel( "maps/mp/gametypes/_callbacksetup",
														  "CodeCallback_StartGameType", qtrue );
	g_scr_data.playerConnect = GScr_LoadScriptAndLabel( "maps/mp/gametypes/_callbacksetup",
														"CodeCallback_PlayerConnect", qtrue );
	g_scr_data.playerDisconnect = GScr_LoadScriptAndLabel( "maps/mp/gametypes/_callbacksetup",
														   "CodeCallback_PlayerDisconnect", qtrue );
	g_scr_data.playerDamage = GScr_LoadScriptAndLabel( "maps/mp/gametypes/_callbacksetup",
													   "CodeCallback_PlayerDamage", qtrue );
	g_scr_data.playerKilled = GScr_LoadScriptAndLabel( "maps/mp/gametypes/_callbacksetup",
													   "CodeCallback_PlayerKilled", qtrue );
}

/*
=============
GScr_LoadLevelScript
=============
*/
void GScr_LoadLevelScript( void ) {
	char scriptName[64];
	vmCvar_t mapname;

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	Com_sprintf( scriptName, sizeof( scriptName ), "maps/mp/%s", mapname.string );

	Scr_LoadScript( scriptName );
	g_scr_data.levelScriptMain = Scr_GetFunctionHandle( scriptName, "main" );
}

/* A two-pointer .data array at 0x20069768, just ahead of scr_functions;
   GScr_PostLoadScripts loads both names through it (0x2002F060) rather than
   storing the literals.  Name invented -- retail carries no symbol for it. */
static const char *scr_classnames[SCR_CLASS_NUM] = { "entity", "hudelem" };

/*
=============
GScr_PostLoadScripts
=============
*/
void GScr_PostLoadScripts( void ) {
	g_scr_data.classMap[SCR_CLASS_ENTITY].name = scr_classnames[SCR_CLASS_ENTITY];
	g_scr_data.classMap[SCR_CLASS_HUDELEM].name = scr_classnames[SCR_CLASS_HUDELEM];
	Scr_SetClassMap( g_scr_data.classMap, SCR_CLASS_NUM );

	GScr_AddFieldsForEntity();
	GScr_AddFieldsForHudElems();
	Scr_AddFields( "radiant", ".txt" );
}

/*
=============
Hunk_AllocXAnimCreate
=============
*/
void *Hunk_AllocXAnimCreate( int size ) {
	return trap_Hunk_AllocLowInternal( size );
}

/*
=============
GScr_LoadScripts
=============
*/
void GScr_LoadScripts( void ) {
	/* The two BG_AnimParseAnimScript out-parameters: 512 animScriptAnim_t (the
	 * type is private to bg_animation.c) and the count of them.  The count is
	 * zeroed at the top of the retail function (0x2002F0DA) because
	 * BG_AnimationIndexForString reads it before the first write. */
	int numAnims = 0;
	byte animScriptBuffer[36864];

	Scr_BeginLoadScripts();

	GScr_LoadGameTypeScript();
	GScr_LoadLevelScript();
	GScr_PostLoadScripts();

	Scr_EndLoadScripts();

	BG_FindAnims();
	BG_AnimParseAnimScript( &bgs_animScriptData, animScriptBuffer, &numAnims );
	Scr_PrecacheAnimTrees( Hunk_AllocXAnimCreate );
	BG_FindAnimTrees();
	Scr_EndLoadAnimTrees();
	BG_FinalizePlayerAnims();
	BG_CreateClientAnimTrees();
}

/*
=============
GScr_FreeScripts
=============
*/
void GScr_FreeScripts( void ) {
	Scr_RemoveClassMap();
}

/*
=============
GetEntity
=============
*/
gentity_t *GetEntity( unsigned int entnum ) {
	if ( entnum >= MAX_GENTITIES ) {
		Scr_ObjectError( "not an entity" );
		return NULL;
	}
	return &g_entities[entnum];
}

/*
=============
GetPlayerEntity
=============
*/
gentity_t *GetPlayerEntity( unsigned int entnum ) {
	gentity_t   *ent;
	const char  *classname;
	const char  *targetname;

	ent = GetEntity( entnum );

	if ( !ent->client ) {
		if ( ent->targetname ) {
			targetname = SL_ConvertToString( ent->targetname );
		} else {
			targetname = "<undefined>";
		}
		classname = SL_ConvertToString( ent->classname );
		Scr_Error( va( "only valid on players; called on entity %i at %.0f %.0f %.0f classname %s targetname %s\n",
					   entnum, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
					   classname, targetname ) );
	}

	return ent;
}

/*
=============
print
=============
*/
void print( void ) {
	int i;
	int paramcount;

	if ( g_no_script_spam.integer ) {
		return;
	}

	paramcount = Scr_GetNumParam();
	for ( i = 0; i < paramcount; i++ ) {
		Com_Printf( "%s", Scr_GetDebugString( i ) );
	}
}

/*
=============
println
=============
*/
void println( void ) {
	if ( g_no_script_spam.integer ) {
		return;
	}

	print();
	Com_Printf( "\n" );
}

/*
=============
Scr_LocalizationError
=============
*/
void Scr_LocalizationError( const char *error ) {
	Com_Error( ERR_LOCALIZATION, error );
}

/*
 * Message types Scr_ConstructMessageString names in its diagnostics.
 */
typedef enum messageType_e {
	MESSAGE_GAME            = 0,
	MESSAGE_CVAR_VALUE      = 1,
	MESSAGE_HINT_STRING     = 2,
	MESSAGE_ANNOUNCEMENT    = 3,
	MESSAGE_CLIENT_CVAR     = 4,
	MESSAGE_CLIENT_CHAT     = 5
} messageType_t;

/* The chunk markers the client's message parser splits on. */
#define MSG_TOKEN_LOCALIZED         20
#define MSG_TOKEN_RAW               21
#define MSG_TOKEN_RESERVED          22

/*
=============
Scr_ConstructMessageString

Joins the script parameters from firstParam on into one client message,
prefixing each run with the marker that says whether the client should look it
up in the localization table or print it verbatim.
=============
*/
void Scr_ConstructMessageString( int msgType, int firstParam, char *string, int maxLen ) {
	const char  *typeName;
	const char  *token;
	gentity_t   *ent;
	int paramIndex;
	int paramCount;
	int i;
	int len;
	int total;
	int marker;
	int markerLen;
	int hasLetters;
	int rawPending;

	switch ( msgType ) {
	case MESSAGE_GAME:
		typeName = "Game Message";
		break;
	case MESSAGE_CVAR_VALUE:
		typeName = "Cvar Value";
		break;
	case MESSAGE_HINT_STRING:
		typeName = "Hint String";
		break;
	case MESSAGE_ANNOUNCEMENT:
		typeName = "Announcement String";
		break;
	case MESSAGE_CLIENT_CVAR:
		typeName = "Client Cvar Value";
		break;
	case MESSAGE_CLIENT_CHAT:
		typeName = "Client Chat Message";
		break;
	default:
		typeName = "BAD";
		break;
	}

	rawPending = 1;
	total = 0;
	string[0] = 0;

	paramCount = Scr_GetNumParam();
	for ( paramIndex = firstParam; paramIndex < paramCount; paramIndex++ ) {
		markerLen = 0;

		if ( Scr_GetType( paramIndex ) == VAR_LOCALIZED_STRING ) {
			token = Scr_GetIString( paramIndex );
			len = strlen( token );

			for ( i = 0; i < len; i++ ) {
				if ( !isalnum( token[i] ) && token[i] != '_' ) {
					Scr_ParamError( paramIndex,
									va( "Illegal localized string reference: %s (must contain only alpha-numeric characters and underscores",
										token ) );
				}
			}

			if ( total ) {
				markerLen = 1;
			}
			if ( total + markerLen + len >= maxLen ) {
				Scr_ParamError( paramIndex, va( "%s is too long. Max length is %i\n", typeName, maxLen ) );
			}
			if ( markerLen ) {
				string[total] = MSG_TOKEN_LOCALIZED;
				string[total + 1] = 0;
				total += markerLen;
			}
			rawPending = 1;
		} else if ( Scr_GetType( paramIndex ) == VAR_OBJECT
					&& Scr_GetPointerType( paramIndex ) == VAR_ENTITY ) {
			ent = Scr_GetEntity( paramIndex );
			if ( !ent->client ) {
				Scr_ParamError( paramIndex, "Entity is not a player" );
			}

			token = va( "%s^7", ent->client->sess.name );
			len = strlen( token );

			if ( rawPending ) {
				markerLen = 1;
			}
			if ( total + markerLen + len >= maxLen ) {
				Scr_ParamError( paramIndex, va( "%s is too long. Max length is %i\n", typeName, maxLen ) );
			}
			if ( markerLen ) {
				string[total] = MSG_TOKEN_RAW;
				string[total + 1] = 0;
				total += markerLen;
			}
			rawPending = 0;
		} else {
			hasLetters = 0;
			token = Scr_GetString( paramIndex );
			len = strlen( token );

			for ( i = 0; i < len; i++ ) {
				if ( token[i] == MSG_TOKEN_LOCALIZED || token[i] == MSG_TOKEN_RAW
					 || token[i] == MSG_TOKEN_RESERVED ) {
					Scr_ParamError( paramIndex,
									va( "bad escape character (%i) present in string", token[i] ) );
				}
				if ( isalpha( token[i] ) ) {
					if ( !hasLetters && cl_languagewarnings.integer ) {
						if ( cl_languagewarningsaserrors.integer ) {
							Com_Error( ERR_LOCALIZATION,
									   va( "non-localized %s strings are not allowed to have letters in them: \"%s\"",
										   typeName, token ) );
						} else {
							Com_Printf( "^3WARNING: Non-localized %s string is not allowed to have letters in it. Must be changed over to a localized string: \"%s\"\n",
										typeName, token );
						}
					}
					hasLetters = 1;
				}
			}

			if ( hasLetters ) {
				marker = MSG_TOKEN_LOCALIZED;
				if ( total ) {
					markerLen = 1;
				}
			} else {
				marker = MSG_TOKEN_RAW;
				if ( rawPending ) {
					markerLen = 1;
				}
			}

			if ( total + markerLen + len >= maxLen ) {
				Scr_ParamError( paramIndex, va( "%s is too long. Max length is %i\n", typeName, maxLen ) );
			}
			if ( markerLen ) {
				string[total] = marker;
				string[total + 1] = 0;
				total += markerLen;
			}
			rawPending = 0;
		}

		for ( i = 0; i < len; i++ ) {
			if ( ( token[i] && (byte)token[i] <= 0x1F ) || (byte)token[i] >= 0x7F ) {
				Scr_ParamError( paramIndex,
								va( "Illegal character '%c'(ascii %i) in %s: %s\n",
									token[i], (byte)token[i], token, typeName ) );
			}
		}

		strcpy( &string[total], token );
		total += len;
	}

	if ( rawPending ) {
		string[total] = MSG_TOKEN_RAW;
		string[total + 1] = 0;
	}
}

/*
=============
Scr_MakeGameMessage
=============
*/
void Scr_MakeGameMessage( int clientNum, const char *command ) {
	char string[MAX_STRING_CHARS];

	Scr_ConstructMessageString( MESSAGE_GAME, 0, string, sizeof( string ) );
	trap_SendServerCommand( clientNum, 0, va( "%s \"%s\"", command, string ) );
}

/*
=============
iprintln
=============
*/
void iprintln( void ) {
	Scr_MakeGameMessage( -1, "f" );
}

/*
=============
iprintlnbold
=============
*/
void iprintlnbold( void ) {
	Scr_MakeGameMessage( -1, "g" );
}

/*
=============
GScr_print3d
=============
*/
void GScr_print3d( void ) {
	const char  *text;
	vec3_t origin;
	vec3_t color;
	float scale;

	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
	scale = 1.0;

	if ( Scr_GetNumParam() > 2 ) {
		if ( Scr_GetNumParam() > 3 ) {
			if ( Scr_GetNumParam() > 4 ) {
				Scr_GetFloat( 4 );
			}
			scale = Scr_GetFloat( 3 );
		}
		Scr_GetVector( 2, color );
	}

	text = Scr_GetString( 1 );
	Scr_GetVector( 0, origin );

	trap_AddDebugString( origin, color, scale, text );
}

/*
=============
GScr_line
=============
*/
void GScr_line( void ) {
	vec3_t start;
	vec3_t end;
	vec3_t color;
	int depthTest;

	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
	depthTest = 1;

	if ( Scr_GetNumParam() > 2 ) {
		if ( Scr_GetNumParam() > 3 ) {
			if ( Scr_GetNumParam() > 4 ) {
				Scr_GetInt( 4 );
			}
			depthTest = (int)Scr_GetFloat( 3 );
		}
		Scr_GetVector( 2, color );
	}

	Scr_GetVector( 1, end );
	Scr_GetVector( 0, start );

	trap_AddDebugLine( start, end, color, depthTest, 0 );
}

/*
=============
GScr_Assert

The retail release build compiles the assert away entirely.
=============
*/
void GScr_Assert( void ) {
}

/*
=============
GScr_IsDefined
=============
*/
void GScr_IsDefined( void ) {
	int type;

	type = Scr_GetType( 0 );
	if ( type != VAR_OBJECT ) {
		Scr_AddInt( type != VAR_UNDEFINED );
		return;
	}

	/* a dead thread / entity / object reads as undefined */
	Scr_AddInt( Scr_GetPointerType( 0 ) < VAR_DEAD_THREAD );
}

/*
=============
GScr_IsAlive
=============
*/
void GScr_IsAlive( void ) {
	if ( Scr_GetType( 0 ) == VAR_OBJECT && Scr_GetPointerType( 0 ) == VAR_ENTITY
		 && Scr_GetEntity( 0 )->health > 0 ) {
		Scr_AddInt( 1 );
	} else {
		Scr_AddInt( 0 );
	}
}

/*
=============
GScr_GetCvar
=============
*/
void GScr_GetCvar( void ) {
	char value[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer( Scr_GetString( 0 ), value, sizeof( value ) );
	Scr_AddString( value );
}

/*
=============
GScr_GetCvarInt
=============
*/
void GScr_GetCvarInt( void ) {
	Scr_AddInt( trap_Cvar_VariableIntegerValue( Scr_GetString( 0 ) ) );
}

/*
=============
GScr_GetCvarFloat
=============
*/
void GScr_GetCvarFloat( void ) {
	Scr_AddFloat( trap_Cvar_VariableValue( Scr_GetString( 0 ) ) );
}

/*
=============
GScr_SetCvar
=============
*/
void GScr_SetCvar( void ) {
	const char  *name;
	const char  *value;
	char message[MAX_STRING_CHARS];
	char cleaned[MAX_STRING_CHARS];
	int i;
	char c;

	name = Scr_GetString( 0 );

	if ( Scr_GetType( 1 ) == VAR_LOCALIZED_STRING ) {
		Scr_ConstructMessageString( MESSAGE_CVAR_VALUE, 1, message, sizeof( message ) );
		value = message;
	} else {
		value = Scr_GetString( 1 );
	}

	/* The destination is 1024 bytes but the loop bound is 8192 -- retail:
	   GScr_SetCvar 0x2002FB0B and GScr_MakeCvarServerInfo 0x20034B69 both
	   walk 0x2000 into a 0x400 frame slot.  Reproduced as-is.  The scrubbed
	   copy in cleaned[] is dead: retail (0x2002FB43 push edi) registers/sets
	   the original value, not the scrubbed buffer, and the loop never
	   terminates cleaned[]. */
	for ( i = 0; i < 0x2000; i++ ) {
		c = value[i];
		if ( !c ) {
			break;
		}
		if ( c == (char)0x92 ) {
			c = '\'';
		} else if ( (byte)c > 0x7F ) {
			c = '.';
		}
		cleaned[i] = c;
		if ( c == '"' ) {
			cleaned[i] = '\'';
		}
	}

	trap_Cvar_Register( NULL, name, value, CVAR_UNSAFE );
	trap_Cvar_Set( name, value );
}

/*
=============
GScr_GetTime
=============
*/
void GScr_GetTime( void ) {
	Scr_AddInt( level.time );
}

/*
=============
Scr_GetEntByNum
=============
*/
void Scr_GetEntByNum( void ) {
	unsigned int entnum;
	gentity_t   *ent;

	entnum = Scr_GetInt( 0 );
	if ( entnum >= MAX_GENTITIES ) {
		return;
	}

	ent = &g_entities[entnum];
	if ( ent->inuse ) {
		Scr_AddEntityNum( ent->s.number, SCR_CLASS_ENTITY );
	}
}

/*
=============
Scr_GetWeaponModel
=============
*/
void Scr_GetWeaponModel( void ) {
	const char  *name;
	byte weapon;

	name = Scr_GetString( 0 );
	weapon = BG_GetWeaponIndexForName( name );
	if ( weapon ) {
		Scr_AddString( bg_weaponInfo[weapon]->worldModel );
		return;
	}

	if ( name[0] && Q_stricmp( name, "none" ) ) {
		Com_Printf( va( "unknown weapon '%s' in getWeaponModel\n", name ) );
	}
	Scr_AddString( "" );
}

/*
=============
Scr_GetWeaponClassname
=============
*/
void Scr_GetWeaponClassname( void ) {
	const char  *name;
	int weapon;
	int i;

	name = Scr_GetString( 0 );
	weapon = BG_GetWeaponIndexForName( name );

	if ( !weapon ) {
		if ( name[0] && Q_stricmp( name, "none" ) ) {
			Com_Printf( va( "unknown weapon '%s' in getWeaponClassname\n", name ) );
		}
		Scr_AddUndefined();
		return;
	}

	/* walk the alt-weapon ring until one of them carries a Radiant name */
	i = weapon;
	while ( !bg_weaponInfo[i]->radiantName[0] ) {
		i = bg_weaponInfo[i]->altWeaponIndex;
		if ( !i || i == weapon ) {
			Com_Printf( va( "^3WARNING^7: no Radiant name found for weapon '%s' in getWeaponClassname\n", name ) );
			Scr_AddUndefined();
			return;
		}
	}

	Scr_AddString( bg_weaponInfo[i]->radiantName );
}

/*
=============
GScr_GetAnimLength
=============
*/
void GScr_GetAnimLength( void ) {
	scr_anim_t anim;

	anim = Scr_GetAnim( 0, NULL );
	if ( !trap_XAnimIsPrimitive( anim ) ) {
		Scr_ParamError( 0, "non-primitive animation has no concept of length" );
	}
	Scr_AddFloat( trap_XAnimGetLengthSeconds( anim ) );
}

/*
=============
GScr_AnimHasNotetrack
=============
*/
void GScr_AnimHasNotetrack( void ) {
	scr_anim_t anim;
	unsigned short notetrack;

	anim = Scr_GetAnim( 0, NULL );
	notetrack = Scr_GetConstString( 1 );
	Scr_AddBool( trap_XAnimNotetrackExists( anim, notetrack ) );
}

/*
=============
GScr_GetBrushModelCenter
=============
*/
void GScr_GetBrushModelCenter( void ) {
	gentity_t   *ent;
	vec3_t center;
	int classnum;
	unsigned int entnum;

	entnum = Scr_GetEntityNum( 0, &classnum );
	if ( classnum || entnum >= MAX_GENTITIES ) {
		Scr_ParamError( 0, "not an entity" );
		ent = NULL;
	} else {
		ent = &g_entities[entnum];
	}

	center[0] = ( ent->r.mins[0] + ent->r.maxs[0] ) * 0.5f;
	center[1] = ( ent->r.mins[1] + ent->r.maxs[1] ) * 0.5f;
	center[2] = ( ent->r.mins[2] + ent->r.maxs[2] ) * 0.5f;

	Scr_AddVector( center );
}

/*
=============
GScr_Spawn
=============
*/
void GScr_Spawn( void ) {
	unsigned short classname;
	vec3_t origin;
	int spawnflags;
	gentity_t   *ent;

	classname = Scr_GetConstString( 0 );
	Scr_GetVector( 1, origin );

	if ( Scr_GetNumParam() > 2 ) {
		spawnflags = Scr_GetInt( 2 );
	} else {
		spawnflags = 0;
	}

	ent = G_Spawn();
	Scr_SetString( &ent->classname, classname );
	ent->r.currentOrigin[0] = origin[0];
	ent->r.currentOrigin[1] = origin[1];
	ent->r.currentOrigin[2] = origin[2];
	ent->spawnflags = spawnflags;

	if ( !G_CallSpawnEntity( ent ) ) {
		Scr_Error( va( "unable to spawn \"%s\" entity", SL_ConvertToString( classname ) ) );
		return;
	}

	Scr_AddEntityNum( ent->s.number, SCR_CLASS_ENTITY );
}

/*
=============
ScrCmd_attach
=============
*/
void ScrCmd_attach( unsigned int entnum ) {
	gentity_t   *ent;
	const char  *modelName;
	const char  *tagName;

	ent = GetEntity( entnum );

	modelName = Scr_GetString( 0 );
	if ( Scr_GetNumParam() < 2 ) {
		tagName = "";
	} else {
		tagName = Scr_GetString( 1 );
	}

	if ( G_EntDetach( ent, modelName, tagName ) ) {
		Scr_Error( va( "model '%s' already attached to tag '%s'", modelName, tagName ) );
	}

	if ( !G_EntAttach( ent, modelName, tagName ) ) {
		Scr_Error( "maximum attached models exceeded" );
	}
}

/*
=============
ScrCmd_detach
=============
*/
void ScrCmd_detach( unsigned int entnum ) {
	gentity_t   *ent;
	const char  *modelName;
	const char  *tagName;
	int i;

	ent = GetEntity( entnum );

	modelName = Scr_GetString( 0 );
	if ( Scr_GetNumParam() < 2 ) {
		tagName = "";
	} else {
		tagName = Scr_GetString( 1 );
	}

	if ( G_EntDetach( ent, modelName, tagName ) ) {
		return;
	}

	Com_Printf( "Current attachments:\n" );
	for ( i = 0; i < MAX_ATTACHED_MODELS; i++ ) {
		if ( !ent->attachModelIndex[i] ) {
			continue;
		}
		if ( !ent->attachTagName[i] ) {
			continue;
		}
		Com_Printf( "model: '%s', tag: '%s'\n",
					trap_GetConfigstringConst( CS_MODELS + ent->attachModelIndex[i] ),
					SL_ConvertToString( ent->attachTagName[i] ) );
	}

	Scr_Error( va( "failed to detach model '%s' from tag '%s'", modelName, tagName ) );
}

/*
=============
ScrCmd_detachAll
=============
*/
void ScrCmd_detachAll( unsigned int entnum ) {
	G_EntDetachAll( GetEntity( entnum ) );
}

/*
=============
ScrCmd_GetAttachSize
=============
*/
void ScrCmd_GetAttachSize( unsigned int entnum ) {
	gentity_t   *ent;
	int i;

	ent = GetEntity( entnum );

	for ( i = 0; i < MAX_ATTACHED_MODELS; i++ ) {
		if ( !ent->attachModelIndex[i] ) {
			break;
		}
	}

	Scr_AddInt( i );
}

/*
=============
ScrCmd_GetAttachModelName
=============
*/
void ScrCmd_GetAttachModelName( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned int index;

	ent = GetEntity( entnum );

	index = Scr_GetInt( 0 );
	if ( index >= MAX_ATTACHED_MODELS || !ent->attachModelIndex[index] ) {
		Scr_ParamError( 0, "bad index" );
	}

	Scr_AddString( trap_GetConfigstringConst( CS_MODELS + ent->attachModelIndex[index] ) );
}

/*
=============
ScrCmd_GetAttachTagName
=============
*/
void ScrCmd_GetAttachTagName( unsigned int entnum ) {
	gentity_t   *ent;
	unsigned int index;

	ent = GetEntity( entnum );

	index = Scr_GetInt( 0 );
	if ( index >= MAX_ATTACHED_MODELS || !ent->attachModelIndex[index] ) {
		Scr_ParamError( 0, "bad index" );
	}

	Scr_AddConstString( ent->attachTagName[index] );
}

/*
=============
ScrCmd_LinkTo
=============
*/
void ScrCmd_LinkTo( unsigned int entnum ) {
	gentity_t   *ent;
	gentity_t   *parent;
	const char  *tagName;
	int numParam;
	int classnum;
	unsigned int parentNum;
	vec3_t originOffset;
	vec3_t anglesOffset;

	ent = GetEntity( entnum );

	if ( Scr_GetType( 0 ) != VAR_OBJECT || Scr_GetPointerType( 0 ) != VAR_ENTITY ) {
		Scr_ParamError( 0, "not an entity" );
	}

	if ( !( ent->flags & FL_SUPPORTS_LINKTO ) ) {
		Scr_ObjectError( va( "entity (classname: '%s') does not currently support linkTo",
							 SL_ConvertToString( ent->classname ) ) );
	}

	parentNum = Scr_GetEntityNum( 0, &classnum );
	if ( classnum || parentNum >= MAX_GENTITIES ) {
		Scr_ParamError( 0, "not an entity" );
		parent = NULL;
	} else {
		parent = &g_entities[parentNum];
	}

	numParam = Scr_GetNumParam();
	if ( numParam < 2 ) {
		tagName = "";
	} else {
		tagName = Scr_GetString( 1 );
	}

	if ( numParam > 2 ) {
		Scr_GetVector( 2, originOffset );
		Scr_GetVector( 3, anglesOffset );
		if ( G_EntLinkToWithOffset( ent, parent, tagName, originOffset, anglesOffset ) ) {
			return;
		}
	} else if ( G_EntLinkTo( ent, parent, tagName ) ) {
		return;
	}

	/* the link failed -- work out which of the three reasons it was */
	if ( !trap_DObjExists( parent ) ) {
		if ( !parent->model ) {
			Scr_Error( "failed to link entity since parent has no model" );
		}
		Scr_Error( va( "failed to link entity since parent model '%s' is invalid",
					   trap_GetConfigstringConst( CS_MODELS + parent->model ) ) );
	}

	if ( tagName[0] ) {
		if ( trap_DObjGetBoneIndex( parent, tagName ) < 0 ) {
			trap_DObjDumpInfo( parent );
			Scr_Error( va( "failed to link entity since tag '%s' does not exist in parent model '%s'",
						   tagName, trap_GetConfigstringConst( CS_MODELS + parent->model ) ) );
		}
	}

	Scr_Error( "failed to link entity due to link cycle" );
}

/*
=============
ScrCmd_Unlink
=============
*/
void ScrCmd_Unlink( unsigned int entnum ) {
	G_EntUnlink( GetEntity( entnum ) );
}

/*
=============
ScrCmd_EnableLinkTo
=============
*/
void ScrCmd_EnableLinkTo( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );

	if ( ent->flags & FL_SUPPORTS_LINKTO ) {
		Scr_ObjectError( "entity already has linkTo enabled" );
	}

	if ( ent->s.eType || ent->physicsObject
		 || ( ( ent->nextthink || ent->think )
			  && _stricmp( SL_ConvertToString( ent->classname ), "trigger_multiple" ) ) ) {
		Scr_ObjectError( va( "entity (classname: '%s') does not currently support enableLinkTo",
							 SL_ConvertToString( ent->classname ) ) );
	}

	ent->nextthink = level.time;
	ent->think = Think_GeneralLink;
	ent->flags |= FL_SUPPORTS_LINKTO;
}

/*
=============
ScrCmd_GetOrigin
=============
*/
void ScrCmd_GetOrigin( unsigned int entnum ) {
	gentity_t   *ent;
	vec3_t origin;

	ent = GetEntity( entnum );

	origin[0] = ent->r.currentOrigin[0];
	origin[1] = ent->r.currentOrigin[1];
	origin[2] = ent->r.currentOrigin[2];

	Scr_AddVector( origin );
}

/*
=============
ScrCmd_GetEye
=============
*/
void ScrCmd_GetEye( unsigned int entnum ) {
	gentity_t   *ent;
	vec3_t eye;

	ent = GetEntity( entnum );

	eye[0] = ent->r.currentOrigin[0];
	eye[1] = ent->r.currentOrigin[1];
	eye[2] = ent->r.currentOrigin[2] + 40.0f;

	Scr_AddVector( eye );
}

/*
=============
ScrCmd_UseBy
=============
*/
void ScrCmd_UseBy( unsigned int entnum ) {
	gentity_t   *ent;
	gentity_t   *other;
	int classnum;
	unsigned int otherNum;

	ent = GetEntity( entnum );

	otherNum = Scr_GetEntityNum( 0, &classnum );
	if ( classnum || otherNum >= MAX_GENTITIES ) {
		Scr_ParamError( 0, "not an entity" );
		other = NULL;
	} else {
		other = &g_entities[otherNum];
	}

	Scr_AddEntityNum( other->s.number, SCR_CLASS_ENTITY );
	Scr_NotifyNum( ent->s.number, SCR_CLASS_ENTITY, scr_const.trigger, 1 );

	if ( ent->use ) {
		ent->use( ent, other, other );
	}
}

/*
=============
ScrCmd_IsTouching
=============
*/
void ScrCmd_IsTouching( unsigned int entnum ) {
	gentity_t   *self;
	gentity_t   *other;
	int classnum;
	unsigned int otherNum;
	vec3_t mins;
	vec3_t maxs;
	vec3_t otherMins;
	vec3_t otherMaxs;

	self = GetEntity( entnum );

	/*
	 * The bounds handed to the contact trap have to be the brush model's, so
	 * when the object is one the two entities trade places.
	 */
	if ( self->r.bmodel ) {
		otherNum = Scr_GetEntityNum( 0, &classnum );
		if ( classnum || otherNum >= MAX_GENTITIES ) {
			Scr_ParamError( 0, "not an entity" );
			self = NULL;
		} else {
			self = &g_entities[otherNum];
		}
		other = GetEntity( entnum );
	} else {
		otherNum = Scr_GetEntityNum( 0, &classnum );
		if ( classnum || otherNum >= MAX_GENTITIES ) {
			Scr_ParamError( 0, "not an entity" );
			other = NULL;
		} else {
			other = &g_entities[otherNum];
		}
	}

	mins[0] = self->r.mins[0] + self->r.currentOrigin[0];
	mins[1] = self->r.mins[1] + self->r.currentOrigin[1];
	mins[2] = self->r.mins[2] + self->r.currentOrigin[2];
	maxs[0] = self->r.maxs[0] + self->r.currentOrigin[0];
	maxs[1] = self->r.maxs[1] + self->r.currentOrigin[1];
	maxs[2] = self->r.maxs[2] + self->r.currentOrigin[2];

	if ( self->r.bmodel && other->r.bmodel ) {
		otherMins[0] = other->r.mins[0] + other->r.currentOrigin[0];
		otherMins[1] = other->r.mins[1] + other->r.currentOrigin[1];
		otherMins[2] = other->r.mins[2] + other->r.currentOrigin[2];
		otherMaxs[0] = other->r.maxs[0] + other->r.currentOrigin[0];
		otherMaxs[1] = other->r.maxs[1] + other->r.currentOrigin[1];
		otherMaxs[2] = other->r.maxs[2] + other->r.currentOrigin[2];

		if ( mins[0] >= otherMaxs[0] && mins[1] >= otherMaxs[1] && mins[2] >= otherMaxs[2]
			 && otherMins[0] >= maxs[0] && otherMins[1] >= maxs[1] && otherMins[2] >= maxs[2] ) {
			Scr_AddInt( 0 );
			return;
		}
	} else {
		if ( self->r.svFlags & SVF_CAPSULE ) {
			if ( !trap_EntityContactCapsule( mins, maxs, other ) ) {
				Scr_AddInt( 0 );
				return;
			}
		} else {
			if ( !trap_EntityContact( mins, maxs, other ) ) {
				Scr_AddInt( 0 );
				return;
			}
		}
	}

	Scr_AddInt( 1 );
}

/*
=============
ScrCmd_LockDoor
=============
*/
void ScrCmd_LockDoor( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );
	if ( ent->classname == scr_const.func_door_rotating || ent->classname == scr_const.func_door ) {
		ent->key = 1;
	}
}

/*
=============
ScrCmd_UnlockDoor
=============
*/
void ScrCmd_UnlockDoor( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );
	if ( ent->classname == scr_const.func_door_rotating || ent->classname == scr_const.func_door ) {
		ent->key = 0;
	}
}

/*
=============
ScrCmd_IsDoorLocked
=============
*/
void ScrCmd_IsDoorLocked( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );
	if ( ( ent->classname == scr_const.func_door_rotating || ent->classname == scr_const.func_door )
		 && ent->key ) {
		Scr_AddInt( 1 );
	} else {
		Scr_AddInt( 0 );
	}
}

/*
=============
ScrCmd_PlaySound
=============
*/
void ScrCmd_PlaySound( unsigned int entnum ) {
	gentity_t   *ent;
	byte alias;

	ent = GetEntity( entnum );
	alias = G_SoundAliasIndex( Scr_GetString( 0 ) );

	ent->r.svFlags &= ~SVF_NOCLIENT;
	ent->r.svFlags |= SVF_BROADCAST;

	if ( alias ) {
		G_AddEvent( ent, EV_SOUND_ALIAS, alias );
	}
}

/*
=============
ScrCmd_PlayLoopSound
=============
*/
void ScrCmd_PlayLoopSound( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );
	ent->s.loopSound = G_SoundAliasIndex( Scr_GetString( 0 ) );
}

/*
=============
ScrCmd_StopLoopSound
=============
*/
void ScrCmd_StopLoopSound( unsigned int entnum ) {
	GetEntity( entnum )->s.loopSound = 0;
}

/*
=============
ScrCmd_Delete
=============
*/
void ScrCmd_Delete( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );

	if ( ent->client ) {
		Scr_Error( "Cannot delete a client entity" );
	}

	Scr_AddEntityNum( ent->s.number, SCR_CLASS_ENTITY );
	Scr_NotifyNum( ent->s.number, SCR_CLASS_ENTITY, scr_const.death, 1 );

	trap_UnlinkEntity( ent );

	ent->use = NULL;
	ent->touch = NULL;
	ent->think = G_FreeEntity;
	ent->nextthink = level.time + 100;
}

/*
=============
ScrCmd_SetModel
=============
*/
void ScrCmd_SetModel( unsigned int entnum ) {
	gentity_t   *ent;
	const char  *modelName;

	ent = GetEntity( entnum );
	modelName = Scr_GetString( 0 );

	if ( modelName[0] ) {
		ent->model = G_ModelIndex( modelName );
	} else {
		ent->model = 0;
	}

	G_DObjUpdate( ent );
	trap_LinkEntity( ent );
}

/*
=============
ScrCmd_GetNormalHealth
=============
*/
void ScrCmd_GetNormalHealth( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );

	if ( ent->client ) {
		if ( ent->health ) {
			Scr_AddFloat( (float)( (double)ent->health / (double)ent->client->sess.maxHealth ) );
		} else {
			Scr_AddFloat( 0.0 );
		}
	} else {
		Scr_AddFloat( (float)ent->health );
	}
}

/*
=============
ScrCmd_SetNormalHealth
=============
*/
void ScrCmd_SetNormalHealth( unsigned int entnum ) {
	gentity_t   *ent;
	float frac;
	int health;

	ent = GetEntity( entnum );

	frac = Scr_GetFloat( 0 );
	if ( frac > 1.0f ) {
		frac = 1.0f;
	}

	if ( ent->client ) {
		health = (int)( (double)ent->client->sess.maxHealth * frac );
	} else if ( ent->maxHealth ) {
		health = (int)( (double)ent->maxHealth * frac );
	} else {
		health = (int)frac;
	}

	if ( health > 0 ) {
		ent->health = health;
	} else {
		Com_Printf( "ERROR: Cannot setnormalhealth to 0 or below.\n" );
	}
}

/*
=============
ScrCmd_Show
=============
*/
void ScrCmd_Show( unsigned int entnum ) {
	GetEntity( entnum )->flags &= ~FL_NOCLIENT;
}

/*
=============
ScrCmd_Hide
=============
*/
void ScrCmd_Hide( unsigned int entnum ) {
	GetEntity( entnum )->flags |= FL_NOCLIENT;
}

/*
=============
ScrCmd_SetContents
=============
*/
void ScrCmd_SetContents( unsigned int entnum ) {
	gentity_t   *ent;
	int contents;

	ent = GetEntity( entnum );
	contents = Scr_GetInt( 0 );

	Scr_AddInt( ent->r.contents );
	ent->r.contents = contents;

	trap_LinkEntity( ent );
}

/*
 * The two entityState_t words the cursor-hint pair lives in.  g_public.h names
 * +216 `scale` and +220 `dmgFlags`, and g_main_mp.c's G_CheckForCursorHints
 * already reads them under those names -- +216 is the hint string's index into
 * the CS_HINTSTRINGS block (255 for "none") and +220 the hint type.
 */

/* The retail search bound is 138, well past the end of predef_hintStrings[];
   the NULL slot at the end of the table is what actually stops the walk. */
#define HINT_SEARCH_MAX             138

#define HINT_INHERIT                ( -1 )
#define HINT_STRING_NONE            255

/* G_InitObjectives comes before them in the file. */
void ClearObjective_OnEntity( objective_t *objective );
void ClearObjective( objective_t *objective );

/*
=============
GScr_SetCursorHint
=============
*/
void GScr_SetCursorHint( unsigned int entnum ) {
	gentity_t   *ent;
	const char  *name;
	char        **s;
	int i;

	ent = GetEntity( entnum );
	name = Scr_GetString( 0 );

	if ( ent->classname == scr_const.trigger_use && !Q_strcasecmp( name, "HINT_INHERIT" ) ) {
		ent->s.dmgFlags = HINT_INHERIT;
		return;
	}

	for ( i = 1; i < HINT_SEARCH_MAX; i++ ) {
		if ( !predef_hintStrings[i] ) {
			break;
		}
		if ( !Q_strcasecmp( name, predef_hintStrings[i] ) ) {
			ent->s.dmgFlags = i;
			return;
		}
	}

	Com_Printf( "List of valid hint type strings\n" );
	if ( ent->classname == scr_const.trigger_use ) {
		Com_Printf( "HINT_INHERIT (for trigger_use entities only)\n" );
	}
	for ( s = &predef_hintStrings[1]; s < &predef_hintStrings[HINT_SEARCH_MAX]; s++ ) {
		if ( !*s ) {
			break;
		}
		Com_Printf( "%s\n", *s );
	}

	Scr_Error( va( "%s is not a valid hint type. See above for list of valid hint types\n", name ) );
}

/*
=============
G_GetHintStringIndex

Interns one hint string in the CS_HINTSTRINGS block, reusing a slot that
already holds it.  Returns qfalse (and -1) when the block is full.
=============
*/
qboolean G_GetHintStringIndex( int *index, const char *string ) {
	int i;
	char cs[MAX_STRING_CHARS];

	for ( i = 0; i < CS_HINTSTRINGS_COUNT; i++ ) {
		trap_GetConfigstring( CS_HINTSTRINGS + i, cs, sizeof( cs ) );

		if ( !cs[0] ) {
			trap_SetConfigstring( CS_HINTSTRINGS + i, string );
			*index = i;
			return qtrue;
		}

		if ( !strcmp( string, cs ) ) {
			*index = i;
			return qtrue;
		}
	}

	*index = -1;
	return qfalse;
}

/*
=============
GScr_SetHintString
=============
*/
void GScr_SetHintString( unsigned int entnum ) {
	gentity_t   *ent;
	char string[MAX_STRING_CHARS];
	int index;

	ent = GetEntity( entnum );

	if ( ent->classname != scr_const.trigger_use ) {
		Scr_Error( "The setHintString command only works on trigger_use entities.\n" );
	}

	if ( Scr_GetType( 0 ) == VAR_STRING && Scr_GetString( 0 ) && !Q_stricmp( Scr_GetString( 0 ), "" ) ) {
		ent->s.scale = HINT_STRING_NONE;
		return;
	}

	Scr_ConstructMessageString( MESSAGE_HINT_STRING, 0, string, sizeof( string ) );

	if ( !G_GetHintStringIndex( &index, string ) ) {
		Scr_Error( va( "Too many different hintstring values. Max allowed is %i different strings",
					   CS_HINTSTRINGS_COUNT ) );
	}

	ent->s.scale = index;
}

/*
=============
GScr_GetEntityNumber
=============
*/
void GScr_GetEntityNumber( unsigned int entnum ) {
	Scr_AddInt( GetEntity( entnum )->s.number );
}

/*
=============
GScr_EnableGrenadeTouchDamage
=============
*/
void GScr_EnableGrenadeTouchDamage( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );
	if ( ent->classname != scr_const.trigger_damage ) {
		Scr_Error( "Currently on supported on damage triggers" );
	}
	ent->flags |= FL_NO_GRENADE_TOUCH_DAMAGE;
}

/*
=============
GScr_DisableGrenadeTouchDamage
=============
*/
void GScr_DisableGrenadeTouchDamage( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetEntity( entnum );
	if ( ent->classname != scr_const.trigger_damage ) {
		Scr_Error( "Currently on supported on damage triggers" );
	}
	ent->flags &= ~FL_NO_GRENADE_TOUCH_DAMAGE;
}

/*
=============
GScr_EnableGrenadeBounce
=============
*/
void GScr_EnableGrenadeBounce( unsigned int entnum ) {
	GetEntity( entnum )->flags &= ~FL_NO_GRENADE_BOUNCE;
}

/*
=============
GScr_DisableGrenadeBounce
=============
*/
void GScr_DisableGrenadeBounce( unsigned int entnum ) {
	GetEntity( entnum )->flags |= FL_NO_GRENADE_BOUNCE;
}

/*
=============
G_InitObjectives

0x200313A0.  No retail symbol; name inferred from United Offensive, which has
the same 16-slot clear.
=============
*/
void G_InitObjectives( void ) {
	int i;

	for ( i = 0; i < MAX_OBJECTIVES; i++ ) {
		ClearObjective( &level.objectives[i] );
	}
}

/*
=============
ObjectiveStateIndexFromString

0x200313D0.  No retail symbol; name inferred from United Offensive.
=============
*/
qboolean ObjectiveStateIndexFromString( int *outState, unsigned short stateName ) {
	if ( stateName == scr_const.empty ) {
		*outState = OBJST_EMPTY;
	} else if ( stateName == scr_const.invisible ) {
		*outState = OBJST_INVISIBLE;
	} else if ( stateName == scr_const.current ) {
		*outState = OBJST_CURRENT;
	} else {
		*outState = OBJST_EMPTY;
		return qfalse;
	}

	return qtrue;
}

/*
=============
ClearObjective_OnEntity
=============
*/
void ClearObjective_OnEntity( objective_t *objective ) {
	gentity_t   *ent;

	if ( objective->entNum != ENTITYNUM_NONE ) {
		ent = &g_entities[objective->entNum];
		if ( ent->inuse ) {
			ent->r.svFlags &= ~SVF_OBJECTIVE;
		}
		objective->entNum = ENTITYNUM_NONE;
	}
}

/*
=============
ClearObjective
=============
*/
void ClearObjective( objective_t *objective ) {
	objective->state = OBJST_EMPTY;
	objective->origin[2] = 0.0;
	objective->origin[1] = 0.0;
	objective->origin[0] = 0.0;
	objective->entNum = ENTITYNUM_NONE;
	objective->teamNum = TEAM_FREE;
	objective->icon = 0;
}

/*
=============
SetObjectiveIcon
=============
*/
void SetObjectiveIcon( unsigned int paramIndex, objective_t *objective ) {
	const char  *name;
	int i;

	name = Scr_GetString( paramIndex );

	i = 0;
	if ( name[0] ) {
		do {
			if ( name[i] <= 31 || name[i] == 127 ) {
				Scr_ParamError( 3, va( "Illegal character '%c'(ascii %i) in objective icon name: %s\n",
									   name[i], (byte)name[i], name ) );
			}
		} while ( name[++i] );

		if ( i >= 64 ) {
			Scr_ParamError( 3, va( "Objective icon name is too long (> %i): %s\n", 63, name ) );
		}
	}

	objective->icon = G_ShaderIndex( name );
}

/*
=============
Scr_Objective_Add
=============
*/
void Scr_Objective_Add( void ) {
	int paramCount;
	int index;
	objective_t *objective;
	unsigned short stateName;
	int state;

	paramCount = Scr_GetNumParam();
	if ( paramCount < 2 ) {
		Scr_Error( "objective_add needs at least the first two parameters out of its parameter list of: index state [string] [position]\n" );
	}

	index = Scr_GetInt( 0 );
	if ( index < 0 || index >= MAX_OBJECTIVES ) {
		Scr_ParamError( 0, va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
							   index, MAX_OBJECTIVES - 1 ) );
	}
	objective = &level.objectives[index];

	ClearObjective_OnEntity( objective );

	stateName = Scr_GetConstString( 1 );
	if ( !ObjectiveStateIndexFromString( &state, stateName ) ) {
		Scr_ParamError( 1, va( "Illegal objective state \"%s\". Valid states are \"empty\", \"invisible\", \"current\"\n",
							   SL_ConvertToString( stateName ) ) );
	}
	objective->state = state;

	if ( paramCount >= 3 ) {
		Scr_GetVector( 2, objective->origin );
		objective->origin[0] = (float)(int)objective->origin[0];
		objective->origin[1] = (float)(int)objective->origin[1];
		objective->origin[2] = (float)(int)objective->origin[2];
		objective->entNum = ENTITYNUM_NONE;

		if ( paramCount >= 4 ) {
			SetObjectiveIcon( 3, objective );
		}
	}

	objective->teamNum = TEAM_FREE;
}

/*
=============
Scr_Objective_Delete
=============
*/
void Scr_Objective_Delete( void ) {
	int index;
	objective_t *objective;

	index = Scr_GetInt( 0 );
	if ( index < 0 || index >= MAX_OBJECTIVES ) {
		Scr_ParamError( 0, va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
							   index, MAX_OBJECTIVES - 1 ) );
	}
	objective = &level.objectives[index];

	ClearObjective_OnEntity( objective );
	ClearObjective( objective );
}

/*
=============
Scr_Objective_State
=============
*/
void Scr_Objective_State( void ) {
	int index;
	objective_t *objective;
	unsigned short stateName;
	int state;

	index = Scr_GetInt( 0 );
	if ( index < 0 || index >= MAX_OBJECTIVES ) {
		Scr_ParamError( 0, va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
							   index, MAX_OBJECTIVES - 1 ) );
	}
	objective = &level.objectives[index];

	stateName = Scr_GetConstString( 1 );
	if ( !ObjectiveStateIndexFromString( &state, stateName ) ) {
		Scr_ParamError( 1, va( "Illegal objective state \"%s\". Valid states are \"empty\", \"invisible\", \"current\"\n",
							   Scr_GetString( 1 ) ) );
	}
	objective->state = state;

	if ( state == OBJST_EMPTY || state == OBJST_INVISIBLE ) {
		ClearObjective_OnEntity( objective );
	}
}

/*
=============
Scr_Objective_Icon
=============
*/
void Scr_Objective_Icon( void ) {
	int index;

	index = Scr_GetInt( 0 );
	if ( index < 0 || index >= MAX_OBJECTIVES ) {
		Scr_ParamError( 0, va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
							   index, MAX_OBJECTIVES - 1 ) );
	}

	SetObjectiveIcon( 1, &level.objectives[index] );
}

/*
=============
Scr_Objective_Position
=============
*/
void Scr_Objective_Position( void ) {
	int index;
	objective_t *objective;

	index = Scr_GetInt( 0 );
	if ( index < 0 || index >= MAX_OBJECTIVES ) {
		Scr_ParamError( 0, va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
							   index, MAX_OBJECTIVES - 1 ) );
	}
	objective = &level.objectives[index];

	ClearObjective_OnEntity( objective );

	Scr_GetVector( 1, objective->origin );
	objective->origin[0] = (float)(int)objective->origin[0];
	objective->origin[1] = (float)(int)objective->origin[1];
	objective->origin[2] = (float)(int)objective->origin[2];
}

/*
=============
Scr_Objective_OnEntity
=============
*/
void Scr_Objective_OnEntity( void ) {
	int index;
	objective_t *objective;
	gentity_t   *ent;
	int classnum;
	unsigned int entnum;

	index = Scr_GetInt( 0 );
	if ( index < 0 || index >= MAX_OBJECTIVES ) {
		Scr_ParamError( 0, va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
							   index, MAX_OBJECTIVES - 1 ) );
	}
	objective = &level.objectives[index];

	ClearObjective_OnEntity( objective );

	entnum = Scr_GetEntityNum( 1, &classnum );
	if ( classnum || entnum >= MAX_GENTITIES ) {
		Scr_ParamError( 1, "not an entity" );
		ent = NULL;
	} else {
		ent = &g_entities[entnum];
	}

	ent->r.svFlags |= SVF_OBJECTIVE;
	objective->entNum = ent->s.number;
}

/*
=============
Scr_Objective_Current
=============
*/
void Scr_Objective_Current( void ) {
	int paramCount;
	int paramIndex;
	int index;
	int selected[MAX_OBJECTIVES];
	int i;

	paramCount = Scr_GetNumParam();
	memset( selected, 0, sizeof( selected ) );

	for ( paramIndex = 0; paramIndex < paramCount; paramIndex++ ) {
		index = Scr_GetInt( paramIndex );
		if ( index < 0 || index >= MAX_OBJECTIVES ) {
			Scr_ParamError( paramIndex,
							va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
								index, MAX_OBJECTIVES - 1 ) );
		}
		selected[index] = 1;
	}

	for ( i = 0; i < MAX_OBJECTIVES; i++ ) {
		if ( selected[i] ) {
			level.objectives[i].state = OBJST_CURRENT;
		} else if ( level.objectives[i].state == OBJST_CURRENT ) {
			level.objectives[i].state = OBJST_ACTIVE;
		}
	}
}

/*
=============
GScr_Objective_Team
=============
*/
void GScr_Objective_Team( void ) {
	int index;
	objective_t *objective;
	unsigned short teamName;

	index = Scr_GetInt( 0 );
	if ( index < 0 || index >= MAX_OBJECTIVES ) {
		Scr_ParamError( 0, va( "index %i is an illegal objective index. Valid indexes are 0 to %i\n",
							   index, MAX_OBJECTIVES - 1 ) );
	}
	objective = &level.objectives[index];

	teamName = Scr_GetConstString( 1 );
	if ( teamName == scr_const.allies ) {
		objective->teamNum = TEAM_ALLIES;
	} else if ( teamName == scr_const.axis ) {
		objective->teamNum = TEAM_AXIS;
	} else if ( teamName == scr_const.none ) {
		objective->teamNum = TEAM_FREE;
	} else {
		Scr_ParamError( 1, va( "Illegal team string '%s'. Must be allies, axis, or none.",
							   SL_ConvertToString( teamName ) ) );
	}
}

/*
=============
GScr_LogPrint
=============
*/
void GScr_LogPrint( void ) {
	char string[MAX_STRING_CHARS];
	const char  *token;
	int i;
	int paramCount;
	int len;
	int total;

	string[0] = 0;
	total = 0;
	paramCount = Scr_GetNumParam();

	for ( i = 0; i < paramCount; i++ ) {
		token = Scr_GetString( i );
		len = strlen( token );
		if ( total + len >= MAX_STRING_CHARS ) {
			break;
		}
		strcat( string, token );
		total += len;
	}

	G_LogPrintf( string );
}

/*
=============
GScr_WorldEntNumber
=============
*/
void GScr_WorldEntNumber( void ) {
	Scr_AddInt( ENTITYNUM_WORLD );
}

/*
=============
GScr_Obituary
=============
*/
void GScr_Obituary( void ) {
	const char  *weaponName;
	int weapon;
	int mod;
	gentity_t   *tent;
	gentity_t   *self;
	gentity_t   *attacker;
	int classnum;
	unsigned int entnum;

	weaponName = Scr_GetString( 2 );
	weapon = BG_GetWeaponIndexForName( weaponName );
	mod = G_IndexForMeansOfDeath( Scr_GetString( 3 ) );

	tent = G_TempEntity( vec3_origin, EV_OBITUARY );

	entnum = Scr_GetEntityNum( 0, &classnum );
	if ( classnum || entnum >= MAX_GENTITIES ) {
		Scr_ParamError( 0, "not an entity" );
		self = NULL;
	} else {
		self = &g_entities[entnum];
	}
	tent->s.otherEntityNum = self->s.number;

	if ( Scr_GetType( 1 ) == VAR_OBJECT && Scr_GetPointerType( 1 ) == VAR_ENTITY ) {
		attacker = Scr_GetEntity( 1 );
		tent->s.attackerEntityNum = attacker->s.number;
	} else {
		tent->s.attackerEntityNum = ENTITYNUM_WORLD;
	}

	tent->r.svFlags = SVF_BROADCAST;

	/* the splash / environmental means of death travel with 0x80 set instead
	   of a weapon index */
	if ( mod == MOD_MELEE || mod == MOD_HEAD_SHOT || mod == MOD_SUICIDE || mod == MOD_FALLING
		 || mod == MOD_CRUSH || mod == MOD_WATER || mod == MOD_SLIME ) {
		tent->s.eventParm = mod | 0x80;
	} else {
		tent->s.eventParm = weapon;
	}
}

/*
=============
GScr_positionWouldTelefrag
=============
*/
void GScr_positionWouldTelefrag( void ) {
	vec3_t origin;
	vec3_t mins;
	vec3_t maxs;
	int entityList[MAX_GENTITIES];
	int num;
	int i;
	gclient_t   *client;

	Scr_GetVector( 0, origin );

	mins[0] = playerMins[0] + origin[0];
	mins[1] = playerMins[1] + origin[1];
	mins[2] = playerMins[2] + origin[2];
	maxs[0] = playerMaxs[0] + origin[0];
	maxs[1] = playerMaxs[1] + origin[1];
	maxs[2] = playerMaxs[2] + origin[2];

	num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES, CONTENTS_BODY );
	if ( num <= 0 ) {
		Scr_AddInt( 0 );
		return;
	}

	for ( i = 0; i < num; i++ ) {
		client = g_entities[entityList[i]].client;
		if ( client && client->ps.pm_type < PM_DEAD ) {
			Scr_AddInt( 1 );
			return;
		}
	}

	Scr_AddInt( 0 );
}

/*
=============
GScr_getStartTime
=============
*/
void GScr_getStartTime( void ) {
	Scr_AddInt( level.startTime );
}

/*
=============
GScr_PrecacheMenu
=============
*/
void GScr_PrecacheMenu( void ) {
	const char  *name;
	char cs[MAX_STRING_CHARS];
	int i;

	name = Scr_GetString( 0 );

	for ( i = 0; i < CS_MENUS_COUNT; i++ ) {
		trap_GetConfigstring( CS_MENUS + i, cs, sizeof( cs ) );
		if ( !Q_stricmp( cs, name ) ) {
			Com_DPrintf( "Script tried to precache the menu '%s' more than once\n", name );
			return;
		}
	}

	for ( i = 0; i < CS_MENUS_COUNT; i++ ) {
		trap_GetConfigstring( CS_MENUS + i, cs, sizeof( cs ) );
		if ( !cs[0] ) {
			break;
		}
	}

	if ( i == CS_MENUS_COUNT ) {
		Scr_Error( va( "Too many menus precached. Max allowed menus is %i", CS_MENUS_COUNT ) );
	}

	trap_SetConfigstring( CS_MENUS + i, name );
}

/*
=============
GScr_GetScriptMenuIndex
=============
*/
int GScr_GetScriptMenuIndex( const char *name ) {
	char cs[MAX_STRING_CHARS];
	int i;

	for ( i = 0; i < CS_MENUS_COUNT; i++ ) {
		trap_GetConfigstring( CS_MENUS + i, cs, sizeof( cs ) );
		if ( !Q_stricmp( name, cs ) ) {
			return i;
		}
	}

	Scr_Error( va( "Menu '%s' was not precached\n", name ) );
	return 0;
}

/*
=============
GScr_PrecacheStatusIcon
=============
*/
void GScr_PrecacheStatusIcon( void ) {
	const char  *name;
	char cs[MAX_STRING_CHARS];
	int i;

	name = Scr_GetString( 0 );

	for ( i = 0; i < CS_STATUSICONS_COUNT; i++ ) {
		trap_GetConfigstring( CS_STATUSICONS + i, cs, sizeof( cs ) );
		if ( !Q_stricmp( cs, name ) ) {
			Com_DPrintf( "Script tried to precache the player status icon '%s' more than once\n", name );
			return;
		}
	}

	for ( i = 0; i < CS_STATUSICONS_COUNT; i++ ) {
		trap_GetConfigstring( CS_STATUSICONS + i, cs, sizeof( cs ) );
		if ( !cs[0] ) {
			break;
		}
	}

	if ( i == CS_STATUSICONS_COUNT ) {
		Scr_Error( va( "Too many player status icons precached. Max allowed is %i",
					   CS_STATUSICONS_COUNT ) );
	}

	trap_SetConfigstring( CS_STATUSICONS + i, name );
}

/*
=============
GScr_GetStatusIconIndex

The index is one-based: slot i lives in configstring CS_STATUSICONS + i, and
zero means "no icon".
=============
*/
int GScr_GetStatusIconIndex( const char *name ) {
	char cs[MAX_STRING_CHARS];
	int i;

	if ( !name[0] ) {
		return 0;
	}

	for ( i = 0; i < CS_STATUSICONS_COUNT; i++ ) {
		trap_GetConfigstring( CS_STATUSICONS + i, cs, sizeof( cs ) );
		if ( !Q_stricmp( name, cs ) ) {
			return i + 1;
		}
	}

	Scr_Error( va( "Status icon '%s' was not precached\n", name ) );
	return 0;
}

/*
=============
GScr_PrecacheHeadIcon
=============
*/
void GScr_PrecacheHeadIcon( void ) {
	const char  *name;
	char cs[MAX_STRING_CHARS];
	int i;

	name = Scr_GetString( 0 );

	for ( i = 0; i < CS_HEADICONS_COUNT; i++ ) {
		trap_GetConfigstring( CS_HEADICONS + i, cs, sizeof( cs ) );
		if ( !Q_stricmp( cs, name ) ) {
			Com_DPrintf( "Script tried to precache the player head icon '%s' more than once\n", name );
			return;
		}
	}

	for ( i = 0; i < CS_HEADICONS_COUNT; i++ ) {
		trap_GetConfigstring( CS_HEADICONS + i, cs, sizeof( cs ) );
		if ( !cs[0] ) {
			break;
		}
	}

	if ( i == CS_HEADICONS_COUNT ) {
		Scr_Error( va( "Too many player head icons precached. Max allowed is %i",
					   CS_HEADICONS_COUNT ) );
	}

	trap_SetConfigstring( CS_HEADICONS + i, name );
}

/*
=============
GScr_GetHeadIconIndex
=============
*/
int GScr_GetHeadIconIndex( const char *name ) {
	char cs[MAX_STRING_CHARS];
	int i;

	if ( !name[0] ) {
		return 0;
	}

	for ( i = 0; i < CS_HEADICONS_COUNT; i++ ) {
		trap_GetConfigstring( CS_HEADICONS + i, cs, sizeof( cs ) );
		if ( !Q_stricmp( name, cs ) ) {
			return i + 1;
		}
	}

	Scr_Error( va( "Head icon '%s' was not precached\n", name ) );
	return 0;
}

/*
=============
Scr_BulletTrace
=============
*/
void Scr_BulletTrace( void ) {
	vec3_t start;
	vec3_t end;
	vec3_t dir;
	trace_t trace;
	int passEntityNum;
	int contentmask;

	passEntityNum = ENTITYNUM_NONE;
	contentmask = MASK_BULLET_CHARACTERS;

	Scr_GetVector( 0, start );
	Scr_GetVector( 1, end );

	if ( !Scr_GetBool( 2 ) ) {
		contentmask = MASK_BULLET;
	}

	if ( Scr_GetType( 3 ) == VAR_OBJECT && Scr_GetPointerType( 3 ) == VAR_ENTITY ) {
		passEntityNum = Scr_GetEntity( 3 )->s.number;
	}

	trap_LocationalTrace( &trace, start, end, passEntityNum, contentmask, bulletPriorityMap );

	Scr_MakeArray();

	Scr_AddFloat( trace.fraction );
	Scr_AddArrayStringIndexed( scr_const.fraction );

	Scr_AddVector( trace.endpos );
	Scr_AddArrayStringIndexed( scr_const.position );

	if ( trace.entityNum == ENTITYNUM_NONE || trace.entityNum == ENTITYNUM_WORLD ) {
		Scr_AddUndefined();
	} else {
		Scr_AddEntityNum( g_entities[trace.entityNum].s.number, SCR_CLASS_ENTITY );
	}
	Scr_AddArrayStringIndexed( scr_const.entity );

	if ( trace.fraction < 1.0f ) {
		Scr_AddVector( trace.normal );
		Scr_AddArrayStringIndexed( scr_const.normal );

		Scr_AddString( trap_SurfaceTypeToName( ( trace.surfaceFlags >> 20 ) & 0x1F ) );
		Scr_AddArrayStringIndexed( scr_const.surfacetype );
	} else {
		dir[0] = end[0] - start[0];
		dir[1] = end[1] - start[1];
		dir[2] = end[2] - start[2];
		VectorNormalize( dir );

		Scr_AddVector( dir );
		Scr_AddArrayStringIndexed( scr_const.normal );

		Scr_AddConstString( scr_const.none );
		Scr_AddArrayStringIndexed( scr_const.surfacetype );
	}
}

/*
=============
Scr_RandomInt
=============
*/
void Scr_RandomInt( void ) {
	int range;

	range = Scr_GetInt( 0 );
	if ( range <= 0 ) {
		Com_Printf( "RandomInt parm: %d  ", range );
		Scr_Error( "RandomInt parm must be positive integer.\n" );
	}

	com_randSeed = 214013 * com_randSeed + 2531011;
	Scr_AddInt( ( range * ( (unsigned int)com_randSeed >> 17 ) ) >> 15 );
}

/*
=============
Scr_RandomFloat
=============
*/
void Scr_RandomFloat( void ) {
	float range;

	range = Scr_GetFloat( 0 );
	com_randSeed = 214013 * com_randSeed + 2531011;
	Scr_AddFloat( (float)( (unsigned int)com_randSeed >> 17 ) * range / 32768.0f );
}

/*
=============
Scr_RandomIntRange
=============
*/
void Scr_RandomIntRange( void ) {
	int min;
	int max;

	min = Scr_GetInt( 0 );
	max = Scr_GetInt( 1 );

	if ( max <= min ) {
		Com_Printf( "RandomIntRange parms: %d %d ", min, max );
		Scr_Error( "RandomIntRange range must be positive integer.\n" );
	}

	com_randSeed = 214013 * com_randSeed + 2531011;
	Scr_AddInt( min + ( ( ( max - min ) * ( (unsigned int)com_randSeed >> 17 ) ) >> 15 ) );
}

/*
=============
Scr_RandomFloatRange
=============
*/
void Scr_RandomFloatRange( void ) {
	float min;
	float max;

	min = Scr_GetFloat( 0 );
	max = Scr_GetFloat( 1 );

	if ( max <= min ) {
		Com_Printf( "Scr_RandomFloatRange parms: %d %d ", min );
		Scr_Error( "Scr_RandomFloatRange range must be positive float.\n" );
	}

	com_randSeed = 214013 * com_randSeed + 2531011;
	Scr_AddFloat( ( max - min ) * (float)( (unsigned int)com_randSeed >> 17 ) / 32768.0f + min );
}

/*
=============
GScr_sin
=============
*/
void GScr_sin( void ) {
	Scr_AddFloat( (float)sin( Scr_GetFloat( 0 ) * M_PI / 180.0f ) );
}

/*
=============
GScr_cos
=============
*/
void GScr_cos( void ) {
	Scr_AddFloat( (float)cos( Scr_GetFloat( 0 ) * M_PI / 180.0f ) );
}

/*
=============
GScr_tan
=============
*/
void GScr_tan( void ) {
	float angle;
	float s;
	float c;

	angle = Scr_GetFloat( 0 ) * M_PI / 180.0f;
	s = (float)sin( angle );
	c = (float)cos( angle );

	if ( c == 0.0 ) {
		Scr_Error( "divide by 0" );
	}

	Scr_AddFloat( s / c );
}

/*
=============
GScr_asin
=============
*/
void GScr_asin( void ) {
	float value;

	value = Scr_GetFloat( 0 );
	if ( value < -1.0 || value > 1.0 ) {
		Scr_Error( va( "%g out of range", value ) );
	}

	Scr_AddFloat( (float)asin( value ) * 180.0f / M_PI );
}

/*
=============
GScr_acos
=============
*/
void GScr_acos( void ) {
	float value;

	value = Scr_GetFloat( 0 );
	if ( value < -1.0 || value > 1.0 ) {
		Scr_Error( va( "%g out of range", value ) );
	}

	Scr_AddFloat( (float)acos( value ) * 180.0f / M_PI );
}

/*
=============
GScr_atan
=============
*/
void GScr_atan( void ) {
	Scr_AddFloat( (float)atan( Scr_GetFloat( 0 ) ) * 180.0f / M_PI );
}

/*
=============
Scr_Distance
=============
*/
void Scr_Distance( void ) {
	vec3_t a;
	vec3_t b;
	float dx;
	float dy;
	float dz;

	Scr_GetVector( 0, a );
	Scr_GetVector( 1, b );

	dx = b[0] - a[0];
	dy = b[1] - a[1];
	dz = b[2] - a[2];

	Scr_AddFloat( (float)sqrt( dz * dz + dy * dy + dx * dx ) );
}

/*
=============
Scr_DistanceSquared
=============
*/
void Scr_DistanceSquared( void ) {
	vec3_t a;
	vec3_t b;
	float dx;
	float dy;
	float dz;

	Scr_GetVector( 0, a );
	Scr_GetVector( 1, b );

	dx = b[0] - a[0];
	dy = b[1] - a[1];
	dz = b[2] - a[2];

	Scr_AddFloat( dz * dz + dy * dy + dx * dx );
}

/*
=============
Scr_Length
=============
*/
void Scr_Length( void ) {
	vec3_t v;

	Scr_GetVector( 0, v );
	Scr_AddFloat( (float)sqrt( v[2] * v[2] + v[1] * v[1] + v[0] * v[0] ) );
}

/*
=============
Scr_LengthSquared
=============
*/
void Scr_LengthSquared( void ) {
	vec3_t v;

	Scr_GetVector( 0, v );
	Scr_AddFloat( v[2] * v[2] + v[1] * v[1] + v[0] * v[0] );
}

/*
=============
Scr_Closer
=============
*/
void Scr_Closer( void ) {
	vec3_t org;
	vec3_t a;
	vec3_t b;
	float da;
	float db;
	float dx;
	float dy;
	float dz;

	Scr_GetVector( 0, org );
	Scr_GetVector( 1, a );
	Scr_GetVector( 2, b );

	dx = org[0] - a[0];
	dy = org[1] - a[1];
	dz = org[2] - a[2];
	da = dz * dz + dy * dy + dx * dx;

	dx = org[0] - b[0];
	dy = org[1] - b[1];
	dz = org[2] - b[2];
	db = dz * dz + dy * dy + dx * dx;

	if ( da >= db ) {
		Scr_AddInt( 0 );
	} else {
		Scr_AddInt( 1 );
	}
}

/*
=============
Scr_VectorDot
=============
*/
void Scr_VectorDot( void ) {
	vec3_t a;
	vec3_t b;

	Scr_GetVector( 0, a );
	Scr_GetVector( 1, b );

	Scr_AddFloat( b[2] * a[2] + b[1] * a[1] + b[0] * a[0] );
}

/*
=============
Scr_VectorNormalize
=============
*/
void Scr_VectorNormalize( void ) {
	vec3_t v;
	vec3_t out;

	Scr_GetVector( 0, v );

	out[0] = v[0];
	out[1] = v[1];
	out[2] = v[2];
	VectorNormalize( out );

	Scr_AddVector( out );
}

/*
=============
Scr_VectorToAngles
=============
*/
void Scr_VectorToAngles( void ) {
	vec3_t v;
	vec3_t angles;

	Scr_GetVector( 0, v );
	vectoangles( v, angles );
	Scr_AddVector( angles );
}

/*
=============
Scr_AnglesToUp
=============
*/
void Scr_AnglesToUp( void ) {
	vec3_t angles;
	vec3_t up;

	Scr_GetVector( 0, angles );
	AngleVectors( angles, NULL, NULL, up );
	Scr_AddVector( up );
}

/*
=============
Scr_AnglesToRight
=============
*/
void Scr_AnglesToRight( void ) {
	vec3_t angles;
	vec3_t right;
	float angle;
	float sr, sp, sy, cr, cp, cy;

	Scr_GetVector( 0, angles );

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	cy = (float)cos( angle );
	sy = (float)sin( angle );
	angle = angles[PITCH] * ( M_PI * 2 / 360 );
	cp = (float)cos( angle );
	sp = (float)sin( angle );
	angle = angles[ROLL] * ( M_PI * 2 / 360 );
	cr = (float)cos( angle );
	sr = (float)sin( angle );

	right[0] = -1 * sr * sp * cy + -1 * cr * -sy;
	right[1] = -1 * sr * sp * sy + -1 * cr * cy;
	right[2] = -1 * sr * cp;

	Scr_AddVector( right );
}

/*
=============
Scr_AnglesToForward
=============
*/
void Scr_AnglesToForward( void ) {
	vec3_t angles;
	vec3_t forward;
	float angle;
	float sp, sy, cp, cy;

	Scr_GetVector( 0, angles );

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	cy = (float)cos( angle );
	sy = (float)sin( angle );
	angle = angles[PITCH] * ( M_PI * 2 / 360 );
	cp = (float)cos( angle );
	sp = (float)sin( angle );

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;

	Scr_AddVector( forward );
}

/*
=============
Scr_MusicPlay
=============
*/
void Scr_MusicPlay( void ) {
	trap_SendServerCommand( -1, 0, va( "o %s", Scr_GetString( 0 ) ) );
}

/*
=============
Scr_MusicStop
=============
*/
void Scr_MusicStop( void ) {
	int numParam;
	int fadeTime;

	numParam = Scr_GetNumParam();
	if ( numParam ) {
		if ( numParam != 1 ) {
			Scr_Error( va( "USAGE: musicStop([fadetime]);\n" ) );
			return;
		}
		fadeTime = Q_ftol( Scr_GetFloat( 0 ) * 1000.0f );
		if ( fadeTime < 0 ) {
			Scr_Error( va( "musicStop: fade time must be >= 0\n" ) );
		}
	} else {
		fadeTime = 0;
	}

	trap_SendServerCommand( -1, 0, va( "p %i", fadeTime ) );
}

/*
=============
Scr_SoundFade
=============
*/
void Scr_SoundFade( void ) {
	float volume;
	int fadeTime;

	volume = Scr_GetFloat( 0 );
	if ( Scr_GetNumParam() > 1 ) {
		fadeTime = (int)( Scr_GetFloat( 1 ) * 1000.0f );
	} else {
		fadeTime = 0;
	}

	trap_SendServerCommand( -1, 0, va( "q %f %i\n", volume, fadeTime ) );
}

/*
=============
Scr_PrecacheModel
=============
*/
void Scr_PrecacheModel( void ) {
	if ( !level.spawning ) {
		Scr_Error( "precacheModel must be called before any wait statements in the gametype or level script\n" );
	}
	G_ModelIndex( Scr_GetString( 0 ) );
}

/*
=============
Scr_PrecacheShellShock
=============
*/
void Scr_PrecacheShellShock( void ) {
	if ( !level.spawning ) {
		Scr_Error( "precacheShellShock must be called before any wait statements in the gametype or level script\n" );
	}
	G_ShellShockIndex( Scr_GetString( 0 ) );
}

/*
=============
Scr_PrecacheItem
=============
*/
void Scr_PrecacheItem( void ) {
	const char  *name;
	gitem_t     *item;

	if ( !level.spawning ) {
		Scr_Error( "precacheItem must be called before any wait statements in the gametype or level script\n" );
	}

	name = Scr_GetString( 0 );
	item = BG_FindItem( name );
	if ( !item ) {
		Scr_ParamError( 0, va( "unknown item '%s'", name ) );
	}

	RegisterItem( item - bg_itemlist, qtrue );
}

/*
=============
Scr_PrecacheShader
=============
*/
void Scr_PrecacheShader( void ) {
	const char  *name;

	if ( !level.spawning ) {
		Scr_Error( "precacheShader must be called before any wait statements in the gametype or level script\n" );
	}

	name = Scr_GetString( 0 );
	if ( !name[0] ) {
		Scr_ParamError( 0, "Shader name string is empty" );
	}

	G_ShaderIndex( name );
}

/*
=============
Scr_PrecacheString
=============
*/
void Scr_PrecacheString( void ) {
	const char  *name;

	if ( !level.spawning ) {
		Scr_Error( "precacheString must be called before any wait statements in the gametype or level script\n" );
	}

	name = Scr_GetIString( 0 );
	if ( name[0] ) {
		G_LocalizedStringIndex( name );
	}
}

/*
=============
Scr_AmbientPlay
=============
*/
void Scr_AmbientPlay( void ) {
	int numParam;
	int fadeTime;
	const char  *alias;

	fadeTime = 0;
	numParam = Scr_GetNumParam();

	if ( numParam != 1 ) {
		if ( numParam != 2 ) {
			Scr_Error( va( "USAGE: ambientPlay(alias_name, <fadetime>);\n" ) );
			return;
		}
		fadeTime = Q_ftol( Scr_GetFloat( 1 ) * 1000.0f );
	}

	alias = Scr_GetString( 0 );
	if ( !alias[0] ) {
		Scr_Error( va( "ambientPlay: alias name cannot be the empty string... use stop or fade version\n" ) );
	}
	if ( fadeTime < 0 ) {
		Scr_Error( va( "ambientPlay: fade time must be >= 0\n" ) );
	}

	trap_SetConfigstring( CS_MUSIC, va( "n\\%s\\t\\%i", alias, level.time + fadeTime ) );
}

/*
=============
Scr_AmbientStop
=============
*/
void Scr_AmbientStop( void ) {
	int numParam;
	int fadeTime;

	numParam = Scr_GetNumParam();
	if ( numParam ) {
		if ( numParam != 1 ) {
			Scr_Error( va( "USAGE: ambientStop(<fadetime>);\n" ) );
			return;
		}
		fadeTime = Q_ftol( Scr_GetFloat( 0 ) * 1000.0f );
		if ( fadeTime < 0 ) {
			Scr_Error( va( "ambientStop: fade time must be >= 0\n" ) );
		}
	} else {
		fadeTime = 0;
	}

	trap_SetConfigstring( CS_MUSIC, va( "t\\%i", level.time + fadeTime ) );
}

/*
=============
Scr_GrenadeExplosionEffect
=============
*/
void Scr_GrenadeExplosionEffect( void ) {
	vec3_t origin;
	vec3_t up;
	vec3_t start;
	vec3_t end;
	trace_t trace;
	gentity_t   *tent;
	gentity_t   *ent;

	Scr_GetVector( 0, origin );
	origin[2] = origin[2] + 1.0f;

	tent = G_TempEntity( origin, EV_GRENADE_EXPLODE );

	up[0] = 0.0;
	up[1] = 0.0;
	up[2] = 1.0;
	tent->s.eventParm = DirToByte( up );

	start[0] = origin[0];
	start[1] = origin[1];
	start[2] = origin[2];
	end[0] = origin[0];
	end[1] = origin[1];
	end[2] = origin[2] - 17.0f;

	trap_Trace( &trace, start, vec3_origin, vec3_origin, end, ENTITYNUM_NONE, MASK_GRENADE_SURFACE );
	tent->s.surfType = ( trace.surfaceFlags >> 20 ) & 0x1F;

	ent = G_Spawn();
	ent->r.currentOrigin[0] = origin[0];
	ent->r.currentOrigin[1] = origin[1];
	ent->r.currentOrigin[2] = origin[2];
	ent->think = Concussive_think;
	ent->nextthink = level.time + 100;
	ent->delay = level.time + 500.0f;
}

/*
=============
GScr_RadiusDamage
=============
*/
void GScr_RadiusDamage( void ) {
	vec3_t origin;
	float radius;
	float damage;
	float minDamage;

	Scr_GetVector( 0, origin );
	radius = Scr_GetFloat( 1 );
	damage = Scr_GetFloat( 2 );
	minDamage = Scr_GetFloat( 3 );

	level.radiusDamageIgnorePlayers = level.scriptIgnoreRadiusDamage;
	G_RadiusDamage( origin, NULL, &g_entities[ENTITYNUM_WORLD], damage, minDamage, radius, NULL, MOD_EXPLOSIVE );
	level.radiusDamageIgnorePlayers = 0;
}

/*
=============
GScr_SetPlayerIgnoreRadiusDamage
=============
*/
void GScr_SetPlayerIgnoreRadiusDamage( void ) {
	level.scriptIgnoreRadiusDamage = Scr_GetBool( 0 );
}

/*
=============
GScr_GetMoveDelta
=============
*/
void GScr_GetMoveDelta( void ) {
	int numParam;
	float startTime;
	float endTime;
	scr_anim_t anim;
	vec3_t delta;
	float quat[2];

	numParam = Scr_GetNumParam();
	startTime = 0.0f;
	endTime = 1.0f;

	if ( numParam != 1 ) {
		if ( numParam != 2 ) {
			endTime = Scr_GetFloat( 2 );
			if ( endTime < 0.0f || endTime > 1.0f ) {
				Scr_ParamError( 2, "end time must be between 0 and 1" );
			}
		}
		startTime = Scr_GetFloat( 1 );
		if ( startTime < 0.0f || startTime > 1.0f ) {
			Scr_ParamError( 1, "start time must be between 0 and 1" );
		}
	}

	anim = Scr_GetAnim( 0, NULL );
	trap_XAnimGetRelDelta( anim, quat, delta, startTime, endTime );

	Scr_AddVector( delta );
}

/*
=============
GScr_GetAngleDelta
=============
*/
void GScr_GetAngleDelta( void ) {
	int numParam;
	float startTime;
	float endTime;
	scr_anim_t anim;
	vec3_t delta;
	float quat[2];

	numParam = Scr_GetNumParam();
	startTime = 0.0f;
	endTime = 1.0f;

	if ( numParam != 1 ) {
		if ( numParam != 2 ) {
			endTime = Scr_GetFloat( 2 );
			if ( endTime < 0.0f || endTime > 1.0f ) {
				Scr_ParamError( 2, "end time must be between 0 and 1" );
			}
		}
		startTime = Scr_GetFloat( 1 );
		if ( startTime < 0.0f || startTime > 1.0f ) {
			Scr_ParamError( 1, "start time must be between 0 and 1" );
		}
	}

	anim = Scr_GetAnim( 0, NULL );
	trap_XAnimGetRelDelta( anim, quat, delta, startTime, endTime );

	Scr_AddFloat( RotationToYaw( quat ) );
}

/*
=============
Scr_LoadFX
=============
*/
void Scr_LoadFX( void ) {
	int index;

	index = G_EffectIndex( Scr_GetString( 0 ) );
	if ( !index && !level.spawning ) {
		Scr_Error( "loadFx must be called before any wait statements in the gametype or level script, or on an already loaded effect\n" );
	}

	Scr_AddInt( index );
}

/*
=============
Scr_PlayFX
=============
*/
void Scr_PlayFX( void ) {
	int effectId;
	vec3_t origin;
	vec3_t forward;
	gentity_t   *tent;
	char cs[MAX_STRING_CHARS];

	if ( Scr_GetNumParam() < 2 || Scr_GetNumParam() > 3 ) {
		Scr_Error( "USAGE: playFx <effect id from loadFx> <vector position of effect> <optional forward vector>" );
	}

	effectId = Scr_GetInt( 0 );
	Scr_GetVector( 1, origin );

	if ( Scr_GetNumParam() == 3 ) {
		Scr_GetVector( 2, forward );
		if ( VectorNormalize( forward ) == 0.0f ) {
			if ( effectId ) {
				trap_GetConfigstring( CS_EFFECTS + effectId, cs, sizeof( cs ) );
			} else {
				strcpy( cs, "not successfully loaded" );
			}
			Scr_Error( va( "playFx called with (0 0 0) forward direction (effect = %s)\n", cs ) );
		}

		tent = G_TempEntity( origin, EV_PLAY_FX_DIR );
		tent->s.eventParm = (byte)effectId;
		tent->s.scale = DirToByte( forward );
	} else {
		G_TempEntity( origin, EV_PLAY_FX )->s.eventParm = (byte)effectId;
	}
}

/*
=============
Scr_PlayFXOnTag
=============
*/
void Scr_PlayFXOnTag( void ) {
	int effectId;
	gentity_t   *ent;
	const char  *tagName;
	int classnum;
	unsigned int entnum;

	if ( Scr_GetNumParam() != 3 ) {
		Scr_Error( "USAGE: playFxOnTag <effect id from loadFx> <entity> <tag name>" );
	}

	effectId = Scr_GetInt( 0 );
	if ( effectId <= 0 || effectId >= 64 ) {
		Scr_ParamError( 0, va( "effect id %i is invalid\n", effectId ) );
	}

	entnum = Scr_GetEntityNum( 1, &classnum );
	if ( classnum || entnum >= MAX_GENTITIES ) {
		Scr_ParamError( 1, "not an entity" );
		ent = NULL;
	} else {
		ent = &g_entities[entnum];
	}

	if ( !ent->model ) {
		Scr_ParamError( 1, "cannot play fx on entity with no model" );
	}

	tagName = Scr_GetString( 2 );
	if ( strchr( tagName, '"' ) ) {
		Scr_ParamError( 2, "cannot use \" characters in tag names\n" );
	}

	if ( trap_DObjGetBoneIndex( ent, tagName ) < 0 ) {
		trap_DObjDumpInfo( ent );
		Scr_ParamError( 2, va( "tag '%s' does not exist on entity with model '%s'",
							   tagName, trap_GetConfigstringConst( CS_MODELS + ent->model ) ) );
	}

	/* the effect and the tag travel to the client as one configstring */
	G_AddEvent( ent, EV_PLAY_FX_ON_TAG,
				G_FindConfigstringIndex( va( "%02d%s", effectId, tagName ),
										 CS_FX, CS_FX_COUNT, qtrue, NULL ) );
}

/*
=============
Scr_PlayLoopedFX
=============
*/
void Scr_PlayLoopedFX( void ) {
	int effectId;
	int numParam;
	int repeatDelay;
	float cullDist;
	vec3_t origin;
	vec3_t forward;
	gentity_t   *ent;
	char cs[MAX_STRING_CHARS];

	if ( Scr_GetNumParam() < 3 || Scr_GetNumParam() > 5 ) {
		Scr_Error( "USAGE: playLoopedFx <effect id from loadFx> <repeat delay> <vector position of effect> <optional cull distance (0 = never cull)> <optional forward vector>" );
	}

	forward[0] = 0.0;
	forward[1] = 0.0;
	forward[2] = 0.0;
	cullDist = 0.0;

	effectId = Scr_GetInt( 0 );

	numParam = Scr_GetNumParam();
	if ( numParam == 4 || numParam == 5 ) {
		if ( numParam == 5 ) {
			Scr_GetVector( 4, forward );
			if ( VectorNormalize( forward ) == 0.0f ) {
				if ( effectId ) {
					trap_GetConfigstring( CS_EFFECTS + effectId, cs, sizeof( cs ) );
				} else {
					strcpy( cs, "not successfully loaded" );
				}
				Scr_Error( va( "playLoopedFx called with (0 0 0) forward direction (effect = %s)\n", cs ) );
			}
		}
		cullDist = Scr_GetFloat( 3 );
	}

	Scr_GetVector( 2, origin );
	repeatDelay = Q_ftol( Scr_GetFloat( 1 ) * 1000.0f );

	ent = G_Spawn();
	ent->r.svFlags |= SVF_BROADCAST;
	ent->s.eType = ET_LOOP_FX;
	ent->s.scale = (byte)effectId;

	ent->s.pos.trBase[0] = origin[0];
	ent->s.pos.trBase[1] = origin[1];
	ent->s.pos.trBase[2] = origin[2];
	ent->s.pos.trType = 0;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	ent->s.pos.trDelta[0] = 0.0;
	ent->s.pos.trDelta[1] = 0.0;
	ent->s.pos.trDelta[2] = 0.0;

	ent->r.currentOrigin[0] = origin[0];
	ent->r.currentOrigin[1] = origin[1];
	ent->r.currentOrigin[2] = origin[2];

	ent->s.origin2[0] = forward[0];
	ent->s.origin2[1] = forward[1];
	ent->s.origin2[2] = forward[2];
	ent->s.angles2[0] = cullDist;
	ent->s.angles2[1] = (float)repeatDelay;

	trap_LinkEntity( ent );
	Scr_AddEntityNum( ent->s.number, SCR_CLASS_ENTITY );
}

/*
=============
Scr_SetFog
=============
*/
void Scr_SetFog( const char *cmdName, float nearDist, float farDist, float density,
				 float red, float green, float blue, float transitionTime ) {
	if ( nearDist < 0.0f ) {
		Scr_Error( va( "%s: near distance must be >= 0", cmdName ) );
	}
	if ( farDist <= nearDist ) {
		Scr_Error( va( "%s: near distance must be less than far distance", cmdName ) );
	}
	if ( red < 0.0f || red > 1.0f || green < 0.0f || green > 1.0f || blue < 0.0f || blue > 1.0f ) {
		Scr_Error( va( "%s: red/green/blue color components must be in the range [0, 1]", cmdName ) );
	}
	if ( transitionTime < 0.0f ) {
		Scr_Error( va( "%s: transition time must be >= 0 seconds", cmdName ) );
	}

	G_setfog( va( "%g %g %g %g %g %g %.0f", nearDist, farDist, density,
				  red, green, blue, transitionTime * 1000.0f ) );
}

/*
=============
Scr_SetLinearFog
=============
*/
void Scr_SetLinearFog( void ) {
	float nearDist;
	float farDist;
	float red;
	float green;
	float blue;
	float transitionTime;

	if ( Scr_GetNumParam() != 6 ) {
		Scr_Error( "USAGE: setCullFog(near distance, far distance, red, green, blue, transition time);\n" );
	}

	nearDist = Scr_GetFloat( 0 );
	farDist = Scr_GetFloat( 1 );
	red = Scr_GetFloat( 2 );
	green = Scr_GetFloat( 3 );
	blue = Scr_GetFloat( 4 );
	transitionTime = Scr_GetFloat( 5 );

	Scr_SetFog( "setCullFog", nearDist, farDist, 1.0, red, green, blue, transitionTime );
}

/*
=============
Scr_SetExponentialFog
=============
*/
void Scr_SetExponentialFog( void ) {
	float density;
	float red;
	float green;
	float blue;
	float transitionTime;

	if ( Scr_GetNumParam() != 5 ) {
		Scr_Error( "USAGE: setExpFog(density, red, green, blue, transition time);\n"
				   "Density must be greater than 0 and less than 1, and typically less than .001.  For example, .0002 means the fog gets .02%% more dense for every 1 unit of distance (about 1%% thicker every 50 units of distance)\n" );
	}

	density = Scr_GetFloat( 0 );
	red = Scr_GetFloat( 1 );
	green = Scr_GetFloat( 2 );
	blue = Scr_GetFloat( 3 );
	transitionTime = Scr_GetFloat( 4 );

	if ( density <= 0.0f || density >= 1.0f ) {
		Scr_Error( "setExpFog: distance must be greater than 0 and less than 1" );
	}

	Scr_SetFog( "setExpFog", 0.0, 1.0, density, red, green, blue, transitionTime );
}

/*
=============
GScr_IsPlayer
=============
*/
void GScr_IsPlayer( void ) {
	if ( Scr_GetType( 0 ) == VAR_OBJECT && Scr_GetPointerType( 0 ) == VAR_ENTITY
		 && Scr_GetEntity( 0 )->client ) {
		Scr_AddInt( 1 );
	} else {
		Scr_AddInt( 0 );
	}
}

/*
=============
GScr_IsPlayerNumber
=============
*/
void GScr_IsPlayerNumber( void ) {
	int num;

	num = Scr_GetInt( 0 );
	if ( num < 0 || num >= MAX_CLIENTS ) {
		Scr_AddInt( 0 );
	} else {
		Scr_AddInt( 1 );
	}
}

/*
=============
GScr_SetWinningPlayer

The winner travels in the CS_SCORES info string; a client number is stored
one-based so that 0 can mean "nobody".
=============
*/
void GScr_SetWinningPlayer( void ) {
	gentity_t   *ent;
	int classnum;
	unsigned int entnum;
	int winner;
	char cs[MAX_STRING_CHARS];
	const char  *value;
	const char  *old;

	entnum = Scr_GetEntityNum( 0, &classnum );
	if ( classnum || entnum >= MAX_GENTITIES ) {
		Scr_ParamError( 0, "not an entity" );
		ent = NULL;
	} else {
		ent = &g_entities[entnum];
	}

	winner = ent->s.number + 1;

	trap_GetConfigstring( CS_SCORES, cs, sizeof( cs ) );
	value = va( "%i", winner );
	old = Info_ValueForKey( cs, "winner" );

	if ( !old || !value || Q_stricmp( old, value ) ) {
		Info_SetValueForKey( cs, "winner", value );
		trap_SetConfigstring( CS_SCORES, cs );
	}
}

/*
=============
GScr_SetWinningTeam

-2 is allies, -1 is axis, 0 is a draw -- the negative range is what keeps the
team winners apart from the one-based client numbers.
=============
*/
void GScr_SetWinningTeam( void ) {
	unsigned short teamName;
	int winner;
	char cs[MAX_STRING_CHARS];
	const char  *value;
	const char  *old;

	teamName = Scr_GetConstString( 0 );

	if ( teamName == scr_const.allies ) {
		winner = -2;
	} else if ( teamName == scr_const.axis ) {
		winner = -1;
	} else if ( teamName == scr_const.none ) {
		winner = 0;
	} else {
		Scr_ParamError( 0, va( "Illegal team string '%s'. Must be allies, axis, or none.",
							   SL_ConvertToString( teamName ) ) );
		return;
	}

	trap_GetConfigstring( CS_SCORES, cs, sizeof( cs ) );
	value = va( "%i", winner );
	old = Info_ValueForKey( cs, "winner" );

	if ( !old || !value || Q_stricmp( old, value ) ) {
		Info_SetValueForKey( cs, "winner", value );
		trap_SetConfigstring( CS_SCORES, cs );
	}
}

/*
=============
GScr_Announcement
=============
*/
void GScr_Announcement( void ) {
	char string[MAX_STRING_CHARS];

	Scr_ConstructMessageString( MESSAGE_ANNOUNCEMENT, 0, string, sizeof( string ) );
	trap_SendServerCommand( -1, 0, va( "c \"%s\" 2", string ) );
}

/*
=============
GScr_ClientAnnouncement
=============
*/
void GScr_ClientAnnouncement( void ) {
	gentity_t   *ent;
	int classnum;
	unsigned int entnum;
	char string[MAX_STRING_CHARS];

	entnum = Scr_GetEntityNum( 0, &classnum );
	if ( classnum || entnum >= MAX_GENTITIES ) {
		Scr_ParamError( 0, "not an entity" );
		ent = NULL;
	} else {
		ent = &g_entities[entnum];
	}

	Scr_ConstructMessageString( MESSAGE_ANNOUNCEMENT, 1, string, sizeof( string ) );
	trap_SendServerCommand( ent->s.number, 0, va( "c \"%s\" 2", string ) );
}

/*
=============
GScr_GetTeamScore
=============
*/
void GScr_GetTeamScore( void ) {
	unsigned short teamName;

	teamName = Scr_GetConstString( 0 );

	if ( teamName == scr_const.allies ) {
		Scr_AddInt( level.teamScores[TEAM_ALLIES] );
		return;
	}

	if ( teamName != scr_const.axis ) {
		Scr_Error( va( "Illegal team string '%s'. Must be allies, or axis.",
					   SL_ConvertToString( teamName ) ) );
	}

	if ( teamName == scr_const.allies ) {
		Scr_AddInt( level.teamScores[TEAM_ALLIES] );
	} else {
		Scr_AddInt( level.teamScores[TEAM_AXIS] );
	}
}

/*
=============
GScr_SetTeamScore
=============
*/
void GScr_SetTeamScore( void ) {
	unsigned short teamName;
	int score;

	teamName = Scr_GetConstString( 0 );
	if ( teamName != scr_const.allies && teamName != scr_const.axis ) {
		Scr_Error( va( "Illegal team string '%s'. Must be allies, or axis.",
					   SL_ConvertToString( teamName ) ) );
	}

	score = Scr_GetInt( 1 );

	if ( teamName == scr_const.allies ) {
		level.teamScores[TEAM_ALLIES] = score;
		trap_SetConfigstring( CS_TEAMSCORE_ALLIES, va( "%i", score ) );
	} else {
		level.teamScores[TEAM_AXIS] = score;
		trap_SetConfigstring( CS_TEAMSCORE_AXIS, va( "%i", score ) );
	}

	level.scoreboardChanged = 1;
}

/*
=============
GScr_SetClientNameMode
=============
*/
void GScr_SetClientNameMode( void ) {
	unsigned short mode;

	mode = Scr_GetConstString( 0 );
	if ( mode == scr_const.auto_change ) {
		level.clientNameMode = CLIENTNAMEMODE_AUTO;
	} else if ( mode == scr_const.manual_change ) {
		level.clientNameMode = CLIENTNAMEMODE_MANUAL;
	} else {
		Scr_Error( "Unknown mode" );
	}
}

/*
=============
GScr_UpdateClientNames

Pushes the userinfo name every connected client is waiting on out to the
scoreboard.  `saved` is written and never read; retail does the same.
=============
*/
void GScr_UpdateClientNames( void ) {
	int i;
	gclient_t   *client;
	char saved[32];

	if ( !level.clientNameMode ) {
		Scr_Error( "Only works in [manual_change] mode" );
	}

	for ( i = 0; i < level.maxclients; i++ ) {
		client = &level.clients[i];

		if ( client->sess.connected != CON_CONNECTED ) {
			continue;
		}
		if ( !strcmp( client->sess.name, client->sess.netname ) ) {
			continue;
		}

		strncpy( saved, client->sess.name, sizeof( saved ) - 1 );
		saved[sizeof( saved ) - 1] = 0;

		strncpy( client->sess.name, client->sess.netname, sizeof( client->sess.name ) - 1 );
		client->sess.name[sizeof( client->sess.name ) - 1] = 0;

		ClientUserinfoChanged( i );
	}
}

/*
=============
GScr_GetTeamPlayersAlive
=============
*/
void GScr_GetTeamPlayersAlive( void ) {
	unsigned short teamName;
	team_t team;
	int count;
	int i;
	gentity_t   *ent;

	teamName = Scr_GetConstString( 0 );

	if ( teamName != scr_const.allies && teamName != scr_const.axis ) {
		Scr_Error( va( "Illegal team string '%s'. Must be allies, or axis.",
					   SL_ConvertToString( teamName ) ) );
	}

	if ( teamName == scr_const.allies ) {
		team = TEAM_ALLIES;
	} else {
		team = TEAM_AXIS;
	}

	count = 0;
	for ( i = 0; i < sv_maxclients.integer; i++ ) {
		ent = &g_entities[i];
		if ( ent->inuse && ent->client->sess.sessionTeam == team && ent->health > 0 ) {
			count++;
		}
	}

	Scr_AddInt( count );
}

/*
=============
GScr_GetNumParts
=============
*/
void GScr_GetNumParts( void ) {
	void        *model;

	model = trap_XModelGet( Scr_GetString( 0 ) );
	Scr_AddInt( trap_XModelNumBones( model ) );
}

/*
=============
GScr_GetPartName
=============
*/
void GScr_GetPartName( void ) {
	void        *model;
	unsigned int index;
	unsigned int numBones;
	unsigned short name;

	model = trap_XModelGet( Scr_GetString( 0 ) );
	index = Scr_GetInt( 1 );

	numBones = trap_XModelNumBones( model );
	if ( index >= numBones ) {
		Scr_ParamError( 1, va( "index out of range (0 - %d)", numBones - 1 ) );
	}

	name = trap_XModelGetBoneNames( model )[index];
	if ( !name ) {
		Scr_ParamError( 0, "bad model" );
	}

	Scr_AddConstString( name );
}

/*
=============
GScr_Earthquake
=============
*/
void GScr_Earthquake( void ) {
	float scale;
	int duration;
	vec3_t origin;
	float radius;
	gentity_t   *tent;

	scale = Scr_GetFloat( 0 );
	duration = Q_ftol( Scr_GetFloat( 1 ) * 1000.0f );
	Scr_GetVector( 2, origin );
	radius = Scr_GetFloat( 3 );

	tent = G_TempEntity( origin, EV_EARTHQUAKE );
	tent->s.angles2[0] = scale;
	tent->s.time = duration;
	tent->s.angles2[1] = radius;
}

/*
=============
GScr_ShellShock
=============
*/
void GScr_ShellShock( unsigned int entnum ) {
	gentity_t   *ent;
	const char  *name;
	int index;
	int duration;
	char cs[MAX_STRING_CHARS];

	ent = GetPlayerEntity( entnum );

	if ( Scr_GetNumParam() != 2 ) {
		Scr_Error( "USAGE: <player> shellshock(<shellshockname>, <duration>)\n" );
	}

	name = Scr_GetString( 0 );

	for ( index = 1; index < CS_SHELLSHOCKS_COUNT; index++ ) {
		trap_GetConfigstring( CS_SHELLSHOCKS + index, cs, sizeof( cs ) );
		if ( !_stricmp( cs, name ) ) {
			break;
		}
	}

	if ( index == CS_SHELLSHOCKS_COUNT ) {
		Scr_Error( va( "shellshock '%s' was not precached\n", name ) );
		return;
	}

	duration = Q_ftol( Scr_GetFloat( 1 ) * 1000.0f );
	if ( duration < 0 || duration > 60000 ) {
		Scr_ParamError( 1, va( "duration %g should be >= 0 and <= 60", duration * 0.001f ) );
	}

	ent->client->ps.shellshockIndex = index;
	ent->client->ps.shellshockTime = level.time;
	ent->client->ps.shellshockDuration = duration;
}

/*
=============
GScr_StopShellShock
=============
*/
void GScr_StopShellShock( unsigned int entnum ) {
	gentity_t   *ent;

	ent = GetPlayerEntity( entnum );

	if ( Scr_GetNumParam() ) {
		Scr_Error( "USAGE: <player> stopshellshock()\n" );
	}

	ent->client->ps.shellshockIndex = 0;
	ent->client->ps.shellshockTime = 0;
	ent->client->ps.shellshockDuration = 0;
}

/*
=============
GScr_ViewKick
=============
*/
void GScr_ViewKick( unsigned int entnum ) {
	gentity_t   *ent;
	vec3_t source;

	ent = GetPlayerEntity( entnum );

	if ( Scr_GetNumParam() != 2 ) {
		Scr_Error( "USAGE: <player> viewkick <force 0-127> <source position>\n" );
	}

	ent->client->damage_blood = ( ent->maxHealth * Scr_GetInt( 0 ) + 50 ) / 100;
	if ( ent->client->damage_blood < 0 ) {
		Scr_Error( va( "viewkick: damage %g < 0\n", Scr_GetFloat( 0 ) ) );
	}

	Scr_GetVector( 1, source );
	ent->client->damage_from[0] = ent->client->ps.origin[0] - source[0];
	ent->client->damage_from[1] = ent->client->ps.origin[1] - source[1];
	ent->client->damage_from[2] = ent->client->ps.origin[2] - source[2];
}

/*
=============
GScr_LocalToWorldCoords
=============
*/
void GScr_LocalToWorldCoords( unsigned int entnum ) {
	gentity_t   *ent;
	vec3_t local;
	vec3_t world;
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	vec3_t left;

	ent = GetEntity( entnum );

	Scr_GetVector( 0, local );
	AngleVectors( ent->r.currentAngles, forward, right, up );

	left[0] = 0.0f - right[0];
	left[1] = 0.0f - right[1];
	left[2] = 0.0f - right[2];

	world[0] = forward[0] * local[2] + up[0] * local[0] + local[1] * left[0];
	world[1] = forward[1] * local[2] + up[1] * local[0] + local[1] * left[1];
	world[2] = forward[2] * local[2] + up[2] * local[0] + local[1] * left[2];

	world[0] = world[0] + ent->r.currentOrigin[0];
	world[1] = world[1] + ent->r.currentOrigin[1];
	world[2] = world[2] + ent->r.currentOrigin[2];

	Scr_AddVector( world );
}

/*
=============
GScr_PlaceSpawnPoint

Drops the spawn point onto the floor: up to the ceiling, back down 256k units,
then a zero-length probe at the resting place to warn about a spawn in solid.
=============
*/
void GScr_PlaceSpawnPoint( unsigned int entnum ) {
	gentity_t   *ent;
	trace_t trace;
	vec3_t start;
	vec3_t end;

	ent = GetEntity( entnum );

	start[0] = ent->r.currentOrigin[0];
	start[1] = ent->r.currentOrigin[1];
	start[2] = ent->r.currentOrigin[2];
	end[0] = ent->r.currentOrigin[0];
	end[1] = ent->r.currentOrigin[1];
	end[2] = ent->r.currentOrigin[2] + 128.0f;
	trap_TraceCapsule( &trace, start, playerMins, playerMaxs, end, ent->s.number, MASK_SPAWNPOINT );

	start[0] = trace.endpos[0];
	start[1] = trace.endpos[1];
	start[2] = trace.endpos[2];
	end[0] = trace.endpos[0];
	end[1] = trace.endpos[1];
	end[2] = trace.endpos[2] - 262144.0f;
	trap_TraceCapsule( &trace, start, playerMins, playerMaxs, end, ent->s.number, MASK_SPAWNPOINT );

	ent->s.groundEntityNum = trace.entityNum;

	start[0] = trace.endpos[0];
	start[1] = trace.endpos[1];
	start[2] = trace.endpos[2];
	trap_TraceCapsule( &trace, start, playerMins, playerMaxs, start, ent->s.number, MASK_SPAWNPOINT );

	if ( trace.allsolid ) {
		Com_Printf( "WARNING: Spawn point entity %i is in solid at (%i, %i, %i)\n",
					ent->s.number, (int)ent->r.currentOrigin[0], (int)ent->r.currentOrigin[1],
					(int)ent->r.currentOrigin[2] );
	}

	ent->s.pos.trType = 0;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	ent->s.pos.trDelta[0] = 0.0;
	ent->s.pos.trDelta[1] = 0.0;
	ent->s.pos.trDelta[2] = 0.0;
	ent->s.pos.trBase[0] = trace.endpos[0];
	ent->s.pos.trBase[1] = trace.endpos[1];
	ent->s.pos.trBase[2] = trace.endpos[2];
	ent->r.currentOrigin[0] = trace.endpos[0];
	ent->r.currentOrigin[1] = trace.endpos[1];
	ent->r.currentOrigin[2] = trace.endpos[2];
}

/*
=============
GScr_MapRestart
=============
*/
void GScr_MapRestart( void ) {
	if ( level.exitRequest ) {
		Scr_Error( level.exitRequest == EXITREQUEST_MAP_RESTART
				   ? "map_restart already called" : "exitlevel already called" );
	}
	level.exitRequest = EXITREQUEST_MAP_RESTART;

	level.matchState = 0;
	if ( Scr_GetNumParam() ) {
		level.matchState = Scr_GetInt( 0 );
	}

	trap_SendConsoleCommand( EXEC_APPEND, "map_restart\n" );
}

/*
=============
GScr_ExitLevel
=============
*/
void GScr_ExitLevel( void ) {
	if ( level.exitRequest ) {
		Scr_Error( level.exitRequest == EXITREQUEST_MAP_RESTART
				   ? "map_restart already called" : "exitlevel already called" );
	}
	level.exitRequest = EXITREQUEST_EXITLEVEL;

	level.matchState = 0;
	if ( Scr_GetNumParam() ) {
		level.matchState = Scr_GetInt( 0 );
	}

	ExitLevel();
}

/*
=============
GScr_AddTestClient
=============
*/
void GScr_AddTestClient( void ) {
	int clientNum;
	gentity_t   *ent;

	clientNum = trap_AddTestClient();
	if ( clientNum < 0 ) {
		return;
	}

	ent = &level.gentities[clientNum];
	if ( ent ) {
		Scr_AddEntityNum( ent->s.number, SCR_CLASS_ENTITY );
	}
}

/*
=============
GScr_MakeCvarServerInfo
=============
*/
void GScr_MakeCvarServerInfo( void ) {
	const char  *name;
	const char  *value;
	char message[MAX_STRING_CHARS];
	char cleaned[MAX_STRING_CHARS];
	int i;
	char c;

	name = Scr_GetString( 0 );

	if ( Scr_GetType( 1 ) == VAR_LOCALIZED_STRING ) {
		Scr_ConstructMessageString( MESSAGE_CVAR_VALUE, 1, message, sizeof( message ) );
		value = message;
	} else {
		value = Scr_GetString( 1 );
	}

	/* The destination is 1024 bytes but the loop bound is 8192 -- retail:
	   GScr_SetCvar 0x2002FB0B and GScr_MakeCvarServerInfo 0x20034B69 both
	   walk 0x2000 into a 0x400 frame slot.  Reproduced as-is.  cleaned[] is
	   dead: retail (0x20034BA6 push eax) registers the original value, not
	   the scrubbed buffer. */
	for ( i = 0; i < 0x2000; i++ ) {
		c = value[i];
		if ( !c ) {
			break;
		}
		if ( c == (char)0x92 ) {
			c = '\'';
		} else if ( (byte)c > 0x7F ) {
			c = '.';
		}
		cleaned[i] = c;
		if ( c == '"' ) {
			cleaned[i] = '\'';
		}
	}

	trap_Cvar_Register( NULL, name, value, CVAR_WOLFINFO );
}

/*
=============
GScr_SetArchive
=============
*/
void GScr_SetArchive( void ) {
	trap_SetArchive( Scr_GetBool( 0 ) );
}

/*
 * ---------------------------------------------------------------------------
 * The builtin dispatch tables.
 *
 * A script function name that is not a script's own goes through
 * Scr_GetFunction; a method call goes through Scr_GetMethod, which asks the
 * player, script-entity and hudelem tables before falling back on
 * BuiltIn_GetMethod.  Both searches are linear and case-sensitive, and both
 * hand the table's own copy of the name back so the VM interns one string.
 * ---------------------------------------------------------------------------
 */

typedef struct scr_functionDef_s {
	const char      *name;
	xfunction_t call;
	int developer;
} scr_functionDef_t;

typedef struct scr_methodDef_s {
	const char      *name;
	xmethod_t call;
} scr_methodDef_t;

/*
 * Two entries need the address of a script-import wrapper.  In retail those
 * wrappers are ordinary functions -- the out-of-line bodies at 0x2003F650 and
 * 0x2003FA70 -- while the import block here is a run of pointers, so the two
 * address-taken slots get a thunk of their own.  Both thunk names are
 * inferred; retail had no separate function to name.
 */
static void GScr_spawnstruct( void ) {
	Scr_AddStruct();
}

static void GScr_resettimeout( void ) {
	Scr_ResetTimeout();
}

static scr_functionDef_t scr_functions[] = {
	{ "print",                          print,                              1 },
	{ "println",                        println,                            1 },
	{ "iprintln",                       iprintln,                           0 },
	{ "iprintlnbold",                   iprintlnbold,                       0 },
	{ "print3d",                        GScr_print3d,                       0 },
	{ "line",                           GScr_line,                          0 },
	{ "getent",                         Scr_getent,                         0 },
	{ "getentarray",                    Scr_GetEntArray,                    0 },
	{ "spawn",                          GScr_Spawn,                         0 },
	{ "spawnstruct",                    GScr_spawnstruct,                   0 },
	{ "assert",                         GScr_Assert,                        1 },
	{ "isdefined",                      GScr_IsDefined,                     0 },
	{ "isalive",                        GScr_IsAlive,                       0 },
	{ "getcvar",                        GScr_GetCvar,                       0 },
	{ "getcvarint",                     GScr_GetCvarInt,                    0 },
	{ "getcvarfloat",                   GScr_GetCvarFloat,                  0 },
	{ "setcvar",                        GScr_SetCvar,                       0 },
	{ "gettime",                        GScr_GetTime,                       0 },
	{ "getentbynum",                    Scr_GetEntByNum,                    0 },
	{ "getweaponmodel",                 Scr_GetWeaponModel,                 0 },
	{ "getweaponclassname",             Scr_GetWeaponClassname,             0 },
	{ "getanimlength",                  GScr_GetAnimLength,                 0 },
	{ "animhasnotetrack",               GScr_AnimHasNotetrack,              0 },
	{ "getbrushmodelcenter",            GScr_GetBrushModelCenter,           0 },
	{ "objective_add",                  Scr_Objective_Add,                  0 },
	{ "objective_delete",               Scr_Objective_Delete,               0 },
	{ "objective_state",                Scr_Objective_State,                0 },
	{ "objective_icon",                 Scr_Objective_Icon,                 0 },
	{ "objective_position",             Scr_Objective_Position,             0 },
	{ "objective_onentity",             Scr_Objective_OnEntity,             0 },
	{ "objective_current",              Scr_Objective_Current,              0 },
	{ "bullettrace",                    Scr_BulletTrace,                    0 },
	{ "getmovedelta",                   GScr_GetMoveDelta,                  0 },
	{ "getangledelta",                  GScr_GetAngleDelta,                 0 },
	{ "randomint",                      Scr_RandomInt,                      0 },
	{ "randomfloat",                    Scr_RandomFloat,                    0 },
	{ "randomintrange",                 Scr_RandomIntRange,                 0 },
	{ "randomfloatrange",               Scr_RandomFloatRange,               0 },
	{ "sin",                            GScr_sin,                           0 },
	{ "cos",                            GScr_cos,                           0 },
	{ "tan",                            GScr_tan,                           0 },
	{ "asin",                           GScr_asin,                          0 },
	{ "acos",                           GScr_acos,                          0 },
	{ "atan",                           GScr_atan,                          0 },
	{ "distance",                       Scr_Distance,                       0 },
	{ "distancesquared",                Scr_DistanceSquared,                0 },
	{ "length",                         Scr_Length,                         0 },
	{ "lengthsquared",                  Scr_LengthSquared,                  0 },
	{ "closer",                         Scr_Closer,                         0 },
	{ "vectordot",                      Scr_VectorDot,                      0 },
	{ "vectornormalize",                Scr_VectorNormalize,                0 },
	{ "vectortoangles",                 Scr_VectorToAngles,                 0 },
	{ "anglestoup",                     Scr_AnglesToUp,                     0 },
	{ "anglestoright",                  Scr_AnglesToRight,                  0 },
	{ "anglestoforward",                Scr_AnglesToForward,                0 },
	{ "musicplay",                      Scr_MusicPlay,                      0 },
	{ "musicstop",                      Scr_MusicStop,                      0 },
	{ "soundfade",                      Scr_SoundFade,                      0 },
	{ "ambientplay",                    Scr_AmbientPlay,                    0 },
	{ "ambientstop",                    Scr_AmbientStop,                    0 },
	{ "precachemodel",                  Scr_PrecacheModel,                  0 },
	{ "precacheshellshock",             Scr_PrecacheShellShock,             0 },
	{ "precacheitem",                   Scr_PrecacheItem,                   0 },
	{ "precacheshader",                 Scr_PrecacheShader,                 0 },
	{ "precachestring",                 Scr_PrecacheString,                 0 },
	{ "loadfx",                         Scr_LoadFX,                         0 },
	{ "playfx",                         Scr_PlayFX,                         0 },
	{ "playfxontag",                    Scr_PlayFXOnTag,                    0 },
	{ "playloopedfx",                   Scr_PlayLoopedFX,                   0 },
	{ "setcullfog",                     Scr_SetLinearFog,                   0 },
	{ "setexpfog",                      Scr_SetExponentialFog,              0 },
	{ "grenadeexplosioneffect",         Scr_GrenadeExplosionEffect,         0 },
	{ "radiusdamage",                   GScr_RadiusDamage,                  0 },
	{ "setplayerignoreradiusdamage",    GScr_SetPlayerIgnoreRadiusDamage,   0 },
	{ "getnumparts",                    GScr_GetNumParts,                   0 },
	{ "getpartname",                    GScr_GetPartName,                   0 },
	{ "earthquake",                     GScr_Earthquake,                    0 },
	{ "newhudelem",                     GScr_NewHudElem,                    0 },
	{ "newclienthudelem",               GScr_NewClientHudElem,              0 },
	{ "newteamhudelem",                 GScr_NewTeamHudElem,                0 },
	{ "resettimeout",                   GScr_resettimeout,                  0 },
	{ "isplayer",                       GScr_IsPlayer,                      0 },
	{ "isplayernumber",                 GScr_IsPlayerNumber,                0 },
	{ "setwinningplayer",               GScr_SetWinningPlayer,              0 },
	{ "setwinningteam",                 GScr_SetWinningTeam,                0 },
	{ "announcement",                   GScr_Announcement,                  0 },
	{ "clientannouncement",             GScr_ClientAnnouncement,            0 },
	{ "getteamscore",                   GScr_GetTeamScore,                  0 },
	{ "setteamscore",                   GScr_SetTeamScore,                  0 },
	{ "setclientnamemode",              GScr_SetClientNameMode,             0 },
	{ "updateclientnames",              GScr_UpdateClientNames,             0 },
	{ "getteamplayersalive",            GScr_GetTeamPlayersAlive,           0 },
	{ "objective_team",                 GScr_Objective_Team,                0 },
	{ "logprint",                       GScr_LogPrint,                      0 },
	{ "worldentnumber",                 GScr_WorldEntNumber,                0 },
	{ "obituary",                       GScr_Obituary,                      0 },
	{ "positionwouldtelefrag",          GScr_positionWouldTelefrag,         0 },
	{ "getstarttime",                   GScr_getStartTime,                  0 },
	{ "precachemenu",                   GScr_PrecacheMenu,                  0 },
	{ "precachestatusicon",             GScr_PrecacheStatusIcon,            0 },
	{ "precacheheadicon",               GScr_PrecacheHeadIcon,              0 },
	{ "map_restart",                    GScr_MapRestart,                    0 },
	{ "exitlevel",                      GScr_ExitLevel,                     0 },
	{ "addtestclient",                  GScr_AddTestClient,                 0 },
	{ "makecvarserverinfo",             GScr_MakeCvarServerInfo,            0 },
	{ "setarchive",                     GScr_SetArchive,                    0 }
};

static scr_methodDef_t scr_methods[] = {
	{ "attach",                         ScrCmd_attach },
	{ "detach",                         ScrCmd_detach },
	{ "detachall",                      ScrCmd_detachAll },
	{ "getattachsize",                  ScrCmd_GetAttachSize },
	{ "getattachmodelname",             ScrCmd_GetAttachModelName },
	{ "getattachtagname",               ScrCmd_GetAttachTagName },
	{ "linkto",                         ScrCmd_LinkTo },
	{ "unlink",                         ScrCmd_Unlink },
	{ "enablelinkto",                   ScrCmd_EnableLinkTo },
	{ "getorigin",                      ScrCmd_GetOrigin },
	{ "geteye",                         ScrCmd_GetEye },
	{ "useby",                          ScrCmd_UseBy },
	{ "istouching",                     ScrCmd_IsTouching },
	{ "lockdoor",                       ScrCmd_LockDoor },
	{ "unlockdoor",                     ScrCmd_UnlockDoor },
	{ "isdoorlocked",                   ScrCmd_IsDoorLocked },
	{ "playsound",                      ScrCmd_PlaySound },
	{ "playloopsound",                  ScrCmd_PlayLoopSound },
	{ "stoploopsound",                  ScrCmd_StopLoopSound },
	{ "delete",                         ScrCmd_Delete },
	{ "setmodel",                       ScrCmd_SetModel },
	{ "getnormalhealth",                ScrCmd_GetNormalHealth },
	{ "setnormalhealth",                ScrCmd_SetNormalHealth },
	{ "show",                           ScrCmd_Show },
	{ "hide",                           ScrCmd_Hide },
	{ "setcontents",                    ScrCmd_SetContents },
	{ "setcursorhint",                  GScr_SetCursorHint },
	{ "sethintstring",                  GScr_SetHintString },
	{ "shellshock",                     GScr_ShellShock },
	{ "stopshellshock",                 GScr_StopShellShock },
	{ "viewkick",                       GScr_ViewKick },
	{ "localtoworldcoords",             GScr_LocalToWorldCoords },
	{ "getentitynumber",                GScr_GetEntityNumber },
	{ "enablegrenadetouchdamage",       GScr_EnableGrenadeTouchDamage },
	{ "disablegrenadetouchdamage",      GScr_DisableGrenadeTouchDamage },
	{ "enablegrenadebounce",            GScr_EnableGrenadeBounce },
	{ "disablegrenadebounce",           GScr_DisableGrenadeBounce },
	{ "placespawnpoint",                GScr_PlaceSpawnPoint }
};

/*
=============
Scr_GetFunction
=============
*/
xfunction_t Scr_GetFunction( const char **pName, int *pDeveloper ) {
	int i;

	for ( i = 0; i < (int)( sizeof( scr_functions ) / sizeof( scr_functions[0] ) ); i++ ) {
		if ( !strcmp( *pName, scr_functions[i].name ) ) {
			*pName = scr_functions[i].name;
			*pDeveloper = scr_functions[i].developer;
			return scr_functions[i].call;
		}
	}

	return NULL;
}

/*
=============
BuiltIn_GetMethod
=============
*/
xmethod_t BuiltIn_GetMethod( const char **pName ) {
	int i;

	for ( i = 0; i < (int)( sizeof( scr_methods ) / sizeof( scr_methods[0] ) ); i++ ) {
		if ( !strcmp( *pName, scr_methods[i].name ) ) {
			*pName = scr_methods[i].name;
			return scr_methods[i].call;
		}
	}

	return NULL;
}

/*
=============
Scr_GetMethod
=============
*/
xmethod_t Scr_GetMethod( const char **pName, int *pDeveloper ) {
	xmethod_t method;

	*pDeveloper = 0;

	method = (xmethod_t)Player_GetMethod( pName );
	if ( method ) {
		return method;
	}

	method = (xmethod_t)ScriptEnt_GetMethod( pName, pDeveloper );
	if ( method ) {
		return method;
	}

	method = (xmethod_t)HudElem_GetMethod( pName, pDeveloper );
	if ( method ) {
		return method;
	}

	return BuiltIn_GetMethod( pName );
}

/*
=============
Scr_SetOrigin

fieldnum is unused: the table entry only exists so that writing "origin"
re-links the entity.
=============
*/
void Scr_SetOrigin( gentity_t *ent, int fieldnum ) {
	vec3_t origin;

	Scr_GetVector( 0, origin );

	ent->s.pos.trBase[0] = origin[0];
	ent->s.pos.trBase[1] = origin[1];
	ent->s.pos.trBase[2] = origin[2];
	ent->r.currentOrigin[0] = origin[0];
	ent->r.currentOrigin[1] = origin[1];
	ent->r.currentOrigin[2] = origin[2];

	ent->s.pos.trType = 0;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	ent->s.pos.trDelta[0] = 0.0;
	ent->s.pos.trDelta[1] = 0.0;
	ent->s.pos.trDelta[2] = 0.0;

	if ( ent->r.linked ) {
		trap_LinkEntity( ent );
	}
}

/*
=============
Scr_SetAngles
=============
*/
void Scr_SetAngles( gentity_t *ent, int fieldnum ) {
	vec3_t angles;

	Scr_GetVector( 0, angles );

	ent->s.apos.trBase[0] = angles[0];
	ent->s.apos.trBase[1] = angles[1];
	ent->s.apos.trBase[2] = angles[2];
	ent->r.currentAngles[0] = angles[0];
	ent->r.currentAngles[1] = angles[1];
	ent->r.currentAngles[2] = angles[2];

	ent->s.apos.trType = 0;
	ent->s.apos.trTime = 0;
	ent->s.apos.trDuration = 0;
	ent->s.apos.trDelta[0] = 0.0;
	ent->s.apos.trDelta[1] = 0.0;
	ent->s.apos.trDelta[2] = 0.0;
}

/*
=============
Scr_SetHealth
=============
*/
void Scr_SetHealth( gentity_t *ent, int fieldnum ) {
	int health;

	health = Scr_GetInt( 0 );
	ent->health = health;

	if ( ent->client ) {
		ent->client->sess.maxHealth = health;
	} else {
		ent->maxHealth = health;
	}
}

/*
=============
GScr_AddVector
=============
*/
void GScr_AddVector( const float *value ) {
	if ( value ) {
		Scr_AddVector( value );
	} else {
		Scr_AddUndefined();
	}
}

/*
=============
GScr_AddEntity
=============
*/
void GScr_AddEntity( gentity_t *ent ) {
	if ( ent ) {
		Scr_AddEntityNum( ent->s.number, SCR_CLASS_ENTITY );
	} else {
		Scr_AddUndefined();
	}
}

/*
=============
Scr_ParseGameTypeList

Builds the list the menus read from maps/mp/gametypes: one entry per .gsc that
does not start with an underscore, with the display name and the team flag
taken from the matching .txt.
=============
*/
void Scr_ParseGameTypeList( void ) {
	char fileList[4096];
	char buffer[MAX_STRING_CHARS];
	char        *text;
	const char  *token;
	const char  *name;
	char        *ext;
	scr_gameTypeEntry_t *entry;
	fileHandle_t f;
	int numFiles;
	int count;
	int i;
	int len;
	int fileLen;

	memset( g_scr_data.gameTypes, 0, sizeof( g_scr_data.gameTypes ) );

	count = 0;
	numFiles = trap_FS_GetFileList( "maps/mp/gametypes", "gsc", fileList, sizeof( fileList ) );
	name = fileList;

	if ( numFiles <= 0 ) {
		g_scr_data.gameTypeCount = 0;
		return;
	}

	entry = g_scr_data.gameTypes;

	for ( i = 0; i < numFiles; i++ ) {
		len = strlen( name );

		if ( name[0] == '_' ) {
			name += len + 1;
			continue;
		}

		ext = (char *)&name[len - 4];
		if ( !Q_stricmp( ".gsc", ext ) ) {
			*ext = 0;
		}

		if ( entry == (scr_gameTypeEntry_t *)g_scr_data.classMap ) {
			G_Printf( "Too many game type scripts found! Only loading the first %i\n",
					  MAX_GAMETYPES - 1 );
			break;
		}

		entry->name = trap_Hunk_AllocLowInternal( strlen( name ) );
		strcpy( entry->name, name );
		strlwr( entry->name );

		fileLen = trap_FS_FOpenFile( va( "maps/mp/gametypes/%s.txt", name ), &f, FS_READ );

		if ( fileLen <= 0 ) {
			Com_Printf( "WARNING: Could not load GameType description file %s for gametype %s\n",
						va( "maps/mp/gametypes/%s.txt", name ), name );
			entry->displayName = entry->name;
			entry->teamBased = 0;
		} else if ( fileLen >= MAX_STRING_CHARS ) {
			Com_Printf( "WARNING: GameType description file %s is too big to load.\n",
						va( "maps/mp/gametypes/%s.txt", name ) );
			entry->displayName = entry->name;
			entry->teamBased = 0;
		} else {
			memset( buffer, 0, sizeof( buffer ) );
			trap_FS_Read( buffer, fileLen, f );

			text = buffer;
			token = Com_Parse( &text );
			entry->displayName = trap_Hunk_AllocLowInternal( strlen( token ) );
			strcpy( entry->displayName, token );

			token = Com_Parse( &text );
			if ( token && !Q_stricmp( "team", token ) ) {
				entry->teamBased = 1;
			} else {
				entry->teamBased = 0;
			}
		}

		if ( fileLen > 0 ) {
			trap_FS_FCloseFile( f );
		}

		entry++;
		count++;
		name += len + 1;
	}

	g_scr_data.gameTypeCount = count;
}

/*
=============
Scr_IsValidGameType

Returns the game type's display name, which doubles as "this game type
exists".
=============
*/
const char *Scr_IsValidGameType( const char *gameType ) {
	int i;

	for ( i = 0; i < g_scr_data.gameTypeCount; i++ ) {
		if ( g_scr_data.gameTypes[i].name && gameType
			 && !Q_stricmp( gameType, g_scr_data.gameTypes[i].name ) ) {
			return g_scr_data.gameTypes[i].displayName;
		}
	}

	return NULL;
}

/*
=============
Scr_LoadGameType
=============
*/
void Scr_LoadGameType( void ) {
	Scr_FreeThread( Scr_ExecThread( g_scr_data.gameTypeMain, 0 ) );
}

/*
=============
Scr_StartupGameType
=============
*/
void Scr_StartupGameType( void ) {
	Scr_FreeThread( Scr_ExecThread( g_scr_data.gameTypeStartup, 0 ) );
}

/*
=============
Scr_PlayerConnect
=============
*/
void Scr_PlayerConnect( gentity_t *ent ) {
	Scr_FreeThread( Scr_ExecEntThreadNum( ent->s.number, SCR_CLASS_ENTITY,
										  g_scr_data.playerConnect, 0 ) );
}

/*
=============
Scr_PlayerDisconnect
=============
*/
void Scr_PlayerDisconnect( gentity_t *ent ) {
	Scr_FreeThread( Scr_ExecEntThreadNum( ent->s.number, SCR_CLASS_ENTITY,
										  g_scr_data.playerDisconnect, 0 ) );
}

/*
=============
Scr_PlayerDamage

CodeCallback_PlayerDamage( eInflictor, eAttacker, iDamage, iDFlags,
                           sMeansOfDeath, sWeapon, vPoint, vDir, sHitLoc )
=============
*/
void Scr_PlayerDamage( int hitLoc, const vec3_t point, int mod, const vec3_t dir,
					   gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
					   int damage, int dflags, int weapon ) {
	Scr_AddConstString( hitLocationConstStrings[hitLoc] );

	GScr_AddVector( dir );
	GScr_AddVector( point );

	Scr_AddString( bg_weaponInfo[weapon]->szInternalName );

	if ( mod >= 0 && mod < sizeof( modNames ) / sizeof( modNames[0] ) ) {
		Scr_AddString( modNames[mod] );
	} else {
		Scr_AddString( "badMOD" );
	}

	Scr_AddInt( dflags );
	Scr_AddInt( damage );

	GScr_AddEntity( attacker );
	GScr_AddEntity( inflictor );

	Scr_FreeThread( Scr_ExecEntThreadNum( self->s.number, SCR_CLASS_ENTITY,
										  g_scr_data.playerDamage, 9 ) );
}

/*
=============
Scr_PlayerKilled

CodeCallback_PlayerKilled( eInflictor, eAttacker, iDamage, sMeansOfDeath,
                           sWeapon, vDir, sHitLoc )
=============
*/
void Scr_PlayerKilled( gentity_t *self, gentity_t *inflictor, gentity_t *attacker,
					   int damage, int mod, int weapon, const float *dir, int hitLoc ) {
	Scr_AddConstString( hitLocationConstStrings[hitLoc] );

	GScr_AddVector( dir );

	Scr_AddString( bg_weaponInfo[weapon]->szInternalName );

	if ( mod >= 0 && mod < sizeof( modNames ) / sizeof( modNames[0] ) ) {
		Scr_AddString( modNames[mod] );
	} else {
		Scr_AddString( "badMOD" );
	}

	Scr_AddInt( damage );

	GScr_AddEntity( attacker );
	GScr_AddEntity( inflictor );

	Scr_FreeThread( Scr_ExecEntThreadNum( self->s.number, SCR_CLASS_ENTITY,
										  g_scr_data.playerKilled, 7 ) );
}

/*
=============
Scr_LoadRead

The game module keeps no script state of its own across a save, so the fifth
game callback always reads nothing.
=============
*/
int Scr_LoadRead( int len ) {
	return 0;
}

/*
=============
Scr_FarHook

0x2003FB40.  The DLL half of script/scr_import.c: it takes the engine's block
of 102 script entry points, drops it over the import pointers, and hands back
the five entry points the VM calls into the game with.

It lives here because it is what publishes the block every Scr_* call in the
module goes through.
=============
*/

/* 102 pointers -- Scr_GetBool through MT_Free. */
#define SCR_IMPORT_BLOCK_SIZE       0x198

typedef enum scrExport_e {
	SCR_EXP_GET_FUNCTION        = 0,
	SCR_EXP_GET_METHOD          = 1,
	SCR_EXP_SET_OBJECT_FIELD    = 2,
	SCR_EXP_GET_OBJECT_FIELD    = 3,
	SCR_EXP_LOAD_READ           = 4,

	SCR_EXP_COUNT               = 5
} scrExport_t;

void *Scr_FarHook( void *scrImports ) {
	if ( scrImports ) {
		memcpy( &Scr_GetBool, scrImports, SCR_IMPORT_BLOCK_SIZE );
	}

	scr_gameExports[SCR_EXP_GET_FUNCTION] = (void ( * )( void ))Scr_GetFunction;
	scr_gameExports[SCR_EXP_GET_METHOD] = (void ( * )( void ))Scr_GetMethod;
	scr_gameExports[SCR_EXP_SET_OBJECT_FIELD] = (void ( * )( void ))Scr_SetObjectField;
	scr_gameExports[SCR_EXP_GET_OBJECT_FIELD] = (void ( * )( void ))Scr_GetObjectField;
	scr_gameExports[SCR_EXP_LOAD_READ] = (void ( * )( void ))Scr_LoadRead;

	return scr_gameExports;
}
