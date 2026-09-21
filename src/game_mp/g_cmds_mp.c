/*
 * @fidelity: likely
 *
 * g_cmds_mp.c -- the client command unit of the CoD 1.1 multiplayer game
 * module (game_mp_x86.dll, imagebase 0x20000000).
 *
 * RTCW's game/g_cmds.c is the ancestor and most of the file is that text with
 * CoD's localized server commands substituted for RTCW's "print" strings: the
 * scoreboard message, the cheat commands, the spectator follow cycle, the
 * say/tell/voice family, callvote/vote (which in RTCW MP already doubles as
 * the complaint handler) and ClientCommand's if-chain.  Cmd_Activate_f and
 * Cmd_MenuResponse_f are CoD's own.
 *
 * The \x14 and \x15 bytes inside the message strings are the localization
 * system's field separators, exactly as they sit in .rdata.
 *
 * Function order is binary order (0x2001D150 .. 0x2001FB60).
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g_local.h"

/*
 * sess.voteCount is read exactly once in the whole module -- Cmd_CallVote_f
 * 0x2001EBB9, `>= MAX_VOTE_COUNT` -- and never written, so the cap is dead.
 * sess.complaints is incremented and tested against g_complaintlimit by
 * Cmd_Vote_f (0x2001F454 / 0x2001F45B / 0x2001F463); it is NOT a teamVoteCount.
 */

/* playerState_t.stats[] indices; g_items_mp.c and g_client_fields_mp.c define
   the same two locally until q_shared.h carries the enum. */
#define STAT_HEALTH             0
#define STAT_MAX_HEALTH         2

#ifndef FL_NOTARGET
#define FL_NOTARGET             0x00000002      /* Cmd_Notarget_f 0x2001DD40 */
#endif

#ifndef EXEC_APPEND
#define EXEC_APPEND             2
#endif

/* Configstrings this unit writes.  CS_FOGVARS: G_setfog (0x2001D638); the
   vote block: Cmd_CallVote_f (0x2001F25E..0x2001F2A7); CS_SCRIPTMENUS:
   Cmd_MenuResponse_f (0x2001FAE2), and 1180 + 32 lands
   exactly on the hint-string block that precedes CS_LOCALIZED_STRINGS 1244. */
#define CS_FOGVARS              12
#define CS_VOTE_TIME            15
#define CS_VOTE_STRING          16
#define CS_VOTE_YES             17
#define CS_VOTE_NO              18
#define CS_SCRIPTMENUS          1180
#define CS_SCRIPTMENUS_COUNT    32

/* playerState_t.pm_flags: PMF_FOLLOW is RTCW's bit unchanged.  0x20 is the
   aim-down-sight flag (bg_pmove.c); StopFollowing drops both
   (0x2001E0BD `and eax, 0FFFEFFDFh`). */
#define PMF_FOLLOW              0x00010000
#define PMF_ADS                 0x00000020

/* playerState_t.eFlags.  EF_VOTED is RTCW's bit (Cmd_CallVote_f 0x2001F243,
   Cmd_Vote_f 0x2001F568).  The 0xC000 pair StopFollowing clears and
   Cmd_Activate_f tests has no recovered name. */
#define EF_VOTED                0x00020000
#define EF_ACTIVATE_MASK        0x0000C000

/* entityState_t.eType values Cmd_Activate_f dispatches on. */
#define ET_TRIGGER              3
#define ET_TURRET               11

/* The content mask StopFollowing's chase-camera trace uses (0x2001DF97).  It
   is bg_pmove.c's MASK_PLAYERSOLID minus CONTENTS_BODY, 0x80 and 0x100 -- the
   same value BG_CheckProneValid uses; no MASK_* name is recovered, so this
   one is inferred. */
#define MASK_SPECTATOR_CAMERA   0x00810011

/* RTCW's chat modes and colour codes, unchanged. */
#define SAY_ALL                 0
#define SAY_TEAM                1
#define SAY_TELL                2

#define Q_COLOR_ESCAPE          '^'
#define COLOR_GREEN             '2'
#define COLOR_YELLOW            '3'
#define COLOR_CYAN              '5'
#define COLOR_MAGENTA           '6'
#define COLOR_WHITE             '7'

#define MAX_SAY_TEXT            150

/* RTCW's cap on how many votes one client may call in a map. */
#define MAX_VOTE_COUNT          3

/* RTCW's VOTE_TIME.  CoD folds it into the store, so level.voteTime holds the
   absolute deadline rather than the moment the vote started -- which is why
   CheckVote (g_main_mp.c) tests `level.time - level.voteTime >= 0` with no
   constant of its own (0x2001F1EF/0x2001F1FC). */
#define VOTE_TIME               30000

/* G_Voice's spam window (0x2001E8AD) and its per-chat charge (0x2001E8F2). */
#define VOICE_CHAT_SQUELCH_MAX  30000
#define VOICE_CHAT_SQUELCH_SPAN 34000

/* Q_stricmp is Q3's `Q_stricmpn( s1, s2, 99999 )`; the retail build inlines it
   everywhere, so the two spellings below are the same call. */

