/*
 * cg_scoreboard_mp.c -- the tab scoreboard.
 * (original source: cgame/cg_scoreboard.c)
 *
 * cgame_mp_x86.dll 0x30029AC0 .. 0x3002B919, sixteen functions.
 *
 * RTCW's cgame/cg_scoreboard.c is the ancestor of CG_DrawScoreboard and
 * CG_DrawClientScore and of nothing else -- CoD threw the rest away and built
 * a column-table-driven list with its own banners, its own scrollbar and an
 * objective-text block above it.  Only CG_DrawScoreboard's fade/killerName
 * prologue is still recognisably RTCW's.
 *
 * The layout is one table: cg_scoreboardColumns below carries the field id,
 * the fraction of the list width, the localized header and the alignment for
 * each of the five columns, and CG_DrawScoreboard_ListColumnHeaders,
 * CG_DrawScoreboard_ListBanner and CG_DrawClientScore all walk it.
 *
 * CG_CheckDrawScoreboardLine has no call site in 1.1: CG_DrawClientScore
 * (0x3002ACE0) and CG_DrawScoreboard_ListBanner (0x3002A989) both carry it
 * inlined, with 12 and 24 for the row height.
 *
 * @fidelity: likely
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cg_local.h"

/* g_local.h's team_t, which the cgame side cannot see. */
#ifndef TEAM_SPECTATOR
#define TEAM_FREE       0
#define TEAM_AXIS       1
#define TEAM_ALLIES     2
#define TEAM_SPECTATOR  3
#endif

/*
 * The list geometry, all in 640x480 virtual units.  Every one of these is a
 * literal in the binary; they are named here for what the drawing uses them
 * for.
 */
#define SB_LIST_X               129.0f  /* left edge of a row */
#define SB_LIST_BOTTOM          432.0f  /* a row that would cross this is clipped */
#define SB_ROW_HEIGHT           12.0f
#define SB_BANNER_HEIGHT        28.0f   /* what ListHeight bills a banner */
#define SB_TEXT_BASELINE        10.5f
#define SB_HEADER_HEIGHT        14.0f
#define SB_TEXT_SCALE           0.22f
#define SB_HEADER_SCALE         0.30f
#define SB_BANNER_SCALE         0.32f
#define SB_STATUSICON_SIZE      13.0f

/* cg_scoreboardColumns[].align */
#define SB_ALIGN_LEFT           0
#define SB_ALIGN_RIGHT          2

/* cg_scoreboardColumns[].id */
#define SB_FIELD_NAME           0
#define SB_FIELD_SCORE          1
#define SB_FIELD_DEATHS         2
#define SB_FIELD_PING           3
#define SB_FIELD_STATUSICON     4

typedef struct scoreboardColumn_s {
	int         id;                     /* +0x00 read at ebp-4 (0x3002AE70) */
	float       widthFrac;              /* +0x04 fraction of the list width */
	const char *label;                  /* +0x08 localized header, "" for none */
	int         align;                  /* +0x0C */
} scoreboardColumn_t;

#define SB_NUM_COLUMNS      5

/* 0x30060D38, five 16-byte records. */
static const scoreboardColumn_t cg_scoreboardColumns[SB_NUM_COLUMNS] = {
	{ SB_FIELD_STATUSICON,  0.05f,  "",                 SB_ALIGN_LEFT  },
	{ SB_FIELD_NAME,        0.41f,  "",                 SB_ALIGN_LEFT  },
	{ SB_FIELD_SCORE,       0.16f,  "CGAME_SB_SCORE",   SB_ALIGN_RIGHT },
	{ SB_FIELD_DEATHS,      0.20f,  "CGAME_SB_DEATHS",  SB_ALIGN_RIGHT },
	{ SB_FIELD_PING,        0.18f,  "CGAME_SB_PING",    SB_ALIGN_RIGHT }
};

/* 0x30060D88, six 16-byte { x, y, w, h } records: the frame and the two
   separators CG_DrawScoreboard_Backdrop draws in "white". */
static const float cg_scoreboardFrame[6][4] = {
	{ 123.0f,  25.0f, 394.0f,   2.0f },
	{ 123.0f, 447.0f, 394.0f,   2.0f },
	{ 123.0f,  27.0f,   2.0f, 420.0f },
	{ 515.0f,  27.0f,   2.0f, 420.0f },
	{ 125.0f,  51.0f, 390.0f,   1.0f },
	{ 125.0f, 432.0f, 390.0f,   1.0f }
};

