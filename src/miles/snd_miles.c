/*
 * miles/snd_miles.c
 *
 * Retail range 0x0044B930-0x00451960, 102 functions.
 *
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "mss32.h"

extern void *Com_PickSoundAlias( const char *name, int source );
extern int Com_UnloadSoundAliasSounds();
extern int FS_FOpenFileRead_Internal();
extern int Hunk_AllocAlignInternal();
extern int j__atol();
extern int FastRound();
extern int sub_524F40();

extern unsigned int __stdcall timeGetTime( void );

#define mss_digitalDriver       mss_digitalDriver
#define mss_3DProvider          mss_3DProvider
#define mss_driverRate          mss_driverRate
#define mss_driverBits          mss_driverBits
#define mss_driverChannels      mss_driverChannels
#define mss_num2DChannels       mss_num2DChannels
#define mss_numStreamChannels   mss_numStreamChannels

#define mss_max3DChannels       ( *(int *)&mss_max3DChannels )

#define MSS_CVAR( g )   ( (cvar_t *)(void *)( g ) )

#define MSS_MAX_3D              32
#define MSS_MAX_STREAM          13
#define MSS_MAX_2D              32
#define MSS_FIRST_3D            0
#define MSS_FIRST_STREAM        32
#define MSS_FIRST_2D            45
#define MSS_NUM_CHANNELS        77      /* 0x8E0890 .. 0x8E1968 */
#define MSS_NUM_BACKGROUND      5
#define MSS_FIRST_FREE_STREAM   5
#define MSS_NUM_ALIAS_CHANNELS  10

#define MSS_NO_ENTITY           0x400

#define MSS_FTOL( f )           ( (int)( (f) + 9.313225746154785e-10 ) )

typedef struct {
	int      entnum;        /* +0x00  0..1023, or MSS_NO_ENTITY             */
	int      channel;       /* +0x04  alias channel enum, doubles as the
	                                  replacement priority                  */
	int      touchTime;     /* +0x08  frame stamp a looping owner refreshes */
	int      endTime;       /* +0x0C  when this sound runs out              */
	float    volume;        /* +0x10                                        */
	int      baseRate;      /* +0x14  the sound file's own sample rate      */
	float    pitch;         /* +0x18                                        */
	void    *alias;         /* +0x1C                                        */
	void    *alias2;        /* +0x20  the far end of a blend                */
	float    blend;         /* +0x24  0 = alias, 1 = alias2                 */
	vec3_t   localOrigin;   /* +0x28  offset in the entity's own frame      */
	int      paused;        /* +0x34                                        */
} mssChannel_t;

typedef struct {
	int      spatialized;   /* +0x00 */
	vec3_t   origin;        /* +0x04 */
} mssStreamInfo_t;

typedef struct {
	float    target;
	float    rate;
} mssBackgroundFade_t;

static mssChannel_t        mss_channels[ MSS_NUM_CHANNELS ];
static int                 mss_2DHandles[ MSS_MAX_2D ];        /* 0x008E1968 */
static int                 mss_3DHandles[ MSS_MAX_3D ];        /* 0x008E19E8 */
static int                 mss_streamHandles[ MSS_MAX_STREAM ];/* 0x008E1A68 */
static mssStreamInfo_t     mss_streamInfo[ MSS_MAX_STREAM ];   /* 0x008E1A9C */
static mssBackgroundFade_t mss_bgFade[ MSS_NUM_BACKGROUND ];   /* 0x008E0814 */

static float               mss_channelFade[ MSS_NUM_ALIAS_CHANNELS ][3];

static vec3_t              mss_listenerOrigin;
static vec3_t              mss_listenerAxis[3];

#define MSS_2D_HANDLE( c )      mss_2DHandles[ (c) - MSS_FIRST_2D ]
#define MSS_3D_HANDLE( c )      mss_3DHandles[ (c) ]
#define MSS_STREAM_HANDLE( c )  mss_streamHandles[ (c) - MSS_FIRST_STREAM ]
#define MSS_STREAM_INFO( c )    mss_streamInfo[ (c) - MSS_FIRST_STREAM ]

#define AL_NAME( a )        ( *(const char **)( (char *)(a) +  0 ) )
#define AL_FILENAME( a )    ( *(const char **)( (char *)(a) +  4 ) )
#define AL_SOUND( a )       ( *(int **)       ( (char *)(a) + 12 ) )
#define AL_VOLMIN( a )      ( *(float *)      ( (char *)(a) + 20 ) )
#define AL_VOLMAX( a )      ( *(float *)      ( (char *)(a) + 24 ) )
#define AL_PITCHMIN( a )    ( *(float *)      ( (char *)(a) + 28 ) )
#define AL_PITCHMAX( a )    ( *(float *)      ( (char *)(a) + 32 ) )
#define AL_MINDIST( a )     ( *(float *)      ( (char *)(a) + 36 ) )
#define AL_MAXDIST( a )     ( *(float *)      ( (char *)(a) + 40 ) )
#define AL_CHANNEL( a )     ( *(int *)        ( (char *)(a) + 44 ) )
#define AL_TYPE( a )        ( *(int *)        ( (char *)(a) + 48 ) )
#define AL_LOOPING( a )     ( *(unsigned char *)( (char *)(a) + 52 ) )
#define AL_MASTER( a )      ( *(unsigned char *)( (char *)(a) + 53 ) )
#define AL_DUCKS( a )       ( *(unsigned char *)( (char *)(a) + 54 ) )
#define AL_STREAMFOUND( a ) ( *(unsigned char *)( (char *)(a) + 55 ) )
#define AL_DUCKVOL( a )     ( *(float *)      ( (char *)(a) + 56 ) )

#define AL_TYPE_SAMPLE      1
#define AL_TYPE_STREAM      2

#define SND_FORMAT( s )     ( ((int *)(s))[0] )
#define SND_DATA( s )       ( *(const void **)&((int *)(s))[1] )
#define SND_DATALEN( s )    ( ((unsigned int *)(s))[2] )
#define SND_RATE( s )       ( ((unsigned int *)(s))[3] )
#define SND_BITS( s )       ( ((int *)(s))[4] )
#define SND_CHANNELS( s )   ( ((int *)(s))[5] )
#define SND_SAMPLES( s )    ( ((unsigned int *)(s))[6] )
#define SND_BLOCKSIZE( s )  ( ((unsigned int *)(s))[7] )

#define mss_paused              mss_paused    /* 0x008E0778 */
#define mss_pauseTime           mss_pauseTime
#define mss_cpuPercent          mss_cpuPercent
#define mss_pendingRestore      mss_pendingRestore
#define mss_pendingRestoreSize  mss_pendingRestoreSize
#define mss_masterVolume        mss_masterVolume
#define mss_globalFadeCur       mss_globalFadeCur
#define mss_globalFadeTarget    ( *(float *)&mss_globalFadeTarget )
#define mss_globalFadeRate      mss_globalFadeRate
#define mss_ambientSlot         mss_ambientSlot
#define mss_roomType            mss_roomType
#define mss_wetLevel            ( *(float *)&mss_wetLevel )
#define mss_wetTarget           ( *(float *)&mss_wetTarget )
#define mss_wetRate             mss_wetRate
#define mss_listenerEntnum      mss_listenerEntnum
#define mss_time                mss_time
#define mss_loopFrameTime       mss_loopFrameTime
#define mss_anyMasters          mss_anyMasters
#define mss_timeScale           mss_timeScale

static int   MSS_UpdateBackgroundVolume( int msec, int slot );
static void  MSS_SpatializeStream( float *volume, int slot, float *pan );
static int   MSS_FindReplacableChannel( int first, int count, int entnum, int priority );
static void *MSS_StartBackground( void *alias, int slot, int msec );

typedef struct {
	const char *name;
	float       requested;
	float       actual;
	int         baseRate;
	float       rateScale;
} mssOverlay_t;

int  MSS_PauseSounds( void );
int  MSS_UnpauseSounds( void );
int  MSS_StopSounds( int flags );
int  MSS_Update( void );
void MSS_Restore( int buffer, int size );

extern char *FS_ShortOSFilePath( const char *fullpath );

/* ---- MSS_Alloc  0x0044B930 ---- CONFIRMED */
void __cdecl MSS_Alloc(void *this)
{
  Hunk_AllocAlignInternal(((unsigned int)this + 3) & 0xFFFFFFFC, 32);
}

/* ---- MSS_Free  0x0044B940 ---- CONFIRMED */
void MSS_Free()
{
  ;
}

/* ---- MSS_FileOpenCallback  0x0044B950 ---- CONFIRMED */
int __stdcall MSS_FileOpenCallback(char *Source, fileHandle_t *file)
{
  int Internal;

  Internal = FS_FOpenFileRead_Internal(Source, file, qtrue, 1);
  return Internal == -1 ? 0 : Internal;
}

/* ---- MSS_FileCloseCallback  0x0044B980 ---- CONFIRMED */
void __stdcall MSS_FileCloseCallback(fileHandle_t f)
{
  FS_FCloseFile(f);
}

/* ---- MSS_FileSeekCallback  0x0044B990 ---- */
int __stdcall MSS_FileSeekCallback( fileHandle_t f, int offset, int type )
{
	int origin;

	switch ( type ) {
	case AIL_FILE_SEEK_BEGIN:
		origin = FS_SEEK_SET;
		break;
	case AIL_FILE_SEEK_CURRENT:
		origin = FS_SEEK_CUR;
		break;
	case AIL_FILE_SEEK_END:
		origin = FS_SEEK_END;
		break;
	default:
		return 0;
	}

	FS_Seek( f, offset, origin );
	return FS_FTell( f );
}

/* ---- MSS_FileReadCallback  0x0044BA10 ---- CONFIRMED */
int __stdcall MSS_FileReadCallback(fileHandle_t f, void *Buffer, size_t ElementCount)
{
  return FS_Read(Buffer, ElementCount, f);
}

/* ---- MSS_Init2D  0x0044BA30 ---- */
int MSS_Init2D( void )
{
	int             rate;           /* ebp -- Hz                       */
	int             bytes;          /* edi -- bytes per sample, 1 or 2 */
	int             channels;       /* ebx -- 1 mono, 2 stereo         */
	const char     *channelName;

	switch ( MSS_CVAR( mss_khz )->integer ) {
	case 11:
		rate = 11025;               /* 0x2B11 */
		break;
	case 44:
		rate = 44100;               /* 0xAC44 */
		break;
	case 22:
		rate = 22050;               /* 0x5622 */
		break;
	default:
		Com_Printf( "invalid value %i for mss_khz, using 22 khz instead\n",
					MSS_CVAR( mss_khz )->integer );
		rate = 22050;
		break;
	}

	if ( MSS_CVAR( mss_bits )->integer == 8 ) {
		bytes = 1;
	} else {
		if ( MSS_CVAR( mss_bits )->integer != 16 ) {
			Com_Printf( "invalid value %i for mss_bits (should be 8 or 16), "
						"using 16 instead\n", MSS_CVAR( mss_bits )->integer );
		}
		bytes = 2;
	}

	channels    = MSS_CVAR( mss_stereo )->integer ? 2 : 1;
	channelName = ( channels == 2 ) ? "stereo" : "mono";

	Com_Printf( "Attempting %i kHz %i bit %s sound\n",
				rate / 1000, 8 * bytes, channelName );

	AIL_set_preference( 1, 77 );
	mss_digitalDriver = AIL_open_digital_driver( rate, bytes, channels, 0 );

	if ( !mss_digitalDriver ) {
		Com_Printf( "couldn't initialize 2D provider: %s\n", AIL_last_error() );
		return 0;
	}

	mss_num2DChannels     = 32;     /* 0x008E1B6C */
	mss_numStreamChannels = 13;     /* 0x008E1B74 */
	mss_driverRate        = rate;   /* 0x008E0768 */
	mss_driverBits        = 8 * bytes;
	mss_driverChannels    = channels;
	*(float *)&mss_timeScale = 1.0f;

	Com_Printf( "2D provider initialized at %i %i %i\n",
				rate, 8 * bytes, channels );
	return 1;
}
#if 0
int MSS_Init2D()
{
  int v0; // eax
  int v1; // ebp
  int v2; // eax
  int v3; // edi
  int v4; // esi
  const char *v5; // eax
  const char *error; // eax
  int v8; // [esp+0h] [ebp-10h]
  int v9; // [esp+4h] [ebp-Ch]
  int v10; // [esp+8h] [ebp-8h]

  v0 = *(_DWORD *)(mss_khz + 32);
  switch ( v0 )
  {
    case 11:
      v1 = 11025;
      break;
    case 22:
LABEL_5:
      v1 = 22050;
      break;
    case 44:
      v1 = 44100;
      break;
    default:
      Com_Printf("invalid value %i for mss_khz, using 22 khz instead\n", *(_DWORD *)(mss_khz + 32));
      goto LABEL_5;
  }
  v2 = *(_DWORD *)(mss_bits + 32);
  if ( v2 == 8 )
  {
    v3 = 1;
  }
  else
  {
    if ( v2 != 16 )
      Com_Printf("invalid value %i for mss_bits (should be 8 or 16), using 16 instead\n", *(_DWORD *)(mss_bits + 32));
    v3 = 2;
  }
  v4 = *(_DWORD *)(mss_stereo + 32);
  v5 = "stereo";
  if ( v4 == 0 )
    v5 = "mono";
  Com_Printf("Attempting %i kHz %i bit %s sound\n", v1 / 1000, 8 * v3, v5);
  AIL_set_preference(1, 77);
  mss_digitalDriver = AIL_open_digital_driver(v1, v3, (v4 != 0) + 1, 0);
  if ( mss_digitalDriver )
  {
    mss_num2DChannels = 32;
    mss_numStreamChannels = 13;
    mss_driverRate = v1;
    mss_driverBits = 8 * v3;
    mss_driverChannels = (v4 != 0) + 1;
    mss_timeScale = 1.0;
    Com_Printf("2D provider initialized at %i %i %i\n", v8, v9, v10);
    return 1;
  }
  else
  {
    error = (const char *)AIL_last_error();
    Com_Printf("couldn't initialize 2D provider: %s\n", error);
    return 0;
  }
}
#endif

