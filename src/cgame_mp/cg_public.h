/*
 * cg_public.h -- the client game module / engine interface for cgame_mp_x86.dll.
 *
 * Call of Duty 1.1 multiplayer.  RTCW's cgame/cg_public.h is the structural
 * model: the shared render and snapshot view, the trap numbers the module calls
 * out on (cgameImport_t) and the entry points the engine calls in
 * (cgameExport_t).
 *
 * The two halves of this ABI are the wrapper bodies in cg_syscalls_mp.c
 * (0x30030460..0x300322C0 in cgame_mp_x86.dll) and CL_CgameSystemCalls in
 * client_mp/cl_cgame_mp.c (retail 0x00401DF0), which says what the engine
 * does with every argument.
 *
 * entityState_t is declared here AND in game_mp/g_public.h AND in
 * server_mp/server.h.  That is the retail tree's own arrangement -- each module
 * carries its own copy of the shared ABI and no module's header includes
 * another's.  The layout and the size assertion are the same ones those two
 * assert.
 *
 * @fidelity: likely
 */

#ifndef __CG_PUBLIC_H__
#define __CG_PUBLIC_H__

#include "../universal/q_shared.h"

#ifndef QDECL
#define QDECL   __cdecl
#endif

typedef int qhandle_t;
typedef int sfxHandle_t;        /* the sound-alias handle cgMedia_t stores */

/*
 * 0x3E80.  CL_CgameSystemCalls case 79 copies 0x5E84 bytes out of
 * cl.gameState (retail 0x00402EE0); MAX_CONFIGSTRINGS is 0x800, so the string
 * pool is 0x5E84 - 0x800*4 - 4.
 */
#include "../universal/protocol_limits.h"

#define MAX_ENTITIES_IN_SNAPSHOT    256     /* CL_GetSnapshot 0x00401120 */
#define MAX_CLIENTS_IN_SNAPSHOT     64

/*
=============================================================================

	THE SHARED ENTITY VIEW

=============================================================================
*/

typedef struct trajectory_t {
	int trType;
	int trTime;
	int trDuration;
	vec3_t trBase;
	vec3_t trDelta;
} trajectory_t;
COD1_ASSERT_SIZE( trajectory_t, 36 );

typedef struct entityState_s {
	int number;
	int eType;
	int eFlags;
	trajectory_t pos;
	trajectory_t apos;
	int time;
	int time2;
	vec3_t origin2;
	vec3_t angles2;
	int otherEntityNum;
	int attackerEntityNum;
	int groundEntityNum;
	int constantLight;
	int loopSound;
	int surfType;
	int index;
	int clientNum;
	int iHeadIcon;
	int iHeadIconTeam;
	int solid;
	int eventParm;
	int eventSequence;
	int events[4];
	unsigned int eventParms[4];
	int weapon;
	int legsAnim;
	int torsoAnim;
	float leanf;
	int scale;
	int dmgFlags;
	int animMovetype;
	float fTorsoHeight;
	float fTorsoPitch;
	float fWaistPitch;
} entityState_t;
COD1_ASSERT_SIZE( entityState_t, 240 );

/*
 * The per-client record the server broadcasts alongside the entities.  Only
 * the size is known here -- CL_GetSnapshot copies 0x5C bytes per client at
 * 0x004011E9, and server_mp/server.h carries the same record as `byte cs[92]`
 * with the note that its first dword is the client number.  cg_snapshot_mp.c
 * carries the field layout.
 */
typedef struct clientState_s {
	byte unknown_0x00[92];
} clientState_t;
COD1_ASSERT_SIZE( clientState_t, 92 );

/*
 * Expanded trap_GetGameState target (syscall 300, with a size check).
 * Legacy syscall 79 retains the stockGameState_t layout below.
 */
typedef struct gameState_t {
	int stringOffsets[MAX_CONFIGSTRINGS];
	char stringData[EXTENDED_MAX_GAMESTATE_CHARS];
	int dataCount;
} gameState_t;
COD1_ASSERT_SIZE( gameState_t, 0x2004 + EXTENDED_MAX_GAMESTATE_CHARS );

/* Preserve the original cgs_t layout. Expanded strings live separately. */
typedef struct stockGameState_t {
	int stringOffsets[MAX_CONFIGSTRINGS];
	char stringData[STOCK_MAX_GAMESTATE_CHARS];
	int dataCount;
} stockGameState_t;
COD1_ASSERT_SIZE( stockGameState_t, 0x5E84 );