/*
==================
DeathmatchScoreboardMessage
==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char entry[MAX_STRING_CHARS];
	char string[1400];
	int stringlength;
	int i, j;
	gclient_t   *cl;
	int numSorted;
	int ping;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;

	numSorted = level.numConnectedClients;
	if ( numSorted > MAX_CLIENTS ) {
		numSorted = MAX_CLIENTS;
	}

	for ( i = 0 ; i < numSorted ; i++ ) {
		cl = &level.clients[ level.sortedClients[i] ];

		if ( cl->sess.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		Com_sprintf( entry, sizeof( entry ), " %i %i %i %i %i",
					 level.sortedClients[i], cl->sess.score, ping,
					 cl->sess.deaths, cl->sess.statusIcon );

		j = strlen( entry );
		if ( stringlength + j > MAX_STRING_CHARS ) {
			break;
		}
		strcpy( string + stringlength, entry );
		stringlength += j;
	}

	trap_SendServerCommand( ent - g_entities, 1,
							va( "b %i %i %i%s", i, level.teamScores[TEAM_AXIS],
								level.teamScores[TEAM_ALLIES], string ) );
}

/*
==================
Cmd_Score_f
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}

/*
==================
CheatsOk
==================
*/
qboolean CheatsOk( gentity_t *ent ) {
	if ( !sv_cheats.integer ) {
		trap_SendServerCommand( ent - g_entities, 0, va( "e \"GAME_CHEATSNOTENABLED\"" ) );
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent - g_entities, 0, va( "e \"GAME_MUSTBEALIVECOMMAND\"" ) );
		return qfalse;
	}
	return qtrue;
}

/*
==================
ConcatArgs
==================
*/
char *ConcatArgs( int start ) {
	int i, c, tlen;
	static char line[MAX_STRING_CHARS];      /* 0x20089308 */
	int len;
	char arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;

		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
static void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;        // skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}
	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t   *cl;
	int idnum;
	char s2[MAX_STRING_CHARS];
	char n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to - g_entities, 0,
									va( "e \"GAME_BADCLIENTSLOT\x15 %i\"", idnum ) );
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->sess.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to - g_entities, 0,
									va( "e \"GAME_CLIENTNOTACTIVE\x15%i\"", idnum ) );
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum = 0, cl = level.clients ; idnum < level.maxclients ; idnum++, cl++ ) {
		if ( cl->sess.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->sess.name, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to - g_entities, 0,
							va( "e \"GAME_USERNOTONSERVER\x15%s\"", s ) );
	return -1;
}

/*
==================
G_setfog

The fog configstring is Scr_SetFog's "%g %g %g %g %g %g %.0f": near distance,
far distance, a third scalar, red, green, blue and the transition time in
milliseconds.  Only the far distance and that third scalar are read back here.
The retail build keeps the seven values in separate stack slots, not the array
spelled below.
==================
*/
void G_setfog( const char *fogString ) {
	float values[7];

	trap_SetConfigstring( CS_FOGVARS, fogString );

	level.fogOpaqueDist = 3.402823466e+38F;
	level.fogOpaqueDistSq = 3.402823466e+38F;

	if ( sscanf( fogString, "%f %f %f %f %f %f %f",
				 &values[0], &values[1], &values[2], &values[3],
				 &values[4], &values[5], &values[6] ) == 7
		 && values[2] >= 1.0 ) {
		level.fogOpaqueDist = values[1] * 0.828f;
		level.fogOpaqueDistSq = level.fogOpaqueDist * level.fogOpaqueDist;
	}
}

