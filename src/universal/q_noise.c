/*
 * universal/q_noise.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/q_noise.c
 *
 * Retail range 0x004DD440-0x004DD91F.
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"

#include <stdlib.h>
#include <math.h>

#define NOISE_TABLE_SIZE    256
#define NOISE_SEED          1001

/* NoiseFloor  0x004DD91F */
static int NoiseFloor( float v ) {
	return (int)floor( (double)v );
}

static float noiseTable[NOISE_TABLE_SIZE];
static int noisePermute[NOISE_TABLE_SIZE];

/* ---- Com_NoiseInit  0x004DD490 ---- */
void Com_NoiseInit( void ) {
	int i;
	int r;

	srand( NOISE_SEED );

	for ( i = 0 ; i < NOISE_TABLE_SIZE ; i++ ) {
		r = rand();
		noiseTable[i] = ( r * ( 1.0f / 32767.0f ) ) + ( r * ( 1.0f / 32767.0f ) ) - 1.0f;
		noisePermute[i] = ( rand() << 8 ) / 0x8000;
	}
}

/* ---- GetNoiseValue  0x004DD440 ---- */
float GetNoiseValue( int x, int y, int z, int t ) {
	int i;

	i = noisePermute[ (byte)t ];
	i = noisePermute[ (byte)( z + i ) ];
	i = noisePermute[ (byte)( y + i ) ];
	i = noisePermute[ (byte)( x + i ) ];

	return noiseTable[i];
}

static float NoiseLerpCube( int ix, int iy, int iz, int it,
							float fx, float fy, float fz ) {
	float xc = 1.0f - fx;
	float yc = 1.0f - fy;
	float lz0, lz1;

	lz0 = fy * ( xc * GetNoiseValue( ix,     iy + 1, iz,     it )
			   + fx * GetNoiseValue( ix + 1, iy + 1, iz,     it ) )
		+ yc * ( xc * GetNoiseValue( ix,     iy,     iz,     it )
			   + fx * GetNoiseValue( ix + 1, iy,     iz,     it ) );

	lz1 = fy * ( xc * GetNoiseValue( ix,     iy + 1, iz + 1, it )
			   + fx * GetNoiseValue( ix + 1, iy + 1, iz + 1, it ) )
		+ yc * ( xc * GetNoiseValue( ix,     iy,     iz + 1, it )
			   + fx * GetNoiseValue( ix + 1, iy,     iz + 1, it ) );

	return fz * lz1 + ( 1.0f - fz ) * lz0;
}

/* ---- Com_NoiseGet4f  0x004DD4F0 ---- */
float Com_NoiseGet4f( float x, float y, float z, float t ) {
	int ix, iy, iz, it;
	float fx, fy, fz, ft;
	float r0, r1;

	ix = NoiseFloor( x );   fx = x - (float)ix;
	iy = NoiseFloor( y );   fy = y - (float)iy;
	iz = NoiseFloor( z );   fz = z - (float)iz;
	it = NoiseFloor( t );   ft = t - (float)it;

	r0 = NoiseLerpCube( ix, iy, iz, it,     fx, fy, fz );
	r1 = NoiseLerpCube( ix, iy, iz, it + 1, fx, fy, fz );

	return ft * r1 + ( 1.0f - ft ) * r0;
}
