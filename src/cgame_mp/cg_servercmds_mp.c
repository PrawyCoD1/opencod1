/*
 * @fidelity: likely
 *
 * cg_servercmds.c -- reliably sequenced text commands sent by the server.
 * These are processed at snapshot transition time, so there will definitely be
 * a valid snapshot this frame.
 *
 * cgame_mp_x86.dll, CoD 1.1, imagebase 0x30000000; functions in address
 * order, 0x3002B920 .. 0x3002E580.  RTCW-MP's cg_servercmds.c is the
 * ancestor of everything here except the script-menu, reverb, local-sound and
 * fog blocks, which are CoD additions.
 *
 * Three recovered names in this file sit one slot low, because 0x3002D440
 * carries no symbol; the bodies are RTCW's and identify themselves:
 *
 *   0x3002D220  recovered as CG_GetVoiceChat           -- it is CG_HeadModelVoiceChats
 *   0x3002D390  recovered as CG_VoiceChatListForClient -- it is CG_GetVoiceChat
 *   0x3002D440  unnamed                                 -- it is CG_VoiceChatListForClient
 *
 * CG_PrecacheScriptMenu (0x3002BFC0) and CG_RegisterServerShader (0x3002C010)
 * are real functions -- LTCG hands them the index in eax / ecx, inlines them
 * into CG_SetConfigValues' loops and tail-calls them from
 * CG_ConfigStringModified.  0x30019BB0, CG_ConfigStringModified's CS_NORTHYAW
 * arm, sits among cg_drawtools' functions and belongs to this file.
 */

#include "cg_local.h"

#include <stdlib.h>
#include <string.h>

/* Declarations no header carries. */

/* q_shared.h's random(); the binary is rand() * (1/32768) with no mask
   (0x3002D3E3). */
#define random()                ( rand() * ( 1.0f / 32768.0f ) )

/* cg_localents.c, cg_marks.c, cg_newDraw.c, cg_weapons.c, cg_shellshock.c and
   cg_draw.c. */
void        CG_InitLocalEntities( void );
void        CG_InitMarkPolys( void );
const char *CG_GetTranslatedLocationString( int location );
void        CG_RegisterItems( void );
void        CG_FinishWeaponChange( int oldWeapon, int newWeapon );
qboolean    CG_LoadShellShockCvars( const char *name );
void        CG_SetShellShockParmsFromCvars( shellshockParms_t *parms );
/* cg_main.c.  The binary hands `name` in eax; returns int. */
int         CG_PlaySoundAliasByName( const char *name, int entnum, const vec3_t origin );
/* game/bg_pmove.c, compiled into cgame as well. */
void        BG_SetupWeaponInfo( void );

/*
 * The configstring layout.  Every base below is read by a branch of
 * CG_ConfigStringModified (0x3002C6B0) or the precache loop of
 * CG_SetConfigValues (0x3002C060); the counts are the ranges those tests
 * span.  game_mp/g_utils_mp.c and g_scr_main_mp.c carry the same numbers on
 * the server side.
 */
#define MAX_CONFIGSTRINGS           2048    /* the bad-index test, 0x3002C6E8 */

#define CS_SERVERINFO               0
#define CS_MUSIC                    3       /* CG_StartAmbient */
#define CS_TEAMSCORE_AXIS           5
#define CS_TEAMSCORE_ALLIES         6
#define CS_ITEMS                    8
#define CS_NORTHYAW                 11
#define CS_FOGVARS                  12
#define CS_LEVEL_START_TIME         13
#define CS_VOTE_TIME                15
#define CS_VOTE_STRING              16
#define CS_VOTE_YES                 17
#define CS_VOTE_NO                  18
#define CS_STATUSICONS              21
#define CS_STATUSICONS_COUNT        8
#define CS_HEADICONS                29
#define CS_HEADICONS_COUNT          15
/* CG_ParseWolfinfo walks 140..203 for the names and 204..267 for the values. */
#define CS_SERVERCVARS_NAME         140
#define CS_SERVERCVARS_VALUE        204
#define CS_SERVERCVARS_COUNT        64
#define CS_MODELS                   268
#define CS_MODELS_COUNT             256
#define CS_SOUNDALIAS               524
#define CS_SOUNDALIAS_COUNT         256
#define CS_EFFECTS                  780
#define CS_EFFECTS_COUNT            64
#define CS_SHELLSHOCKS              1100
#define CS_SHELLSHOCKS_COUNT        16
#define CS_MENUS                    1180
#define CS_MENUS_COUNT              32
#define CS_SHADERS                  1500
#define CS_SHADERS_COUNT            128

/* The second argument every Register trap in this unit passes, except
   CG_ParseVoiceChats' three, which pass 2 (0x3002D05C, 0x3002D0D6, 0x3002D110)
   like cg_main's headiconVoiceChat registration; the engine forwards it to the
   material loader unchanged. */
#define REGISTER_SHADER_ARG2        5

/* RTCW's fogType_t / FOG_CMD_SWITCHFOG, unchanged in the two values CG_ParseFog
   uses.  The `3` of the fog-off path is RTCW's FOG_HUD slot; no CoD 1.1 access
   confirms the spelling, so both stay numeric. */
#define FOG_MAP                     4
#define FOG_CMD_SWITCHFOG           8

/* g_local.h's team_t; cgame has no copy of the enum. */
#define TEAM_AXIS                   1

/* pm_flags bit gating a client that is actually playing.  bg_local.h stops at
   PMF_FOLLOW 0x10000 and this bit has no recovered name. */
#define PMF_INGAME                  0x00040000


/*
=============================================================================

		VOICE CHATS

	voiceChatLists[0] is 0x302F7D80 and voiceChatLists[1] is 0x30340EC8, so
	voiceChatList_t is 299336 == 72 + 64 * 4676.  Inside one voiceChat_t the
	members are CG_ParseVoiceChats' stores: id at +0
	(0x3002CDD4), numSounds at +64 (0x3002CE44), sounds at +68 (0x3002CF45),
	chats at +324 (0x3002CFBD) and sprite at +4420 (0x3002D0F0).

=============================================================================
*/