/* ---- MSS_Init3DProvider  0x0044BB70 ---- CONFIRMED */
int __cdecl MSS_Init3DProvider(char *String2, char *String1)
{
  char *v2;
  int v3;
  int v4;
  const char *v5;
  int v6;
  const char *error;
  int v8;
  const char *v10;
  int v11;
  int v12;

  v2 = String1;
  v3 = 0;
  if ( String1 )
    Com_Printf("available 3D providers:\n");
  v4 = 0;
  v12 = 0;
  while ( AIL_enumerate_3D_providers(&v12, &v11, &String1) )
  {
    if ( v2 )
      Com_Printf("  %s\n", String1);
    if ( !_stricmp(String1, String2) )
      v4 = v11;
    if ( !_stricmp(String1, "Miles Fast 2D Positional Audio") )
      v3 = v11;
  }
  mss_3DProvider = 0;
  if ( v4 )
  {
    if ( AIL_open_3D_provider(v4) )
    {
      error = (const char *)AIL_last_error();
      Com_Printf("couldn't open 3D provider '%s': %s\n", String2, error);
      v6 = mss_3DProvider;
      v5 = String2;
    }
    else
    {
      v5 = String2;
      Com_Printf("using 3D provider '%s'\n", String2);
      v6 = v4;
      mss_3DProvider = v4;
    }
    if ( v6 )
      goto LABEL_16;
  }
  else
  {
    v5 = String2;
  }
  if ( !v3 )
    return 0;
  if ( v3 == v4 )
    return 0;
  if ( *v5 )
  {
    if ( _stricmp(v5, "Miles Fast 2D Positional Audio") )
      Com_Printf("trying to use 'Miles Fast 2D Positional Audio' instead of '%s'\n", v5);
  }
  if ( AIL_open_3D_provider(v3) )
  {
    v10 = (const char *)AIL_last_error();
    Com_Printf("couldn't open 3D provider '%s': %s\n", "Miles Fast 2D Positional Audio", v10);
  }
  else
  {
    Com_Printf("using 3D provider '%s'\n", "Miles Fast 2D Positional Audio");
    mss_3DProvider = v3;
    Cvar_Set2("mss_3d_provider", "Miles Fast 2D Positional Audio", qtrue);
  }
  v6 = mss_3DProvider;
  if ( !mss_3DProvider )
    return 0;
LABEL_16:
  AIL_3D_provider_attribute(v6, "Maximum supported samples", &mss_max3DChannels);
  v8 = mss_max3DChannels;
  if ( mss_max3DChannels > 32 )
  {
    v8 = 32;
    mss_max3DChannels = 32;
  }
  Com_Printf("%i max 3D channels\n", v8);
  AIL_set_3D_distance_factor(mss_3DProvider, 0.0254f);
  return 1;
}

/* ---- MSS_SetListener  0x0044BD60 ---- */
int __cdecl MSS_SetListener( int *axis, int *origin, int entnum )
{
	int i;

	if ( !mss_digitalDriver ) {
		return 0;
	}
	for ( i = 0 ; i < 9 ; i++ ) {
		((float *)mss_listenerAxis)[i] = *(float *)&axis[i];
	}
	for ( i = 0 ; i < 3 ; i++ ) {
		mss_listenerOrigin[i] = *(float *)&origin[i];
	}
	mss_listenerEntnum = entnum;
	return 0;
}

/* ---- MSS_InitChannels  0x0044BDE0 ---- */
int MSS_InitChannels( void )
{
	int i;

	if ( mss_num2DChannels > MSS_MAX_2D ) {
		mss_num2DChannels = MSS_MAX_2D;
	}
	if ( mss_max3DChannels > MSS_MAX_3D ) {
		mss_max3DChannels = MSS_MAX_3D;
	}
	if ( mss_numStreamChannels > MSS_MAX_STREAM ) {
		mss_numStreamChannels = MSS_MAX_STREAM;
	}

	for ( i = 0 ; i < mss_num2DChannels ; i++ ) {
		mss_2DHandles[i] = AIL_allocate_sample_handle( mss_digitalDriver );
		if ( !mss_2DHandles[i] ) {
			Com_Error( ERR_DROP,
				"\x15MILES 2D sound sample allocation failed on channel %i\n", i + 1 );
		}
	}

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		mss_3DHandles[i] = AIL_allocate_3D_sample_handle( mss_3DProvider );
		if ( !mss_3DHandles[i] ) {
			Com_Error( ERR_DROP,
				"\x15MILES 3D sound sample allocation failed on channel %i\n", i + 1 );
		}
	}

	mss_ambientSlot = 2;
	return mss_max3DChannels;
}

/* ---- MSS_Attenuate  0x0044BE80 ---- VERIFIED */
double __cdecl MSS_Attenuate(float distance, float minDist, float maxDist)
{
  double v3;
  double v5;

  v3 = distance - minDist;
  if ( v3 <= 0.0 )
    return 1.0;
  v5 = v3 / (maxDist - minDist);
  if ( v5 < 1.0 )
    return 1.0 - v5;
  else
    return 0.0;
}

/* ---- MSS_GetCurrent3DPosition  0x0044BED0 ---- */
int __cdecl MSS_GetCurrent3DPosition( int entnum, float *out, const float *localOffset )
{
	vec3_t entOrigin;       /* [esp+0x00] -- ONE vec3_t, not three floats */
	float  axis[9];         /* [esp+0x0C] */
	vec3_t result;

	VM_Call( cgvm, 13 , entnum, entOrigin, axis );

	result[0] = entOrigin[0] + axis[0] * localOffset[0]
	                         + axis[3] * localOffset[1]
	                         + axis[6] * localOffset[2];
	result[1] = entOrigin[1] + axis[1] * localOffset[0]
	                         + axis[4] * localOffset[1]
	                         + axis[7] * localOffset[2];
	result[2] = entOrigin[2] + axis[2] * localOffset[0]
	                         + axis[5] * localOffset[1]
	                         + axis[8] * localOffset[2];

	out[0] = result[0];
	out[1] = result[1];
	out[2] = result[2];
	return *(int *)&result[2];
}

/* ---- MSS_Set3DPosition  0x0044BF70 ---- */
int __cdecl MSS_Set3DPosition( const float *origin, int handle )
{
	vec3_t d;
	float  forward, left, up;

	VectorSubtract( origin, mss_listenerOrigin, d );

	forward = DotProduct( mss_listenerAxis[0], d );
	left    = DotProduct( mss_listenerAxis[1], d );
	up      = DotProduct( mss_listenerAxis[2], d );

	AIL_set_3D_position( handle, -left, up, forward );
	return *(int *)&forward;
}

/* ---- MSS_IsAliasChannel3D  0x0044C020 ---- HIGH */
BOOL __cdecl MSS_IsAliasChannel3D(int a1)
{
  return a1 != 1 && (a1 <= 5 || a1 > 9);
}

/* ---- MSS_SetChannelInfo  0x0044C040 ---- */
static void MSS_SetChannelInfo( const float *origin, int entnum, void *alias, int chan,
                                void *alias2, float blend, float volume, float pitch,
                                int baseRate, int lengthMs, int offsetMs )
{
	mssChannel_t *ch = &mss_channels[chan];
	int           aliasChan = AL_CHANNEL( alias );

	if ( entnum >= 0 && entnum < MSS_NO_ENTITY
		&& aliasChan != 1 && !( aliasChan > 5 && aliasChan <= 9 ) ) {
		vec3_t entOrigin;
		float  axis[9];
		vec3_t d;

		VM_Call( cgvm, 13, entnum, entOrigin, axis );
		VectorSubtract( origin, entOrigin, d );

		ch->localOrigin[0] = axis[0] * d[0] + axis[1] * d[1] + axis[2] * d[2];
		ch->localOrigin[1] = axis[3] * d[0] + axis[4] * d[1] + axis[5] * d[2];
		ch->localOrigin[2] = axis[6] * d[0] + axis[7] * d[1] + axis[8] * d[2];
	} else {
		ch->localOrigin[0] = 0.0f;
		ch->localOrigin[1] = 0.0f;
		ch->localOrigin[2] = 0.0f;
	}

	ch->entnum    = entnum;
	ch->channel   = aliasChan;
	ch->volume    = volume;
	ch->pitch     = pitch;
	ch->baseRate  = baseRate;
	ch->alias     = alias;
	ch->alias2    = alias2;
	ch->blend     = blend;
	ch->touchTime = mss_loopFrameTime;
	ch->endTime   = mss_time - offsetMs + lengthMs;

	ch->paused = ( mss_paused && aliasChan != 1 ) ? 1 : 0;
}

/* ---- MSS_CompareReplacableChannels  0x0044C1D0 ---- */
static int MSS_CompareReplacableChannels( int a, int b, int entnum )
{
	const mssChannel_t *ca = &mss_channels[a];
	const mssChannel_t *cb = &mss_channels[b];
	int                 d;

	if ( ca->entnum != cb->entnum ) {
		if ( cb->entnum == entnum ) {
			return -1;
		}
		if ( ca->entnum == entnum ) {
			return 1;
		}
	}
	d = cb->channel - ca->channel;
	if ( d ) {
		return d;
	}
	return cb->endTime - ca->endTime;
}

/* ---- MSS_IsChannelReplacable  0x0044C220 ---- */
static qboolean MSS_IsChannelReplacable( int chan, int priority )
{
	return ( mss_channels[chan].channel <= priority ) ? qtrue : qfalse;
}

/* ---- MSS_SpatializeStream  0x0044C240 ---- */
static void MSS_SpatializeStream( float *volume, int slot, float *pan )
{
	mssChannel_t *ch = &mss_channels[ MSS_FIRST_STREAM + slot ];
	vec3_t        dir;
	float         dist, minDist, maxDist, f, atten, left;

	VectorSubtract( mss_streamInfo[slot].origin, mss_listenerOrigin, dir );
	dist = VectorNormalize( dir );

	left    = DotProduct( mss_listenerAxis[1], dir );
	minDist = ( 1.0f - ch->blend ) * AL_MINDIST( ch->alias )
	        +          ch->blend   * AL_MINDIST( ch->alias2 );

	if ( dist - minDist > 0.0f ) {
		maxDist = ( 1.0f - ch->blend ) * AL_MAXDIST( ch->alias )
		        +          ch->blend   * AL_MAXDIST( ch->alias2 );
		f = ( dist - minDist ) / ( maxDist - minDist );
		atten = ( f < 1.0f ) ? 1.0f - f : 0.0f;
	} else {
		atten = 1.0f;
	}

	*volume = atten * *volume;
	*pan    = ( 1.0f - left ) * 0.5f;
}

/* ---- MSS_FindReplacableChannel  0x0044C350 ---- */
static int MSS_FindReplacableChannel( int first, int count, int entnum, int priority )
{
	int best = -1;
	int i;

	for ( i = first ; i < first + count ; i++ ) {
		const mssChannel_t *c = &mss_channels[i];
		int                 d;

		if ( c->channel > priority ) {
			continue;
		}
		if ( best < 0 ) {
			best = i;
			continue;
		}
		if ( c->entnum != mss_channels[best].entnum ) {
			if ( c->entnum == entnum ) {
				best = i;
				continue;
			}
			if ( mss_channels[best].entnum == entnum ) {
				continue;
			}
		}
		d = c->channel - mss_channels[best].channel;
		if ( !d ) {
			d = c->endTime - mss_channels[best].endTime;
			if ( !d ) {
				continue;
			}
		}
		if ( d < 0 ) {
			best = i;
		}
	}
	return best;
}

/* ---- MSS_Stop2DChannel  0x0044C3D0 ---- */
void MSS_Stop2DChannel( int chan )
{
	AIL_end_sample( MSS_2D_HANDLE( chan ) );
	mss_channels[chan].paused = 0;
}

/* ---- MSS_Pause2DChannel  0x0044C3F0 ---- */
void MSS_Pause2DChannel( int chan )
{
	AIL_stop_sample( MSS_2D_HANDLE( chan ) );
	mss_channels[chan].paused = 1;
}

/* ---- MSS_Unpause2DChannel  0x0044C410 ---- */
void MSS_Unpause2DChannel( int chan, int elapsed )
{
	AIL_resume_sample( MSS_2D_HANDLE( chan ) );
	mss_channels[chan].endTime += elapsed;
	mss_channels[chan].paused = 0;
}

/* ---- MSS_Is2DChannelFree  0x0044C440 ---- */
qboolean MSS_Is2DChannelFree( int chan )
{
	return ( !mss_channels[chan].paused
		&& AIL_sample_status( MSS_2D_HANDLE( chan ) ) == SMP_DONE ) ? qtrue : qfalse;
}

/* ---- MSS_FindFree2DChannel  0x0044C470 ---- */
int MSS_FindFree2DChannel( int entnum, int priority )
{
	int i, chan;

	for ( i = 0 ; i < mss_num2DChannels ; i++ ) {
		if ( !mss_channels[ MSS_FIRST_2D + i ].paused
			&& AIL_sample_status( mss_2DHandles[i] ) == SMP_DONE ) {
			return MSS_FIRST_2D + i;
		}
	}

	chan = MSS_FindReplacableChannel( MSS_FIRST_2D, mss_num2DChannels, entnum, priority );
	if ( chan > 0 ) {
		AIL_end_sample( MSS_2D_HANDLE( chan ) );
		mss_channels[chan].paused = 0;
	}
	return chan;
}

/* ---- MSS_Stop3DChannel  0x0044C510 ---- */
void MSS_Stop3DChannel( int chan )
{
	AIL_end_3D_sample( MSS_3D_HANDLE( chan ) );
	mss_channels[chan].paused = 0;
}

/* ---- MSS_Pause3DChannel  0x0044C530 ---- */
void MSS_Pause3DChannel( int chan )
{
	AIL_stop_3D_sample( MSS_3D_HANDLE( chan ) );
	mss_channels[chan].paused = 1;
}

/* ---- MSS_Unpause3DChannel  0x0044C550 ---- */
void MSS_Unpause3DChannel( int chan, int elapsed )
{
	AIL_resume_3D_sample( MSS_3D_HANDLE( chan ) );
	mss_channels[chan].endTime += elapsed;
	mss_channels[chan].paused = 0;
}

/* ---- MSS_Is3DChannelFree  0x0044C580 ---- */
qboolean MSS_Is3DChannelFree( int chan )
{
	return ( !mss_channels[chan].paused
		&& AIL_3D_sample_status( MSS_3D_HANDLE( chan ) ) == SMP_DONE ) ? qtrue : qfalse;
}

/* ---- MSS_FindFree3DChannel  0x0044C5B0 ---- */
int MSS_FindFree3DChannel( int entnum, int priority )
{
	int i, chan;

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( !mss_channels[i].paused
			&& AIL_3D_sample_status( mss_3DHandles[i] ) == SMP_DONE ) {
			return i;
		}
	}

	chan = MSS_FindReplacableChannel( MSS_FIRST_3D, mss_max3DChannels, entnum, priority );
	if ( chan > 0 ) {
		AIL_end_3D_sample( MSS_3D_HANDLE( chan ) );
		mss_channels[chan].paused = 0;
	}
	return chan;
}

/* ---- MSS_StopStreamChannel  0x0044C640 ---- */
int MSS_StopStreamChannel( int chan )
{
	AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
	MSS_STREAM_HANDLE( chan ) = 0;
	mss_channels[chan].paused = 0;
	return 0;
}

/* ---- MSS_PauseStreamChannel  0x0044C670 ---- */
void MSS_PauseStreamChannel( int chan )
{
	AIL_pause_stream( MSS_STREAM_HANDLE( chan ), 1 );
	mss_channels[chan].paused = 1;
}

/* ---- MSS_UnpauseStreamChannel  0x0044C6A0 ---- */
void MSS_UnpauseStreamChannel( int chan, int elapsed )
{
	AIL_pause_stream( MSS_STREAM_HANDLE( chan ), 0 );
	mss_channels[chan].endTime += elapsed;
	mss_channels[chan].paused = 0;
}

/* ---- MSS_IsStreamChannelFree  0x0044C6D0 ---- */
int MSS_IsStreamChannelFree( int chan )
{
	if ( !MSS_STREAM_HANDLE( chan ) ) {
		return 1;
	}
	if ( !mss_channels[chan].paused
		&& AIL_stream_status( MSS_STREAM_HANDLE( chan ) ) == SMP_DONE ) {
		AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
		MSS_STREAM_HANDLE( chan ) = 0;
		return 1;
	}
	return 0;
}

