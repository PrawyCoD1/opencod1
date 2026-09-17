/*
 * script/scr_vm_state.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_vm.cpp
 *
 * @fidelity: verified
 */

#include "scr_local.h"

extern void Var_Init__Fv( void );
extern void VM_Init__Fv( void );

extern cvar_t *com_developer_script;

extern int      scrVarPub_developer;
extern int      scrVarPub_developerScript;
extern int      dword_8E60BC;
extern int      scrCompilePub_scriptsPos;
extern int      scrCompilePub_loadedScripts;
extern int      scrAnimPub_treeRoot;

#define SCR_LOADING_FLAGS_MASK      0xFFFF0000

qboolean        scr_inited;

unsigned short  scr_timeArrayId;

int             scr_loading;

/* ---- Scr_Init  0x004758F0 ---- VERIFIED */
void Scr_Init( void ) {
	scrVarPub_developer = com_developer ? com_developer->integer : 0;
	scrVarPub_developerScript = com_developer_script ? com_developer_script->integer : 0;

	Var_Init__Fv();
	VM_Init__Fv();

	/* retail zeroes the two bytes at 0x008E60BE/BF individually */
	dword_8E60BC &= ~SCR_LOADING_FLAGS_MASK;
	scrCompilePub_scriptsPos = 0;
	scrCompilePub_loadedScripts = 0;
	scrAnimPub_treeRoot = 0;

	scr_inited = qtrue;
}

/* ---- Scr_Shutdown  0x00475940 ---- */
void Scr_Shutdown( void ) {
	if ( scr_inited ) {
		scr_inited = qfalse;
	}
}

/* ---- Scr_Abort  0x00475960 ---- */
void Scr_Abort( void ) {
	scr_timeArrayId = 0;
	scr_inited = qfalse;
}

/* ---- Scr_SetLoading  0x00475970 ---- */
void Scr_SetLoading( int bLoading ) {
	scr_loading = bLoading;
}

/* ---- Scr_IsSystemActive  0x0047D040 ---- */
qboolean Scr_IsSystemActive( void ) {
	return scr_timeArrayId != 0 ? qtrue : qfalse;
}
