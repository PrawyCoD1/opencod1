/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include <malloc.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "tr_gl_types.h"
#include "tr_tess.h"

#include "../jpeg-6/jinclude.h"
#include "../jpeg-6/jpeglib.h"

int param         = 0x2701;   /* 0x005712F0  gl_filter_min, GL_LINEAR_MIPMAP_NEAREST */
int gl_filter_max  = 0x2601;   /* 0x005712F4  gl_filter_max, GL_LINEAR               */


void CoD_jpeg_mem_src( j_decompress_ptr cinfo, const unsigned char *inbuffer );

int __fastcall CompareMergableShaders( int a1, int a2, int a3, int a4 );
extern int GL_Bind();
extern int __cdecl GL_CheckErrors( const char *location );
extern int R_FindShader();
float VectorNormalize2( const float *v, float *out );

char __cdecl R_LoadImage( const char *name, int *data, unsigned __int16 *width, unsigned __int16 *height, int *a4, _DWORD *a5, int flags );

extern int j__atol();
extern int jinit_compress_master();
extern int qsort_m();
extern void GLimp_SetGamma( unsigned char *red, unsigned char *green, unsigned char *blue );


typedef struct image_s
{
	char            imgName[64];    /* +0x00  strcpy in R_AllocImage        */
	unsigned short  width;          /* +0x40                                */
	unsigned short  height;         /* +0x42                                */
	unsigned short  uploadWidth;    /* +0x44  out-param of UploadImage      */
	unsigned short  uploadHeight;   /* +0x46                                */
	int             cardMemory;     /* +0x48  += level cost                 */
	int             textureMemory;  /* +0x4C                                */
	int             imageTrack;     /* +0x50                                */
	unsigned int    target;         /* +0x54  glBindTexture(target,0)       */
	unsigned int    texnum;         /* +0x58  = index + 1024                */
	int             frameUsed;      /* +0x5C                                */
	unsigned int    internalFormat; /* +0x60  out-param of UploadImage      */
	unsigned int    flags;          /* +0x64  IMAGE_FLAG_*                  */
	void           *link;           /* +0x68  sheet image | delayed shader  */
	int             state0;         /* +0x6C  sheet x/y | delay group       */
	int             state1;         /* +0x70  sheet rotated | groupTriCount */
	struct image_s *hashNext;       /* +0x74                                */
} image_t;

#define IMAGE_FLAG_MIPMAP            0x0001
#define IMAGE_FLAG_ALLOW_PICMIP      0x0002
#define IMAGE_FLAG_USE_PICMIP2       0x0004
#define IMAGE_FLAG_LIGHTMAP          0x0008
#define IMAGE_FLAG_CLAMP_S           0x0010
#define IMAGE_FLAG_CLAMP_T           0x0020
#define IMAGE_FLAG_HEIGHT_TO_NORMAL  0x0040
#define IMAGE_FLAG_DELAYED_UPLOAD    0x0080
#define IMAGE_FLAG_NO_TEXTURE_SHEET  0x0100
#define IMAGE_FLAG_COLOR_DEPTH       0x0200
#define IMAGE_FLAG_NO_OVERBRIGHT     0x0400

#define R_MAX_IMAGES            2048   /* R_AllocImage cap test, 0x004EC58D */
#define R_IMAGE_HASH_SIZE       4096
#define R_IMAGE_FIRST_TEXTURE   1024
#define R_IMAGE_NAME_SIZE         64
#define R_MAX_IMAGE_ALLOCATIONS   32

#define tr_images ( (image_t **)(void *)dword_16C9840 )

#define imageHashTable ( (image_t **)(void *)hashtable )

unsigned char s_gammatable[ 256 ];
unsigned char s_intensitytable[ 256 ];

#define R_IMAGE_ALLOCATION_FILE        0
#define R_IMAGE_ALLOCATION_TEMP_MEMORY 1

typedef struct
{
	int   kind;
	void *memory;
} imageAllocation_t;

imageAllocation_t r_imageAllocations[ R_MAX_IMAGE_ALLOCATIONS ];

static const unsigned char mipBlendColors[16][4] = {
	{   0,   0,   0,   0 },
	{ 255,   0,   0, 128 },
	{   0, 255,   0, 128 },
	{   0,   0, 255, 128 },
	{ 255, 255,   0, 128 },
	{   0, 255, 255, 128 },
	{ 255,   0, 255, 128 },
	{ 255,   0,   0, 128 },
	{   0, 255,   0, 128 },
	{   0,   0, 255, 128 },
	{ 255, 128,   0, 128 },
	{   0, 255, 128, 128 },
	{ 128,   0, 255, 128 },
	{ 255,   0,   0, 128 },
	{   0, 255,   0, 128 },
	{   0,   0, 255, 128 }
};

#define ri_Hunk_Alloc(sz) \
	( (void *)( (*(int (__cdecl **)(int))&ri_Hunk_Alloc)( (int)(sz) ) ) )
#define ri_Hunk_AllocateTempMemory(sz) \
	( (void *)( (*(int (__cdecl **)(int))&ri_Hunk_AllocateTempMemory)( (int)(sz) ) ) )
#define ri_Hunk_FreeTempMemory(p) \
	( (*(void (__cdecl **)(void *))&ri_Hunk_FreeTempMemory)( (p) ) )
#define ri_FS_ReadFile(name, buf) \
	( (*(int (__cdecl **)(const char *, void **))&ri_FS_ReadFile)( (name), (buf) ) )
#define ri_FS_FreeFile(p) \
	( (*(void (__cdecl **)(void *))&ri_FS_FreeFile)( (p) ) )

#define R_CVAR(g) ( *(cvar_t **)&(g) )

int  __cdecl R_PicmipForImageFlags( int flags );
void __cdecl R_RememberImageAllocation( void *memory, int kind );

/* ---- R_PicmipForImageFlags  0x004EA380 ----  VERIFIED */
int __cdecl R_PicmipForImageFlags( int flags )
{
	int picmip;

	if ( flags & IMAGE_FLAG_USE_PICMIP2 )
		picmip = R_CVAR( r_picmip2 )->integer;
	else
		picmip = R_CVAR( r_picmip )->integer;

	if ( !( flags & IMAGE_FLAG_ALLOW_PICMIP ) )
		return 0;

	if ( picmip > 3 )
		picmip = 3;
	if ( picmip < 0 )
		picmip = 0;
	return picmip;
}

/* ---- R_RememberImageAllocation  0x004EA3C0 ----  VERIFIED */
void __cdecl R_RememberImageAllocation( void *memory, int kind )
{
	r_imageAllocations[ dword_14072F0 ].kind   = kind;
	r_imageAllocations[ dword_14072F0 ].memory = memory;
	++dword_14072F0;
}

/* ---- R_ResetImageAllocations  0x004EA3E0 ----  VERIFIED */
void R_ResetImageAllocations( void )
{
	dword_14072F0 = 0;
}

/* ---- R_FreeImageAllocations  0x004EA3F0 ----  VERIFIED */
void R_FreeImageAllocations( void )
{
	while ( dword_14072F0 > 0 )
	{
		imageAllocation_t *a = &r_imageAllocations[ --dword_14072F0 ];

		if ( a->kind == R_IMAGE_ALLOCATION_FILE )
			ri_FS_FreeFile( a->memory );
		else if ( a->kind == R_IMAGE_ALLOCATION_TEMP_MEMORY )
			ri_Hunk_FreeTempMemory( a->memory );
	}
}

/* ---- R_AllocTempMemory  0x004EA440 ----  VERIFIED */
void * __cdecl R_AllocTempMemory( int size )
{
	void *memory = ri_Hunk_AllocateTempMemory( size );

	if ( memory )
		R_RememberImageAllocation( memory, R_IMAGE_ALLOCATION_TEMP_MEMORY );
	return memory;
}

/* ---- R_ReadFile  0x004EA470 ----  VERIFIED */
int __cdecl R_ReadFile( const char *name, void **buffer )
{
	int len = ri_FS_ReadFile( name, buffer );

	if ( len >= 0 )
		R_RememberImageAllocation( *buffer, R_IMAGE_ALLOCATION_FILE );
	return len;
}

/* ---- R_GammaCorrect  0x004EA4B0 ----  VERIFIED */
void __cdecl R_GammaCorrect( unsigned char *buffer, int bytes )
{
	int i;

	for ( i = 0; i < bytes; ++i )
		buffer[ i ] = s_gammatable[ buffer[ i ] ];
}

/* ---- tr_image_generateHashValue  0x004EA4E0 ----  [HIGH] */
int __cdecl tr_image_generateHashValue(char *a1)
{
  char *v1;
  __int16 v2;
  __int16 v3;
  int v4;
  __int16 v5;
  char v6;

  v1 = a1;
  v2 = 0;
  if ( *a1 )
  {
    v3 = 119 - (_WORD)a1;
    do
    {
      v4 = tolower(*v1);
      if ( v4 == 46 )
        break;
      if ( v4 == 92 )
        LOWORD(v4) = 47;
      v5 = v4 * (v3 + (_WORD)v1);
      v6 = v1[1];
      v2 += v5;
      ++v1;
    }
    while ( v6 );
  }
  return v2 & 0xFFF;
}

typedef struct textureMode_s
{
	const char *name;
	int         minimize;
	int         maximize;
} textureMode_t;

static const textureMode_t r_textureModes[6] =      /* 0x005712F8 */
{
	{ "GL_NEAREST",                0x2600, 0x2600 },
	{ "GL_LINEAR",                 0x2601, 0x2601 },
	{ "GL_NEAREST_MIPMAP_NEAREST", 0x2700, 0x2600 },
	{ "GL_LINEAR_MIPMAP_NEAREST",  0x2701, 0x2601 },
	{ "GL_NEAREST_MIPMAP_LINEAR",  0x2702, 0x2600 },
	{ "GL_LINEAR_MIPMAP_LINEAR",   0x2703, 0x2601 }
};

static const char *const r_imageTrackNames[11] =    /* 0x00571340 */
{
	"misc  ", "debug ", "ui    ", "lmap  ", "effect", "hud   ",
	"vmodel", "model ", "world ", "f/x   ", "$tex+?"
};

/* ---- GL_TextureMode  0x004EA530 ----  [HIGH] */
void __cdecl GL_TextureMode(int a1)
{
  int v2;
  char **v3;
  int v4;
  int v5;
  int v6;
  int v7;

  v2 = 0;
  v3 = 0;
  do
  {
    if ( r_textureModes[v2].name && a1
      && !Q_stricmpn( (const char *)a1, r_textureModes[v2].name, 99999 ) )
      break;
    ++v2;
  }
  while ( v2 < 6 );
  if ( v2 == 6 )
  {
    ri_Printf(0, "bad filter name\n");
  }
  else
  {
    v4 = r_textureModes[v2].minimize;
    v5 = r_textureModes[v2].maximize;
    v6 = 0;
    param = v4;
    for ( gl_filter_max = v5; v6 < tr_numImages; ++v6 )
    {
      v7 = dword_16C9840[v6];
      if ( (*(_BYTE *)(v7 + 100) & 1) != 0 )
      {
        GL_Bind((GLenum *)v7);
        glTexParameteri(*(_DWORD *)(v7 + 84), 0x2801u, param);
        glTexParameteri(*(_DWORD *)(v7 + 84), 0x2800u, gl_filter_max);
      }
    }
  }
}

/* ---- R_SumOfUsedImages  0x004EA600 ----  VERIFIED */
_DWORD *__cdecl R_SumOfUsedImages(int *a1, _DWORD *a2, _DWORD *a3)
{
  int v3;
  int v4;
  int v5;
  _DWORD *v6;
  int v7;
  int v8;
  _DWORD *result;

  if ( r_usedImageFrame == tr_frameCount )
    goto LABEL_19;
  v3 = 0;
  v4 = 0;
  v5 = 0;
  r_usedImageFrame = tr_frameCount;
  r_usedImageMemory = 0;
  r_usedLightmapMemory = 0;
  r_usedTextureMemory = 0;
  if ( tr_numImages > 0 )
  {
    do
    {
      v6 = (_DWORD *)dword_16C9840[v5];
      v7 = v6[23];
      if ( v7 == tr_frameCount || v7 == tr_frameCount - 1 )
      {
        v8 = v6[18];
        v3 += v8;
        if ( v6[20] == 3 )
          r_usedLightmapMemory += v8;
        v4 += v6[19];
      }
      ++v5;
    }
    while ( v5 < tr_numImages );
    r_usedTextureMemory = v4;
    r_usedImageMemory = v3;
  }
  if ( com_statmon->integer && v4 > 0x1400000 )
  {
    if ( !sys_timeBaseInit )
    {
      sys_timeBase = timeGetTime();
      sys_timeBaseInit = 1;
    }
    dword_8E1C00 = 3000 - sys_timeBase + timeGetTime();
    if ( !dword_8E1C04 )
    {
      if ( cls_rendererStarted )
        dword_8E1C04 = Material_RegisterHandle("gfx/2d/warning@textures.jpg", 1);
    }
LABEL_19:
    v3 = r_usedImageMemory;
    v4 = r_usedTextureMemory;
  }
  if ( a1 )
    *a1 = v3;
  if ( a2 )
    *a2 = r_usedLightmapMemory;
  result = a3;
  if ( a3 )
    *a3 = v4;
  return result;
}

/* ---- imagecompare  0x004EA750 ----  [HIGH] */
int __cdecl imagecompare(_DWORD *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;

  v2 = dword_16C9840[*a1];
  v3 = *(_DWORD *)(v2 + 80);
  v4 = dword_16C9840[*a2];
  v5 = *(_DWORD *)(v4 + 80);
  if ( v3 < v5 )
    return -1;
  if ( v3 <= v5 )
    return *(_DWORD *)(v2 + 72) - *(_DWORD *)(v4 + 72);
  return 1;
}

/* ---- RE_GetImageMemory  0x004EA790 ----  [HIGH] */
int RE_GetImageMemory()
{
  return tr_imageMemory;
}

static const unsigned char r_imageListClampCase[49] =   /* 0x004EAA50 */
{
  0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  3
};

/* ---- R_ImageList_f  0x004EA7A0 ----  VERIFIED */
void R_ImageList_f()
{
  signed int v0;
  signed int v1;
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  bool v13; // cc
  int i;
  const char *v15;
  int v16;
  _DWORD v17[2];
  _DWORD v18[11]; // [esp+28h] [ebp-202Ch] BYREF
  _DWORD v19[2048]; // [esp+54h] [ebp-2000h] BYREF

  v0 = 0;
  memset(v18, 0, sizeof(v18));
  v1 = tr_numImages;
  v17[0] = (int)"no ";
  v17[1] = (int)"yes";
  v16 = 0;
  if ( tr_numImages > 0 )
  {
    do
    {
      v19[v0] = v0;
      ++v0;
    }
    while ( v0 < v1 );
  }
  qsort_m((unsigned int)v19, v1, 4u, (int (__cdecl *)(unsigned int, _BYTE *))imagecompare);
  ri_Printf(0, "\n      -w-- -h-- -mm- -if--- wrap- -type- -KB--- --name-------\n");
  v2 = 0;
  v3 = 0;
  if ( tr_numImages > 0 )
  {
    do
    {
      v4 = dword_16C9840[v19[v3]];
      v5 = *(unsigned __int16 *)(v4 + 70);
      v2 += v5 * *(unsigned __int16 *)(v4 + 68);
      ri_Printf(
        0,
        "%4i: %4i %4i  %s  ",
        v19[v3],
        *(unsigned __int16 *)(v4 + 68),
        v5,
        (const char *)v17[*(_DWORD *)(v4 + 100) & 1]);
      v6 = *(_DWORD *)(v4 + 96);
      if ( v6 > 32849 )
      {
        if ( v6 > 33777 )
        {
          v11 = v6 - 33778;
          if ( !v11 )
          {
            v15 = "DXT3 ";
            goto LABEL_33;
          }
          if ( v11 == 1 )
          {
            v15 = "DXT5 ";
            goto LABEL_33;
          }
LABEL_30:
          v15 = "?????";
          goto LABEL_33;
        }
        if ( v6 >= 33776 )
        {
          v15 = "DXT1 ";
        }
        else
        {
          v9 = v6 - 32854;
          if ( v9 )
          {
            v10 = v9 - 2;
            if ( v10 )
            {
              if ( v10 != 841 )
                goto LABEL_30;
              v15 = "S3TC4";
            }
            else
            {
              v15 = "RGBA8";
            }
          }
          else
          {
            v15 = "RGBA4";
          }
        }
      }
      else
      {
        if ( v6 == 32849 )
        {
          v15 = "RGB8 ";
          goto LABEL_33;
        }
        if ( v6 > 4 )
        {
          if ( v6 == 6408 )
          {
LABEL_17:
            v15 = "RGBA ";
            goto LABEL_33;
          }
          if ( v6 != 32848 )
            goto LABEL_30;
          v15 = "RGB5 ";
        }
        else
        {
          if ( v6 == 4 )
            goto LABEL_17;
          v7 = v6 - 1;
          if ( v7 )
          {
            v8 = v7 - 1;
            if ( v8 )
            {
              if ( v8 != 1 )
                goto LABEL_30;
              v15 = "RGB  ";
            }
            else
            {
              v15 = "IA   ";
            }
          }
          else
          {
            v15 = "I    ";
          }
        }
      }
LABEL_33:
      ri_Printf(0, v15);
      switch ( r_imageListClampCase[*(_DWORD *)(v4 + 100) & 0x30] )
      {
        case 0:
          ri_Printf(0, " repeat ");
          break;
        case 1:
          ri_Printf(0, " clampx ");
          break;
        case 2:
          ri_Printf(0, " clampy ");
          break;
        case 3:
          ri_Printf(0, " clamp  ");
          break;
        case 4:
          break;
      }
      ri_Printf(
        0,
        " %s %6.1f %s\n",
        r_imageTrackNames[*(_DWORD *)(v4 + 80)],
        (double)*(int *)(v4 + 72) * 0.0009765625,
        (const char *)v4);
      v12 = *(_DWORD *)(v4 + 72);
      v13 = ++v3 < tr_numImages;
      v16 += v12;
      v18[*(_DWORD *)(v4 + 80)] += v12;
    }
    while ( v13 );
  }
  ri_Printf(0, " ---------\n");
  ri_Printf(0, " %i total texels (not including mipmaps)\n", v2);
  ri_Printf(0, " %i total images\n", tr_numImages);
  ri_Printf(0, " %.2f MB total image size\n\n", (double)v16 * 0.00000095367432);
  for ( i = 0; i < 11; ++i )
    ri_Printf(0, "%s: %.2f MB\n", r_imageTrackNames[i], (double)(int)v18[i] * 0.00000095367432);
}

/* ---- R_LightScaleTexture  0x004EAA90 ----  VERIFIED */
void __cdecl R_LightScaleTexture( unsigned char *pixels, int width, int height,
                                  int onlyGamma, int format )
{
	int count = width * height;
	int i;

	if ( format != 0x1908  )
		return;

	if ( onlyGamma )
	{
		if ( glConfig_deviceSupportsGamma )
			return;

		for ( i = 0; i < count; ++i, pixels += 4 )
		{
			pixels[0] = s_gammatable[ pixels[0] ];
			pixels[1] = s_gammatable[ pixels[1] ];
			pixels[2] = s_gammatable[ pixels[2] ];
		}
		return;
	}

	if ( glConfig_deviceSupportsGamma )
	{
		for ( i = 0; i < count; ++i, pixels += 4 )
		{
			pixels[0] = s_intensitytable[ pixels[0] ];
			pixels[1] = s_intensitytable[ pixels[1] ];
			pixels[2] = s_intensitytable[ pixels[2] ];
		}
		return;
	}

	for ( i = 0; i < count; ++i, pixels += 4 )
	{
		pixels[0] = s_gammatable[ s_intensitytable[ pixels[0] ] ];
		pixels[1] = s_gammatable[ s_intensitytable[ pixels[1] ] ];
		pixels[2] = s_gammatable[ s_intensitytable[ pixels[2] ] ];
	}
}

