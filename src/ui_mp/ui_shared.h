/*
 * ui_shared.h -- the menu/widget engine shared by ui_mp_x86.dll and cgame_mp_x86.dll.
 *
 * Call of Duty 1.1.  Addresses are cgame_mp_x86.dll's (imagebase 0x30000000),
 * where ui/ui_shared.c is statically linked -- 281 functions,
 * 0x3003FE30..0x3004AE40.  Quake III / RTCW's ui/ui_shared.h is the structural
 * model; each layout below carries its retail address.
 *
 * HOW THIS UNIT REACHES THE ENGINE.  Exactly as Q3/RTCW do: through the
 * displayContextDef_t function-pointer table behind the file-scope `DC`
 * pointer, which the owning module fills in and hands over via Init_Display.
 * cgame's instance is `cgDC` at 0x301AD720, populated by CG_LoadHudMenu
 * (0x30022BA0), which fills every slot below.  The one
 * exception is the .menu parser, which calls the five trap_PC_* wrappers
 * directly, exactly as RTCW's ui_shared.h declares them; they are declared at
 * the end of this header and live in the owning module's syscall unit.
 *
 * COD1_ASSERT_SIZE appears where the retail size is fixed by an allocation
 * site or array stride in the DLL.
 */

#ifndef __UI_SHARED_H__
#define __UI_SHARED_H__

#include "../universal/q_shared.h"

#define UI_ASSERT_SIZE( type, bytes ) COD1_ASSERT_SIZE( type, bytes )

/*
 * These belong in q_shared.h / the renderer's public header, i.e. in ONE
 * place.  universal/q_shared.h does not carry them yet, and
 * cgame_mp/cg_public.h declares the same four (qhandle_t, refdef_t,
 * refEntity_t, pc_token_t) with identical layouts -- ui_shared.c is
 * statically linked into BOTH modules, so where cg_public.h is already in
 * scope this header defers to it rather than redeclare.
 */
#ifndef __CG_PUBLIC_H__
typedef int qhandle_t;
typedef int sfxHandle_t;
#endif

#define MAX_QPATH               64
#define MAX_STRING_CHARS        1024

/*
 * Keycodes.  The original tree has these in keycodes.h, which ui_shared.h
 * includes.  Values are CoD 1.1's: the engine's keynames[]
 * (client_mp/cl_keys_mp.c 0x0057B8D0) and Menu_HandleKey's dispatch table
 * at 0x30045340.
 */
#define K_TAB               9
#define K_ENTER             13
#define K_ESCAPE            27
#define K_SPACE             32
#define K_BACKSPACE         127
#define K_UPARROW           154
#define K_DOWNARROW         155
#define K_LEFTARROW         156
#define K_RIGHTARROW        157
#define K_ALT               158
#define K_CTRL              159
#define K_SHIFT             160
#define K_INS               161
#define K_DEL               162
#define K_PGDN              163
#define K_PGUP              164
#define K_HOME              165
#define K_END               166
#define K_F1                167
#define K_F11               177
#define K_F12               178
#define K_KP_HOME           182
#define K_KP_UPARROW        183
#define K_KP_PGUP           184
#define K_KP_LEFTARROW      185
#define K_KP_5              186
#define K_KP_RIGHTARROW     187
#define K_KP_END            188
#define K_KP_DOWNARROW      189
#define K_KP_PGDN           190
#define K_KP_ENTER          191
#define K_KP_INS            192
#define K_KP_DEL            193
#define K_MOUSE1            200
#define K_MOUSE2            201
#define K_MOUSE3            202
#define K_MOUSE4            203
#define K_MOUSE5            204
#define K_MWHEELDOWN        205
#define K_MWHEELUP          206

/*
 * menudef.h constants.  Item_ValidateTypeData (0x30048990) and
 * Item_StartCapture (0x300445C0) between them use every itemType value.
 */
#define ITEM_TYPE_TEXT              0
#define ITEM_TYPE_BUTTON            1
#define ITEM_TYPE_RADIOBUTTON       2
#define ITEM_TYPE_CHECKBOX          3
#define ITEM_TYPE_EDITFIELD         4
#define ITEM_TYPE_COMBO             5
#define ITEM_TYPE_LISTBOX           6
#define ITEM_TYPE_MODEL             7
#define ITEM_TYPE_OWNERDRAW         8
#define ITEM_TYPE_NUMERICFIELD      9
#define ITEM_TYPE_SLIDER            10
#define ITEM_TYPE_YESNO             11
#define ITEM_TYPE_MULTI             12
#define ITEM_TYPE_BIND              13
#define ITEM_TYPE_MENUMODEL         14
#define ITEM_TYPE_UPREDITFIELD      15      /* CoD addition: uppercases input */

#define ITEM_ALIGN_LEFT             0
#define ITEM_ALIGN_CENTER           1
#define ITEM_ALIGN_RIGHT            2
/* CoD adds a fourth: centre on the combined item + payload width.  The name
 * is not original; the value is what Item_SetTextExtents compares against
 * (0x300454E0, 0x30045662). */
#define ITEM_ALIGN_CENTER_ADJUSTED  3

/* listBoxDef_t::elementStyle (Item_ListBox_Paint 0x3004781D). */
#define LISTBOX_TEXT                0
#define LISTBOX_IMAGE               1

/* item->textStyle (Item_TextColor 0x30045842). */
#define ITEM_TEXTSTYLE_NORMAL       0
#define ITEM_TEXTSTYLE_BLINK        1

