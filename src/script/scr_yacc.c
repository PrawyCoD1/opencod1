/*
 * script/scr_yacc.c
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/script/scr_yacc.c
 *
 * Retail range 0x0047E420-0x00480D90, 27 functions.
 *
 * @fidelity: verified
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#include "../qcommon/qcommon.h"
#include "../qcommon/hexrays_shim.h"
#include "../qcommon/cod1_globals.h"

typedef unsigned int sval_t;

typedef struct {
	sval_t			val;
	unsigned int	sourcePos;
} yystype;

typedef struct yy_buffer_state {
	FILE	*yy_input_file;
	char	*yy_ch_buf;
	char	*yy_buf_pos;
	int		yy_buf_size;
	int		yy_n_chars;
	int		yy_is_our_buffer;
	int		yy_is_interactive;
	int		yy_at_bol;
	int		yy_fill_buffer;
	int		yy_buffer_status;
} *YY_BUFFER_STATE;

extern sval_t node0( int type );
extern sval_t node1( int type, sval_t a );
extern sval_t node2( int type, sval_t a, sval_t b );
extern sval_t node2_( sval_t a, sval_t b );
extern sval_t node3( int type, sval_t a, sval_t b, sval_t c );
extern sval_t node4( int type, sval_t a, sval_t b, sval_t c, sval_t d );
extern sval_t node5( int type, sval_t a, sval_t b, sval_t c, sval_t d, sval_t e );
extern sval_t node6( int type, sval_t a, sval_t b, sval_t c, sval_t d, sval_t e, sval_t f );
extern sval_t linked_list_end( sval_t a );
extern sval_t prepend_node( sval_t a, sval_t list );
extern sval_t append_node( sval_t list, sval_t a );

void QDECL CompileError( unsigned int sourcePos, const char *format, ... );
unsigned short SL_GetLowercaseStringOfLen( const char *text, unsigned char user,
										   unsigned int size, int type );
unsigned short SL_GetStringOfLen( const char *text, unsigned char user,
								  unsigned int size, int type );

#define yyleng					yyleng	/* 0x016C36A8 */
#define yynerrs					yynerrs	/* 0x016C36B0 */
#define yy_start				yy_start	/* 0x01407348 */
#define yyout					dword_1407350	/* 0x01407350 */
#define yy_init					yy_init	/* 0x005745D8 */
#define yy_last_accepting_cpos	dword_A7A5F0	/* 0x00A7A5F0 */
#define yy_last_accepting_state	dword_A7A60C	/* 0x00A7A60C */
#define g_parseTreeRoot			dword_A7A5FC	/* 0x00A7A5FC -- ScriptParse's out value */
#define g_sourceCharPos			dword_A7A600	/* 0x00A7A600 -- running offset into the script text */
#define g_scriptText			dword_A7A604	/* 0x00A7A604 -- YY_INPUT read cursor */
#define g_scrStatementCount		scrCompilePub_scriptCount	/* 0x008E60B8 -- bumped per statement by rules 31 and 33 */

#define YY_CURRENT_BUFFER	((YY_BUFFER_STATE)(unsigned int)yy_current_buffer)
#define YY_CBUF_P			((char *)(unsigned int)yy_c_buf_p)
#define YY_TEXT				((char *)(unsigned int)yytext_ptr)

yystype yylval;

#define YY_BUF_SIZE			16384		/* 0x4000, retail */
#define YY_END_OF_BUFFER	94
#define YY_JAM_BASE			395
#define YY_META_THRESHOLD	245
#define YY_JAM_STATE		244

#define EOB_ACT_CONTINUE_SCAN	0
#define EOB_ACT_END_OF_FILE		1
#define EOB_ACT_LAST_MATCH		2

#define YYFLAG			(-32768)
#define YYLAST			1066
#define YYNTBASE		90
#define YYMAXUTOK		343
#define YYUNDEFTOK		115
#define YYFINAL			251
#define YYTERROR		1
#define YYEMPTY			(-2)
#define YYMAXDEPTH		10000
#define YYERRTOKEN		257

static YY_BUFFER_STATE	yy_create_buffer( int size, FILE *file );
static void				yy_load_buffer_state( void );
static int				yy_get_next_buffer( void );
static int				yy_get_previous_state( void );
static int				yy_try_NUL_trans( int yy_current_state );
static void				yy_fatal_error( const char *msg );

void					yyrestart( FILE *input_file );
void					yy_switch_to_buffer( YY_BUFFER_STATE new_buffer );
void					yy_delete_buffer( YY_BUFFER_STATE b );
void					yy_init_buffer( YY_BUFFER_STATE b, FILE *file );
void					yy_flush_buffer( YY_BUFFER_STATE b );
YY_BUFFER_STATE			yy_scan_buffer( char *base, unsigned int size );
YY_BUFFER_STATE			yy_scan_string( const char *str );
YY_BUFFER_STATE			yy_scan_bytes( int len, const char *bytes );

short					TextValue( int length, const char *text );
short					StringValue( int length, const char *text );
int						IntegerValue( const char *text );
int						FloatValue( const char *text );

int						yylex( void );
int						yywrap( void );
int						yyerror( void );

static const unsigned char yytranslate[344] = {   /* 0x00572D70  VERIFIED md5 6fb4c39204f2129294590fc037b30706 */
	0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
	66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
	82, 83, 84, 85, 86, 87, 88, 89,
};

static const short yyr1[128] = {   /* 0x00572EC8  VERIFIED md5 405cddbef29c8606f14a2c756ceff74f */
	0, 90, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
	91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
	91, 91, 91, 92, 92, 93, 93, 94, 94, 95, 95, 96,
	96, 97, 97, 98, 98, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 100, 100, 100, 101, 101, 101, 101, 101, 101, 101,
	101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101,
	101, 101, 101, 101, 101, 102, 102, 103, 103, 103, 103, 103,
	103, 103, 103, 104, 104, 104, 104, 105, 105, 106, 106, 106,
	107, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111, 111,
	112, 112, 113, 114, 114, 0, 0, 0,
};

static const short yyr2[128] = {   /* 0x00572FC8  VERIFIED md5 6a567bd9a645035dd469b8728bcef198 */
	0, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,
	2, 2, 2, 1, 0, 1, 1, 3, 1, 3, 2, 1,
	5, 1, 2, 4, 5, 3, 1, 1, 2, 2, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 1,
	1, 1, 3, 4, 1, 1, 3, 2, 1, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5,
	5, 5, 1, 1, 6, 0, 1, 2, 3, 5, 7, 5,
	8, 5, 3, 1, 3, 2, 1, 2, 0, 3, 1, 0,
	3, 1, 0, 3, 1, 3, 1, 3, 1, 7, 9, 5,
	2, 0, 0, 3, 0, 0, 0, 0,
};

static const short yydefact[252] = {   /* 0x005730C8  VERIFIED md5 77e884977bd6c0c0896cfdc21e508804 */
	124, 121, 1, 0, 0, 0, 0, 120, 123, 110, 0, 0,
	109, 0, 0, 110, 0, 0, 0, 0, 104, 108, 119, 0,
	0, 104, 64, 46, 47, 104, 117, 107, 0, 0, 0, 42,
	43, 99, 68, 0, 0, 50, 51, 52, 53, 54, 0, 0,
	0, 61, 0, 30, 0, 0, 0, 86, 87, 0, 59, 60,
	104, 0, 35, 56, 37, 0, 48, 0, 49, 0, 102, 103,
	0, 0, 0, 0, 0, 0, 0, 0, 106, 48, 2, 49,
	0, 0, 57, 44, 45, 58, 67, 69, 29, 0, 0, 38,
	0, 0, 0, 34, 0, 0, 101, 0, 0, 0, 107, 0,
	0, 55, 0, 0, 0, 0, 0, 0, 70, 71, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 91, 0, 92, 25,
	26, 21, 22, 23, 24, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41,
	0, 0, 0, 0, 0, 28, 0, 100, 0, 98, 33, 0,
	0, 0, 62, 0, 0, 0, 0, 107, 66, 72, 73, 74,
	75, 76, 77, 78, 79, 80, 81, 118, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 105, 0, 31, 0, 0, 27, 0, 0, 0, 39,
	63, 112, 0, 114, 0, 116, 0, 0, 0, 36, 93, 95,
	89, 97, 0, 82, 0, 83, 0, 84, 0, 85, 40, 0,
	90, 0, 88, 111, 113, 115, 94, 0, 96, 0, 0, 0,
};

static const short yydefgoto[28] = {   /* 0x005732C0  VERIFIED md5 9687eceff2b645baed645e211d66b94a */
	249, 80, 212, 61, 62, 63, 64, 65, 81, 82, 83, 69,
	241, 70, 71, 24, 84, 13, 218, 220, 222, 7, 2, 3,
	1, 0, 0, 0,
};

