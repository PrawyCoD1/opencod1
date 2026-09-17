/*
 * Original translation unit:
 * /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/xanim/xmodel.cpp
 * Retail range 0x00488150-0x0048BDE0, 102 functions.
 * @fidelity: likely
 */

#include <malloc.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"

#define xanim_pool xmodel_generatedPoolDecl_unused
#include "../qcommon/cod1_globals.h"
#undef xanim_pool

extern void *FS_GetDataForFile( const char *dir, const char *name, const char *ext );
extern int Hunk_ReallocateTempMemory();
extern unsigned short SL_GetStringOfLen( const char *text, unsigned char user,
                                         unsigned int size, int type );
extern int SL_FreeString__FUsPCcUi();
extern void WriteString( unsigned short id );
extern void  __cdecl XAnimCalcRelDeltaParts( float *result, int parts, float weight,
                                 float time0, float time1 );
extern float * __cdecl XAnimCalcAbsDeltaParts( float *result, unsigned short *parts,
                                   float weight, float time );
extern int  __cdecl XAnimFreeInfo( unsigned short handle );
extern int  __cdecl XAnimGetNextNotifyTime( int entry, int node, float time );
extern int XAnimClearServerNotify();
extern void __cdecl XAnimDisplay( void *tree, int nodeIndex, int depth );
extern void __cdecl XAnimCalcTreeDelta_m( void *tree, int nodeIndex, float weight,
                                float *delta, int isFirst, int isChild );
extern int XModelOptimize();

typedef struct XModelPartNameTable_s
{
    short           count;              /* +0x00 */
    unsigned short  handles[1];         /* +0x02, count entries */
} XModelPartNameTable;

typedef struct XModelPartNameTableSlot_s
{
    XModelPartNameTable *partNameTable; /* +0x00 */
    unsigned char   parentPartDeltas[4];/* +0x04, childCount entries */
} XModelPartNameTableSlot;

typedef struct XModelPartColl_s
{
    float           mins[3];
    float           maxs[3];
    float           center[3];
    float           radiusSq;
} XModelPartColl;

typedef struct XModelPartsData_s
{
    XModelPartNameTableSlot *partNameTableSlot; /* +0x00 */
    short           rootPartCount;              /* +0x04 */
    short           pad06;
    XModelPartColl *partCollisions;             /* +0x08 */
    short         (*baseRotations)[4];          /* +0x0C packed quat */
    float         (*baseTranslations)[3];       /* +0x10 */
    unsigned char  *partStateIndices;           /* +0x14 */
} XModelPartsData;

typedef struct XSimpleBlendInfo_s
{
    float           position[3];
    unsigned int    boneMatrixOffset;
} XSimpleBlendInfo;

typedef struct XSurfaceWeightedPoint_s
{
    XSimpleBlendInfo blend;
    float           weight;
} XSurfaceWeightedPoint;

typedef struct XSurfaceRigidVert_s
{
    float           normal[3];
    float           position[3];
} XSurfaceRigidVert;

typedef struct XSurfaceBlendVert_s
{
    float           normal[3];
    int             additiveWeightCount;
    XSimpleBlendInfo blend;
    float           primaryWeight;
} XSurfaceBlendVert;

typedef struct XSurface_s
{
    unsigned char   tileMode;           /* +0x00 */
    unsigned char   pad01;
    short           vertexCount;        /* +0x02 */
    short           triangleCount;      /* +0x04 */
    short           boneMatrixOffset;   /* +0x06  bone * 0x40, or -1 */
    unsigned int    boneUsage[4];       /* +0x08  128-bit bone-use mask */
    XSurfaceWeightedPoint *weightedPoints; /* +0x18 */
    unsigned short (*triangles)[3];     /* +0x1C */
    void           *vertexData;         /* +0x20 rigid verts or blend stream */
    float         (*texCoords)[2];      /* +0x24 */
    void           *optimizedDataARB;   /* +0x28 */
    void           *optimizedDataATI;   /* +0x2C */
    void           *optimizedDataNV;    /* +0x30 */
} XSurface;

typedef struct XStripInfo_s
{
    int             stripCount;
    const unsigned char  *stripVertexCounts;
    const unsigned short *stripIndices;
} XStripInfo;

typedef struct XModelSurfsData_s
{
    struct XModelSurfsData_s *next;     /* +0x00 clone list */
    short           surfaceCount;       /* +0x04 */
    short           pad06;
    XSurface      **surfaces;           /* +0x08 */
} XModelSurfsData;

typedef struct XModelSurfs_s
{
    const char     *name;
    XModelSurfsData *surfs;
    void          (*freeData)(struct XModelSurfs_s *);
} XModelSurfs;

typedef struct XModelPartsEntry_s
{
    const char     *name;
    XModelPartsData *data;
    void          (*freeData)(struct XModelPartsEntry_s *);
} XModelPartsEntry;

typedef struct XModelLodInfo_s
{
    float           distance;           /* +0x00 */
    const char     *name;               /* +0x04 */
    short           surfaceCount;       /* +0x08 */
    short           pad0a;
    unsigned short *surfaceNameTable;   /* +0x0C */
    XModelSurfs    *surfs;              /* +0x10 */
} XModelLodInfo;

typedef struct XModelCollTriPlane_s
{
    float           normal[3];
    float           distance;
} XModelCollTriPlane;

typedef struct XModelCollTri_s
{
    XModelCollTriPlane planes[3];
} XModelCollTri;

typedef struct XModelCollSurf_s
{
    XModelCollTri  *collTris;           /* +0x00 */
    int             numCollTris;        /* +0x04 */
    float           expandedMins[3];    /* +0x08 */
    float           expandedMaxs[3];    /* +0x14 */
    int             basePoseIndex;      /* +0x20 */
    int             contents;           /* +0x24 */
    int             surfaceFlags;       /* +0x28 */
} XModelCollSurf;

typedef struct XModelInfo_s
{
    XModelPartsEntry *parts;            /* +0x00 */
    XModelLodInfo   lodRecords[3];      /* +0x04 */
    XModelCollSurf *collisionSurfaces;  /* +0x40 */
    int             collisionSurfaceCount; /* +0x44 */
    int             contents;           /* +0x48 */
    float           mins[3];            /* +0x4C */
    float           maxs[3];            /* +0x58 */
    short           lodCount;           /* +0x64 */
    short           modelFileCount;     /* +0x66 */
} XModelInfo;

typedef struct XModel_s
{
    const char     *name;
    XModelInfo     *info;
    void          (*freeData)(struct XModel_s *);
} XModel;

typedef struct XModelConfigLod_s
{
    char            name[1024];
    float           distance;
} XModelConfigLod;

typedef struct XModelConfig_s
{
    XModelConfigLod lods[3];
    float           mins[3];
    float           maxs[3];
    int             modelFileCount;
} XModelConfig;

typedef struct DObjSkelMat_s
{
    float           axis[3][4];
    float           origin[4];
} DObjSkelMat;

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
    unsigned short notifyChildIndex;    /* +0x00 */
    short          notifyIndex;         /* +0x02  -1 == no pending notify */
    unsigned short notifyName;          /* +0x04 */
    unsigned short notifyType;          /* +0x06 */
    unsigned short freePrev;            /* +0x08 */
    unsigned short freeNext;            /* +0x0A  node 0's is the free head */
    XAnimState     states[2];           /* +0x0C and +0x28 */
} XAnimInfo;

typedef struct XAnimEntry_s {
    unsigned short childCount;          /* +0x00  0 == leaf */
    unsigned short parentIndex;         /* +0x02 */
    union {
        void *leafAsset;                /* +0x04 */
        struct {
            unsigned short flags;           /* +0x04 */
            unsigned short firstChildIndex; /* +0x06 */
        } parent;
    } u;
} XAnimEntry;

typedef struct XAnim_s {
    const char *name;                   /* +0x00 */
    int         nodeCount;              /* +0x04 */
    XAnimEntry  entries[1];             /* +0x08 */
} XAnim;

typedef struct XAnimTree_s {
    XAnim         *sourceTree;             /* +0x00 */
    int            partRemapTableSelector; /* +0x04 */
    unsigned short poolNodeHandles[1];     /* +0x08  [nodeCount] */
} XAnimTree;

typedef struct XAnimParts_min_s {
    unsigned short  pad00[6];         /* +0x00 .. +0x0B */
    unsigned short *partNameHandles;  /* +0x0C  [0] is the part count */
} XAnimParts_min;

extern XAnimInfo xanim_pool[2048];

/* The lane xanim_activePoolSlot (0x00A9CC58) selects: client 0, server 1. */
#define XANIM_LANE(h)   ( xanim_pool[h].states[xanim_activePoolSlot] )

extern const char *SL_ConvertToString( unsigned short handle );
extern void        SL_RemoveRefToStringOfLen( unsigned short handle,
                                              unsigned int size );
extern void        SL_RemoveRefToString( unsigned short handle );
extern void        SL_AddRefToString( unsigned short handle );

extern int __cdecl XAnimSetAnimInternal_m( int nodeIndex, float weight, float blendTime,
                               float rate, char force,
                               unsigned __int16 notifyName,
                               __int16 notifyType );

extern int  __cdecl XAnimUpdateNotifyIndex_m( int node );
extern void __cdecl XAnimReseedTiming_m( int nodeIndex, int forceClear );
extern int  __cdecl cand_XAnimSeedParentChain( int node, int blendTimeBits );
extern __int16 __cdecl XAnimSetAnimKnobInternal_m( unsigned __int16 notifyName,
                                   XAnimTree *tree, int node, float weight,
                                   int blendTimeBits, float rate,
                                   __int16 notifyType, int animFlags );

typedef void *(*XModelAllocFn)(int size);

#define XMODEL_ASSET_VERSION    14
#define XMODEL_LOD_COUNT        3
#define XMODEL_MAX_BONES        127
#define XMODEL_NO_SINGLE_BONE   (-1)

#define XMODEL_BONE_MATRIX_SHIFT    6

/* the default-model block, 0x00A9CC88-0x00A9CE10 */

static XModelPartNameTable      xmodel_defaultPartNameTable;    /* 0x00A9CC88 */
static XModelPartNameTableSlot  xmodel_defaultPartNameTableSlot;/* 0x00A9CC8C */
static XSurface                 xmodel_defaultSurfaces[1];      /* 0x00A9CC94 */
static XSurface                *xmodel_defaultSurfacePtrs[1];   /* 0x00A9CCC8 */
XModelInfo                      xmodel_defaultCollision;        /* 0x00A9CCCC */
static XModelPartsData          xmodel_defaultParts;            /* 0x00A9CD34 */
static XModelSurfsData          xmodel_defaultSurfs;            /* 0x00A9CD4C */
static XModelPartColl           xmodel_defaultPartColl;         /* 0x00A9CD58 */
static XModel                   xmodel_defaultModelEntry;       /* 0x00A9CD80 */
static XModelPartsEntry         xmodel_defaultPartsEntry;       /* 0x00A9CD8C */
static XModelSurfs              xmodel_defaultSurfsEntry;       /* 0x00A9CD98 */
static XSurfaceRigidVert        xmodel_defaultRigidVerts[3];    /* 0x00A9CDA4 */
static float                    xmodel_defaultTexCoords[3][2];  /* 0x00A9CDEC */
static unsigned short           xmodel_defaultTriangle[3];      /* 0x00A9CE06 */
static unsigned char            xmodel_defaultPartStateIndices[1]; /* 0x00A9CE0C */
static unsigned short           xmodel_defaultSurfaceNames[1];  /* 0x00A9CE0E */

float                           xmodel_testLodDist;             /* 0x00A9CE10 */
unsigned char                   xmodel_testLodsEnabled;         /* 0x00A9CE14 */
XModelLodInfo                   xmodel_testLodDistances[XMODEL_LOD_COUNT]; /* 0x00A9CE18 */

static XModelSurfsData         *xmodel_surfsCloneList;

#define xmodel_enforceExist     xmodel_enforceExist
#define xmodel_animCheck        xmodel_animCheck

#define SL_REFSTRING(h)         ( (char *)GetRefString_var + 8 * (unsigned short)(h) )

/* ---- XModelReleaseString  no-address ---- */
static void XModelReleaseString(unsigned short handle)
{
    char *rec = SL_REFSTRING(handle);

    if ( *(unsigned short *)rec )
        --*(unsigned short *)rec;
    else
        SL_FreeString__FUsPCcUi(handle, rec + 4, strlen(rec + 4) + 1);
}

/* ---- DObjDisplayAnim  0x00488150 ----  [CONFIRMED] */
void __cdecl DObjDisplayAnim(int **a1)
{
  int *v1;

  v1 = *a1;
  if ( v1 )
  {
    XAnimDisplay(v1, 0, 0);
    Com_Printf("\n");
  }
  else
  {
    Com_Printf("NO TREE\n");
  }
}

/* ---- XAnimGetRelDelta_m  0x00488180 ----  VERIFIED */
int __cdecl XAnimGetRelDelta_m(int a1, int a2, int *a3, float *a4, _DWORD *a5)
{
  float delta[6];
  float v5;
  int result;
  int v7;
  int v8;

  dword_A9C620 = a1;
  byte_A9C63C = 0;
  XAnimCalcTreeDelta_m(a5, a2, 1.0f, delta, 1, 0);
  if ( (delta[0] == 0.0) | __UNORDERED__(delta[0], 0.0) || (delta[1] == 0.0) | __UNORDERED__(delta[1], 0.0) )
  {
    *a4 = 0.0;
    a4[1] = 1.0;
  }
  else
  {
    v5 = delta[1];
    *a4 = delta[0];
    a4[1] = v5;
  }
  result = ((int *)delta)[3];
  v7 = ((int *)delta)[4];
  v8 = ((int *)delta)[5];
  *a3 = result;
  a3[1] = v7;
  a3[2] = v8;
  return result;
}

/* ---- XAnimGetAbsDelta_m  0x00488210 ----  VERIFIED */
int __cdecl XAnimGetAbsDelta_m(int a1, _DWORD *a2, _DWORD *a3, float *a4)
{
  float delta[6];
  float v4;
  int result;
  int v6;

  byte_A9C63C = 1;
  dword_A9C620 = 1;
  XAnimCalcTreeDelta_m(a2, a1, 1.0f, delta, 1, 0);
  if ( (delta[0] == 0.0) | __UNORDERED__(delta[0], 0.0) && (delta[1] == 0.0) | __UNORDERED__(delta[1], 0.0) )
  {
    *a4 = 0.0;
    a4[1] = 1.0;
  }
  else
  {
    v4 = delta[1];
    *a4 = delta[0];
    a4[1] = v4;
  }
  result = ((int *)delta)[4];
  v6 = ((int *)delta)[5];
  *a3 = ((int *)delta)[3];
  a3[1] = result;
  a3[2] = v6;
  return result;
}

/* ---- XAnimGetRelDeltaForTime_m  0x00488290 ----  VERIFIED */
char __cdecl XAnimGetRelDeltaForTime_m(int a1, int a2, _DWORD *a3, float *a4, float a5, float a6)
{
  float delta[6];
  int v6;
  float v7;
  int v8;
  int v9;

  if ( *(_WORD *)(a2 + 8 * a1 + 8)
    || (v6 = *(_DWORD *)(*(_DWORD *)(a2 + 8 * a1 + 12) + 4), LOBYTE(a1) = *(_BYTE *)(v6 + 3), !(_BYTE)a1) )
  {
    *a4 = 0.0;
    a4[1] = 1.0;
    a3[1] = 0;
    a3[2] = 0;
    *a3 = 0;
  }
  else
  {
    delta[0] = 0.0;
    delta[1] = 0.0;
    ((int *)delta)[2] = 0;
    ((int *)delta)[3] = 0;
    ((int *)delta)[4] = 0;
    ((int *)delta)[5] = 0;
    XAnimCalcRelDeltaParts(delta, v6, 1.0f, a5, a6);
    if ( (delta[0] == 0.0) | __UNORDERED__(delta[0], 0.0) && (delta[1] == 0.0) | __UNORDERED__(delta[1], 0.0) )
    {
      *a4 = 0.0;
      a4[1] = 1.0;
    }
    else
    {
      v7 = delta[1];
      *a4 = delta[0];
      a4[1] = v7;
    }
    v8 = ((int *)delta)[3];
    LOBYTE(a1) = ((int *)delta)[4];
    v9 = ((int *)delta)[5];
    a3[1] = ((int *)delta)[4];
    a3[2] = v9;
    *a3 = v8;
  }
  return a1;
}

/* ---- XAnimGetAbsDeltaForTime_m  0x00488360 ----  VERIFIED */
int __cdecl XAnimGetAbsDeltaForTime_m(int result, int a2, float *a3, _DWORD *a4, float a5)
{
  float delta[6];
  float v5;
  int v6;

  if ( *(_WORD *)(a2 + 8 * result + 8)
    || (result = *(_DWORD *)(*(_DWORD *)(a2 + 8 * result + 12) + 4), !*(_BYTE *)(result + 3)) )
  {
    *a3 = 0.0;
    a3[1] = 1.0;
    *a4 = 0;
    a4[1] = 0;
    a4[2] = 0;
  }
  else
  {
    delta[0] = 0.0;
    delta[1] = 0.0;
    ((int *)delta)[2] = 0;
    ((int *)delta)[3] = 0;
    ((int *)delta)[4] = 0;
    ((int *)delta)[5] = 0;
    XAnimCalcAbsDeltaParts(delta, (unsigned short *)result, 1.0f, a5);
    if ( (delta[0] == 0.0) | __UNORDERED__(delta[0], 0.0) && (delta[1] == 0.0) | __UNORDERED__(delta[1], 0.0) )
    {
      *a3 = 0.0;
      a3[1] = 1.0;
    }
    else
    {
      v5 = delta[1];
      *a3 = delta[0];
      a3[1] = v5;
    }
    v6 = ((int *)delta)[4];
    result = ((int *)delta)[5];
    *a4 = ((int *)delta)[3];
    a4[1] = v6;
    a4[2] = result;
  }
  return result;
}

