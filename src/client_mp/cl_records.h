#ifndef CL_RECORDS_H
#define CL_RECORDS_H
#include "../universal/protocol_limits.h"

#define ES_NUMBER               0x00
#define ES_EFLAGS               0x08
#define ES_ORIGIN               0x18
#define ES_STANCE               0xE0
#define ES_SIZE                 0xF0

/* clientState_t -- 92 bytes. Stride of cl.parseClients at 0x01531134, and the 22 netFields at 0x00541FC0 describe it. ------------------------------------------------------------------------ */
#define CS_NUMBER               0x00
#define CS_SIZE                 0x5C

#define SNAP_VALID              0x0000
#define SNAP_SNAPFLAGS          0x0004
#define SNAP_SERVERTIME         0x0008
#define SNAP_MESSAGENUM         0x000C
#define SNAP_DELTANUM           0x0010
#define SNAP_PING               0x0014
#define SNAP_PS                 0x001C
#define SNAP_NUMENTITIES        ( 0x20EC + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_NUMCLIENTS         ( 0x20F0 + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_PARSEENTITIESNUM   ( 0x20F4 + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_PARSECLIENTSNUM    ( 0x20F8 + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_SERVERCOMMANDNUM   ( 0x20FC + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_SIZE               ( 0x2100 + PLAYERSTATE_EXTRA_BYTES )

#define SNAP_OUT_SNAPFLAGS      0x00000
#define SNAP_OUT_PING           0x00004
#define SNAP_OUT_SERVERTIME     0x00008
#define SNAP_OUT_PS             0x0000C
#define SNAP_OUT_NUMENTITIES    ( 0x020DC + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_OUT_NUMCLIENTS     ( 0x020E0 + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_OUT_ENTITIES       ( 0x020E4 + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_OUT_CLIENTS        ( 0x110E4 + PLAYERSTATE_EXTRA_BYTES )
#define SNAP_OUT_CMDSEQUENCE    ( 0x127E8 + PLAYERSTATE_EXTRA_BYTES )

#define MAX_ENTITIES_IN_SNAPSHOT  0x100
#define MAX_CLIENTS_IN_SNAPSHOT   0x40

#include "../qcommon/cmd_history.h"

#define GS_STRINGOFFSETS        0x0000
#define GS_STRINGDATA           0x2000
#define GS_DATACOUNT            ( GS_STRINGDATA + EXTENDED_MAX_GAMESTATE_CHARS )
#define GS_SIZE                 ( GS_DATACOUNT + 4 )
#define CS_SYSTEMINFO           1
#define MAX_CONFIGSTRINGS       0x800

#define cl_gameState_stringData \
	( (char *)cl_gameState_stringOffsets + GS_STRINGDATA )
#define cl_gameState_dataCount \
	( *(int *)( (char *)cl_gameState_stringOffsets + GS_DATACOUNT ) )

#define word_78E912             ( cin_sqrTable + 1 )
#define word_78E914             ( cin_sqrTable + 2 )
#define word_78E916             ( cin_sqrTable + 3 )
#define word_78E918             ( cin_sqrTable + 4 )
#define word_78E91A             ( cin_sqrTable + 5 )
#define word_78E91C             ( cin_sqrTable + 6 )
#define word_78E91E             ( cin_sqrTable + 7 )
#define word_78EA10             ( cin_sqrTable + 128 )
#define word_78EA12             ( cin_sqrTable + 129 )
#define word_78EA14             ( cin_sqrTable + 130 )
#define word_78EA16             ( cin_sqrTable + 131 )
#define word_78EA18             ( cin_sqrTable + 132 )
#define word_78EA1A             ( cin_sqrTable + 133 )
#define word_78EA1C             ( cin_sqrTable + 134 )
#define word_78EA1E             ( cin_sqrTable + 135 )

#define cin_file_p1             ( cin_file + 1 )
#define cin_file_p6             ( cin_file + 6 )
#define cin_file_p15             ( cin_file + 15 )
#define cin_mcomp_p16            ( cin_mcomp + 16 )

extern field_t                  g_consoleField;
extern field_t                  chatField;
extern field_t                  historyEditLines[32];

#define dword_142F640           ( g_consoleField.cursor )
#define dword_142F644           ( g_consoleField.scroll )
#define dword_142F648           ( g_consoleField.widthInChars )
#define dword_142F64C           ( g_consoleField.widthInPixels )
#define dword_142F650           ( *(int *)&g_consoleField.charWidth )
#define dword_142F654           ( *(int *)&g_consoleField.charHeight )
#define dword_142F658           ( g_consoleField.fixedWidth )
#define text_in                 ( g_consoleField.buffer )
#define unk_142F65D             ( g_consoleField.buffer + 1 )

#define dword_14303A0           ( chatField.cursor )
#define dword_14303A4           ( chatField.scroll )
#define dword_14303A8           ( chatField.widthInChars )
#define dword_14303AC           ( chatField.widthInPixels )
#define dword_14303B0           ( *(int *)&chatField.charWidth )
#define dword_14303B4           ( *(int *)&chatField.charHeight )
#define dword_14303B8           ( chatField.fixedWidth )
#define byte_14303BC            ( chatField.buffer )

#define unk_14304C0             ( (unsigned char *)historyEditLines )

#define byte_155F3CB            ( cls_servername[255] )
#define unk_15EEBBD             ( (unsigned char *)clc_downloadList + 1 )

#define byte_15EEBAB            ( cls_downloadName[255] )
#define byte_15CE987            ( &clc_serverMessage[255] )
#define byte_15CA47B            ( cls_autoupdateServerNames_0_[63] )
#define byte_15CA4BB            ( cls_autoupdateServerNames_1_[63] )
#define byte_15CA4FB            ( cls_autoupdateServerNames_2_[63] )
#define byte_15CA53B            ( cls_autoupdateServerNames_3_[63] )
#define byte_15CA57B            ( cls_autoupdateServerNames_4_[63] )

#endif
