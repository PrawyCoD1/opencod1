/*
 * universal/com_math.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/universal/com_math.c
 *
 * Retail range 0x0042D5E0-0x004314FF.
 *
 * YawVectors: exe 0x0042E7D0 (alias AngleVectorsFlat_m), game 0x20014470,
 * cgame 0x3003A900, ui 0x40002150.
 *
 * gunrandom: exe 0x0042D6E0 and cgame 0x30039810 (both alias
 * RandomVectorOnCircle), game 0x20013380, ui 0x400010D0; the Mac symbol
 * tables carry the real name.
 *
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"

float __fsqrt( float number );
void SinCos( float radians, float *sinOut, float *cosOut );
void SinCos_double( double radians, double *sinOut, double *cosOut );
int Q_log2( int val );
float Q_acos( float c );
signed char ClampChar( int i );
signed short ClampShort( int i );

int RandomInt( int *seed );
float RandomFloat( int *seed );
float CRandomFloat( int *seed );
void gunrandom( float *outX, float *outY );

byte DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

int VectorCompare( const vec3_t v1, const vec3_t v2 );
vec_t Distance( const vec3_t p1, const vec3_t p2 );
vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 );
vec_t Distance2d( const vec3_t p1, const vec3_t p2 );
vec_t Distance2dSquared( const vec3_t p1, const vec3_t p2 );
void Perp2D( vec3_t out, const vec3_t in );
vec_t VectorNormalize2D( float *v );
vec_t Vector4Normalize( vec4_t v );
void VectorNormalizeFast( vec3_t v );
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
void VectorInverse( vec3_t v );
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
vec_t VectorMax( const vec3_t v );
void Vector5Add_m( const float *a, const float *b, float *out );
void Vector5Scale_m( const float *in, float scale, float *out );
void Vector3Copy_m( vec3_t out, const vec3_t in );
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out );
void PerpendicularVector( vec3_t dst, const vec3_t src );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
							  float degrees );
void RotateAroundDirection( vec3_t axis[3], float yaw );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );

float vectoyaw( const vec3_t vec );
float vectosignedyaw( const vec3_t vec );
float vectopitch( const vec3_t vec );
float vectosignedpitch( const vec3_t vec );
void vectoangles( const vec3_t value1, vec3_t angles );
void vectoanglessigned( const vec3_t value1, vec3_t angles );

void MatrixMultiply( float in1[3][3], float in2[3][3], float out[3][3] );
void MatrixMultiply33InPlace_m( float inout[3][3], const float left[3][3] );
void MatrixMultiply34_m( const float left[3][4], const float right[3][4], float out[3][4] );
void QuatToMatrix( float quatAndMatrix[9] );
float RotationToYaw( const float v[2] );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void ProjectPointOntoVector( vec3_t dst, const vec3_t p, const vec3_t normal );
int BoxOnPlaneSide( const vec3_t emins, const vec3_t emaxs, cplane_t *p );

void RotatePointByAngles_m( const vec3_t in, const vec3_t angles, vec3_t out );
void PolarToVector_m( vec3_t out, float radius, float angle );
float PitchOfVectorAlongYaw( float yawDegrees, const vec3_t vec );

void Com_SRand( int seed );
float Com_RandFloatRange( float lo, float hi );
int Com_RandIntRange( int lo, int hi );

vec3_t vec3_origin = { 0.0f, 0.0f, 0.0f };

/* ---- ColorBytes3  0x00430010 ---- VERIFIED */
unsigned ColorBytes3( float r, float g, float b ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = 0xff;

	return i;
}

/* ---- ColorBytes4  0x00430060 ---- VERIFIED */
unsigned ColorBytes4( float r, float g, float b, float a ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = a * 255;

	return i;
}

/* ---- NormalizeColor  0x004300C0 ---- VERIFIED */
float NormalizeColor( const vec3_t in, vec3_t out ) {
	float max;

	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( !max ) {
		VectorClear( out );
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}

/* ---- NormalizeColorWhite_m  0x00430FD0 ---- VERIFIED */
float NormalizeColorWhite_m( const vec3_t in, vec3_t out ) {
	float max, inv;

	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( max == 0.0f ) {
		out[0] = out[1] = out[2] = 1.0f;
		return 0.0f;
	}

	inv = 1.0f / max;
	out[0] = in[0] * inv;
	out[1] = in[1] * inv;
	out[2] = in[2] * inv;
	return max;
}

#define ANGLE_TO_SHORT      182.04445f
#define ANGLE_FROM_SHORT    ( 360.0f / 65536.0f )

/* ---- AngleMod  0x00430140 ---- VERIFIED */
float AngleMod( float a ) {
	a = ANGLE_FROM_SHORT * ( (int)( a * ANGLE_TO_SHORT ) & 65535 );
	return a;
}

/* ---- LerpAngle  0x00430170 ---- VERIFIED */
float LerpAngle( float from, float to, float frac ) {
	float a;

	if ( to - from > 180 ) {
		to -= 360;
	}
	if ( to - from < -180 ) {
		to += 360;
	}
	a = from + frac * ( to - from );

	return a;
}

/* ---- AngleSubtract  0x004301E0 ---- VERIFIED */
float AngleSubtract( float a1, float a2 ) {
	float a;

	a = a1 - a2;
	while ( a > 180 ) {
		a -= 360;
	}
	while ( a < -180 ) {
		a += 360;
	}
	return a;
}

/* ---- AnglesSubtract  0x00430260 ---- VERIFIED */
void AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 ) {
	v3[0] = AngleSubtract( v1[0], v2[0] );
	v3[1] = AngleSubtract( v1[1], v2[1] );
	v3[2] = AngleSubtract( v1[2], v2[2] );
}

/* ---- AngleNormalize360  0x004302A0 ---- VERIFIED */
float AngleNormalize360( float angle ) {
	return ANGLE_FROM_SHORT * ( (int)( angle * ANGLE_TO_SHORT ) & 65535 );
}

/* ---- AngleNormalize180  0x004302D0 ---- VERIFIED */
float AngleNormalize180( float angle ) {
	angle = AngleNormalize360( angle );
	if ( angle > 180.0 ) {
		angle -= 360.0;
	}
	return angle;
}

/* ---- AngleNormalizePositive_m  0x00430330 ---- VERIFIED */
float AngleNormalizePositive_m( float angle ) {
	if ( angle >= 0.0f ) {
		while ( angle >= 360.0f ) {
			angle -= 360.0f;
		}
	} else {
		do {
			angle += 360.0f;
		} while ( angle < 0.0f );
	}
	return angle;
}

/* ---- AngleNormalizeSigned_m  0x004303B0 ---- VERIFIED */
float AngleNormalizeSigned_m( float angle ) {
	if ( angle > -180.0f ) {
		while ( angle > 180.0f ) {
			angle -= 360.0f;
		}
	} else {
		do {
			angle += 360.0f;
		} while ( angle <= -180.0f );
	}
	return angle;
}

/* ---- AngleDelta  0x00430430 ---- VERIFIED */
float AngleDelta( float angle1, float angle2 ) {
	return AngleNormalize180( angle1 - angle2 );
}

/* ---- RadiusFromBounds  0x00430450 ---- VERIFIED */
float RadiusFromBounds( const vec3_t mins, const vec3_t maxs ) {
	int i;
	vec3_t corner;
	float a, b;

	for ( i = 0 ; i < 3 ; i++ ) {
		a = fabs( mins[i] );
		b = fabs( maxs[i] );
		corner[i] = a > b ? a : b;
	}

	return VectorLength( corner );
}

/* ---- ClearBounds  0x004304D0 ---- VERIFIED */
void ClearBounds( vec3_t mins, vec3_t maxs ) {
	mins[0] = mins[1] = mins[2] = 262144;
	maxs[0] = maxs[1] = maxs[2] = -262144;
}

/* ---- AddPointToBounds  0x004304F0 ---- VERIFIED */
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs ) {
	if ( v[0] < mins[0] ) {
		mins[0] = v[0];
	}
	if ( v[0] > maxs[0] ) {
		maxs[0] = v[0];
	}

	if ( v[1] < mins[1] ) {
		mins[1] = v[1];
	}
	if ( v[1] > maxs[1] ) {
		maxs[1] = v[1];
	}

	if ( v[2] < mins[2] ) {
		mins[2] = v[2];
	}
	if ( v[2] > maxs[2] ) {
		maxs[2] = v[2];
	}
}

