/*
 * @fidelity: verified
 * @fidelity-default: verified
 */

#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"
#include "cl_records.h"
#include "cl_vm.h"

extern void CL_Netchan_Encode( byte *data, int length );
extern void MSG_SetDefaultUserCmd( const byte *ps, usercmd_t *ucmd );
extern void MSG_WriteBits( msg_t *msg, int value, int bits );
extern int  MSG_WriteBitsCompress( const byte *datasrc, int bytecount, byte *buffdest, int capacity );
extern void MSG_WriteByte( msg_t *msg, int c );
extern void MSG_WriteLong( msg_t *msg, int c );
extern void MSG_WriteDeltaUsercmdKey( const usercmd_t *from, const usercmd_t *to,
									  int key, msg_t *msg );
extern void MSG_WriteString( const char *s, msg_t *sb );
extern void Netchan_Transmit( netchan_t *chan, int length, const byte *data );
extern void Netchan_TransmitNextFragment( netchan_t *chan );
extern int  Com_HashKey( const char *string, int maxlen );
extern long j__atol( const char *s );

typedef struct kbutton_s
{
	int			down[2];
	int			downtime;
	int			msec;
	qboolean	active;
	qboolean	wasPressed;
} kbutton_t;

#define in_strafe_active		( ( (kbutton_t *)KB_STRAFE )->active )		/* 0x0087A0F8 */
#define in_speed_active			( ( (kbutton_t *)KB_SPEED )->active )		/* 0x0087A110 */
#define in_down_active			( ( (kbutton_t *)KB_DOWN )->active )		/* 0x0087A140 */
#define in_prone_active			( ( (kbutton_t *)KB_WBUTTONS5 )->active )	/* 0x0087A2C0 */

#define IN_BUTTONS_AT( sym, n )		( &( (kbutton_t *)(sym) )[ n ] )

#define cl_viewangles_PITCH		( *(float *)&cl_viewanglesPitch )
#define cl_viewangles_YAW		( *(float *)&cl_viewanglesYaw )
#define cl_viewangles_ROLL		cl_viewanglesRoll

#define cgameUserAim_PITCH		( *(float *)&cgameUserAim_x )
#define cgameUserAim_YAW		( *(float *)&cgameUserAim_y )
#define cgameUserAim_ROLL		( *(float *)&cgameUserAim_z )

/* cl.cgameMaxPitchSpeed / cl.cgameMaxYawSpeed -- degrees per second, set by CL_CgameSystemCalls at 0x004041A5 / 0x004041AE and read only by CL_MouseMove. Zero disables the clamp. */
#define cgameMaxPitchSpeed		( *(float *)&dword_143A980 )
#define cgameMaxYawSpeed		( *(float *)&dword_143A984 )

#define frame_msec				( *(unsigned int *)&dword_142F608 )
#define old_com_frameTime		cl_oldFrameTime

#define cl_mouseDx				cl_mouseDx
#define cl_mouseDy				cl_mouseDy
#define cl_mouseIndex			cl_mouseIndex
#define cl_joystickAxis			cl_joystickAxis

/* Expanded independently of the original fixed-address client layout. */
unsigned char stru_143A9B0[UCMD_SIZE * CMD_BACKUP];

#define cl_cmds					( (usercmd_t *)stru_143A9B0 )

#define OUTPACKET_CMDNUMBER( i )	dword_143AFB4[ ( (i) & PACKET_MASK ) * 3 ]
#define OUTPACKET_SERVERTIME( i )	dword_143AFB8[ ( (i) & PACKET_MASK ) * 3 ]
#define OUTPACKET_REALTIME( i )		dword_143AFBC[ ( (i) & PACKET_MASK ) * 3 ]

#define clc_netchan				( *(netchan_t *)chan )

#define clc_reliableCommands( i )	( &clc_reliableCommands[ MAX_STRING_CHARS_CMD * ( (i) & 0x3F ) ] )
#define clc_serverCommand( i )		( &clc_serverCommands[ MAX_STRING_CHARS_CMD * ( (i) & 0x3F ) ] )
#define MAX_STRING_CHARS_CMD		1024

#define clc_lastPacketSentTime	clc_lastPacketSentTime
#define clc_reliableAcknowledge	clc_reliableAcknowledge

#define cl_serverId				cl_serverId

#define cl_snap_ps				( (const byte *)&cl_snap_valid + SNAP_PS )

#define cl_snap_ps_pm_type		dword_1432984
#define cl_snap_ps_pm_flags		dword_143298C
#define cl_snap_ps_deltaPitch	dword_14329C8
#define cl_snap_ps_flags_80	dword_1432A00

#define BUTTON_ATTACK			0x01
#define BUTTON_TALK				0x02
#define BUTTON_WALKING			0x10
#define WBUTTON_STANCEHELD		0x02
#define WBUTTON_SHELLSHOCK		0x04
#define WBUTTON_PRONE			0x40
#define WBUTTON_CROUCH			0x80
#define WBUTTON_STANCE_MASK		0xC0

#define clc_move				0
#define clc_moveNoDelta			1
#define clc_clientCommand		2
#define clc_EOF					3

#define MAX_PACKET_USERCMDS		32
#define CL_ENCODE_START			9

#define ANGLE2SHORT_SCALE		182.04445f
#define SHORT2ANGLE_SCALE		0.0054931640625f

#define cl_debugMove_cvar		( (cvar_t *)cl_debugMove )

#define DEBUGGRAPH_MASK			0x3FF

/* ---- IN_MLookDown  0x0040A450 ----  VERIFIED */
void __cdecl IN_MLookDown( void )
{
  KB_MLOOK = qtrue;
}

/* ---- IN_MLookUp  0x0040A460 ----  VERIFIED */
void __cdecl IN_MLookUp( void )
{
	KB_MLOOK = qfalse;
	if ( !cl_freelook->integer ) {
		cl_viewangles_PITCH = -( (float)cl_snap_ps_deltaPitch * SHORT2ANGLE_SCALE );
	}
}

/* ---- IN_KeyDown  0x0040A490 ----  VERIFIED */
void __cdecl IN_KeyDown( void *button )
{
	kbutton_t	*b = (kbutton_t *)button;
	int			k;
	char		*c;

	if ( (unsigned int)cmd_argc > 1 && *cmd_argv[1] ) {
		k = j__atol( cmd_argv[1] );
	} else {
		k = -1;
	}

	if ( k == b->down[0] || k == b->down[1] ) {
		return;
	}

	if ( !b->down[0] ) {
		b->down[0] = k;
	} else if ( !b->down[1] ) {
		b->down[1] = k;
	} else {
		Com_Printf( "Three keys down for a button!\n" );
		return;
	}

	if ( b->active ) {
		return;
	}

	c = (char *)empty_string;
	if ( (unsigned int)cmd_argc > 2 ) {
		c = cmd_argv[2];
	}
	b->downtime = j__atol( c );

	b->active = qtrue;
	b->wasPressed = qtrue;
}

