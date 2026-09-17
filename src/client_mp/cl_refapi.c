/*
 * @fidelity: verified
 */

#include <stdlib.h>
#include <string.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"
#include "cl_refapi.h"

refexport_t re;
int         cls_rendererBound;

extern int  CL_RefPrintf();                     /* 0x00411460 cl_main_mp.c */
extern int  CL_ScaledMilliseconds();            /* 0x00411860 */
extern int  CL_GetFontInfo();                   /* 0x004118F0 */
extern int  CG_GetGameModel();                  /* 0x004118B0 */
extern int  CG_DObjCalcPose();                  /* 0x004118D0 */
extern int  SCR_AdjustFrom640();                /* 0x00416810 cl_scrn_mp.c */
extern int  CIN_UploadCinematic();              /* 0x004082B0 */
extern int  CIN_PlayCinematic();                /* 0x004078D0 */
extern int  CIN_RunCinematic();                 /* 0x00407680 */
extern int  CM_SaveLump();                      /* 0x00419F00 */
extern int  CM_GetPlaneNum();                   /* 0x00428130 */
extern int  Hunk_AllocInternal();               /* 0x00432160 */
extern int  Hunk_FreeTempMemory();              /* 0x00432410 */
extern int  Z_FreeInternal();                   /* 0x004318F0 */
extern int  Com_SaveCvarsToBuffer();            /* 0x00438E30 */
extern int  Com_LoadCvarsFromBuffer();          /* 0x00438EB0 */
extern int  FS_FileIsInPAK();                   /* 0x0043AE20 */
extern int  FS_FileExists();                    /* 0x00429310 */

extern int *GetRefAPI( const void *rimp, int apiVersion );

extern void StatMon_Reset( void );

/* ---- CL_BindRefImports  0x0041193B ---- */
static void CL_BindRefImports( void )
{
    ri_Printf            = (int (*)()) CL_RefPrintf;                    /* +0x00 */
    ri_Error             = (int (*)()) Com_Error;                       /* +0x04 */
    ri_Milliseconds      = (int (*)()) CL_ScaledMilliseconds;           /* +0x08 */
    ri_Hunk_Alloc        = (int (*)()) Hunk_AllocInternal;              /* +0x0C */
    ri_Hunk_AllocateTempMemory        = (int (*)()) Hunk_AllocateTempMemoryInternal; /* +0x10 */
    ri_Malloc        = (int (*)()) Z_MallocInternal;                /* +0x14 */
    ri_Free        = (int (*)()) Z_FreeInternal;                  /* +0x18 */
    ri_Hunk_FreeTempMemory        = (int (*)()) Hunk_FreeTempMemory;             /* +0x1C */
    ri_Cvar_Get          = (int (*)()) Cvar_Get;                        /* +0x20 */
    /* +0x24 Cvar_FindVar -- no symbol */
    ri_Cvar_Set          = (int (*)()) Cvar_Set;                        /* +0x28 */
    ri_Cmd_AddCommand    = (int (*)()) Cmd_AddCommand;                  /* +0x2C */
    ri_Cmd_RemoveCommand = (int (*)()) Cmd_RemoveCommand;               /* +0x30 */
    ri_Cmd_Argc        = (int (*)()) Cmd_Argc;                        /* +0x34 */
    ri_Cmd_Argv        = (int (*)()) Cmd_Argv;                        /* +0x38 */
    ri_Cmd_ExecuteText        = (int (*)()) Cbuf_ExecuteText;                /* +0x3C */
    ri_SaveCvarsToBuffer        = (int (*)()) Com_SaveCvarsToBuffer;           /* +0x40 */
    ri_LoadCvarsFromBuffer        = (int (*)()) Com_LoadCvarsFromBuffer;         /* +0x44 */
    /* +0x48 FS_FileIsInPAK -- no symbol */
    ri_FS_ReadFile        = (int (*)()) FS_ReadFile;                     /* +0x4C */
    ri_FS_FreeFile        = (int (*)()) FS_FreeFile;                     /* +0x50 */
    ri_FS_ListFiles      = (int (*)()) FS_ListFiles;                    /* +0x54 */
    ri_FS_FreeFileList        = (int (*)()) FS_FreeFileList;                 /* +0x58 */
    ri_FS_WriteFile        = (int (*)()) FS_WriteFile;                    /* +0x5C */
    /* +0x60 FS_FileExists -- no symbol */
    ri_FS_FOpenFileByMode        = (int (*)()) FS_FOpenFileByMode;              /* +0x64 */
    ri_FS_FCloseFile        = (int (*)()) FS_FCloseFile;                   /* +0x68 */
    ri_FS_Read        = (int (*)()) FS_Read;                         /* +0x6C */
    /* +0x70 FS_Write -- no symbol */
    ri_CM_SaveLump        = (int (*)()) CM_SaveLump;                     /* +0x74 */
    ri_CM_GetPlaneNum        = (int (*)()) CM_GetPlaneNum;                  /* +0x78 */
    ri_CIN_UploadCinematic        = (int (*)()) CIN_UploadCinematic;             /* +0x7C */
    ri_CIN_PlayCinematic        = (int (*)()) CIN_PlayCinematic;               /* +0x80 */
    ri_CIN_RunCinematic        = (int (*)()) CIN_RunCinematic;                /* +0x84 */
    ri_CG_GetGameModel        = (int (*)()) CG_GetGameModel;                 /* +0x88 */
    ri_CG_DObjCalcPose        = (int (*)()) CG_DObjCalcPose;                 /* +0x8C */
    ri_SCR_AdjustFrom640        = (int (*)()) SCR_AdjustFrom640;               /* +0x90 */
    ri_GetFontInfo        = (int (*)()) CL_GetFontInfo;                  /* +0x94 */
}

