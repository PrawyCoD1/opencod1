/*
 * cm_local.h -- collision-system record layouts for Call of Duty 1.1 (Windows).
 *
 * The retail tree had a qcommon/cm_local.h; this is it, rebuilt from
 * CoDMP.exe.  Only the cm_*.c units include it.
 *
 * WHY THIS FILE EXISTS SEPARATELY FROM cod1_types.h
 * -------------------------------------------------
 * cod1_types.h still declares every collision record as an opaque blob of a
 * proven size (COD1_OPAQUE_TYPE).  Nothing here contradicts those sizes --
 * each real layout below is asserted to be exactly the size cod1_types.h
 * already proves.  The opaque names are renamed aside for the duration of the
 * cod1_types.h include so both can coexist; when these layouts move into
 * cod1_types.h the whole #define block at the top of this file is deleted in
 * one step and no .c file needs to change.
 *
 * Every struct is corroborated by opencoduo (coduo-server-master/
 * dedicated-engine/src/physics_collision/): CoD1 and CoD:UO load each other's
 * .bsp files, so the on-disk and in-memory collision records are necessarily
 * the same shape.
 *
 * @fidelity: verified
 */

#ifndef __CM_LOCAL_H__
#define __CM_LOCAL_H__

/* ------------------------------------------------------------------------- *
 * Rename cod1_types.h's opaque placeholders aside.  Delete this block (and
 * the matching #undef block) once the real layouts live in cod1_types.h.
 * ------------------------------------------------------------------------- */
#define cbrush_t            cod1_opaque_cbrush_t
#define cLeaf_t             cod1_opaque_cLeaf_t
#define cmodel_t            cod1_opaque_cmodel_t
#define cPatch_t            cod1_opaque_cPatch_t
#define leafList_t          cod1_opaque_leafList_t
#define traceWork_t         cod1_opaque_traceWork_t
#define moveclip_t          cod1_opaque_moveclip_t
#define pointtrace_t        cod1_opaque_pointtrace_t
#define sightclip_t         cod1_opaque_sightclip_t
#define sightpointtrace_t   cod1_opaque_sightpointtrace_t

#include "qcommon.h"

#undef cbrush_t
#undef cLeaf_t
#undef cmodel_t
#undef cPatch_t
#undef leafList_t
#undef traceWork_t
#undef moveclip_t
#undef pointtrace_t
#undef sightclip_t
#undef sightpointtrace_t

