/*
 * qcommon/msg_mp.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/msg_mp.c
 *
 * Retail range 0x004446E0-0x00448390, 60 functions.
 *
 * @fidelity: verified
 */

#include "qcommon.h"

huffman_t msgHuff;

static char msg_stringBuf[MAX_STRING_CHARS];
static char msg_bigStringBuf[BIG_INFO_STRING];
static char msg_stringLineBuf[MAX_STRING_CHARS];

extern cvar_t *cl_shownet;

unsigned int kbitmask[33] = {
	0x00000000u, 0x00000001u, 0x00000003u, 0x00000007u,
	0x0000000fu, 0x0000001fu, 0x0000003fu, 0x0000007fu,
	0x000000ffu, 0x000001ffu, 0x000003ffu, 0x000007ffu,
	0x00000fffu, 0x00001fffu, 0x00003fffu, 0x00007fffu,
	0x0000ffffu, 0x0001ffffu, 0x0003ffffu, 0x0007ffffu,
	0x000fffffu, 0x001fffffu, 0x003fffffu, 0x007fffffu,
	0x00ffffffu, 0x01ffffffu, 0x03ffffffu, 0x07ffffffu,
	0x0fffffffu, 0x1fffffffu, 0x3fffffffu, 0x7fffffffu,
	0xffffffffu
};

int msg_hData[256] = {
	250315,  41193,   6292,   7106,   3730,   3750,   6110,  23283,
	 33317,   6950,   7838,   9714,   9257,  17259,   3949,   1778,
	  8288,   1604,   1590,   1663,   1100,   1213,   1238,   1134,
	  1749,   1059,   1246,   1149,   1273,   4486,   2805,   3472,
	 21819,   1159,   1670,   1066,   1043,   1012,   1053,   1070,
	  1726,    888,   1180,    850,    960,    780,   1752,   3296,
	 10630,   4514,   5881,   2685,   4650,   3837,   2093,   1867,
	  2584,   1949,   1972,    940,   1134,   1788,   1670,   1206,
	  5719,   6128,   7222,   6654,   3710,   3795,   1492,   1524,
	  2215,   1140,   1355,    971,   2180,   1248,   1328,   1195,
	  1770,   1078,   1264,   1266,   1168,    965,   1155,   1186,
	  1347,   1228,   1529,   1600,   2617,   2048,   2546,   3275,
	  2410,   3585,   2504,   2800,   2675,   6146,   3663,   2840,
	 14253,   3164,   2221,   1687,   3208,   2739,   3512,   4796,
	  4091,   3515,   5288,   4016,   7937,   6031,   5360,   3924,
	  4892,   3743,   4566,   4807,   5852,   6400,   6225,   8291,
	 23243,   7838,   7073,   8935,   5437,   4483,   3641,   5256,
	  5312,   5328,   5370,   3492,   2458,   1694,   1821,   2121,
	  1916,   1149,   1516,   1367,   1236,   1029,   1258,   1104,
	  1245,   1006,   1149,   1025,   1241,    952,   1287,    997,
	  1713,   1009,   1187,    879,   1099,    929,   1078,    951,
	  1656,    930,   1153,   1030,   1262,   1062,   1214,   1060,
	  1621,    930,   1106,    912,   1034,    892,   1158,    990,
	  1175,    850,   1121,    903,   1087,    920,   1144,   1056,
	  3462,   2240,   4397,  12136,   7758,   1345,   1307,   3278,
	  1950,    886,   1023,   1112,   1077,   1042,   1061,   1071,
	  1484,   1001,   1096,    915,   1052,    995,   1070,    876,
	  1111,    851,   1059,    805,   1112,    923,   1103,    817,
	  1899,   1872,    976,    841,   1127,    956,   1159,    950,
	  7791,    954,   1289,    933,   1127,   3207,   1020,    927,
	  1355,    768,   1040,    745,    952,    805,   1073,    740,
	  1013,    805,   1008,    796,    996,   1057,  11457,  13504
};

#define MSG_ENTITY_FIELDS       59
#define MSG_ENTITY_BASE_FIELDS  67
#define MSG_CLIENT_FIELDS       22
#define MSG_PLAYERSTATE_FIELDS  103
#define MSG_OBJECTIVE_FIELDS     6
#define MSG_HUDELEM_FIELDS      28

#define MSG_ENTITY_NUM_BITS     10
#define MSG_CLIENT_NUM_BITS      6

netField_t stateFields[MSG_ENTITY_FIELDS] = {
	{ "pos.trTime",         16, 32 },
	{ "pos.trBase[0]",      24,  0 },
	{ "pos.trBase[1]",      28,  0 },
	{ "pos.trDelta[1]",     40,  0 },
	{ "pos.trDelta[0]",     36,  0 },
	{ "angles2[1]",        108,  0 },
	{ "apos.trBase[1]",     64,  0 },
	{ "apos.trBase[0]",     60,  0 },
	{ "pos.trBase[2]",      32,  0 },
	{ "pos.trDelta[2]",     44,  0 },
	{ "eventSequence",     164,  8 },
	{ "eType",               4,  8 },
	{ "eFlags",              8, 24 },
	{ "groundEntityNum",   124, 10 },
	{ "legsAnim",          204, 10 },
	{ "clientNum",         144,  8 },
	{ "apos.trBase[2]",     68,  0 },
	{ "events[0]",         168,  8 },
	{ "index",             140,  9 },
	{ "events[1]",         172,  8 },
	{ "events[2]",         176,  8 },
	{ "events[3]",         180,  8 },
	{ "eventParm",         160,  8 },
	{ "torsoAnim",         208, 10 },
	{ "surfType",          136,  8 },
	{ "scale",             216,  8 },
	{ "otherEntityNum",    116, 10 },
	{ "fWaistPitch",       236,  0 },
	{ "pos.trType",         12,  8 },
	{ "angles2[0]",        104,  0 },
	{ "fTorsoHeight",      228,  0 },
	{ "fTorsoPitch",       232,  0 },
	{ "apos.trType",        48,  8 },
	{ "solid",             156, 24 },
	{ "weapon",            200,  6 },
	{ "apos.trTime",        52, 32 },
	{ "apos.trDelta[0]",    72,  0 },
	{ "eventParms[0]",     184,  8 },
	{ "pos.trDuration",     20, 32 },
	{ "animMovetype",      224,  4 },
	{ "eventParms[1]",     188,  8 },
	{ "apos.trDelta[2]",    80,  0 },
	{ "eventParms[2]",     192,  8 },
	{ "eventParms[3]",     196,  8 },
	{ "leanf",             212,  0 },
	{ "apos.trDelta[1]",    76,  0 },
	{ "loopSound",         132,  8 },
	{ "attackerEntityNum", 120, 10 },
	{ "iHeadIcon",         148,  4 },
	{ "iHeadIconTeam",     152,  2 },
	{ "apos.trDuration",    56, 32 },
	{ "time",               84, 32 },
	{ "time2",              88, 32 },
	{ "origin2[0]",         92,  0 },
	{ "origin2[1]",         96,  0 },
	{ "origin2[2]",        100,  0 },
	{ "angles2[2]",        112,  0 },
	{ "constantLight",     128, 32 },
	{ "dmgFlags",          220, 32 }
};

netField_t archivedEntityFields[MSG_ENTITY_BASE_FIELDS] = {
	{ "absmin[1]",         252,  0 },
	{ "absmax[1]",         264,  0 },
	{ "absmin[0]",         248,  0 },
	{ "absmax[0]",         260,  0 },
	{ "absmin[2]",         256,  0 },
	{ "absmax[2]",         268,  0 },
	{ "pos.trBase[1]",      28,  0 },
	{ "pos.trBase[0]",      24,  0 },
	{ "eType",               4,  8 },
	{ "eFlags",              8, 24 },
	{ "pos.trBase[2]",      32,  0 },
	{ "svFlags",           240, 32 },
	{ "groundEntityNum",   124, 10 },
	{ "apos.trBase[1]",     64,  0 },
	{ "clientNum",         144,  8 },
	{ "apos.trBase[0]",     60,  0 },
	{ "index",             140,  9 },
	{ "apos.trBase[2]",     68,  0 },
	{ "eventSequence",     164,  8 },
	{ "events[0]",         168,  8 },
	{ "legsAnim",          204, 10 },
	{ "events[1]",         172,  8 },
	{ "events[2]",         176,  8 },
	{ "events[3]",         180,  8 },
	{ "weapon",            200,  6 },
	{ "pos.trType",         12,  8 },
	{ "pos.trTime",         16, 32 },
	{ "apos.trType",        48,  8 },
	{ "solid",             156, 24 },
	{ "pos.trDuration",     20, 32 },
	{ "eventParms[0]",     184,  8 },
	{ "torsoAnim",         208, 10 },
	{ "pos.trDelta[0]",     36,  0 },
	{ "pos.trDelta[1]",     40,  0 },
	{ "angles2[1]",        108,  0 },
	{ "angles2[0]",        104,  0 },
	{ "animMovetype",      224,  4 },
	{ "pos.trDelta[2]",     44,  0 },
	{ "otherEntityNum",    116, 10 },
	{ "eventParms[1]",     188,  8 },
	{ "surfType",          136,  8 },
	{ "eventParm",         160,  8 },
	{ "eventParms[2]",     192,  8 },
	{ "scale",             216,  8 },
	{ "eventParms[3]",     196,  8 },
	{ "fTorsoHeight",      228,  0 },
	{ "fWaistPitch",       236,  0 },
	{ "fTorsoPitch",       232,  0 },
	{ "apos.trTime",        52, 32 },
	{ "apos.trDelta[0]",    72,  0 },
	{ "apos.trDelta[2]",    80,  0 },
	{ "singleClient",      244,  6 },
	{ "leanf",             212,  0 },
	{ "apos.trDelta[1]",    76,  0 },
	{ "loopSound",         132,  8 },
	{ "attackerEntityNum", 120, 10 },
	{ "iHeadIcon",         148,  4 },
	{ "iHeadIconTeam",     152,  2 },
	{ "apos.trDuration",    56, 32 },
	{ "time",               84, 32 },
	{ "time2",              88, 32 },
	{ "origin2[0]",         92,  0 },
	{ "origin2[1]",         96,  0 },
	{ "origin2[2]",        100,  0 },
	{ "angles2[2]",        112,  0 },
	{ "constantLight",     128, 32 },
	{ "dmgFlags",          220, 32 }
};

