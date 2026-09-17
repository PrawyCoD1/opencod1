/*
 * qcommon/cvar_cmds.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cvar_cmds.c
 *
 * Retail range 0x00439B10-0x0043A72.
 *
 * @fidelity: verified
 */

#include "../qcommon/qcommon.h"

#include <string.h>

/* Cvar_VariableValue  0x00439B10  VERIFIED */
float Cvar_VariableValue( const char *var_name ) {
	cvar_t  *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		return 0;
	}
	return var->value;
}

/* Cvar_VariableIntegerValue  0x00439B30  VERIFIED */
int Cvar_VariableIntegerValue( const char *var_name ) {
	cvar_t  *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		return 0;
	}
	return var->integer;
}

/* Cvar_VariableString  0x00439B50  VERIFIED */
char *Cvar_VariableString( const char *var_name ) {
	cvar_t *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		return "";
	}
	return var->string;
}

/* Cvar_VariableStringBuffer  0x00439B70  VERIFIED */
void Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	cvar_t *var;

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		*buffer = 0;
	} else {
		Q_strncpyz( buffer, var->string, bufsize );
	}
}

/* Cvar_CommandCompletion  0x00439BA0  VERIFIED */
void    Cvar_CommandCompletion( void ( *callback )( const char *s ) ) {
	cvar_t      *cvar;

	for ( cvar = cvar_vars ; cvar ; cvar = cvar->next ) {
		callback( cvar->name );
	}
}

/* ---- Cvar_SetCheatState  0x00439BD0 ---- VERIFIED */
void Cvar_SetCheatState( void ) {
	cvar_t  *var;

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->flags & CVAR_CHEAT ) {
			if ( strcmp( var->resetString, var->string ) ) {
				Cvar_Set( var->name, var->resetString );
			}
		}
	}
}

/* ---- Cvar_Command  0x00439C40 ---- VERIFIED */
qboolean Cvar_Command( void ) {
	cvar_t          *v;

	v = Cvar_FindVar( Cmd_Argv( 0 ) );
	if ( !v ) {
		return qfalse;
	}

	if ( Cmd_Argc() == 1 ) {
		Com_Printf( "\"%s\" is:\"%s" S_COLOR_WHITE "\" default:\"%s" S_COLOR_WHITE "\"\n",
					v->name, v->string, v->resetString );
		if ( v->latchedString ) {
			Com_Printf( "latched: \"%s\"\n", v->latchedString );
		}
		return qtrue;
	}

	Cvar_Set2( v->name, Cmd_Argv( 1 ), qfalse );
	return qtrue;
}

/* ---- Cvar_Toggle_f  0x00439CD0 ---- VERIFIED */
void Cvar_Toggle_f( void ) {
	cvar_t *var;
	const char *string;
	int i;

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "usage: toggle <variable> <optional value sequence>\n" );
		return;
	}

	if ( Cmd_Argc() == 2 ) {
		var = Cvar_FindVar( Cmd_Argv( 1 ) );
		Cvar_Set2( Cmd_Argv( 1 ),
				   va( "%i", var ? (int)var->value == 0 : 1 ), qfalse );
		return;
	}

	var = Cvar_FindVar( Cmd_Argv( 1 ) );
	string = var ? var->string : "";

	for ( i = 2; i < Cmd_Argc() - 1; i++ ) {
		if ( !strcmp( string, Cmd_Argv( i ) ) ) {
			Cvar_Set2( Cmd_Argv( 1 ), Cmd_Argv( i + 1 ), qfalse );
			return;
		}
	}

	Cvar_Set2( Cmd_Argv( 1 ), Cmd_Argv( 2 ), qfalse );
}

#define CVAR_SET_F_BUFFER   30000

/* ---- Cvar_Set_f  0x00439E40 ---- VERIFIED */
void Cvar_Set_f( void ) {
	int i, c, l, len;
	char combined[CVAR_SET_F_BUFFER];

	c = Cmd_Argc();
	if ( c < 3 ) {
		Com_Printf( "usage: set <variable> <value>\n" );
		return;
	}

	combined[0] = 0;
	l = 0;
	for ( i = 2 ; i < c ; i++ ) {
		len = strlen( Cmd_Argv( i ) + 1 );
		if ( l + len >= CVAR_SET_F_BUFFER - 2 ) {
			break;
		}
		strcat( combined, Cmd_Argv( i ) );
		if ( i != c - 1 ) {
			strcat( combined, " " );
		}
		l += len;
	}
	Cvar_Set2( Cmd_Argv( 1 ), combined, qfalse );
}

