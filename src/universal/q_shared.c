/*
 * universal/q_shared.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/q_shared.c
 *
 * Retail range 0x0044A530-0x0044B92.
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

typedef union {
	byte b[8];
	struct { byte b0, b1, b2, b3, b4, b5, b6, b7; };
} qint64;

/*
 * The colour run, .rdata 0x20055480..0x200555AF in the game DLL and
 * 0x30060734..0x30060863 in the cgame DLL -- nineteen vec4_t, sixteen bytes
 * apart, byte-identical in both images.  black, red, green, blue, yellow,
 * magenta, cyan and white carry their Q3/RTCW values and the .75/.5/.25 grey
 * triple is RTCW's; colorLtGreen, colorLtYellow, colorMdYellow, colorLtCyan,
 * colorMdCyan, colorDkCyan, colorOrange and colorLtOrange are the coduomp
 * reconstruction's names (it lists exactly these nineteen in exactly this
 * order with exactly these values), not recovered symbols; colorMdCyan also
 * follows from its slot in the Lt/Md/Dk run.
 *
 * These are const: the run is in .rdata, not .data.  Q3's eight-entry
 * g_color_table follows it, four bytes of alignment padding later.
 */
const vec4_t colorBlack     = { 0.00f, 0.00f, 0.00f, 1.0f };    /* 0x20055480 */
const vec4_t colorRed       = { 1.00f, 0.00f, 0.00f, 1.0f };    /* 0x20055490 */
const vec4_t colorGreen     = { 0.00f, 1.00f, 0.00f, 1.0f };    /* 0x200554A0 */
const vec4_t colorLtGreen   = { 0.00f, 0.70f, 0.00f, 1.0f };    /* 0x200554B0 */
const vec4_t colorBlue      = { 0.00f, 0.00f, 1.00f, 1.0f };    /* 0x200554C0 */
const vec4_t colorYellow    = { 1.00f, 1.00f, 0.00f, 1.0f };    /* 0x200554D0 */
const vec4_t colorLtYellow  = { 0.75f, 0.75f, 0.00f, 1.0f };    /* 0x200554E0 */
const vec4_t colorMdYellow  = { 0.50f, 0.50f, 0.00f, 1.0f };    /* 0x200554F0 */
const vec4_t colorMagenta   = { 1.00f, 0.00f, 1.00f, 1.0f };    /* 0x20055500 */
const vec4_t colorCyan      = { 0.00f, 1.00f, 1.00f, 1.0f };    /* 0x20055510 */
const vec4_t colorLtCyan    = { 0.00f, 0.75f, 0.75f, 1.0f };    /* 0x20055520 */
const vec4_t colorMdCyan    = { 0.00f, 0.50f, 0.50f, 1.0f };    /* 0x20055530 */
const vec4_t colorDkCyan    = { 0.00f, 0.25f, 0.25f, 1.0f };    /* 0x20055540 */
const vec4_t colorWhite     = { 1.00f, 1.00f, 1.00f, 1.0f };    /* 0x20055550 */
const vec4_t colorLtGrey    = { 0.75f, 0.75f, 0.75f, 1.0f };    /* 0x20055560 */
const vec4_t colorMdGrey    = { 0.50f, 0.50f, 0.50f, 1.0f };    /* 0x20055570 */
const vec4_t colorDkGrey    = { 0.25f, 0.25f, 0.25f, 1.0f };    /* 0x20055580 */
const vec4_t colorOrange    = { 1.00f, 0.70f, 0.00f, 1.0f };    /* 0x20055590 */
const vec4_t colorLtOrange  = { 0.75f, 0.525f, 0.00f, 1.0f };   /* 0x200555A0 */

/*
 * g_color_table -- the eight colours the ^N escape selects, indexed by
 * ColorIndex().  cgame DLL 0x30060868, game DLL 0x200555B0, engine
 * 0x00541950; four bytes after colorLtOrange's last float (the 19-entry run
 * ends at 0x30060864 and an alignment pad separates the two objects).  All 32
 * floats are exactly 0 or 1 and the order is Q3's
 * black/red/green/yellow/blue/cyan/magenta/white.  const because it is in
 * .rdata here, where Q3 declares it writable.
 */
