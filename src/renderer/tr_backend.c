/*
 * @fidelity: likely
 * @fidelity-default: unreviewed
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#define funcs_4D4A37 funcs_4D4A37__cod1_hdr   /* cod1_globals.h types it unsigned char[432]; real typed def below */
#include "../qcommon/cod1_globals.h"
#undef funcs_4D4A37
#include "tr_records.h"
#include "tr_shaderregistry.h"
#include "tr_gl_types.h"
#include "tr_orientation.h"   /* backEnd.or 0x016D8DA8 -- one object, not 16 shards */
#include "tr_tess.h"
#include "tr_font.h"

extern void GLimp_EndFrame( void );
extern int RB_BeginImmediateMode();
void __cdecl RB_BeginSurface( void *shader, int vertexComponentCount );
extern int RB_CheckOverflow();
extern int RB_DrawDebug();
extern int RB_EndMultitexture();
void __cdecl RB_EndSurface( void );
extern int RB_RenderFlares();
extern int RB_SelectStorageATI();
extern int RB_SelectStorageNV();
extern int RB_ShadowFinish();
int __cdecl RB_ExecuteRenderCommands( _DWORD *a1, const char *a2 );
__declspec(dllimport) unsigned long __stdcall timeGetTime( void );
extern int RB_glVertex3f();
extern int RE_SetColor();
extern int R_FogOn();
extern double __cdecl R_GetAsianScale( int font, float scale );
extern int *R_GetCharacterGlyph( int character, int fontBase );
extern int R_RotateForModelEntity();
extern int R_SetFogColor();
extern int R_TransformDlights();

typedef void (__cdecl *rb_surfaceFunc_t)( void *surf );
#define rb_surfaceTable   ((rb_surfaceFunc_t *)(void *)funcs_4D4A37)

/* === Merged from renderer/tr_backend_rdata.c (retail linked it as tr_*.obj .rdata/.data). === */

/* renderer/tr_surface.c */
void __cdecl RB_SurfaceBad( void *surf );                          /* 0x00512960 */
void __cdecl RB_SurfaceSkip( void *surf );                         /* 0x00512980 */
void __cdecl RB_SurfacePolychain( void *surf );                    /* 0x005105B0 */
void __cdecl RB_SurfaceEntity( void *surf );                       /* 0x005128A0 */
void __cdecl RB_SurfaceTriangles( void *surf );                    /* 0x005122B0 */
void __cdecl RB_SurfaceOptimized( void *surf );                    /* 0x00512660 */

/* renderer/tr_xanim.c */
void __cdecl RB_SurfaceXModelRigid( void *surf );                  /* 0x0050EA40 */
void __cdecl RB_SurfaceXModelRigidSSE( void *surf );               /* 0x0050EB40 */
void __cdecl RB_SurfaceXModelRigidARB( void *surf );               /* 0x0050EC90 */
void __cdecl RB_SurfaceXModelRigidATI( void *surf );               /* 0x0050EF20 */
void __cdecl RB_SurfaceXModelRigidNV( void *surf );                /* 0x0050F380 */
void __cdecl RB_SurfaceXModelWeight( void *surf );                 /* 0x0050F5E0 */
void __cdecl RB_SurfaceXModelWeightSSE( void *surf );              /* 0x0050F780 */

/* renderer/tr_staticmodel.c */
void __cdecl RB_SurfaceStaticModel( void *surf );                  /* 0x00505580 */
void __cdecl RB_SurfaceStaticModelCached( void *surf );            /* 0x005056A0 */
void __cdecl RB_SurfaceStaticModelT2V3_Generic( void *surf );      /* 0x005065C0 */
void __cdecl RB_SurfaceStaticModelT2V3_ARB( void *surf );          /* 0x005057A0 */
void __cdecl RB_SurfaceStaticModelATI( void *surf );               /* 0x00505C80 */
void __cdecl RB_SurfaceStaticModelT2V3_NV( void *surf );           /* 0x00506130 */
void __cdecl RB_SurfaceStaticModelT2N3V3_Generic( void *surf );    /* 0x00506390 */
void __cdecl RB_SurfaceStaticModelT2N3V3_ARB( void *surf );        /* 0x00505A30 */
void __cdecl RB_SurfaceStaticModelT2N3V3_NV( void *surf );         /* 0x00506820 */

/* 0x00571140.  The retail dword is in the comment on each line. */
rb_surfaceFunc_t funcs_4D4A37[29] =
{
    /*  0  0x00512960 */  RB_SurfaceBad,
    /*  1  0x00512980 */  RB_SurfaceSkip,
    /*  2  0x005105B0 */  RB_SurfacePolychain,
    /*  3  0x005128A0 */  RB_SurfaceEntity,
    /*  4  0x0050EA40 */  RB_SurfaceXModelRigid,
    /*  5  0x0050EB40 */  RB_SurfaceXModelRigidSSE,
    /*  6  0x0050EC90 */  RB_SurfaceXModelRigidARB,
    /*  7  0x0050EF20 */  RB_SurfaceXModelRigidATI,
    /*  8  0x0050F380 */  RB_SurfaceXModelRigidNV,
    /*  9  0x0050F5E0 */  RB_SurfaceXModelWeight,
    /* 10  0x0050F780 */  RB_SurfaceXModelWeightSSE,
    /* 11  0x00505580 */  RB_SurfaceStaticModel,
    /* 12  0x005056A0 */  RB_SurfaceStaticModelCached,
    /* 13  0x005056A0 */  RB_SurfaceStaticModelCached,
    /* 14  0x005056A0 */  RB_SurfaceStaticModelCached,
    /* 15  0x005056A0 */  RB_SurfaceStaticModelCached,
    /* 16  0x005065C0 */  RB_SurfaceStaticModelT2V3_Generic,
    /* 17  0x005057A0 */  RB_SurfaceStaticModelT2V3_ARB,
    /* 18  0x00505C80 */  RB_SurfaceStaticModelATI,
    /* 19  0x00506130 */  RB_SurfaceStaticModelT2V3_NV,
    /* 20  0x00506390 */  RB_SurfaceStaticModelT2N3V3_Generic,
    /* 21  0x00505A30 */  RB_SurfaceStaticModelT2N3V3_ARB,
    /* 22  0x00505C80 */  RB_SurfaceStaticModelATI,
    /* 23  0x00506820 */  RB_SurfaceStaticModelT2N3V3_NV,
    /* 24  0x005122B0 */  RB_SurfaceTriangles,
    /* 25  0x00512660 */  RB_SurfaceOptimized,
    /* 26  0x00512660 */  RB_SurfaceOptimized,
    /* 27  0x00512660 */  RB_SurfaceOptimized,
    /* 28  0x00512660 */  RB_SurfaceOptimized
};

/* ---- GL_Bind  0x004D5450 ----  VERIFIED */
void __cdecl GL_Bind(GLenum *a1)
{
  GLuint v1;
  GLenum v2;

  if ( a1 )
  {
    v1 = a1[22];
  }
  else
  {
    ri_Printf(2, "GL_Bind: NULL image\n");
    v1 = *(_DWORD *)(tr_defaultImage + 88);
  }

  if ( r_nobind->integer )
  {
    if ( tr_dlightImage )
      v1 = *(_DWORD *)(tr_dlightImage + 88);
  }
  v2 = cap[glState_currentTmu];
  if ( v2 != a1[21] )
  {
    if ( v2 )
      glDisable(cap[glState_currentTmu]);
    glEnable(a1[21]);
    cap[glState_currentTmu] = a1[21];
  }
  if ( glState_currentTextures[glState_currentTmu] != v1 )
  {
    a1[23] = tr_frameCount;
    glState_currentTextures[glState_currentTmu] = v1;
    glBindTexture(a1[21], v1);
  }
}

/* ---- GL_SetTextureTarget  0x004D5500 ----  VERIFIED */
void __cdecl GL_SetTextureTarget(GLenum a1)
{
  GLenum v1;

  v1 = cap[glState_currentTmu];
  if ( v1 != a1 )
  {
    if ( v1 )
      glDisable(cap[glState_currentTmu]);
    if ( a1 )
      glEnable(a1);
    cap[glState_currentTmu] = a1;
  }
}

/* ---- GL_SelectTexture  0x004D5540 ----  VERIFIED */
void __cdecl GL_SelectTexture(int a1)
{
  if ( glState_currentTmu != a1 )
  {
    qglActiveTextureARB(a1 + 33984);
    glState_currentTmu = a1;
  }
  if ( glState_currentClientTmu != a1 )
  {
    qglClientActiveTextureARB(a1 + 33984);
    glState_currentClientTmu = a1;
  }
}

/* ---- GL_BindMultitexture  0x004D5580 ----  VERIFIED */
void __cdecl GL_BindMultitexture(int a1, int a2, int a3)
{
  GLuint v3;
  GLuint v4;

  v3 = *(_DWORD *)(a3 + 88);
  v4 = *(_DWORD *)(a1 + 88);

  if ( r_nobind->integer )
  {
    if ( tr_dlightImage )
    {
      v3 = *(_DWORD *)(tr_dlightImage + 88);
      v4 = v3;
    }
  }
  if ( glState_currentTextures[1] != v3 )
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
    *(_DWORD *)(a3 + 92) = tr_frameCount;
    glState_currentTextures[1] = v3;
    glBindTexture(0xDE1u, v3);
  }
  if ( glState_currentTextures[0] != v4 )
  {
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
    *(_DWORD *)(a1 + 92) = tr_frameCount;
    glState_currentTextures[0] = v4;
    glBindTexture(0xDE1u, v4);
  }
}

/* ---- GL_Cull  0x004D5670 ----  VERIFIED */
void __cdecl GL_Cull(int a1)
{
  if ( glState_faceCulling != a1 )
  {
    if ( a1 == 2 )
    {
      glDisable(0xB44u);
      glState_faceCulling = 2;
      return;
    }
    if ( glState_faceCulling == 2 )
      glEnable(0xB44u);
    if ( a1 == 1 )
    {
      if ( !backEnd_viewParms_isMirror )
      {
LABEL_8:
        qglCullFace(1029);
        glState_faceCulling = a1;
        return;
      }
    }
    else if ( backEnd_viewParms_isMirror )
    {
      goto LABEL_8;
    }
    qglCullFace(1028);
    glState_faceCulling = a1;
  }
}

/* ---- GL_TexEnv  0x004D56E0 ----  VERIFIED */
void __cdecl GL_TexEnv(int a1)
{
  if ( a1 != glState_texEnv[glState_currentTmu] )
  {
    glState_texEnv[glState_currentTmu] = a1;
    if ( a1 <= 8448 )
    {
      switch ( a1 )
      {
        case 8448:
          glTexEnvi(0x2300u, 0x2200u, 8448);
          return;
        case 260:
          glTexEnvi(0x2300u, 0x2200u, 260);
          return;
        case 7681:
          glTexEnvi(0x2300u, 0x2200u, 7681);
          return;
      }
      goto LABEL_10;
    }
    if ( a1 != 8449 )
    {
LABEL_10:
      ri_Error(1, "\x15" "GL_TexEnv: invalid env '%x' passed\n", a1);
      return;
    }
    glTexEnvi(0x2300u, 0x2200u, 8449);
  }
}

/* ---- GL_Normalize  0x004D5794 ----  VERIFIED */
void __cdecl GL_Normalize(GLenum a1)
{
  glState_normalizeTarget = a1;
  if ( a1 )
    glEnable(a1);
}

