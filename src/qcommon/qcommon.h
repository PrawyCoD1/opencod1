/*
 * qcommon.h -- shared engine declarations for Call of Duty 1.1.
 *
 * Layout follows RTCW's qcommon.h, which is what CoD 1.1 descends from.  Where
 * a signature was recovered from the binary it is used as recovered, even where
 * that differs from RTCW.
 *
 * @fidelity-default: likely
 */

#ifndef __QCOMMON_H__
#define __QCOMMON_H__

#include "../universal/q_shared.h"
#include "cod1_types.h"

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#ifndef QDECL
#define QDECL
#endif

#ifndef NULL
#define NULL ( (void *)0 )
#endif

/*
=============================================================================
                        LIMITS
=============================================================================
*/

#define MAX_QPATH               64
#define MAX_OSPATH              256
#define MAX_NAME_LENGTH         32
#define MAX_NETNAME             36

#define MAX_STRING_CHARS        1024
#define MAX_STRING_TOKENS       256
#define MAX_TOKEN_CHARS         1024

#define MAX_INFO_STRING         1024
#define MAX_INFO_KEY            1024
#define MAX_INFO_VALUE          1024
#define BIG_INFO_STRING         8192
#define BIG_INFO_KEY            8192
#define BIG_INFO_VALUE          8192

#define MAX_CMD_LINE            1024
#define MAX_CMD_BUFFER          16384

#define MAX_CLIENTS             64
/* Spelled the way RTCW's q_shared.h derives them (1006-1015); the values are
 * CoD 1.1's own -- 1024/1023/1022. */
#define GENTITYNUM_BITS         10
#define MAX_GENTITIES           ( 1 << GENTITYNUM_BITS )
#define ENTITYNUM_NONE          ( MAX_GENTITIES - 1 )
#define ENTITYNUM_WORLD         ( MAX_GENTITIES - 2 )
#define MAX_RELIABLE_COMMANDS   64
#define MAX_DOWNLOAD_WINDOW     8
#define MAX_CHALLENGES          1024
#define MAX_BPS_WINDOW          20
#define PACKET_BACKUP           32
#define PACKET_MASK             ( PACKET_BACKUP - 1 )

#define MAX_JOYSTICK_AXIS       6
#define MAX_FOUND_FILES         0x1000
#define FILE_HASH_SIZE          256

/* M_PI moved to universal/q_shared.h with the vector primitives. */

#define PORT_ANY                -1
#define PORT_SERVER             0x7120
#define PORT_MASTER             20510
#define PORT_AUTHORIZE          20500

#define S_COLOR_WHITE           "^7"
#define Q_IsColorString( p )    ( p && *( p ) == '^' && *( ( p ) + 1 ) && *( ( p ) + 1 ) != '^' )

/* VM entry points.  Numeric values are observed at the retail call sites; the
 * names follow Q3/RTCW convention and are not themselves recovered. */
#define CG_CONSOLE_COMMAND      2
#define GAME_CONSOLE_COMMAND    11
#define UI_CONSOLE_COMMAND      11
#define UI_SET_ACTIVE_MENU      7

typedef enum { EXEC_NOW, EXEC_INSERT, EXEC_APPEND } cbufExec_t;

typedef struct cmd_s {
	byte    *data;
	int maxsize;
	int cursize;
} cmd_t;

typedef struct vm_s vm_t;

/*
=============================================================================

						COMMANDS

=============================================================================
*/

typedef void ( *xcommand_t )( void );

typedef struct cmd_function_s
{
	struct cmd_function_s   *next;
	char                    *name;
	xcommand_t function;
} cmd_function_t;
COD1_ASSERT_SIZE( cmd_function_t, 12 );
/* RTCW keeps cmd_function_t file-static in cmd.c.  CoD 1.1 cannot:
 * qcommon/cod1_globals.h declares `cmd_function_t *cmd_functions` at
 * 0x00890BE8, so the typedef has to be visible to every unit that includes
 * that header.  Fold it back into cmd.c when that declaration goes. */

/*
=============================================================================

						NETWORKING

=============================================================================
*/
enum
{
	MAX_PACKETLEN = 0x578,
	MAX_MSGLEN    = 0x4000,
	FRAGMENT_SIZE = 0x514,
	FRAGMENT_BIT  = 0x80000000,
};
/* PORT_ANY / PORT_SERVER are the #defines in the LIMITS block above, as RTCW
 * spells them.  MAX_LOOPBACK / MAX_LOOPBACK_MASK and the loopback_t pair live
 * in qcommon/net_chan_mp.c, where RTCW keeps them. */