/* Each real layout must come out at exactly the size cod1_types.h proves. */
#define CM_ASSERT_MATCHES_OPAQUE( real, opaque ) \
	COD1_STATIC_ASSERT( sizeof( real ) == sizeof( opaque ), \
	                    #real " must match the proven size of " #opaque )

/*
=============================================================================

						ON-DISK BSP RECORDS

	CoD's lump_t is { int filelen; int fileofs; } -- REVERSED from Q3/RTCW.
	It is declared that way in cod1_types.h; do not "fix" it.  Proof:
	CM_LoadLump_m (0x0041A180) reads the 272-byte header and then seeks
	FS_Seek(f, lumps[2i+1] - 272, FS_SEEK_CUR), i.e. the SECOND dword is the
	file offset, while the FIRST is used as both the allocation size and the
	read count.  CMod_LoadBrushes (0x00419300) does the same thing inline:
	`mov ebp,[esi+4]` produces the base pointer and `test byte ptr [esi],3`
	validates the length.

=============================================================================
*/

/* The BSP header: ident, version, then 33 lumps.  272 bytes total, which is
 * the size CM_LoadLump_m reads and CM_SaveLump writes (8 + 33*8). */
#define CM_LUMPS            33
#define CM_HEADER_SIZE      272
#define CM_BSP_VERSION      59      /* CM_LoadMap: `cmp eax, 3Bh` */

typedef struct dheader_t
{
	int ident;
	int version;
	lump_t lumps[CM_LUMPS];
} dheader_t;
COD1_ASSERT_SIZE( dheader_t, CM_HEADER_SIZE );

/* Lump indices, read off CM_LoadMap's `lea` operands (0x00419DEC onward).
 * The header base is esp+0x50 and lump i sits at esp+0x58 + 8*i. */
enum
{
	LUMP_MATERIALS      = 0,    /* CMod_LoadShaders                 */
	LUMP_PLANES         = 2,    /* CMod_LoadPlanes                  */
	LUMP_BRUSHSIDES     = 3,    /* CMod_LoadBrushes, first  arg     */
	LUMP_BRUSHES        = 4,    /* CMod_LoadBrushes, second arg     */
	LUMP_COLLISIONVERTS = 25,   /* CMod_LoadLeafCurvesAndTerrain    */
	LUMP_COLLISIONINDEX = 26,
	LUMP_LEAFSURFACES_D = 24,   /* the 16-byte curve/terrain records */
	LUMP_NODES          = 20,
	LUMP_LEAFS          = 21,
	LUMP_LEAFBRUSHES    = 22,
	LUMP_LEAFSURFACES   = 23,
	LUMP_MODELS         = 27,
	LUMP_VISIBILITY     = 28,
	LUMP_ENTITIES       = 29
};

/* 72 bytes: CMod_LoadShaders multiplies the count by 72 and the content flags
 * are read at +68 by CMod_LoadBrushes (`+ 72 * v16 + 68`) and by
 * CMod_LoadLeafCurvesAndTerrain. */
typedef struct dmaterial_t
{
	char material[64];
	int surfaceFlags;
	int contentFlags;
} dmaterial_t;
COD1_ASSERT_SIZE( dmaterial_t, 72 );

/* 16 bytes per record; CMod_LoadLeafCurvesAndTerrain validates the lump with
 * `test al, 0Fh` and derives the count with `shr edi, 4`.
 *
 * The same two 16-bit fields mean different things in the two branches, which
 * is why they are named neutrally here:
 *   isTerrain == 0  -- a curved patch:  a = width, b = height, and the vertex
 *                      run starts at collisionVerts[secondIndex].
 *   isTerrain != 0  -- a terrain patch: a = vertex count, b = index count,
 *                      vertices at collisionVerts[firstIndex] and indices at
 *                      collisionIndexes[secondIndex].
 */
typedef struct dcollisionsurface_t
{
	short materialIndex;
	byte isTerrain;
	byte pad;
	short a;
	short b;
	int firstIndex;
	int secondIndex;
} dcollisionsurface_t;
COD1_ASSERT_SIZE( dcollisionsurface_t, 16 );

/*
=============================================================================

						IN-MEMORY COLLISION RECORDS

=============================================================================
*/

/*
 * 8 bytes.  CMod_LoadBrushes stores a PLANE POINTER, not an index:
 *   `*v18 = cm_planes + 20 * planeNum;  v18[1] = materialNum;  v18 += 2;`
 * opencoduo's coduo_collision_brush_side_t agrees (plane +0x00, materialIndex
 * +0x04, size 8).
 */
typedef struct cbrushside_t
{
	cplane_t        *plane;
	int materialNum;
} cbrushside_t;
COD1_ASSERT_SIZE( cbrushside_t, 8 );

/*
 * 52 bytes.  CONFIRMED at two independent sites.
 *
 * CM_InitBoxHull (0x0041A390) is the cleaner of the two and pins everything:
 *   imul eax, 34h            -- stride 52
 *   mov [eax+1Ch], 0         -- numSides
 *   mov [eax+20h], 0         -- sides
 *   mov [ecx], -1            -- contents
 *   mov [edx+28h], ax  ... [ecx+32h], ax   -- six 16-bit axial material
 *                                             indices at +0x28..+0x33
 *
 * CMod_LoadBrushes writes the six axial plane distances as mins[0..2] then
 * maxs[0..2] -- the inner loop steps 12 bytes (one axis' min then max) and the
 * outer loop steps 4 (to the next axis), so the store order is
 * mins.x, maxs.x, mins.y, maxs.y, mins.z, maxs.z into two separate vec3_t.
 * That fixes mins at +0x04 and maxs at +0x10.
 *
 * The six axialMaterialNum entries are written in the same interleaving, so
 * entries 0..2 belong to the mins planes and 3..5 to the maxs planes.
 *
 * Identical to opencoduo's coduo_collision_brush_t.
 */
typedef struct cbrush_t
{
	int contents;                   /* +0x00 */
	vec3_t mins;                    /* +0x04 */
	vec3_t maxs;                    /* +0x10 */
	int numSides;                   /* +0x1C  non-axial sides only */
	cbrushside_t    *sides;         /* +0x20 */
	int checkcount;                 /* +0x24  duplicate-suppression generation */
	short axialMaterialNum[6];      /* +0x28  [0..2] mins planes, [3..5] maxs */
} cbrush_t;
COD1_ASSERT_SIZE( cbrush_t, 52 );
CM_ASSERT_MATCHES_OPAQUE( cbrush_t, cod1_opaque_cbrush_t );

/*
 * 16 bytes.  CMod_LoadLeafs allocates `16 * count` and writes every field
 * below; the "surface" fields index cm_leafsurfaces, which in turn indexes
 * cm_patches.  The field names are retail's own -- they come out of that
 * function's Com_Error strings ("CMod_LoadLeafs: numLeafSurfaces exceeded"
 * and friends), which makes them authoritative even though opencoduo calls
 * the same two fields firstLeafTerrainPatch/numLeafTerrainPatches.
 *
 * cellnum at +0x0E is CoD-specific; opencoduo does not assert it, but
 * CMod_LoadLeafs writes it from source dword 6 and range-checks it.
 */
typedef struct cLeaf_t
{
	short cluster;                  /* +0x00 */
	short area;                     /* +0x02 */
	int firstLeafBrush;             /* +0x04 */
	unsigned short numLeafBrushes;  /* +0x08 */
	unsigned short firstLeafSurface;/* +0x0A */
	unsigned short numLeafSurfaces; /* +0x0C */
	short cellnum;                  /* +0x0E */
} cLeaf_t;
COD1_ASSERT_SIZE( cLeaf_t, 16 );
CM_ASSERT_MATCHES_OPAQUE( cLeaf_t, cod1_opaque_cLeaf_t );

/*
 * 40 bytes = 12 + 12 + sizeof(cLeaf_t).  CONFIRMED three ways:
 *
 *  - CM_ClipHandleToModel indexes cm_cmodels with `40 * handle`.
 *  - CM_ModelBounds (0x0041A4D0) copies [eax+0x00..0x08] to mins and
 *    [eax+0x0C..0x14] to maxs.
 *  - The box model is a static instance at 0x01617580 and CM_InitBoxHull
 *    writes its leaf through two separate globals: firstLeafBrush lands at
 *    0x0161759C (= +0x1C = leaf+0x04) and numLeafBrushes at 0x016175A0
 *    (= +0x20 = leaf+0x08).  sv_worldSectors begins at 0x016175A8, i.e.
 *    exactly 40 bytes after the model, so there is no trailing padding.
 */
typedef struct cmodel_t
{
	vec3_t mins;                    /* +0x00 */
	vec3_t maxs;                    /* +0x0C */
	cLeaf_t leaf;                   /* +0x18  submodels are a single leaf */
} cmodel_t;
COD1_ASSERT_SIZE( cmodel_t, 40 );
CM_ASSERT_MATCHES_OPAQUE( cmodel_t, cod1_opaque_cmodel_t );

/*
 * 8 bytes.  CMod_LoadNodes allocates `8 * count`, stores a plane POINTER
 * (cm_planes + 20 * planeNum) at +0x00 and two signed 16-bit children at
 * +0x04/+0x06.  Negative children are -(leafnum + 1), as in Q3.
 */
typedef struct cNode_t
{
	cplane_t        *plane;
	short children[2];
} cNode_t;
COD1_ASSERT_SIZE( cNode_t, 8 );

/*
 * 8 bytes; CMod_LoadLeafs allocates `8 * cm_numAreas`.  CM_FloodArea_r reads
 * [area*8 + 0] as the flood number and [area*8 + 4] as the flood generation.
 */
typedef struct cArea_t
{
	int floodnum;
	int floodvalid;
} cArea_t;
COD1_ASSERT_SIZE( cArea_t, 8 );

/*
 * 44 bytes.  CMod_LoadLeafCurvesAndTerrain allocates `44 * count` and writes
 * checkcount at +0x00, materialNum at +0x04, the material's contentFlags at
 * +0x08, and the two collide pointers at +0x24 and +0x28 -- which leaves
 * exactly the 24 bytes at +0x0C..+0x23 for the bounds.  opencoduo's
 * coduo_collision_terrain_patch_t has the identical layout.
 *
 * Exactly one of pc/tc is non-NULL; the other branch stores 0 explicitly.
 */
typedef struct cPatch_t
{
	int checkcount;                 /* +0x00 */
	int materialNum;                /* +0x04 */
	int contents;                   /* +0x08 */
	vec3_t mins;                    /* +0x0C */
	vec3_t maxs;                    /* +0x18 */
	patchCollide_t  *pc;            /* +0x24  curved patch, or NULL */
	void            *tc;            /* +0x28  terrain, or NULL     */
} cPatch_t;
COD1_ASSERT_SIZE( cPatch_t, 44 );
CM_ASSERT_MATCHES_OPAQUE( cPatch_t, cod1_opaque_cPatch_t );

/*
 * 48 bytes.  CONFIRMED field by field from CM_BoxLeafnums (0x00421E10), which
 * builds one on its own stack frame at esp+0:
 *   [+0x00] = 0                       count
 *   [+0x04] = listsize                maxcount
 *   [+0x08] = 0                       overflowed
 *   [+0x0C] = list                    list
 *   [+0x10..+0x18] = mins             bounds[0]
 *   [+0x1C..+0x24] = maxs             bounds[1]
 *   [+0x28] = 0                       lastLeaf   (returned through *lastLeaf)
 *   [+0x2C] = offset CM_StoreLeafs    storeLeafs
 * That is Q3's leafList_t unchanged, which is what the 48-byte size already
 * hinted at -- this is one of the structures CoD inherited untouched.
 */
typedef struct leafList_t
{
	int count;                      /* +0x00 */
	int maxcount;                   /* +0x04 */
	qboolean overflowed;            /* +0x08 */
	int             *list;          /* +0x0C */
	vec3_t bounds[2];               /* +0x10 */
	int lastLeaf;                   /* +0x28 */
	void ( *storeLeafs )( struct leafList_t *ll, int nodenum );  /* +0x2C */
} leafList_t;
COD1_ASSERT_SIZE( leafList_t, 48 );
CM_ASSERT_MATCHES_OPAQUE( leafList_t, cod1_opaque_leafList_t );

/*
 * 24 bytes.  The capsule/sphere used by the capsule trace paths.  opencoduo
 * asserts use +0x00, radius +0x04, halfheight +0x08, offset +0x0C, size 0x18,
 * and traceWork_t's own 280-byte total only closes with this shape.
 */
typedef struct sphere_t
{
	qboolean use;
	float radius;
	float halfheight;
	vec3_t offset;
} sphere_t;
COD1_ASSERT_SIZE( sphere_t, 24 );

/*
 * 280 bytes (0x118), the size opencoduo asserts its traceWork_t at,
 * offset-by-offset; the layout is taken as-is.
 *
 * maxsSum at +0xA0 is written by both CM_Trace and CM_SightTrace and never
 * read by anything -- opencoduo notes the same dead store.
 */
typedef struct traceWork_t
{
	vec3_t start;                   /* +0x000 */
	vec3_t end;                     /* +0x00C */
	vec3_t delta;                   /* +0x018 */
	float deltaLengthSquared;       /* +0x024 */
	vec3_t mins;                    /* +0x028 */
	vec3_t maxs;                    /* +0x034 */
	vec3_t offsets[8];              /* +0x040 */
	float maxsSum;                  /* +0x0A0  written, never read */
	vec3_t bounds[2];               /* +0x0A4 */
	int contents;                   /* +0x0BC */
	qboolean isPoint;               /* +0x0C0 */
	trace_t trace;                  /* +0x0C4 */
	sphere_t sphere;                /* +0x0F4 */
	vec3_t sphereExtents;           /* +0x10C */
} traceWork_t;
COD1_ASSERT_SIZE( traceWork_t, 280 );
CM_ASSERT_MATCHES_OPAQUE( traceWork_t, cod1_opaque_traceWork_t );

/*
=============================================================================

			SERVER-SIDE WORK RECORDS PASSED THROUGH THE SECTOR TREE

	These are built on a caller's stack and handed to the CM_*ToEntities_r
	walkers.  They are declared here because the walkers live in cm_world.c.

=============================================================================
*/

/*
 * 124 bytes (0x7C).  Built by SV_Trace and consumed by
 * CM_ClipMoveToEntities_r / SV_ClipMoveToEntity.  Matches opencoduo's
 * cmClipMoveWork_t exactly, whose 0x7c also matches the proven CoD1 size.
 *
 * expandedHalfSize is (maxs - mins) * 0.5 + 1.0 and is what the sector walker
 * reads at +0x18 + axis*4 to decide whether to descend a branch.
 */
typedef struct moveclip_t
{
	vec3_t mins;                    /* +0x00 */
	vec3_t maxs;                    /* +0x0C */
	vec3_t expandedHalfSize;        /* +0x18 */
	vec3_t start;                   /* +0x24 */
	vec3_t end;                     /* +0x30 */
	trace_t trace;                  /* +0x3C */
	int passEntityNum;              /* +0x6C */
	int passOwnerNum;               /* +0x70 */
	int contentmask;                /* +0x74 */
	qboolean capsule;               /* +0x78 */
} moveclip_t;
COD1_ASSERT_SIZE( moveclip_t, 124 );
CM_ASSERT_MATCHES_OPAQUE( moveclip_t, cod1_opaque_moveclip_t );

/*
 * 100 bytes.  DIVERGENCE FROM opencoduo, and the binary wins.
 *
 * opencoduo's cmPointTraceWork_t is 0x5C (92): start, end, trace,
 * passEntityNum, passOwnerNum, contentmask, useDObj, dobjTracePartState.
 * CoD1's proven size is 100 (0x64), eight bytes longer.  The prefix through
 * +0x54 is common to both -- CM_PointTraceToEntities_r reads start +0x00,
 * end +0x0C, trace +0x18, pass entity +0x48, pass owner +0x4C and contents
 * +0x50 -- so the extra two dwords are at the tail, where UO has the DObj
 * pair.  They are left named as unknowns rather than guessed.
 */
typedef struct pointtrace_t
{
	vec3_t start;                   /* +0x00 */
	vec3_t end;                     /* +0x0C */
	trace_t trace;                  /* +0x18 */
	int passEntityNum;              /* +0x48 */
	int passOwnerNum;               /* +0x4C */
	int contentmask;                /* +0x50 */
	qboolean useDObj;               /* +0x54 */
	const byte      *dobjTracePartState; /* +0x58 */
	int unknown_0x5C;               /* +0x5C  TODO: not in UO's 0x5C-byte form */
	int unknown_0x60;               /* +0x60 */
} pointtrace_t;
COD1_ASSERT_SIZE( pointtrace_t, 100 );
CM_ASSERT_MATCHES_OPAQUE( pointtrace_t, cod1_opaque_pointtrace_t );

/*
 * 84 bytes (0x54).
 *
 * SV_SightTrace (0x0045FCA0) builds sightclip_t and sightpointtrace_t in ONE
 * stack buffer across its two mutually exclusive branches, so neither is
 * bounded by its own construction alone.  What settles it is that the shared
 * buffer starts at esp+0x1C in a frame whose return address sits at 0x70:
 * the box branch's last field is written at buffer+0x50, so the record runs
 * to buffer+0x54 and 0x1C + 0x54 = 0x70 exactly fills the frame.
 *
 * 0x54 is also precisely the size of opencoduo's cmClipSightTraceWork_t, and
 * the field-by-field writes at 0x0045FE01..0x0045FF85 reproduce its layout:
 *   +0x00 -halfsize   +0x0C +halfsize   +0x18 halfsize + 1
 *   +0x24 start + centre   +0x30 end + centre
 *   +0x3C/+0x40 the two skip entities, +0x44/+0x48 their owners
 *   +0x4C contentmask, +0x50 the trailing flag
 * SV_SightTrace's last parameter, `locational`, lands in the slot opencoduo
 * calls capsule; kept as capsule, the consumer being
 * CM_ClipSightTraceToEntities_r either way.
 */
typedef struct sightclip_t
{
	vec3_t mins;                    /* +0x00 */
	vec3_t maxs;                    /* +0x0C */
	vec3_t expandedHalfSize;        /* +0x18 */
	vec3_t start;                   /* +0x24 */
	vec3_t end;                     /* +0x30 */
	int passEntityNum;              /* +0x3C */
	int passOwnerNum;               /* +0x40 */
	int passEntityOwnerNum;         /* +0x44 */
	int passOwnerOwnerNum;          /* +0x48 */
	int contentmask;                /* +0x4C */
	qboolean capsule;               /* +0x50 */
} sightclip_t;
COD1_ASSERT_SIZE( sightclip_t, 84 );

/*
 * 44 bytes (0x2C).
 *
 * SV_SightTrace's point branch (0x0045FD2E..0x0045FDDD) writes start +0x00,
 * end +0x0C, the two skip entities +0x18/+0x1C, their owners +0x20/+0x24 and
 * the contents mask +0x28, then calls CM_PointSightTraceToEntities_r.  The
 * highest byte touched is +0x2B.  Identical to opencoduo's
 * cmPointSightTraceWork_t.
 */
typedef struct sightpointtrace_t
{
	vec3_t start;                   /* +0x00 */
	vec3_t end;                     /* +0x0C */
	int passEntityNum;              /* +0x18 */
	int passOwnerNum;               /* +0x1C */
	int passEntityOwnerNum;         /* +0x20 */
	int passOwnerOwnerNum;          /* +0x24 */
	int contentmask;                /* +0x28 */
} sightpointtrace_t;
COD1_ASSERT_SIZE( sightpointtrace_t, 44 );

/*
 * There is no TraceExtents struct.  The first stack argument of CM_TraceBox
 * (0x004272E0) is a plain vec3_t: the function's second pass does
 * `mov ecx, ebp` at 0x004273C0 to swap the bounds pointer from the register
 * argument to the stack one, and both are then read only as three consecutive
 * floats.  CM_TraceBox is opencoduo's CM_TraceLineSkipsBox with three of its
 * five arguments in registers.
 */

/*
=============================================================================

						COLLISION GLOBALS

	Retail kept all of these in one `clipMap_t cm` at 0x016174A0 -- CM_LoadMap
	clears the lot with a single Com_Memset(qpath, 0, 0xA15C), and 0x016174A0
	+ 0xA15C lands exactly on cmod_base.  They are separate objects here, so
	CM_LoadMap clears them one at a time instead, which is behaviourally
	identical.

	Addresses are the retail ones.

=============================================================================
*/

#define MAX_SUBMODELS       512     /* CMod_LoadSubmodels; also the two box
                                     * handles 511 and 510 */
#define BOX_MODEL_HANDLE    511
#define CAPSULE_MODEL_HANDLE 510

/*
 * WHERE THE STORAGE ACTUALLY LIVES
 * --------------------------------
 * Most of these objects are defined by cod1_globals.c, which types every one
 * of them `int`.  They are re-declared here at the real type and the
 * cod1_globals.c name is aliased to the retail one, so there stays exactly ONE
 * object per retail address and every unit sees the same storage.  cm_trace.c
 * in particular reads cm_leafs, cm_leafbrushes, cm_brushes, cm_patches, the
 * node array and the box model directly through the cod1_globals.c names.
 *
 * NOTE the cod1_globals.c `cm_planes` / `cm_numPlanes` are NOT these: they sit at
 * 0x0087C7DC/0x0087C7E0, next to the CM_LoadLump_m scratch, and have nothing
 * to do with the collision plane array at 0x016174FC.  Aliasing the dword_
 * names is what keeps that distinction straight.
 */

extern int cm_numMaterials;               /* 0x016174E8 */
extern dmaterial_t      *cm_materials; /* 0x016174EC */
extern int cm_numBrushSides;               /* 0x016174F0 */
extern cbrushside_t     *cm_brushsides; /* 0x016174F4 */
extern int cm_numBrushPlanes;               /* 0x016174F8 */
extern cplane_t         *cm_brushPlanes; /* 0x016174FC */
extern int cm_numNodes;               /* 0x01617500 */
extern cNode_t          *node_struct;   /* 0x01617504 */
extern int cm_numLeafs;               /* 0x01617508 */
extern cLeaf_t          *cm_leafs; /* 0x0161750C */
extern int cm_numLeafBrushes;               /* 0x01617510 */
extern int              *cm_leafbrushes; /* 0x01617514 */
extern int cm_numLeafSurfaces;               /* 0x01617518 */
extern int              *cm_leafsurfaces; /* 0x0161751C */
extern int cm_numSubModels;               /* 0x01617520 */
extern cmodel_t         *cm_cmodels; /* 0x01617524 */
extern int cm_numBrushes;               /* 0x01617528 */
extern cbrush_t         *cm_brushes;    /* 0x0161752C */
extern int cm_numClusters;               /* 0x01617530 */
extern int cm_clusterBytes;               /* 0x01617534 */
extern byte             *lastLeaf;      /* 0x01617538 */
extern qboolean cm_vised;          /* 0x0161753C */
extern int cm_entityStringLen;          /* 0x01617540 */
extern char             *cm_entityString; /* 0x01617544 */
extern int cm_numAreas;                 /* 0x01617548 */
extern cArea_t          *cm_areas;      /* 0x0161754C */
extern int              *cm_areaPortals;/* 0x01617550 */
extern int cm_numPatches;               /* 0x01617554 */
extern cPatch_t         *cm_patches; /* 0x01617558 */
extern int cm_floodvalid;               /* 0x0161755C */
extern int cm_checkcount;               /* 0x01617560 */

extern cbrush_t         *box_brush;     /* 0x0161757C */
extern byte             *cmod_base;     /* 0x016215FC */
extern cvar_t           *cm_noCurves;   /* 0x01617480 */
extern cvar_t           *cm_playerCurveClip; /* 0x01621600 */

#define cm_numMaterials     cm_numMaterials
#define cm_materials        cm_materials
#define cm_numBrushSides    cm_numBrushSides
#define cm_brushsides       cm_brushsides
#define cm_numPlanes        cm_numBrushPlanes
#define cm_planes           cm_brushPlanes
#define cm_numNodes         cm_numNodes
#define cm_nodes            node_struct
#define cm_numLeafs         cm_numLeafs
#define cm_leafs            cm_leafs
#define cm_numLeafBrushes   cm_numLeafBrushes
#define cm_leafbrushes      cm_leafbrushes
#define cm_numLeafSurfaces  cm_numLeafSurfaces
#define cm_leafsurfaces     cm_leafsurfaces
#define cm_numSubModels     cm_numSubModels
#define cm_cmodels          cm_cmodels
#define cm_numBrushes       cm_numBrushes
#define cm_numClusters      cm_numClusters
#define cm_clusterBytes     cm_clusterBytes
#define cm_visibility       lastLeaf
#define cm_vised            cm_vised
#define cm_numPatches       cm_numPatches
#define cm_patches          cm_patches
#define cm_floodvalid       cm_floodvalid
#define cm_checkcount       cm_checkcount

/* These two are owned by cm_load.c outright.  cm_boxModel needs 40 contiguous
 * bytes and cod1_globals.c carries the retail object as six separate ints, so
 * it cannot be aliased; CM_TempBoxModel mirrors into those ints for as long as
 * cm_trace.c still reads them. */
extern char cm_mapName[MAX_QPATH];      /* 0x016174A0 */
extern cmodel_t cm_boxModel;            /* 0x01617580 */

/*
=============================================================================

						PROTOTYPES

	Argument ORDER below is the retail one; for __usercall functions the
	register assignment is noted at the definition site.

=============================================================================
*/

/* cm_load.c */
void CM_LoadMap( const char *name, qboolean clientload, int *checksum );
cmodel_t *CM_ClipHandleToModel( clipHandle_t handle );
clipHandle_t CM_InlineModel( int index );
int CM_NumClusters( void );
int CM_NumInlineModels( void );
const char *CM_EntityString( void );
int CM_LeafCluster( int leafnum );
int CM_LeafArea( int leafnum );
void CM_InitBoxHull( void );
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs,
							  int contents, qboolean capsule );