netField_t clientStateFields[MSG_CLIENT_FIELDS] = {
	{ "team",                 4,  2 },
	{ "name[0]",             60, 32 },
	{ "name[4]",             64, 32 },
	{ "attachModelIndex[0]", 12,  8 },
	{ "attachModelIndex[2]", 20,  8 },
	{ "attachModelIndex[3]", 24,  8 },
	{ "modelindex",           8,  8 },
	{ "attachModelIndex[1]", 16,  8 },
	{ "name[8]",             68, 32 },
	{ "name[12]",            72, 32 },
	{ "attachModelIndex[4]", 28,  8 },
	{ "name[16]",            76, 32 },
	{ "attachTagIndex[0]",   36,  5 },
	{ "attachTagIndex[1]",   40,  5 },
	{ "attachTagIndex[2]",   44,  5 },
	{ "attachTagIndex[3]",   48,  5 },
	{ "attachTagIndex[4]",   52,  5 },
	{ "attachModelIndex[5]", 32,  8 },
	{ "attachTagIndex[5]",   56,  5 },
	{ "name[20]",            80, 32 },
	{ "name[24]",            84, 32 },
	{ "name[28]",            88, 32 }
};

static netField_t msg_playerStateFields[MSG_PLAYERSTATE_FIELDS] = {
	{ "commandTime",             offsetof( playerState_t, commandTime), 32 },
	{ "origin[1]",              offsetof( playerState_t, origin[1]),  0 },
	{ "origin[0]",              offsetof( playerState_t, origin[0]),  0 },
	{ "origin[2]",              offsetof( playerState_t, origin[2]),  0 },
	{ "viewangles[1]",         offsetof( playerState_t, viewangles[1]),  0 },
	{ "viewangles[0]",         offsetof( playerState_t, viewangles[0]),  0 },
	{ "iCompassFriendInfo",    offsetof( playerState_t, iCompassFriendInfo), 32 },
	{ "eFlags",                offsetof( playerState_t, eFlags), 24 },
	{ "bobCycle",                offsetof( playerState_t, bobCycle),  8 },
	{ "viewHeightCurrent",     offsetof( playerState_t, viewHeightCurrent),  0 },
	{ "eventSequence",         offsetof( playerState_t, eventSequence),  8 },
	{ "legsAnim",              offsetof( playerState_t, legsAnim), 10 },
	{ "pm_flags",               offsetof( playerState_t, pm_flags), 19 },
	{ "delta_angles[1]",        offsetof( playerState_t, delta_angles[1]), 16 },
	{ "velocity[1]",            offsetof( playerState_t, velocity[1]),  0 },
	{ "velocity[0]",            offsetof( playerState_t, velocity[0]),  0 },
	{ "speed",                  offsetof( playerState_t, speed), 16 },
	{ "mins[0]",               offsetof( playerState_t, mins[0]),  0 },
	{ "mins[1]",               offsetof( playerState_t, mins[1]),  0 },
	{ "maxs[0]",               offsetof( playerState_t, maxs[0]),  0 },
	{ "maxs[1]",               offsetof( playerState_t, maxs[1]),  0 },
	{ "maxs[2]",               offsetof( playerState_t, maxs[2]),  0 },
	{ "proneViewHeight",       offsetof( playerState_t, proneViewHeight), -8 },
	{ "crouchViewHeight",      offsetof( playerState_t, crouchViewHeight), -8 },
	{ "standViewHeight",       offsetof( playerState_t, standViewHeight), -8 },
	{ "deadViewHeight",        offsetof( playerState_t, deadViewHeight), -8 },
	{ "walkSpeedScale",        offsetof( playerState_t, walkSpeedScale),  0 },
	{ "runSpeedScale",         offsetof( playerState_t, runSpeedScale),  0 },
	{ "proneSpeedScale",       offsetof( playerState_t, proneSpeedScale),  0 },
	{ "crouchSpeedScale",      offsetof( playerState_t, crouchSpeedScale),  0 },
	{ "strafeSpeedScale",      offsetof( playerState_t, strafeSpeedScale),  0 },
	{ "backSpeedScale",        offsetof( playerState_t, backSpeedScale),  0 },
	{ "leanSpeedScale",        offsetof( playerState_t, leanSpeedScale),  0 },
	{ "friction",              offsetof( playerState_t, friction),  0 },
	{ "groundEntityNum",        offsetof( playerState_t, groundEntityNum), 10 },
	{ "viewHeightTarget",      offsetof( playerState_t, viewHeightTarget), -8 },
	{ "events[0]",             offsetof( playerState_t, events[0]),  8 },
	{ "weapons[0]",            offsetof( playerState_t, weapons[0]), 32 },
	{ "weaponslots[0]",        offsetof( playerState_t, weaponslots[0]), 32 },
	{ "gravity",                offsetof( playerState_t, gravity), 16 },
	{ "serverCursorHintString", offsetof( playerState_t, serverCursorHintString), -8 },
	{ "viewmodelIndex",        offsetof( playerState_t, viewmodelIndex),  8 },
	{ "events[1]",             offsetof( playerState_t, events[1]),  8 },
	{ "events[2]",             offsetof( playerState_t, events[2]),  8 },
	{ "events[3]",             offsetof( playerState_t, events[3]),  8 },
	{ "delta_angles[0]",        offsetof( playerState_t, delta_angles[0]), 16 },
	{ "weapon",                offsetof( playerState_t, weapon),  6 },
	{ "movementDir",           offsetof( playerState_t, movementDir),  8 },
	{ "viewHeightLerpTarget",  offsetof( playerState_t, viewHeightLerpTarget), -8 },
	{ "weaponslots[4]",        offsetof( playerState_t, weaponslots[4]), 32 },
	{ "proneDirection",        offsetof( playerState_t, proneDirection),  0 },
	{ "aimSpreadScale",        offsetof( playerState_t, aimSpreadScale),  0 },
	{ "weapAnim",              offsetof( playerState_t, weapAnim), 10 },
	{ "jumpTime",              offsetof( playerState_t, jumpTime), 32 },
	{ "velocity[2]",            offsetof( playerState_t, velocity[2]),  0 },
	{ "weaponTime",             offsetof( playerState_t, weaponTime), -16 },
	{ "proneTorsoPitch",       offsetof( playerState_t, proneTorsoPitch),  0 },
	{ "proneDirectionPitch",   offsetof( playerState_t, proneDirectionPitch),  0 },
	{ "fTorsoPitch",           offsetof( playerState_t, fTorsoPitch),  0 },
	{ "fWaistPitch",           offsetof( playerState_t, fWaistPitch),  0 },
	{ "fTorsoHeight",          offsetof( playerState_t, fTorsoHeight),  0 },
	{ "weaponstate",           offsetof( playerState_t, weaponstate),  4 },
	{ "torsoTimer",            offsetof( playerState_t, torsoTimer), 16 },
	{ "torsoAnim",             offsetof( playerState_t, torsoAnim), 10 },
	{ "eventParms[0]",         offsetof( playerState_t, eventParms[0]),  8 },
	{ "vLadderVec[0]",          offsetof( playerState_t, vLadderVec[0]),  0 },
	{ "eventParms[3]",         offsetof( playerState_t, eventParms[3]),  8 },
	{ "viewHeightLerpDown",    offsetof( playerState_t, viewHeightLerpDown),  1 },
	{ "weaponDelay",            offsetof( playerState_t, weaponDelay), -16 },
	{ "eventParms[1]",         offsetof( playerState_t, eventParms[1]),  8 },
	{ "viewHeightLerpTime",    offsetof( playerState_t, viewHeightLerpTime), 32 },
	{ "eventParms[2]",         offsetof( playerState_t, eventParms[2]),  8 },
	{ "vLadderVec[1]",          offsetof( playerState_t, vLadderVec[1]),  0 },
	{ "fWeaponPosFrac",        offsetof( playerState_t, fWeaponPosFrac),  0 },
	{ "pm_type",                 offsetof( playerState_t, pm_type),  8 },
	{ "legsTimer",             offsetof( playerState_t, legsTimer), 16 },
	{ "fJumpPeak",             offsetof( playerState_t, fJumpOriginZ),  0 },
	{ "leanf",                  offsetof( playerState_t, leanf),  0 },
	{ "damageEvent",           offsetof( playerState_t, damageEvent),  8 },
	{ "damageYaw",             offsetof( playerState_t, damageYaw),  8 },
	{ "damagePitch",           offsetof( playerState_t, damagePitch),  8 },
	{ "damageCount",           offsetof( playerState_t, damageCount),  7 },
	{ "weaponrechamber[0]",    offsetof( playerState_t, weaponrechamber[0]), 32 },
	{ "grenadeTimeLeft",        offsetof( playerState_t, grenadeTimeLeft), -16 },
	{ "pm_time",                offsetof( playerState_t, pm_time), -16 },
	{ "iFoliageSoundTime",      offsetof( playerState_t, iFoliageSoundTime), 32 },
	{ "deltaTime",            offsetof( playerState_t, deltaTime), 32 },
	{ "serverCursorHint",      offsetof( playerState_t, serverCursorHint),  8 },
	{ "serverCursorHintVal",   offsetof( playerState_t, serverCursorHintVal),  8 },
	{ "shellshockIndex",       offsetof( playerState_t, shellshockIndex),  4 },
	{ "shellshockTime",        offsetof( playerState_t, shellshockTime), 32 },
	{ "shellshockDuration",    offsetof( playerState_t, shellshockDuration), 16 },
	{ "delta_angles[2]",        offsetof( playerState_t, delta_angles[2]), 16 },
	{ "vLadderVec[2]",          offsetof( playerState_t, vLadderVec[2]),  0 },
	{ "clientNum",             offsetof( playerState_t, clientNum),  8 },
	{ "weapons[1]",            offsetof( playerState_t, weapons[1]), 32 },
	{ "weaponrechamber[1]",    offsetof( playerState_t, weaponrechamber[1]), 32 },
	{ "viewangles[2]",         offsetof( playerState_t, viewangles[2]),  0 },
	{ "viewHeightLerpPosAdj",  offsetof( playerState_t, viewHeightLerpPosAdj),  0 },
	{ "mins[2]",               offsetof( playerState_t, mins[2]),  0 },
	{ "viewlocked",            offsetof( playerState_t, viewlocked),  8 },
	{ "viewlocked_entNum",     offsetof( playerState_t, viewlocked_entNum), 16 },
	{ "gunfx",                 offsetof( playerState_t, gunfx),  8 }
};