typedef enum netadrtype_t
{
	NA_BOT           = 0x0,
	NA_BAD           = 0x1,
	NA_LOOPBACK      = 0x2,
	NA_BROADCAST     = 0x3,
	NA_IP            = 0x4,
	NA_IPX           = 0x5,
	NA_BROADCAST_IPX = 0x6,
} netadrtype_t;

typedef enum netsrc_t
{
	NS_CLIENT = 0x0,
	NS_SERVER = 0x1,
} netsrc_t;

/*
 * port is at +18, after ipx, as in RTCW: SV_GetChallenge stores
 * BigShort(PORT_AUTHORIZE) with `mov word ptr [ipx+8], ax`.
 */
typedef struct netadr_t
{
	netadrtype_t type;
	byte ip[4];
	byte ipx[10];
	unsigned short port;
} netadr_t;
COD1_ASSERT_SIZE( netadr_t, 20 );
int NET_CompareAdrSigned( const netadr_t *a, const netadr_t *b );

/* CoD: RTCW's msg_t leads with allowoverflow and carries an oob flag; 1.1 has
 * neither.  Out-of-band is tracked by the caller instead. */
typedef struct msg_t
{
	qboolean overflowed;
	byte            *data;
	int maxsize;
	int cursize;
	int readcount;
	int bit;
} msg_t;
COD1_ASSERT_SIZE( msg_t, 24 );

typedef struct netProfilePacket_t
{
	int iTime;
	int iSize;
	int bFragment;
} netProfilePacket_t;
COD1_ASSERT_SIZE( netProfilePacket_t, 12 );

typedef struct netProfileStream_t
{
	netProfilePacket_t packets[60];
	int iCurrPacket;
	int iBytesPerSecond;
	int iLastBPSCalcTime;
	int iCountedPackets;
	int iCountedFragments;
	int iFragmentPercentage;
	int iLargestPacket;
	int iSmallestPacket;
} netProfileStream_t;
COD1_ASSERT_SIZE( netProfileStream_t, 752 );

typedef struct netProfileInfo_t
{
	netProfileStream_t send;
	netProfileStream_t receive;
} netProfileInfo_t;
COD1_ASSERT_SIZE( netProfileInfo_t, 1504 );

typedef struct netchan_t
{
	netsrc_t sock;
	int dropped;                    // between last packet and previous
	netadr_t remoteAddress;
	int qport;                      // qport value to write when transmitting
	int incomingSequence;
	int outgoingSequence;
	int fragmentSequence;
	int fragmentLength;
	byte fragmentBuffer[MAX_MSGLEN];
	qboolean unsentFragments;
	int unsentFragmentStart;
	int unsentLength;
	byte unsentBuffer[MAX_MSGLEN];
	netProfileInfo_t *pProf;
} netchan_t;
COD1_ASSERT_SIZE( netchan_t, 32832 );
typedef struct netField_t
{
	const char      *name;
	int offset;
	int bits;
} netField_t;
COD1_ASSERT_SIZE( netField_t, 12 );
/* RTCW builds its netField_t tables file-static in msg.c.  1.1 cannot: the
 * client's delta reader (client_mp/cl_parse_mp.c) takes `const netField_t *`
 * arguments and is handed msg_mp.c's tables, so the type is shared. */

/*
=============================================================================

						HUFFMAN

=============================================================================
*/

enum
{
	NYT           = 0x100,      // not yet transmitted
	INTERNAL_NODE = 0x101,
};

typedef struct node_t
{
	struct node_t   *left, *right, *parent;
	struct node_t   *next, *prev;
	struct node_t   **head;
	int weight;
	int symbol;
} node_t;
COD1_ASSERT_SIZE( node_t, 32 );

typedef struct huff_t
{
	int blocNode;
	int blocPtrs;
	node_t          *tree;
	node_t          *lhead;
	node_t          *ltail;
	node_t          *loc[NYT + 1];
	node_t          **freelist;
	node_t nodeList[768];
	node_t          *nodePtrs[768];
} huff_t;
COD1_ASSERT_SIZE( huff_t, 28700 );

typedef struct huffman_t
{
	huff_t compressor;
	huff_t decompressor;
} huffman_t;
COD1_ASSERT_SIZE( huffman_t, 57400 );
/*
=============================================================================

						SHARED / MATH

=============================================================================
*/

typedef enum sysEventType_t
{
	SE_NONE          = 0x0,
	SE_KEY           = 0x1,
	SE_CHAR          = 0x2,
	SE_MOUSE         = 0x3,
	SE_JOYSTICK_AXIS = 0x4,
	SE_CONSOLE       = 0x5,
	SE_PACKET        = 0x6,
} sysEventType_t;

typedef struct sysEvent_t
{
	int evTime;
	sysEventType_t evType;
	int evValue;
	int evValue2;
	int evPtrLength;                // bytes of data pointed to by evPtr
	void            *evPtr;         // this must be manually freed if not NULL
} sysEvent_t;
COD1_ASSERT_SIZE( sysEvent_t, 24 );

