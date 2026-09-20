/*
 * cg_animtree_mp.c -- the client's animation-tree loader and the per-entity
 * DObj cache.
 * (original source: cgame/cg_animtree.c)
 *
 * cgame_mp_x86.dll 0x300133C0 .. 0x30013536, eight functions, plus the one
 * at 0x30005180 that retail keeps in bg_animation.c (see below).  No RTCW
 * ancestor: RTCW has no script animation trees and no DObj system, so this
 * whole unit is CoD's.
 *
 * CGScr_LoadAnimTrees is the client twin of the game DLL's GScr_LoadScripts
 * (game_mp/g_scr_main_mp.c) with the script half removed -- the client loads
 * no .gsc, only the animation tree the shared bg_animation.c resolves against.
 * It ends in a tail call (0x3001342C `add esp, 9014h; jmp 30005180h`) to a
 * separate, unnamed function that seeds the anim trees, with its own prologue
 * and epilogue (push esi/edi .. pop edi/esi; ret at 0x300051D5), placed in
 * bg_animation.c's CGAMEDLL tail between BG_FindAnimTrees (0x30005120) and
 * bg_misc.c's BG_FindItemForWeapon (0x300051E0).  The game DLL has the
 * bg_clientinfo half of it at 0x20004F20, tail-called the same way from
 * GScr_LoadScripts (0x2002F16F), as bg_animation.c's BG_CreateClientAnimTrees
 * under #ifndef CGAMEDLL.  This is the CGAMEDLL half of that same function,
 * kept here only because a bg unit cannot reach cgs.
 *
 * The other seven are the DObj-lifetime helpers.  Six of them exist in the
 * binary only because something took their address or the Mac symbols named
 * them: CG_SetDObjInfo, CG_CheckDObjInfoMatches and CG_SafeDObjFree have no
 * call sites at all in 1.1 -- every user got them inlined (see
 * CG_PreProcess_GetDObj in cg_ent_mp.c and CG_TransitionSnapshot in
 * cg_snapshot_mp.c, which carry all three expanded in place).
 *
 * @fidelity: likely
 */

#include <math.h>

#include "cg_local.h"

/* bg_animation.c, compiled into this DLL as well as into the game DLL. */
extern animScriptData_t bgs_animScriptData;             /* 0x300F0520 */
extern void *bgs_animTree;                              /* 0x3018BBF8 */

void        BG_FindAnims( void );
void        BG_FindAnimTrees( void );
void        BG_FinalizePlayerAnims( void );

/*
 * The script-system import table, the same object g_local.h declares for the
 * game DLL at 0x200A0500.  cgame's copy sits 0x100A600 higher
 * (Scr_FindAnimTree / Scr_FindAnim: game 0x200A0638, cgame 0x300AAC38).
 */
/* takes NOTHING: 0x300133F2 is a bare `call dword_300AAB78` with no push, and
   the engine's own Scr_BeginLoadAnimTrees (0x0046D5F0) is `( void )`. */
extern void ( *Scr_BeginLoadAnimTrees )( void );                        /* 0x300AAB78 */
extern void ( *Scr_EndLoadAnimTrees )( void );                          /* 0x300AAB80 */
extern void ( *Scr_PrecacheAnimTrees )( void *( *allocFn )( int size ) ); /* 0x300AAB84 */

/*
===============
Hunk_AllocXAnimCreate

Handed to Scr_PrecacheAnimTrees as the tree allocator.  The game DLL's
namesake calls trap_Hunk_AllocLowInternal; the client takes the 32-byte
aligned high hunk instead (trap 201).
===============
*/
void *Hunk_AllocXAnimCreate( int size ) {
	return (void *)trap_syscall_0xC9( size );
}