/* ---- IN_KeyUp  0x0040A510 ----  VERIFIED */
void __cdecl IN_KeyUp( void *button )
{
	kbutton_t		*b = (kbutton_t *)button;
	int				k;
	char			*c;
	unsigned int	uptime;

	if ( (unsigned int)cmd_argc <= 1 || !*cmd_argv[1] ) {
		b->down[1] = 0;
		b->down[0] = 0;
		b->active = qfalse;
		return;
	}
	k = j__atol( cmd_argv[1] );

	if ( b->down[0] == k ) {
		b->down[0] = 0;
	} else if ( b->down[1] == k ) {
		b->down[1] = 0;
		if ( b->down[0] ) {
			return;
		}
	} else {
		return;
	}

	if ( b->down[1] ) {
		return;
	}

	b->active = qfalse;

	c = (char *)empty_string;
	if ( (unsigned int)cmd_argc > 2 ) {
		c = cmd_argv[2];
	}
	uptime = j__atol( c );

	if ( uptime ) {
		b->msec += uptime - b->downtime;
	} else {
		b->msec += frame_msec / 2;
	}

	b->active = qfalse;
}

/* ---- CL_KeyState  0x0040A5A0 ----  VERIFIED */
float __cdecl CL_KeyState( void *button )
{
	kbutton_t	*key = (kbutton_t *)button;
	float		val;
	int			msec;

	msec = key->msec;
	key->msec = 0;

	if ( key->active ) {
		if ( !key->downtime ) {
			msec = com_frameTime;
		} else {
			msec += com_frameTime - key->downtime;
		}
		key->downtime = com_frameTime;
	}

	val = (float)( (double)msec / (double)frame_msec );
	if ( val < 0 ) {
		val = 0;
	}
	if ( val > 1 ) {
		val = 1;
	}

	return val;
}

/* ---- CL_SetTempStanceStatus  0x0040A620 ----  VERIFIED */
void __cdecl CL_SetTempStanceStatus( void )
{
  if ( in_prone_active || in_down_active )
    Cvar_Set2("cl_stanceTemp", "1", qtrue);
  else
    Cvar_Set2("cl_stanceTemp", "0", qtrue);
}

/* ---- IN_UpDown  0x0040A670 ----  VERIFIED */
void __cdecl IN_UpDown( void )
{
	int		stance;

	IN_KeyDown( kbutton );

	if ( !in_prone_active && !in_down_active ) {
		stance = cl_stance->integer;
		if ( stance > 1 ) {
			Cvar_Set2( "cl_stance", "1", qtrue );
		} else if ( stance > 0 ) {
			Cvar_Set2( "cl_stance", "0", qtrue );
		} else {
			IN_KeyDown( KB_UP );
		}
	}
}

/* ---- IN_UpUp  0x0040A6E0 ----  VERIFIED */
void __cdecl IN_UpUp( void )
{
  IN_KeyUp(kbutton);
  IN_KeyUp(KB_UP);
}

/* ---- IN_DownDown  0x0040A700 ----  VERIFIED */
void __cdecl IN_DownDown( void )
{
  IN_KeyDown(KB_DOWN);
  if ( in_prone_active || in_down_active )
    Cvar_Set2("cl_stanceTemp", "1", qtrue);
  else
    Cvar_Set2("cl_stanceTemp", "0", qtrue);
}

/* ---- IN_DownUp  0x0040A750 ----  VERIFIED */
void __cdecl IN_DownUp( void )
{
  IN_KeyUp(KB_DOWN);
  if ( in_prone_active || in_down_active )
    Cvar_Set2("cl_stanceTemp", "1", qtrue);
  else
    Cvar_Set2("cl_stanceTemp", "0", qtrue);
}

/* ---- IN_LeftDown  0x0040A7A0 ----  VERIFIED */
void __cdecl IN_LeftDown( void )
{
  IN_KeyDown(KB_LEFT);
}

/* ---- IN_LeftUp  0x0040A7B0 ----  VERIFIED */
void __cdecl IN_LeftUp( void )
{
  IN_KeyUp(KB_LEFT);
}

/* ---- IN_RightDown  0x0040A7C0 ----  VERIFIED */
void __cdecl IN_RightDown( void )
{
  IN_KeyDown(KB_RIGHT);
}

/* ---- IN_RightUp  0x0040A7D0 ----  VERIFIED */
void __cdecl IN_RightUp( void )
{
  IN_KeyUp(KB_RIGHT);
}

/* ---- IN_ForwardDown  0x0040A7E0 ----  VERIFIED */
void __cdecl IN_ForwardDown( void )
{
  IN_KeyDown(KB_FORWARD);
}

/* ---- IN_ForwardUp  0x0040A7F0 ----  VERIFIED */
void __cdecl IN_ForwardUp( void )
{
  IN_KeyUp(KB_FORWARD);
}

/* ---- IN_BackDown  0x0040A800 ----  VERIFIED */
void __cdecl IN_BackDown( void )
{
  IN_KeyDown(KB_BACK);
}

/* ---- IN_BackUp  0x0040A810 ----  VERIFIED */
void __cdecl IN_BackUp( void )
{
  IN_KeyUp(KB_BACK);
}

/* ---- IN_LookupDown  0x0040A820 ----  VERIFIED */
void __cdecl IN_LookupDown( void )
{
  IN_KeyDown(KB_LOOKUP);
}

/* ---- IN_LookupUp  0x0040A830 ----  VERIFIED */
void __cdecl IN_LookupUp( void )
{
  IN_KeyUp(KB_LOOKUP);
}

/* ---- IN_LookdownDown  0x0040A840 ----  VERIFIED */
void __cdecl IN_LookdownDown( void )
{
  IN_KeyDown(KB_LOOKDOWN);
}

/* ---- IN_LookdownUp  0x0040A850 ----  VERIFIED */
void __cdecl IN_LookdownUp( void )
{
  IN_KeyUp(KB_LOOKDOWN);
}

/* ---- IN_MoveleftDown  0x0040A860 ----  VERIFIED */
void __cdecl IN_MoveleftDown( void )
{
  IN_KeyDown(KB_MOVELEFT);
}

/* ---- IN_MoveleftUp  0x0040A870 ----  VERIFIED */
void __cdecl IN_MoveleftUp( void )
{
  IN_KeyUp(KB_MOVELEFT);
}

/* ---- IN_MoverightDown  0x0040A880 ----  VERIFIED */
void __cdecl IN_MoverightDown( void )
{
  IN_KeyDown(KB_MOVERIGHT);
}

/* ---- IN_MoverightUp  0x0040A890 ----  VERIFIED */
void __cdecl IN_MoverightUp( void )
{
  IN_KeyUp(KB_MOVERIGHT);
}

/* ---- IN_SpeedDown  0x0040A8A0 ----  VERIFIED */
void __cdecl IN_SpeedDown( void )
{
  IN_KeyDown(&KB_SPEED);
}