/* ---- GetCardMemoryAmount  0x004EABA0 ----  VERIFIED */
int __cdecl GetCardMemoryAmount(int a1, int a2, int a3)
{
  int result;
  int v4;

  if ( a2 <= 32849 )
  {
    if ( a2 == 32849 )
      return 4 * a3 * a1;
    if ( a2 <= 6408 )
    {
      if ( a2 != 6408 )
      {
        if ( a2 >= 3 )
        {
          if ( a2 > 4 )
          {
            if ( a2 == 6407 )
              return a3 * a1 * (2 * (glConfig_colorBits > 16) + 2);
            return 0;
          }
          return 4 * a3 * a1;
        }
        return 0;
      }
      return 4 * a3 * a1;
    }
    if ( a2 == 6409 )
      return a3 * a1;
    if ( a2 != 32848 )
      return 0;
    return 2 * a3 * a1;
  }
  if ( a2 <= 32993 )
  {
    if ( a2 >= 32992 )
      return 4 * a3 * a1;
    v4 = a2 - 32854;
    if ( v4 )
    {
      if ( v4 != 2 )
        return 0;
      return 4 * a3 * a1;
    }
    return 2 * a3 * a1;
  }
  switch ( a2 )
  {
    case 33776:   /* GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0 */
    case 33777:   /* GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1 */
      result = 8 * ( ( a3 + 3 ) / 4 ) * ( ( a1 + 3 ) / 4 );
      break;
    case 33778:   /* GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2 */
    case 33779:   /* GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3 */
      /* 16 * blocksX * blocksY.  0x004EAC6B-0x004EAC74. */
      result = 16 * ( ( a3 + 3 ) / 4 ) * ( ( a1 + 3 ) / 4 );
      break;
    default:
      return 0;
  }
  return result;
}

/* ---- R_MipMap2  0x004EAC90 ----  VERIFIED */
int __cdecl R_MipMap2(int a1, int a2, int a3)
{
  int v3;
  int v5;
  int *v6;
  int v7;
  int v8;
  _BYTE *v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  int v16;
  _BYTE *v17;
  _BYTE *v18;
  int v19;
  bool v20; // zf
  int v22;
  int v23;
  _BYTE *v24;
  _BYTE *v25;
  _BYTE *v26;
  _BYTE *v27;
  _BYTE *v28;
  _BYTE *v29;
  _BYTE *v30;
  _BYTE *v31;
  _BYTE *v32;
  _BYTE *v33;
  _BYTE *v34;
  _BYTE *v35;
  _BYTE *v36;
  int v37;
  _BYTE *v38;
  int v39;
  _BYTE *v40;
  int v41;
  int v42;
  int v43;
  int v44;
  int v45;
  int *v46;
  int v47;
  int v48;

  v3 = a3;
  v5 = a1 >> 1;
  v39 = a3 >> 1;
  v48 = 4 * (a3 >> 1) * (a1 >> 1);
  v6 = (int *)ri_Hunk_AllocateTempMemory(v48);
  v7 = a1 - 1;
  v46 = v6;
  v47 = v7;
  if ( v5 > 0 )
  {
    v8 = 2;
    v44 = 2;
    v40 = (char *)v6 + 2;
    v45 = v5;
    do
    {
      if ( v39 > 0 )
      {
        v22 = v3 * (v7 & (v8 - 2));
        v9 = v40;
        v23 = v3 * (v7 & v8);
        v10 = v3 * (v7 & (v8 - 1));
        v41 = v3 * (v7 & (v8 - 3));
        v11 = 2;
        v37 = v10;
        v42 = 2;
        v43 = a3 >> 1;
        while ( 1 )
        {
          v12 = v3 - 1;
          v13 = v12 & (v11 - 1);
          v27 = (_BYTE *)(a2 + 4 * (v10 + v13));
          v26 = (_BYTE *)(a2 + 4 * (v13 + v22));
          v14 = v12 & (v11 - 2);
          v25 = (_BYTE *)(a2 + 4 * (v14 + v10));
          v24 = (_BYTE *)(a2 + 4 * (v14 + v22));
          v15 = v12 & v11;
          v34 = (_BYTE *)(a2 + 4 * ((v12 & v11) + v37));
          v32 = (_BYTE *)(a2 + 4 * (v13 + v23));
          v16 = v12 & (v11 - 3);
          v31 = (_BYTE *)(a2 + 4 * (v16 + v37));
          v17 = (_BYTE *)(a2 + 4 * (v41 + v13));
          v33 = (_BYTE *)(a2 + 4 * (v15 + v22));
          v30 = (_BYTE *)(a2 + 4 * (v16 + v22));
          v29 = (_BYTE *)(a2 + 4 * (v14 + v23));
          v28 = (_BYTE *)(a2 + 4 * (v41 + v14));
          v36 = (_BYTE *)(a2 + 4 * (v41 + v15));
          v35 = (_BYTE *)(a2 + 4 * (v16 + v23));
          v38 = (_BYTE *)(a2 + 4 * (v15 + v23));
          v18 = (_BYTE *)(a2 + 4 * (v41 + v16));
          *(v9 - 2) = (*v18
                     + *v35
                     + *v36
                     + *v38
                     + 2
                     * (*v32 + *v31 + *v17 + *v30 + *v29 + *v28 + *v33 + *v34 + 2 * (*v26 + *v27 + *v25 + *v24)))
                    / 36;
          *(v9 - 1) = (v38[1]
                           + v36[1]
                           + v35[1]
                           + v18[1]
                           + 2
                           * (v34[1]
                            + v33[1]
                            + v32[1]
                            + v31[1]
                            + v17[1]
                            + v30[1]
                            + v29[1]
                            + v28[1]
                            + 2 * (v27[1] + v26[1] + v25[1] + v24[1])))
                    / 36;
          *v9 = (v38[2]
                     + v36[2]
                     + v35[2]
                     + v18[2]
                     + 2
                     * (v34[2]
                      + v33[2]
                      + v32[2]
                      + v31[2]
                      + v17[2]
                      + v30[2]
                      + v29[2]
                      + v28[2]
                      + 2 * (v27[2] + v26[2] + v25[2] + v24[2])))
              / 36;
          v19 = (unsigned __int8)v36[3]
              + (unsigned __int8)v35[3]
              + (unsigned __int8)v18[3]
              + 2
              * ((unsigned __int8)v34[3]
               + (unsigned __int8)v33[3]
               + (unsigned __int8)v32[3]
               + (unsigned __int8)v31[3]
               + (unsigned __int8)v17[3]
               + (unsigned __int8)v30[3]
               + (unsigned __int8)v29[3]
               + (unsigned __int8)v28[3]
               + 2
               * ((unsigned __int8)v27[3] + (unsigned __int8)v26[3] + (unsigned __int8)v25[3] + (unsigned __int8)v24[3]));
          v3 = a3;
          v9[1] = ((unsigned __int8)v38[3] + v19) / 36;
          v11 = v42 + 2;
          v9 += 4;
          v42 += 2;
          if ( !--v43 )
            break;
          v10 = v37;
        }
        v7 = v47;
        v8 = v44;
      }
      v8 += 2;
      v20 = v45 == 1;
      v44 = v8;
      v40 += 4 * v39;
      --v45;
    }
    while ( !v20 );
    v6 = v46;
  }
  Com_Memcpy(a2, v6, v48);
  ri_Hunk_FreeTempMemory(v46);
  return 0;
}

/* ---- R_MipMap8  0x004EB120 ----  VERIFIED */
int __cdecl R_MipMap8(int result, unsigned __int8 *a2, int a3)
{
  int v3;
  int v4;
  unsigned __int8 *v5;
  unsigned __int8 *v6;
  int v7;
  int v8;
  int v10;

  if ( a3 != 1 || result != 1 )
  {
    v3 = a3;
    v4 = a3 >> 1;
    result >>= 1;
    v5 = a2;
    v10 = v4;
    if ( v4 && result )
    {
      if ( result > 0 )
      {
        v8 = result;
        do
        {
          if ( v4 > 0 )
          {
            v6 = &a2[v3];
            do
            {
              *v5++ = (*a2 + *v6 + v6[1] + v6[1 - v3]) >> 2;
              a2 += 2;
              v6 += 2;
              --v4;
            }
            while ( v4 );
            v4 = v10;
            v3 = a3;
          }
          a2 += v3;
          result = --v8;
        }
        while ( v8 );
      }
    }
    else
    {
      v7 = result + v4;
      if ( v7 > 0 )
      {
        do
        {
          result = (*a2 + a2[1]) >> 1;
          *v5++ = result;
          a2 += 2;
          --v7;
        }
        while ( v7 );
      }
    }
  }
  return result;
}

/* ---- R_MipMap  0x004EB1D0 ----  VERIFIED */
unsigned __int8 *__fastcall R_MipMap(unsigned int format, unsigned __int8 *data,
                                     char *name, int width, int height, int flags)
{
	unsigned __int8 *src;              /* edi -- the row being consumed   */
	unsigned __int8 *out;              /* esi -- the halved row           */
	unsigned __int8 *next;             /* eax -- src + one source row     */
	char            *hash;
	char            *digits;
	const char      *scan;
	int              rowBytes;         /* ebx = 4 * width                 */
	int              halfWidth;
	int              halfHeight;
	int              rows, cols, n, c, sum;
	float            weight, blend;

	int              loadedData;
	unsigned __int16 loadedWidth;
	unsigned __int16 loadedHeight;
	int              loadedSpare;
	unsigned __int8 *loadedEnd;
	char             tail[64];

	if ( format > 0x1909 )
	{
		if ( format < 0x83F0 || format > 0x83F3 )
			return data;
		return &data[ GetCardMemoryAmount( width, format, height ) ];
	}

	if ( format == 0x1909 )
	{
		R_MipMap8( height, data, width );
		return data;
	}

	if ( format != 0x1908 )
		return data;

	hash = strstr( name, "#" );
	if ( hash )
	{
		digits = hash + 1;
		scan   = hash + 1;
		if ( isdigit( hash[1] ) )
		{
			do
				c = *++scan;
			while ( isdigit( c ) );
		}
		strcpy( tail, scan );                    /* inlined in retail at 0x004EB260 */
		*(char *)scan = 0;
		sprintf( digits, "%i%s", j__atol( digits ) + 1, tail );

		R_LoadImage( name, &loadedData, &loadedWidth, &loadedHeight,
		             &loadedSpare, (_DWORD *)&loadedEnd, flags );

		if ( loadedData )
		{
			if ( loadedWidth  != (unsigned __int16)( ( width  + 1 ) >> 1 )
			  || loadedHeight != (unsigned __int16)( ( height + 1 ) >> 1 ) )
			{
				ri_Printf( 2,
				           "WARNING: custom mipmap image '%s' should have dimensions %i x %i instead of %i x %i, refusing to load\n",
				           name,
				           ( width  + 1 ) >> 1,
				           ( height + 1 ) >> 1,
				           loadedWidth,
				           loadedHeight );
				return 0;
			}
			Com_Memcpy( data, (void *)loadedData, 4 * loadedWidth * loadedHeight );
			return data;
		}
	}

	if ( !r_simpleMipMaps->integer )
	{
		R_MipMap2( height, data, width );
		return data;
	}

	if ( width == 1 && height == 1 )
		return data;

	rowBytes   = 4 * width;
	halfWidth  = width  >> 1;
	halfHeight = height >> 1;
	src = data;
	out = data;

	if ( halfWidth && halfHeight )
	{
		weight = r_weightMipMaps->value;

		if ( weight <= 0.0 || weight > 1.0 )
		{
			/* Plain 2x2 box filter, `sar ecx,2`.  0x004EB9A0-0x004EBAAF. */
			if ( halfHeight > 0 )
			{
				rows = halfHeight;
				do
				{
					if ( halfWidth > 0 )
					{
						next = &src[ rowBytes ];
						cols = halfWidth;
						do
						{
							out[0] = (unsigned __int8)( ( next[0] + next[4] + src[0] + src[4] ) >> 2 );
							out[1] = (unsigned __int8)( ( next[1] + next[5] + src[1] + src[5] ) >> 2 );
							out[2] = (unsigned __int8)( ( next[2] + next[6] + src[2] + src[6] ) >> 2 );
							out[3] = (unsigned __int8)( ( next[3] + next[7] + src[3] + src[7] ) >> 2 );
							out  += 4;
							src  += 8;
							next += 8;
						}
						while ( --cols );
					}
					src += rowBytes;
				}
				while ( --rows );
			}
		}
		else
		{
			blend = ( 1.0f - weight ) * 0.25f;

			if ( halfHeight > 0 )
			{
				rows = halfHeight;
				do
				{
					if ( halfWidth > 0 )
					{
						next = &src[ rowBytes ];
						cols = halfWidth;
						do
						{
							for ( c = 0; c < 4; ++c )
							{
								sum = next[c] + src[c] + next[c + 4] + src[c + 4];
								out[c] = (unsigned __int8)( (float)sum * blend
								                          + (float)src[c] * weight );
							}
							out  += 4;
							src  += 8;
							next += 8;
						}
						while ( --cols );
					}
					src += rowBytes;
				}
				while ( --rows );
			}
		}
		return data;
	}

	n = halfWidth + halfHeight;
	if ( n > 0 )
	{
		do
		{
			out[0] = (unsigned __int8)( ( src[0] + src[4] ) >> 1 );
			out[1] = (unsigned __int8)( ( src[1] + src[5] ) >> 1 );
			out[2] = (unsigned __int8)( ( src[2] + src[6] ) >> 1 );
			out[3] = (unsigned __int8)( ( src[3] + src[7] ) >> 1 );
			out += 4;
			src += 8;
		}
		while ( --n );
	}
	return data;
}

/* ---- R_BlendOverTexture_RGBA  0x004EBB90 ----  VERIFIED */
unsigned __int8 *__cdecl R_BlendOverTexture_RGBA(
        unsigned __int8 *result,
        int a2,
        unsigned __int8 *a3)
{
  int v3;  /* alpha */
  int v4;  /* alpha * colour[0] */
  int v5;  /* alpha * colour[1] */
  int v7;  /* alpha * colour[2] */
  int v8;  /* 255 - alpha */
  int v9;

  v3 = a3[3];
  v4 = v3 * *a3;
  v5 = v3 * a3[1];
  v7 = v3 * a3[2];
  v8 = 255 - v3;
  if ( a2 > 0 )
  {
    v9 = a2;
    do
    {
      *result = (unsigned __int8)( ( v4 + v8 * *result ) >> 8 );
      result[1] = (unsigned __int8)( ( v5 + v8 * result[1] ) >> 8 );
      result[2] = (unsigned __int8)( ( v7 + v8 * result[2] ) >> 8 );
      result += 4;
      --v9;
    }
    while ( v9 );
  }
  return result;
}

/* ---- R_BlendOverTexture_S3TC  0x004EBC00 ----  VERIFIED */
unsigned int __cdecl R_BlendOverTexture_S3TC(int a1, _BYTE *a2, int a3, int a4)
{
  unsigned int result;
  unsigned int v6;
  unsigned int v7;
  int v8;
  int v9;
  int i;
  __int16 v11;
  int v12;
  int v13;
  int v14; // kr00_4
  int v15;
  bool v16; // zf
  _WORD v17[2]; // [esp+10h] [ebp-34h] BYREF
  int v18;
  int v19;
  unsigned int v20;
  _DWORD v21[7];
  unsigned int v22;

  result = (unsigned __int8)a2[3];
  v6 = result * ((*a2 & 0xF8) << 8);
  v7 = result * ((unsigned __int8)a2[2] >> 3);
  v8 = 255 - result;
  v22 = result * 8 * (a2[1] & 0xFC);
  if ( a1 > 0 )
  {
    v18 = a3 - (_DWORD)v17;
    v20 = ((unsigned int)(a1 - 1) >> 4) + 1;
    do
    {
      v9 = 0;
      for ( i = 0; i < 2; ++i )
      {
        v11 = *(_WORD *)((char *)&v17[v9] + v18);
        v17[v9] = v11;
        v21[i + 4] = ((int)(v6 + v8 * (v11 & 0xF800)) >> 8) & 0xF800;
        v12 = ((int)(v7 + v8 * (v11 & 0x1F)) >> 8) & 0x1F;
        v13 = ((int)(v22 + v8 * (v11 & 0x7E0)) >> 8) & 0x7E0;
        v21[i + 2] = v12;
        LOWORD(v12) = LOWORD(v21[i + 2]) | v13;
        v21[i] = v13;
        LOWORD(v12) = LOWORD(v21[i + 4]) | v12;
        *(_WORD *)((char *)&v19 + v9 * 2) = v12;
        ++v9;
      }
      v14 = v19;
      if ( a4 == 8 || v17[1] < v17[0] == HIWORD(v19) < (unsigned __int16)v19 )
      {
        v15 = a3;
        *(_WORD *)a3 = v19;
        *(_WORD *)(a3 + 2) = HIWORD(v14);
      }
      else
      {
        v15 = a3;
        *(_WORD *)a3 = HIWORD(v19);
        *(_WORD *)(a3 + 2) = v14;
        *(_DWORD *)(a3 + 4) ^= ((unsigned int)~*(_DWORD *)(a3 + 4) >> 1) & 0x55555555;
      }
      a3 = a4 + v15;
      result = v20 - 1;
      v16 = v20 == 1;
      v18 += a4;
      --v20;
    }
    while ( !v16 );
  }
  return result;
}

/* ---- R_BlendOverTexture  0x004EBD60 ----  VERIFIED */
unsigned __int8 *__cdecl R_BlendOverTexture(
        unsigned __int8 *result,
        unsigned __int8 *a2,
        unsigned __int8 *a3,
        unsigned int a4)
{
  if ( a4 > 0x83F1 )
  {
    if ( a4 <= 0x83F3 )
      return (unsigned __int8 *)R_BlendOverTexture_S3TC((int)result, a2, (int)(a3 + 8), 16);
  }
  else if ( a4 >= 0x83F0 )
  {
    return (unsigned __int8 *)R_BlendOverTexture_S3TC((int)result, a2, (int)a3, 8);
  }
  else if ( a4 == 6408 )
  {
    return R_BlendOverTexture_RGBA(a3, (int)result, a2);
  }
  return result;
}

/* ---- R_TexImage2D  0x004EBDC0 ----  VERIFIED */
void __cdecl R_TexImage2D( unsigned int target, int level,
                           unsigned int internalFormat, int width, int height,
                           unsigned int format, const void *pixels )
{
	qglGetError_0();

	if ( internalFormat >= 0x83F0u && internalFormat <= 0x83F3u )
	{
		qglCompressedTexImage2DARB(
			target, level, internalFormat, width, height, 0,
			GetCardMemoryAmount( width, (int)internalFormat, height ),
			pixels );
	}
	else
	{
		glTexImage2D( target, level, (int)internalFormat, width, height, 0,
		              format, 0x1401u , pixels );
	}

	qglGetError_0();
}

/* ---- PickInternalFormat  0x004EBE30 ----  VERIFIED */
int __cdecl PickInternalFormat( const unsigned char *pixels, int format,
                                int width, int height, int flags,
                                int isLightmap )
{
	int count;
	int i;
	int bits;

	if ( format != 0x1908  )
		return format;

	if ( isLightmap )
		return 0x8058;

	if ( flags & IMAGE_FLAG_COLOR_DEPTH )
	{
		switch ( glConfig_colorBits )
		{
		case 15:
		case 16:
			return 0x8050;
		case 24:
			return 0x1907;
		case 32:
			return 0x1908;
		default:
			break;
		}
	}

	count = width * height;
	for ( i = 0; i < count; ++i )
	{
		if ( pixels[ 4 * i + 3 ] != 0xFF )
		{
			bits = R_CVAR( r_texturebits )->integer;
			if ( bits == 16 )
				return 0x8056;
			if ( bits == 32 )
				return 0x8058;
			return 4;
		}
	}

	bits = R_CVAR( r_texturebits )->integer;
	if ( bits == 16 )
		return 0x8050;
	if ( bits == 32 )
		return 0x8051;
	return 3;
}