/*
 * trap_GetSnapshot's output record.  Every offset below is the constant
 * CL_GetSnapshot writes to (client_mp/cl_records.h SNAP_OUT_*); numServerCommands
 * is Q3's field in the one slot CL_GetSnapshot leaves alone.
 */
typedef struct snapshot_t {
	/* +0x00000 */ int snapFlags;
	/* +0x00004 */ int ping;
	/* +0x00008 */ int serverTime;
	/* +0x0000C */ playerState_t ps;
	/* +0x020DC */ int numEntities;
	/* +0x020E0 */ int numClients;
	/* +0x020E4 */ entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];
	/* +0x110E4 */ clientState_t clients[MAX_CLIENTS_IN_SNAPSHOT];
	/* +0x127E4 */ int numServerCommands;
	/* +0x127E8 */ int serverCommandSequence;
} snapshot_t;
COD1_ASSERT_SIZE( snapshot_t, 0x127EC + PLAYERSTATE_EXTRA_BYTES );

/*
=============================================================================

	THE SHARED RENDER VIEW

=============================================================================
*/

/*
 * trap_GetGlconfig's target.  CL_CgameSystemCalls case 78 copies 0xA0 bytes out
 * of cls.glconfig; the layout is client_mp/cl_refapi.h's, which the renderer
 * fills in.
 */
typedef struct glconfig_t {
	/* +0x00 */ char   *renderer_string;
	/* +0x04 */ char   *vendor_string;
	/* +0x08 */ char   *version_string;
	/* +0x0C */ char   *extensions_string;
	/* +0x10 */ char   *wextensions_string;    /* CoD1 addition */
	/* +0x14 */ int     maxTextureSize;
	/* +0x18 */ int     maxActiveTextures;
	/* +0x1C */ int     maxLights;             /* CoD1 addition */
	/* +0x20 */ int     colorBits;
	/* +0x24 */ int     depthBits;
	/* +0x28 */ int     stencilBits;
	/* +0x2C */ int     deviceSupportsGamma;
	/* +0x30 */ int     unknown_0x30[2];
	/* +0x38 */ int     textureEnvAddAvailable;
	/* +0x3C */ int     unknown_0x3C[6];
	/* +0x54 */ int     NVFogAvailable;
	/* +0x58 */ int     unknown_0x58[11];
	/* +0x84 */ int     vidWidth;
	/* +0x88 */ int     vidHeight;
	/* +0x8C */ float   windowAspect;
	/* +0x90 */ int     displayFrequency;
	/* +0x94 */ int     isFullscreen;
	/* +0x98 */ int     stereoEnabled;
	/* +0x9C */ int     unknown_0x9C;
} glconfig_t;
COD1_ASSERT_SIZE( glconfig_t, 160 );

/*
 * trap_R_AddRefEntityToScene's argument.  156 bytes: CG_Beam (0x3001B89E)
 * clears the whole record with `rep stosd` of 0x27 dwords.  The named offsets
 * are the ones a builder stores; the holes are left as padding:
 *
 *   +0x00 reType          CG_Beam 0x3001B905 stores 6
 *   +0x04 renderfx        CG_Beam 0x3001B90D stores 0x40
 *   +0x1C axis[3]         CG_Beam 0x3001B8E1.. stores the identity basis
 *   +0x44 origin          CG_Beam 0x3001B8AF copies es.pos.trBase
 *   +0x54 oldorigin       CG_Beam 0x3001B8C4 copies es.origin2
 *   +0x6C shaderRGBA      CG_AddFadeRGB 0x3002008D.. stores four bytes
 *
 * Q3 puts shaderRGBA at 0x74, so CoD1's middle is NOT Q3's -- do not fill the
 * holes from Q3 text without checking the binary.
 */
typedef struct refEntity_s {
	/* +0x00 */ int         reType;
	/* +0x04 */ int         renderfx;
	/* +0x08 */ byte        unknown_0x08[0x14];
	/* +0x1C */ vec3_t      axis[3];
	/* +0x40 */ byte        unknown_0x40[0x04];
	/* +0x44 */ vec3_t      origin;
	/* +0x50 */ byte        unknown_0x50[0x04];
	/* +0x54 */ vec3_t      oldorigin;
	/* +0x60 */ byte        unknown_0x60[0x0C];
	/* +0x6C */ byte        shaderRGBA[4];
	/* +0x70 */ byte        unknown_0x70[0x2C];
} refEntity_t;
COD1_ASSERT_SIZE( refEntity_t, 156 );