/* ---- ExpandBounds  0x00430560 ---- VERIFIED */
void ExpandBounds( const vec3_t mins, vec3_t outMins, const vec3_t maxs, vec3_t outMaxs ) {
	int i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( outMins[i] > mins[i] ) {
			outMins[i] = mins[i];
		}
		if ( outMaxs[i] < maxs[i] ) {
			outMaxs[i] = maxs[i];
		}
	}
}

/* ---- AxisClear  0x004305D0 ---- VERIFIED */
void AxisClear( vec3_t axis[3] ) {
	axis[0][0] = 1;
	axis[0][1] = 0;
	axis[0][2] = 0;
	axis[1][0] = 0;
	axis[1][1] = 1;
	axis[1][2] = 0;
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

/* ---- AxisCopy  0x00430600 ---- VERIFIED */
void AxisCopy( vec3_t in[3], vec3_t out[3] ) {
	VectorCopy( in[0], out[0] );
	VectorCopy( in[1], out[1] );
	VectorCopy( in[2], out[2] );
}

/* ---- AnglesToAxis  0x00430640 ---- VERIFIED */
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] ) {
	vec3_t right;

	AngleVectors( angles, axis[0], right, axis[2] );
	VectorSubtract( vec3_origin, right, axis[1] );
}

/* ---- YawVectors  0x0042E7D0 ---- VERIFIED */
void YawVectors( float yaw, vec3_t forward, vec3_t right ) {
	float angle, s, c;

	angle = yaw * ( M_PI * 2 / 360 );
	s = sin( angle );
	c = cos( angle );

	if ( forward ) {
		forward[0] = c;
		forward[1] = s;
		forward[2] = 0;
	}
	if ( right ) {
		right[0] = s;
		right[1] = -c;
		right[2] = 0;
	}
}