/*
==================
Cmd_Fogswitch_f
==================
*/
void Cmd_Fogswitch_f( void ) {
	G_setfog( ConcatArgs( 1 ) );
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f( gentity_t *ent ) {
	char        *name;
	gitem_t     *it;
	int i;
	qboolean give_all;
	gentity_t   *it_ent;
	int amount;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	// check for an amount (like "give health 30")
	amount = atoi( ConcatArgs( 2 ) );

	name = ConcatArgs( 1 );
	if ( !name || !strlen( name ) ) {
		return;
	}

	if ( Q_stricmp( name, "all" ) == 0 ) {
		give_all = qtrue;
	} else {
		give_all = qfalse;
	}

	if ( give_all || Q_stricmpn( name, "health", 6 ) == 0 ) {
		if ( amount ) {
			ent->health += amount;
		} else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if ( !give_all ) {
			return;
		}
	}

	if ( give_all || Q_stricmp( name, "weapons" ) == 0 ) {
		level.spawning = qtrue;
		for ( i = 1 ; i <= bg_numWeapons ; i++ ) {
			BG_GivePlayerWeapon( &ent->client->ps, i );
		}
		level.spawning = qfalse;
		if ( !give_all ) {
			return;
		}
	}

	if ( give_all || Q_stricmpn( name, "ammo", 4 ) == 0 ) {
		if ( amount ) {
			if ( ent->client->ps.weapon ) {
				Add_Ammo( ent, ent->client->ps.weapon, amount, qtrue );
			}
		} else {
			for ( i = 1 ; i <= bg_numWeapons ; i++ ) {
				Add_Ammo( ent, i, 998, qtrue );
			}
		}
		if ( !give_all ) {
			return;
		}
	}

	if ( Q_stricmpn( name, "allammo", 7 ) == 0 && amount ) {
		for ( i = 1 ; i <= bg_numWeapons ; i++ ) {
			Add_Ammo( ent, i, amount, qtrue );
		}
		return;
	}

	if ( give_all ) {
		return;
	}

	// spawn a dropped item of that name and let the pickup code have it
	it = BG_FindItem( name );
	if ( !it ) {
		return;
	}

	level.spawning = qtrue;

	it_ent = G_Spawn();
	VectorCopy( ent->r.currentOrigin, it_ent->r.currentOrigin );
	G_SetConstString( &it_ent->classname, it->classname );
	G_SpawnItem( it_ent, it );

	it_ent->active = qtrue;
	Touch_Item( it_ent, ent, 1 );
	it_ent->active = qfalse;

	if ( it_ent->inuse ) {
		G_FreeEntity( it_ent );
	}

	level.spawning = qfalse;
}

/*
==================
Cmd_Take_f

The inverse of Cmd_Give_f; CoD's own.
==================
*/
void Cmd_Take_f( gentity_t *ent ) {
	char        *name;
	int i;
	int weapon;
	qboolean take_all;
	int amount;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	amount = atoi( ConcatArgs( 2 ) );

	name = ConcatArgs( 1 );
	if ( !name || !strlen( name ) ) {
		return;
	}

	if ( Q_stricmp( name, "all" ) == 0 ) {
		take_all = qtrue;
	} else {
		take_all = qfalse;
	}

	if ( take_all || Q_stricmpn( name, "health", 6 ) == 0 ) {
		if ( amount ) {
			ent->health -= amount;
			if ( ent->health < 1 ) {
				ent->health = 1;
			}
		} else {
			ent->health = 1;
		}
		if ( !take_all ) {
			return;
		}
	}

	if ( take_all || Q_stricmp( name, "weapons" ) == 0 ) {
		for ( i = 1 ; i <= bg_numWeapons ; i++ ) {
			BG_TakePlayerWeapon( &ent->client->ps, i );
			ent->client->ps.ammo[ bg_weaponInfo[i]->ammoIndex ] = 0;
			ent->client->ps.ammoclip[ bg_weaponInfo[i]->clipIndex ] = 0;
		}
		if ( ent->client->ps.weapon ) {
			ent->client->ps.weapon = 0;
			trap_SendServerCommand( ent - g_entities, 1, "a 0" );
		}
		if ( !take_all ) {
			return;
		}
	}

	if ( take_all || Q_stricmpn( name, "ammo", 4 ) == 0 ) {
		if ( amount ) {
			weapon = ent->client->ps.weapon;
			if ( weapon ) {
				ent->client->ps.ammo[ bg_weaponInfo[weapon]->ammoIndex ] -= amount;
				if ( ent->client->ps.ammo[ bg_weaponInfo[weapon]->ammoIndex ] < 0 ) {
					ent->client->ps.ammoclip[ bg_weaponInfo[weapon]->clipIndex ] +=
						ent->client->ps.ammo[ bg_weaponInfo[weapon]->ammoIndex ];
					ent->client->ps.ammo[ bg_weaponInfo[weapon]->ammoIndex ] = 0;
					if ( ent->client->ps.ammoclip[ bg_weaponInfo[weapon]->clipIndex ] < 0 ) {
						ent->client->ps.ammoclip[ bg_weaponInfo[weapon]->clipIndex ] = 0;
					}
				}
			}
		} else {
			for ( i = 1 ; i <= bg_numWeapons ; i++ ) {
				ent->client->ps.ammo[ bg_weaponInfo[i]->ammoIndex ] = 0;
				ent->client->ps.ammoclip[ bg_weaponInfo[i]->clipIndex ] = 0;
			}
		}
		if ( !take_all ) {
			return;
		}
	}

	if ( Q_stricmpn( name, "allammo", 7 ) == 0 && amount ) {
		for ( i = 1 ; i <= bg_numWeapons ; i++ ) {
			ent->client->ps.ammo[ bg_weaponInfo[i]->ammoIndex ] -= amount;
			if ( ent->client->ps.ammo[ bg_weaponInfo[i]->ammoIndex ] < 0 ) {
				ent->client->ps.ammoclip[ bg_weaponInfo[i]->clipIndex ] +=
					ent->client->ps.ammo[ bg_weaponInfo[i]->ammoIndex ];
				ent->client->ps.ammo[ bg_weaponInfo[i]->ammoIndex ] = 0;
				if ( ent->client->ps.ammoclip[ bg_weaponInfo[i]->clipIndex ] < 0 ) {
					ent->client->ps.ammoclip[ bg_weaponInfo[i]->clipIndex ] = 0;
				}
			}
		}
	}
}

/*
==================
Cmd_God_f
==================
*/
void Cmd_God_f( gentity_t *ent ) {
	const char  *msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if ( !( ent->flags & FL_GODMODE ) ) {
		msg = "GAME_GODMODEOFF";
	} else {
		msg = "GAME_GODMODEON";
	}

	trap_SendServerCommand( ent - g_entities, 0, va( "e \"%s\"", msg ) );
}

/*
==================
Cmd_Notarget_f
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	const char  *msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if ( !( ent->flags & FL_NOTARGET ) ) {
		msg = "GAME_NOTARGETOFF";
	} else {
		msg = "GAME_NOTARGETON";
	}

	trap_SendServerCommand( ent - g_entities, 0, va( "e \"%s\"", msg ) );
}

/*
==================
Cmd_Noclip_f
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	const char  *msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "GAME_NOCLIPOFF";
	} else {
		msg = "GAME_NOCLIPON";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent - g_entities, 0, va( "e \"%s\"", msg ) );
}

/*
==================
Cmd_UFO_f
==================
*/
void Cmd_UFO_f( gentity_t *ent ) {
	const char  *msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->ufo ) {
		msg = "GAME_UFOOFF";
	} else {
		msg = "GAME_UFOON";
	}
	ent->client->ufo = !ent->client->ufo;

	trap_SendServerCommand( ent - g_entities, 0, va( "e \"%s\"", msg ) );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionState != STATE_PLAYING ) {
		return;
	}
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	ent->client->ps.stats[STAT_HEALTH] = 0;
	player_die( ent, ent, ent, 100000, MOD_SUICIDE, 0, NULL, 0 );
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode.  CoD drops the camera behind and above
the followed player instead of RTCW's plain flag clear.
=================
*/
void StopFollowing( gentity_t *ent ) {
	vec3_t origin;
	vec3_t angles;
	vec3_t end;
	vec3_t forward, up;
	vec3_t mins, maxs;
	trace_t trace;

	ent->client->sess.forceSpectatorClient = -1;
	ent->client->spectatorClient = -1;

	if ( !( ent->client->ps.pm_flags & PMF_FOLLOW ) ) {
		return;
	}

	VectorCopy( ent->client->ps.viewangles, angles );
	AngleVectors( angles, forward, NULL, up );
	angles[0] = angles[0] + 15.0f;

	VectorCopy( ent->client->ps.origin, origin );
	origin[2] = origin[2] + ent->client->ps.viewHeightCurrent;
	AddLeanToPosition( origin, ent->client->ps.viewangles[1],
					   ent->client->ps.leanf, 16.0, 20.0 );

	mins[0] = -8.0;  mins[1] = -8.0;  mins[2] = -8.0;
	maxs[0] = 8.0;   maxs[1] = 8.0;   maxs[2] = 8.0;

	end[0] = ( origin[0] - forward[0] * 40.0f ) + up[0] * 10.0f;
	end[1] = ( origin[1] - forward[1] * 40.0f ) + up[1] * 10.0f;
	end[2] = ( origin[2] - forward[2] * 40.0f ) + up[2] * 10.0f;

	trap_TraceCapsule( &trace, origin, mins, maxs, end, ENTITYNUM_NONE,
					   MASK_SPECTATOR_CAMERA );
	VectorCopy( trace.endpos, origin );

	ent->client->ps.viewlocked = 0;
	ent->client->ps.viewlocked_entNum = ENTITYNUM_NONE;
	ent->client->ps.gunfx = 0;
	ent->client->ps.fWeaponPosFrac = 0;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.pm_flags &= ~( PMF_FOLLOW | PMF_ADS );
	ent->client->ps.eFlags &= ~EF_ACTIVATE_MASK;

	G_SetOrigin( ent, origin );
	VectorCopy( origin, ent->client->ps.origin );

	SetClientViewAngle( ent, angles );

	ent->client->ps.shellshockIndex = 0;
	ent->client->ps.shellshockTime = 0;
	ent->client->ps.shellshockDuration = 0;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
qboolean Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int clientnum;
	int original;
	playerState_t ps;

	// if they are playing a following spectator, back it up first
	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	if ( ent->client->sess.forceSpectatorClient >= 0 ) {
		return qfalse;
	}

	clientnum = ent->client->spectatorClient;
	if ( clientnum < 0 ) {
		clientnum = 0;
	}
	original = clientnum;

	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		} else if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		if ( trap_GetArchivedClientInfo( clientnum, &ent->client->sess.archiveTime, &ps ) ) {
			ent->client->spectatorClient = clientnum;
			ent->client->sess.sessionState = STATE_SPECTATOR;
			return qtrue;
		}
	} while ( clientnum != original );

	return qfalse;
}

