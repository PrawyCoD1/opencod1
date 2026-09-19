/*
 * g_client_mp.c -- the multiplayer client lifecycle: connect, begin, spawn,
 * userinfo and disconnect, plus the three small helpers ClientEndFrame leans
 * on (G_SetPlayerSize, G_AddLean, G_GetNonPVSFriendlyInfo).
 *
 * Call of Duty 1.1 multiplayer (game_mp_x86.dll, 0x20019880 .. 0x2001A888).
 * RTCW's game/g_client.c is the ancestor of SetClientViewAngle,
 * ClientCleanName, ClientUserinfoChanged, ClientConnect, ClientBegin,
 * ClientSpawn and ClientDisconnect, and most of that text survives.  What CoD
 * changed:
 *
 *   - there is no SelectSpawnPoint here at all: the game type script picks the
 *     spawn point and calls down through ClientSpawn's origin/angles
 *     arguments;
 *   - connect / begin / disconnect each run a script callback
 *     (CodeCallback_PlayerConnect and friends out of g_scr_data);
 *   - the per-client animation record bg_clientinfo[] is reset on connect and
 *     the name/team half of it is refreshed on every userinfo change;
 *   - SetClientViewAngle clamps the view into the prone cone;
 *   - the ent lives on a DObj, so ClientSpawn drives r.mins/r.maxs, the
 *     view heights and the speed scales out of cvars rather than out of
 *     constants.
 *
 * Names not recovered from the binary are flagged where they are defined.
 *
 * @fidelity: likely
 */

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "g_local.h"

/*
 * Missing from universal/q_shared.h; every unit in this module open-codes
 * them.  ANGLE2SHORT/SHORT2ANGLE are the Q3 pair, unchanged.
 */
#define PITCH                       0
#define YAW                         1
#define ROLL                        2

#define ANGLE2SHORT( x )            ( (int)( (x) * ( 65536.0f / 360.0f ) ) & 65535 )
#define SHORT2ANGLE( x )            ( (float)( x ) * ( 360.0f / 65536.0f ) )

#define Q_COLOR_ESCAPE              '^'
#define ColorIndex( c )             ( ( (c) - '0' ) & 7 )

/* playerState_t.pm_flags.  The first two are bg_pmove.c's names; 0x40000 has
   none -- ClientEndFrame raises it for a client that is actually in the world,
   GetFollowPlayerState refuses to hand out a playerState without it, and the
   intermission / spectator end-frame paths drop it.  The name is inferred;
   g_active_mp.c spells the same one. */
#define PMF_PRONE                   0x00000001
#define PMF_RESPAWNED               0x00000800
#define PMF_INWORLD                 0x00040000

/* entityState_t.eFlags / playerState_t.eFlags. */
#define EF_DEAD                     0x00000001
#define EF_TELEPORT_BIT             0x00000008
#define EF_TURRET_ACTIVE_MASK       0x0000C000

/* entityShared_t.svFlags; SVF_NOCLIENT is the engine side's (sv_game_mp.c:38),
   SVF_CAPSULE sv_world_mp.c:29. */
#define SVF_NOCLIENT                0x00000001
#define SVF_CAPSULE                 0x00000200

#define CONTENTS_BODY               0x02000000

/* ClientSpawn's clipmask (0x2001A111).  It is bg_pmove.c's MASK_PLAYERSOLID
   (0x02810191) without 0x80 and 0x100; the name is inferred; g_active_mp.c
   spells the same one. */
#define MASK_CLIENTSOLID            0x02810011

#define ET_PLAYER                   1

/* playerState_t.stats[] indices.  Only these three are touched by this module
   and none of them has a recovered name: [0] is the health ClientEndFrame
   mirrors, [2] the session max health ClientSpawn restores, [5] a counter
   ClientSpawn bumps so the client can detect the respawn. */
#define STAT_HEALTH                 0
#define STAT_MAX_HEALTH             2
#define STAT_SPAWN_COUNT            5

/* level.clientNameMode: GScr_SetClientNameMode (0x200341A0) writes 1 for
   "manual_change" and 0 for "auto_change".  While it is set a connected
   client's userinfo name only reaches sess.netname; sess.name -- the name the
   script sees -- is left for GScr_UpdateClientNames. */

