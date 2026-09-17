/*
 * Machine-translated from Call of Duty 1.1 (Windows, CoDMP.exe).
 * Original translation unit:
 * /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/xanim/xanim.cpp
 * Retail range 0x00484000-0x00487F00, 56 functions.
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"

#define xanim_deferredNotifies xanim_generatedNotifyDecl_unused
#define xanim_pool xanim_generatedPoolDecl_unused
#define xanim_evalCurrentTime xanim_generatedEvalEndTimeDecl_unused
#define xanim_evalStartTime xanim_generatedEvalStartTimeDecl_unused
#include "../qcommon/cod1_globals.h"
#undef xanim_deferredNotifies
#undef xanim_pool
#undef xanim_evalCurrentTime
#undef xanim_evalStartTime

extern float xanim_evalCurrentTime;    /* 0x00A7A614  xanim_evalCurrentTime */
extern float xanim_evalStartTime;    /* 0x00A9CC7C  xanim_evalStartTime   */

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

extern XAnimInfo xanim_pool[2048];

#define XANIM_LANE(h)   ( xanim_pool[h].states[xanim_activePoolSlot] )

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

typedef struct XAnimDeferredNotify_s {
    const char    *name;         /* +0x00 */
    unsigned short notifyHandle; /* +0x04 */
    unsigned short pad06;
    float          timeFrac;     /* +0x08 */
} XAnimDeferredNotify;

XAnimDeferredNotify xanim_deferredNotifies[128];

extern int IncInParam__Fv();
extern int SL_FreeString__FUsPCcUi();
extern void __cdecl Scr_AddConstString( unsigned short handle );
extern const char *SL_ConvertToString( unsigned short handle );
extern unsigned short SL_GetStringOfLen( const char *text, unsigned char user,
                                         unsigned int size, int type );
extern void        SL_RemoveRefToStringOfLen( unsigned short handle,
                                              unsigned int size );
extern void        SL_AddRefToString( unsigned short handle );
extern void __cdecl Scr_NotifyId( unsigned short id, unsigned short name,
                                  unsigned int paramCount );

typedef struct XAnimModelPartNameTable_s {
    short          count;                    /* +0x00  SIGNED (movsx) */
    unsigned short handles[1];               /* +0x02 */
} XAnimModelPartNameTable;

#define XANIM_REMAP_PREFIX          16
#define XANIM_REMAP_SENTINEL        127
#define XANIM_REMAP_STRING_TYPE     11

/* ---- XAnimAllocTree  0x00484000 ----  VERIFIED */
XAnim *__cdecl XAnimAllocTree( int nodeCount, const char *name, void *(__cdecl *alloc)(int) )
{
    XAnim *tree = (XAnim *)alloc( 8 * nodeCount + 8 );

    tree->name      = name;
    tree->nodeCount = nodeCount;
    return tree;
}

/*
 * Runtime-tree creation path:
 *
 *   cgame trap 131         one argument, the source tree
 *   engine case 131        0x00403783 `mov ebx, [esi+4]` then
 *                          push Hunk_AllocXAnimClientCreateTree -- args[1] is
 *                          the source tree
 *   XAnimAllocTree         0x00484000  8n+8, name at +0, nodeCount at +4
 *   XAnimAllocRuntimeTree  0x00484020  (2n+8) + 2*(3n+1) = 8n+10, Com_Memset
 *                          of the whole block, sourceTree stored at +0
 *   XAnimCopyRuntimeTree   0x00484050  same 8n+10 handed to the copy callback
 *   Hunk_AllocXAnimClientCreate / ...CreateTree  0x00401CE0 / 0x00401CF0 are
 *                          byte-identical in retail -- `mov eax,20h;
 *                          mov ecx,[esp+size]; jmp Hunk_AllocAlignInternal`
 *   Hunk_AllocAlignInternal  size first, align second, high PERMANENT hunk,
 *                          so a tree is not reclaimed under the client
 */

/* ---- XAnimAllocRuntimeTree  0x00484020 ----  VERIFIED */
XAnimTree *__cdecl XAnimAllocRuntimeTree( XAnim *sourceTree, void *(__cdecl *alloc)(int) )
{
    int n    = sourceTree->nodeCount;
    int size = 2 * n + 8 + 2 * ( 3 * n + 1 );
    XAnimTree *tree = (XAnimTree *)alloc( size );

    Com_Memset( tree, 0, size );
    tree->sourceTree = sourceTree;
    return tree;
}

/* ---- XAnimCopyRuntimeTree  0x00484050 ----  VERIFIED */
int __cdecl XAnimCopyRuntimeTree(XAnimTree *tree, int (__cdecl *copy)(XAnimTree *, int))
{
  int n = tree->sourceTree->nodeCount;

  return copy(tree, 2 * n + 8 + 2 * (3 * n + 1));
}

/* ---- XAnimRuntimeTreeSourceTree  0x00484070 ----  VERIFIED */
XAnim *__cdecl XAnimRuntimeTreeSourceTree( XAnimTree *tree )
{
    return tree->sourceTree;
}

/* ---- XAnimSetModel  0x00484080 ----  VERIFIED */
__int16 __cdecl XAnimSetModel(int a1, int a2, int a3)
{
    unsigned char        remap[XANIM_REMAP_PREFIX + 128];
    const unsigned short *animPartNameTable;
    const unsigned short *animPartNames;
    int                   animPartCount;
    unsigned int          remapSize;
    int                   sourcePart;
    int                   modelIndex;
    int                   i;

    /* entry->u.leafAsset is a fileData_t whose +0x04 is the XAnimParts. */
    animPartNameTable =
        ((XAnimParts *)( *(void **)( *(int *)( a1 + 4 ) + 4 ) ))->partNameHandles;
    animPartCount = *(const short *)animPartNameTable;
    animPartNames = animPartNameTable + 1;
    remapSize     = (unsigned int)( animPartCount + XANIM_REMAP_PREFIX );

    Com_Memset( remap, 0, XANIM_REMAP_PREFIX );
    for ( i = 0; i < animPartCount; ++i ) {
        remap[XANIM_REMAP_PREFIX + i] = XANIM_REMAP_SENTINEL;
    }

    sourcePart = 0;
    for ( modelIndex = 0; modelIndex < a3; ++modelIndex ) {
        const XAnimModelPartNameTable *modelParts;
        int                            modelPartCount;
        int                            modelPart;

        modelParts = ***(XAnimModelPartNameTable ****)
                        ( **(int **)( *(int *)( a2 + 4 * modelIndex ) + 4 ) + 4 );
        modelPartCount = modelParts->count;

        for ( modelPart = 0; modelPart < modelPartCount;
              ++modelPart, ++sourcePart ) {
            unsigned short modelPartName = modelParts->handles[modelPart];
            int            animPart;

            for ( animPart = animPartCount - 1; animPart >= 0; --animPart ) {
                if ( modelPartName != animPartNames[animPart] ) {
                    continue;
                }
                if ( remap[XANIM_REMAP_PREFIX + animPart] == XANIM_REMAP_SENTINEL ) {
                    remap[XANIM_REMAP_PREFIX + animPart] = (unsigned char)sourcePart;
                    remap[sourcePart >> 3] |= (unsigned char)( 1 << ( sourcePart & 7 ) );
                }
                break;
            }
        }
    }

    return (__int16)SL_GetStringOfLen( (const char *)remap, 0, remapSize,
                                       XANIM_REMAP_STRING_TYPE );
}
#if 0
// XAnimSetModel: builds the anim-part -> model-bone remap table for a model set (127 sentinel, 16-byte XAnimToXModel bitset header) and interns it as a string.
__int16 __cdecl XAnimSetModel(int a1, int a2, int a3)
{
  int v3;
  int v4;
  unsigned int v5;
  void *v6;
  int v7;
  int v8;
  int v9;
  __int16 *v10;
  int v11;
  int v12;
  __int16 v13;
  int v14;
  char v16[12]; // [esp+0h] [ebp-24h] BYREF
  unsigned int v17;
  int v18;
  int v19;
  int i;
  __int16 *v21;
  int v22;

  v3 = *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4);
  v4 = **(__int16 **)(v3 + 12);
  v19 = *(_DWORD *)(v3 + 12);
  v5 = v4 + 16;
  v6 = alloca(v4 + 16);
  memset(v16, 0, sizeof(v16));
  v17 = 0;
  v7 = v4 - 1;
  for ( i = v4 - 1; v7 >= 0; *((_BYTE *)&v18 + v7 + 1) = 127 )
    --v7;
  v8 = 0;
  v9 = 0;
  v22 = 0;
  if ( a3 > 0 )
  {
    do
    {
      v10 = ***(__int16 ****)(**(_DWORD **)(*(_DWORD *)(a2 + 4 * v8) + 4) + 4);
      v11 = *v10;
      v12 = 0;
      v21 = v10;
      v18 = v11;
      if ( v11 > 0 )
      {
        while ( 1 )
        {
          v13 = v10[v12 + 1];
          v14 = i;
          if ( i >= 0 )
          {
            while ( v13 != *(_WORD *)(v19 + 2 * v14 + 2) )
            {
              if ( --v14 < 0 )
                goto LABEL_13;
            }
            if ( v16[v14 + 16] == 127 )
            {
              v16[v14 + 16] = v9;
              v16[v9 >> 3] |= 1 << (v9 & 7);
            }
          }
LABEL_13:
          ++v12;
          ++v9;
          if ( v12 >= v18 )
            break;
          v10 = v21;
        }
        v8 = v22;
      }
      v22 = ++v8;
    }
    while ( v8 < a3 );
    v5 = v17;
  }
  return SL_GetStringOfLen(v16, 0, v5);
}
#endif

/* ---- XAnimFindByteKey  0x00484180 ----  VERIFIED */
int __cdecl XAnimFindByteKey( const unsigned char *keys, int targetKey, float frameFrac, int keyCount )
{
    int hi  = keyCount;
    int lo  = 0;
    int mid = (int)( (double)keyCount * frameFrac );

    if ( targetKey >= keys[mid] )
    {
        while ( targetKey >= keys[mid + 1] )
        {
            lo = mid + 1;
            for ( ;; )
            {
                mid = (lo + hi) / 2;
                if ( targetKey >= keys[mid] )
                    break;
                ++lo;
                hi = mid;
                if ( targetKey < keys[lo] )
                    return lo - 1;
            }
        }
        return mid;
    }

    hi = mid;
    for ( ;; )
    {
        mid = (lo + hi) / 2;
        if ( targetKey < keys[mid] )
        {
            hi = mid;
            continue;
        }
        if ( targetKey < keys[mid + 1] )
            return mid;
        --hi;
        lo = mid + 1;
        if ( targetKey >= keys[hi] )
            break;
    }
    return hi;
}

/* ---- XAnimFindShortKey  0x00484200 ----  VERIFIED */
int __cdecl XAnimFindShortKey( const unsigned short *keys, int targetKey, float frameFrac, int keyCount )
{
    int hi  = keyCount;
    int lo  = 0;
    int mid = (int)( (double)keyCount * frameFrac );

    if ( targetKey >= keys[mid] )
    {
        while ( targetKey >= keys[mid + 1] )
        {
            lo = mid + 1;
            for ( ;; )
            {
                mid = (lo + hi) / 2;
                if ( targetKey >= keys[mid] )
                    break;
                ++lo;
                hi = mid;
                if ( targetKey < keys[lo] )
                    return lo - 1;
            }
        }
        return mid;
    }

    hi = mid;
    for ( ;; )
    {
        mid = (lo + hi) / 2;
        if ( targetKey < keys[mid] )
        {
            hi = mid;
            continue;
        }
        if ( targetKey < keys[mid + 1] )
            return mid;
        --hi;
        lo = mid + 1;
        if ( targetKey >= keys[hi] )
            break;
    }
    return hi;
}

/* ---- XAnimCalcPartsSmallIndices  0x00484280 ---- */
typedef struct XAnimPartStream_s {
    void  *transStream;   /* +0x00  vec3 float keys, or a single static vec3 */
    __int16 *rotStream;   /* +0x04  int16 quat keys, or a single static pair/quad */
} XAnimPartStream;

/* ---- XAnimCalcPartsSmallIndices  0x00484280 ----  VERIFIED */
int __cdecl XAnimCalcPartsSmallIndices( const XAnimParts *parts, const unsigned char *remap,
                        float frameFrac, float weight, DObjAnimMat *matArray )
{
    unsigned short         frameCount    = parts->frameCountMinusOne;
    double                 fullTime      = (double)frameCount * frameFrac;
    float                  t             = (float)fullTime;
    int                    seed          = (int)fullTime;
    float                  rotScale      = weight * 0.000030518509f;
    int                    animPartCount = *(const short *)parts->partNameHandles;
    const XAnimPartStream *streams       = (const XAnimPartStream *)parts->partStreams;
    const unsigned char   *rot2Bits      = parts->compressedRotationBits;
    int                    i;

    for ( i = 0; i < animPartCount; ++i )
    {
        int bone = remap[i];

        if ( ( (1 << (bone & 7)) & *((const unsigned char *)xanim_evalSkipBits + (bone >> 3)) ) != 0 )
            continue;

        {
            const XAnimPartStream *stream = &streams[i];
            const __int16         *rot    = stream->rotStream;
            DObjAnimMat            *mat   = (DObjAnimMat *)( (char *)matArray + 32 * bone );
            double                  wRaw;
            int                     haveWRaw = 0;

            if ( rot2Bits[i >> 3] & (1 << (i & 7)) )
            {
                if ( !rot )
                {
                    mat->quat[3] += weight;
                }
                else if ( !rot[2] )
                {
                    mat->quat[2] += (double)rot[0] * rotScale;
                    mat->quat[3] += (double)rot[1] * rotScale;
                }
                else
                {
                    unsigned short keyCount = (unsigned short)rot[2];
                    const unsigned char *keys = (const unsigned char *)rot + 6;
                    int    idx;
                    double frac;

                    if ( keyCount >= frameCount ) {
                        idx  = seed;
                        frac = (double)t - seed;
                    } else {
                        idx  = XAnimFindByteKey( keys, seed, frameFrac, keyCount );
                        frac = ((double)t - keys[idx]) / (double)(keys[idx + 1] - keys[idx]);
                    }

                    /* dataPtr at rot+0x00 -- a union with the static pair above. 4 bytes/key: {z,w}. */
                    {
                        const __int16 *data = *(const __int16 **)rot;
                        int k = idx;

                        mat->quat[2] += ( (double)(data[2 * (k + 1)] - data[2 * k]) * frac
                                         + (double)data[2 * k] ) * rotScale;
                        {
                            __int16 wLo = data[2 * k + 1];
                            wRaw = (double)(data[2 * (k + 1) + 1] - wLo) * frac + (double)wLo;
                            haveWRaw = 1;
                        }
                    }
                }
            }
            else
            {
                if ( !rot[2] )
                {
                    mat->quat[0] += (double)rot[0] * rotScale;
                    mat->quat[1] += (double)rot[1] * rotScale;
                    mat->quat[2] += (double)rot[3] * rotScale;
                    mat->quat[3] += (double)rot[4] * rotScale;
                }
                else
                {
                    unsigned short keyCount = (unsigned short)rot[2];
                    const unsigned char *keys = (const unsigned char *)rot + 6;
                    int    idx;
                    double frac;

                    if ( keyCount >= frameCount ) {
                        idx  = seed;
                        frac = (double)t - seed;
                    } else {
                        idx  = XAnimFindByteKey( keys, seed, frameFrac, keyCount );
                        frac = ((double)t - keys[idx]) / (double)(keys[idx + 1] - keys[idx]);
                    }

                    /* dataPtr at rot+0x00 -- a union with the static quad above. 8 bytes/key: {x,y,z,w}. */
                    {
                        const __int16 *data = *(const __int16 **)rot;
                        int k = idx;

                        mat->quat[0] += ( (double)(data[4*(k+1)]   - data[4*k])   * frac + (double)data[4*k]   ) * rotScale;
                        mat->quat[1] += ( (double)(data[4*(k+1)+1] - data[4*k+1]) * frac + (double)data[4*k+1] ) * rotScale;
                        mat->quat[2] += ( (double)(data[4*(k+1)+2] - data[4*k+2]) * frac + (double)data[4*k+2] ) * rotScale;
                        {
                            __int16 wLo = data[4 * k + 3];
                            wRaw = (double)(data[4 * (k + 1) + 3] - wLo) * frac + (double)wLo;
                            haveWRaw = 1;
                        }
                    }
                }
            }

            {
                void *trans = stream->transStream;

                if ( trans )
                {
                    unsigned short keyCount = *(const unsigned short *)( (char *)trans + 4 );

                    if ( keyCount )
                    {
                        int    idx;
                        double frac;

                        if ( keyCount >= frameCount ) {
                            idx  = seed;
                            frac = (double)t - seed;
                        } else {
                            idx  = XAnimFindByteKey( (const unsigned char *)trans + 6, seed, frameFrac, keyCount );
                            frac = ((double)t - *((const unsigned char *)trans + 6 + idx))
                                 / (double)( *((const unsigned char *)trans + 6 + idx + 1)
                                           - *((const unsigned char *)trans + 6 + idx) );
                        }

                        {
                            const float *data = *(const float **)trans;
                            const float *key0 = data + 3 * idx;
                            const float *key1 = key0 + 3;

                            mat->trans[0] += ( (key1[0] - key0[0]) * frac + key0[0] ) * weight;
                            mat->trans[1] += ( (key1[1] - key0[1]) * frac + key0[1] ) * weight;
                            mat->trans[2] += ( (key1[2] - key0[2]) * frac + key0[2] ) * weight;
                        }
                    }
                    else
                    {
                        /* static vec3 at +0x00,+0x08,+0x0C -- skipping the +0x04 keycount slot */
                        mat->trans[0] += weight * *(const float *)( (char *)trans + 0 );
                        mat->trans[1] += weight * *(const float *)( (char *)trans + 8 );
                        mat->trans[2] += weight * *(const float *)( (char *)trans + 12 );
                    }
                }

                if ( haveWRaw )
                    mat->quat[3] += wRaw * rotScale;
                mat->weight += weight;
            }
        }
    }
    return animPartCount;
}