/* 0x3007497C -- which team's block was drawn first the last time the two team
   scores were level.  Initialised to TEAM_ALLIES in the binary's .data. */
static int cg_scoreboardFirstTeam = TEAM_ALLIES;

/* NOT static: cg_draw_mp.c (CG_DrawTeamOverlay) and cg_event_mp.c (CG_Obituary)
   both call it. */
void CG_DrawScoreboard_GetTeamColor( vec4_t color, int team );

/*
=================
CG_DrawScoreboard_Backdrop

The dimmed panel, its frame, and the four corner captions: gametype and map
along the top, hostname and server address along the bottom.  Both caption
pairs shrink their font until they fit.
=================
*/
static void CG_DrawScoreboard_Backdrop( float fade ) {
	vec4_t      color;
	qhandle_t   hShader;
	char        mapname[64];
	char        *p;
	const char  *gametype;
	const char  *address;
	float       scale;
	float       y;
	int         width;
	int         height;
	int         i;

	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = fade * 0.55f;
	trap_R_SetColor( color );

	hShader = trap_R_RegisterShaderNoMip( "black", 5 );
	trap_R_DrawStretchPic( cgs.screenXScale * 120.0f, cgs.screenYScale * 22.0f,
						   cgs.screenXScale * 400.0f, cgs.screenYScale * 430.0f,
						   0.0f, 0.0f, 1.0f, 1.0f, hShader );

	color[3] = fade * 0.1f;
	trap_R_SetColor( color );
	hShader = trap_R_RegisterShaderNoMip( "white", 5 );

	for ( i = 0; i < 6; i++ ) {
		trap_R_DrawStretchPic( cgs.screenXScale * cg_scoreboardFrame[i][0],
							   cgs.screenYScale * cg_scoreboardFrame[i][1],
							   cgs.screenXScale * cg_scoreboardFrame[i][2],
							   cgs.screenYScale * cg_scoreboardFrame[i][3],
							   0.0f, 0.0f, 1.0f, 1.0f, hShader );
	}

	/* the raw fade goes back into color[3] here (0x30029C58, `mov [esp+84h+var_48],
	   edx`); all four captions below draw at `fade`, not the frame's fade*0.1 */
	color[3] = fade;

	gametype = trap_SE_LocalizeMessage(
				   trap_UI_GetGameTypeDisplayName( cgs.gametype ),
				   "scoreboard gametype display" );

	height = trap_R_Text_Height( 0, 0.41f );
	trap_R_Text_Paint( SB_LIST_X, 51.0f - ( 24 - height ) * 0.5f, 0, 0.41f,
					   color, gametype, 0.0f, 0, 3 );

	strcpy( mapname, cgs.mapname );

	p = mapname + strlen( mapname ) - 4;
	if ( !Q_stricmpn( p, ".bsp", 4 ) ) {
		*p = 0;
	}

	p = mapname;
	if ( !Q_stricmpn( p, "maps", 4 ) && ( p[4] == '/' || p[4] == '\\' ) ) {
		p += 5;
	}
	if ( !Q_stricmpn( p, "mp", 2 ) && ( p[2] == '/' || p[2] == '\\' ) ) {
		p += 3;
	}

	/* only the map name shrinks: the gametype width is re-measured at a constant
	   0.41 every pass (edi is loaded once at 0x30029EC2 and pushed unchanged at
	   0x30029F10), so the pair can never actually fit under 386 by itself */
	scale = 0.41f;
	while ( trap_R_Text_Width( p, 0, scale, 0 )
			+ trap_R_Text_Width( gametype, 0, 0.41f, 0 ) + 4 > 386
			&& scale > 0.075f ) {
		scale -= 0.025f;
	}

	width = trap_R_Text_Width( p, 0, scale, 0 ) + 4;
	height = trap_R_Text_Height( 0, scale );
	trap_R_Text_Paint( 511.0f - width, 51.0f - ( 24 - height ) * 0.5f, 0, scale,
					   color, p, 0.0f, 0, 3 );

	address = trap_CL_GetServerIPAddress();
	if ( address && !Q_stricmpn( "0.0.0.0:0", address, 99999 ) ) {
		address = CG_SafeTranslateString_Internal( "cgame", "CGAME_LISTENSERVER" );
	}

	scale = 0.2f;
	while ( trap_R_Text_Width( address, 0, scale, 0 )
			+ trap_R_Text_Width( cgs.sv_hostname, 0, scale, 0 ) + 4 > 386
			&& scale > 0.075f ) {
		scale -= 0.01f;
	}

	height = trap_R_Text_Height( 0, scale );
	y = 447.0f - ( 14 - height ) * 0.5f;
	trap_R_Text_Paint( SB_LIST_X, y, 0, scale, color, cgs.sv_hostname, 0.0f, 0, 3 );

	width = trap_R_Text_Width( address, 0, scale, 0 ) + 4;
	trap_R_Text_Paint( 511.0f - width, y, 0, scale, color, address, 0.0f, 0, 3 );
}

