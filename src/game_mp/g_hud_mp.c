/*
 * @fidelity: likely
 *
 * g_hud_mp.c -- the server-side hudelem system.
 *
 * g_hudelems[] (0x2014F9A0) is a flat pool of 1024 records.  A record is live
 * when its type is non-zero; HudElem_UpdateClient walks the whole pool once per
 * client per frame and packs the records that client may see into the two
 * 31-entry arrays of playerState_t.hud, which is how a hudelem reaches a
 * client at all -- there is no per-client ownership list.
 *
 * The script side sees a hudelem as an object of its own class: hudelemFields[]
 * (0x200558D0) is the field table Scr_Set/GetHudElemField dispatch through, and
 * hudelemMethods[] (0x200559C0) is the method table Scr_GetMethod falls through
 * to after the player and entity tables.
 *
 * Function order is binary order (0x20021410 .. 0x20022760).
 */

#include <stdio.h>
#include <string.h>

#include "g_local.h"

/* forward: bg_slidemove.c */
int Q_ftol( float f );

/* HudElem_Alloc's scan bound (0x200214B7) and HudElem_DestroyAll's memset
 * length 126976 = 1024 * 124 (0x20021694). */
#define MAX_HUDELEMS            1024

/* The per-client half: q_shared.h's hudElemState_t is current[31] +
 * archival[31], and HudElem_UpdateClient stops copying past 31 (0x20022802). */
#define MAX_CLIENT_HUDELEMS     31

/* hudelem_t.type.  Each value is set by one method:
 * HudElem_Alloc 1, HECmd_SetValue 2, HECmd_SetShader 3, and the six timer /
 * clock thunks 4..9. */
typedef enum {
	HE_TYPE_FREE,
	HE_TYPE_TEXT,
	HE_TYPE_VALUE,
	HE_TYPE_SHADER,
	HE_TYPE_TIMER_DOWN,
	HE_TYPE_TIMER_UP,
	HE_TYPE_TENTHS_TIMER_DOWN,
	HE_TYPE_TENTHS_TIMER_UP,
	HE_TYPE_CLOCK_DOWN,
	HE_TYPE_CLOCK_UP
} hudElemType_t;

/*
 * The field types, duplicated from g_spawn_mp.c -- fields[], clientFields[] and
 * hudelemFields[] share them and no header carries the enum yet.
 */
typedef enum {
	F_INT,
	F_FLOAT,
	F_CSTRING,
	F_STRING,               /* const string id; the only type this unit frees */
	F_VECTOR,
	F_ENTITY,
	F_VECTOR_Y,
	F_OBJECT,
	F_MODEL,
	F_IGNORE
} fieldtype_t;

/* Scr_AddEntityNum / Scr_FreeEntityNum class number of a hudelem; g_spawn_mp.c
 * carries the same constant. */
#ifndef SCR_OBJECT_HUDELEM
#define SCR_OBJECT_HUDELEM      1
#endif

typedef struct hudElemField_s hudElemField_t;
typedef void ( *hudElemFieldFunc_t )( g_hudelem_t *elem, int fieldnum );

struct hudElemField_s {
	const char          *name;
	int                 ofs;
	fieldtype_t         type;
	hudElemFieldFunc_t  setter;     /* NULL falls through to Scr_SetGenericField */
	hudElemFieldFunc_t  getter;     /* NULL falls through to Scr_GetGenericField */
};

typedef struct hudElemMethod_s {
	const char  *name;
	void        ( *call )( int elemnum );
} hudElemMethod_t;

#define HEOFS( x )      ( (int)&( ( (g_hudelem_t *)0 )->x ) )

static void HudElem_SetLocalizedString( g_hudelem_t *elem, int fieldnum );
static void HudElem_SetBoolean( g_hudelem_t *elem, int fieldnum );
static void HudElem_SetColor( g_hudelem_t *elem, int fieldnum );
static void HudElem_GetColor( g_hudelem_t *elem, int fieldnum );
static void HudElem_SetAlpha( g_hudelem_t *elem, int fieldnum );
static void HudElem_GetAlpha( g_hudelem_t *elem, int fieldnum );
static void HudElem_SetFontScale( g_hudelem_t *elem, int fieldnum );
static void HudElem_SetFont( g_hudelem_t *elem, int fieldnum );
static void HudElem_GetFont( g_hudelem_t *elem, int fieldnum );
static void HudElem_SetAlignX( g_hudelem_t *elem, int fieldnum );
static void HudElem_GetAlignX( g_hudelem_t *elem, int fieldnum );
static void HudElem_SetAlignY( g_hudelem_t *elem, int fieldnum );
static void HudElem_GetAlignY( g_hudelem_t *elem, int fieldnum );