/* ---- IN_SpeedUp  0x0040A8B0 ----  VERIFIED */
void __cdecl IN_SpeedUp( void )
{
  IN_KeyUp(&KB_SPEED);
}

/* ---- IN_StrafeDown  0x0040A8C0 ----  VERIFIED */
void __cdecl IN_StrafeDown( void )
{
  IN_KeyDown(&KB_STRAFE);
}

/* ---- IN_StrafeUp  0x0040A8D0 ----  VERIFIED */
void __cdecl IN_StrafeUp( void )
{
  IN_KeyUp(&KB_STRAFE);
}

/* ---- IN_AttackDown  0x0040A8E0 ----  VERIFIED */
void __cdecl IN_AttackDown( void )
{
  IN_KeyDown(&KB_BUTTONS0);
}

/* ---- IN_AttackUp  0x0040A8F0 ----  VERIFIED */
void __cdecl IN_AttackUp( void )
{
  IN_KeyUp(&KB_BUTTONS0);
}

/* ---- IN_Button1Down  0x0040A900 ----  VERIFIED */
void __cdecl IN_Button1Down( void )
{
  IN_KeyDown(&KB_BUTTONS1);
}

/* ---- IN_Button1Up  0x0040A910 ----  VERIFIED */
void __cdecl IN_Button1Up( void )
{
  IN_KeyUp(&KB_BUTTONS1);
}

/* ---- IN_UseItemDown  0x0040A920 ----  VERIFIED */
void __cdecl IN_UseItemDown( void )
{
  IN_KeyDown(&KB_WBUTTONS6);
}

/* ---- IN_UseItemUp  0x0040A930 ----  VERIFIED */
void __cdecl IN_UseItemUp( void )
{
  IN_KeyUp(&KB_WBUTTONS6);
}

/* ---- IN_Button3Down  0x0040A940 ----  VERIFIED */
void __cdecl IN_Button3Down( void )
{
  IN_KeyDown(&KB_BUTTONS3);
}

/* ---- IN_Button3Up  0x0040A950 ----  VERIFIED */
void __cdecl IN_Button3Up( void )
{
  IN_KeyUp(&KB_BUTTONS3);
}

/* ---- IN_Button4Down  0x0040A960 ----  VERIFIED */
void __cdecl IN_Button4Down( void )
{
  IN_KeyDown(&KB_BUTTONS4);
}

/* ---- IN_Button4Up  0x0040A970 ----  VERIFIED */
void __cdecl IN_Button4Up( void )
{
  IN_KeyUp(&KB_BUTTONS4);
}

/* ---- IN_MeleeDown  0x0040A980 ----  VERIFIED */
void __cdecl IN_MeleeDown( void )
{
  IN_KeyDown(&KB_BUTTONS5);
}

/* ---- IN_MeleeUp  0x0040A990 ----  VERIFIED */
void __cdecl IN_MeleeUp( void )
{
  IN_KeyUp(&KB_BUTTONS5);
}

/* ---- IN_ActivateDown  0x0040A9A0 ----  VERIFIED */
void __cdecl IN_ActivateDown( void )
{
  IN_KeyDown(&KB_BUTTONS6);
}

/* ---- IN_ActivateUp  0x0040A9B0 ----  VERIFIED */
void __cdecl IN_ActivateUp( void )
{
  IN_KeyUp(&KB_BUTTONS6);
}

/* ---- IN_KickDown  0x0040A9C0 ----  VERIFIED */
void __cdecl IN_KickDown( void )
{
  IN_KeyDown(&KB_WBUTTONS0);
}

/* ---- IN_KickUp  0x0040A9D0 ----  VERIFIED */
void __cdecl IN_KickUp( void )
{
  IN_KeyUp(&KB_WBUTTONS0);
}

/* ---- IN_SprintDown  0x0040A9E0 ----  VERIFIED */
void __cdecl IN_SprintDown( void )
{
  IN_KeyDown(&KB_WBUTTONS1);
}

/* ---- IN_SprintUp  0x0040A9F0 ----  VERIFIED */
void __cdecl IN_SprintUp( void )
{
  IN_KeyUp(&KB_WBUTTONS1);
}

/* ---- IN_ReloadDown  0x0040AA00 ----  VERIFIED */
void __cdecl IN_ReloadDown( void )
{
  IN_KeyDown(&KB_WBUTTONS2);
}

/* ---- IN_ReloadUp  0x0040AA10 ----  VERIFIED */
void __cdecl IN_ReloadUp( void )
{
  IN_KeyUp(&KB_WBUTTONS2);
}

/* ---- IN_LeanLeftDown  0x0040AA20 ----  VERIFIED */
void __cdecl IN_LeanLeftDown( void )
{
  IN_KeyDown(&KB_WBUTTONS3);
}

/* ---- IN_LeanLeftUp  0x0040AA30 ----  VERIFIED */
void __cdecl IN_LeanLeftUp( void )
{
  IN_KeyUp(&KB_WBUTTONS3);
}

/* ---- IN_LeanRightDown  0x0040AA40 ----  VERIFIED */
void __cdecl IN_LeanRightDown( void )
{
  IN_KeyDown(&KB_WBUTTONS4);
}

/* ---- IN_LeanRightUp  0x0040AA50 ----  VERIFIED */
void __cdecl IN_LeanRightUp( void )
{
  IN_KeyUp(&KB_WBUTTONS4);
}

/* ---- IN_ProneDown  0x0040AA60 ----  VERIFIED */
void __cdecl IN_ProneDown( void )
{
  IN_KeyDown(&KB_WBUTTONS5);
  if ( in_prone_active || in_down_active )
    Cvar_Set2("cl_stanceTemp", "1", qtrue);
  else
    Cvar_Set2("cl_stanceTemp", "0", qtrue);
}

/* ---- IN_ProneUp  0x0040AAB0 ----  VERIFIED */
void __cdecl IN_ProneUp( void )
{
  IN_KeyUp(&KB_WBUTTONS5);
  if ( in_prone_active || in_down_active )
    Cvar_Set2("cl_stanceTemp", "1", qtrue);
  else
    Cvar_Set2("cl_stanceTemp", "0", qtrue);
}

/* ---- IN_MP_DropWeaponDown  0x0040AB00 ----  VERIFIED */
void __cdecl IN_MP_DropWeaponDown( void )
{
  IN_KeyDown(&KB_WBUTTONS6);
}

/* ---- IN_MP_DropWeaponUp  0x0040AB10 ----  VERIFIED */
void __cdecl IN_MP_DropWeaponUp( void )
{
  IN_KeyUp(&KB_WBUTTONS6);
}

/* ---- IN_Wbutton7Down  0x0040AB20 ----  VERIFIED */
void __cdecl IN_Wbutton7Down( void )
{
  IN_KeyDown(&KB_WBUTTONS7);
}

/* ---- IN_Wbutton7Up  0x0040AB30 ----  VERIFIED */
void __cdecl IN_Wbutton7Up( void )
{
  IN_KeyUp(&KB_WBUTTONS7);
}