/* ---- XAnimCalcPartsLargeIndices  0x00484670 ----  VERIFIED */
int __cdecl XAnimCalcPartsLargeIndices( const XAnimParts *parts, const unsigned char *remap,
                        float frameFrac, float weight, DObjAnimMat *matArray )
{
    unsigned short         frameCount    = parts->frameCountMinusOne;
    double                 fullTime      = (double)frameCount * frameFrac;
    float                  t             = (float)fullTime;
    int                    seed          = (int)fullTime;
    float                  rotScale      = weight * 0.000030518509f;
    int                    animPartCount = *(const short *)parts->partNameHandles;
    const XAnimPartStream *streams       = (const XAnimPartStream *)parts->partStreams;
    const unsigned char   *rot2Bits      = parts->compressedRotationBits;
    int                    i;

    for ( i = 0; i < animPartCount; ++i )
    {
        int bone = remap[i];

        if ( ( (1 << (bone & 7)) & *((const unsigned char *)xanim_evalSkipBits + (bone >> 3)) ) != 0 )
            continue;

        {
            const XAnimPartStream *stream = &streams[i];
            const __int16         *rot    = stream->rotStream;
            DObjAnimMat            *mat   = (DObjAnimMat *)( (char *)matArray + 32 * bone );
            double                  wRaw;
            int                     haveWRaw = 0;

            if ( rot2Bits[i >> 3] & (1 << (i & 7)) )
            {
                if ( !rot )
                {
                    mat->quat[3] += weight;
                }
                else if ( !rot[2] )
                {
                    mat->quat[2] += (double)rot[0] * rotScale;
                    mat->quat[3] += (double)rot[1] * rotScale;
                }
                else
                {
                    unsigned short keys = (unsigned short)rot[2];
                    int    idx;
                    double frac;

                    if ( keys >= frameCount ) {
                        idx  = seed;
                        frac = (double)t - seed;
                    } else {
                        idx  = XAnimFindShortKey( (const unsigned short *)rot + 3, seed, frameFrac, keys );
                        {
                            unsigned short k0 = (unsigned short)rot[idx + 3];
                            unsigned short k1 = (unsigned short)rot[idx + 4];
                            frac = ((double)t - k0) / (double)(k1 - k0);
                        }
                    }

                    {
                        const __int16 *data = *(const __int16 **)rot;
                        int k = idx;

                        mat->quat[2] += ( (double)(data[2 * (k + 1)] - data[2 * k]) * frac
                                         + (double)data[2 * k] ) * rotScale;
                        {
                            __int16 wLo = data[2 * k + 1];
                            wRaw = (double)(data[2 * (k + 1) + 1] - wLo) * frac + (double)wLo;
                            haveWRaw = 1;
                        }
                    }
                }
            }
            else
            {
                if ( !rot[2] )
                {
                    mat->quat[0] += (double)rot[0] * rotScale;
                    mat->quat[1] += (double)rot[1] * rotScale;
                    mat->quat[2] += (double)rot[3] * rotScale;
                    mat->quat[3] += (double)rot[4] * rotScale;
                }
                else
                {
                    unsigned short keys = (unsigned short)rot[2];
                    int    idx;
                    double frac;

                    if ( keys >= frameCount ) {
                        idx  = seed;
                        frac = (double)t - seed;
                    } else {
                        idx  = XAnimFindShortKey( (const unsigned short *)rot + 3, seed, frameFrac, keys );
                        {
                            unsigned short k0 = (unsigned short)rot[idx + 3];
                            unsigned short k1 = (unsigned short)rot[idx + 4];
                            frac = ((double)t - k0) / (double)(k1 - k0);
                        }
                    }

                    {
                        const __int16 *data = *(const __int16 **)rot;
                        int k = idx;

                        mat->quat[0] += ( (double)(data[4*(k+1)]   - data[4*k])   * frac + (double)data[4*k]   ) * rotScale;
                        mat->quat[1] += ( (double)(data[4*(k+1)+1] - data[4*k+1]) * frac + (double)data[4*k+1] ) * rotScale;
                        mat->quat[2] += ( (double)(data[4*(k+1)+2] - data[4*k+2]) * frac + (double)data[4*k+2] ) * rotScale;
                        {
                            __int16 wLo = data[4 * k + 3];
                            wRaw = (double)(data[4 * (k + 1) + 3] - wLo) * frac + (double)wLo;
                            haveWRaw = 1;
                        }
                    }
                }
            }

            {
                void *trans = stream->transStream;

                if ( trans )
                {
                    unsigned short keyCount = *(const unsigned short *)( (char *)trans + 4 );

                    if ( keyCount )
                    {
                        int    idx;
                        double frac;

                        if ( keyCount >= frameCount ) {
                            idx  = seed;
                            frac = (double)t - seed;
                        } else {
                            const unsigned short *tkeys =
                                (const unsigned short *)( (char *)trans + 6 );

                            idx  = XAnimFindShortKey( tkeys, seed, frameFrac, keyCount );
                            frac = ((double)t - tkeys[idx])
                                 / (double)( tkeys[idx + 1] - tkeys[idx] );
                        }

                        {
                            const float *data = *(const float **)trans;
                            const float *key0 = data + 3 * idx;
                            const float *key1 = key0 + 3;

                            mat->trans[0] += ( (key1[0] - key0[0]) * frac + key0[0] ) * weight;
                            mat->trans[1] += ( (key1[1] - key0[1]) * frac + key0[1] ) * weight;
                            mat->trans[2] += ( (key1[2] - key0[2]) * frac + key0[2] ) * weight;
                        }
                    }
                    else
                    {
                        mat->trans[0] += weight * *(const float *)( (char *)trans + 0 );
                        mat->trans[1] += weight * *(const float *)( (char *)trans + 8 );
                        mat->trans[2] += weight * *(const float *)( (char *)trans + 12 );
                    }
                }

                if ( haveWRaw )
                    mat->quat[3] += wRaw * rotScale;
                mat->weight += weight;
            }
        }
    }
    return animPartCount;
}

/* ---- XAnimCalcNonLoopEnd  0x00484A60 ----  VERIFIED */
int __cdecl XAnimCalcNonLoopEnd( XAnimParts *parts, const unsigned char *remap, float weight, DObjAnimMat *matArray )
{
    int   animPartCount = *(const short *)parts->partNameHandles;
    const unsigned char   *rot2Bits = parts->compressedRotationBits;
    const XAnimPartStream *streams  = (const XAnimPartStream *)parts->partStreams;
    float rotScale    = weight * 0.000030518509f;
    int   eaxLeftover = (int)parts->partNameHandles;
    int   i;

    for ( i = 0; i < animPartCount; ++i )
    {
        int bone = remap[i];
        eaxLeftover = bone;

        if ( ( (1 << (bone & 7)) & *((const unsigned char *)xanim_evalSkipBits + (bone >> 3)) ) != 0 )
            continue;

        {
            const XAnimPartStream *stream = &streams[i];
            const __int16         *rot    = stream->rotStream;
            DObjAnimMat            *mat   = (DObjAnimMat *)( (char *)matArray + 32 * bone );
            double                  wRaw;
            float                   wDelta;

            eaxLeftover = (int)mat;

            if ( rot2Bits[i >> 3] & (1 << (i & 7)) )
            {
                if ( !rot )
                {
                    wDelta = weight;
                }
                else
                {
                    unsigned short keyCount = (unsigned short)rot[2];

                    if ( keyCount )
                    {
                        const __int16 *data = (const __int16 *)( *(const int *)rot + 4 * keyCount );
                        mat->quat[2] += (double)data[0] * rotScale;
                        wRaw = data[1];
                    }
                    else
                    {
                        mat->quat[2] += (double)rot[0] * rotScale;
                        wRaw = rot[1];
                    }
                    wDelta = (float)( wRaw * rotScale );
                }
            }
            else
            {
                unsigned short keyCount = (unsigned short)rot[2];

                if ( keyCount )
                {
                    const __int16 *data = (const __int16 *)( *(const int *)rot + 8 * keyCount );
                    mat->quat[0] += (double)data[0] * rotScale;
                    mat->quat[1] += (double)data[1] * rotScale;
                    mat->quat[2] += (double)data[2] * rotScale;
                    wRaw = data[3];
                }
                else
                {
                    mat->quat[0] += (double)rot[0] * rotScale;
                    mat->quat[1] += (double)rot[1] * rotScale;
                    mat->quat[2] += (double)rot[3] * rotScale;
                    wRaw = rot[4];
                }
                wDelta = (float)( wRaw * rotScale );
            }

            mat->quat[3] += wDelta;

            {
                void *trans = stream->transStream;

                if ( trans )
                {
                    unsigned short keyCount = *(const unsigned short *)( (char *)trans + 4 );

                    if ( keyCount )
                    {
                        const float *data = (const float *)( *(const int *)trans + 12 * keyCount );
                        mat->trans[0] += weight * data[0];
                        mat->trans[1] += weight * data[1];
                        mat->trans[2] += weight * data[2];
                    }
                    else
                    {
                        mat->trans[0] += weight * *(const float *)( (char *)trans + 0 );
                        mat->trans[1] += weight * *(const float *)( (char *)trans + 8 );
                        mat->trans[2] += weight * *(const float *)( (char *)trans + 12 );
                    }
                }
                mat->weight += weight;
            }
        }
    }
    return eaxLeftover;
}
#if 0
// XAnimCalcNonLoopEnd
int __cdecl sub_484A60_raw(int a1, int a2, float a3, int a4)
{
  int v4;
  int result;
  int v6;
  int v7;
  int v8;
  int *v9;
  __int16 *v10;
  unsigned __int16 v11;
  double v12;
  __int16 *v13;
  double v14;
  unsigned __int16 v15;
  __int16 *v16;
  int v17;
  unsigned __int16 v18;
  double v19;
  double v20;
  float *v21;
  float v22;
  int v23;

  v4 = a1;
  result = *(_DWORD *)(a1 + 12);
  v6 = *(_DWORD *)(a1 + 28);
  v22 = a3 * 0.000030518509;
  v7 = *(__int16 *)result;
  v8 = 0;
  v23 = v7;
  if ( v7 > 0 )
  {
    while ( 1 )
    {
      result = *(unsigned __int8 *)(v8 + a2);
      if ( ((1 << (*(_BYTE *)(v8 + a2) & 7)) & *((char *)xanim_evalSkipBits + (result >> 3))) == 0 )
        break;
LABEL_19:
      if ( ++v8 >= v7 )
        return result;
    }
    result = a4 + 32 * result;
    v9 = (int *)(*(_DWORD *)(v4 + 16) + 8 * v8);
    v10 = (__int16 *)v9[1];
    if ( ((1 << (v8 & 7)) & *(char *)((v8 >> 3) + v6)) != 0 )
    {
      if ( !v10 )
      {
        v14 = a3;
LABEL_13:
        *(float *)(result + 12) = v14 + *(float *)(result + 12);
        v17 = *v9;
        if ( *v9 )
        {
          v18 = *(_WORD *)(v17 + 4);
          v19 = a3;
          if ( v18 )
          {
            v21 = (float *)(*(_DWORD *)v17 + 12 * v18);
            *(float *)(result + 20) = v19 * *v21 + *(float *)(result + 20);
            *(float *)(result + 24) = a3 * v21[1] + *(float *)(result + 24);
            v20 = a3 * v21[2];
          }
          else
          {
            *(float *)(result + 20) = v19 * *(float *)v17 + *(float *)(result + 20);
            *(float *)(result + 24) = a3 * *(float *)(v17 + 8) + *(float *)(result + 24);
            v20 = a3 * *(float *)(v17 + 12);
          }
          *(float *)(result + 28) = v20 + *(float *)(result + 28);
        }
        v7 = v23;
        v4 = a1;
        *(float *)(result + 16) = a3 + *(float *)(result + 16);
        goto LABEL_19;
      }
      v11 = v10[2];
      if ( v11 )
      {
        v13 = (__int16 *)(*(_DWORD *)v10 + 4 * v11);
        *(float *)(result + 8) = (double)*v13 * v22 + *(float *)(result + 8);
        v12 = (double)v13[1];
      }
      else
      {
        *(float *)(result + 8) = (double)*v10 * v22 + *(float *)(result + 8);
        v12 = (double)v10[1];
      }
    }
    else
    {
      v15 = v10[2];
      if ( v15 )
      {
        v16 = (__int16 *)(*(_DWORD *)v10 + 8 * v15);
        *(float *)result = (double)*v16 * v22 + *(float *)result;
        *(float *)(result + 4) = (double)v16[1] * v22 + *(float *)(result + 4);
        *(float *)(result + 8) = (double)v16[2] * v22 + *(float *)(result + 8);
        v12 = (double)v16[3];
      }
      else
      {
        *(float *)result = (double)*v10 * v22 + *(float *)result;
        *(float *)(result + 4) = (double)v10[1] * v22 + *(float *)(result + 4);
        *(float *)(result + 8) = (double)v10[3] * v22 + *(float *)(result + 8);
        v12 = (double)v10[4];
      }
    }
    v14 = v12 * v22;
    goto LABEL_13;
  }
  return result;
}
#endif

/* ---- XAnimClearData  0x00484C90 ----  VERIFIED */
DObjAnimMat *__cdecl XAnimClearData( DObjAnimMat *mats )
{
    int partCount = xanim_evalPartCount;
    int i;

    for ( i = 0; i < partCount; ++i, ++mats )
    {
        if ( ( (1 << (i & 7)) & *((const unsigned char *)xanim_evalSkipBits + (i >> 3)) ) == 0 )
        {
            mats->quat[0]  = 0.0f;
            mats->quat[1]  = 0.0f;
            mats->quat[2]  = 0.0f;
            mats->quat[3]  = 0.0f;
            mats->weight   = 0.0f;
            mats->trans[2] = 0.0f;
            mats->trans[1] = 0.0f;
            mats->trans[0] = 0.0f;
        }
    }
    return mats;
}

/* ---- XAnimCalcData  0x00484CE0 ----  VERIFIED */
int __cdecl XAnimCalcData( unsigned int *remapBitset, DObjAnimMat *animMats,
                        void *leafAsset, float weight, float frameFrac )
{
    XAnimParts    *parts;
    unsigned char *remapParts = (unsigned char *)remapBitset + 16;

    xanim_evalPosedBits[0] |= *remapBitset & ~xanim_evalSkipBits[0];
    xanim_evalPosedBits[1] |= ~xanim_evalSkipBits[1] & remapBitset[1];
    xanim_evalPosedBits[2] |= ~xanim_evalSkipBits[2] & remapBitset[2];
    xanim_evalPosedBits[3] |= ~xanim_evalSkipBits[3] & remapBitset[3];

    parts = *(XAnimParts **)( (char *)leafAsset + 4 );

    if ( (frameFrac == 1.0) | __UNORDERED__(frameFrac, 1.0) || !parts->frameCountMinusOne )
        return XAnimCalcNonLoopEnd( (int)parts, (int)remapParts, weight, (int)animMats );
    if ( parts->frameCountMinusOne >= 0x100u )
        return XAnimCalcPartsLargeIndices( parts, remapParts, frameFrac, weight, animMats );
    return XAnimCalcPartsSmallIndices( parts, remapParts, frameFrac, weight, animMats );
}