/* ---- Cvar_SetU_f  0x00439F80 ---- VERIFIED */
void Cvar_SetU_f( void ) {
	cvar_t  *v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf( "usage: setu <variable> <value>\n" );
		return;
	}
	Cvar_Set_f();
	v = Cvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= CVAR_USERINFO;
}

/* ---- Cvar_SetS_f  0x00439FD0 ---- VERIFIED */
void Cvar_SetS_f( void ) {
	cvar_t  *v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf( "usage: sets <variable> <value>\n" );
		return;
	}
	Cvar_Set_f();
	v = Cvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= CVAR_SERVERINFO;
}

/* ---- Cvar_SetA_f  0x0043A020 ---- VERIFIED */
void Cvar_SetA_f( void ) {
	cvar_t  *v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf( "usage: seta <variable> <value>\n" );
		return;
	}
	Cvar_Set_f();
	v = Cvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= CVAR_ARCHIVE;
}

/* ---- Cvar_SetFromCvar_f  0x0043A070 ---- VERIFIED */
void Cvar_SetFromCvar_f( void ) {
	cvar_t  *v;
	const char *value;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf( "usage: setfromcvar <variable> <variablein>\n" );
		return;
	}

	v = Cvar_FindVar( Cmd_Argv( 2 ) );
	if ( v ) {
		value = v->string;
	} else {
		value = "";
	}

	Cvar_Set2( Cmd_Argv( 1 ), value, qfalse );
}

/* ---- Cvar_Reset_f  0x0043A0D0 ---- VERIFIED */
void Cvar_Reset_f( void ) {
	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: reset <variable>\n" );
		return;
	}
	Cvar_Reset( Cmd_Argv( 1 ) );
}

/* ---- Cvar_WriteVariables  0x0043A100 ---- VERIFIED */
void Cvar_WriteVariables( fileHandle_t f ) {
	cvar_t  *var;
	char buffer[1024];

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->name && Q_stricmp( var->name, "cl_cdkey" ) == 0 ) {
			continue;
		}
		if ( var->flags & CVAR_ARCHIVE ) {
			if ( var->latchedString ) {
				Com_sprintf( buffer, sizeof( buffer ), "seta %s \"%s\"\n", var->name, var->latchedString );
			} else {
				Com_sprintf( buffer, sizeof( buffer ), "seta %s \"%s\"\n", var->name, var->string );
			}
			FS_Printf( f, "%s", buffer );
		}
	}
}

/* ---- Cvar_WriteDefaults  0x0043A1B0 ---- VERIFIED */
void Cvar_WriteDefaults( fileHandle_t f ) {
	cvar_t  *var;
	char buffer[1024];

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->name && Q_stricmp( var->name, "cl_cdkey" ) == 0 ) {
			continue;
		}
		if ( var->flags & ( CVAR_ROM | CVAR_USER_CREATED | CVAR_CHEAT | CVAR_UNSAFE ) ) {
			continue;
		}
		Com_sprintf( buffer, sizeof( buffer ), "set %s \"%s\"\n", var->name, var->resetString );
		FS_Printf( f, "%s", buffer );
	}
}