/* No header covers the q_shared / q_math half of the module, and bg_public.h
 * does not declare bg_misc.c's or bg_weapon.c's exports. */
char        *Info_ValueForKey( const char *s, const char *key );
void        Q_strncpyz( char *dest, const char *src, int destsize );
int         Q_stricmp( const char *s1, const char *s2 );
float       AngleNormalize180( float angle );
void        AddLeanToPosition( vec3_t pos, float yaw, float leanFrac,
							   float leanDist, float scale );

/* forward: g_active_mp.c */
void        ClientThink_real( gentity_t *ent, usercmd_t *ucmd );
void        ClientEndFrame( gentity_t *ent );
void        G_SetClientContents( gentity_t *ent );

/* forward: g_cmds_mp.c */
void        StopFollowing( gentity_t *ent );
qboolean    Cmd_FollowCycle_f( gentity_t *ent, int dir );

/* forward: g_scr_main_mp.c -- two members of g_scr_data (0x202CD340); that
   unit owns the struct type, which no header carries. */

/* forward: g_main_mp.c -- the vmCvar_t objects G_RegisterCvars owns. */
extern vmCvar_t g_password;                     /* 0x2016EDA0 */
extern vmCvar_t g_inactivity;                   /* 0x20235D20 */
extern vmCvar_t g_bounds_width;                 /* 0x20234A00 */
extern vmCvar_t g_bounds_height_standing;       /* 0x20234E80 */
extern vmCvar_t bg_viewheight_standing;         /* 0x2014EF20 */
extern vmCvar_t bg_viewheight_crouched;         /* 0x2014F3A0 */
extern vmCvar_t bg_viewheight_prone;            /* 0x2014E7C0 */

/*
===============
SetClientViewAngle

CoD's addition to RTCW's version is the prone cone: while the player is prone
and not on a turret the view is clamped to +-45 degrees of yaw around
ps.proneDirection and to -15..+45 of pitch around ps.proneTorsoPitch, with the
overshoot folded into ps.delta_angles so the client's own prediction agrees.
===============
*/
void SetClientViewAngle( gentity_t *ent, const vec3_t angle ) {
	int cmdAngle;
	float pitch, yaw, roll;
	float diff, excess;

	pitch = angle[PITCH];
	yaw = angle[YAW];
	roll = angle[ROLL];

	if ( ( ent->client->ps.pm_flags & PMF_PRONE )
		 && !( ent->client->ps.eFlags & EF_TURRET_ACTIVE_MASK ) ) {

		diff = AngleNormalize180( AngleNormalize180( ent->client->ps.proneDirection - yaw ) );
		if ( diff > 45.0f || diff < -45.0f ) {
			if ( diff <= 0.0f ) {
				excess = diff + 45.0f;
			} else {
				excess = diff - 45.0f;
			}
			ent->client->ps.delta_angles[YAW] += ANGLE2SHORT( excess );
			if ( excess <= 0.0f ) {
				yaw = SHORT2ANGLE( ANGLE2SHORT( ent->client->ps.proneDirection + 45.0f ) );
			} else {
				yaw = SHORT2ANGLE( ANGLE2SHORT( ent->client->ps.proneDirection - 45.0f ) );
			}
		}

		diff = AngleNormalize180( AngleNormalize180( ent->client->ps.proneTorsoPitch - pitch ) );
		if ( diff > 45.0f || diff < -15.0f ) {
			if ( diff <= 0.0f ) {
				excess = diff + 15.0f;
			} else {
				excess = diff - 45.0f;
			}
			ent->client->ps.delta_angles[PITCH] += ANGLE2SHORT( excess );
			if ( excess <= 0.0f ) {
				pitch = AngleNormalize180( ent->client->ps.proneTorsoPitch + 15.0f );
			} else {
				pitch = AngleNormalize180( ent->client->ps.proneTorsoPitch - 45.0f );
			}
		}
	}

	/* set the delta angle */
	cmdAngle = ANGLE2SHORT( pitch );
	ent->client->ps.delta_angles[PITCH] = cmdAngle - ent->client->sess.cmd.angles[PITCH];
	cmdAngle = ANGLE2SHORT( yaw );
	ent->client->ps.delta_angles[YAW] = cmdAngle - ent->client->sess.cmd.angles[YAW];
	cmdAngle = ANGLE2SHORT( roll );
	ent->client->ps.delta_angles[ROLL] = cmdAngle - ent->client->sess.cmd.angles[ROLL];

	ent->r.currentAngles[PITCH] = pitch;
	ent->r.currentAngles[YAW] = yaw;
	ent->r.currentAngles[ROLL] = roll;

	VectorCopy( ent->r.currentAngles, ent->client->ps.viewangles );
}

