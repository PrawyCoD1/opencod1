/*
 * @fidelity: likely
 *
 * g_client_fields_mp.c -- the script-visible fields of a player.
 *
 * clientFields[] (0x20055C00) is the client half of the entity field table in
 * g_spawn_mp.c.  GScr_AddFieldsForClient registers every entry with the VM
 * under the entity class with 0xC000 set in the field number, and
 * Scr_SetEntityField / Scr_GetEntityField index back into this table by that
 * registration order.  Unlike the entity table each entry carries a getter as
 * well as a setter, both take the gclient_t twice, and the five virtual fields
 * (sessionteam, sessionstate, statusicon, headicon, headiconteam) have offset
 * 0 -- the accessor pair does all the work.
 *
 * Function order is binary order (0x20019130 .. 0x20019850); clientFields[]
 * itself is emitted in table order.
 */

#include <stdlib.h>

#include "g_local.h"

/* A field number with both 0xC000 bits set is a client field; the remaining
 * bits index clientFields[]. */
#define CLIENT_FIELD_MASK       0x0000C000

/*
 * Status and head icons are precached into two configstring blocks by
 * GScr_GetStatusIconIndex (0x20031FF0) and GScr_GetHeadIconIndex (0x200321B0),
 * which scan CS_STATUSICONS + 0..MAX_STATUSICONS-1 and CS_HEADICONS +
 * 0..MAX_HEADICONS-1 and return a 1-based index (0 = none).  bg_public.h
 * carries these in the original tree.
 */
#define CS_STATUSICONS          21
#define MAX_STATUSICONS         8
#define CS_HEADICONS            29
#define MAX_HEADICONS           15

/* playerState_t.stats[] indices; bg_public.h carries the enum in the original
 * tree.  Both are used by ClientScr_SetMaxHealth (0x20019390). */
#define STAT_HEALTH             0
#define STAT_MAX_HEALTH         2

/*
 * The field types, duplicated from g_spawn_mp.c -- fields[] and clientFields[]
 * share them and neither table has a header of its own yet.
 */
typedef enum {
	F_INT,
	F_FLOAT,
	F_CSTRING,              /* char[] stored in place */
	F_STRING,               /* const string id, an unsigned short */
	F_VECTOR,
	F_ENTITY,               /* gentity_t * in memory */
	F_VECTOR_Y,
	F_OBJECT,               /* script object id, an unsigned short */
	F_MODEL,                /* model index, a byte */
	F_IGNORE
} fieldtype_t;

typedef struct clientField_s clientField_t;
typedef void ( *clientFieldFunc_t )( gclient_t *client, gclient_t *self, const clientField_t *field );

struct clientField_s {
	const char *name;
	int ofs;
	fieldtype_t type;
	clientFieldFunc_t setter;       /* NULL falls through to Scr_SetGenericField */
	clientFieldFunc_t getter;       /* NULL falls through to Scr_GetGenericField */
};

#define CFOFS( x ) ( (int)&( ( (gclient_t *)0 )->x ) )

static void ClientScr_ReadOnly( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	Scr_Error( va( "player field %s is read-only\n", field->name ) );
}

static void ClientScr_SetSessionTeam( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	unsigned short string;

	string = Scr_GetConstString( 0 );

	if ( string == scr_const.axis ) {
		client->sess.sessionTeam = TEAM_AXIS;
	} else if ( string == scr_const.allies ) {
		client->sess.sessionTeam = TEAM_ALLIES;
	} else if ( string == scr_const.spectator ) {
		client->sess.sessionTeam = TEAM_SPECTATOR;
	} else if ( string == scr_const.none ) {
		client->sess.sessionTeam = TEAM_FREE;
	} else {
		Scr_Error( va( "'%s' is an illegal sessionteam string. Must be allies, axis, none, or spectator.",
					   SL_ConvertToString( string ) ) );
	}

	ClientUserinfoChanged( client - level.clients );
	CalculateRanks();
}

static void ClientScr_GetSessionTeam( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	switch ( client->sess.sessionTeam ) {
	case TEAM_ALLIES:
		Scr_AddConstString( scr_const.allies );
		break;
	case TEAM_AXIS:
		Scr_AddConstString( scr_const.axis );
		break;
	case TEAM_SPECTATOR:
		Scr_AddConstString( scr_const.spectator );
		break;
	case TEAM_FREE:
		Scr_AddConstString( scr_const.none );
		break;
	default:
		break;
	}
}

