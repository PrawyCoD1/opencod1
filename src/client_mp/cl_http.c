/* 1.5 connected WWW-download protocol; WinHTTP transport owned by a worker.
 * Only the main thread touches engine filesystem, cvars or connection state. */
#include "../qcommon/qcommon.h"
#include "../qcommon/cod1_globals.h"
#include "cl_records.h"
#include "cl_http.h"
#include <windows.h>
#include <winhttp.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#pragma comment(lib, "winhttp.lib")

extern void CL_AddReliableCommand(const char *);
extern void CL_NextDownload(void);
extern void CL_WritePacket(void);
extern char *MSG_ReadString(msg_t *);
extern int MSG_ReadLong(msg_t *);
extern int FS_VerifyDownload(const char *, const char *);
extern void FS_BuildOSPath_Internal(const char *, const char *, char *, int);
extern cvar_t *fs_homepath;

typedef struct httpJob_s {
    struct httpJob_s *next;
    HANDLE thread;
    volatile LONG cancel, count, total;
    int result, expected, redirected;
    DWORD error, status;
    const char *stage;
    char url[2048], path[MAX_OSPATH], temp[MAX_OSPATH];
    char local[MAX_QPATH * 4], remote[MAX_QPATH * 4];
} httpJob_t;
static httpJob_t *active, *retired;
static cvar_t *www;
static unsigned serial;
static int failed, waiting;
static char requested[MAX_QPATH * 4];

static int HTTP_SafeName(const char *name) {
    const unsigned char *p = (const unsigned char *)name;
    size_t n = strlen(name);
    if (n < 5 || n >= MAX_QPATH * 4 || *p == '/' || strstr(name, "..") ||
        Q_stricmp(name + n - 4, ".pk3")) return 0;
    for (; *p; ++p)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '/' || *p == '_' || *p == '-' || *p == '.')) return 0;
    return 1;
}

static DWORD WINAPI HTTP_Worker(void *arg) {
    httpJob_t *j = arg;
    HINTERNET session = NULL, connection = NULL, request = NULL;
    HANDLE file = INVALID_HANDLE_VALUE;
    URL_COMPONENTS u;
    WCHAR url[2048], host[256], resource[2048];
    DWORD status, size, got, written, policy = WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH;
    byte buffer[32768];
    int count = 0;
    j->stage = "URL validation";
    memset(&u, 0, sizeof(u)); u.dwStructSize = sizeof(u);
    u.dwHostNameLength = u.dwUrlPathLength = u.dwExtraInfoLength = (DWORD)-1;
    u.dwUserNameLength = u.dwPasswordLength = (DWORD)-1;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, j->url, -1, url, 2048) ||
        !WinHttpCrackUrl(url, 0, 0, &u) ||
        (u.nScheme != INTERNET_SCHEME_HTTP && u.nScheme != INTERNET_SCHEME_HTTPS) ||
        u.dwUserNameLength || u.dwPasswordLength || !u.dwHostNameLength ||
        u.dwHostNameLength >= 256 || u.dwUrlPathLength + u.dwExtraInfoLength >= 2048) goto done;
    memcpy(host, u.lpszHostName, u.dwHostNameLength * sizeof(WCHAR)); host[u.dwHostNameLength] = 0;
    memcpy(resource, u.lpszUrlPath, u.dwUrlPathLength * sizeof(WCHAR));
    if (u.dwExtraInfoLength) memcpy(resource + u.dwUrlPathLength, u.lpszExtraInfo, u.dwExtraInfoLength * sizeof(WCHAR));
    resource[u.dwUrlPathLength + u.dwExtraInfoLength] = 0;
    j->stage = "opening HTTP session";
    session = WinHttpOpen(L"Call of Duty 1.1x", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!session) goto done;
    WinHttpSetTimeouts(session, 10000, 10000, 10000, 10000);
    j->stage = "connecting";
    connection = WinHttpConnect(session, host, u.nPort, 0);
    if (!connection) goto done;
    j->stage = "opening request";
    request = WinHttpOpenRequest(connection, L"GET", resource, NULL, WINHTTP_NO_REFERER,
                                 WINHTTP_DEFAULT_ACCEPT_TYPES, u.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
    if (!request) goto done;
    WinHttpSetOption(request, WINHTTP_OPTION_AUTOLOGON_POLICY, &policy, sizeof(policy));
    j->stage = "sending request / receiving headers";
    if (InterlockedCompareExchange(&j->cancel, 0, 0) ||
        !WinHttpSendRequest(request, NULL, 0, NULL, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, NULL)) goto done;
    size = sizeof(status);
    j->stage = "HTTP status";
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             NULL, &status, &size, NULL)) goto done;
    j->status = status;
    SetLastError(0);
    if (status != 200) goto done;
    j->stage = "content length";
    size = sizeof(status);
    if (WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                            NULL, &status, &size, NULL)) {
        if (!status || status > INT_MAX || (j->expected && status != (DWORD)j->expected)) goto done;
        InterlockedExchange(&j->total, (LONG)status);
    }
    j->stage = "opening temporary file";
    file = CreateFileA(j->path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) goto done;
    for (;;) {
        j->stage = "reading response";
        if (InterlockedCompareExchange(&j->cancel, 0, 0) ||
            !WinHttpReadData(request, buffer, sizeof(buffer), &got)) goto done;
        if (!got) break;
        j->stage = "response size limit";
        if (got > (DWORD)(INT_MAX - count) || (j->expected && got > (DWORD)(j->expected - count))) goto done;
        j->stage = "writing temporary file";
        if (!WriteFile(file, buffer, got, &written, NULL) || written != got) goto done;
        count += got;
        InterlockedExchange(&j->count, count);
    }
    j->stage = "completed response length";
    SetLastError(0);
    if (count && (!j->expected || count == j->expected) &&
        (!j->total || count == j->total) && !InterlockedCompareExchange(&j->cancel, 0, 0)) j->result = 1;