/* ---- MSS_FindFreeStreamChannel  0x0044C720 ---- */
int MSS_FindFreeStreamChannel( int entnum, int priority )
{
	int i, chan;

	for ( i = MSS_FIRST_FREE_STREAM ; i < mss_numStreamChannels ; i++ ) {
		if ( !mss_streamHandles[i] ) {
			return MSS_FIRST_STREAM + i;
		}
		if ( !mss_channels[ MSS_FIRST_STREAM + i ].paused
			&& AIL_stream_status( mss_streamHandles[i] ) == SMP_DONE ) {
			AIL_close_stream( mss_streamHandles[i] );
			mss_streamHandles[i] = 0;
			return MSS_FIRST_STREAM + i;
		}
	}

	chan = MSS_FindReplacableChannel( MSS_FIRST_STREAM + MSS_FIRST_FREE_STREAM,
									  mss_numStreamChannels - MSS_FIRST_FREE_STREAM,
									  entnum, priority );
	if ( chan > 0 ) {
		AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
		MSS_STREAM_HANDLE( chan ) = 0;
		mss_channels[chan].paused = 0;
	}
	return chan;
}

/* ---- MSS_StopEntityChannel  0x0044C7E0 ---- */
int MSS_StopEntityChannel( int entnum, int aliasChan )
{
	int i, chan, h;

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( mss_channels[i].entnum == entnum && mss_channels[i].channel == aliasChan
			&& ( mss_channels[i].paused
				|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE ) ) {
			AIL_end_3D_sample( mss_3DHandles[i] );
			mss_channels[i].paused = 0;
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		if ( mss_channels[chan].entnum != entnum || mss_channels[chan].channel != aliasChan ) {
			continue;
		}
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		AIL_close_stream( h );
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			mss_channels[chan].paused = 0;
		}
		MSS_STREAM_HANDLE( chan ) = 0;
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( mss_channels[chan].entnum == entnum && mss_channels[chan].channel == aliasChan
			&& ( mss_channels[chan].paused
				|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE ) ) {
			AIL_end_sample( MSS_2D_HANDLE( chan ) );
			mss_channels[chan].paused = 0;
		}
	}
	return mss_num2DChannels + MSS_FIRST_2D;
}

/* ---- MSS_SampleType  0x0044C950 ---- CONFIRMED */
int __cdecl MSS_SampleType(int a1, int a2, int a3)
{
  if ( a2 == 1 )
  {
    if ( a1 == 17 )
      return 5;
    else
      return a3 > 8;
  }
  else if ( a1 == 17 )
  {
    return 7;
  }
  else
  {
    return (a3 > 8) + 2;
  }
}

/* ---- MSS_StartAlias2DSample  0x0044C980 ---- */
int MSS_StartAlias2DSample( int *outChan, void *alias, void *alias2, float blend,
                            int entnum, float volume, float pitch, int startMs,
                            float startFraction )
{
	int    chan, h, rate, totalMs, offsetMs;
	int   *snd;
	int    aliasChan = AL_CHANNEL( alias );

	chan = MSS_FindFree2DChannel( entnum, aliasChan );
	if ( outChan ) {
		*outChan = chan;
	}
	if ( chan < 0 ) {
		return 0;
	}

	h   = MSS_2D_HANDLE( chan );
	snd = AL_SOUND( alias );

	AIL_init_sample( h );
	AIL_set_sample_type( h, MSS_SampleType( SND_FORMAT( snd ), SND_CHANNELS( snd ),
											SND_BITS( snd ) ), 0 );
	AIL_set_sample_address( h, SND_DATA( snd ), SND_DATALEN( snd ) );
	AIL_set_sample_adpcm_block_size( h, SND_BLOCKSIZE( snd ) );

	rate = MSS_FTOL( (double)SND_RATE( snd ) * mss_timeScale * pitch );
	AIL_set_sample_playback_rate( h, rate );

	AIL_set_sample_volume_pan( h,
		mss_masterVolume * mss_channelFade[aliasChan][0] * volume, 0.5f );
	AIL_set_sample_loop_count( h, AL_LOOPING( alias ) == 0 );
	AIL_set_sample_reverb_levels( h, 1.0f, mss_wetLevel );

	totalMs = 0;
	AIL_sample_ms_position( h, &totalMs, 0 );

	if ( startFraction != 0.0f ) {
		offsetMs = MSS_FTOL( (double)totalMs * startFraction );
	} else {
		offsetMs = startMs;
	}

	if ( offsetMs ) {
		AIL_set_sample_ms_position( h, offsetMs );
		if ( !mss_paused || aliasChan == 1 ) {
			AIL_resume_sample( h );
		}
	} else if ( !mss_paused || aliasChan == 1 ) {
		AIL_start_sample( h );
	}

	if ( AL_LOOPING( alias ) ) {
		totalMs = 0;
	}

	MSS_SetChannelInfo( 0, entnum, alias, chan, alias2, blend, volume, pitch,
						SND_RATE( snd ), totalMs, offsetMs );
	return totalMs;
}

/* ---- MSS_StartAlias3DSample  0x0044CB90 ---- */
int MSS_StartAlias3DSample( int *outChan, void *alias, const float *origin, void *alias2,
                            float blend, int entnum, float volume, float pitch,
                            int startMs, float startFraction )
{
	int    chan, h, rate, totalMs, offsetMs;
	int   *snd;
	int    aliasChan = AL_CHANNEL( alias );
	float  minDist, maxDist, dist, f, atten;
	vec3_t d;

	chan = MSS_FindFree3DChannel( entnum, aliasChan );
	if ( outChan ) {
		*outChan = chan;
	}
	if ( chan < 0 ) {
		return 0;
	}

	h   = MSS_3D_HANDLE( chan );
	snd = AL_SOUND( alias );

	minDist = ( 1.0f - blend ) * AL_MINDIST( alias ) + blend * AL_MINDIST( alias2 );
	maxDist = ( 1.0f - blend ) * AL_MAXDIST( alias ) + blend * AL_MAXDIST( alias2 );

	AIL_set_3D_sample_info( h, snd );

	VectorSubtract( mss_listenerOrigin, origin, d );
	dist = (float)sqrt( d[0] * d[0] + d[1] * d[1] + d[2] * d[2] );
	if ( dist - minDist > 0.0f ) {
		f = ( dist - minDist ) / ( maxDist - minDist );
		atten = ( f < 1.0f ) ? 1.0f - f : 0.0f;
	} else {
		atten = 1.0f;
	}

	AIL_set_3D_sample_volume( h,
		mss_masterVolume * ( atten * mss_channelFade[aliasChan][0] * volume ) );
	AIL_set_3D_sample_distances( h, maxDist, maxDist );

	rate = MSS_FTOL( (float)AIL_3D_sample_playback_rate( h ) * mss_timeScale * pitch );
	AIL_set_3D_sample_playback_rate( h, rate );

	MSS_Set3DPosition( origin, h );
	AIL_set_3D_sample_loop_count( h, AL_LOOPING( alias ) == 0 );
	AIL_set_3D_sample_effects_level( h, mss_wetLevel );

	totalMs = MSS_FTOL( (float)( 1000 * SND_SAMPLES( snd ) ) * mss_timeScale / (float)rate );

	if ( startFraction != 0.0f ) {
		offsetMs = MSS_FTOL( (float)totalMs * startFraction );
	} else {
		offsetMs = startMs;
	}

	if ( offsetMs ) {
		AIL_set_3D_sample_offset( h,
			MSS_FTOL( (float)SND_DATALEN( snd ) * startFraction ) );
		if ( !mss_paused ) {
			AIL_resume_3D_sample( h );
		}
	} else if ( !mss_paused ) {
		AIL_start_3D_sample( h );
	}

	if ( AL_LOOPING( alias ) ) {
		totalMs = 0;
	}

	MSS_SetChannelInfo( origin, entnum, alias, chan, alias2, blend, volume, pitch,
						SND_RATE( snd ), totalMs, offsetMs );
	return totalMs;
}

/* ---- MSS_StartAliasSample  0x0044CED0 ---- */
int MSS_StartAliasSample( int startMs, int entnum, void *alias, void *alias2,
                          int *outChan, float blend, const float *origin,
                          float volume, float pitch )
{
	int aliasChan;

	if ( !AL_SOUND( alias ) ) {
		Com_DPrintf(
			"Tried to play sound '%s' from alias '%s', but it was not successfully loaded.\n",
			AL_FILENAME( alias ), AL_NAME( alias ) );
		if ( outChan ) {
			*outChan = -1;
		}
		return 0;
	}

	aliasChan = AL_CHANNEL( alias );
	if ( aliasChan == 1 || ( aliasChan > 5 && aliasChan <= 9 ) ) {
		return MSS_StartAlias2DSample( outChan, alias, alias2, blend, entnum,
									   volume, pitch, startMs, 0.0f );
	}
	return MSS_StartAlias3DSample( outChan, alias, origin, alias2, blend, entnum,
								   volume, pitch, startMs, 0.0f );
}

/* ---- MSS_StartAliasStreamOnChannel  0x0044CF50 ---- */
int MSS_StartAliasStreamOnChannel( void *alias, void *alias2, float blend, int entnum,
                                   const float *origin, float volume, float pitch,
                                   int startMs, float startFraction, int chan )
{
	const char *qpath;
	const char *osPath;
	int         h, rate, totalMs, offsetMs;
	int         aliasChan;
	float       pan;

	if ( !AL_STREAMFOUND( alias ) ) {
		Com_DPrintf(
			"Tried to play streamed sound '%s' from alias '%s', but it was not found at load time.\n",
			AL_FILENAME( alias ), AL_NAME( alias ) );
		return 0;
	}

	if ( MSS_STREAM_HANDLE( chan ) ) {
		AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
		MSS_STREAM_HANDLE( chan ) = 0;
	}

	qpath = va( "sound/%s", AL_FILENAME( alias ) );
	if ( MSS_CVAR( mss_q3fs )->integer ) {
		osPath = qpath;
	} else {
		osPath = FS_ShortOSFilePath( qpath );
	}

	h = AIL_open_stream( mss_digitalDriver, osPath, 0 );
	if ( !h ) {
		Com_Printf( "Couldn't play stream '%s' from alias '%s' - %s\n",
					qpath, AL_NAME( alias ), AIL_last_error() );
		return 0;
	}
	MSS_STREAM_HANDLE( chan ) = h;

	aliasChan = AL_CHANNEL( alias );
	rate = AIL_stream_playback_rate( h );
	AIL_set_stream_playback_rate( h,
		MSS_FTOL( (double)rate * mss_timeScale * pitch ) );
	AIL_set_stream_volume_pan( h,
		mss_masterVolume * mss_channelFade[aliasChan][0] * volume, 0.5f );
	AIL_set_stream_loop_count( h, AL_LOOPING( alias ) == 0 );
	AIL_set_stream_reverb_levels( h, 1.0f, mss_wetLevel );

	totalMs = 0;
	AIL_stream_ms_position( h, &totalMs, 0 );

	if ( startFraction != 0.0f ) {
		offsetMs = MSS_FTOL( (double)totalMs * startFraction );
	} else {
		offsetMs = startMs;
	}

	if ( offsetMs ) {
		AIL_set_stream_ms_position( h, offsetMs );
		if ( !mss_paused || aliasChan == 1 ) {
			AIL_pause_stream( h, 0 );
		}
	} else if ( !mss_paused || aliasChan == 1 ) {
		AIL_start_stream( h );
	}

	if ( AL_LOOPING( alias ) ) {
		totalMs = 0;
	}

	MSS_STREAM_INFO( chan ).spatialized =
		( aliasChan != 1 && ( aliasChan <= 5 || aliasChan > 9 ) ) ? 1 : 0;
	VectorCopy( origin, MSS_STREAM_INFO( chan ).origin );

	MSS_SetChannelInfo( origin, entnum, alias, chan, alias2, blend, volume, pitch,
						rate, totalMs, offsetMs );

	if ( MSS_STREAM_INFO( chan ).spatialized ) {
		MSS_SpatializeStream( &volume, chan - MSS_FIRST_STREAM, &pan );
		AIL_set_stream_volume_pan( h, mss_masterVolume * volume, pan );
	}
	return totalMs;
}

/* ---- MSS_StartAliasStream  0x0044D200 ---- */
int MSS_StartAliasStream( int *outChan, void *alias, int entnum, void *alias2,
                          float blend, const float *origin, float volume, float pitch,
                          int startMs, float startFraction )
{
	int chan;

	chan = MSS_FindFreeStreamChannel( entnum, AL_CHANNEL( alias ) );
	if ( outChan ) {
		*outChan = chan;
	}
	if ( chan < 0 ) {
		return 0;
	}
	return MSS_StartAliasStreamOnChannel( alias, alias2, blend, entnum, origin,
										  volume, pitch, startMs, startFraction, chan );
}

/* ---- MSS_ContinueLoopingSound  0x0044D250 ---- */
int MSS_ContinueLoopingSound( void *alias, void *alias2, float blend, int entnum,
                              const float *origin, int *outChan )
{
	int           i, chan;
	mssChannel_t *ch;
	float         volume;
	int           h;

	volume = ( ( 1.0f - blend ) * AL_VOLMIN( alias )
			 +          blend   * AL_VOLMIN( alias2 ) ) * 0.80000001f;

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		ch = &mss_channels[i];
		if ( ch->entnum == entnum
			&& ( ch->paused || AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE )
			&& AL_LOOPING( ch->alias )
			&& AL_NAME( ch->alias )  == AL_NAME( alias )
			&& AL_NAME( ch->alias2 ) == AL_NAME( alias2 ) ) {
			ch->volume = volume;
			AIL_set_3D_sample_playback_rate( mss_3DHandles[i],
				MSS_FTOL( (double)ch->baseRate * ch->pitch * mss_timeScale ) );
			MSS_Set3DPosition( origin, mss_3DHandles[i] );
			ch->touchTime = mss_loopFrameTime;
			ch->blend     = blend;
			if ( outChan ) {
				*outChan = i;
			}
			return 1;
		}
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		ch = &mss_channels[chan];
		if ( ch->entnum == entnum
			&& ( ch->paused || AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE )
			&& AL_LOOPING( ch->alias )
			&& AL_NAME( ch->alias )  == AL_NAME( alias )
			&& AL_NAME( ch->alias2 ) == AL_NAME( alias2 ) ) {
			ch->volume = volume;
			AIL_set_sample_playback_rate( MSS_2D_HANDLE( chan ),
				MSS_FTOL( (double)ch->baseRate * ch->pitch * mss_timeScale ) );
			ch->touchTime = mss_loopFrameTime;
			ch->blend     = blend;
			if ( outChan ) {
				*outChan = chan;
			}
			return 1;
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		ch = &mss_channels[chan];
		h  = MSS_STREAM_HANDLE( chan );
		if ( ch->entnum != entnum || !h ) {
			continue;
		}
		if ( !ch->paused && AIL_stream_status( h ) == SMP_DONE ) {
			AIL_close_stream( h );
			MSS_STREAM_HANDLE( chan ) = 0;
			continue;
		}
		if ( !AL_LOOPING( ch->alias )
			|| AL_NAME( ch->alias )  != AL_NAME( alias )
			|| AL_NAME( ch->alias2 ) != AL_NAME( alias2 ) ) {
			continue;
		}
		ch->volume = volume;
		AIL_set_stream_playback_rate( h,
			MSS_FTOL( (double)ch->baseRate * ch->pitch * mss_timeScale ) );
		VectorCopy( origin, MSS_STREAM_INFO( chan ).origin );
		ch->touchTime = mss_loopFrameTime;
		ch->blend     = blend;
		if ( outChan ) {
			*outChan = chan;
		}
		return 1;
	}
	return 0;
}

