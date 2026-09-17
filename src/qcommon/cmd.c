/*
 * qcommon/cmd.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cmd.c
 *
 * @fidelity-default: verified
 */

#include "qcommon.h"

#define MAX_CMD_BUFFER  16384

static int cmd_wait;
static cmd_t cmd_text;
static byte cmd_text_buf[MAX_CMD_BUFFER];

int cmd_argc;
char        *cmd_argv[MAX_STRING_TOKENS];
static char cmd_tokenized[BIG_INFO_STRING + MAX_STRING_TOKENS];

static cmd_function_t  *cmd_functions;

/* ---- Cmd_Wait_f  0x00428150 ---- VERIFIED */
void Cmd_Wait_f( void ) {
	if ( Cmd_Argc() == 2 ) {
		cmd_wait = atoi( Cmd_Argv( 1 ) );
	} else {
		cmd_wait = 1;
	}
}

/* ---- Cbuf_Init  0x00428180 ---- VERIFIED */
void Cbuf_Init( void ) {
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.cursize = 0;
}

/* ---- Cbuf_AddText  0x004281A0 ---- VERIFIED */
void Cbuf_AddText( const char *text ) {
	int l;

	l = strlen( text );

	if ( cmd_text.cursize + l >= cmd_text.maxsize ) {
		Com_Printf( "Cbuf_AddText: overflow\n" );
		return;
	}
	Com_Memcpy( &cmd_text.data[cmd_text.cursize], text, l );
	cmd_text.cursize += l;
}

/* ---- Cbuf_InsertText  0x00428200 ---- VERIFIED */
void Cbuf_InsertText( const char *text ) {
	int len;
	int i;

	len = strlen( text ) + 1;
	if ( len + cmd_text.cursize > cmd_text.maxsize ) {
		Com_Printf( "Cbuf_InsertText overflowed\n" );
		return;
	}

	for ( i = cmd_text.cursize - 1 ; i >= 0 ; i-- ) {
		cmd_text.data[ i + len ] = cmd_text.data[ i ];
	}

	Com_Memcpy( cmd_text.data, text, len - 1 );

	cmd_text.data[ len - 1 ] = '\n';

	cmd_text.cursize += len;
}

/* ---- Cbuf_ExecuteText  0x00428290 ---- VERIFIED */
void Cbuf_ExecuteText( int exec_when, const char *text ) {
	switch ( exec_when )
	{
	case EXEC_NOW:
		if ( text && strlen( text ) > 0 ) {
			Cmd_ExecuteString( text );
		} else {
			Cbuf_Execute();
		}
		break;
	case EXEC_INSERT:
		Cbuf_InsertText( text );
		break;
	case EXEC_APPEND:
		Cbuf_AddText( text );
		break;
	default:
		Com_Error( ERR_FATAL, "\x15" "Cbuf_ExecuteText: bad exec_when" );
	}
}

/* ---- Cbuf_Execute  0x00428300 ---- VERIFIED */
void Cbuf_Execute( void ) {
	int i;
	char    *text;
	char line[MAX_CMD_LINE];
	int quotes;

	while ( cmd_text.cursize )
	{
		if ( cmd_wait ) {
			cmd_wait--;
			break;
		}

		text = (char *)cmd_text.data;

		quotes = 0;
		for ( i = 0 ; i < cmd_text.cursize ; i++ )
		{
			if ( text[i] == '"' ) {
				quotes++;
			}
			if ( !( quotes & 1 ) &&  text[i] == ';' ) {
				break;
			}
			if ( text[i] == '\n' || text[i] == '\r' ) {
				break;
			}
		}

		if ( i >= ( MAX_CMD_LINE - 1 ) ) {
			i = MAX_CMD_LINE - 1;
		}

		Com_Memcpy( line, text, i );
		line[i] = 0;

		if ( i == cmd_text.cursize ) {
			cmd_text.cursize = 0;
		} else
		{
			i++;
			cmd_text.cursize -= i;
			memmove( text, text + i, cmd_text.cursize );
		}

		Cmd_ExecuteString( line );
	}
}

/* ---- Cmd_Exec_f  0x00428410 ---- VERIFIED */
void Cmd_Exec_f( void ) {
	char    *f;
	char filename[MAX_QPATH];

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "exec <filename> : execute a script file\n" );
		return;
	}

	Q_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	Com_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	FS_ReadFile( filename, (void **)&f );
	if ( !f ) {
		Com_Printf( "couldn't exec %s\n",Cmd_Argv( 1 ) );
		return;
	}
	Com_Printf( "execing %s\n",Cmd_Argv( 1 ) );

	Cbuf_InsertText( f );

	FS_FreeFile( f );
}

