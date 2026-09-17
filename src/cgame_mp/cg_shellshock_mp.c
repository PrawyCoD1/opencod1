/*
 * cg_shellshock_mp.c -- the shellshock post-effect: screen blend, sound
 * ducking and reverb, mouse attenuation, and the camera view kick.
 * (original source: cgame/cg_shellshock.c)
 *
 * cgame_mp_x86.dll 0x3002E5C0 .. 0x3002F831, eighteen functions.  CoD's own --
 * RTCW has nothing like it.
 *
 * The unit is driven from two places.  CG_DrawActiveFrame (0x30033CDD) picks
 * the parameter block out of cgs.shellshockParms with ps.shellshockIndex,
 * publishes it plus ps.shellshockTime / ps.shellshockDuration into
 * cg.shellshock (that publish is CG_UpdateShellShockSavedScreenBlend, inlined
 * there), and calls CG_UpdateShellShock; CG_DrawActive later calls
 * CG_DrawShellShockSavedScreenBlend off the same three fields.
 *
 * Four of the eighteen are unnamed helpers.  Three of those are inlined
 * everywhere and exist standalone only because something took their address or
 * the Mac symbols named them; sub_3002E720 is a dead developer tool that
 * printed the source text of cg_shellShockCameraTable below -- its
 * "\t{%f, %f},\n" is exactly the shape of the literal it generated, which is
 * why all 262 values in that table round-trip through "%.6f".
 *
 * @fidelity: likely
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "cg_local.h"

/*
 * Every seconds-to-milliseconds conversion in CG_SetShellShockParmsFromCvars
 * compiles as two evaluations of the same Q_ftol against a floor of 1, which is
 * what a MAX-style macro expanding its argument twice produces.
 */
#define SHOCK_MSEC( cv )    ( 1 > Q_ftol( ( cv ).value * 1000.0f ) \
							  ? 1 : Q_ftol( ( cv ).value * 1000.0f ) )

/*
 * universal/q_shared.c, universal/com_math.c and game_mp/bg_slidemove.c --
 * cgame compiles its own copy of all three.
 */
extern vec3_t vec3_origin;
vec_t       VectorNormalize( vec3_t v );

/*
 * The 27 cvars a .shock file carries, in the order the file writes them.
 * 0x300748A0 is the name table, 0x30074910 the matching vmCvar_t table --
 * CG_LoadShellShockCvars walks the second one 108 bytes at 4 (0x3002EB18).
 */
#define NUM_SHOCK_CVARS     27

static const char *cg_shockCvarNames[NUM_SHOCK_CVARS] = {
	"cg_shock_screenBlendTime",
	"cg_shock_screenBlendFadeTime",
	"cg_shock_viewKickPeriod",
	"cg_shock_viewKickRadius",
	"cg_shock_sound",
	"cg_shock_soundFadeInTime",
	"cg_shock_soundFadeOutTime",
	"cg_shock_soundLoopFadeTime",
	"cg_shock_soundLoopEndDelay",
	"cg_shock_soundRoomType",
	"cg_shock_soundWetLevel",
	"cg_shock_soundModEndDelay",
	"cg_shock_volume_auto",
	"cg_shock_volume_menu",
	"cg_shock_volume_weapon",
	"cg_shock_volume_voice",
	"cg_shock_volume_item",
	"cg_shock_volume_body",
	"cg_shock_volume_local",
	"cg_shock_volume_music",
	"cg_shock_volume_announcer",
	"cg_shock_volume_shellshock",
	"cg_shock_mouse",
	"cg_shock_mouse_maxpitchspeed",
	"cg_shock_mouse_maxyawspeed",
	"cg_shock_mouse_sensitivityscale",
	"cg_shock_mouse_fadeTime"
};