/* ---- UploadImage  0x004EBF20 ----  VERIFIED */
int __cdecl UploadImage( const char *name, unsigned char *pixels,
                         unsigned int textureTarget, unsigned int uploadTarget,
                         unsigned int format, int width, int height,
                         int flags, int isLightmap,
                         unsigned int *internalFormat,
                         unsigned short *uploadWidth,
                         unsigned short *uploadHeight,
                         int *cardMemory, int *textureMemory )
{
	char mipName[ 64 ];
	int  sourceWidth  = width;
	int  sourceHeight = height;
	int  scaledWidth;
	int  scaledHeight;
	int  picmip;
	int  textureMipSkips;
	int  selectedFormat;
	int  mipmap;
	int  levelCost;
	int  level;

	strcpy( mipName, name );
	*cardMemory    = 0;
	*textureMemory = 0;

	if ( ( ( width - 1 ) & width ) != 0 || ( ( height - 1 ) & height ) != 0 )
	{
		ri_Printf( 2, "WARNING: image '%s' is %i x %i, which is not a power of 2 on both sides\n",
		           name, width, height );
		return 0;
	}

	picmip = R_PicmipForImageFlags( flags );

	textureMipSkips = 0;
	if ( flags & IMAGE_FLAG_ALLOW_PICMIP )
	{
		textureMipSkips = 2 - picmip;
		if ( textureMipSkips < 0 )
			textureMipSkips = 0;
	}

	scaledWidth  = width  >> picmip;
	scaledHeight = height >> picmip;
	while ( scaledWidth > glConfig_maxTextureSize || scaledHeight > glConfig_maxTextureSize )
	{
		scaledWidth  >>= 1;
		scaledHeight >>= 1;
	}
	if ( scaledWidth < 1 )
		scaledWidth = 1;
	if ( scaledHeight < 1 )
		scaledHeight = 1;

	selectedFormat = PickInternalFormat( pixels, (int)format, width, height,
	                                     flags, isLightmap );
	mipmap = ( flags & IMAGE_FLAG_MIPMAP ) != 0;

	if ( scaledWidth == width && scaledHeight == height && !mipmap )
	{
		R_TexImage2D( uploadTarget, 0, (unsigned int)selectedFormat,
		              scaledWidth, scaledHeight, format, pixels );

		*uploadWidth    = (unsigned short)scaledWidth;
		*uploadHeight   = (unsigned short)scaledHeight;
		*internalFormat = (unsigned int)selectedFormat;

		levelCost = GetCardMemoryAmount( scaledWidth, selectedFormat, scaledHeight );
		*cardMemory    += levelCost;
		*textureMemory += levelCost;
	}
	else
	{
		while ( sourceWidth > scaledWidth || sourceHeight > scaledHeight )
		{
			pixels = R_MipMap( format, pixels, mipName, sourceWidth, sourceHeight, 1 );
			if ( !pixels )
				return 0;

			sourceWidth  >>= 1;
			sourceHeight >>= 1;
			if ( sourceWidth < 1 )
				sourceWidth = 1;
			if ( sourceHeight < 1 )
				sourceHeight = 1;
		}

		R_LightScaleTexture( pixels, scaledWidth, scaledHeight, !mipmap, (int)format );

		*uploadWidth    = (unsigned short)scaledWidth;
		*uploadHeight   = (unsigned short)scaledHeight;
		*internalFormat = (unsigned int)selectedFormat;

		levelCost = GetCardMemoryAmount( scaledWidth, selectedFormat, scaledHeight );
		*cardMemory += levelCost;

		if ( textureMipSkips == 0 )
		{
			*textureMemory += levelCost;
		}
		else if ( mipmap )
		{
			--textureMipSkips;
		}
		else
		{
			*textureMemory += levelCost >> ( 2 * textureMipSkips );
		}

		R_TexImage2D( uploadTarget, 0, (unsigned int)selectedFormat,
		              scaledWidth, scaledHeight, format, pixels );

		if ( mipmap )
		{
			level = 0;

			while ( scaledWidth > 1 || scaledHeight > 1 )
			{
				pixels = R_MipMap( format, pixels, mipName, scaledWidth, scaledHeight, 1 );
				if ( !pixels )
					return 0;

				scaledWidth  >>= 1;
				scaledHeight >>= 1;
				if ( scaledWidth < 1 )
					scaledWidth = 1;
				if ( scaledHeight < 1 )
					scaledHeight = 1;

				++level;

				levelCost = GetCardMemoryAmount( scaledWidth, selectedFormat, scaledHeight );
				*cardMemory += levelCost;
				if ( textureMipSkips )
					--textureMipSkips;
				else
					*textureMemory += levelCost;

				if ( R_CVAR( r_colorMipLevels )->integer )
					R_BlendOverTexture( (unsigned char *)( scaledWidth * scaledHeight ),
					                    (unsigned char *)mipBlendColors[ level & 15 ],
					                    pixels, format );

				R_TexImage2D( uploadTarget, level, (unsigned int)selectedFormat,
				              scaledWidth, scaledHeight, format, pixels );
			}
		}
	}

	if ( mipmap )
	{
		glTexParameteri( textureTarget, 0x2801u , param );
		glTexParameteri( textureTarget, 0x2800u , gl_filter_max );
	}
	else
	{
		glTexParameteri( textureTarget, 0x2801u, 9729  );
		glTexParameteri( textureTarget, 0x2800u, 9729  );
	}

	GL_CheckErrors( "uploading an image" );   /* retail 0x004EC2B8: one push, the string */
	return 1;
}

/* ---- R_AllocImage  0x004EC310 ----  VERIFIED */
image_t * __cdecl R_AllocImage( const char *name, unsigned int target,
                                int width, int height, unsigned int flags,
                                int imageTrack )
{
	image_t *image;
	int      hash;

	if ( strlen( name ) >= R_IMAGE_NAME_SIZE )
		ri_Error( 1, "\025R_AllocImage: \"%s\" is too long\n", name );
	if ( tr_numImages == R_MAX_IMAGES )
		ri_Error( 1, "\025R_AllocImage: MAX_DRAWIMAGES hit\n" );

	image = (image_t *)ri_Hunk_Alloc( sizeof( *image ) );
	tr_images[ tr_numImages ] = image;
	image->texnum = (unsigned int)tr_numImages + R_IMAGE_FIRST_TEXTURE;
	++tr_numImages;

	strcpy( image->imgName, name );
	image->width         = (unsigned short)width;
	image->height        = (unsigned short)height;
	image->flags         = flags;
	image->target        = target;
	image->cardMemory    = 0;
	image->textureMemory = 0;
	image->imageTrack    = imageTrack;


	hash = tr_image_generateHashValue( (char *)name );
	image->hashNext = imageHashTable[ hash ];
	imageHashTable[ hash ] = image;
	return image;
}

/* ---- R_DeleteImage  0x004EC3F0 ----  VERIFIED */
void __cdecl R_DeleteImage( image_t *image )
{
	if ( (unsigned int)glState_currentTextures[ glState_currentTmu ] == image->texnum )
	{
		glBindTexture( image->target, 0 );
		glState_currentTextures[ glState_currentTmu ] = 0;
	}
	qglDeleteTextures( 1, &image->texnum );
}

/* ---- R_FreeImage  0x004EC430 ----  VERIFIED */
void __cdecl R_FreeImage( image_t *image )
{
	int hash = tr_image_generateHashValue( image->imgName );

	imageHashTable[ hash ] = image->hashNext;
	--tr_numImages;

	if ( (unsigned int)glState_currentTextures[ glState_currentTmu ] == image->texnum )
	{
		glBindTexture( image->target, 0 );
		glState_currentTextures[ glState_currentTmu ] = 0;
	}
	qglDeleteTextures( 1, &image->texnum );
}

/* ---- R_CreateImageInternal  0x004EC490 ----  VERIFIED */
int __cdecl R_CreateImageInternal( image_t *image, const float *colorScale,
                                   unsigned char *pixels,
                                   unsigned int uploadTarget,
                                   unsigned int format )
{
	int isLightmap;
	int cardMemory;
	int textureMemory;
	int count;
	int i;
	int c;

	isLightmap = ( strncmp( image->imgName, "*lightmap", 9 ) == 0 );


	if ( colorScale )
	{
		count = (int)image->width * (int)image->height;
		for ( i = 0; i < count; ++i )
		{
			unsigned char *p = pixels + 4 * i;
			for ( c = 0; c < 4; ++c )
			{
				double v = (double)p[ c ] * (double)colorScale[ c ];
				if ( !( v < 255.0 ) )
					v = 255.0;
				else if ( v < 0.0 )
					v = 0.0;
				p[ c ] = (unsigned char)v;
			}
		}
	}

	GL_Bind( image );

	cardMemory    = 0;
	textureMemory = 0;

	if ( !UploadImage( image->imgName, pixels, image->target, uploadTarget,
	                   format, (int)image->width, (int)image->height,
	                   (int)image->flags, isLightmap,
	                   &image->internalFormat,
	                   &image->uploadWidth, &image->uploadHeight,
	                   &cardMemory, &textureMemory ) )
	{
		return 0;
	}

	image->cardMemory    += cardMemory;
	image->textureMemory += textureMemory;
	tr_imageMemory        += cardMemory;

	glTexParameteri( image->target, 0x2802u ,
	                 ( image->flags & IMAGE_FLAG_CLAMP_S ) ? 33071 : 10497 );
	glTexParameteri( image->target, 0x2803u ,
	                 ( image->flags & IMAGE_FLAG_CLAMP_T ) ? 33071 : 10497 );
	return 1;
}

/* ---- R_CreateImage  0x004EC730 ----  VERIFIED */
int __cdecl R_CreateImage( char *name, int flags, int imageTrack,
                           unsigned __int8 *pixels, int width, int height,
                           int format, float *colorScale )
{
  _DWORD *v7;

  v7 = (_DWORD *)R_AllocImage(name, 3553, width, height, flags, imageTrack);
  if ( ( flags & 0x80 ) != 0 )          /* 0x004EC74B `test bl, bl / jns` */
  {
    if ( format == 6408 || width >= 4 && height >= 4 )
    {
      v7[24] = format;
      v7[26] = 0;
      v7[27] = tr_delayedImageGroup;
      v7[28] = tr_delayedImageGroupTriCount;
      return (int)v7;
    }
    v7[25] &= ~0x80u;
    --tr_delayedImageCount;
  }
  if ( !R_CreateImageInternal((int)v7, colorScale, pixels, 0xDE1u, format) )
  {
    R_FreeImage((int)v7);
    return 0;                           /* 0x004EC797 `xor esi, esi`      */
  }
  return (int)v7;
}

/* ---- TransposeDDSBlockDXT1  0x004EC7C0 ----  [HIGH] */
char __cdecl TransposeDDSBlockDXT1(int a1, int a2)
{
  unsigned __int8 v2;
  char v3;
  char result;

  *(_DWORD *)a2 = *(_DWORD *)a1;
  *(_BYTE *)(a2 + 4) = *(_BYTE *)(a1 + 4) & 3
                     | (4 * (*(_BYTE *)(a1 + 5) & 3 | (4 * ((4 * *(_BYTE *)(a1 + 7)) | *(_BYTE *)(a1 + 6) & 3))));
  *(_BYTE *)(a2 + 5) = *(_BYTE *)(a1 + 5) & 0xC
                     | (*(_BYTE *)(a1 + 4) >> 2) & 3
                     | (4 * (*(_BYTE *)(a1 + 6) & 0xC | (4 * (*(_BYTE *)(a1 + 7) & 0xFC))));
  *(_BYTE *)(a2 + 6) = *(_BYTE *)(a1 + 6) & 0x30
                     | (4 * (*(_BYTE *)(a1 + 7) & 0xF0))
                     | ((unsigned __int8)(*(_BYTE *)(a1 + 5) & 0x30 | (*(_BYTE *)(a1 + 4) >> 2) & 0xC) >> 2);
  v2 = *(_BYTE *)(a1 + 5) & 0xCF | (*(_BYTE *)(a1 + 4) >> 2);
  v3 = *(_BYTE *)(a1 + 6) & 0xC3;
  result = *(_BYTE *)(a1 + 7) & 0xC0;
  *(_BYTE *)(a2 + 7) = result | ((unsigned __int8)(v3 | (v2 >> 2)) >> 2);
  return result;
}

/* ---- TransposeDDSBlockDXT3  0x004EC870 ----  [HIGH] */
char __cdecl TransposeDDSBlockDXT3(_BYTE *a1, _BYTE *a2)
{
  char result;

  TransposeDDSBlockDXT1((int)(a2 + 8), (int)(a1 + 8));
  *a1 = *a2 & 0xF | (16 * a2[2]);
  a1[1] = (16 * a2[6]) | a2[4] & 0xF;
  a1[2] = (*a2 >> 4) | a2[2] & 0xF0;
  a1[3] = (a2[4] >> 4) | a2[6] & 0xF0;
  a1[4] = (16 * a2[3]) | a2[1] & 0xF;
  a1[5] = (16 * a2[7]) | a2[5] & 0xF;
  a1[6] = (a2[1] >> 4) | a2[3] & 0xF0;
  result = a2[5] >> 4;
  a1[7] = result | a2[7] & 0xF0;
  return result;
}

/* ---- TransposeDDSBlockDXT5  0x004EC910 ----  [HIGH] */
unsigned int __cdecl TransposeDDSBlockDXT5(_BYTE *a1, int a2)
{
  unsigned int result;
  unsigned int v3;
  int v4;
  unsigned int v5;

  TransposeDDSBlockDXT1((int)(a1 + 8), a2 + 8);
  LOBYTE(v3) = a1[2];
  BYTE2(v3) = a1[4];
  LOBYTE(v4) = a1[5];
  BYTE1(v3) = a1[3];
  BYTE2(v4) = a1[7];
  BYTE1(v4) = a1[6];
  v5 = 0xE001C0 & v4
     | (8 * (v3 & 0xE00 | ((v4 & 0xE00) << 6)))
     | ((0xE001C0 & v3 | ((v4 & 0x1C0000 | (v3 >> 6) & 0x7000) >> 3)) >> 6);
  *(_BYTE *)a2 = *a1;
  *(_BYTE *)(a2 + 1) = a1[1];
  *(_BYTE *)(a2 + 3) = (unsigned __int16)(v3 & 0x8007
                                        | (((v3 >> 6) & 0x1C0 | v4 & 0x7000) >> 3)
                                        | ((v4 & 0x8007 | (unsigned __int16)(8 * (v3 & 0x38 | ((v4 & 0x38) << 6)))) << 6)) >> 8;
  *(_BYTE *)(a2 + 4) = (v3 & 0x38007
                      | (((v3 >> 6) & 0x1C0 | v4 & 0x7000) >> 3)
                      | ((v4 & 0x38007 | (8 * (v3 & 0x38 | ((v4 & 0x38) << 6)))) << 6)) >> 16;
  *(_BYTE *)(a2 + 2) = v3 & 7
                     | (((v3 >> 6) & 0x1C0 | v4 & 0x7000) >> 3)
                     | ((v4 & 7 | (unsigned __int8)(8 * (v3 & 0x38 | ((v4 & 0x38) << 6)))) << 6);
  result = v5 >> 8;
  *(_WORD *)(a2 + 5) = v5;
  *(_BYTE *)(a2 + 7) = BYTE2(v5);
  return result;
}

/* ---- LoadDDS  0x004ECA30 ----  VERIFIED */
void __cdecl LoadDDS(_DWORD *a1, const char *a2, int *a3, _WORD *a4, _WORD *a5, _DWORD *a6, int a7)
{
  _DWORD *v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  unsigned int v13;
  unsigned int v14;
  int v15; // [esp+4h] [ebp-88h] BYREF
  int v16; // [esp+8h] [ebp-84h] BYREF
  _BYTE v17[128]; // [esp+Ch] [ebp-80h] BYREF

  if ( a7 )
  {
    v8 = ri_FS_ReadFile(a2, &v16);
    v9 = v16;
    if ( v8 >= 0 )
    {
      v10 = dword_14072F0;
      r_imageAllocations[dword_14072F0].kind = 0;
      *(int *)&r_imageAllocations[v10].memory = v9;
      dword_14072F0 = v10 + 1;
    }
    if ( v8 <= 128 )
      return;
    v7 = (_DWORD *)v9;
  }
  else
  {
    if ( ri_FS_FOpenFileByMode(a2, &v15, 0) <= 128 )
      return;
    ri_FS_Read(v17, 128, v15);
    ri_FS_FCloseFile(v15);
    v7 = v17;
  }
  if ( *v7 != 542327876 )
    return;
  v11 = v7[21];
  switch ( v11 )
  {
    case 827611204:
      *a1 = 33777;
      break;
    case 861165636:
      *a1 = 33778;
      break;
    case 894720068:
      *a1 = 33779;
      break;
    default:
      return;
  }
  if ( v7[1] == 124 )
  {
    v12 = v7[4];
    if ( ((v12 - 1) & v12) != 0 || ((v7[3] - 1) & v7[3]) != 0 )
    {
      ri_Printf(2, "WARNING: image '%s' is %i x %i, which is not a power of 2 on both sides\n", a2, v12, v7[3]);
      *a1 = 0;
    }
    else
    {
      *a6 = 0;
      v13 = v7[7];
      if ( v13 >= 2 )
      {
        v14 = v7[4];
        if ( v14 <= v7[3] )
          v14 = v7[3];
        if ( 1 << v13 != 2 * v14 )
        {
          ri_Printf(2, "WARNING: image '%s' does not have the correct number of mipmaps", a2);
          *a1 = 0;
          return;
        }
        *a6 = 1;
      }
      if ( v7[4] > 0x8000u || v7[3] > 0x8000u )
      {
        ri_Printf(2, "WARNING: image '%s' is greater than %i on one or more axes", a2, 0x8000);
        *a1 = 0;
      }
      else
      {
        *a4 = *((_WORD *)v7 + 8);
        *a5 = *((_WORD *)v7 + 6);
        if ( a7 )
          *a3 = (int)(v7 + 32);
      }
    }
  }
}