/* ---- GL_State  0x004D57B0 ----  VERIFIED */
int __cdecl GL_State( int stateBits )
{
	int   bits = stateBits;   /* ebx -- retail mutates it before storing */
	int   diff;
	int   srcFactor;          /* edi */
	int   dstFactor;          /* esi */
	float offsetFactor;
	float offsetUnits;

	diff = bits ^ glState_glStateBits;
	if ( !diff )                                        /* 0x004D57BF */
		return 0;

	/* ---- depth test and depth func  0x004D57C5 ---- */
	if ( diff & 0x70000 )
	{
		if ( diff & 0x10000 )
		{
			/* NOTE the polarity: the bit DISABLES.  0x004D57D3. */
			if ( bits & 0x10000 )
				glDisable( 0xB71u );
			else
				glEnable( 0xB71u );
		}
		if ( bits & 0x20000 )
			glDepthFunc( 0x202u );
		else if ( bits & 0x40000 )
			glDepthFunc( 0x207u );
		else
			glDepthFunc( 0x203u );
	}

	/* ---- depth mask  0x004D5817 ---- */
	if ( diff & 0x100 )
		qglDepthMask( ( bits & 0x100 ) != 0 );

	/* ---- polygon mode  0x004D5835 ---- */
	if ( diff & 0x1000 )
	{
		if ( bits & 0x1000 )
			glPolygonMode( 0x408u, 0x1B01u );
		else
			glPolygonMode( 0x408u, 0x1B02u );
	}

	if ( diff & 0x70000000 )
	{
		int atest = bits & 0x70000000;

		if ( (unsigned int)atest > 0x20000000 )
		{
			if ( atest == 0x40000000 )
			{
				glEnable( 0xBC0u );
				glAlphaFunc( 0x206u, 0.5f);
			}
		}
		else if ( atest == 0x20000000 )
		{
			glEnable( 0xBC0u );
			glAlphaFunc( 0x201u, 0.5f);
		}
		else if ( atest )
		{
			if ( atest == 0x10000000 )
			{
				glEnable( 0xBC0u );
				glAlphaFunc( 0x204u, 0.0f);
			}
		}
		else
		{
			glDisable( 0xBC0u );
		}
	}

	if ( diff & 0x100000 )
	{
		if ( *(_DWORD *)(tr_world + 280) && !r_entFullbright->integer && ( bits & 0x100000 ) )
		{
			glEnable( 0xB50u );
		}
		else
		{
			glDisable( 0xB50u );
			if ( bits & 0x100000 )
			{
				if ( glState_clientStateBits & 0x100 )
				{
					glDisableClientState( 0x8076u );
					glState_clientStateBits &= ~0x100;
				}
				qglColor3f( tr_identityLight,
				               tr_identityLight,
				               tr_identityLight);
				bits &= ~0x100000;
			}
		}
	}

	if ( diff & 0x200000 )
	{
		if ( bits & 0x200000 )
		{
			R_FogOn();
		}
		else if ( glState_glStateBits & 0x200000 )
		{
			glDisable( 0xB60u );
			glState_glStateBits &= ~0x200000;
		}
	}

	/* ---- polygon offset  0x004D598D ---- */
	if ( diff & 0xE000 )
	{
		if ( bits & 0xE000 )
		{
			glEnable( 0x8037u );
			if ( bits & 0x2000 )
			{
				qglPolygonOffset( r_offsetfactor->value,
				               r_offsetunits->value);
			}
			else if ( bits & 0x4000 )
			{
				offsetUnits  = r_offsetunits->value  + r_offsetunits->value;
				offsetFactor = r_offsetfactor->value + r_offsetfactor->value;
				qglPolygonOffset( offsetFactor, offsetUnits);
			}
			else
			{
				qglPolygonOffset( 0.0f, r_offsetunits->value);
			}
		}
		else
		{
			glDisable( 0x8037u );
		}
	}

	/* ---- programmable pipeline toggles  0x004D5A0D ---- */
	if ( diff & 0x2000000 )
	{
		if ( bits & 0x2000000 )
			glEnable( 0x8620u );
		else
			glDisable( 0x8620u );
	}
	if ( diff & 0x400000 )
	{
		if ( bits & 0x400000 )
			glEnable( 0x86DEu );
		else
			glDisable( 0x86DEu );
	}
	if ( diff & 0x800000 )
	{
		if ( bits & 0x800000 )
			glEnable( 0x8522u );
		else
			glDisable( 0x8522u );
	}
	if ( diff & 0x1000000 )
	{
		if ( bits & 0x1000000 )
			glEnable( 0x8920u );
		else
			glDisable( 0x8920u );
	}

	glState_glStateBits = bits;
	if ( !( diff & 0xFF ) )
		return 0;

	if ( !( bits & 0xFF ) )
	{
		glDisable( 0xBE2u );
		return R_SetFogColor();                         /* tail jump, 0x004D5BB3 */
	}

	switch ( bits & 0xF )
	{
		case 1:  srcFactor = 0x000; break;
		case 2:  srcFactor = 0x001; break;
		case 3:  srcFactor = 0x306; break;
		case 4:  srcFactor = 0x307; break;
		case 5:  srcFactor = 0x302; break;
		case 6:  srcFactor = 0x303; break;
		case 7:  srcFactor = 0x304; break;
		case 8:  srcFactor = 0x305; break;
		case 9:  srcFactor = 0x308; break;
		default:
			srcFactor = 1;
			ri_Error( 1, "\x15" "GL_State: invalid src blend state bits\n" );
			break;
	}

	switch ( bits & 0xF0 )
	{
		case 0x10:  dstFactor = 0x000; break;
		case 0x20:  dstFactor = 0x001; break;
		case 0x30:  dstFactor = 0x300; break;
		case 0x40:  dstFactor = 0x301; break;
		case 0x50:  dstFactor = 0x302; break;
		case 0x60:  dstFactor = 0x303; break;
		case 0x70:  dstFactor = 0x304; break;
		case 0x80:  dstFactor = 0x305; break;
		default:
			dstFactor = 1;
			ri_Error( 1, "\x15" "GL_State: invalid dst blend state bits\n" );
			break;
	}

	glEnable( 0xBE2u );
	qglBlendFunc( srcFactor, dstFactor );
	return R_SetFogColor();                             /* tail jump, 0x004D5B9E */
}

#if 0
char __cdecl GL_State(int a1)
{
  int v1;
  int v2;
  int v3;
  int v4;
  unsigned int v5;
  int v6;
  float v8;
  float v9;
  int v10;

  v1 = a1;
  v2 = a1 ^ glState_glStateBits;
  v10 = v2;
  if ( v2 )
  {
    if ( (v2 & 0x70000) != 0 )
    {
      if ( (v2 & 0x10000) != 0 )
      {
        if ( (v1 & 0x10000) != 0 )
          glDisable(0xB71u);
        else
          glEnable(0xB71u);
      }
      if ( (v1 & 0x20000) != 0 )
      {
        glDepthFunc(0x202u);
      }
      else if ( (v1 & 0x40000) != 0 )
      {
        glDepthFunc(0x207u);
      }
      else
      {
        glDepthFunc(0x203u);
      }
    }
    if ( (v10 & 0x100) != 0 )
      qglDepthMask((v1 & 0x100) != 0);
    if ( (v10 & 0x1000) != 0 )
    {
      if ( (v1 & 0x1000) != 0 )
        glPolygonMode(0x408u, 0x1B01u);
      else
        glPolygonMode(0x408u, 0x1B02u);
    }
    if ( (v10 & 0x70000000) != 0 )
    {
      v3 = v1 & 0x70000000;
      if ( (v1 & 0x70000000u) > 0x20000000 )
      {
        if ( v3 == 0x40000000 )
        {
          glEnable(0xBC0u);
          glAlphaFunc(0x206u, 0.5);
        }
      }
      else if ( (v1 & 0x70000000) == 0x20000000 )
      {
        glEnable(0xBC0u);
        glAlphaFunc(0x201u, 0.5);
      }
      else if ( v3 )
      {
        if ( v3 == 0x10000000 )
        {
          glEnable(0xBC0u);
          glAlphaFunc(0x204u, 0.0);
        }
      }
      else
      {
        glDisable(0xBC0u);
      }
    }
    if ( (v10 & 0x100000) != 0 )
    {
      if ( *(_DWORD *)(tr_world + 280) && !r_entFullbright->integer && (v1 & 0x100000) != 0 )
      {
        glEnable(0xB50u);
      }
      else
      {
        glDisable(0xB50u);
        if ( (v1 & 0x100000) != 0 )
        {
          if ( (glState_clientStateBits & 0x100) != 0 )
          {
            glDisableClientState(0x8076u);
            glState_clientStateBits &= ~0x100u;
          }
          qglColor3f(tr_identityLight, tr_identityLight, tr_identityLight);
          v1 &= ~0x100000u;
        }
      }
    }
    if ( (v10 & 0x200000) != 0 )
    {
      if ( (v1 & 0x200000) != 0 )
      {
        R_FogOn();
      }
      else if ( (glState_glStateBits & 0x200000) != 0 )
      {
        glDisable(0xB60u);
        glState_glStateBits &= ~0x200000u;
      }
    }
    if ( (v10 & 0xE000) != 0 )
    {
      if ( (v1 & 0xE000) != 0 )
      {
        glEnable(0x8037u);
        if ( (v1 & 0x2000) != 0 )
        {
          qglPolygonOffset(r_offsetfactor->value, r_offsetunits->value);
        }
        else if ( (v1 & 0x4000) != 0 )
        {
          v9 = r_offsetunits->value + r_offsetunits->value;
          v8 = r_offsetfactor->value + r_offsetfactor->value;
          qglPolygonOffset(v8, v9);
        }
        else
        {
          qglPolygonOffset(0.0f, r_offsetunits->value);
        }
      }
      else
      {
        glDisable(0x8037u);
      }
    }
    if ( (v10 & 0x2000000) != 0 )
    {
      if ( (v1 & 0x2000000) != 0 )
        glEnable(0x8620u);
      else
        glDisable(0x8620u);
    }
    /* Retail 0x004D5A32/0x004D5A57/0x004D5A7C: `test ..., 400000h / 800000h /
     * 1000000h` -- EXT_vertex_shader and the ATI fragment-shader state bits. */
    if ( (v10 & 0x400000) != 0 )
    {
      if ( (v1 & 0x400000) != 0 )
        glEnable(0x86DEu);
      else
        glDisable(0x86DEu);
    }
    if ( (v10 & 0x800000) != 0 )
    {
      if ( (v1 & 0x800000) != 0 )
        glEnable(0x8522u);
      else
        glDisable(0x8522u);
    }
    if ( (v10 & 0x1000000) != 0 )
    {
      if ( (v1 & 0x1000000) != 0 )
        glEnable(0x8920u);
      else
        glDisable(0x8920u);
    }
    LOBYTE(v2) = v10;
    glState_glStateBits = v1;
    if ( (_BYTE)v10 )
    {
      if ( (_BYTE)v1 )
      {
        switch ( v1 & 0xF )
        {
          case 1:
            v4 = 0;
            break;
          case 2:
            v4 = 1;
            break;
          case 3:
            v4 = 774;
            break;
          case 4:
            v4 = 775;
            break;
          case 5:
            v4 = 770;
            break;
          case 6:
            v4 = 771;
            break;
          case 7:
            v4 = 772;
            break;
          case 8:
            v4 = 773;
            break;
          case 9:
            v4 = 776;
            break;
          default:
            v4 = 1;
            ri_Error(1, "\x15" "GL_State: invalid src blend state bits\n");
            break;
        }
        v5 = (v1 & 0xF0) - 16;
        if ( v5 > 0x70 )
        {
LABEL_89:
          v6 = 1;
          ri_Error(1, "\x15" "GL_State: invalid dst blend state bits\n");
        }
        else
        {
          switch ( *((_BYTE *)GL_State_dstBlendIndexTable + v5) )
          {
            case 0:
              v6 = 0;
              break;
            case 1:
              v6 = 1;
              break;
            case 2:
              v6 = 768;
              break;
            case 3:
              v6 = 769;
              break;
            case 4:
              v6 = 770;
              break;
            case 5:
              v6 = 771;
              break;
            case 6:
              v6 = 772;
              break;
            case 7:
              v6 = 773;
              break;
            case 8:
              goto LABEL_89;
          }
        }
        glEnable(0xBE2u);
        qglBlendFunc(v4, v6);
      }
      else
      {
        glDisable(0xBE2u);
      }
      LOBYTE(v2) = R_SetFogColor();
    }
  }
  return v2;
}
#endif

/* ---- GL_ClientState  0x004D5C80 ----  VERIFIED */
int __cdecl GL_ClientState(int a1)
{
  int result;
  __int16 v2;
  int v3;

  result = a1;
  v2 = a1 ^ glState_clientStateBits;
  if ( a1 == glState_clientStateBits )
    return result;
  if ( (v2 & 1) != 0 )
  {
    if ( glState_currentClientTmu )
      qglClientActiveTextureARB(33984);
    if ( (a1 & 1) != 0 )
      glEnableClientState(0x8078u);
    else
      glDisableClientState(0x8078u);
    v3 = glState_currentClientTmu;
    if ( !glState_currentClientTmu )
      goto LABEL_11;
    qglClientActiveTextureARB(glState_currentClientTmu + 33984);
  }
  v3 = glState_currentClientTmu;
LABEL_11:
  if ( (v2 & 2) != 0 )
  {
    if ( v3 != 1 )
      qglClientActiveTextureARB(33985);
    if ( (a1 & 2) != 0 )
      glEnableClientState(0x8078u);
    else
      glDisableClientState(0x8078u);
    v3 = glState_currentClientTmu;
    if ( glState_currentClientTmu != 1 )
    {
      qglClientActiveTextureARB(glState_currentClientTmu + 33984);
      v3 = glState_currentClientTmu;
    }
  }
  if ( (v2 & 4) != 0 )
  {
    if ( v3 != 2 )
      qglClientActiveTextureARB(33986);
    if ( (a1 & 4) != 0 )
      glEnableClientState(0x8078u);
    else
      glDisableClientState(0x8078u);
    v3 = glState_currentClientTmu;
    if ( glState_currentClientTmu != 2 )
    {
      qglClientActiveTextureARB(glState_currentClientTmu + 33984);
      v3 = glState_currentClientTmu;
    }
  }
  if ( (v2 & 8) != 0 )
  {
    if ( v3 != 3 )
      qglClientActiveTextureARB(33987);
    if ( (a1 & 8) != 0 )
      glEnableClientState(0x8078u);
    else
      glDisableClientState(0x8078u);
    v3 = glState_currentClientTmu;
    if ( glState_currentClientTmu != 3 )
    {
      qglClientActiveTextureARB(glState_currentClientTmu + 33984);
      v3 = glState_currentClientTmu;
    }
  }
  if ( (v2 & 0x10) != 0 )
  {
    if ( v3 != 4 )
      qglClientActiveTextureARB(33988);
    if ( (a1 & 0x10) != 0 )
      glEnableClientState(0x8078u);
    else
      glDisableClientState(0x8078u);
    v3 = glState_currentClientTmu;
    if ( glState_currentClientTmu != 4 )
    {
      qglClientActiveTextureARB(glState_currentClientTmu + 33984);
      v3 = glState_currentClientTmu;
    }
  }
  if ( (v2 & 0x20) != 0 )
  {
    if ( v3 != 5 )
      qglClientActiveTextureARB(33989);
    if ( (a1 & 0x20) != 0 )
      glEnableClientState(0x8078u);
    else
      glDisableClientState(0x8078u);
    v3 = glState_currentClientTmu;
    if ( glState_currentClientTmu != 5 )
    {
      qglClientActiveTextureARB(glState_currentClientTmu + 33984);
      v3 = glState_currentClientTmu;
    }
  }
  if ( (v2 & 0x40) != 0 )
  {
    if ( v3 != 6 )
      qglClientActiveTextureARB(33990);
    if ( (a1 & 0x40) != 0 )
      glEnableClientState(0x8078u);
    else
      glDisableClientState(0x8078u);
    v3 = glState_currentClientTmu;
    if ( glState_currentClientTmu != 6 )
    {
      qglClientActiveTextureARB(glState_currentClientTmu + 33984);
      v3 = glState_currentClientTmu;
    }
  }
  if ( (char)v2 < 0 )
  {
    if ( v3 != 7 )
      qglClientActiveTextureARB(33991);
    if ( (char)a1 >= 0 )
      glDisableClientState(0x8078u);
    else
      glEnableClientState(0x8078u);
    if ( glState_currentClientTmu != 7 )
      qglClientActiveTextureARB(glState_currentClientTmu + 33984);
  }
  if ( (v2 & 0x100) != 0 )
  {
    if ( (a1 & 0x100) != 0 )
      glEnableClientState(0x8076u);
    else
      glDisableClientState(0x8076u);
  }
  if ( (v2 & 0x200) != 0 )
  {
    if ( (a1 & 0x200) != 0 )
      glEnableClientState(0x8075u);
    else
      glDisableClientState(0x8075u);
  }
  if ( (v2 & 0x400) != 0 )
  {
    if ( (a1 & 0x400) != 0 )
    {
      glEnableClientState(0x8074u);
      result = a1;
      glState_clientStateBits = a1;
      return result;
    }
    glDisableClientState(0x8074u);
  }
  result = a1;
  glState_clientStateBits = a1;
  return result;
}

