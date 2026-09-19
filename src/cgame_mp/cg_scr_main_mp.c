/*
 * cg_scr_main_mp.c -- the client game module's half of the script system.
 * (no original srcfile recovered: see below)
 *
 * This is the cgame twin of game_mp/g_scr_main_mp.c, and it is the same code:
 * the script VM's exports arrive through Scr_FarHook as a block of 102
 * function pointers and five module callbacks go back the other way.  The
 * client has no script VM of its own -- no scr_const, no builtin tables, and
 * all five callbacks are empty -- so where the game module's file is 5000
 * lines this one is the import block and nothing else.
 *
 * THE NAME.  `Scr_FarHook` sits at 0x3003FDE0, in the gap between
 * universal/q_shared.c's last function (OrientationDirFromWorldDir
 * 0x3003F740+72) and ui/ui_shared.c's first (UI_Alloc 0x3003FE30), with no
 * recovered source file of its own.  The game module's twin is the
 * Mac-recovered `g_scr_main_mp.c` (already `_mp`-spelled), and the game
 * module's g_ units map onto cgame's cg_ units name for name (g_main_mp.c /
 * cg_main_mp.c, g_syscalls_mp.c / cg_syscalls_mp.c, g_local.h / cg_local.h);
 * cg_scr_main_mp.c is that mapping applied.
 *
 * ADDRESS RANGE 0x3003F790 .. 0x3003FE2F.  Only the tail of it is written
 * here.  0x3003F790..0x3003FDDC is a run of 101 tiny out-of-line thunks, one
 * per written import slot, each of the shape
 *
 *      push eax; [push ecx; push edx; ...]  call ds:<slot>;  add esp,N;  retn
 *
 * i.e. a __usercall adapter that takes the arguments in eax/ecx/edx (+stack)
 * and re-pushes them cdecl for the imported function.  Every call site inside
 * the DLL calls the SLOT directly (`call ds:Scr_FindAnim` at 0x3000509F and
 * four more in bg_animation.c), so the thunks are dead weight the linker kept
 * and none is written.  The game DLL has the identical run --
 * 0x2003F4F0..0x2003FB40, 0x650 bytes, exactly the size of cgame's
 * 0x3003F790..0x3003FDE0 -- and g_scr_main_mp.c omits them too.
 *
 * THE BLOCK IS THE GAME MODULE'S, SHIFTED BY 0x100A600.  Slot 0 game
 * 0x200A04F8 -> cgame 0x300AAAF8; Scr_FindAnimTree 0x200A0638 -> 0x300AAC38
 * and Scr_FindAnim 0x200A063C -> 0x300AAC3C, under the same names; the
 * five-entry export block below the table shifts the same way (0x200A04E0 ->
 * 0x300AAAE0).  The exe builds ONE block in Scr_NearHook and hands the same
 * pointer to both DLLs -- sv_game_mp.c's SV_InitGameVM and cl_cgame_mp.c's
 * CL_InitCGame both call `Scr_NearHook( NULL )` and pass the result straight
 * to the module's FAR_HOOK vmMain export.  The four slots the game module
 * calls Scr_Unused36/37/4B/4E (54, 55, 75, 78) are the four with no thunk and
 * no reference here either.
 *
 * @fidelity: likely
 */

#include <string.h>

#include "cg_local.h"

/*
 * ---------------------------------------------------------------------------
 * The script VM import block.
 *
 * Scr_FarHook memcpy's 0x198 bytes -- 102 pointers -- over the run of globals
 * starting at 0x300AAAF8, in the order the engine's Scr_NearHook fills them.
 * Names, order and prototypes are game_mp/g_local.h's for the same block; the
 * `[N]` in each comment is the argument count that slot's thunk pushes, and
 * `[jmp]` means the thunk is a bare tail jump.
 *
 * Slots this module never calls keep the game module's spelling, including
 * its empty argument lists where the game side could not recover one.  Six
 * slots have callers here -- Scr_BeginLoadAnimTrees, Scr_EndLoadAnimTrees and
 * Scr_PrecacheAnimTrees from cg_animtree_mp.c, Scr_FindAnim, Scr_FindAnimTree
 * and Scr_GetAnimsIndex from the shared bg_animation.c and cg_players_mp.c --
 * and those units declare them locally, at these same addresses.
 *
 * This is deliberately ONE object and not six: the memcpy above writes a
 * contiguous 0x198-byte run, so defining only the called pointers would give
 * them storage outside the block the engine fills.
 * ---------------------------------------------------------------------------
 */