static vmCvar_t *cg_shockCvars[NUM_SHOCK_CVARS] = {
	&cg_shock_screenBlendTime,
	&cg_shock_screenBlendFadeTime,
	&cg_shock_viewKickPeriod,
	&cg_shock_viewKickRadius,
	&cg_shock_sound,
	&cg_shock_soundFadeInTime,
	&cg_shock_soundFadeOutTime,
	&cg_shock_soundLoopFadeTime,
	&cg_shock_soundLoopEndDelay,
	&cg_shock_soundRoomType,
	&cg_shock_soundWetLevel,
	&cg_shock_soundModEndDelay,
	&cg_shock_volume_auto,
	&cg_shock_volume_menu,
	&cg_shock_volume_weapon,
	&cg_shock_volume_voice,
	&cg_shock_volume_item,
	&cg_shock_volume_body,
	&cg_shock_volume_local,
	&cg_shock_volume_music,
	&cg_shock_volume_announcer,
	&cg_shock_volume_shellshock,
	&cg_shock_mouse,
	&cg_shock_mouse_maxpitchspeed,
	&cg_shock_mouse_maxyawspeed,
	&cg_shock_mouse_sensitivityscale,
	&cg_shock_mouse_fadeTime
};

/* the all-1.0 volume vector CG_EndShellShockSound restores (0x30060D10) */
static const float cg_shellShockNoVolumeChange[SND_CHANNEL_COUNT] = {
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};

/*
 * The view-kick path, 0x300608F8.  128 random unit-disc points, each at least
 * 0.5 away from its predecessor -- sub_3002E720 below is the generator that
 * printed them -- with the first three repeated at the end so the cubic can
 * read index+3 without wrapping.
 */
#define SHELLSHOCK_TABLE_SIZE   128

