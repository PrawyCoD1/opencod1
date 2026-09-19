/*
 * Machine-translated from Call of Duty 1.1 (Windows, CoDMP.exe).
 * Original translation unit:
 * /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/xanim/dobj.cpp
 * Retail range 0x00480DA0-0x00483FC0, 53 functions.
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"

#define xanim_pool       xanim_generatedPoolDecl_unused
#define FindNextSibling  xanim_generatedIndirectionsDecl_unused
#define FindObject       xanim_generatedNodesStatusDecl_unused
#include "../qcommon/cod1_globals.h"
#undef xanim_pool
#undef FindNextSibling
#undef FindObject

#include "../script/scr_local.h"

typedef struct DObjSkelMat_s {
    float axis[3][4];   /* +0x00  the 4th float of each row is never written */
    float origin[3];    /* +0x30 */
    float pad3C;        /* +0x3C  never written by DObjQuatToMatrix43 */
} DObjSkelMat;

typedef struct DObjAnimMat_s {
    float quat[4];      /* +0x00 */
    float weight;       /* +0x10  accumulated contribution weight */
    float trans[3];     /* +0x14 */
} DObjAnimMat;

typedef struct XAnimEvalStorage_s {
    unsigned char evaluatedPartBits[16];  /* +0x00 */
    unsigned char controlPartBits[16];    /* +0x10 */
    unsigned char blockedPartBits[16];    /* +0x20 */
    DObjSkelMat   basePose[1];            /* +0x30  [partCount] */
} XAnimEvalStorage;

typedef struct XAnimEntry_s {
    unsigned short childCount;   /* +0x00  0 == leaf */
    unsigned short parentIndex;  /* +0x02 */
    union {
        void *leafAsset;         /* +0x04  fileData_t * when childCount == 0 */
        struct {
            unsigned short flags;            /* +0x04 */
            unsigned short firstChildIndex;  /* +0x06 */
        } parent;
    } u;
} XAnimEntry;

typedef struct XAnim_s {
    const char *name;        /* +0x00 */
    int         nodeCount;   /* +0x04 */
    XAnimEntry  entries[1];  /* +0x08  [nodeCount] */
} XAnim;

typedef struct XAnimTree_s {
    XAnim         *sourceTree;             /* +0x00 */
    int            partRemapTableSelector; /* +0x04  DObjCreate toggles 0<->1 */
    unsigned short poolNodeHandles[1];     /* +0x08  [nodeCount] */
} XAnimTree;

typedef struct XAnimState_s {
    float          time;                     /* +0x00 */
    float          oldTime;                  /* +0x04 */
    unsigned short cycleCount;               /* +0x08 */
    unsigned short oldCycleCount;            /* +0x0A */
    float          weightBlendTimeRemaining; /* +0x0C */
    float          targetWeight;             /* +0x10 */
    float          currentWeight;            /* +0x14 */
    float          rateScale;                /* +0x18 */
} XAnimState;

typedef struct XAnimInfo_s {
    unsigned short notifyChildIndex; /* +0x00 */
    short          notifyIndex;      /* +0x02  -1 == no pending notify */
    unsigned short notifyName;       /* +0x04  script string handle */
    unsigned short notifyType;       /* +0x06 */
    unsigned short freePrev;         /* +0x08 */
    unsigned short freeNext;         /* +0x0A  node 0's is the free-list head */
    XAnimState     states[2];        /* +0x0C and +0x28 */
} XAnimInfo;

typedef struct XAnimNotifyInfo_s {
    unsigned short name;      /* +0x00 */
    unsigned short pad02;
    float          frameFrac; /* +0x04 */
} XAnimNotifyInfo;

typedef struct XAnimParts_s {
    unsigned short   frameCountMinusOne;    /* +0x00 */
    unsigned char    looped;                /* +0x02 */
    unsigned char    hasDeltaMotion;        /* +0x03 */
    float            frameRate;             /* +0x04 */
    float            frequency;             /* +0x08 */
    unsigned short  *partNameHandles;       /* +0x0C */
    void            *partStreams;           /* +0x10  {trans,rot} stream pairs */
    XAnimNotifyInfo *notify;                /* +0x14 */
    void            *deltaMotion;           /* +0x18 */
    unsigned char   *compressedRotationBits;/* +0x1C */
} XAnimParts;

typedef struct DObjModel_s {
    void       *model;              /* +0x00  XModel * */
    const char *tagName;            /* +0x04  parent bone name, may be NULL/"" */
    short       negativeModelIndex; /* +0x08 */
    short       reserved0A;         /* +0x0A */
} DObjModel;

typedef struct DObj_s {
    XAnimTree     *tree;                      /* +0x00 */
    void          *evalStorage;               /* +0x04  XAnimEvalStorage * */
    int            skelCacheKey;              /* +0x08 */
    unsigned char *partRemapTable;            /* +0x0C  into the tree remap tail */
    unsigned short unknownState10;
    unsigned short scrNotifyId;
    unsigned short tracePartRemapHandle;      /* +0x14 */
    unsigned char  childCount;                /* +0x16 */
    unsigned char  partCount;                 /* +0x17  total bones, < 128 */
    unsigned int   childRefs[8];              /* +0x18  XModel * per child */
    short          childModelIndices[8];      /* +0x38 */
    unsigned char  childParentPartIndices[8]; /* +0x48  0xFF == unparented */
    unsigned char  childPartBaseIndices[8];   /* +0x50 */
} DObj;

typedef struct XModelPartNameTable_s {
    short          count;                    /* +0x00  SIGNED (movsx) */
    unsigned short handles[1];               /* +0x02  count entries */
} XModelPartNameTable;

typedef struct XModelPartNameTableSlot_s {
    XModelPartNameTable *partNameTable;      /* +0x00 */
    unsigned char        parentPartDeltas[1];/* +0x04  partCount-rootPartCount */
} XModelPartNameTableSlot;

typedef struct XModelPartColl_s {
    float mins[3];                           /* +0x00 */
    float maxs[3];                           /* +0x0C */
    float center[3];                         /* +0x18 */
    float radiusSq;                          /* +0x24 */
} XModelPartColl;

typedef struct XModelPartsData_s {
    XModelPartNameTableSlot *partNameTableSlot; /* +0x00 */
    short           rootPartCount;              /* +0x04  SIGNED */
    short           pad06;
    XModelPartColl *partCollisions;             /* +0x08 */
    short         (*baseRotations)[4];          /* +0x0C  packed quat */
    float         (*baseTranslations)[3];       /* +0x10 */
    unsigned char  *partStateIndices;           /* +0x14 */
} XModelPartsData;

typedef struct XModelPartsEntry_s {
    const char      *name;
    XModelPartsData *data;
    void           (*freeData)(struct XModelPartsEntry_s *);
} XModelPartsEntry;

typedef struct XModel_s {
    const char       *name;                  /* +0x00 */
    XModelPartsEntry **info;
    void            (*freeData)(struct XModel_s *);
} XModel;

/* ---- DObjModelParts  no-address ---- */
static XModelPartsData *DObjModelParts( const XModel *model )
{
    return (*model->info)->data;
}

/* ---- DObjModelPartNames  no-address ---- */
static XModelPartNameTable *DObjModelPartNames( const XModel *model )
{
    return DObjModelParts( model )->partNameTableSlot->partNameTable;
}

#define DOBJ_CHILD_MODEL(d,i)   ( (XModel *)(d)->childRefs[i] )

#define SL_REFSTRING(h)     ( (char *)(void *)GetRefString_var \
                              + 8 * (unsigned int)(unsigned short)(h) )
#define SL_STRING(h)        ( (h) ? SL_REFSTRING(h) + 4 : (char *)0 )

#define DOBJ_TRACE_REMAP_PREFIX     16
#define DOBJ_TRACE_REMAP_STRING_TYPE 12
#define XANIM_FILENAME_STRING_TYPE   7
#define DOBJ_CHILD_PARENT_NONE      0xFF

#define DOBJ_BIT_SET(bits,i)    ( ((unsigned char *)(bits))[(i) >> 3] \
                                  |= (unsigned char)(1 << ((i) & 7)) )
#define DOBJ_BIT_TEST(bits,i)   ( ( ((const unsigned char *)(bits))[(i) >> 3] \
                                    & (1 << ((i) & 7)) ) != 0 )

XAnimInfo xanim_pool[2048];

extern int AllocVariable__Fv();
extern int ClearObjectInternal__FUs();
extern void *FS_GetDataForFile( const char *dir, const char *name, const char *ext );
extern int FreeVariable__FUs();
extern unsigned short GetVariableIndexInternal( unsigned short parentId,
                                                unsigned int name );
extern int RemoveRefToObject__FUs();
extern int SL_FreeString__FUsPCcUi();
extern unsigned short SL_GetLowercaseStringOfLen( const char *text,
                                                  unsigned char user,
                                                  unsigned int size, int type );
extern unsigned short SL_GetStringOfLen( const char *text, unsigned char user,
                                         unsigned int size, int type );
extern const char *SL_ConvertToString( unsigned short handle );
extern void        SL_RemoveRefToStringOfLen( unsigned short handle,
                                              unsigned int size );
extern void        SL_RemoveRefToString( unsigned short handle );
extern void __cdecl XAnimClearTree( XAnimTree *tree );
extern int ReadQuat( const short *packed, short *out );
extern int ReadQuat2( const short *packed, short *out );
extern int __cdecl XModelGetLodForDist( const void *model, float distance );
extern int XModelTraceLine( int model, int work, const float *basePose,
                       const float *start, const float *end, int contentmask );

/* ---- XAnimInitSmallTreePool  0x00480DA0 ----  VERIFIED */
__int16 XAnimInitSmallTreePool()
{
  __int16 result;
  int v1[5];

  Com_Memset(v1, 0, 20);
  /* 0x00480DB1: push 0Ch / push 11h / push 0 / push buf -- type 12. */
  result = SL_GetStringOfLen((char *)v1, 0, 0x11u, 12);
  LOWORD(dword_140733C) = result;
  return result;
}

/* ---- Scr_FreeGameStringRef_m  0x00480DE0 ----  VERIFIED */
void Scr_FreeGameStringRef_m( void )
{
    unsigned short handle = (unsigned short)dword_140733C;

    if ( handle ) {
        SL_RemoveRefToStringOfLen( handle, 0x11 );
        LOWORD(dword_140733C) = 0;
    }
}
#if 0
// XAnimShutdownSmallTreePool (retail name of Scr_FreeGameStringRef_m; not a script function): releases the default trace part-remap string and clears its handle.  Inverse of 0x00480DA0.
__int16 Scr_FreeGameStringRef_m()
{
  int v0;

  LOWORD(v0) = dword_140733C;
  if ( (_WORD)dword_140733C )
  {
    v0 = GetRefString_var + 8 * (unsigned __int16)dword_140733C;
    if ( *(_WORD *)v0 )
      --*(_WORD *)v0;
    else
      LOWORD(v0) = SL_FreeString(dword_140733C, (char *)(v0 + 4), 0x11u);
    LOWORD(dword_140733C) = 0;
  }
  return v0;
}
#endif

/* ---- QuatToMatrix_m  0x00480E20 ----  VERIFIED */
int __fastcall QuatToMatrix_m(float *a1, float *a2)
{
  double v2;
  double v3;
  int result;
  double v5;
  double v6;
  double v7;
  double v8;
  double v9;
  double v10;
  double v11;
  double v12;
  float v13;
  float v14;
  float v15;
  float v16;
  float v17;
  float v18;
  float v19;

  v13 = *a2 * *a2;
  v15 = a2[1] * a2[1];
  v17 = a2[2] * a2[2];
  v2 = a2[3] * a2[3] + v17 + v15 + v13;
  if ( (v2 == 0.0) | __UNORDERED__(v2, 0.0) )
  {
    result = 0;
    *a1 = 1.0;
    a1[1] = 0.0;
    a1[2] = 0.0;
    a1[4] = 0.0;
    a1[5] = 1.0;
    a1[6] = 0.0;
    a1[8] = 0.0;
    a1[9] = 0.0;
    a1[10] = 1.0;
    a1[3] = 0.0;
    a1[7] = 0.0;
    a1[11] = 0.0;
  }
  else
  {
    v3 = 2.0 / v2;
    result = 0;
    v5 = v13 * v3;
    v6 = v3 * v15;
    v7 = v3 * v17;
    v8 = v3 * *a2;
    v16 = v8 * a2[2];
    v9 = v8 * a2[1];
    v19 = v8 * a2[3];
    v10 = v3 * a2[1];
    v14 = v10 * a2[2];
    v18 = v10 * a2[3];
    v11 = a2[3] * a2[2];
    a1[3] = 0.0;
    a1[7] = 0.0;
    a1[11] = 0.0;
    v12 = v11 * v3;
    *a1 = 1.0 - (v7 + v6);
    a1[1] = v12 + v9;
    a1[2] = v16 - v18;
    a1[4] = v9 - v12;
    a1[5] = 1.0 - (v7 + v5);
    a1[6] = v14 + v19;
    a1[8] = v18 + v16;
    a1[9] = v14 - v19;
    a1[10] = 1.0 - (v6 + v5);
  }
  return result;
}

const char *__cdecl DObjGetBoneName( DObj *dobj, int boneIndex );

/* ---- DObjDumpInfo  0x00480F60 ---- */
void __cdecl DObjDumpInfo( DObj *dobj )
{
    int i;
    int boneBase;

    if ( !dobj ) {
        Com_Printf( "No Dobj\n" );
        return;
    }

    Com_Printf( "\nModels:\n" );
    boneBase = 0;
    for ( i = 0; i < (int)dobj->childCount; ++i ) {
        const XModel *model = DOBJ_CHILD_MODEL( dobj, i );

        Com_Printf( "%d: '%s'\n", boneBase, model->name );
        boneBase += DObjModelPartNames( model )->count;
    }

    Com_Printf( "\nBones:\n" );
    for ( i = 0; i < (int)dobj->partCount; ++i ) {
        Com_Printf( "Bone %d: '%s'\n", i, DObjGetBoneName( dobj, i ) );
    }

    if ( dobj->tracePartRemapHandle ) {
        const unsigned char *pair;

        Com_Printf( "\nPart duplicates:\n" );
        pair = (const unsigned char *)
                   SL_ConvertToString( dobj->tracePartRemapHandle )
               + DOBJ_TRACE_REMAP_PREFIX;

        while ( pair[0] ) {
            int         target     = pair[0] - 1;
            int         source     = pair[1] - 1;
            const char *sourceName = DObjGetBoneName( dobj, source );
            const char *targetName = DObjGetBoneName( dobj, target );

            Com_Printf( "%d ('%s') -> %d ('%s')\n",
                        target, targetName, source, sourceName );
            pair += 2;
        }
    } else {
        Com_Printf( "\nNo part duplicates.\n" );
    }

    Com_Printf( "\n" );
}
#if 0
// DObjDumpInfo: developer dump of a DObj: child models, bone names via DObjGetBoneName, and the part-duplicate remap table.
void __cdecl DObjDumpInfo(DObj *state)
{
  int v2;
  const char ***childRefs;
  const char **v4;
  int partCount;
  int v6;
  const char *v7;
  unsigned __int16 tracePartRemapHandle;
  int v9;
  int v10;
  unsigned __int8 i;
  int v12;
  int v13;
  const char *v14;
  const char *v15;
  char *Format;

  if ( state )
  {
    Com_Printf("\nModels:\n");
    v2 = 0;
    if ( state->childCount )
    {
      childRefs = (const char ***)state->childRefs;
      Format = (char *)state->childCount;
      do
      {
        v4 = *childRefs;
        Com_Printf("%d: '%s'\n", v2, **childRefs);
        v2 += ****(__int16 ****)(*(_DWORD *)v4[1] + 4);
        ++childRefs;
        --Format;
      }
      while ( Format );
    }
    Com_Printf("\nBones:\n");
    partCount = state->partCount;
    v6 = 0;
    if ( state->partCount )
    {
      do
      {
        v7 = (const char *)DObjGetBoneName((int)state, v6);
        Com_Printf("Bone %d: '%s'\n", v6++, v7);
      }
      while ( v6 < partCount );
    }
    if ( state->tracePartRemapHandle )
    {
      Com_Printf("\nPart duplicates:\n");
      tracePartRemapHandle = state->tracePartRemapHandle;
      if ( tracePartRemapHandle )
        v9 = GetRefString_var + 8 * tracePartRemapHandle + 4;
      else
        v9 = 0;
      v10 = v9 + 16;
      for ( i = *(_BYTE *)(v9 + 16); i; v10 += 2 )
      {
        v12 = *(unsigned __int8 *)(v10 + 1) - 1;
        v13 = i - 1;
        v15 = (const char *)DObjGetBoneName((int)state, v12);
        v14 = (const char *)DObjGetBoneName((int)state, v13);
        Com_Printf("%d ('%s') -> %d ('%s')\n", v13, v14, v12, v15);
        i = *(_BYTE *)(v10 + 2);
      }
    }
    else
    {
      Com_Printf("\nNo part duplicates.\n");
    }
    Com_Printf("\n");
  }
  else
  {
    Com_Printf("No Dobj\n");
  }
}
#endif