/* voiceChat_t / voiceChatList_t and the four sizes they need are in
   cg_local.h: cg_main_mp.c's CG_RegisterSounds reaches voiceChatLists too. */
#define MAX_VOICEFILESIZE       16384
#define MAX_VOICECHATBUFFER     32
#define MAX_SAY_TEXT            150

/* CG_ParseVoiceChats' gender tokens, 0x3002CD2C..0x3002CD68. */
#define GENDER_MALE             0
#define GENDER_FEMALE           1
#define GENDER_NEUTER           2

typedef struct bufferedVoiceChat_s
{
	int clientNum;
	const char  *snd;
	qhandle_t sprite;
	int voiceOnly;
	char cmd[MAX_SAY_TEXT];
	char message[MAX_SAY_TEXT];
	vec3_t origin;
} bufferedVoiceChat_t;
CG_ASSERT_SIZE( bufferedVoiceChat_t, 328 );

voiceChatList_t voiceChatLists[MAX_VOICEFILES];                 /* 0x302F7D80 */
bufferedVoiceChat_t voiceChatBuffer[MAX_VOICECHATBUFFER];       /* 0x305407C0 */

/* CG_VoiceChatLocal's `mode`; RTCW's values, unchanged (0x3002D882). */
#define SAY_ALL                 0
#define SAY_TEAM                1
#define SAY_TELL                2

/* CG_VoiceChat's cg_noTaunt filter, 0x3002DA4A. */
#define VOICECHAT_KILLINSULT    "kill_insult"
#define VOICECHAT_TAUNT         "taunt"
#define VOICECHAT_DEATHINSULT   "death_insult"
#define VOICECHAT_KILLGAUNTLET  "kill_gauntlet"
#define VOICECHAT_PRAISE        "praise"


/*
=================
CG_ParseScores

CoD packs five fields per client where RTCW packs eight, and the fifth is a
one-based status-icon index rather than RTCW's scoreFlags.
=================
*/
static void CG_ParseScores( void ) {
	int i;

	cg.numScores = atoi( CG_Argv( 1 ) );
	if ( cg.numScores > MAX_CLIENTS ) {
		cg.numScores = MAX_CLIENTS;
	}

	memset( cg.teamScores, 0, sizeof( cg.teamScores ) );
	cg.teamScores[1] = atoi( CG_Argv( 2 ) );
	cg.teamScores[2] = atoi( CG_Argv( 3 ) );

	memset( cg.scores, 0, sizeof( cg.scores ) );
	memset( cg.teamPing, 0, sizeof( cg.teamPing ) );
	memset( cg.teamPlayers, 0, sizeof( cg.teamPlayers ) );

	for ( i = 0 ; i < cg.numScores ; i++ ) {
		cg.scores[i].client = atoi( CG_Argv( i * 5 + 4 ) );
		cg.scores[i].score = atoi( CG_Argv( i * 5 + 5 ) );
		cg.scores[i].ping = atoi( CG_Argv( i * 5 + 6 ) );
		cg.scores[i].deaths = atoi( CG_Argv( i * 5 + 7 ) );
		cg.scores[i].statusIcon = atoi( CG_Argv( i * 5 + 8 ) );

		if ( cg.scores[i].statusIcon > 0 && cg.scores[i].statusIcon <= CS_STATUSICONS_COUNT ) {
			cg.scores[i].statusIcon = trap_R_RegisterShaderNoMip(
				CG_ConfigString( CS_STATUSICONS + cg.scores[i].statusIcon - 1 ), REGISTER_SHADER_ARG2 );
		}

		if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS ) {
			cg.scores[i].client = 0;
		}
		(&bg_clientinfo[ cg.scores[i].client ])->score = cg.scores[i].score;

		if ( bg_clientinfo[ cg.scores[i].client ].infoValid ) {
			cg.scores[i].team = bg_clientinfo[ cg.scores[i].client ].team;
		} else {
			cg.scores[i].team = 0;
		}

		cg.teamPlayers[ cg.scores[i].team ]++;
		cg.teamPing[ cg.scores[i].team ] += cg.scores[i].ping;
	}

	for ( i = 0 ; i < 4 ; i++ ) {
		if ( cg.teamPlayers[i] > 0 && cg.teamPing[i] > 0 ) {
			cg.teamPing[i] /= cg.teamPlayers[i];
		} else {
			cg.teamPing[i] = 0;
		}
	}
}

/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received, and whenever
the server updates any serverinfo flagged cvars.

Both copies are a bare strncpy, not Q_strncpyz: neither writes the terminator
the vote-string path in CG_ConfigStringModified does (0x3002BBD6, 0x3002BBF2).
================
*/
void CG_ParseServerinfo( void ) {
	const char  *info;

	info = CG_ConfigString( CS_SERVERINFO );

	strncpy( cgs.sv_hostname, Info_ValueForKey( info, "sv_hostname" ), sizeof( cgs.sv_hostname ) );
	strncpy( cgs.gametype, Info_ValueForKey( info, "g_gametype" ), sizeof( cgs.gametype ) );
	if ( !cgs.localServer ) {
		trap_Cvar_Set( "g_gametype", cgs.gametype );
	}

	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/mp/%s.bsp",
				 Info_ValueForKey( info, "mapname" ) );
}

/*
==================
CG_ParseWolfinfo

Mirrors the server's cvar table into the client's: configstrings 140..203 carry
the names and 204..267 the matching values, and the first empty name ends the
table.  A local server already has the cvars.
==================
*/
void CG_ParseWolfinfo( void ) {
	int i;
	const char  *name;

	if ( cgs.localServer ) {
		return;
	}

	for ( i = CS_SERVERCVARS_VALUE ; i < CS_SERVERCVARS_VALUE + CS_SERVERCVARS_COUNT ; i++ ) {
		name = CG_ConfigString( i - ( CS_SERVERCVARS_VALUE - CS_SERVERCVARS_NAME ) );
		if ( !name[0] ) {
			break;
		}
		trap_Cvar_Set( name, CG_ConfigString( i ) );
	}
}