/*
 * trap_R_RenderScene's argument, and cg.refdef.  Q3's head verbatim and
 * nothing after it: cg.refdef.x is 0x3020957C and cg.refdefViewAngles[1]
 * 0x302095D0, which puts the end of the record at
 * 0x302095CC -- 80 bytes.  cg.refdef.vieworg (0x30209594) is +0x18 and the
 * rdflags dword CG_DrawActive masks (0x302095C8) is +0x4C, both Q3's offsets.
 */
typedef struct refdef_s {
	/* +0x00 */ int x, y, width, height;
	/* +0x10 */ float fov_x, fov_y;
	/* +0x18 */ vec3_t vieworg;
	/* +0x24 */ vec3_t viewaxis[3];
	/* +0x48 */ int time;
	/* +0x4C */ int rdflags;
} refdef_t;
COD1_ASSERT_SIZE( refdef_t, 80 );

/*
 * trap_R_AddPolyToScene's vertex.  32 bytes, NOT Q3's 24 -- CoD carries a
 * second texcoord pair.  CG_AddMarks reaches modulate at +28 inside a
 * 32-byte stride (0x300239E2); CG_PlayerSplash and cg_weapons' tracer quad
 * build four-vert quads on the same stride.
 * The second st pair is what trap_R_MarkFragments fills alongside the first;
 * `st2` is an inferred name.
 */
typedef struct polyVert_t {
	/* +0x00 */ vec3_t xyz;
	/* +0x0C */ float st[2];
	/* +0x14 */ float st2[2];
	/* +0x1C */ byte modulate[4];
} polyVert_t;
COD1_ASSERT_SIZE( polyVert_t, 32 );

/*
 * trap_R_MarkFragments' output record.  12 bytes, not Q3's 8: CoD prepended
 * the shader the hit surface resolved to, and CG_ImpactMark puts THAT into the
 * poly rather than its own markShader argument (0x30023837, 0x30023875).  The
 * fragment loop strides 0xC and reads three dwords (0x300238D1).
 */
typedef struct markFragment_t {
	/* +0x00 */ qhandle_t   markShader;
	/* +0x04 */ int         firstPoint;
	/* +0x08 */ int         numPoints;
} markFragment_t;
COD1_ASSERT_SIZE( markFragment_t, 12 );

/*
 * The script parser token trap_PC_ReadToken fills in.  1040 bytes: in
 * ui_shared.c's PC_Int_Parse (0x300404D0) the local token is 0x410 bytes, its
 * `type` at +0 is compared against 3 (TT_NUMBER) and its `string` at +0x10 is
 * compared against '-'.  That is Q3's botlib layout exactly.
 */
#define MAX_TOKENLENGTH     1024

typedef struct pc_token_s {
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
} pc_token_t;
COD1_ASSERT_SIZE( pc_token_t, 1040 );

/*
=============================================================================

	THE TRAP NUMBERS

	Each constant is the immediate the matching wrapper in cg_syscalls_mp.c
	pushes before `call syscall`, and the comment is what
	CL_CgameSystemCalls does with it.  A CG_SYSCALL_0xNN spelling means the
	Mac symbol table did not name that wrapper.

	Trap 34 is absent on BOTH sides: CL_CgameSystemCalls goes straight from
	case 33 to case 35, and no wrapper pushes it.

	Traps the engine dispatcher answers but no wrapper issues: 101
	(Cmd_RemoveCommand), 109-117 and 120 -- the QVM memory/math block (memset,
	memcpy, strncpy, sin, cos, atan2, sqrt, floor, ceil, acos), unused because
	a native DLL calls the CRT directly.  The game DLL's gameImport_t has the
	same hole for the same reason.

	Traps a wrapper issues that the engine has NO case for: 236, 237, 238,
	239.  Retail falls through to `Com_Error( ERR_DROP, "Bad cgame system
	trap: %i" )` on all four, and nothing in the DLL calls those four
	wrappers -- they are dead code the linker kept.

=============================================================================
*/