static const short yypact[252] = {   /* 0x005732F8  VERIFIED md5 3ea3b2b6fff2920b4349772acf0b1436 */
	-32768, -33, 4, -29, 5, 21, 23, -32768, -32768, 35, 40, 52,
	-32768, 1, 48, 35, 70, 59, 71, 7, -32768, -32768, -32768, 103,
	312, -32768, -8, -32768, -32768, -32768, -32768, 741, 30, 65, 118, -32768,
	-32768, -32768, 741, 741, -1, -32768, -32768, -32768, -32768, -32768, 115, 116,
	117, -32768, 124, -32768, 120, 910, 113, -32768, -32768, 648, -32768, -32768,
	-32768, 78, -32768, -32768, -32768, 143, -4, 208, 271, 119, -32768, -32768,
	396, 480, 741, 741, 741, 741, 741, 741, 1003, -32768, 53, -32768,
	16, 741, -32768, -32768, -32768, -32768, 1003, 1003, 144, 145, 82, -32768,
	741, 741, 564, -32768, 741, 9, -32768, 106, 227, 155, 741, 819,
	156, -32768, 153, 159, 160, 163, 164, 741, -32768, -32768, 741, 741,
	741, 741, 741, 741, 741, 741, 741, 741, -32768, 87, -32768, -32768,
	-32768, -32768, -32768, -32768, -32768, 741, 741, 741, 741, 741, 741, 741,
	741, 741, 741, 741, 741, 741, 741, 741, 741, 741, 741, -32768,
	741, 948, 172, 353, 437, 741, 521, -32768, 168, -32768, 169, 18,
	732, 985, -32768, 741, 741, 741, 741, 741, 1003, 1003, 1003, 1003,
	1003, 1003, 1003, 1003, 1003, 1003, 1003, -32768, 1020, 1036, 392, 475,
	558, 640, 640, 77, 77, 77, 77, -5, -5, 66, 66, -32768,
	-32768, -32768, 1003, 167, -32768, 648, 648, 1003, 142, 648, 741, -32768,
	-32768, 1003, 26, 1003, 49, 1003, 50, 605, 56, -32768, 130, -32768,
	827, -32768, 689, -32768, 179, -32768, 741, -32768, 741, -32768, -32768, 648,
	-32768, 174, -32768, -32768, 1003, 1003, -32768, 648, -32768, 190, 194, -32768,
};

static const short yypgoto[28] = {   /* 0x005734F0  VERIFIED md5 0d3e9d5b347b6de740989614ae0c7cc5 */
	-32768, -7, -32768, 81, -32768, -32768, 157, -63, -22, -24, -17, -30,
	-32768, -39, 97, -13, -91, 184, -32768, -32768, -32768, -32768, -32768, -32768,
	-32768, 0, 0, 0,
};

static const short yytable[1068] = {   /* 0x00573528  VERIFIED md5 b0ffbf61cc1edb14da41a788c7fe7899 */
	67, -32, 66, 92, 114, -122, -65, 68, 4, 8, 93, 16,
	72, 92, 9, 167, 73, 23, 103, 114, 107, 150, 151, 152,
	153, 154, 155, 11, 215, 101, 10, 90, 91, 67, -65, 66,
	231, 17, 114, 12, 68, 85, 86, 17, 108, 14, 163, 104,
	67, 67, 66, 66, 156, 40, 156, 68, 68, 92, 18, 233,
	235, 15, 232, 21, 107, -29, 238, 131, 132, 133, 134, 135,
	136, 51, 67, 5, 66, 20, 157, 109, 67, 68, 66, 51,
	224, 234, 236, 68, 108, 159, 160, 6, 156, 162, 152, 153,
	154, 40, 87, 88, 169, 148, 149, 150, 151, 152, 153, 154,
	176, 22, 25, 177, 178, 179, 180, 181, 182, 183, 184, 185,
	186, 94, 89, 109, 96, 97, 98, 51, 99, 100, 188, 189,
	190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201,
	202, 203, 204, 205, 94, 206, 102, 105, 106, -32, 211, 158,
	85, 128, 164, 166, 170, 157, 171, 94, 217, 219, 221, 223,
	172, 173, 226, 227, 174, 175, 229, 187, 208, 214, -31, 225,
	228, 239, 94, 243, 247, 67, 67, 66, 66, 67, 250, 66,
	68, 68, 251, 161, 68, 95, 240, 19, 246, 0, 0, 0,
	67, 0, 66, 230, 248, 0, 0, 68, 92, 0, 0, 67,
	0, 66, 0, 107, 0, 0, 68, 67, 0, 66, 0, 244,
	0, 245, 68, 26, 27, 28, 29, 0, 31, 0, 32, 0,
	0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0,
	40, 0, 33, 0, 0, 34, 0, 0, 35, 36, 0, 0,
	0, 37, 0, 0, 0, 38, 39, 40, 41, 42, 43, 44,
	45, 46, 109, 47, 48, 0, 51, 110, 111, 112, 0, 0,
	0, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 49,
	50, 51, 0, 0, 0, 52, 53, 54, 55, 56, 57, 115,
	58, 59, 60, 165, 26, 27, 28, 29, 30, 31, 0, 32,
	0, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
	127, 0, 0, 33, 0, 0, 34, 0, 0, 35, 36, 0,
	0, 0, 37, 0, 0, 0, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 209, 47, 48, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
	49, 50, 51, 0, 0, 0, 52, 53, 54, 55, 56, 57,
	0, 58, 59, 60, 26, 27, 28, 29, 129, 31, 0, 32,
	140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 33, 0, 0, 34, 0, 0, 35, 36, 0,
	0, 0, 37, 0, 0, 0, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 210, 47, 48, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
	49, 50, 51, 0, 0, 0, 52, 53, 54, 55, 56, 57,
	0, 58, 59, 60, 26, 27, 28, 29, 130, 31, 0, 32,
	141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152,
	153, 154, 0, 33, 0, 0, 34, 0, 0, 35, 36, 0,
	0, 0, 37, 0, 0, 0, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 213, 47, 48, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
	49, 50, 51, 0, 0, 0, 52, 53, 54, 55, 56, 57,
	0, 58, 59, 60, 26, 27, 28, 29, 0, 31, 0, 32,
	142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
	154, 0, 0, 33, 0, 0, 34, 0, 0, 35, 36, 0,
	0, 0, 37, 0, 0, 0, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 237, 47, 48, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
	49, 50, 51, 0, 0, 0, 52, 53, 54, 55, 56, 57,
	0, 58, 59, 60, 26, 27, 28, 29, 0, 31, 0, 32,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 0,
	0, 0, 0, 33, 0, 0, 34, 0, 0, 35, 36, 0,
	0, 0, 0, 0, 0, 0, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 242, 47, 48, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
	49, 50, 51, 0, 0, 0, 52, 0, 0, 55, 56, 57,
	0, 58, 59, 60, 26, 27, 28, 0, 0, 31, 0, 168,
	86, 26, 27, 28, 0, 0, 31, 0, 32, 0, 0, 0,
	0, 0, 0, 33, 0, 0, 34, 74, 75, 35, 36, 0,
	33, 0, 0, 34, 74, 75, 35, 36, 40, 41, 42, 43,
	44, 45, 0, 0, 0, 40, 41, 42, 43, 44, 45, 0,
	0, 0, 0, 0, 0, 0, 76, 77, 78, 79, 0, 0,
	49, 50, 51, 76, 77, 78, 79, 0, 0, 49, 50, 51,
	0, 58, 59, 0, 0, 0, 0, 26, 27, 28, 58, 59,
	31, 0, 168, 26, 27, 28, 0, 0, 31, 0, 32, 0,
	0, 0, 0, 0, 0, 0, 33, 0, 0, 34, 74, 75,
	35, 36, 33, 0, 0, 34, 0, 0, 35, 36, 0, 40,
	41, 42, 43, 44, 45, 38, 39, 40, 41, 42, 43, 44,
	45, 0, 0, 0, 0, 0, 0, 0, 0, 76, 77, 78,
	79, 0, 0, 49, 50, 51, 0, 0, 0, 0, 0, 49,
	50, 51, 0, 0, 58, 59, 0, 0, 55, 56, 57, 0,
	58, 59, 26, 27, 28, 0, 0, 31, 0, 32, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 33, 0, 0, 34, 0, 0, 35, 36, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 40, 41, 42, 43, 44, 45,
	207, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147,
	148, 149, 150, 151, 152, 153, 154, 0, 0, 0, 49, 50,
	51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 58,
	59, 216, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
	147, 148, 149, 150, 151, 152, 153, 154, 137, 138, 139, 140,
	141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152,
	153, 154, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147,
	148, 149, 150, 151, 152, 153, 154, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 0,
};