/* The engine ORs this into the key for a printable character
 * (Item_TextField_HandleKey 0x30043EEE). */
#define K_CHAR_FLAG                 0x400

/* Item_TextColor's pulse and blink rates (0x30045809, 0x30045842). */
#define PULSE_DIVISOR               75
#define BLINK_DIVISOR               256

/* window.style, from Window_Paint 0x30040BA0 */
#define WINDOW_STYLE_EMPTY          0
#define WINDOW_STYLE_FILLED         1
#define WINDOW_STYLE_GRADIENT       2
#define WINDOW_STYLE_SHADER         3
#define WINDOW_STYLE_TEAMCOLOR      4
#define WINDOW_STYLE_CINEMATIC      5
#define WINDOW_STYLE_SHADER_NOTINT  6       /* CoD addition */

#define MAX_MENUNAME        32
#define MAX_ITEMTEXT        64
#define MAX_ITEMACTION      64
#define MAX_MENUDEFFILE     4096
#define MAX_MENUFILE        32768
#define MAX_MENUS           64
#define MAX_MENUITEMS       128
#define MAX_COLOR_RANGES    10
#define MAX_OPEN_MENUS      16              /* Menus_AddToStack 0x30041900 */

#define WINDOW_MOUSEOVER        0x00000001
#define WINDOW_HASFOCUS         0x00000002
#define WINDOW_VISIBLE          0x00000004
#define WINDOW_GREY             0x00000008
#define WINDOW_DECORATION       0x00000010
#define WINDOW_FADINGOUT        0x00000020
#define WINDOW_FADINGIN         0x00000040
#define WINDOW_MOUSEOVERTEXT    0x00000080
#define WINDOW_INTRANSITION     0x00000100
#define WINDOW_FORECOLORSET     0x00000200
#define WINDOW_HORIZONTAL       0x00000400
#define WINDOW_LB_LEFTARROW     0x00000800
#define WINDOW_LB_RIGHTARROW    0x00001000
#define WINDOW_LB_THUMB         0x00002000
#define WINDOW_LB_PGUP          0x00004000
#define WINDOW_LB_PGDN          0x00008000
#define WINDOW_ORBITING         0x00010000
#define WINDOW_OOB_CLICK        0x00020000
#define WINDOW_WRAPPED          0x00040000
#define WINDOW_AUTOWRAPPED      0x00080000
#define WINDOW_FORCED           0x00100000
#define WINDOW_POPUP            0x00200000
#define WINDOW_BACKCOLORSET     0x00400000
#define WINDOW_TIMEDVISIBLE     0x00800000
#define WINDOW_IGNORE_HUDALPHA  0x01000000  /* Item_OwnerDraw_Paint 0x30047CE1 */

/* Text_Paint on a WINDOW_POPUP item truncates the cvar text at the first '.'
 * (Item_Text_Paint 0x30045EA4) -- the bit is reused on items. */
#define WINDOW_TEXTCVARSHORT    0x00200000

/* CGAME cursor type bits, returned by Display_CursorType (0x3004AB90) */
#define CURSOR_NONE             0x00000001
#define CURSOR_ARROW            0x00000002
#define CURSOR_SIZER            0x00000004

/* UI_Alloc/String_Alloc pools -- cgame's are both 0x20000 (0x3003FE3B,
   0x3003FF6B). */
/* RTCW's split, kept by CoD: the cgame copy's UI_Alloc compares against
   0x20000 (0x3003FE3B), the menu module's against 0x100000 (0x4000ED8B).
   With cgame's pool the UI DLL fails on its first menu load, "UI_Alloc: Out
   of memory" before the error screen can draw. */
#ifdef CGAMEDLL
#define MEM_POOL_SIZE       ( 128 * 1024 )
#else
#define MEM_POOL_SIZE       ( 1024 * 1024 )
#endif
/* Same split for the string pool: ui String_Alloc 0x4000EEBB compares
   against 0x60000 and String_Report 0x4000EF88 pushes it. */
#ifdef CGAMEDLL
#define STRING_POOL_SIZE    ( 128 * 1024 )
#else
#define STRING_POOL_SIZE    ( 384 * 1024 )
#endif

#define HASH_TABLE_SIZE     2048            /* hashForString 0x3003FED3 */
#define KEYWORDHASH_SIZE    512             /* KeywordHash_Key 0x30048AB9 */

#define MAX_STRING_HANDLES  4096
#define MAX_SCRIPT_ARGS     12
#define MAX_EDITFIELD       256

/* int, not RTCW's 16.0 -- Item_ListBox_ThumbDrawPosition 0x400116D0 compares cursorx against ftol()+8 as ints. */
#define SCROLLBAR_SIZE      16
/* float, not RTCW's doubles -- Scroll_Slider_ThumbFunc 0x40012D7C adds dword 96
 * and 0x40012D9F multiplies by dword 1/96; Item_Slider_Paint 0x40015216 subtracts dword 5. */
#define SLIDER_WIDTH        96.0f
#define SLIDER_HEIGHT       16.0f
/* 10, not Q3's 12 -- Item_Slider_Paint 0x30046BF7 draws the thumb 10 wide. */
#define SLIDER_THUMB_WIDTH  10.0f
#define SLIDER_THUMB_HEIGHT 20.0f

#define CVAR_ENABLE     0x00000001
#define CVAR_DISABLE    0x00000002
#define CVAR_SHOW       0x00000004
#define CVAR_HIDE       0x00000008

/* ParseColorRange 0x30049AF0 / 0x30049B10 */
#define RANGETYPE_ABSOLUTE  0
#define RANGETYPE_RELATIVE  1