static netField_t msg_objectiveFields[MSG_OBJECTIVE_FIELDS] = {
	{ "origin[0]",  4,  0 },
	{ "origin[1]",  8,  0 },
	{ "origin[2]", 12,  0 },
	{ "icon",      24, 12 },
	{ "entNum",    16, 10 },
	{ "teamNum",   20,  4 }
};

static netField_t msg_hudElemFields[MSG_HUDELEM_FIELDS] = {
	{ "color.rgba",      28, 32 },
	{ "type",             0,  4 },
	{ "fontScale",       12,  0 },
	{ "y",                8, 10 },
	{ "x",                4, 10 },
	{ "alignY",          24,  2 },
	{ "alignX",          20,  2 },
	{ "time",            92, 32 },
	{ "font",            16,  4 },
	{ "text",           104,  8 },
	{ "shaderIndex",     56,  8 },
	{ "width",           48, 10 },
	{ "height",          52, 10 },
	{ "sort",           108,  0 },
	{ "fromColor.rgba",  32, 32 },
	{ "fadeStartTime",   36, 32 },
	{ "fadeTime",        40, 16 },
	{ "scaleStartTime",  68, 32 },
	{ "scaleTime",       72, 16 },
	{ "fromHeight",      64, 10 },
	{ "value",          100,  0 },
	{ "label",           44,  8 },
	{ "fromWidth",       60, 10 },
	{ "moveStartTime",   84, 32 },
	{ "moveTime",        88, 16 },
	{ "fromX",           76, 10 },
	{ "fromY",           80, 10 },
	{ "duration",        96, 32 }
};

/* MSG_initHuffman  0x00448390  VERIFIED */
void MSG_initHuffman( void ) {
	int i, j;

	msgInit = qtrue;
	Huff_Init( &msgHuff );
	for ( i = 0; i < 256; i++ ) {
		for ( j = 0; j < msg_hData[i]; j++ ) {
			Huff_addRef( &msgHuff.compressor, (byte)i );
			Huff_addRef( &msgHuff.decompressor, (byte)i );
		}
	}
}

/* MSG_Init  0x004446E0  VERIFIED */
void MSG_Init( msg_t *buf, byte *data, int length ) {
	if ( msgInit == qfalse ) {
		MSG_initHuffman();
	}
	memset( buf, 0, sizeof( msg_t ) );
	buf->data = data;
	buf->maxsize = length;
}

/* MSG_BeginReading  0x00444710  VERIFIED */
void MSG_BeginReading( msg_t *msg ) {
	msg->readcount = 0;
	msg->bit = 0;
}

/* MSG_WriteBits  0x00444720  VERIFIED */
void MSG_WriteBits( msg_t *msg, int value, int bits ) {
	int bitInByte;
	unsigned int v;

	if ( msg->maxsize - msg->cursize < 4 ) {
		msg->overflowed = qtrue;
		return;
	}

	v = (unsigned int)value;
	while ( bits ) {
		bitInByte = msg->bit & 7;
		if ( !bitInByte ) {
			msg->bit = 8 * msg->cursize;
			msg->data[msg->cursize] = 0;
			msg->cursize++;
		}
		if ( v & 1 ) {
			msg->data[msg->bit >> 3] |= 1 << bitInByte;
		}
		v >>= 1;
		msg->bit++;
		bits--;
	}
}

/* MSG_WriteBit0  0x00444790  VERIFIED */
void MSG_WriteBit0( msg_t *msg ) {
	if ( msg->cursize >= msg->maxsize ) {
		msg->overflowed = qtrue;
		return;
	}
	if ( ( msg->bit & 7 ) == 0 ) {
		msg->bit = 8 * msg->cursize;
		msg->data[msg->cursize] = 0;
		msg->cursize++;
	}
	msg->bit++;
}

/* MSG_WriteBit1  0x004447C0  VERIFIED */
void MSG_WriteBit1( msg_t *msg ) {
	int bitInByte;

	if ( msg->cursize >= msg->maxsize ) {
		msg->overflowed = qtrue;
		return;
	}
	bitInByte = msg->bit & 7;
	if ( !bitInByte ) {
		msg->bit = 8 * msg->cursize;
		msg->data[msg->cursize] = 0;
		msg->cursize++;
	}
	msg->data[msg->bit >> 3] |= 1 << bitInByte;
	msg->bit++;
}

/* MSG_ReadBits  0x00444810  VERIFIED */
int MSG_ReadBits( msg_t *msg, int bits ) {
	unsigned int value;
	int i, bitInByte;

	value = 0;
	for ( i = 0; i < bits; i++ ) {
		bitInByte = msg->bit & 7;
		if ( !bitInByte ) {
			msg->bit = 8 * msg->readcount;
			msg->readcount++;
		}
		value |= (unsigned int)( ( msg->data[msg->bit >> 3] >> bitInByte ) & 1 ) << i;
		msg->bit++;
	}
	return (int)value;
}

/* MSG_ReadBit  0x00444870  VERIFIED */
int MSG_ReadBit( msg_t *msg ) {
	int bitInByte, value;

	bitInByte = msg->bit & 7;
	if ( !bitInByte ) {
		msg->bit = 8 * msg->readcount;
		msg->readcount++;
	}
	value = ( msg->data[msg->bit >> 3] >> bitInByte ) & 1;
	msg->bit++;
	return value;
}

/* MSG_WriteBitsCompress  0x004448B0  VERIFIED */
int MSG_WriteBitsCompress( const byte *datasrc, int bytecount, byte *buffdest, int capacity ) {
	int i, offset;
	if ( bytecount < 0 || bytecount > MAX_MSGLEN || capacity < 0 || capacity > MAX_MSGLEN ) {
		Com_Error( ERR_DROP, "Invalid compression buffer size" );
		return 0;
	}

	offset = 0;
	for ( i = 0; i < bytecount; i++ ) {
		node_t *node = msgHuff.compressor.loc[datasrc[i]];
		int bits = 0;
		while ( node && node->parent ) {
			bits++;
			node = node->parent;
		}
		if ( capacity < 0 || offset / 8 > capacity || bits > capacity * 8 - offset ) {
			Com_Error( ERR_DROP, "Compressed message exceeds buffer capacity" );
			return 0;
		}
		Huff_offsetTransmit( &msgHuff.compressor, datasrc[i], buffdest, &offset );
	}
	return ( offset + 7 ) >> 3;
}

/* MSG_ReadBitsCompress  0x00444900  VERIFIED */
int MSG_ReadBitsCompress( const byte *input, byte *outputBuf, int readsize, int capacity ) {
	byte *outptr;
	int offset, endOffset, get;
	if ( readsize < 0 || readsize > MAX_MSGLEN || capacity < 0 || capacity > MAX_MSGLEN ) {
		Com_Error( ERR_DROP, "Invalid decompression buffer size" );
		return 0;
	}

	outptr = outputBuf;
	offset = 0;
	endOffset = readsize << 3;
	while ( offset < endOffset ) {
		node_t *node = msgHuff.decompressor.tree;
		if ( outptr - outputBuf == capacity && endOffset - offset < 8 ) {
			return outptr - outputBuf;
		}
		while ( node && node->symbol == INTERNAL_NODE ) {
			/* The last byte may contain padding, not a complete symbol. */
			if ( offset == endOffset ) {
				return outptr - outputBuf;
			}
			node = ( input[offset >> 3] >> ( offset & 7 ) ) & 1 ? node->right : node->left;
			offset++;
		}
		if ( !node || node->symbol > 255 || outptr - outputBuf >= capacity ) {
			Com_Error( ERR_DROP, "Invalid or oversized compressed message" );
			return 0;
		}
		get = node->symbol;
		*outptr++ = (byte)get;
	}
	return outptr - outputBuf;
}

/* MSG_WriteByte  0x00444960  VERIFIED */
void MSG_WriteByte( msg_t *msg, int c ) {
	if ( msg->cursize >= msg->maxsize ) {
		msg->overflowed = qtrue;
		return;
	}
	msg->data[msg->cursize] = (byte)c;
	msg->cursize++;
}

/* MSG_WriteData  0x00444980  VERIFIED */
void MSG_WriteData( msg_t *msg, int length, const void *data ) {
	if ( msg->cursize + length > msg->maxsize ) {
		msg->overflowed = qtrue;
		return;
	}
	memcpy( &msg->data[msg->cursize], data, (size_t)(unsigned int)length );
	msg->cursize += length;
}

/* MSG_WriteShort  0x004449C0  VERIFIED */
void MSG_WriteShort( msg_t *msg, int c ) {
	if ( msg->cursize + 2 > msg->maxsize ) {
		msg->overflowed = qtrue;
		return;
	}
	*(short *)&msg->data[msg->cursize] = (short)c;
	msg->cursize += 2;
}

/* MSG_WriteLong  0x004449F0  VERIFIED */
void MSG_WriteLong( msg_t *msg, int c ) {
	if ( msg->cursize + 4 > msg->maxsize ) {
		msg->overflowed = qtrue;
		return;
	}
	*(int *)&msg->data[msg->cursize] = c;
	msg->cursize += 4;
}