const vec4_t g_color_table[8] = {                               /* 0x200555B0 */
	{ 0.00f, 0.00f, 0.00f, 1.0f },      /* COLOR_BLACK   '0' */
	{ 1.00f, 0.00f, 0.00f, 1.0f },      /* COLOR_RED     '1' */
	{ 0.00f, 1.00f, 0.00f, 1.0f },      /* COLOR_GREEN   '2' */
	{ 1.00f, 1.00f, 0.00f, 1.0f },      /* COLOR_YELLOW  '3' */
	{ 0.00f, 0.00f, 1.00f, 1.0f },      /* COLOR_BLUE    '4' */
	{ 0.00f, 1.00f, 1.00f, 1.0f },      /* COLOR_CYAN    '5' */
	{ 1.00f, 0.00f, 1.00f, 1.0f },      /* COLOR_MAGENTA '6' */
	{ 1.00f, 1.00f, 1.00f, 1.0f },      /* COLOR_WHITE   '7' */
};

/* Com_Clamp  0x0044A530 */
float Com_Clamp( float min, float max, float value ) {
	if ( value < min ) {
		return min;
	}
	if ( value > max ) {
		return max;
	}
	return value;
}

/* ---- Com_SkipPath  0x0044A560 ---- */
char *Com_SkipPath( char *pathname ) {
	char    *last;

	last = pathname;
	while ( *pathname )
	{
		if ( *pathname == '/' ) {
			last = pathname + 1;
		}
		pathname++;
	}
	return last;
}

/* ---- Com_StripExtension  0x0044A580 ---- */
void Com_StripExtension( const char *in, char *out ) {
	while ( *in && *in != '.' ) {
		*out++ = *in++;
	}
	*out = 0;
}

/* ---- Com_StripFilename  0x0044A5A0 ---- */
void Com_StripFilename( char *in, char *out ) {
	char *end;
	Q_strncpyz( out, in, strlen( in ) );
	end = Com_SkipPath( out );
	*end = 0;
}

/* ---- Com_DefaultExtension  0x0044A5F0 ---- */
void Com_DefaultExtension( char *path, int maxSize, const char *extension ) {
	char oldPath[MAX_QPATH];
	char    *src;

	src = path + strlen( path ) - 1;

	while ( *src != '/' && src != path ) {
		if ( *src == '.' ) {
			return;
		}
		src--;
	}

	Q_strncpyz( oldPath, path, sizeof( oldPath ) );
	Com_sprintf( path, maxSize, "%s%s", oldPath, extension );
}

/* ---- Com_BitCheck  0x0044A680 ---- */
qboolean Com_BitCheck( const int array[], int bitNum ) {
	return ( ( array[bitNum >> 5] & ( 1 << ( bitNum & 31 ) ) ) != 0 );
}

/* ---- Com_BitSet  0x0044A6A0 ---- */
void Com_BitSet( int array[], int bitNum ) {
	array[bitNum >> 5] |= ( 1 << ( bitNum & 31 ) );
}

/* ---- Com_BitClear  0x0044A6C0 ---- */
void Com_BitClear( int array[], int bitNum ) {
	array[bitNum >> 5] &= ~( 1 << ( bitNum & 31 ) );
}
static short ( *_BigShort )( short l );
static short ( *_LittleShort )( short l );
static int ( *_BigLong )( int l );
static int ( *_LittleLong )( int l );
static qint64 ( *_BigLong64 )( qint64 l );
static qint64 ( *_LittleLong64 )( qint64 l );
static float ( *_BigFloat )( float l );
static float ( *_LittleFloat )( float l );

/* BigShort  0x0044A740 */
short   BigShort( short l ) {return _BigShort( l );}
short   LittleShort( short l ) {return _LittleShort( l );}
int     BigLong( int l ) {return _BigLong( l );}
int     LittleLong( int l ) {return _LittleLong( l );}
qint64  BigLong64( qint64 l ) {return _BigLong64( l );}
qint64  LittleLong64( qint64 l ) {return _LittleLong64( l );}
float   BigFloat( float l ) {return _BigFloat( l );}
float   LittleFloat( float l ) {return _LittleFloat( l );}

/* ShortSwap  0x0044A750 */
short   ShortSwap( short l ) {
	byte b1,b2;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;

	return ( b1 << 8 ) + b2;
}

/* ShortNoSwap  0x0044A770 */
short   ShortNoSwap( short l ) {
	return l;
}