/* ---- GL_DrawElements  0x004D5F60 ----  VERIFIED */
void __cdecl GL_DrawElements(GLsizei a1, GLenum mode, GLenum type, GLvoid *indices)
{
  ++backEnd_pc_drawCallCount;
  backEnd_pc_drawnIndexCount += a1;
  glDrawElements(mode, a1, type, indices);
}

/* ---- GL_DrawRangeElements  0x004D5FA0 ----  VERIFIED */
void __cdecl GL_DrawRangeElements(
        GLsizei a1,
        const GLvoid *a2,
        int a3,
        GLenum a4,
        GLenum a5,
        int a6)
{
  if ( qglDrawRangeElementsEXT && a3 )
  {
    backEnd_pc_drawnIndexCount += a1;
    ++backEnd_pc_drawCallCount;
    qglDrawRangeElementsEXT(a4, a6, a3, a1, a5, a2);
  }
  else
  {
    backEnd_pc_drawnIndexCount += a1;
    ++backEnd_pc_drawCallCount;
    glDrawElements(a4, a1, a5, a2);
  }
}

/* ---- GL_DrawElementArrayATI  0x004D6000 ----  VERIFIED */
int __cdecl GL_DrawElementArrayATI(int a1, int a2)
{
  backEnd_pc_drawnIndexCount += a1;
  ++backEnd_pc_drawCallCount;
  glDrawElementArrayATI(a2, a1);
  return 0;
}

/* ---- SetViewportAndScissor  0x004D6080 ----  VERIFIED */
void SetViewportAndScissor( void )
{
	glMatrixMode( 0x1701u );
	glLoadMatrixf( backEnd_viewParms_projectionMatrix );  /* viewParms +328, 0x016D8C90 */
	glMatrixMode( 0x1700u );
	glViewport( backEnd_viewParms_viewportX, backEnd_viewParms_viewportY,
	            backEnd_viewParms_viewportWidth, backEnd_viewParms_viewportHeight );
	glScissor( backEnd_viewParms_viewportX, backEnd_viewParms_viewportY,
	           backEnd_viewParms_viewportWidth, backEnd_viewParms_viewportHeight );
}

/* ---- RB_BeginDrawingView  0x004D60F0 ----  VERIFIED */
int RB_BeginDrawingView( void )
{
	int   clearBits;      /* esi */
	int   flags;          /* cl -- only the LOW BYTE of backEnd_refdef_rdflags */
	int   haveWorld;      /* eax, loaded once at 0x004D615E */
	float grey;
	float one = 1.0f;

	if ( r_finish->integer == 1 && !glState_finishCalled )
	{
		qglFinish();
		glState_finishCalled = 1;
	}
	if ( !r_finish->integer )
		glState_finishCalled = 1;

	backEnd_projection2D = 0;
	SetViewportAndScissor();
	GL_State( 0x100 );
	glState_currentLightingEntity = 0;

	flags     = backEnd_refdef_rdflags & 0xFF;
	haveWorld = dword_16DCA54;
	clearBits = 0;

	if ( cg_shadows->integer == 2
	  || ( r_measureOverdraw->integer && ( !haveWorld || ( flags & 8 ) ) ) )
	{
		clearBits = 0x400;
	}

	if ( r_uifullscreen->integer )
	{
		clearBits = 0x100;
		glClear( clearBits );
		goto done;
	}

	clearBits |= 0x100;

	if ( haveWorld )
	{
		if ( flags & 8 )
		{
			if ( !r_fastsky->integer && !( flags & 1 ) )
			{
				if ( dword_16C4BB0 && dword_16C4BB8 )
				{
					glClearColor( red, green,
					              blue, alpha);
					clearBits |= 0x4000;
				}
			}
			else
			{
				clearBits |= 0x4000;
				if ( dword_16C4BB0 )
				{
					glClearColor( red, green,
					              blue, alpha);
				}
				else if ( glfogNum > 0 && dword_16C4C70 )
				{
					glClearColor( flt_16C4C50, flt_16C4C54,
					              flt_16C4C58, flt_16C4C5C);
				}
				else
				{
					goto greyClear;                     /* 0x004D6226 -> loc_4D625B */
				}
			}
		}
		else if ( glfogNum > 0 && dword_16C4C70 )
		{
			if ( flags & 0x20 )
			{
				if ( dword_16C4C40 == 0x2601 )
					clearBits |= 0x4000;
			}
			else if ( !cg_skybox->integer )
			{
				clearBits |= 0x4000;
			}
			if ( clearBits & 0x4000 )
			{
				glClearColor( flt_16C4C50, flt_16C4C54,
				              flt_16C4C58, flt_16C4C5C);
			}
		}
	}
	else if ( flags & 1 )
	{
		clearBits &= ~0x4000;                           /* 0x004D62F3 */
	}
	else if ( r_fastsky->integer )
	{
		clearBits |= 0x4000;
		if ( !dword_16C4C70 )
			goto greyClear;                             /* 0x004D6312 -> loc_4D625B */
		glClearColor( flt_16C4C50, flt_16C4C54,
		              flt_16C4C58, flt_16C4C5C);
	}
	else if ( dword_16C4C70 && dword_16C4C78 )
	{
		glClearColor( flt_16C4C50, flt_16C4C54,
		              flt_16C4C58, flt_16C4C5C);
		clearBits |= 0x4000;
	}

	goto clear;

greyClear:
	grey = tr_identityLight * 0.5f;
	glClearColor( grey, grey, grey,
	              one);

clear:
	if ( clearBits )
		glClear( clearBits );

done:
	if ( backEnd_refdef_rdflags & 4 )
	{
		grey = (float)( (double)( backEnd_refdef_time & 0xFF ) * 0.003921568859368563 );
		glClearColor( grey, grey, grey,
		              one);
		glClear( 0x4000u );
		backEnd_isHyperspace = 1;
	}
	else
	{
		backEnd_isHyperspace = 0;
		backEnd_skyRenderedThisView = 0;
	}
	return 0;
}

#if 0
void RB_BeginDrawingView()
{
  int v0;
  GLbitfield v1;
  GLclampf v3;
  GLclampf v4;
  GLclampf v5;
  GLclampf v6;
  GLclampf reda;
  GLclampf red;

  if ( r_finish->integer == 1 && !glState_finishCalled )
  {
    qglFinish();
    glState_finishCalled = 1;
  }
  if ( !r_finish->integer )
    glState_finishCalled = 1;
  backEnd_projection2D = 0;
  SetViewportAndScissor();
  GL_State(256);
  glState_currentLightingEntity = 0;
  v0 = 0;
  if ( cg_shadows->integer == 2 || r_measureOverdraw->integer && (!dword_16DCA54 || (backEnd_refdef_rdflags & 8) != 0) )
    v0 = 1024;
  if ( !r_uifullscreen->integer )
  {
    v1 = v0 | 0x100;
    if ( dword_16DCA54 )
    {
      if ( (backEnd_refdef_rdflags & 8) != 0 )
      {
        if ( !r_fastsky->integer && (backEnd_refdef_rdflags & 1) == 0 )
        {
          if ( dword_16C4BB0 && dword_16C4BB8 )
          {
            v6 = alpha;
            v5 = blue;
            v4 = green;
            v3 = ::red;
LABEL_42:
            glClearColor(v3, v4, v5, v6);
            v1 |= 0x4000u;
            goto LABEL_43;
          }
          goto LABEL_43;
        }
        v1 |= 0x4000u;
        if ( dword_16C4BB0 )
        {
          glClearColor(::red, green, blue, alpha);
          goto LABEL_43;
        }
        if ( glfogNum <= 0 || !dword_16C4C70 )
          goto LABEL_24;
      }
      else
      {
        if ( glfogNum <= 0 )
          goto LABEL_43;
        if ( !dword_16C4C70 )
          goto LABEL_43;
        if ( (backEnd_refdef_rdflags & 0x20) != 0 ? dword_16C4C40 == 9729 : cg_skybox->integer == 0 )
          v1 |= 0x4000u;
        if ( (v1 & 0x4000) == 0 )
          goto LABEL_43;
      }
    }
    else
    {
      if ( (backEnd_refdef_rdflags & 1) != 0 )
      {
        v1 &= ~0x4000u;
        goto LABEL_43;
      }
      if ( !r_fastsky->integer )
      {
        if ( dword_16C4C70 && dword_16C4C78 )
        {
          v6 = flt_16C4C5C;
          v5 = flt_16C4C58;
          v4 = flt_16C4C54;
          v3 = flt_16C4C50;
          goto LABEL_42;
        }
LABEL_43:
        if ( !v1 )
          goto LABEL_45;
        goto LABEL_44;
      }
      v1 |= 0x4000u;
      if ( !dword_16C4C70 )
      {
LABEL_24:
        red = tr_identityLight * 0.5;
        glClearColor(red, red, red, 1.0);
        goto LABEL_43;
      }
    }
    glClearColor(flt_16C4C50, flt_16C4C54, flt_16C4C58, flt_16C4C5C);
    goto LABEL_43;
  }
  v1 = 256;
LABEL_44:
  glClear(v1);
LABEL_45:
  if ( (backEnd_refdef_rdflags & 4) != 0 )
  {
    reda = (double)(unsigned __int8)backEnd_refdef_time * 0.0039215689;
    glClearColor(reda, reda, reda, 1.0);
    glClear(0x4000u);
    backEnd_isHyperspace = 1;
  }
  else
  {
    backEnd_isHyperspace = 0;
    backEnd_skyRenderedThisView = 0;
  }
}
#endif

/* ---- RB_SetupLight  0x004D63A0 ----  VERIFIED */
void __cdecl RB_SetupLight( float intensity, int lightIndex, int light )
{
  GLenum lightName;
  const GLfloat *diffusePtr;
  float ambient[4];
  float diffuse[4];

  lightName = lightIndex + 0x4000;

  if ( intensity == 1.0 )                           /* 0x004D63A7 */
  {
    qglLightfv( lightName, 0x1200u, (const GLfloat *)( light + 0x14 ) );
    diffusePtr = (const GLfloat *)( light + 0x24 );
  }
  else
  {
    ambient[0] = intensity * *(float *)( light + 0x14 );
    ambient[1] = intensity * *(float *)( light + 0x18 );
    ambient[2] = intensity * *(float *)( light + 0x1C );
    ambient[3] = 1.0f;
    diffuse[0] = intensity * *(float *)( light + 0x24 );
    diffuse[1] = intensity * *(float *)( light + 0x28 );
    diffuse[2] = intensity * *(float *)( light + 0x2C );
    diffuse[3] = 1.0f;
    qglLightfv( lightName, 0x1200u, ambient );
    diffusePtr = diffuse;
  }

  qglLightfv( lightName, 0x1201u, diffusePtr );
  qglLightfv( lightName, 0x1202u, (const GLfloat *)( light + 0x34 ) );
  qglLightfv( lightName, 0x1203u, (const GLfloat *)( light + 0x44 ) );
  qglLightfv( lightName, 0x1204u, (const GLfloat *)( light + 0x54 ) );

  qglLightf( lightName, 0x1205u, *(float *)( light + 0x6C ) );
  qglLightf( lightName, 0x1206u, *(float *)( light + 0x70 ) );
  qglLightf( lightName, 0x1207u, *(float *)( light + 0x60 ) );
  qglLightf( lightName, 0x1208u, *(float *)( light + 0x64 ) );
  qglLightf( lightName, 0x1209u, *(float *)( light + 0x68 ) );

  if ( lightIndex >= glState_numEnabledLights )                /* 0x004D64C5 */
    glEnable( lightName );
}