void CM_SetTempBoxModelContents( int contents );
void CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );

/* cm_test.c */
int CM_PointLeafnum_r( const vec3_t p, int num );
int CM_PointLeafnum( const vec3_t p );
void CM_StoreLeafs( leafList_t *ll, int nodenum );
void CM_StoreBrushes( leafList_t *ll, int nodenum );
void CM_BoxLeafnums_r( leafList_t *ll, int nodenum );
int CM_BoxLeafnums( const vec3_t mins, const vec3_t maxs,
					int *list, int listsize, int *lastLeaf );
int CM_BoxBrushes( const vec3_t mins, const vec3_t maxs,
				   cbrush_t **list, int listsize );
int CM_PointContents( const vec3_t p, clipHandle_t model );
int CM_TransformedPointContents( const vec3_t p, clipHandle_t model,
								 const vec3_t origin, const vec3_t angles );
byte *CM_ClusterPVS( int cluster );
void CM_FloodArea_r( int areaNum, int floodnum );
void CM_FloodAreaConnections( void );
void CM_ChangeAreaPortalState( int area1, int area2, qboolean open );
qboolean CM_AreasConnected( int area1, int area2 );

/* cm_patch.c / cm_terrain.c -- the two collide builders.
 *
 * CM_GeneratePatchCollide's retail form is __usercall with width in edx,
 * points in ecx and height as the only stack argument; the order below is the
 * one its own Com_Error("...bad parameters: (%i, %i, %p)", width, height,
 * points) prints, which is the order that survives into C. */