/* ---- MSS_ChoosePitchAndVolume  0x0044D5C0 ---- HIGH */
int __cdecl MSS_ChoosePitchAndVolume(float *a1, float *a2, float a3, float *a4, float *a5)
{
  double v5;
  int result;
  float v7;
  float v8;
  float v9;
  float v10;

  v5 = 1.0 - a3;
  v7 = v5 * a2[5] + a3 * a1[5];
  v9 = v5 * a2[6] + a3 * a1[6];
  v8 = v5 * a2[7] + a3 * a1[7];
  v10 = v5 * a2[8] + a3 * a1[8];
  *a4 = ((double)rand() * 0.000030517578 * (v9 - v7) + v7) * 0.80000001;
  result = rand();
  *a5 = (double)result * 0.000030517578 * (v10 - v8) + v8;
  return result;
}

/* ---- MSS_PlaySoundAlias_Internal  0x0044D670 ---- */
int __cdecl MSS_PlaySoundAlias_Internal( float *alias2, float *origin, int alias,
                                         int blendBits, int entnum, int *outChan,
                                         int startMs )
{
	float  blend = *(float *)&blendBits;
	float  volume, pitch;
	int    aliasChan;
	int    chan;

	if ( !mss_digitalDriver ) {
		return 0;
	}
	if ( outChan ) {
		*outChan = -1;
	}

	aliasChan = AL_CHANNEL( (void *)alias );
	if ( aliasChan != 1 && ( aliasChan <= 5 || aliasChan > 9 ) ) {
		float  maxDist;
		vec3_t d;

		maxDist = ( 1.0f - blend ) * AL_MAXDIST( (void *)alias )
				+          blend   * alias2[10];
		VectorSubtract( mss_listenerOrigin, origin, d );
		if ( !( maxDist * maxDist >= d[0] * d[0] + d[1] * d[1] + d[2] * d[2] ) ) {
			return 0;
		}
	}

	if ( MSS_ContinueLoopingSound( (void *)alias, alias2, blend, entnum, origin, outChan ) ) {
		return 0;
	}

	if ( aliasChan ) {
		MSS_StopEntityChannel( entnum, aliasChan );
	}

	MSS_ChoosePitchAndVolume( alias2, (float *)alias, blend, &volume, &pitch );

	if ( AL_TYPE( (void *)alias ) == AL_TYPE_SAMPLE ) {
		return MSS_StartAliasSample( startMs, entnum, (void *)alias, alias2, outChan,
									 blend, origin, volume, pitch );
	}
	if ( AL_TYPE( (void *)alias ) != AL_TYPE_STREAM ) {
		return 0;
	}
	chan = 0;
	(void)chan;
	return MSS_StartAliasStream( outChan, (void *)alias, entnum, alias2, blend, origin,
								 volume, pitch, startMs, 0.0f );
}

/* ---- MSS_PlaySoundAlias  0x0044D7D0 ---- */
int __cdecl MSS_PlaySoundAlias( float *alias, int entnum, float *origin, int startMs )
{
	if ( !alias ) {
		return 0;
	}
	return MSS_PlaySoundAlias_Internal( alias, origin, (int)alias, 0, entnum, 0, startMs );
}

/* ---- MSS_ValidateSoundAliasBlend  0x0044D800 ---- HIGH */
int __cdecl MSS_ValidateSoundAliasBlend(_DWORD *a1, _DWORD *a2, int a3)
{
  char v4;

  if ( a1 == a2 )
    return 1;
  if ( a1[1] != a2[1] )
  {
    if ( a3 )
      Com_Error(ERR_DROP, &byte_55F9A0, *a1, *a2);
    return 0;
  }
  v4 = *((_BYTE *)a1 + 52);
  if ( v4 != *((_BYTE *)a2 + 52) )
  {
    if ( a3 )
      Com_Error(ERR_DROP, &byte_55F938, *a1, *a2);
    return 0;
  }
  if ( a1[12] != a2[12] )
  {
    if ( a3 )
      Com_Error(ERR_DROP, &byte_55F8D0, *a1, *a2);
    return 0;
  }
  if ( a1[11] != a2[11] )
  {
    if ( a3 )
      Com_Error(ERR_DROP, &byte_55F870, *a1, *a2);
    return 0;
  }
  if ( *((_BYTE *)a1 + 53) != *((_BYTE *)a2 + 53) )
  {
    if ( a3 )
      Com_Error(ERR_DROP, &byte_55F810, *a1, *a2);
    return 0;
  }
  if ( *((_BYTE *)a1 + 54) != *((_BYTE *)a2 + 54) )
  {
    if ( a3 )
      Com_Error(ERR_DROP, &byte_55F7B0, *a1, *a2);
    return 0;
  }
  if ( v4 )
  {
    if ( !((*((float *)a1 + 7) == *((float *)a1 + 8)) | __UNORDERED__(*((float *)a1 + 7), *((float *)a1 + 8)))
      || !((*((float *)a2 + 7) == *((float *)a2 + 8)) | __UNORDERED__(*((float *)a2 + 7), *((float *)a2 + 8))) )
    {
      if ( a3 )
        Com_Error(ERR_DROP, &byte_55F738, *a1, *a2);
      return 0;
    }
    if ( !((*((float *)a1 + 5) == *((float *)a1 + 6)) | __UNORDERED__(*((float *)a1 + 5), *((float *)a1 + 6)))
      || !((*((float *)a2 + 5) == *((float *)a2 + 6)) | __UNORDERED__(*((float *)a2 + 5), *((float *)a2 + 6))) )
    {
      if ( a3 )
        Com_Error(ERR_DROP, &byte_55F6C0, *a1, *a2);
      return 0;
    }
  }
  return 1;
}

/* ---- MSS_PlayBlendedSoundAliases  0x0044D980 ---- CONFIRMED */
int __cdecl MSS_PlayBlendedSoundAliases(float *a1, _DWORD *a2, int a3, int a4, float *a5, int a6)
{
  if ( !a2 || !a1 )
    return 0;
  MSS_ValidateSoundAliasBlend(a2, a1, 1);
  return MSS_PlaySoundAlias_Internal(a1, a5, (int)a2, a3, a4, 0, a6);
}

/* ---- MSS_PlayLocalSoundAlias  0x0044D9C0 ---- VERIFIED */
int __cdecl MSS_PlayLocalSoundAlias( int source, int aliasName )
{
	float *alias;

	if ( !aliasName ) {
		return 0;
	}
	alias = (float *)Com_PickSoundAlias( (const char *) aliasName, source );
	if ( !alias ) {
		return 0;
	}
	return MSS_PlaySoundAlias_Internal( alias, mss_listenerOrigin, (int)alias, 0,
										mss_listenerEntnum, 0, 0 );
}

/* ---- MSS_StartBackground  0x0044DA00 ---- */
static void *MSS_StartBackground( void *alias, int slot, int msec )
{
	int           chan = MSS_FIRST_STREAM + slot;
	mssChannel_t *ch   = &mss_channels[chan];
	int           paused;
	int           aliasChan;
	float         volume, pitch;

	paused = ( cl_paused->integer != 0 );
	if ( paused != mss_paused ) {
		if ( paused ) {
			MSS_PauseSounds();
		} else {
			MSS_UnpauseSounds();
		}
	}

	aliasChan = AL_CHANNEL( alias );
	if ( aliasChan != 1 && ( aliasChan <= 5 || aliasChan > 9 ) ) {
		Com_Error( ERR_DROP,
			"\x15" "alias %s sound %s played as an ambient / music track uses a 3D channel type; should probably be channel 'local'\n",
			AL_NAME( alias ), AL_FILENAME( alias ) );
	}
	if ( AL_TYPE( alias ) != AL_TYPE_STREAM ) {
		Com_Error( ERR_DROP,
			"\x15" "alias %s sound %s played as an ambient / music track is not streamed; type must be 'streamed'\n",
			AL_NAME( alias ), AL_FILENAME( alias ) );
	}

	volume = ( (double)rand() * 0.000030517578
			 * ( AL_VOLMAX( alias ) - AL_VOLMIN( alias ) ) + AL_VOLMIN( alias ) )
			 * 0.80000001f;
	pitch  = (double)rand() * 0.000030517578
			 * ( AL_PITCHMAX( alias ) - AL_PITCHMIN( alias ) ) + AL_PITCHMIN( alias );

	if ( MSS_STREAM_HANDLE( chan ) ) {
		int wasLive = ( ch->paused
			|| AIL_stream_status( MSS_STREAM_HANDLE( chan ) ) != SMP_DONE );
		AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
		if ( wasLive ) {
			ch->paused = 0;
		}
		MSS_STREAM_HANDLE( chan ) = 0;
	}

	MSS_StartAliasStreamOnChannel( alias, alias, 0.0f, MSS_NO_ENTITY,
								   mss_listenerOrigin, volume, pitch, 0, 0.0f, chan );

	mss_bgFade[slot].target = ch->volume;
	if ( msec <= 0 ) {
		mss_bgFade[slot].rate = 0.0f;
	} else {
		mss_bgFade[slot].rate = ch->volume / (double)msec;
		ch->volume = 0.0f;
	}
	return &ch->volume;
}

/* ---- MSS_StopBackground  0x0044DB90 ---- */
void __cdecl MSS_StopBackground( int slot, int msec )
{
	int           chan = MSS_FIRST_STREAM + slot;
	mssChannel_t *ch   = &mss_channels[chan];

	if ( !MSS_STREAM_HANDLE( chan ) ) {
		return;
	}

	if ( ch->paused || AIL_stream_status( MSS_STREAM_HANDLE( chan ) ) != SMP_DONE ) {
		if ( msec ) {
			if ( mss_bgFade[slot].target > 0.0f ) {
				float rate = mss_bgFade[slot].target / (double)msec;
				mss_bgFade[slot].target = 0.0f;
				mss_bgFade[slot].rate   = -rate;
			}
		} else {
			AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
			MSS_STREAM_HANDLE( chan ) = 0;
			ch->paused = 0;
		}
	} else {
		AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
		MSS_STREAM_HANDLE( chan ) = 0;
	}
}

/* ---- MSS_PlayMusicAlias  0x0044DC40 ---- CONFIRMED */
float *__cdecl MSS_PlayMusicAlias(float *a1)
{
  float *result;

  result = (float *)mss_digitalDriver;
  if ( mss_digitalDriver )
  {
    if ( a1 )
    {
      result = (float *)MSS_IsStreamChannelFree(32);
      if ( result )
        return MSS_StartBackground(a1, 0, 0);
    }
  }
  return result;
}

/* ---- MSS_StopMusic  0x0044DC70 ---- CONFIRMED */
void __cdecl MSS_StopMusic(int a1)
{
  MSS_StopBackground(0, a1);
}

/* ---- MSS_PlayAmbientAlias  0x0044DC80 ---- */
int __cdecl MSS_PlayAmbientAlias( int aliasArg, int msec )
{
	void         *alias = (void *)aliasArg;
	int           slot, chan;
	mssChannel_t *ch;
	void         *cur;
	float         volume;

	if ( !mss_digitalDriver || !alias ) {
		return 0;
	}

	slot = mss_ambientSlot;
	chan = MSS_FIRST_STREAM + slot;
	ch   = &mss_channels[chan];

	if ( !MSS_IsStreamChannelFree( chan ) ) {
		cur = ch->alias;
		if ( AL_NAME( cur ) == AL_NAME( alias ) ) {
			return 1;
		}
		if ( AL_FILENAME( cur )  == AL_FILENAME( alias )
			&& AL_LOOPING( cur )  == AL_LOOPING( alias )
			&& AL_PITCHMIN( cur ) == AL_PITCHMIN( alias )
			&& AL_PITCHMAX( cur ) == AL_PITCHMAX( alias )
			&& AL_CHANNEL( cur )  == AL_CHANNEL( alias ) ) {
			ch->alias  = alias;
			ch->alias2 = alias;
			volume = ( (double)rand() * 0.000030517578
					 * ( AL_VOLMAX( alias ) - AL_VOLMIN( alias ) ) + AL_VOLMIN( alias ) )
					 * 0.80000001f;
			if ( msec ) {
				mss_bgFade[slot].rate = ( volume - mss_bgFade[slot].target ) / (double)msec;
			} else {
				mss_bgFade[slot].rate = 0.0f;
			}
			mss_bgFade[slot].target = volume;
			return 1;
		}
	}

	MSS_StopBackground( mss_ambientSlot, msec );
	mss_ambientSlot = 3 - mss_ambientSlot;
	MSS_StartBackground( alias, mss_ambientSlot, msec );
	return 1;
}

/* ---- MSS_BeginRawSamples  0x0044DD90 ---- CONFIRMED */
int __cdecl MSS_BeginRawSamples(int a1, int a2, int a3)
{
  int sample_handle;
  int v6;
  int v7;
  int v8;
  int result;

  sample_handle = AIL_allocate_sample_handle(mss_digitalDriver);
  dword_8E1B78 = sample_handle;
  if ( !sample_handle )
    Com_Error(ERR_DROP, &byte_55F5A8);
  dword_8E1B7C = a2;
  dword_8E1B80 = a1;
  dword_8E1B84 = a3;
  AIL_init_sample(sample_handle);
  if ( a3 == 1 )
    v6 = a1 != 1;
  else
    v6 = (a1 != 1) + 2;
  AIL_set_sample_type(dword_8E1B78, v6, 0);
  AIL_set_sample_playback_rate(dword_8E1B78, a2);
  AIL_set_sample_volume_pan(dword_8E1B78, *(float *)&mss_masterVolume, 0.5f);
  v7 = AIL_minimum_sample_buffer_size(mss_digitalDriver, a2, v6);
  v8 = 0x2000;
  dword_8E1BAC = 0x2000;
  if ( v7 > 0x2000 )
  {
    v8 = v7;
    dword_8E1BAC = v7;
  }
  dword_8E1B88 = Z_MallocInternal(32 * v8);
  result = 0;
  memset(byte_8E1B8C, 0, sizeof(byte_8E1B8C));
  dword_8E1BB4 = 0;
  dword_8E1BB8 = 0;
  dword_8E1BB0 = 0;
  dbl_8E1BC0 = 0.0;
  dbl_8E1BC8 = 1000.0 / (double)(dword_8E1B7C * dword_8E1B80 * dword_8E1B84);
  return result;
}

/* ---- MSS_EndRawSamples  0x0044DEC0 ---- CONFIRMED */
void MSS_EndRawSamples()
{
  if ( dword_8E1B78 )
  {
    AIL_end_sample(dword_8E1B78);
    AIL_release_sample_handle(dword_8E1B78);
    dword_8E1B78 = 0;
    free(dword_8E1B88);
  }
}

/* ---- MSS_RawSamplesTime  0x0044DF00 ---- CONFIRMED */
int MSS_RawSamplesTime()
{
  __int64 v0;

  LODWORD(v0) = dword_8E1B78;
  if ( dword_8E1B78 )
    return (unsigned __int64)((double)(unsigned int)AIL_sample_position(dword_8E1B78) * dbl_8E1BC8 + dbl_8E1BC0);
  return v0;
}