/* ---- XAnimCalcDeltaParts  0x00484D90 ----  VERIFIED */
float *__cdecl XAnimCalcDeltaParts( unsigned __int16 *deltaRecord, float *rotOut, float *transOut, float time )
{
    unsigned short frameCount = *deltaRecord;              /* deltaRecord+0x00 */
    int          *pair = *(int **)( (char *)deltaRecord + 0x18 );
    __int16      *rot  = (__int16 *)pair[1];
    float        *ret;

    if ( !( (time == 1.0) | __UNORDERED__(time, 1.0) ) && frameCount )
    {
        double fullTime = (double)frameCount * time;
        float  t        = (float)fullTime;
        int    seed      = (int)fullTime;

        if ( !rot )
        {
            rotOut[0] = 0.0f;
            rotOut[1] = 32767.0f;
        }
        else if ( !rot[2] )
        {
            rotOut[0] = (float)rot[0];
            rotOut[1] = (float)rot[1];
        }
        else
        {
            unsigned short keyCount = (unsigned short)rot[2];
            int    idx;
            double frac;
            unsigned short k0, k1;

            if ( frameCount >= 0x100u )
            {
                if ( keyCount >= frameCount ) {
                    idx = seed; frac = (double)t - seed;
                } else {
                    idx = XAnimFindShortKey( (const unsigned short *)rot + 3, seed, time, keyCount );
                    k0 = (unsigned short)rot[idx + 3];
                    k1 = (unsigned short)rot[idx + 4];
                    frac = ((double)t - k0) / (double)(k1 - k0);
                }
            }
            else
            {
                const unsigned char *keys = (const unsigned char *)rot + 6;
                if ( keyCount >= frameCount ) {
                    idx = seed; frac = (double)t - seed;
                } else {
                    idx = XAnimFindByteKey( keys, seed, time, keyCount );
                    frac = ((double)t - keys[idx]) / (double)(keys[idx + 1] - keys[idx]);
                }
            }

            {
                const __int16 *data = *(const __int16 **)rot;
                rotOut[0] = (double)(data[2*(idx+1)]   - data[2*idx])   * frac + (double)data[2*idx];
                rotOut[1] = (double)(data[2*(idx+1)+1] - data[2*idx+1]) * frac + (double)data[2*idx+1];
            }
        }

        {
            int *trans = (int *)*pair;

            if ( !trans )
            {
                transOut[0] = transOut[1] = transOut[2] = 0.0f;
                ret = transOut;
            }
            else
            {
                unsigned short keyCount = *(const unsigned short *)( (char *)trans + 4 );

                if ( !keyCount )
                {
                    transOut[0] = *(const float *)trans;
                    transOut[1] = *(const float *)( (char *)trans + 8 );
                    transOut[2] = *(const float *)( (char *)trans + 12 );
                    ret = transOut;
                }
                else
                {
                    int    idx;
                    double frac;
                    unsigned short wk0, wk1;
                    unsigned char  bk0, bk1;

                    if ( frameCount >= 0x100u )
                    {
                        if ( keyCount < frameCount ) {
                            idx = XAnimFindShortKey( (const unsigned short *)( (char *)trans + 6 ), seed, time, keyCount );
                            wk0 = *(const unsigned short *)( (char *)trans + 6 + 2 * idx );
                            wk1 = *(const unsigned short *)( (char *)trans + 8 + 2 * idx );
                            frac = ((double)t - wk0) / (double)(wk1 - wk0);
                        } else {
                            idx = seed; frac = (double)t - seed;
                        }
                    }
                    else
                    {
                        if ( keyCount < frameCount ) {
                            idx = XAnimFindByteKey( (const unsigned char *)trans + 6, seed, time, keyCount );
                            bk0 = *((const unsigned char *)trans + 6 + idx);
                            bk1 = *((const unsigned char *)trans + 7 + idx);
                            frac = ((double)t - bk0) / (double)(bk1 - bk0);
                        } else {
                            idx = seed; frac = (double)t - seed;
                        }
                    }

                    {
                        const float *data = *(const float **)trans;
                        const float *key0 = data + 3 * idx;
                        const float *key1 = key0 + 3;

                        transOut[0] = (key1[0] - key0[0]) * frac + key0[0];
                        transOut[1] = (key1[1] - key0[1]) * frac + key0[1];
                        transOut[2] = (key1[2] - key0[2]) * frac + key0[2];
                        ret = (float *)key0;
                    }
                }
            }
        }
        return ret;
    }

    if ( rot )
    {
        unsigned short keyCount = (unsigned short)rot[2];
        if ( keyCount )
            rot = (__int16 *)( *(int *)rot + 4 * keyCount );
        rotOut[0] = (float)rot[0];
        rotOut[1] = (float)rot[1];
    }
    else
    {
        rotOut[0] = 0.0f;
        rotOut[1] = 32767.0f;
    }

    {
        int *trans = (int *)*pair;

        if ( !trans )
        {
            transOut[0] = transOut[1] = transOut[2] = 0.0f;
            ret = transOut;
        }
        else
        {
            unsigned short keyCount = *((const unsigned short *)trans + 2);

            if ( keyCount )
            {
                int *key = (int *)( *trans + 12 * keyCount );
                transOut[0] = *(const float *)key;
                transOut[1] = *(const float *)( (char *)key + 4 );
                ret = *(float **)( (char *)key + 8 );
                transOut[2] = *(const float *)&ret;
            }
            else
            {
                transOut[0] = *(const float *)trans;
                transOut[1] = *(const float *)( trans + 2 );
                ret = (float *)trans[3];
                transOut[2] = *(const float *)&ret;
            }
        }
    }
    return ret;
}

/* ---- TransformToQuatRefFrame  0x00485170 ----  VERIFIED */
void __fastcall TransformToQuatRefFrame( float *vec, const float *quat )
{
    double q0sq = quat[0] * quat[0];
    double len2 = quat[1] * quat[1] + q0sq;

    if ( !( (len2 == 0.0) | __UNORDERED__(len2, 0.0) ) )
    {
        double s     = 2.0 / len2;
        float  sq0sq = s * q0sq;
        double sq0q1 = quat[1] * quat[0] * s;
        double v0new = (1.0 - sq0sq) * vec[0] + sq0q1 * vec[1];

        vec[1] = vec[1] - (sq0q1 * vec[0] + sq0sq * vec[1]);
        vec[0] = v0new;
    }
}

/* ---- XAnimCalcRelDeltaParts  0x004851E0 ----  VERIFIED */
void __cdecl XAnimCalcRelDeltaParts(float *result, int parts, float weight, float time0, float time1)
{
  int    deltaStream;
  unsigned short deltaKeyCount;
  double trans1x0;
  double trans1z;
  float *lastKey;
  double scale;
  float rot0[2];
  float rot1[2];
  float trans0[3];
  float trans1[3];
  float delta[3];

  XAnimCalcDeltaParts((unsigned __int16 *)parts, rot0, trans0, time0);
  XAnimCalcDeltaParts((unsigned __int16 *)parts, rot1, trans1, time1);
  if ( *(_BYTE *)(parts + 2)
    && (((time1 < (double)time0) | __UNORDERED__(time1, time0))
        && (deltaStream = **(_DWORD **)(parts + 24)) != 0) )
  {
    deltaKeyCount = *(_WORD *)(deltaStream + 4);
    trans1x0 = trans1[0];
    if ( deltaKeyCount )
    {
      lastKey = (float *)(*(_DWORD *)deltaStream + 12 * deltaKeyCount);
      trans1[0] = (float)(trans1x0 + *lastKey);
      trans1[1] = trans1[1] + lastKey[1];
      trans1z = trans1[2] + lastKey[2];
    }
    else
    {
      trans1[0] = (float)(trans1x0 + *(float *)deltaStream);
      trans1[1] = trans1[1] + *(float *)(deltaStream + 8);
      trans1z = trans1[2] + *(float *)(deltaStream + 12);
    }
  }
  else
  {
    trans1z = trans1[2];
  }
  scale = weight * 9.3137942e-10;
  result[0] = (float)((rot0[1] * rot1[0] - rot0[0] * rot1[1]) * scale + result[0]);
  result[1] = (float)((rot1[1] * rot0[1] + rot0[0] * rot1[0]) * scale + result[1]);
  delta[0] = trans1[0] - trans0[0];
  delta[1] = trans1[1] - trans0[1];
  delta[2] = (float)(trans1z - trans0[2]);
  TransformToQuatRefFrame(delta, rot1);
  result[2] = weight + result[2];
  result[3] = delta[0] * weight + result[3];
  result[4] = delta[1] * weight + result[4];
  result[5] = delta[2] * weight + result[5];
}

/* ---- XAnimCalcAbsDeltaParts  0x00485330 ----  VERIFIED */
float *__cdecl XAnimCalcAbsDeltaParts(float *result_acc, unsigned __int16 *parts, float weight, float time)
{
  float *ret;
  double scale;
  float rot[2];
  float trans[3];

  ret = XAnimCalcDeltaParts(parts, rot, trans, time);
  scale = weight * 0.000030518509;
  result_acc[0] = rot[0] * scale + result_acc[0];
  result_acc[1] = rot[1] * scale + result_acc[1];
  result_acc[2] = weight + result_acc[2];
  result_acc[3] = trans[0] * weight + result_acc[3];
  result_acc[4] = trans[1] * weight + result_acc[4];
  result_acc[5] = trans[2] * weight + result_acc[5];
  return ret;
}

/* ---- XAnimClearServerInfo_m  0x004853B0 ----  VERIFIED */
void __cdecl XAnimClearServerInfo_m( XAnimInfo *node )
{
    unsigned short handle = node->notifyName;

    if ( handle ) {
        const char *text = SL_ConvertToString( handle );

        SL_RemoveRefToStringOfLen( handle, (unsigned int)strlen( text ) + 1 );
    }
}
#if 0
// XAnimClearServerInfo_m (coduo XAnimClearServerInfo): clears a pool node's server notify state, releasing its notify-name string.
char *__cdecl XAnimClearServerInfo_m(char *result)
{
  unsigned __int16 v1;

  v1 = *((_WORD *)result + 2);
  if ( v1 )
  {
    result = (char *)(GetRefString_var + 8 * v1);
    if ( *(_WORD *)result )
      --*(_WORD *)result;
    else
      return (char *)SL_FreeString(v1, result + 4, strlen(result + 4) + 1);
  }
  return result;
}
#endif

/* ---- XAnimFreeInfo  0x00485400 ----  VERIFIED */
int __cdecl XAnimFreeInfo( unsigned __int16 a1 )
{
    XAnimInfo      *node = &xanim_pool[a1];
    unsigned short  handle = node->notifyName;
    unsigned short  oldHead;

    if ( handle ) {
        const char *text = SL_ConvertToString( handle );

        SL_RemoveRefToStringOfLen( handle, (unsigned int)strlen( text ) + 1 );
    }

    oldHead = xanim_pool[0].freeNext;
    node->freePrev = 0;
    node->freeNext = oldHead;
    xanim_pool[oldHead].freePrev = a1;
    xanim_pool[0].freeNext = a1;
    --xanim_poolUsedCount;
    return xanim_poolUsedCount;
}
#if 0
// XAnimFreeInfo: returns one XAnimInfo pool node to the free list (head at 0x00A7A62A) and decrements the used count, releasing its notify string.
int __cdecl XAnimFreeInfo(unsigned __int16 a1)
{
  _WORD *v1;
  unsigned __int16 v2;
  int v3;
  int result;

  v1 = (_WORD *)((char *)&xanim_pool + 68 * a1);
  v2 = v1[2];
  if ( v2 )
  {
    v3 = GetRefString_var + 8 * v2;
    if ( *(_WORD *)v3 )
      --*(_WORD *)v3;
    else
      SL_FreeString(v2, (char *)(v3 + 4), strlen((const char *)(v3 + 4)) + 1);
  }
  v1[4] = 0;
  v1[5] = word_A7A62A[0];
  word_A7A628[34 * (unsigned __int16)word_A7A62A[0]] = a1;
  result = xanim_poolUsedCount - 1;
  word_A7A62A[0] = a1;
  --xanim_poolUsedCount;
  return result;
}
#endif

/* ---- XAnimGetAverageRateFrequency  0x00485490 ----  VERIFIED */
double __cdecl XAnimGetAverageRateFrequency( int nodeIndex )
{
    XAnimTree      *tree   = (XAnimTree *)xanim_currentTree;
    XAnimEntry     *entry  = &tree->sourceTree->entries[nodeIndex];
    unsigned short  handle = tree->poolNodeHandles[nodeIndex];

    if ( !handle )
    {
        XAnimParts *parts = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
        return parts->frequency;
    }
    {
        unsigned short childCount = entry->childCount;
        unsigned short firstChild = entry->u.parent.firstChildIndex;
        int   i;
        float weightSum = 0.0f;
        float freqSum   = 0.0f;

        for ( i = 0; i < childCount; ++i )
        {
            unsigned short childHandle = tree->poolNodeHandles[firstChild + i];

            if ( childHandle )
            {
                XAnimState *lane   = &XANIM_LANE( childHandle );
                float       weight = lane->currentWeight;

                if ( !( (weight == 0.0) | __UNORDERED__(weight, 0.0) ) )
                {
                    double childFreq = XAnimGetAverageRateFrequency( firstChild + i );

                    if ( !( (childFreq == 0.0) | __UNORDERED__(childFreq, 0.0) ) )
                    {
                        weightSum += weight;
                        freqSum   += (float)( childFreq * lane->rateScale * weight );
                    }
                }
            }
        }

        if ( (weightSum == 0.0) | __UNORDERED__(weightSum, 0.0) )
            return 0.0;
        return freqSum / weightSum;
    }
}

/* ---- XAnimUpdateInfoNoWeightClient  0x004855A0 ----  VERIFIED */
char __cdecl XAnimUpdateInfoNoWeightClient( int nodeIndex )
{
    XAnimTree     *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];
    XAnimInfo     *node;
    float          settledWeight;
    unsigned short childCount;
    unsigned short firstChild;
    int i;
    int anyChildAlive;

    if ( !handle )
        return 0;

    node = &xanim_pool[handle];
    node->states[0].weightBlendTimeRemaining = 0.0f;
    settledWeight = node->states[0].targetWeight;
    node->states[0].currentWeight = node->states[0].targetWeight;

    childCount = tree->sourceTree->entries[nodeIndex].childCount;
    firstChild = tree->sourceTree->entries[nodeIndex].u.parent.firstChildIndex;

    if ( childCount )
    {
        anyChildAlive = 0;
        for ( i = 0; i < childCount; ++i )
        {
            if ( XAnimUpdateInfoNoWeightClient( i + firstChild ) )
                anyChildAlive = 1;
        }
        if ( anyChildAlive )
            return 1;
        settledWeight = node->states[0].currentWeight;
    }

    if ( ( (settledWeight == 0.0) | __UNORDERED__(settledWeight, 0.0) )
      && ( (node->states[0].targetWeight == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0) )
      && ( (node->states[1].currentWeight == 0.0) | __UNORDERED__(node->states[1].currentWeight, 0.0) )
      && ( (node->states[1].targetWeight == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) ) )
    {
        XAnimFreeInfo( handle );
        tree->poolNodeHandles[nodeIndex] = 0;
        return 0;
    }
    return 1;
}

/* ---- XAnimGetNextNotifyTime  0x004856A0 ----  VERIFIED */
int __cdecl XAnimGetNextNotifyTime( XAnimEntry *entry, int deadArg, float time )
{
    XAnimParts      *parts   = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
    XAnimNotifyInfo *notify  = parts->notify;
    int              addr    = 0;
    float            bestFrac = 2.0f;
    XAnimNotifyInfo *n;

    for ( n = notify; n->name; ++n )
    {
        float frac = n->frameFrac;

        if ( !( (frac < (double)time) | __UNORDERED__(frac, time) )
          && ( (frac < bestFrac) | __UNORDERED__(frac, bestFrac) ) )
        {
            addr     = (int)n;
            bestFrac = frac;
        }
    }
    return (addr - (int)notify) >> 3;
}

/* ---- XAnimGetNotifyFracLeaf  0x00485700 ----  VERIFIED */
double __cdecl XAnimGetNotifyFracLeaf(float notifyTime)
{
  float endTime      = xanim_evalCurrentTime;
  float cycleEndFrac = xanim_evalWindowTime;
  double cycles;

  if ( (cycleEndFrac == 1.0) | __UNORDERED__(cycleEndFrac, 1.0) )
    return 1.0;

  if ( !((endTime < (double)cycleEndFrac) | __UNORDERED__(endTime, cycleEndFrac)) )
  {
    if ( ((!(endTime < (double)notifyTime) && endTime != notifyTime)
          || ((endTime == 1.0) | __UNORDERED__(endTime, 1.0)))
      && cycleEndFrac <= (double)notifyTime )
    {
      cycles = (double)(xanim_evalWindowFrame - xanim_evalStartFrame);
      return (cycles + notifyTime - xanim_evalStartTime) / dword_A9CC70;
    }
  }
  else
  {
    if ( !(endTime < (double)notifyTime) && endTime != notifyTime )
    {
      cycles = (double)(xanim_evalWindowFrame - xanim_evalStartFrame + 1);
      return (cycles + notifyTime - xanim_evalStartTime) / dword_A9CC70;
    }
    if ( cycleEndFrac <= (double)notifyTime )
    {
      cycles = (double)(xanim_evalWindowFrame - xanim_evalStartFrame);
      return (cycles + notifyTime - xanim_evalStartTime) / dword_A9CC70;
    }
  }
  return 1.0;
}

/* ---- XAnimGetNotifyFracServer  0x004857E0 ----  VERIFIED */
double __fastcall XAnimGetNotifyFracServer( XAnimInfo *node, XAnimEntry *entry )
{
    __int16 notifyIndex;

    if ( !xanim_notifyEntId || !node->notifyName )
        return 1.0;

    if ( entry->childCount )
    {
        if ( !node->notifyChildIndex )
            return XAnimGetNotifyFracLeaf( 1.0f );
        entry = &( (XAnimTree *)xanim_currentTree )->sourceTree->entries[ node->notifyChildIndex ];
    }

    notifyIndex = node->notifyIndex;
    if ( notifyIndex < 0 )
        return XAnimGetNotifyFracLeaf( 1.0f );

    {
        XAnimParts *parts = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
        return XAnimGetNotifyFracLeaf( parts->notify[notifyIndex].frameFrac );
    }
}

#define SL_REFSTRING(h)   ( (char *)(void *)GetRefString_var + 8 * (unsigned int)(unsigned short)(h) )
#define SL_STRING(h)      ( (h) ? SL_REFSTRING(h) + 4 : (char *)0 )

/* ---- XAnimAddClientNotify_m  0x00485870 ----  VERIFIED */
void __cdecl XAnimAddClientNotify_m( unsigned short name, int a2, int a3,
                         float timeFrac, __int16 notify )
{
  int i;

  (void)a2;
  (void)a3;

  for ( i = xanim_numDeferredNotifies - 1; i >= 0; i-- )
  {
    if ( !( timeFrac < xanim_deferredNotifies[i].timeFrac ) )
      break;
    xanim_deferredNotifies[i + 1] = xanim_deferredNotifies[i];
  }

  xanim_deferredNotifies[i + 1].name         = SL_STRING( name );
  xanim_deferredNotifies[i + 1].timeFrac     = timeFrac;
  xanim_deferredNotifies[i + 1].notifyHandle = notify;

  ++xanim_numDeferredNotifies;
}

