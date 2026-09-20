#ifndef CL_UPDATE_H
#define CL_UPDATE_H
#ifdef __cplusplus
extern "C" {
#endif
void CL_UpdateInit(void);
void CL_UpdateCheck(void);
void CL_UpdateStart(void);
void CL_UpdateFrame(void);
/* Returns -1 for an ordinary game launch. */
int CL_UpdateHelper(const char *commandLine);
#ifdef __cplusplus
}
#endif
#endif