/*
=================
CG_DrawObjectiveInfo

The word-wrapped cg_objectiveText block above the list, then the separator
under it.  Returns the y the score list starts at.

The whole loading-screen block in the middle is CG_DrawInformation( 0 )
inlined -- trap_R_RegisterShaderNoMip calls it, and the compiler expanded it
here down to the dead `color` stores cg_info_mp.c already documents.
=================
*/
static float CG_DrawObjectiveInfo( const float *color, float y ) {
	const char  *text;
	const char  *line;          /* start of the line being measured */
	const char  *cursor;
	const char  *lastSpace;
	vec4_t      barColor;
	qhandle_t   hShader;

	if ( !cg_objectiveText.string[0] ) {
		return y;
	}

	y += 4.0f;

	text = CG_TranslateMessage( cg_objectiveText.string, "scoreboard objective info" );

	lastSpace = NULL;
	line = text;
	cursor = text;

	while ( line ) {
		if ( !cursor ) {
			break;
		}

		if ( *line == ' ' ) {
			line++;
		} else if ( *line == '\n' ) {
			line++;
			y += SB_ROW_HEIGHT;
		} else if ( *line == '\\' && line[1] == 'n' ) {
			line += 2;
			y += SB_ROW_HEIGHT;
		} else {
			char c;

			c = cursor[1];
			cursor++;

			if ( !c ) {
				trap_R_Text_Paint( SB_LIST_X, y + 9.0f, 0, 0.24f, color, line,
								   0.0f, cursor - line, 3 );
				y += SB_ROW_HEIGHT;
				break;
			}

			if ( c == '\n' || ( c == '\\' && cursor[1] == 'n' ) ) {
				trap_R_Text_Paint( SB_LIST_X, y + 9.0f, 0, 0.24f, color, line,
								   0.0f, cursor - line, 3 );
				line = ( *cursor == '\n' ) ? cursor + 1 : cursor + 2;
				y += SB_ROW_HEIGHT;
			} else if ( trap_R_Text_Width( line, 0, 0.24f, cursor - line ) > 374 ) {
				/* the limit is lastSpace - line, computed before lastSpace is
				   tested -- a null lastSpace hands the trap a negative limit */
				trap_R_Text_Paint( SB_LIST_X, y + 9.0f, 0, 0.24f, color, line,
								   0.0f, lastSpace - line, 3 );
				line = lastSpace ? lastSpace + 1 : cursor - 1;
				y += SB_ROW_HEIGHT;
			} else {
				if ( *cursor == ' ' ) {
					lastSpace = cursor;
				}
				continue;
			}
		}

		cursor = line;
		lastSpace = NULL;
	}

	y += 2.0f;

	CG_DrawInformation( 0 );

	hShader = trap_R_RegisterShaderNoMip( "white", 5 );
	barColor[0] = color[0];
	barColor[1] = color[1];
	barColor[2] = color[2];
	barColor[3] = color[3] * 0.1f;
	trap_R_SetColor( barColor );
	CG_DrawPic( 125.0f, y, 390.0f, 1.0f, hShader );

	return y + 1.0f;
}

/*
=================
CG_DrawScoreboard_ListColumnHeaders
=================
*/
static float CG_DrawScoreboard_ListColumnHeaders( float y, float width, const float *color ) {
	const char  *label;
	float       x;
	float       offset;
	int         i;

	x = SB_LIST_X;

	for ( i = 0; i < SB_NUM_COLUMNS; i++ ) {
		if ( cg_scoreboardColumns[i].label[0] ) {
			label = CG_SafeTranslateString_Internal( "cgame", cg_scoreboardColumns[i].label );

			if ( cg_scoreboardColumns[i].align == SB_ALIGN_RIGHT ) {
				offset = width * cg_scoreboardColumns[i].widthFrac
						 - trap_R_Text_Width( label, 0, SB_HEADER_SCALE, 0 );
			} else {
				offset = 0.0f;
			}

			trap_R_Text_Paint( x + offset, y + SB_TEXT_BASELINE, 0, SB_HEADER_SCALE,
							   color, label, 0.0f, 0, 3 );
		}

		x += width * cg_scoreboardColumns[i].widthFrac;
	}

	return y + SB_HEADER_HEIGHT;
}