typedef enum
{
	CG_PRINT                               =   0,
	CG_ERROR                               =   1,
	CG_GAMEMESSAGE                         =   2,
	CG_BOLDGAMEMESSAGE                     =   3,
	CG_DEATHMESSAGE                        =   4,
	CG_SUBTITLE                            =   5,
	CG_MILLISECONDS                        =   6,
	CG_CVAR_REGISTER                       =   7,
	CG_CVAR_UPDATE                         =   8,
	CG_CVAR_SET                            =   9,   /* Cvar_Set2( name, value, qtrue ) */
	CG_CVAR_SETVAR                         =  10,   /* Cvar_VMSet */
	CG_CVAR_VARIABLESTRINGBUFFER           =  11,
	CG_ARGC                                =  12,
	CG_ARGV                                =  13,
	CG_ARGS                                =  14,
	CG_FS_FOPENFILE                        =  15,
	CG_FS_READ                             =  16,
	CG_FS_WRITE                            =  17,
	CG_FS_FCLOSEFILE                       =  18,
	CG_FS_GETFILELIST                      =  19,
	CG_COM_SAVECVARSTOBUFFER               =  20,
	CG_COM_LOADCVARSFROMBUFFER             =  21,
	CG_SENDCONSOLECOMMAND                  =  22,   /* Cbuf_AddText */
	CG_ADDCOMMAND                          =  23,
	CG_SENDCLIENTCOMMAND                   =  24,   /* CL_AddReliableCommand */
	CG_UPDATESCREEN                        =  25,
	CG_DRAWNOTIFYLINES                     =  26,   /* Con_DrawNotify */
	CG_DRAWBOLDMESSAGES                    =  27,   /* Con_DrawBoldMessages */
	CG_DRAWMINICONSOLE                     =  28,   /* Con_DrawMiniConsole */
	CG_DRAWSUBTITLES                       =  29,   /* Con_DrawSubtitles */
	CG_DRAWSAY                             =  30,   /* Con_DrawSay */
	CG_CM_LOADMAP                          =  31,
	CG_CM_NUMINLINEMODELS                  =  32,   /* cm.numSubModels */
	CG_CM_INLINEMODEL                      =  33,
	/* 34 is unused: the engine has no case for it and no wrapper issues it. */
	/* The engine reads a third `contents` slot for 35 and 42 that neither
	   wrapper pushes; the wrappers are right and the slot is stale.  See
	   trap_CM_TempBoxModel in cg_syscalls_mp.c for the disassembly. */
	CG_CM_TEMPBOXMODEL                     =  35,   /* CM_TempBoxModel( mins, maxs, contents, qfalse ) */
	CG_CM_POINTCONTENTS                    =  36,
	CG_CM_TRANSFORMEDPOINTCONTENTS         =  37,
	CG_CM_BOXTRACE                         =  38,   /* CM_BoxTrace( .., qfalse ) */
	CG_CM_TRANSFORMEDBOXTRACE              =  39,   /* CM_TransformedBoxTraceExternal( .., qfalse ) */
	CG_CM_CAPSULETRACE                     =  40,   /* CM_BoxTrace( .., qtrue ) */
	CG_CM_TRANSFORMEDCAPSULETRACE          =  41,   /* CM_TransformedBoxTraceExternal( .., qtrue ) */
	CG_CM_TEMPCAPSULEMODEL                 =  42,   /* CM_TempBoxModel( mins, maxs, contents, qtrue ) */
	CG_R_MARKFRAGMENTS                     =  43,   /* re slot +0x98 */
	CG_R_LOADWORLDMAP                      =  44,
	CG_R_REGISTERMODEL                     =  45,
	CG_R_FINISHLOADINGMODELS               =  46,
	CG_SYSCALL_0X2F                        =  47,   /* re.SetIgnorePrecacheErrors */
	CG_R_REGISTERSHADER                    =  48,
	CG_SYSCALL_0X31                        =  49,   /* re.GetShaderFromModel */
	CG_R_GETXMODELBYHANDLE                 =  50,
	CG_R_REGISTERFONT                      =  51,
	CG_R_TEXT_WIDTH                        =  52,
	CG_R_TEXT_HEIGHT                       =  53,
	CG_R_TEXT_PAINT                        =  54,
	CG_R_TEXT_PAINTWITHCURSOR              =  55,
	CG_SE_TRANSLATEREFERENCE               =  56,   /* SEH_StringEd_GetString */
	CG_SE_LOCALIZEMESSAGE                  =  57,   /* SEH_LocalizeTextMessage */
	CG_SE_PRINTSTRLEN                      =  58,
	CG_SE_READCHARFROMSTRING               =  59,
	CG_R_CLEARSCENE                        =  60,
	CG_R_ADDREFENTITYTOSCENE               =  61,
	CG_SYSCALL_0X3E                        =  62,   /* re.GetEntityToken */
	CG_HUNKUSED                            =  63,   /* com_hunkMeminfoRunningTotal */
	CG_R_ADDPOLYTOSCENE                    =  64,
	/* the wrapper's placeholder name is trap_syscall_0x42; it pushes 0x41 (0x30030F88) */
	CG_SYSCALL_0X41                        =  65,   /* re.AddPolysToScene */
	CG_R_ADDLIGHTTOSCENE                   =  66,
	CG_SYSCALL_0X43                        =  67,   /* re.AddCoronaToScene */
	CG_R_SETFOG                            =  68,
	CG_R_RENDERSCENE                       =  69,
	CG_R_SAVESCREEN                        =  70,
	CG_R_BLENDSAVEDSCREEN                  =  71,
	CG_R_SETCOLOR                          =  72,
	CG_R_DRAWSTRETCHPIC                    =  73,
	CG_SYSCALL_0X4A                        =  74,   /* re.StretchPicGradient */
	CG_R_DRAWSTRETCHPICROTATE              =  75,
	CG_R_DRAWQUADPIC                       =  76,
	CG_R_MODELBOUNDS                       =  77,
	CG_GETGLCONFIG                         =  78,
	CG_GETGAMESTATE                        =  300, /* expanded buffer, size argument */
	CG_GETCURRENTSNAPSHOTNUMBER            =  80,
	CG_GETSNAPSHOT                         =  301,
	CG_GETSERVERCOMMAND                    =  82,
	CG_GETCURRENTCMDNUMBER                 =  83,
	CG_GETUSERCMD                          =  84,
	CG_SYSCALL_0X55                        =  85,   /* CL_SetUserCmdValue */
	CG_SETUSERCMDAIMVALUES                 =  86,
	CG_SETUSERCMDINSHELLSHOCK              =  87,
	CG_R_REGISTERSHADERNOMIP               =  88,   /* Material_RegisterHandle */
	CG_MEMORYREMAINING                     =  89,
	CG_SYSCALL_0X5A                        =  90,   /* Key_IsDown */
	CG_SYSCALL_0X5B                        =  91,   /* cls.keyCatchers */
	CG_SYSCALL_0X5C                        =  92,   /* Key_SetCatcher */
	CG_SYSCALL_0X5D                        =  93,   /* Key_GetKey */
	CG_CL_LOOKUPCOLOR                      =  94,   /* the engine's case is PC_AddGlobalDefine; unresolved, see cg_syscalls_mp.c */
	CG_PC_LOADSOURCE                       =  95,
	CG_PC_FREESOURCE                       =  96,
	CG_PC_READTOKEN                        =  97,
	CG_PC_SOURCEFILEANDLINE                =  98,
	CG_REALTIME                            =  99,   /* Com_RealTime */
	CG_SNAPVECTOR                          = 100,   /* Sys_SnapVector */
	/* 101 (Cmd_RemoveCommand) has an engine case but no wrapper. */
	CG_CIN_PLAYCINEMATIC                   = 102,
	CG_CIN_STOPCINEMATIC                   = 103,
	CG_CIN_RUNCINEMATIC                    = 104,
	CG_CIN_DRAWCINEMATIC                   = 105,
	CG_CIN_SETEXTENTS                      = 106,
	CG_SYSCALL_0X6B                        = 107,   /* re slot +0xA0 */
	CG_R_TRACKSTATISTICS                   = 108,   /* re slot +0xA4 */
	/* 109-117, 120: the QVM memory/math block.  Engine cases, no wrappers. */
	CG_SYSCALL_0X76                        = 118,   /* Com_Printf( "%s%i\n", s, i ) */
	CG_SYSCALL_0X77                        = 119,   /* Com_Printf( "%s%f\n", s, f ) */
	CG_UI_LOADMENU                         = 121,
	CG_UI_POPUP                            = 122,
	CG_UI_CLOSEPOPUP                       = 123,
	CG_UI_CLOSEALLMENUS                    = 124,
	CG_UI_GETMAPDISPLAYNAME                = 125,
	CG_UI_GETGAMETYPEDISPLAYNAME           = 126,
	CG_CL_GETSERVERIPADDRESS               = 127,
	CG_XANIMPRECACHE                       = 128,   /* XAnimPrecacheAnimTree_m */
	CG_XANIMCREATEANIMS                    = 129,   /* XAnimAllocTree */
	CG_XANIMCREATE                         = 130,   /* XAnimLoadFileData_m */
	CG_XANIMCREATETREE                     = 131,   /* XAnimAllocRuntimeTree */
	CG_XANIMBLEND                          = 132,   /* XAnimSetParentNode */
	CG_SYSCALL_0X85                        = 133,   /* DObjFree */
	CG_XANIMCLEARGOALWEIGHT                = 134,   /* XAnimClearAnimNode_m */
	CG_XANIMCLEARTREEGOALWEIGHTS           = 135,   /* XAnimClearAnim_m */
	CG_XANIMCLEARTREEGOALWEIGHTSSTRICT     = 136,   /* XAnimClearAnimChildSubtrees_m */
	CG_SYSCALL_0X89                        = 137,   /* XAnimSetAnimKnob_m */
	CG_XANIMSETCOMPLETEGOALWEIGHTKNOBALL   = 138,   /* XAnimSetAnimKnobAll_m */
	CG_SYSCALL_0X8B                        = 139,   /* XAnimClearAnimChildren_m */
	CG_SYSCALL_0X8C                        = 140,   /* XAnimSetAnimInternalLimited_m */
	CG_XANIMSETCOMPLETEGOALWEIGHT          = 141,   /* XAnimSetAnimKnobInternal_m */
	CG_XANIMSETANIMRATE                    = 142,   /* XAnimSetAnimRate_m */
	CG_XANIMISLOOPED                       = 143,   /* XAnimIsLooped_m via scrAnimPub.trees */
	CG_SYSCALL_0X90                        = 144,   /* XAnimIsLooped_m */
	CG_XANIMSETTIME                        = 145,   /* XAnimSetTime_m */
	CG_XANIMGETTIME                        = 146,   /* XAnimGetTime */
	CG_XANIMGETWEIGHT                      = 147,
	CG_SYSCALL_0X94                        = 148,   /* CL_DObjInvalidateSkels */
	CG_SYSCALL_0X95                        = 149,   /* DObjUpdateClientInfo_m */
	CG_SYSCALL_0X96                        = 150,   /* xanim.numDeferredNotifies + the notify array */
	CG_SYSCALL_0X97                        = 151,   /* DObjCalcAnim */
	CG_SYSCALL_0X98                        = 152,   /* DObjDisplayAnim */
	CG_SYSCALL_0X99                        = 153,   /* XAnimGetRelDelta_m */
	CG_SYSCALL_0X9A                        = 154,   /* XAnimGetAbsDelta_m */
	CG_XANIMGETRELDELTA                    = 155,   /* XAnimGetRelDeltaForTime_m */
	CG_SYSCALL_0X9C                        = 156,   /* XAnimGetAbsDeltaForTime_m */
	CG_SYSCALL_0X9D                        = 157,   /* skel bone address out of the DObj */
	CG_SYSCALL_0X9E                        = 158,   /* skel bone address out of the DObj */
	CG_SYSCALL_0X9F                        = 159,   /* DObjMarkRotTransIndex */
	CG_SYSCALL_0XA0                        = 160,   /* DObjMarkControlRotTransIndex */
	CG_XANIMGETANIMNAME                    = 161,
	CG_SYSCALL_0XA2                        = 162,   /* Com_GetClientDObj */
	CG_SYSCALL_0XA3                        = 163,   /* DObjCreate */
	CG_SYSCALL_0XA4                        = 164,   /* Com_ClientDObjCreate */
	CG_SAFEDOBJFREE                        = 165,   /* Com_SafeClientDObjFree */
	CG_SYSCALL_0XA6                        = 166,   /* deref dword */
	CG_SYSCALL_0XA7                        = 167,   /* 96 * dobj[23] + 48 */
	CG_SYSCALL_0XA8                        = 168,   /* CL_DObjCreateSkelForBone */
	CG_SYSCALL_0XA9                        = 169,   /* CL_DObjCreateSkelForBones */
	CG_SYSCALL_0XAA                        = 170,   /* DObjGetHierarchyBits */
	CG_SYSCALL_0XAB                        = 171,   /* DObjCalcSkel */
	CG_SYSCALL_0XAC                        = 172,   /* XModelExists */
	CG_SYSCALL_0XAD                        = 173,   /* dobj part count */
	CG_SYSCALL_0XAE                        = 174,   /* DObjGetBoneIndex */
	CG_SYSCALL_0XAF                        = 175,   /* DObjGetBoneName */
	CG_SYSCALL_0XB0                        = 176,   /* DObjBuildPartCollisionTable */
	CG_SYSCALL_0XB1                        = 177,   /* deref dword */
	CG_XANIMISPRIMITIVE                    = 178,   /* anim tree child count == 0 */
	CG_XANIMGETLENGTH                      = 179,   /* anim length in msec */
	CG_SYSCALL_0XB4                        = 180,   /* XAnimHasTime */
	CG_SYSCALL_0XB5                        = 181,   /* anim tree child count */
	CG_SYSCALL_0XB6                        = 182,   /* anim tree child base */
	CG_XANIMGETANIMTREESIZE                = 183,   /* the DObj's tree */
	CG_SYSCALL_0XB8                        = 184,   /* XAnimCopyTree */
	CG_SYSCALL_0XB9                        = 185,   /* DObjDumpInfo, com_developer only */
	CG_SYSCALL_0XBA                        = 186,   /* StatMon_Warning */
	CG_SYSCALL_0XBB                        = 187,   /* StatMon_GetStatsArray */
	CG_Z_MALLOCINTERNAL                    = 188,
	CG_Z_FREEINTERNAL                      = 189,
	CG_COM_LOADSOUNDALIASES                = 190,
	CG_COM_SOUNDALIASSTRING                = 191,   /* Com_FindSoundAlias */
	CG_COM_PICKSOUNDALIAS                  = 192,
	CG_COM_GETSOUNDALIAS                   = 193,
	CG_MSS_PLAYSOUNDALIAS                  = 194,
	CG_MSS_PLAYBLENDEDSOUNDALIASES         = 195,
	CG_SURFACETYPEFROMNAME                 = 196,
	CG_SURFACETYPETONAME                   = 197,
	CG_ADD_DEBUG_LINE                      = 198,   /* CL_AddDebugLine; spelling from cl_cgame_mp.c */
	CG_GETWEAPONINFOMEMORY                 = 199,   /* Com_GetWeaponInfoMemory */
	CG_SYSCALL_0XC8                        = 200,   /* Com_FreeWeaponInfoMemory */
	CG_SYSCALL_0XC9                        = 201,   /* Hunk_AllocAlignInternal( size, 32 ) */
	CG_HUNKALLOCLOWINTERNAL                = 202,   /* Hunk_AllocLowAlignInternal( size, 32 ) */
	CG_SYSCALL_0XCB                        = 203,   /* Hunk_AllocAlignInternal */
	CG_HUNKALLOCLOWALIGNINTERNAL           = 204,   /* Hunk_AllocLowAlignInternal */
	CG_SYSCALL_0XCD                        = 205,   /* no-op in retail */
	CG_SYSCALL_0XCE                        = 206,   /* no-op in retail */
	CG_SYSCALL_0XCF                        = 207,   /* deref a global dword */
	CG_SYSCALL_0XD0                        = 208,   /* cgameClientLerpOrigin */
	CG_SYSCALL_0XD1                        = 209,   /* MSS_SetListener */
	CG_SYSCALL_0XD2                        = 210,   /* MSS_UpdateLoopingSounds */
	CG_SYSCALL_0XD3                        = 211,   /* MSS_StopSounds */
	CG_SYSCALL_0XD4                        = 212,   /* MSS_PlayMusicAlias */
	CG_SYSCALL_0XD5                        = 213,   /* MSS_StopBackground */
	CG_SYSCALL_0XD6                        = 214,   /* MSS_PlayAmbientAlias */
	CG_SYSCALL_0XD7                        = 215,   /* MSS_FadeAllSounds */
	CG_SYSCALL_0XD8                        = 216,   /* MSS_FadeSelectSounds */
	CG_SYSCALL_0XD9                        = 217,   /* MSS_SetEnvironmentEffects */
	CG_MSS_GETSOUNDOVERLAY                 = 218,
	CG_KEY_GETBINDINGBUF                   = 219,
	CG_KEY_SETBINDING                      = 220,
	CG_KEY_KEYNUMTOSTRINGBUF               = 221,
	CG_SYSCALL_0XDE                        = 222,   /* CFxScheduler::RegisterEffect */
	CG_SYSCALL_0XDF                        = 223,   /* FX_GetBoneIndex */
	CG_SYSCALL_0XE0                        = 224,   /* CFxScheduler::PlayEffect( name, simple ) */
	CG_SYSCALL_0XE1                        = 225,   /* CFxScheduler::PlayEffect( name ) */
	CG_SYSCALL_0XE2                        = 226,   /* CFxScheduler::PlayEffect( name, bolt ) */
	CG_SYSCALL_0XE3                        = 227,   /* CFxScheduler::PlayEffect( id, simple ) */
	CG_SYSCALL_0XE4                        = 228,   /* FX_PlayEffectID */
	CG_SYSCALL_0XE5                        = 229,   /* CFxScheduler::PlayEffect( id, axis ) */
	CG_SYSCALL_0XE6                        = 230,   /* CFxScheduler::AddScheduledEffects */
	CG_SYSCALL_0XE7                        = 231,   /* FX_Init */
	CG_SYSCALL_0XE8                        = 232,   /* FX_Free( 1 ) */
	CG_SYSCALL_0XE9                        = 233,   /* FX_Free( 0 ) */
	CG_SYSCALL_0XEA                        = 234,   /* SFxHelper::AdjustTime */
	CG_SYSCALL_0XEB                        = 235,   /* SFxHelper::AdjustCamera */
	CG_SYSCALL_0XEC                        = 236,   /* no engine case -- dead */
	CG_SYSCALL_0XED                        = 237,   /* no engine case -- dead */
	CG_SYSCALL_0XEE                        = 238,   /* no engine case -- dead */
	CG_SYSCALL_0XEF                        = 239,   /* no engine case -- dead */
	CG_SYSCALL_0XF0                        = 240,   /* two cgame-owned engine dwords */
	CG_SYSCALL_0XF1                        = 241    /* CL_FirstSnapshot */
} cgameImport_t;

