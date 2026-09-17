/*
 * script/scr_import.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_import.c
 *
 * Retail range 0x0046CEA0-0x0046CEF0, 6 functions.
 *
 * @fidelity: verified
 */

#include "scr_local.h"
#include "../qcommon/hexrays_shim.h"

#define FindNextSibling     cod1_globals_FindNextSibling_data
#define FindObject          cod1_globals_FindObject_data
#include "../qcommon/cod1_globals.h"
#undef FindNextSibling
#undef FindObject

#define scrImport_GetFunction       scrImport_GetFunction    /* 0x008E5F04 */
#define scrImport_GetMethod         scrImport_GetMethod    /* 0x008E5F08 */
#define scrImport_SetObjectField    scrImport_SetObjectField    /* 0x008E5F0C */
#define scrImport_GetObjectField    scrImport_GetObjectField    /* 0x008E5F10 */
#define scrImport_LoadRead          scrImport_LoadRead    /* 0x008E5F14 */

#define SCR_IMPORT_TABLE_SIZE       0x14

enum {
	SCR_EXP_GetBool = 0,            /* 0x008E5F18 */
	SCR_EXP_GetInt,                 /* 0x008E5F1C */
	SCR_EXP_GetAnim,                /* 0x008E5F20 */
	SCR_EXP_GetAnimTree,            /* 0x008E5F24 */
	SCR_EXP_GetFloat,               /* 0x008E5F28 */
	SCR_EXP_GetString,              /* 0x008E5F2C */
	SCR_EXP_GetConstString,         /* 0x008E5F30 */
	SCR_EXP_GetDebugString,         /* 0x008E5F34 */
	SCR_EXP_GetIString,             /* 0x008E5F38 */
	SCR_EXP_GetConstIString,        /* 0x008E5F3C */
	SCR_EXP_GetVector,              /* 0x008E5F40 */
	SCR_EXP_GetFunc,                /* 0x008E5F44 */
	SCR_EXP_GetType,                /* 0x008E5F48 */
	SCR_EXP_GetPointerType,         /* 0x008E5F4C */
	SCR_EXP_GetEntityNum,           /* 0x008E5F50 */
	SCR_EXP_GetNumParam,            /* 0x008E5F54 */
	SCR_EXP_AddBool,                /* 0x008E5F58 */
	SCR_EXP_AddInt,                 /* 0x008E5F5C */
	SCR_EXP_AddFloat,               /* 0x008E5F60 */
	SCR_EXP_AddAnim,                /* 0x008E5F64 */
	SCR_EXP_AddUndefined,           /* 0x008E5F68 */
	SCR_EXP_AddEntityNum,           /* 0x008E5F6C */
	SCR_EXP_AddStruct,              /* 0x008E5F70 */
	SCR_EXP_AddString,              /* 0x008E5F74 */
	SCR_EXP_AddIString,             /* 0x008E5F78 */
	SCR_EXP_AddConstString,         /* 0x008E5F7C */
	SCR_EXP_AddVector,              /* 0x008E5F80 */
	SCR_EXP_AddObject,              /* 0x008E5F84 */
	SCR_EXP_AddArray,               /* 0x008E5F88 */
	SCR_EXP_AddArrayStringIndexed,  /* 0x008E5F8C */
	SCR_EXP_MakeArray,              /* 0x008E5F90 */
	SCR_EXP_BeginLoadScripts,       /* 0x008E5F94 */
	SCR_EXP_BeginLoadAnimTrees,     /* 0x008E5F98 */
	SCR_EXP_EndLoadScripts,         /* 0x008E5F9C */
	SCR_EXP_EndLoadAnimTrees,       /* 0x008E5FA0 */
	SCR_EXP_PrecacheAnimTrees,      /* 0x008E5FA4 */
	SCR_EXP_FreeScripts,            /* 0x008E5FA8 */
	SCR_EXP_FreeGameVariable,       /* 0x008E5FAC */
	SCR_EXP_ShutdownSystem,         /* 0x008E5FB0 */
	SCR_EXP_IsSystemActive,         /* 0x008E5FB4 */
	SCR_EXP_AddExecThread,          /* 0x008E5FB8 */
	SCR_EXP_AddExecEntThreadNum,    /* 0x008E5FBC */
	SCR_EXP_ExecThread,             /* 0x008E5FC0 */
	SCR_EXP_ExecEntThreadNum,       /* 0x008E5FC4 */
	SCR_EXP_IsThreadAlive,          /* 0x008E5FC8 */
	SCR_EXP_Error,                  /* 0x008E5FCC */
	SCR_EXP_ErrorWithDialogMessage, /* 0x008E5FD0 */
	SCR_EXP_ParamError,             /* 0x008E5FD4 */
	SCR_EXP_ObjectError,            /* 0x008E5FD8 */
	SCR_EXP_SetDynamicEntityField,  /* 0x008E5FDC */
	SCR_EXP_FreeEntityNum,          /* 0x008E5FE0 */
	SCR_EXP_GetEntityId,            /* 0x008E5FE4 */
	SCR_EXP_SetClassMap,            /* 0x008E5FE8 */
	SCR_EXP_RemoveClassMap,         /* 0x008E5FEC */
	SCR_EXP_UNUSED_36,              /* 0x008E5FF0  never assigned */
	SCR_EXP_UNUSED_37,              /* 0x008E5FF4  never assigned */
	SCR_EXP_AddClassField,          /* 0x008E5FF8 */
	SCR_EXP_AddFields,              /* 0x008E5FFC */
	SCR_EXP_FindField,              /* 0x008E6000 */
	SCR_EXP_GetOffset,              /* 0x008E6004 */
	SCR_EXP_CopyEntityNum,          /* 0x008E6008 */
	SCR_EXP_Init,                   /* 0x008E600C */
	SCR_EXP_Shutdown,               /* 0x008E6010 */
	SCR_EXP_Abort,                  /* 0x008E6014 */
	SCR_EXP_SetLoading,             /* 0x008E6018 */
	/* AllocGameVariable is index 65 and InitSystem index 66: game_mp_x86.dll's
	 * copy of this table (base 0x200A04F8) has J_Scr_AllocGameVariable
	 * (0x2003F810) calling slot 0x200A05FC and j_Scr_InitSystem (0x2003F800)
	 * calling slot 0x200A0600. */
	SCR_EXP_AllocGameVariable,      /* 0x008E601C */
	SCR_EXP_InitSystem,             /* 0x008E6020 */
	SCR_EXP_GetChecksum,            /* 0x008E6024 */
	SCR_EXP_HasSourceFiles,         /* 0x008E6028 */
	SCR_EXP_SaveSource,             /* 0x008E602C */
	SCR_EXP_LoadSource,             /* 0x008E6030 */
	SCR_EXP_SkipSource,             /* 0x008E6034 */
	SCR_EXP_SavePre,                /* 0x008E6038 */
	SCR_EXP_SavePost,               /* 0x008E603C */
	SCR_EXP_SaveShutdown,           /* 0x008E6040 */
	SCR_EXP_UNUSED_4B,              /* 0x008E6044  never assigned */
	SCR_EXP_LoadPre,                /* 0x008E6048 */
	SCR_EXP_LoadShutdown,           /* 0x008E604C */
	SCR_EXP_UNUSED_4E,              /* 0x008E6050  never assigned */
	SCR_EXP_LoadScript,             /* 0x008E6054 */
	SCR_EXP_FindAnimTree,           /* 0x008E6058 */
	SCR_EXP_FindAnim,               /* 0x008E605C */
	SCR_EXP_GetFunctionHandle,      /* 0x008E6060 */
	SCR_EXP_FreeThread,             /* 0x008E6064 */
	SCR_EXP_ConvertThreadToSave,    /* 0x008E6068 */
	SCR_EXP_ConvertThreadFromLoad,  /* 0x008E606C */
	SCR_EXP_SetString,              /* 0x008E6070 */
	SCR_EXP_AllocString,            /* 0x008E6074 */
	SCR_EXP_NotifyNum,              /* 0x008E6078 */
	SCR_EXP_NotifyId,               /* 0x008E607C */
	SCR_EXP_SL_ConvertToString,     /* 0x008E6080 */
	SCR_EXP_SL_GetString,           /* 0x008E6084 */
	SCR_EXP_SL_GetLowercaseString,  /* 0x008E6088 */
	SCR_EXP_SL_FindLowercaseString, /* 0x008E608C */
	SCR_EXP_CreateCanonicalFilename,/* 0x008E6090 */
	SCR_EXP_SetTime,                /* 0x008E6094 */
	SCR_EXP_RunCurrentThreads,      /* 0x008E6098 */
	SCR_EXP_ResetTimeout,           /* 0x008E609C */
	SCR_EXP_GetAnimsIndex,          /* 0x008E60A0 */
	SCR_EXP_GetAnims,               /* 0x008E60A4 */
	SCR_EXP_MT_Alloc,               /* 0x008E60A8 */
	SCR_EXP_MT_Free,                /* 0x008E60AC */

