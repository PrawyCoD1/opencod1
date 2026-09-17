/*
 * cg_main_mp.c -- initialization and primary entry point for the multiplayer
 * client game module (cgame_mp_x86.dll, CoD 1.1, imagebase 0x30000000).
 *
 * Descended from RTCW's cgame/cg_main.c: vmMain, the cvar table, the media
 * registration, the ui_shared display-context wiring and the menu loader are
 * all recognisably RTCW's, with CoD's additions (sound aliases, XAnim, the
 * shell-shock configstrings, the string-localization traps) grafted on.
 *
 * Function order is the binary's (0x300203B0 .. 0x30023434), which is the order
 * they appeared in the original file.
 *
 * @fidelity: likely
 */

#include "cg_local.h"
/* cgDC lives here; ui_shared.h owns displayContextDef_t, cachedAssets_t and
   fontInfo_t. */
#include "../ui_mp/ui_shared.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ---------------------------------------------------------------------------
 * Declarations for code this unit calls that no header carries.  Where a
 * spelling is inferred rather than recovered the comment says so.
 * ---------------------------------------------------------------------------
 */

/* universal/q_shared.c and universal/q_parse.c. */

/* universal/com_math.c (0x3003C770 / 0x3003C730 in this module). */

/* cg_animtree.c */
void        CGScr_LoadAnimTrees( void );
void        CG_FreeClientDObjInfo( void );
void        CG_FreeEntityDObjInfo( void );

/* cg_consolecmds.c */
int         CG_ConsoleCommand( void );
void        CG_InitConsoleCommands( void );

/* cg_effects.c */
void        CG_RegisterImpactEffects( void );

/* cg_localents.c */
void        CG_InitLocalEntities( void );

/* cg_marks.c.  Twelve parameters (vmMain case 14 pushes eleven and leaves the
   twelfth in ecx); several are floats reaching cgame as bit patterns, so they
   are spelled int here. */
void        CG_InitMarkPolys( void );

/* cg_scoreboard.c */
void        CG_RegisterScoreboardGraphics( void );

/* cg_servercmds.c */
void        CG_ParseServerinfo( void );
void        CG_ParseWolfinfo( void );
void        CG_SetConfigValues( void );

/* cg_shellshock.c */
qboolean    CG_LoadShellShockCvars( const char *name );
void        CG_SetShellShockParmsFromCvars( shellshockParms_t *parms );

/* cg_view.c */
void        CG_DrawActiveFrame( int serverTime, int arg1, int demoPlayback, int arg3, int arg4 );

/* game_mp/bg_weapon.c */
void        BG_SetupWeaponInfo( void );

/* cg_weapons.c */
void        CG_RegisterItems( void );
void        CG_FreeWeapons( void );

/* 0x30020260, 0x30020320, 0x30020390 and 0x300203A0 sit between
   cg_localents.c's last function and vmMain; they are this file's by address
   and are written below. */
void        CG_GetDObjOrientation( int entnum, vec3_t origin, vec3_t axis[3] );
void        CG_GetEntityOrientation( int entnum, vec3_t origin, vec3_t axis[3] );
int         CG_SaveState( void *buffer, int size );
int         CG_RestoreState( void *buffer, int size );

/* 0x3003FDE0, in the ui_shared.c address range but named Scr_FarHook; vmMain
   export 16 is a straight tail call to it. */
int         *Scr_FarHook( const void *hooks );

/* ui_shared.c */
extern int  menuCount;                                  /* 0x300EEEE8 */
void        String_Init( void );
void        Menu_Reset( void );
void        Menu_New( int handle, int loadMode );
qboolean    PC_String_Parse( int handle, const char **out );
qboolean    PC_Int_Parse( int handle, int *out );
qboolean    PC_Float_Parse( int handle, float *out );
qboolean    PC_Color_Parse( int handle, vec4_t *out );
void        Controls_GetConfig( void );

/* bg_animation.c, compiled into this DLL as well as into the game DLL.  Same
   objects, same names, same one-rep-stosd clear that G_InitGame does -- see
   game_mp/g_main_mp.c, which carries the identical block for 0x200A1C80. */
extern int  bgs_time;                                   /* 0x300F0310 */
extern int  bgs_animTime;                               /* 0x300F0314 */
extern int  bgs_frametime;                              /* 0x300F0318 */
extern animScriptData_t bgs_animScriptData;             /* 0x300F0520 */
extern void *bgs_animTree;                              /* 0x3018BBF8 */
extern scr_anim_t bgs_rootAnim;                         /* 0x3018BBFC */
extern scr_anim_t bgs_torsoAnim;                        /* 0x3018BC00 */
extern scr_anim_t bgs_legsAnim;                         /* 0x3018BC04 */
extern scr_anim_t bgs_turningAnim;                      /* 0x3018BC08 */

/* The two per-team voice-chat lists are cg_servercmds.c's voiceChatLists[0]
   and [1] (0x302F7D80 / 0x30340EC8, the same CG_LoadVoiceChats reads); the
   type and the extern are in cg_local.h. */


/*
=============================================================================

		GLOBALS

	RTCW's cg_main.c owns these five objects; their sizes are CG_Init's four
	rep stosd (0x30022F9F..0x30022FCF).

=============================================================================
*/

cg_t cg;                                                /* 0x301E2140 */
cgs_t cgs;                                              /* 0x301CC0A0 */
centity_t cg_entities[MAX_GENTITIES];                   /* 0x3020DB80 */
cgWeaponInfo_t cg_weapons[MAX_WEAPONS_CG];              /* 0x301A6940 */

/* cg_weapons_mp.c's itemInfo_t: 256 records of 36 bytes (CG_RegisterItemVisuals
   0x30036082 indexes with `lea ebp,[eax+eax*8]`, and CG_Init clears 0x2400).
   RTCW defines cg_items in cg_main.c, so it is defined here. */
byte cg_items[256 * 36];                                /* 0x301DC3E0 */

displayContextDef_t cgDC;                               /* 0x301AD720 */

/*
=============================================================================

		THE CVAR TABLE

	173 entries at 0x300749A0, walked by CG_RegisterCvars (0x300205E0) and
	CG_UpdateCvars (0x300206A0) with a 16-byte stride.  The vmCvar_t handles
	are declared in cg_local.h with their addresses.

=============================================================================
*/