/*
==============
CG_ParseFog

The fog configstring is the game module's "%g %g %g %g %g %g %.0f" -- near
distance, far distance, density, red, green, blue and the transition time in
milliseconds (g_cmds_mp.c G_setfog).  A string carrying only the first value
means "fade the fog back out over that many milliseconds".
==============
*/
static void CG_ParseFog( void ) {
	char        *p;
	const char  *token;
	float ne, fa, density, r, g, b;
	int time;

	p = (char *)CG_ConfigString( CS_FOGVARS );

	ne = atof( Com_Parse( &p ) );

	token = Com_Parse( &p );
	if ( !token || !token[0] ) {
		trap_R_SetFog( FOG_CMD_SWITCHFOG, 3, (int)ne, 0, 0, 0, 0 );
		return;
	}
	fa = atof( token );

	density = atof( Com_Parse( &p ) );
	r = atof( Com_Parse( &p ) );
	g = atof( Com_Parse( &p ) );
	b = atof( Com_Parse( &p ) );
	time = atoi( Com_Parse( &p ) );

	trap_R_SetFog( FOG_MAP, (int)ne, (int)fa, r, g, b, density );
	trap_R_SetFog( FOG_CMD_SWITCHFOG, FOG_MAP, time, 0, 0, 0, 0 );
}

/*
================
CG_PrecacheScriptMenu

0x3002BFC0; the index arrives in eax.
================
*/
static void CG_PrecacheScriptMenu( int index ) {
	const char  *str;

	str = CG_ConfigString( index );
	if ( str[0] && !trap_UI_LoadMenu( str ) ) {
		CG_Error( "Could not load script menu file '%s'\n", str );
	}
}

/*
================
CG_RegisterServerShader

0x3002C010; the index arrives in ecx.
================
*/
static void CG_RegisterServerShader( int index ) {
	const char  *str;

	str = CG_ConfigString( index );
	if ( str[0] ) {
		trap_R_RegisterShaderNoMip( str, REGISTER_SHADER_ARG2 );
	}
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings.
================
*/
void CG_SetConfigValues( void ) {
	int i;

	cgs.scores1 = atoi( CG_ConfigString( CS_TEAMSCORE_AXIS ) );
	cgs.scores2 = atoi( CG_ConfigString( CS_TEAMSCORE_ALLIES ) );
	cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
	CG_ParseFog();

	for ( i = CS_MENUS ; i < CS_MENUS + CS_MENUS_COUNT ; i++ ) {
		CG_PrecacheScriptMenu( i );
	}

	for ( i = CS_STATUSICONS ; i < CS_STATUSICONS + CS_STATUSICONS_COUNT ; i++ ) {
		trap_R_RegisterShaderNoMip( CG_ConfigString( i ), REGISTER_SHADER_ARG2 );
	}

	for ( i = CS_HEADICONS ; i < CS_HEADICONS + CS_HEADICONS_COUNT ; i++ ) {
		trap_R_RegisterShader( CG_ConfigString( i ), REGISTER_SHADER_ARG2 );
	}

	/* Slot 0 means "no shader", so the precache starts one past the base. */
	for ( i = CS_SHADERS + 1 ; i < CS_SHADERS + CS_SHADERS_COUNT ; i++ ) {
		CG_RegisterServerShader( i );
	}
}

/*
================
CG_ConfigStringModified

The CS_MENUS and CS_SHADERS arms tail-call CG_PrecacheScriptMenu (0x3002C8B4)
and CG_RegisterServerShader (0x3002C90C); the CS_NORTHYAW arm is outlined at
0x30019BB0, a thousand functions away among cg_drawtools', and re-derives the
string rather than reuse `str`.
================
*/
static void CG_ConfigStringModified( void ) {
	const char  *str;
	int num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap_GetGameState( &cg_gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_ITEMS ) {
		CG_RegisterItems();
	} else if ( num == CS_MUSIC ) {
		CG_StartAmbient();
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num >= CS_SERVERCVARS_NAME
				&& num < CS_SERVERCVARS_VALUE + CS_SERVERCVARS_COUNT ) {
		CG_ParseWolfinfo();
	} else if ( num == CS_TEAMSCORE_AXIS ) {
		cgs.scores1 = atoi( str );
	} else if ( num == CS_TEAMSCORE_ALLIES ) {
		cgs.scores2 = atoi( str );
	} else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num == CS_VOTE_TIME ) {
		cgs.voteTime = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_YES ) {
		cgs.voteYes = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_NO ) {
		cgs.voteNo = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_STRING ) {
		Q_strncpyz( cgs.voteString, trap_SE_LocalizeMessage( str, "vote string" ),
					sizeof( cgs.voteString ) );
	} else if ( num == CS_FOGVARS ) {
		CG_ParseFog();
	} else if ( num >= CS_MODELS && num < CS_MODELS + CS_MODELS_COUNT ) {
		cgs.gameModels[ num - CS_MODELS ] = trap_R_RegisterModel( str, 7 );
	} else if ( num >= CS_EFFECTS && num < CS_EFFECTS + CS_EFFECTS_COUNT ) {
		cgs.gameEffects[ num - CS_EFFECTS ] = trap_syscall_0xDE( str );
	} else if ( num >= CS_SHELLSHOCKS && num < CS_SHELLSHOCKS + CS_SHELLSHOCKS_COUNT ) {
		if ( str[0] && CG_LoadShellShockCvars( str ) ) {
			CG_SetShellShockParmsFromCvars( &cgs.shellshockParms[ num - CS_SHELLSHOCKS ] );
		}
	} else if ( num >= CS_MENUS && num < CS_MENUS + CS_MENUS_COUNT ) {
		CG_PrecacheScriptMenu( num );
	} else if ( num >= CS_STATUSICONS && num < CS_STATUSICONS + CS_STATUSICONS_COUNT ) {
		trap_R_RegisterShaderNoMip( CG_ConfigString( num ), REGISTER_SHADER_ARG2 );
	} else if ( num >= CS_HEADICONS && num < CS_HEADICONS + CS_HEADICONS_COUNT ) {
		trap_R_RegisterShader( CG_ConfigString( num ), REGISTER_SHADER_ARG2 );
	} else if ( num >= CS_SHADERS && num < CS_SHADERS + CS_SHADERS_COUNT ) {
		CG_RegisterServerShader( num );
	} else if ( num == CS_NORTHYAW ) {
		cg.compassNorthYaw = atof( CG_ConfigString( CS_NORTHYAW ) );
	}
}