/* ---- Cmd_Vstr_f  0x00428500 ---- VERIFIED */
void Cmd_Vstr_f( void ) {
	char    *v;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "vstr <variablename> : execute a variable command\n" );
		return;
	}

	v = Cvar_VariableString( Cmd_Argv( 1 ) );
	Cbuf_InsertText( va( "%s\n", v ) );
}

/* ---- Cmd_Echo_f  0x00428560 ---- VERIFIED */
void Cmd_Echo_f( void ) {
	int i;

	for ( i = 1 ; i < Cmd_Argc() ; i++ )
		Com_Printf( "%s ",Cmd_Argv( i ) );
	Com_Printf( "\n" );
}

/* ---- Cmd_Argc  0x004285B0 ---- VERIFIED */
int     Cmd_Argc( void ) {
	return cmd_argc;
}

/* ---- Cmd_Argv  0x004285C0 ---- VERIFIED */
char    *Cmd_Argv( int arg ) {
	if ( (unsigned int)arg < (unsigned int)cmd_argc ) {
		return cmd_argv[arg];
	}
	return "";
}

/* ---- Cmd_ArgvBuffer  0x004285E0 ---- VERIFIED */
void    Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, Cmd_Argv( arg ), bufferLength );
}

/* ---- Cmd_Args  0x00428620 ---- VERIFIED */
char    *Cmd_Args( void ) {
	static char cmd_args[MAX_STRING_CHARS];
	int i;

	cmd_args[0] = 0;
	for ( i = 1 ; i < cmd_argc ; i++ ) {
		strcat( cmd_args, cmd_argv[i] );
		if ( i != cmd_argc - 1 ) {
			strcat( cmd_args, " " );
		}
	}

	return cmd_args;
}

/* ---- Cmd_ArgsBuffer  0x004286B0 ---- VERIFIED */
void    Cmd_ArgsBuffer( char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, Cmd_Args(), bufferLength );
}

/* ---- Cmd_TokenizeString2  0x004286D0 ---- VERIFIED */
void Cmd_TokenizeString2( const char *text_in, int max_tokens ) {
	const char  *text;
	char    *textOut;

	cmd_argc = 0;

	if ( !text_in ) {
		return;
	}

	text = text_in;
	textOut = cmd_tokenized;

	while ( 1 ) {
		if ( !--max_tokens ) {
			if ( !*text ) {
				return;
			}
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;
			while ( *text ) {
				*textOut++ = *text++;
			}
			*textOut = 0;
			return;
		}

		while ( 1 ) {
			while ( *text && *text <= ' ' ) {
				text++;
			}
			if ( !*text ) {
				return;
			}

			if ( text[0] == '/' && text[1] == '/' ) {
				return;
			}

			if ( text[0] == '/' && text[1] == '*' ) {
				while ( *text && ( text[0] != '*' || text[1] != '/' ) ) {
					text++;
				}
				if ( !*text ) {
					return;
				}
				text += 2;
			} else {
				break;
			}
		}

		if ( *text == '"' ) {
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;
			text++;
			while ( *text && *text != '"' ) {
				*textOut++ = *text++;
			}
			*textOut++ = 0;
			if ( !*text ) {
				return;
			}
			text++;
		} else {
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;

			while ( *text > ' ' ) {
				if ( text[0] == '"' ) {
					break;
				}
				if ( text[0] == '/' && ( text[1] == '/' || text[1] == '*' ) ) {
					break;
				}
				*textOut++ = *text++;
			}
			*textOut++ = 0;
		}

		if ( !*text ) {
			return;
		}
		if ( *text <= ' ' ) {
			text++;
		}
		if ( cmd_argc == MAX_STRING_TOKENS ) {
			return;
		}
	}
}

/* ---- Cmd_TokenizeString  0x00428830 ---- VERIFIED */
void Cmd_TokenizeString( const char *text_in ) {
	Cmd_TokenizeString2( text_in, 0 );
}

