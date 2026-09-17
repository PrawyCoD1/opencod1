/*
 * universal/com_sndalias.h -- the sound alias system's state as ONE object.
 *
 * Retail CG_LOADSOUNDALIASES (cgame syscall 190, CL_CgameSystemCalls
 * 0x004028xx) is Com_LoadSoundAliases(name, 1) inlined.  Its load loop stores
 * the current CSV name to 0x008931D0 and Com_LoadSoundAliasDefaults
 * (0x00432AD0) reads 0x008931D0 back to stamp node->sourceFile; retail has
 * exactly one object there.  Com_SortTempSoundAliases_r's duplicate check
 * compares sourceFile, so two rows of equal name and sequence from different
 * CSVs (e.g. 'null' in iw_sound.csv and a downloaded pk3) must carry
 * different file names to both survive.
 *
 * SFxHelper__RegisterSound (0x0049A290) is Com_SoundAliasIndex(alias, 1)
 * inlined against snd_aliasTable[1]/snd_aliasTableCount[1] (`Block`/
 * `dword_893DE8`).
 *
 * com_sndalias.c's objects are not `static`; the generated spellings are
 * macros landing inside them at the offsets the retail addresses imply.  A
 * Com_BindHunkGlobals-style binder does not work here: the hunk consumers read
 * their generated name as a POINTER to the real word, these read it as the
 * value itself and assign to it.
 *
 * Include AFTER qcommon/qcommon.h (needs `byte`) and after qcommon/cod1_globals.h
 * where a unit includes that at all -- the macros below would otherwise rewrite
 * the generated declarations themselves.
 *
 *   0x008931D0  const char       *snd_currentFile
 *   0x008931D4  byte              snd_aliasSetLoaded[4]      (.D4/.D5/.D6 live)
 *   0x008931D8  snd_alias_t      *snd_aliasHashTable[3][256] (slices .1D8/.5D8/.9D8)
 *   0x00893DD8  snd_alias_t      *snd_aliasTable[3]          (+4 = [1], +8 = [2])
 *   0x00893DE4  int               snd_aliasTableCount[3]     (+4 = [1], +8 = [2])
 *   0x00893DF0  snd_aliasBuild_t *snd_aliasBuildHead
 *   0x00893DF4  char              snd_aliasLocalizedName[64]
 *   0x00893E34  int               snd_aliasTableChecksum[3]
 *   0x00893E40  char              snd_subtitleReference[1024]
 *
 * `Block` (0x00893DDC, snd_aliasTable[1]) is deliberately NOT macroed: it is
 * also a local variable name in a dozen other units, so a macro would rewrite
 * them.  Its four live call sites say snd_aliasTable[1].
 *
 * cod1_globals.c still reserves the storage under the generated names; dead.
 *
 * @fidelity: verified
 */
#ifndef COM_SNDALIAS_H
#define COM_SNDALIAS_H

struct snd_alias_s;
struct snd_aliasBuild_s;

#define SND_ALIAS_LOCALE_COUNT      3       /* 0 = menu/UI, 1 = in-game, 2 = localized/map */
#define SND_ALIAS_LOADED_SLOTS      4
#define SND_ALIAS_HASH_SIZE         256
#define SND_ALIAS_HASH_SLICE_BYTES  ( SND_ALIAS_HASH_SIZE * (int) sizeof( struct snd_alias_s * ) )

#define SND_LOCALE_MENU             0
#define SND_LOCALE_INGAME           1
#define SND_LOCALE_LOCALIZED        2

extern const char   *snd_currentFile;                                   /* 0x008931D0 */
extern byte         snd_aliasSetLoaded[SND_ALIAS_LOADED_SLOTS];         /* 0x008931D4 */
extern struct snd_alias_s *snd_aliasHashTable[SND_ALIAS_LOCALE_COUNT][SND_ALIAS_HASH_SIZE];
																		/* 0x008931D8 */
extern struct snd_alias_s *snd_aliasTable[SND_ALIAS_LOCALE_COUNT];      /* 0x00893DD8 */
extern int          snd_aliasTableCount[SND_ALIAS_LOCALE_COUNT];        /* 0x00893DE4 */
extern struct snd_aliasBuild_s *snd_aliasBuildHead;                     /* 0x00893DF0 */
extern char         snd_aliasLocalizedName[64];                         /* 0x00893DF4 */
extern int          snd_aliasTableChecksum[SND_ALIAS_LOCALE_COUNT];     /* 0x00893E34 */
extern char         snd_subtitleReference[1024];                        /* 0x00893E40 */

/* ---- generated spellings -> the one real object ------------------------- */

/* cod1_globals types this char[4]; every consumer immediately casts it back to
 * `const char *`, which is what the retail word is.  The macro therefore has to
 * present as the ADDRESS of the pointer, not as an array of char. */
#define snd_currentFilename     ( (char *) (void *) &snd_currentFile )

#define snd_aliasLoaded         snd_aliasSetLoaded
#define snd_aliasLoaded_ui      ( snd_aliasSetLoaded[SND_LOCALE_INGAME] )     /* 0x008931D5 */
#define byte_8931D6             ( snd_aliasSetLoaded[SND_LOCALE_LOCALIZED] )  /* 0x008931D6 */

#define snd_aliasHash           snd_aliasHashTable
#define unk_8935D8              ( snd_aliasHashTable[SND_LOCALE_INGAME] )     /* 0x008935D8 */
#define dword_8939D8            ( snd_aliasHashTable[SND_LOCALE_LOCALIZED] )  /* 0x008939D8 */

#define snd_aliasList           snd_aliasTable
#define snd_aliasList_uiOwned   ( snd_aliasTable[SND_LOCALE_LOCALIZED] )      /* 0x00893DE0 */

#define snd_aliasCount          snd_aliasTableCount
#define dword_893DE8            ( snd_aliasTableCount[SND_LOCALE_INGAME] )    /* 0x00893DE8 */
#define dword_893DEC            ( snd_aliasTableCount[SND_LOCALE_LOCALIZED] ) /* 0x00893DEC */

#define snd_tempBuildHead       snd_aliasBuildHead

/* 0x00893DF4 is the LOCALIZED SET NAME, not a subtitle: Com_LoadSoundAliases
 * 0x00433E90 stricmps it against `name` and strcpys `name` into it.  The real
 * subtitle buffer is snd_subtitleBuf/snd_subtitleReference at 0x00893E40. */
#define snd_subtitleStr         snd_aliasLocalizedName

#define snd_aliasChecksum       snd_aliasTableChecksum
#define dword_893E38            ( snd_aliasTableChecksum[SND_LOCALE_INGAME] ) /* 0x00893E38 */

#define snd_subtitleBuf         snd_subtitleReference

#endif /* COM_SNDALIAS_H */