/* ---- YawToAxis  0x00430690 ---- VERIFIED */
void YawToAxis( float yaw, vec3_t axis[3] ) {
	vec3_t right;

	YawVectors( yaw, axis[0], right );
	VectorSubtract( vec3_origin, right, axis[1] );
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

/* ---- SetPlaneSignbits  0x00430C40 ---- VERIFIED */
void SetPlaneSignbits( cplane_t *out ) {
	int bits, j;

	bits = 0;
	for ( j = 0 ; j < 3 ; j++ ) {
		if ( out->normal[j] < 0 ) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}

/* ---- NormalFromPoints_m  0x0042E8C0 ---- VERIFIED */
void NormalFromPoints_m( const vec3_t a, vec3_t normal, const vec3_t c, const vec3_t d ) {
	vec3_t v1, v2;

	VectorSubtract( c, a, v1 );
	VectorNormalize( v1 );
	VectorSubtract( c, d, v2 );
	VectorNormalize( v2 );

	CrossProduct( v1, v2, normal );
	VectorNormalize( normal );
}

/* ---- ProjectPointOntoLine_m  0x0042E970 ---- VERIFIED */
void ProjectPointOntoLine_m( const vec3_t p1, const vec3_t p2, vec3_t out, const vec3_t org ) {
	vec3_t dir, rel;
	float d;

	VectorSubtract( p2, org, rel );
	VectorSubtract( p1, org, dir );
	VectorNormalize( dir );

	d = DotProduct( dir, rel );
	VectorMA( org, d, dir, out );
}

/* ---- MatrixTranspose  0x0042F0F0 ---- VERIFIED */
void MatrixTranspose( const float in[9], float out[9] ) {
	out[0] = in[0];
	out[1] = in[3];
	out[2] = in[6];
	out[3] = in[1];
	out[4] = in[4];
	out[5] = in[7];
	out[6] = in[2];
	out[7] = in[5];
	out[8] = in[8];
}

/* ---- MatrixInverse  0x0042F130 ---- VERIFIED */
void MatrixInverse( const float in[9], float out[9] ) {
	float c0, det, inv;

	c0 = in[4] * in[8] - in[7] * in[5];
	det = c0 * in[0]
		- ( in[1] * in[8] - in[7] * in[2] ) * in[3]
		+ ( in[1] * in[5] - in[2] * in[4] ) * in[6];
	inv = 1.0f / det;

	out[0] =  c0 * inv;
	out[1] = -( in[1] * in[8] - in[7] * in[2] ) * inv;
	out[2] =  ( in[1] * in[5] - in[2] * in[4] ) * inv;
	out[3] = -( in[3] * in[8] - in[6] * in[5] ) * inv;
	out[4] =  ( in[0] * in[8] - in[2] * in[6] ) * inv;
	out[5] = -( in[0] * in[5] - in[2] * in[3] ) * inv;
	out[6] =  ( in[7] * in[3] - in[6] * in[4] ) * inv;
	out[7] = -( in[7] * in[0] - in[1] * in[6] ) * inv;
	out[8] =  ( in[4] * in[0] - in[1] * in[3] ) * inv;
}

/* ---- MatrixTransformVector33_m  0x0042F900 ---- VERIFIED */
void MatrixTransformVector33_m( const float m[9], vec3_t out, const vec3_t in ) {
	out[0] = m[0] * in[0] + m[1] * in[1] + m[2] * in[2];
	out[1] = m[3] * in[0] + m[4] * in[1] + m[5] * in[2];
	out[2] = m[6] * in[0] + m[7] * in[1] + m[8] * in[2];
}

/* ---- MatrixTransformVector43  0x0042F950 ---- VERIFIED */
void MatrixTransformVector43( const float m[12], vec3_t out, const vec3_t in ) {
	out[0] = m[0] * in[0] + m[3] * in[1] + m[6] * in[2] + m[9];
	out[1] = m[1] * in[0] + m[4] * in[1] + m[7] * in[2] + m[10];
	out[2] = m[2] * in[0] + m[5] * in[1] + m[8] * in[2] + m[11];
}

/* ---- MatrixTransformVector44_m  0x0042F9A0 ---- VERIFIED */
void MatrixTransformVector44_m( const float m[16], vec3_t out, const vec3_t in ) {
	out[0] = m[0] * in[0] + m[4] * in[1] + m[8] * in[2] + m[12];
	out[1] = m[1] * in[0] + m[5] * in[1] + m[9] * in[2] + m[13];
	out[2] = m[2] * in[0] + m[6] * in[1] + m[10] * in[2] + m[14];
}

/* ---- MatrixTransformVector43InPlace_m  0x0042FA70 ---- VERIFIED */
void MatrixTransformVector43InPlace_m( const float m[12], vec3_t v ) {
	vec3_t t;

	t[0] = m[0] * v[0] + m[3] * v[1] + m[6] * v[2] + m[9];
	t[1] = m[1] * v[0] + m[4] * v[1] + m[7] * v[2] + m[10];
	t[2] = m[2] * v[0] + m[5] * v[1] + m[8] * v[2] + m[11];

	VectorCopy( t, v );
}

/* ---- MatrixInverseOrthogonal34  0x0042F240 ---- VERIFIED */
void MatrixInverseOrthogonal34( float out[12], const float in[12] ) {
	vec3_t t;

	out[0] = in[0];
	out[1] = in[3];
	out[2] = in[6];
	out[3] = in[1];
	out[4] = in[4];
	out[5] = in[7];
	out[6] = in[2];
	out[7] = in[5];
	out[8] = in[8];

	t[0] = -in[9];
	t[1] = -in[10];
	t[2] = -in[11];

	out[9]  = t[0] * out[0] + t[1] * out[3] + t[2] * out[6];
	out[10] = t[0] * out[1] + t[1] * out[4] + t[2] * out[7];
	out[11] = t[0] * out[2] + t[1] * out[5] + t[2] * out[8];
}

/* ---- QuatMultiply  0x0042FB50 ---- VERIFIED */
void QuatMultiply( const float a[4], float out[4], const float b[4] ) {
	out[0] = a[3] * b[0] + b[3] * a[0] + a[1] * b[2] - a[2] * b[1];
	out[1] = a[3] * b[1] + b[3] * a[1] + a[2] * b[0] - a[0] * b[2];
	out[2] = a[3] * b[2] + b[3] * a[2] + a[0] * b[1] - a[1] * b[0];
	out[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

/* ---- QuatConjugate  0x0042FBD0 ---- VERIFIED */
void QuatConjugate( float out[4], const float in[4] ) {
	out[0] = -in[0];
	out[1] = -in[1];
	out[2] = -in[2];
	out[3] =  in[3];
}

/* ---- QuatFromPitch_m  0x0042FF30 ---- VERIFIED */
void QuatFromPitch_m( float q[4], float degrees ) {
	float half = degrees * ( M_PI / 360.0f );

	q[0] = 0;
	q[1] = sin( half );
	q[2] = 0;
	q[3] = cos( half );
}

/* QuatFromYaw_m  0x0042FF80  VERIFIED */
void QuatFromYaw_m( float q[4], float degrees ) {
	float half = degrees * ( M_PI / 360.0f );

	q[0] = 0;
	q[1] = 0;
	q[2] = sin( half );
	q[3] = cos( half );
}

/* QuatFromRoll_m  0x0042FFD0  VERIFIED */
void QuatFromRoll_m( float q[4], float degrees ) {
	float half = degrees * ( M_PI / 360.0f );

	q[0] = sin( half );
	q[1] = 0;
	q[2] = 0;
	q[3] = cos( half );
}

/* ---- QuatSinSquaredHalfAngle_m  0x0042FD90 ---- VERIFIED */
float QuatSinSquaredHalfAngle_m( const float q[4] ) {
	float xx, yy, zz, len2, inv;

	xx = q[0] * q[0];
	yy = q[1] * q[1];
	zz = q[2] * q[2];
	len2 = q[3] * q[3] + zz + yy + xx;

	if ( len2 == 0.0f ) {
		return 0.0f;
	}

	inv = 1.0f / len2;
	return zz * inv + yy * inv + xx * inv;
}

/* ---- SinSquaredDegrees_m  0x0042FE50 ---- VERIFIED */
float SinSquaredDegrees_m( float degrees ) {
	double s = sin( degrees * ( M_PI * 2 / 360 ) );

	return (float)( s * s );
}

/* ---- QuatDeltaSinSquaredHalfAngle_m  0x0042FE80 ---- VERIFIED */
float QuatDeltaSinSquaredHalfAngle_m( const float a[4], const float b[4] ) {
	float conj[4], delta[4];

	QuatConjugate( conj, a );
	QuatMultiply( conj, delta, b );

	return QuatSinSquaredHalfAngle_m( delta );
}

/* ---- VectorRound_m  0x00431240 ---- VERIFIED */
void VectorRound_m( vec3_t v ) {
	v[0] = floor( v[0] + 0.5f );
	v[1] = floor( v[1] + 0.5f );
	v[2] = floor( v[2] + 0.5f );
}

/* ---- RoundToPrecision_m  0x00431320 ---- VERIFIED */
float RoundToPrecision_m( float value, int places ) {
	double scaled, integral, frac;

	scaled = pow( 10.0, (double)places ) * value;
	frac = modf( scaled, &integral );

	if ( frac >= 0.5 ) {
		integral += 1.0;
	} else if ( frac <= -0.5 ) {
		integral -= 1.0;
	}

	return (float)( pow( 0.1, (double)places ) * integral );
}

/* ---- RotateVector2D  0x0042FAE0 ---- VERIFIED */
void RotateVector2D( vec3_t v, float degrees ) {
	float angle, s, c, x;

	angle = degrees * ( M_PI * 2 / 360 );
	c = cos( angle );
	s = sin( angle );

	x = c * v[0] - s * v[1];
	v[1] = s * v[0] + c * v[1];
	v[0] = x;
}

/* ---- RotatePointAroundPoint_m  0x00431170 ---- VERIFIED */
void RotatePointAroundPoint_m( const vec3_t point, const vec3_t angles, vec3_t out,
							   const vec3_t pivot ) {
	vec3_t rel, res;

	VectorSubtract( point, pivot, rel );
	RotatePointByAngles_m( rel, angles, res );
	VectorAdd( res, pivot, out );
}

/* ---- __fsqrt  0x0042D5E0 ---- VERIFIED */
float __fsqrt( float number ) {
	long i;
	float x2, y;

	x2 = number * 0.5f;
	y  = number;
	i  = *(long *)&y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = *(float *)&i;
	y  = y * ( 1.5f - ( x2 * y * y ) );

	return y;
}

/* ---- SinCos  0x0042D620 ---- VERIFIED */
void SinCos( float radians, float *sinOut, float *cosOut ) {
	*cosOut = (float)cos( radians );
	*sinOut = (float)sin( radians );
}

void SinCos_double( double radians, double *sinOut, double *cosOut ) {
	*cosOut = cos( radians );
	*sinOut = sin( radians );
}

/* ---- Q_log2  0x0042D790 ---- VERIFIED */
int Q_log2( int val ) {
	int answer;

	answer = 0;
	while ( ( val >>= 1 ) != 0 ) {
		answer++;
	}
	return answer;
}

/* ---- Q_acos  0x0042D7A0 ---- VERIFIED */
float Q_acos( float c ) {
	float angle;

	angle = (float)acos( c );

	if ( angle > M_PI ) {
		return (float)M_PI;
	}
	if ( angle < -M_PI ) {
		return (float)M_PI;
	}
	return angle;
}

/* ---- ClampChar  0x0042D7E0 ---- VERIFIED */
signed char ClampChar( int i ) {
	if ( i < -128 ) {
		return -128;
	}
	if ( i > 127 ) {
		return 127;
	}
	return (signed char)i;
}

signed short ClampShort( int i ) {
	if ( i < -32768 ) {
		return -32768;
	}
	if ( i > 0x7fff ) {
		return 0x7fff;
	}
	return (signed short)i;
}

/* ---- RandomInt  0x0042D660 ---- */
int RandomInt( int *seed ) {
	*seed = ( 69069 * *seed + 1 );
	return *seed;
}

/* ---- RandomFloat  0x0042D670 ---- */
float RandomFloat( int *seed ) {
	return ( RandomInt( seed ) & 0xffff ) / (float)0x10000;
}

/* ---- CRandomFloat  0x0042D6A0 ---- */
float CRandomFloat( int *seed ) {
	float r;

	r = ( RandomInt( seed ) & 0xffff ) / (float)0x10000;
	return ( r - 0.5f ) + ( r - 0.5f );
}

/* ---- gunrandom  0x0042D6E0 ---- VERIFIED */
void gunrandom( float *outX, float *outY ) {
	float degrees, radius, radians, s, c;

	degrees = (float)rand() / 32768.0f * 360.0f;
	radius  = (float)rand() / 32768.0f;

	radians = degrees * (float)M_PI / 180.0f;
	SinCos( radians, &s, &c );

	*outX = c * radius;
	*outY = s * radius;
}

#define NUMVERTEXNORMALS    162

vec3_t bytedirs[NUMVERTEXNORMALS] =
{
	{  -0.525731f,   0.000000f,   0.850651f },
	{  -0.442863f,   0.238856f,   0.864188f },
	{  -0.295242f,   0.000000f,   0.955423f },
	{  -0.309017f,   0.500000f,   0.809017f },
	{  -0.162460f,   0.262866f,   0.951056f },
	{   0.000000f,   0.000000f,   1.000000f },
	{   0.000000f,   0.850651f,   0.525731f },
	{  -0.147621f,   0.716567f,   0.681718f },
	{   0.147621f,   0.716567f,   0.681718f },
	{   0.000000f,   0.525731f,   0.850651f },
	{   0.309017f,   0.500000f,   0.809017f },
	{   0.525731f,   0.000000f,   0.850651f },
	{   0.295242f,   0.000000f,   0.955423f },
	{   0.442863f,   0.238856f,   0.864188f },
	{   0.162460f,   0.262866f,   0.951056f },
	{  -0.681718f,   0.147621f,   0.716567f },
	{  -0.809017f,   0.309017f,   0.500000f },
	{  -0.587785f,   0.425325f,   0.688191f },
	{  -0.850651f,   0.525731f,   0.000000f },
	{  -0.864188f,   0.442863f,   0.238856f },
	{  -0.716567f,   0.681718f,   0.147621f },
	{  -0.688191f,   0.587785f,   0.425325f },
	{  -0.500000f,   0.809017f,   0.309017f },
	{  -0.238856f,   0.864188f,   0.442863f },
	{  -0.425325f,   0.688191f,   0.587785f },
	{  -0.716567f,   0.681718f,  -0.147621f },
	{  -0.500000f,   0.809017f,  -0.309017f },
	{  -0.525731f,   0.850651f,   0.000000f },
	{   0.000000f,   0.850651f,  -0.525731f },
	{  -0.238856f,   0.864188f,  -0.442863f },
	{   0.000000f,   0.955423f,  -0.295242f },
	{  -0.262866f,   0.951056f,  -0.162460f },
	{   0.000000f,   1.000000f,   0.000000f },
	{   0.000000f,   0.955423f,   0.295242f },
	{  -0.262866f,   0.951056f,   0.162460f },
	{   0.238856f,   0.864188f,   0.442863f },
	{   0.262866f,   0.951056f,   0.162460f },
	{   0.500000f,   0.809017f,   0.309017f },
	{   0.238856f,   0.864188f,  -0.442863f },
	{   0.262866f,   0.951056f,  -0.162460f },
	{   0.500000f,   0.809017f,  -0.309017f },
	{   0.850651f,   0.525731f,   0.000000f },
	{   0.716567f,   0.681718f,   0.147621f },
	{   0.716567f,   0.681718f,  -0.147621f },
	{   0.525731f,   0.850651f,   0.000000f },
	{   0.425325f,   0.688191f,   0.587785f },
	{   0.864188f,   0.442863f,   0.238856f },
	{   0.688191f,   0.587785f,   0.425325f },
	{   0.809017f,   0.309017f,   0.500000f },
	{   0.681718f,   0.147621f,   0.716567f },
	{   0.587785f,   0.425325f,   0.688191f },
	{   0.955423f,   0.295242f,   0.000000f },
	{   1.000000f,   0.000000f,   0.000000f },
	{   0.951056f,   0.162460f,   0.262866f },
	{   0.850651f,  -0.525731f,   0.000000f },
	{   0.955423f,  -0.295242f,   0.000000f },
	{   0.864188f,  -0.442863f,   0.238856f },
	{   0.951056f,  -0.162460f,   0.262866f },
	{   0.809017f,  -0.309017f,   0.500000f },
	{   0.681718f,  -0.147621f,   0.716567f },
	{   0.850651f,   0.000000f,   0.525731f },
	{   0.864188f,   0.442863f,  -0.238856f },
	{   0.809017f,   0.309017f,  -0.500000f },
	{   0.951056f,   0.162460f,  -0.262866f },
	{   0.525731f,   0.000000f,  -0.850651f },
	{   0.681718f,   0.147621f,  -0.716567f },
	{   0.681718f,  -0.147621f,  -0.716567f },
	{   0.850651f,   0.000000f,  -0.525731f },
	{   0.809017f,  -0.309017f,  -0.500000f },
	{   0.864188f,  -0.442863f,  -0.238856f },
	{   0.951056f,  -0.162460f,  -0.262866f },
	{   0.147621f,   0.716567f,  -0.681718f },
	{   0.309017f,   0.500000f,  -0.809017f },
	{   0.425325f,   0.688191f,  -0.587785f },
	{   0.442863f,   0.238856f,  -0.864188f },
	{   0.587785f,   0.425325f,  -0.688191f },
	{   0.688191f,   0.587785f,  -0.425325f },
	{  -0.147621f,   0.716567f,  -0.681718f },
	{  -0.309017f,   0.500000f,  -0.809017f },
	{   0.000000f,   0.525731f,  -0.850651f },
	{  -0.525731f,   0.000000f,  -0.850651f },
	{  -0.442863f,   0.238856f,  -0.864188f },
	{  -0.295242f,   0.000000f,  -0.955423f },
	{  -0.162460f,   0.262866f,  -0.951056f },
	{   0.000000f,   0.000000f,  -1.000000f },
	{   0.295242f,   0.000000f,  -0.955423f },
	{   0.162460f,   0.262866f,  -0.951056f },
	{  -0.442863f,  -0.238856f,  -0.864188f },
	{  -0.309017f,  -0.500000f,  -0.809017f },
	{  -0.162460f,  -0.262866f,  -0.951056f },
	{   0.000000f,  -0.850651f,  -0.525731f },
	{  -0.147621f,  -0.716567f,  -0.681718f },
	{   0.147621f,  -0.716567f,  -0.681718f },
	{   0.000000f,  -0.525731f,  -0.850651f },
	{   0.309017f,  -0.500000f,  -0.809017f },
	{   0.442863f,  -0.238856f,  -0.864188f },
	{   0.162460f,  -0.262866f,  -0.951056f },
	{   0.238856f,  -0.864188f,  -0.442863f },
	{   0.500000f,  -0.809017f,  -0.309017f },
	{   0.425325f,  -0.688191f,  -0.587785f },
	{   0.716567f,  -0.681718f,  -0.147621f },
	{   0.688191f,  -0.587785f,  -0.425325f },
	{   0.587785f,  -0.425325f,  -0.688191f },
	{   0.000000f,  -0.955423f,  -0.295242f },
	{   0.000000f,  -1.000000f,   0.000000f },
	{   0.262866f,  -0.951056f,  -0.162460f },
	{   0.000000f,  -0.850651f,   0.525731f },
	{   0.000000f,  -0.955423f,   0.295242f },
	{   0.238856f,  -0.864188f,   0.442863f },
	{   0.262866f,  -0.951056f,   0.162460f },
	{   0.500000f,  -0.809017f,   0.309017f },
	{   0.716567f,  -0.681718f,   0.147621f },
	{   0.525731f,  -0.850651f,   0.000000f },
	{  -0.238856f,  -0.864188f,  -0.442863f },
	{  -0.500000f,  -0.809017f,  -0.309017f },
	{  -0.262866f,  -0.951056f,  -0.162460f },
	{  -0.850651f,  -0.525731f,   0.000000f },
	{  -0.716567f,  -0.681718f,  -0.147621f },
	{  -0.716567f,  -0.681718f,   0.147621f },
	{  -0.525731f,  -0.850651f,   0.000000f },
	{  -0.500000f,  -0.809017f,   0.309017f },
	{  -0.238856f,  -0.864188f,   0.442863f },
	{  -0.262866f,  -0.951056f,   0.162460f },
	{  -0.864188f,  -0.442863f,   0.238856f },
	{  -0.809017f,  -0.309017f,   0.500000f },
	{  -0.688191f,  -0.587785f,   0.425325f },
	{  -0.681718f,  -0.147621f,   0.716567f },
	{  -0.442863f,  -0.238856f,   0.864188f },
	{  -0.587785f,  -0.425325f,   0.688191f },
	{  -0.309017f,  -0.500000f,   0.809017f },
	{  -0.147621f,  -0.716567f,   0.681718f },
	{  -0.425325f,  -0.688191f,   0.587785f },
	{  -0.162460f,  -0.262866f,   0.951056f },
	{   0.442863f,  -0.238856f,   0.864188f },
	{   0.162460f,  -0.262866f,   0.951056f },
	{   0.309017f,  -0.500000f,   0.809017f },
	{   0.147621f,  -0.716567f,   0.681718f },
	{   0.000000f,  -0.525731f,   0.850651f },
	{   0.425325f,  -0.688191f,   0.587785f },
	{   0.587785f,  -0.425325f,   0.688191f },
	{   0.688191f,  -0.587785f,   0.425325f },
	{  -0.955423f,   0.295242f,   0.000000f },
	{  -0.951056f,   0.162460f,   0.262866f },
	{  -1.000000f,   0.000000f,   0.000000f },
	{  -0.850651f,   0.000000f,   0.525731f },
	{  -0.955423f,  -0.295242f,   0.000000f },
	{  -0.951056f,  -0.162460f,   0.262866f },
	{  -0.864188f,   0.442863f,  -0.238856f },
	{  -0.951056f,   0.162460f,  -0.262866f },
	{  -0.809017f,   0.309017f,  -0.500000f },
	{  -0.864188f,  -0.442863f,  -0.238856f },
	{  -0.951056f,  -0.162460f,  -0.262866f },
	{  -0.809017f,  -0.309017f,  -0.500000f },
	{  -0.681718f,   0.147621f,  -0.716567f },
	{  -0.681718f,  -0.147621f,  -0.716567f },
	{  -0.850651f,   0.000000f,  -0.525731f },
	{  -0.688191f,   0.587785f,  -0.425325f },
	{  -0.587785f,   0.425325f,  -0.688191f },
	{  -0.425325f,   0.688191f,  -0.587785f },
	{  -0.425325f,  -0.688191f,  -0.587785f },
	{  -0.587785f,  -0.425325f,  -0.688191f },
	{  -0.688191f,  -0.587785f,  -0.425325f },
};

/* ---- DirToByte  0x0042D810 ---- VERIFIED */
byte DirToByte( vec3_t dir ) {
	byte i, best;
	float d, bestd;

	if ( !dir ) {
		return 0;
	}

	bestd = 0;
	best = 0;
	for ( i = 0 ; i < NUMVERTEXNORMALS ; i++ ) {
		d = DotProduct( dir, bytedirs[i] );
		if ( d > bestd ) {
			bestd = d;
			best = i;
		}
	}

	return best;
}

/* ---- ByteToDir  0x0042D880 ---- VERIFIED */
void ByteToDir( int b, vec3_t dir ) {
	if ( (unsigned)b >= (unsigned)NUMVERTEXNORMALS ) {
		VectorCopy( vec3_origin, dir );
		return;
	}
	VectorCopy( bytedirs[b], dir );
}

/* ---- VectorCompare  0x0042D9A0 ---- */
int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	int i;
	float d;

	for ( i = 0 ; i < 3 ; i++ ) {
		d = v1[i] - v2[i];
		if ( d * d > 0.0000010000001f ) {
			return 0;
		}
	}
	return 1;
}

/* ---- VectorLength  0x0042D9F0 ---- VERIFIED */
vec_t VectorLength( const vec3_t v ) {
	return (vec_t)sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
}

/* ---- Distance  0x0042DA40 ---- VERIFIED */
vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
	float dx, dy, dz;

	dx = p1[0] - p2[0];
	dy = p1[1] - p2[1];
	dz = p1[2] - p2[2];

	return (vec_t)sqrt( dz * dz + dy * dy + dx * dx );
}

vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	float dx, dy, dz;

	dx = p1[0] - p2[0];
	dy = p1[1] - p2[1];
	dz = p1[2] - p2[2];

	return dz * dz + dy * dy + dx * dx;
}

vec_t Distance2d( const vec3_t p1, const vec3_t p2 ) {
	float dx, dy;

	dx = p1[0] - p2[0];
	dy = p1[1] - p2[1];

	return (vec_t)sqrt( dy * dy + dx * dx );
}

vec_t Distance2dSquared( const vec3_t p1, const vec3_t p2 ) {
	float dx, dy;

	dx = p1[0] - p2[0];
	dy = p1[1] - p2[1];

	return dy * dy + dx * dx;
}

/* ---- CrossProduct  0x0042DB40 ---- VERIFIED */
void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/* ---- Perp2D  0x0042DB70 ---- VERIFIED */
void Perp2D( vec3_t out, const vec3_t in ) {
	float y;

	y = -in[0];
	out[0] = in[1];
	out[2] = 0;
	out[1] = y;
}

/* ---- VectorNormalize  0x0042DB90 ---- VERIFIED */
vec_t VectorNormalize( vec3_t v ) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = (float)sqrt( length );

	if ( length ) {
		ilength = 1.0f / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

/* ---- VectorNormalize2D  0x0042DC20 ---- VERIFIED */
vec_t VectorNormalize2D( float *v ) {
	float length, ilength;

	length = (float)sqrt( v[0] * v[0] + v[1] * v[1] );

	if ( length != 0.0f ) {
		ilength = 1.0f / length;
		v[0] *= ilength;
		v[1] *= ilength;
	}

	return length;
}

vec_t Vector4Normalize( vec4_t v ) {
	float length, ilength;

	length = (float)sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3] );

	if ( length != 0.0f ) {
		ilength = 1.0f / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
		v[3] *= ilength;
	}

	return length;
}

