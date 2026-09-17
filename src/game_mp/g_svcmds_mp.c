/*
 * @fidelity: likely
 *
 * g_svcmds_mp.c -- the console commands the server hands the game module.
 *
 * This is RTCW's game/g_svcmds.c with the forceteam / game_memory commands
 * dropped and the say fall-through folded into one call: ConsoleCommand
 * (0x20038590) dispatches through a plain compare chain, not a table.  The
 * whole IP-ban half survives unchanged, including ClientForString, which 1.1
 * keeps but never calls -- forceteam was its only caller in RTCW.
 *
 * Function order is binary order (0x20037E80 .. 0x20038590).
 */

#include <stdlib.h>
#include <string.h>

#include "g_local.h"

/* Q_strncpyz's buffer and trap_Argv's; RTCW's MAX_TOKEN_CHARS. */
#define MAX_TOKEN_CHARS         1024

/* UpdateIPBans' iplist buffer (0x20037F40 reserves 0x400). */
#define MAX_INFO_STRING         1024

/* trap_SendConsoleCommand's exec_when; g_main_mp.c carries EXEC_APPEND 2. */
#ifndef EXEC_INSERT
#define EXEC_INSERT             1
#endif

/* entityState_t.eType, from Svcmd_EntityList_f's own switch.  Value 2 has no
 * case of its own in 1.1.  g_utils_mp.c carries the same subset. */
#define ET_GENERAL              0
#define ET_PLAYER               1
#define ET_ITEM                 3
#define ET_MISSILE              4
#define ET_MOVER                5
#define ET_PORTAL               6
#define ET_INVISIBLE            7
#define ET_SCRIPTMOVER          8

/* g_filterBan is NOT registered in 1.1 -- G_FilterPacket is the only thing
   that ever touches it and only reads .integer, so it stays 0 and the ban
   list is effectively inverted (reproduced).  Nothing else in the module
   references it, so the object lives here rather than in G_RegisterCvars'
   table, which does not carry a row for it. */
vmCvar_t g_filterBan;                           /* 0x2016F100 */

typedef struct ipFilter_s {
	unsigned mask;
	unsigned compare;
} ipFilter_t;

#define MAX_IPFILTERS           1024

static ipFilter_t   ipFilters[MAX_IPFILTERS];       /* 0x20089F40 */
static int          numIPFilters;                   /* 0x20089F3C */

/*
==================
StringToFilter

RTCW's, minus the '*' wildcard branch -- 1.1 rejects anything that is not a
digit outright.
==================
*/
static qboolean StringToFilter( char *s, ipFilter_t *f ) {
	char    num[128];
	int     i, j;
	byte    b[4];
	byte    m[4];

	for ( i = 0; i < 4; i++ ) {
		b[i] = 0;
		m[i] = 0;
	}

	for ( i = 0; i < 4; i++ ) {
		if ( *s < '0' || *s > '9' ) {
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}

		j = 0;
		while ( *s >= '0' && *s <= '9' ) {
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi( num );
		if ( b[i] != 0 ) {
			m[i] = 255;
		}

		if ( !*s ) {
			break;
		}
		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return qtrue;
}

/*
==================
UpdateIPBans
==================
*/
static void UpdateIPBans( void ) {
	byte    b[4];
	int     i;
	char    iplist[MAX_INFO_STRING];

	*iplist = 0;
	for ( i = 0; i < numIPFilters; i++ ) {
		if ( ipFilters[i].compare == 0xffffffff ) {
			continue;
		}

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen( iplist ), sizeof( iplist ) - strlen( iplist ),
					 "%i.%i.%i.%i ", b[0], b[1], b[2], b[3] );
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}

/*
==================
G_FilterPacket
==================
*/
qboolean G_FilterPacket( char *from ) {
	int         i;
	unsigned    in;
	byte        m[4];
	char        *p;

	i = 0;
	p = from;
	while ( *p && i < 4 ) {
		m[i] = 0;
		while ( *p >= '0' && *p <= '9' ) {
			m[i] = m[i] * 10 + ( *p - '0' );
			p++;
		}
		if ( !*p || *p == ':' ) {
			break;
		}
		i++, p++;
	}

	in = *(unsigned *)m;

	for ( i = 0; i < numIPFilters; i++ ) {
		if ( ( in & ipFilters[i].mask ) == ipFilters[i].compare ) {
			return g_filterBan.integer != 0;
		}
	}

	return g_filterBan.integer == 0;
}

/*
==================
AddIP
==================
*/
static void AddIP( char *str ) {
	int i;

	for ( i = 0; i < numIPFilters; i++ ) {
		if ( ipFilters[i].compare == 0xffffffff ) {
			break;              // free spot
		}
	}
	if ( i == numIPFilters ) {
		if ( numIPFilters == MAX_IPFILTERS ) {
			G_Printf( "IP filter list is full\n" );
			return;
		}
		numIPFilters++;
	}

	if ( !StringToFilter( str, &ipFilters[i] ) ) {
		ipFilters[i].compare = 0xffffffffu;
	}

	UpdateIPBans();
}

/*
==================
G_ProcessIPBans
==================
*/
void G_ProcessIPBans( void ) {
	char *s, *t;
	char str[MAX_TOKEN_CHARS];

	numIPFilters = 0;

	Q_strncpyz( str, g_banIPs.string, sizeof( str ) );

	for ( t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr( s, ' ' );
		if ( !s ) {
			break;
		}
		while ( *s == ' ' ) {
			*s++ = 0;
		}
		if ( *t ) {
			AddIP( t );
		}
		t = s;
	}
}

/*
==================
Svcmd_AddIP_f
==================
*/
static void Svcmd_AddIP_f( void ) {
	char str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage:  addip <ip-mask>\n" );
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );
}

/*
==================
Svcmd_RemoveIP_f
==================
*/
static void Svcmd_RemoveIP_f( void ) {
	ipFilter_t  f;
	int         i;
	char        str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage:  sv removeip <ip-mask>\n" );
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if ( !StringToFilter( str, &f ) ) {
		return;
	}

	for ( i = 0; i < numIPFilters; i++ ) {
		if ( ipFilters[i].mask == f.mask && ipFilters[i].compare == f.compare ) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf( "Removed.\n" );

			UpdateIPBans();
			return;
		}
	}

	G_Printf( "Didn't find %s.\n", str );
}