/* LongSwap  0x0044A780 */
int    LongSwap( int l ) {
	byte b1,b2,b3,b4;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;
	b3 = ( l >> 16 ) & 255;
	b4 = ( l >> 24 ) & 255;

	return ( (int)b1 << 24 ) + ( (int)b2 << 16 ) + ( (int)b3 << 8 ) + b4;
}

/* LongNoSwap  0x0044A7B0 */
int LongNoSwap( int l ) {
	return l;
}

/* Long64Swap  0x0044A7C0 */
qint64 Long64Swap( qint64 ll ) {
	qint64 result;

	result.b0 = ll.b7;
	result.b1 = ll.b6;
	result.b2 = ll.b5;
	result.b3 = ll.b4;
	result.b4 = ll.b3;
	result.b5 = ll.b2;
	result.b6 = ll.b1;
	result.b7 = ll.b0;

	return result;
}

/* Long64NoSwap  0x0044A820 */
qint64 Long64NoSwap( qint64 ll ) {
	return ll;
}

/* FloatSwap  0x0044A830 */
float FloatSwap( float f ) {
	union
	{
		float f;
		byte b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

/* FloatNoSwap  0x0044A860 */
float FloatNoSwap( float f ) {
	return f;
}

/* ---- Swap_Init  0x0044A870 ---- */
void Swap_Init( void ) {
	byte swaptest[2] = {1,0};

	if ( *(short *)swaptest == 1 ) {
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigLong64 = Long64Swap;
		_LittleLong64 = Long64NoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	} else
	{
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigLong64 = Long64NoSwap;
		_LittleLong64 = Long64Swap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}
}

/* Q_isprint  0x0044A930 */
int Q_isprint( int c ) {
	if ( c >= 0x20 && c <= 0x7E ) {
		return ( 1 );
	}
	return ( 0 );
}

/* Q_islower  0x0044A950 */
int Q_islower( int c ) {
	if ( c >= 'a' && c <= 'z' ) {
		return ( 1 );
	}
	return ( 0 );
}

/* Q_isupper  0x0044A970 */
int Q_isupper( int c ) {
	if ( c >= 'A' && c <= 'Z' ) {
		return ( 1 );
	}
	return ( 0 );
}

/* Q_isalpha  0x0044A990 */
int Q_isalpha( int c ) {
	if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) {
		return ( 1 );
	}
	return ( 0 );
}

/* Q_isnumeric  0x0044A9B0 */
int Q_isnumeric( int c ) {
	if ( c >= '0' && c <= '9' ) {
		return ( 1 );
	}
	return ( 0 );
}

/* Q_isalphanumeric  0x0044A9D0 */
int Q_isalphanumeric( int c ) {
	if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ) {
		return ( 1 );
	}
	return ( 0 );
}

/* Q_isnamechar  0x0044AA00 */
int Q_isnamechar( int c ) {
	if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' )
		 || c == '_' || c == '-' ) {
		return ( 1 );
	}
	return ( 0 );
}

/* Q_strrchr  0x0044AA40 */
char* Q_strrchr( const char* string, int c ) {
	char cc = c;
	char *s;
	char *sp = (char *)0;

	s = (char*)string;

	while ( *s )
	{
		if ( *s == cc ) {
			sp = s;
		}
		s++;
	}
	if ( cc == 0 ) {
		sp = s;
	}

	return sp;
}

/* ---- Q_strncpyz  0x0044AA70 ----
   RTCW's two Com_Error guards (NULL src, destsize < 1) are NOT in retail 1.1:
   the function is 20 bytes in all four binaries (exe 0x0044AA70, game
   0x2003E630, cgame 0x3003E8D0, ui 0x40005D40) -- strncpy and the
   terminator, nothing else. */
void Q_strncpyz( char *dest, const char *src, int destsize ) {
	strncpy( dest, src, destsize - 1 );
	dest[destsize - 1] = 0;
}

/* Q_stricmpn  0x0044AA90 */
int Q_stricmpn( const char *s1, const char *s2, int n ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;
		}

		if ( c1 != c2 ) {
			if ( c1 >= 'a' && c1 <= 'z' ) {
				c1 -= ( 'a' - 'A' );
			}
			if ( c2 >= 'a' && c2 <= 'z' ) {
				c2 -= ( 'a' - 'A' );
			}
			if ( c1 != c2 ) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while ( c1 );

	return 0;
}