/*
 * The font record.  Identical to the renderer's (renderer/tr_font.h), which is
 * where the original tree declares it; this unit cannot reach that header, and
 * displayContextDef_t embeds six of them, so it is repeated here.  0x5048
 * bytes is the stride between DC->Assets.textFont (DC+0x10C) and
 * DC->Assets.smallFont (DC+0x5154).
 */
typedef struct {
	int     height;
	int     top;
	float   vertBearing;
	float   horzBearing;
	float   xSkip;
	int     imageWidth;
	int     imageHeight;
	float   s;
	float   t;
	float   s2;
	float   t2;
	qhandle_t glyph;
	char    shaderName[32];
} glyphInfo_t;
UI_ASSERT_SIZE( glyphInfo_t, 0x50 );

typedef struct {
	glyphInfo_t glyphs[256];
	float       glyphScale;
	float       fontHeight;
	char        name[64];
} fontInfo_t;
UI_ASSERT_SIZE( fontInfo_t, 0x5048 );

#ifndef __CG_PUBLIC_H__
/*
 * The two refresh records Item_Model_Paint (0x30047020) builds on the stack.
 * The original tree gets these from the renderer's public header; this unit
 * cannot reach it, so the fields Item_Model_Paint touches are declared here
 * with their offsets, and the tails are held open so the stack clears match
 * the retail memsets (0x50 and 0x9C).
 */
typedef struct {
	int     x;                  /* +0x00 */
	int     y;                  /* +0x04 */
	int     width;              /* +0x08 */
	int     height;             /* +0x0C */
	float   fov_x;              /* +0x10 */
	float   fov_y;              /* +0x14 */
	vec3_t  vieworg;            /* +0x18 */
	vec3_t  viewaxis[3];        /* +0x24 */
	int     time;               /* +0x48 */
	int     rdflags;            /* +0x4C */
} refdef_t;
UI_ASSERT_SIZE( refdef_t, 0x50 );

typedef struct {
	int     reType;             /* +0x00 */
	int     renderfx;           /* +0x04 */
	qhandle_t hModel;           /* +0x08 */
	vec3_t  lightingOrigin;     /* +0x0C */
	float   shadowPlane;        /* +0x18 */
	vec3_t  axis[3];            /* +0x1C */
	int     nonNormalizedAxes;  /* +0x40 */
	vec3_t  origin;             /* +0x44 */
	int     frame;              /* +0x50 */
	vec3_t  oldorigin;          /* +0x54 */
	int     oldframe;           /* +0x60 */
	float   backlerp;           /* +0x64 */
	byte    unknown_0x68[52];   /* rest of the record; only its size is known */
} refEntity_t;
UI_ASSERT_SIZE( refEntity_t, 0x9C );

/*
 * The script-parser token.  q_parse.h's in the original tree; this unit is the
 * only consumer here.  PC_Float_Parse (0x300402E0) reads floatvalue at +0x0C
 * and string at +0x10, PC_Int_Parse (0x300404D0) intvalue at +0x08, and both
 * compare type at +0x00 against TT_NUMBER.
 */
typedef struct pc_token_s {
	int     type;
	int     subtype;
	int     intvalue;
	float   floatvalue;
	char    string[1024];
} pc_token_t;
UI_ASSERT_SIZE( pc_token_t, 1040 );

#endif  /* __CG_PUBLIC_H__ */

#define RDF_NOWORLDMODEL    1
#define MAX_TOKEN_CHARS     1024
#define TT_NUMBER           3

typedef struct {
	const char *command;
	const char *args[MAX_SCRIPT_ARGS];
} scriptDef_t;

typedef struct {
	float x;
	float y;
	float w;
	float h;
} rectDef_t;
UI_ASSERT_SIZE( rectDef_t, 16 );

typedef rectDef_t Rectangle;

/* Window_Init 0x30040B00 clears 0xB8 and seeds borderSize, foreColor, cinematic. */
typedef struct {
	Rectangle   rect;               /* +0x00 */
	Rectangle   rectClient;         /* +0x10  what "rect" parses into */
	const char  *name;              /* +0x20 */
	const char  *model;             /* +0x24  never read by this unit */
	const char  *group;             /* +0x28 */
	const char  *cinematicName;     /* +0x2C */
	int         cinematic;          /* +0x30 */
	int         style;              /* +0x34 */
	int         border;             /* +0x38 */
	int         ownerDraw;          /* +0x3C */
	int         ownerDrawFlags;     /* +0x40 */
	float       borderSize;         /* +0x44 */
	int         flags;              /* +0x48 */
	Rectangle   rectEffects;        /* +0x4C */
	Rectangle   rectEffects2;       /* +0x5C */
	int         offsetTime;         /* +0x6C */
	int         nextTime;           /* +0x70 */
	vec4_t      foreColor;          /* +0x74 */
	vec4_t      backColor;          /* +0x84 */
	vec4_t      borderColor;        /* +0x94 */
	vec4_t      outlineColor;       /* +0xA4 */
	qhandle_t   background;         /* +0xB4 */
} windowDef_t;
UI_ASSERT_SIZE( windowDef_t, 184 );

typedef windowDef_t Window;

/* ParseColorRange 0x30049A40 copies 0x1C bytes; low/high at +0x14/+0x18. */
typedef struct {
	vec4_t  color;
	int     type;
	float   low;
	float   high;
} colorRangeDef_t;
UI_ASSERT_SIZE( colorRangeDef_t, 28 );

#define MAX_LB_COLUMNS 16

