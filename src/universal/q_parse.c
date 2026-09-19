/*
 * universal/q_parse.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/q_parse.c
 *
 * Retail range 0x00449790-0x0044A529.
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_TOKEN_CHARS 1024

static parseInfo_t parseInfo_stack[16] = {
	{ "", 1, qfalse, qtrue, qfalse }    /* [0]: retail .data 0x00575FF0 */
};
parseInfo_t *parseInfo = parseInfo_stack;
static int parseInfo_level;

static char *prevTokenPos;
static char *lastTokenPos;

static char com_lineBuffer[MAX_TOKEN_CHARS];

static const char *com_parseSeparators[] = {
	"+=", "-=", "*=", "/=", "&=", "|=",
	"++", "--", "&&", "||", "<=", ">=", "==", "!=",
	NULL
};

/* ---- Com_BeginParseSession  0x00449790 ---- */
void Com_BeginParseSession( const char *name ) {
	parseInfo_t *pi;

	if ( parseInfo_level == 15 ) {
		Com_Error( ERR_FATAL, "\x15" "Com_BeginParseSession: session overflow" );
	}
	parseInfo_level++;
	pi = &parseInfo_stack[parseInfo_level];
	pi->currentLine = 1;
	pi->ungetReady = qfalse;
	pi->spaceDelimited = qtrue;
	pi->csv = qfalse;
	parseInfo = pi;
	strncpy( pi->filename, name, 63 );
	pi->filename[63] = 0;
}

/* ---- Com_EndParseSession  0x00449810 ---- */
void Com_EndParseSession( void ) {
	if ( !parseInfo_level ) {
		Com_Error( ERR_FATAL, "\x15" "Com_EndParseSession: session underflow" );
	}
	parseInfo_level--;
	parseInfo = &parseInfo_stack[parseInfo_level];
}

/* ---- Com_ResetParseSessions  0x00449850 ---- */
void Com_ResetParseSessions( void ) {
	parseInfo_level = 0;
	parseInfo = parseInfo_stack;
}

/* ---- Com_SetSpaceDelimited  0x00449870 ---- */
void Com_SetSpaceDelimited( qboolean enabled ) {
	parseInfo->spaceDelimited = enabled;
}

void Com_SetCSV( qboolean enabled ) {
	parseInfo->csv = enabled;
}

void Com_SetParseNegativeNumbers( qboolean enabled ) {
	parseInfo->parseNegativeNumbers = enabled;
}

/* ---- Com_GetCurrentParseLine  0x004498A0 ---- */
int Com_GetCurrentParseLine( void ) {
	return parseInfo->currentLine;
}

/* ---- Com_ScriptError  0x004498B0 ---- */
void Com_ScriptError( const char *fmt, ... ) {
	va_list argptr;
	char string[32000];

	va_start( argptr, fmt );
	vsprintf( string, fmt, argptr );
	va_end( argptr );

	Com_Error( ERR_DROP, "\x15" "File %s, line %i: %s", parseInfo->filename, parseInfo->currentLine, string );
}

/* ---- Com_ScriptWarning  0x00449930 ---- */
void Com_ScriptWarning( const char *fmt, ... ) {
	va_list argptr;
	char string[32000];

	va_start( argptr, fmt );
	vsprintf( string, fmt, argptr );
	va_end( argptr );

	Com_Printf( "File %s, line %i: %s", parseInfo->filename, parseInfo->currentLine, string );
}

/* ---- Com_UngetToken  0x004499B0 ---- */
void Com_UngetToken( void ) {
	if ( parseInfo->ungetReady ) {
		Com_ScriptError( "UngetToken called twice" );
	}
	lastTokenPos = prevTokenPos;
	parseInfo->ungetReady = qtrue;
}

/* ---- Com_ParseSetMark  0x00449A00 ---- */
void Com_ParseSetMark( int *mark, char **data_p ) {
	mark[0] = parseInfo->currentLine;
	mark[1] = (int)*data_p;
	mark[2] = parseInfo->ungetReady;
	mark[3] = parseInfo->ungetLineSave;
	mark[4] = (int)parseInfo->ungetTokenSave;
}

void Com_ParseReturnToMark( int *mark, char **data_p ) {
	parseInfo->currentLine = mark[0];
	*data_p = (char *)mark[1];
	parseInfo->ungetReady = (qboolean)mark[2];
	parseInfo->ungetLineSave = mark[3];
	parseInfo->ungetTokenSave = (char *)mark[4];
}