static void HECmd_SetText( int elemnum );
static void HECmd_SetShader( int elemnum );
static void HECmd_SetTimer( int elemnum );
static void HECmd_SetTimerUp( int elemnum );
static void HECmd_SetTenthsTimer( int elemnum );
static void HECmd_SetTenthsTimerUp( int elemnum );
static void HECmd_SetClock( int elemnum );
static void HECmd_SetClockUp( int elemnum );
static void HECmd_SetValue( int elemnum );
static void HECmd_FadeOverTime( int elemnum );
static void HECmd_ScaleOverTime( int elemnum );
static void HECmd_MoveOverTime( int elemnum );
static void HECmd_Reset( int elemnum );
static void HECmd_Destroy( int elemnum );

g_hudelem_t g_hudelems[MAX_HUDELEMS];

static const hudElemField_t hudelemFields[] = {
	{ "x",          HEOFS( x ),         F_INT,      NULL,                       NULL },
	{ "y",          HEOFS( y ),         F_INT,      NULL,                       NULL },
	{ "fontscale",  HEOFS( fontScale ), F_FLOAT,    HudElem_SetFontScale,       NULL },
	{ "font",       HEOFS( font ),      F_INT,      HudElem_SetFont,            HudElem_GetFont },
	{ "alignx",     HEOFS( alignX ),    F_INT,      HudElem_SetAlignX,          HudElem_GetAlignX },
	{ "aligny",     HEOFS( alignY ),    F_INT,      HudElem_SetAlignY,          HudElem_GetAlignY },
	{ "color",      HEOFS( color ),     F_INT,      HudElem_SetColor,           HudElem_GetColor },
	{ "alpha",      HEOFS( color ),     F_INT,      HudElem_SetAlpha,           HudElem_GetAlpha },
	{ "label",      HEOFS( label ),     F_INT,      HudElem_SetLocalizedString, NULL },
	{ "sort",       HEOFS( sortKey ),   F_FLOAT,    NULL,                       NULL },
	{ "archived",   HEOFS( archived ),  F_INT,      HudElem_SetBoolean,         NULL },

	{ NULL,         0,                  F_INT,      NULL,                       NULL }
};

static const hudElemMethod_t hudelemMethods[] = {
	{ "settext",            HECmd_SetText },
	{ "setshader",          HECmd_SetShader },
	{ "settimer",           HECmd_SetTimer },
	{ "settimerup",         HECmd_SetTimerUp },
	{ "settenthstimer",     HECmd_SetTenthsTimer },
	{ "settenthstimerup",   HECmd_SetTenthsTimerUp },
	{ "setclock",           HECmd_SetClock },
	{ "setclockup",         HECmd_SetClockUp },
	{ "setvalue",           HECmd_SetValue },
	{ "fadeovertime",       HECmd_FadeOverTime },
	{ "scaleovertime",      HECmd_ScaleOverTime },
	{ "moveovertime",       HECmd_MoveOverTime },
	{ "reset",              HECmd_Reset },
	{ "destroy",            HECmd_Destroy },

	{ NULL,                 NULL }
};

static const char *hudelemFontNames[] = {
	"default",
	"bigfixed",
	"smallfixed"
};

static const char *hudelemAlignXNames[] = {
	"left",
	"center",
	"right"
};

static const char *hudelemAlignYNames[] = {
	"top",
	"middle",
	"bottom"
};