typedef struct columnInfo_s {
	int pos;
	int width;
	int maxChars;
} columnInfo_t;

/* Item_ValidateTypeData allocates 232 for ITEM_TYPE_LISTBOX (0x300489B7). */
typedef struct listBoxDef_s {
	int         startPos;           /* +0x00 */
	int         endPos;             /* +0x04 */
	int         drawPadding;        /* +0x08 */
	int         cursorPos;          /* +0x0C */
	float       elementWidth;       /* +0x10 */
	float       elementHeight;      /* +0x14 */
	int         elementStyle;       /* +0x18 */
	int         numColumns;         /* +0x1C */
	columnInfo_t columnInfo[MAX_LB_COLUMNS];    /* +0x20 */
	const char  *doubleClick;       /* +0xE0 */
	qboolean    notselectable;      /* +0xE4 */
} listBoxDef_t;
UI_ASSERT_SIZE( listBoxDef_t, 232 );

/* 32 bytes (0x30048A2C).  maxCharsGotoNext at +0x14 is a CoD addition. */
typedef struct editFieldDef_s {
	float   minVal;                 /* +0x00 */
	float   maxVal;                 /* +0x04 */
	float   defVal;                 /* +0x08 */
	float   range;                  /* +0x0C */
	int     maxChars;               /* +0x10 */
	int     maxCharsGotoNext;       /* +0x14 */
	int     maxPaintChars;          /* +0x18 */
	int     paintOffset;            /* +0x1C */
} editFieldDef_t;
UI_ASSERT_SIZE( editFieldDef_t, 32 );

#define MAX_MULTI_CVARS 32

/* 392 bytes (0x300489F7). */
typedef struct multiDef_s {
	const char  *cvarList[MAX_MULTI_CVARS];     /* +0x000 */
	const char  *cvarStr[MAX_MULTI_CVARS];      /* +0x080 */
	float       cvarValue[MAX_MULTI_CVARS];     /* +0x100 */
	int         count;                          /* +0x180 */
	qboolean    strDef;                         /* +0x184 */
} multiDef_t;
UI_ASSERT_SIZE( multiDef_t, 392 );

/* 64 bytes (0x30048A14). */
typedef struct modelDef_s {
	int     angle;                  /* +0x00 */
	vec3_t  origin;                 /* +0x04 */
	float   fov_x;                  /* +0x10 */
	float   fov_y;                  /* +0x14 */
	int     rotationSpeed;          /* +0x18 */

	int     animated;               /* +0x1C */
	int     startframe;             /* +0x20 */
	int     numframes;              /* +0x24 */
	int     loopframes;             /* +0x28 */
	int     fps;                    /* +0x2C */

	int     frame;                  /* +0x30 */
	int     oldframe;               /* +0x34 */
	float   backlerp;               /* +0x38 */
	int     frameTime;              /* +0x3C */
} modelDef_t;
UI_ASSERT_SIZE( modelDef_t, 64 );

struct menuDef_s;

/*
 * itemDef_t.  0x258 bytes (Item_Init 0x30048640, MenuParse_itemDef 0x3004A357).
 * Two departures from RTCW: `font` moved up next to `alignment`, and the
 * trailing `imageTrack` -- the second argument every registerShaderNoMip /
 * registerModel / registerFont call in this unit passes (0x30048CE0), which
 * CoD's renderer calls the image track.  Menu_New seeds it and MenuParse_itemDef
 * copies it down from the owning menu.
 */
typedef struct itemDef_s {
	Window      window;             /* +0x000 */
	Rectangle   textRect;           /* +0x0B8 */
	int         type;               /* +0x0C8 */
	int         alignment;          /* +0x0CC */
	int         font;               /* +0x0D0 */
	int         textalignment;      /* +0x0D4 */
	float       textalignx;         /* +0x0D8 */
	float       textaligny;         /* +0x0DC */
	float       textscale;          /* +0x0E0 */
	int         textStyle;          /* +0x0E4 */
	const char  *text;              /* +0x0E8 */
	struct menuDef_s *parent;       /* +0x0EC */
	qhandle_t   asset;              /* +0x0F0 */
	const char  *mouseEnterText;    /* +0x0F4 */
	const char  *mouseExitText;     /* +0x0F8 */
	const char  *mouseEnter;        /* +0x0FC */
	const char  *mouseExit;         /* +0x100 */
	const char  *action;            /* +0x104 */
	const char  *onAccept;          /* +0x108 */
	const char  *onFocus;           /* +0x10C */
	const char  *leaveFocus;        /* +0x110 */
	const char  *cvar;              /* +0x114 */
	const char  *cvarTest;          /* +0x118 */
	const char  *enableCvar;        /* +0x11C */
	int         cvarFlags;          /* +0x120 */
	sfxHandle_t focusSound;         /* +0x124 an alias index, like cachedAssets_t's four */
	int         numColors;          /* +0x128 */
	colorRangeDef_t colorRanges[MAX_COLOR_RANGES];  /* +0x12C */
	int         colorRangeType;     /* +0x244 */
	float       special;            /* +0x248 */
	int         cursorPos;          /* +0x24C */
	void        *typeData;          /* +0x250 */
	int         imageTrack;         /* +0x254 */
} itemDef_t;
UI_ASSERT_SIZE( itemDef_t, 600 );

/*
 * menuDef_t.  0x70C bytes (Menu_Init 0x30048400, Menu_New's 1804-byte stride).
 * fadeInAmount and imageTrack are the CoD additions.
 */
