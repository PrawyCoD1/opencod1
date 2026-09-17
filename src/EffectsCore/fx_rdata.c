/*
 * fx_rdata.c -- EffectsCore globals whose real storage differs from the
 * `extern int` declarations in cod1_globals.h.
 *
 * @fidelity: verified   (storage, sizes and the ten vtables -- no code here)
 *
 * This unit deliberately does NOT include ../qcommon/cod1_globals.h.  The
 * definitions below and that header's declarations of the same names have
 * different types, and they must never meet inside one translation unit; the
 * linker matches them by NAME alone.  Same arrangement, and same reason, as
 * renderer/tr_tess_storage.c.
 *
 * ======================================================================
 * theFxHelper (SFxHelper) @ 0x01407508 -- 132 bytes
 * ======================================================================
 *
 * SFxHelper::AdjustCamera (0x00499D60) takes its destination as a stack
 * argument and writes displacements +0x10 through +0x80 -- twenty-six of them,
 * a camera basis and a view record.  Both call sites (client_mp/cl_cgame_mp.c
 * and universal/memorytree.cpp) pass `&theFxHelper`.
 *
 * SIZE: AdjustCamera's highest displacement is +0x80, read/written as a dword,
 * and SFxHelper::CullSphere (0x0049A0D0) names the absolute address
 * 0x01407588 == base + 0x80.  The next object begins at 0x0140758C: the
 * CPrimitiveTemplate FxMemMgr pool, constructed by its own static-init thunk
 * at 0x0053C180.  The two pool handles at 0x0140758C and 0x01407594 sit
 * between this object and theFxScheduler at 0x014075A0.  Extent
 * 0x01407508..0x0140758B == 0x84 == 132.
 *
 * Tentative definition, not an initialised one: this is .bss in retail --
 * zero at load, filled by AdjustCamera and friends before use -- so what is
 * needed is STORAGE, not contents.  A tentative definition is a COMMON symbol;
 * cod1_globals.c's `int theFxHelper;` is also COMMON; the linker keeps the
 * LARGER.  No LNK2005, no warning, no call-site edits.
 *
 * The eleven further shard names that cod1_globals.h carves out of this same
 * object (0x0140750C..0x01407530 and 0x01407588) are aliased onto it by
 * EffectsCore/fx_types.h, which only EffectsCore/fx_unassigned.cpp includes.
 */

unsigned char theFxHelper[132];

/*
 * ======================================================================
 * fxActiveEffects (SEffectList[1800]) @ 0x00C9CE60 -- 21600 bytes
 * ======================================================================
 *
 * The effect table.  Retail's three walks over it are bounded by the address
 * one past its end, `byte_CA22C0`, where a GenericParser2 token buffer starts.
 * fx_types.h carries the byte_CA22C0 redirect and the three interior shards.
 *
 * SIZE: 0x00CA22C0 - 0x00C9CE60 == 0x5460 == 21600 == 1800 * 12, and 1800 is
 * the literal slot count sub_49ED60 and FX_DrawStats_m count down from.
 *
 * Tentative definition for the same reason as theFxHelper: .bss in retail,
 * storage is what is needed, and largest-COMMON-wins gives it to every
 * consumer without a call-site edit.
 */
unsigned char fxActiveEffects[21600];

/*
 * ======================================================================
 * The twelve primitive vtables -- 336 bytes of function pointers
 * ======================================================================
 *
 * Ten at 0x00559EA0..0x00559FB8, two more at 0x00559450 and 0x0055946C.
 * Stride 0x1C == 28 == SEVEN function pointers; every slot is an address
 * inside the CEffect method range 0x0048DAD0..0x00491440.  Each ctor stores
 * its own table (`*result = &off_559F10;` and its siblings).
 *
 * SLOT ORDER: [0] scalar deleting dtor, [1] Die, [2] Update, [3] Cull,
 * [4] Draw, [5] TypeID, [6] Archive.  Inheritance shows up as shared slots --
 * five tables take CParticle::Die, CLight takes CEffect::Die -- and those are
 * reproduced exactly, not normalised.
 *
 * The seventy methods are declared below rather than through a header: this
 * unit must not see cod1_globals.h (see the note at the top of this file), and
 * the pre-prototype form names the symbol for the linker without asserting a
 * signature that would then have to be kept in step with fx_unassigned.cpp's.
 * The linker matches by name, and every one of these is __cdecl.
 *
 * These are INITIALISED, unlike the two objects above, so they are strong
 * definitions rather than COMMON -- cod1_globals.c's four-byte COMMON binds
 * to them without LNK2005.
 */