/*
==================
G_IsPlaying
==================
*/
qboolean G_IsPlaying( gentity_t *ent ) {
	return ent->client->sess.sessionState == STATE_PLAYING;
}

/*
==================
G_SayTo
==================
*/
void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color,
					 const char *name, const char *message ) {
	const char  *cmd;

	if ( !other ) {
		return;
	}
	if ( !other->inuse ) {
		return;
	}
	if ( !other->client ) {
		return;
	}
	if ( other->client->sess.connected != CON_CONNECTED ) {
		return;
	}
	if ( mode == SAY_TEAM && !OnSameTeam( ent, other ) ) {
		return;
	}

	// the dead and the spectating do not reach the living
	if ( !G_IsPlaying( ent ) && G_IsPlaying( other ) ) {
		return;
	}

	if ( mode == SAY_TEAM ) {
		cmd = "i";
	} else {
		cmd = "h";
	}

	trap_SendServerCommand( other - g_entities, 0,
							va( "%s \"\x15%s%c%c%s\"", cmd, name,
								Q_COLOR_ESCAPE, color, message ) );
}

/*
==================
G_Say
==================
*/
void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int j;
	gentity_t   *other;
	int color;
	char name[64];
	char prefix[64];
	char location[64];
	char text2[128];
	char text[MAX_SAY_TEXT];
	const char  *teamName;

	if ( mode == SAY_TEAM ) {
		if ( ent->client->sess.sessionTeam != TEAM_AXIS
			 && ent->client->sess.sessionTeam != TEAM_ALLIES ) {
			mode = SAY_ALL;
		}
	}

	Q_strncpyz( name, ent->client->sess.name, sizeof( name ) );
	Q_CleanStr( name );

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		Com_sprintf( prefix, sizeof( prefix ), "\x15(\x14GAME_SPECTATOR\x15)" );
	} else if ( ent->health > 0 ) {
		Com_sprintf( prefix, sizeof( prefix ), "\x15" );
	} else {
		Com_sprintf( prefix, sizeof( prefix ), "\x15(\x14GAME_DEAD\x15)" );
	}

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", name, chatText );
		Com_sprintf( text2, sizeof( text2 ), "%s%s%s: ", prefix, name, "^7" );
		color = COLOR_WHITE;
		break;

	case SAY_TEAM:
		if ( ent->client->sess.sessionTeam == TEAM_AXIS ) {
			teamName = "GAME_AXIS";
		} else {
			teamName = "GAME_ALLIES";
		}
		G_LogPrintf( "sayteam: %s: %s\n", name, chatText );
		if ( Team_GetLocationMsg( ent, location, sizeof( location ) ) ) {
			Com_sprintf( text2, sizeof( text2 ), "%s(\x14%s\x15)%s%s (\x14%s\x15): ",
						 prefix, teamName, name, "^7", location );
		} else {
			Com_sprintf( text2, sizeof( text2 ), "%s(\x14%s\x15)%s%s: ",
						 prefix, teamName, name, "^7" );
		}
		color = COLOR_CYAN;
		break;

	case SAY_TELL:
		if ( target && target->client->sess.sessionTeam == ent->client->sess.sessionTeam
			 && Team_GetLocationMsg( ent, location, sizeof( location ) ) ) {
			Com_sprintf( text2, sizeof( text2 ), "%s[%s]%s (%s): ",
						 prefix, name, "^7", location );
		} else {
			Com_sprintf( text2, sizeof( text2 ), "%s[%s]%s: ", prefix, name, "^7" );
		}
		color = COLOR_YELLOW;
		break;
	}

	Q_strncpyz( text, chatText, sizeof( text ) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, text2, text );
		return;
	}

	// echo the text to the console
	if ( dedicated.integer ) {
		G_Printf( "%s%s\n", text2, text );
	}

	// send it to all the apropriate clients
	for ( j = 0; j < level.maxclients; j++ ) {
		other = &g_entities[j];
		G_SayTo( ent, other, mode, color, text2, text );
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char        *p;

	if ( trap_Argc() < 2 && !arg0 ) {
		return;
	}

	if ( arg0 ) {
		p = ConcatArgs( 0 );
	} else {
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int targetNum;
	gentity_t   *target;
	char        *p;
	char arg[MAX_STRING_CHARS];

	if ( trap_Argc() < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->sess.name,
				 target->client->sess.name, p );
	G_Say( ent, target, SAY_TELL, p );
	G_Say( ent, ent, SAY_TELL, p );
}

/*
==================
G_VoiceTo
==================
*/
void G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id,
					   qboolean voiceonly ) {
	int color;
	const char  *cmd;

	if ( !other ) {
		return;
	}
	if ( !other->inuse ) {
		return;
	}
	if ( !other->client ) {
		return;
	}
	if ( other->client->sess.connected != CON_CONNECTED ) {
		return;
	}

	if ( mode == SAY_TEAM ) {
		if ( !OnSameTeam( ent, other ) ) {
			return;
		}
		color = COLOR_CYAN;
		cmd = "k";
	} else if ( mode == SAY_TELL ) {
		color = COLOR_MAGENTA;
		cmd = "l";
	} else {
		color = COLOR_GREEN;
		cmd = "j";
	}

	trap_SendServerCommand( other - g_entities, 0,
							va( "%s %d %d %d %s %i %i %i", cmd, voiceonly,
								ent->s.number, color, id,
								(int)ent->s.pos.trBase[0],
								(int)ent->s.pos.trBase[1],
								(int)ent->s.pos.trBase[2] ) );
}