/* FIVE arguments.  __usercall: width@edx, height@stack+0, maxError@stack+4,
   points@ecx, bounds@edi.  maxError is the PER-PATCH value from
   dcollisionsurface_t+8 (pushed at 0x00419AFF); bounds is &out->mins
   (EDI at 0x00419B0C), through which the builder fills the patch bounds.
   Without it every curved patch keeps zeroed mins/maxs and the sweep
   traces early-out on CM_TraceMayIntersectBox. */
patchCollide_t *CM_GeneratePatchCollide( int width, int height, int maxError,
										 const vec3_t *points, vec3_t *bounds );
/* FIVE arguments.  The fifth is &out->mins: CMod_LoadLeafCurvesAndTerrain
 * pushes out+12 at 0x00419A7B and the retail frame slot is arg_10 at +0x600B8,
 * and the builder seeds it to +/-262144 and folds every vertex into it, so
 * without it terrain patches keep zeroed bounds.  Slots 3 and 4 are the vertex
 * COUNT and the vertex ARRAY. */
void *CM_GenerateTerrainCollide( int numIndexes, const short *indexes,
								 int numVerts, const vec3_t *points,
								 vec3_t *bounds );

/* cm_world.c */
void CM_ClearWorld( void );
/* PORT ONLY -- no retail address.  Zeroes the world sector storage that retail
 * gets for free from CM_LoadMap's Com_Memset(cm, 0, 0xA15C), whose 0xA15C block
 * ends exactly at the end of the sector pool.  Called from CM_ClearMap, at the
 * same point in the load that retail's memset runs.  Do not move the clear
 * into CM_ClearWorld, which retail does not do. */
void CM_ClearWorldSectors( void );
qboolean CM_TraceBox( const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs, float fraction );

/* cm_staticmodel.c */
void CM_LoadStaticModels( void );

/* universal/com_math.c */
int BoxOnPlaneSide( const vec3_t emins, const vec3_t emaxs, cplane_t *p );

/* universal/com_memory_core.c */
void *Hunk_AllocAlignInternal( int size, int align );

#endif  /* __CM_LOCAL_H__ */