/* ---- CL_BindRefExports  no-address ---- */
static void CL_BindRefExports( void )
{
    re.Shutdown                 = (void (*)( int )) re_Shutdown;
    re.BeginRegistration        = (void (*)( const char *, glconfig_t * )) BeginRegistration;
    re.GetXModelByHandle        = (int (*)()) GetXModelByHandle;
    re.RegisterModel            = (int (*)( const char *, int )) RegisterModel;
    re.RegisterShader           = (int (*)( const char *, int )) RegisterShader;
    re.RegisterShaderNoMip      = (int (*)( const char *, int )) RegisterShaderNoMip;
    re.LoadWorldMap             = (int (*)()) LoadWorldMap;
    re.FinishLoadingModels      = (int (*)()) FinishLoadingModels;
    re.SetIgnorePrecacheErrors  = (int (*)()) SetIgnorePrecacheErrors;
    re.GetIgnorePrecacheErrors  = (int (*)()) GetIgnorePrecacheErrors;
    re.GetShaderFromModel       = (int (*)()) GetShaderFromModel;
    re.GetImageMemory           = (int (*)()) GetImageMemory;
    re.GetShaderName            = (int (*)()) GetShaderName;
    re.GetFarPlaneDist          = (int (*)()) GetFarPlaneDist;
    re.EndRegistration          = (int (*)()) EndRegistration;
    re.ClearScene               = (void (*)( void )) ClearScene;
    re.AddRefEntityToScene      = (void (*)( const void *, int )) AddRefEntityToScene;
    re.AddPolyToScene           = (void (*)( int, int, const void * )) AddPolyToScene;
    re.AddPolysToScene          = (void (*)( int, int, const void *, int )) AddPolysToScene;
    re.AddLightToScene          = (void (*)( const float *, float, float, float, float ))
                                      AddLightToScene;
    re.SetFarPlaneDist          = (int (*)()) SetFarPlaneDist;
    re.AddCoronaToScene         = (void (*)( const float *, float, float, float,
                                             float, int, int )) AddCoronaToScene;
    re.SetFog                   = (int (*)()) SetFog;
    re.SaveFogState             = (int (*)()) SaveFogState;
    re.RestoreFogState          = (int (*)()) RestoreFogState;
    re.RenderScene              = (void (*)( const void * )) RenderScene;
    re.ClearFlares              = (int (*)()) ClearFlares;
    re.SetColor                 = (void (*)( const float * )) SetColor;
    re.StretchPic               = (void (*)( float, float, float, float,
                                             float, float, float, float, int )) StretchPic;
    re.StretchPicGradient       = (int (*)()) StretchPicGradient;
    re.StretchPicRotate         = (int (*)()) StretchPicRotate;
    re.DrawQuadPic              = (int (*)()) DrawQuadPic;
    re.StretchRaw               = (int (*)()) StretchRaw;
    re.UploadCinematic          = (int (*)()) UploadCinematic;
    re.BeginFrame               = (void (*)( int )) BeginFrame;
    re.EndFrame                 = (void (*)( int *, int * )) EndFrame;
    re.SaveScreen               = (int (*)()) SaveScreen;
    re.BlendSavedScreen         = (int (*)()) BlendSavedScreen;
    re.LerpTag                  = (int (*)()) MarkFragments;
    re.ModelBounds              = (void (*)( int, float *, float * )) ModelBounds;
    re.TrackStatistics          = (int (*)()) TrackStatistics;
    re.PickShader               = (int (*)()) PickShader;
    re.RegisterFont             = (int (*)()) RegisterFont;
    re.GetEntityToken           = (int (*)()) GetEntityToken;
    re.ResetImageAllocations    = (int (*)()) ResetImageAllocations;
    re.FreeImageAllocations     = (int (*)()) FreeImageAllocations;
    re.CubemapShot              = (int (*)()) CubemapShot;
    re.CubemapWaterShot         = (int (*)()) CubemapWaterShot;
    re.LocateDebugStrings       = (int (*)()) LocateDebugStrings;
    re.LocateDebugLines         = (int (*)()) LocateDebugLines;
    re.AddPlume                 = (int (*)()) AddPlume;
    re.Text_Width               = (int (*)()) Text_Width;
    re.Text_Height              = (int (*)()) Text_Height;
    re.Text_Paint               = (int (*)()) Text_Paint;
    re.Text_ConsoleWidth        = (int (*)()) Text_ConsoleWidth;
    re.Text_ConsolePaint        = (int (*)()) Text_ConsolePaint;
    re.Text_PaintWithCursor     = (int (*)()) Text_PaintWithCursor;

    com_serverEndpoint       = (int (*)()) re.Shutdown;              /* +0x00 */
    re_BeginRegistration     = (int (*)()) re.BeginRegistration;     /* +0x04 */
    dword_1432868            = (int (*)()) re.GetXModelByHandle;     /* +0x08 */
    dword_143286C            = (int (*)()) re.RegisterModel;         /* +0x0C */
    re_RegisterShader        = (int (*)()) re.RegisterShader;        /* +0x10 */
    Material_RegisterHandle  = (int (*)()) re.RegisterShaderNoMip;   /* +0x14 */
    dword_1432878            = (int (*)()) re.LoadWorldMap;          /* +0x18 */
    dword_143287C            = (void *)    re.FinishLoadingModels;   /* +0x1C */
    dword_1432880            = (int (*)()) re.SetIgnorePrecacheErrors; /* +0x20 */
    /* +0x24 GetIgnorePrecacheErrors -- no cell declared */
    dword_1432888            = (int (*)()) re.GetShaderFromModel;    /* +0x28 */
    /* +0x2C GetImageMemory -- no cell declared */
    dword_1432890            = (int (*)()) re.GetShaderName;         /* +0x30 */
    dword_1432894            = (void *)    re.GetFarPlaneDist;       /* +0x34 */
    cgvm_dllEntry            = (int (*)()) re.EndRegistration;       /* +0x38 */
    dword_143289C            = (void *)    re.ClearScene;            /* +0x3C */
    dword_14328A0            = (int (*)()) re.AddRefEntityToScene;   /* +0x40 */
    dword_14328A4            = (int (*)()) re.AddPolyToScene;        /* +0x44 */
    dword_14328A8            = (int (*)()) re.AddPolysToScene;       /* +0x48 */
    dword_14328AC            = (int (*)()) re.AddLightToScene;       /* +0x4C */
    /* +0x50 SetFarPlaneDist -- no cell declared */
    dword_14328B4            = (int (*)()) re.AddCoronaToScene;      /* +0x54 */
    dword_14328B8            = (int (*)()) re.SetFog;                /* +0x58 */
    cgame_SaveExportTable    = (int (*)()) re.SaveFogState;          /* +0x5C */
    cgame_RestoreExportTable = (int (*)()) re.RestoreFogState;       /* +0x60 */
    dword_14328C4            = (int (*)()) re.RenderScene;           /* +0x64 */
    dword_14328C8            = (int (*)()) re.ClearFlares;           /* +0x68 */
    re_SetColor              = (int (*)()) re.SetColor;              /* +0x6C */
    re_DrawStretchPic        = (int (*)()) re.StretchPic;            /* +0x70 */
    dword_14328D4            = (int (*)()) re.StretchPicGradient;    /* +0x74 */
    dword_14328D8            = (int (*)()) re.StretchPicRotate;      /* +0x78 */
    dword_14328DC            = (int (*)()) re.DrawQuadPic;           /* +0x7C */
    re_DrawStretchRawPtr     = (int (*)()) re.StretchRaw;            /* +0x80 */
    re_UploadCinematicPtr    = (int (*)()) re.UploadCinematic;       /* +0x84 */
    re_BeginFrame            = (int (*)()) re.BeginFrame;            /* +0x88 */
    dword_14328EC            = (int (*)()) re.EndFrame;              /* +0x8C */
    dword_14328F0            = (void *)    re.SaveScreen;            /* +0x90 */
    dword_14328F4            = (int (*)()) re.BlendSavedScreen;      /* +0x94 */
    dword_14328F8            = (int (*)()) re.LerpTag;               /* +0x98 */
    dword_14328FC            = (int (*)()) re.ModelBounds;           /* +0x9C */
    dword_1432900            = (int (*)()) re.TrackStatistics;       /* +0xA0 */
    dword_1432904            = (int (*)()) re.PickShader;            /* +0xA4 */
    dword_1432908            = (int (*)()) re.RegisterFont;          /* +0xA8 */
    dword_143290C            = (int (*)()) re.GetEntityToken;        /* +0xAC */
    /* +0xB0 ResetImageAllocations, +0xB4 FreeImageAllocations -- no cells declared */
    dword_1432918            = (int (*)()) re.CubemapShot;           /* +0xB8 */
    dword_143291C            = (int (*)()) re.CubemapWaterShot;      /* +0xBC */
    dword_1432920            = (int (*)()) re.LocateDebugStrings;    /* +0xC0 */
    dword_1432924            = (int (*)()) re.LocateDebugLines;      /* +0xC4 */
    dword_1432928            = (void *)    re.AddPlume;              /* +0xC8 */
    dword_143292C            = (int (*)()) re.Text_Width;            /* +0xCC */
    dword_1432930            = (int (*)()) re.Text_Height;           /* +0xD0 */
    dword_1432934            = (int (*)()) re.Text_Paint;            /* +0xD4 */
    dword_1432938            = (int (*)()) re.Text_ConsoleWidth;     /* +0xD8 */
    dword_143293C            = (int (*)()) re.Text_ConsolePaint;     /* +0xDC */
    dword_1432940            = (int (*)()) re.Text_PaintWithCursor;  /* +0xE0 */
}