static char MSG_CleanChar( char c ) {
	if ( c == (char)0x92 ) {
		return '\'';
	}
	if ( (byte)c > 0x7F ) {
		return '.';
	}
	return c;
}

/* MSG_WriteString  0x00444A20  VERIFIED */
void MSG_WriteString( const char *s, msg_t *sb ) {
	char string[MAX_STRING_CHARS];
	int i, len;

	if ( !s ) {
		MSG_WriteByte( sb, 0 );
		return;
	}

	len = strlen( s );
	if ( len >= MAX_STRING_CHARS ) {
		Com_Printf( "MSG_WriteString: MAX_STRING_CHARS" );
		MSG_WriteByte( sb, 0 );
		return;
	}

	for ( i = 0; i < len; i++ ) {
		string[i] = MSG_CleanChar( s[i] );
	}
	string[len] = 0;
	MSG_WriteData( sb, len + 1, string );
}

/* MSG_WriteBigString  0x00444B40  VERIFIED */
void MSG_WriteBigString( const char *s, msg_t *sb ) {
	char string[BIG_INFO_STRING];
	int i, len;

	if ( !s ) {
		MSG_WriteByte( sb, 0 );
		return;
	}

	len = strlen( s );
	if ( len >= BIG_INFO_STRING ) {
		Com_Printf( "MSG_WriteString: BIG_INFO_STRING" );
		MSG_WriteByte( sb, 0 );
		return;
	}

	strncpy( string, s, BIG_INFO_STRING - 1 );
	string[BIG_INFO_STRING - 1] = 0;
	for ( i = 0; i < len; i++ ) {
		string[i] = MSG_CleanChar( string[i] );
	}
	MSG_WriteData( sb, len + 1, string );
}

/* MSG_WriteAngle  0x00444CD0  VERIFIED */
void MSG_WriteAngle( msg_t *sb, float f ) {
	MSG_WriteByte( sb, (int)( f * 0.71111113f ) & 255 );
}

/* MSG_WriteAngle16  0x00444D00  VERIFIED */
void MSG_WriteAngle16( msg_t *sb, float f ) {
	MSG_WriteShort( sb, (int)( f * 182.04445f ) & 65535 );
}

/* MSG_ReadByte  0x00444D40  VERIFIED */
int MSG_ReadByte( msg_t *msg ) {
	int c;

	if ( msg->readcount >= msg->cursize ) {
		return -1;
	}
	c = msg->data[msg->readcount];
	msg->readcount++;
	return c;
}

/* MSG_ReadShort  0x00444D60  VERIFIED */
int MSG_ReadShort( msg_t *msg ) {
	int c;

	if ( msg->readcount + 2 > msg->cursize ) {
		return -1;
	}
	c = *(short *)&msg->data[msg->readcount];
	msg->readcount += 2;
	return c;
}

/* MSG_ReadLong  0x00444D90  VERIFIED */
int MSG_ReadLong( msg_t *msg ) {
	int c;

	if ( msg->readcount + 4 > msg->cursize ) {
		return -1;
	}
	c = *(int *)&msg->data[msg->readcount];
	msg->readcount += 4;
	return c;
}

/* MSG_ReadString  0x00444DB0  VERIFIED */
char *MSG_ReadString( msg_t *msg ) {
	unsigned int i;
	int c;

	for ( i = 0; i < MAX_STRING_CHARS - 1; i++ ) {
		c = MSG_ReadByte( msg );
		if ( c == -1 || c == 0 ) {
			break;
		}
		msg_stringBuf[i] = MSG_CleanChar( (char)c );
	}
	msg_stringBuf[i] = 0;
	return msg_stringBuf;
}

/* MSG_ReadBigString  0x00444E00  VERIFIED */
char *MSG_ReadBigString( msg_t *msg ) {
	unsigned int i;
	int c;

	for ( i = 0; i < BIG_INFO_STRING - 1; i++ ) {
		c = MSG_ReadByte( msg );
		if ( c == -1 || c == 0 ) {
			break;
		}
		if ( c == '%' ) {
			c = '.';
		}
		msg_bigStringBuf[i] = MSG_CleanChar( (char)c );
	}
	msg_bigStringBuf[i] = 0;
	return msg_bigStringBuf;
}

/* MSG_ReadStringLine  0x00444E60  VERIFIED */
char *MSG_ReadStringLine( msg_t *msg ) {
	unsigned int i;
	int c;

	for ( i = 0; i < MAX_STRING_CHARS - 1; i++ ) {
		c = MSG_ReadByte( msg );
		if ( c == -1 || c == 0 || c == '\n' ) {
			break;
		}
		if ( c == '%' ) {
			c = '.';
		}
		msg_stringLineBuf[i] = MSG_CleanChar( (char)c );
	}
	msg_stringLineBuf[i] = 0;
	return msg_stringLineBuf;
}

/* MSG_ReadAngle16  0x00444EC0  VERIFIED */
float MSG_ReadAngle16( msg_t *msg ) {
	return (float)MSG_ReadShort( msg ) * 0.0054931640625f;
}

/* MSG_ReadData  0x00444F10  VERIFIED */
void MSG_ReadData( msg_t *msg, void *data, int len ) {
	if ( msg->readcount + len > msg->cursize ) {
		memset( data, 0xFF, (size_t)(unsigned int)len );
		return;
	}
	memcpy( data, &msg->data[msg->readcount], (size_t)(unsigned int)len );
	msg->readcount += len;
}

/* MSG_WriteDeltaKeyFloat  0x00444F60  VERIFIED */
void MSG_WriteDeltaKeyFloat( msg_t *msg, int oldV, int newV, int bits ) {
	if ( oldV == newV ) {
		MSG_WriteBit0( msg );
		return;
	}
	MSG_WriteBit1( msg );
	MSG_WriteBits( msg, newV, bits );
}

/* MSG_ReadDeltaKeyFloat  0x00444FB0  VERIFIED */
int MSG_ReadDeltaKeyFloat( msg_t *msg, int oldV, int bits ) {
	if ( !MSG_ReadBit( msg ) ) {
		return oldV;
	}
	return MSG_ReadBits( msg, bits );
}

/* MSG_WriteDeltaKey  0x00445010  VERIFIED */
void MSG_WriteDeltaKey( msg_t *msg, int key, int oldV, int newV, int bits ) {
	if ( oldV == newV ) {
		MSG_WriteBit0( msg );
		return;
	}
	MSG_WriteBit1( msg );
	MSG_WriteBits( msg, key ^ newV, bits );
}

/* MSG_ReadDeltaKey  0x00445060  VERIFIED */
int MSG_ReadDeltaKey( msg_t *msg, int key, int oldV, int bits ) {
	if ( !MSG_ReadBit( msg ) ) {
		return oldV;
	}
	return MSG_ReadBits( msg, bits ) ^ ( key & kbitmask[bits] );
}

/* MSG_WriteKey  0x004450D0  VERIFIED */
void MSG_WriteKey( msg_t *msg, int key, int newV, int bits ) {
	MSG_WriteBits( msg, key ^ newV, bits );
}

/* MSG_ReadKey  0x004450E0  VERIFIED */
int MSG_ReadKey( msg_t *msg, int key, int bits ) {
	return MSG_ReadBits( msg, bits ) ^ ( key & kbitmask[bits] );
}

/* MSG_WriteDeltaKeyByte  0x00445100  VERIFIED */
void MSG_WriteDeltaKeyByte( msg_t *msg, int key, int oldV, int newV ) {
	if ( (byte)oldV == (byte)newV ) {
		MSG_WriteBit0( msg );
		return;
	}
	MSG_WriteBit1( msg );
	MSG_WriteByte( msg, key ^ newV );
}

/* MSG_ReadDeltaKeyByte  0x00445160  VERIFIED */
int MSG_ReadDeltaKeyByte( msg_t *msg, int key, int oldV ) {
	if ( !MSG_ReadBit( msg ) ) {
		return oldV;
	}
	return (byte)( MSG_ReadByte( msg ) ^ key );
}

/* MSG_WriteDeltaKeyShort  0x004451E0  VERIFIED */
void MSG_WriteDeltaKeyShort( msg_t *msg, int key, int oldV, int newV ) {
	if ( (short)oldV == (short)newV ) {
		MSG_WriteBit0( msg );
		return;
	}
	MSG_WriteBit1( msg );
	MSG_WriteShort( msg, key ^ newV );
}

/* MSG_ReadDeltaKeyShort  0x00445250  VERIFIED */
int MSG_ReadDeltaKeyShort( msg_t *msg, int key, int oldV ) {
	if ( !MSG_ReadBit( msg ) ) {
		return oldV;
	}
	return (short)( MSG_ReadShort( msg ) ^ key );
}

/* MSG_WriteReliableCommandToBuffer  0x004452D0  VERIFIED */
void MSG_WriteReliableCommandToBuffer( const char *pszCommand, int iBufferSize, char *pszBuffer ) {
	int len, i;
	char c;

	len = strlen( pszCommand );
	if ( len >= iBufferSize ) {
		Com_Printf( "WARNING: Reliable command is too long (%i/%i) and will be truncated: '%s'\n",
					len, iBufferSize, pszCommand );
	}
	if ( !len ) {
		Com_Printf( "WARNING: Empty reliable command\n" );
	}

	for ( i = 0; i < iBufferSize && pszCommand[i] != '\0'; i++ ) {
		c = MSG_CleanChar( pszCommand[i] );
		if ( c == '%' ) {
			c = '.';
		}
		pszBuffer[i] = c;
	}

	if ( i < iBufferSize ) {
		pszBuffer[i] = 0;
	} else {
		pszBuffer[iBufferSize - 1] = 0;
	}
}

#define PMF_FOLLOW_ACTIVE       0x40000
#define EF_PRONE                0x40
#define EF_CROUCH               0x20

#define UCMD_WBUTTON_PRONE      0x40
#define UCMD_WBUTTON_CROUCH     0x80
#define UCMD_WBUTTON_LEAN_LEFT  0x10
#define UCMD_WBUTTON_LEAN_RIGHT 0x20
#define UCMD_BUTTON_ADS         0x10
#define UCMD_STANCE_UPMOVE      -127