static const short yycheck[1068] = {   /* 0x00573D80  VERIFIED md5 453df993dc0ae9acc8b8f09e4a04f29c */
	24, 9, 24, 4, 67, 38, 10, 24, 4, 38, 11, 10,
	25, 4, 9, 106, 29, 10, 57, 82, 11, 26, 27, 28,
	29, 30, 10, 4, 10, 53, 9, 38, 39, 57, 38, 57,
	10, 36, 101, 4, 57, 11, 12, 36, 35, 5, 37, 60,
	72, 73, 72, 73, 36, 44, 36, 72, 73, 4, 10, 10,
	10, 9, 36, 4, 11, 73, 10, 74, 75, 76, 77, 78,
	79, 74, 98, 71, 98, 7, 85, 70, 104, 98, 104, 74,
	175, 36, 36, 104, 35, 96, 97, 87, 36, 100, 28, 29,
	30, 44, 33, 34, 107, 24, 25, 26, 27, 28, 29, 30,
	115, 38, 7, 118, 119, 120, 121, 122, 123, 124, 125, 126,
	127, 40, 4, 70, 9, 9, 9, 74, 4, 9, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150,
	151, 152, 153, 154, 67, 156, 37, 73, 9, 9, 161, 73,
	11, 38, 52, 4, 4, 168, 9, 82, 171, 172, 173, 174,
	9, 9, 209, 210, 9, 9, 213, 88, 4, 9, 9, 12,
	38, 51, 101, 4, 10, 209, 210, 209, 210, 213, 0, 213,
	209, 210, 0, 98, 213, 40, 228, 15, 239, -1, -1, -1,
	228, -1, 228, 214, 247, -1, -1, 228, 4, -1, -1, 239,
	-1, 239, -1, 11, -1, -1, 239, 247, -1, 247, -1, 234,
	-1, 236, 247, 4, 5, 6, 7, -1, 9, -1, 11, -1,
	-1, -1, -1, 35, -1, -1, -1, -1, -1, -1, -1, -1,
	44, -1, 27, -1, -1, 30, -1, -1, 33, 34, -1, -1,
	-1, 38, -1, -1, -1, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 70, 52, 53, -1, 74, 75, 76, 77, -1, -1,
	-1, -1, -1, -1, 84, -1, -1, -1, -1, -1, -1, 72,
	73, 74, -1, -1, -1, 78, 79, 80, 81, 82, 83, 40,
	85, 86, 87, 88, 4, 5, 6, 7, 8, 9, -1, 11,
	-1, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
	65, -1, -1, 27, -1, -1, 30, -1, -1, 33, 34, -1,
	-1, -1, 38, -1, -1, -1, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 10, 52, 53, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	72, 73, 74, -1, -1, -1, 78, 79, 80, 81, 82, 83,
	-1, 85, 86, 87, 4, 5, 6, 7, 8, 9, -1, 11,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	28, 29, 30, 27, -1, -1, 30, -1, -1, 33, 34, -1,
	-1, -1, 38, -1, -1, -1, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 10, 52, 53, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	72, 73, 74, -1, -1, -1, 78, 79, 80, 81, 82, 83,
	-1, 85, 86, 87, 4, 5, 6, 7, 8, 9, -1, 11,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, -1, 27, -1, -1, 30, -1, -1, 33, 34, -1,
	-1, -1, 38, -1, -1, -1, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 10, 52, 53, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	72, 73, 74, -1, -1, -1, 78, 79, 80, 81, 82, 83,
	-1, 85, 86, 87, 4, 5, 6, 7, -1, 9, -1, 11,
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, -1, -1, 27, -1, -1, 30, -1, -1, 33, 34, -1,
	-1, -1, 38, -1, -1, -1, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 10, 52, 53, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	72, 73, 74, -1, -1, -1, 78, 79, 80, 81, 82, 83,
	-1, 85, 86, 87, 4, 5, 6, 7, -1, 9, -1, 11,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, -1,
	-1, -1, -1, 27, -1, -1, 30, -1, -1, 33, 34, -1,
	-1, -1, -1, -1, -1, -1, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 10, 52, 53, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	72, 73, 74, -1, -1, -1, 78, -1, -1, 81, 82, 83,
	-1, 85, 86, 87, 4, 5, 6, -1, -1, 9, -1, 11,
	12, 4, 5, 6, -1, -1, 9, -1, 11, -1, -1, -1,
	-1, -1, -1, 27, -1, -1, 30, 31, 32, 33, 34, -1,
	27, -1, -1, 30, 31, 32, 33, 34, 44, 45, 46, 47,
	48, 49, -1, -1, -1, 44, 45, 46, 47, 48, 49, -1,
	-1, -1, -1, -1, -1, -1, 66, 67, 68, 69, -1, -1,
	72, 73, 74, 66, 67, 68, 69, -1, -1, 72, 73, 74,
	-1, 85, 86, -1, -1, -1, -1, 4, 5, 6, 85, 86,
	9, -1, 11, 4, 5, 6, -1, -1, 9, -1, 11, -1,
	-1, -1, -1, -1, -1, -1, 27, -1, -1, 30, 31, 32,
	33, 34, 27, -1, -1, 30, -1, -1, 33, 34, -1, 44,
	45, 46, 47, 48, 49, 42, 43, 44, 45, 46, 47, 48,
	49, -1, -1, -1, -1, -1, -1, -1, -1, 66, 67, 68,
	69, -1, -1, 72, 73, 74, -1, -1, -1, -1, -1, 72,
	73, 74, -1, -1, 85, 86, -1, -1, 81, 82, 83, -1,
	85, 86, 4, 5, 6, -1, -1, 9, -1, 11, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 27, -1, -1, 30, -1, -1, 33, 34, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, 44, 45, 46, 47, 48, 49,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, -1, -1, -1, 72, 73,
	74, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 85,
	86, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 30, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,
};

static const short yy_accept[248] = {   /* 0x005745E0  VERIFIED md5 ba852008d4e77d3a01e71a03ee514598 */
	0, 0, 0, 0, 0, 0, 0, 94, 92, 1, 4, 33,
	92, 43, 32, 19, 11, 12, 30, 28, 37, 29, 38, 31,
	35, 40, 42, 22, 41, 23, 39, 90, 13, 14, 18, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 9, 17, 10, 34, 3, 3, 4, 21, 0, 7,
	0, 89, 0, 0, 67, 0, 16, 60, 0, 0, 0, 0,
	65, 56, 63, 57, 64, 36, 0, 88, 6, 5, 66, 0,
	35, 0, 75, 26, 24, 20, 25, 27, 90, 0, 59, 90,
	90, 90, 90, 90, 84, 90, 90, 90, 90, 90, 52, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 58, 15, 2,
	0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 5, 0,
	36, 61, 62, 91, 90, 90, 90, 90, 90, 90, 90, 90,
	55, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	0, 0, 0, 0, 0, 0, 0, 91, 51, 90, 80, 90,
	90, 53, 90, 90, 50, 90, 90, 90, 48, 90, 90, 87,
	90, 45, 90, 0, 0, 0, 0, 69, 0, 72, 82, 90,
	90, 85, 86, 49, 90, 90, 90, 90, 90, 90, 54, 0,
	0, 68, 0, 0, 90, 90, 78, 44, 79, 46, 90, 90,
	0, 0, 70, 0, 90, 81, 90, 90, 0, 0, 71, 83,
	90, 76, 74, 0, 47, 90, 0, 90, 0, 90, 0, 90,
	0, 77, 0, 73, 0, 0, 0, 0,
};

static const int yy_ec[256] = {   /* 0x005747D0  VERIFIED md5 239da89e6ef0aa0d1fe9026c4359dc66 */
	0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1,
	1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 5, 6,
	1, 7, 8, 1, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 19,
	20, 21, 22, 23, 1, 24, 24, 24, 24, 25, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 26, 27, 28, 29, 30,
	1, 31, 32, 33, 34, 35, 36, 37, 38, 39, 24, 40,
	41, 42, 43, 44, 24, 24, 45, 46, 47, 48, 49, 50,
	24, 51, 52, 53, 54, 55, 56, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1,
};

static const int yy_meta[58] = {   /* 0x00574BD0  VERIFIED md5 6adfe5ee1309a6a8ca2b30cd01c665f0 */
	0, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1,
	4, 4, 1, 3, 1, 1, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 1, 1, 1, 1, 0,
};

static const short yy_base[256] = {   /* 0x00574CB8  VERIFIED md5 607ffa8eb9d3d2a36b34ddbbd8712dd4 */
	0, 391, 390, 0, 0, 54, 55, 392, 395, 395, 389, 369,
	54, 44, 368, 56, 37, 395, 367, 50, 395, 49, 50, 68,
	63, 369, 395, 65, 365, 69, 395, 358, 395, 395, 363, 55,
	60, 66, 67, 72, 73, 41, 76, 74, 79, 81, 87, 80,
	92, 93, 395, 74, 395, 395, 395, 367, 380, 395, 116, 395,
	0, 395, 338, 334, 395, 121, 395, 395, 335, 337, 334, 329,
	395, 395, 395, 395, 395, 113, 336, 395, 395, 0, 395, 357,
	117, 127, 395, 352, 395, 395, 395, 351, 344, 0, 395, 106,
	119, 109, 113, 122, 343, 120, 123, 124, 125, 126, 342, 132,
	133, 135, 134, 137, 140, 136, 144, 147, 150, 395, 395, 395,
	329, 328, 164, 395, 0, 322, 321, 317, 318, 310, 0, 344,
	343, 395, 395, 332, 145, 152, 161, 146, 163, 165, 168, 170,
	331, 171, 172, 174, 175, 178, 177, 176, 182, 183, 181, 188,
	315, 313, 314, 323, 343, 313, 316, 323, 322, 192, 321, 194,
	193, 320, 195, 199, 319, 198, 200, 203, 318, 204, 213, 317,
	215, 208, 218, 296, 305, 331, 293, 395, 296, 395, 311, 216,
	219, 310, 309, 308, 220, 222, 223, 229, 225, 227, 307, 288,
	302, 395, 321, 293, 230, 235, 302, 301, 300, 299, 231, 240,
	290, 293, 395, 313, 241, 295, 242, 243, 286, 273, 395, 285,
	245, 246, 395, 267, 267, 256, 251, 248, 245, 253, 246, 258,
	255, 262, 94, 395, 395, 296, 300, 304, 306, 310, 314, 123,
	316, 0, 0, 0,
};

