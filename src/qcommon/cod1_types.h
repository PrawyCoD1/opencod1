/*
 * cod1_types.h -- size-only placeholders for the engine records whose real
 * layouts live in qcommon/cm_local.h, qcommon/cm_patch_local.h and
 * server_mp/sv_world_mp.c.  Everything else the retail build declared lives
 * in universal/q_shared.h, qcommon/qcommon.h and server_mp/server.h; the
 * static-assertion macros are in q_shared.h, which this header includes.
 *
 * @fidelity-default: verified
 */

#ifndef __COD1_TYPES_H__
#define __COD1_TYPES_H__

#include "../universal/q_shared.h"


/*
=============================================================================

			SIZE WITNESSES

	Opaque blobs of a known size.  The size is real: it comes from how the
	retail code indexes arrays of them.  Do not add members to them.

	The real layouts live elsewhere -- collision in qcommon/cm_local.h and
	qcommon/cm_patch_local.h, the four clip records in
	server_mp/sv_world_mp.c -- and each of those files renames the placeholder
	below aside for the duration of its qcommon.h include and then asserts its
	real layout against sizeof() the placeholder.  So the witness has to stay
	reachable from BOTH the qcommon layer and the server layer, which is this
	header.  They go away in one step when the real layouts are promoted into
	their own headers.

=============================================================================
*/

#define COD1_OPAQUE_TYPE( name, bytes )	\
	typedef struct name { byte opaque[bytes]; } name; \
	COD1_ASSERT_SIZE( name, bytes )

COD1_OPAQUE_TYPE( traceWork_t,       280 );
COD1_OPAQUE_TYPE( cmodel_t,          40 );
COD1_OPAQUE_TYPE( cbrush_t,          52 );
COD1_OPAQUE_TYPE( cLeaf_t,           16 );
COD1_OPAQUE_TYPE( cPatch_t,          44 );
COD1_OPAQUE_TYPE( patchCollide_t,    16 );
COD1_OPAQUE_TYPE( leafList_t,        48 );
COD1_OPAQUE_TYPE( moveclip_t,        124 );
COD1_OPAQUE_TYPE( pointtrace_t,      100 );
COD1_OPAQUE_TYPE( cStaticModel_t,    84 );
COD1_OPAQUE_TYPE( facet_t,           320 );

/* Sizes from the construction site in SV_SightTrace (0x0045FCA0): the shared
 * buffer sits at esp+0x1C, the box branch's last store lands at buffer+0x50,
 * so the record ends at buffer+0x54 -- and 0x1C + 0x54 = 0x70 fills the frame
 * exactly.  0x54 is also precisely opencoduo's cmClipSightTraceWork_t, whose
 * field order the writes at 0x45FE01-0x45FF85 reproduce one for one.  Real
 * layouts are in qcommon/cm_local.h.
 */
COD1_OPAQUE_TYPE( sightclip_t,       84 );
COD1_OPAQUE_TYPE( sightpointtrace_t, 44 );


#endif  /* __COD1_TYPES_H__ */