typedef struct menuDef_s {
	Window      window;             /* +0x000 */
	const char  *font;              /* +0x0B8 */
	qboolean    fullScreen;         /* +0x0BC */
	int         itemCount;          /* +0x0C0 */
	int         fontIndex;          /* +0x0C4 */
	int         cursorItem;         /* +0x0C8 */
	int         fadeCycle;          /* +0x0CC */
	float       fadeClamp;          /* +0x0D0 */
	float       fadeAmount;         /* +0x0D4 */
	float       fadeInAmount;       /* +0x0D8 */
	const char  *onOpen;            /* +0x0DC */
	const char  *onClose;           /* +0x0E0 */
	const char  *onESC;             /* +0x0E4 */
	const char  *onKey[255];        /* +0x0E8 */
	const char  *soundName;         /* +0x4E4 */
	int         imageTrack;         /* +0x4E8 */
	vec4_t      focusColor;         /* +0x4EC */
	vec4_t      disableColor;       /* +0x4FC */
	itemDef_t   *items[MAX_MENUITEMS];  /* +0x50C */
} menuDef_t;
UI_ASSERT_SIZE( menuDef_t, 1804 );

/*
 * The asset cache the display context carries inline.  Offsets are relative to
 * DC: textFont at DC+0x10C (MenuParse_font 0x30049E34),
 * six 0x5048-byte fonts, gradientBar at DC+0x1E2C0 (GradientBar_Paint), the
 * scroll/slider handles from CG_LoadHudMenu's registrations (0x30022E10),
 * itemFocusSound at DC+0x1E2FC (Script_SetFocus), the four fade lanes at
 * DC+0x1E300.. (Menu_Init) and fontRegistered at DC+0x1E32C.
 */
typedef struct {
	const char  *fontStr;
	const char  *cursorStr;
	const char  *gradientStr;
	fontInfo_t  textFont;
	fontInfo_t  smallFont;
	fontInfo_t  bigFont;
	fontInfo_t  extraBigFont;
	fontInfo_t  boldFont;
	fontInfo_t  consoleFont;
	qhandle_t   cursor;
	qhandle_t   gradientBar;
	qhandle_t   scrollBarArrowUp;
	qhandle_t   scrollBarArrowDown;
	qhandle_t   scrollBarArrowLeft;
	qhandle_t   scrollBarArrowRight;
	qhandle_t   scrollBar;
	qhandle_t   scrollBarThumb;
	qhandle_t   buttonMiddle;
	qhandle_t   buttonInside;
	qhandle_t   solidBox;
	qhandle_t   sliderBar;
	qhandle_t   sliderThumb;
	/* sfxHandle_t, not RTCW's const char *: CG_Asset_Parse stores what trap 191
	   (Com_FindSoundAlias) returned -- an alias INDEX -- into all four
	   (0x30021FC4/0x30021FCC and the three beside it).  The reads below hand
	   that index to playClientSoundAliasByName, which resolves a NAME; that
	   mismatch is retail's, reproduced rather than corrected. */
	sfxHandle_t menuEnterSound;
	sfxHandle_t menuExitSound;
	sfxHandle_t menuBuzzSound;
	sfxHandle_t itemFocusSound;
	float       fadeClamp;
	int         fadeCycle;
	float       fadeAmount;
	float       fadeInAmount;
	float       shadowX;
	float       shadowY;
	vec4_t      shadowColor;
	float       shadowFadeClamp;
	qboolean    fontRegistered;
} cachedAssets_t;

typedef struct {
	const char *name;
	void ( *handler )( itemDef_t *item, char **args );
} commandDef_t;

/*
 * The engine interface.  0x1E3E0 bytes; every slot below 0xE0 is a callback
 * CG_LoadHudMenu (0x30022BA0) assigns by name, except the ones marked
 * "ui only" -- cgame leaves those null and only ui_mp_x86.dll fills them, but
 * this unit calls them, so they must keep their places.
 */
