#ifndef CL_CONWINDOWS_H
#define CL_CONWINDOWS_H

typedef struct {
	int  *times;         /* +0x00 */
	int  *endTimes;      /* +0x04 */
	int  *lines;         /* +0x08 */
	int   index;         /* +0x0C */
	int   size;          /* +0x10 */
	int   activeLines;   /* +0x14 */
	int   scrollTime;    /* +0x18 */
	int   fadeIn;        /* +0x1C */
	int   fadeOut;       /* +0x20 */
} conMessageWindow_t;

extern conMessageWindow_t con_notifyWindowRec;
extern conMessageWindow_t con_boldWindowRec;
extern conMessageWindow_t con_subtitleWindowRec;
extern conMessageWindow_t con_miniconsoleWindowRec;

/* notify (game message), retail 0x0142EFCC ---- */
#define con_notifyWindow        (*(int *)&con_notifyWindowRec.times)        /* +0x00 */
#define dword_142EFD0           (*(int *)&con_notifyWindowRec.endTimes)     /* +0x04 */
#define dword_142EFD4           (*(int *)&con_notifyWindowRec.lines)        /* +0x08 */
#define dword_142EFD8           con_notifyWindowRec.index                   /* +0x0C */
#define con_gamemessagelines    con_notifyWindowRec.size                    /* +0x10 */
#define dword_142EFE0           con_notifyWindowRec.activeLines             /* +0x14 */
#define dword_142EFE4           con_notifyWindowRec.scrollTime              /* +0x18 */
#define dword_142EFE8           con_notifyWindowRec.fadeIn                  /* +0x1C */
#define dword_142EFEC           con_notifyWindowRec.fadeOut                 /* +0x20 */

/* bold game message, retail 0x0142F050 ---- */
#define con_boldWindow          (*(int *)&con_boldWindowRec.times)          /* +0x00 */
#define dword_142F054           (*(int *)&con_boldWindowRec.endTimes)       /* +0x04 */
#define dword_142F058           (*(int *)&con_boldWindowRec.lines)          /* +0x08 */
#define dword_142F05C           con_boldWindowRec.index                     /* +0x0C */
#define dword_142F060           con_boldWindowRec.size                      /* +0x10 */
#define dword_142F064           con_boldWindowRec.activeLines               /* +0x14 */
#define dword_142F068           con_boldWindowRec.scrollTime                /* +0x18 */
#define dword_142F06C           con_boldWindowRec.fadeIn                    /* +0x1C */
#define dword_142F070           con_boldWindowRec.fadeOut                   /* +0x20 */

/* subtitle, retail 0x0142F0D4 ---- */
#define con_subtitleWindow      (*(int *)&con_subtitleWindowRec.times)      /* +0x00 */
#define dword_142F0D8           (*(int *)&con_subtitleWindowRec.endTimes)   /* +0x04 */
#define dword_142F0DC           (*(int *)&con_subtitleWindowRec.lines)      /* +0x08 */
#define dword_142F0E0           con_subtitleWindowRec.index                 /* +0x0C */
#define dword_142F0E4           con_subtitleWindowRec.size                  /* +0x10 */
#define dword_142F0E8           con_subtitleWindowRec.activeLines           /* +0x14 */
#define dword_142F0EC           con_subtitleWindowRec.scrollTime            /* +0x18 */
#define dword_142F0F0           con_subtitleWindowRec.fadeIn                /* +0x1C */
#define dword_142F0F4           con_subtitleWindowRec.fadeOut               /* +0x20 */

/* mini-console, retail 0x0142F5A8 ---- */
#define con_miniconsoleWindow   (*(int *)&con_miniconsoleWindowRec.times)   /* +0x00 */
#define dword_142F5AC           (*(int *)&con_miniconsoleWindowRec.endTimes)/* +0x04 */
#define dword_142F5B0           (*(int *)&con_miniconsoleWindowRec.lines)   /* +0x08 */
#define dword_142F5B4           con_miniconsoleWindowRec.index              /* +0x0C */
#define dword_142F5B8           con_miniconsoleWindowRec.size               /* +0x10 */
#define dword_142F5BC           con_miniconsoleWindowRec.activeLines        /* +0x14 */
#define dword_142F5C0           con_miniconsoleWindowRec.scrollTime         /* +0x18 */
#define dword_142F5C4           con_miniconsoleWindowRec.fadeIn             /* +0x1C */
#define dword_142F5C8           con_miniconsoleWindowRec.fadeOut            /* +0x20 */

#endif