/*
==================
HudElem_SetDefaults

Clears everything the element's type owns; every method that changes the type
starts here.  Deliberately leaves x/y, the colour, the label, the sort key and
the move block alone.
==================
*/
static void HudElem_SetDefaults( g_hudelem_t *elem ) {
	elem->width = 0;
	elem->height = 0;
	elem->materialIndex = 0;
	elem->scaleFromWidth = 0;
	elem->scaleFromHeight = 0;
	elem->scaleStartTime = 0;
	elem->scaleTime = 0;
	elem->timerValue = 0;
	elem->rotationPeriodMs = 0;
	elem->value = 0;
	elem->text = 0;
}

/*
==================
HudElem_Reset

Everything back to a plain white text element.  Name inferred: no retail
symbol -- the body survives only at 0x20021440, reached by the tail call in
HECmd_Reset and inlined into HudElem_Alloc.
==================
*/
static void HudElem_Reset( g_hudelem_t *elem ) {
	elem->type = HE_TYPE_TEXT;
	elem->x = 0;
	elem->y = 0;
	elem->fontScale = 1.0f;
	elem->font = 0;
	elem->alignX = 0;
	elem->alignY = 0;
	elem->color = -1;
	elem->fromColor = 0;
	elem->fadeStartTime = 0;
	elem->fadeTime = 0;
	elem->label = 0;
	elem->sortKey = 0;
	elem->archived = qtrue;

	HudElem_SetDefaults( elem );
}

/*
==================
HudElem_Alloc
==================
*/
static g_hudelem_t *HudElem_Alloc( int clientNum, int team ) {
	g_hudelem_t *elem;
	int         i;

	for ( i = 0, elem = g_hudelems; elem->type; i++, elem++ ) {
		if ( i + 1 >= MAX_HUDELEMS ) {
			return NULL;
		}
	}

	HudElem_Reset( elem );

	elem->clientNum = clientNum;
	elem->team = team;

	return elem;
}

/*
==================
HudElem_Free
==================
*/
static void HudElem_Free( g_hudelem_t *elem ) {
	Scr_FreeHudElemConstStrings( elem );
	Scr_FreeHudElem( elem );
	elem->type = HE_TYPE_FREE;
}

/*
==================
HudElem_ClientDisconnect
==================
*/
void HudElem_ClientDisconnect( gentity_t *ent ) {
	g_hudelem_t *elem;
	int         i;

	for ( i = 0, elem = g_hudelems; i < MAX_HUDELEMS; i++, elem++ ) {
		if ( !elem->type ) {
			continue;
		}
		if ( elem->clientNum == ent->s.number ) {
			HudElem_Free( elem );
		}
	}
}

/*
==================
HudElem_DestroyAll
==================
*/
void HudElem_DestroyAll( void ) {
	g_hudelem_t *elem;
	int         i;

	for ( i = 0, elem = g_hudelems; i < MAX_HUDELEMS; i++, elem++ ) {
		if ( elem->type ) {
			HudElem_Free( elem );
		}
	}

	memset( g_hudelems, 0, sizeof( g_hudelems ) );
}

/*
==================
HudElem_SetEnumString
==================
*/
static void HudElem_SetEnumString( g_hudelem_t *elem, const hudElemField_t *field,
								   const char **names, int count ) {
	int         i;
	const char  *string;
	char        buffer[2048];

	string = Scr_GetString( 0 );

	for ( i = 0; i < count; i++ ) {
		if ( !Q_stricmp( names[i], string ) ) {
			*(int *)( (byte *)elem + field->ofs ) = i;
			return;
		}
	}

	sprintf( buffer, "\"%s\" is not a valid value for hudelem field \"%s\"\nShould be one of:",
			 string, field->name );
	/* The length is the whole buffer, not what is left of it -- retail passes
	   0x800 at 0x2002173C, so a long enough name list runs past the frame.
	   Reproduced as-is; the truncation on the next line is retail's too. */
	for ( i = 0; i < count; i++ ) {
		strncat( buffer, va( " %s", names[i] ), sizeof( buffer ) );
		buffer[sizeof( buffer ) - 1] = 0;
	}
	Scr_Error( buffer );
}

/*
==================
HudElem_GetEnumString
==================
*/
static void HudElem_GetEnumString( g_hudelem_t *elem, const hudElemField_t *field,
								   const char **names ) {
	Scr_AddString( names[ *(int *)( (byte *)elem + field->ofs ) ] );
}