extern void CEffect__scalar_dtor();  extern void nullsub_32();            /* CEffect::Die  */
extern void CEffect__Update();       extern void CEffect__Cull();
extern void nullsub_33();            /* CEffect::Draw */
extern void CEffect__TypeID();       extern void CEffect__Archive();

extern void CParticle__scalar_dtor();          extern void CParticle__Die();
extern void CParticle__Update();               extern void CParticle__Cull();
extern void CParticle__Draw();                 extern void CParticle__TypeID();
extern void CParticle__Archive();

extern void COrientedParticle__scalar_dtor();  extern void COrientedParticle__Update();
extern void COrientedParticle__Cull();         extern void COrientedParticle__Draw();
extern void COrientedParticle__TypeID();       extern void COrientedParticle__Archive();

extern void CLine__scalar_dtor();    extern void nullsub_34();            /* CLine::Die */
extern void CLine__Update();         extern void CLine__Cull();
extern void CLine__Draw();           extern void CLine__TypeID();
extern void CLine__Archive();

extern void CElectricity__scalar_dtor();  extern void nullsub_35();       /* CElectricity::Die */
extern void CElectricity__Update();       extern void CElectricity__Cull();
extern void CElectricity__Draw();         extern void CElectricity__TypeID();
extern void CElectricity__Archive();

extern void CTail__scalar_dtor();    extern void CTail__Update();
extern void CTail__Cull();           extern void CTail__Draw();
extern void CTail__TypeID();         extern void CTail__Archive();

extern void CCylinder__scalar_dtor();     extern void CCylinder__Update();
extern void CCylinder__Cull();            extern void CCylinder__Draw();
extern void CCylinder__TypeID();          extern void CCylinder__Archive();

extern void CEmitter__scalar_dtor();      extern void CEmitter__Update();
extern void CEmitter__Cull();             extern void CEmitter__Draw();
extern void CEmitter__TypeID();           extern void CEmitter__Archive();

extern void CLight__scalar_dtor();   extern void CLight__Update();
extern void CLight__Cull();          extern void CLight__Draw();
extern void CLight__TypeID();        extern void CLight__Archive();

extern void CFlash__scalar_dtor();   extern void CFlash__Update();
extern void CFlash__Cull();          extern void CFlash__Draw();
extern void CFlash__TypeID();        extern void CFlash__Archive();

extern void cand_CFxPrimType14__scalar_dtor();  extern void cand_CFxPrimType14__Update();
extern void cand_CFxPrimType14__Cull();         extern void cand_CFxPrimType14__Draw();
extern void cand_CFxPrimType14__TypeID();       extern void cand_CFxPrimType14__Archive();

extern void cand_CFxPrimType13__scalar_dtor();  extern void cand_CFxPrimType13__Update();
extern void cand_CFxPrimType13__Cull();         extern void cand_CFxPrimType13__Draw();
extern void cand_CFxPrimType13__TypeID();       extern void cand_CFxPrimType13__Archive();

/*
 * The two CoD1-only primitives sit apart from the other ten, at 0x00559450 and
 * 0x0055946C.  Both are 28 bytes; both take CEffect::Die; both are reached
 * from FX_Add and, for type 13, from FX_AddFlash (0x004A0FA0) -- which
 * installs off_55946C, NOT CFlash's off_559EA0.  Retail quirk, reproduced.
 */

