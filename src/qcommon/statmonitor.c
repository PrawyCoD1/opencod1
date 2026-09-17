/*
 * qcommon/statmonitor.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/statmonitor.c
 *
 * Retail range 0x004519B0-0x00451A7.
 *
 * @fidelity-default: verified
 */

#include "qcommon.h"

extern int (*Material_RegisterHandle)();   /* 0x01432874, indirect call target */

#define MAX_STATMONITOR_ENTRIES 64

static statmonitor_t stats[MAX_STATMONITOR_ENTRIES];
static int statCount;

/* ---- StatMon_Warning  0x004519B0 ---- */
void StatMon_Warning( int type, int duration, const char *materialName ) {
	if ( !com_statmon->integer ) {
		return;
	}

	if ( type < 0 || type >= MAX_STATMONITOR_ENTRIES ) {
		Com_Error( ERR_DROP, "\x15" "StatMon_UpdateEntry: invalid entry '%i'\n", type );
	}

	stats[type].endtime = Sys_Milliseconds() + duration;

	if ( !stats[type].material && cls_rendererStarted ) {
		stats[type].material = Material_RegisterHandle( materialName, 1 );
	}

	if ( type >= statCount ) {
		statCount = type + 1;
	}
}

/* ---- StatMon_GetStatsArray  0x00451A50 ---- */
void StatMon_GetStatsArray( const statmonitor_t **array, int *count ) {
	*array = stats;
	*count = statCount;
}

/* ---- StatMon_Reset  0x00451A60 ---- */
void StatMon_Reset( void ) {
	memset( stats, 0, sizeof( stats ) );
	statCount = 0;
}