/* ---- VectorNormalizeFast  0x0042DD30 ---- VERIFIED */
void VectorNormalizeFast( vec3_t v ) {
	float ilength;

	ilength = __fsqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );

	v[0] *= ilength;
	v[1] *= ilength;
	v[2] *= ilength;
}

/* ---- VectorNormalize2  0x0042DDC0 ---- VERIFIED */
vec_t VectorNormalize2( const vec3_t v, vec3_t out ) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = (float)sqrt( length );

	if ( length == 0.0f ) {
		VectorClear( out );
	} else {
		ilength = 1.0f / length;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
	}

	return length;
}

/* ---- VectorInverse  0x0042DE50 ---- VERIFIED */
void VectorInverse( vec3_t v ) {
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out ) {
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
	out[3] = in[3] * scale;
}

/* ---- VectorMax  0x0042DEA0 ---- VERIFIED */
vec_t VectorMax( const vec3_t v ) {
	float m;

	m = ( v[0] < v[1] ) ? v[1] : v[0];

	return ( m < v[2] ) ? v[2] : m;
}

/* ---- Vector5Add_m  0x00431290 ---- VERIFIED */
void Vector5Add_m( const float *a, const float *b, float *out ) {
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
	out[3] = a[3] + b[3];
	out[4] = a[4] + b[4];
}