/* Q_strncmp  0x0044AAF0 */
int Q_strncmp( const char *s1, const char *s2, int n ) {
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;
		}

		if ( c1 != c2 ) {
			return c1 < c2 ? -1 : 1;
		}
	} while ( c1 );

	return 0;
}

/* Q_stricmp  0x0044AB30 */
int Q_stricmp( const char *s1, const char *s2 ) {
	return ( s1 && s2 ) ? Q_stricmpn( s1, s2, 99999 ) : -1;
}

/* Q_strlwr  0x0044AB50 */
char *Q_strlwr( char *s1 ) {
	char    *s;

	s = s1;
	while ( *s ) {
		*s = tolower( *s );
		s++;
	}
	return s1;
}

/* Q_strupr  0x0044AB80 */
char *Q_strupr( char *s1 ) {
	char    *s;

	s = s1;
	while ( *s ) {
		*s = toupper( *s );
		s++;
	}
	return s1;
}

/* Q_strcat  0x0044ABB0 */
void Q_strcat( char *dest, int size, const char *src ) {
	int l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "\x15" "Q_strcat: already overflowed" );
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}

/* Q_CleanStr  0x0044AC00 */
char *Q_CleanStr( char *string ) {
	char*   d;
	char*   s;
	int c;

	s = string;
	d = string;
	while ( ( c = *s ) != 0 ) {
		if ( Q_IsColorString( s ) ) {
			s++;
		} else if ( c >= 0x20 && c <= 0x7E )   {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

/* ---- Q_CleanCharacter  0x0044AC50 ---- */
char Q_CleanCharacter( char c ) {
	if ( c == (char)0x92 ) {
		return '\'';
	}
	if ( (unsigned char)c > 0x7F ) {
		return '.';
	}
	return c;
}

/* ---- Com_sprintf  0x0044AC60 ---- */
int QDECL Com_sprintf( char *dest, int size, const char *fmt, ... ) {
	int len;
	va_list argptr;

	va_start( argptr,fmt );
	len = Q_vsnprintf( dest, size, fmt, argptr );
	va_end( argptr );

	dest[size - 1] = 0;

	return len;
}

/* Q_strncasecmp  0x0044AC80 */
int Q_strncasecmp( char *s1, char *s2, int n ) {
	int c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;

		}
		if ( c1 != c2 ) {
			if ( c1 >= 'a' && c1 <= 'z' ) {
				c1 -= ( 'a' - 'A' );
			}
			if ( c2 >= 'a' && c2 <= 'z' ) {
				c2 -= ( 'a' - 'A' );
			}
			if ( c1 != c2 ) {
				return -1;
			}
		}
	} while ( c1 );

	return 0;
}

/* Q_strcasecmp  0x0044ACD0 */
int Q_strcasecmp( char *s1, char *s2 ) {
	return Q_strncasecmp( s1, s2, 99999 );
}
/* ---- va  0x0044ACE0 ---- */
char    * QDECL va( char *format, ... ) {
	va_list argptr;
	#define MAX_VA_STRING   32000
	static char temp_buffer[MAX_VA_STRING];
	static char string[MAX_VA_STRING];
	static int index = 0;
	char    *buf;
	int len;

	va_start( argptr, format );
	vsprintf( temp_buffer, format,argptr );
	va_end( argptr );

	if ( ( len = strlen( temp_buffer ) ) >= MAX_VA_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Attempted to overrun string in call to va()\n" );
	}

	if ( len + index >= MAX_VA_STRING - 1 ) {
		index = 0;
	}

	buf = &string[index];
	memcpy( buf, temp_buffer, len + 1 );

	index += len + 1;

	return buf;
}

/* ---- tv  0x0044AD70 ---- */
float   *tv( float x, float y, float z ) {
	static int index;
	static vec3_t vecs[8];
	float   *v;

	v = vecs[index];
	index = ( index + 1 ) & 7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/* ---- Info_ValueForKey  0x0044ADA0 ---- */
char *Info_ValueForKey( const char *s, const char *key ) {
	char pkey[BIG_INFO_KEY];
	static char value[2][BIG_INFO_VALUE];
	static int valueindex = 0;
	char    *o;

	if ( !s || !key ) {
		return "";
	}

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_ValueForKey: oversize infostring" );
	}

	valueindex ^= 1;
	if ( *s == '\\' ) {
		s++;
	}
	while ( 1 )
	{
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return "";
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while ( *s != '\\' && *s )
		{
			*o++ = *s++;
		}
		*o = 0;

		if ( !Q_stricmp( key, pkey ) ) {
			return value[valueindex];
		}

		if ( !*s ) {
			break;
		}
		s++;
	}

	return "";
}

/* ---- Info_NextPair  0x0044AEA0 ---- */
void Info_NextPair( const char **head, char *key, char *value ) {
	char    *o;
	const char  *s;

	s = *head;

	if ( *s == '\\' ) {
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}

/* ---- Info_RemoveKey  0x0044AEF0 ---- */
void Info_RemoveKey( char *s, const char *key ) {
	char    *start;
	char pkey[MAX_INFO_KEY];
	char value[MAX_INFO_VALUE];
	char    *o;

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_RemoveKey: oversize infostring" );
	}

	if ( strchr( key, '\\' ) ) {
		return;
	}

	while ( 1 )
	{
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if ( !strcmp( key, pkey ) ) {
			strcpy( start, s );
			return;
		}

		if ( !*s ) {
			return;
		}
	}

}

/* ---- Info_RemoveKey_Big  0x0044B000 ---- */
void Info_RemoveKey_Big( char *s, const char *key ) {
	char    *start;
	char pkey[BIG_INFO_KEY];
	char value[BIG_INFO_VALUE];
	char    *o;

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_RemoveKey_Big: oversize infostring" );
	}

	if ( strchr( key, '\\' ) ) {
		return;
	}

	while ( 1 )
	{
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if ( !strcmp( key, pkey ) ) {
			strcpy( start, s );
			return;
		}

		if ( !*s ) {
			return;
		}
	}

}