typedef struct {
	qhandle_t   ( *registerShaderNoMip )( const char *p, int imageTrack );                   /* +0x000 */
	void        ( *setColor )( const vec4_t v );                                            /* +0x004 */
	void        ( *drawHandlePic )( float x, float y, float w, float h, qhandle_t asset );   /* +0x008 */
	void        ( *drawStretchPic )( float x, float y, float w, float h,
									 float s1, float t1, float s2, float t2,
									 qhandle_t hShader );                                   /* +0x00C */
	void        ( *drawText )( float x, float y, int font, float scale, const vec4_t color,
							   const char *text, float adjust, int limit, int style );      /* +0x010 */
	int         ( *textWidth )( const char *text, int font, float scale, int limit );        /* +0x014 */
	int         ( *textHeight )( int font, float scale );                                   /* +0x018 */
	const char  *( *translateReference )( const char *reference );                          /* +0x01C */
	const char  *( *safeTranslateString )( const char *reference );                         /* +0x020 */
	const char  *( *translatedMessage )( const char *msg, const char *context );            /* +0x024 */
	void        ( *setFont )( int font );                                                   /* +0x028  ui only */
	qhandle_t   ( *registerModel )( const char *p, int imageTrack );                        /* +0x02C */
	void        ( *modelBounds )( qhandle_t model, vec3_t mins, vec3_t maxs );               /* +0x030 */
	void        ( *fillRect )( float x, float y, float w, float h, const vec4_t color );     /* +0x034 */
	void        ( *drawRect )( float x, float y, float w, float h, float size,
							   const vec4_t color );                                        /* +0x038 */
	void        ( *drawSides )( float x, float y, float w, float h, float size );            /* +0x03C */
	void        ( *drawTopBottom )( float x, float y, float w, float h, float size );        /* +0x040 */
	void        ( *clearScene )( void );                                                    /* +0x044 */
	void        ( *addRefEntityToScene )( const refEntity_t *re );                           /* +0x048 */
	void        ( *renderScene )( const refdef_t *fd );                                      /* +0x04C */
	void        ( *registerFont )( const char *pFontname, int pointSize, fontInfo_t *font,
								   int imageTrack );                                        /* +0x050 */
	void        ( *ownerDrawItem )( float x, float y, float w, float h,
									float text_x, float text_y,
									int ownerDraw, int ownerDrawFlags, int align,
									float special, int font, float scale,
									vec4_t color, qhandle_t shader, int textStyle );        /* +0x054 */
	float       ( *getValue )( int ownerDraw, int type );                                   /* +0x058 */
	qboolean    ( *ownerDrawVisible )( int flags );                                         /* +0x05C */
	void        ( *runScript )( char **p );                                                 /* +0x060 */
	void        ( *getTeamColor )( vec4_t *color );                                         /* +0x064 */
	void        ( *getCVarString )( const char *cvar, char *buffer, int bufsize );           /* +0x068 */
	float       ( *getCVarValue )( const char *cvar );                                      /* +0x06C */
	void        ( *setCVar )( const char *cvar, const char *value );                        /* +0x070 */
	const char  *( *configString )( int index );                                            /* +0x074 */
	void        ( *drawTextWithCursor )( float x, float y, int font, float scale,
										 const vec4_t color, const char *text,
										 int cursorPos, char cursor, int limit,
										 int style );                                       /* +0x078 */
	void        ( *setOverstrikeMode )( qboolean b );                                       /* +0x07C  ui only */
	qboolean    ( *getOverstrikeMode )( void );                                             /* +0x080  ui only */
	void        ( *playClientSoundAliasByName )( const char *name );                         /* +0x084 */
	qboolean    ( *ownerDrawHandleKey )( int ownerDraw, int flags, float *special, int key ); /* +0x088 */
	int         ( *feederCount )( float feederID );                                         /* +0x08C */
	const char  *( *feederItemText )( float feederID, int index, int column,
									  qhandle_t *handle );                                  /* +0x090 */
	const char  *( *fileText )( const char *fileName );                                     /* +0x094  ui only */
	qhandle_t   ( *feederItemImage )( float feederID, int index );                          /* +0x098 */
	void        ( *feederSelection )( float feederID, int index );                          /* +0x09C */
	void        ( *feederAddItem )( float feederID, const char *name, int index );           /* +0x0A0  ui only */
	void        ( *getAutoUpdate )( void );                                                 /* +0x0A4  ui only */
	qboolean    ( *runningGame )( void );                                                   /* +0x0A8  ui only */
	void        ( *keynumToStringBuf )( int keynum, char *buf, int buflen );                 /* +0x0AC */
	void        ( *getBindingBuf )( int keynum, char *buf, int buflen );                     /* +0x0B0 */
	void        ( *setBinding )( int keynum, const char *binding );                          /* +0x0B4 */
	void        ( *executeText )( int exec_when, const char *text );                         /* +0x0B8  ui only */
	void        ( *Error )( int level, const char *error, ... );                             /* +0x0BC */
	void        ( *Print )( const char *msg, ... );                                         /* +0x0C0 */
	void        ( *Pause )( qboolean b );                                                   /* +0x0C4 */
	int         ( *ownerDrawWidth )( int ownerDraw, int font, float scale );                 /* +0x0C8 */
	/* the sound-alias name->index lookup, NOT a loader: cgame stores
	   trap_Com_SoundAliasString (trap 191, Com_FindSoundAlias) here
	   (CG_LoadHudMenu 0x30022D6A). */
	int         ( *registerSound )( const char *alias );                                    /* +0x0CC */
	int         ( *playCinematic )( const char *name, float x, float y, float w, float h );  /* +0x0D0 */
	void        ( *stopCinematic )( int handle );                                           /* +0x0D4 */
	void        ( *drawCinematic )( int handle, float x, float y, float w, float h );        /* +0x0D8 */
	void        ( *runCinematicFrame )( int handle );                                       /* +0x0DC */

	float       yscale;             /* +0x0E0 */
	float       xscale;             /* +0x0E4 */
	float       bias;               /* +0x0E8 */
	int         realTime;           /* +0x0EC */
	int         frameTime;          /* +0x0F0 */
	int         cursorx;            /* +0x0F4 */
	int         cursory;            /* +0x0F8 */
	qboolean    debug;              /* +0x0FC */

	cachedAssets_t Assets;          /* +0x100 */

	/* The renderer's glconfig_t, which this unit never reads.  Held open at
	 * its retail width so whiteShader/gradientImage/cursor/FPS keep their
	 * offsets -- FPS at DC+0x1E3DC is the one Menu_PaintAll prints. */
	byte        glconfig[160];      /* +0x1E330 */
	qhandle_t   whiteShader;        /* +0x1E3D0 */
	qhandle_t   gradientImage;      /* +0x1E3D4 */
	qhandle_t   cursor;             /* +0x1E3D8 */
	float       FPS;                /* +0x1E3DC */
} displayContextDef_t;
UI_ASSERT_SIZE( displayContextDef_t, 0x1E3E0 );

/*
 * The .menu keyword tables.  12-byte rows, chained through +0x08 into a
 * 512-slot bucket array (Item_SetupKeywordHash 0x30049C50).
 */