/*
==================
G_Voice
==================
*/
void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id,
			  qboolean voiceonly ) {
	int j;
	gentity_t   *other;

	// don't allow excessive spamming of voice chats
	ent->voiceChatSquelch -= ( level.time - ent->voiceChatPreviousTime );
	ent->voiceChatPreviousTime = level.time;

	if ( ent->voiceChatSquelch < 0 ) {
		ent->voiceChatSquelch = 0;
	}

	if ( ent->voiceChatSquelch >= VOICE_CHAT_SQUELCH_MAX ) {
		trap_SendServerCommand( ent - g_entities, 0,
								"e \"\x15^1\x14GAME_SPAMPROTECT\x15^7: \x14GAME_VOICECHATIGNORED\"" );
		return;
	}

	if ( g_voiceChatsAllowed.integer ) {
		ent->voiceChatSquelch += ( VOICE_CHAT_SQUELCH_SPAN / g_voiceChatsAllowed.integer );
	} else {
		return;
	}

	if ( target ) {
		G_VoiceTo( ent, target, mode, id, voiceonly );
		return;
	}

	// echo the text to the console
	if ( dedicated.integer ) {
		G_Printf( "voice: %s %s\n", ent->client->sess.name, id );
	}

	// send it to all the apropriate clients
	for ( j = 0; j < level.maxclients; j++ ) {
		other = &g_entities[j];
		G_VoiceTo( ent, other, mode, id, voiceonly );
	}
}

/*
==================
Cmd_Voice_f
==================
*/
void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
	char        *p;

	if ( trap_Argc() < 2 && !arg0 ) {
		return;
	}

	if ( arg0 ) {
		p = ConcatArgs( 0 );
	} else {
		p = ConcatArgs( 1 );
	}

	G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_GameCommand_f

Q3's quick-order command, with the localized string ids in place of the
English orders -- including Q3's off-by-one bound test.
==================
*/
static const char *gc_orders[] = {
	"GAME_GC_HOLDYOURPOSITION",
	"GAME_GC_HOLDTHISPOSITION",
	"GAME_GC_COMEHERE",
	"GAME_GC_COVERME",
	"GAME_GC_GUARDLOCATION",
	"GAME_GC_SEARCHDESTROY",
	"GAME_GC_REPORT"
};