/* 284 bytes (0x11C), NOT the 268 a three-int head implies: the console
 * history ring unk_14304C0 is 9088 bytes, which is exactly 32 * 284.
 * Field_CompleteCommand copies sizeof(field_t), so the size matters. */
typedef struct field_t
{
	int cursor;
	int scroll;
	int widthInChars;
	int widthInPixels;              /* +0x0C  620 console, 588/543 chat */
	float charWidth;                /* +0x10  8.0f console, 0.0f proportional */
	float charHeight;               /* +0x14 */
	int fixedWidth;                 /* +0x18  1 console, 0 chat */
	char buffer[256];               /* +0x1C */
} field_t;
COD1_ASSERT_SIZE( field_t, 284 );
typedef struct lump_t
{
	int filelen;
	int fileofs;
} lump_t;
COD1_ASSERT_SIZE( lump_t, 8 );
/* RTCW's home for lump_t is qcommon/qfiles.h, which does not exist here yet.
 * qcommon/cm_load.c and renderer/tr_bsp.c both read it, so it stays at this
 * level until that header exists. */

/*
=============================================================================

						VIRTUAL MACHINE

=============================================================================
*/

typedef struct statmonitor_s
{
	int endtime;
	void            *material;
} statmonitor_t;
COD1_ASSERT_SIZE( statmonitor_t, 8 );

enum
{
	MAX_LOGFILENAMESIZE = 0x400,
};

/*
=============================================================================
                        COMMON
=============================================================================
*/

void QDECL Com_Printf( const char *fmt, ... );
void QDECL Com_DPrintf( const char *fmt, ... );
void QDECL Com_Error( int code, const char *fmt, ... );
void Com_PrintMessage( int channel, const char *msg );
void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char * ) );
void Com_EndRedirect( void );

/* NOT RETAIL -- the hang watchdog, qcommon/hangwatch.c.  Install from WinMain,
 * Tick from the frame loop, Note from Com_PrintMessage. */
void Cod1_InstallHangWatchdog( void );
void Cod1_HangWatchdogTick( void );
void Cod1_HangWatchdogNote( const char *msg );

void Com_Init( char *commandLine );
void Com_Frame( void );
void Com_Shutdown( const char *finalmsg );
void Com_Close( void );
void Com_Restart( void );
void Com_Quit_f( void );

/* NOT RETAIL, diagnostic: set to a short literal by whoever asks the engine to
 * quit; Com_Quit_f prints it and clears it.  NULL means "the quit command". */
extern const char *com_quitReason;
void Com_ErrorCleanup( void );
void Com_SetErrorMessage( const char *errorMessage );
void Com_CleanupSkeletons( void );

void Com_ParseCommandLine( char *commandLine );
qboolean Com_SafeMode( void );
void Com_ForceSafeMode( void );
void Com_StartupVariable( const char *match );
qboolean Com_AddStartupCommands( void );

void Com_InitJournaling( void );
void Com_InitPushEvent( void );
void Com_PushEvent( sysEvent_t *event );
sysEvent_t Com_GetEvent( void );
sysEvent_t Com_GetRealEvent( void );
int Com_EventLoop( void );
int Com_Milliseconds( void );
int Com_ModifyMsec( int msec );

void Com_WriteConfiguration( void );
void Com_WriteConfigToFile( const char *filename );
void Com_WriteDefaultsToFile( const char *filename );
void Com_WriteConfig_f( void );
void Com_WriteDefaults_f( void );

int Com_ConfigureChecksum( const char *data, int len );
qboolean Com_ConfigureFileChanged( void );
void Com_SetRecommended( int restart );

void Com_ReadCDKey( const char *filename );
void Com_AppendCDKey( const char *filename );
void Com_WriteCDKey( const char *filename, const char *cdkey, const char *checksum );

void Info_Print( const char *s );
void Field_Clear( field_t *edit );
void Field_CompleteCommand( field_t *field );

void Com_Memset( void *dest, int val, size_t count );
void Com_Memcpy( void *dest, const void *src, size_t count );
qboolean Com_Filter( const char *filter, const char *name, int casesensitive );
qboolean Com_FilterPath( const char *filter, const char *name, int casesensitive );

void *Z_MallocInternal( int size );
char *CopyStringInternal( const char *in );
void *Hunk_AllocateTempMemoryInternal( int size );
void Hunk_FreeTempMemoryInternal( void *buf );
void Hunk_ResetTempMark( void );
void Hunk_ClearToStart( void );
void Com_InitHunkMemory( void );