/* The five module entry points Scr_FarHook hands back, immediately below the
   import block. */
static void ( *scr_cgameExports[5] )( void );                                           /* 0x300AAAE0 */

/* Script imports retain their retail slot order. Scr_FarHook copies each
 * independent pointer without assuming linker adjacency. */

int         ( *Scr_GetBool )( unsigned int num );                                       /* 0x300AAAF8 [1] */
int         ( *Scr_GetInt )( unsigned int num );                                        /* 0x300AAAFC [1] */
scr_anim_t  ( *Scr_GetAnim )( unsigned int num, void *anims );                          /* 0x300AAB00 [2] */
void        ( *Scr_GetAnimTree )();                                                     /* 0x300AAB04 [1] */
float       ( *Scr_GetFloat )( unsigned int num );                                      /* 0x300AAB08 [1] */
const char  *( *Scr_GetString )( unsigned int num );                                    /* 0x300AAB0C [1] */
unsigned short ( *Scr_GetConstString )( unsigned int num );                             /* 0x300AAB10 [1] */
const char  *( *Scr_GetDebugString )( unsigned int num );                               /* 0x300AAB14 [1] */
const char  *( *Scr_GetIString )( unsigned int num );                                   /* 0x300AAB18 [1] */
void        ( *Scr_GetConstIString )();                                                 /* 0x300AAB1C [1] */
void        ( *Scr_GetVector )( unsigned int num, vec3_t out );                         /* 0x300AAB20 [2] */
void        ( *Scr_GetFunc )();                                                         /* 0x300AAB24 [1] */
int         ( *Scr_GetType )( unsigned int num );                                       /* 0x300AAB28 [1] */
int         ( *Scr_GetPointerType )( unsigned int num );                                /* 0x300AAB2C [1] */
unsigned int ( *Scr_GetEntityNum )( unsigned int num, int *classnum );                  /* 0x300AAB30 [2] */
unsigned int ( *Scr_GetNumParam )( void );                                              /* 0x300AAB34 [jmp] */
void        ( *Scr_AddBool )( int value );                                              /* 0x300AAB38 [1] */
void        ( *Scr_AddInt )( int value );                                               /* 0x300AAB3C [1] */
void        ( *Scr_AddFloat )( float value );                                           /* 0x300AAB40 [jmp] */
void        ( *Scr_AddAnim )();                                                         /* 0x300AAB44 [jmp] */
void        ( *Scr_AddUndefined )( void );                                              /* 0x300AAB48 [jmp] */
void        ( *Scr_AddEntityNum )( int entnum, int classnum );                          /* 0x300AAB4C [2] */
void        ( *Scr_AddStruct )( void );                                                 /* 0x300AAB50 [jmp] */
void        ( *Scr_AddString )( const char *string );                                   /* 0x300AAB54 [1] */
void        ( *Scr_AddIString )();                                                      /* 0x300AAB58 [1] */
void        ( *Scr_AddConstString )( unsigned short id );                               /* 0x300AAB5C [1] */
void        ( *Scr_AddVector )( const float *value );                                   /* 0x300AAB60 [1] */
void        ( *Scr_AddObject )( unsigned short id );                                    /* 0x300AAB64 [1] */
void        ( *Scr_AddArray )( void );                                                  /* 0x300AAB68 [jmp] */
void        ( *Scr_AddArrayStringIndexed )( unsigned short id );                        /* 0x300AAB6C [1] */
void        ( *Scr_MakeArray )( void );                                                 /* 0x300AAB70 [jmp] */
void        ( *Scr_BeginLoadScripts )( int type );                                      /* 0x300AAB74 [jmp] */
void        ( *Scr_BeginLoadAnimTrees )( void );                                        /* 0x300AAB78 [jmp] */
void        ( *Scr_EndLoadScripts )( void );                                            /* 0x300AAB7C [jmp] */
void        ( *Scr_EndLoadAnimTrees )( void );                                          /* 0x300AAB80 [jmp] */
void        ( *Scr_PrecacheAnimTrees )( void *( *allocFn )( int size ) );               /* 0x300AAB84 [1] */
void        ( *Scr_FreeScripts )( int bComplete );                                      /* 0x300AAB88 [1] */
void        ( *Scr_FreeGameVariable )( int bComplete );                                 /* 0x300AAB8C [1] */
void        ( *Scr_ShutdownSystem )( int bComplete );                                   /* 0x300AAB90 [1] */
int         ( *Scr_IsSystemActive )( int bComplete );                                   /* 0x300AAB94 [1] */
void        ( *Scr_AddExecThread )();                                                   /* 0x300AAB98 [2] */
void        ( *Scr_AddExecEntThreadNum )( int entnum, int classnum, int handle, unsigned int paramcount );      /* 0x300AAB9C [4] */
int         ( *Scr_ExecThread )( unsigned int handle, unsigned int paramcount );        /* 0x300AABA0 [2] */
unsigned short ( *Scr_ExecEntThreadNum )( int entnum, int classnum, int handle, unsigned int paramcount );      /* 0x300AABA4 [4] */
void        ( *Scr_IsThreadAlive )();                                                   /* 0x300AABA8 [1] */
void        ( *Scr_Error )( const char *error );                                        /* 0x300AABAC [1] */
void        ( *Scr_ErrorWithDialogMessage )();                                          /* 0x300AABB0 [2] */
void        ( *Scr_ParamError )( unsigned int num, const char *error );                 /* 0x300AABB4 [2] */
void        ( *Scr_ObjectError )( const char *error );                                  /* 0x300AABB8 [1] */
void        ( *Scr_SetDynamicEntityField )( int entnum, int classnum, int fieldnum );   /* 0x300AABBC [3] */
void        ( *Scr_FreeEntityNum )( int entnum, int classnum );                         /* 0x300AABC0 [2] */
unsigned short ( *Scr_GetEntityId )( int entnum, int classnum );                        /* 0x300AABC4 [2] */
/* the game module types classMap `scr_classStruct_t *`; cgame has no such
   type and never calls the slot, so the pointer stays untyped here. */