/* ---- RB_EnableHWLights  0x004D64E0 ----  VERIFIED */
int __cdecl RB_EnableHWLights(int a1, int a2, int a3)
{
  int v4;
  double v5;
  double v6;
  int v7;
  int v8;
  bool v9; // zf
  float *v10;
  int **v11;
  int v12;
  float *v13;
  int *v14;
  int i;
  int v16;
  int v17;
  int v18;
  char v19;
  float v20;
  float *v21;
  int v22;
  int v23;
  float hwLightFrame[11];    /* [esp+18h]..[esp+40h]: R,G,B,A then lights */
  float *const v26 = &hwLightFrame[2];

  if ( glState_currentLightingEntity == backEnd_currentEntity && glState_currentLightingFlags == (*(_DWORD *)(tess_shader + 84) & 0x18) )
    return 0;
  glState_currentLightingEntity = backEnd_currentEntity;
  glState_currentLightingFlags = *(_DWORD *)(tess_shader + 84) & 0x18;
  v18 = a1;
  v17 = a2;
  v4 = 0;
  v16 = a3;
  hwLightFrame[0] = *(float *)(tr_world + 248) * *(float *)(backEnd_currentEntity + 272) + *(float *)(tr_world + 216);
  hwLightFrame[1] = *(float *)(tr_world + 252) * *(float *)(backEnd_currentEntity + 272) + *(float *)(tr_world + 220);
  v26[0] = *(float *)(tr_world + 256) * *(float *)(backEnd_currentEntity + 272) + *(float *)(tr_world + 224);
  if ( (*(_BYTE *)(backEnd_currentEntity + 4) & 0x20) != 0 )
  {
    v20 = v26[0] * 0.114 + hwLightFrame[1] * 0.58700001 + hwLightFrame[0] * 0.29899999;
    v5 = tr_identityLight * r_entMinLight->value;
    if ( (v20 == 0.0) | __UNORDERED__(v20, 0.0) )
    {
      hwLightFrame[0] = v5;
      hwLightFrame[1] = v5;
      v26[0] = v5;
    }
    else if ( (v20 < v5) | __UNORDERED__(v20, v5) )
    {
      v6 = v5 / v20;
      hwLightFrame[0] = hwLightFrame[0] * v6;
      hwLightFrame[1] = hwLightFrame[1] * v6;
      v26[0] = v6 * v26[0];
    }
  }
  glLightModelfv(0xB53u, hwLightFrame);
  v7 = *(_DWORD *)(backEnd_currentEntity + 204);
  v8 = 0;
  v9 = v7 == 0;
  v23 = v7;
  if ( v7 > 0 )
  {
    v10 = v26;
    v21 = v26;
    v11 = (int **)(backEnd_currentEntity + 208);
    do
    {
      v12 = v8;
      if ( v8 > 0 )
      {
        v22 = **v11;
        v13 = v10;
        do
        {
          v14 = **(int ***)v13;
          if ( v22 > *v14 )
            break;
          if ( *v11 > v14 )
            break;
          v13[1] = *v13;
          --v12;
          --v13;
        }
        while ( v12 > 0 );
      }
      LODWORD(v26[v12 + 1]) = v11;
      ++v8;
      v10 = v21 + 1;
      v11 += 2;
      ++v21;
    }
    while ( v8 < v23 );
    v4 = 0;
    v9 = v23 == 0;
  }
  if ( !v9 )
  {
    glLoadMatrixf(tr_viewParms_world_modelMatrix);
    v4 = 1;
  }
  for ( i = 0; i < *(_DWORD *)(backEnd_currentEntity + 204); ++i )
    RB_SetupLight(*(float *)(LODWORD(v26[i + 1]) + 4),
                  i,
                  *(int *)LODWORD(v26[i + 1]));
  for ( ; i < glState_numEnabledLights; ++i )
    glDisable(i + 0x4000);
  glState_numEnabledLights = *(_DWORD *)(backEnd_currentEntity + 204);
  return v4;
}

/* ---- RB_RenderDrawSurfList  0x004D6710 ----  [HIGH] */
void __cdecl RB_RenderDrawSurfList(int a1, int a2)
{
  int v3;
  int *v4;
  unsigned int v5;
  void *v6;
  int v7;
  int v8;
  int v9;
  int v10;
  int v11;
  double v12;
  float *v13;
  int *v14;
  double v15;
  int v16;
  GLenum v17;
  GLenum v18;
  int v19;
  bool v20; // zf
  int v21;
  GLclampd v22;
  int v23;
  int v24;
  float v25;
  int v26;
  int *v27;
  int v28;
  int v29;
  int v30;
  float *v31;
  int v32;
  int v33;
  int v34;
  int v35;

  v25 = backEnd_refdef_floatTime;
  RB_BeginDrawingView();
  v3 = -1;
  v30 = 0;
  v26 = -1;
  backEnd_currentEntity = (int)&tr_worldEntity;
  v24 = 0;
  v31 = 0;
  v28 = 0;
  v23 = 0;
  LOBYTE(v32) = 0;
  v29 = -1;
  backEnd_pc_surfaceCount += a1;
  if ( a1 > 0 )
  {
    v4 = (int *)(a2 + 4);
    v27 = (int *)(a2 + 4);
    v33 = a1;
    while ( 1 )
    {
      v5 = *(v4 - 1);
      if ( v5 != v3 )
        break;
      rb_surfaceTable[*(_DWORD *)*v4]((void *)*v4);
LABEL_78:
      v4 += 2;
      v20 = v33 == 1;
      v27 = v4;
      --v33;
      if ( v20 )
      {
        if ( v24 )
          RB_EndSurface();
        goto LABEL_81;
      }
    }
    v7 = (v5 >> 8) & 0x3FF;
    v8 = dword_16CF84C[(v5 >> 18) & 0xFFF];
    v9 = v5 >> 30;
    v10 = *(v4 - 1) & 1;
    v11 = (v5 >> 2) & 1;
    v34 = v5;
    v35 = v7;
    if ( (v5 & 2) != 0 )
    {
      v35 = 1022;
      v7 = 1022;
    }
    if ( v8 != v24 || v9 != v30 || v10 != v28 || v11 != v29 )
      goto LABEL_13;
    if ( v7 == v26 )
      goto LABEL_57;
    if ( (*(_BYTE *)(v8 + 76) & 4) == 0 )
    {
LABEL_13:
      if ( v24 )
      {
        RB_EndSurface();
        v7 = v35;
        v32 = *(_DWORD *)(v24 + 76) ^ *(_DWORD *)(v8 + 76);
      }
      if ( v7 == 1022 )
        backEnd_currentEntity = (int)&tr_worldEntity;
      else
        backEnd_currentEntity = backEnd_refdef_entities + 696 * v7;
      if ( v9 != glState_currentStorageMode )
      {
        if ( glConfig_NVVertexArrayRange )
        {
          RB_SelectStorageNV(v9);
        }
        else if ( glConfig_ATIVertexArrayObject )
        {
          RB_SelectStorageATI(v9);
        }
        glState_currentStorageMode = v9;
      }
      RB_BeginSurface((void *)v8, 3);   /* 0x004D6886: push 3 / mov ecx,ebp */
      v7 = v35;
      v24 = v8;
      v28 = v10;
      v29 = v11;
      v30 = v9;
    }
    if ( v7 != v26 )
    {
      v12 = v25;
      v13 = 0;
      v23 = 0;
      if ( v7 == 1022 )
      {
        backEnd_refdef_floatTime = v25;
        backEnd_currentEntity = (int)&tr_worldEntity;
        qmemcpy(&backEnd_or, &backEnd_viewParms_world, 0x7Cu);
        tess_shaderTime = v12 - *(float *)(tess_shader + 380);
        R_TransformDlights(backEnd_refdef_num_dlights, (float *)backEnd_refdef_dlights, &backEnd_or);
        if ( glState_normalizeTarget )
        {
          glDisable(glState_normalizeTarget);
          glState_normalizeTarget = 0;
        }
      }
      else
      {
        v14 = (int *)(backEnd_refdef_entities + 696 * v7);
        backEnd_currentEntity = (int)v14;
        v15 = v12 - *((float *)v14 + 30);
        backEnd_refdef_floatTime = v15;
        tess_shaderTime = v15 - *(float *)(tess_shader + 380);
        if ( (*(_BYTE *)(tess_shader + 76) & 4) != 0 || (v16 = *v14, *v14 != 1) && v16 != 2 && v16 )
        {
          qmemcpy(&backEnd_or, &backEnd_viewParms_world, 0x7Cu);
          v14 = &backEnd_pc_surfaceCount;
        }
        else
        {
          v13 = backEnd_viewParms_originX;
          R_RotateForModelEntity(&backEnd_or, backEnd_viewParms_originX, (int)v14);
        }
        if ( *(_DWORD *)(backEnd_currentEntity + 156) )
          R_TransformDlights(backEnd_refdef_entityDlightCount, (float *)backEnd_refdef_dlights, &backEnd_or);
        v23 = *(_DWORD *)(backEnd_currentEntity + 4) & 0x18;
        if ( (*(_BYTE *)(tess_shader + 84) & 0x18) != 0 )
        {
          RB_EnableHWLights((int)v13, 0, (int)v14);
          v17 = *(_DWORD *)(backEnd_currentEntity + 684);
          if ( v17 != glState_normalizeTarget )
          {
            if ( glState_normalizeTarget )
              glDisable(glState_normalizeTarget);
            glState_normalizeTarget = v17;
            if ( v17 )
              glEnable(v17);
          }
        }
        else if ( glState_normalizeTarget )
        {
          glDisable(glState_normalizeTarget);
          glState_normalizeTarget = 0;
        }
        v13 = (float *)v23;
      }
      glLoadMatrixf(flt_16D8DE4);
      if ( v31 != v13 )
      {
        switch ( (unsigned int)v13 )
        {
          case 0u:
            HIDWORD(v22) = 1072693248;
            goto LABEL_51;
          case 8u:
            glMatrixMode(0x1701u);
            glLoadMatrixf(backEnd_viewParms_depthHackProjectionMatrix);
            glMatrixMode(0x1700u);
            glDepthRange(0.0, 0.2000000029802322);
            break;
          case 0x10u:
          case 0x18u:
            HIDWORD(v22) = 1071644672;
LABEL_51:
            LODWORD(v22) = 0;
            glDepthRange(0.0, v22);
            break;
          default:
            break;
        }
        if ( v31 == (float *)8 )
        {
          glMatrixMode(0x1701u);
          glLoadMatrixf(backEnd_viewParms_projectionMatrix);
          glMatrixMode(0x1700u);
        }
        v31 = v13;
      }
      v26 = v35;
LABEL_77:
      /* 0x004D6BA6 -- the same dispatch on the second path. */
      rb_surfaceTable[*(_DWORD *)*v27]((void *)*v27);
      v3 = v34;
      v4 = v27;
      goto LABEL_78;
    }
LABEL_57:
    if ( (*(_BYTE *)(tess_shader + 84) & 0x18) != 0 )
    {
      if ( RB_EnableHWLights(v10, v8, v11) )
        glLoadMatrixf(flt_16D8DE4);
      v18 = *(_DWORD *)(backEnd_currentEntity + 684);
      if ( v18 != glState_normalizeTarget )
      {
        if ( glState_normalizeTarget )
          glDisable(glState_normalizeTarget);
        glState_normalizeTarget = v18;
        if ( v18 )
          glEnable(v18);
      }
    }
    else
    {
      if ( !glState_normalizeTarget )
        goto LABEL_68;
      glDisable(glState_normalizeTarget);
      glState_normalizeTarget = 0;
    }
    v7 = v35;
LABEL_68:
    if ( v7 != 1022 && (v32 & 4) != 0 )
    {
      if ( (*(_BYTE *)(tess_shader + 76) & 4) != 0
        || (v19 = *(_DWORD *)backEnd_currentEntity, *(_DWORD *)backEnd_currentEntity != 1) && v19 != 2 && v19 )
      {
        qmemcpy(&backEnd_or, &backEnd_viewParms_world, 0x7Cu);
      }
      else
      {
        R_RotateForModelEntity(&backEnd_or, backEnd_viewParms_originX, backEnd_currentEntity);
      }
      glLoadMatrixf(flt_16D8DE4);
    }
    goto LABEL_77;
  }
LABEL_81:
  v21 = storageClass;
  if ( storageClass != glState_currentStorageMode )
  {
    if ( glConfig_NVVertexArrayRange )
    {
      RB_SelectStorageNV(storageClass);
    }
    else if ( glConfig_ATIVertexArrayObject )
    {
      RB_SelectStorageATI(storageClass);
    }
    glState_currentStorageMode = v21;
  }
  backEnd_refdef_floatTime = v25;
  backEnd_currentEntity = (int)&tr_worldEntity;
  qmemcpy(&backEnd_or, &backEnd_viewParms_world, 0x7Cu);
  R_TransformDlights(backEnd_refdef_num_dlights, (float *)backEnd_refdef_dlights, &backEnd_or);
  glLoadMatrixf(backEnd_viewParms_worldModelMatrix);
  if ( v23 )
    glDepthRange(0.0, 1.0);
  RB_ShadowFinish();
  RB_RenderFlares();
}