/* ---- DObjMatrixTransformVector43InPlace  0x004810B0 ----  VERIFIED */
float *__cdecl DObjMatrixTransformVector43InPlace(float *result, float *a2)
{
  double v2;
  double v3;

  v2 = result[8] * a2[2] + result[4] * a2[1] + *a2 * *result + result[12];
  v3 = result[9] * a2[2] + result[5] * a2[1] + result[1] * *a2 + result[13];
  a2[2] = result[10] * a2[2] + result[6] * a2[1] + result[2] * *a2 + result[14];
  *a2 = v2;
  a2[1] = v3;
  return result;
}

/* ---- DObjQuatMultiplyIntoFirst  0x00481110 ----  VERIFIED */
float *__cdecl DObjQuatMultiplyIntoFirst(float *result, float *a2)
{
  double v2;
  double v3;
  double v4;
  double v5;
  float v6;

  v2 = result[3] * *a2 + a2[1] * result[2] + a2[3] * *result - a2[2] * result[1];
  v3 = a2[3] * result[1] - *a2 * result[2] + result[3] * a2[1] + a2[2] * *result;
  v6 = a2[3] * result[2] + result[1] * *a2 - a2[1] * *result + result[3] * a2[2];
  v4 = a2[3] * result[3] - *result * *a2 - result[1] * a2[1];
  v5 = a2[2] * result[2];
  result[2] = v6;
  result[3] = v4 - v5;
  *result = v2;
  result[1] = v3;
  return result;
}

/* ---- DObjQuatMultiplyIntoSecond  0x004811A0 ----  VERIFIED */
float *__cdecl DObjQuatMultiplyIntoSecond(float *result, float *a2)
{
  double v2;
  double v3;
  double v4;
  double v5;
  float v6;

  v2 = a2[3] * *result + result[1] * a2[2] + result[3] * *a2 - result[2] * a2[1];
  v3 = result[3] * a2[1] - *result * a2[2] + a2[3] * result[1] + result[2] * *a2;
  v6 = result[3] * a2[2] + a2[1] * *result - result[1] * *a2 + a2[3] * result[2];
  v4 = result[3] * a2[3] - *a2 * *result - a2[1] * result[1];
  v5 = result[2] * a2[2];
  result[2] = v6;
  result[3] = v4 - v5;
  *result = v2;
  result[1] = v3;
  return result;
}