/* ---- Info_Validate  0x0044B120 ---- */
qboolean Info_Validate( const char *s ) {
	if ( strchr( s, '\"' ) ) {
		return qfalse;
	}
	if ( strchr( s, ';' ) ) {
		return qfalse;
	}
	return qtrue;
}

/* ---- Info_SetValueForKey  0x0044B150 ---- */
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char newi[MAX_INFO_STRING];
	char cleanValue[MAX_INFO_STRING];
	int i, j;

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_SetValueForKey: oversize infostring" );
	}

	j = 0;
	for ( i = 0 ; i < MAX_INFO_STRING - 1 ; i++ ) {
		char c = value[i];
		if ( !c ) {
			break;
		}
		if ( c != '\\' && c != ';' && c != '\"' ) {
			cleanValue[j++] = c;
		}
	}
	cleanValue[j] = 0;

	if ( strchr( key, '\\' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \\\nkey: '%s'\nvalue: '%s'",
				   key, value );
	}
	if ( strchr( key, ';' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a semicolon\nkey: '%s'\nvalue: '%s'",
				   key, value );
	}
	if ( strchr( key, '\"' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \"\nkey: '%s'\nvalue: '%s'",
				   key, value );
	}

	Info_RemoveKey( s, key );
	if ( !cleanValue[0] ) {
		return;
	}

	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, cleanValue );

	if ( strlen( newi ) + strlen( s ) > MAX_INFO_STRING ) {
		Com_Error( ERR_DROP,
				   "\x15" "Info string length exceeded\nkey: '%s'\nvalue: '%s'\nInfo string:\n%s\n",
				   key, value, s );
	}

	strcat( s, newi );
}