/* ---- RB_SetGL2D  0x004D6CB0 ----  VERIFIED */
int RB_SetGL2D( void )
{
	int msec;

	backEnd_projection2D = 1;
	RB_EndMultitexture();
	glViewport( 0, 0, dwStyle, dwExStyle );
	glScissor( 0, 0, dwStyle, dwExStyle );
	glMatrixMode( 0x1701u );
	qglLoadIdentity();
	qglOrtho( 0.0, (double)dwStyle, (double)dwExStyle,
	               0.0, 0.0, 1.0 );
	glMatrixMode( 0x1700u );
	qglLoadIdentity();
	GL_State( 0x10065 );

	if ( glState_glStateBits & 0x200000 )
	{
		glDisable( 0xB60u );
		glState_glStateBits &= ~0x200000;
	}
	if ( glState_faceCulling != 2 )
	{
		glDisable( 0xB44u );
		glState_faceCulling = 2;
	}
	glDisable( 0x3000u );

	msec = ri_Milliseconds();
	backEnd_refdef_time = msec;
	backEnd_refdef_floatTime = (float)( (double)msec * 0.001f );
	return msec;
}

#if 0
int RB_SetGL2D()
{
  int result;

  backEnd_projection2D = 1;
  RB_EndMultitexture();
  glViewport(0, 0, dwStyle, dwExStyle);
  glScissor(0, 0, dwStyle, dwExStyle);
  glMatrixMode(0x1701u);
  qglLoadIdentity();
  qglOrtho(
    0,
    0,
    COERCE_UNSIGNED_INT64((double)(int)dwStyle),
    HIDWORD(COERCE_UNSIGNED_INT64((double)(int)dwStyle)),
    COERCE_UNSIGNED_INT64((double)(int)dwExStyle),
    HIDWORD(COERCE_UNSIGNED_INT64((double)(int)dwExStyle)),
    0,
    0,
    0,
    0,
    0,
    1072693248);
  glMatrixMode(0x1700u);
  qglLoadIdentity();
  GL_State(65637);
  if ( (glState_glStateBits & 0x200000) != 0 )
  {
    glDisable(0xB60u);
    glState_glStateBits &= ~0x200000u;
  }
  if ( glState_faceCulling != 2 )
  {
    glDisable(0xB44u);
    glState_faceCulling = 2;
  }
  glDisable(0x3000u);
  result = ri_Milliseconds();
  backEnd_refdef_time = result;
  backEnd_refdef_floatTime = (double)result * 0.001;
  return result;
}
#endif

/* ---- RE_StretchRaw  0x004D6DB0 ----  VERIFIED */
void __cdecl RE_StretchRaw(
        const char *a1,
        int a2,
        int a3,
        int a4,
        int a5,
        GLsizei width,
        GLsizei height,
        GLvoid *pixels,
        int a9,
        int a10)
{
  cvar_t *v10;
  _DWORD *v11;
  char v12;
  char v13;
  unsigned __int16 *v14;
  int v15;
  char v16;
  double v17;
  GLsizei v18;
  double v19;
  char *v20;
  int v21;
  double v22;
  char *v23;
  double v24;
  char *v25;
  char *v26;
  int v27;
  int v28;
  int v29;
  float v30;
  int v31;

  if ( tr_registered )
  {
    v10 = r_skipBackEnd;
    v11 = (_DWORD *)(backEndData + 1636096);
    *(_DWORD *)((char *)v11 + *(_DWORD *)(backEndData + 1898240)) = 0;
    v11[0x10000] = 0;
    if ( !v10->integer )
      RB_ExecuteRenderCommands(v11, a1);
    qglFinish();
    v29 = 0;
    if ( r_speeds->integer )
      v29 = ri_Milliseconds();
    v12 = 0;
    if ( width > 1 )
    {
      do
        ++v12;
      while ( 1 << v12 < width );
    }
    v13 = 0;
    if ( height > 1 )
    {
      do
        ++v13;
      while ( 1 << v13 < height );
    }
    if ( 1 << v12 != width || 1 << v13 != height )
      ri_Error(1, "\x15" "Draw_StretchRaw: size not a power of 2: %i by %i",
               width, height);
    GL_Bind((GLenum *)tr_scratchImages[a9]);
    v14 = (unsigned __int16 *)tr_scratchImages[a9];
    if ( width == v14[32] && height == v14[33] )
    {
      if ( a10 )
        glTexSubImage2D(0xDE1u, 0, 0, 0, width, height, 0x1908u, 0x1401u, pixels);
    }
    else
    {
      v14[34] = width;
      *(_WORD *)(tr_scratchImages[a9] + 64) = width;
      *(_WORD *)(tr_scratchImages[a9] + 70) = height;
      *(_WORD *)(tr_scratchImages[a9] + 66) = height;
      glTexImage2D(0xDE1u, 0, 3, width, height, 0, 0x1908u, 0x1401u, pixels);
      glTexParameteri(0xDE1u, 0x2801u, 9729);
      glTexParameteri(0xDE1u, 0x2800u, 9729);
      glTexParameteri(0xDE1u, 0x2802u, 33071);
      glTexParameteri(0xDE1u, 0x2803u, 33071);
    }
    if ( r_speeds->integer )
    {
      v15 = ri_Milliseconds();
      ri_Printf(0, "qglTexSubImage2D %i, %i: %i msec\n", width, height, v15 - v29);
    }
    RB_SetGL2D();
    RB_BeginImmediateMode();
    v16 = (unsigned __int64)(tr_identityLight * 255.0);
    v30 = (float)width;
    rbDebug_immediateColorG = v16;
    rbDebug_immediateColorB = v16;
    v17 = 0.5 / v30;
    v18 = rbDebug_immediateVertexCount;
    rbDebug_immediateColorR = v16;
    rbDebug_immediateColorA = -1;
    mode = 7;
    *(float *)&rbDebug_immediateTexCoordS = v17;
    v19 = (double)height;
    *(float *)&rbDebug_immediateTexCoordT = 0.5 / v19;
    if ( rbDebug_immediateVertexCount < rbDebug_immediateVertexCapacity )
    {
      v20 = (char *)rbDebug_immediateVertices + 32 * rbDebug_immediateVertexCount;
      v20[28] = v16;
      v20[29] = rbDebug_immediateColorG;
      v20[30] = rbDebug_immediateColorB;
      v20[31] = rbDebug_immediateColorA;
      *((_DWORD *)v20 + 3) = rbDebug_immediateTexCoordS;
      v21 = rbDebug_immediateTexCoordT;
      *(float *)v20 = (float)a2;
      *((_DWORD *)v20 + 4) = v21;
      *((_DWORD *)v20 + 2) = 0;
      *((float *)v20 + 1) = (float)a3;
      v16 = rbDebug_immediateColorR;
      v18 = ++rbDebug_immediateVertexCount;
    }
    *(float *)&v31 = 0.5 / v19;
    rbDebug_immediateTexCoordT = v31;
    v22 = (v30 - 0.5) / v30;
    *(float *)&rbDebug_immediateTexCoordS = v22;
    if ( v18 < rbDebug_immediateVertexCapacity )
    {
      v23 = (char *)rbDebug_immediateVertices + 32 * v18;
      v23[28] = v16;
      v23[29] = rbDebug_immediateColorG;
      v23[30] = rbDebug_immediateColorB;
      v23[31] = rbDebug_immediateColorA;
      *((_DWORD *)v23 + 3) = rbDebug_immediateTexCoordS;
      *((_DWORD *)v23 + 4) = rbDebug_immediateTexCoordT;
      *((_DWORD *)v23 + 2) = 0;
      *(float *)v23 = (float)(a4 + a2);
      *((float *)v23 + 1) = (float)a3;
      v16 = rbDebug_immediateColorR;
      v18 = ++rbDebug_immediateVertexCount;
    }
    *(float *)&rbDebug_immediateTexCoordS = v22;
    v24 = (v19 - 0.5) / v19;
    *(float *)&rbDebug_immediateTexCoordT = v24;
    if ( v18 < rbDebug_immediateVertexCapacity )
    {
      v25 = (char *)rbDebug_immediateVertices + 32 * v18;
      v25[28] = v16;
      v25[29] = rbDebug_immediateColorG;
      v25[30] = rbDebug_immediateColorB;
      v25[31] = rbDebug_immediateColorA;
      *((_DWORD *)v25 + 3) = rbDebug_immediateTexCoordS;
      *((_DWORD *)v25 + 4) = rbDebug_immediateTexCoordT;
      *(float *)v25 = (float)(a4 + a2);
      *((_DWORD *)v25 + 2) = 0;
      *((float *)v25 + 1) = (float)(a3 + a5);
      v16 = rbDebug_immediateColorR;
      v18 = ++rbDebug_immediateVertexCount;
    }
    *(float *)&rbDebug_immediateTexCoordS = v17;
    *(float *)&rbDebug_immediateTexCoordT = v24;
    if ( v18 < rbDebug_immediateVertexCapacity )
    {
      v26 = (char *)rbDebug_immediateVertices + 32 * v18;
      v26[28] = v16;
      v26[29] = rbDebug_immediateColorG;
      v26[30] = rbDebug_immediateColorB;
      v26[31] = rbDebug_immediateColorA;
      *((_DWORD *)v26 + 3) = rbDebug_immediateTexCoordS;
      v27 = rbDebug_immediateTexCoordT;
      *(float *)v26 = (float)a2;
      *((_DWORD *)v26 + 4) = v27;
      *((_DWORD *)v26 + 2) = 0;
      *((float *)v26 + 1) = (float)(a5 + a3);
      v18 = ++rbDebug_immediateVertexCount;
    }
    glDrawArrays(mode, 0, v18);
    rbDebug_immediateVertexCount = 0;
    mode = 0;
    rbDebug_immediateModeActive = 0;
    if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
    {
      if ( r_fog->integer )
      {
        v28 = dword_16C4BB0;
        if ( (backEnd_refdef_rdflags & 8) == 0 )
          v28 = glfogNum;
        if ( v28 )
        {
          glEnable(0xB60u);
          glState_glStateBits |= 0x200000u;
        }
      }
    }
    glVertexPointer(3, 0x1406u, 0, tess_xyz);
  }
}

/* ---- RE_UploadCinematic  0x004D7290 ----  VERIFIED */
void __cdecl RE_UploadCinematic(int a1, int a2, GLsizei width, GLsizei height, GLvoid *pixels, int a6, int a7)
{
  unsigned __int16 *v7;

  GL_Bind((GLenum *)tr_scratchImages[a6]);
  v7 = (unsigned __int16 *)tr_scratchImages[a6];
  if ( width == v7[32] && height == v7[33] )
  {
    if ( a7 )
      glTexSubImage2D(0xDE1u, 0, 0, 0, width, height, 0x1908u, 0x1401u, pixels);
  }
  else
  {
    v7[34] = width;
    *(_WORD *)(tr_scratchImages[a6] + 64) = width;
    *(_WORD *)(tr_scratchImages[a6] + 70) = height;
    *(_WORD *)(tr_scratchImages[a6] + 66) = height;
    glTexImage2D(0xDE1u, 0, 3, width, height, 0, 0x1908u, 0x1401u, pixels);
    glTexParameteri(0xDE1u, 0x2801u, 9729);
    glTexParameteri(0xDE1u, 0x2800u, 9729);
    glTexParameteri(0xDE1u, 0x2802u, 33071);
    glTexParameteri(0xDE1u, 0x2803u, 33071);
  }
}

/* ---- RB_SetColor  0x004D7390 ----  VERIFIED */
int __cdecl RB_SetColor(int a1)
{
  backEnd_color2D = *(_DWORD *)(a1 + 4);
  return a1 + 8;
}