/*
=================
CG_DrawScoreboard_ListHeight

How tall the whole list wants to be, and how many scrollable lines it has.
=================
*/
static float CG_DrawScoreboard_ListHeight( int *numLines ) {
	float   height;
	int     i;

	height = 10.0f;
	*numLines = 0;

	if ( cg.teamPlayers[TEAM_AXIS] || cg.teamPlayers[TEAM_ALLIES] ) {
		*numLines = 2;
		height = 66.0f;
	}
	if ( cg.teamPlayers[TEAM_FREE] ) {
		height += SB_BANNER_HEIGHT;
		( *numLines )++;
	}
	if ( cg.teamPlayers[TEAM_SPECTATOR] ) {
		height += SB_BANNER_HEIGHT;
		( *numLines )++;
	}

	for ( i = 0; i < cg.numScores; i++ ) {
		height += SB_ROW_HEIGHT;
		( *numLines )++;
	}

	return height;
}

/*
=================
CG_CheckDrawScoreboardLine

Advance past the scrolled-off lines and stop once the list runs off the
bottom.  No call site in 1.1 -- both users carry it inlined.
=================
*/
static qboolean CG_CheckDrawScoreboardLine( int *lineIndex, float y, float height ) {
	if ( cg.scoreboardListFull ) {
		return qfalse;
	}

	if ( *lineIndex < cg.scoreboardScrollPos ) {
		( *lineIndex )++;
		return qfalse;
	}

	if ( y + height > SB_LIST_BOTTOM ) {
		cg.scoreboardListFull = qtrue;
		return qfalse;
	}

	( *lineIndex )++;
	return qtrue;
}

/*
=================
CG_DrawScoreboard_GetTeamColor

g_TeamColor_Axis / g_TeamColor_Allies are "r g b" strings; anything else is
white.  Only the first three components are written -- every caller has
already put the fade into color[3].
=================
*/
void CG_DrawScoreboard_GetTeamColor( vec4_t color, int team ) {
	char buffer[1024];

	switch ( team ) {
	case TEAM_AXIS:
		trap_Cvar_VariableStringBuffer( "g_TeamColor_Axis", buffer, sizeof( buffer ) );
		sscanf( buffer, "%f %f %f", &color[0], &color[1], &color[2] );
		break;
	case TEAM_ALLIES:
		trap_Cvar_VariableStringBuffer( "g_TeamColor_Allies", buffer, sizeof( buffer ) );
		sscanf( buffer, "%f %f %f", &color[0], &color[1], &color[2] );
		break;
	default:
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
		break;
	}

	color[0] = Com_Clamp( 0.0f, 1.0f, color[0] );
	color[1] = Com_Clamp( 0.0f, 1.0f, color[1] );
	color[2] = Com_Clamp( 0.0f, 1.0f, color[2] );
}

