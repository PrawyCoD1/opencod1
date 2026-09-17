/*
 * qcommon/vm.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/vm.c
 *
 * Retail range 0x00460210-0x004604.
 *
 * @fidelity-default: verified
 */

#include "qcommon.h"

#define MAX_VM  3

typedef struct vm_s {
	int ( *systemCall )( int *parms );
	char name[64];
	char fqpath[68];
	void            *dllHandle;
	int ( *entryPoint )( int callnum, ... );
} vm_t;

vm_t    *currentVM = NULL;
static vm_t vmTable[MAX_VM];

/* ---- VM_Init  0x00460210 ---- */
void VM_Init( void ) {
	Com_Memset( vmTable, 0, sizeof( vmTable ) );
}

/* ---- VM_DllSyscall  0x00460230 ---- */
int QDECL VM_DllSyscall( int arg, ... ) {
	return currentVM->systemCall( &arg );
}

/* ---- VM_Restart  0x00460250 ---- */
vm_t *VM_Restart( vm_t *vm ) {
	char name[MAX_QPATH];
	int ( *systemCall )( int *parms );

	systemCall = vm->systemCall;
	Q_strncpyz( name, vm->name, sizeof( name ) );

	VM_Free( vm );

	return VM_Create( name, systemCall );
}

/* ---- VM_Create  0x004602E0 ---- */
vm_t *VM_Create( const char *module, int ( *systemCalls )( int * ) ) {
	vm_t    *vm;
	int i;

	if ( !module || !module[0] || !systemCalls ) {
		Com_Error( ERR_FATAL, "\x15" "VM_Create: bad parms" );
	}

	for ( i = 0 ; i < MAX_VM ; i++ ) {
		if ( !Q_stricmp( vmTable[i].name, module ) ) {
			return &vmTable[i];
		}
	}

	for ( i = 0 ; i < MAX_VM ; i++ ) {
		if ( !vmTable[i].name[0] ) {
			break;
		}
	}

	if ( i == MAX_VM ) {
		Com_Error( ERR_FATAL, "\x15" "VM_Create: no free vm_t" );
	}

	vm = &vmTable[i];

	Q_strncpyz( vm->name, module, sizeof( vm->name ) );
	vm->systemCall = systemCalls;

	vm->dllHandle = Sys_LoadDll( module, vm->fqpath, &vm->entryPoint, VM_DllSyscall );
	if ( !vm->dllHandle ) {
		Com_Error( ERR_FATAL, "%s\n", SEH_GetLanguageString( "WIN_UNABLE_LOAD_DLL_BODY" ) );
	}

	return vm;
}

/* ---- VM_Free  0x004603E0 ---- */
void VM_Free( vm_t *vm ) {
	if ( vm->dllHandle ) {
		if ( !FreeLibrary( vm->dllHandle ) ) {
			Com_Error( ERR_FATAL, "\x15" "Sys_UnloadDll FreeLibrary failed" );
		}
	}
	Com_Memset( vm, 0, sizeof( *vm ) );
	currentVM = NULL;
}

/* ---- VM_Clear  0x00460420 ---- */
void VM_Clear( void ) {
	int i;

	for ( i = 0 ; i < MAX_VM ; i++ ) {
		if ( vmTable[i].dllHandle ) {
			if ( !FreeLibrary( vmTable[i].dllHandle ) ) {
				Com_Error( ERR_FATAL, "\x15" "Sys_UnloadDll FreeLibrary failed" );
			}
		}
		Com_Memset( &vmTable[i], 0, sizeof( vm_t ) );
	}
	currentVM = NULL;
}

/* ---- VM_Call  0x00460480 ---- */
int QDECL VM_Call( vm_t *vm, int callnum, ... ) {
	vm_t    *oldVM;
	int r;
	int a[12];
	int i;
	va_list ap;

	va_start( ap, callnum );
	for ( i = 0 ; i < 12 ; i++ ) {
		a[i] = va_arg( ap, int );
	}
	va_end( ap );

	oldVM = currentVM;
	currentVM = vm;

	r = vm->entryPoint( callnum, a[0], a[1], a[2], a[3], a[4], a[5],
						a[6], a[7], a[8], a[9], a[10], a[11] );

	currentVM = oldVM;
	return r;
}