/*
==================
HudElem_SetLocalizedString
==================
*/
static void HudElem_SetLocalizedString( g_hudelem_t *elem, int fieldnum ) {
	*(int *)( (byte *)elem + hudelemFields[fieldnum].ofs ) =
		G_LocalizedStringIndex( Scr_GetIString( 0 ) );
}

/*
==================
HudElem_SetBoolean
==================
*/
static void HudElem_SetBoolean( g_hudelem_t *elem, int fieldnum ) {
	*(int *)( (byte *)elem + hudelemFields[fieldnum].ofs ) = Scr_GetBool( 0 );
}

/*
==================
HudElem_SetColor

The alpha byte is left alone; setColor and setAlpha share the field offset but
neither reads it -- both address the packed colour directly.

Each component is clamped to 0..1 and scaled to a byte in place (retail
0x20021830 .. 0x2002195D has the three copies back to back, no helper).  Each
byte goes through Q_ftol; retail inlines its fld/fadd/fistp body at
0x2002188B/0x200218F2/0x20021955.
==================
*/
static void HudElem_SetColor( g_hudelem_t *elem, int fieldnum ) {
	vec3_t  color;
	byte    *rgba;
	float   f;

	Scr_GetVector( 0, color );

	rgba = (byte *)&elem->color;

	f = color[0];
	if ( f > 1.0f ) {
		f = 1.0f;
	} else if ( f < 0.0f ) {
		f = 0.0f;
	}
	f = f * 255.0f;
	rgba[0] = Q_ftol( f );

	f = color[1];
	if ( f > 1.0f ) {
		f = 1.0f;
	} else if ( f < 0.0f ) {
		f = 0.0f;
	}
	f = f * 255.0f;
	rgba[1] = Q_ftol( f );

	f = color[2];
	if ( f > 1.0f ) {
		f = 1.0f;
	} else if ( f < 0.0f ) {
		f = 0.0f;
	}
	f = f * 255.0f;
	rgba[2] = Q_ftol( f );
}

/*
==================
HudElem_GetColor
==================
*/
static void HudElem_GetColor( g_hudelem_t *elem, int fieldnum ) {
	const byte  *rgba;
	vec3_t      color;

	rgba = (const byte *)&elem->color;
	color[0] = rgba[0] * ( 1.0f / 255.0f );
	color[1] = rgba[1] * ( 1.0f / 255.0f );
	color[2] = rgba[2] * ( 1.0f / 255.0f );

	Scr_AddVector( color );
}

/*
==================
HudElem_SetAlpha
==================
*/
static void HudElem_SetAlpha( g_hudelem_t *elem, int fieldnum ) {
	byte    *rgba;
	float   alpha;

	alpha = Scr_GetFloat( 0 );
	if ( alpha > 1.0f ) {
		alpha = 1.0f;
	} else if ( alpha < 0.0f ) {
		alpha = 0.0f;
	}
	alpha = alpha * 255.0f;

	rgba = (byte *)&elem->color;
	rgba[3] = Q_ftol( alpha );
}

/*
==================
HudElem_GetAlpha
==================
*/
static void HudElem_GetAlpha( g_hudelem_t *elem, int fieldnum ) {
	const byte *rgba;

	rgba = (const byte *)&elem->color;
	Scr_AddFloat( rgba[3] * ( 1.0f / 255.0f ) );
}

/*
==================
HudElem_SetFontScale
==================
*/
static void HudElem_SetFontScale( g_hudelem_t *elem, int fieldnum ) {
	float scale;

	scale = Scr_GetFloat( 0 );
	if ( scale <= 0.0f ) {
		Scr_Error( va( "font scale was %g; should be > 0", scale ) );
	}

	elem->fontScale = scale;
}

/*
==================
HudElem_SetFont
==================
*/
static void HudElem_SetFont( g_hudelem_t *elem, int fieldnum ) {
	HudElem_SetEnumString( elem, &hudelemFields[fieldnum], hudelemFontNames, 3 );
}

/*
==================
HudElem_GetFont
==================
*/
static void HudElem_GetFont( g_hudelem_t *elem, int fieldnum ) {
	HudElem_GetEnumString( elem, &hudelemFields[fieldnum], hudelemFontNames );
}

