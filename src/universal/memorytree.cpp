/*
 * universal/memorytree.cpp
 *
 * @fidelity: likely
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

extern void MakeNormalVectors( const float *forward, float *right, float *up );
extern int SEH_GetLocalizedString_m();
extern int SL_FindLowercaseString();
extern void Sys_OutOfMemoryPrep( void );
extern int DObjFindPartIndex();   /* 0x00481230, defined in xanim/dobj.cpp */
extern int FxMem_ClaimBlock_m();
extern int sub_4920B0();
extern int sub_492110();
extern int sub_492170();
extern int sub_4921D0();
extern int sub_492230();
extern int sub_492290();
extern int sub_4922F0();
extern int sub_492350();
extern int sub_4923B0();
extern int CFxScheduler__RegisterEffect();
extern int CFxScheduler__PlayEffect_id_simple_m();
extern int CFxScheduler__PlayEffect_name_bolt_m();
extern int CFxScheduler__PlayEffect_id_axis();
extern int CFxScheduler__PlayEffect_name_simple_m();
extern int CFxScheduler__PlayEffect_name_m();
extern int CFxScheduler__AddScheduledEffects();
extern int SFxHelper__AdjustTime();
extern void SFxHelper__AdjustCamera( int self, int view, float zfar );
extern int FX_Free();
extern int FX_Init();
extern int _tr_init();
extern int _tr_stored_block();
extern int _tr_align();
extern int _tr_flush_block();
extern int zcalloc();
extern int zcfree();

int __cdecl FxPool_MakeBlockCurrent( int block, int pool );

char *__cdecl GP2_GetToken( char allowLineBreaks, char readUntilNewline, char **text );
void **__cdecl CTextPool__CTextPool( size_t initSize, void **this_ );
int __cdecl CTextPool__AllocText( int *this_, const char *text, int addNULL, int poolPtr );
char __cdecl CGPObject__WriteText( const char *text, int **textPool, int this_ );
int __cdecl CGPValue__AddValue( int textPool, const char *newValue, int this_ );
int *__cdecl CGPValue__Duplicate( int this_, int **textPool );
char __cdecl CGPValue__Parse( int **textPool, int this_, char **dataPtr );
char __cdecl CGPValue__Write( int this_, int **textPool, int depth );
int __cdecl CGPGroup__Clean( int this_ );
int *__cdecl CGPGroup__Duplicate( int this_, int **textPool, int initParent );
_DWORD *__cdecl CGP_InsertInOrder( _DWORD *listHead, int newObj, int parent,
                                   _DWORD *inOrderHead, int currentSlot );
int __cdecl CGPGroup__AddPair( const char *value, int **textPool, int this_, const char *name );
int __cdecl CGPGroup__AddGroup( int **textPool, const char *name, int this_ );
char __cdecl CGPGroup__Parse( _BYTE *this_, char **dataPtr, int **textPool );
char __cdecl CGPGroup__Write( const char **this_, int **textPool, int depth );
void __cdecl CGenericParser2__CleanTextPool( int this_ );

int __cdecl adler32( unsigned int adler, unsigned __int8 *buf, unsigned int len );
int __cdecl crc32( int crc, unsigned int len, _BYTE *buf );
unsigned int __cdecl compress2( int sourceLen, int dest, int destLen, int *source, unsigned int level );
int __cdecl deflateInit2_( _BYTE *version, _DWORD *strm, int windowBits, unsigned int level,
                           int method, int memLevel, unsigned int strategy, int stream_size );
int __cdecl deflateParams( int strm, unsigned int level, unsigned int strategy );
int __cdecl putShortMSB( int s, __int16 b );
int __cdecl flush_pending( int strm );
int __cdecl deflate( int strm, int flush );
unsigned int __cdecl deflateEnd( int strm );
unsigned int __cdecl deflate_read_buf( int strm, void *buf, unsigned int size );
int __fastcall lm_init( int unused, int s );
unsigned int __cdecl longest_match( unsigned int cur_match, _DWORD *s );
int __cdecl deflate_fill_window( int s );
int __cdecl deflate_stored( int *s, int flush );
int __cdecl deflate_fast( int s, int flush );
int __cdecl deflate_slow( _DWORD *s, int flush );

#define GP2_MAX_TOKEN_SIZE  1024
static char gp2_token[ GP2_MAX_TOKEN_SIZE + 4 ];

static int   zlib_tablesBuilt;
static unsigned int  zlib_crcTable[256];
static unsigned char zlib_distCode[512];    /* 0x00540CB8, d_code() lookup   */
static unsigned char zlib_lengthCode[256];  /* 0x00540EB8, length_code[]     */

static void zlib_BuildTables( void ) {
	static const int extra_lbits[29] = { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,
	                                     3,3,3,3,4,4,4,4,5,5,5,5,0 };
	static const int extra_dbits[30] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,
	                                     7,7,8,8,9,9,10,10,11,11,12,12,13,13 };
	unsigned int c;
	int n, k, code, length, dist;

	if ( zlib_tablesBuilt ) {
		return;
	}

	for ( n = 0; n < 256; n++ ) {
		c = (unsigned int)n;
		for ( k = 0; k < 8; k++ ) {
			c = ( c & 1 ) ? ( 0xEDB88320u ^ ( c >> 1 ) ) : ( c >> 1 );
		}
		zlib_crcTable[n] = c;
	}

	length = 0;
	for ( code = 0; code < 28; code++ ) {
		for ( n = 0; n < ( 1 << extra_lbits[code] ); n++ ) {
			zlib_lengthCode[length++] = (unsigned char)code;
		}
	}
	zlib_lengthCode[255] = 28;

	dist = 0;
	for ( code = 0; code < 16; code++ ) {
		for ( n = 0; n < ( 1 << extra_dbits[code] ); n++ ) {
			zlib_distCode[dist++] = (unsigned char)code;
		}
	}
	dist >>= 7;
	for ( ; code < 30; code++ ) {
		for ( n = 0; n < ( 1 << ( extra_dbits[code] - 7 ) ); n++ ) {
			zlib_distCode[256 + dist++] = (unsigned char)code;
		}
	}

	zlib_tablesBuilt = 1;
}

typedef struct {
	unsigned short good_length;
	unsigned short max_lazy;
	unsigned short nice_length;
	unsigned short max_chain;
	int ( __cdecl *func )( int s, int flush );
} deflate_config_t;

static int __cdecl deflate_stored_thunk( int s, int flush ) { return deflate_stored( (int *)s, flush ); }
static int __cdecl deflate_slow_thunk  ( int s, int flush ) { return deflate_slow( (_DWORD *)s, flush ); }

static const deflate_config_t deflate_configuration_table[10] = {
	{  0,   0,   0,    0, deflate_stored_thunk },
	{  4,   4,   8,    4, deflate_fast         },
	{  4,   5,  16,    8, deflate_fast         },
	{  4,   6,  32,   32, deflate_fast         },
	{  4,   4,  16,   16, deflate_slow_thunk   },
	{  8,  16,  32,   32, deflate_slow_thunk   },
	{  8,  16, 128,  128, deflate_slow_thunk   },
	{  8,  32, 128,  256, deflate_slow_thunk   },
	{ 32, 128, 258, 1024, deflate_slow_thunk   },
	{ 32, 258, 258, 4096, deflate_slow_thunk   }
};

/* ---- FxPool_AllocPrimType14  0x004A10E0 ---- VERIFIED */
int *__cdecl FxPool_AllocPrimType14(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      FxPool_MakeBlockCurrent((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 560);
    return v6;
  }
  return result;
}

/* ---- FxPool_FreePrimType14  0x004A1170 ---- VERIFIED */
_DWORD *__fastcall FxPool_FreePrimType14(char *a1, int *a2)
{
  int v2;
  int v3;
  _DWORD *result;
  int v5;
  int v6;
  _DWORD *v7;
  int v8;

  result = (_DWORD *)((char *)&unk_A9CE58 + ((a1 - (char *)&unk_A9CE58) & 0xFFFF8000));
  *(_DWORD *)a1 = result[8189];
  result[8189] = a1;
  v5 = result[8188] + 1;
  result[8188] = v5;
  v6 = v5;
  v7 = (_DWORD *)a2[1];
  if ( !v7[8189] )
    return (_DWORD *)FxPool_MakeBlockCurrent((int)result, (int)a2);
  if ( v6 >= *a2 )
  {
    if ( result != v7 )
    {
LABEL_10:
      v2 = result[0x1FFF];
      if ( v2 )
        *(_DWORD *)(v2 + 32760) = result[8190];
      v3 = result[8190];
      if ( v3 )
        *(_DWORD *)(v3 + 32764) = result[0x1FFF];
      result[0x1FFF] = 0;
      result[8190] = 0;
      result[8188] = -1;
      return result;
    }
    v8 = result[8190];
    if ( v8 && *(_DWORD *)(v8 + 32752) )
    {
      a2[1] = v8;
      goto LABEL_10;
    }
  }
  return result;
}