/* Vector5Scale_m  0x004312C0  VERIFIED */
void Vector5Scale_m( const float *in, float scale, float *out ) {
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
	out[3] = in[3] * scale;
	out[4] = in[4] * scale;
}

/* Vector3Copy_m  0x0042D930  VERIFIED */
void Vector3Copy_m( vec3_t out, const vec3_t in ) {
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

/* ---- VectorRotate  0x0042DEE0 ---- VERIFIED */
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out ) {
	out[0] = DotProduct( in, matrix[0] );
	out[1] = DotProduct( in, matrix[1] );
	out[2] = DotProduct( in, matrix[2] );
}

/* ---- PerpendicularVector  0x0042E840 ---- VERIFIED */
void PerpendicularVector( vec3_t dst, const vec3_t src ) {
	int pos;
	int i;
	float minelem;
	vec3_t tempvec;

	minelem = 1.0f;
	pos = 0;
	for ( i = 0 ; i < 3 ; i++ ) {
		if ( (float)fabs( src[i] ) < minelem ) {
			pos = i;
			minelem = (float)fabs( src[i] );
		}
	}

	tempvec[0] = tempvec[1] = tempvec[2] = 0.0f;
	tempvec[pos] = 1.0f;

	ProjectPointOntoVector( dst, tempvec, src );
	VectorNormalize( dst );
}

/* ---- RotatePointAroundVector  0x0042DF30 ---- VERIFIED */
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
							  float degrees ) {
	float m[3][3];
	float im[3][3];
	float zrot[3][3];
	float tmpmat[3][3];
	float rot[3][3];
	int i;
	vec3_t vr, vup, vf;
	float rad;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[2][2] = 1.0f;

	rad = degrees * (float)M_PI / 180.0f;
	zrot[0][0] = (float)cos( rad );
	zrot[0][1] = (float)sin( rad );
	zrot[1][0] = -zrot[0][1];
	zrot[1][1] = zrot[0][0];

	MatrixMultiply( m, zrot, tmpmat );
	MatrixMultiply( tmpmat, im, rot );

	for ( i = 0 ; i < 3 ; i++ ) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

/* ---- RotateAroundDirection  0x0042E130 ---- VERIFIED */
void RotateAroundDirection( vec3_t axis[3], float yaw ) {
	PerpendicularVector( axis[1], axis[0] );

	if ( yaw != 0.0f ) {
		vec3_t temp;

		VectorCopy( axis[1], temp );
		RotatePointAroundVector( axis[1], axis[0], temp, yaw );
	}

	CrossProduct( axis[0], axis[1], axis[2] );
}

/* ---- MakeNormalVectors  0x0042E1B0 ---- VERIFIED */
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up ) {
	float d;

	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct( right, forward );
	VectorMA( right, -d, forward, right );
	VectorNormalize( right );
	CrossProduct( right, forward, up );
}

/* ---- vectoyaw  0x0042E250 ---- VERIFIED */
float vectoyaw( const vec3_t vec ) {
	float yaw;

	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		return 0.0f;
	}

	yaw = (float)( 180.0 * atan2( vec[1], vec[0] ) / (double)(float)M_PI );
	if ( yaw < 0.0f ) {
		yaw += 360.0f;
	}
	return yaw;
}

float vectosignedyaw( const vec3_t vec ) {
	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		return 0.0f;
	}

	return (float)( 180.0 * atan2( vec[1], vec[0] ) / (double)(float)M_PI );
}

/* ---- vectopitch  0x0042E310 ---- VERIFIED */
float vectopitch( const vec3_t vec ) {
	float forward, pitch;

	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		return ( vec[2] <= 0.0f ) ? 90.0f : 270.0f;
	}

	forward = (float)sqrt( vec[0] * vec[0] + vec[1] * vec[1] );
	pitch = (float)( -180.0 * atan2( vec[2], forward ) / (double)(float)M_PI );
	if ( pitch < 0.0f ) {
		pitch += 360.0f;
	}
	return pitch;
}

float vectosignedpitch( const vec3_t vec ) {
	float forward;

	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		return ( vec[2] <= 0.0f ) ? 90.0f : -90.0f;
	}

	forward = (float)sqrt( vec[0] * vec[0] + vec[1] * vec[1] );
	return (float)( -180.0 * atan2( vec[2], forward ) / (double)(float)M_PI );
}

