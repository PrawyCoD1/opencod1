#ifndef CMD_HISTORY_H
#define CMD_HISTORY_H

/* CoD 1.5 CL_GetUserCmd uses a 128-entry ring (mask 0x7f).
 * Shared by engine storage and cgame prediction/loading/disconnect checks. */
#define CMD_BACKUP 128
#define CMD_MASK (CMD_BACKUP - 1)
#define UCMD_SIZE 24

#endif