/*
===========
ClientCleanName

RTCW's, unchanged apart from the buffer being CoD's 32-byte sess.netname /
sess.name.
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
	int len, colorlessLen;
	char ch;
	char *p;
	int spaces;

	/* save room for the trailing null byte */
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while ( 1 ) {
		ch = *in++;
		if ( !ch ) {
			break;
		}

		/* don't allow leading spaces */
		if ( !*p && ch == ' ' ) {
			continue;
		}

		/* check colors */
		if ( ch == Q_COLOR_ESCAPE ) {
			/* solo trailing carat is not a color prefix */
			if ( !*in ) {
				break;
			}

			/* don't allow black in a name, period */
			if ( ColorIndex( *in ) == 0 ) {
				in++;
				continue;
			}

			/* make sure room in dest for both chars */
			if ( len > outSize - 2 ) {
				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		/* don't allow too many consecutive spaces */
		if ( ch == ' ' ) {
			spaces++;
			if ( spaces > 3 ) {
				continue;
			}
		} else {
			spaces = 0;
		}

		if ( len > outSize - 1 ) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	/* don't allow empty names */
	if ( *p == 0 || colorlessLen == 0 ) {
		Q_strncpyz( p, "UnnamedPlayer", outSize );
	}
}

/*
===========
ClientUserinfoChanged

Called from ClientConnect when the player first connects and directly by the
server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	gentity_t   *ent;
	gclient_t   *client;
	clientInfo_t *ci;
	char        *s;
	char oldname[MAX_STRING_CHARS];
	char userinfo[MAX_STRING_CHARS];

	ent = g_entities + clientNum;
	client = ent->client;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	/* check for malformed or illegal info strings */
	if ( strchr( userinfo, '"' ) || strchr( userinfo, ';' ) ) {
		strcpy( userinfo, "\\name\\badinfo" );
	}

	/* check for local client */
	s = Info_ValueForKey( userinfo, "ip" );
	if ( s && !strcmp( s, "localhost" ) ) {
		client->sess.localClient = qtrue;
	}

	/* check the item prediction */
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	client->sess.predictItemPickup = atoi( s ) != 0;

	/* set name */
	if ( client->sess.connected == CON_CONNECTED && level.clientNameMode ) {
		/* the script owns sess.name while renames are manual; only the
		   pending name changes here */
		s = Info_ValueForKey( userinfo, "name" );
		ClientCleanName( s, client->sess.netname, sizeof( client->sess.netname ) );
	} else {
		Q_strncpyz( oldname, client->sess.name, sizeof( oldname ) );
		s = Info_ValueForKey( userinfo, "name" );
		ClientCleanName( s, client->sess.name, sizeof( client->sess.name ) );
		Q_strncpyz( client->sess.netname, client->sess.name, sizeof( client->sess.netname ) );
	}

	client->sess.handicap = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->sess.handicap < 1 || client->sess.handicap > 100 ) {
		client->sess.handicap = 100;
	}

	ci = &bg_clientinfo[clientNum];
	ci->clientNum = clientNum;
	Q_strncpyz( ci->name, client->sess.name, sizeof( ci->name ) );
	ci->team = client->sess.sessionTeam;
}