static void ClientScr_SetSessionState( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	unsigned short string;

	string = Scr_GetConstString( 0 );

	if ( string == scr_const.playing ) {
		client->sess.sessionState = STATE_PLAYING;
	} else if ( string == scr_const.dead ) {
		client->sess.sessionState = STATE_DEAD;
	} else if ( string == scr_const.spectator ) {
		client->sess.sessionState = STATE_SPECTATOR;
	} else if ( string == scr_const.intermission ) {
		client->sess.sessionState = STATE_INTERMISSION;
	} else {
		Scr_Error( va( "'%s' is an illegal sessionstate string. Must be playing, dead, spectator, or intermission.",
					   SL_ConvertToString( string ) ) );
	}
}

static void ClientScr_GetSessionState( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	switch ( client->sess.sessionState ) {
	case STATE_PLAYING:
		Scr_AddConstString( scr_const.playing );
		break;
	case STATE_DEAD:
		Scr_AddConstString( scr_const.dead );
		break;
	case STATE_SPECTATOR:
		Scr_AddConstString( scr_const.spectator );
		break;
	case STATE_INTERMISSION:
		Scr_AddConstString( scr_const.intermission );
		break;
	default:
		break;
	}
}

static void ClientScr_SetMaxHealth( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	client->sess.maxHealth = Scr_GetInt( 0 );

	if ( client->sess.maxHealth < 1 ) {
		client->sess.maxHealth = 1;
	}

	if ( client->ps.stats[STAT_HEALTH] > client->sess.maxHealth ) {
		client->ps.stats[STAT_HEALTH] = client->sess.maxHealth;
	}

	g_entities[client - level.clients].health = client->ps.stats[STAT_HEALTH];
	client->ps.stats[STAT_MAX_HEALTH] = client->sess.maxHealth;
}

static void ClientScr_SetScore( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	client->sess.score = Scr_GetInt( 0 );
	CalculateRanks();
}

static void ClientScr_SetSpectatorClient( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	int num;

	num = Scr_GetInt( 0 );

	/* Scr_Error does not return, so the store below is reached only on the
	   valid path even though the compiler shares it. */
	if ( num < -1 || num >= MAX_CLIENTS ) {
		Scr_Error( "spectatorclient can only be set to -1, or a valid client number" );
	}

	client->sess.forceSpectatorClient = num;
}

static void ClientScr_SetStatusIcon( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	client->sess.statusIcon = GScr_GetStatusIconIndex( Scr_GetString( 0 ) );
}

static void ClientScr_GetStatusIcon( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	char string[MAX_STRING_CHARS];

	if ( !client->sess.statusIcon ) {
		Scr_AddString( "" );
		return;
	}

	/* retail pushes nothing at all when the stored index is out of range */
	if ( client->sess.statusIcon > MAX_STATUSICONS ) {
		return;
	}

	trap_GetConfigstring( CS_STATUSICONS + client->sess.statusIcon - 1, string, sizeof( string ) );
	Scr_AddString( string );
}

static void ClientScr_SetHeadIcon( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	gentity_t *ent;

	ent = &g_entities[client - level.clients];
	ent->s.iHeadIcon = GScr_GetHeadIconIndex( Scr_GetString( 0 ) );
}

static void ClientScr_GetHeadIcon( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	gentity_t *ent;
	char string[MAX_STRING_CHARS];

	ent = &g_entities[client - level.clients];

	if ( !ent->s.iHeadIcon ) {
		Scr_AddString( "" );
		return;
	}

	if ( ent->s.iHeadIcon > MAX_HEADICONS ) {
		return;
	}

	trap_GetConfigstring( CS_HEADICONS + ent->s.iHeadIcon - 1, string, sizeof( string ) );
	Scr_AddString( string );
}

static void ClientScr_SetHeadIconTeam( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	gentity_t *ent;
	unsigned short string;

	ent = &g_entities[client - level.clients];
	string = Scr_GetConstString( 0 );

	if ( string == scr_const.none ) {
		ent->s.iHeadIconTeam = TEAM_FREE;
	} else if ( string == scr_const.allies ) {
		ent->s.iHeadIconTeam = TEAM_ALLIES;
	} else if ( string == scr_const.axis ) {
		ent->s.iHeadIconTeam = TEAM_AXIS;
	} else if ( string != scr_const.spectator ) {
		/* The retail test really is inverted: "spectator" -- the one value the
		   error message names as legal -- is the only string that errors, and
		   every unknown string silently becomes TEAM_SPECTATOR. */
		ent->s.iHeadIconTeam = TEAM_SPECTATOR;
	} else {
		Scr_Error( va( "'%s' is an illegal head icon team string. Must be none, allies, axis, or spectator.",
					   SL_ConvertToString( string ) ) );
	}
}