static const float cg_shellShockCameraTable[SHELLSHOCK_TABLE_SIZE + 3][2] = {
	{ -0.563551f, -0.004430f },
	{ -0.282062f, -0.757933f },
	{ 0.413047f, 0.244246f },
	{ 0.527895f, -0.723897f },
	{ -0.329777f, 0.669800f },
	{ -0.394248f, -0.763090f },
	{ 0.126207f, 0.497769f },
	{ 0.004986f, -0.014130f },
	{ 0.559913f, 0.112825f },
	{ -0.333089f, -0.573283f },
	{ 0.335404f, -0.107176f },
	{ -0.569060f, -0.213141f },
	{ -0.166676f, 0.785084f },
	{ 0.299592f, 0.037593f },
	{ -0.516867f, 0.510759f },
	{ 0.138009f, 0.034823f },
	{ -0.156167f, 0.829048f },
	{ -0.999458f, 0.020317f },
	{ 0.300029f, 0.252944f },
	{ 0.030215f, -0.295732f },
	{ -0.917362f, -0.050711f },
	{ 0.044177f, -0.269289f },
	{ 0.588424f, 0.362577f },
	{ -0.379913f, 0.619214f },
	{ 0.204432f, -0.019423f },
	{ 0.018499f, 0.468079f },
	{ 0.916187f, -0.247878f },
	{ 0.003799f, 0.108210f },
	{ 0.057363f, 0.606240f },
	{ 0.324595f, 0.158733f },
	{ -0.130529f, -0.183388f },
	{ 0.715672f, -0.363858f },
	{ 0.984258f, 0.106096f },
	{ -0.003313f, 0.345535f },
	{ -0.320351f, -0.573936f },
	{ 0.063455f, -0.003239f },
	{ -0.570173f, -0.759313f },
	{ 0.106456f, 0.283726f },
	{ -0.668163f, 0.142388f },
	{ -0.501119f, -0.720006f },
	{ -0.253281f, 0.524032f },
	{ -0.064084f, -0.165943f },
	{ -0.194672f, 0.433550f },
	{ -0.281800f, -0.417744f },
	{ 0.045786f, 0.402986f },
	{ 0.105064f, -0.558937f },
	{ 0.312244f, 0.688318f },
	{ -0.263294f, -0.256811f },
	{ 0.659186f, 0.070672f },
	{ 0.093625f, -0.046812f },
	{ -0.875020f, 0.288509f },
	{ 0.329359f, 0.105941f },
	{ -0.181309f, 0.259865f },
	{ 0.261597f, -0.074070f },
	{ -0.296082f, 0.031858f },
	{ 0.038584f, 0.565947f },
	{ -0.253445f, -0.717865f },
	{ -0.211836f, 0.336521f },
	{ 0.890123f, 0.004950f },
	{ -0.979825f, -0.170790f },
	{ 0.045346f, 0.022240f },
	{ -0.345796f, 0.522712f },
	{ 0.108525f, 0.165424f },
	{ -0.572796f, -0.473399f },
	{ 0.368605f, -0.865844f },
	{ 0.075571f, -0.327703f },
	{ -0.466353f, -0.565594f },
	{ -0.358837f, 0.610302f },
	{ 0.603884f, 0.440023f },
	{ 0.002465f, -0.144449f },
	{ -0.294915f, 0.799970f },
	{ -0.028347f, -0.112071f },
	{ -0.009472f, 0.686061f },
	{ 0.071150f, 0.019910f },
	{ 0.962690f, 0.024926f },
	{ 0.309208f, 0.871549f },
	{ -0.123782f, -0.312301f },
	{ -0.433055f, -0.895981f },
	{ 0.962495f, -0.263777f },
	{ -0.511460f, -0.359478f },
	{ -0.044013f, 0.020210f },
	{ -0.109340f, -0.761230f },
	{ 0.171003f, -0.107461f },
	{ 0.418912f, 0.435294f },
	{ 0.444940f, -0.139643f },
	{ 0.518574f, 0.365965f },
	{ -0.506997f, 0.655597f },
	{ 0.510525f, 0.508961f },
	{ -0.296173f, -0.675837f },
	{ 0.851332f, 0.307192f },
	{ -0.008474f, -0.188744f },
	{ 0.552703f, 0.427086f },
	{ 0.080334f, -0.002805f },
	{ 0.035656f, 0.610991f },
	{ 0.770593f, 0.398874f },
	{ -0.522137f, 0.324362f },
	{ 0.006045f, 0.042788f },
	{ 0.482456f, 0.848994f },
	{ 0.226058f, -0.522367f },
	{ -0.674606f, -0.547814f },
	{ -0.441998f, 0.598840f },
	{ -0.183957f, -0.270234f },
	{ 0.518850f, 0.634946f },
	{ 0.430386f, 0.125257f },
	{ -0.185496f, -0.264459f },
	{ 0.023690f, 0.312978f },
	{ -0.444287f, 0.849928f },
	{ 0.291978f, -0.897679f },
	{ -0.045826f, -0.047128f },
	{ -0.114246f, 0.511975f },
	{ 0.738133f, 0.607667f },
	{ -0.786889f, -0.384057f },
	{ 0.182993f, 0.265086f },
	{ -0.399450f, -0.309031f },
	{ -0.482895f, 0.265662f },
	{ 0.059671f, 0.097760f },
	{ 0.793174f, -0.015972f },
	{ 0.201658f, 0.492445f },
	{ -0.707371f, -0.026190f },
	{ -0.320882f, 0.372280f },
	{ 0.572813f, -0.537255f },
	{ 0.337619f, 0.116293f },
	{ -0.606537f, 0.173373f },
	{ -0.166593f, -0.335112f },
	{ -0.583993f, 0.182916f },
	{ -0.573519f, -0.623348f },
	{ -0.392707f, 0.449474f },
	{ 0.151474f, 0.840401f },
	{ -0.563551f, -0.004430f },
	{ -0.282062f, -0.757933f },
	{ 0.413047f, 0.244246f },
};

/*
===============
sub_3002E5C0

Unnamed and uncalled: the floor-to-int companion of Q_ftol, biased
by -(0.5 - 2^-30) instead of +2^-30, and like it inline-asm round-to-nearest
(retail 0x3002E5C0: the double built with two immediate stores, fld / fsub
qword / fistp -- an (int) cast would go through ftol and truncate).  Inlined
into CG_UpdateShellShockCamera (0x3002F72D).
===============
*/
static int sub_3002E5C0( float f ) {
	double  bias = 0.49999999906867743;
	int     i;

	__asm fld f
	__asm fsub bias
	__asm fistp i

	return i;
}