void        ( *Scr_SetClassMap )( void *classMap, unsigned int count );                 /* 0x300AABC8 [2] */
void        ( *Scr_RemoveClassMap )( void );                                            /* 0x300AABCC [jmp] */
void        ( *Scr_Unused36 )();                                                        /* 0x300AABD0 -- never written */
void        ( *Scr_Unused37 )();                                                        /* 0x300AABD4 -- never written */
void        ( *Scr_AddClassField )( unsigned short classnum, const char *name, unsigned short fieldnum );       /* 0x300AABD8 [3] */
void        ( *Scr_AddFields )( const char *path, const char *extension );              /* 0x300AABDC [2] */
unsigned short ( *Scr_FindField )( const char *name, int *type );                       /* 0x300AABE0 [2] */
int         ( *Scr_GetOffset )( unsigned short classnum, const char *name );            /* 0x300AABE4 [2] */
void        ( *Scr_CopyEntityNum )( int sourceEntnum, int destEntnum, int classnum );   /* 0x300AABE8 [3] */
void        ( *Scr_Init )();                                                            /* 0x300AABEC [2] */
void        ( *Scr_Shutdown )();                                                        /* 0x300AABF0 [jmp] */
void        ( *Scr_Abort )();                                                           /* 0x300AABF4 [jmp] */
void        ( *Scr_SetLoading )( int loading );                                         /* 0x300AABF8 [1] */
void        ( *Scr_AllocGameVariable )( int classnum, int time );                       /* 0x300AABFC [2] */
void        ( *Scr_InitSystem )( void );                                                /* 0x300AAC00 [jmp] */
void        ( *Scr_GetChecksum )();                                                     /* 0x300AAC04 [1] */
void        ( *Scr_HasSourceFiles )();                                                  /* 0x300AAC08 [jmp] */
void        ( *Scr_SaveSource )();                                                      /* 0x300AAC0C [1] */
void        ( *Scr_LoadSource )();                                                      /* 0x300AAC10 [1] */
void        ( *Scr_SkipSource )();                                                      /* 0x300AAC14 [1] */
void        ( *Scr_SavePre )();                                                         /* 0x300AAC18 [1] */
void        ( *Scr_SavePost )();                                                        /* 0x300AAC1C [1] */
void        ( *Scr_SaveShutdown )();                                                    /* 0x300AAC20 [jmp] */
void        ( *Scr_Unused4B )();                                                        /* 0x300AAC24 -- never written */
void        ( *Scr_LoadPre )();                                                         /* 0x300AAC28 [2] */
void        ( *Scr_LoadShutdown )();                                                    /* 0x300AAC2C [jmp] */
void        ( *Scr_Unused4E )();                                                        /* 0x300AAC30 -- never written */
int         ( *Scr_LoadScript )( const char *scriptName );                              /* 0x300AAC34 [1] */
void        *( *Scr_FindAnimTree )( const char *treeName );                             /* 0x300AAC38 [1] */
void        ( *Scr_FindAnim )( const char *treeName, const char *animName, scr_anim_t *anim );  /* 0x300AAC3C [3] */
int         ( *Scr_GetFunctionHandle )( const char *scriptName, const char *label );    /* 0x300AAC40 [2] */
void        ( *Scr_FreeThread )( int threadId );                                        /* 0x300AAC44 [1] */
void        ( *Scr_ConvertThreadToSave )();                                             /* 0x300AAC48 [1] */
void        ( *Scr_ConvertThreadFromLoad )();                                           /* 0x300AAC4C [1] */
void        ( *Scr_SetString )( unsigned short *dest, unsigned short id );              /* 0x300AAC50 [2] */
unsigned short ( *Scr_AllocString )( const char *string, unsigned int user );           /* 0x300AAC54 [2] */
void        ( *Scr_NotifyNum )( int entnum, int classnum, unsigned short event, unsigned int paramcount );      /* 0x300AAC58 [4] */
void        ( *Scr_NotifyId )();                                                        /* 0x300AAC5C [3] */
const char  *( *SL_ConvertToString )( unsigned short id );                              /* 0x300AAC60 [1] */
unsigned short ( *SL_GetString )( const char *string, unsigned int user );              /* 0x300AAC64 [2] */
unsigned short ( *SL_GetLowercaseString )( const char *string, unsigned int user );     /* 0x300AAC68 [2] */
unsigned short ( *SL_FindLowercaseString )( const char *string );                       /* 0x300AAC6C [1] */
void        ( *Scr_CreateCanonicalFilename )();                                         /* 0x300AAC70 [1] */
void        ( *Scr_SetTime )( int time );                                               /* 0x300AAC74 [1] */
void        ( *Scr_RunCurrentThreads )( void );                                         /* 0x300AAC78 [jmp] */
void        ( *Scr_ResetTimeout )( void );                                              /* 0x300AAC7C [jmp] */
int         ( *Scr_GetAnimsIndex )( int anims );                                        /* 0x300AAC80 [1] */
void        ( *Scr_GetAnims )();                                                        /* 0x300AAC84 [1] */
void        *( *MT_Alloc )( int size, int type );                                       /* 0x300AAC88 [2] */
void        ( *MT_Free )( void *p, int size );                                          /* 0x300AAC8C [2] */