/* ---- MSS_UpdateRawSamples  0x0044DF40 ---- */
int MSS_UpdateRawSamples( void )
{
	int buf;

	if ( !dword_8E1B78 ) {
		return 0;
	}
	AIL_set_sample_volume_pan( dword_8E1B78, mss_masterVolume, 0.5f );

	if ( !byte_8E1B8C[ dword_8E1BB4 ] ) {
		return 0;
	}
	buf = AIL_sample_buffer_ready( dword_8E1B78 );
	if ( buf == -1 ) {
		return 0;
	}
	dbl_8E1BC0 = (double)dword_8E1BAC * dbl_8E1BC8 + dbl_8E1BC0;
	AIL_load_sample_buffer( dword_8E1B78, buf,
		(char *)dword_8E1B88 + dword_8E1BB4 * dword_8E1BAC, dword_8E1BAC );
	byte_8E1B8C[ dword_8E1BB4 ] = 0;
	dword_8E1BB4 = ( dword_8E1BB4 + 1 ) % 32;
	return dword_8E1BB4;
}

/* ---- MSS_RawSamples  0x0044DFF0 ---- */
int __cdecl MSS_RawSamples( int channels, int width, int rate, int samples, void *data )
{
	int remaining, n;

	if ( !mss_digitalDriver ) {
		return 0;
	}
	if ( !dword_8E1B78 ) {
		MSS_BeginRawSamples( width, rate, channels );
		if ( !dword_8E1B78 ) {
			return 0;
		}
	}

	remaining = channels * width * samples;
	while ( remaining ) {
		while ( byte_8E1B8C[ dword_8E1BB8 ] ) {
			MSS_Update();
		}
		n = dword_8E1BAC - dword_8E1BB0;
		if ( n > remaining ) {
			n = remaining;
		}
		Com_Memcpy( (char *)dword_8E1B88 + dword_8E1BAC * dword_8E1BB8 + dword_8E1BB0,
					data, n );
		data = (char *)data + n;
		remaining -= n;
		dword_8E1BB0 += n;
		if ( dword_8E1BB0 == dword_8E1BAC ) {
			dword_8E1BB0 = 0;
			byte_8E1B8C[ dword_8E1BB8 ] = 1;
			dword_8E1BB8 = ( dword_8E1BB8 + 1 ) % 32;
		}
	}
	return dword_8E1B78;
}

/* ---- MSS_FadeAllSounds  0x0044E0F0 ---- CONFIRMED */
void __cdecl MSS_FadeAllSounds(float a1, int a2)
{
  /* Retail 0x0044E0FE is a RAW DWORD copy: mss_globalFadeTarget is a float lvalue by macro, so converting instead of copying stores 1065353216.0f. */
  LODWORD( mss_globalFadeTarget ) = LODWORD( a1 );
  mss_globalFadeRate = a1 - mss_globalFadeCur;
  if ( a2 )
  {
    mss_globalFadeRate = mss_globalFadeRate / (double)a2;
  }
  else if ( (a1 == 0.0) | __UNORDERED__(a1, 0.0) )
  {
    MSS_StopSounds(0);
  }
}

/* ---- MSS_FadeSelectSounds  0x0044E140 ---- */
int __cdecl MSS_FadeSelectSounds( float *targets, int msec )
{
	int i;

	for ( i = 0 ; i < MSS_NUM_ALIAS_CHANNELS ; i++ ) {
		float rate = targets[i] - mss_channelFade[i][0];

		mss_channelFade[i][1] = targets[i];
		if ( msec ) {
			rate = rate / (double)msec;
		}
		mss_channelFade[i][2] = rate;
	}
	return (int)targets;
}

/* ---- MSS_PauseSounds  0x0044E2F0 ---- */
int MSS_PauseSounds( void )
{
	int chan, i, h;

	if ( !mss_digitalDriver || mss_paused ) {
		return mss_paused;
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( ( mss_channels[chan].paused
				|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE )
			&& AL_CHANNEL( mss_channels[chan].alias ) != 1 ) {
			AIL_stop_sample( MSS_2D_HANDLE( chan ) );
			mss_channels[chan].paused = 1;
		}
	}

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( mss_channels[i].paused
			|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE ) {
			AIL_stop_3D_sample( mss_3DHandles[i] );
			mss_channels[i].paused = 1;
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			if ( AL_CHANNEL( mss_channels[chan].alias ) != 1 ) {
				AIL_pause_stream( h, 1 );
				mss_channels[chan].paused = 1;
			}
		} else {
			AIL_close_stream( h );
			MSS_STREAM_HANDLE( chan ) = 0;
		}
	}

	mss_paused    = 1;
	mss_pauseTime = mss_time;
	return mss_time;
}

/* ---- MSS_UnpauseSounds  0x0044E470 ---- */
int MSS_UnpauseSounds( void )
{
	int chan, i, h, elapsed;

	if ( !mss_digitalDriver || !mss_paused ) {
		return mss_paused;
	}
	elapsed = mss_time - mss_pauseTime;

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( mss_channels[chan].paused ) {
			AIL_resume_sample( MSS_2D_HANDLE( chan ) );
			mss_channels[chan].endTime += elapsed;
			mss_channels[chan].paused = 0;
		}
	}

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( mss_channels[i].paused ) {
			AIL_resume_3D_sample( mss_3DHandles[i] );
			mss_channels[i].endTime += elapsed;
			mss_channels[i].paused = 0;
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused ) {
			AIL_pause_stream( h, 0 );
			mss_channels[chan].endTime += elapsed;
			mss_channels[chan].paused = 0;
		} else if ( AIL_stream_status( h ) == SMP_DONE ) {
			AIL_close_stream( h );
			MSS_STREAM_HANDLE( chan ) = 0;
		}
	}

	mss_paused    = 0;
	mss_pauseTime = 0;
	return 0;
}

/* ---- MSS_UpdateLoopingSounds  0x0044E600 ---- */
int __cdecl MSS_UpdateLoopingSounds( void )
{
	int chan, i, h;

	if ( !mss_digitalDriver || mss_paused ) {
		return 0;
	}

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( ( mss_channels[i].paused
				|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE )
			&& AL_LOOPING( mss_channels[i].alias )
			&& mss_channels[i].touchTime != mss_loopFrameTime ) {
			AIL_end_3D_sample( mss_3DHandles[i] );
			mss_channels[i].paused = 0;
		}
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( ( mss_channels[chan].paused
				|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE )
			&& AL_LOOPING( mss_channels[chan].alias )
			&& mss_channels[chan].touchTime != mss_loopFrameTime ) {
			AIL_end_sample( MSS_2D_HANDLE( chan ) );
			mss_channels[chan].paused = 0;
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			if ( !AL_LOOPING( mss_channels[chan].alias )
				|| mss_channels[chan].touchTime == mss_loopFrameTime ) {
				continue;
			}
			AIL_close_stream( h );
			mss_channels[chan].paused = 0;
		} else {
			AIL_close_stream( h );
		}
		MSS_STREAM_HANDLE( chan ) = 0;
	}

	mss_loopFrameTime = mss_time;
	return mss_loopFrameTime;
}

/* ---- MSS_UpdateBackgroundVolume  0x0044E7A0 ---- */
static int MSS_UpdateBackgroundVolume( int msec, int slot )
{
	int           chan = MSS_FIRST_STREAM + slot;
	mssChannel_t *ch   = &mss_channels[chan];
	float         v;

	v = (double)msec * mss_bgFade[slot].rate + ch->volume;

	if ( mss_bgFade[slot].rate > 0.0f ) {
		if ( v > mss_bgFade[slot].target ) {
			v = mss_bgFade[slot].target;
		}
	} else if ( v < mss_bgFade[slot].target ) {
		v = mss_bgFade[slot].target;
		if ( v == 0.0f ) {
			AIL_close_stream( MSS_STREAM_HANDLE( chan ) );
			MSS_STREAM_HANDLE( chan ) = 0;
			ch->paused = 0;
			return 0;
		}
	}

	ch->volume    = v;
	ch->touchTime = mss_loopFrameTime;
	return 1;
}

/* ---- MSS_UpdateVolume  0x0044E840 ---- */
void MSS_UpdateVolume( float *fade, int msec )
{
	fade[0] = (double)msec * fade[2] + fade[0];

	if ( fade[2] < 0.0f ) {
		if ( fade[0] < fade[1] ) {
			fade[0] = fade[1];
			fade[2] = 0.0f;
		}
	} else if ( fade[0] > fade[1] ) {
		fade[0] = fade[1];
		fade[2] = 0.0f;
	}
}

/* ---- MSS_UpdateMasterVolumes  0x0044E890 ---- */
void __cdecl MSS_UpdateMasterVolumes( int msec )
{
	cvar_t *volume = MSS_CVAR( mss_volume );
	float   clamped;
	int     i;

	for ( i = 0 ; i < MSS_NUM_ALIAS_CHANNELS ; i++ ) {
		mss_channelFade[i][0] = mss_channelFade[i][2] * (double)msec + mss_channelFade[i][0];
		if ( mss_channelFade[i][2] < 0.0f ) {
			if ( mss_channelFade[i][0] < mss_channelFade[i][1] ) {
				mss_channelFade[i][0] = mss_channelFade[i][1];
				mss_channelFade[i][2] = 0.0f;
			}
		} else if ( mss_channelFade[i][0] > mss_channelFade[i][1] ) {
			mss_channelFade[i][0] = mss_channelFade[i][1];
			mss_channelFade[i][2] = 0.0f;
		}
	}

	if ( mss_globalFadeRate == 0.0f ) {
		if ( !volume->modified ) {
			return;
		}
	} else {
		mss_globalFadeCur = (double)msec * mss_globalFadeRate + mss_globalFadeCur;
		if ( mss_globalFadeRate < 0.0f ) {
			if ( mss_globalFadeCur < mss_globalFadeTarget ) {
				mss_globalFadeCur  = mss_globalFadeTarget;
				mss_globalFadeRate = 0.0f;
			}
		} else if ( mss_globalFadeCur > mss_globalFadeTarget ) {
			mss_globalFadeCur  = mss_globalFadeTarget;
			mss_globalFadeRate = 0.0f;
		}
		if ( mss_globalFadeCur == 0.0f && mss_globalFadeRate == 0.0f ) {
			MSS_StopSounds( 0 );
		}
	}

	volume->modified = qfalse;
	if ( volume->value > 1.0f ) {
		clamped = 1.0f;
	} else if ( volume->value >= 0.0f ) {
		clamped = volume->value;
	} else {
		clamped = 0.0f;
	}
	mss_masterVolume = clamped * mss_globalFadeCur;
}

/* ---- MSS_AnyMasters  0x0044ED60 ---- */
int MSS_AnyMasters( void )
{
	int chan, i, h;

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( ( mss_channels[i].paused
				|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE )
			&& AL_MASTER( mss_channels[i].alias ) ) {
			return 1;
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			if ( AL_MASTER( mss_channels[chan].alias ) ) {
				return 1;
			}
		} else {
			AIL_close_stream( h );
			MSS_STREAM_HANDLE( chan ) = 0;
		}
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( ( mss_channels[chan].paused
				|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE )
			&& AL_MASTER( mss_channels[chan].alias ) ) {
			return 1;
		}
	}
	return 0;
}

/* ---- MSS_Update3DChannel  0x0044EEA0 ---- */
static void MSS_Update3DChannel( int chan )
{
	mssChannel_t *ch = &mss_channels[chan];
	int           h  = mss_3DHandles[chan];
	vec3_t        origin, d;
	float         minDist, maxDist, dist, f, atten, volume, ducked;

	MSS_GetCurrent3DPosition( ch->entnum, origin, ch->localOrigin );
	MSS_Set3DPosition( origin, h );

	minDist = ( 1.0f - ch->blend ) * AL_MINDIST( ch->alias )
	        +          ch->blend   * AL_MINDIST( ch->alias2 );

	VectorSubtract( mss_listenerOrigin, origin, d );
	dist = (float)sqrt( d[0] * d[0] + d[1] * d[1] + d[2] * d[2] );

	if ( dist - minDist > 0.0f ) {
		maxDist = ( 1.0f - ch->blend ) * AL_MAXDIST( ch->alias )
		        +          ch->blend   * AL_MAXDIST( ch->alias2 );
		f = ( dist - minDist ) / ( maxDist - minDist );
		atten = ( f < 1.0f ) ? 1.0f - f : 0.0f;
	} else {
		atten = 1.0f;
	}

	volume = atten * ch->volume;
	if ( mss_anyMasters && AL_DUCKS( ch->alias ) ) {
		ducked = ( 1.0f - ch->blend ) * AL_DUCKVOL( ch->alias )
		       +          ch->blend   * AL_DUCKVOL( ch->alias2 );
		if ( volume > ducked ) {
			volume = ducked;
		}
	}

	AIL_set_3D_sample_volume( h,
		mss_masterVolume * ( volume * mss_channelFade[ AL_CHANNEL( ch->alias ) ][0] ) );
}

/* ---- MSS_Update2DChannel  0x0044F070 ---- */
static void MSS_Update2DChannel( int msec, int chan )
{
	mssChannel_t *ch = &mss_channels[chan];
	float         volume = ch->volume;
	float         ducked;

	(void)msec;

	if ( mss_anyMasters && AL_DUCKS( ch->alias ) ) {
		ducked = ( 1.0f - ch->blend ) * AL_DUCKVOL( ch->alias )
		       +          ch->blend   * AL_DUCKVOL( ch->alias2 );
		if ( volume > ducked ) {
			volume = ducked;
		}
	}

	AIL_set_sample_volume_pan( MSS_2D_HANDLE( chan ),
		volume * mss_channelFade[ AL_CHANNEL( ch->alias ) ][0] * mss_masterVolume,
		0.5f );
}

/* ---- MSS_UpdateStreamChannel  0x0044F100 ---- */
static int MSS_UpdateStreamChannel( int msec, int chan )
{
	mssChannel_t *ch;
	int           h;
	float         volume, pan, ducked;

	if ( chan < MSS_FIRST_STREAM + MSS_NUM_BACKGROUND
		&& !MSS_UpdateBackgroundVolume( msec, chan - MSS_FIRST_STREAM ) ) {
		return 0;
	}

	ch     = &mss_channels[chan];
	h      = MSS_STREAM_HANDLE( chan );
	volume = ch->volume;
	pan    = 0.5f;

	if ( MSS_STREAM_INFO( chan ).spatialized ) {
		MSS_GetCurrent3DPosition( ch->entnum, MSS_STREAM_INFO( chan ).origin,
								  ch->localOrigin );
		MSS_SpatializeStream( &volume, chan - MSS_FIRST_STREAM, &pan );
	}

	if ( mss_anyMasters && AL_DUCKS( ch->alias ) ) {
		ducked = ( 1.0f - ch->blend ) * AL_DUCKVOL( ch->alias )
		       +          ch->blend   * AL_DUCKVOL( ch->alias2 );
		if ( volume > ducked ) {
			volume = ducked;
		}
	}

	AIL_set_stream_volume_pan( h,
		mss_masterVolume * ( volume * mss_channelFade[ AL_CHANNEL( ch->alias ) ][0] ),
		pan );
	return 1;
}

/* ---- MSS_UpdateAllChannels  0x0044F210 ---- */
static void MSS_UpdateAllChannels( int msec )
{
	int chan, i, h;

	mss_anyMasters = MSS_AnyMasters();

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( mss_channels[i].paused
			|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE ) {
			MSS_Update3DChannel( i );
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			MSS_UpdateStreamChannel( msec, chan );
		} else {
			AIL_close_stream( h );
			MSS_STREAM_HANDLE( chan ) = 0;
		}
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( mss_channels[chan].paused
			|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE ) {
			MSS_Update2DChannel( msec, chan );
		}
	}
}