/* ---- CL_ClearRefExports  no-address ---- */
static void CL_ClearRefExports( void )
{
    memset( &re, 0, sizeof( re ) );

    com_serverEndpoint       = 0;
    re_BeginRegistration     = 0;
    dword_1432868            = 0;
    dword_143286C            = 0;
    re_RegisterShader        = 0;
    Material_RegisterHandle  = 0;
    dword_1432878            = 0;
    dword_143287C            = 0;
    dword_1432880            = 0;
    dword_1432888            = 0;
    dword_1432890            = 0;
    dword_1432894            = 0;
    cgvm_dllEntry            = 0;
    dword_143289C            = 0;
    dword_14328A0            = 0;
    dword_14328A4            = 0;
    dword_14328A8            = 0;
    dword_14328AC            = 0;
    dword_14328B4            = 0;
    dword_14328B8            = 0;
    cgame_SaveExportTable    = 0;
    cgame_RestoreExportTable = 0;
    dword_14328C4            = 0;
    dword_14328C8            = 0;
    re_SetColor              = 0;
    re_DrawStretchPic        = 0;
    dword_14328D4            = 0;
    dword_14328D8            = 0;
    dword_14328DC            = 0;
    re_DrawStretchRawPtr     = 0;
    re_UploadCinematicPtr    = 0;
    re_BeginFrame            = 0;
    dword_14328EC            = 0;
    dword_14328F0            = 0;
    dword_14328F4            = 0;
    dword_14328F8            = 0;
    dword_14328FC            = 0;
    dword_1432900            = 0;
    dword_1432904            = 0;
    dword_1432908            = 0;
    dword_143290C            = 0;
    dword_1432918            = 0;
    dword_143291C            = 0;
    dword_1432920            = 0;
    dword_1432924            = 0;
    dword_1432928            = 0;
    dword_143292C            = 0;
    dword_1432930            = 0;
    dword_1432934            = 0;
    dword_1432938            = 0;
    dword_143293C            = 0;
    dword_1432940            = 0;

    cls_rendererBound = 0;
}