/* ---- XAnimAllocInfo  0x00488450 ----  VERIFIED */
XAnimInfo *__cdecl XAnimAllocInfo( XAnimTree *tree, int nodeIndex )
{
    unsigned short handle;
    unsigned short newHead;

    handle = xanim_pool[0].freeNext;
    if ( !handle ) {
        Com_Error( ERR_DROP, "\x15" "exceeded maximum number of anim info" );
    }

    ++xanim_poolUsedCount;
    if ( xanim_poolUsedCount > xanim_poolHighWaterCount ) {
        xanim_poolHighWaterCount = xanim_poolUsedCount;
    }

    newHead = xanim_pool[handle].freeNext;
    xanim_pool[0].freeNext        = newHead;
    xanim_pool[newHead].freePrev  = 0;
    tree->poolNodeHandles[nodeIndex] = handle;

    return &xanim_pool[handle];
}
#if 0
// XAnimAllocInfo: allocates an XAnimInfo pool node (pops the free head at 0x00A7A62A, bumps the high-water counter, Com_Error when exhausted) and stores its handle in the runtime tree slot.
char *__cdecl XAnimAllocInfo(int a1, int a2)
{
  __int16 v2;
  int v3;
  int v4;

  v2 = word_A7A62A[0];
  if ( !word_A7A62A[0] )
    Com_Error(ERR_DROP, &byte_55A4E0);
  v3 = ++xanim_poolUsedCount;
  if ( xanim_poolUsedCount > xanim_poolHighWaterCount )
    xanim_poolHighWaterCount = v3;
  v4 = 34 * (unsigned __int16)word_A7A62A[0];
  word_A7A62A[0] = word_A7A62A[v4];
  word_A7A628[34 * (unsigned __int16)word_A7A62A[0]] = 0;
  *(_WORD *)(a1 + 2 * a2 + 8) = v2;
  return (char *)&xanim_pool + v4 * 2;
}
#endif

/* ---- XAnimParentChainHasWeight_m  0x004884D0 ----  VERIFIED */
char __cdecl XAnimParentChainHasWeight_m( int nodeIndex, XAnimTree *tree )
{
    XAnim *source;

    if ( nodeIndex == 0 ) {
        return 1;
    }

    source = tree->sourceTree;
    for ( ;; ) {
        nodeIndex = source->entries[nodeIndex].parentIndex;

        if ( XANIM_LANE( tree->poolNodeHandles[nodeIndex] ).currentWeight == 0.0f ) {
            return 0;
        }
        if ( nodeIndex == 0 ) {
            return 1;
        }
    }
}
#if 0
// XAnimParentChainHasWeight_m: true while every ancestor of the node still carries weight.
char __cdecl XAnimParentChainHasWeight_m(int a1, _DWORD *a2)
{
  double v2;

  if ( !a1 )
    return 1;
  while ( 1 )
  {
    a1 = *(unsigned __int16 *)(*a2 + 8 * a1 + 10);
    v2 = *((float *)&unk_A7A640 + 17 * *((unsigned __int16 *)a2 + a1 + 4) + 7 * xanim_activePoolSlot);
    if ( (v2 == 0.0) | __UNORDERED__(v2, 0.0) )
      break;
    if ( !a1 )
      return 1;
  }
  return 0;
}
#endif

/* ---- XAnimSubtreeHasWeight_m  0x00488510 ----  VERIFIED */
char __cdecl XAnimSubtreeHasWeight_m( XAnimTree *tree, int nodeIndex )
{
    unsigned short  handle;
    XAnimEntry     *entry;
    int             childCount;
    int             firstChild;
    int             i;

    handle = tree->poolNodeHandles[nodeIndex];
    if ( handle == 0 ) {
        return 0;
    }
    if ( XANIM_LANE( handle ).currentWeight == 0.0f ) {
        return 0;
    }

    entry      = &tree->sourceTree->entries[nodeIndex];
    childCount = entry->childCount;
    if ( childCount == 0 ) {
        return 1;
    }

    firstChild = entry->u.parent.firstChildIndex;
    for ( i = 0; i < childCount; ++i ) {
        if ( XAnimSubtreeHasWeight_m( tree, firstChild + i ) ) {
            return 1;
        }
    }
    return 0;
}
#if 0
// XAnimSubtreeHasWeight_m: true if the node has weight and (is a leaf or has a weighted descendant).
char __cdecl XAnimSubtreeHasWeight_m(_DWORD *a1, int a2)
{
  unsigned __int16 v2;
  double v3;
  _WORD *v5;
  int v6;
  int v7;
  int v8;

  v2 = *((_WORD *)a1 + a2 + 4);
  if ( !v2 )
    return 0;
  v3 = *((float *)&unk_A7A640 + 17 * v2 + 7 * xanim_activePoolSlot);
  if ( (v3 == 0.0) | __UNORDERED__(v3, 0.0) )
    return 0;
  v5 = (_WORD *)(*a1 + 8 * a2 + 8);
  v6 = (unsigned __int16)*v5;
  if ( *v5 )
  {
    v7 = 0;
    v8 = *(unsigned __int16 *)(*a1 + 8 * a2 + 14);
    while ( !XAnimSubtreeHasWeight_m(a1, v7 + v8) )
    {
      if ( ++v7 >= v6 )
        return 0;
    }
  }
  return 1;
}
#endif

/* ---- XAnimNodeIsActive_m  0x00488590 ----  VERIFIED */
BOOL __cdecl XAnimNodeIsActive_m(_DWORD *a1, int a2)
{
  return XAnimParentChainHasWeight_m(a2, (XAnimTree *)a1) && XAnimSubtreeHasWeight_m((XAnimTree *)a1, a2);
}

/* ---- XAnimClearAnimNode_m  0x004885C0 ----  VERIFIED */
void __cdecl XAnimClearAnimNode_m(_DWORD *a1, int a2, float a3)
{
  unsigned __int16 v4;
  char *v5;
  int v6;

  v4 = *((_WORD *)a1 + a2 + 4);
  if ( v4 )
  {
    v5 = (char *)&xanim_pool + 68 * v4;
    v6 = (int)&v5[28 * xanim_activePoolSlot + 12];
    if ( XAnimParentChainHasWeight_m(a2, (XAnimTree *)a1) && XAnimSubtreeHasWeight_m((XAnimTree *)a1, a2) )
    {
      if ( (*(float *)(v6 + 16) == 0.0) | __UNORDERED__(*(float *)(v6 + 16), 0.0) )
      {
        if ( *(float *)(v6 + 12) > (double)a3 )
          *(float *)(v6 + 12) = a3;
      }
      else
      {
        *(_DWORD *)(v6 + 16) = 0;
        *(float *)(v6 + 12) = a3;
      }
    }
    else
    {
      *(_DWORD *)(v6 + 20) = 0;
      *(_DWORD *)(v6 + 12) = 0;
      *(_DWORD *)(v6 + 16) = 0;
    }
    if ( xanim_activePoolSlot )
      XAnimClearServerNotify((int)v5);
  }
}

/* ---- XAnimClearAnimSubtree_m  0x00488660 ----  VERIFIED */
void __cdecl XAnimClearAnimSubtree_m(_DWORD *a1, int a2, float a3)
{
  int v3;
  _WORD *v4;
  int v5;

  if ( *((_WORD *)a1 + a2 + 4) )
  {
    XAnimClearAnimNode_m(a1, a2, a3);
    v3 = *(unsigned __int16 *)(*a1 + 8 * a2 + 8);
    v4 = (_WORD *)(*a1 + 8 * a2 + 8);
    v5 = 0;
    if ( *v4 )
    {
      do
      {
        XAnimClearAnimSubtree_m(a1, v5 + (unsigned __int16)v4[3], a3);
        ++v5;
      }
      while ( v5 < v3 );
    }
  }
}

/* ---- XAnimClearAnim_m  0x004886C0 ----  VERIFIED */
void __cdecl XAnimClearAnim_m(int a1, _DWORD *a2, float a3)
{
  if ( (a3 < 0.001) | __UNORDERED__(a3, 0.001) )
    a3 = 0.0;
  XAnimClearAnimSubtree_m(a2, a1, a3);
}

/* ---- XAnimClearAnimChildSubtrees_m  0x004886F0 ----  VERIFIED */
void __cdecl XAnimClearAnimChildSubtrees_m(_DWORD *a1, int v2, float a2)
{
  int v3;
  _WORD *v4;
  int v5;

  if ( (a2 < 0.001) | __UNORDERED__(a2, 0.001) )
    a2 = 0.0;
  v3 = *(unsigned __int16 *)(*a1 + 8 * v2 + 8);
  v4 = (_WORD *)(*a1 + 8 * v2 + 8);
  v5 = 0;
  if ( *v4 )
  {
    do
    {
      XAnimClearAnimSubtree_m(a1, v5 + (unsigned __int16)v4[3], a2);
      ++v5;
    }
    while ( v5 < v3 );
  }
}

/* ---- XAnimSetGoalWeight_m  0x00488750 ----  VERIFIED */
void __cdecl XAnimSetGoalWeight_m( XAnimTree *tree, int nodeIndex, float goalWeight,
                         float blendFactor )
{
    XAnimEntry *parentEntry;
    int         siblingCount;
    int         firstSibling;
    int         i;
    float       maxWeight = 0.0f;
    float       blendTime;

    if ( nodeIndex == 0 ) {
        return;
    }

    parentEntry  = &tree->sourceTree->entries[
                       tree->sourceTree->entries[nodeIndex].parentIndex];
    siblingCount = parentEntry->childCount;
    firstSibling = parentEntry->u.parent.firstChildIndex;

    for ( i = 0; i < siblingCount; ++i ) {
        int            sibling = firstSibling + i;
        unsigned short handle  = tree->poolNodeHandles[sibling];
        float          weight  = handle ? XANIM_LANE( handle ).currentWeight
                                        : 0.0f;
        float          candidate;

        if ( sibling == nodeIndex ) {
            union { float f; unsigned int u; } bits;

            bits.f    = goalWeight - weight;
            bits.u   &= 0x7FFFFFFFu;
            candidate = bits.f;
        } else {
            candidate = weight;
        }

        if ( candidate > maxWeight ) {
            maxWeight = candidate;
        }
    }

    blendTime = maxWeight * blendFactor;
    if ( blendTime < 0.001f ) {
        blendTime = 0.0f;
    }

    for ( i = 0; i < siblingCount; ++i ) {
        int sibling = parentEntry->u.parent.firstChildIndex + i;

        if ( sibling != nodeIndex ) {
            XAnimClearAnimNode_m( (_DWORD *)tree, sibling, blendTime );
        }
    }
}
#if 0
// XAnimSetGoalWeight_m: redistributes goal weights across a node's siblings before a new animation is set.  Called first by all four XAnimSetAnim* entry points.
void __cdecl XAnimSetGoalWeight_m(_DWORD *a1, int a2, float a3, float a4)
{
  double v4;
  int v5;
  signed int v6;
  int v7;
  signed int v8;
  int v9;
  int v10;
  int v11;
  unsigned __int16 *v12;
  unsigned __int16 v13;
  double v14;
  double v15;
  unsigned __int16 v16;
  double v17;
  unsigned __int16 v18;
  double v19;
  int v20;
  _WORD *v21;
  int v22;
  double v23;
  double v24;
  signed int i;
  int v26;
  int v27;
  int v28;
  int v29;
  int v30;
  int v31;
  float v32;
  float v33;
  float v34;
  float v35;
  float v36;
  float v37;

  if ( a2 )
  {
    v4 = 0.0;
    v5 = *(unsigned __int16 *)(*a1 + 8 * a2 + 10);
    v6 = *(unsigned __int16 *)(*a1 + 8 * v5 + 8);
    v7 = *a1 + 8 * v5 + 8;
    v8 = 0;
    if ( (unsigned int)v6 >= 4 )
    {
      v9 = *(unsigned __int16 *)(*a1 + 8 * v5 + 14);
      v10 = 3;
      v11 = v9 + 2;
      v12 = (unsigned __int16 *)a1 + v9 + 5;
      do
      {
        v13 = *(v12 - 1);
        if ( v13 )
          v14 = *((float *)&unk_A7A640 + 17 * v13 + 7 * xanim_activePoolSlot);
        else
          v14 = 0.0;
        if ( v11 - 2 == a2 )
        {
          v32 = a3 - v14;
          v27 = LODWORD(v32) & 0x7FFFFFFF;
        }
        else
        {
          *(float *)&v27 = v14;
        }
        if ( *(float *)&v27 > v4 )
          v4 = *(float *)&v27;
        if ( *v12 )
          v15 = *((float *)&unk_A7A640 + 17 * *v12 + 7 * xanim_activePoolSlot);
        else
          v15 = 0.0;
        if ( v11 - 1 == a2 )
        {
          v33 = a3 - v15;
          v28 = LODWORD(v33) & 0x7FFFFFFF;
        }
        else
        {
          *(float *)&v28 = v15;
        }
        if ( *(float *)&v28 > v4 )
          v4 = *(float *)&v28;
        v16 = v12[1];
        if ( v16 )
          v17 = *((float *)&unk_A7A640 + 17 * v16 + 7 * xanim_activePoolSlot);
        else
          v17 = 0.0;
        if ( v11 == a2 )
        {
          v34 = a3 - v17;
          v29 = LODWORD(v34) & 0x7FFFFFFF;
        }
        else
        {
          *(float *)&v29 = v17;
        }
        if ( *(float *)&v29 > v4 )
          v4 = *(float *)&v29;
        v18 = v12[2];
        if ( v18 )
          v19 = *((float *)&unk_A7A640 + 17 * v18 + 7 * xanim_activePoolSlot);
        else
          v19 = 0.0;
        if ( v11 + 1 == a2 )
        {
          v35 = a3 - v19;
          v30 = LODWORD(v35) & 0x7FFFFFFF;
        }
        else
        {
          *(float *)&v30 = v19;
        }
        if ( *(float *)&v30 > v4 )
          v4 = *(float *)&v30;
        v10 += 4;
        v8 += 4;
        v12 += 4;
        v11 += 4;
      }
      while ( v10 < v6 );
    }
    if ( v8 < v6 )
    {
      v20 = *(unsigned __int16 *)(v7 + 6) + v8;
      v21 = (_WORD *)a1 + v20 + 4;
      v22 = v6 - v8;
      do
      {
        if ( *v21 )
          v23 = *((float *)&unk_A7A640 + 17 * (unsigned __int16)*v21 + 7 * xanim_activePoolSlot);
        else
          v23 = 0.0;
        if ( v20 == a2 )
        {
          v36 = a3 - v23;
          v31 = LODWORD(v36) & 0x7FFFFFFF;
        }
        else
        {
          *(float *)&v31 = v23;
        }
        if ( *(float *)&v31 > v4 )
          v4 = *(float *)&v31;
        ++v21;
        ++v20;
        --v22;
      }
      while ( v22 );
    }
    v24 = v4 * a4;
    v37 = v24;
    if ( (v24 < 0.001) | __UNORDERED__(v24, 0.001) )
      v37 = 0.0;
    for ( i = 0; i < v6; ++i )
    {
      v26 = *(unsigned __int16 *)(v7 + 6);
      if ( v26 + i != a2 )
        XAnimClearAnimNode_m(a1, v26 + i, v37);
    }
  }
}
#endif

/* ---- XAnimSetAnimKnob_m  0x00488A10 ----  VERIFIED */
__int16 __cdecl XAnimSetAnimKnob_m(
        _DWORD *a1,
        int a2,
        float a3,
        int a4,
        float a5,
        __int16 a6,
        __int16 a7,
        int a8)
{
  if ( (a3 < 0.001) | __UNORDERED__(a3, 0.001) )
    a3 = 0.0;
  XAnimSetGoalWeight_m((XAnimTree *)a1, a2, a3, *(float *)&a4);
  return XAnimSetAnimKnobInternal_m(a6, (XAnimTree *)a1, a2, a3, a4, a5, a7, a8);
}

/* ---- XAnimSetAnimKnobAll_m  0x00488A70 ----  VERIFIED */
int __cdecl XAnimSetAnimKnobAll_m(
        int a1,
        _DWORD *a2,
        int a3,
        float a4,
        int a5,
        float a6,
        __int16 a7,
        __int16 a8,
        int a9)
{
  int v9;
  float *v10;
  _WORD *v11;
  int v13;

  v9 = a1;
  xanim_currentTree = (int)a2;
  if ( (a4 < 0.001) | __UNORDERED__(a4, 0.001) )
    a4 = 0.0;
  XAnimSetGoalWeight_m((XAnimTree *)a2, a1, a4, *(float *)&a5);
  v13 = XAnimSetAnimInternal_m(v9, a4, *(float *)&a5, a6, 0, a7, a8);
  cand_XAnimSeedParentChain(v9, a5);
  XAnimReseedTiming_m(v9, a9);
  if ( xanim_activePoolSlot )
    XAnimUpdateNotifyIndex_m(v9);
  if ( !v9 )
    return 1;
  while ( 1 )
  {
    v9 = *(unsigned __int16 *)(*a2 + 8 * v9 + 10);
    if ( v9 == a3 )
      return v13;
    XAnimSetGoalWeight_m((XAnimTree *)a2, v9, 1.0f, *(float *)&a5);
    XAnimSetAnimInternal_m(v9, 1.0f, *(float *)&a5, 1.0f, 0, 0, 0);
    XAnimReseedTiming_m(v9, a9);
    if ( !xanim_activePoolSlot )
      goto LABEL_15;
    v10 = (float *)((char *)&xanim_pool + 68 * *(unsigned __int16 *)(xanim_currentTree + 2 * v9 + 8));
    if ( !*((_WORD *)v10 + 2) )
      goto LABEL_15;
    if ( *((_DWORD *)v10 + 10) == 1065353216 )
    {
      *((_WORD *)v10 + 1) = -1;
    }
    else
    {
      v11 = (_WORD *)(*(_DWORD *)xanim_currentTree + 8 * v9 + 8);
      if ( !*v11 )
        goto LABEL_14;
      if ( *(_WORD *)v10 )
      {
        v11 = (_WORD *)(*(_DWORD *)xanim_currentTree + 8 * *(unsigned __int16 *)v10 + 8);
LABEL_14:
        *((_WORD *)v10 + 1) = XAnimGetNextNotifyTime((int)v11, (int)v10, v10[10]);
      }
    }
LABEL_15:
    if ( !v9 )
      return 1;
  }
}

/* ---- XAnimSetAnimLimited_m  0x00488BC0 ---- */
extern int __cdecl XAnimSetAnimInternalLimited_m( __int16 a1, unsigned __int16 a2, int a3, int a4,
                               float a5, int a6, float a7, int a8 );

/* ---- XAnimSetAnimLimited_m  0x00488BC0 ----  VERIFIED */
int __cdecl XAnimSetAnimLimited_m(int a1, _DWORD *a2, float a3, int a4, float a5, __int16 a6, __int16 a7, int a8)
{
  if ( (a3 < 0.001) | __UNORDERED__(a3, 0.001) )
    a3 = 0.0;
  XAnimSetGoalWeight_m((XAnimTree *)a2, a1, a3, *(float *)&a4);
  return XAnimSetAnimInternalLimited_m(a7, a6, (int)a2, a1, a3, a4, a5, a8);
}

/* ---- XAnimClearAnimChildren_m  0x00488C20 ----  VERIFIED */
void __cdecl XAnimClearAnimChildren_m(_DWORD *a1, int v2, float a2)
{
  int v3;
  _WORD *v4;
  int v5;

  if ( (a2 < 0.001) | __UNORDERED__(a2, 0.001) )
    a2 = 0.0;
  v3 = *(unsigned __int16 *)(*a1 + 8 * v2 + 8);
  v4 = (_WORD *)(*a1 + 8 * v2 + 8);
  v5 = 0;
  if ( *v4 )
  {
    do
    {
      XAnimClearAnimNode_m(a1, v5 + (unsigned __int16)v4[3], a2);
      ++v5;
    }
    while ( v5 < v3 );
  }
}

