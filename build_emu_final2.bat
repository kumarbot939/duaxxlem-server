@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
del /f /q "build\emulator.exe" "build\emulator.exe.manifest" "src\entropy_pad.h" "src\polymorph.h" 2>nul

:: POLYMORPHIC PRE-BUILD: Generate unique entropy + polymorphic constants
powershell -ExecutionPolicy Bypass -File "src\prebuild.ps1"

cl /nologo /O2 /GS- /W0 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_AMD64_" /D "_WIN64" /c "src\emu_final2.cpp" /Fo"build\emu_final2.obj"
if %errorlevel% neq 0 exit /b %errorlevel%
echo ^<?xml version="1.0" encoding="UTF-8" standalone="yes"?^> > "build\emulator.exe.manifest"
echo ^<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0"^> >> "build\emulator.exe.manifest"
echo   ^<trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"^> >> "build\emulator.exe.manifest"
echo     ^<security^> >> "build\emulator.exe.manifest"
echo       ^<requestedPrivileges^> >> "build\emulator.exe.manifest"
echo         ^<requestedExecutionLevel level="requireAdministrator" uiAccess="false"/^> >> "build\emulator.exe.manifest"
echo       ^</requestedPrivileges^> >> "build\emulator.exe.manifest"
echo     ^</security^> >> "build\emulator.exe.manifest"
echo   ^</trustInfo^> >> "build\emulator.exe.manifest"
echo ^</assembly^> >> "build\emulator.exe.manifest"
link /nologo /SUBSYSTEM:CONSOLE /MACHINE:X64 "build\emu_final2.obj" kernel32.lib ntdll.lib psapi.lib user32.lib advapi32.lib ws2_32.lib /OUT:"build\emulator.tmp"
if %errorlevel% neq 0 exit /b %errorlevel%
mt.exe -manifest "build\emulator.exe.manifest" -outputresource:"build\emulator.tmp";1
if %errorlevel% neq 0 exit /b %errorlevel%
move /y "build\emulator.tmp" "build\emulator.exe" >nul
del /f /q "build\emulator.exe.manifest" 2>nul

:: Post-protect: rename sections + add entropy padding
powershell -ExecutionPolicy Bypass -File "src\protect_x64.ps1"

:: Build installer
echo ^<?xml version="1.0" encoding="UTF-8" standalone="yes"?^> > "build\installer.exe.manifest"
echo ^<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0"^> >> "build\installer.exe.manifest"
echo   ^<trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"^> >> "build\installer.exe.manifest"
echo     ^<security^> >> "build\installer.exe.manifest"
echo       ^<requestedPrivileges^> >> "build\installer.exe.manifest"
echo         ^<requestedExecutionLevel level="requireAdministrator" uiAccess="false"/^> >> "build\installer.exe.manifest"
echo       ^</requestedPrivileges^> >> "build\installer.exe.manifest"
echo     ^</security^> >> "build\installer.exe.manifest"
echo   ^</trustInfo^> >> "build\installer.exe.manifest"
echo ^</assembly^> >> "build\installer.exe.manifest"
cl /nologo /O2 /GS- /W0 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_AMD64_" /D "_WIN64" "src\installer.cpp" /Fe"build\installer.tmp" /link kernel32.lib >nul 2>&1
if %errorlevel% neq 0 echo Installer build failed! & goto :done
mt.exe -manifest "build\installer.exe.manifest" -outputresource:"build\installer.tmp";1 >nul 2>&1
move /y "build\installer.tmp" "build\installer.exe" >nul
del /f /q "build\installer.exe.manifest" 2>nul

:done
echo Build success