/*
==================
HudElem_SetAlignX
==================
*/
static void HudElem_SetAlignX( g_hudelem_t *elem, int fieldnum ) {
	HudElem_SetEnumString( elem, &hudelemFields[fieldnum], hudelemAlignXNames, 3 );
}

/*
==================
HudElem_GetAlignX
==================
*/
static void HudElem_GetAlignX( g_hudelem_t *elem, int fieldnum ) {
	HudElem_GetEnumString( elem, &hudelemFields[fieldnum], hudelemAlignXNames );
}

/*
==================
HudElem_SetAlignY
==================
*/
static void HudElem_SetAlignY( g_hudelem_t *elem, int fieldnum ) {
	HudElem_SetEnumString( elem, &hudelemFields[fieldnum], hudelemAlignYNames, 3 );
}

/*
==================
HudElem_GetAlignY
==================
*/
static void HudElem_GetAlignY( g_hudelem_t *elem, int fieldnum ) {
	HudElem_GetEnumString( elem, &hudelemFields[fieldnum], hudelemAlignYNames );
}

/*
==================
Scr_GetHudElemField
==================
*/
void Scr_GetHudElemField( int elemnum, int fieldnum ) {
	g_hudelem_t             *elem;
	const hudElemField_t    *field;

	elem = &g_hudelems[elemnum];
	field = &hudelemFields[fieldnum];

	if ( field->getter ) {
		field->getter( elem, fieldnum );
	} else {
		Scr_GetGenericField( elem, field->type, field->ofs );
	}
}

/*
==================
Scr_SetHudElemField
==================
*/
void Scr_SetHudElemField( int elemnum, int fieldnum ) {
	g_hudelem_t             *elem;
	const hudElemField_t    *field;

	elem = &g_hudelems[elemnum];
	field = &hudelemFields[fieldnum];

	if ( field->setter ) {
		field->setter( elem, fieldnum );
	} else {
		Scr_SetGenericField( elem, field->type, field->ofs );
	}
}

/*
==================
Scr_FreeHudElemConstStrings

No hudelem field is F_STRING in 1.1, so the loop never fires; the table walk is
the same one Scr_FreeEntityConstStrings does.
==================
*/
void Scr_FreeHudElemConstStrings( g_hudelem_t *elem ) {
	const hudElemField_t *field;

	for ( field = hudelemFields; field->name; field++ ) {
		if ( field->type == F_STRING ) {
			Scr_SetString( (unsigned short *)( (byte *)elem + field->ofs ), 0 );
		}
	}
}

/*
==================
GScr_NewHudElem
==================
*/
void GScr_NewHudElem( void ) {
	g_hudelem_t *elem;

	elem = HudElem_Alloc( ENTITYNUM_NONE, TEAM_FREE );
	if ( !elem ) {
		Scr_Error( "out of hudelems" );
	}

	Scr_AddHudElem( elem );
}

/*
==================
GScr_NewClientHudElem
==================
*/
void GScr_NewClientHudElem( void ) {
	gentity_t   *ent;
	g_hudelem_t *elem;

	ent = Scr_GetEntity( 0 );
	if ( !ent->client ) {
		Scr_ParamError( 0, "not a client" );
	}

	elem = HudElem_Alloc( ent->s.number, TEAM_FREE );
	if ( !elem ) {
		Scr_Error( "out of hudelems" );
	}

	Scr_AddHudElem( elem );
}

/*
==================
GScr_NewTeamHudElem
==================
*/
void GScr_NewTeamHudElem( void ) {
	unsigned short  string;
	team_t          team;
	g_hudelem_t     *elem;

	string = Scr_GetConstString( 0 );
	if ( string == scr_const.allies ) {
		team = TEAM_ALLIES;
	} else if ( string == scr_const.axis ) {
		team = TEAM_AXIS;
	} else if ( string == scr_const.spectator ) {
		team = TEAM_SPECTATOR;
	} else {
		Scr_ParamError( 0, va( "team \"%s\" should be \"allies\", \"axis\", or \"spectator\"",
							   Scr_GetString( 0 ) ) );
		team = TEAM_FREE;
	}

	elem = HudElem_Alloc( ENTITYNUM_NONE, team );
	if ( !elem ) {
		Scr_Error( "out of hudelems" );
	}

	Scr_AddHudElem( elem );
}