/*
==================
Svcmd_EntityList_f
==================
*/
void Svcmd_EntityList_f( void ) {
	int         e;
	gentity_t   *check;

	check = g_entities + 1;
	for ( e = 1; e < level.num_entities; e++, check++ ) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf( "%3i:", e );
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf( "ET_GENERAL          " );
			break;
		case ET_PLAYER:
			G_Printf( "ET_PLAYER           " );
			break;
		case ET_ITEM:
			G_Printf( "ET_ITEM             " );
			break;
		case ET_MISSILE:
			G_Printf( "ET_MISSILE          " );
			break;
		case ET_MOVER:
			G_Printf( "ET_MOVER            " );
			break;
		case ET_PORTAL:
			G_Printf( "ET_PORTAL           " );
			break;
		case ET_INVISIBLE:
			G_Printf( "ET_INVISIBLE        " );
			break;
		case ET_SCRIPTMOVER:
			G_Printf( "ET_SCRIPTMOVER      " );
			break;
		default:
			G_Printf( "%3i                 ", check->s.eType );
			break;
		}

		if ( check->classname ) {
			G_Printf( "%s", SL_ConvertToString( check->classname ) );
		}
		G_Printf( "\n" );
	}
}

/*
==================
ClientForString

No caller in 1.1: RTCW reached it from forceteam, which this module does not
have.
==================
*/
gclient_t *ClientForString( const char *s ) {
	gclient_t   *cl;
	int         i;
	int         idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->sess.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i = 0, cl = level.clients; i < level.maxclients; i++, cl++ ) {
		if ( cl->sess.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( s, cl->sess.name ) ) {
			return cl;
		}
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
==================
ConsoleCommand

The dedicated-server fall-through of RTCW's version, folded: "say" and anything
else differ only in where ConcatArgs starts.  \x15 is the localization escape,
so the client renders GAME_SERVER out of its own string table.
==================
*/
qboolean ConsoleCommand( void ) {
	char cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp( cmd, "entitylist" ) == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "addip" ) == 0 ) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "removeip" ) == 0 ) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if ( Q_stricmp( cmd, "listip" ) == 0 ) {
		trap_SendConsoleCommand( EXEC_INSERT, "g_banIPs\n" );
		return qtrue;
	}

	if ( dedicated.integer ) {
		// everything that is not "say" is also printed as a say command
		trap_SendServerCommand( -1, 0, va( "e \"GAME_SERVER\x15: %s\"",
										   ConcatArgs( Q_stricmp( cmd, "say" ) == 0 ) ) );
		return qtrue;
	}

	return qfalse;
}