/* ---- Info_SetValueForKey_Big  0x0044B300 ---- */
void Info_SetValueForKey_Big( char *s, const char *key, const char *value ) {
	char newi[BIG_INFO_STRING];
	char cleanValue[BIG_INFO_STRING];
	int i, j;

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_SetValueForKey: oversize infostring" );
	}

	j = 0;
	for ( i = 0 ; i < BIG_INFO_STRING - 1 ; i++ ) {
		char c = value[i];
		if ( !c ) {
			break;
		}
		if ( c != '\\' && c != ';' && c != '\"' ) {
			cleanValue[j++] = c;
		}
	}
	cleanValue[j] = 0;

	if ( strchr( key, '\\' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \\\nkey: '%s'\nvalue: '%s'",
				   key, value );
	}
	if ( strchr( key, ';' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a semicolon\nkey: '%s'\nvalue: '%s'",
				   key, value );
	}
	if ( strchr( key, '\"' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \"\nkey: '%s'\nvalue: '%s'",
				   key, value );
	}

	Info_RemoveKey_Big( s, key );
	if ( !cleanValue[0] ) {
		return;
	}

	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, cleanValue );

	if ( strlen( newi ) + strlen( s ) > BIG_INFO_STRING ) {
		Com_Error( ERR_DROP,
				   "\x15" "BIG Info string length exceeded\nkey: '%s'\nvalue: '%s'\nInfo string:\n%s\n",
				   key, value, s );
	}

	strcat( s, newi );
}

typedef enum {
	CS_FIELD_CUSTOM     = 0,
	CS_FIELD_STRING1024 = 1,
	CS_FIELD_STRING64   = 2,
	CS_FIELD_STRING256  = 3,
	CS_FIELD_INT        = 4,
	CS_FIELD_QBOOLEAN   = 5,
	CS_FIELD_FLOAT      = 6,
	CS_FIELD_MILLISECONDS = 7,
} csFieldType_t;

typedef struct {
	const char  *name;
	int offset;
	int type;
} csField_t;

/*
 * ---- ParseConfigStringToStruct  0x0044B4B0 ---- (game DLL 0x2003F070,
 * cgame DLL 0x3003F310)
 *
 * __usercall in both DLLs: `fields` rides in edx and everything else is on the
 * stack, at arg_0 base, arg_4 numFields, arg_8 s, arg_C numCustomTypes,
 * arg_10 the type>=8 handler, arg_14 the string copier.  Stack args keep source
 * order, so the four leading parameters are (fields, base, numFields, s); the
 * register-passed one is the leftmost.
 */
qboolean ParseConfigStringToStruct( const csField_t *fields, void *structBase, int numFields,
									const char *s, int numCustomTypes,
									qboolean ( *customHandler )( void *base, const char *value, int type ),
									void ( *vectorHandler )( void *dest, const char *value ) ) {
	int i;
	char *value;
	byte *base = (byte *)structBase;

	for ( i = 0; i < numFields; i++ ) {
		value = Info_ValueForKey( s, fields[i].name );
		if ( !*value ) {
			continue;
		}

		if ( fields[i].type >= 8 ) {
			if ( numCustomTypes <= 0 || fields[i].type >= numCustomTypes ) {
				Com_Error( ERR_DROP, "\x15" "Bad field type %i\n", fields[i].type );
			} else if ( !customHandler( structBase, value, fields[i].type ) ) {
				return qfalse;
			}
			continue;
		}

		switch ( fields[i].type ) {
		case CS_FIELD_CUSTOM:
			vectorHandler( base + fields[i].offset, value );
			break;
		case CS_FIELD_STRING1024:
			Q_strncpyz( (char *)( base + fields[i].offset ), value, 1024 );
			break;
		case CS_FIELD_STRING64:
			Q_strncpyz( (char *)( base + fields[i].offset ), value, 64 );
			break;
		case CS_FIELD_STRING256:
			Q_strncpyz( (char *)( base + fields[i].offset ), value, 256 );
			break;
		case CS_FIELD_INT:
			*(int *)( base + fields[i].offset ) = atoi( value );
			break;
		case CS_FIELD_QBOOLEAN:
			*(int *)( base + fields[i].offset ) = ( atoi( value ) != 0 );
			break;
		case CS_FIELD_FLOAT:
			*(float *)( base + fields[i].offset ) = (float)atof( value );
			break;
		case CS_FIELD_MILLISECONDS:
			*(int *)( base + fields[i].offset ) = (int)( (float)atof( value ) * 1000.0f );
			break;
		default:
			break;
		}
	}

	/* The epilogue really is `cmp eax, ecx; setz` on the loop counter against
	   numFields, not a constant qtrue -- it differs for numFields < 0. */
	return i == numFields;
}

/* RTCW's q_math.c Q_fabs.  No standalone copy survives in either DLL: retail
 * carries the 0x7FFFFFFF mask inline (GetLeanFraction 0x2003F1FD,
 * AddLeanToPosition 0x2003F262), so __inline under /Ob1 as bg_local.h does. */