/* ---- IN_ButtonDown  0x0040AB40 ----  VERIFIED */
void __cdecl IN_ButtonDown( void )
{
  IN_KeyDown(&KB_BUTTONS1);
}

/* ---- IN_ButtonUp  0x0040AB50 ----  VERIFIED */
void __cdecl IN_ButtonUp( void )
{
  IN_KeyUp(&KB_BUTTONS1);
}

/* ---- IN_CenterView  0x0040AB60 ----  VERIFIED */
void __cdecl IN_CenterView( void )
{
  *(float *)&cl_viewanglesPitch = -((double)dword_14329C8 * 0.0054931641);
}

/* ---- IN_LowerStance  0x0040AB80 ----  VERIFIED */
void __cdecl IN_LowerStance( void )
{
	int		stance;

	if ( !in_prone_active && !in_down_active ) {
		stance = cl_stance->integer;
		if ( stance < 1 ) {
			Cvar_Set2( "cl_stance", "1", qtrue );
		} else if ( stance < 2 ) {
			Cvar_Set2( "cl_stance", "2", qtrue );
		}
	}
}

/* ---- IN_RaiseStance  0x0040ABE0 ----  VERIFIED */
void __cdecl IN_RaiseStance( void )
{
	int		stance;

	if ( !in_prone_active && !in_down_active ) {
		stance = cl_stance->integer;
		if ( stance > 1 ) {
			Cvar_Set2( "cl_stance", "1", qtrue );
		} else if ( stance > 0 ) {
			Cvar_Set2( "cl_stance", "0", qtrue );
		}
	}
}

/* ---- IN_ToggleCrouch  0x0040AC40 ----  VERIFIED */
void __cdecl IN_ToggleCrouch( void )
{
	if ( !in_prone_active && !in_down_active ) {
		if ( cl_stance->integer == 1 ) {
			Cvar_Set2( "cl_stance", "0", qtrue );
		} else {
			Cvar_Set2( "cl_stance", "1", qtrue );
		}
	}
}

/* ---- IN_ToggleProne  0x0040AC90 ----  VERIFIED */
void __cdecl IN_ToggleProne( void )
{
	if ( !in_prone_active && !in_down_active ) {
		if ( cl_stance->integer == 2 ) {
			Cvar_Set2( "cl_stance", "0", qtrue );
		} else {
			Cvar_Set2( "cl_stance", "2", qtrue );
		}
	}
}

/* ---- IN_GoProne  0x0040ACE0 ----  VERIFIED */
void __cdecl IN_GoProne( void )
{
  if ( !in_prone_active && !in_down_active )
    Cvar_Set2("cl_stance", "2", qtrue);
}

/* ---- IN_GoCrouch  0x0040AD10 ----  VERIFIED */
void __cdecl IN_GoCrouch( void )
{
  if ( !in_prone_active && !in_down_active )
    Cvar_Set2("cl_stance", "1", qtrue);
}

/* ---- IN_GoStandDown  0x0040AD40 ----  VERIFIED */
void __cdecl IN_GoStandDown( void )
{
	int		jumpTime;

	IN_KeyDown( kbutton );

	jumpTime = cl_goStandJumpTime->integer;
	if ( jumpTime > 0 ) {
		if ( com_frameTime - cl_lastStandTime < jumpTime ) {
			IN_KeyDown( KB_UP );
			cl_lastStandTime = 0;
			return;
		}
	} else if ( !cl_stance->integer ) {
		IN_KeyDown( KB_UP );
		cl_lastStandTime = 0;
		return;
	}

	cl_lastStandTime = com_frameTime;
	if ( !in_prone_active && !in_down_active ) {
		Cvar_Set2( "cl_stance", "0", qtrue );
	}
}

/* ---- IN_GoStandUp  0x0040ADC0 ----  VERIFIED */
void __cdecl IN_GoStandUp( void )
{
  IN_KeyUp(kbutton);
  IN_KeyUp(KB_UP);
}

/* ---- CL_AdjustAngles  0x0040ADE0 ----  VERIFIED */
void __cdecl CL_AdjustAngles( void )
{
	float	speed;

	speed = (float)cls_frametime;
	if ( in_speed_active ) {
		speed = speed * cl_anglespeedkey->value;
	}
	speed = speed * 0.001f;

	if ( !in_strafe_active ) {
		cl_viewangles_YAW -= speed * cl_yawspeed->value * CL_KeyState( KB_RIGHT );
		cl_viewangles_YAW += speed * cl_yawspeed->value * CL_KeyState( KB_LEFT );
	}

	cl_viewangles_PITCH -= speed * cl_pitchspeed->value * CL_KeyState( KB_LOOKUP );
	cl_viewangles_PITCH += speed * cl_pitchspeed->value * CL_KeyState( KB_LOOKDOWN );
}

/* ---- CL_KeyMove  0x0040AE90 ----  VERIFIED */
void __cdecl CL_KeyMove( void *cmd )
{
	usercmd_t	*c = (usercmd_t *)cmd;
	int			forward, side, up;

	if ( in_prone_active ) {
		c->wbuttons = (byte)( ( c->wbuttons & ~WBUTTON_STANCE_MASK ) | WBUTTON_PRONE );
		c->wbuttons |= WBUTTON_STANCEHELD;
	} else if ( in_down_active ) {
		c->wbuttons = (byte)( ( c->wbuttons & ~WBUTTON_STANCE_MASK ) | WBUTTON_CROUCH );
		c->wbuttons |= WBUTTON_STANCEHELD;
	} else if ( cl_stance->integer == 1 ) {
		c->wbuttons = (byte)( ( c->wbuttons & ~WBUTTON_STANCE_MASK ) | WBUTTON_CROUCH );
		c->wbuttons &= (byte)~WBUTTON_STANCEHELD;
	} else if ( cl_stance->integer == 2 ) {
		c->wbuttons = (byte)( ( c->wbuttons & ~WBUTTON_STANCE_MASK ) | WBUTTON_PRONE );
		c->wbuttons &= (byte)~WBUTTON_STANCEHELD;
	} else {
		c->wbuttons &= (byte)~WBUTTON_STANCE_MASK;
		c->wbuttons &= (byte)~WBUTTON_STANCEHELD;
	}

	if ( in_speed_active == cl_run->integer ) {
		c->buttons |= BUTTON_WALKING;
	} else {
		c->buttons &= (byte)~BUTTON_WALKING;
	}

	side = 0;
	up = 0;

	if ( in_strafe_active ) {
		side  = (int)( CL_KeyState( KB_RIGHT ) * 127.0f );
		side += (int)( CL_KeyState( KB_LEFT ) * -127.0f );
	}
	side += (int)( CL_KeyState( KB_MOVERIGHT ) * 127.0f );
	side += (int)( CL_KeyState( KB_MOVELEFT ) * -127.0f );

	if ( cl_snap_ps_pm_type == 2 || cl_snap_ps_pm_type == 3 || cl_snap_ps_pm_type == 4 ) {
		up = (int)( CL_KeyState( kbutton ) * 127.0f );
	}
	up += (int)( CL_KeyState( KB_UP ) * 127.0f );

	if ( c->wbuttons & WBUTTON_CROUCH ) {
		if ( in_down_active ) {
			up += (int)( CL_KeyState( KB_DOWN ) * -127.0f );
		} else {
			up -= 127;
		}
	} else if ( c->wbuttons & WBUTTON_PRONE ) {
		up -= 127;
	}

	forward  = (int)( CL_KeyState( KB_FORWARD ) * 127.0f );
	forward += (int)( CL_KeyState( KB_BACK ) * -127.0f );

	if ( ( cl_snap_ps_flags_80 & 0xC000 ) == 0 ) {
		c->forwardmove = ClampChar( forward );
		c->rightmove   = ClampChar( side );
		c->upmove      = ClampChar( up );
	}
}

