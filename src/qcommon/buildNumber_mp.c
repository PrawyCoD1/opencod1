/*
 * qcommon/buildNumber_mp.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/buildNumber_mp.c
 *
 * @fidelity-default: verified
 */

#include "qcommon.h"

#define PRODUCT_BUILD_NUMBER    1413

#ifndef PRODUCT_BUILD_DATE
#define PRODUCT_BUILD_DATE      __DATE__
#endif
#ifndef PRODUCT_BUILD_TIME
#define PRODUCT_BUILD_TIME      __TIME__
#endif

static char buildnum[32];

/* ---- getBuildNumber  0x00401000 ---- */
char *getBuildNumber( void ) {
	sprintf( buildnum, "%d %s %s", PRODUCT_BUILD_NUMBER, PRODUCT_BUILD_DATE, PRODUCT_BUILD_TIME );
	return buildnum;
}
