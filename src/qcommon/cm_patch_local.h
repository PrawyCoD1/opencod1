/*
 * cm_patch_local.h -- the curved-patch collision record layouts for
 * Call of Duty 1.1 (Windows, CoDMP.exe).
 *
 * qcommon/cod1_types.h still declares patchCollide_t (16 bytes) and facet_t
 * (320 bytes) as opaque blobs, and qcommon/cm_local.h passes them around by
 * pointer without ever looking inside.  Both of those are shared headers, so
 * the real layouts live here instead and the opaque names are renamed aside
 * for the duration of the qcommon.h include -- exactly the pattern cm_local.h
 * uses for cbrush_t and friends.
 *
 * Include this INSTEAD of (and before) qcommon.h.  When these layouts move
 * into cod1_types.h the whole #define/#undef block below is deleted in one
 * step and no .c file changes.
 *
 * Consumers: qcommon/cm_patch.c, qcommon/cm_terrain.c.
 *
 * ---------------------------------------------------------------------------
 * EVIDENCE
 * ---------------------------------------------------------------------------
 *
 * These three records are Q3's, inherited essentially unchanged, and three
 * independent sources agree:
 *
 *  1. The sizes cod1_types.h already proves from array strides:
 *         patchCollide_t  16      facet_t  320 (0x140)   patchPlane_t  20
 *
 *  2. opencoduo (coduo-server-master/dedicated-engine/src/physics_collision/
 *     cm_terrain_private.h) asserts exactly these three shapes at exactly
 *     these sizes, field by field, against the CoD:UO Linux binary.  UO is the
 *     same engine revision and loads CoD1 .bsp files, so the in-memory
 *     collision records are necessarily the same shape.
 *
 *  3. The CoD 1.1 access WIDTHS and OFFSETS:
 *
 *     CM_PositionTestInPatchCollide (0x0041DB90)
 *         dword [pc + 0x08]                     numFacets
 *         dword [pc + 0x0C]                     facets
 *         facet stride 320 (`v5 += 320`)        sizeof(facet_t)
 *         dword [facet + 0x04]                  numBorders
 *         dword walk from facet + 0x70, step 4  borderInward[26]
 *
 *     CM_SightTracePointThroughPatchCollide (0x0041CE60) walks the same two
 *     arrays with the same strides, and CM_CheckFacetPlane (0x0041D0E0) is
 *     handed `planes[n].plane` as a bare float* -- i.e. the four floats at
 *     patchPlane_t + 0x00, with signbits following at + 0x10.
 *
 *     borderPlanes at +0x08 is what forces borderInward to +0x70: 26 dwords is
 *     104 bytes and 0x08 + 104 = 0x70 exactly.  borderNoAdjust then starts at
 *     0x70 + 104 = 0xD8 and the record closes at 0xD8 + 104 = 0x140 = 320 with
 *     NO trailing padding, which is the proven size.  There is no gap anywhere
 *     in the record, so no _pad member is needed.
 *
 *     The three arrays are dword-wide at every site observed; borderInward and
 *     borderNoAdjust are qboolean (int), not byte.  CM_PositionTestInPatchCollide
 *     steps borderInward by 4 (`v12 += 4`), which settles it.
 *
 * NOTE: opencoduo's file naming is the reverse of
 * what you would expect: its cm_terrain_private.h holds THESE records (the
 * Q3-style planes/facets pair built by CM_GeneratePatchCollide from a width x
 * height control grid), while its cm_patch_private.h holds the *other* CoD
 * collide -- a facetCount header followed by inline 0x48-byte facets carrying
 * vertex spheres and edge cylinders, built by CM_GenerateTerrainCollide from an
 * index/vertex list.  CoD1 splits them the same way but names the files the
 * other way round: qcommon/cm_patch.c builds the records below and
 * qcommon/cm_terrain.c builds the sphere/cylinder form.  Do not cross them.
 *
 * @fidelity: verified
 */

#ifndef __CM_PATCH_LOCAL_H__
#define __CM_PATCH_LOCAL_H__

/* Rename cod1_types.h's opaque placeholders aside.  Delete this block, the
 * matching #undef block and the two typedefs below once the real layouts live
 * in cod1_types.h. */
#define patchCollide_t      cod1_opaque_patchCollide_t
#define facet_t             cod1_opaque_facet_t

#include "qcommon.h"

#undef patchCollide_t
#undef facet_t

/* patchPlane_t lives here, where RTCW keeps it (qcommon/cm_patch.h).  It is
 * the same record as opencoduo's { vec3_t normal; float dist; uint32 signbits; }.
 * The spelling below is the retail one; keep using patchPlane_t.plane as a
 * float[4] so CM_CheckFacetPlane's bare-float* parameter stays honest. */

typedef struct patchPlane_t
{
	float plane[4];
	int signbits;
} patchPlane_t;
COD1_ASSERT_SIZE( patchPlane_t, 20 );

#define MAX_PATCH_PLANES        4096    /* CM_FindPlane2's overflow guard */
#define MAX_FACET_BORDERS       26      /* borderPlanes[] bound; 26*4 = 104 */

typedef struct facet_s
{
	int surfacePlane;                           /* +0x000 */
	int numBorders;                             /* +0x004  <= MAX_FACET_BORDERS */
	int borderPlanes[MAX_FACET_BORDERS];        /* +0x008 */
	qboolean borderInward[MAX_FACET_BORDERS];   /* +0x070 */
	qboolean borderNoAdjust[MAX_FACET_BORDERS]; /* +0x0D8 */
} facet_t;                                      /*  =0x140 */
COD1_ASSERT_SIZE( facet_t, 320 );
COD1_STATIC_ASSERT( sizeof( facet_t ) == sizeof( cod1_opaque_facet_t ),
					"facet_t must match the proven size of cod1_opaque_facet_t" );

typedef struct patchCollide_s
{
	int numPlanes;                  /* +0x00 */
	patchPlane_t    *planes;        /* +0x04 */
	int numFacets;                  /* +0x08 */
	facet_t         *facets;        /* +0x0C */
} patchCollide_t;                   /*  =0x10 */
COD1_ASSERT_SIZE( patchCollide_t, 16 );
COD1_STATIC_ASSERT( sizeof( patchCollide_t ) == sizeof( cod1_opaque_patchCollide_t ),
					"patchCollide_t must match the proven size of cod1_opaque_patchCollide_t" );

/*
 * cm_local.h declares CM_GeneratePatchCollide as returning the OPAQUE
 * patchCollide_t*, and cPatch_t.pc is an opaque pointer too.  Units that
 * include this header therefore need one cast at each boundary; it disappears
 * with the #define block above.
 */
#define CM_PC( p )      ( (patchCollide_t *) (p) )
#define CM_PC_OPAQUE( p ) ( (cod1_opaque_patchCollide_t *) (p) )

/*
 * THE PATCH PLANE POOL -- `patchPlane_t cm_patchPlanes[4096]` at 0x0087C7E0,
 * `cm_numPatchPlanes` at 0x0087C7DC; both are defined in cm_patch.c as
 * uninitialised file-scope objects.  CM_FindPlane2 and CM_FindPlane both
 * refuse to append at the literal 4096, and 4096 * 20 lands exactly on the
 * next symbol flt_8907E0.
 *
 * cm_planes / cm_numPlanes in cm_load.c and cm_world.c are a DIFFERENT object --
 * the BSP plane array at 0x016174FC / 0x016174F8, aliased in cm_local.h.  Do not
 * cross them.
 */

#endif  /* __CM_PATCH_LOCAL_H__ */