/* MSG_SetDefaultUserCmd  0x00445360  VERIFIED */
void MSG_SetDefaultUserCmd( const byte *ps, usercmd_t *ucmd ) {
	const int *psi = (const int *)ps;
	const float *psf = (const float *)ps;
	int i;

	memset( ucmd, 0, sizeof( usercmd_t ) );
	ucmd->weapon = (byte)psi[44];

	for ( i = 0; i < 2; i++ ) {
		ucmd->angles[i] = ( (int)( psf[48 + i] * 182.04445f )
							- psi[18 + i] ) & 0xFFFF;
	}

	if ( ( psi[3] & PMF_FOLLOW_ACTIVE ) == 0 ) {
		return;
	}

	if ( psi[32] & EF_PRONE ) {
		ucmd->wbuttons |= UCMD_WBUTTON_PRONE;
		ucmd->upmove = UCMD_STANCE_UPMOVE;
	} else if ( psi[32] & EF_CROUCH ) {
		ucmd->wbuttons |= UCMD_WBUTTON_CROUCH;
		ucmd->upmove = UCMD_STANCE_UPMOVE;
	}

	if ( psf[16] > 0.0f ) {
		ucmd->wbuttons |= UCMD_WBUTTON_LEAN_RIGHT;
	} else if ( psf[16] < 0.0f ) {
		ucmd->wbuttons |= UCMD_WBUTTON_LEAN_LEFT;
	}

	if ( psf[46] != 0.0f ) {
		ucmd->buttons |= UCMD_BUTTON_ADS;
	}
}

#define UCMD_MOVE_THRESHOLD  10
#define UCMD_MOVE_POSITIVE   127
#define UCMD_MOVE_NEGATIVE   ( -127 )

/* MSG_HorMoveTo  0x00445430  VERIFIED */
int MSG_HorMoveTo( int forwardmove, int rightmove ) {
	int flags = 0;

	if ( forwardmove > UCMD_MOVE_THRESHOLD ) {
		flags = 1;
	} else if ( forwardmove < -UCMD_MOVE_THRESHOLD ) {
		flags = 2;
	}
	if ( rightmove > UCMD_MOVE_THRESHOLD ) {
		flags |= 4;
	} else if ( rightmove < -UCMD_MOVE_THRESHOLD ) {
		flags |= 8;
	}
	return flags;
}

/* MSG_VertMoveTo  0x00445460  VERIFIED */
int MSG_VertMoveTo( int upmove ) {
	if ( upmove > UCMD_MOVE_THRESHOLD ) {
		return 1;
	}
	if ( upmove < -UCMD_MOVE_THRESHOLD ) {
		return 2;
	}
	return 0;
}

/* MSG_HorMoveFrom  0x00445480  VERIFIED */
void MSG_HorMoveFrom( int flags, signed char *forwardmove, signed char *rightmove ) {
	if ( flags & 1 ) {
		*forwardmove = UCMD_MOVE_POSITIVE;
	} else if ( flags & 2 ) {
		*forwardmove = UCMD_MOVE_NEGATIVE;
	} else {
		*forwardmove = 0;
	}

	if ( flags & 4 ) {
		*rightmove = UCMD_MOVE_POSITIVE;
	} else if ( flags & 8 ) {
		*rightmove = UCMD_MOVE_NEGATIVE;
	} else {
		*rightmove = 0;
	}
}

/* MSG_VertMoveFrom  0x004454B0  VERIFIED */
void MSG_VertMoveFrom( int flags, signed char *upmove ) {
	if ( flags & 1 ) {
		*upmove = UCMD_MOVE_POSITIVE;
	} else if ( flags & 2 ) {
		*upmove = UCMD_MOVE_NEGATIVE;
	} else {
		*upmove = 0;
	}
}

/* MSG_WriteDeltaUsercmdKey  0x004454D0  VERIFIED */
void MSG_WriteDeltaUsercmdKey( const usercmd_t *from, const usercmd_t *to, int key, msg_t *msg ) {
	int newHor, oldHor, newVert, oldVert, timeKey;

	if ( (unsigned int)( to->serverTime - from->serverTime ) < 256 ) {
		MSG_WriteBit1( msg );
		MSG_WriteByte( msg, to->serverTime - from->serverTime );
	} else {
		MSG_WriteBit0( msg );
		MSG_WriteLong( msg, to->serverTime );
	}

	newHor = MSG_HorMoveTo( to->forwardmove, to->rightmove );
	oldHor = MSG_HorMoveTo( from->forwardmove, from->rightmove );
	newVert = MSG_VertMoveTo( to->upmove );
	oldVert = MSG_VertMoveTo( from->upmove );

	if ( ( ( from->buttons ^ to->buttons ) & 0xFE ) == 0
		 && from->wbuttons == to->wbuttons
		 && from->weapon == to->weapon
		 && from->angles[2] == to->angles[2]
		 && oldVert == newVert ) {

		if ( from->angles[0] == to->angles[0]
			 && from->angles[1] == to->angles[1]
			 && ( ( from->buttons ^ to->buttons ) & 1 ) == 0
			 && oldHor == newHor ) {
			MSG_WriteKey( msg, key, 0, 1 );
			return;
		}

		MSG_WriteKey( msg, key, 1, 1 );
		MSG_WriteKey( msg, key, 0, 1 );
		timeKey = key ^ to->serverTime;
		MSG_WriteKey( msg, timeKey, to->buttons & 1, 1 );
		MSG_WriteDeltaKeyShort( msg, timeKey, from->angles[0], to->angles[0] );
		MSG_WriteDeltaKeyShort( msg, timeKey, from->angles[1], to->angles[1] );
		MSG_WriteDeltaKey( msg, timeKey, oldHor, newHor, 4 );
		return;
	}

	MSG_WriteKey( msg, key, 1, 1 );
	MSG_WriteKey( msg, key, 1, 1 );
	MSG_WriteKey( msg, key, to->buttons & 1, 1 );
	MSG_WriteDeltaKeyShort( msg, key, from->angles[0], to->angles[0] );
	MSG_WriteDeltaKeyShort( msg, key, from->angles[1], to->angles[1] );
	MSG_WriteDeltaKey( msg, key, oldHor, newHor, 4 );

	timeKey = key ^ to->serverTime;
	MSG_WriteDeltaKeyShort( msg, timeKey, from->angles[2], to->angles[2] );
	MSG_WriteDeltaKey( msg, timeKey, from->buttons >> 1, to->buttons >> 1, 6 );
	MSG_WriteDeltaKeyByte( msg, timeKey, from->wbuttons, to->wbuttons );
	MSG_WriteDeltaKey( msg, timeKey, oldVert, newVert, 2 );
	MSG_WriteDeltaKey( msg, timeKey, from->weapon, to->weapon, msg->extended ? 8 : 6 );
}

/* MSG_ReadDeltaUsercmdKey  0x004458E0  VERIFIED */
void MSG_ReadDeltaUsercmdKey( msg_t *msg, int key, const usercmd_t *from, usercmd_t *to ) {
	int timeKey;

	memcpy( to, from, sizeof( usercmd_t ) );

	if ( MSG_ReadBit( msg ) ) {
		to->serverTime = from->serverTime + MSG_ReadByte( msg );
	} else {
		to->serverTime = MSG_ReadLong( msg );
	}

	if ( !MSG_ReadKey( msg, key, 1 ) ) {
		return;
	}

	to->buttons &= ~1u;
	if ( !MSG_ReadKey( msg, key, 1 ) ) {
		timeKey = key ^ to->serverTime;
		to->buttons |= (byte)MSG_ReadKey( msg, timeKey, 1 );
		to->angles[0] = MSG_ReadDeltaKeyShort( msg, timeKey, from->angles[0] ) & 0xFFFF;
		to->angles[1] = MSG_ReadDeltaKeyShort( msg, timeKey, from->angles[1] ) & 0xFFFF;
		MSG_HorMoveFrom( MSG_ReadDeltaKey( msg, timeKey,
										   MSG_HorMoveTo( from->forwardmove, from->rightmove ), 4 ),
						 &to->forwardmove, &to->rightmove );
		return;
	}

	to->buttons |= (byte)MSG_ReadKey( msg, key, 1 );
	to->angles[0] = MSG_ReadDeltaKeyShort( msg, key, from->angles[0] ) & 0xFFFF;
	to->angles[1] = MSG_ReadDeltaKeyShort( msg, key, from->angles[1] ) & 0xFFFF;
	MSG_HorMoveFrom( MSG_ReadDeltaKey( msg, key,
									   MSG_HorMoveTo( from->forwardmove, from->rightmove ), 4 ),
					 &to->forwardmove, &to->rightmove );

	timeKey = key ^ to->serverTime;
	to->angles[2] = MSG_ReadDeltaKeyShort( msg, timeKey, from->angles[2] ) & 0xFFFF;
	to->buttons &= 1u;
	to->buttons |= (byte)( MSG_ReadDeltaKey( msg, timeKey, from->buttons >> 1, 6 ) << 1 );
	to->wbuttons = (byte)MSG_ReadDeltaKeyByte( msg, timeKey, from->wbuttons );
	MSG_VertMoveFrom( MSG_ReadDeltaKey( msg, timeKey,
										MSG_VertMoveTo( from->upmove ), 2 ),
					  &to->upmove );
	to->weapon = (byte)MSG_ReadDeltaKey( msg, timeKey, from->weapon, msg->extended ? 8 : 6 );
}

#define MSG_SMALL_FLOAT_BIAS   4096
#define MSG_SMALL_FLOAT_RANGE  8192
#define MSG_SMALL_FLOAT_BITS   5