/*
===========
ClientConnect

Called when a player begins connecting to the server.  Called again for every
map change or tournament restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return a string with the
reason for denial.
============
*/
const char *ClientConnect( int clientNum, unsigned int scriptId ) {
	gentity_t   *ent;
	gclient_t   *client;
	clientInfo_t *ci;
	void        *animTree;
	char        *value;
	char userinfo[MAX_STRING_CHARS];

	client = level.clients + clientNum;
	ci = &bg_clientinfo[clientNum];

	memset( client, 0, sizeof( gclient_t ) );

	/* the XAnim tree GScr_LoadScripts built is the one thing in the animation
	   record that has to survive a reconnect */
	animTree = ci->animTree;
	memset( ci, 0, sizeof( clientInfo_t ) );
	ci->animTree = animTree;
	ci->infoValid = qtrue;
	*(int *)&ci->unknown_0x004[0] = 1;

	client->sess.connected = CON_CONNECTING;
	client->sess.pers = (unsigned short)scriptId;
	client->sess.sessionTeam = TEAM_SPECTATOR;
	client->sess.sessionState = STATE_SPECTATOR;
	client->spectatorClient = -1;
	client->sess.forceSpectatorClient = -1;

	ent = g_entities + clientNum;
	G_InitGentity( ent );
	ent->client = client;
	ent->touch = NULL;
	ent->pain = NULL;

	client->sess.clientNum = clientNum;
	client->ps.clientNum = clientNum;
	client->ps.eFlags = 16;
	ent->r.svFlags = SVF_CAPSULE;
	client->sess.complaintClient = -1;
	client->sess.complaintEndTime = -1;

	/* get and distribute the relevant paramters */
	ClientUserinfoChanged( clientNum );

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	/* check for a password */
	value = Info_ValueForKey( userinfo, "ip" );
	if ( strcmp( value, "localhost" ) ) {
		value = Info_ValueForKey( userinfo, "password" );
		if ( g_password.string[0]
			 && Q_stricmp( g_password.string, "none" )
			 && strcmp( g_password.string, value ) ) {
			G_FreeEntity( ent );
			return "GAME_INVALIDPASSWORD";
		}
	}

	Scr_FreeThread( Scr_ExecEntThreadNum( ent->s.number, 0, g_scr_data.playerConnect, 0 ) );

	CalculateRanks();

	return NULL;
}