/* ---- CL_JoystickMove  0x0040B130 ----  VERIFIED */
void __cdecl CL_JoystickMove( void *cmd )
{
	usercmd_t	*c = (usercmd_t *)cmd;
	float		anglespeed;

	if ( in_speed_active == cl_run->integer ) {
		c->buttons |= BUTTON_WALKING;
	}

	anglespeed = (float)cls_frametime;
	if ( in_speed_active ) {
		anglespeed = anglespeed * cl_anglespeedkey->value;
	}
	anglespeed = anglespeed * 0.001f;

	if ( !in_strafe_active ) {
		cl_viewangles_YAW += (float)cl_joystickAxis[0] * cl_yawspeed->value * anglespeed;
	} else {
		c->rightmove = ClampChar( c->rightmove + cl_joystickAxis[0] );
	}

	if ( KB_MLOOK ) {
		cl_viewangles_PITCH += (float)cl_joystickAxis[1] * cl_pitchspeed->value * anglespeed;
	} else {
		c->forwardmove = ClampChar( c->forwardmove + cl_joystickAxis[1] );
	}

	c->upmove = ClampChar( c->upmove + cl_joystickAxis[2] );
}

/* ---- CL_MouseMove  0x0040B240 ----  VERIFIED */
void __cdecl CL_MouseMove( void *cmd )
{
	usercmd_t	*c = (usercmd_t *)cmd;
	float		mx, my;
	float		accelSensitivity;
	float		rate;
	float		delta, maxDelta;

	if ( m_filter->integer ) {
		mx = (float)( cl_mouseDx[0] + cl_mouseDx[1] ) * 0.5f;
		my = (float)( cl_mouseDy[0] + cl_mouseDy[1] ) * 0.5f;
	} else {
		mx = (float)cl_mouseDx[cl_mouseIndex];
		my = (float)cl_mouseDy[cl_mouseIndex];
	}
	cl_mouseIndex ^= 1;
	cl_mouseDx[cl_mouseIndex] = 0;
	cl_mouseDy[cl_mouseIndex] = 0;

	rate = (float)( sqrt( (double)( my * my + mx * mx ) ) / (double)frame_msec );
	accelSensitivity = rate * cl_mouseAccel->value + cl_sensitivity->value;

	accelSensitivity = cgameSensitivity * accelSensitivity;

	if ( rate != 0.0f && cl_showmouserate->integer ) {
		Com_Printf( "%f : %f\n", rate, accelSensitivity );
	}

	if ( cl_snap_ps_pm_flags & 0x4000 ) {
		return;
	}

	if ( cl_snap_ps_flags_80 & 0xC000 ) {
		mx = mx * 2.5f;
		my = my + my;
	} else {
		mx = accelSensitivity * mx;
		my = accelSensitivity * my;
	}

	if ( mx == 0.0f && my == 0.0f ) {
		return;
	}

	if ( in_strafe_active ) {
		c->rightmove = ClampChar( (int)( mx * m_side->value ) + c->rightmove );
	} else {
		delta = mx * m_yaw->value;
		if ( cgameMaxYawSpeed != 0.0f ) {
			maxDelta = (float)frame_msec * cgameMaxYawSpeed * 0.001f;
			if ( delta > maxDelta ) {
				delta = maxDelta;
			}
			if ( delta < -maxDelta ) {
				delta = -maxDelta;
			}
		}
		cl_viewangles_YAW -= delta;
	}

	if ( ( KB_MLOOK || cl_freelook->integer ) && !in_strafe_active ) {
		delta = my * m_pitch->value;
		if ( cgameMaxPitchSpeed != 0.0f ) {
			maxDelta = (float)frame_msec * cgameMaxPitchSpeed * 0.001f;
			if ( delta > maxDelta ) {
				delta = maxDelta;
			}
			if ( delta < -maxDelta ) {
				delta = -maxDelta;
			}
		}
		cl_viewangles_PITCH += delta;
		return;
	}

	c->forwardmove = ClampChar( c->forwardmove - (int)( my * m_forward->value ) );
}

/* ---- CL_CmdButtons  0x0040B600 ----  VERIFIED */
void __cdecl CL_CmdButtons( void *cmd )
{
	static kbutton_t * const in_buttons[7] = {
		IN_BUTTONS_AT( KB_BUTTONS0,  0 ),
		IN_BUTTONS_AT( KB_BUTTONS1,  0 ),
		IN_BUTTONS_AT( KB_WBUTTONS6, 0 ),
		IN_BUTTONS_AT( KB_BUTTONS3,  0 ),
		IN_BUTTONS_AT( KB_BUTTONS4,  0 ),
		IN_BUTTONS_AT( KB_BUTTONS5,  0 ),
		IN_BUTTONS_AT( KB_BUTTONS6,  0 )
	};
	static kbutton_t * const in_wbuttons[7] = {
		IN_BUTTONS_AT( KB_WBUTTONS0, 0 ),
		IN_BUTTONS_AT( KB_WBUTTONS0, 1 ),
		IN_BUTTONS_AT( KB_WBUTTONS1, 0 ),
		IN_BUTTONS_AT( KB_WBUTTONS2, 0 ),
		IN_BUTTONS_AT( KB_WBUTTONS3, 0 ),
		IN_BUTTONS_AT( KB_WBUTTONS4, 0 ),
		IN_BUTTONS_AT( KB_WBUTTONS5, 0 )
	};
	usercmd_t	*c = (usercmd_t *)cmd;
	kbutton_t	*b;
	int			i;

	for ( i = 0 ; i < 7 ; i++ ) {
		b = in_buttons[i];
		if ( b->active || b->wasPressed ) {
			c->buttons |= (byte)( 1 << i );
		}
		b->wasPressed = qfalse;
	}

	for ( i = 0 ; i < 7 ; i++ ) {
		b = in_wbuttons[i];
		if ( b->active || b->wasPressed ) {
			c->wbuttons |= (byte)( 1 << i );
		}
		b->wasPressed = qfalse;
	}

	if ( cls_keyCatchers && !cl_bypassMouseInput->integer ) {
		c->buttons |= BUTTON_TALK;
	}

	if ( cgameUserCmdInShellshock ) {
		c->wbuttons |= WBUTTON_SHELLSHOCK;
	}
}

