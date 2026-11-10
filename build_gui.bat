@echo off
setlocal

echo ========================================
echo   STEM VCS - Web GUI Build (MinGW)
echo ========================================
echo.

REM Stop any running GUI servers first
taskkill /F /IM stem_gui.exe 2>nul >nul
if %ERRORLEVEL% EQU 0 (
    echo [INFO] Stopped running GUI server
    timeout /t 1 /nobreak >nul
)

REM Use local MinGW
set PATH=%~dp0tools\mingw64\bin;%PATH%
set CC=gcc

REM Verify compiler
where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] GCC not found in tools\mingw64\bin!
    pause
    exit /b 1
)

echo [OK] Using: 
gcc --version | findstr gcc
echo.

REM Create directories
if not exist "bin" mkdir bin
if not exist "obj" mkdir obj

echo [INFO] Compiling core library...

REM Compile core files (reuse from CLI)
if not exist "obj\config.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\config.o src\core\config.c
if not exist "obj\error.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\error.o src\core\error.c
if not exist "obj\hash.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\hash.o src\core\hash.c
if not exist "obj\index.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\index.o src\core\index.c
if not exist "obj\init.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\init.o src\core\init.c
if not exist "obj\object.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\object.o src\core\object.c
if not exist "obj\reference.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\reference.o src\core\reference.c
if not exist "obj\repository.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\repository.o src\core\repository.c
if not exist "obj\util.o" %CC% -c -O2 -Wall -Wextra -Iinclude -o obj\util.o src\core\util.c

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Core compilation failed!
    pause
    exit /b 1
)

echo [OK] Core library compiled
echo.
echo [INFO] Compiling Web GUI server (PRODUCTION)...

REM Compile GUI server with real STEM integration
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\server.o src\gui_web\server_real.c

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] GUI server compilation failed!
    pause
    exit /b 1
)

echo [OK] GUI server compiled
echo.
echo [INFO] Linking STEM GUI executable...

REM Link everything
%CC% -o bin\stem_gui.exe ^
    obj\config.o ^
    obj\error.o ^
    obj\hash.o ^
    obj\index.o ^
    obj\init.o ^
    obj\object.o ^
    obj\reference.o ^
    obj\repository.o ^
    obj\util.o ^
    obj\server.o ^
    -lws2_32

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Linking failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo.
echo Created: bin\stem_gui.exe
echo.
echo Start the GUI server:
echo   bin\stem_gui.exe
echo.
echo Then open your browser to:
echo   http://localhost:8765
echo.

pause