/* ---- XAnimSetClientTime  0x004858F0 ----  VERIFIED */
void __cdecl XAnimSetClientTime( XAnimInfo *node, XAnimEntry *entry, __int16 notify )
{
    node->states[0].time          = xanim_evalCurrentTime;
    node->states[0].oldTime       = xanim_evalStartTime;
    node->states[0].cycleCount    = xanim_evalCurrentFrame;
    node->states[0].oldCycleCount = xanim_evalStartFrame;

    if ( !notify )
        return;
    if ( (dword_A9CC70 == 0.0) | __UNORDERED__(dword_A9CC70, 0.0) )
        return;

    xanim_evalWindowTime = xanim_evalStartTime;
    xanim_evalWindowFrame  = xanim_evalStartFrame;

    if ( (xanim_evalStartTime == 1.0) | __UNORDERED__(xanim_evalStartTime, 1.0) )
    {
    fireShortcut:
        XAnimAddClientNotify_m( xanim_endNotifyStringHandle, (int)node, (int)entry, XAnimGetNotifyFracLeaf( 1.0f ), notify );
        return;
    }

    if ( entry->childCount )
    {
        if ( (xanim_evalCurrentTime < (double)xanim_evalStartTime) | __UNORDERED__(xanim_evalCurrentTime, xanim_evalStartTime)
          || (xanim_evalCurrentTime == 1.0) | __UNORDERED__(xanim_evalCurrentTime, 1.0) )
            goto fireShortcut;
        return;
    }

    {
        XAnimParts      *parts   = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
        unsigned short   idx     = (unsigned short)XAnimGetNextNotifyTime( entry, (int)node, xanim_evalStartTime );
        XAnimNotifyInfo *cursor  = parts->notify + idx;
        float            endTime = xanim_evalCurrentTime;
        int              stepWrapped;

        stepWrapped = (endTime < (double)xanim_evalStartTime) | __UNORDERED__(endTime, xanim_evalStartTime);

        if ( stepWrapped )
        {
            if ( endTime <= cursor->frameFrac )
            {
                if ( xanim_evalStartTime <= (double)cursor->frameFrac )
                {
                    XAnimNotifyInfo *n2;

                    for ( ;; )
                    {
                        XAnimAddClientNotify_m( cursor->name, (int)node, (int)entry, XAnimGetNotifyFracLeaf( cursor->frameFrac ), notify );
                        ++cursor;
                        if ( !cursor->name )
                            break;
                    }

                    n2 = parts->notify;
                    if ( endTime > (double)n2->frameFrac )
                    {
                        int lt, eq;
                        float lookaheadFrac;
                        do
                        {
                            XAnimAddClientNotify_m( n2->name, (int)node, (int)entry, XAnimGetNotifyFracLeaf( n2->frameFrac ), notify );
                            lookaheadFrac = *(float *)( (char *)n2 + 0x14 );
                            lt = endTime < (double)lookaheadFrac;
                            eq = ( endTime == lookaheadFrac );
                            n2 = (XAnimNotifyInfo *)( (char *)n2 + 0x10 );
                        }
                        while ( !lt && !eq );
                    }
                }
            }
            else
            {
                for ( ;; )
                {
                    XAnimAddClientNotify_m( cursor->name, (int)node, (int)entry, XAnimGetNotifyFracLeaf( cursor->frameFrac ), notify );
                    ++cursor;
                    if ( !( cursor->name && endTime > (double)cursor->frameFrac ) )
                        break;
                }
            }
        }
        else if ( (endTime == 1.0) | __UNORDERED__(endTime, 1.0) )
        {
            if ( xanim_evalStartTime <= (double)cursor->frameFrac )
            {
                for ( ;; )
                {
                    XAnimAddClientNotify_m( cursor->name, (int)node, (int)entry, XAnimGetNotifyFracLeaf( cursor->frameFrac ), notify );
                    ++cursor;
                    if ( !cursor->name )
                        break;
                }
            }
        }
        else if ( endTime > (double)cursor->frameFrac && xanim_evalStartTime <= (double)cursor->frameFrac )
        {
            for ( ;; )
            {
                XAnimAddClientNotify_m( cursor->name, (int)node, (int)entry, XAnimGetNotifyFracLeaf( cursor->frameFrac ), notify );
                ++cursor;
                if ( !( cursor->name && endTime > (double)cursor->frameFrac ) )
                    break;
            }
        }
    }
}

/* ---- XAnimUpdateClientInfoSyncInternal  0x00485BA0 ----  VERIFIED */
void __cdecl XAnimUpdateClientInfoSyncInternal( int nodeIndex, int syncFlag )
{
    XAnimTree     *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];

    if ( handle )
    {
        XAnimInfo *node = &xanim_pool[handle];
        float      currentTime = *(float *)&xanim_currentTime;

        if ( (node->states[0].targetWeight == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0) )
            syncFlag = 0;

        if ( currentTime + 0.001 <= node->states[0].weightBlendTimeRemaining )
        {
            float newWeight = (node->states[0].targetWeight - node->states[0].currentWeight)
                             / node->states[0].weightBlendTimeRemaining * currentTime
                             + node->states[0].currentWeight;
            node->states[0].currentWeight = newWeight;
            if ( (newWeight < 0.0000010000001) | __UNORDERED__(newWeight, 0.0000010000001) )
                node->states[0].currentWeight = node->states[0].targetWeight * 0.001f;
            node->states[0].weightBlendTimeRemaining -= currentTime;
        }
        else
        {
            node->states[0].currentWeight            = node->states[0].targetWeight;
            node->states[0].weightBlendTimeRemaining = 0.0f;
        }

        {
            unsigned short childCount = tree->sourceTree->entries[nodeIndex].childCount;
            unsigned short firstChild = tree->sourceTree->entries[nodeIndex].u.parent.firstChildIndex;
            unsigned short notifyName = syncFlag ? node->notifyType : 0;
            float          weight0;
            int            weight0IsZero, target0IsZero;

            XAnimSetClientTime( node, &tree->sourceTree->entries[nodeIndex], notifyName );

            weight0        = node->states[0].currentWeight;
            weight0IsZero  = (weight0 == 0.0) | __UNORDERED__(weight0, 0.0);
            target0IsZero  = (node->states[0].targetWeight == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0);

            if ( childCount )
            {
                if ( weight0IsZero && target0IsZero )
                {
                    int anyChildAlive = 0;
                    int i;

                    for ( i = 0; i < childCount; ++i )
                    {
                        if ( XAnimUpdateInfoNoWeightClient( i + firstChild ) )
                            anyChildAlive = 1;
                    }
                    if ( !anyChildAlive
                      && ( (node->states[1].currentWeight == 0.0) | __UNORDERED__(node->states[1].currentWeight, 0.0) )
                      && ( (node->states[1].targetWeight == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) ) )
                    {
                        XAnimFreeInfo( handle );
                        tree->poolNodeHandles[nodeIndex] = 0;
                    }
                }
                else
                {
                    int i;
                    for ( i = 0; i < childCount; ++i )
                        XAnimUpdateClientInfoSyncInternal( i + firstChild, syncFlag );
                }
            }
            else if ( weight0IsZero && target0IsZero
                   && ( (node->states[1].currentWeight == 0.0) | __UNORDERED__(node->states[1].currentWeight, 0.0) )
                   && ( (node->states[1].targetWeight == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) ) )
            {
                XAnimFreeInfo( handle );
                tree->poolNodeHandles[nodeIndex] = childCount;
            }
        }
    }
}

/* ---- XAnimUpdateClientInfoInternal  0x00485DA0 ----  VERIFIED */
void __cdecl XAnimUpdateClientInfoInternal( int nodeIndex, float dtime, int notify )
{
    XAnimTree     *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];

    if ( !handle )
        return;

    {
        XAnimInfo  *node = &xanim_pool[handle];
        XAnimEntry *entry;
        unsigned short childCount;
        float      currentTime = *(float *)&xanim_currentTime;
        int        weight0IsZero, target0IsZero;

        if ( (node->states[0].targetWeight == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0) )
            notify = (notify & ~0xFF);

        if ( currentTime + 0.001 <= node->states[0].weightBlendTimeRemaining )
        {
            float newWeight = (node->states[0].targetWeight - node->states[0].currentWeight)
                             / node->states[0].weightBlendTimeRemaining * currentTime
                             + node->states[0].currentWeight;
            node->states[0].currentWeight = newWeight;
            if ( (newWeight < 0.0000010000001) | __UNORDERED__(newWeight, 0.0000010000001) )
                node->states[0].currentWeight = node->states[0].targetWeight * 0.001f;
            node->states[0].weightBlendTimeRemaining -= currentTime;
        }
        else
        {
            node->states[0].currentWeight            = node->states[0].targetWeight;
            node->states[0].weightBlendTimeRemaining = 0.0f;
        }

        entry      = &tree->sourceTree->entries[nodeIndex];
        childCount = entry->childCount;
        weight0IsZero = (node->states[0].currentWeight == 0.0) | __UNORDERED__(node->states[0].currentWeight, 0.0);
        target0IsZero = (node->states[0].targetWeight  == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0);

        if ( !childCount )
        {
            if ( weight0IsZero && target0IsZero )
            {
                if ( ( (node->states[1].currentWeight == 0.0) | __UNORDERED__(node->states[1].currentWeight, 0.0) )
                  && ( (node->states[1].targetWeight == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) ) )
                {
                    XAnimFreeInfo( handle );
                    tree->poolNodeHandles[nodeIndex] = childCount;
                }
            }
            else
            {
                XAnimParts    *parts = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
                unsigned short cycle = node->states[0].cycleCount;
                float          step  = parts->frequency * node->states[0].rateScale * dtime;
                double         newTime = (double)step + node->states[0].time;

                if ( newTime >= 1.0 )
                {
                    if ( parts->looped )
                    {
                        do
                        {
                            newTime = newTime - 1.0;
                            ++cycle;
                        }
                        while ( newTime >= 1.0 );
                    }
                    else
                    {
                        newTime = 1.0;
                    }
                }

                xanim_evalCurrentTime   = (float)newTime;
                xanim_evalStartTime   = node->states[0].time;
                dword_A9CC70 = step;
                xanim_evalStartFrame  = node->states[0].cycleCount;
                xanim_evalCurrentFrame  = cycle;
                XAnimSetClientTime( node, entry, (unsigned char)notify ? node->notifyType : 0 );
            }
            return;
        }

        if ( weight0IsZero && target0IsZero )
        {
            int anyChildAlive = 0;
            unsigned short firstChild = entry->u.parent.firstChildIndex;
            int i;

            for ( i = 0; i < childCount; ++i )
            {
                if ( XAnimUpdateInfoNoWeightClient( i + firstChild ) )
                    anyChildAlive = 1;
            }
            if ( !anyChildAlive
              && ( (node->states[1].currentWeight == 0.0) | __UNORDERED__(node->states[1].currentWeight, 0.0) )
              && ( (node->states[1].targetWeight == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) ) )
            {
                XAnimFreeInfo( handle );
                tree->poolNodeHandles[nodeIndex] = 0;
            }
        }
        else if ( entry->u.parent.flags & 3 )
        {
            unsigned short cycle = node->states[0].cycleCount;
            float          step  = XAnimGetAverageRateFrequency( nodeIndex ) * node->states[0].rateScale * dtime;
            double         newTime = (double)step + node->states[0].time;
            unsigned short firstChild;
            int i;

            if ( newTime >= 1.0 )
            {
                if ( entry->u.parent.flags & 2 )
                {
                    newTime = 1.0;
                }
                else
                {
                    do
                    {
                        newTime = newTime - 1.0;
                        ++cycle;
                    }
                    while ( newTime >= 1.0 );
                }
            }

            xanim_evalCurrentTime   = (float)newTime;
            dword_A9CC70 = step;
            xanim_evalStartTime   = node->states[0].time;
            xanim_evalStartFrame  = node->states[0].cycleCount;
            xanim_evalCurrentFrame  = cycle;
            XAnimSetClientTime( node, entry, (unsigned char)notify ? node->notifyType : 0 );

            xanim_evalCurrentTime   = node->states[0].time;
            xanim_evalStartTime   = node->states[0].oldTime;
            xanim_evalStartFrame  = node->states[0].oldCycleCount;
            xanim_evalCurrentFrame  = node->states[0].cycleCount;

            firstChild = entry->u.parent.firstChildIndex;
            for ( i = 0; i < childCount; ++i )
                XAnimUpdateClientInfoSyncInternal( i + firstChild, notify );
        }
        else
        {
            float step = dtime * node->states[0].rateScale;
            unsigned short firstChild = entry->u.parent.firstChildIndex;
            int i;

            for ( i = 0; i < childCount; ++i )
                XAnimUpdateClientInfoInternal( i + firstChild, step, notify );
        }
    }
}

/* ---- XAnimUpdateOldServerTime  0x00486150 ----  VERIFIED */
__int16 __cdecl XAnimUpdateOldServerTime( int nodeIndex )
{
    XAnimTree      *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short  handle = tree->poolNodeHandles[nodeIndex];
    __int16         result = handle;

    if ( handle )
    {
        XAnimInfo *node = &xanim_pool[handle];

        node->states[0].time                    = node->states[1].oldTime;
        node->states[0].cycleCount               = node->states[1].oldCycleCount;
        node->states[0].targetWeight             = node->states[1].targetWeight;
        node->states[0].rateScale                = node->states[1].rateScale;
        node->states[0].weightBlendTimeRemaining = node->states[1].weightBlendTimeRemaining;
        node->states[0].currentWeight            = node->states[1].currentWeight;

        {
            unsigned short childCount = tree->sourceTree->entries[nodeIndex].childCount;
            unsigned short firstChild = tree->sourceTree->entries[nodeIndex].u.parent.firstChildIndex;
            int i;

            for ( i = 0; i < childCount; ++i )
                result = XAnimUpdateOldServerTime( i + firstChild );
        }
    }
    return result;
}

/* ---- XAnimUpdateOldServerTimeNoWeight  0x004861D0 ----  VERIFIED */
char __cdecl XAnimUpdateOldServerTimeNoWeight( int nodeIndex )
{
    XAnimTree     *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];
    XAnimInfo     *node;
    float          settledWeight;
    unsigned short childCount;
    unsigned short firstChild;
    int i;
    int anyChildAlive;

    if ( !handle )
        return 0;

    node = &xanim_pool[handle];
    node->states[1].weightBlendTimeRemaining = 0.0f;
    settledWeight = node->states[1].targetWeight;
    node->states[1].currentWeight = node->states[1].targetWeight;

    childCount = tree->sourceTree->entries[nodeIndex].childCount;
    firstChild = tree->sourceTree->entries[nodeIndex].u.parent.firstChildIndex;

    if ( childCount )
    {
        anyChildAlive = 0;
        for ( i = 0; i < childCount; ++i )
        {
            if ( XAnimUpdateOldServerTimeNoWeight( i + firstChild ) )
                anyChildAlive = 1;
        }
        if ( anyChildAlive )
            return 1;
        settledWeight = node->states[1].currentWeight;
    }

    if ( ( (settledWeight == 0.0) | __UNORDERED__(settledWeight, 0.0) )
      && ( (node->states[1].targetWeight == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) )
      && ( (node->states[0].currentWeight == 0.0) | __UNORDERED__(node->states[0].currentWeight, 0.0) )
      && ( (node->states[0].targetWeight == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0) ) )
    {
        XAnimFreeInfo( handle );
        tree->poolNodeHandles[nodeIndex] = 0;
        return 0;
    }
    return 1;
}

/* ---- XAnimInitServerTime_m  0x004862D0 ----  VERIFIED */
void __cdecl XAnimInitServerTime_m( int nodeIndex )
{
    XAnimTree     *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];

    if ( handle )
    {
        float      currentTime = *(float *)&xanim_currentTime;
        double     now001      = currentTime + 0.001;
        XAnimInfo *node        = &xanim_pool[handle];
        int        blendActive;

        node->states[1].oldTime      = node->states[1].time;
        blendActive = ( now001 <= (double)node->states[1].weightBlendTimeRemaining );
        node->states[1].oldCycleCount = node->states[1].cycleCount;

        if ( blendActive )
        {
            float newWeight = (node->states[1].targetWeight - node->states[1].currentWeight)
                             / node->states[1].weightBlendTimeRemaining * currentTime
                             + node->states[1].currentWeight;
            node->states[1].currentWeight = newWeight;
            if ( (newWeight < 0.0000010000001) | __UNORDERED__(newWeight, 0.0000010000001) )
                node->states[1].currentWeight = node->states[1].targetWeight * 0.001f;
            node->states[1].weightBlendTimeRemaining -= currentTime;
        }
        else
        {
            node->states[1].currentWeight            = node->states[1].targetWeight;
            node->states[1].weightBlendTimeRemaining = 0.0f;
        }

        {
            unsigned short childCount = tree->sourceTree->entries[nodeIndex].childCount;
            unsigned short firstChild = tree->sourceTree->entries[nodeIndex].u.parent.firstChildIndex;
            float          weight1    = node->states[1].currentWeight;
            int            weight1IsZero = (weight1 == 0.0) | __UNORDERED__(weight1, 0.0);
            int            target1IsZero = (node->states[1].targetWeight == 0.0)
                                          | __UNORDERED__(node->states[1].targetWeight, 0.0);

            if ( childCount )
            {
                if ( weight1IsZero && target1IsZero )
                {
                    int anyChildAlive = 0;
                    int i;

                    for ( i = 0; i < childCount; ++i )
                    {
                        if ( XAnimUpdateOldServerTimeNoWeight( i + firstChild ) )
                            anyChildAlive = 1;
                    }
                    if ( !anyChildAlive
                      && ( (node->states[0].currentWeight == 0.0) | __UNORDERED__(node->states[0].currentWeight, 0.0) )
                      && ( (node->states[0].targetWeight == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0) ) )
                    {
                        XAnimFreeInfo( handle );
                        tree->poolNodeHandles[nodeIndex] = 0;
                    }
                }
                else
                {
                    int i;
                    for ( i = 0; i < childCount; ++i )
                        XAnimInitServerTime_m( i + firstChild );
                }
            }
            else if ( weight1IsZero && target1IsZero
                   && ( (node->states[0].currentWeight == 0.0) | __UNORDERED__(node->states[0].currentWeight, 0.0) )
                   && ( (node->states[0].targetWeight == 0.0) | __UNORDERED__(node->states[0].targetWeight, 0.0) ) )
            {
                XAnimFreeInfo( handle );
                tree->poolNodeHandles[nodeIndex] = 0;
            }
        }
    }
}