/* MSG_WriteDeltaField  0x00445C00  VERIFIED */
void MSG_WriteDeltaField( const byte *from, const byte *to, msg_t *msg, const netField_t *field ) {
	int offset = field->offset;
	int newValue = *(const int *)( to + offset );
	int oldValue = *(const int *)( from + offset );
	unsigned int v;
	int bits, lowBits;
	float f;
	int trunc;

	if ( oldValue == newValue ) {
		MSG_WriteBit0( msg );
		return;
	}
	MSG_WriteBit1( msg );

	if ( field->bits ) {
		if ( !newValue ) {
			MSG_WriteBit0( msg );
			return;
		}
		MSG_WriteBit1( msg );

		bits = field->bits;
		if ( msg->extended ) {
			if ( !strcmp( field->name, "weapon" ) ) bits = 8;
			else if ( !strncmp( field->name, "eventParm", 9 ) ) bits = 9;
		}
		if ( bits < 0 ) {
			bits = -bits;
		}
		v = (unsigned int)newValue;
		lowBits = bits & 7;
		if ( lowBits ) {
			MSG_WriteBits( msg, (int)v, lowBits );
			bits -= lowBits;
			v >>= lowBits;
		}
		while ( bits ) {
			MSG_WriteByte( msg, (int)( v & 0xFF ) );
			v >>= 8;
			bits -= 8;
		}
		return;
	}

	f = *(const float *)( to + offset );
	if ( f == 0.0f ) {
		MSG_WriteBit0( msg );
		return;
	}
	MSG_WriteBit1( msg );

	trunc = (int)f;
	if ( (float)trunc == f
		 && (unsigned int)( trunc + MSG_SMALL_FLOAT_BIAS ) < MSG_SMALL_FLOAT_RANGE ) {
		MSG_WriteBit0( msg );
		MSG_WriteBits( msg, trunc + MSG_SMALL_FLOAT_BIAS, MSG_SMALL_FLOAT_BITS );
		MSG_WriteByte( msg, ( trunc + MSG_SMALL_FLOAT_BIAS ) >> MSG_SMALL_FLOAT_BITS );
	} else {
		MSG_WriteBit1( msg );
		MSG_WriteLong( msg, newValue );
	}
}

/* MSG_WriteDeltaFields_0  0x00445D60  VERIFIED */
void MSG_WriteDeltaFields_0( int force, msg_t *msg, const byte *from, const byte *to,
							 int numFields, const netField_t *stateFields ) {
	int i;

	if ( !force ) {
		for ( i = 0; i < numFields; i++ ) {
			if ( *(const int *)( from + stateFields[i].offset )
				 != *(const int *)( to + stateFields[i].offset ) ) {
				break;
			}
		}
		if ( i >= numFields ) {
			MSG_WriteBit0( msg );
			return;
		}
	}

	MSG_WriteBit1( msg );
	for ( i = 0; i < numFields; i++ ) {
		MSG_WriteDeltaField( from, to, msg, &stateFields[i] );
	}
}

/* MSG_WriteDeltaStruct  0x00445E40  VERIFIED */
void MSG_WriteDeltaStruct( int numFields, msg_t *msg, const byte *from, const byte *to,
						   qboolean force, int indexBits, const netField_t *stateFields,
						   qboolean bChangeBit ) {
	int i, lastChanged;

	if ( !to ) {
		if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
			Com_Printf( "W|%3i: #%-3i remove\n", msg->cursize, *(const int *)from );
		}
		if ( bChangeBit ) {
			MSG_WriteBit1( msg );
		}
		MSG_WriteBits( msg, *(const int *)from, indexBits );
		MSG_WriteBit1( msg );
		return;
	}

	lastChanged = 0;
	for ( i = 0; i < numFields; i++ ) {
		if ( *(const int *)( from + stateFields[i].offset )
			 != *(const int *)( to + stateFields[i].offset ) ) {
			lastChanged = i + 1;
		}
	}

	if ( !lastChanged ) {
		if ( force ) {
			if ( bChangeBit ) {
				MSG_WriteBit1( msg );
			}
			MSG_WriteBits( msg, *(const int *)to, indexBits );
			MSG_WriteBit0( msg );
			MSG_WriteBit0( msg );
		}
		return;
	}

	if ( bChangeBit ) {
		MSG_WriteBit1( msg );
	}
	MSG_WriteBits( msg, *(const int *)to, indexBits );
	MSG_WriteBit0( msg );
	MSG_WriteBit1( msg );
	MSG_WriteByte( msg, lastChanged );
	for ( i = 0; i < lastChanged; i++ ) {
		MSG_WriteDeltaField( from, to, msg, &stateFields[i] );
	}
}

/* MSG_WriteDeltaEntity  0x00446050  VERIFIED */
void MSG_WriteDeltaEntity( msg_t *msg, const byte *from, const byte *to, qboolean force ) {
	MSG_WriteDeltaStruct( MSG_ENTITY_FIELDS, msg, from, to, force,
						  MSG_ENTITY_NUM_BITS, stateFields, qfalse );
}

/* MSG_WriteDeltaArchivedEntity  0x00446070  VERIFIED */
void MSG_WriteDeltaArchivedEntity( msg_t *msg, const byte *from, const byte *to, qboolean force ) {
	MSG_WriteDeltaStruct( MSG_ENTITY_BASE_FIELDS, msg, from, to, force,
						  MSG_ENTITY_NUM_BITS, archivedEntityFields, qfalse );
}

/* MSG_WriteDeltaClient  0x00446090  VERIFIED */
void MSG_WriteDeltaClient( msg_t *msg, const byte *from, const byte *to, qboolean force ) {
	byte nullClient[92];

	if ( !from ) {
		memset( nullClient, 0, sizeof( nullClient ) );
		from = nullClient;
	}
	MSG_WriteDeltaStruct( MSG_CLIENT_FIELDS, msg, from, to, force,
						  MSG_CLIENT_NUM_BITS, clientStateFields, qtrue );
}

/* MSG_ReadDeltaField  0x004460F0  VERIFIED */
void MSG_ReadDeltaField( const byte *from, msg_t *msg, byte *to,
						 const netField_t *field, qboolean print ) {
	int offset = field->offset;
	unsigned int v;
	int bits, lowBits, bit;
	int small;
	float f;

	if ( !MSG_ReadBit( msg ) ) {
		*(int *)( to + offset ) = *(const int *)( from + offset );
		return;
	}

	if ( field->bits ) {
		if ( !MSG_ReadBit( msg ) ) {
			*(int *)( to + offset ) = 0;
			return;
		}
		bits = field->bits;
		if ( msg->extended ) {
			if ( !strcmp( field->name, "weapon" ) ) bits = 8;
			else if ( !strncmp( field->name, "eventParm", 9 ) ) bits = 9;
		}
		if ( bits < 0 ) {
			bits = -bits;
		}
		lowBits = bits & 7;
		v = lowBits ? (unsigned int)MSG_ReadBits( msg, lowBits ) : 0;
		for ( bit = lowBits; bit < bits; bit += 8 ) {
			v |= (unsigned int)MSG_ReadByte( msg ) << bit;
		}
		*(int *)( to + offset ) = (int)v;
		if ( print ) {
			Com_Printf( "%s:%i ", field->name, (int)v );
		}
		return;
	}

	if ( !MSG_ReadBit( msg ) ) {
		*(int *)( to + offset ) = 0;
		return;
	}
	if ( MSG_ReadBit( msg ) ) {
		*(int *)( to + offset ) = MSG_ReadLong( msg );
		if ( print ) {
			Com_Printf( "%s:%f ", field->name, *(float *)( to + offset ) );
		}
		return;
	}
	small = MSG_ReadBits( msg, MSG_SMALL_FLOAT_BITS );
	small += MSG_ReadByte( msg ) << MSG_SMALL_FLOAT_BITS;
	small -= MSG_SMALL_FLOAT_BIAS;
	f = (float)small;
	*(float *)( to + offset ) = f;
	if ( print ) {
		Com_Printf( "%s:%i ", field->name, small );
	}
}

/* MSG_ReadDeltaFields  0x004462A0  VERIFIED */
void MSG_ReadDeltaFields( msg_t *msg, int numFields, const byte *from, byte *to,
						  const netField_t *stateFields ) {
	int i;

	if ( !MSG_ReadBit( msg ) ) {
		for ( i = 0; i < numFields; i++ ) {
			*(int *)( to + stateFields[i].offset ) =
				*(const int *)( from + stateFields[i].offset );
		}
		return;
	}

	for ( i = 0; i < numFields; i++ ) {
		MSG_ReadDeltaField( from, msg, to, &stateFields[i], qfalse );
	}
}

/* MSG_ReadDeltaStruct  0x004464E0  VERIFIED */
qboolean MSG_ReadDeltaStruct( msg_t *msg, const byte *from, byte *to, unsigned int number,
							  int numFields, int indexBits, const netField_t *stateFields ) {
	int lastChanged, i;
	qboolean print;

	(void)indexBits;

	if ( MSG_ReadBit( msg ) ) {
		if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
			Com_Printf( "%3i: #%-3i remove\n", msg->readcount, number );
		}
		return qtrue;
	}

	if ( !MSG_ReadBit( msg ) ) {
		memcpy( to, from, (size_t)( 4 * numFields + 4 ) );
		return qfalse;
	}

	lastChanged = MSG_ReadByte( msg );
	if ( lastChanged > numFields ) {
		lastChanged = numFields;
	}

	print = qfalse;
	if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
		print = qtrue;
		Com_Printf( "%3i: #%-3i ", msg->readcount, *(const int *)to );
	}

	*(int *)to = (int)number;

	for ( i = 0; i < lastChanged; i++ ) {
		MSG_ReadDeltaField( from, msg, to, &stateFields[i], print );
	}
	for ( i = lastChanged; i < numFields; i++ ) {
		*(int *)( to + stateFields[i].offset ) =
			*(const int *)( from + stateFields[i].offset );
	}
	return qfalse;
}

/* MSG_ReadDeltaEntity  0x004464E0  VERIFIED */
qboolean MSG_ReadDeltaEntity( msg_t *msg, const byte *from, byte *to, int number ) {
	return MSG_ReadDeltaStruct( msg, from, to, (unsigned int)number,
								MSG_ENTITY_FIELDS, MSG_ENTITY_NUM_BITS, stateFields );
}