/* ---- LoadTGA  0x004ECC20 ----  [HIGH] */
char __cdecl LoadTGA( int a1, int *a2, unsigned __int16 *a3, unsigned __int16 *a4,
                      _DWORD *a5, int a6 )
{
	const char    *name = (const char *)a1;
	unsigned char  headerBytes[18];
	const unsigned char *hdr;
	const unsigned char *src = 0;
	void          *fileBuffer = 0;

	int idLength, colorMapType, imageType, pixelSize, imageDescriptor;
	int srcWidth, srcHeight;
	int pixelStride, rowAdvance;
	unsigned char *out, *dst;
	int row, col;

	if ( a6 )
	{
		if ( R_ReadFile( name, &fileBuffer ) < 0 || !fileBuffer )
			return 0;
		hdr = (const unsigned char *)fileBuffer;
		src = hdr + 18;
	}
	else
	{
		int handle;

		if ( ri_FS_FOpenFileByMode( name, &handle, 0 ) < 18 )
			return 0;
		ri_FS_Read( headerBytes, 18, handle );
		ri_FS_FCloseFile( handle );
		hdr = headerBytes;
	}

	idLength        = hdr[0];
	colorMapType    = hdr[1];
	imageType       = hdr[2];
	pixelSize       = hdr[16];
	imageDescriptor = hdr[17];

	if ( imageType != 2 && imageType != 10 && imageType != 3 )
	{
		ri_Error( 1, "\x15" "LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n" );
	}
	if ( colorMapType )
	{
		ri_Error( 1, "\x15" "LoadTGA: colormaps not supported\n" );
	}
	if ( pixelSize != 32 && pixelSize != 24 && imageType != 3 )
	{
		ri_Error( 1, "\x15" "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n" );
	}

	srcWidth  = hdr[12] | ( hdr[13] << 8 );
	srcHeight = hdr[14] | ( hdr[15] << 8 );

	*a3 = (unsigned __int16) srcWidth;
	*a4 = (unsigned __int16) srcHeight;
	*a5 = 0x1908;

	if ( !a6 )
		return 1;

	out = (unsigned char *) R_AllocTempMemory( 4 * srcWidth * srcHeight );
	*a2 = (int) out;
	if ( !out )
		return 0;

	pixelStride = 4;
	if ( imageDescriptor & 0x20 )
	{
		rowAdvance = 0;
		dst = out;
	}
	else
	{
		rowAdvance = -srcWidth * 8;
		dst = out + 4 * srcWidth * ( srcHeight - 1 );
	}

	if ( a6 == 2 && srcWidth < srcHeight )
	{
		*a3 = (unsigned __int16) srcHeight;
		*a4 = (unsigned __int16) srcWidth;
		pixelStride = 4 * srcHeight;

		if ( imageDescriptor & 0x20 )
		{
			rowAdvance = 4 - srcWidth * pixelStride;
			dst = out;
		}
		else
		{
			rowAdvance = -4 - srcWidth * pixelStride;
			dst = out + 4 * ( srcHeight - 1 );
		}
	}

	src += idLength;

	if ( imageType == 2 || imageType == 3 )
	{
		for ( row = 0; row < srcHeight; row++ )
		{
			for ( col = 0; col < srcWidth; col++ )
			{
				if ( pixelSize == 32 )
				{
					unsigned char b = *src++, g = *src++, r = *src++, a = *src++;
					dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = a;
				}
				else if ( pixelSize == 24 )
				{
					unsigned char b = *src++, g = *src++, r = *src++;
					dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = 255;
				}
				else if ( pixelSize == 8 )
				{
					unsigned char v = *src++;
					dst[0] = v; dst[1] = v; dst[2] = v; dst[3] = 255;
				}
				else
				{
					ri_Error( 1, "\x15" "LoadTGA: illegal pixel_size '%d' in file '%s'\n",
					          pixelSize, name );
				}
				dst += pixelStride;
			}
			dst += rowAdvance;
		}
		return 1;
	}

	row = 0;
	col = 0;
	while ( row < srcHeight )
	{
		unsigned char packetHeader = *src++;
		int           packetLength = ( packetHeader & 0x7F ) + 1;
		int           i;

		if ( ( packetHeader & 0x80 ) == 0 )
		{
			for ( i = 0; i < packetLength; i++ )
			{
				unsigned char r, g, b, a;

				if ( pixelSize == 32 )
				{
					b = *src++; g = *src++; r = *src++; a = *src++;
				}
				else if ( pixelSize == 24 )
				{
					b = *src++; g = *src++; r = *src++; a = 255;
				}
				else
				{
					ri_Error( 1, "\x15" "LoadTGA: illegal pixel_size '%d' in file '%s'\n",
					          pixelSize, name );
					return 0;
				}

				dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = a;
				dst += pixelStride;

				if ( ++col == srcWidth )
				{
					col = 0;
					if ( ++row == srcHeight )
						return 1;
					dst += rowAdvance;
				}
			}
		}
		else
		{
			unsigned char r, g, b, a;

			if ( pixelSize == 32 )
			{
				b = *src++; g = *src++; r = *src++; a = *src++;
			}
			else if ( pixelSize == 24 )
			{
				b = *src++; g = *src++; r = *src++; a = 255;
			}
			else
			{
				ri_Error( 1, "\x15" "LoadTGA: illegal pixel_size '%d' in file '%s'\n",
				          pixelSize, name );
				return 0;
			}

			for ( i = 0; i < packetLength; i++ )
			{
				dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = a;
				dst += pixelStride;

				if ( ++col == srcWidth )
				{
					col = 0;
					if ( ++row == srcHeight )
						return 1;
					dst += rowAdvance;
				}
			}
		}
	}

	return 1;
}

#define JPEG_DECOMPRESS_SIZE        424
#define JD_ERR                        0
#define JD_MEM                        4   /* 0x00516550 mov eax,[esi+4]      */
#define JD_OUTPUT_WIDTH              92   /* 0x004ED238                      */
#define JD_OUTPUT_HEIGHT             96   /* 0x004ED24A                      */
#define JD_OUTPUT_COMPONENTS        104   /* 0x004ED25B                      */
#define JD_OUTPUT_SCANLINE          120   /* 0x004ED306                      */
#define JPEG_ERROR_MGR_SIZE         132
/* jpeg_memory_mgr::self_destruct, slot 10 of 11 -- 0x004ED423 `call [eax+28h]`. */
#define JM_SELF_DESTRUCT             40

typedef char cod1_jpeg_abi_check[
    (   sizeof( struct jpeg_decompress_struct ) == JPEG_DECOMPRESS_SIZE
     && sizeof( struct jpeg_error_mgr )         == JPEG_ERROR_MGR_SIZE
     && offsetof( struct jpeg_decompress_struct, output_width )      == JD_OUTPUT_WIDTH
     && offsetof( struct jpeg_decompress_struct, output_height )     == JD_OUTPUT_HEIGHT
     && offsetof( struct jpeg_decompress_struct, output_components ) == JD_OUTPUT_COMPONENTS
     && offsetof( struct jpeg_decompress_struct, output_scanline )   == JD_OUTPUT_SCANLINE
     && offsetof( struct jpeg_memory_mgr,        self_destruct )     == JM_SELF_DESTRUCT
     && sizeof( boolean ) == 1 ) ? 1 : -1 ];
/* ---- LoadJPG  0x004ED160 ----  VERIFIED */
int __cdecl LoadJPG( const char *name, _DWORD *a1, _WORD *a2, _WORD *a3, _DWORD *a4, int a5 )
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr         jerr;

  void         *fileBuf;
  void         *scratch;
  JSAMPROW      rowPtr;
  unsigned int  width, height;
  unsigned int  x;
  _BYTE        *out;
  _BYTE        *src;
  int           rowBytes;
  int           result;
  int           j;

  fileBuf = 0;
  result  = ri_FS_ReadFile( name, &fileBuf );
  if ( result >= 0 )
    R_RememberImageAllocation( fileBuf, R_IMAGE_ALLOCATION_FILE );
  if ( !fileBuf )
    return result;

  cinfo.err = jpeg_std_error( &jerr );   /* inlined by retail, 0x004ED1C0 */

  jpeg_create_decompress( &cinfo );      /* 0x005164A0 */
  CoD_jpeg_mem_src( &cinfo, (const unsigned char *)fileBuf );   /* 0x00516430 */
  jpeg_read_header( &cinfo, TRUE );      /* 0x00516770, `push 1`             */
  jpeg_start_decompress( &cinfo );       /* 0x005160F0 */

  width  = cinfo.output_width;
  height = cinfo.output_height;

  if ( width > 0x8000 || height > 0x8000 )
  {
    ri_Printf( 2, "WARNING: image '%s' is larger than %i on at least one side\n", name, 0x8000 );
    if ( cinfo.mem )
      ( *cinfo.mem->self_destruct )( (j_common_ptr)&cinfo );
    return result;
  }

  if ( cinfo.output_components != 4 )
  {
    ri_Printf( 2, "WARNING: jpeg image '%s' is not RGB\n", name );
    if ( cinfo.mem )
      ( *cinfo.mem->self_destruct )( (j_common_ptr)&cinfo );
    return result;
  }

  *a2 = (_WORD)width;
  *a3 = (_WORD)height;
  *a4 = 6408;

  if ( !a5 )
  {
    jpeg_destroy( (j_common_ptr)&cinfo );  /* 0x004ED2B1; dimensions only */
    return result;
  }

  out      = (_BYTE *)R_AllocTempMemory( 4 * width * height );
  *a1      = (_DWORD)out;
  rowBytes = 4 * width;

  if ( a5 == 2 && width < height )
  {
    scratch = alloca( rowBytes );
    while ( cinfo.output_scanline < height )
    {
      rowPtr = (JSAMPROW)scratch;
      jpeg_read_scanlines( &cinfo, &rowPtr, 1 );

      src = (_BYTE *)scratch;
      for ( x = 0; x < width; ++x )
      {
        out[0] = src[0];
        out[1] = src[1];
        out[2] = src[2];
        out[3] = 0xFF;                    /* 0x004ED350 mov byte[edi+3],0FFh */
        out   += rowBytes;                /* down one row -- 0x004ED35A     */
        src   += 4;
      }
      /* back to the top of the image, one texel right.  0x004ED36A-0x004ED37A */
      out += 4 - (int)width * rowBytes;
    }
    *a2 = (_WORD)height;                  /* extents swap -- 0x004ED384 */
    *a3 = (_WORD)width;
  }
  else
  {
    while ( cinfo.output_scanline < height )
    {
      rowPtr = (JSAMPROW)out;
      jpeg_read_scanlines( &cinfo, &rowPtr, 1 );
      for ( j = 3; j < rowBytes; j += 4 )
        out[j] = 0xFF;
      out += rowBytes;
    }
  }

  jpeg_finish_decompress( &cinfo );       /* 0x00516930 */
  if ( cinfo.mem )
    ( *cinfo.mem->self_destruct )( (j_common_ptr)&cinfo );   /* 0x004ED423 */
  return result;
}

/* ---- init_destination  0x004ED450 ----  [HIGH] */
_DWORD *__cdecl init_destination(int a1)
{
  _DWORD *result;
  int v2;

  result = *(_DWORD **)(a1 + 20);
  v2 = result[6];
  *result = result[5];
  result[1] = v2;
  return result;
}

/* ---- empty_output_buffer  0x004ED470 ----  VERIFIED */
int __cdecl empty_output_buffer( void *cinfo )
{
  (void)cinfo;
  return 1;
}

/* ---- jpeg_start_compress  0x004ED480 ----  VERIFIED */
void __cdecl jpeg_start_compress( j_compress_ptr cinfo, boolean write_all_tables )
{
  _DWORD *a1 = (_DWORD *)cinfo;
  char v3;

  if ( a1[4] != 100 )
  {
    *(_DWORD *)(*a1 + 20) = 17;
    *(_DWORD *)(*a1 + 24) = a1[4];
    (*(void (__cdecl **)(_DWORD *))*a1)(a1);
  }
  if ( write_all_tables )
    jpeg_suppress_tables(cinfo, FALSE);
  (*(void (__cdecl **)(_DWORD *))(*a1 + 16))(a1);
  (*(void (__cdecl **)(_DWORD *))(a1[5] + 8))(a1);
  jinit_compress_master((int)a1);
  (*(void (__cdecl **)(_DWORD *))a1[76])(a1);
  v3 = *((_BYTE *)a1 + 168);
  a1[49] = 0;
  a1[4] = (v3 != 0) + 101;
}

/* ---- jpeg_write_scanlines  0x004ED4F0 ----  VERIFIED */
unsigned int __cdecl jpeg_write_scanlines( j_compress_ptr cinfo, JSAMPARRAY scanlines,
                                           unsigned int num_lines )
{
  unsigned int a1 = num_lines;
  _DWORD *a2 = (_DWORD *)cinfo;
  int a3 = (int)scanlines;
  {
  int v4;
  int v5;
  int v6;
  int result;
  int v8; // [esp+4h] [ebp-4h] BYREF

  if ( a2[4] != 101 )
  {
    *(_DWORD *)(*a2 + 20) = 17;
    *(_DWORD *)(*a2 + 24) = a2[4];
    (*(void (__cdecl **)(_DWORD *))*a2)(a2);
  }
  if ( a2[49] >= a2[7] )
  {
    *(_DWORD *)(*a2 + 20) = 117;
    (*(void (__cdecl **)(_DWORD *, int))(*a2 + 4))(a2, -1);
  }
  v4 = a2[2];
  if ( v4 )
  {
    *(_DWORD *)(v4 + 4) = a2[49];
    *(_DWORD *)(a2[2] + 8) = a2[7];
    (*(void (__cdecl **)(_DWORD *))a2[2])(a2);
  }
  v5 = a2[76];
  if ( *(_BYTE *)(v5 + 12) )
    (*(void (__cdecl **)(_DWORD *))(v5 + 4))(a2);
  if ( a1 > a2[7] - a2[49] )
    a1 = a2[7] - a2[49];
  v6 = a2[77];
  v8 = 0;
  (*(void (__cdecl **)(_DWORD *, int, int *, unsigned int))(v6 + 4))(a2, a3, &v8, a1);
  result = v8;
  a2[49] += v8;
  return (unsigned int)result;
  }
}

/* ---- term_destination  0x004ED5B0 ----  [HIGH] */
int __cdecl term_destination(int a1)
{
  int result;

  result = *(_DWORD *)(a1 + 20);
  hackSize = *(_DWORD *)(result + 24) - *(_DWORD *)(result + 4);
  return result;
}

typedef struct {
  struct jpeg_destination_mgr pub;
  unsigned char *buffer;
  unsigned int   bufferSize;
} cod_destination_mgr;

typedef cod_destination_mgr *cod_dest_ptr;

/* ---- jpegDest  0x004ED5D0 ----  VERIFIED */
void
jpegDest( j_compress_ptr cinfo, unsigned char *buffer, unsigned int bufferSize )
{
  cod_dest_ptr dest;

  if ( cinfo->dest == NULL ) {
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ( (j_common_ptr) cinfo, JPOOL_PERMANENT,
                                   SIZEOF( cod_destination_mgr ) );
  }

  dest = (cod_dest_ptr) cinfo->dest;
  dest->pub.init_destination    = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination    = term_destination;
  dest->buffer                  = buffer;
  dest->bufferSize               = bufferSize;
}

#if 0
_DWORD *__cdecl jpegDest_raw(int a1, int a2, int a3)
{
  _DWORD *result;

  if ( !*(_DWORD *)(a1 + 20) )
    *(_DWORD *)(a1 + 20) = (**(int (__cdecl ***)(int, _DWORD, int))(a1 + 4))(a1, 0, 28);
  result = *(_DWORD **)(a1 + 20);
  result[2] = init_destination;
  result[3] = empty_output_buffer;
  result[4] = term_destination;
  result[5] = a2;
  result[6] = a3;
  return result;
}
#endif   /* jpegDest_raw */

/* ---- SaveJPG  0x004ED610 ----  [HIGH] */
int __cdecl SaveJPG(int fileName, int quality, int width, unsigned int height, int flipped, int pixels)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr       jerr;
  unsigned char *buffer;
  unsigned int   bufferSize;
  unsigned int   rowBytes;
  unsigned int   prevLine;
  JSAMPROW       rowPointer[1];
  int            result;

  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_compress( &cinfo );

  bufferSize = 4 * height * width;
  buffer = (unsigned char *)ri_Hunk_AllocateTempMemory( bufferSize );
  jpegDest( &cinfo, buffer, bufferSize );

  cinfo.image_width      = width;
  cinfo.image_height     = height;
  cinfo.input_components = 4;
  cinfo.in_color_space   = JCS_RGB;

  jpeg_set_defaults( &cinfo );
  jpeg_set_quality( &cinfo, quality, TRUE );
  jpeg_start_compress( &cinfo, TRUE );

  rowBytes = 4 * width;
  prevLine = 0;
  while ( cinfo.next_scanline < cinfo.image_height )
  {
    if ( flipped )
      rowPointer[0] = (JSAMPROW)(pixels + rowBytes * (height - prevLine - 1));
    else
      rowPointer[0] = (JSAMPROW)(pixels + rowBytes * prevLine);
    jpeg_write_scanlines( &cinfo, rowPointer, 1u );
    prevLine = cinfo.next_scanline;
  }

  jpeg_finish_compress( &cinfo );
  ri_FS_WriteFile( fileName, buffer, hackSize );
  ri_Hunk_FreeTempMemory( buffer );

  result = (int)cinfo.mem;
  if ( cinfo.mem )
    (*cinfo.mem->self_destruct)( (j_common_ptr)&cinfo );
  return result;
}

/* ---- R_MangleTextureName  0x004ED8E0 ----  VERIFIED */
char *__cdecl R_MangleTextureName(int a1, int a2, float *a3, const char *a4, float a5)
{
  char *result;

  if ( (a2 & 0x40) != 0 )
  {
    result = va("$h2n_%s:%x/%x/%g", a4, a1, a2, a5);
    goto LABEL_12;
  }
  if ( !a3 )
    goto LABEL_4;
  if ( *((_DWORD *)a3 + 3) == 1065353216 )
  {
    if ( (*a3 == a3[1]) | __UNORDERED__(*a3, a3[1]) && (*a3 == a3[2]) | __UNORDERED__(*a3, a3[2]) )
    {
      if ( *(_DWORD *)a3 == 1065353216 )
      {
LABEL_4:
        result = va("$tex_%s:%x/%x", a4, a1, a2);
        goto LABEL_12;
      }
      result = va("$tex_%s:%x/%x/%g", a4, a1, a2, *a3);
    }
    else
    {
      result = va("$tex_%s:%x/%x/%g/%g/%g", a4, a1, a2, *a3, a3[1], a3[2]);
    }
  }
  else
  {
    result = va("$tex_%s:%x/%x/%g/%g/%g/%g", a4, a1, a2, *a3, a3[1], a3[2], a3[3]);
  }
LABEL_12:
  if ( strlen(result) >= 0x40 )
  {
    ri_Printf(2, "WARNING: mangled name for $texturename + %s is too long\n", a4);
    return 0;
  }
  return result;
}

/* ---- R_UnmangleTextureName  0x004ED9F0 ----  VERIFIED */
int __cdecl R_UnmangleTextureName(
        char *a1,
        int *a2,
        char *a3,
        char *Str1,
        int a5,
        int a6,
        _DWORD *a7)
{
  char *v8;
  int v9;
  char *v10;
  int v11;
  char v12;
  char v13;
  char *i;
  char *v15;
  int v16;
  const char *v17;
  int result;
  int v19;
  char *v20;

  v8 = strchr(Str1 + 5, 58);
  v9 = v8 - (Str1 + 5);
  v20 = v8;
  if ( v8 == Str1 + 5 )
  {
    v10 = a3;
    v11 = a1 - a3;
    do
    {
      v12 = *v10;
      v10[v11] = *v10;
      ++v10;
    }
    while ( v12 );
  }
  else
  {
    v13 = *a3;
    for ( i = a1; v13; v13 = (i++)[a3 - a1 + 1] )
    {
      if ( v13 == 46 )
        break;
      *i = v13;
    }
    *i = 0;
    v15 = &a1[strlen(a1) + 1];
    v16 = v15 - (a1 + 1);
    Com_Memcpy((int)(v15 - 1), (int *)(Str1 + 5), v9);
    a1[v9 + v16] = 0;
    v8 = v20;
  }
  v17 = v8 + 1;
  if ( !strncmp(Str1, "$h2n", 4u) )
  {
    result = sscanf(v17, "%x/%x/%g", a5, a6, a7);
    *a2 = 1065353216;
    a2[1] = 1065353216;
    a2[2] = 1065353216;
    a2[3] = 1065353216;
  }
  else
  {
    result = sscanf(v17, "%x/%x/%g/%g/%g/%g", a5, a6, a2, a2 + 1, a2 + 2, a2 + 3);
    if ( result < 3 )
      *a2 = 1065353216;
    if ( result < 5 )
    {
      v19 = *a2;
      a2[1] = *a2;
      a2[2] = v19;
    }
    if ( result < 6 )
    {
      result = (int)(a2 + 3);
      a2[3] = 1065353216;
    }
    *a7 = 1065353216;
  }
  return result;
}

