/*
 *
 * Retail range 0x0043D1B0-0x0043D51F, 13 functions.
 *
 * Every function in this unit is Q3/RTCW botlib l_libvar.c
 * instruction-for-instruction; there are no CoD divergences.  libvar_t's layout:
 * memset(v, 0, 0x18) in LibVarAlloc; LibVarGetString reads +4, LibVarChanged
 * +12, LibVarGetValue +16, and LibVarDeAllocAll walks the list link at +20.
 *
 * The retail unit is almost certainly plain l_libvar.c; the _mp suffix matches
 * the other botlib units (l_log_mp.c, l_script_mp.c).
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"
#include "l_memory.h"
#include "l_libvar.h"

libvar_t *libvarlist = NULL;                 /* 0x01646424 */

/*
=================
LibVarStringValue          0x0043D1B0

Hand-rolled float parser: an optional leading '-', a digit run, an optional '.'
and a fractional run.  Anything else terminates.  Deliberately not strtod --
botlib predates any locale handling here.
=================
*/
float LibVarStringValue( char *string ) {
	int dotfound = 0;
	float value = 0;

	while ( *string ) {
		if ( *string < '0' || *string > '9' ) {
			if ( dotfound || *string != '.' ) {
				return 0;
			}
			dotfound = 10;
			string++;
		}
		if ( dotfound ) {
			value = value + (float) ( *string - '0' ) / (float) dotfound;
			dotfound *= 10;
		} else {
			value = value * 10.0 + (float) ( *string - '0' );
		}
		string++;
	}
	return value;
}

/*
=================
LibVarAlloc                0x0043D240

The name string is appended to the allocation rather than allocated separately:
GetClearedMemory( sizeof(libvar_t) + strlen(var_name) + 1 ), name = v + 24.
=================
*/
libvar_t *LibVarAlloc( char *var_name ) {
	libvar_t *v;

	v = (libvar_t *) GetMemory( sizeof( libvar_t ) + strlen( var_name ) + 1 );
	memset( v, 0, sizeof( libvar_t ) );
	v->name = (char *) v + sizeof( libvar_t );
	strcpy( v->name, var_name );

	v->next = libvarlist;
	libvarlist = v;

	return v;
}

/*
=================
LibVarDeAlloc              0x0043D2A0
=================
*/
void LibVarDeAlloc( libvar_t *v ) {
	if ( v->string ) {
		FreeMemory( v->string );
	}
	FreeMemory( v );
}

/*
=================
LibVarDeAllocAll           0x0043D2E0
=================
*/
void LibVarDeAllocAll( void ) {
	libvar_t *v;

	for ( v = libvarlist; v; v = libvarlist ) {
		libvarlist = libvarlist->next;
		LibVarDeAlloc( v );
	}
	libvarlist = NULL;
}

/*
=================
LibVarGet                  0x0043D350
=================
*/
libvar_t *LibVarGet( char *var_name ) {
	libvar_t *v;

	for ( v = libvarlist; v; v = v->next ) {
		if ( !Q_stricmp( v->name, var_name ) ) {
			return v;
		}
	}
	return NULL;
}

/*
=================
LibVarGetString            0x0043D390
=================
*/
char *LibVarGetString( char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		return v->string;
	}
	return "";
}

/*
=================
LibVarGetValue             0x0043D3B0
=================
*/
float LibVarGetValue( char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		return v->value;
	}
	return 0;
}

/*
=================
LibVar                     0x0043D3D0

Get-or-create.  The default value is only applied on creation.
=================
*/
libvar_t *LibVar( char *var_name, char *value ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		return v;
	}

	/* create new variable */
	v = LibVarAlloc( var_name );
	v->string = (char *) GetMemory( strlen( value ) + 1 );
	strcpy( v->string, value );
	v->value = LibVarStringValue( v->string );
	v->modified = qtrue;

	return v;
}

/*
=================
LibVarString               0x0043D440
=================
*/
char *LibVarString( char *var_name, char *value ) {
	return LibVar( var_name, value )->string;
}

/*
=================
LibVarValue                0x0043D450
=================
*/
float LibVarValue( char *var_name, char *value ) {
	return LibVar( var_name, value )->value;
}

/*
=================
LibVarSet                  0x0043D460
=================
*/
void LibVarSet( char *var_name, char *value ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		FreeMemory( v->string );
	} else {
		v = LibVarAlloc( var_name );
	}

	v->string = (char *) GetMemory( strlen( value ) + 1 );
	strcpy( v->string, value );
	v->value = LibVarStringValue( v->string );
	v->modified = qtrue;
}

/*
=================
LibVarChanged              0x0043D4F0
=================
*/
qboolean LibVarChanged( char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		return v->modified;
	}
	return qfalse;
}

/*
=================
LibVarSetNotModified       0x0043D500
=================
*/
void LibVarSetNotModified( char *var_name ) {
	libvar_t *v;

	v = LibVarGet( var_name );
	if ( v ) {
		v->modified = qfalse;
	}
}
