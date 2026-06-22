#include <windows.h>

#define FAKE_HANDLE ((HANDLE)0xDEADBEEF)

// Minimal OBJECT_ATTRIBUTES and UNICODE_STRING for x64
typedef struct { USHORT len, max; PWCHAR buf; } MYUNISTR;
typedef struct { ULONG len; HANDLE root; MYUNISTR* name; ULONG attr; void* sd; void* sqos; } MYOBJATTR;

typedef NTSTATUS(NTAPI* fnNtCreateFile)(PHANDLE, ACCESS_MASK, MYOBJATTR*, PVOID, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
typedef HANDLE(WINAPI* fnCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI* fnCreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL(WINAPI* fnDeviceIoControl)(HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(WINAPI* fnCloseHandle)(HANDLE);

static fnNtCreateFile OrigNtCreateFile = NULL;
static fnCreateFileA OrigCreateFileA = NULL;
static fnCreateFileW OrigCreateFileW = NULL;
static fnDeviceIoControl OrigDeviceIoControl = NULL;
static fnCloseHandle OrigCloseHandle = NULL;
static BYTE origBytes_NC[5], origBytes_CA[5], origBytes_CW[5], origBytes_DI[5], origBytes_CH[5];
static void* addr_NC, * addr_CA, * addr_CW, * addr_DI, * addr_CH;

void SaveAndPatch(void** addr, const char* mod, const char* name, void* hook, BYTE* save) {
    HMODULE h = GetModuleHandleA(mod);
    if (!h) h = LoadLibraryA(mod);
    if (!h) return;
    void* target = GetProcAddress(h, name);
    if (!target) return;
    *addr = target;
    DWORD op;
    VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &op);
    memcpy(save, target, 5);
    BYTE jmp[5] = { 0xE9 };
    *(int*)(jmp + 1) = (BYTE*)hook - (BYTE*)target - 5;
    memcpy(target, jmp, 5);
    VirtualProtect(target, 5, op, &op);
}

void Unpatch(void* addr, BYTE* save) {
    DWORD op;
    VirtualProtect(addr, 5, PAGE_EXECUTE_READWRITE, &op);
    memcpy(addr, save, 5);
    VirtualProtect(addr, 5, op, &op);
}

void Repatch(void* addr, void* hook) {
    DWORD op;
    VirtualProtect(addr, 5, PAGE_EXECUTE_READWRITE, &op);
    BYTE jmp[5] = { 0xE9 };
    *(int*)(jmp + 1) = (BYTE*)hook - (BYTE*)addr - 5;
    memcpy(addr, jmp, 5);
    VirtualProtect(addr, 5, op, &op);
}

NTSTATUS NTAPI HookNtCreateFile(PHANDLE fh, ACCESS_MASK da, MYOBJATTR* oa, PVOID isb,
    PLARGE_INTEGER as, ULONG fa, ULONG sa, ULONG cd, ULONG co, PVOID eb, ULONG el) {
    if (oa && oa->name && oa->name->buf) {
        if (wcsstr(oa->name->buf, L"\\\\.\\VGC") || wcsstr(oa->name->buf, L"\\Device\\VGC") || wcsstr(oa->name->buf, L"VGC")) {
            *fh = FAKE_HANDLE;
            return 0;
        }
    }
    Unpatch(addr_NC, origBytes_NC);
    NTSTATUS r = OrigNtCreateFile(fh, da, oa, isb, as, fa, sa, cd, co, eb, el);
    Repatch(addr_NC, HookNtCreateFile);
    return r;
}

HANDLE WINAPI HookCreateFileA(LPCSTR path, DWORD dwAccess, DWORD dwShare,
    LPSECURITY_ATTRIBUTES sa, DWORD disp, DWORD flags, HANDLE tmpl) {
    if (path && strstr(path, "\\\\.\\VGC")) return FAKE_HANDLE;
    Unpatch(addr_CA, origBytes_CA);
    HANDLE r = OrigCreateFileA(path, dwAccess, dwShare, sa, disp, flags, tmpl);
    Repatch(addr_CA, HookCreateFileA);
    return r;
}

HANDLE WINAPI HookCreateFileW(LPCWSTR path, DWORD dwAccess, DWORD dwShare,
    LPSECURITY_ATTRIBUTES sa, DWORD disp, DWORD flags, HANDLE tmpl) {
    if (path) {
        wchar_t vgc[] = L"\\\\.\\VGC";
        if (wcsstr(path, vgc)) return FAKE_HANDLE;
    }
    Unpatch(addr_CW, origBytes_CW);
    HANDLE r = OrigCreateFileW(path, dwAccess, dwShare, sa, disp, flags, tmpl);
    Repatch(addr_CW, HookCreateFileW);
    return r;
}

BOOL WINAPI HookDeviceIoControl(HANDLE h, DWORD code, LPVOID in, DWORD inSz,
    LPVOID out, DWORD outSz, LPDWORD ret, LPOVERLAPPED ov) {
    if (h == FAKE_HANDLE) {
        if (ret) *ret = 0;
        return TRUE;
    }
    Unpatch(addr_DI, origBytes_DI);
    BOOL r = OrigDeviceIoControl(h, code, in, inSz, out, outSz, ret, ov);
    Repatch(addr_DI, HookDeviceIoControl);
    return r;
}

BOOL WINAPI HookCloseHandle(HANDLE h) {
    if (h == FAKE_HANDLE) return TRUE;
    Unpatch(addr_CH, origBytes_CH);
    BOOL r = OrigCloseHandle(h);
    Repatch(addr_CH, HookCloseHandle);
    return r;
}

__declspec(dllexport) void HookInit() {
    SaveAndPatch(&addr_NC, "ntdll.dll", "NtCreateFile", HookNtCreateFile, origBytes_NC);
    SaveAndPatch(&addr_CA, "kernel32.dll", "CreateFileA", HookCreateFileA, origBytes_CA);
    SaveAndPatch(&addr_CW, "kernel32.dll", "CreateFileW", HookCreateFileW, origBytes_CW);
    SaveAndPatch(&addr_DI, "kernel32.dll", "DeviceIoControl", HookDeviceIoControl, origBytes_DI);
    SaveAndPatch(&addr_CH, "kernel32.dll", "CloseHandle", HookCloseHandle, origBytes_CH);
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hMod);
        HookInit();
    }
    return TRUE;
}