/*
===============
sub_3002E5F0

Unnamed and uncalled -- inlined twice into CG_UpdateShellShockCamera
(0x3002F757, 0x3002F788).  The textbook Catmull-Rom cubic through four
samples.
===============
*/
static float sub_3002E5F0( float t, float y0, float y1, float y2, float y3 ) {
	float a0, a1, a2, a3;

	a0 = y3 - y2 + y1 - y0;
	a1 = y0 - y1 - a0;
	a2 = y2 - y0;
	a3 = y1;

	return ( ( a0 * t + a1 ) * t + a2 ) * t + a3;
}

/*
===============
sub_3002E630

Unnamed.  Picks a random point in the unit disc at least minDist
away from prev.  Only sub_3002E720 calls it, so this pair is the generator
that produced cg_shellShockCameraTable and nothing else.

The FSINCOS pair is cg_animtree.c's sub_30013520 inlined (0x3002E683).
===============
*/
static void sub_3002E630( const float *prev, float *out, float minDist ) {
	float angle, radius;
	float sinAngle, cosAngle;
	float dx, dy;
	float minDistSquared;

	minDistSquared = minDist * minDist;

	do {
		angle = ( (float)rand() / 32768.0f + (float)rand() / 32768.0f - 1.0f ) * 3.1415927f;
		cosAngle = (float)cos( angle );
		sinAngle = (float)sin( angle );

		radius = (float)rand() / 32768.0f;
		out[0] = radius * cosAngle;
		out[1] = radius * sinAngle;

		dx = out[0] - prev[0];
		dy = out[1] - prev[1];
	} while ( dx * dx + dy * dy < minDistSquared );
}

/*
===============
sub_3002E720

Unnamed, uncalled, and left in the retail build: the developer
command that printed cg_shellShockCameraTable's source text.
===============
*/
static void sub_3002E720( void ) {
	float table[SHELLSHOCK_TABLE_SIZE][2];
	unsigned int i;

	sub_3002E630( vec3_origin, table[0], 0.5f );
	for ( i = 1; i < SHELLSHOCK_TABLE_SIZE; i++ ) {
		sub_3002E630( table[i - 1], table[i], 0.5f );
	}

	for ( i = 0; i < SHELLSHOCK_TABLE_SIZE; i++ ) {
		Com_Printf( "\t{%f, %f},\n", table[i][0], table[i][1] );
	}
}

/*
===============
CG_PerturbCamera

Tilts cg.refdef.viewaxis by the two-axis view kick CG_UpdateShellShockCamera
left in cg.shellshock.viewDelta.  Called at the end of CG_DrawActiveFrame
(0x30033DBD).
===============
*/
void CG_PerturbCamera( void ) {
	vec3_t axis[3];
	vec3_t oldAxis[3];

	if ( cg.shellshock.viewDelta[0] == 0.0f && cg.shellshock.viewDelta[1] == 0.0f ) {
		return;
	}

	axis[0][0] = 1.0f;
	axis[0][1] = cg.shellshock.viewDelta[0];
	axis[0][2] = cg.shellshock.viewDelta[1];
	axis[2][0] = 0.0f;
	axis[2][1] = 0.0f;
	axis[2][2] = 1.0f;

	VectorNormalize( axis[0] );
	CrossProduct( axis[2], axis[0], axis[1] );
	VectorNormalize( axis[1] );
	CrossProduct( axis[0], axis[1], axis[2] );

	VectorCopy( cg.refdef.viewaxis[0], oldAxis[0] );
	VectorCopy( cg.refdef.viewaxis[1], oldAxis[1] );
	VectorCopy( cg.refdef.viewaxis[2], oldAxis[2] );
	MatrixMultiply( axis, oldAxis, cg.refdef.viewaxis );
}

