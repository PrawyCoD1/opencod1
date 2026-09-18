#ifndef CL_VM_H
#define CL_VM_H

void LAN_CopyDisplayHostname( char *dest, int size, const char *source );

/* connstate_t, coduo client/cgame.h:630-640.  CL_KeyEvent and Console_Key
 * compare cls_state against CA_CINEMATIC (7) and CA_LOGO (8); coduo
 * cinematic.c:250-251 is the same three-way state test. */
#define CA_DISCONNECTED     0
#define CA_CONNECTING       1
#define CA_CONNECTED        3
#define CA_ACTIVE           6
#define CA_CINEMATIC        7
#define CA_LOGO             8

#define COMMAND_HISTORY     32      /* RTCW client/keys.h:49 */

#define KEYCATCH_CONSOLE    0x0001
#define KEYCATCH_UI         0x0002
#define KEYCATCH_MESSAGE    0x0004
#define KEYCATCH_CGAME      0x0008

#define CON_CHANNEL_DROP    4

#define MAX_KEYS            256
#define K_CHAR_FLAG         0x400

#define CG_SHUTDOWN         1
#define CG_KEY_EVENT        5   /* 0x0040DF6D, 0x0040E04B */
#define CG_CHECK_EXEC_KEY   6   /* 0x0040DEC2 */
#define CG_MOUSE_EVENT      7
#define CG_EVENT_HANDLING   8
#define UI_SHUTDOWN         2
#define UI_KEY_EVENT        3
#define UI_MOUSE_EVENT      4
#define UI_GETAPIVERSION    0
#define UI_INIT             1
#define UI_REFRESH          5
#define UI_IS_FULLSCREEN    6
#define UI_SET_ACTIVE_MENU  7
#define UI_API_VERSION      7

/* uiMenuCommand_t, from the two indices CL_Frame pushes at 0x004112B3 and 0x004112E0 and from CL_Disconnect's teardown call. */
#define UIMENU_NONE         0
#define UIMENU_MAIN         1
#define UIMENU_INGAME       2
#define UIMENU_NEED_CD      3

/* CIN_RunCinematic's handle bound, `cmp eax, 10h` at 0x00411432. */
#ifndef MAX_VIDEO_HANDLES
#define MAX_VIDEO_HANDLES   16
#endif

#endif