static const char * const mss_roomTypeNames[26] = {
	"generic",      "paddedcell",   "room",        "bathroom",
	"livingroom",   "stoneroom",    "auditorium",  "concerthall",
	"cave",         "arena",        "hangar",      "carpetedhallway",
	"hallway",      "stonecorridor","alley",       "forest",
	"city",         "mountains",    "quarry",      "plain",
	"parkinglot",   "sewerpipe",    "underwater",  "drugged",
	"dizzy",        "psychotic"
};

/* ---- MSS_RoomTypeFromString  0x0044F340 ---- */
unsigned int __cdecl MSS_RoomTypeFromString( const char *name )
{
	unsigned int i;
	unsigned int index;

	if ( *name < '0' || *name > '9' ) {
		for ( i = 0 ; i < 26 ; i++ ) {
			if ( !_stricmp( name, mss_roomTypeNames[i] ) ) {
				return i;
			}
		}
	} else {
		index = j__atol( name );
		if ( index < 26 ) {
			return index;
		}
	}

	Com_Printf( "roomtype must be an integer from 0 to %i, or one of the following strings:\n", 25 );
	for ( i = 0 ; i < 26 ; i++ ) {
		Com_Printf( "  %s\n", mss_roomTypeNames[i] );
	}
	return 0;
}

/* ---- MSS_SetEnvironmentEffects  0x0044F3C0 ---- CONFIRMED */
int __cdecl MSS_SetEnvironmentEffects(const char *a1, float a2, int a3)
{
  int result;
  unsigned int v4;

  result = mss_digitalDriver;
  if ( mss_digitalDriver )
  {
    v4 = MSS_RoomTypeFromString(a1);
    AIL_set_digital_master_room_type(mss_digitalDriver, v4);
    AIL_set_3D_room_type(mss_3DProvider, v4);
    result = 1;
    mss_roomType = v4;
    /* Retail 0x0044F401 is a RAW DWORD copy, same as MSS_FadeAllSounds: mss_wetTarget is a float lvalue by macro. */
    LODWORD( mss_wetTarget ) = LODWORD( a2 );
    if ( a3 < 1 )
      a3 = 1;
    mss_wetRate = (a2 - *(float *)&mss_wetLevel) / (double)a3;
  }
  return result;
}

/* ---- MSS_UpdateRoomEffects  0x0044F430 ---- */
void __cdecl MSS_UpdateRoomEffects( int msec )
{
	cvar_t *roomtype = MSS_CVAR( mss_roomtype );
	cvar_t *wetlevel = MSS_CVAR( mss_wetlevel );
	int     chan, i, h;

	if ( roomtype->modified || wetlevel->modified ) {
		float wanted;

		roomtype->modified = qfalse;
		wetlevel->modified = qfalse;

		if ( wetlevel->value > 1.0f ) {
			wanted = 1.0f;
		} else if ( wetlevel->value >= 0.0f ) {
			wanted = wetlevel->value;
		} else {
			wanted = 0.0f;
		}

		if ( mss_digitalDriver ) {
			unsigned int room = MSS_RoomTypeFromString( roomtype->string );

			AIL_set_digital_master_room_type( mss_digitalDriver, room );
			AIL_set_3D_room_type( mss_3DProvider, room );
			mss_roomType  = room;
			mss_wetTarget = wanted;
			mss_wetRate   = wanted - mss_wetLevel;
		}
	}

	if ( mss_wetRate == 0.0f ) {
		return;
	}

	mss_wetLevel = (double)msec * mss_wetRate + mss_wetLevel;
	if ( mss_wetRate < 0.0f ) {
		if ( mss_wetLevel <= mss_wetTarget ) {
			mss_wetLevel = mss_wetTarget;
			mss_wetRate  = 0.0f;
		}
	} else if ( mss_wetLevel >= mss_wetTarget ) {
		mss_wetLevel = mss_wetTarget;
		mss_wetRate  = 0.0f;
	}

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( mss_channels[i].paused
			|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE ) {
			AIL_set_3D_sample_effects_level( mss_3DHandles[i], mss_wetLevel );
		}
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( mss_channels[chan].paused
			|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE ) {
			AIL_set_sample_reverb_levels( MSS_2D_HANDLE( chan ), 1.0f, mss_wetLevel );
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			AIL_set_stream_reverb_levels( h, 1.0f, mss_wetLevel );
		} else {
			AIL_close_stream( h );
			MSS_STREAM_HANDLE( chan ) = 0;
		}
	}
}

/* ---- MSS_UpdateTimeScale  0x0044F6C0 ---- */
void MSS_UpdateTimeScale( void )
{
	float scale = com_timescale->value;
	float ratio;
	int   chan, i, h;

	if ( scale <= 0.0f ) {
		scale = 1.0f;
	}
	if ( scale == mss_timeScale ) {
		return;
	}

	ratio = scale / mss_timeScale;
	mss_timeScale = scale;

	for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
		if ( mss_channels[i].paused
			|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE ) {
			h = mss_3DHandles[i];
			AIL_set_3D_sample_playback_rate( h,
				MSS_FTOL( (double)AIL_3D_sample_playback_rate( h ) * ratio ) );
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			AIL_set_stream_playback_rate( h,
				MSS_FTOL( (double)AIL_stream_playback_rate( h ) * ratio ) );
		} else {
			AIL_close_stream( h );
			MSS_STREAM_HANDLE( chan ) = 0;
		}
	}

	for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
		if ( mss_channels[chan].paused
			|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE ) {
			h = MSS_2D_HANDLE( chan );
			AIL_set_sample_playback_rate( h,
				MSS_FTOL( (double)AIL_sample_playback_rate( h ) * ratio ) );
		}
	}
}

/* ---- MSS_UpdatePause  0x0044F8F0 ---- */
int MSS_UpdatePause( void )
{
	int paused = ( cl_paused->integer != 0 );

	if ( paused == mss_paused ) {
		return paused;
	}
	if ( paused ) {
		return MSS_PauseSounds();
	}
	return MSS_UnpauseSounds();
}

/* ---- MSS_Update  0x0044F920 ---- */
int MSS_Update( void )
{
	int now, msec, paused;

	if ( !mss_digitalDriver ) {
		return 0;
	}
	mss_cpuPercent = AIL_digital_CPU_percent( mss_digitalDriver );
	if ( com_statmon->integer && mss_cpuPercent > 2 ) {
		StatMon_Warning( 2, 3000, "gfx/2d/warning@soundcpu.jpg" );
	}

	if ( !sys_timeBaseInit ) {
		sys_timeBase     = timeGetTime();
		sys_timeBaseInit = 1;
	}
	now      = timeGetTime() - sys_timeBase;
	msec     = now - mss_time;
	mss_time = now;

	paused = ( cl_paused->integer != 0 );
	if ( paused != mss_paused ) {
		if ( paused ) {
			MSS_PauseSounds();
		} else {
			MSS_UnpauseSounds();
		}
	}

	MSS_UpdateMasterVolumes( msec );

	if ( !mss_paused ) {
		if ( mss_pendingRestore ) {
			MSS_Restore( (int)mss_pendingRestore, mss_pendingRestoreSize );
			free( mss_pendingRestore );
			mss_pendingRestore     = 0;
			mss_pendingRestoreSize = 0;
		}
		MSS_UpdateTimeScale();
		MSS_UpdateRoomEffects( msec );
		MSS_UpdateAllChannels( msec );
	}

	return MSS_UpdateRawSamples();
}

/* ---- MSS_StopSounds  0x0044FA40 ---- */
int __cdecl MSS_StopSounds( int flags )
{
	int chan, i, h;

	if ( !mss_digitalDriver ) {
		return 0;
	}

	if ( ( flags & 8 ) == 0 ) {
		for ( chan = MSS_FIRST_2D ; chan < MSS_FIRST_2D + mss_num2DChannels ; chan++ ) {
			if ( mss_channels[chan].paused
				|| AIL_sample_status( MSS_2D_HANDLE( chan ) ) != SMP_DONE ) {
				AIL_end_sample( MSS_2D_HANDLE( chan ) );
				mss_channels[chan].paused = 0;
			}
		}
		for ( i = 0 ; i < mss_max3DChannels ; i++ ) {
			if ( mss_channels[i].paused
				|| AIL_3D_sample_status( mss_3DHandles[i] ) != SMP_DONE ) {
				AIL_end_3D_sample( mss_3DHandles[i] );
				mss_channels[i].paused = 0;
			}
		}
	}

	for ( chan = MSS_FIRST_STREAM ; chan < MSS_FIRST_STREAM + mss_numStreamChannels ; chan++ ) {
		h = MSS_STREAM_HANDLE( chan );
		if ( !h ) {
			continue;
		}
		if ( mss_channels[chan].paused || AIL_stream_status( h ) != SMP_DONE ) {
			if ( ( ( flags & 2 ) && chan == 32 )
				|| ( ( flags & 4 ) && ( chan == 33 || chan == 34 ) ) ) {
				continue;
			}
			AIL_close_stream( h );
			mss_channels[chan].paused = 0;
		} else {
			AIL_close_stream( h );
		}
		MSS_STREAM_HANDLE( chan ) = 0;
	}

	if ( ( flags & 1 ) == 0 ) {
		unsigned int room = MSS_RoomTypeFromString( "generic" );

		AIL_set_digital_master_room_type( mss_digitalDriver, room );
		AIL_set_3D_room_type( mss_3DProvider, room );
		mss_roomType  = room;
		mss_wetRate   = -mss_wetLevel;
		mss_wetTarget = 0.0f;
	}
	return mss_numStreamChannels + MSS_FIRST_STREAM;
}

/* ---- MSS_LoadSoundFile  0x0044FC10 ---- CONFIRMED */
_DWORD *__cdecl MSS_LoadSoundFile(const char *a1)
{
  char *v2;
  void *v3;
  unsigned int v4;
  _DWORD *v5;
  unsigned int v6;
  int v7;
  int v8;
  _DWORD *v9;
  int v10;
  int v11;
  int v12;
  int v13;
  void *Block;
  int v15[31];
  unsigned int retaddr;

  v15[30] = retaddr ^ _security_cookie;
  if ( !mss_digitalDriver )
    return 0;
  v2 = va("sound/%s", a1);
  if ( FS_ReadFile(v2, &Block) >= 0 )
  {
    v3 = Block;
    if ( AIL_WAV_info(Block, v15) )
    {
      v4 = v15[3];
      if ( v15[3] <= (unsigned int)mss_driverRate && (v15[4] <= mss_driverBits || v15[0] == 17) && v15[5] <= mss_driverChannels )
      {
        v5 = Hunk_AllocAlignInternal((v15[2] + 39) & 0xFFFFFFFC, 32);
        qmemcpy(v5, v15, 0x24u);
        v5[1] = v5 + 9;
        v5[8] = v5 + 9;
        Com_Memcpy((int)(v5 + 9), (int *)v15[1], v15[2]);
      }
      else
      {
        v6 = v15[6];
        v13 = v15[6];
        if ( v15[3] > (unsigned int)mss_driverRate )
        {
          do
          {
            v4 >>= 1;
            v6 >>= 1;
          }
          while ( v4 > mss_driverRate );
          v13 = v6;
        }
        v11 = mss_driverBits;
        if ( mss_driverBits >= v15[4] )
          v11 = v15[4];
        v12 = mss_driverChannels;
        if ( mss_driverChannels >= v15[5] )
          v12 = v15[5];
        v7 = MSS_SampleType(v15[0], v12, v11);
        v8 = AIL_size_processed_digital_audio(v4, v7, 1, v15);
        v9 = Hunk_AllocAlignInternal((v8 + 39) & 0xFFFFFFFC, 32);
        v10 = v15[0];
        v5 = v9;
        v9[4] = v11;
        v9[6] = v13;
        *v9 = v10;
        v9 += 9;
        v5[1] = v9;
        v5[2] = v8;
        v5[3] = v4;
        v5[5] = v12;
        v5[7] = v15[7];
        v5[8] = v9;
        AIL_process_digital_audio(v9, v8, v4, v7, 1, v15);
        v3 = Block;
      }
      FS_FreeFile(v3);
      return v5;
    }
    else
    {
      Com_Printf("Sound file '%s' is in an invalid or corrupted format\n", v2);
      FS_FreeFile(v3);
      return 0;
    }
  }
  else
  {
    Com_Printf("^1Sound file '%s' not found\n", v2);
    return 0;
  }
}

/* ---- MSS_UnloadSoundFile  0x0044FE10 ---- CONFIRMED */
void MSS_UnloadSoundFile()
{
  ;
}

/* ---- MSS_GetSoundFileSize  0x0044FE20 ---- CONFIRMED */
int __cdecl MSS_GetSoundFileSize(int a1)
{
  return *(_DWORD *)(a1 + 8) + 36;
}

/* MSS_ShutdownAtExit */
static void MSS_ShutdownAtExit( void )
{
	AIL_shutdown();
}

/* MSS_InitFailed */
static void MSS_InitFailed( void )
{
	if ( Cvar_Get( "r_vc_compile", "0", 0 )->integer != 2 ) {
		Com_Printf( "Miles sound system initialization failed\n" );
	}
}

/* MSS_ClearState */
static void MSS_ClearState( void )
{
	mss_digitalDriver     = 0;
	mss_3DProvider        = 0;
	mss_driverRate        = 0;
	mss_driverBits        = 0;
	mss_driverChannels    = 0;
	mss_num2DChannels     = 0;
	mss_max3DChannels     = 0;
	mss_numStreamChannels = 0;
}

/* ---- MSS_Init  0x0044FE60 ---- */
int MSS_Init( void )
{
	cvar_t      *volume;
	unsigned int now;
	int          i;

	Com_Printf( "\n------- Miles sound system initialization -------\n" );

	mss_q3fs = (int)Cvar_Get( "mss_q3fs", "1", 32  );
	if ( MSS_CVAR( mss_q3fs )->integer ) {
		AIL_set_file_callbacks( (AIL_FILE_OPEN_CB) MSS_FileOpenCallback,
								(AIL_FILE_CLOSE_CB)MSS_FileCloseCallback,
								(AIL_FILE_SEEK_CB) MSS_FileSeekCallback,
								(AIL_FILE_READ_CB) MSS_FileReadCallback );
	}
	AIL_set_redist_directory( "miles" );
	atexit( MSS_ShutdownAtExit );

	if ( !AIL_startup() ) {
		MSS_InitFailed();
		return 0;
	}

	mss_errorOnMissing = (int)Cvar_Get( "mss_errorOnMissing", "1", 1 );
	mss_khz            = (int)Cvar_Get( "mss_khz",    "44", 0x21 );
	mss_bits           = (int (*)())Cvar_Get( "mss_bits",   "16", 0x21 );
	mss_stereo         = (int)Cvar_Get( "mss_stereo", "1",  0x21 );
	mss_3d_provider    = (int)Cvar_Get( "mss_3d_provider",
										"Miles Fast 2D Positional Audio", 0x21 );
	mss_volume         = (int)Cvar_Get( "mss_volume",   "0.8", 1 );
	mss_roomtype       = (int)Cvar_Get( "mss_roomtype", "0", 0x200 );
	mss_wetlevel       = (int)Cvar_Get( "mss_wetlevel", "0", 0x200 );

	if ( !MSS_Init2D() ) {
		AIL_shutdown();
		MSS_ClearState();
		MSS_InitFailed();
		return 0;
	}

	if ( !MSS_Init3DProvider( MSS_CVAR( mss_3d_provider )->string, (char *)1 ) ) {
		AIL_shutdown();
		MSS_ClearState();
		MSS_InitFailed();
		return 0;
	}

	MSS_InitChannels();

	mss_globalFadeCur    = 1.0f;
	mss_globalFadeTarget = 1.0f;
	mss_globalFadeRate   = 0.0f;

	for ( i = 0 ; i < MSS_NUM_ALIAS_CHANNELS ; i++ ) {
		mss_channelFade[i][0] = 1.0f;
		mss_channelFade[i][1] = 1.0f;
		mss_channelFade[i][2] = 0.0f;
	}

	if ( !sys_timeBaseInit ) {
		sys_timeBase     = timeGetTime();
		sys_timeBaseInit = 1;
	}
	now               = timeGetTime() - sys_timeBase;
	mss_time          = now;
	mss_loopFrameTime = now;
	mss_anyMasters    = 0;

	volume = MSS_CVAR( mss_volume );
	if ( volume->value < 0.0f ) {
		mss_masterVolume = 0.0f;
	} else if ( volume->value > 1.0f ) {
		mss_masterVolume = 1.0f;
	} else {
		mss_masterVolume = volume->value;
	}

	Com_Printf( "------- Miles successfully initialized -------\n" );
	return 1;
}