/* 0x00559450 -- CoD1-only primitive, TypeID 14. */
void (*off_559450[7])() = {
  cand_CFxPrimType14__scalar_dtor, nullsub_32, cand_CFxPrimType14__Update,
  cand_CFxPrimType14__Cull, cand_CFxPrimType14__Draw, cand_CFxPrimType14__TypeID,
  cand_CFxPrimType14__Archive
};

/* 0x0055946C -- CoD1-only primitive, TypeID 13. */
void (*off_55946C[7])() = {
  cand_CFxPrimType13__scalar_dtor, nullsub_32, cand_CFxPrimType13__Update,
  cand_CFxPrimType13__Cull, cand_CFxPrimType13__Draw, cand_CFxPrimType13__TypeID,
  cand_CFxPrimType13__Archive
};

/* 0x00559EA0 -- CFlash, TypeID 12.  Die inherited from CParticle. */
void (*off_559EA0[7])() = {
  CFlash__scalar_dtor, CParticle__Die, CFlash__Update,
  CFlash__Cull, CFlash__Draw, CFlash__TypeID,
  CFlash__Archive
};

/* 0x00559EBC -- CLine, TypeID 2. */
void (*off_559EBC[7])() = {
  CLine__scalar_dtor, nullsub_34, CLine__Update,
  CLine__Cull, CLine__Draw, CLine__TypeID,
  CLine__Archive
};

/* 0x00559ED8 -- CCylinder, TypeID 4.  Die inherited from CParticle. */
void (*off_559ED8[7])() = {
  CCylinder__scalar_dtor, CParticle__Die, CCylinder__Update,
  CCylinder__Cull, CCylinder__Draw, CCylinder__TypeID,
  CCylinder__Archive
};

/* 0x00559EF4 -- COrientedParticle, TypeID 8.  Derives from CParticle. */
void (*off_559EF4[7])() = {
  COrientedParticle__scalar_dtor, CParticle__Die,
  COrientedParticle__Update, COrientedParticle__Cull,
  COrientedParticle__Draw, COrientedParticle__TypeID,
  COrientedParticle__Archive
};

/* 0x00559F10 -- CParticle, TypeID 1. */
void (*off_559F10[7])() = {
  CParticle__scalar_dtor, CParticle__Die, CParticle__Update,
  CParticle__Cull, CParticle__Draw, CParticle__TypeID,
  CParticle__Archive
};

/* 0x00559F2C -- CElectricity, TypeID 9. */
void (*off_559F2C[7])() = {
  CElectricity__scalar_dtor, nullsub_35, CElectricity__Update,
  CElectricity__Cull, CElectricity__Draw, CElectricity__TypeID,
  CElectricity__Archive
};

/* 0x00559F48 -- CEmitter, TypeID 5.  Die inherited from CParticle. */
void (*off_559F48[7])() = {
  CEmitter__scalar_dtor, CParticle__Die, CEmitter__Update,
  CEmitter__Cull, CEmitter__Draw, CEmitter__TypeID,
  CEmitter__Archive
};

/* 0x00559F64 -- CLight, TypeID 11.  Die inherited from CEffect, not CParticle. */
void (*off_559F64[7])() = {
  CLight__scalar_dtor, nullsub_32, CLight__Update,
  CLight__Cull, CLight__Draw, CLight__TypeID,
  CLight__Archive
};

/* 0x00559F80 -- CEffect, the base.  TypeID 0; Die and Draw are empty. */
void (*off_559F80[7])() = {
  CEffect__scalar_dtor, nullsub_32, CEffect__Update,
  CEffect__Cull, nullsub_33, CEffect__TypeID,
  CEffect__Archive
};

/* 0x00559F9C -- CTail, TypeID 3.  Derives from CParticle. */
void (*off_559F9C[7])() = {
  CTail__scalar_dtor, CParticle__Die, CTail__Update,
  CTail__Cull, CTail__Draw, CTail__TypeID,
  CTail__Archive
};