unsigned Com_BlockChecksum( void *buffer, int length );
unsigned Com_BlockChecksumKey( void *buffer, int length, int key );

void *Com_GetClientDObj( int handle );
void *Com_GetServerDObj( int handle );
int Com_GetFreeDObjIndex( void );

/* TWO DIFFERENT LIMITS.
 *
 * MAX_DOBJS is the POOL SLOT count, and retail sizes exactly three things by it:
 *     dobj_pool      0x00894B58  0x16000 = 1024 * 88   (stride `imul 58h`)
 *     alloc bitmap   0x00894A48  0x00100 = 1024 * 2 bits
 *     Com_GetFreeDObjIndex' bound, `cmp eax, 400h` at 0x004388DA and 0x004388FF
 *
 * The handle tables are a DIFFERENT domain -- subscripted by the caller's
 * handle, never by a pool slot -- and retail does not size them together.
 * Com_InitDObj (0x00438B70) and its inlined copy in Com_Restart (0x00438C92)
 * clear them at different lengths:
 *     dobj_serverHandles  0x00894248  0x800 = 1024 entries -- entity numbers
 *     dobj_clientHandles  0x008AC7C0  0x880 = 1088 entries
 * Element width 2 in both, from `mov dobj_clientHandles[ecx*2], ax` (0x00438980)
 * and `movsx eax, dobj_serverHandles[ecx*2]` (0x00438A70).
 *
 * The 64 entries above MAX_GENTITIES are weapon viewmodel slots: the cgame
 * creates them during "LOADING... - items" at handle MAX_GENTITIES + weapon
 * index, and frees/recreates one of them on every weapon change. */
#define MAX_DOBJS 1024
#define MAX_CLIENT_DOBJS ( MAX_GENTITIES + 64 )
extern unsigned short com_clientDObjHandles[MAX_CLIENT_DOBJS];
extern unsigned short com_serverDObjHandles[MAX_DOBJS];
extern byte com_dobjAllocBits[MAX_DOBJS / 4];
extern int com_dobjLastIndex;
extern byte com_dobjPool[];

extern cvar_t   *com_developer;
extern cvar_t   *com_dedicated;
extern cvar_t   *com_speeds;
extern cvar_t   *com_timescale;
extern cvar_t   *com_fixedtime;
extern cvar_t   *com_viewlog;
extern cvar_t   *com_maxfps;
extern cvar_t   *com_logfile;
extern cvar_t   *com_statmon;
extern cvar_t   *com_journal;
extern cvar_t   *com_sv_running;
extern cvar_t   *com_cl_running;
extern cvar_t   *com_introplayed;
extern cvar_t   *com_animCheck;
extern int com_frameTime;
extern int com_frameNumber;
extern qboolean com_errorEntered;
extern qboolean com_fullyInitialized;
extern int com_skelTimeStamp;
extern int fs_loadingMode;
extern int time_game, time_frontend, time_backend;

/*
=============================================================================
                        CMD
=============================================================================
*/

void Cbuf_Init( void );
void Cbuf_AddText( const char *text );
void Cbuf_InsertText( const char *text );
void Cbuf_ExecuteText( int exec_when, const char *text );
void Cbuf_Execute( void );

void Cmd_Init( void );
void Cmd_Shutdown( void );
void Cmd_AddCommand( const char *cmd_name, xcommand_t function );
void Cmd_RemoveCommand( const char *cmd_name );
void Cmd_CommandCompletion( void ( *callback )( const char *s ) );
int Cmd_Argc( void );
char *Cmd_Argv( int arg );
void Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
char *Cmd_Args( void );
void Cmd_ArgsBuffer( char *buffer, int bufferLength );
void Cmd_TokenizeString( const char *text_in );
void Cmd_ExecuteString( const char *text );

/*
=============================================================================
                        CVAR
=============================================================================
*/

cvar_t *Cvar_Get( const char *var_name, const char *var_value, int flags );
cvar_t *Cvar_Set2( const char *var_name, const char *value, qboolean force );
cvar_t *Cvar_FindVar( const char *var_name );
void Cvar_Set( const char *var_name, const char *value );
void Cvar_SetLatched( const char *var_name, const char *value );
void Cvar_SetValue( const char *var_name, float value );
void Cvar_Reset( const char *var_name );
void Cvar_VMSet( const char *value, vmCvar_t *vmCvar );
void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void Cvar_Update( vmCvar_t *vmCvar );
void Cvar_Init( void );
void Cvar_Shutdown( void );
void Cvar_AddCommands( void );