/* ---- MSS_Shutdown  0x00450130 ---- */
void MSS_Shutdown( void )
{
	if ( mss_pendingRestore ) {
		free( mss_pendingRestore );
		mss_pendingRestore     = 0;
		mss_pendingRestoreSize = 0;
	}
	if ( !mss_digitalDriver ) {
		return;
	}

	MSS_StopSounds( 0 );

	AIL_close_3D_provider( mss_3DProvider );
	mss_3DProvider = 0;
	AIL_shutdown();
	MSS_ClearState();
}

/* ---- MSS_ErrorCleanup  0x00450240 ---- CONFIRMED */
void MSS_ErrorCleanup()
{
  if ( mss_pendingRestore )
  {
    free(mss_pendingRestore);
    mss_pendingRestore = 0;
  }
}

/* ---- MSS_SetDirectSoundHWND_m  0x00450260 ---- VERIFIED */
int __cdecl MSS_SetDirectSoundHWND_m(void *this)
{
  int result;

  result = mss_digitalDriver;
  if ( mss_digitalDriver )
    AIL_set_DirectSound_HWND(mss_digitalDriver, this);
  return result;
}

#define MSS_ERR_SAVE_OVERFLOW \
	"\x15" "buffer overflow while saving sound state: more than %i bytes used\n"
#define MSS_ERR_RESTORE_OVERFLOW \
	"\x15" "buffer overflow while restoring sound state: probably due to an old or corrupt savegame\n"
#define MSS_ERR_STATE_TOO_BIG \
	"\x15" "sound state buffer size too big; probably due to an old or corrupt savegame\n"

/* ---- MSS_Write  0x00450280 ---- CONFIRMED */
int __cdecl MSS_Write(signed int a1, int a2, unsigned int a3, int a4, const void *a5)
{
  if ( (int)(a2 + a3) > a1 )
    Com_Error(ERR_DROP, MSS_ERR_SAVE_OVERFLOW, a1);
  qmemcpy((void *)(a4 + a2), a5, a3);
  return a2 + a3;
}

/* ---- MSS_Read  0x004502C0 ---- CONFIRMED */
int __cdecl MSS_Read(signed int a1, int a2, unsigned int a3, int a4, void *a5)
{
  if ( (int)(a2 + a3) > a1 )
    Com_Error(ERR_DROP, MSS_ERR_RESTORE_OVERFLOW);
  qmemcpy(a5, (const void *)(a4 + a2), a3);
  return a2 + a3;
}

/* ---- MSS_SaveChanInfo  0x00450300 ---- HIGH */
int __cdecl MSS_SaveChanInfo(int a1, int a2, int a3, int a4)
{
  int result;
  _DWORD *v6;
  _DWORD *v7;

  if ( a4 + 2 > a3 )
    Com_Error(ERR_DROP, MSS_ERR_SAVE_OVERFLOW, a3);
  *(_WORD *)(a2 + a4) = *(_WORD *)a1;
  if ( a4 + 3 > a3 )
    Com_Error(ERR_DROP, MSS_ERR_SAVE_OVERFLOW, a3);
  *(_BYTE *)(a2 + a4 + 2) = *(_BYTE *)(a1 + 4);
  if ( a4 + 7 > a3 )
    Com_Error(ERR_DROP, MSS_ERR_SAVE_OVERFLOW, a3);
  *(_DWORD *)(a2 + a4 + 3) = *(_DWORD *)(a1 + 12);
  if ( a4 + 11 > a3 )
    Com_Error(ERR_DROP, MSS_ERR_SAVE_OVERFLOW, a3);
  *(_DWORD *)(a2 + a4 + 7) = *(_DWORD *)(a1 + 16);
  if ( a4 + 15 > a3 )
    Com_Error(ERR_DROP, MSS_ERR_SAVE_OVERFLOW, a3);
  *(_DWORD *)(a2 + a4 + 11) = *(_DWORD *)(a1 + 36);
  result = a4 + 27;
  if ( a4 + 27 > a3 )
    Com_Error(ERR_DROP, MSS_ERR_SAVE_OVERFLOW, a3);
  v6 = (_DWORD *)(a1 + 40);
  v7 = (_DWORD *)(a4 + 15 + a2);
  *v7 = *v6;
  v7[1] = v6[1];
  v7[2] = v6[2];
  return result;
}

/* ---- MSS_RestoreChanInfo  0x00450410 ---- HIGH */
int __cdecl MSS_RestoreChanInfo(_WORD *a1, int a2, int a3, int a4)
{
  int v5;
  int result;
  _DWORD *v7;

  memset(a1, 0, 0x38u);
  if ( a4 + 2 > a2 )
    Com_Error(ERR_DROP, MSS_ERR_RESTORE_OVERFLOW);
  *a1 = *(_WORD *)(a4 + a3);
  if ( a4 + 3 > a2 )
    Com_Error(ERR_DROP, MSS_ERR_RESTORE_OVERFLOW);
  *((_BYTE *)a1 + 4) = *(_BYTE *)(a4 + 2 + a3);
  if ( a4 + 7 > a2 )
    Com_Error(ERR_DROP, MSS_ERR_RESTORE_OVERFLOW);
  *((_DWORD *)a1 + 3) = *(_DWORD *)(a4 + 3 + a3);
  if ( a4 + 11 > a2 )
    Com_Error(ERR_DROP, MSS_ERR_RESTORE_OVERFLOW);
  v5 = a4 + 15;
  *((_DWORD *)a1 + 4) = *(_DWORD *)(a4 + 7 + a3);
  if ( a4 + 15 > a2 )
    Com_Error(ERR_DROP, MSS_ERR_RESTORE_OVERFLOW);
  result = a4 + 27;
  *((_DWORD *)a1 + 9) = *(_DWORD *)(a4 + 11 + a3);
  if ( a4 + 27 > a2 )
    Com_Error(ERR_DROP, MSS_ERR_RESTORE_OVERFLOW);
  v7 = a1 + 20;
  *v7 = *(_DWORD *)(v5 + a3);
  v7[1] = *(_DWORD *)(v5 + a3 + 4);
  v7[2] = *(_DWORD *)(v5 + a3 + 8);
  return result;
}

extern void *Com_GetSoundAlias( int source, int index );
extern int   Com_SoundAliasIndex( const void *alias, int source );
extern int   Com_SoundAliasChecksum( int source );

#define MSS_ALIAS_SOURCE    1

#define MSS_TYPE_LOADED     1
#define MSS_TYPE_STREAMED   2

typedef struct {
	float   startFraction;
	float   pitch;
	float   volume;
	vec3_t  position;
} mssSave3D_t;

typedef struct {
	float   startFraction;
	float   pitch;
	float   volume;
	float   pan;
} mssSave2D_t;

typedef struct {
	float   startFraction;
	int     rate;
	float   channelVolume;
	float   volume;
	float   pan;
	vec3_t  origin;
} mssSaveStream_t;

/* ---- MSS_Save3DChannel  0x00450530 ---- VERIFIED */
static int MSS_Save3DChannel( int chan, int base, int offset, int size )
{
	mssChannel_t *ch = &mss_channels[chan];
	int           h  = MSS_3D_HANDLE( chan );
	int           aliasIndex, alias2Index;
	unsigned int  sampleOffset, sampleLength;
	int           rate;
	float         vol;
	mssSave3D_t   saved;

	if ( !ch->paused && AIL_3D_sample_status( h ) == SMP_DONE ) {
		return offset;
	}

	aliasIndex = Com_SoundAliasIndex( ch->alias, MSS_ALIAS_SOURCE );
	if ( !aliasIndex ) {
		return offset;
	}
	alias2Index = Com_SoundAliasIndex( ch->alias2, MSS_ALIAS_SOURCE );
	if ( !alias2Index ) {
		return offset;
	}

	sampleOffset = AIL_3D_sample_offset( h );
	sampleLength = AIL_3D_sample_length( h );
	rate         = AIL_3D_sample_playback_rate( h );
	if ( !sampleLength || !rate ) {
		return offset;
	}

	saved.startFraction = (float) sampleOffset / (float) sampleLength;
	saved.pitch         = ch->pitch;

	vol = AIL_3D_sample_volume( h );
	if ( mss_masterVolume == 0.0f ) {
		saved.volume = ch->volume;
	} else {
		saved.volume = vol / mss_masterVolume;
	}

	AIL_3D_position( h, &saved.position[0], &saved.position[1], &saved.position[2] );

	offset = MSS_Write( size, offset, 2, base, &aliasIndex );
	offset = MSS_Write( size, offset, 2, base, &alias2Index );
	offset = MSS_SaveChanInfo( (int) ch, base, size, offset );
	return MSS_Write( size, offset, sizeof( mssSave3D_t ), base, &saved );
}