static const short yy_def[256] = {   /* 0x00574EB8  VERIFIED md5 4c10c1820627cc0ab8066dce40030b0d */
	0, 245, 245, 244, 3, 246, 246, 244, 244, 244, 244, 244,
	247, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 248, 244, 244, 244, 248,
	248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
	248, 248, 244, 244, 244, 244, 244, 244, 244, 244, 247, 244,
	247, 244, 244, 244, 244, 249, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 250, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 248, 251, 244, 248,
	248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
	248, 248, 248, 248, 248, 248, 248, 248, 248, 244, 244, 244,
	244, 244, 249, 244, 249, 244, 244, 244, 244, 244, 250, 244,
	244, 244, 244, 252, 248, 248, 248, 248, 248, 248, 248, 248,
	248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
	244, 244, 244, 244, 244, 244, 244, 252, 248, 248, 248, 248,
	248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248,
	248, 248, 248, 244, 244, 244, 244, 244, 244, 244, 248, 248,
	248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 244,
	244, 244, 244, 244, 248, 248, 248, 248, 248, 248, 248, 248,
	244, 244, 244, 244, 248, 248, 248, 248, 244, 244, 244, 248,
	248, 248, 244, 244, 248, 248, 244, 248, 244, 248, 244, 248,
	244, 248, 244, 244, 0, 244, 244, 244, 244, 244, 244, 244,
	244, 0, 0, 0,
};

static const short yy_nxt[452] = {   /* 0x005750B8  VERIFIED md5 bf38a3a5c1d49ecca9f8e5977705b8d7 */
	0, 8, 10, 9, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31, 31, 32, 8, 33, 34, 31, 35, 36, 37, 38, 39,
	40, 41, 31, 42, 31, 43, 31, 44, 31, 45, 46, 47,
	48, 31, 49, 31, 31, 50, 51, 52, 53, 9, 9, 59,
	61, 65, 73, 75, 66, 55, 55, 77, 93, 68, 76, 74,
	105, 69, 79, 62, 70, 67, 83, 80, 84, 60, 93, 71,
	81, 87, 88, 93, 85, 82, 90, 91, 63, 93, 93, 117,
	78, 97, 95, 93, 93, 93, 99, 93, 103, 96, 93, 93,
	93, 107, 98, 100, 106, 101, 93, 102, 109, 104, 112, 93,
	93, 59, 110, 108, 115, 113, 123, 135, 118, 243, 77, 116,
	83, 93, 84, 114, 93, 111, 85, 131, 93, 131, 85, 60,
	132, 136, 93, 93, 124, 93, 93, 93, 93, 93, 137, 138,
	139, 142, 140, 93, 93, 93, 93, 93, 93, 143, 141, 93,
	145, 123, 144, 93, 93, 93, 93, 149, 150, 93, 153, 93,
	147, 146, 148, 165, 152, 151, 154, 164, 93, 155, 93, 124,
	93, 167, 168, 93, 166, 93, 93, 93, 169, 93, 93, 93,
	93, 93, 172, 173, 93, 93, 93, 178, 170, 174, 176, 93,
	171, 179, 180, 93, 93, 93, 93, 175, 177, 93, 93, 93,
	181, 182, 93, 93, 190, 191, 194, 93, 196, 198, 193, 195,
	93, 192, 93, 93, 199, 93, 93, 93, 197, 93, 93, 200,
	93, 202, 93, 201, 93, 93, 93, 208, 209, 212, 93, 213,
	214, 211, 215, 93, 93, 93, 93, 210, 93, 93, 222, 93,
	227, 228, 220, 232, 93, 223, 221, 93, 229, 93, 239, 235,
	233, 93, 242, 240, 238, 236, 93, 237, 241, 8, 8, 8,
	8, 54, 54, 54, 54, 58, 234, 58, 58, 92, 92, 122,
	93, 122, 122, 130, 231, 130, 130, 163, 163, 230, 93, 226,
	225, 224, 93, 93, 93, 93, 219, 218, 217, 216, 93, 93,
	93, 93, 93, 207, 206, 205, 204, 203, 93, 93, 93, 93,
	93, 93, 93, 189, 188, 187, 186, 185, 184, 183, 93, 93,
	132, 132, 162, 161, 160, 159, 158, 157, 156, 93, 93, 93,
	134, 133, 77, 129, 128, 127, 126, 125, 121, 120, 56, 119,
	94, 93, 89, 86, 72, 64, 57, 56, 244, 9, 9, 7,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244,
};

static const short yy_chk[452] = {   /* 0x00575440  VERIFIED md5 2c681c79528641d7ee723c491ec0fb8f */
	0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 6, 12,
	13, 15, 19, 21, 15, 5, 6, 22, 41, 16, 21, 19,
	41, 16, 23, 13, 16, 15, 24, 23, 24, 12, 35, 16,
	23, 27, 27, 36, 24, 23, 29, 29, 13, 37, 38, 51,
	22, 37, 35, 39, 40, 43, 38, 42, 40, 36, 44, 47,
	45, 43, 37, 38, 42, 39, 46, 39, 45, 40, 47, 48,
	49, 58, 46, 44, 49, 47, 65, 251, 51, 242, 77, 49,
	84, 95, 84, 48, 97, 46, 77, 85, 98, 85, 84, 58,
	85, 95, 96, 101, 65, 99, 102, 103, 104, 105, 96, 97,
	98, 102, 99, 107, 108, 110, 109, 113, 111, 103, 101, 112,
	105, 122, 104, 114, 136, 139, 115, 110, 111, 116, 114, 137,
	108, 107, 109, 137, 113, 112, 115, 136, 138, 116, 140, 122,
	141, 139, 140, 142, 138, 143, 145, 146, 141, 147, 148, 151,
	150, 149, 145, 146, 154, 152, 153, 151, 142, 147, 149, 155,
	143, 152, 153, 165, 168, 167, 170, 148, 150, 173, 171, 174,
	154, 155, 175, 177, 165, 167, 171, 181, 174, 177, 170, 173,
	178, 168, 180, 191, 178, 182, 192, 196, 175, 197, 198, 180,
	200, 182, 201, 181, 199, 208, 214, 191, 192, 198, 209, 199,
	200, 197, 201, 215, 220, 222, 223, 196, 228, 229, 214, 235,
	220, 222, 208, 228, 237, 215, 209, 233, 223, 239, 237, 233,
	229, 241, 240, 238, 236, 234, 232, 235, 239, 245, 245, 245,
	245, 246, 246, 246, 246, 247, 231, 247, 247, 248, 248, 249,
	227, 249, 249, 250, 225, 250, 250, 252, 252, 224, 221, 219,
	217, 216, 213, 212, 211, 210, 207, 206, 204, 203, 202, 195,
	194, 193, 190, 188, 186, 185, 184, 183, 179, 176, 172, 169,
	166, 164, 163, 162, 161, 160, 159, 158, 157, 156, 144, 135,
	132, 131, 129, 128, 127, 126, 125, 121, 120, 106, 100, 92,
	91, 87, 83, 78, 71, 70, 69, 68, 63, 62, 56, 55,
	34, 31, 28, 25, 18, 14, 11, 10, 7, 2, 1, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244, 244,
	244, 244, 244, 244, 244, 244, 244, 244,
};

/* ---- __yy_memcpy  0x0047E420 ---- */
char *__yy_memcpy( char *from, int count, char *to )
{
	int		offset;
	int		n = count;

	if ( n > 0 ) {
		offset = (int)( to - from );
		do {
			from[offset] = *from;
			++from;
			--n;
		} while ( n );
	}
	return from;
}

/* ---- yy_get_previous_state  0x004806D0 ---- */
static int yy_get_previous_state( void )
{
	int				yy_current_state;
	unsigned char	yy_c;
	char			*yy_cp;

	yy_current_state = yy_start;

	for ( yy_cp = YY_TEXT; yy_cp < YY_CBUF_P; ++yy_cp ) {
		yy_c = (unsigned char)( *yy_cp ? yy_ec[(unsigned char)*yy_cp] : 1 );

		if ( yy_accept[yy_current_state] ) {
			yy_last_accepting_state = yy_current_state;
			yy_last_accepting_cpos = (int)yy_cp;
		}
		while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state ) {
			yy_current_state = yy_def[yy_current_state];
			if ( yy_current_state >= YY_META_THRESHOLD ) {
				yy_c = (unsigned char)yy_meta[yy_c];
			}
		}
		yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
	}

	return yy_current_state;
}

/* ---- yy_try_NUL_trans  0x004807A0 ---- */
static int yy_try_NUL_trans( int yy_current_state )
{
	int				yy_is_jam;
	unsigned char	yy_c = 1;

	if ( yy_accept[yy_current_state] ) {
		yy_last_accepting_state = yy_current_state;
		yy_last_accepting_cpos = yy_c_buf_p;
	}
	while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state ) {
		yy_current_state = yy_def[yy_current_state];
		if ( yy_current_state >= YY_META_THRESHOLD ) {
			yy_c = (unsigned char)yy_meta[yy_c];
		}
	}
	yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
	yy_is_jam = ( yy_current_state == YY_JAM_STATE );

	return yy_is_jam ? 0 : yy_current_state;
}