/* ---- XAnimClearTree_r  0x00488C80 ----  VERIFIED */
int __cdecl XAnimClearTree_r(_DWORD *a1, int a2)
{
  int result;
  unsigned __int16 v4;
  int v5;
  _WORD *v6;
  int v7;
  unsigned __int16 v8;

  result = a2;
  v4 = *((_WORD *)a1 + a2 + 4);
  v8 = v4;
  if ( v4 )
  {
    v5 = *(unsigned __int16 *)(*a1 + 8 * a2 + 8);
    v6 = (_WORD *)(*a1 + 8 * a2 + 8);
    v7 = 0;
    if ( *v6 )
    {
      do
      {
        XAnimClearTree_r(a1, v7 + (unsigned __int16)v6[3]);
        ++v7;
      }
      while ( v7 < v5 );
      v4 = v8;
    }
    result = XAnimFreeInfo(v4);
    *((_WORD *)a1 + a2 + 4) = 0;
  }
  return result;
}

/* ---- XAnimClearTree  0x00488CF0 ----  VERIFIED */
void __cdecl XAnimClearTree( XAnimTree *tree )
{
    unsigned short *table;
    int             nodeCount;
    int             tableIndex;
    int             i;

    XAnimClearTree_r( (_DWORD *)tree, 0 );

    nodeCount = tree->sourceTree->nodeCount;
    table     = (unsigned short *)( (char *)tree + 8 + 2 * nodeCount );

    for ( tableIndex = 0; tableIndex < 2; ++tableIndex ) {
        for ( i = nodeCount - 1; i >= 0; --i ) {
            unsigned short handle = table[i];

            if ( handle ) {
                XAnimParts_min *parts =
                    (XAnimParts_min *)
                        ( (void **)tree->sourceTree->entries[i].u.leafAsset )[1];

                SL_RemoveRefToStringOfLen(
                    handle,
                    (unsigned int)( (int)*(const short *)parts->partNameHandles
                                    + 16 ) );
                table[i] = 0;
            }
        }
        table = (unsigned short *)( (char *)table + 3 * nodeCount + 1 );
    }
}
#if 0
// XAnimClearTree: tears a runtime tree down completely: pool nodes (XAnimClearTree_r) plus the two interned part-remap tables (second table at tree+8+2n+(3n+1)).
int __cdecl XAnimClearTree(_DWORD *a1)
{
  int v1;
  char *v2;
  int i;
  unsigned __int16 v4;
  int v5;
  int result;
  int v7;
  char *v8;

  XAnimClearTree_r(a1, 0);
  v1 = *(_DWORD *)(*a1 + 4);
  v2 = (char *)a1 + 2 * v1 + 8;
  v8 = (char *)a1 + 3 * v1 + 2 * v1 + 9;
  v7 = 2;
  do
  {
    for ( i = *(_DWORD *)(*a1 + 4) - 1; i >= 0; --i )
    {
      v4 = *(_WORD *)&v2[2 * i];
      if ( v4 )
      {
        v5 = GetRefString_var + 8 * v4;
        if ( *(_WORD *)v5 )
          --*(_WORD *)v5;
        else
          SL_FreeString(v4, (char *)(v5 + 4), **(__int16 **)(*(_DWORD *)(*(_DWORD *)(*a1 + 8 * i + 12) + 4) + 12) + 16);
        *(_WORD *)&v2[2 * i] = 0;
      }
    }
    v2 = v8;
    result = --v7;
  }
  while ( v7 );
  return result;
}
#endif

/* ---- cand_XAnimClearSecondaryTiming  0x00488DA0 ----  [HIGH] */
int __cdecl cand_XAnimClearSecondaryTiming(int result)
{
  *(_DWORD *)(result + 40) = 0;
  *(_WORD *)(result + 48) = 0;
  *(_DWORD *)(result + 44) = 0;
  *(_WORD *)(result + 50) = 0;
  return result;
}

/* ---- cand_XAnimClearPrimaryTiming  0x00488DC0 ----  VERIFIED */
int __cdecl cand_XAnimClearPrimaryTiming(int result)
{
  *(_DWORD *)(result + 12) = 0;
  *(_WORD *)(result + 20) = 0;
  *(_DWORD *)(result + 16) = 0;
  *(_WORD *)(result + 22) = 0;
  return result;
}

/* ---- cand_XAnimFindHeaviestLeaf  0x00488DE0 ----  VERIFIED */
int __cdecl cand_XAnimFindHeaviestLeaf(int a1)
{
  int result;
  unsigned __int16 *v2;
  int v3;
  int v4;
  int v5;
  unsigned __int16 *v6;
  int v7;
  int v8;
  float v9;
  int v10;
  float v11;

  result = a1;
  v2 = (unsigned __int16 *)(*(_DWORD *)xanim_currentTree + 8 * a1 + 8);
  v3 = 0;
  if ( *v2 )
  {
    v11 = 0.0;
    v8 = 0;
    v10 = *v2;
    v4 = v2[3];
    v5 = 28 * xanim_activePoolSlot;
    v6 = (unsigned __int16 *)(xanim_currentTree + 2 * v4 + 8);
    do
    {
      v9 = *(float *)(68 * *v6 + v5 + 10987068);
      if ( v9 > (double)v11 )
      {
        v7 = cand_XAnimFindHeaviestLeaf(v4 + v3);
        if ( v7 )
        {
          v11 = v9;
          v8 = v7;
        }
      }
      ++v3;
      ++v6;
    }
    while ( v3 < v10 );
    return v8;
  }
  return result;
}

/* ---- XAnimSetAnimInternal_m  0x00488E80 ----  VERIFIED */
int __cdecl XAnimSetAnimInternal_m( int nodeIndex, float weight, float blendTime,
                        float rate, char force, unsigned __int16 notifyName,
                        __int16 notifyType )
{
    XAnimTree      *tree = (XAnimTree *)xanim_currentTree;
    XAnimInfo      *node;
    XAnimState     *lane;
    unsigned short  handle;
    float           weightDelta;
    float           blendRemaining;
    int             slot;

    handle = tree->poolNodeHandles[nodeIndex];
    if ( handle ) {
        node = &xanim_pool[handle];
        if ( xanim_activePoolSlot && node->notifyName ) {
            SL_RemoveRefToString( node->notifyName );
        }
    } else {
        if ( weight == 0.0f && !force ) {
            return 0;
        }
        node = XAnimAllocInfo( tree, nodeIndex );
        Com_Memset( node->states, 0, 2 * sizeof( XAnimState ) );
        node->notifyName       = 0;
        node->notifyIndex      = -1;
        node->notifyChildIndex = 0;
        node->notifyType       = 0;
    }

    if ( nodeIndex == 0 ) {
        weight    = 1.0f;
        blendTime = 0.0f;
        rate      = 1.0f;
    }

    slot = xanim_activePoolSlot;
    lane = &node->states[slot];

    weightDelta = weight - lane->currentWeight;
    *(int *)&weightDelta &= 0x7FFFFFFF;

    lane->targetWeight             = weight;
    blendRemaining                 = weightDelta * blendTime;
    lane->weightBlendTimeRemaining = blendRemaining;

    if ( blendRemaining < 0.001f ) {
        lane->weightBlendTimeRemaining = 0.0f;
        lane->currentWeight            = weight;
    } else if ( lane->currentWeight == 0.0f ) {
        lane->currentWeight = weight * 0.001f;
    }
    lane->rateScale = rate;

    if ( slot ) {
        XAnimEntry *entry = &tree->sourceTree->entries[nodeIndex];

        node->notifyName = notifyName;
        if ( notifyName ) {
            SL_AddRefToString( notifyName );
        }
        node->notifyIndex = -1;

        if ( !notifyName || entry->childCount == 0
          || ( entry->u.parent.flags & 3 ) == 0 ) {
            node->notifyChildIndex = 0;
            return 0;
        }

        node->notifyChildIndex = (unsigned short)cand_XAnimFindHeaviestLeaf( nodeIndex );
        if ( !node->notifyChildIndex ) {
            return 2;
        }
    } else {
        node->notifyType = notifyType;
    }
    return 0;
}
#if 0
// XAnimSetAnimInternal_m: installs weight, blend time, rate and notify name on one node, allocating its pool node on demand.  Common worker of every XAnimSetAnim* path.
int __cdecl XAnimSetAnimInternal_m(int a1, float a2, float a3, float a4, char a5, unsigned __int16 a6, __int16 a7)
{
  unsigned __int16 v7;
  char *v8;
  unsigned __int16 v9;
  double v10;
  int v11;
  float *v12;
  double v13;
  _DWORD *v14;
  int v15;
  __int16 v16;
  float v18;

  v7 = *(_WORD *)(xanim_currentTree + 2 * a1 + 8);
  if ( v7 )
  {
    v8 = (char *)&xanim_pool + 68 * v7;
    if ( xanim_activePoolSlot )
    {
      v9 = *((_WORD *)v8 + 2);
      if ( v9 )
        SL_RemoveRefToString(v9);
    }
  }
  else
  {
    if ( (a2 == 0.0) | __UNORDERED__(a2, 0.0) && !a5 )
      return 0;
    v8 = XAnimAllocInfo(xanim_currentTree, a1);
    Com_Memset((int *)v8 + 3, 0, 56);
    *((_WORD *)v8 + 2) = 0;
    *((_WORD *)v8 + 1) = -1;
    *(_WORD *)v8 = 0;
    *((_WORD *)v8 + 3) = 0;
  }
  if ( a1 )
  {
    v10 = a4;
  }
  else
  {
    v10 = 1.0;
    a2 = 1.0;
    a3 = 0.0;
  }
  v11 = xanim_activePoolSlot;
  v18 = a2 - *(float *)&v8[28 * xanim_activePoolSlot + 32];
  v12 = (float *)&v8[28 * xanim_activePoolSlot];
  v12[7] = a2;
  v13 = COERCE_FLOAT(LODWORD(v18) & 0x7FFFFFFF) * a3;
  v12[6] = v13;
  if ( (v13 < 0.001) | __UNORDERED__(v13, 0.001) )
  {
    v12[6] = 0.0;
    v12[8] = a2;
  }
  else if ( (v12[8] == 0.0) | __UNORDERED__(v12[8], 0.0) )
  {
    v12[8] = a2 * 0.001;
  }
  v12[9] = v10;
  if ( v11 )
  {
    *((_WORD *)v8 + 2) = a6;
    if ( a6 )
      ++*(_WORD *)(GetRefString_var + 8 * a6);
    v14 = (_DWORD *)xanim_currentTree;
    *((_WORD *)v8 + 1) = -1;
    v15 = *v14 + 8 * a1 + 8;
    if ( !a6 || !*(_WORD *)v15 || (*(_BYTE *)(v15 + 4) & 3) == 0 )
    {
      *(_WORD *)v8 = 0;
      return 0;
    }
    v16 = cand_XAnimFindHeaviestLeaf(a1);
    *(_WORD *)v8 = v16;
    if ( !v16 )
      return 2;
  }
  else
  {
    *((_WORD *)v8 + 3) = a7;
  }
  return 0;
}
#endif

/* ---- XAnimSetRate_m  0x00489010 ----  VERIFIED */
int __cdecl XAnimSetRate_m(int nodeIndex, float rate)
{
    XAnimTree     *tree = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];

    XANIM_LANE( handle ).rateScale = rate;
    return 28 * xanim_activePoolSlot;
}
#if 0
// XAnimSetRate_m: sets a node's playback rate scale (XAnimState.rateScale at +0x18 of the active lane).
int __cdecl XAnimSetRate_m(int nodeIndex, float rate)
{
  int result;

  result = 28 * xanim_activePoolSlot;
  *((float *)&unk_A7A644 + 17 * *(unsigned __int16 *)(xanim_currentTree + 2 * nodeIndex + 8) + 7 * xanim_activePoolSlot) = rate;
  return result;
}
#endif

/* ---- XAnimSetupSyncNodes_r  0x00489040 ----  VERIFIED */
char __cdecl XAnimSetupSyncNodes_r(_DWORD *a1, int a2, int a3)
{
  int v3;
  _DWORD *v4;
  _DWORD *v5;
  __int16 v6;
  int v7;
  int v8;

  v3 = LOWORD(a1[2 * a2 + 2]);
  v4 = &a1[2 * a2 + 2];
  if ( *(_WORD *)v4 )
  {
    if ( (a1[2 * a2 + 3] & 3) != 0 )
    {
      v7 = 0;
      do
      {
        v4 = &a1[2 * *((unsigned __int16 *)v4 + 3) + 2];
        ++v7;
      }
      while ( *(_WORD *)v4 );
      Com_Error(ERR_DROP, &byte_55A418, *a1, v7, *(_DWORD *)v4[1]);
    }
    v8 = 0;
    v6 = ((_BYTE)a3 == 0) + 1;
    LOWORD(a1[2 * a2 + 3]) |= v6;
    if ( v3 > 0 )
    {
      do
      {
        LOBYTE(v6) = XAnimSetupSyncNodes_r(a1, v8 + HIWORD(a1[2 * a2 + 3]), a3);
        ++v8;
      }
      while ( v8 < v3 );
    }
  }
  else
  {
    v5 = (_DWORD *)a1[2 * a2 + 3];
    LOBYTE(v6) = a3;
    if ( *(_BYTE *)(v5[1] + 2) != (_BYTE)a3 )
    {
      if ( (_BYTE)a3 )
        Com_Error(ERR_DROP, &byte_55A4A0, *v5, *a1);
      Com_Error(ERR_DROP, &byte_55A460, *v5, *a1);
    }
  }
  return v6;
}

/* ---- XAnimCheckSyncNodes_r  0x00489110 ----  VERIFIED */
void __cdecl XAnimCheckSyncNodes_r(_DWORD *a1, int a2)
{
  _DWORD *v2;
  int v3;
  int v4;
  int v5;
  int v6;

  v2 = a1;
  v3 = LOWORD(a1[2 * a2 + 2]);
  if ( LOWORD(a1[2 * a2 + 2]) )
  {
    v4 = a1[2 * a2 + 3] & 3;
    if ( (a1[2 * a2 + 3] & 3) != 0 )
    {
      if ( v4 == 3 )
        Com_Error(ERR_DROP, &byte_55A3E0);
      LOBYTE(a1[2 * a2 + 3]) |= 4u;
      v5 = 0;
      for ( LOBYTE(a1) = v4 == 1; v5 < v3; ++v5 )
        XAnimSetupSyncNodes_r(v2, v5 + HIWORD(v2[2 * a2 + 3]), (int)a1);
    }
    else
    {
      v6 = 0;
      if ( LOWORD(a1[2 * a2 + 2]) )
      {
        do
        {
          XAnimCheckSyncNodes_r(a1, v6 + HIWORD(a1[2 * a2 + 3]));
          ++v6;
        }
        while ( v6 < v3 );
      }
    }
  }
}

/* ---- XAnimSetupSyncNodes  0x004891B0 ----  [HIGH] */
void __cdecl XAnimSetupSyncNodes(_DWORD *a1)
{
  XAnimCheckSyncNodes_r(a1, 0);
}

/* ---- XAnimIsPrimitive  0x004891E0 ----  [HIGH] */
BOOL __fastcall XAnimIsPrimitive(int a1, int a2)
{
  return *(_WORD *)(a2 + 8 * a1 + 8) == 0;
}

/* ---- XAnimSetTime_m  0x004891F0 ----  VERIFIED */
__int16 __cdecl XAnimSetTime_m( int node, XAnimTree *tree, int timeBits )
{
    unsigned short handle = tree->poolNodeHandles[node];

    if ( handle ) {
        XAnimState *lane = &XANIM_LANE( handle );

        *(int *)&lane->time    = timeBits;
        lane->cycleCount       = 0;
        *(int *)&lane->oldTime = timeBits;
        lane->oldCycleCount    = 0;
    }
    return (__int16)handle;
}
#if 0
// XAnimSetTime_m: hard-sets a node's animation time (writes XAnimState time/oldTime and zeroes both cycle counts).  Cgame trap 145.
__int16 __cdecl XAnimSetTime_m(int *a1, int a2, int a3)
{
  LOWORD(a1) = *(_WORD *)(a2 + 2 * (_DWORD)a1 + 8);
  if ( (_WORD)a1 )
  {
    a1 = &dword_A7A62C[17 * (unsigned __int16)a1 + 7 * xanim_activePoolSlot];
    *a1 = a3;
    *((_WORD *)a1 + 4) = 0;
    a1[1] = a3;
    *((_WORD *)a1 + 5) = 0;
  }
  return (__int16)a1;
}
#endif

/* ---- cand_XAnimCopyTiming  0x00489230 ----  VERIFIED */
int __fastcall cand_XAnimCopyTiming(int a1, int a2)
{
  int result;

  result = 28 * xanim_activePoolSlot;
  *(_DWORD *)(result + a1 + 12) = *(_DWORD *)(28 * xanim_activePoolSlot + a2 + 12);
  *(_WORD *)(result + a1 + 20) = *(_WORD *)(result + a2 + 20);
  *(_DWORD *)(result + a1 + 16) = *(_DWORD *)(result + a2 + 16);
  *(_WORD *)(result + a1 + 22) = *(_WORD *)(result + a2 + 22);
  return result;
}

/* ---- XAnimSetUser  0x00489260 ----  [CONFIRMED] */
int __cdecl XAnimSetUser(int result)
{
  xanim_activePoolSlot = result;
  return result;
}

/* ---- XAnimUpdateNotifyIndex_m  0x00489270 ----  VERIFIED */
int __cdecl XAnimUpdateNotifyIndex_m( int node )
{
    XAnimTree  *tree = (XAnimTree *)xanim_currentTree;
    XAnimInfo  *info = &xanim_pool[tree->poolNodeHandles[node]];
    XAnimEntry *entry;

    if ( !info->notifyName ) {
        return (__int16)node;
    }

    if ( *(const int *)&info->states[1].time == 1065353216 ) {
        info->notifyIndex = -1;
        return (__int16)node;
    }

    entry = &tree->sourceTree->entries[node];
    if ( entry->childCount != 0 ) {
        if ( !info->notifyChildIndex ) {
            return (__int16)info->notifyChildIndex;
        }
        entry = &tree->sourceTree->entries[info->notifyChildIndex];
    }

    info->notifyIndex = (short)XAnimGetNextNotifyTime( (int)entry, (int)info,
                                           info->states[1].time );
    return info->notifyIndex;
}
#if 0
// XAnimUpdateNotifyIndex_m: recomputes which notetrack a node will hit next (XAnimInfo.notifyIndex at +0x02).
__int16 __cdecl XAnimUpdateNotifyIndex_m(int a1)
{
  float *v1;
  bool v2; // zf
  int v3;

  v1 = (float *)((char *)&xanim_pool + 68 * *(unsigned __int16 *)(xanim_currentTree + 2 * a1 + 8));
  if ( *((_WORD *)v1 + 2) )
  {
    if ( *((_DWORD *)v1 + 10) != 1065353216 )
    {
      v2 = *(_WORD *)(*(_DWORD *)xanim_currentTree + 8 * a1 + 8) == 0;
      v3 = *(_DWORD *)xanim_currentTree + 8 * a1 + 8;
      if ( !v2 )
      {
        LOWORD(a1) = *(_WORD *)v1;
        if ( !*(_WORD *)v1 )
          return a1;
        v3 = *(_DWORD *)xanim_currentTree + 8 * (unsigned __int16)a1 + 8;
      }
      LOWORD(a1) = XAnimGetNextNotifyTime(v3, (int)v1, v1[10]);
      *((_WORD *)v1 + 1) = a1;
      return a1;
    }
    *((_WORD *)v1 + 1) = -1;
  }
  return a1;
}
#endif