/* ---- DObjFindPartIndex  0x00481230 ----  VERIFIED */
int __cdecl DObjFindPartIndex( DObj *dobj, unsigned short nameHandle )
{
    int childIndex;
    int partBase = 0;

    for ( childIndex = 0; childIndex < (int)dobj->childCount; ++childIndex ) {
        XModelPartNameTable *names =
            DObjModelPartNames( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        int localPart;

        for ( localPart = names->count - 1; localPart >= 0; --localPart ) {
            if ( nameHandle == names->handles[localPart] )
                return partBase + localPart;
        }
        partBase += names->count;
    }
    return -1;
}

/* ---- DObjMeldModel_m  0x00481290 ----  VERIFIED */
void __cdecl DObjMeldModel_m( DObj *dobj )
{
    unsigned char        remap[2 * 128 + DOBJ_TRACE_REMAP_PREFIX + 1];
    unsigned int         remapOffset;
    int                  partBase;
    int                  childIndex;
    XModelPartNameTable *names;

    Com_Memset( remap, 0, DOBJ_TRACE_REMAP_PREFIX );

    partBase    = DObjModelPartNames( DOBJ_CHILD_MODEL( dobj, 0 ) )->count;
    remapOffset = DOBJ_TRACE_REMAP_PREFIX;

    for ( childIndex = 1; childIndex < (int)dobj->childCount; ++childIndex ) {
        XModel *model = DOBJ_CHILD_MODEL( dobj, childIndex );

        names = DObjModelPartNames( model );

        if ( dobj->childParentPartIndices[childIndex] == DOBJ_CHILD_PARENT_NONE ) {
            int localPart;
            int remappedRootPart = 0;

            for ( localPart = 0; localPart < names->count; ++localPart ) {
                int source = localPart + partBase;
                int target = DObjFindPartIndex( dobj, names->handles[localPart] );

                if ( target != source ) {
                    if ( localPart == 0 ) {
                        remappedRootPart = 1;
                    }
                    remap[remapOffset++] = (unsigned char)( source + 1 );
                    DOBJ_BIT_SET( remap, source );
                    remap[remapOffset++] = (unsigned char)( target + 1 );
                }
            }

            if ( !remappedRootPart ) {
                Com_Printf( "WARNING: Attempting to meld model, but root part "
                            "'%s' of model '%s' not found in model '%s' or any "
                            "of its descendants\n",
                            SL_ConvertToString( names->handles[0] ),
                            model->name,
                            DOBJ_CHILD_MODEL( dobj, 0 )->name );
            }
        }

        partBase += names->count;
    }

    if ( remapOffset > DOBJ_TRACE_REMAP_PREFIX ) {
        remap[remapOffset] = 0;
        dobj->tracePartRemapHandle =
            SL_GetStringOfLen( (const char *)remap, 0, remapOffset + 1,
                               DOBJ_TRACE_REMAP_STRING_TYPE );
    } else {
        dobj->tracePartRemapHandle = (unsigned short)dword_140733C;
    }
}
#if 0
// DObjBuildTracePartRemap (retail name of DObjMeldModel_m): builds the duplicate-bone remap byte string stored in DObj+0x14 so traces can fold melded models onto shared bones.
__int16 __cdecl DObjMeldModel_m(int a1)
{
  int v1;
  void *v2;
  unsigned __int8 v3;
  int v4;
  unsigned int v5;
  int v6;
  bool v7; // zf
  __int16 *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int *v13;
  unsigned int v14;
  unsigned __int16 v15;
  const char *v16;
  bool v17; // cc
  char *v18;
  __int16 result;
  int v20[3]; // [esp+0h] [ebp-2Ch] BYREF
  int v21;
  int v22;
  int v23;
  __int16 *v24;
  int v25;
  int *v26;
  int *v27;
  char v28;

  v1 = a1;
  v2 = alloca(2 * *(unsigned __int8 *)(a1 + 23) + 17);
  v27 = v20;
  Com_Memset(v20, 0, 16);
  v3 = *(_BYTE *)(a1 + 22);
  v25 = ****(__int16 ****)(**(_DWORD **)(*(_DWORD *)(a1 + 24) + 4) + 4);
  v4 = 1;
  v5 = 16;
  v23 = 1;
  if ( v3 <= 1u )
    goto LABEL_21;
  v26 = (int *)(a1 + 28);
  do
  {
    v6 = *v26;
    v7 = *(_BYTE *)(v1 + v4 + 72) == 0xFF;
    v22 = *v26;
    if ( !v7 )
      goto LABEL_18;
    v8 = ***(__int16 ****)(**(_DWORD **)(v6 + 4) + 4);
    v9 = *v8;
    v10 = 0;
    v24 = v8;
    v21 = v9;
    v28 = 0;
    if ( v9 <= 0 )
      goto LABEL_14;
    while ( 1 )
    {
      v11 = DObjFindPartIndex(v1, v8[v10 + 1]);
      v12 = v10 + v25;
      if ( v11 != v10 + v25 )
      {
        if ( !v10 )
          v28 = 1;
        v13 = v27;
        *((_BYTE *)v27 + v5) = v12 + 1;
        *((_BYTE *)v13 + (v12 >> 3)) |= 1 << (v12 & 7);
        v1 = a1;
        v14 = v5 + 1;
        *((_BYTE *)v27 + v14) = v11 + 1;
        v5 = v14 + 1;
      }
      if ( ++v10 >= v21 )
        break;
      v8 = v24;
    }
    if ( !v28 )
    {
      v8 = v24;
LABEL_14:
      v15 = v8[1];
      if ( v15 )
        v16 = (const char *)(GetRefString_var + 8 * v15 + 4);
      else
        v16 = 0;
      Com_Printf(
        "WARNING: Attempting to meld model, but root part '%s' of model '%s' not found in model '%s' or any of its descendants\n",
        v16,
        *(const char **)v22,
        **(const char ***)(v1 + 24));
    }
LABEL_18:
    v25 += ****(__int16 ****)(**(_DWORD **)(v22 + 4) + 4);
    v4 = v23 + 1;
    v17 = ++v23 < *(unsigned __int8 *)(v1 + 22);
    ++v26;
  }
  while ( v17 );
  if ( v5 > 0x10 )
  {
    v18 = (char *)v27;
    *((_BYTE *)v27 + v5) = 0;
    result = SL_GetStringOfLen(v18, 0, v5 + 1);
    *(_WORD *)(v1 + 20) = result;
    return result;
  }
LABEL_21:
  result = dword_140733C;
  *(_WORD *)(v1 + 20) = dword_140733C;
  return result;
}
#endif

/* ---- DObjTraceRemapTarget  no-address ---- */
static int DObjTraceRemapTarget( const unsigned char *traceRemap, int sourcePart )
{
    const unsigned char *pair = traceRemap + DOBJ_TRACE_REMAP_PREFIX;

    while ( sourcePart != (int)pair[0] - 1 ) {
        pair += 2;
    }
    return (int)pair[1] - 1;
}

/* ---- DObjGetHierarchyBits  0x00481440 ----  VERIFIED */
int __cdecl DObjGetHierarchyBits( int boneIndex, DObj *dobj, unsigned int *partBits )
{
    const unsigned char     *traceRemap;
    const unsigned char     *parentPartDeltas;
    XModelPartsData         *parts;
    int                      childBases[8];
    int                      childIndex;
    int                      childEnd;

    partBits[0] = 0;
    partBits[1] = 0;
    partBits[2] = 0;
    partBits[3] = 0;

    if ( !dobj->tracePartRemapHandle ) {
        DObjMeldModel_m( dobj );
    }
    traceRemap = (const unsigned char *)
                     SL_ConvertToString( dobj->tracePartRemapHandle );

    childBases[0] = 0;
    childIndex    = 0;
    parts         = DObjModelParts( DOBJ_CHILD_MODEL( dobj, 0 ) );
    childEnd      = parts->partNameTableSlot->partNameTable->count;

    while ( childEnd <= boneIndex ) {
        ++childIndex;
        if ( childIndex == (int)dobj->childCount ) {
            return childEnd;
        }
        childBases[childIndex] = childEnd;
        parts    = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        childEnd = childBases[childIndex]
                 + parts->partNameTableSlot->partNameTable->count;
    }

    parentPartDeltas = parts->partNameTableSlot->parentPartDeltas;

    for ( ;; ) {
        int localPart = boneIndex - childBases[childIndex];

        for ( ;; ) {
            DOBJ_BIT_SET( partBits, boneIndex );

            if ( DOBJ_BIT_TEST( traceRemap, boneIndex ) ) {
                boneIndex = DObjTraceRemapTarget( traceRemap, boneIndex );
            } else {
                int deltaIndex = localPart - parts->rootPartCount;

                if ( deltaIndex >= 0 ) {
                    boneIndex -= parentPartDeltas[deltaIndex];
                    break;
                }
                boneIndex = dobj->childParentPartIndices[childIndex];
                if ( boneIndex == DOBJ_CHILD_PARENT_NONE ) {
                    return parts->rootPartCount;
                }
            }

            do {
                --childIndex;
                localPart = boneIndex - childBases[childIndex];
            } while ( localPart < 0 );

            parts            = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
            parentPartDeltas = parts->partNameTableSlot->parentPartDeltas;
        }
    }
}
#if 0
// DObjGetHierarchyBits: fills a 128-bit part bitset with the bone plus every ancestor, honouring the trace remap.  Callers: SV_DObjGetHierarchyBits and cgame trap 170.  SERVER PATH.
int __cdecl DObjGetHierarchyBits(int a1, int a2, _DWORD *a3)
{
  int v4;
  unsigned __int16 v5;
  _DWORD *v6;
  int result;
  int *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  _DWORD *v17;
  int v18;
  int i;
  int v20;
  _DWORD v21[8];

  *a3 = 0;
  a3[1] = 0;
  a3[2] = 0;
  a3[3] = 0;
  v4 = *(unsigned __int8 *)(a2 + 22);
  if ( !*(_WORD *)(a2 + 20) )
    DObjMeldModel_m(a2);
  v5 = *(_WORD *)(a2 + 20);
  if ( v5 )
    v18 = GetRefString_var + 8 * v5 + 4;
  else
    v18 = 0;
  v6 = *(_DWORD **)(**(_DWORD **)(*(_DWORD *)(a2 + 24) + 4) + 4);
  result = **(__int16 **)*v6;
  v8 = (int *)(a2 + 24);
  v9 = 0;
  v21[0] = 0;
  v17 = v6;
  if ( result > a1 )
  {
LABEL_9:
    for ( i = *v6 + 4; ; a1 -= *(unsigned __int8 *)(i + v13) )
    {
      v11 = a1 - v21[v9];
      while ( 1 )
      {
        *((_BYTE *)a3 + (a1 >> 3)) |= 1 << (a1 & 7);
        v12 = *(char *)((a1 >> 3) + v18);
        v20 = 1;
        if ( (v12 & (1 << (a1 & 7))) != 0 )
        {
          v14 = v18 + 16;
          if ( a1 != *(unsigned __int8 *)(v18 + 16) - 1 )
          {
            do
            {
              v15 = *(unsigned __int8 *)(v14 + 2);
              v14 += 2;
            }
            while ( a1 != v15 - 1 );
          }
          a1 = *(unsigned __int8 *)(v14 + 1) - 1;
          goto LABEL_18;
        }
        result = *((__int16 *)v17 + 2);
        v13 = v11 - result;
        if ( v13 >= 0 )
          break;
        a1 = *(unsigned __int8 *)(v9 + a2 + 72);
        if ( a1 == 255 )
          return result;
        do
        {
LABEL_18:
          v16 = v21[--v9];
          v11 = a1 - v16;
        }
        while ( a1 - v16 < 0 );
        v17 = *(_DWORD **)(**(_DWORD **)(*(_DWORD *)(a2 + 4 * v9 + 24) + 4) + 4);
        i = *v17 + 4;
      }
    }
  }
  while ( 1 )
  {
    ++v9;
    ++v8;
    if ( v9 == v4 )
      return result;
    v10 = *v8;
    v21[v9] = result;
    v6 = *(_DWORD **)(**(_DWORD **)(v10 + 4) + 4);
    result = v21[v9] + **(__int16 **)*v6;
    v17 = v6;
    if ( result > a1 )
      goto LABEL_9;
  }
}
#endif

/* ---- DObjBackfillParentPartBits  0x004815B0 ----  VERIFIED */
int __cdecl DObjBackfillParentPartBits( DObj *dobj, unsigned char *partBits )
{
    const unsigned char     *traceRemap;
    const unsigned char     *parentPartDeltas;
    XModelPartsData         *parts;
    int                      childBases[8];
    int                      childIndex;
    int                      childEnd;
    int                      boneIndex;

    boneIndex = (int)dobj->partCount - 1;

    if ( !dobj->tracePartRemapHandle ) {
        DObjMeldModel_m( dobj );
    }
    traceRemap = (const unsigned char *)
                     SL_ConvertToString( dobj->tracePartRemapHandle );

    childBases[0] = 0;
    childIndex    = 0;
    parts         = DObjModelParts( DOBJ_CHILD_MODEL( dobj, 0 ) );
    childEnd      = parts->partNameTableSlot->partNameTable->count;

    while ( childEnd <= boneIndex ) {
        ++childIndex;
        childBases[childIndex] = childEnd;
        parts    = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        childEnd = childBases[childIndex]
                 + parts->partNameTableSlot->partNameTable->count;
    }
    parentPartDeltas = parts->partNameTableSlot->parentPartDeltas;

    for ( ;; ) {
        int localPart;
        int parentPart;

        for ( ;; ) {
            localPart = boneIndex - childBases[childIndex];
            if ( localPart >= 0 ) {
                break;
            }
            --childIndex;
            if ( childIndex < 0 ) {
                return 0;
            }
            parts            = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
            parentPartDeltas = parts->partNameTableSlot->parentPartDeltas;
        }

        if ( partBits[boneIndex >> 3] == 0 && ( ( 1 << ( boneIndex & 7 ) ) & 1 ) != 0 ) {
            --boneIndex;
            continue;
        }

        if ( DOBJ_BIT_TEST( traceRemap, boneIndex ) ) {
            parentPart = DObjTraceRemapTarget( traceRemap, boneIndex );
        } else {
            int deltaIndex = localPart - parts->rootPartCount;

            if ( deltaIndex >= 0 ) {
                parentPart = boneIndex - parentPartDeltas[deltaIndex];
            } else {
                parentPart = dobj->childParentPartIndices[childIndex];
                if ( parentPart == DOBJ_CHILD_PARENT_NONE ) {
                    --boneIndex;
                    continue;
                }
            }
        }

        DOBJ_BIT_SET( partBits, parentPart );
        --boneIndex;
    }
}
#if 0
// DObjBackfillParentPartBits: walks bones high-to-low adding each set bone's parent to the bitset, so a partial selection becomes closed under parenthood.
int __cdecl DObjBackfillParentPartBits(int a1, int a2)
{
  int v2;
  int v3;
  unsigned __int16 v4;
  __int16 ***v5;
  int v6;
  int v7;
  int v8;
  int result;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  _DWORD v20[8];

  v2 = 0;
  v3 = *(unsigned __int8 *)(a1 + 23) - 1;
  if ( !*(_WORD *)(a1 + 20) )
    DObjMeldModel_m(a1);
  v4 = *(_WORD *)(a1 + 20);
  if ( v4 )
    v17 = GetRefString_var + 8 * v4 + 4;
  else
    v17 = 0;
  v5 = *(__int16 ****)(**(_DWORD **)(*(_DWORD *)(a1 + 24) + 4) + 4);
  v6 = ***v5;
  v7 = a1 + 24;
  for ( v20[0] = 0; v6 <= v3; v6 = v20[v2] + ***v5 )
  {
    v8 = *(_DWORD *)(v7 + 4);
    ++v2;
    v7 += 4;
    v20[v2] = v6;
    v5 = *(__int16 ****)(**(_DWORD **)(v8 + 4) + 4);
  }
  v18 = (int)(*v5 + 1);
  v16 = a1 + 4 * v2 + 24;
  while ( 1 )
  {
    while ( 1 )
    {
      result = v3 - v20[v2];
      v19 = result;
      if ( result < 0 )
        break;
      v11 = 1 << (v3 & 7);
      if ( ((*(_BYTE *)((v3 >> 3) + a2) == 0) & (unsigned __int8)v11) != 0 )
      {
        --v3;
      }
      else
      {
        if ( (*(char *)((v3 >> 3) + v17) & v11) != 0 )
        {
          v14 = v17 + 16;
          if ( v3 != *(unsigned __int8 *)(v17 + 16) - 1 )
          {
            do
            {
              v15 = *(unsigned __int8 *)(v14 + 2);
              v14 += 2;
            }
            while ( v3 != v15 - 1 );
          }
          v13 = *(unsigned __int8 *)(v14 + 1) - 1;
          goto LABEL_22;
        }
        v12 = *((__int16 *)v5 + 2);
        if ( v19 - v12 >= 0 )
        {
          v13 = v3 - *(unsigned __int8 *)(v18 + v19 - v12);
          goto LABEL_22;
        }
        v13 = *(unsigned __int8 *)(a1 + v2 + 72);
        if ( v13 == 255 )
        {
          --v3;
        }
        else
        {
LABEL_22:
          *(_BYTE *)(a2 + (v13 >> 3)) = (1 << (v13 & 7)) | *(_BYTE *)((v13 >> 3) + a2);
          --v3;
        }
      }
    }
    --v2;
    v10 = v16 - 4;
    v16 -= 4;
    if ( v2 < 0 )
      return result;
    v5 = *(__int16 ****)(**(_DWORD **)(*(_DWORD *)v10 + 4) + 4);
    v18 = (int)(*v5 + 1);
  }
}
#endif

/* ---- DObjSkelIsBoneUpToDate  0x00481720 ----  VERIFIED */
BOOL __fastcall DObjSkelIsBoneUpToDate( int boneIndex, DObj *dobj )
{
    XAnimEvalStorage *storage = (XAnimEvalStorage *)dobj->evalStorage;
    return DOBJ_BIT_TEST( storage->blockedPartBits, boneIndex );
}

/* ---- DObjCalcSkel  0x00481780 ----  VERIFIED */
int __cdecl DObjCalcSkel( const unsigned int *partBits, DObj *dobj )
{
    XAnimEvalStorage    *storage;
    unsigned int        *storageBits;
    unsigned int         blockedOrNotRequested[4];
    unsigned int         buildBits[4];
    unsigned int         skipQuatBits[4];
    const unsigned char *traceRemap;
    const unsigned char *remapPair;
    DObjSkelMat         *matrices;
    DObjSkelMat         *matrix;
    DObjAnimMat         *evalParts;
    DObjAnimMat         *evalPart;
    int                  word;
    int                  childIndex;
    int                  globalPart;
    int                  allBlocked;

    storage     = (XAnimEvalStorage *)dobj->evalStorage;
    storageBits = (unsigned int *)storage->evaluatedPartBits;
    allBlocked  = 1;

    for ( word = 0; word < 4; ++word ) {
        blockedOrNotRequested[word] = storageBits[8 + word] | ~partBits[word];
        if ( blockedOrNotRequested[word] != 0xFFFFFFFFu ) {
            allBlocked = 0;
        }
    }
    if ( allBlocked ) {
        return 0;
    }

    if ( !dobj->tracePartRemapHandle ) {
        DObjMeldModel_m( dobj );
    }
    traceRemap = (const unsigned char *)
                     SL_ConvertToString( dobj->tracePartRemapHandle );

    for ( word = 0; word < 4; ++word ) {
        storageBits[8 + word] |= partBits[word];
        buildBits[word]    = ~blockedOrNotRequested[word] & storageBits[4 + word];
        skipQuatBits[word]  = blockedOrNotRequested[word] | buildBits[word]
                            | ( (const unsigned int *)traceRemap )[word];
    }
    for ( word = 0; word < 4; ++word ) {
        buildBits[word] = ~skipQuatBits[word] | buildBits[word];
    }

    matrices  = storage->basePose;
    evalParts = (DObjAnimMat *)&storage->basePose[dobj->partCount];

    evalPart   = evalParts;
    globalPart = 0;
    remapPair  = traceRemap + DOBJ_TRACE_REMAP_PREFIX;

    for ( childIndex = 0; childIndex < (int)dobj->childCount; ++childIndex ) {
        XModelPartsData     *parts;
        XModelPartNameTable *names;
        const unsigned char *parentPartDeltas;
        unsigned char        parentPart;
        int                  remaining;
        int                  localPart;

        parts            = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        names            = parts->partNameTableSlot->partNameTable;
        parentPartDeltas = parts->partNameTableSlot->parentPartDeltas;
        parentPart       = dobj->childParentPartIndices[childIndex];

        if ( parentPart == DOBJ_CHILD_PARENT_NONE ) {
            for ( remaining = parts->rootPartCount; remaining != 0; --remaining ) {
                if ( !DOBJ_BIT_TEST( buildBits, globalPart )
                  && globalPart == (int)remapPair[0] - 1 ) {
                    int duplicatePart = (int)remapPair[1] - 1;

                    remapPair += 2;
                    if ( !DOBJ_BIT_TEST( blockedOrNotRequested, globalPart ) ) {
                        evalPart->quat[0] = evalParts[duplicatePart].quat[0];
                        evalPart->quat[1] = evalParts[duplicatePart].quat[1];
                        evalPart->quat[2] = evalParts[duplicatePart].quat[2];
                        evalPart->quat[3] = evalParts[duplicatePart].quat[3];
                    }
                }
                ++evalPart;
                ++globalPart;
            }
        } else {
            DObjAnimMat *parentEvalPart = &evalParts[parentPart];

            for ( remaining = parts->rootPartCount; remaining != 0; --remaining ) {
                if ( !DOBJ_BIT_TEST( skipQuatBits, globalPart ) ) {
                    DObjQuatMultiplyIntoFirst( evalPart->quat, parentEvalPart->quat );
                } else if ( DOBJ_BIT_TEST( buildBits, globalPart ) ) {
                    DObjQuatMultiplyIntoSecond( evalPart->quat, parentEvalPart->quat );
                }
                ++evalPart;
                ++globalPart;
            }
        }

        remaining = (int)names->count - (int)parts->rootPartCount;
        for ( localPart = 0; remaining != 0; ++localPart, --remaining ) {
            int parentIndex = globalPart - (int)parentPartDeltas[localPart];

            if ( !DOBJ_BIT_TEST( skipQuatBits, globalPart ) ) {
                DObjQuatMultiplyIntoFirst( evalPart->quat, evalParts[parentIndex].quat );
            } else if ( DOBJ_BIT_TEST( buildBits, globalPart ) ) {
                DObjQuatMultiplyIntoSecond( evalPart->quat, evalParts[parentIndex].quat );
            } else if ( globalPart == (int)remapPair[0] - 1 ) {
                int duplicatePart = (int)remapPair[1] - 1;

                remapPair += 2;
                if ( !DOBJ_BIT_TEST( blockedOrNotRequested, globalPart ) ) {
                    evalPart->quat[0] = evalParts[duplicatePart].quat[0];
                    evalPart->quat[1] = evalParts[duplicatePart].quat[1];
                    evalPart->quat[2] = evalParts[duplicatePart].quat[2];
                    evalPart->quat[3] = evalParts[duplicatePart].quat[3];
                }
            }
            ++evalPart;
            ++globalPart;
        }
    }

    evalPart   = evalParts;
    matrix     = matrices;
    globalPart = 0;
    remapPair  = traceRemap + DOBJ_TRACE_REMAP_PREFIX;

    for ( childIndex = 0; childIndex < (int)dobj->childCount; ++childIndex ) {
        XModelPartsData     *parts;
        XModelPartNameTable *names;
        const unsigned char *parentPartDeltas;
        float              (*baseTranslations)[3];
        unsigned char        parentPart;
        int                  remaining;
        int                  localPart;

        parts            = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        names            = parts->partNameTableSlot->partNameTable;
        parentPartDeltas = parts->partNameTableSlot->parentPartDeltas;
        baseTranslations = parts->baseTranslations;
        parentPart       = dobj->childParentPartIndices[childIndex];

        if ( parentPart == DOBJ_CHILD_PARENT_NONE ) {
            for ( remaining = parts->rootPartCount; remaining != 0; --remaining ) {
                if ( DOBJ_BIT_TEST( buildBits, globalPart ) ) {
                    QuatToMatrix_m( (float *)matrix, evalPart->quat );
                    matrix->origin[0] = evalPart->trans[0];
                    matrix->origin[1] = evalPart->trans[1];
                    matrix->origin[2] = evalPart->trans[2];
                    matrix->pad3C     = 0.0f;
                } else if ( globalPart == (int)remapPair[0] - 1 ) {
                    int duplicatePart = (int)remapPair[1] - 1;

                    remapPair += 2;
                    if ( !DOBJ_BIT_TEST( blockedOrNotRequested, globalPart ) ) {
                        evalPart->trans[0] = evalParts[duplicatePart].trans[0];
                        evalPart->trans[1] = evalParts[duplicatePart].trans[1];
                        evalPart->trans[2] = evalParts[duplicatePart].trans[2];
                        *matrix = matrices[duplicatePart];
                    }
                }
                ++evalPart;
                ++matrix;
                ++globalPart;
            }
        } else {
            DObjSkelMat *parentMatrix = &matrices[parentPart];

            for ( remaining = parts->rootPartCount; remaining != 0; --remaining ) {
                if ( DOBJ_BIT_TEST( buildBits, globalPart ) ) {
                    QuatToMatrix_m( (float *)matrix, evalPart->quat );
                    DObjMatrixTransformVector43InPlace( (float *)parentMatrix, evalPart->trans );
                    matrix->origin[0] = evalPart->trans[0];
                    matrix->origin[1] = evalPart->trans[1];
                    matrix->origin[2] = evalPart->trans[2];
                    matrix->pad3C     = 0.0f;
                }
                ++evalPart;
                ++matrix;
                ++globalPart;
            }
        }

        remaining = (int)names->count - (int)parts->rootPartCount;
        for ( localPart = 0; remaining != 0; ++localPart, --remaining ) {
            if ( DOBJ_BIT_TEST( buildBits, globalPart ) ) {
                int parentIndex = globalPart - (int)parentPartDeltas[localPart];

                QuatToMatrix_m( (float *)matrix, evalPart->quat );
                evalPart->trans[0] += baseTranslations[localPart][0];
                evalPart->trans[1] += baseTranslations[localPart][1];
                evalPart->trans[2] += baseTranslations[localPart][2];
                DObjMatrixTransformVector43InPlace( (float *)&matrices[parentIndex], evalPart->trans );
                matrix->origin[0] = evalPart->trans[0];
                matrix->origin[1] = evalPart->trans[1];
                matrix->origin[2] = evalPart->trans[2];
                matrix->pad3C     = 0.0f;
            } else if ( globalPart == (int)remapPair[0] - 1 ) {
                int duplicatePart = (int)remapPair[1] - 1;

                remapPair += 2;
                if ( !DOBJ_BIT_TEST( blockedOrNotRequested, globalPart ) ) {
                    evalPart->trans[0] = evalParts[duplicatePart].trans[0];
                    evalPart->trans[1] = evalParts[duplicatePart].trans[1];
                    evalPart->trans[2] = evalParts[duplicatePart].trans[2];
                    *matrix = matrices[duplicatePart];
                }
            }
            ++evalPart;
            ++matrix;
            ++globalPart;
        }
    }

    return globalPart;
}
#if 0
// DObjCalcSkel: composes the DObjAnimMat output of DObjCalcAnim down the bone hierarchy into world-space DObjSkelMat base poses in eval storage; the trace functions read exactly this.  Callers: SV_DObjCalcSkel and cgame trap 171.  Arg order is (partBits, state).  SERVER PATH.
int __cdecl DObjCalcSkel(_DWORD *a1, int a2)
{
  _DWORD *v3;
  int v4;
  int v5;
  bool v6;
  int result;
  unsigned __int16 v8;
  _DWORD *v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;
  int v22;
  int v23;
  int v24;
  float *v25;
  float *v26;
  int v27;
  float *v28;
  __int16 ***v29;
  unsigned __int8 v30;
  int v31;
  unsigned __int8 *v32;
  int v33;
  float v34;
  float *v35;
  float *v36;
  int v37;
  unsigned __int8 *v38;
  int v39;
  int v40;
  unsigned __int8 *v41;
  float *v42;
  bool v43; // cc
  float *v44;
  float *v45;
  int v46;
  __int16 ***v47;
  unsigned __int8 v48;
  int v49;
  unsigned __int8 *v50;
  int v51;
  float *v52;
  bool v53; // zf
  float *v54;
  int v55;
  float *v56;
  double v57;
  double v58;
  float *v59;
  int v60;
  int v61;
  double v62;
  double v63;
  unsigned __int8 *v64;
  int v65;
  float *v66;
  unsigned __int8 *v67;
  unsigned __int8 *v68;
  int v69;
  int v70;
  float *v71;
  int v72;
  int v73;
  __int16 ***v74;
  __int16 ***v75;
  unsigned __int8 *v76;
  float *v77;
  int v78;
  int v79;
  int v80;
  int v81;
  int i;
  int v83;
  int v84;
  unsigned __int8 *v85;
  int v86;
  int v87;
  float *v88;
  int v89;
  int v90;
  int v91;
  int v92;
  int v93;
  int v94;
  int v95;
  int v96;
  int v97;
  int v98;
  _DWORD v99[4];

  v3 = *(_DWORD **)(a2 + 4);
  v4 = v3[8] | ~*a1;
  v5 = ~v4;
  v95 = v4;
  v6 = v4 == -1;
  v96 = v3[9] | ~a1[1];
  v89 = ~v96;
  if ( v96 != -1 )
    v6 = 0;
  v97 = v3[10] | ~a1[2];
  v83 = ~v97;
  if ( v97 != -1 )
    v6 = 0;
  v98 = v3[11] | ~a1[3];
  result = ~v98;
  v78 = ~v98;
  if ( v98 != -1 )
    v6 = 0;
  if ( !v6 )
  {
    if ( !*(_WORD *)(a2 + 20) )
      DObjMeldModel_m(a2);
    v8 = *(_WORD *)(a2 + 20);
    if ( v8 )
      v9 = (_DWORD *)(GetRefString_var + 8 * v8 + 4);
    else
      v9 = 0;
    v3[8] |= *a1;
    v10 = v5 & v3[4];
    v11 = v95;
    v85 = (unsigned __int8 *)(v9 + 4);
    v12 = v3[9];
    v13 = v10 | *v9;
    v91 = v10;
    v3[9] = a1[1] | v12;
    v14 = v11 | v13;
    v15 = v3[10];
    v16 = v89 & v3[5] | v9[1];
    v92 = v89 & v3[5];
    v17 = v96 | v16;
    v3[10] = a1[2] | v15;
    v18 = a1[3];
    v19 = v83 & v3[6] | v9[2];
    v93 = v83 & v3[6];
    v20 = v97 | v19;
    v3[11] |= v18;
    v21 = v78 & v3[7];
    v22 = v98 | v21 | v9[3];
    v67 = (unsigned __int8 *)(v9 + 4);
    v99[0] = v14;
    v91 |= ~v14;
    v99[1] = v17;
    v92 |= ~v17;
    v99[2] = v20;
    v23 = *(unsigned __int8 *)(a2 + 23);
    v24 = *(unsigned __int8 *)(a2 + 22);
    v25 = (float *)(v3 + 12);
    v99[3] = v22;
    v26 = &v25[16 * v23];
    v93 |= ~v20;
    v27 = 0;
    v77 = v25;
    v94 = ~v22 | v21;
    v84 = v24;
    v71 = v26;
    v28 = v26;
    v72 = 0;
    if ( v24 )
    {
      v69 = a2 + 24;
      while ( 1 )
      {
        v29 = *(__int16 ****)(**(_DWORD **)(*(_DWORD *)v69 + 4) + 4);
        v30 = *(_BYTE *)(a2 + v72 + 72);
        v74 = v29;
        if ( v30 == 0xFF )
        {
          if ( *((_WORD *)v29 + 2) )
          {
            v79 = *((__int16 *)v29 + 2);
            do
            {
              v31 = 1 << (v27 & 7);
              if ( (*((char *)&v91 + (v27 >> 3)) & v31) == 0 && v27 == *v67 - 1 )
              {
                v32 = v67 + 2;
                v67 += 2;
                if ( (*((char *)&v95 + (v27 >> 3)) & v31) == 0 )
                {
                  v33 = 8 * *(v32 - 1);
                  v34 = v26[v33 - 8];
                  v35 = &v26[v33 - 8];
                  *v28 = v34;
                  v28[1] = v35[1];
                  v28[2] = v35[2];
                  v28[3] = v35[3];
                }
              }
              v28 += 8;
              ++v27;
              --v79;
            }
            while ( v79 );
LABEL_32:
            v29 = v74;
          }
          goto LABEL_33;
        }
        v36 = &v26[8 * v30];
        if ( *((_WORD *)v29 + 2) )
          break;
LABEL_33:
        v38 = (unsigned __int8 *)(*v29 + 1);
        if ( ***v29 != *((__int16 *)v29 + 2) )
        {
          v81 = ***v29 - *((__int16 *)v29 + 2);
          do
          {
            v39 = v27 >> 3;
            v40 = 1 << (v27 & 7);
            if ( (*((char *)v99 + (v27 >> 3)) & v40) != 0 )
            {
              if ( (*((char *)&v91 + v39) & v40) != 0 )
              {
                DObjQuatMultiplyIntoSecond(v28, &v28[-8 * *v38]);
              }
              else if ( v27 == *v67 - 1 )
              {
                v41 = v67 + 2;
                v67 += 2;
                if ( (*((char *)&v95 + v39) & v40) == 0 )
                {
                  v42 = &v71[8 * *(v41 - 1) - 8];
                  *v28 = *v42;
                  v28[1] = v42[1];
                  v28[2] = v42[2];
                  v28[3] = v42[3];
                }
              }
            }
            else
            {
              DObjQuatMultiplyIntoFirst(v28, &v28[-8 * *v38]);
            }
            v28 += 8;
            ++v38;
            ++v27;
            --v81;
          }
          while ( v81 );
          v26 = v71;
        }
        v43 = ++v72 < v84;
        v69 += 4;
        if ( !v43 )
        {
          v24 = v84;
          goto LABEL_46;
        }
      }
      v80 = *((__int16 *)v29 + 2);
      while ( 1 )
      {
        v37 = 1 << (v27 & 7);
        if ( (*((char *)v99 + (v27 >> 3)) & v37) == 0 )
          break;
        if ( (*((char *)&v91 + (v27 >> 3)) & v37) != 0 )
        {
          DObjQuatMultiplyIntoSecond(v28, v36);
          goto LABEL_30;
        }
LABEL_31:
        v28 += 8;
        ++v27;
        if ( !--v80 )
          goto LABEL_32;
      }
      DObjQuatMultiplyIntoFirst(v28, v36);
LABEL_30:
      v26 = v71;
      goto LABEL_31;
    }
LABEL_46:
    v44 = v77;
    v68 = v85;
    v45 = v26;
    result = 0;
    v46 = 0;
    v70 = 0;
    v73 = 0;
    if ( v24 > 0 )
    {
      for ( i = a2 + 24; ; i += 4 )
      {
        v47 = *(__int16 ****)(**(_DWORD **)(*(_DWORD *)i + 4) + 4);
        v48 = *(_BYTE *)(a2 + v46 + 72);
        v75 = v47;
        if ( v48 == 0xFF )
        {
          if ( *((_WORD *)v47 + 2) )
          {
            v86 = *((__int16 *)v47 + 2);
            while ( 1 )
            {
              v49 = 1 << (result & 7);
              if ( (*((char *)&v91 + (result >> 3)) & v49) != 0 )
              {
                QuatToMatrix_m(v44, v45);
                v44[12] = v45[5];
                v44[13] = v45[6];
                v44[14] = v45[7];
                v44[15] = 0.0;
              }
              else
              {
                if ( result != *v68 - 1 )
                  goto LABEL_57;
                v50 = v68 + 2;
                v68 += 2;
                if ( (*((char *)&v95 + (result >> 3)) & v49) != 0 )
                  goto LABEL_57;
                v51 = *(v50 - 1) - 1;
                v52 = &v71[8 * v51];
                v45[5] = v52[5];
                v45[6] = v52[6];
                v45[7] = v52[7];
                qmemcpy(v44, &v77[16 * v51], 0x40u);
              }
              result = v70;
LABEL_57:
              v45 += 8;
              v44 += 16;
              ++result;
              v53 = v86 == 1;
              v70 = result;
              --v86;
              if ( v53 )
                goto LABEL_64;
            }
          }
        }
        else
        {
          v54 = &v77[16 * v48];
          v55 = *((__int16 *)v47 + 2);
          if ( *((_WORD *)v47 + 2) )
          {
            v56 = v45 + 6;
            v87 = v55;
            do
            {
              if ( ((1 << (result & 7)) & *((char *)&v91 + (result >> 3))) != 0 )
              {
                QuatToMatrix_m(v44, v45);
                v57 = v54[8] * v56[1] + v54[4] * *v56 + *v54 * *(v56 - 1) + v54[12];
                v58 = v54[9] * v56[1] + v54[5] * *v56 + v54[1] * *(v56 - 1) + v54[13];
                v56[1] = v54[10] * v56[1] + v54[6] * *v56 + v54[2] * *(v56 - 1) + v54[14];
                *(v56 - 1) = v57;
                *v56 = v58;
                v44[12] = *(v56 - 1);
                v44[13] = *v56;
                result = v70;
                v44[14] = v56[1];
                v44[15] = 0.0;
              }
              v45 += 8;
              v56 += 8;
              v44 += 16;
              ++result;
              v53 = v87 == 1;
              v70 = result;
              --v87;
            }
            while ( !v53 );
LABEL_64:
            v47 = v75;
          }
        }
        v59 = (float *)v47[4];
        v76 = (unsigned __int8 *)(*v47 + 1);
        v88 = v59;
        if ( ***v47 != *((__int16 *)v47 + 2) )
          break;
LABEL_74:
        v46 = v73 + 1;
        v43 = ++v73 < v84;
        if ( !v43 )
          return result;
      }
      v90 = ***v47 - *((__int16 *)v47 + 2);
      while ( 1 )
      {
        v60 = 1 << (result & 7);
        if ( (*((char *)&v91 + (result >> 3)) & v60) != 0 )
        {
          QuatToMatrix_m(v44, v45);
          v45[5] = *v59 + v45[5];
          v45[6] = v59[1] + v45[6];
          v45[7] = v59[2] + v45[7];
          v61 = *v76 << 6;
          v62 = *(float *)((char *)v44 - v61 + 32) * v45[7]
              + *(float *)((char *)v44 - v61 + 16) * v45[6]
              + v45[5] * *(float *)((char *)v44 - v61)
              + *(float *)((char *)v44 - v61 + 48);
          v63 = *(float *)((char *)v44 - v61 + 4) * v45[5]
              + *(float *)((char *)v44 - v61 + 36) * v45[7]
              + *(float *)((char *)v44 - v61 + 20) * v45[6]
              + *(float *)((char *)v44 - v61 + 52);
          v45[7] = *(float *)((char *)v44 - v61 + 8) * v45[5]
                 + *(float *)((char *)v44 - v61 + 40) * v45[7]
                 + *(float *)((char *)v44 - v61 + 24) * v45[6]
                 + *(float *)((char *)v44 - v61 + 56);
          v45[5] = v62;
          v45[6] = v63;
          v44[12] = v45[5];
          v44[13] = v45[6];
          v44[14] = v45[7];
          v44[15] = 0.0;
        }
        else
        {
          if ( result != *v68 - 1 )
            goto LABEL_73;
          v64 = v68 + 2;
          v68 += 2;
          if ( (*((char *)&v95 + (result >> 3)) & v60) != 0 )
            goto LABEL_73;
          v65 = *(v64 - 1) - 1;
          v66 = &v71[8 * v65];
          v45[5] = v66[5];
          v45[6] = v66[6];
          v45[7] = v66[7];
          qmemcpy(v44, &v77[16 * v65], 0x40u);
          v59 = v88;
        }
        result = v70;
LABEL_73:
        v59 += 3;
        v45 += 8;
        v44 += 16;
        ++result;
        v53 = v90 == 1;
        v88 = v59;
        ++v76;
        v70 = result;
        --v90;
        if ( v53 )
          goto LABEL_74;
      }
    }
  }
  return result;
}
#endif

/* ---- DObjBasePoseForChild  0x00481E90 ----  VERIFIED */
DObjSkelMat *__cdecl DObjBasePoseForChild(DObj *dobj, int childIndex)
{
  XAnimEvalStorage *storage = (XAnimEvalStorage *)dobj->evalStorage;

  return &storage->basePose[dobj->childPartBaseIndices[childIndex]];
}

/* ---- DObjCreate  0x00481EA0 ----  VERIFIED */
char __cdecl DObjCreate( unsigned short scrNotifyId, DObj *dobj, const DObjModel *models,
                          unsigned short modelCount, XAnimTree *tree )
{
    unsigned char childIndex;
    int           modelIndex;
    int           partBase;
    int           total;

    dobj->tree                = tree;
    dobj->evalStorage          = 0;
    dobj->skelCacheKey         = 0;
    dobj->scrNotifyId          = scrNotifyId;
    dobj->tracePartRemapHandle = 0;

    if ( tree ) {
        int            nodeCount = tree->sourceTree->nodeCount;
        unsigned char *remapTable = (unsigned char *)tree + 8 + 2 * nodeCount
                                   + tree->partRemapTableSelector * ( 3 * nodeCount + 1 );
        unsigned char  generation = remapTable[2 * nodeCount] + 1;

        dobj->partRemapTable = remapTable;
        if ( remapTable[2 * nodeCount] == (unsigned char)-1 ) {
            generation = 1;
            Com_Memset( &remapTable[2 * nodeCount + 1], 0, nodeCount );
        }
        remapTable[2 * nodeCount] = generation;
        tree->partRemapTableSelector = 1 - tree->partRemapTableSelector;
    } else {
        dobj->partRemapTable = 0;
    }

    if ( !modelCount ) {
        dobj->childCount = 0;
        dobj->partCount  = 0;
        return 0;
    }

    childIndex = 0;
    partBase   = 0;
    total      = 0;

    for ( modelIndex = 0; modelIndex < (int)modelCount; ++modelIndex ) {
        const XModel  *model    = (const XModel *)models[modelIndex].model;
        const char    *tagName  = models[modelIndex].tagName;
        unsigned char  parentPart = DOBJ_CHILD_PARENT_NONE;
        int            found      = 1;

        dobj->childParentPartIndices[childIndex] = DOBJ_CHILD_PARENT_NONE;
        dobj->childModelIndices[childIndex]      = models[modelIndex].negativeModelIndex;
        dobj->childPartBaseIndices[childIndex]   = (unsigned char)partBase;
        dobj->childRefs[childIndex]              = (unsigned int)model;

        if ( modelIndex && tagName && *tagName ) {
            unsigned short tagHandle = SL_FindStringOfLen( tagName, strlen( tagName ) + 1 );

            found = 0;
            if ( tagHandle && childIndex > 0 ) {
                int j;

                for ( j = 0; j < (int)childIndex && !found; ++j ) {
                    XModelPartNameTable *names =
                        DObjModelPartNames( DOBJ_CHILD_MODEL( dobj, j ) );
                    int localPart;

                    for ( localPart = names->count - 1; localPart >= 0; --localPart ) {
                        if ( tagHandle == names->handles[localPart] ) {
                            parentPart = (unsigned char)( dobj->childPartBaseIndices[j] + localPart );
                            found = 1;
                            break;
                        }
                    }
                }
            }
            if ( !found ) {
                Com_Printf( "WARNING: Part '%s' not found in model '%s' or any of "
                            "its descendants\n",
                            tagName, DOBJ_CHILD_MODEL( dobj, 0 )->name );
            } else {
                dobj->childParentPartIndices[childIndex] = parentPart;
            }
        }

        if ( found ) {
            XModelPartNameTable *names = DObjModelPartNames( model );

            if ( partBase + names->count >= 128 )
                Com_Error( ERR_DROP, "\025dobj for xmodel '%s' has more than %d bones",
                           DOBJ_CHILD_MODEL( dobj, 0 )->name, 127 );

            total = partBase + names->count;
            ++childIndex;
        }
        partBase = total;
    }

    dobj->childCount = childIndex;
    dobj->partCount  = (unsigned char)total;
    return (char)total;
}

/* ---- DObjFree  0x004820C0 ----  VERIFIED */
int __cdecl DObjFree( unsigned int releaseTree, DObj *dobj )
{
    XAnimTree      *tree;
    unsigned short  handle;

    tree = dobj->tree;
    if ( tree ) {
        if ( releaseTree ) {
            XAnimClearTree( tree );
        }
        tree->partRemapTableSelector =
            ( dobj->partRemapTable
              != (unsigned char *)tree + 8 + 2 * tree->sourceTree->nodeCount );
        dobj->partRemapTable = 0;
        dobj->tree           = 0;
    }

    handle = dobj->tracePartRemapHandle;
    if ( handle ) {
        if ( handle != (unsigned short)dword_140733C ) {
            const char *text = SL_ConvertToString( handle );

            SL_RemoveRefToStringOfLen(
                handle,
                (unsigned int)strlen( text + DOBJ_TRACE_REMAP_PREFIX )
                    + DOBJ_TRACE_REMAP_PREFIX + 1 );
        }
        dobj->tracePartRemapHandle = 0;
    }
    return 0;
}
#if 0
// DObjFree (coduo DObjDestroyRecord): releases a DObj: drops the trace part-remap string ref and optionally XAnimClearTree's the runtime tree.  Callers: Com_SafeClientDObjFree, Com_ServerDObjFree.
unsigned int __cdecl DObjFree(unsigned int result, int a2)
{
  _DWORD *v2;
  unsigned __int16 v3;
  int v4;

  v2 = *(_DWORD **)a2;
  if ( *(_DWORD *)a2 )
  {
    if ( result )
      XAnimClearTree(v2);
    result = *(_DWORD *)(a2 + 12) != (_DWORD)v2 + 2 * *(_DWORD *)(*v2 + 4) + 8;
    v2[1] = result;
    *(_DWORD *)(a2 + 12) = 0;
    *(_DWORD *)a2 = 0;
  }
  v3 = *(_WORD *)(a2 + 20);
  if ( v3 )
  {
    if ( v3 != (_WORD)dword_140733C )
    {
      v4 = GetRefString_var + 8 * v3;
      result = strlen((const char *)(v4 + 20));
      if ( *(_WORD *)v4 )
      {
        --*(_WORD *)v4;
        *(_WORD *)(a2 + 20) = 0;
        return result;
      }
      result = SL_FreeString(v3, (char *)(v4 + 4), result + 17);
    }
    *(_WORD *)(a2 + 20) = 0;
  }
  return result;
}
#endif

/* ---- DObjEvalStorageSize  0x00482170 ----  VERIFIED */
int __cdecl DObjEvalStorageSize(DObj *dobj)
{
  return (int)sizeof(DObjSkelMat) * dobj->partCount
       + (int)sizeof(DObjAnimMat) * dobj->partCount
       + 48;
}

/* ---- DObjRefreshEvalStorageKey  0x00482180 ----  VERIFIED */
BOOL __cdecl DObjRefreshEvalStorageKey(DObj *dobj, int skelCacheKey)
{
  if ( dobj->skelCacheKey == skelCacheKey )
    return dobj->evalStorage != 0;
  dobj->skelCacheKey    = skelCacheKey;
  dobj->unknownState10  = 0;
  dobj->evalStorage     = 0;
  return 0;
}

/* ---- DObjEvalStorageIsCurrent  0x004821B0 ----  VERIFIED */
BOOL __cdecl DObjEvalStorageIsCurrent(DObj *dobj, int skelCacheKey)
{
  return dobj->skelCacheKey == skelCacheKey && dobj->evalStorage != 0;
}

/* ---- DObjBindEvalStorage  0x004821D0 ----  VERIFIED */
void __cdecl DObjBindEvalStorage(void *storage, DObj *dobj)
{
  XAnimEvalStorage *s = (XAnimEvalStorage *)storage;
  int i;

  dobj->evalStorage = storage;
  for ( i = 0; i < 16; i++ )
  {
    s->evaluatedPartBits[i] = 0;
    s->controlPartBits[i]   = 0;
    s->blockedPartBits[i]   = 0;
  }
}

/* ---- DObjGetChildCount  0x004821F0 ----  VERIFIED */
int __cdecl DObjGetChildCount(DObj *dobj)
{
  return dobj->childCount;
}

/* ---- DObjGetChildRef  0x00482200 ----  VERIFIED */
int __cdecl DObjGetChildRef(int childIndex, DObj *dobj)
{
  return (int)dobj->childRefs[childIndex];
}

/* ---- DObjGetBounds  0x00482210 ----  VERIFIED */
int __cdecl DObjGetBounds( DObj *dobj, float *mins, float *maxs )
{
    const unsigned int *info = (const unsigned int *)DOBJ_CHILD_MODEL( dobj, 0 )->info;

    ( (unsigned int *)mins )[0] = info[19];
    ( (unsigned int *)mins )[1] = info[20];
    ( (unsigned int *)mins )[2] = info[21];
    ( (unsigned int *)maxs )[0] = info[22];
    ( (unsigned int *)maxs )[1] = info[23];
    ( (unsigned int *)maxs )[2] = info[24];
    return info[24];
}

/* ---- DObjEvalStorageSeedOutput  0x00482240 ----  VERIFIED */
DObjAnimMat *__cdecl DObjEvalStorageSeedOutput(DObj *dobj)
{
  XAnimEvalStorage *storage = (XAnimEvalStorage *)dobj->evalStorage;

  return (DObjAnimMat *)&storage->basePose[dobj->partCount];
}

/* ---- DObjBuildPartCollisionTable  0x00482250 ----  VERIFIED */
int __cdecl DObjBuildPartCollisionTable( XModelPartColl **out, DObj *dobj )
{
    int childIndex;

    for ( childIndex = 0; childIndex < (int)dobj->childCount; ++childIndex ) {
        XModelPartsData *parts = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        int               count = parts->partNameTableSlot->partNameTable->count;
        int               localPart;

        for ( localPart = 0; localPart < count; ++localPart ) {
            *out++ = &parts->partCollisions[localPart];
        }
    }
    return (int)dobj;
}

/* ---- DObjMarkRotTransIndex  0x004822B0 ----  VERIFIED */
int __cdecl DObjMarkRotTransIndex(DObj *dobj, const unsigned char *partBits, int boneIndex)
{
  int mask  = 1 << (boneIndex & 7);
  int byteI = boneIndex >> 3;
  XAnimEvalStorage *storage;

  if ( (partBits[byteI] & mask) == 0 )
    return 0;
  storage = (XAnimEvalStorage *)dobj->evalStorage;
  if ( (storage->blockedPartBits[byteI] & mask) != 0 )
    return 0;
  storage->evaluatedPartBits[byteI] |= (unsigned char)mask;
  return 1;
}

/* ---- DObjMarkControlRotTransIndex  0x004822F0 ----  VERIFIED */
int __cdecl DObjMarkControlRotTransIndex(DObj *dobj, const unsigned char *partBits, int boneIndex)
{
  int mask  = 1 << (boneIndex & 7);
  int byteI = boneIndex >> 3;
  XAnimEvalStorage *storage;

  if ( (partBits[byteI] & mask) == 0 )
    return 0;
  storage = (XAnimEvalStorage *)dobj->evalStorage;
  if ( (storage->blockedPartBits[byteI] & mask) != 0 )
    return 0;
  storage->controlPartBits[byteI]   |= (unsigned char)mask;
  storage->evaluatedPartBits[byteI] |= (unsigned char)mask;
  return 1;
}

/* ---- DObjGetNumSurfaces  0x00482330 ----  VERIFIED */
int __cdecl DObjGetNumSurfaces( DObj *dobj, const int *lodIndices )
{
    int childIndex;
    int result = 0;

    for ( childIndex = (int)dobj->childCount - 1; childIndex >= 0; --childIndex ) {
        int lod = lodIndices[childIndex];

        if ( lod >= 0 ) {
            const char *info  = (const char *)DOBJ_CHILD_MODEL( dobj, childIndex )->info;
            int         surfs = *(int *)( info + 4 * ( 5 * lod + 5 ) );

            if ( surfs ) {
                result += *(short *)( *(int *)( surfs + 4 ) + 4 );
            }
        }
    }
    return result;
}

/* ---- DObjGetSurface  0x00482370 ----  VERIFIED */
int __cdecl DObjGetSurface( int childIndex, DObj *dobj, int surfaceIndex, const int *lodIndices )
{
    const char *info     = (const char *)DOBJ_CHILD_MODEL( dobj, childIndex )->info;
    int          lod       = lodIndices[childIndex];
    int          lodRecord = *(int *)( info + 20 * ( lod + 1 ) );
    int          surfsHdr  = *(int *)( lodRecord + 4 );
    int         *surfaces  = *(int **)( surfsHdr + 8 );

    return surfaces[surfaceIndex];
}

/* ---- DObjGetBoneName_m  0x004823A0 ----  VERIFIED */
const char *__cdecl DObjGetBoneName_m( int childIndex, int dobj,
                                       int surfaceIndex, int lodIndices )
{
    unsigned short handle;

    handle = *(unsigned short *)
        ( *(int *)( *(int *)( *(int *)( dobj + 4 * childIndex + 24 ) + 4 )
                    + 20 * *(int *)( lodIndices + 4 * childIndex ) + 16 )
          + 2 * surfaceIndex );

    if ( handle ) {
        return SL_ConvertToString( handle );
    }
    return "DEFAULT";
}
#if 0
// DObjGetSurfaceName (retail name of DObjGetBoneName_m): surface name string for (child, surface, lod), or the literal DEFAULT.  Indexes a LOD surface-name table, not bones.
const char *__cdecl DObjGetBoneName_m(int a1, int a2, int a3, int a4)
{
  unsigned __int16 v4;

  v4 = *(_WORD *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(a2 + 4 * a1 + 24) + 4) + 20 * *(_DWORD *)(a4 + 4 * a1) + 16)
                + 2 * a3);
  if ( v4 )
    return (const char *)(GetRefString_var + 8 * v4 + 4);
  else
    return "DEFAULT";
}
#endif

