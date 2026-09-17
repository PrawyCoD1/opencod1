
#ifndef TR_SMCACHE_H
#define TR_SMCACHE_H

typedef struct {
    int next;                       /* +0x00 */
    int prev;                       /* +0x04 */
} smcLink_t;

typedef struct {
    unsigned short usedVertexCount; /* +0x00 */
    unsigned char  allocated;       /* +0x02 */
    unsigned char  pad03;           /* +0x03  never read or written */
} smcNode_t;

typedef union {
    smcLink_t freeLink;
    struct {
        int surfaceType;            /* +0x00  12..15, the backend selector */
        int source;                 /* +0x04  XSurface *  */
        int vertexOffset;           /* +0x08  in vertices, written by SMC_Allocate */
        int surfaceIndex;           /* +0x0C  index into owner->surfaceLightingCache */
        int owner;                  /* +0x10  renderer_static_model_t * */
    } cached;
} smcSurface_t;

typedef struct {
    smcLink_t    lruLink;           /* +0x000  threaded on cache->lruList */
    int          lastUsedFrame;     /* +0x008  tr.frameCount stamp */
    smcNode_t    tree[31];          /* +0x00C  buddy tree, 0-based heap */
    smcSurface_t surfaces[16];      /* +0x088 */
} smcPage_t;

typedef struct {
    smcPage_t pages[128];           /* +0x0000 */
    smcLink_t freeLists[5];         /* +0xE400  [0] = 512 verts .. [4] = 32 */
    smcLink_t lruList;              /* +0xE428  whole pages, LRU order */
    int       allocatedVertexCount; /* +0xE430  sum of 1 << sizeShift */
    int       usedVertexCount;      /* +0xE434  sum of surf->vertCount */
} smcCache_t;                       /* 0xE438 == 58424 */

typedef char smc_assert_node_size[(sizeof(smcNode_t)    ==     4) ? 1 : -1];
typedef char smc_assert_surf_size[(sizeof(smcSurface_t) ==    20) ? 1 : -1];
typedef char smc_assert_page_size[(sizeof(smcPage_t)    ==   456) ? 1 : -1];
typedef char smc_assert_cache_sz[(sizeof(smcCache_t)    == 58424) ? 1 : -1];

/* Storage is in renderer/tr_staticmodelcache.c.  Retail 0x011CC230. */
extern smcCache_t r_smcCache;

#define unk_11CC230   (*(unsigned char *)&r_smcCache)
#define unk_11CC2B8   (*(unsigned char *)&r_smcCache.pages[0].surfaces[0])

#define dword_11DA630 r_smcCache.freeLists[0].next
#define dword_11DA634 r_smcCache.freeLists[0].prev
#define dword_11DA638 r_smcCache.freeLists[1].next
#define dword_11DA63C r_smcCache.freeLists[1].prev
#define dword_11DA640 r_smcCache.freeLists[2].next
#define dword_11DA644 r_smcCache.freeLists[2].prev
#define dword_11DA648 r_smcCache.freeLists[3].next
#define dword_11DA64C r_smcCache.freeLists[3].prev
#define dword_11DA650 r_smcCache.freeLists[4].next
#define dword_11DA654 r_smcCache.freeLists[4].prev
#define dword_11DA658 r_smcCache.lruList.next
#define dword_11DA65C r_smcCache.lruList.prev
#define dword_11DA660 r_smcCache.allocatedVertexCount
#define dword_11DA664 r_smcCache.usedVertexCount

#endif