/* ---- XAnimPushTimingToSubtree_m  0x004892D0 ----  VERIFIED */
int __cdecl XAnimPushTimingToSubtree_m(int a1, int a2)
{
  int result;
  unsigned __int16 *v3;
  int v4;
  int v5;
  int v6;
  int v7;
  char *v8;
  bool v9;
  int v10;

  result = xanim_currentTree;
  v3 = (unsigned __int16 *)(*(_DWORD *)xanim_currentTree + 8 * a1 + 8);
  v4 = 0;
  v10 = 0;
  if ( *v3 )
  {
    do
    {
      HIWORD(result) = HIWORD(xanim_currentTree);
      v5 = v4 + v3[3];
      LOWORD(result) = *(_WORD *)(xanim_currentTree + 2 * v5 + 8);
      if ( (_WORD)result )
      {
        v6 = (unsigned __int16)result;
        v7 = 28 * xanim_activePoolSlot;
        v8 = (char *)&xanim_pool + 68 * v6;
        v9 = xanim_activePoolSlot == 0;
        *(_DWORD *)&v8[v7 + 12] = *(_DWORD *)(28 * xanim_activePoolSlot + a2 + 12);
        *(_WORD *)&v8[v7 + 20] = *(_WORD *)(v7 + a2 + 20);
        *(_DWORD *)&v8[v7 + 16] = *(_DWORD *)(v7 + a2 + 16);
        *(_WORD *)&v8[v7 + 22] = *(_WORD *)(v7 + a2 + 22);
        if ( !v9 )
          XAnimUpdateNotifyIndex_m(v5);
        result = XAnimPushTimingToSubtree_m(v5, a2);
        v4 = v10;
      }
      v10 = ++v4;
    }
    while ( v4 < *v3 );
  }
  return result;
}

/* ---- XAnimReseedTiming_m  0x00489370 ----  VERIFIED */
void __cdecl XAnimReseedTiming_m( int nodeIndex, int forceClear )
{
    XAnimTree *tree = (XAnimTree *)xanim_currentTree;
    XAnim     *source;
    XAnimInfo *poolNode = 0;
    XAnimState *state;
    int        walk;
    int        cur;
    int        found = 0;

    if ( nodeIndex != 0 ) {
        source = tree->sourceTree;
        walk   = nodeIndex;

        for ( ;; ) {
            XAnimEntry *entry = &source->entries[walk];

            poolNode = &xanim_pool[ tree->poolNodeHandles[walk] ];

            if ( entry->childCount != 0 && ( entry->u.parent.flags & 4 ) != 0 ) {
                found = 1;
                break;
            }
            if ( entry->parentIndex == 0 ) {
                break;
            }
            walk = entry->parentIndex;
        }
    }

    if ( !found ) {
        source = tree->sourceTree;
        if ( source->entries[nodeIndex].childCount != 0 ) {
            return;
        }
        if ( forceClear == 0
          && XAnimParentChainHasWeight_m( nodeIndex, tree )
          && XAnimSubtreeHasWeight_m( tree, nodeIndex ) ) {
            return;
        }

        state = &XANIM_LANE( tree->poolNodeHandles[nodeIndex] );
        state->time          = 0.0f;
        state->cycleCount    = 0;
        state->oldTime       = 0.0f;
        state->oldCycleCount = 0;
        return;
    }

    if ( forceClear != 0
      || !XAnimParentChainHasWeight_m( walk, tree )
      || !XAnimSubtreeHasWeight_m( tree, walk ) ) {
        state = &poolNode->states[xanim_activePoolSlot];
        state->time          = 0.0f;
        state->cycleCount    = 0;
        state->oldTime       = 0.0f;
        state->oldCycleCount = 0;
    }

    XAnimPushTimingToSubtree_m( nodeIndex, (int)poolNode );

    if ( nodeIndex == walk ) {
        return;
    }

    {
        const XAnimState *src = &poolNode->states[xanim_activePoolSlot];
        const float          *srcTime          = &src->time;
        const unsigned short *srcCycleCount    = &src->cycleCount;
        const float          *srcOldTime       = &src->oldTime;
        const unsigned short *srcOldCycleCount = &src->oldCycleCount;

        cur = nodeIndex;
        do {
            XAnimInfo *node = &xanim_pool[ tree->poolNodeHandles[cur] ];

            state = &node->states[xanim_activePoolSlot];
            state->time          = *srcTime;
            state->cycleCount    = *srcCycleCount;
            state->oldTime       = *srcOldTime;
            state->oldCycleCount = *srcOldCycleCount;

            if ( xanim_activePoolSlot && node->notifyName != 0 ) {
                if ( *(const int *)&node->states[1].time == 1065353216 ) {
                    node->notifyIndex = -1;
                } else {
                    XAnimEntry *entry = &tree->sourceTree->entries[cur];
                    int         armed = 1;

                    if ( entry->childCount != 0 ) {
                        if ( node->notifyChildIndex == 0 ) {
                            armed = 0;
                        } else {
                            entry = &tree->sourceTree->entries[
                                        node->notifyChildIndex];
                        }
                    }
                    if ( armed ) {
                        node->notifyIndex = (short)XAnimGetNextNotifyTime(
                            (int)entry, (int)node, node->states[1].time );
                    }
                }
            }

            cur = tree->sourceTree->entries[cur].parentIndex;
        } while ( cur != walk );
    }
}
#if 0
// XAnimReseedTiming_m: applies per-anim flags and notify wiring after an animation is installed.  Reached from every set-anim path after XAnimSetAnimInternal_m.
char __cdecl XAnimReseedTiming_m(int a1, int a2)
{
  int v2;
  int v3;
  _DWORD *v4;
  int i;
  char *v6;
  int *v7;
  int v8;
  int v9;
  char *v10;
  _WORD *v11;
  int *v13;
  char *v14;
  int *v15;
  char *v16;

  v2 = a1;
  v3 = a1;
  v4 = (_DWORD *)xanim_currentTree;
  if ( a1 )
  {
    for ( i = *(_DWORD *)xanim_currentTree; ; v3 = *(unsigned __int16 *)(i + 8 * v3 + 10) )
    {
      v6 = (char *)&xanim_pool + 68 * *(unsigned __int16 *)(xanim_currentTree + 2 * v3 + 8);
      if ( *(_WORD *)(i + 8 * v3 + 8) )
      {
        if ( (*(_BYTE *)(i + 8 * v3 + 12) & 4) != 0 )
          break;
      }
      a1 = *(unsigned __int16 *)(i + 8 * v3 + 10);
      if ( !*(_WORD *)(i + 8 * v3 + 10) )
        goto LABEL_7;
    }
    if ( a2 || !XAnimParentChainHasWeight_m(v3, (_DWORD *)xanim_currentTree) || !XAnimSubtreeHasWeight_m(v4, v3) )
    {
      if ( xanim_activePoolSlot )
      {
        *((_DWORD *)v6 + 10) = 0;
        *((_WORD *)v6 + 24) = 0;
        *((_DWORD *)v6 + 11) = 0;
        *((_WORD *)v6 + 25) = 0;
      }
      else
      {
        *((_DWORD *)v6 + 3) = 0;
        *((_WORD *)v6 + 10) = 0;
        *((_DWORD *)v6 + 4) = 0;
        *((_WORD *)v6 + 11) = 0;
      }
    }
    LOBYTE(v7) = XAnimPushTimingToSubtree_m(v2, (int)v6);
    if ( v2 != v3 )
    {
      v9 = 7 * xanim_activePoolSlot;
      v13 = (int *)&v6[28 * xanim_activePoolSlot + 12];
      v14 = &v6[28 * xanim_activePoolSlot + 20];
      v15 = (int *)&v6[28 * xanim_activePoolSlot + 16];
      v16 = &v6[28 * xanim_activePoolSlot + 22];
      do
      {
        v7 = (int *)((char *)&xanim_pool + 68 * *((unsigned __int16 *)v4 + v2 + 4));
        v7[v9 + 3] = *v13;
        LOWORD(v7[v9 + 5]) = *(_WORD *)v14;
        v7[v9 + 4] = *v15;
        HIWORD(v7[v9 + 5]) = *(_WORD *)v16;
        LOBYTE(v7) = xanim_activePoolSlot;
        if ( !xanim_activePoolSlot )
          goto LABEL_29;
        v10 = (char *)&xanim_pool + 68 * *((unsigned __int16 *)v4 + v2 + 4);
        if ( !*((_WORD *)v10 + 2) )
          goto LABEL_29;
        if ( *((_DWORD *)v10 + 10) == 1065353216 )
        {
          *((_WORD *)v10 + 1) = -1;
        }
        else
        {
          v11 = (_WORD *)(*v4 + 8 * v2 + 8);
          if ( !*v11 )
            goto LABEL_28;
          LOWORD(v7) = *(_WORD *)v10;
          if ( *(_WORD *)v10 )
          {
            v11 = (_WORD *)(*v4 + 8 * (unsigned __int16)v7 + 8);
LABEL_28:
            LOWORD(v7) = XAnimGetNextNotifyTime((int)v11, (int)v10, *((float *)v10 + 10));
            *((_WORD *)v10 + 1) = (_WORD)v7;
          }
        }
LABEL_29:
        v2 = *(unsigned __int16 *)(*v4 + 8 * v2 + 10);
      }
      while ( v2 != a1 );
    }
  }
  else
  {
LABEL_7:
    v7 = *(int **)xanim_currentTree;
    if ( !*(_WORD *)(*(_DWORD *)xanim_currentTree + 8 * v2 + 8) )
    {
      if ( a2 || !XAnimParentChainHasWeight_m(v2, (_DWORD *)xanim_currentTree) || (LOBYTE(v7) = XAnimSubtreeHasWeight_m(v4, v2), !(_BYTE)v7) )
      {
        v8 = *((unsigned __int16 *)v4 + v2 + 4);
        if ( xanim_activePoolSlot )
          v7 = &dword_A7A648[17 * v8];
        else
          v7 = &dword_A7A62C[17 * v8];
        *v7 = 0;
        *((_WORD *)v7 + 4) = 0;
        v7[1] = 0;
        *((_WORD *)v7 + 5) = 0;
      }
    }
  }
  return (char)v7;
}
#endif

/* ---- cand_XAnimSeedParentChain  0x00489570 ----  VERIFIED */
int __cdecl cand_XAnimSeedParentChain(int result, int a2)
{
  int i;

  for ( i = result; i; result = XAnimSetAnimInternal_m(i, 0.0, *(float *)&a2, 1.0, 1, 0, 0) )
  {
    result = xanim_currentTree;
    i = *(unsigned __int16 *)(*(_DWORD *)xanim_currentTree + 8 * i + 10);
    if ( *(_WORD *)(xanim_currentTree + 2 * i + 8) )
      break;
  }
  return result;
}

/* ---- XAnimSetAnimInternalLimited_m  0x004895C0 ----  VERIFIED */
int __cdecl XAnimSetAnimInternalLimited_m(
        __int16 a1,
        unsigned __int16 a2,
        int a3,
        int a4,
        float a5,
        int a6,
        float a7,
        int a8)
{
  int v8;

  xanim_currentTree = a3;
  if ( (a5 < 0.001) | __UNORDERED__(a5, 0.001) )
    a5 = 0.0;
  v8 = XAnimSetAnimInternal_m(a4, a5, *(float *)&a6, a7, 0, a2, a1);
  cand_XAnimSeedParentChain(a4, a6);
  XAnimReseedTiming_m(a4, a8);
  if ( xanim_activePoolSlot )
    XAnimUpdateNotifyIndex_m(a4);
  return v8;
}

/* ---- XAnimSetAnimRate_m  0x00489630 ----  VERIFIED */
int __cdecl XAnimSetAnimRate_m( XAnimTree *tree, int node, float rate )
{
    unsigned short handle;

    handle = tree->poolNodeHandles[node];
    xanim_currentTree = (int)tree;
    XANIM_LANE( handle ).rateScale = rate;
    return 28 * xanim_activePoolSlot;
}
#if 0
// XAnimSetAnimRate_m: sets a node's rate scale; the same rateScale write as XAnimSetRate_m but with an explicit tree argument.  Cgame trap 142.
int __cdecl XAnimSetAnimRate_m(int a1, int a2, float a3)
{
  int v3;
  int result;

  v3 = *(unsigned __int16 *)(a1 + 2 * a2 + 8);
  xanim_currentTree = a1;
  result = 28 * xanim_activePoolSlot;
  *((float *)&unk_A7A644 + 17 * v3 + 7 * xanim_activePoolSlot) = a3;
  return result;
}
#endif

/* ---- XAnimIsLooped_m  0x00489660 ----  VERIFIED */
int __cdecl XAnimIsLooped_m(int a1, int a2)
{
  if ( *(_WORD *)(a2 + 8 * a1 + 8) )
    return *(_BYTE *)(a2 + 8 * a1 + 12) & 1;
  else
    return *(unsigned __int8 *)(*(_DWORD *)(*(_DWORD *)(a2 + 8 * a1 + 12) + 4) + 2);
}

/* ---- XAnimSetAnimKnobInternal_m  0x004896B0 ----  VERIFIED */
__int16 __cdecl XAnimSetAnimKnobInternal_m( unsigned __int16 notifyName, XAnimTree *tree,
                            int node, float weight, int blendTimeBits,
                            float rate, __int16 notifyType, int animFlags )
{
    int ancestor;

    xanim_currentTree = (int)tree;
    if ( weight < 0.001f ) {
        weight = 0.0f;
    }

    XAnimSetAnimInternal_m( node, weight, *(float *)&blendTimeBits, rate, 0,
                notifyName, notifyType );

    if ( node ) {
        ancestor = node;
        do {
            unsigned short handle;

            ancestor = tree->sourceTree->entries[ancestor].parentIndex;
            handle   = tree->poolNodeHandles[ancestor];
            if ( handle && XANIM_LANE( handle ).targetWeight != 0.0f ) {
                continue;
            }
            XAnimSetAnimInternal_m( ancestor, 1.0f, *(float *)&blendTimeBits, 1.0f,
                        0, 0, 0 );
        } while ( ancestor );
    }

    XAnimReseedTiming_m( node, animFlags );
    if ( xanim_activePoolSlot ) {
        return (__int16)XAnimUpdateNotifyIndex_m( node );
    }
    return (__int16)xanim_activePoolSlot;
}
#if 0
// XAnimSetAnimKnobInternal_m: knob set-anim worker for XAnimSetAnimKnob_m; seeds unweighted ancestors at full weight.
__int16 __cdecl XAnimSetAnimKnobInternal_m(
        unsigned __int16 a1,
        _DWORD *a2,
        int a3,
        float a4,
        int a5,
        float a6,
        __int16 a7,
        int a8)
{
  int v8;
  unsigned __int16 v9;
  double v10;
  __int16 result;

  xanim_currentTree = (int)a2;
  if ( (a4 < 0.001) | __UNORDERED__(a4, 0.001) )
    a4 = 0.0;
  v8 = a3;
  XAnimSetAnimInternal_m(a3, a4, *(float *)&a5, a6, 0, a1, a7);
  if ( a3 )
  {
    do
    {
      v8 = *(unsigned __int16 *)(*a2 + 8 * v8 + 10);
      v9 = *((_WORD *)a2 + v8 + 4);
      if ( v9 )
      {
        v10 = *((float *)&unk_A7A63C + 17 * v9 + 7 * xanim_activePoolSlot);
        if ( !((v10 == 0.0) | __UNORDERED__(v10, 0.0)) )
          continue;
      }
      XAnimSetAnimInternal_m(v8, 1.0, *(float *)&a5, 1.0, 0, 0, 0);
    }
    while ( v8 );
  }
  XAnimReseedTiming_m(a3, a8);
  result = xanim_activePoolSlot;
  if ( xanim_activePoolSlot )
    return XAnimUpdateNotifyIndex_m(a3);
  return result;
}
#endif

/* ---- cand_XAnimReadTransform  0x00489780 ----  VERIFIED */
float *__cdecl cand_XAnimReadTransform(float *this)
{
  int v1;
  double v2;
  __int16 v3;
  double v4;
  float *result;

  v1 = scrSave_readPos + 4;
  *this = *(float *)scrSave_readPos;
  v1 += 4;
  v2 = *(float *)(v1 - 4);
  v1 += 2;
  this[1] = v2;
  *((_WORD *)this + 4) = *(_WORD *)(v1 - 2);
  v3 = *(_WORD *)v1;
  v1 += 2;
  *((_WORD *)this + 5) = v3;
  this[4] = *(float *)v1;
  v1 += 4;
  this[6] = *(float *)v1;
  v1 += 8;
  v4 = *(float *)(v1 - 4);
  result = (float *)(v1 + 4);
  this[3] = v4;
  scrSave_readPos = (int)result;
  this[5] = *(result - 1);
  return result;
}

/* ---- cand_XAnimWriteState  0x004897E0 ----  VERIFIED */
int *__cdecl cand_XAnimWriteState(int *a1)
{
  int v1;
  int v2;
  int *v3;
  __int16 v4;
  _WORD *v5;
  __int16 v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int *result;
  int v12;
  int v13;
  int v14;

  v1 = currentPos + 4;
  v12 = *a1;
  *(_DWORD *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 4)) = v12;
  v2 = a1[1];
  currentPos = v1;
  v1 += 4;
  v13 = v2;
  v3 = (int *)(currentPos + Hunk_ReallocateTempMemory(v1));
  currentPos = v1;
  *v3 = v13;
  v4 = *((_WORD *)a1 + 4);
  v1 += 2;
  v5 = (_WORD *)(currentPos + Hunk_ReallocateTempMemory(v1));
  currentPos = v1;
  *v5 = v4;
  v6 = *((_WORD *)a1 + 5);
  v1 += 2;
  *(_WORD *)(currentPos + Hunk_ReallocateTempMemory(v1)) = v6;
  v7 = a1[4];
  currentPos = v1;
  v1 += 4;
  *(_DWORD *)(currentPos + Hunk_ReallocateTempMemory(v1)) = v7;
  v8 = a1[6];
  currentPos = v1;
  v1 += 4;
  *(_DWORD *)(currentPos + Hunk_ReallocateTempMemory(v1)) = v8;
  v9 = a1[3];
  currentPos = v1;
  v1 += 4;
  *(_DWORD *)(currentPos + Hunk_ReallocateTempMemory(v1)) = v9;
  v10 = a1[5];
  currentPos = v1;
  v1 += 4;
  v14 = v10;
  result = (int *)(currentPos + Hunk_ReallocateTempMemory(v1));
  currentPos = v1;
  *result = v14;
  return result;
}