vmCvar_t cg_ignore;
vmCvar_t cg_drawGun;
vmCvar_t cg_cursorHints;
vmCvar_t cg_hintFadeTime;
vmCvar_t cg_fov;
vmCvar_t cg_viewsize;
vmCvar_t cg_letterbox;
vmCvar_t cg_stereoSeparation;
vmCvar_t cg_shadows;
vmCvar_t cg_draw2D;
vmCvar_t cg_drawStatus;
vmCvar_t cg_drawFPS;
vmCvar_t cg_drawSoundOverlay;
vmCvar_t cg_drawScriptUsage;
vmCvar_t cg_drawShader;
vmCvar_t cg_drawSnapshot;
vmCvar_t cg_drawCrosshair;
vmCvar_t cg_drawCrosshairNames;
vmCvar_t cg_drawCrosshairPickups;
vmCvar_t cg_drawRewards;
vmCvar_t cg_hudAlpha;
vmCvar_t cg_hudCompassSize;
vmCvar_t cg_hudCompassMaxRange;
vmCvar_t cg_hudCompassMinRange;
vmCvar_t cg_hudCompassMinRadius;
vmCvar_t cg_hudCompassSpringyPointers;
vmCvar_t cg_hudObjectiveMinHeight;
vmCvar_t cg_hudObjectiveMaxHeight;
vmCvar_t cg_hudObjectiveMaxRange;
vmCvar_t cg_hudObjectiveMinAlpha;
vmCvar_t cg_hudStanceFlash_r;
vmCvar_t cg_hudStanceFlash_g;
vmCvar_t cg_hudStanceFlash_b;
vmCvar_t cg_hudStanceHintPrints;
vmCvar_t cg_hudDamageIconWidth;
vmCvar_t cg_hudDamageIconHeight;
vmCvar_t cg_hudDamageIconOffset;
vmCvar_t cg_hudDamageIconTime;
vmCvar_t cg_hudDamageIconInScope;
vmCvar_t cg_weaponCycleDelay;
vmCvar_t cg_weaponSelect;
vmCvar_t cg_crosshairAlpha;
vmCvar_t cg_crosshairAlphaMin;
vmCvar_t cg_crosshairDynamic;
vmCvar_t cg_crosshairNoGun;
vmCvar_t cg_brass;
vmCvar_t cg_marks;
vmCvar_t cg_lagometer;
vmCvar_t cg_railTrailTime;
vmCvar_t cg_gunX;
vmCvar_t cg_gunY;
vmCvar_t cg_gunZ;
vmCvar_t cg_gun_move_f;
vmCvar_t cg_gun_move_r;
vmCvar_t cg_gun_move_u;
vmCvar_t cg_gun_ofs_f;
vmCvar_t cg_gun_ofs_r;
vmCvar_t cg_gun_ofs_u;
vmCvar_t cg_gun_move_rate;
vmCvar_t cg_gun_move_minspeed;
vmCvar_t cg_centertime;
vmCvar_t cg_bobAmplitudeStanding;
vmCvar_t cg_bobAmplitudeDucked;
vmCvar_t cg_bobAmplitudeProne;
vmCvar_t cg_bobMax;
vmCvar_t cg_skybox;
vmCvar_t cg_debugProneCheck;
vmCvar_t cg_debugProneCheckDepthCheck;
vmCvar_t cg_debugposition;
vmCvar_t cg_debugevents;
vmCvar_t cg_errordecay;
vmCvar_t cg_nopredict;
vmCvar_t cg_noplayeranims;
vmCvar_t cg_showmiss;
vmCvar_t cg_footsteps;
vmCvar_t cg_tracerchance;
vmCvar_t cg_tracerwidth;
vmCvar_t cg_tracerSpeed;
vmCvar_t cg_tracerlength;
vmCvar_t cg_thirdPersonRange;
vmCvar_t cg_thirdPersonAngle;
vmCvar_t cg_thirdPerson;
vmCvar_t cg_chatTime;
vmCvar_t cg_chatHeight;
vmCvar_t cg_predictItems;
vmCvar_t cg_drawTeamOverlay;
vmCvar_t cg_stats;
vmCvar_t cg_timescale;
vmCvar_t pmove_fixed;
vmCvar_t pmove_msec;
vmCvar_t cg_hudFiles;
vmCvar_t cl_stance;
vmCvar_t cl_stanceTemp;
vmCvar_t cg_noTaunt;
vmCvar_t cg_voiceSpriteTime;
vmCvar_t cg_teamChatsOnly;
vmCvar_t cg_noVoiceChats;
vmCvar_t cg_noVoiceText;
vmCvar_t cl_paused;
vmCvar_t g_synchronousClients;
vmCvar_t cg_currentSelectedPlayer;
vmCvar_t cg_currentSelectedPlayerName;
vmCvar_t cg_deadbodyque;
vmCvar_t cg_gametype;
vmCvar_t cg_norender;
vmCvar_t cg_animState;
vmCvar_t cl_waitForFire;
vmCvar_t cg_dumpAnims;
vmCvar_t cg_developer;
vmCvar_t con_minicon;
vmCvar_t cg_version;
vmCvar_t cg_subtitles;
vmCvar_t cg_subtitleMinTime;
vmCvar_t cg_subtitleWidth;
vmCvar_t cg_gameMessageWidth;
vmCvar_t cg_gameBoldMessageWidth;
vmCvar_t cl_languagewarnings;
vmCvar_t cl_languagewarningsaserrors;
vmCvar_t cg_objectiveText;
vmCvar_t cg_scoreboardScrollStep;
vmCvar_t cg_descriptiveText;
vmCvar_t cg_r_optimize;
vmCvar_t cg_r_optimizeXModels;
vmCvar_t bg_viewheight_standing;
vmCvar_t bg_viewheight_crouched;
vmCvar_t bg_viewheight_prone;
vmCvar_t bg_duck2prone_time;
vmCvar_t bg_prone2duck_time;
vmCvar_t bg_ladder_yawcap;
vmCvar_t bg_prone_yawcap;
vmCvar_t bg_prone_softyawedge;
vmCvar_t bg_foliagesnd_minspeed;
vmCvar_t bg_foliagesnd_maxspeed;
vmCvar_t bg_foliagesnd_slowinterval;
vmCvar_t bg_foliagesnd_fastinterval;
vmCvar_t bg_foliagesnd_resetinterval;
vmCvar_t bg_fallDamageMinHeight;
vmCvar_t bg_fallDamageMaxHeight;
vmCvar_t bg_debugWeaponAnim;
vmCvar_t bg_debugWeaponState;
vmCvar_t cg_shock_screenBlendTime;
vmCvar_t cg_shock_screenBlendFadeTime;
vmCvar_t cg_shock_viewKickPeriod;
vmCvar_t cg_shock_viewKickRadius;
vmCvar_t cg_shock_sound;
vmCvar_t cg_shock_soundFadeInTime;
vmCvar_t cg_shock_soundFadeOutTime;
vmCvar_t cg_shock_soundLoopFadeTime;
vmCvar_t cg_shock_soundLoopEndDelay;
vmCvar_t cg_shock_soundRoomType;
vmCvar_t cg_shock_soundWetLevel;
vmCvar_t cg_shock_soundModEndDelay;
vmCvar_t cg_shock_volume_auto;
vmCvar_t cg_shock_volume_menu;
vmCvar_t cg_shock_volume_weapon;
vmCvar_t cg_shock_volume_voice;
vmCvar_t cg_shock_volume_item;
vmCvar_t cg_shock_volume_body;
vmCvar_t cg_shock_volume_local;
vmCvar_t cg_shock_volume_music;
vmCvar_t cg_shock_volume_announcer;
vmCvar_t cg_shock_volume_shellshock;
vmCvar_t cg_shock_mouse;
vmCvar_t cg_shock_mouse_maxpitchspeed;
vmCvar_t cg_shock_mouse_maxyawspeed;
vmCvar_t cg_shock_mouse_sensitivityscale;
vmCvar_t cg_shock_mouse_fadeTime;
vmCvar_t cg_debuganim;
vmCvar_t bg_swingSpeed;
vmCvar_t cg_blood;
vmCvar_t cl_serverloadmap;
vmCvar_t cl_serverloadgametype;
vmCvar_t cl_serverloadwaiting;

typedef struct cvarTable_t {
	vmCvar_t    *vmCvar;
	char        *cvarName;
	char        *defaultString;
	int cvarFlags;
} cvarTable_t;

/* NOT static: cg_view_mp.c's CG_DrawActiveFrame walks this table every frame
   (0x30033A2A). */