float Cvar_VariableValue( const char *var_name );
int Cvar_VariableIntegerValue( const char *var_name );
char *Cvar_VariableString( const char *var_name );
void Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void Cvar_CommandCompletion( void ( *callback )( const char *s ) );
void Cvar_SetCheatState( void );
qboolean Cvar_Command( void );
void Cvar_WriteVariables( fileHandle_t f );
void Cvar_WriteDefaults( fileHandle_t f );
char *Cvar_InfoString( int bit );
char *Cvar_InfoString_Big( int bit );
void Cvar_InfoStringBuffer( int bit, char *buff, int buffsize );
void Com_CvarDump( int channel );
void SV_SetConfig( int start, int max, int bit );

extern cvar_t   *cvar_vars;
extern cvar_t   *cvar_cheats;
extern int cvar_modifiedFlags;
extern cvar_t cvar_indexes[];
extern int cvar_numIndexes;

/*
=============================================================================
                        FILESYSTEM
=============================================================================
*/

qboolean FS_Initialized( void );
void FS_InitFilesystem( void );
void FS_Restart( int checksumFeed );
qboolean FS_ConditionalRestart( int checksumFeed );
void FS_Shutdown( qboolean closemfp );
/* 0x00428CD0.  The accessor for files.c's `static int fs_loadStack`.  Retail's
 * Com_InitHunkMemory reads the global directly (`mov eax, fs_loadStack` at
 * 0x00431E00) because /GL inlined this two-instruction body across the unit
 * boundary; the source could only have gone through the accessor. */
int FS_LoadStack( void );
void FS_AddCommands( void );
void FS_RemoveCommands( void );

int FS_FOpenFileRead( const char *qpath, fileHandle_t *file, qboolean uniqueFILE );
int FS_FOpenFileReadStream( const char *qpath, fileHandle_t *file );
fileHandle_t FS_FOpenFileWrite( const char *filename );
fileHandle_t FS_FOpenTextFileWrite( const char *filename );
fileHandle_t FS_FOpenFileAppend( const char *filename );
int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
void FS_FCloseFile( fileHandle_t f );
int FS_Read( void *buffer, int len, fileHandle_t f );
int FS_Read2( void *buffer, int len, fileHandle_t f );
int FS_Write( const void *buffer, int len, fileHandle_t f );
void QDECL FS_Printf( fileHandle_t f, const char *fmt, ... );
int FS_Seek( fileHandle_t f, int offset, int origin );
int FS_FTell( fileHandle_t f );
void FS_Flush( fileHandle_t f );
void FS_ForceFlush( fileHandle_t f );
int FS_filelength( fileHandle_t f );
int FS_ReadFile( const char *qpath, void **buffer );
void FS_FreeFile( void *buffer );
void FS_WriteFile( const char *qpath, const void *buffer, int size );
void FS_ResetFiles( void );
qboolean FS_CopyFile( char *fromOSPath, char *toOSPath );
qboolean FS_CreatePath( char *OSPath );

char **FS_ListFiles( const char *path, const char *extension, int *numfiles );
void FS_FreeFileList( char **list );
int FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize );
int FS_GetModList( char *listbuf, int bufsize );

qboolean FS_SV_FileExists( const char *file );
fileHandle_t FS_SV_FOpenFileWrite( const char *filename );
int FS_SV_FOpenFileRead( const char *filename, fileHandle_t *fp );
void FS_SV_Rename( const char *from, const char *to );

void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames );
int FS_ListUnapprovedReferencedPaks( char *buffer, int size );
void FS_PureServerSetReferencedPaks( const char *pakSums, const char *pakNames );
void FS_ShutdownServerPakNames( void );
void FS_ShutdownServerReferencedPaks( void );
void FS_ClearPakReferences( int flags );
void FS_Path_f( void );
void FS_FullPath_f( void );
void FS_Dir_f( void );
void FS_NewDir_f( void );
void FS_TouchFile_f( void );

/*
=============================================================================
                        Q_SHARED  (universal/q_shared.c)
=============================================================================
*/

float Com_Clamp( float min, float max, float value );
char *Com_SkipPath( char *pathname );
void Com_StripExtension( const char *in, char *out );
void Com_StripFilename( char *in, char *out );
void Com_DefaultExtension( char *path, int maxSize, const char *extension );
qboolean Com_BitCheck( const int array[], int bitNum );
void Com_BitSet( int array[], int bitNum );
void Com_BitClear( int array[], int bitNum );