/* MSG_ReadDeltaArchivedEntity  0x00446500  VERIFIED */
qboolean MSG_ReadDeltaArchivedEntity( msg_t *msg, const byte *from, byte *to, int number ) {
	return MSG_ReadDeltaStruct( msg, from, to, (unsigned int)number,
								MSG_ENTITY_BASE_FIELDS, MSG_ENTITY_NUM_BITS, archivedEntityFields );
}

/* MSG_ReadDeltaClient  0x00446520  VERIFIED */
qboolean MSG_ReadDeltaClient( msg_t *msg, const byte *from, byte *to, int number ) {
	byte nullClient[92];

	if ( !from ) {
		memset( nullClient, 0, sizeof( nullClient ) );
		from = nullClient;
	}
	return MSG_ReadDeltaStruct( msg, from, to, (unsigned int)number,
								MSG_CLIENT_FIELDS, MSG_CLIENT_NUM_BITS, clientStateFields );
}

#define MSG_HUD_COUNT_BITS      5
#define MSG_HUD_LASTFIELD_BITS  5

/* MSG_WriteDeltaHudElems  0x00446570  VERIFIED */
void MSG_WriteDeltaHudElems( const hudelem_t *from, int count, msg_t *msg, const hudelem_t *to ) {
	int used, elem, i, lastChanged;
	const byte *oldElem, *newElem;

	for ( used = 0; used < count && *(const int *)&to[used] != 0; used++ ) {
	}

	MSG_WriteBits( msg, used, MSG_HUD_COUNT_BITS );

	for ( elem = 0; elem < used; elem++ ) {
		oldElem = (const byte *)&from[elem];
		newElem = (const byte *)&to[elem];

		lastChanged = 0;
		for ( i = 0; i < MSG_HUDELEM_FIELDS; i++ ) {
			if ( *(const int *)( oldElem + msg_hudElemFields[i].offset )
				 != *(const int *)( newElem + msg_hudElemFields[i].offset ) ) {
				lastChanged = i;
			}
		}

		MSG_WriteBits( msg, lastChanged, MSG_HUD_LASTFIELD_BITS );
		for ( i = 0; i <= lastChanged; i++ ) {
			MSG_WriteDeltaField( oldElem, newElem, msg, &msg_hudElemFields[i] );
		}
	}
}

/* MSG_ReadDeltaHudElems  0x00446710  VERIFIED */
void MSG_ReadDeltaHudElems( msg_t *msg, const hudelem_t *from, hudelem_t *to, int count ) {
	int used, elem, i, lastChanged;
	const byte *oldElem;
	byte *newElem;

	used = MSG_ReadBits( msg, MSG_HUD_COUNT_BITS );
	if ( used > count ) {
		used = count;
	}

	for ( elem = 0; elem < used; elem++ ) {
		oldElem = (const byte *)&from[elem];
		newElem = (byte *)&to[elem];

		lastChanged = MSG_ReadBits( msg, MSG_HUD_LASTFIELD_BITS );
		if ( lastChanged >= MSG_HUDELEM_FIELDS ) {
			lastChanged = MSG_HUDELEM_FIELDS - 1;
		}

		for ( i = 0; i <= lastChanged; i++ ) {
			MSG_ReadDeltaField( oldElem, msg, newElem, &msg_hudElemFields[i], qfalse );
		}
		for ( i = lastChanged + 1; i < MSG_HUDELEM_FIELDS; i++ ) {
			*(int *)( newElem + msg_hudElemFields[i].offset ) =
				*(const int *)( oldElem + msg_hudElemFields[i].offset );
		}
	}

	memset( &to[used], 0, (size_t)( count - used ) * sizeof( hudelem_t ) );
}

#define PS_STATS_OFFSET        offsetof( playerState_t, stats )
#define PS_STATS_COUNT           6
#define PS_STATS_BITS            6
#define PS_STATS_WEAPON_INDEX    3
#define PS_STATS_WEAPON_BITS     ( msg->extended ? 8 : 6 )

#define PS_AMMO_OFFSET         offsetof( playerState_t, ammo )
#define PS_AMMOCLIP_OFFSET     offsetof( playerState_t, ammoclip )
#define PS_AMMO_GROUPS           ( msg->extended ? 16 : 4 )
#define PS_AMMO_GROUP_SIZE      16

#define PS_OBJECTIVE_OFFSET   offsetof( playerState_t, objective )
#define PS_OBJECTIVE_COUNT      16
#define PS_OBJECTIVE_SIZE       28
#define PS_OBJECTIVE_STATE_BITS  3

#define PS_HUD_CURRENT_OFFSET offsetof( playerState_t, hud.current )
#define PS_HUD_ARCHIVE_OFFSET offsetof( playerState_t, hud.archival )
#define PS_HUD_ELEMS            31
#define PS_HUD_TOTAL_BYTES    6944

/* MSG_WritePlayerStateField  0x00446570 */
static void MSG_WritePlayerStateField( const byte *from, const byte *to, msg_t *msg,
									   const netField_t *field ) {
	int offset = field->offset;
	int newValue = *(const int *)( to + offset );
	unsigned int v;
	int bits, lowBits;
	float f;
	int trunc;

	if ( *(const int *)( from + offset ) == newValue ) {
		MSG_WriteBit0( msg );
		return;
	}
	MSG_WriteBit1( msg );

	if ( field->bits ) {
		bits = field->bits;
		if ( msg->extended ) {
			if ( !strcmp( field->name, "weapon" ) ) bits = 8;
			else if ( !strncmp( field->name, "eventParm", 9 ) ) bits = 9;
		}
		if ( bits < 0 ) {
			bits = -bits;
		}
		v = (unsigned int)newValue;
		lowBits = bits & 7;
		if ( lowBits ) {
			MSG_WriteBits( msg, (int)v, lowBits );
			bits -= lowBits;
			v >>= lowBits;
		}
		while ( bits ) {
			MSG_WriteByte( msg, (int)( v & 0xFF ) );
			v >>= 8;
			bits -= 8;
		}
		return;
	}

	f = *(const float *)( to + offset );
	trunc = (int)f;
	if ( (float)trunc == f
		 && (unsigned int)( trunc + MSG_SMALL_FLOAT_BIAS ) < MSG_SMALL_FLOAT_RANGE ) {
		MSG_WriteBit0( msg );
		MSG_WriteBits( msg, trunc + MSG_SMALL_FLOAT_BIAS, MSG_SMALL_FLOAT_BITS );
		MSG_WriteByte( msg, ( trunc + MSG_SMALL_FLOAT_BIAS ) >> MSG_SMALL_FLOAT_BITS );
	} else {
		MSG_WriteBit1( msg );
		MSG_WriteLong( msg, newValue );
	}
}

static void MSG_ReadPlayerStateField( const byte *from, msg_t *msg, byte *to,
									  const netField_t *field, qboolean print ) {
	int offset = field->offset;
	unsigned int v;
	int bits, lowBits, bit;
	int small;

	if ( !MSG_ReadBit( msg ) ) {
		*(int *)( to + offset ) = *(const int *)( from + offset );
		return;
	}

	if ( field->bits ) {
		bits = field->bits;
		if ( msg->extended ) {
			if ( !strcmp( field->name, "weapon" ) ) bits = 8;
			else if ( !strncmp( field->name, "eventParm", 9 ) ) bits = 9;
		}
		if ( bits < 0 ) {
			bits = -bits;
		}
		lowBits = bits & 7;
		v = lowBits ? (unsigned int)MSG_ReadBits( msg, lowBits ) : 0;
		for ( bit = lowBits; bit < bits; bit += 8 ) {
			v |= (unsigned int)MSG_ReadByte( msg ) << bit;
		}
		*(int *)( to + offset ) = (int)v;
		if ( print ) {
			Com_Printf( "%s:%i ", field->name, (int)v );
		}
		return;
	}

	if ( MSG_ReadBit( msg ) ) {
		*(int *)( to + offset ) = MSG_ReadLong( msg );
		if ( print ) {
			Com_Printf( "%s:%f ", field->name, *(float *)( to + offset ) );
		}
		return;
	}
	small = MSG_ReadBits( msg, MSG_SMALL_FLOAT_BITS );
	small += MSG_ReadByte( msg ) << MSG_SMALL_FLOAT_BITS;
	small -= MSG_SMALL_FLOAT_BIAS;
	*(float *)( to + offset ) = (float)small;
	if ( print ) {
		Com_Printf( "%s:%i ", field->name, small );
	}
}

static void MSG_WritePlayerStateStat( msg_t *msg, const byte *to, int stat ) {
	int value = *(const int *)( to + PS_STATS_OFFSET + stat * 4 );

	if ( stat == PS_STATS_WEAPON_INDEX ) {
		MSG_WriteBits( msg, value, PS_STATS_WEAPON_BITS );
	} else if ( stat == PS_STATS_COUNT - 1 ) {
		MSG_WriteByte( msg, value );
	} else {
		MSG_WriteShort( msg, value );
	}
}

static int MSG_ReadPlayerStateStat( msg_t *msg, int stat ) {
	if ( stat == PS_STATS_WEAPON_INDEX ) {
		return MSG_ReadBits( msg, PS_STATS_WEAPON_BITS );
	}
	if ( stat == PS_STATS_COUNT - 1 ) {
		return MSG_ReadByte( msg );
	}
	return MSG_ReadShort( msg );
}

static unsigned int MSG_PlayerStateAmmoGroupBits( const byte *from, const byte *to,
												  int base, int group ) {
	unsigned int bits = 0;
	int item, offset;

	for ( item = 0; item < PS_AMMO_GROUP_SIZE; item++ ) {
		offset = base + ( group * PS_AMMO_GROUP_SIZE + item ) * 4;
		if ( *(const int *)( from + offset ) != *(const int *)( to + offset ) ) {
			bits |= 1u << item;
		}
	}
	return bits;
}