/*
===========
ClientBegin

Called when a client has finished connecting and is ready to be placed into the
level.  This will happen on every level load and level restart, but doesn't
happen on respawns.
============
*/
void ClientBegin( int clientNum ) {
	gentity_t   *ent;

	level.clients[clientNum].sess.connected = CON_CONNECTED;

	CalculateRanks();

	ent = g_entities + clientNum;
	Scr_NotifyNum( ent->s.number, 0, scr_const.begin, 0 );
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world: after ClientBegin, and
after each respawn.  Initializes all non-persistant parts of the playerState.
The script picks origin and angles; there is no SelectSpawnPoint in 1.1.
============
*/
void ClientSpawn( gentity_t *ent, const vec3_t origin, const vec3_t angles ) {
	int index;
	gclient_t       *client;
	clientSession_t saved;
	int savedSpawnCount;
	int eFlags;
	usercmd_t ucmd;

	index = ent - g_entities;
	client = ent->client;

	if ( ( client->ps.pm_flags & PMF_INWORLD )
		 && ( client->ps.eFlags & EF_TURRET_ACTIVE_MASK ) ) {
		G_ClientStopUsingTurret( &level.gentities[client->ps.viewlocked_entNum] );
	}

	G_EntUnlink( ent );

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	Scr_SetString( &ent->classname, scr_const.player );

	ent->r.svFlags |= SVF_NOCLIENT;
	ent->clipmask = MASK_CLIENTSOLID;
	ent->takedamage = qfalse;

	G_SetClientContents( ent );

	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = FL_SUPPORTS_LINKTO;

	VectorCopy( playerMins, ent->r.mins );
	VectorCopy( playerMaxs, ent->r.maxs );

	eFlags = client->ps.eFlags;
	savedSpawnCount = client->ps.stats[STAT_SPAWN_COUNT];
	saved = client->sess;

	memset( client, 0, sizeof( gclient_t ) );

	client->sess = saved;
	client->ps.stats[STAT_MAX_HEALTH] = client->sess.maxHealth;
	client->ps.stats[STAT_SPAWN_COUNT] = savedSpawnCount + 1;
	client->sess.clientNum = index;
	client->ps.clientNum = index;

	/* the two bits carried across the wipe are the last teleport bit and one
	   more flag with no recovered name */
	client->ps.eFlags = ( eFlags & 0x20008 ) | 16;
	client->spectatorClient = -1;

	trap_GetUsercmd( client - level.clients, &client->sess.cmd );

	/* toggle the teleport bit so the client can detect the respawn */
	client->ps.eFlags ^= EF_TELEPORT_BIT;

	VectorCopy( ent->r.mins, client->ps.mins );
	VectorCopy( ent->r.maxs, client->ps.maxs );

	client->ps.proneViewHeight = bg_viewheight_prone.integer;
	client->ps.crouchViewHeight = bg_viewheight_crouched.integer;
	client->ps.standViewHeight = bg_viewheight_standing.integer;
	client->ps.deadViewHeight = 8;
	client->ps.viewHeightTarget = bg_viewheight_standing.integer;
	client->ps.viewHeightCurrent = (float)bg_viewheight_standing.integer;
	client->ps.viewHeightLerpTime = 0;
	client->ps.viewHeightLerpPosAdj = 0;

	client->ps.walkSpeedScale = 0.4f;
	client->ps.runSpeedScale = 1.0f;
	client->ps.proneSpeedScale = 0.15f;
	client->ps.crouchSpeedScale = 0.65f;
	client->ps.strafeSpeedScale = 0.8f;
	client->ps.backSpeedScale = 0.7f;
	client->ps.leanSpeedScale = 0.4f;
	client->ps.friction = 1.0f;

	G_SetOrigin( ent, origin );
	VectorCopy( origin, client->ps.origin );

	client->ps.pm_flags |= PMF_RESPAWNED;

	SetClientViewAngle( ent, angles );

	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;
	client->latched_wbuttons = 0;

	/* run a client frame to drop exactly to the floor, initialize animations
	   and other things */
	memset( &ucmd, 0, sizeof( ucmd ) );
	ucmd.serverTime = level.time;
	ucmd.angles[PITCH] = -client->ps.delta_angles[PITCH];
	ucmd.angles[YAW] = -client->ps.delta_angles[YAW];
	ucmd.angles[ROLL] = -client->ps.delta_angles[ROLL];

	client->ps.commandTime = level.time - 100;

	ClientEndFrame( ent );
	ClientThink_real( ent, &ucmd );

	/* positively link the client, even if the command times are weird */
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
}

/*
===========
ClientDisconnect

Called when a player drops from the server.  Will not be called between levels.

This should NOT be called directly by any game logic, call trap_DropClient
instead.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t   *ent;
	gclient_t   *client;
	int i;

	client = level.clients + clientNum;
	ent = g_entities + clientNum;

	/* stop any menu the player has open */
	Scr_AddString( "disconnect" );
	Scr_AddString( "-1" );
	Scr_NotifyNum( ent->s.number, 0, scr_const.menuresponse, 2 );

	/* send effect if they were completely connected */
	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].sess.connected
			 && level.clients[i].sess.sessionState == STATE_SPECTATOR
			 && level.clients[i].spectatorClient == clientNum ) {
			if ( !Cmd_FollowCycle_f( &g_entities[i], 1 ) ) {
				StopFollowing( &g_entities[i] );
			}
		}
	}

	/* drop the complaint the leaver was the subject of */
	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].sess.complaintClient == clientNum ) {
			level.clients[i].sess.complaintClient = -1;
			level.clients[i].sess.complaintEndTime = 0;
			trap_SendServerCommand( i, 1, "m -2" );
			break;
		}
	}

	HudElem_ClientDisconnect( ent );

	Scr_FreeThread( Scr_ExecEntThreadNum( ent->s.number, 0, g_scr_data.playerDisconnect, 0 ) );

	G_FreeEntity( ent );

	client->sess.connected = CON_DISCONNECTED;
	memset( &client->sess.clientNum, 0,
			sizeof( clientSession_t ) - offsetof( clientSession_t, clientNum ) );

	CalculateRanks();
}