/* ---- cand_XAnimReadString  0x004898F0 ----  VERIFIED */
__int16 __cdecl cand_XAnimReadString(int a1)
{
  __int16 *v1;
  __int16 v2;
  __int16 v3;
  char *v4;
  unsigned int v5;
  __int16 StringOfLen;
  int v7;
  double v8;
  double v9;
  __int16 result;
  double v11;

  v1 = (__int16 *)(scrSave_readPos + 2);
  *(_WORD *)(a1 + 2) = *(_WORD *)scrSave_readPos;
  v2 = *v1++;
  *(_WORD *)a1 = v2;
  v3 = *v1++;
  *(_WORD *)(a1 + 6) = v3;
  LOBYTE(v3) = *(_BYTE *)v1;
  v4 = (char *)v1 + 1;
  scrSave_readPos = (int)v4;
  if ( (_BYTE)v3 )
  {
    v5 = strlen(v4);
    /* type 14 -- `push 0Eh` at 0x0048992F. */
    StringOfLen = SL_GetStringOfLen(v4, 0, v5 + 1, 14);
    v4 = (char *)(v5 + 1 + scrSave_readPos);
  }
  else
  {
    StringOfLen = 0;
  }
  *(_WORD *)(a1 + 4) = StringOfLen;
  *(float *)(a1 + 12) = *(float *)v4;
  v7 = (int)(v4 + 4);
  v8 = *(float *)v7;
  v7 += 4;
  *(float *)(a1 + 16) = v8;
  *(_WORD *)(a1 + 20) = *(_WORD *)v7;
  *(_WORD *)(a1 + 22) = *(_WORD *)(v7 + 2);
  v7 += 2;
  *(float *)(a1 + 28) = *(float *)(v7 + 2);
  v7 += 6;
  *(float *)(a1 + 36) = *(float *)v7;
  v7 += 8;
  *(float *)(a1 + 24) = *(float *)(v7 - 4);
  v7 += 8;
  *(float *)(a1 + 32) = *(float *)(v7 - 8);
  v7 += 6;
  *(float *)(a1 + 40) = *(float *)(v7 - 10);
  v7 += 6;
  v9 = *(float *)(v7 - 12);
  v7 += 4;
  *(float *)(a1 + 44) = v9;
  *(_WORD *)(a1 + 48) = *(_WORD *)(v7 - 12);
  result = *(_WORD *)(v7 - 10);
  *(_WORD *)(a1 + 50) = result;
  *(float *)(a1 + 56) = *(float *)(v7 - 8);
  v7 += 4;
  v11 = *(float *)(v7 - 8);
  v7 += 4;
  *(float *)(a1 + 64) = v11;
  scrSave_readPos = v7;
  *(float *)(a1 + 52) = *(float *)(v7 - 8);
  *(float *)(a1 + 60) = *(float *)(v7 - 4);
  return result;
}

/* ---- cand_XAnimWriteInfo  0x004899F0 ----  VERIFIED */
int *__cdecl cand_XAnimWriteInfo(int *a1)
{
  __int16 v1;
  int v2;
  _WORD *v3;
  __int16 v4;
  _WORD *v5;
  __int16 v6;
  _WORD *v7;
  int v8;
  unsigned __int16 v9;
  _BYTE *v11;

  v1 = *((_WORD *)a1 + 1);
  v2 = currentPos + 2;
  v3 = (_WORD *)(currentPos + Hunk_ReallocateTempMemory(currentPos + 2));
  currentPos = v2;
  *v3 = v1;
  v4 = *(_WORD *)a1;
  v2 += 2;
  v5 = (_WORD *)(currentPos + Hunk_ReallocateTempMemory(v2));
  currentPos = v2;
  *v5 = v4;
  v6 = *((_WORD *)a1 + 3);
  v2 += 2;
  v7 = (_WORD *)(currentPos + Hunk_ReallocateTempMemory(v2));
  currentPos = v2;
  *v7 = v6;
  v8 = v2 + 1;
  if ( *((_WORD *)a1 + 2) )
  {
    *(_BYTE *)(currentPos + Hunk_ReallocateTempMemory(v8)) = 1;
    v9 = *((_WORD *)a1 + 2);
    currentPos = v8;
    WriteString(v9);
  }
  else
  {
    v11 = (_BYTE *)(currentPos + Hunk_ReallocateTempMemory(v8));
    currentPos = v8;
    *v11 = 0;
  }
  cand_XAnimWriteState(a1 + 3);
  return cand_XAnimWriteState(a1 + 10);
}

/* ---- cand_XAnimReadInfo  0x00489AB0 ----  VERIFIED */
void __cdecl cand_XAnimReadInfo( XAnimInfo *dst, const void *src )
{
  qmemcpy( dst, src, 0x44u );

  if ( dst->notifyName )
    ++*(unsigned short *)SL_REFSTRING( dst->notifyName );
}

/* ---- cand_XAnimRestoreTree  0x00489AE0 ----  VERIFIED */
unsigned int __cdecl cand_XAnimRestoreTree(int a1)
{
  int v1;
  int v3;
  unsigned int result;
  int i;
  char *v6;
  unsigned char *v8;

  v1 = ((unsigned int)(*(_DWORD *)(*(_DWORD *)a1 + 4) - 1) >> 3) + 1;
  v8 = (unsigned char *)alloca(v1);
  v3 = scrSave_readPos;
  Com_Memcpy((int)v8, (int *)scrSave_readPos, v1);
  result = *(_DWORD *)(*(_DWORD *)a1 + 4);
  scrSave_readPos = v1 + v3;
  for ( i = 0; i < (int)result; ++i )
  {
    if ( ((1 << (i & 7)) & (char)v8[i >> 3]) != 0 )
    {
      v6 = (char *)XAnimAllocInfo((XAnimTree *)a1, i);
      cand_XAnimReadString((int)v6);
    }
    result = *(_DWORD *)(*(_DWORD *)a1 + 4);
  }
  return result;
}

/* ---- cand_XAnimSaveTree  0x00489B70 ----  VERIFIED */
void __cdecl cand_XAnimSaveTree( XAnimTree *tree )
{
  unsigned int   bitmapBytes;
  unsigned char *bitmap;
  unsigned int   i;
  int            newPos;
  char          *dst;

  bitmapBytes = ( ( (unsigned int)tree->sourceTree->nodeCount - 1 ) >> 3 ) + 1;
  bitmap = (unsigned char *)alloca( bitmapBytes );
  Com_Memset( bitmap, 0, bitmapBytes );

  for ( i = 0; i < (unsigned int)tree->sourceTree->nodeCount; i++ )
    if ( tree->poolNodeHandles[i] )
      bitmap[i >> 3] |= 1 << ( i & 7 );

  newPos = currentPos + bitmapBytes;
  dst    = (char *)Hunk_ReallocateTempMemory( newPos ) + currentPos;
  currentPos = newPos;
  Com_Memcpy( dst, bitmap, bitmapBytes );

  for ( i = 0; i < (unsigned int)tree->sourceTree->nodeCount; i++ )
    if ( tree->poolNodeHandles[i] )
      cand_XAnimWriteInfo( (int *)&xanim_pool[ tree->poolNodeHandles[i] ] );
}

/* ---- XAnimCopyTree  0x00489C90 ----  VERIFIED */
void __cdecl XAnimCopyTree( const XAnimTree *from, XAnimTree *to )
{
    int            nodeCount;
    int            i;
    unsigned short srcHandle;
    XAnimInfo     *dst;

    nodeCount = from->sourceTree->nodeCount;

    for ( i = 0 ; i < nodeCount ; i++ ) {
        srcHandle = from->poolNodeHandles[i];

        if ( !srcHandle ) {
            if ( to->poolNodeHandles[i] ) {
                XAnimFreeInfo( to->poolNodeHandles[i] );
                to->poolNodeHandles[i] = 0;
            }
            continue;
        }

        if ( to->poolNodeHandles[i] ) {
            dst = &xanim_pool[ to->poolNodeHandles[i] ];
            if ( dst->notifyName ) {
                XModelReleaseString( dst->notifyName );
            }
        } else {
            dst = XAnimAllocInfo( to, i );
        }

        memcpy( dst, &xanim_pool[srcHandle], sizeof( XAnimInfo ) );

        if ( dst->notifyName ) {
            SL_AddRefToString( dst->notifyName );
        }
    }
}
#if 0
// XAnimCopyTree_m: copies one runtime tree's live pool nodes onto another tree.  Cgame trap 184.
int __cdecl XAnimCopyTree(int result, int a2)
{
  int v2;
  int v3;
  int v4;
  unsigned __int16 *v5;
  unsigned __int16 v6;
  char *v7;
  unsigned __int16 v8;
  int v9;
  unsigned __int16 v10;
  int v11;
  int i;
  unsigned __int16 v13;
  int v14;

  v2 = 0;
  v14 = *(_DWORD *)(*(_DWORD *)result + 4);
  v11 = 0;
  if ( v14 > 0 )
  {
    v3 = a2;
    v4 = result - a2;
    v5 = (unsigned __int16 *)(a2 + 8);
    for ( i = v4; ; v4 = i )
    {
      v6 = *(unsigned __int16 *)((char *)v5 + v4);
      v13 = v6;
      if ( v6 )
      {
        if ( *v5 )
        {
          v7 = (char *)&xanim_pool + 68 * *v5;
          v8 = *((_WORD *)v7 + 2);
          if ( v8 )
          {
            v9 = GetRefString_var + 8 * v8;
            if ( *(_WORD *)v9 )
            {
              --*(_WORD *)v9;
            }
            else
            {
              SL_FreeString(v8, (char *)(v9 + 4), strlen((const char *)(v9 + 4)) + 1);
              v6 = v13;
            }
          }
        }
        else
        {
          v7 = XAnimAllocInfo(v3, v2);
        }
        qmemcpy(v7, (char *)&xanim_pool + 68 * v6, 0x44u);
        v10 = *((_WORD *)v7 + 2);
        if ( v10 )
          ++*(_WORD *)(GetRefString_var + 8 * v10);
      }
      else if ( *v5 )
      {
        XAnimFreeInfo(*v5);
        *v5 = 0;
      }
      result = v14;
      v2 = v11 + 1;
      ++v5;
      if ( ++v11 >= v14 )
        break;
      v3 = a2;
    }
  }
  return result;
}
#endif

static void XModelReadSurface(XSurface *surface, const unsigned char **pos,
                              XModelAllocFn alloc);
static XModelSurfs *XModelSurfsCloneSurfs(const XModelSurfs *entry,
                                          XModelAllocFn alloc);
static XModelPartsEntry *XModelCreateDefaultParts(void);
static XModelSurfs *XModelCreateDefaultSurfs(void);
void XModelCreateDefault(XModel *entry);
XModel *XModelCreateDefault_(void);
void XModelPartsFree(XModelPartsEntry *entry);
void XModelFree(XModel *entry);
void XModelSurfsFree(XModelSurfs *entry);
void XModelExpandQuatToAxis(float *quat);
int ReadQuat(const short *packed, short *out);

/* ---- XModelEnforceExist  0x00489DB0 ---- */
void __cdecl XModelEnforceExist(int enforce)
{
    xmodel_enforceExist = enforce;
}

/* ---- XModelBad  0x00489DC0 ----  VERIFIED */
int __cdecl XModelBad(const XModel *model)
{
    return model->info == &xmodel_defaultCollision;
}

/* ---- SetAnimCheck  0x00489DD0 ----  VERIFIED */
static void __cdecl SetAnimCheck(int enable)
{
    xmodel_animCheck = enable;
}

/* ---- ReadQuat  0x00489DE0 ----  VERIFIED */
int ReadQuat(const short *packed, short *out)
{
    short x = packed[0];
    short y = packed[1];
    short z = packed[2];
    int lenSq;

    out[0] = x;
    out[1] = y;
    out[2] = z;
    lenSq = 1073676289 - z * z - y * y - x * x;
    if ( lenSq <= 0 )
    {
        out[3] = 0;
        return 0;
    }
    out[3] = (short)(unsigned int)floor(sqrt((double)lenSq) + 0.5);
    return (int)(unsigned int)floor(sqrt((double)lenSq) + 0.5);
}

/* ---- ReadQuat2  0x00489E60 ----  VERIFIED */
int ReadQuat2(const short *packed, short *out)
{
    short x = packed[0];
    int lenSq;

    out[0] = x;
    lenSq = 1073676289 - x * x;
    if ( lenSq <= 0 )
    {
        out[1] = 0;
        return 0;
    }
    out[1] = (short)(unsigned int)floor(sqrt((double)lenSq) + 0.5);
    return (int)(unsigned int)floor(sqrt((double)lenSq) + 0.5);
}

/* ---- XModelExpandQuatToAxis  0x00489EB0 ----  VERIFIED */
void __cdecl XModelExpandQuatToAxis(float *quat)
{
    float x2, y2, z2;
    double lengthSq, scale, xx, yy, zz, x, y;
    float xy, xz, xw, yz, yw, zw;

    x2 = quat[0] * quat[0];
    y2 = quat[1] * quat[1];
    z2 = quat[2] * quat[2];
    lengthSq = quat[3] * quat[3] + z2 + y2 + x2;
    if ( lengthSq == 0.0 )
    {
        quat[0]  = 1.0f; quat[1] = 0.0f; quat[2]  = 0.0f;
        quat[4]  = 0.0f; quat[5] = 1.0f; quat[6]  = 0.0f;
        quat[8]  = 0.0f; quat[9] = 0.0f; quat[10] = 1.0f;
        return;
    }

    scale = 2.0 / lengthSq;
    xx = x2 * scale;
    yy = scale * y2;
    zz = scale * z2;
    x  = scale * quat[0];
    xz = (float)(x * quat[2]);
    xy = (float)(x * quat[1]);
    xw = (float)(x * quat[3]);
    y  = scale * quat[1];
    yz = (float)(y * quat[2]);
    yw = (float)(y * quat[3]);
    zw = (float)(quat[3] * quat[2] * scale);

    quat[0]  = (float)(1.0 - (zz + yy));
    quat[1]  = zw + xy;
    quat[2]  = xz - yw;
    quat[4]  = xy - zw;
    quat[5]  = (float)(1.0 - (zz + xx));
    quat[6]  = yz + xw;
    quat[8]  = yw + xz;
    quat[9]  = yz - xw;
    quat[10] = (float)(1.0 - (yy + xx));
}

/* ---- XModelSurfsFree  0x00489FE0 ----  VERIFIED */
void XModelSurfsFree(XModelSurfs *entry)
{
    (void)entry;
}

/* ---- XModelPartsFree  0x00489FF0 ----  VERIFIED */
void __cdecl XModelPartsFree(XModelPartsEntry *entry)
{
    const XModelPartNameTable *table =
        entry->data->partNameTableSlot->partNameTable;
    int count = table->count;
    int i;

    for ( i = 0; i < count; ++i )
        XModelReleaseString(table->handles[i]);
}

/* ---- XModelFree  0x0048A070 ----  VERIFIED */
void __cdecl XModelFree(XModel *entry)
{
    XModelInfo *info = entry->info;
    int lodIndex;
    int i;

    for ( lodIndex = 0; lodIndex < XMODEL_LOD_COUNT; ++lodIndex )
    {
        XModelLodInfo *lod = &info->lodRecords[lodIndex];

        if ( !lod->surfaceNameTable )
            continue;
        for ( i = 0; i < lod->surfaceCount; ++i )
            XModelReleaseString(lod->surfaceNameTable[i]);
        lod->surfaceNameTable = 0;
    }
}

/* ---- ReadBlend  0x0048A100 ---- */
static void ReadBlend(XSurface *surface, XSimpleBlendInfo *blend,
                      const unsigned char **pos)
{
    const unsigned char *cursor = *pos;
    short boneIndex;

    boneIndex = *(const short *)cursor;
    cursor += 2;

    ((unsigned char *)surface->boneUsage)[boneIndex >> 3] |=
        (unsigned char)(1u << (boneIndex & 7));
    blend->boneMatrixOffset = (unsigned int)(boneIndex << XMODEL_BONE_MATRIX_SHIFT);

    blend->position[0] = *(const float *)cursor; cursor += 4;
    blend->position[1] = *(const float *)cursor; cursor += 4;
    blend->position[2] = *(const float *)cursor; cursor += 4;

    *pos = cursor;
}

/* ---- XSurfaceUnstrip  0x0048A160 ---- */
static void XSurfaceUnstrip(const XStripInfo *strips, unsigned short (*triangles)[3])
{
    const unsigned short *stripIndex = strips->stripIndices;
    int strip;

    for ( strip = 0; strip < strips->stripCount; ++strip )
    {
        unsigned char vertexCount = strips->stripVertexCounts[strip];
        unsigned short previousA = stripIndex[0];
        unsigned short previousB = stripIndex[1];
        unsigned short current   = stripIndex[2];
        int stripVertex;

        stripIndex += 3;
        if ( previousA != previousB && previousA != current && previousB != current )
        {
            (*triangles)[0] = previousA;
            (*triangles)[1] = previousB;
            (*triangles)[2] = current;
            ++triangles;
        }

        for ( stripVertex = 3; stripVertex < vertexCount; stripVertex += 2 )
        {
            unsigned short next = stripIndex[0];
            unsigned short next2;

            if ( current != previousB && current != next && previousB != next )
            {
                (*triangles)[0] = current;
                (*triangles)[1] = previousB;
                (*triangles)[2] = next;
                ++triangles;
            }

            if ( stripVertex + 1 >= vertexCount )
            {
                stripIndex += 1;
                break;
            }

            next2 = stripIndex[1];
            stripIndex += 2;
            if ( current != next && current != next2 && next != next2 )
            {
                (*triangles)[0] = current;
                (*triangles)[1] = next;
                (*triangles)[2] = next2;
                ++triangles;
            }

            previousB = next;
            current = next2;
        }
    }
}

