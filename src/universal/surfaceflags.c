/*
 * universal/surfaceflags.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/surfaceflags.c
 *
 * Retail range 0x004390F0-0x0043917.
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"

#include <string.h>

typedef struct {
	const char  *name;
	int nonOpaque;
	int surfaceFlags;
} surfaceType_t;

static surfaceType_t surfaceTypes[] = {
	{ "bark",        0, 0x00100000 },
	{ "brick",       0, 0x00200000 },
	{ "carpet",      0, 0x00300000 },
	{ "cloth",       0, 0x00400000 },
	{ "concrete",    0, 0x00500000 },
	{ "dirt",        0, 0x00600000 },
	{ "flesh",       0, 0x00700000 },
	{ "foliage",     1, 0x00800000 },
	{ "glass",       1, 0x00900000 },
	{ "grass",       0, 0x00A00000 },
	{ "gravel",      0, 0x00B00000 },
	{ "ice",         0, 0x00C00000 },
	{ "metal",       0, 0x00D00000 },
	{ "mud",         0, 0x00E00000 },
	{ "paper",       0, 0x00F00000 },
	{ "plaster",     0, 0x01000000 },
	{ "rock",        0, 0x01100000 },
	{ "sand",        0, 0x01200000 },
	{ "snow",        0, 0x01300000 },
	{ "water",       0, 0x01400000 },
	{ "wood",        0, 0x01500000 },
	{ "asphalt",     0, 0x01600000 },
	{ "opaqueglass", 0, 0x00900000 },
};

#define NUM_SURFACE_TYPES ( sizeof( surfaceTypes ) / sizeof( surfaceTypes[0] ) )

#define NUM_MATCHABLE_SURFACE_TYPES ( NUM_SURFACE_TYPES - 1 )

/* ---- Com_SurfaceTypeFromName  0x004390F0 ---- */
int Com_SurfaceTypeFromName( const char *name ) {
	int i;

	if ( !Q_stricmp( name, "default" ) ) {
		return 0;
	}

	for ( i = 0 ; i < NUM_MATCHABLE_SURFACE_TYPES ; i++ ) {
		if ( !Q_stricmp( name, surfaceTypes[i].name ) ) {
			return ( surfaceTypes[i].surfaceFlags >> 20 ) & 0x1F;
		}
	}

	return -1;
}

/* ---- Com_SurfaceTypeToName  0x00439150 ---- */
const char *Com_SurfaceTypeToName( int type ) {
	if ( type <= 0 || type > NUM_SURFACE_TYPES ) {
		return "default";
	}
	return surfaceTypes[type - 1].name;
}

/* ---- Com_AddToString  0x00439170 ---- */
int Com_AddToString( const char *src, char *msg, int len, int maxlen, int mayAddQuotes ) {
	int addQuotes = 0;
	int i;

	if ( mayAddQuotes ) {
		if ( !src[0] ) {
			addQuotes = 1;
		} else {
			for ( i = 0 ; i < maxlen - len && src[i] ; i++ ) {
				if ( src[i] <= ' ' ) {
					addQuotes = 1;
					break;
				}
			}
		}
	}

	if ( addQuotes && len < maxlen ) {
		msg[len++] = '"';
	}

	while ( *src && len < maxlen ) {
		msg[len++] = *src++;
	}

	if ( addQuotes && len < maxlen ) {
		msg[len++] = '"';
	}

	return len;
}