/* ---- R_HeightmapImage  0x004EDB30 ----  VERIFIED */
int __cdecl R_HeightmapImage(int a1, int a2, int a3, float a4)
{
  int v4;
  int *v5;
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  int v12;
  int v13;
  int v14;
  int v15;
  double v16;
  double v17;
  long double v18;
  float v20;
  float v21;
  int v22;
  int v23;
  int v24;
  float v25;
  float v26;
  int *v27;
  int v28;
  int v29;
  int v30;
  float v31;
  float v32;
  float v33;

  v4 = a2;
  v20 = 1.0;
  if ( a4 > 0.0 )
    v20 = 1.0 / a4;
  v5 = (int *)ri_Hunk_AllocateTempMemory(4 * a3 * a2);
  v6 = 0;
  v27 = v5;
  if ( a3 > 0 )
  {
    do
    {
      v7 = a3;
      if ( v6 )
        v7 = v6;
      v8 = v7 - 1;
      v9 = v6 + 1;
      v28 = v6 + 1;
      v29 = v6 + 1 == a3 ? 0 : v6 + 1;
      v10 = 0;
      if ( v4 > 0 )
      {
        v11 = v4 * v6;
        v12 = v4 * v8;
        v23 = v11;
        v22 = v4 * v8;
        v24 = v4 * v29;
        while ( 1 )
        {
          v30 = v10 ? v10 : a3;
          v13 = 4 * (v11 + v30 - 1);
          v14 = v11 + v10;
          v15 = 4 * ((v10 + 1 == a3 ? 0 : v10 + 1) + v11);
          v16 = (double)(*(unsigned __int8 *)(v15 + a1)
                       + *(unsigned __int8 *)(a1 + v15 + 2)
                       + *(unsigned __int8 *)(a1 + v15 + 1)
                       - *(unsigned __int8 *)(a1 + v13 + 1)
                       - *(unsigned __int8 *)(a1 + v13 + 2)
                       - *(unsigned __int8 *)(v13 + a1))
              * 0.00065359479;
          v14 *= 4;
          v17 = (double)(*(unsigned __int8 *)(4 * (v24 + v10) + a1)
                       + *(unsigned __int8 *)(a1 + 4 * (v24 + v10) + 2)
                       + *(unsigned __int8 *)(a1 + 4 * (v24 + v10) + 1)
                       - *(unsigned __int8 *)(a1 + 4 * (v10 + v12) + 1)
                       - *(unsigned __int8 *)(a1 + 4 * (v10 + v12) + 2)
                       - *(unsigned __int8 *)(4 * (v10 + v12) + a1))
              * 0.00065359479;
          v31 = v17;
          v25 = v20 * v20;
          v18 = 1.0 / sqrt(v17 * v31 + v16 * v16 + v25);
          v21 = v18;
          v26 = (v18 * v16 + 1.0) * 127.5;
          v5 = v27;
          *((_BYTE *)v27 + v14) = (int)(v26 + 9.313225746154785e-10);
          v32 = (v21 * v31 + 1.0) * 127.5;
          *((_BYTE *)v27 + v14 + 1) = (int)(v32 + 9.313225746154785e-10);
          v33 = (v21 * v20 + 1.0) * 127.5;
          v4 = a2;
          *((_BYTE *)v27 + v14 + 2) = (int)(v33 + 9.313225746154785e-10);
          *((_BYTE *)v27 + v14 + 3) = *(_BYTE *)(a1 + v14 + 3);
          if ( ++v10 >= a2 )
            break;
          v12 = v22;
          v11 = v23;
        }
        v9 = v28;
      }
      v6 = v9;
    }
    while ( v9 < a3 );
  }
  Com_Memcpy(a1, v5, 4 * a3 * a2);
  ri_Hunk_FreeTempMemory(v5);
  return 0;
}

/* ---- R_FlipImageDiagonally  0x004EDDC0 ----  VERIFIED */
int __fastcall R_FlipImageDiagonally(int a1, int a2, int a3)
{
  int result;
  int v4;
  int *v5;
  int v6;
  _DWORD *v7;
  int *v8;
  int *v9;
  int v10;
  int v11;
  int *v12;
  _DWORD *v13;

  result = a3;
  if ( a3 == a2 )
  {
    v4 = 1;
    if ( a3 > 1 )
    {
      v5 = (int *)(a1 + 4);
      v6 = 4 * a3;
      v7 = (_DWORD *)(4 * a3 + a1);
      v12 = v5;
      v13 = v7;
      do
      {
        if ( v4 > 0 )
        {
          v8 = v7;
          v9 = v12;
          v10 = v4;
          do
          {
            v11 = *v9;
            *v9 = *v8;
            *v8++ = v11;
            v9 = (int *)((char *)v9 + v6);
            --v10;
          }
          while ( v10 );
          result = a3;
        }
        ++v4;
        v7 = &v13[v6 / 4u];
        v13 = (_DWORD *)((char *)v13 + v6);
        ++v12;
      }
      while ( v4 < result );
    }
  }
  return result;
}

/* ---- R_FlipImageHorizontally  0x004EDE40 ----  VERIFIED */
int __cdecl R_FlipImageHorizontally(int result, int a2, int a3)
{
  int v3;
  int v5;
  _DWORD *v6;
  int v7;
  int v8;

  if ( a2 > 0 )
  {
    v3 = 4 * result;
    result /= 2;
    v8 = a2;
    do
    {
      v5 = 0;
      if ( result > 0 )
      {
        v6 = (_DWORD *)(a3 + v3 - 4);
        do
        {
          v7 = *(_DWORD *)(a3 + 4 * v5);
          *(_DWORD *)(a3 + 4 * v5) = *v6;
          *v6 = v7;
          ++v5;
          --v6;
        }
        while ( v5 < result );
      }
      a3 += v3;
      --v8;
    }
    while ( v8 );
  }
  return result;
}

/* ---- R_FlipImageVertically  0x004EDEA0 ----  VERIFIED */
int __cdecl R_FlipImageVertically(int result, _DWORD *a2, int a3)
{
  int v3;
  int v4;
  _DWORD *v5;
  int v6;
  int v7;
  int v8;
  _DWORD *v9;
  int v10;
  bool v11; // zf
  _DWORD *v12;
  int v13;
  int v14;
  int v15;

  v3 = a3;
  v4 = a3 * (result - 1);
  v14 = v4;
  if ( a3 > 0 )
  {
    v5 = a2;
    v6 = result / 2;
    v15 = result / 2;
    v12 = a2;
    v13 = a3;
    do
    {
      if ( v6 > 0 )
      {
        v7 = 4 * v3;
        v8 = -4 * v3;
        v9 = &v5[v4];
        do
        {
          v10 = *v5;
          *v5 = *v9;
          *v9 = v10;
          v5 = (_DWORD *)((char *)v5 + v7);
          v9 = (_DWORD *)((char *)v9 + v8);
          --v6;
        }
        while ( v6 );
        v3 = a3;
        v4 = v14;
        v6 = v15;
        v5 = v12;
      }
      ++v5;
      result = v13 - 1;
      v11 = v13 == 1;
      v12 = v5;
      --v13;
    }
    while ( !v11 );
  }
  return result;
}

/* ---- R_LoadImage  0x004EDF20 ----  VERIFIED */
char __cdecl R_LoadImage(const char *name, int *a1, unsigned __int16 *a2, unsigned __int16 *a3, int *a4, _DWORD *a5, int a6)
{
  const char *v8;
  int v9;
  signed int v10;
  char *v11;
  char *v12;
  char *v13;
  char *v15;
  char v16[68]; // [esp+18h] [ebp-4Ch] BYREF
  unsigned int v17;
  unsigned int retaddr;
  char *v19;

  v17 = retaddr ^ _security_cookie;
  v8 = name;
  *a1 = 0;
  *a2 = 0;
  *a3 = 0;
  *a4 = 0;
  *a5 = 0;
  v10 = strlen(name);
  if ( v10 >= 5 )
  {
    if ( v10 >= 64 )
      ri_Error(1, "\x15" "image name '%s' is longer than %i characters\n", v8, 63);
    if ( v8[v10 - 4] == 46 )
      v10 -= 4;
    if ( v10 >= 60 )
      ri_Error(1, "\x15" "image name '%s' with its extension is longer than %i characters\n", v8, 63);
    qmemcpy(v16, v8, v10);
    v15 = &v16[v10 + 2];
    *v15 = 100;
    v16[v10] = 46;
    v16[v10 + 4] = 0;
    v11 = &v16[v10 + 1];
    v12 = &v16[v10 + 3];
    *v12 = 115;
    v19 = v12;
    *v11 = 100;
    LoadDDS(a4, v16, a1, a2, a3, a5, a6);
    v9 = *a4;
    if ( *a4 )
      goto LABEL_16;
    v13 = v15;
    *a5 = 1;
    *v19 = 97;
    *v11 = 116;
    *v13 = 103;
    LoadTGA((int)v16, a1, a2, a3, a4, a6);
    v9 = *a4;
    if ( *a4 || (*v19 = 103, *v11 = 106, *v13 = 112, LoadJPG(v16, a1, a2, a3, a4, a6), (v9 = *a4) != 0) )
    {
LABEL_16:
      if ( !*a1 )
        ++tr_delayedImageCount;
    }
  }
  return v9;
}

/* ---- R_LoadCubeMapImage  0x004EE0C0 ----  VERIFIED */
int __cdecl R_LoadCubeMapImage( const char *name, int flags, int imageTrack,
                                const float *colorScale )
{
  const char    *suffix[6];
  unsigned char *faces[6];
  char           filename[68];
  image_t       *image;
  unsigned __int16 loadedWidth, loadedHeight;
  int   loadedFormat, loadedSpare;
  int   width, height, format;
  int   face, s, t, i;
  int   len;

  suffix[0] = "rt";
  suffix[1] = "lf";
  suffix[2] = "bk";
  suffix[3] = "ft";
  suffix[4] = "up";
  suffix[5] = "dn";

  width  = 0;
  height = 0;
  format = 0;

  if ( name && Q_stricmpn( name, "$renormalize", 99999 ) == 0 )
  {
    /* ---- the procedural normalisation cube map (0x004EE137) ---- */
    int   axis[6][3];
    float sign[6][3];
    float dir[3];
    float normalized[3];
    unsigned char *out;

    axis[0][0] = 0; axis[0][1] = 2; axis[0][2] = 1;
    axis[1][0] = 0; axis[1][1] = 2; axis[1][2] = 1;
    axis[2][0] = 1; axis[2][1] = 0; axis[2][2] = 2;
    axis[3][0] = 1; axis[3][1] = 0; axis[3][2] = 2;
    axis[4][0] = 2; axis[4][1] = 0; axis[4][2] = 1;
    axis[5][0] = 2; axis[5][1] = 0; axis[5][2] = 1;

    sign[0][0] =  1.0f; sign[0][1] = -1.0f; sign[0][2] = -1.0f;
    sign[1][0] = -1.0f; sign[1][1] =  1.0f; sign[1][2] = -1.0f;
    sign[2][0] =  1.0f; sign[2][1] =  1.0f; sign[2][2] =  1.0f;
    sign[3][0] = -1.0f; sign[3][1] =  1.0f; sign[3][2] = -1.0f;
    sign[4][0] =  1.0f; sign[4][1] =  1.0f; sign[4][2] = -1.0f;
    sign[5][0] = -1.0f; sign[5][1] = -1.0f; sign[5][2] = -1.0f;

    flags &= 0xFFFFFFF8;                          /* 0x004EE16D */
    width  = 64;
    height = 64;

    for ( face = 0; face < 6; face++ )
    {
      out = (unsigned char *)ri_Hunk_AllocateTempMemory( 0x4000 );
      if ( out )
        R_RememberImageAllocation( out, R_IMAGE_ALLOCATION_TEMP_MEMORY );
      faces[face] = out;

      dir[ axis[face][0] ] = sign[face][0];

      for ( t = 1; t < 129; t += 2 )
      {
        dir[ axis[face][2] ] = ( (float)t * 0.015625f - 1.0f ) * sign[face][2];

        for ( s = 1; s < 129; s += 2 )
        {
          dir[ axis[face][1] ] = ( (float)s * 0.015625f - 1.0f ) * sign[face][1];

          VectorNormalize2( dir, normalized );

          out[0] = (unsigned char)(int)( ( normalized[0] + 1.0 ) * 127.5
                                         + 9.3132257461547852e-10 );
          out[1] = (unsigned char)(int)( ( normalized[1] + 1.0 ) * 127.5
                                         + 9.3132257461547852e-10 );
          out[2] = (unsigned char)(int)( ( normalized[2] + 1.0 ) * 127.5
                                         + 9.3132257461547852e-10 );
          out[3] = 0xFF;
          out += 4;
        }
      }
    }
  }
  else
  {
    /* ---- six face files (0x004EE4A3) ---- */
    len = strlen( name );
    if ( len + 7 >= 64 )
    {
      ri_Printf( 2, "Cube map name will exceed max qpath\n" );
      return 0;
    }

    strcpy( filename, name );
    strcat( filename, "_**.tga" );                /* 0x0054BAE8, 8 bytes */

    for ( face = 0; face < 6; face++ )
    {
      strcpy( filename + len + 1, suffix[face] );

      R_LoadImage( filename, (int *)&faces[face], &loadedWidth, &loadedHeight,
                   &loadedFormat, (_DWORD *)&loadedSpare, 1 );

      if ( !faces[face] )
        return 0;

      if ( face == 0 )
      {
        width  = loadedWidth;
        height = loadedHeight;
        format = loadedFormat;
        if ( loadedWidth != loadedHeight )
        {
          ri_Printf( 2, "Cube map face images should be square ('%s')\n", filename );
          return 0;
        }
      }
      else
      {
        if ( width != loadedWidth || height != loadedHeight )
        {
          ri_Printf( 2, "Cube map face images are not all the same size ('%s')\n", filename );
          return 0;
        }
        if ( format != loadedFormat )
        {
          ri_Printf( 2, "Cube map face images are not all the same format ('%s')\n", filename );
          return 0;
        }
      }
    }

    /* 0x004EE5C7.  Asymmetric on purpose -- face 1 takes three. */
    R_FlipImageDiagonally( (int)faces[0], height, width );
    R_FlipImageDiagonally( (int)faces[1], height, width );
    R_FlipImageHorizontally( width, height, (int)faces[1] );
    R_FlipImageVertically( height, (_DWORD *)faces[1], width );
    R_FlipImageVertically( height, (_DWORD *)faces[2], width );
    R_FlipImageHorizontally( width, height, (int)faces[3] );
    R_FlipImageDiagonally( (int)faces[4], height, width );
    R_FlipImageDiagonally( (int)faces[5], height, width );
  }

  image = R_AllocImage( name, 0x8513u, width, height, flags | 0x30, imageTrack );

  for ( i = 0; i < 6; i++ )
  {
    if ( !R_CreateImageInternal( image, colorScale, faces[i],
                                 0x8515u + i, 0x1908u ) )
    {
      R_FreeImage( image );                       /* inlined at 0x004EE70F */
      return 0;
    }
  }

  return (int)image;
}

/* ---- R_FindExistingImage  0x004EE770 ----  VERIFIED */
int __cdecl R_FindExistingImage( char *name, int target, int flags, int trackType )
{
    int  image;
    int  diff;

    image = hashtable[ tr_image_generateHashValue( name ) ];
    while ( image )
    {
        if ( name && image && !Q_stricmpn( name, (const char *)image, 99999 ) )
            break;
        image = *(_DWORD *)( image + 116 );   /* +0x74, next in bucket */
    }
    if ( !image )
        return 0;

    diff = *(_DWORD *)( image + 100 ) ^ flags;   /* +0x64 */
    if ( diff
      && strcmp( name, "*white" )
      && strcmp( name, "*gray" ) )
    {
        if ( diff & 1 )
            ri_Printf( 1, "WARNING: reused image %s with mixed mipmap allowance\n", name );
        if ( diff & 2 )
            ri_Printf( 1, "WARNING: reused image %s with mixed picmip allowance\n", name );
        if ( diff & 4 )
            ri_Printf( 1, "WARNING: reused image %s with mixed picmip type (model / world)\n", name );
        if ( diff & 0x30 )
            ri_Printf( 0, "WARNING: reused image %s with mixed clamp modes\n", name );
    }

    if ( *(_DWORD *)( image + 84 ) != target )   /* +0x54, GL target */
        ri_Printf( 2, "ERROR: image '%s' cannot be used in a cube map and in a normal texture", name );

    if ( *(_DWORD *)( image + 80 ) != trackType )   /* +0x50 */
        ri_Printf( 0, "WARNING: image '%s' changed type from %s to %s.\n", name,
                   r_imageTrackNames[ *(_DWORD *)( image + 80 ) ],
                   r_imageTrackNames[ trackType ] );
    *(_DWORD *)( image + 80 ) = trackType;

    return image;
}

/* ---- R_LoadSingleDelayedImage  0x004EE8A0 ----  VERIFIED */
int __cdecl R_LoadSingleDelayedImage(int a1)
{
  int v1;
  int v2;
  unsigned __int16 v3;
  int v4;
  int v5;
  const char *v6;
  const char *v7;
  int v8;
  int v10; // [esp+10h] [ebp-54h] BYREF
  int v11; // [esp+14h] [ebp-50h] BYREF
  int v12; // [esp+18h] [ebp-4Ch] BYREF
  int v13; // [esp+1Ch] [ebp-48h] BYREF
  char v14[64]; // [esp+20h] [ebp-44h] BYREF
  unsigned int v15;
  unsigned int retaddr;

  v1 = a1;
  v2 = *(_DWORD *)(a1 + 100);
  v15 = retaddr ^ _security_cookie;
  *(_DWORD *)(a1 + 100) = v2 & 0xFFFFFF7F;
  *(_DWORD *)(v1 + 104) = 0;
  *(_WORD *)(v1 + 108) = 0;
  *(_WORD *)(v1 + 110) = 0;
  *(_BYTE *)(v1 + 112) = 0;
  --tr_delayedImageCount;
  v12 = 0;
  v11 = 0;
  v10 = 0;
  a1 = 0;
  R_LoadImage((const char *)v1, &v12, (unsigned __int16 *)&v11, (unsigned __int16 *)&v10, &a1, &v13, 1);
  v3 = v11;
  v4 = v13;
  if ( (_WORD)v11 != *(_WORD *)(v1 + 64)
    || (_WORD)v10 != *(_WORD *)(v1 + 66)
    || (v5 = a1, a1 != *(_DWORD *)(v1 + 96))
    || a1 != 6408 && !v13 && (*(_BYTE *)(v1 + 100) & 1) != 0 )
  {
    R_FreeImageAllocations();
    v6 = "with";
    if ( !v4 )
      v6 = "without";
    v7 = "with";
    if ( (*(_BYTE *)(v1 + 100) & 1) == 0 )
      v7 = "without";
    ri_Error(
      1,
      "\x15" "delayed-load image '%s' changed from %ix%i type 0x%04x %s mipmaps to %ix%i type 0x%04x %s mipmaps\n",
      v1,
      v3,
      (unsigned __int16)v10,
      a1,
      v7,
      *(unsigned __int16 *)(v1 + 64),
      *(unsigned __int16 *)(v1 + 66),
      *(_DWORD *)(v1 + 96),
      v6);
    v5 = a1;
  }
  if ( !R_CreateImageInternal(v1, 0, (unsigned __int8 *)v12, 0xDE1u, v5) )
  {
    strcpy(v14, (const char *)v1);
    v8 = glState_currentTextures[glState_currentTmu];
    a1 = *(_DWORD *)(v1 + 116);
    if ( v8 == *(_DWORD *)(v1 + 88) )
    {
      glBindTexture(*(_DWORD *)(v1 + 84), 0);
      glState_currentTextures[glState_currentTmu] = 0;
    }
    qglDeleteTextures(1, v1 + 88);
    qmemcpy((void *)v1, (const void *)tr_defaultImage, 0x78u);
    strcpy((char *)v1, v14);
    *(_DWORD *)(v1 + 116) = a1;
  }
  R_FreeImageAllocations();
  return 0;
}