void Swap_Init( void );
short BigShort( short l );
short LittleShort( short l );
int BigLong( int l );
int LittleLong( int l );
float BigFloat( float l );
float LittleFloat( float l );

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );
int Q_isnumeric( int c );
int Q_isalphanumeric( int c );
int Q_isnamechar( int c );
char *Q_strrchr( const char *string, int c );
void Q_strncpyz( char *dest, const char *src, int destsize );
int Q_stricmpn( const char *s1, const char *s2, int n );
int Q_strncmp( const char *s1, const char *s2, int n );
int Q_stricmp( const char *s1, const char *s2 );
char *Q_strlwr( char *s1 );
char *Q_strupr( char *s1 );
void Q_strcat( char *dest, int size, const char *src );
char *Q_CleanStr( char *string );
char Q_CleanCharacter( char c );
int Q_strncasecmp( char *s1, char *s2, int n );
int Q_strcasecmp( char *s1, char *s2 );
int Q_vsnprintf( char *dest, size_t size, const char *fmt, va_list ap );
/* 0x0044AC60.  Returns _vsnprintf's result, as retail does; always terminates
 * dest at size-1.  Retail has NO truncation warning -- see the definition. */
int QDECL Com_sprintf( char *dest, int size, const char *fmt, ... );
char *QDECL va( char *format, ... );
float *tv( float x, float y, float z );

char *Info_ValueForKey( const char *s, const char *key );
void Info_NextPair( const char **head, char *key, char *value );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_Big( char *s, const char *key );
qboolean Info_Validate( const char *s );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );

float GetLeanFraction( float f );
float UnGetLeanFraction( float f );
void AddLeanToPosition( float *pos, float yaw, float leanFrac, float leanDist, float scale );
void OrientationPosToWorldPos( const float *orient, const float *pos, float *out );
void OrientationDirToWorldDir( const float *orient, const float *dir, float *out );
void OrientationPosFromWorldPos( const float *orient, const float *worldPos, float *out );
void OrientationDirFromWorldDir( const float *orient, const float *worldDir, float *out );

int Com_SurfaceTypeFromName( const char *name );
const char *Com_SurfaceTypeToName( int type );
int Com_AddToString( const char *src, char *msg, int len, int maxlen, int mayAddQuotes );

/*
=============================================================================
                        PARSER, NOISE, HUFFMAN, MD4, VM
=============================================================================
*/

extern parseInfo_t *parseInfo;   /* the active parse session */
void Com_BeginParseSession( const char *name );
void Com_EndParseSession( void );
void Com_ResetParseSessions( void );
char *Com_Parse( char **data_p );
char *Com_ParseExt( char **data_p, qboolean allowLineBreaks );
extern parseInfo_t  *parseInfo;

void Com_NoiseInit( void );
float GetNoiseValue( int x, int y, int z, int t );
/* 0x004DD4F0.  FOUR arguments, result in ST0 -- there is no output pointer
 * (each caller's `push edi` is a register save, not a fifth argument). */
float Com_NoiseGet4f( float x, float y, float z, float t );

void Huff_Init( huffman_t *huff );
void Huff_Compress( msg_t *mbuf, int offset );
void Huff_Decompress( msg_t *mbuf, int offset );
void Huff_addRef( huff_t *huff, byte ch );
int Huff_Receive( node_t *node, int *ch, byte *fin );
void Huff_offsetReceive( node_t *node, int *ch, byte *fin, int *offset );
void Huff_transmit( huff_t *huff, int ch, byte *fout );
void Huff_offsetTransmit( huff_t *huff, int ch, byte *fout, int *offset );
void Huff_putBit( int bit, byte *fout, int *offset );
int Huff_getBit( byte *fin, int *offset );
extern qboolean msgInit;
void MSG_initHuffman( void );

void VM_Init( void );
vm_t *VM_Create( const char *module, int ( *systemCalls )( int * ) );
vm_t *VM_Restart( vm_t *vm );
void VM_Free( vm_t *vm );
void VM_Clear( void );
int QDECL VM_Call( vm_t *vm, int callnum, ... );
extern vm_t     *currentVM;

char *getBuildNumber( void );

void StatMon_Warning( int type, int duration, const char *materialName );
void StatMon_GetStatsArray( const statmonitor_t **array, int *count );
void StatMon_Reset( void );

/*
=============================================================================
                        SYSTEM  (win32)
=============================================================================
*/

void Sys_Init( void );
void *Com_MallocOrDie( int size );   /* not retail -- see cmd.c */
void QDECL Sys_Error( const char *error, ... );
int Sys_Milliseconds( void );
sysEvent_t Sys_GetEvent( void );
void Sys_ShowConsole( int level, qboolean quitOnClose );
void Sys_OutOfMemoryError( void );
qboolean Sys_ShouldReconfigure( void );
void Sys_SetConfigCvars( int configSum );
int Sys_GetVideoRam( void );	/* bytes, 0 when unknown -- the MB figure is sys_vidMBValue */
int Sys_GetSystemRam( void );
float Sys_GetCPUSpeed( void );
qboolean Sys_ConfigureChecksumChanged( int checksum );
qboolean Sys_CheckCrashOrRerun( void );
void Sys_DestroySplashWindow( void );
void Sys_DestroyConsole( void );
/* The single-instance-marker cleanup (win32/win_main.c, retail 0x004640A0).
   Retail inlines DeleteFileA(FileName), FileName = 0x008E3B10, at five sites:

     Com_Init   0x004377E8   (Com_InitHunkMemory follows at 0x004377F3)
     Com_Frame  0x004381DD   (the com_dedicated re-test follows at 0x004381E8)
     Com_Quit_f 0x00463172
     Sys_GetSystemRam's two low-memory exits 0x00460CC8 / 0x00460DDC */
