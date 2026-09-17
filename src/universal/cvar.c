/*
 * universal/cvar.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/cvar.c
 *
 * Retail range 0x004391F0-0x00439B0.
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

cvar_t      *cvar_vars;
cvar_t      *cvar_cheats;
int cvar_modifiedFlags;

cvar_t cvar_indexes[MAX_CVARS];
int cvar_numIndexes;

static cvar_t  *cvar_hashTable[FILE_HASH_SIZE];

/* ---- generateHashValue_Cvar  0x004391F0 ---- */
static long generateHashValue_Cvar( const char *fname ) {
	int i;
	long hash;
	char letter;

	if ( !fname ) {
		Com_Error( ERR_DROP, "\x15" "null name in generateHashValue" );
	}
	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		letter = tolower( fname[i] );
		hash += (long)( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( FILE_HASH_SIZE - 1 );
	return hash;
}

/* ---- Cvar_ValidateString  0x00439240 ---- */
static qboolean Cvar_ValidateString( const char *s ) {
	if ( !s ) {
		return qfalse;
	}
	if ( strchr( s, '\\' ) ) {
		return qfalse;
	}
	if ( strchr( s, '\"' ) ) {
		return qfalse;
	}
	if ( strchr( s, ';' ) ) {
		return qfalse;
	}
	return qtrue;
}

/* ---- Cvar_FindVar  0x00439280 ---- */
cvar_t *Cvar_FindVar( const char *var_name ) {
	cvar_t  *var;
	long hash;

	hash = generateHashValue_Cvar( var_name );

	for ( var = cvar_hashTable[hash] ; var ; var = var->hashNext ) {
		if ( !Q_stricmp( var_name, var->name ) ) {
			return var;
		}
	}

	return NULL;
}

/* ---- Cvar_Shutdown  0x004392D0 ---- */
void Cvar_Shutdown( void ) {
	int i;
	cvar_t  *var;

	for ( i = 0 ; i < FILE_HASH_SIZE ; i++ ) {
		for ( var = cvar_hashTable[i] ; var ; var = var->hashNext ) {
			if ( var->latchedString ) {
				free( var->latchedString );
				var->latchedString = NULL;
			}
			if ( var->string ) {
				free( var->string );
				var->string = NULL;
			}
			if ( var->resetString ) {
				free( var->resetString );
				var->resetString = NULL;
			}
			free( var->name );
			var->name = NULL;
		}
	}
}

/* ---- Cvar_Get  0x00439350 ---- */
cvar_t *Cvar_Get( const char *var_name, const char *var_value, int flags ) {
	cvar_t  *var;
	long hash;
	cvar_t  **prev;
	int len;

	if ( !var_name || !var_value ) {
		Com_Error( ERR_FATAL, "\x15" "Cvar_Get: NULL parameter" );
	}

	if ( strchr( var_name, '\\' ) || strchr( var_name, '\"' ) || strchr( var_name, ';' ) ) {
		Com_Error( ERR_FATAL, va( "invalid cvar name string: %s", var_name ) );
	}

	var = Cvar_FindVar( var_name );
	if ( var ) {
		if ( ( var->flags & ( CVAR_USER_CREATED | CVAR_UNSAFE ) )
			 && !( flags & ( CVAR_USER_CREATED | CVAR_UNSAFE ) )
			 && ( var_value[0] || ( flags & CVAR_CHEAT ) ) ) {
			var->flags &= ~( CVAR_USER_CREATED | CVAR_UNSAFE );
			free( var->resetString );
			var->resetString = CopyStringInternal( var_value );
			cvar_modifiedFlags |= flags;
		}

		var->flags |= flags;

		if ( !var->resetString[0] ) {
			free( var->resetString );
			var->resetString = CopyStringInternal( var_value );
		} else if ( var_value[0] && strcmp( var->resetString, var_value ) ) {
			Com_DPrintf( "Warning: cvar \"%s\" given initial values: \"%s\" and \"%s\"\n",
						 var_name, var->resetString, var_value );
		}

		if ( var->latchedString ) {
			char *s;

			s = var->latchedString;
			var->latchedString = NULL;
			Cvar_Set2( var_name, s, qtrue );
			free( s );
		}

		return var;
	}

	if ( cvar_numIndexes >= MAX_CVARS ) {
		Com_Error( ERR_FATAL, "MAX_CVARS" );
	}
	var = &cvar_indexes[cvar_numIndexes];
	cvar_numIndexes++;

	len = strlen( var_name ) + 1;
	var->name = malloc( len );
	if ( !var->name ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( var->name, 0, len );
	strcpy( var->name, var_name );

	len = strlen( var_value ) + 1;
	var->string = malloc( len );
	if ( !var->string ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( var->string, 0, len );
	strcpy( var->string, var_value );

	var->modified = qtrue;
	var->modificationCount = 1;
	var->value = atof( var->string );
	var->integer = atoi( var->string );

	len = strlen( var_value ) + 1;
	var->resetString = malloc( len );
	if ( !var->resetString ) {
		Sys_OutOfMemoryError();
	}
	Com_Memset( var->resetString, 0, len );
	strcpy( var->resetString, var_value );

	prev = &cvar_vars;
	while ( *prev ) {
		if ( Q_stricmp( var->name, ( *prev )->name ) < 0 ) {
			break;
		}
		prev = &( *prev )->next;
	}
	var->next = *prev;
	*prev = var;

	var->flags = flags;

	hash = generateHashValue_Cvar( var_name );
	var->hashNext = cvar_hashTable[hash];
	cvar_hashTable[hash] = var;

	return var;
}

/* ---- Cvar_Set2  0x00439650 ---- */
cvar_t *Cvar_Set2( const char *var_name, const char *value, qboolean force ) {
	cvar_t  *var;

	Com_PrintMessage( 4, va( "      cvar set %s %s\n", var_name, value ) );

	if ( !Cvar_ValidateString( var_name ) ) {
		Com_Error( ERR_FATAL, va( "invalid cvar name string: %s", var_name ) );
	}

	var = Cvar_FindVar( var_name );
	if ( !var ) {
		if ( !value ) {
			return NULL;
		}
		if ( !force ) {
			return Cvar_Get( var_name, value, CVAR_USER_CREATED );
		} else {
			return Cvar_Get( var_name, value, 0 );
		}
	}

	if ( !value ) {
		value = var->resetString;
	}

	if ( !strcmp( value, var->string ) ) {
		if ( ( var->flags & CVAR_LATCH ) && var->latchedString ) {
			free( var->latchedString );
			var->latchedString = NULL;
		}
		return var;
	}

	cvar_modifiedFlags |= var->flags;

	if ( !force ) {
		if ( var->flags & CVAR_ROM ) {
			Com_Printf( "%s is read only.\n", var_name );
			return var;
		}

		if ( var->flags & CVAR_INIT ) {
			Com_Printf( "%s is write protected.\n", var_name );
			return var;
		}

		if ( ( var->flags & CVAR_CHEAT ) && !cvar_cheats->integer ) {
			Com_Printf( "%s is cheat protected.\n", var_name );
			return var;
		}

		if ( var->flags & CVAR_LATCH ) {
			if ( var->latchedString ) {
				if ( !strcmp( value, var->latchedString ) ) {
					return var;
				}
				free( var->latchedString );
			} else {
				if ( !strcmp( value, var->string ) ) {
					return var;
				}
			}

			Com_Printf( "%s will be changed upon restarting.\n", var_name );
			var->latchedString = CopyStringInternal( value );
			var->modified = qtrue;
			return var;
		}
	} else {
		if ( var->latchedString ) {
			free( var->latchedString );
			var->latchedString = NULL;
		}
	}

	if ( !strcmp( value, var->string ) ) {
		return var;
	}

	var->modified = qtrue;
	var->modificationCount++;

	free( var->string );

	var->string = CopyStringInternal( value );
	var->value = atof( var->string );
	var->integer = atoi( var->string );

	return var;
}

/* ---- Cvar_Set  0x00439910 ---- */
void Cvar_Set( const char *var_name, const char *value ) {
	Cvar_Set2( var_name, value, qtrue );
}

/* ---- Cvar_VMSet  0x00439920 ---- */
void Cvar_VMSet( const char *value, vmCvar_t *vmCvar ) {
	Cvar_Set2( cvar_indexes[vmCvar->handle].name, value, qtrue );
	Cvar_Update( vmCvar );
}

/* ---- Cvar_SetLatched  0x00439950 ---- */
void Cvar_SetLatched( const char *var_name, const char *value ) {
	Cvar_Set2( var_name, value, qfalse );
}

/* ---- Cvar_SetValue  0x00439960 ---- */
void Cvar_SetValue( const char *var_name, float value ) {
	char val[32];

	if ( value == (int)value ) {
		Com_sprintf( val, sizeof( val ), "%i",(int)value );
	} else {
		Com_sprintf( val, sizeof( val ), "%f",value );
	}
	Cvar_Set( var_name, val );
}

/* ---- Cvar_Reset  0x004399F0 ---- */
void Cvar_Reset( const char *var_name ) {
	Cvar_Set2( var_name, NULL, qfalse );
}

/* ---- Cvar_Register  0x00439A00 ---- */
void    Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	cvar_t  *cv;

	cv = Cvar_Get( varName, defaultValue, flags );
	if ( !vmCvar ) {
		return;
	}
	vmCvar->handle = cv - cvar_indexes;
	vmCvar->modificationCount = -1;
	Cvar_Update( vmCvar );
}

/* ---- Cvar_Update  0x00439A40 ---- */
void    Cvar_Update( vmCvar_t *vmCvar ) {
	cvar_t  *cv = NULL;

	if ( (unsigned)vmCvar->handle >= cvar_numIndexes ) {
		Com_Error( ERR_DROP, "\x15" "Cvar_Update: handle out of range" );
	}

	cv = cvar_indexes + vmCvar->handle;

	if ( cv->modificationCount == vmCvar->modificationCount ) {
		return;
	}
	if ( !cv->string ) {
		return;
	}
	vmCvar->modificationCount = cv->modificationCount;
	if ( strlen( cv->string ) + 1 > MAX_CVAR_VALUE_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Cvar_Update: src %s length %d exceeds MAX_CVAR_VALUE_STRING",
				   cv->string,
				   strlen( cv->string ) );
	}
	Q_strncpyz( vmCvar->string, cv->string, MAX_CVAR_VALUE_STRING );

	vmCvar->value = cv->value;
	vmCvar->integer = cv->integer;
}

/* ---- Cvar_Init  0x00439AF0 ---- */
void Cvar_Init( void ) {
	cvar_cheats = Cvar_Get( "sv_cheats", "0", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_AddCommands();
}