/*
=================
CG_DrawScoreboard_ListBanner

One team's header row: the banner material, the team name with its player
count, and -- for the two real teams -- the team score and average ping under
the matching columns.
=================
*/
static float CG_DrawScoreboard_ListBanner( const float *color, float y, float width,
										   float height, int team, int *lineIndex ) {
	char        bannerShader[64];
	char        teamName[64];
	vec4_t      drawColor;
	const char  *title;
	const char  *value;
	qhandle_t   hShader;
	float       nextY;
	float       textY;
	float       x;
	float       offset;
	int         i;

	if ( cg.scoreboardListFull ) {
		return y;
	}
	if ( *lineIndex < cg.scoreboardScrollPos ) {
		( *lineIndex )++;
		return y;
	}

	nextY = y + 24.0f;
	if ( nextY > SB_LIST_BOTTOM ) {
		cg.scoreboardListFull = qtrue;
		return y;
	}
	( *lineIndex )++;

	drawColor[0] = 1.0f;
	drawColor[1] = 1.0f;
	drawColor[2] = 1.0f;
	drawColor[3] = color[3];

	if ( cg.teamPlayers[team] == 1 ) {
		title = trap_SE_LocalizeMessage(
					va( "CGAME_SB_PLAYER\x15%i", cg.teamPlayers[team] ),
					"scoreboard banner text" );
	} else {
		title = trap_SE_LocalizeMessage(
					va( "CGAME_SB_PLAYERS\x15%i", cg.teamPlayers[team] ),
					"scoreboard banner text" );
	}

	switch ( team ) {
	case TEAM_FREE:
		trap_Cvar_VariableStringBuffer( "g_ScoresBanner_None", bannerShader,
										sizeof( bannerShader ) );
		break;
	case TEAM_AXIS:
		trap_Cvar_VariableStringBuffer( "g_ScoresBanner_Axis", bannerShader,
										sizeof( bannerShader ) );
		trap_Cvar_VariableStringBuffer( "g_TeamName_Axis", teamName, sizeof( teamName ) );
		title = va( "%s (%s)",
					trap_SE_LocalizeMessage( teamName, "scoreboard team name" ),
					title );
		break;
	case TEAM_ALLIES:
		trap_Cvar_VariableStringBuffer( "g_ScoresBanner_Allies", bannerShader,
										sizeof( bannerShader ) );
		trap_Cvar_VariableStringBuffer( "g_TeamName_Allies", teamName, sizeof( teamName ) );
		title = va( "%s (%s)",
					trap_SE_LocalizeMessage( teamName, "scoreboard team name" ),
					title );
		break;
	default:
		trap_Cvar_VariableStringBuffer( "g_ScoresBanner_Spectators", bannerShader,
										sizeof( bannerShader ) );
		title = va( "%s (%s)",
					trap_SE_LocalizeMessage( "CGAME_SPECTATORS",
														   "scoreboard team name" ),
					title );
		break;
	}

	hShader = trap_R_RegisterShaderNoMip( bannerShader, 5 );
	trap_R_SetColor( drawColor );
	CG_DrawPic( SB_LIST_X, y, width, height, hShader );

	CG_DrawScoreboard_GetTeamColor( drawColor, team );

	textY = y + 18.0f;
	trap_R_Text_Paint( 133.0f, textY, 0, SB_BANNER_SCALE, drawColor, title,
					   0.0f, 0, 3 );

	if ( team == TEAM_AXIS || team == TEAM_ALLIES ) {
		x = SB_LIST_X;
		for ( i = 0; i < SB_NUM_COLUMNS; i++ ) {
			if ( cg_scoreboardColumns[i].id == SB_FIELD_SCORE
				 || cg_scoreboardColumns[i].id == SB_FIELD_PING ) {

				if ( cg_scoreboardColumns[i].id == SB_FIELD_SCORE ) {
					value = va( "%i", cg.teamScores[team] );
				} else {
					value = va( "%i", cg.teamPing[team] );
				}

				if ( cg_scoreboardColumns[i].align == SB_ALIGN_RIGHT ) {
					offset = width * cg_scoreboardColumns[i].widthFrac
							 - trap_R_Text_Width( value, 0, SB_BANNER_SCALE, 0 );
				} else {
					offset = 0.0f;
				}

				trap_R_Text_Paint( x + offset, textY, 0, SB_BANNER_SCALE, drawColor,
								   value, 0.0f, 0, 3 );
			}

			x += width * cg_scoreboardColumns[i].widthFrac;
		}
	}

	return nextY;
}