/* ---- yy_get_next_buffer  0x00480500 ---- */
static int yy_get_next_buffer( void )
{
	YY_BUFFER_STATE	b = YY_CURRENT_BUFFER;
	char			*dest = b->yy_ch_buf;
	char			*source = YY_TEXT;
	int				number_to_move, i;
	int				ret_val;
	int				num_to_read;

	if ( YY_CBUF_P > &b->yy_ch_buf[yy_n_chars + 1] ) {
		yy_fatal_error( "fatal flex scanner internal error--end of buffer missed" );
	}

	if ( !b->yy_fill_buffer ) {
		return ( YY_CBUF_P - YY_TEXT - 1 == 0 ) ? EOB_ACT_END_OF_FILE : EOB_ACT_LAST_MATCH;
	}

	number_to_move = (int)( YY_CBUF_P - YY_TEXT ) - 1;
	for ( i = 0; i < number_to_move; ++i ) {
		*( dest++ ) = *( source++ );
	}

	if ( b->yy_buffer_status == 2  ) {
		yy_n_chars = 0;
		b->yy_n_chars = 0;
	} else {
		num_to_read = b->yy_buf_size - number_to_move - 1;

		while ( num_to_read <= 0 ) {
			char	*old_base = b->yy_ch_buf;
			int		offset = (int)( YY_CBUF_P - old_base );

			if ( b->yy_is_our_buffer ) {
				int	new_size = b->yy_buf_size * 2;

				if ( new_size <= 0 ) {
					new_size = b->yy_buf_size + ( b->yy_buf_size / 8 );
				}
				b->yy_buf_size = new_size;
				b->yy_ch_buf = (char *)realloc( old_base, new_size + 2 );
				b = YY_CURRENT_BUFFER;
			} else {
				b->yy_ch_buf = NULL;
			}

			if ( !b->yy_ch_buf ) {
				yy_fatal_error( "fatal error - scanner input buffer overflow" );
			}

			yy_c_buf_p = (int)( b->yy_ch_buf + offset );
			num_to_read = b->yy_buf_size - number_to_move - 1;
		}

		if ( num_to_read > 8192 ) {
			num_to_read = 8192;
		}

		i = 0;
		if ( num_to_read > 0 ) {
			char	*p = (char *)(unsigned int)g_scriptText;
			char	c;

			for ( ;; ) {
				c = *p++;
				if ( !c ) {
					g_scriptText = (int)( p - 1 );
					break;
				}
				if ( c == '\n' ) {
					g_scriptText = (int)p;
					b->yy_ch_buf[number_to_move + i] = '\n';
					++i;
					break;
				}
				b->yy_ch_buf[number_to_move + i] = c;
				++i;
				if ( i >= num_to_read ) {
					g_scriptText = (int)p;
					break;
				}
			}
		}

		yy_n_chars = i;
		b->yy_n_chars = i;
	}

	if ( yy_n_chars == 0 ) {
		if ( number_to_move == 0 ) {
			ret_val = EOB_ACT_END_OF_FILE;
			yyrestart( (FILE *)(unsigned int)yyin );
			b = YY_CURRENT_BUFFER;
		} else {
			ret_val = EOB_ACT_LAST_MATCH;
			b->yy_buffer_status = 2 ;
		}
	} else {
		ret_val = EOB_ACT_CONTINUE_SCAN;
	}

	yy_n_chars += number_to_move;
	b->yy_ch_buf[yy_n_chars] = 0;
	b->yy_ch_buf[yy_n_chars + 1] = 0;
	yytext_ptr = (int)b->yy_ch_buf;

	return ret_val;
}

#define YY_USER_ACTION \
	do { \
		yylval.sourcePos = (unsigned int)g_sourceCharPos; \
		g_sourcePos = g_sourceCharPos; \
		g_sourceCharPos += yyleng; \
	} while ( 0 )

/* ---- yylex  0x0047F460 ---- */
int yylex( void )
{
	register int	yy_current_state;
	register char	*yy_cp;
	register char	*yy_bp;
	register int	yy_act;

	if ( yy_init ) {
		yy_init = 0;

		if ( !yy_start ) {
			yy_start = 1;
		}
		if ( !yyin ) {
			yyin = (int)stdin;
		}
		if ( !yyout ) {
			yyout = (int)stdout;
		}
		if ( !yy_current_buffer ) {
			yy_current_buffer = (int)yy_create_buffer( YY_BUF_SIZE, (FILE *)(unsigned int)yyin );
		}
		yy_load_buffer_state();
	}

	for ( ;; ) {
		yy_cp = YY_CBUF_P;

		*yy_cp = (char)yy_hold_char;

		yy_bp = yy_cp;
		yy_current_state = yy_start;

yy_match:
		do {
			unsigned char	yy_c = (unsigned char)yy_ec[(unsigned char)*yy_cp];

			if ( yy_accept[yy_current_state] ) {
				yy_last_accepting_state = yy_current_state;
				yy_last_accepting_cpos = (int)yy_cp;
			}
			while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state ) {
				yy_current_state = yy_def[yy_current_state];
				if ( yy_current_state >= YY_META_THRESHOLD ) {
					yy_c = (unsigned char)yy_meta[yy_c];
				}
			}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
			++yy_cp;
		} while ( yy_base[yy_current_state] != YY_JAM_BASE );

yy_find_action:
		yy_act = yy_accept[yy_current_state];
		if ( yy_act == 0 ) {
			yy_cp = (char *)(unsigned int)yy_last_accepting_cpos;
			yy_current_state = yy_last_accepting_state;
			yy_act = yy_accept[yy_current_state];
		}

		yytext_ptr = (int)yy_bp;
		yyleng = (int)( yy_cp - yy_bp );
		yy_hold_char = (unsigned char)*yy_cp;
		*yy_cp = '\0';
		yy_c_buf_p = (int)yy_cp;

do_action:
		switch ( yy_act ) {

		case 0:
			*yy_cp = (char)yy_hold_char;
			yy_cp = (char *)(unsigned int)yy_last_accepting_cpos;
			yy_current_state = yy_last_accepting_state;
			goto yy_find_action;

		case 1:  YY_USER_ACTION; break;
		case 2:  YY_USER_ACTION; yy_start = 3; break;
		case 3:  YY_USER_ACTION; break;
		case 4:  YY_USER_ACTION; break;
		case 5:  YY_USER_ACTION; break;
		case 6:  YY_USER_ACTION; yy_start = 5; break;

		case 7:  YY_USER_ACTION; StringValue( yyleng - 2, YY_TEXT + 1 ); return 259;
		case 8:  YY_USER_ACTION; StringValue( yyleng - 3, YY_TEXT + 2 ); return 260;

		case 9:  YY_USER_ACTION; return 261;
		case 10: YY_USER_ACTION; return 262;
		case 11: YY_USER_ACTION; return 263;
		case 12: YY_USER_ACTION; return 264;
		case 13: YY_USER_ACTION; return 265;
		case 14: YY_USER_ACTION; return 266;
		case 15: YY_USER_ACTION; return 267;
		case 16: YY_USER_ACTION; return 268;
		case 17: YY_USER_ACTION; return 269;
		case 18: YY_USER_ACTION; return 270;
		case 19: YY_USER_ACTION; return 271;
		case 20: YY_USER_ACTION; return 272;
		case 21: YY_USER_ACTION; return 273;
		case 22: YY_USER_ACTION; return 274;
		case 23: YY_USER_ACTION; return 275;
		case 24: YY_USER_ACTION; return 276;
		case 25: YY_USER_ACTION; return 277;
		case 26: YY_USER_ACTION; return 278;
		case 27: YY_USER_ACTION; return 279;
		case 28: YY_USER_ACTION; return 280;
		case 29: YY_USER_ACTION; return 281;
		case 30: YY_USER_ACTION; return 282;
		case 31: YY_USER_ACTION; return 283;
		case 32: YY_USER_ACTION; return 284;
		case 33: YY_USER_ACTION; return 285;
		case 34: YY_USER_ACTION; return 286;

		case 35: YY_USER_ACTION; IntegerValue( YY_TEXT ); return 287;
		case 36: YY_USER_ACTION; FloatValue( YY_TEXT ); return 288;

		case 37: YY_USER_ACTION; return 290;
		case 38: YY_USER_ACTION; return 289;
		case 39: YY_USER_ACTION; return 295;
		case 40: YY_USER_ACTION; return 291;
		case 41: YY_USER_ACTION; return 294;
		case 42: YY_USER_ACTION; return 292;
		case 43: YY_USER_ACTION; return 293;
		case 44: YY_USER_ACTION; return 296;
		case 45: YY_USER_ACTION; return 297;
		case 46: YY_USER_ACTION; return 298;
		case 47: YY_USER_ACTION; return 299;
		case 48: YY_USER_ACTION; return 300;
		case 49: YY_USER_ACTION; return 301;
		case 50: YY_USER_ACTION; return 302;
		case 51: YY_USER_ACTION; return 303;
		case 52: YY_USER_ACTION; return 304;
		case 53: YY_USER_ACTION; return 305;
		case 54: YY_USER_ACTION; return 306;
		case 55: YY_USER_ACTION; return 307;
		case 56: YY_USER_ACTION; return 308;
		case 57: YY_USER_ACTION; return 309;
		case 58: YY_USER_ACTION; return 310;
		case 59: YY_USER_ACTION; return 311;
		case 60: YY_USER_ACTION; return 312;
		case 61: YY_USER_ACTION; return 313;
		case 62: YY_USER_ACTION; return 314;
		case 63: YY_USER_ACTION; return 315;
		case 64: YY_USER_ACTION; return 316;
		case 65: YY_USER_ACTION; return 317;
		case 66: YY_USER_ACTION; return 318;
		case 67: YY_USER_ACTION; return 319;
		case 68: YY_USER_ACTION; return 320;
		case 69: YY_USER_ACTION; return 321;
		case 70: YY_USER_ACTION; return 322;
		case 71: YY_USER_ACTION; return 323;
		case 72: YY_USER_ACTION; return 324;
		case 73: YY_USER_ACTION; return 325;
		case 74: YY_USER_ACTION; return 326;
		case 75: YY_USER_ACTION; return 327;
		case 76: YY_USER_ACTION; return 329;
		case 77: YY_USER_ACTION; return 330;
		case 78: YY_USER_ACTION; return 331;
		case 79: YY_USER_ACTION; return 332;
		case 80: YY_USER_ACTION; return 333;
		case 81: YY_USER_ACTION; return 334;
		case 82: YY_USER_ACTION; return 335;
		case 83: YY_USER_ACTION; return 336;
		case 84: YY_USER_ACTION; return 337;
		case 85: YY_USER_ACTION; return 338;
		case 86: YY_USER_ACTION; return 339;
		case 87: YY_USER_ACTION; return 340;
		case 88: YY_USER_ACTION; return 341;
		case 89: YY_USER_ACTION; return 342;

		case 90: YY_USER_ACTION; TextValue( yyleng, YY_TEXT ); return 258;
		case 91: YY_USER_ACTION; TextValue( yyleng, YY_TEXT ); return 328;

		case 92:
			YY_USER_ACTION;
			CompileError( g_sourcePos, "bad token '%s'", YY_TEXT );

		case 93:
			YY_USER_ACTION;
			fwrite( YY_TEXT, yyleng, 1, (FILE *)(unsigned int)yyout );
			break;

		case YY_END_OF_BUFFER:
		{
			int	yy_amount_of_matched_text = (int)( yy_cp - YY_TEXT ) - 1;

			*yy_cp = (char)yy_hold_char;

			if ( YY_CURRENT_BUFFER->yy_buffer_status == 0  ) {
				yy_n_chars = YY_CURRENT_BUFFER->yy_n_chars;
				YY_CURRENT_BUFFER->yy_input_file = (FILE *)(unsigned int)yyin;
				YY_CURRENT_BUFFER->yy_buffer_status = 1 ;
			}

			if ( YY_CBUF_P <= &YY_CURRENT_BUFFER->yy_ch_buf[yy_n_chars] ) {
				int	yy_next_state;

				yy_c_buf_p = (int)( YY_TEXT + yy_amount_of_matched_text );
				yy_current_state = yy_get_previous_state();
				yy_next_state = yy_try_NUL_trans( yy_current_state );
				yy_bp = YY_TEXT;

				if ( yy_next_state ) {
					yy_c_buf_p += 1;
					yy_cp = YY_CBUF_P;
					yy_current_state = yy_next_state;
					goto yy_match;
				} else {
					yy_cp = YY_CBUF_P;
					goto yy_find_action;
				}
			}

			switch ( yy_get_next_buffer() ) {
			case EOB_ACT_CONTINUE_SCAN:
				yy_c_buf_p = (int)( YY_TEXT + yy_amount_of_matched_text );
				yy_current_state = yy_get_previous_state();
				yy_cp = YY_CBUF_P;
				yy_bp = YY_TEXT;
				goto yy_match;

			case EOB_ACT_END_OF_FILE:
				yy_did_buffer_switch_on_eof = 0;
				if ( yywrap() ) {
					yy_c_buf_p = yytext_ptr;
					yy_act = YY_END_OF_BUFFER + ( yy_start - 1 ) / 2 + 1;
					goto do_action;
				}
				break;

			case EOB_ACT_LAST_MATCH:
				yy_c_buf_p = (int)( &YY_CURRENT_BUFFER->yy_ch_buf[yy_n_chars] );
				yy_current_state = yy_get_previous_state();
				yy_cp = YY_CBUF_P;
				yy_bp = YY_TEXT;
				goto yy_find_action;
			}
			break;
		}

		case 95:
		case 96:
		case 97:
			return 0;

		default:
			yy_fatal_error( "fatal flex scanner internal error--no action found" );
		}
	}
}

