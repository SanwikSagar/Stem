@echo off
setlocal

echo ========================================
echo   STEM VCS - CLI Build (MinGW)
echo ========================================
echo.

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

REM Compile core files
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\config.o src\core\config.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\error.o src\core\error.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\hash.o src\core\hash.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\index.o src\core\index.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\init.o src\core\init.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\object.o src\core\object.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\reference.o src\core\reference.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\repository.o src\core\repository.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\util.o src\core\util.c

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Core compilation failed!
    pause
    exit /b 1
)

echo [OK] Core library compiled
echo.
echo [INFO] Compiling CLI tool...

REM Compile CLI files
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\main.o src\cli\main.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_add.o src\cli\cmd_add.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_commit.o src\cli\cmd_commit.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_init.o src\cli\cmd_init.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_remote.o src\cli\cmd_remote.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_status.o src\cli\cmd_status.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_branch.o src\cli\cmd_branch.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_merge.o src\cli\cmd_merge.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_history.o src\cli\cmd_history.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\cmd_tag.o src\cli\cmd_tag.c
%CC% -c -O2 -Wall -Wextra -Iinclude -o obj\output.o src\cli\output.c

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CLI compilation failed!
    pause
    exit /b 1
)

echo [OK] CLI compiled
echo.
echo [INFO] Linking STEM executable...

REM Link everything
%CC% -o bin\stem.exe ^
    obj\config.o ^
    obj\error.o ^
    obj\hash.o ^
    obj\index.o ^
    obj\init.o ^
    obj\object.o ^
    obj\reference.o ^
    obj\repository.o ^
    obj\util.o ^
    obj\main.o ^
    obj\cmd_add.o ^
    obj\cmd_commit.o ^
    obj\cmd_init.o ^
    obj\cmd_remote.o ^
    obj\cmd_status.o ^
    obj\cmd_branch.o ^
    obj\cmd_merge.o ^
    obj\cmd_history.o ^
    obj\cmd_tag.o ^
    obj\output.o ^
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
echo Created: bin\stem.exe
echo.

REM Test the binary
bin\stem.exe --version

echo.
echo Try these commands:
echo   bin\stem help
echo   bin\stem init
echo   bin\stem doctor
echo.

pause