/*
=================
CG_DrawClientScore

One player's row.  Every string is clipped to its column by shortening the
trap_R_Text_Paint limit until it measures short enough.
=================
*/
static float CG_DrawClientScore( const float *color, float y, const score_t *score,
								 float width, int alt, int *lineIndex ) {
	clientInfo_t    *ci;
	vec4_t          drawColor;
	qhandle_t       hShader;
	const char      *s;
	float           nextY;
	float           x;
	float           offset;
	int             limit;
	int             i;

	if ( cg.scoreboardListFull ) {
		return y;
	}
	if ( *lineIndex < cg.scoreboardScrollPos ) {
		( *lineIndex )++;
		return y;
	}

	nextY = y + SB_ROW_HEIGHT;
	if ( nextY > SB_LIST_BOTTOM ) {
		cg.scoreboardListFull = qtrue;
		return y;
	}
	( *lineIndex )++;

	ci = &bg_clientinfo[score->client];
	if ( !ci->infoValid ) {
		return y;
	}

	if ( score->client == cg.clientNum ) {
		hShader = trap_R_RegisterShaderNoMip( "white", 5 );
		drawColor[0] = color[0];
		drawColor[1] = color[1];
		drawColor[2] = color[2];
		drawColor[3] = color[3] * 0.2f;
		trap_R_SetColor( drawColor );
		CG_DrawPic( SB_LIST_X, y, width, SB_ROW_HEIGHT, hShader );
	} else if ( alt ) {
		hShader = trap_R_RegisterShaderNoMip( "black", 5 );
		drawColor[0] = color[0];
		drawColor[1] = color[1];
		drawColor[2] = color[2];
		drawColor[3] = color[3] * 0.15f;
		trap_R_SetColor( drawColor );
		CG_DrawPic( SB_LIST_X, y, width, SB_ROW_HEIGHT, hShader );
	}

	x = SB_LIST_X;

	for ( i = 0; i < SB_NUM_COLUMNS; i++ ) {
		if ( cg_scoreboardColumns[i].id == SB_FIELD_STATUSICON ) {
			if ( score->statusIcon ) {
				if ( cg_scoreboardColumns[i].align == SB_ALIGN_RIGHT ) {
					offset = width * cg_scoreboardColumns[i].widthFrac - SB_STATUSICON_SIZE;
				} else {
					offset = 0.0f;
				}

				drawColor[0] = 1.0f;
				drawColor[1] = 1.0f;
				drawColor[2] = 1.0f;
				drawColor[3] = color[3];
				trap_R_SetColor( drawColor );
				CG_DrawPic( x + offset, y, SB_STATUSICON_SIZE, SB_STATUSICON_SIZE,
							score->statusIcon );
			}
			x += width * cg_scoreboardColumns[i].widthFrac;
			continue;
		}

		s = "";
		switch ( cg_scoreboardColumns[i].id ) {
		case SB_FIELD_NAME:
			s = ci->name;
			break;
		case SB_FIELD_SCORE:
			if ( score->team != TEAM_SPECTATOR ) {
				s = va( "%i", score->score );
			}
			break;
		case SB_FIELD_DEATHS:
			if ( score->team != TEAM_SPECTATOR ) {
				s = va( "%i", score->deaths );
			}
			break;
		case SB_FIELD_PING:
			s = va( "%i", score->ping );
			break;
		default:
			break;
		}

		if ( !s[0] ) {
			x += width * cg_scoreboardColumns[i].widthFrac;
			continue;
		}

		limit = 0;
		if ( width * cg_scoreboardColumns[i].widthFrac
			 < trap_R_Text_Width( s, 0, SB_TEXT_SCALE, 0 ) ) {
			limit = strlen( s );
			do {
				limit--;
			} while ( width * cg_scoreboardColumns[i].widthFrac
					  < trap_R_Text_Width( s, 0, SB_TEXT_SCALE, limit ) );
		}

		if ( cg_scoreboardColumns[i].align == SB_ALIGN_RIGHT ) {
			offset = width * cg_scoreboardColumns[i].widthFrac
					 - trap_R_Text_Width( s, 0, SB_TEXT_SCALE, limit );
		} else {
			offset = 0.0f;
		}

		drawColor[0] = 1.0f;
		drawColor[1] = 1.0f;
		drawColor[2] = 1.0f;
		drawColor[3] = color[3];
		trap_R_Text_Paint( x + offset, y + SB_TEXT_BASELINE, 0, SB_TEXT_SCALE,
						   drawColor, s, 0.0f, limit, 3 );

		x += width * cg_scoreboardColumns[i].widthFrac;
	}

	return nextY;
}

/*
=================
CG_DrawTeamOfClientScore

Every score on one team, alternating the row shade.
=================
*/
static float CG_DrawTeamOfClientScore( const float *color, float y, int team, float width,
									   int *lineIndex ) {
	vec4_t  teamColor;
	int     alt;
	int     i;

	CG_DrawScoreboard_GetTeamColor( teamColor, team );
	teamColor[3] = color[3];

	alt = 0;
	for ( i = 0; i < cg.numScores; i++ ) {
		if ( !bg_clientinfo[cg.scores[i].client].infoValid ) {
			continue;
		}
		if ( cg.scores[i].team != team ) {
			continue;
		}

		y = CG_DrawClientScore( teamColor, y, &cg.scores[i], width, alt, lineIndex );
		alt ^= 1;
	}

	return y;
}