/* ---- CL_InitRef  0x00411920 ----  [CONFIRMED] */
void CL_InitRef( void )
{
    int *refapi;

    Com_Printf( "----- Initializing Renderer ----\n" );

    CL_BindRefImports();

    refapi = GetRefAPI( (const void *) 0, REF_API_VERSION );

    Com_Printf( "-------------------------------\n" );

    if ( !refapi ) {
        Com_Error( ERR_FATAL, "EXE_ERR_COULDNT_INIT_REFRESH" );
        return;
    }

    CL_BindRefExports();
    cls_rendererBound = 1;

    {
        const int *slot = (const int *) &re;
        int        i, nulls = 0;

        for ( i = 0; i < (int) ( sizeof( re ) / sizeof( int ) ); i++ ) {
            if ( !slot[i] ) {
                nulls++;
                Com_DPrintf( "CL_InitRef: refexport_t slot +0x%02X is NULL "
                             "at bind time\n", i * 4 );
            }
        }
        Com_DPrintf( "CL_InitRef: %d of %d refexport_t slots bound\n",
                     (int) ( sizeof( re ) / sizeof( int ) ) - nulls,
                     (int) ( sizeof( re ) / sizeof( int ) ) );
    }

    CL_BssSnapshot();

    Cvar_Set2( "cl_paused", "0", qtrue );
}