static void MSG_WritePlayerStateAmmoGroup( msg_t *msg, const byte *to, int base,
										   int group, unsigned int bits ) {
	int item;

	MSG_WriteShort( msg, (int)bits );
	for ( item = 0; item < PS_AMMO_GROUP_SIZE; item++ ) {
		if ( bits & ( 1u << item ) ) {
			MSG_WriteShort( msg, *(const int *)( to + base
												 + ( group * PS_AMMO_GROUP_SIZE + item ) * 4 ) );
		}
	}
}

static void MSG_ReadPlayerStateAmmoGroup( msg_t *msg, byte *to, int base, int group ) {
	unsigned int bits;
	int item;

	bits = (unsigned int)MSG_ReadShort( msg );
	for ( item = 0; item < PS_AMMO_GROUP_SIZE; item++ ) {
		if ( bits & ( 1u << item ) ) {
			*(int *)( to + base + ( group * PS_AMMO_GROUP_SIZE + item ) * 4 ) =
				MSG_ReadShort( msg );
		}
	}
}

/* MSG_WriteDeltaPlayerstate  0x00446830  VERIFIED */
void MSG_WriteDeltaPlayerstate( msg_t *msg, const byte *from, const byte *to, int number ) {
	static byte nullState[sizeof( playerState_t )];
	int i, lastChanged;
	unsigned int statBits;
	unsigned int anyAmmo = 0;
	unsigned int ammoBits[16];
	unsigned int clipBits;

	(void)number;

	if ( !from ) {
		memset( nullState, 0, sizeof( nullState ) );
		from = nullState;
	}

	lastChanged = 0;
	for ( i = 0; i < MSG_PLAYERSTATE_FIELDS; i++ ) {
		if ( *(const int *)( from + msg_playerStateFields[i].offset )
			 != *(const int *)( to + msg_playerStateFields[i].offset ) ) {
			lastChanged = i + 1;
		}
	}

	MSG_WriteByte( msg, lastChanged );
	for ( i = 0; i < lastChanged; i++ ) {
		MSG_WritePlayerStateField( from, to, msg, &msg_playerStateFields[i] );
	}

	if ( msg->extended ) {
		const playerState_t *oldPs = (const playerState_t *)from;
		const playerState_t *newPs = (const playerState_t *)to;
		for ( i = 2; i < WEAPON_MASK_WORDS; i++ ) {
			MSG_WriteDeltaKey( msg, 0, oldPs->weapons[i], newPs->weapons[i], 32 );
			MSG_WriteDeltaKey( msg, 0, oldPs->weaponrechamber[i], newPs->weaponrechamber[i], 32 );
		}
	}
	statBits = 0;
	for ( i = 0; i < PS_STATS_COUNT; i++ ) {
		if ( *(const int *)( from + PS_STATS_OFFSET + i * 4 )
			 != *(const int *)( to + PS_STATS_OFFSET + i * 4 ) ) {
			statBits |= 1u << i;
		}
	}
	if ( !statBits ) {
		MSG_WriteBit0( msg );
	} else {
		MSG_WriteBit1( msg );
		MSG_WriteBits( msg, (int)statBits, PS_STATS_BITS );
		for ( i = 0; i < PS_STATS_COUNT; i++ ) {
			if ( statBits & ( 1u << i ) ) {
				MSG_WritePlayerStateStat( msg, to, i );
			}
		}
	}

	for ( i = 0; i < PS_AMMO_GROUPS; i++ ) {
		ammoBits[i] = MSG_PlayerStateAmmoGroupBits( from, to, PS_AMMO_OFFSET, i );
		anyAmmo |= ammoBits[i];
	}
	if ( !anyAmmo ) {
		MSG_WriteBit0( msg );
	} else {
		MSG_WriteBit1( msg );
		for ( i = 0; i < PS_AMMO_GROUPS; i++ ) {
			if ( !ammoBits[i] ) {
				MSG_WriteBit0( msg );
				continue;
			}
			MSG_WriteBit1( msg );
			MSG_WritePlayerStateAmmoGroup( msg, to, PS_AMMO_OFFSET, i, ammoBits[i] );
		}
	}

	for ( i = 0; i < PS_AMMO_GROUPS; i++ ) {
		clipBits = MSG_PlayerStateAmmoGroupBits( from, to, PS_AMMOCLIP_OFFSET, i );
		if ( !clipBits ) {
			MSG_WriteBit0( msg );
			continue;
		}
		MSG_WriteBit1( msg );
		MSG_WritePlayerStateAmmoGroup( msg, to, PS_AMMOCLIP_OFFSET, i, clipBits );
	}

	if ( memcmp( from + PS_OBJECTIVE_OFFSET, to + PS_OBJECTIVE_OFFSET,
				 PS_OBJECTIVE_COUNT * PS_OBJECTIVE_SIZE ) == 0 ) {
		MSG_WriteBit0( msg );
	} else {
		MSG_WriteBit1( msg );
		for ( i = 0; i < PS_OBJECTIVE_COUNT; i++ ) {
			const byte *oldObj = from + PS_OBJECTIVE_OFFSET + i * PS_OBJECTIVE_SIZE;
			const byte *newObj = to + PS_OBJECTIVE_OFFSET + i * PS_OBJECTIVE_SIZE;

			MSG_WriteBits( msg, *(const int *)newObj, PS_OBJECTIVE_STATE_BITS );
			MSG_WriteDeltaFields_0( qfalse, msg, oldObj, newObj,
									MSG_OBJECTIVE_FIELDS, msg_objectiveFields );
		}
	}

	if ( memcmp( from + PS_HUD_CURRENT_OFFSET, to + PS_HUD_CURRENT_OFFSET,
				 PS_HUD_TOTAL_BYTES ) == 0 ) {
		MSG_WriteBit0( msg );
	} else {
		MSG_WriteBit1( msg );
		MSG_WriteDeltaHudElems( (const hudelem_t *)( from + PS_HUD_ARCHIVE_OFFSET ),
								PS_HUD_ELEMS, msg,
								(const hudelem_t *)( to + PS_HUD_ARCHIVE_OFFSET ) );
		MSG_WriteDeltaHudElems( (const hudelem_t *)( from + PS_HUD_CURRENT_OFFSET ),
								PS_HUD_ELEMS, msg,
								(const hudelem_t *)( to + PS_HUD_CURRENT_OFFSET ) );
	}
}

/* MSG_ReadDeltaPlayerstate  0x00447860  VERIFIED */
void MSG_ReadDeltaPlayerstate( msg_t *msg, const byte *from, byte *to, int number ) {
	static byte nullState[sizeof( playerState_t )];
	int i, j, lastChanged;
	unsigned int statBits;
	unsigned int anyAmmo = 0;
	qboolean print;

	(void)number;

	if ( !from ) {
		memset( nullState, 0, sizeof( nullState ) );
		from = nullState;
	}

	memcpy( to, from, sizeof( playerState_t ) );

	print = qfalse;
	if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -2 ) ) {
		print = qtrue;
		Com_Printf( "%3i: playerstate ", msg->readcount );
	}

	lastChanged = MSG_ReadByte( msg );
	if ( lastChanged > MSG_PLAYERSTATE_FIELDS ) {
		lastChanged = MSG_PLAYERSTATE_FIELDS;
	}
	for ( i = 0; i < lastChanged; i++ ) {
		MSG_ReadPlayerStateField( from, msg, to, &msg_playerStateFields[i], print );
	}

	if ( msg->extended ) {
		const playerState_t *oldPs = (const playerState_t *)from;
		playerState_t *newPs = (playerState_t *)to;
		for ( i = 2; i < WEAPON_MASK_WORDS; i++ ) {
			newPs->weapons[i] = MSG_ReadDeltaKey( msg, 0, oldPs->weapons[i], 32 );
			newPs->weaponrechamber[i] = MSG_ReadDeltaKey( msg, 0, oldPs->weaponrechamber[i], 32 );
		}
	}
	if ( MSG_ReadBit( msg ) ) {
		statBits = (unsigned int)MSG_ReadBits( msg, PS_STATS_BITS );
		for ( i = 0; i < PS_STATS_COUNT; i++ ) {
			if ( statBits & ( 1u << i ) ) {
				*(int *)( to + PS_STATS_OFFSET + i * 4 ) = MSG_ReadPlayerStateStat( msg, i );
			}
		}
	}

	if ( MSG_ReadBit( msg ) ) {
		for ( i = 0; i < PS_AMMO_GROUPS; i++ ) {
			if ( MSG_ReadBit( msg ) ) {
				MSG_ReadPlayerStateAmmoGroup( msg, to, PS_AMMO_OFFSET, i );
			}
		}
	}

	for ( i = 0; i < PS_AMMO_GROUPS; i++ ) {
		if ( MSG_ReadBit( msg ) ) {
			MSG_ReadPlayerStateAmmoGroup( msg, to, PS_AMMOCLIP_OFFSET, i );
		}
	}

	if ( MSG_ReadBit( msg ) ) {
		for ( i = 0; i < PS_OBJECTIVE_COUNT; i++ ) {
			const byte *oldObj = from + PS_OBJECTIVE_OFFSET + i * PS_OBJECTIVE_SIZE;
			byte *newObj = to + PS_OBJECTIVE_OFFSET + i * PS_OBJECTIVE_SIZE;

			j = MSG_ReadBits( msg, PS_OBJECTIVE_STATE_BITS );
			*(int *)newObj = j;
			MSG_ReadDeltaFields( msg, MSG_OBJECTIVE_FIELDS, oldObj, newObj,
								 msg_objectiveFields );
		}
	}

	if ( MSG_ReadBit( msg ) ) {
		MSG_ReadDeltaHudElems( msg, (const hudelem_t *)( from + PS_HUD_ARCHIVE_OFFSET ),
							   (hudelem_t *)( to + PS_HUD_ARCHIVE_OFFSET ), PS_HUD_ELEMS );
		MSG_ReadDeltaHudElems( msg, (const hudelem_t *)( from + PS_HUD_CURRENT_OFFSET ),
							   (hudelem_t *)( to + PS_HUD_CURRENT_OFFSET ), PS_HUD_ELEMS );
	}
}