/* ---- XModelReadSurface  0x0048A260 ----  VERIFIED */
static void XModelReadSurface(XSurface *surface, const unsigned char **pos,
                              XModelAllocFn alloc)
{
    const unsigned char *cursor = *pos;
    short stripCount;
    short singleBoneIndex;
    short weightedPointCount = 0;
    short compactWeightedVertexCount = 0;
    int totalStripIndices;
    const unsigned char *stripScan;
    unsigned char *stripVertexCounts;
    unsigned short *stripIndices;
    unsigned short *stripIndexOut;
    unsigned char *blendCursor;
    XSurfaceRigidVert *rigidVertex;
    XStripInfo strips;
    short triangleAllocCount;
    int i;

    surface->tileMode = *cursor;              cursor += 1;
    surface->vertexCount = *(const short *)cursor;   cursor += 2;
    surface->triangleCount = *(const short *)cursor; cursor += 2;
    stripCount = *(const short *)cursor;             cursor += 2;
    singleBoneIndex = *(const short *)cursor;        cursor += 2;

    Com_Memset(surface->boneUsage, 0, 16);

    if ( singleBoneIndex == XMODEL_NO_SINGLE_BONE )
    {
        weightedPointCount = *(const short *)cursor;         cursor += 2;
        compactWeightedVertexCount = *(const short *)cursor; cursor += 2;
        surface->boneMatrixOffset = XMODEL_NO_SINGLE_BONE;
        surface->vertexData =
            alloc(4 * (9 * (int)surface->vertexCount - (int)compactWeightedVertexCount));
        surface->weightedPoints =
            (XSurfaceWeightedPoint *)alloc(20 * (int)weightedPointCount);
        surface->texCoords = (float (*)[2])alloc(8 * (int)surface->vertexCount);
    }
    else
    {
        surface->boneMatrixOffset =
            (short)(singleBoneIndex << XMODEL_BONE_MATRIX_SHIFT);
        ((unsigned char *)surface->boneUsage)[singleBoneIndex >> 3] |=
            (unsigned char)(1u << (singleBoneIndex & 7));
        surface->vertexData = alloc(24 * (int)surface->vertexCount);
        surface->weightedPoints = 0;
        surface->texCoords = (float (*)[2])alloc(8 * (int)surface->vertexCount);
    }

    totalStripIndices = 0;
    stripScan = cursor;
    for ( i = 0; i < stripCount; ++i )
    {
        unsigned char n = *stripScan;

        totalStripIndices += n;
        stripScan += 1 + 2 * (int)n;
    }

    stripVertexCounts = (unsigned char *)alloca((size_t)stripCount);
    stripIndices = (unsigned short *)alloca((size_t)(2 * totalStripIndices));

    stripIndexOut = stripIndices;
    for ( i = 0; i < stripCount; ++i )
    {
        unsigned char n = *cursor;

        stripVertexCounts[i] = n;
        cursor += 1;
        qmemcpy(stripIndexOut, cursor, (size_t)(2 * (int)n));
        cursor += 2 * (int)n;
        stripIndexOut += n;
    }

    blendCursor = (unsigned char *)surface->vertexData;
    rigidVertex = (XSurfaceRigidVert *)surface->vertexData;
    for ( i = 0; i < surface->vertexCount; ++i )
    {
        if ( singleBoneIndex == XMODEL_NO_SINGLE_BONE )
        {
            XSurfaceBlendVert *vert = (XSurfaceBlendVert *)blendCursor;

            vert->normal[0] = *(const float *)cursor; cursor += 4;
            vert->normal[1] = *(const float *)cursor; cursor += 4;
            vert->normal[2] = *(const float *)cursor; cursor += 4;
            surface->texCoords[i][0] = *(const float *)cursor; cursor += 4;
            surface->texCoords[i][1] = *(const float *)cursor; cursor += 4;
            vert->additiveWeightCount = *(const short *)cursor; cursor += 2;

            ReadBlend(surface, &vert->blend, &cursor);

            if ( vert->additiveWeightCount == 0 )
            {
                blendCursor += 32;
            }
            else
            {
                vert->primaryWeight = *(const float *)cursor; cursor += 4;
                blendCursor += 36;
            }
        }
        else
        {
            rigidVertex->normal[0] = *(const float *)cursor; cursor += 4;
            rigidVertex->normal[1] = *(const float *)cursor; cursor += 4;
            rigidVertex->normal[2] = *(const float *)cursor; cursor += 4;
            surface->texCoords[i][0] = *(const float *)cursor; cursor += 4;
            surface->texCoords[i][1] = *(const float *)cursor; cursor += 4;
            rigidVertex->position[0] = *(const float *)cursor; cursor += 4;
            rigidVertex->position[1] = *(const float *)cursor; cursor += 4;
            rigidVertex->position[2] = *(const float *)cursor; cursor += 4;
            ++rigidVertex;
        }
    }

    for ( i = 0; i < weightedPointCount; ++i )
    {
        ReadBlend(surface, &surface->weightedPoints[i].blend, &cursor);
        surface->weightedPoints[i].weight = *(const float *)cursor;
        cursor += 4;
    }

    triangleAllocCount = surface->triangleCount;
    if ( (triangleAllocCount & 1) != 0 )
        ++triangleAllocCount;

    surface->triangles = (unsigned short (*)[3])alloc(6 * (int)triangleAllocCount);

    strips.stripCount = stripCount;
    strips.stripVertexCounts = stripVertexCounts;
    strips.stripIndices = stripIndices;
    XSurfaceUnstrip(&strips, surface->triangles);

    if ( (surface->triangleCount & 1) != 0 )
    {
        unsigned short repeated = surface->triangles[triangleAllocCount - 2][2];

        surface->triangleCount = triangleAllocCount;
        surface->triangles[triangleAllocCount - 1][0] = repeated;
        surface->triangles[triangleAllocCount - 1][1] = repeated;
        surface->triangles[triangleAllocCount - 1][2] = repeated;
    }

    *pos = cursor;
}

/* ---- XModelCreateDefaultSurface  0x0048A800 ----  VERIFIED */
static void __cdecl XModelCreateDefaultSurface(XSurface *surface)
{
    int i;

    surface->triangleCount = 1;
    surface->vertexCount = 3;
    Com_Memset(surface->boneUsage, 0, 16);
    surface->vertexData = xmodel_defaultRigidVerts;
    surface->texCoords = xmodel_defaultTexCoords;
    surface->boneMatrixOffset = 0;
    surface->weightedPoints = 0;
    surface->triangles = (unsigned short (*)[3])xmodel_defaultTriangle;
    xmodel_defaultTriangle[0] = 0;
    xmodel_defaultTriangle[1] = 1;
    xmodel_defaultTriangle[2] = 2;

    for ( i = 0; i < surface->vertexCount; ++i )
    {
        xmodel_defaultRigidVerts[i].normal[0] = 1.0f;
        xmodel_defaultRigidVerts[i].normal[1] = 0.0f;
        xmodel_defaultRigidVerts[i].normal[2] = 0.0f;
        xmodel_defaultTexCoords[i][0] = 0.0f;
        xmodel_defaultTexCoords[i][1] = 0.0f;
        xmodel_defaultRigidVerts[i].position[0] = 0.0f;
        xmodel_defaultRigidVerts[i].position[1] = 0.0f;
        xmodel_defaultRigidVerts[i].position[2] = 0.0f;
    }
}

/* ---- XModelCreateDefaultParts  0x0048A890 ----  VERIFIED */
static XModelPartsEntry *XModelCreateDefaultParts(void)
{
    xmodel_defaultParts.baseRotations = 0;
    xmodel_defaultParts.baseTranslations = 0;
    xmodel_defaultPartStateIndices[0] = 0;
    xmodel_defaultPartNameTable.handles[0] = 0;
    xmodel_defaultPartsEntry.freeData = 0;
    xmodel_defaultPartNameTable.count = 1;
    xmodel_defaultPartNameTableSlot.partNameTable = &xmodel_defaultPartNameTable;
    xmodel_defaultParts.partNameTableSlot = &xmodel_defaultPartNameTableSlot;
    xmodel_defaultParts.rootPartCount = 1;
    xmodel_defaultParts.partStateIndices = xmodel_defaultPartStateIndices;

    xmodel_defaultPartColl.mins[0] = -16.0f;
    xmodel_defaultPartColl.mins[1] = -16.0f;
    xmodel_defaultPartColl.mins[2] = -16.0f;
    xmodel_defaultPartColl.maxs[0] =  16.0f;
    xmodel_defaultPartColl.maxs[1] =  16.0f;
    xmodel_defaultPartColl.maxs[2] =  16.0f;
    xmodel_defaultParts.partCollisions = &xmodel_defaultPartColl;

    xmodel_defaultPartsEntry.data = &xmodel_defaultParts;
    xmodel_defaultPartsEntry.name = "DEFAULT";
    return &xmodel_defaultPartsEntry;
}

/* ---- XModelCreateDefaultSurfs  0x0048A940 ----  VERIFIED */
static XModelSurfs *XModelCreateDefaultSurfs(void)
{
    xmodel_defaultSurfs.surfaceCount = 1;
    xmodel_defaultSurfacePtrs[0] = &xmodel_defaultSurfaces[0];
    xmodel_defaultSurfs.surfaces = xmodel_defaultSurfacePtrs;
    XModelCreateDefaultSurface(&xmodel_defaultSurfaces[0]);

    xmodel_defaultSurfsEntry.surfs = &xmodel_defaultSurfs;
    xmodel_defaultSurfsEntry.freeData = 0;
    xmodel_defaultSurfsEntry.name = "DEFAULT";
    return &xmodel_defaultSurfsEntry;
}

/* ---- XModelCreateDefault  0x0048A990 ----  VERIFIED */
void __cdecl XModelCreateDefault(XModel *entry)
{
    int lodIndex;

    xmodel_defaultCollision.parts = XModelCreateDefaultParts();

    for ( lodIndex = 0; lodIndex < XMODEL_LOD_COUNT; ++lodIndex )
    {
        XModelLodInfo *lod = &xmodel_defaultCollision.lodRecords[lodIndex];

        lod->surfs = 0;
        lod->name = (const char *)&empty_string;
        lod->distance = 0.0f;
        lod->surfaceCount = 1;
        lod->surfaceNameTable = xmodel_defaultSurfaceNames;
        xmodel_defaultSurfaceNames[0] = 0;
    }

    xmodel_defaultCollision.lodRecords[0].surfs = XModelCreateDefaultSurfs();
    entry->freeData = 0;
    entry->info = &xmodel_defaultCollision;
    xmodel_defaultCollision.lodCount = 1;
    xmodel_defaultCollision.modelFileCount = 0;
}

/* ---- XModelCreateDefault_  0x0048AA40 ----  VERIFIED */
XModel *XModelCreateDefault_(void)
{
    XModelCreateDefault(&xmodel_defaultModelEntry);
    xmodel_defaultModelEntry.name = "DEFAULT";
    return &xmodel_defaultModelEntry;
}

/* ---- XModelSurfsCloneSurfs  0x0048AA60 ---- */
static XModelSurfs *XModelSurfsCloneSurfs(const XModelSurfs *entry,
                                          XModelAllocFn alloc)
{
    XModelSurfsData *clone = (XModelSurfsData *)alloc(12);
    XModelSurfs *asset;

    clone->surfaceCount = entry->surfs->surfaceCount;
    clone->surfaces = (XSurface **)alloc(4 * (int)clone->surfaceCount);
    Com_Memcpy(clone->surfaces, entry->surfs->surfaces,
               (size_t)(4 * (int)clone->surfaceCount));

    clone->next = xmodel_surfsCloneList;
    xmodel_surfsCloneList = clone;

    asset = (XModelSurfs *)alloc(12);
    asset->surfs = clone;
    asset->freeData = XModelSurfsFree;
    asset->name = entry->name;
    return asset;
}

/* ---- XModelSurfsPrecache  0x0048AAC0 ----  VERIFIED */
XModelSurfs *__cdecl XModelSurfsPrecache(const char *name, XModelAllocFn alloc)
{
    XModelSurfs *entry;
    void *fileBuffer;
    const unsigned char *cursor;
    XModelSurfsData *surfs;
    XSurface *surfaces;
    short version;
    short surfaceCount;
    int i;
    char path[1024];

    entry = (XModelSurfs *)FS_GetDataForFile("xmodelsurfs", name, "");
    if ( !entry )
    {
        if ( xmodel_enforceExist )
            Com_Error(ERR_DROP,
                      "\x15" "Cannot precache 'xmodelsurfs/%s'.\n"
                      "you may need to get latest and run converter to fix",
                      name);
        Com_Printf("ERROR: Cannot precache 'xmodelsurfs/%s'", name);
        return 0;
    }

    if ( entry->surfs )
        return XModelSurfsCloneSurfs(entry, alloc);

    sprintf(path, "xmodelsurfs/%s", name);
    if ( FS_ReadFile(path, &fileBuffer) < 0 )
        Com_Error(ERR_DROP, "\x15" "Cannot find 'xmodelsurfs/%s'.", name);

    version = *(const short *)fileBuffer;
    if ( version != XMODEL_ASSET_VERSION )
    {
        FS_FreeFile(fileBuffer);
        Com_Error(ERR_DROP,
                  "\x15" "xmodelsurfs '%s' out of date (version %d, expecting %d)",
                  name, version, XMODEL_ASSET_VERSION);
    }

    surfaceCount = *((const short *)fileBuffer + 1);
    cursor = (const unsigned char *)fileBuffer + 4;

    surfs = (XModelSurfsData *)alloc(12);
    surfs->surfaceCount = surfaceCount;
    surfs->surfaces = (XSurface **)alloc(4 * (int)surfaceCount);
    surfaces = (XSurface *)alloc(52 * (int)surfaceCount);

    for ( i = 0; i < surfaceCount; ++i )
    {
        surfs->surfaces[i] = &surfaces[i];
        XModelReadSurface(surfs->surfaces[i], &cursor, alloc);
    }

    FS_FreeFile(fileBuffer);

    entry->surfs = surfs;
    entry->freeData = XModelSurfsFree;
    surfs->next = xmodel_surfsCloneList;
    xmodel_surfsCloneList = surfs;
    return entry;
}

/* ---- XModelPartsPrecache  0x0048AC80 ----  VERIFIED */
XModelPartsEntry *__cdecl XModelPartsPrecache(const char *name, XModelAllocFn alloc)
{
    XModelPartsEntry *entry;
    void *fileBuffer;
    const unsigned char *cursor;
    XModelPartNameTable *table;
    XModelPartNameTableSlot *slot;
    XModelPartsData *parts;
    XModelPartColl *partColls;
    short version;
    short childPartCount;
    short rootPartCount;
    short totalPartCount;
    int i;
    char path[1024];

    entry = (XModelPartsEntry *)FS_GetDataForFile("xmodelparts", name, "");
    if ( !entry )
    {
        if ( xmodel_enforceExist )
            Com_Error(ERR_DROP,
                      "\x15" "Cannot precache 'xmodelparts/%s'.\n"
                      "you may need to get latest and run converter to fix",
                      name);
        Com_Printf("ERROR: Cannot precache 'xmodelparts/%s'", name);
        return 0;
    }

    if ( entry->data )
        return entry;

    sprintf(path, "xmodelparts/%s", name);
    if ( FS_ReadFile(path, &fileBuffer) < 0 )
        Com_Error(ERR_DROP, "\x15" "Cannot find 'xmodelparts/%s'.", name);

    version = *(const short *)fileBuffer;
    if ( version != XMODEL_ASSET_VERSION )
    {
        FS_FreeFile(fileBuffer);
        Com_Error(ERR_DROP,
                  "\x15" "xmodelparts '%s' out of date (version %d, expecting %d)",
                  name, version, XMODEL_ASSET_VERSION);
    }

    childPartCount = *((const short *)fileBuffer + 1);
    rootPartCount  = *((const short *)fileBuffer + 2);
    totalPartCount = (short)(childPartCount + rootPartCount);
    cursor = (const unsigned char *)fileBuffer + 6;

    table = (XModelPartNameTable *)alloc(2 * (int)totalPartCount + 2);
    table->count = totalPartCount;
    if ( totalPartCount >= XMODEL_MAX_BONES + 1 )
        Com_Error(ERR_DROP, "\x15" "xmodel '%s' has more than %d bones",
                  name, XMODEL_MAX_BONES);

    slot = (XModelPartNameTableSlot *)alloc((int)childPartCount + 7);
    slot->partNameTable = table;

    parts = (XModelPartsData *)alloc(24 * (int)childPartCount + 104);
    parts->partNameTableSlot = slot;
    parts->baseRotations = (short (*)[4])((char *)parts + 24);
    parts->baseTranslations = childPartCount
        ? (float (*)[3])((char *)parts + 24 + 8 * (int)childPartCount)
        : (float (*)[3])0;
    parts->partStateIndices = (unsigned char *)alloc((int)totalPartCount);
    parts->rootPartCount = rootPartCount;

    partColls = (XModelPartColl *)alloc(40 * (int)totalPartCount);

    for ( i = rootPartCount; i < totalPartCount; ++i )
    {
        int child = i - rootPartCount;

        slot->parentPartDeltas[child] = (unsigned char)(i - *cursor);
        cursor += 1;
        parts->baseTranslations[child][0] = *(const float *)cursor; cursor += 4;
        parts->baseTranslations[child][1] = *(const float *)cursor; cursor += 4;
        parts->baseTranslations[child][2] = *(const float *)cursor; cursor += 4;
        ReadQuat((const short *)cursor, parts->baseRotations[child]);
        cursor += 6;
    }

    for ( i = 0; i < totalPartCount; ++i )
    {
        XModelPartColl *coll = &partColls[i];
        size_t nameLength = strlen((const char *)cursor);
        float dx, dy, dz;

        /* type 10 -- `push 0Ah` at 0x0048AED9. */
        table->handles[i] = (unsigned short)
            SL_GetStringOfLen((const char *)cursor, 0, nameLength + 1, 10);
        cursor += nameLength + 1;

        coll->mins[0] = *(const float *)cursor; cursor += 4;
        coll->mins[1] = *(const float *)cursor; cursor += 4;
        coll->mins[2] = *(const float *)cursor; cursor += 4;
        coll->maxs[0] = *(const float *)cursor; cursor += 4;
        coll->maxs[1] = *(const float *)cursor; cursor += 4;
        coll->maxs[2] = *(const float *)cursor; cursor += 4;

        coll->center[0] = coll->maxs[0] + coll->mins[0];
        coll->center[1] = coll->maxs[1] + coll->mins[1];
        coll->center[2] = coll->maxs[2] + coll->mins[2];
        coll->center[0] = coll->center[0] * 0.5f;
        coll->center[1] = coll->center[1] * 0.5f;
        coll->center[2] = coll->center[2] * 0.5f;

        dx = coll->maxs[0] - coll->center[0];
        dy = coll->maxs[1] - coll->center[1];
        dz = coll->maxs[2] - coll->center[2];
        coll->radiusSq = dz * dz + dy * dy + dx * dx;
    }

    qmemcpy(parts->partStateIndices, cursor, (size_t)totalPartCount);
    FS_FreeFile(fileBuffer);

    parts->partCollisions = partColls;
    entry->data = parts;
    entry->freeData = XModelPartsFree;
    return entry;
}