/*
===============
CG_DrawShellShockSavedScreenBlend

The smeared-screen overlay: blend the previously saved frame over the current
one, then save the result for the next frame.  Returns whether the effect is
still running, which is what tells CG_DrawActive to keep the saved screen.
===============
*/
qboolean CG_DrawShellShockSavedScreenBlend( int startTime, int duration,
											shellshockParms_t *parmsIn ) {
	const shellshockParms_t *parms = parmsIn;
	int                 remaining;
	int                 blend;

	if ( !startTime || duration <= 0 ) {
		cg.shellshock.savedScreenBlend = 0;
		return qfalse;
	}

	remaining = startTime + duration - cg.time;
	if ( remaining <= 0 ) {
		cg.shellshock.savedScreenBlend = 0;
		return qfalse;
	}

	blend = parms->screenBlendTime;
	if ( remaining < parms->screenBlendFadeTime ) {
		blend = Q_ftol( (float)remaining / parms->screenBlendFadeTime * parms->screenBlendTime );
	}

	if ( cg.shellshock.savedScreenBlend ) {
		trap_R_BlendSavedScreen( blend );
	}
	trap_R_SaveScreen();

	cg.shellshock.savedScreenBlend = 1;
	return qtrue;
}

/*
===============
CG_SaveShellShockCvars

The shellshock_save console command.  Writes scripts/<name>.shock.
===============
*/
qboolean CG_SaveShellShockCvars( const char *name ) {
	fileHandle_t    f;
	char            buffer[65536];

	if ( !trap_Com_SaveCvarsToBuffer( cg_shockCvarNames, NUM_SHOCK_CVARS,
									  buffer, sizeof( buffer ) ) ) {
		return qfalse;
	}

	if ( trap_FS_FOpenFile( va( "scripts/%s.shock", name ), &f, FS_WRITE ) < 0 ) {
		return qfalse;
	}

	trap_FS_Write( buffer, strlen( buffer ), f );
	trap_FS_FCloseFile( f );

	return qtrue;
}

/*
===============
CG_LoadShellShockCvars

Reads scripts/<name>.shock back into the 27 cvars and republishes every one of
them.  Returns what Com_LoadCvarsFromBuffer returned; 0 when the file is
missing.
===============
*/
int CG_LoadShellShockCvars( const char *name ) {
	const char      *filename;
	fileHandle_t    f;
	int             len;
	char            *buffer;
	int             result;
	unsigned int    i;

	filename = va( "scripts/%s.shock", name );

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len < 0 ) {
		CG_Printf( "^1couldn't open '%s'\n", filename );
		return 0;
	}

	buffer = trap_Z_MallocInternal( len + 1 );
	trap_FS_Read( buffer, len, f );
	buffer[len] = 0;
	trap_FS_FCloseFile( f );

	result = trap_Com_LoadCvarsFromBuffer( cg_shockCvarNames, NUM_SHOCK_CVARS, buffer, filename );
	trap_Z_FreeInternal( buffer );

	for ( i = 0; i < NUM_SHOCK_CVARS; i++ ) {
		trap_Cvar_Update( cg_shockCvars[i] );
	}

	return result;
}

