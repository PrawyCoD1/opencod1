#ifndef CL_CINEMATIC_H
#define CL_CINEMATIC_H

#define MAX_VIDEO_HANDLES   16      /* cmp ecx, 10h at 0x00407604 */

/* connstate_t, coduo client/cgame.h:630-640.  Only the three values cl_cin_mp.c
 * actually tests are declared here; cl_vm.h carries the rest for the other units. */
#define CA_DISCONNECTED     0
#define CA_CINEMATIC        7       /* cmp cls_state, 7 at 0x00407653 */
#define CA_LOGO             8

/* Q3's e_status unchanged: FMV_EOF is the 2 every bounds check compares against, and RoQReset parks the handle in FMV_LOOPED = 5 (0x00406EF4). */
typedef enum {
	FMV_IDLE,
	FMV_PLAY,
	FMV_EOF,
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

#define ROQ_FILE            0x1084
#define ROQ_QUAD            0x1000
#define ROQ_QUAD_INFO       0x1001
#define ROQ_CODEBOOK        0x1002
#define ROQ_QUAD_VQ         0x1011
#define ROQ_QUAD_JPEG       0x1012
#define ROQ_QUAD_HANG       0x1013
#define ROQ_PACKET          0x1030
#define ZA_SOUND_MONO       0x1020
#define ZA_SOUND_STEREO     0x1021

#define MAXSIZE             8       /* cmp esi, 8 in recurseQuad, 0x00406A80 */
#define MINSIZE             4       /* cmp esi, 4 in recurseQuad, 0x00406AD1 */

#define DEFAULT_CIN_WIDTH   512     /* mov dword_877828[eax], 512 in CIN_PlayCinematic, 0x00407A5B */
#define DEFAULT_CIN_HEIGHT  512

#define LETTERBOX_OFFSET    105

#define CIN_system          1
#define CIN_loop            2
#define CIN_hold            4
#define CIN_silent          8
#define CIN_shader          16
#define CIN_letterbox       32

typedef struct {
	char			fileName[MAX_OSPATH];
	int				CIN_WIDTH, CIN_HEIGHT;
	int				xpos, ypos, width, height;
	qboolean		looping, holdAtEnd, dirty, alterGameState, silent, shader;
	qboolean		letterBox;
	qboolean		soundStarted;
	int				iFile;
	e_status		status;
	unsigned int	startTime;
	unsigned int	lastAdvanceTime;
	unsigned int	lastTime;
	long			tfps;
	long			RoQPlayed;
	long			ROQSize;
	unsigned int	RoQFrameSize;
	long			onQuad;
	long			numQuads;
	long			samplesPerLine;
	unsigned int	roq_id;
	long			screenDelta;

	void			(*VQ0)( byte *status, void *qdata );
	void			(*VQ1)( byte *status, void *qdata );
	void			(*VQNormal)( byte *status, void *qdata );
	void			(*VQBuffer)( byte *status, void *qdata );

	long			samplesPerPixel;
	byte			*gray;
	unsigned int	xsize, ysize, maxsize, minsize;

	qboolean		half, smootheddouble;
	long			inMemory;
	long			normalBuffer0;
	long			roq_flags;
	long			roqF0;
	long			roqF1;
	long			t[2];
	long			roqFPS;
	int				playonwalls;
	byte			*buf;
	long			drawX, drawY;
} cinematic_t;

COD1_ASSERT_SIZE( cinematic_t, 464 );

#define CIN_SCRATCH_SIZE    0x250614

#define CIN_ENTRY( i )      ( (cinematic_t *)cinTable + (i) )

#endif