/*
 * ---------------------------------------------------------------------------
 * The five callbacks the module publishes back.
 *
 * All five are empty in cgame, and the engine never uses them: the game side
 * feeds the block it gets back into a second `Scr_NearHook( gameCallbacks )`
 * (sv_game_mp.c SV_InitGameVM), while CL_InitCGame throws cgame's return value
 * away.  Their bodies match the game module's five slot for slot -- the two
 * that return a dispatch-table entry return 0, the two field accessors are
 * void, and the save reader returns 0.
 *
 * They sit at 0x30013370..0x300133B0, immediately below cg_animtree.c's first
 * function (Hunk_AllocXAnimCreate 0x300133C0), so by address they belong to
 * that file rather than this one.  Written here because they exist only to
 * be published by Scr_FarHook, which is how game_mp/g_scr_main_mp.c places
 * its own five.  Names are that file's, for the same five VM slots.
 * ---------------------------------------------------------------------------
 */

/* 0x30013370 */
void *Scr_GetFunction( const char **pName, int *pDeveloper ) {
	return NULL;
}

/* 0x30013380 */
void *Scr_GetMethod( const char **pName, int *pDeveloper ) {
	return NULL;
}

/* 0x30013390 */
void Scr_SetObjectField( int classnum, int objectNum, int fieldnum ) {
}

/* 0x300133A0 */
void Scr_GetObjectField( int classnum, int objectNum, int fieldnum ) {
}

/* 0x300133B0 */
int Scr_LoadRead( int len ) {
	return 0;
}

/*
=============
Scr_FarHook

0x3003FDE0.  Takes the engine's block of 102 script entry points, drops it
over the import pointers, and hands back the five entry points the VM would
call into the client with.  vmMain's CG_SCRIPT_HOOK (16) is a straight tail
call to it.
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

/* cg_main_mp.c declares this `int *Scr_FarHook( const void * )` for vmMain's
   `return (int)Scr_FarHook( arg0 )`; the game module spells the same function
   `void *Scr_FarHook( void * )`. */