static void Cmd_GameCommand_f( gentity_t *ent ) {
	int player;
	int order;
	char str[MAX_STRING_CHARS];

	trap_Argv( 1, str, sizeof( str ) );
	player = atoi( str );
	trap_Argv( 2, str, sizeof( str ) );
	order = atoi( str );

	if ( player < 0 || player >= MAX_CLIENTS ) {
		return;
	}
	if ( order < 0 || order > sizeof( gc_orders ) / sizeof( char * ) ) {
		return;
	}
	G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
	G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	trap_SendServerCommand( ent - g_entities, 0,
							va( "e \"\x15%s\n\"", vtos( ent->r.currentOrigin ) ) );
}

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) {
	int i;
	int clientNum;
	char arg1[256];
	char arg2[256];
	char arg3[256];
	char cleanName[64];
	vmCvar_t mapname;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_VOTINGNOTENABLED\"" );
		return;
	}

	if ( level.voteTime ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_VOTEALREADYINPROGRESS\"" );
		return;
	}

	if ( ent->client->sess.voteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_MAXVOTESCALLED\"" );
		return;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_NOSPECTATORCALLVOTE\"" );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	if ( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_INVALIDVOTESTRING\"" );
		return;
	}

	if ( Q_stricmp( arg1, "map_restart" ) && Q_stricmp( arg1, "map_rotate" )
		 && Q_stricmp( arg1, "typemap" ) && Q_stricmp( arg1, "map" )
		 && Q_stricmp( arg1, "g_gametype" ) && Q_stricmp( arg1, "kick" )
		 && Q_stricmp( arg1, "clientkick" ) ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_INVALIDVOTESTRING\"" );
		trap_SendServerCommand( ent - g_entities, 0,
								"e \"GAME_VOTECOMMANDSARE\x15 map_restart, map_rotate, "
								"map <mapname>, g_gametype <typename>, typemap <typename> "
								"<mapname>, kick <player>, clientkick <clientnum>\"" );
		return;
	}

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}

	if ( !Q_stricmp( arg1, "typemap" ) ) {
		if ( !Scr_IsValidGameType( arg2 ) ) {
			trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_INVALIDGAMETYPE\"" );
			return;
		}
		if ( !Q_stricmp( arg2, g_gametype.string ) ) {
			arg2[0] = 0;
		}

		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
		if ( !Q_stricmp( arg3, mapname.string ) ) {
			arg3[0] = 0;
		}

		if ( !arg2[0] && !arg3[0] ) {
			trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_TYPEMAP_NOCHANGE\"" );
			return;
		}

		if ( !arg2[0] ) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s", arg3 );
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
						 "GAME_VOTE_MAP\x15%s", arg3 );
		} else if ( !arg3[0] ) {
			Com_sprintf( level.voteString, sizeof( level.voteString ),
						 "g_gametype %s; map_restart", arg2 );
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
						 "GAME_VOTE_GAMETYPE\x14%s", Scr_IsValidGameType( arg2 ) );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ),
						 "g_gametype %s; map %s", arg2, arg3 );
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
						 "GAME_VOTE_GAMETYPE\x14%s\x15 - \x14GAME_VOTE_MAP\x15%s",
						 Scr_IsValidGameType( arg2 ), arg3 );
		}
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
		if ( !Scr_IsValidGameType( arg2 ) ) {
			trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_INVALIDGAMETYPE\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ),
					 "%s %s; map_restart", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
					 "GAME_VOTE_GAMETYPE\x14%s", Scr_IsValidGameType( arg2 ) );
	} else if ( !Q_stricmp( arg1, "map_restart" ) ) {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s", arg1 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
					 "GAME_VOTE_MAPRESTART" );
	} else if ( !Q_stricmp( arg1, "map_rotate" ) ) {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s", arg1 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
					 "GAME_VOTE_NEXTMAP" );
	} else if ( !Q_stricmp( arg1, "map" ) ) {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
					 "GAME_VOTE_MAP\x15%s", arg2 );
	} else if ( Q_stricmp( arg1, "kick" ) && Q_stricmp( arg1, "clientkick" ) ) {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
					 "\x15%s", level.voteString );
	} else {
		clientNum = MAX_CLIENTS;

		if ( !Q_stricmp( arg1, "kick" ) ) {
			// resolve the name against every slot, not just the connected count
			for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
				if ( level.clients[i].sess.connected != CON_CONNECTED ) {
					continue;
				}
				Q_strncpyz( cleanName, level.clients[i].sess.name, sizeof( cleanName ) );
				Q_CleanStr( cleanName );
				if ( !Q_stricmp( arg2, cleanName ) ) {
					clientNum = i;
				}
			}
		} else {
			clientNum = atoi( arg2 );
			if ( !clientNum && Q_stricmp( arg2, "0" ) ) {
				trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_CLIENTNOTONSERVER\"" );
				return;
			}
			if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
				trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_CLIENTNOTONSERVER\"" );
				return;
			}
			if ( level.clients[clientNum].sess.connected != CON_CONNECTED ) {
				trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_CLIENTNOTONSERVER\"" );
				return;
			}
			Q_strncpyz( cleanName, level.clients[clientNum].sess.name, sizeof( cleanName ) );
			Q_CleanStr( cleanName );
		}

		if ( clientNum == MAX_CLIENTS ) {
			trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_CLIENTNOTONSERVER\"" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ),
					 "clientkick \"%d\"", clientNum );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
					 "GAME_VOTE_KICK\x15(%i)%s", clientNum,
					 level.clients[clientNum].sess.name );
	}

	trap_SendServerCommand( -1, 0,
							va( "e \"GAME_CALLEDAVOTE\x15%s\"", ent->client->sess.name ) );

	// start the voting, the caller automatically votes yes.  level.voteTime is
	// the deadline, not the start.
	level.voteTime = level.time + VOTE_TIME;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va( "%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
	trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char msg[64];
	int num;

	// complaints supercede voting (and share the command)
	if ( ent->client->sess.complaintEndTime > level.time ) {
		gclient_t *cl = g_entities[ ent->client->sess.complaintClient ].client;

		if ( !cl ) {
			return;
		}
		if ( cl->sess.connected != CON_CONNECTED ) {
			return;
		}
		if ( cl->sess.localClient ) {
			trap_SendServerCommand( ent - g_entities, 1, "m -3" );
			return;
		}

		// reset this ent's complaintEndTime so they can't send multiple complaints
		ent->client->sess.complaintEndTime = -1;
		ent->client->sess.complaintClient = -1;

		trap_Argv( 1, msg, sizeof( msg ) );

		/* the msg[1] on the second and third tests is the ancestor's typo,
		   reproduced here because the binary reproduces it. */
		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			// increase their complaint counter
			cl->sess.complaints++;

			num = g_complaintlimit.integer - cl->sess.complaints;

			if ( num <= 0 && !cl->sess.localClient ) {
				trap_DropClient( cl - level.clients, "GAME_KICKEDFROMCOMPLAINTS" );
				trap_SendServerCommand( ent - g_entities, 1, "m -1" );
				return;
			}

			trap_SendServerCommand( cl->ps.clientNum, 0,
									va( "e \"\x15^1\x14GAME_WARNING\x15^7: "
										"\x14GAME_COMPLAINTFILEDAGAINST\x15%d\"", num ) );
			trap_SendServerCommand( ent - g_entities, 1, "m -1" );
		} else {
			trap_SendServerCommand( ent - g_entities, 1, "m -2" );
		}

		return;
	}

	// reset this ent's complaintEndTime so they can't send multiple complaints
	ent->client->sess.complaintEndTime = -1;
	ent->client->sess.complaintClient = -1;

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_NOVOTEINPROGRESS\"" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_VOTEALREADYCAST\"" );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_NOSPECTATORVOTE\"" );
		return;
	}

	trap_SendServerCommand( ent - g_entities, 0, "e \"GAME_VOTECAST\"" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t origin, angles;
	char buffer[MAX_STRING_CHARS];
	int i;

	if ( !sv_cheats.integer ) {
		trap_SendServerCommand( ent - g_entities, 0, va( "e \"GAME_CHEATSNOTENABLED\"" ) );
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent - g_entities, 0,
								va( "e \"GAME_USAGE\x15: setviewpos x y z yaw\"" ) );
		return;
	}

	angles[0] = 0;
	angles[2] = 0;

	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[1] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