cvarTable_t cvarTable[] = {                  /* 0x300749A0 */
	{ &cg_ignore, "cg_ignore", "0", 0 },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE | CVAR_CHEAT },
	{ &cg_cursorHints, "cg_cursorHints", "3", CVAR_ARCHIVE },
	{ &cg_hintFadeTime, "cg_hintFadeTime", "100", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "80", CVAR_ARCHIVE | CVAR_CHEAT },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_letterbox, "cg_letterbox", "0", CVAR_TEMP },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE },
	{ &cg_shadows, "cg_shadows", "0", CVAR_ARCHIVE | CVAR_CHEAT },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_CHEAT },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE },
	{ &cg_drawSoundOverlay, "cg_drawSoundOverlay", "0", 0 },
	{ &cg_drawScriptUsage, "cg_drawScriptUsage", "0", 0 },
	{ &cg_drawShader, "cg_drawShader", "0", CVAR_CHEAT },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "1", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawCrosshairPickups, "cg_drawCrosshairPickups", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_hudAlpha, "cg_hudAlpha", "1.0", CVAR_ARCHIVE },
	{ &cg_hudCompassSize, "cg_hudCompassSize", "1.0", CVAR_ARCHIVE },
	{ &cg_hudCompassMaxRange, "cg_hudCompassMaxRange", "1024", CVAR_ARCHIVE },
	{ &cg_hudCompassMinRange, "cg_hudCompassMinRange", "0", CVAR_ARCHIVE },
	{ &cg_hudCompassMinRadius, "cg_hudCompassMinRadius", "0", CVAR_ARCHIVE },
	{ &cg_hudCompassSpringyPointers, "cg_hudCompassSpringyPointers", "0", CVAR_ARCHIVE },
	{ &cg_hudObjectiveMinHeight, "cg_hudObjectiveMinHeight", "-70", CVAR_ARCHIVE },
	{ &cg_hudObjectiveMaxHeight, "cg_hudObjectiveMaxHeight", "70", CVAR_ARCHIVE },
	{ &cg_hudObjectiveMaxRange, "cg_hudObjectiveMaxRange", "2048", CVAR_ARCHIVE },
	{ &cg_hudObjectiveMinAlpha, "cg_hudObjectiveMinAlpha", "1", CVAR_ARCHIVE },
	{ &cg_hudStanceFlash_r, "cg_hudStanceFlash_r", "1.0", 0 },
	{ &cg_hudStanceFlash_g, "cg_hudStanceFlash_g", "1.0", 0 },
	{ &cg_hudStanceFlash_b, "cg_hudStanceFlash_b", "0.3", 0 },
	{ &cg_hudStanceHintPrints, "cg_hudStanceHintPrints", "0", CVAR_ARCHIVE },
	{ &cg_hudDamageIconWidth, "cg_hudDamageIconWidth", "128", CVAR_ARCHIVE },
	{ &cg_hudDamageIconHeight, "cg_hudDamageIconHeight", "64", CVAR_ARCHIVE },
	{ &cg_hudDamageIconOffset, "cg_hudDamageIconOffset", "32", CVAR_ARCHIVE },
	{ &cg_hudDamageIconTime, "cg_hudDamageIconTime", "2000", CVAR_ARCHIVE },
	{ &cg_hudDamageIconInScope, "cg_hudDamageIconInScope", "0", CVAR_ARCHIVE },
	{ &cg_weaponCycleDelay, "cg_weaponCycleDelay", "0", CVAR_ARCHIVE },
	{ &cg_weaponSelect, "cg_weaponSelect", "0", CVAR_ROM },
	{ &cg_crosshairAlpha, "cg_crosshairAlpha", "1.0", CVAR_ARCHIVE },
	{ &cg_crosshairAlphaMin, "cg_crosshairAlphaMin", "0.7", CVAR_ARCHIVE },
	{ &cg_crosshairDynamic, "cg_crosshairDynamic", "0", CVAR_ARCHIVE },
	{ &cg_crosshairNoGun, "cg_crosshairNoGun", "gfx/reticle/hud@center_ads.tga", CVAR_ARCHIVE },
	{ &cg_brass, "cg_brass", "1", CVAR_ARCHIVE },
	{ &cg_marks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "0", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE },
	{ &cg_gunX, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gunY, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gunZ, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_gun_move_f, "cg_gun_move_f", "0", CVAR_CHEAT },
	{ &cg_gun_move_r, "cg_gun_move_r", "0", CVAR_CHEAT },
	{ &cg_gun_move_u, "cg_gun_move_u", "0", CVAR_CHEAT },
	{ &cg_gun_ofs_f, "cg_gun_ofs_f", "0", CVAR_CHEAT },
	{ &cg_gun_ofs_r, "cg_gun_ofs_r", "0", CVAR_CHEAT },
	{ &cg_gun_ofs_u, "cg_gun_ofs_u", "0", CVAR_CHEAT },
	{ &cg_gun_move_rate, "cg_gun_move_rate", "0", CVAR_CHEAT },
	{ &cg_gun_move_minspeed, "cg_gun_move_minspeed", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "5", CVAR_CHEAT },
	{ &cg_bobAmplitudeStanding, "cg_bobAmplitudeStanding", "0.007", CVAR_CHEAT },
	{ &cg_bobAmplitudeDucked, "cg_bobAmplitudeDucked", "0.0075", CVAR_CHEAT },
	{ &cg_bobAmplitudeProne, "cg_bobAmplitudeProne", "0.03", CVAR_CHEAT },
	{ &cg_bobMax, "cg_bobMax", "8", CVAR_CHEAT },
	{ &cg_skybox, "cg_skybox", "1", CVAR_CHEAT },
	{ &cg_debugProneCheck, "cg_debugProneCheck", "0", CVAR_CHEAT },
	{ &cg_debugProneCheckDepthCheck, "cg_debugProneCheckDepthCheck", "1", CVAR_CHEAT },
	{ &cg_debugposition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugevents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errordecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_noplayeranims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
	{ &cg_tracerchance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerwidth, "cg_tracerwidth", "0.8", CVAR_CHEAT },
	{ &cg_tracerSpeed, "cg_tracerSpeed", "4500", CVAR_CHEAT },
	{ &cg_tracerlength, "cg_tracerlength", "160", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "120", CVAR_CHEAT },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_CHEAT },
	{ &cg_chatTime, "cg_chatTime", "12000", CVAR_ARCHIVE },
	{ &cg_chatHeight, "cg_chatHeight", "8", CVAR_ARCHIVE },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "2", CVAR_ARCHIVE },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_timescale, "timescale", "1", 0 },
	{ &pmove_fixed, "pmove_fixed", "0", 0 },
	{ &pmove_msec, "pmove_msec", "8", 0 },
	{ &cg_hudFiles, "cg_hudFiles", "ui_mp/hud.txt", 0 },
	{ &cl_stance, "cl_stance", "0", CVAR_TEMP },
	{ &cl_stanceTemp, "cl_stanceTemp", "0", CVAR_TEMP },
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE },
	{ &cg_voiceSpriteTime, "cg_voiceSpriteTime", "6000", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
	{ &cl_paused, "cl_paused", "0", CVAR_ROM },
	{ &g_synchronousClients, "g_synchronousClients", "0", 0 },
	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE },
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE },
	{ &cg_deadbodyque, "cg_deadbodyque", "32", CVAR_ARCHIVE },
	{ &cg_gametype, "g_gametype", "dm", 0 },
	{ &cg_norender, "cg_norender", "0", 0 },
	{ &cg_animState, "cg_animState", "0", CVAR_CHEAT },
	{ &cl_waitForFire, "cl_waitForFire", "0", CVAR_ROM },
	{ &cg_dumpAnims, "cg_dumpAnims", "-1", CVAR_CHEAT },
	{ &cg_developer, "developer", "0", CVAR_TEMP },
	{ &con_minicon, "con_minicon", "0", CVAR_ARCHIVE },
	{ &cg_version, "version", "0", CVAR_TEMP },
	{ &cg_subtitles, "cg_subtitles", "1", CVAR_ARCHIVE },
	{ &cg_subtitleMinTime, "cg_subtitleMinTime", "3", CVAR_ARCHIVE },
	{ &cg_subtitleWidth, "cg_subtitleWidth", "80", CVAR_ARCHIVE },
	{ &cg_gameMessageWidth, "cg_gameMessageWidth", "70", CVAR_ARCHIVE },
	{ &cg_gameBoldMessageWidth, "cg_gameBoldMessageWidth", "60", CVAR_ARCHIVE },
	{ &cl_languagewarnings, "cl_languagewarnings", "0", 0 },
	{ &cl_languagewarningsaserrors, "cl_languagewarningsaserrors", "0", 0 },
	{ &cg_objectiveText, "cg_objectiveText", "", 0 },
	{ &cg_scoreboardScrollStep, "cg_scoreboardScrollStep", "3", CVAR_ARCHIVE },
	{ &cg_descriptiveText, "cg_descriptiveText", "1", CVAR_ARCHIVE },
	{ &cg_r_optimize, "r_optimize", "1", CVAR_LATCH },
	{ &cg_r_optimizeXModels, "r_optimizeXModels", "1", CVAR_LATCH },
	{ &bg_viewheight_standing, "bg_viewheight_standing", "60", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_viewheight_crouched, "bg_viewheight_crouched", "40", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_viewheight_prone, "bg_viewheight_prone", "11", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_duck2prone_time, "bg_duck2prone_time", "400", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_prone2duck_time, "bg_prone2duck_time", "400", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_ladder_yawcap, "bg_ladder_yawcap", "100", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_prone_yawcap, "bg_prone_yawcap", "85", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_prone_softyawedge, "bg_prone_softyawedge", "1", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_foliagesnd_minspeed, "bg_foliagesnd_minspeed", "40", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_foliagesnd_maxspeed, "bg_foliagesnd_maxspeed", "180", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_foliagesnd_slowinterval, "bg_foliagesnd_slowinterval", "1500", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_foliagesnd_fastinterval, "bg_foliagesnd_fastinterval", "500", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_foliagesnd_resetinterval, "bg_foliagesnd_resetinterval", "500", CVAR_CHEAT | CVAR_NORESTART },
	{ &bg_fallDamageMinHeight, "bg_fallDamageMinHeight", "256", CVAR_SYSTEMINFO | CVAR_CHEAT },
	{ &bg_fallDamageMaxHeight, "bg_fallDamageMaxHeight", "480", CVAR_SYSTEMINFO | CVAR_CHEAT },
	{ &bg_debugWeaponAnim, "bg_debugWeaponAnim", "0", CVAR_CHEAT },
	{ &bg_debugWeaponState, "bg_debugWeaponState", "0", CVAR_CHEAT },
	{ &cg_shock_screenBlendTime, "cg_shock_screenBlendTime", "0.4", CVAR_CHEAT },
	{ &cg_shock_screenBlendFadeTime, "cg_shock_screenBlendFadeTime", "1", CVAR_CHEAT },
	{ &cg_shock_viewKickPeriod, "cg_shock_viewKickPeriod", ".75", CVAR_CHEAT },
	{ &cg_shock_viewKickRadius, "cg_shock_viewKickRadius", ".05", CVAR_CHEAT },
	{ &cg_shock_sound, "cg_shock_sound", "1", CVAR_CHEAT },
	{ &cg_shock_soundFadeInTime, "cg_shock_soundFadeInTime", "0.25", CVAR_CHEAT },
	{ &cg_shock_soundFadeOutTime, "cg_shock_soundFadeOutTime", "2.5", CVAR_CHEAT },
	{ &cg_shock_soundLoopFadeTime, "cg_shock_soundLoopFadeTime", "1.5", CVAR_CHEAT },
	{ &cg_shock_soundLoopEndDelay, "cg_shock_soundLoopEndDelay", "-3.0", CVAR_CHEAT },
	{ &cg_shock_soundRoomType, "cg_shock_soundRoomType", "underwater", CVAR_CHEAT },
	{ &cg_shock_soundWetLevel, "cg_shock_soundWetLevel", "0.5", CVAR_CHEAT },
	{ &cg_shock_soundModEndDelay, "cg_shock_soundModEndDelay", "2", CVAR_CHEAT },
	{ &cg_shock_volume_auto, "cg_shock_volume_auto", "0.1", CVAR_CHEAT },
	{ &cg_shock_volume_menu, "cg_shock_volume_menu", "1.0", CVAR_CHEAT },
	{ &cg_shock_volume_weapon, "cg_shock_volume_weapon", "0.5", CVAR_CHEAT },
	{ &cg_shock_volume_voice, "cg_shock_volume_voice", "0.2", CVAR_CHEAT },
	{ &cg_shock_volume_item, "cg_shock_volume_item", "0.1", CVAR_CHEAT },
	{ &cg_shock_volume_body, "cg_shock_volume_body", "0.1", CVAR_CHEAT },
	{ &cg_shock_volume_local, "cg_shock_volume_local", "1.0", CVAR_CHEAT },
	{ &cg_shock_volume_music, "cg_shock_volume_music", "1.0", CVAR_CHEAT },
	{ &cg_shock_volume_announcer, "cg_shock_volume_announcer", "1.0", CVAR_CHEAT },
	{ &cg_shock_volume_shellshock, "cg_shock_volume_shellshock", "1.0", CVAR_CHEAT },
	{ &cg_shock_mouse, "cg_shock_mouse", "1", CVAR_CHEAT },
	{ &cg_shock_mouse_maxpitchspeed, "cg_shock_mouse_maxpitchspeed", "90", CVAR_CHEAT },
	{ &cg_shock_mouse_maxyawspeed, "cg_shock_mouse_maxyawspeed", "90", CVAR_CHEAT },
	{ &cg_shock_mouse_sensitivityscale, "cg_shock_mouse_sensitivityscale", "0.5", CVAR_CHEAT },
	{ &cg_shock_mouse_fadeTime, "cg_shock_mouse_fadeTime", "2", CVAR_CHEAT },
	{ &cg_debuganim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &bg_swingSpeed, "bg_swingSpeed", "0.2", CVAR_CHEAT },
	{ &cg_blood, "cg_blood", "1", CVAR_ARCHIVE },
	{ &cl_serverloadmap, "cl_serverloadmap", "", CVAR_ROM },
	{ &cl_serverloadgametype, "cl_serverloadgametype", "", CVAR_ROM },
	{ &cl_serverloadwaiting, "cl_serverloadwaiting", "0", CVAR_ROM }
};