typedef struct keywordHash_s {
	char        *keyword;
	qboolean    ( *func )( itemDef_t *item, int handle );
	struct keywordHash_s *next;
} keywordHash_t;
UI_ASSERT_SIZE( keywordHash_t, 12 );

/* Controls_GetConfig 0x30046590 walks 0x18-byte rows; bind1/bind2 at +0x10/+0x14. */
typedef struct {
	char    *command;
	int     id;
	int     defaultbind1;
	int     defaultbind2;
	int     bind1;
	int     bind2;
} bind_t;
UI_ASSERT_SIZE( bind_t, 24 );

extern displayContextDef_t *DC;
extern qboolean g_waitingForKey;
extern qboolean g_editingField;
extern itemDef_t *g_editItem;
extern int menuCount;

/*
 * ui_shared.c
 */
void        *UI_Alloc( int size );
void        UI_InitMemory( void );
qboolean    UI_OutOfMemory( void );
const char  *String_Alloc( const char *p );
void        String_Report( void );
void        String_Init( void );
void        PC_SourceWarning( int handle, char *format, ... );
void        PC_SourceError( int handle, char *format, ... );
void        LerpColor( vec4_t a, vec4_t b, vec4_t c, float t );
qboolean    Float_Parse( char **p, float *f );
qboolean    PC_Float_Parse( int handle, float *f );
qboolean    Color_Parse( char **p, vec4_t *c );
qboolean    PC_Color_Parse( int handle, vec4_t *c );
qboolean    Int_Parse( char **p, int *i );
qboolean    PC_Int_Parse( int handle, int *i );
qboolean    Rect_Parse( char **p, rectDef_t *r );
qboolean    PC_Rect_Parse( int handle, rectDef_t *r );
qboolean    String_Parse( char **p, const char **out );
qboolean    PC_String_Parse( int handle, const char **out );
qboolean    PC_Char_Parse( int handle, char *out );
qboolean    PC_Script_Parse( int handle, const char **out );
void        Init_Display( displayContextDef_t *dc );
void        Window_Init( Window *w );
/* Argument order is the retail one: Item_Paint (0x300482ED) passes the owning
 * menu's fadeAmount, fadeInAmount, fadeClamp, fadeCycle in that order. */
void        Window_Paint( Window *w, float fadeAmount, float fadeInAmount, float fadeClamp, float fadeCycle );
void        Fade( int *flags, float *f, float clamp, int *nextTime, int offsetTime,
				  qboolean bFlags, float fadeAmount, float fadeInAmount );