/*
===============
CG_SetShellShockParmsFromCvars

Resolves the 27 cvars into one parameter block.  CG_ConfigStringModified calls
it once per configstring 1100..1115, right after CG_LoadShellShockCvars has
filled the cvars from that shellshock's file (0x3002C87E / 0x3002C89A).
===============
*/
void CG_SetShellShockParmsFromCvars( shellshockParms_t *parmsIn ) {
	shellshockParms_t *parms = parmsIn;

	parms->screenBlendFadeTime = SHOCK_MSEC( cg_shock_screenBlendFadeTime );
	parms->screenBlendTime = SHOCK_MSEC( cg_shock_screenBlendTime );

	parms->lerpTime = 3000;
	parms->viewKickFreq = 0.001f / ( 0.001f > cg_shock_viewKickPeriod.value
									 ? 0.001f : cg_shock_viewKickPeriod.value );
	parms->viewKickRadius = cg_shock_viewKickRadius.value;

	parms->soundEnabled = cg_shock_sound.integer != 0;
	parms->soundFadeInTime = SHOCK_MSEC( cg_shock_soundFadeInTime );
	parms->soundFadeOutTime = SHOCK_MSEC( cg_shock_soundFadeOutTime );
	parms->soundLoopFadeTime = SHOCK_MSEC( cg_shock_soundLoopFadeTime );
	parms->soundLoopEndDelay = SHOCK_MSEC( cg_shock_soundLoopEndDelay );

	strncpy( parms->soundRoomType, cg_shock_soundRoomType.string,
			 sizeof( parms->soundRoomType ) - 1 );
	parms->soundRoomType[sizeof( parms->soundRoomType ) - 1] = 0;

	parms->soundWetLevel = Com_Clamp( 0.0f, 1.0f, cg_shock_soundWetLevel.value );
	parms->soundModEndDelay = SHOCK_MSEC( cg_shock_soundModEndDelay );

	parms->soundVolume[SND_CHANNEL_AUTO] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_auto.value );
	parms->soundVolume[SND_CHANNEL_MENU] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_menu.value );
	parms->soundVolume[SND_CHANNEL_WEAPON] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_weapon.value );
	parms->soundVolume[SND_CHANNEL_VOICE] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_voice.value );
	parms->soundVolume[SND_CHANNEL_ITEM] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_item.value );
	parms->soundVolume[SND_CHANNEL_BODY] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_body.value );
	parms->soundVolume[SND_CHANNEL_LOCAL] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_local.value );
	parms->soundVolume[SND_CHANNEL_MUSIC] = Com_Clamp( 0.0f, 1.0f, cg_shock_volume_music.value );
	parms->soundVolume[SND_CHANNEL_ANNOUNCER] =
		Com_Clamp( 0.0f, 1.0f, cg_shock_volume_announcer.value );
	parms->soundVolume[SND_CHANNEL_SHELLSHOCK] =
		Com_Clamp( 0.0f, 1.0f, cg_shock_volume_shellshock.value );

	parms->mouseEnabled = cg_shock_mouse.integer != 0;
	parms->mouseFadeTime = SHOCK_MSEC( cg_shock_mouse_fadeTime );
	parms->mouseMaxPitchSpeed = cg_shock_mouse_maxpitchspeed.value;
	parms->mouseMaxYawSpeed = cg_shock_mouse_maxyawspeed.value;
	parms->mouseSensitivityScale = cg_shock_mouse_sensitivityscale.value;
}

/*
===============
CG_EndShellShockSound
===============
*/
void CG_EndShellShockSound( void ) {
	const char *alias;

	trap_syscall_0xD8( cg_shellShockNoVolumeChange, 0 );
	trap_syscall_0xD9( "generic", 0.0f, 0 );

	if ( cg.shellshock.soundState ) {
		cg.shellshock.soundState = 0;
		alias = trap_Com_PickSoundAlias( "shellshock_end_abort" );
		trap_MSS_PlaySoundAlias( alias, ENTITYNUM_NONE, vec3_origin, 0 );
	}
}

/*
===============
CG_EndShellShockMouse
===============
*/
void CG_EndShellShockMouse( void ) {
	cg.shellshock.mouseSensitivityScale = 1.0f;
	trap_syscall_0xF0( 0.0f, 0.0f );
}

/*
===============
CG_EndShellShockCamera

No call site in 1.1 -- CG_EndShellShock and CG_UpdateShellShockCamera both
carry it inlined.
===============
*/
void CG_EndShellShockCamera( void ) {
	cg.shellshock.viewDelta[0] = 0.0f;
	cg.shellshock.viewDelta[1] = 0.0f;
}