/* ---- DObjGetSurfaces  0x004823E0 ---- */
typedef struct DObjSurfaceRef_s {
    short childIndex;
    short surfaceIndex;
} DObjSurfaceRef;

/* ---- DObjGetSurfaces  0x004823E0 ----  VERIFIED */
int __cdecl DObjGetSurfaces( unsigned int *partBits, DObj *dobj, DObjSurfaceRef *out,
                        const int *lodIndices )
{
    int childIndex;
    int outIndex = 0;

    Com_Memset( partBits, 0, 16 );

    for ( childIndex = 0; childIndex < (int)dobj->childCount; ++childIndex ) {
        int lod = lodIndices[childIndex];
        const char *info;
        int         surfsField;

        if ( lod < 0 )
            continue;

        info       = (const char *)DOBJ_CHILD_MODEL( dobj, childIndex )->info;
        surfsField = *(int *)( info + 4 * ( 5 * lod + 5 ) );
        if ( !surfsField )
            continue;

        {
            int             boneCount = DObjModelPartNames(
                                            DOBJ_CHILD_MODEL( dobj, childIndex ) )->count;
            int             surfsHdr     = *(int *)( surfsField + 4 );
            int             surfaceCount = *(short *)( surfsHdr + 4 );
            unsigned int  **surfaces     = *(unsigned int ***)( surfsHdr + 8 );
            int             childBaseBit = dobj->childPartBaseIndices[childIndex];
            int             extraWords   = ( boneCount - 1 ) >> 5;
            int             destWord     = childBaseBit >> 5;
            int             shift        = childBaseBit & 0x1F;
            int             i;

            if ( shift ) {
                int carryShift = 32 - shift;

                for ( i = 0; i < surfaceCount; ++i, ++outIndex ) {
                    unsigned int *bits = surfaces[i] + 2;
                    int           k;

                    out[outIndex].childIndex   = (short)childIndex;
                    out[outIndex].surfaceIndex = (short)i;

                    partBits[destWord] |= bits[0] << shift;
                    for ( k = 1; k <= extraWords; ++k ) {
                        partBits[destWord + k] |= ( bits[k] << shift ) | ( bits[k - 1] >> carryShift );
                    }
                    /* The final partial source word may end in the last mask
                     * word. Its carry is then outside the 128-bit destination. */
                    if ( destWord + extraWords + 1 < 4 ) {
                        partBits[destWord + extraWords + 1] |= bits[extraWords] >> carryShift;
                    }
                }
            } else {
                for ( i = 0; i < surfaceCount; ++i, ++outIndex ) {
                    unsigned int *bits = surfaces[i] + 2;
                    int           k;

                    out[outIndex].childIndex   = (short)childIndex;
                    out[outIndex].surfaceIndex = (short)i;

                    for ( k = 0; k <= extraWords; ++k ) {
                        partBits[destWord + k] |= bits[k];
                    }
                }
            }
        }
    }
    return dobj->childCount;
}