/* retail 0x30020600 counts down from a folded 0xad -- no cvarTableSize global */
#define cvarTableSize   ( sizeof( cvarTable ) / sizeof( cvarTable[0] ) )

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );

/*
 * cg.unknown_0x2AA40[96] is the view-model placement block; cg_weapons_mp.c
 * names the whole of it.  CG_AddPlayerWeapon writes these two (0x30037014 /
 * 0x30037031) and CG_GetDObjOrientation is the only other reader.
 */
#define CG_VIEWMODELORIGIN()    ( (float *)&cg.unknown_0x2AA40[0x2C] )   /* cg+0x2AA6C */
#define CG_VIEWMODELAXIS()      ( (vec3_t *)&cg.unknown_0x2AA40[0x38] )  /* cg+0x2AA78 */

/*
================
CG_GetDObjOrientation         0x30020260

vmMain export 12.  Retail is __usercall( entnum@<ecx>, origin@<edx>,
axis@<eax> ) and tail-jumps into AnglesToAxis, which is why it has no epilogue
of its own.

DObj handles 0..MAX_GENTITIES-1 are entities; MAX_GENTITIES..+MAX_CLIENTS are
the view-model DObjs, and every one of those answers with the single view-model
placement -- the index is bounds-checked and then discarded (0x300202A7).
================
*/
void CG_GetDObjOrientation( int entnum, vec3_t origin, vec3_t axis[3] ) {
	if ( entnum >= 0 && entnum < MAX_GENTITIES ) {
		VectorCopy( cg_entities[entnum].lerpOrigin, origin );
		AnglesToAxis( cg_entities[entnum].lerpAngles, axis );
	} else if ( entnum >= MAX_GENTITIES && entnum - MAX_GENTITIES < MAX_CLIENTS ) {
		VectorCopy( CG_VIEWMODELORIGIN(), origin );
		AxisCopy( CG_VIEWMODELAXIS(), axis );
	}
}

/*
================
CG_GetEntityOrientation       0x30020320

vmMain export 13.  __usercall( entnum@<eax>, axis@<edx>, origin@<ecx> ), and
unlike export 12 it bounds-checks nothing.  AnglesToAxis is inlined here rather
than called: the AngleVectors at 0x30020355 takes axis[0] as `forward`, a stack
temporary as `right` and axis[2] as `up`, and 0x3002035B..0x3002037F is the
`VectorSubtract( vec3_origin, right, axis[1] )` tail -- flt_300608E8..F0 are the
three zeroes of vec3_origin.
================
*/
void CG_GetEntityOrientation( int entnum, vec3_t origin, vec3_t axis[3] ) {
	VectorCopy( cg_entities[entnum].lerpOrigin, origin );
	AnglesToAxis( cg_entities[entnum].lerpAngles, axis );
}

/*
================
CG_SaveState                  0x30020390
CG_RestoreState               0x300203A0

The cgame halves of CL_SaveCgameState / CL_RestoreCgameState
(client_mp/cl_cgame_mp.c 0x00404F80 / 0x00404FB0): the engine appends
VM_Call( cgvm, 17/18, buffer + used, size - used ) to the export table it wrote
itself and adds the returned byte count.  Retail's cgame stores nothing, so
both are `xor eax, eax; retn` and neither touches its arguments -- the parameter
list below comes from the engine call site, not from these bodies.

Nothing in the DLL references either symbol: vmMain inlined both, which is why
cases 17 and 18 share one `xor esi, esi` at 0x3002056E.  The out-of-line copies
survived anyway.
================
*/
int CG_SaveState( void *buffer, int size ) {
	return 0;
}

int CG_RestoreState( void *buffer, int size ) {
	return 0;
}

/*
================
vmMain

This is the only way control passes into the module.
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
			int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 ) {
	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1, arg2 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2, arg3, arg4 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_KEY_EVENT:
		return 0;
	case CG_CHECK_EXEC_KEY:
		return CG_KeyInterceptEvent( arg0, arg1 );
	case CG_MOUSE_EVENT:
		cgDC.cursorx = cgs.cursorX;
		cgDC.cursory = cgs.cursorY;
		return 0;
	case CG_EVENT_HANDLING:
		return 0;
	case 9:
		return cgs.gameModels[arg0];
	case 10:
		/* ( owner, dobj, partBits ): 0x300204C0 pushes arg0 and leaves arg1 in
		   edi / arg2 in eax, and the callee's three traps take edi as the DObj
		   and eax as the bitset.  The renderer agrees -- tr_xanim.c passes
		   refEntity+0x94, which is RE_OWNER. */
		CG_DObjCalcPose( (centity_t *)arg0, arg1, (int *)arg2 );
		return 0;
	case 11:
		CG_DObjCalcBoneGeneric( arg0, arg1 );
		return 0;
	case 12:
		CG_GetDObjOrientation( arg0, (float *)arg1, (vec3_t *)arg2 );
		return 0;
	case 13:
		/* arg1 reaches the callee in ecx and arg2 in edx (0x30020506), which
		   is the same (entnum, origin, axis) order case 12 uses -- the two
		   differ only in which register carries which. */
		CG_GetEntityOrientation( arg0, (float *)arg1, (vec3_t *)arg2 );
		return 0;
	case 14:
		/* raw pass-through -- 0x3002051D..0x30020548 pushes the dwords
		   unchanged, so the six slots CG_ImpactMark reads as floats are
		   reinterpreted in place; IntAsFloat is an out-of-line call under
		   /Ob1 and retail does not call it here. */
		CG_ImpactMark( arg0, (const float *)arg1, (const float *)arg2, *(float *)&arg3,
					   *(float *)&arg4, *(float *)&arg5, *(float *)&arg6,
					   *(float *)&arg7, arg8, *(float *)&arg9, arg10, arg11 );
		return 0;
	case CG_PROFILE_DRAW_TRAP:
		/* arg3 is SHADOW: 0x30020448 pushes a literal 0 into the setColor slot
		   and leaves arg3 in eax; the callee never touches eax until
		   0x300196C7, where `neg eax / sbb eax,eax / and eax,3` -- eax ? 3 : 0
		   -- becomes trap_R_Text_Paint's last argument, which is the drop
		   shadow.  edx = virtualScreen and ecx = setColor. */
		CG_DrawStringExt( (float)arg0, (float)arg1, (const char *)arg2, NULL,
						  qfalse, arg3, (float)arg4, (float)arg5, arg6, qtrue );
		return 0;
	case CG_SCRIPT_HOOK:
		return (int)Scr_FarHook( (const void *)arg0 );
	case 17:
	case 18:
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int i;
	cvarTable_t *cv;
	char var[MAX_STRING_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
	}

	/* see if we are also running the server on this machine */
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	trap_Cvar_Set( "cl_stance", "0" );
	trap_Cvar_Set( "cl_run", "1" );
	trap_Cvar_Set( "cg_objectiveText", "" );
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int i;
	cvarTable_t *cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Update( cv->vmCvar );
	}
}