done:
    j->error = GetLastError();
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (session) WinHttpCloseHandle(session);
    if (!j->result || InterlockedCompareExchange(&j->cancel, 0, 0)) DeleteFileA(j->path);
    return 0;
}

void CL_HTTPInit(void) {
    www = Cvar_Get("cl_wwwDownload", "1", CVAR_ARCHIVE | CVAR_USERINFO);
}

void CL_HTTPCancel(void) {
    if (active) {
        InterlockedExchange(&active->cancel, 1);
        active->next = retired; retired = active; active = NULL;
    }
    failed = waiting = 0; requested[0] = 0;
}

void CL_HTTPGamestate(void) {
    int keepFailed = failed;
    CL_HTTPCancel();
    failed = keepFailed;
}

static int HTTP_Start(const char *url, int expected, int redirected) {
    httpJob_t *j;
    fileHandle_t f;
    if (!www || !www->integer || failed || active || expected < 0 ||
        !HTTP_SafeName(requested) || !HTTP_SafeName((char *)cls_downloadName) ||
        (Q_stricmpn(url, "http://", 7) && Q_stricmpn(url, "https://", 8)) || strlen(url) >= 2048) return 0;
    j = calloc(1, sizeof(*j));
    if (!j) return 0;
    Q_strncpyz(j->url, url, sizeof(j->url));
    Q_strncpyz(j->local, (char *)cls_downloadName, sizeof(j->local));
    Q_strncpyz(j->remote, requested, sizeof(j->remote));
    Com_sprintf(j->temp, sizeof(j->temp), "%s.www-%lu-%u.tmp", j->local, GetCurrentProcessId(), ++serial);
    FS_BuildOSPath_Internal(fs_homepath->string, j->temp, j->path, 1);
    if (!j->path[0]) { free(j); return 0; }
    f = FS_SV_FOpenFileWrite(j->temp);
    if (!f) { free(j); return 0; }
    FS_FCloseFile(f);
    j->expected = expected; j->total = expected; j->redirected = redirected;
    j->thread = CreateThread(NULL, 0, HTTP_Worker, j, 0, NULL);
    if (!j->thread) { DeleteFileA(j->path); free(j); return 0; }
    active = j;
    Cvar_Set2("cl_downloadName", j->url, qtrue);
    Cvar_SetValue("cl_downloadSize", (float)expected);
    Cvar_SetValue("cl_downloadCount", 0);
    Cvar_SetValue("cl_downloadTime", (float)cls_realtime);
    Com_Printf("HTTP download: %s\n", url);
    return 1;
}