/* ---- vectoangles  0x0042E470 ---- VERIFIED */
void vectoangles( const vec3_t value1, vec3_t angles ) {
	float forward;
	float yaw, pitch;

	if ( value1[1] == 0.0f && value1[0] == 0.0f ) {
		yaw = 0.0f;
		pitch = ( value1[2] <= 0.0f ) ? 90.0f : 270.0f;
	} else {
		yaw = (float)( 180.0 * atan2( value1[1], value1[0] ) / (double)(float)M_PI );
		if ( yaw < 0.0f ) {
			yaw += 360.0f;
		}

		forward = (float)sqrt( value1[0] * value1[0] + value1[1] * value1[1] );
		pitch = (float)( -180.0 * atan2( value1[2], forward ) / (double)(float)M_PI );
		if ( pitch < 0.0f ) {
			pitch += 360.0f;
		}
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0.0f;
}

/* vectoanglessigned  0x0042E590  VERIFIED */
void vectoanglessigned( const vec3_t value1, vec3_t angles ) {
	float forward;
	float yaw, pitch;

	if ( value1[1] == 0.0f && value1[0] == 0.0f ) {
		angles[0] = ( value1[2] <= 0.0f ) ? 90.0f : -90.0f;
		angles[1] = 0.0f;
		angles[2] = 0.0f;
		return;
	}

	yaw = (float)( 180.0 * atan2( value1[1], value1[0] ) / (double)(float)M_PI );

	forward = (float)sqrt( value1[0] * value1[0] + value1[1] * value1[1] );
	pitch = (float)( -180.0 * atan2( value1[2], forward ) / (double)(float)M_PI );

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0.0f;
}

/* ---- AngleVectors  0x0042E660 ---- VERIFIED */
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up ) {
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = angles[1] * ( (float)M_PI * 2 / 360 );
	SinCos( angle, &sy, &cy );

	angle = angles[0] * ( (float)M_PI * 2 / 360 );
	SinCos( angle, &sp, &cp );

	if ( forward ) {
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if ( !right && !up ) {
		return;
	}

	angle = angles[2] * ( (float)M_PI * 2 / 360 );
	SinCos( angle, &sr, &cr );

	if ( right ) {
		right[0] = ( -1 * sr * sp * cy + -1 * cr * -sy );
		right[1] = ( -1 * sr * sp * sy + -1 * cr * cy );
		right[2] = -1 * sr * cp;
	}

	if ( up ) {
		up[0] = ( cr * sp * cy + -sr * -sy );
		up[1] = ( cr * sp * sy + -sr * cy );
		up[2] = cr * cp;
	}
}

/* ---- MatrixMultiply  0x0042EA10 ---- VERIFIED */
void MatrixMultiply( float in1[3][3], float in2[3][3], float out[3][3] ) {
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}

/* ---- MatrixMultiply33InPlace_m  0x0042EAF0 ---- VERIFIED */
void MatrixMultiply33InPlace_m( float inout[3][3], const float left[3][3] ) {
	float t[3][3];
	int i, j;

	for ( i = 0 ; i < 3 ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			t[i][j] = left[i][0] * inout[0][j]
					+ left[i][1] * inout[1][j]
					+ left[i][2] * inout[2][j];
		}
	}

	memcpy( inout, t, sizeof( t ) );
}

/* ---- MatrixMultiply34_m  0x0042EC10 ---- VERIFIED */
void MatrixMultiply34_m( const float left[3][4], const float right[3][4], float out[3][4] ) {
	int i, j;

	for ( i = 0 ; i < 3 ; i++ ) {
		for ( j = 0 ; j < 4 ; j++ ) {
			out[i][j] = left[i][0] * right[0][j]
					  + left[i][1] * right[1][j]
					  + left[i][2] * right[2][j]
					  + ( j == 3 ? left[i][3] : 0.0f );
		}
	}
}

/* ---- QuatToMatrix  0x0042FBF0 ---- VERIFIED */
void QuatToMatrix( float quatAndMatrix[9] ) {
	float x, y, z, w;
	float xx, yy, zz, len2, s;
	float xs, ys;
	float xy, xz, xw, yz, yw, zw;

	x = quatAndMatrix[0];
	y = quatAndMatrix[1];
	z = quatAndMatrix[2];
	w = quatAndMatrix[3];

	xx = x * x;
	yy = y * y;
	zz = z * z;
	len2 = w * w + zz + yy + xx;

	if ( len2 == 0.0f ) {
		quatAndMatrix[0] = 1.0f;
		quatAndMatrix[1] = 0.0f;
		quatAndMatrix[2] = 0.0f;
		quatAndMatrix[3] = 0.0f;
		quatAndMatrix[4] = 1.0f;
		quatAndMatrix[5] = 0.0f;
		quatAndMatrix[6] = 0.0f;
		quatAndMatrix[7] = 0.0f;
		quatAndMatrix[8] = 1.0f;
		return;
	}

	s = 2.0f / len2;

	xx *= s;
	yy *= s;
	zz *= s;

	xs = s * x;
	xy = xs * y;
	xz = xs * z;
	xw = xs * w;

	ys = s * y;
	yz = ys * z;
	yw = ys * w;

	zw = w * z * s;

	quatAndMatrix[0] = 1.0f - ( zz + yy );
	quatAndMatrix[1] = zw + xy;
	quatAndMatrix[2] = xz - yw;
	quatAndMatrix[3] = xy - zw;
	quatAndMatrix[4] = 1.0f - ( zz + xx );
	quatAndMatrix[5] = yz + xw;
	quatAndMatrix[6] = yw + xz;
	quatAndMatrix[7] = yz - xw;
	quatAndMatrix[8] = 1.0f - ( yy + xx );
}

/* ---- RotationToYaw  0x0042FEC0 ---- VERIFIED */
float RotationToYaw( const float v[2] ) {
	float xx, scale;
	double num, den;

	xx = v[0] * v[0];
	scale = xx + v[1] * v[1];
	scale = 2.0f / scale;

	num = (double)( v[0] * v[1] * scale );
	den = 1.0 - (double)( xx * scale );

	return (float)( atan2( num, den ) * (double)(float)( 180.0 / 3.1415926535897931 ) );
}

/* ---- PlaneFromPoints  0x00430AD0 ---- VERIFIED */
qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c ) {
	vec3_t d1, d2;

	VectorSubtract( b, a, d1 );
	VectorSubtract( c, a, d2 );
	CrossProduct( d2, d1, plane );

	if ( VectorNormalize( plane ) == 0 ) {
		return qfalse;
	}

	plane[3] = DotProduct( a, plane );
	return qtrue;
}

/* ---- ProjectPointOntoVector  0x00430B90 ---- VERIFIED */
void ProjectPointOntoVector( vec3_t dst, const vec3_t p, const vec3_t normal ) {
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0f / DotProduct( normal, normal );

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/* ---- BoxOnPlaneSide  0x00430C80 ---- VERIFIED */
int BoxOnPlaneSide( const vec3_t emins, const vec3_t emaxs, cplane_t *p ) {
	float dist1, dist2;
	int sides;

	switch ( p->signbits ) {
	case 0:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 1:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 2:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 3:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 4:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;
		Com_Error( ERR_DROP, "\x15" "BoxOnPlaneSide: invalid signbits for plane" );
		break;
	}

	sides = 0;
	if ( dist1 >= p->dist ) {
		sides = 1;
	}
	if ( dist2 < p->dist ) {
		sides |= 2;
	}

	return sides;
}

/* ---- RotatePointByAngles_m  0x00431060 ---- VERIFIED */
void RotatePointByAngles_m( const vec3_t in, const vec3_t angles, vec3_t out ) {
	static const int pair[3][2] = { { 1, 2 }, { 2, 0 }, { 0, 1 } };
	vec3_t v, r;
	float rad, s, c;
	int i, j, k;

	VectorCopy( in, v );
	VectorCopy( in, r );

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( angles[i] != 0.0f ) {
			rad = angles[i] * (float)M_PI / 180.0f;
			c = (float)cos( rad );
			s = (float)sin( rad );

			j = pair[i][0];
			k = pair[i][1];

			r[j] = v[j] * c - v[k] * s;
			r[k] = v[k] * c + v[j] * s;
		}
		VectorCopy( r, v );
	}

	VectorCopy( r, out );
}

/* ---- PolarToVector_m  0x004311C0 ---- VERIFIED */
void PolarToVector_m( vec3_t out, float radius, float angle ) {
	float s1, c1, s2, c2;

	SinCos( angle, &s1, &c1 );
	SinCos( angle, &s2, &c2 );

	out[0] = c2 * c1 * radius;
	out[1] = c2 * s1 * radius;
	out[2] = s2 * radius;
}