/*
===============
CG_CrosshairPlayer
===============
*/
int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list argptr;
	char text[MAX_STRING_CHARS];

	va_start( argptr, msg );
	vsprintf( text, msg, argptr );
	va_end( argptr );

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list argptr;
	char text[MAX_STRING_CHARS];

	va_start( argptr, msg );
	vsprintf( text, msg, argptr );
	va_end( argptr );

	trap_Error( text );
}

void CG_GameMessage( const char *msg ) {
	trap_GameMessage( msg, cg_gameMessageWidth.integer );
}

void CG_BoldGameMessage( const char *msg ) {
	trap_BoldGameMessage( msg, cg_gameBoldMessageWidth.integer );
}

/*
===============
CG_DreathMessage

The retail spelling of the name, and dead code in 1.1 -- nothing in the module
calls it.
===============
*/
void CG_DreathMessage( const char *attacker, const float *attackerColor, const char *victim,
					   const float *victimColor, const char *icon, float iconWidth,
					   float iconHeight, const float *iconColor ) {
	trap_DeathMessage( attacker, attackerColor, victim, victimColor, icon,
					   iconWidth, iconHeight, iconColor );
}

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list argptr;
	char text[MAX_STRING_CHARS];

	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr );

	CG_Error( "%s", text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list argptr;
	char text[MAX_STRING_CHARS];

	va_start( argptr, msg );
	vsprintf( text, msg, argptr );
	va_end( argptr );

	CG_Printf( "%s", text );
}

void QDECL Com_DPrintf( const char *msg, ... ) {
	va_list argptr;
	char text[MAX_STRING_CHARS];

	if ( !cg_developer.integer ) {
		return;
	}

	va_start( argptr, msg );
	vsprintf( text, msg, argptr );
	va_end( argptr );

	CG_Printf( "%s", text );
}

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}

/*
=================
CG_RegisterSurfaceTypeSounds

One alias per surface type, "<baseName>_<surfaceName>".  The argument order is
INFERRED: both parameters arrive in registers (ebx/edi at 0x30020980), so the
binary does not fix it.
=================
*/
static void CG_RegisterSurfaceTypeSounds( sfxHandle_t *table, const char *baseName ) {
	int i;
	char name[256];

	for ( i = 0 ; i < 23 ; i++ ) {
		sprintf( name, "%s_%s", baseName, trap_SurfaceTypeToName( i ) );
		table[i] = trap_Com_SoundAliasString( name );
	}
}

/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int size;

	size = trap_MemoryRemaining();
	CG_ParseVoiceChats( "mp/axis_chat.voice", &voiceChatLists[0], MAX_VOICECHATS );
	CG_ParseVoiceChats( "mp/allies_chat.voice", &voiceChatLists[1], MAX_VOICECHATS );
	CG_Printf( "voice chat memory size = %d\n", size - trap_MemoryRemaining() );

	cgs.media.announceGermanTwoMinutes = trap_Com_SoundAliasString( "mp_announce_g_twominutes" );
	cgs.media.announceAlliedTwoMinutes = trap_Com_SoundAliasString( "mp_announce_a_twominutes" );
	cgs.media.announceGermanThirtySeconds = trap_Com_SoundAliasString( "mp_announce_g_thirtyseconds" );
	cgs.media.announceAlliedThirtySeconds = trap_Com_SoundAliasString( "mp_announce_a_thirtyseconds" );

	cgs.media.gibSound = trap_Com_SoundAliasString( "player_gib" );
	cgs.media.gibBounceSound = trap_Com_SoundAliasString( "player_gib_bounce" );
	cgs.media.flamebarrelBounce = trap_Com_SoundAliasString( "flamebarrel_bounce" );
	cgs.media.noAmmoSound = trap_Com_SoundAliasString( "player_out_of_ammo" );
	cgs.media.talkSound = trap_Com_SoundAliasString( "player_talk" );
	cgs.media.landDamageSound = trap_Com_SoundAliasString( "land_damage" );
	cgs.media.playerWaterIn = trap_Com_SoundAliasString( "player_water_in" );
	cgs.media.playerWaterOut = trap_Com_SoundAliasString( "player_water_out" );
	cgs.media.grenadePulse[0] = trap_Com_SoundAliasString( "player_grenade_pulse_0" );
	cgs.media.grenadePulse[1] = trap_Com_SoundAliasString( "player_grenade_pulse_1" );
	cgs.media.grenadePulse[2] = trap_Com_SoundAliasString( "player_grenade_pulse_2" );
	cgs.media.grenadePulse[3] = trap_Com_SoundAliasString( "player_grenade_pulse_3" );
	cgs.media.debrisBounce = trap_Com_SoundAliasString( "debris_bounce" );
	cgs.media.debrisHitPlayer = trap_Com_SoundAliasString( "debris_hit_player" );
	cgs.media.flameSound = trap_Com_SoundAliasString( "flame_sound" );
	cgs.media.flameBlowSound = trap_Com_SoundAliasString( "flame_blow_sound" );
	cgs.media.flameStreamSound = trap_Com_SoundAliasString( "flame_stream_sound" );
	cgs.media.flameCrackSound = trap_Com_SoundAliasString( "flame_crack_sound" );
	cgs.media.playerBoneBounce = trap_Com_SoundAliasString( "player_bone_bounce" );

	/* The ten 23-entry surface tables live in cgMedia_t's unknown_0x2F8 hole
	   (92-byte stride, cgs+0x9CDC..).  Table 3 (cgs+0x9DF0) is registered
	   somewhere else -- CG_RegisterSounds skips it. */
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[0 * 92], "grenade_bounce" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[1 * 92], "grenade_explode" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[2 * 92], "rocket_explode" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[4 * 92], "bullet_small" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[5 * 92], "bullet_large" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[6 * 92], "step_run" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[7 * 92], "step_walk" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[8 * 92], "step_prone" );
	CG_RegisterSurfaceTypeSounds( (sfxHandle_t *)&cgs.media.unknown_0x2F8[9 * 92], "land" );

	cgs.media.gearRattleRun = trap_Com_SoundAliasString( "gear_rattle_run" );
	cgs.media.gearRattleWalk = trap_Com_SoundAliasString( "gear_rattle_walk" );
	cgs.media.movementFoliage = trap_Com_SoundAliasString( "movement_foliage" );
	cgs.media.whizbySound = trap_Com_SoundAliasString( "whizby" );
	cgs.media.meleeSwingLarge = trap_Com_SoundAliasString( "melee_swing_large" );
	cgs.media.meleeSwingSmall = trap_Com_SoundAliasString( "melee_swing_small" );
	cgs.media.meleeHit = trap_Com_SoundAliasString( "melee_hit" );
	cgs.media.gameMessageSound = trap_Com_SoundAliasString( "game_message" );
	cgs.media.objectiveCompleteSound = trap_Com_SoundAliasString( "objective_complete" );
	cgs.media.spotlightSpark = trap_Com_SoundAliasString( "spotlight_spark" );
}