/*
=======================
CG_AddToTeamChat
=======================
*/
void CG_AddToTeamChat( const char *str ) {
	int len;
	char    *p, *ls;
	int lastcolor;
	int chatHeight;

	if ( cg_chatHeight.integer < TEAMCHAT_HEIGHT ) {
		chatHeight = cg_chatHeight.integer;
	} else {
		chatHeight = TEAMCHAT_HEIGHT;
	}

	if ( chatHeight <= 0 || cg_chatTime.integer <= 0 ) {
		// team chat disabled, dump into normal chat
		cgs.teamChatPos = cgs.teamLastChatPos = 0;
		return;
	}

	len = 0;

	p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while ( *str ) {
		if ( len > TEAMCHAT_WIDTH - 1 ) {
			if ( ls ) {
				str -= ( p - ls );
				str++;
				p -= ( p - ls );
			}
			*p = 0;

			cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;

			cgs.teamChatPos++;
			p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
			*p = 0;
			*p++ = '^';
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( str && *str == '^' && str[1] && str[1] != '^' && str[1] >= '0' && str[1] <= '7' ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if ( *str == ' ' ) {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0;

	cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
	cgs.teamChatPos++;

	if ( cgs.teamChatPos - cgs.teamLastChatPos > chatHeight ) {
		cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
	}
}

/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot is completely new and
should not be interpolated to.
===============
*/
static void CG_MapRestart( void ) {
	if ( cg_showmiss.integer ) {
		CG_Printf( "CG_MapRestart\n" );
	}

	if ( cgs.localServer ) {
		trap_syscall_0xC8( 1 );
		BG_SetupWeaponInfo();
	}

	cg.centerPrintTime = 0;
	cg.itemPickupTime = 0;
	cg.cursorHintFade = 0;

	cgs.unknown_0x0A430 = -1;
	cgs.unknown_0x0A434 = 0;

	CG_InitLocalEntities();
	CG_InitMarkPolys();
	trap_syscall_0xE9();

	// make sure the "3 frags left" warnings play again
	cgs.voteTime = 0;

	cg.mapRestart = qtrue;

	trap_syscall_0xD3( 0 );
	CG_StartAmbient();

	cg.unknown_0x2AF58 = 0;
	cg.v_dmg_time = 0;
	memset( cg.damageIndicators, 0, sizeof( cg.damageIndicators ) );
	memset( cg.unknown_0x2A7EC, 0, sizeof( cg.unknown_0x2A7EC ) );
	cg.unknown_0x2A804 = 0;

	trap_Cvar_Set( "cg_thirdPerson", "0" );
	trap_Cvar_Set( "cl_stance", "0" );
	trap_Cvar_Set( "cl_run", "1" );

	trap_Cvar_Set( "ui_scriptMenuAllowResponse", "0" );
	CG_CloseScriptMenu();
	CG_CloseScriptMenu();
	trap_Cvar_Set( "ui_scriptMenuAllowResponse", "1" );

	trap_UI_CloseAllMenus();

	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}

	trap_Cvar_Set( "cg_objectiveText", "" );

	trap_syscall_0xF1();
}

/*
=================
CG_ParseVoiceChats

RTCW's parser with trap_S_RegisterSound replaced by the sound-alias lookup,
COM_RestoreParseSession by Com_UngetToken and the "sprites/voiceChat" default
by the headiconVoiceChat material CG_PlayVoiceChat compares against.  Both
callers discard the qboolean, but 0x3002D1A0 still returns it.
=================
*/
/* NOT static: cg_main_mp.c's CG_RegisterSounds carries the same two calls
   (0x30020A19, 0x30020A2A) that CG_LoadVoiceChats does. */
int CG_ParseVoiceChats( const char *filename, voiceChatList_t *voiceChatList,
						int maxVoiceChats ) {
	int len, i;
	int current = 0;
	fileHandle_t f;
	char buf[MAX_VOICEFILESIZE];
	char    **p, *ptr;
	char    *token;
	voiceChat_t *voiceChats;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( "^1voice chat file not found: %s\n", filename ) );
		return qfalse;
	}
	if ( len >= MAX_VOICEFILESIZE ) {
		trap_Print( va( "^1voice chat file too large: %s is %i, max allowed is %i",
						filename, len, MAX_VOICEFILESIZE ) );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ptr = buf;
	p = &ptr;

	Com_sprintf( voiceChatList->name, sizeof( voiceChatList->name ), "%s", filename );
	voiceChats = voiceChatList->voiceChats;
	for ( i = 0; i < maxVoiceChats; i++ ) {
		voiceChats[i].id[0] = 0;
	}

	token = Com_ParseExt( p, qtrue );
	if ( !token || token[0] == 0 ) {
		return qtrue;
	}
	if ( !Q_stricmp( token, "female" ) ) {
		voiceChatList->gender = GENDER_FEMALE;
	} else if ( !Q_stricmp( token, "male" ) )        {
		voiceChatList->gender = GENDER_MALE;
	} else if ( !Q_stricmp( token, "neuter" ) )        {
		voiceChatList->gender = GENDER_NEUTER;
	} else {
		trap_Print( va( "^1expected gender not found in voice chat file: %s\n", filename ) );
		return qfalse;
	}

	voiceChatList->numVoiceChats = 0;
	while ( 1 ) {
		token = Com_ParseExt( p, qtrue );
		if ( !token || token[0] == 0 ) {
			return qtrue;
		}
		Com_sprintf( voiceChats[voiceChatList->numVoiceChats].id,
					 sizeof( voiceChats[voiceChatList->numVoiceChats].id ), "%s", token );

		token = Com_ParseExt( p, qtrue );
		if ( !token || Q_stricmp( token, "{" ) ) {
			trap_Print( va( "^1expected { found %s in voice chat file: %s\n", token, filename ) );
			return qfalse;
		}

		voiceChats[voiceChatList->numVoiceChats].numSounds = 0;
		current = voiceChats[voiceChatList->numVoiceChats].numSounds;

		while ( 1 ) {
			token = Com_ParseExt( p, qtrue );
			if ( !token || token[0] == 0 ) {
				return qtrue;
			}
			if ( !Q_stricmp( token, "}" ) ) {
				break;
			}
			voiceChats[voiceChatList->numVoiceChats].sounds[current] =
				(const char *)trap_Com_SoundAliasString( token );

			token = Com_ParseExt( p, qtrue );
			if ( !token || token[0] == 0 ) {
				return qtrue;
			}
			Com_sprintf( voiceChats[voiceChatList->numVoiceChats].chats[current], MAX_CHATSIZE,
						 "%s", token );

			// specify the sprite shader to show above the player's head
			token = Com_ParseExt( p, qfalse );
			if ( !token || Q_stricmp( token, "}" ) == 0 || token[0] == 0 ) {
				voiceChats[voiceChatList->numVoiceChats].sprite[current] =
					trap_R_RegisterShader( "headiconVoiceChat", 2 );
				Com_UngetToken();
			} else {
				voiceChats[voiceChatList->numVoiceChats].sprite[current] =
					trap_R_RegisterShader( token, 2 );
				if ( voiceChats[voiceChatList->numVoiceChats].sprite[current] == 0 ) {
					voiceChats[voiceChatList->numVoiceChats].sprite[current] =
						trap_R_RegisterShader( "headiconVoiceChat", 2 );
				}
			}

			voiceChats[voiceChatList->numVoiceChats].numSounds++;
			current = voiceChats[voiceChatList->numVoiceChats].numSounds;

			if ( voiceChats[voiceChatList->numVoiceChats].numSounds >= MAX_VOICESOUNDS ) {
				break;
			}
		}

		voiceChatList->numVoiceChats++;
		if ( voiceChatList->numVoiceChats >= maxVoiceChats ) {
			return qtrue;
		}
	}
	return qtrue;
}

/*
=================
CG_LoadVoiceChats
=================
*/
void CG_LoadVoiceChats( void ) {
	int size;

	size = trap_MemoryRemaining();
	CG_ParseVoiceChats( "mp/axis_chat.voice", &voiceChatLists[0], MAX_VOICECHATS );
	CG_ParseVoiceChats( "mp/allies_chat.voice", &voiceChatLists[1], MAX_VOICECHATS );
	CG_Printf( "voice chat memory size = %d\n", size - trap_MemoryRemaining() );
}

/*
=================
CG_HeadModelVoiceChats

Uncalled in 1.1 -- CG_VoiceChatListForClient picks by team instead -- but the
code is still linked in.
=================
*/
int CG_HeadModelVoiceChats( char *filename ) {
	int len, i;
	fileHandle_t f;
	char buf[MAX_VOICEFILESIZE];
	char    **p, *ptr;
	char    *token;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( "voice chat file not found: %s\n", filename ) );
		return -1;
	}
	if ( len >= MAX_VOICEFILESIZE ) {
		trap_Print( va( "^1voice chat file too large: %s is %i, max allowed is %i",
						filename, len, MAX_VOICEFILESIZE ) );
		trap_FS_FCloseFile( f );
		return -1;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	ptr = buf;
	p = &ptr;

	token = Com_ParseExt( p, qtrue );
	if ( !token || token[0] == 0 ) {
		return -1;
	}

	for ( i = 0; i < MAX_VOICEFILES; i++ ) {
		if ( !Q_stricmp( token, voiceChatLists[i].name ) ) {
			return i;
		}
	}

	return -1;
}

/*
=================
CG_GetVoiceChat
=================
*/
static int CG_GetVoiceChat( voiceChatList_t *voiceChatList, const char *id, const char **snd,
							qhandle_t *sprite, char **chat ) {
	int i, rnd;

	for ( i = 0; i < voiceChatList->numVoiceChats; i++ ) {
		if ( !Q_stricmp( id, voiceChatList->voiceChats[i].id ) ) {
			rnd = (int)( random() * voiceChatList->voiceChats[i].numSounds );
			*snd = voiceChatList->voiceChats[i].sounds[rnd];
			*sprite = voiceChatList->voiceChats[i].sprite[rnd];
			*chat = voiceChatList->voiceChats[i].chats[rnd];
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
CG_VoiceChatListForClient

CoD dropped RTCW's head-model search: the list is the client's team.
=================
*/
static voiceChatList_t *CG_VoiceChatListForClient( int clientNum ) {
	if ( !bg_clientinfo[ clientNum ].infoValid ) {
		return &voiceChatLists[0];
	}
	if ( bg_clientinfo[ clientNum ].team == TEAM_AXIS ) {
		return &voiceChatLists[0];
	}
	return &voiceChatLists[1];
}

/*
=================
CG_PlayVoiceChat
=================
*/
static void CG_PlayVoiceChat( bufferedVoiceChat_t *vchat ) {
	if ( !cg_noVoiceChats.integer ) {
		CG_PlaySoundAliasByName( vchat->snd, cg.snap->ps.clientNum, cg.snap->ps.origin );

		// show the icon above the head
		if ( vchat->clientNum == cg.snap->ps.clientNum ) {
			cg.predictedPlayerEntity.voiceChatSprite = vchat->sprite;
			if ( vchat->sprite == cgs.media.headiconVoiceChat ) {
				cg.predictedPlayerEntity.voiceChatSpriteTime = cg.time + cg_voiceSpriteTime.integer;
			} else {
				cg.predictedPlayerEntity.voiceChatSpriteTime = cg.time + cg_voiceSpriteTime.integer * 2;
			}
		} else {
			cg_entities[ vchat->clientNum ].voiceChatSprite = vchat->sprite;
			VectorCopy( vchat->origin, cg_entities[ vchat->clientNum ].lerpOrigin );
			if ( vchat->sprite == cgs.media.headiconVoiceChat ) {
				cg_entities[ vchat->clientNum ].voiceChatSpriteTime = cg.time + cg_voiceSpriteTime.integer;
			} else {
				cg_entities[ vchat->clientNum ].voiceChatSpriteTime = cg.time + cg_voiceSpriteTime.integer * 2;
			}
		}
	}

	if ( !vchat->voiceOnly && !cg_noVoiceText.integer ) {
		CG_AddToTeamChat( vchat->message );
		CG_Printf( va( ": %s\n", vchat->message ) );
	}
	voiceChatBuffer[cg.voiceChatBufferOut].snd = 0;
}

/*
=====================
CG_PlayBufferedVoiceChats
=====================
*/
void CG_PlayBufferedVoiceChats( void ) {
	if ( cg.voiceChatTime < cg.time ) {
		if ( cg.voiceChatBufferOut != cg.voiceChatBufferIn
			 && voiceChatBuffer[cg.voiceChatBufferOut].snd ) {
			CG_PlayVoiceChat( &voiceChatBuffer[cg.voiceChatBufferOut] );

			cg.voiceChatBufferOut = ( cg.voiceChatBufferOut + 1 ) % MAX_VOICECHATBUFFER;
			cg.voiceChatTime = cg.time + 1000;
		}
	}
}

/*
=====================
CG_AddBufferedVoiceChat

The new system doesn't buffer but overwrites vchats.
=====================
*/
static void CG_AddBufferedVoiceChat( bufferedVoiceChat_t *vchat ) {
	memcpy( &voiceChatBuffer[0], vchat, sizeof( bufferedVoiceChat_t ) );
	cg.voiceChatBufferIn = 0;
	CG_PlayVoiceChat( &voiceChatBuffer[0] );
}

/*
=================
CG_GetTranslatedVoiceChatString

cg_main.c's CG_SafeTranslateString_Internal with the domain fixed to the voice
chat table.
=================
*/
static const char *CG_GetTranslatedVoiceChatString( const char *reference ) {
	static char translated[MAX_STRING_CHARS];
	const char  *localized;

	localized = trap_SE_TranslateReference( reference );
	if ( localized ) {
		return localized;
	}

	if ( cl_languagewarnings.integer ) {
		if ( cl_languagewarningsaserrors.integer ) {
			Com_Error( 7, "Could not translate voice chat string \"%s\"", reference );
		} else {
			Com_Printf( "^3WARNING: Could not translate voice chat string \"%s\"\n", reference );
		}
		strcpy( translated, "^1UNLOCALIZED(^7" );
		strcat( translated, reference );
		strcat( translated, "^1)^7" );
		return translated;
	}

	strcpy( translated, reference );
	return translated;
}

/*
=================
CG_VoiceChatLocal
=================
*/
static void CG_VoiceChatLocal( int mode, qboolean voiceOnly, int clientNum, int color,
							   const char *cmd, vec3_t origin ) {
	char        *chat;
	voiceChatList_t *voiceChatList;
	clientInfo_t *ci;
	const char  *snd;
	qhandle_t sprite;
	bufferedVoiceChat_t vchat;
	const char  *loc;

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &bg_clientinfo[ clientNum ];

	if ( !ci->infoValid ) {
		return;
	}

	voiceChatList = CG_VoiceChatListForClient( clientNum );

	if ( CG_GetVoiceChat( voiceChatList, cmd, &snd, &sprite, &chat ) ) {
		if ( mode == SAY_TEAM || !cg_teamChatsOnly.integer ) {
			vchat.clientNum = clientNum;
			vchat.snd = snd;
			vchat.sprite = sprite;
			vchat.voiceOnly = voiceOnly;
			VectorCopy( origin, vchat.origin );
			Q_strncpyz( vchat.cmd, cmd, sizeof( vchat.cmd ) );

			loc = CG_GetTranslatedLocationString( (ci)->location );

			if ( mode == SAY_TELL ) {
				Com_sprintf( vchat.message, sizeof( vchat.message ), "[%s]%s[%s]: %c%c%s",
							 ci->name, "^3", loc, '^', color,
							 CG_GetTranslatedVoiceChatString( chat ) );
			} else if ( mode == SAY_TEAM )   {
				Com_sprintf( vchat.message, sizeof( vchat.message ), "(%s)%s(%s): %c%c%s",
							 ci->name, "^3", loc, '^', color,
							 CG_GetTranslatedVoiceChatString( chat ) );
			} else {
				Com_sprintf( vchat.message, sizeof( vchat.message ), "%s %s(%s): %c%c%s",
							 ci->name, "^3", loc, '^', color,
							 CG_GetTranslatedVoiceChatString( chat ) );
			}
			CG_AddBufferedVoiceChat( &vchat );
		}
	}
}

/*
=================
CG_VoiceChat
=================
*/
static void CG_VoiceChat( int mode ) {
	const char  *cmd;
	int clientNum, color;
	qboolean voiceOnly;
	vec3_t origin;

	voiceOnly = atoi( CG_Argv( 1 ) );
	clientNum = atoi( CG_Argv( 2 ) );
	color = atoi( CG_Argv( 3 ) );

	origin[0] = atoi( CG_Argv( 5 ) );
	origin[1] = atoi( CG_Argv( 6 ) );
	origin[2] = atoi( CG_Argv( 7 ) );

	cmd = CG_Argv( 4 );

	if ( cg_noTaunt.integer != 0 ) {
		if ( !strcmp( cmd, VOICECHAT_KILLINSULT ) || !strcmp( cmd, VOICECHAT_TAUNT ) ||	\
			 !strcmp( cmd, VOICECHAT_DEATHINSULT ) || !strcmp( cmd, VOICECHAT_KILLGAUNTLET ) ||	\
			 !strcmp( cmd, VOICECHAT_PRAISE ) ) {
			return;
		}
	}

	CG_VoiceChatLocal( mode, voiceOnly, clientNum, color, cmd, origin );
}

/*
=================
CG_OpenScriptMenu

"cmd mr <index> <response>" is the way back to the server, so every refusal
still has to answer it.
=================
*/
static void CG_OpenScriptMenu( void ) {
	int index;
	qboolean noMouse;
	const char  *menu;
	char waiting[MAX_STRING_CHARS];

	noMouse = qfalse;

	index = atoi( CG_Argv( 1 ) );
	if ( index < 0 || index >= CS_MENUS_COUNT ) {
		Com_Printf( "Server tried to open a bad script menu index: %i\n", index );
		trap_SendConsoleCommand( va( "cmd mr %i bad\n", index ) );
		return;
	}

	menu = CG_ConfigString( CS_MENUS + index );
	if ( !menu[0] ) {
		Com_Printf( "Server tried to open a non-loaded script menu index: %i\n", index );
		trap_SendConsoleCommand( va( "cmd mr %i bad\n", index ) );
		return;
	}

	if ( trap_Argc() > 2 && CG_Argv( 2 ) && CG_Argv( 2 )[0] ) {
		noMouse = qtrue;
	}

	trap_Cvar_Set( "ui_newScriptMenu", menu );
	trap_Cvar_Set( "ui_newScriptMenuIndex", va( "%i", index ) );

	if ( trap_UI_Popup( noMouse ? "UIMENU_SCRIPT_POPUP_NO_MOUSE" : "UIMENU_SCRIPT_POPUP" ) ) {
		return;
	}

	// the ui was busy -- park the menu and try again next frame
	trap_Cvar_Set( "ui_newScriptMenu", "" );
	trap_Cvar_Set( "ui_newScriptMenuIndex", "-1" );

	trap_Cvar_VariableStringBuffer( "ui_waitingScriptMenu", waiting, sizeof( waiting ) );
	if ( waiting[0] ) {
		if ( !Q_stricmp( waiting, menu ) ) {
			return;
		}
		trap_Cvar_VariableStringBuffer( "ui_waitingScriptMenuIndex", waiting, sizeof( waiting ) );
		trap_SendConsoleCommand( va( "cmd mr %s noop\n", waiting ) );
	}

	trap_Cvar_Set( "ui_waitingScriptMenu", menu );
	trap_Cvar_Set( "ui_waitingScriptMenuIndex", va( "%i", index ) );
	trap_Cvar_Set( "ui_waitingScriptMenuNoMouse", va( "%i", noMouse ) );
}

/*
=================
CG_CheckOpenWaitingScriptMenu

One buffer carries all three cvars in turn; the binary reuses the same 1024
bytes for each read and each set (0x3002DD2E onwards).
=================
*/
static void CG_CheckOpenWaitingScriptMenu( void ) {
	char string[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer( "ui_waitingScriptMenu", string, sizeof( string ) );
	if ( !string[0] ) {
		return;
	}

	trap_Cvar_Set( "ui_newScriptMenu", string );
	trap_Cvar_VariableStringBuffer( "ui_waitingScriptMenuIndex", string, sizeof( string ) );
	trap_Cvar_Set( "ui_newScriptMenuIndex", string );
	trap_Cvar_VariableStringBuffer( "ui_waitingScriptMenuNoMouse", string, sizeof( string ) );

	if ( trap_UI_Popup( atoi( string ) ? "UIMENU_SCRIPT_POPUP_NO_MOUSE" : "UIMENU_SCRIPT_POPUP" ) ) {
		trap_Cvar_Set( "ui_waitingScriptMenu", "" );
		trap_Cvar_Set( "ui_waitingScriptMenuIndex", "-1" );
		trap_Cvar_Set( "ui_waitingScriptMenuNoMouse", "0" );
	} else {
		trap_Cvar_Set( "ui_newScriptMenu", "" );
		trap_Cvar_Set( "ui_newScriptMenuIndex", "-1" );
	}
}

/*
=================
CG_CloseScriptMenu
=================
*/
void CG_CloseScriptMenu( void ) {
	trap_UI_ClosePopup( "UIMENU_SCRIPT_POPUP" );
	trap_UI_ClosePopup( "UIMENU_SCRIPT_POPUP_NO_MOUSE" );

	trap_Cvar_Set( "ui_scriptMenu", "" );
	trap_Cvar_Set( "ui_scriptMenuIndex", "-1" );
	trap_Cvar_Set( "ui_newScriptMenu", "" );
	trap_Cvar_Set( "ui_newScriptMenuIndex", "-1" );
	trap_Cvar_Set( "ui_waitingScriptMenu", "" );
	trap_Cvar_Set( "ui_waitingScriptMenuIndex", "-1" );
	trap_Cvar_Set( "ui_waitingScriptMenuNoMouse", "0" );
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if ( text[i] == '\x19' ) {
			continue;
		}
		text[l++] = text[i];
	}
	text[l] = '\0';
}

/*
=================
CG_ReverbCmd

"r <roomtype> <wetlevel> <time>".  The clamp is a MAX() macro: both arms of it
compile to the same round-to-nearest fistp, which is why the conversion appears
twice (0x3002DFE0 and 0x3002E010).
=================
*/
static void CG_ReverbCmd( void ) {
	int argc;
	float dryLevel;
	float time;

	argc = trap_Argc();
	if ( argc != 4 ) {
		CG_Printf( "ERROR: CG_ReverbCmd called with %i args (should be 4)\n", argc );
		return;
	}

	dryLevel = atof( CG_Argv( 2 ) );
	time = atof( CG_Argv( 3 ) );

	trap_syscall_0xD9( CG_Argv( 1 ), dryLevel,
					   0 > Q_ftol( time * 1000.0f ) ? 0 : Q_ftol( time * 1000.0f ) );
}

/*
=================
CG_LocalSound

"s <index>".  The upper bound is inclusive (0x3002E084 `cmp eax,100h; jg`), so
index 256 reaches configstring 780 -- which is CS_EFFECTS, not a sound alias.
Retail quirk.
=================
*/
static void CG_LocalSound( void ) {
	int argc;
	int index;

	argc = trap_Argc();
	if ( argc != 2 ) {
		CG_Printf( "ERROR: CG_LocalSound called with %i args (should be 2)\n", argc );
		return;
	}

	index = atoi( CG_Argv( 1 ) );
	if ( index <= 0 || index > CS_SOUNDALIAS_COUNT ) {
		CG_Printf( "ERROR: CG_LocalSound called with index %i (should be in range[1,%i])\n",
				   index, CS_SOUNDALIAS_COUNT );
		return;
	}

	CG_PlaySoundAliasByName( CG_ConfigString( CS_SOUNDALIAS + index ),
							 cg.snap->ps.clientNum, cg.snap->ps.origin );
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with Cmd_Argc() / Cmd_Argv().

CoD replaced RTCW's named commands with single letters, so this is one switch on
cmd[0] rather than a chain of strcmp: an empty argv(0) means the server system
claimed the command.
=================
*/
static void CG_ServerCommand( void ) {
	const char  *cmd;
	char text[MAX_SAY_TEXT];
	int i, argc;

	cmd = CG_Argv( 0 );

	switch ( cmd[0] ) {
	case 0:
		// server claimed the command
		return;

	case 'a':
		CG_FinishWeaponChange( cg_weaponSelect.integer, atoi( CG_Argv( 1 ) ) );
		return;

	case 'b':
		CG_ParseScores();
		return;

	case 'c':
		CG_BoldGameMessage( CG_TranslateMessage( CG_Argv( 1 ), "announcement message" ) );
		return;

	case 'd':
		CG_ConfigStringModified();
		return;

	case 'e':
	case 'f':
		CG_GameMessage( CG_TranslateMessage( CG_Argv( 1 ), "game message" ) );
		return;

	case 'g':
		CG_PlayClientSoundAliasByName( (const char *)cgs.media.gameMessageSound );
		CG_BoldGameMessage( CG_TranslateMessage( CG_Argv( 1 ), "bold game message" ) );
		return;

	case 'h':
		if ( cg_teamChatsOnly.integer ) {
			return;
		}
		cmd = trap_SE_LocalizeMessage( CG_Argv( 1 ), "chat message" );
		CG_PlayClientSoundAliasByName( (const char *)cgs.media.talkSound );
		Q_strncpyz( text, cmd, sizeof( text ) );
		CG_RemoveChatEscapeChar( text );
		CG_AddToTeamChat( text );
		CG_Printf( "%s\n", text );
		return;

	case 'i':
		cmd = trap_SE_LocalizeMessage( CG_Argv( 1 ), "team chat message" );
		CG_PlayClientSoundAliasByName( (const char *)cgs.media.talkSound );
		Q_strncpyz( text, cmd, sizeof( text ) );
		CG_RemoveChatEscapeChar( text );
		CG_AddToTeamChat( text );
		CG_Printf( "%s\n", text );
		return;

	case 'j':
		CG_VoiceChat( SAY_ALL );
		return;

	case 'k':
		CG_VoiceChat( SAY_TEAM );
		return;

	case 'l':
		CG_VoiceChat( SAY_TELL );
		return;

	case 'm':
		cgs.unknown_0x0A434 = cg.time + 20000;
		cgs.unknown_0x0A430 = atoi( CG_Argv( 1 ) );
		if ( cgs.unknown_0x0A430 < 0 ) {
			cgs.unknown_0x0A434 = cg.time + 10000;
		}
		return;

	case 'n':
		CG_MapRestart();
		return;

	case 'o':
		trap_syscall_0xD4( (const float *)trap_Com_PickSoundAlias( CG_Argv( 1 ) ) );
		return;

	case 'p':
		trap_syscall_0xD5( atoi( CG_Argv( 1 ) ) );
		return;

	case 'q':
		trap_syscall_0xD7( atof( CG_Argv( 1 ) ), atoi( CG_Argv( 2 ) ) );
		return;

	case 'r':
		CG_ReverbCmd();
		return;

	case 's':
		CG_LocalSound();
		return;

	case 't':
		CG_OpenScriptMenu();
		return;

	case 'u':
		CG_CloseScriptMenu();
		return;

	case 'v':
		strcpy( text, CG_Argv( 1 ) );
		trap_Cvar_Set( text, CG_Argv( 2 ) );
		return;

	default:
		CG_Printf( "Unknown client game command: %s\n", CG_Argv( 0 ) );
		argc = trap_Argc();
		if ( argc > 1 ) {
			CG_Printf( "Arguments(%i):", argc - 1 );
			for ( i = 1 ; i < argc ; i++ ) {
				CG_Printf( " %s", CG_Argv( i ) );
			}
			CG_Printf( "\n" );
		}
		return;
	}
}

/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along with this this
snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	CG_CheckOpenWaitingScriptMenu();

	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( trap_GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