/*
=================
CG_DrawScrollbar

The track down the right edge, the thumb sized to the visible fraction, and
the two arrow / key hint pairs.
=================
*/
static void CG_DrawScrollbar( float y, int numLines, const float *color, int drawnLines ) {
	vec4_t      drawColor;
	qhandle_t   hShader;
	float       trackHeight;
	float       thumbY;
	float       thumbHeight;
	int         visible;

	drawColor[0] = color[0];
	drawColor[1] = color[1];
	drawColor[2] = color[2];

	hShader = trap_R_RegisterShaderNoMip( "black", 5 );
	drawColor[3] = color[3] * 0.5f;
	trap_R_SetColor( drawColor );

	trackHeight = SB_LIST_BOTTOM - y - 1.0f;
	trap_R_DrawStretchPic( cgs.screenXScale * 505.0f, cgs.screenYScale * y,
						   cgs.screenXScale * 8.0f, cgs.screenYScale * trackHeight,
						   0.0f, 0.0f, 1.0f, 1.0f, hShader );

	thumbY = y + 1.0f;
	thumbHeight = trackHeight - 2.0f;

	if ( cg.scoreboardScrollPos && numLines ) {
		thumbY += (float)cg.scoreboardScrollPos / numLines * thumbHeight;
	}

	visible = drawnLines - cg.scoreboardScrollPos;
	if ( visible > 1 && numLines > 1 ) {
		thumbHeight = (float)visible / numLines * thumbHeight;
	}

	hShader = trap_R_RegisterShaderNoMip( "white", 5 );
	drawColor[3] = color[3] * 0.25f;
	trap_R_SetColor( drawColor );
	trap_R_DrawStretchPic( cgs.screenXScale * 506.0f, cgs.screenYScale * thumbY,
						   cgs.screenXScale * 6.0f, cgs.screenYScale * thumbHeight,
						   0.0f, 0.0f, 1.0f, 1.0f, hShader );

	drawColor[3] = color[3];
	trap_R_SetColor( drawColor );

	if ( cg.scoreboardScrollPos > 0 ) {
		CG_DrawPic( 521.0f, y, 16.0f, 16.0f,
					trap_R_RegisterShaderNoMip( "hudScoreboardScroll_UpArrow", 5 ) );
		CG_DrawPic( 521.0f, y + 18.0f, 16.0f, 16.0f,
					trap_R_RegisterShaderNoMip( "hudScoreboardScroll_UpKey", 5 ) );
	}

	if ( drawnLines < numLines ) {
		CG_DrawPic( 521.0f, 415.0f, 16.0f, 16.0f,
					trap_R_RegisterShaderNoMip( "hudScoreboardScroll_DownArrow", 5 ) );
		CG_DrawPic( 521.0f, 397.0f, 16.0f, 16.0f,
					trap_R_RegisterShaderNoMip( "hudScoreboardScroll_DownKey", 5 ) );
	}
}

/*
=================
CG_DrawScoreboard_ScoresList

The objective block, the column headers, then a banner-plus-rows block per
team.  The two real teams go in score order, and a tie keeps whichever order
the last tie produced.
=================
*/
static void CG_DrawScoreboard_ScoresList( float fade ) {
	vec4_t      color;
	float       y;
	float       headerY;
	float       width;
	float       listHeight;
	int         numLines;
	int         lineIndex;
	int         team;
	qboolean    scrollbar;

	cg.scoreboardListFull = qfalse;

	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = fade;

	y = CG_DrawObjectiveInfo( color, 52.0f );

	listHeight = CG_DrawScoreboard_ListHeight( &numLines );
	if ( listHeight > SB_LIST_BOTTOM - y ) {
		width = 374.0f;
		scrollbar = qtrue;
	} else {
		width = 382.0f;
		scrollbar = qfalse;
	}

	y = CG_DrawScoreboard_ListColumnHeaders( y + 4.0f, width, color );
	headerY = y;

	lineIndex = 0;

	if ( cg.teamPlayers[TEAM_AXIS] || cg.teamPlayers[TEAM_ALLIES] ) {
		if ( cg.teamScores[TEAM_ALLIES] < cg.teamScores[TEAM_AXIS] ) {
			team = TEAM_AXIS;
		} else if ( cg.teamScores[TEAM_ALLIES] > cg.teamScores[TEAM_AXIS] ) {
			team = TEAM_ALLIES;
		} else {
			team = cg_scoreboardFirstTeam;
		}
		cg_scoreboardFirstTeam = team;

		y = CG_DrawScoreboard_ListBanner( color, y, width, 32.0f, team, &lineIndex );
		y = CG_DrawTeamOfClientScore( color, y, team, width, &lineIndex );
		y += 4.0f;

		team = ( team == TEAM_AXIS ) ? TEAM_ALLIES : TEAM_AXIS;

		y = CG_DrawScoreboard_ListBanner( color, y, width, 32.0f, team, &lineIndex );
		y = CG_DrawTeamOfClientScore( color, y, team, width, &lineIndex );
		y += 4.0f;
	}

	if ( cg.teamPlayers[TEAM_FREE] ) {
		y = CG_DrawScoreboard_ListBanner( color, y, width, 32.0f, TEAM_FREE, &lineIndex );
		y = CG_DrawTeamOfClientScore( color, y, TEAM_FREE, width, &lineIndex );
		y += 4.0f;
	}

	if ( cg.teamPlayers[TEAM_SPECTATOR] ) {
		y = CG_DrawScoreboard_ListBanner( color, y, width, 32.0f, TEAM_SPECTATOR, &lineIndex );
		CG_DrawTeamOfClientScore( color, y, TEAM_SPECTATOR, width, &lineIndex );
	}

	if ( scrollbar ) {
		CG_DrawScrollbar( headerY, numLines, color, lineIndex );
	}

	if ( cg.scoreboardScrollPos > numLines - 1 ) {
		cg.scoreboardScrollPos = numLines - 1;
	}

	trap_R_SetColor( NULL );
}