/*
==================
GScr_AddFieldsForHudElems
==================
*/
void GScr_AddFieldsForHudElems( void ) {
	const hudElemField_t *field;

	for ( field = hudelemFields; field->name; field++ ) {
		switch ( field->type ) {
		case F_INT:
		case F_FLOAT:
		case F_CSTRING:
		case F_STRING:
		case F_VECTOR:
		case F_ENTITY:
		case F_OBJECT:
		case F_MODEL:
			Scr_AddClassField( g_scr_data.classMap[SCR_CLASS_HUDELEM].classnum, field->name,
							   (unsigned short)( field - hudelemFields ) );
			break;
		default:
			break;
		}
	}
}

/*
==================
HECmd_SetText
==================
*/
static void HECmd_SetText( int elemnum ) {
	g_hudelem_t *elem;
	const char  *string;

	elem = &g_hudelems[elemnum];
	string = Scr_GetIString( 0 );

	HudElem_SetDefaults( elem );
	elem->type = HE_TYPE_TEXT;
	elem->text = G_LocalizedStringIndex( string );
}

/*
==================
HECmd_SetShader
==================
*/
static void HECmd_SetShader( int elemnum ) {
	g_hudelem_t *elem;
	int         numParam;
	int         shader;
	int         width;
	int         height;

	numParam = Scr_GetNumParam();
	if ( numParam != 1 && numParam != 3 ) {
		Scr_Error( "USAGE: <hudelem> setShader(\"shadername\"[, optional_width, optional_height]);" );
	}

	elem = &g_hudelems[elemnum];
	shader = G_ShaderIndex( Scr_GetString( 0 ) );

	if ( numParam == 1 ) {
		width = 0;
		height = 0;
	} else {
		width = Scr_GetInt( 1 );
		if ( width < 0 ) {
			Scr_ParamError( 1, va( "width %i < 0", width ) );
		}
		height = Scr_GetInt( 2 );
		if ( height < 0 ) {
			Scr_ParamError( 2, va( "height %i < 0", height ) );
		}
	}

	HudElem_SetDefaults( elem );
	elem->type = HE_TYPE_SHADER;
	elem->width = width;
	elem->height = height;
	elem->materialIndex = shader;
}

/*
==================
HECmd_SetTimer_Internal

Q_rint's 0.5 - 2^-30 bias under a round-to-nearest fistp is a ceil: any
fraction at all pushes the millisecond count up (retail inlines the body at
0x200220A8; SetClock's two sites are 0x2002218B/0x200221EE).
==================
*/
static void HECmd_SetTimer_Internal( int elemnum, hudElemType_t type, const char *name ) {
	g_hudelem_t *elem;
	float       seconds;
	int         msec;

	if ( Scr_GetNumParam() != 1 ) {
		Scr_Error( va( "USAGE: <hudelem> %s(time_in_seconds);\n", name ) );
	}

	elem = &g_hudelems[elemnum];

	seconds = Scr_GetFloat( 0 ) * 1000.0f;
	msec = Q_rint( seconds );
	if ( msec <= 0 && type != HE_TYPE_TIMER_UP ) {
		Scr_ParamError( 0, va( "time %g should be > 0", msec * 0.001f ) );
	}

	HudElem_SetDefaults( elem );
	elem->type = type;
	elem->timerValue = level.time + msec;
}