/* ---- XAnimClearServerNotify  0x004864A0 ----  VERIFIED */
void __cdecl XAnimClearServerNotify( XAnimInfo *node )
{
    unsigned short handle = node->notifyName;

    if ( handle ) {
        const char *text = SL_ConvertToString( handle );

        SL_RemoveRefToStringOfLen( handle, (unsigned int)strlen( text ) + 1 );
        node->notifyName = 0;
    }
    node->notifyIndex = -1;
}
#if 0
// XAnimClearServerNotify: clears one node's pending server notify and releases its name string.
void __cdecl XAnimClearServerNotify(int a1)
{
  unsigned __int16 v1;
  int v2;

  v1 = *(_WORD *)(a1 + 4);
  if ( v1 )
  {
    v2 = GetRefString_var + 8 * v1;
    if ( *(_WORD *)v2 )
    {
      --*(_WORD *)v2;
      *(_WORD *)(a1 + 4) = 0;
      *(_WORD *)(a1 + 2) = -1;
      return;
    }
    SL_FreeString(v1, (char *)(v2 + 4), strlen((const char *)(v2 + 4)) + 1);
    *(_WORD *)(a1 + 4) = 0;
  }
  *(_WORD *)(a1 + 2) = -1;
}
#endif

/* ---- NotifyServerNotetrack  0x00486510 ----  VERIFIED */
void __cdecl NotifyServerNotetrack( unsigned __int16 notifyName, unsigned __int16 entId,
                         unsigned __int16 notifyId )
{
    Scr_AddConstString( notifyName );
    Scr_NotifyId( entId, notifyId, 1 );
}
#if 0
// NotifyServerNotetrack: fires a GSC notify for a notetrack on the entity owning the tree.  SERVER PATH.
int *__cdecl NotifyServerNotetrack(unsigned __int16 a1, unsigned __int16 a2, unsigned __int16 a3)
{
  _WORD *v3;
  int v4;

  IncInParam();
  v3 = (_WORD *)scrVmPub_top;
  v4 = GetRefString_var;
  *(_DWORD *)(scrVmPub_top + 4) = 1;
  *v3 = a1;
  ++*(_WORD *)(v4 + 8 * a1);
  return Scr_NotifyId(a2, a3, 1);
}
#endif

/* ---- XAnimGetServerNotifyFracSyncTotal  0x00486550 ----  VERIFIED */
double __cdecl XAnimGetServerNotifyFracSyncTotal( XAnimInfo *node, XAnimEntry *entry )
{
    XAnimTree     *tree = (XAnimTree *)xanim_currentTree;
    double         frac = XAnimGetNotifyFracServer( node, entry );
    unsigned short childCount = entry->childCount;

    if ( childCount )
    {
        unsigned short firstChild = entry->u.parent.firstChildIndex;
        int i;

        for ( i = 0; i < childCount; ++i )
        {
            unsigned short childHandle = tree->poolNodeHandles[firstChild + i];

            if ( childHandle )
            {
                XAnimInfo *childNode = &xanim_pool[childHandle];

                if ( !( (childNode->states[1].currentWeight == 0.0) | __UNORDERED__(childNode->states[1].currentWeight, 0.0) )
                  && !( (childNode->states[1].targetWeight  == 0.0) | __UNORDERED__(childNode->states[1].targetWeight, 0.0) ) )
                {
                    double childFrac = XAnimGetServerNotifyFracSyncTotal( childNode, &tree->sourceTree->entries[firstChild + i] );

                    if ( (childFrac < frac) | __UNORDERED__(childFrac, frac) )
                        frac = childFrac;
                }
            }
        }
    }
    return frac;
}

/* ---- XAnimFindServerNoteTrack  0x004865F0 ----  VERIFIED */
double __cdecl XAnimFindServerNoteTrack( int nodeIndex, float dtime )
{
    XAnimTree     *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];
    XAnimInfo     *node;
    XAnimEntry    *entry;
    unsigned short childCount;

    if ( !handle )
        return 1.0;
    node = &xanim_pool[handle];

    if ( (node->states[1].currentWeight == 0.0) | __UNORDERED__(node->states[1].currentWeight, 0.0)
      || (node->states[1].targetWeight  == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) )
        return 1.0;

    entry      = &tree->sourceTree->entries[nodeIndex];
    childCount = entry->childCount;

    if ( !childCount )
    {
        XAnimParts *parts = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
        float       step  = parts->frequency * node->states[1].rateScale * dtime;

        if ( !( (step == 0.0) | __UNORDERED__(step, 0.0) ) )
        {
            double         newTime  = (double)step + node->states[1].oldTime;
            unsigned short cycle    = node->states[1].oldCycleCount;
            unsigned short newCycle = cycle;

            if ( parts->looped )
            {
                for ( ; newTime >= 1.0; ++newCycle )
                    newTime = newTime - 1.0;
            }
            else if ( newTime >= 1.0 )
            {
                newTime = 1.0;
            }

            {
                unsigned short baseCycle = node->states[1].cycleCount;

                if ( node->states[1].time - newTime <= (double)( (int)newCycle - (int)baseCycle ) )
                {
                    xanim_evalCurrentTime   = (float)newTime;
                    xanim_evalStartFrame  = cycle;
                    xanim_evalWindowTime = node->states[1].time;
                    dword_A9CC70 = step;
                    xanim_evalStartTime   = node->states[1].oldTime;
                    xanim_evalWindowFrame  = baseCycle;
                    return XAnimGetNotifyFracServer( node, entry );
                }
            }
        }
        return 1.0;
    }

    if ( entry->u.parent.flags & 3 )
    {
        double         newTime;
        unsigned short cycle, newCycle, baseCycle;
        float          step = XAnimGetAverageRateFrequency( nodeIndex ) * node->states[1].rateScale * dtime;

        if ( (step == 0.0) | __UNORDERED__(step, 0.0) )
            return 1.0;

        newTime  = (double)step + node->states[1].oldTime;
        cycle    = node->states[1].oldCycleCount;
        newCycle = cycle;

        if ( entry->u.parent.flags & 2 )
        {
            if ( newTime >= 1.0 )
                newTime = 1.0;
        }
        else
        {
            for ( ; newTime >= 1.0; ++newCycle )
                newTime = newTime - 1.0;
        }

        baseCycle = node->states[1].cycleCount;
        if ( node->states[1].time - newTime > (double)( (int)newCycle - (int)baseCycle ) )
            return 1.0;

        xanim_evalCurrentTime   = (float)newTime;
        dword_A9CC70 = step;
        xanim_evalStartTime   = node->states[1].oldTime;
        xanim_evalStartFrame  = cycle;
        xanim_evalWindowTime = node->states[1].time;
        xanim_evalWindowFrame  = baseCycle;
        return XAnimGetServerNotifyFracSyncTotal( node, entry );
    }
    else
    {
        float step = dtime * node->states[1].rateScale;
        unsigned short firstChild;
        double minFrac;
        int i;

        if ( (step == 0.0) | __UNORDERED__(step, 0.0) )
            return 1.0;

        firstChild = entry->u.parent.firstChildIndex;
        minFrac    = 1.0;
        for ( i = 0; i < childCount; ++i )
        {
            double childFrac = XAnimFindServerNoteTrack( i + firstChild, step );
            if ( (childFrac < minFrac) | __UNORDERED__(childFrac, minFrac) )
                minFrac = childFrac;
        }
        return minFrac;
    }
}

/* ---- XAnimProcessServerNotify  0x00486870 ----  VERIFIED */
void __cdecl XAnimProcessServerNotify( XAnimInfo *node, XAnimEntry *entry )
{
    XAnimTree       *tree;
    float            endTime;
    XAnimParts      *parts;
    XAnimNotifyInfo *cursor;
    int              stepWrapped;
    float            finalTime;
    int              haveFinalTime = 0;

    if ( !xanim_notifyEntId || !node->notifyName )
        return;

    if ( (xanim_evalStartTime == 1.0) | __UNORDERED__(xanim_evalStartTime, 1.0) )
    {
    fireShortcut:
        Scr_AddConstString( xanim_endNotifyStringHandle );
        Scr_NotifyId( xanim_notifyEntId, node->notifyName, 1 );
        return;
    }

    if ( entry->childCount )
    {
        if ( !node->notifyChildIndex )
        {
            if ( (xanim_evalCurrentTime < (double)xanim_evalStartTime) | __UNORDERED__(xanim_evalCurrentTime, xanim_evalStartTime)
              || (xanim_evalCurrentTime == 1.0) | __UNORDERED__(xanim_evalCurrentTime, 1.0) )
                goto fireShortcut;
            return;
        }
        tree  = (XAnimTree *)xanim_currentTree;
        entry = &tree->sourceTree->entries[node->notifyChildIndex];
    }

    endTime = xanim_evalCurrentTime;
    parts   = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
    cursor  = parts->notify + (__int16)node->notifyIndex;

    stepWrapped = (endTime < (double)xanim_evalStartTime) | __UNORDERED__(endTime, xanim_evalStartTime);

    if ( stepWrapped )
    {
        if ( endTime <= cursor->frameFrac )
        {
            if ( xanim_evalStartTime > (double)cursor->frameFrac )
                return;

            for ( ;; )
            {
                NotifyServerNotetrack( cursor->name, xanim_notifyEntId, node->notifyName );
                ++cursor;
                if ( !cursor->name )
                    break;
            }

            cursor = parts->notify;
            if ( endTime > (double)cursor->frameFrac )
            {
                int lt, eq;
                do
                {
                    NotifyServerNotetrack( cursor->name, xanim_notifyEntId, node->notifyName );
                    lt = endTime < (double)cursor[1].frameFrac;
                    eq = ( endTime == cursor[1].frameFrac );
                    ++cursor;
                }
                while ( !lt && !eq );
            }
            finalTime = endTime;
            haveFinalTime = 1;
        }
        else
        {
            for ( ;; )
            {
                NotifyServerNotetrack( cursor->name, xanim_notifyEntId, node->notifyName );
                ++cursor;
                if ( !( cursor->name && endTime > (double)cursor->frameFrac ) )
                    break;
            }
            finalTime = endTime;
            haveFinalTime = 1;
        }
    }
    else if ( (endTime == 1.0) | __UNORDERED__(endTime, 1.0) )
    {
        if ( xanim_evalStartTime <= (double)cursor->frameFrac )
        {
            for ( ;; )
            {
                NotifyServerNotetrack( cursor->name, xanim_notifyEntId, node->notifyName );
                ++cursor;
                if ( !cursor->name )
                    break;
            }
        }
        return;
    }
    else if ( endTime > (double)cursor->frameFrac && xanim_evalStartTime <= (double)cursor->frameFrac )
    {
        for ( ;; )
        {
            NotifyServerNotetrack( cursor->name, xanim_notifyEntId, node->notifyName );
            ++cursor;
            if ( !( cursor->name && endTime > (double)cursor->frameFrac ) )
                break;
        }
        finalTime = endTime;
        haveFinalTime = 1;
    }

    if ( haveFinalTime )
        node->notifyIndex = XAnimGetNextNotifyTime( entry, (int)node, finalTime );
}

/* ---- XAnimProcessServerNotify_r  0x00486B30 ----  VERIFIED */
void __cdecl XAnimProcessServerNotify_r( XAnimInfo *node, XAnimEntry *entry )
{
    unsigned short childCount = entry->childCount;

    XAnimProcessServerNotify( node, entry );
    if ( childCount )
    {
        unsigned short firstChild = entry->u.parent.firstChildIndex;
        int i;

        for ( i = 0; i < childCount; ++i )
        {
            XAnimTree     *tree        = (XAnimTree *)xanim_currentTree;
            unsigned short childHandle = tree->poolNodeHandles[firstChild + i];

            if ( childHandle )
            {
                XAnimInfo *childNode = &xanim_pool[childHandle];

                if ( !( (childNode->states[1].currentWeight == 0.0) | __UNORDERED__(childNode->states[1].currentWeight, 0.0) )
                  && !( (childNode->states[1].targetWeight  == 0.0) | __UNORDERED__(childNode->states[1].targetWeight, 0.0) ) )
                {
                    XAnimProcessServerNotify_r( childNode, &tree->sourceTree->entries[firstChild + i] );
                }
            }
        }
    }
}

/* ---- XAnimStampSecondaryWindowStart_m  0x00486BB0 ----  VERIFIED */
void __cdecl XAnimStampSecondaryWindowStart_m( int nodeIndex )
{
    XAnimTree      *tree = (XAnimTree *)xanim_currentTree;
    unsigned short  handle = tree->poolNodeHandles[nodeIndex];
    XAnimEntry     *entry;
    int             child;

    if ( !handle ) {
        return;
    }

    xanim_pool[handle].states[1].time       = *(const float *)&xanim_evalCurrentTime;
    xanim_pool[handle].states[1].cycleCount = xanim_evalCurrentFrame;

    entry = &tree->sourceTree->entries[nodeIndex];
    for ( child = 0; child < (int)entry->childCount; ++child ) {
        XAnimStampSecondaryWindowStart_m( entry->u.parent.firstChildIndex + child );
    }
}
#if 0
// XAnimStampSecondaryWindowStart_m (coduo XAnimStampSecondaryWindowStart): records the start of the secondary timing window for a node.
__int16 __cdecl XAnimStampSecondaryWindowStart_m(int a1)
{
  int *v1;
  int v2;
  int *v3;
  unsigned __int16 *v4;
  int v5;

  v1 = (int *)xanim_currentTree;
  LOWORD(v2) = *(_WORD *)(xanim_currentTree + 2 * a1 + 8);
  if ( (_WORD)v2 )
  {
    v3 = &dword_A7A648[17 * (unsigned __int16)v2];
    *v3 = LODWORD(xanim_evalCurrentTime);
    *((_WORD *)v3 + 4) = xanim_evalCurrentFrame;
    v2 = *v1;
    v4 = (unsigned __int16 *)(*v1 + 8 * a1 + 8);
    v5 = 0;
    if ( *v4 )
    {
      do
      {
        LOWORD(v2) = XAnimStampSecondaryWindowStart_m(v5 + v4[3]);
        ++v5;
      }
      while ( v5 < *v4 );
    }
  }
  return v2;
}
#endif

/* ---- XAnimUpdateServerInfoInternal  0x00486C10 ----  VERIFIED */
void __cdecl XAnimUpdateServerInfoInternal( int nodeIndex, float dtime, int notify )
{
    XAnimTree     *tree   = (XAnimTree *)xanim_currentTree;
    unsigned short handle = tree->poolNodeHandles[nodeIndex];

    if ( !handle )
        return;

    {
        XAnimInfo  *node = &xanim_pool[handle];
        XAnimEntry *entry;
        unsigned short childCount;

        if ( (node->states[1].currentWeight == 0.0) | __UNORDERED__(node->states[1].currentWeight, 0.0) )
            return;

        if ( (node->states[1].targetWeight == 0.0) | __UNORDERED__(node->states[1].targetWeight, 0.0) )
            notify = (notify & ~0xFF);

        entry      = &tree->sourceTree->entries[nodeIndex];
        childCount = entry->childCount;

        if ( !childCount )
        {
            XAnimParts    *parts = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
            unsigned short cycle = node->states[1].oldCycleCount;
            double         newTime = (double)parts->frequency * node->states[1].rateScale * dtime
                                    + node->states[1].oldTime;
            float          savedTime = (float)newTime;

            if ( parts->looped )
            {
                if ( (newTime < 0.0) | __UNORDERED__(newTime, 0.0) )
                {
                    do
                    {
                        newTime = newTime + 1.0;
                        --cycle;
                    }
                    while ( (newTime < 0.0) | __UNORDERED__(newTime, 0.0) );
                    savedTime = (float)newTime;
                }
                if ( !(newTime < 1.0) )
                {
                    do
                    {
                        newTime = newTime - 1.0;
                        ++cycle;
                    }
                    while ( newTime >= 1.0 );
                    savedTime = (float)newTime;
                }
            }
            else
            {
                if ( newTime < 1.0 )
                {
                    if ( newTime <= 0.0 )
                        return;
                }
                else
                {
                    newTime = 1.0;
                    savedTime = (float)newTime;
                }
            }

            {
                unsigned short baseCycle = node->states[1].cycleCount;

                if ( node->states[1].time - newTime <= (double)(cycle - baseCycle) )
                {
                    if ( (unsigned char)notify )
                    {
                        float oldTime = node->states[1].time;
                        xanim_evalCurrentTime  = (float)newTime;
                        xanim_evalStartTime  = oldTime;
                        xanim_evalStartFrame = baseCycle;
                        XAnimProcessServerNotify( node, entry );
                        newTime = savedTime;
                    }
                    node->states[1].cycleCount = cycle;
                    node->states[1].time       = (float)newTime;
                }
            }
            return;
        }

        if ( entry->u.parent.flags & 3 )
        {
            unsigned short cycle = node->states[1].oldCycleCount;
            double newTime = (double)XAnimGetAverageRateFrequency( nodeIndex ) * node->states[1].rateScale * dtime
                            + node->states[1].oldTime;

            if ( entry->u.parent.flags & 2 )
            {
                if ( newTime < 1.0 )
                {
                    if ( newTime <= 0.0 )
                        return;
                }
                else
                {
                    newTime = 1.0;
                }
            }
            else
            {
                for ( ; (newTime < 0.0) | __UNORDERED__(newTime, 0.0); --cycle )
                    newTime = newTime + 1.0;
                for ( ; newTime >= 1.0; ++cycle )
                    newTime = newTime - 1.0;
            }

            {
                unsigned short baseCycle = node->states[1].cycleCount;

                if ( node->states[1].time - newTime <= (double)(cycle - baseCycle) )
                {
                    xanim_evalCurrentTime = (float)newTime;
                    if ( (unsigned char)notify )
                    {
                        xanim_evalStartTime  = node->states[1].time;
                        xanim_evalStartFrame = baseCycle;
                        XAnimProcessServerNotify_r( node, entry );
                    }
                    xanim_evalCurrentFrame = cycle;
                    XAnimStampSecondaryWindowStart_m( nodeIndex );
                }
            }
        }
        else
        {
            float step = dtime * node->states[1].rateScale;
            unsigned short firstChild = entry->u.parent.firstChildIndex;
            int i;

            for ( i = 0; i < childCount; ++i )
                XAnimUpdateServerInfoInternal( i + firstChild, step, notify );
        }
    }
}