/* ---- XModelLoadConfigFile  0x0048B020 ----  VERIFIED */
static const unsigned char *XModelLoadConfigFile(const char *name,
                                                 const unsigned char *cursor,
                                                 XModelConfig *config)
{
    short version;
    int lodIndex;
    int i;

    version = *(const short *)cursor;
    cursor += 2;
    if ( version != XMODEL_ASSET_VERSION )
    {
        Com_Error(ERR_DROP,
                  "\x15" "xmodel '%s' out of date (version %d, expecting %d)",
                  name, version, XMODEL_ASSET_VERSION);
        return 0;
    }

    for ( i = 0; i < 3; ++i ) { config->mins[i] = *(const float *)cursor; cursor += 4; }
    for ( i = 0; i < 3; ++i ) { config->maxs[i] = *(const float *)cursor; cursor += 4; }

    for ( lodIndex = 0; lodIndex < XMODEL_LOD_COUNT; ++lodIndex )
    {
        config->lods[lodIndex].distance = *(const float *)cursor;
        cursor += 4;
        strcpy(config->lods[lodIndex].name, (const char *)cursor);
        cursor += strlen((const char *)cursor) + 1;
    }

    config->modelFileCount = *(const int *)cursor;
    cursor += 4;
    return cursor;
}

/* ---- XModelLoadCollData  0x0048B0E0 ----  VERIFIED */
static const unsigned char *XModelLoadCollData(const unsigned char *cursor,
                                               XModelInfo *info,
                                               XModelAllocFn alloc)
{
    int surfaceCount;
    int surfaceIndex;

    surfaceCount = *(const int *)cursor;
    cursor += 4;
    info->collisionSurfaceCount = surfaceCount;
    info->contents = 0;

    if ( !surfaceCount )
    {
        info->collisionSurfaces = 0;
        return cursor;
    }

    info->collisionSurfaces = (XModelCollSurf *)alloc(44 * surfaceCount);

    for ( surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex )
    {
        XModelCollSurf *surf = &info->collisionSurfaces[surfaceIndex];
        int triCount;
        int triIndex;

        triCount = *(const int *)cursor;
        cursor += 4;
        surf->numCollTris = triCount;
        surf->collTris = (XModelCollTri *)alloc(48 * triCount);

        for ( triIndex = 0; triIndex < triCount; ++triIndex )
        {
            float *dst = (float *)&surf->collTris[triIndex];
            int lane;

            for ( lane = 0; lane < 12; ++lane )
            {
                dst[lane] = *(const float *)cursor;
                cursor += 4;
            }
        }

        surf->expandedMins[0] = *(const float *)cursor        - 0.001f;
        surf->expandedMins[1] = *((const float *)cursor + 1)  - 0.001f;
        surf->expandedMins[2] = *((const float *)cursor + 2)  - 0.001f;
        surf->expandedMaxs[0] = *((const float *)cursor + 3)  + 0.001f;
        surf->expandedMaxs[1] = *((const float *)cursor + 4)  + 0.001f;
        surf->expandedMaxs[2] = *((const float *)cursor + 5)  + 0.001f;
        surf->basePoseIndex   = *((const int *)cursor + 6);
        surf->contents        = *((const int *)cursor + 7) & 0xDFFF7FFB;
        surf->surfaceFlags    = *((const int *)cursor + 8);
        cursor += 36;

        info->contents |= surf->contents;
    }

    return cursor;
}

/* ---- XModelExists  0x0048B270 ----  VERIFIED */
int __cdecl XModelExists(const char *name)
{
    return FS_GetDataForFile("xmodel", name, "") != 0;
}

/* ---- XModelPrecache  0x0048B290 ----  VERIFIED */
XModel *__cdecl XModelPrecache(const char *name, int loadSurfaces,
                               XModelAllocFn alloc, XModelAllocFn allocMesh)
{
    XModel *entry;
    XModelInfo *info;
    void *fileBuffer;
    const unsigned char *cursor;
    char *namePool;
    int nameBytes;
    int nameLengths[XMODEL_LOD_COUNT];
    int lodIndex;
    int i;
    XModelConfig config;
    char path[1024];

    entry = (XModel *)FS_GetDataForFile("xmodel", name, "");
    if ( !entry )
    {
        if ( xmodel_enforceExist )
        {
            Com_Error(ERR_DROP,
                      "\x15" "Cannot precache 'xmodel/%s'.\n"
                      "you may need to get latest and run converter to fix",
                      name);
            return 0;
        }
        Com_Printf("ERROR: Cannot precache 'xmodel/%s'", name);
        return XModelCreateDefault_();
    }

    info = entry->info;
    if ( info )
    {
        if ( info == &xmodel_defaultCollision )
            return entry;
    }
    else
    {
        sprintf(path, "xmodel/%s", name);
        if ( FS_ReadFile(path, &fileBuffer) < 0 )
            Com_Error(ERR_DROP, "\x15" "Cannot find 'xmodel/%s'.", name);

        cursor = XModelLoadConfigFile(name, (const unsigned char *)fileBuffer, &config);
        if ( !cursor )
        {
            FS_FreeFile(fileBuffer);
            return 0;
        }

        nameBytes = 0;
        for ( lodIndex = 0; lodIndex < XMODEL_LOD_COUNT; ++lodIndex )
        {
            nameLengths[lodIndex] = (int)strlen(config.lods[lodIndex].name) + 1;
            nameBytes += nameLengths[lodIndex];
        }

        info = (XModelInfo *)alloc(nameBytes + 104);
        cursor = XModelLoadCollData(cursor, info, alloc);

        info->lodCount = 0;
        namePool = (char *)info + 104;
        for ( lodIndex = 0; lodIndex < XMODEL_LOD_COUNT; ++lodIndex )
        {
            XModelLodInfo *lod = &info->lodRecords[lodIndex];

            strcpy(namePool, config.lods[lodIndex].name);
            lod->name = namePool;

            if ( *namePool )
            {
                short surfaceCount;

                ++info->lodCount;
                surfaceCount = *(const short *)cursor;
                cursor += 2;
                lod->surfaceCount = surfaceCount;
                lod->surfaceNameTable =
                    (unsigned short *)alloc(2 * (int)surfaceCount);
                for ( i = 0; i < surfaceCount; ++i )
                {
                    const char *surfName = (const char *)cursor;

                    cursor += strlen(surfName) + 1;
                    /* type 8 -- `push 8` at 0x0048B473. */
                    lod->surfaceNameTable[i] = (unsigned short)
                        SL_GetStringOfLen(surfName, 0, strlen(surfName) + 1, 8);
                }
            }
            else
            {
                lod->surfaceNameTable = 0;
            }

            lod->distance = config.lods[lodIndex].distance;
            namePool += nameLengths[lodIndex];
        }

        FS_FreeFile(fileBuffer);

        info->mins[0] = config.mins[0];
        info->mins[1] = config.mins[1];
        info->mins[2] = config.mins[2];
        info->maxs[0] = config.maxs[0];
        info->maxs[1] = config.maxs[1];
        info->maxs[2] = config.maxs[2];
        info->modelFileCount = (short)config.modelFileCount;

        entry->info = info;
        entry->freeData = XModelFree;
        info->lodRecords[0].surfs = 0;
        info->lodRecords[1].surfs = 0;
        info->lodRecords[2].surfs = 0;
    }

    info->parts = XModelPartsPrecache(info->lodRecords[0].name, alloc);
    if ( info->parts )
    {
        if ( loadSurfaces )
        {
            for ( lodIndex = 0; lodIndex < XMODEL_LOD_COUNT; ++lodIndex )
            {
                XModelLodInfo *lod = &info->lodRecords[lodIndex];
                XModelSurfs *surfs;

                if ( !*lod->name )
                    break;
                surfs = XModelSurfsPrecache(lod->name, alloc);
                lod->surfs = surfs;
                if ( !surfs )
                {
                    XModelFree(entry);
                    XModelCreateDefault(entry);
                    return entry;
                }
#ifndef DEDICATED
                if ( xmodel_animCheck && loadSurfaces == 2 )
                    XModelOptimize(surfs->surfs, allocMesh);
#endif
            }
        }
    }
    else
    {
        XModelFree(entry);
        XModelCreateDefault(entry);
    }

    return entry;
}

/* ---- XModelClearData  0x0048B600 ----  VERIFIED */
void __cdecl XModelClearData(unsigned int rangeEnd, unsigned int rangeStart)
{
    XModelSurfsData **link = &xmodel_surfsCloneList;
    XModelSurfsData *surfs = xmodel_surfsCloneList;

    while ( surfs )
    {
        if ( (unsigned int)surfs < rangeStart || (unsigned int)surfs >= rangeEnd )
        {
            int i;

            for ( i = 0; i < surfs->surfaceCount; ++i )
            {
                XSurface *surface = surfs->surfaces[i];

                if ( surface )
                {
                    unsigned int nv = (unsigned int)surface->optimizedDataNV;
                    unsigned int ati;

                    if ( nv >= rangeStart && nv < rangeEnd )
                        surface->optimizedDataNV = 0;
                    ati = (unsigned int)surface->optimizedDataATI;
                    if ( ati >= rangeStart && ati < rangeEnd )
                        surface->optimizedDataATI = 0;
                }
            }
            link = &surfs->next;
        }
        else
        {
            *link = surfs->next;
        }
        surfs = surfs->next;
    }
}

/* ---- XModelNumBones  0x0048B670 ----  VERIFIED */
int __cdecl XModelNumBones(const XModel *model)
{
    return model->info->parts->data->partNameTableSlot->partNameTable->count;
}

/* ---- XModelBoneNames  0x0048B680 ----  VERIFIED */
unsigned short *__cdecl XModelBoneNames(const XModel *model)
{
    return model->info->parts->data->partNameTableSlot->partNameTable->handles;
}

/* ---- XModelGetBoneIndex  0x0048B690 ----  VERIFIED */
int __cdecl XModelGetBoneIndex(const XModel *model, unsigned short name)
{
    const XModelPartNameTable *table =
        model->info->parts->data->partNameTableSlot->partNameTable;
    int partIndex;

    for ( partIndex = table->count - 1; partIndex >= 0; --partIndex )
    {
        if ( table->handles[partIndex] == name )
            break;
    }
    return partIndex;
}

/* ---- XModelGetBounds  0x0048B6B0 ----  VERIFIED */
void __cdecl XModelGetBounds(const XModel *model, float *mins, float *maxs)
{
    const XModelInfo *info = model->info;

    mins[0] = info->mins[0];
    mins[1] = info->mins[1];
    mins[2] = info->mins[2];
    maxs[0] = info->maxs[0];
    maxs[1] = info->maxs[1];
    maxs[2] = info->maxs[2];
}

/* ---- XModelGetSurfaces  0x0048B6E0 ----  VERIFIED */
int __cdecl XModelGetSurfaces(int lodIndex, const XModel *model,
                              XSurface ***surfaces)
{
    const XModelSurfsData *surfs = model->info->lodRecords[lodIndex].surfs->surfs;

    *surfaces = surfs->surfaces;
    return surfs->surfaceCount;
}

/* ---- XModelGetSurfaceName  0x0048B710 ----  VERIFIED */
const char *__cdecl XModelGetSurfaceName(int lodIndex, const XModel *model,
                                         int surfaceIndex)
{
    unsigned short handle =
        model->info->lodRecords[lodIndex].surfaceNameTable[surfaceIndex];

    if ( handle )
        return SL_REFSTRING(handle) + 4;
    return "DEFAULT";
}

/* ---- XSurfaceCloneSurface  0x0048B740 ----  VERIFIED */
XSurface *__cdecl XSurfaceCloneSurface(const XSurface *surface,
                                       XModelAllocFn alloc)
{
    XSurface *clone = (XSurface *)alloc(52);
    int texCoordBytes;

    qmemcpy(clone, surface, 0x34);
    clone->optimizedDataATI = 0;
    clone->optimizedDataNV = 0;

    texCoordBytes = 8 * (int)clone->vertexCount;
    clone->texCoords = (float (*)[2])alloc(texCoordBytes);
    Com_Memcpy(clone->texCoords, surface->texCoords, (size_t)texCoordBytes);
    return clone;
}

/* ---- XSurfaceGetNumVerts  0x0048B790 ----  VERIFIED */
int __cdecl XSurfaceGetNumVerts(const XSurface *surface)
{
    return surface->vertexCount;
}

/* ---- XSurfaceGetNumTris  0x0048B7A0 ----  VERIFIED */
int __cdecl XSurfaceGetNumTris(const XSurface *surface)
{
    return surface->triangleCount;
}

/* ---- XSurfaceTileMode  0x0048B7B0 ----  VERIFIED */
int __cdecl XSurfaceTileMode(const XSurface *surface)
{
    return surface->tileMode;
}

/* ---- XSurfaceGetTris  0x0048B7C0 ---- */
unsigned int __cdecl XSurfaceGetTris(unsigned int result, int a2, unsigned short a3)
{
  int v4;
  _DWORD *v5;
  int v6;
  int v7;
  _DWORD *v8;
  _DWORD *v9;
  void *v10;

  if ( a3 )
  {
    v4 = a3 | (a3 << 16);
    v5 = *(_DWORD **)(a2 + 28);
    v6 = *(__int16 *)(a2 + 4) >> 1;
    do
    {
      *(_DWORD *)result = v4 + *v5;
      v7 = v5[1];
      v8 = v5 + 1;
      v9 = (_DWORD *)(result + 4);
      *v9++ = v4 + v7;
      *v9 = v4 + v8[1];
      result = (unsigned int)(v9 + 1);
      v5 = v8 + 2;
      --v6;
    }
    while ( v6 );
  }
  else
  {
    v10 = (void *)result;
    result = 6 * *(__int16 *)(a2 + 4);
    qmemcpy(v10, *(const void **)(a2 + 28), result);
  }
  return result;
}

/* ---- XSurfaceGetBlendInfoArray  0x0048B830 ----  VERIFIED */
void *__cdecl XSurfaceGetBlendInfoArray(const XSurface *surface)
{
    return surface->vertexData;
}

/* ---- XSurfaceGetTexCoordArray  0x0048B840 ----  VERIFIED */
float *__cdecl XSurfaceGetTexCoordArray(const XSurface *surface)
{
    return (float *)surface->texCoords;
}

/* ---- XSurfaceGetVertexInfoArray  0x0048B850 ----  VERIFIED */
XSurfaceWeightedPoint *__cdecl XSurfaceGetVertexInfoArray(const XSurface *surface)
{
    return surface->weightedPoints;
}

/* ---- XSurfaceGetBoneIndex  0x0048B860 ----  VERIFIED */
int __cdecl XSurfaceGetBoneIndex(const XSurface *surface)
{
    return surface->boneMatrixOffset;
}

/* ---- XSurfaceRemapTextureCoordinates  0x0048B870 ---- */
void __cdecl XSurfaceRemapTextureCoordinates(XSurface *surface,
                                             const float *scale,
                                             const float *offset,
                                             int sourceUIndex, int sourceVIndex)
{
    float *texCoords = (float *)surface->texCoords;
    int i;

    for ( i = surface->vertexCount; i; --i )
    {
        float sourceV = texCoords[sourceVIndex];

        texCoords[0] = texCoords[sourceUIndex] * scale[0] + offset[0];
        texCoords[1] = sourceV * scale[1] + offset[1];
        texCoords += 2;
    }
}

/* ---- XModelGetName  0x0048B8A0 ----  VERIFIED */
const char *__cdecl XModelGetName(const XModel *model)
{
    return model->name;
}

/* ---- XModelGetContents  0x0048B8B0 ----  VERIFIED */
int __cdecl XModelGetContents(const XModel *model)
{
    return model->info->contents;
}

/* ---- XSurfaceTransformPoint43  0x0048B8C0 ----  VERIFIED */
void __cdecl XSurfaceTransformPoint43(const DObjSkelMat *matrix,
                                      const float *point, float *out)
{
    out[0] = matrix->axis[2][0] * point[2] + matrix->axis[1][0] * point[1]
           + point[0] * matrix->axis[0][0] + matrix->origin[0];
    out[1] = matrix->axis[0][1] * point[0] + matrix->axis[2][1] * point[2]
           + matrix->axis[1][1] * point[1] + matrix->origin[1];
    out[2] = matrix->axis[0][2] * point[0] + matrix->axis[2][2] * point[2]
           + matrix->axis[1][2] * point[1] + matrix->origin[2];
}

/* ---- XSurfaceAccumulateWeightedPoint43  0x0048B910 ----  VERIFIED */
void __cdecl XSurfaceAccumulateWeightedPoint43(const DObjSkelMat *matrix,
                                               const float *point, float *out,
                                               float weight)
{
    out[0] += (matrix->axis[1][0] * point[1] + matrix->axis[2][0] * point[2]
             + point[0] * matrix->axis[0][0] + matrix->origin[0]) * weight;
    out[1] += (matrix->axis[1][1] * point[1] + matrix->axis[2][1] * point[2]
             + matrix->axis[0][1] * point[0] + matrix->origin[1]) * weight;
    out[2] += (matrix->axis[1][2] * point[1] + matrix->axis[2][2] * point[2]
             + matrix->axis[0][2] * point[0] + matrix->origin[2]) * weight;
}

/* ---- XSurfaceTransformNormal43  0x0048B980 ----  VERIFIED */
void __cdecl XSurfaceTransformNormal43(const DObjSkelMat *matrix,
                                       const float *normal, float *out)
{
    out[0] = matrix->axis[2][0] * normal[2] + matrix->axis[1][0] * normal[1]
           + normal[0] * matrix->axis[0][0];
    out[1] = matrix->axis[0][1] * normal[0] + matrix->axis[2][1] * normal[2]
           + matrix->axis[1][1] * normal[1];
    out[2] = matrix->axis[0][2] * normal[0] + matrix->axis[2][2] * normal[2]
           + matrix->axis[1][2] * normal[1];
}

/* ---- XSurfaceTransformVectorRows43  0x0048B9D0 ----  VERIFIED */
void __cdecl XSurfaceTransformVectorRows43(const DObjSkelMat *matrix,
                                           const float *vector, float *out)
{
    out[0] = matrix->axis[0][1] * vector[1] + matrix->axis[0][2] * vector[2]
           + vector[0] * matrix->axis[0][0];
    out[1] = matrix->axis[1][0] * vector[0] + matrix->axis[1][1] * vector[1]
           + matrix->axis[1][2] * vector[2];
    out[2] = matrix->axis[2][0] * vector[0] + matrix->axis[2][1] * vector[1]
           + matrix->axis[2][2] * vector[2];
}