/*
=================
Cmd_Activate_f

CoD's "use" key.  ClientThink_real calls it, not ClientCommand.
=================
*/
qboolean Cmd_Activate_f( gentity_t *ent ) {
	gentity_t   *other;
	int entNum;

	if ( !Scr_IsSystemActive( qtrue ) ) {
		return qfalse;
	}

	if ( ent->active ) {
		ent->active = ( ent->client->ps.eFlags & EF_ACTIVATE_MASK ) ? 2 : 0;
		return qtrue;
	}

	G_CheckForCursorHints( ent );

	entNum = ent->client->ps.serverCursorHintTrace.entityNum;
	if ( entNum == ENTITYNUM_NONE ) {
		return qfalse;
	}
	other = &g_entities[entNum];

	if ( !other->classname ) {
		return qtrue;
	}

	if ( other->classname == scr_const.func_door
		 || other->classname == scr_const.func_door_rotating ) {
		G_TryDoor( other, ent, ent );
		return qtrue;
	}

	if ( other->classname == scr_const.trigger_use ) {
		Scr_AddEntityNum( ent->s.number, 0 );
		Scr_NotifyNum( other->s.number, 0, scr_const.trigger, 1 );
		other->use( other, ent, ent );
		return qtrue;
	}

	if ( other->s.eType == ET_TRIGGER ) {
		Scr_AddEntityNum( ent->s.number, 0 );
		Scr_NotifyNum( other->s.number, 0, scr_const.touch, 1 );
		if ( !other->touch ) {
			return qfalse;
		}
		other->active = qtrue;
		other->touch( other, ent, 0 );
		return qtrue;
	}

	if ( other->s.eType == ET_TURRET ) {
		if ( !G_IsTurretUsable( other, ent ) ) {
			return qfalse;
		}
		other->use( other, ent, ent );
		return qtrue;
	}

	if ( other->classname == scr_const.misc_flak && !other->active ) {
		if ( infront( ent, other ) ) {
			return qfalse;
		}
		if ( level.clients[ ent->s.clientNum ].ps.grenadeTimeLeft ) {
			return qfalse;
		}
		other->active = qtrue;
		ent->active = qtrue;
		other->r.ownerNum = ent->s.number;
		VectorCopy( other->r.currentAngles, other->TargetAngles );
		return qtrue;
	}

	if ( other->classname == scr_const.script_brushmodel ) {
		Scr_AddEntityNum( ent->s.number, 0 );
		Scr_NotifyNum( other->s.number, 0, scr_const.trigger, 1 );
		if ( !other->use ) {
			return qfalse;
		}
		other->use( other, ent, ent );
		return qtrue;
	}

	if ( other->classname == scr_const.team_WOLF_checkpoint
		 && other->count != ent->client->sess.sessionTeam ) {
		other->health++;
		return qtrue;
	}

	return qtrue;
}

