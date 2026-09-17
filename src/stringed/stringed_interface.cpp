/*
 * Reconstructed from Call of Duty 1.1 (Windows, CoDMP.exe).
 *
 * Original translation unit:
 *   /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/stringed/stringed_interface.cpp
 *
 * NOT YET RECONSTRUCTED.
 *
 * Retail range 0x004AA7F0-0x004AFDA8, 252 functions, currently in
 * stringed/stringed_unassigned.cpp.  SEH_PrintStrlen ends at 0x004AA7EB and
 * MD5Transform starts at 0x004AFDB0, with NOP padding in each gap.
 *
 * CStringEdPackage -- the .str database -- and the SE_* free
 * functions over it, compiled as C++ against MSVC's STL.  Roughly 60 of the 252
 * functions are the localization code proper; the other ~190 are template
 * instantiations of std::string, std::map<std::string, CStringEdString> and
 * std::vector<std::string> that MSVC emitted into this object because this is
 * where they were used.  The STL bodies are identifiable by their throw strings
 * ("map/set<T> too long", "vector<T> too long", "invalid map/set<T> iterator")
 * and by calls to std::_String_base::_Xlen / _Xran.
 *
 * ONE GLOBAL: TheStringPackage at 0x01407450, constructed from the CRT
 * static-init table by 0x004AA830 and torn down at exit by 0x004AA910.
 *
 * LAYOUTS:
 *
 *   std::string       28 bytes: allocator +0, _Bx.buf[16] +4, _Mysize +20,
 *                     _Myres +24.  SSO threshold 16 -- the text is inline at +4
 *                     when _Myres < 16, otherwise +4 holds the pointer.
 *   CStringEdString   60 bytes: std::string localized +0,
 *                     std::string reference +28, int flags +56.
 *   map node         104 bytes: _Left +0, _Parent +4, _Right +8,
 *                     key std::string +12, CStringEdString value +40,
 *                     _Color +100, _Isnil +101.
 *   CStringEdPackage >=180 bytes: int +0; std::string +4/+32/+60/+88; int +116;
 *                     std::map +120 (head +124, size +128);
 *                     int +132 "return the reference text, not the localized
 *                     one"; std::vector +136 (first +140, last +144, end +148);
 *                     std::map +152 (head +156, size +160);
 *                     std::vector +164 (first +168, last +172, end +176).
 *
 * ENTRY POINTS.  SE_GetString is the single gate: every SEH_ entry point in
 * stringed_hooks.cpp goes through it.
 *
 *   SE_GetString              0x004ABF00   the map lookup
 *   CStringEdPackage::Clear   0x004AAAA0   13 callers, incl. FS_Restart
 *   SE_LoadLanguageFile_m     0x004ABC50   the .str reader
 *   SE_ParseLine_m            0x004AB470   keyword dispatch
 *   SE_GetNumLanguages        0x004AC1B0   0 makes SEH_StringEd_SetLanguage-
 *                                          Strings report "No language string
 *                                          information available for %s"
 *   SE_LoadAllLanguageFiles_m 0x004AC7B0   returns an ERROR STRING or NULL,
 *                                          not a boolean
 *
 * THE .str FORMAT, from the parser's own literals: VERSION, CONFIG, FILENOTES,
 * NOTES, REFERENCE, FLAGS, ENDMARKER, and per-language LANG_<NAME> lines.
 * Keys are "<UPPERCASED BASENAME>_<REFERENCE>"; "#same" aliases an entry to the
 * reference language.  The line pump is SE_GetNextLine_m (0x004AAF30), which
 * trims, strips // comments outside quotes, and unescapes \n.
 *
 * The unit NAME is inferred: the 1.3 Mac xSYM tables name three stringed units
 * and give no addresses, so "interface" versus "ingame" for this block and the
 * one before it rests on which name better fits the content.  The range is
 * solid regardless.
 *
 * @fidelity: raw
 */

#include "../qcommon/cod1_types.h"


/* ISO C requires a translation unit to contain at least one declaration. */
typedef int stringed_interface_placeholder_t;