/* ---- XSurfaceGetVerts  0x0048BA20 ---- */
int __cdecl XSurfaceGetVerts(char *a1, float *a2, int a3, int a4, int a5)
{
  bool v6;
  int result;
  int v8;
  int v10;
  char *v11;
  unsigned int v12;
  char *v13;
  char *v14;
  char v15;
  int v16;
  double v17;
  int v18;
  int v19;
  int v20;
  double v21;
  float *v22;
  int v23;
  double v24;
  int v25;
  int v26;
  double v27;
  double v28;
  double v29;
  int v30;

  v6 = a1 == 0;
  result = a3;
  v8 = *(_DWORD *)(a3 + 32);
  v10 = *(__int16 *)(a3 + 6);
  v30 = v10;
  if ( !v6 )
  {
    v11 = *(char **)(a3 + 36);
    v12 = 8 * *(__int16 *)(a3 + 2);
    qmemcpy(a1, v11, 4 * (v12 >> 2));
    v14 = &v11[4 * (v12 >> 2)];
    v13 = &a1[4 * (v12 >> 2)];
    v15 = v12;
    result = a3;
    qmemcpy(v13, v14, v15 & 3);
    v10 = v30;
  }
  if ( v10 == -1 )
  {
    v18 = *(_DWORD *)(result + 24);
    result = *(__int16 *)(result + 2);
    if ( result )
    {
      v19 = result;
      do
      {
        if ( a5 )
        {
          v20 = *(_DWORD *)(v8 + 28);
          v21 = *(float *)(v20 + a4 + 32);
          v22 = (float *)(a4 + v20);
          a5 += 12;
          *(float *)(a5 - 12) = v21 * *(float *)(v8 + 8) + v22[4] * *(float *)(v8 + 4) + *(float *)v8 * *v22;
          *(float *)(a5 - 8) = v22[9] * *(float *)(v8 + 8) + v22[1] * *(float *)v8 + v22[5] * *(float *)(v8 + 4);
          *(float *)(a5 - 4) = v22[10] * *(float *)(v8 + 8) + v22[2] * *(float *)v8 + v22[6] * *(float *)(v8 + 4);
        }
        v23 = *(_DWORD *)(v8 + 28);
        v24 = *(float *)(v23 + a4 + 16);
        result = a4 + v23;
        *a2 = v24 * *(float *)(v8 + 20)
            + *(float *)(result + 32) * *(float *)(v8 + 24)
            + *(float *)(v8 + 16) * *(float *)result
            + *(float *)(result + 48);
        a2[1] = *(float *)(result + 20) * *(float *)(v8 + 20)
              + *(float *)(result + 4) * *(float *)(v8 + 16)
              + *(float *)(result + 36) * *(float *)(v8 + 24)
              + *(float *)(result + 52);
        a2[2] = *(float *)(result + 24) * *(float *)(v8 + 20)
              + *(float *)(result + 8) * *(float *)(v8 + 16)
              + *(float *)(result + 40) * *(float *)(v8 + 24)
              + *(float *)(result + 56);
        v25 = *(_DWORD *)(v8 + 12);
        if ( v25 )
        {
          *a2 = *(float *)(v8 + 32) * *a2;
          a2[1] = a2[1] * *(float *)(v8 + 32);
          a2[2] = *(float *)(v8 + 32) * a2[2];
          do
          {
            v26 = *(_DWORD *)(v18 + 12);
            v27 = *(float *)(v18 + 16);
            v28 = *(float *)(v26 + a4 + 32);
            result = a4 + v26;
            v29 = v28 * *(float *)(v18 + 8);
            --v25;
            v18 += 20;
            *a2 = (v29
                 + *(float *)(result + 16) * *(float *)(v18 - 16)
                 + *(float *)(v18 - 20) * *(float *)result
                 + *(float *)(result + 48))
                * v27
                + *a2;
            a2[1] = (*(float *)(result + 36) * *(float *)(v18 - 12)
                   + *(float *)(result + 4) * *(float *)(v18 - 20)
                   + *(float *)(result + 20) * *(float *)(v18 - 16)
                   + *(float *)(result + 52))
                  * v27
                  + a2[1];
            a2[2] = (*(float *)(result + 40) * *(float *)(v18 - 12)
                   + *(float *)(result + 8) * *(float *)(v18 - 20)
                   + *(float *)(result + 24) * *(float *)(v18 - 16)
                   + *(float *)(result + 56))
                  * v27
                  + a2[2];
          }
          while ( v25 );
          v8 += 36;
        }
        else
        {
          v8 += 32;
        }
        a2 += 3;
        --v19;
      }
      while ( v19 );
    }
  }
  else
  {
    v16 = *(__int16 *)(result + 2);
    if ( *(_WORD *)(result + 2) )
    {
      result = v8 + 16;
      do
      {
        if ( a5 )
        {
          a5 += 12;
          *(float *)(a5 - 12) = *(float *)v8 * *(float *)(v10 + a4)
                              + *(float *)(result - 8) * *(float *)(v10 + a4 + 32)
                              + *(float *)(v10 + a4 + 16) * *(float *)(result - 12);
          *(float *)(a5 - 8) = *(float *)(v10 + a4 + 4) * *(float *)v8
                             + *(float *)(v10 + a4 + 36) * *(float *)(result - 8)
                             + *(float *)(v10 + a4 + 20) * *(float *)(result - 12);
          *(float *)(a5 - 4) = *(float *)(v10 + a4 + 8) * *(float *)v8
                             + *(float *)(v10 + a4 + 40) * *(float *)(result - 8)
                             + *(float *)(v10 + a4 + 24) * *(float *)(result - 12);
        }
        v8 += 24;
        v17 = *(float *)(v10 + a4 + 16) * *(float *)result;
        result += 24;
        a2 += 3;
        --v16;
        *(a2 - 3) = v17
                  + *(float *)(result - 20) * *(float *)(v10 + a4 + 32)
                  + *(float *)(result - 28) * *(float *)(v10 + a4)
                  + *(float *)(v10 + a4 + 48);
        *(a2 - 2) = *(float *)(result - 24) * *(float *)(v10 + a4 + 20)
                  + *(float *)(v10 + a4 + 4) * *(float *)(result - 28)
                  + *(float *)(v10 + a4 + 36) * *(float *)(result - 20)
                  + *(float *)(v10 + a4 + 52);
        *(a2 - 1) = *(float *)(result - 24) * *(float *)(v10 + a4 + 24)
                  + *(float *)(v10 + a4 + 8) * *(float *)(result - 28)
                  + *(float *)(v10 + a4 + 40) * *(float *)(result - 20)
                  + *(float *)(v10 + a4 + 56);
      }
      while ( v16 );
    }
  }
  return result;
}

/* ---- XModelGetNumLods  0x0048BCD0 ----  VERIFIED */
int __cdecl XModelGetNumLods(const XModel *model)
{
    return model->info->lodCount;
}

/* ---- XModelGetModelFileCount  0x0048BCE0 ----  VERIFIED */
int __cdecl XModelGetModelFileCount(const XModel *model)
{
    return model->info->modelFileCount;
}

/* ---- XModelSetTestLods  0x0048BCF0 ----  VERIFIED */
void __cdecl XModelSetTestLods(int lodIndex, float distance)
{
    if ( lodIndex == 0 )
        xmodel_testLodsEnabled = (unsigned char)(distance >= 0.0f);
    if ( distance < 0.0f )
        distance = 0.0f;
    xmodel_testLodDistances[lodIndex].distance = distance;
}

/* ---- XModelSetTestLodDist  0x0048BD40 ----  VERIFIED */
void __cdecl XModelSetTestLodDist(float distance)
{
    if ( distance <= 0.0f )
        xmodel_testLodDist = 0.0f;
    else
        xmodel_testLodDist = distance;
}

/* ---- XModelGetLodForDist  0x0048BD70 ----  VERIFIED */
int __cdecl XModelGetLodForDist(const XModel *model, float distance)
{
    const XModelLodInfo *lod;
    int lodCount;
    int lodIndex;

    if ( xmodel_testLodsEnabled )
        lod = xmodel_testLodDistances;
    else
        lod = model->info->lodRecords;

    if ( xmodel_testLodDist != 0.0f )
        distance = xmodel_testLodDist;

    lodCount = model->info->lodCount;
    for ( lodIndex = 0; lodIndex < lodCount; ++lodIndex, ++lod )
    {
        if ( lod->distance == 0.0f )
            return lodIndex;
        if ( distance < lod->distance )
            return lodIndex;
    }
    return -1;
}

/* ---- XModelGetBasePose  0x0048BDE0 ----  VERIFIED */
void __cdecl XModelGetBasePose(XModel *model, DObjSkelMat *basePose)
{
    const XModelPartsData *parts = model->info->parts->data;
    const XModelPartNameTableSlot *slot = parts->partNameTableSlot;
    const unsigned char *deltas = slot->parentPartDeltas;
    const short (*rotation)[4] = parts->baseRotations;
    const float (*translation)[3] = parts->baseTranslations;
    int totalPartCount = slot->partNameTable->count;
    int rootPartCount = parts->rootPartCount;
    float *row = (float *)basePose;
    int i;

    for ( i = rootPartCount; i; --i )
    {
        row[0] = 0.0f;
        row[1] = 0.0f;
        row[2] = 0.0f;
        row[3] = 1.0f;
        row += 16;
    }

    for ( i = rootPartCount; i < totalPartCount; ++i )
    {
        int child = i - rootPartCount;
        float qx = (float)rotation[child][0] * 0.000030518509f;
        float qy = (float)rotation[child][1] * 0.000030518509f;
        float qz = (float)rotation[child][2] * 0.000030518509f;
        float qw = (float)rotation[child][3] * 0.000030518509f;
        const float *parent = &row[-16 * (int)deltas[child]];

        row[0] = qz * parent[1] + qx * parent[3] + qw * parent[0] - qy * parent[2];
        row[1] = qy * parent[3] - qz * parent[0] + qx * parent[2] + qw * parent[1];
        row[2] = qz * parent[3] + qy * parent[0] - qx * parent[1] + qw * parent[2];
        row[3] = qw * parent[3] - qx * parent[0] - qy * parent[1] - qz * parent[2];
        row += 16;
    }

    row = (float *)basePose;
    for ( i = rootPartCount; i; --i )
    {
        row[0]  = 1.0f; row[1] = 0.0f; row[2]  = 0.0f;
        row[4]  = 0.0f; row[5] = 1.0f; row[6]  = 0.0f;
        row[8]  = 0.0f; row[9] = 0.0f; row[10] = 1.0f;
        row[14] = 0.0f;
        row[13] = 0.0f;
        row[12] = 0.0f;
        row += 16;
    }

    for ( i = rootPartCount; i < totalPartCount; ++i )
    {
        int child = i - rootPartCount;
        const float *parent;
        const float *t;

        XModelExpandQuatToAxis(row);
        parent = &row[-16 * (int)deltas[child]];
        t = translation[child];

        row[12] = parent[8] * t[2] + parent[4] * t[1] + t[0] * parent[0] + parent[12];
        row[13] = parent[9] * t[2] + parent[5] * t[1] + parent[1] * t[0] + parent[13];
        row[14] = parent[10] * t[2] + parent[6] * t[1] + parent[2] * t[0] + parent[14];
        row += 16;
    }
}

/* ---- sub_48BDE0  no-address ---- */
void __cdecl sub_48BDE0(XModel *model, DObjSkelMat *basePose)
{
    XModelGetBasePose(model, basePose);
}

extern qboolean CM_TraceBox( const float *start, const float *end,
                             const float *mins, const float *maxs,
                             float fraction );

/* ---- XModelTraceLine  0x0048C070 ----  VERIFIED */
int __cdecl XModelTraceLine(
        int          model,        /* @<eax>  XModel *                        */
        int          work,         /* @<esi>  48-byte trace record, narrowed  */
        const float *basePose,
        const float *start,
        const float *end,
        int          contentmask)
{
  unsigned char *collLod;
  unsigned char *surf;
  const float   *tris;
  const float   *tri;
  const float   *m;
  int            numSurfs, surfIndex, surfOffset;
  int            numTris, triIndex;
  int            boneIndex, hitBone;
  float          localStart[3];
  float          localEnd[3];
  float          delta[3];
  float          mid[3];
  float          dx, dy, dz;
  float          dStart, dEnd, denom, frac, t, u, v;
  float          n0, n1, n2;

  collLod  = *(unsigned char **)(model + 4);
  hitBone  = -1;
  numSurfs = *(int *)(collLod + 0x44);
  if ( numSurfs <= 0 )
    return -1;

  surfOffset = 0;
  for ( surfIndex = 0; surfIndex < numSurfs; surfIndex++, surfOffset += 44 )
  {
    surf = *(unsigned char **)(collLod + 0x40) + surfOffset;

    if ( ( *(int *)(surf + 0x24) & contentmask ) == 0 )
      continue;

    boneIndex = *(int *)(surf + 0x20);
    m         = basePose + ( boneIndex << 4 );

    dx = start[0] - m[12];
    dy = start[1] - m[13];
    dz = start[2] - m[14];
    localStart[0] = dx * m[0] + dz * m[2]  + dy * m[1];
    localStart[1] = dz * m[6] + dy * m[5]  + dx * m[4];
    localStart[2] = dx * m[8] + dz * m[10] + dy * m[9];

    dx = end[0] - m[12];
    dy = end[1] - m[13];
    dz = end[2] - m[14];
    localEnd[0] = dx * m[0] + dz * m[2]  + dy * m[1];
    localEnd[1] = dz * m[6] + dy * m[5]  + dx * m[4];
    localEnd[2] = dx * m[8] + dz * m[10] + dy * m[9];

    delta[0] = localEnd[0] - localStart[0];
    delta[1] = localEnd[1] - localStart[1];
    delta[2] = localEnd[2] - localStart[2];

    if ( CM_TraceBox( localStart, localEnd,
                      (const float *)( surf + 0x08 ),
                      (const float *)( surf + 0x14 ),
                      *(float *)work ) != qfalse )
      continue;

    numTris = *(int *)(surf + 4);
    tris    = *(const float **)surf;

    for ( triIndex = 0; triIndex < numTris; triIndex++ )
    {
      tri = tris + triIndex * 12;

      dEnd = localEnd[0] * tri[0] + localEnd[2] * tri[2] + localEnd[1] * tri[1]
             - tri[3];
      if ( dEnd >= 0.0f )
        continue;

      dStart = localStart[0] * tri[0] + localStart[2] * tri[2]
               + localStart[1] * tri[1] - tri[3];
      if ( dStart <= 0.0f )
        continue;

      denom = dStart - dEnd;
      frac  = ( dStart - 0.125f ) / denom;
      if ( frac >= *(float *)work )
        continue;

      t      = dStart / denom;
      mid[0] = delta[0] * t + localStart[0];
      mid[1] = delta[1] * t + localStart[1];
      mid[2] = t * delta[2] + localStart[2];

      u = mid[2] * tri[6] + mid[1] * tri[5] + mid[0] * tri[4] - tri[7];
      if ( u < -0.001f || u > 1.001f )
        continue;

      v = mid[2] * tri[10] + mid[1] * tri[9] + mid[0] * tri[8] - tri[11];
      if ( v < -0.001f || u + v > 1.001f )
        continue;

      *(unsigned char *)( work + 47 ) = 0;
      *(unsigned char *)( work + 46 ) = 0;
      *(float *)work                  = frac;
      *(int *)( work + 28 )           = *(int *)( surf + 0x28 );
      *(int *)( work + 32 )           = *(int *)( surf + 0x24 );
      *(float *)( work + 16 )         = tri[0];
      *(float *)( work + 20 )         = tri[1];
      *(float *)( work + 24 )         = tri[2];
      hitBone                         = boneIndex;
    }
  }

  if ( hitBone < 0 )
    return -1;

  m  = basePose + ( hitBone << 4 );
  n0 = m[8]  * *(float *)( work + 24 ) + m[4] * *(float *)( work + 20 )
       + m[0] * *(float *)( work + 16 );
  n1 = m[9]  * *(float *)( work + 24 ) + m[5] * *(float *)( work + 20 )
       + m[1] * *(float *)( work + 16 );
  n2 = m[10] * *(float *)( work + 24 ) + m[6] * *(float *)( work + 20 )
       + m[2] * *(float *)( work + 16 );
  *(float *)( work + 24 ) = n2;
  *(float *)( work + 16 ) = n0;
  *(float *)( work + 20 ) = n1;

  return hitBone;
}

/* ---- XModelGetStaticBounds  0x0048C3E0 ----  VERIFIED */
int __cdecl XModelGetStaticBounds(int a1, float *a2, float *a3, float *a4)
{
  int v4;
  float *v6;
  int i;
  float v8;
  float v9;
  float v10;
  bool v11;
  int v12;
  int v13;
  int v14;
  float v15;
  float v16;
  float v17;
  float v18;
  float v19;

  v4 = *(_DWORD *)(a1 + 4);
  v14 = v4;
  if ( !*(_DWORD *)(v4 + 68) )
    return 0;
  *a2 = 3.4028235e38;
  a2[1] = 3.4028235e38;
  a2[2] = 3.4028235e38;
  *a4 = -3.4028235e38;
  a4[1] = -3.4028235e38;
  a4[2] = -3.4028235e38;
  v13 = 0;
  if ( *(int *)(v4 + 68) > 0 )
  {
    v12 = 0;
    while ( 1 )
    {
      v6 = (float *)(v12 + *(_DWORD *)(v4 + 64));
      for ( i = 0; i < 8; ++i )
      {
        if ( (i & 1) != 0 )
          v8 = v6[2];
        else
          v8 = v6[5];
        v15 = v8;
        if ( (i & 2) != 0 )
          v9 = v6[3];
        else
          v9 = v6[6];
        v16 = v9;
        if ( (i & 4) != 0 )
          v10 = v6[4];
        else
          v10 = v6[7];
        v17 = v15 * *a3 + v10 * a3[6] + v16 * a3[3];
        v18 = v15 * a3[1] + v10 * a3[7] + v16 * a3[4];
        v19 = v10 * a3[8] + v16 * a3[5] + v15 * a3[2];
        if ( *a2 > (double)v17 )
          *a2 = v17;
        if ( (*a4 < (double)v17) | __UNORDERED__(*a4, v17) )
          *a4 = v17;
        if ( a2[1] > (double)v18 )
          a2[1] = v18;
        if ( (a4[1] < (double)v18) | __UNORDERED__(a4[1], v18) )
          a4[1] = v18;
        if ( a2[2] > (double)v19 )
          a2[2] = v19;
        if ( (a4[2] < (double)v19) | __UNORDERED__(a4[2], v19) )
          a4[2] = v19;
      }
      v11 = ++v13 < *(_DWORD *)(v14 + 68);
      v12 += 44;
      if ( !v11 )
        break;
      v4 = v14;
    }
  }
  return 1;
}
