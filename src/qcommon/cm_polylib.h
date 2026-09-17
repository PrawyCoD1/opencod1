/*
 * cm_polylib.h -- the winding record for Call of Duty 1.1 (Windows, CoDMP.exe).
 *
 * RTCW has a qcommon/cm_polylib.h and declares winding_t in it; this is the
 * counterpart.  Two units touch a winding: qcommon/cm_polylib.c, which owns
 * the whole API, and qcommon/cm_patch.c, which builds and clips them.  Both
 * include this.  Only the type lives here; the polylib prototypes are
 * declared at their use sites.
 *
 * @fidelity: verified
 */

#ifndef __CM_POLYLIB_H__
#define __CM_POLYLIB_H__

#include "../universal/q_shared.h"

/* NOTE, from cm_polylib.c: windings are variable length.  The declared p[4] is
 * the minimum the retail record is asserted at, not a bound -- AllocWinding
 * sizes each one as sizeof(int) + points * sizeof(vec3_t). */
typedef struct winding_t
{
	int numpoints;
	vec3_t p[4];
} winding_t;
COD1_ASSERT_SIZE( winding_t, 52 );

#endif  /* __CM_POLYLIB_H__ */