/* ---- R_UpdateDelayLoadImage  0x004EEA60 ----  VERIFIED */
char __cdecl R_UpdateDelayLoadImage(int a1, int a2, int a3)
{
  int v3;
  int v4;
  int v5;
  int i;
  int v7;

  v3 = a1;
  LOBYTE(a1) = *(_BYTE *)(a1 + 100);
  if ( (char)a1 < 0 )
  {
    LOBYTE(a1) = a3;
    if ( a3 )
    {
LABEL_3:
      LOBYTE(a1) = R_LoadSingleDelayedImage(v3);
      return a1;
    }
    v4 = *(_DWORD *)(v3 + 104);
    if ( a2 == v4 )
      return a1;
    if ( v4 )
    {
      a1 = CompareMergableShaders(a2, v4, v3, v3);
      if ( a1 )
        goto LABEL_3;
    }
    else
    {
      *(_DWORD *)(v3 + 104) = a2;
    }
    v5 = *(_DWORD *)(v3 + 108);
    if ( v5 != tr_delayedImageGroup )
    {
      LOBYTE(a1) = tr_numImages;
      for ( i = 0; i < tr_numImages; ++i )
      {
        v7 = dword_16C9840[i];
        if ( v7 )
        {
          if ( *(char *)(v7 + 100) < 0 && *(_DWORD *)(v7 + 108) == v5 )
            *(_DWORD *)(v7 + 108) = tr_delayedImageGroup;
        }
        LOBYTE(a1) = tr_numImages;
      }
    }
  }
  return a1;
}

/* ---- R_FindImageFile  0x004EEAE0 ----  VERIFIED */
int __cdecl R_FindImageFile(char *a1, int a2, int a3, int a4, int a5, float a6)
{
  char *v6;
  cvar_t *integer;
  int ExistingImage;
  int v9;
  int v11;
  int v12;  /* flags; int, not char */
  unsigned __int8 *v13;
  int v15;
  int v17;
  int CubeMapImage;
  unsigned __int16 v19; // [esp+14h] [ebp-14h] BYREF
  unsigned __int16 v20; // [esp+18h] [ebp-10h] BYREF
  unsigned __int8 *v21; // [esp+1Ch] [ebp-Ch] BYREF
  int v22; // [esp+20h] [ebp-8h] BYREF
  unsigned int v23; // [esp+24h] [ebp-4h] BYREF

  v6 = a1;
  if ( !a1 )
    return 0;
  integer = (cvar_t *)r_optimize->integer;
  if ( !integer || (integer = r_optimizeTextures, !r_optimizeTextures->integer) )
    a3 &= ~0x80u;
  if ( a4 == 10 )
  {
    v6 = R_MangleTextureName(a2, a3, (float *)a5, a1, a6);
    if ( !v6 )
      return 0;
  }
  ExistingImage = R_FindExistingImage(v6, a2, a3, a4);
  v9 = ExistingImage;
  if ( ExistingImage )
  {
    if ( *(char *)(ExistingImage + 100) < 0 )
    {
      R_UpdateDelayLoadImage(ExistingImage, 0, (a3 & 0x80) == 0);
      if ( *(char *)(v9 + 100) < 0 )
      {
        *(_DWORD *)(v9 + 112) += tr_delayedImageGroupTriCount;
        return v9;
      }
      return v9;
    }
    if ( *(_DWORD *)(ExistingImage + 104) )
    {
      ri_Error(1, "\x15" "tried to load image '%s', which has been merged with other textures, after the map finished loading\n", v6);
      return 0;
    }
    return v9;
  }
  if ( a4 == 10 )
  {
    v11 = R_AllocImage(v6, a2, 0, 0, a3, 10);
    R_FreeImageAllocations();
    return v11;
  }
  if ( a2 != 3553 )
  {
    if ( a2 == 34067 )
    {
      CubeMapImage = R_LoadCubeMapImage(v6, a3, a4, (float *)a5);
      R_FreeImageAllocations();
      return CubeMapImage;
    }
    ri_Printf(2, "Unknown texture target 0x%04x\n", a2);
    goto LABEL_41;
  }
  v12 = a3;
  if ( (char)a3 < 0
    && ((a3 & 0x40) != 0
     || a5
     && (*(_DWORD *)a5 != 1065353216
      || *(_DWORD *)(a5 + 4) != 1065353216
      || *(_DWORD *)(a5 + 8) != 1065353216
      || *(_DWORD *)(a5 + 12) != 1065353216)) )
  {
    v12 = a3 & ~0x80;
  }
  R_LoadImage(v6, (int *)&v21, &v20, &v19, (int *)&v23, &v22, (v12 & 0x80) == 0);
  if ( !v22 && (v12 & 1) != 0 )
  {
    ri_Printf(2, "image '%s' requested mipmaps, but they aren't available\n", v6);
LABEL_41:
    v9 = 0;
    R_FreeImageAllocations();
    return v9;
  }
  if ( !v23 )
    goto LABEL_41;
  if ( (v12 & 0x40) == 0 )
  {
    v17 = R_CreateImage(v6, v12, a4, v21, v20, v19, v23, (float *)a5);
    R_FreeImageAllocations();
    return v17;
  }
  if ( v23 != 6408 )
  {
    if ( v23 > 0x83EF && v23 <= 0x83F3 )
      ri_Printf(2, "image '%s': heightToNormal not valid for DDS textures\n", v6);
    goto LABEL_41;
  }
  v13 = v21;
  if ( ( v12 & 0x80 ) == 0 )            /* 0x004EECA5 `test bl, bl / js`  */
    R_HeightmapImage((int)v21, v20, v19, a6);
  v15 = R_CreateImage(v6, v12, a4, v13, v20, v19, 6408, (float *)a5);
  R_FreeImageAllocations();
  return v15;
}

/* ---- R_FindImageInstance  0x004EED80 ----  VERIFIED */
int __cdecl R_FindImageInstance(int a1, char *a2, int a3, char *Str1, int a5)
{
  float *v5;
  int v6;
  int result;
  int v8;
  int v9; // [esp+4h] [ebp-68h] BYREF
  float v10; // [esp+8h] [ebp-64h] BYREF
  int v11; // [esp+Ch] [ebp-60h] BYREF
  float v12[4]; // [esp+10h] [ebp-5Ch] BYREF -- var_5C..var_50, stride 4
  char v16[68]; // [esp+20h] [ebp-4Ch] BYREF
  unsigned int v17;
  unsigned int retaddr;

  v17 = retaddr ^ _security_cookie;
  R_UnmangleTextureName(v16, (int *)v12, a2, Str1, (int)&v11, (int)&v9, &v10);
  if ( (v12[0] == 1.0) | __UNORDERED__(v12[0], 1.0)
    && (v12[1] == 1.0) | __UNORDERED__(v12[1], 1.0)
    && (v12[2] == 1.0) | __UNORDERED__(v12[2], 1.0)
    && (v12[3] == 1.0) | __UNORDERED__(v12[3], 1.0) )
  {
    v5 = 0;
  }
  else
  {
    v5 = v12;
  }
  if ( a1 || !r_optimizeTextures->integer || !r_optimize->integer || (v9 & 0x100) != 0 )
    v6 = v9 & 0xFFFFFF7F;
  else
    v6 = v9 | 0x80;
  v9 = v6;
  result = R_FindImageFile(v16, v11, v6, a3, (int)v5, v10);
  v8 = result;
  if ( result )
  {
    R_UpdateDelayLoadImage(result, a5, a1);
    return v8;
  }
  return result;
}

/* ---- R_BeginDelayedImageGroup  0x004EEE80 ----  [HIGH] */
cvar_t *__cdecl R_BeginDelayedImageGroup(int a1)
{
  cvar_t *result;

  if ( tr_modelsFinishedLoading )
  {
    result = r_errorOnConflicts;
    if ( r_errorOnConflicts->integer )
    {
      result = (cvar_t *)tr_ignorePrecacheErrorCount;
      if ( !tr_ignorePrecacheErrorCount )
        return (cvar_t *)ri_Error(1, "\x15" "model '%s' not precached\n", a1);
    }
  }
  else
  {
    result = (cvar_t *)(tr_delayedImageGroupSequence + 1);
    tr_delayedImageGroupSequence = (int)result;
    tr_delayedImageGroup = (int)result;
  }
  return result;
}

/* ---- R_SetImageGroupTriCount  0x004EEED0 ----  [HIGH] */
int __cdecl R_SetImageGroupTriCount(int a1)
{
  int result;

  result = tr_delayedImageGroup;
  if ( tr_delayedImageGroup )
  {
    result = a1;
    tr_delayedImageGroupTriCount = a1;
  }
  return result;
}

/* ---- R_SetImageGroupTileMode  0x004EEEF0 ----  [HIGH] */
int __cdecl R_SetImageGroupTileMode(int a1)
{
  int result;

  result = tr_delayedImageGroup;
  if ( tr_delayedImageGroup )
  {
    result = a1;
    tr_delayedImageGroupTileMode = a1;
  }
  return result;
}

/* ---- R_EndDelayedImageGroup  0x004EEF10 ----  [HIGH] */
int R_EndDelayedImageGroup()
{
  int result;

  result = 0;
  tr_delayedImageGroup = 0;
  tr_delayedImageGroupTriCount = 0;
  return result;
}

/* ---- CopyImageTileLevel_DXT1  0x004EEF20 ----  VERIFIED */
void __cdecl CopyImageTileLevel_DXT1(
        unsigned __int16 a1,
        unsigned __int16 a2,
        unsigned __int16 a3,
        int a4,
        int *a5,
        unsigned __int16 a6,
        unsigned __int16 a7)
{
  int *v7;
  int v9; // kr00_4
  unsigned __int16 v10;
  int v11;
  int v12;
  int v13;
  int *v14;
  int v15;
  bool v16; // zf
  int v18;
  int v19;
  int v20;
  int v21;

  v7 = a5;
  v9 = a3 * a1;
  v10 = a7;
  v11 = v9 / 2 + a4 + 2 * a6;
  if ( a7 < a2 )
  {
    if ( a7 )
    {
      v18 = 2 * (a3 - a2);
      v14 = a5;
      v19 = (unsigned __int16)(((unsigned __int16)(a7 - 1) >> 2) + 1);
      do
      {
        if ( a2 )
        {
          v15 = 2 * v10;
          v20 = (unsigned __int16)(((unsigned __int16)(a2 - 1) >> 2) + 1);
          do
          {
            TransposeDDSBlockDXT1((int)v14, v11);
            v14 = (int *)((char *)v14 + v15);
            v11 += 8;
            --v20;
          }
          while ( v20 );
          v10 = a7;
        }
        v11 += v18;
        v14 = a5 + 2;
        v16 = v19 == 1;
        a5 += 2;
        --v19;
      }
      while ( !v16 );
    }
  }
  else if ( a2 )
  {
    v12 = 2 * a7;
    v21 = 2 * a3;
    v13 = (unsigned __int16)(((unsigned __int16)(a2 - 1) >> 2) + 1);
    do
    {
      Com_Memcpy((void *)v11, v7, v12);
      v7 = (int *)((char *)v7 + v12);
      v11 += v21;
      --v13;
    }
    while ( v13 );
  }
}

/* ---- CopyImageTile_DXT1  0x004EF020 ----  VERIFIED */
char __cdecl CopyImageTile_DXT1(
        int a1,
        int a2,
        int *a3,
        int a4,
        unsigned __int16 a5,
        unsigned __int16 a6,
        unsigned __int16 a7,
        unsigned __int16 a8)
{
  unsigned __int16 v8;
  unsigned __int16 v9;
  int v10;
  char result;
  int v12;
  int CardMemoryAmount;
  int *v14;
  unsigned __int16 v15;
  unsigned __int16 v16;

  v8 = a8;
  v9 = a7;
  v10 = a1;
  CopyImageTileLevel_DXT1(a6, a8, *(_WORD *)(a1 + 64), a2, a3, a5, a7);
  result = *(_BYTE *)(a1 + 100);
  if ( (result & 1) != 0 )
  {
    v15 = *(_WORD *)(a1 + 64);
    v16 = *(_WORD *)(a1 + 66);
    while ( 1 )
    {
      v12 = *(_DWORD *)(v10 + 96);
      CardMemoryAmount = GetCardMemoryAmount(v15, v12, v16);
      v15 >>= 1;
      v16 >>= 1;
      a2 += CardMemoryAmount;
      a5 >>= 1;
      a6 >>= 1;
      v14 = (int *)((char *)a3 + GetCardMemoryAmount(v9, v12, v8));
      result = a5;
      v9 >>= 1;
      v8 >>= 1;
      a3 = v14;
      if ( (a5 & 3) != 0 )
        break;
      if ( (a6 & 3) != 0 )
        break;
      CopyImageTileLevel_DXT1(a6, v8, v15, a2, v14, a5, v9);
      if ( v9 <= 1u && v8 <= 1u )
        break;
      v10 = a1;
    }
  }
  return result;
}

/* ---- CopyImageTileLevel_DXT3  0x004EF130 ----  VERIFIED */
void __cdecl CopyImageTileLevel_DXT3(
        unsigned __int16 a1,
        unsigned __int16 a2,
        unsigned __int16 a3,
        int a4,
        int a5,
        unsigned __int16 a6,
        unsigned __int16 a7)
{
  int *v7;
  int v8;
  unsigned __int16 v9;
  _BYTE *v11;
  int v12;
  int v13;
  _BYTE *v14;
  int v15;
  bool v16; // zf
  int v18;
  int v19;
  int v20;
  int v21;

  v7 = (int *)a5;
  v8 = a1 * a3 + a4;
  v9 = a6;
  v11 = (_BYTE *)(v8 + 4 * a2);
  if ( a6 < a7 )
  {
    if ( a6 )
    {
      v18 = 4 * (a1 - a7);
      v14 = (_BYTE *)a5;
      v19 = (unsigned __int16)(((unsigned __int16)(a6 - 1) >> 2) + 1);
      do
      {
        if ( a7 )
        {
          v15 = 4 * v9;
          v21 = (unsigned __int16)(((unsigned __int16)(a7 - 1) >> 2) + 1);
          do
          {
            TransposeDDSBlockDXT3(v11, v14);
            v14 += v15;
            v11 += 16;
            --v21;
          }
          while ( v21 );
          v9 = a6;
        }
        v11 += v18;
        v14 = (_BYTE *)(a5 + 16);
        LOBYTE(a1) = v19 - 1;
        v16 = v19 == 1;
        a5 += 16;
        --v19;
      }
      while ( !v16 );
    }
  }
  else if ( a7 )
  {
    v12 = 4 * a6;
    v20 = 4 * a1;
    v13 = (unsigned __int16)(((unsigned __int16)(a7 - 1) >> 2) + 1);
    do
    {
      Com_Memcpy((void *)v11, v7, v12);
      v7 = (int *)((char *)v7 + v12);
      v11 += v20;
      --v13;
    }
    while ( v13 );
  }
}

/* ---- CopyImageTile_DXT3  0x004EF220 ----  VERIFIED */
char __cdecl CopyImageTile_DXT3(
        int a1,
        int a2,
        int a3,
        int a4,
        unsigned __int16 a5,
        unsigned __int16 a6,
        unsigned __int16 a7,
        unsigned __int16 a8)
{
  unsigned __int16 v8;
  unsigned __int16 v9;
  int v10;
  char result;
  int v12;
  int CardMemoryAmount;
  int v14;
  unsigned __int16 v15;
  unsigned __int16 v16;

  v8 = a8;
  v9 = a7;
  v10 = a1;
  CopyImageTileLevel_DXT3(*(_WORD *)(a1 + 64), a5, a6, a2, a3, a7, a8);
  result = *(_BYTE *)(a1 + 100);
  if ( (result & 1) != 0 )
  {
    v15 = *(_WORD *)(a1 + 64);
    v16 = *(_WORD *)(a1 + 66);
    while ( 1 )
    {
      v12 = *(_DWORD *)(v10 + 96);
      CardMemoryAmount = GetCardMemoryAmount(v15, v12, v16);
      v15 >>= 1;
      v16 >>= 1;
      a2 += CardMemoryAmount;
      a5 >>= 1;
      a6 >>= 1;
      v14 = GetCardMemoryAmount(v9, v12, v8) + a3;
      result = a5;
      v9 >>= 1;
      v8 >>= 1;
      a3 = v14;
      if ( (a5 & 3) != 0 )
        break;
      if ( (a6 & 3) != 0 )
        break;
      CopyImageTileLevel_DXT3(v15, a5, a6, a2, v14, v9, v8);
      if ( v9 <= 1u && v8 <= 1u )
        break;
      v10 = a1;
    }
  }
  return result;
}

/* ---- CopyImageTileLevel_DXT5  0x004EF330 ----  VERIFIED */
void __cdecl CopyImageTileLevel_DXT5(
        unsigned __int16 a1,
        unsigned __int16 a2,
        unsigned __int16 a3,
        int a4,
        int a5,
        unsigned __int16 a6,
        unsigned __int16 a7)
{
  int *v7;
  int v9;
  unsigned __int16 v10;
  int v11;
  int v12;
  int v13;
  _BYTE *v14;
  int v15;
  bool v16; // zf
  int v18;
  int v19;
  int v20;
  int v21;

  v7 = (int *)a5;
  v9 = a1 * a3 + a4;
  v10 = a6;
  v11 = v9 + 4 * a2;
  if ( a6 < a7 )
  {
    if ( a6 )
    {
      v18 = 4 * (a1 - a7);
      v14 = (_BYTE *)a5;
      v19 = (unsigned __int16)(((unsigned __int16)(a6 - 1) >> 2) + 1);
      do
      {
        if ( a7 )
        {
          v15 = 4 * v10;
          v21 = (unsigned __int16)(((unsigned __int16)(a7 - 1) >> 2) + 1);
          do
          {
            TransposeDDSBlockDXT5(v14, v11);
            v14 += v15;
            v11 += 16;
            --v21;
          }
          while ( v21 );
          v10 = a6;
        }
        v11 += v18;
        v14 = (_BYTE *)(a5 + 16);
        LOBYTE(a1) = v19 - 1;
        v16 = v19 == 1;
        a5 += 16;
        --v19;
      }
      while ( !v16 );
    }
  }
  else if ( a7 )
  {
    v12 = 4 * a6;
    v20 = 4 * a1;
    v13 = (unsigned __int16)(((unsigned __int16)(a7 - 1) >> 2) + 1);
    do
    {
      Com_Memcpy((void *)v11, v7, v12);
      v7 = (int *)((char *)v7 + v12);
      v11 += v20;
      --v13;
    }
    while ( v13 );
  }
}

