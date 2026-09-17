/*
 * Reconstructed from Call of Duty 1.1 (Windows, CoDMP.exe).
 * Original translation unit:
 * /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/EffectsCore/FxMemMgr.cpp
 * Retail range 0x0048D290-0x0048D67F (FxMem_ClaimBlock and friends).
 * @fidelity: likely
 */

#include "../qcommon/cod1_types.h"
#include <stdlib.h>

/* The arena.  0x00A9CE58, 64 blocks of 32768.                         */

unsigned char   unk_A9CE58[0x200000];

int             dword_140C9B8[2] = {  82, 0 };
int             dword_140C9C0[2] = {  45, 0 };
int             dword_140C9C8[2] = { 209, 0 };
int             dword_140C9D0[2] = { 103, 0 };
int             dword_140C9D8[2] = {  95, 0 };
int             dword_140C9E0[2] = { 481, 0 };
int             dword_140C9E8[2] = { 125, 0 };
int             dword_140C9F0[2] = {  75, 0 };
int             dword_140C9F8[2] = {  99, 0 };
int             dword_140CA00[2] = {  95, 0 };
int             dword_140CA08[2] = { 148, 0 };
int             dword_140CA10[2] = {  58, 0 };
int             dword_140CA18[2] = {  99, 0 };
int             dword_140758C[2] = {  54, 0 };
int             dword_1407594[2] = { 481, 0 };

extern int  __stdcall CFxScheduler__ctor_real( int a1 );   /* 0x00492B20 */
extern void __cdecl   sub_492980( int a1 );                /* 0x00492980 */
extern int  __cdecl   sub_48D230( void );                  /* 0x0048D230 */

extern unsigned int dword_14075A0[];                       /* 0x014075A0 */

/* ---- FxScheduler_static_fini  0x0053C230 ---- */
static void __cdecl FxScheduler_static_fini( void )
{
	sub_492980( (int)dword_14075A0 );
}

/* ---- FxScheduler_static_init  0x0053C1E0 ---- */
static void __cdecl FxScheduler_static_init( void )
{
	CFxScheduler__ctor_real( (int)dword_14075A0 );
	atexit( FxScheduler_static_fini );
}

/* ---- FxMemMgr_static_init  0x0053BEE0 ---- */
static void __cdecl FxMemMgr_static_init( void )
{
	sub_48D230();
}

/* ---- FX_StaticInit  no-address ---- */
void FX_StaticInit( void )
{
	FxScheduler_static_init();     /* __xc_a + 0x18, 0x0053C1E0 */
	FxMemMgr_static_init();        /* __xc_a + 0x50, 0x0053BEE0 */
}