static unsigned char *cl_bssCopy;
static unsigned char *cl_bssLo;
static unsigned int   cl_bssLen;

/* ---- CL_BssBounds  no-address ---- */
static void CL_BssBounds( void )
{
    void *probe[] = {
        (void *) &com_serverEndpoint, (void *) &re_Shutdown,
        (void *) &ri_Printf,          (void *) &cl_debuggraph,
        (void *) &cl_timegraph,       (void *) &cl_graphheight,
        (void *) &sv_maxRate,         (void *) &sv_gametype,
        (void *) &sv_maxclients,      (void *) &cls_state,
        (void *) &whiteShader,        (void *) &dwStyle,
        (void *) &dwExStyle,          (void *) &tr_registered,
        (void *) &dword_1432940,      (void *) &Material_RegisterHandle,
        (void *) &cgame_SaveExportTable, (void *) &ri_GetFontInfo,
        (void *) &cl_active,          (void *) &clc_clientNum
    };
    unsigned int lo = 0xFFFFFFFFu, hi = 0;
    int i;

    for ( i = 0; i < (int) ( sizeof( probe ) / sizeof( probe[0] ) ); i++ ) {
        unsigned int a = (unsigned int) probe[i];
        if ( a < lo ) { lo = a; }
        if ( a > hi ) { hi = a; }
    }
    cl_bssLo  = (unsigned char *) lo;
    cl_bssLen = ( hi + 4 ) - lo;
}