/* ---- Cvar_List_f  0x0043A260 ---- VERIFIED */
void Cvar_List_f( void ) {
	cvar_t  *var;
	int i;
	char    *match;

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	i = 0;
	for ( var = cvar_vars ; var ; var = var->next, i++ )
	{
		if ( match && !Com_Filter( match, var->name, qfalse ) ) {
			continue;
		}

		if ( var->flags & CVAR_SERVERINFO ) {
			Com_Printf( "S" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_USERINFO ) {
			Com_Printf( "U" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_ROM ) {
			Com_Printf( "R" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_INIT ) {
			Com_Printf( "I" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_ARCHIVE ) {
			Com_Printf( "A" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_LATCH ) {
			Com_Printf( "L" );
		} else {
			Com_Printf( " " );
		}
		if ( var->flags & CVAR_CHEAT ) {
			Com_Printf( "C" );
		} else {
			Com_Printf( " " );
		}

		Com_Printf( " %s \"%s\"\n", var->name, var->string );
	}

	Com_Printf( "\n%i total cvars\n", i );
	Com_Printf( "%i cvar indexes\n", cvar_numIndexes );
}

/* ---- Cvar_Dump_f  0x0043A3B0 ---- VERIFIED */
void Cvar_Dump_f( void ) {
	Com_CvarDump( 0 );
}

/* ---- Com_CvarDump  0x0043A3C0 ---- VERIFIED */
void Com_CvarDump( int channel ) {
	cvar_t  *var;
	int i;
	char    *match;
	char buffer[8196];

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	if ( !channel && !( com_logfile && com_logfile->integer ) ) {
		return;
	}

	i = 0;
	Com_PrintMessage( channel,
					  "=============================== CVAR DUMP ========================================\n" );

	for ( var = cvar_vars ; var ; var = var->next, i++ ) {
		if ( match && !Com_Filter( match, var->name, qfalse ) ) {
			continue;
		}
		if ( var->latchedString ) {
			Com_sprintf( buffer, sizeof( buffer ), "      %s \"%s\" -- latched \"%s\"\n",
						 var->name, var->string, var->latchedString );
		} else {
			Com_sprintf( buffer, sizeof( buffer ), "      %s \"%s\"\n", var->name, var->string );
		}
		Com_PrintMessage( channel, buffer );
	}

	Com_sprintf( buffer, sizeof( buffer ), "\n%i total cvars\n%i cvar indexes\n", i, cvar_numIndexes );
	Com_PrintMessage( channel, buffer );
	Com_PrintMessage( channel,
					  "=============================== END CVAR DUMP =====================================\n" );
}

/* ---- Cvar_Restart_f  0x0043A520 ---- VERIFIED */
void Cvar_Restart_f( void ) {
	cvar_t  *var;
	cvar_t  **prev;

	prev = &cvar_vars;
	while ( 1 ) {
		var = *prev;
		if ( !var ) {
			break;
		}

		if ( var->flags & ( CVAR_ROM | CVAR_INIT | CVAR_NORESTART ) ) {
			prev = &var->next;
			continue;
		}

		if ( var->flags & CVAR_USER_CREATED ) {
			*prev = var->next;
			if ( var->name ) {
				free( var->name );
			}
			if ( var->string ) {
				free( var->string );
			}
			if ( var->latchedString ) {
				free( var->latchedString );
			}
			if ( var->resetString ) {
				free( var->resetString );
			}
			memset( var, 0, sizeof( var ) );
			continue;
		}

		Cvar_Set( var->name, var->resetString );

		prev = &var->next;
	}
}

/* ---- SV_SetConfig  0x0043A5B0 ---- VERIFIED */
void SV_SetConfig( int start, int max, int bit ) {
	cvar_t  *var;

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->flags & bit ) {
			SV_SetConfigValueForKey( start, var->name, var->string, max );
		}
	}
}

/* ---- Cvar_InfoString  0x0043A5F0 ---- VERIFIED */
char    *Cvar_InfoString( int bit ) {
	static char info[MAX_INFO_STRING];
	cvar_t  *var;

	info[0] = 0;

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->flags & bit ) {
			Info_SetValueForKey( info, var->name, var->string );
		}
	}
	return info;
}

/* ---- Cvar_InfoString_Big  0x0043A630 ---- VERIFIED */
char    *Cvar_InfoString_Big( int bit ) {
	static char info[BIG_INFO_STRING];
	cvar_t  *var;

	info[0] = 0;

	for ( var = cvar_vars ; var ; var = var->next ) {
		if ( var->flags & bit ) {
			Info_SetValueForKey_Big( info, var->name, var->string );
		}
	}
	return info;
}

/* ---- Cvar_InfoStringBuffer  0x0043A670 ---- VERIFIED */
void Cvar_InfoStringBuffer( int bit, char* buff, int buffsize ) {
	Q_strncpyz( buff, Cvar_InfoString( bit ), buffsize );
}

/* ---- Cvar_AddCommands  0x0043A690 ---- VERIFIED */
void Cvar_AddCommands( void ) {
	Cmd_AddCommand( "toggle", Cvar_Toggle_f );
	Cmd_AddCommand( "set", Cvar_Set_f );
	Cmd_AddCommand( "sets", Cvar_SetS_f );
	Cmd_AddCommand( "setu", Cvar_SetU_f );
	Cmd_AddCommand( "seta", Cvar_SetA_f );
	Cmd_AddCommand( "setfromcvar", Cvar_SetFromCvar_f );
	Cmd_AddCommand( "reset", Cvar_Reset_f );
	Cmd_AddCommand( "cvarlist", Cvar_List_f );
	Cmd_AddCommand( "cvardump", Cvar_Dump_f );
	Cmd_AddCommand( "cvar_restart", Cvar_Restart_f );
}