/*
===========
G_SetPlayerSize

Push the two player-bounds cvars into the shared bounding box.  G_InitGame and
G_RunFrame both inline this rather than call it.
============
*/
void G_SetPlayerSize( void ) {
	playerMaxs[2] = g_bounds_height_standing.value;
	playerMins[0] = playerMins[1] = g_bounds_width.value * -0.5f;
	playerMaxs[0] = playerMaxs[1] = g_bounds_width.value * 0.5f;
}

/*
===========
G_AddLean

Slide a point sideways by the client's current lean, with the same 16-unit
reach and 20-unit pivot height the weapon and damage paths use.
============
*/
void G_AddLean( gentity_t *ent, vec3_t pos ) {
	AddLeanToPosition( pos, ent->client->ps.viewangles[YAW], ent->client->ps.leanf,
					   16.0f, 20.0f );
}

/*
===========
G_GetNonPVSFriendlyInfo

Pick one live team mate that is NOT in this client's snapshot and pack him into
ps.iCompassFriendInfo for the compass:

    bits  0..5   his entity number
    bits  6..14  (dx/4 + 255), dx clamped to [-1022,1024]
    bits 15..23  (dy/4 + 255)
    bits 24..31  his yaw as a byte

The scan resumes from the entity the previous frame reported, so over 64 frames
every out-of-snapshot team mate gets a turn.  A zero result means "nobody".
============
*/
int G_GetNonPVSFriendlyInfo( gentity_t *ent, const vec3_t origin, int lastEntNum ) {
	int team;
	int i, start;
	gentity_t   *other;
	int x, y;
	float xScale, yScale;

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_FREE || team == TEAM_SPECTATOR ) {
		return 0;
	}

	if ( lastEntNum == ENTITYNUM_NONE ) {
		start = 0;
	} else {
		start = lastEntNum + 1;
	}

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		other = &g_entities[( start + i ) % MAX_CLIENTS];

		if ( !other->inuse ) {
			continue;
		}
		if ( other->s.eType != ET_PLAYER ) {
			continue;
		}
		if ( other->s.eFlags & EF_DEAD ) {
			continue;
		}
		if ( !other->client || other->client->sess.sessionTeam != team ) {
			continue;
		}
		if ( !trap_InSnapshot( origin, other->s.number ) ) {
			break;
		}
	}

	if ( i == MAX_CLIENTS ) {
		return 0;
	}

	x = (int)( other->r.currentOrigin[0] - origin[0] + 0.5f );
	y = (int)( other->r.currentOrigin[1] - origin[1] + 0.5f );

	xScale = 1.0f;
	if ( x > 1024 ) {
		xScale = 1024.0f / (float)x;
	} else if ( x < -1022 ) {
		xScale = -1022.0f / (float)x;
	}

	yScale = 1.0f;
	if ( y > 1024 ) {
		yScale = 1024.0f / (float)y;
	} else if ( y < -1022 ) {
		yScale = -1022.0f / (float)y;
	}

	/* scale the longer leg down so the shorter one keeps its direction */
	if ( xScale < 1.0f || yScale < 1.0f ) {
		if ( xScale < yScale ) {
			y = (int)( (float)y * xScale );
		} else if ( yScale < xScale ) {
			x = (int)( (float)x * yScale );
		}
	}

	if ( x > 1024 ) {
		x = 1024;
	} else if ( x < -1022 ) {
		x = -1022;
	}
	if ( y > 1024 ) {
		y = 1024;
	} else if ( y < -1022 ) {
		y = -1022;
	}

	return ( other->s.number & 0x3F )
		   | ( ( ( ( x + 2 ) / 4 + 255 ) << 6 ) & 0x7FC0 )
		   | ( ( ( ( y + 2 ) / 4 + 255 ) << 15 ) & 0xFF8000 )
		   | ( (int)( other->r.currentAngles[YAW] * ( 256.0f / 360.0f ) ) << 24 );
}