/* ---- CL_FinishMove  0x0040B690 ----  VERIFIED */
void __cdecl CL_FinishMove( void *cmd )
{
	usercmd_t	*c = (usercmd_t *)cmd;

	c->weapon = (byte)cgameUserCmdValue;

	if ( cl_serverTime - cl_snap_serverTime > 5000 ) {
		c->serverTime = cl_snap_serverTime + 5000;
	} else {
		c->serverTime = cl_serverTime;
	}

	c->angles[0] = (int)( ( cl_viewangles_PITCH + cgameUserAim_PITCH ) * ANGLE2SHORT_SCALE ) & 0xFFFF;
	c->angles[1] = (int)( ( cgameUserAim_YAW + cl_viewangles_YAW ) * ANGLE2SHORT_SCALE ) & 0xFFFF;
	c->angles[2] = (int)( ( cgameUserAim_ROLL + cl_viewangles_ROLL ) * ANGLE2SHORT_SCALE ) & 0xFFFF;
}

/* ---- CL_CreateCmd  0x0040B720 ----  VERIFIED */
usercmd_t __cdecl CL_CreateCmd( void )
{
	usercmd_t	cmd;
	float		oldPitch, oldYaw;
	int			i;

	oldPitch = cl_viewangles_PITCH;
	oldYaw   = cl_viewangles_YAW;

	CL_AdjustAngles();

	memset( &cmd, 0, sizeof( cmd ) );

	CL_CmdButtons( &cmd );

	CL_KeyMove( &cmd );

	CL_MouseMove( &cmd );

	CL_JoystickMove( &cmd );

	if ( cl_viewPitchCompensate->value != 0.0f ) {
		cl_viewangles_PITCH += cl_viewPitchCompensate->value;
		Cvar_Set2( "cl_viewPitchCompensate", "0", qtrue );
	}
	if ( cl_viewYawCompensate->value != 0.0f ) {
		cl_viewangles_YAW += cl_viewYawCompensate->value;
		Cvar_Set2( "cl_viewYawCompensate", "0", qtrue );
	}

	if ( cl_viewangles_PITCH - oldPitch > 90.0f ) {
		cl_viewangles_PITCH = oldPitch + 90.0f;
	} else if ( oldPitch - cl_viewangles_PITCH > 90.0f ) {
		cl_viewangles_PITCH = oldPitch - 90.0f;
	}

	CL_FinishMove( &cmd );

	if ( cl_debugMove_cvar->integer ) {
		if ( cl_debugMove_cvar->integer == 1 ) {
			i = 2 * ( dword_87C7D0 & DEBUGGRAPH_MASK );
			flt_87A7D0[i] = (float)fabs( cl_viewangles_YAW - oldYaw );
			dword_87A7D4[i] = 0;
			dword_87C7D0++;
		}
		if ( cl_debugMove_cvar->integer == 2 ) {
			i = 2 * ( dword_87C7D0 & DEBUGGRAPH_MASK );
			flt_87A7D0[i] = (float)fabs( cl_viewangles_PITCH - oldPitch );
			dword_87A7D4[i] = 0;
			dword_87C7D0++;
		}
	}

	return cmd;
}

/* ---- CL_CreateNewCommands  0x0040B8D0 ----  VERIFIED */
void __cdecl CL_CreateNewCommands( void )
{
	int		cmdNum;

	if ( cls_state < 5 ) {
		return;
	}

	frame_msec = com_frameTime - old_com_frameTime;

	if ( frame_msec > 200 ) {
		frame_msec = 200;
	}
	old_com_frameTime = com_frameTime;

	cl_cmdNumber++;
	cmdNum = cl_cmdNumber & CMD_MASK;
	cl_cmds[cmdNum] = CL_CreateCmd();
}

/* ---- CL_ReadyToSendPacket  0x0040B940 ----  VERIFIED */
qboolean __cdecl CL_ReadyToSendPacket( void )
{
	int		oldPacketNum;
	int		delta;

	if ( clc_demoplaying || cls_state == 7 || cls_state == 8 ) {
		return qfalse;
	}

	if ( cls_downloadTempName[0] && cls_realtime - clc_lastPacketSentTime < 50 ) {
		return qfalse;
	}

	if ( cls_state != 6
		 && cls_state != 5
		 && !cls_downloadTempName[0]
		 && cls_realtime - clc_lastPacketSentTime < 1000 ) {
		return qfalse;
	}

	if ( clc_netchan.remoteAddress.type == NA_LOOPBACK ) {
		return qtrue;
	}
	if ( Sys_IsLANAddress( clc_netchan.remoteAddress ) ) {
		return qtrue;
	}

	if ( cl_maxpackets->integer < 15 ) {
		Cvar_Set2( "cl_maxpackets", "15", qtrue );
	} else if ( cl_maxpackets->integer > 100 ) {
		Cvar_Set2( "cl_maxpackets", "100", qtrue );
	}

	oldPacketNum = ( clc_netchan.outgoingSequence - 1 ) & PACKET_MASK;
	delta = cls_realtime - OUTPACKET_REALTIME( oldPacketNum );
	if ( delta < 1000 / cl_maxpackets->integer ) {
		return qfalse;
	}

	return qtrue;
}