/* ---- TextValue  0x0047F370 ---- VERIFIED */
short TextValue( int length, const char *text )
{
	short	id = (short)SL_GetLowercaseStringOfLen( text, 0, length + 1, 13 );

	*(unsigned short *)&yylval.val = (unsigned short)id;
	return id;
}

/* ---- StringValue  0x0047F390 ---- VERIFIED */
short StringValue( int length, const char *text )
{
	char			*buf = (char *)_alloca( length + 1 );
	char			*out = buf;
	int				remaining = length;
	char			c;
	short			id;

	while ( remaining ) {
		if ( *text == '\\' ) {
			if ( remaining - 1 == 0 ) {
				break;
			}
			++text;
			c = *text;
			switch ( c ) {
			case 'n':	*out = '\n'; break;
			case 'r':	*out = '\r'; break;
			case 't':	*out = '\t'; break;
			default:	*out = c;    break;
			}
			++out;
			remaining -= 2;
		} else {
			--remaining;
			*out++ = *text;
		}
		++text;
	}
	*out = '\0';

	id = (short)SL_GetStringOfLen( buf, 0, strlen( buf ) + 1, 13 );
	*(unsigned short *)&yylval.val = (unsigned short)id;
	return id;
}

/* ---- IntegerValue  0x0047F420 ---- */
int IntegerValue( const char *text )
{
	return sscanf( text, "%d", &yylval.val );
}

/* ---- FloatValue  0x0047F440 ---- */
int FloatValue( const char *text )
{
	return sscanf( text, "%f", &yylval.val );
}

/* ---- yyrestart  0x00480840 ---- */
void yyrestart( FILE *input_file )
{
	if ( !yy_current_buffer ) {
		yy_current_buffer = (int)yy_create_buffer( YY_BUF_SIZE, (FILE *)(unsigned int)yyin );
	}
	yy_init_buffer( YY_CURRENT_BUFFER, input_file );
	yy_load_buffer_state();
}

/* ---- yy_switch_to_buffer  0x004808D0 ---- */
void yy_switch_to_buffer( YY_BUFFER_STATE new_buffer )
{
	if ( YY_CURRENT_BUFFER == new_buffer ) {
		return;
	}

	if ( yy_current_buffer ) {
		*YY_CBUF_P = (char)yy_hold_char;
		YY_CURRENT_BUFFER->yy_buf_pos = YY_CBUF_P;
		YY_CURRENT_BUFFER->yy_n_chars = yy_n_chars;
	}

	yy_current_buffer = (int)new_buffer;
	yy_load_buffer_state();

	yy_did_buffer_switch_on_eof = 1;
}

/* ---- yy_load_buffer_state  0x00480940 ---- */
static void yy_load_buffer_state( void )
{
	yy_n_chars = YY_CURRENT_BUFFER->yy_n_chars;
	yytext_ptr = (int)YY_CURRENT_BUFFER->yy_buf_pos;
	yy_c_buf_p = yytext_ptr;
	yyin = (int)YY_CURRENT_BUFFER->yy_input_file;
	yy_hold_char = (unsigned char)*YY_CBUF_P;
}

/* ---- yy_create_buffer  0x00480970 ---- */
static YY_BUFFER_STATE yy_create_buffer( int size, FILE *file )
{
	YY_BUFFER_STATE	b;

	b = (YY_BUFFER_STATE)malloc( sizeof( struct yy_buffer_state ) );
	if ( !b ) {
		yy_fatal_error( "out of dynamic memory in yy_create_buffer()" );
	}

	b->yy_buf_size = size;

	b->yy_ch_buf = (char *)malloc( size + 2 );
	if ( !b->yy_ch_buf ) {
		yy_fatal_error( "out of dynamic memory in yy_create_buffer()" );
	}

	b->yy_is_our_buffer = 1;

	yy_init_buffer( b, file );

	return b;
}

/* ---- yy_delete_buffer  0x00480A20 ---- VERIFIED */
void yy_delete_buffer( YY_BUFFER_STATE b )
{
	if ( !b ) {
		return;
	}

	if ( b == YY_CURRENT_BUFFER ) {
		yy_current_buffer = 0;
	}
	if ( b->yy_is_our_buffer ) {
		free( b->yy_ch_buf );
	}
	free( b );
}

/* ---- yy_init_buffer  0x00480A60 ---- */
void yy_init_buffer( YY_BUFFER_STATE b, FILE *file )
{
	yy_flush_buffer( b );

	b->yy_input_file = file;
	b->yy_fill_buffer = 1;
	b->yy_is_interactive = file ? ( _isatty( _fileno( file ) ) > 0 ) : 0;
}

/* ---- yy_flush_buffer  0x00480AA0 ---- */
void yy_flush_buffer( YY_BUFFER_STATE b )
{
	if ( !b ) {
		return;
	}

	b->yy_n_chars = 0;

	b->yy_ch_buf[0] = 0;
	b->yy_ch_buf[1] = 0;

	b->yy_buf_pos = b->yy_ch_buf;
	b->yy_at_bol = 1;
	b->yy_buffer_status = 0 ;

	if ( b == YY_CURRENT_BUFFER ) {
		yy_load_buffer_state();
	}
}