/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int i;
	char name[10];
	vec3_t mins, maxs;
	int j;
	const char *modelName;
	const char *fxName;
	const char *shellShockName;
	static const char *sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b"
	};

	Com_Printf( "^5---------- Fx System Initialization ---------\n" );
	trap_syscall_0xE7();
	Com_Printf( "^5----- Fx System Initialization Complete -----\n" );

	CG_LoadingString( " - textures" );

	for ( i = 0 ; i < 11 ; i++ ) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i], 5 );
	}

	cgs.media.disconnectShader = trap_R_RegisterShader( "gfx/2d/net.tga", 5 );
	cgs.media.lagometerShader = trap_R_RegisterShader( "lagometer", 5 );
	cgs.media.headiconDisconnected = trap_R_RegisterShader( "headiconDisconnected", 5 );
	cgs.media.headiconYouInKillCam = trap_R_RegisterShader( "headiconYouInKillCam", 5 );

	/* precache only -- the module never keeps these handles */
	trap_R_RegisterShader( "killIconMelee", 5 );
	trap_R_RegisterShader( "killIconSuicide", 5 );
	trap_R_RegisterShader( "killIconFalling", 5 );
	trap_R_RegisterShader( "killIconCrush", 5 );
	trap_R_RegisterShader( "killIconDrown", 5 );
	trap_R_RegisterShader( "killIconSlime", 5 );
	trap_R_RegisterShader( "killIconDied", 5 );

	cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer", 4 );
	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select", 5 );
	cgs.media.hintShaders[HINT_ACTIVATE] = trap_R_RegisterShader( "hintActivate", 5 );
	cgs.media.hintShaders[HINT_NOACTIVATE] = trap_R_RegisterShader( "hintNoActivate", 5 );
	cgs.media.hintShaders[HINT_DOOR] = trap_R_RegisterShader( "hintDoor", 5 );
	cgs.media.hintShaders[HINT_NODOOR] = trap_R_RegisterShader( "hintNoDoor", 5 );
	cgs.media.hintShaders[HINT_MG42] = trap_R_RegisterShader( "hintMg42", 5 );
	cgs.media.hintShaders[HINT_HEALTH] = trap_R_RegisterShader( "hintHealth", 5 );
	cgs.media.hintShaders[HINT_LADDER] = trap_R_RegisterShader( "hintLadder", 5 );
	cgs.media.hintShaders[HINT_FRIENDLY] = trap_R_RegisterShader( "hintFriendly", 5 );
	cgs.media.hudStanceStand = trap_R_RegisterShader( "hudStanceStand", 5 );
	cgs.media.hudStanceCrouch = trap_R_RegisterShader( "hudStanceCrouch", 5 );
	cgs.media.hudStanceProne = trap_R_RegisterShader( "hudStanceProne", 5 );
	cgs.media.hudStanceFlash = trap_R_RegisterShader( "hudStanceFlash", 5 );
	cgs.media.hudObjective = trap_R_RegisterShader( "hudObjective", 5 );
	cgs.media.hudObjectiveUp = trap_R_RegisterShader( "hudObjectiveUp", 5 );
	cgs.media.hudObjectiveDown = trap_R_RegisterShader( "hudObjectiveDown", 5 );
	cgs.media.objectiveFriendly = trap_R_RegisterShader( "gfx/hud/hud@objective_friendly.tga", 5 );
	cgs.media.objectiveFriendlyChat = trap_R_RegisterShader( "gfx/hud/hud@objective_friendly_chat.tga", 5 );
	cgs.media.hudHitDirection = trap_R_RegisterShader( "hudHitDirection", 5 );

	cgs.media.checkboxClear = trap_R_RegisterShaderNoMip( "ui/assets/checkbox_clear", 5 );
	cgs.media.checkboxChecked = trap_R_RegisterShaderNoMip( "ui/assets/checkbox_checked", 5 );
	cgs.media.checkboxFail = trap_R_RegisterShaderNoMip( "ui/assets/checkbox_fail", 5 );

	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile", 5 );
	cgs.media.hudNoWeaponIcon = trap_R_RegisterShader( "hudNoWeaponIcon", 5 );
	cgs.media.flareShader = trap_R_RegisterShader( "flareShader", 4 );
	cgs.media.hudColorBar = trap_R_RegisterShader( "hudColorBar", 5 );
	cgs.media.hudAlliedIcon = trap_R_RegisterShader( "hudAlliedIcon", 5 );
	cgs.media.hudAxisIcon = trap_R_RegisterShader( "hudAxisIcon", 5 );

	CG_LoadingString( " - models" );

	cgs.media.headiconVoiceChat = trap_R_RegisterShader( "headiconVoiceChat", 2 );
	cgs.media.headiconTalkBalloon = trap_R_RegisterShader( "headiconTalkBalloon", 2 );
	cgs.media.railCoreShader = trap_R_RegisterShader( "railCore", 1 );

	CG_RegisterScoreboardGraphics();

	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	CG_LoadingString( " - items" );

	CG_RegisterItems();

	cgs.media.markShadowShader = trap_R_RegisterShader( "markShadow", 4 );
	cgs.media.wakeMarkShader = trap_R_RegisterShader( "wake", 4 );

	CG_LoadingString( " - inline models" );

	cgs.unknown_0x0711C = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.unknown_0x0711C ; i++ ) {
		Com_sprintf( name, sizeof( name ), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name, 7 );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			/* cgs+0x7920: the inline-model midpoint table inside unknown_0x07520 */
			( (vec3_t *)&cgs.unknown_0x07520[0x400] )[i][j] =
				mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	CG_LoadingString( " - server models" );

	for ( i = 1 ; i < MAX_MODELS ; i++ ) {
		modelName = CG_ConfigString( 268 + i );      /* CS_MODELS */
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName, 7 );
	}

	for ( i = 1 ; i < MAX_FX ; i++ ) {
		fxName = CG_ConfigString( 780 + i );         /* CS_FX */
		if ( !fxName[0] ) {
			break;
		}
		cgs.gameEffects[i] = trap_syscall_0xDE( fxName );
	}

	for ( i = 1 ; i < MAX_SHELLSHOCK_PARMS ; i++ ) {
		shellShockName = CG_ConfigString( 1100 + i );    /* CS_SHELLSHOCKS */
		if ( !shellShockName[0] ) {
			break;
		}
		if ( !CG_LoadShellShockCvars( shellShockName ) ) {
			CG_Error( "couldn't register shell shock '%s' -- see console\n", shellShockName );
		}
		CG_SetShellShockParmsFromCvars( &cgs.shellshockParms[i] );
	}

	CG_RegisterImpactEffects();

	/* cgs+0xA42C, inside cgMedia_t's unknown_0x70C hole */
	*(int *)&cgs.media.unknown_0x70C[0x33C] =
		trap_syscall_0xDE( "fx/impacts/flesh_hit_noblood.efx" );

	CG_LoadingString( " - game media done" );
}

/*
================
CG_ConfigString
================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[index];
}

/*
================
CG_StartAmbient
================
*/
void CG_StartAmbient( void ) {
	const char *info;
	const char *alias;
	int t;

	info = CG_ConfigString( 3 );                     /* CS_AMBIENT */
	alias = trap_Com_PickSoundAlias( Info_ValueForKey( info, "n" ) );

	t = atoi( Info_ValueForKey( info, "t" ) ) - cg.time;
	if ( t < 0 || !cg.time ) {
		t = 0;
	}

	trap_syscall_0xD6( (int)alias, t );
}

void CG_PlayClientSoundAliasByName( const char *name ) {
	CG_PlaySoundAliasByName( name, cg.snap->ps.clientNum, cg.snap->ps.origin );
}

void CG_PlayEntitySoundAliasByName( int entnum, const char *name ) {
	CG_PlaySoundAliasByName( name, entnum, cg_entities[entnum].currentState.pos.trBase );
}

/*
================
CG_PlaySoundAliasByName

Returns the length of the sound in milliseconds, and posts the alias' subtitle
if it has one.  The engine's sound-alias record is not typed here: cgame only
ever reaches its +0x08 member, which is the subtitle reference.

`name` arrives in eax (0x30021BF5), so the binary only fixes the relative
order of entnum and origin.  The binary loads eax from the msec at both exits,
so the return is an int.
================
*/
int CG_PlaySoundAliasByName( const char *name, int entnum, const vec3_t origin ) {
	const char *alias;
	const char *subtitle;
	int msec;

	alias = trap_Com_PickSoundAlias( name );
	if ( !alias ) {
		return 0;
	}

	msec = trap_MSS_PlaySoundAlias( alias, entnum, origin, 0 );

	subtitle = ( (const char **)alias )[2];
	if ( msec && subtitle ) {
		/* MAX with Q_ftol evaluated twice: retail inlines the first (0x30021C38)
		   and calls the out-of-line copy at 0x3000CAC0 for the second. */
		trap_Subtitle( subtitle, Q_ftol( cg_subtitleMinTime.value * 1000 ) > msec
					   ? Q_ftol( cg_subtitleMinTime.value * 1000 ) : msec,
					   cg_subtitleWidth.integer );
	}

	return msec;
}