int  Sys_DeleteInstanceMarker( void );
/* Retail's Com_Quit_f (0x00435D80) tail, in order: timeEndPeriod(1),
   IN_Shutdown, the Sys_DestroyConsole window teardown, DeleteFileA(FileName),
   `seh_localizationBase = 0; seh_localizationTable = 0;`, Cvar_Shutdown,
   Cmd_Shutdown, Key_Shutdown, exit(0).

   Sys_ShutdownTimer is timeEndPeriod(1), in win32/win_platform.c beside the
   timeBeginPeriod(1) it undoes.

   Sys_PlatformExit is a `return 0` link stub, and it is NOT a platform exit
   hook: it sits in Key_Shutdown's slot (retail 0x0040E2B0), between
   Cmd_Shutdown and exit(0).  Key_Shutdown lives in client_mp/, which a
   dedicated server does not link, so binding it needs a dedicated/ boundary
   stub as well, and everything it would free is reclaimed by the exit(0) two
   lines later.  Do not "implement" Sys_PlatformExit as something else. */
void Sys_ShutdownTimer( void );
void Sys_PlatformExit( void );
qboolean Sys_IsLANAddress( netadr_t adr );
/* win32/win_shared.c -- the search-path defaults.  Sys_DefaultHomePath
 * returns NULL on Windows; callers fall back to fs_basepath. */
char *Sys_DefaultCDPath( void );
char *Sys_DefaultBasePath( void );
char *Sys_DefaultHomePath( void );
char *Sys_DefaultInstallPath( void );
char *Sys_Cwd( void );
char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, int wantsubs );
void Sys_FreeFileList( char **list );
void *Sys_LoadDll( const char *name, char *fqpath, int ( **entryPoint )( int, ... ), int ( *systemcalls )( int, ... ) );
char *SEH_GetLanguageString( const char *key );
char *SEH_LocalizeTextMessage( const char *text, const char *type, int flag );
void SEH_UpdateLanguageInfo( void );
void Conbuf_AppendText( const char *msg );
void IN_Shutdown( void );
void IN_Init( void );       /* win32/win_input.c 0x00461960 -- called from Sys_Init's tail */

/*
=============================================================================
                        DECLARATIONS PENDING THEIR OWN HEADERS

    Each will move to its real header as its unit lands.
=============================================================================
*/

void CL_Init( void );
void CL_Shutdown( void );
void CL_Frame( int msec );
void CL_Disconnect( qboolean showMainMenu );
void CL_ShutdownCGame( void );
void CL_ShutdownUI( void );
void CL_ShutdownRef( void );
void CL_StartHunkUsers( void );
/* __usercall(txt@<ecx>, channel@<ebx>, msgtime, width) in retail.  msgtime 0
 * means "derive it from the channel's cvar". */
void CL_ConsolePrint( const char *txt, int channel, int msgtime, int width );
void CL_ConsoleFixPosition( int a );
void CL_PacketEvent( netadr_t from, msg_t *msg );
void CL_KeyEvent( int key, int down, unsigned time );
void CL_CharEvent( int key );
void CL_MouseEvent( int dx, int dy, int time );
void CL_JoystickEvent( int axis, int value, int time );
void CL_ForwardCommandToServer( const char *string );
qboolean CL_CDKeyValidate( const char *key, const char *checksum );

/* CL_AddDebugLine  0x00414410, defined in client_mp/cl_main_mp.c.  Retail is
   __usercall(start@<ebx>, end@<edi>, color@<esi>, depthTest, duration,
   fromServer) -- six arguments, three of them in registers.  Declared here,
   not as `extern int CL_AddDebugLine();` in each caller, so the unprototyped
   spelling cannot disable argument checking.  `color` is a vec4 (four
   floats), not a vec3.  depthTest and fromServer are int rather than qboolean
   because the game-trap call site passes `args[N]`. */
void CL_AddDebugLine( const float *start, const float *end, const float *color,
					  int depthTest, int duration, int fromServer );