#define XANIM_EVAL_SKIP(i)  ( ( ((const unsigned char *)xanim_evalSkipBits)[(i) >> 3] \
                                & ( 1 << ( (i) & 7 ) ) ) != 0 )

/* ---- XAnimCalc  0x00486EC0 ----  VERIFIED */
void __cdecl XAnimCalc( int nodeIndex, float weight, DObjAnimMat *part,
                         int clear, int normalize )
{
    XAnimTree   *tree   = (XAnimTree *)xanim_currentTree;
    XAnim       *source = tree->sourceTree;
    XAnimEntry  *entry;
    DObjAnimMat  scratch[128];
    DObjAnimMat *blendParts;
    int          firstActiveChild;
    int          secondActiveChild;
    float        firstWeight;
    float        secondWeight;
    int          firstChildIndex;
    int          child;
    int          i;

    firstActiveChild  = -1;
    secondActiveChild = -1;
    firstWeight       = 0.0f;
    secondWeight      = 0.0f;
    firstChildIndex   = 0;

    for ( ;; ) {
        entry = &source->entries[nodeIndex];

        if ( entry->childCount == 0 ) {
            unsigned char  *remapTable;
            unsigned short *remapHandles;
            unsigned char  *remapGeneration;
            unsigned short  handle;

            if ( clear ) {
                XAnimClearData( (_DWORD *)part );
            }

            remapTable      = ((DObj *)xanim_evalDObj)->partRemapTable;
            remapHandles    = (unsigned short *)remapTable;
            remapGeneration = remapTable + 2 * source->nodeCount;

            if ( remapHandles[nodeIndex] == 0 ) {
                remapGeneration[nodeIndex + 1] = remapGeneration[0];
                remapHandles[nodeIndex] =
                    (unsigned short)XAnimSetModel( (int)entry, xanim_evalChildRefs,
                                                xanim_evalChildCount );
            } else if ( remapGeneration[nodeIndex + 1] != remapGeneration[0] ) {
                XAnimParts   *parts;
                unsigned int  remapSize;

                parts = (XAnimParts *)( (void **)entry->u.leafAsset )[1];
                remapSize = (unsigned int)
                    ( (int)*(const short *)parts->partNameHandles
                      + XANIM_REMAP_PREFIX );

                remapGeneration[nodeIndex + 1] = remapGeneration[0];
                SL_RemoveRefToStringOfLen( remapHandles[nodeIndex], remapSize );
                remapHandles[nodeIndex] =
                    (unsigned short)XAnimSetModel( (int)entry, xanim_evalChildRefs,
                                                xanim_evalChildCount );
            }

            handle = tree->poolNodeHandles[nodeIndex];
            XAnimCalcData( (_DWORD *)SL_ConvertToString( remapHandles[nodeIndex] ),
                        (int)part,
                        (int)entry->u.leafAsset,
                        weight,
                        XANIM_LANE( handle ).time );
            return;
        }

        firstActiveChild = -1;
        firstWeight      = 0.0f;
        firstChildIndex  = entry->u.parent.firstChildIndex;

        for ( child = 0; child < (int)entry->childCount; ++child ) {
            unsigned short handle = tree->poolNodeHandles[firstChildIndex + child];

            if ( handle ) {
                float w = XANIM_LANE( handle ).currentWeight;

                if ( w != 0.0f ) {
                    firstActiveChild = child;
                    firstWeight      = w;
                    break;
                }
            }
        }

        if ( firstActiveChild < 0 ) {
            if ( clear ) {
                XAnimClearData( (_DWORD *)part );
            }
            return;
        }

        secondActiveChild = -1;
        secondWeight      = 0.0f;

        for ( child = firstActiveChild + 1; child < (int)entry->childCount;
              ++child ) {
            unsigned short handle = tree->poolNodeHandles[firstChildIndex + child];

            if ( handle ) {
                float w = XANIM_LANE( handle ).currentWeight;

                if ( w != 0.0f ) {
                    secondActiveChild = child;
                    secondWeight      = w;
                    break;
                }
            }
        }

        if ( secondActiveChild < 0 ) {
            nodeIndex = firstChildIndex + firstActiveChild;
            continue;
        }
        break;
    }

    blendParts = clear ? part : scratch;

    XAnimCalc( firstChildIndex + firstActiveChild,  firstWeight,
                blendParts, 1, 1 );
    XAnimCalc( firstChildIndex + secondActiveChild, secondWeight,
                blendParts, 0, 1 );

    for ( child = secondActiveChild + 1; child < (int)entry->childCount;
          ++child ) {
        int            childIndex = firstChildIndex + child;
        unsigned short handle     = tree->poolNodeHandles[childIndex];

        if ( handle ) {
            float w = XANIM_LANE( handle ).currentWeight;

            if ( w != 0.0f ) {
                XAnimCalc( childIndex, w, blendParts, 0, 1 );
            }
        }
    }

    if ( !normalize ) {
        for ( i = 0; i < xanim_evalPartCount; ++i ) {
            if ( !XANIM_EVAL_SKIP( i ) && part[i].weight != 0.0f ) {
                float scale = 1.0f / part[i].weight;

                part[i].quat[0]  *= scale;
                part[i].quat[1]  *= scale;
                part[i].quat[2]  *= scale;
                part[i].quat[3]  *= scale;
                part[i].trans[0] *= scale;
                part[i].trans[1] *= scale;
                part[i].trans[2] *= scale;
            }
        }
    } else if ( !clear ) {
        for ( i = 0; i < xanim_evalPartCount; ++i ) {
            float lengthSq;

            if ( XANIM_EVAL_SKIP( i ) ) {
                continue;
            }

            lengthSq = blendParts[i].quat[0] * blendParts[i].quat[0]
                     + blendParts[i].quat[1] * blendParts[i].quat[1]
                     + blendParts[i].quat[2] * blendParts[i].quat[2]
                     + blendParts[i].quat[3] * blendParts[i].quat[3];
            if ( lengthSq != 0.0f ) {
                float scale = weight / (float)sqrt( (double)lengthSq );

                part[i].quat[0] += scale * blendParts[i].quat[0];
                part[i].quat[1] += scale * blendParts[i].quat[1];
                part[i].quat[2] += scale * blendParts[i].quat[2];
                part[i].quat[3] += scale * blendParts[i].quat[3];
            }

            if ( blendParts[i].weight != 0.0f ) {
                float scale = weight / blendParts[i].weight;

                part[i].weight   += weight;
                part[i].trans[0] += scale * blendParts[i].trans[0];
                part[i].trans[1] += scale * blendParts[i].trans[1];
                part[i].trans[2] += scale * blendParts[i].trans[2];
            }
        }
    } else {
        for ( i = 0; i < xanim_evalPartCount; ++i ) {
            float lengthSq;

            if ( XANIM_EVAL_SKIP( i ) ) {
                continue;
            }

            lengthSq = part[i].quat[0] * part[i].quat[0]
                     + part[i].quat[1] * part[i].quat[1]
                     + part[i].quat[2] * part[i].quat[2]
                     + part[i].quat[3] * part[i].quat[3];
            if ( lengthSq != 0.0f ) {
                float scale = weight / (float)sqrt( (double)lengthSq );

                part[i].quat[0] *= scale;
                part[i].quat[1] *= scale;
                part[i].quat[2] *= scale;
                part[i].quat[3] *= scale;
            }

            if ( part[i].weight != 0.0f ) {
                float scale = weight / part[i].weight;

                part[i].weight    = weight;
                part[i].trans[0] *= scale;
                part[i].trans[1] *= scale;
                part[i].trans[2] *= scale;
            }
        }
    }
}
#if 0
// XAnimCalc: recursively weight-blends the whole tree into the DObjAnimMat array.  Sole callee of DObjCalcAnim.  SERVER PATH -- this produces the bone data traces read.
void __cdecl XAnimCalc(int a1, float a2, float *a3, char a4, char a5)
{
  int v5;
  int v6;
  int v7;
  int v8;
  int i;
  unsigned __int16 v10;
  int v11;
  double v12;
  int v13;
  int v14;
  _WORD *v15;
  int v16;
  double v17;
  int v18;
  int v19;
  bool v20; // zf
  _DWORD *v21;
  char *v22;
  char v23;
  int v24;
  unsigned __int16 v25;
  _DWORD *v26;
  float *v27;
  void *v28;
  int j;
  int v30;
  unsigned __int16 v31;
  double v32;
  int v33;
  int v34;
  int v35;
  double v36;
  float *v37;
  double v38;
  float *v39;
  double v40;
  float *v41;
  double v42;
  double v43;
  int v44;
  int v45;
  double v46;
  long double v47;
  double v48;
  double v49;
  long double v50;
  double v51;
  int v52;
  int v53;
  int v54; // [esp+0h] [ebp-20h] BYREF
  float v55;
  int v56;
  int v57;
  float v58;
  int v59;
  float *v60;

  v5 = xanim_currentTree;
  v6 = a1;
  v7 = *(unsigned __int16 *)(*(_DWORD *)xanim_currentTree + 8 * a1 + 8);
  v56 = *(_DWORD *)xanim_currentTree;
  v8 = v56 + 8 * a1 + 8;
  v59 = v7;
  if ( !v7 )
    goto LABEL_14;
  while ( 2 )
  {
    for ( i = 0; ; ++i )
    {
      v57 = i;
      if ( i >= v59 )
      {
        if ( a4 )
          XAnimClearData(a3);
        return;
      }
      v10 = *(_WORD *)(v5 + 2 * (i + *(unsigned __int16 *)(v8 + 6)) + 8);
      if ( v10 )
      {
        v11 = 68 * v10;
        v12 = *(float *)(v11 + 28 * xanim_activePoolSlot + 10987072);
        v55 = *(float *)(v11 + 28 * xanim_activePoolSlot + 10987072);
        if ( !((v12 == 0.0) | __UNORDERED__(v12, 0.0)) )
          break;
      }
    }
    v13 = i + 1;
    if ( i + 1 >= v59 )
    {
LABEL_13:
      v6 = i + *(unsigned __int16 *)(v8 + 6);
      v18 = *(unsigned __int16 *)(v56 + 8 * v6 + 8);
      v8 = v56 + 8 * v6 + 8;
      v59 = v18;
      if ( v18 )
        continue;
LABEL_14:
      if ( a4 )
        XAnimClearData(a3);
      v19 = *(_DWORD *)(xanim_evalDObj + 12);
      v20 = *(_WORD *)(v19 + 2 * v6) == 0;
      v21 = (_DWORD *)(xanim_evalDObj + 12);
      v22 = (char *)(v19 + 2 * *(_DWORD *)(*(_DWORD *)xanim_currentTree + 4));
      v23 = *v22;
      if ( v20 )
      {
        v21 = (_DWORD *)(xanim_evalDObj + 12);
        v24 = xanim_evalChildCount;
        v22[v6 + 1] = v23;
        v53 = v24;
        v52 = xanim_evalChildRefs;
      }
      else
      {
        if ( v22[v6 + 1] == v23 )
        {
LABEL_21:
          v25 = *(_WORD *)(*v21 + 2 * v6);
          if ( v25 )
            v26 = (_DWORD *)(GetRefString_var + 8 * v25 + 4);
          else
            v26 = 0;
          XAnimCalcData(
            v26,
            (int)a3,
            *(_DWORD *)(v8 + 4),
            a2,
            *(float *)&dword_A7A62C[17 * *(unsigned __int16 *)(xanim_currentTree + 2 * v6 + 8) + 7 * xanim_activePoolSlot]);
          return;
        }
        v22[v6 + 1] = v23;
        SL_RemoveRefToStringOfLen(
          *(_WORD *)(*v21 + 2 * v6),
          **(__int16 **)(*(_DWORD *)(*(_DWORD *)(v8 + 4) + 4) + 12) + 16);
        v53 = xanim_evalChildCount;
        v21 = (_DWORD *)(xanim_evalDObj + 12);
        v52 = xanim_evalChildRefs;
      }
      *(_WORD *)(*v21 + 2 * v6) = XAnimSetModel(v8, v52, v53);
      goto LABEL_21;
    }
    break;
  }
  v14 = 28 * xanim_activePoolSlot;
  v15 = (_WORD *)(v5 + 2 * (v13 + *(unsigned __int16 *)(v8 + 6)) + 8);
  while ( 1 )
  {
    if ( *v15 )
    {
      v16 = 68 * (unsigned __int16)*v15;
      v17 = *(float *)(v16 + v14 + 10987072);
      v58 = *(float *)(v16 + v14 + 10987072);
      if ( !((v17 == 0.0) | __UNORDERED__(v17, 0.0)) )
        break;
    }
    ++v13;
    ++v15;
    if ( v13 >= v59 )
    {
      v5 = xanim_currentTree;
      goto LABEL_13;
    }
  }
  v27 = a3;
  if ( a4 )
  {
    v60 = a3;
  }
  else
  {
    v28 = alloca(dword_A7A61C);
    i = v57;
    v60 = (float *)&v54;
  }
  XAnimCalc(i + *(unsigned __int16 *)(v8 + 6), v55, v60, 1, 1);
  XAnimCalc(v13 + *(unsigned __int16 *)(v8 + 6), v58, v60, 0, 1);
  for ( j = v13 + 1; j < v59; ++j )
  {
    v30 = *(unsigned __int16 *)(v8 + 6) + j;
    v31 = *(_WORD *)(xanim_currentTree + 2 * v30 + 8);
    if ( v31 )
    {
      v32 = *((float *)&unk_A7A640 + 17 * v31 + 7 * xanim_activePoolSlot);
      v58 = *((float *)&unk_A7A640 + 17 * v31 + 7 * xanim_activePoolSlot);
      if ( !((v32 == 0.0) | __UNORDERED__(v32, 0.0)) )
        XAnimCalc(v30, v58, v60, 0, 1);
    }
  }
  if ( a5 )
  {
    v44 = xanim_evalPartCount;
    v45 = 0;
    if ( a4 )
    {
      if ( xanim_evalPartCount > 0 )
      {
        do
        {
          if ( ((1 << (v45 & 7)) & *((char *)xanim_evalSkipBits + (v45 >> 3))) == 0 )
          {
            v46 = v27[3] * v27[3] + v27[1] * v27[1] + *v27 * *v27 + v27[2] * v27[2];
            if ( !((v46 == 0.0) | __UNORDERED__(v46, 0.0)) )
            {
              v47 = a2 / sqrt(v46);
              *v27 = v47 * *v27;
              v27[1] = v47 * v27[1];
              v27[2] = v47 * v27[2];
              v27[3] = v47 * v27[3];
            }
            if ( !((v27[4] == 0.0) | __UNORDERED__(v27[4], 0.0)) )
            {
              v48 = a2 / v27[4];
              v27[4] = a2;
              v27[5] = v48 * v27[5];
              v27[6] = v48 * v27[6];
              v27[7] = v48 * v27[7];
            }
          }
          ++v45;
          v27 += 8;
        }
        while ( v45 < v44 );
      }
    }
    else if ( xanim_evalPartCount > 0 )
    {
      do
      {
        if ( ((1 << (v45 & 7)) & *((char *)xanim_evalSkipBits + (v45 >> 3))) == 0 )
        {
          v49 = v60[3] * v60[3] + v60[1] * v60[1] + *v60 * *v60 + v60[2] * v60[2];
          if ( !((v49 == 0.0) | __UNORDERED__(v49, 0.0)) )
          {
            v50 = a2 / sqrt(v49);
            *v27 = v50 * *v60 + *v27;
            v27[1] = v50 * v60[1] + v27[1];
            v27[2] = v50 * v60[2] + v27[2];
            v27[3] = v50 * v60[3] + v27[3];
          }
          if ( !((v60[4] == 0.0) | __UNORDERED__(v60[4], 0.0)) )
          {
            v51 = a2 / v60[4];
            v27[4] = a2 + v27[4];
            v27[5] = v51 * v60[5] + v27[5];
            v27[6] = v51 * v60[6] + v27[6];
            v27[7] = v51 * v60[7] + v27[7];
          }
        }
        ++v45;
        v27 += 8;
        v60 += 8;
      }
      while ( v45 < v44 );
    }
  }
  else
  {
    v33 = xanim_evalPartCount;
    v34 = 0;
    if ( xanim_evalPartCount >= 4 )
    {
      v35 = 2;
      do
      {
        if ( ((1 << (v34 & 7)) & *((char *)xanim_evalSkipBits + (v34 >> 3))) == 0
          && !((v27[4] == 0.0) | __UNORDERED__(v27[4], 0.0)) )
        {
          v36 = 1.0 / v27[4];
          *v27 = v36 * *v27;
          v27[1] = v36 * v27[1];
          v27[2] = v36 * v27[2];
          v27[3] = v36 * v27[3];
          v27[5] = v36 * v27[5];
          v27[6] = v36 * v27[6];
          v27[7] = v36 * v27[7];
        }
        v37 = v27 + 8;
        if ( ((1 << ((v35 - 1) & 7)) & *((char *)xanim_evalSkipBits + ((v35 - 1) >> 3))) == 0
          && !((v37[4] == 0.0) | __UNORDERED__(v37[4], 0.0)) )
        {
          v38 = 1.0 / v37[4];
          *v37 = v38 * *v37;
          v37[1] = v38 * v37[1];
          v37[2] = v38 * v37[2];
          v37[3] = v38 * v37[3];
          v37[5] = v38 * v37[5];
          v37[6] = v38 * v37[6];
          v37[7] = v38 * v37[7];
        }
        v39 = v37 + 8;
        if ( ((1 << (v35 & 7)) & *((char *)xanim_evalSkipBits + (v35 >> 3))) == 0
          && !((v39[4] == 0.0) | __UNORDERED__(v39[4], 0.0)) )
        {
          v40 = 1.0 / v39[4];
          *v39 = v40 * *v39;
          v39[1] = v40 * v39[1];
          v39[2] = v40 * v39[2];
          v39[3] = v40 * v39[3];
          v39[5] = v40 * v39[5];
          v39[6] = v40 * v39[6];
          v39[7] = v40 * v39[7];
        }
        v41 = v39 + 8;
        if ( ((1 << ((v35 + 1) & 7)) & *((char *)xanim_evalSkipBits + ((v35 + 1) >> 3))) == 0
          && !((v41[4] == 0.0) | __UNORDERED__(v41[4], 0.0)) )
        {
          v42 = 1.0 / v41[4];
          *v41 = v42 * *v41;
          v41[1] = v42 * v41[1];
          v41[2] = v42 * v41[2];
          v41[3] = v42 * v41[3];
          v41[5] = v42 * v41[5];
          v41[6] = v42 * v41[6];
          v41[7] = v42 * v41[7];
        }
        v33 = xanim_evalPartCount;
        v35 += 4;
        v27 = v41 + 8;
        v34 += 4;
      }
      while ( v35 + 1 < xanim_evalPartCount );
    }
    for ( ; v34 < v33; v27 += 8 )
    {
      if ( ((1 << (v34 & 7)) & *((char *)xanim_evalSkipBits + (v34 >> 3))) == 0
        && !((v27[4] == 0.0) | __UNORDERED__(v27[4], 0.0)) )
      {
        v43 = 1.0 / v27[4];
        *v27 = v43 * *v27;
        v27[1] = v43 * v27[1];
        v27[2] = v43 * v27[2];
        v27[3] = v43 * v27[3];
        v27[5] = v43 * v27[5];
        v27[6] = v43 * v27[6];
        v27[7] = v43 * v27[7];
      }
      ++v34;
    }
  }
}
#endif