int *Scr_FarHook( const void *hooks ) {
	if ( hooks ) {
		/* Import globals need not be adjacent, especially with sanitizer redzones. */
		static void *const destinations[] = {
			&Scr_GetBool,
			&Scr_GetInt,
			&Scr_GetAnim,
			&Scr_GetAnimTree,
			&Scr_GetFloat,
			&Scr_GetString,
			&Scr_GetConstString,
			&Scr_GetDebugString,
			&Scr_GetIString,
			&Scr_GetConstIString,
			&Scr_GetVector,
			&Scr_GetFunc,
			&Scr_GetType,
			&Scr_GetPointerType,
			&Scr_GetEntityNum,
			&Scr_GetNumParam,
			&Scr_AddBool,
			&Scr_AddInt,
			&Scr_AddFloat,
			&Scr_AddAnim,
			&Scr_AddUndefined,
			&Scr_AddEntityNum,
			&Scr_AddStruct,
			&Scr_AddString,
			&Scr_AddIString,
			&Scr_AddConstString,
			&Scr_AddVector,
			&Scr_AddObject,
			&Scr_AddArray,
			&Scr_AddArrayStringIndexed,
			&Scr_MakeArray,
			&Scr_BeginLoadScripts,
			&Scr_BeginLoadAnimTrees,
			&Scr_EndLoadScripts,
			&Scr_EndLoadAnimTrees,
			&Scr_PrecacheAnimTrees,
			&Scr_FreeScripts,
			&Scr_FreeGameVariable,
			&Scr_ShutdownSystem,
			&Scr_IsSystemActive,
			&Scr_AddExecThread,
			&Scr_AddExecEntThreadNum,
			&Scr_ExecThread,
			&Scr_ExecEntThreadNum,
			&Scr_IsThreadAlive,
			&Scr_Error,
			&Scr_ErrorWithDialogMessage,
			&Scr_ParamError,
			&Scr_ObjectError,
			&Scr_SetDynamicEntityField,
			&Scr_FreeEntityNum,
			&Scr_GetEntityId,
			&Scr_SetClassMap,
			&Scr_RemoveClassMap,
			&Scr_Unused36,
			&Scr_Unused37,
			&Scr_AddClassField,
			&Scr_AddFields,
			&Scr_FindField,
			&Scr_GetOffset,
			&Scr_CopyEntityNum,
			&Scr_Init,
			&Scr_Shutdown,
			&Scr_Abort,
			&Scr_SetLoading,
			&Scr_AllocGameVariable,
			&Scr_InitSystem,
			&Scr_GetChecksum,
			&Scr_HasSourceFiles,
			&Scr_SaveSource,
			&Scr_LoadSource,
			&Scr_SkipSource,
			&Scr_SavePre,
			&Scr_SavePost,
			&Scr_SaveShutdown,
			&Scr_Unused4B,
			&Scr_LoadPre,
			&Scr_LoadShutdown,
			&Scr_Unused4E,
			&Scr_LoadScript,
			&Scr_FindAnimTree,
			&Scr_FindAnim,
			&Scr_GetFunctionHandle,
			&Scr_FreeThread,
			&Scr_ConvertThreadToSave,
			&Scr_ConvertThreadFromLoad,
			&Scr_SetString,
			&Scr_AllocString,
			&Scr_NotifyNum,
			&Scr_NotifyId,
			&SL_ConvertToString,
			&SL_GetString,
			&SL_GetLowercaseString,
			&SL_FindLowercaseString,
			&Scr_CreateCanonicalFilename,
			&Scr_SetTime,
			&Scr_RunCurrentThreads,
			&Scr_ResetTimeout,
			&Scr_GetAnimsIndex,
			&Scr_GetAnims,
			&MT_Alloc,
			&MT_Free,
		};
		unsigned int i;
		for ( i = 0; i < sizeof( destinations ) / sizeof( destinations[0] ); i++ ) {
			memcpy( destinations[i], (const char *)hooks + i * sizeof( void * ), sizeof( void * ) );
		}
	}

	scr_cgameExports[SCR_EXP_GET_FUNCTION] = (void ( * )( void ))Scr_GetFunction;
	scr_cgameExports[SCR_EXP_GET_METHOD] = (void ( * )( void ))Scr_GetMethod;
	scr_cgameExports[SCR_EXP_SET_OBJECT_FIELD] = (void ( * )( void ))Scr_SetObjectField;
	scr_cgameExports[SCR_EXP_GET_OBJECT_FIELD] = (void ( * )( void ))Scr_GetObjectField;
	scr_cgameExports[SCR_EXP_LOAD_READ] = (void ( * )( void ))Scr_LoadRead;

	return (int *)scr_cgameExports;
}