/* ---- CopyImageTile_DXT5  0x004EF420 ----  VERIFIED */
char __cdecl CopyImageTile_DXT5(
        int a1,
        int a2,
        int a3,
        int a4,
        unsigned __int16 a5,
        unsigned __int16 a6,
        unsigned __int16 a7,
        unsigned __int16 a8)
{
  unsigned __int16 v8;
  unsigned __int16 v9;
  int v10;
  char result;
  int v12;
  int CardMemoryAmount;
  int v14;
  unsigned __int16 v15;
  unsigned __int16 v16;

  v8 = a8;
  v9 = a7;
  v10 = a1;
  CopyImageTileLevel_DXT5(*(_WORD *)(a1 + 64), a5, a6, a2, a3, a7, a8);
  result = *(_BYTE *)(a1 + 100);
  if ( (result & 1) != 0 )
  {
    v15 = *(_WORD *)(a1 + 64);
    v16 = *(_WORD *)(a1 + 66);
    while ( 1 )
    {
      v12 = *(_DWORD *)(v10 + 96);
      CardMemoryAmount = GetCardMemoryAmount(v15, v12, v16);
      v15 >>= 1;
      v16 >>= 1;
      a2 += CardMemoryAmount;
      a5 >>= 1;
      a6 >>= 1;
      v14 = GetCardMemoryAmount(v9, v12, v8) + a3;
      result = a5;
      v9 >>= 1;
      v8 >>= 1;
      a3 = v14;
      if ( (a5 & 3) != 0 )
        break;
      if ( (a6 & 3) != 0 )
        break;
      CopyImageTileLevel_DXT5(v15, a5, a6, a2, v14, v9, v8);
      if ( v9 <= 1u && v8 <= 1u )
        break;
      v10 = a1;
    }
  }
  return result;
}

/* ---- CopyImageTile_RGBA  0x004EF530 ----  VERIFIED */
int __cdecl CopyImageTile_RGBA(
        int result,
        int *a2,
        unsigned __int16 a3,
        int a4,
        int a5,
        unsigned __int16 a6,
        unsigned __int16 a7)
{
  int v8;
  int v9;
  bool v10; // zf
  int v11;

  v8 = a5 + 4 * (a6 + *(unsigned __int16 *)(a4 + 64) * a3);
  if ( (_WORD)result )
  {
    v9 = 4 * a7;
    v11 = (unsigned __int16)result;
    do
    {
      Com_Memcpy(v8, a2, v9);
      a2 = (int *)((char *)a2 + v9);
      result = v11 - 1;
      v10 = v11 == 1;
      v8 += 4 * *(unsigned __int16 *)(a4 + 64);
      --v11;
    }
    while ( !v10 );
  }
  return result;
}

/* ---- LoadImageTile  0x004EF590 ----  VERIFIED */
int __cdecl LoadImageTile(int a1, int a2, int a3, int a4, unsigned __int16 a5, unsigned __int16 a6)
{
  bool v6; // cf
  unsigned __int8 *v7;
  unsigned __int16 v9;
  unsigned __int16 v10;
  int v11;
  int v12; // [esp+8h] [ebp-14h] BYREF
  int v13; // [esp+Ch] [ebp-10h] BYREF
  unsigned int v14; // [esp+10h] [ebp-Ch] BYREF
  unsigned __int8 *v15; // [esp+14h] [ebp-8h] BYREF
  int v16; // [esp+18h] [ebp-4h] BYREF

  *(_DWORD *)(a1 + 100) &= ~0x80u;
  *(_DWORD *)(a1 + 88) = *(_DWORD *)(a2 + 88);
  v6 = *(_WORD *)(a1 + 64) < *(_WORD *)(a1 + 66);
  *(_DWORD *)(a1 + 104) = a2;
  *(_WORD *)(a1 + 110) = a6;
  *(_WORD *)(a1 + 108) = a5;
  *(_BYTE *)(a1 + 112) = v6;
  --tr_delayedImageCount;
  R_LoadImage((const char *)a1, (int *)&v15, (unsigned __int16 *)&v13, (unsigned __int16 *)&v12, (int *)&v14, &v16, 2);
  v7 = v15;
  if ( !v15 )
    return ri_Error(1, "\x15" "image '%s' got deleted between initial scan and actual load\n", a1);
  v9 = v12;
  v10 = v13;
  if ( (unsigned __int16)v12 + (unsigned __int16)v13 != *(unsigned __int16 *)(a1 + 64) + *(unsigned __int16 *)(a1 + 66)
    || (_WORD)v13 != *(_WORD *)(a1 + 64) && (_WORD)v13 != *(_WORD *)(a1 + 66)
    || v14 != *(_DWORD *)(a1 + 96)
    || v14 != 6408 && !v16 && (*(_BYTE *)(a1 + 100) & 1) != 0 )
  {
    return ri_Error(1, "\x15" "image '%s' changed between initial scan and actual load\n", a1);
  }
  v15 = (unsigned __int8 *)R_PicmipForImageFlags(*(_DWORD *)(a1 + 100));
  if ( v15 )
  {
    do
    {
      if ( v10 <= 4u )
        break;
      if ( v9 <= 4u )
        break;
      v7 = R_MipMap(v14, v7, (char *)a1, v10, v9, 2);
      if ( !v7 )
        ri_Error(1, "\x15" "tried to put image '%s' on a texture sheet, but it has an invalid custom mipmap\n", a1);
      v10 >>= 1;
      v9 >>= 1;
      --v15;
    }
    while ( v15 );
    LOWORD(v12) = v9;
    LOWORD(v13) = v10;
  }
  v11 = *(_DWORD *)(a2 + 96);
  if ( v11 > 33778 )
  {
    if ( v11 == 33779 )
      CopyImageTile_DXT5(a2, a3, (int)v7, a4, a5, a6, v13, v12);
  }
  else if ( v11 == 33778 )
  {
    CopyImageTile_DXT3(a2, a3, (int)v7, a4, a5, a6, v13, v12);
  }
  else if ( v11 == 6408 )
  {
    CopyImageTile_RGBA(v12, (int *)v7, a6, a2, a3, a5, v13);
  }
  else if ( v11 > 33775 )
  {
    CopyImageTile_DXT1(a2, a3, (int *)v7, a4, a5, a6, v13, v12);
  }
  R_FreeImageAllocations();
  return 0;
}

/* ---- UploadImageGroup_r  0x004EF800 ----  VERIFIED */
int __cdecl UploadImageGroup_r(int *a1, int a2, int a3, int a4, int a5, int a6)
{
  int v6;
  int *v7;
  int i;
  int *v9;

  v6 = a6;
  v7 = a1;
  for ( i = a5; v7[1]; v7 = (int *)v7[1] )
  {
    v9 = (int *)*v7;
    if ( v7[4] == *(_DWORD *)(*v7 + 16) )
    {
      UploadImageGroup_r(v9, a2, a3, a4, i, v6);
      LOWORD(a6) = *(_WORD *)(*v7 + 20) + a6;
      v6 = a6;
    }
    else
    {
      UploadImageGroup_r(v9, a2, a3, a4, i, v6);
      LOWORD(a5) = *(_WORD *)(*v7 + 16) + a5;
      i = a5;
    }
  }
  if ( r_debugOptTex->integer )
    ri_Printf(
      0,
      "  %-36s(%4i,%4i)%4i x %4i from %4i x %4i\n",
      (const char *)*v7,
      (unsigned __int16)a5,
      (unsigned __int16)a6,
      v7[4],
      v7[5],
      *(unsigned __int16 *)(*v7 + 64),
      *(unsigned __int16 *)(*v7 + 66));
  return LoadImageTile(*v7, a2, a3, a4, i, v6);
}

/* ---- UploadImageGroup  0x004EF8E0 ----  VERIFIED */
int __cdecl UploadImageGroup(int a1, int *a2, int a3, int a4)
{
  bool v6; // zf
  int integer;
  char *v9;
  int v10;
  int v11;
  int v12;
  unsigned __int8 *v13;
  char *v14;
  int v15;
  int v16;
  unsigned int v17;
  int CardMemoryAmount;
  int v19;

  v6 = a2[1] == 0;
  integer = r_debugOptTex->integer;
  if ( v6 )
  {
    if ( integer )
      ri_Printf(
        0,
        "image %2i: %4i x %4i from %4i x %4i -- %s\n",
        a1,
        a2[4],
        a2[5],
        *(unsigned __int16 *)(*a2 + 64),
        *(unsigned __int16 *)(*a2 + 66),
        (const char *)*a2);
    return R_LoadSingleDelayedImage(*a2);
  }
  else
  {
    if ( integer )
      ri_Printf(0, "sheet %2i: %4i x %4i\n", a1, a2[4], a2[5]);
    v16 = a2[5];
    v15 = a2[4];
    v9 = va("*sheet%03i", a1);
    v10 = R_AllocImage(v9, 3553, v15, v16, a4, 7);
    *(_DWORD *)(v10 + 96) = a3;
    v11 = a2[5];
    v19 = v10;
    CardMemoryAmount = GetCardMemoryAmount(a2[4], a3, v11);
    if ( a3 != 6408 && (*(_BYTE *)(v10 + 100) & 1) != 0 )
    {
      v12 = a2[4];
      do
      {
        do
        {
          v12 = (v12 + 1) >> 1;
          v11 = (v11 + 1) >> 1;
          CardMemoryAmount += GetCardMemoryAmount(v12, a3, v11);
        }
        while ( v12 != 1 );
      }
      while ( v11 != 1 );
      v10 = v19;
    }
    v13 = (unsigned __int8 *)ri_Malloc(CardMemoryAmount);
    UploadImageGroup_r(a2, v10, (int)v13, (int)&v13[CardMemoryAmount], 0, 0);
    if ( a3 == 6408 )
    {
      if ( r_debugOptTex->integer )
      {
        v17 = *(unsigned __int16 *)(v10 + 66);
        v14 = va("%s.jpg", (const char *)(v10 + 1));
        SaveJPG((int)v14, 100, *(unsigned __int16 *)(v10 + 64), v17, 0, (int)v13);
      }
    }
    R_CreateImageInternal(v10, 0, v13, 0xDE1u, a3);
    return ri_Free(v13);
  }
}

/* ---- compare_mergable_common  0x004EFA60 ----  [HIGH] */
int __cdecl compare_mergable_common(_DWORD *a1, _DWORD *a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int result;

  v2 = a1[5];
  v3 = a1[4];
  v4 = v3;
  if ( v3 >= v2 )
    v4 = a1[5];
  v5 = a2[4];
  v6 = a2[5];
  v7 = v5;
  if ( v5 >= v6 )
    v7 = a2[5];
  result = v4 - v7;
  if ( !result )
  {
    result = v3 + v2 - v6 - v5;
    if ( !result )
      return a1[2] - a2[2];
  }
  return result;
}

/* ---- compare_mergable_xxxxx  0x004EFAB0 ----  [HIGH] */
int __cdecl compare_mergable_xxxxx(_DWORD **a1, _DWORD **a2)
{
  int result;

  result = (*a1)[3] - (*a2)[3];
  if ( !result )
    return compare_mergable_common(*a1, *a2);
  return result;
}

/* ---- compare_mergable_xxxx  0x004EFAE0 ----  [HIGH] */
int __cdecl compare_mergable_xxxx(_DWORD **a1, _DWORD **a2)
{
  int v2;

  v2 = (int)*a2;
  if ( !*a1 )
    return v2 != 0;
  if ( v2 )
    return compare_mergable_common(*a1, *a2);
  return -1;
}

/* ---- PickMergeDirection  0x004EFB10 ----  VERIFIED */
int __cdecl PickMergeDirection(int a1, int a2, int a3)
{
  int v3;
  int v4;
  int v5;
  int i;
  int v7;
  int v8;
  int v10;
  int v11;

  v3 = *(_DWORD *)(a2 + 4 * a1);
  v4 = *(_DWORD *)(v3 + 20);
  v5 = *(_DWORD *)(v3 + 16);
  if ( v5 == v4 )
    return 0;
  if ( v5 != glConfig_maxTextureSize )
  {
    for ( i = a1 + 1; i < a3; ++i )
    {
      v7 = *(_DWORD *)(a2 + 4 * i);
      if ( *(_DWORD *)(v7 + 20) != v4 )
        break;
      v8 = *(_DWORD *)(v7 + 16);
      if ( v8 > 2 * v5 )
        break;
      if ( v8 == 2 * v5 )
      {
        if ( i < a3 )
        {
          while ( *(_DWORD *)(*(_DWORD *)(a2 + 4 * i) + 20) == v4 )
          {
            if ( ++i >= a3 )
              return 0;
          }
          if ( i < a3 )
          {
            while ( 1 )
            {
              v10 = *(_DWORD *)(a2 + 4 * i);
              if ( *(_DWORD *)(v10 + 16) >= v5 || *(_DWORD *)(v10 + 20) != 2 * v4 )
                break;
              if ( ++i >= a3 )
                return 0;
            }
            if ( i < a3 )
            {
              v11 = *(_DWORD *)(a2 + 4 * i);
              if ( *(_DWORD *)(v11 + 16) == v5 && *(_DWORD *)(v11 + 20) == 2 * v4 )
                return 1;
            }
          }
        }
        return 0;
      }
    }
  }
  return 1;
}

/* ---- MergeImageList  0x004EFBB0 ----  VERIFIED */
int __cdecl MergeImageList(int a1, int a2, int a3, int a4)
{
  int v4;
  int v5;
  _DWORD *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int i;
  int v12;
  int v13;
  int v14;
  int v16;

  v4 = a3;
  if ( a3 != 1 )
  {
    v5 = 1;
    v16 = 1;
    if ( a3 > 1 )
    {
      v6 = (_DWORD *)(a1 + 24 * a4 + 12);
      do
      {
        v7 = *(_DWORD *)(a2 + 4 * v5);
        v8 = *(_DWORD *)(v7 + 20);
        if ( v8 == glConfig_maxTextureSize )
          break;
        v9 = *(_DWORD *)(a2 + 4 * v5 - 4);
        if ( *(_DWORD *)(v7 + 16) == *(_DWORD *)(v9 + 16) && v8 == *(_DWORD *)(v9 + 20) )
        {
          *(v6 - 3) = v9;
          *(v6 - 2) = *(_DWORD *)(a2 + 4 * v5);
          *v6 = *(_DWORD *)(*(_DWORD *)(a2 + 4 * v5) + 12);
          *(v6 - 1) = *(_DWORD *)(*(_DWORD *)(a2 + 4 * v5 - 4) + 8) + *(_DWORD *)(*(_DWORD *)(a2 + 4 * v5) + 8);
          v10 = PickMergeDirection(v5, a2, v4);
          if ( v10 )
          {
            if ( v10 == 1 )
            {
              v6[1] = *(_DWORD *)(*(_DWORD *)(a2 + 4 * v5) + 16);
              v6[2] = 2 * *(_DWORD *)(*(_DWORD *)(a2 + 4 * v5) + 20);
            }
          }
          else
          {
            v6[1] = 2 * *(_DWORD *)(*(_DWORD *)(a2 + 4 * v5) + 16);
            v6[2] = *(_DWORD *)(*(_DWORD *)(a2 + 4 * v5) + 20);
          }
          for ( i = v5 + 1; i < a3; *(_DWORD *)(a2 + 4 * i++ - 8) = v12 )
          {
            v12 = *(_DWORD *)(a2 + 4 * i);
            v13 = *(_DWORD *)(v12 + 20);
            v14 = v6[2];
            if ( v13 > v14 )
              break;
            if ( v13 == v14 && *(_DWORD *)(v12 + 16) >= v6[1] )
              break;
          }
          *(_DWORD *)(a2 + 4 * i - 8) = v6 - 3;
          if ( i < a3 )
          {
            qmemcpy((void *)(a2 + 4 * i - 4), (const void *)(a2 + 4 * i), 4 * (a3 - i));
            v5 = v16;
            i = a3;
          }
          *(_DWORD *)(a2 + 4 * i - 4) = 0;
          --a3;
          ++a4;
          v6 += 6;
          v4 = a3;
        }
        else
        {
          v16 = ++v5;
        }
      }
      while ( v5 < v4 );
    }
  }
  return a4;
}

/* ---- CombineImageGroups  0x004EFD00 ----  VERIFIED */
int __cdecl CombineImageGroups(unsigned int *a1, unsigned int a2)
{
  unsigned int v2;
  signed int i;
  int v4;
  int v5;
  int v6;
  int v8;
  int v9;
  signed int j;
  int v11;

  qsort_m(a2, *a1, 4u, (int (__cdecl *)(unsigned int, _BYTE *))compare_mergable_xxxx);
  if ( *a1 )
  {
    do
    {
      if ( *(_DWORD *)(a2 + 4 * *a1 - 4) )
        break;
      v2 = *a1 - 1;
      *a1 = v2;
    }
    while ( v2 );
  }
  for ( i = 1; i < (int)*a1; ++i )
  {
    v4 = *(_DWORD *)(a2 + 4 * i);
    v5 = *(_DWORD *)(v4 + 20);
    if ( v5 == glConfig_maxTextureSize )
      break;
    v6 = *(_DWORD *)(a2 + 4 * i - 4);
    if ( *(_DWORD *)(v4 + 16) == *(_DWORD *)(v6 + 16) && v5 == *(_DWORD *)(v6 + 20) )
    {
      v8 = *(_DWORD *)(*(_DWORD *)(a2 + 4 * i - 4) + 12);
      v9 = *(_DWORD *)(*(_DWORD *)(a2 + 4 * i) + 12);
      for ( j = 0; j < (int)*a1; ++j )
      {
        v11 = *(_DWORD *)(a2 + 4 * j);
        if ( *(_DWORD *)(v11 + 12) == v9 )
          *(_DWORD *)(v11 + 12) = v8;
      }
      qsort_m(a2, *a1, 4u, (int (__cdecl *)(unsigned int, _BYTE *))compare_mergable_xxxxx);
      return v8;
    }
  }
  return 0;
}

/* ---- MergeAndLoadDelayedImages  0x004EFDC0 ----  VERIFIED */
void __cdecl MergeAndLoadDelayedImages(int a1, signed int a2, int a3, int a4)
{
  signed int v4;
  int v7;
  int *v8;
  int v9;
  unsigned __int16 v10;
  int v11;
  cvar_t *v12;
  int integer;
  int v14;
  int i;
  int v16;
  int v17;
  int v18;
  int v19;
  signed int v20;
  signed int j;
  int k;
  int v23;
  int v24;
  signed int m;
  signed int v26;
  int n;
  _DWORD *v28;  /* pointer index array */
  char *v29;
  _DWORD *v30;  /* group record array */
  int v31;

  v4 = a2;
  if ( r_debugOptTex->integer )
    ri_Printf(0, "%i groups -> ", a2);
  v30 = (_DWORD *)alloca((48 * v4 - 21) & 0xFFFFFFFC);
  v28 = (_DWORD *)alloca((8 * v4 - 4 + 3) & 0xFFFFFFFC);
  if ( v4 > 0 )
  {
    v7 = a1;
    /* retail 0x004EFE22 `lea eax, [edi+14h]` -- record 0's sixth dword */
    v8 = (int *)(v30 + 5);
    v29 = (char *)v28 - a1;
    v31 = v4;
    do
    {
      *(v8 - 5) = *(_DWORD *)v7;
      *(v8 - 4) = 0;
      *(v8 - 2) = *(_DWORD *)(*(_DWORD *)v7 + 108);
      *(v8 - 3) = *(_DWORD *)(*(_DWORD *)v7 + 112);
      LOWORD(v9) = *(_WORD *)(*(_DWORD *)v7 + 66);
      v10 = *(_WORD *)(*(_DWORD *)v7 + 64);
      if ( v10 <= (unsigned __int16)v9 )
        v9 = (unsigned __int16)v9;
      else
        v9 = v10;
      *(v8 - 1) = v9;
      *v8 = *(unsigned __int16 *)(*(_DWORD *)v7 + 64) + *(unsigned __int16 *)(*(_DWORD *)v7 + 66) - v9;
      v11 = *(_DWORD *)(*(_DWORD *)v7 + 100);
      if ( (v11 & 2) != 0 )
      {
        v12 = r_picmip2;
        if ( (v11 & 4) == 0 )
          v12 = r_picmip;
        integer = v12->integer;
        if ( (v11 & 2) != 0 )
        {
          v14 = 3;
          if ( integer <= 3 )
            v14 = integer;
          for ( i = v14 & ~(v14 >> 31); i; *v8 = v17 >> 1 )
          {
            v16 = *(v8 - 1);
            if ( v16 <= 4 )
              break;
            v17 = *v8;
            if ( *v8 <= 4 )
              break;
            --i;
            *(v8 - 1) = v16 >> 1;
          }
        }
      }
      if ( *(v8 - 1) > glConfig_maxTextureSize )
      {
        do
        {
          v18 = *(v8 - 1) >> 1;
          v19 = *v8 >> 1;
          *(v8 - 1) = v18;
          *v8 = v19;
        }
        while ( v18 > glConfig_maxTextureSize );
      }
      *(_DWORD *)&v29[v7] = v8 - 5;
      v8 += 6;
      v7 += 4;
      --v31;
    }
    while ( v31 );
    v4 = a2;
  }
  v31 = v4;
  qsort_m((unsigned int)v28, v4, 4u, (int (__cdecl *)(unsigned int, _BYTE *))compare_mergable_xxxxx);
  v20 = 0;
  if ( v4 > 0 )
  {
    while ( 1 )
    {
      for ( j = v20 + 1; j < v4; ++j )
      {
        if ( *(_DWORD *)(v28[v20] + 12) != *(_DWORD *)(v28[j] + 12) )
          break;
      }
      v31 = MergeImageList((int)v30, (int)&v28[v20], j - v20, v31);
      v20 = j;
      if ( j >= a2 )
        break;
      v4 = a2;
    }
  }
  for ( k = CombineImageGroups((unsigned int *)&a2, (unsigned int)v28);
        k;
        k = CombineImageGroups((unsigned int *)&a2, (unsigned int)v28) )
  {
    v23 = 0;
    if ( *(_DWORD *)(v28[0] + 12) != k )
    {
      do
        v24 = *(_DWORD *)(v28[++v23] + 12);
      while ( v24 != k );
    }
    for ( m = v23 + 1; m < a2; ++m )
    {
      if ( *(_DWORD *)(v28[v23] + 12) != *(_DWORD *)(v28[m] + 12) )
        break;
    }
    v31 = MergeImageList((int)v30, (int)&v28[v23], m - v23, v31);
  }
  v26 = a2;
  if ( r_debugOptTex->integer )
    ri_Printf(0, "%i groups\n", a2);
  for ( n = 0; n < v26; ++n )
    UploadImageGroup(n, (int *)v28[n], a3, a4);
}