/* The other two of the same family, declared here for the same reason.

   CL_AddDebugString  0x00414340 -- __usercall(origin@<ebx>, color@<edi>,
   scale, text, fromServer).  Two register arguments, three stack.  `color` is
   a vec4 like CL_AddDebugLine's, read [edi] .. [edi+0Ch].  `scale` arrives as
   a raw dword the callee stores straight into the record, so the game trap
   must reinterpret args[3]'s bits rather than convert them -- see
   SV_ArgFloat at the G_ADD_DEBUG_STRING site.

   CL_FlushDebugData  0x00414500 -- fromServer@<ebx>, nothing on the stack.
   EBX is never written in the body, only compared against the per-entry
   fromServer byte; the two retail call sites set it immediately before the
   call (SV_Frame 0x0045B520 `mov ebx, 1`, RB_SwapBuffers 0x004D87E4
   `xor ebx, ebx`), which is what makes it an argument and not a stale
   register.  `int`, not `qboolean`, to match CL_AddDebugLine above. */
void CL_AddDebugString( const float *origin, const float *color, float scale,
						const char *text, int fromServer );
void CL_FlushDebugData( int fromServer );
/* CL_SetupForNewServerMap  0x004142F0, defined in client_mp/cl_main_mp.c.
   __usercall(mapname@<edi>, gametype@<esi>), no stack arguments -- the callee's
   `add esp, 24h` accounts for exactly its own nine outgoing pushes.  Declared
   here rather than locally in its only caller, CL_ConnectionlessPacket
   0x004109D0, so an `extern int CL_SetupForNewServerMap();` spelling cannot
   recur: both arguments are handed straight to Com_Printf as `%s`, so a
   dropped argument is a wild-pointer read, not a wrong number. */
void CL_SetupForNewServerMap( const char *mapname, const char *gametype );
void CIN_CloseAllVideos( void );
void Key_WriteBindings( fileHandle_t f );
void Key_Bind_f( void );
void Key_Unbind_f( void );
void Key_UnbindAll_f( void );
void Key_Bindlist_f( void );

void SV_Init( void );
void SV_Shutdown( const char *finalmsg );
void SV_ShutdownGameProgs( void );
void SV_Frame( int msec );
void SV_PacketEvent( netadr_t from, msg_t *msg );
void SV_SetConfigValueForKey( int start, const char *key, const char *value, int max );

void NET_Init( void );
qboolean NET_GetLoopPacket( netsrc_t sock, netadr_t *net_from, msg_t *net_message );
void Netchan_Init( int qport );
void MSG_Init( msg_t *buf, byte *data, int length );
int MSG_ReadBits( msg_t *msg, int bits );

void Scr_Init( void );
void Scr_Shutdown( void );

/* The server's game VM, retail 0x014073D0, defined in
 * server_mp/sv_init_mp_hand.c.  Declared here because qcommon/cmd.c forwards
 * unrecognised console words to it and must not include server.h (cm_local.h
 * and server.h collide).  There is no separate `gvm`; do not reintroduce one. */
extern vm_t     *vm;

/* The two client VMs, retail 0x01617348 and 0x0161747C. */
extern vm_t     *cgvm;
extern vm_t     *uivm;
extern void     *s_hunkData;
extern fileHandle_t com_journalDataFile;

/*
 * THE FOUR clientStatic_t FIELDS qcommon TOUCHES, AS THE FOUR REAL GLOBALS.
 *
 * There is no `cls` object in retail: all four are separate globals
 * (0x0155F2C8 is not even in the same neighbourhood), and retail spells them
 * as such -- Com_Shutdown 0x004358C9 / 0x004358D3, Com_ErrorCleanup
 * 0x0043597F, and the StatMon/UI_CONSOLE_COMMAND sites.  Com_Shutdown's two
 * stores are what tell CL_StartHunkUsers to bring the renderer back up after
 * Hunk_ClearToStart.
 */
extern int cls_rendererStarted;    /* 0x0155F3CC */
extern int cls_loadingPlaque;      /* 0x0155F3D4 -- cls.uiStarted, see cl_main_mp.c */
extern int cls_cddialog;           /* 0x0155F2C8 */
extern int cls_realtime;           /* 0x0155F3E0 */

/* Minimal server view needed by qcommon; the real server_t lands with sv_*. */
typedef enum { SS_DEAD, SS_LOADING, SS_GAME } serverState_t;
typedef struct { serverState_t state; } server_public_t;
extern server_public_t sv;
extern int g_unk_A9CC58;          /* TODO: unresolved global at 0x00A9CC58 */

extern char cl_cdkey[34];
extern char cl_cdkeychecksum[10];
extern int cl_cdkeyZero;
extern const char *g_languages[];
extern cvar_t *cl_language;
extern cvar_t *fs_ignoreLocalized;
extern int fs_numServerPaks;
extern int fs_serverPaks[];

#endif  /* __QCOMMON_H__ */