/* ---- CL_BssSnapshot  no-address ---- */
void CL_BssSnapshot( void )
{
    static int enabled = -1;

    if ( enabled < 0 ) {
        enabled = Cvar_Get( "cl_bssWatch", "0", 0 )->integer ? 1 : 0;
    }
    if ( !enabled ) {
        return;
    }

    if ( !cl_bssCopy ) {
        CL_BssBounds();
        cl_bssCopy = (unsigned char *) malloc( cl_bssLen );
        if ( !cl_bssCopy ) {
            Com_Printf( "CL_BssSnapshot: malloc(%u) failed\n", cl_bssLen );
            return;
        }
        Com_Printf( "CL_BssSnapshot: watching %p..%p (%u bytes)\n",
                    (void *) cl_bssLo, (void *) ( cl_bssLo + cl_bssLen ),
                    cl_bssLen );
    }
    memcpy( cl_bssCopy, cl_bssLo, cl_bssLen );
}

/* ---- CL_BssDiff  no-address ---- */
int CL_BssDiff( const char *where )
{
    unsigned int i, runStart = 0, runs = 0, bytes = 0;
    unsigned int lowest = 0xFFFFFFFFu, highest = 0;
    int          inRun = 0;

    if ( !cl_bssCopy ) {
        return 0;
    }

    for ( i = 0; i < cl_bssLen; i++ ) {
        int cleared = ( cl_bssCopy[i] != 0 && cl_bssLo[i] == 0 );

        if ( cleared && !inRun ) {
            inRun = 1;
            runStart = i;
        } else if ( !cleared && inRun ) {
            inRun = 0;
            runs++;
            bytes += i - runStart;
            if ( runs <= 120 ) {
                Com_Printf( "  cleared %p .. %p  (%u bytes)\n",
                            (void *) ( cl_bssLo + runStart ),
                            (void *) ( cl_bssLo + i ), i - runStart );
            }
            if ( (unsigned int) ( cl_bssLo + runStart ) < lowest ) {
                lowest = (unsigned int) ( cl_bssLo + runStart );
            }
            if ( (unsigned int) ( cl_bssLo + i ) > highest ) {
                highest = (unsigned int) ( cl_bssLo + i );
            }
        }
    }
    if ( inRun ) {
        runs++;
        bytes += cl_bssLen - runStart;
        if ( (unsigned int) ( cl_bssLo + runStart ) < lowest ) {
            lowest = (unsigned int) ( cl_bssLo + runStart );
        }
        highest = (unsigned int) ( cl_bssLo + cl_bssLen );
    }

    if ( runs ) {
        Com_Printf( "CL_BssDiff(%s): %u cleared runs, %u bytes, "
                    "outermost %p..%p (span %u)\n",
                    where ? where : "?", runs, bytes,
                    (void *) lowest, (void *) highest,
                    highest - lowest );

        if ( highest > lowest ) {
            unsigned int j, survivors = 0, firstSurv = 0;
            for ( j = lowest; j < highest; j++ ) {
                if ( *(unsigned char *) j ) {
                    if ( !survivors ) { firstSurv = j; }
                    survivors++;
                }
            }
            Com_Printf( "  inside that span, %u of %u bytes are STILL "
                        "non-zero (first at %p) -- %s\n",
                        survivors, highest - lowest, (void *) firstSurv,
                        survivors ? "SCATTERED STORES, not one memset"
                                  : "one contiguous clear" );
        }
    }

    memcpy( cl_bssCopy, cl_bssLo, cl_bssLen );
    return (int) runs;
}