	SCR_EXP_COUNT
};

typedef void ( *scrExportFn_t )( void );

static scrExportFn_t scrExports[SCR_EXP_COUNT];

extern int Scr_Abort();
extern int Scr_AddAnim();
extern int Scr_AddArray();
extern int Scr_AddArrayStringIndexed();
extern int Scr_AddBool();
extern int Scr_AddConstString();
extern int Scr_AddEntityNum();
extern int Scr_AddExecEntThreadNum();
extern int Scr_AddExecThread();
extern int Scr_AddFloat();
extern int Scr_AddIString();
extern int Scr_AddInt();
extern int Scr_AddObject();
extern int Scr_AddString();
extern int Scr_AddStruct();
extern int Scr_AddUndefined();
extern int Scr_AddVector();
extern int Scr_BeginLoadAnimTrees();
extern int Scr_BeginLoadScripts();
extern unsigned short Scr_ConvertThreadFromLoad( unsigned short saveId );
extern int Scr_ConvertThreadToSave();
extern int Scr_CopyEntityNum();
extern int Scr_EndLoadAnimTrees();
extern int Scr_EndLoadScripts();
extern int Scr_ErrorWithDialogMessage();
extern int Scr_ExecEntThreadNum();
extern int Scr_ExecThread();
extern int Scr_FindAnim();
extern int Scr_FindAnimTree();
extern int Scr_FreeScripts();
extern int Scr_FreeThread();
extern int Scr_GetAnim();
extern int Scr_GetAnimTree();
extern int Scr_GetAnims();
extern int Scr_GetAnimsIndex();
extern int Scr_GetBool();
extern int Scr_GetChecksum();
extern int Scr_GetConstIString();
extern int Scr_GetConstString();
extern int Scr_GetDebugString();
extern int Scr_GetEntityNum();
extern int Scr_GetFloat();
extern int Scr_GetFunc();
extern int Scr_GetFunctionHandle();
extern int Scr_GetIString();
extern int Scr_GetInt();
extern int Scr_GetNumParam();
extern int Scr_GetPointerType();
extern int Scr_GetString();
extern int Scr_GetType();
extern int Scr_GetVector();
extern int Scr_HasSourceFiles();
extern int Scr_InitSystem();
extern int Scr_IsSystemActive();
extern void Scr_LoadPre( void );
extern int Scr_LoadScript();
extern void Scr_LoadShutdown( void );
extern int Scr_LoadSource();
extern int Scr_MakeArray();
extern int Scr_NotifyId();
extern int Scr_NotifyNum();
extern int Scr_ObjectError();
extern int Scr_ParamError();
extern int Scr_PrecacheAnimTrees();
extern int Scr_ResetTimeout();
extern int Scr_RunCurrentThreads();
extern void Scr_SavePost( void );
extern void Scr_SavePre( void );
extern int Scr_SaveShutdown();
extern int Scr_SaveSource();
extern int Scr_SetDynamicEntityField();
extern int Scr_SetLoading();
extern int Scr_SetTime();
extern int Scr_ShutdownSystem();
extern int Scr_SkipSource();