/*
=================
CG_DrawScoreboard

Returns qtrue if the scoreboard drew, which is what suppresses the rest of the
2D layer.  The fade / killerName prologue is RTCW's, with FADE_TIME 100.
=================
*/
qboolean CG_DrawScoreboard( void ) {
	float   fade;
	float   *fadeColor;

	if ( cl_paused.integer ) {
		return qfalse;
	}

	if ( cg.showScores ) {
		fade = 1.0f;
	} else {
		fadeColor = CG_FadeColor( cg.scoreFadeTime, 100 );
		if ( !fadeColor ) {
			/* next time the scoreboard comes up, don't print the killer */
			cg.killerName[0] = 0;
			return qfalse;
		}
		fade = *fadeColor;
	}

	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );
	}

	CG_DrawScoreboard_Backdrop( fade );
	CG_DrawScoreboard_ScoresList( fade );

	return qtrue;
}

/*
=================
CG_ScoreboardDisplayed
=================
*/
qboolean CG_ScoreboardDisplayed( void ) {
	return cg.showScores != 0;
}

/*
=================
CG_ScrollScoreboardUp
=================
*/
void CG_ScrollScoreboardUp( void ) {
	if ( cg.scoreboardScrollPos <= 0 ) {
		return;
	}

	cg.scoreboardScrollPos -= cg_scoreboardScrollStep.integer;
	if ( cg.scoreboardScrollPos < 0 ) {
		cg.scoreboardScrollPos = 0;
	}
}

/*
=================
CG_ScrollScoreboardDown
=================
*/
void CG_ScrollScoreboardDown( void ) {
	if ( !cg.scoreboardListFull ) {
		return;
	}

	cg.scoreboardScrollPos += cg_scoreboardScrollStep.integer;
	if ( cg.scoreboardScrollPos > cg.numScores - 1 ) {
		cg.scoreboardScrollPos = cg.numScores - 1;
	}
}

/*
=================
CG_RegisterScoreboardGraphics
=================
*/
void CG_RegisterScoreboardGraphics( void ) {
	char buffer[1024];

	trap_R_RegisterShaderNoMip( "black", 5 );
	trap_R_RegisterShaderNoMip( "white", 5 );
	trap_R_RegisterShaderNoMip( "black", 5 );
	trap_R_RegisterShaderNoMip( "white", 5 );
	trap_R_RegisterShaderNoMip( "white", 5 );
	trap_R_RegisterShaderNoMip( "black", 5 );
	trap_R_RegisterShaderNoMip( "hudScoreboardScroll_UpArrow", 5 );
	trap_R_RegisterShaderNoMip( "hudScoreboardScroll_UpKey", 5 );
	trap_R_RegisterShaderNoMip( "hudScoreboardScroll_DownArrow", 5 );
	trap_R_RegisterShaderNoMip( "hudScoreboardScroll_DownKey", 5 );

	trap_Cvar_VariableStringBuffer( "g_ScoresBanner_Spectators", buffer, sizeof( buffer ) );
	trap_R_RegisterShaderNoMip( buffer, 5 );

	trap_Cvar_VariableStringBuffer( "g_ScoresBanner_Axis", buffer, sizeof( buffer ) );
	trap_R_RegisterShaderNoMip( buffer, 5 );

	trap_Cvar_VariableStringBuffer( "g_ScoresBanner_Allies", buffer, sizeof( buffer ) );
	trap_R_RegisterShaderNoMip( buffer, 5 );

	trap_Cvar_VariableStringBuffer( "g_ScoresBanner_None", buffer, sizeof( buffer ) );
	trap_R_RegisterShaderNoMip( buffer, 5 );
}