/* ---- DObjGetBoneIndex  0x004825E0 ----  VERIFIED */
int __cdecl DObjGetBoneIndex( const char *tagName, DObj *dobj )
{
    unsigned short handle = SL_FindLowercaseString( tagName );

    if ( handle )
        return DObjFindPartIndex( dobj, handle );
    return -1;
}

/* ---- DObjGetBoneName  0x00482600 ----  VERIFIED */
const char *__cdecl DObjGetBoneName( DObj *dobj, int boneIndex )
{
    XModelPartNameTable *names;
    int                  base;
    int                  childIndex;

    if ( !dobj->childCount ) {
        return 0;
    }

    base       = 0;
    childIndex = 0;
    for ( ;; ) {
        names = DObjModelPartNames( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        if ( boneIndex - base < (int)names->count ) {
            break;
        }
        base += names->count;
        if ( ++childIndex >= (int)dobj->childCount ) {
            return 0;
        }
    }

    return SL_ConvertToString( names->handles[boneIndex - base] );
}
#if 0
// DObjGetBoneName: global bone index -> bone-name string, walking child models to find the owning one.  Callers: DObjDumpInfo, cgame trap 175.
int __cdecl DObjGetBoneName(int a1, int a2)
{
  int v2;
  int v3;
  int i;
  __int16 *v5;
  unsigned __int16 v7;

  v2 = 0;
  v3 = 0;
  if ( !*(_BYTE *)(a1 + 22) )
    return 0;
  for ( i = a1 + 24; ; i += 4 )
  {
    v5 = ***(__int16 ****)(**(_DWORD **)(*(_DWORD *)i + 4) + 4);
    if ( a2 - v2 < *v5 )
      break;
    v2 += *v5;
    if ( ++v3 >= *(unsigned __int8 *)(a1 + 22) )
      return 0;
  }
  v7 = v5[a2 - v2 + 1];
  if ( v7 )
    return GetRefString_var + 8 * v7 + 4;
  else
    return 0;
}
#endif

/* ---- DObjGetTree  0x00482660 ----  VERIFIED */
XAnimTree *__cdecl DObjGetTree(DObj *dobj)
{
  return dobj->tree;
}

/* ---- DObjHasDefaultModel  0x00482670 ---- */
// DObjHasDefaultModel: true if any child model failed to load and is showing the DEFAULT model (XModel->info compared against the default XModelInfo at 0x00A9CCCC).
extern char xmodel_defaultCollision[];

/* ---- DObjHasDefaultModel  0x00482670 ----  VERIFIED */
int __cdecl DObjHasDefaultModel( DObj *dobj )
{
    int childIndex;

    for ( childIndex = (int)dobj->childCount - 1; childIndex >= 0; --childIndex ) {
        if ( (void *)DOBJ_CHILD_MODEL( dobj, childIndex )->info == (void *)xmodel_defaultCollision )
            return 1;
    }
    return 0;
}

/* ---- DObjNumBones  0x004826A0 ----  VERIFIED */
int __cdecl DObjNumBones(DObj *dobj)
{
  return dobj->partCount;
}

/* ---- DObjMatrixTransformVector43  0x004826B0 ----  VERIFIED */
float *__cdecl DObjMatrixTransformVector43(float *result, float *a2, float *a3)
{
  *a2 = result[8] * a3[2] + result[4] * a3[1] + *a3 * *result + result[12];
  a2[1] = result[1] * *a3 + result[9] * a3[2] + result[5] * a3[1] + result[13];
  a2[2] = result[2] * *a3 + result[10] * a3[2] + result[6] * a3[1] + result[14];
  return result;
}

/* ---- DObjMatrixInverseTransformVector43  0x00482700 ----  VERIFIED */
float *__cdecl DObjMatrixInverseTransformVector43(float *result, float *a2, float *a3)
{
  double v3;
  double v4;
  double v5;

  v3 = *a2 - result[12];
  v4 = a2[1] - result[13];
  v5 = a2[2] - result[14];
  *a3 = v5 * result[2] + v4 * result[1] + v3 * *result;
  a3[1] = v5 * result[6] + v4 * result[5] + v3 * result[4];
  a3[2] = v5 * result[10] + v4 * result[9] + v3 * result[8];
  return result;
}

typedef struct DObjTrace_s {
    float          fraction;              /* +0x00 */
    int            surfaceFlags;          /* +0x04 */
    float          normal[3];             /* +0x08  model space */
    unsigned short partName;              /* +0x14 */
    unsigned short partGroup;             /* +0x16 */
    unsigned char  startsolid;            /* +0x18 */
    unsigned char  allsolid;              /* +0x19 */
    unsigned char  pad1A[2];              /* +0x1A */
} DObjTrace;

/* ---- DObjTraceParts  0x00482750 ----  VERIFIED */
int __cdecl DObjTraceParts( DObjTrace *trace, DObj *dobj, const float *start,
                        const float *end, const unsigned char *partState )
{
    unsigned short       remappedPartStateIndices[128];
    float                delta[3];
    float                invLenSq;
    const unsigned char *remapPair;
    DObjSkelMat         *basePose;
    unsigned int         bestPriority;
    int                  globalPart;
    int                  childIndex;
    int                  hitAxis;
    float                hitSign;

    delta[0] = end[0] - start[0];
    delta[1] = end[1] - start[1];
    delta[2] = end[2] - start[2];
    invLenSq = 1.0f / ( delta[0] * delta[0] + delta[1] * delta[1]
                      + delta[2] * delta[2] );

    basePose = DObjBasePoseForChild( dobj, 0 );

    trace->surfaceFlags = 0;
    trace->startsolid   = 0;
    trace->allsolid     = 0;
    trace->partName     = 0;
    trace->partGroup    = 0;
    trace->normal[0]    = 0.0f;
    trace->normal[1]    = 0.0f;
    trace->normal[2]    = 0.0f;

    if ( !dobj->tracePartRemapHandle ) {
        DObjMeldModel_m( dobj );
    }
    remapPair = (const unsigned char *)
                    SL_ConvertToString( dobj->tracePartRemapHandle )
              + DOBJ_TRACE_REMAP_PREFIX;

    bestPriority = 2;
    globalPart   = 0;
    hitAxis      = -1;
    hitSign      = 0.0f;

    for ( childIndex = 0; childIndex < (int)dobj->childCount; ++childIndex ) {
        XModelPartsData     *parts;
        XModelPartNameTable *names;
        XModelPartColl      *partColls;
        const unsigned char *partStateIndices;
        const unsigned char *parentPartDeltas;
        int                  rootPartCount;
        int                  localPart;

        parts            = DObjModelParts( DOBJ_CHILD_MODEL( dobj, childIndex ) );
        names            = parts->partNameTableSlot->partNameTable;
        parentPartDeltas = parts->partNameTableSlot->parentPartDeltas;
        partColls        = parts->partCollisions;
        partStateIndices = parts->partStateIndices;
        rootPartCount    = parts->rootPartCount;

        for ( localPart = 0; localPart < (int)names->count;
              ++localPart, ++globalPart, ++basePose ) {
            unsigned short  partStateIndex;
            unsigned int    partPriority;
            XModelPartColl *part;
            float           center[3];
            float           startToCenter[3];
            float           projection;
            float           distanceSq;
            float           radiusDelta;
            float           localStart[3];
            float           localEnd[3];
            float           enter;
            float           exitFrac;
            float           sign;
            const float    *bounds;
            int             startInside;
            int             endInside;
            int             axis;
            int             missed;

            partStateIndex = partStateIndices[localPart];
            partPriority   = partState[partStateIndex];

            if ( globalPart == (int)remapPair[0] - 1 ) {
                const unsigned char *pair = remapPair;

                remapPair += 2;
                if ( partPriority == 1 ) {
                    partStateIndex = remappedPartStateIndices[pair[1] - 1];
                    partPriority   = partState[partStateIndex];
                }
            } else if ( partPriority == 1 ) {
                if ( localPart >= rootPartCount ) {
                    partStateIndex = remappedPartStateIndices[
                        globalPart
                        - (int)parentPartDeltas[localPart - rootPartCount]];
                } else if ( dobj->childParentPartIndices[childIndex]
                            == DOBJ_CHILD_PARENT_NONE ) {
                    partStateIndex = 0;
                } else {
                    partStateIndex = remappedPartStateIndices[
                        dobj->childParentPartIndices[childIndex]];
                }
                partPriority = partState[partStateIndex];
            }

            remappedPartStateIndices[globalPart] = partStateIndex;

            part = &partColls[localPart];
            if ( part->radiusSq == 0.0f || bestPriority > partPriority ) {
                continue;
            }

            DObjMatrixTransformVector43( (float *)basePose, center, part->center );
            startToCenter[0] = start[0] - center[0];
            startToCenter[1] = start[1] - center[1];
            startToCenter[2] = start[2] - center[2];
            projection = -( startToCenter[0] * delta[0]
                          + startToCenter[1] * delta[1]
                          + startToCenter[2] * delta[2] ) * invLenSq;

            if ( projection < 1.0f ) {
                if ( projection > 0.0f ) {
                    float nearest[3];

                    nearest[0] = startToCenter[0] + delta[0] * projection;
                    nearest[1] = startToCenter[1] + delta[1] * projection;
                    nearest[2] = startToCenter[2] + delta[2] * projection;
                    distanceSq = nearest[0] * nearest[0]
                               + nearest[1] * nearest[1]
                               + nearest[2] * nearest[2];
                } else {
                    distanceSq = startToCenter[0] * startToCenter[0]
                               + startToCenter[1] * startToCenter[1]
                               + startToCenter[2] * startToCenter[2];
                }
            } else {
                float endToCenter[3];

                endToCenter[0] = end[0] - center[0];
                endToCenter[1] = end[1] - center[1];
                endToCenter[2] = end[2] - center[2];
                distanceSq = endToCenter[0] * endToCenter[0]
                           + endToCenter[1] * endToCenter[1]
                           + endToCenter[2] * endToCenter[2];
            }

            radiusDelta = part->radiusSq - distanceSq;
            if ( radiusDelta <= 0.0f ) {
                continue;
            }
            if ( bestPriority == partPriority
              && projection - (float)sqrt( (double)( radiusDelta * invLenSq ) )
                     >= trace->fraction ) {
                continue;
            }

            DObjMatrixInverseTransformVector43( (float *)basePose, (float *)start, localStart );
            DObjMatrixInverseTransformVector43( (float *)basePose, (float *)end, localEnd );

            enter       = 0.0f;
            exitFrac    = trace->fraction;
            startInside = 1;
            endInside   = 1;
            sign        = -1.0f;
            bounds      = part->mins;
            missed      = 0;

            for ( ;; ) {
                for ( axis = 0; axis < 3; ++axis ) {
                    float startDist = ( localStart[axis] - bounds[axis] ) * sign;
                    float endDist   = ( localEnd[axis]   - bounds[axis] ) * sign;
                    float denom;

                    if ( startDist > 0.0f ) {
                        if ( endDist > 0.0f ) {
                            missed = 1;
                            break;
                        }
                        startInside = 0;
                        denom = startDist - endDist;
                        if ( enter * denom < startDist ) {
                            enter = startDist / denom;
                            if ( enter >= exitFrac ) {
                                missed = 1;
                                break;
                            }
                            hitSign = sign;
                            hitAxis = axis;
                        }
                    } else if ( endDist > 0.0f ) {
                        endInside = 0;
                        denom = startDist - endDist;
                        if ( exitFrac * denom < startDist ) {
                            exitFrac = startDist / denom;
                            if ( enter >= exitFrac ) {
                                missed = 1;
                                break;
                            }
                        }
                    }
                }
                if ( missed || sign == 1.0f ) {
                    break;
                }
                sign   = 1.0f;
                bounds = part->maxs;
            }
            if ( missed ) {
                continue;
            }

            if ( startInside ) {
                trace->startsolid = 1;
                if ( endInside ) {
                    trace->allsolid  = 1;
                    trace->fraction  = 0.0f;
                    trace->partName  = names->handles[localPart];
                    trace->partGroup = partStateIndex;
                    trace->normal[0] = 0.0f;
                    trace->normal[1] = 0.0f;
                    trace->normal[2] = 0.0f;
                    return 0;
                }
                continue;
            }

            if ( bestPriority == partPriority ) {
                if ( enter >= trace->fraction ) {
                    continue;
                }
            } else {
                bestPriority = partPriority;
            }

            trace->fraction  = enter;
            trace->partName  = names->handles[localPart];
            trace->partGroup = partStateIndex;
            trace->normal[0] = hitSign * basePose->axis[hitAxis][0];
            trace->normal[1] = hitSign * basePose->axis[hitAxis][1];
            trace->normal[2] = hitSign * basePose->axis[hitAxis][2];
        }
    }

    return childIndex;
}
#if 0
// DObjTraceParts: per-bone trace against the animated skeleton using a caller-supplied part-state filter (sv_world_mp.c passes the pointer from cmPointTraceWork+0x58).  SERVER PATH -- this is CoD1 hit detection.
char __cdecl DObjTraceParts(int a1, int a2, float *a3, float *a4, int a5)
{
  int v5;
  double v6;
  float *v7;
  unsigned __int16 v8;
  int v9;
  char result;
  __int16 ***v11;
  __int16 *v12;
  int v13;
  unsigned __int16 v14;
  unsigned int v15;
  unsigned __int8 *v16;
  int v17;
  __int16 **v18;
  double v19;
  float *v20;
  double v21;
  double v22;
  double v23;
  double v24;
  double v25;
  double v26;
  double v27;
  double v28;
  char v29;
  double v30;
  double v31;
  double v32;
  double v33;
  double v34;
  double v35;
  int i;
  double v37;
  char v38;
  bool v39; // c0
  bool v40; // c2
  bool v41; // c3
  char v42;
  bool v43; // c0
  bool v44; // c3
  double v45;
  double v46;
  float *v47;
  bool v48; // cc
  float v49;
  float v50;
  char v51;
  float v52;
  int v53;
  unsigned int v54;
  int v55;
  float v56;
  float v57;
  float v58;
  float v59;
  float v60;
  unsigned __int8 *v61;
  unsigned int v62;
  float v63;
  float v64;
  float v65;
  _BYTE *v66;
  int v67;
  _WORD *v68;
  int v69;
  int v70;
  float v71;
  int v72;
  float v73;
  int v74;
  float v75;
  float v76;
  __int16 *v77;
  __int16 ***v78;
  float v79[3];
  float v80[2];
  float v81[66];
  unsigned int retaddr;

  LODWORD(v81[65]) = retaddr ^ _security_cookie;
  v5 = a2;
  v63 = *a4 - *a3;
  v62 = 2;
  v64 = a4[1] - a3[1];
  v6 = a4[2] - a3[2];
  v7 = (float *)((*(unsigned __int8 *)(a2 + 80) << 6) + *(_DWORD *)(a2 + 4) + 48);
  v65 = v6;
  *(_DWORD *)(a1 + 4) = 0;
  *(_BYTE *)(a1 + 24) = 0;
  *(_BYTE *)(a1 + 25) = 0;
  *(_WORD *)(a1 + 20) = 0;
  *(_WORD *)(a1 + 22) = 0;
  *(_DWORD *)(a1 + 16) = 0;
  *(_DWORD *)(a1 + 12) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  v8 = *(_WORD *)(a2 + 20);
  v71 = 1.0 / (v6 * v65 + v64 * v64 + v63 * v63);
  if ( v8 )
    v9 = GetRefString_var + 8 * v8 + 4;
  else
    v9 = 0;
  v61 = (unsigned __int8 *)(v9 + 16);
  result = *(_BYTE *)(a2 + 22);
  v55 = 0;
  v70 = -1;
  v57 = 0.0;
  v72 = 0;
  if ( !result )
    return result;
  v66 = (_BYTE *)(a2 + 72);
  v67 = a2 + 24;
  do
  {
    v11 = *(__int16 ****)(**(_DWORD **)(*(_DWORD *)v67 + 4) + 4);
    v12 = **v11;
    v13 = 0;
    v78 = v11;
    v77 = v12;
    v74 = *v12;
    v53 = 0;
    if ( v74 <= 0 )
      goto LABEL_55;
    v69 = 0;
    v68 = v12 + 1;
    while ( 1 )
    {
      v14 = *((unsigned __int8 *)v11[5] + v13);
      v15 = *(unsigned __int8 *)(*((unsigned __int8 *)v11[5] + v13) + a5);
      v54 = v15;
      if ( v55 == *v61 - 1 )
      {
        v16 = v61 + 2;
        v61 += 2;
        if ( v15 == 1 )
        {
          v14 = *((_WORD *)v81 + *(v16 - 1) + 1);
LABEL_18:
          v54 = *(unsigned __int8 *)(v14 + a5);
          v15 = v54;
        }
      }
      else if ( v15 == 1 )
      {
        v17 = *((__int16 *)v11 + 2);
        if ( v13 >= v17 )
        {
          v14 = *((_WORD *)&v81[1] + v55 - *((unsigned __int8 *)*v11 + v13 - v17 + 4));
        }
        else if ( *v66 == 0xFF )
        {
          v14 = 0;
        }
        else
        {
          v14 = *((_WORD *)&v81[1] + (unsigned __int8)*v66);
        }
        goto LABEL_18;
      }
      v18 = v11[2];
      *((_WORD *)&v81[1] + v55) = v14;
      v19 = *(float *)&v18[v69 + 9];
      v20 = (float *)&v18[v69];
      if ( (v19 == 0.0) | __UNORDERED__(v19, 0.0) || v62 > v15 )
        goto LABEL_53;
      v21 = v20[6] * *v7 + v20[8] * v7[8] + v7[4] * v20[7] + v7[12];
      v75 = v20[8] * v7[9] + v20[7] * v7[5] + v20[6] * v7[1] + v7[13];
      v76 = v20[8] * v7[10] + v20[7] * v7[6] + v20[6] * v7[2] + v7[14];
      v58 = *a3 - v21;
      v59 = a3[1] - v75;
      v22 = a3[2] - v76;
      v60 = v22;
      v23 = -((v22 * v65 + v59 * v64 + v58 * v63) * v71);
      v49 = v23;
      if ( !((v23 < 1.0) | __UNORDERED__(v23, 1.0)) )
      {
        v24 = *a4 - v21;
        v25 = a4[1] - v75;
        v26 = a4[2] - v76;
        goto LABEL_26;
      }
      if ( v49 > 0.0 )
      {
        v24 = v49 * v63 + v58;
        v25 = v49 * v64 + v59;
        v26 = v49 * v65 + v60;
LABEL_26:
        v27 = v26 * v26 + v25 * v25 + v24 * v24;
        goto LABEL_27;
      }
      v27 = v60 * v60 + v59 * v59 + v58 * v58;
LABEL_27:
      v73 = v20[9] - v27;
      if ( v73 > 0.0 && (v62 != v15 || v49 - sqrt(v73 * v71) < *(float *)a1) )
      {
        v52 = 0.0;
        v28 = *a3 - v7[12];
        v51 = 1;
        v29 = 1;
        v30 = a3[1] - v7[13];
        v31 = a3[2] - v7[14];
        v79[0] = v28 * *v7 + v30 * v7[1] + v31 * v7[2];
        v79[1] = v31 * v7[6] + v30 * v7[5] + v28 * v7[4];
        v79[2] = v31 * v7[10] + v30 * v7[9] + v28 * v7[8];
        v32 = *a4 - v7[12];
        v33 = a4[1] - v7[13];
        v34 = a4[2] - v7[14];
        v56 = *(float *)a1;
        v80[0] = v32 * *v7 + v33 * v7[1] + v34 * v7[2];
        v80[1] = v34 * v7[6] + v33 * v7[5] + v32 * v7[4];
        v81[0] = v34 * v7[10] + v33 * v7[9] + v32 * v7[8];
        v35 = -1.0;
        while ( 2 )
        {
          for ( i = 0; i < 3; ++i )
          {
            v50 = (v79[i] - v20[i]) * v35;
            v37 = (v80[i] - v20[i]) * v35;
            v39 = v50 < 0.0;
            v40 = __UNORDERED__(v50, 0.0);
            v41 = v50 == 0.0;
            v42 = v38;
            v43 = v37 < 0.0;
            v44 = v37 == 0.0;
            if ( (v42 & 0x41) != 0 )
            {
              if ( !v43 && !v44 )
              {
                v46 = v50 - v37;
                v29 = 0;
                if ( (v56 * v46 < v50) | __UNORDERED__(v56 * v46, v50) )
                {
                  v56 = v50 / v46;
                  if ( v52 >= (double)v56 )
                    goto LABEL_52;
                }
              }
            }
            else
            {
              if ( !v43 && !v44 )
                goto LABEL_52;
              v45 = v50 - v37;
              v51 = 0;
              if ( (v52 * v45 < v50) | __UNORDERED__(v52 * v45, v50) )
              {
                v52 = v50 / v45;
                if ( v52 >= (double)v56 )
                  goto LABEL_52;
                v57 = v35;
                v70 = i;
              }
            }
          }
          if ( !((v35 == 1.0) | __UNORDERED__(v35, 1.0)) )
          {
            v35 = 1.0;
            v20 += 3;
            continue;
          }
          break;
        }
        if ( v51 )
        {
          *(_BYTE *)(a1 + 24) = 1;
          if ( v29 )
          {
            result = 0;
            *(_BYTE *)(a1 + 25) = 1;
            *(_DWORD *)a1 = 0;
            *(_WORD *)(a1 + 20) = v77[v53 + 1];
            *(_WORD *)(a1 + 22) = v14;
            *(_DWORD *)(a1 + 16) = 0;
            *(_DWORD *)(a1 + 12) = 0;
            *(_DWORD *)(a1 + 8) = 0;
            return result;
          }
          goto LABEL_52;
        }
        if ( v62 == v54 )
        {
          if ( v52 < (double)*(float *)a1 )
          {
LABEL_51:
            *(float *)a1 = v52;
            v47 = &v7[4 * v70];
            *(_WORD *)(a1 + 20) = *v68;
            *(_WORD *)(a1 + 22) = v14;
            *(float *)(a1 + 8) = v57 * *v47;
            *(float *)(a1 + 12) = v57 * v47[1];
            *(float *)(a1 + 16) = v57 * v47[2];
          }
LABEL_52:
          v13 = v53;
          goto LABEL_53;
        }
        v62 = v54;
        goto LABEL_51;
      }
LABEL_53:
      ++v13;
      v69 += 10;
      v7 += 16;
      v53 = v13;
      ++v68;
      ++v55;
      if ( v13 >= v74 )
        break;
      v11 = v78;
    }
    v5 = a2;
LABEL_55:
    v67 += 4;
    result = v72 + 1;
    v48 = ++v72 < *(unsigned __int8 *)(v5 + 22);
    ++v66;
  }
  while ( v48 );
  return result;
}
#endif

/* ---- DObjTraceModelParts  0x00482DC0 ---- */
// DObjTraceModelParts: content-mask trace across every child model, advancing the base pose by XModelNumBones per child; records the hit bone name.  sv_world_mp.c passes contentMask from cmPointTraceWork+0x50.  SERVER PATH.

typedef struct DObjTraceWork_s {
    float          fraction;              /* +0x00  seeded, narrowed by callee */
    float          unk04[3];              /* +0x04  untouched on this path */
    float          normal[3];             /* +0x10  hit normal, model space */
    int            unk1C;                 /* +0x1C  -> svDObjTrace_t.surfaceFlags */
    int            unk20;                 /* +0x20  written, never read here */
    unsigned char  unk24[12];             /* +0x24  callee clears +0x2E/+0x2F */
} DObjTraceWork;

/* ---- DObjTraceModelParts  0x00482DC0 ----  VERIFIED */
int __cdecl DObjTraceModelParts( DObjTrace *trace, DObj *dobj, float *a3, float *a4, int a5 )
{
  DObjTraceWork  work;
  unsigned char *basePose;
  __int16       *partNames;
  int            i;
  int            hit;

  basePose = (unsigned char *)dobj->evalStorage + 48
             + ( dobj->childPartBaseIndices[0] << 6 );

  work.fraction      = trace->fraction;
  trace->partName    = 0;
  trace->partGroup   = 0;
  trace->startsolid  = 0;
  trace->allsolid    = 0;
  work.normal[0] = 0.0f;
  work.normal[1] = 0.0f;
  work.normal[2] = 0.0f;
  work.unk1C     = 0;

  for ( i = 0; i < (int)dobj->childCount; i++ )
  {
    partNames = (__int16 *)DObjModelPartNames( DOBJ_CHILD_MODEL( dobj, i ) );
    hit = XModelTraceLine( (int)dobj->childRefs[i], (int)&work,
                      (const float *)basePose, a3, a4, a5 );
    if ( hit >= 0 )
      trace->partName = partNames[hit + 1];
    basePose += partNames[0] << 6;
  }

  trace->fraction     = work.fraction;
  trace->surfaceFlags = work.unk1C;
  trace->normal[0]    = work.normal[0];
  trace->normal[1]    = work.normal[1];
  trace->normal[2]    = work.normal[2];
  return *(int *)&work.normal[1];
}

/* ---- DObjGetLodForDist_m  0x00482EA0 ----  VERIFIED */
int __cdecl DObjGetLodForDist_m(int childIndex, DObj *dobj, float distance)
{
  return XModelGetLodForDist((const void *)dobj->childRefs[childIndex], distance);
}

/* ---- DObjHasContents  0x00482EB0 ----  VERIFIED */
int __cdecl DObjHasContents( DObj *dobj, int contentsMask )
{
    int childIndex;

    for ( childIndex = 0; childIndex < (int)dobj->childCount; ++childIndex ) {
        const char *info = (const char *)DOBJ_CHILD_MODEL( dobj, childIndex )->info;

        if ( contentsMask & *(int *)( info + 72 ) )
            return 1;
    }
    return 0;
}

/* ---- XAnimGetPoolHighWaterBytes  0x00482EE0 ----  VERIFIED */
int XAnimGetPoolHighWaterBytes()
{
  return 68 * xanim_poolHighWaterCount;
}

/* ---- XAnimGetPoolUsedBytes  0x00482EF0 ----  VERIFIED */
int XAnimGetPoolUsedBytes()
{
  return 68 * xanim_poolUsedCount;
}

/* ---- XAnimInit  0x00482F00 ----  VERIFIED */
__int16 XAnimInit()
{
    __int16 result;
    int     node;

    for ( node = 0; node < 2048; ++node ) {
        xanim_pool[node].freePrev = (unsigned short)( ( node + 2047 ) % 2048 );
        xanim_pool[node].freeNext = (unsigned short)( ( node + 1 ) % 2048 );
    }

    xanim_pool[0].states[0].time          = 0.0f;
    xanim_pool[0].states[0].oldTime       = 0.0f;
    xanim_pool[0].states[0].cycleCount    = 0;
    xanim_pool[0].states[0].oldCycleCount = 0;
    xanim_pool[0].states[1].time          = 0.0f;
    xanim_pool[0].states[1].oldTime       = 0.0f;
    xanim_pool[0].states[1].cycleCount    = 0;
    xanim_pool[0].states[1].oldCycleCount = 0;

    result = SL_GetStringOfLen( "end", 0, (unsigned int)strlen( "end" ) + 1, 3 );
    LOWORD(xanim_endNotifyStringHandle) = result;

    xanim_poolUsedCount = 1;
    xanim_poolHighWaterCount = 1;

    return result;
}
#if 0
// XAnimInit: threads the 2048-entry (68-byte stride) XAnimInfo free list, clears the sentinel node, and interns the "end" notify string.
__int16 XAnimInit()
{
  int v0;
  __int16 *v1;
  int v2;
  __int16 result;

  v0 = 1;
  v1 = word_A7A62A;
  v2 = 2048;
  do
  {
    *(v1 - 1) = (v0 + 2046) % 2048;
    *v1 = v0 % 2048;
    ++v0;
    v1 += 34;
    --v2;
  }
  while ( v2 );
  dword_A7A62C[0] = 0;
  dword_A7A630[0] = 0;
  dword_A7A648[0] = 0;
  dword_A7A64C = 0;
  word_A7A634 = 0;
  word_A7A636 = 0;
  word_A7A650 = 0;
  word_A7A652 = 0;
  /* 0x0055A73C is the "end" string itself, not a pointer to it.
   * 0x00482F9D pushes type 3. */
  result = SL_GetStringOfLen("end", 0, strlen("end") + 1, 3);
  LOWORD(xanim_endNotifyStringHandle) = result;
  xanim_poolUsedCount = 1;
  xanim_poolHighWaterCount = 1;
  return result;
}
#endif

/* ---- XAnimShutdown  0x00482FD0 ----  VERIFIED */
__int16 XAnimShutdown()
{
    unsigned short handle = (unsigned short)xanim_endNotifyStringHandle;

    if ( handle ) {
        const char *text = SL_ConvertToString( handle );

        SL_RemoveRefToStringOfLen( handle, (unsigned int)strlen( text ) + 1 );
        LOWORD(xanim_endNotifyStringHandle) = 0;
    }
    return 0;
}
#if 0
// XAnimShutdown: releases the end-notify string handle (0x01407338).
__int16 XAnimShutdown()
{
  int v0;

  LOWORD(v0) = xanim_endNotifyStringHandle;
  if ( (_WORD)xanim_endNotifyStringHandle )
  {
    v0 = GetRefString_var + 8 * (unsigned __int16)xanim_endNotifyStringHandle;
    if ( *(_WORD *)v0 )
      --*(_WORD *)v0;
    else
      LOWORD(v0) = SL_FreeString(xanim_endNotifyStringHandle, (char *)(v0 + 4), strlen((const char *)(v0 + 4)) + 1);
    LOWORD(xanim_endNotifyStringHandle) = 0;
  }
  return v0;
}
#endif

_DWORD *__cdecl XAnimPrecacheAnimTree_m( const char *animName,
                                         int (__cdecl *a2)(int) );

/* ---- XAnimBeginLoadFiles  0x00483040 ----  VERIFIED */
void XAnimBeginLoadFiles( void )
{
  VariableValueInternal *node;

  node = AllocVariable();
  node->status = 111;
  node->u.halfword[0] = 0;
  node->u.halfword[1] = 0;

  LOWORD( dword_1407334 ) = (unsigned short)( node - scrVarNodes );
}

/* ---- XAnimLoadPendingFiles  0x00483080 ----  VERIFIED */
void __cdecl XAnimLoadPendingFiles( int (__cdecl *allocFn)(int) )
{
  unsigned short handle;
  unsigned short id;
  unsigned int   name;

  handle = (unsigned short)dword_1407334;
  id     = scrVarIndirections[ scrVarNodes[handle].nextSibling ].id;

  while ( ( scrVarNodes[id].status & VAR_STATUS_TYPE_MASK ) < VAR_THREAD && id )
  {
    name = scrVarNodes[id].status >> VAR_STATUS_NAME_SHIFT;

    XAnimPrecacheAnimTree_m( SL_STRING( name ), allocFn );

    id = scrVarIndirections[ scrVarNodes[id].nextSibling ].id;
  }

  RemoveRefToObject( handle );              /* inlined at 0x0048311E */
  LOWORD( dword_1407334 ) = 0;
}

typedef struct XAnimFileEntry_s {
    const char *name;                               /* +0x00 */
    XAnimParts *parts;                              /* +0x04 */
    void      (*freeData)( struct XAnimFileEntry_s * ); /* +0x08 */
} XAnimFileEntry;

/* ---- XAnimFreeMemory  0x00483170 ----  VERIFIED */
void __cdecl XAnimFreeMemory( XAnimFileEntry *entry )
{
    XAnimParts      *parts = entry->parts;
    unsigned short  *partNameHandles;
    XAnimNotifyInfo *notify;
    int              partNameCount;
    int              i;

    if ( (unsigned short)dword_1407334 ) {
        unsigned short nameHandle;

        nameHandle = SL_GetLowercaseStringOfLen(
                         entry->name, 0,
                         (unsigned int)strlen( entry->name ) + 1,
                         XANIM_FILENAME_STRING_TYPE );
        GetVariableIndexInternal( (unsigned short)dword_1407334, nameHandle );
        SL_RemoveRefToString( nameHandle );
    }

    partNameHandles = parts->partNameHandles;
    partNameCount   = (short)partNameHandles[0];

    for ( i = 0; i < partNameCount; ++i ) {
        SL_RemoveRefToString( partNameHandles[i + 1] );
    }

    notify = parts->notify;
    if ( notify != 0 ) {
        while ( notify->name != 0 ) {
            SL_RemoveRefToString( notify->name );
            ++notify;
        }
    }
}
#if 0
// XAnimFreeMemory: fileData free callback for an XAnimParts asset (installed by XAnimLoadFile): drops the part-name and notetrack string refs.
int __cdecl XAnimFreeMemory(int *a1)
{
  unsigned __int16 LowercaseStringOfLen;
  __int16 v3;
  int result;
  __int16 *v5;
  int v6;
  int v7;
  unsigned __int16 v8;
  unsigned __int16 *v9;
  unsigned __int16 j;
  int i;
  int v12;

  v12 = a1[1];
  if ( (_WORD)dword_1407334 )
  {
    LowercaseStringOfLen = SL_GetLowercaseStringOfLen(*a1, 0, strlen((const char *)*a1) + 1, 7);
    GetVariableIndexInternal(dword_1407334, LowercaseStringOfLen);
    v3 = *(_WORD *)(GetRefString_var + 8 * LowercaseStringOfLen);
    if ( v3 )
      *(_WORD *)(GetRefString_var + 8 * LowercaseStringOfLen) = v3 - 1;
    else
      SL_FreeString(
        LowercaseStringOfLen,
        (char *)(GetRefString_var + 8 * LowercaseStringOfLen + 4),
        GetRefString_var
      + 8 * LowercaseStringOfLen
      + 4
      + strlen((const char *)(GetRefString_var + 8 * LowercaseStringOfLen + 4))
      + 1
      - (GetRefString_var
       + 8 * LowercaseStringOfLen
       + 5)
      + 1);
  }
  result = v12;
  v5 = *(__int16 **)(v12 + 12);
  v6 = *v5;
  v7 = 0;
  for ( i = v6; v7 < v6; ++v7 )
  {
    v8 = v5[v7 + 1];
    result = GetRefString_var + 8 * v8;
    if ( *(_WORD *)result )
    {
      --*(_WORD *)result;
    }
    else
    {
      result = SL_FreeString(v8, (char *)(result + 4), strlen((const char *)(result + 4)) + 1);
      v6 = i;
    }
  }
  v9 = *(unsigned __int16 **)(v12 + 20);
  if ( v9 )
  {
    for ( j = *v9; j; v9 += 4 )
    {
      result = GetRefString_var + 8 * j;
      if ( *(_WORD *)result )
        --*(_WORD *)result;
      else
        result = SL_FreeString(j, (char *)(result + 4), strlen((const char *)(result + 4)) + 1);
      j = v9[4];
    }
  }
  return result;
}
#endif

/* ---- ReadNoteTracks  0x004832C0 ----  VERIFIED */
char *__cdecl ReadNoteTracks( unsigned char *cursor, int unused, XAnimParts *parts,
                          int (__cdecl *alloc)(int) )
{
    int              count   = *cursor;
    char            *namePtr = (char *)( cursor + 1 );
    XAnimNotifyInfo *notify  = (XAnimNotifyInfo *)alloc( 8 * count + 10 );

    parts->notify = notify;

    while ( count-- > 0 ) {
        unsigned int   len = (unsigned int)strlen( namePtr );
        unsigned short frame;

        /* 0x004832F9: push 3 / push len+1 / push 0 / push name -- type 3. */
        notify->name = SL_GetStringOfLen( namePtr, 0, len + 1, 3 );
        frame = *(unsigned short *)&namePtr[len + 1];
        namePtr += len + 3;

        notify->frameFrac = parts->frameCountMinusOne
                           ? (float)( (double)frame / (double)parts->frameCountMinusOne )
                           : 0.0f;
        ++notify;
    }

    notify->name      = SL_GetStringOfLen( "end", 0, (unsigned int)strlen( "end" ) + 1, 3 );
    notify->frameFrac = 1.0f;
    ( notify + 1 )->name = 0;

    return namePtr;
}

typedef struct XAnimTrackHeader_s {
    void           *data;              /* +0x00 */
    unsigned short  keyCountMinusOne;  /* +0x04 */
    unsigned char   frameIndex[1];     /* +0x06  [keyCount], byte or word */
} XAnimTrackHeader;

typedef struct XAnimPartStreamPair_s {
    void *trans;   /* +0x00 */
    void *rot;     /* +0x04 */
} XAnimPartStreamPair;

/* ---- XAnimAllocTrackHeader  no-address ----  VERIFIED */
static XAnimTrackHeader *XAnimAllocTrackHeader( unsigned char **cursorInOut,
                                                int (__cdecl *alloc)(int),
                                                int keyCount, int totalFrames,
                                                int smallIndices )
{
    unsigned char    *cursor = *cursorInOut;
    XAnimTrackHeader *header;

    if ( keyCount >= totalFrames ) {
        *cursorInOut = cursor;
        return (XAnimTrackHeader *)alloc( 8 );
    }
    if ( smallIndices ) {
        header = (XAnimTrackHeader *)alloc( keyCount + 7 );
        Com_Memcpy( header->frameIndex, cursor, keyCount );
        cursor += keyCount;
    } else {
        header = (XAnimTrackHeader *)alloc( 2 * keyCount + 6 );
        Com_Memcpy( header->frameIndex, cursor, 2 * keyCount );
        cursor += 2 * keyCount;
    }
    *cursorInOut = cursor;
    return header;
}

/* ---- XAnimReadSmallRotationTrack  no-address ---- */
static void *XAnimReadSmallRotationTrack( unsigned char **cursorInOut,
                                          int (__cdecl *alloc)(int),
                                          int keyCount, int totalFrames,
                                          int smallIndices, int negateFirst )
{
    unsigned char *cursor = *cursorInOut;
    void          *track;

    if ( !keyCount ) {
        track = 0;
    } else if ( keyCount == 1 ) {
        short           frame[2];
        unsigned short *out = (unsigned short *)alloc( 8 );

        ReadQuat2( (const short *)cursor, frame );
        cursor += 2;
        if ( negateFirst ) {
            frame[0] = -frame[0];
            frame[1] = -frame[1];
        }
        out[0] = (unsigned short)frame[0];
        out[1] = (unsigned short)frame[1];
        out[2] = 0;
        track = out;
    } else {
        XAnimTrackHeader *header =
            XAnimAllocTrackHeader( &cursor, alloc, keyCount, totalFrames, smallIndices );
        short (*data)[2] = (short (*)[2])alloc( 4 * keyCount );
        int    i;

        header->data = data;

        ReadQuat2( (const short *)cursor, data[0] );
        cursor += 2;
        if ( negateFirst ) {
            data[0][0] = -data[0][0];
            data[0][1] = -data[0][1];
        }
        for ( i = 1; i < keyCount; ++i ) {
            ReadQuat2( (const short *)cursor, data[i] );
            cursor += 2;
        }
        for ( i = 1; i < keyCount; ++i ) {
            int dot = data[i][0] * data[i - 1][0] + data[i][1] * data[i - 1][1];

            if ( dot < 0 ) {
                data[i][0] = -data[i][0];
                data[i][1] = -data[i][1];
            }
        }
        header->keyCountMinusOne = (unsigned short)( keyCount - 1 );
        track = header;
    }
    *cursorInOut = cursor;
    return track;
}

/* ---- XAnimReadFullRotationTrack  no-address ---- */
static void *XAnimReadFullRotationTrack( unsigned char **cursorInOut,
                                         int (__cdecl *alloc)(int),
                                         int keyCount, int totalFrames,
                                         int smallIndices, int negateFirst )
{
    unsigned char *cursor = *cursorInOut;
    void          *track;

    if ( !keyCount ) {
        track = 0;
    } else if ( keyCount == 1 ) {
        short           frame[4];
        unsigned short *out = (unsigned short *)alloc( 10 );

        ReadQuat( (const short *)cursor, frame );
        cursor += 6;
        if ( negateFirst ) {
            frame[0] = -frame[0]; frame[1] = -frame[1];
            frame[2] = -frame[2]; frame[3] = -frame[3];
        }
        out[0] = (unsigned short)frame[0]; out[1] = (unsigned short)frame[1];
        out[3] = (unsigned short)frame[2]; out[4] = (unsigned short)frame[3];
        out[2] = 0;
        track = out;
    } else {
        XAnimTrackHeader *header =
            XAnimAllocTrackHeader( &cursor, alloc, keyCount, totalFrames, smallIndices );
        short (*data)[4] = (short (*)[4])alloc( 8 * keyCount );
        int    i;

        header->data = data;

        ReadQuat( (const short *)cursor, data[0] );
        cursor += 6;
        if ( negateFirst ) {
            data[0][0] = -data[0][0]; data[0][1] = -data[0][1];
            data[0][2] = -data[0][2]; data[0][3] = -data[0][3];
        }
        for ( i = 1; i < keyCount; ++i ) {
            ReadQuat( (const short *)cursor, data[i] );
            cursor += 6;
        }
        for ( i = 1; i < keyCount; ++i ) {
            int dot = data[i - 1][0] * data[i][0] + data[i][3] * data[i - 1][3]
                    + data[i][2] * data[i - 1][2] + data[i][1] * data[i - 1][1];

            if ( dot < 0 ) {
                data[i][0] = -data[i][0]; data[i][1] = -data[i][1];
                data[i][2] = -data[i][2]; data[i][3] = -data[i][3];
            }
        }
        header->keyCountMinusOne = (unsigned short)( keyCount - 1 );
        track = header;
    }
    *cursorInOut = cursor;
    return track;
}

/* ---- XAnimReadTranslationTrack  no-address ---- */
static void *XAnimReadTranslationTrack( unsigned char **cursorInOut,
                                        int (__cdecl *alloc)(int),
                                        int keyCount, int totalFrames,
                                        int smallIndices )
{
    unsigned char *cursor = *cursorInOut;
    void          *track;

    if ( !keyCount ) {
        track = 0;
    } else if ( keyCount == 1 ) {
        unsigned char *out = (unsigned char *)alloc( 16 );

        *(float *)( out + 0 )  = *(const float *)( cursor + 0 );
        *(unsigned short *)( out + 4 ) = 0;
        *(float *)( out + 8 )  = *(const float *)( cursor + 4 );
        *(float *)( out + 12 ) = *(const float *)( cursor + 8 );
        cursor += 12;
        track = out;
    } else {
        XAnimTrackHeader *header =
            XAnimAllocTrackHeader( &cursor, alloc, keyCount, totalFrames, smallIndices );
        float (*data)[3] = (float (*)[3])alloc( 12 * keyCount );
        int    i;

        header->data = data;
        for ( i = 0; i < keyCount; ++i ) {
            Com_Memcpy( data[i], cursor, 12 );
            cursor += 12;
        }
        header->keyCountMinusOne = (unsigned short)( keyCount - 1 );
        track = header;
    }
    *cursorInOut = cursor;
    return track;
}

/* ---- XAnimPrecacheAnimTree_m  0x004833A0 ----  VERIFIED */
_DWORD *__cdecl XAnimPrecacheAnimTree_m( const char *animName, int (__cdecl *alloc)(int) )
{
    XAnimFileEntry *fileData = (XAnimFileEntry *)FS_GetDataForFile( "xanim", animName, "" );

    if ( !fileData )
        Com_Error( ERR_DROP, "\025Cannot precache 'xanim/%s'.", animName );

    if ( !fileData->parts ) {
        char                  Buffer[1024];
        void                 *Block;
        short                 version;
        unsigned short        rawFrameCount;
        unsigned short        partCount;
        unsigned char         flagsByte;
        short                 rawFrameRate;
        unsigned char        *cursor;
        int                   looped;
        int                   hasDeltaMotion;
        int                   totalFrames;
        int                   smallIndices;
        XAnimParts           *parts;
        unsigned short       *partNameHandles;
        int                   bitsetBytes;
        const unsigned char  *negateBits;
        unsigned char        *isSmallBits;
        XAnimPartStreamPair  *partStreams;
        int                   i;

        sprintf( Buffer, "xanim/%s", animName );
        if ( FS_ReadFile( Buffer, &Block ) < 0 )
            /* 0x0055A6F4 */
            Com_Error( ERR_DROP, "\025Cannot find 'xanim/%s'.", animName );

        version = *(short *)Block;
        if ( version != 14 ) {
            FS_FreeFile( Block );
            /* 0x0055A6C0 */
            Com_Error( ERR_DROP, "\025xanim '%s' out of date (version %d, expecting %d)",
                       animName, version, 14 );
        }

        rawFrameCount = *( (unsigned short *)Block + 1 );
        partCount     = *( (unsigned short *)Block + 2 );
        flagsByte     = *( (unsigned char *)Block + 6 );
        rawFrameRate  = *(short *)( (char *)Block + 7 );
        cursor        = (unsigned char *)Block + 9;

        looped         = flagsByte & 1;
        hasDeltaMotion = ( flagsByte & 2 ) != 0;

        partNameHandles     = (unsigned short *)alloc( 2 * partCount + 2 );
        partNameHandles[0]  = partCount;

        parts = (XAnimParts *)alloc( 32 );
        parts->looped          = (unsigned char)looped;
        parts->hasDeltaMotion  = (unsigned char)hasDeltaMotion;
        parts->frameRate       = (float)rawFrameRate;
        parts->partNameHandles = partNameHandles;

        totalFrames  = rawFrameCount + ( looped ? 1 : 0 );
        smallIndices = totalFrames <= 0x100;
        parts->frameCountMinusOne = (unsigned short)( totalFrames - 1 );
        parts->frequency = ( totalFrames == 1 )
                          ? 0.0f
                          : (float)( (double)parts->frameRate / (double)( totalFrames - 1 ) );

        if ( hasDeltaMotion ) {
            XAnimPartStreamPair *dm = (XAnimPartStreamPair *)alloc( 8 );
            unsigned short       rotCount;
            unsigned short       transCount;

            parts->deltaMotion = dm;

            rotCount = *(unsigned short *)cursor;
            cursor  += 2;
            dm->rot = XAnimReadSmallRotationTrack( &cursor, alloc, rotCount,
                                                   totalFrames, smallIndices, 0 );

            transCount = *(unsigned short *)cursor;
            cursor    += 2;
            dm->trans = XAnimReadTranslationTrack( &cursor, alloc, transCount,
                                                   totalFrames, smallIndices );
        }

        bitsetBytes = ( ( partCount - 1 ) >> 3 ) + 1;
        negateBits  = cursor;
        isSmallBits = (unsigned char *)alloc( bitsetBytes );
        Com_Memcpy( isSmallBits, cursor + bitsetBytes, bitsetBytes );
        cursor += 2 * bitsetBytes;
        parts->compressedRotationBits = isSmallBits;

        for ( i = 0; i < (int)partCount; ++i ) {
            unsigned int len = (unsigned int)strlen( (const char *)cursor );

            /* 0x00483861: push 9 / push len+1 / push 0 / push name -- type 9. */
            partNameHandles[i + 1] = SL_GetStringOfLen( (const char *)cursor, 0, len + 1, 9 );
            cursor += len + 1;
        }

        partStreams         = (XAnimPartStreamPair *)alloc( 8 * partCount );
        parts->partStreams  = partStreams;

        for ( i = 0; i < (int)partCount; ++i ) {
            int            smallRotation = DOBJ_BIT_TEST( isSmallBits, i );
            int            negateFirst   = DOBJ_BIT_TEST( negateBits, i );
            unsigned short rotCount;
            unsigned short transCount;

            rotCount = *(unsigned short *)cursor;
            cursor  += 2;
            partStreams[i].rot = smallRotation
                ? XAnimReadSmallRotationTrack( &cursor, alloc, rotCount, totalFrames,
                                               smallIndices, negateFirst )
                : XAnimReadFullRotationTrack( &cursor, alloc, rotCount, totalFrames,
                                              smallIndices, negateFirst );

            transCount = *(unsigned short *)cursor;
            cursor    += 2;
            partStreams[i].trans = XAnimReadTranslationTrack( &cursor, alloc, transCount,
                                                              totalFrames, smallIndices );
        }

        cursor = (unsigned char *)ReadNoteTracks( cursor, (int)animName, parts, alloc );

        FS_FreeFile( Block );

        fileData->parts    = parts;
        fileData->freeData = XAnimFreeMemory;
    }

    return (_DWORD *)fileData;
}

/* ---- XAnimSetLeafNode  0x00483F80 ----  VERIFIED */
void *__cdecl XAnimSetLeafNode(const char *animName, XAnim *tree, unsigned __int16 index)
{
  void *result;
  XAnimEntry *entry;

  result = (void *)FS_GetDataForFile("xanim", animName, "");
  if ( !result )
    Com_Error(ERR_DROP, "\025Cannot find 'xanim/%s'", animName);      /* 0x0055A6A8 */
  entry = &tree->entries[index];
  entry->childCount = 0;
  entry->u.leafAsset = result;
  return result;
}

/* ---- XAnimLoadFileData_m  no-address ----  VERIFIED */
void *__cdecl XAnimLoadFileData_m(const char *animName, XAnim *tree, unsigned __int16 index)
{
  return XAnimSetLeafNode(animName, tree, index);
}

/* ---- XAnimSetParentNode  0x00483FC0 ----  VERIFIED */
unsigned __int16 *__cdecl XAnimSetParentNode(
        unsigned short childCount,
        unsigned short flags,
        XAnim *tree,
        unsigned short index,
        int unused,
        unsigned short firstChildIndex )
{
    XAnimEntry     *entry  = &tree->entries[index];
    unsigned short *result = (unsigned short *)entry;
    int             i;

    entry->childCount               = childCount;
    entry->u.parent.flags           = flags;
    entry->u.parent.firstChildIndex = firstChildIndex;

    for ( i = 0; i < (int)childCount; ++i ) {
        int childSlot = i + tree->entries[index].u.parent.firstChildIndex;

        tree->entries[childSlot].parentIndex = index;
    }
    return result;
}