/*
=================
CG_Asset_Parse

`loadMode` is the value the sole caller chain passes down from CG_LoadHudMenu
(5); it is the second argument of every shader/font registration below.  The
spelling is UO's, not a recovered one.
=================
*/
static qboolean CG_Asset_Parse( int handle, int loadMode ) {
	pc_token_t token;
	const char *tempStr;

	if ( !trap_PC_ReadToken( handle, &token ) ) {
		return qfalse;
	}
	if ( Q_stricmp( token.string, "{" ) != 0 ) {
		return qfalse;
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			return qfalse;
		}

		if ( Q_stricmp( token.string, "}" ) == 0 ) {
			return qtrue;
		}

		/* font */
		if ( Q_stricmp( token.string, "font" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return qfalse;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.textFont, loadMode );
			continue;
		}

		/* smallFont */
		if ( Q_stricmp( token.string, "smallFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return qfalse;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.smallFont, loadMode );
			continue;
		}

		/* bigfont */
		if ( Q_stricmp( token.string, "bigfont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return qfalse;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.bigFont, loadMode );
			continue;
		}

		/* extrabigfont */
		if ( Q_stricmp( token.string, "extrabigfont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return qfalse;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.extraBigFont, loadMode );
			continue;
		}

		/* boldFont */
		if ( Q_stricmp( token.string, "boldFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return qfalse;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.boldFont, loadMode );
			continue;
		}

		/* consoleFont */
		if ( Q_stricmp( token.string, "consoleFont" ) == 0 ) {
			int pointSize;
			if ( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) ) {
				return qfalse;
			}
			cgDC.registerFont( tempStr, pointSize, &cgDC.Assets.consoleFont, loadMode );
			continue;
		}

		/* gradientbar */
		if ( Q_stricmp( token.string, "gradientbar" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( tempStr, loadMode );
			continue;
		}

		/* enterMenuSound */
		if ( Q_stricmp( token.string, "menuEnterSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = trap_Com_SoundAliasString( tempStr );
			continue;
		}

		/* exitMenuSound */
		if ( Q_stricmp( token.string, "menuExitSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = trap_Com_SoundAliasString( tempStr );
			continue;
		}

		/* itemFocusSound */
		if ( Q_stricmp( token.string, "itemFocusSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_Com_SoundAliasString( tempStr );
			continue;
		}

		/* menuBuzzSound */
		if ( Q_stricmp( token.string, "menuBuzzSound" ) == 0 ) {
			if ( !PC_String_Parse( handle, &tempStr ) ) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = trap_Com_SoundAliasString( tempStr );
			continue;
		}

		if ( Q_stricmp( token.string, "cursor" ) == 0 ) {
			if ( !PC_String_Parse( handle, &cgDC.Assets.cursorStr ) ) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr, loadMode );
			continue;
		}

		if ( Q_stricmp( token.string, "fadeClamp" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.fadeClamp ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeCycle" ) == 0 ) {
			if ( !PC_Int_Parse( handle, &cgDC.Assets.fadeCycle ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeAmount" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.fadeAmount ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "fadeInAmount" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.fadeInAmount ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowX" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.shadowX ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowY" ) == 0 ) {
			if ( !PC_Float_Parse( handle, &cgDC.Assets.shadowY ) ) {
				return qfalse;
			}
			continue;
		}

		if ( Q_stricmp( token.string, "shadowColor" ) == 0 ) {
			if ( !PC_Color_Parse( handle, &cgDC.Assets.shadowColor ) ) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
}

/*
=================
CG_ParseMenu
=================
*/
void CG_ParseMenu( const char *menuFile, int loadMode ) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource( menuFile );
	if ( !handle ) {
		handle = trap_PC_LoadSource( "ui_mp/testhud.menu" );
	}
	if ( !handle ) {
		return;
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( Q_stricmp( token.string, "assetGlobalDef" ) == 0 ) {
			if ( CG_Asset_Parse( handle, loadMode ) ) {
				continue;
			} else {
				break;
			}
		}

		if ( Q_stricmp( token.string, "menudef" ) == 0 ) {
			/* start a new menu */
			Menu_New( handle, loadMode );
		}
	}
	trap_PC_FreeSource( handle );
}

/*
=================
CG_Load_Menu
=================
*/
qboolean CG_Load_Menu( char **p, int loadMode ) {
	char *token;

	token = Com_Parse( p );

	if ( token[0] != '{' ) {
		return qfalse;
	}

	while ( 1 ) {
		token = Com_Parse( p );

		if ( token == NULL ) {
			return qfalse;
		}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			return qtrue;
		}

		if ( !token[0] ) {
			return qfalse;
		}

		CG_ParseMenu( token, loadMode );
	}
}

/* 4096; the fixed buffer CG_LoadMenus reads the hud file into (0x30094B20). */
#define MAX_MENUDEFFILE     4096

/*
=================
CG_LoadMenus
=================
*/
void CG_LoadMenus( const char *menuFile, int loadMode ) {
	char *token;
	char *p;
	int len, start;
	fileHandle_t f;
	static char fileText[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

	len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) {
		trap_Error( va( "^3menu file not found: %s, using default\n", menuFile ) );
		len = trap_FS_FOpenFile( "ui_mp/hud.txt", &f, FS_READ );
		if ( !f ) {
			trap_Error( va( "^1default menu file not found: ui/hud.txt, unable to continue!\n", menuFile ) );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) {
		trap_Error( va( "^1menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( fileText, len, f );
	fileText[len] = 0;
	trap_FS_FCloseFile( f );

	Com_Compress( fileText );

	menuCount = 0;
	p = fileText;

	while ( 1 ) {
		token = Com_Parse( &p );
		if ( !token || token[0] == 0 || token[0] == '}' ) {
			break;
		}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if ( Q_stricmp( token, "loadmenu" ) == 0 ) {
			if ( CG_Load_Menu( &p, loadMode ) ) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_Printf( "UI menu load time = %d milli seconds\n", trap_Milliseconds() - start );
}

static qboolean CG_OwnerDrawHandleKey( int ownerDraw, int flags, float *special, int key ) {
	return qfalse;
}

static int CG_FeederCount( float feederID ) {
	int i, count;

	count = 0;
	if ( feederID == 5 ) {                  /* FEEDER_REDTEAM_LIST */
		for ( i = 0 ; i < cg.numScores ; i++ ) {
			if ( cg.scores[i].team == 1 ) {
				count++;
			}
		}
	} else if ( feederID == 6 ) {            /* FEEDER_BLUETEAM_LIST */
		for ( i = 0 ; i < cg.numScores ; i++ ) {
			if ( cg.scores[i].team == 2 ) {
				count++;
			}
		}
	} else if ( feederID == 11 ) {           /* FEEDER_SCOREBOARD */
		return cg.numScores;
	}

	return count;
}

static const char *CG_FeederItemText( float feederID, int index, int column, qhandle_t *handle ) {
	return "";
}

static qhandle_t CG_FeederItemImage( float feederID, int index ) {
	return 0;
}

static void CG_FeederSelection( float feederID, int index ) {
}

static float CG_Cvar_Get( const char *cvar ) {
	char buff[128];

	memset( buff, 0, sizeof( buff ) );
	trap_Cvar_VariableStringBuffer( cvar, buff, sizeof( buff ) );
	return atof( buff );
}

static int CG_OwnerDrawWidth( int ownerDraw, int font, float scale ) {
	const char *s;

	switch ( ownerDraw ) {
	case 39:
		return trap_R_Text_Width( cgs.gametype, font, scale, 0 );
	case 50:
		s = "";
		if ( cg.killerName[0] ) {
			s = va( "Fragged by %s", cg.killerName );
		}
		return trap_R_Text_Width( s, font, scale, 0 );
	}
	return 0;
}

static int CG_PlayCinematic( const char *name, float x, float y, float w, float h ) {
	return trap_CIN_PlayCinematic( name, x, y, w, h, 2 );
}

static void CG_StopCinematic( int handle ) {
	trap_CIN_StopCinematic( handle );
}

static void CG_DrawCinematic( int handle, float x, float y, float w, float h ) {
	trap_CIN_SetExtents( handle, x, y, w, h );
	trap_CIN_DrawCinematic( handle );
}

static void CG_RunCinematicFrame( int handle ) {
	trap_CIN_RunCinematic( handle );
}

/*
================
CG_SafeTranslateString_Internal

The argument order is INFERRED (both parameters arrive in registers, eax/ecx at
0x30022842); `domain` is the one that prints first in the warning.
================
*/
const char *CG_SafeTranslateString_Internal( const char *domain, const char *reference ) {
	static char translated[MAX_STRING_CHARS];
	const char *localized;

	localized = trap_SE_TranslateReference( reference );
	if ( localized ) {
		return localized;
	}

	if ( cl_languagewarnings.integer ) {
		if ( cl_languagewarningsaserrors.integer ) {
			Com_Error( 7, "Could not translate %s string \"%s\"", domain, reference );
		} else {
			Com_Printf( "^3WARNING: Could not translate %s string \"%s\"\n", domain, reference );
		}
		strcpy( translated, "^1UNLOCALIZED(^7" );
		strcat( translated, reference );
		strcat( translated, "^1)^7" );
		return translated;
	}

	strcpy( translated, reference );
	return translated;
}

const char *CG_SafeTranslateString( const char *reference ) {
	return CG_SafeTranslateString_Internal( "cgame", reference );
}

const char *CG_TranslateMessage( const char *src, const char *reference );

const char *CG_SafeTranslateHudElemString( int index ) {
	if ( !index ) {
		return "";
	}
	/* CS_HUDELEM_STRINGS */
	return CG_TranslateMessage( CG_ConfigString( index + 1244 ), "hudelem string" );
}

/*
================
CG_TranslateMessage

Localizes src, then replaces the first "[{command}]" marker with the localized
name of the key bound to command.  The brackets themselves are kept.
================
*/
const char *CG_TranslateMessage( const char *src, const char *reference ) {
	static char buf[2][MAX_STRING_CHARS];
	static int index;
	char command[MAX_STRING_CHARS];
	const char *s;
	const char *start, *end;
	char *keyName;
	char *out;
	int i, keyLen;

	s = trap_SE_LocalizeMessage( src, reference );

	for ( i = 0 ; s[i] ; i++ ) {
		if ( !s[i + 1] || !s[i + 2] || !s[i + 3] || !s[i + 4] ) {
			return s;
		}
		if ( s[i] == '[' && s[i + 1] == '{' ) {
			break;
		}
	}
	if ( !s[i] ) {
		return s;
	}
	start = &s[i];

	for ( ; s[i] ; i++ ) {
		if ( !s[i + 1] ) {
			return s;
		}
		if ( s[i] == '}' && s[i + 1] == ']' ) {
			break;
		}
	}
	if ( !s[i] ) {
		return s;
	}
	end = &s[i];

	Q_strncpyz( command, start + 2, ( end - start ) - 1 );

	Controls_GetConfig();
	if ( !GetKeyBindingLocalizedString( command, &keyName ) ) {
		return s;
	}
	keyLen = strlen( keyName );

	index ^= 1;
	out = buf[index];

	if ( *end ) {
		while ( s[i] ) {
			i++;
		}
	}

	if ( keyLen + i - ( end - start ) >= MAX_STRING_CHARS ) {
		Com_Printf( "String too long to add key binding: %s\n", s );
		return s;
	}

	Q_strncpyz( out, s, ( start - s ) + 2 );
	Q_strncpyz( out + ( start - s ) + 1, keyName, keyLen + 1 );
	Q_strncpyz( out + ( start - s ) + 1 + keyLen, end + 1,
				MAX_STRING_CHARS - ( ( start - s ) + 1 + keyLen ) );

	return out;
}

/*
=================
CG_LoadHudMenu
=================
*/
static void CG_LoadHudMenu( void ) {
	char buff[MAX_STRING_CHARS];
	const char *hudSet;

	trap_Cvar_Set( "cg_hudFiles", "ui_mp/hud.txt" );

	cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	cgDC.setColor = &trap_R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &trap_R_DrawStretchPic;
	cgDC.drawText = &trap_R_Text_Paint;
	cgDC.textWidth = &trap_R_Text_Width;
	cgDC.textHeight = &trap_R_Text_Height;
	cgDC.translateReference = &trap_SE_TranslateReference;
	cgDC.safeTranslateString = &CG_SafeTranslateString;
	cgDC.translatedMessage = &CG_TranslateMessage;
	cgDC.registerModel = &trap_R_RegisterModel;
	cgDC.modelBounds = &trap_R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &trap_R_ClearScene;
	cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	cgDC.renderScene = &trap_R_RenderScene;
	/* cg_syscalls_mp.c types trap_R_RegisterFont's third argument int; here it
	   is the fontInfo_t CG_Asset_Parse fills in */
	cgDC.registerFont = ( int ( * )( const char *, int, fontInfo_t *, int ) )&trap_R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	cgDC.getValue = &CG_GetValue;
	cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	cgDC.runScript = &CG_RunMenuScript;
	cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = &trap_Cvar_Set;
	cgDC.getCVarString = &trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue = &CG_Cvar_Get;
	cgDC.configString = &CG_ConfigString;
	cgDC.drawTextWithCursor = &trap_R_Text_PaintWithCursor;
	cgDC.playClientSoundAliasByName = &CG_PlayClientSoundAliasByName;
	cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	/* +0xB4/+0xB0/+0xAC in store order (0x30022D2E..0x30022D42).  Nothing
	   assigns trap_MSS_GetSoundOverlay (218) -- the three slots take traps
	   220, 219 and 221. */
	cgDC.setBinding = &trap_Key_SetBinding;
	cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
	cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	cgDC.Error = &Com_Error;
	cgDC.Print = &Com_Printf;
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	cgDC.registerSound = &trap_Com_SoundAliasString;
	cgDC.playCinematic = &CG_PlayCinematic;
	cgDC.stopCinematic = &CG_StopCinematic;
	cgDC.drawCinematic = &CG_DrawCinematic;
	cgDC.runCinematicFrame = &CG_RunCinematicFrame;

	Init_Display( &cgDC );

	Menu_Reset();

	trap_Cvar_VariableStringBuffer( "cg_hudFiles", buff, sizeof( buff ) );
	hudSet = buff;
	if ( hudSet[0] == '\0' ) {
		hudSet = "ui_mp/hud.txt";
	}

	CG_LoadMenus( hudSet, 5 );
}

/*
=================
CG_AssetCache
=================
*/
static void CG_AssetCache( void ) {
	cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( "ui/assets/gradientbar2.tga", 2 );
	cgDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( "ui/assets/scrollbar.tga", 2 );
	cgDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( "ui/assets/scrollbar_arrow_dwn_a.tga", 2 );
	cgDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( "ui/assets/scrollbar_arrow_up_a.tga", 2 );
	cgDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( "ui/assets/scrollbar_arrow_left.tga", 2 );
	cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( "ui/assets/scrollbar_arrow_right.tga", 2 );
	cgDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( "ui/assets/scrollbar_thumb.tga", 2 );
	cgDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( "ui/assets/slider2.tga", 2 );
	cgDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( "ui/assets/sliderbutt_1.tga", 2 );
}

/*
=================
CG_InitVote

0x30022F10 -- the compiler laid it down as a tail chunk of CG_Init rather than
a function of its own.  CG_Init's last act is a tail jump into it (0x300233E2).
=================
*/
static void CG_InitVote( void ) {
	cgs.voteTime = atoi( CG_ConfigString( 15 ) );
	cgs.voteYes = atoi( CG_ConfigString( 17 ) );
	cgs.voteNo = atoi( CG_ConfigString( 18 ) );
	Q_strncpyz( cgs.voteString,
				trap_SE_LocalizeMessage( CG_ConfigString( 16 ), "vote string" ),
				sizeof( cgs.voteString ) );
}

/*
=================
CG_Init

Called after every level change or subsystem restart.
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char *s;
	int i;

	/* clear everything */
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof( cg_entities ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );
	memset( cg_items, 0, sizeof( cg_items ) );

	/* retail: one rep stosd over 0x300F0520..0x3019CE0C (0x30022FDB), exactly
	   the G_InitGame clear the game DLL does over its own copy */
	memset( &bgs_animScriptData, 0, sizeof( bgs_animScriptData ) );
	bgs_animTree = NULL;
	memset( &bgs_rootAnim, 0, sizeof( bgs_rootAnim ) );
	memset( &bgs_torsoAnim, 0, sizeof( bgs_torsoAnim ) );
	memset( &bgs_legsAnim, 0, sizeof( bgs_legsAnim ) );
	memset( &bgs_turningAnim, 0, sizeof( bgs_turningAnim ) );
	memset( bg_clientinfo, 0, sizeof( bg_clientinfo ) );

	bgs_time = 0;
	bgs_animTime = 0;
	bgs_frametime = 0;

	/* RF, init the anim scripting */
	bgs_animScriptData.soundIndex = trap_Com_SoundAliasString;
	bgs_animScriptData.playSound = CG_PlayEntitySoundAliasByName;

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	cgs.media.whiteShader = trap_R_RegisterShader( "white", 5 );
	cgs.media.hudSoftLineShader = trap_R_RegisterShader( "hudSoftLine", 5 );
	cgs.media.hudSoftLineHShader = trap_R_RegisterShader( "hudSoftLineH", 5 );

	/* load a few needed things before we do any screen updates */
	CG_RegisterCvars();

	CG_InitConsoleCommands();

	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth * ( 1.0 / 640.0 );
	cgs.screenYScale = cgs.glconfig.vidHeight * ( 1.0 / 480.0 );

	/* get the rendering configuration from the client system */
	trap_GetGameState( &cgs.gameState );

	/* check version */
	s = CG_ConfigString( 2 );                /* CS_GAME_VERSION */
	if ( strcmp( s, "cod" ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", "cod", s );
	}

	s = CG_ConfigString( 13 );               /* CS_LEVEL_START_TIME */
	cgs.levelStartTime = atoi( s );

	/* cgs+0x0A438 is a float, not the int cg_local.h declares (0x300230FB) */
	i = rand();
	*(float *)&cgs.unknown_0x0A438 = 2.0f * ( i * ( 1.0f / 32768.0f ) ) - 1.0f;

	CG_ParseServerinfo();
	CG_ParseWolfinfo();

	BG_SetupWeaponInfo();
	CGScr_LoadAnimTrees();

	CG_LoadingString( "collision map" );

	trap_CM_LoadMap( cgs.mapname );

	String_Init();

	CG_LoadingString( "graphics" );

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

	CG_LoadingString( "sound aliases" );

	trap_Com_LoadSoundAliases( cgs.mapname );

	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_LoadingString( "game media" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_AssetCache();
	CG_LoadHudMenu();

	CG_InitLocalEntities();
	CG_InitMarkPolys();

	/* remove the last loading update */
	cg.infoScreenText[0] = 0;

	CG_SetConfigValues();

	CG_LoadingString( "" );

	cg.compassNorthYaw = atof( CG_ConfigString( 11 ) );  /* CS_COMPASS_NORTHYAW */

	trap_R_FinishLoadingModels();

	trap_syscall_0xD3( 0 );

	CG_StartAmbient();

	CG_InitVote();
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart.
=================
*/
void CG_Shutdown( void ) {
	trap_syscall_0x6B( 0 );
	trap_syscall_0xD7( 1.0f, 0 );

	CG_FreeWeapons();
	CG_FreeClientDObjInfo();
	CG_FreeEntityDObjInfo();

	trap_syscall_0xC8( 0 );
}