/*
=============================================================================

	THE ENTRY POINTS

	Numbering is vmMain's own switch (0x300203B0).  Only the members whose
	spelling the engine side already uses on a VM_Call site are named:
	client_mp/cl_vm.h has 1 and 5-8, cl_cgame_mp.c has 0 and 16,
	cl_net_chan_mp.c has 15, and 2, 3 and 4 keep RTCW's spelling because
	vmMain dispatches them to CG_ConsoleCommand, CG_DrawActiveFrame and
	CG_CrosshairPlayer.

	vmMain also answers 9 through 14, 17 and 18.  They have no name on the
	engine side, so they are left out rather than invented:

		 9  returns an element of the cgame's game-model table
		    (CG_GetGameModel, retail 0x004118B0)
		10  CG_DObjCalcPose            (retail 0x004118D0)
		11  CG_DObjCalcBoneGeneric
		12  CG_GetDObjOrientation
		13  CG_GetEntityOrientation
		14  CG_ImpactMark
		17  CL_SaveCgameState's tail    (retail returns 0)
		18  CL_RestoreCgameState's tail (retail returns 0)

	5 and 8 exist only as `return 0` in retail: vmMain shares one exit for
	both.  The engine still calls them.

=============================================================================
*/

typedef enum
{
	CG_INIT                 = 0,
	CG_SHUTDOWN             = 1,
	CG_CONSOLE_COMMAND      = 2,
	CG_DRAW_ACTIVE_FRAME    = 3,
	CG_CROSSHAIR_PLAYER     = 4,
	CG_KEY_EVENT            = 5,
	CG_CHECK_EXEC_KEY       = 6,    /* CG_KeyInterceptEvent */
	CG_MOUSE_EVENT          = 7,
	CG_EVENT_HANDLING       = 8,
	CG_PROFILE_DRAW_TRAP    = 15,   /* CG_DrawStringExt */
	CG_SCRIPT_HOOK          = 16
} cgameExport_t;

#endif  /* __CG_PUBLIC_H__ */