/* ---- SkipWhitespace  0x00449A70 ---- */
static char *SkipWhitespace( char *data, qboolean *hasNewLines ) {
	int c;

	while ( ( c = *data ) <= ' ' ) {
		if ( !c ) {
			return NULL;
		}
		if ( c == '\n' ) {
			parseInfo->currentLine++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

/* ---- Com_Compress  0x00449AB0 ---- */
int Com_Compress( char *data_p ) {
	char    *datai, *datao;
	char c;
	int size;

	size = 0;
	datao = datai = data_p;
	if ( datai ) {
		while ( ( c = *datai ) != 0 ) {
			// retail 0x00449AC3: explicit '\r'/'\n' arm before the '/' test (Q3's newline flag, made a plain copy); c is a char (8-bit cmp, no movsx)
			if ( c == '\r' || c == '\n' ) {
				*datao++ = c;
				datai++;
				size++;
			} else if ( c == '/' && datai[1] == '/' ) {
				while ( *datai && *datai != '\n' ) {
					datai++;
				}
			}
			else if ( c == '/' && datai[1] == '*' ) {
				if ( *datai ) {
					while ( *datai && ( *datai != '*' || datai[1] != '/' ) ) {
						datai++;
					}
					if ( *datai ) {
						datai += 2;
					}
				}
			} else {
				*datao++ = c;
				datai++;
				size++;
			}
		}
	}
	*datao = 0;
	return size;
}

/* ---- Com_GetLastTokenPos  0x00449B40 ---- */
char *Com_GetLastTokenPos( void ) {
	return lastTokenPos;
}

/* ---- Com_ParseCSV  0x00449B50 ---- */
static char *Com_ParseCSV( qboolean allowLineBreaks, char **data_p ) {
	char    *data;
	char c;
	int len;

	len = 0;
	data = *data_p;
	parseInfo->token[0] = 0;

	if ( allowLineBreaks ) {
		while ( *data == '\r' || *data == '\n' ) {
			data++;
		}
	} else if ( *data == '\r' || *data == '\n' ) {
		return parseInfo->token;
	}

	prevTokenPos = lastTokenPos;
	lastTokenPos = data;

	c = *data;
	if ( c ) {
		while ( c != ',' && c != '\n' ) {
			if ( c != '\r' ) {
				if ( c == '"' ) {
					while ( 1 ) {
						data++;
						while ( *data == '"' ) {
							if ( data[1] != '"' ) {
								goto nextChar;
							}
							if ( len < MAX_TOKEN_CHARS - 1 ) {
								parseInfo->token[len++] = '"';
							}
							data += 2;
						}
						if ( len < MAX_TOKEN_CHARS - 1 ) {
							parseInfo->token[len++] = *data;
						}
					}
				}
				if ( len < MAX_TOKEN_CHARS - 1 ) {
					parseInfo->token[len++] = c;
				}
			}
nextChar:
			c = *++data;
			if ( !c ) {
				break;
			}
		}
	}

	if ( !*data ) {
		*data_p = NULL;
	} else {
		if ( *data != '\n' ) {
			data++;
		}
		*data_p = data;
	}
	parseInfo->token[len] = 0;
	return parseInfo->token;
}

/* ---- Com_ParseExt  0x00449C10 ---- */
char *Com_ParseExt( char **data_p, qboolean allowLineBreaks ) {
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	char    *data;

	if ( !data_p ) {
		Com_Error( ERR_FATAL, "\x15" "Com_ParseExt: NULL data_p" );
	}
	data = *data_p;
	len = 0;
	parseInfo->token[0] = 0;

	if ( !data ) {
		*data_p = NULL;
		return parseInfo->token;
	}

	parseInfo->ungetLineSave = parseInfo->currentLine;
	parseInfo->ungetTokenSave = data;

	if ( parseInfo->csv ) {
		return Com_ParseCSV( allowLineBreaks, data_p );
	}

	data = SkipWhitespace( data, &hasNewLines );
	if ( !data ) {
		*data_p = NULL;
		return parseInfo->token;
	}

	while ( 1 )
	{
		if ( hasNewLines && !allowLineBreaks ) {
			*data_p = data;
			return parseInfo->token;
		}

		c = *data;

		if ( c == '/' && data[1] == '/' ) {
			while ( *data && *data != '\n' ) {
				data++;
			}
			data = SkipWhitespace( data, &hasNewLines );
			if ( !data ) {
				*data_p = NULL;
				return parseInfo->token;
			}
			continue;
		}
		if ( c == '/' && data[1] == '*' ) {
			while ( *data && ( *data != '*' || data[1] != '/' ) ) {
				if ( *data == '\n' ) {
					parseInfo->currentLine++;
				}
				data++;
			}
			if ( *data ) {
				data += 2;
			}
			data = SkipWhitespace( data, &hasNewLines );
			if ( !data ) {
				*data_p = NULL;
				return parseInfo->token;
			}
			continue;
		}
		break;
	}

	if ( c == '\"' ) {
		data++;
		while ( 1 )
		{
			c = *data++;
			if ( c == '\\' && *data == '\"' ) {
				c = '\"';
				data++;
			} else if ( c == '\"' || !c ) {
				break;
			}
			if ( *data == '\n' ) {
				parseInfo->currentLine++;
			}
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
		}
		if ( len == MAX_TOKEN_CHARS ) {
			len = 0;
		}
		parseInfo->token[len] = 0;
		*data_p = data;
		return parseInfo->token;
	}

	if ( parseInfo->spaceDelimited ) {
		do
		{
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
		} while ( c > ' ' );

		if ( len == MAX_TOKEN_CHARS ) {
			len = 0;
		}
		parseInfo->token[len] = 0;
		*data_p = data;
		return parseInfo->token;
	}

	if ( ( c >= '0' && c <= '9' ) ||
		 ( parseInfo->parseNegativeNumbers && c == '-' && data[1] >= '0' && data[1] <= '9' ) ||
		 ( c == '.' && data[1] >= '0' && data[1] <= '9' ) ) {
		do {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
		} while ( ( c >= '0' && c <= '9' ) || c == '.' );

		if ( c == 'e' || c == 'E' ) {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
			if ( c == '-' || c == '+' ) {
				if ( len < MAX_TOKEN_CHARS - 1 ) {
					parseInfo->token[len] = c;
					len++;
				}
				data++;
				c = *data;
			}
			while ( c >= '0' && c <= '9' ) {
				if ( len < MAX_TOKEN_CHARS - 1 ) {
					parseInfo->token[len] = c;
					len++;
				}
				data++;
				c = *data;
			}
		}
	} else if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
				c == '_' || c == '/' || c == '\\' ) {
		do {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
		} while ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
				  c == '_' || ( c >= '0' && c <= '9' ) ||
				  c == '/' || c == '\\' || c == ':' || c == '.' );
	} else {
		const char **sep = com_parseSeparators;

		if ( sep ) {
			while ( *sep ) {
				int seplen = strlen( *sep );
				int i;

				for ( i = 0; i < seplen; i++ ) {
					if ( data[i] != ( *sep )[i] ) {
						break;
					}
				}
				if ( i == seplen ) {
					memcpy( parseInfo->token, *sep, seplen );
					parseInfo->token[seplen] = 0;
					*data_p = data + seplen;
					return parseInfo->token;
				}
				sep++;
			}
		}

		parseInfo->token[0] = *data;
		parseInfo->token[1] = 0;
		*data_p = data + 1;
		return parseInfo->token;
	}

	if ( len == MAX_TOKEN_CHARS ) {
		len = 0;
	}
	parseInfo->token[len] = 0;
	*data_p = data;
	return parseInfo->token;
}

/* ---- Com_Parse  0x00449F80 ---- */
char *Com_Parse( char **data_p ) {
	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	return Com_ParseExt( data_p, qtrue );
}

/* ---- Com_ParseOnLine  0x00449FC0 ---- */
char *Com_ParseOnLine( char **data_p ) {
	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		if ( !parseInfo->spaceDelimited ) {
			return parseInfo->token;
		}
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	return Com_ParseExt( data_p, qfalse );
}

/* ---- COM_MatchToken  0x0044A010 ---- */
qboolean COM_MatchToken( char **data_p, const char *match, int warn ) {
	char    *token;

	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	token = Com_ParseExt( data_p, qtrue );

	if ( strcmp( token, match ) ) {
		if ( !warn ) {
			Com_ScriptError( "MatchToken: %s != %s", token, match );
		}
		Com_ScriptWarning( "MatchToken: %s != %s", token, match );
		return qtrue;
	}
	return qfalse;
}

/* ---- Com_SkipBracedSection  0x0044A0B0 ---- */
int Com_SkipBracedSection( char **data_p, int depth ) {
	char    *token;
	int braceLevel;
	qboolean found;

	found = qfalse;
	braceLevel = 0;
	do {
		if ( parseInfo->ungetReady ) {
			parseInfo->ungetReady = qfalse;
			*data_p = parseInfo->ungetTokenSave;
			parseInfo->currentLine = parseInfo->ungetLineSave;
		}
		token = Com_ParseExt( data_p, qtrue );
		if ( token[1] == 0 ) {
			if ( token[0] == '{' ) {
				if ( braceLevel == depth ) {
					found = qtrue;
				} else {
					braceLevel++;
				}
			} else if ( token[0] == '}' ) {
				braceLevel--;
			}
		}
	} while ( braceLevel && *data_p );
	return found;
}

/* ---- Com_SkipRestOfLine  0x0044A130 ---- */
void Com_SkipRestOfLine( int unused, char **data_p ) {
	char    *p;
	int c;

	(void)unused;

	p = *data_p;
	if ( !p ) {
		return;
	}
	c = *p;
	if ( !c ) {
		*data_p = p;
		return;
	}
	while ( 1 ) {
		p++;
		if ( c == '\n' ) {
			break;
		}
		c = *p;
		if ( !c ) {
			*data_p = p;
			return;
		}
	}
	parseInfo->currentLine++;
	*data_p = p;
}

/* ---- Com_ParseRestOfLine  0x0044A160 ---- */
char *Com_ParseRestOfLine( char **data_p ) {
	char    *token;

	com_lineBuffer[0] = 0;
	while ( 1 ) {
		if ( parseInfo->ungetReady ) {
			parseInfo->ungetReady = qfalse;
			if ( !parseInfo->spaceDelimited ) {
				token = parseInfo->token;
				goto gotToken;
			}
			*data_p = parseInfo->ungetTokenSave;
			parseInfo->currentLine = parseInfo->ungetLineSave;
		}
		token = Com_ParseExt( data_p, qfalse );
gotToken:
		if ( !token[0] ) {
			return com_lineBuffer;
		}

		if ( com_lineBuffer[0] ) {
			Q_strcat( com_lineBuffer, sizeof( com_lineBuffer ), " " );
		}
		Q_strcat( com_lineBuffer, sizeof( com_lineBuffer ), token );
	}
}

/* ---- Com_ParseFloat  0x0044A270 ---- */
float Com_ParseFloat( char **data_p ) {
	char    *token;

	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	token = Com_ParseExt( data_p, qtrue );
	if ( *token ) {
		return (float)atof( token );
	}
	return 0;
}

int Com_ParseInt( char **data_p ) {
	char    *token;

	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	token = Com_ParseExt( data_p, qtrue );
	if ( *token ) {
		return atol( token );
	}
	return 0;
}

/* ---- Parse_1DMatrix  0x0044A310 ---- */
void Parse_1DMatrix( char **data_p, int x, float *m ) {
	char    *token;
	int i;

	COM_MatchToken( data_p, "(", 0 );

	for ( i = 0 ; i < x ; i++ ) {
		if ( parseInfo->ungetReady ) {
			parseInfo->ungetReady = qfalse;
			*data_p = parseInfo->ungetTokenSave;
			parseInfo->currentLine = parseInfo->ungetLineSave;
		}
		token = Com_ParseExt( data_p, qtrue );
		m[i] = (float)atof( token );
	}

	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	token = Com_ParseExt( data_p, qtrue );
	if ( strcmp( token, ")" ) ) {
		Com_ScriptError( "MatchToken: %s != %s", token, ")" );
	}
}

void Parse_2DMatrix( char **data_p, int y, int x, float *m ) {
	char    *token;
	int i;

	COM_MatchToken( data_p, "(", 0 );

	for ( i = 0 ; i < y ; i++ ) {
		Parse_1DMatrix( data_p, x, m + i * x );
	}

	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	token = Com_ParseExt( data_p, qtrue );
	if ( strcmp( token, ")" ) ) {
		Com_ScriptError( "MatchToken: %s != %s", token, ")" );
	}
}

void Parse_3DMatrix( char **data_p, int z, int y, int x, float *m ) {
	char    *token;
	int i;

	COM_MatchToken( data_p, "(", 0 );

	for ( i = 0 ; i < z ; i++ ) {
		Parse_2DMatrix( data_p, y, x, m + i * x * y );
	}

	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	token = Com_ParseExt( data_p, qtrue );
	if ( strcmp( token, ")" ) ) {
		Com_ScriptError( "MatchToken: %s != %s", token, ")" );
	}
}
