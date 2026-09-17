/*
 * Machine-translated from Call of Duty 1.1 (Windows, CoDMP.exe).
 * Original translation unit:
 * /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/EffectsCore/fx_unassigned.cpp
 * Retail range 0x0048C070-0x004A0FA0, 685 functions: 681 translated, 4 stubbed.
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "fx_types.h"
#include "../renderer/qgl.h"
#include "../universal/com_sndalias.h"

#define FXS_BX(s)       ( (unsigned char *)(s) + 4 )
#define FXS_SIZE(s)     ( *(unsigned int *)( (unsigned char *)(s) + 20 ) )
#define FXS_RES(s)      ( *(unsigned int *)( (unsigned char *)(s) + 24 ) )
#define FXS_PTR(s)      ( FXS_RES(s) < 16 ? FXS_BX(s) : *(unsigned char **)FXS_BX(s) )

#define FXN_LEFT(n)     ( *(unsigned char **)( (unsigned char *)(n) +  0 ) )
#define FXN_PARENT(n)   ( *(unsigned char **)( (unsigned char *)(n) +  4 ) )
#define FXN_RIGHT(n)    ( *(unsigned char **)( (unsigned char *)(n) +  8 ) )
#define FXN_KEY(n)      ( (unsigned char *)(n) + 12 )
#define FXN_MAPPED(n)   ( *(int *)( (unsigned char *)(n) + 40 ) )
#define FXN_COLOR(n)    ( *( (unsigned char *)(n) + 44 ) )
#define FXN_ISNIL(n)    ( *( (unsigned char *)(n) + 45 ) )

#define FX_RED          0
#define FX_BLACK        1

#define FXT_HEAD(t)     ( *(unsigned char **)( (unsigned char *)(t) + 4 ) )
#define FXT_SIZE(t)     ( *(unsigned int  *)( (unsigned char *)(t) + 8 ) )

#define FX_MAP_PAIR_SIZE   32
#define FXP_MAPPED(p)      ( *(int *)( (unsigned char *)(p) + 28 ) )

typedef struct DObj_s {
    void          *tree;                      /* +0x00  XAnimTree *          */
    void          *evalStorage;               /* +0x04                       */
    int            skelCacheKey;              /* +0x08  vs com_skelTimeStamp */
    unsigned char *partRemapTable;            /* +0x0C                       */
    unsigned short unknownState10;            /* +0x10                       */
    unsigned short scrNotifyId;               /* +0x12                       */
    unsigned short tracePartRemapHandle;      /* +0x14                       */
    unsigned char  childCount;                /* +0x16                       */
    unsigned char  partCount;                 /* +0x17  total bones, < 128   */
    unsigned int   childRefs[8];              /* +0x18  XModel * per child   */
    short          childModelIndices[8];      /* +0x38                       */
    unsigned char  childParentPartIndices[8]; /* +0x48  0xFF == unparented   */
    unsigned char  childPartBaseIndices[8];   /* +0x50                       */
} DObj;

char __cdecl FX_GetBoneOrientation( float *out, _DWORD *bolt );

void AxisCopy( float in[3][3], float out[3][3] );
void CM_Trace( void *results, const float *start, const float *end,
               const float *mins, const float *maxs, int model,
               const float *origin, int brushmask, int capsule,
               const void *sphere );
extern qboolean CM_TraceBox( const float *start, const float *end,
                             const float *mins, const float *maxs,
                             float fraction );
extern void *Com_PickSoundAlias( const char *name, int source );
double __cdecl Com_RandFloatRange_m( float lo, float hi );
extern int DObjCalcAnim();
extern int DObjCalcSkel();
extern int DObjCreate();
int __cdecl DObjFree( unsigned int releaseTree, void *dobj );
void *Hunk_AllocAlignInternal( int size, int align );
int __cdecl MSS_PlaySoundAlias_Internal( float *alias2, float *origin, int alias,
                                         int blendBits, int entnum, int *outChan,
                                         int startMs );
void __cdecl MakeNormalVectors( const float *forward, float *right, float *up );
extern int R_CreateBufferARB();
void RotatePointAroundVector( float *dst, const float *dir, const float *point,
                              float degrees );
float VectorNormalize2( const float *v, float *out );
extern int exception___exception_void_();
extern int exception__exception_void_();
extern int j__atol();
void j__free( void *block );
int *__cdecl FxPool_AllocPrimType14( int *pool, int size );
_DWORD *__fastcall FxPool_FreePrimType14( char *element, int *pool );
int *__cdecl FxPool_AllocLight( int *pool, int size );
int *__cdecl FxPool_AllocParticle( int *pool, int size );
int *__cdecl FxPool_AllocLine( int *pool, int size );
int *__cdecl FxPool_AllocElectricity( int *pool, int size );
int *__cdecl FxPool_AllocOrientedParticle( int *pool, int size );
int *__cdecl FxPool_AllocTail( int *pool, int size );
int *__cdecl FxPool_AllocCylinder( int *pool, int size );
int *__cdecl FxPool_AllocEmitter( int *pool, int size );
int *__cdecl FxPool_AllocFlash( int *pool, int size );
int __cdecl CGenericParser2__Clean( int this_ );
char __cdecl CGenericParser2__Parse( int this_, char **dataPtr, int cleanFirst,
                                     char writeable );
void vectoangles( const float *value1, float *angles );

#define FX_BLOCK_SIZE       0x8000
#define FX_ARENA_SIZE       0x200000
#define FX_BLOCK_HDR        0x7FF0
#define FX_HDR_FREECOUNT    32752
#define FX_HDR_FREELIST     32756
#define FX_HDR_PREV         32760
#define FX_HDR_NEXT         32764

int     __cdecl   sub_48D230( void );
int     __stdcall sub_48D260( int a1 );
_DWORD *__cdecl   FxMem_ClaimBlock_m( int count, int elemSize, int prev );
int    *__cdecl   sub_4917B0( int *pool, int elemSize );
int    *__cdecl   sub_4918D0( int *pool, int elemSize );

_DWORD *__fastcall sub_491960( char *p, int *pool );
_DWORD *__fastcall sub_4919F0( char *p, int *pool );
_DWORD *__fastcall sub_491AA0( char *p, int *pool );
_DWORD *__fastcall sub_491B30( char *p, int *pool );
_DWORD *__fastcall sub_491BC0( char *p, int *pool );
_DWORD *__fastcall sub_491C50( char *p, int *pool );
_DWORD *__fastcall sub_491CE0( char *p, int *pool );
_DWORD *__fastcall sub_491D70( char *p, int *pool );
_DWORD *__fastcall sub_491E00( char *p, int *pool );
_DWORD *__fastcall sub_491E90( char *p, int *pool );
_DWORD *__fastcall sub_491F20( char *p, int *pool );
_DWORD *__fastcall sub_4976D0( char *p, int *pool );
_DWORD *__fastcall sub_4977F0( char *p, int *pool );

/* sub_497F90's stub is __stdcall; declared here so sub_497890's call does not
 * implicitly declare a cdecl one that collides with the definition below. */
int     __stdcall  sub_497F90( int a1, int a2, void ***a3 );
int     __stdcall  sub_497890( int tree, void *key );
char    __stdcall  sub_498100( void **node );
int    *__fastcall sub_498230( int unused, int **pptr );
_DWORD *__stdcall  sub_498310( int list );
_DWORD *__stdcall  sub_498DB0( int list, int next, int prev, _DWORD *val );
int     __fastcall sub_498DE0( unsigned int n, int list );
_DWORD *__stdcall  sub_499550( int tree );
int    *__fastcall sub_499650( int unused, int **pptr );

unsigned int __cdecl sub_498690( int self, _DWORD *where, unsigned int count,
                                 _DWORD *pval );
_DWORD *__stdcall sub_4998B0( _DWORD *first, _DWORD *last, _DWORD *dest );
_DWORD *__cdecl   sub_4998E0( _DWORD *pval, _DWORD *dest, _DWORD *end );
_DWORD *__cdecl   sub_499900( _DWORD *out, int first, int last, _DWORD *destLast );
_DWORD *__cdecl   sub_499B40( _DWORD *dest, _DWORD *pval, int count );
_DWORD *__cdecl   sub_499C20( _DWORD *dest, _DWORD *first, _DWORD *last );

void __cdecl   CFxScheduler__PlayEffect_id_axis( _DWORD *self, int id, float *org, int *axis, int *bolt );
int  __stdcall CPrimitiveTemplate__ctor( int self );

char *__cdecl CFxScheduler__GetNewEffectTemplate( int *outId, char *name, int scheduler );
char *__cdecl CFxScheduler__CopyEffect_m( int effectId, int scheduler, int *outId );
char __cdecl CPrimitiveTemplate__ParsePrimitive( int self, int group );

char   __cdecl CParticle__UpdateOrigin( int self );
float *__cdecl CParticle__UpdateVelocity( float *self );
void   __cdecl CParticle__UpdateSize( int self );
void   __cdecl CParticle__UpdateSize2( int self );
void   __cdecl CParticle__UpdateAlpha( int self );
void   __cdecl CTail__UpdateLength_m( int self );
void   __cdecl CTail__CalcNewEndpoint_m( int self );
float *__cdecl CEmitter__UpdateAngles_m( float *self );
void   __cdecl CLight__UpdateSize_m( int self );
void   __cdecl CLight__UpdateRGB_m( int self );

char __cdecl SFxHelper__CullSphere( float *origin, int unusedThis, float radius );
char __cdecl SFxHelper__CullCylinder( float *p1, float *p2, int unusedThis,
                                      float radius1, float radius2 );
void __cdecl CFxBoltFrame__Release( char *self );

void __cdecl CEffect__dtor( void *self );
void __cdecl CParticle__dtor( void *self );
void __cdecl COrientedParticle__dtor( void *self );
void __cdecl CLine__dtor( void *self );
void __cdecl CElectricity__dtor( void *self );
void __cdecl CTail__dtor( void *self );
void __cdecl CCylinder__dtor( void *self );
void __cdecl CEmitter__dtor( void *self );
void __cdecl CLight__dtor( void *self );
void __cdecl CFlash__dtor( void *self );

void __cdecl CFxScheduler__CreateEffect( _DWORD *self, int tmpl, int *bolt,
                                         float *origin, float *axisIn, int timeOffset );
void SFxHelper__Print( int unusedThis, char *Format, ... );

int __cdecl FX_AddParticle( int a1, char **a2, float *orient, float *a4, char a5, int a6, int a7, float a8, int a9, int a10, float a11, int a12, int a13, float a14, _DWORD *a15, _DWORD *a16, float a17, int a18, int a19, _DWORD *a20, _DWORD *a21, int a22, int a23, int a24, int a25, int a26, unsigned int a27 );
int __cdecl FX_AddLine( int a1, char **a2, float *out, int a4, int a5, float a6, int a7, int a8, float a9, _DWORD *a10, _DWORD *a11, float a12, int a13, int a14, int a15 );
int __cdecl FX_AddElectricity( int a1, char **a2, float *out, int a4, int a5, float a6, int a7, int a8, float a9, _DWORD *a10, _DWORD *a11, float a12, int a13, int a14, int a15, int a16 );
int __cdecl FX_AddTail( int a1, char **a2, float *orient, float *a4, int a5, int a6, float a7, int a8, int a9, float a10, int a11, int a12, float a13, _DWORD *a14, _DWORD *a15, float a16, _DWORD *a17, _DWORD *a18, int a19, int a20, int a21, int a22, int a23, int a24 );
int __cdecl FX_AddCylinder( int a1, char **a2, float *orient, int a4, int a5, float a6, int a7, int a8, float a9, int a10, int a11, float a12, int a13, int a14, float a15, _DWORD *a16, _DWORD *a17, float a18, int a19, int a20, int a21 );
int __cdecl FX_AddEmitter( int a1, char **a2, float *orient, float *a4, int a5, int a6, float a7, int a8, int a9, float a10, _DWORD *a11, _DWORD *a12, float a13, _DWORD *a14, _DWORD *a15, _DWORD *a16, _DWORD *a17, int a18, int a19, int a20, int a21, int a22, int a23, int a24, int a25, unsigned int a26 );
int __cdecl FX_AddLight( char **a1, float *out, int a3, int a4, float a5, _DWORD *a6, _DWORD *a7, float a8, int a9, int a10 );
int __cdecl FX_AddOrientedParticle( int a1, char **a2, int a3, float *orient, float *a5, char a6, int a7, int a8, float a9, int a10, int a11, float a12, int a13, int a14, float a15, _DWORD *a16, _DWORD *a17, float a18, int a19, int a20, _DWORD *a21, _DWORD *a22, int a23, int a24, int a25, int a26, int a27, unsigned int a28 );
int __cdecl FX_AddFlash( _DWORD *a1, _DWORD *a2, _DWORD *a3, float a4, int a5, int a6, int a7 );

int *__cdecl FxMem_AllocScheduledEffect( int *pool, int elemSize );

int *__cdecl sub_497640( int *pool, int elemSize );

typedef void *( *XModelAllocFn )( int size );
void *XModelPrecache( const char *name, int loadSurfaces, XModelAllocFn alloc,
                      XModelAllocFn allocMesh );
void *__cdecl FxModelAlloc_m( unsigned int size );

char *__cdecl CFxModel__Register( char *name );

/* ---- sub_48CB30  0x0048CB30 ----  VERIFIED */
char __cdecl sub_48CB30(int a1, _DWORD *a2)
{
  char v3; // [esp+1h] [ebp-1h] BYREF

  v3 = HIBYTE(a1);
  CFxArchive__ReadData(1, a2, &v3);
  return v3;
}

/* ---- sub_48CB50  0x0048CB50 ----  [HIGH] */
__int16 __cdecl sub_48CB50(int a1, _DWORD *a2)
{
  int v3; // [esp+0h] [ebp-4h] BYREF

  v3 = a1;
  CFxArchive__ReadData(2, a2, &v3);
  return v3;
}

/* ---- sub_48CB70  0x0048CB70 ----  [HIGH] */
int __cdecl sub_48CB70(int *a1, char a2)
{
  return CFxArchive__WriteData(a1, &a2, 1);
}

/* ---- sub_48CB80  0x0048CB80 ----  [HIGH] */
int __cdecl sub_48CB80(int *a1, char a2)
{
  return CFxArchive__WriteData(a1, &a2, 2);
}

/* ---- sub_48CB90  0x0048CB90 ----  [HIGH] */
int __cdecl sub_48CB90(int result)
{
  *(_DWORD *)result = 0;
  *(_DWORD *)(result + 4) = 0;
  *(_DWORD *)(result + 8) = 0;
  *(_BYTE *)(result + 12) = 0;
  *(_BYTE *)(result + 13) = 0;
  *(_DWORD *)(result + 16) = 0;
  *(_DWORD *)(result + 7228) = 0;
  *(_DWORD *)(result + 7220) = 0;
  *(_DWORD *)(result + 7224) = 0;
  return result;
}

/* ---- sub_48CBC0  0x0048CBC0 ----  VERIFIED */
int __cdecl sub_48CBC0(int a1, int a2, int a3)
{
  int result;

  *(_DWORD *)a2 = a1;
  result = 0;
  *(_DWORD *)(a2 + 4) = a3;
  *(_DWORD *)(a2 + 8) = 0;
  *(_BYTE *)(a2 + 12) = 1;
  *(_BYTE *)(a2 + 13) = 0;
  *(_DWORD *)(a2 + 16) = 0;
  *(_DWORD *)(a2 + 7228) = 0;
  *(_DWORD *)(a2 + 7220) = 0;
  *(_DWORD *)(a2 + 7224) = 0;
  memset((void *)(a2 + 20), 0, 0x1C20u);
  return result;
}

/* ---- sub_48CC00  0x0048CC00 ----  VERIFIED */
int __cdecl sub_48CC00(int result, int a2, int a3)
{
  *(_DWORD *)result = a3;
  *(_DWORD *)(result + 4) = a2;
  *(_DWORD *)(result + 8) = 0;
  *(_BYTE *)(result + 12) = 0;
  *(_BYTE *)(result + 13) = 1;
  *(_DWORD *)(result + 16) = 0;
  *(_DWORD *)(result + 7228) = 0;
  *(_DWORD *)(result + 7220) = 0;
  *(_DWORD *)(result + 7224) = 0;
  return result;
}

/* ---- sub_48CC30  0x0048CC30 ----  [HIGH] */
int __cdecl sub_48CC30(int result, int a2, int a3)
{
  *(_DWORD *)(result + 4 * a2 + 20) = a3;
  return result;
}

/* ---- sub_48CC40  0x0048CC40 ----  VERIFIED */
int __cdecl sub_48CC40(_DWORD *a1)
{
  __int16 v3; // [esp+4h] [ebp-4h] BYREF

  CFxArchive__ReadData(2, a1, &v3);
  return a1[v3 + 5];
}

/* ---- CFxArchive__ReadShader_m  0x0048CC60 ----  VERIFIED */
int __cdecl CFxArchive__ReadShader_m(_DWORD *this)
{
  int v2;
  unsigned __int8 v4; // [esp+7h] [ebp-45h] BYREF
  _BYTE v5[64]; // [esp+8h] [ebp-44h] BYREF
  unsigned int v6;
  unsigned int retaddr;

  v6 = retaddr ^ _security_cookie;
  CFxArchive__ReadData(1, this, &v4);
  if ( !v4 || v4 >= 0x40u )
    return 0;
  v2 = v4;
  CFxArchive__ReadData(v4, this, v5);
  v5[v2] = 0;
  return re_RegisterShader(v5, 9);
}

/* ---- CFxArchive__ReadEffectID_m  0x0048CCE0 ----  VERIFIED */
char *__cdecl CFxArchive__ReadEffectID_m(_DWORD *this)
{
  int v2;
  unsigned __int8 v4; // [esp+7h] [ebp-45h] BYREF
  char String1[64]; // [esp+8h] [ebp-44h] BYREF
  unsigned int v6;
  unsigned int retaddr;

  v6 = retaddr ^ _security_cookie;
  CFxArchive__ReadData(1, this, &v4);
  if ( !v4 || v4 >= 0x40u )
    return 0;
  v2 = v4;
  CFxArchive__ReadData(v4, this, String1);
  String1[v2] = 0;
  return CFxModel__Register(String1);
}

/* ---- CFxArchive__ReadData  0x0048CD60 ----  [HIGH] */
int __cdecl CFxArchive__ReadData(int a1, _DWORD *a2, _BYTE *a3)
{
  int v4;
  int v5;
  int i;
  int v7;
  char v8;
  int v9;
  int v10;
  int v11;
  int v12;

  v4 = a1;
  a2[4] += a1;
  while ( !a2[1805] )
  {
LABEL_7:
    for ( i = a2[1806]; i; i = a2[1806] )
    {
      if ( !v4 )
        break;
      --a2[1806];
      --v4;
      *a3++ = 0;
      if ( !v4 )
        return i;
    }
    if ( a2[2] >= a2[1] )
      Com_Error(ERR_DROP, &byte_55A034);
    v7 = a2[2];
    v8 = *(_BYTE *)(v7 + *a2);
    a2[2] = v7 + 1;
    v9 = v8 & 0x3F;
    v10 = v8 & 0xC0;
    v11 = v9 + 1;
    if ( v10 )
    {
      v12 = v10 - 64;
      if ( v12 )
      {
        if ( v12 == 64 )
        {
          a2[1805] = 4;
          a2[1806] = v11;
        }
        else
        {
          a2[1805] = v11;
          a2[1806] = 0;
        }
      }
      else
      {
        a2[1805] = 2;
        a2[1806] = v11;
      }
    }
    else
    {
      a2[1805] = 1;
      a2[1806] = v11;
    }
  }
  while ( 1 )
  {
    v5 = a2[1];
    --a2[1805];
    --v4;
    if ( a2[2] >= v5 )
      Com_Error(ERR_DROP, &byte_55A034);
    LOBYTE(i) = *(_BYTE *)(a2[2] + *a2);
    *a3++ = i;
    ++a2[2];
    if ( !v4 )
      return i;
    if ( !a2[1805] )
      goto LABEL_7;
  }
}

/* ---- sub_48CEA0  0x0048CEA0 ----  VERIFIED */
int __cdecl sub_48CEA0(int a1, int *a2)
{
  int v3; // [esp+0h] [ebp-4h] BYREF

  v3 = a1;
  return CFxArchive__WriteData(a2, &v3, 2);
}

/* ---- CFxArchive__WriteShader_m  0x0048CEC0 ----  VERIFIED */
int __cdecl CFxArchive__WriteShader_m(int a1, int *a2)
{
  char *v3;
  int v4;
  int result;
  _BYTE v6[4]; // [esp+Ch] [ebp-4h] BYREF

  v3 = (char *)dword_1432890(a1);
  if ( !v3 )
    v3 = &empty_string;
  v4 = strlen(v3);
  v6[0] = v4;
  result = CFxArchive__WriteData(a2, v6, 1);
  if ( v4 )
    return CFxArchive__WriteData(a2, v3, v4);
  return result;
}

/* ---- CFxArchive__WriteEffectID_m  0x0048CF10 ----  VERIFIED */
int __cdecl CFxArchive__WriteEffectID_m(int a1, int *a2)
{
  char *v3;
  int v4;
  int result;
  _BYTE v6[4]; // [esp+Ch] [ebp-4h] BYREF

  if ( !a1 || (v3 = (char *)(a1 - 64), a1 == 64) )
    v3 = &empty_string;
  v4 = strlen(v3);
  v6[0] = v4;
  result = CFxArchive__WriteData(a2, v6, 1);
  if ( v4 )
    return CFxArchive__WriteData(a2, v3, v4);
  return result;
}

/* ---- CFxArchive__WriteData  0x0048CF60 ----  [CONFIRMED] */
int __cdecl CFxArchive__WriteData(int *a1, _BYTE *a2, int a3)
{
  int v3;
  int v4;
  int v5;
  _BYTE *v6;
  int v7;
  int result;
  int v9;
  int v10;
  int v11;
  char v12;
  _BYTE *v13;
  unsigned __int8 v14;
  int v15;
  int v16;
  int v17;
  int v18;
  _BYTE *v19;
  int v20;
  int v21;

  v3 = a1[1807];
  v4 = a1[2];
  v5 = a3;
  v6 = a2;
  a1[4] += a3;
  if ( v3 == v4 )
  {
    *(_BYTE *)(v3 + *a1) = -64;
    *(_BYTE *)(a1[2] + *a1 + 1) = *a2;
    v6 = a2 + 1;
    v5 = a3 - 1;
    a1[2] += 2;
    --a3;
  }
  v7 = a1[1];
  if ( v5 + a1[2] > v7 )
    Com_Error(ERR_DROP, &byte_559FE0, v7, v5);
  result = 0;
  if ( v5 > 0 )
  {
    while ( 1 )
    {
      v9 = a1[1807];
      if ( (*(_BYTE *)(*a1 + v9) & 0xC0) != 0
        && (*(_BYTE *)(*a1 + v9) & 0xC0) != 0x40
        && (*(_BYTE *)(*a1 + v9) & 0xC0) != 0x80 )
      {
        break;
      }
      if ( result < v5 )
      {
        while ( !v6[result] )
        {
          v19 = (_BYTE *)(*a1 + a1[1807]);
          if ( (*v19 & 0x3F) == 0x3F )
            break;
          ++result;
          ++*v19;
          if ( result >= v5 )
            return result;
        }
        v20 = a1[2];
        v21 = *a1;
        a1[1807] = v20;
        *(_BYTE *)(v20 + v21) = -64;
        *(_BYTE *)(a1[2] + *a1 + 1) = v6[result];
        a1[2] += 2;
LABEL_35:
        if ( ++result < v5 )
          continue;
      }
      return result;
    }
    if ( result < v5 )
    {
      while ( 1 )
      {
        v10 = a1[1807];
        v11 = *a1;
        v12 = *(_BYTE *)(*a1 + v10);
        v13 = (_BYTE *)(*a1 + v10);
        v14 = v12 & 0x3F;
        if ( (v12 & 0x3F) == 0 )
          break;
        if ( v14 == 1 )
        {
          if ( !v6[result] )
          {
            v5 = a3;
            *(_BYTE *)(*a1 + a1[1807]) = 64;
            goto LABEL_35;
          }
          goto LABEL_25;
        }
        if ( v14 == 3 )
        {
          if ( !v6[result] )
          {
            v5 = a3;
            *(_BYTE *)(*a1 + a1[1807]) = 0x80;
            goto LABEL_35;
          }
          goto LABEL_25;
        }
        if ( v14 != 63 )
        {
          if ( v14 > 5u && !v6[result] )
          {
            v17 = a1[2] + v11;
            if ( !*(_BYTE *)(v17 - 2) && !*(_BYTE *)(v17 - 1) )
            {
              v5 = a3;
              *(_BYTE *)(*a1 + a1[1807]) -= 3;
              *(_BYTE *)(a1[2] + *a1 - 2) = *(_BYTE *)(a1[2] + *a1 - 3);
              *(_BYTE *)(a1[2] + *a1 - 3) = 2;
              v18 = a1[2];
              a1[1807] = v18 - 3;
              a1[2] = v18 - 1;
              goto LABEL_35;
            }
          }
          goto LABEL_25;
        }
        v15 = a1[2];
        a1[1807] = v15;
        *(_BYTE *)(v11 + v15) = -64;
        *(_BYTE *)(a1[2] + *a1 + 1) = v6[result];
        v16 = a1[2] + 2;
LABEL_26:
        ++result;
        a1[2] = v16;
        if ( result >= a3 )
          return result;
      }
      if ( !v6[result] )
      {
        v5 = a3;
        *(_BYTE *)(v11 + a1[1807]) = 0;
        goto LABEL_35;
      }
LABEL_25:
      *v13 = v12 + 1;
      *(_BYTE *)(a1[2] + *a1) = v6[result];
      v16 = a1[2] + 1;
      goto LABEL_26;
    }
  }
  return result;
}

/* ---- CFxArchive__Value_bytes_m  0x0048D190 ----  [HIGH] */
char __cdecl CFxArchive__Value_bytes_m(int a1, int a2, _BYTE *a3)
{
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(a1, (_DWORD *)a2, a3);
  else
    return CFxArchive__WriteData((int *)a2, a3, a1);
}

/* ---- sub_48D1B0  0x0048D1B0 ----  VERIFIED */
char __cdecl sub_48D1B0(int a1, _DWORD *a2)
{
  char result;
  int v4; // [esp+4h] [ebp-4h] BYREF

  if ( *(_BYTE *)(a1 + 12) )
  {
    result = CFxArchive__ReadData(2, (_DWORD *)a1, &v4);
    *a2 = *(_DWORD *)(a1 + 4 * (__int16)v4 + 20);
  }
  else
  {
    v4 = *(unsigned __int16 *)a2;
    return CFxArchive__WriteData((int *)a1, &v4, 2);
  }
  return result;
}

/* ---- CFxArchive__Value_shader_m  0x0048D1F0 ----  VERIFIED */
int __cdecl CFxArchive__Value_shader_m(int a1, int *a2)
{
  int result;

  if ( !*(_BYTE *)(a1 + 12) )
    return CFxArchive__WriteShader_m(*a2, (int *)a1);
  result = CFxArchive__ReadShader_m((_DWORD *)a1);
  *a2 = result;
  return result;
}

/* ---- CFxArchive__Value_effectID_m  0x0048D210 ----  VERIFIED */
char *__cdecl CFxArchive__Value_effectID_m(int a1, int *a2)
{
  char *result;

  if ( !*(_BYTE *)(a1 + 12) )
    return (char *)CFxArchive__WriteEffectID_m(*a2, (int *)a1);
  result = CFxArchive__ReadEffectID_m((_DWORD *)a1);
  *a2 = (int)result;
  return result;
}

/* ---- sub_48D230  0x0048D230 ----  VERIFIED */
int __cdecl sub_48D230( void )
{
  unsigned int off;

  for ( off = 0; off < FX_ARENA_SIZE; off += FX_BLOCK_SIZE )
    *(int *)( (char *)&unk_A9CE58 + off + FX_HDR_FREECOUNT ) = -1;
  return (int)off;
}

/* ---- sub_48D260  0x0048D260 ----  VERIFIED */
int __stdcall sub_48D260( int a1 )
{
  unsigned int off;

  for ( off = 0; off < FX_ARENA_SIZE; off += FX_BLOCK_SIZE )
    *(int *)( (char *)&unk_A9CE58 + off + FX_HDR_FREECOUNT ) = -1;
  return a1;
}

/* ---- FxMem_ClaimBlock_m  0x0048D290 ----  [CONFIRMED] */
_DWORD *__cdecl FxMem_ClaimBlock_m( int count, int elemSize, int prev )
{
  char        *block;
  unsigned int off;
  int          i;
  int          last;
  char        *p;

  block = (char *)&unk_A9CE58;
  off   = 0;
  while ( 1 )
  {
    if ( *(int *)( block + FX_HDR_FREECOUNT ) < 0 )
      break;
    off   += FX_BLOCK_SIZE;
    block += FX_BLOCK_SIZE;
    if ( off >= FX_ARENA_SIZE )
    {
      Com_DPrintf( "^1Out of effects memory!\n" );
      return 0;
    }
  }

  if ( dword_1407328 < (int)off )
    dword_1407328 = off;

  last = 0;
  *(int *)( block + FX_HDR_FREECOUNT ) = count;
  if ( count - 1 > 0 )
  {
    p    = block;
    last = count - 1;
    for ( i = count - 1; i; --i )
    {
      *(char **)p = p + elemSize;
      p += elemSize;
    }
  }
  *(_DWORD *)( block + elemSize * last ) = 0;
  *(char **)( block + FX_HDR_FREELIST ) = block;
  if ( prev )
    *(char **)( prev + FX_HDR_NEXT ) = block;
  *(int *)( block + FX_HDR_PREV ) = prev;
  *(int *)( block + FX_HDR_NEXT ) = 0;
  return (_DWORD *)block;
}

/* ---- sub_48D330  0x0048D330 ----  VERIFIED */
int __cdecl sub_48D330(int result)
{
  int v1;
  int v2;

  v1 = *(_DWORD *)(result + 32764);
  if ( v1 )
    *(_DWORD *)(v1 + 32760) = *(_DWORD *)(result + 32760);
  v2 = *(_DWORD *)(result + 32760);
  if ( v2 )
    *(_DWORD *)(v2 + 32764) = *(_DWORD *)(result + 32764);
  *(_DWORD *)(result + 32764) = 0;
  *(_DWORD *)(result + 32760) = 0;
  return result;
}

/* ---- sub_48D3C0  0x0048D3C0 ----  VERIFIED */
char *__cdecl sub_48D3C0(char *this)
{
  char *result;

  result = (char *)&unk_A9CE58 + ((this - (char *)&unk_A9CE58) & 0xFFFF8000);
  *(_DWORD *)this = *((_DWORD *)result + 8189);
  *((_DWORD *)result + 8189) = this;
  ++*((_DWORD *)result + 8188);
  return result;
}

/* ---- sub_48D3F0  0x0048D3F0 ----  [HIGH] */
_DWORD *__cdecl sub_48D3F0(_DWORD *result, _DWORD *a2)
{
  *result = *a2;
  result[1] = a2[1];
  return result;
}

/* ---- sub_48D400  0x0048D400 ----  [HIGH] */
_DWORD *__cdecl sub_48D400(_DWORD *result)
{
  ++*result;
  return result;
}

/* ---- sub_48D410  0x0048D410 ----  VERIFIED */
int *__cdecl sub_48D410(int a1)
{
  return sub_4917B0(&dword_140C9E0, a1);
}

/* ---- sub_48D420  0x0048D420 ----  NOT TRANSLATED */
int sub_48D420( void ) { return 0; }

/* ---- sub_48D430  0x0048D430 ----  [HIGH] */
_DWORD *__cdecl sub_48D430(_DWORD *result)
{
  *result = 0;
  return result;
}

/* ---- sub_48D440  0x0048D440 ----  [HIGH] */
_DWORD *__cdecl sub_48D440(_DWORD *result, _DWORD *a2)
{
  ++*a2;
  *result = a2;
  return result;
}

/* ---- sub_48D460  0x0048D460 ----  VERIFIED */
void __cdecl sub_48D460(char **a1, char **a2)
{
  char *v2;
  char *v3;

  v2 = *a2;
  if ( *a2 != *a1 )
  {
    if ( v2 )
    {
      CFxBoltFrame__Release(v2);
      *a2 = 0;
    }
    v3 = *a1;
    if ( *a1 )
    {
      ++*(_DWORD *)v3;
      *a2 = v3;
    }
  }
}

/* ---- sub_48D480  0x0048D480 ----  VERIFIED */
void __cdecl sub_48D480(char **a1, char *a2)
{
  char *v2;

  v2 = *a1;
  if ( *a1 != a2 )
  {
    if ( v2 )
      CFxBoltFrame__Release(v2);
    ++*(_DWORD *)a2;
    *a1 = a2;
  }
}

/* ---- sub_48D4A0  0x0048D4A0 ----  [HIGH] */
int __cdecl sub_48D4A0(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_48D4B0  0x0048D4B0 ----  [HIGH] */
BOOL __cdecl sub_48D4B0(_DWORD *this)
{
  return *this != 0;
}

/* ---- sub_48D4C0  0x0048D4C0 ----  VERIFIED */
int *__cdecl sub_48D4C0(int a1)
{
  return sub_4918D0(&dword_140C9C8, a1);
}

/* ---- sub_48D4D0  0x0048D4D0 ----  VERIFIED */
_DWORD *__cdecl sub_48D4D0(char *this)
{
  return sub_491960(this, &dword_140C9C8);
}

/* ---- sub_48D4E0  0x0048D4E0 ----  VERIFIED */
_DWORD *__cdecl sub_48D4E0(char *this)
{
  return sub_4919F0(this, &dword_140CA08);
}

/* ---- sub_48D4F0  0x0048D4F0 ----  VERIFIED */
_DWORD *__cdecl sub_48D4F0(char *this)
{
  return sub_491AA0(this, &dword_140C9E8);
}

/* ---- sub_48D500  0x0048D500 ----  VERIFIED */
_DWORD *__cdecl sub_48D500(char *this)
{
  return sub_491B30(this, &dword_140C9D0);
}

/* ---- sub_48D510  0x0048D510 ----  VERIFIED */
_DWORD *__cdecl sub_48D510(char *this)
{
  return sub_491BC0(this, &dword_140CA18);
}

/* ---- sub_48D520  0x0048D520 ----  VERIFIED */
_DWORD *__cdecl sub_48D520(char *this)
{
  return sub_491C50(this, &dword_140C9C0);
}

/* ---- sub_48D530  0x0048D530 ----  VERIFIED */
_DWORD *__cdecl sub_48D530(char *this)
{
  return sub_491CE0(this, &dword_140C9F8);
}

/* ---- sub_48D540  0x0048D540 ----  VERIFIED */
_DWORD *__cdecl sub_48D540(char *this)
{
  return sub_491D70(this, &dword_140CA00);
}

/* ---- sub_48D550  0x0048D550 ----  VERIFIED */
_DWORD *__cdecl sub_48D550(char *this)
{
  return sub_491E00(this, &dword_140C9D8);
}

/* ---- sub_48D560  0x0048D560 ----  VERIFIED */
int __cdecl sub_48D560(float *a1)
{
  int result;

  result = rand();
  a1[89] = ((double)result * 0.000030517578 + (double)result * 0.000030517578 - 1.0) * a1[98] + a1[97];
  return result;
}

/* ---- sub_48D590  0x0048D590 ----  VERIFIED */
_DWORD *__cdecl sub_48D590(char *this)
{
  return sub_491E90(this, &dword_140C9B8);
}

/* ---- sub_48D5A0  0x0048D5A0 ----  VERIFIED */
_DWORD *__cdecl sub_48D5A0(char *this)
{
  return sub_491F20(this, &dword_140C9F0);
}

/* ---- sub_48D5B0  0x0048D5B0 ----  [HIGH] */
char __cdecl sub_48D5B0(int a1)
{
  return *(_BYTE *)(a1 + 12);
}

/* ---- sub_48D5C0  0x0048D5C0 ----  VERIFIED */
int __cdecl sub_48D5C0(int a1, _DWORD *a2)
{
  int v3; // [esp+0h] [ebp-4h] BYREF

  v3 = a1;
  CFxArchive__ReadData(4, a2, &v3);
  return v3;
}

/* ---- sub_48D5E0  0x0048D5E0 ----  VERIFIED */
int __cdecl sub_48D5E0(int *a1, char a2)
{
  return CFxArchive__WriteData(a1, &a2, 4);
}

/* ---- sub_48D5F0  0x0048D5F0 ----  [HIGH] */
char __cdecl sub_48D5F0(_BYTE *a1, int a2)
{
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(4, (_DWORD *)a2, a1);
  else
    return CFxArchive__WriteData((int *)a2, a1, 4);
}

/* ---- sub_48D620  0x0048D620 ----  [HIGH] */
char __cdecl sub_48D620(_BYTE *a1, int a2)
{
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(4, (_DWORD *)a2, a1);
  else
    return CFxArchive__WriteData((int *)a2, a1, 4);
}

/* ---- CFxArchive__Value_vec3_m  0x0048D650 ----  [HIGH] */
char __cdecl CFxArchive__Value_vec3_m(_BYTE *a1, int a2)
{
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(12, (_DWORD *)a2, a1);
  else
    return CFxArchive__WriteData((int *)a2, a1, 12);
}

/* ---- ClampVec  0x0048D680 ----  [CONFIRMED] */
int __cdecl ClampVec(int a1, int a2)
{
  int i;
  unsigned __int64 v3; // rax

  for ( i = 0; i < 3; ++i )
  {
    v3 = (unsigned __int64)(*(float *)(a1 + 4 * i) * 255.0);
    if ( (int)v3 >= 0 )
    {
      if ( (int)v3 > 255 )
        LODWORD(v3) = 255;
    }
    else
    {
      LODWORD(v3) = 0;
    }
    *(_BYTE *)(i + a2) = v3;
  }
  return v3;
}

/* ---- CFxBoltFrame__Acquire  0x0048D6C0 ----  [HIGH] */
int **__cdecl CFxBoltFrame__Acquire(int **a1, int *a2)
{
  int *v2;
  int *v3;

  v2 = (int *)dword_1407320;
  if ( dword_1407320 )
  {
    while ( v2[15] != *a2 || v2[16] != a2[1] )
    {
      v2 = (int *)v2[14];
      if ( !v2 )
        goto LABEL_5;
    }
  }
  else
  {
LABEL_5:
    v3 = sub_4917B0(&dword_140C9E0, 68);
    if ( v3 )
    {
      *v3 = 0;
      v3[1] = 0;
      v3[15] = *a2;
      v3[16] = a2[1];
      v3[14] = dword_1407320;
      dword_1407320 = (int)v3;
      ++*v3;
      *a1 = v3;
      return a1;
    }
    v2 = 0;
  }
  ++*v2;
  *a1 = v2;
  return a1;
}

/* ---- CFxBoltFrame__Release  0x0048D730 ----  VERIFIED */
void __cdecl CFxBoltFrame__Release( char *this )
{
  int     *link;
  int     *pool;
  char    *node;
  _DWORD  *block;
  _DWORD  *cur;
  int      count;
  int      nxt;
  int      prv;

  if ( --*(int *)this != 0 )
    return;

  if ( dword_1407320 )
  {
    link = &dword_1407320;
    while ( 1 )
    {
      node = (char *)*link;
      if ( node == this )
      {
        *link = *(_DWORD *)( this + 56 );
        break;
      }
      link = (int *)( node + 56 );
      if ( !*link )
        break;
    }
  }

  block        = (_DWORD *)( (char *)&unk_A9CE58
                             + ( ( this - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)this = block[8189];
  block[8189]  = (_DWORD)this;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  pool = &dword_140C9E0;
  cur  = (_DWORD *)pool[1];
  if ( !cur[8189] )
  {
    sub_491F90( (int)block, (int)pool );
    return;
  }

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      prv = (int)block[8190];
      if ( !prv || !*(int *)( prv + FX_HDR_FREECOUNT ) )
        return;
      pool[1] = prv;
    }
    nxt = (int)block[8191];
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
}

/* ---- sub_48D770  0x0048D770 ----  VERIFIED */
int __cdecl sub_48D770(int a1)
{
  if ( *(int *)(a1 + 60) < 0 )
    return 0;
  if ( *(_DWORD *)(a1 + 4) != com_skelTimeStamp )
  {
    *(_DWORD *)(a1 + 4) = com_skelTimeStamp;
    if ( !FX_GetBoneOrientation((float *)(a1 + 8), (_DWORD *)(a1 + 60)) )
    {
      *(_DWORD *)(a1 + 60) = -1;
      *(_DWORD *)(a1 + 64) = -1;
      return 0;
    }
  }
  return a1 + 8;
}

/* ---- sub_48D7B0  0x0048D7B0 ----  VERIFIED */
_DWORD *__cdecl sub_48D7B0(_DWORD *result, _DWORD *a2)
{
  *result = 0;
  result[1] = 0;
  result[15] = *a2;
  result[16] = a2[1];
  result[14] = dword_1407320;
  dword_1407320 = (int)result;
  return result;
}

/* ---- CFxBoltFramePtr__Archive  0x0048D7E0 ----  [HIGH]  NOT TRANSLATED */
int CFxBoltFramePtr__Archive( int archive, char **framePtr ) { return 0; }

/* ---- FxModelAlloc_m  0x0048D8D0 ----  VERIFIED */
void *__cdecl FxModelAlloc_m(unsigned int a1)
{
  Hunk_AllocAlignInternal(a1, 32);
}

/* ---- CFxModel__Register  0x0048D8E0 ----  VERIFIED */
char *__cdecl CFxModel__Register( char *String1 )
{
  char          *rec;
  int           *node;
  void          *model;
  unsigned char *storage;
  unsigned short handle;
  int            i;
  int            dobjModel[3];
  int            partBits[4];

  if ( _strnicmp( String1, "xmodel/", 7u ) )
    return 0;

  for ( rec = dword_1407324; rec; rec = *(char **)( rec + 0x98 ) )
  {
    if ( !_stricmp( String1, rec ) )
      return rec + 0x40;
  }

  node = sub_4918D0( &dword_140C9C8, 0x9C );
  if ( !node )
    return 0;

  model = XModelPrecache( String1 + 7, 2, FxModelAlloc_m, FxModelAlloc_m );
  if ( model )
  {
    dobjModel[0] = (int)model;
    dobjModel[1] = 0;
    handle = (unsigned short)dword_143286C( String1, 9 );
    *(unsigned short *)&dobjModel[2] = handle;
    if ( handle )
    {
      DObjCreate( 0, (char *)node + 0x40, dobjModel, 1, 0 );

      storage = (unsigned char *)Hunk_AllocAlignInternal(
                    96 * *( (unsigned char *)node + 0x40 + 0x17 ) + 48, 32 );
      *(_DWORD *)( (char *)node + 0x44 ) = (_DWORD)storage;
      for ( i = 0; i < 4; ++i )
      {
        *(_DWORD *)( storage + 4 * i +  0 ) = 0;
        *(_DWORD *)( storage + 4 * i + 16 ) = 0;
        *(_DWORD *)( storage + 4 * i + 32 ) = 0;
      }

      partBits[0] = -1;
      partBits[1] = -1;
      partBits[2] = -1;
      partBits[3] = -1;
      DObjCalcAnim( (char *)node + 0x40, partBits );
      DObjCalcSkel( partBits, (char *)node + 0x40 );

      strcpy( (char *)node, String1 );
      *(_DWORD *)( (char *)node + 0x98 ) = (_DWORD)dword_1407324;
      dword_1407324 = (char *)node;
      return (char *)node + 0x40;
    }
  }

  sub_491960( (char *)node, &dword_140C9C8 );
  return 0;
}

/* ---- nullsub_31  0x0048DA40 ----  [HIGH] */
void nullsub_31()
{
  ;
}

/* ---- CFxModel__NameForDObj  0x0048DA50 ----  [CONFIRMED] */
char *__cdecl CFxModel__NameForDObj(int a1)
{
  if ( a1 )
    return (char *)(a1 - 64);
  else
    return &empty_string;
}

/* ---- CFxModel__Clean  0x0048DA60 ----  [HIGH] */
_DWORD *CFxModel__Clean()
{
  char *i;
  _DWORD *result;

  for ( i = dword_1407324; dword_1407324; i = dword_1407324 )
  {
    dword_1407324 = (char *)*((_DWORD *)i + 38);
    DObjFree(1u, (void *)(i + 64));
    result = sub_491960(i, &dword_140C9C8);
  }
  return result;
}

/* ---- CEffect__ctor  0x0048DAB0 ----  [CONFIRMED] */
_DWORD *__cdecl CEffect__ctor(_DWORD *result)
{
  *result = &off_559F80;
  result[54] = 0;
  return result;
}

/* ---- CEffect__scalar_dtor  0x0048DAD0 ----  [CONFIRMED] */
char *__cdecl CEffect__scalar_dtor(char *this, char a2)
{
  CEffect__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_4919F0(this, &dword_140CA08);
  return this;
}

/* ---- CEffect__dtor  0x0048DB00 ----  VERIFIED */
void __cdecl CEffect__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- nullsub_32  0x0048DB20 ----  [CONFIRMED] */
void nullsub_32()
{
  ;
}

/* ---- CEffect__Cull  0x0048DB30 ----  [CONFIRMED] */
char CEffect__Cull()
{
  return 0;
}

/* ---- nullsub_33  0x0048DB40 ----  [CONFIRMED] */
void nullsub_33()
{
  ;
}

/* ---- CEffect__Update  0x0048DB50 ----  [HIGH] */
bool __cdecl CEffect__Update(_DWORD *this)
{
  return this[4] <= fxMTime;
}

/* ---- CEffect__TypeID  0x0048DB60 ----  [CONFIRMED] */
char CEffect__TypeID()
{
  return 0;
}

/* ---- CEffect__Archive  0x0048DB70 ----  [CONFIRMED] */
void __cdecl CEffect__Archive(int this, int a2)
{
  int v2;

  v2 = a2;
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, (_BYTE *)(this + 4));
  else
    CFxArchive__WriteData((int *)a2, (_BYTE *)(this + 4), 12);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)v2, (_BYTE *)(this + 16));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 16), 4);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)v2, (_BYTE *)(this + 20));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 20), 4);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)v2, (_BYTE *)(this + 24));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 24), 4);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)v2, (_BYTE *)(this + 28));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 28), 12);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)v2, (_BYTE *)(this + 40));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 40), 12);
  if ( *(_BYTE *)(v2 + 12) )
  {
    CFxArchive__ReadData(2, (_DWORD *)v2, &a2);
    *(_DWORD *)(this + 52) = *(_DWORD *)(v2 + 4 * (__int16)a2 + 20);
  }
  else
  {
    a2 = *(unsigned __int16 *)(this + 52);
    CFxArchive__WriteData((int *)v2, &a2, 2);
  }
  if ( *(_BYTE *)(v2 + 12) )
  {
    CFxArchive__ReadData(2, (_DWORD *)v2, &a2);
    *(_DWORD *)(this + 56) = *(_DWORD *)(v2 + 4 * (__int16)a2 + 20);
  }
  else
  {
    a2 = *(unsigned __int16 *)(this + 56);
    CFxArchive__WriteData((int *)v2, &a2, 2);
  }
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(156, (_DWORD *)v2, (_BYTE *)(this + 60));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 60), 156);
  if ( *(_BYTE *)(v2 + 12) )
    *(_DWORD *)(this + 164) = CFxArchive__ReadShader_m((_DWORD *)v2);
  else
    CFxArchive__WriteShader_m(*(_DWORD *)(this + 164), (int *)v2);
  if ( *(_BYTE *)(v2 + 12) )
    *(_DWORD *)(this + 204) = CFxArchive__ReadEffectID_m((_DWORD *)v2);
  else
    CFxArchive__WriteEffectID_m(*(_DWORD *)(this + 204), (int *)v2);
  CFxBoltFramePtr__Archive(v2, (char **)(this + 216));
}

/* ---- CParticle__ctor  0x0048DD30 ----  [CONFIRMED] */
_DWORD *__cdecl CParticle__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559F10;
  result[15] = 4;
  return result;
}

/* ---- CParticle__scalar_dtor  0x0048DD50 ----  [CONFIRMED] */
char *__cdecl CParticle__scalar_dtor(char *this, char a2)
{
  CParticle__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491B30(this, &dword_140C9D0);
  return this;
}

/* ---- CParticle__dtor  0x0048DD80 ----  VERIFIED */
void __cdecl CParticle__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- CParticle__Die  0x0048DDA0 ----  [CONFIRMED] */
void __cdecl CParticle__Die(int this)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6[3]; // [esp+8h] [ebp-Ch] BYREF

  v2 = *(_DWORD *)(this + 24);
  if ( (v2 & 0x20000000) != 0 && (v2 & 0x40000000) == 0 )
  {
    v3 = rand();
    *(float *)v6 = (double)v3 * 0.000030517578 + (double)v3 * 0.000030517578 - 1.0;
    v4 = rand();
    *(float *)&v6[1] = (double)v4 * 0.000030517578 + (double)v4 * 0.000030517578 - 1.0;
    v5 = rand();
    *(float *)&v6[2] = (double)v5 * 0.000030517578 + (double)v5 * 0.000030517578 - 1.0;
    VectorNormalize((float *)v6);
    sub_493730(v6, dword_14075A0, *(_DWORD *)(this + 56), (float *)(this + 4));
  }
}

/* ---- CParticle__Cull  0x0048DE50 ----  [CONFIRMED] */
char __cdecl CParticle__Cull(float *this)
{
  return SFxHelper__CullSphere(this + 32, (int)&theFxHelper, this[46]);
}

/* ---- CParticle__Draw  0x0048DE70 ----  [CONFIRMED] */
int __cdecl CParticle__Draw(_DWORD *this)
{
  if ( (this[6] & 0x100000) != 0 )
    this[16] |= 8u;
  return dword_14328A0(this + 15, 0);
}

/* ---- CParticle__Update  0x0048DE90 ----  [CONFIRMED] */
char __cdecl CParticle__Update(int this)
{
  int v3;
  const float *v4;
  int v5;
  int v6;

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  if ( !CParticle__UpdateOrigin(this) )
    return 0;
  v3 = *(_DWORD *)(this + 216);
  if ( v3 )
  {
    v4 = (const float *)sub_48D770(v3);
    if ( !v4 )
      return 0;
    OrientationPosToWorldPos(v4, (const float *)(this + 4), (float *)(this + 128));
  }
  else
  {
    v5 = *(_DWORD *)(this + 8);
    v6 = *(_DWORD *)(this + 12);
    *(_DWORD *)(this + 128) = *(_DWORD *)(this + 4);
    *(_DWORD *)(this + 132) = v5;
    *(_DWORD *)(this + 136) = v6;
  }
  CParticle__UpdateSize(this);
  CParticle__UpdateSize2(this);
  CParticle__UpdateRGB(this);
  CParticle__UpdateAlpha(this);
  CParticle__UpdateRotation(this);
  return 1;
}

/* ---- CParticle__UpdateOrigin  0x0048DF20 ----  VERIFIED */
char __cdecl CParticle__UpdateOrigin( int a1 )
{
  int     bolt;
  float  *orient;
  float   t, halfTT;
  float   newOrigin[3];
  float   startWorld[3];
  float   endWorld[3];
  trace_t trace;
  int     flags;
  float   nx, ny, nz;
  float   vx, vy, vz;
  float   scale, dot2;

  CParticle__UpdateVelocity( (float *)a1 );

  t      = (float)( (double)dword_1407514 * 0.001 );
  halfTT = (float)( (double)t * t * 0.5 );

  newOrigin[0] = halfTT * *(float *)( a1 + 232 ) + t * *(float *)( a1 + 220 ) + *(float *)( a1 + 4 );
  newOrigin[1] = halfTT * *(float *)( a1 + 236 ) + t * *(float *)( a1 + 224 ) + *(float *)( a1 + 8 );
  newOrigin[2] = halfTT * *(float *)( a1 + 240 ) + t * *(float *)( a1 + 228 ) + *(float *)( a1 + 12 );

  if ( ( *(_DWORD *)( a1 + 24 ) & 0x2000000 ) == 0 )
    goto STORE;

  bolt   = *(_DWORD *)( a1 + 216 );
  orient = 0;
  if ( bolt )
  {
    if ( *(int *)( bolt + 60 ) < 0 )
      return 0;
    if ( *(_DWORD *)( bolt + 4 ) != com_skelTimeStamp )
    {
      *(_DWORD *)( bolt + 4 ) = com_skelTimeStamp;
      if ( !FX_GetBoneOrientation( (float *)( bolt + 8 ), (_DWORD *)( bolt + 60 ) ) )
      {
        *(_DWORD *)( bolt + 64 ) = -1;
        *(_DWORD *)( bolt + 60 ) = -1;
        return 0;
      }
    }
    orient = (float *)( bolt + 8 );
    if ( !orient )
      return 0;

    startWorld[0] = *(float *)( a1 + 12 ) * orient[9] + orient[3] * *(float *)( a1 + 4 )
                  + *(float *)( a1 + 8 ) * orient[6] + orient[0];
    startWorld[1] = *(float *)( a1 + 4 ) * orient[4] + *(float *)( a1 + 12 ) * orient[10]
                  + *(float *)( a1 + 8 ) * orient[7] + orient[1];
    startWorld[2] = *(float *)( a1 + 12 ) * orient[11] + *(float *)( a1 + 8 ) * orient[8]
                  + orient[5] * *(float *)( a1 + 4 ) + orient[2];

    endWorld[0] = newOrigin[2] * orient[9] + newOrigin[0] * orient[3]
                + newOrigin[1] * orient[6] + orient[0];
    endWorld[1] = newOrigin[0] * orient[4] + newOrigin[2] * orient[10]
                + newOrigin[1] * orient[7] + orient[1];
    endWorld[2] = newOrigin[0] * orient[5] + newOrigin[2] * orient[11]
                + newOrigin[1] * orient[8] + orient[2];
  }
  else
  {
    startWorld[0] = *(float *)( a1 + 4 );
    startWorld[1] = *(float *)( a1 + 8 );
    startWorld[2] = *(float *)( a1 + 12 );
    endWorld[0]   = newOrigin[0];
    endWorld[1]   = newOrigin[1];
    endWorld[2]   = newOrigin[2];
  }

  trace.fraction = 1.0f;
  if ( *(_DWORD *)( a1 + 24 ) & 0x4000000 )
    CM_Trace( &trace, startWorld, endWorld,
              (const float *)( a1 + 0x1C ), (const float *)( a1 + 0x28 ),
              0, vec3_origin, 1, 0, 0 );
  else
    CM_Trace( &trace, startWorld, endWorld, 0, 0,
              0, vec3_origin, 1, 0, 0 );

  trace.entityNum = 1022;
  if ( trace.fraction == 1.0 )
    trace.entityNum = 1023;

  if ( !( trace.fraction < 1.0f ) || trace.startsolid || trace.allsolid )
  {
STORE:
    *(float *)( a1 + 4 )  = newOrigin[0];
    *(float *)( a1 + 8 )  = newOrigin[1];
    *(float *)( a1 + 12 ) = newOrigin[2];
    return 1;
  }

  if ( *(int *)( a1 + 24 ) < 0 )                        /* impactFx, 0x80000000 */
    sub_493730( (int *)trace.normal, dword_14075A0, *(_DWORD *)( a1 + 52 ),
                trace.endpos );

  flags = *(_DWORD *)( a1 + 24 );
  if ( ( flags & 0x40000000 ) != 0 )
    return 0;

  if ( *(_DWORD *)( a1 + 216 ) )
  {
    OrientationDirFromWorldDir( (const float *)orient, trace.normal, startWorld );
    nx = startWorld[0];
    ny = startWorld[1];
  }
  else
  {
    nx = trace.normal[0];
    ny = trace.normal[1];
    startWorld[2] = trace.normal[2];
  }
  nz = startWorld[2];

  scale = trace.fraction * t;
  vx = scale * *(float *)( a1 + 232 ) + *(float *)( a1 + 220 );
  *(float *)( a1 + 220 ) = vx;
  vy = scale * *(float *)( a1 + 236 ) + *(float *)( a1 + 224 );
  *(float *)( a1 + 224 ) = vy;
  vz = scale * *(float *)( a1 + 240 ) + *(float *)( a1 + 228 );
  *(float *)( a1 + 228 ) = vz;

  dot2 = (float)( ( vz * nz + vy * ny + vx * nx ) * -2.0 );
  *(float *)( a1 + 220 ) = ( dot2 * nx + vx ) * *(float *)( a1 + 312 );
  *(float *)( a1 + 224 ) = ( dot2 * ny + vy ) * *(float *)( a1 + 312 );
  *(float *)( a1 + 228 ) = ( dot2 * nz + vz ) * *(float *)( a1 + 312 );

  if ( trace.normal[2] > 0.0f )
  {
    if ( *(float *)( a1 + 228 ) < 4.0f )
    {
      *(_DWORD *)( a1 + 228 ) = 0;
      *(_DWORD *)( a1 + 224 ) = 0;
      *(_DWORD *)( a1 + 220 ) = 0;
      *(_DWORD *)( a1 + 240 ) = 0;
      *(_DWORD *)( a1 + 236 ) = 0;
      *(_DWORD *)( a1 + 232 ) = 0;
      *(_DWORD *)( a1 + 24 )  = flags & 0x7DFFFFFF;
    }
  }

  *(float *)( a1 + 4 )  = ( newOrigin[0] - *(float *)( a1 + 4 ) )  * trace.fraction + *(float *)( a1 + 4 );
  *(float *)( a1 + 8 )  = ( newOrigin[1] - *(float *)( a1 + 8 ) )  * trace.fraction + *(float *)( a1 + 8 );
  *(float *)( a1 + 12 ) = ( newOrigin[2] - *(float *)( a1 + 12 ) ) * trace.fraction + *(float *)( a1 + 12 );
  return 1;
}

/* ---- CParticle__UpdateVelocity  0x0048E340 ----  VERIFIED */
float *__cdecl CParticle__UpdateVelocity( float *result )
{
  result[55] = (float)( (double)dword_1407514 * 0.001 * result[58] + result[55] );
  result[56] = (float)( (double)dword_1407514 * 0.001 * result[59] + result[56] );
  result[57] = (float)( (double)dword_1407514 * 0.001 * result[60] + result[57] );
  return result;
}

/* ---- CParticle__UpdateSize  0x0048E3A0 ----  VERIFIED */
void __cdecl CParticle__UpdateSize( int a1 )
{
  int   flags = *(_DWORD *)( a1 + 24 );
  int   mode;
  float linear = 1.0f;
  float frac   = 1.0f;

  if ( ( flags & 0x100 ) != 0 )
    linear = (float)( 1.0 - (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                          / (double)( *(_DWORD *)( a1 + 20 ) - *(_DWORD *)( a1 + 16 ) ) );

  mode = flags & 0xC00;
  if ( mode == 0x400 )
  {
    if ( (double)fxMTime > *(float *)( a1 + 252 ) )
      frac = (float)( 1.0 - ( (double)fxMTime - *(float *)( a1 + 252 ) )
                          / ( (double)*(int *)( a1 + 20 ) - *(float *)( a1 + 252 ) ) );
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x100 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }
  else if ( mode == 0x800 )
  {
    linear = (float)( cos( (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                           * *(float *)( a1 + 252 ) ) * linear );
  }
  else if ( mode == 0xC00 )
  {
    if ( (double)fxMTime < *(float *)( a1 + 252 ) )
      frac = (float)( ( *(float *)( a1 + 252 ) - (double)fxMTime )
                    / ( *(float *)( a1 + 252 ) - (double)*(int *)( a1 + 16 ) ) );
    else
      frac = 0.0f;
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x100 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }

  if ( ( flags & 0x200 ) != 0 )
    linear = (float)( (double)rand() * 0.000030517578125 * linear );

  *(float *)( a1 + 184 ) = (float)( ( 1.0 - linear ) * *(float *)( a1 + 248 )
                                    + linear * *(float *)( a1 + 244 ) );
}

/* ---- CParticle__UpdateSize2  0x0048E4F0 ----  VERIFIED */
void __cdecl CParticle__UpdateSize2( int a1 )
{
  int   flags = *(_DWORD *)( a1 + 24 );
  int   mode;
  float linear = 1.0f;
  float frac   = 1.0f;

  if ( ( flags & 0x10000 ) != 0 )
    linear = (float)( 1.0 - (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                          / (double)( *(_DWORD *)( a1 + 20 ) - *(_DWORD *)( a1 + 16 ) ) );

  mode = flags & 0xC0000;
  if ( mode == 0x40000 )
  {
    if ( (double)fxMTime > *(float *)( a1 + 264 ) )
      frac = (float)( 1.0 - ( (double)fxMTime - *(float *)( a1 + 264 ) )
                          / ( (double)*(int *)( a1 + 20 ) - *(float *)( a1 + 264 ) ) );
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x10000 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }
  else if ( mode == 0x80000 )
  {
    linear = (float)( cos( (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                           * *(float *)( a1 + 264 ) ) * linear );
  }
  else if ( mode == 0xC0000 )
  {
    if ( (double)fxMTime < *(float *)( a1 + 264 ) )
      frac = (float)( ( *(float *)( a1 + 264 ) - (double)fxMTime )
                    / ( *(float *)( a1 + 264 ) - (double)*(int *)( a1 + 16 ) ) );
    else
      frac = 0.0f;
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x10000 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }

  if ( ( flags & 0x20000 ) != 0 )
    linear = (float)( (double)rand() * 0.000030517578125 * linear );

  *(float *)( a1 + 160 ) = (float)( ( 1.0 - linear ) * *(float *)( a1 + 260 )
                                    + linear * *(float *)( a1 + 256 ) );
}

/* ---- CParticle__UpdateRGB  0x0048E640 ----  [CONFIRMED] */
int __cdecl CParticle__UpdateRGB(int a1)
{
  int v1;
  double v2;
  int v3;
  double v4;
  double v5;
  double v6;
  double v7;
  double v8;
  float v10;
  float v11;
  float v12;
  float v13[3]; // [esp+10h] [ebp-Ch] BYREF

  v1 = *(_DWORD *)(a1 + 24);
  v2 = 1.0;
  v10 = 1.0;
  if ( (v1 & 0x10) != 0 )
    v10 = 1.0 - (double)(fxMTime - *(_DWORD *)(a1 + 16)) / (double)(*(_DWORD *)(a1 + 20) - *(_DWORD *)(a1 + 16));
  v3 = v1 & 0xC0;
  if ( v3 == 64 )
  {
    v4 = (double)fxMTime;
    if ( v4 > *(float *)(a1 + 292) )
    {
      v12 = v4;
      v2 = 1.0 - (v12 - *(float *)(a1 + 292)) / ((double)*(int *)(a1 + 20) - *(float *)(a1 + 292));
    }
LABEL_12:
    if ( (*(_DWORD *)(a1 + 24) & 0x10) != 0 )
      v2 = (v2 + v10) * 0.5;
    goto LABEL_14;
  }
  if ( v3 != 128 )
  {
    if ( v3 != 192 )
      goto LABEL_15;
    v5 = (double)fxMTime;
    if ( (v5 < *(float *)(a1 + 292)) | __UNORDERED__(v5, *(float *)(a1 + 292)) )
      v2 = (*(float *)(a1 + 292) - v5) / (*(float *)(a1 + 292) - (double)*(int *)(a1 + 16));
    else
      v2 = 0.0;
    goto LABEL_12;
  }
  v2 = cos((double)(fxMTime - *(_DWORD *)(a1 + 16)) * *(float *)(a1 + 292)) * v10;
LABEL_14:
  v10 = v2;
LABEL_15:
  if ( (v1 & 0x20) != 0 )
    v10 = (double)rand() * 0.000030517578 * v10;
  v6 = v10 * *(float *)(a1 + 268);
  v7 = v10 * *(float *)(a1 + 272);
  v13[2] = v10 * *(float *)(a1 + 276);
  v8 = 1.0 - v10;
  v11 = v8;
  v13[0] = v8 * *(float *)(a1 + 280) + v6;
  v13[1] = v11 * *(float *)(a1 + 284) + v7;
  v13[2] = v11 * *(float *)(a1 + 288) + v13[2];
  return ClampVec((int)v13, a1 + 168);
}

/* ---- CParticle__UpdateAlpha  0x0048E7D0 ----  VERIFIED */
void __cdecl CParticle__UpdateAlpha( int a1 )
{
  int   flags = *(_DWORD *)( a1 + 24 );
  int   mode;
  float linear = 1.0f;
  float mres   = 1.0f;
  float alpha;
  int   r, g, b;

  if ( ( flags & 1 ) != 0 )
    linear = (float)( 1.0 - (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                          / (double)( *(_DWORD *)( a1 + 20 ) - *(_DWORD *)( a1 + 16 ) ) );

  mode = flags & 0xC;
  if ( mode == 4 )
  {
    if ( (double)fxMTime > *(float *)( a1 + 304 ) )
      mres = (float)( 1.0 - ( (double)fxMTime - *(float *)( a1 + 304 ) )
                          / ( (double)*(int *)( a1 + 20 ) - *(float *)( a1 + 304 ) ) );
    linear = ( flags & 1 ) ? (float)( ( linear + mres ) * 0.5 ) : mres;
  }
  else if ( mode == 8 )
  {
    linear = (float)( linear * cos( (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                                    * *(float *)( a1 + 304 ) ) );
  }
  else if ( mode == 0xC )
  {
    if ( (double)fxMTime < *(float *)( a1 + 304 ) )
      mres = (float)( ( *(float *)( a1 + 304 ) - (double)fxMTime )
                    / ( *(float *)( a1 + 304 ) - (double)*(int *)( a1 + 16 ) ) );
    else
      mres = 0.0f;
    linear = ( flags & 1 ) ? (float)( ( linear + mres ) * 0.5 ) : mres;
  }

  alpha = (float)( ( 1.0 - linear ) * *(float *)( a1 + 300 )
                   + linear * *(float *)( a1 + 296 ) );
  if ( alpha < 0.0f )
    alpha = 0.0f;
  else if ( alpha > 1.0f )
    alpha = 1.0f;

  if ( ( flags & 2 ) != 0 )
    alpha = (float)( (double)rand() * 0.000030517578125 * alpha );

  if ( ( *(_DWORD *)( a1 + 24 ) & 0x8000000 ) != 0 )
  {
    *(_BYTE *)( a1 + 171 ) = (_BYTE)(int)( alpha * 255.0 );
  }
  else
  {
    r = *(unsigned __int8 *)( a1 + 168 );
    g = *(unsigned __int8 *)( a1 + 169 );
    b = *(unsigned __int8 *)( a1 + 170 );
    *(_BYTE *)( a1 + 168 ) = (_BYTE)(int)( (double)r * alpha );
    *(_BYTE *)( a1 + 169 ) = (_BYTE)(int)( (double)g * alpha );
    *(_BYTE *)( a1 + 170 ) = (_BYTE)(int)( (double)b * alpha );
    *(_BYTE *)( a1 + 171 ) = 0xFF;
  }
}

/* ---- CParticle__UpdateRotation  0x0048E9C0 ----  [CONFIRMED] */
int __cdecl CParticle__UpdateRotation(int result)
{
  *(float *)(result + 188) = (double)dword_1407514 * *(float *)(result + 308) * 0.0099999998 + *(float *)(result + 188);
  *(float *)(result + 308) = (1.0 - (double)dword_1407514 * 0.00065) * *(float *)(result + 308);
  return result;
}

/* ---- CParticle__TypeID  0x0048EA00 ----  [CONFIRMED] */
char CParticle__TypeID()
{
  return 1;
}

/* ---- CParticle__Archive  0x0048EA10 ----  [CONFIRMED] */
char __cdecl CParticle__Archive(_BYTE *this, int a2)
{
  CEffect__Archive((int)this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 220);
  else
    CFxArchive__WriteData((int *)a2, this + 220, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 232);
  else
    CFxArchive__WriteData((int *)a2, this + 232, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 244);
  else
    CFxArchive__WriteData((int *)a2, this + 244, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 248);
  else
    CFxArchive__WriteData((int *)a2, this + 248, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 252);
  else
    CFxArchive__WriteData((int *)a2, this + 252, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 256);
  else
    CFxArchive__WriteData((int *)a2, this + 256, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 260);
  else
    CFxArchive__WriteData((int *)a2, this + 260, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 264);
  else
    CFxArchive__WriteData((int *)a2, this + 264, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 268);
  else
    CFxArchive__WriteData((int *)a2, this + 268, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 280);
  else
    CFxArchive__WriteData((int *)a2, this + 280, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 292);
  else
    CFxArchive__WriteData((int *)a2, this + 292, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 296);
  else
    CFxArchive__WriteData((int *)a2, this + 296, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 300);
  else
    CFxArchive__WriteData((int *)a2, this + 300, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 304);
  else
    CFxArchive__WriteData((int *)a2, this + 304, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 308);
  else
    CFxArchive__WriteData((int *)a2, this + 308, 4);
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(4, (_DWORD *)a2, this + 312);
  else
    return CFxArchive__WriteData((int *)a2, this + 312, 4);
}

/* ---- COrientedParticle__ctor  0x0048EC50 ----  [CONFIRMED] */
_DWORD *__cdecl COrientedParticle__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559EF4;
  result[15] = 12;
  return result;
}

/* ---- COrientedParticle__scalar_dtor  0x0048EC70 ----  [CONFIRMED] */
char *__cdecl COrientedParticle__scalar_dtor(char *this, char a2)
{
  COrientedParticle__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491CE0(this, &dword_140C9F8);
  return this;
}

/* ---- COrientedParticle__dtor  0x0048ECA0 ----  VERIFIED */
void __cdecl COrientedParticle__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- COrientedParticle__Cull  0x0048ECC0 ----  [CONFIRMED] */
char __cdecl COrientedParticle__Cull(float *this)
{
  return SFxHelper__CullSphere(this + 32, (int)&theFxHelper, this[46]);
}

/* ---- COrientedParticle__Draw  0x0048ECE0 ----  [CONFIRMED] */
int __cdecl COrientedParticle__Draw(_DWORD *this)
{
  if ( (this[6] & 0x100000) != 0 )
    this[16] |= 8u;
  return dword_14328A0(this + 15, 0);
}

/* ---- COrientedParticle__Update  0x0048ED00 ----  [CONFIRMED] */
char __cdecl COrientedParticle__Update(int this)
{
  int v3;
  const float *v4;
  const float *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  if ( !CParticle__UpdateOrigin(this) )
    return 0;
  CParticle__UpdateSize(this);
  CParticle__UpdateSize2(this);
  CParticle__UpdateRGB(this);
  CParticle__UpdateAlpha(this);
  v3 = *(_DWORD *)(this + 216);
  *(float *)(this + 188) = (double)dword_1407514 * *(float *)(this + 308) * 0.0099999998 + *(float *)(this + 188);
  *(float *)(this + 308) = (1.0 - (double)dword_1407514 * 0.00065) * *(float *)(this + 308);
  if ( v3 )
  {
    v4 = (const float *)sub_48D770(v3);
    v5 = v4;
    if ( !v4 )
      return 0;
    OrientationPosToWorldPos(v4, (const float *)(this + 4), (float *)(this + 128));
    OrientationDirToWorldDir(v5, (const float *)(this + 316), (float *)(this + 88));
    return 1;
  }
  else
  {
    v6 = *(_DWORD *)(this + 4);
    v7 = *(_DWORD *)(this + 8);
    *(_DWORD *)(this + 136) = *(_DWORD *)(this + 12);
    v8 = *(_DWORD *)(this + 324);
    *(_DWORD *)(this + 128) = v6;
    v9 = *(_DWORD *)(this + 316);
    *(_DWORD *)(this + 132) = v7;
    v10 = *(_DWORD *)(this + 320);
    *(_DWORD *)(this + 96) = v8;
    *(_DWORD *)(this + 88) = v9;
    *(_DWORD *)(this + 92) = v10;
    return 1;
  }
}

/* ---- COrientedParticle__TypeID  0x0048EDF0 ----  [CONFIRMED] */
char COrientedParticle__TypeID()
{
  return 8;
}

/* ---- COrientedParticle__Archive  0x0048EE00 ----  [CONFIRMED] */
char __cdecl COrientedParticle__Archive(_BYTE *this, int a2)
{
  CParticle__Archive(this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(12, (_DWORD *)a2, this + 316);
  else
    return CFxArchive__WriteData((int *)a2, this + 316, 12);
}

/* ---- CLine__ctor  0x0048EE40 ----  [CONFIRMED] */
_DWORD *__cdecl CLine__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559EBC;
  result[15] = 13;
  return result;
}

/* ---- CLine__scalar_dtor  0x0048EE60 ----  [CONFIRMED] */
char *__cdecl CLine__scalar_dtor(char *this, char a2)
{
  CLine__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491BC0(this, &dword_140CA18);
  return this;
}

/* ---- CLine__dtor  0x0048EE90 ----  VERIFIED */
void __cdecl CLine__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- nullsub_34  0x0048EEB0 ----  [CONFIRMED] */
void nullsub_34()
{
  ;
}

/* ---- CLine__Cull  0x0048EEC0 ----  [CONFIRMED] */
char __cdecl CLine__Cull(float *this)
{
  return SFxHelper__CullCylinder(this + 32, this + 36, (int)&theFxHelper, this[46], this[46]);
}

/* ---- CLine__Draw  0x0048EEF0 ----  [CONFIRMED] */
int __cdecl CLine__Draw(_DWORD *this)
{
  if ( (this[6] & 0x100000) != 0 )
    this[16] |= 8u;
  return dword_14328A0(this + 15, 0);
}

/* ---- CLine__Update  0x0048EF10 ----  [CONFIRMED] */
char __cdecl CLine__Update(int this)
{
  int v3;
  const float *v4;
  const float *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  v3 = *(_DWORD *)(this + 216);
  if ( v3 )
  {
    v4 = (const float *)sub_48D770(v3);
    if ( !v4 )
      return 0;
    OrientationPosToWorldPos(v4, (const float *)(this + 4), (float *)(this + 128));
    OrientationPosToWorldPos(v4, (const float *)(this + 316), (float *)(this + 144));
  }
  else
  {
    v6 = *(_DWORD *)(this + 8);
    v7 = *(_DWORD *)(this + 12);
    *(_DWORD *)(this + 128) = *(_DWORD *)(this + 4);
    v8 = *(_DWORD *)(this + 316);
    *(_DWORD *)(this + 132) = v6;
    v9 = *(_DWORD *)(this + 320);
    *(_DWORD *)(this + 136) = v7;
    v10 = *(_DWORD *)(this + 324);
    *(_DWORD *)(this + 144) = v8;
    *(_DWORD *)(this + 148) = v9;
    *(_DWORD *)(this + 152) = v10;
  }
  CParticle__UpdateSize(this);
  CParticle__UpdateRGB(this);
  CParticle__UpdateAlpha(this);
  return 1;
}

/* ---- CLine__TypeID  0x0048EFC0 ----  [CONFIRMED] */
char CLine__TypeID()
{
  return 2;
}

/* ---- CLine__Archive  0x0048EFD0 ----  [CONFIRMED] */
char __cdecl CLine__Archive(_BYTE *this, int a2)
{
  CParticle__Archive(this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(12, (_DWORD *)a2, this + 316);
  else
    return CFxArchive__WriteData((int *)a2, this + 316, 12);
}

/* ---- CElectricity__ctor  0x0048F010 ----  [CONFIRMED] */
_DWORD *__cdecl CElectricity__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559F2C;
  result[15] = 14;
  return result;
}

/* ---- CElectricity__scalar_dtor  0x0048F030 ----  [CONFIRMED] */
char *__cdecl CElectricity__scalar_dtor(char *this, char a2)
{
  CElectricity__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491C50(this, &dword_140C9C0);
  return this;
}

/* ---- CElectricity__dtor  0x0048F060 ----  VERIFIED */
void __cdecl CElectricity__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- nullsub_35  0x0048F080 ----  [CONFIRMED] */
void nullsub_35()
{
  ;
}

/* ---- CElectricity__Cull  0x0048F090 ----  [CONFIRMED] */
char CElectricity__Cull()
{
  return 0;
}

/* ---- CElectricity__Initialize  0x0048F0A0 ----  [HIGH] */
int __cdecl CElectricity__Initialize(int a1)
{
  unsigned __int64 v1; // rax
  int v2;
  int result;

  v1 = (unsigned __int64)((double)rand() * 0.000030517578 * 1265536.0);
  v2 = *(_DWORD *)(a1 + 16);
  *(_DWORD *)(a1 + 140) = v1;
  result = *(_DWORD *)(a1 + 24);
  *(float *)(a1 + 188) = (float)(theFxHelper + *(_DWORD *)(a1 + 20) - v2);
  if ( (result & 0x100000) != 0 )
    *(_DWORD *)(a1 + 64) |= 8u;
  if ( (result & 0x2000000) != 0 )
    *(_DWORD *)(a1 + 64) |= 0x400u;
  if ( (result & 0x1000000) != 0 )
    *(_DWORD *)(a1 + 64) |= 0x800u;
  if ( (result & 0x4000000) != 0 )
    *(_DWORD *)(a1 + 64) |= 0x1000u;
  return result;
}

/* ---- CElectricity__Draw  0x0048F120 ----  [CONFIRMED] */
int __cdecl CElectricity__Draw(int this)
{
  int v1;

  v1 = *(_DWORD *)(this + 20);
  *(_DWORD *)(this + 192) = *(_DWORD *)(this + 328);
  *(float *)(this + 196) = (float)(v1 - *(_DWORD *)(this + 16));
  return dword_14328A0(this + 60, 0);
}

/* ---- CElectricity__Update  0x0048F160 ----  [CONFIRMED] */
char __cdecl CElectricity__Update(int this)
{
  int v4;
  const float *v5;
  const float *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  CParticle__UpdateSize(this);
  CParticle__UpdateRGB(this);
  CParticle__UpdateAlpha(this);
  v4 = *(_DWORD *)(this + 216);
  if ( v4 )
  {
    v5 = (const float *)sub_48D770(v4);
    if ( v5 )
    {
      OrientationPosToWorldPos(v5, (const float *)(this + 4), (float *)(this + 128));
      OrientationPosToWorldPos(v5, (const float *)(this + 316), (float *)(this + 144));
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    v7 = *(_DWORD *)(this + 4);
    v8 = *(_DWORD *)(this + 8);
    *(_DWORD *)(this + 136) = *(_DWORD *)(this + 12);
    v9 = *(_DWORD *)(this + 324);
    *(_DWORD *)(this + 128) = v7;
    v10 = *(_DWORD *)(this + 316);
    *(_DWORD *)(this + 132) = v8;
    v11 = *(_DWORD *)(this + 320);
    *(_DWORD *)(this + 152) = v9;
    *(_DWORD *)(this + 144) = v10;
    *(_DWORD *)(this + 148) = v11;
    return 1;
  }
}

/* ---- CElectricity__TypeID  0x0048F210 ----  [CONFIRMED] */
char CElectricity__TypeID()
{
  return 9;
}

/* ---- CElectricity__Archive  0x0048F220 ----  [CONFIRMED] */
char __cdecl CElectricity__Archive(_BYTE *this, int a2)
{
  CParticle__Archive(this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 316);
  else
    CFxArchive__WriteData((int *)a2, this + 316, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 328);
  else
    CFxArchive__WriteData((int *)a2, this + 328, 4);
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(384, (_DWORD *)a2, this + 332);
  else
    return CFxArchive__WriteData((int *)a2, this + 332, 384);
}

/* ---- CTail__ctor  0x0048F2A0 ----  [CONFIRMED] */
_DWORD *__cdecl CTail__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559F9C;
  result[15] = 13;
  return result;
}

/* ---- CTail__scalar_dtor  0x0048F2C0 ----  [CONFIRMED] */
char *__cdecl CTail__scalar_dtor(char *this, char a2)
{
  CTail__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491D70(this, &dword_140CA00);
  return this;
}

/* ---- CTail__dtor  0x0048F2F0 ----  VERIFIED */
void __cdecl CTail__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- CTail__Cull  0x0048F310 ----  [CONFIRMED] */
char __cdecl CTail__Cull(float *this)
{
  return SFxHelper__CullCylinder(this + 32, this + 36, (int)&theFxHelper, this[46], this[46]);
}

/* ---- CTail__Draw  0x0048F340 ----  [CONFIRMED] */
int __cdecl CTail__Draw(_DWORD *this)
{
  if ( (this[6] & 0x100000) != 0 )
    this[16] |= 8u;
  return dword_14328A0(this + 15, 0);
}

/* ---- CTail__Update  0x0048F360 ----  [CONFIRMED] */
char __cdecl CTail__Update(int this)
{
  int v3;
  int v4;
  _DWORD *v5;
  int v6;
  const float *v7;
  int v8;
  int v9;

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  v3 = *(_DWORD *)(this + 8);
  v4 = *(_DWORD *)(this + 12);
  v5 = (_DWORD *)(this + 4);
  *(_DWORD *)(this + 316) = *(_DWORD *)(this + 4);
  *(_DWORD *)(this + 320) = v3;
  *(_DWORD *)(this + 324) = v4;
  if ( !CParticle__UpdateOrigin(this) )
    return 0;
  v6 = *(_DWORD *)(this + 216);
  if ( v6 )
  {
    v7 = (const float *)sub_48D770(v6);
    if ( !v7 )
      return 0;
    OrientationPosToWorldPos(v7, (const float *)(this + 4), (float *)(this + 128));
  }
  else
  {
    v8 = *(_DWORD *)(this + 8);
    v9 = *(_DWORD *)(this + 12);
    *(_DWORD *)(this + 128) = *v5;
    *(_DWORD *)(this + 132) = v8;
    *(_DWORD *)(this + 136) = v9;
  }
  CParticle__UpdateSize(this);
  CTail__UpdateLength_m(this);
  CParticle__UpdateRGB(this);
  CParticle__UpdateAlpha(this);
  CTail__CalcNewEndpoint_m(this);
  return 1;
}

/* ---- CTail__UpdateLength_m  0x0048F400 ----  VERIFIED */
void __cdecl CTail__UpdateLength_m( int a1 )
{
  int   flags = *(_DWORD *)( a1 + 24 );
  int   mode;
  float linear = 1.0f;
  float frac   = 1.0f;

  if ( ( flags & 0x1000 ) != 0 )
    linear = (float)( 1.0 - (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                          / (double)( *(_DWORD *)( a1 + 20 ) - *(_DWORD *)( a1 + 16 ) ) );

  mode = flags & 0xC000;
  if ( mode == 0x4000 )
  {
    if ( (double)fxMTime > *(float *)( a1 + 336 ) )
      frac = (float)( 1.0 - ( (double)fxMTime - *(float *)( a1 + 336 ) )
                          / ( (double)*(int *)( a1 + 20 ) - *(float *)( a1 + 336 ) ) );
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x1000 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }
  else if ( mode == 0x8000 )
  {
    linear = (float)( cos( (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                           * *(float *)( a1 + 336 ) ) * linear );
  }
  else if ( mode == 0xC000 )
  {
    if ( (double)fxMTime < *(float *)( a1 + 336 ) )
      frac = (float)( ( *(float *)( a1 + 336 ) - (double)fxMTime )
                    / ( *(float *)( a1 + 336 ) - (double)*(int *)( a1 + 16 ) ) );
    else
      frac = 0.0f;
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x1000 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }

  if ( ( flags & 0x2000 ) != 0 )
    linear = (float)( (double)rand() * 0.000030517578125 * linear );

  *(float *)( a1 + 340 ) = (float)( ( 1.0 - linear ) * *(float *)( a1 + 332 )
                                    + linear * *(float *)( a1 + 328 ) );
}

/* ---- CTail__CalcNewEndpoint_m  0x0048F550 ----  VERIFIED */
void __cdecl CTail__CalcNewEndpoint_m( int a1 )
{
  int    bolt;
  float *orient;
  float  dir[3];
  float  x, y, z;

  dir[0] = *(float *)( a1 + 316 ) - *(float *)( a1 + 4 );
  dir[1] = *(float *)( a1 + 320 ) - *(float *)( a1 + 8 );
  dir[2] = *(float *)( a1 + 324 ) - *(float *)( a1 + 12 );
  VectorNormalize( dir );

  bolt = *(_DWORD *)( a1 + 216 );
  if ( !bolt )
  {
    *(float *)( a1 + 144 ) = dir[0] * *(float *)( a1 + 340 ) + *(float *)( a1 + 4 );
    *(float *)( a1 + 148 ) = dir[1] * *(float *)( a1 + 340 ) + *(float *)( a1 + 8 );
    *(float *)( a1 + 152 ) = dir[2] * *(float *)( a1 + 340 ) + *(float *)( a1 + 12 );
    return;
  }

  orient = 0;
  if ( *(int *)( bolt + 60 ) >= 0 )
  {
    if ( *(_DWORD *)( bolt + 4 ) == com_skelTimeStamp )
    {
      orient = (float *)( bolt + 8 );
    }
    else
    {
      *(_DWORD *)( bolt + 4 ) = com_skelTimeStamp;
      if ( FX_GetBoneOrientation( (float *)( bolt + 8 ), (_DWORD *)( bolt + 60 ) ) )
      {
        orient = (float *)( bolt + 8 );
      }
      else
      {
        *(_DWORD *)( bolt + 60 ) = -1;
        *(_DWORD *)( bolt + 64 ) = -1;
      }
    }
  }

  x = dir[0] * *(float *)( a1 + 340 ) + *(float *)( a1 + 4 );
  y = dir[1] * *(float *)( a1 + 340 ) + *(float *)( a1 + 8 );
  z = dir[2] * *(float *)( a1 + 340 ) + *(float *)( a1 + 12 );

  *(float *)( a1 + 144 ) = z * orient[9]  + y * orient[6]  + x * orient[3]  + orient[0];
  *(float *)( a1 + 148 ) = z * orient[10] + y * orient[7]  + x * orient[4]  + orient[1];
  *(float *)( a1 + 152 ) = z * orient[11] + y * orient[8]  + x * orient[5]  + orient[2];
}

/* ---- CTail__TypeID  0x0048F690 ----  [CONFIRMED] */
char CTail__TypeID()
{
  return 3;
}

/* ---- CTail__Archive  0x0048F6A0 ----  [CONFIRMED] */
char __cdecl CTail__Archive(_BYTE *this, int a2)
{
  CParticle__Archive(this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 316);
  else
    CFxArchive__WriteData((int *)a2, this + 316, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 328);
  else
    CFxArchive__WriteData((int *)a2, this + 328, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 332);
  else
    CFxArchive__WriteData((int *)a2, this + 332, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 336);
  else
    CFxArchive__WriteData((int *)a2, this + 336, 4);
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(4, (_DWORD *)a2, this + 340);
  else
    return CFxArchive__WriteData((int *)a2, this + 340, 4);
}

/* ---- CCylinder__ctor  0x0048F760 ----  [CONFIRMED] */
_DWORD *__cdecl CCylinder__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559ED8;
  result[15] = 15;
  return result;
}

/* ---- CCylinder__scalar_dtor  0x0048F780 ----  [CONFIRMED] */
char *__cdecl CCylinder__scalar_dtor(char *this, char a2)
{
  CCylinder__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491E00(this, &dword_140C9D8);
  return this;
}

/* ---- CCylinder__dtor  0x0048F7B0 ----  VERIFIED */
void __cdecl CCylinder__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- CCylinder__Cull  0x0048F7D0 ----  [CONFIRMED] */
char __cdecl CCylinder__Cull(float *this)
{
  return SFxHelper__CullCylinder(this + 32, this + 36, (int)&theFxHelper, this[40], this[46]);
}

/* ---- CCylinder__Draw  0x0048F800 ----  [CONFIRMED] */
int __cdecl CCylinder__Draw(_DWORD *this)
{
  if ( (this[6] & 0x100000) != 0 )
    this[16] |= 8u;
  return dword_14328A0(this + 15, 0);
}

/* ---- CCylinder__Update  0x0048F820 ----  [CONFIRMED] */
char __cdecl CCylinder__Update(int this)
{
  char result;
  int v4;
  const float *v5;
  float *v6;
  const float *v7;
  double v8;
  int v9;
  int v10;
  double v11;
  float pos[3]; // [esp+4h] [ebp-Ch] BYREF

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  CParticle__UpdateSize(this);
  CParticle__UpdateSize2(this);
  CTail__UpdateLength_m(this);
  CParticle__UpdateRGB(this);
  CParticle__UpdateAlpha(this);
  v4 = *(_DWORD *)(this + 216);
  if ( v4 )
  {
    v5 = (const float *)sub_48D770(v4);
    if ( v5 )
    {
      OrientationPosToWorldPos(v5, (const float *)(this + 4), (float *)(this + 128));
      pos[0] = *(float *)(this + 88) * *(float *)(this + 340) + *(float *)(this + 4);
      pos[1] = *(float *)(this + 92) * *(float *)(this + 340) + *(float *)(this + 8);
      pos[2] = *(float *)(this + 96) * *(float *)(this + 340) + *(float *)(this + 12);
      OrientationPosToWorldPos(v5, pos, (float *)(this + 144));
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    v8 = *(float *)(this + 88) * *(float *)(this + 340);
    v9 = *(_DWORD *)(this + 4);
    v10 = *(_DWORD *)(this + 8);
    *(_DWORD *)(this + 136) = *(_DWORD *)(this + 12);
    v11 = v8 + *(float *)(this + 4);
    *(_DWORD *)(this + 128) = v9;
    *(_DWORD *)(this + 132) = v10;
    *(float *)(this + 144) = v11;
    result = 1;
    *(float *)(this + 148) = *(float *)(this + 92) * *(float *)(this + 340) + *(float *)(this + 8);
    *(float *)(this + 152) = *(float *)(this + 96) * *(float *)(this + 340) + *(float *)(this + 12);
  }
  return result;
}

/* ---- CCylinder__TypeID  0x0048F920 ----  [CONFIRMED] */
char CCylinder__TypeID()
{
  return 4;
}

/* ---- CCylinder__Archive  0x0048F930 ----  [CONFIRMED] */
char __cdecl CCylinder__Archive(_BYTE *this, int a2)
{
  return CTail__Archive(this, a2);
}

/* ---- CEmitter__ctor  0x0048F940 ----  [CONFIRMED] */
_DWORD *__cdecl CEmitter__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559F48;
  result[15] = 1;
  return result;
}

/* ---- CEmitter__scalar_dtor  0x0048F960 ----  [CONFIRMED] */
char *__cdecl CEmitter__scalar_dtor(char *this, char a2)
{
  CEmitter__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491E90(this, &dword_140C9B8);
  return this;
}

/* ---- CEmitter__dtor  0x0048F990 ----  VERIFIED */
void __cdecl CEmitter__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- CEmitter__Cull  0x0048F9B0 ----  [CONFIRMED] */
char CEmitter__Cull()
{
  return 0;
}

/* ---- CEmitter__Draw  0x0048F9C0 ----  [CONFIRMED] */
int __cdecl CEmitter__Draw(_DWORD *this)
{
  int result;

  if ( (this[6] & 0x1000000) != 0 )
    return dword_14328A0(this + 15, 0);
  return result;
}

/* ---- CEmitter__Update  0x0048F9E0 ----  [CONFIRMED] */
char __cdecl CEmitter__Update(int this)
{
  int v1;
  float v3;
  float v4;
  float *v5;
  int v6;
  int v7;
  int v8;
  float v9;
  int v10;
  float *v11;
  float *v12;
  int v13;
  double v14;
  int v15;
  int v16;
  int v17;
  double v18;
  float *v19;
  double v20;
  double v21;
  int v22;
  int *v23;
  int v24;
  float *v25;
  float v26;
  float v27;
  int v28;
  float v29;
  double v30;
  float v31;
  double v32;
  float v33;
  float v34;
  float v35;
  int v36;
  float *v37;
  int *v38;
  int *v39;
  _BYTE v40[16]; // [esp-Ch] [ebp-D4h] BYREF
  int v41;
  float v42;
  float v43;
  float v44;
  float v45;
  float i;
  float v47;
  /* retail var_A8/var_A4/var_A0, frame 0x20..0x2C, is ONE vec3_t: 0x0048FF56
   * `lea eax, [esp+0DCh+var_A8]` hands that base to PlayEffect_id_axis as the
   * origin. */
  float v48[3]; // [esp+20h] [ebp-A8h] BYREF
  float v51;
  float v52;
  float v53;
  float v54;
  float v55;
  float v56;
  float v57;
  float v58;
  float v59;
  float v60;
  float v61;
  float v62;
  float v63;
  float v64;
  float v65;
  float v66;
  float v67;
  float v68;
  int v69;
  float v70;
  float v71;
  float v72;
  float v73;
  float v74;
  float v75;
  double v76;
  float v77[3]; // [esp+98h] [ebp-30h] BYREF

  v41 = this;
  v1 = 0;
  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  v3 = *(float *)(this + 8);
  v55 = *(float *)(this + 4);
  v4 = *(float *)(this + 12);
  v56 = v3;
  v57 = v4;
  if ( !CParticle__UpdateOrigin(this) )
    return 0;
  v5 = (float *)v41;
  if ( (v55 == *(float *)(v41 + 4)) | __UNORDERED__(v55, *(float *)(v41 + 4)) )
  {
    if ( (v56 == *(float *)(v41 + 8)) | __UNORDERED__(v56, *(float *)(v41 + 8)) )
    {
      if ( (v57 == *(float *)(v41 + 12)) | __UNORDERED__(v57, *(float *)(v41 + 12)) )
      {
        *(float *)(v41 + 372) = *(float *)(v41 + 372) * 0.69999999;
        *(float *)(v41 + 376) = *(float *)(v41 + 376) * 0.69999999;
        *(float *)(v41 + 380) = *(float *)(v41 + 380) * 0.69999999;
        v5 = (float *)v41;
      }
    }
  }
  CEmitter__UpdateAngles_m(v5);
  CParticle__UpdateSize(v41);
  v6 = v41;
  v7 = *(_DWORD *)(v41 + 216);
  if ( v7 )
  {
    v1 = sub_48D770(v7);
    if ( !v1 )
      return 0;
    v6 = v41;
  }
  if ( (*(_DWORD *)(v6 + 24) & 0x1000000) != 0 )
  {
    if ( *(_DWORD *)(v6 + 216) )
    {
      OrientationPosToWorldPos((const float *)v1, (const float *)(v6 + 4), (float *)(v6 + 128));
      v8 = 96;
      LODWORD(v9) = &v40[80];
      for ( i = v9; ; v9 = i )
      {
        v10 = v41;
        v11 = (float *)(v8 + v41 - 8);
        v12 = (float *)(v8 + LODWORD(v9));
        *v12 = *v11 * *(float *)(v41 + 184);
        *(float *)&v40[v8 + 84] = *(float *)(v8 + v10 - 4) * *(float *)(v10 + 184);
        *(float *)&v40[v8 + 88] = *(float *)(v8 + v10) * *(float *)(v10 + 184);
        OrientationDirToWorldDir((const float *)v1, v12, v11);
        v8 += 12;
        if ( v8 >= 132 )
          break;
      }
      *(_DWORD *)(v41 + 124) = *(_DWORD *)(v41 + 184);
      v6 = v41;
      v13 = *(_DWORD *)(v41 + 352);
      if ( v13 >= fxMTime )
      {
        v53 = 0.0;
        v52 = 0.0;
        v51 = 0.0;
      }
      else
      {
        v14 = *(float *)v1 - *(float *)(v41 + 340);
        LODWORD(i) = fxMTime - v13;
        v51 = v14;
        v52 = *(float *)(v1 + 4) - *(float *)(v41 + 344);
        v53 = *(float *)(v1 + 8) - *(float *)(v41 + 348);
        i = (float)(fxMTime - v13);
        v47 = i * 0.001;
        v51 = v47 * v51;
        v52 = v47 * v52;
        v53 = v47 * v53;
      }
    }
    else
    {
      *(_DWORD *)(v6 + 128) = *(_DWORD *)(v6 + 4);
      *(_DWORD *)(v41 + 132) = *(_DWORD *)(v41 + 8);
      *(_DWORD *)(v41 + 136) = *(_DWORD *)(v41 + 12);
      *(float *)(v41 + 88) = *(float *)(v41 + 184) * *(float *)(v41 + 88);
      *(float *)(v41 + 92) = *(float *)(v41 + 184) * *(float *)(v41 + 92);
      *(float *)(v41 + 96) = *(float *)(v41 + 184) * *(float *)(v41 + 96);
      *(float *)(v41 + 100) = *(float *)(v41 + 184) * *(float *)(v41 + 100);
      *(float *)(v41 + 104) = *(float *)(v41 + 184) * *(float *)(v41 + 104);
      *(float *)(v41 + 108) = *(float *)(v41 + 184) * *(float *)(v41 + 108);
      *(float *)(v41 + 112) = *(float *)(v41 + 184) * *(float *)(v41 + 112);
      *(float *)(v41 + 116) = *(float *)(v41 + 184) * *(float *)(v41 + 116);
      *(float *)(v41 + 120) = *(float *)(v41 + 184) * *(float *)(v41 + 120);
      *(_DWORD *)(v41 + 124) = *(_DWORD *)(v41 + 184);
      v6 = v41;
    }
  }
  if ( (*(_DWORD *)(v6 + 24) & 0x10000000) != 0 )
  {
    v15 = *(_DWORD *)(v6 + 352);
    v47 = *(float *)(v6 + 356);
    *(float *)&v16 = 0.0;
    v59 = v47 * v47;
    if ( v15 < fxMTime )
    {
      while ( 1 )
      {
        v16 += 12;
        v42 = *(float *)&v16;
        v17 = *(_DWORD *)(v6 + 216);
        i = (float)v16;
        v45 = i * 0.001;
        v43 = v45;
        v42 = v45 * v45 * 0.5;
        v73 = *(float *)(v6 + 340) + *(float *)(v6 + 316);
        v74 = *(float *)(v6 + 344) + *(float *)(v6 + 320);
        v75 = *(float *)(v6 + 348) + *(float *)(v6 + 324);
        v48[0] = v45 * *(float *)(v6 + 328) + v42 * *(float *)(v6 + 232) + *(float *)(v6 + 316);
        v48[1] = v45 * *(float *)(v6 + 332) + v42 * *(float *)(v6 + 236) + *(float *)(v6 + 320);
        v48[2] = v45 * *(float *)(v6 + 336) + v42 * *(float *)(v6 + 240) + *(float *)(v6 + 324);
        if ( v17 )
        {
          v48[0] = v43 * v51 + v48[0];
          v48[1] = v43 * v52 + v48[1];
          v48[2] = v43 * v53 + v48[2];
        }
        v55 = v73 - v48[0];
        v56 = v74 - v48[1];
        v57 = v75 - v48[2];
        v18 = v57 * v57 + v56 * v56 + v55 * v55;
        if ( !((v18 < v59) | __UNORDERED__(v18, v59)) )
          break;
        v15 += 12;
LABEL_42:
        if ( v15 >= fxMTime )
          return 1;
      }
      if ( !v17 )
      {
        v39 = 0;
        v23 = (int *)(v6 + 88);
        v24 = *(_DWORD *)(v6 + 384);
        v38 = v23;
        v37 = v48;                                  /* 0x0048FF56 */
        v36 = v24;
        goto LABEL_37;
      }
      if ( *(int *)(v17 + 60) < 0 )
      {
        v19 = 0;
LABEL_35:
        v20 = v48[2] * v19[9];
        v39 = (int *)(*(_DWORD *)(v6 + 216) + 60);
        v77[0] = v20 + v48[1] * v19[6] + v48[0] * v19[3] + *v19;
        v77[1] = v48[2] * v19[10] + v48[1] * v19[7] + v48[0] * v19[4] + v19[1];
        v21 = v48[2] * v19[11] + v48[1] * v19[8] + v48[0] * v19[5] + v19[2];
        v38 = (int *)(v6 + 88);
        v22 = *(_DWORD *)(v6 + 384);
        v37 = v77;
        v77[2] = v21;
        v36 = v22;
LABEL_37:
        CFxScheduler__PlayEffect_id_axis(dword_14075A0, v36, v37, v38, v39);
        v25 = (float *)v41;
        v26 = *(float *)(v41 + 332);
        v62 = *(float *)(v41 + 336);
        v65 = *(float *)(v41 + 328);
        v61 = v26;
        v27 = *(float *)(v41 + 236);
        v60 = *(float *)(v41 + 240);
        v66 = *(float *)(v41 + 232);
        v68 = v27;
        v54 = v45;
        v67 = v65 * v65 + v61 * v61 + v62 * v62;
        v63 = *(float *)(v41 + 336) * *(float *)(v41 + 240)
            + *(float *)(v41 + 332) * *(float *)(v41 + 236)
            + *(float *)(v41 + 328) * *(float *)(v41 + 232);
        v64 = v66 * v66 + v68 * v68 + v60 * v60;
        v58 = v45 * v45;
        v42 = v58 * v45;
        v44 = v42 * v45;
        i = v42 * v64 + v58 * v63 * 3.0 + v45 * v67 + v45 * v67;
        if ( !((i == 0.0) | __UNORDERED__(i, 0.0)) )
        {
          v76 = 9.313225746154785e-10;
          v44 = v44 * v64 * 0.25 + v42 * v63 + v58 * v67 - v59;
          v44 = v44 / i * 1000.0;
          v69 = (int)(v44 + 9.313225746154785e-10);
          v25 = (float *)v41;
          v28 = *(_DWORD *)(v41 + 352);
          v16 -= v69;
          v42 = *(float *)&v16;
          if ( v16 < v15 - v28 )
          {
            v16 = v15 - v28;
            LODWORD(v42) = v15 - v28;
          }
          v44 = (float)SLODWORD(v42);
          v45 = v44 * 0.001;
          v43 = v45;
          v42 = v45 * v45 * 0.5;
          v48[0] = v45 * *(float *)(v41 + 328) + v42 * *(float *)(v41 + 232) + *(float *)(v41 + 316);
          v48[1] = v45 * *(float *)(v41 + 332) + v42 * *(float *)(v41 + 236) + *(float *)(v41 + 320);
          v48[2] = v45 * *(float *)(v41 + 336) + v42 * *(float *)(v41 + 240) + *(float *)(v41 + 324);
        }
        v29 = v48[1];
        v70 = v45 * v25[58] + v25[82];
        v71 = v45 * v25[59] + v25[83];
        v30 = v45 * v25[60] + v25[84];
        v25[79] = v48[0];
        v31 = v48[2];
        *(float *)(v41 + 320) = v29;
        v72 = v30;
        v32 = v45 * v51;
        v33 = v70;
        *(float *)(v41 + 324) = v31;
        v34 = v71;
        *(float *)(v41 + 328) = v33;
        v35 = v72;
        *(float *)(v41 + 332) = v34;
        *(float *)(v41 + 336) = v35;
        *(float *)(v41 + 340) = v32 + *(float *)(v41 + 340);
        *(float *)(v41 + 344) = v45 * v52 + *(float *)(v41 + 344);
        *(float *)(v41 + 348) = v45 * v53 + *(float *)(v41 + 348);
        *(_DWORD *)(v41 + 352) += v16;
        v15 = *(_DWORD *)(v41 + 352);
        *(float *)&v16 = 0.0;
        v44 = (float)rand();
        *(float *)(v41 + 356) = (v44 / 32768.0 + v44 / 32768.0 - 1.0) * *(float *)(v41 + 392) + *(float *)(v41 + 388);
        v6 = v41;
        v47 = *(float *)(v41 + 356);
        v59 = v47 * v47;
        goto LABEL_42;
      }
      if ( *(_DWORD *)(v17 + 4) != com_skelTimeStamp )
      {
        *(_DWORD *)(v17 + 4) = com_skelTimeStamp;
        if ( !FX_GetBoneOrientation((float *)(v17 + 8), (_DWORD *)(v17 + 60)) )
        {
          *(_DWORD *)(v17 + 60) = -1;
          *(_DWORD *)(v17 + 64) = -1;
          v6 = v41;
          v19 = 0;
          goto LABEL_35;
        }
        v6 = v41;
      }
      v19 = (float *)(v17 + 8);
      goto LABEL_35;
    }
  }
  return 1;
}

/* ---- CEmitter__UpdateAngles_m  0x00490300 ----  VERIFIED */
float *__cdecl CEmitter__UpdateAngles_m( float *a1 )
{
  float  right[3];
  float  ft;

  ft = (float)dword_1407514;
  a1[90] = (float)( ft * 0.0099999998f * a1[93] + a1[90] );
  ft = (float)dword_1407514;
  a1[91] = (float)( ft * 0.0099999998f * a1[94] + a1[91] );
  ft = (float)dword_1407514;
  a1[92] = (float)( ft * 0.0099999998f * a1[95] + a1[92] );

  AngleVectors( a1 + 90, a1 + 22, right, a1 + 28 );

  a1[25] = 0.0f - right[0];
  a1[26] = 0.0f - right[1];
  a1[27] = 0.0f - right[2];
  return a1;
}

/* ---- CEmitter__TypeID  0x004903C0 ----  [CONFIRMED] */
char CEmitter__TypeID()
{
  return 5;
}

/* ---- CEmitter__Archive  0x004903D0 ----  [CONFIRMED] */
int __cdecl CEmitter__Archive(int this, int a2)
{
  int v2;
  int result;

  v2 = a2;
  CParticle__Archive((_BYTE *)this, a2);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)v2, (_BYTE *)(this + 316));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 316), 12);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)v2, (_BYTE *)(this + 328));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 328), 12);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)v2, (_BYTE *)(this + 340));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 340), 12);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)v2, (_BYTE *)(this + 352));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 352), 4);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)v2, (_BYTE *)(this + 356));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 356), 4);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)v2, (_BYTE *)(this + 360));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 360), 12);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)v2, (_BYTE *)(this + 372));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 372), 12);
  if ( *(_BYTE *)(v2 + 12) )
  {
    CFxArchive__ReadData(2, (_DWORD *)v2, &a2);
    *(_DWORD *)(this + 384) = *(_DWORD *)(v2 + 4 * (__int16)a2 + 20);
  }
  else
  {
    a2 = *(unsigned __int16 *)(this + 384);
    CFxArchive__WriteData((int *)v2, &a2, 2);
  }
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)v2, (_BYTE *)(this + 388));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 388), 4);
  if ( *(_BYTE *)(v2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)v2, (_BYTE *)(this + 392));
  else
    CFxArchive__WriteData((int *)v2, (_BYTE *)(this + 392), 4);
  result = *(_DWORD *)(this + 204);
  if ( !result )
    *(_DWORD *)(this + 24) &= ~0x1000000u;
  return result;
}

/* ---- CLight__ctor  0x00490570 ----  [CONFIRMED] */
_DWORD *__cdecl CLight__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559F64;
  return result;
}

/* ---- CLight__scalar_dtor  0x00490590 ----  [CONFIRMED] */
char *__cdecl CLight__scalar_dtor(char *this, char a2)
{
  CLight__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491AA0(this, &dword_140C9E8);
  return this;
}

/* ---- CLight__dtor  0x004905C0 ----  VERIFIED */
void __cdecl CLight__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- CLight__Cull  0x004905E0 ----  [CONFIRMED] */
char __cdecl CLight__Cull(float *this)
{
  return SFxHelper__CullSphere(this + 32, (int)&theFxHelper, this[46]);
}

/* ---- CLight__Draw  0x00490600 ----  [CONFIRMED] */
int __cdecl CLight__Draw(float *this)
{
  return ((int (__cdecl *)(_DWORD, _DWORD, _DWORD, _DWORD, _DWORD))dword_14328AC)(
           this + 32,
           this[46],
           this[18],
           this[19],
           this[20]);
}

/* ---- CLight__Update  0x00490630 ----  [CONFIRMED] */
char __cdecl CLight__Update(int this)
{
  int v4;
  const float *v5;
  int v6;
  int v7;

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  CLight__UpdateSize_m(this);
  CLight__UpdateRGB_m(this);
  v4 = *(_DWORD *)(this + 216);
  if ( v4 )
  {
    v5 = (const float *)sub_48D770(v4);
    if ( v5 )
    {
      OrientationPosToWorldPos(v5, (const float *)(this + 4), (float *)(this + 128));
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    v6 = *(_DWORD *)(this + 4);
    v7 = *(_DWORD *)(this + 8);
    *(_DWORD *)(this + 136) = *(_DWORD *)(this + 12);
    *(_DWORD *)(this + 128) = v6;
    *(_DWORD *)(this + 132) = v7;
    return 1;
  }
}

/* ---- CLight__UpdateSize_m  0x004906A0 ----  VERIFIED */
void __cdecl CLight__UpdateSize_m( int a1 )
{
  int   flags = *(_DWORD *)( a1 + 24 );
  int   mode;
  float linear = 1.0f;
  float frac   = 1.0f;

  if ( ( flags & 0x100 ) != 0 )
    linear = (float)( 1.0 - (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                          / (double)( *(_DWORD *)( a1 + 20 ) - *(_DWORD *)( a1 + 16 ) ) );

  mode = flags & 0xC00;
  if ( mode == 0x400 )
  {
    if ( (double)fxMTime > *(float *)( a1 + 228 ) )
      frac = (float)( 1.0 - ( (double)fxMTime - *(float *)( a1 + 228 ) )
                          / ( (double)*(int *)( a1 + 20 ) - *(float *)( a1 + 228 ) ) );
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x100 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }
  else if ( mode == 0x800 )
  {
    linear = (float)( cos( (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                           * *(float *)( a1 + 228 ) ) * linear );
  }
  else if ( mode == 0xC00 )
  {
    if ( (double)fxMTime < *(float *)( a1 + 228 ) )
      frac = (float)( ( *(float *)( a1 + 228 ) - (double)fxMTime )
                    / ( *(float *)( a1 + 228 ) - (double)*(int *)( a1 + 16 ) ) );
    else
      frac = 0.0f;
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x100 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }

  if ( ( flags & 0x200 ) != 0 )
    linear = (float)( (double)rand() * 0.000030517578125 * linear );

  *(float *)( a1 + 184 ) = (float)( ( 1.0 - linear ) * *(float *)( a1 + 224 )
                                    + linear * *(float *)( a1 + 220 ) );
}

/* ---- CLight__UpdateRGB_m  0x004907F0 ----  VERIFIED */
void __cdecl CLight__UpdateRGB_m( int a1 )
{
  int   flags = *(_DWORD *)( a1 + 24 );
  int   mode;
  float linear = 1.0f;
  float frac   = 1.0f;
  float inv;

  if ( ( flags & 0x10 ) != 0 )
    linear = (float)( 1.0 - (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                          / (double)( *(_DWORD *)( a1 + 20 ) - *(_DWORD *)( a1 + 16 ) ) );

  mode = flags & 0xC0;
  if ( mode == 0x40 )
  {
    if ( (double)fxMTime > *(float *)( a1 + 256 ) )
      frac = (float)( 1.0 - ( (double)fxMTime - *(float *)( a1 + 256 ) )
                          / ( (double)*(int *)( a1 + 20 ) - *(float *)( a1 + 256 ) ) );
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x10 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }
  else if ( mode == 0x80 )
  {
    linear = (float)( cos( (double)( fxMTime - *(_DWORD *)( a1 + 16 ) )
                           * *(float *)( a1 + 256 ) ) * linear );
  }
  else if ( mode == 0xC0 )
  {
    if ( (double)fxMTime < *(float *)( a1 + 256 ) )
      frac = (float)( ( *(float *)( a1 + 256 ) - (double)fxMTime )
                    / ( *(float *)( a1 + 256 ) - (double)*(int *)( a1 + 16 ) ) );
    else
      frac = 0.0f;
    if ( ( *(_DWORD *)( a1 + 24 ) & 0x10 ) != 0 )
      frac = (float)( ( frac + linear ) * 0.5 );
    linear = frac;
  }

  if ( ( flags & 0x20 ) != 0 )
    linear = (float)( (double)rand() * 0.000030517578125 * linear );

  inv = (float)( 1.0 - linear );
  *(float *)( a1 + 72 ) = (float)( inv * *(float *)( a1 + 244 ) + linear * *(float *)( a1 + 232 ) );
  *(float *)( a1 + 76 ) = (float)( inv * *(float *)( a1 + 248 ) + linear * *(float *)( a1 + 236 ) );
  *(float *)( a1 + 80 ) = (float)( inv * *(float *)( a1 + 252 ) + linear * *(float *)( a1 + 240 ) );
}

/* ---- CLight__TypeID  0x00490970 ----  [CONFIRMED] */
char CLight__TypeID()
{
  return 11;
}

/* ---- CLight__Archive  0x00490980 ----  [CONFIRMED] */
char __cdecl CLight__Archive(_BYTE *this, int a2)
{
  CEffect__Archive((int)this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 220);
  else
    CFxArchive__WriteData((int *)a2, this + 220, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 224);
  else
    CFxArchive__WriteData((int *)a2, this + 224, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 228);
  else
    CFxArchive__WriteData((int *)a2, this + 228, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 232);
  else
    CFxArchive__WriteData((int *)a2, this + 232, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 244);
  else
    CFxArchive__WriteData((int *)a2, this + 244, 12);
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(4, (_DWORD *)a2, this + 256);
  else
    return CFxArchive__WriteData((int *)a2, this + 256, 4);
}

/* ---- cand_CFxPrimType14__Cull  0x00490A70 ----  VERIFIED */
char cand_CFxPrimType14__Cull()
{
  return 0;
}

/* ---- cand_CFxPrimType14__Draw  0x00490A80 ----  VERIFIED */
int __cdecl cand_CFxPrimType14__Draw(int this)
{
  int v2;
  double v3;
  int v4;
  int v5;
  double v6;
  int v7;
  int v8;
  double v9;
  int v10;
  double v11;
  int v12;
  int v13;
  int v14;
  double v15;
  double v16;
  int v17;
  int v18;
  int v19;
  int v20;
  int v21;
  double v22;
  int v23;
  double v24;
  int v25;
  int v26;
  int v27;
  double v28;
  int v29;
  int v30;
  int v31;
  int v32;
  double v33;
  double v34;
  int v35;
  int v36;
  int v37;
  int v38;
  int v39;
  int v41;
  float v42;
  float v43;
  float v44;
  float v45;
  float v46;
  float v47;
  float v48;
  float v49;
  float v50;
  float v51;
  float v52;
  float v53;
  int v54; // [esp+10h] [ebp-60h] BYREF
  int v55;
  int v56;
  int v57;
  int v58;
  char v59;
  char v60;
  char v61;
  int v62;
  int v63;
  int v64;
  int v65;
  int v66;
  char v67;
  char v68;
  char v69;
  int v70;
  int v71;
  int v72;
  int v73;
  int v74;
  char v75;
  char v76;
  char v77;

  v2 = *(_DWORD *)(this + 220);
  v3 = *(float *)(this + 256) * *(float *)(this + 276);
  v4 = *(_DWORD *)(this + 224);
  v5 = *(_DWORD *)(this + 228);
  v6 = *(float *)(this + 260) * *(float *)(this + 276);
  v54 = v2;
  v7 = *(_DWORD *)(this + 304);
  v55 = v4;
  v8 = *(_DWORD *)(this + 308);
  v42 = v6;
  v9 = *(float *)(this + 264);
  v56 = v5;
  v10 = *(_DWORD *)(this + 312);
  v11 = v9 * *(float *)(this + 276);
  v62 = v7;
  v12 = *(_DWORD *)(this + 472);
  v63 = v8;
  v48 = v11;
  v13 = *(_DWORD *)(this + 476);
  v64 = v10;
  v14 = *(_DWORD *)(this + 480);
  v70 = v12;
  v71 = v13;
  v72 = v14;
  v59 = (unsigned __int64)v3;
  v60 = (unsigned __int64)v42;
  v15 = *(float *)(this + 360) * *(float *)(this + 340);
  v61 = (unsigned __int64)v48;
  v43 = *(float *)(this + 360) * *(float *)(this + 344);
  v49 = *(float *)(this + 360) * *(float *)(this + 348);
  v67 = (unsigned __int64)v15;
  v68 = (unsigned __int64)v43;
  v16 = *(float *)(this + 528) * *(float *)(this + 508);
  v69 = (unsigned __int64)v49;
  v44 = *(float *)(this + 528) * *(float *)(this + 512);
  v50 = *(float *)(this + 528) * *(float *)(this + 516);
  v75 = (unsigned __int64)v16;
  v76 = (unsigned __int64)v44;
  v17 = *(_DWORD *)(this + 300);
  v18 = *(_DWORD *)(this + 380);
  v77 = (unsigned __int64)v50;
  v57 = *(_DWORD *)(this + 296);
  v19 = *(_DWORD *)(this + 384);
  v58 = v17;
  v20 = *(_DWORD *)(this + 548);
  v65 = v18;
  v21 = *(_DWORD *)(this + 552);
  v66 = v19;
  v73 = v20;
  v74 = v21;
  dword_14328A4(*(_DWORD *)(this + 556), 3, &v54);
  v22 = *(float *)(this + 528) * *(float *)(this + 508);
  v23 = *(_DWORD *)(this + 476);
  v24 = *(float *)(this + 528) * *(float *)(this + 512);
  v25 = *(_DWORD *)(this + 480);
  v54 = *(_DWORD *)(this + 472);
  v26 = *(_DWORD *)(this + 388);
  v45 = v24;
  v55 = v23;
  v27 = *(_DWORD *)(this + 392);
  v28 = *(float *)(this + 528) * *(float *)(this + 516);
  v56 = v25;
  v29 = *(_DWORD *)(this + 396);
  v62 = v26;
  v30 = *(_DWORD *)(this + 304);
  v51 = v28;
  v63 = v27;
  v31 = *(_DWORD *)(this + 308);
  v64 = v29;
  v32 = *(_DWORD *)(this + 312);
  v70 = v30;
  v71 = v31;
  v72 = v32;
  v59 = (unsigned __int64)v22;
  v60 = (unsigned __int64)v45;
  v33 = *(float *)(this + 424) * *(float *)(this + 444);
  v61 = (unsigned __int64)v51;
  v46 = *(float *)(this + 428) * *(float *)(this + 444);
  v52 = *(float *)(this + 432) * *(float *)(this + 444);
  v67 = (unsigned __int64)v33;
  v68 = (unsigned __int64)v46;
  v34 = *(float *)(this + 360) * *(float *)(this + 340);
  v69 = (unsigned __int64)v52;
  v47 = *(float *)(this + 360) * *(float *)(this + 344);
  v53 = *(float *)(this + 360) * *(float *)(this + 348);
  v75 = (unsigned __int64)v34;
  v76 = (unsigned __int64)v47;
  v35 = *(_DWORD *)(this + 548);
  v36 = *(_DWORD *)(this + 464);
  v77 = (unsigned __int64)v53;
  v37 = *(_DWORD *)(this + 552);
  v57 = v35;
  v38 = *(_DWORD *)(this + 468);
  v58 = v37;
  v39 = *(_DWORD *)(this + 380);
  v65 = v36;
  v66 = v38;
  v73 = v39;
  v41 = *(_DWORD *)(this + 556);
  v74 = *(_DWORD *)(this + 384);
  return dword_14328A4(v41, 3, &v54);
}

/* ---- cand_CFxPrimType14__Update  0x00490DC0 ----  VERIFIED */
char __cdecl cand_CFxPrimType14__Update(float *this)
{
  int v1;
  int v3;
  float *v4;
  double v5;
  int v6;
  double v7;
  double v8;
  double v9;

  v1 = *((_DWORD *)this + 4);
  if ( v1 > fxMTime )
    return 0;
  v3 = *((_DWORD *)this + 5);
  v4 = this + 69;
  v5 = (double)(v3 - fxMTime) / (double)(v3 - v1);
  v6 = 4;
  v7 = 1.0 - v5;
  do
  {
    v8 = v7 * *(v4 - 1) + v5 * *(v4 - 2);
    *v4 = v8;
    if ( (v8 < 0.0) | __UNORDERED__(v8, 0.0) )
      *v4 = 0.0;
    *(v4 - 5) = v5 * *(v4 - 11) + v7 * *(v4 - 8);
    *(v4 - 4) = v5 * *(v4 - 10) + v7 * *(v4 - 7);
    *(v4 - 3) = v5 * *(v4 - 9) + v7 * *(v4 - 6);
    v9 = v5 * v4[1] + v7 * v4[3];
    v4[5] = v9;
    if ( v9 > 1.0 )
      v4[5] = 1.0;
    v4 += 21;
    --v6;
    *(v4 - 15) = v5 * *(v4 - 19) + v7 * *(v4 - 17);
  }
  while ( v6 );
  return 1;
}

/* ---- cand_CFxPrimType14__TypeID  0x00490EA0 ----  [HIGH] */
char cand_CFxPrimType14__TypeID()
{
  return 14;
}

/* ---- cand_CFxPrimType14__Archive  0x00490EB0 ----  VERIFIED */
int __cdecl cand_CFxPrimType14__Archive(int this, int a2)
{
  int result;

  CEffect__Archive(this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(336, (_DWORD *)a2, (_BYTE *)(this + 220));
  else
    CFxArchive__WriteData((int *)a2, (_BYTE *)(this + 220), 336);
  if ( !*(_BYTE *)(a2 + 12) )
    return CFxArchive__WriteShader_m(*(_DWORD *)(this + 556), (int *)a2);
  result = CFxArchive__ReadShader_m((_DWORD *)a2);
  *(_DWORD *)(this + 556) = result;
  return result;
}

/* ---- CFlash__ctor  0x00490F10 ----  [CONFIRMED] */
_DWORD *__cdecl CFlash__ctor(_DWORD *result)
{
  result[54] = 0;
  result[15] = 4;
  *result = &off_559EA0;
  return result;
}

/* ---- CFlash__scalar_dtor  0x00490F30 ----  [CONFIRMED] */
char *__cdecl CFlash__scalar_dtor(char *this, char a2)
{
  CFlash__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491F20(this, &dword_140C9F0);
  return this;
}

/* ---- CFlash__dtor  0x00490F60 ----  VERIFIED */
void __cdecl CFlash__dtor( void *self )
{
  _DWORD *this = (_DWORD *)self;
  char   *bolt;

  *this = (_DWORD)&off_559F80;
  bolt  = (char *)this[54];
  if ( bolt )
    CFxBoltFrame__Release( bolt );
}

/* ---- CFlash__Cull  0x00490F80 ----  [CONFIRMED] */
char __cdecl CFlash__Cull(float *this)
{
  return SFxHelper__CullSphere(this + 1, (int)&theFxHelper, this[46]);
}

/* ---- CFlash__Draw  0x00490FA0 ----  [CONFIRMED] */
int __cdecl CFlash__Draw(int this)
{
  int v1;
  char v2;
  float *v3;
  float *v4;
  float *v5;
  double v6;
  char v7;
  double v8;
  double v9;
  float v10;
  _BYTE v12[160]; // [esp+4h] [ebp-A0h] BYREF

  v1 = *(_DWORD *)(this + 316);
  if ( v1 > 0 )
  {
    v2 = *(_BYTE *)(this + 168);
    v3 = (float *)v12;
    v4 = (float *)(this + 396);
    v5 = (float *)(this + 340);
    do
    {
      v6 = *(v5 - 1) + *(float *)(this + 4);
      *((_BYTE *)v3 + 29) = *(_BYTE *)(this + 169);
      *((_BYTE *)v3 + 30) = *(_BYTE *)(this + 170);
      v7 = *(_BYTE *)(this + 171);
      *v3 = v6;
      v8 = *v5;
      *((_BYTE *)v3 + 31) = v7;
      v9 = v8 + *(float *)(this + 8);
      v10 = *v4;
      *((_BYTE *)v3 + 28) = v2;
      v5 += 3;
      v3[1] = v9;
      v3 += 8;
      v4 += 2;
      --v1;
      *(v3 - 6) = *(v5 - 2) + *(float *)(this + 12);
      *(v3 - 5) = v10;
      *(v3 - 4) = *(v4 - 1);
    }
    while ( v1 );
  }
  return dword_14328A4(*(_DWORD *)(this + 164), *(_DWORD *)(this + 316), v12);
}

/* ---- CFlash__Init_m  0x00491050 ----  VERIFIED */
int __stdcall CFlash__Init_m(int a1)
{
  double angle1, angle2;
  double c1, s1, c2, s2;
  int count, i;
  float *p, x, y, z;

  angle1 = (double)dword_1407514 * *(float *)(a1 + 324) * 0.0099999998 * 3.1415927 / 180.0;
  c1 = cos(angle1);
  s1 = sin(angle1);

  angle2 = (double)dword_1407514 * *(float *)(a1 + 320) * 0.0099999998 * 3.1415927 / 180.0;
  c2 = cos(angle2);
  s2 = sin(angle2);

  count = *(_DWORD *)(a1 + 316);
  for ( i = 0; i < count; i++ )
  {
    p = (float *)(a1 + 336 + 12 * i);
    x = p[0];
    y = p[1];
    z = p[2];
    p[0] = (float)(c1 * x + (s2 * s1) * y + (c2 * s1) * z);
    p[1] = (float)(-s1 * x + (s2 * c1) * y + (c2 * c1) * z);
    p[2] = (float)(s2 * z - c2 * y);
  }
  return a1;
}

/* ---- CFlash__Update  0x00491230 ----  [CONFIRMED] */
char __cdecl CFlash__Update(int this)
{
  float v3;
  float v4;
  float v5;

  if ( *(_DWORD *)(this + 16) > fxMTime )
    return 0;
  CParticle__UpdateRGB(this);
  CParticle__UpdateAlpha(this);
  if ( fxMTime > *(_DWORD *)(this + 332) )
  {
    v3 = *(float *)(this + 4);
    v4 = *(float *)(this + 8);
    v5 = *(float *)(this + 12);
    if ( !CParticle__UpdateOrigin(this) )
      return 0;
    CFlash__Init_m(this);
    if ( (v3 == *(float *)(this + 4)) | __UNORDERED__(v3, *(float *)(this + 4)) )
    {
      if ( (v4 == *(float *)(this + 8)) | __UNORDERED__(v4, *(float *)(this + 8)) )
      {
        if ( (v5 == *(float *)(this + 12)) | __UNORDERED__(v5, *(float *)(this + 12)) )
        {
          *(float *)(this + 320) = (1.0 - (double)dword_1407514 * 0.0064999997) * *(float *)(this + 320);
          *(float *)(this + 324) = (1.0 - (double)dword_1407514 * 0.0064999997) * *(float *)(this + 324);
          *(float *)(this + 328) = (1.0 - (double)dword_1407514 * 0.0064999997) * *(float *)(this + 328);
        }
      }
    }
  }
  return 1;
}

/* ---- CFlash__DrawHelper_m  0x00491320 ----  VERIFIED */
void __fastcall CFlash__DrawHelper_m(int a1, int a2)
{
  double v2;
  double v3;
  double v4;
  int v5;
  int v6;
  int v7;
  double v8;
  double v9;
  float *v10;
  double v11;
  float v12;
  float v13;
  float v14;
  float v15;

  if ( *(int *)(a2 + 316) >= 3 )
  {
    v2 = 0.0;
    v3 = 0.0;
    v4 = 0.0;
    v5 = a2 + 340;
    v6 = *(_DWORD *)(a2 + 316);
    do
    {
      v5 += 12;
      --v6;
      v2 = v2 + *(float *)(v5 - 16);
      v3 = v3 + *(float *)(v5 - 12);
      v4 = v4 + *(float *)(v5 - 8);
    }
    while ( v6 );
    v15 = v4;
    v7 = 0;
    v8 = 1.0 / (double)*(int *)(a2 + 316);
    v12 = v8;
    v14 = v8 * v2;
    *(float *)(a2 + 4) = v14;
    *(float *)(a2 + 8) = v12 * v3;
    *(float *)(a2 + 12) = v12 * v15;
    v9 = 0.0;
    v10 = (float *)(a2 + 336);
    do
    {
      *v10 = *v10 - *(float *)(a2 + 4);
      v10[1] = v10[1] - *(float *)(a2 + 8);
      v11 = v10[2] - *(float *)(a2 + 12);
      v10[2] = v11;
      v13 = v11 * v11 + v10[1] * v10[1] + *v10 * *v10;
      if ( (v9 < v13) | __UNORDERED__(v9, v13) )
        v9 = v13;
      ++v7;
      v10 += 3;
    }
    while ( v7 < *(_DWORD *)(a2 + 316) );
    *(float *)(a2 + 184) = sqrt(v9);
  }
}

/* ---- CFlash__TypeID  0x00491430 ----  [CONFIRMED] */
char CFlash__TypeID()
{
  return 15;
}

/* ---- CFlash__Archive  0x00491440 ----  [CONFIRMED] */
char __cdecl CFlash__Archive(_BYTE *this, int a2)
{
  CParticle__Archive(this, a2);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 316);
  else
    CFxArchive__WriteData((int *)a2, this + 316, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a2, this + 320);
  else
    CFxArchive__WriteData((int *)a2, this + 320, 12);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a2, this + 332);
  else
    CFxArchive__WriteData((int *)a2, this + 332, 4);
  if ( *(_BYTE *)(a2 + 12) )
    CFxArchive__ReadData(60, (_DWORD *)a2, this + 336);
  else
    CFxArchive__WriteData((int *)a2, this + 336, 60);
  if ( *(_BYTE *)(a2 + 12) )
    return CFxArchive__ReadData(40, (_DWORD *)a2, this + 396);
  else
    return CFxArchive__WriteData((int *)a2, this + 396, 40);
}

/* ---- cand_CFxPrimType13__Update  0x00491500 ----  VERIFIED */
char __cdecl cand_CFxPrimType13__Update(void *this)
{
  CLight__UpdateRGB_m((int)this);
  return 1;
}

/* ---- cand_CFxPrimType13__helper  0x00491510 ----  VERIFIED */
void __cdecl cand_CFxPrimType13__helper(float *a1)
{
  double v1;
  double v2;
  double v3;
  float v4[3]; /* [esp+4h] [ebp-Ch] BYREF -- retail vec3_t */

  v4[0] = a1[1] - flt_140751C;
  v4[1] = a1[2] - flt_1407520;
  v4[2] = a1[3] - flt_1407524;
  v1 = VectorNormalize(v4);
  v2 = flt_1407530 * v4[2] + flt_140752C * v4[1] + flt_1407528 * v4[0];
  if ( v1 > 600.0 )
    goto LABEL_7;
  if ( !((v2 < 0.5) | __UNORDERED__(v2, 0.5)) )
    goto LABEL_8;
  if ( v1 > 100.0 )
  {
LABEL_7:
    v2 = 0.0;
    goto LABEL_8;
  }
  if ( (v2 < 0.5) | __UNORDERED__(v2, 0.5) )
  {
    if ( v1 <= 100.0 )
      v2 = v2 + 1.1;
  }
LABEL_8:
  v3 = v2 * (1.0 - v1 * v1 * 0.0000027777778);
  a1[58] = v3 * a1[58];
  a1[59] = v3 * a1[59];
  a1[60] = v3 * a1[60];
  a1[61] = v3 * a1[61];
  a1[62] = v3 * a1[62];
  a1[63] = v3 * a1[63];
}

/* ---- cand_CFxPrimType13__Draw  0x00491630 ----  VERIFIED */
int __cdecl cand_CFxPrimType13__Draw(int this)
{
  int v2;
  float *v3;
  float *v4;
  int v5;
  double v6;
  unsigned __int64 v7; // rax
  double v8;
  double v9;

  v2 = this + 60;
  v3 = (float *)(this + 72);
  *(_DWORD *)(this + 60) = 4;
  v4 = (float *)(this + 72);
  v5 = 3;
  do
  {
    if ( *v4 <= 1.0 )
    {
      if ( (*v4 < 0.0) | __UNORDERED__(*v4, 0.0) )
        *v4 = 0.0;
    }
    else
    {
      *v4 = 1.0;
    }
    ++v4;
    --v5;
  }
  while ( v5 );
  v6 = *(float *)(this + 76) * 255.0;
  *(_BYTE *)(this + 168) = (unsigned __int64)(*v3 * 255.0);
  v7 = (unsigned __int64)v6;
  v8 = *(float *)(this + 80) * 255.0;
  *(_BYTE *)(this + 169) = v7;
  *(_BYTE *)(this + 170) = (unsigned __int64)v8;
  *(_BYTE *)(this + 171) = -1;
  *(float *)(this + 128) = flt_140751C;
  *(float *)(this + 132) = flt_1407520;
  *(float *)(this + 136) = flt_1407524;
  *(float *)(this + 128) = flt_1407528 * 8.0 + *(float *)(this + 128);
  *(float *)(this + 132) = flt_140752C * 8.0 + *(float *)(this + 132);
  v9 = flt_1407530;
  *(_DWORD *)(this + 184) = 1094713344;
  *(_DWORD *)(this + 160) = 1094713344;
  *(float *)(this + 136) = v9 * 8.0 + *(float *)(this + 136);
  return dword_14328A0(v2, 0);
}

/* ---- cand_CFxPrimType13__TypeID  0x00491750 ----  [HIGH] */
char cand_CFxPrimType13__TypeID()
{
  return 13;
}

/* ---- cand_CFxPrimType13__Archive  0x00491760 ----  VERIFIED */
char __cdecl cand_CFxPrimType13__Archive(_BYTE *this, int a2)
{
  return CLight__Archive(this, a2);
}

/* ---- sub_491770  0x00491770 ----  [HIGH] */
int __cdecl sub_491770(int result, int a2)
{
  *(_DWORD *)(result + 16) = a2;
  *(float *)(result + 180) = (double)a2 * 0.001;
  return result;
}

/* ---- sub_491790  0x00491790 ----  VERIFIED */
_QWORD *__cdecl sub_491790(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(481.0);
  return a1;
}

/* ---- sub_4917B0  0x004917B0 ----  VERIFIED */
int *__cdecl sub_4917B0( int *pool, int elemSize )
{
  int  *head;
  int  *block;
  int  *node;
  int   count;

  head  = (int *)pool[1];
  block = head;
  if ( head )
  {
    while ( !block[8189] )                  /* +0x7FF4 freeList */
    {
      block = (int *)block[8190];           /* +0x7FF8 prev     */
      if ( !block )
        goto claim;
    }
    if ( block != head )
      sub_491F90( (int)block, (int)pool );
    goto take;
  }

claim:
  block = (int *)FxMem_ClaimBlock_m( pool[0], elemSize, (int)head );
  if ( !block )
    return block;
  pool[1] = (int)block;

take:
  node        = (int *)block[8189];
  count       = block[8188] - 1;            /* +0x7FF0 freeCount */
  block[8189] = *node;
  block[8188] = count;
  Com_Memset( node, 0, 0x44u );
  return node;
}

/* ---- sub_4918B0  0x004918B0 ----  VERIFIED */
_QWORD *__cdecl sub_4918B0(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(209.0);
  return a1;
}

/* ---- sub_4918D0  0x004918D0 ----  VERIFIED */
int *__cdecl sub_4918D0( int *pool, int elemSize )
{
  int  *head;
  int  *block;
  int  *node;
  int   count;

  head  = (int *)pool[1];
  block = head;
  if ( head )
  {
    while ( !block[8189] )
    {
      block = (int *)block[8190];
      if ( !block )
        goto claim;
    }
    if ( block != head )
      sub_491FF0( (int)block, (int)pool );
    goto take;
  }

claim:
  block = (int *)FxMem_ClaimBlock_m( pool[0], elemSize, (int)head );
  if ( !block )
    return block;
  pool[1] = (int)block;

take:
  node        = (int *)block[8189];
  count       = block[8188] - 1;
  block[8189] = *node;
  block[8188] = count;
  Com_Memset( node, 0, 0x9Cu );
  return node;
}

/* ---- sub_491960  0x00491960 ----  VERIFIED */
_DWORD *__fastcall sub_491960( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_491FF0( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_4919D0  0x004919D0 ----  VERIFIED */
_QWORD *__cdecl sub_4919D0(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(148.0);
  return a1;
}

/* ---- sub_4919F0  0x004919F0 ----  VERIFIED */
_DWORD *__fastcall sub_4919F0( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( cur[8189] )
  {
    if ( count < pool[0] )
      return block;
    if ( block == cur )
    {
      nxt = (int)block[8190];
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  else
  {
    nxt = (int)block[8191];
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8190] = 0;
    block[8191] = 0;
    block[8190] = pool[1];
    *(_DWORD *)( pool[1] + FX_HDR_NEXT ) = (_DWORD)block;
    pool[1] = (int)block;
  }
  return block;
}

/* ---- sub_491A60  0x00491A60 ----  VERIFIED */
_QWORD *__cdecl sub_491A60(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(58.0);
  return a1;
}

/* ---- sub_491A80  0x00491A80 ----  VERIFIED */
_QWORD *__cdecl sub_491A80(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(125.0);
  return a1;
}

/* ---- sub_491AA0  0x00491AA0 ----  VERIFIED */
_DWORD *__fastcall sub_491AA0( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_4920B0( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491B10  0x00491B10 ----  VERIFIED */
_QWORD *__cdecl sub_491B10(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(103.0);
  return a1;
}

/* ---- sub_491B30  0x00491B30 ----  VERIFIED */
_DWORD *__fastcall sub_491B30( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_492110( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491BA0  0x00491BA0 ----  VERIFIED */
_QWORD *__cdecl sub_491BA0(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(99.0);
  return a1;
}

/* ---- sub_491BC0  0x00491BC0 ----  VERIFIED */
_DWORD *__fastcall sub_491BC0( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_492170( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491C30  0x00491C30 ----  VERIFIED */
_QWORD *__cdecl sub_491C30(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(45.0);
  return a1;
}

/* ---- sub_491C50  0x00491C50 ----  VERIFIED */
_DWORD *__fastcall sub_491C50( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_4921D0( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491CC0  0x00491CC0 ----  VERIFIED */
_QWORD *__cdecl sub_491CC0(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(99.0);
  return a1;
}

/* ---- sub_491CE0  0x00491CE0 ----  VERIFIED */
_DWORD *__fastcall sub_491CE0( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_492230( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491D50  0x00491D50 ----  VERIFIED */
_QWORD *__cdecl sub_491D50(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(95.0);
  return a1;
}

/* ---- sub_491D70  0x00491D70 ----  VERIFIED */
_DWORD *__fastcall sub_491D70( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_492290( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491DE0  0x00491DE0 ----  VERIFIED */
_QWORD *__cdecl sub_491DE0(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(95.0);
  return a1;
}

/* ---- sub_491E00  0x00491E00 ----  VERIFIED */
_DWORD *__fastcall sub_491E00( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_4922F0( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491E70  0x00491E70 ----  VERIFIED */
_QWORD *__cdecl sub_491E70(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(82.0);
  return a1;
}

/* ---- sub_491E90  0x00491E90 ----  VERIFIED */
_DWORD *__fastcall sub_491E90( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_492350( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491F00  0x00491F00 ----  VERIFIED */
_QWORD *__cdecl sub_491F00(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(75.0);
  return a1;
}

/* ---- sub_491F20  0x00491F20 ----  VERIFIED */
_DWORD *__fastcall sub_491F20( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_4923B0( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_491F90  0x00491F90 ----  VERIFIED */
int __cdecl sub_491F90(int result, int a2)
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

/* ---- sub_491FF0  0x00491FF0 ----  VERIFIED */
int __cdecl sub_491FF0(int result, int a2)
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

/* ---- sub_4920B0  0x004920B0 ----  VERIFIED */
int __cdecl sub_4920B0(int result, int a2)
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

/* ---- sub_492110  0x00492110 ----  VERIFIED */
int __cdecl sub_492110(int result, int a2)
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

/* ---- sub_492170  0x00492170 ----  VERIFIED */
int __cdecl sub_492170(int result, int a2)
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

/* ---- sub_4921D0  0x004921D0 ----  VERIFIED */
int __cdecl sub_4921D0(int result, int a2)
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

/* ---- sub_492230  0x00492230 ----  VERIFIED */
int __cdecl sub_492230(int result, int a2)
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

/* ---- sub_492290  0x00492290 ----  VERIFIED */
int __cdecl sub_492290(int result, int a2)
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

/* ---- sub_4922F0  0x004922F0 ----  VERIFIED */
int __cdecl sub_4922F0(int result, int a2)
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

/* ---- sub_492350  0x00492350 ----  VERIFIED */
int __cdecl sub_492350(int result, int a2)
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

/* ---- sub_4923B0  0x004923B0 ----  VERIFIED */
int __cdecl sub_4923B0(int result, int a2)
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

/* ---- sub_492410  0x00492410 ----  VERIFIED */
int __cdecl sub_492410(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_492420  0x00492420 ----  [HIGH] */
int __cdecl sub_492420(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

/* ---- sub_492430  0x00492430 ----  [HIGH] */
int __cdecl sub_492430(int a1)
{
  return *(_DWORD *)(a1 + 28);
}

/* ---- nullsub_36  0x00492440 ----  [HIGH] */
void nullsub_36()
{
  ;
}

/* ---- sub_492450  0x00492450 ----  [HIGH] */
_BYTE *__cdecl sub_492450(_BYTE *a1, _BYTE *a2)
{
  _BYTE *result;

  result = a2;
  *a1 = *a2;
  return result;
}

/* ---- sub_492460  0x00492460 ----  VERIFIED */
int __cdecl sub_492460(const char *a1, const char *a2, int a3)
{
  return memcmp(a2, a1, a3);
}

/* ---- sub_492480  0x00492480 ----  VERIFIED */
unsigned int __cdecl sub_492480(const char *a1)
{
  return strlen(a1);
}

/* ---- sub_4924A0  0x004924A0 ----  VERIFIED */
void *__cdecl sub_4924A0(void *a1, const void *a2, unsigned int a3)
{
  qmemcpy(a1, a2, a3);
  return a1;
}

/* ---- j__memcpy  0x004924D0 ----  VERIFIED */
void *__cdecl j__memcpy(void *a1, const void *Src, size_t Size)
{
  return memcpy(a1, Src, Size);
}

/* ---- nullsub_37  0x004924E0 ----  [HIGH] */
void nullsub_37()
{
  ;
}

/* ---- nullsub_38  0x004924F0 ----  [HIGH] */
void nullsub_38()
{
  ;
}

/* ---- sub_492500  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_492500() { return 0; }
#if 0
int __cdecl sub_492500(int this, void ***a2)
{
  exception__exception_void_((_DWORD *)this);
  *(_DWORD *)this = &std::logic_error::`vftable';
  *(_DWORD *)(this + 32) = 0;
  *(_DWORD *)(this + 36) = 15;
  *(_BYTE *)(this + 16) = 0;
  sub_497C10((void ***)(this + 12), a2, 0, (void **)0xFFFFFFFF);
  return this;
}
#endif

/* ---- sub_492560  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_492560() { return 0; }
#if 0
void __cdecl sub_492560(void **this)
{
  *this = &std::logic_error::`vftable';
  if ( (unsigned int)this[9] >= 0x10 )
    j__free(this[4]);
  this[9] = (void *)15;
  this[8] = 0;
  *((_BYTE *)this + 16) = 0;
  exception___exception_void_(this);
}
#endif

/* ---- unknown_libname_2  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int unknown_libname_2() { return 0; }
#if 0
// Microsoft VisualC 2-14/net runtime
_DWORD *__cdecl unknown_libname_2(_DWORD *this)
{
  if ( this[9] < 0x10u )
    return this + 4;
  else
    return (_DWORD *)this[4];
}
#endif

/* ---- sub_4925B0  0x004925B0 ----  VERIFIED */
void **__cdecl sub_4925B0(void **Block, char a2)
{
  sub_492560(Block);
  if ( (a2 & 1) != 0 )
    j__free(Block);
  return Block;
}

/* ---- sub_4925D0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4925D0() { return 0; }
#if 0
_DWORD *__cdecl sub_4925D0(_DWORD *this, void ***a2)
{
  sub_492500((int)this, a2);
  *this = &std::length_error::`vftable';
  return this;
}
#endif

/* ---- sub_4925F0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4925F0() { return 0; }
#if 0
void __cdecl sub_4925F0(void **this)
{
  *this = &std::logic_error::`vftable';
  if ( (unsigned int)this[9] >= 0x10 )
    j__free(this[4]);
  this[9] = (void *)15;
  this[8] = 0;
  *((_BYTE *)this + 16) = 0;
  exception___exception_void_(this);
}
#endif

/* ---- sub_492630  0x00492630 ----  VERIFIED */
void **__cdecl sub_492630(void **Block, char a2)
{
  sub_4925F0(Block);
  if ( (a2 & 1) != 0 )
    j__free(Block);
  return Block;
}

/* ---- sub_492650  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_492650() { return 0; }
#if 0
_DWORD *__cdecl sub_492650(_DWORD *this, void ***a2)
{
  sub_492500((int)this, a2);
  *this = &std::out_of_range::`vftable';
  return this;
}
#endif

/* ---- sub_492670  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_492670() { return 0; }
#if 0
void __cdecl sub_492670(void **this)
{
  *this = &std::logic_error::`vftable';
  if ( (unsigned int)this[9] >= 0x10 )
    j__free(this[4]);
  this[9] = (void *)15;
  this[8] = 0;
  *((_BYTE *)this + 16) = 0;
  exception___exception_void_(this);
}
#endif

/* ---- sub_4926B0  0x004926B0 ----  VERIFIED */
void **__cdecl sub_4926B0(void **Block, char a2)
{
  sub_492670(Block);
  if ( (a2 & 1) != 0 )
    j__free(Block);
  return Block;
}

/* ---- sub_4926D0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4926D0() { return 0; }
#if 0
_DWORD *__cdecl sub_4926D0(_DWORD *this, void **a2)
{
  sub_4926F0((int)this, a2);
  *this = &std::length_error::`vftable';
  return this;
}
#endif

/* ---- sub_4926F0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4926F0() { return 0; }
#if 0
int __cdecl sub_4926F0(int this, void ***a2)
{
  exception::exception_exception((exception *)this, (const struct exception *)a2);
  *(_DWORD *)this = &std::logic_error::`vftable';
  *(_DWORD *)(this + 36) = 15;
  *(_DWORD *)(this + 32) = 0;
  *(_BYTE *)(this + 16) = 0;
  sub_497C10((void ***)(this + 12), a2 + 3, 0, (void **)0xFFFFFFFF);
  return this;
}
#endif

/* ---- sub_492760  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_492760() { return 0; }
#if 0
_DWORD *__cdecl sub_492760(_DWORD *this, void ***a2)
{
  sub_4926F0((int)this, a2);
  *this = &std::out_of_range::`vftable';
  return this;
}
#endif

/* ---- sub_492780  0x00492780 ----  VERIFIED */
int __cdecl sub_492780(int a1)
{
  int v1;
  int v2;

  v1 = *(_DWORD *)(a1 + 4);
  if ( !v1 )
    return 0;
  v2 = (*(_DWORD *)(a1 + 8) - v1) >> 2;
  if ( !v2 )
    return 0;
  com_randSeed_m = 214013 * com_randSeed_m + 2531011;
  return *(_DWORD *)(v1 + 4 * ((int)(v2 * ((unsigned int)com_randSeed_m >> 17)) >> 15));
}

/* ---- sub_4927C0  0x004927C0 ----  [HIGH] */
double __cdecl sub_4927C0(int a1)
{
  return *(float *)(a1 + 4);
}

/* ---- sub_4927D0  0x004927D0 ----  [HIGH] */
double __cdecl sub_4927D0(int a1)
{
  return *(float *)a1;
}

/* ---- sub_4927E0  0x004927E0 ----  [HIGH] */
double __cdecl sub_4927E0(float *a1, float a2)
{
  return (a1[1] - *a1) * a2 + *a1;
}

/* ---- sub_4927F0  0x004927F0 ----  VERIFIED */
double __cdecl sub_4927F0(float *this)
{
  if ( (*this == this[1]) | __UNORDERED__(*this, this[1]) )
    return *this;
  else
    return Com_RandFloatRange_m(*this, this[1]);
}

/* ---- sub_492810  0x00492810 ----  VERIFIED */
unsigned __int64 __cdecl sub_492810(float *this)
{
  if ( (*this == this[1]) | __UNORDERED__(*this, this[1]) )
    return (unsigned __int64)*this;
  else
    return (unsigned __int64)(Com_RandFloatRange_m(*this, this[1]) + 0.5);
}

/* ---- CPrimitiveTemplate__dtor  0x00492840 ----  VERIFIED */
void __cdecl CPrimitiveTemplate__dtor(int a1)
{
  if ( *(_DWORD *)(a1 + 172) )
    j__free(*(void **)(a1 + 172));
  *(_DWORD *)(a1 + 172) = 0;
  *(_DWORD *)(a1 + 176) = 0;
  *(_DWORD *)(a1 + 180) = 0;
  if ( *(_DWORD *)(a1 + 156) )
    j__free(*(void **)(a1 + 156));
  *(_DWORD *)(a1 + 156) = 0;
  *(_DWORD *)(a1 + 160) = 0;
  *(_DWORD *)(a1 + 164) = 0;
  if ( *(_DWORD *)(a1 + 140) )
    j__free(*(void **)(a1 + 140));
  *(_DWORD *)(a1 + 140) = 0;
  *(_DWORD *)(a1 + 144) = 0;
  *(_DWORD *)(a1 + 148) = 0;
  if ( *(_DWORD *)(a1 + 124) )
    j__free(*(void **)(a1 + 124));
  *(_DWORD *)(a1 + 124) = 0;
  *(_DWORD *)(a1 + 128) = 0;
  *(_DWORD *)(a1 + 132) = 0;
  if ( *(_DWORD *)(a1 + 108) )
    j__free(*(void **)(a1 + 108));
  *(_DWORD *)(a1 + 108) = 0;
  *(_DWORD *)(a1 + 112) = 0;
  *(_DWORD *)(a1 + 116) = 0;
}

/* ---- sub_4928F0  0x004928F0 ----  VERIFIED */
int *__cdecl sub_4928F0(int a1)
{
  return sub_497640(&dword_140758C, a1);
}

/* ---- sub_492900  0x00492900 ----  VERIFIED */
_DWORD *__cdecl sub_492900(char *this)
{
  return sub_4976D0(this, &dword_140758C);
}

/* ---- sub_492940  0x00492940 ----  [HIGH] */
BOOL __cdecl sub_492940(_DWORD *this, int a2)
{
  return this[2] <= a2;
}

/* ---- sub_492960  0x00492960 ----  VERIFIED */
int *__cdecl sub_492960(int a1)
{
  return FxMem_AllocScheduledEffect(&dword_1407594, a1);
}

/* ---- sub_492970  0x00492970 ----  VERIFIED */
_DWORD *__cdecl sub_492970(char *this)
{
  return sub_4977F0(this, &dword_1407594);
}

/* ---- sub_492980  0x00492980 ----  VERIFIED */
void __cdecl sub_492980(int a1)
{
  _DWORD *v2;
  _DWORD *v3;
  int **v4;
  int v5;
  int v6; // [esp+Ch] [ebp-4h] BYREF

  v2 = *(_DWORD **)(a1 + 21520);
  if ( v2 )
    v3 = (_DWORD *)*v2;
  else
    v3 = 0;
  sub_498D60(a1 + 21516, &v6, v3, v2);
  j__free(*(void **)(a1 + 21520));
  *(_DWORD *)(a1 + 21520) = 0;
  *(_DWORD *)(a1 + 21524) = 0;
  v4 = *(int ***)(a1 + 21508);
  v5 = a1 + 21504;
  sub_4989B0(v5, (int **)&v6, *v4, (int *)v4);
  j__free(*(void **)(v5 + 4));
  *(_DWORD *)(v5 + 4) = 0;
  *(_DWORD *)(v5 + 8) = 0;
}

/* ---- sub_492A20  0x00492A20 ----  [HIGH] */
float *__cdecl sub_492A20(float *result, float *a2, float a3, float a4, float a5)
{
  *a2 = a4 * result[3] + a5 * result[6] + a3 * *result;
  a2[1] = a3 * result[1] + a4 * result[4] + a5 * result[7];
  a2[2] = a3 * result[2] + a4 * result[5] + a5 * result[8];
  return result;
}

/* ---- sub_492A80  0x00492A80 ----  VERIFIED */
void __cdecl sub_492A80(int a1, int a2)
{
  unsigned int v2;
  int v3;
  _DWORD *v4;
  int v5;
  unsigned int v6;
  _DWORD *v7;

  v2 = 0;
  if ( *(_DWORD *)(a1 + 4) )
    j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  *(_DWORD *)(a1 + 12) = 0;
  while ( 1 )
  {
    v3 = *(_DWORD *)(a2 + 4);
    if ( !v3 || v2 >= (*(_DWORD *)(a2 + 8) - v3) >> 2 )
      break;
    v4 = (_DWORD *)(v3 + 4 * v2);
    v5 = *(_DWORD *)(a1 + 4);
    if ( v5 )
      v6 = (*(_DWORD *)(a1 + 8) - v5) >> 2;
    else
      v6 = 0;
    if ( v5 && v6 < (*(_DWORD *)(a1 + 12) - v5) >> 2 )
    {
      v7 = *(_DWORD **)(a1 + 8);
      sub_499B40(v7, v4, 1);
      *(_DWORD *)(a1 + 8) = v7 + 1;
      ++v2;
    }
    else
    {
      sub_498690(a1, *(_DWORD **)(a1 + 8), 1u, v4);
      ++v2;
    }
  }
}

/* ---- CFxScheduler__ctor_real  0x00492B20 ----  VERIFIED */
int __stdcall CFxScheduler__ctor_real(int a1)
{
  int v1;
  _DWORD *v2;

  v1 = a1 + 21504;
  *(_BYTE *)(a1 + 21504) = a1;
  v2 = sub_499550(a1 + 21504);
  *(_DWORD *)(a1 + 21508) = v2;
  *((_BYTE *)v2 + 45) = 1;
  *(_DWORD *)(*(_DWORD *)(v1 + 4) + 4) = *(_DWORD *)(v1 + 4);
  **(_DWORD **)(v1 + 4) = *(_DWORD *)(v1 + 4);
  *(_DWORD *)(*(_DWORD *)(v1 + 4) + 8) = *(_DWORD *)(v1 + 4);
  *(_DWORD *)(a1 + 21512) = 0;
  *(_DWORD *)(a1 + 21520) = sub_498310(a1 + 21516);
  *(_DWORD *)(a1 + 21524) = 0;
  memset((void *)a1, 0, 0x5400u);
  return a1;
}

/* ---- sub_492BB0  0x00492BB0 ----  VERIFIED */
int __cdecl sub_492BB0(char *this, int a2)
{
  char *v3;
  int v4;
  char v5;
  int result;
  int v7;
  char *v8;
  char **v9;
  int *v10;
  char *v11;
  char *i;

  v3 = this + 2;
  *(_BYTE *)(a2 + 1) = 1;
  v4 = a2 - (_DWORD)this;
  do
  {
    v5 = *v3;
    v3[v4] = *v3;
    ++v3;
  }
  while ( v5 );
  result = *((_DWORD *)this + 17);
  v7 = 0;
  *(_DWORD *)(a2 + 68) = result;
  if ( result > 0 )
  {
    v8 = &this[-a2];
    v9 = (char **)(a2 + 72);
    for ( i = v8; ; v8 = i )
    {
      v10 = sub_497640(&dword_140758C, 604);
      v11 = v10 ? (char *)CPrimitiveTemplate__ctor((int)v10) : 0;
      *v9 = v11;
      CPrimitiveTemplate__copy_m(v11, *(char **)((char *)v9 + (_DWORD)v8));
      **v9 = 1;
      result = *(_DWORD *)(a2 + 68);
      ++v7;
      ++v9;
      if ( v7 >= result )
        break;
    }
  }
  return result;
}

/* ---- CFxScheduler__Clean  0x00492C80 ----  [CONFIRMED] */
void __stdcall CFxScheduler__Clean(int a1, int a2, int a3)
{
  int v3;
  void **v4;
  int v5;
  char **v6;
  char *v7;
  int v8;
  _DWORD *v9;
  char **v10;
  char *v11;
  _DWORD *v12;
  int *v13;
  int v14;
  bool v15; // zf
  _DWORD *v16;
  _DWORD *v17;
  void *v18; // [esp+10h] [ebp-2Ch] BYREF
  void *Block;
  int v20;
  unsigned int v21;
  unsigned int v22;
  int v23;
  unsigned int retaddr;

  v3 = a1;
  v22 = retaddr ^ _security_cookie;
  v4 = *(void ***)(a1 + 21520);
  v5 = 0;
  if ( v4 )
    v6 = (char **)*v4;
  else
    v6 = 0;
  if ( v6 != *(char ***)(a1 + 21520) )
  {
    do
    {
      v7 = *v6;
      sub_4977F0(v6[2], &dword_1407594);
      if ( v6 != *(char ***)(a1 + 21520) )
      {
        *(_DWORD *)v6[1] = *v6;
        *((_DWORD *)*v6 + 1) = v6[1];
        j__free(v6);
        --*(_DWORD *)(a1 + 21524);
      }
      v6 = (char **)v7;
    }
    while ( v7 != *(char **)(a1 + 21520) );
  }
  if ( (_BYTE)a2 )
  {
    v8 = 1;
    a2 = 1;
    v9 = (_DWORD *)(a1 + 236);
    do
    {
      if ( v8 != a3 )
      {
        if ( *((_BYTE *)v9 - 68) )
        {
          if ( (int)*v9 > 0 )
          {
            v10 = (char **)(v9 + 1);
            do
            {
              v11 = *v10;
              if ( *v10 )
              {
                CPrimitiveTemplate__dtor((int)v11);
                sub_4976D0(v11, &dword_140758C);
              }
              ++v5;
              ++v10;
            }
            while ( v5 < *v9 );
            v3 = a1;
            v8 = a2;
            v5 = 0;
          }
        }
        *((_BYTE *)v9 - 68) = 0;
      }
      ++v8;
      v9 += 42;
      a2 = v8;
    }
    while ( v8 < 128 );
    if ( a3 )
    {
      v21 = 15;
      v20 = 0;
      LOBYTE(Block) = 0;
      v13 = *(int **)(v3 + 21508);
      v14 = *v13;
      v15 = *v13 == (_DWORD)v13;
      v23 = 0;
      a2 = v14;
      if ( !v15 )
      {
        while ( *(_DWORD *)(v14 + 40) != a3 )
        {
          sub_498230(a3, (int **)&a2);
          v14 = a2;
          if ( (int *)a2 == v13 )
            goto LABEL_27;
        }
        sub_497C10((void ***)&v18, (void ***)(v14 + 12), 0, (void **)0xFFFFFFFF);
      }
LABEL_27:
      sub_498100(*(void ***)(*(_DWORD *)(v3 + 21508) + 4));
      *(_DWORD *)(*(_DWORD *)(v3 + 21508) + 4) = *(_DWORD *)(v3 + 21508);
      v16 = *(_DWORD **)(v3 + 21508);
      *(_DWORD *)(v3 + 21512) = 0;
      *v16 = v16;
      *(_DWORD *)(*(_DWORD *)(v3 + 21508) + 8) = *(_DWORD *)(v3 + 21508);
      v17 = (_DWORD *)sub_497890(v3 + 21504, &v18);
      *v17 = a3;
      if ( v21 >= 0x10 )
        j__free(Block);
    }
    else
    {
      sub_498100(*(void ***)(*(_DWORD *)(v3 + 21508) + 4));
      *(_DWORD *)(*(_DWORD *)(v3 + 21508) + 4) = *(_DWORD *)(v3 + 21508);
      v12 = *(_DWORD **)(v3 + 21508);
      *(_DWORD *)(v3 + 21512) = 0;
      *v12 = v12;
      *(_DWORD *)(*(_DWORD *)(v3 + 21508) + 8) = *(_DWORD *)(v3 + 21508);
    }
  }
}

/* ---- CPrimitiveTemplate__scalar_deleting_dtor  0x00492E80 ----  [HIGH] */
char *__cdecl CPrimitiveTemplate__scalar_deleting_dtor(char *a1, char a2)
{
  CPrimitiveTemplate__dtor((int)a1);
  if ( (a2 & 1) != 0 )
    sub_4976D0(a1, &dword_140758C);
  return a1;
}

/* ---- CFxScheduler__RegisterEffect  0x00492EB0 ----  VERIFIED */
int __cdecl CFxScheduler__RegisterEffect( _DWORD *this, char *Source, char isPath )
{
  char          *src;
  char          *dst;
  char          *file;
  int            len;
  int            id;
  fileHandle_t   f;
  unsigned char *node;
  unsigned char  key[28];
  int            it;
  char           Destination[64];
  char           Buffer[64];
  char          *dataPtr;
  int            gp2[14];
  _BYTE          buffer[65536];

  if ( isPath )
  {
    char *scan;
    src = Source;
    for ( scan = Source; *scan; ++scan )
    {
      if ( *scan == '/' || *scan == '\\' )
        src = scan + 1;
    }
  }
  else
  {
    src = Source;
  }

  dst = Destination;
  while ( *src && *src != '.' )
    *dst++ = *src++;
  *dst = 0;

  if ( !isPath )
    strlwr( Destination );

  FXS_RES ( key ) = 15;
  FXS_SIZE( key ) = 0;
  key[4]          = 0;
  sub_498410( (int)key, Destination, strlen( Destination ) );

  sub_4979A0( &it, (int)( (char *)this + 0x5400 ), (int)key );
  node = (unsigned char *)it;

  if ( FXS_RES( key ) >= 0x10 )
    free( *(void **)FXS_BX( key ) );
  FXS_RES ( key ) = 15;
  FXS_SIZE( key ) = 0;
  key[4]          = 0;

  if ( node != *(unsigned char **)( (char *)this + 0x5404 ) )
    return *(int *)( node + 40 );

  gp2[0] = (int)"Top Level";
  memset( &gp2[1], 0, 40 );
  *( (char *)&gp2[11] ) = 0;
  gp2[12] = 0;
  *( (char *)&gp2[13] ) = 0;
  dataPtr = 0;

  if ( isPath )
  {
    file = Source;
  }
  else
  {
    sprintf( Buffer, "%s.efx", Destination );
    file = Buffer;
  }

  len = FS_FOpenFileByMode( file, &f, FS_READ );
  if ( len <= 0 )
  {
    SFxHelper__Print( (int)&theFxHelper,
                      "Effect file load failed: %s: file not found\n", file );
    CGenericParser2__Clean( (int)gp2 );
    return 0;
  }
  if ( (unsigned int)len >= 0xFFFF )
  {
    SFxHelper__Print( (int)&theFxHelper,
                      "Effect file load failed: %s: file too large\n", file );
    FS_FCloseFile( f );
    CGenericParser2__Clean( (int)gp2 );
    return 0;
  }

  FS_Read( buffer, len, f );
  buffer[len] = 0;
  dataPtr = (char *)buffer;
  CGenericParser2__Parse( (int)gp2, &dataPtr, 1, 0 );
  FS_FCloseFile( f );

  id = CFxScheduler__ParseEffect( Destination, (int)this, gp2 );
  CGenericParser2__Clean( (int)gp2 );
  return id;
}

/* ---- CFxScheduler__ParseEffect  0x004931B0 ----  [CONFIRMED] */
int __cdecl CFxScheduler__ParseEffect(char *this, int a2, int *a3)
{
  char *v3;
  int v4;
  const char *v5;
  int v6;
  int *v7;
  int v8;
  int v9;

  v3 = CFxScheduler__GetNewEffectTemplate(&a2, this, a2);
  if ( !a2 || !v3 )
    return 0;
  v4 = a3[7];
  if ( v4 )
  {
    while ( 1 )
    {
      v5 = *(const char **)v4;
      if ( !_stricmp(*(const char **)v4, "particle") )
        break;
      if ( !_stricmp(v5, "line") )
      {
        v6 = 2;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "tail") )
      {
        v6 = 3;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "sound") )
      {
        v6 = 6;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "cylinder") )
      {
        v6 = 4;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "electricity") )
      {
        v6 = 9;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "emitter") )
      {
        v6 = 5;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "decal") )
      {
        v6 = 7;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "orientedparticle") )
      {
        v6 = 8;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "fxrunner") )
      {
        v6 = 10;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "light") )
      {
        v6 = 11;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "cameraShake") )
      {
        v6 = 12;
        goto LABEL_30;
      }
      if ( !_stricmp(v5, "flash") )
      {
        v6 = 13;
        goto LABEL_30;
      }
LABEL_36:
      v4 = *(_DWORD *)(v4 + 4);
      if ( !v4 )
        return a2;
    }
    v6 = 1;
LABEL_30:
    v7 = sub_497640(&dword_140758C, 604);
    a3 = v7;
    if ( v7 )
      v8 = CPrimitiveTemplate__ctor((int)v7);
    else
      v8 = 0;
    *(_DWORD *)(v8 + 72) = v6;
    CPrimitiveTemplate__ParsePrimitive(v8, v4);
    v9 = *((_DWORD *)v3 + 17);
    if ( v9 < 24 )
    {
      *(_DWORD *)&v3[4 * v9 + 72] = v8;
      ++*((_DWORD *)v3 + 17);
    }
    else
    {
      SFxHelper__Print((int)&theFxHelper, "FxScheduler:  Error--too many primitives in an effect\n");
    }
    goto LABEL_36;
  }
  return a2;
}

/* ---- CFxScheduler__AddPrimitiveToEffect  0x004933F0 ----  [HIGH] */
void __cdecl CFxScheduler__AddPrimitiveToEffect(int a1, int a2, int a3)
{
  int v3;

  v3 = *(_DWORD *)(a1 + 68);
  if ( v3 < 24 )
  {
    *(_DWORD *)(a1 + 4 * v3 + 72) = a3;
    ++*(_DWORD *)(a1 + 68);
  }
  else
  {
    SFxHelper__Print((int)&theFxHelper, "FxScheduler:  Error--too many primitives in an effect\n");
  }
}

/* ---- CFxScheduler__GetNewEffectTemplate  0x00493420 ----  VERIFIED */
char *__cdecl CFxScheduler__GetNewEffectTemplate( int *outId, char *name, int scheduler )
{
  int            i;
  char          *t;
  unsigned char  key[28];

  i = 1;
  for ( t = (char *)( scheduler + FX_EFFECT_TEMPLATE_SIZE );
        *t;
        t += FX_EFFECT_TEMPLATE_SIZE )
  {
    if ( ++i >= FX_EFFECT_TEMPLATE_COUNT )
    {
      SFxHelper__Print( (int)&theFxHelper,
                        "FxScheduler:  Error--reached max effects\n" );
      *outId = 0;
      return 0;
    }
  }

  *outId = i;
  memset( t, 0, FX_EFFECT_TEMPLATE_SIZE );

  if ( name )
  {
    FXS_RES ( key ) = 15;
    FXS_SIZE( key ) = 0;
    key[4]          = 0;
    sub_498410( (int)key, name, strlen( name ) );

    *(int *)sub_497890( scheduler + FX_SCHEDULER_EFFECT_IDS_MAP_OFFSET, key ) = i;

    if ( FXS_RES( key ) >= 0x10 )
      free( *(void **)FXS_BX( key ) );

    strcpy( t + 2, name );
  }

  *t = 1;
  return t;
}

/* ---- sub_493540  0x00493540 ----  VERIFIED */
char *__fastcall sub_493540(int a1, char *a2, int *a3)
{
  int *v4;
  char *v5;
  void *v7; // [esp+8h] [ebp-28h] BYREF
  void *Block;
  int v9;
  unsigned int v10;
  int v11;

  v10 = 15;
  v9 = 0;
  LOBYTE(Block) = 0;
  sub_498410((int)&v7, a2, (void **)strlen(a2));
  v11 = 0;
  v4 = (int *)sub_497890(a1 + 21504, &v7);
  v5 = CFxScheduler__CopyEffect_m(*v4, a1, a3);
  if ( v10 >= 0x10 )
    j__free(Block);
  return v5;
}

/* ---- CFxScheduler__CopyEffect_m  0x004935E0 ----  VERIFIED */
char *__cdecl CFxScheduler__CopyEffect_m( int effectId, int scheduler, int *outId )
{
  char *src;
  char *dst;

  if ( effectId >= FX_EFFECT_ID_FIRST && effectId < FX_EFFECT_TEMPLATE_COUNT
       && ( src = (char *)( FX_EFFECT_TEMPLATE_SIZE * effectId + scheduler ), *src ) )
  {
    dst = CFxScheduler__GetNewEffectTemplate( outId, 0, scheduler );
    if ( dst && *outId )
    {
      sub_492BB0( src, (int)dst );
      dst[1] = 1;
      return dst;
    }
  }
  else
  {
    SFxHelper__Print( (int)&theFxHelper,
                      "FxScheduler: Bad effect file copy request\n" );
  }

  *outId = 0;
  return 0;
}

/* ---- sub_493650  0x00493650 ----  VERIFIED */
int __cdecl sub_493650(int a1, int a2, char *String2)
{
  int v3;
  int i;

  if ( !a1 )
    return 0;
  if ( !*(_BYTE *)a1 )
    return 0;
  v3 = 0;
  if ( *(int *)(a1 + 68) <= 0 )
    return 0;
  for ( i = a1 + 72; _stricmp((const char *)(*(_DWORD *)i + 8), String2); i += 4 )
  {
    if ( ++v3 >= *(_DWORD *)(a1 + 68) )
      return 0;
  }
  return *(_DWORD *)(a1 + 4 * v3 + 72);
}

/* ---- ReportPlayEffectError  0x004936A0 ----  [CONFIRMED] */
void __cdecl ReportPlayEffectError(int a1)
{
  SFxHelper__Print((int)&theFxHelper, "CFxScheduler::PlayEffect called with invalid effect ID: %i\n", a1);
}

/* ---- CFxScheduler__PlayEffect_id_simple_m  0x004936C0 ----  [HIGH] */
void __cdecl CFxScheduler__PlayEffect_id_simple_m(float *a1, int a2, _DWORD *a3)
{
  int v3[9]; // [esp+0h] [ebp-24h] BYREF

  v3[0] = 0;
  v3[1] = 0;
  v3[2] = 1065353216;
  v3[3] = 1065353216;
  memset(&v3[4], 0, 12);
  v3[7] = 1065353216;
  v3[8] = 0;
  CFxScheduler__PlayEffect_id_axis(a3, a2, a1, v3, 0);
}

/* ---- sub_493730  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_493730() { return 0; }
#if 0
void __cdecl sub_493730(int *a1, _DWORD *a2, int a3, float *a4)
{
  int v4;
  int v5;
  /* retail 0x00493730 frame 0x08..0x2C is ONE 36-byte axis[3][3]; 0x00493762
   * passes that base. */
  int v6[9]; // [esp+8h] [ebp-24h] BYREF

  v4 = a1[1];
  v5 = a1[2];
  v6[0] = *a1;
  v6[1] = v4;
  v6[2] = v5;
  /* forward first */
  MakeNormalVectors( (const float *)a1, (float *)&v6[3], (float *)&v6[6] );
  CFxScheduler__PlayEffect_id_axis(a2, a3, a4, v6, 0);
}
#endif

/* ---- CFxScheduler__PlayEffect_name_bolt_m  0x00493780 ----  [HIGH] */
void __cdecl CFxScheduler__PlayEffect_name_bolt_m(char *a1, _DWORD *a2, float *a3, int *a4, int *a5)
{
  char v5;
  bool v6; // zf
  _BYTE *v7;
  int v8;
  int *v9;
  void *v10; // [esp+0h] [ebp-6Ch] BYREF
  void *Block;
  int v12;
  unsigned int v13;
  _DWORD v14[20]; // [esp+1Ch] [ebp-50h] BYREF
  unsigned int retaddr;

  v5 = *a1;
  v6 = *a1 == 0;
  v14[16] = retaddr ^ _security_cookie;
  v7 = v14;
  if ( !v6 )
  {
    v8 = a1 - (char *)v14;
    do
    {
      if ( v5 == 46 )
        break;
      *v7 = v5;
      v5 = (v7++)[v8 + 1];
    }
    while ( v5 );
  }
  *v7 = 0;
  v13 = 15;
  v12 = 0;
  LOBYTE(Block) = 0;
  sub_498410((int)&v10, v14, (void **)strlen((const char *)v14));
  v14[19] = 0;
  v9 = (int *)sub_497890((int)(a2 + 5376), &v10);
  CFxScheduler__PlayEffect_id_axis(a2, *v9, a3, a4, a5);
  if ( v13 >= 0x10 )
    j__free(Block);
}

/* ---- CFxScheduler__PlayEffect_id_axis  0x00493870 ----  VERIFIED */
void __cdecl   CFxScheduler__PlayEffect_id_axis( _DWORD *a1, int a2, float *a3, int *a4, int *a5 )
{
  int      effectId;
  int      primIndex;
  int      primCount;
  int     *primSlot;
  _DWORD  *effectTemplate;
  int      tmpl;
  int      cullRadius;
  int      count;
  int      spawned;
  int      totalSpawned;
  int      delayMsec;
  int      flags;
  int     *rec;
  int     *listHead;
  int      headNode;
  _DWORD  *node;
  float    spread;
  float    dx, dy, dz;
  float    delay;
  float    randSample;
  /* retail [ebp-30h] is ONE 48-byte orientation, origin then 3x3 axis;
   * FX_GetBoneOrientation writes all 12 floats through one pointer, so keep
   * them contiguous. */
  float    orient[12];         /* [ebp-30h] BYREF */

  effectId = a2;
  spread   = 0.0;

  /* 0x0049387E-0x0049389F: 1 <= id < 128 and the template must be active. */
  if ( a2 < 1 || a2 >= 128 || !*((unsigned char *)a1 + 0xA8 * a2) )
  {
    dword_1432880(1);
    a2 = CFxScheduler__RegisterEffect( (_DWORD *)dword_14075A0, "fx/error.efx", 0 );
    dword_1432880(0);
    if ( !a2 )
    {
      SFxHelper__Print( (int)&theFxHelper,
                        "CFxScheduler::PlayEffect called with invalid effect ID: %i\n",
                        0 );
      return;
    }
    effectId = a2;
  }

  if ( fx_freeze->integer || !fx_enable->integer )
    return;

  if ( a5 && *a5 >= 0 )
  {
    if ( !FX_GetBoneOrientation( orient, (_DWORD *)a5 ) )
      return;
  }
  else
  {
    if ( a3 )
    {
      orient[0] = a3[0];
      orient[1] = a3[1];
      orient[2] = a3[2];
    }
    else
    {
      orient[0] = 0.0;
      orient[1] = 0.0;
      orient[2] = 0.0;
    }
    AxisCopy( a4, orient + 3 );
  }

  effectTemplate = &a1[42 * effectId];
  primCount      = effectTemplate[17];          /* +0x44 primitiveCount */
  primSlot       = (int *)(effectTemplate + 18);/* +0x48 primitives[]   */
  totalSpawned   = 0;
  primIndex      = 0;

  if ( primCount > 0 )
  {
    do
    {
      ++dword_1407318;
      tmpl       = *primSlot;
      cullRadius = *(int *)(tmpl + 100);         /* +0x64, INTEGER */

      dx = flt_140751C - a3[0];
      dy = flt_1407520 - a3[1];
      dz = flt_1407524 - a3[2];

      if ( !cullRadius
        || dz * dz + dy * dy + dx * dx <= (double)( cullRadius * cullRadius ) )
      {
        if ( *(float *)(tmpl + 84) == *(float *)(tmpl + 88) )
          count = (int)*(float *)(tmpl + 84);
        else
          count = (int)( Com_RandFloatRange_m( *(float *)(tmpl + 84),
                                               *(float *)(tmpl + 88) ) + 0.5 );

        /* 0x00493A63: a BYTE test at +0, a DWORD store at +4. */
        if ( *(unsigned char *)tmpl )
          *(int *)(tmpl + 4) = count;

        if ( ( *(int *)(tmpl + 188) & 0x2000 ) != 0 )
          spread = (float)( fabs( *(float *)(tmpl + 80) - *(float *)(tmpl + 76) )
                            / (double)count );

        totalSpawned += count;
        spawned = 0;

        if ( count > 0 )
        {
          while ( 1 )
          {
            flags = *(int *)(tmpl + 188);
            ++dword_140731C;

            if ( ( flags & 0x2000 ) != 0 )
            {
              delay = (float)( (double)spawned * spread );
            }
            else if ( *(float *)(tmpl + 76) == *(float *)(tmpl + 80) )
            {
              delay = *(float *)(tmpl + 76);
            }
            else
            {
              com_randSeed_m = 214013 * com_randSeed_m + 2531011;
              randSample = (float)(unsigned int)( (unsigned int)com_randSeed_m >> 17 );
              delay = (float)( ( *(float *)(tmpl + 80) - *(float *)(tmpl + 76) )
                               * randSample / 32768.0
                               + *(float *)(tmpl + 76) );
            }

            delayMsec = (int)delay;

            if ( delayMsec < 1 )
            {
              CFxScheduler__CreateEffect( a1, tmpl, a5, orient, (int)( orient + 3 ), -delayMsec );
            }
            else
            {
              rec = (int *)FxMem_AllocScheduledEffect( (int *)&dword_1407594, 0x44 );
              if ( rec )
              {
                rec[2] = fxMTime + delayMsec;   /* +0x08 scheduledTime */
                rec[0] = a2;                          /* +0x00 effectId      */
                rec[1] = primIndex;                   /* +0x04 primitiveIndex*/

                if ( a5 )
                {
                  rec[3] = a5[0];                     /* +0x0C boltInfo      */
                  rec[4] = a5[1];
                  if ( a5[0] >= 0 && a5[1] < 0 )
                    AxisCopy( a4, (float (*)[3])(rec + 8) );
                }
                else
                {
                  rec[3] = -1;
                  rec[4] = -1;
                }

                if ( !a5 || a5[0] < 0 )
                {
                  if ( a3 )
                  {
                    rec[5] = *(int *)&a3[0];          /* +0x14 origin */
                    rec[6] = *(int *)&a3[1];
                    rec[7] = *(int *)&a3[2];
                  }
                  else
                  {
                    rec[7] = 0;
                    rec[6] = 0;
                    rec[5] = 0;
                  }
                  rec[8]  = a4[0];                    /* +0x20 axis */
                  rec[9]  = a4[1];
                  rec[10] = a4[2];
                  rec[11] = a4[3];
                  rec[12] = a4[4];
                  rec[13] = a4[5];
                  rec[14] = a4[6];
                  rec[15] = a4[7];
                  rec[16] = a4[8];
                }

                listHead = (int *)a1[5380];           /* +0x5410 _Myhead */
                headNode = listHead ? *listHead : 0;
                node = (_DWORD *)sub_498DB0( (int)(a1 + 5379), headNode,
                                             *(int *)(headNode + 4), (_DWORD *)&rec );
                sub_498DE0( 1u, (int)(a1 + 5379) );
                *(int *)(headNode + 4) = (int)node;
                *(_DWORD *)node[1] = (_DWORD)node;
              }
            }

            if ( ++spawned >= count )
              break;
          }
        }
      }
      ++primSlot;
    }
    while ( ++primIndex < (int)effectTemplate[17] );

    if ( totalSpawned && fx_count->integer )
      ((void (__cdecl *)( float *, int, unsigned int *, int ))dword_1432928)(
          orient, totalSpawned, dword_541870, 3000 );
  }

  /* 0x00493D0F: a temporary template is consumed by playing it once. */
  if ( ((unsigned char *)effectTemplate)[1] )
    ((unsigned char *)effectTemplate)[0] = 0;
}

/* ---- CFxScheduler__PlayEffect_name_simple_m  0x00493D30 ----  [HIGH] */
void __cdecl CFxScheduler__PlayEffect_name_simple_m(char *a1, _DWORD *a2, float *a3)
{
  _BYTE *v3;
  char v4;
  int v5;
  int *v6;
  void *v7; // [esp+0h] [ebp-6Ch] BYREF
  void *Block;
  int v9;
  unsigned int v10;
  _DWORD v11[20]; // [esp+1Ch] [ebp-50h] BYREF
  unsigned int retaddr;

  v3 = v11;
  v11[16] = retaddr ^ _security_cookie;
  v4 = *a1;
  if ( *a1 )
  {
    v5 = a1 - (char *)v11;
    do
    {
      if ( v4 == 46 )
        break;
      *v3 = v4;
      v4 = (v3++)[v5 + 1];
    }
    while ( v4 );
  }
  *v3 = 0;
  v10 = 15;
  v9 = 0;
  LOBYTE(Block) = 0;
  sub_498410((int)&v7, v11, (void **)strlen((const char *)v11));
  v11[19] = 0;
  v6 = (int *)sub_497890((int)(a2 + 5376), &v7);
  CFxScheduler__PlayEffect_id_simple_m(a3, *v6, a2);
  if ( v10 >= 0x10 )
    j__free(Block);
}

/* ---- CFxScheduler__PlayEffect_name_m  0x00493E10 ----  [HIGH] */
void __cdecl CFxScheduler__PlayEffect_name_m(int *a1, char *a2, _DWORD *a3, float *a4)
{
  char v4;
  _BYTE *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  void *v11; // [esp+10h] [ebp-90h] BYREF
  void *Block;
  int v13;
  unsigned int v14;
  int v15[9]; // [esp+2Ch] [ebp-74h] BYREF  -- retail: float axis[3][3]
  _DWORD v18[20]; // [esp+50h] [ebp-50h] BYREF
  unsigned int retaddr;

  v18[16] = retaddr ^ _security_cookie;
  v4 = *a2;
  v6 = v18;
  if ( *a2 )
  {
    v7 = a2 - (char *)v18;
    do
    {
      if ( v4 == 46 )
        break;
      *v6 = v4;
      v4 = (v6++)[v7 + 1];
    }
    while ( v4 );
  }
  *v6 = 0;
  v14 = 15;
  v13 = 0;
  LOBYTE(Block) = 0;
  sub_498410((int)&v11, v18, (void **)strlen((const char *)v18));
  v18[19] = 0;
  v8 = *(_DWORD *)sub_497890((int)(a3 + 5376), &v11);
  v9 = a1[1];
  v10 = a1[2];
  v15[0] = *a1;
  v15[1] = v9;
  v15[2] = v10;
  MakeNormalVectors( (const float *)a1, (float *)&v15[3], (float *)&v15[6] );
  CFxScheduler__PlayEffect_id_axis(a3, v8, a4, v15, 0);
  if ( v14 >= 0x10 )
    j__free(Block);
}

void FX_DrawStats_m();

/* ---- CFxScheduler__AddScheduledEffects  0x00493F40 ----  VERIFIED */
void __cdecl CFxScheduler__AddScheduledEffects( _DWORD *self )
{
  _DWORD *node;
  _DWORD *next;
  int    *rec;
  int     tmpl;
  int     now;
  float   ori[12];

  if ( !fx_enable->integer )
    return;

  node = self[5380] ? *(_DWORD **)self[5380] : 0;

  if ( node != (_DWORD *)self[5380] )
  {
    now = fxMTime;
    do
    {
      rec  = (int *)node[2];
      next = (_DWORD *)node[0];

      if ( rec[2] <= now )                  /* +0x08 scheduledTime reached */
      {
        tmpl = self[42 * rec[0] + 18 + rec[1]];

        if ( rec[3] < 0 )
        {
          CFxScheduler__CreateEffect( self, tmpl, rec + 3,
                                      (float *)(rec + 5), (float *)(rec + 8),
                                      now - rec[2] );
        }
        else
        {
          FX_GetBoneOrientation( ori, (_DWORD *)(rec + 3) );
          rec = (int *)node[2];             /* retail reloads at 0x00493FAC */
          CFxScheduler__CreateEffect( self, tmpl, rec + 3,
                                      ori, ori + 3,
                                      fxMTime - rec[2] );
        }

        sub_4977F0( (char *)node[2], &dword_1407594 );

        if ( node != (_DWORD *)self[5380] )
        {
          *(_DWORD *)node[1]       = node[0];
          *(_DWORD *)(node[0] + 4) = node[1];
          j__free( node );
          --self[5381];
        }
        now = fxMTime;
      }
      node = next;
    }
    while ( node != (_DWORD *)self[5380] );
  }

  FX_DrawStats_m();
}

/* ---- CFxScheduler__CreateEffect  0x00494050 ----  VERIFIED
 * @fidelity: verified
 */
void __cdecl CFxScheduler__CreateEffect( _DWORD *self, int tmpl, int *bolt,
                                         float *origin, float *axisIn,
                                         int timeOffset )
{
  float  axis[9];          /* [ebp-D4h] BYREF                              */
  float  start[3];         /* [ebp-E4h] BYREF -- effect origin             */
  char  *boltFrame;             /* [ebp-D8h] BYREF -- passed to every FX_Add*   */
  trace_t results;         /* [ebp-64h] BYREF -- 48 bytes, matches trace_t */
  int    v132;             /* [ebp-11Ch] BYREF -- float/int scratch slot   */
  float  v119;
  float  v7, v9;
  int    v8;
  int    v163;
  int    v6;
  char *tpl = (char *)tmpl;
  float endp[3], tmp3[3], spawnOrigin[3], rng3[3];
  float velocity[3], accel[3], sum3[3];
  float v120, v137, v138, v140, frac;
  /*
   * [ebp-10Ch] is an untyped dword slot: retail does `fstp [ebp-10Ch]` then
   * `mov esi, [ebp-10Ch]` (0x496936/0x49693A), a bit copy, not a conversion,
   * so it stays int.
   */
  int   v136;
  int   d6, *bf, *bfKeep;
  int   life, lifeI, r254, r1DC, r124, r12C;
  int   v1E4_0, v1E4_1, v1E4_2,  v1FC_0, v1FC_1, v1FC_2;
  int   v214_0, v214_1, v214_2,  v22C_0, v22C_1, v22C_2;
  int   e134_0, e134_1, e134_2,  t14C;
  int   density, variance, flags;
  int   pick68, pick78, pick88, pick98;
  int   r1E4, r1EC, r1F4, r1FC, r204, r20C, r214, r21C, r224, pick;

  v163 = (int)self;                       /* 0x00494071 */
  boltFrame = 0;                               /* 0x00494078 */
  v6   = tmpl;                            /* 0x0049407C */
  /* nine-dword axis spill, 0x0049408A-0x004940CB */
  axis[0] = axisIn[0];  axis[1] = axisIn[1];  axis[2] = axisIn[2];
  axis[3] = axisIn[3];  axis[4] = axisIn[4];  axis[5] = axisIn[5];
  v7      = axisIn[6];
  axis[6] = v7;
  axis[7] = axisIn[7];  axis[8] = axisIn[8];
  if ( ( *(int *)(tmpl + 0xBC) & 0x1000 ) != 0 )
  {
    *(int *)&v132 = rand();                                  /* 0x004940F3 */
    v119 = (float)( *(int *)&v132 / 32768.0 * 360.0 );       /* 0x00494112 */
    RotatePointAroundVector( &axis[0], &axis[3], axisIn + 3, v119 );
    v6 = tmpl;                                               /* 0x00494124 */
    axis[6] = axis[1] * axis[5] - axis[2] * axis[4];          /* 0x00494138 */
    axis[7] = axis[2] * axis[3] - axis[0] * axis[5];          /* 0x0049414E */
    axis[8] = axis[0] * axis[4] - axis[1] * axis[3];          /* 0x00494164 */
  }
  if ( ( *(int *)(v6 + 0xBC) & 0x100 ) != 0 )
  {
    if ( *(float *)(v6 + 0xDC) == *(float *)(v6 + 0xE0) )     /* 0x00494188 */
      v8 = *(int *)(v6 + 0xDC);                               /* 0x0049418A */
    else
    {
      *(float *)&v132 = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xDC),
                                                     *(float *)(v6 + 0xE0) );
      v8 = v132;
    }
    *(int *)&start[0] = v8;                                   /* 0x004941B0 */
    v9 = *(float *)(v6 + 0xE4);                               /* 0x004941C5 */
    if ( v9 != *(float *)(v6 + 0xE8) )
    {
      *(float *)&v132 = (float)Com_RandFloatRange_m( v9, *(float *)(v6 + 0xE8) );
      v9 = *(float *)&v132;
    }
    start[1] = v9;                                            /* 0x004941E5 */
    if ( *(float *)(v6 + 0xEC) == *(float *)(v6 + 0xF0) )     /* 0x004941FA */
      start[2] = *(float *)(v6 + 0xEC);
    else
    {
      *(float *)&v132 = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xEC),
                                                     *(float *)(v6 + 0xF0) );
      *(int *)&start[2] = v132;                               /* 0x00494229 */
    }
  }
  else
  {
    float ofsUp, ofsRight, ofsFwd;
    if ( *(float *)(v6 + 0xEC) == *(float *)(v6 + 0xF0) )     /* 0x00494243 */
      ofsUp = *(float *)(v6 + 0xEC);
    else
      ofsUp = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xEC),
                                           *(float *)(v6 + 0xF0) );
    if ( *(float *)(v6 + 0xE4) == *(float *)(v6 + 0xE8) )     /* 0x0049427C */
      ofsRight = *(float *)(v6 + 0xE4);
    else
      ofsRight = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xE4),
                                              *(float *)(v6 + 0xE8) );
    if ( *(float *)(v6 + 0xDC) == *(float *)(v6 + 0xE0) )     /* 0x004942B5 */
      ofsFwd = *(float *)(v6 + 0xDC);
    else
      ofsFwd = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xDC),
                                            *(float *)(v6 + 0xE0) );
    start[0] = ofsFwd * axis[0] + ofsRight * axis[3] + ofsUp * axis[6];
    start[1] = ofsFwd * axis[1] + ofsRight * axis[4] + ofsUp * axis[7];
    start[2] = ofsFwd * axis[2] + ofsRight * axis[5] + ofsUp * axis[8];
  }
  start[0] = start[0] + origin[0];                            /* 0x0049434F */
  start[1] = start[1] + origin[1];
  start[2] = start[2] + origin[2];
  if ( ( *(int *)(v6 + 0xBC) & 1 ) != 0 )                     /* 0x00494371 */
  {
    float yaw, pitch, sinYaw, cosYaw, sinPitch, cosPitch;
    float rad, hgt, horiz;
    *(int *)&v132 = rand();                                   /* 0x0049438C */
    yaw    = (float)( *(int *)&v132 / 32768.0 * 360.0 * 3.1415927 / 180.0 );
    cosYaw = (float)cos( yaw );                               /* 0x004943C8 */
    sinYaw = (float)sin( yaw );                               /* 0x004943CA */
    *(int *)&v132 = rand();                                   /* 0x004943E3 */
    pitch    = (float)( *(int *)&v132 / 32768.0 * 180.0 * 3.1415927 / 180.0 );
    cosPitch = (float)cos( pitch );                           /* 0x0049441F */
    sinPitch = (float)sin( pitch );                           /* 0x00494421 */
    v6 = tmpl;
    if ( *(float *)(tmpl + 0x10C) == *(float *)(tmpl + 0x110) )
      rad = *(float *)(tmpl + 0x10C);
    else
      rad = (float)Com_RandFloatRange_m( *(float *)(tmpl + 0x10C),
                                         *(float *)(tmpl + 0x110) );
    if ( *(float *)(tmpl + 0x114) == *(float *)(tmpl + 0x118) )
      hgt = *(float *)(tmpl + 0x114);
    else
      hgt = (float)Com_RandFloatRange_m( *(float *)(tmpl + 0x114),
                                         *(float *)(tmpl + 0x118) );
    horiz = rad * sinPitch;                                   /* 0x004944A6 */
    *(float *)&v136 = horiz * sinYaw;                         /* 0x004944B2 */
    v137  = horiz * cosYaw;                                   /* 0x004944BE */
    v138  = hgt   * cosPitch;                                 /* 0x004944CA */
    start[0] = *(float *)&v136 + start[0];
    start[1] = v137 + start[1];
    start[2] = v138 + start[2];
    if ( ( *(int *)(tmpl + 0xBC) & 2 ) != 0 )
    {
      VectorNormalize2( (float *)&v136, &axis[0] );           /* 0x00494508 */
      MakeNormalVectors( &axis[0], &axis[3], &axis[6] );      /* 0x0049451B */
    }
    goto LABEL_43;
  }
  if ( ( *(int *)(v6 + 0xBC) & 4 ) != 0 )                     /* 0x0049452C */
  {
    float hgt, rad, axial;
    float ofs[3];
    float tmp3[3];
    if ( *(float *)(v6 + 0x114) == *(float *)(v6 + 0x118) )
      hgt = *(float *)(v6 + 0x114);
    else
      hgt = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x114),
                                         *(float *)(v6 + 0x118) );
    *(int *)&v132 = rand();                                   /* 0x0049457F */
    axial = (float)( ( *(int *)&v132 / 32768.0
                     + *(int *)&v132 / 32768.0 - 1.0 ) * hgt * 0.5 );
    ofs[0] = axial;
    if ( *(float *)(tmpl + 0x10C) == *(float *)(tmpl + 0x110) )
      rad = *(float *)(tmpl + 0x10C);
    else
      rad = (float)Com_RandFloatRange_m( *(float *)(tmpl + 0x10C),
                                         *(float *)(tmpl + 0x110) );
    tmp3[0] = axis[3] * rad;                                  /* 0x004945E4 */
    tmp3[1] = axis[4] * rad;
    tmp3[2] = axis[5] * rad;
    tmp3[0] = axis[0] * ofs[0] + tmp3[0];                     /* 0x00494618 */
    tmp3[1] = axis[1] * ofs[0] + tmp3[1];
    tmp3[2] = axis[2] * ofs[0] + tmp3[2];
    *(int *)&v132 = rand();                                   /* 0x00494660 */
    v120 = (float)( *(int *)&v132 / 32768.0 * 360.0 );
    RotatePointAroundVector( &axis[0], ofs, tmp3, v120 );     /* 0x00494681 */
    v6 = tmpl;
    start[0] = ofs[0] + start[0];                             /* 0x00494698 */
    start[1] = ofs[1] + start[1];
    start[2] = ofs[2] + start[2];
    if ( ( *(int *)(tmpl + 0xBC) & 2 ) != 0 )                 /* 0x004946BC */
    {
      float refY = 0.0, refZ = 1.0;
      VectorNormalize2( ofs, &axis[0] );                      /* 0x004946DC */
      if ( axis[2] == 1.0 )
      {
        refY = 1.0;
        refZ = 0.0;
      }
      axis[3] = refY * axis[2] - refZ * axis[1];              /* 0x00494713 */
      axis[4] = refZ * axis[0] - axis[2] * 0.0;
      axis[5] = axis[1] * 0.0 - refY * axis[0];
      axis[6] = axis[1] * axis[5] - axis[2] * axis[4];        /* 0x0049475F */
    }
    goto LABEL_43;
  }
LABEL_43:                                     /* 0x00494796 */
  {
    int type = *(int *)(v6 + 0x48);                           /* 0x00494796 */
    if ( type == 1 || type == 8 || type == 3 || type == 5 )   /* 0x004947AB */
    {
      if ( ( *(int *)(v6 + 0xBC) & 0x400 ) != 0 )             /* 0x004947BA */
      {
        float a, b, c;
        if ( *(float *)(v6 + 0x164) == *(float *)(v6 + 0x168) )
          a = *(float *)(v6 + 0x164);                         /* 0x004947D3 */
        else
          a = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x164),
                                           *(float *)(v6 + 0x168) );
        velocity[0] = a;                                             /* 0x004947FF */
        if ( *(float *)(v6 + 0x16C) == *(float *)(v6 + 0x170) )
          b = *(float *)(v6 + 0x16C);                         /* 0x00494813 */
        else
          b = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x16C),
                                           *(float *)(v6 + 0x170) );
        velocity[1] = b;                                             /* 0x0049483F */
        if ( *(float *)(v6 + 0x174) == *(float *)(v6 + 0x178) )
          velocity[2] = *(float *)(v6 + 0x174);                      /* 0x00494859 */
        else
          velocity[2] = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x174),
                                              *(float *)(v6 + 0x178) );
      }
      else
      {
        float compUp, compRight, compFwd;
        if ( *(float *)(v6 + 0x174) == *(float *)(v6 + 0x178) )
          compUp = *(float *)(v6 + 0x174);                    /* 0x004948A8 */
        else
          compUp = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x174),
                                                *(float *)(v6 + 0x178) );
        if ( *(float *)(v6 + 0x16C) == *(float *)(v6 + 0x170) )
          compRight = *(float *)(v6 + 0x16C);                 /* 0x004948DB */
        else
          compRight = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x16C),
                                                   *(float *)(v6 + 0x170) );
        if ( *(float *)(v6 + 0x164) == *(float *)(v6 + 0x168) )
          compFwd = *(float *)(v6 + 0x164);                   /* 0x0049490E */
        else
          compFwd = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x164),
                                                 *(float *)(v6 + 0x168) );
        velocity[0] = compRight * axis[3] + compUp * axis[6] + compFwd * axis[0];
        velocity[1] = compRight * axis[4] + compUp * axis[7] + compFwd * axis[1];
        velocity[2] = compUp    * axis[8] + compFwd  * axis[2] + compRight * axis[5];
      }
    }
    if ( ( *(int *)(tmpl + 0xBC) & 0x10000 ) != 0 )
      Com_Printf( "FX_PORT wind TBD.\n" );
    if ( ( *(int *)(v6 + 0xBC) & 0x800 ) != 0 )               /* 0x004949BD */
    {
      if ( *(float *)(v6 + 0x17C) == *(float *)(v6 + 0x180) )
        accel[0] = *(float *)(v6 + 0x17C);                        /* 0x004949D6 */
      else
        accel[0] = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x17C),
                                            *(float *)(v6 + 0x180) );
      if ( *(float *)(v6 + 0x184) == *(float *)(v6 + 0x188) )
        accel[1] = *(float *)(v6 + 0x184);                        /* 0x00494A16 */
      else
        accel[1] = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x184),
                                            *(float *)(v6 + 0x188) );
      if ( *(float *)(v6 + 0x18C) != *(float *)(v6 + 0x190) )
        accel[2] = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x18C),
                                            *(float *)(v6 + 0x190) );
      else
        accel[2] = *(float *)(v6 + 0x18C);                        /* 0x00494A54 */
    }
    else
    {
      float cUp, cRight, cFwd;
      if ( *(float *)(v6 + 0x18C) == *(float *)(v6 + 0x190) )
        cUp = *(float *)(v6 + 0x18C);                         /* 0x00494A99 */
      else
        cUp = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x18C),
                                           *(float *)(v6 + 0x190) );
      if ( *(float *)(v6 + 0x184) == *(float *)(v6 + 0x188) )
        cRight = *(float *)(v6 + 0x184);                      /* 0x00494AD2 */
      else
        cRight = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x184),
                                              *(float *)(v6 + 0x188) );
      if ( *(float *)(v6 + 0x17C) == *(float *)(v6 + 0x180) )
        cFwd = *(float *)(v6 + 0x17C);                        /* 0x00494B0B */
      else
        cFwd = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x17C),
                                            *(float *)(v6 + 0x180) );
      accel[0] = cRight * axis[3] + cUp * axis[6] + cFwd * axis[0];
      accel[1] = cRight * axis[4] + cUp * axis[7] + cFwd * axis[1];
      accel[2] = cUp    * axis[8] + cFwd * axis[2] + cRight * axis[5];
    }
    if ( *(float *)(v6 + 0x194) == *(float *)(v6 + 0x198) )
      v140 = *(float *)(v6 + 0x194);                          /* 0x00494BA7 */
    else
      v140 = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x194),
                                          *(float *)(v6 + 0x198) );
    accel[2] = v140 + accel[2];                               /* 0x00494BE1 */
    if ( timeOffset > 0 )                                     /* 0x00494BE8 */
    {
      float t, half, coeff;
      int   i;
      t    = (float)( (double)timeOffset * 0.001 );           /* 0x00494BFF */
      half = (float)( t * t * 0.5 );                          /* 0x00494C11 */
      velocity[0] = t * accel[0] + velocity[0];                         /* 0x00494C1B */
      velocity[1] = t * accel[1] + velocity[1];                         /* 0x00494C34 */
      velocity[2] = t * accel[2] + velocity[2];                         /* 0x00494C4D */
      coeff = half + t;                                       /* 0x00494C66 */
      for ( i = 0; i < 3; ++i )                               /* 0x00494C72 */
        start[i] = coeff * velocity[i] + start[i];
    }
  }
  {
    int type2 = *(int *)(v6 + 0x48);                          /* 0x00494C8D */
    if ( type2 == 2 || type2 == 9 )                           /* 0x00494C98 */
    {
      int flags = *(int *)(v6 + 0xBC);
      if ( ( flags & 0x10 ) != 0 )                            /* 0x00494CA6 */
      {
        float endp[3];
        endp[0] = (float)( axis[0] * 16384.0 + start[0] );    /* 0x00494CBC */
        endp[1] = (float)( axis[1] * 16384.0 + start[1] );
        endp[2] = (float)( axis[2] * 16384.0 + start[2] );
        if ( ( flags & 0x40 ) != 0 )                          /* 0x00494CE4 */
        {
          if ( ( flags & 0x200 ) != 0 )                       /* 0x00494CED */
          {
            float dx, dy, dz;
            if ( *(float *)(v6 + 0xF4) == *(float *)(v6 + 0xF8) )
              dx = *(float *)(v6 + 0xF4);                     /* 0x00494D0C */
            else
              dx = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xF4),
                                                *(float *)(v6 + 0xF8) );
            if ( *(float *)(v6 + 0xFC) == *(float *)(v6 + 0x100) )
              dy = *(float *)(v6 + 0xFC);                     /* 0x00494D45 */
            else
              dy = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xFC),
                                                *(float *)(v6 + 0x100) );
            if ( *(float *)(v6 + 0x104) == *(float *)(v6 + 0x108) )
              dz = *(float *)(v6 + 0x104);                    /* 0x00494D7E */
            else
              dz = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x104),
                                                *(float *)(v6 + 0x108) );
            endp[0] = dx + endp[0];                           /* 0x00494DA6 */
            endp[1] = dy + endp[1];
          }
          else
          {
            float lx, ly, lz, world[3];
            if ( *(float *)(v6 + 0x104) == *(float *)(v6 + 0x108) )
              lz = *(float *)(v6 + 0x104);                    /* 0x00494DD2 */
            else
              lz = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x104),
                                                *(float *)(v6 + 0x108) );
            if ( *(float *)(v6 + 0xFC) == *(float *)(v6 + 0x100) )
              ly = *(float *)(v6 + 0xFC);                     /* 0x00494E0B */
            else
              ly = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xFC),
                                                *(float *)(v6 + 0x100) );
            if ( *(float *)(v6 + 0xF4) == *(float *)(v6 + 0xF8) )
              lx = *(float *)(v6 + 0xF4);                     /* 0x00494E44 */
            else
              lx = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xF4),
                                                *(float *)(v6 + 0xF8) );
            sub_492A20( axis, world, lx, ly, lz );            /* 0x00494E78 */
            endp[0] = world[0] + endp[0];
            endp[1] = world[1] + endp[1];
            endp[2] = world[2] + endp[2];
          }
        }
        results.fraction = 1.0;                               /* 0x00494ED0 */
        CM_Trace( &results, start, endp,
                  NULL, NULL,
                  0, vec3_origin, 1, 0,
                  NULL );
        results.entityNum = 1022;                             /* 0x00494EEC */
        if ( results.fraction == 1.0 )
          results.entityNum = 1023;

        /* The traced end point IS the second point of the line/electricity
         * primitive.  Written before the 0x20 test, so it applies whether or
         * not the impact effect below is chained. */
        spawnOrigin[1] = results.endpos[1];                   /* 0x00494F23 */
        spawnOrigin[0] = results.endpos[0];                   /* 0x00494F31 */
        spawnOrigin[2] = results.endpos[2];                   /* 0x00494F38 */

        if ( ( *(int *)(v6 + 0xBC) & 0x20 ) != 0 )            /* 0x00494F2A */
        {
          int listBase, listCount, chosen;
          /* one contiguous axis[3][3], retail esp+0xFC/0x108/0x114 */
          int chainAxis[9];

          listBase = *(int *)(v6 + 0x7C);                     /* 0x00494F45 */
          if ( listBase
            && (listCount = (*(int *)(v6 + 0x80) - listBase) >> 2) != 0 )
          {
            com_randSeed_m = 214013 * com_randSeed_m + 2531011;  /* 0x00494F5F */
            chosen = *(int *)( listBase
                       + 4 * ( (int)( listCount
                                      * ( (unsigned int)com_randSeed_m >> 17 ) ) >> 15 ) );
          }
          else
          {
            chosen = 0;                                       /* 0x00494F5B */
          }
          chainAxis[0] = *(int *)&results.normal[0];          /* 0x00494FB1 */
          chainAxis[1] = *(int *)&results.normal[1];          /* 0x00494F95 */
          chainAxis[2] = *(int *)&results.normal[2];          /* 0x00494FB8 */
          MakeNormalVectors( results.normal,
                             (float *)&chainAxis[3],
                             (float *)&chainAxis[6] );        /* 0x00494FBF */
          CFxScheduler__PlayEffect_id_axis( (_DWORD *)v163, chosen,
                                            spawnOrigin, chainAxis, 0 ); /* 0x00494FDF */
        }
      }
      else                                                    /* 0x00494FF0 */
      {
        if ( ( flags & 0x200 ) != 0 )                         /* 0x00494FF0 */
        {
          /* world-space offsets: no axis rotation, straight into spawnOrigin */
          if ( *(float *)(v6 + 0xF4) == *(float *)(v6 + 0xF8) )       /* 0x00494FF9 */
            spawnOrigin[0] = *(float *)(v6 + 0xF4);
          else
            spawnOrigin[0] = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xF4),
                                                          *(float *)(v6 + 0xF8) );
          if ( *(float *)(v6 + 0xFC) == *(float *)(v6 + 0x100) )      /* 0x0049502A */
            spawnOrigin[1] = *(float *)(v6 + 0xFC);
          else
            spawnOrigin[1] = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xFC),
                                                          *(float *)(v6 + 0x100) );
          if ( *(float *)(v6 + 0x104) == *(float *)(v6 + 0x108) )     /* 0x0049506A */
            spawnOrigin[2] = *(float *)(v6 + 0x104);
          else
            spawnOrigin[2] = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x104),
                                                          *(float *)(v6 + 0x108) );
        }
        else                                                  /* 0x004950C0 */
        {
          float ofsX, ofsY, ofsZ;
          if ( *(float *)(v6 + 0x104) == *(float *)(v6 + 0x108) )     /* 0x004950C0 */
            ofsZ = *(float *)(v6 + 0x104);
          else
            ofsZ = (float)Com_RandFloatRange_m( *(float *)(v6 + 0x104),
                                                *(float *)(v6 + 0x108) );
          if ( *(float *)(v6 + 0xFC) == *(float *)(v6 + 0x100) )      /* 0x004950F9 */
            ofsY = *(float *)(v6 + 0xFC);
          else
            ofsY = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xFC),
                                                *(float *)(v6 + 0x100) );
          if ( *(float *)(v6 + 0xF4) == *(float *)(v6 + 0xF8) )       /* 0x00495132 */
            ofsX = *(float *)(v6 + 0xF4);
          else
            ofsX = (float)Com_RandFloatRange_m( *(float *)(v6 + 0xF4),
                                                *(float *)(v6 + 0xF8) );
          spawnOrigin[0] = ofsY * axis[3] + ofsZ * axis[6] + ofsX * axis[0]; /* 0x0049516B */
          spawnOrigin[1] = ofsY * axis[4] + ofsZ * axis[7] + ofsX * axis[1]; /* 0x0049518E */
          spawnOrigin[2] = ofsZ * axis[8] + ofsX * axis[2] + ofsY * axis[5]; /* 0x004951B1 */
        }
        /* 0x004951D4: ESI is arg_8 = origin, not start. */
        spawnOrigin[0] = spawnOrigin[0] + origin[0];          /* 0x004951D4 */
        spawnOrigin[1] = spawnOrigin[1] + origin[1];          /* 0x004951E4 */
        spawnOrigin[2] = spawnOrigin[2] + origin[2];          /* 0x004951F5 */
      }
    }
  }

  if ( *(int *)(tmpl + 0x48) != 6                             /* 0x00495206 */
    && *(int *)(tmpl + 0x48) != 10
    && *(int *)(tmpl + 0x48) != 12 )                          /* 0x0049521E */
  {
    if ( ( *(int *)(tmpl + 0xBC) & 0x4000 ) != 0 )            /* 0x0049522D */
    {
      *(int *)&v132 = rand();                                 /* 0x00495247 */
      frac = (float)( *(int *)&v132 / 32768.0 );              /* 0x00495255 */
      rng3[0] = (float)( ( *(float *)(tmpl + 0x1B0) - *(float *)(tmpl + 0x1AC) )
                         * frac + *(float *)(tmpl + 0x1AC) ); /* 0x0049526F */
      rng3[1] = (float)( ( *(float *)(tmpl + 0x1B8) - *(float *)(tmpl + 0x1B4) )
                         * frac + *(float *)(tmpl + 0x1B4) ); /* 0x0049528C */
      rng3[2] = (float)( ( *(float *)(tmpl + 0x1C0) - *(float *)(tmpl + 0x1BC) )
                         * frac + *(float *)(tmpl + 0x1BC) ); /* 0x004952A9 */
      tmp3[0] = (float)( ( *(float *)(tmpl + 0x1C8) - *(float *)(tmpl + 0x1C4) )
                         * frac + *(float *)(tmpl + 0x1C4) ); /* 0x004952C6 */
      tmp3[1] = (float)( ( *(float *)(tmpl + 0x1D0) - *(float *)(tmpl + 0x1CC) )
                         * frac + *(float *)(tmpl + 0x1CC) ); /* 0x004952E3 */
      tmp3[2] = (float)( ( *(float *)(tmpl + 0x1D8) - *(float *)(tmpl + 0x1D4) )
                         * frac + *(float *)(tmpl + 0x1D4) ); /* 0x00495300 */
    }
    else
    {
      if ( *(float *)(tmpl + 0x1AC) != *(float *)(tmpl + 0x1B0) )   /* 0x00495323 */
        *(float *)&d6 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1AC),
                                              *(float *)(tmpl + 0x1B0) ); /* 0x00495332 */
      else
        d6 = *(int *)(tmpl + 0x1AC);
      rng3[0] = *(float *)&d6;                                /* 0x00495343 */

      if ( *(float *)(tmpl + 0x1B4) == *(float *)(tmpl + 0x1B8) )   /* 0x00495355 */
        d6 = *(int *)(tmpl + 0x1B4);                          /* 0x00495357 */
      else
        *(float *)&d6 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1B4),
                                              *(float *)(tmpl + 0x1B8) ); /* 0x00495372 */
      rng3[1] = *(float *)&d6;                                /* 0x00495383 */

      if ( *(float *)(tmpl + 0x1BC) == *(float *)(tmpl + 0x1C0) )   /* 0x00495395 */
        d6 = *(int *)(tmpl + 0x1BC);                          /* 0x00495397 */
      else
        *(float *)&d6 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1BC),
                                              *(float *)(tmpl + 0x1C0) ); /* 0x004953B2 */
      rng3[2] = *(float *)&d6;                                /* 0x004953C3 */

      if ( *(float *)(tmpl + 0x1C4) != *(float *)(tmpl + 0x1C8) )   /* 0x004953DB */
        *(float *)&d6 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1C4),
                                              *(float *)(tmpl + 0x1C8) ); /* 0x004953EA */
      else
        d6 = *(int *)(tmpl + 0x1C4);
      tmp3[0] = *(float *)&d6;                                /* 0x004953FB */

      if ( *(float *)(tmpl + 0x1CC) == *(float *)(tmpl + 0x1D0) )   /* 0x0049540D */
        d6 = *(int *)(tmpl + 0x1CC);                          /* 0x0049540F */
      else
        *(float *)&d6 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1CC),
                                              *(float *)(tmpl + 0x1D0) ); /* 0x0049542A */
      tmp3[1] = *(float *)&d6;                                /* 0x0049543B */

      if ( *(float *)(tmpl + 0x1D4) == *(float *)(tmpl + 0x1D8) )   /* 0x0049544D */
        d6 = *(int *)(tmpl + 0x1D4);                          /* 0x0049544F */
      else
        *(float *)&d6 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1D4),
                                              *(float *)(tmpl + 0x1D8) ); /* 0x0049546A */
      tmp3[2] = *(float *)&d6;                                /* 0x00495475 */
    }
  }

  if ( ( *(int *)(tmpl + 0xB8) & 0x200000 ) != 0
       && bolt && *bolt >= 0 )                                /* 0x00495496 */
  {
    v132 = 0;
    bf = *CFxBoltFrame__Acquire( &v132, bolt );               /* 0x004954A1 */
    if ( bf )                                                 /* 0x004954A5 */
    {
      bfKeep = bf;
      ++*bf;                                                  /* 0x004954AC  refcount at +0 */
      boltFrame = (char *)bf;                                 /* 0x004954AE */
    }
    else
    {
      bfKeep = (int *)boltFrame;                              /* 0x004954B4 */
    }
    if ( v132 )                                               /* 0x004954BE */
      CFxBoltFrame__Release( (char *)v132 );                  /* 0x004954C0 */
    if ( !bfKeep )                                            /* 0x004954C7 */
      return;                                                 /* -> 0x00497085 */
    if ( !sub_48D770( bfKeep ) )                              /* 0x004954CD */
    {
      CFxBoltFrame__Release( (char *)bfKeep );                /* -> 0x00497080 */
      return;
    }
  }

  switch ( *(int *)(tmpl + 0x48) )
  {
    case 1:
      /* life  +0x5C */
      if ( *(float *)(tmpl + 0x5C) == *(float *)(tmpl + 0x60) )    /* 0x00495503 */
        life = *(int *)(tmpl + 0x5C);                              /* 0x00495508 */
      else
        *(float *)&life = Com_RandFloatRange_m( *(float *)(tmpl + 0x5C),
                                                *(float *)(tmpl + 0x60) );   /* 0x0049551B */

      /* +0x254 */
      if ( *(float *)(tmpl + 0x254) == *(float *)(tmpl + 0x258) )  /* 0x00495533 */
        r254 = *(int *)(tmpl + 0x254);                             /* 0x0049553B */
      else
        *(float *)&r254 = Com_RandFloatRange_m( *(float *)(tmpl + 0x254),
                                                *(float *)(tmpl + 0x258) );  /* 0x00495554 */

      if ( *(float *)(tmpl + 0x12C) == *(float *)(tmpl + 0x130) )  /* 0x0049556C */
        r12C = *(int *)(tmpl + 0x12C);                             /* 0x00495574 */
      else
        *(float *)&r12C = Com_RandFloatRange_m( *(float *)(tmpl + 0x12C),
                                                *(float *)(tmpl + 0x130) );  /* 0x0049558D */
      if ( *(float *)(tmpl + 0x124) == *(float *)(tmpl + 0x128) )  /* 0x004955A5 */
        r124 = *(int *)(tmpl + 0x124);                             /* 0x004955AD */
      else
        *(float *)&r124 = Com_RandFloatRange_m( *(float *)(tmpl + 0x124),
                                                *(float *)(tmpl + 0x128) );  /* 0x004955C6 */

      /* +0x1DC */
      if ( *(float *)(tmpl + 0x1DC) == *(float *)(tmpl + 0x1E0) )  /* 0x004955DE */
        r1DC = *(int *)(tmpl + 0x1DC);                             /* 0x004955E6 */
      else
        *(float *)&r1DC = Com_RandFloatRange_m( *(float *)(tmpl + 0x1DC),
                                                *(float *)(tmpl + 0x1E0) );  /* 0x004955FF */

      /* vector range +0x1E4, high component first */
      if ( *(float *)(tmpl + 0x1F4) == *(float *)(tmpl + 0x1F8) )  /* 0x00495617 */
        v1E4_2 = *(int *)(tmpl + 0x1F4);                           /* 0x0049561F */
      else
        *(float *)&v1E4_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1F4),
                                                  *(float *)(tmpl + 0x1F8) );/* 0x00495638 */
      if ( *(float *)(tmpl + 0x1EC) == *(float *)(tmpl + 0x1F0) )  /* 0x00495650 */
        v1E4_1 = *(int *)(tmpl + 0x1EC);                           /* 0x00495658 */
      else
        *(float *)&v1E4_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1EC),
                                                  *(float *)(tmpl + 0x1F0) );/* 0x00495671 */
      if ( *(float *)(tmpl + 0x1E4) == *(float *)(tmpl + 0x1E8) )  /* 0x00495689 */
        v1E4_0 = *(int *)(tmpl + 0x1E4);                           /* 0x00495691 */
      else
        *(float *)&v1E4_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1E4),
                                                  *(float *)(tmpl + 0x1E8) );/* 0x004956AA */

      /* vector range +0x214, high component first */
      if ( *(float *)(tmpl + 0x224) == *(float *)(tmpl + 0x228) )  /* 0x004956C2 */
        v214_2 = *(int *)(tmpl + 0x224);                           /* 0x004956CA */
      else
        *(float *)&v214_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x224),
                                                  *(float *)(tmpl + 0x228) );/* 0x004956E3 */
      if ( *(float *)(tmpl + 0x21C) == *(float *)(tmpl + 0x220) )  /* 0x004956FB */
        v214_1 = *(int *)(tmpl + 0x21C);                           /* 0x00495703 */
      else
        *(float *)&v214_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x21C),
                                                  *(float *)(tmpl + 0x220) );/* 0x0049571C */
      if ( *(float *)(tmpl + 0x214) == *(float *)(tmpl + 0x218) )  /* 0x00495734 */
        v214_0 = *(int *)(tmpl + 0x214);                           /* 0x0049573C */
      else
        *(float *)&v214_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x214),
                                                  *(float *)(tmpl + 0x218) );/* 0x00495755 */

      /* vector range +0x1FC, high component first */
      if ( *(float *)(tmpl + 0x20C) == *(float *)(tmpl + 0x210) )  /* 0x0049576D */
        v1FC_2 = *(int *)(tmpl + 0x20C);                           /* 0x0049576F */
      else
        *(float *)&v1FC_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x20C),
                                                  *(float *)(tmpl + 0x210) );/* 0x0049578A */
      if ( *(float *)(tmpl + 0x204) == *(float *)(tmpl + 0x208) )  /* 0x004957A6 */
        v1FC_1 = *(int *)(tmpl + 0x204);                           /* 0x004957A8 */
      else
        *(float *)&v1FC_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x204),
                                                  *(float *)(tmpl + 0x208) );/* 0x004957C3 */
      if ( *(float *)(tmpl + 0x1FC) == *(float *)(tmpl + 0x200) )  /* 0x004957DF */
        v1FC_0 = *(int *)(tmpl + 0x1FC);                           /* 0x004957E1 */
      else
        *(float *)&v1FC_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1FC),
                                                  *(float *)(tmpl + 0x200) );/* 0x004957FC */

      flags  = *(int *)(tmpl + 0xB8);                              /* pushed 0x0049580D */
      pick68 = sub_492780( tmpl + 0x68 );                          /* 0x00495811 */
      lifeI  = (int)*(float *)&life; /* 0x0049581B */
      pick78 = sub_492780( tmpl + 0x78 );                          /* 0x00495824 */
      pick88 = sub_492780( tmpl + 0x88 );                          /* 0x00495830 */

      FX_AddParticle( (int)start,                                  /* 0x004958B0 */
                      &boltFrame,
                      velocity,
                      accel,
                      *(char *)(tmpl + 0xC0),
                      v1FC_0, v1FC_1, *(float *)&v1FC_2,
                      v214_0, v214_1, *(float *)&v214_2,
                      v1E4_0, v1E4_1, *(float *)&v1E4_2,
                      rng3, tmp3, *(float *)&r1DC,
                      r124, r12C,
                      (_DWORD *)(tmpl + 0xC4),
                      (_DWORD *)(tmpl + 0xD0),
                      r254,
                      pick88, pick78, lifeI, pick68, flags );
      goto LABEL_501;                                              /* 0x004958B8 */

    case 2:
      /* life  +0x5C */
      if ( *(float *)(tmpl + 0x5C) == *(float *)(tmpl + 0x60) )    /* 0x004958C8 */
        life = *(int *)(tmpl + 0x5C);                              /* 0x004958CD */
      else
        *(float *)&life = Com_RandFloatRange_m( *(float *)(tmpl + 0x5C),
                                                *(float *)(tmpl + 0x60) );   /* 0x004958E0 */

      /* +0x1DC */
      if ( *(float *)(tmpl + 0x1DC) == *(float *)(tmpl + 0x1E0) )  /* 0x004958F8 */
        r1DC = *(int *)(tmpl + 0x1DC);                             /* 0x00495900 */
      else
        *(float *)&r1DC = Com_RandFloatRange_m( *(float *)(tmpl + 0x1DC),
                                                *(float *)(tmpl + 0x1E0) );  /* 0x00495919 */

      /* vector range +0x1E4, high component first */
      if ( *(float *)(tmpl + 0x1F4) == *(float *)(tmpl + 0x1F8) )  /* 0x00495931 */
        v1E4_2 = *(int *)(tmpl + 0x1F4);                           /* 0x00495939 */
      else
        *(float *)&v1E4_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1F4),
                                                  *(float *)(tmpl + 0x1F8) );/* 0x00495952 */
      if ( *(float *)(tmpl + 0x1EC) == *(float *)(tmpl + 0x1F0) )  /* 0x0049596A */
        v1E4_1 = *(int *)(tmpl + 0x1EC);                           /* 0x00495972 */
      else
        *(float *)&v1E4_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1EC),
                                                  *(float *)(tmpl + 0x1F0) );/* 0x0049598B */
      if ( *(float *)(tmpl + 0x1E4) == *(float *)(tmpl + 0x1E8) )  /* 0x004959A3 */
        v1E4_0 = *(int *)(tmpl + 0x1E4);                           /* 0x004959AB */
      else
        *(float *)&v1E4_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1E4),
                                                  *(float *)(tmpl + 0x1E8) );/* 0x004959C4 */

      /* vector range +0x1FC, high component first */
      if ( *(float *)(tmpl + 0x20C) == *(float *)(tmpl + 0x210) )  /* 0x004959DC */
        v1FC_2 = *(int *)(tmpl + 0x20C);                           /* 0x004959DE */
      else
        *(float *)&v1FC_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x20C),
                                                  *(float *)(tmpl + 0x210) );/* 0x004959F9 */
      if ( *(float *)(tmpl + 0x204) == *(float *)(tmpl + 0x208) )  /* 0x00495A15 */
        v1FC_1 = *(int *)(tmpl + 0x204);                           /* 0x00495A17 */
      else
        *(float *)&v1FC_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x204),
                                                  *(float *)(tmpl + 0x208) );/* 0x00495A32 */
      if ( *(float *)(tmpl + 0x1FC) == *(float *)(tmpl + 0x200) )  /* 0x00495A4E */
        v1FC_0 = *(int *)(tmpl + 0x1FC);                           /* 0x00495A50 */
      else
        *(float *)&v1FC_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1FC),
                                                  *(float *)(tmpl + 0x200) );/* 0x00495A6B */

      flags  = *(int *)(tmpl + 0xB8);                              /* pushed 0x00495A7C */
      pick68 = sub_492780( tmpl + 0x68 );                          /* 0x00495A80 */
      lifeI  = (int)*(float *)&life; /* 0x00495A8A */

      FX_AddLine( (int)start,                                      /* 0x00495ACD */
                  &boltFrame,
                  spawnOrigin,
                  v1FC_0, v1FC_1, *(float *)&v1FC_2,
                  v1E4_0, v1E4_1, *(float *)&v1E4_2,
                  rng3, tmp3,
                  *(float *)&r1DC,
                  lifeI, pick68, flags );
      goto LABEL_501;                                              /* 0x00495AD5 */

    case 3:
      /* life  +0x5C -> var_11C */
      if ( *(float *)(tmpl + 0x5C) == *(float *)(tmpl + 0x60) )   /* 0x00495AE5 */
        life = *(int *)(tmpl + 0x5C);                             /* 0x00495AE7 */
      else
        *(float *)&life = Com_RandFloatRange_m( *(float *)(tmpl + 0x5C),
                                                *(float *)(tmpl + 0x60) );  /* 0x00495AF8 */

      /* +0x254 -> var_100 */
      if ( *(float *)(tmpl + 0x254) == *(float *)(tmpl + 0x258) ) /* 0x00495B15 */
        r254 = *(int *)(tmpl + 0x254);                            /* 0x00495B17 */
      else
        *(float *)&r254 = Com_RandFloatRange_m( *(float *)(tmpl + 0x254),
                                                *(float *)(tmpl + 0x258) );  /* 0x00495B31 */

      /* +0x1DC -> var_F4 */
      if ( *(float *)(tmpl + 0x1DC) == *(float *)(tmpl + 0x1E0) ) /* 0x00495B4E */
        r1DC = *(int *)(tmpl + 0x1DC);                            /* 0x00495B50 */
      else
        *(float *)&r1DC = Com_RandFloatRange_m( *(float *)(tmpl + 0x1DC),
                                                *(float *)(tmpl + 0x1E0) );  /* 0x00495B6A */

      /* vector range at +0x1E4, drawn HIGH COMPONENT FIRST */
      if ( *(float *)(tmpl + 0x1F4) == *(float *)(tmpl + 0x1F8) ) /* 0x00495B87 */
        v1E4_2 = *(int *)(tmpl + 0x1F4);
      else
        *(float *)&v1E4_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1F4),
                                                  *(float *)(tmpl + 0x1F8) );/* 0x00495BA8 */
      if ( *(float *)(tmpl + 0x1EC) == *(float *)(tmpl + 0x1F0) ) /* 0x00495BC0 */
        v1E4_1 = *(int *)(tmpl + 0x1EC);
      else
        *(float *)&v1E4_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1EC),
                                                  *(float *)(tmpl + 0x1F0) );/* 0x00495BE1 */
      if ( *(float *)(tmpl + 0x1E4) == *(float *)(tmpl + 0x1E8) ) /* 0x00495BF9 */
        v1E4_0 = *(int *)(tmpl + 0x1E4);
      else
        *(float *)&v1E4_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1E4),
                                                  *(float *)(tmpl + 0x1E8) );/* 0x00495C1A */

      /* vector range at +0x22C, drawn high component first */
      if ( *(float *)(tmpl + 0x23C) == *(float *)(tmpl + 0x240) ) /* 0x00495C32 */
        v22C_2 = *(int *)(tmpl + 0x23C);
      else
        *(float *)&v22C_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x23C),
                                                  *(float *)(tmpl + 0x240) );/* 0x00495C53 */
      if ( *(float *)(tmpl + 0x234) == *(float *)(tmpl + 0x238) ) /* 0x00495C6B */
        v22C_1 = *(int *)(tmpl + 0x234);
      else
        *(float *)&v22C_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x234),
                                                  *(float *)(tmpl + 0x238) );/* 0x00495C8C */
      if ( *(float *)(tmpl + 0x22C) == *(float *)(tmpl + 0x230) ) /* 0x00495CA4 */
        v22C_0 = *(int *)(tmpl + 0x22C);
      else
        *(float *)&v22C_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x22C),
                                                  *(float *)(tmpl + 0x230) );/* 0x00495CC5 */

      /* vector range at +0x1FC, drawn high component first */
      if ( *(float *)(tmpl + 0x20C) == *(float *)(tmpl + 0x210) ) /* 0x00495CDD */
        v1FC_2 = *(int *)(tmpl + 0x20C);
      else
        *(float *)&v1FC_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x20C),
                                                  *(float *)(tmpl + 0x210) );/* 0x00495CFA */
      if ( *(float *)(tmpl + 0x204) == *(float *)(tmpl + 0x208) ) /* 0x00495D16 */
        v1FC_1 = *(int *)(tmpl + 0x204);
      else
        *(float *)&v1FC_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x204),
                                                  *(float *)(tmpl + 0x208) );/* 0x00495D33 */
      if ( *(float *)(tmpl + 0x1FC) == *(float *)(tmpl + 0x200) ) /* 0x00495D4F */
        v1FC_0 = *(int *)(tmpl + 0x1FC);
      else
        *(float *)&v1FC_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1FC),
                                                  *(float *)(tmpl + 0x200) );/* 0x00495D6C */

      flags  = *(int *)(tmpl + 0xB8);                             /* pushed 0x00495D7D */
      pick68 = sub_492780( tmpl + 0x68 );                         /* 0x00495D81 */

      lifeI  = (int)*(float *)&life;                              /* 0x00495D8B */
      pick78 = sub_492780( tmpl + 0x78 );                         /* 0x00495D94 */
      pick88 = sub_492780( tmpl + 0x88 );                         /* 0x00495DA0 */

      FX_AddTail( (int)start,                                     /* 0x00495E0D */
                  &boltFrame,
                  velocity,
                  accel,
                  v1FC_0, v1FC_1, *(float *)&v1FC_2,
                  v22C_0, v22C_1, *(float *)&v22C_2,
                  v1E4_0, v1E4_1, *(float *)&v1E4_2,
                  rng3, tmp3, *(float *)&r1DC,
                  (_DWORD *)(tmpl + 0xC4),
                  (_DWORD *)(tmpl + 0xD0),
                  r254,
                  pick88, pick78, lifeI, pick68, flags );
      goto LABEL_501;                                             /* 0x00495E15 */

    case 4:
      /* life  +0x5C */
      if ( *(float *)(tmpl + 0x5C) == *(float *)(tmpl + 0x60) )    /* 0x00496080 */
        life = *(int *)(tmpl + 0x5C);                              /* 0x00496085 */
      else
        *(float *)&life = Com_RandFloatRange_m( *(float *)(tmpl + 0x5C),
                                                *(float *)(tmpl + 0x60) );   /* 0x00496098 */

      /* +0x1DC */
      if ( *(float *)(tmpl + 0x1DC) == *(float *)(tmpl + 0x1E0) )  /* 0x004960B8 */
        r1DC = *(int *)(tmpl + 0x1DC);
      else
        *(float *)&r1DC = Com_RandFloatRange_m( *(float *)(tmpl + 0x1DC),
                                                *(float *)(tmpl + 0x1E0) );

      /* vector range +0x1E4, high component first */
      if ( *(float *)(tmpl + 0x1F4) == *(float *)(tmpl + 0x1F8) )  /* 0x004960F1 */
        v1E4_2 = *(int *)(tmpl + 0x1F4);
      else
        *(float *)&v1E4_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1F4),
                                                  *(float *)(tmpl + 0x1F8) );
      if ( *(float *)(tmpl + 0x1EC) == *(float *)(tmpl + 0x1F0) )  /* 0x0049612A */
        v1E4_1 = *(int *)(tmpl + 0x1EC);
      else
        *(float *)&v1E4_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1EC),
                                                  *(float *)(tmpl + 0x1F0) );
      if ( *(float *)(tmpl + 0x1E4) == *(float *)(tmpl + 0x1E8) )  /* 0x00496163 */
        v1E4_0 = *(int *)(tmpl + 0x1E4);
      else
        *(float *)&v1E4_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1E4),
                                                  *(float *)(tmpl + 0x1E8) );

      /* vector range +0x22C, high component first */
      if ( *(float *)(tmpl + 0x23C) == *(float *)(tmpl + 0x240) )  /* 0x0049619C */
        v22C_2 = *(int *)(tmpl + 0x23C);
      else
        *(float *)&v22C_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x23C),
                                                  *(float *)(tmpl + 0x240) );
      if ( *(float *)(tmpl + 0x234) == *(float *)(tmpl + 0x238) )  /* 0x004961D5 */
        v22C_1 = *(int *)(tmpl + 0x234);
      else
        *(float *)&v22C_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x234),
                                                  *(float *)(tmpl + 0x238) );
      if ( *(float *)(tmpl + 0x22C) == *(float *)(tmpl + 0x230) )  /* 0x0049620E */
        v22C_0 = *(int *)(tmpl + 0x22C);
      else
        *(float *)&v22C_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x22C),
                                                  *(float *)(tmpl + 0x230) );

      /* vector range +0x214, high component first */
      if ( *(float *)(tmpl + 0x224) == *(float *)(tmpl + 0x228) )  /* 0x00496247 */
        v214_2 = *(int *)(tmpl + 0x224);
      else
        *(float *)&v214_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x224),
                                                  *(float *)(tmpl + 0x228) );
      if ( *(float *)(tmpl + 0x21C) == *(float *)(tmpl + 0x220) )  /* 0x00496280 */
        v214_1 = *(int *)(tmpl + 0x21C);
      else
        *(float *)&v214_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x21C),
                                                  *(float *)(tmpl + 0x220) );
      if ( *(float *)(tmpl + 0x214) == *(float *)(tmpl + 0x218) )  /* 0x004962B9 */
        v214_0 = *(int *)(tmpl + 0x214);
      else
        *(float *)&v214_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x214),
                                                  *(float *)(tmpl + 0x218) );

      if ( *(float *)(tmpl + 0x20C) == *(float *)(tmpl + 0x210) )  /* 0x004962EC */
        v1FC_2 = *(int *)(tmpl + 0x20C);
      else
        *(float *)&v1FC_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x20C),
                                                  *(float *)(tmpl + 0x210) );
      if ( *(float *)(tmpl + 0x204) == *(float *)(tmpl + 0x208) )  /* 0x00496325 */
        v1FC_1 = *(int *)(tmpl + 0x204);
      else
        *(float *)&v1FC_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x204),
                                                  *(float *)(tmpl + 0x208) );
      if ( *(float *)(tmpl + 0x1FC) == *(float *)(tmpl + 0x200) )  /* 0x0049635E */
        v1FC_0 = *(int *)(tmpl + 0x1FC);
      else
        *(float *)&v1FC_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1FC),
                                                  *(float *)(tmpl + 0x200) );  /* 0x00496374 */

      flags  = *(int *)(tmpl + 0xB8);                              /* pushed 0x0049638A */
      pick68 = sub_492780( tmpl + 0x68 );                          /* 0x0049638E */
      lifeI  = (int)*(float *)&life; /* 0x00496398 */

      FX_AddCylinder( (int)start,                                  /* 0x004963F9 */
                      &boltFrame,
                      &axis[0],
                      v1FC_0, v1FC_1, *(float *)&v1FC_2,
                      v214_0, v214_1, *(float *)&v214_2,
                      v22C_0, v22C_1, *(float *)&v22C_2,
                      v1E4_0, v1E4_1, *(float *)&v1E4_2,
                      rng3, tmp3, *(float *)&r1DC,
                      lifeI, pick68, flags );
      goto LABEL_501;                                              /* 0x00496401 */

    case 5:

      /* ---- +0x134 vector range, low component first here ---------------- */
      if ( *(float *)(tmpl + 0x134) == *(float *)(tmpl + 0x138) )  /* 0x00496417 */
        e134_0 = *(int *)(tmpl + 0x134);                           /* 0x0049641F */
      else
        *(float *)&e134_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x134),
                                                  *(float *)(tmpl + 0x138) );/* 0x00496438 */
      if ( *(float *)(tmpl + 0x13C) == *(float *)(tmpl + 0x140) )  /* 0x00496450 */
        e134_1 = *(int *)(tmpl + 0x13C);                           /* 0x00496458 */
      else
        *(float *)&e134_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x13C),
                                                  *(float *)(tmpl + 0x140) );/* 0x00496471 */
      if ( *(float *)(tmpl + 0x144) == *(float *)(tmpl + 0x148) )  /* 0x00496489 */
        e134_2 = *(int *)(tmpl + 0x144);                           /* 0x00496491 */
      else
        *(float *)&e134_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x144),
                                                  *(float *)(tmpl + 0x148) );/* 0x004964A5 */

      /* retail 0x004964B9 turns the effect axis into angles first. */
      vectoangles( &axis[0], endp );                               /* 0x004964B9 */
      sum3[0] = *(float *)&e134_0 + endp[0];                       /* 0x004964C6 */
      sum3[1] = *(float *)&e134_1 + endp[1];                       /* 0x004964D5 */
      sum3[2] = *(float *)&e134_2 + endp[2];                       /* 0x004964E4 */

      /* ---- +0x14C vector range, drawn straight into spawnOrigin --------- */
      if ( *(float *)(tmpl + 0x14C) != *(float *)(tmpl + 0x150) )  /* 0x00496502 */
        *(float *)&t14C = Com_RandFloatRange_m( *(float *)(tmpl + 0x14C),
                                                *(float *)(tmpl + 0x150) );  /* 0x00496511 */
      else
        t14C = *(int *)(tmpl + 0x14C);
      spawnOrigin[0] = *(float *)&t14C;                            /* 0x00496522 */

      if ( *(float *)(tmpl + 0x154) == *(float *)(tmpl + 0x158) )  /* 0x00496534 */
        t14C = *(int *)(tmpl + 0x154);                             /* 0x00496536 */
      else
        *(float *)&t14C = Com_RandFloatRange_m( *(float *)(tmpl + 0x154),
                                                *(float *)(tmpl + 0x158) );  /* 0x00496551 */
      spawnOrigin[1] = *(float *)&t14C;                            /* 0x00496562 */

      if ( *(float *)(tmpl + 0x15C) == *(float *)(tmpl + 0x160) )  /* 0x00496574 */
        t14C = *(int *)(tmpl + 0x15C);                             /* 0x00496576 */
      else
        *(float *)&t14C = Com_RandFloatRange_m( *(float *)(tmpl + 0x15C),
                                                *(float *)(tmpl + 0x160) );  /* 0x00496591 */
      spawnOrigin[2] = *(float *)&t14C;                            /* 0x0049659C */

      pick68 = sub_492780( tmpl + 0x68 );                          /* 0x004965A6 */

      if ( *(float *)(tmpl + 0x5C) == *(float *)(tmpl + 0x60) )    /* 0x004965BB */
        life = *(int *)(tmpl + 0x5C);                              /* 0x004965BD */
      else
        *(float *)&life = Com_RandFloatRange_m( *(float *)(tmpl + 0x5C),
                                                *(float *)(tmpl + 0x60) );   /* 0x004965CD */
      if ( *(float *)(tmpl + 0x1A4) == *(float *)(tmpl + 0x1A8) )  /* 0x004965EB */
        variance = *(int *)(tmpl + 0x1A4);                         /* 0x004965ED */
      else
        *(float *)&variance = Com_RandFloatRange_m( *(float *)(tmpl + 0x1A4),
                                                    *(float *)(tmpl + 0x1A8) );/* 0x00496600 */
      if ( *(float *)(tmpl + 0x19C) == *(float *)(tmpl + 0x1A0) )  /* 0x0049661E */
        density = *(int *)(tmpl + 0x19C);                          /* 0x00496620 */
      else
        *(float *)&density = Com_RandFloatRange_m( *(float *)(tmpl + 0x19C),
                                                   *(float *)(tmpl + 0x1A0) );/* 0x00496633 */
      if ( *(float *)(tmpl + 0x254) == *(float *)(tmpl + 0x258) )  /* 0x00496651 */
        r254 = *(int *)(tmpl + 0x254);                             /* 0x00496653 */
      else
        *(float *)&r254 = Com_RandFloatRange_m( *(float *)(tmpl + 0x254),
                                                *(float *)(tmpl + 0x258) );  /* 0x00496666 */
      if ( *(float *)(tmpl + 0x1DC) == *(float *)(tmpl + 0x1E0) )  /* 0x00496684 */
        r1DC = *(int *)(tmpl + 0x1DC);                             /* 0x00496686 */
      else
        *(float *)&r1DC = Com_RandFloatRange_m( *(float *)(tmpl + 0x1DC),
                                                *(float *)(tmpl + 0x1E0) );  /* 0x00496699 */

      /* vector range +0x1E4, high component first */
      if ( *(float *)(tmpl + 0x1F4) == *(float *)(tmpl + 0x1F8) )  /* 0x004966B7 */
        v1E4_2 = *(int *)(tmpl + 0x1F4);                           /* 0x004966B9 */
      else
        *(float *)&v1E4_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1F4),
                                                  *(float *)(tmpl + 0x1F8) );/* 0x004966CC */
      if ( *(float *)(tmpl + 0x1EC) == *(float *)(tmpl + 0x1F0) )  /* 0x004966EA */
        v1E4_1 = *(int *)(tmpl + 0x1EC);                           /* 0x004966EC */
      else
        *(float *)&v1E4_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1EC),
                                                  *(float *)(tmpl + 0x1F0) );/* 0x004966FF */
      if ( *(float *)(tmpl + 0x1E4) == *(float *)(tmpl + 0x1E8) )  /* 0x0049671D */
        v1E4_0 = *(int *)(tmpl + 0x1E4);                           /* 0x0049671F */
      else
        *(float *)&v1E4_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1E4),
                                                  *(float *)(tmpl + 0x1E8) );/* 0x00496732 */

      /* vector range +0x1FC, high component first */
      if ( *(float *)(tmpl + 0x20C) == *(float *)(tmpl + 0x210) )  /* 0x0049674A */
        v1FC_2 = *(int *)(tmpl + 0x20C);                           /* 0x0049674C */
      else
        *(float *)&v1FC_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x20C),
                                                  *(float *)(tmpl + 0x210) );/* 0x00496767 */
      if ( *(float *)(tmpl + 0x204) == *(float *)(tmpl + 0x208) )  /* 0x00496783 */
        v1FC_1 = *(int *)(tmpl + 0x204);                           /* 0x00496785 */
      else
        *(float *)&v1FC_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x204),
                                                  *(float *)(tmpl + 0x208) );/* 0x004967A0 */
      if ( *(float *)(tmpl + 0x1FC) == *(float *)(tmpl + 0x200) )  /* 0x004967BC */
        v1FC_0 = *(int *)(tmpl + 0x1FC);                           /* 0x004967BE */
      else
        *(float *)&v1FC_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1FC),
                                                  *(float *)(tmpl + 0x200) );/* 0x004967D9 */

      flags     = *(int *)(tmpl + 0xB8);                           /* 0x004967EE */
      lifeI     = (int)*(float *)&life; /* 0x004967F0 */
      pick98    = sub_492780( tmpl + 0x98 );                       /* 0x00496806 */
      pick78    = sub_492780( tmpl + 0x78 );                       /* 0x0049680F */
      pick88    = sub_492780( tmpl + 0x88 );                       /* 0x0049681B */

      FX_AddEmitter( (int)start,                                   /* 0x00496889 */
                     &boltFrame,
                     velocity,
                     accel,
                     v1FC_0, v1FC_1, *(float *)&v1FC_2,
                     v1E4_0, v1E4_1, *(float *)&v1E4_2,
                     rng3, tmp3, *(float *)&r1DC,
                     sum3,
                     spawnOrigin,
                     (_DWORD *)(tmpl + 0xC4),
                     (_DWORD *)(tmpl + 0xD0),
                     r254,
                     pick88, pick78, pick98,
                     density, variance,
                     lifeI, pick68, flags );
      goto LABEL_501;                                              /* 0x00496891 */

    case 9:
      /* life  +0x5C -> var_11C */
      if ( *(float *)(tmpl + 0x5C) == *(float *)(tmpl + 0x60) )    /* 0x00495E25 */
        life = *(int *)(tmpl + 0x5C);                              /* 0x00495E2A */
      else
        *(float *)&life = Com_RandFloatRange_m( *(float *)(tmpl + 0x5C),
                                                *(float *)(tmpl + 0x60) );   /* 0x00495E3D */

      /* +0x254 -> var_100 */
      if ( *(float *)(tmpl + 0x254) == *(float *)(tmpl + 0x258) )  /* 0x00495E55 */
        r254 = *(int *)(tmpl + 0x254);                             /* 0x00495E5D */
      else
        *(float *)&r254 = Com_RandFloatRange_m( *(float *)(tmpl + 0x254),
                                                *(float *)(tmpl + 0x258) );  /* 0x00495E76 */

      /* +0x1DC -> var_F4 */
      if ( *(float *)(tmpl + 0x1DC) == *(float *)(tmpl + 0x1E0) )  /* 0x00495E8E */
        r1DC = *(int *)(tmpl + 0x1DC);                             /* 0x00495E96 */
      else
        *(float *)&r1DC = Com_RandFloatRange_m( *(float *)(tmpl + 0x1DC),
                                                *(float *)(tmpl + 0x1E0) );  /* 0x00495EAF */

      /* vector range at +0x1E4, drawn high component first */
      if ( *(float *)(tmpl + 0x1F4) == *(float *)(tmpl + 0x1F8) )  /* 0x00495EC7 */
        v1E4_2 = *(int *)(tmpl + 0x1F4);                           /* 0x00495ECF */
      else
        *(float *)&v1E4_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1F4),
                                                  *(float *)(tmpl + 0x1F8) );/* 0x00495EE8 */
      if ( *(float *)(tmpl + 0x1EC) == *(float *)(tmpl + 0x1F0) )  /* 0x00495F00 */
        v1E4_1 = *(int *)(tmpl + 0x1EC);                           /* 0x00495F08 */
      else
        *(float *)&v1E4_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1EC),
                                                  *(float *)(tmpl + 0x1F0) );/* 0x00495F21 */
      if ( *(float *)(tmpl + 0x1E4) == *(float *)(tmpl + 0x1E8) )  /* 0x00495F39 */
        v1E4_0 = *(int *)(tmpl + 0x1E4);                           /* 0x00495F41 */
      else
        *(float *)&v1E4_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1E4),
                                                  *(float *)(tmpl + 0x1E8) );/* 0x00495F5A */

      /* vector range at +0x1FC, drawn high component first */
      if ( *(float *)(tmpl + 0x20C) == *(float *)(tmpl + 0x210) )  /* 0x00495F72 */
        v1FC_2 = *(int *)(tmpl + 0x20C);                           /* 0x00495F74 */
      else
        *(float *)&v1FC_2 = Com_RandFloatRange_m( *(float *)(tmpl + 0x20C),
                                                  *(float *)(tmpl + 0x210) );/* 0x00495F8F */
      if ( *(float *)(tmpl + 0x204) == *(float *)(tmpl + 0x208) )  /* 0x00495FAB */
        v1FC_1 = *(int *)(tmpl + 0x204);                           /* 0x00495FAD */
      else
        *(float *)&v1FC_1 = Com_RandFloatRange_m( *(float *)(tmpl + 0x204),
                                                  *(float *)(tmpl + 0x208) );/* 0x00495FC8 */
      if ( *(float *)(tmpl + 0x1FC) == *(float *)(tmpl + 0x200) )  /* 0x00495FE4 */
        v1FC_0 = *(int *)(tmpl + 0x1FC);                           /* 0x00495FE6 */
      else
        *(float *)&v1FC_0 = Com_RandFloatRange_m( *(float *)(tmpl + 0x1FC),
                                                  *(float *)(tmpl + 0x200) );/* 0x00496001 */

      flags  = *(int *)(tmpl + 0xB8);                              /* pushed 0x00496012 */
      pick68 = sub_492780( tmpl + 0x68 );                          /* 0x00496016 */
      lifeI  = (int)*(float *)&life; /* 0x00496020 */

      FX_AddElectricity( (int)start,                               /* 0x00496068 */
                         &boltFrame,
                         spawnOrigin,
                         v1FC_0, v1FC_1, *(float *)&v1FC_2,
                         v1E4_0, v1E4_1, *(float *)&v1E4_2,
                         rng3, tmp3, *(float *)&r1DC,
                         r254,
                         lifeI, pick68, flags );
      goto LABEL_501;                                              /* 0x00496070 */

    case 7:
        if ( *(float *)(tpl + 0x124) == *(float *)(tpl + 0x128) )
            r124 = *(int *)(tpl + 0x124);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x124),
                                  *(float *)(tpl + 0x128) );
            r124 = v136;
        }
        if ( *(float *)(tpl + 0x1E4) == *(float *)(tpl + 0x1E8) )
            r1E4 = *(int *)(tpl + 0x1E4);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1E4),
                                  *(float *)(tpl + 0x1E8) );
            r1E4 = v136;
        }
        if ( *(float *)(tpl + 0x1FC) == *(float *)(tpl + 0x200) )
            r1FC = *(int *)(tpl + 0x1FC);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1FC),
                                  *(float *)(tpl + 0x200) );
            r1FC = v136;
        }
        if ( *(float *)(tpl + 0x5C) == *(float *)(tpl + 0x60) )
            life = *(int *)(tpl + 0x5C);
        else
            *(float *)&life = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x5C),
                                  *(float *)(tpl + 0x60) );
        pick = sub_492780( (int)(tpl + 0x68) );
        VM_Call( cgvm, 14,
                 pick,
                 start,
                 axis,
                 r124,
                 *(int *)&rng3[0], *(int *)&rng3[1],
                 *(int *)&rng3[2],
                 r1E4,
                 1,
                 r1FC,
                 0,
                 (int)*(float *)&life );
        goto LABEL_501;
    case 8:
        if ( *(float *)(tpl + 0x5C) == *(float *)(tpl + 0x60) )
            life = *(int *)(tpl + 0x5C);
        else
            *(float *)&life = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x5C),
                                  *(float *)(tpl + 0x60) );
        if ( *(float *)(tpl + 0x254) == *(float *)(tpl + 0x258) )
            r254 = *(int *)(tpl + 0x254);
        else
            *(float *)&r254 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x254),
                                  *(float *)(tpl + 0x258) );
        if ( *(float *)(tpl + 0x12C) == *(float *)(tpl + 0x130) )
            r12C = *(int *)(tpl + 0x12C);
        else
            *(float *)&r12C = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x12C),
                                  *(float *)(tpl + 0x130) );
        if ( *(float *)(tpl + 0x124) == *(float *)(tpl + 0x128) )
            r124 = *(int *)(tpl + 0x124);
        else
            *(float *)&r124 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x124),
                                  *(float *)(tpl + 0x128) );
        if ( *(float *)(tpl + 0x1DC) == *(float *)(tpl + 0x1E0) )
            r1DC = *(int *)(tpl + 0x1DC);
        else
            *(float *)&r1DC = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1DC),
                                  *(float *)(tpl + 0x1E0) );
        if ( *(float *)(tpl + 0x1F4) == *(float *)(tpl + 0x1F8) )
            r1F4 = *(int *)(tpl + 0x1F4);
        else
            *(float *)&r1F4 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1F4),
                                  *(float *)(tpl + 0x1F8) );
        if ( *(float *)(tpl + 0x1EC) == *(float *)(tpl + 0x1F0) )
            r1EC = *(int *)(tpl + 0x1EC);
        else
            *(float *)&r1EC = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1EC),
                                  *(float *)(tpl + 0x1F0) );
        if ( *(float *)(tpl + 0x1E4) == *(float *)(tpl + 0x1E8) )
            r1E4 = *(int *)(tpl + 0x1E4);
        else
            *(float *)&r1E4 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1E4),
                                  *(float *)(tpl + 0x1E8) );
        if ( *(float *)(tpl + 0x224) == *(float *)(tpl + 0x228) )
            r224 = *(int *)(tpl + 0x224);
        else
            *(float *)&r224 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x224),
                                  *(float *)(tpl + 0x228) );
        if ( *(float *)(tpl + 0x21C) == *(float *)(tpl + 0x220) )
            r21C = *(int *)(tpl + 0x21C);
        else
            *(float *)&r21C = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x21C),
                                  *(float *)(tpl + 0x220) );
        if ( *(float *)(tpl + 0x214) == *(float *)(tpl + 0x218) )
            r214 = *(int *)(tpl + 0x214);
        else
            *(float *)&r214 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x214),
                                  *(float *)(tpl + 0x218) );
        if ( *(float *)(tpl + 0x20C) == *(float *)(tpl + 0x210) )
            r20C = *(int *)(tpl + 0x20C);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x20C),
                                  *(float *)(tpl + 0x210) );
            r20C = v136;
        }
        if ( *(float *)(tpl + 0x204) == *(float *)(tpl + 0x208) )
            r204 = *(int *)(tpl + 0x204);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x204),
                                  *(float *)(tpl + 0x208) );
            r204 = v136;
        }
        if ( *(float *)(tpl + 0x1FC) == *(float *)(tpl + 0x200) )
            r1FC = *(int *)(tpl + 0x1FC);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1FC),
                                  *(float *)(tpl + 0x200) );
            r1FC = v136;
        }
        FX_AddOrientedParticle(
            (int)axis,
            &boltFrame,
            (int)start,
            velocity,
            accel,
            (char)*(unsigned char *)(tpl + 0xC0),
            r1FC, r204, *(float *)&r20C,
            r214, r21C, *(float *)&r224,
            r1E4, r1EC, *(float *)&r1F4,
            (_DWORD *)rng3,
            (_DWORD *)tmp3,
            *(float *)&r1DC,
            r124, r12C,
            (_DWORD *)(tpl + 0xC4),
            (_DWORD *)(tpl + 0xD0),
            r254,
            sub_492780( (int)(tpl + 0x88) ),
            sub_492780( (int)(tpl + 0x78) ),
            (int)*(float *)&life,
            sub_492780( (int)(tpl + 0x68) ),
            *(unsigned int *)(tpl + 0xB8) );
        goto LABEL_501;
    case 6:
        SFxHelper__PlaySound( sub_492780( (int)(tpl + 0x68) ),
                              (int)&theFxHelper,
                              start,
                              1023 );
        goto LABEL_501;
    case 10:
        CFxScheduler__PlayEffect_id_axis(
            self,
            sub_492780( (int)(tpl + 0xA8) ),
            start,
            (int *)axis,
            boltFrame ? (int *)(boltFrame + 0x3C) : (int *)0 );
        goto LABEL_501;
    case 11:
        if ( *(float *)(tpl + 0x5C) == *(float *)(tpl + 0x60) )
            life = *(int *)(tpl + 0x5C);
        else
            *(float *)&life = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x5C),
                                  *(float *)(tpl + 0x60) );
        if ( *(float *)(tpl + 0x1DC) == *(float *)(tpl + 0x1E0) )
            r1DC = *(int *)(tpl + 0x1DC);
        else
            *(float *)&r1DC = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1DC),
                                  *(float *)(tpl + 0x1E0) );
        if ( *(float *)(tpl + 0x20C) == *(float *)(tpl + 0x210) )
            r20C = *(int *)(tpl + 0x20C);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x20C),
                                  *(float *)(tpl + 0x210) );
            r20C = v136;
        }
        if ( *(float *)(tpl + 0x204) == *(float *)(tpl + 0x208) )
            r204 = *(int *)(tpl + 0x204);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x204),
                                  *(float *)(tpl + 0x208) );
            r204 = v136;
        }
        if ( *(float *)(tpl + 0x1FC) == *(float *)(tpl + 0x200) )
            r1FC = *(int *)(tpl + 0x1FC);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1FC),
                                  *(float *)(tpl + 0x200) );
            r1FC = v136;
        }
        FX_AddLight( &boltFrame,
                     start,
                     r1FC,
                     r204,
                     *(float *)&r20C,
                     (_DWORD *)rng3,
                     (_DWORD *)tmp3,
                     *(float *)&r1DC,
                     (int)*(float *)&life,
                     *(int *)(tpl + 0xB8) );
        goto LABEL_501;
    case 12:
        if ( *(float *)(tpl + 0x5C) != *(float *)(tpl + 0x60) )
            Com_RandFloatRange_m( *(float *)(tpl + 0x5C),
                                  *(float *)(tpl + 0x60) );
        if ( *(float *)(tpl + 0x10C) != *(float *)(tpl + 0x110) )
            Com_RandFloatRange_m( *(float *)(tpl + 0x10C),
                                  *(float *)(tpl + 0x110) );
        if ( *(float *)(tpl + 0x254) != *(float *)(tpl + 0x258) )
            Com_RandFloatRange_m( *(float *)(tpl + 0x254),
                                  *(float *)(tpl + 0x258) );
        break;
    case 13:
        if ( *(float *)(tpl + 0x5C) == *(float *)(tpl + 0x60) )
            life = *(int *)(tpl + 0x5C);
        else
            *(float *)&life = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x5C),
                                  *(float *)(tpl + 0x60) );
        if ( *(float *)(tpl + 0x1DC) == *(float *)(tpl + 0x1E0) )
            r1DC = *(int *)(tpl + 0x1DC);
        else
        {
            *(float *)&v136 = (float)Com_RandFloatRange_m(
                                  *(float *)(tpl + 0x1DC),
                                  *(float *)(tpl + 0x1E0) );
            r1DC = v136;
        }
        FX_AddFlash(
            (_DWORD *)rng3,
            (_DWORD *)start,
            (_DWORD *)tmp3,
            *(float *)&r1DC,
            (int)*(float *)&life,
            sub_492780( (int)(tpl + 0x68) ),
            *(int *)(tpl + 0xB8) );
        break;

    default:
      break;
  }

LABEL_501:
  v6 = tmpl;                                                 /* 0x00497047 */

  if ( *(unsigned char *)(v6 + 0) != 0 )                     /* 0x0049704E */
  {
    --*(int *)(v6 + 4);                                      /* 0x00497054 */
    if ( *(int *)(tmpl + 4) <= 0 )                           /* 0x00497061 */
    {
      CPrimitiveTemplate__dtor( tmpl );                      /* 0x00497067 */
      sub_4976D0( tmpl, (int)&dword_140758C );               /* 0x00497073 */
    }
  }

  if ( boltFrame )                                           /* 0x00497078 */
    CFxBoltFrame__Release( boltFrame );                      /* 0x00497080 */
}

#if 0
void __cdecl CFxScheduler__CreateEffect(_DWORD *this, int a2, int *a3, float *a4, int a5, int a6)
{
  int v6;
  float v7;
  int v8;
  float v9;
  float *v10;
  int v11;
  double v12;
  int v13;
  int v14;
  int v15;
  double v16;
  int v17;
  double v18;
  int v19;
  double v20;
  int v21;
  double v22;
  int v23;
  int v24;
  int v25;
  int v26;
  double v27;
  int v28;
  int v29;
  int v30;
  bool v31; // zf
  int v32;
  int v33;
  int v34;
  int v35;
  double v36;
  int v37;
  double v38;
  int v39;
  int v40;
  double v41;
  float v42;
  double v43;
  float v44;
  double v45;
  int v46;
  double v47;
  float v48;
  double v49;
  float v50;
  char *v51;
  char *v52;
  char *v53;
  int v54;
  int v55;
  int v56;
  int v57;
  int v58;
  int v59;
  int v60;
  int v61;
  int v62;
  int v63;
  int v64;
  int v65;
  int v66;
  int v67;
  int v68;
  int v69;
  int v70;
  int v71;
  int v72;
  int v73;
  double v74;
  int v75;
  double v76;
  int v77;
  double v78;
  int v79;
  int v80;
  int v81;
  int v82;
  int v83;
  int v84;
  int v85;
  int v86;
  int v87;
  int v88;
  int v89;
  int v90;
  int v91;
  int v92;
  int v93;
  char *v94;
  int v95;
  int v96;
  int v97;
  int v98;
  int v99;
  int v100;
  int v101;
  int v102;
  float v103;
  int v104;
  float v105;
  int v106;
  int v107;
  int v108;
  int v109;
  int v110;
  int v111;
  int v112;
  int v113;
  int v114;
  int v115;
  int v116;
  int v117;
  int v118;
  float v119;
  float v120;
  int v121;
  int v122;
  int v123;
  int v124;
  int v125;
  int v126;
  int v127;
  int v128;
  int *v129;
  int v130;
  int v131;
  int v132; // [esp+18h] [ebp-11Ch] BYREF
  clipHandle_t model; // [esp+1Ch] [ebp-118h] BYREF
  int brushmask; // [esp+20h] [ebp-114h] BYREF
  int capsule;
  int v136; // [esp+28h] [ebp-10Ch] BYREF
  float v137;
  float v138;
  float v139;
  int v140;
  int v141; // [esp+3Ch] [ebp-F8h] BYREF
  int v142;
  int v143;
  int v144;
  int v145;
  float start[3]; // [esp+50h] [ebp-E4h] BYREF
  char *v147; // [esp+5Ch] [ebp-D8h] BYREF
  int v148; // [esp+60h] [ebp-D4h] BYREF
  float v149;
  float v150;
  int v151; // [esp+6Ch] [ebp-C8h] BYREF
  float v152;
  float v153;
  float v154; // [esp+78h] [ebp-BCh] BYREF
  float v155;
  float v156;
  int v157; // [esp+84h] [ebp-B0h] BYREF
  float v158;
  float v159;
  int v160; // [esp+90h] [ebp-A4h] BYREF
  float v161;
  float v162;
  _DWORD *v163;
  int v164; // [esp+A0h] [ebp-94h] BYREF
  float v165;
  float v166;
  int v167; // [esp+ACh] [ebp-88h] BYREF
  float v168;
  int v169;
  int v170; // [esp+B8h] [ebp-7Ch] BYREF
  float v171;
  float v172;
  int v173[3]; // [esp+C4h] [ebp-70h] BYREF
  trace_t results; // [esp+D0h] [ebp-64h] BYREF
  int v175[3]; // [esp+100h] [ebp-34h] BYREF
  char v176[12]; // [esp+10Ch] [ebp-28h] BYREF
  float v177[7]; // [esp+118h] [ebp-1Ch] BYREF

  v163 = this;
  v147 = 0;
  v6 = a2;
  v177[6] = 0.0;
  v148 = *(int *)a5;
  v149 = *(float *)(a5 + 4);
  v150 = *(float *)(a5 + 8);
  v151 = *(int *)(a5 + 12);
  v152 = *(float *)(a5 + 16);
  v153 = *(float *)(a5 + 20);
  v7 = *(float *)(a5 + 24);
  v154 = v7;
  v155 = *(float *)(a5 + 28);
  v156 = *(float *)(a5 + 32);
  if ( (*(_DWORD *)(a2 + 188) & 0x1000) != 0 )
  {
    *(float *)&v132 = (float)rand();
    v119 = *(float *)&v132 / 32768.0 * 360.0;
    RotatePointAroundVector((float *)&v148, (float *)&v151, (float *)(a5 + 12), v119);
    v6 = a2;
    v154 = v149 * v153 - v150 * v152;
    v155 = v150 * *(float *)&v151 - *(float *)&v148 * v153;
    v156 = *(float *)&v148 * v152 - v149 * *(float *)&v151;
  }
  if ( (*(_DWORD *)(v6 + 188) & 0x100) != 0 )
  {
    if ( (*(float *)(v6 + 220) == *(float *)(v6 + 224)) | __UNORDERED__(*(float *)(v6 + 220), *(float *)(v6 + 224)) )
    {
      v8 = *(_DWORD *)(v6 + 220);
    }
    else
    {
      *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 220), *(float *)(v6 + 224));
      v8 = v132;
    }
    LODWORD(start[0]) = v8;
    v9 = *(float *)(v6 + 228);
    if ( !((v9 == *(float *)(v6 + 232)) | __UNORDERED__(v9, *(float *)(v6 + 232))) )
    {
      *(float *)&v132 = Com_RandFloatRange_m(v9, *(float *)(v6 + 232));
      v9 = *(float *)&v132;
    }
    start[1] = v9;
    if ( (*(float *)(v6 + 236) == *(float *)(v6 + 240)) | __UNORDERED__(*(float *)(v6 + 236), *(float *)(v6 + 240)) )
    {
      start[2] = *(float *)(v6 + 236);
    }
    else
    {
      *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 236), *(float *)(v6 + 240));
      LODWORD(start[2]) = v132;
    }
  }
  else
  {
    if ( (*(float *)(v6 + 236) == *(float *)(v6 + 240)) | __UNORDERED__(*(float *)(v6 + 236), *(float *)(v6 + 240)) )
      capsule = *(int *)(v6 + 236);
    else
      *(float *)&capsule = Com_RandFloatRange_m(*(float *)(v6 + 236), *(float *)(v6 + 240));
    if ( (*(float *)(v6 + 228) == *(float *)(v6 + 232)) | __UNORDERED__(*(float *)(v6 + 228), *(float *)(v6 + 232)) )
      brushmask = *(int *)(v6 + 228);
    else
      *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 228), *(float *)(v6 + 232));
    if ( (*(float *)(v6 + 220) == *(float *)(v6 + 224)) | __UNORDERED__(*(float *)(v6 + 220), *(float *)(v6 + 224)) )
      model = *(clipHandle_t *)(v6 + 220);
    else
      *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 220), *(float *)(v6 + 224));
    start[0] = *(float *)&model * *(float *)&v148 + *(float *)&brushmask * *(float *)&v151 + *(float *)&capsule * v154;
    start[1] = *(float *)&model * v149 + *(float *)&brushmask * v152 + *(float *)&capsule * v155;
    start[2] = *(float *)&model * v150 + *(float *)&brushmask * v153 + *(float *)&capsule * v156;
  }
  v10 = a4;
  start[0] = start[0] + *a4;
  start[1] = start[1] + a4[1];
  start[2] = start[2] + a4[2];
  v11 = *(_DWORD *)(v6 + 188);
  if ( (v11 & 1) != 0 )
  {
    *(float *)&v132 = (float)rand();
    v139 = COERCE_FLOAT(&brushmask);
    *(float *)&v142 = *(float *)&v132 / 32768.0 * 360.0 * 3.1415927 / 180.0;
    *(float *)&brushmask = cos(*(float *)&v142);
    *(float *)&capsule = sin(*(float *)&v142);
    *(float *)&v132 = (float)rand();
    v139 = COERCE_FLOAT(&model);
    v12 = *(float *)&v132;
    *(float *)&v132 = COERCE_FLOAT(&v141);
    *(float *)&v142 = v12 / 32768.0 * 180.0 * 3.1415927 / 180.0;
    *(float *)&model = cos(*(float *)&v142);
    *(float *)&v141 = sin(*(float *)&v142);
    v6 = a2;
    if ( (*(float *)(a2 + 268) == *(float *)(a2 + 272)) | __UNORDERED__(*(float *)(a2 + 268), *(float *)(a2 + 272)) )
      v143 = *(int *)(a2 + 268);
    else
      *(float *)&v143 = Com_RandFloatRange_m(*(float *)(a2 + 268), *(float *)(a2 + 272));
    if ( (*(float *)(a2 + 276) == *(float *)(a2 + 280)) | __UNORDERED__(*(float *)(a2 + 276), *(float *)(a2 + 280)) )
    {
      v7 = *(float *)(a2 + 276);
      *(float *)&v140 = v7;
    }
    else
    {
      *(float *)&v140 = Com_RandFloatRange_m(*(float *)(a2 + 276), *(float *)(a2 + 280));
    }
    *(float *)&v143 = *(float *)&v143 * *(float *)&v141;
    *(float *)&v136 = *(float *)&v143 * *(float *)&capsule;
    v137 = *(float *)&v143 * *(float *)&brushmask;
    v138 = *(float *)&v140 * *(float *)&model;
    start[0] = *(float *)&v136 + start[0];
    start[1] = v137 + start[1];
    start[2] = v138 + start[2];
    if ( (*(_BYTE *)(a2 + 188) & 2) != 0 )
    {
      VectorNormalize2((float *)&v136, (float *)&v148);
      /* forward first */
      MakeNormalVectors( (const float *)&v148, (float *)&v151, (float *)&v154 );
    }
LABEL_42:
    v10 = a4;
    goto LABEL_43;
  }
  if ( (v11 & 4) != 0 )
  {
    if ( (*(float *)(v6 + 276) == *(float *)(v6 + 280)) | __UNORDERED__(*(float *)(v6 + 276), *(float *)(v6 + 280)) )
      v140 = *(int *)(v6 + 276);
    else
      *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 276), *(float *)(v6 + 280));
    *(float *)&v132 = (float)rand();
    *(float *)&v136 = (*(float *)&v132 / 32768.0 + *(float *)&v132 / 32768.0 - 1.0) * *(float *)&v140 * 0.5;
    if ( (*(float *)(a2 + 268) == *(float *)(a2 + 272)) | __UNORDERED__(*(float *)(a2 + 268), *(float *)(a2 + 272)) )
      v137 = *(float *)(a2 + 268);
    else
      v137 = Com_RandFloatRange_m(*(float *)(a2 + 268), *(float *)(a2 + 272));
    *(float *)&v157 = *(float *)&v151 * v137;
    v158 = v152 * v137;
    v159 = v153 * v137;
    *(float *)&v157 = *(float *)&v148 * *(float *)&v136 + *(float *)&v157;
    v158 = v149 * *(float *)&v136 + v158;
    v159 = v150 * *(float *)&v136 + v159;
    *(float *)&v132 = (float)rand();
    v120 = *(float *)&v132 / 32768.0 * 360.0;
    RotatePointAroundVector((float *)&v148, (float *)&v136, (float *)&v157, v120);
    v6 = a2;
    start[0] = *(float *)&v136 + start[0];
    start[1] = v137 + start[1];
    start[2] = v138 + start[2];
    if ( (*(_BYTE *)(a2 + 188) & 2) != 0 )
    {
      v158 = 0.0;
      v159 = 1.0;
      VectorNormalize2((float *)&v136, (float *)&v148);
      if ( v150 == 1.0 )
      {
        v158 = 1.0;
        v159 = 0.0;
      }
      *(float *)&v151 = v158 * v150 - v159 * v149;
      v152 = v159 * *(float *)&v148 - v150 * 0.0;
      v153 = v149 * 0.0 - v158 * *(float *)&v148;
      v154 = v149 * v153 - v150 * v152;
      v155 = v150 * *(float *)&v151 - *(float *)&v148 * v153;
      v156 = *(float *)&v148 * v152 - v149 * *(float *)&v151;
    }
    goto LABEL_42;
  }
LABEL_43:
  v13 = *(_DWORD *)(v6 + 72);
  if ( v13 == 1 || v13 == 8 || v13 == 3 || v13 == 5 )
  {
    v14 = *(_DWORD *)(v6 + 188);
    if ( (v14 & 0x400) != 0 )
    {
      if ( (*(float *)(v6 + 356) == *(float *)(v6 + 360)) | __UNORDERED__(*(float *)(v6 + 356), *(float *)(v6 + 360)) )
      {
        v15 = *(int *)(v6 + 356);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 356), *(float *)(v6 + 360));
        v15 = v132;
      }
      v16 = *(float *)(v6 + 364);
      v167 = v15;
      if ( (v16 == *(float *)(v6 + 368)) | __UNORDERED__(v16, *(float *)(v6 + 368)) )
      {
        v17 = *(int *)(v6 + 364);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 364), *(float *)(v6 + 368));
        v17 = v132;
      }
      v18 = *(float *)(v6 + 372);
      v168 = *(float *)&v17;
      if ( (v18 == *(float *)(v6 + 376)) | __UNORDERED__(v18, *(float *)(v6 + 376)) )
      {
        v169 = *(int *)(v6 + 372);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 372), *(float *)(v6 + 376));
        v169 = v132;
      }
    }
    else
    {
      if ( (*(float *)(v6 + 372) == *(float *)(v6 + 376)) | __UNORDERED__(*(float *)(v6 + 372), *(float *)(v6 + 376)) )
        model = *(clipHandle_t *)(v6 + 372);
      else
        *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 372), *(float *)(v6 + 376));
      if ( (*(float *)(v6 + 364) == *(float *)(v6 + 368)) | __UNORDERED__(*(float *)(v6 + 364), *(float *)(v6 + 368)) )
        capsule = *(int *)(v6 + 364);
      else
        *(float *)&capsule = Com_RandFloatRange_m(*(float *)(v6 + 364), *(float *)(v6 + 368));
      if ( (*(float *)(v6 + 356) == *(float *)(v6 + 360)) | __UNORDERED__(*(float *)(v6 + 356), *(float *)(v6 + 360)) )
        brushmask = *(int *)(v6 + 356);
      else
        *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 356), *(float *)(v6 + 360));
      *(float *)&v167 = *(float *)&capsule * *(float *)&v151
                      + *(float *)&model * v154
                      + *(float *)&brushmask * *(float *)&v148;
      v168 = *(float *)&capsule * v152 + *(float *)&model * v155 + *(float *)&brushmask * v149;
      *(float *)&v169 = *(float *)&model * v156 + *(float *)&brushmask * v150 + *(float *)&capsule * v153;
    }
    if ( (v14 & 0x10000) != 0 )
    {
      Com_Printf("FX_PORT wind TBD.\n");
      v10 = a4;
      v6 = a2;
    }
    if ( (*(_DWORD *)(v6 + 188) & 0x800) != 0 )
    {
      if ( (*(float *)(v6 + 380) == *(float *)(v6 + 384)) | __UNORDERED__(*(float *)(v6 + 380), *(float *)(v6 + 384)) )
      {
        v19 = *(int *)(v6 + 380);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 380), *(float *)(v6 + 384));
        v19 = v132;
      }
      v20 = *(float *)(v6 + 388);
      v170 = v19;
      if ( (v20 == *(float *)(v6 + 392)) | __UNORDERED__(v20, *(float *)(v6 + 392)) )
      {
        v21 = *(int *)(v6 + 388);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 388), *(float *)(v6 + 392));
        v21 = v132;
      }
      v22 = *(float *)(v6 + 396);
      v171 = *(float *)&v21;
      v23 = *(int *)(v6 + 396);
      if ( !((v22 == *(float *)(v6 + 400)) | __UNORDERED__(v22, *(float *)(v6 + 400))) )
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)&v23, *(float *)(v6 + 400));
        v23 = v132;
      }
      v172 = *(float *)&v23;
    }
    else
    {
      if ( (*(float *)(v6 + 396) == *(float *)(v6 + 400)) | __UNORDERED__(*(float *)(v6 + 396), *(float *)(v6 + 400)) )
        model = *(clipHandle_t *)(v6 + 396);
      else
        *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 396), *(float *)(v6 + 400));
      if ( (*(float *)(v6 + 388) == *(float *)(v6 + 392)) | __UNORDERED__(*(float *)(v6 + 388), *(float *)(v6 + 392)) )
        capsule = *(int *)(v6 + 388);
      else
        *(float *)&capsule = Com_RandFloatRange_m(*(float *)(v6 + 388), *(float *)(v6 + 392));
      if ( (*(float *)(v6 + 380) == *(float *)(v6 + 384)) | __UNORDERED__(*(float *)(v6 + 380), *(float *)(v6 + 384)) )
        brushmask = *(int *)(v6 + 380);
      else
        *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 380), *(float *)(v6 + 384));
      *(float *)&v170 = *(float *)&capsule * *(float *)&v151
                      + *(float *)&model * v154
                      + *(float *)&brushmask * *(float *)&v148;
      v171 = *(float *)&capsule * v152 + *(float *)&model * v155 + *(float *)&brushmask * v149;
      v172 = *(float *)&model * v156 + *(float *)&brushmask * v150 + *(float *)&capsule * v153;
    }
    if ( (*(float *)(v6 + 404) == *(float *)(v6 + 408)) | __UNORDERED__(*(float *)(v6 + 404), *(float *)(v6 + 408)) )
    {
      v7 = *(float *)(v6 + 404);
      *(float *)&v140 = v7;
    }
    else
    {
      *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 404), *(float *)(v6 + 408));
    }
    v172 = *(float *)&v140 + v172;
    if ( a6 > 0 )
    {
      v24 = 0;
      *(float *)&v132 = (float)a6;
      *(float *)&v141 = *(float *)&v132 * 0.001;
      *(float *)&v132 = *(float *)&v141 * *(float *)&v141 * 0.5;
      *(float *)&v167 = *(float *)&v141 * *(float *)&v170 + *(float *)&v167;
      v168 = *(float *)&v141 * v171 + v168;
      *(float *)&v169 = *(float *)&v141 * v172 + *(float *)&v169;
      *(float *)&v132 = *(float *)&v132 + *(float *)&v141;
      do
      {
        ++v24;
        start[v24 - 1] = *(float *)&v132 * *(float *)((char *)&v166 + v24 * 4) + start[v24 - 1];
      }
      while ( v24 < 3 );
    }
  }
  v25 = *(_DWORD *)(v6 + 72);
  if ( v25 == 2 || v25 == 9 )
  {
    v26 = *(_DWORD *)(v6 + 188);
    if ( (v26 & 0x10) != 0 )
    {
      *(float *)&v136 = *(float *)&v148 * 16384.0 + start[0];
      v137 = v149 * 16384.0 + start[1];
      v138 = v150 * 16384.0 + start[2];
      if ( (v26 & 0x40) != 0 )
      {
        if ( (v26 & 0x200) != 0 )
        {
          if ( (*(float *)(v6 + 244) == *(float *)(v6 + 248))
             | __UNORDERED__(*(float *)(v6 + 244), *(float *)(v6 + 248)) )
          {
            v140 = *(int *)(v6 + 244);
          }
          else
          {
            *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 244), *(float *)(v6 + 248));
          }
          if ( (*(float *)(v6 + 252) == *(float *)(v6 + 256))
             | __UNORDERED__(*(float *)(v6 + 252), *(float *)(v6 + 256)) )
          {
            v143 = *(int *)(v6 + 252);
          }
          else
          {
            *(float *)&v143 = Com_RandFloatRange_m(*(float *)(v6 + 252), *(float *)(v6 + 256));
          }
          if ( (*(float *)(v6 + 260) == *(float *)(v6 + 264))
             | __UNORDERED__(*(float *)(v6 + 260), *(float *)(v6 + 264)) )
          {
            model = *(clipHandle_t *)(v6 + 260);
          }
          else
          {
            *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 260), *(float *)(v6 + 264));
          }
          *(float *)&v136 = *(float *)&v140 + *(float *)&v136;
          v137 = *(float *)&v143 + v137;
          v27 = *(float *)&model;
        }
        else
        {
          if ( (*(float *)(v6 + 260) == *(float *)(v6 + 264))
             | __UNORDERED__(*(float *)(v6 + 260), *(float *)(v6 + 264)) )
          {
            v28 = *(_DWORD *)(v6 + 260);
          }
          else
          {
            *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 260), *(float *)(v6 + 264));
            v28 = v132;
          }
          if ( (*(float *)(v6 + 252) == *(float *)(v6 + 256))
             | __UNORDERED__(*(float *)(v6 + 252), *(float *)(v6 + 256)) )
          {
            v29 = *(_DWORD *)(v6 + 252);
          }
          else
          {
            *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 252), *(float *)(v6 + 256));
            v29 = v132;
          }
          if ( (*(float *)(v6 + 244) == *(float *)(v6 + 248))
             | __UNORDERED__(*(float *)(v6 + 244), *(float *)(v6 + 248)) )
          {
            v30 = *(_DWORD *)(v6 + 244);
          }
          else
          {
            *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 244), *(float *)(v6 + 248));
            v30 = v132;
          }
          sub_492A20((float *)&v148, (float *)&v157, *(float *)&v30, *(float *)&v29, *(float *)&v28);
          *(float *)&v136 = *(float *)&v157 + *(float *)&v136;
          v137 = v158 + v137;
          v27 = v159;
        }
        v138 = v27 + v138;
      }
      results.fraction = 1.0;
      CM_Trace(&results, start, (const float *)&v136, 0, vec3_origin, 1, 0, v131);
      results.entityNum = 1022;
      if ( (results.fraction == 1.0) | __UNORDERED__(results.fraction, 1.0) )
        results.entityNum = 1023;
      v6 = a2;
      v7 = results.endpos[2];
      v161 = results.endpos[1];
      v31 = (*(_BYTE *)(a2 + 188) & 0x20) == 0;
      v160 = SLODWORD(results.endpos[0]);
      v162 = results.endpos[2];
      if ( !v31 )
      {
        v32 = *(_DWORD *)(a2 + 124);
        if ( v32 && (v33 = (*(_DWORD *)(a2 + 128) - v32) >> 2) != 0 )
        {
          com_randSeed_m = 214013 * com_randSeed_m + 2531011;
          v34 = *(_DWORD *)(v32 + 4 * ((int)(v33 * ((unsigned int)com_randSeed_m >> 17)) >> 15));
        }
        else
        {
          v34 = 0;
        }
        v175[1] = LODWORD(results.normal[1]);
        v175[0] = LODWORD(results.normal[0]);
        v175[2] = LODWORD(results.normal[2]);
        /* forward first */
        MakeNormalVectors( results.normal, (float *)v176, v177 );
        CFxScheduler__PlayEffect_id_axis(v163, v34, (float *)&v160, v175, 0);
        v6 = a2;
      }
    }
    else
    {
      if ( (v26 & 0x200) != 0 )
      {
        v35 = *(int *)(v6 + 244);
        if ( !((*(float *)&v35 == *(float *)(v6 + 248)) | __UNORDERED__(*(float *)&v35, *(float *)(v6 + 248))) )
        {
          *(float *)&v132 = Com_RandFloatRange_m(*(float *)&v35, *(float *)(v6 + 248));
          v35 = v132;
        }
        v36 = *(float *)(v6 + 252);
        v160 = v35;
        if ( (v36 == *(float *)(v6 + 256)) | __UNORDERED__(v36, *(float *)(v6 + 256)) )
        {
          v37 = *(int *)(v6 + 252);
        }
        else
        {
          *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 252), *(float *)(v6 + 256));
          v37 = v132;
        }
        v38 = *(float *)(v6 + 260);
        v161 = *(float *)&v37;
        if ( (v38 == *(float *)(v6 + 264)) | __UNORDERED__(v38, *(float *)(v6 + 264)) )
        {
          v162 = *(float *)(v6 + 260);
        }
        else
        {
          *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 260), *(float *)(v6 + 264));
          v162 = *(float *)&v132;
        }
      }
      else
      {
        if ( (*(float *)(v6 + 260) == *(float *)(v6 + 264)) | __UNORDERED__(*(float *)(v6 + 260), *(float *)(v6 + 264)) )
          model = *(clipHandle_t *)(v6 + 260);
        else
          *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 260), *(float *)(v6 + 264));
        if ( (*(float *)(v6 + 252) == *(float *)(v6 + 256)) | __UNORDERED__(*(float *)(v6 + 252), *(float *)(v6 + 256)) )
          capsule = *(int *)(v6 + 252);
        else
          *(float *)&capsule = Com_RandFloatRange_m(*(float *)(v6 + 252), *(float *)(v6 + 256));
        if ( (*(float *)(v6 + 244) == *(float *)(v6 + 248)) | __UNORDERED__(*(float *)(v6 + 244), *(float *)(v6 + 248)) )
          brushmask = *(int *)(v6 + 244);
        else
          *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 244), *(float *)(v6 + 248));
        *(float *)&v160 = *(float *)&capsule * *(float *)&v151
                        + *(float *)&model * v154
                        + *(float *)&brushmask * *(float *)&v148;
        v161 = *(float *)&capsule * v152 + *(float *)&model * v155 + *(float *)&brushmask * v149;
        v162 = *(float *)&model * v156 + *(float *)&brushmask * v150 + *(float *)&capsule * v153;
      }
      *(float *)&v160 = *(float *)&v160 + *v10;
      v161 = v161 + v10[1];
      v162 = v162 + v10[2];
    }
  }
  v39 = *(_DWORD *)(v6 + 72);
  if ( v39 != 6 && v39 != 10 && v39 != 12 )
  {
    if ( (*(_DWORD *)(v6 + 188) & 0x4000) != 0 )
    {
      v6 = a2;
      *(float *)&v132 = (float)rand();
      *(float *)&v141 = *(float *)&v132 / 32768.0;
      *(float *)&v164 = (*(float *)(a2 + 432) - *(float *)(a2 + 428)) * *(float *)&v141 + *(float *)(a2 + 428);
      v165 = (*(float *)(a2 + 440) - *(float *)(a2 + 436)) * *(float *)&v141 + *(float *)(a2 + 436);
      v166 = (*(float *)(a2 + 448) - *(float *)(a2 + 444)) * *(float *)&v141 + *(float *)(a2 + 444);
      *(float *)&v157 = (*(float *)(a2 + 456) - *(float *)(a2 + 452)) * *(float *)&v141 + *(float *)(a2 + 452);
      v158 = (*(float *)(a2 + 464) - *(float *)(a2 + 460)) * *(float *)&v141 + *(float *)(a2 + 460);
      v159 = (*(float *)(a2 + 472) - *(float *)(a2 + 468)) * *(float *)&v141 + *(float *)(a2 + 468);
    }
    else
    {
      v40 = *(int *)(v6 + 428);
      if ( !((*(float *)&v40 == *(float *)(v6 + 432)) | __UNORDERED__(*(float *)&v40, *(float *)(v6 + 432))) )
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)&v40, *(float *)(v6 + 432));
        v40 = v132;
      }
      v41 = *(float *)(v6 + 436);
      v164 = v40;
      if ( (v41 == *(float *)(v6 + 440)) | __UNORDERED__(v41, *(float *)(v6 + 440)) )
      {
        v42 = *(float *)(v6 + 436);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 436), *(float *)(v6 + 440));
        v42 = *(float *)&v132;
      }
      v43 = *(float *)(v6 + 444);
      v165 = v42;
      if ( (v43 == *(float *)(v6 + 448)) | __UNORDERED__(v43, *(float *)(v6 + 448)) )
      {
        v44 = *(float *)(v6 + 444);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 444), *(float *)(v6 + 448));
        v44 = *(float *)&v132;
      }
      v45 = *(float *)(v6 + 452);
      v166 = v44;
      v46 = *(int *)(v6 + 452);
      if ( !((v45 == *(float *)(v6 + 456)) | __UNORDERED__(v45, *(float *)(v6 + 456))) )
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)&v46, *(float *)(v6 + 456));
        v46 = v132;
      }
      v47 = *(float *)(v6 + 460);
      v157 = v46;
      if ( (v47 == *(float *)(v6 + 464)) | __UNORDERED__(v47, *(float *)(v6 + 464)) )
      {
        v48 = *(float *)(v6 + 460);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 460), *(float *)(v6 + 464));
        v48 = *(float *)&v132;
      }
      v49 = *(float *)(v6 + 468);
      v158 = v48;
      if ( (v49 == *(float *)(v6 + 472)) | __UNORDERED__(v49, *(float *)(v6 + 472)) )
      {
        v50 = *(float *)(v6 + 468);
      }
      else
      {
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 468), *(float *)(v6 + 472));
        v50 = *(float *)&v132;
      }
      v159 = v50;
    }
  }
  if ( (*(_DWORD *)(v6 + 184) & 0x200000) != 0 && a3 && *a3 >= 0 )
  {
    v51 = (char *)*CFxBoltFrame__Acquire((int **)&v132, a3);
    if ( v51 )
    {
      v52 = v51;
      ++*(_DWORD *)v51;
      v147 = v51;
    }
    else
    {
      v52 = v147;
    }
    if ( *(float *)&v132 != 0.0 )
      CFxBoltFrame__Release((char *)v132);
    if ( !v52 )
      return;
    if ( !sub_48D770((int)v52) )
    {
      v53 = v52;
LABEL_506:
      CFxBoltFrame__Release(v53);
      return;
    }
    v6 = a2;
  }
  switch ( *(_DWORD *)(v6 + 72) )
  {
    case 1:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v140 = *(int *)(v6 + 92);
      else
        *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 596) == *(float *)(v6 + 600)) | __UNORDERED__(*(float *)(v6 + 596), *(float *)(v6 + 600)) )
        v143 = *(int *)(v6 + 596);
      else
        *(float *)&v143 = Com_RandFloatRange_m(*(float *)(v6 + 596), *(float *)(v6 + 600));
      if ( (*(float *)(v6 + 300) == *(float *)(v6 + 304)) | __UNORDERED__(*(float *)(v6 + 300), *(float *)(v6 + 304)) )
        model = *(clipHandle_t *)(v6 + 300);
      else
        *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 300), *(float *)(v6 + 304));
      if ( (*(float *)(v6 + 292) == *(float *)(v6 + 296)) | __UNORDERED__(*(float *)(v6 + 292), *(float *)(v6 + 296)) )
        brushmask = *(int *)(v6 + 292);
      else
        *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 292), *(float *)(v6 + 296));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        capsule = *(int *)(v6 + 476);
      else
        *(float *)&capsule = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 500) == *(float *)(v6 + 504)) | __UNORDERED__(*(float *)(v6 + 500), *(float *)(v6 + 504)) )
        v141 = *(int *)(v6 + 500);
      else
        *(float *)&v141 = Com_RandFloatRange_m(*(float *)(v6 + 500), *(float *)(v6 + 504));
      if ( (*(float *)(v6 + 492) == *(float *)(v6 + 496)) | __UNORDERED__(*(float *)(v6 + 492), *(float *)(v6 + 496)) )
        v144 = *(int *)(v6 + 492);
      else
        *(float *)&v144 = Com_RandFloatRange_m(*(float *)(v6 + 492), *(float *)(v6 + 496));
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
        v145 = *(int *)(v6 + 484);
      else
        *(float *)&v145 = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
      if ( (*(float *)(v6 + 548) == *(float *)(v6 + 552)) | __UNORDERED__(*(float *)(v6 + 548), *(float *)(v6 + 552)) )
        v142 = *(int *)(v6 + 548);
      else
        *(float *)&v142 = Com_RandFloatRange_m(*(float *)(v6 + 548), *(float *)(v6 + 552));
      if ( (*(float *)(v6 + 540) == *(float *)(v6 + 544)) | __UNORDERED__(*(float *)(v6 + 540), *(float *)(v6 + 544)) )
        v139 = *(float *)(v6 + 540);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 540), *(float *)(v6 + 544));
      if ( (*(float *)(v6 + 532) == *(float *)(v6 + 536)) | __UNORDERED__(*(float *)(v6 + 532), *(float *)(v6 + 536)) )
        v132 = *(int *)(v6 + 532);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 532), *(float *)(v6 + 536));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v54 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v54 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v55 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v55 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v56 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v56 = v136;
      }
      v121 = *(_DWORD *)(v6 + 184);
      v115 = sub_492780(v6 + 104);
      v111 = (unsigned __int64)*(float *)&v140;
      v107 = sub_492780(v6 + 120);
      v57 = sub_492780(v6 + 136);
      FX_AddParticle(
        (int)start,
        &v147,
        (float *)&v167,
        (float *)&v170,
        *(_BYTE *)(v6 + 192),
        v56,
        v55,
        *(float *)&v54,
        v132,
        SLODWORD(v139),
        *(float *)&v142,
        v145,
        v144,
        *(float *)&v141,
        &v164,
        &v157,
        *(float *)&capsule,
        brushmask,
        model,
        (_DWORD *)(v6 + 196),
        (_DWORD *)(v6 + 208),
        v143,
        v57,
        v107,
        v111,
        v115,
        v121);
      goto LABEL_501;
    case 2:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        v139 = *(float *)(v6 + 476);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 500) == *(float *)(v6 + 504)) | __UNORDERED__(*(float *)(v6 + 500), *(float *)(v6 + 504)) )
        v142 = *(int *)(v6 + 500);
      else
        *(float *)&v142 = Com_RandFloatRange_m(*(float *)(v6 + 500), *(float *)(v6 + 504));
      if ( (*(float *)(v6 + 492) == *(float *)(v6 + 496)) | __UNORDERED__(*(float *)(v6 + 492), *(float *)(v6 + 496)) )
        v145 = *(int *)(v6 + 492);
      else
        *(float *)&v145 = Com_RandFloatRange_m(*(float *)(v6 + 492), *(float *)(v6 + 496));
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
        v144 = *(int *)(v6 + 484);
      else
        *(float *)&v144 = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v58 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v58 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v59 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v59 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v60 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v60 = v136;
      }
      v122 = *(_DWORD *)(v6 + 184);
      v61 = sub_492780(v6 + 104);
      FX_AddLine(
        (int)start,
        &v147,
        (float *)&v160,
        v60,
        v59,
        *(float *)&v58,
        v144,
        v145,
        *(float *)&v142,
        &v164,
        &v157,
        v139,
        (unsigned __int64)*(float *)&v132,
        v61,
        v122);
      goto LABEL_501;
    case 3:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 596) == *(float *)(v6 + 600)) | __UNORDERED__(*(float *)(v6 + 596), *(float *)(v6 + 600)) )
        v139 = *(float *)(v6 + 596);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 596), *(float *)(v6 + 600));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        v142 = *(int *)(v6 + 476);
      else
        *(float *)&v142 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 500) == *(float *)(v6 + 504)) | __UNORDERED__(*(float *)(v6 + 500), *(float *)(v6 + 504)) )
        v145 = *(int *)(v6 + 500);
      else
        *(float *)&v145 = Com_RandFloatRange_m(*(float *)(v6 + 500), *(float *)(v6 + 504));
      if ( (*(float *)(v6 + 492) == *(float *)(v6 + 496)) | __UNORDERED__(*(float *)(v6 + 492), *(float *)(v6 + 496)) )
        v144 = *(int *)(v6 + 492);
      else
        *(float *)&v144 = Com_RandFloatRange_m(*(float *)(v6 + 492), *(float *)(v6 + 496));
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
        v140 = *(int *)(v6 + 484);
      else
        *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
      if ( (*(float *)(v6 + 572) == *(float *)(v6 + 576)) | __UNORDERED__(*(float *)(v6 + 572), *(float *)(v6 + 576)) )
        v143 = *(int *)(v6 + 572);
      else
        *(float *)&v143 = Com_RandFloatRange_m(*(float *)(v6 + 572), *(float *)(v6 + 576));
      if ( (*(float *)(v6 + 564) == *(float *)(v6 + 568)) | __UNORDERED__(*(float *)(v6 + 564), *(float *)(v6 + 568)) )
        model = *(clipHandle_t *)(v6 + 564);
      else
        *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 564), *(float *)(v6 + 568));
      if ( (*(float *)(v6 + 556) == *(float *)(v6 + 560)) | __UNORDERED__(*(float *)(v6 + 556), *(float *)(v6 + 560)) )
        brushmask = *(int *)(v6 + 556);
      else
        *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 556), *(float *)(v6 + 560));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v62 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v62 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v63 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v63 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v64 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v64 = v136;
      }
      v123 = *(_DWORD *)(v6 + 184);
      v116 = sub_492780(v6 + 104);
      v112 = (unsigned __int64)*(float *)&v132;
      v108 = sub_492780(v6 + 120);
      v65 = sub_492780(v6 + 136);
      FX_AddTail(
        (int)start,
        &v147,
        (float *)&v167,
        (float *)&v170,
        v64,
        v63,
        *(float *)&v62,
        brushmask,
        model,
        *(float *)&v143,
        v140,
        v144,
        *(float *)&v145,
        &v164,
        &v157,
        *(float *)&v142,
        (_DWORD *)(v6 + 196),
        (_DWORD *)(v6 + 208),
        SLODWORD(v139),
        v65,
        v108,
        v112,
        v116,
        v123);
      goto LABEL_501;
    case 4:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        v139 = *(float *)(v6 + 476);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 500) == *(float *)(v6 + 504)) | __UNORDERED__(*(float *)(v6 + 500), *(float *)(v6 + 504)) )
        v142 = *(int *)(v6 + 500);
      else
        *(float *)&v142 = Com_RandFloatRange_m(*(float *)(v6 + 500), *(float *)(v6 + 504));
      if ( (*(float *)(v6 + 492) == *(float *)(v6 + 496)) | __UNORDERED__(*(float *)(v6 + 492), *(float *)(v6 + 496)) )
        v145 = *(int *)(v6 + 492);
      else
        *(float *)&v145 = Com_RandFloatRange_m(*(float *)(v6 + 492), *(float *)(v6 + 496));
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
        v144 = *(int *)(v6 + 484);
      else
        *(float *)&v144 = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
      if ( (*(float *)(v6 + 572) == *(float *)(v6 + 576)) | __UNORDERED__(*(float *)(v6 + 572), *(float *)(v6 + 576)) )
        v140 = *(int *)(v6 + 572);
      else
        *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 572), *(float *)(v6 + 576));
      if ( (*(float *)(v6 + 564) == *(float *)(v6 + 568)) | __UNORDERED__(*(float *)(v6 + 564), *(float *)(v6 + 568)) )
        v143 = *(int *)(v6 + 564);
      else
        *(float *)&v143 = Com_RandFloatRange_m(*(float *)(v6 + 564), *(float *)(v6 + 568));
      if ( (*(float *)(v6 + 556) == *(float *)(v6 + 560)) | __UNORDERED__(*(float *)(v6 + 556), *(float *)(v6 + 560)) )
        model = *(clipHandle_t *)(v6 + 556);
      else
        *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 556), *(float *)(v6 + 560));
      if ( (*(float *)(v6 + 548) == *(float *)(v6 + 552)) | __UNORDERED__(*(float *)(v6 + 548), *(float *)(v6 + 552)) )
        brushmask = *(int *)(v6 + 548);
      else
        *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 548), *(float *)(v6 + 552));
      if ( (*(float *)(v6 + 540) == *(float *)(v6 + 544)) | __UNORDERED__(*(float *)(v6 + 540), *(float *)(v6 + 544)) )
        capsule = *(int *)(v6 + 540);
      else
        *(float *)&capsule = Com_RandFloatRange_m(*(float *)(v6 + 540), *(float *)(v6 + 544));
      if ( (*(float *)(v6 + 532) == *(float *)(v6 + 536)) | __UNORDERED__(*(float *)(v6 + 532), *(float *)(v6 + 536)) )
        v141 = *(int *)(v6 + 532);
      else
        *(float *)&v141 = Com_RandFloatRange_m(*(float *)(v6 + 532), *(float *)(v6 + 536));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v70 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v70 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v71 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v71 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v72 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v72 = v136;
      }
      v125 = *(_DWORD *)(v6 + 184);
      v73 = sub_492780(v6 + 104);
      FX_AddCylinder(
        (int)start,
        &v147,
        (float *)&v148,
        v72,
        v71,
        *(float *)&v70,
        v141,
        capsule,
        *(float *)&brushmask,
        model,
        v143,
        *(float *)&v140,
        v144,
        v145,
        *(float *)&v142,
        &v164,
        &v157,
        v139,
        (unsigned __int64)*(float *)&v132,
        v73,
        v125);
      goto LABEL_501;
    case 5:
      if ( (*(float *)(v6 + 308) == *(float *)(v6 + 312)) | __UNORDERED__(*(float *)(v6 + 308), *(float *)(v6 + 312)) )
        v132 = *(int *)(v6 + 308);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 308), *(float *)(v6 + 312));
      if ( (*(float *)(v6 + 316) == *(float *)(v6 + 320)) | __UNORDERED__(*(float *)(v6 + 316), *(float *)(v6 + 320)) )
        v139 = *(float *)(v6 + 316);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 316), *(float *)(v6 + 320));
      v74 = *(float *)(v6 + 324);
      if ( (v74 == *(float *)(v6 + 328)) | __UNORDERED__(v74, *(float *)(v6 + 328)) )
      {
        v142 = *(int *)(v6 + 324);
      }
      else
      {
        v74 = Com_RandFloatRange_m(*(float *)(v6 + 324), *(float *)(v6 + 328));
        *(float *)&v142 = v74;
      }
      vectoangles((void *)LODWORD(v7), (int)&v136, &v148, v74);
      *(float *)v173 = *(float *)&v132 + *(float *)&v136;
      *(float *)&v173[1] = v139 + v137;
      *(float *)&v173[2] = *(float *)&v142 + v138;
      v75 = *(int *)(v6 + 332);
      if ( !((*(float *)&v75 == *(float *)(v6 + 336)) | __UNORDERED__(*(float *)&v75, *(float *)(v6 + 336))) )
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)&v75, *(float *)(v6 + 336));
        v75 = v136;
      }
      v76 = *(float *)(v6 + 340);
      v160 = v75;
      if ( (v76 == *(float *)(v6 + 344)) | __UNORDERED__(v76, *(float *)(v6 + 344)) )
      {
        v77 = *(int *)(v6 + 340);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 340), *(float *)(v6 + 344));
        v77 = v136;
      }
      v78 = *(float *)(v6 + 348);
      v161 = *(float *)&v77;
      if ( (v78 == *(float *)(v6 + 352)) | __UNORDERED__(v78, *(float *)(v6 + 352)) )
      {
        v79 = *(int *)(v6 + 348);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 348), *(float *)(v6 + 352));
        v79 = v136;
      }
      v162 = *(float *)&v79;
      v80 = sub_492780(v6 + 104);
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 420) == *(float *)(v6 + 424)) | __UNORDERED__(*(float *)(v6 + 420), *(float *)(v6 + 424)) )
        v139 = *(float *)(v6 + 420);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 420), *(float *)(v6 + 424));
      if ( (*(float *)(v6 + 412) == *(float *)(v6 + 416)) | __UNORDERED__(*(float *)(v6 + 412), *(float *)(v6 + 416)) )
        v142 = *(int *)(v6 + 412);
      else
        *(float *)&v142 = Com_RandFloatRange_m(*(float *)(v6 + 412), *(float *)(v6 + 416));
      if ( (*(float *)(v6 + 596) == *(float *)(v6 + 600)) | __UNORDERED__(*(float *)(v6 + 596), *(float *)(v6 + 600)) )
        v145 = *(int *)(v6 + 596);
      else
        *(float *)&v145 = Com_RandFloatRange_m(*(float *)(v6 + 596), *(float *)(v6 + 600));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        v144 = *(int *)(v6 + 476);
      else
        *(float *)&v144 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 500) == *(float *)(v6 + 504)) | __UNORDERED__(*(float *)(v6 + 500), *(float *)(v6 + 504)) )
        v140 = *(int *)(v6 + 500);
      else
        *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 500), *(float *)(v6 + 504));
      if ( (*(float *)(v6 + 492) == *(float *)(v6 + 496)) | __UNORDERED__(*(float *)(v6 + 492), *(float *)(v6 + 496)) )
        v143 = *(int *)(v6 + 492);
      else
        *(float *)&v143 = Com_RandFloatRange_m(*(float *)(v6 + 492), *(float *)(v6 + 496));
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
        model = *(clipHandle_t *)(v6 + 484);
      else
        *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v81 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v81 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v82 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v82 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v83 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v83 = v136;
      }
      v126 = *(_DWORD *)(v6 + 184);
      v117 = v80;
      v113 = (unsigned __int64)*(float *)&v132;
      v109 = LODWORD(v139);
      v106 = v142;
      v104 = sub_492780(v6 + 152);
      v102 = sub_492780(v6 + 120);
      v84 = sub_492780(v6 + 136);
      FX_AddEmitter(
        (int)start,
        &v147,
        (float *)&v167,
        (float *)&v170,
        v83,
        v82,
        *(float *)&v81,
        model,
        v143,
        *(float *)&v140,
        &v164,
        &v157,
        *(float *)&v144,
        v173,
        &v160,
        (_DWORD *)(v6 + 196),
        (_DWORD *)(v6 + 208),
        v145,
        v84,
        v102,
        v104,
        v106,
        v109,
        v113,
        v117,
        v126);
      goto LABEL_501;
    case 6:
      v93 = sub_492780(v6 + 104);
      SFxHelper__PlaySound(v93, (int)&theFxHelper, start, 1023);
      goto LABEL_501;
    case 7:
      if ( (*(float *)(v6 + 292) == *(float *)(v6 + 296)) | __UNORDERED__(*(float *)(v6 + 292), *(float *)(v6 + 296)) )
      {
        v85 = *(_DWORD *)(v6 + 292);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 292), *(float *)(v6 + 296));
        v85 = v136;
      }
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
      {
        v86 = *(_DWORD *)(v6 + 484);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
        v86 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v87 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v87 = v136;
      }
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      v127 = (unsigned __int64)*(float *)&v132;
      v105 = v166;
      v103 = v165;
      v101 = v164;
      v88 = sub_492780(v6 + 104);
      VM_Call(cgvm, 14, v88, start, &v148, v85, v101, v103, v105, v86, 1, v87, 0, v127);
      goto LABEL_501;
    case 8:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 596) == *(float *)(v6 + 600)) | __UNORDERED__(*(float *)(v6 + 596), *(float *)(v6 + 600)) )
        v139 = *(float *)(v6 + 596);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 596), *(float *)(v6 + 600));
      if ( (*(float *)(v6 + 300) == *(float *)(v6 + 304)) | __UNORDERED__(*(float *)(v6 + 300), *(float *)(v6 + 304)) )
        v142 = *(int *)(v6 + 300);
      else
        *(float *)&v142 = Com_RandFloatRange_m(*(float *)(v6 + 300), *(float *)(v6 + 304));
      if ( (*(float *)(v6 + 292) == *(float *)(v6 + 296)) | __UNORDERED__(*(float *)(v6 + 292), *(float *)(v6 + 296)) )
        v145 = *(int *)(v6 + 292);
      else
        *(float *)&v145 = Com_RandFloatRange_m(*(float *)(v6 + 292), *(float *)(v6 + 296));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        v144 = *(int *)(v6 + 476);
      else
        *(float *)&v144 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 500) == *(float *)(v6 + 504)) | __UNORDERED__(*(float *)(v6 + 500), *(float *)(v6 + 504)) )
        v140 = *(int *)(v6 + 500);
      else
        *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 500), *(float *)(v6 + 504));
      if ( (*(float *)(v6 + 492) == *(float *)(v6 + 496)) | __UNORDERED__(*(float *)(v6 + 492), *(float *)(v6 + 496)) )
        v143 = *(int *)(v6 + 492);
      else
        *(float *)&v143 = Com_RandFloatRange_m(*(float *)(v6 + 492), *(float *)(v6 + 496));
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
        model = *(clipHandle_t *)(v6 + 484);
      else
        *(float *)&model = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
      if ( (*(float *)(v6 + 548) == *(float *)(v6 + 552)) | __UNORDERED__(*(float *)(v6 + 548), *(float *)(v6 + 552)) )
        brushmask = *(int *)(v6 + 548);
      else
        *(float *)&brushmask = Com_RandFloatRange_m(*(float *)(v6 + 548), *(float *)(v6 + 552));
      if ( (*(float *)(v6 + 540) == *(float *)(v6 + 544)) | __UNORDERED__(*(float *)(v6 + 540), *(float *)(v6 + 544)) )
        capsule = *(int *)(v6 + 540);
      else
        *(float *)&capsule = Com_RandFloatRange_m(*(float *)(v6 + 540), *(float *)(v6 + 544));
      if ( (*(float *)(v6 + 532) == *(float *)(v6 + 536)) | __UNORDERED__(*(float *)(v6 + 532), *(float *)(v6 + 536)) )
        v141 = *(int *)(v6 + 532);
      else
        *(float *)&v141 = Com_RandFloatRange_m(*(float *)(v6 + 532), *(float *)(v6 + 536));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v89 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v89 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v90 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v90 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v91 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v91 = v136;
      }
      v128 = *(_DWORD *)(v6 + 184);
      v118 = sub_492780(v6 + 104);
      v114 = (unsigned __int64)*(float *)&v132;
      v110 = sub_492780(v6 + 120);
      v92 = sub_492780(v6 + 136);
      FX_AddOrientedParticle(
        (int)&v148,
        &v147,
        (int)start,
        (float *)&v167,
        (float *)&v170,
        *(_BYTE *)(v6 + 192),
        v91,
        v90,
        *(float *)&v89,
        v141,
        capsule,
        *(float *)&brushmask,
        model,
        v143,
        *(float *)&v140,
        &v164,
        &v157,
        *(float *)&v144,
        v145,
        v142,
        (_DWORD *)(v6 + 196),
        (_DWORD *)(v6 + 208),
        SLODWORD(v139),
        v92,
        v110,
        v114,
        v118,
        v128);
      goto LABEL_501;
    case 9:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 596) == *(float *)(v6 + 600)) | __UNORDERED__(*(float *)(v6 + 596), *(float *)(v6 + 600)) )
        v139 = *(float *)(v6 + 596);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 596), *(float *)(v6 + 600));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        v142 = *(int *)(v6 + 476);
      else
        *(float *)&v142 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 500) == *(float *)(v6 + 504)) | __UNORDERED__(*(float *)(v6 + 500), *(float *)(v6 + 504)) )
        v145 = *(int *)(v6 + 500);
      else
        *(float *)&v145 = Com_RandFloatRange_m(*(float *)(v6 + 500), *(float *)(v6 + 504));
      if ( (*(float *)(v6 + 492) == *(float *)(v6 + 496)) | __UNORDERED__(*(float *)(v6 + 492), *(float *)(v6 + 496)) )
        v144 = *(int *)(v6 + 492);
      else
        *(float *)&v144 = Com_RandFloatRange_m(*(float *)(v6 + 492), *(float *)(v6 + 496));
      if ( (*(float *)(v6 + 484) == *(float *)(v6 + 488)) | __UNORDERED__(*(float *)(v6 + 484), *(float *)(v6 + 488)) )
        v140 = *(int *)(v6 + 484);
      else
        *(float *)&v140 = Com_RandFloatRange_m(*(float *)(v6 + 484), *(float *)(v6 + 488));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v66 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v66 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v67 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v67 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v68 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v68 = v136;
      }
      v124 = *(_DWORD *)(v6 + 184);
      v69 = sub_492780(v6 + 104);
      FX_AddElectricity(
        (int)start,
        &v147,
        (float *)&v160,
        v68,
        v67,
        *(float *)&v66,
        v140,
        v144,
        *(float *)&v145,
        &v164,
        &v157,
        *(float *)&v142,
        SLODWORD(v139),
        (unsigned __int64)*(float *)&v132,
        v69,
        v124);
      goto LABEL_501;
    case 0xA:
      if ( v147 )
        v94 = v147 + 60;
      else
        v94 = 0;
      v129 = (int *)v94;
      v95 = sub_492780(v6 + 168);
      CFxScheduler__PlayEffect_id_axis(v163, v95, start, &v148, v129);
      goto LABEL_501;
    case 0xB:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
        v139 = *(float *)(v6 + 476);
      else
        v139 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
      if ( (*(float *)(v6 + 524) == *(float *)(v6 + 528)) | __UNORDERED__(*(float *)(v6 + 524), *(float *)(v6 + 528)) )
      {
        v96 = *(_DWORD *)(v6 + 524);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 524), *(float *)(v6 + 528));
        v96 = v136;
      }
      if ( (*(float *)(v6 + 516) == *(float *)(v6 + 520)) | __UNORDERED__(*(float *)(v6 + 516), *(float *)(v6 + 520)) )
      {
        v97 = *(_DWORD *)(v6 + 516);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 516), *(float *)(v6 + 520));
        v97 = v136;
      }
      if ( (*(float *)(v6 + 508) == *(float *)(v6 + 512)) | __UNORDERED__(*(float *)(v6 + 508), *(float *)(v6 + 512)) )
      {
        v98 = *(_DWORD *)(v6 + 508);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 508), *(float *)(v6 + 512));
        v98 = v136;
      }
      FX_AddLight(
        &v147,
        start,
        v98,
        v97,
        *(float *)&v96,
        &v164,
        &v157,
        v139,
        (unsigned __int64)*(float *)&v132,
        *(_DWORD *)(v6 + 184));
      goto LABEL_501;
    case 0xC:
      if ( !((*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96))) )
        Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( !((*(float *)(v6 + 268) == *(float *)(v6 + 272)) | __UNORDERED__(*(float *)(v6 + 268), *(float *)(v6 + 272))) )
        Com_RandFloatRange_m(*(float *)(v6 + 268), *(float *)(v6 + 272));
      if ( !((*(float *)(v6 + 596) == *(float *)(v6 + 600)) | __UNORDERED__(*(float *)(v6 + 596), *(float *)(v6 + 600))) )
        Com_RandFloatRange_m(*(float *)(v6 + 596), *(float *)(v6 + 600));
      break;
    case 0xD:
      if ( (*(float *)(v6 + 92) == *(float *)(v6 + 96)) | __UNORDERED__(*(float *)(v6 + 92), *(float *)(v6 + 96)) )
        v132 = *(int *)(v6 + 92);
      else
        *(float *)&v132 = Com_RandFloatRange_m(*(float *)(v6 + 92), *(float *)(v6 + 96));
      if ( (*(float *)(v6 + 476) == *(float *)(v6 + 480)) | __UNORDERED__(*(float *)(v6 + 476), *(float *)(v6 + 480)) )
      {
        v99 = *(_DWORD *)(v6 + 476);
      }
      else
      {
        *(float *)&v136 = Com_RandFloatRange_m(*(float *)(v6 + 476), *(float *)(v6 + 480));
        v99 = v136;
      }
      v130 = *(_DWORD *)(v6 + 184);
      v100 = sub_492780(v6 + 104);
      FX_AddFlash(&v164, start, &v157, *(float *)&v99, (unsigned __int64)*(float *)&v132, v100, v130);
LABEL_501:
      v6 = a2;
      break;
    default:
      break;
  }
  if ( *(_BYTE *)v6 )
  {
    --*(_DWORD *)(v6 + 4);
    if ( *(int *)(a2 + 4) <= 0 )
    {
      CPrimitiveTemplate__dtor(a2);
      sub_4976D0((char *)a2, &dword_140758C);
    }
  }
  v53 = v147;
  if ( v147 )
    goto LABEL_506;
}
#endif

/* ---- CFxScheduler__SaveState_m  0x004970E0 ----  [CONFIRMED] */
unsigned __int8 __cdecl CFxScheduler__SaveState_m(int this, _DWORD *a2)
{
  __int16 i;
  int v4;
  unsigned __int8 result;
  int *v6;
  int *v7;
  int v8;
  _DWORD *v9;
  int v10;
  int *v11;
  char *v12;
  int v13;
  int v14;
  int v15;
  int *k;
  char v17[4]; // [esp+10h] [ebp-110h] BYREF
  int *j; // [esp+14h] [ebp-10Ch] BYREF
  int v19; // [esp+18h] [ebp-108h] BYREF
  char Source[256]; // [esp+1Ch] [ebp-104h] BYREF
  unsigned int v21;
  unsigned int retaddr;

  v21 = retaddr ^ _security_cookie;
  if ( *(_BYTE *)(this + 12) )
  {
    CFxArchive__ReadData(2, (_DWORD *)this, &j);
    for ( i = (__int16)j; (_WORD)j; i = (__int16)j )
    {
      CFxArchive__ReadData(1, (_DWORD *)this, v17);
      if ( v17[0] )
      {
        v4 = (unsigned __int8)v17[0];
        CFxArchive__ReadData((unsigned __int8)v17[0], (_DWORD *)this, Source);
        Source[v4] = 0;
        *(_DWORD *)(this + 4 * i + 20) = CFxScheduler__RegisterEffect(a2, Source, 0);
      }
      CFxArchive__ReadData(2, (_DWORD *)this, &j);
    }
    CFxArchive__ReadData(4, (_DWORD *)this, &j);
    result = (unsigned __int8)j;
    if ( j )
    {
      v6 = j;
      do
      {
        v7 = FxMem_AllocScheduledEffect(&dword_1407594, 68);
        j = v7;
        sub_497320(this, (unsigned __int16 *)v7);
        v8 = *v7;
        if ( *v7 > 0
          && (v9 = &a2[42 * v8], *(_BYTE *)v9)
          && (v10 = v7[1], v10 >= 0)
          && v10 < v9[17]
          && a2[42 * v8 + 18 + v10] )
        {
          result = sub_497B10(&j, (int)(a2 + 5379));
        }
        else
        {
          result = (unsigned __int8)sub_4977F0((char *)v7, &dword_1407594);
        }
        v6 = (int *)((char *)v6 - 1);
      }
      while ( v6 );
    }
  }
  else
  {
    v11 = *(int **)dword_140C9A4;
    for ( j = *(int **)dword_140C9A4; j != (int *)dword_140C9A4; v11 = j )
    {
      if ( (unsigned int)v11[9] < 0x10 )
        v12 = (char *)(v11 + 4);
      else
        v12 = (char *)v11[4];
      v13 = v11[10];
      v14 = strlen(v12);
      if ( v14 >= 256 )
        Com_Error(ERR_DROP, &byte_559C88);
      v19 = v13;
      CFxArchive__WriteData((int *)this, &v19, 2);
      v17[0] = v14;
      CFxArchive__WriteData((int *)this, v17, 1);
      if ( v14 )
        CFxArchive__WriteData((int *)this, v12, v14);
      sub_498230(v15, &j);
    }
    v19 = 0;
    CFxArchive__WriteData((int *)this, &v19, 2);
    v19 = a2[5381];
    CFxArchive__WriteData((int *)this, &v19, 4);
    result = dword_140C9B0;
    if ( dword_140C9B0 )
    {
      for ( k = *(int **)dword_140C9B0; k != (int *)dword_140C9B0; k = (int *)*k )
        result = sub_497320(this, (unsigned __int16 *)k[2]);
    }
  }
  return result;
}

/* ---- sub_497320  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_497320() { return 0; }
#if 0
char __cdecl sub_497320(int a1, unsigned __int16 *a2)
{
  int v4; // [esp+4h] [ebp-4h] BYREF

  if ( *(_BYTE *)(a1 + 12) )
  {
    CFxArchive__ReadData(2, (_DWORD *)a1, &v4);
    *(_DWORD *)a2 = *(_DWORD *)(a1 + 4 * (__int16)v4 + 20);
  }
  else
  {
    v4 = *a2;
    CFxArchive__WriteData((int *)a1, &v4, 2);
  }
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a1, (_BYTE *)a2 + 4);
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)a2 + 4, 4);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a1, (_BYTE *)a2 + 8);
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)a2 + 8, 4);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(8, (_DWORD *)a1, (_BYTE *)a2 + 12);
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)a2 + 12, 8);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a1, (_BYTE *)a2 + 20);
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)a2 + 20, 12);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a1, (_BYTE *)a2 + 32);
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)a2 + 32, 12);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(12, (_DWORD *)a1, (_BYTE *)a2 + 44);
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)a2 + 44, 12);
  if ( *(_BYTE *)(a1 + 12) )
    return CFxArchive__ReadData(12, (_DWORD *)a1, (_BYTE *)a2 + 56);
  else
    return CFxArchive__WriteData((int *)a1, (_BYTE *)a2 + 56, 12);
}
#endif

/* ---- sub_497440  0x00497440 ----  VERIFIED */
int __cdecl sub_497440(int result)
{
  *(_DWORD *)(result + 24) = 15;
  *(_DWORD *)(result + 20) = 0;
  *(_BYTE *)(result + 4) = 0;
  return result;
}

/* ---- sub_497450  0x00497450 ----  VERIFIED */
int __cdecl sub_497450(int this, void ***a2)
{
  *(_DWORD *)(this + 20) = 0;
  *(_DWORD *)(this + 24) = 15;
  *(_BYTE *)(this + 4) = 0;
  sub_497C10((void ***)this, a2, 0, (void **)0xFFFFFFFF);
  return this;
}

/* ---- sub_497480  0x00497480 ----  VERIFIED */
int __cdecl sub_497480(int this, char *a2)
{
  *(_DWORD *)(this + 24) = 15;
  *(_DWORD *)(this + 20) = 0;
  *(_BYTE *)(this + 4) = 0;
  sub_498410(this, a2, (void **)strlen(a2));
  return this;
}

/* ---- sub_4974F0  0x004974F0 ----  VERIFIED */
void ***__cdecl sub_4974F0(void ***a1, void ***a2)
{
  return sub_497C10(a2, a1, 0, (void **)0xFFFFFFFF);
}

/* ---- sub_497500  0x00497500 ----  VERIFIED */
int __cdecl sub_497500(int a1)
{
  if ( *(_DWORD *)(a1 + 24) < 0x10u )
    return a1 + 4;
  else
    return *(_DWORD *)(a1 + 4);
}

/* ---- sub_497510  0x00497510 ----  [HIGH] */
int __cdecl sub_497510(_DWORD *this)
{
  return this[5];
}

/* ---- sub_497520  0x00497520 ----  VERIFIED */
void __cdecl sub_497520(int a1)
{
  if ( *(_DWORD *)(a1 + 4) )
    j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  *(_DWORD *)(a1 + 12) = 0;
}

/* ---- sub_497550  0x00497550 ----  VERIFIED */
int __cdecl sub_497550(int a1)
{
  int v1;

  v1 = *(_DWORD *)(a1 + 4);
  if ( v1 )
    return (*(_DWORD *)(a1 + 8) - v1) >> 2;
  else
    return 0;
}

/* ---- sub_497570  0x00497570 ----  [HIGH] */
int __cdecl sub_497570(int a1, int a2)
{
  return *(_DWORD *)(a1 + 4) + 4 * a2;
}

/* ---- sub_497580  0x00497580 ----  [HIGH] */
int __cdecl sub_497580(int a1, int a2)
{
  return *(_DWORD *)(a1 + 4) + 4 * a2;
}

/* ---- sub_497590  0x00497590 ----  VERIFIED */
_DWORD *__cdecl sub_497590(_DWORD *a1, _DWORD *a2)
{
  int v2;
  unsigned int v3;
  _DWORD *v4;
  _DWORD *result;

  v2 = a2[1];
  if ( v2 )
    v3 = (a2[2] - v2) >> 2;
  else
    v3 = 0;
  if ( !v2 || v3 >= (a2[3] - v2) >> 2 )
    return (_DWORD *)sub_498690((int)a2, (_DWORD *)a2[2], 1u, a1);
  v4 = (_DWORD *)a2[2];
  result = sub_499B40(v4, a1, 1);
  a2[2] = v4 + 1;
  return result;
}

/* ---- sub_4975F0  0x004975F0 ----  VERIFIED */
void __cdecl sub_4975F0(int a1)
{
  if ( *(_DWORD *)(a1 + 4) )
    j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  *(_DWORD *)(a1 + 12) = 0;
}

/* ---- sub_497620  0x00497620 ----  VERIFIED */
_QWORD *__cdecl sub_497620(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(54.0);
  return a1;
}

/* ---- sub_497640  0x00497640 ----  VERIFIED */
int *__cdecl sub_497640( int *pool, int elemSize )
{
  int  cur;
  int *block;
  int *elem;

  cur   = pool[1];
  block = (int *)cur;
  if ( cur )
  {
    while ( !block[8189] )
    {
      block = (int *)block[8190];
      if ( !block )
        goto CLAIM;
    }
    if ( block != (int *)cur )
      sub_497E80( (int)block, (int)pool );
    goto TAKE;
  }

CLAIM:
  block = FxMem_ClaimBlock_m( *pool, elemSize, cur );
  if ( !block )
    return block;
  pool[1] = (int)block;

TAKE:
  elem         = (int *)block[8189];
  block[8189]  = *elem;
  block[8188]  = block[8188] - 1;
  Com_Memset( elem, 0, 0x25Cu );
  return elem;
}

/* ---- sub_4976D0  0x004976D0 ----  VERIFIED */
_DWORD *__fastcall sub_4976D0( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_497E80( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_497740  0x00497740 ----  VERIFIED */
_QWORD *__cdecl sub_497740(_QWORD *a1)
{
  *a1 = (unsigned int)(unsigned __int64)floor(481.0);
  return a1;
}

/* ---- FxMem_AllocScheduledEffect  0x00497750 ----  VERIFIED */
int *__cdecl FxMem_AllocScheduledEffect(int *a1, int a2)
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
      sub_497EE0((int)v4, (int)a1);
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
    Com_Memset(v6, 0, 0x44u);
    return v6;
  }
  return result;
}

/* ---- sub_4977F0  0x004977F0 ----  VERIFIED */
_DWORD *__fastcall sub_4977F0( char *p, int *pool )
{
  _DWORD *block;
  _DWORD *cur;
  int     count;
  int     nxt;
  int     prv;

  block       = (_DWORD *)( (char *)&unk_A9CE58
                            + ( ( p - (char *)&unk_A9CE58 ) & 0xFFFF8000 ) );
  *(_DWORD *)p = block[8189];
  block[8189]  = (_DWORD)p;
  count        = (int)block[8188] + 1;
  block[8188]  = count;

  cur = (_DWORD *)pool[1];
  if ( !cur[8189] )
    return (_DWORD *)sub_497EE0( (int)block, (int)pool );

  if ( count >= pool[0] )
  {
    if ( block == cur )
    {
      nxt = (int)block[8190];                       /* prev, +0x7FF8 */
      if ( !nxt || !*(int *)( nxt + FX_HDR_FREECOUNT ) )
        return block;
      pool[1] = nxt;
    }
    nxt = (int)block[8191];                         /* next, +0x7FFC */
    if ( nxt )
      *(_DWORD *)( nxt + FX_HDR_PREV ) = block[8190];
    prv = (int)block[8190];
    if ( prv )
      *(_DWORD *)( prv + FX_HDR_NEXT ) = block[8191];
    block[8191] = 0;
    block[8190] = 0;
    block[8188] = (_DWORD)-1;
  }
  return block;
}

/* ---- sub_497860  0x00497860 ----  VERIFIED */
int __cdecl sub_497860(int a1)
{
  _DWORD *v1;
  char v3;

  *(_BYTE *)a1 = v3;
  v1 = sub_499550(a1);
  *(_DWORD *)(a1 + 4) = v1;
  *((_BYTE *)v1 + 45) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  **(_DWORD **)(a1 + 4) = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(a1 + 8) = 0;
  return a1;
}

/* ---- sub_497890  0x00497890 ----  VERIFIED */
int __stdcall sub_497890( int tree, void *key )
{
  unsigned char pair[FX_MAP_PAIR_SIZE];
  char          it[8];
  unsigned char *node;

  FXS_RES( pair )  = 15;
  FXS_SIZE( pair ) = 0;
  pair[4]          = 0;
  sub_497C10( (int)pair, (int)key, 0, 0xFFFFFFFF );
  FXP_MAPPED( pair ) = 0;

  node = *(unsigned char **)sub_497F90( tree, (int)it, (void ***)pair );

  if ( FXS_RES( pair ) >= 0x10 )
    free( *(void **)FXS_BX( pair ) );
  return (int)( node + 40 );
}

/* ---- sub_497920  0x00497920 ----  VERIFIED */
int __cdecl sub_497920(int a1)
{
  int result;
  int *v3; // [esp+4h] [ebp-4h] BYREF

  sub_4989B0(a1, &v3, **(int ***)(a1 + 4), *(int **)(a1 + 4));
  j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- sub_497950  0x00497950 ----  [HIGH] */
_DWORD *__cdecl sub_497950(_DWORD *result, int a2)
{
  *result = **(_DWORD **)(a2 + 4);
  return result;
}

/* ---- sub_497960  0x00497960 ----  [HIGH] */
_DWORD *__cdecl sub_497960(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 4);
  return result;
}

/* ---- sub_497970  0x00497970 ----  VERIFIED */
_DWORD *__cdecl sub_497970(int a1)
{
  _DWORD *result;

  sub_498100(*(void ***)(*(_DWORD *)(a1 + 4) + 4));
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  result = *(_DWORD **)(a1 + 4);
  *(_DWORD *)(a1 + 8) = 0;
  *result = result;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
  return result;
}

/* ---- sub_4979A0  0x004979A0 ----  VERIFIED */
int __cdecl sub_4979A0( int *result, int tree, int key )
{
  unsigned char *node;
  unsigned char *head;
  const char    *np;
  unsigned int   nlen;

  node = (unsigned char *)sub_498C30( tree, key );
  head = FXT_HEAD( tree );
  if ( node != head )
  {
    nlen = FXS_SIZE( FXN_KEY( node ) );
    np   = (const char *)FXS_PTR( FXN_KEY( node ) );
    if ( sub_499AB0( FXS_SIZE( key ), key, 0, np, nlen ) < 0 )
      node = head;
  }
  *result = (int)node;
  return (int)result;
}

/* ---- sub_497A00  0x00497A00 ----  [HIGH] */
BOOL __cdecl sub_497A00(_DWORD *a1, _DWORD *a2)
{
  return *a1 != *a2;
}

/* ---- sub_497A10  0x00497A10 ----  [HIGH] */
_DWORD *__cdecl sub_497A10(_DWORD *result)
{
  *result = 0;
  return result;
}

/* ---- sub_497A20  0x00497A20 ----  [HIGH] */
int __cdecl sub_497A20(_DWORD *a1)
{
  return *a1 + 12;
}

/* ---- sub_497A30  0x00497A30 ----  VERIFIED */
int **__cdecl sub_497A30(int a1, int **a2)
{
  sub_498230(a1, a2);
  return a2;
}

/* ---- sub_497A40  0x00497A40 ----  VERIFIED */
int __cdecl sub_497A40(int a1)
{
  *(_DWORD *)(a1 + 4) = sub_498310(a1);
  *(_DWORD *)(a1 + 8) = 0;
  return a1;
}

/* ---- sub_497A60  0x00497A60 ----  VERIFIED */
void __cdecl sub_497A60(int a1)
{
  _DWORD *v2;
  _DWORD *v3;
  int v4; // [esp+4h] [ebp-4h] BYREF

  v2 = *(_DWORD **)(a1 + 4);
  if ( v2 )
    v3 = (_DWORD *)*v2;
  else
    v3 = 0;
  sub_498D60(a1, &v4, v3, v2);
  j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
}

/* ---- sub_497AA0  0x00497AA0 ----  VERIFIED */
_DWORD *__cdecl sub_497AA0(_DWORD *result, int a2)
{
  _DWORD *v2;

  v2 = *(_DWORD **)(a2 + 4);
  if ( v2 )
    *result = *v2;
  else
    *result = 0;
  return result;
}

/* ---- sub_497AB0  0x00497AB0 ----  [HIGH] */
_DWORD *__cdecl sub_497AB0(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 4);
  return result;
}

/* ---- sub_497AC0  0x00497AC0 ----  [HIGH] */
int __cdecl sub_497AC0(int a1)
{
  return *(_DWORD *)(a1 + 8);
}

/* ---- sub_497AD0  0x00497AD0 ----  VERIFIED */
int __cdecl sub_497AD0(int a1, _DWORD *a2)
{
  int *v2;
  int v3;
  _DWORD *v4;
  int result;

  v2 = *(int **)(a1 + 4);
  if ( v2 )
    v3 = *v2;
  else
    v3 = 0;
  v4 = sub_498DB0(a1, v3, *(_DWORD *)(v3 + 4), a2);
  result = sub_498DE0(1u, a1);
  *(_DWORD *)(v3 + 4) = v4;
  *(_DWORD *)v4[1] = v4;
  return result;
}

/* ---- sub_497B10  0x00497B10 ----  VERIFIED */
int __cdecl sub_497B10(_DWORD *a1, int a2)
{
  int v2;
  _DWORD *v3;
  int result;

  v2 = *(_DWORD *)(a2 + 4);
  v3 = sub_498DB0(a2, v2, *(_DWORD *)(v2 + 4), a1);
  result = sub_498DE0(1u, a2);
  *(_DWORD *)(v2 + 4) = v3;
  *(_DWORD *)v3[1] = v3;
  return result;
}

/* ---- sub_497B40  0x00497B40 ----  VERIFIED */
_DWORD *__cdecl sub_497B40(_DWORD *a1, int a2, _DWORD **Block)
{
  int v3;

  v3 = (int)*Block;
  if ( Block != *(_DWORD ***)(a2 + 4) )
  {
    *Block[1] = *Block;
    (*Block)[1] = Block[1];
    j__free(Block);
    --*(_DWORD *)(a2 + 8);
  }
  *a1 = v3;
  return a1;
}

/* ---- sub_497B80  0x00497B80 ----  [HIGH] */
BOOL __cdecl sub_497B80(_DWORD *a1, _DWORD *a2)
{
  return *a1 != *a2;
}

/* ---- sub_497B90  0x00497B90 ----  [HIGH] */
_DWORD *__cdecl sub_497B90(_DWORD *result)
{
  *result = 0;
  return result;
}

/* ---- sub_497BA0  0x00497BA0 ----  [HIGH] */
int __cdecl sub_497BA0(_DWORD *a1)
{
  return *a1 + 8;
}

/* ---- sub_497BB0  0x00497BB0 ----  [HIGH] */
_DWORD **__cdecl sub_497BB0(_DWORD **result)
{
  *result = (_DWORD *)**result;
  return result;
}

/* ---- sub_497BC0  0x00497BC0 ----  VERIFIED */
_DWORD *__cdecl sub_497BC0(_DWORD *result, _DWORD **a2, int a3)
{
  _DWORD **v3;

  v3 = (_DWORD **)*a2;
  *a2 = (_DWORD *)**a2;
  *result = v3;
  return result;
}

/* ---- sub_497C00  0x00497C00 ----  VERIFIED */
void ***__cdecl sub_497C00(void ***a1, void ***a2)
{
  return sub_497C10(a2, a1, 0, (void **)0xFFFFFFFF);
}

/* ---- sub_497C10  0x00497C10 ----  VERIFIED */
int __cdecl sub_497C10( int self, int src, unsigned int off, unsigned int count )
{
  unsigned int   n;
  unsigned char *dst;

  /* retail 0x00497C1F: if ( str._Mysize < off ) std::_String_base::_Xran(); */

  n = FXS_SIZE( src ) - off;
  if ( count < n )
    n = count;

  if ( self == src )
  {
    sub_4984B0( self, off + n, 0xFFFFFFFF );
    sub_4984B0( self, 0, off );
    return self;
  }

  if ( sub_498590( self, n, 1 ) )
  {
    memcpy( FXS_PTR( self ), FXS_PTR( src ) + off, n );
    FXS_SIZE( self ) = n;
    dst = FXS_PTR( self );
    dst[n] = 0;
  }
  return self;
}

/* ---- sub_497CC0  0x00497CC0 ----  VERIFIED */
void ***__cdecl sub_497CC0(void *this, char *a2)
{
  return sub_498410((int)this, a2, (void **)strlen(a2));
}

/* ---- unknown_libname_3  0x00497CF0 ----  VERIFIED */
_DWORD *__cdecl unknown_libname_3(_DWORD *this)
{
  if ( this[6] < 0x10u )
    return this + 1;
  else
    return (_DWORD *)this[1];
}

/* ---- sub_497D00  0x00497D00 ----  VERIFIED */
void __cdecl sub_497D00(int this, char a2)
{
  if ( a2 )
  {
    if ( *(_DWORD *)(this + 24) >= 0x10u )
      j__free(*(void **)(this + 4));
  }
  *(_DWORD *)(this + 24) = 15;
  *(_DWORD *)(this + 20) = 0;
  *(_BYTE *)(this + 4) = 0;
}

/* ---- type_info__operator__type_info  0x00497D40 ----  [HIGH] */
void *__cdecl type_info__operator__type_info(void *this, int a2)
{
  return this;
}

/* ---- sub_497D70  0x00497D70 ----  VERIFIED */
int __cdecl sub_497D70(int a1)
{
  int v1;

  v1 = *(_DWORD *)(a1 + 4);
  if ( v1 )
    return (*(_DWORD *)(a1 + 12) - v1) >> 2;
  else
    return 0;
}

/* ---- sub_497D90  0x00497D90 ----  [HIGH] */
_DWORD *__cdecl sub_497D90(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 4);
  return result;
}

/* ---- sub_497DA0  0x00497DA0 ----  [HIGH] */
_DWORD *__cdecl sub_497DA0(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 4);
  return result;
}

/* ---- sub_497DB0  0x00497DB0 ----  [HIGH] */
_DWORD *__cdecl sub_497DB0(_DWORD *result, int a2)
{
  *result = *(_DWORD *)(a2 + 8);
  return result;
}

/* ---- sub_497DC0  0x00497DC0 ----  VERIFIED */
_DWORD *__cdecl sub_497DC0(_DWORD *a1, int a2, _DWORD *a3, _DWORD *a4)
{
  int v4;
  int v5;

  v4 = *(_DWORD *)(a2 + 4);
  if ( v4 && (*(_DWORD *)(a2 + 8) - v4) >> 2 )
    v5 = ((int)a3 - v4) >> 2;
  else
    v5 = 0;
  sub_498690(a2, a3, 1u, a4);
  *a1 = *(_DWORD *)(a2 + 4) + 4 * v5;
  return a1;
}

/* ---- sub_497E10  0x00497E10 ----  VERIFIED */
void __cdecl sub_497E10(int a1)
{
  if ( *(_DWORD *)(a1 + 4) )
    j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  *(_DWORD *)(a1 + 12) = 0;
}

/* ---- sub_497E40  0x00497E40 ----  VERIFIED */
_DWORD *__cdecl sub_497E40(_DWORD *a1, _DWORD *a2, int a3)
{
  sub_499B40(a2, a1, a3);
  return &a2[a3];
}

/* ---- sub_497E60  0x00497E60 ----  [HIGH] */
int __cdecl sub_497E60(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_497E70  0x00497E70 ----  [HIGH] */
_DWORD *__cdecl sub_497E70(_DWORD *result, _DWORD *a2, int a3)
{
  *result = *a2 + 4 * a3;
  return result;
}

/* ---- sub_497E80  0x00497E80 ----  VERIFIED */
int __cdecl sub_497E80(int result, int a2)
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

/* ---- sub_497EE0  0x00497EE0 ----  VERIFIED */
int __cdecl sub_497EE0(int result, int a2)
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

/* ---- sub_497F40  0x00497F40 ----  [HIGH] */
int __cdecl sub_497F40(int a1)
{
  return a1 + 12;
}

/* ---- sub_497F50  0x00497F50 ----  [HIGH] */
int __cdecl sub_497F50(int a1)
{
  return a1 + 12;
}

/* ---- sub_497F60  0x00497F60 ----  VERIFIED */
int __cdecl sub_497F60(_BYTE *a1, int a2, int a3)
{
  _DWORD *v3;

  *(_BYTE *)a2 = *a1;
  v3 = sub_499550(a2);
  *(_DWORD *)(a2 + 4) = v3;
  *((_BYTE *)v3 + 45) = 1;
  *(_DWORD *)(*(_DWORD *)(a2 + 4) + 4) = *(_DWORD *)(a2 + 4);
  **(_DWORD **)(a2 + 4) = *(_DWORD *)(a2 + 4);
  *(_DWORD *)(*(_DWORD *)(a2 + 4) + 8) = *(_DWORD *)(a2 + 4);
  *(_DWORD *)(a2 + 8) = 0;
  return a2;
}

/* ---- sub_497F90  0x00497F90 ----  VERIFIED */
int __stdcall sub_497F90( int tree, int result, void ***val )
{
  unsigned char *node;
  unsigned char *where;
  unsigned char *pred;
  unsigned char *key = (unsigned char *)val;
  char           addleft;
  unsigned int   vlen, nlen, n;
  int            ans;

  where   = FXT_HEAD( tree );
  node    = FXN_PARENT( where );
  addleft = 1;

  if ( !FXN_ISNIL( node ) )
  {
    vlen = FXS_SIZE( key );
    do
    {
      where = node;
      nlen  = FXS_SIZE( FXN_KEY( node ) );

      ans = 0;
      if ( vlen )
      {
        n   = vlen < nlen ? vlen : nlen;
        ans = memcmp( FXS_PTR( key ), FXS_PTR( FXN_KEY( node ) ), n );
        if ( ans )
          ans = ans < 0 ? -1 : 1;
      }
      if ( ans == 0 )
        ans = ( vlen < nlen ) ? -1 : ( vlen != nlen );

      addleft = (char)( ans < 0 );
      node = addleft ? FXN_LEFT( node ) : FXN_RIGHT( node );
    }
    while ( !FXN_ISNIL( node ) );
  }

  pred = where;
  if ( addleft )
  {
    if ( where == FXN_LEFT( FXT_HEAD( tree ) ) )
    {
      *(int *)result       = *(int *)sub_498A60( (int)where, tree, (void ***)&pred, 1, (int)key );
      *(char *)( result + 4 ) = 1;
      return result;
    }
    sub_499650( 0, (int **)&pred );
  }

  if ( sub_499AB0( FXS_SIZE( FXN_KEY( pred ) ), (int)FXN_KEY( pred ), 0,
                   (const char *)FXS_PTR( key ), FXS_SIZE( key ) ) < 0 )
  {
    *(int *)result       = *(int *)sub_498A60( (int)where, tree, (void ***)&pred, addleft, (int)key );
    *(char *)( result + 4 ) = 1;
    return result;
  }

  *(int *)result          = (int)pred;
  *(char *)( result + 4 ) = 0;
  return result;
}

/* ---- sub_4980F0  0x004980F0 ----  VERIFIED */
int **__cdecl sub_4980F0(int a1, int a2, int **a3)
{
  *a3 = sub_498C30(a1, a2);
  return a3;
}

/* ---- sub_498100  0x00498100 ----  VERIFIED */
char __stdcall sub_498100( void **node )
{
  void **p;
  void **q;
  char   isnil;

  p     = node;
  isnil = *( (char *)node + 45 );
  while ( !isnil )
  {
    sub_498100( (void **)p[2] );
    q = (void **)p[0];
    sub_499A30( (int)p );
    free( p );
    p     = q;
    isnil = *( (char *)p + 45 );
  }
  return isnil;
}

/* ---- sub_498140  0x00498140 ----  [HIGH] */
int __cdecl sub_498140(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

/* ---- sub_498150  0x00498150 ----  [HIGH] */
int __cdecl sub_498150(int a1)
{
  return *(_DWORD *)(a1 + 4) + 8;
}

/* ---- sub_498160  0x00498160 ----  [HIGH] */
int __cdecl sub_498160(int a1)
{
  return *(_DWORD *)(a1 + 4) + 4;
}

/* ---- sub_498170  0x00498170 ----  VERIFIED */
int __cdecl sub_498170(int a1)
{
  int result;
  int *v3; // [esp+4h] [ebp-4h] BYREF

  sub_4989B0(a1, &v3, **(int ***)(a1 + 4), *(int **)(a1 + 4));
  j__free(*(void **)(a1 + 4));
  result = 0;
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- nullsub_39  0x004981A0 ----  [HIGH] */
void nullsub_39()
{
  ;
}

/* ---- sub_4981B0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4981B0() { return 0; }
#if 0
bool __cdecl sub_4981B0(_DWORD *a1, std::_String_base *a2, int a3)
{
  unsigned int v3;
  const char *v4;

  v3 = a1[5];
  if ( a1[6] < 0x10u )
    v4 = (const char *)(a1 + 1);
  else
    v4 = (const char *)a1[1];
  return sub_499AB0(*((_DWORD *)a2 + 5), a2, 0, v4, v3) < 0;
}
#endif

/* ---- sub_4981E0  0x004981E0 ----  VERIFIED */
int __cdecl sub_4981E0(int a1, void ****a2, _DWORD *a3)
{
  *(_DWORD *)(a1 + 20) = 0;
  *(_DWORD *)(a1 + 24) = 15;
  *(_BYTE *)(a1 + 4) = 0;
  sub_497C10((void ****)a1, a2, 0, (void **)0xFFFFFFFF);
  *(_DWORD *)(a1 + 28) = *a3;
  return a1;
}

/* ---- sub_498210  0x00498210 ----  [HIGH] */
_DWORD *__cdecl sub_498210(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- sub_498220  0x00498220 ----  [HIGH] */
BOOL __cdecl sub_498220(_DWORD *a1, _DWORD *a2)
{
  return *a2 == *a1;
}

/* ---- sub_498230  0x00498230 ----  VERIFIED */
int *__fastcall sub_498230( int unused, int **pptr )
{
  int *p;
  int *q;

  (void)unused;
  p = *pptr;
  if ( !*( (char *)p + 45 ) )
  {
    q = (int *)p[2];
    if ( *( (char *)q + 45 ) )
    {
      for ( p = (int *)p[1]; !*( (char *)p + 45 ); p = (int *)p[1] )
      {
        if ( *pptr != (int *)p[2] )
          break;
        *pptr = p;
      }
      *pptr = p;
    }
    else
    {
      p = (int *)q[0];
      if ( !*( (char *)p + 45 ) )
      {
        do
        {
          q = p;
          p = (int *)p[0];
        }
        while ( !*( (char *)p + 45 ) );
      }
      *pptr = q;
    }
  }
  return p;
}

/* ---- sub_498290  0x00498290 ----  [HIGH] */
int __cdecl sub_498290(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_4982A0  0x004982A0 ----  [HIGH] */
_DWORD *__cdecl sub_4982A0(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- nullsub_40  0x004982B0 ----  [HIGH] */
void nullsub_40()
{
  ;
}

/* ---- sub_4982C0  0x004982C0 ----  [HIGH] */
int __cdecl sub_4982C0(int a1)
{
  return a1 + 4;
}

/* ---- sub_4982D0  0x004982D0 ----  [HIGH] */
int __cdecl sub_4982D0(int a1)
{
  return a1 + 8;
}

/* ---- sub_4982E0  0x004982E0 ----  VERIFIED */
int __cdecl sub_4982E0(_DWORD *a1, int a2, int a3)
{
  _DWORD *v3;
  int result;

  v3 = sub_498DB0(a2, a3, *(_DWORD *)(a3 + 4), a1);
  result = sub_498DE0(1u, a2);
  *(_DWORD *)(a3 + 4) = v3;
  *(_DWORD *)v3[1] = v3;
  return result;
}

/* ---- sub_498310  0x00498310 ----  VERIFIED */
_DWORD *__stdcall sub_498310( int list )
{
  _DWORD *p;

  (void)list;
  p = (_DWORD *)malloc( 0xC );
  if ( p )
    p[0] = (_DWORD)p;
  if ( p != (_DWORD *)-4 )
    p[1] = (_DWORD)p;
  return p;
}

/* ---- sub_498330  0x00498330 ----  VERIFIED */
void __cdecl sub_498330(int a1)
{
  _DWORD *v2;
  _DWORD *v3;
  int v4; // [esp+4h] [ebp-4h] BYREF

  v2 = *(_DWORD **)(a1 + 4);
  if ( v2 )
    v3 = (_DWORD *)*v2;
  else
    v3 = 0;
  sub_498D60(a1, &v4, v3, v2);
  j__free(*(void **)(a1 + 4));
  *(_DWORD *)(a1 + 4) = 0;
  *(_DWORD *)(a1 + 8) = 0;
}

/* ---- nullsub_41  0x00498370 ----  [HIGH] */
void __stdcall nullsub_41(int a1)
{
  ;
}

/* ---- nullsub_42  0x00498380 ----  [HIGH] */
void nullsub_42()
{
  ;
}

/* ---- sub_498390  0x00498390 ----  VERIFIED */
void __cdecl sub_498390(void *a1, int a2, int a3)
{
  j__free(a1);
}

/* ---- nullsub_43  0x004983A0 ----  [HIGH] */
void __stdcall nullsub_43(int a1, int a2)
{
  ;
}

/* ---- sub_4983B0  0x004983B0 ----  [HIGH] */
_DWORD *__cdecl sub_4983B0(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- sub_4983C0  0x004983C0 ----  [HIGH] */
BOOL __cdecl sub_4983C0(_DWORD *a1, _DWORD *a2)
{
  return *a2 == *a1;
}

/* ---- sub_4983D0  0x004983D0 ----  [HIGH] */
int __cdecl sub_4983D0(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_4983E0  0x004983E0 ----  [HIGH] */
_DWORD *__cdecl sub_4983E0(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- sub_4983F0  0x004983F0 ----  [HIGH] */
int __cdecl sub_4983F0(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_498400  0x00498400 ----  [HIGH] */
_DWORD *__cdecl sub_498400(_DWORD *result, _DWORD *a2, int a3)
{
  *result = *a2 + 4 * a3;
  return result;
}

/* ---- sub_498410  0x00498410 ----  VERIFIED */
int __cdecl sub_498410( int self, const char *ptr, unsigned int count )
{
  unsigned char *data;
  unsigned char *dst;

  data = FXS_PTR( self );
  if ( (const char *)data <= ptr )
  {
    data = FXS_PTR( self );
    if ( ptr < (const char *)( data + FXS_SIZE( self ) ) )
      return sub_497C10( self, self, (unsigned int)( ptr - (const char *)FXS_PTR( self ) ),
                         count );
  }

  if ( sub_498590( self, count, 1 ) )
  {
    memcpy( FXS_PTR( self ), ptr, count );
    FXS_SIZE( self ) = count;
    dst = FXS_PTR( self );
    dst[count] = 0;
  }
  return self;
}

/* ---- sub_4984B0  0x004984B0 ----  VERIFIED */
int __cdecl sub_4984B0( int self, unsigned int off, unsigned int count )
{
  unsigned int   mysize, newlen;
  unsigned char *p;

  mysize = FXS_SIZE( self );
  /* retail 0x004984BA: if ( _Mysize < off ) _Xran(); */
  if ( mysize - off < count )
    count = mysize - off;
  if ( count )
  {
    p = FXS_PTR( self );
    memcpy( p + off, p + off + count, mysize - off - count );

    newlen = FXS_SIZE( self ) - count;
    /* retail 0x0049850D: if ( newlen > 0xFFFFFFFE ) _Xlen(); */
    if ( FXS_RES( self ) < newlen )
      sub_498F10( self, newlen, mysize );

    FXS_SIZE( self ) = newlen;
    p = FXS_PTR( self );
    p[newlen] = 0;
  }
  return self;
}

/* ---- sub_498570  0x00498570 ----  VERIFIED */
int __cdecl sub_498570(_DWORD *this, int a2)
{
  bool v2; // cf
  int result;

  v2 = this[6] < 0x10u;
  result = a2;
  this[5] = a2;
  if ( v2 )
    *((_BYTE *)this + a2 + 4) = 0;
  else
    *(_BYTE *)(this[1] + a2) = 0;
  return result;
}

/* ---- sub_498590  0x00498590 ----  VERIFIED */
int __cdecl sub_498590( int self, unsigned int n, char trim )
{
  unsigned int res;

  /* retail 0x00498596: if ( n > 0xFFFFFFFE ) _Xlen(); */
  res = FXS_RES( self );

  if ( res < n )
  {
    sub_498F10( self, n, trim ? 0 : FXS_SIZE( self ) );
    return n != 0;
  }

  if ( trim && n < 16 )
  {
    if ( res >= 16 )
      free( *(void **)FXS_BX( self ) );
    FXS_RES( self )  = 15;
    FXS_SIZE( self ) = 0;
    *FXS_BX( self )  = 0;
    return n != 0;
  }

  if ( n == 0 )
  {
    FXS_SIZE( self ) = 0;
    *FXS_PTR( self ) = 0;
    return 0;
  }
  return 1;
}

/* ---- unknown_libname_4  0x00498596 ----  [UNKNOWN]  AUTO-STUBBED */
int unknown_libname_4() { return 0; }
#if 0
// Microsoft VisualC 2-14/net runtime
_DWORD *__cdecl unknown_libname_4(_DWORD *this)
{
  if ( this[6] < 0x10u )
    return this + 1;
  else
    return (_DWORD *)this[1];
}
#endif

/* ---- sub_498670  0x00498670 ----  VERIFIED */
void __stdcall sub_498670(void *Block, int a2)
{
  j__free(Block);
}

/* ---- nullsub_44  0x00498680 ----  [HIGH] */
void __stdcall nullsub_44(int a1, int a2, int a3)
{
  ;
}

/* ---- sub_498690  0x00498690 ----  VERIFIED */
unsigned int __cdecl sub_498690( int this, _DWORD *where, unsigned int count,
                                 _DWORD *pval )
{
  _DWORD      *first;
  _DWORD      *last;
  _DWORD      *newbuf;
  _DWORD      *mid;
  _DWORD      *src;
  _DWORD      *out;
  unsigned int capacity;
  unsigned int size;
  unsigned int newcap;
  unsigned int total;
  int          val;

  val      = *pval;
  first    = *(_DWORD **)( this + 4 );
  capacity = first ? (unsigned int)( ( *(int *)( this + 12 ) - (int)first ) >> 2 )
                   : 0;
  if ( !count )
    return capacity;

  size = first ? (unsigned int)( ( *(int *)( this + 8 ) - (int)first ) >> 2 ) : 0;
  if ( 0x3FFFFFFF - size < count )
  {
    sub_499080( this );
    return capacity;
  }

  if ( capacity < size + count )
  {
    newcap = ( 0x3FFFFFFF - ( capacity >> 1 ) >= capacity )
             ? capacity + ( capacity >> 1 ) : 0;
    if ( newcap < size + count )
      newcap = size + count;

    newbuf = (_DWORD *)malloc( 4 * newcap );

    mid = sub_499C20( newbuf, *(_DWORD **)( this + 4 ), where );
    sub_499B40( mid, &val, count );
    sub_499C20( mid + count, where, *(_DWORD **)( this + 8 ) );

    total = size + count;
    if ( *(_DWORD *)( this + 4 ) )
      free( *(void **)( this + 4 ) );

    *(_DWORD *)( this + 12 ) = (_DWORD)( (char *)newbuf + 4 * newcap );
    *(_DWORD *)( this + 8 )  = (_DWORD)( newbuf + total );
    *(_DWORD *)( this + 4 )  = (_DWORD)newbuf;
    return (unsigned int)( (char *)newbuf + 4 * newcap );
  }

  last = *(_DWORD **)( this + 8 );
  if ( (unsigned int)( last - where ) >= count )
  {
    src = last - count;
    *(_DWORD **)( this + 8 ) = sub_4998B0( src, last, last );
    sub_499900( &out, (int)where, (int)src, last );
    return (unsigned int)sub_4998E0( &val, where, where + count );
  }

  sub_4998B0( where, last, where + count );
  last = *(_DWORD **)( this + 8 );
  sub_499B40( last, &val, count - (unsigned int)( last - where ) );
  *(_DWORD *)( this + 8 ) += 4 * count;
  return (unsigned int)sub_4998E0( &val, where, last );
}

/* ---- sub_498930  0x00498930 ----  VERIFIED */
void __cdecl sub_498930(void *a1, int a2, int a3)
{
  j__free(a1);
}

/* ---- sub_498940  0x00498940 ----  [HIGH] */
_DWORD *__cdecl sub_498940(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- sub_498950  0x00498950 ----  [HIGH] */
_DWORD *__cdecl sub_498950(_DWORD *result, _DWORD *a2)
{
  *result = *a2;
  return result;
}

/* ---- sub_498960  0x00498960 ----  [HIGH] */
int __cdecl sub_498960(_DWORD *a1, _DWORD *a2)
{
  return (*a1 - *a2) >> 2;
}

/* ---- sub_498970  0x00498970 ----  [HIGH] */
int __cdecl sub_498970(int a1)
{
  return a1 + 45;
}

/* ---- nullsub_45  0x00498980 ----  [HIGH] */
void nullsub_45()
{
  ;
}

/* ---- sub_498990  0x00498990 ----  [HIGH] */
int __cdecl sub_498990(int a1)
{
  return a1 + 4;
}

/* ---- sub_4989A0  0x004989A0 ----  [HIGH] */
int __cdecl sub_4989A0(int a1)
{
  return a1 + 8;
}

/* ---- sub_4989B0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4989B0() { return 0; }
#if 0
int **__cdecl sub_4989B0(int a1, int **a2, int *a3, int *a4)
{
  int v4;
  int *v5;
  int *v6;
  int **v7;
  _DWORD *v8;
  int *v10;
  void *v11;

  v4 = *(_DWORD *)(a1 + 4);
  v5 = a3;
  v6 = a4;
  v7 = a2;
  if ( a3 == *(int **)v4 && a4 == (int *)v4 )
  {
    sub_498100(*(void ***)(v4 + 4));
    *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
    v8 = *(_DWORD **)(a1 + 4);
    *(_DWORD *)(a1 + 8) = 0;
    *v8 = v8;
    *(_DWORD *)(*(_DWORD *)(a1 + 4) + 8) = *(_DWORD *)(a1 + 4);
    *v7 = **(int ***)(a1 + 4);
    return v7;
  }
  else
  {
    if ( a3 != a4 )
    {
      do
      {
        v10 = v5;
        sub_498230(v4, &a3);
        sub_499170(v11, a1, (int **)&a2, v10);
        v5 = a3;
      }
      while ( a3 != v6 );
    }
    *v7 = v5;
    return v7;
  }
}
#endif

/* ---- sub_498A30  0x00498A30 ----  VERIFIED */
int __cdecl sub_498A30(int a1)
{
  _DWORD *v1;
  int result;

  v1 = sub_499550(a1);
  *(_DWORD *)(a1 + 4) = v1;
  *((_BYTE *)v1 + 45) = 1;
  *(_DWORD *)(*(_DWORD *)(a1 + 4) + 4) = *(_DWORD *)(a1 + 4);
  **(_DWORD **)(a1 + 4) = *(_DWORD *)(a1 + 4);
  result = *(_DWORD *)(a1 + 4);
  *(_DWORD *)(result + 8) = result;
  *(_DWORD *)(a1 + 8) = 0;
  return result;
}

/* ---- sub_498A60  0x00498A60 ----  VERIFIED */
int __cdecl sub_498A60( int wherenode, int tree, void ***result,
                        char addleft, int val )
{
  unsigned char *head  = FXT_HEAD( tree );
  unsigned char *where = (unsigned char *)wherenode;
  unsigned char *node;
  unsigned char *cur, *parent, *grand, *uncle;

  node = (unsigned char *)sub_499590( tree, (int)head, (int)where, (int)head,
                                      val, FX_RED );
  FXT_SIZE( tree )++;

  head = FXT_HEAD( tree );
  if ( where == head )
  {
    FXN_PARENT( head ) = node;
    FXN_LEFT  ( FXT_HEAD( tree ) ) = node;
    FXN_RIGHT ( FXT_HEAD( tree ) ) = node;
  }
  else if ( addleft )
  {
    FXN_LEFT( where ) = node;
    head = FXT_HEAD( tree );
    if ( where == FXN_LEFT( head ) )
      FXN_LEFT( head ) = node;
  }
  else
  {
    FXN_RIGHT( where ) = node;
    head = FXT_HEAD( tree );
    if ( where == FXN_RIGHT( head ) )
      FXN_RIGHT( head ) = node;
  }

  cur = node;
  while ( FXN_COLOR( FXN_PARENT( cur ) ) == FX_RED )
  {
    parent = FXN_PARENT( cur );
    grand  = FXN_PARENT( parent );

    if ( parent == FXN_LEFT( grand ) )
    {
      uncle = FXN_RIGHT( grand );
      if ( FXN_COLOR( uncle ) == FX_RED )
      {
        FXN_COLOR( parent ) = FX_BLACK;
        FXN_COLOR( uncle )  = FX_BLACK;
        FXN_COLOR( FXN_PARENT( FXN_PARENT( cur ) ) ) = FX_RED;
        cur = FXN_PARENT( FXN_PARENT( cur ) );
      }
      else
      {
        if ( cur == FXN_RIGHT( parent ) )
        {
          cur = parent;
          sub_499480( (int)cur, tree );
        }
        FXN_COLOR( FXN_PARENT( cur ) ) = FX_BLACK;
        FXN_COLOR( FXN_PARENT( FXN_PARENT( cur ) ) ) = FX_RED;
        sub_4994F0( (int)FXN_PARENT( FXN_PARENT( cur ) ), tree );
      }
    }
    else
    {
      uncle = FXN_LEFT( grand );
      if ( FXN_COLOR( uncle ) == FX_RED )
      {
        FXN_COLOR( parent ) = FX_BLACK;
        FXN_COLOR( uncle )  = FX_BLACK;
        FXN_COLOR( FXN_PARENT( FXN_PARENT( cur ) ) ) = FX_RED;
        cur = FXN_PARENT( FXN_PARENT( cur ) );
      }
      else
      {
        if ( cur == FXN_LEFT( parent ) )
        {
          cur = parent;
          sub_4994F0( (int)cur, tree );
        }
        FXN_COLOR( FXN_PARENT( cur ) ) = FX_BLACK;
        FXN_COLOR( FXN_PARENT( FXN_PARENT( cur ) ) ) = FX_RED;
        sub_499480( (int)FXN_PARENT( FXN_PARENT( cur ) ), tree );
      }
    }
  }

  FXN_COLOR( FXN_PARENT( FXT_HEAD( tree ) ) ) = FX_BLACK;
  *(int *)result = (int)node;
  return (int)result;
}

/* ---- sub_498C30  0x00498C30 ----  VERIFIED */
int __cdecl sub_498C30( int tree, int key )
{
  unsigned char *node  = FXN_PARENT( FXT_HEAD( tree ) );
  unsigned char *where = FXT_HEAD( tree );
  unsigned int   klen  = FXS_SIZE( key );
  unsigned int   nlen, n;
  int            ans;

  while ( !FXN_ISNIL( node ) )
  {
    nlen = FXS_SIZE( FXN_KEY( node ) );

    ans = 0;
    if ( nlen )
    {
      n   = nlen < klen ? nlen : klen;
      ans = memcmp( FXS_PTR( FXN_KEY( node ) ), FXS_PTR( key ), n );
      if ( ans )
        ans = ans < 0 ? -1 : 1;
    }
    if ( ans == 0 )
      ans = ( nlen < klen ) ? -1 : ( nlen != klen );

    if ( ans < 0 )
    {
      node = FXN_RIGHT( node );
    }
    else
    {
      where = node;
      node  = FXN_LEFT( node );
    }
  }
  return (int)where;
}

/* ---- sub_498CD0  0x00498CD0 ----  VERIFIED */
_DWORD *__cdecl sub_498CD0(_DWORD *result)
{
  int *v1;

  v1 = (int *)*result;
  if ( !*(_BYTE *)(*result + 45) )
  {
    do
    {
      result = v1;
      v1 = (int *)*v1;
    }
    while ( !*((_BYTE *)v1 + 45) );
  }
  return result;
}

/* ---- sub_498CF0  0x00498CF0 ----  [HIGH] */
_BYTE *__cdecl sub_498CF0(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- nullsub_46  0x00498D00 ----  [HIGH] */
void nullsub_46()
{
  ;
}

/* ---- nullsub_47  0x00498D10 ----  [HIGH] */
void __stdcall nullsub_47(int a1)
{
  ;
}

/* ---- sub_498D20  0x00498D20 ----  VERIFIED */
void __cdecl sub_498D20(void *a1, int a2, int a3)
{
  j__free(a1);
}

/* ---- sub_498D30  0x00498D30 ----  VERIFIED */
int __cdecl sub_498D30(int a1, int a2)
{
  return sub_499A30(a1);
}

/* ---- nullsub_48  0x00498D40 ----  [HIGH] */
void __stdcall nullsub_48(int a1, int a2)
{
  ;
}

/* ---- sub_498D50  0x00498D50 ----  VERIFIED */
int **__cdecl sub_498D50(int a1, int **a2)
{
  sub_499650(a1, a2);
  return a2;
}

/* ---- sub_498D60  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_498D60() { return 0; }
#if 0
_DWORD *__cdecl sub_498D60(int a1, _DWORD *a2, _DWORD *Block, _DWORD *a4)
{
  _DWORD *v4;
  _DWORD **v5;
  bool v6; // zf

  v4 = Block;
  while ( v4 != a4 )
  {
    v5 = (_DWORD **)v4;
    v6 = v4 == *(_DWORD **)(a1 + 4);
    v4 = (_DWORD *)*v4;
    if ( !v6 )
    {
      *v5[1] = *v5;
      (*v5)[1] = v5[1];
      j__free(v5);
      --*(_DWORD *)(a1 + 8);
    }
  }
  *a2 = v4;
  return a2;
}
#endif

/* ---- sub_498DB0  0x00498DB0 ----  VERIFIED */
_DWORD *__stdcall sub_498DB0( int list, int next, int prev, _DWORD *val )
{
  _DWORD *p;

  (void)list;
  p = (_DWORD *)malloc( 0xC );
  if ( p )
  {
    p[0] = (_DWORD)next;
    p[1] = (_DWORD)prev;
    p[2] = *val;
  }
  return p;
}

/* ---- sub_498DE0  0x00498DE0 ----  VERIFIED */
int __fastcall sub_498DE0( unsigned int n, int list )
{
  unsigned int size;

  size = *(unsigned int *)( list + 8 );
  if ( 0x3FFFFFFF - size < n )
    Com_Error( ERR_FATAL, "list<T> too long" );
  *(unsigned int *)( list + 8 ) = size + n;
  return (int)( size + n );
}

/* ---- nullsub_49  0x00498E80 ----  [HIGH] */
void __stdcall nullsub_49(int a1)
{
  ;
}

/* ---- nullsub_50  0x00498E90 ----  [HIGH] */
void __stdcall nullsub_50(int a1)
{
  ;
}

/* ---- sub_498EA0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_498EA0() { return 0; }
#if 0
void *__cdecl sub_498EA0(int a1, int a2, int a3)
{
  return operator new(12 * a1);
}
#endif

/* ---- sub_498EC0  0x00498EC0 ----  VERIFIED */
_DWORD *__cdecl sub_498EC0(_DWORD *result, _DWORD *a2, int a3)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- nullsub_51  0x00498ED0 ----  [HIGH] */
void __stdcall nullsub_51(int a1, int a2)
{
  ;
}

/* ---- sub_498EE0  0x00498EE0 ----  [HIGH] */
_DWORD *__cdecl sub_498EE0(_DWORD *result, int a2)
{
  *result = a2;
  return result;
}

/* ---- sub_498EF0  0x00498EF0 ----  [HIGH] */
int __cdecl sub_498EF0(int result, _DWORD *a2, _BYTE *a3)
{
  *(_DWORD *)result = *a2;
  *(_BYTE *)(result + 4) = *a3;
  return result;
}

/* ---- sub_498F00  0x00498F00 ----  [HIGH] */
int sub_498F00()
{
  return -2;
}

/* ---- sub_498F10  0x00498F10 ----  VERIFIED */
int __cdecl sub_498F10( int self, unsigned int n, unsigned int oldlen )
{
  unsigned char *p;
  unsigned int   res;

  res = n | 15;
  if ( res > 0xFFFFFFFE )
    res = n;

  p = (unsigned char *)malloc( res + 1 );

  if ( oldlen )
    memcpy( p, FXS_PTR( self ), oldlen );

  if ( FXS_RES( self ) >= 16 )
    free( *(void **)FXS_BX( self ) );

  *FXS_BX( self )                  = 0;
  *(unsigned char **)FXS_BX( self ) = p;
  FXS_RES ( self ) = res;
  FXS_SIZE( self ) = oldlen;

  if ( res < 16 )
    FXS_BX( self )[oldlen] = 0;
  else
    p[oldlen] = 0;
  return (int)p;
}

/* ---- sub_499030  0x00499030 ----  VERIFIED */
BOOL __fastcall sub_499030(_DWORD *a1, int a2, unsigned int a3)
{
  unsigned int v3;
  _DWORD *v4;
  _DWORD *v5;

  v3 = a1[6];
  v4 = a1 + 1;
  if ( v3 < 0x10 )
    v5 = a1 + 1;
  else
    v5 = (_DWORD *)*v4;
  if ( (unsigned int)v5 > a3 )
    return 0;
  if ( v3 >= 0x10 )
    v4 = (_DWORD *)*v4;
  return a3 < (unsigned int)v4 + a1[5];
}

/* ---- sub_499070  0x00499070 ----  [HIGH] */
int __stdcall sub_499070(int a1)
{
  return 0x3FFFFFFF;
}

/* ---- sub_499080  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499080() { return 0; }
#if 0
void  sub_499080()
{
  void **v0; // [esp+0h] [ebp-50h] BYREF
  char v1;
  int v2;
  int v3;
  _DWORD pExceptionObject[13]; // [esp+1Ch] [ebp-34h] BYREF

  v3 = 15;
  v2 = 0;
  v1 = 0;
  sub_498410((int)&v0, "vector<T> too long", (void **)strlen("vector<T> too long"));
  pExceptionObject[12] = 0;
  sub_492500((int)pExceptionObject, &v0);
  pExceptionObject[0] = &std::length_error::`vftable';
  CxxThrowException_x_x_(pExceptionObject, (_ThrowInfo *)&_TI3_AVlength_error_std__);
}
#endif

/* ---- sub_499100  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499100() { return 0; }
#if 0
void *__cdecl sub_499100(int a1, int a2, int a3)
{
  return operator new(4 * a1);
}
#endif

/* ---- sub_499120  0x00499120 ----  [HIGH] */
int __cdecl sub_499120(int a1)
{
  return *(_DWORD *)a1;
}

/* ---- sub_499130  0x00499130 ----  [HIGH] */
_DWORD *__cdecl sub_499130(_DWORD *result, int a2, _DWORD *a3)
{
  *result = *a3 - 4 * a2;
  return result;
}

/* ---- sub_499150  0x00499150 ----  [HIGH] */
int __cdecl sub_499150(int a1)
{
  return a1 + 44;
}

/* ---- sub_499160  0x00499160 ----  [HIGH] */
int __stdcall sub_499160(int a1)
{
  return 0x7FFFFFF;
}

/* ---- sub_499170  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499170() { return 0; }
#if 0
int **__cdecl sub_499170(void *this, int a2, int **a3, int *a4)
{
  int *v4;
  int *v5;
  int v6;
  int *v7;
  int *v8;
  int v9;
  int v10;
  int *v11;
  int v12;
  int **v13;
  char v14;
  _BYTE *v15;
  bool v16; // zf
  _BYTE *v17;
  int v18;
  int **result;
  int *Block;
  void **v21; // [esp+Ch] [ebp-50h] BYREF
  char v22;
  int v23;
  int v24;
  _DWORD pExceptionObject[13]; // [esp+28h] [ebp-34h] BYREF

  v4 = a4;
  if ( *((_BYTE *)a4 + 45) )
  {
    v24 = 15;
    v23 = 0;
    v22 = 0;
    sub_498410((int)&v21, "invalid map/set<T> iterator", (void **)strlen("invalid map/set<T> iterator"));
    pExceptionObject[12] = 0;
    sub_492500((int)pExceptionObject, &v21);
    pExceptionObject[0] = &std::out_of_range::`vftable';
    CxxThrowException_x_x_(pExceptionObject, (_ThrowInfo *)&_TI3_AVout_of_range_std__);
  }
  v5 = a4;
  Block = a4;
  sub_498230((int)this, &a4);
  v6 = *v4;
  v7 = v4 + 2;
  if ( !*(_BYTE *)(*v4 + 45) )
  {
    if ( *(_BYTE *)(*v7 + 45) )
      goto LABEL_7;
    v4 = a4;
    v7 = a4 + 2;
  }
  v6 = *v7;
LABEL_7:
  if ( v4 == v5 )
  {
    v8 = (int *)v5[1];
    if ( !*(_BYTE *)(v6 + 45) )
      *(_DWORD *)(v6 + 4) = v8;
    v9 = a2;
    v10 = *(_DWORD *)(a2 + 4);
    if ( *(int **)(v10 + 4) == v5 )
    {
      *(_DWORD *)(v10 + 4) = v6;
    }
    else if ( (int *)*v8 == v5 )
    {
      *v8 = v6;
    }
    else
    {
      v8[2] = v6;
    }
    if ( **(int ***)(v9 + 4) == v5 )
    {
      if ( *(_BYTE *)(v6 + 45) )
        v11 = v8;
      else
        v11 = sub_498CD0((_DWORD *)v6);
      **(_DWORD **)(v9 + 4) = v11;
    }
    if ( *(int **)(*(_DWORD *)(v9 + 4) + 8) == v5 )
    {
      if ( *(_BYTE *)(v6 + 45) )
        *(_DWORD *)(*(_DWORD *)(v9 + 4) + 8) = v8;
      else
        *(_DWORD *)(*(_DWORD *)(v9 + 4) + 8) = sub_499730(v6);
    }
  }
  else
  {
    *(_DWORD *)(*v5 + 4) = v4;
    *v4 = *v5;
    if ( v4 == (int *)v5[2] )
    {
      v8 = v4;
    }
    else
    {
      v8 = (int *)v4[1];
      if ( !*(_BYTE *)(v6 + 45) )
        *(_DWORD *)(v6 + 4) = v8;
      *v8 = v6;
      *v7 = v5[2];
      *(_DWORD *)(v5[2] + 4) = v4;
    }
    v12 = *(_DWORD *)(a2 + 4);
    if ( *(int **)(v12 + 4) == v5 )
    {
      *(_DWORD *)(v12 + 4) = v4;
    }
    else
    {
      v13 = (int **)v5[1];
      if ( *v13 == v5 )
        *v13 = v4;
      else
        v13[2] = v4;
    }
    v4[1] = v5[1];
    v14 = *((_BYTE *)v4 + 44);
    *((_BYTE *)v4 + 44) = *((_BYTE *)v5 + 44);
    v9 = a2;
    *((_BYTE *)v5 + 44) = v14;
  }
  if ( *((_BYTE *)Block + 44) == 1 )
  {
    if ( v6 != *(_DWORD *)(*(_DWORD *)(v9 + 4) + 4) )
    {
      do
      {
        if ( *(_BYTE *)(v6 + 44) != 1 )
          break;
        v15 = (_BYTE *)*v8;
        if ( v6 == *v8 )
        {
          v15 = (_BYTE *)v8[2];
          if ( !v15[44] )
          {
            v15[44] = 1;
            *((_BYTE *)v8 + 44) = 0;
            sub_499480(v8, v9);
            v15 = (_BYTE *)v8[2];
          }
          if ( v15[45] )
            goto LABEL_53;
          if ( *(_BYTE *)(*(_DWORD *)v15 + 44) != 1 || *(_BYTE *)(*((_DWORD *)v15 + 2) + 44) != 1 )
          {
            if ( *(_BYTE *)(*((_DWORD *)v15 + 2) + 44) == 1 )
            {
              *(_BYTE *)(*(_DWORD *)v15 + 44) = 1;
              v15[44] = 0;
              sub_4994F0(v15, v9);
              v15 = (_BYTE *)v8[2];
            }
            v15[44] = *((_BYTE *)v8 + 44);
            *((_BYTE *)v8 + 44) = 1;
            *(_BYTE *)(*((_DWORD *)v15 + 2) + 44) = 1;
            sub_499480(v8, v9);
            break;
          }
        }
        else
        {
          if ( !v15[44] )
          {
            v15[44] = 1;
            *((_BYTE *)v8 + 44) = 0;
            sub_4994F0(v8, v9);
            v15 = (_BYTE *)*v8;
          }
          if ( v15[45] )
            goto LABEL_53;
          if ( *(_BYTE *)(*((_DWORD *)v15 + 2) + 44) != 1 || *(_BYTE *)(*(_DWORD *)v15 + 44) != 1 )
          {
            if ( *(_BYTE *)(*(_DWORD *)v15 + 44) == 1 )
            {
              *(_BYTE *)(*((_DWORD *)v15 + 2) + 44) = 1;
              v15[44] = 0;
              sub_499480(v15, v9);
              v15 = (_BYTE *)*v8;
            }
            v15[44] = *((_BYTE *)v8 + 44);
            *((_BYTE *)v8 + 44) = 1;
            *(_BYTE *)(*(_DWORD *)v15 + 44) = 1;
            sub_4994F0(v8, v9);
            break;
          }
        }
        v15[44] = 0;
LABEL_53:
        v6 = (int)v8;
        v16 = v8 == *(int **)(*(_DWORD *)(v9 + 4) + 4);
        v8 = (int *)v8[1];
      }
      while ( !v16 );
    }
    *(_BYTE *)(v6 + 44) = 1;
  }
  if ( (unsigned int)Block[9] >= 0x10 )
    j__free((void *)Block[4]);
  Block[9] = 15;
  LOBYTE(a2) = 0;
  Block[8] = 0;
  v17 = unknown_libname_4(Block + 3);
  sub_492450(v17, &a2);
  j__free(Block);
  v18 = *(_DWORD *)(v9 + 8);
  if ( v18 )
    *(_DWORD *)(v9 + 8) = v18 - 1;
  result = a3;
  *a3 = a4;
  return result;
}
#endif

/* ---- sub_499480  0x00499480 ----  VERIFIED */
int __cdecl sub_499480( int node_, int tree )
{
  unsigned char *node = (unsigned char *)node_;
  unsigned char *r    = FXN_RIGHT( node );
  unsigned char *head;

  FXN_RIGHT( node ) = FXN_LEFT( r );
  if ( !FXN_ISNIL( FXN_LEFT( r ) ) )
    FXN_PARENT( FXN_LEFT( r ) ) = node;
  FXN_PARENT( r ) = FXN_PARENT( node );

  head = FXT_HEAD( tree );
  if ( node == FXN_PARENT( head ) )
    FXN_PARENT( head ) = r;
  else if ( node == FXN_LEFT( FXN_PARENT( node ) ) )
    FXN_LEFT( FXN_PARENT( node ) ) = r;
  else
    FXN_RIGHT( FXN_PARENT( node ) ) = r;

  FXN_LEFT( r )      = node;
  FXN_PARENT( node ) = r;
  return (int)r;
}

/* ---- sub_4994E0  0x004994E0 ----  [HIGH] */
int __cdecl sub_4994E0(int a1)
{
  return *(_DWORD *)(a1 + 4) + 4;
}

/* ---- sub_4994F0  0x004994F0 ----  VERIFIED */
int __cdecl sub_4994F0(_DWORD *this, int a2)
{
  int result;
  int v3;
  int v4;
  _DWORD *v5;

  result = *this;
  *this = *(_DWORD *)(*this + 8);
  v3 = *(_DWORD *)(result + 8);
  if ( !*(_BYTE *)(v3 + 45) )
    *(_DWORD *)(v3 + 4) = this;
  *(_DWORD *)(result + 4) = this[1];
  v4 = *(_DWORD *)(a2 + 4);
  if ( this == *(_DWORD **)(v4 + 4) )
  {
    *(_DWORD *)(v4 + 4) = result;
    *(_DWORD *)(result + 8) = this;
    this[1] = result;
  }
  else
  {
    v5 = (_DWORD *)this[1];
    if ( this == (_DWORD *)v5[2] )
      v5[2] = result;
    else
      *v5 = result;
    *(_DWORD *)(result + 8) = this;
    this[1] = result;
  }
  return result;
}

/* ---- sub_499550  0x00499550 ----  VERIFIED */
_DWORD *__stdcall sub_499550( int tree )
{
  _DWORD *p;

  (void)tree;
  p = (_DWORD *)malloc( 0x30 );
  if ( p )
    p[0] = 0;
  if ( p != (_DWORD *)-4 )
    p[1] = 0;
  if ( p != (_DWORD *)-8 )
    p[2] = 0;
  *( (char *)p + 44 ) = 1;
  *( (char *)p + 45 ) = 0;
  return p;
}

/* ---- sub_499590  0x00499590 ----  VERIFIED */
int __cdecl sub_499590( int tree, int left, int parent, int right,
                        int val, char colour )
{
  unsigned char *node;

  (void)tree;

  node = (unsigned char *)malloc( 0x30 );
  if ( node )
  {
    FXN_LEFT  ( node ) = (unsigned char *)left;
    FXN_PARENT( node ) = (unsigned char *)parent;
    FXN_RIGHT ( node ) = (unsigned char *)right;

    FXS_RES ( FXN_KEY( node ) ) = 15;
    FXS_SIZE( FXN_KEY( node ) ) = 0;
    *FXS_BX ( FXN_KEY( node ) ) = 0;
    sub_497C10( (int)FXN_KEY( node ), val, 0, 0xFFFFFFFF );

    FXN_MAPPED( node ) = FXP_MAPPED( val );
    FXN_COLOR ( node ) = colour;
    FXN_ISNIL ( node ) = 0;
  }
  return (int)node;
}

/* ---- sub_499640  0x00499640 ----  [HIGH] */
_BYTE *__cdecl sub_499640(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- sub_499650  0x00499650 ----  VERIFIED */
int *__fastcall sub_499650( int unused, int **pptr )
{
  int **p;
  int  *r;
  int  *q;

  (void)unused;
  p = (int **)pptr[0];
  r = (int *)p;
  if ( *( (char *)p + 45 ) )
  {
    r     = (int *)p[2];
    *pptr = r;
  }
  else
  {
    q = (int *)p[0];
    if ( *( (char *)q + 45 ) )
    {
      r = (int *)p[1];
      if ( !*( (char *)r + 45 ) )
      {
        do
        {
          if ( *pptr != (int *)r[0] )
            break;
          *pptr = r;
          r     = (int *)r[1];
        }
        while ( !*( (char *)r + 45 ) );
        if ( !*( (char *)r + 45 ) )
          *pptr = r;
      }
    }
    else
    {
      for ( r = (int *)q[2]; !*( (char *)r + 45 ); r = (int *)r[2] )
        q = r;
      *pptr = q;
    }
  }
  return r;
}

/* ---- sub_4996B0  0x004996B0 ----  VERIFIED */
int **__cdecl sub_4996B0(int **a1, int a2, int **a3, int a4)
{
  int *v4;

  v4 = *a1;
  sub_498230(a2, a1);
  *a3 = v4;
  return a3;
}

/* ---- sub_4996C0  0x004996C0 ----  [HIGH] */
int __stdcall sub_4996C0(int a1)
{
  return 0x3FFFFFFF;
}

/* ---- nullsub_52  0x004996D0 ----  [HIGH] */
void __stdcall nullsub_52(int a1)
{
  ;
}

/* ---- sub_4996E0  0x004996E0 ----  VERIFIED */
_DWORD *__cdecl sub_4996E0(_DWORD *result, int a2, int a3, _DWORD *a4)
{
  *result = a3;
  result[1] = a2;
  result[2] = *a4;
  return result;
}

/* ---- sub_499700  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
void *__stdcall sub_499700(size_t Size, int a2)
{ return 0; }
#if 0
// ?do_narrow@?$ctype@G@std@@MBEDGD@Z
// doubtful name
void *__stdcall sub_499700(size_t Size, int a2)
{
  return operator new(Size);
}
#endif

/* ---- sub_499720  0x00499720 ----  [HIGH] */
int __stdcall sub_499720(int a1)
{
  return 0x3FFFFFFF;
}

/* ---- sub_499730  0x00499730 ----  VERIFIED */
int __cdecl sub_499730(int result)
{
  int i;

  for ( i = *(_DWORD *)(result + 8); !*(_BYTE *)(i + 45); i = *(_DWORD *)(i + 8) )
    result = i;
  return result;
}

/* ---- sub_499750  0x00499750 ----  [HIGH] */
_BYTE *__cdecl sub_499750(_BYTE *result, _BYTE *a2, int a3)
{
  *result = *a2;
  return result;
}

/* ---- sub_499760  0x00499760 ----  [HIGH] */
int __stdcall sub_499760(int a1)
{
  return 0x7FFFFFF;
}

/* ---- sub_499770  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499770() { return 0; }
#if 0
void *__cdecl sub_499770(int a1, int a2, int a3)
{
  return operator new(48 * a1);
}
#endif

/* ---- sub_499790  0x00499790 ----  VERIFIED */
int __cdecl sub_499790(int a1, int a2, int a3, int a4, int a5, char a6)
{
  *(_DWORD *)(a4 + 4) = a3;
  *(_DWORD *)a4 = a1;
  *(_DWORD *)(a4 + 8) = a2;
  *(_DWORD *)(a4 + 36) = 15;
  *(_DWORD *)(a4 + 32) = 0;
  *(_BYTE *)(a4 + 16) = 0;
  sub_497C10((void ****)(a4 + 12), (void ****)a5, 0, (void **)0xFFFFFFFF);
  *(_DWORD *)(a4 + 40) = *(_DWORD *)(a5 + 28);
  *(_BYTE *)(a4 + 44) = a6;
  *(_BYTE *)(a4 + 45) = 0;
  return a4;
}

/* ---- sub_4997E0  0x004997E0 ----  VERIFIED */
_DWORD *__cdecl sub_4997E0(_DWORD *result, _DWORD *a2, int a3)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- sub_4997F0  0x004997F0 ----  [HIGH] */
int __stdcall sub_4997F0(int a1)
{
  return 0x3FFFFFFF;
}

/* ---- sub_499800  0x00499800 ----  VERIFIED */
int __cdecl sub_499800(int a1, int a2)
{
  *(_DWORD *)(a2 + 24) = 15;
  *(_DWORD *)(a2 + 20) = 0;
  *(_BYTE *)(a2 + 4) = 0;
  sub_497C10((void ****)a2, (void ****)a1, 0, (void **)0xFFFFFFFF);
  *(_DWORD *)(a2 + 28) = *(_DWORD *)(a1 + 28);
  return a2;
}

/* ---- sub_499830  0x00499830 ----  [HIGH] */
_BYTE *__cdecl sub_499830(_BYTE *result, char a2)
{
  *result = a2;
  return result;
}

/* ---- sub_499840  0x00499840 ----  VERIFIED */
_DWORD *__cdecl sub_499840(_DWORD *a1, _DWORD *a2, int a3)
{
  return sub_499B40(a2, a1, a3);
}

/* ---- sub_499860  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499860() { return 0; }
#if 0
bool __cdecl sub_499860(_DWORD *a1, std::_String_base *a2)
{
  unsigned int v2;
  const char *v3;

  v2 = a1[5];
  if ( a1[6] < 0x10u )
    v3 = (const char *)(a1 + 1);
  else
    v3 = (const char *)a1[1];
  return sub_499AB0(*((_DWORD *)a2 + 5), a2, 0, v3, v2) < 0;
}
#endif

/* ---- nullsub_53  0x00499890 ----  [HIGH] */
void nullsub_53()
{
  ;
}

/* ---- nullsub_54  0x004998A0 ----  [HIGH] */
void nullsub_54()
{
  ;
}

/* ---- sub_4998B0  0x004998B0 ----  VERIFIED */
_DWORD *__stdcall sub_4998B0( _DWORD *first, _DWORD *last, _DWORD *dest )
{
  return sub_499C20( dest, first, last );
}

/* ---- sub_4998E0  0x004998E0 ----  VERIFIED */
_DWORD *__cdecl sub_4998E0( _DWORD *pval, _DWORD *dest, _DWORD *end )
{
  while ( dest != end )
  {
    *dest = *pval;
    ++dest;
  }
  return dest;
}

/* ---- sub_499900  0x00499900 ----  VERIFIED */
_DWORD *__cdecl sub_499900( _DWORD *out, int first, int last, _DWORD *destLast )
{
  _DWORD *src = (_DWORD *)last;
  _DWORD *dst = destLast;

  while ( src != (_DWORD *)first )
    *--dst = *--src;
  *out = (_DWORD)dst;
  return dst;
}

/* ---- sub_499930  0x00499930 ----  VERIFIED */
int __cdecl sub_499930(int a1)
{
  return sub_499A30(a1);
}

/* ---- nullsub_55  0x00499940 ----  [HIGH] */
void nullsub_55()
{
  ;
}

/* ---- nullsub_56  0x00499950 ----  [HIGH] */
void __stdcall nullsub_56(int a1)
{
  ;
}

/* ---- sub_499960  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499960() { return 0; }
#if 0
void *__cdecl sub_499960(int a1)
{
  return operator new(12 * a1);
}
#endif

/* ---- sub_499970  0x00499970 ----  VERIFIED */
_DWORD *__cdecl sub_499970(_DWORD *result, _DWORD *a2)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- nullsub_57  0x00499980 ----  [HIGH] */
void nullsub_57()
{
  ;
}

/* ---- sub_499990  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499990() { return 0; }
#if 0
void *__cdecl sub_499990(int a1)
{
  return operator new(4 * a1);
}
#endif

/* ---- sub_4999A0  0x004999A0 ----  VERIFIED */
char *__cdecl sub_4999A0(char *result, char *a2)
{
  char v2;

  v2 = *a2;
  *a2 = *result;
  *result = v2;
  return result;
}

/* ---- nullsub_58  0x004999B0 ----  [HIGH] */
void __stdcall nullsub_58(int a1)
{
  ;
}

/* ---- nullsub_59  0x004999C0 ----  [HIGH] */
void __stdcall nullsub_59(int a1)
{
  ;
}

/* ---- sub_4999D0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4999D0() { return 0; }
#if 0
void *__cdecl sub_4999D0(size_t Size)
{
  return operator new(Size);
}
#endif

/* ---- nullsub_60  0x004999E0 ----  [HIGH] */
void __stdcall nullsub_60(int a1)
{
  ;
}

/* ---- sub_4999F0  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_4999F0() { return 0; }
#if 0
void *__cdecl sub_4999F0(int a1)
{
  return operator new(48 * a1);
}
#endif

/* ---- sub_499A00  0x00499A00 ----  VERIFIED */
_DWORD *__cdecl sub_499A00(_DWORD *result, _DWORD *a2)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- sub_499A10  0x00499A10 ----  VERIFIED */
void *__cdecl sub_499A10(void *a1, char a2)
{
  sub_499A30((int)a1);
  if ( (a2 & 1) != 0 )
    j__free(a1);
  return a1;
}

/* ---- sub_499A30  0x00499A30 ----  VERIFIED */
int __cdecl sub_499A30(int a1)
{
  int result;

  if ( *(_DWORD *)(a1 + 36) >= 0x10u )
    j__free(*(void **)(a1 + 16));
  result = 0;
  *(_DWORD *)(a1 + 36) = 15;
  *(_DWORD *)(a1 + 32) = 0;
  *(_BYTE *)(a1 + 16) = 0;
  return result;
}

/* ---- sub_499A60  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_499A60() { return 0; }
#if 0
int __cdecl sub_499A60(int a1, std::_String_base *a2)
{
  if ( *(_DWORD *)(a1 + 24) < 0x10u )
    return sub_499AB0(*((_DWORD *)a2 + 5), a2, 0, (const char *)(a1 + 4), *(_DWORD *)(a1 + 20));
  else
    return sub_499AB0(*((_DWORD *)a2 + 5), a2, 0, *(const char **)(a1 + 4), *(_DWORD *)(a1 + 20));
}
#endif

/* ---- sub_499A90  0x00499A90 ----  [HIGH] */
_DWORD *__cdecl sub_499A90(_DWORD *result)
{
  *result += 4;
  return result;
}

/* ---- sub_499AA0  0x00499AA0 ----  [HIGH] */
BOOL __cdecl sub_499AA0(_DWORD *a1, _DWORD *a2)
{
  return *a1 != *a2;
}

/* ---- sub_499AB0  0x00499AB0 ----  VERIFIED */
int __cdecl sub_499AB0( unsigned int n0, int self, unsigned int off,
                        const char *ptr, unsigned int count )
{
  unsigned int mysize, n;
  int          ans;

  mysize = FXS_SIZE( self );
  /* retail 0x00499ABB: if ( _Mysize < off ) _Xran(); */
  if ( mysize - off < n0 )
    n0 = mysize - off;

  if ( n0 )
  {
    n   = n0 < count ? n0 : count;
    ans = memcmp( FXS_PTR( self ) + off, ptr, n );
    if ( ans )
      return ans < 0 ? -1 : 1;
  }

  if ( n0 < count )
    return -1;
  return n0 != count;
}

/* ---- sub_499B20  0x00499B20 ----  [HIGH] */
BOOL __cdecl sub_499B20(_DWORD *a1, _DWORD *a2)
{
  return *a2 == *a1;
}

/* ---- sub_499B30  0x00499B30 ----  VERIFIED */
char __fastcall sub_499B30(int a1)
{
  return HIBYTE(a1);
}

/* ---- sub_499B40  0x00499B40 ----  VERIFIED */
_DWORD *__cdecl sub_499B40( _DWORD *dest, _DWORD *pval, int count )
{
  while ( count > 0 )
  {
    if ( dest )
      *dest = *pval;
    ++dest;
    --count;
  }
  return dest;
}

/* ---- nullsub_61  0x00499B60 ----  [HIGH] */
void nullsub_61()
{
  ;
}

/* ---- sub_499B70  0x00499B70 ----  VERIFIED */
_DWORD *__cdecl sub_499B70(_DWORD *a1, _DWORD *a2, _DWORD *a3)
{
  return sub_499C20(a3, a1, a2);
}

/* ---- sub_499BA0  0x00499BA0 ----  VERIFIED */
char __fastcall sub_499BA0(int a1)
{
  return HIBYTE(a1);
}

/* ---- sub_499BB0  0x00499BB0 ----  VERIFIED */
_DWORD *__cdecl sub_499BB0(_DWORD *result, int a2, int a3, _DWORD *a4)
{
  int v4;
  _DWORD *v5;
  int v6;

  v4 = a3;
  if ( a2 == a3 )
  {
    *result = a4;
  }
  else
  {
    v5 = a4;
    do
    {
      v6 = *(_DWORD *)(v4 - 4);
      v4 -= 4;
      *--v5 = v6;
    }
    while ( v4 != a2 );
    *result = v5;
  }
  return result;
}

/* ---- sub_499BE0  0x00499BE0 ----  VERIFIED */
_DWORD *__cdecl sub_499BE0(_DWORD *result, _DWORD *a2, int a3)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- nullsub_62  0x00499BF0 ----  [HIGH] */
void __stdcall nullsub_62(int a1, int a2)
{
  ;
}

/* ---- sub_499C00  0x00499C00 ----  [HIGH] */
_DWORD *__cdecl sub_499C00(_DWORD *result)
{
  *result -= 4;
  return result;
}

/* ---- sub_499C10  0x00499C10 ----  VERIFIED */
char __fastcall sub_499C10(int a1)
{
  return HIBYTE(a1);
}

/* ---- sub_499C20  0x00499C20 ----  VERIFIED */
_DWORD *__cdecl sub_499C20( _DWORD *dest, _DWORD *first, _DWORD *last )
{
  while ( first != last )
  {
    if ( dest )
      *dest = *first;
    ++first;
    ++dest;
  }
  return dest;
}

/* ---- sub_499C50  0x00499C50 ----  VERIFIED */
_DWORD *__cdecl sub_499C50(_DWORD *result, _DWORD *a2)
{
  if ( result )
    *result = *a2;
  return result;
}

/* ---- nullsub_63  0x00499C60 ----  [HIGH] */
void nullsub_63()
{
  ;
}

/* ---- sub_499C70  0x00499C70 ----  VERIFIED */
_DWORD *__cdecl sub_499C70(_DWORD *result)
{
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;
  result[4] = 0;
  return result;
}

/* ---- sub_499C80  0x00499C80 ----  VERIFIED */
_DWORD *__cdecl sub_499C80(_DWORD *result)
{
  *result = 0;
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;
  result[4] = 0;
  return result;
}

/* ---- SFxHelper__Print  0x00499CA0 ----  VERIFIED */
void SFxHelper__Print( int unusedThis, char *Format, ... )
{
  char    Buffer[1024];
  va_list ArgList;

  (void)unusedThis;
  va_start( ArgList, Format );
  vsprintf( Buffer, Format, ArgList );
  Com_Printf( Buffer );
}

/* ---- SFxHelper__AdjustTime  0x00499D00 ----  VERIFIED */
int *__cdecl SFxHelper__AdjustTime( int *this, int realTime )
{
  if ( fx_freeze->integer )
  {
    this[3] = 0;
    this[0] = realTime;
    return this;
  }

  if ( this[0] )
  {
    this[3] = realTime - this[0];
    if ( this[3] >= 0 )
    {
      if ( this[3] > 200 )
        this[3] = 200;
      this[2] = this[1];
      this[1] = this[3] + this[1];
      this[0] = realTime;
      return this;
    }
  }

  this[3] = 0;
  this[2] = this[1];
  this[1] = this[3] + this[1];
  this[0] = realTime;
  return this;
}

/* ---- SFxHelper__AdjustCamera  0x00499D60 ----  [CONFIRMED] */
void __cdecl SFxHelper__AdjustCamera(int a1, int a2, float a3)
{
  double v3;
  double v4;
  double v5;
  long double v6;
  float *v7;
  float *v8;
  int v9;
  int v10;
  double v11;
  float v12; // [esp+0h] [ebp-14h] BYREF
  float v13; // [esp+4h] [ebp-10h] BYREF
  float v14;
  float *v15;
  float *v16;

  *(_DWORD *)(a1 + 20) = *(_DWORD *)(a2 + 24);
  *(_DWORD *)(a1 + 24) = *(_DWORD *)(a2 + 28);
  *(_DWORD *)(a1 + 28) = *(_DWORD *)(a2 + 32);
  *(_DWORD *)(a1 + 32) = *(_DWORD *)(a2 + 36);
  *(_DWORD *)(a1 + 36) = *(_DWORD *)(a2 + 40);
  *(_DWORD *)(a1 + 40) = *(_DWORD *)(a2 + 44);
  v3 = *(float *)(a2 + 16) * 0.0087266462;
  v16 = &v13;
  v14 = v3;
  v15 = &v12;
  v13 = cos(v14);
  v12 = sin(v14);
  *(float *)(a1 + 48) = v12 * *(float *)(a2 + 36);
  *(float *)(a1 + 52) = v12 * *(float *)(a2 + 40);
  *(float *)(a1 + 56) = v12 * *(float *)(a2 + 44);
  *(float *)(a1 + 48) = v13 * *(float *)(a2 + 48) + *(float *)(a1 + 48);
  *(float *)(a1 + 52) = v13 * *(float *)(a2 + 52) + *(float *)(a1 + 52);
  *(float *)(a1 + 56) = v13 * *(float *)(a2 + 56) + *(float *)(a1 + 56);
  *(float *)(a1 + 64) = v12 * *(float *)(a2 + 36);
  *(float *)(a1 + 68) = v12 * *(float *)(a2 + 40);
  *(float *)(a1 + 72) = v12 * *(float *)(a2 + 44);
  *(float *)(a1 + 64) = -v13 * *(float *)(a2 + 48) + *(float *)(a1 + 64);
  v4 = -v13 * *(float *)(a2 + 52);
  v15 = &v13;
  *(float *)(a1 + 68) = v4 + *(float *)(a1 + 68);
  v5 = -v13 * *(float *)(a2 + 56);
  v16 = &v12;
  *(float *)(a1 + 72) = v5 + *(float *)(a1 + 72);
  v14 = *(float *)(a2 + 20) * 0.0087266462;
  v6 = sin(v14);
  v7 = v16;
  *v15 = cos(v14);
  *v7 = v6;
  *(float *)(a1 + 80) = v12 * *(float *)(a2 + 36);
  *(float *)(a1 + 84) = v12 * *(float *)(a2 + 40);
  *(float *)(a1 + 88) = v12 * *(float *)(a2 + 44);
  *(float *)(a1 + 80) = v13 * *(float *)(a2 + 60) + *(float *)(a1 + 80);
  *(float *)(a1 + 84) = v13 * *(float *)(a2 + 64) + *(float *)(a1 + 84);
  *(float *)(a1 + 88) = v13 * *(float *)(a2 + 68) + *(float *)(a1 + 88);
  *(float *)(a1 + 96) = v12 * *(float *)(a2 + 36);
  *(float *)(a1 + 100) = v12 * *(float *)(a2 + 40);
  *(float *)(a1 + 104) = v12 * *(float *)(a2 + 44);
  *(float *)(a1 + 96) = -v13 * *(float *)(a2 + 60) + *(float *)(a1 + 96);
  *(float *)(a1 + 100) = -v13 * *(float *)(a2 + 64) + *(float *)(a1 + 100);
  *(float *)(a1 + 104) = -v13 * *(float *)(a2 + 68) + *(float *)(a1 + 104);
  *(_DWORD *)(a1 + 128) = 5;
  if ( a3 > 0.0 )
  {
    *(float *)(a1 + 112) = -*(float *)(a2 + 36);
    *(float *)(a1 + 116) = -*(float *)(a2 + 40);
    *(float *)(a1 + 120) = -*(float *)(a2 + 44);
    *(_DWORD *)(a1 + 128) = 6;
  }
  v8 = (float *)a1;
  v9 = 0;
  if ( *(int *)(a1 + 128) > 0 )
  {
    v10 = 0;
    do
    {
      ++v9;
      v11 = v8[v10 + 8] * v8[5];
      v10 += 4;
      v8[v10 + 7] = v11 + v8[v10 + 6] * v8[7] + v8[v10 + 5] * v8[6];
      v8 = (float *)a1;
    }
    while ( v9 < *(_DWORD *)(a1 + 128) );
  }
  if ( a3 > 0.0 )
    v8[31] = v8[31] - a3;
}

/* ---- SFxHelper__CullSphere  0x0049A0D0 ----  VERIFIED */
char __cdecl SFxHelper__CullSphere( float *origin, int unusedThis, float radius )
{
  int    i;
  float *plane;

  (void)unusedThis;
  plane = &flt_140752C - 1;
  for ( i = 0; i < dword_1407588; ++i )
  {
    if ( plane[0] * origin[0] + plane[1] * origin[1] + plane[2] * origin[2]
         + radius < plane[3] )
      return 1;
    plane += 4;
  }
  return 0;
}

/* ---- SFxHelper__CullCylinder  0x0049A120 ----  VERIFIED */
char __cdecl SFxHelper__CullCylinder( float *p1, float *p2, int unusedThis,
                                      float radius1, float radius2 )
{
  int    i;
  float *plane;

  (void)unusedThis;
  plane = &flt_140752C - 1;
  for ( i = 0; i < dword_1407588; ++i )
  {
    if ( plane[0] * p1[0] + plane[1] * p1[1] + plane[2] * p1[2]
         + radius1 < plane[3] )
    {
      if ( plane[0] * p2[0] + plane[1] * p2[1] + plane[2] * p2[2]
           + radius2 < plane[3] )
        return 1;
    }
    plane += 4;
  }
  return 0;
}

/* ---- SFxHelper__OpenFile  0x0049A190 ----  [HIGH] */
int __cdecl SFxHelper__OpenFile(char *qpath, fileHandle_t *a2, int a3, int a4)
{
  return FS_FOpenFileByMode(qpath, a2, FS_READ);
}

/* ---- SFxHelper__ReadFile  0x0049A1A0 ----  [HIGH] */
int __cdecl SFxHelper__ReadFile(int ElementCount, void *Buffer, fileHandle_t a3, int a4)
{
  FS_Read(Buffer, ElementCount, a3);
  return 1;
}

/* ---- SFxHelper__CloseFile  0x0049A1C0 ----  [HIGH] */
void __cdecl SFxHelper__CloseFile(fileHandle_t a1, int a2)
{
  FS_FCloseFile(a1);
}

/* ---- SFxHelper__PlaySound  0x0049A1D0 ----  [HIGH] */
int __cdecl SFxHelper__PlaySound(int result, int a2, float *a3, int a4)
{
  if ( result > 0 && result <= snd_aliasTableCount[SND_LOCALE_INGAME] )
  {
    result = (int)snd_aliasTable[SND_LOCALE_INGAME] + 68 * result - 68;
    if ( result )
    {
      result = (int)Com_PickSoundAlias(*(const char **)result, 1);
      if ( result )
#ifdef DEDICATED
        return 0;
#else
        return MSS_PlaySoundAlias_Internal((float *)result, a3, result, 0, a4, 0, 0);
#endif
    }
  }
  return result;
}

/* ---- SFxHelper__Trace  0x0049A220 ----  VERIFIED */
void __cdecl SFxHelper__Trace( int brushmask, const float *end, const float *start,
                               trace_t *results, int unusedThis,
                               const float *mins, const float *maxs, int unused4 )
{
  (void)unusedThis;
  (void)unused4;

  results->fraction = 1.0f;
  CM_Trace( results, start, end, mins, maxs, 0, vec3_origin, brushmask, 0, 0 );
  results->entityNum = ( results->fraction == 1.0 ) ? 1023 : 1022;
}

/* ---- SFxHelper__AddFxToScene  0x0049A270 ----  [HIGH] */
int __cdecl SFxHelper__AddFxToScene(int a1, int a2)
{
  return dword_14328A0(a1, 0);
}

/* ---- SFxHelper__RegisterShader  0x0049A280 ----  [HIGH] */
int __cdecl SFxHelper__RegisterShader(int a1, int a2)
{
  return re_RegisterShader(a1, 9);
}

/* ---- SFxHelper__RegisterSound  0x0049A290 ----  [HIGH] */
int __stdcall SFxHelper__RegisterSound(int a1)
{
  int result;

  result = ((_BYTE *)Com_PickSoundAlias((const char *)a1, 1) - (_BYTE *)snd_aliasTable[SND_LOCALE_INGAME] + 68) / 68;
  if ( result <= 0 || result > snd_aliasTableCount[SND_LOCALE_INGAME] )
    return 0;
  return result;
}

/* ---- SFxHelper__RegisterModel  0x0049A2D0 ----  [HIGH] */
char *__cdecl SFxHelper__RegisterModel(char *a1, int a2)
{
  return CFxModel__Register(a1);
}

/* ---- SFxHelper__SetIgnorePrecacheErrors  0x0049A2E0 ----  [HIGH] */
int __cdecl SFxHelper__SetIgnorePrecacheErrors(unsigned __int8 a1, int a2)
{
  return dword_1432880(a1);
}

/* ---- SFxHelper__GetShaderName  0x0049A2F0 ----  [HIGH] */
int __cdecl SFxHelper__GetShaderName(int a1, int a2)
{
  return dword_1432890(a1);
}

/* ---- SFxHelper__AddLightToScene  0x0049A300 ----  [HIGH] */
char __cdecl SFxHelper__AddLightToScene(int a1, int a2)
{
  char result;

  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a1, (_BYTE *)(a2 + 4));
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)(a2 + 4), 4);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a1, (_BYTE *)(a2 + 8));
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)(a2 + 8), 4);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a1, (_BYTE *)(a2 + 12));
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)(a2 + 12), 4);
  if ( *(_BYTE *)(a1 + 12) )
    CFxArchive__ReadData(4, (_DWORD *)a1, (_BYTE *)(a2 + 16));
  else
    CFxArchive__WriteData((int *)a1, (_BYTE *)(a2 + 16), 4);
  result = *(_BYTE *)(a1 + 12);
  if ( result )
    *(_DWORD *)a2 = 0;
  return result;
}

/* ---- SFxHelper__CameraShake  0x0049A390 ----  [HIGH] */
int __stdcall SFxHelper__CameraShake(int a1, int a2, int a3, int a4, int a5, int a6)
{
  return dword_14328AC(a2, a3, a4, a5, a6);
}

/* ---- sub_49A3C0  0x0049A3C0 ----  VERIFIED */
int __cdecl sub_49A3C0(int a1, int a2, int a3, int a4)
{
  return dword_14328A4(a2, a3, a1);
}

/* ---- nullsub_64  0x0049A3D0 ----  [HIGH] */
void __stdcall nullsub_64(int a1, int a2, int a3, int a4, int a5)
{
  ;
}

/* ---- sub_49A3E0  0x0049A3E0 ----  [HIGH] */
int __cdecl sub_49A3E0(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

/* ---- sub_49A3F0  0x0049A3F0 ----  [HIGH] */
int __cdecl sub_49A3F0(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

/* ---- sub_49A400  0x0049A400 ----  [HIGH] */
int __cdecl sub_49A400(int a1)
{
  return *(_DWORD *)(a1 + 16);
}

/* ---- sub_49A410  0x0049A410 ----  [HIGH] */
int __cdecl sub_49A410(int a1)
{
  return *(_DWORD *)(a1 + 16);
}

/* ---- sub_49A420  0x0049A420 ----  VERIFIED */
_DWORD *__cdecl sub_49A420(_DWORD *a1, int a2)
{
  return sub_497590(&a2, a1);
}

/* ---- sub_49A430  0x0049A430 ----  [HIGH] */
_DWORD *__cdecl sub_49A430(_DWORD *result)
{
  *result = 0;
  result[1] = 0;
  return result;
}

/* ---- sub_49A440  0x0049A440 ----  [HIGH] */
_DWORD *__cdecl sub_49A440(_DWORD *result, int a2, int a3)
{
  *result = a2;
  result[1] = a3;
  return result;
}

/* ---- sub_49A450  0x0049A450 ----  VERIFIED */
int __cdecl sub_49A450(_DWORD *a1, _DWORD *a2)
{
  int result;

  *a2 = *a1;
  result = a1[1];
  a2[1] = result;
  return result;
}

/* ---- CPrimitiveTemplate__ctor  0x0049A460 ----  [CONFIRMED] */
int __stdcall CPrimitiveTemplate__ctor( int self )
{
  *(int *)( self + 76  ) = 0;
  *(int *)( self + 80  ) = 0;
  *(int *)( self + 84  ) = 0;
  *(int *)( self + 88  ) = 0;
  *(int *)( self + 92  ) = 0;
  *(int *)( self + 96  ) = 0;
  *(int *)( self + 108 ) = 0;
  *(int *)( self + 112 ) = 0;
  *(int *)( self + 116 ) = 0;
  *(int *)( self + 124 ) = 0;
  *(int *)( self + 128 ) = 0;
  *(int *)( self + 132 ) = 0;
  *(int *)( self + 140 ) = 0;
  *(int *)( self + 144 ) = 0;
  *(int *)( self + 148 ) = 0;
  *(int *)( self + 156 ) = 0;
  *(int *)( self + 160 ) = 0;
  *(int *)( self + 164 ) = 0;
  *(int *)( self + 172 ) = 0;
  *(int *)( self + 176 ) = 0;
  *(int *)( self + 180 ) = 0;
  *(int *)( self + 220 ) = 0;
  *(int *)( self + 224 ) = 0;
  *(int *)( self + 228 ) = 0;
  *(int *)( self + 232 ) = 0;
  *(int *)( self + 236 ) = 0;
  *(int *)( self + 240 ) = 0;
  *(int *)( self + 244 ) = 0;
  *(int *)( self + 248 ) = 0;
  *(int *)( self + 252 ) = 0;
  *(int *)( self + 256 ) = 0;
  *(int *)( self + 260 ) = 0;
  *(int *)( self + 264 ) = 0;
  *(int *)( self + 268 ) = 0;
  *(int *)( self + 272 ) = 0;
  *(int *)( self + 276 ) = 0;
  *(int *)( self + 280 ) = 0;
  *(int *)( self + 284 ) = 0;
  *(int *)( self + 288 ) = 0;
  *(int *)( self + 292 ) = 0;
  *(int *)( self + 296 ) = 0;
  *(int *)( self + 300 ) = 0;
  *(int *)( self + 304 ) = 0;
  *(int *)( self + 308 ) = 0;
  *(int *)( self + 312 ) = 0;
  *(int *)( self + 316 ) = 0;
  *(int *)( self + 320 ) = 0;
  *(int *)( self + 324 ) = 0;
  *(int *)( self + 328 ) = 0;
  *(int *)( self + 332 ) = 0;
  *(int *)( self + 336 ) = 0;
  *(int *)( self + 340 ) = 0;
  *(int *)( self + 344 ) = 0;
  *(int *)( self + 348 ) = 0;
  *(int *)( self + 352 ) = 0;
  *(int *)( self + 356 ) = 0;
  *(int *)( self + 360 ) = 0;
  *(int *)( self + 364 ) = 0;
  *(int *)( self + 368 ) = 0;
  *(int *)( self + 372 ) = 0;
  *(int *)( self + 376 ) = 0;
  *(int *)( self + 380 ) = 0;
  *(int *)( self + 384 ) = 0;
  *(int *)( self + 388 ) = 0;
  *(int *)( self + 392 ) = 0;
  *(int *)( self + 396 ) = 0;
  *(int *)( self + 400 ) = 0;
  *(int *)( self + 404 ) = 0;
  *(int *)( self + 408 ) = 0;
  *(int *)( self + 412 ) = 0;
  *(int *)( self + 416 ) = 0;
  *(int *)( self + 420 ) = 0;
  *(int *)( self + 424 ) = 0;
  *(int *)( self + 428 ) = 0;
  *(int *)( self + 432 ) = 0;
  *(int *)( self + 436 ) = 0;
  *(int *)( self + 440 ) = 0;
  *(int *)( self + 444 ) = 0;
  *(int *)( self + 448 ) = 0;
  *(int *)( self + 452 ) = 0;
  *(int *)( self + 456 ) = 0;
  *(int *)( self + 460 ) = 0;
  *(int *)( self + 464 ) = 0;
  *(int *)( self + 468 ) = 0;
  *(int *)( self + 472 ) = 0;
  *(int *)( self + 476 ) = 0;
  *(int *)( self + 480 ) = 0;
  *(int *)( self + 484 ) = 0;
  *(int *)( self + 488 ) = 0;
  *(int *)( self + 492 ) = 0;
  *(int *)( self + 496 ) = 0;
  *(int *)( self + 500 ) = 0;
  *(int *)( self + 504 ) = 0;
  *(int *)( self + 508 ) = 0;
  *(int *)( self + 512 ) = 0;
  *(int *)( self + 516 ) = 0;
  *(int *)( self + 520 ) = 0;
  *(int *)( self + 524 ) = 0;
  *(int *)( self + 528 ) = 0;
  *(int *)( self + 532 ) = 0;
  *(int *)( self + 536 ) = 0;
  *(int *)( self + 540 ) = 0;
  *(int *)( self + 544 ) = 0;
  *(int *)( self + 548 ) = 0;
  *(int *)( self + 552 ) = 0;
  *(int *)( self + 556 ) = 0;
  *(int *)( self + 560 ) = 0;
  *(int *)( self + 564 ) = 0;
  *(int *)( self + 568 ) = 0;
  *(int *)( self + 572 ) = 0;
  *(int *)( self + 576 ) = 0;
  *(int *)( self + 580 ) = 0;
  *(int *)( self + 584 ) = 0;
  *(int *)( self + 588 ) = 0;
  *(int *)( self + 592 ) = 0;
  *(int *)( self + 596 ) = 0;
  *(int *)( self + 600 ) = 0;
  *(int *)( self + 0   ) = 0;
  *(int *)( self + 8   ) = 0;
  *(int *)( self + 100 ) = 0;
  *(int *)( self + 188 ) = 0;
  *(int *)( self + 184 ) = 0;
  *(int *)( self + 192 ) = 0;
  *(float *)( self + 92  ) = 1.0f;
  *(float *)( self + 96  ) = 1.0f;
  *(float *)( self + 84  ) = 1.0f;
  *(float *)( self + 88  ) = 1.0f;
  *(float *)( self + 268 ) = 1.0f;
  *(float *)( self + 272 ) = 1.0f;
  *(float *)( self + 276 ) = 1.0f;
  *(float *)( self + 280 ) = 1.0f;
  *(int *)( self + 40  ) = 0;
  *(int *)( self + 196 ) = 0;
  *(int *)( self + 200 ) = 0;
  *(int *)( self + 204 ) = 0;
  *(int *)( self + 208 ) = 0;
  *(int *)( self + 212 ) = 0;
  *(int *)( self + 216 ) = 0;
  *(float *)( self + 428 ) = 1.0f;
  *(float *)( self + 432 ) = 1.0f;
  *(float *)( self + 436 ) = 1.0f;
  *(float *)( self + 440 ) = 1.0f;
  *(float *)( self + 444 ) = 1.0f;
  *(float *)( self + 448 ) = 1.0f;
  *(float *)( self + 452 ) = 1.0f;
  *(float *)( self + 456 ) = 1.0f;
  *(float *)( self + 460 ) = 1.0f;
  *(float *)( self + 464 ) = 1.0f;
  *(float *)( self + 468 ) = 1.0f;
  *(float *)( self + 472 ) = 1.0f;
  *(float *)( self + 484 ) = 1.0f;
  *(float *)( self + 488 ) = 1.0f;
  *(float *)( self + 492 ) = 1.0f;
  *(float *)( self + 496 ) = 1.0f;
  *(float *)( self + 508 ) = 1.0f;
  *(float *)( self + 512 ) = 1.0f;
  *(float *)( self + 516 ) = 1.0f;
  *(float *)( self + 520 ) = 1.0f;
  *(float *)( self + 532 ) = 1.0f;
  *(float *)( self + 536 ) = 1.0f;
  *(float *)( self + 540 ) = 1.0f;
  *(float *)( self + 544 ) = 1.0f;
  *(float *)( self + 556 ) = 1.0f;
  *(float *)( self + 560 ) = 1.0f;
  *(float *)( self + 564 ) = 1.0f;
  *(float *)( self + 568 ) = 1.0f;
  *(float *)( self + 580 ) = 1.0f;
  *(float *)( self + 584 ) = 1.0f;
  *(float *)( self + 588 ) = 1.0f;
  *(float *)( self + 592 ) = 1.0f;
  *(float *)( self + 420 ) = 1.0f;
  *(float *)( self + 424 ) = 1.0f;
  *(float *)( self + 412 ) = 10.0f;
  *(float *)( self + 416 ) = 10.0f;
  return self;
}
/* ---- sub_49A880  0x0049A880 ----  [CONFIRMED] */
_DWORD *__cdecl sub_49A880(_DWORD *result)
{
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;
  return result;
}

/* ---- CPrimitiveTemplate__copy_m  0x0049A890 ----  VERIFIED */
int __cdecl CPrimitiveTemplate__copy_m(char *a1, char *a2)
{
  char *v2;
  char v3;
  char *v4;
  char v5;
  int result;

  v2 = a2 + 8;
  do
  {
    v3 = *v2;
    v2[a1 - a2] = *v2;
    ++v2;
  }
  while ( v3 );
  *((_DWORD *)a1 + 18) = *((_DWORD *)a2 + 18);
  *((_DWORD *)a1 + 19) = *((_DWORD *)a2 + 19);
  *((_DWORD *)a1 + 20) = *((_DWORD *)a2 + 20);
  *((_DWORD *)a1 + 21) = *((_DWORD *)a2 + 21);
  *((_DWORD *)a1 + 22) = *((_DWORD *)a2 + 22);
  *((_DWORD *)a1 + 23) = *((_DWORD *)a2 + 23);
  *((_DWORD *)a1 + 24) = *((_DWORD *)a2 + 24);
  *((_DWORD *)a1 + 25) = *((_DWORD *)a2 + 25);
  sub_492A80((int)(a1 + 104), (int)(a2 + 104));
  sub_492A80((int)(a1 + 120), (int)(a2 + 120));
  sub_492A80((int)(a1 + 136), (int)(a2 + 136));
  sub_492A80((int)(a1 + 152), (int)(a2 + 152));
  sub_492A80((int)(a1 + 168), (int)(a2 + 168));
  v4 = a2 + 40;
  do
  {
    v5 = *v4;
    v4[a1 - a2] = *v4;
    ++v4;
  }
  while ( v5 );
  *((_DWORD *)a1 + 46) = *((_DWORD *)a2 + 46);
  *((_DWORD *)a1 + 47) = *((_DWORD *)a2 + 47);
  *((_DWORD *)a1 + 49) = *((_DWORD *)a2 + 49);
  *((_DWORD *)a1 + 50) = *((_DWORD *)a2 + 50);
  *((_DWORD *)a1 + 51) = *((_DWORD *)a2 + 51);
  *((_DWORD *)a1 + 52) = *((_DWORD *)a2 + 52);
  *((_DWORD *)a1 + 53) = *((_DWORD *)a2 + 53);
  *((_DWORD *)a1 + 54) = *((_DWORD *)a2 + 54);
  *((_DWORD *)a1 + 55) = *((_DWORD *)a2 + 55);
  *((_DWORD *)a1 + 56) = *((_DWORD *)a2 + 56);
  *((_DWORD *)a1 + 57) = *((_DWORD *)a2 + 57);
  *((_DWORD *)a1 + 58) = *((_DWORD *)a2 + 58);
  *((_DWORD *)a1 + 59) = *((_DWORD *)a2 + 59);
  *((_DWORD *)a1 + 60) = *((_DWORD *)a2 + 60);
  *((_DWORD *)a1 + 61) = *((_DWORD *)a2 + 61);
  *((_DWORD *)a1 + 62) = *((_DWORD *)a2 + 62);
  *((_DWORD *)a1 + 63) = *((_DWORD *)a2 + 63);
  *((_DWORD *)a1 + 64) = *((_DWORD *)a2 + 64);
  *((_DWORD *)a1 + 65) = *((_DWORD *)a2 + 65);
  *((_DWORD *)a1 + 66) = *((_DWORD *)a2 + 66);
  *((_DWORD *)a1 + 67) = *((_DWORD *)a2 + 67);
  *((_DWORD *)a1 + 68) = *((_DWORD *)a2 + 68);
  *((_DWORD *)a1 + 73) = *((_DWORD *)a2 + 73);
  *((_DWORD *)a1 + 74) = *((_DWORD *)a2 + 74);
  *((_DWORD *)a1 + 75) = *((_DWORD *)a2 + 75);
  *((_DWORD *)a1 + 76) = *((_DWORD *)a2 + 76);
  *((_DWORD *)a1 + 77) = *((_DWORD *)a2 + 77);
  *((_DWORD *)a1 + 78) = *((_DWORD *)a2 + 78);
  *((_DWORD *)a1 + 79) = *((_DWORD *)a2 + 79);
  *((_DWORD *)a1 + 80) = *((_DWORD *)a2 + 80);
  *((_DWORD *)a1 + 81) = *((_DWORD *)a2 + 81);
  *((_DWORD *)a1 + 82) = *((_DWORD *)a2 + 82);
  *((_DWORD *)a1 + 83) = *((_DWORD *)a2 + 83);
  *((_DWORD *)a1 + 84) = *((_DWORD *)a2 + 84);
  *((_DWORD *)a1 + 85) = *((_DWORD *)a2 + 85);
  *((_DWORD *)a1 + 86) = *((_DWORD *)a2 + 86);
  *((_DWORD *)a1 + 87) = *((_DWORD *)a2 + 87);
  *((_DWORD *)a1 + 88) = *((_DWORD *)a2 + 88);
  *((_DWORD *)a1 + 89) = *((_DWORD *)a2 + 89);
  *((_DWORD *)a1 + 90) = *((_DWORD *)a2 + 90);
  *((_DWORD *)a1 + 91) = *((_DWORD *)a2 + 91);
  *((_DWORD *)a1 + 92) = *((_DWORD *)a2 + 92);
  *((_DWORD *)a1 + 93) = *((_DWORD *)a2 + 93);
  *((_DWORD *)a1 + 94) = *((_DWORD *)a2 + 94);
  *((_DWORD *)a1 + 95) = *((_DWORD *)a2 + 95);
  *((_DWORD *)a1 + 96) = *((_DWORD *)a2 + 96);
  *((_DWORD *)a1 + 97) = *((_DWORD *)a2 + 97);
  *((_DWORD *)a1 + 98) = *((_DWORD *)a2 + 98);
  *((_DWORD *)a1 + 99) = *((_DWORD *)a2 + 99);
  *((_DWORD *)a1 + 100) = *((_DWORD *)a2 + 100);
  *((_DWORD *)a1 + 101) = *((_DWORD *)a2 + 101);
  *((_DWORD *)a1 + 102) = *((_DWORD *)a2 + 102);
  *((_DWORD *)a1 + 103) = *((_DWORD *)a2 + 103);
  *((_DWORD *)a1 + 104) = *((_DWORD *)a2 + 104);
  *((_DWORD *)a1 + 105) = *((_DWORD *)a2 + 105);
  *((_DWORD *)a1 + 106) = *((_DWORD *)a2 + 106);
  *((_DWORD *)a1 + 107) = *((_DWORD *)a2 + 107);
  *((_DWORD *)a1 + 108) = *((_DWORD *)a2 + 108);
  *((_DWORD *)a1 + 109) = *((_DWORD *)a2 + 109);
  *((_DWORD *)a1 + 110) = *((_DWORD *)a2 + 110);
  *((_DWORD *)a1 + 111) = *((_DWORD *)a2 + 111);
  *((_DWORD *)a1 + 112) = *((_DWORD *)a2 + 112);
  *((_DWORD *)a1 + 113) = *((_DWORD *)a2 + 113);
  *((_DWORD *)a1 + 114) = *((_DWORD *)a2 + 114);
  *((_DWORD *)a1 + 115) = *((_DWORD *)a2 + 115);
  *((_DWORD *)a1 + 116) = *((_DWORD *)a2 + 116);
  *((_DWORD *)a1 + 117) = *((_DWORD *)a2 + 117);
  *((_DWORD *)a1 + 118) = *((_DWORD *)a2 + 118);
  *((_DWORD *)a1 + 119) = *((_DWORD *)a2 + 119);
  *((_DWORD *)a1 + 120) = *((_DWORD *)a2 + 120);
  *((_DWORD *)a1 + 121) = *((_DWORD *)a2 + 121);
  *((_DWORD *)a1 + 122) = *((_DWORD *)a2 + 122);
  *((_DWORD *)a1 + 123) = *((_DWORD *)a2 + 123);
  *((_DWORD *)a1 + 124) = *((_DWORD *)a2 + 124);
  *((_DWORD *)a1 + 125) = *((_DWORD *)a2 + 125);
  *((_DWORD *)a1 + 126) = *((_DWORD *)a2 + 126);
  *((_DWORD *)a1 + 127) = *((_DWORD *)a2 + 127);
  *((_DWORD *)a1 + 128) = *((_DWORD *)a2 + 128);
  *((_DWORD *)a1 + 129) = *((_DWORD *)a2 + 129);
  *((_DWORD *)a1 + 130) = *((_DWORD *)a2 + 130);
  *((_DWORD *)a1 + 131) = *((_DWORD *)a2 + 131);
  *((_DWORD *)a1 + 132) = *((_DWORD *)a2 + 132);
  *((_DWORD *)a1 + 133) = *((_DWORD *)a2 + 133);
  *((_DWORD *)a1 + 134) = *((_DWORD *)a2 + 134);
  *((_DWORD *)a1 + 135) = *((_DWORD *)a2 + 135);
  *((_DWORD *)a1 + 136) = *((_DWORD *)a2 + 136);
  *((_DWORD *)a1 + 137) = *((_DWORD *)a2 + 137);
  *((_DWORD *)a1 + 138) = *((_DWORD *)a2 + 138);
  *((_DWORD *)a1 + 139) = *((_DWORD *)a2 + 139);
  *((_DWORD *)a1 + 140) = *((_DWORD *)a2 + 140);
  *((_DWORD *)a1 + 141) = *((_DWORD *)a2 + 141);
  *((_DWORD *)a1 + 142) = *((_DWORD *)a2 + 142);
  *((_DWORD *)a1 + 143) = *((_DWORD *)a2 + 143);
  *((_DWORD *)a1 + 144) = *((_DWORD *)a2 + 144);
  *((_DWORD *)a1 + 145) = *((_DWORD *)a2 + 145);
  *((_DWORD *)a1 + 146) = *((_DWORD *)a2 + 146);
  *((_DWORD *)a1 + 147) = *((_DWORD *)a2 + 147);
  *((_DWORD *)a1 + 148) = *((_DWORD *)a2 + 148);
  result = *((_DWORD *)a2 + 149);
  *((_DWORD *)a1 + 149) = result;
  *((_DWORD *)a1 + 150) = *((_DWORD *)a2 + 150);
  return result;
}

/* ---- CPrimitiveTemplate__assign_m  0x0049ADF0 ----  VERIFIED */
int __cdecl CPrimitiveTemplate__assign_m(char *a1, char *a2)
{
  char *v2;
  char v3;
  int result;

  v2 = a1 + 8;
  do
  {
    v3 = *v2;
    v2[a2 - a1] = *v2;
    ++v2;
  }
  while ( v3 );
  *((_DWORD *)a2 + 18) = *((_DWORD *)a1 + 18);
  *((_DWORD *)a2 + 19) = *((_DWORD *)a1 + 19);
  *((_DWORD *)a2 + 20) = *((_DWORD *)a1 + 20);
  *((_DWORD *)a2 + 21) = *((_DWORD *)a1 + 21);
  *((_DWORD *)a2 + 22) = *((_DWORD *)a1 + 22);
  *((_DWORD *)a2 + 23) = *((_DWORD *)a1 + 23);
  *((_DWORD *)a2 + 24) = *((_DWORD *)a1 + 24);
  sub_492A80((int)(a2 + 104), (int)(a1 + 104));
  sub_492A80((int)(a2 + 120), (int)(a1 + 120));
  sub_492A80((int)(a2 + 136), (int)(a1 + 136));
  sub_492A80((int)(a2 + 152), (int)(a1 + 152));
  sub_492A80((int)(a2 + 168), (int)(a1 + 168));
  *((_DWORD *)a2 + 46) = *((_DWORD *)a1 + 46);
  *((_DWORD *)a2 + 47) = *((_DWORD *)a1 + 47);
  *((_DWORD *)a2 + 49) = *((_DWORD *)a1 + 49);
  *((_DWORD *)a2 + 50) = *((_DWORD *)a1 + 50);
  *((_DWORD *)a2 + 51) = *((_DWORD *)a1 + 51);
  *((_DWORD *)a2 + 52) = *((_DWORD *)a1 + 52);
  *((_DWORD *)a2 + 53) = *((_DWORD *)a1 + 53);
  *((_DWORD *)a2 + 54) = *((_DWORD *)a1 + 54);
  *((_DWORD *)a2 + 55) = *((_DWORD *)a1 + 55);
  *((_DWORD *)a2 + 56) = *((_DWORD *)a1 + 56);
  *((_DWORD *)a2 + 57) = *((_DWORD *)a1 + 57);
  *((_DWORD *)a2 + 58) = *((_DWORD *)a1 + 58);
  *((_DWORD *)a2 + 59) = *((_DWORD *)a1 + 59);
  *((_DWORD *)a2 + 60) = *((_DWORD *)a1 + 60);
  *((_DWORD *)a2 + 61) = *((_DWORD *)a1 + 61);
  *((_DWORD *)a2 + 62) = *((_DWORD *)a1 + 62);
  *((_DWORD *)a2 + 63) = *((_DWORD *)a1 + 63);
  *((_DWORD *)a2 + 64) = *((_DWORD *)a1 + 64);
  *((_DWORD *)a2 + 65) = *((_DWORD *)a1 + 65);
  *((_DWORD *)a2 + 66) = *((_DWORD *)a1 + 66);
  *((_DWORD *)a2 + 67) = *((_DWORD *)a1 + 67);
  *((_DWORD *)a2 + 68) = *((_DWORD *)a1 + 68);
  *((_DWORD *)a2 + 73) = *((_DWORD *)a1 + 73);
  *((_DWORD *)a2 + 74) = *((_DWORD *)a1 + 74);
  *((_DWORD *)a2 + 75) = *((_DWORD *)a1 + 75);
  *((_DWORD *)a2 + 76) = *((_DWORD *)a1 + 76);
  *((_DWORD *)a2 + 77) = *((_DWORD *)a1 + 77);
  *((_DWORD *)a2 + 78) = *((_DWORD *)a1 + 78);
  *((_DWORD *)a2 + 79) = *((_DWORD *)a1 + 79);
  *((_DWORD *)a2 + 80) = *((_DWORD *)a1 + 80);
  *((_DWORD *)a2 + 81) = *((_DWORD *)a1 + 81);
  *((_DWORD *)a2 + 82) = *((_DWORD *)a1 + 82);
  *((_DWORD *)a2 + 83) = *((_DWORD *)a1 + 83);
  *((_DWORD *)a2 + 84) = *((_DWORD *)a1 + 84);
  *((_DWORD *)a2 + 85) = *((_DWORD *)a1 + 85);
  *((_DWORD *)a2 + 86) = *((_DWORD *)a1 + 86);
  *((_DWORD *)a2 + 87) = *((_DWORD *)a1 + 87);
  *((_DWORD *)a2 + 88) = *((_DWORD *)a1 + 88);
  *((_DWORD *)a2 + 89) = *((_DWORD *)a1 + 89);
  *((_DWORD *)a2 + 90) = *((_DWORD *)a1 + 90);
  *((_DWORD *)a2 + 91) = *((_DWORD *)a1 + 91);
  *((_DWORD *)a2 + 92) = *((_DWORD *)a1 + 92);
  *((_DWORD *)a2 + 93) = *((_DWORD *)a1 + 93);
  *((_DWORD *)a2 + 94) = *((_DWORD *)a1 + 94);
  *((_DWORD *)a2 + 95) = *((_DWORD *)a1 + 95);
  *((_DWORD *)a2 + 96) = *((_DWORD *)a1 + 96);
  *((_DWORD *)a2 + 97) = *((_DWORD *)a1 + 97);
  *((_DWORD *)a2 + 98) = *((_DWORD *)a1 + 98);
  *((_DWORD *)a2 + 99) = *((_DWORD *)a1 + 99);
  *((_DWORD *)a2 + 100) = *((_DWORD *)a1 + 100);
  *((_DWORD *)a2 + 101) = *((_DWORD *)a1 + 101);
  *((_DWORD *)a2 + 102) = *((_DWORD *)a1 + 102);
  *((_DWORD *)a2 + 103) = *((_DWORD *)a1 + 103);
  *((_DWORD *)a2 + 104) = *((_DWORD *)a1 + 104);
  *((_DWORD *)a2 + 105) = *((_DWORD *)a1 + 105);
  *((_DWORD *)a2 + 106) = *((_DWORD *)a1 + 106);
  *((_DWORD *)a2 + 107) = *((_DWORD *)a1 + 107);
  *((_DWORD *)a2 + 108) = *((_DWORD *)a1 + 108);
  *((_DWORD *)a2 + 109) = *((_DWORD *)a1 + 109);
  *((_DWORD *)a2 + 110) = *((_DWORD *)a1 + 110);
  *((_DWORD *)a2 + 111) = *((_DWORD *)a1 + 111);
  *((_DWORD *)a2 + 112) = *((_DWORD *)a1 + 112);
  *((_DWORD *)a2 + 113) = *((_DWORD *)a1 + 113);
  *((_DWORD *)a2 + 114) = *((_DWORD *)a1 + 114);
  *((_DWORD *)a2 + 115) = *((_DWORD *)a1 + 115);
  *((_DWORD *)a2 + 116) = *((_DWORD *)a1 + 116);
  *((_DWORD *)a2 + 117) = *((_DWORD *)a1 + 117);
  *((_DWORD *)a2 + 118) = *((_DWORD *)a1 + 118);
  *((_DWORD *)a2 + 119) = *((_DWORD *)a1 + 119);
  *((_DWORD *)a2 + 120) = *((_DWORD *)a1 + 120);
  *((_DWORD *)a2 + 121) = *((_DWORD *)a1 + 121);
  *((_DWORD *)a2 + 122) = *((_DWORD *)a1 + 122);
  *((_DWORD *)a2 + 123) = *((_DWORD *)a1 + 123);
  *((_DWORD *)a2 + 124) = *((_DWORD *)a1 + 124);
  *((_DWORD *)a2 + 125) = *((_DWORD *)a1 + 125);
  *((_DWORD *)a2 + 126) = *((_DWORD *)a1 + 126);
  *((_DWORD *)a2 + 127) = *((_DWORD *)a1 + 127);
  *((_DWORD *)a2 + 128) = *((_DWORD *)a1 + 128);
  *((_DWORD *)a2 + 129) = *((_DWORD *)a1 + 129);
  *((_DWORD *)a2 + 130) = *((_DWORD *)a1 + 130);
  *((_DWORD *)a2 + 131) = *((_DWORD *)a1 + 131);
  *((_DWORD *)a2 + 132) = *((_DWORD *)a1 + 132);
  *((_DWORD *)a2 + 133) = *((_DWORD *)a1 + 133);
  *((_DWORD *)a2 + 134) = *((_DWORD *)a1 + 134);
  *((_DWORD *)a2 + 135) = *((_DWORD *)a1 + 135);
  *((_DWORD *)a2 + 136) = *((_DWORD *)a1 + 136);
  *((_DWORD *)a2 + 137) = *((_DWORD *)a1 + 137);
  *((_DWORD *)a2 + 138) = *((_DWORD *)a1 + 138);
  *((_DWORD *)a2 + 139) = *((_DWORD *)a1 + 139);
  *((_DWORD *)a2 + 140) = *((_DWORD *)a1 + 140);
  *((_DWORD *)a2 + 141) = *((_DWORD *)a1 + 141);
  *((_DWORD *)a2 + 142) = *((_DWORD *)a1 + 142);
  *((_DWORD *)a2 + 143) = *((_DWORD *)a1 + 143);
  *((_DWORD *)a2 + 144) = *((_DWORD *)a1 + 144);
  *((_DWORD *)a2 + 145) = *((_DWORD *)a1 + 145);
  *((_DWORD *)a2 + 146) = *((_DWORD *)a1 + 146);
  *((_DWORD *)a2 + 147) = *((_DWORD *)a1 + 147);
  *((_DWORD *)a2 + 148) = *((_DWORD *)a1 + 148);
  *((_DWORD *)a2 + 149) = *((_DWORD *)a1 + 149);
  result = *((_DWORD *)a1 + 150);
  *((_DWORD *)a2 + 150) = result;
  return result;
}

/* ---- CPrimitiveTemplate__ParseFloat  0x0049B330 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseFloat(const char *a1, _DWORD *a2, _DWORD *a3, int a4)
{
  int v4;

  if ( !a3 )
    return 0;
  if ( !a2 )
    return 0;
  v4 = sscanf(a1, "%f %f", a3, a2);
  if ( !v4 )
    return 0;
  if ( v4 == 1 )
    *a2 = *a3;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseVector  0x0049B360 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseVector(_DWORD *a1, _DWORD *a2, int a3, char *Buffer)
{
  int v4;

  if ( !a2 )
    return 0;
  if ( !a1 )
    return 0;
  v4 = sscanf(Buffer, "%f %f %f   %f %f %f", a2, a2 + 1, a2 + 2, a1, a1 + 1, a1 + 2);
  if ( v4 < 3 || v4 == 4 || v4 == 5 )
    return 0;
  if ( v4 == 3 )
  {
    *a1 = *a2;
    a1[1] = a2[1];
    a1[2] = a2[2];
  }
  return 1;
}

/* ---- CPrimitiveTemplate__ParseGroupFlags  0x0049B3E0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseGroupFlags(_DWORD *a1, int a2, char *Buffer)
{
  int v4;
  int v5;
  char *i;
  int v7;
  char v8;
  char flags[4][32]; // [esp+8h] [ebp-84h] BYREF

  if ( !a1 )
    return 0;
  memset( flags, 0, sizeof( flags ) );
  flags[3][0] = '0';
  v8 = 1;
  v4 = sscanf(Buffer, "%s %s %s %s", flags[0], flags[1], flags[2], flags[3]);
  *a1 = 0;
  v5 = 0;
  for ( i = flags[0]; ; i += 32 )
  {
    v7 = v5 + 1;
    if ( v5 + 1 > v4 )
      break;
    if ( !_stricmp(i, "linear") )
    {
      *a1 |= 1u;
    }
    else if ( !_stricmp(i, "nonlinear") )
    {
      *a1 |= 4u;
    }
    else if ( !_stricmp(i, "wave") )
    {
      *a1 |= 8u;
    }
    else if ( !_stricmp(i, "random") )
    {
      *a1 |= 2u;
    }
    else if ( !_stricmp(i, "clamp") )
    {
      *a1 |= 0xCu;
    }
    else
    {
      v8 = 0;
    }
    v5 = v7;
    if ( v7 >= 4 )
      return v8;
  }
  return 1;
}

/* ---- CPrimitiveTemplate__ParseMin_m  0x0049B550 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseMin_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  _DWORD v5[3]; // [esp+8h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v5, v5, (int)a2, a1) != 1 )
    return 0;
  v2 = v5[0];
  v3 = v5[1];
  a2[51] = v5[2];
  a2[46] |= 0x6000000u;
  a2[49] = v2;
  a2[50] = v3;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseMax_m  0x0049B5A0 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseMax_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  _DWORD v5[3]; // [esp+8h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v5, v5, (int)a2, a1) != 1 )
    return 0;
  v2 = v5[0];
  v3 = v5[1];
  a2[54] = v5[2];
  a2[46] |= 0x6000000u;
  a2[52] = v2;
  a2[53] = v3;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseLife_m  0x0049B5F0 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseLife_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v5; // [esp+0h] [ebp-8h] BYREF
  float v6; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v5, &v6);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v5;
  else
    v3 = v6;
  *(float *)(a2 + 92) = v5;
  *(float *)(a2 + 96) = v3;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseDelay_m  0x0049B650 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseDelay_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v5; // [esp+0h] [ebp-8h] BYREF
  float v6; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v5, &v6);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v5;
  else
    v3 = v6;
  *(float *)(a2 + 76) = v5;
  *(float *)(a2 + 80) = v3;
  return 1;
}

/* ---- sub_49B6B0  0x0049B6B0 ----  [HIGH] */
char __cdecl sub_49B6B0(const char *a1, int a2)
{
  int v2;
  double v3;
  float v5; // [esp+0h] [ebp-8h] BYREF
  float v6; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v5, &v6);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v5;
  else
    v3 = v6;
  *(float *)(a2 + 84) = v5;
  *(float *)(a2 + 88) = v3;
  return 1;
}

/* ---- sub_49B710  0x0049B710 ----  VERIFIED */
char __cdecl sub_49B710(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 600) = v3;
  *(float *)(a2 + 596) = v4;
  *(_DWORD *)(a2 + 184) |= 0x2000000u;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseOrigin_m  0x0049B770 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseOrigin_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[55] = v8[0];
  v4 = v9[1];
  a2[56] = v2;
  v5 = v8[2];
  a2[57] = v3;
  v6 = v9[2];
  a2[58] = v4;
  a2[60] = v6;
  a2[59] = v5;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseOrigin2_m  0x0049B7E0 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseOrigin2_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[61] = v8[0];
  v4 = v9[1];
  a2[62] = v2;
  v5 = v8[2];
  a2[63] = v3;
  v6 = v9[2];
  a2[64] = v4;
  a2[66] = v6;
  a2[65] = v5;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseRadius_m  0x0049B850 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseRadius_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 272) = v3;
  *(float *)(a2 + 268) = v4;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseHeight_m  0x0049B8A0 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseHeight_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 280) = v3;
  *(float *)(a2 + 276) = v4;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseWind_m  0x0049B8F0 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseWind_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 288) = v3;
  *(float *)(a2 + 284) = v4;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseRotation_m  0x0049B940 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseRotation_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 296) = v3;
  *(float *)(a2 + 292) = v4;
  return 1;
}

/* ---- sub_49B990  0x0049B990 ----  [HIGH] */
char __cdecl sub_49B990(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 304) = v3;
  *(float *)(a2 + 300) = v4;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseAngle_m  0x0049B9E0 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseAngle_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[77] = v8[0];
  v4 = v9[1];
  a2[78] = v2;
  v5 = v8[2];
  a2[79] = v3;
  v6 = v9[2];
  a2[80] = v4;
  a2[82] = v6;
  a2[81] = v5;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseAngleDelta_m  0x0049BA50 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseAngleDelta_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[83] = v8[0];
  v4 = v9[1];
  a2[84] = v2;
  v5 = v8[2];
  a2[85] = v3;
  v6 = v9[2];
  a2[86] = v4;
  a2[88] = v6;
  a2[87] = v5;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseVelocity_m  0x0049BAC0 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseVelocity_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[89] = v8[0];
  v4 = v9[1];
  a2[90] = v2;
  v5 = v8[2];
  a2[91] = v3;
  v6 = v9[2];
  a2[92] = v4;
  a2[94] = v6;
  a2[93] = v5;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseFlags  0x0049BB30 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseFlags( int a1, char *Buffer )
{
  char  flags[7][32];
  char  ok;
  int   count;
  int   i;
  char *tok;

  memset( flags, 0, sizeof( flags ) );
  ok = 1;

  count = sscanf( Buffer, "%s %s %s %s %s %s %s",
                  flags[0], flags[1], flags[2], flags[3],
                  flags[4], flags[5], flags[6] );

  i   = 0;
  tok = flags[0];
  while ( 1 )
  {
    if ( i + 1 > count )
      return 1;

    if ( !_stricmp( tok, "useModel" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x01000000u;
    else if ( !_stricmp( tok, "useBBox" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x04000000u;
    else if ( !_stricmp( tok, "usePhysics" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x02000000u;
    else if ( !_stricmp( tok, "expensivePhysics" ) )
      ;
    else if ( !_stricmp( tok, "impactKills" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x40000000u;
    else if ( !_stricmp( tok, "impactFx" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x80000000u;
    else if ( !_stricmp( tok, "deathFx" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x20000000u;
    else if ( !_stricmp( tok, "useAlpha" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x08000000u;
    else if ( !_stricmp( tok, "emitFx" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x10000000u;
    else if ( !_stricmp( tok, "depthHack" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x00100000u;
    else if ( !_stricmp( tok, "relative" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x00200000u;
    else if ( !_stricmp( tok, "setShaderTime" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x00400000u;
    else if ( !_stricmp( tok, "continualLighting" ) )
      *(_DWORD *)( a1 + 184 ) |= 0x00800000u;
    else
      ok = 0;

    ++i;
    tok += 32;
    if ( i >= 7 )
      return ok;
  }
}

/* ---- CPrimitiveTemplate__ParseSpawnFlags  0x0049BE30 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseSpawnFlags(int a1, char *Buffer)
{
  int v2;
  char *i;
  int v4;
  char v6;
  int v7;
  char flags[7][32]; // [esp+10h] [ebp-E8h] BYREF

  memset( flags, 0, sizeof( flags ) );
  v6 = 1;
  v7 = sscanf(Buffer, "%s %s %s %s %s %s %s", flags[0], flags[1], flags[2],
              flags[3], flags[4], flags[5], flags[6]);
  v2 = 0;
  for ( i = flags[0]; ; i += 32 )
  {
    v4 = v2 + 1;
    if ( v2 + 1 > v7 )
      break;
    if ( !_stricmp(i, "org2fromTrace") )
    {
      *(_DWORD *)(a1 + 188) |= 0x10u;
    }
    else if ( !_stricmp(i, "traceImpactFx") )
    {
      *(_DWORD *)(a1 + 188) |= 0x20u;
    }
    else if ( !_stricmp(i, "org2isOffset") )
    {
      *(_DWORD *)(a1 + 188) |= 0x40u;
    }
    else if ( !_stricmp(i, "cheapOrgCalc") )
    {
      *(_DWORD *)(a1 + 188) |= 0x100u;
    }
    else if ( !_stricmp(i, "cheapOrg2Calc") )
    {
      *(_DWORD *)(a1 + 188) |= 0x200u;
    }
    else if ( !_stricmp(i, "absoluteVel") )
    {
      *(_DWORD *)(a1 + 188) |= 0x400u;
    }
    else if ( !_stricmp(i, "absoluteAccel") )
    {
      *(_DWORD *)(a1 + 188) |= 0x800u;
    }
    else if ( !_stricmp(i, "orgOnSphere") )
    {
      *(_DWORD *)(a1 + 188) |= 1u;
    }
    else if ( !_stricmp(i, "orgOnCylinder") )
    {
      *(_DWORD *)(a1 + 188) |= 4u;
    }
    else if ( !_stricmp(i, "axisFromSphere") )
    {
      *(_DWORD *)(a1 + 188) |= 2u;
    }
    else if ( !_stricmp(i, "randrotaroundfwd") )
    {
      *(_DWORD *)(a1 + 188) |= 0x1000u;
    }
    else if ( !_stricmp(i, "evenDistribution") )
    {
      *(_DWORD *)(a1 + 188) |= 0x2000u;
    }
    else if ( !_stricmp(i, "rgbComponentInterpolation") )
    {
      *(_DWORD *)(a1 + 188) |= 0x4000u;
    }
    else if ( !_stricmp(i, "affectedByWind") )
    {
      *(_DWORD *)(a1 + 188) |= 0x10000u;
    }
    else if ( !_stricmp(i, "lessAttenuation") )
    {
      *(_DWORD *)(a1 + 188) |= 0x20000u;
    }
    else
    {
      v6 = 0;
    }
    v2 = v4;
    if ( v4 >= 7 )
      return v6;
  }
  return 1;
}

/* ---- CPrimitiveTemplate__ParseAcceleration_m  0x0049C170 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseAcceleration_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[95] = v8[0];
  v4 = v9[1];
  a2[96] = v2;
  v5 = v8[2];
  a2[97] = v3;
  v6 = v9[2];
  a2[98] = v4;
  a2[100] = v6;
  a2[99] = v5;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseGravity_m  0x0049C1E0 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseGravity_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 408) = v3;
  *(float *)(a2 + 404) = v4;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseDensity_m  0x0049C230 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseDensity_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 416) = v3;
  *(float *)(a2 + 412) = v4;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseVariance_m  0x0049C280 ----  [HIGH] */
char __cdecl CPrimitiveTemplate__ParseVariance_m(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 424) = v3;
  *(float *)(a2 + 420) = v4;
  return 1;
}

/* ---- sub_49C2D0  0x0049C2D0 ----  VERIFIED */
char __cdecl sub_49C2D0(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[107] = v8[0];
  v4 = v9[1];
  a2[108] = v2;
  v5 = v8[2];
  a2[109] = v3;
  v6 = v9[2];
  a2[110] = v4;
  a2[112] = v6;
  a2[111] = v5;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseRGBEnd_m  0x0049C340 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParseRGBEnd_m(char *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  _DWORD v8[3]; // [esp+8h] [ebp-18h] BYREF
  _DWORD v9[3]; // [esp+14h] [ebp-Ch] BYREF

  if ( CPrimitiveTemplate__ParseVector(v9, v8, (int)a2, a1) != 1 )
    return 0;
  v2 = v9[0];
  v3 = v8[1];
  a2[113] = v8[0];
  v4 = v9[1];
  a2[114] = v2;
  v5 = v8[2];
  a2[115] = v3;
  v6 = v9[2];
  a2[116] = v4;
  a2[118] = v6;
  a2[117] = v5;
  return 1;
}

/* ---- sub_49C3B0  0x0049C3B0 ----  [HIGH] */
char __cdecl sub_49C3B0(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 480) = v3;
  *(float *)(a2 + 476) = v4;
  return 1;
}

/* ---- sub_49C400  0x0049C400 ----  VERIFIED */
char __cdecl sub_49C400(char *a1, int a2)
{
  int v3; // [esp+4h] [ebp-4h] BYREF

  if ( CPrimitiveTemplate__ParseGroupFlags(&v3, a2, a1) != 1 )
    return 0;
  *(_DWORD *)(a2 + 184) |= 16 * v3;
  return 1;
}

/* ---- sub_49C430  0x0049C430 ----  [HIGH] */
char __cdecl sub_49C430(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 488) = v3;
  *(float *)(a2 + 484) = v4;
  return 1;
}

/* ---- sub_49C480  0x0049C480 ----  [HIGH] */
char __cdecl sub_49C480(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 496) = v3;
  *(float *)(a2 + 492) = v4;
  return 1;
}

/* ---- sub_49C4D0  0x0049C4D0 ----  [HIGH] */
char __cdecl sub_49C4D0(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 504) = v3;
  *(float *)(a2 + 500) = v4;
  return 1;
}

/* ---- sub_49C520  0x0049C520 ----  VERIFIED */
char __cdecl sub_49C520(char *a1, int a2)
{
  int v3; // [esp+4h] [ebp-4h] BYREF

  if ( CPrimitiveTemplate__ParseGroupFlags(&v3, a2, a1) != 1 )
    return 0;
  *(_DWORD *)(a2 + 184) |= v3;
  return 1;
}

/* ---- sub_49C550  0x0049C550 ----  [HIGH] */
char __cdecl sub_49C550(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 512) = v3;
  *(float *)(a2 + 508) = v4;
  return 1;
}

/* ---- sub_49C5A0  0x0049C5A0 ----  [HIGH] */
char __cdecl sub_49C5A0(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 520) = v3;
  *(float *)(a2 + 516) = v4;
  return 1;
}

/* ---- sub_49C5F0  0x0049C5F0 ----  [HIGH] */
char __cdecl sub_49C5F0(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 528) = v3;
  *(float *)(a2 + 524) = v4;
  return 1;
}

/* ---- sub_49C640  0x0049C640 ----  VERIFIED */
char __cdecl sub_49C640(char *a1, int a2)
{
  int v3; // [esp+4h] [ebp-4h] BYREF

  if ( CPrimitiveTemplate__ParseGroupFlags(&v3, a2, a1) != 1 )
    return 0;
  *(_DWORD *)(a2 + 184) |= v3 << 8;
  return 1;
}

/* ---- sub_49C670  0x0049C670 ----  [HIGH] */
char __cdecl sub_49C670(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 536) = v3;
  *(float *)(a2 + 532) = v4;
  return 1;
}

/* ---- sub_49C6C0  0x0049C6C0 ----  [HIGH] */
char __cdecl sub_49C6C0(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 544) = v3;
  *(float *)(a2 + 540) = v4;
  return 1;
}

/* ---- sub_49C710  0x0049C710 ----  [HIGH] */
char __cdecl sub_49C710(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 552) = v3;
  *(float *)(a2 + 548) = v4;
  return 1;
}

/* ---- sub_49C760  0x0049C760 ----  VERIFIED */
char __cdecl sub_49C760(char *a1, int a2)
{
  int v3; // [esp+4h] [ebp-4h] BYREF

  if ( CPrimitiveTemplate__ParseGroupFlags(&v3, a2, a1) != 1 )
    return 0;
  *(_DWORD *)(a2 + 184) |= v3 << 16;
  return 1;
}

/* ---- sub_49C790  0x0049C790 ----  [HIGH] */
char __cdecl sub_49C790(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 560) = v3;
  *(float *)(a2 + 556) = v4;
  return 1;
}

/* ---- sub_49C7E0  0x0049C7E0 ----  [HIGH] */
char __cdecl sub_49C7E0(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 568) = v3;
  *(float *)(a2 + 564) = v4;
  return 1;
}

/* ---- sub_49C830  0x0049C830 ----  [HIGH] */
char __cdecl sub_49C830(const char *a1, int a2)
{
  int v2;
  double v3;
  float v4;
  float v6; // [esp+0h] [ebp-8h] BYREF
  float v7; // [esp+4h] [ebp-4h] BYREF

  v2 = sscanf(a1, "%f %f", &v6, &v7);
  if ( !v2 )
    return 0;
  if ( v2 == 1 )
    v3 = v6;
  else
    v3 = v7;
  v4 = v6;
  *(float *)(a2 + 576) = v3;
  *(float *)(a2 + 572) = v4;
  return 1;
}

/* ---- sub_49C880  0x0049C880 ----  VERIFIED */
char __cdecl sub_49C880(char *a1, int a2)
{
  int v3; // [esp+4h] [ebp-4h] BYREF

  if ( CPrimitiveTemplate__ParseGroupFlags(&v3, a2, a1) != 1 )
    return 0;
  *(_DWORD *)(a2 + 184) |= v3 << 12;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseShaders  0x0049C8B0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseShaders(int a1, int a2)
{
  _DWORD *v2;
  _DWORD *v4;
  _DWORD *v5;
  int v6;
  int v7;
  unsigned int v8;
  _DWORD *v9;
  int v11;
  int v12; // [esp+10h] [ebp-4h] BYREF

  v2 = *(_DWORD **)(a1 + 16);
  if ( !v2 )
  {
LABEL_15:
    SFxHelper__Print((int)&theFxHelper, "CPrimitiveTemplate::ParseShaders called with an empty list!\n");
    return 0;
  }
  if ( !v2[1] )
  {
    v11 = *v2;
    if ( v11 )
    {
      v12 = re_RegisterShader(v11, 9);
      sub_497590(&v12, (_DWORD *)(a2 + 104));
      return 1;
    }
    goto LABEL_15;
  }
  v4 = v2;
  v5 = (_DWORD *)(a2 + 104);
  do
  {
    v6 = re_RegisterShader(*v4, 9);
    v7 = v5[1];
    v12 = v6;
    if ( v7 )
      v8 = (v5[2] - v7) >> 2;
    else
      v8 = 0;
    if ( v7 && v8 < (v5[3] - v7) >> 2 )
    {
      v9 = (_DWORD *)v5[2];
      sub_499B40(v9, &v12, 1);
      v5[2] = v9 + 1;
    }
    else
    {
      sub_498690((int)v5, (_DWORD *)v5[2], 1u, &v12);
    }
    v4 = (_DWORD *)v4[1];
  }
  while ( v4 );
  return 1;
}

/* ---- CPrimitiveTemplate__ParseSounds  0x0049C9A0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseSounds(int a1, int a2)
{
  _DWORD *v2;
  _DWORD *v4;
  _DWORD *v5;
  int v6;
  int v7;
  unsigned int v8;
  _DWORD *v9;
  int v11;
  int v12; // [esp+10h] [ebp-4h] BYREF

  v2 = *(_DWORD **)(a1 + 16);
  if ( !v2 )
    goto LABEL_21;
  if ( v2[1] )
  {
    v4 = v2;
    v5 = (_DWORD *)(a2 + 104);
    do
    {
      /* 0x0049C9C6 `mov ecx, [ebx]` -- the list node's name, field +0. */
      v6 = ((_BYTE *)Com_PickSoundAlias(*(const char **)v4, 1) - (_BYTE *)snd_aliasTable[SND_LOCALE_INGAME] + 68) / 68;
      if ( v6 <= 0 || v6 > snd_aliasTableCount[SND_LOCALE_INGAME] )
        v6 = 0;
      v12 = v6;
      v7 = v5[1];
      if ( v7 )
        v8 = (v5[2] - v7) >> 2;
      else
        v8 = 0;
      if ( v7 && v8 < (v5[3] - v7) >> 2 )
      {
        v9 = (_DWORD *)v5[2];
        sub_499B40(v9, &v12, 1);
        v5[2] = v9 + 1;
      }
      else
      {
        sub_498690((int)v5, (_DWORD *)v5[2], 1u, &v12);
      }
      v4 = (_DWORD *)v4[1];
    }
    while ( v4 );
    return 1;
  }
  if ( !*v2 )
  {
LABEL_21:
    SFxHelper__Print((int)&theFxHelper, "CPrimitiveTemplate::ParseSounds called with an empty list!\n");
    return 0;
  }
  v11 = ((_BYTE *)Com_PickSoundAlias(*(const char **)v2, 1) - (_BYTE *)snd_aliasTable[SND_LOCALE_INGAME] + 68) / 68;
  if ( v11 <= 0 || v11 > snd_aliasTableCount[SND_LOCALE_INGAME] )
    v11 = 0;
  v12 = v11;
  sub_497590(&v12, (_DWORD *)(a2 + 104));
  return 1;
}

/* ---- CPrimitiveTemplate__ParseModels  0x0049CAE0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseModels(int a1, _DWORD *a2)
{
  int v2;
  int v3;
  char *v4;
  int v5;
  unsigned int v6;
  _DWORD *v7;
  char *v8;
  char *v10; // [esp+10h] [ebp-4h] BYREF

  v2 = *(_DWORD *)(a1 + 16);
  if ( v2 )
  {
    if ( *(_DWORD *)(v2 + 4) )
    {
      v3 = v2;
      do
      {
        v4 = CFxModel__Register(*(char **)v3);
        v5 = a2[27];
        v10 = v4;
        if ( v5 )
          v6 = (a2[28] - v5) >> 2;
        else
          v6 = 0;
        if ( v5 && v6 < (a2[29] - v5) >> 2 )
        {
          v7 = (_DWORD *)a2[28];
          sub_499B40(v7, &v10, 1);
          a2[28] = v7 + 1;
        }
        else
        {
          sub_498690((int)(a2 + 26), (_DWORD *)a2[28], 1u, &v10);
        }
        v3 = *(_DWORD *)(v3 + 4);
      }
      while ( v3 );
      goto LABEL_15;
    }
    v8 = *(char **)v2;
    if ( v8 )
    {
      v10 = CFxModel__Register(v8);
      sub_497590(&v10, a2 + 26);
LABEL_15:
      a2[46] |= 0x1000000;
      return 1;
    }
  }
  SFxHelper__Print((int)&theFxHelper, "CPrimitiveTemplate::ParseModels called with an empty list!\n");
  return 0;
}

/* ---- CPrimitiveTemplate__ParseImpactFxStrings  0x0049CBD0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseImpactFxStrings(int a1, _DWORD *a2)
{
  int v2;
  _DWORD *v3;
  int v4;
  _DWORD *v5;
  int v6;
  unsigned int v7;
  _DWORD *v8;
  char *v9;
  _DWORD *v10;
  char *v12;

  v2 = *(_DWORD *)(a1 + 16);
  v3 = a2;
  if ( !v2 )
    goto LABEL_19;
  if ( *(_DWORD *)(v2 + 4) )
  {
    v4 = v2;
    while ( 1 )
    {
      v5 = (_DWORD *)CFxScheduler__RegisterEffect(dword_14075A0, *(char **)v4, 0);
      if ( !v5 )
        goto LABEL_18;
      v6 = v3[31];
      a2 = v5;
      if ( v6 )
        v7 = (v3[32] - v6) >> 2;
      else
        v7 = 0;
      if ( v6 && v7 < (v3[33] - v6) >> 2 )
      {
        v8 = (_DWORD *)v3[32];
        sub_499B40(v8, &a2, 1);
        v3[32] = v8 + 1;
      }
      else
      {
        sub_498690((int)(v3 + 30), (_DWORD *)v3[32], 1u, &a2);
      }
      v4 = *(_DWORD *)(v4 + 4);
      if ( !v4 )
        goto LABEL_17;
    }
  }
  v9 = *(char **)v2;
  if ( !v9 )
  {
LABEL_19:
    v12 = "CPrimitiveTemplate::ParseImpactFxStrings called with an empty list!\n";
    goto LABEL_20;
  }
  v10 = (_DWORD *)CFxScheduler__RegisterEffect(dword_14075A0, v9, 0);
  if ( !v10 )
  {
LABEL_18:
    v12 = "FxTemplate: Impact effect file not found.\n";
LABEL_20:
    SFxHelper__Print((int)&theFxHelper, v12);
    return 0;
  }
  a2 = v10;
  sub_497590(&a2, v3 + 30);
LABEL_17:
  v3[46] |= 0x82000000;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseDeathFxStrings  0x0049CCE0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseDeathFxStrings(int a1, _DWORD *a2)
{
  int v2;
  _DWORD *v3;
  int v4;
  _DWORD *v5;
  int v6;
  unsigned int v7;
  _DWORD *v8;
  char *v9;
  _DWORD *v10;
  char *v12;

  v2 = *(_DWORD *)(a1 + 16);
  v3 = a2;
  if ( !v2 )
    goto LABEL_19;
  if ( *(_DWORD *)(v2 + 4) )
  {
    v4 = v2;
    while ( 1 )
    {
      v5 = (_DWORD *)CFxScheduler__RegisterEffect(dword_14075A0, *(char **)v4, 0);
      if ( !v5 )
        goto LABEL_18;
      v6 = v3[35];
      a2 = v5;
      if ( v6 )
        v7 = (v3[36] - v6) >> 2;
      else
        v7 = 0;
      if ( v6 && v7 < (v3[37] - v6) >> 2 )
      {
        v8 = (_DWORD *)v3[36];
        sub_499B40(v8, &a2, 1);
        v3[36] = v8 + 1;
      }
      else
      {
        sub_498690((int)(v3 + 34), (_DWORD *)v3[36], 1u, &a2);
      }
      v4 = *(_DWORD *)(v4 + 4);
      if ( !v4 )
        goto LABEL_17;
    }
  }
  v9 = *(char **)v2;
  if ( !v9 )
  {
LABEL_19:
    v12 = "CPrimitiveTemplate::ParseDeathFxStrings called with an empty list!\n";
    goto LABEL_20;
  }
  v10 = (_DWORD *)CFxScheduler__RegisterEffect(dword_14075A0, v9, 0);
  if ( !v10 )
  {
LABEL_18:
    v12 = "FxTemplate: Death effect file not found.\n";
LABEL_20:
    SFxHelper__Print((int)&theFxHelper, v12);
    return 0;
  }
  a2 = v10;
  sub_497590(&a2, v3 + 34);
LABEL_17:
  v3[46] |= 0x20000000u;
  return 1;
}

/* ---- CPrimitiveTemplate__ParseEmitterFxStrings  0x0049CE00 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseEmitterFxStrings(int a1, _DWORD *a2)
{
  int v2;
  _DWORD *v3;
  int v4;
  _DWORD *v5;
  int v6;
  unsigned int v7;
  _DWORD *v8;
  char *v9;
  _DWORD *v10;
  char *v12;

  v2 = *(_DWORD *)(a1 + 16);
  v3 = a2;
  if ( !v2 )
    goto LABEL_19;
  if ( *(_DWORD *)(v2 + 4) )
  {
    v4 = v2;
    while ( 1 )
    {
      v5 = (_DWORD *)CFxScheduler__RegisterEffect(dword_14075A0, *(char **)v4, 0);
      if ( !v5 )
        goto LABEL_18;
      v6 = v3[39];
      a2 = v5;
      if ( v6 )
        v7 = (v3[40] - v6) >> 2;
      else
        v7 = 0;
      if ( v6 && v7 < (v3[41] - v6) >> 2 )
      {
        v8 = (_DWORD *)v3[40];
        sub_499B40(v8, &a2, 1);
        v3[40] = v8 + 1;
      }
      else
      {
        sub_498690((int)(v3 + 38), (_DWORD *)v3[40], 1u, &a2);
      }
      v4 = *(_DWORD *)(v4 + 4);
      if ( !v4 )
        goto LABEL_17;
    }
  }
  v9 = *(char **)v2;
  if ( !v9 )
  {
LABEL_19:
    v12 = "CPrimitiveTemplate::ParseEmitterFxStrings called with an empty list!\n";
    goto LABEL_20;
  }
  v10 = (_DWORD *)CFxScheduler__RegisterEffect(dword_14075A0, v9, 0);
  if ( !v10 )
  {
LABEL_18:
    v12 = "FxTemplate: Emitter effect file not found.\n";
LABEL_20:
    SFxHelper__Print((int)&theFxHelper, v12);
    return 0;
  }
  a2 = v10;
  sub_497590(&a2, v3 + 38);
LABEL_17:
  v3[46] |= 0x10000000u;
  return 1;
}

/* ---- CPrimitiveTemplate__ParsePlayFxStrings  0x0049CF20 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParsePlayFxStrings(int a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  unsigned int v6;
  _DWORD *v7;
  char *v9;
  int v10;
  char *v11;
  int v12; // [esp+10h] [ebp-4h] BYREF

  v2 = *(_DWORD *)(a1 + 16);
  if ( !v2 )
    goto LABEL_18;
  if ( !*(_DWORD *)(v2 + 4) )
  {
    v9 = *(char **)v2;
    if ( v9 )
    {
      v10 = CFxScheduler__RegisterEffect(dword_14075A0, v9, 0);
      if ( v10 )
      {
        v12 = v10;
        sub_497590(&v12, a2 + 42);
        return 1;
      }
      goto LABEL_17;
    }
LABEL_18:
    v11 = "CPrimitiveTemplate::ParsePlayFxStrings called with an empty list!\n";
    goto LABEL_19;
  }
  v3 = v2;
  while ( 1 )
  {
    v4 = CFxScheduler__RegisterEffect(dword_14075A0, *(char **)v3, 0);
    if ( !v4 )
      break;
    v5 = a2[43];
    v12 = v4;
    if ( v5 )
      v6 = (a2[44] - v5) >> 2;
    else
      v6 = 0;
    if ( v5 && v6 < (a2[45] - v5) >> 2 )
    {
      v7 = (_DWORD *)a2[44];
      sub_499B40(v7, &v12, 1);
      a2[44] = v7 + 1;
    }
    else
    {
      sub_498690((int)(a2 + 42), (_DWORD *)a2[44], 1u, &v12);
    }
    v3 = *(_DWORD *)(v3 + 4);
    if ( !v3 )
      return 1;
  }
LABEL_17:
  v11 = "FxTemplate: Effect file not found.\n";
LABEL_19:
  SFxHelper__Print((int)&theFxHelper, v11);
  return 0;
}

/* ---- CPrimitiveTemplate__ParseRGB  0x0049D040 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseRGB(_DWORD *a1, int a2)
{
  int i;
  char **v4;
  const char *v5;
  char *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v15; // [esp+8h] [ebp-24h] BYREF
  int v16; // [esp+Ch] [ebp-20h] BYREF
  int v17; // [esp+10h] [ebp-1Ch] BYREF
  _DWORD v18[3]; // [esp+14h] [ebp-18h] BYREF
  _DWORD v19[3]; // [esp+20h] [ebp-Ch] BYREF

  for ( i = *(_DWORD *)(a2 + 16); i; i = *(_DWORD *)(i + 4) )
  {
    v4 = *(char ***)(i + 16);
    v5 = *(const char **)i;
    if ( v4 )
      v6 = *v4;
    else
      v6 = 0;
    if ( !_stricmp(v5, "start") )
    {
      if ( CPrimitiveTemplate__ParseVector(v19, v18, (int)a1, v6) == 1 )
      {
        v7 = v19[0];
        v8 = v18[1];
        a1[107] = v18[0];
        v9 = v19[1];
        a1[108] = v7;
        v10 = v18[2];
        a1[109] = v8;
        v11 = v19[2];
        a1[110] = v9;
        a1[111] = v10;
        a1[112] = v11;
      }
    }
    else if ( !_stricmp(v5, off_55A73C) )
    {
      CPrimitiveTemplate__ParseRGBEnd_m(v6, a1);
    }
    else if ( !_stricmp(v5, "parm") || !_stricmp(v5, "parms") )
    {
      v12 = sscanf(v6, "%f %f", &v15, &v16);
      if ( v12 )
      {
        if ( v12 == 1 )
          v16 = v15;
        v13 = v16;
        a1[119] = v15;
        a1[120] = v13;
      }
    }
    else if ( !_stricmp(v5, "flags") || !_stricmp(v5, "flag") )
    {
      if ( CPrimitiveTemplate__ParseGroupFlags(&v17, (int)a1, v6) == 1 )
        a1[46] |= 16 * v17;
    }
    else
    {
      SFxHelper__Print((int)&theFxHelper, "Unknown key parsing an RGB group: %s\n", v5);
    }
  }
  return 1;
}

/* ---- CPrimitiveTemplate__ParseAlpha  0x0049D1D0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseAlpha(int a1, _DWORD *a2)
{
  int v2;
  _DWORD *i;
  char **v4;
  const char *v5;
  char *v6;
  int v7;
  _DWORD *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  _DWORD *v14; // [esp+8h] [ebp-18h] BYREF
  int v15; // [esp+Ch] [ebp-14h] BYREF
  int v16; // [esp+10h] [ebp-10h] BYREF
  int v17; // [esp+14h] [ebp-Ch] BYREF
  int v18; // [esp+18h] [ebp-8h] BYREF
  int v19; // [esp+1Ch] [ebp-4h] BYREF

  v2 = *(_DWORD *)(a1 + 16);
  for ( i = a2; v2; v2 = *(_DWORD *)(v2 + 4) )
  {
    v4 = *(char ***)(v2 + 16);
    v5 = *(const char **)v2;
    if ( v4 )
      v6 = *v4;
    else
      v6 = 0;
    if ( !_stricmp(v5, "start") )
    {
      v7 = sscanf(v6, "%f %f", &a2, &v14);
      if ( v7 )
      {
        if ( v7 == 1 )
          v14 = a2;
        v8 = v14;
        i[121] = a2;
        i[122] = v8;
      }
    }
    else if ( !_stricmp(v5, off_55A73C) )
    {
      v9 = sscanf(v6, "%f %f", &v15, &v16);
      if ( v9 )
      {
        if ( v9 == 1 )
          v16 = v15;
        v10 = v16;
        i[123] = v15;
        i[124] = v10;
      }
    }
    else if ( !_stricmp(v5, "parm") || !_stricmp(v5, "parms") )
    {
      v11 = sscanf(v6, "%f %f", &v17, &v18);
      if ( v11 )
      {
        if ( v11 == 1 )
          v18 = v17;
        v12 = v18;
        i[125] = v17;
        i[126] = v12;
      }
    }
    else if ( !_stricmp(v5, "flags") || !_stricmp(v5, "flag") )
    {
      if ( CPrimitiveTemplate__ParseGroupFlags(&v19, (int)i, v6) == 1 )
        i[46] |= v19;
    }
    else
    {
      SFxHelper__Print((int)&theFxHelper, "Unknown key parsing an Alpha group: %s\n", v5);
    }
  }
  return 1;
}

/* ---- CPrimitiveTemplate__ParseSize  0x0049D380 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseSize(int a1, _DWORD *a2)
{
  int v2;
  _DWORD *i;
  char **v4;
  const char *v5;
  char *v6;
  int v7;
  _DWORD *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  _DWORD *v14; // [esp+8h] [ebp-18h] BYREF
  int v15; // [esp+Ch] [ebp-14h] BYREF
  int v16; // [esp+10h] [ebp-10h] BYREF
  int v17; // [esp+14h] [ebp-Ch] BYREF
  int v18; // [esp+18h] [ebp-8h] BYREF
  int v19; // [esp+1Ch] [ebp-4h] BYREF

  v2 = *(_DWORD *)(a1 + 16);
  for ( i = a2; v2; v2 = *(_DWORD *)(v2 + 4) )
  {
    v4 = *(char ***)(v2 + 16);
    v5 = *(const char **)v2;
    if ( v4 )
      v6 = *v4;
    else
      v6 = 0;
    if ( !_stricmp(v5, "start") )
    {
      v7 = sscanf(v6, "%f %f", &a2, &v14);
      if ( v7 )
      {
        if ( v7 == 1 )
          v14 = a2;
        v8 = v14;
        i[127] = a2;
        i[128] = v8;
      }
    }
    else if ( !_stricmp(v5, off_55A73C) )
    {
      v9 = sscanf(v6, "%f %f", &v15, &v16);
      if ( v9 )
      {
        if ( v9 == 1 )
          v16 = v15;
        v10 = v16;
        i[129] = v15;
        i[130] = v10;
      }
    }
    else if ( !_stricmp(v5, "parm") || !_stricmp(v5, "parms") )
    {
      v11 = sscanf(v6, "%f %f", &v17, &v18);
      if ( v11 )
      {
        if ( v11 == 1 )
          v18 = v17;
        v12 = v18;
        i[131] = v17;
        i[132] = v12;
      }
    }
    else if ( !_stricmp(v5, "flags") || !_stricmp(v5, "flag") )
    {
      if ( CPrimitiveTemplate__ParseGroupFlags(&v19, (int)i, v6) == 1 )
        i[46] |= v19 << 8;
    }
    else
    {
      SFxHelper__Print((int)&theFxHelper, "Unknown key parsing a Size group: %s\n", v5);
    }
  }
  return 1;
}

/* ---- CPrimitiveTemplate__ParseSize2  0x0049D530 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseSize2(int a1, _DWORD *a2)
{
  int v2;
  _DWORD *i;
  char **v4;
  const char *v5;
  char *v6;
  int v7;
  _DWORD *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  _DWORD *v14; // [esp+8h] [ebp-18h] BYREF
  int v15; // [esp+Ch] [ebp-14h] BYREF
  int v16; // [esp+10h] [ebp-10h] BYREF
  int v17; // [esp+14h] [ebp-Ch] BYREF
  int v18; // [esp+18h] [ebp-8h] BYREF
  int v19; // [esp+1Ch] [ebp-4h] BYREF

  v2 = *(_DWORD *)(a1 + 16);
  for ( i = a2; v2; v2 = *(_DWORD *)(v2 + 4) )
  {
    v4 = *(char ***)(v2 + 16);
    v5 = *(const char **)v2;
    if ( v4 )
      v6 = *v4;
    else
      v6 = 0;
    if ( !_stricmp(v5, "start") )
    {
      v7 = sscanf(v6, "%f %f", &a2, &v14);
      if ( v7 )
      {
        if ( v7 == 1 )
          v14 = a2;
        v8 = v14;
        i[133] = a2;
        i[134] = v8;
      }
    }
    else if ( !_stricmp(v5, off_55A73C) )
    {
      v9 = sscanf(v6, "%f %f", &v15, &v16);
      if ( v9 )
      {
        if ( v9 == 1 )
          v16 = v15;
        v10 = v16;
        i[135] = v15;
        i[136] = v10;
      }
    }
    else if ( !_stricmp(v5, "parm") || !_stricmp(v5, "parms") )
    {
      v11 = sscanf(v6, "%f %f", &v17, &v18);
      if ( v11 )
      {
        if ( v11 == 1 )
          v18 = v17;
        v12 = v18;
        i[137] = v17;
        i[138] = v12;
      }
    }
    else if ( !_stricmp(v5, "flags") || !_stricmp(v5, "flag") )
    {
      if ( CPrimitiveTemplate__ParseGroupFlags(&v19, (int)i, v6) == 1 )
        i[46] |= v19 << 16;
    }
    else
    {
      SFxHelper__Print((int)&theFxHelper, "Unknown key parsing a Size2 group: %s\n", v5);
    }
  }
  return 1;
}

/* ---- CPrimitiveTemplate__ParseLength  0x0049D6E0 ----  [CONFIRMED] */
char __cdecl CPrimitiveTemplate__ParseLength(int a1, _DWORD *a2)
{
  int v2;
  _DWORD *i;
  char **v4;
  const char *v5;
  char *v6;
  int v7;
  _DWORD *v8;
  int v9;
  int v10;
  int v11;
  int v12;
  _DWORD *v14; // [esp+8h] [ebp-18h] BYREF
  int v15; // [esp+Ch] [ebp-14h] BYREF
  int v16; // [esp+10h] [ebp-10h] BYREF
  int v17; // [esp+14h] [ebp-Ch] BYREF
  int v18; // [esp+18h] [ebp-8h] BYREF
  int v19; // [esp+1Ch] [ebp-4h] BYREF

  v2 = *(_DWORD *)(a1 + 16);
  for ( i = a2; v2; v2 = *(_DWORD *)(v2 + 4) )
  {
    v4 = *(char ***)(v2 + 16);
    v5 = *(const char **)v2;
    if ( v4 )
      v6 = *v4;
    else
      v6 = 0;
    if ( !_stricmp(v5, "start") )
    {
      v7 = sscanf(v6, "%f %f", &a2, &v14);
      if ( v7 )
      {
        if ( v7 == 1 )
          v14 = a2;
        v8 = v14;
        i[139] = a2;
        i[140] = v8;
      }
    }
    else if ( !_stricmp(v5, off_55A73C) )
    {
      v9 = sscanf(v6, "%f %f", &v15, &v16);
      if ( v9 )
      {
        if ( v9 == 1 )
          v16 = v15;
        v10 = v16;
        i[141] = v15;
        i[142] = v10;
      }
    }
    else if ( !_stricmp(v5, "parm") || !_stricmp(v5, "parms") )
    {
      v11 = sscanf(v6, "%f %f", &v17, &v18);
      if ( v11 )
      {
        if ( v11 == 1 )
          v18 = v17;
        v12 = v18;
        i[143] = v17;
        i[144] = v12;
      }
    }
    else if ( !_stricmp(v5, "flags") || !_stricmp(v5, "flag") )
    {
      if ( CPrimitiveTemplate__ParseGroupFlags(&v19, (int)i, v6) == 1 )
        i[46] |= v19 << 12;
    }
    else
    {
      SFxHelper__Print((int)&theFxHelper, "Unknown key parsing a Length group: %s\n", v5);
    }
  }
  return 1;
}

/* ---- sub_49D890  0x0049D890 ----  VERIFIED */
char __cdecl sub_49D890(char *a1, int a2)
{
  int v2;
  char v3;

  v2 = a2 + 40 - (_DWORD)a1;
  do
  {
    v3 = *a1;
    a1[v2] = *a1;
    ++a1;
  }
  while ( v3 );
  return 1;
}

/* ---- CPrimitiveTemplate__ParsePrimitive  0x0049D8B0 ----  VERIFIED */
char __cdecl CPrimitiveTemplate__ParsePrimitive( int a1, int a2 )
{
  int         pair;
  int         group;
  const char *key;
  char       *val;
  int         n;
  char       *d;
  int         lo;
  int         hi;

  for ( pair = *(_DWORD *)( a2 + 16 ); pair; pair = *(_DWORD *)( pair + 4 ) )
  {
    key = *(const char **)pair;
    if ( *(_DWORD *)( pair + 16 ) )
      val = **(char ***)( pair + 16 );
    else
      val = 0;

    if ( !_stricmp( key, "count" ) )
    {
      n = sscanf( val, "%f %f", &lo, &hi );
      if ( n )
      {
        if ( n == 1 )
          hi = lo;
        *(_DWORD *)( a1 + 84 ) = lo;
        *(_DWORD *)( a1 + 88 ) = hi;
      }
    }
    else if ( !_stricmp( key, "shaders" ) || !_stricmp( key, "shader" ) )
      CPrimitiveTemplate__ParseShaders( pair, a1 );
    else if ( !_stricmp( key, "models" ) || !_stricmp( key, "model" ) )
      CPrimitiveTemplate__ParseModels( pair, (_DWORD *)a1 );
    else if ( !_stricmp( key, "sounds" ) || !_stricmp( key, "sound" ) )
      CPrimitiveTemplate__ParseSounds( pair, a1 );
    else if ( !_stricmp( key, "impactfx" ) )
      CPrimitiveTemplate__ParseImpactFxStrings( pair, (_DWORD *)a1 );
    else if ( !_stricmp( key, "deathfx" ) )
      CPrimitiveTemplate__ParseDeathFxStrings( pair, (_DWORD *)a1 );
    else if ( !_stricmp( key, "emitfx" ) )
      CPrimitiveTemplate__ParseEmitterFxStrings( pair, (_DWORD *)a1 );
    else if ( !_stricmp( key, "playfx" ) )
      CPrimitiveTemplate__ParsePlayFxStrings( pair, (_DWORD *)a1 );
    else if ( !_stricmp( key, "life" ) )
      CPrimitiveTemplate__ParseLife_m( val, a1 );
    else if ( !_stricmp( key, "cullrange" ) )
      *(_DWORD *)( a1 + 100 ) = j__atol( val );
    else if ( !_stricmp( key, "delay" ) )
      CPrimitiveTemplate__ParseDelay_m( val, a1 );
    else if ( !_stricmp( key, "bounce" ) || !_stricmp( key, "intensity" ) )
      sub_49B710( val, a1 );
    else if ( !_stricmp( key, "min" ) )
      CPrimitiveTemplate__ParseMin_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "max" ) )
      CPrimitiveTemplate__ParseMax_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "angle" ) || !_stricmp( key, "angles" ) )
      CPrimitiveTemplate__ParseAngle_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "angleDelta" ) )
      CPrimitiveTemplate__ParseAngleDelta_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "velocity" ) || !_stricmp( key, "vel" ) )
      CPrimitiveTemplate__ParseVelocity_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "acceleration" ) || !_stricmp( key, "accel" ) )
      CPrimitiveTemplate__ParseAcceleration_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "gravity" ) )
      CPrimitiveTemplate__ParseGravity_m( val, a1 );
    else if ( !_stricmp( key, "density" ) )
      CPrimitiveTemplate__ParseDensity_m( val, a1 );
    else if ( !_stricmp( key, "variance" ) )
      CPrimitiveTemplate__ParseVariance_m( val, a1 );
    else if ( !_stricmp( key, "origin" ) )
      CPrimitiveTemplate__ParseOrigin_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "origin2" ) )
      CPrimitiveTemplate__ParseOrigin2_m( val, (_DWORD *)a1 );
    else if ( !_stricmp( key, "radius" ) )
      CPrimitiveTemplate__ParseRadius_m( val, a1 );
    else if ( !_stricmp( key, "height" ) )
      CPrimitiveTemplate__ParseHeight_m( val, a1 );
    else if ( !_stricmp( key, "wind" ) )
      CPrimitiveTemplate__ParseWind_m( val, a1 );
    else if ( !_stricmp( key, "rotation" ) )
      CPrimitiveTemplate__ParseRotation_m( val, a1 );
    else if ( !Q_stricmp( "rotationDelta", key ) )
      sub_49B990( val, a1 );
    else if ( !_stricmp( key, "flags" ) || !_stricmp( key, "flag" ) )
      CPrimitiveTemplate__ParseFlags( a1, val );
    else if ( !_stricmp( key, "spawnFlags" ) || !_stricmp( key, "spawnFlag" ) )
      CPrimitiveTemplate__ParseSpawnFlags( a1, val );
    else if ( !_stricmp( key, "nonUniformScale" ) )
      *(_BYTE *)( a1 + 192 ) = j__atol( val ) != 0;
    else if ( !_stricmp( key, "name" ) )
    {
      if ( val )
      {
        d = (char *)( a1 + 8 );
        while ( ( *d++ = *val++ ) != 0 )
          ;
      }
    }
    else if ( !_stricmp( key, "materialImpact" ) )
    {
      d = (char *)( a1 + 40 );
      while ( ( *d++ = *val++ ) != 0 )
        ;
    }
    else
    {
      SFxHelper__Print( (int)&theFxHelper,
                        "Unknown key parsing an effect primitive: %s\n", key );
    }
  }

  for ( group = *(_DWORD *)( a2 + 28 ); group; group = *(_DWORD *)( group + 4 ) )
  {
    key = *(const char **)group;
    if ( !_stricmp( key, "rgb" ) )
      CPrimitiveTemplate__ParseRGB( (_DWORD *)a1, group );
    else if ( !_stricmp( key, "alpha" ) )
      CPrimitiveTemplate__ParseAlpha( group, (_DWORD *)a1 );
    else if ( !_stricmp( key, "size" ) || !_stricmp( key, "width" ) )
      CPrimitiveTemplate__ParseSize( group, (_DWORD *)a1 );
    else if ( !_stricmp( key, "size2" ) || !_stricmp( key, "width2" ) )
      CPrimitiveTemplate__ParseSize2( group, (_DWORD *)a1 );
    else if ( !_stricmp( key, "length" ) || !_stricmp( key, "height" ) )
      CPrimitiveTemplate__ParseLength( group, (_DWORD *)a1 );
    else
    {
      SFxHelper__Print( (int)&theFxHelper,
                        "Unknown group key parsing a particle: %s\n", key );
    }
  }
  return 1;
}

/* ---- sub_49DF30  0x0049DF30 ----  VERIFIED */
_DWORD *__cdecl sub_49DF30(_DWORD *result)
{
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;
  return result;
}

/* ---- sub_49DF40  0x0049DF40 ----  VERIFIED */
char __cdecl sub_49DF40(unsigned int a1, _DWORD *a2)
{
  a2[1] = 0;
  a2[2] = 0;
  a2[3] = 0;
  if ( !a1 )
    return 0;
  if ( a1 > 0x3FFFFFFF )
    sub_499080();
  return sub_49DF62(a1, a2);
}

/* ---- sub_49DF5F  0x0049DF5F ----  [HIGH] */
char sub_49DF5F()
{
  return 1;
}

/* ---- sub_49DF62  AUTO-STUBBED ----  [UNKNOWN]  AUTO-STUBBED */
int sub_49DF62() { return 0; }
#if 0
char __cdecl sub_49DF62(int a1, _DWORD *a2)
{
  int v2;
  char *v3;

  v2 = 4 * a1;
  v3 = (char *)operator new(4 * a1);
  a2[1] = v3;
  a2[2] = v3;
  a2[3] = &v3[v2];
  return 1;
}
#endif

/* ---- nullsub_65  0x0049DF90 ----  [HIGH] */
void __stdcall nullsub_65(int a1)
{
  ;
}

/* ---- nullsub_66  0x0049DFA0 ----  [HIGH] */
void nullsub_66()
{
  ;
}

/* ---- nullsub_67  0x0049DFB0 ----  [HIGH] */
void __stdcall nullsub_67(int a1)
{
  ;
}

/* ---- sub_49DFC0  0x0049DFC0 ----  [HIGH] */
int __cdecl sub_49DFC0(int result, int a2, int a3)
{
  *(_DWORD *)(result + 172) = a2;
  *(_DWORD *)(result + 176) = a3;
  return result;
}

/* ---- sub_49DFE0  0x0049DFE0 ----  VERIFIED */
_DWORD *__cdecl sub_49DFE0(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[7] = *a2;
    result[8] = a2[1];
    result[9] = a2[2];
  }
  else
  {
    result[9] = 0;
    result[8] = 0;
    result[7] = 0;
  }
  return result;
}

/* ---- sub_49E010  0x0049E010 ----  VERIFIED */
_DWORD *__cdecl sub_49E010(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[10] = *a2;
    result[11] = a2[1];
    result[12] = a2[2];
  }
  else
  {
    result[12] = 0;
    result[11] = 0;
    result[10] = 0;
  }
  return result;
}

/* ---- sub_49E040  0x0049E040 ----  [HIGH] */
int __cdecl sub_49E040(int result, int a2)
{
  *(_DWORD *)(result + 24) = a2;
  return result;
}

/* ---- sub_49E050  0x0049E050 ----  [HIGH] */
int __cdecl sub_49E050(int a1, int a2)
{
  int result;

  result = ~a1;
  *(_DWORD *)(a2 + 24) &= result;
  return result;
}

/* ---- sub_49E060  0x0049E060 ----  VERIFIED */
_DWORD *__cdecl sub_49E060(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[1] = *a2;
    result[2] = a2[1];
    result[3] = a2[2];
  }
  else
  {
    result[3] = 0;
    result[2] = 0;
    result[1] = 0;
  }
  return result;
}

/* ---- sub_49E090  0x0049E090 ----  [HIGH] */
int __cdecl sub_49E090(int result, int a2)
{
  *(_DWORD *)(result + 20) = a2;
  return result;
}

/* ---- sub_49E0A0  0x0049E0A0 ----  [HIGH] */
int __cdecl sub_49E0A0(int result, int a2)
{
  *(_DWORD *)(result + 52) = a2;
  return result;
}

/* ---- sub_49E0B0  0x0049E0B0 ----  [HIGH] */
int __cdecl sub_49E0B0(int result, int a2)
{
  *(_DWORD *)(result + 56) = a2;
  return result;
}

/* ---- cand_CFxPrimType14__ctor  0x0049E0C0 ----  VERIFIED */
_DWORD *__cdecl cand_CFxPrimType14__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_559450;
  return result;
}

/* ---- cand_CFxPrimType14__dtor  0x0049E0E0 ----  VERIFIED */
void __cdecl cand_CFxPrimType14__dtor(_DWORD *this)
{
  char *v1;

  *this = &off_559F80;
  v1 = (char *)this[54];
  if ( v1 )
    CFxBoltFrame__Release(v1);
}

/* ---- sub_49E100  0x0049E100 ----  VERIFIED */
int *__cdecl sub_49E100(int a1)
{
  return FxPool_AllocPrimType14(&dword_140CA10, a1);
}

/* ---- sub_49E110  0x0049E110 ----  VERIFIED */
_DWORD *__cdecl sub_49E110(char *this)
{
  return FxPool_FreePrimType14(this, &dword_140CA10);
}

/* ---- cand_CFxPrimType14__scalar_dtor  0x0049E120 ----  VERIFIED */
char *__cdecl cand_CFxPrimType14__scalar_dtor(char *this, char a2)
{
  cand_CFxPrimType14__dtor(this);
  if ( (a2 & 1) != 0 )
    FxPool_FreePrimType14(this, &dword_140CA10);
  return this;
}

/* ---- sub_49E150  0x0049E150 ----  [HIGH] */
int __cdecl sub_49E150(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[55] = a2;
  return result;
}

/* ---- sub_49E160  0x0049E160 ----  [HIGH] */
int __cdecl sub_49E160(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[56] = a2;
  return result;
}

/* ---- sub_49E170  0x0049E170 ----  [HIGH] */
int __cdecl sub_49E170(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[57] = a2;
  return result;
}

/* ---- sub_49E180  0x0049E180 ----  VERIFIED */
_DWORD *__cdecl sub_49E180(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[58] = *a2;
    result[59] = a2[1];
    result[60] = a2[2];
  }
  else
  {
    result[60] = 0;
    result[59] = 0;
    result[58] = 0;
  }
  return result;
}

/* ---- sub_49E1C0  0x0049E1C0 ----  VERIFIED */
_DWORD *__cdecl sub_49E1C0(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[61] = *a2;
    result[62] = a2[1];
    result[63] = a2[2];
  }
  else
  {
    result[63] = 0;
    result[62] = 0;
    result[61] = 0;
  }
  return result;
}

/* ---- sub_49E200  0x0049E200 ----  [HIGH] */
int __cdecl sub_49E200(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[64] = a2;
  return result;
}

/* ---- sub_49E210  0x0049E210 ----  VERIFIED */
int *__cdecl sub_49E210(int a1)
{
  return FxPool_AllocLight(&dword_140C9E8, a1);
}

/* ---- cand_CFxPrimType13__ctor  0x0049E220 ----  VERIFIED */
_DWORD *__cdecl cand_CFxPrimType13__ctor(_DWORD *result)
{
  result[54] = 0;
  *result = &off_55946C;
  return result;
}

/* ---- cand_CFxPrimType13__dtor  0x0049E240 ----  VERIFIED */
void __cdecl cand_CFxPrimType13__dtor(_DWORD *this)
{
  char *v1;

  *this = &off_559F80;
  v1 = (char *)this[54];
  if ( v1 )
    CFxBoltFrame__Release(v1);
}

/* ---- cand_CFxPrimType13__Cull  0x0049E260 ----  [HIGH] */
char cand_CFxPrimType13__Cull()
{
  return 0;
}

/* ---- sub_49E270  0x0049E270 ----  [HIGH] */
int __cdecl sub_49E270(int result, int a2)
{
  *(_DWORD *)(result + 164) = a2;
  return result;
}

/* ---- cand_CFxPrimType13__scalar_dtor  0x0049E280 ----  VERIFIED */
char *__cdecl cand_CFxPrimType13__scalar_dtor(char *this, char a2)
{
  cand_CFxPrimType13__dtor(this);
  if ( (a2 & 1) != 0 )
    sub_491AA0(this, &dword_140C9E8);
  return this;
}

/* ---- sub_49E2B0  0x0049E2B0 ----  [HIGH] */
int __cdecl sub_49E2B0(int result, int a2)
{
  *(_DWORD *)(result + 164) = a2;
  return result;
}

/* ---- sub_49E2C0  0x0049E2C0 ----  VERIFIED */
_DWORD *__cdecl sub_49E2C0(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[55] = *a2;
    result[56] = a2[1];
    result[57] = a2[2];
  }
  else
  {
    result[57] = 0;
    result[56] = 0;
    result[55] = 0;
  }
  return result;
}

/* ---- sub_49E300  0x0049E300 ----  VERIFIED */
_DWORD *__cdecl sub_49E300(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[58] = *a2;
    result[59] = a2[1];
    result[60] = a2[2];
  }
  else
  {
    result[60] = 0;
    result[59] = 0;
    result[58] = 0;
  }
  return result;
}

/* ---- sub_49E340  0x0049E340 ----  [HIGH] */
int __cdecl sub_49E340(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[61] = a2;
  return result;
}

/* ---- sub_49E350  0x0049E350 ----  [HIGH] */
int __cdecl sub_49E350(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[62] = a2;
  return result;
}

/* ---- sub_49E360  0x0049E360 ----  [HIGH] */
int __cdecl sub_49E360(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[63] = a2;
  return result;
}

/* ---- sub_49E370  0x0049E370 ----  [HIGH] */
int __cdecl sub_49E370(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[64] = a2;
  return result;
}

/* ---- sub_49E380  0x0049E380 ----  [HIGH] */
int __cdecl sub_49E380(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[65] = a2;
  return result;
}

/* ---- sub_49E390  0x0049E390 ----  [HIGH] */
int __cdecl sub_49E390(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[66] = a2;
  return result;
}

/* ---- sub_49E3A0  0x0049E3A0 ----  VERIFIED */
_DWORD *__cdecl sub_49E3A0(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[67] = *a2;
    result[68] = a2[1];
    result[69] = a2[2];
  }
  else
  {
    result[69] = 0;
    result[68] = 0;
    result[67] = 0;
  }
  return result;
}

/* ---- sub_49E3E0  0x0049E3E0 ----  VERIFIED */
_DWORD *__cdecl sub_49E3E0(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[70] = *a2;
    result[71] = a2[1];
    result[72] = a2[2];
  }
  else
  {
    result[72] = 0;
    result[71] = 0;
    result[70] = 0;
  }
  return result;
}

/* ---- sub_49E420  0x0049E420 ----  [HIGH] */
int __cdecl sub_49E420(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[73] = a2;
  return result;
}

/* ---- sub_49E430  0x0049E430 ----  [HIGH] */
int __cdecl sub_49E430(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[74] = a2;
  return result;
}

/* ---- sub_49E440  0x0049E440 ----  [HIGH] */
int __cdecl sub_49E440(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[75] = a2;
  return result;
}

/* ---- sub_49E450  0x0049E450 ----  [HIGH] */
int __cdecl sub_49E450(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[76] = a2;
  return result;
}

/* ---- sub_49E460  0x0049E460 ----  [HIGH] */
int __cdecl sub_49E460(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[47] = a2;
  return result;
}

/* ---- sub_49E470  0x0049E470 ----  [HIGH] */
int __cdecl sub_49E470(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[77] = a2;
  return result;
}

/* ---- sub_49E480  0x0049E480 ----  [HIGH] */
int __cdecl sub_49E480(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[78] = a2;
  return result;
}

/* ---- sub_49E490  0x0049E490 ----  VERIFIED */
int *__cdecl sub_49E490(int a1)
{
  return FxPool_AllocParticle(&dword_140C9D0, a1);
}

/* ---- sub_49E4A0  0x0049E4A0 ----  VERIFIED */
int __cdecl sub_49E4A0(_DWORD *a1, _DWORD *a2)
{
  int result;

  a2[79] = *a1;
  a2[80] = a1[1];
  result = a1[2];
  a2[81] = result;
  return result;
}

/* ---- sub_49E4C0  0x0049E4C0 ----  VERIFIED */
int *__cdecl sub_49E4C0(int a1)
{
  return FxPool_AllocLine(&dword_140CA18, a1);
}

/* ---- sub_49E4D0  0x0049E4D0 ----  [HIGH] */
int __cdecl sub_49E4D0(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[82] = a2;
  return result;
}

/* ---- sub_49E4E0  0x0049E4E0 ----  VERIFIED */
int *__cdecl sub_49E4E0(int a1)
{
  return FxPool_AllocElectricity(&dword_140C9C0, a1);
}

/* ---- sub_49E4F0  0x0049E4F0 ----  VERIFIED */
int __cdecl sub_49E4F0(_DWORD *a1, _DWORD *a2)
{
  int result;

  a2[79] = *a1;
  a2[80] = a1[1];
  result = a1[2];
  a2[81] = result;
  return result;
}

/* ---- sub_49E510  0x0049E510 ----  VERIFIED */
int *__cdecl sub_49E510(int a1)
{
  return FxPool_AllocOrientedParticle(&dword_140C9F8, a1);
}

/* ---- sub_49E520  0x0049E520 ----  [HIGH] */
int __cdecl sub_49E520(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[82] = a2;
  return result;
}

/* ---- sub_49E530  0x0049E530 ----  [HIGH] */
int __cdecl sub_49E530(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[83] = a2;
  return result;
}

/* ---- sub_49E540  0x0049E540 ----  [HIGH] */
int __cdecl sub_49E540(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[84] = a2;
  return result;
}

/* ---- sub_49E550  0x0049E550 ----  VERIFIED */
int *__cdecl sub_49E550(int a1)
{
  return FxPool_AllocTail(&dword_140CA00, a1);
}

/* ---- sub_49E560  0x0049E560 ----  [HIGH] */
int __cdecl sub_49E560(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[64] = a2;
  return result;
}

/* ---- sub_49E570  0x0049E570 ----  [HIGH] */
int __cdecl sub_49E570(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[65] = a2;
  return result;
}

/* ---- sub_49E580  0x0049E580 ----  [HIGH] */
int __cdecl sub_49E580(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[66] = a2;
  return result;
}

/* ---- sub_49E590  0x0049E590 ----  VERIFIED */
int __cdecl sub_49E590(_DWORD *a1, _DWORD *a2)
{
  int result;

  a2[22] = *a1;
  a2[23] = a1[1];
  result = a1[2];
  a2[24] = result;
  return result;
}

/* ---- sub_49E5B0  0x0049E5B0 ----  VERIFIED */
int *__cdecl sub_49E5B0(int a1)
{
  return FxPool_AllocCylinder(&dword_140C9D8, a1);
}

/* ---- sub_49E5C0  0x0049E5C0 ----  [HIGH] */
int __cdecl sub_49E5C0(int result, int a2)
{
  *(_DWORD *)(result + 204) = a2;
  return result;
}

/* ---- sub_49E5D0  0x0049E5D0 ----  VERIFIED */
_DWORD *__cdecl sub_49E5D0(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[90] = *a2;
    result[91] = a2[1];
    result[92] = a2[2];
  }
  else
  {
    result[92] = 0;
    result[91] = 0;
    result[90] = 0;
  }
  return result;
}

/* ---- sub_49E610  0x0049E610 ----  VERIFIED */
_DWORD *__cdecl sub_49E610(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[93] = *a2;
    result[94] = a2[1];
    result[95] = a2[2];
  }
  else
  {
    result[95] = 0;
    result[94] = 0;
    result[93] = 0;
  }
  return result;
}

/* ---- sub_49E650  0x0049E650 ----  [HIGH] */
int __cdecl sub_49E650(int result, int a2)
{
  *(_DWORD *)(result + 384) = a2;
  return result;
}

/* ---- sub_49E660  0x0049E660 ----  [HIGH] */
int __cdecl sub_49E660(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[97] = a2;
  return result;
}

/* ---- sub_49E670  0x0049E670 ----  [HIGH] */
int __cdecl sub_49E670(_DWORD *this, int a2)
{
  int result;

  result = a2;
  this[98] = a2;
  return result;
}

/* ---- sub_49E680  0x0049E680 ----  [HIGH] */
int __cdecl sub_49E680(int result, int a2)
{
  *(_DWORD *)(result + 352) = a2;
  return result;
}

/* ---- sub_49E690  0x0049E690 ----  VERIFIED */
_DWORD *__cdecl sub_49E690(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[79] = *a2;
    result[80] = a2[1];
    result[81] = a2[2];
  }
  else
  {
    result[81] = 0;
    result[80] = 0;
    result[79] = 0;
  }
  return result;
}

/* ---- sub_49E6D0  0x0049E6D0 ----  VERIFIED */
_DWORD *__cdecl sub_49E6D0(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[82] = *a2;
    result[83] = a2[1];
    result[84] = a2[2];
  }
  else
  {
    result[84] = 0;
    result[83] = 0;
    result[82] = 0;
  }
  return result;
}

/* ---- sub_49E710  0x0049E710 ----  VERIFIED */
_DWORD *__cdecl sub_49E710(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[85] = *a2;
    result[86] = a2[1];
    result[87] = a2[2];
  }
  else
  {
    result[87] = 0;
    result[86] = 0;
    result[85] = 0;
  }
  return result;
}

/* ---- sub_49E750  0x0049E750 ----  VERIFIED */
_DWORD *__cdecl sub_49E750(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[18] = *a2;
    result[19] = a2[1];
    result[20] = a2[2];
    result[16] |= 0x80u;
  }
  return result;
}

/* ---- sub_49E770  0x0049E770 ----  VERIFIED */
int *__cdecl sub_49E770(int a1)
{
  return FxPool_AllocEmitter(&dword_140C9B8, a1);
}

/* ---- sub_49E780  0x0049E780 ----  [HIGH] */
int __cdecl sub_49E780(int result, int a2)
{
  *(_DWORD *)(result + 316) = a2;
  return result;
}

/* ---- sub_49E790  0x0049E790 ----  VERIFIED */
_DWORD *__cdecl sub_49E790(_DWORD *result, _DWORD *a2)
{
  if ( a2 )
  {
    result[80] = *a2;
    result[81] = a2[1];
    result[82] = a2[2];
  }
  else
  {
    result[82] = 0;
    result[81] = 0;
    result[80] = 0;
  }
  return result;
}

/* ---- sub_49E7D0  0x0049E7D0 ----  [HIGH] */
int __cdecl sub_49E7D0(int result, int a2)
{
  *(_DWORD *)(a2 + 332) = result + fxMTime;
  return result;
}

/* ---- sub_49E7E0  0x0049E7E0 ----  VERIFIED */
int *__cdecl sub_49E7E0(int a1)
{
  return FxPool_AllocFlash(&dword_140C9F0, a1);
}

/* ---- sub_49E7F0  0x0049E7F0 ----  [HIGH] */
int __cdecl sub_49E7F0(int a1)
{
  return *(_DWORD *)(a1 + 8);
}

/* ---- sub_49E800  0x0049E800 ----  [HIGH] */
int __cdecl sub_49E800(int a1)
{
  return *(_DWORD *)(a1 + 21524);
}

/* ---- FX_Free  0x0049E810 ----  [CONFIRMED] */
char __cdecl FX_Free(int a1)
{
  _DWORD *v1;

  if ( fxActiveEffects )
    (**(void (__cdecl ***)(int, int))fxActiveEffects)(fxActiveEffects, 1);
  fxActiveEffects = 0;
  fxFreeListHead = (int)&fxActiveEffects;
  v1 = &unk_C9CE6C;
  do
  {
    if ( *v1 )
      (**(void (__cdecl ***)(_DWORD, int))*v1)(*v1, 1);
    *v1 = 0;
    *(v1 - 1) = v1;
    v1 += 3;
  }
  while ( (int)v1 < (int)byte_CA22C0 );
  dword_CA22BC = 0;
  *(_DWORD *)fxActiveCount = 0;
  CFxModel__Clean();
  CFxScheduler__Clean((int)dword_14075A0, a1, 0);
  return 1;
}

/* ---- sub_49E890  0x0049E890 ----  VERIFIED */
void sub_49E890()
{
  int *v0;

  v0 = &fxActiveEffects;
  do
  {
    if ( *v0 )
      (**(void (__cdecl ***)(int, int))*v0)(*v0, 1);
    *v0 = 0;
    v0 += 3;
  }
  while ( (int)v0 < (int)byte_CA22C0 );
  *(_DWORD *)fxActiveCount = 0;
  CFxScheduler__Clean((int)dword_14075A0, 0, 0);
}

/* ---- FX_GetBoneOrientation  0x0049E8D0 ----  VERIFIED */
char __cdecl FX_GetBoneOrientation(float *a1, _DWORD *a2)
{
  int v4;
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
  signed __int16 v17;
  DObj *v18;
  float *evalStorage;
  float *v20;
  double v21;
  double v22;
  cvar_t *v23;
  float v24;
  float v25[3];  /* [esp+1Ch] [ebp-30h] BYREF -- retail vec3_t origin */
  float v28[9];  /* [esp+28h] [ebp-24h] BYREF -- retail 3x3 axis */
  float fxDebugScale;
  float fxDebugEnd[3];

  VM_Call(cgvm, 12, *a2, v25, v28);
  v4 = a2[1];
  if ( v4 >= 0 )
  {
    v17 = (short)com_clientDObjHandles[*a2];
    /* retail 0x0049E96A-0x0049E96D `imul esi, 58h` then `add esi, offset
     * dobj_pool`.  dobj_pool is com_dobjPool, stride 0x58. */
    if ( v17 && (v18 = (DObj *)&com_dobjPool[v17 * 0x58]) != 0 && v4 < v18->partCount )
    {
      VM_Call(cgvm, 11, *a2, v4);
      evalStorage = (float *)v18->evalStorage;
      v20 = &evalStorage[16 * a2[1] + 12 + 16 * v18->childPartBaseIndices[0]];
      a1[3] = v28[6] * evalStorage[16 * a2[1] + 14 + 16 * v18->childPartBaseIndices[0]]
            + v28[3] * evalStorage[16 * a2[1] + 13 + 16 * v18->childPartBaseIndices[0]]
            + v28[0] * *v20;
      a1[4] = v28[7] * v20[2] + v28[4] * v20[1] + v28[1] * *v20;
      a1[5] = v28[8] * v20[2] + v28[5] * v20[1] + v28[2] * *v20;
      a1[6] = v28[0] * v20[4] + v28[6] * v20[6] + v28[3] * v20[5];
      a1[7] = v28[1] * v20[4] + v28[7] * v20[6] + v28[4] * v20[5];
      a1[8] = v28[2] * v20[4] + v28[8] * v20[6] + v28[5] * v20[5];
      a1[9] = v28[0] * v20[8] + v28[6] * v20[10] + v28[3] * v20[9];
      a1[10] = v28[1] * v20[8] + v28[7] * v20[10] + v28[4] * v20[9];
      a1[11] = v28[2] * v20[8] + v28[8] * v20[10] + v28[5] * v20[9];
      *a1 = v28[0] * v20[12] + v28[6] * v20[14] + v28[3] * v20[13] + v25[0];
      a1[1] = v28[1] * v20[12] + v28[7] * v20[14] + v28[4] * v20[13] + v25[1];
      v21 = v28[2] * v20[12] + v28[8] * v20[14];
      v22 = v28[5] * v20[13];
      v23 = fx_debugBolt;
      v24 = v21 + v22 + v25[2];
      a1[2] = v24;
      if ( v23->integer )
      {
        fxDebugScale = (float)v23->integer;

        fxDebugEnd[0] = fxDebugScale * a1[3] + a1[0];
        fxDebugEnd[1] = fxDebugScale * a1[4] + a1[1];
        fxDebugEnd[2] = fxDebugScale * a1[5] + v24;
        CL_AddDebugLine( a1, fxDebugEnd, (const float *)dword_541830, 1, 0, 0 );

        fxDebugEnd[0] = fxDebugScale * a1[6] + a1[0];
        fxDebugEnd[1] = fxDebugScale * a1[7] + a1[1];
        fxDebugEnd[2] = fxDebugScale * a1[8] + a1[2];
        CL_AddDebugLine( a1, fxDebugEnd, (const float *)dword_541840, 1, 0, 0 );

        fxDebugEnd[0] = fxDebugScale * a1[9] + a1[0];
        fxDebugEnd[1] = fxDebugScale * a1[10] + a1[1];
        fxDebugEnd[2] = fxDebugScale * a1[11] + a1[2];
        CL_AddDebugLine( a1, fxDebugEnd, (const float *)dword_541860, 1, 0, 0 );
      }
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    v5 = v25[1];
    v6 = v25[2];
    *a1 = v25[0];
    v7 = v28[0];
    a1[1] = v5;
    v8 = v28[1];
    a1[2] = v6;
    v9 = v28[2];
    a1[3] = v7;
    v10 = v28[3];
    a1[4] = v8;
    v11 = v28[4];
    a1[5] = v9;
    v12 = v28[5];
    a1[6] = v10;
    v13 = v28[6];
    a1[7] = v11;
    v14 = v28[7];
    a1[8] = v12;
    v15 = v28[8];
    a1[9] = v13;
    a1[10] = v14;
    a1[11] = v15;
    return 1;
  }
}

/* ---- FX_Init  0x0049EC10 ----  [CONFIRMED] */
int __cdecl FX_Init()
{
  int *v0;

  if ( !dword_1407314 )
  {
    dword_1407314 = 1;
    v0 = &fxActiveEffects;
    do
    {
      *v0 = 0;
      v0 += 3;
    }
    while ( (int)v0 < (int)byte_CA22C0 );
  }
  FX_Free(1);
  theFxHelper = 0;
  fxMTime = 0;
  dword_1407510 = 0;
  dword_1407514 = 0;
  dword_1407518 = 0;
  return 1;
}

/* ---- FX_FreeMember  0x0049EC70 ----  [HIGH] */
int __cdecl FX_FreeMember(_DWORD *a1)
{
  int result;

  (*(void (__cdecl **)(_DWORD))(*(_DWORD *)*a1 + 4))(*a1);
  if ( *a1 )
    (**(void (__cdecl ***)(_DWORD, int))*a1)(*a1, 1);
  a1[2] = fxFreeListHead;
  result = *(_DWORD *)fxActiveCount - 1;
  *a1 = 0;
  fxFreeListHead = (int)a1;
  *(_DWORD *)fxActiveCount = result;
  return result;
}

/* ---- FX_GetValidEffect  0x0049ECB0 ----  [CONFIRMED] */
int *FX_GetValidEffect()
{
  int *result;

  result = (int *)fxFreeListHead;
  if ( fxFreeListHead )
  {
    if ( *(_DWORD *)fxFreeListHead )
      return 0;
    else
      fxFreeListHead = *(_DWORD *)(fxFreeListHead + 8);
  }
  else
  {
    if ( com_statmon->integer )
      StatMon_Warning(7, 3000, "gfx/2d/warning@maxeffects.jpg");
    (*(void (__cdecl **)(int))(*(_DWORD *)fxActiveEffects + 4))(fxActiveEffects);
    if ( fxActiveEffects )
      (**(void (__cdecl ***)(int, int))fxActiveEffects)(fxActiveEffects, 1);
    --*(_DWORD *)fxActiveCount;
    fxActiveEffects = 0;
    dword_C9CE68 = fxFreeListHead;
    fxFreeListHead = 0;
    return &fxActiveEffects;
  }
  return result;
}

/* ---- sub_49ED40  0x0049ED40 ----  [HIGH] */
BOOL sub_49ED40()
{
  return *(int *)fxActiveCount > 0 || *(int *)fxScheduledCount > 0;
}

/* ---- sub_49ED60  0x0049ED60 ----  VERIFIED */
int __cdecl sub_49ED60(int a1)
{
  int v1;
  int *v2;
  int v3;
  _BYTE v5[4]; // [esp+10h] [ebp-1C48h] BYREF
  int v6; // [esp+14h] [ebp-1C44h] BYREF
  int v7[2]; // [esp+18h] [ebp-1C40h] BYREF
  int v8;
  char v9;
  char v10;
  int v11;
  int v12;
  int v13;
  int v14;

  v7[0] = a1;
  v7[1] = v1;
  v8 = 0;
  v9 = 0;
  v10 = 1;
  v11 = 0;
  v14 = 0;
  v12 = 0;
  v13 = 0;
  SFxHelper__AddLightToScene((int)v7, (int)&theFxHelper);
  CFxScheduler__SaveState_m((int)v7, dword_14075A0);
  v2 = &fxActiveEffects;
  v3 = 1800;
  do
  {
    if ( *v2 )
    {
      v5[0] = (*(int (__cdecl **)(int))(*(_DWORD *)*v2 + 20))(*v2);
      CFxArchive__WriteData(v7, v5, 1);
      v6 = v2[1];
      CFxArchive__WriteData(v7, &v6, 4);
      (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v2 + 24))(*v2, v7);
    }
    v2 += 3;
    --v3;
  }
  while ( v3 );
  v5[0] = 0;
  CFxArchive__WriteData(v7, v5, 1);
  return v8;
}

/* ---- FX_Add  0x0049EE40 ----  [HIGH] */
int __cdecl FX_Add(int a1, int a2)
{
  int v2;
  int *v3;
  int *v4; _DWORD *v5;
  int *v6; _DWORD *v7;
  int *v8; _DWORD *v9;
  int *v10; _DWORD *v11;
  int *v12; _DWORD *v13;
  int *v14; _DWORD *v15;
  int *v16; _DWORD *v17;
  int *v18; _DWORD *v19;
  int *v20; _DWORD *v21;
  int *v22; _DWORD *v23;
  int *v24; _DWORD *v25;
  int result;
  unsigned __int8 v28;  // BYREF -- the type-tag byte
  int v29;
  int *v30;
  int v31;                // BYREF -- the 4-byte id ReadData(4, ...) reads per record
  int v32[2];              // BYREF -- the CFxArchive being read: {buffer, ...}
  int v33;
  char v34;
  char v35;
  int v36;
  unsigned char v37[7200]; // BYREF -- CFxArchive's inline scratch buffer (fx_types.h +0x14)
  int v38;
  int v39;
  int v40;

  FX_Free(0);
  v32[0] = a1;
  v32[1] = a2;
  v33 = 0;
  v34 = 1;
  v35 = 0;
  v36 = 0;
  v40 = 0;
  v38 = 0;
  v39 = 0;
  memset(v37, 0, sizeof(v37));
  fxFreeListHead = 0;
  SFxHelper__AddLightToScene((int)v32, (int)&theFxHelper);
  CFxScheduler__SaveState_m((int)v32, dword_14075A0);
  v29 = 0;
  v30 = &fxActiveEffects;
  while ( 1 )
  {
    CFxArchive__ReadData(1, v32, &v28);
    v2 = v28;
    if ( !v28 )
      break;
    CFxArchive__ReadData(4, v32, &v31);
    v3 = v30;
    v30[1] = v31;
    switch ( v2 )
    {
      case 1:
        v4 = FxPool_AllocParticle(&dword_140C9D0, 316);
        if ( !v4 )
          goto LABEL_26;
        v5 = CParticle__ctor(v4);
        *v3 = (int)v5;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 2:
        v6 = FxPool_AllocLine(&dword_140CA18, 328);
        if ( !v6 )
          goto LABEL_26;
        v7 = CLine__ctor(v6);
        *v3 = (int)v7;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 3:
        v8 = FxPool_AllocTail(&dword_140CA00, 344);
        if ( !v8 )
          goto LABEL_26;
        v9 = CTail__ctor(v8);
        *v3 = (int)v9;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 4:
        v10 = FxPool_AllocCylinder(&dword_140C9D8, 344);
        if ( !v10 )
          goto LABEL_26;
        v11 = CCylinder__ctor(v10);
        *v3 = (int)v11;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 5:
        v12 = FxPool_AllocEmitter(&dword_140C9B8, 396);
        if ( !v12 )
          goto LABEL_26;
        v13 = CEmitter__ctor(v12);
        *v3 = (int)v13;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 8:
        v14 = FxPool_AllocOrientedParticle(&dword_140C9F8, 328);
        if ( !v14 )
          goto LABEL_26;
        v15 = COrientedParticle__ctor(v14);
        *v3 = (int)v15;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 9:
        v16 = FxPool_AllocElectricity(&dword_140C9C0, 716);
        if ( !v16 )
          goto LABEL_26;
        v17 = CElectricity__ctor(v16);
        *v3 = (int)v17;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 11:
        v18 = FxPool_AllocLight(&dword_140C9E8, 260);
        if ( !v18 )
          goto LABEL_26;
        v19 = CLight__ctor(v18);
        *v3 = (int)v19;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 13:
        v20 = FxPool_AllocLight(&dword_140C9E8, 260);
        if ( !v20 )
          goto LABEL_26;
        v21 = cand_CFxPrimType13__ctor(v20);
        *v3 = (int)v21;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 14:
        v22 = FxPool_AllocPrimType14(&dword_140CA10, 560);
        if ( !v22 )
          goto LABEL_26;
        v23 = cand_CFxPrimType14__ctor(v22);
        *v3 = (int)v23;
        (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        break;
      case 15:
        v24 = FxPool_AllocFlash(&dword_140C9F0, 436);
        if ( v24 )
        {
          v25 = CFlash__ctor(v24);
          *v3 = (int)v25;
          (*(void (__cdecl **)(int, int *))(*(_DWORD *)*v3 + 24))(*v3, v32);
        }
        else
        {
LABEL_26:
          *v3 = 0;
          (*(void (__cdecl **)(int *))(*(_DWORD *)0 + 24))(v32);
        }
        break;
      default:
        *v3 = 0;
        v3[1] = 0;
        break;
    }
    v30 = v3 + 3;
    if ( ++v29 >= 1800 )
      return v33;
  }
  result = v33;
  fxFreeListHead = (int)&fxActiveEffects + 12 * v29;
  return result;
}

/* ---- FX_DrawStats_m  0x0049F1E0 ----  VERIFIED */
void FX_DrawStats_m()
{
  int *v0;
  int v1;
  _DWORD *v2;
  int v3;

  *(_DWORD *)fxDrawnCount = 0;
  v0 = &fxActiveEffects;
  v1 = 1800;
  do
  {
    v2 = (_DWORD *)*v0;
    if ( !*v0 )
      goto LABEL_17;
    if ( fxMTime > v0[1] )
    {
      v2[6] &= ~0x40000000u;
      (*(void (__cdecl **)(int))(*(_DWORD *)*v0 + 4))(*v0);
      if ( *v0 )
        (**(void (__cdecl ***)(int, int))*v0)(*v0, 1);
      v0[2] = fxFreeListHead;
LABEL_7:
      v3 = *(_DWORD *)fxActiveCount - 1;
      *v0 = 0;
      fxFreeListHead = (int)v0;
      *(_DWORD *)fxActiveCount = v3;
      goto LABEL_17;
    }
    if ( dword_1407514 > 0 && !(*(unsigned __int8 (__cdecl **)(_DWORD *))(*v2 + 8))(v2) )
    {
      (*(void (__cdecl **)(int))(*(_DWORD *)*v0 + 4))(*v0);
      if ( *v0 )
        (**(void (__cdecl ***)(int, int))*v0)(*v0, 1);
      v0[2] = fxFreeListHead;
      goto LABEL_7;
    }
    if ( fx_draw->integer )
    {
      if ( !fx_cull->integer || !(*(unsigned __int8 (__cdecl **)(int))(*(_DWORD *)*v0 + 12))(*v0) )
      {
        ++*(_DWORD *)fxDrawnCount;
        (*(void (__cdecl **)(int))(*(_DWORD *)*v0 + 16))(*v0);
      }
    }
LABEL_17:
    v0 += 3;
    --v1;
  }
  while ( v1 );
  if ( fx_debug->integer )
  {
    SFxHelper__Print((int)&theFxHelper, "Active    FX: %i\n", *(_DWORD *)fxActiveCount);
    SFxHelper__Print((int)&theFxHelper, "Drawn     FX: %i\n", *(_DWORD *)fxDrawnCount);
    SFxHelper__Print((int)&theFxHelper, "Scheduled FX: %i\n", *(_DWORD *)fxScheduledCount);
  }
}

/* ---- FX_AddPrimitive  0x0049F310 ----  [HIGH] */
int __cdecl FX_AddPrimitive(int a1, int *a2)
{
  int *v2;
  int v3;
  double v4;
  int result;

  v2 = FX_GetValidEffect();
  *v2 = *a2;
  v2[1] = a1 + fxMTime;
  ++*(_DWORD *)fxActiveCount;
  v3 = *a2;
  v4 = (double)fxMTime * 0.001;
  *(_DWORD *)(v3 + 16) = fxMTime;
  *(float *)(v3 + 180) = v4;
  result = a1 + fxMTime;
  *(_DWORD *)(*a2 + 20) = a1 + fxMTime;
  return result;
}

/* ---- FX_AddParticle  0x0049F360 ----  [CONFIRMED] */
int __cdecl FX_AddParticle(
        int a1,
        char **a2,
        float *orient,
        float *a4,
        char a5,
        int a6,
        int a7,
        float a8,
        int a9,
        int a10,
        float a11,
        int a12,
        int a13,
        float a14,
        _DWORD *a15,
        _DWORD *a16,
        float a17,
        int a18,
        int a19,
        _DWORD *a20,
        _DWORD *a21,
        int a22,
        int a23,
        int a24,
        int a25,
        int a26,
        unsigned int a27)
{
  int *v30;
  int v31;
  char *v32;
  _DWORD *v33;
  int v34;
  const float *v35;
  const float *v36;
  float v37;
  float v38;
  float *v39;
  unsigned int v40;
  double v41;
  double v42;
  double v43;
  double v44;
  int v45; // [esp+8h] [ebp-28h] BYREF
  float worldPos[3]; // [esp+Ch] [ebp-24h] BYREF
  float out[3]; // [esp+18h] [ebp-18h] BYREF
  float v48[3]; // [esp+24h] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v30 = FxPool_AllocParticle(&dword_140C9D0, 316);
  if ( v30 )
  {
    v30[54] = 0;
    *v30 = (int)&off_559F10;
    v30[15] = 4;
    v31 = (int)v30;
  }
  else
  {
    v31 = 0;
  }
  v45 = v31;
  if ( v31 )
  {
    v32 = *(char **)(v31 + 216);
    if ( v32 != *a2 )
    {
      if ( v32 )
      {
        CFxBoltFrame__Release(v32);
        *(_DWORD *)(v31 + 216) = 0;
      }
      v33 = *a2;
      if ( !*a2 )
        goto LABEL_14;
      ++*v33;
      *(_DWORD *)(v31 + 216) = v33;
    }
    v34 = (int)*a2;
    if ( v34 )
    {
      v35 = (const float *)sub_48D770(v34);
      OrientationPosFromWorldPos(v35, (float *)a1, worldPos);
      OrientationDirFromWorldDir(v35, orient, out);
      OrientationDirFromWorldDir(v35, a4, v48);
      v37 = worldPos[1];
      v38 = worldPos[2];
      *(float *)(v31 + 4) = worldPos[0];
      *(float *)(v31 + 8) = v37;
      *(float *)(v31 + 12) = v38;
      sub_49E2C0((_DWORD *)v31, out);
      v39 = v48;
      goto LABEL_18;
    }
LABEL_14:
    if ( a1 )
    {
      *(_DWORD *)(v31 + 4) = *(_DWORD *)a1;
      *(_DWORD *)(v31 + 8) = *(_DWORD *)(a1 + 4);
      *(_DWORD *)(v31 + 12) = *(_DWORD *)(a1 + 8);
    }
    else
    {
      *(_DWORD *)(v31 + 12) = 0;
      *(_DWORD *)(v31 + 8) = 0;
      *(_DWORD *)(v31 + 4) = 0;
    }
    sub_49E2C0((_DWORD *)v31, orient);
    v39 = a4;
LABEL_18:
    sub_49E300((_DWORD *)v31, v39);
    if ( a15 )
    {
      *(_DWORD *)(v31 + 268) = *a15;
      *(_DWORD *)(v31 + 272) = a15[1];
      *(_DWORD *)(v31 + 276) = a15[2];
    }
    else
    {
      *(_DWORD *)(v31 + 276) = 0;
      *(_DWORD *)(v31 + 272) = 0;
      *(_DWORD *)(v31 + 268) = 0;
    }
    if ( a16 )
    {
      *(_DWORD *)(v31 + 280) = *a16;
      *(_DWORD *)(v31 + 284) = a16[1];
      *(_DWORD *)(v31 + 288) = a16[2];
    }
    else
    {
      *(_DWORD *)(v31 + 288) = 0;
      *(_DWORD *)(v31 + 284) = 0;
      *(_DWORD *)(v31 + 280) = 0;
    }
    v40 = a27;
    if ( (a27 & 0xC0) == 0x80 )
    {
      v41 = a17 * 0.0031415902;
    }
    else
    {
      if ( (a27 & 0xC0) == 0 )
        goto LABEL_29;
      v41 = (double)a25 * a17 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v31 + 292) = v41;
LABEL_29:
    *(_DWORD *)(v31 + 296) = a12;
    *(_DWORD *)(v31 + 300) = a13;
    if ( (a27 & 0xC) == 8 )
    {
      v42 = a14 * 0.0031415902;
    }
    else
    {
      if ( (a27 & 0xC) == 0 )
        goto LABEL_34;
      v42 = (double)a25 * a14 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v31 + 304) = v42;
LABEL_34:
    *(_DWORD *)(v31 + 244) = a6;
    *(_DWORD *)(v31 + 248) = a7;
    if ( (a27 & 0xC00) == 0x800 )
    {
      v43 = a8 * 0.0031415902;
      *(float *)(v31 + 252) = v43;
      if ( !a5 )
      {
        *(float *)(v31 + 264) = v43;
LABEL_37:
        v40 = ((a27 & 0xF00) << 8) | a27 & 0xFFF0FFFF;
        *(_DWORD *)(v31 + 256) = a6;
        *(_DWORD *)(v31 + 260) = a7;
LABEL_38:
        *(_DWORD *)(v31 + 24) = v40;
        *(_DWORD *)(v31 + 164) = a26;
        *(_DWORD *)(v31 + 188) = a18;
        *(_DWORD *)(v31 + 308) = a19;
        *(_DWORD *)(v31 + 312) = a22;
        if ( a20 )
        {
          *(_DWORD *)(v31 + 28) = *a20;
          *(_DWORD *)(v31 + 32) = a20[1];
          *(_DWORD *)(v31 + 36) = a20[2];
        }
        else
        {
          *(_DWORD *)(v31 + 36) = 0;
          *(_DWORD *)(v31 + 32) = 0;
          *(_DWORD *)(v31 + 28) = 0;
        }
        if ( a21 )
        {
          *(_DWORD *)(v31 + 40) = *a21;
          *(_DWORD *)(v31 + 44) = a21[1];
          *(_DWORD *)(v31 + 48) = a21[2];
        }
        else
        {
          *(_DWORD *)(v31 + 48) = 0;
          *(_DWORD *)(v31 + 44) = 0;
          *(_DWORD *)(v31 + 40) = 0;
        }
        *(_DWORD *)(v31 + 56) = a23;
        *(_DWORD *)(v31 + 52) = a24;
        FX_AddPrimitive(a25, &v45);
        return v31;
      }
    }
    else if ( (a27 & 0xC00) != 0 )
    {
      v44 = (double)a25 * a8 * 0.0099999998;
      *(float *)(v31 + 252) = (double)fxMTime + v44;
      if ( !a5 )
      {
        *(float *)(v31 + 264) = (double)fxMTime + v44;
        goto LABEL_37;
      }
    }
    else if ( !a5 )
    {
      goto LABEL_37;
    }
    *(_DWORD *)(v31 + 256) = a9;
    *(_DWORD *)(v31 + 260) = a10;
    if ( (a27 & 0xC0000) == 0x800 )
    {
      *(float *)(v31 + 264) = a11 * 0.0031415902;
    }
    else if ( (a27 & 0xC0000) != 0 )
    {
      *(float *)(v31 + 264) = (double)a25 * a11 * 0.0099999998 + (double)fxMTime;
    }
    goto LABEL_38;
  }
  return v31;
}

/* ---- FX_AddLine  0x0049F740 ----  [CONFIRMED] */
int __cdecl FX_AddLine(
        int a1,
        char **a2,
        float *out,
        int a4,
        int a5,
        float a6,
        int a7,
        int a8,
        float a9,
        _DWORD *a10,
        _DWORD *a11,
        float a12,
        int a13,
        int a14,
        int a15)
{
  int *v18;
  int v19;
  char *v20;
  _DWORD *v21;
  int v22;
  const float *v23;
  const float *v24;
  float v25;
  float v26;
  float v27;
  float v28;
  float v29;
  double v30;
  double v31;
  double v32;
  int v33; // [esp+8h] [ebp-1Ch] BYREF
  float worldPos[3]; // [esp+Ch] [ebp-18h] BYREF
  float v35[3]; // [esp+18h] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v18 = FxPool_AllocLine(&dword_140CA18, 328);
  if ( v18 )
  {
    v18[54] = 0;
    *v18 = (int)&off_559EBC;
    v18[15] = 13;
    v19 = (int)v18;
  }
  else
  {
    v19 = 0;
  }
  v33 = v19;
  if ( v19 )
  {
    v20 = *(char **)(v19 + 216);
    if ( v20 != *a2 )
    {
      if ( v20 )
      {
        CFxBoltFrame__Release(v20);
        *(_DWORD *)(v19 + 216) = 0;
      }
      v21 = *a2;
      if ( !*a2 )
        goto LABEL_14;
      ++*v21;
      *(_DWORD *)(v19 + 216) = v21;
    }
    v22 = (int)*a2;
    if ( v22 )
    {
      v23 = (const float *)sub_48D770(v22);
      OrientationPosFromWorldPos(v23, (float *)a1, worldPos);
      OrientationPosFromWorldPos(v23, out, v35);
      v25 = worldPos[1];
      v26 = worldPos[2];
      *(float *)(v19 + 4) = worldPos[0];
      v27 = v35[0];
      *(float *)(v19 + 8) = v25;
      v28 = v35[1];
      *(float *)(v19 + 12) = v26;
      v29 = v35[2];
      *(float *)(v19 + 316) = v27;
      *(float *)(v19 + 320) = v28;
      *(float *)(v19 + 324) = v29;
      goto LABEL_18;
    }
LABEL_14:
    if ( a1 )
    {
      *(_DWORD *)(v19 + 4) = *(_DWORD *)a1;
      *(_DWORD *)(v19 + 8) = *(_DWORD *)(a1 + 4);
      *(_DWORD *)(v19 + 12) = *(_DWORD *)(a1 + 8);
    }
    else
    {
      *(_DWORD *)(v19 + 12) = 0;
      *(_DWORD *)(v19 + 8) = 0;
      *(_DWORD *)(v19 + 4) = 0;
    }
    *(float *)(v19 + 316) = *out;
    *(float *)(v19 + 320) = out[1];
    *(float *)(v19 + 324) = out[2];
LABEL_18:
    if ( a10 )
    {
      *(_DWORD *)(v19 + 268) = *a10;
      *(_DWORD *)(v19 + 272) = a10[1];
      *(_DWORD *)(v19 + 276) = a10[2];
    }
    else
    {
      *(_DWORD *)(v19 + 276) = 0;
      *(_DWORD *)(v19 + 272) = 0;
      *(_DWORD *)(v19 + 268) = 0;
    }
    if ( a11 )
    {
      *(_DWORD *)(v19 + 280) = *a11;
      *(_DWORD *)(v19 + 284) = a11[1];
      *(_DWORD *)(v19 + 288) = a11[2];
    }
    else
    {
      *(_DWORD *)(v19 + 288) = 0;
      *(_DWORD *)(v19 + 284) = 0;
      *(_DWORD *)(v19 + 280) = 0;
    }
    if ( (a15 & 0xC0) == 0x80 )
    {
      v30 = a12 * 0.0031415902;
    }
    else
    {
      if ( (a15 & 0xC0) == 0 )
        goto LABEL_29;
      v30 = (double)a13 * a12 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v19 + 292) = v30;
LABEL_29:
    *(_DWORD *)(v19 + 296) = a7;
    *(_DWORD *)(v19 + 300) = a8;
    if ( (a15 & 0xC) == 8 )
    {
      v31 = a9 * 0.0031415902;
    }
    else
    {
      if ( (a15 & 0xC) == 0 )
        goto LABEL_34;
      v31 = (double)a13 * a9 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v19 + 304) = v31;
LABEL_34:
    *(_DWORD *)(v19 + 244) = a4;
    *(_DWORD *)(v19 + 248) = a5;
    if ( (a15 & 0xC00) == 0x800 )
    {
      v32 = a6 * 0.0031415902;
    }
    else
    {
      if ( (a15 & 0xC00) == 0 )
      {
LABEL_39:
        *(_DWORD *)(v19 + 164) = a14;
        *(_DWORD *)(v19 + 24) = a15;
        *(_DWORD *)(v19 + 172) = 1065353216;
        *(_DWORD *)(v19 + 176) = 1065353216;
        FX_AddPrimitive(a13, &v33);
        return v19;
      }
      v32 = (double)a13 * a6 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v19 + 252) = v32;
    goto LABEL_39;
  }
  return v19;
}

/* ---- FX_AddElectricity  0x0049F9D0 ----  [CONFIRMED] */
int __cdecl FX_AddElectricity(
        int a1,
        char **a2,
        float *out,
        int a4,
        int a5,
        float a6,
        int a7,
        int a8,
        float a9,
        _DWORD *a10,
        _DWORD *a11,
        float a12,
        int a13,
        int a14,
        int a15,
        int a16)
{
  int *v19;
  int v20;
  char *v21;
  _DWORD *v22;
  int v23;
  const float *v24;
  const float *v25;
  float v26;
  float v27;
  float v28;
  float v29;
  float v30;
  double v31;
  double v32;
  double v33;
  int v34; // [esp+8h] [ebp-1Ch] BYREF
  float worldPos[3]; // [esp+Ch] [ebp-18h] BYREF
  float v36[3]; // [esp+18h] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v19 = FxPool_AllocElectricity(&dword_140C9C0, 716);
  if ( v19 )
  {
    v19[54] = 0;
    *v19 = (int)&off_559F2C;
    v19[15] = 14;
    v20 = (int)v19;
  }
  else
  {
    v20 = 0;
  }
  v34 = v20;
  if ( v20 )
  {
    v21 = *(char **)(v20 + 216);
    if ( v21 != *a2 )
    {
      if ( v21 )
      {
        CFxBoltFrame__Release(v21);
        *(_DWORD *)(v20 + 216) = 0;
      }
      v22 = *a2;
      if ( !*a2 )
        goto LABEL_14;
      ++*v22;
      *(_DWORD *)(v20 + 216) = v22;
    }
    v23 = (int)*a2;
    if ( v23 )
    {
      v24 = (const float *)sub_48D770(v23);
      OrientationPosFromWorldPos(v24, (float *)a1, worldPos);
      OrientationPosFromWorldPos(v24, out, v36);
      v26 = worldPos[1];
      v27 = worldPos[2];
      *(float *)(v20 + 4) = worldPos[0];
      v28 = v36[0];
      *(float *)(v20 + 8) = v26;
      v29 = v36[1];
      *(float *)(v20 + 12) = v27;
      v30 = v36[2];
      *(float *)(v20 + 4) = v28;
      *(float *)(v20 + 8) = v29;
      *(float *)(v20 + 12) = v30;
      goto LABEL_18;
    }
LABEL_14:
    if ( a1 )
    {
      *(_DWORD *)(v20 + 4) = *(_DWORD *)a1;
      *(_DWORD *)(v20 + 8) = *(_DWORD *)(a1 + 4);
      *(_DWORD *)(v20 + 12) = *(_DWORD *)(a1 + 8);
    }
    else
    {
      *(_DWORD *)(v20 + 12) = 0;
      *(_DWORD *)(v20 + 8) = 0;
      *(_DWORD *)(v20 + 4) = 0;
    }
    *(float *)(v20 + 316) = *out;
    *(float *)(v20 + 320) = out[1];
    *(float *)(v20 + 324) = out[2];
LABEL_18:
    if ( a10 )
    {
      *(_DWORD *)(v20 + 268) = *a10;
      *(_DWORD *)(v20 + 272) = a10[1];
      *(_DWORD *)(v20 + 276) = a10[2];
    }
    else
    {
      *(_DWORD *)(v20 + 276) = 0;
      *(_DWORD *)(v20 + 272) = 0;
      *(_DWORD *)(v20 + 268) = 0;
    }
    if ( a11 )
    {
      *(_DWORD *)(v20 + 280) = *a11;
      *(_DWORD *)(v20 + 284) = a11[1];
      *(_DWORD *)(v20 + 288) = a11[2];
    }
    else
    {
      *(_DWORD *)(v20 + 288) = 0;
      *(_DWORD *)(v20 + 284) = 0;
      *(_DWORD *)(v20 + 280) = 0;
    }
    if ( (a16 & 0xC0) == 0x80 )
    {
      v31 = a12 * 0.0031415902;
    }
    else
    {
      if ( (a16 & 0xC0) == 0 )
        goto LABEL_29;
      v31 = (double)a14 * a12 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v20 + 292) = v31;
LABEL_29:
    *(_DWORD *)(v20 + 296) = a7;
    *(_DWORD *)(v20 + 300) = a8;
    if ( (a16 & 0xC) == 8 )
    {
      v32 = a9 * 0.0031415902;
    }
    else
    {
      if ( (a16 & 0xC) == 0 )
        goto LABEL_34;
      v32 = (double)a14 * a9 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v20 + 304) = v32;
LABEL_34:
    *(_DWORD *)(v20 + 244) = a4;
    *(_DWORD *)(v20 + 248) = a5;
    if ( (a16 & 0xC00) == 0x800 )
    {
      v33 = a6 * 0.0031415902;
    }
    else
    {
      if ( (a16 & 0xC00) == 0 )
      {
LABEL_39:
        *(_DWORD *)(v20 + 164) = a15;
        *(_DWORD *)(v20 + 24) = a16;
        *(_DWORD *)(v20 + 328) = a13;
        *(_DWORD *)(v20 + 172) = 1065353216;
        *(_DWORD *)(v20 + 176) = 1065353216;
        FX_AddPrimitive(a14, &v34);
        CElectricity__Initialize(v20);
        return v20;
      }
      v33 = (double)a14 * a6 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v20 + 252) = v33;
    goto LABEL_39;
  }
  return v20;
}

/* ---- FX_AddTail  0x0049FC60 ----  [CONFIRMED] */
int __cdecl FX_AddTail(
        int a1,
        char **a2,
        float *orient,
        float *a4,
        int a5,
        int a6,
        float a7,
        int a8,
        int a9,
        float a10,
        int a11,
        int a12,
        float a13,
        _DWORD *a14,
        _DWORD *a15,
        float a16,
        _DWORD *a17,
        _DWORD *a18,
        int a19,
        int a20,
        int a21,
        int a22,
        int a23,
        int a24)
{
  int *v27;
  int v28;
  char *v29;
  _DWORD *v30;
  int v31;
  const float *v32;
  const float *v33;
  float v34;
  float v35;
  float *v36;
  double v37;
  double v38;
  double v39;
  double v40;
  int v41; // [esp+8h] [ebp-28h] BYREF
  float worldPos[3]; // [esp+Ch] [ebp-24h] BYREF
  float out[3]; // [esp+18h] [ebp-18h] BYREF
  float v44[3]; // [esp+24h] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v27 = FxPool_AllocTail(&dword_140CA00, 344);
  if ( v27 )
  {
    v27[54] = 0;
    *v27 = (int)&off_559F9C;
    v27[15] = 13;
    v28 = (int)v27;
  }
  else
  {
    v28 = 0;
  }
  v41 = v28;
  if ( v28 )
  {
    v29 = *(char **)(v28 + 216);
    if ( v29 != *a2 )
    {
      if ( v29 )
      {
        CFxBoltFrame__Release(v29);
        *(_DWORD *)(v28 + 216) = 0;
      }
      v30 = *a2;
      if ( !*a2 )
        goto LABEL_14;
      ++*v30;
      *(_DWORD *)(v28 + 216) = v30;
    }
    v31 = (int)*a2;
    if ( v31 )
    {
      v32 = (const float *)sub_48D770(v31);
      OrientationPosFromWorldPos(v32, (float *)a1, worldPos);
      OrientationDirFromWorldDir(v32, orient, out);
      OrientationDirFromWorldDir(v32, a4, v44);
      v34 = worldPos[1];
      v35 = worldPos[2];
      *(float *)(v28 + 4) = worldPos[0];
      *(float *)(v28 + 8) = v34;
      *(float *)(v28 + 12) = v35;
      sub_49E2C0((_DWORD *)v28, out);
      v36 = v44;
      goto LABEL_18;
    }
LABEL_14:
    if ( a1 )
    {
      *(_DWORD *)(v28 + 4) = *(_DWORD *)a1;
      *(_DWORD *)(v28 + 8) = *(_DWORD *)(a1 + 4);
      *(_DWORD *)(v28 + 12) = *(_DWORD *)(a1 + 8);
    }
    else
    {
      *(_DWORD *)(v28 + 12) = 0;
      *(_DWORD *)(v28 + 8) = 0;
      *(_DWORD *)(v28 + 4) = 0;
    }
    sub_49E2C0((_DWORD *)v28, orient);
    v36 = a4;
LABEL_18:
    sub_49E300((_DWORD *)v28, v36);
    if ( a14 )
    {
      *(_DWORD *)(v28 + 268) = *a14;
      *(_DWORD *)(v28 + 272) = a14[1];
      *(_DWORD *)(v28 + 276) = a14[2];
    }
    else
    {
      *(_DWORD *)(v28 + 276) = 0;
      *(_DWORD *)(v28 + 272) = 0;
      *(_DWORD *)(v28 + 268) = 0;
    }
    if ( a15 )
    {
      *(_DWORD *)(v28 + 280) = *a15;
      *(_DWORD *)(v28 + 284) = a15[1];
      *(_DWORD *)(v28 + 288) = a15[2];
    }
    else
    {
      *(_DWORD *)(v28 + 288) = 0;
      *(_DWORD *)(v28 + 284) = 0;
      *(_DWORD *)(v28 + 280) = 0;
    }
    if ( (a24 & 0xC0) == 0x80 )
    {
      v37 = a16 * 0.0031415902;
    }
    else
    {
      if ( (a24 & 0xC0) == 0 )
        goto LABEL_29;
      v37 = (double)a22 * a16 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v28 + 292) = v37;
LABEL_29:
    *(_DWORD *)(v28 + 296) = a11;
    *(_DWORD *)(v28 + 300) = a12;
    if ( (a24 & 0xC) == 8 )
    {
      v38 = a13 * 0.0031415902;
    }
    else
    {
      if ( (a24 & 0xC) == 0 )
        goto LABEL_34;
      v38 = (double)a22 * a13 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v28 + 304) = v38;
LABEL_34:
    *(_DWORD *)(v28 + 244) = a5;
    *(_DWORD *)(v28 + 248) = a6;
    if ( (a24 & 0xC00) == 0x800 )
    {
      v39 = a7 * 0.0031415902;
    }
    else
    {
      if ( (a24 & 0xC00) == 0 )
        goto LABEL_39;
      v39 = (double)a22 * a7 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v28 + 252) = v39;
LABEL_39:
    *(_DWORD *)(v28 + 328) = a8;
    *(_DWORD *)(v28 + 332) = a9;
    if ( (a24 & 0xC000) == 0x8000 )
    {
      v40 = a10 * 0.0031415902;
    }
    else
    {
      if ( (a24 & 0xC000) == 0 )
      {
LABEL_44:
        *(_DWORD *)(v28 + 24) = a24;
        *(_DWORD *)(v28 + 164) = a23;
        *(_DWORD *)(v28 + 312) = a19;
        if ( a17 )
        {
          *(_DWORD *)(v28 + 28) = *a17;
          *(_DWORD *)(v28 + 32) = a17[1];
          *(_DWORD *)(v28 + 36) = a17[2];
        }
        else
        {
          *(_DWORD *)(v28 + 36) = 0;
          *(_DWORD *)(v28 + 32) = 0;
          *(_DWORD *)(v28 + 28) = 0;
        }
        if ( a18 )
        {
          *(_DWORD *)(v28 + 40) = *a18;
          *(_DWORD *)(v28 + 44) = a18[1];
          *(_DWORD *)(v28 + 48) = a18[2];
        }
        else
        {
          *(_DWORD *)(v28 + 48) = 0;
          *(_DWORD *)(v28 + 44) = 0;
          *(_DWORD *)(v28 + 40) = 0;
        }
        *(_DWORD *)(v28 + 172) = 1065353216;
        *(_DWORD *)(v28 + 176) = 1065353216;
        *(_DWORD *)(v28 + 56) = a20;
        *(_DWORD *)(v28 + 52) = a21;
        FX_AddPrimitive(a22, &v41);
        return v28;
      }
      v40 = (double)a22 * a10 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v28 + 336) = v40;
    goto LABEL_44;
  }
  return v28;
}

/* ---- FX_AddCylinder  0x0049FFB0 ----  [CONFIRMED] */
int __cdecl FX_AddCylinder(
        int a1,
        char **a2,
        float *orient,
        int a4,
        int a5,
        float a6,
        int a7,
        int a8,
        float a9,
        int a10,
        int a11,
        float a12,
        int a13,
        int a14,
        float a15,
        _DWORD *a16,
        _DWORD *a17,
        float a18,
        int a19,
        int a20,
        int a21)
{
  int *v24;
  int v25;
  char *v26;
  _DWORD *v27;
  int v28;
  const float *v29;
  float v30;
  float v31;
  float v32;
  float v33;
  float v34;
  double v35;
  double v36;
  double v37;
  double v38;
  double v39;
  int v40; // [esp+Ch] [ebp-1Ch] BYREF
  float worldPos[3]; // [esp+10h] [ebp-18h] BYREF
  float out[3]; // [esp+1Ch] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v24 = FxPool_AllocCylinder(&dword_140C9D8, 344);
  if ( v24 )
  {
    v24[54] = 0;
    *v24 = (int)&off_559ED8;
    v24[15] = 15;
    v25 = (int)v24;
  }
  else
  {
    v25 = 0;
  }
  v40 = v25;
  if ( v25 )
  {
    v26 = *(char **)(v25 + 216);
    if ( v26 != *a2 )
    {
      if ( v26 )
      {
        CFxBoltFrame__Release(v26);
        *(_DWORD *)(v25 + 216) = 0;
      }
      v27 = *a2;
      if ( !*a2 )
        goto LABEL_14;
      ++*v27;
      *(_DWORD *)(v25 + 216) = v27;
    }
    v28 = (int)*a2;
    if ( v28 )
    {
      v29 = (const float *)sub_48D770(v28);
      OrientationPosFromWorldPos(v29, (float *)a1, worldPos);
      OrientationDirFromWorldDir(v29, orient, out);
      v30 = worldPos[0];
      v31 = worldPos[1];
      *(float *)(v25 + 12) = worldPos[2];
      v32 = out[2];
      *(float *)(v25 + 4) = v30;
      v33 = out[0];
      *(float *)(v25 + 8) = v31;
      v34 = out[1];
      *(float *)(v25 + 96) = v32;
      *(float *)(v25 + 88) = v33;
      *(float *)(v25 + 92) = v34;
      goto LABEL_18;
    }
LABEL_14:
    if ( a1 )
    {
      *(_DWORD *)(v25 + 4) = *(_DWORD *)a1;
      *(_DWORD *)(v25 + 8) = *(_DWORD *)(a1 + 4);
      *(_DWORD *)(v25 + 12) = *(_DWORD *)(a1 + 8);
    }
    else
    {
      *(_DWORD *)(v25 + 12) = 0;
      *(_DWORD *)(v25 + 8) = 0;
      *(_DWORD *)(v25 + 4) = 0;
    }
    *(float *)(v25 + 88) = *orient;
    *(float *)(v25 + 92) = orient[1];
    *(float *)(v25 + 96) = orient[2];
LABEL_18:
    if ( a16 )
    {
      *(_DWORD *)(v25 + 268) = *a16;
      *(_DWORD *)(v25 + 272) = a16[1];
      *(_DWORD *)(v25 + 276) = a16[2];
    }
    else
    {
      *(_DWORD *)(v25 + 276) = 0;
      *(_DWORD *)(v25 + 272) = 0;
      *(_DWORD *)(v25 + 268) = 0;
    }
    if ( a17 )
    {
      *(_DWORD *)(v25 + 280) = *a17;
      *(_DWORD *)(v25 + 284) = a17[1];
      *(_DWORD *)(v25 + 288) = a17[2];
    }
    else
    {
      *(_DWORD *)(v25 + 288) = 0;
      *(_DWORD *)(v25 + 284) = 0;
      *(_DWORD *)(v25 + 280) = 0;
    }
    if ( (a21 & 0xC0) == 0x80 )
    {
      v35 = a18 * 0.0031415902;
    }
    else
    {
      if ( (a21 & 0xC0) == 0 )
        goto LABEL_29;
      v35 = (double)a19 * a18 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v25 + 292) = v35;
LABEL_29:
    *(_DWORD *)(v25 + 244) = a4;
    *(_DWORD *)(v25 + 248) = a5;
    if ( (a21 & 0xC00) == 0x800 )
    {
      v36 = a6 * 0.0031415902;
    }
    else
    {
      if ( (a21 & 0xC00) == 0 )
        goto LABEL_34;
      v36 = (double)a19 * a6 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v25 + 252) = v36;
LABEL_34:
    *(_DWORD *)(v25 + 256) = a7;
    *(_DWORD *)(v25 + 260) = a8;
    if ( (a21 & 0xC0000) == 0x80000 )
    {
      v37 = a9 * 0.0031415902;
    }
    else
    {
      if ( (a21 & 0xC0000) == 0 )
        goto LABEL_39;
      v37 = (double)a19 * a9 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v25 + 264) = v37;
LABEL_39:
    *(_DWORD *)(v25 + 328) = a10;
    *(_DWORD *)(v25 + 332) = a11;
    if ( (a21 & 0xC000) == 0x8000 )
    {
      v38 = a12 * 0.0031415902;
    }
    else
    {
      if ( (a21 & 0xC000) == 0 )
        goto LABEL_44;
      v38 = (double)a19 * a12 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v25 + 336) = v38;
LABEL_44:
    *(_DWORD *)(v25 + 296) = a13;
    *(_DWORD *)(v25 + 300) = a14;
    if ( (a21 & 0xC) == 8 )
    {
      v39 = a15 * 0.0031415902;
    }
    else
    {
      if ( (a21 & 0xC) == 0 )
      {
LABEL_49:
        *(_DWORD *)(v25 + 164) = a20;
        *(_DWORD *)(v25 + 24) = a21;
        FX_AddPrimitive(a19, &v40);
        return v25;
      }
      v39 = (double)a19 * a15 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v25 + 304) = v39;
    goto LABEL_49;
  }
  return v25;
}

/* ---- FX_AddEmitter  0x004A02C0 ----  [CONFIRMED] */
int __cdecl FX_AddEmitter(
        int a1,
        char **a2,
        float *orient,
        float *a4,
        int a5,
        int a6,
        float a7,
        int a8,
        int a9,
        float a10,
        _DWORD *a11,
        _DWORD *a12,
        float a13,
        _DWORD *a14,
        _DWORD *a15,
        _DWORD *a16,
        _DWORD *a17,
        int a18,
        int a19,
        int a20,
        int a21,
        int a22,
        int a23,
        int a24,
        int a25,
        unsigned int a26)
{
  int *v29;
  int v30;
  char *v31;
  _DWORD *v32;
  int v33;
  float *v34;
  const float *v35;
  float v36;
  float v37;
  unsigned int v38;
  double v39;
  double v40;
  double v41;
  int v42;
  int v43; // [esp+10h] [ebp-28h] BYREF
  float worldPos[3]; // [esp+14h] [ebp-24h] BYREF
  float out[3]; // [esp+20h] [ebp-18h] BYREF
  float v46[3]; // [esp+2Ch] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v29 = FxPool_AllocEmitter(&dword_140C9B8, 396);
  if ( v29 )
  {
    v29[54] = 0;
    v29[15] = 1;
    *v29 = (int)&off_559F48;
    v30 = (int)v29;
  }
  else
  {
    v30 = 0;
  }
  v43 = v30;
  if ( v30 )
  {
    v31 = *(char **)(v30 + 216);
    if ( v31 != *a2 )
    {
      if ( v31 )
      {
        CFxBoltFrame__Release(v31);
        *(_DWORD *)(v30 + 216) = 0;
      }
      v32 = *a2;
      if ( !*a2 )
        goto LABEL_14;
      ++*v32;
      *(_DWORD *)(v30 + 216) = v32;
    }
    v33 = (int)*a2;
    if ( v33 )
    {
      v34 = (float *)sub_48D770(v33);
      OrientationPosFromWorldPos(v34, (float *)a1, worldPos);
      OrientationDirFromWorldDir(v34, orient, out);
      OrientationDirFromWorldDir(v34, a4, v46);
      v36 = worldPos[1];
      v37 = worldPos[2];
      *(float *)(v30 + 4) = worldPos[0];
      *(float *)(v30 + 8) = v36;
      *(float *)(v30 + 12) = v37;
      sub_49E2C0((_DWORD *)v30, out);
      sub_49E300((_DWORD *)v30, v46);
      sub_49E690((_DWORD *)v30, worldPos);
      sub_49E6D0((_DWORD *)v30, out);
      sub_49E710((_DWORD *)v30, v34);
      goto LABEL_18;
    }
LABEL_14:
    if ( a1 )
    {
      *(_DWORD *)(v30 + 4) = *(_DWORD *)a1;
      *(_DWORD *)(v30 + 8) = *(_DWORD *)(a1 + 4);
      *(_DWORD *)(v30 + 12) = *(_DWORD *)(a1 + 8);
    }
    else
    {
      *(_DWORD *)(v30 + 12) = 0;
      *(_DWORD *)(v30 + 8) = 0;
      *(_DWORD *)(v30 + 4) = 0;
    }
    sub_49E2C0((_DWORD *)v30, orient);
    sub_49E300((_DWORD *)v30, a4);
    sub_49E690((_DWORD *)v30, (_DWORD *)a1);
    sub_49E6D0((_DWORD *)v30, orient);
    *(_DWORD *)(v30 + 348) = 0;
    *(_DWORD *)(v30 + 344) = 0;
    *(_DWORD *)(v30 + 340) = 0;
LABEL_18:
    v38 = a26;
    if ( (a26 & 0x800000) == 0 )
    {
      if ( a1 )
      {
        *(_DWORD *)(v30 + 72) = *(_DWORD *)a1;
        *(_DWORD *)(v30 + 76) = *(_DWORD *)(a1 + 4);
        *(_DWORD *)(v30 + 80) = *(_DWORD *)(a1 + 8);
        *(_DWORD *)(v30 + 64) |= 0x80u;
      }
    }
    if ( a11 )
    {
      *(_DWORD *)(v30 + 268) = *a11;
      *(_DWORD *)(v30 + 272) = a11[1];
      *(_DWORD *)(v30 + 276) = a11[2];
    }
    else
    {
      *(_DWORD *)(v30 + 276) = 0;
      *(_DWORD *)(v30 + 272) = 0;
      *(_DWORD *)(v30 + 268) = 0;
    }
    if ( a12 )
    {
      *(_DWORD *)(v30 + 280) = *a12;
      *(_DWORD *)(v30 + 284) = a12[1];
      *(_DWORD *)(v30 + 288) = a12[2];
    }
    else
    {
      *(_DWORD *)(v30 + 288) = 0;
      *(_DWORD *)(v30 + 284) = 0;
      *(_DWORD *)(v30 + 280) = 0;
    }
    if ( (a26 & 0xC0) == 0x80 )
    {
      v39 = a13 * 0.0031415902;
    }
    else
    {
      if ( (a26 & 0xC0) == 0 )
        goto LABEL_32;
      v39 = (double)a24 * a13 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v30 + 292) = v39;
LABEL_32:
    *(_DWORD *)(v30 + 244) = a5;
    *(_DWORD *)(v30 + 248) = a6;
    if ( (a26 & 0xC00) == 0x800 )
    {
      v40 = a7 * 0.0031415902;
    }
    else
    {
      if ( (a26 & 0xC00) == 0 )
        goto LABEL_37;
      v40 = (double)a24 * a7 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v30 + 252) = v40;
LABEL_37:
    *(_DWORD *)(v30 + 296) = a8;
    *(_DWORD *)(v30 + 300) = a9;
    if ( (a26 & 0xC) == 8 )
    {
      v41 = a10 * 0.0031415902;
    }
    else
    {
      if ( (a26 & 0xC) == 0 )
      {
LABEL_42:
        if ( !a25 )
          v38 = a26 & 0xFEFFFFFF;
        if ( a14 )
        {
          *(_DWORD *)(v30 + 360) = *a14;
          *(_DWORD *)(v30 + 364) = a14[1];
          *(_DWORD *)(v30 + 368) = a14[2];
        }
        else
        {
          *(_DWORD *)(v30 + 368) = 0;
          *(_DWORD *)(v30 + 364) = 0;
          *(_DWORD *)(v30 + 360) = 0;
        }
        if ( a15 )
        {
          *(_DWORD *)(v30 + 372) = *a15;
          *(_DWORD *)(v30 + 376) = a15[1];
          *(_DWORD *)(v30 + 380) = a15[2];
        }
        else
        {
          *(_DWORD *)(v30 + 380) = 0;
          *(_DWORD *)(v30 + 376) = 0;
          *(_DWORD *)(v30 + 372) = 0;
        }
        *(_DWORD *)(v30 + 24) = v38;
        *(_DWORD *)(v30 + 204) = a25;
        *(_DWORD *)(v30 + 312) = a18;
        if ( a16 )
        {
          *(_DWORD *)(v30 + 28) = *a16;
          *(_DWORD *)(v30 + 32) = a16[1];
          *(_DWORD *)(v30 + 36) = a16[2];
        }
        else
        {
          *(_DWORD *)(v30 + 36) = 0;
          *(_DWORD *)(v30 + 32) = 0;
          *(_DWORD *)(v30 + 28) = 0;
        }
        if ( a17 )
        {
          *(_DWORD *)(v30 + 40) = *a17;
          *(_DWORD *)(v30 + 44) = a17[1];
          *(_DWORD *)(v30 + 48) = a17[2];
        }
        else
        {
          *(_DWORD *)(v30 + 48) = 0;
          *(_DWORD *)(v30 + 44) = 0;
          *(_DWORD *)(v30 + 40) = 0;
        }
        *(_DWORD *)(v30 + 56) = a19;
        *(_DWORD *)(v30 + 52) = a20;
        *(_DWORD *)(v30 + 384) = a21;
        *(_DWORD *)(v30 + 388) = a22;
        *(_DWORD *)(v30 + 392) = a23;
        *(_DWORD *)(v30 + 352) = fxMTime;
        v42 = rand();
        *(float *)(v30 + 356) = ((double)v42 * 0.000030517578 + (double)v42 * 0.000030517578 - 1.0)
                              * *(float *)(v30 + 392)
                              + *(float *)(v30 + 388);
        FX_AddPrimitive(a24, &v43);
        return v30;
      }
      v41 = (double)a24 * a10 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v30 + 304) = v41;
    goto LABEL_42;
  }
  return v30;
}

/* ---- FX_AddLight  0x004A06F0 ----  [CONFIRMED] */
int __cdecl FX_AddLight(
        char **a1,
        float *out,
        int a3,
        int a4,
        float a5,
        _DWORD *a6,
        _DWORD *a7,
        float a8,
        int a9,
        int a10)
{
  int *v12;
  int v13;
  char *v14;
  _DWORD *v15;
  int v16;
  const float *v17;
  float v18;
  float v19;
  double v20;
  double v21;
  int v22; // [esp+8h] [ebp-10h] BYREF
  float worldPos[3]; // [esp+Ch] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v12 = FxPool_AllocLight(&dword_140C9E8, 260);
  if ( v12 )
  {
    v12[54] = 0;
    *v12 = (int)&off_559F64;
    v13 = (int)v12;
  }
  else
  {
    v13 = 0;
  }
  v22 = v13;
  if ( v13 )
  {
    v14 = *(char **)(v13 + 216);
    if ( v14 != *a1 )
    {
      if ( v14 )
      {
        CFxBoltFrame__Release(v14);
        *(_DWORD *)(v13 + 216) = 0;
      }
      v15 = *a1;
      if ( !*a1 )
        goto LABEL_14;
      ++*v15;
      *(_DWORD *)(v13 + 216) = v15;
    }
    v16 = (int)*a1;
    if ( v16 )
    {
      v17 = (const float *)sub_48D770(v16);
      OrientationPosFromWorldPos(v17, out, worldPos);
      v18 = worldPos[1];
      v19 = worldPos[2];
      *(float *)(v13 + 4) = worldPos[0];
      *(float *)(v13 + 8) = v18;
      *(float *)(v13 + 12) = v19;
      goto LABEL_17;
    }
LABEL_14:
    if ( out )
    {
      *(float *)(v13 + 4) = *out;
      *(float *)(v13 + 8) = out[1];
      *(float *)(v13 + 12) = out[2];
    }
    else
    {
      *(_DWORD *)(v13 + 12) = 0;
      *(_DWORD *)(v13 + 8) = 0;
      *(_DWORD *)(v13 + 4) = 0;
    }
LABEL_17:
    if ( a6 )
    {
      *(_DWORD *)(v13 + 232) = *a6;
      *(_DWORD *)(v13 + 236) = a6[1];
      *(_DWORD *)(v13 + 240) = a6[2];
    }
    else
    {
      *(_DWORD *)(v13 + 240) = 0;
      *(_DWORD *)(v13 + 236) = 0;
      *(_DWORD *)(v13 + 232) = 0;
    }
    if ( a7 )
    {
      *(_DWORD *)(v13 + 244) = *a7;
      *(_DWORD *)(v13 + 248) = a7[1];
      *(_DWORD *)(v13 + 252) = a7[2];
    }
    else
    {
      *(_DWORD *)(v13 + 252) = 0;
      *(_DWORD *)(v13 + 248) = 0;
      *(_DWORD *)(v13 + 244) = 0;
    }
    if ( (a10 & 0xC0) == 0x80 )
    {
      v20 = a8 * 0.0031415902;
    }
    else
    {
      if ( (a10 & 0xC0) == 0 )
        goto LABEL_28;
      v20 = (double)a9 * a8 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v13 + 256) = v20;
LABEL_28:
    *(_DWORD *)(v13 + 220) = a3;
    *(_DWORD *)(v13 + 224) = a4;
    if ( (a10 & 0xC00) == 0x800 )
    {
      v21 = a5 * 0.0031415902;
    }
    else
    {
      if ( (a10 & 0xC00) == 0 )
      {
LABEL_33:
        *(_DWORD *)(v13 + 24) = a10;
        FX_AddPrimitive(a9, &v22);
        return v13;
      }
      v21 = (double)a9 * a5 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v13 + 228) = v21;
    goto LABEL_33;
  }
  return v13;
}

/* ---- FX_AddOrientedParticle  0x004A08D0 ----  [CONFIRMED] */
int __cdecl FX_AddOrientedParticle(
        int a1,
        char **a2,
        int a3,
        float *orient,
        float *a5,
        char a6,
        int a7,
        int a8,
        float a9,
        int a10,
        int a11,
        float a12,
        int a13,
        int a14,
        float a15,
        _DWORD *a16,
        _DWORD *a17,
        float a18,
        int a19,
        int a20,
        _DWORD *a21,
        _DWORD *a22,
        int a23,
        int a24,
        int a25,
        int a26,
        int a27,
        unsigned int a28)
{
  int *v31;
  int v32;
  int v33;
  char *v34;
  _DWORD *v35;
  int v36;
  const float *v37;
  const float *v38;
  const float *v39;
  float v40;
  float v41;
  float v42;
  float v43;
  float v44;
  float *v45;
  unsigned int v46;
  double v47;
  double v48;
  double v49;
  double v50;
  int v51; // [esp+8h] [ebp-34h] BYREF
  float worldPos[3]; // [esp+Ch] [ebp-30h] BYREF
  float out[3]; // [esp+18h] [ebp-24h] BYREF
  float v54[3]; // [esp+24h] [ebp-18h] BYREF
  float v55[3]; // [esp+30h] [ebp-Ch] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v31 = FxPool_AllocOrientedParticle(&dword_140C9F8, 328);
  v32 = 0;
  if ( v31 )
  {
    v31[54] = 0;
    *v31 = (int)&off_559EF4;
    v31[15] = 12;
    v33 = (int)v31;
  }
  else
  {
    v33 = 0;
  }
  v51 = v33;
  if ( v33 )
  {
    v34 = *(char **)(v33 + 216);
    if ( v34 != *a2 )
    {
      if ( v34 )
      {
        CFxBoltFrame__Release(v34);
        *(_DWORD *)(v33 + 216) = 0;
        v32 = 0;
      }
      v35 = *a2;
      if ( !*a2 )
        goto LABEL_14;
      ++*v35;
      *(_DWORD *)(v33 + 216) = v35;
    }
    v36 = (int)*a2;
    if ( v36 )
    {
      v37 = (const float *)sub_48D770(v36);
      OrientationPosFromWorldPos(v37, (float *)a3, worldPos);
      OrientationDirFromWorldDir(v37, (const float *)a1, out);
      OrientationDirFromWorldDir(v37, orient, v54);
      OrientationDirFromWorldDir(v37, a5, v55);
      v40 = worldPos[1];
      v41 = worldPos[2];
      *(float *)(v33 + 4) = worldPos[0];
      v42 = out[0];
      *(float *)(v33 + 8) = v40;
      v43 = out[1];
      *(float *)(v33 + 12) = v41;
      v44 = out[2];
      *(float *)(v33 + 316) = v42;
      *(float *)(v33 + 320) = v43;
      *(float *)(v33 + 324) = v44;
      sub_49E2C0((_DWORD *)v33, v54);
      v45 = v55;
      goto LABEL_18;
    }
LABEL_14:
    if ( a3 )
    {
      *(_DWORD *)(v33 + 4) = *(_DWORD *)a3;
      *(_DWORD *)(v33 + 8) = *(_DWORD *)(a3 + 4);
      v32 = *(_DWORD *)(a3 + 8);
    }
    else
    {
      *(_DWORD *)(v33 + 8) = 0;
      *(_DWORD *)(v33 + 4) = 0;
    }
    *(_DWORD *)(v33 + 12) = v32;
    *(_DWORD *)(v33 + 316) = *(_DWORD *)a1;
    *(_DWORD *)(v33 + 320) = *(_DWORD *)(a1 + 4);
    *(_DWORD *)(v33 + 324) = *(_DWORD *)(a1 + 8);
    sub_49E2C0((_DWORD *)v33, orient);
    v45 = a5;
LABEL_18:
    sub_49E300((_DWORD *)v33, v45);
    if ( a16 )
    {
      *(_DWORD *)(v33 + 268) = *a16;
      *(_DWORD *)(v33 + 272) = a16[1];
      *(_DWORD *)(v33 + 276) = a16[2];
    }
    else
    {
      *(_DWORD *)(v33 + 276) = 0;
      *(_DWORD *)(v33 + 272) = 0;
      *(_DWORD *)(v33 + 268) = 0;
    }
    if ( a17 )
    {
      *(_DWORD *)(v33 + 280) = *a17;
      *(_DWORD *)(v33 + 284) = a17[1];
      *(_DWORD *)(v33 + 288) = a17[2];
    }
    else
    {
      *(_DWORD *)(v33 + 288) = 0;
      *(_DWORD *)(v33 + 284) = 0;
      *(_DWORD *)(v33 + 280) = 0;
    }
    v46 = a28;
    if ( (a28 & 0xC0) == 0x80 )
    {
      v47 = a18 * 0.0031415902;
    }
    else
    {
      if ( (a28 & 0xC0) == 0 )
        goto LABEL_29;
      v47 = (double)a26 * a18 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v33 + 292) = v47;
LABEL_29:
    *(_DWORD *)(v33 + 296) = a13;
    *(_DWORD *)(v33 + 300) = a14;
    if ( (a28 & 0xC) == 8 )
    {
      v48 = a15 * 0.0031415902;
    }
    else
    {
      if ( (a28 & 0xC) == 0 )
        goto LABEL_34;
      v48 = (double)a26 * a15 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v33 + 304) = v48;
LABEL_34:
    *(_DWORD *)(v33 + 244) = a7;
    *(_DWORD *)(v33 + 248) = a8;
    if ( (a28 & 0xC00) == 0x800 )
    {
      v49 = a9 * 0.0031415902;
      *(float *)(v33 + 252) = v49;
      if ( !a6 )
      {
        *(float *)(v33 + 264) = v49;
LABEL_37:
        v46 = ((a28 & 0xF00) << 8) | a28 & 0xFFF0FFFF;
        *(_DWORD *)(v33 + 256) = a7;
        *(_DWORD *)(v33 + 260) = a8;
LABEL_38:
        *(_DWORD *)(v33 + 24) = v46;
        *(_DWORD *)(v33 + 164) = a27;
        *(_DWORD *)(v33 + 188) = a19;
        *(_DWORD *)(v33 + 308) = a20;
        *(_DWORD *)(v33 + 312) = a23;
        if ( a21 )
        {
          *(_DWORD *)(v33 + 28) = *a21;
          *(_DWORD *)(v33 + 32) = a21[1];
          *(_DWORD *)(v33 + 36) = a21[2];
        }
        else
        {
          *(_DWORD *)(v33 + 36) = 0;
          *(_DWORD *)(v33 + 32) = 0;
          *(_DWORD *)(v33 + 28) = 0;
        }
        if ( a22 )
        {
          *(_DWORD *)(v33 + 40) = *a22;
          *(_DWORD *)(v33 + 44) = a22[1];
          *(_DWORD *)(v33 + 48) = a22[2];
        }
        else
        {
          *(_DWORD *)(v33 + 48) = 0;
          *(_DWORD *)(v33 + 44) = 0;
          *(_DWORD *)(v33 + 40) = 0;
        }
        *(_DWORD *)(v33 + 56) = a24;
        *(_DWORD *)(v33 + 52) = a25;
        FX_AddPrimitive(a26, &v51);
        return v33;
      }
    }
    else if ( (a28 & 0xC00) != 0 )
    {
      v50 = (double)a26 * a9 * 0.0099999998;
      *(float *)(v33 + 252) = (double)fxMTime + v50;
      if ( !a6 )
      {
        *(float *)(v33 + 264) = (double)fxMTime + v50;
        goto LABEL_37;
      }
    }
    else if ( !a6 )
    {
      goto LABEL_37;
    }
    *(_DWORD *)(v33 + 256) = a10;
    *(_DWORD *)(v33 + 260) = a11;
    if ( (a28 & 0xC0000) == 0x800 )
    {
      *(float *)(v33 + 264) = a12 * 0.0031415902;
    }
    else if ( (a28 & 0xC0000) != 0 )
    {
      *(float *)(v33 + 264) = (double)a26 * a12 * 0.0099999998 + (double)fxMTime;
    }
    goto LABEL_38;
  }
  return v33;
}

/* ---- cand_CFlash_Create  0x004A0D00 ----  [CONFIRMED] */
int __cdecl cand_CFlash_Create(
        int a1,
        int a2,
        int a3,
        _DWORD *a4,
        _DWORD *a5,
        int a6,
        int a7,
        float a8,
        _DWORD *a9,
        _DWORD *a10,
        float a11,
        _DWORD *a12,
        int a13,
        int a14,
        int a15,
        int a16,
        int a17)
{
  _DWORD *v17;
  int *v19;
  int v20;
  int v21;
  _DWORD *v22;
  _DWORD *v23;
  _DWORD *v24;
  double v25;
  double v26;
  int v28; // [esp+Ch] [ebp-4h] BYREF

  v17 = a4;
  if ( dword_1407514 >= 1 && a2 )
  {
    v19 = FxPool_AllocFlash(&dword_140C9F0, 436);
    if ( v19 )
    {
      v19[54] = 0;
      v19[15] = 4;
      *v19 = (int)&off_559EA0;
      v20 = (int)v19;
    }
    else
    {
      v20 = 0;
    }
    v28 = v20;
    if ( !v20 )
      return v20;
    v21 = 0;
    if ( a3 > 0 )
    {
      v22 = (_DWORD *)(v20 + 396);
      v23 = (_DWORD *)(a2 + 8);
      v24 = (_DWORD *)(v20 + 340);
      do
      {
        *(v24 - 1) = *(v23 - 2);
        *v24 = *(v23 - 1);
        v24[1] = *v23;
        *v22 = *(_DWORD *)(a1 + 8 * v21);
        v22[1] = *(_DWORD *)(a1 + 8 * v21++ + 4);
        v23 += 3;
        v24 += 3;
        v22 += 2;
      }
      while ( v21 < a3 );
      v17 = a4;
    }
    if ( v17 )
    {
      *(_DWORD *)(v20 + 220) = *v17;
      *(_DWORD *)(v20 + 224) = v17[1];
      *(_DWORD *)(v20 + 228) = v17[2];
    }
    else
    {
      *(_DWORD *)(v20 + 228) = 0;
      *(_DWORD *)(v20 + 224) = 0;
      *(_DWORD *)(v20 + 220) = 0;
    }
    if ( a5 )
    {
      *(_DWORD *)(v20 + 232) = *a5;
      *(_DWORD *)(v20 + 236) = a5[1];
      *(_DWORD *)(v20 + 240) = a5[2];
    }
    else
    {
      *(_DWORD *)(v20 + 240) = 0;
      *(_DWORD *)(v20 + 236) = 0;
      *(_DWORD *)(v20 + 232) = 0;
    }
    if ( a9 )
    {
      *(_DWORD *)(v20 + 268) = *a9;
      *(_DWORD *)(v20 + 272) = a9[1];
      *(_DWORD *)(v20 + 276) = a9[2];
    }
    else
    {
      *(_DWORD *)(v20 + 276) = 0;
      *(_DWORD *)(v20 + 272) = 0;
      *(_DWORD *)(v20 + 268) = 0;
    }
    if ( a10 )
    {
      *(_DWORD *)(v20 + 280) = *a10;
      *(_DWORD *)(v20 + 284) = a10[1];
      *(_DWORD *)(v20 + 288) = a10[2];
    }
    else
    {
      *(_DWORD *)(v20 + 288) = 0;
      *(_DWORD *)(v20 + 284) = 0;
      *(_DWORD *)(v20 + 280) = 0;
    }
    if ( (a17 & 0xC0) == 0x80 )
    {
      v25 = a11 * 0.0031415902;
    }
    else
    {
      if ( (a17 & 0xC0) == 0 )
        goto LABEL_28;
      v25 = (double)a15 * a11 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v20 + 292) = v25;
LABEL_28:
    *(_DWORD *)(v20 + 296) = a6;
    *(_DWORD *)(v20 + 300) = a7;
    if ( (a17 & 0xC) == 8 )
    {
      v26 = a8 * 0.0031415902;
    }
    else
    {
      if ( (a17 & 0xC) == 0 )
      {
LABEL_33:
        *(_DWORD *)(v20 + 24) = a17;
        *(_DWORD *)(v20 + 164) = a16;
        if ( a12 )
        {
          *(_DWORD *)(v20 + 320) = *a12;
          *(_DWORD *)(v20 + 324) = a12[1];
          *(_DWORD *)(v20 + 328) = a12[2];
        }
        else
        {
          *(_DWORD *)(v20 + 328) = 0;
          *(_DWORD *)(v20 + 324) = 0;
          *(_DWORD *)(v20 + 320) = 0;
        }
        *(_DWORD *)(v20 + 312) = a13;
        *(_DWORD *)(v20 + 332) = a14 + fxMTime;
        *(_DWORD *)(v20 + 316) = a3;
        CFlash__DrawHelper_m(a14, v20);
        FX_AddPrimitive(a15, &v28);
        return v20;
      }
      v26 = (double)a15 * a8 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v20 + 304) = v26;
    goto LABEL_33;
  }
  return 0;
}

/* ---- FX_AddFlash  0x004A0FA0 ----  VERIFIED */
int __cdecl FX_AddFlash(_DWORD *a1, _DWORD *a2, _DWORD *a3, float a4, int a5, int a6, int a7)
{
  int *v10;
  int v11;
  double v12;
  int v13; // [esp+Ch] [ebp-4h] BYREF

  if ( dword_1407514 < 1 )
    return 0;
  v10 = FxPool_AllocLight(&dword_140C9E8, 260);
  if ( v10 )
  {
    v10[54] = 0;
    *v10 = (int)&off_55946C;
    v11 = (int)v10;
  }
  else
  {
    v11 = 0;
  }
  v13 = v11;
  if ( v11 )
  {
    if ( a2 )
    {
      *(_DWORD *)(v11 + 4) = *a2;
      *(_DWORD *)(v11 + 8) = a2[1];
      *(_DWORD *)(v11 + 12) = a2[2];
    }
    else
    {
      *(_DWORD *)(v11 + 12) = 0;
      *(_DWORD *)(v11 + 8) = 0;
      *(_DWORD *)(v11 + 4) = 0;
    }
    if ( a1 )
    {
      *(_DWORD *)(v11 + 232) = *a1;
      *(_DWORD *)(v11 + 236) = a1[1];
      *(_DWORD *)(v11 + 240) = a1[2];
    }
    else
    {
      *(_DWORD *)(v11 + 240) = 0;
      *(_DWORD *)(v11 + 236) = 0;
      *(_DWORD *)(v11 + 232) = 0;
    }
    if ( a3 )
    {
      *(_DWORD *)(v11 + 244) = *a3;
      *(_DWORD *)(v11 + 248) = a3[1];
      *(_DWORD *)(v11 + 252) = a3[2];
    }
    else
    {
      *(_DWORD *)(v11 + 252) = 0;
      *(_DWORD *)(v11 + 248) = 0;
      *(_DWORD *)(v11 + 244) = 0;
    }
    if ( (a7 & 0xC0) == 0x80 )
    {
      v12 = a4 * 0.0031415902;
    }
    else
    {
      if ( (a7 & 0xC0) == 0 )
      {
LABEL_21:
        *(_DWORD *)(v11 + 164) = a6;
        *(_DWORD *)(v11 + 24) = a7;
        cand_CFxPrimType13__helper((float *)v11);
        FX_AddPrimitive(a5, &v13);
        return v11;
      }
      v12 = (double)a5 * a4 * 0.0099999998 + (double)fxMTime;
    }
    *(float *)(v11 + 256) = v12;
    goto LABEL_21;
  }
  return v11;
}