/* ---- CL_RefApiIntegrityCheck  no-address ---- */
void CL_RefApiIntegrityCheck( const char *where )
{
    static int reported;
    int        dead;

    {
        static int  uiDiffSeq;
        static int  uiDiffHits;
        char        label[64];

        if ( uiDiffHits < 100000 ) {
            Com_sprintf( label, sizeof( label ), "UI syscall #%d", ++uiDiffSeq );
            if ( CL_BssDiff( label ) ) {
                uiDiffHits++;
            }
        }

    }

    if ( reported || !cls_rendererBound ) {
        return;
    }

    dead = 0;

    /* refexport_t, renderer side, 0x00CA26C8 */
    if ( !re_Shutdown )              dead |= 0x0001;   /* +0x00 */
    if ( !EndRegistration )          dead |= 0x0002;   /* +0x38 */
    if ( !BeginFrame )               dead |= 0x0004;   /* +0x88 */
    if ( !RegisterFont )             dead |= 0x0008;   /* +0xA8 */
    if ( !Text_PaintWithCursor )     dead |= 0x0010;   /* +0xE0 */

    /* refimport_t, 0x016D8920 */
    if ( !ri_Printf )                dead |= 0x0020;   /* +0x00 */
    if ( !ri_FS_ListFiles )          dead |= 0x0040;   /* +0x54 */
    if ( !ri_GetFontInfo )            dead |= 0x0080;   /* +0x94 */

    /* refexport_t, client side, 0x01432860 */
    if ( !com_serverEndpoint )       dead |= 0x0100;   /* +0x00 */
    if ( !Material_RegisterHandle )  dead |= 0x0200;   /* +0x14 */
    if ( !cgame_SaveExportTable )    dead |= 0x0400;   /* +0x5C */
    if ( !re_BeginFrame )            dead |= 0x0800;   /* +0x88 */
    if ( !dword_1432940 )            dead |= 0x1000;   /* +0xE0 */

    if ( !dead ) {
        return;
    }

    reported = 1;
    Com_Printf( "CL_RefApiIntegrityCheck(%s): BOUNDARY TABLE SLOTS WENT NULL "
                "AFTER CL_InitRef, mask 0x%04X\n", where ? where : "?", dead );
    {
        const int *slot = (const int *) &re;
        int        i, nulls = 0;

        for ( i = 0; i < (int) ( sizeof( re ) / sizeof( int ) ); i++ ) {
            if ( !slot[i] ) {
                nulls++;
            }
        }
        Com_Printf( "  hand-written `re` at %p: %d of %d slots still bound\n",
                    (void *) &re,
                    (int) ( sizeof( re ) / sizeof( int ) ) - nulls,
                    (int) ( sizeof( re ) / sizeof( int ) ) );
    }
    Com_Printf( "  refexport renderer 0x00CA26C8: +0x00 %s +0x38 %s +0x88 %s "
                "+0xA8 %s +0xE0 %s\n",
                re_Shutdown ? "ok" : "NULL", EndRegistration ? "ok" : "NULL",
                BeginFrame ? "ok" : "NULL", RegisterFont ? "ok" : "NULL",
                Text_PaintWithCursor ? "ok" : "NULL" );
    Com_Printf( "  refimport          0x016D8920: +0x00 %s +0x54 %s +0x94 %s\n",
                ri_Printf ? "ok" : "NULL", ri_FS_ListFiles ? "ok" : "NULL",
                ri_GetFontInfo ? "ok" : "NULL" );
    Com_Printf( "  refexport client   0x01432860: +0x00 %s +0x14 %s +0x5C %s "
                "+0x88 %s +0xE0 %s\n",
                com_serverEndpoint ? "ok" : "NULL",
                Material_RegisterHandle ? "ok" : "NULL",
                cgame_SaveExportTable ? "ok" : "NULL",
                re_BeginFrame ? "ok" : "NULL", dword_1432940 ? "ok" : "NULL" );
}

/* ---- CL_ShutdownRef  0x00411500 ----  [CONFIRMED] */
void CL_ShutdownRef( void )
{
    if ( !com_serverEndpoint ) {
        return;
    }

    re.Shutdown( 1 );

    CL_ClearRefExports();
    StatMon_Reset();
}
