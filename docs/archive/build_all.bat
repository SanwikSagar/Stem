@echo off
setlocal

echo ========================================
echo   STEM VCS - Complete Build System
echo ========================================
echo.
echo Building both CLI and GUI applications...
echo.

REM Use local MinGW
set PATH=%~dp0tools\mingw64\bin;%PATH%

REM Clean build
if exist "obj\*.o" del /Q obj\*.o 2>nul

echo [1/2] Building CLI...
echo.
call build_cli.bat
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CLI build failed!
    pause
    exit /b 1
)

echo.
echo [2/2] Building GUI...
echo.
call build_gui.bat
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] GUI build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   ALL BUILDS COMPLETE!
echo ========================================
echo.
echo Created executables:
echo   bin\stem.exe         - Command-line tool
echo   bin\stem-gui.exe     - Desktop GUI application
echo.
echo Quick start:
echo   1. CLI: bin\stem.exe help
echo   2. GUI: bin\stem-gui.exe
echo.

pause