/* ---- Scr_GetFunction  0x0046CEA0 ---- */
void *Scr_GetFunction( const char **name, int *developerOnly ) {
	return (void *)scrImport_GetFunction( name, developerOnly );
}

/* ---- Scr_GetMethod  0x0046CEB0 ---- */
void *Scr_GetMethod( const char **name, int *developerOnly ) {
	return (void *)scrImport_GetMethod( name, developerOnly );
}

/* ---- Scr_SetObjectField  0x0046CEC0 ---- */
void Scr_SetObjectField( int classnum, int entnum, int offset ) {
	scrImport_SetObjectField( classnum, entnum, offset );
}

/* ---- Scr_GetObjectField  0x0046CED0 ---- */
void Scr_GetObjectField( int classnum, int entnum, int offset ) {
	scrImport_GetObjectField( classnum, entnum, offset );
}

/* ---- Scr_LoadRead  0x0046CEE0 ---- */
int Scr_LoadRead( int len ) {
	return scrImport_LoadRead( len );
}

/* ---- Scr_NearHook  0x0046CEF0 ---- */
void *Scr_NearHook( const void *gameCallbacks ) {
	if ( gameCallbacks ) {
		int ( **src )() = (int ( ** )()) gameCallbacks;

		scrImport_GetFunction    = src[0];
		scrImport_GetMethod      = src[1];
		scrImport_SetObjectField = src[2];
		scrImport_GetObjectField = src[3];
		scrImport_LoadRead       = src[4];
	}

	scrExports[SCR_EXP_GetBool]                 = (scrExportFn_t)Scr_GetBool;
	scrExports[SCR_EXP_GetInt]                  = (scrExportFn_t)Scr_GetInt;
	scrExports[SCR_EXP_GetAnim]                 = (scrExportFn_t)Scr_GetAnim;
	scrExports[SCR_EXP_GetAnimTree]             = (scrExportFn_t)Scr_GetAnimTree;
	scrExports[SCR_EXP_GetFloat]                = (scrExportFn_t)Scr_GetFloat;
	scrExports[SCR_EXP_GetString]               = (scrExportFn_t)Scr_GetString;
	scrExports[SCR_EXP_GetConstString]          = (scrExportFn_t)Scr_GetConstString;
	scrExports[SCR_EXP_GetDebugString]          = (scrExportFn_t)Scr_GetDebugString;
	scrExports[SCR_EXP_GetIString]              = (scrExportFn_t)Scr_GetIString;
	scrExports[SCR_EXP_GetConstIString]         = (scrExportFn_t)Scr_GetConstIString;
	scrExports[SCR_EXP_GetVector]               = (scrExportFn_t)Scr_GetVector;
	scrExports[SCR_EXP_GetFunc]                 = (scrExportFn_t)Scr_GetFunc;
	scrExports[SCR_EXP_GetType]                 = (scrExportFn_t)Scr_GetType;
	scrExports[SCR_EXP_GetPointerType]          = (scrExportFn_t)Scr_GetPointerType;
	scrExports[SCR_EXP_GetEntityNum]            = (scrExportFn_t)Scr_GetEntityNum;
	scrExports[SCR_EXP_GetNumParam]             = (scrExportFn_t)Scr_GetNumParam;

	scrExports[SCR_EXP_AddBool]                 = (scrExportFn_t)Scr_AddBool;
	scrExports[SCR_EXP_AddInt]                  = (scrExportFn_t)Scr_AddInt;
	scrExports[SCR_EXP_AddFloat]                = (scrExportFn_t)Scr_AddFloat;
	scrExports[SCR_EXP_AddAnim]                 = (scrExportFn_t)Scr_AddAnim;
	scrExports[SCR_EXP_AddUndefined]            = (scrExportFn_t)Scr_AddUndefined;
	scrExports[SCR_EXP_AddEntityNum]            = (scrExportFn_t)Scr_AddEntityNum;
	scrExports[SCR_EXP_AddStruct]               = (scrExportFn_t)Scr_AddStruct;
	scrExports[SCR_EXP_AddString]               = (scrExportFn_t)Scr_AddString;
	scrExports[SCR_EXP_AddIString]              = (scrExportFn_t)Scr_AddIString;
	scrExports[SCR_EXP_AddConstString]          = (scrExportFn_t)Scr_AddConstString;
	scrExports[SCR_EXP_AddVector]               = (scrExportFn_t)Scr_AddVector;
	scrExports[SCR_EXP_AddObject]               = (scrExportFn_t)Scr_AddObject;
	scrExports[SCR_EXP_AddArray]                = (scrExportFn_t)Scr_AddArray;
	scrExports[SCR_EXP_AddArrayStringIndexed]   = (scrExportFn_t)Scr_AddArrayStringIndexed;
	scrExports[SCR_EXP_MakeArray]               = (scrExportFn_t)Scr_MakeArray;

	scrExports[SCR_EXP_BeginLoadScripts]        = (scrExportFn_t)Scr_BeginLoadScripts;
	scrExports[SCR_EXP_BeginLoadAnimTrees]      = (scrExportFn_t)Scr_BeginLoadAnimTrees;
	scrExports[SCR_EXP_EndLoadScripts]          = (scrExportFn_t)Scr_EndLoadScripts;
	scrExports[SCR_EXP_EndLoadAnimTrees]        = (scrExportFn_t)Scr_EndLoadAnimTrees;
	scrExports[SCR_EXP_PrecacheAnimTrees]       = (scrExportFn_t)Scr_PrecacheAnimTrees;
	scrExports[SCR_EXP_FreeScripts]             = (scrExportFn_t)Scr_FreeScripts;
	scrExports[SCR_EXP_FreeGameVariable]        = (scrExportFn_t)Scr_FreeGameVariable;
	scrExports[SCR_EXP_ShutdownSystem]          = (scrExportFn_t)Scr_ShutdownSystem;
	scrExports[SCR_EXP_IsSystemActive]          = (scrExportFn_t)Scr_IsSystemActive;
	scrExports[SCR_EXP_AddExecThread]           = (scrExportFn_t)Scr_AddExecThread;
	scrExports[SCR_EXP_AddExecEntThreadNum]     = (scrExportFn_t)Scr_AddExecEntThreadNum;
	scrExports[SCR_EXP_ExecThread]              = (scrExportFn_t)Scr_ExecThread;
	scrExports[SCR_EXP_ExecEntThreadNum]        = (scrExportFn_t)Scr_ExecEntThreadNum;
	scrExports[SCR_EXP_IsThreadAlive]           = (scrExportFn_t)Scr_IsThreadAlive;

	scrExports[SCR_EXP_Error]                   = (scrExportFn_t)Scr_Error;
	scrExports[SCR_EXP_ErrorWithDialogMessage]  = (scrExportFn_t)Scr_ErrorWithDialogMessage;
	scrExports[SCR_EXP_ParamError]              = (scrExportFn_t)Scr_ParamError;
	scrExports[SCR_EXP_ObjectError]             = (scrExportFn_t)Scr_ObjectError;

	scrExports[SCR_EXP_SetDynamicEntityField]   = (scrExportFn_t)Scr_SetDynamicEntityField;
	scrExports[SCR_EXP_FreeEntityNum]           = (scrExportFn_t)Scr_FreeEntityNum;
	scrExports[SCR_EXP_GetEntityId]             = (scrExportFn_t)Scr_GetEntityId;
	scrExports[SCR_EXP_SetClassMap]             = (scrExportFn_t)Scr_SetClassMap;
	scrExports[SCR_EXP_RemoveClassMap]          = (scrExportFn_t)Scr_RemoveClassMap;
	scrExports[SCR_EXP_AddClassField]           = (scrExportFn_t)Scr_AddClassField;
	scrExports[SCR_EXP_AddFields]               = (scrExportFn_t)Scr_AddFields;
	scrExports[SCR_EXP_FindField]               = (scrExportFn_t)Scr_FindField;
	scrExports[SCR_EXP_GetOffset]               = (scrExportFn_t)Scr_GetOffset;
	scrExports[SCR_EXP_CopyEntityNum]           = (scrExportFn_t)Scr_CopyEntityNum;

	scrExports[SCR_EXP_Init]                    = (scrExportFn_t)Scr_Init;
	scrExports[SCR_EXP_Shutdown]                = (scrExportFn_t)Scr_Shutdown;
	scrExports[SCR_EXP_Abort]                   = (scrExportFn_t)Scr_Abort;
	scrExports[SCR_EXP_SetLoading]              = (scrExportFn_t)Scr_SetLoading;
	scrExports[SCR_EXP_InitSystem]              = (scrExportFn_t)Scr_InitSystem;
	scrExports[SCR_EXP_AllocGameVariable]       = (scrExportFn_t)Scr_AllocGameVariable;
	scrExports[SCR_EXP_GetChecksum]             = (scrExportFn_t)Scr_GetChecksum;

	scrExports[SCR_EXP_HasSourceFiles]          = (scrExportFn_t)Scr_HasSourceFiles;
	scrExports[SCR_EXP_SaveSource]              = (scrExportFn_t)Scr_SaveSource;
	scrExports[SCR_EXP_LoadSource]              = (scrExportFn_t)Scr_LoadSource;
	scrExports[SCR_EXP_SkipSource]              = (scrExportFn_t)Scr_SkipSource;
	scrExports[SCR_EXP_SavePre]                 = (scrExportFn_t)Scr_SavePre;
	scrExports[SCR_EXP_SavePost]                = (scrExportFn_t)Scr_SavePost;
	scrExports[SCR_EXP_SaveShutdown]            = (scrExportFn_t)Scr_SaveShutdown;
	scrExports[SCR_EXP_LoadPre]                 = (scrExportFn_t)Scr_LoadPre;
	scrExports[SCR_EXP_LoadShutdown]            = (scrExportFn_t)Scr_LoadShutdown;

	scrExports[SCR_EXP_LoadScript]              = (scrExportFn_t)Scr_LoadScript;
	scrExports[SCR_EXP_FindAnimTree]            = (scrExportFn_t)Scr_FindAnimTree;
	scrExports[SCR_EXP_FindAnim]                = (scrExportFn_t)Scr_FindAnim;
	scrExports[SCR_EXP_GetFunctionHandle]       = (scrExportFn_t)Scr_GetFunctionHandle;

	scrExports[SCR_EXP_FreeThread]              = (scrExportFn_t)Scr_FreeThread;
	scrExports[SCR_EXP_ConvertThreadToSave]     = (scrExportFn_t)Scr_ConvertThreadToSave;
	scrExports[SCR_EXP_ConvertThreadFromLoad]   = (scrExportFn_t)Scr_ConvertThreadFromLoad;

	scrExports[SCR_EXP_SetString]               = (scrExportFn_t)Scr_SetString;
	scrExports[SCR_EXP_AllocString]             = (scrExportFn_t)Scr_AllocString;
	scrExports[SCR_EXP_NotifyNum]               = (scrExportFn_t)Scr_NotifyNum;
	scrExports[SCR_EXP_NotifyId]                = (scrExportFn_t)Scr_NotifyId;

	scrExports[SCR_EXP_SL_ConvertToString]      = (scrExportFn_t)SL_ConvertToString;
	scrExports[SCR_EXP_SL_GetString]            = (scrExportFn_t)SL_GetString;
	scrExports[SCR_EXP_SL_GetLowercaseString]   = (scrExportFn_t)SL_GetLowercaseString;
	scrExports[SCR_EXP_SL_FindLowercaseString]  = (scrExportFn_t)SL_FindLowercaseString;
	scrExports[SCR_EXP_CreateCanonicalFilename] = (scrExportFn_t)Scr_CreateCanonicalFilename;

	scrExports[SCR_EXP_SetTime]                 = (scrExportFn_t)Scr_SetTime;
	scrExports[SCR_EXP_RunCurrentThreads]       = (scrExportFn_t)Scr_RunCurrentThreads;
	scrExports[SCR_EXP_ResetTimeout]            = (scrExportFn_t)Scr_ResetTimeout;
	scrExports[SCR_EXP_GetAnimsIndex]           = (scrExportFn_t)Scr_GetAnimsIndex;
	scrExports[SCR_EXP_GetAnims]                = (scrExportFn_t)Scr_GetAnims;

	scrExports[SCR_EXP_MT_Alloc]                = (scrExportFn_t)MT_Alloc;
	scrExports[SCR_EXP_MT_Free]                 = (scrExportFn_t)MT_Free;

	return (void *)&scrExports[0];
}