/* ---- Cmd_AddCommand  0x00428840 ---- VERIFIED */
void    Cmd_AddCommand( const char *cmd_name, xcommand_t function ) {
	cmd_function_t  *cmd;
	int len;

	for ( cmd = cmd_functions ; cmd ; cmd = cmd->next ) {
		if ( !strcmp( cmd_name, cmd->name ) ) {
			if ( function != NULL ) {
				Com_Printf( "Cmd_AddCommand: %s already defined\n", cmd_name );
			}
			return;
		}
	}

	cmd = Com_MallocOrDie( sizeof( cmd_function_t ) );
	Com_Memset( cmd, 0, sizeof( cmd_function_t ) );

	len = strlen( cmd_name ) + 1;
	cmd->name = Com_MallocOrDie( len );
	Com_Memset( cmd->name, 0, len );
	strcpy( cmd->name, cmd_name );

	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/* ---- Cmd_RemoveCommand  0x00428990 ---- VERIFIED */
void    Cmd_RemoveCommand( const char *cmd_name ) {
	cmd_function_t  *cmd, **back;

	back = &cmd_functions;
	while ( 1 ) {
		cmd = *back;
		if ( !cmd ) {
			return;
		}
		if ( !strcmp( cmd_name, cmd->name ) ) {
			*back = cmd->next;
			if ( cmd->name ) {
				free( cmd->name );
			}
			free( cmd );
			return;
		}
		back = &cmd->next;
	}
}

/* ---- Cmd_Shutdown  0x00428A10 ---- VERIFIED */
void    Cmd_Shutdown( void ) {
	cmd_function_t  *cmd;

	while ( cmd_functions ) {
		cmd = cmd_functions;
		cmd_functions = cmd_functions->next;
		free( cmd->name );
		free( cmd );
	}
}

/* ---- Cmd_CommandCompletion  0x00428A50 ---- VERIFIED */
void    Cmd_CommandCompletion( void ( *callback )( const char *s ) ) {
	cmd_function_t  *cmd;

	for ( cmd = cmd_functions ; cmd ; cmd = cmd->next ) {
		callback( cmd->name );
	}
}

/* ---- Cmd_ExecuteString  0x00428A80 ---- VERIFIED */
void    Cmd_ExecuteString( const char *text ) {
	cmd_function_t  *cmd, **prev;

	Cmd_TokenizeString2( text, 0 );
	if ( !Cmd_Argc() ) {
		return;
	}

	for ( prev = &cmd_functions ; *prev ; prev = &cmd->next ) {
		cmd = *prev;
		if ( cmd_argv[0] && cmd->name && !Q_stricmpn( cmd->name, cmd_argv[0], 99999 ) ) {
			*prev = cmd->next;
			cmd->next = cmd_functions;
			cmd_functions = cmd;

			if ( !cmd->function ) {
				break;
			} else {
				cmd->function();
			}
			return;
		}
	}

	if ( Cvar_Command() ) {
		return;
	}

	if ( com_cl_running && com_cl_running->integer && cgvm ) {
		if ( VM_Call( cgvm, CG_CONSOLE_COMMAND ) ) {
			return;
		}
	}

	if ( com_sv_running && com_sv_running->integer && sv.state == SS_GAME ) {
		g_unk_A9CC58 = 1;
		if ( VM_Call( vm, GAME_CONSOLE_COMMAND ) ) {
			return;
		}
	}

	if ( com_cl_running && com_cl_running->integer && uivm ) {
		if ( VM_Call( uivm, UI_CONSOLE_COMMAND, cls_realtime ) ) {
			return;
		}
	}

	CL_ForwardCommandToServer( text );
}

/* ---- Cmd_List_f  0x00428BB0 ---- VERIFIED */
void Cmd_List_f( void ) {
	cmd_function_t  *cmd;
	int i;
	char            *match;

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	i = 0;
	for ( cmd = cmd_functions ; cmd ; cmd = cmd->next ) {
		if ( match && !Com_Filter( match, cmd->name, qfalse ) ) {
			continue;
		}

		Com_Printf( "%s\n", cmd->name );
		i++;
	}
	Com_Printf( "%i commands\n", i );
}

/* ---- Cmd_Init  0x00428C30 ---- VERIFIED */
void Cmd_Init( void ) {
	Cmd_AddCommand( "cmdlist",Cmd_List_f );
	Cmd_AddCommand( "exec",Cmd_Exec_f );
	Cmd_AddCommand( "vstr",Cmd_Vstr_f );
	Cmd_AddCommand( "echo",Cmd_Echo_f );
	Cmd_AddCommand( "wait", Cmd_Wait_f );
}

void *Com_MallocOrDie( int size ) {
	void *p;

	p = malloc( size );
	if ( !p ) {
		Sys_Error( "Com_MallocOrDie: failed to allocate %i bytes", size );
	}
	return p;
}