/* ---- XAnimDisplay  0x004875A0 ----  VERIFIED */
void __cdecl XAnimDisplay( XAnimTree *tree, int nodeIndex, int depth )
{
    unsigned short  handle;
    XAnimEntry     *entry;
    XAnimInfo      *info;
    XAnimState     *state;
    int             childCount;
    int             i;

    handle = tree->poolNodeHandles[nodeIndex];
    if ( handle == 0 ) {
        return;
    }

    entry      = &tree->sourceTree->entries[nodeIndex];
    info       = &xanim_pool[handle];
    state      = &info->states[xanim_activePoolSlot];
    childCount = entry->childCount;

    if ( state->currentWeight == 0.0f ) {
        return;
    }

    for ( i = depth; i > 0; --i ) {
        Com_Printf( " " );
    }

    if ( childCount != 0 ) {
        if ( xanim_activePoolSlot && info->notifyName != 0 ) {
            if ( entry->childCount != 0 && ( entry->u.parent.flags & 3 ) == 0 ) {
                Com_Printf( "(index) %d: (weight) %f -> %f, '%s'\n",
                            nodeIndex, state->currentWeight, state->targetWeight,
                            SL_ConvertToString( info->notifyName ) );
            } else if ( info->notifyChildIndex != 0 ) {
                float frac;

                if ( info->notifyIndex < 0 ) {
                    frac = 1.0f;
                } else {
                    XAnimParts *parts = *(XAnimParts **)(
                        (char *)tree->sourceTree
                                    ->entries[info->notifyChildIndex].u.leafAsset
                        + 4 );

                    frac = parts->notify[info->notifyIndex].frameFrac;
                }
                Com_Printf(
                    "(index) %d: (weight) %f -> %f, (time) %f -> %f, '%s', (%f)\n",
                    nodeIndex, state->currentWeight, state->targetWeight,
                    state->oldTime, state->time,
                    SL_ConvertToString( info->notifyName ), frac );
            } else {
                Com_Printf(
                    "(index) %d: (weight) %f -> %f, (time) %f -> %f, '%s'\n",
                    nodeIndex, state->currentWeight, state->targetWeight,
                    state->oldTime, state->time,
                    SL_ConvertToString( info->notifyName ) );
            }
        } else if ( entry->childCount != 0
                 && ( entry->u.parent.flags & 3 ) == 0 ) {
            Com_Printf( "(index) %d: (weight) %f -> %f\n",
                        nodeIndex, state->currentWeight, state->targetWeight );
        } else {
            Com_Printf( "(index) %d: (weight) %f -> %f, (time) %f -> %f\n",
                        nodeIndex, state->currentWeight, state->targetWeight,
                        state->oldTime, state->time );
        }

        for ( i = 0; i < childCount; ++i ) {
            XAnimDisplay( tree, entry->u.parent.firstChildIndex + i, depth + 1 );
        }
    } else {
        void      **fileData = (void **)entry->u.leafAsset;
        XAnimParts *parts    = (XAnimParts *)fileData[1];
        float       elapsed  = state->time - state->oldTime;
        float       realTimeDelta;

        if ( elapsed < 0.0f ) {
            elapsed += 1.0f;
        }
        realTimeDelta = ( parts->frequency == 0.0f )
                            ? 0.0f
                            : elapsed / parts->frequency;

        if ( xanim_activePoolSlot && info->notifyName != 0 ) {
            float frac;

            if ( info->notifyIndex < 0 ) {
                frac = 1.0f;
            } else {
                frac = parts->notify[info->notifyIndex].frameFrac;
            }
            Com_Printf(
                "(name) %s: (weight) %f -> %f, (time) %f -> %f, "
                "(realtimedelta) %f, '%s' (%f)\n",
                (const char *)fileData[0],
                state->currentWeight, state->targetWeight,
                state->oldTime, state->time, realTimeDelta,
                SL_ConvertToString( info->notifyName ), frac );
        } else {
            Com_Printf(
                "(name) %s: (weight) %f -> %f, (time) %f -> %f, "
                "(realtimedelta) %f\n",
                (const char *)fileData[0],
                state->currentWeight, state->targetWeight,
                state->oldTime, state->time, realTimeDelta );
        }
    }
}
#if 0
// XAnimDisplay: recursive developer print of a runtime tree: per node weight, time and notetrack.
void __cdecl XAnimDisplay(int *a1, int ArgList, int a3)
{
  int *v3;
  unsigned __int16 v4;
  unsigned __int16 *v5;
  unsigned __int16 *v6;
  float *v7;
  double v8;
  const char **v9;
  double v10;
  const char *v11;
  double v12;
  unsigned __int16 v13;
  __int16 v14;
  double v15;
  unsigned __int16 v16;
  int v17;
  unsigned __int16 v18;
  const char *v19;
  __int16 v20;
  double v21;
  const char *v22;
  int i;
  int v24;
  int v25;
  unsigned __int16 *v26;

  v3 = a1;
  v4 = *((_WORD *)a1 + ArgList + 4);
  if ( v4 )
  {
    v5 = (unsigned __int16 *)(*a1 + 8 * ArgList + 8);
    v6 = (unsigned __int16 *)((char *)&xanim_pool + 68 * v4);
    v25 = *v5;
    v7 = (float *)&v6[14 * xanim_activePoolSlot + 6];
    v26 = v5;
    v8 = *(float *)&v6[14 * xanim_activePoolSlot + 16];
    if ( !((v8 == 0.0) | __UNORDERED__(v8, 0.0)) )
    {
      if ( a3 > 0 )
      {
        v24 = a3;
        do
        {
          Com_Printf(" ");
          --v24;
        }
        while ( v24 );
        v5 = v26;
        v3 = a1;
      }
      if ( v25 )
      {
        if ( xanim_activePoolSlot && (v16 = v6[2]) != 0 )
        {
          v17 = *v3;
          if ( *(_WORD *)(*v3 + 8 * ArgList + 8) && (*(_BYTE *)(v17 + 8 * ArgList + 12) & 3) == 0 )
          {
            Com_Printf(
              "(index) %d: (weight) %f -> %f, '%s'\n",
              ArgList,
              v7[5],
              v7[4],
              (const char *)(GetRefString_var + 8 * v16 + 4));
          }
          else
          {
            v18 = *v6;
            if ( *v6 )
            {
              v20 = v6[1];
              if ( v20 < 0 )
                v21 = 1.0;
              else
                v21 = *(float *)(*(_DWORD *)(*(_DWORD *)(*(_DWORD *)(v17 + 8 * v18 + 12) + 4) + 20) + 8 * v20 + 4);
              v22 = (const char *)SL_ConvertToString(v16);
              Com_Printf(
                "(index) %d: (weight) %f -> %f, (time) %f -> %f, '%s', (%f)\n",
                ArgList,
                v7[5],
                v7[4],
                v7[1],
                *v7,
                v22,
                v21);
            }
            else
            {
              v19 = (const char *)SL_ConvertToString(v16);
              Com_Printf(
                "(index) %d: (weight) %f -> %f, (time) %f -> %f, '%s'\n",
                ArgList,
                v7[5],
                v7[4],
                v7[1],
                *v7,
                v19);
            }
          }
        }
        else if ( *(_WORD *)(*v3 + 8 * ArgList + 8) && (*(_BYTE *)(*v3 + 8 * ArgList + 12) & 3) == 0 )
        {
          Com_Printf("(index) %d: (weight) %f -> %f\n", ArgList, v7[5], v7[4]);
        }
        else
        {
          Com_Printf("(index) %d: (weight) %f -> %f, (time) %f -> %f\n", ArgList, v7[5], v7[4], v7[1], *v7);
        }
        for ( i = 0; i < v25; ++i )
          XAnimDisplay(a1, i + v26[3], a3 + 1);
      }
      else
      {
        v9 = (const char **)*((_DWORD *)v5 + 1);
        v10 = *v7 - v7[1];
        v11 = v9[1];
        if ( (v10 < 0.0) | __UNORDERED__(v10, 0.0) )
          v10 = v10 + 1.0;
        if ( (*((float *)v11 + 2) == 0.0) | __UNORDERED__(*((float *)v11 + 2), 0.0) )
          v12 = 0.0;
        else
          v12 = v10 / *((float *)v11 + 2);
        if ( xanim_activePoolSlot && (v13 = v6[2]) != 0 )
        {
          v14 = v6[1];
          if ( v14 < 0 )
            v15 = 1.0;
          else
            v15 = *(float *)(*((_DWORD *)v11 + 5) + 8 * v14 + 4);
          Com_Printf(
            "(name) %s: (weight) %f -> %f, (time) %f -> %f, (realtimedelta) %f, '%s' (%f)\n",
            *v9,
            v7[5],
            v7[4],
            v7[1],
            *v7,
            v12,
            (const char *)(GetRefString_var + 8 * v13 + 4),
            v15);
        }
        else
        {
          Com_Printf(
            "(name) %s: (weight) %f -> %f, (time) %f -> %f, (realtimedelta) %f\n",
            *v9,
            v7[5],
            v7[4],
            v7[1],
            *v7,
            v12);
        }
      }
    }
  }
}
#endif

/* ---- XAnimCalcTreeDelta_m  0x004878D0 ----  VERIFIED */
void __cdecl XAnimCalcTreeDelta_m( XAnimTree *tree, int nodeIndex, float weight,
                         float *delta, int isFirst, int isChild )
{
    XAnimEntry *entry      = &tree->sourceTree->entries[nodeIndex];
    int         childCount = entry->childCount;
    int         firstChild;
    int         first;
    int         second;
    int         i;
    float       firstWeight  = 0.0f;
    float       secondWeight = 0.0f;
    float      *out;
    float       lengthSq;
    float       scale;
    float       scratch[6];

    if ( childCount == 0 ) {
        XAnimParts     *parts;
        XAnimState     *state;
        unsigned short  handle;

        if ( (unsigned char)isFirst ) {
            delta[0] = 0.0f;
            delta[1] = 0.0f;
            delta[2] = 0.0f;
            delta[3] = 0.0f;
            delta[4] = 0.0f;
            delta[5] = 0.0f;
        }

        parts = *(XAnimParts **)( (char *)entry->u.leafAsset + 4 );
        if ( !parts->hasDeltaMotion ) {
            return;
        }

        handle = tree->poolNodeHandles[nodeIndex];
        if ( handle == 0 ) {
            return;
        }
        state = &XANIM_LANE( handle );

        if ( byte_A9C63C ) {
            XAnimCalcAbsDeltaParts( delta, (unsigned short *)parts, weight, state->time );
        } else {
            XAnimCalcRelDeltaParts( delta, (int)parts, weight, state->oldTime, state->time );
        }
        return;
    }

    firstChild = entry->u.parent.firstChildIndex;

    for ( first = 0; first < childCount; ++first ) {
        unsigned short handle = tree->poolNodeHandles[firstChild + first];

        if ( handle != 0 ) {
            firstWeight = dword_A9C620 ? XANIM_LANE( handle ).targetWeight
                                       : XANIM_LANE( handle ).currentWeight;
            if ( firstWeight != 0.0f ) {
                break;
            }
        }
    }

    if ( first >= childCount ) {
        if ( (unsigned char)isFirst ) {
            delta[0] = 0.0f;
            delta[1] = 0.0f;
            delta[2] = 0.0f;
            delta[3] = 0.0f;
            delta[4] = 0.0f;
            delta[5] = 0.0f;
        }
        return;
    }

    for ( second = first + 1; second < childCount; ++second ) {
        unsigned short handle = tree->poolNodeHandles[firstChild + second];

        if ( handle != 0 ) {
            secondWeight = dword_A9C620 ? XANIM_LANE( handle ).targetWeight
                                        : XANIM_LANE( handle ).currentWeight;
            if ( secondWeight != 0.0f ) {
                break;
            }
        }
    }

    if ( second >= childCount ) {
        XAnimCalcTreeDelta_m( tree, firstChild + first, weight, delta, isFirst, isChild );
        return;
    }

    out = (unsigned char)isFirst ? delta : scratch;

    XAnimCalcTreeDelta_m( tree, firstChild + first,  firstWeight,  out, 1, 1 );
    XAnimCalcTreeDelta_m( tree, firstChild + second, secondWeight, out, 0, 1 );

    for ( i = second + 1; i < childCount; ++i ) {
        unsigned short handle = tree->poolNodeHandles[firstChild + i];

        if ( handle != 0 ) {
            float childWeight = dword_A9C620 ? XANIM_LANE( handle ).targetWeight
                                             : XANIM_LANE( handle ).currentWeight;

            if ( childWeight != 0.0f ) {
                XAnimCalcTreeDelta_m( tree, firstChild + i, childWeight, out, 0, 1 );
            }
        }
    }

    if ( !(unsigned char)isChild ) {
        if ( delta[2] == 0.0f ) {
            return;
        }
        scale = 1.0f / delta[2];
        delta[3] *= scale;
        delta[4] *= scale;
        delta[5] *= scale;
        return;
    }

    if ( (unsigned char)isFirst ) {
        lengthSq = delta[0] * delta[0] + delta[1] * delta[1];
        if ( lengthSq != 0.0f ) {
            scale = weight / (float)sqrt( (double)lengthSq );

            delta[0] *= scale;
            delta[1] *= scale;
        }

        if ( delta[2] == 0.0f ) {
            return;
        }
        scale    = weight / delta[2];
        delta[2] = weight;
        delta[3] *= scale;
        delta[4] *= scale;
        delta[5] *= scale;
        return;
    }

    lengthSq = scratch[0] * scratch[0] + scratch[1] * scratch[1];
    if ( lengthSq != 0.0f ) {
        scale = weight / (float)sqrt( (double)lengthSq );

        delta[0] += scale * scratch[0];
        delta[1] += scale * scratch[1];
    }

    if ( scratch[2] == 0.0f ) {
        return;
    }
    scale     = weight / scratch[2];
    delta[2] += weight;
    delta[3] += scale * scratch[3];
    delta[4] += scale * scratch[4];
    delta[5] += scale * scratch[5];
}
#if 0
// XAnimCalcTreeDelta_m: recursively accumulates weighted root motion over a tree into a quaternion plus translation.  Only caller of XAnimCalcAbsDeltaParts and XAnimCalcRelDeltaParts.
void __cdecl XAnimCalcTreeDelta_m(_DWORD *a1, int a2, float a3, int a4, int a5, int a6)
{
  int v7;
  int v8;
  int v9;
  unsigned __int16 v10;
  float *v11;
  _WORD *v12;
  int *v13;
  double v14;
  int v15;
  _WORD *v16;
  int *v17;
  double v18;
  float *v19;
  int i;
  int v21;
  unsigned __int16 v22;
  int *v23;
  double v24;
  double v25;
  double v26;
  long double v27;
  double v28;
  long double v29;
  double v30;
  float v31;
  int v32;
  char v33; // [esp+1Ch] [ebp-18h] BYREF
  int v34;
  float v35;
  float v36;
  int v37;

  v7 = *(unsigned __int16 *)(*a1 + 8 * a2 + 8);
  v8 = *a1 + 8 * a2 + 8;
  v32 = v8;
  if ( !*(_WORD *)v8 )
  {
    if ( (_BYTE)a5 )
    {
      *(_DWORD *)a4 = 0;
      *(_DWORD *)(a4 + 4) = 0;
      *(_DWORD *)(a4 + 8) = 0;
      *(_DWORD *)(a4 + 12) = 0;
      *(_DWORD *)(a4 + 16) = 0;
      *(_DWORD *)(a4 + 20) = 0;
    }
    v9 = *(_DWORD *)(*(_DWORD *)(v8 + 4) + 4);
    if ( *(_BYTE *)(v9 + 3) )
    {
      v10 = *((_WORD *)a1 + a2 + 4);
      if ( v10 )
      {
        v11 = (float *)&dword_A7A62C[17 * v10 + 7 * xanim_activePoolSlot];
        if ( byte_A9C63C )
          XAnimCalcAbsDeltaParts((float *)a4, (unsigned __int16 *)v9, a3, *v11);
        else
          XAnimCalcRelDeltaParts((float *)a4, v9, a3, v11[1], *v11);
      }
    }
    return;
  }
  v37 = 0;
  if ( v7 <= 0 )
  {
LABEL_17:
    if ( (_BYTE)a5 )
    {
      *(_DWORD *)a4 = 0;
      *(_DWORD *)(a4 + 4) = 0;
      *(_DWORD *)(a4 + 8) = 0;
      *(_DWORD *)(a4 + 12) = 0;
      *(_DWORD *)(a4 + 16) = 0;
      *(_DWORD *)(a4 + 20) = 0;
    }
  }
  else
  {
    v34 = *(unsigned __int16 *)(v8 + 6);
    v12 = (_WORD *)a1 + v34 + 4;
    while ( 1 )
    {
      if ( *v12 )
      {
        v13 = &dword_A7A62C[17 * (unsigned __int16)*v12 + 7 * xanim_activePoolSlot];
        v14 = dword_A9C620 ? *((float *)v13 + 4) : *((float *)v13 + 5);
        v31 = v14;
        if ( !((v14 == 0.0) | __UNORDERED__(v14, 0.0)) )
          break;
      }
      ++v12;
      if ( ++v37 >= v7 )
        goto LABEL_17;
    }
    v15 = v37 + 1;
    if ( v37 + 1 >= v7 )
    {
LABEL_28:
      XAnimCalcTreeDelta_m(a1, v37 + v34, a3, a4, a5, a6);
      return;
    }
    v16 = (_WORD *)a1 + v34 + v15 + 4;
    while ( 1 )
    {
      if ( *v16 )
      {
        v17 = &dword_A7A62C[17 * (unsigned __int16)*v16 + 7 * xanim_activePoolSlot];
        v18 = dword_A9C620 ? *((float *)v17 + 4) : *((float *)v17 + 5);
        if ( !((v18 == 0.0) | __UNORDERED__(v18, 0.0)) )
          break;
      }
      ++v15;
      ++v16;
      if ( v15 >= v7 )
        goto LABEL_28;
    }
    v19 = (float *)a4;
    if ( !(_BYTE)a5 )
      v19 = (float *)&v33;
    XAnimCalcTreeDelta_m(a1, v37 + v34, v31, (int)v19, 1, 1);
    v35 = v18;
    XAnimCalcTreeDelta_m(a1, v15 + *(unsigned __int16 *)(v32 + 6), v35, (int)v19, 0, 1);
    for ( i = v15 + 1; i < v7; ++i )
    {
      v21 = *(unsigned __int16 *)(v32 + 6) + i;
      v22 = *((_WORD *)a1 + v21 + 4);
      if ( v22 )
      {
        v23 = &dword_A7A62C[17 * v22 + 7 * xanim_activePoolSlot];
        if ( dword_A9C620 )
          v24 = *((float *)v23 + 4);
        else
          v24 = *((float *)v23 + 5);
        if ( !((v24 == 0.0) | __UNORDERED__(v24, 0.0)) )
        {
          v36 = v24;
          XAnimCalcTreeDelta_m(a1, v21, v36, (int)v19, 0, 1);
        }
      }
    }
    if ( !(_BYTE)a6 )
    {
      if ( (*(float *)(a4 + 8) == 0.0) | __UNORDERED__(*(float *)(a4 + 8), 0.0) )
        return;
      v25 = 1.0 / *(float *)(a4 + 8);
LABEL_47:
      *(float *)(a4 + 12) = v25 * *(float *)(a4 + 12);
      *(float *)(a4 + 16) = v25 * *(float *)(a4 + 16);
      *(float *)(a4 + 20) = v25 * *(float *)(a4 + 20);
      return;
    }
    if ( (_BYTE)a5 )
    {
      v26 = *(float *)a4 * *(float *)a4 + *(float *)(a4 + 4) * *(float *)(a4 + 4);
      if ( !((v26 == 0.0) | __UNORDERED__(v26, 0.0)) )
      {
        v27 = a3 / sqrt(v26);
        *(float *)a4 = v27 * *(float *)a4;
        *(float *)(a4 + 4) = v27 * *(float *)(a4 + 4);
      }
      if ( !((*(float *)(a4 + 8) == 0.0) | __UNORDERED__(*(float *)(a4 + 8), 0.0)) )
      {
        v25 = a3 / *(float *)(a4 + 8);
        *(float *)(a4 + 8) = a3;
        goto LABEL_47;
      }
    }
    else
    {
      v28 = *v19 * *v19 + v19[1] * v19[1];
      if ( !((v28 == 0.0) | __UNORDERED__(v28, 0.0)) )
      {
        v29 = a3 / sqrt(v28);
        *(float *)a4 = v29 * *v19 + *(float *)a4;
        *(float *)(a4 + 4) = v29 * v19[1] + *(float *)(a4 + 4);
      }
      if ( !((v19[2] == 0.0) | __UNORDERED__(v19[2], 0.0)) )
      {
        v30 = a3 / v19[2];
        *(float *)(a4 + 8) = a3 + *(float *)(a4 + 8);
        *(float *)(a4 + 12) = v30 * v19[3] + *(float *)(a4 + 12);
        *(float *)(a4 + 16) = v30 * v19[4] + *(float *)(a4 + 16);
        *(float *)(a4 + 20) = v30 * v19[5] + *(float *)(a4 + 20);
      }
    }
  }
}
#endif

