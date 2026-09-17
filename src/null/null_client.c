/*
 * null/null_client.c -- the client half of the engine, for the dedicated
 * server build only.  Quake III's null_client.c shape: every entry point
 * common and server code call into the client is here as an empty body, so
 * the dedicated exe links without client_mp/, the renderer, the sound engine
 * or the two client VMs.  Nothing in here is retail code; retail's Windows
 * client is one binary and reads `dedicated` at run time, and its Linux
 * dedicated server (cod_lnxded) is the binary this build imitates.
 */

#include "qcommon.h"

vm_t    *cgvm;
vm_t    *uivm;

void CL_Init( void ) {
}

void CL_Shutdown( void ) {
}

void CL_Frame( int msec ) {
	(void)msec;
}

void CL_Disconnect( qboolean showMainMenu ) {
	(void)showMainMenu;
}

void CL_ShutdownAll( void ) {
}

void CL_ShutdownRef( void ) {
}

void CL_ShutdownCGame( void ) {
}

void CL_ShutdownUI( void ) {
}

void CL_FlushMemory( void ) {
}

void CL_StartHunkUsers( void ) {
}

void CL_MapLoading( void ) {
}

void CL_ForwardCommandToServer( const char *string ) {
	(void)string;
}

void CL_ConsolePrint( const char *txt, int channel, int msgtime, int width ) {
	(void)txt; (void)channel; (void)msgtime; (void)width;
}

void CL_ConsoleFixPosition( int a ) {
	(void)a;
}

void CL_PacketEvent( netadr_t from, msg_t *msg ) {
	(void)from; (void)msg;
}

void CL_KeyEvent( int key, int down, unsigned time ) {
	(void)key; (void)down; (void)time;
}

void CL_CharEvent( int key ) {
	(void)key;
}

void CL_MouseEvent( int dx, int dy, int time ) {
	(void)dx; (void)dy; (void)time;
}

void CL_JoystickEvent( int axis, int value, int time ) {
	(void)axis; (void)value; (void)time;
}

void CL_InitKeyCommands( void ) {
}

void Key_WriteBindings( fileHandle_t f ) {
	(void)f;
}

void Key_Bind_f( void ) {
}

void Key_Bindlist_f( void ) {
}

void Key_Unbind_f( void ) {
}

void Key_UnbindAll_f( void ) {
}

qboolean CL_CDKeyValidate( const char *key, const char *checksum ) {
	(void)key; (void)checksum;
	return qtrue;
}

void CL_Netchan_PrintProfileStats( qboolean bPrintToConsole ) {
	(void)bPrintToConsole;
}

void CIN_CloseAllVideos( void ) {
}

/* the server's debug-draw requests are drawn by the client; nobody here */
void CL_AddDebugLine( const float *start, const float *end, const float *color,
					  int depthTest, int duration, int fromServer ) {
	(void)start; (void)end; (void)color;
	(void)depthTest; (void)duration; (void)fromServer;
}

void CL_AddDebugString( const float *origin, const float *color, float scale,
						const char *text, int fromServer ) {
	(void)origin; (void)color; (void)scale; (void)text; (void)fromServer;
}

void CL_FlushDebugData( int fromServer ) {
	(void)fromServer;
}