/* ---- RB_DrawStretchPic  0x004D73A0 ----  VERIFIED */
int __cdecl RB_DrawStretchPic(
        int a1,
        void *a3,
        float a4,
        float a5,
        float a6,
        float a7,
        int a8,
        int a9,
        int a10,
        int a11,
        int *a12)
{
  int v12;
  double v13;
  unsigned __int16 v14;
  int v15;
  int v16;
  int v17;
  float *v18;
  _DWORD *v19;
  double v20;
  int result;

  if ( !backEnd_projection2D )
    RB_SetGL2D();

  if ( a1 != tess_shader )
  {
    if ( tess_numIndexes )
      RB_EndSurface();
    backEnd_currentEntity = (int)&backEnd_entity2D;
    v12 = storageClass;
    if ( storageClass != glState_currentStorageMode )
    {
      if ( glConfig_NVVertexArrayRange )
      {
        RB_SelectStorageNV(storageClass);
      }
      else if ( glConfig_ATIVertexArrayObject )
      {
        RB_SelectStorageATI(storageClass);
      }
      glState_currentStorageMode = v12;
    }
    RB_BeginSurface((void *)a1, 3);   /* 0x004D740B: push 3 / mov ecx,ebx */
  }
  if ( tess_numVertexes + 4 >= 0x2000 || tess_numIndexes + 6 >= 49152 )
    RB_CheckOverflow(4, 6);
  v13 = a4 + a6;
  v14 = tess_numVertexes;
  tess_numVertexes += 4;
  v15 = tess_numIndexes;
  tess_numIndexes += 6;
  tess_indexes[v15] = v14 + 3;
  word_17A7F62[v15] = v14;
  word_17A7F68[v15] = v14;
  word_17A7F64[v15] = v14 + 2;
  word_17A7F66[v15] = v14 + 2;
  word_17A7F6A[v15] = v14 + 1;
  v16 = v14;
  v17 = *a12;
  dword_181FF6C[v16] = *a12;
  dword_181FF68[v16] = v17;
  dword_181FF64[v16] = v17;
  dword_181FF60[v16] = v17;
  v18 = (float *)(4 * tess_vertexComponentCount * v16 + ((int)(char *)tess_xyz));
  *v18 = a4;
  v18[1] = a5;
  ++v18;
  v18[1] = 0.0;
  dword_17DFF60[2 * v16] = a8;
  ++v18;
  v19 = (_DWORD *)(8 * v16 + ((int)(char *)tess_texCoords0));
  v19[1] = a9;
  v18[1] = v13;
  ++v18;
  v18[1] = a5;
  ++v19;
  ++v18;
  v18[1] = 0.0;
  v19[1] = a10;
  ++v18;
  ++v19;
  v19[1] = a9;
  v18[1] = v13;
  v20 = a5 + a7;
  v18 += 3;
  v19 += 2;
  *(v18 - 1) = v20;
  *v18 = 0.0;
  *v19 = a10;
  v19[1] = a11;
  v18[1] = a4;
  v19 += 2;
  v18 += 2;
  *v18 = v20;
  v18[1] = 0.0;
  result = a8;
  *v19 = a8;
  v19[1] = a11;
  return result;
}

/* ---- RB_StretchPic  0x004D75A0 ----  VERIFIED */
int __cdecl RB_StretchPic(int a2)
{
  RB_DrawStretchPic(
    *(_DWORD *)(a2 + 4),
    *(void **)(a2 + 8),
    *(float *)(a2 + 8),
    *(float *)(a2 + 12),
    *(float *)(a2 + 16),
    *(float *)(a2 + 20),
    *(_DWORD *)(a2 + 24),
    *(_DWORD *)(a2 + 28),
    *(_DWORD *)(a2 + 32),
    *(_DWORD *)(a2 + 36),
    &backEnd_color2D);
  return a2 + 40;
}

/* ---- RB_StretchPicGradient  0x004D75E0 ----  VERIFIED */
int __cdecl RB_StretchPicGradient(int a1, void *a2)
{
  int v2;
  unsigned __int16 v3;
  int v4;
  int v5;
  int v6;
  int v7;
  int v8;
  float *v9;
  _DWORD *v10;

  if ( !backEnd_projection2D )
    RB_SetGL2D();
  if ( *(_DWORD *)(a1 + 4) != tess_shader )
  {
    if ( tess_numIndexes )
      RB_EndSurface();
    backEnd_currentEntity = (int)&backEnd_entity2D;
    v2 = storageClass;
    if ( storageClass != glState_currentStorageMode )
    {
      if ( glConfig_NVVertexArrayRange )
      {
        RB_SelectStorageNV(storageClass);
      }
      else if ( glConfig_ATIVertexArrayObject )
      {
        RB_SelectStorageATI(storageClass);
      }
      glState_currentStorageMode = v2;
    }
    RB_BeginSurface(*(void **)(a1 + 4), 3);   /* 0x004D7650: push 3 / mov ecx,ebp; ebp = [edi+4] */
  }
  if ( tess_numVertexes + 4 >= 0x2000 || tess_numIndexes + 6 >= 49152 )
    RB_CheckOverflow(4, 6);
  v3 = tess_numVertexes;
  tess_numVertexes += 4;
  v4 = tess_numIndexes;
  tess_numIndexes += 6;
  word_17A7F62[v4] = v3;
  word_17A7F68[v4] = v3;
  tess_indexes[v4] = v3 + 3;
  word_17A7F64[v4] = v3 + 2;
  word_17A7F66[v4] = v3 + 2;
  word_17A7F6A[v4] = v3 + 1;
  v5 = v3;
  v6 = backEnd_color2D;
  dword_181FF64[v5] = backEnd_color2D;
  dword_181FF60[v5] = v6;   /* +0 shard, see RB_DrawStretchPic; 0x004D76EF */
  v7 = *(_DWORD *)(a1 + 40);
  dword_181FF6C[v5] = v7;
  dword_181FF68[v5] = v7;
  v8 = tess_vertexComponentCount * v5;
  dword_17BFF60[v8] = *(_DWORD *)(a1 + 8);
  v9 = (float *)(4 * v8 + ((int)(char *)tess_xyz));
  v9[1] = *(float *)(a1 + 12);
  ++v9;
  v9[1] = 0.0;
  dword_17DFF60[2 * v5] = *(_DWORD *)(a1 + 24);
  v10 = (_DWORD *)(8 * v5 + ((int)(char *)tess_texCoords0));
  v10[1] = *(_DWORD *)(a1 + 28);
  v9 += 4;
  v10 += 2;
  *(v9 - 2) = *(float *)(a1 + 16) + *(float *)(a1 + 8);
  *(v9 - 1) = *(float *)(a1 + 12);
  *v9 = 0.0;
  *v10 = *(_DWORD *)(a1 + 32);
  v10[1] = *(_DWORD *)(a1 + 28);
  v9 += 3;
  *(v9 - 2) = *(float *)(a1 + 16) + *(float *)(a1 + 8);
  v10 += 4;
  v9 += 2;
  *(v9 - 3) = *(float *)(a1 + 12) + *(float *)(a1 + 20);
  *(v9 - 2) = 0.0;
  *(v10 - 2) = *(_DWORD *)(a1 + 32);
  *(v10 - 1) = *(_DWORD *)(a1 + 36);
  *(v9 - 1) = *(float *)(a1 + 8);
  *v9 = *(float *)(a1 + 12) + *(float *)(a1 + 20);
  v9[1] = 0.0;
  *v10 = *(_DWORD *)(a1 + 24);
  v10[1] = *(_DWORD *)(a1 + 36);
  return a1 + 48;
}

/* ---- RB_StretchPicRotate  0x004D77E0 ----  VERIFIED */
int __cdecl RB_StretchPicRotate(int a1, void *a2)
{
  int v2;
  unsigned __int16 v3;
  int v4;
  int v5;
  int v6;
  double v7;
  int v8;
  double v9;
  double v10;
  double v11;
  double v12;
  int v13;
  int v14;
  float v16; // [esp+Ch] [ebp-28h] BYREF
  float v17[3]; // [esp+10h] [ebp-24h] BYREF
  float v18;
  float v19;
  float v20;
  float v21;
  float v22;
  float v23;

  if ( !backEnd_projection2D )
    RB_SetGL2D();
  if ( *(_DWORD *)(a1 + 4) != tess_shader )
  {
    if ( tess_numIndexes )
      RB_EndSurface();
    backEnd_currentEntity = (int)&backEnd_entity2D;
    v2 = storageClass;
    if ( storageClass != glState_currentStorageMode )
    {
      if ( glConfig_NVVertexArrayRange )
      {
        RB_SelectStorageNV(storageClass);
      }
      else if ( glConfig_ATIVertexArrayObject )
      {
        RB_SelectStorageATI(storageClass);
      }
      glState_currentStorageMode = v2;
    }
    RB_BeginSurface(*(void **)(a1 + 4), 3);   /* 0x004D7852: push 3 / mov ecx,ebp; ebp = [edi+4] */
  }
  if ( tess_numVertexes + 4 >= 0x2000 || tess_numIndexes + 6 >= 49152 )
    RB_CheckOverflow(4, 6);
  v3 = tess_numVertexes;
  v4 = tess_numIndexes;
  tess_numVertexes += 4;
  tess_numIndexes += 6;
  tess_indexes[v4] = v3 + 3;
  word_17A7F62[v4] = v3;
  word_17A7F68[v4] = v3;
  word_17A7F64[v4] = v3 + 2;
  word_17A7F66[v4] = v3 + 2;
  word_17A7F6A[v4] = v3 + 1;
  v5 = v3;
  v6 = backEnd_color2D;
  dword_181FF6C[v5] = backEnd_color2D;
  dword_181FF68[v5] = v6;
  dword_181FF64[v5] = v6;
  dword_181FF60[v5] = v6;
  dword_17DFF60[2 * v5] = *(_DWORD *)(a1 + 24);
  dword_17DFF64[2 * v5] = *(_DWORD *)(a1 + 28);
  dword_17DFF68[2 * v5] = *(_DWORD *)(a1 + 32);
  dword_17DFF6C[2 * v5] = *(_DWORD *)(a1 + 28);
  dword_17DFF70[2 * v5] = *(_DWORD *)(a1 + 32);
  dword_17DFF74[2 * v5] = *(_DWORD *)(a1 + 36);
  dword_17DFF78[2 * v5] = *(_DWORD *)(a1 + 24);
  dword_17DFF7C[2 * v5] = *(_DWORD *)(a1 + 36);
  v7 = *(float *)(a1 + 16) * 0.5;
  LODWORD(v17[2]) = v17;
  LODWORD(v17[1]) = &v16;
  v20 = v7;
  v21 = *(float *)(a1 + 20) * 0.5;
  v22 = v20 + *(float *)(a1 + 8);
  v23 = v21 + *(float *)(a1 + 12);
  v18 = *(float *)(a1 + 40) * 3.1415927 * 0.0055555557;
  v17[0] = cos(v18);
  v16 = sin(v18);
  v8 = tess_vertexComponentCount * v5;
  v9 = v17[0] * v20;
  v10 = v16 * v20;
  v18 = -(v16 * v21);
  v19 = v17[0] * v21;
  v11 = v22 - v9;
  v17[0] = v11;
  *(float *)&tess_xyz[v8] = v11 - v18;
  v12 = v23 - v10;
  v16 = v12;
  v13 = 4 * v8 + ((int)(char *)tess_xyz + 36);
  *(float *)(v13 - 32) = v12 - v19;
  *(_DWORD *)(v13 - 28) = 0;
  v14 = 4 * v8 + ((int)(char *)tess_xyz + 40);
  *(float *)(v14 - 28) = v9 + v22 - v18;
  *(float *)(v14 - 24) = v23 + v10 - v19;
  *(_DWORD *)(v14 - 20) = 0;
  *(float *)(v14 - 16) = v18 + v9 + v22;
  *(float *)(v14 - 12) = v19 + v10 + v23;
  *(_DWORD *)(v14 - 8) = 0;
  *(float *)(v14 - 4) = v17[0] + v18;
  *(float *)v14 = v16 + v19;
  *(_DWORD *)(v14 + 4) = 0;
  return a1 + 44;
}

/* ---- RB_DrawQuadPic  0x004D7AB0 ----  VERIFIED */
_DWORD *__cdecl RB_DrawQuadPic(_DWORD *a1, void *a2)
{
  int v2;
  unsigned __int16 v3;
  int v4;
  int v5;
  int v6;
  _DWORD *v7;

  if ( !backEnd_projection2D )
    RB_SetGL2D();
  if ( a1[1] != tess_shader )
  {
    if ( tess_numIndexes )
      RB_EndSurface();
    backEnd_currentEntity = (int)&backEnd_entity2D;
    v2 = storageClass;
    if ( storageClass != glState_currentStorageMode )
    {
      if ( glConfig_NVVertexArrayRange )
      {
        RB_SelectStorageNV(storageClass);
      }
      else if ( glConfig_ATIVertexArrayObject )
      {
        RB_SelectStorageATI(storageClass);
      }
      glState_currentStorageMode = v2;
    }
    RB_BeginSurface((void *)a1[1], 3);   /* 0x004D7B20: push 3 / mov ecx,ebp; ebp = [edi+4] */
  }
  if ( tess_numVertexes + 4 >= 0x2000 || tess_numIndexes + 6 >= 49152 )
    RB_CheckOverflow(4, 6);
  v3 = tess_numVertexes;
  tess_numVertexes += 4;
  v4 = tess_numIndexes;
  tess_numIndexes += 6;
  word_17A7F62[v4] = v3;
  word_17A7F68[v4] = v3;
  tess_indexes[v4] = v3 + 3;
  word_17A7F64[v4] = v3 + 2;
  word_17A7F66[v4] = v3 + 2;
  word_17A7F6A[v4] = v3 + 1;
  v5 = backEnd_color2D;
  dword_181FF6C[v3] = backEnd_color2D;
  dword_181FF68[v3] = v5;
  dword_181FF64[v3] = v5;
  dword_181FF60[v3] = v5;   /* +0 shard; 0x004D7BCE */
  dword_17DFF60[2 * v3] = a1[10];
  dword_17DFF64[2 * v3] = a1[11];
  dword_17DFF68[2 * v3] = a1[12];
  dword_17DFF6C[2 * v3] = a1[13];
  dword_17DFF70[2 * v3] = a1[14];
  dword_17DFF74[2 * v3] = a1[15];
  dword_17DFF78[2 * v3] = a1[16];
  dword_17DFF7C[2 * v3] = a1[17];
  v6 = tess_vertexComponentCount * v3;
  dword_17BFF60[v6] = a1[2];
  v7 = (_DWORD *)(4 * v6 + ((int)(char *)tess_xyz));
  v7[1] = a1[3];
  ++v7;
  v7[1] = 0;
  ++v7;
  v7[1] = a1[4];
  ++v7;
  v7[1] = a1[5];
  ++v7;
  v7[1] = 0;
  ++v7;
  v7[1] = a1[6];
  ++v7;
  v7[1] = a1[7];
  ++v7;
  v7[1] = 0;
  v7 += 2;
  *v7++ = a1[8];
  *v7 = a1[9];
  v7[1] = 0;
  return a1 + 18;
}