/* ---- yy_scan_buffer  0x00480B00 ---- */
YY_BUFFER_STATE yy_scan_buffer( char *base, unsigned int size )
{
	YY_BUFFER_STATE	b;

	if ( size < 2 || base[size - 2] != 0 || base[size - 1] != 0 ) {
		return NULL;
	}

	b = (YY_BUFFER_STATE)malloc( sizeof( struct yy_buffer_state ) );
	if ( !b ) {
		yy_fatal_error( "out of dynamic memory in yy_scan_buffer()" );
	}

	b->yy_buf_size = size - 2;
	b->yy_n_chars = b->yy_buf_size;
	b->yy_ch_buf = base;
	b->yy_buf_pos = base;
	b->yy_is_our_buffer = 0;
	b->yy_input_file = NULL;
	b->yy_is_interactive = 0;
	b->yy_at_bol = 1;
	b->yy_fill_buffer = 0;
	b->yy_buffer_status = 0 ;

	yy_switch_to_buffer( b );

	return b;
}

/* ---- yy_scan_string  0x00480B90 ---- VERIFIED */
YY_BUFFER_STATE yy_scan_string( const char *str )
{
	int	len = 0;

	if ( *str ) {
		while ( str[++len] ) {
			;
		}
	}
	return yy_scan_bytes( len, str );
}

/* ---- yy_scan_bytes  0x00480BC0 ---- */
YY_BUFFER_STATE yy_scan_bytes( int len, const char *bytes )
{
	YY_BUFFER_STATE	b;
	char			*buf;
	int				i;

	buf = (char *)malloc( len + 2 );
	if ( !buf ) {
		yy_fatal_error( "out of dynamic memory in yy_scan_bytes()" );
	}

	for ( i = 0; i < len; ++i ) {
		buf[i] = bytes[i];
	}
	buf[len] = 0;
	buf[len + 1] = 0;

	b = yy_scan_buffer( buf, len + 2 );
	if ( !b ) {
		yy_fatal_error( "bad buffer in yy_scan_bytes()" );
	}

	b->yy_is_our_buffer = 1;

	return b;
}

/* ---- yy_fatal_error  0x00480C50 ---- */
static void yy_fatal_error( const char *msg )
{
	fprintf( stderr, "%s\n", msg );
	exit( 2 );
}

/* ---- yy_flex_alloc  0x00480C70 ---- VERIFIED */
void *yy_flex_alloc( size_t size )
{
	return malloc( size );
}

/* ---- yy_flex_realloc  0x00480C80 ---- VERIFIED */
void *yy_flex_realloc( void *ptr, size_t size )
{
	return realloc( ptr, size );
}

/* ---- yy_flex_free  0x00480C90 ---- VERIFIED */
void yy_flex_free( void *ptr )
{
	free( ptr );
}

/* ---- yyerror  0x00480CA0 ---- */
int yyerror( void )
{
	if ( !yychar ) {
		CompileError( g_sourcePos, "unexpected end of file found" );
	}
	if ( yychar != YYERRTOKEN ) {
		CompileError( g_sourcePos, "bad syntax" );
	}
	return 0;
}

static short	yyss[YYMAXDEPTH];
static yystype	yyvs[YYMAXDEPTH];

#define V( k )	( yyvsp[( k )].val )
#define P( k )	( yyvsp[( k )].sourcePos )