/* ---- PitchOfVectorAlongYaw  0x004313C0 ---- VERIFIED */
float PitchOfVectorAlongYaw( float yawDegrees, const vec3_t vec ) {
	float rad, s, c;
	vec3_t flatForward, projected;

	rad = yawDegrees * 0.017453292f;
	SinCos( rad, &s, &c );

	flatForward[0] = c;
	flatForward[1] = s;
	flatForward[2] = 0.0f;

	ProjectPointOntoVector( projected, flatForward, vec );

	return vectopitch( projected );
}

/*
 * One seed and the three entry points that use it: game DLL
 * 0x200170D0..0x20017152, cgame DLL 0x3003D560..0x3003D5E2 (byte-identical).
 * com_randSeed is at 0x2006AD50 in the game DLL, initialised in the image to
 * 0x89ABCDEF.  Nothing else in the module touches it -- RandomInt/RandomFloat/
 * CRandomFloat above carry their own seed by pointer, Q3-style, and use a
 * different generator.
 */
int com_randSeed = 0x89ABCDEF;

/* ---- Com_SRand  game DLL 0x200170D0 ---- VERIFIED */
void Com_SRand( int seed ) {
	com_randSeed = seed;
}

/*
 * ---- Com_RandFloatRange  game DLL 0x200170E0 ---- VERIFIED
 *
 * The shift is `shr`, not `sar`, and the fild carries the compiler's
 * unsigned-to-float fixup, so the shifted seed is unsigned; the divisor is a
 * real fdiv against 32768.0 in .rdata, not a reciprocal multiply.
 */
float Com_RandFloatRange( float lo, float hi ) {
	com_randSeed = 214013 * com_randSeed + 2531011;
	return ( hi - lo ) * (float)( (unsigned)com_randSeed >> 17 ) / 32768.0f + lo;
}

/*
 * ---- Com_RandIntRange  game DLL 0x20017130 ---- VERIFIED
 *
 * Same generator; here the product is closed back to int (imul + sar 15).
 * lo and hi ride in edx and ecx, so their (lo, hi) order is not fixed by the
 * ABI; it follows Com_RandFloatRange's stack order.  The roles are unambiguous
 * either way: the subtrahend is the value added back at the end.
 */
int Com_RandIntRange( int lo, int hi ) {
	com_randSeed = 214013 * com_randSeed + 2531011;
	return ( (int)( (unsigned)com_randSeed >> 17 ) * ( hi - lo ) >> 15 ) + lo;
}


/* ==========================================================================
 * Merged from com_math_raw.c (retail linked it as a separate translation unit).
 * ========================================================================== */

extern float vectosignedpitch( const vec3_t vec );
extern void  vectoangles( const vec3_t value1, vec3_t angles );
extern void  vectoanglessigned( const vec3_t value1, vec3_t angles );
extern void  SinCos( float radians, float *sinOut, float *cosOut );

/* _DotProduct  0x0042D8D0  VERIFIED */
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 ) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

/* _VectorSubtract  0x0042D8F0  VERIFIED */
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out ) {
	out[0] = veca[0] - vecb[0];
	out[1] = veca[1] - vecb[1];
	out[2] = veca[2] - vecb[2];
}

/* _VectorAdd  0x0042D910  VERIFIED */
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out ) {
	out[0] = veca[0] + vecb[0];
	out[1] = veca[1] + vecb[1];
	out[2] = veca[2] + vecb[2];
}