/* ---- RB_Text_PaintChar  0x004D7CB0 ----  VERIFIED */
int __cdecl RB_Text_PaintChar(
        float a2,
        int a3,
        float a4,
        float a5,
        int a6,
        float a7,
        int a8,
        int a9,
        int a10,
        int a11,
        int a12,          /* glyph shader HANDLE, then reused as scratch.
                           * Retail reads it with `mov esi, [esp+arg_28]` at
                           * 0x004D7CBA -- an INTEGER load -- before the `fstp`
                           * at 0x004D7CC3 overwrites the slot with a4*a7.  Must
                           * stay int: as a float the handle round-trips x87 as
                           * a DENORMAL (20 == 0x14 == 2.8e-44) and flushes to
                           * zero, resolving every glyph to tr_shaders[0]. */
        int *a13)
{
  int v13;
  char *v14;

  v13 = a12;
  *(float *)&a12 = a4 * a7;
  a7 = a5 * a7;
  ri_SCR_AdjustFrom640(&a2, &a3, &a12, &a7);
  if ( v13 >= 0 && v13 < tr_numShaders[0] )
  {
    v14 = (char *)tr_shaders[v13];
  }
  else
  {
    ri_Printf(2, "R_GetShaderByHandle: out of range hShader '%d'\n", v13);
    v14 = tr_defaultShader;
  }

  return RB_DrawStretchPic((int)v14, (void *)a12, a2, *(float *)&a3, *(float *)&a12, a7, a8, a9, a10, a11, a13);
}

static const unsigned int s_consoleColorTable[8] =
{
	0xFF000000u, 0xFF0000FFu, 0xFF00FF00u, 0xFF00FFFFu,
	0xFFFF0000u, 0xFFFFFF00u, 0xFFFF00FFu, 0xFFFFFFFFu
};

/* ---- RB_Text_PaintWithCursor  0x004D7D60 ----  VERIFIED */
int __cdecl RB_Text_PaintWithCursor(
        const char *a1,
        int a2,
        float a3,
        int a4,
        float a5,
        int a6,
        int a7,
        unsigned __int8 a8,
        float a9,
        int a10)
{
  int v11;
  int v12;
  int v13;
  const char *v14;
  signed int v15;
  unsigned __int8 v16;
  char v17;
  unsigned int v18;
  unsigned __int8 v19;
  unsigned int v20;
  unsigned __int8 v21;
  char v22;
  char v23;
  int *CharacterGlyph;
  int *v25;
  int v26;
  int v27;
  double v28;
  int v29;  /* glyph shader HANDLE (retail: mov esi, [esp+arg_28]) */
  int v30;
  int v31;
  float v32;
  int v33;
  int v34;  /* glyph shader HANDLE */
  int v35;
  double v36;
  int v37;
  int v38;  /* glyph shader HANDLE */
  int v39;
  float v41;
  int v42;
  int v43;
  int v44;
  float v45;
  float v46;
  float v47;
  float v48;
  int v49;
  float v50;
  float v51;
  float v52;
  float v53;
  float v54;
  int v55;  /* glyph shader HANDLE */
  float v56;
  int v57; // [esp+44h] [ebp-2Ch] BYREF
  float v58;
  float v59;
  float v60;
  signed int v61;
  float AsianScale;
  int v63;
  float v64;
  int v65; // [esp+64h] [ebp-Ch] BYREF
  signed int v66;
  int v67;

  v11 = ri_GetFontInfo(a4, LODWORD(a5));
  v59 = a5 * *(float *)(v11 + 20480);
  AsianScale = 1.0;
  v64 = 0.0;
  if ( g_currentAsian )
  {
    AsianScale = (float)R_GetAsianScale(v11, a5);
    v64 = ((double)g_asianGlyph.height * AsianScale * a5 - a5 * *(float *)(v11 + 20484)) * 0.125;
  }
  v12 = *(_DWORD *)a6;
  v13 = BYTE3(*(_DWORD *)a6);
  v65 = 0;
  BYTE3(v65) = v13;
  v14 = a1;
  v57 = v12;
  v15 = strlen(a1);
  v66 = v15;
  v63 = a2;
  v58 = 0.0;
  v61 = 0;
  while ( v14 )
  {
    v16 = *v14;
    if ( !*v14 || v61 >= v15 )
      break;
    if ( g_currentAsian )
    {
      switch ( cl_language->integer )
      {
        case 8:
          v21 = v14[1];
          if ( v16 >= 0xB0u && v16 <= 0xC8u && v21 > 0xA0u && v21 != 0xFF )
          {
            v18 = v21 + (v16 << 8);
            v14 += 2;
            goto LABEL_33;
          }
          break;
        case 9:
          v19 = v14[1];
          v20 = (unsigned int)(v19 + (v16 << 8)) >> 8;
          if ( ((unsigned __int8)v20 >= 0xA1u && (unsigned __int8)v20 <= 0xC6u
             || (unsigned __int8)v20 >= 0xC9u && (unsigned __int8)v20 <= 0xF9u)
            && (v19 >= 0x40u && v19 <= 0x7Eu || v19 >= 0xA1u && v19 != 0xFF) )
          {
            v18 = *((unsigned __int8 *)v14 + 1) + (v16 << 8);
            v14 += 2;
            goto LABEL_33;
          }
          break;
        case 0xA:
          if ( (v17 = v14[1], v16 >= 0x81u) && v16 <= 0x9Fu || v16 >= 0xE0u && v16 <= 0xEFu )
          {
            if ( (unsigned __int8)v17 >= 0x40u && (unsigned __int8)v17 <= 0x7Eu || v17 <= -4 )
            {
              v18 = (unsigned __int8)v17 + (v16 << 8);
              v14 += 2;
              goto LABEL_33;
            }
          }
          break;
      }
    }
    v18 = v16;
    ++v14;
LABEL_33:
    switch ( v18 )
    {
      case 0x5Eu:
        if ( !v14 )
          goto LABEL_47;
        v22 = *v14;
        if ( *v14 == 94 || (unsigned __int8)v22 < 0x30u || (unsigned __int8)v22 > 0x37u )
          goto LABEL_47;
        if ( (v22 & 7) == 7 )
        {
          v57 = *(_DWORD *)a6;
        }
        else
        {
          v23 = *(_BYTE *)(a6 + 3);
          v57 = s_consoleColorTable[v22 & 7];
          BYTE3(v57) = v23;
        }
        ++v14;
        break;
      case 0xAu:
        a2 = v63;
        a3 = a5 * *(float *)(v11 + 20484) + a3;
        if ( g_currentAsian )
          a3 = a3 + 4.0;
        break;
      case 0xDu:
        a2 = v63;
        break;
      default:
LABEL_47:
        CharacterGlyph = R_GetCharacterGlyph(v18, v11);
        v25 = CharacterGlyph;
        v56 = v59;
        if ( v18 > 0xFF )
          v56 = AsianScale * v59;
        v60 = v56 * *((float *)CharacterGlyph + 2);
        v58 = v56 * *((float *)CharacterGlyph + 3);
        if ( v18 > 0xFF )
          v60 = v60 + v64;
        if ( !((a9 == 0.0) | __UNORDERED__(a9, 0.0)) )
          v58 = (a9 - v56 * *((float *)CharacterGlyph + 4)) * 0.5 + v58;
        if ( a10 == 3 || a10 == 6 )
        {
          v26 = CharacterGlyph[9];
          v55 = v25[11];
          v27 = v25[8];
          v67 = (a10 != 3) + 1;
          v28 = (double)v67;
          v50 = (float)v25[6];
          v46 = (float)v25[5];
          *(float *)&v42 = a3 - v60 + v28;
          v41 = v28 + v58 + *(float *)&a2;
          RB_Text_PaintChar(v41, v42, v46, v50, a4, v56, v25[7], v27, v26, v25[10], v55, &v65);
        }
        v29 = v25[11];
        v30 = v25[10];
        v31 = v25[9];
        *(float *)&v67 = v58 + *(float *)&a2;
        v32 = *(float *)&v67;
        v51 = (float)v25[6];
        v47 = (float)v25[5];
        *(float *)&v43 = a3 - v60;
        RB_Text_PaintChar(*(float *)&v67, v43, v47, v51, a4, v56, v25[7], v25[8], v31, v30, v29, &v57);
        if ( v61 == a7 )
        {
          if ( !sys_timeBaseInit )
          {
            sys_timeBase = timeGetTime();
            sys_timeBaseInit = 1;
          }
          if ( (((int)(timeGetTime() - sys_timeBase) / 256) & 1) != 0 )
          {
            v33 = 80 * a8;
            v34 = *(int *)(v33 + v11 + 44);
            v35 = v11 + v33;
            v52 = (float)*(int *)(v35 + 24);
            v48 = (float)*(int *)(v35 + 20);
            *(float *)&v44 = a3 - v56 * *(float *)(v35 + 8);
            RB_Text_PaintChar(
              v32,
              v44,
              v48,
              v52,
              a4,
              v59,
              *(_DWORD *)(v35 + 28),
              *(_DWORD *)(v35 + 32),
              *(_DWORD *)(v35 + 36),
              *(_DWORD *)(v35 + 40),
              v34,
              &v57);
          }
        }
        if ( (a9 == 0.0) | __UNORDERED__(a9, 0.0) )
          v36 = v56 * *((float *)v25 + 4) + *(float *)&a2;
        else
          v36 = *(float *)&a2 + a9;
        *(float *)&a2 = v36;
        ++v61;
        break;
    }
    v15 = v66;
  }
  if ( a7 == v15 )
  {
    if ( !sys_timeBaseInit )
    {
      sys_timeBase = timeGetTime();
      sys_timeBaseInit = 1;
    }
    if ( (((int)(timeGetTime() - sys_timeBase) / 256) & 1) != 0 )
    {
      v37 = 80 * a8;
      v38 = *(int *)(v37 + v11 + 44);
      v39 = v11 + v37;
      v54 = (float)*(int *)(v39 + 24);
      v53 = (float)*(int *)(v39 + 20);
      *(float *)&v49 = a3 - v59 * *(float *)(v39 + 8);
      v45 = v58 + *(float *)&a2;
      RB_Text_PaintChar(
        v45,
        v49,
        v53,
        v54,
        a4,
        v59,
        *(_DWORD *)(v39 + 28),
        *(_DWORD *)(v39 + 32),
        *(_DWORD *)(v39 + 36),
        *(_DWORD *)(v39 + 40),
        v38,
        &v57);
    }
  }
  RE_SetColor(0);
  return v15;
}

/* ---- RB_Text_Paint  0x004D82B0 ----  VERIFIED */
int __cdecl RB_Text_Paint(int a1)
{
  return a1
       + ((RB_Text_PaintWithCursor(
             (const char *)(a1 + 37),
             *(_DWORD *)(a1 + 4),
             *(float *)(a1 + 8),
             *(_DWORD *)(a1 + 12),
             *(float *)(a1 + 16),
             a1 + 20,
             *(_DWORD *)(a1 + 32),
             *(_BYTE *)(a1 + 36),
             *(float *)(a1 + 24),
             *(_DWORD *)(a1 + 28))
         + 41)
        & 0xFFFFFFFC);
}

/* ---- RB_DrawSurfs  0x004D82F0 ----  VERIFIED */
int *__cdecl RB_DrawSurfs(void *a1, int *a2)
{
  if ( tess_numIndexes )
    RB_EndSurface();
  qmemcpy(&unk_16D89C0, a2 + 1, 0x188u);
  qmemcpy(backEnd_viewParms_originX, a2 + 99, 0x260u);
  RB_RenderDrawSurfList(a2[252], a2[251]);
  return a2 + 253;
}

/* ---- RB_DrawBuffer  0x004D8350 ----  VERIFIED */
int __cdecl RB_DrawBuffer(int a1)
{
  GLclampf blue;

  glDrawBuffer(*(_DWORD *)(a1 + 4));
  if ( r_clear->integer )
  {
    blue = tr_identityLight * 0.5;
    glClearColor(tr_identityLight, 0.0, blue, 1.0);
    glClear(0x4100u);
  }
  return a1 + 8;
}