static __inline float Q_fabs( float f ) {
	int tmp = *( int * ) &f;
	tmp &= 0x7FFFFFFF;
	return *( float * ) &tmp;
}

/* ---- GetLeanFraction  0x0044B630 ---- (game DLL 0x2003F1F0) */
float GetLeanFraction( float f ) {
	return ( 2.0f - Q_fabs( f ) ) * f;
}

/* ---- UnGetLeanFraction  0x0044B660 ---- */
float UnGetLeanFraction( float f ) {
	return 1.0f - (float)sqrt( 1.0f - f );
}

/* ---- AddLeanToPosition  0x0044B680 ---- (game DLL 0x2003F240)
 * Retail (LTCG) inlines GetLeanFraction and AngleVectors( angles, NULL, right,
 * NULL ): with forward/up folded away, 0x2003F2A1..0x2003F365 is AngleVectors'
 * three SinCos_float blocks and its right[] arithmetic, the same bytes as
 * 0x200143CD..0x20014416 in the standalone copy (the pitch block loads a
 * stored 0.0f; the `sr*sp` temp and the `fmul -1.0` of the RTCW text are
 * there).  The unoptimized Linux UO game keeps both calls. */
void AddLeanToPosition( float *pos, float yaw, float leanFrac, float leanDist, float scale ) {
	float frac;
	vec3_t angles;
	vec3_t right;

	if ( leanFrac == 0.0f ) {
		return;
	}

	frac = GetLeanFraction( leanFrac );

	angles[0] = 0;
	angles[1] = yaw;
	angles[2] = frac * leanDist;
	AngleVectors( angles, NULL, right, NULL );

	frac = frac * scale;

	pos[0] += right[0] * frac;
	pos[1] += right[1] * frac;
	pos[2] += right[2] * frac;
}

/* OrientationPosToWorldPos  0x0044B7F0 */
void OrientationPosToWorldPos( const float *orient, const float *pos, float *out ) {
	out[0] = orient[0] + orient[3] * pos[0] + orient[6] * pos[1] + orient[9] * pos[2];
	out[1] = orient[1] + orient[4] * pos[0] + orient[7] * pos[1] + orient[10] * pos[2];
	out[2] = orient[2] + orient[5] * pos[0] + orient[8] * pos[1] + orient[11] * pos[2];
}

/* OrientationDirToWorldDir  0x0044B840 */
void OrientationDirToWorldDir( const float *orient, const float *dir, float *out ) {
	out[0] = orient[3] * dir[0] + orient[6] * dir[1] + orient[9] * dir[2];
	out[1] = orient[4] * dir[0] + orient[7] * dir[1] + orient[10] * dir[2];
	out[2] = orient[5] * dir[0] + orient[8] * dir[1] + orient[11] * dir[2];
}

/* OrientationPosFromWorldPos  0x0044B890 */
void OrientationPosFromWorldPos( const float *orient, const float *worldPos, float *out ) {
	float d[3];

	d[0] = worldPos[0] - orient[0];
	d[1] = worldPos[1] - orient[1];
	d[2] = worldPos[2] - orient[2];

	out[0] = d[0] * orient[3] + d[1] * orient[4] + d[2] * orient[5];
	out[1] = d[0] * orient[6] + d[1] * orient[7] + d[2] * orient[8];
	out[2] = d[0] * orient[9] + d[1] * orient[10] + d[2] * orient[11];
}

/* OrientationDirFromWorldDir  0x0044B8E0 */
void OrientationDirFromWorldDir( const float *orient, const float *worldDir, float *out ) {
	out[0] = worldDir[0] * orient[3] + worldDir[1] * orient[4] + worldDir[2] * orient[5];
	out[1] = worldDir[0] * orient[6] + worldDir[1] * orient[7] + worldDir[2] * orient[8];
	out[2] = worldDir[0] * orient[9] + worldDir[1] * orient[10] + worldDir[2] * orient[11];
}

/* ---- Q_vsnprintf  0x00528B64 ---- */
int Q_vsnprintf( char *dest, size_t size, const char *fmt, va_list ap ) {
	int len;

	if ( !size ) {
		return 0;
	}

	len = _vsnprintf( dest, size, fmt, ap );
	dest[size - 1] = 0;

	return len < 0 ? (int) size : len;
}
