/*
 * Reconstructed from Call of Duty 1.1 (Windows, CoDMP.exe).
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/stringed/stringed_ingame.cpp
 *
 * NOT YET RECONSTRUCTED.
 *
 * Retail range 0x004AA360-0x004AA7EB, 14 functions, currently in
 * stringed/stringed_unassigned.cpp.  The preceding function
 * (SEH_LocalizeTextMessage) ends at 0x004AA352 and the following one (the
 * CStringEdString constructor) starts at 0x004AA7F0, with NOP padding in each
 * gap.
 *
 * Multibyte text handling for the CJK builds, plus the
 * printed-length measurement the console and the HUD use.  No STL, no
 * localization tables -- this unit is purely "how do I read and measure a
 * character".  Its consumers are all display code: R_GetAsianCode,
 * R_GetCharacterGlyph and R_GetGlyphHorizAdvance in the renderer,
 * CL_CgameSystemCalls, CL_ConsolePrint_AddLine and Field_AdjustScroll.
 *
 * Not server-relevant: a dedicated server never measures or decodes display
 * text.
 *
 * Inventory, in address order:
 *   0x004AA360  Korean_ValidKSC5601HangulCode
 *   0x004AA380  Korean_ValidKSC5601HangulCodeWord
 *   0x004AA3A0  Korean_CollapseKSC5601HangulCode
 *   0x004AA3E0  Taiwanese_ValidBig5Code
 *   0x004AA410  Taiwanese_IsTrailingPunctuation
 *   0x004AA430  Taiwanese_CollapseBig5Code
 *   0x004AA490  Japanese_ValidShiftJISCode
 *   0x004AA4C0  Japanese_ValidShiftJISCodeWord
 *   0x004AA4F0  Japanese_IsTrailingPunctuation
 *   0x004AA510  Japanese_CollapseShiftJISCode
 *   0x004AA580  SEH_ReadCharFromString
 *   0x004AA690  Language_IsAsian
 *   0x004AA6A0  Language_UsesSpaceWordBreak_m
 *   0x004AA6C0  SEH_PrintStrlen
 *
 * The unit NAME is inferred.  The 1.3 Mac xSYM tables list three stringed
 * units and this block has to be one of them; "ingame" fits code that only
 * runs while text is on screen, and "interface" fits the CStringEdPackage
 * block that follows.  The alternative reading is that this block is the tail
 * of stringed_hooks.cpp -- SEH_ReadCharFromString and SEH_PrintStrlen do carry
 * the hooks prefix, and the 1.3 Mac stringed_hooks.c has Language_IsAsian and
 * g_currentAsian in it.  The ADDRESS RANGE is solid either way.
 *
 * Language_IsAsian sits at 0x004AA690, between Japanese_CollapseShiftJISCode
 * and SEH_PrintStrlen.  MSVC emits functions in source order within a unit, so
 * in the 1.1 Windows build it belongs to THIS unit, not to the hooks one where
 * the 1.3 Mac source puts it.
 *
 * @fidelity: raw
 */

#include "../qcommon/cod1_types.h"


/* ISO C requires a translation unit to contain at least one declaration. */
typedef int stringed_ingame_placeholder_t;