/*
===============
CG_EndShellShock
===============
*/
void CG_EndShellShock( void ) {
	CG_EndShellShockSound();

	cg.shellshock.mouseSensitivityScale = 1.0f;
	trap_syscall_0xF0( 0.0f, 0.0f );

	cg.shellshock.viewDelta[0] = 0.0f;
	cg.shellshock.viewDelta[1] = 0.0f;

	trap_SetUserCmdInShellshock( qfalse );
}

/*
===============
CG_UpdateShellShockSound

Ducks every sound channel toward the shellshock's volume vector, applies the
room reverb, crossfades the looping sound in, and schedules the tail.
===============
*/
void CG_UpdateShellShockSound( shellshockParms_t *parmsIn, int msec, int duration ) {
	const shellshockParms_t *parms = parmsIn;
	float               lerp;
	float               volume[SND_CHANNEL_COUNT];
	int                 remaining;
	int                 i;
	const char          *loopAlias;
	const char          *loopSilentAlias;
	const char          *endAlias;
	float               loopFrac;
	int                 endTime;

	if ( !parms->soundEnabled ) {
		CG_EndShellShockSound();
		return;
	}

	remaining = parms->soundModEndDelay + parms->soundFadeOutTime + duration - msec;
	if ( remaining < parms->soundFadeOutTime ) {
		lerp = (float)remaining / parms->soundFadeOutTime;
	} else if ( msec < parms->soundFadeInTime ) {
		lerp = (float)msec / parms->soundFadeInTime;
	} else {
		lerp = 1.0f;
	}

	if ( lerp < 0.0f ) {
		lerp = 0.0f;
	}

	for ( i = 0; i < SND_CHANNEL_COUNT; i++ ) {
		volume[i] = ( parms->soundVolume[i] - 1.0f ) * lerp + 1.0f;
	}
	trap_syscall_0xD8( volume, 0 );

	if ( lerp != 0.0f ) {
		trap_syscall_0xD9( parms->soundRoomType, lerp * parms->soundWetLevel, 0 );
	} else {
		trap_syscall_0xD9( "generic", 0.0f, 0 );
	}

	remaining = parms->soundLoopEndDelay + parms->soundLoopFadeTime + duration - msec;
	if ( remaining > 0 ) {
		loopAlias = trap_Com_PickSoundAlias( "shellshock_loop" );
		loopSilentAlias = trap_Com_PickSoundAlias( "shellshock_loop_silent" );

		if ( parms->soundLoopFadeTime ) {
			loopFrac = 1.0f - (float)remaining / parms->soundLoopFadeTime;
			if ( loopFrac < 0.0f ) {
				loopFrac = 0.0f;
			}
		} else {
			loopFrac = lerp;
		}

		trap_MSS_PlayBlendedSoundAliases( loopAlias, loopSilentAlias,
										  loopFrac, ENTITYNUM_NONE, vec3_origin, 0 );
	}

	endTime = cg.time + duration + parms->soundLoopEndDelay - msec;
	if ( cg.time < endTime ) {
		if ( cg.shellshock.soundState ) {
			cg.shellshock.soundState = 0;
			endAlias = trap_Com_PickSoundAlias( "shellshock_end_abort" );
			trap_MSS_PlaySoundAlias( endAlias, ENTITYNUM_NONE, vec3_origin, 0 );
		}
	} else if ( endTime != cg.shellshock.soundState ) {
		cg.shellshock.soundState = endTime;
		endAlias = trap_Com_PickSoundAlias( "shellshock_end" );
		trap_MSS_PlaySoundAlias( endAlias, ENTITYNUM_NONE, vec3_origin,
								 cg.time - endTime );
	}
}