/*
===============
BG_CreateClientAnimTrees

0x30005180, unnamed; the game DLL's twin is bg_animation.c's
BG_CreateClientAnimTrees (0x20004F20), compiled out under CGAMEDLL.  Retail
keeps this one in bg_animation.c too (see the file header), but bg_animation.c
cannot include cg_local.h -- cg_public.h redefines g_public.h's trajectory_t
and entityState_t -- so it cannot see cg_corpseinfo, and the function lives
here instead.
trap_XAnimCreateTree is inlined in retail (a bare syscall 0x83 per iteration).
bgs_animTree is read once, before both loops (0x30005182 mov edi), and the
same edi is pushed for all 72 calls.
===============
*/
void BG_CreateClientAnimTrees( void ) {
	void *tree;
	int i;

	tree = bgs_animTree;

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		bg_clientinfo[i].animTree = (void *)trap_XAnimCreateTree( (int)tree );
	}
	/* the corpse records get their own runtime trees: cg_corpseinfo is a
	   frozen clientInfo_t and CG_ResetEntity copies over everything but this */
	for ( i = 0; i < MAX_CORPSES; i++ ) {
		cg_corpseinfo[i].animTree = (void *)trap_XAnimCreateTree( (int)tree );
	}
}

/*
===============
CGScr_LoadAnimTrees
===============
*/
void CGScr_LoadAnimTrees( void ) {
	/* ZEROED, and it has to be: BG_AnimParseAnimScript hands this to
	   BG_AnimationIndexForString, which READS the count before anything writes
	   it, so a garbage start appends every animation past the end of the array
	   and the tree built from it is malformed.  Retail zeroes it at function
	   entry before the first call -- 0x300133E0 is `mov eax, 9004h;
	   __alloca_probe; mov [esp+9004h+numAnims], 0`, the 4 bytes past the
	   36864-byte (512 x 72) buffer -- and the game module does the same at
	   0x2002F0DA. */
	int numAnims = 0;
	byte animScriptBuffer[36864];

	Scr_BeginLoadAnimTrees();

	BG_FindAnims();
	BG_AnimParseAnimScript( &bgs_animScriptData, animScriptBuffer, &numAnims );
	Scr_PrecacheAnimTrees( Hunk_AllocXAnimCreate );
	BG_FindAnimTrees();
	Scr_EndLoadAnimTrees();
	BG_FinalizePlayerAnims();

	BG_CreateClientAnimTrees();
}

/*
===============
CG_FreeClientDObjInfo

The 64 client slots.  CG_Shutdown drops these before the entity slots.
===============
*/
void CG_FreeClientDObjInfo( void ) {
	int i;

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		trap_SafeDObjFree( i, 1 );
		cg.iEntityLastType[i] = 0;
		cg.pEntityLastXModel[i] = NULL;
	}
}

/*
===============
CG_SetDObjInfo

No call site in 1.1 -- inlined everywhere (cg_ent_mp.c 0x3001D3FC).
===============
*/
void CG_SetDObjInfo( int entityNum, int eType, void *xmodel ) {
	cg.iEntityLastType[entityNum] = eType;
	cg.pEntityLastXModel[entityNum] = xmodel;
}

/*
===============
CG_CheckDObjInfoMatches

No call site in 1.1 -- inlined (cg_ent_mp.c 0x3001D370).
===============
*/
qboolean CG_CheckDObjInfoMatches( int entityNum, int eType, void *xmodel ) {
	return cg.iEntityLastType[entityNum] == eType && cg.pEntityLastXModel[entityNum] == xmodel;
}

/*
===============
CG_SafeDObjFree

No call site in 1.1 -- inlined (cg_ent_mp.c 0x3001D386, cg_snapshot_mp.c
0x3002FB2B).
===============
*/
void CG_SafeDObjFree( int entityNum ) {
	trap_SafeDObjFree( entityNum, 1 );
	cg.iEntityLastType[entityNum] = 0;
	cg.pEntityLastXModel[entityNum] = NULL;
}

/*
===============
CG_FreeEntityDObjInfo

The non-client entity slots, 64 .. ENTITYNUM_NONE-1.
===============
*/
void CG_FreeEntityDObjInfo( void ) {
	int i;

	for ( i = MAX_CLIENTS; i < ENTITYNUM_NONE; i++ ) {
		trap_SafeDObjFree( i, 1 );
		cg.iEntityLastType[i] = 0;
		cg.pEntityLastXModel[i] = NULL;
	}
}

/*
===============
sub_30013520

Unnamed and uncalled: one FSINCOS, both results stored through pointers.
Every user got it inlined.
===============
*/
static void sub_30013520( float rad, float *sinOut, float *cosOut ) {
	*cosOut = (float)cos( rad );
	*sinOut = (float)sin( rad );
}