static void ClientScr_GetHeadIconTeam( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	gentity_t *ent;

	ent = &g_entities[client - level.clients];

	switch ( ent->s.iHeadIconTeam ) {
	case TEAM_AXIS:
		Scr_AddConstString( scr_const.axis );
		break;
	case TEAM_ALLIES:
		Scr_AddConstString( scr_const.allies );
		break;
	case TEAM_SPECTATOR:
		Scr_AddConstString( scr_const.spectator );
		break;
	default:
		Scr_AddConstString( scr_const.none );
		break;
	}
}

static void ClientScr_SetArchiveTime( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	client->sess.archiveTime = (int)( Scr_GetFloat( 0 ) * 1000.0f );
}

static void ClientScr_GetArchiveTime( gclient_t *client, gclient_t *self, const clientField_t *field ) {
	Scr_AddFloat( client->sess.archiveTime * 0.001f );
}

clientField_t clientFields[] = {
	{ "name",               CFOFS( sess.name ),                     F_CSTRING,      ClientScr_ReadOnly,             NULL },
	{ "sessionteam",        0,                                      F_STRING,       ClientScr_SetSessionTeam,       ClientScr_GetSessionTeam },
	{ "sessionstate",       0,                                      F_STRING,       ClientScr_SetSessionState,      ClientScr_GetSessionState },
	{ "maxhealth",          CFOFS( sess.maxHealth ),                F_INT,          ClientScr_SetMaxHealth,         NULL },
	{ "handicap",           CFOFS( sess.handicap ),                 F_INT,          ClientScr_ReadOnly,             NULL },
	{ "score",              CFOFS( sess.score ),                    F_INT,          ClientScr_SetScore,             NULL },
	{ "deaths",             CFOFS( sess.deaths ),                   F_INT,          NULL,                           NULL },
	{ "statusicon",         0,                                      F_STRING,       ClientScr_SetStatusIcon,        ClientScr_GetStatusIcon },
	{ "headicon",           0,                                      F_STRING,       ClientScr_SetHeadIcon,          ClientScr_GetHeadIcon },
	{ "headiconteam",       0,                                      F_STRING,       ClientScr_SetHeadIconTeam,      ClientScr_GetHeadIconTeam },
	{ "spectatorclient",    CFOFS( sess.forceSpectatorClient ),     F_INT,          ClientScr_SetSpectatorClient,   NULL },
	{ "archivetime",        CFOFS( sess.archiveTime ),              F_FLOAT,        ClientScr_SetArchiveTime,       ClientScr_GetArchiveTime },
	{ "pers",               CFOFS( sess.pers ),                     F_OBJECT,       ClientScr_ReadOnly,             NULL },

	{ NULL, 0, F_INT, NULL, NULL }
};

void GScr_AddFieldsForClient( unsigned short classnum ) {
	clientField_t *f;
	int i;

	for ( i = 0, f = clientFields ; f->name ; f++, i++ ) {
		switch ( f->type ) {
		case F_INT:
		case F_FLOAT:
		case F_CSTRING:
		case F_STRING:
		case F_VECTOR:
		case F_ENTITY:
		case F_OBJECT:
			Scr_AddClassField( classnum, f->name, (unsigned short)( i | CLIENT_FIELD_MASK ) );
			break;
		default:
			break;
		}
	}
}

void Scr_SetClientField( gclient_t *client, unsigned int fieldnum ) {
	clientField_t *f;

	f = &clientFields[fieldnum];
	if ( f->setter ) {
		f->setter( client, client, f );
		return;
	}

	Scr_SetGenericField( client, f->type, f->ofs );
}

void Scr_GetClientField( gclient_t *client, unsigned int fieldnum ) {
	clientField_t *f;

	f = &clientFields[fieldnum];
	if ( f->getter ) {
		f->getter( client, client, f );
		return;
	}

	Scr_GetGenericField( client, f->type, f->ofs );
}