/* ---- yyparse  0x0047E440 ---- */
int yyparse( void )
{
	register int	yystate;
	register int	yyn;
	short			*yyssp;
	yystype			*yyvsp;
	int				yychar1;
	int				yylen;
	int				yyerrstatus;
	yystype			yyval;

	yynerrs = 0;
	yychar = YYEMPTY;
	yyerrstatus = 0;
	yychar1 = 0;

	yyssp = yyss - 1;
	yyvsp = yyvs;
	yystate = 0;
	yyval.val = 0;
	yyval.sourcePos = 0;

yynewstate:
	*++yyssp = (short)yystate;

	if ( yyssp >= yyss + YYMAXDEPTH - 1 ) {
		yyerror();
		return 2;
	}

	yyn = yypact[yystate];
	if ( yyn == YYFLAG ) {
		goto yydefault;
	}

	if ( yychar == YYEMPTY ) {
		yychar = yylex();
	}

	if ( yychar <= 0 ) {
		yychar = 0;
		yychar1 = 0;
	} else {
		yychar1 = ( (unsigned int)yychar > YYMAXUTOK ) ? YYUNDEFTOK : yytranslate[yychar];
	}

	yyn += yychar1;
	if ( yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1 ) {
		goto yydefault;
	}

	yyn = yytable[yyn];
	if ( yyn < 0 ) {
		if ( yyn == YYFLAG ) {
			goto yyerrlab;
		}
		yyn = -yyn;
		goto yyreduce;
	}
	if ( yyn == 0 ) {
		goto yyerrlab;
	}
	if ( yyn == YYFINAL ) {
		return 0;
	}

	if ( yychar ) {
		yychar = YYEMPTY;
	}
	*++yyvsp = yylval;
	if ( yyerrstatus ) {
		--yyerrstatus;
	}
	yystate = yyn;
	goto yynewstate;

yydefault:
	yyn = yydefact[yystate];
	if ( yyn == 0 ) {
		goto yyerrlab;
	}

yyreduce:
	yylen = yyr2[yyn];
	if ( yylen > 0 ) {
		yyval = yyvsp[1 - yylen];
	}

	switch ( yyn ) {

	case 1:
		g_parseTreeRoot = (int)node2_( yyval.val, V( 0 ) );
		break;

	case 2:
		yyval.val = node2( 4, yyval.val, yyval.sourcePos );
		break;

	case 3:
		yyval.val = node5( 43, yyval.val, yyval.sourcePos, V( 0 ), P( 0 ), P( -1 ) );
		yyval.sourcePos = P( -1 );
		break;

	case 4:
		yyval.val = node5( 44, yyval.val, yyval.sourcePos, V( 0 ), P( 0 ), P( -1 ) );
		yyval.sourcePos = P( -1 );
		break;

	case 5:		yyval.val = node4( 45, yyval.val, V( 0 ), 62, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 6:		yyval.val = node4( 45, yyval.val, V( 0 ), 63, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 7:		yyval.val = node4( 45, yyval.val, V( 0 ), 64, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 8:		yyval.val = node4( 45, yyval.val, V( 0 ), 65, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 9:		yyval.val = node4( 45, yyval.val, V( 0 ), 66, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 10:	yyval.val = node4( 45, yyval.val, V( 0 ), 67, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 11:	yyval.val = node4( 45, yyval.val, V( 0 ), 68, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 12:	yyval.val = node4( 45, yyval.val, V( 0 ), 69, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 13:	yyval.val = node4( 45, yyval.val, V( 0 ), 70, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 14:	yyval.val = node4( 45, yyval.val, V( 0 ), 71, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 15:	yyval.val = node4( 45, yyval.val, V( 0 ), 72, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 16:	yyval.val = node4( 45, yyval.val, V( 0 ), 73, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 17:	yyval.val = node4( 45, yyval.val, V( 0 ), 74, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 18:	yyval.val = node4( 45, yyval.val, V( 0 ), 75, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 19:	yyval.val = node4( 45, yyval.val, V( 0 ), 76, P( -1 ) ); yyval.sourcePos = P( -1 ); break;
	case 20:	yyval.val = node4( 45, yyval.val, V( 0 ), 77, P( -1 ) ); yyval.sourcePos = P( -1 ); break;

	case 21:	yyval.val = node2( 46, V( 0 ), P( 0 ) ); break;
	case 22:	yyval.val = node2( 47, V( 0 ), P( 0 ) ); break;
	case 23:	yyval.val = node2( 48, V( 0 ), P( 0 ) ); break;
	case 24:	yyval.val = node2( 49, V( 0 ), P( 0 ) ); break;
	case 25:	yyval.val = node2( 50, V( 0 ), P( 0 ) ); break;
	case 26:	yyval.val = node2( 51, V( 0 ), P( 0 ) ); break;

	case 27:
		yyval.val = node1( 63, yyval.val );
		break;

	case 28:
	case 89:
	case 99:
	case 107:
	case 122:
		yyval.val = node0( 0 );
		break;

	case 31:
		yyval.val = node3( 17, yyval.val, V( 0 ), yyval.sourcePos );
		++g_scrStatementCount;
		break;

	case 32:
		yyval.val = node2( 16, yyval.val, yyval.sourcePos );
		break;

	case 33:
		yyval.val = node3( 17, yyval.val, V( 0 ), yyval.sourcePos );
		yyval.sourcePos = P( -1 );
		++g_scrStatementCount;
		break;

	case 34:
		yyval.val = node2( 16, V( 0 ), yyval.sourcePos );
		break;

	case 35:
	case 56:
		yyval.val = node2( 14, yyval.val, yyval.sourcePos );
		break;

	case 36:
		yyval.val = node2( 18, V( -2 ), P( -2 ) );
		break;

	case 37:
		yyval.val = node2( 22, yyval.val, yyval.sourcePos );
		break;

	case 38:
		yyval.val = node3( 26, V( 0 ), yyval.sourcePos, P( 0 ) );
		yyval.sourcePos = P( 0 );
		break;

	case 39:
		yyval.val = node3( 19, yyval.val, V( -1 ), P( -2 ) );
		yyval.sourcePos = P( -2 );
		break;

	case 40:
		yyval.val = node5( 20, yyval.val, V( -3 ), V( -1 ), yyval.sourcePos, P( -2 ) );
		yyval.sourcePos = P( -2 );
		break;

	case 41:	yyval.val = node2( 42, V( -1 ), yyval.sourcePos ); break;
	case 42:	yyval.val = node2( 5, yyval.val, yyval.sourcePos ); break;
	case 43:	yyval.val = node2( 6, yyval.val, yyval.sourcePos ); break;
	case 44:	yyval.val = node2( 7, V( 0 ), yyval.sourcePos ); break;
	case 45:	yyval.val = node2( 8, V( 0 ), yyval.sourcePos ); break;
	case 46:	yyval.val = node2( 9, yyval.val, yyval.sourcePos ); break;
	case 47:	yyval.val = node2( 10, yyval.val, yyval.sourcePos ); break;
	case 48:	yyval.val = node1( 15, yyval.val ); break;
	case 49:	yyval.val = node2( 13, yyval.val, yyval.sourcePos ); break;
	case 50:	yyval.val = node1( 27, yyval.sourcePos ); break;
	case 51:	yyval.val = node1( 28, yyval.sourcePos ); break;
	case 52:	yyval.val = node1( 29, yyval.sourcePos ); break;
	case 53:	yyval.val = node1( 30, yyval.sourcePos ); break;
	case 54:	yyval.val = node1( 31, yyval.sourcePos ); break;

	case 55:
		yyval.val = node2( 52, yyval.val, yyval.sourcePos );
		yyval.sourcePos = P( 0 );
		break;

	case 57:	yyval.val = node0( 64 ); break;
	case 58:	yyval.val = node2( 65, V( 0 ), P( 0 ) ); break;
	case 59:	yyval.val = node2( 69, yyval.val, yyval.sourcePos ); break;
	case 60:	yyval.val = node2( 70, yyval.val, yyval.sourcePos ); break;
	case 61:	yyval.val = node1( 71, yyval.sourcePos ); break;

	case 62:
		yyval.val = node4( 12, yyval.val, V( 0 ), yyval.sourcePos, P( 0 ) );
		yyval.sourcePos = P( 0 );
		break;

	case 63:
		yyval.val = node4( 11, yyval.val, V( -1 ), yyval.sourcePos, P( -1 ) );
		yyval.sourcePos = P( -2 );
		break;

	case 64:	yyval.val = node2( 3, yyval.val, yyval.sourcePos ); break;
	case 65:	yyval.val = node1( 21, yyval.val ); break;
	case 66:	yyval.val = node3( 2, yyval.val, V( 0 ), P( -1 ) ); break;
	case 67:	yyval.val = node2( 23, V( 0 ), yyval.sourcePos ); break;
	case 68:	yyval.val = node1( 24, yyval.sourcePos ); break;
	case 69:	yyval.val = node3( 25, V( 0 ), P( 0 ), yyval.sourcePos ); break;
	case 70:	yyval.val = node2( 37, yyval.val, yyval.sourcePos ); break;
	case 71:	yyval.val = node2( 38, yyval.val, yyval.sourcePos ); break;

	case 72:	yyval.val = node4( 39, yyval.val, V( 0 ), 62, P( -1 ) ); break;
	case 73:	yyval.val = node4( 39, yyval.val, V( 0 ), 63, P( -1 ) ); break;
	case 74:	yyval.val = node4( 39, yyval.val, V( 0 ), 64, P( -1 ) ); break;
	case 75:	yyval.val = node4( 39, yyval.val, V( 0 ), 71, P( -1 ) ); break;
	case 76:	yyval.val = node4( 39, yyval.val, V( 0 ), 72, P( -1 ) ); break;
	case 77:	yyval.val = node4( 39, yyval.val, V( 0 ), 73, P( -1 ) ); break;
	case 78:	yyval.val = node4( 39, yyval.val, V( 0 ), 74, P( -1 ) ); break;
	case 79:	yyval.val = node4( 39, yyval.val, V( 0 ), 75, P( -1 ) ); break;
	case 80:	yyval.val = node4( 39, yyval.val, V( 0 ), 76, P( -1 ) ); break;
	case 81:	yyval.val = node4( 39, yyval.val, V( 0 ), 77, P( -1 ) ); break;

	case 82:	yyval.val = node4( 54, yyval.val, V( -1 ), yyval.sourcePos, P( -3 ) ); break;
	case 83:	yyval.val = node4( 55, yyval.val, V( -1 ), yyval.sourcePos, P( -3 ) ); break;
	case 84:	yyval.val = node4( 56, yyval.val, V( -1 ), yyval.sourcePos, P( -3 ) ); break;
	case 85:	yyval.val = node4( 57, yyval.val, V( -1 ), yyval.sourcePos, P( -1 ) ); break;
	case 86:	yyval.val = node1( 61, yyval.sourcePos ); break;
	case 87:	yyval.val = node1( 62, yyval.sourcePos ); break;
	case 88:	yyval.val = node4( 35, V( -4 ), V( -1 ), P( -1 ), P( -3 ) ); break;

	case 92:	yyval.val = node2( 40, V( -1 ), yyval.sourcePos ); break;
	case 93:	yyval.val = node3( 32, V( -2 ), V( 0 ), P( -2 ) ); break;
	case 94:	yyval.val = node4( 33, V( -4 ), V( -2 ), V( 0 ), P( -4 ) ); break;
	case 95:	yyval.val = node4( 34, V( -2 ), V( 0 ), P( -2 ), yyval.sourcePos ); break;
	case 96:	yyval.val = node6( 36, V( -5 ), V( -4 ), V( -2 ), V( 0 ), P( -4 ), yyval.sourcePos ); break;
	case 97:	yyval.val = node3( 58, V( -2 ), V( 0 ), P( -2 ) ); break;
	case 98:	yyval.val = node2( 41, V( -1 ), yyval.sourcePos ); break;
	case 100:	yyval.val = node2( 59, V( -1 ), yyval.sourcePos ); break;
	case 101:	yyval.val = node1( 60, yyval.sourcePos ); break;

	case 103:
		yyval.val = append_node( yyval.val, V( 0 ) );
		break;

	case 104:
	case 110:
	case 121:
	case 124:
		yyval.val = linked_list_end( node0( 0 ) );
		break;

	case 105:
	case 115:
		yyval.val = prepend_node( node2_( V( 0 ), P( 0 ) ), yyval.val );
		break;

	case 106:
	case 116:
	{
		sval_t	tail = node0( 0 );

		yyval.val = prepend_node( node2_( yyval.val, yyval.sourcePos ), tail );
		break;
	}

	case 108:
	case 111:
	case 113:
	case 120:
		yyval.val = append_node( yyval.val, node2_( V( 0 ), P( 0 ) ) );
		break;

	case 109:
	case 112:
	case 114:
	{
		sval_t	cell = node2_( yyval.val, yyval.sourcePos );

		yyval.val = append_node( linked_list_end( node0( 0 ) ), cell );
		break;
	}

	case 117:	yyval.val = node4( 66, yyval.val, V( -4 ), V( -1 ), yyval.sourcePos ); break;
	case 118:	yyval.val = node4( 67, V( -7 ), V( -5 ), V( -2 ), P( -7 ) ); break;
	case 119:	yyval.val = node2( 68, V( -2 ), P( -2 ) ); break;

	case 123:
		yyval.val = append_node( yyval.val, V( -1 ) );
		break;

	default:
		break;
	}

	yyvsp -= yylen;
	yyssp -= yylen;

	*++yyvsp = yyval;

	yyn = yyr1[yyn];
	yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
	if ( yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp ) {
		yystate = yytable[yystate];
	} else {
		yystate = yydefgoto[yyn - YYNTBASE];
	}
	goto yynewstate;

yyerrlab:
	if ( yyerrstatus == 0 ) {
		++yynerrs;
		yyerror();
	}

	if ( yyerrstatus == 3 ) {
		if ( yychar == 0 ) {
			return 1;
		}
		yychar = YYEMPTY;
	}

	yyerrstatus = 3;

	for ( ;; ) {
		yyn = yypact[yystate];
		if ( yyn != YYFLAG ) {
			yyn += YYTERROR;
			if ( yyn >= 0 && yyn <= YYLAST && yycheck[yyn] == YYTERROR ) {
				yyn = yytable[yyn];
				if ( yyn < 0 ) {
					if ( yyn != YYFLAG ) {
						yyn = -yyn;
						goto yyreduce;
					}
				} else if ( yyn != 0 ) {
					break;
				}
			}
		}

		if ( yyssp == yyss ) {
			return 1;
		}
		--yyssp;
		--yyvsp;
		yystate = *yyssp;
	}

	if ( yyn == YYFINAL ) {
		return 0;
	}

	*++yyvsp = yylval;
	yystate = yyn;
	goto yynewstate;
}

/* ---- ScriptParse  0x00480CE0 ---- */
int ScriptParse( const char *text, int *parseTree )
{
	struct yy_buffer_state	buffer;
	char					buf[YY_BUF_SIZE + 4];

	g_scriptText = (int)text;
	g_sourceCharPos = 0;
	g_sourcePos = 0;
	yy_init = 1;

	buffer.yy_buf_size = YY_BUF_SIZE;
	buffer.yy_ch_buf = buf;
	buffer.yy_is_our_buffer = 0;

	yy_flush_buffer( &buffer );

	buffer.yy_input_file = NULL;
	buffer.yy_fill_buffer = 1;
	buffer.yy_is_interactive = 0;

	yy_current_buffer = (int)&buffer;

	yy_start = 3;

	yyparse();

	*parseTree = g_parseTreeRoot;
	return g_parseTreeRoot;
}

/* ---- yywrap  0x00480D90 ---- */
int yywrap( void )
{
	return 1;
}