/* ---- compare_image_types  0x004F0030 ----  [HIGH] */
int __cdecl compare_image_types(int a1, int a2)
{
  int result;

  result = *(_DWORD *)(*(_DWORD *)a1 + 96) - *(_DWORD *)(*(_DWORD *)a2 + 96);
  if ( !result )
    return (*(_DWORD *)(*(_DWORD *)a1 + 100) & 1) - (*(_DWORD *)(*(_DWORD *)a2 + 100) & 1);
  return result;
}

/* ---- R_LoadDelayedImages  0x004F0060 ----  VERIFIED */
void R_LoadDelayedImages()
{
  signed int v1;
  int v2;
  int v3;
  signed int v4;
  signed int v5;
  int v6;
  _DWORD *v7;
  int v8;
  signed int v9;

  tr_delayedImageGroup = 0;
  if ( tr_delayedImageCount )
  {
    v7 = (_DWORD *)alloca((4 * tr_delayedImageCount + 3) & 0xFFFFFFFC);
    v1 = 0;
    v2 = 0;
    v9 = 0;
    if ( tr_numImages > 0 )
    {
      do
      {
        v3 = dword_16C9840[v2];
        if ( v3 )
        {
          if ( (*(_BYTE *)(v3 + 100) & 0x80) != 0 )
            v7[v1++] = v3;
        }
        ++v2;
      }
      while ( v2 < tr_numImages );
      v9 = v1;
    }
    qsort_m((unsigned int)v7, v1, 4u, (int (__cdecl *)(unsigned int, _BYTE *))compare_image_types);
    v4 = 0;
    if ( v1 > 0 )
    {
      do
      {
        v5 = v4 + 1;
        if ( v4 + 1 < v1 )
        {
          v8 = *(_DWORD *)(v7[v4] + 96);
          do
          {
            v6 = v7[v5];
            if ( v8 != *(_DWORD *)(v6 + 96) )
              break;
            if ( ((*(_BYTE *)(v6 + 100) ^ *(_BYTE *)(v7[v4] + 100)) & 1) != 0 )
              break;
            ++v5;
          }
          while ( v5 < v1 );
        }
        MergeAndLoadDelayedImages((int)&v7[v4], v5 - v4, *(_DWORD *)(v7[v4] + 96), *(_DWORD *)(v7[v4] + 100) & 1);
        v1 = v9;
        v4 = v5;
      }
      while ( v5 < v9 );
    }
    tr_delayedImageCount = 0;
  }
}

/* ---- R_CreateDlightImage  0x004F0150 ----  VERIFIED */
int R_CreateDlightImage()
{
  double v0;
  double v1;
  unsigned __int8 *v2;
  int v3;
  unsigned __int64 v4; // rax
  bool v5; // cc
  int v6;
  int result;
  int v8;
  unsigned __int8 *v9;
  int v10;
  float v11;
  cvar_t *v12;
  cvar_t *v13;
  unsigned __int8 v14[16384]; // [esp+28h] [ebp-8020h] BYREF
  _DWORD v15[4]; // [esp+4028h] [ebp-4020h] BYREF
  __int16 v16;
  _BYTE v17[16384]; // [esp+403Ah] [ebp-400Eh] BYREF
  unsigned int v18;
  unsigned int retaddr;

  v18 = retaddr ^ _security_cookie;
  v13 = ri_Cvar_Get("r_dlightRadius", "0.1124278", 0);
  v12 = ri_Cvar_Get("r_dlightShift", "0.3162278", 0);
  v0 = v12->value * v12->value;
  v8 = 0;
  v9 = &v14[1];
  v1 = 1.0 / (v0 + 1.0);
  v11 = 255.0 / (1.0 / (v13->value * v13->value + v0) - v1);
  do
  {
    v2 = v9;
    v3 = 0;
    v10 = 0;
    do
    {
      v4 = (unsigned __int64)(v11
                            / (((31.5 - (double)v10) * (31.5 - (double)v10) + (31.5 - (double)v8) * (31.5 - (double)v8))
                             * 0.0010078106
                             + v0)
                            - v1 * v11);
      if ( (int)v4 <= 255 )
      {
        if ( (int)v4 < 0 )
          LOBYTE(v4) = 0;
      }
      else
      {
        LOBYTE(v4) = -1;
      }
      ++v3;
      v2[1] = v4;
      *v2 = v4;
      *(v2 - 1) = v4;
      v2[2] = -1;
      v2 += 256;
      v10 = v3;
    }
    while ( v3 < 64 );
    v5 = ++v8 < 64;
    v9 += 4;
  }
  while ( v5 );
  v6 = R_AllocImage("*dlight", 3553, 64, 64, 48, 0);
  if ( !R_CreateImageInternal(v6, 0, v14, 0xDE1u, 6408) )
  {
    R_FreeImage(v6);
    v6 = 0;
  }
  tr_dlightImage = v6;
  result = 1;
  if ( v13->modificationCount > 1 || v12->modificationCount > 1 )
  {
    v15[0] = 0x20000;
    v15[1] = 0;
    v15[2] = 0;
    v15[3] = 4194368;
    v16 = 32;
    qmemcpy(v17, v14, sizeof(v17));
    return ri_FS_WriteFile("dlight.tga", v15, 16402);
  }
  return result;
}

/* ---- R_CreateDefaultImage  0x004F0380 ----  VERIFIED */
int R_CreateDefaultImage()
{
  int v0;
  char *v1;
  int v2;
  int result;
  int v4[258]; // [esp+10h] [ebp-408h] BYREF
  unsigned int retaddr;

  v4[257] = retaddr ^ _security_cookie;
  memset(v4, 0x20u, 0x400u);
  v0 = 0;
  v1 = (char *)v4 + 1;
  do
  {
    BYTE2(v4[v0]) = 0;
    BYTE1(v4[v0]) = 0;
    LOBYTE(v4[v0]) = 0;
    BYTE3(v4[v0]) = -1;
    BYTE2(v4[v0 + 240]) = 0;
    BYTE1(v4[v0 + 240]) = 0;
    LOBYTE(v4[v0 + 240]) = 0;
    BYTE3(v4[v0 + 240]) = -1;
    v1[1] = 0;
    *v1 = 0;
    *(v1 - 1) = 0;
    v1[2] = -1;
    v1[61] = 0;
    v1[60] = 0;
    v1[59] = 0;
    v1[62] = -1;
    ++v0;
    v1 += 64;
  }
  while ( v0 < 16 );
  v2 = R_AllocImage("*default", 3553, 16, 16, 1, 0);
  result = R_CreateImageInternal(v2, 0, (unsigned __int8 *)v4, 0xDE1u, 6408);
  if ( result )
  {
    tr_defaultImage = v2;
  }
  else
  {
    R_FreeImage(v2);
    tr_defaultImage = 0;
  }
  return result;
}

/* ---- SmallestTextureSizeFitting  0x004F0480 ----  [HIGH] */
int __cdecl SmallestTextureSizeFitting(char *this)
{
  int v1;
  int result;

  v1 = (int)(this - 1);
  for ( result = 1; v1; result *= 2 )
    v1 >>= 1;
  return result;
}

/* ---- R_CreateScreenImage  0x004F04A0 ----  VERIFIED */
int R_CreateScreenImage()
{
  int v0;
  int v1;
  int v2;
  int v3;
  int *v4;
  int v5;
  int v7;

  v0 = dwStyle - 1;
  v1 = 1;
  v7 = 1;
  if ( dwStyle != 1 )
  {
    do
    {
      v0 >>= 1;
      v1 *= 2;
    }
    while ( v0 );
    v7 = v1;
  }
  v2 = dwExStyle - 1;
  v3 = 1;
  if ( dwExStyle != 1 )
  {
    do
    {
      v2 >>= 1;
      v3 *= 2;
    }
    while ( v2 );
  }
  v4 = (int *)ri_Hunk_AllocateTempMemory(4 * v1 * v3);
  Com_Memset(v4, 255, 4 * v1 * v3);
  v5 = R_AllocImage("*screen", 3553, v1, v3, 560, 0);
  if ( !R_CreateImageInternal(v5, 0, (unsigned __int8 *)v4, 0xDE1u, 6408) )
  {
    R_FreeImage(v5);
    v5 = 0;
  }
  tr_screenImage = v5;
  tr_screenImageWidth = v7;
  tr_screenImageHeight = v3;
  ri_Hunk_FreeTempMemory(v4);
  return 0;
}

/* ---- R_CreateBuiltinImages  0x004F0560 ----  VERIFIED */
int R_CreateBuiltinImages()
{
  int v0;
  char v1;
  char *v2;
  int v3;
  _BYTE *v4;
  int v5;
  int v6;
  int v7;
  int *v8;
  int v9;
  int HashValue;
  int v11;
  int v12;
  int v14[258]; // [esp+10h] [ebp-408h] BYREF
  unsigned int retaddr;

  v14[257] = retaddr ^ _security_cookie;
  R_CreateDefaultImage();
  memset(v14, 0xFFu, 0x400u);
  v0 = R_AllocImage("*white", 3553, 8, 8, 0, 0);
  if ( !R_CreateImageInternal(v0, 0, (unsigned __int8 *)v14, 0xDE1u, 6408) )
  {
    R_FreeImage(v0);
    v0 = 0;
  }
  v1 = tr_identityLightByte;
  tr_whiteImage = v0;
  v2 = (char *)v14 + 1;
  v3 = 16;
  do
  {
    v4 = v2;
    v5 = 16;
    do
    {
      v4[1] = v1;
      *v4 = v1;
      *(v4 - 1) = v1;
      v4[2] = -1;
      v4 += 64;
      --v5;
    }
    while ( v5 );
    v2 += 4;
    --v3;
  }
  while ( v3 );
  v6 = R_AllocImage("*identityLight", 3553, 8, 8, 0, 0);
  if ( !R_CreateImageInternal(v6, 0, (unsigned __int8 *)v14, 0xDE1u, 6408) )
  {
    R_FreeImage(v6);
    v6 = 0;
  }
  memset(v14, 0xC0u, 0x400u);
  tr_identityLightImage = v6;
  v7 = R_AllocImage("*gray", 3553, 8, 8, 0, 0);
  if ( !R_CreateImageInternal(v7, 0, (unsigned __int8 *)v14, 0xDE1u, 6408) )
  {
    R_FreeImage(v7);
    v7 = 0;
  }
  tr_grayImage = v7;
  v8 = tr_scratchImages;
  do
  {
    v9 = R_AllocImage("*scratch", 3553, 16, 16, 50, 0);
    if ( !R_CreateImageInternal(v9, 0, (unsigned __int8 *)v14, 0xDE1u, 6408) )
    {
      HashValue = tr_image_generateHashValue((char *)v9);
      v11 = *(_DWORD *)(v9 + 116);
      --tr_numImages;
      v12 = glState_currentTmu;
      hashtable[HashValue] = v11;
      if ( glState_currentTextures[v12] == *(_DWORD *)(v9 + 88) )
      {
        glBindTexture(*(_DWORD *)(v9 + 84), 0);
        glState_currentTextures[glState_currentTmu] = 0;
      }
      qglDeleteTextures(1, v9 + 88);
      v9 = 0;
    }
    *v8++ = v9;
  }
  while ( v8 < &tr_scratchImages[32] );
  R_CreateDlightImage();
  return R_CreateScreenImage();
}

/* ---- R_SetColorMappings  0x004F0780 ----  VERIFIED */
void R_SetColorMappings()
{
  int integer;
  double v1;
  int v2;
  unsigned __int64 v3; // rax
  int v4;
  cvar_t *v5;
  int v6;
  unsigned __int64 v7; // rax
  const char *v8;
  int v9;
  int v10;
  float value;

  integer = r_overBrightBits->integer;
  tr_overbrightBits = integer;
  if ( !glConfig_deviceSupportsGamma )
  {
    integer = 0;
    tr_overbrightBits = 0;
  }
  if ( !glConfig_isFullscreen )
  {
    integer = 0;
    tr_overbrightBits = 0;
  }
  if ( glConfig_colorBits <= 16 )
  {
    if ( integer > 1 )
    {
      integer = 1;
      goto LABEL_12;
    }
  }
  else if ( integer > 2 )
  {
    integer = 2;
    goto LABEL_12;
  }
  if ( integer >= 0 )
    goto LABEL_13;
  integer = 0;
LABEL_12:
  tr_overbrightBits = integer;
LABEL_13:
  v1 = 1.0 / (double)(1 << integer);
  tr_identityLight = v1;
  tr_identityLightByte = (unsigned __int8)(unsigned __int64)(v1 * 255.0);
  if ( r_intensity->value <= 1.0 )
  {
    ri_Cvar_Set("r_intensity", "1");
    LOBYTE(integer) = tr_overbrightBits;
  }
  if ( (r_gamma->value < 0.5) | __UNORDERED__(r_gamma->value, 0.5) )
  {
    v8 = "0.5";
LABEL_19:
    ri_Cvar_Set("r_gamma", v8);
    LOBYTE(integer) = tr_overbrightBits;
    goto LABEL_20;
  }
  if ( r_gamma->value > 3.0 )
  {
    v8 = "3.0";
    goto LABEL_19;
  }
LABEL_20:
  v2 = 0;
  value = r_gamma->value;
  v9 = 0;
  do
  {
    if ( (value == 1.0) | __UNORDERED__(value, 1.0) )
      LODWORD(v3) = v2;
    else
      v3 = (unsigned __int64)(pow((double)v9 * 0.0039215689, 1.0 / value) * 255.0 + 0.5);
    v4 = (_DWORD)v3 << integer;
    if ( v4 >= 0 )
    {
      if ( v4 > 255 )
        LOBYTE(v4) = -1;
    }
    else
    {
      LOBYTE(v4) = 0;
    }
    s_gammatable[v2++] = v4;
    v9 = v2;
  }
  while ( v2 < 256 );
  v5 = r_intensity;
  v6 = 0;
  v10 = 0;
  do
  {
    v7 = (unsigned __int64)((double)v10 * v5->value);
    if ( (int)v7 > 255 )
      LOBYTE(v7) = -1;
    s_intensitytable[v6++] = v7;
    v10 = v6;
  }
  while ( v6 < 256 );
  if ( glConfig_deviceSupportsGamma )
    GLimp_SetGamma( (unsigned char *)s_gammatable,
                    (unsigned char *)s_gammatable,
                    (unsigned char *)s_gammatable );
}

/* ---- R_InitImages  0x004F0980 ----  [HIGH] */
int R_InitImages()
{
  memset(hashtable, 0, sizeof(hashtable));
  R_SetColorMappings();
  return R_CreateBuiltinImages();
}

/* ---- R_DeleteTextures  0x004F09A0 ----  VERIFIED */
void R_DeleteTextures()
{
  int i;

  for ( i = 0; i < tr_numImages; ++i )
    qglDeleteTextures(1, dword_16C9840[i] + 88);
  memset(dword_16C9840, 0, sizeof(dword_16C9840));
  memset(glState_currentTextures, 0, 0x20u);
  if ( glBindTexture )
  {
    if ( qglActiveTextureARB )
    {
      if ( glState_currentTmu != 1 )
      {
        qglActiveTextureARB(33985);
        glState_currentTmu = 1;
      }
      if ( glState_currentClientTmu != 1 )
      {
        qglClientActiveTextureARB(33985);
        glState_currentClientTmu = 1;
      }
      glBindTexture(0xDE1u, 0);
      if ( glState_currentTmu )
      {
        qglActiveTextureARB(33984);
        glState_currentTmu = 0;
      }
      if ( glState_currentClientTmu )
      {
        qglClientActiveTextureARB(33984);
        glState_currentClientTmu = 0;
      }
    }
    glBindTexture(0xDE1u, 0);
  }
}

/* ---- RE_GetShaderFromModel  0x004F0AA0 ----  VERIFIED */
int __cdecl RE_GetShaderFromModel(int a1, int a2)
{
  int v2;
  int v3;
  int v4;
  int v5;
  int v6;
  char *Shader;
  int v8;
  int v9;
  const char *v10;
  int v12;

  v2 = a2;
  if ( a2 < 0 )
    v2 = 0;
  v3 = tr_models[a1];
  if ( !v3 )
    return 0;
  v4 = *(_DWORD *)(v3 + 76);
  if ( !v4 )
    return 0;
  v5 = *(_DWORD *)(v4 + 24);
  if ( !v5 )
    return 0;
  if ( v2 >= *(_DWORD *)(v4 + 28) )
    v2 = 0;
  v6 = v5 + 12 * v2;
  Shader = *(char **)(v6 + 4);
  v12 = v6;
  if ( *((int *)Shader + 16) > -1 )
  {
    v8 = 1;
    v9 = hashtable[tr_image_generateHashValue(Shader)];
    if ( v9 )
    {
      v10 = *(const char **)(v6 + 4);
      while ( strcmp(v10, (const char *)v9) )
      {
        v9 = *(_DWORD *)(v9 + 116);
        if ( !v9 )
          goto LABEL_15;
      }
      v8 = *(_DWORD *)(v9 + 100) & 1;
LABEL_15:
      v6 = v12;
    }
    Shader = R_FindShader(*(char **)(v6 + 4), -1, v8, (const char *)8);
    *(_DWORD *)(*((_DWORD *)Shader + 85) + 1636) = 10;
    *(_DWORD *)(*((_DWORD *)Shader + 85) + 1668) = 1048832;
  }
  return *((_DWORD *)Shader + 17);
}