int CL_HTTPBegin(const char *remoteName) {
    char base[1536], url[2048];
    const char *value;
    int i;
    if (!www) CL_HTTPInit();
    waiting = 0;
    Q_strncpyz(requested, remoteName, sizeof(requested));
    if (!www->integer || failed) return 0;
    /* Read the current server's info, never a leftover local cvar. */
    for (i = 0; i < 2; ++i) {
        const char *info = cl_gameState_stringData + cl_gameState_stringOffsets[i];
        value = Info_ValueForKey(info, "sv_wwwBaseURL");
        if (!*value) continue;
        Q_strncpyz(base, value, sizeof(base));
        /* A 1.5-style server will send its own redirect in response to download. */
        if (atoi(Info_ValueForKey(info, "sv_wwwDownload"))) return 0;
        while (*base && base[strlen(base)-1] == '/') base[strlen(base)-1] = 0;
        Com_sprintf(url, sizeof(url), "%s/%s", base, remoteName);
        return HTTP_Start(url, 0, 0);
    }
    return 0;
}

int CL_HTTPRedirect(msg_t *msg) {
    char url[2048];
    int size, flags;
    if (msg->cursize - msg->readcount < 2 ||
        *(unsigned short *)(msg->data + msg->readcount) != 65535 ||
        *(int *)cls_downloadBlock != 0 || cls_downloadCount != 0) return 0;
    msg->readcount += 2;
    Q_strncpyz(url, MSG_ReadString(msg), sizeof(url));
    size = MSG_ReadLong(msg); flags = MSG_ReadLong(msg);
    if (size <= 0 || flags < 0 || msg->readcount > msg->cursize)
        Com_Error(ERR_DROP, "Invalid HTTP download redirect");
    if (active || waiting) return 1; /* retransmitted redirect */
    if (!requested[0] || !cls_downloadName[0]) {
        CL_AddReliableCommand("stopdl"); return 1;
    }
    CL_AddReliableCommand("wwwdl ack");
    /* Keep the connection alive. Servers requiring a browser/disconnected
       transfer receive the normal 1.5 failure handshake and use UDP. */
    if (flags || !HTTP_Start(url, size, 1)) {
        failed = waiting = 1;
        CL_AddReliableCommand("wwwdl fail");
    }
    CL_WritePacket();
    return 1;
}

void CL_HTTPFrame(void) {
    httpJob_t **link = &retired, *j;
    while ((j = *link) != NULL) {
        if (WaitForSingleObject(j->thread, 0) == WAIT_OBJECT_0) {
            *link = j->next; CloseHandle(j->thread); DeleteFileA(j->path); free(j);
        } else link = &j->next;
    }
    if (!(j = active)) return;
    Cvar_SetValue("cl_downloadCount", (float)InterlockedCompareExchange(&j->count, 0, 0));
    Cvar_SetValue("cl_downloadSize", (float)InterlockedCompareExchange(&j->total, 0, 0));
    if (WaitForSingleObject(j->thread, 0) != WAIT_OBJECT_0) return;
    CloseHandle(j->thread); active = NULL;
    if (j->result && FS_VerifyDownload(j->path, j->remote)) {
        char target[MAX_OSPATH];
        FS_BuildOSPath_Internal(fs_homepath->string, j->local, target, 0);
        if (!MoveFileExA(j->path, target, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            DeleteFileA(j->path); free(j);
            Com_Error(ERR_DROP, "Could not install downloaded PK3");
            return;
        }
        cls_downloadName[0] = cls_downloadTempName[0] = 0;
        Cvar_Set2("cl_downloadName", "", qtrue);
        if (j->redirected) CL_AddReliableCommand("wwwdl done");
        Com_Printf("HTTP download complete: %s\n", j->remote);
        free(j);
        CL_NextDownload();
        CL_WritePacket();
    } else {
        DeleteFileA(j->path);
        failed = 1;
        Cvar_Set2("cl_downloadName", j->remote, qtrue);
        if (!j->result)
            Com_Printf("HTTP failure: %s at %s (Windows error %lu, HTTP %lu, %ld/%ld bytes)\n",
                       j->remote, j->stage, j->error, j->status, j->count, j->total);
        Com_Printf("HTTP %s: %s; falling back to UDP\n", j->result ? "verification failed" : "transfer failed", j->remote);
        if (j->redirected) {
            waiting = 1; CL_AddReliableCommand("wwwdl fail");
        } else {
            cls_downloadCount = clc_downloadSize = 0;
            *(int *)cls_downloadBlock = 0;
            Cvar_SetValue("cl_downloadCount", 0); Cvar_SetValue("cl_downloadSize", 0);
            Cvar_SetValue("cl_downloadTime", (float)cls_realtime);
            CL_AddReliableCommand(va("download %s", j->remote));
        }
        free(j); CL_WritePacket();
    }
}