void        GradientBar_Paint( rectDef_t *rect, vec4_t color );
void        Item_SetScreenCoords( itemDef_t *item, float x, float y );
void        Item_UpdatePosition( itemDef_t *item );
qboolean    IsVisible( int flags );
void        ToWindowCoords( float *x, float *y, windowDef_t *window );
void        Item_TextColor( itemDef_t *item, vec4_t *newColor );
void        Item_Text_Paint( itemDef_t *item );
void        Item_Text_Wrapped_Paint( itemDef_t *item, const char *text, vec4_t color );
void        Item_Text_AutoWrapped_Paint( itemDef_t *item, const char *text, vec4_t color );
void        Item_TextField_Paint( itemDef_t *item );
void        Item_YesNo_Paint( itemDef_t *item );
void        Item_Multi_Paint( itemDef_t *item );
void        Item_Slider_Paint( itemDef_t *item );
void        Item_Bind_Paint( itemDef_t *item );
void        Item_Model_Paint( itemDef_t *item );
void        Item_Image_Paint( itemDef_t *item );
void        Item_ListBox_Paint( itemDef_t *item );
void        Item_OwnerDraw_Paint( itemDef_t *item );
void        adjustfrom640( float *x, float *y, float *w, float *h );
int         Item_ListBox_OverLB( itemDef_t *item, float x, float y );
void        Item_ListBox_MouseEnter( itemDef_t *item, float x, float y );
qboolean    Item_ListBox_HandleKey( itemDef_t *item, int key, qboolean down, qboolean force );
qboolean    Item_YesNo_HandleKey( itemDef_t *item, int key );
int         Item_Multi_CountSettings( itemDef_t *item );
int         Item_Multi_FindCvarByValue( itemDef_t *item );
const char  *Item_Multi_Setting( itemDef_t *item );
qboolean    Item_Multi_HandleKey( itemDef_t *item, int key );
qboolean    Item_TextField_HandleKey( itemDef_t *item, int key );
qboolean    Item_Slider_HandleKey( itemDef_t *item, int key, qboolean down );
qboolean    Item_Bind_HandleKey( itemDef_t *item, int key, qboolean down );
void        Item_StartCapture( itemDef_t *item, int key );
void        Item_MouseEnter( itemDef_t *item, float x, float y );
void        Item_MouseLeave( itemDef_t *item );
void        Item_SetMouseOver( itemDef_t *item, qboolean focus );
itemDef_t   *Menu_HitTest( menuDef_t *menu, float x, float y );
qboolean    Menus_RemoveFromStack( menuDef_t *menu );
void        Menus_AddToStack( menuDef_t *menu );
qboolean    Menus_MenuIsInStack( menuDef_t *menu );
void        Menu_RunCloseScript( menuDef_t *menu );
void        Menus_HandleOOBClick( menuDef_t *menu, int key, qboolean down );
qboolean    ParseColorRange( itemDef_t *item, int handle, int type );
void        Menu_UpdatePosition( menuDef_t *menu );
void        Menu_PostParse( menuDef_t *menu );
itemDef_t   *Menu_ClearFocus( menuDef_t *menu );
qboolean    Rect_ContainsPoint( rectDef_t *rect, float x, float y );
int         Menu_ItemsMatchingGroup( menuDef_t *menu, const char *name );
itemDef_t   *Menu_GetMatchingItemByNumber( menuDef_t *menu, int index, const char *name );
itemDef_t   *Menu_FindItemByName( menuDef_t *menu, const char *p );
void        Menu_ShowItemByName( menuDef_t *menu, const char *p, qboolean bShow );
void        Menu_FadeItemByName( menuDef_t *menu, const char *p, qboolean fadeOut );
menuDef_t   *Menus_FindByName( const char *p );
void        Menus_Close( menuDef_t *menu );
void        Menus_CloseByName( const char *p );
void        Menus_CloseAll( void );
void        Menu_TransitionItemByName( menuDef_t *menu, const char *p, rectDef_t rectFrom, rectDef_t rectTo, int time, float amt );
void        Menu_OrbitItemByName( menuDef_t *menu, const char *p, float x, float y, float cx, float cy, int time );
void        Item_RunScript( itemDef_t *item, const char *s );
qboolean    Item_EnableShowViaCvar( itemDef_t *item, int flag );
qboolean    Item_SetFocus( itemDef_t *item, float x, float y );
qboolean    Item_HandleKey( itemDef_t *item, int key, qboolean down );
void        Item_Action( itemDef_t *item );
itemDef_t   *Menu_SetPrevCursorItem( menuDef_t *menu );
itemDef_t   *Menu_SetNextCursorItem( menuDef_t *menu );
void        Menu_CloseCinematics( menuDef_t *menu );
void        Display_CloseCinematics( void );
void        Menus_Open( menuDef_t *menu );
void        Menus_OpenByName( const char *p );
int         Display_VisibleMenuCount( void );
rectDef_t   *Item_CorrectedTextRect( itemDef_t *item );
void        Menu_HandleKey( menuDef_t *menu, int key, qboolean down );
void        Item_SetTextExtents( itemDef_t *item, int *width, int *height, const char *text );
void        Item_Paint( itemDef_t *item );
void        Menu_Init( menuDef_t *menu, int imageTrack );
itemDef_t   *Menu_GetFocusedItem( menuDef_t *menu );
menuDef_t   *Menu_GetFocused( void );
void        Menu_ScrollFeeder( menuDef_t *menu, int feeder, qboolean down );
void        Menu_SetFeederSelection( menuDef_t *menu, int feeder, int index, const char *name );
qboolean    UI_IsFullscreen( void );
void        Item_Init( itemDef_t *item, int imageTrack );
qboolean    Menu_HandleMouseMove( menuDef_t *menu, float x, float y );
void        Menu_Paint( menuDef_t *menu, qboolean forcePaint );
void        Item_ValidateTypeData( itemDef_t *item );
qboolean    Item_Parse( int handle, itemDef_t *item );
void        Item_InitControls( itemDef_t *item );
qboolean    Menu_Parse( int handle, menuDef_t *menu );
void        Menu_New( int handle, int imageTrack );
int         Menu_Count( void );
void        Menu_PaintAll( void );
void        Menu_Reset( void );
displayContextDef_t *Display_GetContext( void );
void        *Display_CaptureItem( int x, int y );
qboolean    Display_MouseMove( void *p, int x, int y );
int         Display_CursorType( int x, int y );
qboolean    Display_KeyBindPending( void );
void        Display_HandleKey( int key, qboolean down, int x, int y );
void        Display_CacheAll( void );

void        Controls_GetKeyAssignment( char *command, int *twokeys );
void        Controls_GetConfig( void );
void        Controls_SetConfig( qboolean restart );
void        Controls_SetDefaults( void );
int         BindingIDFromName( const char *name );
char        *BindingFromName( const char *name );
qboolean    GetCommandHasBinding( const char *name );
int         GetKeyBindings( const char *name, const char **bind1, const char **bind2 );
int         GetKeyBindingLocalizedString( const char *name, char **out );

/*
 * The engine calls this unit does NOT route through DC.  The .menu parser
 * talks to the script system directly, exactly as RTCW's ui_shared.h declares.
 * The wrappers live in the owning module's syscall unit.
 */
int         trap_PC_AddGlobalDefine( char *define );
int         trap_PC_LoadSource( const char *filename );
int         trap_PC_FreeSource( int handle );
int         trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int         trap_PC_SourceFileAndLine( int handle, char *filename, int *line );

/* forward: q_shared.c / com_shared.c -- q_shared.h carries none of these yet. */
int         Q_stricmp( const char *s1, const char *s2 );
int         Q_stricmpn( const char *s1, const char *s2, int n );
int         Q_strncmp( const char *s1, const char *s2, int n );
void        Q_strncpyz( char *dest, const char *src, int destsize );
void        Q_strcat( char *dest, int size, const char *src );
qboolean    Q_isnumeric( int c );
void        Com_StripExtension( const char *in, char *out );
char        *va( const char *format, ... );
void        Com_Error( int level, const char *fmt, ... );
void        Com_Printf( const char *msg, ... );
char        *Com_ParseOnLine( char **data_p );
extern parseInfo_t *parseInfo;

/* forward: com_math.c */
void        AxisClear( vec3_t axis[3] );
void        AnglesToAxis( const vec3_t angles, vec3_t axis[3] );

#endif  /* __UI_SHARED_H__ */