/* ---- XAnimGetLength  0x00487C90 ----  VERIFIED */
double __cdecl XAnimGetLength( int nodeIndex, const XAnim *sourceTree )
{
    void       *leafAsset = sourceTree->entries[nodeIndex].u.leafAsset;
    XAnimParts *parts     = *(XAnimParts **)( (char *)leafAsset + 4 );

    return (double)parts->frameCountMinusOne / parts->frameRate;
}

/* ---- XAnimGetTime  0x00487CB0 ----  VERIFIED */
double __cdecl XAnimGetTime( int nodeIndex, XAnimTree *tree )
{
    unsigned short handle = tree->poolNodeHandles[nodeIndex];

    if ( handle ) {
        return XANIM_LANE( handle ).time;
    }
    return 0.0;
}
#if 0
// XAnimGetTime: current normalised time of a tree node in the active state lane (pool node + 0x0C + 28*activeSlot = XAnimState.time; 0x00A9CC58 is xanim_activePoolPayloadSlot).  Cgame trap 146.
double __cdecl XAnimGetTime(int a1, int a2)
{
  unsigned __int16 v2;

  v2 = *(_WORD *)(a2 + 2 * a1 + 8);
  if ( v2 )
    return *(float *)&dword_A7A62C[17 * v2 + 7 * xanim_activePoolSlot];
  else
    return 0.0;
}
#endif

/* ---- XAnimGetWeight  0x00487CE0 ----  VERIFIED */
double __cdecl XAnimGetWeight( int nodeIndex, XAnimTree *tree )
{
    unsigned short handle = tree->poolNodeHandles[nodeIndex];

    if ( handle ) {
        return XANIM_LANE( handle ).currentWeight;
    }
    return 0.0;
}
#if 0
// XAnimGetWeight: current blend weight of a tree node (XAnimState.currentWeight at +0x14 of the active lane).  Cgame trap 147.
double __cdecl XAnimGetWeight(int a1, int a2)
{
  unsigned __int16 v2;

  v2 = *(_WORD *)(a2 + 2 * a1 + 8);
  if ( v2 )
    return *((float *)&unk_A7A640 + 17 * v2 + 7 * xanim_activePoolSlot);
  else
    return 0.0;
}
#endif

/* ---- XAnimHasTime  0x00487D10 ----  VERIFIED */
BOOL __cdecl XAnimHasTime( int nodeIndex, XAnimTree *tree )
{
    unsigned short handle = tree->poolNodeHandles[nodeIndex];
    XAnimState    *lane;

    if ( !handle ) {
        return 1;
    }
    lane = &XANIM_LANE( handle );

    if ( lane->time >= lane->oldTime
      && *(const int *)&lane->time != 1065353216
      && lane->cycleCount <= lane->oldCycleCount ) {
        return 0;
    }
    return 1;
}
#if 0
// XAnimHasTime: false once a node's animation has run out (time past oldTime, at 1.0, cycle count not advancing).  Cgame trap 180.
BOOL __cdecl XAnimHasTime(int a1, int a2)
{
  unsigned __int16 v2;
  double v3;
  int *v4;
  BOOL result;

  v2 = *(_WORD *)(a2 + 2 * a1 + 8);
  result = 1;
  if ( v2 )
  {
    v3 = *(float *)&dword_A7A62C[17 * v2 + 7 * xanim_activePoolSlot];
    v4 = &dword_A7A62C[17 * v2 + 7 * xanim_activePoolSlot];
    if ( !((v3 < *(float *)&dword_A7A630[17 * v2 + 7 * xanim_activePoolSlot])
         | __UNORDERED__(v3, *(float *)&dword_A7A630[17 * v2 + 7 * xanim_activePoolSlot]))
      && *v4 != 1065353216
      && *((_WORD *)v4 + 4) <= *((_WORD *)v4 + 5) )
    {
      return 0;
    }
  }
  return result;
}
#endif

/* ---- XAnimGetNumChildren_m  0x00487D60 ----  VERIFIED */
int __cdecl XAnimGetNumChildren_m(int a1, int a2)
{
  return *(unsigned __int16 *)(a2 + 8 * a1 + 8);
}

/* ---- XAnimGetChildIndex_m  0x00487D70 ----  VERIFIED */
int __cdecl XAnimGetChildIndex_m(int a1, int a2, int a3)
{
  return a3 + *(unsigned __int16 *)(a2 + 8 * a1 + 14);
}

/* ---- XAnimGetAnimName  0x00487D80 ----  VERIFIED */
const char *__cdecl XAnimGetAnimName( int nodeIndex, const XAnim *sourceTree )
{
    const XAnimEntry *entry = &sourceTree->entries[nodeIndex];

    if ( entry->childCount )
        return "<non-leaf anim>";
    return *(const char **)entry->u.leafAsset;
}

/* ---- XAnimGetAnimTreeDebugName_m  0x00487DA0 ----  VERIFIED */
const char *__cdecl XAnimGetAnimTreeDebugName_m( const XAnim *sourceTree )
{
    return sourceTree->name;
}

/* ---- XAnimGetAnimTreeSize  0x00487DB0 ----  VERIFIED */
int __cdecl XAnimGetAnimTreeSize( const XAnim *sourceTree )
{
    return sourceTree->nodeCount;
}

/* ---- DObjUpdateOldServerTime_m  0x00487DC0 ----  VERIFIED */
__int16 __cdecl DObjUpdateOldServerTime_m( DObj *dobj )
{
    XAnimTree *tree = dobj->tree;

    if ( tree )
    {
        xanim_currentTree = (int)tree;
        return XAnimUpdateOldServerTime( 0 );
    }
    return 0;
}

/* ---- DObjInitServerTime  0x00487DE0 ----  VERIFIED */
void __cdecl DObjInitServerTime( DObj *dobj, int serverTime )
{
    XAnimTree *tree = dobj->tree;

    if ( tree )
    {
        xanim_currentTime = serverTime;
        xanim_currentTree = (int)tree;
        XAnimInitServerTime_m( 0 );
    }
}

/* ---- DObjUpdateClientInfo_m  0x00487E10 ----  VERIFIED */
void __cdecl DObjUpdateClientInfo_m(DObj *dobj, float dtime)
{
  XAnimTree *tree;

  xanim_numDeferredNotifies = 0;
  tree = dobj->tree;
  if ( tree )
  {
    xanim_currentTree = (int)tree;
    xanim_notifyEntId = dobj->scrNotifyId;
    xanim_currentTime = LODWORD(dtime);
    XAnimUpdateClientInfoInternal(0, dtime, 1);
  }
}

/* ---- DObjUpdateServerInfo  0x00487E50 ----  VERIFIED */
int __cdecl DObjUpdateServerInfo(DObj *dobj, float serverTime, int stopOnNotetrack)
{
  XAnimTree *tree;
  double frac;
  double clamped;
  float clampedf;

  tree = dobj->tree;
  if ( !tree )
    return 0;
  xanim_currentTree = (int)tree;
  xanim_notifyEntId = dobj->scrNotifyId;

  if ( !stopOnNotetrack )
  {
    XAnimUpdateServerInfoInternal(0, serverTime, 0);
    return 0;
  }

  frac = XAnimFindServerNoteTrack(0, serverTime);
  if ( (frac == 1.0) | __UNORDERED__(frac, 1.0)
    || (clamped = frac * serverTime + 0.001, clamped > serverTime) )
  {
    XAnimUpdateServerInfoInternal(0, serverTime, 1);
    return 0;
  }
  clampedf = (float)clamped;
  XAnimUpdateServerInfoInternal(0, clampedf, 1);
  return 1;
}

/* ---- XAnimGetClientNotifies_m  0x00487EF0 ----  VERIFIED */
int __cdecl XAnimGetClientNotifies_m( XAnimDeferredNotify **out )
{
    *out = xanim_deferredNotifies;
    return xanim_numDeferredNotifies;
}

/* ---- DObjCalcAnim  0x00487F00 ----  VERIFIED */
int __cdecl DObjCalcAnim( DObj *dobj, unsigned int *requestBits )
{
    XAnimEvalStorage *storage = (XAnimEvalStorage *)dobj->evalStorage;
    unsigned int skip0, skip1, skip2, skip3;
    int          allSkipped;

    skip0 = *(unsigned int *)&storage->evaluatedPartBits[0]  | ~requestBits[0];
    skip1 = *(unsigned int *)&storage->evaluatedPartBits[4]  | ~requestBits[1];
    skip2 = *(unsigned int *)&storage->evaluatedPartBits[8]  | ~requestBits[2];
    skip3 = *(unsigned int *)&storage->evaluatedPartBits[12] | ~requestBits[3];
    xanim_evalPosedBits[0] = skip0;
    xanim_evalPosedBits[1] = skip1;
    xanim_evalPosedBits[2] = skip2;
    xanim_evalPosedBits[3] = skip3;
    allSkipped = ( skip0 == -1 ) && ( skip1 == -1 ) && ( skip2 == -1 ) && ( skip3 == -1 );

    if ( !allSkipped )
    {
        int bone;
        int i;
        DObjAnimMat *out;

        *(unsigned int *)&storage->evaluatedPartBits[0]  |= requestBits[0];
        *(unsigned int *)&storage->evaluatedPartBits[4]  |= requestBits[1];
        *(unsigned int *)&storage->evaluatedPartBits[8]  |= requestBits[2];
        *(unsigned int *)&storage->evaluatedPartBits[12] |= requestBits[3];
        xanim_evalSkipBits[0] = skip0;
        xanim_evalSkipBits[1] = skip1;
        xanim_evalSkipBits[2] = skip2;
        xanim_evalSkipBits[3] = skip3;

        xanim_evalChildCount = dobj->childCount;
        xanim_evalChildRefs = (int)dobj->childRefs;
        xanim_evalDObj = (int)dobj;
        xanim_currentTree = (int)dobj->tree;
        xanim_evalPartCount = dobj->partCount;

        out = (DObjAnimMat *)( (char *)storage + 0x30 + dobj->partCount * 0x40 );

        if ( dobj->tree )
        {
            dword_A7A61C = 32 * dobj->partCount;
            BYTE3( xanim_evalSkipBits[3] ) = BYTE3( xanim_evalPosedBits[3] ) | 0x80;
            XAnimCalc( 0, 1.0f, out, 1, 0 );
        }

        bone = 0;
        for ( i = 0; i < dobj->childCount; ++i )
        {
            __int16 ***partsData = *(__int16 ****)( **(_DWORD **)( *(_DWORD *)( xanim_evalChildRefs + 4 * i ) + 4 ) + 4 );
            int   modelPartCount = *((__int16 *)partsData + 2);
            __int16 *basePose    = (__int16 *)partsData[3];
            int   j;

            for ( j = modelPartCount; j; --j )
            {
                if ( ( (1 << (bone & 7)) & *((char *)xanim_evalPosedBits + (bone >> 3)) ) == 0 )
                {
                    out->quat[0]  = 0.0f;
                    out->quat[1]  = 0.0f;
                    out->quat[2]  = 0.0f;
                    out->quat[3]  = 1.0f;
                    out->trans[2] = 0.0f;
                    out->trans[1] = 0.0f;
                    out->trans[0] = 0.0f;
                }
                ++out;
                ++bone;
            }

            if ( ***partsData != modelPartCount )
            {
                int remaining = ***partsData - modelPartCount;

                do
                {
                    if ( ( (1 << (bone & 7)) & *((char *)xanim_evalPosedBits + (bone >> 3)) ) == 0 )
                    {
                        out->quat[0]  = (double)basePose[0] * 0.000030518509;
                        out->quat[1]  = (double)basePose[1] * 0.000030518509;
                        out->quat[2]  = (double)basePose[2] * 0.000030518509;
                        out->trans[2] = 0.0f;
                        out->trans[1] = 0.0f;
                        out->trans[0] = 0.0f;
                        out->quat[3]  = (double)basePose[3] * 0.000030518509;
                    }
                    ++out;
                    ++bone;
                    basePose += 4;
                    --remaining;
                }
                while ( remaining );
            }
        }
        return dobj->childCount;
    }
    return 0;
}