/* ---- MSS_Restore3DChannel  0x00450700 ---- VERIFIED */
static int MSS_Restore3DChannel( int size, int base, int offset, int aliasIndex )
{
	mssChannel_t ch;
	mssSave3D_t  saved;
	int          alias2Index;
	void        *alias, *alias2;
	int          chan = -1;

	if ( offset + 2 > size ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	alias2Index = *(unsigned short *)( base + offset );

	offset = MSS_RestoreChanInfo( (_WORD *) &ch, size, base, offset + 2 );

	if ( offset + (int) sizeof( saved ) > size ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	memcpy( &saved, (const void *)( base + offset ), sizeof( saved ) );
	offset += sizeof( saved );

	alias  = Com_GetSoundAlias( MSS_ALIAS_SOURCE, aliasIndex );
	alias2 = Com_GetSoundAlias( MSS_ALIAS_SOURCE, alias2Index );

	if ( alias && alias2
		&& AL_TYPE( alias ) == MSS_TYPE_LOADED
		&& AL_TYPE( alias2 ) == MSS_TYPE_LOADED
		&& AL_SOUND( alias ) && AL_SOUND( alias2 ) == AL_SOUND( alias )
		&& MSS_ValidateSoundAliasBlend( (_DWORD *) alias, (_DWORD *) alias2, 0 ) )
	{
		MSS_StartAlias3DSample( &chan, alias, saved.position, alias2, ch.blend,
		                        ch.entnum, saved.volume, saved.pitch, 0,
		                        saved.startFraction );
		if ( chan >= 0 ) {
			VectorCopy( ch.localOrigin, mss_channels[chan].localOrigin );
		}
	}
	return offset;
}

/* ---- MSS_Save2DChannel  0x004508D0 ---- VERIFIED */
static int MSS_Save2DChannel( int chan, int base, int offset, int size )
{
	mssChannel_t *ch = &mss_channels[chan];
	int           h  = MSS_2D_HANDLE( chan );
	int           aliasIndex, alias2Index;
	int           totalMs, currentMs, rate;
	mssSave2D_t   saved;

	if ( !ch->paused && AIL_sample_status( h ) == SMP_DONE ) {
		return offset;
	}

	aliasIndex = Com_SoundAliasIndex( ch->alias, MSS_ALIAS_SOURCE );
	if ( !aliasIndex ) {
		return offset;
	}
	alias2Index = Com_SoundAliasIndex( ch->alias2, MSS_ALIAS_SOURCE );
	if ( !alias2Index ) {
		return offset;
	}

	AIL_sample_ms_position( h, &totalMs, &currentMs );
	rate = AIL_sample_playback_rate( h );
	if ( !totalMs || !rate ) {
		return offset;
	}

	saved.startFraction = (float) (unsigned int) currentMs
	                    / (float) (unsigned int) totalMs;
	saved.pitch         = ch->pitch;

	AIL_sample_volume_pan( h, &saved.volume, &saved.pan );
	if ( mss_masterVolume == 0.0f ) {
		saved.volume = ch->volume;
	} else {
		saved.volume = saved.volume / mss_masterVolume;
	}

	offset = MSS_Write( size, offset, 2, base, &aliasIndex );
	offset = MSS_Write( size, offset, 2, base, &alias2Index );
	offset = MSS_SaveChanInfo( (int) ch, base, size, offset );
	return MSS_Write( size, offset, sizeof( mssSave2D_t ), base, &saved );
}

/* ---- MSS_Restore2DChannel  0x00450AA0 ---- VERIFIED */
static int MSS_Restore2DChannel( int size, int base, int offset, int aliasIndex )
{
	mssChannel_t ch;
	mssSave2D_t  saved;
	void        *alias, *alias2;
	int          chan = -1;

	if ( offset + 2 > size ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}

	offset = MSS_RestoreChanInfo( (_WORD *) &ch, size, base, offset + 2 );

	if ( offset + (int) sizeof( saved ) > size ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	memcpy( &saved, (const void *)( base + offset ), sizeof( saved ) );
	offset += sizeof( saved );

	alias  = Com_GetSoundAlias( MSS_ALIAS_SOURCE, aliasIndex );
	alias2 = Com_GetSoundAlias( MSS_ALIAS_SOURCE, aliasIndex );

	if ( alias && alias2
		&& AL_TYPE( alias ) == MSS_TYPE_LOADED
		&& AL_TYPE( alias2 ) == MSS_TYPE_LOADED
		&& AL_SOUND( alias ) && AL_SOUND( alias2 ) == AL_SOUND( alias )
		&& MSS_ValidateSoundAliasBlend( (_DWORD *) alias, (_DWORD *) alias2, 0 ) )
	{
		MSS_StartAlias2DSample( &chan, alias, alias2, ch.blend, ch.entnum,
		                        saved.volume, saved.pitch, 0,
		                        saved.startFraction );
		if ( chan >= 0 ) {
			AIL_set_sample_volume_pan( MSS_2D_HANDLE( chan ),
			                           mss_masterVolume * saved.volume,
			                           saved.pan );
			VectorCopy( ch.localOrigin, mss_channels[chan].localOrigin );
		}
	}
	return offset;
}

/* ---- MSS_SaveStreamChannel  0x00450C80 ---- VERIFIED */
static int MSS_SaveStreamChannel( int chan, int base, int offset, int size )
{
	mssChannel_t   *ch;
	int             h;
	int             aliasIndex, alias2Index;
	int             totalMs, currentMs, rate;
	mssSaveStream_t saved;

	if ( chan >= MSS_FIRST_STREAM + MSS_NUM_BACKGROUND ) {
		if ( MSS_IsStreamChannelFree( chan ) ) {
			return offset;
		}
	} else {
		if ( chan == MSS_FIRST_STREAM + 1 || chan == MSS_FIRST_STREAM + 2 ) {
			return offset;
		}
		if ( MSS_IsStreamChannelFree( chan ) ) {
			aliasIndex = 0;
			return MSS_Write( size, offset, 2, base, &aliasIndex );
		}
	}

	ch = &mss_channels[chan];
	h  = MSS_STREAM_HANDLE( chan );

	aliasIndex = Com_SoundAliasIndex( ch->alias, MSS_ALIAS_SOURCE );
	if ( !aliasIndex ) {
		return offset;
	}
	alias2Index = Com_SoundAliasIndex( ch->alias2, MSS_ALIAS_SOURCE );
	if ( !alias2Index ) {
		return offset;
	}

	AIL_stream_ms_position( h, &totalMs, &currentMs );
	if ( !totalMs ) {
		return offset;
	}

	saved.startFraction = (float) (unsigned int) currentMs
	                    / (float) (unsigned int) totalMs;

	rate       = AIL_stream_playback_rate( h );
	saved.rate = MSS_FTOL( (float) rate / mss_timeScale );

	saved.channelVolume = ch->volume;

	AIL_stream_volume_pan( h, &saved.volume, &saved.pan );
	if ( mss_masterVolume == 0.0f ) {
		saved.volume = ch->volume;
	} else {
		saved.volume = saved.volume / mss_masterVolume;
	}

	VectorCopy( MSS_STREAM_INFO( chan ).origin, saved.origin );

	offset = MSS_Write( size, offset, 2, base, &aliasIndex );
	offset = MSS_Write( size, offset, 2, base, &alias2Index );
	offset = MSS_SaveChanInfo( (int) ch, base, size, offset );
	return MSS_Write( size, offset, sizeof( mssSaveStream_t ), base, &saved );
}

/* ---- MSS_RestoreStreamChannel  0x00450EA0 ---- VERIFIED */
static int MSS_RestoreStreamChannel( int size, int base, int offset,
                                     int aliasIndex, int chan )
{
	mssChannel_t    ch;
	mssSaveStream_t saved;
	void           *alias, *alias2;
	int             duration;
	int             h;

	if ( offset + 2 > size ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}

	offset = MSS_RestoreChanInfo( (_WORD *) &ch, size, base, offset + 2 );

	if ( offset + (int) sizeof( saved ) > size ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	memcpy( &saved, (const void *)( base + offset ), sizeof( saved ) );
	offset += sizeof( saved );

	alias  = Com_GetSoundAlias( MSS_ALIAS_SOURCE, aliasIndex );
	alias2 = Com_GetSoundAlias( MSS_ALIAS_SOURCE, aliasIndex );

	if ( alias && alias2
		&& AL_TYPE( alias ) == MSS_TYPE_STREAMED
		&& AL_TYPE( alias2 ) == MSS_TYPE_STREAMED
		&& MSS_ValidateSoundAliasBlend( (_DWORD *) alias, (_DWORD *) alias2, 0 ) )
	{
		if ( chan < 0 ) {
			duration = MSS_StartAliasStream( &chan, alias, ch.entnum, alias2,
			                                 ch.blend, saved.origin,
			                                 saved.channelVolume, 1.0f, 0,
			                                 saved.startFraction );
		} else {
			duration = MSS_StartAliasStreamOnChannel( alias, alias2, ch.blend,
			                                          ch.entnum, saved.origin,
			                                          saved.channelVolume, 1.0f,
			                                          0, saved.startFraction,
			                                          chan );
		}

		if ( chan >= 0 ) {
			h = MSS_STREAM_HANDLE( chan );

			mss_channels[chan].endTime -=
				MSS_FTOL( (float) duration * saved.startFraction );

			AIL_set_stream_volume_pan( h, mss_masterVolume * saved.volume,
			                           saved.pan );
			AIL_set_stream_playback_rate( h,
				MSS_FTOL( (float) saved.rate * mss_channels[chan].pitch
				          * mss_timeScale ) );

			VectorCopy( ch.localOrigin, mss_channels[chan].localOrigin );
		}
	}
	return offset;
}

/* ---- MSS_Save  0x004510F0 ---- VERIFIED */
int MSS_Save( int size, int base )
{
	int   offset;
	int   i;
	int   checksum;
	short terminator = 0;
	struct {
		int   roomType;
		float wetLevel;
		float wetTarget;
		float wetRate;
	} roomEffects;   /* 0x008E0840..0x008E084F, one record in retail */

	checksum = Com_SoundAliasChecksum( MSS_ALIAS_SOURCE );

	offset = MSS_Write( size, 0, sizeof( checksum ), base, &checksum );
	offset = MSS_Write( size, offset, sizeof( mss_channelFade ), base,
	                    mss_channelFade );

	roomEffects.roomType  = mss_roomType;
	roomEffects.wetLevel  = mss_wetLevel;
	roomEffects.wetTarget = mss_wetTarget;
	roomEffects.wetRate   = mss_wetRate;
	offset = MSS_Write( size, offset, sizeof( roomEffects ), base, &roomEffects );

	offset = MSS_Write( size, offset, sizeof( mss_bgFade[0] ), base, &mss_bgFade[0] );
	offset = MSS_Write( size, offset, sizeof( mss_bgFade[3] ), base, &mss_bgFade[3] );
	offset = MSS_Write( size, offset, sizeof( mss_bgFade[4] ), base, &mss_bgFade[4] );

	if ( mss_digitalDriver ) {
		for ( i = 0; i < mss_max3DChannels; i++ ) {
			offset = MSS_Save3DChannel( i, base, offset, size );
		}
	}
	offset = MSS_Write( size, offset, 2, base, &terminator );

	if ( mss_digitalDriver ) {
		for ( i = MSS_FIRST_2D; i < mss_num2DChannels + MSS_FIRST_2D; i++ ) {
			offset = MSS_Save2DChannel( i, base, offset, size );
		}
	}
	offset = MSS_Write( size, offset, 2, base, &terminator );

	if ( mss_digitalDriver ) {
		for ( i = MSS_FIRST_STREAM; i < mss_numStreamChannels + MSS_FIRST_STREAM; i++ ) {
			offset = MSS_SaveStreamChannel( i, base, offset, size );
		}
	}
	return MSS_Write( size, offset, 2, base, &terminator );
}

/* ---- MSS_Restore  0x00451330 ---- VERIFIED */
void MSS_Restore( int buffer, int size )
{
	int offset;
	int i;
	int aliasIndex;

	if ( !mss_digitalDriver ) {
		return;
	}

	if ( size < 4 ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	if ( *(int *) buffer != Com_SoundAliasChecksum( MSS_ALIAS_SOURCE ) ) {
		Com_Printf( "^3Couldn't restore sound state: sound aliases have changed since saving.\n" );
		return;
	}

	if ( size < 4 + (int) sizeof( mss_channelFade ) ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	memcpy( mss_channelFade, (const void *)( buffer + 4 ), sizeof( mss_channelFade ) );

	if ( size < 140 ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	mss_roomType  = *(int   *)( buffer + 124 );
	mss_wetLevel  = *(float *)( buffer + 128 );
	mss_wetTarget = *(float *)( buffer + 132 );
	mss_wetRate   = *(float *)( buffer + 136 );

	AIL_set_digital_master_room_type( mss_digitalDriver, mss_roomType );
	AIL_set_3D_room_type( mss_3DProvider, mss_roomType );

	if ( size < 148 ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	mss_bgFade[0].target = *(float *)( buffer + 140 );
	mss_bgFade[0].rate   = *(float *)( buffer + 144 );

	if ( size < 156 ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	mss_bgFade[3].target = *(float *)( buffer + 148 );
	mss_bgFade[3].rate   = *(float *)( buffer + 152 );

	if ( size < 164 ) {
		Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
	}
	mss_bgFade[4].target = *(float *)( buffer + 156 );
	mss_bgFade[4].rate   = *(float *)( buffer + 160 );

	offset = 164;

	for ( ;; ) {
		if ( offset + 2 > size ) {
			Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
		}
		aliasIndex = *(unsigned short *)( buffer + offset );
		if ( !aliasIndex ) {
			offset += 2;
			break;
		}
		offset = MSS_Restore3DChannel( size, buffer, offset + 2, aliasIndex );
	}

	for ( ;; ) {
		if ( offset + 2 > size ) {
			Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
		}
		aliasIndex = *(unsigned short *)( buffer + offset );
		if ( !aliasIndex ) {
			offset += 2;
			break;
		}
		offset = MSS_Restore2DChannel( size, buffer, offset + 2, aliasIndex );
	}

	for ( i = 0; i < MSS_NUM_BACKGROUND; i++ ) {
		if ( i == 1 || i == 2 ) {
			continue;
		}
		if ( offset + 2 > size ) {
			Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
		}
		aliasIndex = *(unsigned short *)( buffer + offset );
		offset += 2;
		if ( !aliasIndex ) {
			continue;
		}
		offset = MSS_RestoreStreamChannel( size, buffer, offset, aliasIndex,
		                                   MSS_FIRST_STREAM + i );
	}

	for ( ;; ) {
		if ( offset + 2 > size ) {
			Com_Error( ERR_DROP, MSS_ERR_RESTORE_OVERFLOW );
		}
		aliasIndex = *(unsigned short *)( buffer + offset );
		if ( !aliasIndex ) {
			offset += 2;
			break;
		}
		offset = MSS_RestoreStreamChannel( size, buffer, offset + 2, aliasIndex, -1 );
	}

	if ( offset != size ) {
		Com_Error( ERR_DROP, MSS_ERR_STATE_TOO_BIG );
	}
}

/* ---- MSS_QueueRestore_m  0x004515F0 ---- VERIFIED */
void __cdecl MSS_QueueRestore_m(int a1, int *a2)
{
  MSS_StopSounds(6);
  if ( mss_pendingRestore )
    free(mss_pendingRestore);
  mss_pendingRestoreSize = a1;
  mss_pendingRestore = Z_MallocInternal(a1);
  Com_Memcpy((int)mss_pendingRestore, a2, a1);
}

/* ---- MSS_GetSoundOverlay2D  0x00451630 ---- */
static int MSS_GetSoundOverlay2D( mssOverlay_t *buffer, int maxCount )
{
	int i, count, h, rate;

	count = maxCount;
	if ( count > mss_num2DChannels ) {
		count = mss_num2DChannels;
	}
	if ( count <= 0 || !buffer ) {
		return count;
	}

	for ( i = 0 ; i < count ; i++ ) {
		mssChannel_t *ch = &mss_channels[ MSS_FIRST_2D + i ];

		if ( !ch->paused && AIL_sample_status( mss_2DHandles[i] ) == SMP_DONE ) {
			buffer[i].name = 0;
			continue;
		}
		h = mss_2DHandles[i];
		buffer[i].name = AL_FILENAME( ch->alias );

		rate = AIL_sample_playback_rate( h );
		if ( !rate ) {
			rate = ch->baseRate;
		}
		buffer[i].requested = ch->volume * 1.25f;
		AIL_sample_volume_pan( h, &buffer[i].actual, 0 );
		buffer[i].actual = buffer[i].actual * 1.25f;
		if ( mss_masterVolume != 0.0f ) {
			buffer[i].actual = buffer[i].actual / mss_masterVolume;
		}
		buffer[i].baseRate  = ch->baseRate;
		buffer[i].rateScale = (float)rate / (float)ch->baseRate;
	}
	return count;
}

/* ---- MSS_GetSoundOverlay3D  0x00451730 ---- */
static int MSS_GetSoundOverlay3D( mssOverlay_t *buffer, int maxCount )
{
	int   i, count;
	int   h, rate;
	float x, y, z;

	count = maxCount;
	if ( count > mss_max3DChannels ) {
		count = mss_max3DChannels;
	}
	if ( !buffer ) {
		return count;
	}

	for ( i = 0 ; i < count ; i++ ) {
		mssChannel_t *ch = &mss_channels[i];

		if ( !ch->paused && AIL_3D_sample_status( mss_3DHandles[i] ) == SMP_DONE ) {
			buffer[i].name = 0;
			continue;
		}
		h = mss_3DHandles[i];
		buffer[i].name = AL_FILENAME( ch->alias );

		AIL_3D_sample_length( h );
		rate = AIL_3D_sample_playback_rate( h );
		if ( !rate ) {
			rate = ch->baseRate;
		}
		AIL_3D_position( h, &x, &y, &z );

		buffer[i].requested = ch->volume * 1.25f;
		buffer[i].actual    = AIL_3D_sample_volume( h ) * 1.25f;
		if ( mss_masterVolume != 0.0f ) {
			buffer[i].actual = buffer[i].actual / mss_masterVolume;
		}
		buffer[i].baseRate  = ch->baseRate;
		buffer[i].rateScale = (float)rate / (float)ch->baseRate;
	}
	return count;
}

/* ---- MSS_GetSoundOverlayStream  0x00451860 ---- */
static int MSS_GetSoundOverlayStream( mssOverlay_t *buffer, int maxCount )
{
	int i, count, h, rate;

	count = maxCount;
	if ( count > mss_numStreamChannels ) {
		count = mss_numStreamChannels;
	}
	if ( count <= 0 || !buffer ) {
		return count;
	}

	for ( i = 0 ; i < count ; i++ ) {
		mssChannel_t *ch = &mss_channels[ MSS_FIRST_STREAM + i ];

		h = mss_streamHandles[i];
		if ( !h ) {
			buffer[i].name = 0;
			continue;
		}
		if ( !ch->paused && AIL_stream_status( h ) == SMP_DONE ) {
			AIL_close_stream( h );
			mss_streamHandles[i] = 0;
			buffer[i].name = 0;
			continue;
		}
		buffer[i].name = AL_FILENAME( ch->alias );

		rate = AIL_stream_playback_rate( h );
		buffer[i].requested = ch->volume * 1.25f;
		AIL_stream_volume_pan( h, &buffer[i].actual, 0 );
		buffer[i].actual = buffer[i].actual * 1.25f;
		if ( mss_masterVolume != 0.0f ) {
			buffer[i].actual = buffer[i].actual / mss_masterVolume;
		}
		buffer[i].baseRate  = ch->baseRate;
		buffer[i].rateScale = (float)rate / (float)ch->baseRate;
	}
	return count;
}

/* ---- MSS_GetSoundOverlay  0x00451960 ---- VERIFIED */
int __cdecl MSS_GetSoundOverlay( int *cpuOut, int maxCount, void *buffer, int which )
{
	if ( !mss_digitalDriver ) {
		return 0;
	}
	if ( cpuOut ) {
		*cpuOut = mss_cpuPercent;
	}
	if ( which == 1 ) {
		return MSS_GetSoundOverlay3D( buffer, maxCount );
	}
	if ( which == 2 ) {
		return MSS_GetSoundOverlayStream( buffer, maxCount );
	}
	if ( which == 3 ) {
		return MSS_GetSoundOverlay2D( buffer, maxCount );
	}
	return 0;
}