/* ---- CL_WritePacket  0x0040BA50 ----  VERIFIED */
void __cdecl CL_WritePacket( void )
{
	msg_t		msg;
	byte		data[MAX_MSGLEN];
	byte		packet[MAX_MSGLEN];
	usercmd_t	nullcmd;
	usercmd_t	*cmd, *oldcmd;
	int			i, j;
	int			packetNum, oldPacketNum;
	int			count, key, size;

	if ( clc_demoplaying || cls_state == 7 || cls_state == 8 ) {
		return;
	}

	MSG_SetDefaultUserCmd( cl_snap_ps, &nullcmd );
	oldcmd = &nullcmd;

	if ( msgInit == qfalse ) {
		MSG_initHuffman();
	}
	memset( &msg, 0, sizeof( msg ) );
	msg.data = data;
	msg.maxsize = clc_netchan.extended ? MAX_MSGLEN : STOCK_MAX_MSGLEN;
	msg.extended = clc_netchan.extended;

	MSG_WriteByte( &msg, (byte)cl_serverId );

	MSG_WriteLong( &msg, clc_serverMessageSequence );

	MSG_WriteLong( &msg, clc_serverCommandSequence );

	for ( i = clc_reliableAcknowledge + 1 ; i <= clc_reliableSequence ; i++ ) {
		MSG_WriteBits( &msg, clc_clientCommand, 2 );
		MSG_WriteLong( &msg, i );
		MSG_WriteString( clc_reliableCommands( i ), &msg );
	}

	if ( cl_packetdup->integer < 0 ) {
		Cvar_Set2( "cl_packetdup", "0", qtrue );
	} else if ( cl_packetdup->integer > 5 ) {
		Cvar_Set2( "cl_packetdup", "5", qtrue );
	}

	oldPacketNum = ( clc_netchan.outgoingSequence - cl_packetdup->integer - 1 ) & PACKET_MASK;
	count = cl_cmdNumber - OUTPACKET_CMDNUMBER( oldPacketNum );
	if ( count > MAX_PACKET_USERCMDS ) {
		count = MAX_PACKET_USERCMDS;
		Com_Printf( "MAX_PACKET_USERCMDS\n" );
	}

	if ( count >= 1 ) {
		if ( cl_showSend->integer ) {
			Com_Printf( "(%i)", count );
		}

		if ( cl_nodelta->integer
			 || !cl_snap_valid
			 || clc_demowaiting
			 || clc_serverMessageSequence != *(int *)cl_snap_messageNum ) {
			MSG_WriteBits( &msg, clc_moveNoDelta, 2 );
		} else {
			MSG_WriteBits( &msg, clc_move, 2 );
		}

		MSG_WriteByte( &msg, count );

		key = clc_checksumFeed;
		key ^= clc_serverMessageSequence;
		key ^= Com_HashKey( clc_serverCommand( clc_serverCommandSequence ), 32 );

		for ( i = 0 ; i < count ; i++ ) {
			j = ( cl_cmdNumber - count + i + 1 ) & CMD_MASK;
			cmd = &cl_cmds[j];
			MSG_WriteDeltaUsercmdKey( oldcmd, cmd, key, &msg );
			oldcmd = cmd;
		}
	}

	MSG_WriteBits( &msg, clc_EOF, 2 );

	Com_Memcpy( packet, msg.data, CL_ENCODE_START );
	size = MSG_WriteBitsCompress( msg.data + CL_ENCODE_START,
								  msg.cursize - CL_ENCODE_START,
								  packet + CL_ENCODE_START, sizeof( packet ) - CL_ENCODE_START );

	packetNum = clc_netchan.outgoingSequence & PACKET_MASK;
	OUTPACKET_REALTIME( packetNum )  = cls_realtime;
	OUTPACKET_SERVERTIME( packetNum ) = oldcmd->serverTime;
	OUTPACKET_CMDNUMBER( packetNum ) = cl_cmdNumber;
	clc_lastPacketSentTime = cls_realtime;

	size += CL_ENCODE_START;
	if ( cl_showSend->integer ) {
		Com_Printf( "%i ", size );
	}

	CL_Netchan_Encode( packet + CL_ENCODE_START, size - CL_ENCODE_START );
	Netchan_Transmit( &clc_netchan, size, packet );

	while ( clc_netchan.unsentFragments ) {
		Netchan_TransmitNextFragment( &clc_netchan );
	}
}

/* ---- CL_SendCmd  0x0040BF30 ----  VERIFIED */
void __cdecl CL_SendCmd( void )
{
	if ( cls_state < 3 ) {
		return;
	}

	if ( com_sv_running->integer && sv_paused->integer == 1 && cl_paused->integer == 1 ) {
		return;
	}

	CL_CreateNewCommands();

	if ( !CL_ReadyToSendPacket() ) {
		if ( cl_showSend->integer ) {
			Com_Printf( ". " );
		}
		return;
	}

	CL_WritePacket();
}

/* ---- CL_InitInput  0x0040BF90 ----  VERIFIED */
void __cdecl CL_InitInput( void )
{
  Cmd_AddCommand("centerview", IN_CenterView);
  Cmd_AddCommand("+moveup", IN_UpDown);
  Cmd_AddCommand("-moveup", IN_UpUp);
  Cmd_AddCommand("+movedown", IN_DownDown);
  Cmd_AddCommand("-movedown", IN_DownUp);
  Cmd_AddCommand("+left", IN_LeftDown);
  Cmd_AddCommand("-left", IN_LeftUp);
  Cmd_AddCommand("+right", IN_RightDown);
  Cmd_AddCommand("-right", IN_RightUp);
  Cmd_AddCommand("+forward", IN_ForwardDown);
  Cmd_AddCommand("-forward", IN_ForwardUp);
  Cmd_AddCommand("+back", IN_BackDown);
  Cmd_AddCommand("-back", IN_BackUp);
  Cmd_AddCommand("+lookup", IN_LookupDown);
  Cmd_AddCommand("-lookup", IN_LookupUp);
  Cmd_AddCommand("+lookdown", IN_LookdownDown);
  Cmd_AddCommand("-lookdown", IN_LookdownUp);
  Cmd_AddCommand("+strafe", IN_StrafeDown);
  Cmd_AddCommand("-strafe", IN_StrafeUp);
  Cmd_AddCommand("+moveleft", IN_MoveleftDown);
  Cmd_AddCommand("-moveleft", IN_MoveleftUp);
  Cmd_AddCommand("+moveright", IN_MoverightDown);
  Cmd_AddCommand("-moveright", IN_MoverightUp);
  Cmd_AddCommand("+speed", IN_SpeedDown);
  Cmd_AddCommand("-speed", IN_SpeedUp);
  Cmd_AddCommand("+attack", IN_AttackDown);
  Cmd_AddCommand("-attack", IN_AttackUp);
  Cmd_AddCommand("+melee", IN_MeleeDown);
  Cmd_AddCommand("-melee", IN_MeleeUp);
  Cmd_AddCommand("+activate", IN_ActivateDown);
  Cmd_AddCommand("-activate", IN_ActivateUp);
  Cmd_AddCommand("+reload", IN_ReloadDown);
  Cmd_AddCommand("-reload", IN_ReloadUp);
  Cmd_AddCommand("+leanleft", IN_LeanLeftDown);
  Cmd_AddCommand("-leanleft", IN_LeanLeftUp);
  Cmd_AddCommand("+leanright", IN_LeanRightDown);
  Cmd_AddCommand("-leanright", IN_LeanRightUp);
  Cmd_AddCommand("+dropweapon", IN_MP_DropWeaponDown);
  Cmd_AddCommand("-dropweapon", IN_MP_DropWeaponUp);
  Cmd_AddCommand("+prone", IN_ProneDown);
  Cmd_AddCommand("-prone", IN_ProneUp);
  Cmd_AddCommand("+mlook", IN_MLookDown);
  Cmd_AddCommand("-mlook", IN_MLookUp);
  Cmd_AddCommand("lowerstance", IN_LowerStance);
  Cmd_AddCommand("raisestance", IN_RaiseStance);
  Cmd_AddCommand("togglecrouch", IN_ToggleCrouch);
  Cmd_AddCommand("toggleprone", IN_ToggleProne);
  Cmd_AddCommand("goprone", IN_GoProne);
  Cmd_AddCommand("gocrouch", IN_GoCrouch);
  Cmd_AddCommand("+gostand", IN_GoStandDown);
  Cmd_AddCommand("-gostand", IN_GoStandUp);
  cl_nodelta = Cvar_Get("cl_nodelta", "0", 0);
  cl_debugMove = (int)Cvar_Get("cl_debugMove", "0", 0);
}