/*
=================
Cmd_EntityCount_f
=================
*/
void Cmd_EntityCount_f( gentity_t *ent ) {
	if ( sv_cheats.integer ) {
		G_Printf( "entity count = %i\n", level.num_entities );
	}
}

/*
=================
Cmd_MenuResponse_f
=================
*/
void Cmd_MenuResponse_f( gentity_t *ent ) {
	char menu[MAX_STRING_CHARS];
	char response[MAX_STRING_CHARS];
	char arg[MAX_STRING_CHARS];
	int menuIndex;

	if ( trap_Argc() == 4 ) {
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( atoi( arg ) != trap_Cvar_VariableIntegerValue( "sv_serverId" ) ) {
			return;
		}

		trap_Argv( 2, menu, sizeof( menu ) );
		menuIndex = atoi( menu );
		if ( menuIndex >= 0 && menuIndex < CS_SCRIPTMENUS_COUNT ) {
			trap_GetConfigstring( menuIndex + CS_SCRIPTMENUS, menu, sizeof( menu ) );
		}

		trap_Argv( 3, response, sizeof( response ) );
	} else {
		menu[0] = 0;
		strcpy( response, "bad" );
	}

	Scr_AddString( response );
	Scr_AddString( menu );
	Scr_NotifyNum( ent->s.number, 0, scr_const.menuresponse, 2 );
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t   *ent;
	char cmd[MAX_STRING_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;     // not fully in game yet
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	/* UO voice requests are dispatched to mod scripts as one pair string. */
	if ( Q_stricmp( cmd, "voice" ) == 0 ) {
		if ( trap_Argc() > 1 ) {
			Scr_AddString( ConcatArgs( 1 ) );
			Scr_NotifyNum( ent->s.number, 0, scr_const_vsay, 1 );
		}
		return;
	}

	/* LTCG inlines Cmd_Say_f / Cmd_Voice_f into the four chat cases with
	   arg0 = qfalse, so the arg-count gate and the ConcatArgs( 1 ) show up
	   here rather than in a call. */
	if ( Q_stricmp( cmd, "say" ) == 0 ) {
		if ( trap_Argc() < 2 ) {
			return;
		}
		G_Say( ent, NULL, SAY_ALL, ConcatArgs( 1 ) );
		return;
	}
	if ( Q_stricmp( cmd, "say_team" ) == 0 ) {
		if ( trap_Argc() < 2 ) {
			return;
		}
		G_Say( ent, NULL, SAY_TEAM, ConcatArgs( 1 ) );
		return;
	}
	if ( Q_stricmp( cmd, "vsay" ) == 0 ) {
		if ( trap_Argc() < 2 ) {
			return;
		}
		G_Voice( ent, NULL, SAY_ALL, ConcatArgs( 1 ), qfalse );
		return;
	}
	if ( Q_stricmp( cmd, "vsay_team" ) == 0 ) {
		if ( trap_Argc() < 2 ) {
			return;
		}
		G_Voice( ent, NULL, SAY_TEAM, ConcatArgs( 1 ), qfalse );
		return;
	}
	if ( Q_stricmp( cmd, "tell" ) == 0 ) {
		Cmd_Tell_f( ent );
		return;
	}
	if ( Q_stricmp( cmd, "score" ) == 0 ) {
		DeathmatchScoreboardMessage( ent );
		return;
	}

	// ignore all other commands when at intermission
	if ( ent->client->ps.pm_type == PM_INTERMISSION ) {
		return;
	}

	if ( Q_stricmp( cmd, "mr" ) == 0 ) {
		Cmd_MenuResponse_f( ent );
	} else if ( Q_stricmp( cmd, "give" ) == 0 ) {
		Cmd_Give_f( ent );
	} else if ( Q_stricmp( cmd, "take" ) == 0 ) {
		Cmd_Take_f( ent );
	} else if ( Q_stricmp( cmd, "god" ) == 0 ) {
		Cmd_God_f( ent );
	} else if ( Q_stricmp( cmd, "notarget" ) == 0 ) {
		Cmd_Notarget_f( ent );
	} else if ( Q_stricmp( cmd, "noclip" ) == 0 ) {
		Cmd_Noclip_f( ent );
	} else if ( Q_stricmp( cmd, "ufo" ) == 0 ) {
		Cmd_UFO_f( ent );
	} else if ( Q_stricmp( cmd, "kill" ) == 0 ) {
		Cmd_Kill_f( ent );
	} else if ( Q_stricmp( cmd, "follownext" ) == 0 ) {
		Cmd_FollowCycle_f( ent, 1 );
	} else if ( Q_stricmp( cmd, "followprev" ) == 0 ) {
		Cmd_FollowCycle_f( ent, -1 );
	} else if ( Q_stricmp( cmd, "where" ) == 0 ) {
		Cmd_Where_f( ent );
	} else if ( Q_stricmp( cmd, "callvote" ) == 0 ) {
		Cmd_CallVote_f( ent );
	} else if ( Q_stricmp( cmd, "vote" ) == 0 ) {
		Cmd_Vote_f( ent );
	} else if ( Q_stricmp( cmd, "gc" ) == 0 ) {
		Cmd_GameCommand_f( ent );
	} else if ( Q_stricmp( cmd, "setviewpos" ) == 0 ) {
		Cmd_SetViewpos_f( ent );
	} else if ( Q_stricmp( cmd, "entitycount" ) == 0 ) {
		Cmd_EntityCount_f( ent );
	} else {
		trap_SendServerCommand( clientNum, 0,
								va( "e \"GAME_UNKNOWNCLIENTCOMMAND\x15%s\"", cmd ) );
	}
}