/* ---- FxPool_AllocLight  0x004A11E0 ---- HIGH */
int *__cdecl FxPool_AllocLight(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_4920B0((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 260);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocParticle  0x004A1270 ---- HIGH */
int *__cdecl FxPool_AllocParticle(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_492110((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 316);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocLine  0x004A1300 ---- HIGH */
int *__cdecl FxPool_AllocLine(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_492170((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 328);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocElectricity  0x004A1390 ---- HIGH */
int *__cdecl FxPool_AllocElectricity(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_4921D0((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 716);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocOrientedParticle  0x004A1420 ---- HIGH */
int *__cdecl FxPool_AllocOrientedParticle(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_492230((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 328);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocTail  0x004A14B0 ---- HIGH */
int *__cdecl FxPool_AllocTail(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_492290((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 344);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocCylinder  0x004A1540 ---- HIGH */
int *__cdecl FxPool_AllocCylinder(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_4922F0((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 344);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocEmitter  0x004A15D0 ---- HIGH */
int *__cdecl FxPool_AllocEmitter(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_492350((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 396);
    return v6;
  }
  return result;
}

/* ---- FxPool_AllocFlash  0x004A1660 ---- HIGH */
int *__cdecl FxPool_AllocFlash(int *a1, int a2)
{
  int v3;
  int *v4;
  int *result;
  int *v6;
  int v7;

  v3 = a1[1];
  v4 = (int *)v3;
  if ( v3 )
  {
    while ( !v4[8189] )
    {
      v4 = (int *)v4[8190];
      if ( !v4 )
        goto LABEL_4;
    }
    if ( v4 != (int *)v3 )
      sub_4923B0((int)v4, (int)a1);
    goto LABEL_9;
  }
LABEL_4:
  result = FxMem_ClaimBlock_m(*a1, a2, v3);
  v4 = result;
  if ( result )
  {
    a1[1] = (int)result;
LABEL_9:
    v6 = (int *)v4[8189];
    v7 = v4[8188] - 1;
    v4[8189] = *v6;
    v4[8188] = v7;
    Com_Memset(v6, 0, 436);
    return v6;
  }
  return result;
}

/* ---- FxPool_MakeBlockCurrent  0x004A16F0 ---- VERIFIED */
int __cdecl FxPool_MakeBlockCurrent(int result, int a2)
{
  int v2;
  int v3;

  v2 = *(_DWORD *)(result + 32764);
  if ( v2 )
    *(_DWORD *)(v2 + 32760) = *(_DWORD *)(result + 32760);
  v3 = *(_DWORD *)(result + 32760);
  if ( v3 )
    *(_DWORD *)(v3 + 32764) = *(_DWORD *)(result + 32764);
  *(_DWORD *)(result + 32760) = 0;
  *(_DWORD *)(result + 32764) = 0;
  *(_DWORD *)(result + 32760) = *(_DWORD *)(a2 + 4);
  *(_DWORD *)(*(_DWORD *)(a2 + 4) + 32764) = result;
  *(_DWORD *)(a2 + 4) = result;
  return result;
}

/* ---- FX_RegisterEffect  0x004A1750 ---- VERIFIED */
int __cdecl FX_RegisterEffect(char *a1)
{
  return CFxScheduler__RegisterEffect(dword_14075A0, a1, 0);
}

/* ---- FX_GetBoneIndex  0x004A1760 ---- VERIFIED */
int __cdecl FX_GetBoneIndex(int entnum, const char *boneName)
{
  short handle;
  int   dobj;
  short nameHandle;

  handle = (short)com_clientDObjHandles[entnum];
  if ( !handle )
    return -1;

  /* retail 0x004A1774 `add esi, offset dobj_pool`: dobj_pool IS com_dobjPool,
   * the table common.c defines. */
  dobj = (int)( (char *)com_dobjPool + (int)handle * 0x58 );
  if ( !dobj )       /* retail 0x004A177A tests a base+offset too -- dead there as here */
    return -1;

  nameHandle = (short)SL_FindLowercaseString(boneName);
  if ( !nameHandle )
    return -1;

  return DObjFindPartIndex( dobj, nameHandle );
}

/* ---- FX_PlaySimpleEffect  0x004A17B0 ---- HIGH */
void __cdecl FX_PlaySimpleEffect(float *a1, char *a2)
{
  CFxScheduler__PlayEffect_name_simple_m(a2, dword_14075A0, a1);
}

/* ---- FX_PlayEffect  0x004A17C0 ---- HIGH */
void __cdecl FX_PlayEffect(float *a1, char *a2, int *a3)
{
  CFxScheduler__PlayEffect_name_m(a3, a2, dword_14075A0, a1);
}

/* ---- FX_PlayEntityEffect  0x004A17D0 ---- HIGH */
void __cdecl FX_PlayEntityEffect(int *a1, float *a2, int *a3, char *a4)
{
  CFxScheduler__PlayEffect_name_bolt_m(a4, dword_14075A0, a2, a3, a1);
}

/* ---- FX_PlaySimpleEffectID  0x004A17F0 ---- HIGH */
void __fastcall FX_PlaySimpleEffectID(float *a1, int a2)
{
  CFxScheduler__PlayEffect_id_simple_m(a1, a2, dword_14075A0);
}

/* ---- FX_PlayEffectID  0x004A1800 ---- VERIFIED */
void __cdecl FX_PlayEffectID(int *a1, int a2, float *a3)
{
  int v3;
  int v4;
  /* retail var_24/var_20/var_1C + right + up occupy ONE contiguous 36-byte
   * slot, frame 0x08..0x2C, and 0x004A1832 hands that base to
   * PlayEffect_id_axis as axis[3][3].  Three separate locals let MSVC place
   * rows 1 and 2 anywhere, so the callee read them off the frame. */
  int v5[9]; // [esp+8h] [ebp-24h] BYREF   (retail var_24 = axis[3][3])

  v3 = a1[1];
  v4 = a1[2];
  v5[0] = *a1;
  v5[1] = v3;
  v5[2] = v4;
  MakeNormalVectors((const float *)a1, (float *)&v5[3], (float *)&v5[6]);
  CFxScheduler__PlayEffect_id_axis(dword_14075A0, a2, a3, v5, 0);
}

/* ---- FX_PlayEntityEffectID  0x004A1850 ---- HIGH */
void __cdecl FX_PlayEntityEffectID(int *a1, float *a2, int *a3, int a4)
{
  CFxScheduler__PlayEffect_id_axis(dword_14075A0, a4, a2, a3, a1);
}

/* ---- FX_AddScheduledEffects  0x004A1870 ---- HIGH */
void FX_AddScheduledEffects()
{
  CFxScheduler__AddScheduledEffects(dword_14075A0);
}

/* ---- FX_InitSystem  0x004A1880 ---- HIGH */
int __fastcall FX_InitSystem()
{
  return FX_Init();
}

/* ---- FX_FreeSystem  0x004A1890 ---- HIGH */
int FX_FreeSystem()
{
  return (unsigned __int8)FX_Free(1);
}

/* ---- FX_FreeActive  0x004A18A0 ---- HIGH */
int FX_FreeActive()
{
  return (unsigned __int8)FX_Free(0);
}

/* ---- FX_AdjustCamera  0x004A18B0 ---- HIGH */
void __cdecl FX_AdjustCamera(int view, float zfar)
{
  SFxHelper__AdjustCamera((int)&theFxHelper, view, zfar);
}

/* ---- FX_AdjustTime  0x004A18D0 ---- HIGH */
int *__cdecl FX_AdjustTime(int a1)
{
  return SFxHelper__AdjustTime(&theFxHelper, a1);
}

/* ---- CTextPool__GetNext  0x004A18E0 ---- VERIFIED */
int __cdecl CTextPool__GetNext(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

/* ---- CTextPool__SetNext  0x004A18F0 ---- HIGH */
int __cdecl CTextPool__SetNext(int result, int a2)
{
  *(_DWORD *)(result + 4) = a2;
  return result;
}

/* ---- CTextPool__operator_new  0x004A1900 ---- HIGH */
int *__cdecl CTextPool__operator_new(size_t a1)
{
  return Z_MallocInternal(a1);
}

/* ---- CGPObject__SetNext  0x004A1920 ---- HIGH */
int __cdecl CGPObject__SetNext(int result, int a2)
{
  *(_DWORD *)(result + 4) = a2;
  return result;
}

/* ---- CGPObject__GetInOrderNext  0x004A1930 ---- HIGH */
int __cdecl CGPObject__GetInOrderNext(int a1)
{
  return *(_DWORD *)(a1 + 8);
}

/* ---- CGPObject__SetInOrderNext  0x004A1940 ---- HIGH */
int __cdecl CGPObject__SetInOrderNext(int result, int a2)
{
  *(_DWORD *)(result + 8) = a2;
  return result;
}

/* ---- CGPObject__SetInOrderPrevious  0x004A1950 ---- HIGH */
int __cdecl CGPObject__SetInOrderPrevious(int result, int a2)
{
  *(_DWORD *)(result + 12) = a2;
  return result;
}

/* ---- CGPObject__operator_new  0x004A1960 ---- HIGH */
int *__cdecl CGPObject__operator_new(size_t a1)
{
  return Z_MallocInternal(a1);
}

/* ---- CGPGroup__SetWriteable  0x004A1980 ---- HIGH */
int __cdecl CGPGroup__SetWriteable(int result, char a2)
{
  *(_BYTE *)(result + 44) = a2;
  return result;
}

/* ---- CGenericParser2__SetWriteable  0x004A1990 ---- HIGH */
int __cdecl CGenericParser2__SetWriteable(int result, char a2)
{
  *(_BYTE *)(result + 52) = a2;
  return result;
}

/* ---- GP2_GetToken  0x004A19A0 ---- CONFIRMED */
char *__cdecl GP2_GetToken(char allowLineBreaks, char readUntilNewline, char **text)
{
  char *p;
  int   len;
  char  c;
  int   newLine;

  p = *text;
  len = 0;
  gp2_token[0] = 0;
  if ( !p )
    return gp2_token;

  for ( ;; )
  {
    c = *p;
    newLine = 0;
    if ( c <= 32 )
    {
      do
      {
        if ( !c )
        {
          *text = 0;
          return gp2_token;
        }
        if ( c == '\n' )
          newLine = 1;
        c = *++p;
      }
      while ( c <= 32 );

      if ( newLine && !allowLineBreaks )
        break;
    }

    c = *p;
    if ( c == '/' && p[1] == '/' )
    {
      c = p[2];
      p += 2;
      while ( c && c != '\n' )
        c = *++p;
      continue;
    }
    if ( c == '/' && p[1] == '*' )
    {
      c = p[2];
      p += 2;
      if ( c )
      {
        while ( c != '*' || p[1] != '/' )
        {
          c = *++p;
          if ( !c )
            goto nextToken;
        }
        if ( *p )
          p += 2;
      }
      continue;
    }

    if ( c == '"' && !readUntilNewline )
    {
      c = p[1];
      p += 2;
      if ( c != '"' )
      {
        do
        {
          if ( !c )
            break;
          if ( len < GP2_MAX_TOKEN_SIZE )
            gp2_token[len++] = c;
          c = *p++;
        }
        while ( c != '"' );
        goto trimQuotes;
      }
      goto terminate;
    }

    if ( !readUntilNewline )
    {
      if ( c > 32 )
      {
        do
        {
          if ( len < GP2_MAX_TOKEN_SIZE )
            gp2_token[len++] = c;
          c = *++p;
        }
        while ( c > 32 );
        goto trimQuotes;
      }
      goto terminate;
    }

    if ( c == '\n' )
      goto terminate;
    do
    {
      if ( c == '\r' )
        break;
      if ( c == '/' && ( p[1] == '/' || p[1] == '*' ) )
        break;
      if ( len < GP2_MAX_TOKEN_SIZE )
        gp2_token[len++] = c;
      c = *++p;
    }
    while ( c != '\n' );

    while ( len && gp2_token[len - 1] < 32 )
      --len;

trimQuotes:
    if ( gp2_token[0] == '"' )
    {
      --len;
      memcpy( gp2_token, gp2_token + 1, len );
      if ( len && gp2_token[len - 1] == '"' )
        --len;
    }
    if ( len >= GP2_MAX_TOKEN_SIZE )
      len = 0;

terminate:
    gp2_token[len] = 0;
    break;

nextToken:
    ;
  }

  *text = p;
  return gp2_token;
}

/* ---- CTextPool__CTextPool  0x004A1B40 ---- HIGH */
void **__cdecl CTextPool__CTextPool(size_t a1, void **a2)
{
  a2[1] = 0;
  a2[2] = (void *)a1;
  a2[3] = 0;
  *a2 = Z_MallocInternal(a1);
  return a2;
}

/* ---- CTextPool__dtor  0x004A1B60 ---- HIGH */
void __cdecl CTextPool__dtor(void **a1)
{
  free(*a1);
}

/* ---- CTextPool__AllocText  0x004A1B70 ---- CONFIRMED */
int __cdecl CTextPool__AllocText(int *this, const char *a2, int a3, int a4)
{
  const char *v5;
  int v6;
  int v7;
  int *v8;
  int *v9;
  int v11;
  int v12;
  size_t v13;

  v5 = &a2[strlen(a2) + 1];
  v6 = v5 - (a2 + 1) + ((_BYTE)a3 != 0);
  v7 = this[3];
  if ( v7 + v5 - a2 + ((_BYTE)a3 != 0) <= this[2] )
  {
    strcpy((char *)(v7 + *this), a2);
    v11 = *this;
    v12 = v6 + this[3];
    this[3] = v12;
    *(_BYTE *)(v12 + v11) = 0;
    return *this + this[3] - v6;
  }
  else if ( a4 )
  {
    v8 = Z_MallocInternal(0x10u);
    if ( v8 )
    {
      v13 = this[2];
      v8[1] = 0;
      v8[2] = v13;
      v8[3] = 0;
      *v8 = (int)Z_MallocInternal(v13);
    }
    else
    {
      v8 = 0;
    }
    *(_DWORD *)(*(_DWORD *)a4 + 4) = v8;
    v9 = *(int **)(*(_DWORD *)a4 + 4);
    *(_DWORD *)a4 = v9;
    return CTextPool__AllocText(v9, a2, a3, 0);
  }
  else
  {
    return 0;
  }
}

/* ---- CTextPool__DeleteChain  0x004A1C60 ---- HIGH */
void __cdecl CTextPool__DeleteChain(int a1)
{
  int v1;
  int v2;

  v1 = a1;
  if ( a1 )
  {
    do
    {
      v2 = *(_DWORD *)(v1 + 4);
      free(*(void **)v1);
      free((void *)v1);
      v1 = v2;
    }
    while ( v2 );
  }
}

/* ---- CTextPool__scalar_dtor  0x004A1C90 ---- HIGH */
void **__cdecl CTextPool__scalar_dtor(void **a1, char a2)
{
  free(*a1);
  if ( (a2 & 1) != 0 )
    free(a1);
  return a1;
}

/* ---- CGPObject__CGPObject  0x004A1CC0 ---- HIGH */
_DWORD *__cdecl CGPObject__CGPObject(_DWORD *result, int a2)
{
  *result = a2;
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;
  return result;
}

/* ---- CGPObject__WriteText  0x004A1CD0 ---- HIGH */
char __cdecl CGPObject__WriteText(const char *a1, int **a2, int a3)
{
  if ( strchr(a1, 32) || !*a1 )
  {
    CTextPool__AllocText(*a2, "\"", 0, (int)a2);
    CTextPool__AllocText(*a2, a1, 0, (int)a2);
    CTextPool__AllocText(*a2, "\"", 0, (int)a2);
    return 1;
  }
  else
  {
    CTextPool__AllocText(*a2, a1, 0, (int)a2);
    return 1;
  }
}

/* ---- CGPValue__CGPValue  0x004A1D30 ---- HIGH */
_DWORD *__cdecl CGPValue__CGPValue(int a1, _DWORD *a2, const char *a3)
{
  *a2 = a1;
  a2[1] = 0;
  a2[2] = 0;
  a2[3] = 0;
  a2[4] = 0;
  if ( a3 )
    CGPValue__AddValue(0, a3, (int)a2);
  return a2;
}

/* ---- CGPValue__Clean  0x004A1D50 ---- HIGH */
int __cdecl CGPValue__Clean(int a1)
{
  int result;
  int v2;

  result = *(_DWORD *)(a1 + 16);
  if ( result )
  {
    do
    {
      v2 = *(_DWORD *)(*(_DWORD *)(a1 + 16) + 4);
      free(*(void **)(a1 + 16));
      result = v2;
      *(_DWORD *)(a1 + 16) = v2;
    }
    while ( v2 );
  }
  return result;
}

/* ---- CGPValue__Duplicate  0x004A1D80 ---- HIGH */
int *__cdecl CGPValue__Duplicate(int this, int **a2)
{
  int **v2;
  const char *v4;
  int *v5;
  int *v6;
  const char **v7;
  int v9;
  int *v10;
  int *v11;
  int *v12;
  int *v13;
  char *v14;
  char *LocalizedString_m;
  const char **v16;

  v2 = a2;
  if ( a2 )
    v4 = (const char *)CTextPool__AllocText(*a2, *(const char **)this, 1, (int)a2);
  else
    v4 = *(const char **)this;
  v5 = Z_MallocInternal(0x14u);
  if ( v5 )
  {
    *v5 = (int)v4;
    v5[1] = 0;
    v5[2] = 0;
    v5[3] = 0;
    v5[4] = 0;
    v6 = v5;
  }
  else
  {
    v6 = 0;
  }
  v7 = *(const char ***)(this + 16);
  v16 = v7;
  if ( v7 )
  {
    while ( 1 )
    {
      v9 = v2 ? CTextPool__AllocText(*v2, *v7, 1, (int)v2) : (int)*v7;
      if ( v6[4] )
      {
        v12 = (int *)malloc(0x10u);
        v13 = v12;
        if ( !v12 )
        {
LABEL_20:
          Sys_OutOfMemoryPrep();
          LocalizedString_m = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_TITLE");
          v14 = SEH_GetLocalizedString_m("WIN_OUT_OF_MEM_BODY");
          MessageBoxA(0, v14, LocalizedString_m, 0x10u);
          exit(-1);
        }
        Com_Memset(v12, 0, 16);
        *v13 = v9;
        v13[1] = 0;
        v13[2] = 0;
        v13[3] = 0;
        *(_DWORD *)(*(_DWORD *)(v6[4] + 8) + 4) = v13;
        *(_DWORD *)(v6[4] + 8) = *(_DWORD *)(*(_DWORD *)(v6[4] + 8) + 4);
      }
      else
      {
        v10 = (int *)malloc(0x10u);
        v11 = v10;
        if ( !v10 )
          goto LABEL_20;
        Com_Memset(v10, 0, 16);
        *v11 = v9;
        v11[1] = 0;
        v11[2] = 0;
        v11[3] = 0;
        v6[4] = (int)v11;
        v11[2] = (int)v11;
      }
      v16 = (const char **)v16[1];
      if ( !v16 )
        break;
      v7 = v16;
      v2 = a2;
    }
  }
  return v6;
}

/* ---- CGPValue__IsList  0x004A1EE0 ---- HIGH */
bool __cdecl CGPValue__IsList(int a1)
{
  int v1;

  v1 = *(_DWORD *)(a1 + 16);
  return v1 && *(_DWORD *)(v1 + 4);
}

/* ---- CGPValue__GetTopValue  0x004A1F00 ---- HIGH */
int __cdecl CGPValue__GetTopValue(int a1)
{
  int v1;

  v1 = *(_DWORD *)(a1 + 16);
  if ( v1 )
    return *(_DWORD *)v1;
  else
    return 0;
}

/* ---- CGPValue__AddValue  0x004A1F10 ---- HIGH */
int __cdecl CGPValue__AddValue(int textPool, const char *newValue, int this_)
{
  int *obj;
  int *list;

  if ( textPool )
    newValue = (const char *)CTextPool__AllocText( *(int **)textPool, newValue, 1, textPool );

  obj = Z_MallocInternal( 0x10u );
  if ( obj )
  {
    obj[0] = (int)newValue;
    obj[1] = 0;
    obj[2] = 0;
    obj[3] = 0;
  }

  list = *(int **)( this_ + 16 );
  if ( list )
  {
    *(_DWORD *)( list[2] + 4 ) = (int)obj;
    list[2] = *(_DWORD *)( list[2] + 4 );
    return (int)list;
  }

  *(_DWORD *)( this_ + 16 ) = (int)obj;
  if ( obj )
    obj[2] = (int)obj;
  return (int)obj;
}

/* ---- CGPValue__Parse  0x004A1F90 ---- CONFIRMED */
char __cdecl CGPValue__Parse(int **a1, int a2, char **a3)
{
  char *v3;
  const char *v4;

  v3 = GP2_GetToken(1, 1, a3);
  if ( !*v3 )
    return 0;
  while ( _stricmp(v3, "]") )
  {
    v4 = (const char *)CTextPool__AllocText(*a1, v3, 1, (int)a1);
    CGPValue__AddValue(0, v4, a2);
    v3 = GP2_GetToken(1, 1, a3);
    if ( !*v3 )
      return 0;
  }
  return 1;
}

/* ---- CGPValue__Write  0x004A2000 ---- CONFIRMED */
char __cdecl CGPValue__Write(int a1, int **a2, int a3)
{
  int v4;
  int v5;
  const char *v6;
  int *v7;
  const char *v8;
  int v10;
  int v11;
  int v12;
  const char *v13;
  int v14;
  const char *v15;
  const char *v16;
  int **v17;
  int **v18;

  if ( !*(_DWORD *)(a1 + 16) )
    return 1;
  v4 = a3;
  if ( a3 > 0 )
  {
    v5 = a3;
    do
    {
      CTextPool__AllocText(*a2, "\t", 0, (int)a2);
      --v5;
    }
    while ( v5 );
  }
  v6 = *(const char **)a1;
  if ( strchr(*(const char **)a1, 32) || !*v6 )
  {
    CTextPool__AllocText(*a2, "\"", 0, (int)a2);
    CTextPool__AllocText(*a2, v6, 0, (int)a2);
    v17 = a2;
    v15 = "\"";
  }
  else
  {
    v17 = a2;
    v15 = v6;
  }
  CTextPool__AllocText(*a2, v15, 0, (int)v17);
  v7 = *a2;
  if ( *(_DWORD *)(*(_DWORD *)(a1 + 16) + 4) )
  {
    CTextPool__AllocText(v7, "\r\n", 0, (int)a2);
    if ( a3 > 0 )
    {
      v10 = a3;
      do
      {
        CTextPool__AllocText(*a2, "\t", 0, (int)a2);
        --v10;
      }
      while ( v10 );
    }
    CTextPool__AllocText(*a2, "[\r\n", 0, (int)a2);
    v11 = *(_DWORD *)(a1 + 16);
    if ( v11 )
    {
      do
      {
        if ( a3 + 1 > 0 )
        {
          v12 = a3 + 1;
          do
          {
            CTextPool__AllocText(*a2, "\t", 0, (int)a2);
            --v12;
          }
          while ( v12 );
        }
        v13 = *(const char **)v11;
        if ( strchr(*(const char **)v11, 32) || !*v13 )
        {
          CTextPool__AllocText(*a2, "\"", 0, (int)a2);
          CTextPool__AllocText(*a2, v13, 0, (int)a2);
          v18 = a2;
          v16 = "\"";
        }
        else
        {
          v18 = a2;
          v16 = v13;
        }
        CTextPool__AllocText(*a2, v16, 0, (int)v18);
        CTextPool__AllocText(*a2, "\r\n", 0, (int)a2);
        v11 = *(_DWORD *)(v11 + 4);
      }
      while ( v11 );
      v4 = a3;
    }
    if ( v4 > 0 )
    {
      v14 = v4;
      do
      {
        CTextPool__AllocText(*a2, "\t", 0, (int)a2);
        --v14;
      }
      while ( v14 );
    }
    CTextPool__AllocText(*a2, "]\r\n", 0, (int)a2);
    return 1;
  }
  CTextPool__AllocText(v7, "\t\t", 0, (int)a2);
  v8 = **(const char ***)(a1 + 16);
  if ( strchr(v8, 32) || !*v8 )
  {
    CTextPool__AllocText(*a2, "\"", 0, (int)a2);
    CTextPool__AllocText(*a2, v8, 0, (int)a2);
    CTextPool__AllocText(*a2, "\"", 0, (int)a2);
    CTextPool__AllocText(*a2, "\r\n", 0, (int)a2);
    return 1;
  }
  else
  {
    CTextPool__AllocText(*a2, v8, 0, (int)a2);
    CTextPool__AllocText(*a2, "\r\n", 0, (int)a2);
    return 1;
  }
}

/* ---- CGPGroup__CGPGroup  0x004A2200 ---- HIGH */
int __cdecl CGPGroup__CGPGroup(int result, int a2, int a3)
{
  *(_DWORD *)result = a3;
  *(_DWORD *)(result + 4) = 0;
  *(_DWORD *)(result + 8) = 0;
  *(_DWORD *)(result + 12) = 0;
  *(_DWORD *)(result + 16) = 0;
  *(_DWORD *)(result + 20) = 0;
  *(_DWORD *)(result + 24) = 0;
  *(_DWORD *)(result + 28) = 0;
  *(_DWORD *)(result + 32) = 0;
  *(_DWORD *)(result + 36) = 0;
  *(_DWORD *)(result + 40) = a2;
  *(_BYTE *)(result + 44) = 0;
  return result;
}

/* ---- CGPGroup__dtor  0x004A2230 ---- HIGH */
int __cdecl CGPGroup__dtor(int this)
{
  return CGPGroup__Clean(this);
}

/* ---- CGPGroup__GetNumSubGroups  0x004A2240 ---- HIGH */
int __cdecl CGPGroup__GetNumSubGroups(_DWORD *this)
{
  int v1;
  int result;

  v1 = this[7];
  result = 0;
  do
  {
    v1 = *(_DWORD *)(v1 + 4);
    ++result;
  }
  while ( v1 );
  return result;
}

/* ---- CGPGroup__GetNumPairs  0x004A2250 ---- HIGH */
int __cdecl CGPGroup__GetNumPairs(_DWORD *this)
{
  int v1;
  int result;

  v1 = this[4];
  result = 0;
  do
  {
    v1 = *(_DWORD *)(v1 + 4);
    ++result;
  }
  while ( v1 );
  return result;
}

/* ---- CGPGroup__Clean  0x004A2260 ---- HIGH */
int __cdecl CGPGroup__Clean(int this)
{
  int result;
  int v3;
  int v4;
  _DWORD *v5;

  for ( result = *(_DWORD *)(this + 16); result; *(_DWORD *)(this + 16) = result )
  {
    v3 = *(_DWORD *)(this + 16);
    *(_DWORD *)(this + 24) = *(_DWORD *)(v3 + 4);
    if ( v3 )
    {
      if ( *(_DWORD *)(v3 + 16) )
      {
        do
        {
          v4 = *(_DWORD *)(*(_DWORD *)(v3 + 16) + 4);
          free(*(void **)(v3 + 16));
          *(_DWORD *)(v3 + 16) = v4;
        }
        while ( v4 );
      }
      free((void *)v3);
    }
    result = *(_DWORD *)(this + 24);
  }
  if ( *(_DWORD *)(this + 28) )
  {
    do
    {
      v5 = *(_DWORD **)(this + 28);
      *(_DWORD *)(this + 36) = v5[1];
      if ( v5 )
      {
        CGPGroup__Clean((int)v5);
        free(v5);
      }
      result = *(_DWORD *)(this + 36);
      *(_DWORD *)(this + 28) = result;
    }
    while ( result );
  }
  *(_DWORD *)(this + 24) = 0;
  *(_DWORD *)(this + 20) = 0;
  *(_DWORD *)(this + 16) = 0;
  *(_DWORD *)(this + 36) = 0;
  *(_DWORD *)(this + 32) = 0;
  *(_DWORD *)(this + 28) = 0;
  *(_DWORD *)(this + 40) = 0;
  *(_BYTE *)(this + 44) = 0;
  return result;
}

/* ---- CGPValue__scalar_dtor  0x004A2300 ---- HIGH */
void **__cdecl CGPValue__scalar_dtor(void **a1, char a2)
{
  void *v2;

  if ( a1[4] )
  {
    do
    {
      v2 = (void *)*((_DWORD *)a1[4] + 1);
      free(a1[4]);
      a1[4] = v2;
    }
    while ( v2 );
  }
  if ( (a2 & 1) != 0 )
    free(a1);
  return a1;
}

/* ---- CGPGroup__scalar_dtor  0x004A2340 ---- HIGH */
void *__cdecl CGPGroup__scalar_dtor(void *a1, char a2)
{
  CGPGroup__Clean((int)a1);
  if ( (a2 & 1) != 0 )
    free(a1);
  return a1;
}

/* ---- CGPGroup__Duplicate  0x004A2360 ---- CONFIRMED */
int *__cdecl CGPGroup__Duplicate(int this, int **a2, int a3)
{
  const char *v4;
  int *v5;
  int *v6;
  int i;
  int *v8;
  int j;
  int *v10;

  if ( a2 )
    v4 = (const char *)CTextPool__AllocText(*a2, *(const char **)this, 1, (int)a2);
  else
    v4 = *(const char **)this;
  v5 = Z_MallocInternal(0x30u);
  if ( v5 )
  {
    *v5 = (int)v4;
    v5[1] = 0;
    v5[2] = 0;
    v5[3] = 0;
    v5[4] = 0;
    v5[5] = 0;
    v5[6] = 0;
    v5[7] = 0;
    v5[8] = 0;
    v5[9] = 0;
    v5[10] = 0;
    *((_BYTE *)v5 + 44) = 0;
    v6 = v5;
  }
  else
  {
    v6 = 0;
  }
  for ( i = *(_DWORD *)(this + 28); i; i = *(_DWORD *)(i + 4) )
  {
    v8 = CGPGroup__Duplicate(i, a2, (int)v6);
    CGP_InsertInOrder(v6 + 7, (int)v8, (int)v6, v6 + 8, (int)(v6 + 9));
  }
  for ( j = *(_DWORD *)(this + 16); j; j = *(_DWORD *)(j + 4) )
  {
    v10 = CGPValue__Duplicate(j, a2);
    CGP_InsertInOrder(v6 + 4, (int)v10, (int)v6, v6 + 5, (int)(v6 + 6));
  }
  return v6;
}

/* ---- CGP_InsertInOrder  0x004A2430 ---- HIGH */
_DWORD *__cdecl CGP_InsertInOrder(_DWORD *listHead, int newObj, int parent,
                                  _DWORD *inOrderHead, int currentSlot)
{
  _DWORD *result;
  int walk;
  int prev;

  if ( !*listHead )
  {
    *inOrderHead = newObj;
    *listHead = newObj;
    *(_DWORD *)currentSlot = newObj;
    return listHead;
  }

  result = inOrderHead;
  *(_DWORD *)( *(_DWORD *)currentSlot + 4 ) = newObj;

  walk = *inOrderHead;
  prev = 0;
  if ( walk )
  {
    for ( ;; )
    {
      result = (_DWORD *)_stricmp( *(const char **)newObj, *(const char **)walk );
      if ( (int)result < 0 )
      {
        *(_DWORD *)( walk + 12 ) = newObj;
        *(_DWORD *)( newObj + 8 ) = walk;
        break;
      }
      prev = walk;
      walk = *(_DWORD *)( walk + 8 );
      if ( !walk )
        break;
    }
  }

  if ( prev )
  {
    *(_DWORD *)( prev + 8 ) = newObj;
    *(_DWORD *)( newObj + 12 ) = prev;
    *(_DWORD *)currentSlot = newObj;
  }
  else
  {
    *inOrderHead = newObj;
    *(_DWORD *)currentSlot = newObj;
  }
  return result;
}

/* ---- CGPGroup__AddPair  0x004A24B0 ---- HIGH */
int __cdecl CGPGroup__AddPair(const char *a1, int **a2, int a3, const char *a4)
{
  const char *v4;
  int *v6;
  int v7;

  v4 = a4;
  if ( a2 )
  {
    v4 = (const char *)CTextPool__AllocText(*a2, a4, 1, (int)a2);
    if ( a1 )
      a1 = (const char *)CTextPool__AllocText(*a2, a1, 1, (int)a2);
  }
  v6 = Z_MallocInternal(0x14u);
  if ( v6 )
  {
    *v6 = (int)v4;
    v6[1] = 0;
    v6[2] = 0;
    v6[3] = 0;
    v6[4] = 0;
    if ( a1 )
      CGPValue__AddValue(0, a1, (int)v6);
    v7 = (int)v6;
  }
  else
  {
    v7 = 0;
  }
  CGP_InsertInOrder((_DWORD *)(a3 + 16), v7, a3, (_DWORD *)(a3 + 20), a3 + 24);
  return v7;
}

/* ---- CGPGroup__AddPairObject  0x004A2560 ---- HIGH */
_DWORD *__cdecl CGPGroup__AddPairObject(int a1, int a2)
{
  return CGP_InsertInOrder((_DWORD *)(a1 + 16), a2, a1, (_DWORD *)(a1 + 20), a1 + 24);
}

/* ---- CGPGroup__AddGroup  0x004A2580 ---- HIGH */
int __cdecl CGPGroup__AddGroup(int **a1, const char *a2, int a3)
{
  const char *v3;
  int *v4;
  int v5;

  v3 = a2;
  if ( a1 )
    v3 = (const char *)CTextPool__AllocText(*a1, a2, 1, (int)a1);
  v4 = Z_MallocInternal(0x30u);
  if ( v4 )
  {
    *v4 = (int)v3;
    v4[1] = 0;
    v4[2] = 0;
    v4[3] = 0;
    v4[4] = 0;
    v4[5] = 0;
    v4[6] = 0;
    v4[7] = 0;
    v4[8] = 0;
    v4[9] = 0;
    v4[10] = 0;
    *((_BYTE *)v4 + 44) = 0;
    v5 = (int)v4;
  }
  else
  {
    v5 = 0;
  }
  CGP_InsertInOrder((_DWORD *)(a3 + 28), v5, a3, (_DWORD *)(a3 + 32), a3 + 36);
  return v5;
}

/* ---- CGPGroup__AddGroupObject  0x004A25F0 ---- HIGH */
_DWORD *__cdecl CGPGroup__AddGroupObject(int a1, int a2)
{
  return CGP_InsertInOrder((_DWORD *)(a1 + 28), a2, a1, (_DWORD *)(a1 + 32), a1 + 36);
}

/* ---- CGPGroup__FindSubGroup  0x004A2610 ---- HIGH */
int __cdecl CGPGroup__FindSubGroup(int a1, const char *a2)
{
  int v2;

  v2 = *(_DWORD *)(a1 + 28);
  if ( !v2 )
    return 0;
  while ( _stricmp(a2, *(const char **)v2) )
  {
    v2 = *(_DWORD *)(v2 + 4);
    if ( !v2 )
      return 0;
  }
  return v2;
}

/* ---- CGPGroup__Parse  0x004A2640 ---- CONFIRMED */
char __cdecl CGPGroup__Parse(_BYTE *this, char **a2, int **a3)
{
  _BYTE *v3;
  char *v4;
  char *v5;
  _BYTE *v6;
  int v8;
  char v10[1024];

  v3 = this;
  v4 = GP2_GetToken(1, 0, a2);
  if ( *v4 )
  {
    while ( _stricmp(v4, "}") )
    {
      strcpy(v10, v4);
      v5 = GP2_GetToken(1, 1, a2);
      if ( !_stricmp(v5, "{") )
      {
        v6 = (_BYTE *)CGPGroup__AddGroup(a3, v10, (int)v3);
        v6[44] = v3[44];
        if ( !CGPGroup__Parse(v6, a2, a3) )
          return 0;
      }
      else if ( !_stricmp(v5, "[") )
      {
        v8 = CGPGroup__AddPair(0, a3, (int)v3, v10);
        if ( !CGPValue__Parse(a3, v8, a2) )
          return 0;
        v3 = this;
      }
      else
      {
        CGPGroup__AddPair(v5, a3, (int)v3, v10);
      }
      v4 = GP2_GetToken(1, 0, a2);
      if ( !*v4 )
        goto LABEL_11;
    }
  }
  else
  {
LABEL_11:
    if ( *((_DWORD *)v3 + 10) )
      return 0;
  }
  return 1;
}

/* ---- CGPGroup__Write  0x004A2790 ---- CONFIRMED */
char __cdecl CGPGroup__Write(const char **this, int **a2, int a3)
{
  int v3;
  char *v6;
  const char *v7;
  const char *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  const char *v14;
  int **v15;
  int v16;

  v3 = a3;
  v6 = (char *)this[7];
  v7 = this[4];
  if ( a3 >= 0 )
  {
    if ( a3 > 0 )
    {
      v16 = a3;
      do
      {
        CTextPool__AllocText(*a2, "\t", 0, (int)a2);
        --v16;
      }
      while ( v16 );
    }
    v8 = *this;
    if ( strchr(v8, 32) || !*v8 )
    {
      CTextPool__AllocText(*a2, "\"", 0, (int)a2);
      CTextPool__AllocText(*a2, v8, 0, (int)a2);
      v15 = a2;
      v14 = "\"";
    }
    else
    {
      v15 = a2;
      v14 = v8;
    }
    CTextPool__AllocText(*a2, v14, 0, (int)v15);
    CTextPool__AllocText(*a2, "\r\n", 0, (int)a2);
    v9 = a3;
    if ( a3 > 0 )
    {
      do
      {
        CTextPool__AllocText(*a2, "\t", 0, (int)a2);
        --v9;
      }
      while ( v9 );
    }
    CTextPool__AllocText(*a2, "{\r\n", 0, (int)a2);
    v3 = a3;
  }
  if ( v7 )
  {
    v10 = v3 + 1;
    do
    {
      CGPValue__Write((int)v7, a2, v10);
      v7 = (const char *)*((_DWORD *)v7 + 1);
    }
    while ( v7 );
    v3 = a3;
  }
  if ( v6 )
  {
    v11 = v3 + 1;
    do
    {
      CGPGroup__Write((const char **)v6, a2, v11);
      v6 = (char *)*((_DWORD *)v6 + 1);
    }
    while ( v6 );
    v3 = a3;
  }
  if ( v3 >= 0 )
  {
    if ( v3 > 0 )
    {
      v12 = v3;
      do
      {
        CTextPool__AllocText(*a2, "\t", 0, (int)a2);
        --v12;
      }
      while ( v12 );
    }
    CTextPool__AllocText(*a2, "}\r\n", 0, (int)a2);
  }
  return 1;
}

/* ---- CGPGroup__FindPair  0x004A28C0 ---- HIGH */
int __cdecl CGPGroup__FindPair(int a1, const char *a2)
{
  int v2;

  v2 = *(_DWORD *)(a1 + 16);
  if ( !v2 )
    return 0;
  while ( _stricmp(*(const char **)v2, a2) )
  {
    v2 = *(_DWORD *)(v2 + 4);
    if ( !v2 )
      return 0;
  }
  return v2;
}

/* ---- CGPGroup__FindPairValue  0x004A28F0 ---- HIGH */
int __cdecl CGPGroup__FindPairValue(int a1, const char *a2, int a3)
{
  int v3;
  int v5;

  v3 = *(_DWORD *)(a1 + 16);
  if ( !v3 )
    return a3;
  while ( _stricmp(*(const char **)v3, a2) )
  {
    v3 = *(_DWORD *)(v3 + 4);
    if ( !v3 )
      return a3;
  }
  v5 = *(_DWORD *)(v3 + 16);
  if ( v5 )
    return *(_DWORD *)v5;
  else
    return 0;
}

/* ---- CGenericParser2__CGenericParser2  0x004A2930 ---- CONFIRMED */
int __cdecl CGenericParser2__CGenericParser2(int result)
{
  *(_DWORD *)result = "Top Level";
  *(_DWORD *)(result + 4) = 0;
  *(_DWORD *)(result + 8) = 0;
  *(_DWORD *)(result + 12) = 0;
  *(_DWORD *)(result + 16) = 0;
  *(_DWORD *)(result + 20) = 0;
  *(_DWORD *)(result + 24) = 0;
  *(_DWORD *)(result + 28) = 0;
  *(_DWORD *)(result + 32) = 0;
  *(_DWORD *)(result + 36) = 0;
  *(_DWORD *)(result + 40) = 0;
  *(_BYTE *)(result + 44) = 0;
  *(_DWORD *)(result + 48) = 0;
  *(_BYTE *)(result + 52) = 0;
  return result;
}

/* ---- CGenericParser2__Clean  0x004A2960 ---- HIGH */
int __cdecl CGenericParser2__Clean(int a1)
{
  CGenericParser2__CleanTextPool(a1);
  return CGPGroup__Clean(a1);
}

/* ---- CGenericParser2__Parse  0x004A29B0 ---- HIGH */
char __cdecl CGenericParser2__Parse(int a1, char **a2, int a3, char a4)
{
  int *v4;
  int *v5;
  char v6;

  if ( (_BYTE)a3 )
    CGenericParser2__CleanTextPool(a1);
  if ( !*(_DWORD *)(a1 + 48) )
  {
    v4 = Z_MallocInternal(0x10u);
    v5 = v4;
    if ( v4 )
    {
      v4[1] = 0;
      v4[2] = 10240;
      v4[3] = 0;
      *v4 = (int)Z_MallocInternal(0x2800u);
    }
    else
    {
      v5 = 0;
    }
    *(_DWORD *)(a1 + 48) = v5;
  }
  v6 = a4;
  *(_BYTE *)(a1 + 52) = a4;
  *(_BYTE *)(a1 + 44) = v6;
  a3 = *(_DWORD *)(a1 + 48);
  return CGPGroup__Parse((_BYTE *)a1, a2, (int **)&a3);
}

/* ---- CGenericParser2__CleanTextPool  0x004A2A60 ---- HIGH */
void __cdecl CGenericParser2__CleanTextPool(int this_)
{
  int pool;
  int next;

  CGPGroup__Clean( this_ );

  pool = *(_DWORD *)( this_ + 48 );
  if ( pool )
  {
    do
    {
      next = *(_DWORD *)( pool + 4 );
      free( *(void **)pool );
      free( (void *)pool );
      pool = next;
    }
    while ( next );
  }
  *(_DWORD *)( this_ + 48 ) = 0;
}

/* ---- CGenericParser2__Write  0x004A2AA0 ---- HIGH */
char __cdecl CGenericParser2__Write(int a1, int *a2)
{
  int v2;
  int i;

  v2 = *(_DWORD *)(a1 + 28);
  for ( i = *(_DWORD *)(a1 + 16); i; i = *(_DWORD *)(i + 4) )
    CGPValue__Write(i, &a2, 0);
  for ( ; v2; v2 = *(_DWORD *)(v2 + 4) )
    CGPGroup__Write((const char **)v2, &a2, 0);
  return 1;
}

/* ---- adler32  0x004A2AF0 ---- VERIFIED */
int __cdecl adler32(unsigned int a1, unsigned __int8 *a2, unsigned int a3)
{
  unsigned __int8 *v3;
  unsigned int v4;
  unsigned int v5;
  unsigned int i;
  int v8;
  unsigned int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;
  int v22;
  int v23;
  int v24;
  int v25;
  int v26;
  int v27;
  int v28;
  int v29;
  int v30;
  int v31;
  int v32;
  int v33;
  int v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;

  v3 = a2;
  v4 = (unsigned __int16)a1;
  v5 = HIWORD(a1);
  if ( !a2 )
    return 1;
  for ( i = a3; i; v5 %= 0xFFF1u )
  {
    v8 = i;
    if ( i >= 0x15B0 )
      v8 = 5552;
    i -= v8;
    if ( v8 >= 16 )
    {
      v9 = (unsigned int)v8 >> 4;
      v8 += -16 * ((unsigned int)v8 >> 4);
      do
      {
        v10 = *v3 + v4;
        v11 = v10 + v5;
        v12 = v3[1] + v10;
        v13 = v12 + v11;
        v14 = v3[2] + v12;
        v15 = v14 + v13;
        v16 = v3[3] + v14;
        v17 = v16 + v15;
        v18 = v3[4] + v16;
        v19 = v18 + v17;
        v20 = v3[5] + v18;
        v21 = v20 + v19;
        v22 = v3[6] + v20;
        v23 = v22 + v21;
        v24 = v3[7] + v22;
        v25 = v24 + v23;
        v26 = v3[8] + v24;
        v27 = v26 + v25;
        v28 = v3[9] + v26;
        v29 = v28 + v27;
        v30 = v3[10] + v28;
        v31 = v30 + v29;
        v32 = v3[11] + v30;
        v33 = v32 + v31;
        v34 = v3[12] + v32;
        v35 = v34 + v33;
        v36 = v3[13] + v34;
        v37 = v36 + v35;
        v38 = v3[14] + v36;
        v39 = v38 + v37;
        v4 = v3[15] + v38;
        v5 = v4 + v39;
        v3 += 16;
        --v9;
      }
      while ( v9 );
    }
    for ( ; v8; --v8 )
    {
      v4 += *v3++;
      v5 += v4;
    }
    v4 %= 0xFFF1u;
  }
  return v4 | (v5 << 16);
}

/* ---- compress2  0x004A2C20 ---- VERIFIED */
unsigned int __cdecl compress2(int a1, int a2, int a3, int *a4, unsigned int a5)
{
  int v5;
  unsigned int result;
  int v7;
  _DWORD v8[14];

  v8[0] = a1;
  v5 = *a4;
  v8[1] = a3;
  v8[3] = a2;
  v8[4] = v5;
  memset(&v8[8], 0, 12);
  result = deflateInit2_("1.1.4", v8, 15, a5, 8, 8, 0, 56);
  if ( !result )
  {
    v7 = deflate((int)v8, 4u);
    if ( v7 == 1 )
    {
      *a4 = v8[5];
      return deflateEnd((int)v8);
    }
    else
    {
      deflateEnd((int)v8);
      result = -5;
      if ( v7 )
        return v7;
    }
  }
  return result;
}

/* ---- compress  0x004A2CC0 ---- CONFIRMED */
unsigned int __cdecl compress(int *a1, int a2, int a3, int a4)
{
  return compress2(a4, a2, a3, a1, 0xFFFFFFFF);
}

/* ---- get_crc_table  0x004A2CD0 ---- CONFIRMED */
int *get_crc_table()
{
  return dword_5413D0;
}

/* ---- crc32  0x004A2CE0 ---- VERIFIED */
int __cdecl crc32(int crc, unsigned int len, _BYTE *buf)
{
  unsigned int c;

  if ( !buf )
    return 0;

  zlib_BuildTables();

  c = ~(unsigned int)crc;
  while ( len-- )
    c = zlib_crcTable[(unsigned __int8)(c ^ *buf++)] ^ (c >> 8);

  return (int)~c;
}

/* ---- deflateInit_  0x004A2E00 ---- VERIFIED */
int __cdecl deflateInit_(int a1, _DWORD *a2, unsigned int a3, _BYTE *a4)
{
  return deflateInit2_(a4, a2, 15, a3, 8, 8, 0, a1);
}

/* ---- deflateInit2_  0x004A2E20 ---- CONFIRMED */
int __cdecl deflateInit2_(
        _BYTE *a1,
        _DWORD *a2,
        int a3,
        unsigned int a4,
        int a5,
        int a6,
        unsigned int a7,
        int a8)
{
  int v9;
  bool v11;
  unsigned int v12;
  _DWORD *v13;
  _DWORD *v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  unsigned int v20;
  int v21;
  _DWORD *v23;
  bool v24;
  int v27;

  zlib_BuildTables();
  v9 = 0;
  if ( !a1 || *a1 != '1' || a8 != 56 )
    return -6;
  if ( !a2 )
    return -2;
  v11 = a2[8] == 0;
  a2[6] = 0;
  if ( v11 )
  {
    a2[8] = (int)zcalloc;
    a2[10] = 0;
  }
  if ( !a2[9] )
    a2[9] = (int)zcfree;
  v12 = a4;
  if ( a4 == -1 )
  {
    a4 = 6;
    v12 = 6;
  }
  if ( a3 < 0 )
  {
    v9 = 1;
    a3 = -a3;
  }
  if ( a6 < 1 || a6 > 9 || a5 != 8 || a3 < 9 || a3 > 15 || v12 > 9 || a7 > 2 )
    return -2;
  v13 = (_DWORD *)((int (__cdecl *)(_DWORD, int, int))a2[8])(a2[10], 1, 5816);
  v14 = v13;
  if ( !v13 )
    return -4;
  a2[7] = v13;
  v13[6] = v9;
  v13[10] = a3;
  v13[11] = (1 << a3) - 1;
  v13[18] = a6 + 7;
  v13[17] = 1 << (a6 + 7);
  v13[19] = (1 << (a6 + 7)) - 1;
  *v13 = a2;
  v13[9] = 1 << a3;
  v13[20] = (a6 + 9) / 3u;
  v15 = ((int (__cdecl *)(_DWORD, int, int))a2[8])(a2[10], 1 << a3, 2);
  v16 = v14[9];
  v14[12] = v15;
  v17 = ((int (__cdecl *)(_DWORD, int, int))a2[8])(a2[10], v16, 2);
  v18 = v14[17];
  v14[14] = v17;
  v14[15] = ((int (__cdecl *)(_DWORD, int, int))a2[8])(a2[10], v18, 2);
  v27 = 1 << (a6 + 6);
  v14[1445] = v27;
  v19 = ((int (__cdecl *)(_DWORD, int, int))a2[8])(a2[10], v27, 4);
  v20 = v14[1445];
  v14[3] = 4 * v20;
  v21 = v14[12];
  v14[2] = v19;
  if ( !v21 || !v14[14] || !v14[15] || !v19 )
  {
    a2[6] = "insufficient memory";
    deflateEnd((int)a2);
    return -4;
  }
  v14[1447] = v19 + 2 * (v20 >> 1);
  v14[1444] = v19 + 2 * v20 + v20;
  v14[31] = a4;
  v14[32] = a7;
  *((_BYTE *)v14 + 29) = 8;
  v23 = (_DWORD *)a2[7];
  if ( !v23 || !a2[8] || !a2[9] )
    return -2;
  a2[5] = 0;
  a2[2] = 0;
  a2[6] = 0;
  a2[11] = 2;
  v23[4] = v23[2];
  v24 = (int)v23[6] < 0;
  v23[5] = 0;
  if ( v24 )
    v23[6] = 0;
  v23[1] = v23[6] != 0 ? 113 : 42;
  a2[12] = 1;
  v23[8] = 0;
  _tr_init(0, (int)v23);
  lm_init(0, (int)v23);
  return 0;
}

/* ---- deflateParams  0x004A3190 ---- CONFIRMED */
int __cdecl deflateParams(int a1, unsigned int a2, unsigned int a3)
{
  int result;
  _DWORD *v4;
  int v6;
  int v7;
  unsigned int v8;
  unsigned int v9;
  int v10;
  char *v11;
  int v12;
  int v13;
  int v14;

  result = 0;
  if ( !a1 )
    return -2;
  v4 = *(_DWORD **)(a1 + 28);
  if ( !v4 )
    return -2;
  if ( a2 == -1 )
  {
    a2 = 6;
  }
  else if ( a2 > 9 )
  {
    return -2;
  }
  if ( a3 > 2 )
    return -2;
  if ( deflate_configuration_table[v4[31]].func == deflate_configuration_table[a2].func || !*(_DWORD *)(a1 + 8) )
    goto LABEL_47;
  if ( !*(_DWORD *)(a1 + 12) || !*(_DWORD *)a1 && *(_DWORD *)(a1 + 4) || (v6 = v4[1], v6 == 666) )
  {
    v11 = "stream error";
    result = -2;
    goto LABEL_46;
  }
  if ( *(_DWORD *)(a1 + 16) )
  {
    v7 = v4[8];
    *v4 = a1;
    v4[8] = 1;
    if ( v6 == 42 )
    {
      v8 = (v4[31] - 1) >> 1;
      if ( v8 > 3 )
        v8 = 3;
      v9 = (v8 << 6) | (((v4[10] - 8) << 12) + 2048);
      if ( v4[25] )
        v9 |= 0x20u;
      v4[1] = 113;
      putShortMSB((int)v4, v9 - v9 % 0x1F + 31);
      if ( v4[25] )
      {
        v10 = putShortMSB((int)v4, *(_WORD *)(a1 + 50));
        putShortMSB(v10, *(_DWORD *)(a1 + 48));
      }
      *(_DWORD *)(a1 + 48) = 1;
    }
    if ( v4[5] )
    {
      flush_pending(a1);
      if ( !*(_DWORD *)(a1 + 16) )
      {
        v4[8] = -1;
        result = 0;
        goto LABEL_47;
      }
LABEL_29:
      v12 = v4[1];
      v13 = *(_DWORD *)(a1 + 4);
      if ( v12 == 666 )
      {
        if ( v13 )
        {
          *(char **)(a1 + 24) = "buffer error";
          result = -5;
          goto LABEL_47;
        }
      }
      else if ( v13 )
      {
        goto LABEL_35;
      }
      if ( !v4[27] && v12 == 666 )
        goto LABEL_44;
LABEL_35:
      v14 = deflate_configuration_table[v4[31]].func((int)v4, 1);
      if ( v14 == 2 || v14 == 3 )
        v4[1] = 666;
      if ( v14 && v14 != 2 )
      {
        if ( v14 != 1 )
        {
LABEL_44:
          result = 0;
          goto LABEL_47;
        }
        _tr_align((int)v4);
        flush_pending(a1);
      }
      if ( !*(_DWORD *)(a1 + 16) )
        v4[8] = -1;
      goto LABEL_44;
    }
    if ( *(_DWORD *)(a1 + 4) || v7 < 1 )
      goto LABEL_29;
    v11 = "buffer error";
    result = -5;
LABEL_46:
    *(_DWORD *)(a1 + 24) = v11;
    goto LABEL_47;
  }
  *(char **)(a1 + 24) = "buffer error";
  result = -5;
LABEL_47:
  if ( v4[31] != a2 )
  {
    v4[31] = a2;
    v4[30] = deflate_configuration_table[a2].max_lazy;
    v4[33] = deflate_configuration_table[a2].good_length;
    v4[34] = deflate_configuration_table[a2].nice_length;
    v4[29] = deflate_configuration_table[a2].max_chain;
  }
  v4[32] = a3;
  return result;
}

/* ---- putShortMSB  0x004A3410 ---- CONFIRMED */
int __cdecl putShortMSB(int result, __int16 a2)
{
  int v2;
  int v3;

  *(_BYTE *)(*(_DWORD *)(result + 8) + *(_DWORD *)(result + 20)) = HIBYTE(a2);
  v2 = *(_DWORD *)(result + 8);
  v3 = *(_DWORD *)(result + 20) + 1;
  *(_DWORD *)(result + 20) = v3;
  *(_BYTE *)(v3 + v2) = a2;
  ++*(_DWORD *)(result + 20);
  return result;
}

/* ---- flush_pending  0x004A3440 ---- CONFIRMED */
int __cdecl flush_pending(int result)
{
  int v1;
  unsigned int v2;
  int v3;
  int v4;
  int v5;

  v1 = *(_DWORD *)(result + 28);
  v2 = *(_DWORD *)(v1 + 20);
  if ( v2 > *(_DWORD *)(result + 16) )
    v2 = *(_DWORD *)(result + 16);
  if ( v2 )
  {
    qmemcpy(*(void **)(result + 12), *(const void **)(v1 + 16), v2);
    v3 = *(_DWORD *)(result + 28);
    *(_DWORD *)(result + 12) += v2;
    *(_DWORD *)(v3 + 16) += v2;
    v4 = *(_DWORD *)(result + 16);
    v5 = *(_DWORD *)(result + 28);
    *(_DWORD *)(result + 20) += v2;
    *(_DWORD *)(result + 16) = v4 - v2;
    *(_DWORD *)(v5 + 20) -= v2;
    result = *(_DWORD *)(result + 28);
    if ( !*(_DWORD *)(result + 20) )
      *(_DWORD *)(result + 16) = *(_DWORD *)(result + 8);
  }
  return result;
}

/* ---- deflate  0x004A34B0 ---- CONFIRMED */
int __cdecl deflate(int a1, int a2)
{
  int v2;
  int v3;
  signed int v4;
  unsigned int v5;
  unsigned int v6;
  int v7;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  signed int v14;

  if ( !a1 )
    return -2;
  v2 = *(_DWORD *)(a1 + 28);
  if ( !v2 || (unsigned int)a2 > 4 )
    return -2;
  if ( !*(_DWORD *)(a1 + 12)
    || !*(_DWORD *)a1 && *(_DWORD *)(a1 + 4)
    || (v3 = *(_DWORD *)(v2 + 4), v3 == 666) && a2 != 4 )
  {
    *(char **)(a1 + 24) = "stream error";
    return -2;
  }
  if ( !*(_DWORD *)(a1 + 16) )
  {
LABEL_27:
    *(char **)(a1 + 24) = "buffer error";
    return -5;
  }
  v4 = *(_DWORD *)(v2 + 32);
  *(_DWORD *)v2 = a1;
  v14 = v4;
  *(_DWORD *)(v2 + 32) = a2;
  if ( v3 == 42 )
  {
    v5 = (*(_DWORD *)(v2 + 124) - 1) >> 1;
    if ( v5 > 3 )
      v5 = 3;
    v6 = (v5 << 6) | (((*(_DWORD *)(v2 + 40) - 8) << 12) + 2048);
    if ( *(_DWORD *)(v2 + 100) )
      v6 |= 0x20u;
    *(_DWORD *)(v2 + 4) = 113;
    putShortMSB(v2, v6 - v6 % 0x1F + 31);
    if ( *(_DWORD *)(v2 + 100) )
    {
      v7 = putShortMSB(v2, *(_WORD *)(a1 + 50));
      putShortMSB(v7, *(_DWORD *)(a1 + 48));
    }
    *(_DWORD *)(a1 + 48) = 1;
  }
  if ( *(_DWORD *)(v2 + 20) )
  {
    flush_pending(a1);
    if ( !*(_DWORD *)(a1 + 16) )
    {
      *(_DWORD *)(v2 + 32) = -1;
      return 0;
    }
  }
  else if ( !*(_DWORD *)(a1 + 4) && a2 <= v14 && a2 != 4 )
  {
    *(char **)(a1 + 24) = "buffer error";
    return -5;
  }
  v9 = *(_DWORD *)(v2 + 4);
  v10 = *(_DWORD *)(a1 + 4);
  if ( v9 == 666 )
  {
    if ( v10 )
      goto LABEL_27;
  }
  else if ( v10 )
  {
    goto LABEL_32;
  }
  if ( *(_DWORD *)(v2 + 108) || a2 && v9 != 666 )
  {
LABEL_32:
    v11 = deflate_configuration_table[*(_DWORD *)(v2 + 124)].func(v2, a2);
    if ( v11 == 2 || v11 == 3 )
      *(_DWORD *)(v2 + 4) = 666;
    if ( !v11 || v11 == 2 )
    {
      if ( *(_DWORD *)(a1 + 16) )
        return 0;
    }
    else
    {
      if ( v11 != 1 )
        goto LABEL_43;
      if ( a2 == 1 )
      {
        _tr_align(v2);
      }
      else
      {
        _tr_stored_block(v2, 0, 0, 0);
        if ( a2 == 3 )
        {
          *(_WORD *)(*(_DWORD *)(v2 + 60) + 2 * *(_DWORD *)(v2 + 68) - 2) = 0;
          memset(*(void **)(v2 + 60), 0, 2 * *(_DWORD *)(v2 + 68) - 2);
        }
      }
      flush_pending(a1);
      if ( *(_DWORD *)(a1 + 16) )
        goto LABEL_43;
    }
    *(_DWORD *)(v2 + 32) = -1;
    return 0;
  }
LABEL_43:
  if ( a2 != 4 )
    return 0;
  if ( *(_DWORD *)(v2 + 24) )
    return 1;
  v12 = putShortMSB(v2, *(_WORD *)(a1 + 50));
  putShortMSB(v12, *(_DWORD *)(a1 + 48));
  flush_pending(a1);
  v13 = *(_DWORD *)(v2 + 20);
  *(_DWORD *)(v2 + 24) = -1;
  return v13 == 0;
}

/* ---- deflateEnd  0x004A3730 ---- CONFIRMED */
unsigned int __cdecl deflateEnd(int a1)
{
  int v1;
  int v2;
  int v3;
  int v4;
  int v5;

  if ( !a1 )
    return -2;
  v1 = *(_DWORD *)(a1 + 28);
  if ( !v1 )
    return -2;
  v2 = *(_DWORD *)(v1 + 4);
  if ( v2 != 42 && v2 != 113 && v2 != 666 )
    return -2;
  v3 = *(_DWORD *)(v1 + 8);
  if ( v3 )
    (*(void (__cdecl **)(_DWORD, int))(a1 + 36))(*(_DWORD *)(a1 + 40), v3);
  v4 = *(_DWORD *)(a1 + 28);
  if ( *(_DWORD *)(v4 + 60) )
    (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), *(_DWORD *)(v4 + 60));
  if ( *(_DWORD *)(*(_DWORD *)(a1 + 28) + 56) )
    (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), *(_DWORD *)(*(_DWORD *)(a1 + 28) + 56));
  v5 = *(_DWORD *)(a1 + 28);
  if ( *(_DWORD *)(v5 + 48) )
    (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), *(_DWORD *)(v5 + 48));
  (*(void (__cdecl **)(_DWORD, _DWORD))(a1 + 36))(*(_DWORD *)(a1 + 40), *(_DWORD *)(a1 + 28));
  *(_DWORD *)(a1 + 28) = 0;
  return v2 != 113 ? 0 : 0xFFFFFFFD;
}

/* ---- deflateCopy  0x004A37E0 ---- CONFIRMED */
int __cdecl deflateCopy(_DWORD *a1, int a2)
{
  _DWORD *v2;
  _DWORD *v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  void *v10;
  int v11;
  int v12;
  int v14;

  if ( !a1 )
    return -2;
  if ( !a2 )
    return -2;
  v14 = a1[7];
  if ( !v14 )
    return -2;
  qmemcpy((void *)a2, a1, 0x38u);
  v2 = (_DWORD *)(*(int (__cdecl **)(_DWORD, int, int))(a2 + 32))(*(_DWORD *)(a2 + 40), 1, 5816);
  v3 = v2;
  if ( v2 )
  {
    *(_DWORD *)(a2 + 28) = v2;
    qmemcpy(v2, (const void *)v14, 0x16B8u);
    v4 = v2[9];
    *v2 = a2;
    v2[12] = (*(int (__cdecl **)(_DWORD, int, int))(a2 + 32))(*(_DWORD *)(a2 + 40), v4, 2);
    v5 = (*(int (__cdecl **)(_DWORD, _DWORD, int))(a2 + 32))(*(_DWORD *)(a2 + 40), v3[9], 2);
    v6 = v3[17];
    v3[14] = v5;
    v7 = (*(int (__cdecl **)(_DWORD, int, int))(a2 + 32))(*(_DWORD *)(a2 + 40), v6, 2);
    v8 = v3[1445];
    v3[15] = v7;
    v9 = (*(int (__cdecl **)(_DWORD, int, int))(a2 + 32))(*(_DWORD *)(a2 + 40), v8, 4);
    v10 = (void *)v3[12];
    v3[2] = v9;
    if ( v10 && v3[14] && v3[15] && v9 )
    {
      qmemcpy(v10, *(const void **)(v14 + 48), 2 * v3[9]);
      qmemcpy((void *)v3[14], *(const void **)(v14 + 56), 2 * v3[9]);
      qmemcpy((void *)v3[15], *(const void **)(v14 + 60), 2 * v3[17]);
      qmemcpy((void *)v3[2], *(const void **)(v14 + 8), v3[3]);
      v11 = v3[2];
      v3[4] = v11 + *(_DWORD *)(v14 + 16) - *(_DWORD *)(v14 + 8);
      v12 = v11 + 2 * v3[1445] + v3[1445];
      v3[1447] = v9 + 2 * (v3[1445] >> 1);
      v3[1444] = v12;
      v3[708] = v3 + 35;
      v3[711] = v3 + 608;
      v3[714] = v3 + 669;
      return 0;
    }
    deflateEnd(a2);
  }
  return -4;
}

/* ---- deflate_read_buf  0x004A3990 ---- CONFIRMED */
unsigned int __cdecl deflate_read_buf(int a1, void *a2, unsigned int size)
{
  unsigned int v3;
  unsigned int v5;

  v3 = *(_DWORD *)(a1 + 4);
  if ( v3 > size )
    v3 = size;
  if ( !v3 )
    return 0;
  *(_DWORD *)(a1 + 4) -= v3;
  if ( !*(_DWORD *)(*(_DWORD *)(a1 + 28) + 24) )
    *(_DWORD *)(a1 + 48) = adler32(*(_DWORD *)(a1 + 48), *(unsigned __int8 **)a1, v3);
  qmemcpy(a2, *(const void **)a1, v3);
  v5 = v3 + *(_DWORD *)a1;
  *(_DWORD *)(a1 + 8) += v3;
  *(_DWORD *)a1 = v5;
  return v3;
}

/* ---- lm_init  0x004A3A00 ---- CONFIRMED */
int __fastcall lm_init(int unused, int s)
{
  int level;

  (void)unused;

  *(_DWORD *)( s + 52 ) = 2 * *(_DWORD *)( s + 36 );

  *(_WORD *)( *(_DWORD *)( s + 60 ) + 2 * *(_DWORD *)( s + 68 ) - 2 ) = 0;
  memset( *(void **)( s + 60 ), 0, 2 * *(_DWORD *)( s + 68 ) - 2 );

  level = *(_DWORD *)( s + 124 );
  *(_DWORD *)( s + 120 ) = deflate_configuration_table[level].max_lazy;
  *(_DWORD *)( s + 132 ) = deflate_configuration_table[level].good_length;
  *(_DWORD *)( s + 136 ) = deflate_configuration_table[level].nice_length;
  *(_DWORD *)( s + 116 ) = deflate_configuration_table[level].max_chain;

  *(_DWORD *)( s + 100 ) = 0;
  *(_DWORD *)( s +  84 ) = 0;
  *(_DWORD *)( s + 108 ) = 0;
  *(_DWORD *)( s +  96 ) = 0;
  *(_DWORD *)( s +  64 ) = 0;
  *(_DWORD *)( s + 112 ) = 2;
  *(_DWORD *)( s +  88 ) = 2;
  return 2;
}

/* ---- longest_match  0x004A3A90 ---- CONFIRMED */
unsigned int __cdecl longest_match(unsigned int a1, _DWORD *a2)
{
  unsigned int v2;
  unsigned int v3;
  int v4;
  _BYTE *v5;
  _BYTE *v6;
  _BYTE *v7;
  char v8;
  _BYTE *v9;
  _BYTE *v10;
  _BYTE *v11;
  char v12;
  _BYTE *v13;
  char v14;
  _BYTE *v15;
  char v16;
  _BYTE *v17;
  char v18;
  _BYTE *v19;
  char v20;
  _BYTE *v21;
  char v22;
  _BYTE *v23;
  char v24;
  _BYTE *v25;
  char v26;
  int v27;
  unsigned int result;
  char v29;
  char v30;
  unsigned int v31;
  unsigned int v32;
  unsigned int v33;
  unsigned int v34;

  v2 = a2[25];
  v3 = a2[28];
  v31 = a2[29];
  v32 = a2[34];
  v4 = a2[9];
  v5 = (_BYTE *)(v2 + a2[12]);
  if ( v2 <= v4 - 262 )
    v33 = 0;
  else
    v33 = v2 - v4 + 262;
  v29 = v5[v3 - 1];
  v30 = v5[v3];
  v6 = v5 + 258;
  if ( v3 >= a2[33] )
    v31 >>= 2;
  v34 = a2[27];
  if ( v32 > v34 )
    v32 = a2[27];
  do
  {
    v7 = (_BYTE *)(a1 + a2[12]);
    if ( v7[v3] == v30 && v7[v3 - 1] == v29 && *v7 == *v5 )
    {
      v8 = v7[1];
      v9 = v7 + 1;
      if ( v8 == v5[1] )
      {
        v10 = v5 + 2;
        v11 = v9 + 1;
        do
        {
          v12 = *++v10;
          v13 = v11 + 1;
          if ( v12 != *v13 )
            break;
          v14 = *++v10;
          v15 = v13 + 1;
          if ( v14 != *v15 )
            break;
          v16 = *++v10;
          v17 = v15 + 1;
          if ( v16 != *v17 )
            break;
          v18 = *++v10;
          v19 = v17 + 1;
          if ( v18 != *v19 )
            break;
          v20 = *++v10;
          v21 = v19 + 1;
          if ( v20 != *v21 )
            break;
          v22 = *++v10;
          v23 = v21 + 1;
          if ( v22 != *v23 )
            break;
          v24 = *++v10;
          v25 = v23 + 1;
          if ( v24 != *v25 )
            break;
          v26 = *++v10;
          v11 = v25 + 1;
          if ( v26 != *v11 )
            break;
        }
        while ( v10 < v6 );
        v27 = v10 - v6 + 258;
        v5 = v6 - 258;
        if ( v27 > (int)v3 )
        {
          a2[26] = a1;
          v3 = v27;
          if ( v27 >= (int)v32 )
            break;
          v29 = v5[v27 - 1];
          v30 = v5[v27];
        }
      }
    }
    a1 = *(unsigned __int16 *)(a2[14] + 2 * (a1 & a2[11]));
    if ( a1 <= v33 )
      break;
    --v31;
  }
  while ( v31 );
  result = v34;
  if ( v3 <= v34 )
    return v3;
  return result;
}

/* ---- deflate_fill_window  0x004A3C00 ---- CONFIRMED */
int __cdecl deflate_fill_window(int a1)
{
  unsigned int v1;  /* wsize */
  unsigned int v2;  /* strstart */
  int v3;  /* lookahead */
  unsigned int more;  /* free space at the end of the window */
  int v5;
  unsigned int v6;
  int v7;
  int v8;
  _WORD *v9;
  unsigned int v10;
  __int16 v11;
  unsigned int v12;
  _WORD *v13;
  unsigned int v14;
  __int16 v15;
  int result;
  unsigned int v17;
  unsigned __int8 *v18;
  int v19;
  int v20;

  v1 = *(_DWORD *)(a1 + 36);
  do
  {
    v2 = *(_DWORD *)(a1 + 100);
    v3 = *(_DWORD *)(a1 + 108);
    more = *(_DWORD *)(a1 + 52) - v2 - v3;
    if ( more )
    {
      if ( more == (unsigned int)-1 )
      {
        --more;
        goto LABEL_18;
      }
LABEL_7:
      if ( v2 >= *(_DWORD *)(a1 + 36) + v1 - 262 )
      {
        qmemcpy(*(void **)(a1 + 48), (const void *)(*(_DWORD *)(a1 + 48) + v1), v1);
        v5 = *(_DWORD *)(a1 + 84);
        v6 = *(_DWORD *)(a1 + 104) - v1;
        *(_DWORD *)(a1 + 100) -= v1;
        v7 = *(_DWORD *)(a1 + 60);
        *(_DWORD *)(a1 + 104) = v6;
        v8 = *(_DWORD *)(a1 + 68);
        *(_DWORD *)(a1 + 84) = v5 - v1;
        v9 = (_WORD *)(v7 + 2 * v8);
        do
        {
          v10 = (unsigned __int16)*--v9;
          if ( v10 < v1 )
            v11 = 0;
          else
            v11 = v10 - v1;
          --v8;
          *v9 = v11;
        }
        while ( v8 );
        v12 = v1;
        v13 = (_WORD *)(*(_DWORD *)(a1 + 56) + 2 * v1);
        do
        {
          v14 = (unsigned __int16)*--v13;
          if ( v14 < v1 )
            v15 = 0;
          else
            v15 = v14 - v1;
          --v12;
          *v13 = v15;
        }
        while ( v12 );
        more += v1;
      }
      goto LABEL_18;
    }
    if ( v2 || v3 )
      goto LABEL_7;
    more = v1;
LABEL_18:
    result = *(_DWORD *)a1;
    if ( !*(_DWORD *)(*(_DWORD *)a1 + 4) )
      break;
    result = deflate_read_buf(result, (void *)(*(_DWORD *)(a1 + 48) + *(_DWORD *)(a1 + 100) + *(_DWORD *)(a1 + 108)), more);
    v17 = result + *(_DWORD *)(a1 + 108);
    *(_DWORD *)(a1 + 108) = v17;
    if ( v17 >= 3 )
    {
      v18 = (unsigned __int8 *)(*(_DWORD *)(a1 + 100) + *(_DWORD *)(a1 + 48));
      v19 = *v18;
      v20 = *(_DWORD *)(a1 + 80);
      *(_DWORD *)(a1 + 64) = v19;
      result = *(_DWORD *)(a1 + 76) & (v18[1] ^ (v19 << v20));
      *(_DWORD *)(a1 + 64) = result;
    }
    if ( v17 >= 0x106 )
      break;
    result = *(_DWORD *)(*(_DWORD *)a1 + 4);
  }
  while ( result );
  return result;
}

/* ---- deflate_stored  0x004A3D20 ---- CONFIRMED */
int __cdecl deflate_stored(int *a1, int a2)
{
  unsigned int v2;
  int v3;
  bool v4;
  int v5;
  unsigned int v6;
  unsigned int v7;
  _BYTE *v8;
  int v9;
  int v10;
  unsigned int v11;
  int v12;
  int v13;
  int v14;
  _DWORD *v15;
  int v16;
  unsigned int v17;
  _BYTE *v18;
  int v19;
  int v21;
  _BYTE *v22;
  int v23;
  int v24;
  int v25;

  v25 = 0xFFFF;
  if ( (unsigned int)(a1[3] - 5) < 0xFFFF )
    v25 = a1[3] - 5;
  while ( 1 )
  {
    v2 = a1[27];
    if ( v2 <= 1 )
    {
      deflate_fill_window((int)a1);
      v2 = a1[27];
      if ( !v2 )
        break;
    }
    v3 = a1[25];
    v4 = v2 + v3 == 0;
    a1[25] = v2 + v3;
    v5 = a1[21];
    v6 = a1[25];
    a1[27] = 0;
    v7 = v5 + v25;
    if ( !v4 && v6 < v7 )
      goto LABEL_31;
    a1[27] = v6 - v7;
    a1[25] = v7;
    v8 = v5 < 0 ? 0 : (_BYTE *)(v5 + a1[12]);
    _tr_flush_block(v25, (int)a1, 0, v8);
    v9 = *a1;
    a1[21] = a1[25];
    v10 = *(_DWORD *)(v9 + 28);
    v11 = *(_DWORD *)(v10 + 20);
    if ( v11 > *(_DWORD *)(v9 + 16) )
      v11 = *(_DWORD *)(v9 + 16);
    if ( v11 )
    {
      qmemcpy(*(void **)(v9 + 12), *(const void **)(v10 + 16), v11);
      v12 = *(_DWORD *)(v9 + 28);
      *(_DWORD *)(v9 + 12) += v11;
      *(_DWORD *)(v12 + 16) += v11;
      v13 = *(_DWORD *)(v9 + 16);
      v14 = *(_DWORD *)(v9 + 28);
      *(_DWORD *)(v9 + 20) += v11;
      *(_DWORD *)(v9 + 16) = v13 - v11;
      *(_DWORD *)(v14 + 20) -= v11;
      v15 = *(_DWORD **)(v9 + 28);
      if ( !v15[5] )
        v15[4] = v15[2];
    }
    if ( *(_DWORD *)(*a1 + 16) )
    {
LABEL_31:
      v16 = a1[21];
      v17 = a1[25] - v16;
      if ( v17 < a1[9] - 262 )
        continue;
      v18 = v16 < 0 ? 0 : (_BYTE *)(v16 + a1[12]);
      _tr_flush_block(v17, (int)a1, 0, v18);
      v19 = *a1;
      a1[21] = a1[25];
      flush_pending(v19);
      if ( *(_DWORD *)(*a1 + 16) )
        continue;
    }
    return 0;
  }
  if ( !a2 )
    return 0;
  v21 = a1[21];
  if ( v21 < 0 )
    v22 = 0;
  else
    v22 = (_BYTE *)(v21 + a1[12]);
  _tr_flush_block(a1[25] - v21, (int)a1, a2 == 4, v22);
  v23 = *a1;
  a1[21] = a1[25];
  flush_pending(v23);
  v24 = 0;
  if ( !*(_DWORD *)(*a1 + 16) )
    return a2 != 4 ? 0 : 2;
  LOBYTE(v24) = a2 == 4;
  return 2 * v24 + 1;
}

/* ---- deflate_fast  0x004A3EE0 ---- CONFIRMED */
int __cdecl deflate_fast(int a1, int a2)
{
  unsigned int v2;
  unsigned int v3;
  int v4;
  int v5;
  int v6;
  __int16 v7;
  unsigned __int8 v8;
  unsigned __int16 v9;
  int v10;
  unsigned int v11;
  BOOL v12;
  unsigned int v13;
  unsigned int v14;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;
  unsigned int v23;
  unsigned __int8 *v24;
  int v25;
  int v26;
  unsigned __int8 v27;
  BOOL v28;
  int v29;
  _BYTE *v30;
  int v32;
  _BYTE *v33;

  zlib_BuildTables();
  v2 = 0;
  while ( 1 )
  {
    v3 = *(_DWORD *)(a1 + 108);
    if ( v3 < 0x106 )
    {
      deflate_fill_window(a1);
      v3 = *(_DWORD *)(a1 + 108);
      if ( v3 < 0x106 && !a2 )
        return 0;
      if ( !v3 )
        break;
    }
    if ( v3 >= 3 )
    {
      v4 = *(_DWORD *)(a1 + 100);
      v5 = *(_DWORD *)(a1 + 60);
      v6 = *(_DWORD *)(a1 + 76)
         & (*(unsigned __int8 *)(*(_DWORD *)(a1 + 48) + v4 + 2)
          ^ (*(_DWORD *)(a1 + 64) << *(_DWORD *)(a1 + 80)));
      *(_DWORD *)(a1 + 64) = v6;
      v2 = *(unsigned __int16 *)(v5 + 2 * v6);
      *(_WORD *)(*(_DWORD *)(a1 + 56) + 2 * (v4 & *(_DWORD *)(a1 + 44))) = v2;
      *(_WORD *)(*(_DWORD *)(a1 + 60) + 2 * *(_DWORD *)(a1 + 64)) = *(_WORD *)(a1 + 100);
    }
    if ( v2 )
    {
      if ( *(_DWORD *)(a1 + 100) - v2 <= *(_DWORD *)(a1 + 36) - 262 && *(_DWORD *)(a1 + 128) != 2 )
        *(_DWORD *)(a1 + 88) = longest_match(v2, (_DWORD *)a1);
    }
    if ( *(_DWORD *)(a1 + 88) < 3u )
    {
      v27 = *(_BYTE *)(*(_DWORD *)(a1 + 100) + *(_DWORD *)(a1 + 48));
      *(_WORD *)(*(_DWORD *)(a1 + 5788) + 2 * *(_DWORD *)(a1 + 5784)) = 0;
      *(_BYTE *)(*(_DWORD *)(a1 + 5776) + (*(_DWORD *)(a1 + 5784))++) = v27;
      ++*(_WORD *)(a1 + 4 * v27 + 140);
      v28 = *(_DWORD *)(a1 + 5784) == *(_DWORD *)(a1 + 5780) - 1;
      --*(_DWORD *)(a1 + 108);
      v12 = v28;
    }
    else
    {
      v7 = *(_WORD *)(a1 + 100) - *(_WORD *)(a1 + 104);
      v8 = *(_DWORD *)(a1 + 88) - 3;
      *(_WORD *)(*(_DWORD *)(a1 + 5788) + 2 * *(_DWORD *)(a1 + 5784)) = v7;
      *(_BYTE *)(*(_DWORD *)(a1 + 5776) + (*(_DWORD *)(a1 + 5784))++) = v8;
      v9 = v7 - 1;
      ++*(_WORD *)(a1 + 4 * (unsigned __int8)zlib_lengthCode[v8] + 1168);
      if ( v9 >= 0x100u )
        v10 = (unsigned __int8)zlib_distCode[256 + (v9 >> 7)];
      else
        v10 = (unsigned __int8)zlib_distCode[v9];
      ++*(_WORD *)(a1 + 4 * v10 + 2432);
      v11 = *(_DWORD *)(a1 + 120);
      v12 = *(_DWORD *)(a1 + 5784) == *(_DWORD *)(a1 + 5780) - 1;
      v13 = *(_DWORD *)(a1 + 88);
      v14 = *(_DWORD *)(a1 + 108) - v13;
      *(_DWORD *)(a1 + 108) = v14;
      if ( v13 > v11 || v14 < 3 )
      {
        v23 = v13 + *(_DWORD *)(a1 + 100);
        v24 = (unsigned __int8 *)(v23 + *(_DWORD *)(a1 + 48));
        *(_DWORD *)(a1 + 100) = v23;
        v25 = *(_DWORD *)(a1 + 80);
        *(_DWORD *)(a1 + 88) = 0;
        v26 = *v24;
        *(_DWORD *)(a1 + 64) = v26;
        *(_DWORD *)(a1 + 64) = *(_DWORD *)(a1 + 76) & (v24[1] ^ (v26 << v25));
        goto LABEL_24;
      }
      *(_DWORD *)(a1 + 88) = v13 - 1;
      do
      {
        v15 = *(_DWORD *)(a1 + 48);
        v16 = *(_DWORD *)(a1 + 64);
        v17 = *(_DWORD *)(a1 + 100) + 1;
        *(_DWORD *)(a1 + 100) = v17;
        v18 = v17;
        v19 = *(unsigned __int8 *)(v17 + v15 + 2);
        v20 = *(_DWORD *)(a1 + 60);
        v21 = *(_DWORD *)(a1 + 76) & ((v16 << *(_DWORD *)(a1 + 80)) ^ v19);
        *(_DWORD *)(a1 + 64) = v21;
        v2 = *(unsigned __int16 *)(v20 + 2 * v21);
        *(_WORD *)(*(_DWORD *)(a1 + 56) + 2 * (*(_DWORD *)(a1 + 44) & v18)) = v2;
        *(_WORD *)(*(_DWORD *)(a1 + 60) + 2 * *(_DWORD *)(a1 + 64)) = *(_WORD *)(a1 + 100);
      }
      while ( (*(_DWORD *)(a1 + 88))-- != 1 );
    }
    ++*(_DWORD *)(a1 + 100);
LABEL_24:
    if ( v12 )
    {
      v29 = *(_DWORD *)(a1 + 84);
      v30 = v29 < 0 ? 0 : (_BYTE *)(v29 + *(_DWORD *)(a1 + 48));
      _tr_flush_block(*(_DWORD *)(a1 + 100) - v29, a1, 0, v30);
      *(_DWORD *)(a1 + 84) = *(_DWORD *)(a1 + 100);
      flush_pending(*(_DWORD *)a1);
      if ( !*(_DWORD *)(*(_DWORD *)a1 + 16) )
        return 0;
    }
  }
  v32 = *(_DWORD *)(a1 + 84);
  if ( v32 < 0 )
    v33 = 0;
  else
    v33 = (_BYTE *)(v32 + *(_DWORD *)(a1 + 48));
  _tr_flush_block(*(_DWORD *)(a1 + 100) - v32, a1, a2 == 4, v33);
  *(_DWORD *)(a1 + 84) = *(_DWORD *)(a1 + 100);
  flush_pending(*(_DWORD *)a1);
  if ( *(_DWORD *)(*(_DWORD *)a1 + 16) )
    return 2 * (a2 == 4) + 1;
  else
    return a2 != 4 ? 0 : 2;
}

/* ---- deflate_slow  0x004A41E0 ---- CONFIRMED */
int __cdecl deflate_slow(_DWORD *a1, int a2)
{
  unsigned int v2;
  unsigned int v3;
  int v4;
  int v5;
  int v6;
  unsigned int v7;
  int v8;
  unsigned int v9;
  unsigned int v10;
  unsigned int v11;
  unsigned __int8 v12;
  __int16 v13;
  unsigned __int16 v14;
  int v15;
  int v16;
  BOOL v17;
  unsigned int v18;
  int v19;
  int v20;
  int v22;
  int v23;
  int v24;
  _BYTE *v25;
  int v26;
  unsigned __int8 v28;
  int v29;
  _BYTE *v30;
  int v31;
  int v32;
  int v33;
  int v34;
  unsigned __int8 v35;
  int v36;
  _BYTE *v37;

  zlib_BuildTables();
  v2 = 0;
  while ( 1 )
  {
    v3 = a1[27];
    if ( v3 < 0x106 )
    {
      deflate_fill_window((int)a1);
      v3 = a1[27];
      if ( v3 < 0x106 && !a2 )
        return 0;
      if ( !v3 )
        break;
    }
    if ( v3 >= 3 )
    {
      v4 = a1[25];
      v5 = a1[15];
      v6 = a1[19] & (*(unsigned __int8 *)(a1[12] + v4 + 2) ^ (a1[16] << a1[20]));
      a1[16] = v6;
      v2 = *(unsigned __int16 *)(v5 + 2 * v6);
      *(_WORD *)(a1[14] + 2 * (v4 & a1[11])) = v2;
      *(_WORD *)(a1[15] + 2 * a1[16]) = *((_WORD *)a1 + 50);
    }
    v7 = a1[22];
    v8 = a1[26];
    a1[28] = v7;
    a1[23] = v8;
    a1[22] = 2;
    if ( v2 )
    {
      if ( v7 < a1[30] && a1[25] - v2 <= a1[9] - 262 )
      {
        if ( a1[32] != 2 )
          a1[22] = longest_match(v2, a1);
        v9 = a1[22];
        if ( v9 <= 5 && (a1[32] == 1 || v9 == 3 && (unsigned int)(a1[25] - a1[26]) > 0x1000) )
          a1[22] = 2;
      }
    }
    v10 = a1[28];
    if ( v10 < 3 || a1[22] > v10 )
    {
      if ( a1[24] )
      {
        v28 = *(_BYTE *)(a1[25] + a1[12] - 1);
        *(_WORD *)(a1[1447] + 2 * a1[1446]) = 0;
        *(_BYTE *)(a1[1444] + a1[1446]++) = v28;
        ++LOWORD(a1[v28 + 35]);
        if ( a1[1446] == a1[1445] - 1 )
        {
          v29 = a1[21];
          if ( v29 < 0 )
            v30 = 0;
          else
            v30 = (_BYTE *)(v29 + a1[12]);
          _tr_flush_block(a1[25] - v29, (int)a1, 0, v30);
          v31 = *a1;
          a1[21] = a1[25];
          flush_pending(v31);
        }
        v32 = a1[27] - 1;
        ++a1[25];
        a1[27] = v32;
        goto LABEL_32;
      }
      v33 = a1[25] + 1;
      v34 = a1[27] - 1;
      a1[24] = 1;
      a1[25] = v33;
      a1[27] = v34;
    }
    else
    {
      v11 = a1[25] + a1[27] - 3;
      v12 = *((_BYTE *)a1 + 112) - 3;
      v13 = a1[25] - *((_WORD *)a1 + 46) - 1;
      *(_WORD *)(a1[1447] + 2 * a1[1446]) = v13;
      *(_BYTE *)(a1[1444] + a1[1446]++) = v12;
      v14 = v13 - 1;
      ++LOWORD(a1[(unsigned __int8)zlib_lengthCode[v12] + 292]);
      if ( v14 >= 0x100u )
        v15 = (unsigned __int8)zlib_distCode[256 + (v14 >> 7)];
      else
        v15 = (unsigned __int8)zlib_distCode[v14];
      ++LOWORD(a1[v15 + 608]);
      v16 = a1[28];
      v17 = a1[1446] == a1[1445] - 1;
      a1[27] += 1 - v16;
      a1[28] = v16 - 2;
      do
      {
        v18 = a1[25] + 1;
        a1[25] = v18;
        if ( v18 <= v11 )
        {
          v19 = a1[15];
          v20 = a1[19] & (*(unsigned __int8 *)(a1[12] + v18 + 2) ^ (a1[16] << a1[20]));
          a1[16] = v20;
          v2 = *(unsigned __int16 *)(v19 + 2 * v20);
          *(_WORD *)(a1[14] + 2 * (v18 & a1[11])) = v2;
          *(_WORD *)(a1[15] + 2 * a1[16]) = *((_WORD *)a1 + 50);
        }
      }
      while ( a1[28]-- != 1 );
      v22 = a1[25] + 1;
      a1[24] = 0;
      a1[22] = 2;
      a1[25] = v22;
      v23 = v22;
      if ( v17 )
      {
        v24 = a1[21];
        if ( v24 < 0 )
          v25 = 0;
        else
          v25 = (_BYTE *)(v24 + a1[12]);
        _tr_flush_block(v23 - v24, (int)a1, 0, v25);
        v26 = *a1;
        a1[21] = a1[25];
        flush_pending(v26);
LABEL_32:
        if ( !*(_DWORD *)(*a1 + 16) )
          return 0;
      }
    }
  }
  if ( a1[24] )
  {
    v35 = *(_BYTE *)(a1[25] + a1[12] - 1);
    *(_WORD *)(a1[1447] + 2 * a1[1446]) = 0;
    *(_BYTE *)(a1[1444] + a1[1446]++) = v35;
    ++LOWORD(a1[v35 + 35]);
    a1[24] = 0;
  }
  v36 = a1[21];
  if ( v36 < 0 )
    v37 = 0;
  else
    v37 = (_BYTE *)(v36 + a1[12]);
  _tr_flush_block(a1[25] - v36, (int)a1, a2 == 4, v37);
  a1[21] = a1[25];
  flush_pending(*a1);
  if ( *(_DWORD *)(*a1 + 16) )
    return 2 * (a2 == 4) + 1;
  else
    return a2 != 4 ? 0 : 2;
}


int __cdecl sub_4A2AF0( unsigned int adler, unsigned __int8 *buf, unsigned int len )
{
	return adler32( adler, buf, len );
}
int __cdecl sub_4A2CE0( int crc, unsigned int len, _BYTE *buf )
{
	return crc32( crc, len, buf );
}
