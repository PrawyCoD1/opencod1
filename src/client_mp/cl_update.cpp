/* The update origin is compiled in, not supplied by a game server. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include "cl_update.h"
#include "../qcommon/product_build.h"
#ifndef UPDATE_PUBLIC_KEY_HEADER
#define UPDATE_PUBLIC_KEY_HEADER "cl_update_key.h"
#endif
#include UPDATE_PUBLIC_KEY_HEADER
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "advapi32.lib")
extern "C" {
void Com_Printf(const char *, ...);
void *Cvar_Get(const char *, const char *, int);
void Cvar_Set2(const char *, const char *, int);
void Cbuf_AddText(const char *);
}

namespace {
const unsigned updateBuild = PRODUCT_BUILD_NUMBER;
const char *endpoint = "http://91.99.100.203/updater.txt";
const int cvarROM = 64;
struct File { std::string path, url, hash; };
struct Manifest { unsigned build = 0; bool forced = false; std::vector<File> files; std::string signedText; };
struct Job {
    HANDLE thread = NULL;
    bool install = false, ok = false;
    Manifest manifest;
    std::string error, root;
    volatile LONG downloaded = 0;
};
Job *job = NULL;
Manifest available;
bool ready = false;

std::string Trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
std::string Lower(std::string s) {
    for (char &c : s) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return s;
}
bool Allowed(const std::string &p) {
    static const char *names[] = {"codmp.exe", "cod1x.dll", "main/cgame_mp_x86.dll",
        "main/game_mp_x86.dll", "main/ui_mp_x86.dll"};
    for (const char *name : names) if (Lower(p) == name) return true;
    return false;
}
bool HashValid(const std::string &s) {
    if (s.size() != 64) return false;
    for (char c : s) if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    return true;
}
bool URL(const std::string &s) {
    if (s.size() > 2047 || s.find_first_of("\r\n\t ") != std::string::npos) return false;
    return s.compare(0, 7, "http://") == 0 || s.compare(0, 8, "https://") == 0;
}
bool Parse(const std::string &text, Manifest &m, std::string &error) {
    std::istringstream input(text);
    std::string line;
    bool hasBuild = false, hasForced = false;
    m = Manifest();
    while (std::getline(input, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) { error = "Invalid updater.txt line"; return false; }
        std::string key = Trim(line.substr(0, eq)), value = Trim(line.substr(eq + 1));
        if (key == "build") {
            if (hasBuild || value.empty() || value.find_first_not_of("0123456789") != std::string::npos || value.size() > 9) {
                error = "Invalid update build"; return false;
            }
            m.build = (unsigned)strtoul(value.c_str(), NULL, 10); hasBuild = true;
        } else if (key == "forced") {
            if (hasForced || (value != "0" && value != "1")) { error = "Invalid forced field"; return false; }
            hasForced = true; m.forced = value == "1";
        } else if (key == "file") {
            size_t p = value.find('|'), q = p == std::string::npos ? p : value.find('|', p + 1);
            if (p == std::string::npos || q == std::string::npos) { error = "Expected file=path|url|sha256"; return false; }
            File f = {Trim(value.substr(0, p)), Trim(value.substr(p + 1, q - p - 1)), Lower(Trim(value.substr(q + 1)))};
            if (!HashValid(f.hash)) { error = "Missing or invalid file SHA-256"; return false; }
            m.files.push_back(f);
        } else { error = "Unknown updater.txt field: " + key; return false; }
    }
    if (!hasBuild || !m.build || m.files.empty() || m.files.size() > 5) { error = "Incomplete update manifest"; return false; }
    for (size_t i = 0; i < m.files.size(); ++i) {
        const File &f = m.files[i];
        if (!Allowed(f.path) || !URL(f.url)) { error = "Unsupported update path or URL"; return false; }
        for (size_t n = 0; n < i; ++n) if (Lower(m.files[n].path) == Lower(f.path)) { error = "Duplicate update target"; return false; }
    }
    return true;
}

bool Authenticate(const std::string &text, Manifest &m, std::string &error) {
    m = Manifest();
    const std::string prefix = "cod-update-rsa3072-sha256-v1=";
    const size_t sigBytes = 384, start = prefix.size() + sigBytes * 2 + 1;
    error = "Missing or invalid publisher signature";
    if (text.size() <= start || text.size() > 65536 || text.compare(0, prefix.size(), prefix) || text[start - 1] != '\n') return false;
    BYTE signature[sigBytes];
    for (size_t i = 0; i < sigBytes; ++i) {
        unsigned value = 0;
        for (size_t n = 0; n < 2; ++n) {
            char c = text[prefix.size() + i * 2 + n];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
            value = value * 16 + (c <= '9' ? c - '0' : c - 'a' + 10);
        }
        signature[i] = (BYTE)value;
    }
    HCRYPTPROV provider = 0; HCRYPTKEY key = 0; HCRYPTHASH hash = 0;
    bool ok = CryptAcquireContextA(&provider, NULL, MS_ENH_RSA_AES_PROV_A, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) &&
        CryptImportKey(provider, updatePublicKey, sizeof(updatePublicKey), 0, 0, &key) &&
        CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash) &&
        CryptHashData(hash, (const BYTE *)text.data() + start, (DWORD)(text.size() - start), 0) &&
        CryptVerifySignatureA(hash, signature, sizeof(signature), key, NULL, 0);
    if (hash) CryptDestroyHash(hash);
    if (key) CryptDestroyKey(key);
    if (provider) CryptReleaseContext(provider, 0);
    if (!ok) return false;
    const std::string domain = "product=opencod1-windows-x86\n";
    if (text.compare(start, domain.size(), domain)) { error = "Wrong update product"; return false; }
    if (!Parse(text.substr(start + domain.size()), m, error)) return false;
    m.signedText = text;
    return true;
}

unsigned Current(const std::string &root);

std::string Root() {
    char path[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, path, sizeof(path));
    if (!n || n >= sizeof(path)) return "";
    std::string s(path); return s.substr(0, s.find_last_of("\\/"));
}
std::string Stage(const std::string &root) { return root + "\\.cod-update"; }
std::string Number(size_t n) { return std::to_string(n); }
std::string Sha(const std::string &path) {
    HCRYPTPROV provider = 0; HCRYPTHASH hash = 0;
    HANDLE f = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return "";
    bool ok = CryptAcquireContextA(&provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) != 0;
    if (ok) ok = CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash) != 0;
    BYTE bytes[32768], digest[32]; DWORD got, size = sizeof(digest);
    while (ok) {
        if (!ReadFile(f, bytes, sizeof(bytes), &got, NULL)) { ok = false; break; }
        if (!got) break;
        ok = CryptHashData(hash, bytes, got, 0) != 0;
    }
    if (ok) ok = CryptGetHashParam(hash, HP_HASHVAL, digest, &size, 0) != 0;
    if (hash) CryptDestroyHash(hash);
    if (provider) CryptReleaseContext(provider, 0);
    CloseHandle(f);
    if (!ok) return "";
    const char *hex = "0123456789abcdef"; std::string out;
    for (BYTE b : digest) { out += hex[b >> 4]; out += hex[b & 15]; }
    return out;
}
bool Fetch(const std::string &url, const std::string &path, std::string *text, Job *j) {
    WCHAR wide[2048], host[256], resource[2048]; URL_COMPONENTS u = {};
    HINTERNET session = NULL, connection = NULL, request = NULL;
    HANDLE file = INVALID_HANDLE_VALUE;
    bool ok = false; DWORD code = 0, size, got, written, count = 0;
    DWORD limit = text ? 65536 : 128 * 1024 * 1024;
    BYTE buffer[32768];
    u.dwStructSize = sizeof(u);
    u.dwHostNameLength = u.dwUrlPathLength = u.dwExtraInfoLength = (DWORD)-1;
    u.dwUserNameLength = u.dwPasswordLength = (DWORD)-1;
    if (!URL(url) || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, url.c_str(), -1, wide, 2048) ||
        !WinHttpCrackUrl(wide, 0, 0, &u) || !u.dwHostNameLength || u.dwHostNameLength >= 256 ||
        u.dwUserNameLength || u.dwPasswordLength || u.dwUrlPathLength + u.dwExtraInfoLength >= 2048) goto done;
    memcpy(host, u.lpszHostName, u.dwHostNameLength * 2); host[u.dwHostNameLength] = 0;
    memcpy(resource, u.lpszUrlPath, u.dwUrlPathLength * 2);
    if (u.dwExtraInfoLength) memcpy(resource + u.dwUrlPathLength, u.lpszExtraInfo, u.dwExtraInfoLength * 2);
    resource[u.dwUrlPathLength + u.dwExtraInfoLength] = 0;
    session = WinHttpOpen(L"CoD 1.1x updater", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!session) goto done;
    WinHttpSetTimeouts(session, 10000, 10000, 10000, 15000);
    connection = WinHttpConnect(session, host, u.nPort, 0); if (!connection) goto done;
    request = WinHttpOpenRequest(connection, L"GET", resource, NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, u.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
    if (!request) goto done;
    { DWORD policy = WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH;
      WinHttpSetOption(request, WINHTTP_OPTION_AUTOLOGON_POLICY, &policy, sizeof(policy)); }
    if (!WinHttpSendRequest(request, L"Cache-Control: no-cache\r\n", (DWORD)-1, NULL, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, NULL)) goto done;
    size = sizeof(code);
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &code, &size, NULL) || code != 200) goto done;
    if (!text) {
        file = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE) goto done;
    }
    for (;;) {
        if (!WinHttpReadData(request, buffer, sizeof(buffer), &got)) goto done;
        if (!got) break;
        if (got > limit - count) goto done;
        count += got;
        if (text) text->append((char *)buffer, got);
        else if (!WriteFile(file, buffer, got, &written, NULL) || written != got) goto done;
        if (j && !text) InterlockedExchangeAdd(&j->downloaded, got);
    }
    ok = count != 0;
done:
    if (!ok && j) j->error = "Update download failed (HTTP " + Number(code) + ", Windows " + Number(GetLastError()) + "): " + url;
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (session) WinHttpCloseHandle(session);
    if (!ok && !path.empty()) DeleteFileA(path.c_str());
    return ok;
}
bool Binary(const std::string &path, bool dll) {
    std::ifstream f(path, std::ios::binary); IMAGE_DOS_HEADER dos = {}; IMAGE_NT_HEADERS32 nt = {};
    if (!f.read((char *)&dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < sizeof(dos) || dos.e_lfanew > 1024 * 1024) return false;
    f.seekg(dos.e_lfanew);
    return f.read((char *)&nt, sizeof(nt)) && nt.Signature == IMAGE_NT_SIGNATURE &&
        nt.FileHeader.Machine == IMAGE_FILE_MACHINE_I386 && nt.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
        !!(nt.FileHeader.Characteristics & IMAGE_FILE_DLL) == dll;
}
DWORD WINAPI Worker(void *arg) {
    Job *j = (Job *)arg;
    try {
        if (!j->install) {
            std::string text;
            j->ok = Fetch(endpoint, "", &text, j) && Authenticate(text, j->manifest, j->error);
            return 0;
        }
        Manifest verified;
        if (!Authenticate(j->manifest.signedText, verified, j->error)) return 0;
        j->manifest = verified;
        if (verified.build <= Current(j->root)) { j->error = "Update is not newer than the installed build"; return 0; }
        std::string stage = Stage(j->root);
        if (!CreateDirectoryA(stage.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) { j->error = "Cannot create update staging folder"; return 0; }
        DWORD attr = GetFileAttributesA(stage.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_REPARSE_POINT)) { j->error = "Invalid update staging folder"; return 0; }
        for (size_t i = 0; i < j->manifest.files.size(); ++i) {
            File &f = j->manifest.files[i];
            std::string path = stage + "\\" + Number(i) + ".new";
            if (!Fetch(f.url, path, NULL, j)) return 0;
            std::string hash = Sha(path);
            if (hash.empty() || hash != f.hash || !Binary(path, Lower(f.path).find(".dll") != std::string::npos)) {
                j->error = "Invalid update binary or SHA-256 mismatch: " + f.path; DeleteFileA(path.c_str()); return 0;
            }
            f.hash = hash;
        }
        std::ofstream plan(stage + "\\plan.txt", std::ios::binary | std::ios::trunc);
        plan << j->manifest.signedText;
        plan.close(); j->ok = !plan.fail();
        if (!j->ok) j->error = "Cannot write update plan";
    } catch (...) { j->error = "Update failed: insufficient resources"; }
    return 0;
}
bool Spawn(const std::string &exe, std::string args, const std::string &root, bool inherit = false) {
    STARTUPINFOA si = {}; PROCESS_INFORMATION pi = {}; si.cb = sizeof(si);
    std::string command = "\"" + exe + "\" " + args;
    if (!CreateProcessA(exe.c_str(), &command[0], NULL, NULL, inherit, CREATE_NO_WINDOW, NULL, root.c_str(), &si, &pi)) return false;
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess); return true;
}
unsigned Current(const std::string &root) {
    std::ifstream f(Stage(root) + "\\installed-build.txt"); unsigned build = 0; f >> build;
    return (std::max)(build, updateBuild);
}
void Report(const std::string &text) {
    Com_Printf("Auto-update: %s\n", text.c_str());
    Cvar_Set2("cl_updateStatus", text.c_str(), 1);
}
}

extern "C" void CL_UpdateInit(void) {
    Cvar_Get("cl_updateStatus", "", cvarROM);
    Cvar_Get("cl_updateforced", "0", cvarROM);
    Cvar_Get("cl_updateDownloading", "0", cvarROM);
    Cvar_Get("cl_updateDownloadedBytes", "0", cvarROM);
    CL_UpdateCheck();
}
extern "C" void CL_UpdateCheck(void) {
    if (job) return;
    job = new Job(); job->root = Root();
    job->thread = CreateThread(NULL, 0, Worker, job, 0, NULL);
    if (!job->thread) { delete job; job = NULL; Report("Cannot start update check"); }
}
extern "C" void CL_UpdateStart(void) {
    if (job) return;
    if (!ready) { CL_UpdateCheck(); return; }
    job = new Job(); job->install = true; job->manifest = available; job->root = Root();
    job->thread = CreateThread(NULL, 0, Worker, job, 0, NULL);
    if (!job->thread) { delete job; job = NULL; Report("Cannot start update download"); return; }
    Cvar_Set2("cl_updateDownloading", "1", 1);
    Report("Downloading update. The game will restart after verification.");
}
extern "C" void CL_UpdateFrame(void) {
    if (!job) return;
    if (job->install) Cvar_Set2("cl_updateDownloadedBytes", Number(InterlockedCompareExchange(&job->downloaded, 0, 0)).c_str(), 1);
    if (WaitForSingleObject(job->thread, 0) != WAIT_OBJECT_0) return;
    Job *j = job; job = NULL; CloseHandle(j->thread);
    Cvar_Set2("cl_updateDownloading", "0", 1);
    if (!j->ok) {
        ready = false; available = Manifest();
        Cvar_Set2("cl_updateavailable", "0", 1);
        Cvar_Set2("cl_updateforced", "0", 1);
        Report(j->error);
    }
    else if (!j->install) {
        available = j->manifest; ready = available.build > Current(j->root);
        Cvar_Set2("cl_updateavailable", ready ? "1" : "0", 1);
        Cvar_Set2("cl_updateversion", Number(available.build).c_str(), 1);
        Cvar_Set2("cl_updateoldversion", Number(Current(j->root)).c_str(), 1);
        Cvar_Set2("cl_updateforced", available.forced ? "1" : "0", 1);
        Report(ready ? "Build " + Number(available.build) + " available. Use Auto Update in the menu." : "Already up to date.");
    } else {
        char self[MAX_PATH]; GetModuleFileNameA(NULL, self, sizeof(self));
        std::string helper = j->root + "\\cod-update-helper.exe";
        HANDLE parent = NULL;
        DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &parent, SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, TRUE, 0);
        if (!parent || !CopyFileA(self, helper.c_str(), FALSE) ||
            !Spawn(helper, "--cod-update-helper " + Number((size_t)parent), j->root, true)) Report("Cannot start update helper; game files were not replaced.");
        else { Report("Update verified. Restarting..."); Cbuf_AddText("quit\n"); }
        if (parent) CloseHandle(parent);
    }
    delete j;
}

static int ApplyUpdate(const std::string &root, bool restart) {
    std::string stage = Stage(root), error;
    Manifest m; std::ifstream plan(stage + "\\plan.txt", std::ios::binary);
    char bytes[65537]; plan.read(bytes, sizeof(bytes));
    std::string text(bytes, (size_t)plan.gcount());
    if (!Authenticate(text, m, error) || m.build <= Current(root)) return 1;
    std::vector<bool> existed(m.files.size(), false);
    size_t applied = 0;
    bool ok = true;
    /* Verify every staged file and prepare backups before replacing any file. */
    for (size_t i = 0; i < m.files.size(); ++i) {
        std::string source = stage + "\\" + Number(i) + ".new", dest = root + "\\" + m.files[i].path;
        DWORD attr = GetFileAttributesA(dest.c_str());
        existed[i] = attr != INVALID_FILE_ATTRIBUTES;
        if ((existed[i] && (attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) ||
            (GetFileAttributesA(stage.c_str()) & FILE_ATTRIBUTE_REPARSE_POINT) ||
            (m.files[i].path.find('/') != std::string::npos && (GetFileAttributesA((root + "\\main").c_str()) & FILE_ATTRIBUTE_REPARSE_POINT)) ||
            Sha(source) != m.files[i].hash || !Binary(source, Lower(m.files[i].path).find(".dll") != std::string::npos) ||
            (existed[i] && !CopyFileA(dest.c_str(), (stage + "\\" + Number(i) + ".bak").c_str(), FALSE))) { ok = false; break; }
    }
    if (ok) for (; applied < m.files.size(); ++applied) {
        std::string source = stage + "\\" + Number(applied) + ".new", dest = root + "\\" + m.files[applied].path;
        if (!MoveFileExA(source.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) { ok = false; break; }
    }
    bool restored = true;
    if (!ok) while (applied) {
        --applied;
        std::string dest = root + "\\" + m.files[applied].path;
        if (existed[applied]) restored &= CopyFileA((stage + "\\" + Number(applied) + ".bak").c_str(), dest.c_str(), FALSE) != 0;
        else restored &= DeleteFileA(dest.c_str()) != 0;
    }
    if (ok) { std::ofstream stamp(stage + "\\installed-build.txt"); stamp << m.build; }
    std::ofstream log(stage + "\\result.txt");
    log << (ok ? "Update installed" : restored ? "Update failed; originals restored" : "Update failed; restore backups manually") << "\n"; log.close();
    if (!ok && restart) MessageBoxA(NULL, restored ? "Update failed. Original files were preserved or restored." : "Update failed. Restore files from .cod-update backups before launching.", "Call of Duty update", MB_OK | MB_ICONERROR);
    if (restart && (ok || restored)) Spawn(root + "\\CodMP.exe", "", root);
    return ok ? 0 : 1;
}

extern "C" int CL_UpdateHelper(const char *commandLine) {
    const char *prefix = "--cod-update-helper ";
    if (strncmp(commandLine, prefix, strlen(prefix))) return -1;
    std::string handleText = Trim(commandLine + strlen(prefix));
    if (handleText.empty() || handleText.find_first_not_of("0123456789") != std::string::npos) return 1;
    HANDLE parent = (HANDLE)(size_t)strtoul(handleText.c_str(), NULL, 10);
    if (!parent || !GetProcessId(parent)) return 1;
    DWORD wait = WaitForSingleObject(parent, 60000); CloseHandle(parent);
    if (wait != WAIT_OBJECT_0) return 1;
    return ApplyUpdate(Root(), true);
}
