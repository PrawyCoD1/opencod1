/*
 * botlib library variables -- reconstructed from CoD 1.1 (CoDMP.exe).
 *
 * Retail bodies at 0x0043D1B0-0x0043D51F; see l_libvar_mp.c.
 *
 * libvar_t is 24 bytes:
 *   sizeof   memset( v, 0, 0x18 ) in LibVarAlloc at 0x0043D26E, with the name
 *            string written at v + 24 immediately afterwards.
 *   +4       LibVarGetString returns it (0x0043D390).
 *   +12      LibVarChanged reads it, LibVarSetNotModified clears it
 *            (0x0043D4F0 / 0x0043D500).
 *   +16      LibVarGetValue returns it as a float (0x0043D3B0).
 *   +20      the list link walked by LibVarDeAllocAll (0x0043D2E0).
 * That leaves +0 (name, written first in LibVarAlloc) and +8, which is never
 * touched by any surviving function -- it is RTCW's `flags`, kept for layout.
 */

#ifndef __L_LIBVAR_H__
#define __L_LIBVAR_H__

typedef struct libvar_s {
	char *name;                 /* +0  */
	char *string;               /* +4  */
	int flags;                  /* +8  -- never read by any retail function */
	qboolean modified;          /* +12 */
	float value;                /* +16 */
	struct libvar_s *next;      /* +20 */
} libvar_t;                     /* 24 bytes */

COD1_ASSERT_SIZE( libvar_t, 24 );

extern libvar_t *libvarlist;    /* 0x01646424 */

float LibVarStringValue( char *string );
libvar_t *LibVarAlloc( char *var_name );
void LibVarDeAlloc( libvar_t *v );
void LibVarDeAllocAll( void );
libvar_t *LibVarGet( char *var_name );
char *LibVarGetString( char *var_name );
float LibVarGetValue( char *var_name );
libvar_t *LibVar( char *var_name, char *value );
char *LibVarString( char *var_name, char *value );
float LibVarValue( char *var_name, char *value );
void LibVarSet( char *var_name, char *value );
qboolean LibVarChanged( char *var_name );
void LibVarSetNotModified( char *var_name );

#endif /* __L_LIBVAR_H__ */