/*
==================
HECmd_SetClock_Internal
==================
*/
static void HECmd_SetClock_Internal( int elemnum, hudElemType_t type, const char *name ) {
	g_hudelem_t *elem;
	int         numParam;
	float       seconds;
	int         msec;
	int         duration;
	int         shader;
	int         width;
	int         height;

	numParam = Scr_GetNumParam();
	if ( numParam != 3 && numParam != 5 ) {
		Scr_Error( va( "USAGE: <hudelem> %s(time_in_seconds, total_clock_time_in_seconds, shadername[, width, height]);\n",
					   name ) );
	}

	elem = &g_hudelems[elemnum];

	seconds = Scr_GetFloat( 0 ) * 1000.0f;
	msec = Q_rint( seconds );
	if ( msec <= 0 && type != HE_TYPE_CLOCK_UP ) {
		Scr_ParamError( 0, va( "time %g should be > 0", msec * 0.001f ) );
	}

	seconds = Scr_GetFloat( 1 ) * 1000.0f;
	duration = Q_rint( seconds );
	if ( duration <= 0 ) {
		Scr_ParamError( 1, va( "duration %g should be > 0", duration * 0.001f ) );
	}

	shader = G_ShaderIndex( Scr_GetString( 2 ) );

	if ( numParam == 3 ) {
		width = 0;
		height = 0;
	} else {
		width = Scr_GetInt( 3 );
		if ( width < 0 ) {
			Scr_ParamError( 3, va( "width %i < 0", width ) );
		}
		height = Scr_GetInt( 4 );
		if ( height < 0 ) {
			Scr_ParamError( 4, va( "height %i < 0", height ) );
		}
	}

	HudElem_SetDefaults( elem );
	elem->type = type;
	elem->timerValue = level.time + msec;
	elem->width = width;
	elem->height = height;
	elem->materialIndex = shader;
	elem->rotationPeriodMs = duration;
}

/*
==================
HECmd_SetTimer
==================
*/
static void HECmd_SetTimer( int elemnum ) {
	HECmd_SetTimer_Internal( elemnum, HE_TYPE_TIMER_DOWN, "setTimer" );
}

/*
==================
HECmd_SetTimerUp
==================
*/
static void HECmd_SetTimerUp( int elemnum ) {
	HECmd_SetTimer_Internal( elemnum, HE_TYPE_TIMER_UP, "setTimerUp" );
}

/*
==================
HECmd_SetTenthsTimer
==================
*/
static void HECmd_SetTenthsTimer( int elemnum ) {
	HECmd_SetTimer_Internal( elemnum, HE_TYPE_TENTHS_TIMER_DOWN, "setTenthsTimer" );
}

/*
==================
HECmd_SetTenthsTimerUp
==================
*/
static void HECmd_SetTenthsTimerUp( int elemnum ) {
	HECmd_SetTimer_Internal( elemnum, HE_TYPE_TENTHS_TIMER_UP, "setTenthsTimerUp" );
}

/*
==================
HECmd_SetClock
==================
*/
static void HECmd_SetClock( int elemnum ) {
	HECmd_SetClock_Internal( elemnum, HE_TYPE_CLOCK_DOWN, "setClock" );
}

/*
==================
HECmd_SetClockUp
==================
*/
static void HECmd_SetClockUp( int elemnum ) {
	HECmd_SetClock_Internal( elemnum, HE_TYPE_CLOCK_UP, "setClockUp" );
}

/*
==================
HECmd_SetValue
==================
*/
static void HECmd_SetValue( int elemnum ) {
	g_hudelem_t *elem;
	float       value;

	elem = &g_hudelems[elemnum];
	value = Scr_GetFloat( 0 );

	HudElem_SetDefaults( elem );
	elem->value = value;
	elem->type = HE_TYPE_VALUE;
}

/*
==================
HECmd_FadeOverTime
==================
*/
static void HECmd_FadeOverTime( int elemnum ) {
	g_hudelem_t *elem;
	float       seconds;

	elem = &g_hudelems[elemnum];

	seconds = Scr_GetFloat( 0 );
	if ( seconds <= 0.0f ) {
		Scr_ParamError( 0, va( "fade time %g <= 0", seconds ) );
	} else if ( seconds > 60.0f ) {
		Scr_ParamError( 0, va( "fade time %g > 60", seconds ) );
	}

	elem->fadeStartTime = level.time;
	elem->fadeTime = Q_ftol( seconds * 1000.0f );
	elem->fromColor = elem->color;
}