/* _VectorCopy  0x0042D930  VERIFIED */
void _VectorCopy( const vec3_t in, vec3_t out ) {
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

/* _VectorScale  0x0042D950  VERIFIED */
void _VectorScale( const vec3_t in, vec_t scale, vec3_t out ) {
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

/* _VectorMA  0x0042D970  VERIFIED */
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc ) {
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}

/* ---- _CrossProduct  0x0042DB40 ---- VERIFIED */
void _CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/*
 * ---- MatrixMultiply43  0x0042ED40 ---- VERIFIED  (game DLL 0x200149E0)
 *
 * The 4x3 sibling of com_math.c's MatrixMultiply, three functions further down
 * the same run (MatrixMultiply, MatrixMultiply33InPlace, MatrixMultiply34,
 * MatrixMultiply43).  Both are __usercall in the retail image with no stack
 * args; (in1, in2, out) map onto (ecx, eax, edx), the same roles as
 * MatrixMultiply at 0x3003AB40: edx is written, out = M(ecx) . M(eax).
 * Alias MatrixMultiply43Compact, whose prototype puts the destination SECOND;
 * the (in1, in2, out) order here follows Q3's MatrixMultiply instead.
 */
void MatrixMultiply43( const float in1[4][3], const float in2[4][3], float out[4][3] ) {
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
	out[3][0] = in1[3][0] * in2[0][0] + in1[3][1] * in2[1][0] + in1[3][2] * in2[2][0] + in2[3][0];
	out[3][1] = in1[3][0] * in2[0][1] + in1[3][1] * in2[1][1] + in1[3][2] * in2[2][1] + in2[3][1];
	out[3][2] = in1[3][0] * in2[0][2] + in1[3][1] * in2[1][2] + in1[3][2] * in2[2][2] + in2[3][2];
}

/* ---- DObjSkelMatrixMultiply43  0x0042EE70 ---- VERIFIED */
float *__cdecl DObjSkelMatrixMultiply43(float *result, float *a2, float *a3)
{
  *a2 = *a3 * *result + a3[1] * result[3] + result[6] * a3[2];
  a2[3] = a3[5] * result[3] + a3[4] * *result + a3[6] * result[6];
  a2[6] = a3[9] * result[3] + a3[8] * *result + a3[10] * result[6];
  a2[1] = a3[1] * result[4] + result[7] * a3[2] + result[1] * *a3;
  a2[4] = a3[6] * result[7] + a3[5] * result[4] + a3[4] * result[1];
  a2[7] = a3[10] * result[7] + a3[9] * result[4] + a3[8] * result[1];
  a2[2] = *a3 * result[2] + a3[1] * result[5] + result[8] * a3[2];
  a2[5] = a3[6] * result[8] + a3[5] * result[5] + a3[4] * result[2];
  a2[8] = a3[10] * result[8] + a3[9] * result[5] + a3[8] * result[2];
  a2[9] = a3[13] * result[3] + a3[12] * *result + a3[14] * result[6] + result[9];
  a2[10] = a3[14] * result[7] + a3[13] * result[4] + a3[12] * result[1] + result[10];
  a2[11] = a3[14] * result[8] + a3[13] * result[5] + a3[12] * result[2] + result[11];
  return result;
}

/* ---- DObjSkel2MatrixMultiply43  0x0042EFA0 ---- VERIFIED */
float *__cdecl DObjSkel2MatrixMultiply43(float *result, int a2, float *a3)
{
  double v3;

  *(float *)a2 = *a3 * *result + a3[1] * result[3] + result[6] * a3[2];
  *(float *)(a2 + 16) = a3[5] * result[3] + a3[4] * *result + a3[6] * result[6];
  *(float *)(a2 + 32) = a3[9] * result[3] + a3[8] * *result + a3[10] * result[6];
  *(float *)(a2 + 4) = a3[1] * result[4] + result[7] * a3[2] + result[1] * *a3;
  *(float *)(a2 + 20) = a3[6] * result[7] + a3[5] * result[4] + a3[4] * result[1];
  *(float *)(a2 + 36) = a3[10] * result[7] + a3[9] * result[4] + a3[8] * result[1];
  *(float *)(a2 + 8) = *a3 * result[2] + a3[1] * result[5] + result[8] * a3[2];
  *(float *)(a2 + 24) = a3[6] * result[8] + a3[5] * result[5] + a3[4] * result[2];
  *(float *)(a2 + 40) = a3[10] * result[8] + a3[9] * result[5] + a3[8] * result[2];
  *(_DWORD *)(a2 + 12) = 0;
  *(_DWORD *)(a2 + 28) = 0;
  *(_DWORD *)(a2 + 44) = 0;
  *(float *)(a2 + 48) = a3[12] * *result + result[6] * a3[14] + a3[13] * result[3] + result[9];
  *(float *)(a2 + 52) = result[7] * a3[14] + result[4] * a3[13] + result[1] * a3[12] + result[10];
  v3 = a3[12] * result[2] + a3[14] * result[8] + a3[13] * result[5] + result[11];
  *(_DWORD *)(a2 + 60) = 1065353216;
  *(float *)(a2 + 56) = (float)v3;
  return result;
}

/* ---- MatrixInverse44  0x0042F300 ---- VERIFIED */
float *__cdecl MatrixInverse44(float *result, float *a2)
{
  float v2;
  int v3;
  float v4;
  float v5;
  float v6;
  float v7;
  float v8;
  float v9;
  float v10;
  float v11;
  float v12;
  float v13;
  float v14;
  float v15;
  float v16;
  float v17;
  float v18;
  float v19;
  float v20;
  float v21;
  float v22;
  float v23;
  float v24;
  float v25;
  float v26;
  float v27;
  float v28;
  float v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;
  float v35;
  float v36;
  float v37;
  float v38;
  float v39;
  float v40;
  float v41;
  float v42;
  float v43;
  float v44;

  v30 = *a2;
  v34 = a2[1];
  v38 = a2[2];
  v41 = a2[3];
  v31 = a2[4];
  v35 = a2[5];
  v39 = a2[6];
  v42 = a2[7];
  v32 = a2[8];
  v36 = a2[9];
  v40 = a2[10];
  v43 = a2[11];
  v33 = a2[12];
  v37 = a2[13];
  v2 = a2[14];
  v44 = a2[15];
  v6 = v44 * v40;
  v8 = v43 * v2;
  v10 = v39 * v44;
  v12 = v42 * v2;
  v14 = v39 * v43;
  v16 = v42 * v40;
  v18 = v38 * v44;
  v20 = v41 * v2;
  v22 = v38 * v43;
  v24 = v41 * v40;
  v26 = v38 * v42;
  v28 = v41 * v39;
  *result = v37 * v14 + v36 * v12 + v35 * v6 - (v37 * v16 + v36 * v10 + v35 * v8);
  result[1] = v37 * v24 + v36 * v18 + v34 * v8 - (v37 * v22 + v36 * v20 + v34 * v6);
  result[2] = v37 * v26 + v35 * v20 + v34 * v10 - (v37 * v28 + v35 * v18 + v34 * v12);
  result[3] = v36 * v28 + v35 * v22 + v34 * v16 - (v36 * v26 + v35 * v24 + v34 * v14);
  result[4] = v33 * v16 + v32 * v10 + v31 * v8 - (v33 * v14 + v32 * v12 + v31 * v6);
  result[5] = v33 * v22 + v30 * v6 + v32 * v20 - (v33 * v24 + v30 * v8 + v32 * v18);
  result[6] = v33 * v28 + v30 * v12 + v31 * v18 - (v33 * v26 + v30 * v10 + v31 * v20);
  result[7] = v30 * v14 + v32 * v26 + v31 * v24 - (v30 * v16 + v32 * v28 + v31 * v22);
  v7 = v32 * v37;
  v9 = v33 * v36;
  v11 = v31 * v37;
  v13 = v33 * v35;
  v15 = v31 * v36;
  v17 = v32 * v35;
  v19 = v30 * v37;
  v21 = v33 * v34;
  v23 = v30 * v36;
  v25 = v32 * v34;
  v27 = v30 * v35;
  v29 = v31 * v34;
  result[8] = v13 * v43 + v42 * v7 + v15 * v44 - (v42 * v9 + v11 * v43 + v17 * v44);
  result[9] = v41 * v9 + v19 * v43 + v25 * v44 - (v21 * v43 + v41 * v7 + v23 * v44);
  result[10] = v21 * v42 + v41 * v11 + v27 * v44 - (v41 * v13 + v19 * v42 + v29 * v44);
  result[11] = v41 * v17 + v23 * v42 + v29 * v43 - (v41 * v15 + v25 * v42 + v27 * v43);
  result[12] = v17 * v2 + v39 * v9 + v11 * v40 - (v15 * v2 + v39 * v7 + v13 * v40);
  result[13] = v23 * v2 + v38 * v7 + v21 * v40 - (v25 * v2 + v38 * v9 + v19 * v40);
  result[14] = v38 * v13 + v19 * v39 + v29 * v2 - (v21 * v39 + v38 * v11 + v27 * v2);
  result[15] = v38 * v15 + v25 * v39 + v27 * v40 - (v38 * v17 + v23 * v39 + v29 * v40);
  v4 = v32 * result[2] + v33 * result[3] + v30 * *result + v31 * result[1];
  v3 = 0;
  v5 = (float)(1.0 / v4);
  do
  {
    ++v3;
    result[v3 - 1] = v5 * result[v3 - 1];
  }
  while ( v3 < 16 );
  return result;
}

/* ---- MatrixTransformVector  0x0042F8B0 ---- VERIFIED */
float *__cdecl MatrixTransformVector(float *result, float *a2, float *a3)
{
  *a2 = result[3] * a3[1] + result[6] * a3[2] + *a3 * *result;
  a2[1] = result[1] * *a3 + result[4] * a3[1] + result[7] * a3[2];
  a2[2] = result[2] * *a3 + result[5] * a3[1] + result[8] * a3[2];
  return result;
}

/* ---- MatrixTransposeTransformVector  0x0042F9F0 ---- VERIFIED */
float *__cdecl MatrixTransposeTransformVector(float *result, float *a2, float *a3)
{
  float v3;
  float v4;
  float v5;

  v3 = *a2 - result[9];
  v4 = a2[1] - result[10];
  v5 = a2[2] - result[11];
  *a3 = v5 * result[2] + v4 * result[1] + v3 * *result;
  a3[1] = v5 * result[5] + v4 * result[4] + v3 * result[3];
  a3[2] = v5 * result[8] + v4 * result[7] + v3 * result[6];
  return result;
}

/* ---- AxisToAngles  0x004306E0 ---- VERIFIED */
void AxisToAngles( const vec3_t axis[3], vec3_t angles ) {
	vec3_t right;
	float s, c, t, roll;

	// first get the pitch and yaw from the forward vector
	vectoangles( axis[0], angles );

	// now get the roll from the right vector, reverse-rotated by yaw then pitch
	VectorCopy( axis[1], right );

	SinCos( -angles[1] * (float)M_PI / 180.0f, &s, &c );
	t        = c * right[0] - s * right[1];
	right[1] = c * right[1] + s * right[0];

	SinCos( -angles[0] * (float)M_PI / 180.0f, &s, &c );
	right[0] = t * c + s * right[2];
	right[2] = c * right[2] - t * s;

	roll = vectosignedpitch( right );
	if ( right[1] < 0.0f ) {
		angles[2] = ( roll < 0.0f ? 180.0f : -180.0f ) + roll;
	} else {
		angles[2] = -roll;
	}
}

/* Axis4ToAngles and AxisToSignedAngles are byte-for-byte copies of AxisToAngles's
   roll math in retail (331 bytes each, game 0x20016380/0x200164D0/0x20016620);
   no shared helper exists there, so each carries its own copy. */
/* ---- Axis4ToAngles  0x00430830 ---- VERIFIED */
void Axis4ToAngles( const float matrix[3][4], vec3_t angles ) {
	vec3_t right;
	float s, c, t, roll;

	// first get the pitch and yaw from the forward vector
	vectoangles( matrix[0], angles );

	// now get the roll from the right vector, reverse-rotated by yaw then pitch
	VectorCopy( matrix[1], right );

	SinCos( -angles[1] * (float)M_PI / 180.0f, &s, &c );
	t        = c * right[0] - s * right[1];
	right[1] = c * right[1] + s * right[0];

	SinCos( -angles[0] * (float)M_PI / 180.0f, &s, &c );
	right[0] = t * c + s * right[2];
	right[2] = c * right[2] - t * s;

	roll = vectosignedpitch( right );
	if ( right[1] < 0.0f ) {
		angles[2] = ( roll < 0.0f ? 180.0f : -180.0f ) + roll;
	} else {
		angles[2] = -roll;
	}
}

/* ---- AxisToSignedAngles  0x00430980 ---- VERIFIED */
void AxisToSignedAngles( const vec3_t axis[3], vec3_t angles ) {
	vec3_t right;
	float s, c, t, roll;

	// first get the pitch and yaw from the forward vector
	vectoanglessigned( axis[0], angles );

	// now get the roll from the right vector, reverse-rotated by yaw then pitch
	VectorCopy( axis[1], right );

	SinCos( -angles[1] * (float)M_PI / 180.0f, &s, &c );
	t        = c * right[0] - s * right[1];
	right[1] = c * right[1] + s * right[0];

	SinCos( -angles[0] * (float)M_PI / 180.0f, &s, &c );
	right[0] = t * c + s * right[2];
	right[2] = c * right[2] - t * s;

	roll = vectosignedpitch( right );
	if ( right[1] < 0.0f ) {
		angles[2] = ( roll < 0.0f ? 180.0f : -180.0f ) + roll;
	} else {
		angles[2] = -roll;
	}
}