/* ---- CL_ShutdownInput  0x0040C2D0 ----  VERIFIED */
void __cdecl CL_ShutdownInput( void )
{
  Cmd_RemoveCommand("centerview");
  Cmd_RemoveCommand("+moveup");
  Cmd_RemoveCommand("-moveup");
  Cmd_RemoveCommand("+movedown");
  Cmd_RemoveCommand("-movedown");
  Cmd_RemoveCommand("+left");
  Cmd_RemoveCommand("-left");
  Cmd_RemoveCommand("+right");
  Cmd_RemoveCommand("-right");
  Cmd_RemoveCommand("+forward");
  Cmd_RemoveCommand("-forward");
  Cmd_RemoveCommand("+back");
  Cmd_RemoveCommand("-back");
  Cmd_RemoveCommand("+lookup");
  Cmd_RemoveCommand("-lookup");
  Cmd_RemoveCommand("+lookdown");
  Cmd_RemoveCommand("-lookdown");
  Cmd_RemoveCommand("+strafe");
  Cmd_RemoveCommand("-strafe");
  Cmd_RemoveCommand("+moveleft");
  Cmd_RemoveCommand("-moveleft");
  Cmd_RemoveCommand("+moveright");
  Cmd_RemoveCommand("-moveright");
  Cmd_RemoveCommand("+speed");
  Cmd_RemoveCommand("-speed");
  Cmd_RemoveCommand("+attack");
  Cmd_RemoveCommand("-attack");
  Cmd_RemoveCommand("+melee");
  Cmd_RemoveCommand("-melee");
  Cmd_RemoveCommand("+activate");
  Cmd_RemoveCommand("-activate");
  Cmd_RemoveCommand("+reload");
  Cmd_RemoveCommand("-reload");
  Cmd_RemoveCommand("+leanleft");
  Cmd_RemoveCommand("-leanleft");
  Cmd_RemoveCommand("+leanright");
  Cmd_RemoveCommand("-leanright");
  Cmd_RemoveCommand("+dropweapon");
  Cmd_RemoveCommand("-dropweapon");
  Cmd_RemoveCommand("+prone");
  Cmd_RemoveCommand("-prone");
  Cmd_RemoveCommand("+mlook");
  Cmd_RemoveCommand("-mlook");
  Cmd_RemoveCommand("lowerstance");
  Cmd_RemoveCommand("raisestance");
  Cmd_RemoveCommand("togglecrouch");
  Cmd_RemoveCommand("toggleprone");
  Cmd_RemoveCommand("goprone");
  Cmd_RemoveCommand("gocrouch");
  Cmd_RemoveCommand("+gostand");
  Cmd_RemoveCommand("-gostand");
}

/* ---- CL_ClearKeys  0x0040C4E0 ----  VERIFIED */
void __cdecl CL_ClearKeys( void )
{
	memset( KB_LEFT,      0, 24 );	/* 0x0087A028 */
	memset( KB_RIGHT,     0, 24 );	/* 0x0087A040 */
	memset( KB_FORWARD,   0, 24 );	/* 0x0087A058 */
	memset( KB_BACK,      0, 24 );	/* 0x0087A070 */
	memset( KB_LOOKUP,    0, 24 );	/* 0x0087A088 */
	memset( KB_LOOKDOWN,  0, 24 );	/* 0x0087A0A0 */
	memset( KB_MOVELEFT,  0, 24 );	/* 0x0087A0B8 */
	memset( KB_MOVERIGHT, 0, 24 );	/* 0x0087A0D0 */
	memset( KB_STRAFE,    0, 24 );	/* 0x0087A0E8 */
	memset( KB_SPEED,     0, 24 );	/* 0x0087A100 */
	memset( KB_UP,        0, 24 );	/* 0x0087A118 */
	memset( KB_DOWN,      0, 24 );	/* 0x0087A130 */
	memset( kbutton,      0, 24 );	/* 0x0087A148 the raw +moveup key */
	memset( KB_BUTTONS0,  0, 24 );	/* 0x0087A160 */
	memset( KB_BUTTONS1,  0, 24 );	/* 0x0087A178 */
	memset( KB_WBUTTONS6, 0, 24 );	/* 0x0087A190 */
	memset( KB_BUTTONS3,  0, 24 );	/* 0x0087A1A8 */
	memset( KB_BUTTONS4,  0, 24 );	/* 0x0087A1C0 */
	memset( KB_BUTTONS5,  0, 24 );	/* 0x0087A1D8 */
	memset( KB_BUTTONS6,  0, 48 );	/* 0x0087A1F0 + the unnamed 0x0087A208 */
	memset( KB_WBUTTONS0, 0, 48 );	/* 0x0087A220 + the unnamed 0x0087A238 */
	memset( KB_WBUTTONS1, 0, 24 );	/* 0x0087A250 */
	memset( KB_WBUTTONS2, 0, 24 );	/* 0x0087A268 */
	memset( KB_WBUTTONS3, 0, 24 );	/* 0x0087A280 */
	memset( KB_WBUTTONS4, 0, 24 );	/* 0x0087A298 */
	memset( KB_WBUTTONS5, 0, 24 );	/* 0x0087A2B0 */
	memset( KB_WBUTTONS7, 0, 40 );	/* 0x0087A2C8 .. 0x0087A2F0 */
	KB_MLOOK = qfalse;				/* 0x0087A2F0 in_mlooking.active */
}

#define cl_mouseDx      cl_mouseDx
#define cl_mouseDy      cl_mouseDy
#define cl_mouseIndex   cl_mouseIndex
#define cl_joystickAxis cl_joystickAxis

/* ---- CL_MouseEvent  0x0040B0A0 ----  [CONFIRMED] */
void CL_MouseEvent( int dx, int dy, int time ) {
	(void)time;

	if ( cls_keyCatchers & KEYCATCH_UI ) {
		if ( cl_bypassMouseInput && cl_bypassMouseInput->integer == 1 ) {
		} else {
			if ( uivm ) {
				VM_Call( uivm, UI_MOUSE_EVENT, dx, dy );
			}
			return;
		}
	} else if ( cls_keyCatchers & KEYCATCH_CGAME ) {
		if ( cgvm ) {
			VM_Call( cgvm, CG_MOUSE_EVENT, dx, dy );
		}
		return;
	}

	cl_mouseDx[cl_mouseIndex] += dx;
	cl_mouseDy[cl_mouseIndex] += dy;
}

/* ---- CL_JoystickEvent  0x0040B100 ----  [CONFIRMED] */
void CL_JoystickEvent( int axis, int value, int time ) {
	(void)time;

	if ( axis < 0 || axis >= 6 ) {
		Com_Error( ERR_DROP, "CL_JoystickEvent: bad axis %i", axis );
		return;
	}
	cl_joystickAxis[axis] = value;
}