/*
==================
HECmd_ScaleOverTime
==================
*/
static void HECmd_ScaleOverTime( int elemnum ) {
	g_hudelem_t *elem;
	float       seconds;
	int         width;
	int         height;

	elem = &g_hudelems[elemnum];

	if ( Scr_GetNumParam() != 3 ) {
		Scr_Error( "hudelem scaleOverTime(time_in_seconds, new_width, new_height)" );
	}

	seconds = Scr_GetFloat( 0 );
	if ( seconds <= 0.0f ) {
		Scr_ParamError( 0, va( "scale time %g <= 0", seconds ) );
	} else if ( seconds > 60.0f ) {
		Scr_ParamError( 0, va( "scale time %g > 60", seconds ) );
	}

	width = Scr_GetInt( 1 );
	height = Scr_GetInt( 2 );

	elem->scaleStartTime = level.time;
	elem->scaleTime = Q_ftol( seconds * 1000.0f );
	elem->scaleFromWidth = elem->width;
	elem->scaleFromHeight = elem->height;
	elem->width = width;
	elem->height = height;
}

/*
==================
HECmd_MoveOverTime
==================
*/
static void HECmd_MoveOverTime( int elemnum ) {
	g_hudelem_t *elem;
	float       seconds;

	elem = &g_hudelems[elemnum];

	seconds = Scr_GetFloat( 0 );
	if ( seconds <= 0.0f ) {
		Scr_ParamError( 0, va( "move time %g <= 0", seconds ) );
	} else if ( seconds > 60.0f ) {
		Scr_ParamError( 0, va( "move time %g > 60", seconds ) );
	}

	elem->moveStartTime = level.time;
	elem->moveTime = Q_ftol( seconds * 1000.0f );
	elem->moveFromX = elem->x;
	elem->moveFromY = elem->y;
}

/*
==================
HECmd_Reset
==================
*/
static void HECmd_Reset( int elemnum ) {
	HudElem_Reset( &g_hudelems[elemnum] );
}

/*
==================
HECmd_Destroy
==================
*/
static void HECmd_Destroy( int elemnum ) {
	HudElem_Free( &g_hudelems[elemnum] );
}

/*
==================
HudElem_GetMethod

Scr_GetMethod's third and last try; it has already cleared *type, which is why
nothing here writes it.
==================
*/
void ( *HudElem_GetMethod( const char **pName, int *type ) )( int elemnum ) {
	int i;

	for ( i = 0; i < 14; i++ ) {
		if ( !strcmp( *pName, hudelemMethods[i].name ) ) {
			*pName = hudelemMethods[i].name;
			return hudelemMethods[i].call;
		}
	}

	return NULL;
}

/*
==================
HudElem_UpdateClient

Repacks one client's two hudelem arrays out of the global pool.  Anything past
MAX_CLIENT_HUDELEMS is counted and dropped, not written.
==================
*/
void HudElem_UpdateClient( gclient_t *client, int entNum, int flags ) {
	g_hudelem_t *elem;
	hudelem_t   *he;
	int         i;
	int         archivalCount;
	int         currentCount;

	if ( flags & HE_UPDATE_ARCHIVAL ) {
		memset( client->ps.hud.archival, 0, sizeof( client->ps.hud.archival ) );
	}
	if ( flags & HE_UPDATE_CURRENT ) {
		memset( client->ps.hud.current, 0, sizeof( client->ps.hud.current ) );
	}

	archivalCount = 0;
	currentCount = 0;

	for ( i = 0, elem = g_hudelems; i < MAX_HUDELEMS; i++, elem++ ) {
		if ( !elem->type ) {
			continue;
		}
		if ( elem->team && elem->team != client->sess.sessionTeam ) {
			continue;
		}
		if ( elem->clientNum != ENTITYNUM_NONE && elem->clientNum != entNum ) {
			continue;
		}

		if ( elem->archived ) {
			if ( !( flags & HE_UPDATE_ARCHIVAL ) ) {
				continue;
			}
			he = &client->ps.hud.archival[archivalCount++];
			if ( archivalCount > MAX_CLIENT_HUDELEMS ) {
				continue;
			}
		} else {
			if ( !( flags & HE_UPDATE_CURRENT ) ) {
				continue;
			}
			he = &client->ps.hud.current[currentCount++];
			if ( currentCount > MAX_CLIENT_HUDELEMS ) {
				continue;
			}
		}

		memcpy( he, elem, sizeof( *he ) );
	}
}