/* ---- RB_ShowImages  0x004D83A0 ----  [HIGH] */
void RB_ShowImages()
{
  int i;
  int v1;
  double v2;
  double v3;
  double v4;
  int v5;
  int v6;
  int v7;
  int v8;

  if ( !backEnd_projection2D )
    RB_SetGL2D();
  glClear(0x4000u);
  qglFinish();
  v8 = ri_Milliseconds();
  GL_Bind((GLenum *)tr_screenImage);
  glCopyTexSubImage2D(0xDE1u, 0, 0, 0, 0, 0, dwStyle, dwExStyle);
  for ( i = 0; i < tr_numImages; ++i )
  {
    v1 = dword_16C9840[i];
    if ( !*(_DWORD *)(v1 + 104) )
    {
      *(_DWORD *)(*(_DWORD *)(tr_showImagesShader + 340) + 4) = v1;
      RB_BeginSurface((void *)tr_showImagesShader, 3);   /* 0x004D8448: mov ecx,tr_showImagesShader / push edi, edi=3 from 0x004D840C */
      v2 = (double)(int)dwStyle * 0.03125;
      v3 = (double)(int)dwExStyle * 0.041666668;
      *(float *)&v7 = (double)(i % 32) * v2;
      v4 = (double)(i / 32) * v3;
      if ( r_showImages->integer == 2 )
      {
        v5 = dword_16C9840[i];
        v2 = v2 * (double)*(unsigned __int16 *)(v5 + 68) * 0.0009765625;
        v3 = v3 * (double)*(unsigned __int16 *)(v5 + 70) * 0.0009765625;
      }
      flt_17BFF64[0] = v4;
      tess_texCoords0[0] = 0;
      dword_17DFF64[0] = 0;
      dword_17BFF60[0] = v7;
      flt_17BFF6C = *(float *)&v7 + v2;
      dword_17BFF68[0] = 0;
      dword_17DFF68[0] = 1065353216;
      flt_17BFF70 = v4;
      dword_17DFF6C[0] = 0;
      dword_17BFF74 = 0;
      flt_17BFF78 = flt_17BFF6C;
      dword_17DFF70[0] = 1065353216;
      dword_17DFF74[0] = 1065353216;
      dword_17BFF80 = 0;
      dword_17DFF78[0] = 0;
      dword_17DFF7C[0] = 1065353216;
      dword_17BFF84 = v7;
      dword_17BFF8C = 0;
      flt_17BFF7C = v4 + v3;
      tess_indexes[0] = 0;
      flt_17BFF88 = flt_17BFF7C;
      word_17A7F62[0] = 1;
      word_17A7F64[0] = 3;
      word_17A7F66[0] = 3;
      word_17A7F68[0] = 1;
      word_17A7F6A[0] = 2;
      tess_numIndexes = 6;
      tess_numVertexes = 4;
      RB_EndSurface();
    }
  }
  qglFinish();
  v6 = ri_Milliseconds();
  ri_Printf(0, "%i msec to draw all images\n", v6 - v8);
}

/* ---- RB_SaveScreen  0x004D8620 ----  [HIGH] */
int __cdecl RB_SaveScreen(int a3)
{
  void *a1;
  int result;

  if ( !backEnd_projection2D )
    RB_SetGL2D();
  if ( tess_numVertexes )
    RB_EndSurface();
  RB_EndMultitexture();
  GL_Bind((GLenum *)tr_screenImage);
  glCopyTexSubImage2D(0xDE1u, 0, 0, 0, 0, 0, dwStyle, dwExStyle);
  tr_screenImageSaveTime = backEnd_refdef_time;
  result = a3 + 4;
  *(float *)&tr_screenImageSMax = (double)(int)dwStyle / (double)tr_screenImageWidth;
  *(float *)&tr_screenImageTMax = (double)(int)dwExStyle / (double)tr_screenImageHeight;
  return result;
}

/* ---- RB_BlendSavedScreen  0x004D86B0 ----  VERIFIED */
int __cdecl RB_BlendSavedScreen(int a2)
{
  int v2;
  long double v3;
  float v5;
  float v6;
  int v7; // [esp+24h] [ebp-14h] BYREF
  float v8;
  int v9;
  double v10;

  if ( !backEnd_projection2D )
    RB_SetGL2D();
  v2 = *(_DWORD *)(a2 + 4);
  v7 = backEnd_refdef_time - tr_screenImageSaveTime;
  v8 = *(float *)&v2;
  if ( backEnd_refdef_time - tr_screenImageSaveTime < v2 )
  {
    v3 = pow(0.009999999776482582, (double)v7 / (double)SLODWORD(v8));
    if ( v3 > 0.99000001 )
      v3 = 0.99000001;
    LOWORD(v7) = -1;
    v8 = v3 * 255.0;
    BYTE2(v7) = -1;
    v10 = 9.313225746154785e-10;
    v9 = (int)(v8 + 9.313225746154785e-10);
    BYTE3(v7) = v9;
    v6 = (float)(int)dwExStyle;
    v5 = (float)(int)dwStyle;
    RB_DrawStretchPic(tr_screenImageShader, &v7, 0.0, 0.0, v5, v6, 0, tr_screenImageTMax, tr_screenImageSMax, 0, &v7);
  }
  return a2 + 8;
}

static const float s_overdrawColors[14][3] =
{
	{ 0.00f, 0.00f, 0.00f },
	{ 0.00f, 1.00f, 0.00f },
	{ 0.00f, 1.00f, 1.00f },
	{ 0.00f, 0.00f, 1.00f },
	{ 1.00f, 1.00f, 0.00f },
	{ 1.00f, 0.70f, 0.00f },
	{ 1.00f, 0.00f, 0.00f },
	{ 1.00f, 0.30f, 0.30f },
	{ 1.00f, 0.50f, 0.50f },
	{ 1.00f, 0.00f, 0.70f },
	{ 1.00f, 0.00f, 1.00f },
	{ 0.52f, 0.41f, 0.30f },
	{ 1.00f, 1.00f, 1.00f },
	{ 0.50f, 0.50f, 0.50f }
};
static const GLint s_overdrawStencilRefs[14] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 20, 40, 80, 200 };

/* ---- RB_SwapBuffers  0x004D8790 ----  VERIFIED */
int __cdecl RB_SwapBuffers(void *a1, const char *a2, int a4)
{
  int integer;
  int v5;
  unsigned __int8 *v6;
  signed int v7;
  GLint *v8;
  float *v9;
  double v10;
  unsigned __int64 v11; // rax
  double v12;
  int v13;
  int v15;
  int v16;
  int v17;
  int v18;
  int v19;

  if ( tess_numIndexes )
    RB_EndSurface();
  if ( cls_rendererStarted )
  {
    if ( dword_15CA640 )
      dword_1432920(dword_15CA640, cls_debugStringCount);
    if ( dword_15CA650 )
      dword_1432924(dword_15CA650, cls_debugLineCount);
  }
  RB_DrawDebug(a2);
  CL_FlushDebugData( 0 );
  if ( r_showImages->integer )
    RB_ShowImages();
  integer = r_measureOverdraw->integer;
  if ( integer )
  {
    v5 = 0;
    v19 = 0;
    if ( integer >= 2 )
    {
      v6 = (unsigned __int8 *)ri_Hunk_AllocateTempMemory(dwStyle * dwExStyle);
      glReadPixels(0, 0, dwStyle, dwExStyle, 0x1901u, 0x1401u, v6);
      v7 = 0;
      if ( (int)(dwStyle * dwExStyle) > 0 )
      {
        do
          v5 += v6[v7++];
        while ( v7 < (int)(dwStyle * dwExStyle) );
        v19 = v5;
      }
      backEnd_pc_overdrawSum = (double)v19 + backEnd_pc_overdrawSum;
      ri_Hunk_FreeTempMemory(v6);
    }
    if ( r_measureOverdraw->integer != 2 )
    {
      RB_SetGL2D();
      RB_BeginImmediateMode();
      GL_State(65554);
      GL_Bind((GLenum *)tr_whiteImage);
      if ( glState_faceCulling != 2 )
      {
        glDisable(0xB44u);
        glState_faceCulling = 2;
      }
      qglLoadIdentity();
      glStencilOp(0x1E00u, 0x1E00u, 0x1E00u);
      v8 = (GLint *)&s_overdrawStencilRefs[13];
      v9 = (float *)&s_overdrawColors[13][2];
      do
      {
        glStencilFunc(0x206u, *v8, 0xFFFFFFFF);
        v10 = *(v9 - 1) * 255.0;
        rbDebug_immediateColorR = (unsigned __int64)(*(v9 - 2) * 255.0);
        v11 = (unsigned __int64)v10;
        v12 = *v9 * 255.0;
        rbDebug_immediateColorG = v11;
        rbDebug_immediateColorB = (unsigned __int64)v12;
        rbDebug_immediateColorA = -1;
        mode = 9;
        RB_glVertex3f(0, 0, 0);
        *(float *)&v15 = (float)(int)dwStyle;
        RB_glVertex3f(v15, 0, 0);
        *(float *)&v17 = (float)(int)dwExStyle;
        *(float *)&v16 = (float)(int)dwStyle;
        RB_glVertex3f(v16, v17, 0);
        *(float *)&v18 = (float)(int)dwExStyle;
        RB_glVertex3f(0, v18, 0);
        glDrawArrays(mode, 0, rbDebug_immediateVertexCount);
        v9 -= 3;
        --v8;
        rbDebug_immediateVertexCount = 0;
        mode = 0;
      }
      while ( v9 >= &s_overdrawColors[0][2] );
      rbDebug_immediateModeActive = 0;
      if ( !backEnd_projection2D && (glState_glStateBits & 0x200000) == 0 )
      {
        if ( r_fog->integer )
        {
          v13 = dword_16C4BB0;
          if ( (backEnd_refdef_rdflags & 8) == 0 )
            v13 = glfogNum;
          if ( v13 )
          {
            glEnable(0xB60u);
            glState_glStateBits |= 0x200000u;
          }
        }
      }
      glVertexPointer(3, 0x1406u, 0, tess_xyz);
    }
  }
  if ( !glState_finishCalled )
    qglFinish();
  if ( Stream )
    fprintf(Stream, "%s", (int)"***************** RB_SwapBuffers *****************\n\n\n");
  if ( r_swapDelay->integer )
  {
    qglFlush();
    backEnd_projection2D = 0;
    backEnd_endFramePending = 1;
  }
  else
  {
    GLimp_EndFrame();
    backEnd_projection2D = 0;
  }
  return a4 + 4;
}

/* ---- RB_ExecuteRenderCommands  0x004D8AB0 ----  [HIGH] */
int __cdecl RB_ExecuteRenderCommands(_DWORD *a1, const char *a2)
{
  int v3;
  void *v4;
  unsigned int v5;
  _DWORD *v6;
  int result;

  v3 = ri_Milliseconds();
  if ( backEnd_endFramePending )
  {
    backEnd_endFramePending = 0;
    GLimp_EndFrame();
  }
  v4 = (void *)tr_dynamicBufferFrameSerial;
  backEnd_dynamicBuffer_freeBytes = backEnd_dynamicBuffer_capacity;
  backEnd_dynamicBuffer_allocationSequence = 0;
  backEnd_dynamicBuffer_reclaimSequence = 1;
  backEnd_dynamicBuffer_allocations[0] = -1;
  backEnd_dynamicBuffer_currentOffset = 0;
  backEnd_dynamicBuffer_frameSerial = tr_dynamicBufferFrameSerial;
  v5 = *a1 - 1;
  while ( 2 )
  {
    switch ( v5 )
    {
      case 0u:
        backEnd_color2D = a1[1];
        a1 += 2;
        goto LABEL_17;
      case 1u:
        a2 = (const char *)a1[1];
        RB_DrawStretchPic(
          (int)a2,
          (void *)a1[2],
          *((float *)a1 + 2),
          *((float *)a1 + 3),
          *((float *)a1 + 4),
          *((float *)a1 + 5),
          a1[6],
          a1[7],
          a1[8],
          a1[9],
          &backEnd_color2D);
        a1 += 10;
        goto LABEL_17;
      case 2u:
        v6 = (_DWORD *)RB_StretchPicGradient((int)a1, v4);
        goto LABEL_16;
      case 3u:
        v6 = (_DWORD *)RB_StretchPicRotate((int)a1, v4);
        goto LABEL_16;
      case 4u:
        v6 = RB_DrawQuadPic(a1, v4);
        goto LABEL_16;
      case 5u:
        a1 = (_DWORD *)((char *)a1
                      + ((RB_Text_PaintWithCursor(
                            (const char *)a1 + 37,
                            a1[1],
                            *((float *)a1 + 2),
                            a1[3],
                            *((float *)a1 + 4),
                            (int)(a1 + 5),
                            a1[8],
                            *((_BYTE *)a1 + 36),
                            *((float *)a1 + 6),
                            a1[7])
                        + 41)
                       & 0xFFFFFFFC));
        goto LABEL_17;
      case 6u:
        a2 = (const char *)a1;
        v6 = RB_DrawSurfs(v4, a1);
        goto LABEL_16;
      case 7u:
        v6 = (_DWORD *)RB_DrawBuffer((int)a1);
        goto LABEL_16;
      case 8u:
        v6 = (_DWORD *)RB_SaveScreen((int)a1);
        goto LABEL_16;
      case 9u:
        v6 = (_DWORD *)RB_BlendSavedScreen((int)a1);
        goto LABEL_16;
      case 0xAu:
        v6 = (_DWORD *)RB_SwapBuffers(v4, a2, (int)a1);
LABEL_16:
        a1 = v6;
LABEL_17:
        v5 = *a1 - 1;
        if ( v5 > 0xA )
          goto LABEL_18;
        continue;
      default:
LABEL_18:
        backEnd_pc_msec = ri_Milliseconds() - v3;
        result = backEnd_dynamicBuffer_frameSerial;
        if ( tr_dynamicBufferMaxFrameSerial < backEnd_dynamicBuffer_frameSerial )
          tr_dynamicBufferMaxFrameSerial = backEnd_dynamicBuffer_frameSerial;
        return result;
    }
  }
}