/*
===============
CG_UpdateShellShockMouse

cg.shellshock.mouseSensitivityScale is what CL_MouseMove reads back; trap 240
carries the pitch and yaw speed caps into the same place.
===============
*/
void CG_UpdateShellShockMouse( shellshockParms_t *parmsIn, int msec, int duration ) {
	const shellshockParms_t *parms = parmsIn;
	int                 remaining;
	float               lerp;
	float               scale;
	float               recip;

	if ( !parms->mouseEnabled ) {
		cg.shellshock.mouseSensitivityScale = 1.0f;
		trap_syscall_0xF0( 0.0f, 0.0f );
		return;
	}

	remaining = duration - msec;
	if ( remaining < parms->mouseFadeTime ) {
		if ( remaining <= 0 ) {
			CG_EndShellShockMouse();
			return;
		}

		lerp = (float)remaining / parms->mouseFadeTime;
		if ( lerp != 1.0f ) {
			scale = ( parms->mouseSensitivityScale - 1.0f ) * lerp + 1.0f;
			cg.shellshock.mouseSensitivityScale = scale;

			recip = 1.0f / scale;
			trap_syscall_0xF0( parms->mouseMaxPitchSpeed * recip,
							   parms->mouseMaxYawSpeed * recip );
			return;
		}
	}

	cg.shellshock.mouseSensitivityScale = parms->mouseSensitivityScale;
	trap_syscall_0xF0( parms->mouseMaxPitchSpeed, parms->mouseMaxYawSpeed );
}

/*
===============
CG_UpdateShellShockCamera

Walks cg_shellShockCameraTable at viewKickFreq cycles per millisecond,
Catmull-Rom between the samples, and scales the result by a 3t^2-2t^3 ramp that
decays over the last lerpTime milliseconds.  duration seeds the table phase, so
two shellshocks of different lengths do not kick identically.
===============
*/
void CG_UpdateShellShockCamera( shellshockParms_t *parmsIn, int msec, int duration ) {
	const shellshockParms_t *parms = parmsIn;
	int                 remaining;
	float               ramp;
	float               amplitude;
	float               phase;
	float               frac;
	int                 index;
	const float         ( *p )[2];

	remaining = duration - msec;
	if ( remaining <= 0 ) {
		cg.shellshock.viewDelta[0] = 0.0f;
		cg.shellshock.viewDelta[1] = 0.0f;
		return;
	}

	if ( remaining < parms->lerpTime ) {
		ramp = (float)remaining / parms->lerpTime;
	} else {
		ramp = 1.0f;
	}
	amplitude = ( 3.0f - ( ramp + ramp ) ) * ramp * ramp * parms->viewKickRadius;

	phase = msec * parms->viewKickFreq;
	index = sub_3002E5C0( phase );
	frac = phase - index;

	index = ( duration * 61 + index ) & ( SHELLSHOCK_TABLE_SIZE - 1 );
	p = &cg_shellShockCameraTable[index];

	cg.shellshock.viewDelta[0] = sub_3002E5F0( frac, p[0][0], p[1][0], p[2][0], p[3][0] ) * amplitude;
	cg.shellshock.viewDelta[1] = sub_3002E5F0( frac, p[0][1], p[1][1], p[2][1], p[3][1] ) * amplitude;
}

/*
===============
CG_UpdateShellShock
===============
*/
void CG_UpdateShellShock( int startTime, shellshockParms_t *parms, int duration ) {
	int msec;

	msec = cg.time - startTime;
	if ( !startTime || msec < 0 ) {
		CG_EndShellShock();
		return;
	}

	CG_UpdateShellShockSound( parms, msec, duration );
	CG_UpdateShellShockMouse( parms, msec, duration );
	CG_UpdateShellShockCamera( parms, msec, duration );

	trap_SetUserCmdInShellshock( msec < duration );
}

/*
===============
CG_UpdateShellShockSavedScreenBlend

Publishes the three fields CG_DrawSavedScreenBlend reads back.  No call site in
1.1: CG_DrawActiveFrame carries it inlined (0x30033CED).
===============
*/
void CG_UpdateShellShockSavedScreenBlend( shellshockParms_t *parms, int startTime, int duration ) {
	cg.shellshock.parms = parms;
	cg.shellshock.startTime = startTime;
	cg.shellshock.duration = duration;
}
