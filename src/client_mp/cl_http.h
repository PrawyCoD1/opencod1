#ifndef CL_HTTP_H
#define CL_HTTP_H
void CL_HTTPInit(void);
void CL_HTTPCancel(void);
void CL_HTTPFrame(void);
void CL_HTTPGamestate(void);
int CL_HTTPBegin(const char *remoteName);
int CL_HTTPRedirect(msg_t *msg);
#endif
